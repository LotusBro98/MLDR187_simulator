#include <cassert>
#include <cstring>

#include "debug_module.h"
#include "debug_defines.h"

#include "debug_rom_defines.h"
#include "debug_rom.h"
#include "jtag_dtm.h"
#include "remote_bitbang.h"

#define get_field(reg, mask) (((reg) & (mask)) / ((mask) & ~((mask) << 1)))
#define set_field(reg, mask, val) (((reg) & ~(mask)) | (((val) * ((mask) & ~((mask) << 1))) & (mask)))

#define DEBUG_START             0x0
#define DEBUG_END               (0x1000 - 1)

#if 0
#  define D(x) x
#else
#  define D(x)
#endif

// Return the number of bits wide that a field has to be to encode up to n
// different values.
// 1->0, 2->1, 3->2, 4->2
static unsigned field_width(unsigned n)
{
    unsigned i = 0;
    n -= 1;
    while (n) {
        i++;
        n >>= 1;
    }
    return i;
}

///////////////////////// debug_module_t

debug_module_t::debug_module_t(const debug_module_config_t &config) :
        Device(),
        config(config),
        program_buffer_bytes((config.support_impebreak ? 4 : 0) + 4*config.progbufsize),
        debug_progbuf_start(debug_data_start - program_buffer_bytes),
        debug_abstract_start(debug_progbuf_start - debug_abstract_size*4),
        custom_base(0),
        // The spec lets a debugger select nonexistent harts. Create hart_state for
        // them because I'm too lazy to add the code to just ignore accesses.
        hart_state(),
        rti_remaining(0),
        jtagDtm(this, 5),
        remoteBitbang(2442, &jtagDtm)
{
    D(fprintf(stderr, "debug_data_start=0x%x\n", debug_data_start));
    D(fprintf(stderr, "debug_progbuf_start=0x%x\n", debug_progbuf_start));
    D(fprintf(stderr, "debug_abstract_start=0x%x\n", debug_abstract_start));
    D(fflush(stderr));

    program_buffer = new uint8_t[program_buffer_bytes];

    memset(&debug_rom_flags, 0, sizeof(debug_rom_flags));
    memset(program_buffer, 0, program_buffer_bytes);
    memset(dmdata, 0, sizeof(dmdata));

    if (config.support_impebreak) {
        program_buffer[4*config.progbufsize] = ebreak();
        program_buffer[4*config.progbufsize+1] = ebreak() >> 8;
        program_buffer[4*config.progbufsize+2] = ebreak() >> 16;
        program_buffer[4*config.progbufsize+3] = ebreak() >> 24;
    }

    write32(debug_rom_whereto, 0,
            jal(REG_ZERO, debug_abstract_start - DEBUG_ROM_WHERETO));

    memset(debug_abstract, 0, sizeof(debug_abstract));

    reset();
}

debug_module_t::~debug_module_t()
{
    delete[] program_buffer;
}

void debug_module_t::reset()
{
    core.cpu.halt_request = false;

    memset(&dmcontrol, 0, sizeof(dmcontrol));

    memset(&dmstatus, 0, sizeof(dmstatus));
    dmstatus.impebreak = config.support_impebreak;
    dmstatus.authenticated = !config.require_authentication;
    dmstatus.version = 2;

    memset(&abstractcs, 0, sizeof(abstractcs));
    abstractcs.datacount = sizeof(dmdata) / 4;
    abstractcs.progbufsize = config.progbufsize;

    memset(&abstractauto, 0, sizeof(abstractauto));

    memset(&sbcs, 0, sizeof(sbcs));
    if (config.max_sba_data_width > 0) {
        sbcs.version = 1;
        sbcs.asize = sizeof(reg_t) * 8;
    }
    if (config.max_sba_data_width >= 64)
        sbcs.access64 = true;
    if (config.max_sba_data_width >= 32)
        sbcs.access32 = true;
    if (config.max_sba_data_width >= 16)
        sbcs.access16 = true;
    if (config.max_sba_data_width >= 8)
        sbcs.access8 = true;

    challenge = rand();
}

