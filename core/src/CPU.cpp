#include <CPU.h>
#include <memory.h>
#include <Config.h>
#include <chrono>
#include <cstdio>
#include "Exception.h"
#include "Core.h"
#include "bits.h"

#define FIELD(x, base, size) (((x) >> (base)) & ~(((uint32_t)-1) << (size)))
#define BIT(x, n) (((x) >> (n)) & 1)
#define SIGN_BIT(b, n) (((uint32_t)-(b)) << (n))

extern void (CPU::*const execute_op[Instruction::COUNT])(Instruction& inst);

CPU::CPU():
execute_op  {
    &CPU::execute_ILL,
    &CPU::execute_NOP,
    &CPU::execute_LUI,
    &CPU::execute_AUIPC,
    &CPU::execute_JAL,
    &CPU::execute_JALR,
    &CPU::execute_BEQ,
    &CPU::execute_BNE,
    &CPU::execute_BLT,
    &CPU::execute_BGE,
    &CPU::execute_BLTU,
    &CPU::execute_BGEU,
    &CPU::execute_LB,
    &CPU::execute_LH,
    &CPU::execute_LW,
    &CPU::execute_LBU,
    &CPU::execute_LHU,
    &CPU::execute_SB,
    &CPU::execute_SH,
    &CPU::execute_SW,
    &CPU::execute_ADDI,
    &CPU::execute_SLTI,
    &CPU::execute_SLTIU,
    &CPU::execute_XORI,
    &CPU::execute_ORI,
    &CPU::execute_ANDI,
    &CPU::execute_SLLI,
    &CPU::execute_SRLI,
    &CPU::execute_SRAI,
    &CPU::execute_ADD,
    &CPU::execute_SUB,
    &CPU::execute_SLL,
    &CPU::execute_SLT,
    &CPU::execute_SLTU,
    &CPU::execute_XOR,
    &CPU::execute_SRL,
    &CPU::execute_SRA,
    &CPU::execute_OR,
    &CPU::execute_AND,
    &CPU::execute_ECALL,
    &CPU::execute_EBREAK,
    &CPU::execute_MRET,
    &CPU::execute_DRET,
    &CPU::execute_CSRRW,
    &CPU::execute_CSRRS,
    &CPU::execute_CSRRC,
    &CPU::execute_CSRRWI,
    &CPU::execute_CSRRSI,
    &CPU::execute_CSRRCI,
    &CPU::execute_MUL,
    &CPU::execute_MULH,
    &CPU::execute_MULHSU,
    &CPU::execute_MULHU,
    &CPU::execute_DIV,
    &CPU::execute_DIVU,
    &CPU::execute_REM,
    &CPU::execute_REMU,
}
{}

uint32_t cnt = 0;
uint32_t delta_time = 0;
uint32_t IPS = 0;
auto start = std::chrono::high_resolution_clock::now();

static void measureIPS() {
    cnt++;

    if ((cnt & 0xffff) != 0)
        return;

    auto elapsed = std::chrono::high_resolution_clock::now() - start;
    start = std::chrono::high_resolution_clock::now();
    delta_time += std::chrono::duration_cast<std::chrono::microseconds>(elapsed).count();

    if (delta_time > 1000000) {
        IPS = (uint64_t)cnt * 1000000 / delta_time;
        delta_time = 0;
        cnt = 0;
        fprintf(stdout, "IPS = %d\n", IPS);
        fflush(stdout);
    }
}

void CPU::step() {
    if (!debug_mode && halt_request) {
        enter_debug_mode(DCSR_CAUSE_DEBUGINT);
    }

    Instruction inst = {};
    try {
        uint32_t raw = fetch();
        inst.decode(raw);
        execute(inst);
    } catch (Exception& e) {
        handle_exception(e);
    }

    if (single_step == 1) {
        enter_debug_mode(DCSR_CAUSE_STEP);
        single_step = 0;
    } else if (single_step > 1) {
        single_step--;
    }

    measureIPS();
}

void CPU::execute(Instruction& inst) {
    auto executor = execute_op[inst.code];
    regs[0] = 0;
    (*this.*executor)(inst);
    regs[0] = 0;
    pc += 4 >> inst.is_c_ext;
}

