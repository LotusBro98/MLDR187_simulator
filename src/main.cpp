#include <Core.h>
#include <iostream>

int main(int argc, char* argv[]) {
    sim::Core core;

    core.load_image_from_hex_file(std::string(argv[1]));
    while (true) {
        core.step();
    }

    std::cout << "Hello, World!" << std::endl;
    return 0;
}
