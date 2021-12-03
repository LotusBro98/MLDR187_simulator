#include <cstdint>
#include <vector>
#include <MemoryStorage.h>
#include <memory.h>

Memory::Region::Region(uint32_t base, uint32_t size) {
    this->base = base;
    this->size = size;
    this->data = nullptr;
}

Memory::Region::~Region() {
    delete[] data;
}

void Memory::Region::allocate() {
    data = new uint8_t[size];
}

bool Memory::Region::is_within(uint32_t addr) {
    return addr >= base && addr < base + size;
}

Memory::Memory() = default;

Memory::~Memory() = default;

void *Memory::v2p(uint32_t virtual_address) {
    for (auto & r : regions) {
        if (r.is_within(virtual_address)) {
            return virtual_address - r.base + r.data;
        }
    }
    return nullptr;
}

void Memory::add_region(uint32_t base, uint32_t size) {
    regions.emplace_back(base, size);
    regions.back().allocate();
}

void Memory::read(uint32_t addr, void* buf, uint32_t len) {
    void* real_addr = v2p(addr);
    void* real_addr_end = v2p(addr + len);
    if (real_addr == nullptr || real_addr_end == nullptr || (uint8_t*)real_addr_end - (uint8_t*)real_addr != len)
        throw std::runtime_error("load access fault");
    memcpy(buf, real_addr, len);
}

void Memory::write(uint32_t addr, void* buf, uint32_t len) {
    void* real_addr = v2p(addr);
    void* real_addr_end = v2p(addr + len);
    if (real_addr == nullptr || real_addr_end == nullptr || (uint8_t*)real_addr_end - (uint8_t*)real_addr != len)
        throw std::runtime_error("store access fault");
    memcpy(real_addr, buf, len);
}

uint32_t Memory::read_word(uint32_t addr) noexcept(false) {
    uint32_t v;
    read(addr, &v, 4);
    return v;
}

uint16_t Memory::read_half(uint32_t addr) noexcept(false) {
    uint16_t v;
    read(addr, &v, 2);
    return v;
}

uint8_t Memory::read_byte(uint32_t addr) noexcept(false) {
    uint8_t v;
    read(addr, &v, 1);
    return v;
}

void Memory::write_word(uint32_t addr, uint32_t word) noexcept(false) {
    write(addr, &word, 4);
}

void Memory::write_half(uint32_t addr, uint16_t half) noexcept(false) {
    write(addr, &half, 2);
}

void Memory::write_byte(uint32_t addr, uint8_t byte) noexcept(false) {
    write(addr, &byte, 1);
}

void Memory::reset() {
    for (Region& r : regions) {
        memset(r.data, 0, r.size);
    }
}
