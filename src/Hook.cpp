#include "Hook.hpp"

#include "fmt/core.h"
#include "log.hpp"
#include "tools.hpp"

#define CPPHTTPLIB_OPENSSL_SUPPORT
#include "httplib.h"

using json = nlohmann::json;

std::vector<std::string> Hook::variables {
    "PeerIP",
    "DeviceName"
};

bool Hook::is_excluded(const wg::Device &device, const wg::Peer &peer) {
    return  check_exclude("PeerIP", peer.allowed_ips[0]) ||
            check_exclude("DeviceName", device.name);
}

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
    SPDLOG_DEBUG("Compiling pattern :\n{}", pattern.dump(2));
    // Add braces around the pattern to escape the json braces
    _compiled_pattern = "{" + pattern.dump() + "}";
    for (int i = 0; const auto &variable : variables) {
        // Replace variables names by strings like {0}, {1}, etc
        // for fmt formatting
        std::string i_str = fmt::format("{{{}}}", i);
        // Add braces to make a fmt placeholder
        std::string to_replace { "{" + variable + "}" };
        _compiled_pattern = tools::replace_all(_compiled_pattern, to_replace, i_str);
        i++;
    }
    SPDLOG_DEBUG("Compiled pattern : {}", _compiled_pattern);
}

bool Hook::check_exclude(std::string_view variable_name, std::string_view value) {
    if (this->exclude.is_null()) {
        return false;
    }

    if (!this->exclude.contains(variable_name)) {
        return false;
    }

    auto find_it = std::find(
        this->exclude[variable_name].begin(),
        this->exclude[variable_name].end(),
        value
    );

    return find_it != this->exclude[variable_name].end();
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
    SPDLOG_DEBUG("Running hook {}", json(*this).dump(2));
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