uint32_t debug_module_t::read(uint32_t addr, uint32_t len)
{
    addr = DEBUG_START + addr;
    const void* src = nullptr;

    if (addr >= DEBUG_ROM_ENTRY && (addr + len) <= (DEBUG_ROM_ENTRY + debug_rom_raw_len)) {
        src = debug_rom_raw + addr - DEBUG_ROM_ENTRY;
    } else if (addr >= DEBUG_ROM_WHERETO && (addr + len) <= (DEBUG_ROM_WHERETO + 4)) {
        src = debug_rom_whereto + addr - DEBUG_ROM_WHERETO;
    } else if (addr >= DEBUG_ROM_FLAGS && ((addr + len) <= DEBUG_ROM_FLAGS + 1024)) {
        src = &debug_rom_flags + addr - DEBUG_ROM_FLAGS;
    } else if (addr >= debug_abstract_start && ((addr + len) <= (debug_abstract_start + sizeof(debug_abstract)))) {
        src = debug_abstract + addr - debug_abstract_start;
    } else if (addr >= debug_data_start && (addr + len) <= (debug_data_start + sizeof(dmdata))) {
        src = dmdata + addr - debug_data_start;
    } else if (addr >= debug_progbuf_start && ((addr + len) <= (debug_progbuf_start + program_buffer_bytes))) {
        src = program_buffer + addr - debug_progbuf_start;
    } else {
//        D(fprintf(stderr, "ERROR: invalid read from debug module: %zd bytes at 0x%08x\n", len, addr));
        throw EXC_LAF;
    }

    D(fprintf(stderr, "load(addr=0x%lx, len=%d, bytes=0x%08x); \n", addr, (unsigned) len, *(uint32_t*)src));

    switch (len) {
        case 1:
            return *(uint8_t*)src;
        case 2:
            return *(uint16_t*)src;
        case 4:
            return *(uint32_t*)src;
        default:
            throw EXC_LAF;
    }
}

void debug_module_t::write(uint32_t addr, uint32_t value, uint32_t len)
{
    D(fprintf(stderr, "store(addr=0x%lx, len=%d, bytes=0x%08x); \n", addr, (unsigned) len, value));

    addr = DEBUG_START + addr;
    const void* dst = nullptr;

    if (addr >= debug_data_start && (addr + len) <= (debug_data_start + sizeof(dmdata))) {
        dst = dmdata + addr - debug_data_start;
    } else if (addr >= debug_progbuf_start && ((addr + len) <= (debug_progbuf_start + program_buffer_bytes))) {
        dst = program_buffer + addr - debug_progbuf_start;
    }
    else if (addr == DEBUG_ROM_HALTED) {
        hart_state.halted = true;
        if (!(debug_rom_flags & (1 << DEBUG_ROM_FLAG_GO)))
            abstract_command_completed = true;
    }
    else if (addr == DEBUG_ROM_GOING) {
        debug_rom_flags &= ~(1 << DEBUG_ROM_FLAG_GO);
    }
    else if (addr == DEBUG_ROM_RESUMING) {
        hart_state.halted = false;
        hart_state.resumeack = true;
        debug_rom_flags &= ~(1 << DEBUG_ROM_FLAG_RESUME);
    }
    else if (addr == DEBUG_ROM_EXCEPTION) {
        if (abstractcs.cmderr == CMDERR_NONE)
            abstractcs.cmderr = CMDERR_EXCEPTION;
    }
    else {
        D(fprintf(stderr, "ERROR: invalid store to debug module: %zd bytes at 0x%016\n", len, addr));
        throw EXC_SAF;
    }

    if (dst == nullptr)
        return;

    switch (len) {
        case 1:
            *(uint8_t*)dst = value;
            return;
        case 2:
            *(uint16_t*)dst = value;
            return;
        case 4:
            *(uint32_t*)dst = value;
            return;
        default:
            throw EXC_SAF;
    }
}

void debug_module_t::write32(uint8_t *memory, unsigned int index, uint32_t value)
{
    uint8_t* base = memory + index * 4;
    base[0] = value & 0xff;
    base[1] = (value >> 8) & 0xff;
    base[2] = (value >> 16) & 0xff;
    base[3] = (value >> 24) & 0xff;
}

uint32_t debug_module_t::read32(uint8_t *memory, unsigned int index)
{
    uint8_t* base = memory + index * 4;
    uint32_t value = ((uint32_t) base[0]) |
                     (((uint32_t) base[1]) << 8) |
                     (((uint32_t) base[2]) << 16) |
                     (((uint32_t) base[3]) << 24);
    return value;
}

