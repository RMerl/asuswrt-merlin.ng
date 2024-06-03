httpApi.hookGetMore([
    "get_clientlist",
    "get_clientlist_from_json_database",
    "channel_list_2g",
    "channel_list_5g",
    "channel_list_5g_2",
    "channel_list_6g",
    "channel_list_6g_2",
    "chanspecs_2g",
    "chanspecs_5g",
    "chanspecs_5g_2",
    "chanspecs_6g",
    "chanspecs_6g_2",
    "get_ui_support",
    "language_support_list",
    "wl_cap_2g",
    "wl_cap_5g",
    "wl_cap_5g_2",
    "wl_cap_6g",
    "wl_cap_6g_2",
    "wl_nband_info",
    "get_label_mac",
    "uptime",
    "get_cfg_clientlist",
    "wl_control_channel",
    "get_wl_channel_list_2g",
    "get_wl_channel_list_5g",
    "get_wl_channel_list_5g_2",
    "get_wl_channel_list_6g",
    "get_wl_channel_list_6g_2",
]);

var ui_support = httpApi.hookGet("get_ui_support");
let timeArray = "";
function isSupport(_ptn){
	var ui_support = [<% get_ui_support(); %>][0];
	return (ui_support[_ptn]) ? ui_support[_ptn] : 0;
}

function territoryCodeCheck(tcode) {
    if (tcode === "") {
        return false;
    }

    let { territory_code } = nvram;
    return territory_code.search(tcode) !== -1;
}

class WirelessAttribute {
    constructor() {
        this.wlUnit = "";
        this.name = "";
        this.postfixHook = "";
        this.capability = [];
        this.channel = [];
        this.pscChannel = [];
        this.chanspecs = [];
        this.ch320MHz = {};
        this.ch160MHz = {};
        this.ch80_80MHz = {};
        this.ch80MHz = {};
        this.ch40MHz = {};
        this.ch20MHz = {};
        this.dfsSupport = false;
        this.acsCH13Support = false;
        this.uNII4Support = false;
        // this.countryCode = "";
        // this.sdkVersion = "";
        // this.chipNumber = "";
        this.nSupport = true;
        this.acSupport = false;
        this.axSupport = false;
        this.beSupport = false;

        // this.noVHTSupport = true;
        this.bw20MHzSupport = false;
        this.bw40MHzSupport = false;
        this.bw80MHzSupport = false;
        this.bw80_80MHzSupport = false;
        this.bw160MHzSupport = false;
        this.bw320MHzSupport = false;

        // this.acsBand1Support = false;
        // this.acsBand3Support = false;
        // this.QAM256Support = false;
        // this.QAM1024Support = false;

        this.heFrameSupport = false;
        this.mboSupport = false;
        this.twtSupport = false;
        this.xboxOptSupport = false;
        this.bgProtectSupport = false;
        this.disable11bSupport = false;

        this.wlMode = [];
        this.authMethod = [];
        this.joinSmartConnect = false;
        /* NVRAM value */
        this.ssidValue = "";
        this.hideSSIDValue = "";
        this.wlModeValue = "";
        this.bandwidthValue = "";
        this.bw160Value = "";
        this.channelValue = "";
        this.extChannelValue = "";
        this.authMethodValue = "";
        this.wpaEncryptValue = "";
        this.wpaKeyValue = "";
        this.protectManageFrameValue = "";
        this.wpaGroupKeyIntervalValue = "";
        this.radiusIpValue = "";
        this.radiusPortValue = "";
        this.radiusKeyValue = "";
        this.curCtrlChannel = "";
        this.mfpValue = "";
        this.wpaGtkValue = "";
        this.wepEncryptValue = "";
        this.wepKeyIndexValue = "";
        this.wepKey1Value = "";
        this.wepKey2Value = "";
        this.wepKey3Value = "";
        this.wepKey4Value = "";
        this.wepPassPhraseValue = "";
    }
}
let nvram = httpApi.nvramGet([
    "productid",
    "odmpid",
    "preferred_lang",
    "sw_mode",
    "wlc_psta",
    "wlc_express",
    "firmver",
    "buildno",
    "extendno",
    "swpjverno",
    "wlnband_list",
    "smart_connect_x",
    "smart_connect_selif_x",
    "acs_ch13",
    "acs_dfs",
    "acs_band3",
    "acs_unii4",
    "psc6g",
    "territory_code",
    "fh_ap_enabled",
    "dwb_mode",
    "dwb_band",
    "wps_enable",
    "location_code",
    "wlc_band",
    "mld_enable",
]);

