#ifndef REMOTE_BITBANG_H
#define REMOTE_BITBANG_H

#include <stdint.h>
#include <thread>

#include "jtag_dtm.h"

class remote_bitbang_t
{
public:
    // Create a new server, listening for connections from localhost on the given
    // port.
    remote_bitbang_t(uint16_t port, jtag_dtm_t *tap);

    // Do a bit of work.
    void tick();

    std::thread start();

private:
    jtag_dtm_t *tap;

    int socket_fd;
    int client_fd;

    static const ssize_t buf_size = 64 * 1024;
    char recv_buf[buf_size];
    ssize_t recv_start, recv_end;

    [[noreturn]] void thread_func();
    // Check for a client connecting, and accept if there is one.
    void accept();
    // Execute any commands the client has for us.
    void execute_commands();
};

#endif
