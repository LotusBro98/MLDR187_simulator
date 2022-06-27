#include <cstring>
#include "GPIO.h"
#include "macros.h"

#define BIT(x, i) ((x >> (i)) & 1)
#define SET_BIT(x, i, b) x = (x & ~(1 << (i))) | ((b) << (i))

GPIO_module::GPIO_module(uint32_t base) : Device(), regs(), base_addr(base) {

}

uint32_t GPIO_module::get_start_addr() {
    return base_addr;
}

uint32_t GPIO_module::get_end_addr() {
    return base_addr + sizeof(regs);
}

uint32_t GPIO_module::get_priority() {
    return 1;
}

void GPIO_module::reset() {
    memset(&regs, 0, sizeof(regs));
}

uint32_t GPIO_module::read(uint32_t addr, uint32_t len) noexcept(false) {
    assert_param(len == 4);

    if (addr == offsetof(MDR_GPIO_TypeDef, RXTX) ||
        addr == offsetof(MDR_GPIO_TypeDef, SETTX) ||
        addr == offsetof(MDR_GPIO_TypeDef, CLRTX) ||
        addr == offsetof(MDR_GPIO_TypeDef, RDTX))
    {
        return pin_mask;
    } else {
        return read_from_mem(addr, len, &regs);
    }
}

void GPIO_module::write(uint32_t addr, uint32_t value, uint32_t len) noexcept(false) {
    assert_param(len == 4);

    if (addr == offsetof(MDR_GPIO_TypeDef, RXTX)) {
        set_pin_mask(value);
    } else if (addr == offsetof(MDR_GPIO_TypeDef, SETTX)) {
        set_pin_mask(pin_mask | value);
    } else if (addr == offsetof(MDR_GPIO_TypeDef, CLRTX)) {
        set_pin_mask(pin_mask & ~value);
    } else {
        write_to_mem(addr, value, len, &regs);
    }
}

void GPIO_module::set_callback(uint32_t pin, gpio_front_callback* callback) {
    assert_param(pin < MAX_PINS);
    callbacks[pin] = callback;
}

void GPIO_module::set_pin_mask(uint32_t mask) {
    uint32_t change = pin_mask ^ mask;
    for (int i = 0; i < MAX_PINS; i++) {
        if (!BIT(change, i))
            continue;
        gpio_front_callback* callback = callbacks[i];
        if (callback == nullptr)
            continue;
        bool rising = BIT(mask, i);
        (*callback)(base_addr, i, rising);
    }
    pin_mask = mask;
}
