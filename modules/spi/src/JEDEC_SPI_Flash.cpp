#include <cstring>
#include <fstream>
#include "JEDEC_SPI_Flash.h"

typedef enum
{
    FLASH_READ_DATA_SLOW = 0x03,		/*!< Read data with 33+ MHz speed */
    FLASH_READ_DATA_FAST = 0x0B,		/*!< Read data with 50+ MHz speed */
    FLASH_ERASE_SECTOR_4K = 0x20,		/*!< Erase 4KB sector */
    FLASH_ERASE_BLOCK_32K = 0x52,		/*!< Erase 32KB block */
    FLASH_ERASE_BLOCK_64K = 0xD8,		/*!< Erase 64KB block */
    FLASH_ERASE_CHIP = 0x60,			/*!< Erase whole chip */
    FLASH_PROGRAM_PAGE = 0x02,			/*!< Program 1..256 bytes */
    FLASH_WRITE_ENABLE = 0x06,			/*!< Sets Write Enable Latch */
    FLASH_WRITE_DISABLE = 0x04,			/*!< Resets Write Enable Latch */
    FLASH_READ_STATUS_REGISTER = 0x05,	/*!< Reads status register */
    FLASH_WRITE_STATUS_REGISTER = 0x01,	/*!< Writes status register */
    FLASH_READ_ID = 0x9F,				/*!< Read JEDEC ID */
} flash_command_e;

JEDEC_SPI_Flash::JEDEC_SPI_Flash(uint32_t baseAddr, uint32_t size, const char* save_file) :
    SPI_module(baseAddr),
    size(size),
    save_file(save_file)
{
    data = new uint8_t[size];
    memset(data, 0xff, size);
    std::ifstream save_file_fstream(save_file, std::ios_base::in | std::ios_base::binary);
    save_file_fstream.read((char*)data, size);
    save_file_fstream.close();
}

JEDEC_SPI_Flash::~JEDEC_SPI_Flash() {
    std::ofstream info_bank_fstream(save_file, std::ios_base::out | std::ios_base::binary);
    info_bank_fstream.write((char*)data, size);
    info_bank_fstream.close();
    delete[] data;
}

uint8_t JEDEC_SPI_Flash::exchange_bytes(uint8_t byte) {
    if (cmd_offset == 0) {
        cmd = byte;
        addr = 0;
        cmd_offset++;
        return 0;
    } else if (cmd_offset < 4) {
        addr |= (uint32_t)byte << ((2 - (cmd_offset-1)) << 3);
        cmd_offset++;
        return 0;
    } else {
        return exchange_data(cmd, addr, cmd_offset++ - 4, byte);
    }
}

void JEDEC_SPI_Flash::on_cs_fall() {
    cmd_offset = 0;
    cmd = 0;
    addr = 0;
}

void JEDEC_SPI_Flash::on_cs_rise() {
    if (cmd == FLASH_ERASE_SECTOR_4K)
        erase_page(addr, 0x1000);

    cmd_offset = 0;
    cmd = 0;
    addr = 0;
}

uint8_t JEDEC_SPI_Flash::exchange_data(uint8_t cmd, uint32_t addr, uint32_t i, uint8_t byte) {
    switch (cmd) {
        case FLASH_READ_DATA_SLOW:
            return addr + i < size ? data[addr + i] : 0;
        case FLASH_READ_STATUS_REGISTER:
            return status_register;
        case FLASH_PROGRAM_PAGE:
            if (addr + i < size)
                data[addr + i] = byte;
            return 0;
        default:
            return 0;
    }
}

void JEDEC_SPI_Flash::erase_page(uint32_t addr, uint32_t page_size) {
    uint32_t start = addr % page_size;
    if (start + page_size > size)
        return;
    memset(data, 0xff, page_size);
}
