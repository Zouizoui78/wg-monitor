#ifndef HOOK_HPP
#define HOOK_HPP

#include "nlohmann/json.hpp"
#include <string>

enum HookType {
    WEBHOOK
};

enum HookEvents {
    NONE = 0,
    PEER_HANDSHAKE = 0b01,
    PEER_CONNECTION_LOST = 0b10,
    PEER_ENDPOINT_CHANGED = 0b100,
    NEW_PEER = 0b1000
};

struct Hook {
    uint8_t type = WEBHOOK;
    uint8_t events = NONE;
    std::string url{""};
    std::string pattern{""};
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Hook, type, events, url, pattern);

#endif // HOOK_HPP