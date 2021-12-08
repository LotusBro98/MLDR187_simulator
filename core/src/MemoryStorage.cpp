#include <cstdint>
#include <vector>
#include <MemoryStorage.h>
#include <memory.h>
#include <CPU.h>

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
        if (r->is_within(virtual_address)) {
            return virtual_address - r->base + r->data;
        }
    }
    return nullptr;
}

void Memory::add_region(uint32_t base, uint32_t size) {
    regions.push_back(new Region(base, size));
    regions.back()->allocate();
}

void Memory::read(uint32_t addr, void* buf, uint32_t len) {
    void* real_addr = v2p(addr);
    void* real_addr_end = v2p(addr + len - 1);
    if (real_addr == nullptr || real_addr_end == nullptr || (uint8_t*)real_addr_end - (uint8_t*)real_addr != len - 1)
        throw CPU::EXC_LAF;
    memcpy(buf, real_addr, len);
}

void Memory::write(uint32_t addr, void* buf, uint32_t len) {
    void* real_addr = v2p(addr);
    void* real_addr_end = v2p(addr + len - 1);
    if (real_addr == nullptr || real_addr_end == nullptr || (uint8_t*)real_addr_end - (uint8_t*)real_addr != len - 1)
        throw CPU::EXC_SAF;
    memcpy(real_addr, buf, len);
}

uint32_t Memory::read_word(uint32_t addr) {
    if (addr % 4 != 0)
        throw CPU::EXC_LAM;
    uint32_t v;
    read(addr, &v, 4);
    return v;
}

uint16_t Memory::read_half(uint32_t addr) {
    if (addr % 2 != 0)
        throw CPU::EXC_LAM;
    uint16_t v;
    read(addr, &v, 2);
    return v;
}

uint8_t Memory::read_byte(uint32_t addr) {
    uint8_t v;
    read(addr, &v, 1);
    return v;
}

void Memory::write_word(uint32_t addr, uint32_t word) {
    if (addr % 4 != 0)
        throw CPU::EXC_SAM;
    write(addr, &word, 4);
}

void Memory::write_half(uint32_t addr, uint16_t half) {
    if (addr % 2 != 0)
        throw CPU::EXC_SAM;
    write(addr, &half, 2);
}

void Memory::write_byte(uint32_t addr, uint8_t byte) {
    write(addr, &byte, 1);
}

void Memory::reset() {
    for (Region* r : regions) {
        memset(r->data, 0, r->size);
    }
}
