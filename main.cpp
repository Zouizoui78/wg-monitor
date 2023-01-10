#include <iostream>
#include <csignal>

#include "http.hpp"

std::function<void (int signal)> handler;
void signal_handler_caller(int signal) {
    std::cout << "\nReceived signal " << strsignal(signal) << std::endl;
    handler(signal);
}

int main(int argc, char **argv) {
    std::string addr("0.0.0.0");
    uint16_t port = 3000;

    if (argc == 2) {
        port = std::stoul(argv[1]);
    }

    HTTPServer http_server;

    handler = [&http_server](int signal) {
        std::cout << "Stopping server..." << std::endl;
        http_server.stop();
    };
    signal(SIGINT, signal_handler_caller);

    std::cout << "Starting server on " << addr << ":" << port << std::endl;
    bool server_status = http_server.listen(addr, port);
    if (!server_status) {
        std::cout << "Failed to start server : " << strerror(errno) << std::endl;
        return 1;
    }

    std::cout << "Quitting" << std::endl;
}