#ifndef MLDR187_SIMULATOR_BKP_H
#define MLDR187_SIMULATOR_BKP_H

#include "Device.h"
#include "MLDR187.h"

class BKP_module : public Device {
public:
    explicit BKP_module();

    uint32_t read(uint32_t addr, uint32_t len) noexcept(false) override;
    void write(uint32_t addr, uint32_t value, uint32_t len) noexcept(false) override;

    uint32_t get_start_addr() override;
    uint32_t get_end_addr() override;

    uint32_t get_priority() override;

    void reset() override;
private:
    MDR_BKP_TypeDef regs;
};

#endif //MLDR187_SIMULATOR_BKP_H
