#pragma once

#include "link_state.hpp"
#include "misc_types.hpp"
#include "vn_status.hpp"
#include <sys/fcntl.h>
#include <sys/socket.h>
#include <sys/unistd.h>

namespace brk_bt {

    class LinkStateProtocol : immovable {
    public:
        LinkStateProtocol(in_addr ip, uint16_t udp_port);
        ~LinkStateProtocol() { ::close(m_sock); }

        void update(vn_status&&);
        void sendto(std::span<std::byte> bytes, in_addr ip, uint16_t port);
        void update();

        int fd() const { return m_sock; }

    private:
        LinkStateProtocol(int sock, in_addr ip, uint16_t udp_port) : m_sock(sock), m_ip(ip), m_port(udp_port) {
            test_fail(sock, "Failed to create UDP socket");
        }

        int m_sock;
        in_addr m_ip;
        uint16_t m_port;

        std::map<char, link_state> m_graph;
        vn_status m_status = {};
    };

} // namespace brk_bt
