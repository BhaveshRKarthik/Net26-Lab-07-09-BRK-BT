#include <arpa/inet.h>
#include <atomic>
#include <algorithm>
#include <cstring>
#include <string>
#include <string_view>
#include <iostream>
#include <fstream>
#include <sstream>
#include <netinet/in.h>
#include <print>
#include <signal.h>
#include <sys/socket.h>
#include <unistd.h>

#include "misc_types.hpp"
#include "graph.hpp"

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "usage: " << argv[0] << " <oracle-config-file>\n";
        exit(EXIT_FAILURE);
    }
    std::ifstream oracle_cfg(argv[1]);
    defer {
        oracle_cfg.close();
    };
    if (!oracle_cfg) throw brk_bt::exception_with_location("");
    Neighbourhood nbd;

    std::string cline;
    
    constexpr std::string_view whitespace = " \t\r\n\v\f";
    int i = 0;
    int expected_nodes = 0;
    while (std::getline(oracle_cfg, cline)) {
        if (cline.starts_with('#')) continue;
        if (cline.find_first_not_of(whitespace) == std::string::npos) continue;

        std::stringstream ss(cline);
        int n;
        int j = i + 1;
        int loop_iterations = 0;
        while (ss >> n) {
            if (i == 0) ++expected_nodes;
            if (j >= 26) throw brk_bt::exception_with_location("Too integer many entries in line");
            nbd[i, j] = nbd[j, i] = n;
            ++j;
            ++loop_iterations;
        }
        if (loop_iterations != expected_nodes) throw brk_bt::exception_with_location("Bad number of integer entries");
        --expected_nodes;
        ++i;
    }
    if (expected_nodes != 0) throw brk_bt::exception_with_location("Rows terminated prematurely");

}