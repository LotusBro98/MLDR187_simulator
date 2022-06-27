#ifndef MLDR187_SIMULATOR_SPI_H
#define MLDR187_SIMULATOR_SPI_H

#include "Device.h"
#include "MLDR187.h"
#include "FIFO.h"
#include "types.h"

class SPI_module : public Device {
public:
    class cs_pin_callback : public gpio_front_callback {
    public:
        explicit cs_pin_callback(SPI_module *spiModule);

        void operator()(uint32_t base, uint32_t pin, bool rising) override;

    private:
        SPI_module * spiModule;
    };

    explicit SPI_module(uint32_t base_addr);

    uint32_t read(uint32_t addr, uint32_t len) noexcept(false) override;
    void write(uint32_t addr, uint32_t value, uint32_t len) noexcept(false) override;

    uint32_t get_start_addr() override;
    uint32_t get_end_addr() override;

    uint32_t get_priority() override;

    void reset() override;
    void tick() override;

    cs_pin_callback * get_cs_pin_cb();

private:
    virtual uint8_t exchange_bytes(uint8_t byte);
    virtual void on_cs_fall();
    virtual void on_cs_rise();

    cs_pin_callback cs_pin_cb;

    uint32_t get_status_flags();

    MDR_SSP_TypeDef regs;
    uint32_t base_addr;
    FIFO recv_buffer;
};

#endif //MLDR187_SIMULATOR_SPI_H
