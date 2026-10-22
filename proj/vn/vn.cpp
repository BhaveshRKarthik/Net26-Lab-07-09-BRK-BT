#include <iostream>

#include "misc_types.hpp"
#include "arg_parse.hpp"
#include "OracleNodeClient.hpp"

brk_bt::config config;

int main(int argc, char* argv[]) {
    auto cfg = brk_bt::parse_argv(argc, argv);
    if (!cfg) {
        brk_bt::help_options();
        return 1;
    }

    auto [on_ip, vn_ip, on_port, vn_port] = config = *cfg;

    brk_bt::OracleNodeClient on_sock;
    on_sock.connect(on_ip, on_port);
    std::cout << "CONNECTED\n";

    // brk_bt::connect_tuple con_msg{ .ip = self_ip, .udp_port = self_udp_port };
    // on_sock.send(con_msg.to_binary());

    // brk_bt::IPv4UDPPeer my_sock{ self_udp_port };
    // while (true) {
    //     auto s = on_sock.recv();
    //     // each message from the server consists of one or more LINK-STATE messages
    //     // I can read one tuple at a time from the byte stream but currently you
    //     // cannot separate lists of tuples sent at different points in time, they
    //     // just get merged by tcp
    //     std::cout << "=== BEGIN ===\n";
    //     if (s) {
    //         auto& ls = *s;
    //         std::cout << ls.size() << '\n';
    //         for (auto& tup : ls.tuples) {
    //             std::println("{}@{}:{}={}", tup.nbr_alpha, tup.ip, tup.udp_port, tup.cost);
    //         }
    //     } else {
    //     }
    //     // OPERATION OF VIRTUAL NODE 2(D) -> need to be able to tell lists of tuples apart
    //     // On receiving LINK-STATE message, print to STDOUT in human-readable format, the
    //     // neighbourhood link state. E.g. node E in given example network can print
    //     std::cout << "=== END ===\n";
    //     std::cout.flush(); // does not seem to help
    //     sleep(1);
    // }
}
