#include <string>
#include "Core.h"

Core::Core() : bus(), cpu() {};

Core::~Core() = default;

void Core::step() {
    cpu.step();
    bus.tick();
    fflush(stderr);
}

void Core::reset() {
    bus.reset();
    cpu.reset();
}

void Core::init() {
    bus.init();
}
