#include <Instruction.h>
#include <stdexcept>

/* RV-IMC */

enum Type {
    TYPE_INVALID = 0, // Invalid
    TYPE_R,
    TYPE_I_JALR,
    TYPE_I_ARITHM,
    TYPE_I_SYSTEM,
    TYPE_I_LOAD,
    TYPE_S,
    TYPE_B,
    TYPE_U,
    TYPE_J,

    TYPE_CR,
    TYPE_CI,
    TYPE_CSS,
    TYPE_CIW,
    TYPE_CL,
    TYPE_CS_B,
    TYPE_CJ,
};

#define FIELD(x, base, size) (((x) >> (base)) & ~(((uint32_t)-1) << (size)))
#define BIT(x, n) (((x) >> (n)) & 1)

static void (Instruction::*const decode_type[])(uint32_t inst) = {
        /* TYPE_INVALID */ nullptr,
        /* TYPE_R */ &Instruction::decode_type_R,
        /* TYPE_I */ &Instruction::decode_type_I_JALR,
        /* TYPE_I */ &Instruction::decode_type_I_arithm,
        /* TYPE_I */ &Instruction::decode_type_I_system,
        /* TYPE_I */ &Instruction::decode_type_I_load,
        /* TYPE_S */ &Instruction::decode_type_S,
        /* TYPE_B */ &Instruction::decode_type_B,
        /* TYPE_U */ &Instruction::decode_type_U,
        /* TYPE_J */ &Instruction::decode_type_J,

        /* TYPE_CR */ &Instruction::decode_type_CR,
        /* TYPE_CI */ &Instruction::decode_type_CI,
        /* TYPE_CSS */ &Instruction::decode_type_CSS,
        /* TYPE_CIW */ &Instruction::decode_type_CIW,
        /* TYPE_CL */ &Instruction::decode_type_CL,
        /* TYPE_CS_B */ &Instruction::decode_type_CS_B,
        /* TYPE_CJ */ &Instruction::decode_type_CJ,
};

void Instruction::decode(uint32_t inst) {
    uint32_t opcode = inst & 0x7f;
    if ((opcode & 0x3) == 0x3) {
        is_c_ext = false;
        decode_basic(inst);
    } else {
        is_c_ext = true;
        decode_compressed(inst);
    }
}


void Instruction::decode_basic(uint32_t inst) {
    static const Type type_by_opcode[32] = {
        TYPE_I_LOAD,  TYPE_INVALID, TYPE_INVALID, TYPE_INVALID, TYPE_I_ARITHM, TYPE_U,       TYPE_INVALID, TYPE_INVALID,
        TYPE_S,       TYPE_INVALID, TYPE_INVALID, TYPE_INVALID, TYPE_R,        TYPE_U,       TYPE_INVALID, TYPE_INVALID,
        TYPE_INVALID, TYPE_INVALID, TYPE_INVALID, TYPE_INVALID, TYPE_INVALID,  TYPE_INVALID, TYPE_INVALID, TYPE_INVALID,
        TYPE_B,       TYPE_I_JALR,  TYPE_INVALID, TYPE_J,       TYPE_I_SYSTEM, TYPE_INVALID, TYPE_INVALID, TYPE_INVALID
    };

    uint32_t opcode = (inst & 0x7f) >> 2;
    Type type = type_by_opcode[opcode];
    auto decode_func = decode_type[type];
    if (decode_func == nullptr)
        throw std::runtime_error("illegal instruction");

    (*this.*decode_func)(inst);
}

void Instruction::decode_compressed(uint32_t inst) {
    static const Type type_by_opcode[32] = {
        TYPE_CIW, TYPE_INVALID, TYPE_CL, TYPE_INVALID, TYPE_INVALID, TYPE_INVALID, TYPE_CS_B, TYPE_INVALID,
        TYPE_CI, TYPE_CJ, TYPE_CI, TYPE_CI, TYPE_CS_B, TYPE_CJ, TYPE_CS_B, TYPE_CS_B,
        TYPE_CI, TYPE_INVALID, TYPE_CI, TYPE_INVALID, TYPE_CR, TYPE_INVALID, TYPE_CSS, TYPE_INVALID,
    };

    uint32_t op = FIELD(inst, 0, 2);
    uint32_t funct3 = FIELD(inst, 13, 3);
    uint32_t opcode = op << 3 | funct3;
    Type type = type_by_opcode[opcode];

    auto decode_func = decode_type[type];
    if (decode_func == nullptr)
        throw std::runtime_error("illegal instruction");

    (*this.*decode_func)(inst);
}

