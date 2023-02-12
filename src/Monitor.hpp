#ifndef MONITOR_HPP
#define MONITOR_HPP

#include "Hook.hpp"

namespace wg {

class Monitor {
    public:
    Monitor();
    ~Monitor();

    bool is_ok();

    bool is_running();
    void start();
    void stop();

    private:

    // hooks
    bool parse_hooks();
    std::vector<Hook> _hooks;

    bool _is_ok = false;

    // background monitoring
    std::atomic<bool> _running = false;
    std::thread _thread;
    void monitor();
};

} // namespace wg

#endif