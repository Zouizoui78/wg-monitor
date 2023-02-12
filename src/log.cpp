#include "log.hpp"

void log_init() {
    spdlog::set_pattern("[%-7l] %v");
    #ifdef DEBUG
    spdlog::set_level(spdlog::level::debug);
    #endif
}