#include <bkp.h>
#include <cstring>

BKP_module::BKP_module() :Device(), regs() {

}

uint32_t BKP_module::get_start_addr() {
    return MDR_BKP_BASE;
}

uint32_t BKP_module::get_end_addr() {
    return MDR_BKP_BASE + sizeof(MDR_BKP_TypeDef);
}

uint32_t BKP_module::get_priority() {
    return 1;
}

void BKP_module::reset() {
    memset(&regs, 0, sizeof(regs));
    regs.CLK = BKP_CLK_LSEON | BKP_CLK_LSERDY | BKP_CLK_LSION | BKP_CLK_LSIRDY;
}

uint32_t BKP_module::read(uint32_t addr, uint32_t len) noexcept(false) {
    return read_from_mem(addr, len, &regs);
}

void BKP_module::write(uint32_t addr, uint32_t value, uint32_t len) noexcept(false) {
    write_to_mem(addr, value, len, &regs);
}
