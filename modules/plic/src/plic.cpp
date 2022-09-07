#include <cstring>
#include "plic.h"
#include "macros.h"
#include "Exception.h"
#include "Core.h"

PLIC_module::PLIC_module() : Device()
{

}

uint32_t PLIC_module::get_start_addr() {
    return MDR_PLIC_PRI_BASE;
}

uint32_t PLIC_module::get_end_addr() {
    return MDR_PLIC_ICC + 4;
}

uint32_t PLIC_module::get_priority() {
    return 1;
}

void PLIC_module::reset() {
    memset(priority, 0, sizeof(priority));
    pending_mask = 0;
    enable_mask = 0;
    priority_threshold = 0;
}

void PLIC_module::tick() {
    pending_mask = core.pending_peripheral_interrupts;
    core.pending_peripheral_interrupts = 0;

    if (take_interrupt()) {
        core.cpu.raise_ext_interrupt();
    }
}


uint32_t PLIC_module::read(uint32_t addr, uint32_t len) noexcept(false) {
    assert_param(len == 4); // TODO non-4-byte read support
    addr = get_start_addr() + addr;

    switch (addr) {
        case MDR_PLIC_ICC:
            return take_interrupt();
        case MDR_PLIC_IEM:
            return enable_mask;
        case MDR_PLIC_IPM:
            return pending_mask;
        case MDR_PLIC_THR:
            return priority_threshold;
        default:
            if (!(addr >= MDR_PLIC_PRI_BASE && addr < MDR_PLIC_PRI_BASE + 4 * PLIC_N_INTERRUPTS))
                throw EXC_LAF;
            return read_from_mem(addr - MDR_PLIC_PRI_BASE, len, &priority);
    }
}

void PLIC_module::write(uint32_t addr, uint32_t value, uint32_t len) noexcept(false) {
    assert_param(len == 4); // TODO non-4-byte read support
    addr = get_start_addr() + addr;

    switch (addr) {
        case MDR_PLIC_ICC:
            release_interrupt(value - 1);
            break;
        case MDR_PLIC_IEM:
            enable_mask = value & ~1;
            break;
        case MDR_PLIC_IPM:
            throw EXC_SAF;
        case MDR_PLIC_THR:
            priority_threshold = value & 7;
            break;
        default:
            if (!(addr >= MDR_PLIC_PRI_BASE && addr < MDR_PLIC_PRI_BASE + 4 * PLIC_N_INTERRUPTS))
                throw EXC_LAF;
            write_to_mem(addr - MDR_PLIC_PRI_BASE, value & 7, len, &priority);
            break;
    }
}

uint32_t PLIC_module::take_interrupt() {
    uint32_t max_priority = priority_threshold;
    uint32_t max_irq = 0;

    for (int i = 0; i < PLIC_N_INTERRUPTS; i++) {
        if (priority[i] > max_priority && is_enabled(i) && is_pending(i))
            max_irq = i + 1;
    }

    return max_irq;
}

void PLIC_module::release_interrupt(uint32_t irqN) {
    set_pending(irqN, false);
}

bool PLIC_module::is_pending(uint32_t irqN) {
    return (pending_mask >> (irqN + 1)) & 1;
}

bool PLIC_module::is_enabled(uint32_t irqN) {
    return (enable_mask >> (irqN + 1)) & 1;
}

void PLIC_module::set_pending(uint32_t irqN, bool value) {
    pending_mask = pending_mask & ~(1 << (irqN + 1)) | (value & 1) << (irqN + 1);
}
