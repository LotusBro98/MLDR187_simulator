#ifndef MLDR187_SIMULATOR_MEMORY_H
#define MLDR187_SIMULATOR_MEMORY_H

#include <vector>
#include <cstddef>
#include <cstdint>
#include <string>
#include <stdexcept>
#include "Device.h"

class Memory : public Device {
public:
    Memory(uint32_t base, uint32_t size);
    Memory(uint32_t base, uint32_t size, uint8_t *buf, uint8_t reset_value);
    ~Memory();

    uint32_t read(uint32_t addr, uint32_t len) noexcept(false) override;
    void write(uint32_t addr, uint32_t value, uint32_t len) noexcept(false) override;

    uint32_t get_start_addr() override;
    uint32_t get_end_addr() override;

    void init() override;
    void reset() override;

private:
    uint32_t base;
    uint32_t size;
    uint8_t * data;
    bool allocated;
    uint8_t reset_value;
};

#endif //MLDR187_SIMULATOR_MEMORY_H