void CPU::handle_exception(Exception& e) {
    // TODO more actions
    csr[0x341] = pc; // mepc
    csr[0x342] = e.code;  // mcause
    if (debug_mode) {
        pc = DEBUG_ROM_TVEC;
    } else {
        pc = csr[0x305]; // mtvec
    }
}

void CPU::reset() {
    halt_request = debug_mode;
    debug_mode = false;
    single_step = 0;
    memset(regs, 0, sizeof(regs));
    memset(csr, 0, sizeof(csr));
    privilege = PRV_M;
    pc = START_PC;
}

uint32_t CPU::fetch() {
    uint32_t inst = core.bus.read_half(pc);
    if ((inst & 0x3) == 0x3) {
        /* if not compressed */
        inst |= core.bus.read_half(pc + 2) << 16;
    }
    return inst;
}

void CPU::execute_LUI(Instruction &inst) {
    regs[inst.rd] = inst.immediate;
}

void CPU::execute_ILL(Instruction &inst) {
    throw EXC_II;
}

void CPU::execute_AUIPC(Instruction &inst) {
    regs[inst.rd] = inst.immediate + pc;
}

void CPU::execute_JAL(Instruction &inst) {
    regs[inst.rd] = pc + (4 >> inst.is_c_ext);
    pc += inst.immediate - (4 >> inst.is_c_ext);
}

void CPU::execute_JALR(Instruction &inst) {
    uint32_t tmp = regs[inst.rs1];
    regs[inst.rd] = pc + (4 >> inst.is_c_ext);
    pc = tmp + inst.immediate - (4 >> inst.is_c_ext);
}

void CPU::execute_BNE(Instruction &inst) {
    if (regs[inst.rs1] != regs[inst.rs2])
        pc += inst.immediate - (4 >> inst.is_c_ext);
}

void CPU::execute_BEQ(Instruction &inst) {
    if (regs[inst.rs1] == regs[inst.rs2])
        pc += inst.immediate - (4 >> inst.is_c_ext);
}

void CPU::execute_BLT(Instruction &inst) {
    if ((int32_t)regs[inst.rs1] < (int32_t)regs[inst.rs2])
        pc += inst.immediate - (4 >> inst.is_c_ext);
}

void CPU::execute_BGE(Instruction &inst) {
    if ((int32_t)regs[inst.rs1] >= (int32_t)regs[inst.rs2])
        pc += inst.immediate - (4 >> inst.is_c_ext);
}

void CPU::execute_BLTU(Instruction &inst) {
    if (regs[inst.rs1] < regs[inst.rs2])
        pc += inst.immediate - (4 >> inst.is_c_ext);
}

void CPU::execute_BGEU(Instruction &inst) {
    if (regs[inst.rs1] >= regs[inst.rs2])
        pc += inst.immediate - (4 >> inst.is_c_ext);
}

void CPU::execute_LB(Instruction &inst) {
    uint32_t byte = core.bus.read_byte(regs[inst.rs1] + inst.immediate);
    regs[inst.rd] = byte | SIGN_BIT(BIT(byte, 7), 7);
}

void CPU::execute_LH(Instruction &inst) {
    uint32_t half = core.bus.read_half(regs[inst.rs1] + inst.immediate);
    regs[inst.rd] = half | SIGN_BIT(BIT(half, 15), 15);
}

void CPU::execute_LW(Instruction &inst) {
    regs[inst.rd] = core.bus.read_word(regs[inst.rs1] + inst.immediate);
}

void CPU::execute_LBU(Instruction &inst) {
    regs[inst.rd] = core.bus.read_byte(regs[inst.rs1] + inst.immediate);
}

void CPU::execute_LHU(Instruction &inst) {
    regs[inst.rd] = core.bus.read_half(regs[inst.rs1] + inst.immediate);
}

void CPU::execute_SB(Instruction &inst) {
    core.bus.write_byte(regs[inst.rs1] + inst.immediate, regs[inst.rs2]);
}

void CPU::execute_SH(Instruction &inst) {
    core.bus.write_half(regs[inst.rs1] + inst.immediate, regs[inst.rs2]);
}

