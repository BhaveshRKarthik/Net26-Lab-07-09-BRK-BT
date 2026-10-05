#include <arpa/inet.h>
#include <atomic>
#include <cstring>
#include <iostream>
#include <netinet/in.h>
#include <print>
#include <signal.h>
#include <sys/socket.h>
#include <unistd.h>

#include "misc_types.hpp"

class IPv4TCPClient {
    int m_sock;

private:
    friend class IPv4TCPServer;
    IPv4TCPClient(int sock) : m_sock(sock) {
        if (m_sock < 0) throw brk_bt::exception_with_location("Failed to create socket");
    }

public:
    IPv4TCPClient() : IPv4TCPClient(socket(PF_INET, SOCK_STREAM, 0)) {}
    ~IPv4TCPClient() { close(m_sock); }

public:
    void connect(int port) {
        sockaddr_in addr{ .sin_family = AF_INET, .sin_port = htons(port) };
        inet_aton("127.0.0.1", &(addr.sin_addr));
        if (::connect(m_sock, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) < 0) throw brk_bt::exception_with_location("Failed to connect");
    }

    void recv() {
        char buffer[1024] = {};
        std::string_view msg;
        while (true) {
            ssize_t recv_len = ::recv(m_sock, buffer, sizeof(buffer), 0);
            if (recv_len < 0) throw brk_bt::exception_with_location("Failed to recv");
            if (recv_len == 0) break;
            msg = { buffer, static_cast<std::size_t>(recv_len) };
            std::println("{}", msg);
        }
    }

    void send(const char* buf, int len, int flags = 0) {
        ssize_t sent = 0;
        while (sent != len) {
            ssize_t curr = ::send(m_sock, buf, len, flags);
            if (curr < 0) throw brk_bt::exception_with_location("Failed to send");
            sent += curr;
            buf += curr;
        }
    }
};

struct KeyboardInterrupt : std::runtime_error {
    KeyboardInterrupt() : std::runtime_error("KeyboardInterrupt") {}
};

class sig {
public:
    static void check_signals() {
        if (end.load(std::memory_order_relaxed)) {
            // Safe to throw here! We are back in normal program flow.
            throw KeyboardInterrupt();
        }
    }

private:
    static void handle_sigint(int signal) {
        if (signal == SIGINT) {
            end.store(true, std::memory_order_relaxed);
        }
    }

    static inline std::atomic_bool end = [] {
        signal(SIGINT, &handle_sigint);
        return false;
    }();
};

class IPv4TCPServer {
    int m_sock;

public:
    IPv4TCPServer() : m_sock(socket(PF_INET, SOCK_STREAM, 0)) {
        if (m_sock == -1) throw brk_bt::exception_with_location("Failed to create socket");
    }
    ~IPv4TCPServer() { close(m_sock); }

public:
    void bind(uint16_t port) {
        sockaddr_in addr{ .sin_family = AF_INET, .sin_port = htons(port) };
        inet_aton("127.0.0.1", &(addr.sin_addr));
        if (::bind(m_sock, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) < 0) {
            throw brk_bt::exception_with_location("Failed to bind");
        }
    }

private:
    void listen(int backlog) {
        if (::listen(m_sock, backlog) < 0) {
            throw brk_bt::exception_with_location("Failed to listen");
        }
    }

    IPv4TCPClient accept() {
        sockaddr_in addr{};
        socklen_t size = sizeof(addr);
        return ::accept(m_sock, reinterpret_cast<sockaddr*>(&addr), &size);
    }

public:
    void run(int backlog) {
        listen(backlog);
        while (true) {
            sig::check_signals();
            auto cli                    = accept();
            static constexpr char msg[] = "Hello Socket!";
            cli.send(msg, sizeof(msg));
        }
    }
};



int main(int argc, char** argv) {
    using namespace std::string_view_literals;

    if (argc != 3) {
        std::println("Usage: {} [client|server] port", argv[0]);
        return 1;
    }
    try {
        std::string_view mode = argv[1];
        int port = std::atoi(argv[2]);
        if (mode == "client"sv) {
            IPv4TCPClient client;
            client.connect(port);
            client.recv();
        } else if (mode == "server"sv) {
            IPv4TCPServer server;
            server.bind(port);
            server.run(5);
        }
    } catch (const std::exception& e) {
        std::println("Caught error: {}", e.what());
    } catch (...) {
        std::println("Caught unknown error");
    }
}
