#ifndef MLDR187_SIMULATOR_EXCEPTION_H
#define MLDR187_SIMULATOR_EXCEPTION_H

//#include "enums.h"

#include <exception>
#include <cstdint>

class Exception : public std::exception {
public:
    Exception(int code, const char * name);
    const char *what() const _GLIBCXX_TXN_SAFE_DYN _GLIBCXX_USE_NOEXCEPT override;
    const uint32_t code;
private:
    const char * name;
};

#define EXC_IAM     Exception(0, "Instruction address misaligned");
#define EXC_IAF     Exception(1, "Instruction access fault");
#define EXC_II      Exception(2, "Illegal instruction");
#define EXC_BREAK   Exception(3, "Breakpoint");
#define EXC_LAM     Exception(4, "Load address misaligned");
#define EXC_LAF     Exception(5, "Load access fault");
#define EXC_SAM     Exception(6, "Store/AMO address misaligned");
#define EXC_SAF     Exception(7, "Store/AMO access fault");
#define EXC_ECALLU  Exception(8, "Environment call from U-mode");
#define EXC_ECALLS  Exception(9, "Environment call from S-mode");
#define EXC_RES_10  Exception(10, "Reserved");
#define EXC_ECALLM  Exception(11, "Environment call from M-mode");
#define EXC_IPF     Exception(12, "Instruction page fault");
#define EXC_LPF     Exception(13, "Load page fault");
#define EXC_RES_14  Exception(14, "Reserved");
#define EXC_SPF     Exception(15, "Store/AMO page fault");

#endif //MLDR187_SIMULATOR_EXCEPTION_H
