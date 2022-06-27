#include <cstring>
#include "spi.h"

SPI_module::SPI_module(uint32_t base_addr):
    base_addr(base_addr),
    regs{},
    recv_buffer(8),
    cs_pin_cb(this)
{

}

uint32_t SPI_module::get_start_addr() {
    return base_addr;
}

uint32_t SPI_module::get_end_addr() {
    return base_addr + sizeof(regs);
}

uint32_t SPI_module::get_priority() {
    return 1;
}

void SPI_module::reset() {
    memset(&regs, 0, sizeof(regs));
}

void SPI_module::tick() {
}

uint32_t SPI_module::read(uint32_t addr, uint32_t len) noexcept(false) {
    if (addr == offsetof(MDR_SSP_TypeDef, DR))
        return recv_buffer.pop();
    else if (addr == offsetof(MDR_SSP_TypeDef, SR))
        return get_status_flags();
    else
        return read_from_mem(addr, len, &regs);
}

void SPI_module::write(uint32_t addr, uint32_t value, uint32_t len) noexcept(false) {
    if (addr == offsetof(MDR_SSP_TypeDef, DR))
        recv_buffer.push(exchange_bytes(value));
    else
        write_to_mem(addr, value, len, &regs);
}

uint8_t SPI_module::exchange_bytes(uint8_t byte) {
    return 0;
}

uint32_t SPI_module::get_status_flags() {
    uint32_t status = SSP_SR_TFE | SSP_SR_TNF;
    status |= (!recv_buffer.empty()) * SSP_SR_RNE;
    status |= recv_buffer.full() * SSP_SR_RFF;
    return status;
}

void SPI_module::on_cs_fall() {

}

void SPI_module::on_cs_rise() {

}

SPI_module::cs_pin_callback *SPI_module::get_cs_pin_cb() {
    return &cs_pin_cb;
}

SPI_module::cs_pin_callback::cs_pin_callback(SPI_module *spiModule) : spiModule(spiModule) {}

void SPI_module::cs_pin_callback::operator()(uint32_t base, uint32_t pin, bool rising) {
    if (rising) {
        spiModule->on_cs_rise();
    } else {
        spiModule->on_cs_fall();
    }
}
