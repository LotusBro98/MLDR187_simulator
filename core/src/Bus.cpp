#include "Bus.h"
#include "enums.h"
#include "macros.h"
#include "Exception.h"

void Bus::add_device(Device *device) {
    devices.push_back(device);
}

Device *Bus::get_device(uint32_t addr) {
    for (auto d : devices) {
        if (d->is_within(addr)) {
            return d;
        }
    }
    return nullptr;
}

uint32_t Bus::read(uint32_t addr, uint32_t len) noexcept(false) {
    assert_param(len == 1 || len == 2 || len == 4);
    if (len != 1 && len != 2 && len != 4)
        throw EXC_LAF;
    if (addr % len != 0)
        throw EXC_LAM;
    Device * device = get_device(addr);
    if (device == nullptr)
        throw EXC_LAF;
    addr -= device->get_start_addr();
    return device->read(addr, len);
}

void Bus::write(uint32_t addr, uint32_t value, uint32_t len) noexcept(false) {
    assert_param(len == 1 || len == 2 || len == 4);
    if (addr % len != 0)
        throw EXC_SAM;
    Device * device = get_device(addr);
    if (device == nullptr)
        throw EXC_SAF;
    addr -= device->get_start_addr();
    device->write(addr, value, len);
}

void Bus::tick() {
    for(Device * device : devices) {
        device->tick();
    }
}

void Bus::init() {
    for(Device * device : devices) {
        device->init();
    }
}

void Bus::reset() {
    for(Device * device : devices) {
        device->reset();
    }
}
