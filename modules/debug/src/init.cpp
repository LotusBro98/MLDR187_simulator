
#include "../include/debug_module.h"
#include "../include/jtag_dtm.h"
#include "../include/remote_bitbang.h"

debug_module_t debugModule(debug_module_config_t{15, 32, false, 20, false, true, false, false});
jtag_dtm_t jtagDtm(&debugModule, 20);
remote_bitbang_t remoteBitbang(2442, &jtagDtm);
