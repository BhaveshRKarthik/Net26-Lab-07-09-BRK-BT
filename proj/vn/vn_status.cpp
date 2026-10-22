#include "vn_status.hpp"
#include "arg_parse.hpp"

namespace brk_bt {
    static uint32_t read_u32(std::span<std::byte> bytes) {
        return static_cast<uint32_t>(bytes[1]) << 24 | static_cast<uint32_t>(bytes[2]) << 16 | static_cast<uint32_t>(bytes[3]) << 8 | static_cast<uint32_t>(bytes[4]);
    }

    vn_status vn_status::from_binary(std::span<std::byte> bytes) {
        vn_status result{ .alpha = '\0', .udp_port = 0, .ip = { 0 }, .seq_no = 0, .neighbours = {} };
        while (!bytes.empty()) {
            char alpha       = static_cast<char>(bytes[0]);
            link_state_tup x = {
                .ip       = read_u32(bytes),
                .udp_port = static_cast<uint16_t>(static_cast<uint16_t>(bytes[5]) << 8 | static_cast<uint16_t>(bytes[6])),
                .cost     = read_u32(bytes.subspan(6)),
            };

            if (x.cost == 0) {
                if (result.alpha != '\0') throw exception_with_location("Repeated cost 0 case");
                result.alpha    = alpha;
                result.ip       = x.ip;
                result.udp_port = x.udp_port;
            } else {
                auto [_, b] = result.neighbours.try_emplace(alpha, x);
                if (!b) throw exception_with_location("Repeated alphabet");
            }
            bytes = bytes.subspan(11);
        }
        if (!result.alpha) throw exception_with_location("No node with cost 0 given in LINK-STATE");
        return result;
    }

    std::vector<std::byte> vn_status::to_binary() {
        ++seq_no;
        std::vector<std::byte> result{ std::byte(alpha), std::byte(seq_no >> 24), std::byte(seq_no >> 16), std::byte(seq_no >> 8), std::byte(seq_no) };
        result.reserve(5 + neighbours.size() * 5);
        for (auto [alpha, tup] : neighbours) {
            result.insert(result.end(),
                          { std::byte(alpha), std::byte(tup.cost >> 24), std::byte(tup.cost >> 16), std::byte(tup.cost >> 8), std::byte(tup.cost) });
        }
        return result;
    }

} // namespace brk_bt
