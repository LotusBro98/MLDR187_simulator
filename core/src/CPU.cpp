#include <CPU.h>
#include <memory.h>
#include <Config.h>

#define FIELD(x, base, size) (((x) >> (base)) & ~(((uint32_t)-1) << (size)))
#define BIT(x, n) (((x) >> (n)) & 1)
#define SIGN_BIT(b, n) (((uint32_t)-(b)) << (n))

extern void (CPU::*const execute_op[Instruction::COUNT])(Instruction& inst);

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
    auto executor = execute_op[inst.code];
    (*this.*executor)(inst);
    pc += 4 >> inst.is_c_ext;
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

void CPU::execute_LUI(Instruction &inst) {
    regs[inst.rd] = inst.immediate;
}

void CPU::execute_ILL(Instruction &inst) {
    throw std::runtime_error("Illegal instruction");
}

void CPU::execute_AUIPC(Instruction &inst) {
    regs[inst.rd] = inst.immediate + pc;
}

void CPU::execute_JAL(Instruction &inst) {
    regs[inst.rd] = pc + (4 >> inst.is_c_ext);
    pc += inst.immediate;
}

void CPU::execute_JALR(Instruction &inst) {
    regs[inst.rd] = pc + (4 >> inst.is_c_ext);
    pc = regs[inst.rs1] + inst.immediate;
}

void CPU::execute_BNE(Instruction &inst) {
    if (regs[inst.rs1] != regs[inst.rs2])
        pc += inst.immediate - (4 >> inst.is_c_ext);
}

void CPU::execute_BEQ(Instruction &inst) {
    if (regs[inst.rs1] == regs[inst.rs2])
        pc += inst.immediate - (4 >> inst.is_c_ext);
}

void CPU::execute_BLT(Instruction &inst) {
    if ((int32_t)regs[inst.rs1] < (int32_t)regs[inst.rs2])
        pc += inst.immediate - (4 >> inst.is_c_ext);
}

void CPU::execute_BGE(Instruction &inst) {
    if ((int32_t)regs[inst.rs1] >= (int32_t)regs[inst.rs2])
        pc += inst.immediate - (4 >> inst.is_c_ext);
}

void CPU::execute_BLTU(Instruction &inst) {
    if (regs[inst.rs1] < regs[inst.rs2])
        pc += inst.immediate - (4 >> inst.is_c_ext);
}

void CPU::execute_BGEU(Instruction &inst) {
    if (regs[inst.rs1] >= regs[inst.rs2])
        pc += inst.immediate - (4 >> inst.is_c_ext);
}

void CPU::execute_LB(Instruction &inst) {
    uint32_t byte = memory->read_byte(regs[inst.rs1] + inst.immediate);
    regs[inst.rd] = byte | SIGN_BIT(BIT(byte, 7), 7);
}

void CPU::execute_LH(Instruction &inst) {
    uint32_t half = memory->read_half(regs[inst.rs1] + inst.immediate);
    regs[inst.rd] = half | SIGN_BIT(BIT(half, 15), 15);
}

void CPU::execute_LW(Instruction &inst) {
    regs[inst.rd] = memory->read_word(regs[inst.rs1] + inst.immediate);
}

void CPU::execute_LBU(Instruction &inst) {
    regs[inst.rd] = memory->read_byte(regs[inst.rs1] + inst.immediate);
}

void CPU::execute_LHU(Instruction &inst) {
    regs[inst.rd] = memory->read_half(regs[inst.rs1] + inst.immediate);
}

void CPU::execute_SB(Instruction &inst) {
    memory->write_byte(regs[inst.rs1] + inst.immediate, regs[inst.rs2]);
}

void CPU::execute_SH(Instruction &inst) {
    memory->write_half(regs[inst.rs1] + inst.immediate, regs[inst.rs2]);
}

void CPU::execute_SW(Instruction &inst) {
    memory->write_word(regs[inst.rs1] + inst.immediate, regs[inst.rs2]);
}

void (CPU::*const execute_op[Instruction::COUNT])(Instruction& inst) = {
        &CPU::execute_ILL,
        &CPU::execute_LUI,
        &CPU::execute_AUIPC,
        &CPU::execute_JAL,
        &CPU::execute_JALR,
        &CPU::execute_BEQ,
        &CPU::execute_BNE,
        &CPU::execute_BLT,
        &CPU::execute_BGE,
        &CPU::execute_BLTU,
        &CPU::execute_BGEU,
        &CPU::execute_LB,
        &CPU::execute_LH,
        &CPU::execute_LW,
        &CPU::execute_LBU,
        &CPU::execute_LHU,
        &CPU::execute_SB,
        &CPU::execute_SH,
        &CPU::execute_SW,
        &CPU::execute_ADDI,
        &CPU::execute_SLTI,
        &CPU::execute_SLTIU,
        &CPU::execute_XORI,
        &CPU::execute_ORI,
        &CPU::execute_ANDI,
        &CPU::execute_SLLI,
        &CPU::execute_SRLI,
        &CPU::execute_SRAI,
        &CPU::execute_ADD,
        &CPU::execute_SUB,
        &CPU::execute_SLL,
        &CPU::execute_SLT,
        &CPU::execute_SLTU,
        &CPU::execute_XOR,
        &CPU::execute_SRL,
        &CPU::execute_SRA,
        &CPU::execute_OR,
        &CPU::execute_AND,
        &CPU::execute_ECALL,
        &CPU::execute_EBREAK,
        &CPU::execute_CSRRW,
        &CPU::execute_CSRRS,
        &CPU::execute_CSRRC,
        &CPU::execute_CSRRWI,
        &CPU::execute_CSRRSI,
        &CPU::execute_CSRRCI,
        &CPU::execute_MUL,
        &CPU::execute_MULH,
        &CPU::execute_MULHSU,
        &CPU::execute_MULHU,
        &CPU::execute_DIV,
        &CPU::execute_DIVU,
        &CPU::execute_REM,
        &CPU::execute_REMU,
};