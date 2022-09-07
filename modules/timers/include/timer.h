#ifndef MLDR187_SIMULATOR_TIMER_H
#define MLDR187_SIMULATOR_TIMER_H

#include <cstdint>

#include "Device.h"
#include "MLDR187.h"

class Timer : public Device {
public:

    explicit Timer(uint32_t base_addr, uint32_t irqN);

    uint32_t read(uint32_t addr, uint32_t len) noexcept(false) override;
    void write(uint32_t addr, uint32_t value, uint32_t len) noexcept(false) override;

    uint32_t get_start_addr() override;
    uint32_t get_end_addr() override;

    uint32_t get_priority() override;

    void reset() override;
    void tick() override;

private:
    uint32_t irqN;
    MDR_TIM_TypeDef regs;
    uint32_t base_addr;
};

#endif //MLDR187_SIMULATOR_TIMER_H
