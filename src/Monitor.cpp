#include <filesystem>
#include <fstream>
#include "log.hpp"
#include "Monitor.hpp"
#include "nlohmann/json.hpp"
#include "tools.hpp"

#define CONNECTION_LOST_DELAY_S 300

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
    #ifdef DEBUG
        std::this_thread::sleep_for(std::chrono::seconds(5));
    #else
        std::this_thread::sleep_for(std::chrono::seconds(1));
    #endif
    }
}

bool Monitor::process_devices(const WGDeviceMap& devices) {
    bool ok = true;

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

        if (!process_peer(device, peer, *prev_peer)) {
            SPDLOG_ERROR("Peer {} from device {} processing failed", peer.allowed_ips[0], device.name);
            ok = false;
        }
    }
    return ok;
}

bool Monitor::process_peer(const wg::Device &device, const wg::Peer &peer, const wg::Peer &previous_peer) {
    SPDLOG_DEBUG("Processing peer {}", peer.allowed_ips[0]);

    int64_t last_handshake_diff = peer.last_handshake - previous_peer.last_handshake;
#ifdef DEBUG
    if (last_handshake_diff > 0) {
#else
    if (last_handshake_diff >= CONNECTION_LOST_DELAY_S) { // 5min
#endif
        SPDLOG_DEBUG("Handshake event for peer {}", peer.allowed_ips[0]);
        hook(HookEvents::PEER_HANDSHAKE, device, peer);
    }

    return true;
}

void Monitor::hook(HookEvents event, const wg::Device &device, const Peer &peer) {
    auto hooks = get_not_excluded_hooks(event, device, peer);
    for (const auto &hook : hooks) {
        hook->run(device, peer);
    }
}

HooksVector Monitor::get_not_excluded_hooks(HookEvents events, const wg::Device &device, const wg::Peer &peer) const {
    HooksVector hooks = get_hooks_by_events(events);
    for (auto it = hooks.begin() ; it != hooks.end() ; ) {
        if ((*it)->is_excluded(device, peer)) {
            it = hooks.erase(it);
        }
        else {
            ++it;
        }
    }
    return hooks;
}

HooksVector Monitor::get_hooks_by_events(HookEvents events) const {
    HooksVector ret;
    for (const auto &hook : _hooks) {
        if (hook->events & events) {
            ret.push_back(hook);
        }
    }
    return ret;
}

bool Monitor::parse_hooks() {
    std::string path("hooks.json");

    if (!fs::exists(path)) {
        SPDLOG_INFO("{} doesn't exist, no hook parsed", path);
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

    if (!parsed.is_array()) {
        SPDLOG_ERROR("hooks.json must be an array");
    }

    SPDLOG_DEBUG("Parsed hooks :\n{}", parsed.dump(2));

    for (const auto &hook_json : parsed) {
        _hooks.push_back(Hook::factory(hook_json));
    }

    return true;
}

} // namespace wg