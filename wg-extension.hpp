#ifndef WG_EXTENSION
#define WG_EXTENSION

#include <string>
#include <vector>

#include "json.hpp"

extern "C" {
#include "wireguard.h"
}

namespace wg {
    int count_devices();
    std::vector<std::string> list_device_names();

    // json conversions
    nlohmann::json device_to_json(wg_device *device);
    nlohmann::json peer_to_json(wg_peer *peer);
    nlohmann::json get_device_json(std::string_view device_name);
}

#endif // WG_EXTENSION