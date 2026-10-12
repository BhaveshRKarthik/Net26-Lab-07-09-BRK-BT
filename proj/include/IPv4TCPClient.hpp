#pragma once

#include <cstdint>
#include <netinet/in.h>
#include <string>
#include <string_view>
#include <sys/socket.h>
#include <unistd.h>

#include "misc_types.hpp"

namespace brk_bt {
    class IPv4TCPClient :immovable {
    public:
        IPv4TCPClient() : IPv4TCPClient(socket(PF_INET, SOCK_STREAM, 0)) {}
        ~IPv4TCPClient() { close(m_sock); }

    public:
        void connect(in_addr ip, std::uint16_t port) {
            sockaddr_in addr{ .sin_family = AF_INET, .sin_port = htons(port), .sin_addr = ip };
            test_fail(::connect(m_sock, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)), "Failed to connect");
        }

        std::string recv(int flags = 0);

        void send(std::string_view data, int flags = 0);

    protected:
        friend class IPv4TCPServer;
        IPv4TCPClient(int sock) : m_sock(sock) { test_fail(sock, "Invalid Socket"); }

    private:
        int m_sock;
    };

} // namespace brk_bt
