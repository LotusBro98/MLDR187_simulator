#include <string>
#include <intelhex.h>
#include "Core.h"

Core::Core() : cpu(&memory) {
    // TODO no hardcode
    memory.add_region(0x10000000, 0x40000);
    memory.add_region(0x80000000, 0x10000);
    memory.add_region(0x80010000, 0x8000);
    memory.add_region(0x20000000, 0x4000);
    reset();
};

Core::~Core() = default;

void Core::step() {
    cpu.step();
}

void Core::load_image_from_hex_file(const std::string &path) {
    auto * data = new intelhex::hex_data(path);
    for (auto & v : *data) {
        uint32_t addr = v.first;
        for (auto& c : v.second) {
            memory.write_byte(addr++, c);
        }
    }
    delete data;
}

void Core::reset() {
    memory.reset();
    cpu.reset();
}
