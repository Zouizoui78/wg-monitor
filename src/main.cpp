#include <iostream>
#include <csignal>

#include "http.hpp"
#include "log.hpp"

#define DEFAULT_PORT 4000

std::function<void (int signal)> handler;
void signal_handler_caller(int signal) {
    SPDLOG_INFO("Received signal {}", strsignal(signal));
    handler(signal);
}

int main(int argc, char **argv) {
    log_init();

    signal(SIGINT, signal_handler_caller);
    signal(SIGTERM, signal_handler_caller);
    signal(SIGKILL, signal_handler_caller);

    std::string addr("0.0.0.0");
    uint16_t port = DEFAULT_PORT;

    if (argc == 2) {
        port = std::stoul(argv[1]);
    }

    HTTPServer http_server;
    if (!http_server.is_ok()) {
        return 1;
    }

    handler = [&http_server](int signal) {
        SPDLOG_INFO("Stopping server...");
        http_server.stop();
    };

    SPDLOG_INFO("Starting server on {}:{}", addr, port);
    bool server_status = http_server.listen(addr, port);
    if (!server_status) {
        SPDLOG_ERROR("Failed to start server : {}", strerror(errno));
        return 1;
    }

    SPDLOG_INFO("Quitting");
}