#include "misc_types.hpp"
#include <format>
#include <cstring>
#include <cerrno>

namespace brk_bt {
    std::string exception_with_location::error_string(const std::source_location& loc, std::string_view err) {
        return std::format("[{}@{}:{}] {}", loc.function_name(), loc.file_name(), loc.line(), err);
    }

    void test_fail(ssize_t test, std::string_view error_msg, const std::source_location& loc) {
        if (test < 0) {
            throw exception_with_location(std::format("{}: {}", error_msg, std::strerror(errno)), loc);
        }
    }

} // namespace brk_bt