let system = { time: "" };
system.productId = nvram.productid;
system.modelName = (() => {
    let { odmpid, productid } = nvram;
    return odmpid ? odmpid : productid;
})();
system.isBRCMplatform = isSupport("bcmwifi");
system.isQCAplatform = isSupport("qcawifi");
system.isMTKplatform = isSupport("rawifi");
system.isINTEKplatform = isSupport("lantiq");
system.wpa3Support = isSupport("wpa3");
system.newWiFiCertSupport = isSupport("wifi2017");
system.wifiLogoSupport = isSupport("wifilogo");
system.wpa3EnterpriseSupport = isSupport("wpa3-e");
system.oweTransSupport = isSupport("owe_trans");
system.dualWANSupport = isSupport("dualwan");
system.dnsDpiSupport = isSupport("dns_dpi");
system.lyraHideSupport = isSupport("lyra_hide");
system.conCurrentRepeaterSupport = isSupport("concurrep");
system.lightEffectSupoort = isSupport("ledg");
system.mloSupport = isSupport("mlo");
system.isKRSku = territoryCodeCheck("KR");
system.isEUSku = territoryCodeCheck("EU");
system.isAASku = territoryCodeCheck("AA");
system.isAUSku = territoryCodeCheck("AU");
system.isCNSku = territoryCodeCheck("CN");
system.locationCode = nvram.location_code;
system.psc6g = nvram.psc6g;
system.acs_dfs = nvram.acs_dfs;
system.acs_band3 = nvram.acs_band3;
system.acs_unii4 = nvram.acs_unii4;
system.acs_ch13 = nvram.acs_ch13;
system.mloEnabled = (() => {
    let { mloSupport } = system;
    let { mld_enable } = nvram;
    return mloSupport && mld_enable === "1";
})();
system.fh_ap_enabled = nvram.fh_ap_enabled;
system.assassinMode = (() => {
    let { isCNSku, locationCode, productId } = system;
    return {
        assassinModeSupport: isCNSku && productId === "TUF-AX3000",
        assassinModeEnable: isCNSku && productId === "TUF-AX3000" && locationCode === "XX",
    };
})();
let wlPostfixIndexTransform = {
    "2g1": "2g",
    "5g1": "5g",
    "5g2": "5g_2",
    "6g1": "6g",
    "6g2": "6g_2",
};
system.aMesh = (() => {
    let { wlnband_list, dwb_mode, dwb_band } = nvram;
    let nBandArray = wlnband_list.split("&#60");
    let object = {};
    object.aMeshSupport = isSupport("amas");
    object.aMeshRouterSupport = isSupport("amasRouter");
    object.aMeshPrelinkSupport = isSupport("prelink");
    object.dwbMode = system.mloSupport ? "0" : dwb_mode;
    object.dwbBand = nBandArray[dwb_band];
    object.nodeList = (() => {
        let { aMeshSupport, aMeshRouterSupport } = object;
        let list = {};
        if (aMeshSupport && aMeshRouterSupport) {
            let cfg_clientlist = httpApi.hookGet("get_cfg_clientlist");
            list = JSON.parse(JSON.stringify(cfg_clientlist));

            // put a new attribute
            for (let item of list) {
                let { ui_model_name, model_name } = item;
                item["display_name"] = ui_model_name ? ui_model_name : model_name;
            }
        }

        return list;
    })();

    object.channel = (() => {
        let _channel = {};
        if (dwb_mode === "1") {
            nBandArray.forEach((element) => {
                let postfixIndex = wlPostfixIndexTransform[element];
                let dwbChannel = (() => httpApi.hookGet(`get_wl_channel_list_${postfixIndex}`) || {})();
                _channel[element] = dwbChannel;
            });

            return _channel;
        }

        return {};
    })();

    return object;
})();

// Smart Connect
system.smartConnect = (() => {
    let referenceArray = ["", "", "", "6g2", "6g1", "5g2", "5g1", "2g1"];
    let { smart_connect_x, smart_connect_selif_x } = nvram;
    let object = {};
    object.support = isSupport("smart_connect") || isSupport("bandstr") || isSupport("smart_connect_v2");
    object.version = isSupport("smart_connect_v2") ? "v2" : isSupport("smart_connect") || isSupport("bandstr") ? "v1" : "";
    object.v2Band = (() => {
        const maxLength = 8;
        let biString = parseInt(smart_connect_selif_x).toString(2);
        let bandArray = biString.split("");
        while (bandArray.length < maxLength) {
            bandArray.unshift("0");
        }

        return bandArray;
    })();

    object.radioSeqArray = referenceArray;
    object.smartConnectEnable = smart_connect_x !== "0";
    object.v1Type = smart_connect_x;
    object.smartConnectReferenceIndex = (() => {
        if (object.version === "v2") {
            let _index = object.v2Band.findLastIndex((element) => element === "1");
            return referenceArray[_index];
        } else {
            if (object.v1Type === "1" || object.v1Type === "3") {
                return "2g1";
            } else if (object.v1Type === "2") {
                return "5g1";
            }
        }

        return "2g1";
    })();

    return object;
})();

