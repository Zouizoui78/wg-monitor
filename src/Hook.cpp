#include "Hook.hpp"

#include "fmt/core.h"
#include "log.hpp"
#include "tools.hpp"

#define CPPHTTPLIB_OPENSSL_SUPPORT
#include "httplib.h"

using json = nlohmann::json;

std::vector<std::string> Hook::variables {
    "{PeerIP}",
    "{DeviceName}"
};

std::shared_ptr<Hook> Hook::factory(std::string_view type) {
    if (type == "webhook") {
        return std::make_shared<Webhook>();
    }
    else {
        return std::make_shared<DummyHook>();
    }
}

std::shared_ptr<Hook> Hook::factory(const json& j) {
    std::shared_ptr<Hook> hook;

    if (j["type"] == "webhook") {
        hook = std::make_shared<Webhook>(j);
    }
    else {
        hook = std::make_shared<DummyHook>(j);
    }

    hook->compile_pattern();
    return hook;
}

void Hook::compile_pattern() {
    SPDLOG_DEBUG("Compiling pattern :\n{}", pattern.dump(4));
    _compiled_pattern = "{" + pattern.dump() + "}";
    for (int i = 0; const auto &variable : variables) {
        // Replace variables names by strings like {0}, {1}, etc
        // for fmt formatting
        std::string i_str = fmt::format("{{{}}}", i);
        _compiled_pattern = tools::replace_all(_compiled_pattern, variable, i_str);
        i++;
    }
    SPDLOG_DEBUG("Compiled pattern : {}", _compiled_pattern);
}

std::string Hook::format(const wg::Device &device, const wg::Peer &peer) const {
    std::string formatted_pattern = fmt::format(
        fmt::runtime(_compiled_pattern),
        peer.allowed_ips[0],
        device.name
    );
    SPDLOG_DEBUG("Formatted pattern = {}", formatted_pattern);
    return formatted_pattern;
}


// WEBHOOK
bool Webhook::run(const wg::Device &device, const wg::Peer &peer) const {
    SPDLOG_DEBUG("Running hook {}", json(*this).dump(4));
    std::string formatted_pattern = format(device, peer);
    httplib::Client client(host);
    client.Post(url, formatted_pattern, "application/json");
    SPDLOG_DEBUG("Hook done");
    return true;
}


// DUMMY HOOK
bool DummyHook::run(const wg::Device &device, const wg::Peer &peer) const {
    SPDLOG_WARN("Dummy hook running");
    return true;
}