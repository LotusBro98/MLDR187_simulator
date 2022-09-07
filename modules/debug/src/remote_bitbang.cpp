#ifdef _WIN32
#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0501  /* Windows XP. */
#endif
#include <winsock2.h>
#include <Ws2tcpip.h>
#define SOCK_NONBLOCK FIONBIO
#else
/* Assume that any non-Windows platform uses POSIX-style sockets instead. */
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>  /* Needed for getaddrinfo() and freeaddrinfo() */
#include <unistd.h> /* Needed for close() */
#endif

#include <cerrno>
#include <fcntl.h>
#include <cstdlib>
#include <cstring>
#include <unistd.h>

#include <algorithm>
#include <cassert>
#include <cstdio>
#include <thread>

#include "remote_bitbang.h"

#if 1
#  define D(x) x
#else
#  define D(x)
#endif

/////////// remote_bitbang_t

int sockInit(void)
{
#ifdef _WIN32
    WSADATA wsa_data;
    return WSAStartup(MAKEWORD(1,1), &wsa_data);
#else
    return 0;
#endif
}

int sockQuit(void)
{
#ifdef _WIN32
    return WSACleanup();
#else
    return 0;
#endif
}

remote_bitbang_t::remote_bitbang_t(uint16_t port, jtag_dtm_t *tap) :
        tap(tap),
        socket_fd(0),
        client_fd(0),
        recv_start(0),
        recv_end(0)
{
    sockInit();

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

    printf("Listening for remote bitbang connection on port %d.\n",
           ntohs(addr.sin_port));
    fflush(stdout);

//    bitbang_thread = new std::thread(&remote_bitbang_t::thread_func, *this);
}

remote_bitbang_t::~remote_bitbang_t() {
//    stop_request = true;
//    bitbang_thread->join();
//    delete bitbang_thread;
}

void remote_bitbang_t::thread_func() {
    while (!stop_request) {
        tick();
    }
}

void remote_bitbang_t::accept()
{
    client_fd = ::accept(socket_fd, NULL, NULL);
    if (client_fd == -1) {
        if (errno == EAGAIN || WSAGetLastError() == WSAEWOULDBLOCK) {
            // No client waiting to connect right now.
        } else {
            fprintf(stderr, "failed to accept on socket: %s (%d)\n", strerror(errno),
                    WSAGetLastError());
            abort();
        }
    } else {
//        fcntl(client_fd, F_SETFL, O_NONBLOCK);
        u_long iMode;
        int iResult = ioctlsocket(client_fd, FIONBIO, &iMode);
        if (iResult != 0) {
            fprintf(stderr, "failed to set nonblocking socket (%s) (%d)\n",
                    strerror(errno), WSAGetLastError());
        }
    }
}

void remote_bitbang_t::tick()
{
    if (client_fd > 0) {
        execute_commands();
    } else {
        static int cnt = 0;
        cnt = (cnt + 1) & 0xffff;
        if (cnt == 0)
            this->accept();
    }
}

void remote_bitbang_t::execute_commands()
{
    static char send_buf[buf_size];
    unsigned total_processed = 0;
    bool quit = false;
    bool in_rti = tap->state() == RUN_TEST_IDLE;
    bool entered_rti = false;
    while (1) {
        if (recv_start < recv_end) {
            unsigned send_offset = 0;
            while (recv_start < recv_end) {
                uint8_t command = recv_buf[recv_start];

                switch (command) {
                    case 'B': /* fprintf(stderr, "*BLINK*\n"); */ break;
                    case 'b': /* fprintf(stderr, "_______\n"); */ break;
                    case 'r': tap->reset(); break;
                    case '0': tap->set_pins(0, 0, 0); break;
                    case '1': tap->set_pins(0, 0, 1); break;
                    case '2': tap->set_pins(0, 1, 0); break;
                    case '3': tap->set_pins(0, 1, 1); break;
                    case '4': tap->set_pins(1, 0, 0); break;
                    case '5': tap->set_pins(1, 0, 1); break;
                    case '6': tap->set_pins(1, 1, 0); break;
                    case '7': tap->set_pins(1, 1, 1); break;
                    case 'R': send_buf[send_offset++] = tap->tdo() ? '1' : '0'; break;
                    case 'Q': quit = true; break;
                    default:
                        fprintf(stderr, "remote_bitbang got unsupported command '%c'\n",
                                command);
                }
                recv_start++;
                total_processed++;
                if (!in_rti && tap->state() == RUN_TEST_IDLE) {
                    entered_rti = true;
                    break;
                }
                in_rti = false;
            }
            unsigned sent = 0;
            while (sent < send_offset) {
                ssize_t bytes = send(client_fd, send_buf + sent, send_offset, 0);
                if (bytes == -1) {
                    fprintf(stderr, "failed to write to socket: %s (%d)\n", strerror(errno), WSAGetLastError());
                    abort();
                }
                sent += bytes;
            }
        }

        if (total_processed > buf_size || quit || entered_rti) {
            // Don't go forever, because that could starve the main simulation.
            break;
        }

        recv_start = 0;
        recv_end = recv(client_fd, recv_buf, buf_size, 0);

        if (recv_end == -1) {
            if (errno == EAGAIN || WSAGetLastError() == WSAEWOULDBLOCK) {
                break;
            } else {
                fprintf(stderr, "remote_bitbang failed to read on socket: %s (%d)\n",
                        strerror(errno), WSAGetLastError());
                abort();
            }
        }

        if (quit) {
            fprintf(stderr, "Remote Bitbang received 'Q'\n");
        }

        if (recv_end == 0 || quit) {
            // The remote disconnected.
            fprintf(stderr, "Received nothing. Quitting.\n");
            close(client_fd);
            client_fd = 0;
            break;
        }
    }
}

