#pragma once

#include <cerrno>
#include <span>
#include <cstdint>
#include <optional>
#include <sys/epoll.h>
#include <unistd.h>

#include "misc_types.hpp"

namespace brk_bt {

    class Epoll : immovable {
    public:
        Epoll(std::span<const int> fds) : m_epoll(::epoll_create1(EPOLL_CLOEXEC)) { 
            test_fail(m_epoll, "epoll_create1 failed"); 
            for (int fd : fds) {
                add(fd);
            }
        }
        ~Epoll() { ::close(m_epoll); }

        void add(int fd, std::uint32_t events = EPOLLIN) {
            epoll_event evnt{};
            evnt.events  = events;
            evnt.data.fd = fd;
            test_fail(::epoll_ctl(m_epoll, EPOLL_CTL_ADD, fd, &evnt), "epoll_ctl(ADD) failed");
        }

        std::optional<epoll_event> wait(int timeout_ms = -1) {
            epoll_event evnt{};
            int n = ::epoll_wait(m_epoll, &evnt, 1, timeout_ms);
            if (n == 0 || (n < 0 && errno == EINTR)) return std::nullopt;
            test_fail(n, "epoll_wait failed");
            return evnt;
        }

    private:
        int m_epoll;
    };

} // namespace brk_bt