void Instruction::decode_type_R(uint32_t inst) {
    rd = FIELD(inst, 7, 5);
    rs1 = FIELD(inst, 15, 5);
    rs2 = FIELD(inst, 20, 5);
    uint32_t funct3 = FIELD(inst, 12, 3);
    uint32_t funct7 = FIELD(inst, 25, 7);

    uint32_t funct =
            BIT(funct7, 0) << 4 |
            BIT(funct7, 5) << 3 |
            funct3;

    static const Code code_funct [32] = {
        ADD, SLL, SLT, SLTU, XOR, SRL, OR,  AND,
        SUB, ILL, ILL, ILL, ILL, SRA, ILL, ILL,
        MUL, MULH, MULHSU, MULHU, DIV, DIVU, REM, REMU,
        ILL, ILL, ILL, ILL, ILL, ILL, ILL, ILL,
    };

    code = code_funct[funct];
}

void Instruction::decode_type_B(uint32_t inst) {
    rs1 = FIELD(inst, 15, 5);
    rs2 = FIELD(inst, 20, 5);
    uint32_t funct3 = FIELD(inst, 12, 3);

    immediate =
            (BIT(inst, 31) ? 0xfffff000 : 0) |
            BIT(inst, 7) << 11 |
            FIELD(inst, 25, 6) << 5 |
            FIELD(inst, 8, 4) << 1;

    static const Code code_funct [8] = {
            BEQ, BNE, ILL, ILL, BLT, BGE, BLTU, BGEU
    };

    code = code_funct[funct3];
}

void Instruction::decode_type_U(uint32_t inst) {
    rd = FIELD(inst, 7, 5);
    code = BIT(inst, 5) ? LUI : AUIPC;
    immediate = FIELD(inst, 12, 20);
}

void Instruction::decode_type_J(uint32_t inst) {
    rd = FIELD(inst, 7, 5);
    code = JAL;
    immediate =
            (BIT(inst, 31) ? 0xfff00000 : 0) |
            FIELD(inst, 12, 8) << 12 |
            BIT(inst, 20) << 11 |
            FIELD(inst, 25, 6) << 5 |
            FIELD(inst, 21, 4) << 1;
}

void Instruction::decode_type_S(uint32_t inst) {
    rs1 = FIELD(inst, 15, 5);
    rs2 = FIELD(inst, 20, 5);
    uint32_t funct3 = FIELD(inst, 12, 3);

    immediate =
            (BIT(inst, 31) ? 0xfffff800 : 0) |
            FIELD(inst, 25, 6) << 5 |
            FIELD(inst, 8, 4) << 1 |
            BIT(inst, 7) << 1;

    static const Code code_funct[8] = {
            SB, SH, SW, ILL, ILL, ILL, ILL, ILL
    };

    code = code_funct[funct3];
}

void Instruction::decode_type_I_JALR(uint32_t inst) {
    rs1 = FIELD(inst, 15, 5);
    rd = FIELD(inst, 7, 5);
    immediate = FIELD(inst, 20, 12);
    code = JALR;
}

void Instruction::decode_type_I_arithm(uint32_t inst) {
    rs1 = FIELD(inst, 15, 5);
    rd = FIELD(inst, 7, 5);
    uint32_t funct3 = FIELD(inst, 12, 3);
    immediate = FIELD(inst, 20, 12);
    uint32_t funct =
            BIT(inst, 30) << 3 |
            funct3;

    static const Code code_funct[16] = {
            ADDI, SLLI, SLTI, SLTIU, XORI, SRLI, ORI, ANDI,
            ADDI, SLLI, SLTI, SLTIU, XORI, SRAI, ORI, ANDI,
    };

    code = code_funct[funct];

    if (funct3 == 0x05)
        immediate &= 0x1f;
}

void Instruction::decode_type_I_load(uint32_t inst) {
    rs1 = FIELD(inst, 15, 5);
    rd = FIELD(inst, 7, 5);
    uint32_t funct3 = FIELD(inst, 12, 3);
    immediate = FIELD(inst, 20, 12);

    static const Code code_funct[8] = {
            LB, LH, LW, ILL, LBU, LHU, ILL, ILL
    };

    code = code_funct[funct3];
}

