#include <arpa/inet.h>
#include <ifaddrs.h>
#include <atomic>
#include <cstring>
#include <iostream>
#include <netinet/in.h>
#include <print>
#include <signal.h>
#include <sys/socket.h>
#include <unistd.h>

#include "misc_types.hpp"
#include "IPv4TCPClient.hpp"
#include "IPv4UDPPeer.hpp"
#include "msg.hpp"

void help_options() {
    std::cout << 
    "./vn [OPTIONS]\n"
    "\t--on-ip\n"
    "\t--on-port\n"
    "\t--my-ip\n"
    "\t--my-port\n"
    "\t--help\n";
}

// Returns IPv4 address in network byte order, or 0 on failure.
uint32_t get_interface_ip(const char* ifname) {
    ifaddrs* list = nullptr;
    if (getifaddrs(&list) != 0) return 0;

    uint32_t result = 0;
    for (ifaddrs* ifa = list; ifa; ifa = ifa->ifa_next) {
        if (!ifa->ifa_addr || ifa->ifa_addr->sa_family != AF_INET) continue;
        if (std::strcmp(ifa->ifa_name, ifname) != 0) continue;

        auto* sin = reinterpret_cast<sockaddr_in*>(ifa->ifa_addr);
        result = sin->sin_addr.s_addr;
        break;
    }
    freeifaddrs(list);
    return result;
}


int main(int argc, char* argv[]) {
    in_addr on_ip {0};
    uint16_t on_tcp_port = 5000;

    uint32_t self_ip       = 0;
    uint16_t self_udp_port = 0;

    if (argc == 1) {
        help_options();
        return 0;
    }

    for (int i = 1; i < argc; ++i) {
        auto& arg = argv[i];
        using namespace std::string_view_literals;
        if (arg == "--on-ip"sv) {
            if (++i == argc) throw brk_bt::exception_with_location("Expected: ON IP");
            on_ip.s_addr = atoi(argv[i]);
        } else if (arg == "--on-port"sv) {
            if (++i == argc) throw brk_bt::exception_with_location("Expected: ON TCP port");
            on_tcp_port = atoi(argv[i]);
        } else if (arg == "--my-port"sv) {
            if (++i == argc) throw brk_bt::exception_with_location("Expected: VN UDP port");
            self_udp_port = atoi(argv[i]);
        } else if (arg == "--my-ip"sv) {
            if (++i == argc) throw brk_bt::exception_with_location("Expected: VN IP");
            self_ip = atoi(argv[i]);
        } else if (arg == "--help"sv) {
            help_options();
            return 0;
        } else {
            throw brk_bt::exception_with_location(std::format("Unknown Argument {}", arg));
        }
    }

    brk_bt::IPv4TCPClient on_sock;
    on_sock.connect(on_ip, on_tcp_port);
    std::cout << "CONNECTED\n";

    if (!self_ip)
        brk_bt::test_fail((self_ip = get_interface_ip("eth0")) - 1, "Could not find My IP");


    brk_bt::connect_tuple con_msg {
        .ip = self_ip,
        .udp_port = self_udp_port
    };
    on_sock.send(con_msg.to_binary());

    brk_bt::IPv4UDPPeer my_sock{self_udp_port};
    while (true) {
        auto s = on_sock.recv(); 
        // each message from the server consists of one or more LINK-STATE messages
        // I can read one tuple at a time from the byte stream but currently you
        // cannot separate lists of tuples sent at different points in time, they
        // just get merged by tcp
        std::cout << "=== BEGIN ===\n";
        if (s) {
            auto& ls = *s;
            std::cout << ls.size() << '\n';
            for (auto& tup : ls.tuples) {
                std::println("{}@{}:{}={}", tup.nbr_alpha, tup.ip, tup.udp_port, tup.cost);
            }
        } else {

        }
        // OPERATION OF VIRTUAL NODE 2(D) -> need to be able to tell lists of tuples apart
        // On receiving LINK-STATE message, print to STDOUT in human-readable format, the
        // neighbourhood link state. E.g. node E in given example network can print    
        std::cout << "=== END ===\n";
        std::cout.flush(); // does not seem to help
        sleep(1);
    }

}
