#include <CPU.h>
#include <memory.h>
#include <Config.h>

CPU::CPU(Memory *memory) {
    this->memory = memory;
}

void CPU::step() {
    Instruction inst = {};
    uint32_t raw = fetch();
    inst.decode(raw);
    execute(inst);
}

void CPU::execute(Instruction& inst) {
    pc += 4;
}

void CPU::reset() {
    pc = START_PC;
    memset(regs, 0, sizeof(regs));
}

uint32_t CPU::fetch() {
    uint32_t inst = memory->read_half(pc);
    if ((inst & 0x3) == 0x3) {
        /* if not compressed */
        inst |= memory->read_half(pc + 2) << 16;
    }
    return inst;
}
