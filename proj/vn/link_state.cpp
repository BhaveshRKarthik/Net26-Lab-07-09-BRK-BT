#include "link_state.hpp"

namespace brk_bt {

    static uint32_t read_u32(std::span<std::byte> bytes) {
        return static_cast<uint32_t>(bytes[1]) << 24 | static_cast<uint32_t>(bytes[2]) << 16 | static_cast<uint32_t>(bytes[3]) << 8 | static_cast<uint32_t>(bytes[4]);
    }

    std::vector<std::byte> link_state::to_binary() const {
        std::vector<std::byte> result{ std::byte(src_alpha), std::byte(seq_no >> 24), std::byte(seq_no >> 16), std::byte(seq_no >> 8), std::byte(seq_no) };
        result.reserve(5 + 5 * neighbours.size());
        for (auto [alpha, cost] : neighbours) {
            result.insert(result.end(), { std::byte(alpha), std::byte(cost >> 24), std::byte(cost >> 16), std::byte(cost >> 8), std::byte(cost) });
        }
        return result;
    }

    bool link_state::from_binary(std::span<std::byte> bytes) {
        if (bytes.size() % 5 || bytes.empty()) throw exception_with_location("invalid link state size");

        uint32_t new_seq_no = read_u32(bytes);
        if (new_seq_no <= seq_no) return false;

        timeout = initial_timeout;
        src_alpha = static_cast<char>(bytes[0]);
        seq_no = new_seq_no;
        neighbours.clear();
        while (!(bytes = bytes.subspan(5)).empty()) {
            neighbours[static_cast<char>(bytes[0])] = read_u32(bytes);
        }
        return true;
    }
} // namespace brk_bt
