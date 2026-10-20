#include "msg.hpp"
#include "misc_types.hpp"

namespace brk_bt {

    lsp_packet lsp_packet::from_binary(std::span<const std::byte> bytes) {
        uint32_t off      = 0;
        char origin_alpha = static_cast<char>(bytes[off++]);
        uint32_t seq_no   = (static_cast<uint32_t>(bytes[off + 0]) << 24) + (static_cast<uint32_t>(bytes[off + 1]) << 16) +
                            (static_cast<uint32_t>(bytes[off + 2]) << 8) + static_cast<uint32_t>(bytes[off + 3]);
        off += 4;
        lsp_packet pkt{ .seq_no = seq_no, .origin_alpha = origin_alpha };
        auto& data = pkt.data.tuples;

        unsigned char N = static_cast<unsigned char>(bytes[off++]);
        if (N >= 26) throw exception_with_location("Unexpected length");
        static_assert(sizeof(ls_tuple) == 12);
        if (bytes.size() != 1 + 4 + 1 + 11 * N) throw exception_with_location("Unexpected packet size");

        data.reserve(N);

        for (int i = 0; i < N; ++i) {
            char nbr_alpha    = static_cast<char>(bytes[off++]);
            uint16_t udp_port = (static_cast<uint16_t>(bytes[off + 0]) << 8) + static_cast<uint16_t>(bytes[off + 1]);
            off += 2;
            uint32_t ip = (static_cast<uint32_t>(bytes[off + 0]) << 24) + (static_cast<uint32_t>(bytes[off + 1]) << 16) +
                          (static_cast<uint32_t>(bytes[off + 2]) << 8) + static_cast<uint32_t>(bytes[off + 3]);
            off += 4;
            uint32_t cost = (static_cast<uint32_t>(bytes[off + 0]) << 24) + (static_cast<uint32_t>(bytes[off + 1]) << 16) +
                            (static_cast<uint32_t>(bytes[off + 2]) << 8) + static_cast<uint32_t>(bytes[off + 3]);
            off += 4;
            data.push_back({ nbr_alpha, ip, udp_port, cost });
        }


        return pkt;
    }

    std::vector<std::byte> lsp_packet::to_binary() const {
        std::vector<std::byte> bin;
        const uint32_t N = data.size();
        bin.reserve(1 + 4 + 1 + 11 * N);

        bin.push_back(static_cast<std::byte>(origin_alpha));

        bin.push_back(static_cast<std::byte>((seq_no >> 24) & 0xff));
        bin.push_back(static_cast<std::byte>((seq_no >> 16) & 0xff));
        bin.push_back(static_cast<std::byte>((seq_no >> 8) & 0xff));
        bin.push_back(static_cast<std::byte>((seq_no >> 0) & 0xff));

        bin.push_back(static_cast<std::byte>(N));

        for (int i = 0; i < N; ++i) {
            const auto& tuple = data.tuples[i];
            bin.push_back(static_cast<std::byte>(tuple.nbr_alpha));

            bin.push_back(static_cast<std::byte>((tuple.udp_port >> 8) & 0xff));
            bin.push_back(static_cast<std::byte>((tuple.udp_port >> 0) & 0xff));

            bin.push_back(static_cast<std::byte>((tuple.ip >> 24) & 0xff));
            bin.push_back(static_cast<std::byte>((tuple.ip >> 16) & 0xff));
            bin.push_back(static_cast<std::byte>((tuple.ip >> 8) & 0xff));
            bin.push_back(static_cast<std::byte>((tuple.ip >> 0) & 0xff));

            bin.push_back(static_cast<std::byte>((tuple.cost >> 24) & 0xff));
            bin.push_back(static_cast<std::byte>((tuple.cost >> 16) & 0xff));
            bin.push_back(static_cast<std::byte>((tuple.cost >> 8) & 0xff));
            bin.push_back(static_cast<std::byte>((tuple.cost >> 0) & 0xff));
        }

        return bin;
    }

} // namespace brk_bt
