#ifndef MLDR187_SIMULATOR_JEDEC_SPI_FLASH_H
#define MLDR187_SIMULATOR_JEDEC_SPI_FLASH_H


#include "spi.h"

class JEDEC_SPI_Flash : public SPI_module {
public:
    JEDEC_SPI_Flash(uint32_t baseAddr, uint32_t size, const char* save_file);
    ~JEDEC_SPI_Flash() override;

private:
    uint32_t size;
    uint8_t * data;
    const char* save_file;

    uint32_t cmd_offset = 0;
    uint8_t cmd;
    uint32_t addr;

    uint8_t status_register = 0;

    uint8_t exchange_bytes(uint8_t byte) override;
    void on_cs_fall() override;
    void on_cs_rise() override;

    uint8_t exchange_data(uint8_t cmd, uint32_t addr, uint32_t i, uint8_t byte);
    void erase_page(uint32_t addr, uint32_t page_size);
};


#endif //MLDR187_SIMULATOR_JEDEC_SPI_FLASH_H
