
#include "Core.h"

Core core = {};

Memory MEM_FLASH    (0x10000000, 0x40000);
Memory MEM_RAM_TCMA (0x80000000, 0x10000);
Memory MEM_RAM_TCMB (0x80010000, 0x8000);
Memory MEM_RAM_AHB  (0x20000000, 0x4000);
