#include "arg_parse.hpp"
#include "misc_types.hpp"

#include <arpa/inet.h>
#include <ifaddrs.h>
#include <iostream>
#include <string_view>
#include <format>

namespace brk_bt {

    void help_options() {
        std::cout << "./vn [OPTIONS]\n"
                     "\t--on-ip\n"
                     "\t--on-port\n"
                     "\t--vn-ip\n"
                     "\t--vn-port\n"
                     "\t--help\n";
    }

    uint32_t get_interface_ip(std::string_view ifname) {
        ifaddrs* list = nullptr;
        test_fail(getifaddrs(&list), "getifaddrs failed");
        defer {
            freeifaddrs(list);
        };

        for (ifaddrs* ifa = list; ifa; ifa = ifa->ifa_next) {
            if (!ifa->ifa_addr || ifa->ifa_addr->sa_family != AF_INET) continue;
            if (ifa->ifa_name != ifname) continue;

            auto* sin = reinterpret_cast<sockaddr_in*>(ifa->ifa_addr);
            return ntohl(sin->sin_addr.s_addr);
        }

        throw exception_with_location("Unknown Interface");
    }

    std::optional<config> parse_argv(int argc, char* argv[]) {
        using namespace std::string_view_literals;

        std::optional<config> cfg = std::nullopt;

        if (argc == 1) {
            return cfg;
        }
        if (argc == 2 && argv[1] == "--help"sv) {
            return cfg;
        }

        cfg = config{
            .on_ip   = { 0 },
            .vn_ip   = get_interface_ip("eth0"),
            .on_port = 5000,
            .vn_port = 0,
        };

        int found_mask = 0;

        for (int i = 1; i < argc; ++i) {
            auto& arg = argv[i];

            if (arg == "--on-ip"sv) {
                if (++i == argc) throw exception_with_location("Expected: ON IP");
                if (found_mask & 0b0001) throw exception_with_location("Repeated: ON IP");
                test_fail(inet_pton(AF_INET, argv[i], &cfg->on_ip)-1, "Invalid IP");
                cfg->on_ip = ntohl(cfg->on_ip);
                found_mask |= 0b0001;
            } else if (arg == "--vn-ip"sv) {
                if (++i == argc) throw exception_with_location("Expected: VN IP");
                if (found_mask & 0b0010) throw exception_with_location("Repeated: VN IP");
                test_fail(inet_pton(AF_INET, argv[i], &cfg->vn_ip)-1, "Invalid IP");
                cfg->vn_ip = ntohl(cfg->vn_ip);
                found_mask |= 0b0010;
            } else if (arg == "--on-port"sv) {
                if (++i == argc) throw exception_with_location("Expected: ON TCP port");
                if (found_mask & 0b0100) throw exception_with_location("Repeated: ON port");
                cfg->on_port = static_cast<uint16_t>(atoi(argv[i]));
                if (cfg->on_port < 1000) throw exception_with_location("ON port in Reserved Range (<1000)");
                found_mask |= 0b0100;
            } else if (arg == "--vn-port"sv) {
                if (++i == argc) throw exception_with_location("Expected: VN UDP port");
                if (found_mask & 0b1000) throw exception_with_location("Repeated: VN port");
                cfg->vn_port = static_cast<uint16_t>(atoi(argv[i]));
                if (cfg->vn_port < 1000) throw exception_with_location("VN port in Reserved Range (<1000)");
                found_mask |= 0b1000;
            } else {
                throw exception_with_location(std::format("Unknown Argument {}", arg));
            }
        }
        if ((found_mask & 0b1001) != 0b1001) throw exception_with_location("ON IP and VN port are required");
        return cfg;
    }

} // namespace brk_bt
