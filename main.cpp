#include "Core.h"
#include "modules/libintelhex/include/intelhex.h"

void load_image_from_hex_file(const std::string &path) {
    auto * data = new intelhex::hex_data(path);
    for (auto & v : *data) {
        uint32_t addr = v.first;
        for (auto& c : v.second) {
            core.bus.write(addr++, c, 1);
        }
    }
    delete data;
}

int main(int argc, char* argv[]) {
    core.init();
    core.reset();
//    core.load_image_from_hex_file(std::string(argv[1]));
    while (true) {
        core.step();
    }

    return 0;
}
