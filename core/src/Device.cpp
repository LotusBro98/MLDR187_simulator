#include "Device.h"
#include "Core.h"

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
