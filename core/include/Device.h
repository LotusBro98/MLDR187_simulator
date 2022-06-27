#ifndef MLDR187_SIMULATOR_DEVICE_H
#define MLDR187_SIMULATOR_DEVICE_H

#include <cstdint>

#define DEVICE_MAX_PRIORITY 2

class Device {
public:
    Device();

    virtual uint32_t read(uint32_t addr, uint32_t len) noexcept(false) = 0;
    virtual void write(uint32_t addr, uint32_t value, uint32_t len) noexcept(false) = 0;

    virtual uint32_t get_start_addr() = 0;
    virtual uint32_t get_end_addr() = 0;
    virtual uint32_t get_priority();

    virtual void init();
    virtual void tick();
    virtual void reset();

    inline uint8_t  read_byte(uint32_t addr) noexcept(false) { return read(addr, 1); }
    inline uint16_t read_half(uint32_t addr) noexcept(false) { return read(addr, 2); }
    inline uint32_t read_word(uint32_t addr) noexcept(false) { return read(addr, 4); }

    inline void write_byte(uint32_t addr, uint8_t  value) noexcept(false) { write(addr, value, 1); }
    inline void write_half(uint32_t addr, uint16_t value) noexcept(false) { write(addr, value, 2); }
    inline void write_word(uint32_t addr, uint32_t value) noexcept(false) { write(addr, value, 4); }

    bool is_within(uint32_t addr);

protected:
    static uint32_t read_from_mem(uint32_t addr, uint32_t len, void* buf);
    static void write_to_mem(uint32_t addr, uint32_t value, uint32_t len, void* buf);
};

#endif //MLDR187_SIMULATOR_DEVICE_H
