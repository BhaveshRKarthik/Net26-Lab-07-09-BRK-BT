#include "OracleNodeClient.hpp"
#include <iostream>
#include <format>
#include <vector>

namespace brk_bt {
    std::optional<vn_status> OracleNodeClient::recv() {
        std::optional<vn_status> result = std::nullopt;

        ssize_t recv_len = 0;

        uint8_t size;
        recv_len = ::recv(m_sock, &size, 1, MSG_DONTWAIT);
        if (recv_len < 0 && (errno == EAGAIN || errno == EWOULDBLOCK)) return result;
        test_fail(recv_len, "Failed to recv size byte");
        if (recv_len == 0) throw exception_with_location("TCP Connection with ON closed by ON.");

        const size_t buf_size = 11 * size;
        std::vector<std::byte> buf(buf_size);
        size_t total_read = 0;

        while (total_read < buf_size) {
            recv_len = ::recv(m_sock, buf.data() + total_read, buf_size - total_read, 0);
            test_fail(recv_len, "Failed to recv data");
            if (recv_len == 0) throw exception_with_location("TCP Connection with ON closed by ON.");
            total_read += static_cast<size_t>(recv_len);
        }
        test_fail(recv_len, "Failed to recv");
        result = vn_status::from_binary(buf);

        std::cout << std::format("Received LINK-STATE: {} = 0", result->alpha);
        for (auto [alpha, tup] : result->neighbours) {
            std::cout << std::format(", {} = {}", alpha, tup.cost);
        }
        std::cout << std::endl;
        return result;
    }

    void OracleNodeClient::send(std::span<const std::byte> data) {
        while (!data.empty()) {
            ssize_t curr = ::send(m_sock, data.data(), data.size(), MSG_NOSIGNAL);
            test_fail(curr, "Failed to send");
            data = data.subspan(static_cast<size_t>(curr));
        }
    }

} // namespace brk_bt