unsigned debug_module_t::sb_access_bits()
{
    return 8 << sbcs.sbaccess;
}

void debug_module_t::sb_autoincrement()
{
    if (!sbcs.autoincrement || !config.max_sba_data_width)
        return;

    uint64_t value = sbaddress[0] + sb_access_bits() / 8;
    sbaddress[0] = value;
    uint32_t carry = value >> 32;

    value = sbaddress[1] + carry;
    sbaddress[1] = value;
    carry = value >> 32;

    value = sbaddress[2] + carry;
    sbaddress[2] = value;
    carry = value >> 32;

    sbaddress[3] += carry;
}

void debug_module_t::sb_read()
{
    reg_t address = ((uint64_t) sbaddress[1] << 32) | sbaddress[0];
    try {
        if (sbcs.sbaccess == 0 && config.max_sba_data_width >= 8) {
            sbdata[0] = core.bus.read_byte(address);
        } else if (sbcs.sbaccess == 1 && config.max_sba_data_width >= 16) {
            sbdata[0] = core.bus.read_half(address);
        } else if (sbcs.sbaccess == 2 && config.max_sba_data_width >= 32) {
            sbdata[0] = core.bus.read_word(address);
        } else {
            sbcs.error = 3;
        }
    } catch (Exception& t) {
        sbcs.error = 2;
    }
}

void debug_module_t::sb_write()
{
    reg_t address = ((uint64_t) sbaddress[1] << 32) | sbaddress[0];
    D(fprintf(stderr, "sb_write() 0x%x @ 0x%lx\n", sbdata[0], address));
    if (sbcs.sbaccess == 0 && config.max_sba_data_width >= 8) {
        core.bus.write_byte(address, sbdata[0]);
    } else if (sbcs.sbaccess == 1 && config.max_sba_data_width >= 16) {
        core.bus.write_half(address, sbdata[0]);
    } else if (sbcs.sbaccess == 2 && config.max_sba_data_width >= 32) {
        core.bus.write_word(address, sbdata[0]);
    } else {
        sbcs.error = 3;
    }
}

