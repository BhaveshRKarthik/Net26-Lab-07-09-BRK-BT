#pragma once

#include <cstddef>
#include <cstdint>
#include <map>
#include <span>
#include <vector>

#include "misc_types.hpp"

namespace brk_bt {
    struct link_state {
        constexpr static uint8_t initial_timeout = 12;

        char src_alpha;
        uint8_t timeout = initial_timeout;
        std::uint32_t seq_no;
        std::map<char, uint32_t> neighbours;

        std::vector<std::byte> to_binary() const;

        bool from_binary(std::span<std::byte> raw);
    };
} // namespace brk_bt
