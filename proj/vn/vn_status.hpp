#pragma once

#include "misc_types.hpp"
#include <cstddef>
#include <map>
#include <netinet/in.h>
#include <span>

namespace brk_bt {

    struct link_state_tup {
        in_addr ip;
        uint16_t udp_port;
        uint32_t cost;
    };

    struct vn_status {
        char alpha;
        std::map<char, link_state_tup> neighbours;

        static vn_status from_binary(std::span<std::byte> bytes);
    };

} // namespace brk_bt
