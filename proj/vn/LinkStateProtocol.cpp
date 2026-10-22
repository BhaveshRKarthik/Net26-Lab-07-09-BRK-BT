#pragma once

#include "LinkStateProtocol.hpp"

namespace brk_bt {

    LinkStateProtocol::LinkStateProtocol(in_addr ip, uint16_t udp_port) : LinkStateProtocol(socket(AF_INET, SOCK_DGRAM | SOCK_NONBLOCK, 0), ip, udp_port) {
        int fl = ::fcntl(m_sock, F_GETFL, 0);
        test_fail(fl, "fcntl failed");
        if (!(fl & O_NONBLOCK)) throw exception_with_location("Blocking socket handed to IPv4UDPPeer");

        sockaddr_in addr{ .sin_family = AF_INET, .sin_port = htons(m_port), .sin_addr = { htonl(INADDR_ANY) }, .sin_zero = { 0 } };

        test_fail(::bind(m_sock, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)), "Failed to bind to port");
    }

    void LinkStateProtocol::update(vn_status&& recvd) {
        if (recvd.alpha != m_status.alpha) throw exception_with_location("LINK-STATE has unexpected 0-cost alpha");
        if (recvd.ip.s_addr != m_status.ip.s_addr) throw exception_with_location("LINK-STATE has unexpected 0-cost ip");
        if (recvd.udp_port != m_status.udp_port) throw exception_with_location("LINK-STATE has unexpected 0-cost udp port");

        m_status.neighbours = std::move(recvd.neighbours);

        auto to_send_dgram = m_status.to_binary();

        for (auto [alpha, state] : m_status.neighbours) {
            sendto(to_send_dgram, state.ip, state.udp_port);
        }
    }

    void LinkStateProtocol::update() {
        sockaddr_in addr{};
        socklen_t addr_len = sizeof(addr);
        std::byte buf[65536];
        ssize_t recv_len = ::recvfrom(m_sock, buf, sizeof(buf), 0, reinterpret_cast<sockaddr*>(&addr), &addr_len);
        test_fail(recv_len, "Failed to recvfrom");

        auto dgram = std::span(buf, static_cast<size_t>(recv_len));
        auto dgram_src = static_cast<char>(buf[0]);

        if (m_graph[dgram_src].from_binary(dgram)) {
            for (auto [alpha, state] : m_status.neighbours) {
                if (state.ip.s_addr != addr.sin_addr.s_addr || state.udp_port != addr.sin_port)
                    sendto(dgram, state.ip, state.udp_port);
            }
        }
    }

    void LinkStateProtocol::sendto(std::span<std::byte> bytes, in_addr ip, uint16_t port) {
        sockaddr_in addr = {
            .sin_family = AF_INET,
            .sin_port   = port,
            .sin_addr   = ip,
            .sin_zero   = { 0 },
        };
        ::sendto(m_sock, bytes.data(), bytes.size(), MSG_NOSIGNAL, reinterpret_cast<const sockaddr*>(&addr), sizeof(addr));
    }

} // namespace brk_bt
