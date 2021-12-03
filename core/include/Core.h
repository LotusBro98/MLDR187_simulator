#ifndef MLDR187_SIMULATOR_CORE_H
#define MLDR187_SIMULATOR_CORE_H

#include <MemoryStorage.h>
#include <string>
#include "CPU.h"

class Core {
public:
    Core();
    ~Core();

    void load_image_from_hex_file(const std::string& path);
    void step();
    void reset();
private:
    Memory memory;
    CPU cpu;
};

#endif //MLDR187_SIMULATOR_CORE_H