// Wireless
system.wlBandSeq = (() => {
    let wlObj = {};
    let { wlnband_list, dwb_mode } = nvram;
    let nBandArray = wlnband_list.split("&#60");
    let curCtrlChannelArray = httpApi.hookGet("wl_control_channel");
    let {
        isBRCMplatform,
        isEUSku,
        isAASku,
        isAUSku,
        productId,
        wpa3EnterpriseSupport,
        oweTransSupport,
        newWiFiCertSupport,
        wifiLogoSupport,
        smartConnect,
        aMesh,
    } = system;

    for (let i = 0; i < nBandArray.length; i++) {
        let wlIfIndex = nBandArray[i];
        let postfixIndex = "";
        let _nvram = httpApi.nvramGet([
            `${wlIfIndex}_nmode_x`,
            `${wlIfIndex}_closed`,
            `${wlIfIndex}_bw`,
            `${wlIfIndex}_chanspec`,
            `${wlIfIndex}_channel`,
            `${wlIfIndex}_nctrlsb`,
            `${wlIfIndex}_auth_mode_x`,
            `${wlIfIndex}_crypto`,
            `${wlIfIndex}_mfp`,
            `${wlIfIndex}_wpa_gtk_rekey`,
            `${wlIfIndex}_radius_ipaddr`,
            `${wlIfIndex}_radius_port`,
            `${wlIfIndex}_radius_key`,
            `${wlIfIndex}_bw_160`,
            `${wlIfIndex}_wep_x`,
            `${wlIfIndex}_key`,
            `${wlIfIndex}_key1`,
            `${wlIfIndex}_key2`,
            `${wlIfIndex}_key3`,
            `${wlIfIndex}_key4`,
            `${wlIfIndex}_phrase_x`,
            `${wlIfIndex}_11be`,
        ]);

        let _nvramAscii = httpApi.nvramCharToAscii([`${wlIfIndex}_ssid`, `${wlIfIndex}_wpa_psk`]);
        postfixIndex = wlPostfixIndexTransform[wlIfIndex];
        wlObj[wlIfIndex] = new WirelessAttribute();
        wlObj[wlIfIndex].wlUnit = i;
        wlObj[wlIfIndex].name = (() => {
            let wlIfNameObject = {
                "2g1": "2.4 GHz",
                "5g1": (() => (nBandArray.find((element) => element === "5g2") === "5g2" ? "5 GHz-1" : "5 GHz"))(),
                "5g2": "5 GHz-2",
                "6g1": (() => (nBandArray.find((element) => element === "6g2") === "6g2" ? "6 GHz-1" : "6 GHz"))(),
                "6g2": "6 GHz-2",
            };

            return wlIfNameObject[wlIfIndex];
        })();
        wlObj[wlIfIndex].postfixHook = postfixIndex;
        wlObj[wlIfIndex].prefixNvram = wlIfIndex;
        wlObj[wlIfIndex].capability = (() => {
            let _cap = httpApi.hookGet(`wl_cap_${postfixIndex}`);
            return _cap ? _cap.trim().split(" ") : [];
        })();
        wlObj[wlIfIndex].ssidValue = decodeURIComponent(_nvramAscii[`${wlIfIndex}_ssid`]);
        wlObj[wlIfIndex].hideSSIDValue = _nvram[`${wlIfIndex}_closed`];
        wlObj[wlIfIndex].wlModeValue = _nvram[`${wlIfIndex}_nmode_x`];
        wlObj[wlIfIndex].bandwidthValue = _nvram[`${wlIfIndex}_bw`];
        wlObj[wlIfIndex].bw160Value = _nvram[`${wlIfIndex}_bw_160`];
        wlObj[wlIfIndex].channelValue = isBRCMplatform ? _nvram[`${wlIfIndex}_chanspec`] : _nvram[`${wlIfIndex}_channel`];
        wlObj[wlIfIndex].extChannelValue = _nvram[`${wlIfIndex}_nctrlsb`];
        wlObj[wlIfIndex].authMethodValue = _nvram[`${wlIfIndex}_auth_mode_x`];
        wlObj[wlIfIndex].wpaEncryptValue = _nvram[`${wlIfIndex}_crypto`];
        wlObj[wlIfIndex].wpaKeyValue = decodeURIComponent(_nvramAscii[`${wlIfIndex}_wpa_psk`]);
        wlObj[wlIfIndex].protectManageFrameValue = _nvram[`${wlIfIndex}_mfp`];
        wlObj[wlIfIndex].wpaGroupKeyIntervalValue = _nvram[`${wlIfIndex}_wpa_gtk_rekey`];
        wlObj[wlIfIndex].radiusIpValue = _nvram[`${wlIfIndex}_radius_ipaddr`];
        wlObj[wlIfIndex].radiusPortValue = _nvram[`${wlIfIndex}_radius_port`];
        wlObj[wlIfIndex].radiusKeyValue = _nvram[`${wlIfIndex}_radius_key`];
        wlObj[wlIfIndex].channel = (() => {
            let _channel = [];
            let { channel } = aMesh;
            if (dwb_mode === "1") {
                _channel = (() => {
                    let chanlist = (() => (channel[wlIfIndex].chan_20m ? channel[wlIfIndex].chan_20m.chanlist : []))();
                    if (chanlist[0] === "0") {
                        chanlist.shift();
                    }

                    return chanlist;
                })();
            } else {
                _channel = httpApi.hookGet(`channel_list_${postfixIndex}`) || [];
            }

            if (wlIfIndex === "6g1" || wlIfIndex === "6g2") {
                _channel = _channel.filter((ch) => {
                    if (parseInt(ch) <= 221) {
                        return ch;
                    }
                });
            }

            return _channel.map((element) => element.toString());
        })();
        wlObj[wlIfIndex].curCtrlChannel = curCtrlChannelArray[i];
        wlObj[wlIfIndex].mfpValue = _nvram[`${wlIfIndex}_mfp`];
        wlObj[wlIfIndex].wpaGtkValue = _nvram[`${wlIfIndex}_wpa_gtk_rekey`];
        wlObj[wlIfIndex].wepEncryptValue = _nvram[`${wlIfIndex}_wep_x`];
        wlObj[wlIfIndex].wepKeyIndexValue = _nvram[`${wlIfIndex}_key`];
        wlObj[wlIfIndex].wepKey1Value = _nvram[`${wlIfIndex}_key1`];
        wlObj[wlIfIndex].wepKey2Value = _nvram[`${wlIfIndex}_key2`];
        wlObj[wlIfIndex].wepKey3Value = _nvram[`${wlIfIndex}_key3`];
        wlObj[wlIfIndex].wepKey4Value = _nvram[`${wlIfIndex}_key4`];
        wlObj[wlIfIndex].wepPassPhraseValue = _nvram[`${wlIfIndex}_phrase_x`];
        if (isBRCMplatform) {
            wlObj[wlIfIndex].chanspecs = (() => {
                if (dwb_mode === "1") {
                    let channel = [];
                    let dwbChannel = objectDeepCopy(aMesh.channel[wlIfIndex]);
                    if (dwbChannel.auto) {
                        delete dwbChannel["auto"];
                    }

                    for (let { chanspec } of Object.values(dwbChannel)) {
                        if (chanspec) {
                            if (chanspec[0] === "0") {
                                chanspec.shift();
                            }

                            channel = channel.concat(chanspec);
                        }
                    }

                    return channel;
                }

                return httpApi.hookGet(`chanspecs_${postfixIndex}`);
            })();
            wlObj[wlIfIndex].chanspecs.forEach((element) => {
                let _ch = element.split("u")[0].split("l")[0].split("/")[0];
                if (_ch.indexOf("6g") !== -1) {
                    _ch = _ch.split("6g")[1];
                }

                if (element.indexOf("u") !== -1 || element.indexOf("l") !== -1 || element.indexOf("/40") !== -1) {
                    if (wlObj[wlIfIndex].ch40MHz[_ch] === undefined) {
                        wlObj[wlIfIndex].ch40MHz[_ch] = [];
                    }

                    wlObj[wlIfIndex].ch40MHz[_ch].push(element);
                } else if (element.indexOf("/80") !== -1) {
                    if (wlObj[wlIfIndex].ch80MHz[_ch] === undefined) {
                        wlObj[wlIfIndex].ch80MHz[_ch] = [];
                    }

                    wlObj[wlIfIndex].ch80MHz[_ch].push(element);
                } else if (element.indexOf("/160") !== -1) {
                    if (productId === "ET8_V2" && wlIfIndex === "5g1") {
                        return;
                    }

                    if (wlObj[wlIfIndex].ch160MHz[_ch] === undefined) {
                        wlObj[wlIfIndex].ch160MHz[_ch] = [];
                    }

                    wlObj[wlIfIndex].ch160MHz[_ch].push(element);
                } else if (element.indexOf("/320") !== -1) {
                    if (wlObj[wlIfIndex].ch320MHz[_ch] === undefined) {
                        wlObj[wlIfIndex].ch320MHz[_ch] = [];
                    }

                    wlObj[wlIfIndex].ch320MHz[_ch].push(element);
                } else {
                    if (wlObj[wlIfIndex].ch20MHz[_ch] === undefined) {
                        wlObj[wlIfIndex].ch20MHz[_ch] = [];
                    }

                    wlObj[wlIfIndex].ch20MHz[_ch].push(element);
                }
            });
        } else {
            let wlChannels = wlObj[wlIfIndex].channel;
            for (let element of wlChannels) {
                // 20 MHz
                if (wlObj[wlIfIndex].ch20MHz[element] === undefined) {
                    wlObj[wlIfIndex].ch20MHz[element] = [];
                }

                wlObj[wlIfIndex].ch20MHz[element].push(element);
                let channelNumber = parseInt(element);
                let channelNumberOperation = channelNumber;
                // 40 MHz
                if (wlIfIndex === "2g1") {
                    if (channelNumber - 4 >= 0) {
                        if (wlObj[wlIfIndex].ch40MHz[element] === undefined) {
                            wlObj[wlIfIndex].ch40MHz[element] = [];
                        }

                        wlObj[wlIfIndex].ch40MHz[element].push(`${element}_lower`);
                    }

                    if (channelNumber + 4 <= wlChannels.length) {
                        if (wlObj[wlIfIndex].ch40MHz[element] === undefined) {
                            wlObj[wlIfIndex].ch40MHz[element] = [];
                        }

                        wlObj[wlIfIndex].ch40MHz[element].push(`${element}_upper`);
                    }
                } else {
                    let channelStr = "";
                    if (channelNumber > 144) {
                        channelNumberOperation -= 1;
                    }

                    if (channelNumberOperation % 8 === 0) {
                        channelStr = (channelNumber - 4).toString();
                    } else {
                        channelStr = (channelNumber + 4).toString();
                    }

                    if (wlChannels.indexOf(channelStr) !== -1) {
                        if (wlObj[wlIfIndex].ch40MHz[element] === undefined) {
                            wlObj[wlIfIndex].ch40MHz[element] = [];
                        }

                        wlObj[wlIfIndex].ch40MHz[element].push(element);
                    }
                }

                // 80 MHz
                if (wlIfIndex === "2g1") {
                    continue;
                }

                let bw80MHzSupport = isSupport("11AC");
                if (bw80MHzSupport) {
                    const bw80MaxCount = 4;
                    let bw80count = 0;
                    let channelPartition = {};
                    if (wlIfIndex === "5g1" || wlIfIndex === "5g2") {
                        channelPartition = {
                            3: ["36", "40", "44", "48"],
                            4: ["52", "56", "60", "64"],
                            7: ["100", "104", "108", "112"],
                            8: ["116", "120", "124", "128"],
                            9: ["132", "136", "140", "144"],
                            10: ["149", "153", "157", "161"],
                            11: ["165", "169", "173", "177"],
                        };

                        if (channelNumber > 144) {
                            channelNumberOperation -= 1;
                        }
                    } else if (wlIfIndex === "6g1" || wlIfIndex === "6g2") {
                        channelPartition = {
                            1: ["1", "5", "9", "13"],
                            2: ["17", "21", "25", "29"],
                            3: ["33", "37", "41", "45"],
                            4: ["49", "53", "57", "61"],
                            5: ["65", "69", "73", "77"],
                            6: ["81", "85", "89", "93"],
                            7: ["97", "101", "105", "109"],
                            8: ["113", "117", "121", "125"],
                            9: ["129", "133", "137", "141"],
                            10: ["145", "149", "153", "157"],
                            11: ["161", "165", "169", "173"],
                            12: ["177", "181", "185", "189"],
                            13: ["193", "197", "201", "205"],
                            14: ["209", "213", "217", "221"],
                            15: ["225", "229", "233", ""],
                        };

                        channelNumberOperation += 3;
                    }

                    let channelGroupIndex = Math.ceil(channelNumberOperation / 16);
                    let targetPartition = channelPartition[channelGroupIndex];
                    for (let _channel of targetPartition) {
                        wlChannels.includes(_channel) && bw80count++;
                    }

                    if (bw80MaxCount === bw80count) {
                        if (wlObj[wlIfIndex].ch80MHz[element] === undefined) {
                            wlObj[wlIfIndex].ch80MHz[element] = [];
                        }

                        wlObj[wlIfIndex].ch80MHz[element].push(element);
                    }
                }

                // 160 MHz
                let bw160MHzSupport = isSupport("vht160");
                if (bw160MHzSupport) {
                    const bw160MaxCount = 8;
                    let bw160count = 0;
                    let channelPartition = {};
                    if (wlIfIndex === "5g1" || wlIfIndex === "5g2") {
                        channelPartition = {
                            2: ["36", "40", "44", "48", "52", "56", "60", "64"],
                            4: ["100", "104", "108", "112", "116", "120", "124", "128"],
                            5: ["132", "136", "140", "144"],
                            6: ["149", "153", "157", "161", "165", "169", "173", "177"],
                        };

                        if (channelNumber > 144) {
                            channelNumberOperation -= 1;
                        }
                    } else if (wlIfIndex === "6g1" || wlIfIndex === "6g2") {
                        channelPartition = {
                            1: ["1", "5", "9", "13", "17", "21", "25", "29"],
                            2: ["33", "37", "41", "45", "49", "53", "57", "61"],
                            3: ["65", "69", "73", "77", "81", "85", "89", "93"],
                            4: ["97", "101", "105", "109", "113", "117", "121", "125"],
                            5: ["129", "133", "137", "141", "145", "149", "153", "157"],
                            6: ["161", "165", "169", "173", "177", "181", "185", "189"],
                            7: ["193", "197", "201", "205", "209", "213", "217", "221"],
                            8: ["225", "229", "233", ""],
                        };

                        channelNumberOperation += 3;
                    }

                    let channelGroupIndex = Math.ceil(channelNumberOperation / 32);
                    let targetPartition = channelPartition[channelGroupIndex];
                    for (let _channel of targetPartition) {
                        wlChannels.includes(_channel) && bw160count++;
                    }

                    if (bw160MaxCount === bw160count) {
                        if (wlObj[wlIfIndex].ch160MHz[element] === undefined) {
                            wlObj[wlIfIndex].ch160MHz[element] = [];
                        }

                        wlObj[wlIfIndex].ch160MHz[element].push(element);
                    }
                }
            }

            // wait for QCA/MTK sample
            wlObj[wlIfIndex].ch320MHz = {};
        }

        wlObj[wlIfIndex].pscChannel = (() => {
            let pscChannel = [];
            if (wlIfIndex !== "6g1" && wlIfIndex !== "6g2") {
                return pscChannel;
            }

            let channel = wlObj[wlIfIndex].channel;
            let pscChannelAll = ["5", "21", "37", "53", "69", "85", "101", "117", "133", "149", "165", "181", "197", "213"];
            pscChannelAll.forEach((element) => {
                if (channel.indexOf(element) !== -1) {
                    pscChannel.push(element);
                }
            });

            let pscRestrictedByModel = (() => {
                return (
                    productId === "GT-AXE11000" ||
                    productId === "RT-AXE95Q" ||
                    productId === "ET12" ||
                    productId === "ET8_V2" ||
                    productId === "ET8PRO"
                );
            })();
            let pscRestrictedBySku = (() => {
                return isEUSku || isAASku || isAUSku;
            })();

            if (pscRestrictedBySku) {
                pscChannel = pscChannel.filter((element) => {
                    let ch = parseInt(element);
                    return ch <= 85;
                });
            }

            if (pscRestrictedByModel) {
                pscChannel = pscChannel.filter((element) => {
                    let ch = parseInt(element);
                    return ch >= 37 && ch <= 213;
                });
            }

            return pscChannel;
        })();

        wlObj[wlIfIndex].bw20MHzSupport = (() => {
            let chObj = wlObj[wlIfIndex].ch20MHz;
            return Object.keys(chObj).length !== 0;
        })();
        wlObj[wlIfIndex].bw40MHzSupport = (() => {
            let chObj = wlObj[wlIfIndex].ch40MHz;
            return Object.keys(chObj).length !== 0;
        })();
        wlObj[wlIfIndex].bw80MHzSupport = (() => {
            let chObj = wlObj[wlIfIndex].ch80MHz;
            return Object.keys(chObj).length !== 0;
        })();
        wlObj[wlIfIndex].bw80_80MHzSupport = isSupport("vht80_80");
        wlObj[wlIfIndex].bw160MHzSupport = (() => {
            let chObj = wlObj[wlIfIndex].ch160MHz;
            return Object.keys(chObj).length !== 0;
        })();
        wlObj[wlIfIndex].bw320MHzSupport = (() => {
            let chObj = wlObj[wlIfIndex].ch320MHz;
            return Object.keys(chObj).length !== 0;
        })();

        wlObj[wlIfIndex].acSupport = (() => {
            if (productId === "RT-AX92U" && wlIfIndex === "2g1") {
                return false;
            }

            return isSupport("11AC");
        })();
        wlObj[wlIfIndex].axSupport = (() => {
            if (wlObj[wlIfIndex].capability.length !== 0) {
                return wlObj[wlIfIndex].capability.find((element) => element === "11ax") === "11ax";
            }

            return isSupport("11AX");
        })();
        wlObj[wlIfIndex].beSupport = (() => {
            if (wlObj[wlIfIndex].capability.length !== 0) {
                return wlObj[wlIfIndex].capability.find((element) => element === "11be") === "11be";
            }

            return isSupport("wifi7");
        })();
        wlObj[wlIfIndex].acsCH13Support = (() => wlIfIndex === "2g1" && wlObj[wlIfIndex].channel.length > 11)();
        wlObj[wlIfIndex].dfsSupport = (() => {
            if (wlIfIndex === "5g1" || wlIfIndex === "5g2") {
                return wlObj[wlIfIndex].channel.find((element) => element === "56" || element === "100") !== undefined;
            }

            return false;
        })();

        wlObj[wlIfIndex].uNII4Support = (() => {
            if (wlIfIndex === "5g1" || wlIfIndex === "5g2") {
                return wlObj[wlIfIndex].channel.find((element) => element === "173" || element === "177") !== undefined;
            }

            return false;
        })();

        wlObj[wlIfIndex].wifi7ModeEnabled = (() => {
            return _nvram[`${wlIfIndex}_11be`] === "1";
        })();

        wlObj[wlIfIndex].wlMode = (() => {
            /**
             * 2.4 GHZ:{Auto, N only, Legacy}
             * 5 GHz(AC):｛Auto, N only, N/AC mixed, Legacy｝
             * 5 GHz(AX): {Auto, N only, n/AC/AX mixed, Legacy}
             * 6 GHZ: {Auto, AX only}
             **/
            const wlModeObject = {
                0: "<#Auto#>",
                1: "N only",
                2: "Legacy",
                8: "N/AC/AX mixed",
                9: "AX only",
            };

            let _array = [];
            let { acSupport, axSupport } = wlObj[wlIfIndex];

            if (wlIfIndex === "2g1") {
                delete wlModeObject["8"];
                delete wlModeObject["9"];
            } else if (wlIfIndex === "5g1" || wlIfIndex === "5g2") {
                if (axSupport) {
                    // AX models
                    delete wlModeObject["1"];
                } else if (acSupport && !axSupport) {
                    // AC models
                    delete wlModeObject["1"];
                    delete wlModeObject["9"];
                    wlModeObject["8"] = "N/AC mixed";
                } else if (!axSupport && !acSupport) {
                    // N models
                    delete wlModeObject["8"];
                    delete wlModeObject["9"];
                }
            } else if (wlIfIndex === "6g1" || wlIfIndex === "6g2") {
                delete wlModeObject["1"];
                delete wlModeObject["2"];
                delete wlModeObject["8"];
            }

            for (let [key, index] of Object.entries(wlModeObject)) {
                _array.push([index, key]);
            }

            return _array;
        })();

        wlObj[wlIfIndex].authMethod = (() => {
            const authMethodObj = {
                open: "Open System",
                openowe: "Enhanced OPEN Transition",
                owe: "Enhanced Open",
                shared: "Shared Key",
                psk: "WPA-Personal",
                psk2: "WPA2-Personal",
                sae: "WPA3-Personal",
                pskpsk2: "WPA/WPA2-Personal",
                psk2sae: "WPA2/WPA3-Personal",
                wpa: "WPA-Enterprise",
                wpa2: "WPA2-Enterprise",
                wpa3: "WPA3-Enterprise",
                "suite-b": "WPA3-Enterprise 192-bit",
                wpawpa2: "WPA/WPA2-Enterprise",
                wpa2wpa3: "WPA2/WPA3-Enterprise",
                radius: "Radius with 802.1x",
            };

            let { beSupport } = wlObj[wlIfIndex];

            if (wlIfIndex === "6g1" || wlIfIndex === "6g2") {
                delete authMethodObj["open"];
                delete authMethodObj["openowe"];
                delete authMethodObj["shared"];
                delete authMethodObj["psk"];
                delete authMethodObj["psk2"];
                delete authMethodObj["pskpsk2"];
                delete authMethodObj["psk2sae"];
                delete authMethodObj["wpa"];
                delete authMethodObj["wpa2"];
                delete authMethodObj["wpawpa2"];
                delete authMethodObj["wpa2wpa3"];
                delete authMethodObj["radius"];
            }

            if (!beSupport) {
                delete authMethodObj["owe"];
            }

            if (!wpa3EnterpriseSupport) {
                delete authMethodObj["wpa3"];
                delete authMethodObj["suite-b"];
                delete authMethodObj["wpa2wpa3"];
            }

            if (!oweTransSupport) {
                delete authMethodObj["openowe"];
            }

            if (newWiFiCertSupport) {
                delete authMethodObj["psk"];
                delete authMethodObj["wpa"];
            }

            if (wifiLogoSupport) {
                delete authMethodObj["shared"];
                delete authMethodObj["psk"];
                delete authMethodObj["radius"];
            }

            return authMethodObj;
        })();

        wlObj[wlIfIndex].xboxOptSupport = isSupport("optimize_xbox");
        wlObj[wlIfIndex].bgProtectSupport = wlIfIndex === "2g1";
        wlObj[wlIfIndex].disable11bSupport = wlIfIndex === "2g1";
        wlObj[wlIfIndex].heFrameSupport = (() => {
            if (wlObj[wlIfIndex].capability.length !== 0) {
                return wlObj[wlIfIndex].capability.find((element) => element === "11ax") === "11ax";
            }

            return isSupport("11AX");
        })();
        wlObj[wlIfIndex].mboSupport = (() => {
            if (wlObj[wlIfIndex].capability.length !== 0) {
                return wlObj[wlIfIndex].capability.find((element) => element === "11ax") === "11ax";
            }

            return isSupport("11AX");
        })();
        wlObj[wlIfIndex].twtSupport = (() => {
            if (wlObj[wlIfIndex].capability.length !== 0) {
                return wlObj[wlIfIndex].capability.find((element) => element === "11ax") === "11ax";
            }

            return isSupport("11AX");
        })();

        wlObj[wlIfIndex].joinSmartConnect = (() => {
            const referenceArray = ["", "", "", "6g2", "6g1", "5g2", "5g1", "2g1"];
            let { v2Band, version, v1Type } = smartConnect;
            if (version === "v2") {
                let index = referenceArray.findIndex((element) => element === wlIfIndex);
                return v2Band[index] === "1";
            } else {
                if (v1Type === "1") {
                    return true;
                } else if (v1Type === "2") {
                    if (wlIfIndex === "5g1" || wlIfIndex === "5g2") {
                        return true;
                    }
                } else if (v1Type === "3") {
                    if (wlIfIndex === "2g1" || wlIfIndex === "5g1") {
                        return true;
                    }
                }

                return false;
            }
        })();
    }

    // sort the sequence to 2.4 GHz/5 GHz/6 GHz
    return Object.keys(wlObj)
        .sort()
        .reduce((objNew, key) => {
            objNew[key] = wlObj[key];
            return objNew;
        }, {});
})();

