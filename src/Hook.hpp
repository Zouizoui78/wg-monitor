#ifndef HOOK_HPP
#define HOOK_HPP

#include "nlohmann/json.hpp"
#include <string>
#include "wg-extension.hpp"

enum HookEvents {
    PEER_HANDSHAKE = 0b1,
    PEER_CONNECTION_LOST = 0b10,
    PEER_ENDPOINT_CHANGED = 0b100,
    PEER_ADDED = 0b1000
};

/**
 * Hook structure
 * Variables : PeerIP, DeviceName
 */
struct Hook {
    uint8_t events = 0;
    nlohmann::json pattern {};
    nlohmann::json exclude {};

    [[nodiscard]]
    virtual bool run(const wg::Device &device, const wg::Peer &peer) const = 0;

    [[nodiscard]]
    bool is_excluded(const wg::Device &device, const wg::Peer &peer);

    [[nodiscard]]
    static std::shared_ptr<Hook> factory(std::string_view type);

    [[nodiscard]]
    static std::shared_ptr<Hook> factory(const nlohmann::json& json);

protected:
    static std::vector<std::string> variables;

    void compile_pattern();

    [[nodiscard]]
    bool check_exclude(std::string_view variable_name, std::string_view value);

    [[nodiscard]]
    std::string format(const wg::Device &device, const wg::Peer &peer) const;
    std::string _compiled_pattern { "" };
};

struct Webhook : Hook {
    std::string host { "" };
    std::string url { "" };

    [[nodiscard]]
    virtual bool run(const wg::Device &device, const wg::Peer &peer) const override;
};

struct DummyHook : Hook {
    [[nodiscard]]
    virtual bool run(const wg::Device &device, const wg::Peer &peer) const override;
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Hook, events, pattern, exclude);

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(
    Webhook,
    events,
    pattern,
    exclude,
    host,
    url
);

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(DummyHook, events, pattern, exclude);

#endif // HOOK_HPP