#include "msg.hpp"
#include "arpa/inet.h"
#include <iostream>
#include <string>
#include <cstdint>
#include <cstdio>
#include <cassert>

uint32_t str_to_ip_uint(const std::string& ip_str) {
    unsigned int bytes[4];
    char dummy;

    // sscanf parses the components. The trailing %c helps ensure 
    // there is no extra garbage text after the 4th octet.
    int parsed = std::sscanf(ip_str.c_str(), "%u.%u.%u.%u%c", 
                             &bytes[0], &bytes[1], &bytes[2], &bytes[3], &dummy);

    if (parsed != 4 || bytes[0] > 255 || bytes[1] > 255 || bytes[2] > 255 || bytes[3] > 255) {
        std::cerr << "Invalid IPv4 address!\n";
        return 0;
    }

    return (static_cast<uint32_t>(bytes[0]) << 24) |
           (static_cast<uint32_t>(bytes[1]) << 16) |
           (static_cast<uint32_t>(bytes[2]) <<  8) |
           (static_cast<uint32_t>(bytes[3]));
}

using namespace brk_bt;

int main() {
    lsp_tuple C = {'C', str_to_ip_uint("10.0.0.2"), 5000, 1};
    lsp_tuple D = {'D', str_to_ip_uint("10.0.0.2"), 5001, 7};
    lsp_tuple E = {'E', str_to_ip_uint("10.0.0.2"), 5002, 0};

    lsp_packet pkt {{C, D, E}, 0xaabbccdd, 'X'};

    lsp_packet test_pkt = lsp_packet::from_binary(pkt.to_binary());

    assert(pkt.seq_no == test_pkt.seq_no);
    assert(pkt.data == test_pkt.data);   
}