void Instruction::decode_type_I_system(uint32_t inst) {
    rs1 = FIELD(inst, 15, 5);
    rd = FIELD(inst, 7, 5);
    uint32_t funct3 = FIELD(inst, 12, 3);
    immediate = FIELD(inst, 20, 12);

    uint32_t funct =
            BIT(inst, 20) << 3 |
            funct3;

    static const Code code_funct[16] = {
            ECALL,  CSRRW, CSRRS, CSRRC, ILL, CSRRWI, CSRRSI, CSRRCI,
            EBREAK, CSRRW, CSRRS, CSRRC, ILL, CSRRWI, CSRRSI, CSRRCI
    };

    code = code_funct[funct];
}

// TODO sign-extend

void Instruction::decode_type_CR(uint32_t inst) {
    uint32_t bit = BIT(inst, 12);
    uint32_t r1 = FIELD(inst, 7, 5);
    uint32_t r2 = FIELD(inst, 2, 5);

    if (r2 != 0) {
        rd = r1;
        rs1 = bit == 0 ? 0 : r1;
        rs2 = r2;
        code = ADD;
        return;
    }

    if (r1 != 0) {
        rd = bit;
        rs1 = r1;
        immediate = 0;
        code = JALR;
        return;
    }

    if (bit != 0) {
        code = EBREAK;
        return;
    }

    throw std::runtime_error("illegal instruction");
}

void Instruction::decode_type_CI(uint32_t inst) {
    uint32_t r = FIELD(inst, 7, 5);
    uint32_t imm = FIELD(inst, 2, 5) | BIT(inst, 12) << 5;
    uint32_t op = FIELD(inst, 0, 2);
    uint32_t funct3 = FIELD(inst, 13, 3);

    if (op == 1) {
        if (funct3 == 0) {
            code = ADDI;
            rd = r;
            rs1 = r;
            immediate = imm;
        } else if (funct3 == 2) {
            code = ADDI;
            rd = r;
            rs1 = 0;
            immediate = imm;
        } else if (funct3 == 3) {
            if (imm == 0) {
                throw std::runtime_error("illegal instruction");
            }
            if (r == 2) {
                code = ADDI;
                rd = 2;
                rs1 = 2;
                immediate = BIT(imm, 4) << 4 |
                            BIT(imm, 0) << 5 |
                            BIT(imm, 3) << 6 |
                            BIT(imm, 1) << 7 |
                            BIT(imm, 2) << 8 |
                            BIT(imm, 5) << 9 ;
            } else {
                code = LUI;
                rd = r;
                immediate = imm << 12;
            }
        } else {
            throw std::runtime_error("illegal instruction");
        }
    } else if (op == 2) {
        if (funct3 == 0) {
            if (imm == 0 || BIT(imm, 5) == 1) {
                throw std::runtime_error("illegal instruction");
            }
            code = SLLI;
            rd = r;
            rs1 = r;
            immediate = imm;
        } else if (funct3 == 2) {
            code = LW;
            rd = r;
            rs1 = 2;
            immediate = BIT(imm, 2) << 2 |
                        BIT(imm, 3) << 3 |
                        BIT(imm, 4) << 4 |
                        BIT(imm, 5) << 5 |
                        BIT(imm, 0) << 6 |
                        BIT(imm, 1) << 7 ;
        } else {
            throw std::runtime_error("illegal instruction");
        }
    } else {
        throw std::runtime_error("illegal instruction");
    }

}

void Instruction::decode_type_CSS(uint32_t inst) {
    uint32_t r = FIELD(inst, 2, 5);
    uint32_t imm = FIELD(inst, 7, 6);

    code = SW;
    rs2 = r;
    rs1 = 2;
    immediate = BIT(imm, 2) << 2 |
                BIT(imm, 3) << 3 |
                BIT(imm, 4) << 4 |
                BIT(imm, 5) << 5 |
                BIT(imm, 0) << 6 |
                BIT(imm, 1) << 7 ;
}

