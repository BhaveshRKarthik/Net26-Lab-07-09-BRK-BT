#pragma once

#include "config.hpp"
#include <cstddef>
#include <cstdint>
#include <span>
#include <vector>

namespace brk_bt {
    struct lsp_tuple {
        char nbr_alpha;
        uint16_t udp_port;
        uint32_t ip; // ip below udp_port due to padding
        uint32_t cost;

        lsp_tuple(char NodeAlphabet, uint32_t IPAddress, uint16_t UDPPort, uint32_t Cost)
            : nbr_alpha(NodeAlphabet), udp_port(UDPPort), ip(IPAddress), cost(Cost) {}

        bool operator==(const lsp_tuple& other) const {
            return nbr_alpha == other.nbr_alpha &&
                   udp_port == other.udp_port &&
                   ip == other.ip &&
                   cost == other.cost;
        }
    };

    struct lsp_packet {
        std::vector<lsp_tuple> data;
        uint32_t seq_no;
        char origin_alpha;

        static lsp_packet from_binary(std::span<const std::byte> bytes);
        std::vector<std::byte> to_binary() const;
    };
} // namespace brk_bt
