#include "Device.h"
#include "Core.h"
#include "macros.h"

bool Device::is_within(uint32_t addr) {
    return addr >= get_start_addr() && addr < get_end_addr();
}

Device::Device() {
    core.bus.add_device(this);
}

void Device::init() {

}

void Device::tick() {

}

void Device::reset() {

}

uint32_t Device::read_from_mem(uint32_t addr, uint32_t len, void *buf) {
    assert_param(len == 1 || len == 2 || len == 4);
    void * src = (uint8_t*)buf + addr;
    switch (len) {
        case 1:
            return *(uint8_t*)src;
        case 2:
            return *(uint16_t*)src;
        case 4:
            return *(uint32_t*)src;
        default:
            throw EXC_LAF;
    }
}

void Device::write_to_mem(uint32_t addr, uint32_t value, uint32_t len, void *buf) {
    assert_param(len == 1 || len == 2 || len == 4);
    void * dst = (uint8_t*)buf + addr;
    switch (len) {
        case 1:
            *(uint8_t*)dst = value;
            return;
        case 2:
            *(uint16_t*)dst = value;
            return;
        case 4:
            *(uint32_t*)dst = value;
            return;
        default:
            throw EXC_SAF;
    }
}

uint32_t Device::get_priority() {
    return 0;
}

