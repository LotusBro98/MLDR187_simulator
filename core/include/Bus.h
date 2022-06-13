#ifndef MLDR187_SIMULATOR_BUS_H
#define MLDR187_SIMULATOR_BUS_H

#include <cstdint>
#include <vector>
#include "Device.h"
#include "enums.h"

class Bus {
public:
    uint32_t read(uint32_t addr, uint32_t len) noexcept(false);
    void write(uint32_t addr, uint32_t value, uint32_t len) noexcept(false);

    inline uint8_t  read_byte(uint32_t addr) noexcept(false) { return read(addr, 1); }
    inline uint16_t read_half(uint32_t addr) noexcept(false) { return read(addr, 2); }
    inline uint32_t read_word(uint32_t addr) noexcept(false) { return read(addr, 4); }

    inline void write_byte(uint32_t addr, uint8_t  value) noexcept(false) { write(addr, value, 1); }
    inline void write_half(uint32_t addr, uint16_t value) noexcept(false) { write(addr, value, 2); }
    inline void write_word(uint32_t addr, uint32_t value) noexcept(false) { write(addr, value, 4); }

    void init();
    void tick();
    void reset();

    void add_device(Device * device);
private:
    Device * get_device(uint32_t addr);

    std::vector<Device*> devices;
};

#endif //MLDR187_SIMULATOR_BUS_H
