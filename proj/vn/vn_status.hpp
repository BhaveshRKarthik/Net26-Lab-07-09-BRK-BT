#pragma once

#include "misc_types.hpp"
#include <cstddef>
#include <map>
#include <netinet/in.h>
#include <span>
#include <vector>

namespace brk_bt {

    struct link_state_tup {
        uint32_t ip;
        uint16_t udp_port;
        uint32_t cost;
    };

    struct vn_status {
        char alpha;
        uint16_t udp_port;
        uint32_t ip;
        uint32_t seq_no;
        std::map<char, link_state_tup> neighbours;

        static vn_status from_binary(std::span<std::byte> bytes);
        std::vector<std::byte> to_binary();
    };

} // namespace brk_bt