system.channelBandwidthObject = (() => {
    let { isBRCMplatform } = system;
    const bandwidthObject = {
        auto: {
            value: "0",
            string: "20/40/80/160/320",
        },
        "20mhz": { value: "1", string: "20 MHz" },
        "40mhz": { value: "2", string: "40 MHz" },
        "80mhz": { value: "3", string: "80 MHz" },
        "80_80mhz": { value: "4", string: "20/40/80/160/320" },
        "160mhz": { value: "5", string: "160 MHz" },
        "320mhz": { value: "6", string: "320 MHz" },
    };

    if (!isBRCMplatform) {
        bandwidthObject["auto"].value = "1";
        bandwidthObject["20mhz"].value = "0";
    }

    return bandwidthObject;
})();

system.extensionChannelObject = {
    0: "<#Auto#>",
    l: "<#WLANConfig11b_EChannelAbove#>",
    u: "<#WLANConfig11b_EChannelBelow#>",
    "320-1": "<#WLANConfig11b_EChannelAbove#>",
    "320-2": "<#WLANConfig11b_EChannelBelow#>",
    lower: "<#WLANConfig11b_EChannelAbove#>",
    upper: "<#WLANConfig11b_EChannelBelow#>",
};

system.wpaEncryptObject = {
    tkip: "TKIP",
    aes: "AES",
    "aes+gcmp256": "AES+GCMP256",
    "tkip+aes": "TKIP+AES",
    "suite-b": "Suite B",
};

