#pragma once

#include <cerrno>
#include <cstdint>
#include <fcntl.h>
#include <netinet/in.h>
#include <optional>
#include <string>
#include <string_view>
#include <sys/socket.h>
#include <unistd.h>
#include <utility>

#include "misc_types.hpp"

namespace brk_bt {

    struct Datagram {
        std::string data;
        sockaddr_in from;
    };

    struct FdProtector {
    public:
        explicit FdProtector(int fd) : m_fd(fd) {}
        ~FdProtector() { reset(); }
        FdProtector(const FdProtector&)            = delete;
        FdProtector& operator=(const FdProtector&) = delete;

        FdProtector(FdProtector&& o) noexcept : m_fd(std::exchange(o.m_fd, -1)) {}
        FdProtector& operator=(FdProtector&& o) noexcept {
            if (this != &o) {
                reset();
                m_fd = std::exchange(o.m_fd, -1);
            }
            return *this;
        }

        int get() const { return m_fd; }

    private:
        void reset() {
            if (m_fd >= 0) ::close(m_fd);
            m_fd = -1;
        }
        int m_fd = -1;
    };

    class IPv4UDPPeer : immovable {
    public:
        explicit IPv4UDPPeer(uint16_t port) : IPv4UDPPeer(FdProtector{ socket(PF_INET, SOCK_DGRAM | SOCK_NONBLOCK, 0) }, port) {}

        bool sendto(const sockaddr_in& dest, std::string_view data, int flags) const;
        std::optional<Datagram> recv(int flags) const;

    protected:
        IPv4UDPPeer(FdProtector sock, uint16_t port) : m_sock(std::move(sock)), m_port(port) {
            test_fail(m_sock.get(), "Invalid Socket");

            int fl = ::fcntl(m_sock.get(), F_GETFL, 0);
            test_fail(fl, "fcntl failed");
            if (!(fl & O_NONBLOCK)) throw exception_with_location("Blocking socket handed to IPv4UDPPeer");

            sockaddr_in addr{ .sin_family = AF_INET, .sin_port = htons(m_port), .sin_addr = { htonl(INADDR_ANY) } };

            // int on = 1;
            // test_fail(::setsockopt(m_sock.get(), SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on)), "setsockopt SO_REUSEADDR failed");
            test_fail(::bind(m_sock.get(), reinterpret_cast<sockaddr*>(&addr), sizeof(addr)), "Failed to acquire port");

            socklen_t sz = sizeof(addr);
            test_fail(::getsockname(m_sock.get(), reinterpret_cast<sockaddr*>(&addr), &sz), "getsockname failed");
            m_port = ntohs(addr.sin_port);
        }

    private:
        FdProtector m_sock;
        uint16_t m_port;
    };
} // namespace brk_bt
