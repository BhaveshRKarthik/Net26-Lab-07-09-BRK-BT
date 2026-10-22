#include "IPv4UDPPeer.hpp"

namespace brk_bt {

    bool IPv4UDPPeer::sendto(const sockaddr_in& dest, std::string_view data, int flags) const {
        ssize_t n = ::sendto(m_sock.get(), data.data(), data.size(), flags | MSG_NOSIGNAL, reinterpret_cast<const sockaddr*>(&dest), sizeof(dest));
        if (n < 0 && (errno == EAGAIN || errno == EWOULDBLOCK)) return false; // report "would block" to the caller; the socket buffer is full
        test_fail(n, "Failed to send");
        return true;
    }

    std::optional<Datagram> IPv4UDPPeer::recv(int flags) const {
        Datagram result;
        socklen_t addrlen = sizeof(result.from);
        char buf[65536];
        ssize_t recv_len = ::recvfrom(m_sock.get(), buf, sizeof(buf), flags, reinterpret_cast<sockaddr*>(&result.from), &addrlen);
        if ((recv_len < 0) && (errno == EAGAIN || errno == EWOULDBLOCK)) {
            return std::nullopt;
        }

        test_fail(recv_len, "Failed to recv");

        result.data.assign(buf, static_cast<size_t>(recv_len));
        return result;
    }
} // namespace brk_bt
