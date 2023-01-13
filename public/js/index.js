function get_dom_node_by_id(id) {
    var ret = document.getElementById(id);
    if (!ret)
        console.error("Could not find id: " + id);
    return ret;
}

function remove_dom_node_children(node) {
    node.innerHTML = "";
}

function get_devices() {
    ajax.get(
        "/api/device",
        null,
        get_devices_success,
        get_devices_error
    );
}

function get_devices_success(res) {
    if (typeof(res) != "string") {
        console.log("get_devices : received data is not a string");
        return;
    }
    var devices = JSON.parse(res);
    load_devices(devices);
}

function get_devices_error(res) {
    let body = get_dom_node_by_id("body");
    body.innerHTML = res;
}

function load_devices(devices) {
    let body = get_dom_node_by_id("body");
    let device_names = Object.keys(devices).sort();

    for (let device_name of device_names) {
        let device_el = document.createElement("div");

        let title_el = document.createElement("h2");
        title_el.innerHTML = device_name;
        device_el.appendChild(title_el);

        let peers = devices[device_name]["peers"];
        for (let peer of peers) {
            let el = document.createElement("p");
            let ip = peer["allowed_ips"][0];

            let handshake = peer["last_handshake"];
            if (handshake == 0) {
                handshake = "No handshake";
            }
            else {
                handshake = new Date(peer["last_handshake"] * 1000);
                handshake = handshake.toLocaleDateString() + " "
                    + handshake.toLocaleTimeString();
            }

            el.innerHTML = ip + " : " + handshake;
            device_el.appendChild(el);
        }
        body.appendChild(device_el);
    }
}

document.addEventListener("DOMContentLoaded", function(event) {
    get_devices();
});