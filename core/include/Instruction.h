#ifndef MLDR187_SIMULATOR_INSTRUCTION_H
#define MLDR187_SIMULATOR_INSTRUCTION_H

/* RV-IMC */

#include <cstdint>

class Instruction {
public:
    enum Code {
        ILL = 0,
        NOP,

        LUI, AUIPC,
        JAL, JALR,
        BEQ, BNE, BLT, BGE, BLTU, BGEU,
        LB, LH, LW, LBU, LHU,
        SB, SH, SW,
        ADDI,
        SLTI, SLTIU,
        XORI, ORI, ANDI, SLLI, SRLI, SRAI,
        ADD, SUB,
        SLL, SLT, SLTU,
        XOR, SRL, SRA, OR, AND,
        ECALL, EBREAK, MRET, DRET,
        CSRRW, CSRRS, CSRRC, CSRRWI, CSRRSI, CSRRCI,
        MUL, MULH, MULHSU, MULHU,
        DIV, DIVU, REM, REMU,

        COUNT
    };

    Instruction();

    explicit Instruction(Code code, uint32_t immediate = 0, uint32_t rs1 = 0, uint32_t rs2 = 0, uint32_t rd = 0, bool isCExt = false);

    Code code;
    uint32_t immediate;
    uint32_t rs1;
    uint32_t rs2;
    uint32_t rd;
    bool is_c_ext;

    void decode(uint32_t inst);

    void decode_basic(uint32_t inst);
    void decode_compressed(uint32_t inst);

//    uint32_t encode();

//private:
    void decode_type_R(uint32_t inst);
    void decode_type_I_JALR(uint32_t inst);
    void decode_type_I_arithm(uint32_t inst);
    void decode_type_I_system(uint32_t inst);
    void decode_type_I_load(uint32_t inst);
    void decode_type_S(uint32_t inst);
    void decode_type_B(uint32_t inst);
    void decode_type_U(uint32_t inst);
    void decode_type_J(uint32_t inst);

    void decode_type_CR(uint32_t inst);
    void decode_type_CI(uint32_t inst);
    void decode_type_CSS(uint32_t inst);
    void decode_type_CIW(uint32_t inst);
    void decode_type_CL(uint32_t inst);
    void decode_type_CS_B(uint32_t inst);
    void decode_type_CJ(uint32_t inst);

//    uint32_t encode_type_S();
//    uint32_t encode_type_I_load();
};

uint32_t lw(uint32_t rd, uint32_t base, uint16_t offset);
uint32_t sw(uint32_t src, uint32_t base, uint16_t offset);
uint32_t ebreak();
uint32_t jal(unsigned int rd, uint32_t imm);
uint32_t csrw(unsigned int source, unsigned int csr);
uint32_t csrr(unsigned int rd, unsigned int csr);

#endif //MLDR187_SIMULATOR_INSTRUCTION_H
