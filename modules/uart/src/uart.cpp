#include "uart.h"
#include "Core.h"
#include <cstdlib>
#include <cstring>

UART_module::UART_module(uint32_t base_addr, uint32_t irqN):
    base_addr(base_addr),
    irqN(irqN),
    regs{},
    recv_buffer(16)
{

}

uint32_t UART_module::get_start_addr() {
    return base_addr;
}

uint32_t UART_module::get_end_addr() {
    return base_addr + sizeof(regs);
}

uint32_t UART_module::get_priority() {
    return 1;
}

void UART_module::reset() {
    memset(&regs, 0, sizeof(regs));
}

void UART_module::tick() {
    while (available() && !recv_buffer.full()) {
        recv_buffer.push(read_byte());
    }

    if (get_interrupt_flags() & regs.IMSC) {
        core.raise_peripheral_interrupt(irqN);
    }
}

uint32_t UART_module::read(uint32_t addr, uint32_t len) noexcept(false) {
    if (addr == offsetof(MDR_UART_TypeDef, DR))
        return recv_buffer.pop();
    else if (addr == offsetof(MDR_UART_TypeDef, FR))
        return get_status_flags();
    else if (addr == offsetof(MDR_UART_TypeDef, RIS))
        return get_interrupt_flags();
    else if (addr == offsetof(MDR_UART_TypeDef, MIS))
        return get_interrupt_flags() & regs.IMSC;
    else
        return read_from_mem(addr, len, &regs);
}

void UART_module::write(uint32_t addr, uint32_t value, uint32_t len) noexcept(false) {
    if (addr == offsetof(MDR_UART_TypeDef, DR))
        write_byte(value);
    else
        write_to_mem(addr, value, len, &regs);
}

uint32_t UART_module::get_status_flags() {
    uint32_t status = UART_FR_TXFE;
    status |= recv_buffer.empty() * UART_FR_RXFE;
    status |= recv_buffer.full() * UART_FR_RXFF;
    return status;
}

uint32_t UART_module::get_interrupt_flags() {
    uint32_t status = UART_IT_TFE | UART_IT_TNBSY | UART_IT_TX;
    status |= (!recv_buffer.empty()) * UART_IT_RNE;
    status |= recv_buffer.full() * UART_IT_RX;
    return status;
}
