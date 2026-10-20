#pragma once

#include "config.hpp"
#include "misc_types.hpp"
#include <cstddef>
#include <cstdint>
#include <span>
#include <vector>

namespace brk_bt {

    struct ls_tuple {
        char nbr_alpha;
        uint16_t udp_port;
        uint32_t ip; // ip below udp_port due to padding
        uint32_t cost;

        ls_tuple(char NodeAlphabet, uint32_t IPAddress, uint16_t UDPPort, uint32_t Cost)
            : nbr_alpha(NodeAlphabet), udp_port(UDPPort), ip(IPAddress), cost(Cost) {}

        bool operator==(const ls_tuple& other) const {
            return nbr_alpha == other.nbr_alpha && udp_port == other.udp_port && ip == other.ip && cost == other.cost;
        }
    };


    struct link_state {
        std::vector<ls_tuple> tuples;

        link_state() = default;
        link_state(std::vector<ls_tuple> x) : tuples(std::move(x)) {}

        constexpr auto size() const { return tuples.size(); }

        static link_state from_binary(std::span<const std::byte> bytes) {
            if (bytes.size() % 11 != 0) throw exception_with_location("Unexpected size");

            const uint32_t N = bytes.size() / 11;
            link_state ls;
            auto& tuples = ls.tuples;
            tuples.reserve(N);

            uint32_t off = 0;
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
                tuples.push_back({ nbr_alpha, ip, udp_port, cost });
            }

            return ls;
        }

        std::vector<std::byte> to_binary() const {
            std::vector<std::byte> bin;
            const uint32_t N = size();

            for (int i = 0; i < N; ++i) {
                const auto& tuple = tuples[i];
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
    };

    struct connect_tuple {
        uint32_t ip;
        uint16_t udp_port;

        static connect_tuple from_binary(std::span<const std::byte> bytes) {
            connect_tuple ct;
            if (bytes.size() != sizeof(uint32_t) + sizeof(uint16_t)) throw exception_with_location("Unexpected size");

            ct.ip = (static_cast<uint32_t>(bytes[0]) << 24) + (static_cast<uint32_t>(bytes[1]) << 16) + (static_cast<uint32_t>(bytes[2]) << 8) +
                    static_cast<uint32_t>(bytes[3]);

            ct.udp_port = (static_cast<uint16_t>(bytes[4]) << 8) + static_cast<uint16_t>(bytes[5]);

            return ct;
        }

        std::vector<std::byte> to_binary() const {
            return { static_cast<std::byte>((ip >> 24) & 0xff), static_cast<std::byte>((ip >> 16) & 0xff),      static_cast<std::byte>((ip >> 8) & 0xff),
                     static_cast<std::byte>(ip & 0xff),         static_cast<std::byte>((udp_port >> 8) & 0xff), static_cast<std::byte>(udp_port & 0xff) };
        }
    };

    struct lsp_packet {
        char origin_alpha;
        link_state data;
        uint32_t seq_no;

        static lsp_packet from_binary(std::span<const std::byte> bytes);
        std::vector<std::byte> to_binary() const;
    };
} // namespace brk_bt