bool debug_module_t::dmi_read(unsigned address, uint32_t *value)
{
    uint32_t result = 0;
    D(fprintf(stderr, "dmi_read(0x%x) -> ", address));
    if (address >= DM_DATA0 && address < DM_DATA0 + abstractcs.datacount) {
        unsigned i = address - DM_DATA0;
        result = read32(dmdata, i);
        if (abstractcs.busy) {
            result = -1;
            D(fprintf(stderr, "\ndmi_read(0x%02x (data[%d]) -> -1 because abstractcs.busy==true\n", address, i));
        }

        if (abstractcs.busy && abstractcs.cmderr == CMDERR_NONE) {
            abstractcs.cmderr = CMDERR_BUSY;
        }

        if (!abstractcs.busy && ((abstractauto.autoexecdata >> i) & 1)) {
            perform_abstract_command();
        }
    } else if (address >= DM_PROGBUF0 && address < DM_PROGBUF0 + config.progbufsize) {
        unsigned i = address - DM_PROGBUF0;
        result = read32(program_buffer, i);
        if (abstractcs.busy) {
            result = -1;
            D(fprintf(stderr, "\ndmi_read(0x%02x (progbuf[%d]) -> -1 because abstractcs.busy==true\n", address, i));
        }
        if (!abstractcs.busy && ((abstractauto.autoexecprogbuf >> i) & 1)) {
            perform_abstract_command();
        }

    } else {
        switch (address) {
            case DM_DMCONTROL:
            {
                result = set_field(result, DM_DMCONTROL_HALTREQ, dmcontrol.haltreq);
                result = set_field(result, DM_DMCONTROL_RESUMEREQ, dmcontrol.resumereq);
                result = set_field(result, DM_DMCONTROL_HARTSELHI, dmcontrol.hartsel >> DM_DMCONTROL_HARTSELLO_LENGTH);
                result = set_field(result, DM_DMCONTROL_HASEL, dmcontrol.hasel);
                result = set_field(result, DM_DMCONTROL_HARTSELLO, dmcontrol.hartsel);
                result = set_field(result, DM_DMCONTROL_HARTRESET, dmcontrol.hartreset);
                result = set_field(result, DM_DMCONTROL_NDMRESET, dmcontrol.ndmreset);
                result = set_field(result, DM_DMCONTROL_DMACTIVE, dmcontrol.dmactive);
            }
                break;
            case DM_DMSTATUS:
            {
                dmstatus.allhalted = true;
                dmstatus.anyhalted = false;
                dmstatus.allrunning = true;
                dmstatus.anyrunning = false;
                dmstatus.allnonexistant = false;
                dmstatus.anynonexistant = false;
                dmstatus.allunavail = false;
                dmstatus.anyunavail = false;
                dmstatus.anyresumeack = hart_state.resumeack;
                dmstatus.allresumeack = hart_state.resumeack;
                dmstatus.allhalted = hart_state.halted;
                dmstatus.anyhalted = hart_state.halted;
                dmstatus.allrunning = !hart_state.halted;
                dmstatus.anyrunning = !hart_state.halted;

                result = set_field(result, DM_DMSTATUS_IMPEBREAK, dmstatus.impebreak);
                result = set_field(result, DM_DMSTATUS_ALLHAVERESET, hart_state.havereset);
                result = set_field(result, DM_DMSTATUS_ANYHAVERESET, hart_state.havereset);
                result = set_field(result, DM_DMSTATUS_ALLNONEXISTENT, dmstatus.allnonexistant);
                result = set_field(result, DM_DMSTATUS_ALLUNAVAIL, dmstatus.allunavail);
                result = set_field(result, DM_DMSTATUS_ALLRUNNING, dmstatus.allrunning);
                result = set_field(result, DM_DMSTATUS_ALLHALTED, dmstatus.allhalted);
                result = set_field(result, DM_DMSTATUS_ALLRESUMEACK, dmstatus.allresumeack);
                result = set_field(result, DM_DMSTATUS_ANYNONEXISTENT, dmstatus.anynonexistant);
                result = set_field(result, DM_DMSTATUS_ANYUNAVAIL, dmstatus.anyunavail);
                result = set_field(result, DM_DMSTATUS_ANYRUNNING, dmstatus.anyrunning);
                result = set_field(result, DM_DMSTATUS_ANYHALTED, dmstatus.anyhalted);
                result = set_field(result, DM_DMSTATUS_ANYRESUMEACK, dmstatus.anyresumeack);
                result = set_field(result, DM_DMSTATUS_AUTHENTICATED, dmstatus.authenticated);
                result = set_field(result, DM_DMSTATUS_AUTHBUSY, dmstatus.authbusy);
                result = set_field(result, DM_DMSTATUS_VERSION, dmstatus.version);
            }
                break;
            case DM_ABSTRACTCS:
                result = set_field(result, DM_ABSTRACTCS_CMDERR, abstractcs.cmderr);
                result = set_field(result, DM_ABSTRACTCS_BUSY, abstractcs.busy);
                result = set_field(result, DM_ABSTRACTCS_DATACOUNT, abstractcs.datacount);
                result = set_field(result, DM_ABSTRACTCS_PROGBUFSIZE,
                                   abstractcs.progbufsize);
                break;
            case DM_ABSTRACTAUTO:
                result = set_field(result, DM_ABSTRACTAUTO_AUTOEXECPROGBUF, abstractauto.autoexecprogbuf);
                result = set_field(result, DM_ABSTRACTAUTO_AUTOEXECDATA, abstractauto.autoexecdata);
                break;
            case DM_COMMAND:
                result = 0;
                break;
            case DM_HARTINFO:
                result = set_field(result, DM_HARTINFO_NSCRATCH, 1);
                result = set_field(result, DM_HARTINFO_DATAACCESS, 1);
                result = set_field(result, DM_HARTINFO_DATASIZE, abstractcs.datacount);
                result = set_field(result, DM_HARTINFO_DATAADDR, debug_data_start);
                break;
            case DM_HAWINDOWSEL:
                result = hawindowsel;
                break;
            case DM_HAWINDOW:
            {
                unsigned base = hawindowsel * 32;
                for (unsigned i = 0; i < 32; i++) {
                    unsigned n = base + i;
                    if (n < 1) {
                        result |= 1 << i;
                    }
                }
            }
                break;
            case DM_SBCS:
                result = set_field(result, DM_SBCS_SBVERSION, sbcs.version);
                result = set_field(result, DM_SBCS_SBREADONADDR, sbcs.readonaddr);
                result = set_field(result, DM_SBCS_SBACCESS, sbcs.sbaccess);
                result = set_field(result, DM_SBCS_SBAUTOINCREMENT, sbcs.autoincrement);
                result = set_field(result, DM_SBCS_SBREADONDATA, sbcs.readondata);
                result = set_field(result, DM_SBCS_SBERROR, sbcs.error);
                result = set_field(result, DM_SBCS_SBASIZE, sbcs.asize);
                result = set_field(result, DM_SBCS_SBACCESS128, sbcs.access128);
                result = set_field(result, DM_SBCS_SBACCESS64, sbcs.access64);
                result = set_field(result, DM_SBCS_SBACCESS32, sbcs.access32);
                result = set_field(result, DM_SBCS_SBACCESS16, sbcs.access16);
                result = set_field(result, DM_SBCS_SBACCESS8, sbcs.access8);
                break;
            case DM_SBADDRESS0:
                result = sbaddress[0];
                break;
            case DM_SBADDRESS1:
                result = sbaddress[1];
                break;
            case DM_SBADDRESS2:
                result = sbaddress[2];
                break;
            case DM_SBADDRESS3:
                result = sbaddress[3];
                break;
            case DM_SBDATA0:
                result = sbdata[0];
                if (sbcs.error == 0) {
                    if (sbcs.readondata) {
                        sb_read();
                    }
                    if (sbcs.error == 0) {
                        sb_autoincrement();
                    }
                }
                break;
            case DM_SBDATA1:
                result = sbdata[1];
                break;
            case DM_SBDATA2:
                result = sbdata[2];
                break;
            case DM_SBDATA3:
                result = sbdata[3];
                break;
            case DM_AUTHDATA:
                result = challenge;
                break;
            case DM_DMCS2:
                result = set_field(result, DM_DMCS2_GROUP,
                                   hart_state.haltgroup);
                break;
            default:
                result = 0;
                D(fprintf(stderr, "Unexpected. Returning Error."));
                return false;
        }
    }
    D(fprintf(stderr, "0x%x\n", result));
    *value = result;
    return true;
}

