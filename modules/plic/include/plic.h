#ifndef MLDR187_SIMULATOR_PLIC_H
#define MLDR187_SIMULATOR_PLIC_H

#include "Device.h"
#include "MLDR187.h"

#define PLIC_N_INTERRUPTS 31

class PLIC_module : public Device {
public:
    explicit PLIC_module();

    uint32_t read(uint32_t addr, uint32_t len) noexcept(false) override;
    void write(uint32_t addr, uint32_t value, uint32_t len) noexcept(false) override;

    uint32_t get_start_addr() override;
    uint32_t get_end_addr() override;

    uint32_t get_priority() override;

    void reset() override;

    void tick() override;

private:
    uint32_t take_interrupt();
    void release_interrupt(uint32_t irqN);

    bool is_pending(uint32_t irqN);
    bool is_enabled(uint32_t irqN);

    void set_pending(uint32_t irqN, bool value);

    uint32_t priority[PLIC_N_INTERRUPTS] = {};
    uint32_t pending_mask = 0;
    uint32_t enable_mask = 0;
    uint32_t priority_threshold = 0;
};

#endif //MLDR187_SIMULATOR_PLIC_H
