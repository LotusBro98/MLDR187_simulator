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
    TYPE_CS,
    TYPE_CB,
    TYPE_CJ
};

#define M(a, m, args...) (a)->cls->m((a), args)

#define FIELD(x, base, size) (((x) >> (base)) & ~(((uint32_t)-1) << (size)))
#define BIT(x, n) (((x) >> (n)) & 1)

static const Type type_by_opcode[32] = {
        TYPE_I_LOAD,  TYPE_INVALID, TYPE_INVALID, TYPE_INVALID, TYPE_I_ARITHM, TYPE_U,       TYPE_INVALID, TYPE_INVALID,
        TYPE_S,       TYPE_INVALID, TYPE_INVALID, TYPE_INVALID, TYPE_R,        TYPE_U,       TYPE_INVALID, TYPE_INVALID,
        TYPE_INVALID, TYPE_INVALID, TYPE_INVALID, TYPE_INVALID, TYPE_INVALID,  TYPE_INVALID, TYPE_INVALID, TYPE_INVALID,
        TYPE_B,       TYPE_I_JALR,  TYPE_INVALID, TYPE_J,       TYPE_I_SYSTEM, TYPE_INVALID, TYPE_INVALID, TYPE_INVALID
};

class Type_R : Instruction {
public:
    explicit Type_R(uint32_t inst) {

    }
};

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
        /* TYPE_J */ &Type_R::Type_R,
};

void Instruction::decode(uint32_t inst) {
    uint32_t opcode = inst & 0x7f;
    if ((opcode & 0x3) != 0x3)
        throw std::runtime_error("illegal instruction");

    struct InstClass;
    struct InstClassMethods {
        void (*decode)(InstClass* obj, uint32_t inst);
        void (*decode2)(InstClass* obj, uint32_t inst);
    } *g_cls;

    struct InstClass {
        InstClassMethods * cls = g_cls;
        uint32_t r1;
        uint32_t r2;
    } self;

    M(&self, decode, inst);

    Type type = type_by_opcode[opcode >> 2];
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
