#ifndef HTTP_SERVER_HPP
#define HTTP_SERVER_HPP

#include "Hook.hpp"
#include "httplib.h"

class HTTPServer : public httplib::Server {
    public:
    HTTPServer();

    bool is_ok();

    private:
    void register_handlers();

    // api
    void get_device_names(const httplib::Request &req, httplib::Response &res);
    void get_devices(const httplib::Request &req, httplib::Response &res);
    void get_device(const httplib::Request &req, httplib::Response &res);
    void get_device_peers(const httplib::Request &req, httplib::Response &res);

    httplib::Server::HandlerResponse pre_routing(const httplib::Request &req, httplib::Response &res);

    // hooks
    bool parse_hooks();
    std::vector<Hook> _hooks;

    bool _is_ok = false;
};

#endif // HTTP_SERVER_HPP