#ifndef MLDR187_SIMULATOR_CORE_H
#define MLDR187_SIMULATOR_CORE_H

#include "CPU.h"
#include "Device.h"
#include "Bus.h"

class Core {
public:
    Core();
    ~Core();

    void init();
    void step();
    void reset();

    Bus bus;
    CPU cpu;
};

extern Core core;

#endif //MLDR187_SIMULATOR_CORE_H
