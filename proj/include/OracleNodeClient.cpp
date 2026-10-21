#include "OracleNodeClient.hpp"

namespace brk_bt {
    std::optional<link_state> OracleNodeClient::recv() {
        ssize_t recv_len = 0;

        uint8_t size;
        recv_len = ::recv(m_sock, &size, 1, MSG_DONTWAIT);
        if (recv_len < 0 && (errno == EAGAIN || errno == EWOULDBLOCK)) return std::nullopt;
        test_fail(recv_len, "Failed to recv size byte");
        if (recv_len == 0) throw exception_with_location("TCP Connection with ON closed by ON.");

        const size_t buf_size = 11 * size;
        std::vector<std::byte> buf(buf_size);
        size_t total_read = 0;

        while (total_read < buf_size) {
            recv_len = ::recv(m_sock, buf.data() + total_read, buf_size - total_read, 0);
            test_fail(recv_len, "Failed to recv data");
            if (recv_len == 0) throw exception_with_location("TCP Connection with ON closed by ON.");
            total_read += recv_len;
        }
        test_fail(recv_len, "Failed to recv");
        return link_state::from_binary(buf);
    }

    void OracleNodeClient::send(std::span<const std::byte> data) {
        while (!data.empty()) {
            ssize_t curr = ::send(m_sock, data.data(), data.size(), MSG_NOSIGNAL);
            test_fail(curr, "Failed to send");
            data = data.subspan(curr);
        }
    }

} // namespace brk_bt
