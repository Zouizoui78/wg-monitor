#ifndef HTTP_SERVER_HPP
#define HTTP_SERVER_HPP

#include "httplib.h"

class HTTPServer : public httplib::Server {
    public:
    HTTPServer();

    private:
    void register_handlers();

    // api
    void get_device_names(const httplib::Request &req, httplib::Response &res);
    void get_devices(const httplib::Request &req, httplib::Response &res);
    void get_device(const httplib::Request &req, httplib::Response &res);
    void get_peers(const httplib::Request &req, httplib::Response &res);
};

#endif // HTTP_SERVER_HPP