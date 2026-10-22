#include "link_state.hpp"

namespace brk_bt {

    std::vector<std::byte> link_state::to_binary() const {
        std::vector<std::byte> result{ std::byte(src_alpha), std::byte(seq_no >> 24), std::byte(seq_no >> 16), std::byte(seq_no >> 8), std::byte(seq_no) };
        result.reserve(5 + 5 * neighbours.size());
        for (auto [alpha, cost] : neighbours) {
            result.insert(result.end(), { std::byte(alpha), std::byte(cost >> 24), std::byte(cost >> 16), std::byte(cost >> 8), std::byte(cost) });
        }
        return result;
    }

    bool link_state::from_binary(std::span<std::byte> bytes) {
        if (bytes.size() % 5) throw exception_with_location("link state size is always a multiple of 5");

        uint32_t new_seq_no = static_cast<uint32_t>(bytes[1] << 24 | bytes[2] << 16 | bytes[3] << 8 | bytes[4]);
        if (new_seq_no <= seq_no) return false;

        src_alpha = static_cast<char>(bytes[0]);
        seq_no = new_seq_no;
        neighbours.clear();
        while (!(bytes = bytes.subspan(5)).empty()) {
            neighbours[static_cast<char>(bytes[0])] = static_cast<uint32_t>(bytes[1] << 24 | bytes[2] << 16 | bytes[3] << 8 | bytes[4]);
        }
        return true;
    }
} // namespace brk_bt
