#pragma once

#include <exception>
#include <source_location>
#include <stdexcept>
#include <string_view>
#include <utility>

namespace brk_bt {
    class immovable {
    public:
        immovable()                            = default;
        immovable(const immovable&)            = delete;
        immovable(immovable&&)                 = delete;
        immovable& operator=(const immovable&) = delete;
        immovable& operator=(immovable&&)      = delete;
    };

    struct exception_with_location : public std::runtime_error {
        static std::string error_string(const std::source_location& loc, std::string_view err) {
            return std::format("[{}@{}:{}] {}", loc.function_name(), loc.file_name(), loc.line(), err);
        }

        exception_with_location(std::string_view err, const std::source_location& loc = std::source_location::current())
            : std::runtime_error(error_string(loc, err)) {}
    };

    // maybe useful later?
    namespace detail::defer {
        template<typename Func>
        class defer_impl : immovable {
        public:
            defer_impl(Func f) : m_func(std::move(f)), m_old_uncaught_exceptions(std::uncaught_exceptions()) {}
            ~defer_impl() noexcept(noexcept(m_func())) {
                try {
                    m_func();
                } catch (...) {
                    if (std::uncaught_exceptions() <= m_old_uncaught_exceptions) throw;
                }
            }

        private:
            Func m_func;
            int m_old_uncaught_exceptions;
        };

    } // namespace detail::defer

#define BRK_BT_GLUE(x, y) x##y
#define BRK_BT_UNIQ_NAME(x, y) BRK_BT_GLUE(x, y)
#define defer const ::brk_bt::detail::defer::defer_impl BRK_BT_UNIQ_NAME(defer_guard_, __COUNTER__) = [&]()

} // namespace brk_bt
