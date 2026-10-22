#pragma once

#include "misc_types.hpp"
#include <cstddef>
#include <cstdint>
#include <vector>


namespace brk_bt {

    struct connect_msg {
        uint32_t ip;
        uint16_t port;
        std::vector<std::byte> to_binary() {
            return { std::byte(ip >> 24), std::byte(ip >> 16), std::byte(ip >> 8), std::byte(ip), std::byte(port >> 8), std::byte(port) };
        }
    };

} // namespace brk_bt
