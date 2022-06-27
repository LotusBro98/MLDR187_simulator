#include <cstring>
#include "flash.h"

#define MAIN_BANK_SIZE 0x40000
#define INFO_BANK_SIZE 0x8000
#define PAGE_SIZE 0x1000

Flash_module::Flash_module(uint8_t* main_bank, uint8_t* info_bank):
    Device(), regs(), main_bank(main_bank), info_bank(info_bank)
{

}

uint32_t Flash_module::get_start_addr() {
    return MDR_EEPROM_CTRL_BASE;
}

uint32_t Flash_module::get_end_addr() {
    return MDR_EEPROM_CTRL_BASE + sizeof(regs);
}

uint32_t Flash_module::get_priority() {
    return 1;
}

void Flash_module::reset() {
    memset(&regs, 0, sizeof(regs));
}

uint32_t Flash_module::read(uint32_t addr, uint32_t len) noexcept(false) {
    return read_from_mem(addr, len, &regs);
}

void Flash_module::write(uint32_t addr, uint32_t value, uint32_t len) noexcept(false) {
    write_to_mem(addr, value, len, &regs);
    if (addr == offsetof(MDR_EEPROM_CTRL_TypeDef, CMD)) {
        if (regs.CMD & EEPROM_CMD_ERASE) {
            erase_page(regs.ADR, !(regs.CMD & EEPROM_CMD_IFREN));
        }
        if (regs.CMD & EEPROM_CMD_PROG) {
            program_word(regs.ADR, regs.DI, !(regs.CMD & EEPROM_CMD_IFREN));
        }
    }
}

void Flash_module::erase_page(uint32_t addr, bool main_bank_sel) {
    uint32_t size = main_bank_sel ? MAIN_BANK_SIZE : INFO_BANK_SIZE;
    uint8_t * buf = main_bank_sel ? main_bank : info_bank;

    addr &= size - 1;
    addr &= ~(PAGE_SIZE - 1);

    memset(buf + addr, 0xff, PAGE_SIZE);
}

void Flash_module::program_word(uint32_t addr, uint32_t word, bool main_bank_sel) {
    uint32_t size = main_bank_sel ? MAIN_BANK_SIZE : INFO_BANK_SIZE;
    uint8_t * buf = main_bank_sel ? main_bank : info_bank;

    addr &= size - 1;
    addr &= ~3;

    *(uint32_t*)(buf + addr) &= word;
}
