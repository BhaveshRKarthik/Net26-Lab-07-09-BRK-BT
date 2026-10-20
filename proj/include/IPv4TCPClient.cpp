#include "IPv4TCPClient.hpp"

namespace brk_bt {
    std::optional<link_state> IPv4TCPClient::recv(int flags) {
        ssize_t recv_len = 0;
        
        uint8_t size;
        recv_len = ::recv(m_sock, &size, 1, flags);
        if (recv_len == 0 || (recv_len < 0 && (errno == EAGAIN || errno == EWOULDBLOCK))) return std::nullopt;
        test_fail(recv_len, "Failed to recv size byte");

        std::vector<std::byte> buf(11*size);
        do {
            recv_len = ::recv(m_sock, buf.data(), buf.size(), 0);
        } while (recv_len < 0 && errno == EAGAIN);
        test_fail(recv_len, "Failed to recv");
        return link_state::from_binary(buf);
    }

    void IPv4TCPClient::send(std::string_view data, int flags) {
        while (!data.empty()) {
            ssize_t curr = ::send(m_sock, data.data(), data.size(), flags | MSG_NOSIGNAL);
            test_fail(curr, "Failed to send");
            data.remove_prefix(curr);
        }
    }

} // namespace brk_bt
