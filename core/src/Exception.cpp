#include "Exception.h"

const char *Exception::what() const _GLIBCXX_TXN_SAFE_DYN _GLIBCXX_USE_NOEXCEPT {
    return name;
}

Exception::Exception(int code, const char *name) : code(code), name(name) {}
