#include <Instruction.h>
#include <stdexcept>
#include <CPU.h>

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
#define SIGN_BIT(b, n) (((uint32_t)-(b)) << (n))

#define SET_FIELD(x, base, size, val) x = ((x) & ~((~(((uint32_t)-1) << (size))) << (base))) | (((val) & ~(((uint32_t)-1) << (size))) << (base))
#define SET_BIT(x, n, val) x = ((x) & ~(1 << (n))) | (((val) & 1) << (n))

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

//static uint32_t (Instruction::*const encode_type[])() = {
//        /* TYPE_INVALID */ nullptr,
//        /* TYPE_R */ 0/*&Instruction::decode_type_R*/,
//        /* TYPE_I */ 0/*&Instruction::decode_type_I_JALR*/,
//        /* TYPE_I */ 0/*&Instruction::decode_type_I_arithm*/,
//        /* TYPE_I */ 0/*&Instruction::decode_type_I_system*/,
//        /* TYPE_I */ &Instruction::encode_type_I_load,
//        /* TYPE_S */ &Instruction::encode_type_S,
//        /* TYPE_B */ 0/*&Instruction::decode_type_B*/,
//        /* TYPE_U */ 0/*&Instruction::decode_type_U*/,
//        /* TYPE_J */ 0/*&Instruction::decode_type_J*/,
//
//        /* TYPE_CR */ 0/*&Instruction::decode_type_CR*/,
//        /* TYPE_CI */ 0/*&Instruction::decode_type_CI*/,
//        /* TYPE_CSS */ 0/*&Instruction::decode_type_CSS*/,
//        /* TYPE_CIW */ 0/*&Instruction::decode_type_CIW*/,
//        /* TYPE_CL */ 0/*&Instruction::decode_type_CL*/,
//        /* TYPE_CS_B */ 0/*&Instruction::decode_type_CS_B*/,
//        /* TYPE_CJ */ 0/*&Instruction::decode_type_CJ*/,
//};

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
        TYPE_I_LOAD,  TYPE_INVALID, TYPE_INVALID, TYPE_I_SYSTEM, TYPE_I_ARITHM, TYPE_U,       TYPE_INVALID, TYPE_INVALID,
        TYPE_S,       TYPE_INVALID, TYPE_INVALID, TYPE_INVALID, TYPE_R,        TYPE_U,       TYPE_INVALID, TYPE_INVALID,
        TYPE_INVALID, TYPE_INVALID, TYPE_INVALID, TYPE_INVALID, TYPE_INVALID,  TYPE_INVALID, TYPE_INVALID, TYPE_INVALID,
        TYPE_B,       TYPE_I_JALR,  TYPE_INVALID, TYPE_J,       TYPE_I_SYSTEM, TYPE_INVALID, TYPE_INVALID, TYPE_INVALID
    };

    uint32_t opcode = (inst & 0x7f) >> 2;
    Type type = type_by_opcode[opcode];
    auto decode_func = decode_type[type];
    if (decode_func == nullptr)
        throw EXC_II;

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
        throw EXC_II;

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
    immediate = FIELD(inst, 12, 20) << 12;
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
            FIELD(inst, 7, 5) << 0 ;

    static const Code code_funct[8] = {
            SB, SH, SW, ILL, ILL, ILL, ILL, ILL
    };

    code = code_funct[funct3];
}

void Instruction::decode_type_I_JALR(uint32_t inst) {
    rs1 = FIELD(inst, 15, 5);
    rd = FIELD(inst, 7, 5);
    immediate = SIGN_BIT(BIT(inst, 31), 11) | FIELD(inst, 20, 11);
    code = JALR;
}

void Instruction::decode_type_I_arithm(uint32_t inst) {
    rs1 = FIELD(inst, 15, 5);
    rd = FIELD(inst, 7, 5);
    uint32_t funct3 = FIELD(inst, 12, 3);
    immediate = SIGN_BIT(BIT(inst, 31), 11) | FIELD(inst, 20, 11);
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
    immediate = SIGN_BIT(BIT(inst, 31), 11) | FIELD(inst, 20, 11);

    static const Code code_funct[8] = {
            LB, LH, LW, ILL, LBU, LHU, ILL, ILL
    };

    code = code_funct[funct3];
}

#define MATCH_MRET 0x30200073
#define MATCH_DRET 0x7b200073

#define MATCH_FENCE 0xf
#define MASK_FENCE  0x707f
#define MATCH_FENCE_I 0x100f
#define MASK_FENCE_I  0x707f
#define MATCH_WFI 0x10500073

void Instruction::decode_type_I_system(uint32_t inst) {
    rs1 = FIELD(inst, 15, 5);
    rd = FIELD(inst, 7, 5);
    uint32_t funct3 = FIELD(inst, 12, 3);
    immediate = FIELD(inst, 20, 12);

    if (inst == MATCH_MRET) {
        code = MRET;
        return;
    } else if (inst == MATCH_DRET) {
        code = DRET;
        return;
    } else if ((inst & MASK_FENCE) == MATCH_FENCE || (inst & MASK_FENCE_I) == MATCH_FENCE_I) {
        code = NOP;
        return;
    } else if (inst == MATCH_WFI) {
        code = NOP;
        return;
    }

    uint32_t funct =
            BIT(inst, 20) << 3 |
            funct3;

    static const Code code_funct[16] = {
            ECALL,  CSRRW, CSRRS, CSRRC, ILL, CSRRWI, CSRRSI, CSRRCI,
            EBREAK, CSRRW, CSRRS, CSRRC, ILL, CSRRWI, CSRRSI, CSRRCI
    };

    code = code_funct[funct];
}

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

    throw EXC_II;
}

