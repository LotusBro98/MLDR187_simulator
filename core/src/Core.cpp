#include <string>
#include <intelhex.h>
#include "Core.h"

Core::Core() : cpu(&memory) {
    memory.add_region(0x10000000, 0x40000);
    reset();
};

Core::~Core() = default;

void Core::step() {
    cpu.step();
}

void Core::load_image_from_hex_file(const std::string &path) {
    intelhex::hex_data data(path);
    for (auto & v : data) {
        uint32_t addr = v.first;
        for (auto& c : v.second) {
            memory.write_byte(addr++, c);
        }
    }
}

void Core::reset() {
    memory.reset();
    cpu.reset();
}