system.mfpObject = {
    0: "<#WLANConfig11b_WirelessCtrl_buttonname#>",
    1: "<#WLANConfig11b_x_mfp_opt1#>",
    2: "<#WLANConfig11b_x_mfp_opt2#>",
};

system.wepEncryptObject = { 0: "<#wl_securitylevel_0#>", 1: "WEP-64bits", 2: "WEP-128bits" };
system.currentOPMode = (() => {
    const opModeObject = {
        rt: { id: "RT", desc: "<#wireless_router#>" },
        ap: { id: "AP", desc: "<#OP_AP_item#>" },
        re: { id: "RE", desc: "<#OP_RE_item#>" },
        mb: { id: "MB", desc: "<#OP_MB_item#>" },
        ew2: { id: "EW2", desc: "<#OP_RE2G_item#>" },
        ew5: { id: "EW5", desc: "<#OP_RE5G_item#>" },
        hs: { id: "HS", desc: "Hotspot" },
    };

    let { sw_mode, wlc_psta, wlc_express } = nvram;
    let _index = "";
    if (sw_mode === "1") {
        _index = "rt";
    } else if (sw_mode === "3" && wlc_psta === "0") {
        _index = "ap";
    } else if ((sw_mode === "2" && wlc_psta === "0") || (sw_mode === "3" && wlc_psta === "2")) {
        _index = "re";
    } else if (
        (sw_mode === "3" && wlc_psta === "1" && wlc_express === "0") ||
        (sw_mode === "3" && wlc_psta === "3" && wlc_express === "0") ||
        (sw_mode === "2" && wlc_psta === "1" && wlc_express === "0")
    ) {
        /*	Media Bridge
            Broadcom: sw_mode = 3 & wlc_psta = 1, sw_mode = 3 & wlc_psta = 3
            MTK/QCA: sw_mode = 2 & wlc_psta = 1
        */
        _index = "mb";
    } else if (sw_mode === "2" && wlc_psta === "0" && wlc_express === "1") {
        _index = "ew2";
    } else if (sw_mode === "2" && wlc_psta === "0" && wlc_express === "2") {
        _index = "ew5";
    } else if (sw_mode === "5") {
        _index = "hs";
    }

    return opModeObject[_index];
})();

