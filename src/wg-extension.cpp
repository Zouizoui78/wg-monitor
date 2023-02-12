#include "log.hpp"
#include "wg-extension.hpp"

#include <arpa/inet.h> // inet_ntop
#include <cstring> // Needed for wg_for_each_device_name

using json = nlohmann::json;

namespace wg {

Peer::Peer() {}

Peer::Peer(wg_peer* peer) {
    char ipv4_str[INET_ADDRSTRLEN] = "";
    inet_ntop(AF_INET, &(peer->endpoint.addr4.sin_addr.s_addr), ipv4_str, INET_ADDRSTRLEN);
    endpoint = ipv4_str;

    last_handshake = peer->last_handshake_time.tv_sec;
    persistent_keepalive = peer->persistent_keepalive_interval;
    rx = peer->rx_bytes;
    tx = peer->tx_bytes;

    wg_key_b64_string key;
    wg_key_to_base64(key, peer->public_key);
    public_key = key;

    wg_allowedip *allowed_ip = nullptr;
    wg_for_each_allowedip(peer, allowed_ip) {
        inet_ntop(AF_INET, &(allowed_ip->ip4.s_addr), ipv4_str, INET_ADDRSTRLEN);
        allowed_ips.push_back(ipv4_str);
    }
}

Device::Device() {}

Device::Device(wg_device* device) {
    name = device->name;
    listen_port = device->listen_port;

    wg_key_b64_string key;
    wg_key_to_base64(key, device->public_key);
    public_key = key;

    wg_peer *peer;
    wg_for_each_peer(device, peer) {
        peers.push_back(peer);
    }
}

int count_devices() {
    char *device_names = wg_list_device_names();
    if (device_names == NULL) {
        return -1;
    }

    char *device = nullptr;
    size_t len = 0;
    int ret = 0;
    wg_for_each_device_name(device_names, device, len) {
        ret++;
    }

    free(device_names);
    return ret;
}

std::vector<std::string> list_device_names() {
    std::vector<std::string> ret;

    char *device_names = wg_list_device_names();
    if (device_names == NULL) {
        return ret;
    }

    char *device = nullptr;
    size_t len = 0;
    wg_for_each_device_name(device_names, device, len) {
        ret.push_back(device);
    }

    free(device_names);
    return ret;
}

WGDeviceMap get_devices() {
    WGDeviceMap ret;

    char *device_names = wg_list_device_names();
    if (device_names == NULL) {
        return ret;
    }

    char *device_name = nullptr;
    wg_device *device = nullptr;
    size_t len = 0;
    wg_for_each_device_name(device_names, device_name, len) {
        int ok = wg_get_device(&device, device_name);
        if (ok || device == nullptr) {
            SPDLOG_WARN("Failed to get device {}", device_name);
            SPDLOG_WARN("Note that this program must be run as root");
            continue;
        }
        ret[device->name] = device;
        wg_free_device(device);
    }

    free(device_names);
    return ret;
}

bool get_device(Device &output, std::string_view device_name) {
    wg_device *device = nullptr;
    wg_get_device(&device, device_name.cbegin());
    if (device == nullptr) {
        return false;
    }
    output = Device(device);
    return true;
}

}