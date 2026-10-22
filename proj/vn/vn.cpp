#include <iostream>

#include "LinkStateProtocol.hpp"
#include "OracleNodeClient.hpp"
#include "arg_parse.hpp"
#include "connect_msg.hpp"
#include "epoll.hpp"
#include "misc_types.hpp"

int main(int argc, char* argv[]) {
    auto cfg = brk_bt::parse_argv(argc, argv);
    if (!cfg) {
        brk_bt::help_options();
        return 1;
    }

    auto [on_ip, vn_ip, on_port, vn_port] = *cfg;

    brk_bt::LinkStateProtocol lsp(vn_ip, vn_port);

    brk_bt::OracleNodeClient on;
    on.connect(on_ip, on_port);

    brk_bt::connect_msg con_msg{ .ip = vn_ip, .port = vn_port };
    brk_bt::Epoll ep({ { on.fd(), lsp.fd() } });

    on.send(con_msg.to_binary());
    std::cout << "CONNECTED\n";


    while (true) {
        auto ev = ep.wait(10000);
        if (!ev) continue;

        if (ev->data.fd == on.fd()) {
            if (auto status = on.recv()) lsp.update(std::move(*status));
        } else if (ev->data.fd == lsp.fd()) {
            lsp.update();
        }
    }


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
