#ifndef MONITOR_HPP
#define MONITOR_HPP

#include "Hook.hpp"
#include "wg-extension.hpp"

namespace wg {

using HooksVector = std::vector<std::shared_ptr<Hook>>;

class Monitor {
    public:
    Monitor();
    ~Monitor();

    bool is_ok();

    bool is_running();
    void start();
    void stop();

    private:

    bool _is_ok = false;

    // hooks
    bool process_devices(const WGDeviceMap& devices);
    bool process_device(const wg::Device &device, const wg::Device &previous_device);
    bool process_peer(const wg::Device &device, const wg::Peer &peer, const wg::Peer &previous_peer);

    void handshake_hook_impl(const wg::Device &device, const Peer &peer);

    bool parse_hooks();

    HooksVector get_hooks_by_events(HookEvents events);
    HooksVector _hooks;

    // background monitoring
    std::atomic<bool> _running = false;
    std::thread _thread;
    WGDeviceMap _previous_devices;
    void monitor();
};

} // namespace wg

#endif