void CPU::execute_SW(Instruction &inst) {
    core.bus.write_word(regs[inst.rs1] + inst.immediate, regs[inst.rs2]);
}

void CPU::execute_ADDI(Instruction &inst) {
    regs[inst.rd] = regs[inst.rs1] + inst.immediate;
}

void CPU::execute_SLTI(Instruction &inst) {
    regs[inst.rd] = (int32_t)regs[inst.rs1] < (int32_t)inst.immediate;
}

void CPU::execute_SLTIU(Instruction &inst) {
    regs[inst.rd] = regs[inst.rs1] < inst.immediate;
}

void CPU::execute_XORI(Instruction &inst) {
    regs[inst.rd] = regs[inst.rs1] ^ inst.immediate;
}

void CPU::execute_ORI(Instruction &inst) {
    regs[inst.rd] = regs[inst.rs1] | inst.immediate;
}

void CPU::execute_ANDI(Instruction &inst) {
    regs[inst.rd] = regs[inst.rs1] & inst.immediate;
}

void CPU::execute_SLLI(Instruction &inst) {
    regs[inst.rd] = regs[inst.rs1] << inst.immediate;
}

void CPU::execute_SRLI(Instruction &inst) {
    regs[inst.rd] = regs[inst.rs1] >> inst.immediate;
}

void CPU::execute_SRAI(Instruction &inst) {
    regs[inst.rd] = ((int32_t)regs[inst.rs1]) >> inst.immediate;
}

void CPU::execute_ADD(Instruction &inst) {
    regs[inst.rd] = regs[inst.rs1] + regs[inst.rs2];
}

void CPU::execute_SUB(Instruction &inst) {
    regs[inst.rd] = regs[inst.rs1] - regs[inst.rs2];
}

void CPU::execute_SLL(Instruction &inst) {
    regs[inst.rd] = regs[inst.rs1] << (regs[inst.rs2] & 32);
}

void CPU::execute_SLT(Instruction &inst) {
    regs[inst.rd] = (int32_t)regs[inst.rs1] < (int32_t)regs[inst.rs2];
}

void CPU::execute_SLTU(Instruction &inst) {
    regs[inst.rd] = regs[inst.rs1] < regs[inst.rs2];
}

void CPU::execute_XOR(Instruction &inst) {
    regs[inst.rd] = regs[inst.rs1] ^ regs[inst.rs2];
}

void CPU::execute_SRL(Instruction &inst) {
    regs[inst.rd] = regs[inst.rs1] >> (regs[inst.rs2] & 32);
}

void CPU::execute_SRA(Instruction &inst) {
    regs[inst.rd] = ((int32_t)regs[inst.rs1]) >> (regs[inst.rs2] & 32);
}

void CPU::execute_OR(Instruction &inst) {
    regs[inst.rd] = regs[inst.rs1] | regs[inst.rs2];
}

void CPU::execute_AND(Instruction &inst) {
    regs[inst.rd] = regs[inst.rs1] & regs[inst.rs2];
}

void CPU::execute_MUL(Instruction &inst) {
    regs[inst.rd] = regs[inst.rs1] * regs[inst.rs2];
}

void CPU::execute_MULH(Instruction &inst) {
    regs[inst.rd] = ((int64_t)regs[inst.rs1] * (int64_t)regs[inst.rs2]) >> 32;
}

void CPU::execute_MULHSU(Instruction &inst) {
    regs[inst.rd] = ((int64_t)regs[inst.rs1] * (uint64_t)regs[inst.rs2]) >> 32;
}

void CPU::execute_MULHU(Instruction &inst) {
    regs[inst.rd] = ((uint64_t)regs[inst.rs1] * (uint64_t)regs[inst.rs2]) >> 32;
}

void CPU::execute_DIV(Instruction &inst) {
    int32_t rs1 = (int32_t)regs[inst.rs1];
    int32_t rs2 = (int32_t)regs[inst.rs2];
    if (rs2 == 0) {
        regs[inst.rd] = -1;
    } else if (rs1 == INT32_MIN && rs2 == -1) {
        regs[inst.rd] = rs1;
    } else {
        regs[inst.rd] = rs1 / rs2;
    }
}

