#include <cstdint>
#include <vector>
#include "MemoryStorage.h"
#include <memory.h>
#include "CPU.h"
#include "Exception.h"
#include "Core.h"

Memory::Memory(uint32_t base, uint32_t size): Device(), base(base), size(size), allocated(true) {
    data = new uint8_t[size];
    reset_value = 0;
}

Memory::Memory(uint32_t base, uint32_t size, uint8_t *buf, uint8_t reset_value, bool clear_on_reset):
    Device(), base(base), size(size), data(buf), allocated(false), reset_value(reset_value), clear_on_reset(clear_on_reset) {

}

Memory::~Memory() {
    if (allocated)
        delete[] data;
}

void Memory::reset() {
    if (clear_on_reset)
        memset(data, reset_value, size);
}

void Memory::init() {
}

uint32_t Memory::read(uint32_t addr, uint32_t len) noexcept(false) {
    switch (len) {
        case 1:
            return *(uint8_t*)(data + addr);
        case 2:
            return *(uint16_t*)(data + addr);
        case 4:
            return *(uint32_t*)(data + addr);
        default:
            throw EXC_LAF;
    }
}

void Memory::write(uint32_t addr, uint32_t value, uint32_t len) noexcept(false) {
    switch (len) {
        case 1:
            *(uint8_t*)(data + addr) = (uint8_t)value;
            break;
        case 2:
            *(uint16_t*)(data + addr) = (uint16_t)value;
            break;
        case 4:
            *(uint32_t*)(data + addr) = (uint32_t)value;
            break;
        default:
            throw EXC_SAF;
    }
}

uint32_t Memory::get_start_addr() {
    return base;
}

uint32_t Memory::get_end_addr() {
    return base + size;
}
