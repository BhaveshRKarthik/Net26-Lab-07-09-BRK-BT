#pragma once

#include <cstddef>
#include <cstdint>
#include <map>
#include <span>
#include <vector>

#include "misc_types.hpp"

namespace brk_bt {
    struct link_state {
        char src_alpha;
        std::uint32_t seq_no;
        std::map<char, uint32_t> neighbours;

        std::vector<std::byte> to_binary() const;

        void from_binary(std::span<std::byte> raw);
    };
} // namespace brk_bt
