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

    uint32_t pending_peripheral_interrupts;
    void raise_peripheral_interrupt(uint32_t irqN);
    bool stop_request = false;

    Bus bus;
    CPU cpu;
private:

};

extern Core core;

#endif //MLDR187_SIMULATOR_CORE_H
