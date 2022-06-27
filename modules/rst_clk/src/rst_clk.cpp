#include <cstring>
#include <cstdio>
#include "rst_clk.h"

uint32_t RST_CLK_module::get_start_addr() {
    return MDR_RST_CLK_BASE;
}

uint32_t RST_CLK_module::get_end_addr() {
    return MDR_RST_CLK_BASE + sizeof(MDR_RST_CLK_TypeDef);
}

void RST_CLK_module::reset() {
    memset(&regs, 0, sizeof(regs));
    regs.CLOCK_STATUS = RST_CLK_CLOCK_STATUS_PLLRDY | RST_CLK_CLOCK_STATUS_HSERDY;
}

uint32_t RST_CLK_module::read(uint32_t addr, uint32_t len) noexcept(false) {
    return read_from_mem(addr, len, &regs);
}

void RST_CLK_module::write(uint32_t addr, uint32_t value, uint32_t len) noexcept(false) {
    write_to_mem(addr, value, len, &regs);
}

RST_CLK_module::RST_CLK_module() : Device(), regs{} {
}

uint32_t RST_CLK_module::get_priority() {
    return 1;
}