void debug_module_t::run_test_idle()
{
    if (rti_remaining > 0) {
        rti_remaining--;
    }
    if (rti_remaining == 0 && abstractcs.busy && abstract_command_completed) {
        abstractcs.busy = false;
    }
}

bool debug_module_t::perform_abstract_command()
{
    if (abstractcs.cmderr != CMDERR_NONE)
        return true;
    if (abstractcs.busy) {
        abstractcs.cmderr = CMDERR_BUSY;
        return true;
    }

    if ((command >> 24) != 0) {
        abstractcs.cmderr = CMDERR_NOTSUP;
        return true;
    }

    if (!hart_state.halted) {
        abstractcs.cmderr = CMDERR_HALTRESUME;
        return true;
    }

    // register access
    unsigned size = get_field(command, AC_ACCESS_REGISTER_AARSIZE);
    bool write = get_field(command, AC_ACCESS_REGISTER_WRITE);
    unsigned regno = get_field(command, AC_ACCESS_REGISTER_REGNO);

    if (size != 2) {
        abstractcs.cmderr = CMDERR_NOTSUP;
        return true;
    }

    if (command == 0x271009) {
        command += 1;
        command -= 1;
    }

    unsigned i = 0;
    if (get_field(command, AC_ACCESS_REGISTER_TRANSFER)) {
        if (regno < 0x1000 && config.support_abstract_csr_access) {
            write32(debug_abstract, i++, csrw(REG_S0, CSR_DSCRATCH0));
            if (write) {
                write32(debug_abstract, i++, lw(REG_S0, REG_ZERO, debug_data_start));
                write32(debug_abstract, i++, csrw(REG_S0, regno));
            } else {
                write32(debug_abstract, i++, csrr(REG_S0, regno));
                write32(debug_abstract, i++, sw(REG_S0, REG_ZERO, debug_data_start));
            }
            write32(debug_abstract, i++, csrr(REG_S0, CSR_DSCRATCH0));
        } else if (regno >= 0x1000 && regno < 0x1020) {
            unsigned regnum = regno - 0x1000;

            if (write)
                write32(debug_abstract, i++, lw(regnum, REG_ZERO, debug_data_start));
            else
                write32(debug_abstract, i++, sw(regnum, REG_ZERO, debug_data_start));

            if (regno == 0x1000 + REG_S0 && write) {
                /*
                 * The exception handler starts out be restoring dscratch to s0,
                 * which was saved before executing the abstract memory region. Since
                 * we just wrote s0, also make sure to write that same value to
                 * dscratch in case an exception occurs in a program buffer that
                 * might be executed later.
                 */
                write32(debug_abstract, i++, csrw(REG_S0, CSR_DSCRATCH0));
            }
        } else if (regno >= 0xc000 && (regno & 1) == 1) {
            // Support odd-numbered custom registers, to allow for debugger testing.
            unsigned custom_number = regno - 0xc000;
            abstractcs.cmderr = CMDERR_NONE;
            if (write) {
                // Writing V to custom register N will cause future reads of N to
                // return V, reads of N-1 will return V-1, etc.
                custom_base = read32(dmdata, 0) - custom_number;
            } else {
                write32(dmdata, 0, custom_number + custom_base);
                write32(dmdata, 1, 0);
            }
            return true;

        } else {
            abstractcs.cmderr = CMDERR_NOTSUP;
            return true;
        }
    }

    if (get_field(command, AC_ACCESS_REGISTER_POSTEXEC)) {
        write32(debug_abstract, i,
                jal(REG_ZERO, debug_progbuf_start - debug_abstract_start - 4 * i));
        i++;
    } else {
        write32(debug_abstract, i++, ebreak());
    }

    debug_rom_flags |= 1 << DEBUG_ROM_FLAG_GO;
    rti_remaining = config.abstract_rti;
    abstract_command_completed = false;

    abstractcs.busy = true;

    return true;
}

