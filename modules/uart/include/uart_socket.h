#ifndef MLDR187_SIMULATOR_UART_SOCKET_H
#define MLDR187_SIMULATOR_UART_SOCKET_H

#include "uart.h"

class UART_Socket : public UART_module {
public:
    UART_Socket(uint32_t base_addr, uint32_t irqN, uint32_t port);
    ~UART_Socket() override;

private:
    int socket_fd;
    int client_fd;

    bool accept();

    void write_byte(uint8_t byte) override;
    bool available() override;
    uint8_t read_byte() override;
};

#endif //MLDR187_SIMULATOR_UART_SOCKET_H