system.firmwareVer = (() => {
    let { firmver, buildno, extendno, swpjverno } = nvram;
    let fwString = "";
    extendno = extendno === "" ? "0" : extendno;
    fwString = swpjverno !== "" ? `${swpjverno}_${extendno}` : `${firmver}.${buildno}_${extendno}`;
    return { full: fwString, number: fwString.split("-g")[0] };
})();

system.labelMac = httpApi.hookGet("get_label_mac");
system.client = (() => {
    let nmp = httpApi.hookGet("get_clientlist");
    let database = httpApi.hookGet("get_clientlist_from_json_database");
    let _client = { total: 0, activeCount: 0, detail: {}, wireless: {}, wiredCount: 0, wirelessCount: 0 };
    let mapArray = [];
    let mapArrayForIsWL = ["2g1", "5g1", "5g2", "6g1", "6g2"];

    for (let key of Object.keys(system.wlBandSeq)) {
        _client.wireless[key] = { count: 0 };
        mapArray.push(key);
    }

    for (let [key, value] of Object.entries(database)) {
        if (key === "maclist" || key === "ClientAPILevel") {
            continue;
        }

        _client.detail[key] = { ...value };
        if (nmp[key] !== undefined) {
            if (nmp[key].isOnline !== "0" && !nmp[key].amesh_isRe) {
                _client.activeCount++;
                if (nmp[key].isWL !== "0") {
                    let _index = mapArrayForIsWL[nmp[key].isWL - 1];
                    _client.wireless[_index].count++;
                    _client.wirelessCount++;
                } else {
                    _client.wiredCount++;
                }
            }

            Object.assign(_client.detail[key], nmp[key]);
        }

        _client.total++;
    }

    return _client;
})();

