#include <cstdio>
#include <winsock2.h>
#include <windows.h>
#include <ws2tcpip.h>

#include "uart_socket.h"

static int sockInit()
{
#ifdef _WIN32
    WSADATA wsa_data;
    return WSAStartup(MAKEWORD(1,1), &wsa_data);
#else
    return 0;
#endif
}

UART_Socket::UART_Socket(uint32_t base_addr, uint32_t irqN, uint32_t port):
    UART_module(base_addr, irqN), client_fd(0)
{
    socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_fd == -1) {
        fprintf(stderr, "remote_bitbang failed to make socket: %s (%d)\n",
                strerror(errno), WSAGetLastError());
        abort();
    }

//    fcntl(socket_fd, F_SETFL, O_NONBLOCK);
    u_long iMode;
    int iResult = ioctlsocket(socket_fd, FIONBIO, &iMode);
    int reuseaddr = 1;
    if (setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, (char *)&reuseaddr,
                   sizeof(int)) == -1) {
        fprintf(stderr, "remote_bitbang failed setsockopt: %s (%d)\n",
                strerror(errno), WSAGetLastError());
        abort();
    }

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(port);

    if (bind(socket_fd, (struct sockaddr *) &addr, sizeof(addr)) == -1) {
        fprintf(stderr, "remote_bitbang failed to bind socket: %s (%d)\n",
                strerror(errno), WSAGetLastError());
        abort();
    }

    if (listen(socket_fd, 1) == -1) {
        fprintf(stderr, "remote_bitbang failed to listen on socket: %s (%d)\n",
                strerror(errno), WSAGetLastError());
        abort();
    }

    socklen_t addrlen = sizeof(addr);
    if (getsockname(socket_fd, (struct sockaddr *) &addr, &addrlen) == -1) {
        fprintf(stderr, "remote_bitbang getsockname failed: %s (%d)\n",
                strerror(errno), WSAGetLastError());
        abort();
    }

    printf("Listening for UART connection on port %d.\n",
           ntohs(addr.sin_port));
    fflush(stdout);
}

UART_Socket::~UART_Socket() {
    closesocket(socket_fd);
}

bool UART_Socket::accept()
{
    client_fd = ::accept(socket_fd, NULL, NULL);
    if (client_fd == -1) {
        if (errno == EAGAIN || WSAGetLastError() == WSAEWOULDBLOCK) {
            return false;
        } else {
            fprintf(stderr, "failed to accept on socket: %s (%d)\n", strerror(errno),
                    WSAGetLastError());
            abort();
        }
    }

//    fcntl(client_fd, F_SETFL, O_NONBLOCK);
    u_long iMode;
    int iResult = ioctlsocket(client_fd, FIONBIO, &iMode);
    if (iResult != 0) {
        fprintf(stderr, "failed to set nonblocking socket (%s) (%d)\n",
                strerror(errno), WSAGetLastError());
    }
    return true;
}

void UART_Socket::write_byte(uint8_t byte) {
    if (client_fd <= 0) {
        if (!this->accept())
            return;
    }

    ssize_t bytes = send(client_fd, (const char*)&byte, 1, 0);
    if (bytes == -1) {
        fprintf(stderr, "failed to write to socket: %s (%d)\n", strerror(errno), WSAGetLastError());
        abort();
    }
}

uint8_t UART_Socket::read_byte() {
    if (client_fd <= 0) {
        if (!this->accept())
            return 0;
    }

    uint8_t byte;
    ssize_t bytes = recv(client_fd, (char*)&byte, 1, 0);

    if (bytes == -1) {
        if (errno == EAGAIN || WSAGetLastError() == WSAEWOULDBLOCK) {
            return 0;
        } else {
            fprintf(stderr, "remote_bitbang failed to read on socket: %s (%d)\n",
                    strerror(errno), WSAGetLastError());
            abort();
        }
    }
}

bool UART_Socket::available() {
    if (client_fd <= 0) {
        static int cnt = 0;
        cnt = (cnt + 1) & 0xffff;
        if (cnt != 0)
            return false;
        if (!this->accept())
            return false;
    }

    u_long bytes_available;
    int iResult = ioctlsocket(client_fd, FIONREAD, &bytes_available);
    if (iResult != 0) {
        fprintf(stderr, "failed to check available bytes on socket (%s) (%d)\n",
                strerror(errno), WSAGetLastError());
        abort();
        return false;
    }

    return bytes_available > 0;
}
