#pragma once

#include <netinet/in.h>
#include <optional>

namespace brk_bt {

    struct config {
        uint32_t on_ip;
        uint32_t vn_ip;
        uint16_t on_port;
        uint16_t vn_port;
    };

    std::optional<config> parse_argv(int argc, char* argv[]);

    void help_options();

} // namespace brk_bt