system.aimesh_node_list = (() => {
    let { aMeshSupport, aMeshRouterSupport } = system.aMesh;
    let _node_list = {};
    if (aMeshSupport && aMeshRouterSupport) {
        let cfg_clientlist = httpApi.hookGet("get_cfg_clientlist");
        _node_list = JSON.parse(JSON.stringify(cfg_clientlist));

        // put a new attribute
        for (let item of _node_list) {
            let { ui_model_name, model_name } = item;
            item["display_name"] = ui_model_name ? ui_model_name : model_name;
        }
    }

    return _node_list;
})();

system.language = (() => {
    let list = httpApi.hookGet("language_support_list");
    let { preferred_lang } = nvram;
    return { currentLang: preferred_lang, supportList: { ...list } };
})();

system.time = getTime();

// (function updateEthPortStatus() {
//     httpApi.get_port_status_array(system.labelMac, (response) => {
//         system.portInfo = { ...response };
//     });

//     setTimeout(updateEthPortStatus, 3000);
// })();

function wlObjConstructor() {
    this.ssid = "";
    this.capability = [];
    this.channel = [];
    this.chanspecs = [];
    this.countryCode = "";
    this.sdkVersion = "";
    this.chipsetNumer = "";
    this.capability = "";
    this.nSupport = true;
    this.acSupport = false;
    this.axSupport = false;
    this.adSupport = false;
    this.aySupport = false;
    this.noVHTSupport = true;
    this.bw80MHzSupport = false;
    this.bw160MHzSupport = false;
    this.dfsSupport = false;
    this.acsCH13Support = false;
    this.acsBand1Support = false;
    this.acsBand3Support = false;
    this.channel160MHz = [];
    this.channel80MHz = [];
    this.channel40MHz = [];
    this.channel20MHz = [];
    this.QAM256Support = false;
    this.QAM1024Support = false;
    this.xboxOpt = false;
    this.heFrameSupport = false;
    this.mboSupport = false;
    this.twtSupport = false;
    this.bw160Enabled = false;
}

