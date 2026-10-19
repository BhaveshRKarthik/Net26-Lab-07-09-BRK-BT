#include <arpa/inet.h>
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

void help_options() {
    std::cout << 
    "./vn [OPTIONS]\n"
    "\t--on-ip\n"
    "\t--on-port\n"
    "\t--my-ip\n"
    "\t--my-port\n"
    "\t--help\n";
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
        auto arg = argv[i];

        if (arg == "--on-ip") {
            if (++i == argc) throw brk_bt::exception_with_location("Expected: ON IP");
            on_ip.s_addr = atoi(argv[i]);
        } else if (arg == "--on-port") {
            if (++i == argc) throw brk_bt::exception_with_location("Expected: ON TCP port");
            on_tcp_port = atoi(argv[i]);
        } else if (arg == "--my-port") {
            if (++i == argc) throw brk_bt::exception_with_location("Expected: VN UDP port");
            self_udp_port = atoi(argv[i]);
        } else if (arg == "--my-ip") {
            if (++i == argc) throw brk_bt::exception_with_location("Expected: VN IP");
            self_ip = atoi(argv[i]);
        } else if (arg == "--help") {
            help_options();
            return 0;
        }
    }

    brk_bt::IPv4TCPClient on_sock;
    on_sock.connect(on_ip, on_tcp_port);

}