void Instruction::decode_type_CIW(uint32_t inst) {
    uint32_t r = FIELD(inst, 2, 3);
    uint32_t imm = FIELD(inst, 5, 8);

    if (r == 0 || imm == 0) {
        throw std::runtime_error("illegal instruction");
    }

    code = ADDI;
    rd = r + 8;
    rs1 = 2;
    immediate = BIT(imm, 1) << 2 |
                BIT(imm, 0) << 3 |
                BIT(imm, 6) << 4 |
                BIT(imm, 7) << 5 |
                BIT(imm, 2) << 6 |
                BIT(imm, 3) << 7 |
                BIT(imm, 4) << 8 |
                BIT(imm, 5) << 9 ;
}

void Instruction::decode_type_CL(uint32_t inst) {
    uint32_t r1 = FIELD(inst, 7, 3);
    uint32_t r2 = FIELD(inst, 2, 3);
    uint32_t imm = FIELD(inst, 5, 2) | FIELD(inst, 10, 3) << 2;
    code = LW;
    rs1 = 8 + r1;
    rd = 8 + r2;
    immediate = BIT(imm, 1) << 2 |
                BIT(imm, 2) << 3 |
                BIT(imm, 3) << 4 |
                BIT(imm, 4) << 5 |
                BIT(imm, 0) << 6 ;
}

void Instruction::decode_type_CJ(uint32_t inst) {
    uint32_t funct3 = FIELD(inst, 13, 3);
    uint32_t imm = FIELD(inst, 2, 11);
    immediate = BIT(imm, 1) << 1  |
                BIT(imm, 2) << 2  |
                BIT(imm, 3) << 3  |
                BIT(imm, 9) << 4  |
                BIT(imm, 0) << 5  |
                BIT(imm, 5) << 6  |
                BIT(imm, 4) << 7  |
                BIT(imm, 7) << 8  |
                BIT(imm, 8) << 9  |
                BIT(imm, 6) << 10 |
                BIT(imm, 10) << 11;

    code = JAL;
    rd = funct3 == 1 ? 1 : 0;
}

void Instruction::decode_type_CS_B(uint32_t inst) {
    uint32_t funct3 = FIELD(inst, 13, 3);
    uint32_t op = FIELD(inst, 0, 2);
    uint32_t r1 = FIELD(inst, 7, 3);

    if (op == 0) {
        // Type CS
        uint32_t r2 = FIELD(inst, 2, 3);
        uint32_t imm = FIELD(inst, 5, 2) | FIELD(inst, 10, 3) << 2;
        code = SW;
        rs1 = 8 + r1;
        rs2 = 8 + r2;
        immediate = BIT(imm, 1) << 2 |
                    BIT(imm, 2) << 3 |
                    BIT(imm, 3) << 4 |
                    BIT(imm, 4) << 5 |
                    BIT(imm, 0) << 6 ;
        return;
    }

    if (funct3 == 4) {
        uint32_t funct2 = FIELD(inst, 10, 2);
        if (funct2 == 3) {
            // Type CS
            uint32_t funct1 = BIT(inst, 12);
            uint32_t funct = FIELD(inst, 5, 2);
            uint32_t r2 = FIELD(inst, 2, 3);
            if (funct1 == 1) {
                throw std::runtime_error("illegal instruction");
            }
            const Code codes[4] = {
                    SUB, XOR, OR, AND
            };
            code = codes[funct];
            rd = 8 + r1;
            rs1 = 8 + r1;
            rs2 = 8 + r2;
        } else {
            // Type CB
            uint32_t imm = FIELD(inst, 2, 5) | BIT(inst, 12) << 5;
            if (funct2 != 2 && BIT(imm, 5) != 0) {
                throw std::runtime_error("illegal instruction");
            }
            immediate = imm;
            rd = r1;
            rs1 = r1;
            const Code codes[3] = {
                SRLI, SRAI, ANDI
            };
            code = codes[funct2];
        }
    } else {
        // Type CB
        uint32_t imm = FIELD(inst, 2, 5) | FIELD(inst, 10, 3) << 5;
        code = funct3 == 6 ? BEQ : BNE;
        immediate = BIT(imm, 1) << 1 |
                    BIT(imm, 2) << 2 |
                    BIT(imm, 5) << 3 |
                    BIT(imm, 6) << 4 |
                    BIT(imm, 0) << 5 |
                    BIT(imm, 3) << 6 |
                    BIT(imm, 4) << 7 |
                    BIT(imm, 7) << 8 ;
        rs1 = 8 + r1;
        rs2 = 0;
    }
}
