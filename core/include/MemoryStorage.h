#ifndef MLDR187_SIMULATOR_MEMORY_H
#define MLDR187_SIMULATOR_MEMORY_H

#include <vector>
#include <cstddef>
#include <cstdint>
#include <string>
#include <stdexcept>

class Memory {
public:
    Memory();
    ~Memory();

    void add_region(uint32_t base, uint32_t size);

    uint32_t read_word(uint32_t addr) noexcept(false);
    uint16_t read_half(uint32_t addr) noexcept(false);
    uint8_t read_byte(uint32_t addr) noexcept(false);
    void read(uint32_t addr, void* buf, uint32_t len) noexcept(false);

    void write_word(uint32_t addr, uint32_t word) noexcept(false);
    void write_half(uint32_t addr, uint16_t half) noexcept(false);
    void write_byte(uint32_t addr, uint8_t byte) noexcept(false);
    void write(uint32_t addr, void* buf, uint32_t len) noexcept(false);

    void reset();

private:
    struct Region {
        uint32_t base;
        uint32_t size;
        uint8_t * data;

        Region(uint32_t base, uint32_t size);
        ~Region();

        void allocate();
        bool is_within(uint32_t addr);
    };

    void* v2p(uint32_t virtual_address);

    std::vector<Region> regions;
};

#endif //MLDR187_SIMULATOR_MEMORY_H
