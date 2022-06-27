#ifndef MLDR187_SIMULATOR_GPIO_H
#define MLDR187_SIMULATOR_GPIO_H

#include "Device.h"
#include "types.h"
#include <MLDR187.h>

#define MAX_PINS 16

class GPIO_module : public Device {
public:
    explicit GPIO_module(uint32_t base);

    uint32_t read(uint32_t addr, uint32_t len) noexcept(false) override;
    void write(uint32_t addr, uint32_t value, uint32_t len) noexcept(false) override;

    uint32_t get_start_addr() override;
    uint32_t get_end_addr() override;

    uint32_t get_priority() override;

    void reset() override;

    void set_callback(uint32_t pin, gpio_front_callback* callback);

private:
    void set_pin_mask(uint32_t mask);

    uint32_t pin_mask;
    gpio_front_callback* callbacks[MAX_PINS] = {};
    uint32_t base_addr;
    MDR_GPIO_TypeDef regs;
};


#endif //MLDR187_SIMULATOR_GPIO_H