bool debug_module_t::dmi_write(unsigned address, uint32_t value)
{
    D(fprintf(stderr, "dmi_write(0x%x, 0x%x)\n", address, value));

    if (!dmstatus.authenticated && address != DM_AUTHDATA &&
        address != DM_DMCONTROL)
        return false;

    if (address >= DM_DATA0 && address < DM_DATA0 + abstractcs.datacount) {
        unsigned i = address - DM_DATA0;
        if (!abstractcs.busy)
            write32(dmdata, address - DM_DATA0, value);

        if (abstractcs.busy && abstractcs.cmderr == CMDERR_NONE) {
            abstractcs.cmderr = CMDERR_BUSY;
        }

        if (!abstractcs.busy && ((abstractauto.autoexecdata >> i) & 1)) {
            perform_abstract_command();
        }
        return true;

    } else if (address >= DM_PROGBUF0 && address < DM_PROGBUF0 + config.progbufsize) {
        unsigned i = address - DM_PROGBUF0;

        if (!abstractcs.busy)
            write32(program_buffer, i, value);

        if (!abstractcs.busy && ((abstractauto.autoexecprogbuf >> i) & 1)) {
            perform_abstract_command();
        }
        return true;

    }

    switch (address) {
        case DM_DMCONTROL:
        {
            if (!dmcontrol.dmactive && get_field(value, DM_DMCONTROL_DMACTIVE))
                reset();
            dmcontrol.dmactive = get_field(value, DM_DMCONTROL_DMACTIVE);
            if (!dmstatus.authenticated || !dmcontrol.dmactive)
                return true;

            dmcontrol.haltreq = get_field(value, DM_DMCONTROL_HALTREQ);
            dmcontrol.resumereq = get_field(value, DM_DMCONTROL_RESUMEREQ);
            dmcontrol.hartreset = get_field(value, DM_DMCONTROL_HARTRESET);
            dmcontrol.ndmreset = get_field(value, DM_DMCONTROL_NDMRESET);
            if (config.support_hasel)
                dmcontrol.hasel = get_field(value, DM_DMCONTROL_HASEL);
            else
                dmcontrol.hasel = 0;
            dmcontrol.hartsel = get_field(value, DM_DMCONTROL_HARTSELHI) <<
                                                                         DM_DMCONTROL_HARTSELLO_LENGTH;
            dmcontrol.hartsel |= get_field(value, DM_DMCONTROL_HARTSELLO);
            dmcontrol.hartsel &= 1;

            if (get_field(value, DM_DMCONTROL_ACKHAVERESET)) {
                hart_state.havereset = false;
            }
            core.cpu.halt_request = dmcontrol.haltreq;
            if (dmcontrol.haltreq) {
                D(fprintf(stderr, "halt hart\n"));
                fflush(stderr);
            }
            if (dmcontrol.resumereq) {
                D(fprintf(stderr, "resume hart\n"));
                fflush(stderr);
                debug_rom_flags |= (1 << DEBUG_ROM_FLAG_RESUME);
                hart_state.resumeack = false;
            }
            if (dmcontrol.hartreset) {
                core.cpu.reset();
            }

            if (dmcontrol.ndmreset) {
                core.cpu.reset();
            }
        }
            return true;

        case DM_COMMAND: {
            command = value;
            bool ret = perform_abstract_command();
            fprintf(stderr, "abstract cmd 0x%x. Error: %d \n", command, abstractcs.cmderr);
            return ret;
        }

        case DM_HAWINDOWSEL:
            hawindowsel = value & 1;
            return true;

        case DM_HAWINDOW:
            return true;

        case DM_ABSTRACTCS:
            abstractcs.cmderr = (cmderr_t) (((uint32_t) (abstractcs.cmderr)) & (~(uint32_t)(get_field(value, DM_ABSTRACTCS_CMDERR))));
            return true;

        case DM_ABSTRACTAUTO:
            abstractauto.autoexecprogbuf = get_field(value,
                                                     DM_ABSTRACTAUTO_AUTOEXECPROGBUF);
            abstractauto.autoexecdata = get_field(value,
                                                  DM_ABSTRACTAUTO_AUTOEXECDATA);
            return true;
        case DM_SBCS:
            sbcs.readonaddr = get_field(value, DM_SBCS_SBREADONADDR);
            sbcs.sbaccess = get_field(value, DM_SBCS_SBACCESS);
            sbcs.autoincrement = get_field(value, DM_SBCS_SBAUTOINCREMENT);
            sbcs.readondata = get_field(value, DM_SBCS_SBREADONDATA);
            sbcs.error &= ~get_field(value, DM_SBCS_SBERROR);
            return true;
        case DM_SBADDRESS0:
            sbaddress[0] = value;
            if (sbcs.error == 0 && sbcs.readonaddr) {
                sb_read();
                sb_autoincrement();
            }
            return true;
        case DM_SBADDRESS1:
            sbaddress[1] = value;
            return true;
        case DM_SBADDRESS2:
            sbaddress[2] = value;
            return true;
        case DM_SBADDRESS3:
            sbaddress[3] = value;
            return true;
        case DM_SBDATA0:
            sbdata[0] = value;
            if (sbcs.error == 0) {
                sb_write();
                if (sbcs.error == 0) {
                    sb_autoincrement();
                }
            }
            return true;
        case DM_SBDATA1:
            sbdata[1] = value;
            return true;
        case DM_SBDATA2:
            sbdata[2] = value;
            return true;
        case DM_SBDATA3:
            sbdata[3] = value;
            return true;
        case DM_AUTHDATA:
            D(fprintf(stderr, "debug authentication: got 0x%x; 0x%x unlocks\n", value,
                      challenge + secret));
            if (config.require_authentication) {
                if (value == challenge + secret) {
                    dmstatus.authenticated = true;
                } else {
                    dmstatus.authenticated = false;
                    challenge = rand();
                }
            }
            return true;
        case DM_DMCS2:
            if (config.support_haltgroups && get_field(value, DM_DMCS2_HGWRITE)) {
                hart_state.haltgroup = get_field(value, DM_DMCS2_GROUP);
            }
            return true;
    }
    return false;
}

void debug_module_t::proc_reset(unsigned id)
{
    hart_state.havereset = true;
    hart_state.halted = false;
    hart_state.haltgroup = 0;
}

uint32_t debug_module_t::get_start_addr() {
    return DEBUG_START;
}

uint32_t debug_module_t::get_end_addr() {
    return DEBUG_END;
}

void debug_module_t::tick() {
    remoteBitbang.tick();
}
