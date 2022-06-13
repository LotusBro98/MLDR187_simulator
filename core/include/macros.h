#ifndef MLDR187_SIMULATOR_MACROS_H
#define MLDR187_SIMULATOR_MACROS_H

#include <cassert>

#ifdef DEBUG
#define assert_param(x) assert(x)
#else
#define assert_param(x)
#endif

#endif //MLDR187_SIMULATOR_MACROS_H
