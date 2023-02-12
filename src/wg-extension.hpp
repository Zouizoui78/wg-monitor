#ifndef WG_EXTENSION
#define WG_EXTENSION

#include <string>
#include <vector>

#include "json.hpp"

extern "C" {
#include "wireguard.h"
}

namespace wg {

struct Peer {
    Peer();
    Peer(wg_peer* peer);

    std::string public_key;
    std::string endpoint;

    int64_t last_handshake;
    uint64_t rx, tx;
    uint16_t persistent_keepalive;

    std::vector<std::string> allowed_ips;
};

struct Device {
    Device();
    Device(wg_device* device);

    std::string name;
    uint16_t listen_port;

    std::string public_key;

    std::vector<Peer> peers;
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(
    Peer,
    public_key,
    endpoint,
    last_handshake,
    rx, tx,
    persistent_keepalive,
    allowed_ips
);

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(
    Device,
    name,
    listen_port,
    public_key,
    peers
);

using WGDeviceMap = std::map<std::string, Device>;

int count_devices();
std::vector<std::string> list_device_names();
WGDeviceMap get_devices();
bool get_device(Device &output, std::string_view device_name);

}

#endif // WG_EXTENSION