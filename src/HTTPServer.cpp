#include "HTTPServer.hpp"
#include "json.hpp"
#include "log.hpp"
#include "wg-extension.hpp"

using namespace std::placeholders;
using json = nlohmann::json;

#define REGISTER_HANDLER(__route, __handler) \
this->Get(__route, std::bind(&HTTPServer::__handler, this, _1, _2));

HTTPServer::HTTPServer() : httplib::Server() {
    register_handlers();
    this->set_mount_point("/", "./public");
}

void HTTPServer::register_handlers() {
    // We go from the most specific to the less specific route
    // otherwise less specific can match instead of more specific
    REGISTER_HANDLER(R"(/api/devices/(.+)/peers)", get_device_peers);
    REGISTER_HANDLER(R"(/api/devices/(.+))", get_device);
    REGISTER_HANDLER("/api/devices", get_devices);
    REGISTER_HANDLER("/api/devicenames", get_device_names);

#ifdef DEBUG
    set_pre_routing_handler(std::bind(&HTTPServer::pre_routing, this, _1, _2));
#endif
}

void HTTPServer::get_device_names(const httplib::Request &req, httplib::Response &res) {
    auto device_names = wg::list_device_names();
    res.set_content(json(device_names).dump(), "application/json");
}

void HTTPServer::get_devices(const httplib::Request &req, httplib::Response &res) {
    auto device_names = wg::list_device_names();
    json devices;

    int ok = 0;
    for (std::string_view device_name : device_names) {
        json device_json = wg::get_device_json(device_name);
        if (device_json.empty()) {
            continue;
        }
        devices[device_name] = device_json;
    }

    res.set_content(json(devices).dump(), "application/json");
}

void HTTPServer::get_device(const httplib::Request &req, httplib::Response &res) {
    std::string device_name = req.matches[1];
    json device_json = wg::get_device_json(device_name);
    res.set_content(device_json.dump(), "application/json");
}

void HTTPServer::get_device_peers(const httplib::Request &req, httplib::Response &res) {
    std::string device_name = req.matches[1];
    json device_json = wg::get_device_json(device_name);
    res.set_content(device_json["peers"].dump(), "application/json");
}

httplib::Server::HandlerResponse HTTPServer::pre_routing(const httplib::Request &req, httplib::Response &res) {
    SPDLOG_DEBUG("{} {}", req.method, req.path);
    return HandlerResponse::Unhandled;
}