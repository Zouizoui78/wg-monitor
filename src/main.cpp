#include <iostream>
#include <csignal>

#include "HTTPServer.hpp"
#include "spdlog/spdlog.h"
#include "Monitor.hpp"

#define DEFAULT_PORT 4000

std::function<void (int signal)> handler;
void signal_handler_caller(int signal) {
    handler(signal);
}

int main(int argc, char **argv) {
#ifdef DEBUG
    spdlog::set_level(spdlog::level::debug);
#else
    spdlog::set_pattern("[%l] %v");
#endif

    signal(SIGINT, signal_handler_caller);
    signal(SIGTERM, signal_handler_caller);

    std::string addr("0.0.0.0");
    uint16_t port = DEFAULT_PORT;

    if (argc == 2) {
        port = std::stoul(argv[1]);
    }

    wg::Monitor monitor;
    if (!monitor.is_ok()) {
        return 1;
    }

    HTTPServer http_server;
    handler = [&http_server](int signal) {
        SPDLOG_INFO("Received signal {}", strsignal(signal));
        http_server.stop();
        SPDLOG_INFO("Stopped http server");
    };

    SPDLOG_INFO("Starting server on {}:{}", addr, port);
    bool server_status = http_server.listen(addr, port);
    if (!server_status) {
        SPDLOG_ERROR("Failed to start server : {}", strerror(errno));
        return 1;
    }

    SPDLOG_INFO("Quitting");
}