function getTime() {
    let bootTime = 0;
    let timeMillSec = 0;
    let timezone = "";
    let weekday = ["Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"];
    if (Object.keys(system.time).length === 0 || system.time.timestamp % 60000 === 0) {
        // sync system time per 60 seconds
        let uptime = httpApi.hookGet("uptime");
        cachedData.clear(["uptime"]);
        timeArray = uptime.split("(")[0].split(" ");
        // remove timezone and regenerate time string to get timemillisecond to avoid timezone offset issue
        timezone = timeArray.pop();
        timeArray.join("");
        bootTime = parseInt(uptime.split("(")[1].split(" ")[0]);
        timeMillSec = Date.parse(timeArray);
    } else if (system !== undefined && system.time && system.time.timestamp % 60000 !== 0) {
        timeMillSec = system.time.timestamp + 1000;
    }

    let sysTime = new Date(timeMillSec);
    let timeString = `${sysTime.getHours() < 10 ? "0" + sysTime.getHours() : sysTime.getHours()}:${
        sysTime.getMinutes() < 10 ? "0" + sysTime.getMinutes() : sysTime.getMinutes()
    }:${sysTime.getSeconds() < 10 ? "0" + sysTime.getSeconds() : sysTime.getSeconds()}`;

    return {
        current: timeString,
        uptime: bootTime,
        weekday: weekday[sysTime.getDay()],
        timestamp: timeMillSec,
        timezone,
    };
}

function objectDeepCopy(objectData) {
    if (typeof objectData === "object" && objectData !== null) {
        return JSON.parse(JSON.stringify(objectData));
    }

    return "";
}

var tooltipTriggerList = [].slice.call(document.querySelectorAll('[data-bs-toggle="tooltip"]'));
var tooltipList = tooltipTriggerList.map(function (tooltipTriggerEl) {
    return new bootstrap.Tooltip(tooltipTriggerEl);
});
var popoverTriggerList = [].slice.call(document.querySelectorAll('[data-bs-toggle="popover"]'));
var popoverList = popoverTriggerList.map(function (popoverTriggerEl) {
    return new bootstrap.Popover(popoverTriggerEl);
});

function webInterface(message) {
    var returnData = {};
    if (message != undefined && message != "") {
        var messageFromApp = JSON.parse(message);

        if (messageFromApp.dataHandler != undefined) {
            if (httpApi != undefined) {
                httpApi.app_dataHandler = messageFromApp.dataHandler.toString() == "1" ? true : false;
                returnData.dataHandler = "1";
            }
        }

        if (messageFromApp.hideHeader != undefined) {
            if (messageFromApp.hideHeader.toString() == "1") {
                $(".notInApp").attr("style", "display:none !important;");
            }
        }
    }

    return returnData;
}

var postMessageToApp = function () {};
try {
    if (window.webkit.messageHandlers.appInterface) {
        postMessageToApp = function (jsonObj) {
            window.webkit.messageHandlers.appInterface.postMessage(JSON.stringify(jsonObj));
        };
    }
} catch (e) {
    if (window.appInterface) {
        postMessageToApp = function (jsonObj) {
            window.appInterface.postMessage(JSON.stringify(jsonObj));
        };
    }
}
