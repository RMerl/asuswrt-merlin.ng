export function isSupport(_ptn) {
    const ui_support = [<% get_ui_support();%>][0];
    return (ui_support[_ptn]) ? ui_support[_ptn] : 0;
}

/**
 * Returns an array of {ssid, wl} for all MAINFH profiles.
 * wl is the actual interface name (e.g. "wl0.1", "wl1.1").
 * ap_wifi_rl format: "<node>wlIface1,wlIface2>sdnIdx<...>"
 */
export function getMainFhIfname() {
    let main_fh_info = [];
    if (isSupport("sdn_mainfh")) {
        const nvramData = httpApi.nvramCharToAscii(["sdn_rl", "ap_wifi_rl"], true);
        const sdn_rl = decodeURIComponent(nvramData.sdn_rl);
        const ap_wifi_rl = decodeURIComponent(nvramData.ap_wifi_rl || "");

        // Build map: sdnIdx → [wl interfaces] from ap_wifi_rl
        // ap_wifi_rl format: "<node>wlIface1,wlIface2>sdnIdx<node>wlIface>sdnIdx..."
        // e.g. "<0>wl0.1,wl2.1>2><0>wl1.1>5><53>wl0.2,wl1.2>4"
        const sdnToWl = {};
        ap_wifi_rl.split("<").forEach(entry => {
            if (!entry) return;
            const parts = entry.split(">");
            if (parts.length < 3) return;
            // parts[0] = node MAC (or 0), parts[1] = comma-separated wl ifaces, parts[2] = sdnIdx
            const sdnIdx = parts[2];
            parts[1].split(",").forEach(wl => {
                if (!wl) return;
                if (!sdnToWl[sdnIdx]) sdnToWl[sdnIdx] = [];
                if (!sdnToWl[sdnIdx].includes(wl)) sdnToWl[sdnIdx].push(wl);
            });
        });

        const mainFH = sdn_rl.split("<").filter(item => item.includes("MAINFH"));
        mainFH.forEach(item => {
            if (!item) return;
            const parts = item.split(">");
            const sdnIdx = parts[0];
            const apmIdx = parts[5];
            const apm_config = httpApi.nvramCharToAscii([`apm${apmIdx}_ssid`], true);
            const apm_ssid = decodeURIComponent(apm_config[`apm${apmIdx}_ssid`]);
            const wlInterfaces = sdnToWl[sdnIdx] || [];
            if (wlInterfaces.length > 0) {
                wlInterfaces.forEach(wl => {
                    main_fh_info.push({"ssid": apm_ssid, "wl": wl});
                });
            } else {
                main_fh_info.push({"ssid": apm_ssid, "wl": ""});
            }
        });
    }
    if (main_fh_info.length === 0) {
        main_fh_info = [{"ssid": "", "wl": ""}];
    }
    return main_fh_info;
}

