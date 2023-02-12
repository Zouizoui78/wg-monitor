#include "log.hpp"

void log_init() {
    #ifdef DEBUG
    spdlog::set_level(spdlog::level::debug);
    #else
    spdlog::set_pattern("[%l] %v");
    #endif
}