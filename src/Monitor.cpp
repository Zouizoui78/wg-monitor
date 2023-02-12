#include <filesystem>
#include <fstream>
#include "log.hpp"
#include "Monitor.hpp"
#include "nlohmann/json.hpp"

using json = nlohmann::json;
namespace fs = std::filesystem;

namespace wg {

Monitor::Monitor() {
    start();
    _is_ok = is_running() && parse_hooks();
}

Monitor::~Monitor() {
    if (is_running()) {
        stop();
    }
}

bool Monitor::is_ok() {
    return _is_ok;
}

bool Monitor::is_running() {
    return _running;
}

void Monitor::start() {
    if (is_running()) {
        return;
    }

    _running = true;
    _thread = std::thread(std::bind(&Monitor::monitor, this));
    SPDLOG_INFO("Started monitor");
}

void Monitor::stop() {
    if (!is_running()) {
        return;
    }

    _running = false;
    _thread.join();
    SPDLOG_INFO("Stopped monitor");
}

void Monitor::monitor() {
    while (is_running()) {
        // TODO : monitoring impl
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
}

bool Monitor::parse_hooks() {
    std::string path("hooks.json");

    if (!fs::exists(path)) {
        SPDLOG_INFO("{} doesn't exist, considering hooks parsing done", path);
        return true;
    }

    std::ifstream in(path);
    if (!in.is_open()) {
        SPDLOG_ERROR("Failed to open {}", path);
        return false;
    }

    json parsed;
    try {
        parsed = nlohmann::json::parse(in);
    }
    catch (...) {
        SPDLOG_ERROR("Failed to parse hooks, json error");
        return false;
    }

    _hooks = parsed;
    SPDLOG_INFO("Parsed hooks :\n{}", parsed.dump(4));

    return true;
}

} // namespace wg