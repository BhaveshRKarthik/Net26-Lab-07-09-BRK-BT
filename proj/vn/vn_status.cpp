#include "vn_status.hpp"
#include "arg_parse.hpp"

extern brk_bt::config config;

namespace brk_bt {
    vn_status vn_status::from_binary(std::span<std::byte> bytes) {
        vn_status result{ .alpha = '\0', .neighbours = {} };
        while (!bytes.empty()) {
            char alpha       = static_cast<char>(bytes[0]);
            link_state_tup x = {
                .ip       = static_cast<uint32_t>(bytes[1] << 24 | bytes[2] << 16 | bytes[3] << 8 | bytes[4]),
                .udp_port = static_cast<uint16_t>(bytes[5] << 8 | bytes[6]),
                .cost     = static_cast<uint32_t>(bytes[7] << 24 | bytes[8] << 16 | bytes[9] << 8 | bytes[10]),
            };

            if (x.cost == 0) {
                if (result.alpha != '\0') throw exception_with_location("Repeated cost 0 case");
                result.alpha = alpha;
                if (x.ip.s_addr != ::config.vn_ip.s_addr && x.udp_port != ::config.vn_port) throw exception_with_location("ON sent cost 0 with wrong IP/port");
            } else {
                auto [_, b] = result.neighbours.try_emplace(alpha, x);
                if (!b) throw exception_with_location("Repeated alphabet");
            }
            bytes = bytes.subspan(11);
        }
        if (!result.alpha) throw exception_with_location("No node with cost 0 given in LINK-STATE");
        return result;
    }

} // namespace brk_bt
