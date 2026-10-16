export function isSupport(_ptn) {
    const ui_support = [<% get_ui_support();%>][0];
    return (ui_support[_ptn]) ? ui_support[_ptn] : 0;
}

export function isSwMode(mode) {
    const ui_sw_mode = [<% get_operation_mode();%>][0];
    return ui_sw_mode == String(mode).toLowerCase();
}

const ARK_FEATURE_LABELS = {
    ark_qoe: `<#Adaptive_QoE#>`,
    ark_limiter: `<#Bandwidth_Limiter#>`,
    ark_log: `<#Adaptive_History#>`,
};

/**
 * Confirms with the user before enabling Ark features that would disable
 * hardware NAT acceleration. Reads the live `ark_feature_nat_notice` hook.
 *
 * @param {string|string[]} featureKey - key(s) to check, e.g. "ark_qoe" or ["ark_qoe","ark_limiter"]
 * @returns {boolean} true if no prompt needed or the user confirmed; false if user cancelled
 */
export function confirmArkFeatureNatNotice(featureKey) {
    const notice = httpApi.hookGet("ark_feature_nat_notice") || {};
    const keys = Array.isArray(featureKey) ? featureKey : [featureKey];

    if (!keys.some(k => notice[k] == "1")) return true;

    const featureList = Object.keys(ARK_FEATURE_LABELS)
        .filter(k => isSupport(k) == "1" && notice[k] == "1" && (k !== "ark_limiter" || isSupport("UI4") == "1"))
        .map(k => ARK_FEATURE_LABELS[k])
        .join(", ");

    const confirm_string = `<#Gear_Accelerator_confirm#>`.replaceAll('%@', featureList);
    return confirm(confirm_string);
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
/**
 * Self-contained input validators, extracted from validator.js so ES-module
 * pages (e.g. clientlist.module.js) don't have to pull in the legacy global
 * `validator` object or state.js.
 *
 * Deliberately a MINIMAL copy — only the paths those pages actually exercise —
 * to keep the maintenance overlap with validator.js small:
 *   - validIPForm: IPv4 "without netmask" (validator.js flag 0), the only form
 *     clientlist calls. validator.js's WAN/LAN/netmask/sw_mode/document.form
 *     branches never run for a nameless client-IP field, so they're omitted here.
 *   - haveFullWidthChar / lengthInUtf8: copied as-is (already tiny, no deps).
 * Dict tokens (<#JS_validip#>) are rendered server-side by renderAsp.
 */

export function haveFullWidthChar(obj) {
    var re = /[^\x00-\xff]/g;
    if (obj.value.match(re)) {
        return false;
    }
    else {
        return true;
    }
}

export function lengthInUtf8(str) {
    var asciiLength = str.match(/[\x00-\x7f]/g) ? str.match(/[\x00-\x7f]/g).length : 0;
    var multiByteLength = encodeURI(str.replace(/[\x00-\x7f]/g)).match(/%/g) ? encodeURI(str.replace(/[\x00-\x7f]/g, '')).match(/%/g).length : 0;
    return asciiLength + multiByteLength;
}

// Mirrors validator.ipAddrFinal's failure side effects exactly:
// alert, clear the field, refocus/select, then return false.
function rejectIp(o) {
    alert(o.value + " <#JS_validip#>");
    o.value = "";
    o.focus();
    o.select();
    return false;
}

/**
 * IPv4 format check, "without netmask" (validator.js validIPForm flag 0) for a
 * nameless field: empty is accepted; a valid dotted-quad (each octet 0-255) is
 * normalized to "a.b.c.d" and returns true; anything else is rejected via
 * rejectIp(). The trailing `flag` arg from legacy call sites is accepted and
 * ignored (only the no-netmask form is supported here).
 */
export function validIPForm(obj) {
    if (obj.value === "") {
        return true;
    }

    var num = -1, pos = 0, v1, v2, v3, v4;
    for (var i = 0; i < obj.value.length; ++i) {
        var c = obj.value.charAt(i);
        if (c >= '0' && c <= '9') {
            num = (num === -1) ? (c - '0') : num * 10 + (c - '0');
        }
        else {
            if (num < 0 || num > 255 || c !== '.') {
                return rejectIp(obj);
            }
            if (pos === 0) v1 = num;
            else if (pos === 1) v2 = num;
            else if (pos === 2) v3 = num;
            num = -1;
            ++pos;
        }
    }

    if (pos !== 3 || num < 0 || num > 255) {
        return rejectIp(obj);
    }
    v4 = num;

    obj.value = v1 + "." + v2 + "." + v3 + "." + v4;
    return true;
}