void CPU::execute_DIVU(Instruction &inst) {
    uint32_t rs1 = regs[inst.rs1];
    uint32_t rs2 = regs[inst.rs2];
    if (rs2 == 0) {
        regs[inst.rd] = -1;
    } else {
        regs[inst.rd] = rs1 / rs2;
    }
}

void CPU::execute_REM(Instruction &inst) {
    int32_t rs1 = (int32_t)regs[inst.rs1];
    int32_t rs2 = (int32_t)regs[inst.rs2];
    if (rs2 == 0) {
        regs[inst.rd] = rs1;
    } else if (rs1 == INT32_MIN && rs2 == -1) {
        regs[inst.rd] = 0;
    } else {
        regs[inst.rd] = rs1 % rs2;
    }
}

void CPU::execute_REMU(Instruction &inst) {
    uint32_t rs1 = regs[inst.rs1];
    uint32_t rs2 = regs[inst.rs2];
    if (rs2 == 0) {
        regs[inst.rd] = rs1;
    } else {
        regs[inst.rd] = rs1 % rs2;
    }
}

void CPU::execute_CSRRW(Instruction &inst) {
    uint32_t tmp = regs[inst.rs1];
    if (inst.rd != 0) {
        regs[inst.rd] = csr[inst.immediate];
    }
    csr[inst.immediate] = tmp;
}

void CPU::execute_CSRRS(Instruction &inst) {
    uint32_t tmp = regs[inst.rs1];
    regs[inst.rd] = csr[inst.immediate];
    csr[inst.immediate] |= tmp;
}

void CPU::execute_CSRRC(Instruction &inst) {
    uint32_t tmp = regs[inst.rs1];
    regs[inst.rd] = csr[inst.immediate];
    csr[inst.immediate] &= ~tmp;
}

void CPU::execute_CSRRWI(Instruction &inst) {
    regs[inst.rd] = csr[inst.immediate];
    csr[inst.immediate] = inst.rs1;
}

void CPU::execute_CSRRSI(Instruction &inst) {
    regs[inst.rd] = csr[inst.immediate];
    csr[inst.immediate] |= inst.rs1;
}

void CPU::execute_CSRRCI(Instruction &inst) {
    regs[inst.rd] = csr[inst.immediate];
    csr[inst.immediate] &= ~inst.rs1;
}

void CPU::execute_DRET(Instruction &inst) {
    if (!debug_mode)
        throw EXC_II;
    pc = csr[0x7b1] - (4 >> inst.is_c_ext); // pc = dpc
    privilege = csr[CSR_DCSR] & CSR_DCSR_PRV; // prv = dcsr->prv

    debug_mode = false;
    if (csr[CSR_DCSR] & CSR_DCSR_STEP) {
        single_step = 2;
    }
}

void CPU::execute_MRET(Instruction &inst) {
    // todo implement
    pc = csr[0x341] - (4 >> inst.is_c_ext); // pc = mepc
}

void CPU::execute_EBREAK(Instruction &inst) {
    enter_debug_mode(DCSR_CAUSE_SWBP);
    pc -= (4 >> inst.is_c_ext);
//    throw EXC_BREAK;
}

void CPU::execute_ECALL(Instruction &inst) {
    // TODO user mode
    throw EXC_ECALLM;
}

void CPU::enter_debug_mode(uint32_t cause) {
    if (debug_mode) {
        pc = DEBUG_ROM_ENTRY;
        return;
    }

    debug_mode = true;

    uint32_t * dcsr = &csr[CSR_DCSR];
    *dcsr &= ~(CSR_DCSR_PRV | CSR_DCSR_CAUSE);
    *dcsr |= (cause     & ((1 << CSR_DCSR_CAUSE_LENGTH) - 1)) << CSR_DCSR_CAUSE_OFFSET;
    *dcsr |= (privilege & ((1 << CSR_DCSR_PRV_LENGTH  ) - 1)) << CSR_DCSR_PRV_OFFSET;
    privilege = PRV_M;

    uint32_t * dpc  = &csr[0x7b1];
    *dpc = pc;
    pc = DEBUG_ROM_ENTRY;
}

void CPU::execute_NOP(Instruction &inst) {

}
