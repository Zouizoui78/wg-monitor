#include "Hook.hpp"

#include "fmt/core.h"
#include "log.hpp"
#include "tools.hpp"

#define CPPHTTPLIB_OPENSSL_SUPPORT
#include "httplib.h"

using json = nlohmann::json;

std::vector<std::string> Hook::variables  {
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
    if (j["type"] == "webhook") {
        return std::make_shared<Webhook>(j);
    }
    else {
        return std::make_shared<DummyHook>();
    }
}

bool Webhook::run(const wg::Device &device, const wg::Peer &peer) const {
    SPDLOG_DEBUG("Running hook {}", json(*this).dump(4));
    SPDLOG_DEBUG("Pattern = {}", pattern);
    std::string pattern_copy = pattern
    ;
    for (int i = 0; const auto &variable : variables) {
        std::string i_str = fmt::format("{{{}}}", i);
        pattern_copy = tools::replace_all(pattern_copy, variable, i_str);
        i++;
    }
    SPDLOG_DEBUG("Pattern after substitution = {}", pattern_copy);
    std::string formatted_pattern = fmt::format(
        fmt::runtime(pattern_copy),
        peer.allowed_ips[0],
        device.name
    );
    SPDLOG_DEBUG("Formatted pattern = {}", formatted_pattern);
    httplib::Client client(host);
    return true;
}

bool DummyHook::run(const wg::Device &device, const wg::Peer &peer) const {
    SPDLOG_WARN("Dummy hook running");
    return true;
}