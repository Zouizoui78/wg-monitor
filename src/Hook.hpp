#ifndef HOOK_HPP
#define HOOK_HPP

#include "nlohmann/json.hpp"
#include <string>
#include "wg-extension.hpp"

enum HookType {
    WEBHOOK
};

enum HookEvents {
    PEER_HANDSHAKE = 0b1,
    PEER_CONNECTION_LOST = 0b10,
    PEER_ENDPOINT_CHANGED = 0b100,
    PEER_ADDED = 0b1000
};

struct Hook {
    uint8_t type = WEBHOOK;
    uint8_t events = 0;
    std::string url{""};
    std::string pattern{""};

    bool run(const wg::Device &device, const wg::Peer &peer);
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Hook, type, events, url, pattern);

#endif // HOOK_HPP