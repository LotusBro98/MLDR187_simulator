#include <Core.h>
#include <iostream>
#include "../debug/include/remote_bitbang.h"

int main(int argc, char* argv[]) {
    Core core;
    debug_module_t debugModule(&core, debug_module_config_t{16, 32, false, 1, false, true, false, false});
    jtag_dtm_t jtagDtm(&debugModule, 1);
    remote_bitbang_t remoteBitbang(2442, &jtagDtm);

    std::thread debug_thread = remoteBitbang.start();

//    core.load_image_from_hex_file(std::string(argv[1]));
    while (true) {
        core.step();
    }

    debug_thread.join();

    std::cout << "Hello, World!" << std::endl;
    return 0;
}
