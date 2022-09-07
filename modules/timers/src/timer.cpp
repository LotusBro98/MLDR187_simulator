#include <cstring>
#include "timer.h"
#include "Core.h"

Timer::Timer(uint32_t base_addr, uint32_t irqN):
        base_addr(base_addr),
        irqN(irqN),
        regs{}
{

}

uint32_t Timer::get_start_addr() {
    return base_addr;
}

uint32_t Timer::get_end_addr() {
    return base_addr + sizeof(regs);
}

uint32_t Timer::get_priority() {
    return 1;
}

void Timer::reset() {
    memset(&regs, 0, sizeof(regs));
}

void Timer::tick() {
    // tick timer
    regs.CNT++;
    if (regs.CNT >= regs.ARR) {
        regs.STATUS |= TIM_CNTARREVENT;
        regs.CNT = 0;
    }

    if (regs.STATUS & regs.IE) {
        core.raise_peripheral_interrupt(irqN);
    }
}

uint32_t Timer::read(uint32_t addr, uint32_t len) noexcept(false) {
    return read_from_mem(addr, len, &regs);
}

void Timer::write(uint32_t addr, uint32_t value, uint32_t len) noexcept(false) {
    write_to_mem(addr, value, len, &regs);
}
