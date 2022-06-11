#ifndef MLDR187_SIMULATOR_ENUMS_H
#define MLDR187_SIMULATOR_ENUMS_H

enum Register {
    REG_ZERO = 0,
    REG_RA,
    REG_SP,
    REG_GP,
    REG_TP,
    REG_T0,
    REG_T1,
    REG_T2,
    REG_S0,
    REG_S1,
    REG_A0,
    REG_A1,
    REG_A2,
    REG_A3,
    REG_A4,
    REG_A5,
    REG_A6,
    REG_A7,
    REG_S2,
    REG_S3,
    REG_S4,
    REG_S5,
    REG_S6,
    REG_S7,
    REG_S8,
    REG_S9,
    REG_S10,
    REG_S11,
    REG_T3,
    REG_T4,
    REG_T5,
    REG_T6
};

enum Exception {
    EXC_IAM = 0, //Instruction address misaligned
    EXC_IAF, //Instruction access fault
    EXC_II, //Illegal instruction
    EXC_BREAK, //Breakpoint
    EXC_LAM, //Load address misaligned
    EXC_LAF, //Load access fault
    EXC_SAM, //Store/AMO address misaligned
    EXC_SAF, //Store/AMO access fault
    EXC_ECALLU, //Environment call from U-mode
    EXC_ECALLS, //Environment call from S-mode
    EXC_RES_10, //Reserved
    EXC_ECALLM, //Environment call from M-mode
    EXC_IPF, //Instruction page fault
    EXC_LPF, //Load page fault
    EXC_RES_14, //Reserved
    EXC_SPF, //Store/AMO page fault
};

#endif //MLDR187_SIMULATOR_ENUMS_H
