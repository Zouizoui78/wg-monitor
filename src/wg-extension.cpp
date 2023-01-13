#include "wg-extension.hpp"

#include <arpa/inet.h> // inet_ntop
#include <cstring> // Needed for wg_for_each_device_name

using json = nlohmann::json;

namespace wg {

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

json device_to_json(wg_device *device) {
    json ret;
    ret["name"] = device->name;
    ret["listen_port"] = device->listen_port;

    wg_key_b64_string key;
    wg_key_to_base64(key, device->public_key);
    ret["public_key"] = key;

    ret["peers"] = std::vector<json>();
    wg_peer *peer;
    wg_for_each_peer(device, peer) {
        ret["peers"].push_back(peer_to_json(peer));
    }

    return ret;
}

json peer_to_json(wg_peer *peer) {
    json ret;

    char ipv4_str[INET_ADDRSTRLEN] = "";
    inet_ntop(AF_INET, &(peer->endpoint.addr4.sin_addr.s_addr), ipv4_str, INET_ADDRSTRLEN);
    ret["endpoint"] = ipv4_str;

    ret["last_handshake"] = peer->last_handshake_time.tv_sec;
    ret["persistent_keepalive"] = peer->persistent_keepalive_interval;
    ret["rx"] = peer->rx_bytes;
    ret["tx"] = peer->tx_bytes;

    wg_key_b64_string key;
    wg_key_to_base64(key, peer->public_key);
    ret["public_key"] = key;

    ret["allowed_ips"] = json();
    wg_allowedip *allowed_ip = nullptr;
    wg_for_each_allowedip(peer, allowed_ip) {
        inet_ntop(AF_INET, &(allowed_ip->ip4.s_addr), ipv4_str, INET_ADDRSTRLEN);
        ret["allowed_ips"].push_back(ipv4_str);
    }

    return ret;
}

nlohmann::json get_device_json(std::string_view device_name) {
    json ret;
    wg_device *device = nullptr;
    int ok = wg_get_device(&device, device_name.cbegin());
    if (ok < 0 || device == nullptr) {
        return ret;
    }

    ret = wg::device_to_json(device);
    wg_free_device(device);
    return ret;
}

}