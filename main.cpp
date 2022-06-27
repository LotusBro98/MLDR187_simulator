#include <cstring>
#include "Core.h"
#include "modules/bkp/include/bkp.h"
#include "modules/debug/include/debug_module.h"
#include "modules/debug/include/jtag_dtm.h"
#include "modules/debug/include/remote_bitbang.h"
#include "modules/memory/include/MemoryStorage.h"
#include "modules/plic/include/plic.h"
#include "modules/rst_clk/include/rst_clk.h"
#include "modules/spi/include/JEDEC_SPI_Flash.h"
#include "modules/gpio/include/GPIO.h"
#include "modules/flash/include/flash.h"

Core core;

#define FLASH_MAIN_BANK_SIZE 0x40000
#define FLASH_INFO_BANK_SIZE 0x8000

int main(int argc, char* argv[]) {

    BKP_module BKP;
    PLIC_module PLIC;
    RST_CLK_module RST_CLK;

    uint8_t * FLASH_MAIN_BANK = new uint8_t[FLASH_MAIN_BANK_SIZE];
    uint8_t * FLASH_INFO_BANK = new uint8_t[FLASH_INFO_BANK_SIZE];

    Memory MEM_FLASH    (0x10000000, 0x40000, FLASH_MAIN_BANK, 0xff);
    Memory MEM_RAM_TCMA (0x80000000, 0x10000);
    Memory MEM_RAM_TCMB (0x80010000, 0x8000);
    Memory MEM_RAM_AHB  (0x20000000, 0x4000);
    Memory MEM_PER_AHB  (0x40000000, 0x100000);

    debug_module_t debugModule(debug_module_config_t{15, 32, false, 5, false, true, false, false});

    GPIO_module PORTA(MDR_GPIO1_BASE);
    GPIO_module PORTB(MDR_GPIO2_BASE);
    GPIO_module PORTC(MDR_GPIO3_BASE);
    GPIO_module PORTD(MDR_GPIO4_BASE);

    JEDEC_SPI_Flash SPI_FLASH(MDR_SPI1_BASE, 0x200000);
    PORTA.set_callback(12, SPI_FLASH.get_cs_pin_cb());

    SPI_module SPI_DISPLAY(MDR_SPI3_BASE);

    Flash_module INTERNAL_FLASH(FLASH_MAIN_BANK, FLASH_INFO_BANK);

    core.init();
    core.reset();

    while (true)
    {
        core.step();
    }

    return 0;
}
