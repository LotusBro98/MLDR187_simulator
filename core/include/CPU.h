#ifndef MLDR187_SIMULATOR_CPU_H
#define MLDR187_SIMULATOR_CPU_H

#include <MemoryStorage.h>
#include <cstdint>
#include <Instruction.h>
#include "enums.h"
#include "bits.h"
#include "Exception.h"

class CPU {
public:
    CPU();

    void step();
    void reset();

    bool halt_request = false;
private:
    void (CPU::*const execute_op[Instruction::COUNT])(Instruction& inst);

    bool debug_mode = false;
    int single_step = 0;
    uint32_t regs[32] = {};
    uint32_t pc = 0;
    uint32_t csr[0x1000] = {};
    uint32_t privilege = PRV_M;

    void enter_debug_mode(uint32_t cause);

    uint32_t fetch();
    void execute(Instruction& inst);

    void handle_exception(Exception &e);

    void execute_ILL(Instruction &inst);
    void execute_NOP(Instruction &inst);
    void execute_LUI(Instruction &inst);
    void execute_AUIPC(Instruction &inst);
    void execute_JAL(Instruction &inst);
    void execute_JALR(Instruction &inst);
    void execute_BEQ(Instruction &inst);
    void execute_BNE(Instruction &inst);
    void execute_BLT(Instruction &inst);
    void execute_BGE(Instruction &inst);
    void execute_BLTU(Instruction &inst);
    void execute_BGEU(Instruction &inst);
    void execute_LB(Instruction &inst);
    void execute_LH(Instruction &inst);
    void execute_LW(Instruction &inst);
    void execute_LBU(Instruction &inst);
    void execute_LHU(Instruction &inst);
    void execute_SB(Instruction &inst);
    void execute_SH(Instruction &inst);
    void execute_SW(Instruction &inst);
    void execute_ADDI(Instruction &inst);
    void execute_SLTI(Instruction &inst);
    void execute_SLTIU(Instruction &inst);
    void execute_XORI(Instruction &inst);
    void execute_ORI(Instruction &inst);
    void execute_ANDI(Instruction &inst);
    void execute_SLLI(Instruction &inst);
    void execute_SRLI(Instruction &inst);
    void execute_SRAI(Instruction &inst);
    void execute_ADD(Instruction &inst);
    void execute_SUB(Instruction &inst);
    void execute_SLL(Instruction &inst);
    void execute_SLT(Instruction &inst);
    void execute_SLTU(Instruction &inst);
    void execute_XOR(Instruction &inst);
    void execute_SRL(Instruction &inst);
    void execute_SRA(Instruction &inst);
    void execute_OR(Instruction &inst);
    void execute_AND(Instruction &inst);
    void execute_ECALL(Instruction &inst);
    void execute_EBREAK(Instruction &inst);
    void execute_MRET(Instruction &inst);
    void execute_DRET(Instruction &inst);
    void execute_CSRRW(Instruction &inst);
    void execute_CSRRS(Instruction &inst);
    void execute_CSRRC(Instruction &inst);
    void execute_CSRRWI(Instruction &inst);
    void execute_CSRRSI(Instruction &inst);
    void execute_CSRRCI(Instruction &inst);
    void execute_MUL(Instruction &inst);
    void execute_MULH(Instruction &inst);
    void execute_MULHSU(Instruction &inst);
    void execute_MULHU(Instruction &inst);
    void execute_DIV(Instruction &inst);
    void execute_DIVU(Instruction &inst);
    void execute_REM(Instruction &inst);
    void execute_REMU(Instruction &inst);
};

#endif //MLDR187_SIMULATOR_CPU_H
