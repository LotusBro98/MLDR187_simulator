#ifndef MLDR187_SIMULATOR_CPU_H
#define MLDR187_SIMULATOR_CPU_H

#include <MemoryStorage.h>
#include <cstdint>
#include <Instruction.h>

class CPU {
public:
    explicit CPU(Memory * memory);

    void step();
    void reset();
private:
    Memory* memory;

    uint32_t regs[32] = {};
    uint32_t pc = 0;

    uint32_t fetch();
    void execute(Instruction& inst);
};

#endif //MLDR187_SIMULATOR_CPU_H
