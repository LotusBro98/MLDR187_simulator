#ifndef MLDR187_SIMULATOR_UART_H
#define MLDR187_SIMULATOR_UART_H

#include "Device.h"
#include "MLDR187.h"
#include "FIFO.h"

class UART_module : public Device {
public:

    explicit UART_module(uint32_t base_addr, uint32_t irqN);

    uint32_t read(uint32_t addr, uint32_t len) noexcept(false) override;
    void write(uint32_t addr, uint32_t value, uint32_t len) noexcept(false) override;

    uint32_t get_start_addr() override;
    uint32_t get_end_addr() override;

    uint32_t get_priority() override;

    void reset() override;
    void tick() override;

private:
    virtual void write_byte(uint8_t byte) = 0;
    virtual bool available() = 0;
    virtual uint8_t read_byte() = 0;

    uint32_t get_status_flags();
    uint32_t get_interrupt_flags();

    uint32_t irqN;
    MDR_UART_TypeDef regs;
    uint32_t base_addr;
    FIFO recv_buffer;
};

#endif //MLDR187_SIMULATOR_UART_H
