#include "IPv4TCPClient.hpp"

namespace brk_bt {
    std::string IPv4TCPClient::recv(int flags) {
        std::string result;
        char buf[1024];
        ssize_t recv_len;
        do {
            recv_len = ::recv(m_sock, buf, sizeof(buf), flags);
            test_fail(recv_len, "Failed to recv");
            result += std::string_view{ buf, static_cast<std::size_t>(recv_len) };
        } while (recv_len > 0);

        return result;
    }

    void IPv4TCPClient::send(std::string_view data, int flags) {
        while (!data.empty()) {
            ssize_t curr = ::send(m_sock, data.data(), data.size(), flags | MSG_NOSIGNAL);
            test_fail(curr, "Failed to send");
            data.remove_prefix(curr);
        }
    }

} // namespace brk_bt
