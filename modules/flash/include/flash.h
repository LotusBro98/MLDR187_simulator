#ifndef MLDR187_SIMULATOR_FLASH_H
#define MLDR187_SIMULATOR_FLASH_H

#include "Device.h"
#include <MLDR187.h>

class Flash_module : Device {
public:
    explicit Flash_module(uint8_t* main_bank, uint8_t* info_bank);

    uint32_t read(uint32_t addr, uint32_t len) noexcept(false) override;
    void write(uint32_t addr, uint32_t value, uint32_t len) noexcept(false) override;

    uint32_t get_start_addr() override;
    uint32_t get_end_addr() override;

    uint32_t get_priority() override;

    void reset() override;

private:

    void erase_page(uint32_t addr, bool main_bank_sel);
    void program_word(uint32_t addr, uint32_t word, bool main_bank_sel);

    MDR_EEPROM_CTRL_TypeDef regs;
    uint8_t * main_bank;
    uint8_t * info_bank;
};


#endif //MLDR187_SIMULATOR_FLASH_H
