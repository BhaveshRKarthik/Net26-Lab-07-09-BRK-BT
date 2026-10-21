#pragma once

#include <cstdint>
#include <netinet/in.h>
#include <string>
#include <optional>
#include <string_view>
#include <sys/socket.h>
#include <unistd.h>

#include "misc_types.hpp"
#include "msg.hpp"

namespace brk_bt {
    class IPv4TCPClient : immovable {
    public:
        IPv4TCPClient() : IPv4TCPClient(socket(PF_INET, SOCK_STREAM, 0)) {}
        ~IPv4TCPClient() { close(m_sock); }

    public:
        void connect(in_addr ip, std::uint16_t port) {
            sockaddr_in addr{ .sin_family = AF_INET, .sin_port = htons(port), .sin_addr = ip };
            test_fail(::connect(m_sock, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)), "Failed to connect");
        }

        std::optional<link_state> recv();

        void send(std::span<const std::byte> data);

    private:
        IPv4TCPClient(int sock) : m_sock(sock) { test_fail(sock, "Invalid Socket"); }

    private:
        int m_sock;
    };

} // namespace brk_bt
