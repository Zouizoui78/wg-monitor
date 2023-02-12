#include <filesystem>
#include <fstream>
#include "log.hpp"
#include "Monitor.hpp"
#include "nlohmann/json.hpp"

using json = nlohmann::json;
namespace fs = std::filesystem;

namespace wg {

Monitor::Monitor() {
    bool parsing_ok = parse_hooks();
    start();
    _is_ok = is_running() && parsing_ok;
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
        SPDLOG_DEBUG("Monitor loop");
        auto devices = wg::get_devices();
        if (!devices.empty()) {
            process_devices(devices);
        }
        _previous_devices = devices;
        SPDLOG_DEBUG("Monitor loop done");
        std::this_thread::sleep_for(std::chrono::seconds(5));
    }
}

bool Monitor::process_devices(const WGDeviceMap& devices) {
    bool ok = true;

    SPDLOG_DEBUG("previous = \n{}", json(_previous_devices).dump(4));
    SPDLOG_DEBUG("current = \n{}", json(devices).dump(4));

    for (const auto& [name, device] : devices) {
        auto previous_device_it = _previous_devices.find(name);
        if (previous_device_it == _previous_devices.end()) {
            // TODO new device hook
            continue;
        }
        if (!process_device(device, previous_device_it->second)) {
            SPDLOG_ERROR("Device {} processing failed", name);
            ok = false;
        }
    }
    return ok;
}

bool Monitor::process_device(const wg::Device &device, const wg::Device &previous_device) {
    SPDLOG_DEBUG("Processing device {}", device.name);
    bool ok = true;
    for (const auto &peer : device.peers) {
        const std::vector<Peer> &prev_peers = previous_device.peers;
        auto prev_peer = std::find_if(
            prev_peers.begin(),
            prev_peers.end(),
            [&peer](const Peer &candidate) {
                return peer.allowed_ips[0] == candidate.allowed_ips[0];
            }
        );

        if (prev_peer == prev_peers.end()) {
            // TODO new peer hook
            continue;
        }

        if (!process_peer(peer, *prev_peer)) {
            SPDLOG_ERROR("Peer {} from device {} processing failed", peer.allowed_ips[0], device.name);
            ok = false;
        }
    }
    return ok;
}

bool Monitor::process_peer(const wg::Peer &peer, const wg::Peer &previous_peer) {
    SPDLOG_DEBUG("Processing peer {}", peer.allowed_ips[0]);

    int64_t last_handshake_diff = peer.last_handshake - previous_peer.last_handshake;
    if (last_handshake_diff > 300) { // 5min
        handshake_hook_impl(peer);
    }

    return true;
}

void Monitor::handshake_hook_impl(const Peer &peer) {
    SPDLOG_DEBUG("Handshake event for peer {}", peer.allowed_ips[0]);
}

std::vector<Hook> Monitor::get_hooks_by_events(HookEvents events) {
    std::vector<Hook> ret;
    for (const auto &hook : _hooks) {
        if (hook.events & events) {
            ret.push_back(hook);
        }
    }
    return ret;
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