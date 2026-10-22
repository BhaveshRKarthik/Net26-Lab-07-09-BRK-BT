#pragma once

#include "link_state.hpp"
#include "misc_types.hpp"
#include "vn_status.hpp"
#include <map>
#include <netinet/in.h>
#include <span>
#include <unistd.h>
#include <vector>

namespace brk_bt {

    class LinkStateProtocol : immovable {
    public:
        LinkStateProtocol(uint32_t ip, uint16_t udp_port);
        ~LinkStateProtocol() { ::close(m_sock); }

        void update(vn_status&&);
        void sendto(std::span<const std::byte> bytes, uint32_t ip, uint16_t port);
        void update();

        int fd() const { return m_sock; }

        void dump();
        void decr_counters();

    private:
        LinkStateProtocol(int sock, uint32_t ip, uint16_t udp_port)
            : m_sock(sock), m_status{ .alpha = '\0', .udp_port = udp_port, .ip = ip, .seq_no = 0, .neighbours = {} } {
            test_fail(sock, "Failed to create UDP socket");
        }

        int m_sock;

        std::map<char, link_state> m_graph;
        vn_status m_status = {};
    };

} // namespace brk_bt