void Instruction::decode_type_CI(uint32_t inst) {
    uint32_t r = FIELD(inst, 7, 5);
    uint32_t imm = FIELD(inst, 2, 5) | BIT(inst, 12) << 5;
    uint32_t op = FIELD(inst, 0, 2);
    uint32_t funct3 = FIELD(inst, 13, 3);

    if (op == 1) {
        if (funct3 == 0) {
            //ADDI
            code = ADDI;
            rd = r;
            rs1 = r;
            immediate = SIGN_BIT(BIT(imm, 5), 5) | imm;
        } else if (funct3 == 2) {
            // LI
            code = ADDI;
            rd = r;
            rs1 = 0;
            immediate = SIGN_BIT(BIT(imm, 5), 5) | imm;
        } else if (funct3 == 3) {
            if (imm == 0) {
                throw EXC_II;
            }
            if (r == 2) {
                // ADDI sp
                code = ADDI;
                rd = 2;
                rs1 = 2;
                immediate = BIT(imm, 4) << 4 |
                            BIT(imm, 0) << 5 |
                            BIT(imm, 3) << 6 |
                            BIT(imm, 1) << 7 |
                            BIT(imm, 2) << 8 |
                        SIGN_BIT(BIT(imm, 5), 9);
            } else {
                // LUI
                code = LUI;
                rd = r;
                immediate = (SIGN_BIT(BIT(imm, 5), 5) | imm) << 12;
            }
        } else {
            throw EXC_II;
        }
    } else if (op == 2) {
        if (funct3 == 0) {
            if (imm == 0 || BIT(imm, 5) == 1) {
                throw EXC_II;
            }
            code = SLLI;
            rd = r;
            rs1 = r;
            immediate = imm;
        } else if (funct3 == 2) {
            // LW sp
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
            throw EXC_II;
        }
    } else {
        throw EXC_II;
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

    if (r == 0 && imm == 0) {
        throw EXC_II;
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
            SIGN_BIT(BIT(imm, 10), 11);

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
                throw EXC_II;
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
                throw EXC_II;
            }
            immediate = imm;
            rd = 8 + r1;
            rs1 = 8 + r1;
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
                SIGN_BIT(BIT(imm, 7), 8);
        rs1 = 8 + r1;
        rs2 = 0;
    }
}

Instruction::Instruction(Instruction::Code code, uint32_t immediate, uint32_t rs1, uint32_t rs2, uint32_t rd, bool isCExt):
    code(code), immediate(immediate), rs1(rs1), rs2(rs2), rd(rd), is_c_ext(isCExt) {}

Instruction::Instruction() {

}

//uint32_t Instruction::encode() {
//    const Type type_by_op[Code::COUNT] = {
//        [LW] = TYPE_I_LOAD,
//        [SW] = TYPE_S,
//    };
//
//    Type type = type_by_op[this->code];
//    auto encode_func = encode_type[type];
//    if (encode_func == nullptr)
//        throw EXC_II;
//
//    return (*this.*encode_func)();
//}
//
//uint32_t Instruction::encode_type_S() {
//    const uint32_t funct3_code[Code::COUNT] = {
//        [SB] = 0,
//        [SH] = 1,
//        [SW] = 2
//    };
//
//    uint32_t inst = 0;
//    SET_FIELD(inst, 15, 5, rs1);
//    SET_FIELD(inst, 20, 5, rs2);
//    SET_FIELD(inst, 12, 3, funct3_code[code]);
//
//    SET_FIELD(inst, 25, 6, FIELD(immediate, 5, 6));
//    SET_FIELD(inst, 8, 6, FIELD(immediate, 5, 6));
//
//    immediate =
//            (BIT(inst, 31) ? 0xfffff800 : 0) |
//            FIELD(inst, 25, 6) << 5 |
//            FIELD(inst, 8, 4) << 1 |
//            BIT(inst, 7) << 1;
//}

#define MATCH_LW 0x2003
#define MATCH_SW 0x2023
#define MATCH_EBREAK 0x100073
#define MATCH_JAL 0x6f
#define MATCH_CSRRW 0x1073
#define MATCH_CSRRS 0x2073

static uint32_t bits(uint32_t value, unsigned int hi, unsigned int lo) {
    return (value >> lo) & ((1 << (hi+1-lo)) - 1);
}

static uint32_t bit(uint32_t value, unsigned int b) {
    return (value >> b) & 1;
}

uint32_t sw(unsigned int src, unsigned int base, uint16_t offset)
{
    return (bits(offset, 11, 5) << 25) |
           (src << 20) |
           (base << 15) |
           (bits(offset, 4, 0) << 7) |
           MATCH_SW;
}

uint32_t lw(unsigned int rd, unsigned int base, uint16_t offset)
{
    return (bits(offset, 11, 0) << 20) |
           (base << 15) |
           (bits(rd, 4, 0) << 7) |
           MATCH_LW;
}

uint32_t ebreak() { return MATCH_EBREAK; }

uint32_t jal(unsigned int rd, uint32_t imm) {
    return (bit(imm, 20) << 31) |
           (bits(imm, 10, 1) << 21) |
           (bit(imm, 11) << 20) |
           (bits(imm, 19, 12) << 12) |
           (rd << 7) |
           MATCH_JAL;
}

uint32_t csrw(unsigned int source, unsigned int csr) {
    return (csr << 20) | (source << 15) | MATCH_CSRRW;
}

uint32_t csrr(unsigned int rd, unsigned int csr) {
    return (csr << 20) | (rd << 7) | MATCH_CSRRS;
}