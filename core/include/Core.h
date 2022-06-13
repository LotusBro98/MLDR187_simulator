#ifndef MLDR187_SIMULATOR_CORE_H
#define MLDR187_SIMULATOR_CORE_H

#include <MemoryStorage.h>
#include <string>
#include "CPU.h"
#include "Device.h"
#include "Bus.h"

class Core {
public:
    Core();
    ~Core();

    void load_image_from_hex_file(const std::string& path);
    void init();
    void step();
    void reset();
//private:
    Bus bus;
    CPU cpu;
};

extern Core core;

#endif //MLDR187_SIMULATOR_CORE_H
