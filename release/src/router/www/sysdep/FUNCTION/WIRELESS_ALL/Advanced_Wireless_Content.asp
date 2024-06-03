<!DOCTYPE html>
<html>
    <head>
        <meta charset="UTF-8" />
        <meta http-equiv="X-UA-Compatible" content="IE=edge" />
        <meta name="viewport" content="width=device-width, initial-scale=1.0,  user-scalable=no" />
        <link rel="shortcut icon" href="images/favicon.png" />
        <title><#Web_Title#> - <#menu5_1_1#></title>
        <link rel="stylesheet" href="index_style.css" />
        <link rel="stylesheet" href="form_style.css" />
        <link rel="stylesheet" href="usp_style.css" />
        <link rel="stylesheet" href="other.css" />
        <link rel="stylesheet" href="css/confirm_block.css" />
        <script src="/js/confirm_block.js"></script>
        <script src="/js/jquery.js"></script>
        <script src="/js/httpApi.js"></script>
        <script src="/state.js"></script>
        <script src="/help.js"></script>
        <script src="/general.js"></script>
        <script src="/popup.js"></script>
        <script src="/md5.js"></script>
        <script src="/validator.js"></script>

        <script src="/js/asus.js"></script>
        <script src="/switcherplugin/jquery.iphone-switch.js"></script>
        <style>
            .setup_help_icon {
                width: 40px;
                height: 40px;
                border-radius: 24px;
                background-color: rgba(164, 183, 195, 0.2);
                background-image: url(images/New_ui/vpn_icon_all_collect.svg);
                background-repeat: no-repeat;
                background-position: -324px 0px;
                width: 24px;
                height: 24px;
                margin-top: -24px;
                margin-left: 218px;
                cursor: pointer;
            }
            .a-hint-text {
                color: #fc0 !important;
                text-decoration: underline !important;
                cursor: pointer;
            }
            .splitline-padding {
                margin: 10px 0 10px 5px;
            }
            .smart-connect-rule-link {
                display: table-cell;
                vertical-align: middle;
            }
            .radio-smartcon-enable {
                width: 94px;
                display: table-cell;
            }
        </style>
        <script>
            let { dwb_mode, dwb_band } = httpApi.nvramGet(["dwb_mode", "dwb_band"]);
            let nbandListArray = nvram["wlnband_list"].split("&#60");
            let systemManipulable = objectDeepCopy(system);
            document.addEventListener("DOMContentLoaded", function () {
                let { isBRCMplatform } = systemManipulable;
                show_menu();
                generateMainField();
                eventBind();
                if (isBRCMplatform) {
                    document.getElementById("smartcon_rule_link").style.display = "";
                }
            });
            function eventBind() {
                document.querySelectorAll(".setup_help_icon").forEach((element) => {
                    let confirm_content = "<b>WPA3-Personal</b><br>";
                    confirm_content += "<#WLANConfig11b_AuthenticationMethod_wpa3#><br><br>";
                    confirm_content += "<b>WPA2/WPA3-Personal</b><br>";
                    confirm_content += "<#WLANConfig11b_AuthenticationMethod_wpa32#><br><br>";
                    confirm_content += "<b>WPA2-Personal</b><br>";
                    confirm_content += "<#WLANConfig11b_AuthenticationMethod_wpa2#><br><br>";
                    confirm_content += "<b>WPA-Auto-Personal</b><br>";
                    confirm_content += "<#WLANConfig11b_AuthenticationMethod_wpa21#>";
                    element.addEventListener("click", function () {
                        let confirm_flag = 1;
                        if (confirm_flag == 1) {
                            if ($(".confirm_block").length > 0) {
                                $(".confirm_block").remove();
                            }
                            if (window.scrollTo) window.scrollTo(0, 0);
                            htmlbodyforIE = document.getElementsByTagName("html");
                            htmlbodyforIE[0].style.overflow = "hidden";

                            $("#Loading").css("visibility", "visible");
                            $("#loadingBlock").css("visibility", "hidden");

                            confirm_asus({
                                title: "<#WLANConfig11b_AuthenticationMethod_itemname#>",
                                contentA: confirm_content,
                                contentC: "",
                                left_button: "Hidden",
                                left_button_callback: function () {},
                                left_button_args: {},
                                right_button: "<#CTL_ok#>",
                                right_button_callback: function () {
                                    confirm_cancel();
                                    htmlbodyforIE = document.getElementsByTagName("html");
                                    htmlbodyforIE[0].style.overflow = "";
                                    $("#Loading").css("visibility", "hidden");
                                    return false;
                                },
                                right_button_args: {},
                                iframe: "",
                                margin: "0px",
                                note_display_flag: 0,
                            });

                            $(".confirm_block").css("zIndex", 10001);
                        }
                    });
                });
            }
            function generateMainField() {
                let code = "";
                let { wlBandSeq, smartConnect } = systemManipulable;
                let { smartConnectEnable, smartConnectReferenceIndex } = smartConnect;

                // GENERATE SMART CONNECT
                code += `
                    <thead>
                        <tr>
                            <td colspan="2">Smart Connect</td>
                        </tr>
                    </thead>
                `;

                // Smart Connect Radio Band
                code += generateSmartConnectRadio();

                // SSID, Hide SSID
                code += generateSSID("smart_connect");

                // Authentication Method
                code += generateAuthenticationMethod("smart_connect");

                // WPA Encryption
                code += generateWpaEncryption("smart_connect");

                // WPA Key
                code += generateWpaKey("smart_connect");

                // MFP
                code += generateMfp("smart_connect");

                //Group Key
                code += generateGroupKey("smart_connect");

                // WEP Encryption
                code += generateWepEncryption("smart_connect");

                // Key Index, WEP Key 1~4, ASUS passphrase
                code += generateWepKey("smart_connect");

                // RADIUS Server
                code += generateRadiusSettings("smart_connect");
                document.getElementById("band_separate").innerHTML = code;
                document.getElementById("band_separate").style.display = smartConnectEnable ? "" : "none";

                /* EACH BAND*/
                code = "";
                for (let { prefixNvram, name, joinSmartConnect } of Object.values(wlBandSeq)) {
                    code += `
                        <thead>
                            <tr>
                                <td colspan="2">${name}</td>
                            </tr>
                        </thead>
                    `;

                    // SSID, Hide SSID
                    code += generateSSID(prefixNvram);

                    // AiMesh Wireless Backhaul
                    code += generateWirelessBackhaul(prefixNvram);

                    // CHANNEL BANDWIDTH
                    code += generateChannelBandwidth(prefixNvram);

                    // CONTROL CHANNEL
                    code += generateControlChannel(prefixNvram);

                    // Extension Channel
                    code += generateExtensionChannel(prefixNvram);

                    // Authentication Method
                    code += generateAuthenticationMethod(prefixNvram);

                    // WPA Encryption
                    code += generateWpaEncryption(prefixNvram);

                    // WPA Key
                    code += generateWpaKey(prefixNvram);

                    // MFP
                    code += generateMfp(prefixNvram);

                    //Group Key
                    code += generateGroupKey(prefixNvram);

                    // WEP Encryption
                    code += generateWepEncryption(prefixNvram);

                    // Key Index, WEP Key 1~4, ASUS passphrase
                    code += generateWepKey(prefixNvram);

                    // RADIUS Server
                    code += generateRadiusSettings(prefixNvram);
                }

                document.getElementById("eachBandField").innerHTML = code;
            }
            function wirelessBackhaulHandler(prefix) {
                if (prefix !== nbandListArray[dwb_band]) {
                    return "";
                }

                let { wlBandSeq, smartConnect, aMesh } = systemManipulable;
                let { smartConnectReferenceIndex, smartConnectEnable } = smartConnect;
                let prefixNvram = prefix === "smart_connect" ? smartConnectReferenceIndex : prefix;
                let { joinSmartConnect } = wlBandSeq[prefixNvram];
                let cfgClientList = httpApi.hookGet("get_cfg_clientlist");
                let { fronthaul_ap_option_off, fronthaul_ap_option_on } = httpApi.aimesh_get_node_capability(cfgClientList[0]);
                let displayFlag = (() => {
                    let { dwbMode } = aMesh;
                    let smartConnectIncludeDwbBand =
                        document.getElementById(`smart_connect_check_${prefix}`) &&
                        document.getElementById(`smart_connect_check_${prefix}`).checked &&
                        smartConnectEnable;
                    return dwbMode === "1" && smartConnectIncludeDwbBand && fronthaul_ap_option_on && fronthaul_ap_option_off ? "" : "none";
                })();

                document.getElementById("mesh_backhaul_field").style.display = displayFlag;
            }
            function ssidHandler(prefix) {
                let elementArray = ["_ssid_field", "_hide_ssid_field", "_auth_method_field"];
                let { wlBandSeq, smartConnect, aMesh } = systemManipulable;
                let { smartConnectEnable, smartConnectReferenceIndex } = smartConnect;
                let prefixNvram = prefix === "smart_connect" ? smartConnectReferenceIndex : prefix;
                let { authMethodValue, joinSmartConnect } = wlBandSeq[prefixNvram];
                elementArray.forEach((element) => {
                    let target = document.getElementById(`${prefix}${element}`);
                    if (target) {
                        let displayFlag = (() => {
                            let { dwbBand, dwbMode } = aMesh;
                            if (dwbMode === "1" && dwbBand === prefix) {
                                return "";
                            }

                            return smartConnectEnable && joinSmartConnect && prefix !== "smart_connect" ? "none" : "";
                        })();

                        target.style.display = displayFlag;
                    }
                });

                authenticationMethodChange(authMethodValue, prefix);
            }
            function smartConnectRadioChange(check, prefix) {
                systemManipulable.wlBandSeq[prefix].joinSmartConnect = check;
                let { wlBandSeq, mloEnabled } = systemManipulable;
                let { joinSmartConnect } = wlBandSeq[prefix];
                if (mloEnabled) {
                    mloHint();
                }

                let count = 0;
                for (let { joinSmartConnect } of Object.values(wlBandSeq)) {
                    if (joinSmartConnect) {
                        count++;
                    }
                }

                if (count < 2) {
                    alert("<#smart_connect_alert#>");
                    systemManipulable.wlBandSeq[prefix].joinSmartConnect = !check;
                    document.getElementById(`smart_connect_check_${prefix}`).checked = !check;
                    return false;
                }

                ssidHandler(prefix);
                wirelessBackhaulHandler(prefix);
            }

            function smartConnectChange(v1Type) {
                let { wlBandSeq } = systemManipulable;
                if (v1Type === "1") {
                    for (let prefix of Object.keys(wlBandSeq)) {
                        let check = true;
                        systemManipulable.wlBandSeq[prefix].joinSmartConnect = check;
                        ssidHandler(prefix);
                        wirelessBackhaulHandler(prefix);
                    }
                } else if (v1Type === "2") {
                    for (let prefix of Object.keys(wlBandSeq)) {
                        let check = false;
                        if (prefix === "5g1" || prefix === "5g2") {
                            check = true;
                        }

                        systemManipulable.wlBandSeq[prefix].joinSmartConnect = check;
                        ssidHandler(prefix);
                        wirelessBackhaulHandler(prefix);
                    }
                } else if (v1Type === "3") {
                    for (let prefix of Object.keys(wlBandSeq)) {
                        let check = false;
                        if (prefix !== "6g1" && prefix !== "6g2") {
                            check = true;
                        }

                        systemManipulable.wlBandSeq[prefix].joinSmartConnect = check;
                        ssidHandler(prefix);
                        wirelessBackhaulHandler(prefix);
                    }
                }
            }

            function controlChannelChange(channel, prefix) {
                const autoChannelSettingsElement = ["_acs_dfs_field", "_unii4_field", "_acs_ch13_field"];
                let { wlBandSeq, smartConnect, isBRCMplatform } = systemManipulable;
                let { smartConnectReferenceIndex } = smartConnect;
                let prefixNvram = prefix === "smart_connect" ? smartConnectReferenceIndex : prefix;
                let { bandwidthValue, chanspecs, channelValue } = wlBandSeq[prefixNvram];
                if (prefix === "2g1") {
                    if (isBRCMplatform) {
                        if (bandwidthValue === "0" || bandwidthValue === "2") {
                            if (chanspecs.indexOf(`${channel}u`) !== -1) {
                                channelValue = `${channel}u`;
                            } else if (chanspecs.indexOf(`${channel}l`) !== -1) {
                                channelValue = `${channel}l`;
                            } else {
                                channelValue = channel;
                            }
                        }
                    } else {
                        if (bandwidthValue === "1" || bandwidthValue === "2") {
                            channelValue = channel;
                        }
                    }
                } else if (prefix === "6g1" || prefix === "6g2") {
                    if (bandwidthValue === "0" || bandwidthValue === "6") {
                        if (chanspecs.indexOf(`6g${channel}/320-1`) !== -1) {
                            channelValue = `6g${channel}/320-1`;
                        } else if (chanspecs.indexOf(`6g${channel}/320-2`) !== -1) {
                            channelValue = `6g${channel}/320-2`;
                        } else {
                            channelValue = channel;
                        }
                    }
                }

                if (systemManipulable.wlBandSeq[prefix]) {
                    systemManipulable.wlBandSeq[prefix].channelValue = channelValue;
                }

                autoChannelSettingsElement.forEach((element) => {
                    let target = document.getElementById(`${prefix}${element}`);
                    if (target) {
                        let displayFlag = channel === "0" ? "" : "none";
                        target.style.display = displayFlag;
                    }
                });

                extensionChannelHandler(prefix);
            }

            function pscChannelEnable(check, prefix) {
                const prefix6G = ["6g1", "6g2"];
                let { wlBandSeq } = systemManipulable;
                systemManipulable.psc6g = check ? "1" : "0";
                controlChannelHandler(prefix);

                // sync PSC of 6 GHz-1 and 6 GHz-2
                for (let element of prefix6G) {
                    if (element === prefix) {
                        continue;
                    }

                    if (wlBandSeq[element]) {
                        document.getElementById(`${element}_psc6g`).checked = check;
                        controlChannelHandler(element);
                    }
                }
            }

            function bandwidth160MHzEnable(check, prefix) {
                let bw160Value = check ? "1" : "0";
                let { wlBandSeq, smartConnect, channelBandwidthObject } = systemManipulable;
                let { smartConnectReferenceIndex } = smartConnect;
                let prefixNvram = prefix === "smart_connect" ? smartConnectReferenceIndex : prefix;
                let { bw80MHzSupport, bw160MHzSupport, bw320MHzSupport, band80_80MhzSupport, bandwidthValue } = wlBandSeq[prefixNvram];
                let bandwidthStringObject = objectDeepCopy(channelBandwidthObject);
                let autoBandwidthString = bandwidthStringObject.auto.string;
                if (!bw320MHzSupport) {
                    autoBandwidthString = autoBandwidthString.split("/320")[0];
                    delete bandwidthStringObject["320mhz"];
                }

                if (!bw320MHzSupport && (!bw160MHzSupport || (bw160MHzSupport && bw160Value === "0"))) {
                    autoBandwidthString = autoBandwidthString.split("/160")[0];
                    delete bandwidthStringObject["160mhz"];
                }

                if (!band80_80MhzSupport) {
                    delete bandwidthStringObject["80_80mhz"];
                }

                if (!bw80MHzSupport) {
                    autoBandwidthString = autoBandwidthString.split("/80")[0];
                    delete bandwidthStringObject["80mhz"];
                }

                bandwidthStringObject.auto.string = `${autoBandwidthString} MHz`;
                let channelBandwidthSnippet = "";
                for (let { value, string } of Object.values(bandwidthStringObject)) {
                    channelBandwidthSnippet += `<option value="${value}" ${bandwidthValue === value ? "selected" : ""}>${string}</option>`;
                }

                document.getElementById(`${prefix}_channel_bandwidth`).innerHTML = channelBandwidthSnippet;
                if (systemManipulable.wlBandSeq[prefix]) {
                    systemManipulable.wlBandSeq[prefix].bw160Value = bw160Value;
                    if (bw160Value === "0" && bandwidthValue === "5") {
                        bandwidthValue = "0";
                        systemManipulable.wlBandSeq[prefix].bandwidthValue = bandwidthValue;
                    }
                }

                channelBandwidthChange(bandwidthValue, prefix);
            }

            function extensionChannelHandler(prefix) {
                let { isBRCMplatform, wlBandSeq, smartConnect, extensionChannelObject } = systemManipulable;
                let { smartConnectReferenceIndex } = smartConnect;
                let prefixNvram = prefix === "smart_connect" ? smartConnectReferenceIndex : prefix;
                let { bandwidthValue, channelValue, chanspecs, ch320MHz, channel, extChannelValue } = wlBandSeq[prefixNvram];
                let extensionChannelString = objectDeepCopy(extensionChannelObject);
                let postfixChannelValue = "0";
                if (isBRCMplatform) {
                    delete extensionChannelString["lower"];
                    delete extensionChannelString["upper"];
                } else {
                    delete extensionChannelString["l"];
                    delete extensionChannelString["u"];
                }

                if (channelValue === "0" || prefix === "5g1" || prefix === "5g2") {
                    // Control Channel is Auto or radio is 5 GHz, the Extension Channel is only Auto
                    delete extensionChannelString["l"];
                    delete extensionChannelString["u"];
                    delete extensionChannelString["320-1"];
                    delete extensionChannelString["320-2"];
                    delete extensionChannelString["lower"];
                    delete extensionChannelString["upper"];
                } else {
                    if (prefix === "2g1") {
                        delete extensionChannelString["0"];
                        delete extensionChannelString["320-1"];
                        delete extensionChannelString["320-2"];
                        let chNumber = channelValue.split("u")[0].split("l")[0];
                        if (isBRCMplatform) {
                            if (channelValue.indexOf("l") !== -1 || channelValue.indexOf("u") !== -1) {
                                postfixChannelValue = channelValue.slice(-1);
                            }

                            if (chanspecs.indexOf(`${chNumber}u`) === -1) {
                                delete extensionChannelString["u"];
                            }

                            if (chanspecs.indexOf(`${chNumber}l`) === -1) {
                                delete extensionChannelString["l"];
                            }
                        } else {
                            if (parseInt(chNumber) - 4 <= 0) {
                                delete extensionChannelString["upper"];
                            }

                            if (parseInt(chNumber) + 4 > channel.length) {
                                delete extensionChannelString["lower"];
                            }

                            postfixChannelValue = extChannelValue;
                        }
                    } else if (prefix === "6g1" || prefix === "6g2") {
                        delete extensionChannelString["l"];
                        delete extensionChannelString["u"];
                        if (bandwidthValue !== "0" && bandwidthValue !== "6") {
                            // for 20 MHz, 40 MHz, 80 MHz, 160 MHz
                            delete extensionChannelString["320-1"];
                            delete extensionChannelString["320-2"];
                        } else {
                            let chNumber = channelValue.split("/320")[0].slice(2);
                            postfixChannelValue = channelValue.split("/")[1];
                            if (ch320MHz[chNumber]) {
                                delete extensionChannelString["0"];
                                if (ch320MHz[chNumber].indexOf(`6g${chNumber}/320-1`) === -1) {
                                    delete extensionChannelString["320-1"];
                                }

                                if (ch320MHz[chNumber].indexOf(`6g${chNumber}/320-2`) === -1) {
                                    delete extensionChannelString["320-2"];
                                }
                            } else {
                                // for bandwidth is Auto and does not support 320 MHz
                                delete extensionChannelString["320-1"];
                                delete extensionChannelString["320-2"];
                            }
                        }
                    }
                }

                let extensionChannelSnippet = "";
                for (let [value, desc] of Object.entries(extensionChannelString)) {
                    extensionChannelSnippet += `<option value="${value}" ${
                        postfixChannelValue === value ? "selected" : ""
                    }>${desc}</option>`;
                }

                document.getElementById(`${prefix}_extension_channel`).innerHTML = extensionChannelSnippet;
                let displayFlag = (() => {
                    if (prefix === "2g1" && channelValue === "14" && postfixChannelValue === "0") {
                        return "none";
                    }

                    if (isBRCMplatform) {
                        if (bandwidthValue === "1") {
                            return "none";
                        }
                    } else {
                        if (bandwidthValue === "0") {
                            return "none";
                        }
                    }

                    return "";
                })();

                document.getElementById(`${prefix}_extension_channel_field`).style.display = displayFlag;
            }

            function controlChannelHandler(prefix) {
                const autoChannelSettingsElement = ["_acs_dfs_field", "_unii4_field", "_acs_ch13_field"];
                let { wlBandSeq, smartConnect, psc6g, isBRCMplatform } = systemManipulable;
                let { smartConnectReferenceIndex } = smartConnect;
                let prefixNvram = prefix === "smart_connect" ? smartConnectReferenceIndex : prefix;
                let {
                    bandwidthValue,
                    channel,
                    channelValue,
                    chanspecs,
                    ch320MHz,
                    ch160MHz,
                    ch80MHz,
                    ch40MHz,
                    ch20MHz,
                    dfsSupport,
                    uNII4Support,
                    pscChannel,
                    bw160Value,
                } = wlBandSeq[prefixNvram];
                if (prefix === "6g1" || prefix === "6g2") {
                    if (psc6g === "1") {
                        channel = [...pscChannel];
                    }
                }

                let channelSnippet = `<option value="0"><#Auto#></option>`;
                for (let element of channel) {
                    let chValue = "";
                    let selected = "";
                    let chNumber = 0;
                    if ((isBRCMplatform && bandwidthValue === "0") || (!isBRCMplatform && bandwidthValue === "1")) {
                        // AUTO BANDWIDTH
                        if (ch320MHz[element]) {
                            chValue = element;
                            chNumber = channelValue.split("/320")[0].slice(2);
                        } else if (ch160MHz[element] && ch160MHz[element].length !== 0 && bw160Value === "1") {
                            chValue = ch160MHz[element][0];
                            if (prefix === "6g1" || prefix === "6g2") {
                                chNumber = channelValue.split("/160")[0].slice(2);
                            } else {
                                chNumber = channelValue.split("/160")[0];
                            }
                        } else if (ch80MHz[element]) {
                            chValue = ch80MHz[element][0];
                            if (prefix === "6g1" || prefix === "6g2") {
                                chNumber = channelValue.split("/80")[0].slice(2);
                            } else {
                                chNumber = channelValue.split("/80")[0];
                            }
                        } else if (ch40MHz[element]) {
                            if (prefix === "2g1") {
                                chValue = element;
                                chNumber = channelValue.split("u")[0].split("l")[0];
                            } else {
                                chValue = ch40MHz[element][0];
                                if (prefix === "6g1" || prefix === "6g2") {
                                    chNumber = channelValue.split("/40")[0].slice(2);
                                } else {
                                    chNumber = channelValue.split("u")[0].split("l")[0];
                                }
                            }
                        } else {
                            chValue = ch20MHz[element][0];
                            chNumber = channelValue;
                        }
                    } else if (bandwidthValue === "6") {
                        // 320 MHz
                        if (!ch320MHz[element]) {
                            continue;
                        }

                        chValue = element;
                        chNumber = channelValue.split("/320")[0].slice(2);
                    } else if (bandwidthValue === "5") {
                        // 160 MHz
                        if (!ch160MHz[element]) {
                            continue;
                        }

                        chValue = ch160MHz[element][0];
                        if (prefix === "6g1" || prefix === "6g2") {
                            chNumber = channelValue.split("/160")[0].slice(2);
                        } else {
                            chNumber = channelValue.split("/160")[0];
                        }
                    } else if (bandwidthValue === "3") {
                        // 80 MHz
                        if (!ch80MHz[element]) {
                            continue;
                        }

                        chValue = ch80MHz[element][0];
                        if (prefix === "6g1" || prefix === "6g2") {
                            chNumber = channelValue.split("/80")[0].slice(2);
                        } else {
                            chNumber = channelValue.split("/80")[0];
                        }
                    } else if (bandwidthValue === "2") {
                        // 40 MHz
                        if (!ch40MHz[element]) {
                            continue;
                        }

                        if (prefix === "2g1") {
                            chValue = element;
                            chNumber = channelValue.split("u")[0].split("l")[0];
                        } else {
                            chValue = ch40MHz[element][0];
                            if (prefix === "6g1" || prefix === "6g2") {
                                chNumber = channelValue.split("/40")[0].slice(2);
                            } else {
                                chNumber = channelValue.split("u")[0].split("l")[0];
                            }
                        }
                    } else if ((isBRCMplatform && bandwidthValue === "1") || (!isBRCMplatform && bandwidthValue === "0")) {
                        // 20 MHz
                        chValue = ch20MHz[element][0];
                        chNumber = channelValue;
                    }

                    selected = element === chNumber ? "selected" : "";
                    channelSnippet += `<option value="${chValue}" ${selected}>${element}</option>`;
                }

                document.getElementById(`${prefix}_control_channel`).innerHTML = channelSnippet;
                let newChannelValue = document.getElementById(`${prefix}_control_channel`).value;

                // combine control channel and extension channel values to fit the legal channel format
                if (prefix === "2g1") {
                    if (bandwidthValue === "0" || bandwidthValue === "2") {
                        if (newChannelValue !== "0") {
                            if (chanspecs.indexOf(`${newChannelValue}u`) !== -1) {
                                newChannelValue = `${newChannelValue}u`;
                            } else if (chanspecs.indexOf(`${newChannelValue}l`) !== -1) {
                                newChannelValue = `${newChannelValue}l`;
                            }
                        }
                    }
                } else if (prefix === "6g1" || prefix === "6g2") {
                    if (bandwidthValue === "0" || bandwidthValue === "6") {
                        if (chanspecs.indexOf(`6g${newChannelValue}/320-1`) !== -1) {
                            newChannelValue = `6g${newChannelValue}/320-1`;
                        } else if (chanspecs.indexOf(`6g${newChannelValue}/320-2`) !== -1) {
                            newChannelValue = `6g${newChannelValue}/320-2`;
                        }
                    }
                }

                systemManipulable.wlBandSeq[prefix].channelValue = newChannelValue;
                autoChannelSettingsElement.forEach((element) => {
                    let target = document.getElementById(`${prefix}${element}`);
                    if (target) {
                        let displayFlag = newChannelValue === "0" ? "" : "none";
                        target.style.display = displayFlag;
                    }
                });

                extensionChannelHandler(prefix);
            }

            function channelBandwidthChange(bandwidthValue, prefix) {
                let { isBRCMplatform } = systemManipulable;
                if (systemManipulable && systemManipulable.wlBandSeq[prefix]) {
                    systemManipulable.wlBandSeq[prefix].bandwidthValue = bandwidthValue;
                }

                // let displayFlag =
                let displayFlag = (() => {
                    if (isBRCMplatform) {
                        return bandwidthValue === "1" ? "none" : "";
                    }

                    return bandwidthValue === "0" ? "none" : "";
                })();
                let extensionChannelElement = document.getElementById(`${prefix}_extension_channel_field`);
                if (extensionChannelElement) {
                    extensionChannelElement.style.display = displayFlag;
                }

                controlChannelHandler(prefix);
            }

            function wpaEncryptHandler(prefix) {
                let { wlBandSeq, smartConnect, wpaEncryptObject, aMesh } = systemManipulable;
                let { smartConnectEnable, smartConnectReferenceIndex } = smartConnect;
                let prefixNvram = prefix === "smart_connect" ? smartConnectReferenceIndex : prefix;
                let { wpaEncryptValue, joinSmartConnect, authMethodValue, wifi7ModeEnabled } = wlBandSeq[prefixNvram];
                let wpaEncryptStringObject = objectDeepCopy(wpaEncryptObject);
                let wpaEncryptSnippet = "";
                if (authMethodValue !== "psk" && authMethodValue !== "wpa") {
                    delete wpaEncryptStringObject["tkip"];
                }

                if (authMethodValue !== "pskpsk2" && authMethodValue !== "wpawpa2") {
                    delete wpaEncryptStringObject["tkip+aes"];
                }

                if (authMethodValue !== "suite-b") {
                    delete wpaEncryptStringObject["suite-b"];
                }

                if (authMethodValue === "suite-b") {
                    delete wpaEncryptStringObject["aes"];
                }

                if (!wifi7ModeEnabled || authMethodValue !== "sae") {
                    delete wpaEncryptStringObject["aes+gcmp256"];
                } else {
                    delete wpaEncryptStringObject["aes"];
                }

                if (
                    authMethodValue === "openowe" ||
                    authMethodValue === "owe" ||
                    authMethodValue === "psk" ||
                    authMethodValue === "psk2" ||
                    authMethodValue === "sae" ||
                    authMethodValue === "pskpsk2" ||
                    authMethodValue === "psk2sae" ||
                    authMethodValue === "wpa" ||
                    authMethodValue === "wpa2" ||
                    authMethodValue === "wpa3" ||
                    authMethodValue === "suite-b" ||
                    authMethodValue === "wpawpa2" ||
                    authMethodValue === "wpa2wpa3"
                ) {
                    for (let [value, desc] of Object.entries(wpaEncryptStringObject)) {
                        wpaEncryptSnippet += `<option value="${value}" ${wpaEncryptValue === value ? "selected" : ""}>${desc}</option>`;
                    }

                    let target = document.getElementById(`${prefix}_wpa_encrypt`);
                    if (target) {
                        target.innerHTML = wpaEncryptSnippet;
                    }

                    target = document.getElementById(`${prefix}_wpa_encrypt_field`);
                    if (target) {
                        let displayFlag = (() => {
                            let { dwbMode, dwbBand } = aMesh;
                            if (dwbMode === "1" && dwbBand === prefix) {
                                return "";
                            }

                            if (prefix === "smart_connect") {
                                return smartConnectEnable ? "" : "none";
                            }

                            return smartConnectEnable && joinSmartConnect ? "none" : "";
                        })();
                        target.style.display = displayFlag;
                    }
                }
            }
            function wpaKeyHandler(prefix) {
                let { wlBandSeq, smartConnect, aMesh } = systemManipulable;
                let { smartConnectEnable, smartConnectReferenceIndex } = smartConnect;
                let prefixNvram = prefix === "smart_connect" ? smartConnectReferenceIndex : prefix;
                let { authMethodValue, joinSmartConnect } = wlBandSeq[prefixNvram];
                if (
                    authMethodValue === "psk" ||
                    authMethodValue === "psk2" ||
                    authMethodValue === "sae" ||
                    authMethodValue === "pskpsk2" ||
                    authMethodValue === "psk2sae"
                ) {
                    let target = document.getElementById(`${prefix}_wpa_key_field`);
                    if (target) {
                        let displayFlag = (() => {
                            let { dwbMode, dwbBand } = aMesh;
                            if (dwbMode === "1" && dwbBand === prefix) {
                                return "";
                            }

                            if (prefix === "smart_connect") {
                                return "";
                            }

                            return smartConnectEnable && joinSmartConnect ? "none" : "";
                        })();
                        target.style.display = displayFlag;
                    }
                }
            }

            function mfpHandler(prefix) {
                let { wlBandSeq, smartConnect, mfpObject, aMesh } = systemManipulable;
                let { smartConnectEnable, smartConnectReferenceIndex } = smartConnect;
                let prefixNvram = prefix === "smart_connect" ? smartConnectReferenceIndex : prefix;
                let { authMethodValue, mfpValue, joinSmartConnect } = wlBandSeq[prefixNvram];
                let mfpStringObject = objectDeepCopy(mfpObject);
                let mfpSnipper = "";
                if (
                    authMethodValue === "openowe" ||
                    authMethodValue === "owe" ||
                    authMethodValue === "sae" ||
                    authMethodValue === "wpa3" ||
                    authMethodValue === "suite-b"
                ) {
                    delete mfpStringObject["0"];
                    delete mfpStringObject["1"];
                } else if (authMethodValue === "psk2sae" || authMethodValue === "wpa2wpa3") {
                    delete mfpStringObject["0"];
                } else if (authMethodValue === "pskpsk2" || authMethodValue === "wpawpa2") {
                    delete mfpStringObject["2"];
                }

                if (
                    authMethodValue === "openowe" ||
                    authMethodValue === "owe" ||
                    authMethodValue === "psk2" ||
                    authMethodValue === "sae" ||
                    authMethodValue === "pskpsk2" ||
                    authMethodValue === "psk2sae" ||
                    authMethodValue === "wpa2" ||
                    authMethodValue === "wpa3" ||
                    authMethodValue === "suite-b" ||
                    authMethodValue === "wpawpa2" ||
                    authMethodValue === "wpa2wpa3"
                ) {
                    for (let [value, desc] of Object.entries(mfpStringObject)) {
                        mfpSnipper += `
                                     <option value="${value}"
                                     ${mfpValue === value ? "selected" : ""}>${desc}</option>
                                 `;
                    }

                    let target = document.getElementById(`${prefix}_mfp`);
                    if (target) {
                        target.innerHTML = mfpSnipper;
                    }

                    target = document.getElementById(`${prefix}_mfp_field`);
                    if (target) {
                        let displayFlag = (() => {
                            let { dwbMode, dwbBand } = aMesh;
                            if (dwbMode === "1" && dwbBand === prefix) {
                                return "";
                            }

                            if (prefix === "smart_connect") {
                                return "";
                            }

                            return smartConnectEnable && joinSmartConnect ? "none" : "";
                        })();
                        target.style.display = displayFlag;
                    }
                }
            }

            function groupKeyHandler(prefix) {
                let { wlBandSeq, smartConnect, aMesh } = systemManipulable;
                let { smartConnectEnable, smartConnectReferenceIndex } = smartConnect;
                let prefixNvram = prefix === "smart_connect" ? smartConnectReferenceIndex : prefix;
                let { authMethodValue, joinSmartConnect } = wlBandSeq[prefixNvram];
                if (
                    authMethodValue === "openowe" ||
                    authMethodValue === "owe" ||
                    authMethodValue === "psk" ||
                    authMethodValue === "psk2" ||
                    authMethodValue === "sae" ||
                    authMethodValue === "pskpsk2" ||
                    authMethodValue === "psk2sae" ||
                    authMethodValue === "wpa" ||
                    authMethodValue === "wpa2" ||
                    authMethodValue === "wpa3" ||
                    authMethodValue === "suite-b" ||
                    authMethodValue === "wpawpa2" ||
                    authMethodValue === "wpa2wpa3"
                ) {
                    let target = document.getElementById(`${prefix}_group_key_field`);
                    if (target) {
                        let displayFlag = (() => {
                            let { dwbMode, dwbBand } = aMesh;
                            if (dwbMode === "1" && dwbBand === prefix) {
                                return "";
                            }

                            if (prefix === "smart_connect") {
                                return "";
                            }

                            return smartConnectEnable && joinSmartConnect ? "none" : "";
                        })();
                        target.style.display = displayFlag;
                    }
                }
            }

            function wepEncryptionHandler(prefix) {
                let { wlBandSeq, smartConnect, wepEncryptObject, aMesh } = systemManipulable;
                let { smartConnectEnable, smartConnectReferenceIndex } = smartConnect;
                let prefixNvram = prefix === "smart_connect" ? smartConnectReferenceIndex : prefix;
                let { authMethodValue, wlModeValue, wepEncryptValue, joinSmartConnect } = wlBandSeq[prefixNvram];
                let wepEncryptStringObject = objectDeepCopy(wepEncryptObject);
                if (authMethodValue === "shared") {
                    delete wepEncryptStringObject["0"];
                }

                if (authMethodValue === "shared" && wepEncryptValue === "0") {
                    wepEncryptValue = "1";
                    systemManipulable.wlBandSeq[prefix].wepEncryptValue = wepEncryptValue;
                }

                let wepEncryptSnippet = "";
                if ((authMethodValue === "open" && wlModeValue === "2") || authMethodValue === "shared") {
                    for (let [value, desc] of Object.entries(wepEncryptStringObject)) {
                        wepEncryptSnippet += `<option value="${value}" ${wepEncryptValue === value ? "selected" : ""}>${desc}</option>`;
                    }

                    wepEncryptionChange(wepEncryptValue, prefix);
                    let target = document.getElementById(`${prefix}_wep_encrypt`);
                    if (target) {
                        target.innerHTML = wepEncryptSnippet;
                    }

                    target = document.getElementById(`${prefix}_wep_encrypt_field`);
                    if (target) {
                        let displayFlag = (() => {
                            let { dwbMode, dwbBand } = aMesh;
                            if (dwbMode === "1" && dwbBand === prefix) {
                                return "";
                            }

                            if (prefix === "smart_connect") {
                                return "";
                            }

                            return smartConnectEnable && joinSmartConnect ? "none" : "";
                        })();
                        target.style.display = displayFlag;
                    }
                }
            }

            function wepEncryptionChange(wepEncryptValue, prefix) {
                let wepSettingsElement = [
                    "_wep_key_index_field",
                    "_wep_key1_field",
                    "_wep_key2_field",
                    "_wep_key3_field",
                    "_wep_key4_field",
                    "_pass_phrase_field",
                ];

                let { wlBandSeq, smartConnect } = systemManipulable;
                let { smartConnectEnable, smartConnectReferenceIndex } = smartConnect;
                let prefixNvram = prefix === "smart_connect" ? smartConnectReferenceIndex : prefix;
                let { wlModeValue, joinSmartConnect } = wlBandSeq[prefixNvram];
                let wepEncryptDescElement = ["_wep_encrypt_desc_64", "_wep_encrypt_desc_128"];
                let displayFlag = (() => {
                    if (smartConnectEnable && joinSmartConnect) {
                        return "none";
                    }

                    return wepEncryptValue === "0" ? "none" : "";
                })();
                wepSettingsElement.forEach((element) => {
                    let target = document.getElementById(`${prefix}${element}`);
                    if (target) {
                        target.style.display = displayFlag;
                    }
                });

                wepEncryptDescElement.forEach((element) => {
                    let target = document.getElementById(`${prefix}${element}`);
                    if (target) {
                        target.style.display = "none";
                    }
                });

                if (wepEncryptValue === "1") {
                    let target = document.getElementById(`${prefix}_wep_encrypt_desc_64`);
                    if (target) {
                        target.style.display = "";
                    }
                } else if (wepEncryptValue === "2") {
                    let target = document.getElementById(`${prefix}_wep_encrypt_desc_128`);
                    if (target) {
                        target.style.display = "";
                    }
                }
            }

            function radiusHandler(prefix) {
                let radiusSettingsElement = ["_radius_ip_field", "_radius_port_field", "_radius_key_field"];
                let { wlBandSeq, smartConnect, aMesh } = systemManipulable;
                let { smartConnectEnable, smartConnectReferenceIndex } = smartConnect;
                let prefixNvram = prefix === "smart_connect" ? smartConnectReferenceIndex : prefix;
                let { authMethodValue, joinSmartConnect } = wlBandSeq[prefixNvram];
                let displayFlag = (() => {
                    let { dwbMode, dwbBand } = aMesh;
                    if (
                        authMethodValue === "wpa" ||
                        authMethodValue === "wpa2" ||
                        authMethodValue === "wpa3" ||
                        authMethodValue === "wpawpa2" ||
                        authMethodValue === "wpa2wpa3" ||
                        authMethodValue === "suite-b" ||
                        authMethodValue === "radius"
                    ) {
                        if (dwbMode === "1" && dwbBand === prefix) {
                            return "";
                        }

                        if (prefix === "smart_connect") {
                            return "";
                        }

                        return smartConnectEnable && joinSmartConnect ? "none" : "";
                    }
                })();

                radiusSettingsElement.forEach((element) => {
                    let target = document.getElementById(`${prefix}${element}`);
                    if (target) {
                        target.style.display = displayFlag;
                    }
                });
            }

            function authenticationMethodChange(authMethodValue, prefix) {
                const elementsArray = [
                    "_wpa_encrypt_field",
                    "_wpa_key_field",
                    "_mfp_field",
                    "_group_key_field",
                    "_wep_encrypt_field",
                    "_wep_key_index_field",
                    "_wep_key1_field",
                    "_wep_key2_field",
                    "_wep_key3_field",
                    "_wep_key4_field",
                    "_pass_phrase_field",
                    "_radius_ip_field",
                    "_radius_port_field",
                    "_radius_key_field",
                ];

                let { smartConnect, mloEnabled } = systemManipulable;
                let { smartConnectEnable, smartConnectReferenceIndex } = smartConnect;
                let prefixNvram = prefix === "smart_connect" ? smartConnectReferenceIndex : prefix;
                if (systemManipulable.wlBandSeq[prefixNvram]) {
                    const { beSupport, wifi7ModeEnabled, authMethodValue: authMethodValueOri } = systemManipulable.wlBandSeq[prefixNvram];
                    systemManipulable.wlBandSeq[prefixNvram].authMethodValue = authMethodValue;
                    if (beSupport) {
                        if (authMethodValue === "sae") {
                            // systemManipulable.wlBandSeq[prefixNvram].wifi7ModeEnabled = true;
                        } else {
                            if (mloEnabled) {
                                confirm_asus({
                                    title: "MLO Hint",
                                    contentA: `<b><#WiFi7_mlo_adjust_hint#></b>`,
                                    contentC: "",
                                    left_button: "<#CTL_Cancel#>",
                                    left_button_callback: function () {
                                        confirm_cancel();
                                        document.getElementById(`${prefix}_auth_method`).value = authMethodValueOri;
                                        return false;
                                    },
                                    left_button_args: {},
                                    right_button: "<#btn_go#>",
                                    right_button_callback: function () {
                                        confirm_cancel();
                                        location.href = "/MLO.asp";
                                    },
                                    right_button_args: {},
                                    iframe: "",
                                    margin: (() => {
                                        return `${document.documentElement.scrollTop}px 0 0 25px`;
                                    })(),
                                    note_display_flag: 0,
                                });
                            } else if (
                                wifi7ModeEnabled &&
                                authMethodValue !== "psk2sae" &&
                                authMethodValue !== "wpa2wpa3" &&
                                authMethodValue !== "wpa3" &&
                                authMethodValue !== "suite-b" &&
                                authMethodValue !== "owe"
                            ) {
                                confirm_asus({
                                    title: "",
                                    contentA: `<#WiFi7_disable_note#>`,
                                    contentC: "",
                                    left_button: "<#checkbox_No#>",
                                    left_button_callback: function () {
                                        confirm_cancel();
                                        document.getElementById(`${prefix}_auth_method`).value = authMethodValueOri;
                                        return false;
                                    },
                                    left_button_args: {},
                                    right_button: "<#checkbox_Yes#>",
                                    right_button_callback: function () {
                                        confirm_cancel();
                                        systemManipulable.wlBandSeq[prefixNvram].wifi7ModeEnabled = false;
                                    },
                                    right_button_args: {},
                                    iframe: "",
                                    margin: (() => {
                                        return `${document.documentElement.scrollTop}px 0 0 25px`;
                                    })(),
                                    note_display_flag: 0,
                                });
                            }
                        }
                    }
                }

                elementsArray.forEach((element) => {
                    let target = document.getElementById(`${prefix}${element}`);
                    if (target) {
                        target.style.display = "none";
                    }
                });

                wpaEncryptHandler(prefix);
                wpaKeyHandler(prefix);
                mfpHandler(prefix);
                groupKeyHandler(prefix);
                wepEncryptionHandler(prefix);
                radiusHandler(prefix);
            }

            function generateSmartConnectRadio() {
                let { wlBandSeq, smartConnect, aMesh } = systemManipulable;
                let { aMeshPrelinkSupport } = aMesh;
                let { version, v1Type } = smartConnect;
                let radioBandSnippet = "";
                if (version === "v2") {
                    let { v2Band, radioSeqArray } = smartConnect;
                    for (let { name, prefixNvram } of Object.values(wlBandSeq)) {
                        let checked = (() => {
                            let index = radioSeqArray.findIndex((element) => element === prefixNvram);
                            return v2Band[index] === "1" ? "checked" : "";
                        })();

                        radioBandSnippet += `<input id="smart_connect_check_${prefixNvram}" type="checkbox" onchange="smartConnectRadioChange(this.checked,'${prefixNvram}')" ${checked} />${name}`;
                    }
                } else {
                    // Smart Connect v1
                    if (wlBandSeq["2g1"] && wlBandSeq["5g1"] && wlBandSeq["6g1"]) {
                        radioBandSnippet += `
                            <select class="input_option" id="smart_connect_x" onChange="smartConnectChange(this.value)">
                                <option class="content_input_fd" value="1"
                                ${v1Type === "1" ? "selected" : ""}><#smart_connect_tri#> (2.4 GHz, 5 GHz and 6 GHz)</optio>
                                <option class="content_input_fd" value="3"
                                ${v1Type === "3" ? "selected" : ""}><#smart_connect_dual#> (2.4 GHz and 5 GHz)</optio>
                            </select>
                        `;
                    } else if (wlBandSeq["2g1"] && wlBandSeq["5g1"] && wlBandSeq["5g2"]) {
                        radioBandSnippet += `
                            <select class="input_option" id="smart_connect_x" onChange="smartConnectChange(this.value)">
                                <option class="content_input_fd" value="1"
                                ${v1Type === "1" ? "selected" : ""}><#smart_connect_tri#> (2.4 GHz, 5 GHz-1 and 5 GHz-2)</optio>
                                ${
                                    aMeshPrelinkSupport
                                        ? ""
                                        : `<option class="content_input_fd" value="2"
                                        ${v1Type === "2" ? "selected" : ""}><#smart_connect_dual#> (5 GHz-1 and 5 GHz-2)</optio>`
                                }
                            </select>
                        `;
                    } else {
                        radioBandSnippet += `
                            <select class="input_option" id="smart_connect_x" onChange="smartConnectChange(this.value)">
                                <option  class="content_input_fd" value="1"
                                ${v1Type === "1" ? "selected" : ""}><#smart_connect_dual#> (2.4 GHz and 5 GHz)</optio>
                            </select>
                        `;
                    }
                }

                return `
                    <tr id="smart_connect_band_field">
                   		<th>Radio Bands</th>
                   		<td>${radioBandSnippet}</td>
                   	</tr>
                `;
            }
            function generateSSID(prefix) {
                let { wlBandSeq, smartConnect, aMesh } = systemManipulable;
                let { smartConnectEnable, v2Band, smartConnectReferenceIndex, radioSeqArray, version, v1Type } = smartConnect;
                let prefixNvram = prefix === "smart_connect" ? smartConnectReferenceIndex : prefix;
                let { ssidValue, hideSSIDValue } = wlBandSeq[prefixNvram];
                let displayFlag = (() => {
                    let { dwbMode, dwbBand } = aMesh;
                    if (dwbMode === "1" && dwbBand === prefix) {
                        return "";
                    }

                    if (prefix === "smart_connect") {
                        return smartConnectEnable ? "" : "none";
                    }

                    if (!smartConnectEnable) {
                        return "";
                    }

                    if (version === "v2") {
                        let index = radioSeqArray.findIndex((element) => element === prefixNvram);
                        return v2Band[index] === "1" ? "none" : "";
                    } else {
                        // SMART CONNECT v1
                        if (v1Type === "1") {
                            return "none";
                        } else if (v1Type === "2") {
                            if (prefix === "5g1" || prefix === "5g2") {
                                return "none";
                            }
                        } else if (v1Type === "3") {
                            if (prefix === "2g1" || prefix === "5g1") {
                                return "none";
                            }
                        }

                        return "";
                    }
                })();

                return `
                    <tr id="${prefix}_ssid_field" style="display:${displayFlag}">
                        <th>
                            <a class="hintstyle" href="javascript:void(0);" onClick="openHint(0, 1);"><#QIS_finish_wireless_item1#></a>
                        </th>
                        <td>
                            <input id="${prefix}_ssid" type="text" maxlength="32" class="input_32_table" value="${ssidValue}" onkeypress="validator.isString(this, event)" autocorrect="off" autocapitalize="off">
                        </td>
                    </tr>
                    <tr id="${prefix}_hide_ssid_field" style="display:${displayFlag}">
                        <th>
                            <a class="hintstyle" href="javascript:void(0);" onClick="openHint(0, 2);"><#WLANConfig11b_x_BlockBCSSID_itemname#></a>
                        </th>
                        <td>
                            <input id="${prefix}_hide_ssid_yes"  name=${prefix}_hide_ssid  type="radio" value="1" class="input"
                            ${hideSSIDValue === "1" ? "checked" : ""}><#checkbox_Yes#>
                            <input name=${prefix}_hide_ssid type="radio" value="0" class="input"
                            ${hideSSIDValue === "0" ? "checked" : ""}><#checkbox_No#>
                        </td>
                    </tr>
                `;
            }

            function generateWirelessBackhaul(prefix) {
                if (prefix !== nbandListArray[dwb_band]) {
                    return "";
                }

                let { wlBandSeq, smartConnect, fh_ap_enabled, aMesh } = systemManipulable;
                let { smartConnectReferenceIndex, smartConnectEnable } = smartConnect;
                let prefixNvram = prefix === "smart_connect" ? smartConnectReferenceIndex : prefix;
                let { joinSmartConnect } = wlBandSeq[prefixNvram];
                let cfgClientList = httpApi.hookGet("get_cfg_clientlist");
                let { fronthaul_ap_option_off, fronthaul_ap_option_on } = httpApi.aimesh_get_node_capability(cfgClientList[0]);
                let displayFlag = (() => {
                    let { dwbMode } = aMesh;
                    let smartConnectIncludeDwbBand =
                        document.getElementById(`smart_connect_check_${prefix}`) &&
                        document.getElementById(`smart_connect_check_${prefix}`).checked &&
                        smartConnectEnable;
                    return dwbMode === "1" && smartConnectIncludeDwbBand && fronthaul_ap_option_on && fronthaul_ap_option_off ? "" : "none";
                })();

                return `
                    <tr id="mesh_backhaul_field" style="display:${displayFlag}">
                        <th>AiMesh Wireless Backhaul</th>
                        <td>
                            <select id="fh_ap_enabled" class="input_option">
                                ${
                                    fronthaul_ap_option_on
                                        ? `<option value="2"
                                        ${fh_ap_enabled === "2" ? "selected" : ""}><#AiMesh_WiFi_Backhaul_both#></option>`
                                        : ""
                                }
                                ${
                                    fronthaul_ap_option_off
                                        ? `<option value="0"
                                        ${fh_ap_enabled === "0" ? "selected" : ""}><#AiMesh_WiFi_Backhaul_dedicated_backhaul#></option>`
                                        : ""
                                }
                            </select>
                        </td>
                    </tr>
                `;
            }

            function generateChannelBandwidth(prefix) {
                let { wlBandSeq, smartConnect, channelBandwidthObject } = systemManipulable;
                let { smartConnectReferenceIndex } = smartConnect;
                let prefixNvram = prefix === "smart_connect" ? smartConnectReferenceIndex : prefix;
                let { bw80MHzSupport, bw160MHzSupport, bw320MHzSupport, bw80_80MHzSupport, bw160Value, bandwidthValue } =
                    wlBandSeq[prefixNvram];
                let bandwidthStringObject = objectDeepCopy(channelBandwidthObject);
                let autoBandwidthString = bandwidthStringObject.auto.string;
                if (!bw320MHzSupport) {
                    autoBandwidthString = autoBandwidthString.split("/320")[0];
                    delete bandwidthStringObject["320mhz"];
                }

                if (!bw320MHzSupport && (!bw160MHzSupport || (bw160MHzSupport && bw160Value === "0"))) {
                    autoBandwidthString = autoBandwidthString.split("/160")[0];
                    delete bandwidthStringObject["160mhz"];
                }

                if (!bw80_80MHzSupport) {
                    delete bandwidthStringObject["80_80mhz"];
                }

                if (!bw80MHzSupport) {
                    autoBandwidthString = autoBandwidthString.split("/80")[0];
                    delete bandwidthStringObject["80mhz"];
                }

                bandwidthStringObject.auto.string = `${autoBandwidthString} MHz`;
                let channelBandwidthSnippet = "";
                for (let { value, string } of Object.values(bandwidthStringObject)) {
                    channelBandwidthSnippet += `<option value="${value}" ${bandwidthValue === value ? "selected" : ""}>${string}</option>`;
                }

                let bw160Snippet = "";
                if (bw160MHzSupport && !bw320MHzSupport) {
                    bw160Snippet += `
                        <span>
                            <input id="${prefix}_bw160_enable" type="checkbox" onClick="bandwidth160MHzEnable(this.checked, '${prefix}')"
                             ${bw160Value === "1" ? "checked" : ""}><#WLANConfig11b_ChannelBW_Enable160M#>
                        </span>
                    `;
                }

                return `
                    <tr>
                        <th><#WLANConfig11b_ChannelBW_itemname#></th>
                        <td>
                            <select id="${prefix}_channel_bandwidth" class="input_option" onChange="channelBandwidthChange(this.value, '${prefix}')">${channelBandwidthSnippet}</select>
                            ${bw160Snippet}
                        </td>
                    </tr>
                `;
            }

            function generateControlChannel(prefix) {
                let { wlBandSeq, smartConnect, psc6g, acs_dfs, acs_band3, acs_unii4, acs_ch13, language } = systemManipulable;
                let { smartConnectEnable, v2Band, smartConnectReferenceIndex, radioSeqArray } = smartConnect;
                let prefixNvram = prefix === "smart_connect" ? smartConnectReferenceIndex : prefix;
                let {
                    bandwidthValue,
                    curCtrlChannel,
                    channelValue,
                    channel,
                    pscChannel,
                    ch320MHz,
                    ch160MHz,
                    ch80MHz,
                    ch40MHz,
                    ch20MHz,
                    acsCH13Support,
                    dfsSupport,
                    uNII4Support,
                    bw160Value,
                } = wlBandSeq[prefixNvram];
                if ((prefix === "6g1" || prefix === "6g2") && psc6g === "1") {
                    channel = [...pscChannel];
                }

                let channelSnippet = `<option value="0"><#Auto#></option>`;
                for (let element of channel) {
                    let chValue = "";
                    let selected = "";
                    let chNumber = 0;
                    if (bandwidthValue === "0") {
                        // Auto Bandwidth
                        if (ch320MHz[element]) {
                            // separate channel number due to the 320-1 and 320-2
                            chValue = element;

                            // to get channel number
                            chNumber = channelValue.split("/320")[0].slice(2);
                        } else if (ch160MHz[element] && bw160Value === "1") {
                            chValue = ch160MHz[element][0];
                            if (prefix === "6g1" || prefix === "6g2") {
                                chNumber = channelValue.split("/160")[0].slice(2);
                            } else {
                                chNumber = channelValue.split("/160")[0];
                            }
                        } else if (ch80MHz[element]) {
                            chValue = ch80MHz[element][0];
                            if (prefix === "6g1" || prefix === "6g2") {
                                chNumber = channelValue.split("/80")[0].slice(2);
                            } else {
                                chNumber = channelValue.split("/80")[0];
                            }
                        } else if (ch40MHz[element]) {
                            if (prefix === "2g1") {
                                // separate channel number due to the channel likes 5u, 5l
                                chValue = element;
                                chNumber = channelValue.split("u")[0].split("l")[0];
                            } else {
                                chValue = ch40MHz[element][0];
                                if (prefix === "6g1" || prefix === "6g2") {
                                    chNumber = channelValue.split("/40")[0].slice(2);
                                } else {
                                    chNumber = channelValue.split("u")[0].split("l")[0];
                                }
                            }
                        } else {
                            chValue = ch20MHz[element][0];
                            chNumber = channelValue;
                        }
                    } else if (bandwidthValue === "6") {
                        // 320 MHz
                        if (!ch320MHz[element]) {
                            continue;
                        }

                        chValue = element;
                        chNumber = channelValue.split("/320")[0].slice(2);
                    } else if (bandwidthValue === "5") {
                        // 160 MHz
                        if (!ch160MHz[element]) {
                            continue;
                        }

                        chValue = ch160MHz[element][0];
                        if (prefix === "6g1" || prefix === "6g2") {
                            chNumber = channelValue.split("/160")[0].slice(2);
                        } else {
                            chNumber = channelValue.split("/160")[0];
                        }
                    } else if (bandwidthValue === "3") {
                        // 80 MHz
                        if (!ch80MHz[element]) {
                            continue;
                        }

                        chValue = ch80MHz[element][0];
                        if (prefix === "6g1" || prefix === "6g2") {
                            chNumber = channelValue.split("/80")[0].slice(2);
                        } else {
                            chNumber = channelValue.split("/80")[0];
                        }
                    } else if (bandwidthValue === "2") {
                        // 40 MHz
                        if (!ch40MHz[element]) {
                            continue;
                        }

                        if (prefix === "2g1") {
                            // separate channel number due to the channel likes 5u, 5l
                            chValue = element;
                            chNumber = channelValue.split("u")[0].split("l")[0];
                        } else {
                            // 5 GHz, 6 GHz
                            chValue = ch40MHz[element][0];
                            if (prefix === "6g1" || prefix === "6g2") {
                                chNumber = channelValue.split("/40")[0].slice(2);
                            } else {
                                chNumber = channelValue.split("u")[0].split("l")[0];
                            }
                        }
                    } else if (bandwidthValue === "1") {
                        // 20 MHz
                        chValue = ch20MHz[element][0];
                        chNumber = channelValue;
                    }

                    selected = element === chNumber ? "selected" : "";
                    channelSnippet += `<option value="${chValue}" ${selected}>${element}</option>`;
                }

                let currentChannelSnippet = `<span id="${prefix}_current_channel" style="margin-left: 10px;display:${
                    channelValue === "0" ? "" : "none"
                }">Current Control Channel: ${curCtrlChannel}</span>`;

                let acsCh13Snippet = "";
                if (prefix === "2g1" && acsCH13Support) {
                    acsCh13Snippet = `
                        <div id="${prefix}_acs_ch13_field" style="display:${channelValue === "0" ? "" : "none"}">
                            <span>
                                <input id="${prefix}_acs_ch13" type="checkbox"
                                ${acs_ch13 === "1" ? "checked" : ""}><#WLANConfig11b_EChannel_acs_ch13#>
                            </span>
                        </div>
                    `;
                }

                let psc6gSnippet = "";
                if (prefix === "6g1" || prefix === "6g2") {
                    // due to HTML tag <a> with hard code ID in the dict string, it needs to be replaced
                    let { currentLang } = language;
                    let faqOldTag = `<a id="psc_faq_link">`;
                    let faqLink = `https://nw-dlcdnet.asus.com/support/forward.html?model=&type=Faq&lang=${currentLang}&kw=&num=151`;
                    let faqNewTag = `<a class="a-hint-text" href="${faqLink}" target="_blank">`;
                    let faqStr = `<#PSC_Faq#>`;
                    faqStr = faqStr.replace(faqOldTag, faqNewTag);
                    psc6gSnippet += `
                        <div id="${prefix}_psc6g_field">
                            <div>
                                <span>
                                    <input id="${prefix}_psc6g" type="checkbox"
                                    ${
                                        psc6g === "1" ? "checked" : ""
                                    } onClick="pscChannelEnable(this.checked, '${prefix}')"><#Enable_PSC_Hint#>
                                </span>
                            </div>
                            <div><span id="${prefix}_auto_suggest"><#WLANConfig11b_AuthenticationMethod_auto_desc#></span></div>
                            <div><span>${faqStr}</span></div>
                        </div>
                    `;
                }

                let acsDfsSnippet = "";
                if (dfsSupport) {
                    const disableFlag = (() => {
                        if (prefix === "5g2") {
                            if (channel.includes("100") && !channel.includes("149")) {
                                return "disabled";
                            }
                        }

                        return "";
                    })();
                    acsDfsSnippet = `
                        <div id="${prefix}_acs_dfs_field" style="display:${channelValue === "0" ? "" : "none"}">
                            <span>
                                <input id="${prefix}_acs_dfs" type="checkbox" ${disableFlag}
                                ${prefix === "5g1" && acs_dfs === "1" ? "checked" : ""}
                                ${prefix === "5g2" && acs_band3 === "1" ? "checked" : ""}><#WLANConfig11b_EChannel_dfs#>
                            </span>
                        </div>
                    `;
                }

                let uNII4Snippet = "";
                if (uNII4Support) {
                    uNII4Snippet += `
                        <div id="${prefix}_unii4_field" style="display:${channelValue === "0" ? "" : "none"}">
                            <span>
                                <input id="${prefix}_unii4" type="checkbox"
                                ${acs_unii4 === "1" ? "checked" : ""}><#WLANConfig11b_EChannel_U-NII-4#>
                            </span>
                        </div>
                    `;
                }

                return `
                    <tr>
                        <th><#WLANConfig11b_Channel_itemname#></th>
                        <td>
                            <select id="${prefix}_control_channel" class="input_option" onChange="controlChannelChange(this.value, '${prefix}')">${channelSnippet}</select>
                            ${currentChannelSnippet}
                            ${acsCh13Snippet}
                            ${acsDfsSnippet}
                            ${uNII4Snippet}
                            ${psc6gSnippet}
                        </td>
                    </tr>
                `;
            }

            function generateExtensionChannel(prefix) {
                let { wlBandSeq, smartConnect, isBRCMplatform, extensionChannelObject } = systemManipulable;
                let { smartConnectReferenceIndex } = smartConnect;
                let prefixNvram = prefix === "smart_connect" ? smartConnectReferenceIndex : prefix;
                let { channelValue, chanspecs, ch320MHz, bandwidthValue, extChannelValue } = wlBandSeq[prefixNvram];
                let extensionChannelString = objectDeepCopy(extensionChannelObject);

                // if channel bandwidth is 20 MHz then not to display extension channel
                let displayFlag = (() => {
                    if (isBRCMplatform) {
                        if (bandwidthValue === "1" || channelValue === "14") {
                            return "none";
                        }
                    } else {
                        // MTK, QCA platforms
                        if (bandwidthValue === "0") {
                            return "none";
                        }
                    }

                    return "";
                })();

                if (isBRCMplatform) {
                    delete extensionChannelString["lower"];
                    delete extensionChannelString["upper"];
                } else {
                    delete extensionChannelString["l"];
                    delete extensionChannelString["u"];
                }

                if (prefix !== "6g1" && prefix !== "6g2") {
                    delete extensionChannelString["320-1"];
                    delete extensionChannelString["320-2"];
                }

                if (channelValue === "0" || prefix === "5g1" || prefix === "5g2") {
                    delete extensionChannelString["l"];
                    delete extensionChannelString["u"];
                    delete extensionChannelString["320-1"];
                    delete extensionChannelString["320-2"];
                    delete extensionChannelString["lower"];
                    delete extensionChannelString["upper"];
                } else {
                    let ch = channelValue.slice(0, -1);
                    if (prefix === "2g1") {
                        delete extensionChannelString["0"];
                        if (chanspecs.indexOf(`${ch}u`) === -1) {
                            delete extensionChannelString["u"];
                        }

                        if (chanspecs.indexOf(`${ch}l`) === -1) {
                            delete extensionChannelString["l"];
                        }
                    } else if (prefix === "6g1" || prefix === "6g2") {
                        delete extensionChannelString["l"];
                        delete extensionChannelString["u"];
                        if (bandwidthValue !== "0" && bandwidthValue !== "6") {
                            delete extensionChannelString["320-1"];
                            delete extensionChannelString["320-2"];
                        } else {
                            let ch = channelValue.split("/320")[0].slice(2);
                            if (ch320MHz[ch]) {
                                delete extensionChannelString["0"];
                                if (ch320MHz[ch].indexOf(`6g${ch}/320-1`) === -1) {
                                    delete extensionChannelString["320-1"];
                                }

                                if (ch320MHz[ch].indexOf(`6g${ch}/320-2`) === -1) {
                                    delete extensionChannelString["320-2"];
                                }
                            } else {
                                // for channel that not support 320 MHz
                                delete extensionChannelString["320-1"];
                                delete extensionChannelString["320-2"];
                            }
                        }
                    }
                }

                let postfix = "0";
                if (channelValue.indexOf("u") !== -1 || channelValue.indexOf("l") !== -1) {
                    postfix = channelValue.slice(-1);
                } else if (channelValue.indexOf("320-1") !== -1 || channelValue.indexOf("320-2") !== -1) {
                    postfix = channelValue.split("/")[1];
                } else {
                    // MTK, QCA platforms
                    postfix = extChannelValue;
                }

                let extensionChannelSnippet = "";
                for (let [value, desc] of Object.entries(extensionChannelString)) {
                    extensionChannelSnippet += `<option value="${value}" ${postfix === value ? "selected" : ""}>${desc}</option>`;
                }

                return `
                    <tr id="${prefix}_extension_channel_field" style="display:${displayFlag}">
                        <th><#WLANConfig11b_EChannel_itemname#></th>
                        <td>
                            <select id="${prefix}_extension_channel" class="input_option">${extensionChannelSnippet}</select>
                        </td>
                    </tr>
                `;
            }

            function generateAuthenticationMethod(prefix) {
                let { wlBandSeq, newWiFiCertSupport, wifiLogoSupport, isKRSku, currentOPMode, smartConnect, aMesh } = systemManipulable;
                let { aMeshSupport, aMeshRouterSupport } = aMesh;
                let { smartConnectEnable, v2Band, smartConnectReferenceIndex } = smartConnect;
                let prefixNvram = prefix === "smart_connect" ? smartConnectReferenceIndex : prefix;
                let { wlModeValue, authMethod, authMethodValue, joinSmartConnect } = wlBandSeq[prefixNvram];
                let displayFlagAuthMethodSuggest = authMethodValue === "open" ? "" : "none";
                let displayFlagAuthMethod = (() => {
                    let { dwbMode, dwbBand } = aMesh;
                    if (dwbMode === "1" && dwbBand === prefix) {
                        return "";
                    }

                    if (prefix === "smart_connect") {
                        return smartConnectEnable ? "" : "none";
                    }

                    if (smartConnectEnable && joinSmartConnect) {
                        return "none;";
                    }

                    return "";
                })();

                if (newWiFiCertSupport) {
                    delete authMethod["psk"];
                    delete authMethod["wpa"];
                }

                if (wifiLogoSupport) {
                    delete authMethod["shared"];
                    delete authMethod["psk"];
                    delete authMethod["radius"];
                }

                let { id: opModeId } = currentOPMode;
                // Wireless mode: Legacy or Repeater mode
                if (wlModeValue !== "2" || opModeId === "RE") {
                    delete authMethod["shared"];
                    delete authMethod["psk"];
                    delete authMethod["wpa"];
                    delete authMethod["radius"];
                }

                if (isKRSku) {
                    delete authMethod["open"];
                }

                if (aMeshSupport && aMeshRouterSupport && (opModeId === "RT" || opModeId === "AP")) {
                    let reNodeCount = httpApi.hookGet("get_cfg_clientlist").length;

                    // if RE node connected then remove whole Enterprise
                    if (reNodeCount > 1) {
                        delete authMethod["wpa"];
                        delete authMethod["wpa2"];
                        delete authMethod["wpa3"];
                        delete authMethod["suite-b"];
                        delete authMethod["wpawpa2"];
                        delete authMethod["wpa2wpa3"];
                        delete authMethod["radius"];
                    }
                }

                let authMethodSnippet = "";
                for (let [value, desc] of Object.entries(authMethod)) {
                    authMethodSnippet += `<option value='${value}' ${authMethodValue === value ? "selected" : ""}>${desc}</option>`;
                }

                return `
                    <tr id="${prefix}_auth_method_field" style="display:${displayFlagAuthMethod}">
                        <th>
                            <a class="hintstyle" href="javascript:void(0);" onClick="openHint(0, 5);"><#WLANConfig11b_AuthenticationMethod_itemname#></a>
                        </th>
                        <td>
                            <select id="${prefix}_auth_method" class="input_option select_auth_mode" onChange="authenticationMethodChange(this.value, '${prefix}')">${authMethodSnippet}</select>
                            <div class="setup_help_icon"></div>
                            <span id="${prefix}_auth_method_suggest" style="display:${displayFlagAuthMethodSuggest}">Suggest to use "Enhanced Open transition" for better device compatibility</span>
                        </td>
                    </tr>
                `;
            }

            function generateWpaEncryption(prefix) {
                let { wlBandSeq, smartConnect, wpaEncryptObject, aMesh } = systemManipulable;
                let { smartConnectEnable, smartConnectReferenceIndex } = smartConnect;
                let prefixNvram = prefix === "smart_connect" ? smartConnectReferenceIndex : prefix;
                let { authMethodValue, wpaEncryptValue, joinSmartConnect, wifi7ModeEnabled } = wlBandSeq[prefixNvram];
                let wpaEncryptStringObject = objectDeepCopy(wpaEncryptObject);
                let displayFlag = (() => {
                    if (
                        authMethodValue === "openowe" ||
                        authMethodValue === "owe" ||
                        authMethodValue === "psk" ||
                        authMethodValue === "psk2" ||
                        authMethodValue === "sae" ||
                        authMethodValue === "pskpsk2" ||
                        authMethodValue === "psk2sae" ||
                        authMethodValue === "wpa" ||
                        authMethodValue === "wpa2" ||
                        authMethodValue === "wpa3" ||
                        authMethodValue === "suite-b" ||
                        authMethodValue === "wpawpa2" ||
                        authMethodValue === "wpa2wpa3"
                    ) {
                        let { dwbMode, dwbBand } = aMesh;
                        if (dwbMode === "1" && dwbBand === prefix) {
                            return "";
                        }

                        if (prefix === "smart_connect") {
                            return smartConnectEnable ? "" : "none";
                        }

                        if (smartConnectEnable && joinSmartConnect) {
                            return "none";
                        }

                        return "";
                    }

                    return "none";
                })();

                if (authMethodValue !== "psk" && authMethodValue !== "wpa") {
                    delete wpaEncryptStringObject["tkip"];
                }

                if (authMethodValue !== "pskpsk2" && authMethodValue !== "wpawpa2") {
                    delete wpaEncryptStringObject["tkip+aes"];
                }

                if (authMethodValue !== "suite-b") {
                    delete wpaEncryptStringObject["suite-b"];
                }

                if (authMethodValue === "suite-b") {
                    delete wpaEncryptStringObject["aes"];
                }

                if (!wifi7ModeEnabled || authMethodValue !== "sae") {
                    delete wpaEncryptStringObject["aes+gcmp256"];
                } else {
                    delete wpaEncryptStringObject["aes"];
                }

                let wpaEncryptSnippet = "";
                for (let [value, desc] of Object.entries(wpaEncryptStringObject)) {
                    wpaEncryptSnippet += `<option value="${value}" ${wpaEncryptValue === value ? "selected" : ""}>${desc}</option>`;
                }

                return `
                	<tr id="${prefix}_wpa_encrypt_field" style="display:${displayFlag}">
                		<th>
                			<a class="hintstyle" href="javascript:void(0);" onClick="openHint(0, 6);"><#WLANConfig11b_WPAType_itemname#></a>
                		</th>
                		<td>
                			<select class="input_option" id="${prefix}_wpa_encrypt">${wpaEncryptSnippet}</select>
                		</td>
                	</tr>
                `;
            }

            function generateWpaKey(prefix) {
                let { wlBandSeq, smartConnect, aMesh } = systemManipulable;
                let { smartConnectEnable, smartConnectReferenceIndex } = smartConnect;
                let prefixNvram = prefix === "smart_connect" ? smartConnectReferenceIndex : prefix;
                let { authMethodValue, wpaKeyValue, joinSmartConnect } = wlBandSeq[prefixNvram];
                let displayFlag = (() => {
                    if (
                        authMethodValue === "psk" ||
                        authMethodValue === "psk2" ||
                        authMethodValue === "sae" ||
                        authMethodValue === "pskpsk2" ||
                        authMethodValue === "psk2sae"
                    ) {
                        let { dwbMode, dwbBand } = aMesh;
                        if (dwbMode === "1" && dwbBand === prefix) {
                            return "";
                        }

                        if (prefix === "smart_connect") {
                            return smartConnectEnable ? "" : "none";
                        }

                        if (smartConnectEnable && joinSmartConnect) {
                            return "none";
                        }

                        return "";
                    }

                    return "none";
                })();

                return `
                    <tr id="${prefix}_wpa_key_field" style="display:${displayFlag}">
                        <th>
                        <a class="hintstyle" href="javascript:void(0);" onClick="openHint(0, 7);"><#WLANConfig11b_x_PSKKey_itemname#></a>
                        </th>
                        <td>
                            <input id="${prefix}_wpa_key" type="password" maxlength="64" class="input_32_table" autocorrect="off" autocapitalize="off" onfocus="plainPasswordSwitch(this, 'focus')" onblur="plainPasswordSwitch(this, 'blur')" value="${wpaKeyValue}">
                        </td>
                    </tr>
                `;
            }

            function generateMfp(prefix) {
                let { wlBandSeq, smartConnect, mfpObject, aMesh } = systemManipulable;
                let { smartConnectEnable, smartConnectReferenceIndex } = smartConnect;
                let prefixNvram = prefix === "smart_connect" ? smartConnectReferenceIndex : prefix;
                let { authMethodValue, mfpValue, joinSmartConnect } = wlBandSeq[prefixNvram];
                let mfpStringObject = objectDeepCopy(mfpObject);
                let displayFlag = (() => {
                    if (
                        authMethodValue === "openowe" ||
                        authMethodValue === "owe" ||
                        authMethodValue === "psk2" ||
                        authMethodValue === "sae" ||
                        authMethodValue === "pskpsk2" ||
                        authMethodValue === "psk2sae" ||
                        authMethodValue === "wpa2" ||
                        authMethodValue === "wpa3" ||
                        authMethodValue === "suite-b" ||
                        authMethodValue === "wpawpa2" ||
                        authMethodValue === "wpa2wpa3"
                    ) {
                        let { dwbMode, dwbBand } = aMesh;
                        if (dwbMode === "1" && dwbBand === prefix) {
                            return "";
                        }

                        if (prefix === "smart_connect") {
                            return smartConnectEnable ? "" : "none";
                        }

                        if (smartConnectEnable && joinSmartConnect) {
                            return "none";
                        }

                        return "";
                    }

                    return "none";
                })();

                if (
                    authMethodValue === "openowe" ||
                    authMethodValue === "owe" ||
                    authMethodValue === "sae" ||
                    authMethodValue === "wpa3" ||
                    authMethodValue === "suite-b"
                ) {
                    delete mfpStringObject["0"];
                    delete mfpStringObject["1"];
                } else if (authMethodValue === "psk2sae" || authMethodValue === "wpa2wpa3") {
                    delete mfpStringObject["0"];
                } else if (authMethodValue === "pskpsk2" || authMethodValue === "wpawpa2") {
                    delete mfpStringObject["2"];
                }

                let mfpSnipper = "";
                for (let [value, desc] of Object.entries(mfpStringObject)) {
                    mfpSnipper += `
                        <option value="${value}" ${mfpValue === value ? "selected" : ""}>${desc}</option>
                    `;
                }

                return `
                    <tr id="${prefix}_mfp_field" style="display:${displayFlag}">
                        <th><#WLANConfig11b_x_mfp#></th>
                        <td>
                            <select class="input_option" id="${prefix}_mfp">${mfpSnipper}</select>
                        </td>
                    </tr>
                `;
            }

            function generateGroupKey(prefix) {
                let { wlBandSeq, smartConnect, aMesh } = systemManipulable;
                let { smartConnectEnable, smartConnectReferenceIndex } = smartConnect;
                let prefixNvram = prefix === "smart_connect" ? smartConnectReferenceIndex : prefix;
                let { authMethodValue, wpaGtkValue, joinSmartConnect } = wlBandSeq[prefixNvram];
                let displayFlag = (() => {
                    if (
                        authMethodValue === "openowe" ||
                        authMethodValue === "owe" ||
                        authMethodValue === "psk" ||
                        authMethodValue === "psk2" ||
                        authMethodValue === "sae" ||
                        authMethodValue === "pskpsk2" ||
                        authMethodValue === "psk2sae" ||
                        authMethodValue === "wpa" ||
                        authMethodValue === "wpa2" ||
                        authMethodValue === "wpa3" ||
                        authMethodValue === "suite-b" ||
                        authMethodValue === "wpawpa2" ||
                        authMethodValue === "wpa2wpa3"
                    ) {
                        let { dwbMode, dwbBand } = aMesh;
                        if (dwbMode === "1" && dwbBand === prefix) {
                            return "";
                        }

                        if (prefix === "smart_connect") {
                            return smartConnectEnable ? "" : "none";
                        }

                        if (smartConnectEnable && joinSmartConnect) {
                            return "none";
                        }

                        return "";
                    }

                    return "none";
                })();

                return `
                    <tr id="${prefix}_group_key_field" style="display:${displayFlag}">
                        <th>
                            <a class="hintstyle" href="javascript:void(0);" onClick="openHint(0, 11);"><#WLANConfig11b_x_Rekey_itemname#></a>
                        </th>
                        <td>
                            <input type="text" maxlength="7" id="${prefix}_group_key" class="input_6_table" value="${wpaGtkValue}" onKeyPress="return validator.isNumber(this,event);" autocorrect="off" autocapitalize="off">
                        </td>
                    </tr>
                `;
            }

            function generateWepEncryption(prefix) {
                let { wlBandSeq, smartConnect, wepEncryptObject, aMesh } = systemManipulable;
                let { smartConnectEnable, smartConnectReferenceIndex } = smartConnect;
                let prefixNvram = prefix === "smart_connect" ? smartConnectReferenceIndex : prefix;
                let { wlModeValue, authMethodValue, wepEncryptValue, joinSmartConnect } = wlBandSeq[prefixNvram];
                let wepEncryptStringObject = objectDeepCopy(wepEncryptObject);
                let displayFlag = (() => {
                    if ((authMethodValue === "open" && wlModeValue === "2") || authMethodValue === "shared") {
                        let { dwbMode, dwbBand } = aMesh;
                        if (dwbMode === "1" && dwbBand === prefix) {
                            return "";
                        }

                        if (prefix === "smart_connect") {
                            return smartConnectEnable ? "" : "none";
                        }

                        if (smartConnectEnable && joinSmartConnect) {
                            return "none";
                        }

                        return "";
                    }

                    return "none";
                })();

                // due to the WEP Encryption of Shared Key is no "NONE"
                // set the value to 1 to show WEP Encrypt Description correctly
                if (authMethodValue === "shared" && wepEncryptValue === "0") {
                    wepEncryptValue = "1";
                    systemManipulable.wlBandSeq[prefix].wepEncryptValue = wepEncryptValue;
                }

                if (authMethodValue === "shared") {
                    delete wepEncryptStringObject["0"];
                }

                let wepEncryptSnippet = "";
                for (let [value, desc] of Object.entries(wepEncryptStringObject)) {
                    wepEncryptSnippet += `<option value="${value}" ${wepEncryptValue === value ? "selected" : ""}>${desc}</option>`;
                }

                return `
                    <tr id='${prefix}_wep_encrypt_field' style="display:${displayFlag}">
                        <th>
                            <a class="hintstyle" href="javascript:void(0);" onClick="openHint(0, 9);"><#WLANConfig11b_WEPType_itemname#></a>
                        </th>
                        <td>
                            <select class="input_option" id="${prefix}_wep_encrypt" onChange="wepEncryptionChange(this.value, '${prefix}')">${wepEncryptSnippet}</select>
                            <span id="${prefix}_wep_encrypt_desc_64"
                            style="display:${wepEncryptValue === "1" ? "" : "none"}"><#WLANConfig11b_WEPKey_itemtype1#></span>
                            <span id="${prefix}_wep_encrypt_desc_128"
                            style="display:${wepEncryptValue === "2" ? "" : "none"}"><#WLANConfig11b_WEPKey_itemtype2#></span>
                        </td>
                    </tr>
                `;
            }

            function generateWepKey(prefix) {
                let { wlBandSeq, smartConnect, aMesh } = systemManipulable;
                let { smartConnectEnable, smartConnectReferenceIndex } = smartConnect;
                let prefixNvram = prefix === "smart_connect" ? smartConnectReferenceIndex : prefix;
                let {
                    authMethodValue,
                    wepEncryptValue,
                    wepKeyIndexValue,
                    wepKey1Value,
                    wepKey2Value,
                    wepKey3Value,
                    wepKey4Value,
                    wepPassPhraseValue,
                    joinSmartConnect,
                } = wlBandSeq[prefixNvram];
                let displayFlag = (() => {
                    if ((authMethodValue === "open" && wepEncryptValue !== "0") || authMethodValue === "shared") {
                        let { dwbMode, dwbBand } = aMesh;
                        if (dwbMode === "1" && dwbBand === prefix) {
                            return "";
                        }

                        if (prefix === "smart_connect") {
                            return smartConnectEnable ? "" : "none";
                        }

                        if (smartConnectEnable && joinSmartConnect) {
                            return "none";
                        }

                        return "";
                    }

                    return "none";
                })();

                return `
                    <tr id="${prefix}_wep_key_index_field" style="display:${displayFlag}">
                        <th>
                            <a class="hintstyle" href="javascript:void(0);" onClick="openHint(0, 10);"><#WLANConfig11b_WEPDefaultKey_itemname#></a>
                        </th>
                        <td>
                            <select id="${prefix}_key" class="input_option" onChange="">
                                <option value="1" ${wepKeyIndexValue === "1" ? "selected" : ""}>1</option>
                                <option value="2" ${wepKeyIndexValue === "2" ? "selected" : ""}>2</option>
                                <option value="3" ${wepKeyIndexValue === "3" ? "selected" : ""}>3</option>
                                <option value="4" ${wepKeyIndexValue === "4" ? "selected" : ""}>4</option>
                            </select>
                        </td>
                    </tr>
                    <tr id="${prefix}_wep_key1_field" style="display:${displayFlag}">
                        <th>
                            <a class="hintstyle" href="javascript:void(0);" onClick="openHint(0, 18);"><#WLANConfig11b_WEPKey1_itemname#></a>
                        </th>
                        <td>
                            <input id="${prefix}_key1" type="text" maxlength="32" class="input_32_table" value="${wepKey1Value}" onKeyUp="return change_wlkey(this, 'WLANConfig11b');" autocorrect="off" autocapitalize="off">
                        </td>
                    </tr>
                    <tr id="${prefix}_wep_key2_field" style="display:${displayFlag}">
                        <th>
                            <a class="hintstyle" href="javascript:void(0);" onClick="openHint(0, 18);"><#WLANConfig11b_WEPKey2_itemname#></a>
                        </th>
                        <td>
                            <input id="${prefix}_key2" type="text" maxlength="32" class="input_32_table" value="${wepKey2Value}" onKeyUp="return change_wlkey(this, 'WLANConfig11b');" autocorrect="off" autocapitalize="off">
                        </td>
                    </tr>
                    <tr id="${prefix}_wep_key3_field" style="display:${displayFlag}">
                        <th>
                            <a class="hintstyle" href="javascript:void(0);" onClick="openHint(0, 18);"><#WLANConfig11b_WEPKey3_itemname#></a>
                        </th>
                        <td>
                            <input id="${prefix}_key3" type="text" maxlength="32" class="input_32_table" value="${wepKey3Value}" onKeyUp="return change_wlkey(this, 'WLANConfig11b');" autocorrect="off" autocapitalize="off">
                        </td>
                    </tr>
                    <tr id="${prefix}_wep_key4_field" style="display:${displayFlag}">
                        <th>
                            <a class="hintstyle" href="javascript:void(0);" onClick="openHint(0, 18);"><#WLANConfig11b_WEPKey4_itemname#></a>
                        </th>
                        <td>
                            <input id="${prefix}_key4" type="text" maxlength="32" class="input_32_table" value="${wepKey4Value}" onKeyUp="return change_wlkey(this, 'WLANConfig11b');" autocorrect="off" autocapitalize="off">
                        </td>
                    </tr>
                    <tr id="${prefix}_pass_phrase_field" style="display:${displayFlag}">
                        <th>
                            <a class="hintstyle" href="javascript:void(0);" onClick="openHint(0, 8);"><#WLANConfig11b_x_Phrase_itemname#></a>
                        </th>
                        <td>
                            <input id="${prefix}_pass_phrase" type="text" maxlength="64" class="input_32_table" value="${wepPassPhraseValue}" onKeyUp="return is_wlphrase('WLANConfig11b', 'wl_phrase_x', this);" autocorrect="off" autocapitalize="off">
                        </td>
                    </tr>
                `;
            }

            function generateRadiusSettings(prefix) {
                let { wlBandSeq, smartConnect, aMesh } = systemManipulable;
                let { smartConnectEnable, smartConnectReferenceIndex } = smartConnect;
                let prefixNvram = prefix === "smart_connect" ? smartConnectReferenceIndex : prefix;
                let { authMethodValue, radiusIpValue, radiusPortValue, radiusKeyValue, joinSmartConnect } = wlBandSeq[prefixNvram];
                let displayFlag = (() => {
                    if (
                        authMethodValue === "wpa" ||
                        authMethodValue === "wpa2" ||
                        authMethodValue === "wpa3" ||
                        authMethodValue === "wpawpa2" ||
                        authMethodValue === "wpa2wpa3" ||
                        authMethodValue === "suite-b" ||
                        authMethodValue === "radius"
                    ) {
                        let { dwbMode, dwbBand } = aMesh;
                        if (dwbMode === "1" && dwbBand === prefix) {
                            return "";
                        }

                        if (prefix === "smart_connect") {
                            return smartConnectEnable ? "" : "none";
                        }

                        if (smartConnectEnable && joinSmartConnect) {
                            return "none";
                        }

                        return "";
                    }

                    return "none";
                })();

                return `
                    <tr id="${prefix}_radius_ip_field" style="display:${displayFlag}">
                        <th>
                            <a class="hintstyle" href="javascript:void(0);" onClick="openHint(2,1);"><#WLANAuthentication11a_ExAuthDBIPAddr_itemname#></a>
                        </th>
                        <td>
                            <input id="${prefix}_radius_ip" type="text" maxlength="39" class="input_32_table" value="${radiusIpValue}" onKeyPress="return validator.isIPAddr(this, event)" autocorrect="off" autocapitalize="off">
                        </td>
                    </tr>
                    <tr id="${prefix}_radius_port_field" style="display:${displayFlag}">
                        <th>
                            <a class="hintstyle" href="javascript:void(0);" onClick="openHint(2,2);"><#WLANAuthentication11a_ExAuthDBPortNumber_itemname#></a>
                        </th>
                        <td>
                            <input id="${prefix}_radius_port" type="text" maxlength="5" class="input_6_table" value="${radiusPortValue}" onkeypress="return validator.isNumber(this,event)" autocorrect="off" autocapitalize="off"/>
                        </td>
                    </tr>
                    <tr id="${prefix}_radius_key_field" style="display:${displayFlag}">
                        <th>
                            <a class="hintstyle" href="javascript:void(0);"  onClick="openHint(2,3);"><#WLANAuthentication11a_ExAuthDBPassword_itemname#></a>
                        </th>
                        <td>
                            <input id="${prefix}_radius_key" type="password" maxlength="64" class="input_32_table" value="${radiusKeyValue}" autocorrect="off" autocapitalize="off">
                        </td>
                    </tr>
                `;
            }

            function apply() {
                let { wlBandSeq, isKRSku, smartConnect, isBRCMplatform, isMTKplatform, isQCAplatform, aMesh } = systemManipulable;
                let { smartConnectEnable, radioSeqArray, version } = smartConnect;
                let { dwbBand, dwbMode } = aMesh;
                let postObject = {};
                let commonStringError = {};
                let validateErrorCount = 0;
                if (smartConnectEnable && version === "v2") {
                    postObject.smart_connect_selif_x = (() => {
                        if (smartConnectEnable) {
                            let smartConnectSelifArray = [];
                            for (let element of radioSeqArray) {
                                if (element === "") {
                                    continue;
                                }

                                let smartConnectRadioElement = document.getElementById(`smart_connect_check_${element}`);
                                if (smartConnectRadioElement) {
                                    let value = smartConnectRadioElement.checked ? "1" : "0";
                                    smartConnectSelifArray.push(value);
                                } else {
                                    smartConnectSelifArray.push("0");
                                }
                            }

                            // put string array to binary string and transforms it to decimal
                            let smartConnectSelifStr = smartConnectSelifArray.join("");
                            return parseInt(smartConnectSelifStr, 2);
                        }

                        return 7;
                    })();
                }

                postObject.smart_connect_x = (() => {
                    /*
                     * for Smart Connect v2
                     * with 2.4 GHz return 1
                     * without 2.4 GHz return 2
                     *
                     * for Smart Connect v1
                     * All band return 1
                     * dual 5 GHz return 2
                     * 2.4 GHz & 5 GHz without 6 GHz return 3
                     */

                    if (!smartConnectEnable) {
                        return "0";
                    }

                    if (version === "v2") {
                        let binaryString = postObject.smart_connect_selif_x.toString(2);
                        return binaryString.slice(-1) === "1" ? "1" : "2";
                    }

                    return document.getElementById("smart_connect_x").value;
                })();

                let dwbInfo = {
                    ssid: "",
                    sameSsidCount: 0,
                    targetObject: "",
                    sameSsidString: "The fronthaul SSID is the same as the backhaul SSID.",
                };

                if (dwbMode === "1") {
                    dwbInfo.ssid = document.getElementById(`${dwbBand}_ssid`).value;
                    dwbInfo.targetObject = document.getElementById(`${dwbBand}_ssid`);
                }

                for (let [key, value] of Object.entries(wlBandSeq)) {
                    let { joinSmartConnect, beSupport, wifi7ModeEnabled } = value;

                    // WiFi 7 mode
                    if (beSupport) {
                        postObject[`${key}_11be`] = (() => {
                            return wifi7ModeEnabled ? "1" : "0";
                        })();
                    }

                    //SSID
                    postObject[`${key}_ssid`] = (() => {
                        if (smartConnectEnable && joinSmartConnect) {
                            validator.stringSSID(document.getElementById("smart_connect_ssid")) ? "" : validateErrorCount++;
                            if (dwbMode === "1") {
                                if (dwbInfo.ssid === document.getElementById("smart_connect_ssid").value) {
                                    dwbInfo.sameSsidCount++;
                                }

                                if (dwbBand === key) {
                                    return document.getElementById(`${key}_ssid`).value;
                                }

                                return document.getElementById("smart_connect_ssid").value;
                            }

                            return document.getElementById("smart_connect_ssid").value;
                        }

                        validator.stringSSID(document.getElementById(`${key}_ssid`)) ? "" : validateErrorCount++;
                        if (dwbBand !== key && dwbInfo.ssid === document.getElementById(`${key}_ssid`).value) {
                            dwbInfo.sameSsidCount++;
                        }

                        return document.getElementById(`${key}_ssid`).value;
                    })();

                    // Hide SSID
                    postObject[`${key}_closed`] = (() => {
                        if (smartConnectEnable && joinSmartConnect) {
                            if (dwbMode === "1" && dwbBand === key) {
                                return document.getElementById(`${key}_hide_ssid_yes`).checked ? "1" : "0";
                            }

                            return document.getElementById("smart_connect_hide_ssid_yes").checked ? "1" : "0";
                        }

                        return document.getElementById(`${key}_hide_ssid_yes`).checked ? "1" : "0";
                    })();

                    // Channel Bandwidth
                    postObject[`${key}_bw`] = (() => {
                        return document.getElementById(`${key}_channel_bandwidth`).value;
                    })();

                    // Control Channel
                    if (isBRCMplatform) {
                        postObject[`${key}_chanspec`] = (() => {
                            let channelBandwidth = document.getElementById(`${key}_channel_bandwidth`).value;
                            let controlChannel = document.getElementById(`${key}_control_channel`).value;
                            let extensionChannel = document.getElementById(`${key}_extension_channel`).value;
                            if (controlChannel === "0") {
                                return "0";
                            }

                            if (key === "2g1") {
                                if (channelBandwidth === "0" || channelBandwidth === "2") {
                                    return `${controlChannel}${extensionChannel}`;
                                }
                            } else if (key === "6g1" || key === "6g2") {
                                if (channelBandwidth === "0") {
                                    if (
                                        controlChannel.indexOf("/40") !== -1 ||
                                        controlChannel.indexOf("/80") !== -1 ||
                                        controlChannel.indexOf("/160") !== -1
                                    ) {
                                        return controlChannel;
                                    } else {
                                        return `6g${controlChannel}/${extensionChannel}`;
                                    }
                                } else if (channelBandwidth === "6") {
                                    return `6g${controlChannel}/${extensionChannel}`;
                                }
                            }

                            return controlChannel;
                        })();
                    } else {
                        let channelBandwidth = document.getElementById(`${key}_channel_bandwidth`).value;
                        let controlChannel = document.getElementById(`${key}_control_channel`).value;
                        let extensionChannel = document.getElementById(`${key}_extension_channel`).value;
                        postObject[`${key}_channel`] = controlChannel;
                        if (channelBandwidth !== "0" && controlChannel !== "0") {
                            if (extensionChannel !== "0") {
                                postObject[`${key}_nctrlsb`] = extensionChannel;
                            }
                        }
                    }

                    // Authentication Method
                    postObject[`${key}_auth_mode_x`] = (() => {
                        if (smartConnectEnable && joinSmartConnect) {
                            let authMethodValue = document.getElementById("smart_connect_auth_method").value;
                            if (key === "6g1" || key === "6g2") {
                                if (authMethodValue === "open" || authMethodValue === "openowe") {
                                    authMethodValue = "owe";
                                } else if (
                                    authMethodValue === "psk" ||
                                    authMethodValue === "psk2" ||
                                    authMethodValue === "pskpsk2" ||
                                    authMethodValue === "psk2sae"
                                ) {
                                    authMethodValue = "sae";
                                } else if (
                                    authMethodValue === "wpa" ||
                                    authMethodValue === "wpa2" ||
                                    authMethodValue === "wpawpa2" ||
                                    authMethodValue === "wpa2wpa3"
                                ) {
                                    authMethodValue = "wpa3";
                                }
                            }

                            if (dwbMode === "1" && dwbBand === key) {
                                return document.getElementById(`${key}_auth_method`).value;
                            }

                            return authMethodValue;
                        }

                        return document.getElementById(`${key}_auth_method`).value;
                    })();

                    let authMethodValue = postObject[`${key}_auth_mode_x`];
                    if (
                        authMethodValue === "psk" ||
                        authMethodValue === "psk2" ||
                        authMethodValue === "sae" ||
                        authMethodValue === "pskpsk2" ||
                        authMethodValue === "psk2sae"
                    ) {
                        postObject[`${key}_wpa_psk`] = (() => {
                            if (smartConnectEnable && joinSmartConnect) {
                                let is_common_string = check_common_string(
                                    document.getElementById("smart_connect_wpa_key").value,
                                    "wpa_key"
                                );

                                if (is_common_string && Object.keys(commonStringError).length === 0) {
                                    commonStringError = {
                                        string: "<#JS_common_passwd#>",
                                        targetObject: document.getElementById("smart_connect_wpa_key"),
                                    };
                                }

                                if (isKRSku) {
                                    validator.psk_KR(document.getElementById("smart_connect_wpa_key")) ? "" : validateErrorCount++;
                                }

                                if (dwbMode === "1" && dwbBand === key) {
                                    return document.getElementById(`${key}_wpa_key`).value;
                                }

                                return document.getElementById("smart_connect_wpa_key").value;
                            }

                            let is_common_string = check_common_string(document.getElementById(`${key}_wpa_key`).value, "wpa_key");
                            if (is_common_string && Object.keys(commonStringError).length === 0) {
                                commonStringError = {
                                    string: "<#JS_common_passwd#>",
                                    targetObject: document.getElementById(`${key}_wpa_key`),
                                };
                            }

                            if (isKRSku) {
                                validator.psk_KR(document.getElementById(`${key}_wpa_key`)) ? "" : validateErrorCount++;
                            } else {
                                validator.psk(document.getElementById(`${key}_wpa_key`)) ? "" : validateErrorCount++;
                            }

                            return document.getElementById(`${key}_wpa_key`).value;
                        })();
                    }

                    // WPA Encryption
                    // Group Key
                    if (
                        authMethodValue === "owe" ||
                        authMethodValue === "openowe" ||
                        authMethodValue === "psk" ||
                        authMethodValue === "psk2" ||
                        authMethodValue === "sae" ||
                        authMethodValue === "pskpsk2" ||
                        authMethodValue === "psk2sae" ||
                        authMethodValue === "wpa" ||
                        authMethodValue === "wpa2" ||
                        authMethodValue === "wpa3" ||
                        authMethodValue === "suite-b" ||
                        authMethodValue === "wpawpa2" ||
                        authMethodValue === "wpa2wpa3"
                    ) {
                        postObject[`${key}_crypto`] = (() => {
                            if (smartConnectEnable && joinSmartConnect) {
                                let wpaEncryption = document.getElementById("smart_connect_wpa_encrypt").value;
                                if (key === "6g1" || key === "6g2") {
                                    wpaEncryption = "aes";
                                }

                                if (dwbMode === "1" && dwbBand === key) {
                                    return document.getElementById(`${key}_wpa_encrypt`).value;
                                }

                                return wpaEncryption;
                            }

                            return document.getElementById(`${key}_wpa_encrypt`).value;
                        })();

                        postObject[`${key}_wpa_gtk_rekey`] = (() => {
                            if (smartConnectEnable && joinSmartConnect) {
                                validator.range(document.getElementById("smart_connect_group_key"), 0, 2592000) ? "" : validateErrorCount++;
                                if (dwbMode === "1" && dwbBand === key) {
                                    return document.getElementById(`${key}_group_key`).value;
                                }

                                return document.getElementById("smart_connect_group_key").value;
                            }

                            validator.range(document.getElementById(`${key}_group_key`), 0, 2592000) ? "" : validateErrorCount++;
                            return document.getElementById(`${key}_group_key`).value;
                        })();
                    }

                    // Protect Management Frames
                    if (
                        authMethodValue === "owe" ||
                        authMethodValue === "openowe" ||
                        authMethodValue === "psk2" ||
                        authMethodValue === "sae" ||
                        authMethodValue === "pskpsk2" ||
                        authMethodValue === "psk2sae" ||
                        authMethodValue === "wpa2" ||
                        authMethodValue === "wpa3" ||
                        authMethodValue === "suite-b" ||
                        authMethodValue === "wpawpa2" ||
                        authMethodValue === "wpa2wpa3"
                    ) {
                        postObject[`${key}_mfp`] = (() => {
                            if (smartConnectEnable && joinSmartConnect) {
                                let mfp = document.getElementById("smart_connect_mfp").value;
                                if (key === "6g1" || key === "6g2") {
                                    mfp = "2";
                                }

                                if (dwbMode === "1" && dwbBand === key) {
                                    return document.getElementById(`${key}_mfp`).value;
                                }

                                return mfp;
                            }

                            return document.getElementById(`${key}_mfp`).value;
                        })();
                    }

                    // RADIUS IP address, port, key
                    if (
                        authMethodValue === "wpa" ||
                        authMethodValue === "wpa2" ||
                        authMethodValue === "wpa3" ||
                        authMethodValue === "suite-b" ||
                        authMethodValue === "wpawpa2" ||
                        authMethodValue === "wpa2wpa3" ||
                        authMethodValue === "radius"
                    ) {
                        postObject[`${key}_radius_ipaddr`] = (() => {
                            if (smartConnectEnable && joinSmartConnect) {
                                // if (authMethodValue === "radius" && (key === "6g1" || key === "6g2")) {
                                //     alert("6 GHz不支援RADIUS with 802.1x");
                                //     return false;
                                // }

                                if (dwbMode === "1" && dwbBand === key) {
                                    return document.getElementById(`${key}_radius_ip`).value;
                                }

                                return document.getElementById("smart_connect_radius_ip").value;
                            }

                            return document.getElementById(`${key}_radius_ip`).value;
                        })();
                        postObject[`${key}_radius_port`] = (() => {
                            if (smartConnectEnable && joinSmartConnect) {
                                // if (authMethodValue === "radius" && (key === "6g1" || key === "6g2")) {
                                //     alert("6 GHz不支援RADIUS with 802.1x");
                                //     return false;
                                // }

                                if (dwbMode === "1" && dwbBand === key) {
                                    return document.getElementById(`${key}_radius_port`).value;
                                }

                                return document.getElementById("smart_connect_radius_port").value;
                            }

                            return document.getElementById(`${key}_radius_port`).value;
                        })();
                        postObject[`${key}_radius_key`] = (() => {
                            if (smartConnectEnable && joinSmartConnect) {
                                // if (authMethodValue === "radius" && (key === "6g1" || key === "6g2")) {
                                //     alert("6 GHz不支援RADIUS with 802.1x");
                                //     return false;
                                // }

                                if (dwbMode === "1" && dwbBand === key) {
                                    return document.getElementById(`${key}_radius_key`).value;
                                }

                                return document.getElementById("smart_connect_radius_key").value;
                            }

                            return document.getElementById(`${key}_radius_key`).value;
                        })();
                    }

                    // WEP
                    let wepEncryptValue = postObject[`${key}_wep_encrypt`];
                    if (authMethodValue === "shared" || (authMethodValue === "open" && wepEncryptValue !== "0")) {
                        if (smartConnectEnable && joinSmartConnect) {
                            // if (key === "6g1" || key === "6g2") {
                            //     alert("6 GHz不支援Shared Key");
                            //     return false;
                            // }

                            postObject[`${key}_wep_x`] = document.getElementById("smart_connect_wep_encrypt").value;
                            postObject[`${key}_key`] = document.getElementById("smart_connect_key").value;
                            postObject[`${key}_key1`] = document.getElementById("smart_connect_key1").value;
                            postObject[`${key}_key2`] = document.getElementById("smart_connect_key2").value;
                            postObject[`${key}_key3`] = document.getElementById("smart_connect_key3").value;
                            postObject[`${key}_key4`] = document.getElementById("smart_connect_key4").value;
                            postObject[`${key}_phrase_x`] = document.getElementById("smart_connect_pass_phrase").value;
                            if (dwbMode === "1" && dwbBand !== key) {
                                postObject[`${key}_wep_x`] = document.getElementById(`${key}_wep_encrypt`).value;
                                postObject[`${key}_key`] = document.getElementById(`${key}_key`).value;
                                postObject[`${key}_key1`] = document.getElementById(`${key}_key1`).value;
                                postObject[`${key}_key2`] = document.getElementById(`${key}_key2`).value;
                                postObject[`${key}_key3`] = document.getElementById(`${key}_key3`).value;
                                postObject[`${key}_key4`] = document.getElementById(`${key}_key4`).value;
                                postObject[`${key}_phrase_x`] = document.getElementById(`${key}_pass_phrase`).value;
                            }
                        } else {
                            postObject[`${key}_wep_x`] = document.getElementById(`${key}_wep_encrypt`).value;
                            postObject[`${key}_key`] = document.getElementById(`${key}_key`).value;
                            postObject[`${key}_key1`] = document.getElementById(`${key}_key1`).value;
                            postObject[`${key}_key2`] = document.getElementById(`${key}_key2`).value;
                            postObject[`${key}_key3`] = document.getElementById(`${key}_key3`).value;
                            postObject[`${key}_key4`] = document.getElementById(`${key}_key4`).value;
                            postObject[`${key}_phrase_x`] = document.getElementById(`${key}_pass_phrase`).value;
                        }
                    }

                    // ACS CH13
                    if (key === "2g1") {
                        let acsCh13Element = document.getElementById(`${key}_acs_ch13`);
                        let acsCh13FieldElement = document.getElementById(`${key}_acs_ch13_field`);
                        if (acsCh13Element && acsCh13FieldElement.style.display === "") {
                            postObject[`acs_ch13`] = acsCh13Element.checked ? "1" : "0";
                        }
                    }

                    // Enable 160 MHz, ACS DFS, ACS U-NII-4
                    if (key === "5g1" || key === "5g2") {
                        let enable160MHzElement = document.getElementById(`${key}_bw160_enable`);
                        if (enable160MHzElement && enable160MHzElement.style.display === "") {
                            postObject[`${key}_bw_160`] = enable160MHzElement.checked ? "1" : "0";
                        }

                        let acsDfsElement = document.getElementById(`${key}_acs_dfs`);
                        let acsDfsFieldElement = document.getElementById(`${key}_acs_dfs_field`);
                        if (acsDfsElement && acsDfsFieldElement.style.display === "") {
                            if (key === "5g1") {
                                postObject[`acs_dfs`] = acsDfsElement.checked ? "1" : "0";
                            } else if (key === "5g2") {
                                postObject[`acs_band3`] = acsDfsElement.checked ? "1" : "0";
                            }
                        }

                        let acsUnii4Element = document.getElementById(`${key}_unii4`);
                        let acsUnii4FieldElement = document.getElementById(`${key}_unii4_field`);
                        if (acsUnii4Element && acsUnii4FieldElement.style.display === "") {
                            postObject[`acs_unii4`] = acsUnii4Element.checked ? "1" : "0";
                        }
                    }

                    // PSC channels
                    if (key === "6g1" || key === "6g2") {
                        let psc6gElement = document.getElementById(`${key}_psc6g`);
                        if (psc6gElement && psc6gElement.style.display === "") {
                            postObject[`psc6g`] = psc6gElement.checked ? "1" : "0";
                        }
                    }
                }

                if (dwbMode === "1") {
                    let target = document.getElementById("mesh_backhaul_field");
                    if (target && target.style.display !== "none") {
                        postObject[`fh_ap_enabled`] = document.getElementById("fh_ap_enabled").value;
                    }
                }

                postObject.action_mode = "apply";
                postObject.rc_service = "restart_wireless";

                // default take BRCM platform
                let restartTime = 10;
                if (isMTKplatform) {
                    restartTime = 25;
                } else if (isQCAplatform) {
                    restartTime = 30;
                }

                if (dwbInfo.sameSsidCount !== 0) {
                    let { targetObject, sameSsidString } = dwbInfo;
                    alert(sameSsidString);
                    targetObject.focus();
                    targetObject.select();
                    return false;
                }

                if (validateErrorCount !== 0) {
                    return false;
                }

                if (Object.keys(commonStringError).length !== 0) {
                    let { string, targetObject } = commonStringError;
                    if (!confirm(string)) {
                        targetObject.focus();
                        targetObject.select();
                        return false;
                    }
                }

                httpApi.nvramSet(postObject, function () {
                    showLoading(restartTime);
                    setTimeout(function () {
                        location.reload();
                    }, restartTime * 1000);
                });
            }

            function mloHint() {
                let confirm_flag = 1;

                if (confirm_flag == 1) {
                    if ($(".confirm_block").length > 0) {
                        $(".confirm_block").remove();
                    }
                    if (window.scrollTo) window.scrollTo(0, 0);
                    htmlbodyforIE = document.getElementsByTagName("html");
                    htmlbodyforIE[0].style.overflow = "hidden";

                    $("#Loading").css("visibility", "visible");
                    $("#loadingBlock").css("visibility", "hidden");

                    confirm_asus({
                        title: "MLO Hint",
                        contentA: `<b><#WiFi7_mlo_adjust_hint#></b>`,
                        contentC: "",
                        left_button: "<#CTL_Cancel#>",
                        left_button_callback: function () {
                            location.href = location.href;
                        },
                        left_button_args: {},
                        right_button: "<#btn_go#>",
                        right_button_callback: function () {
                            top.location.href = "/MLO.asp";
                        },
                        right_button_args: {},
                        iframe: "",
                        margin: "0px",
                        note_display_flag: 0,
                    });

                    $(".confirm_block").css("zIndex", 10001);
                }
            }
        </script>
    </head>

    <body class="bg">
        <div id="TopBanner"></div>
        <div id="Loading" class="popup_bg"></div>
        <div id="hiddenMask" class="popup_bg">
            <table cellpadding="4" cellspacing="0" id="dr_sweet_advise" class="dr_sweet_advise" align="center">
                <tr>
                    <td>
                        <div class="drword" id="drword">
                            <#Main_alert_proceeding_desc4#> <#Main_alert_proceeding_desc1#>...
                            <br />
                            <div id="disconnect_hint" style="display: none"><#Main_alert_proceeding_desc2#></div>
                            <br />
                        </div>
                        <div id="wireless_client_detect" style="margin-left: 10px; position: absolute; display: none; width: 400px">
                            <img src="images/loading.gif" />
                            <div style="margin: -55px 0 0 75px"><#QKSet_Internet_Setup_fail_method1#></div>
                        </div>
                        <div class="drImg"><img src="images/alertImg.png" /></div>
                        <div style="height: 100px"></div>
                    </td>
                </tr>
            </table>
            <!--[if lte IE 6.5]><iframe class="hackiframe"></iframe><![endif]-->
        </div>
        <script></script>
        <iframe name="hidden_frame" id="hidden_frame" width="0" height="0" frameborder="0"></iframe>
        <table class="content" align="center" cellpadding="0" cellspacing="0" style="margin-top: -15px">
            <tr>
                <td width="17">&nbsp;</td>

                <!--=====Beginning of Main Menu=====-->
                <td valign="top" width="202">
                    <div id="mainMenu"></div>
                    <div id="subMenu"></div>
                </td>

                <td valign="top">
                    <div id="tabMenu" class="submenuBlock"></div>

                    <!--===================================Beginning of Main Content===========================================-->
                    <table width="98%" border="0" align="left" cellpadding="0" cellspacing="0">
                        <tr>
                            <td align="left" valign="top">
                                <table width="760px" border="0" cellpadding="4" cellspacing="0" class="FormTitle" id="FormTitle">
                                    <tbody>
                                        <tr>
                                            <td bgcolor="#4D595D" valign="top">
                                                <div>&nbsp;</div>
                                                <div class="formfonttitle"><#menu5_1#> - <#menu5_1_1#></div>
                                                <div class="splitLine splitline-padding"></div>
                                                <div class="formfontdesc"><#adv_wl_desc#></div>

                                                <!-- Smart Connect Switch Button -->
                                                <table
                                                    width="100%"
                                                    border="1"
                                                    align="center"
                                                    cellpadding="4"
                                                    cellspacing="0"
                                                    id="WLgeneral"
                                                    class="FormTable"
                                                >
                                                    <tr id="smartcon_enable_field">
                                                        <th width="30%">
                                                            <a class="hintstyle" href="javascript:void(0);" onClick="openHint(0,27);"
                                                                ><#smart_connect_enable#></a
                                                            >
                                                        </th>
                                                        <td>
                                                            <div id="smartcon_enable_block" style="display: none">
                                                                <span id="smart_connect_enable_word">&nbsp;&nbsp;</span>
                                                                <input
                                                                    type="button"
                                                                    name="enableSmartConbtn"
                                                                    id="enableSmartConbtn"
                                                                    value=""
                                                                    class="button_gen"
                                                                    onClick=""
                                                                />
                                                                <br />
                                                            </div>

                                                            <div id="radio_smartcon_enable" class="left radio-smartcon-enable"></div>
                                                            <div
                                                                id="smartcon_rule_link"
                                                                class="smart-connect-rule-link"
                                                                style="display: none"
                                                            >
                                                                <a href="Advanced_Smart_Connect.asp" class="a-hint-text"
                                                                    ><#smart_connect_rule#></a
                                                                >
                                                            </div>

                                                            <script type="text/javascript">
                                                                $("#radio_smartcon_enable").iphoneSwitch(
                                                                    system.smartConnect.smartConnectEnable,
                                                                    function () {
                                                                        systemManipulable.smartConnect.smartConnectEnable = true;
                                                                        let { wlBandSeq, smartConnect } = systemManipulable;
                                                                        let { version } = smartConnect;

                                                                        if (version === "v2") {
                                                                            ssidHandler("smart_connect");
                                                                            for (let preifx of Object.keys(wlBandSeq)) {
                                                                                ssidHandler(preifx);
                                                                                wirelessBackhaulHandler(preifx);
                                                                            }
                                                                        } else {
                                                                            let smartConnectX =
                                                                                document.getElementById("smart_connect_x").value;

                                                                            ssidHandler("smart_connect");
                                                                            smartConnectChange(smartConnectX);
                                                                        }

                                                                        document.getElementById("band_separate").style.display = "";
                                                                    },
                                                                    function () {
                                                                        systemManipulable.smartConnect.smartConnectEnable = false;
                                                                        let { wlBandSeq, smartConnect, mloEnabled } = systemManipulable;
                                                                        let { version } = smartConnect;
                                                                        if (mloEnabled) {
                                                                            mloHint();
                                                                        }

                                                                        if (version === "v2") {
                                                                            ssidHandler("smart_connect");
                                                                            for (let preifx of Object.keys(wlBandSeq)) {
                                                                                ssidHandler(preifx);
                                                                                wirelessBackhaulHandler(preifx);
                                                                            }
                                                                        } else {
                                                                            for (let preifx of Object.keys(wlBandSeq)) {
                                                                                systemManipulable.wlBandSeq[
                                                                                    preifx
                                                                                ].joinSmartConnect = false;
                                                                                ssidHandler(preifx);
                                                                                wirelessBackhaulHandler(preifx);
                                                                            }
                                                                        }

                                                                        document.getElementById("band_separate").style.display = "none";
                                                                    }
                                                                );
                                                            </script>
                                                        </td>
                                                    </tr>
                                                </table>

                                                <table
                                                    id="band_separate"
                                                    width="100%"
                                                    border="1"
                                                    align="center"
                                                    cellpadding="4"
                                                    cellspacing="0"
                                                    bordercolor="#6b8fa3"
                                                    class="FormTable"
                                                ></table>
                                                <table
                                                    id="eachBandField"
                                                    width="100%"
                                                    border="1"
                                                    align="center"
                                                    cellpadding="4"
                                                    cellspacing="0"
                                                    bordercolor="#6b8fa3"
                                                    class="FormTable"
                                                ></table>

                                                <div class="apply_gen">
                                                    <input
                                                        type="button"
                                                        id="applyButton"
                                                        class="button_gen"
                                                        value="<#CTL_apply#>"
                                                        onclick="apply();"
                                                    />
                                                </div>
                                            </td>
                                        </tr>
                                    </tbody>
                                </table>
                            </td>
                        </tr>
                    </table>
                    <!--===================================Ending of Main Content===========================================-->
                </td>
                <td width="10" align="center" valign="top"></td>
            </tr>
        </table>
        <div id="footer"></div>
    </body>
</html>
