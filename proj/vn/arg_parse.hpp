#pragma once

#include <netinet/in.h>
#include <optional>

namespace brk_bt {

    struct config {
        in_addr on_ip;
        in_addr vn_ip;
        uint16_t on_port;
        uint16_t vn_port;
    };

    std::optional<config> parse_argv(int argc, char* argv[]);

    void help_options();

} // namespace brk_bt
