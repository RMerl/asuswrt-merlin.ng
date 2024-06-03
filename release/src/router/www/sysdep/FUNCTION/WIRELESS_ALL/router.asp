<!DOCTYPE html>
<html>
    <head>
        <meta charset="utf-8" />
        <meta http-equiv="X-UA-Compatible" content="IE=11;IE=Edge" />
        <meta http-equiv="Pragma" CONTENT="no-cache" />
        <meta http-equiv="Expires" CONTENT="-1" />
        <meta name="viewport" content="width=device-width, initial-scale=1.0" />
        <link rel="shortcut icon" href="images/favicon.png" />
        <title><#menu1#> - <#menu5_1#></title>
        <link rel="stylesheet" href="../NM_style.css" type="text/css" />
        <link rel="stylesheet" href="../form_style.css" type="text/css" />
        <link rel="stylesheet" href="../css/networkMap.css" type="text/css" />
        <link rel="stylesheet" href="../pwdmeter.css" />
        <link rel="stylesheet" href="../css/confirm_block.css" />

        <script src="../js/jquery.js"></script>
        <script src="../js/httpApi.js" t></script>
        <script src="../state.js"></script>

        <!-- <script src="../js/device.js"></script> -->
        <script src="/validator.js"></script>
        <script src="/help.js"></script>

        <script src="/js/asus.js"></script>
        <script src="../switcherplugin/jquery.iphone-switch.js"></script>
        <script src="/js/confirm_block.js"></script>
    </head>
    <body>
        <style>
            .pwdmeter_container {
                margin-top: 5px;
                margin-left: 1px;
                display: none;
            }

            .medium_pwdmeter {
                width: 241px;
            }

            .customized_scoreBar {
                background-size: 300%;
            }
        </style>
        <script>
            (function () {
                var dynamic_include_js = function (_src) {
                    $("<script>").attr("type", "text/javascript").attr("src", _src).appendTo("head");
                };
                if (parent.amesh_support && (parent.isSwMode("rt") || parent.isSwMode("ap")))
                    dynamic_include_js("/require/modules/amesh.js");
                if (parent.lantiq_support) dynamic_include_js("/calendar/jquery-ui.js");
            })();

            // START
            let systemManipulable = objectDeepCopy(system);
            document.addEventListener("DOMContentLoaded", function () {
                let { assassinMode, lightEffectSupoort, currentOPMode } = systemManipulable;
                // Media Bridge site survey, hide the others fields
                if (currentOPMode.id === "MB") {
                    document.querySelector("#media_bridge_site_survey_field").style.display = "";
                    document.querySelector("#assassin_mode").style.display = "none";
                    document.querySelector("#light_effect_tab").style.display = "none";
                    document.querySelector("#smart_connect_field").style.display = "none";
                    document.querySelector("#wl_settings_field").style.display = "none";
                    document.querySelector("#apply_button").style.display = "none";
                    return true;
                }

                let { assassinModeSupport } = assassinMode;
                // Assassin mode
                if (assassinModeSupport) {
                    document.querySelector("#assassin_mode").style.display = "";
                }

                // AURA RGB
                if (lightEffectSupoort) {
                    document.querySelector("#light_effect_tab").style.display = "";
                }

                // Smart Connect
                generateSmartConnect();

                // WIRELESS
                generateWireless();
            });

            function generateSmartConnect() {
                let { smartConnect, wlBandSeq } = systemManipulable;
                let { support, version, v1Type, smartConnectEnable } = smartConnect;
                if (!support) {
                    return true;
                }

                let code = "";
                let radioBandSnippet = "";
                if (version === "v2") {
                    let displayFlag = smartConnectEnable ? "" : "none";

                    for (let { name, prefixNvram, joinSmartConnect } of Object.values(wlBandSeq)) {
                        radioBandSnippet += `<input id="smart_connect_check_${prefixNvram}" type="checkbox" onchange="smartConnectRadioChange(this.checked,'${prefixNvram}')" ${
                            joinSmartConnect ? "checked" : ""
                        } />${name}`;
                    }

                    code += `
                        <select class="input_option" id="smartConnectSwitch" onchange="enableSmartConnect(this.value)">
                            <option value="1" ${smartConnectEnable ? "selected" : ""}><#CTL_Enabled#></option>
                            <option value="0" ${!smartConnectEnable ? "selected" : ""}><#CTL_close#></option>
                        </select>
                        <div id="smart_connect_mode_field" style="display:${displayFlag}">${radioBandSnippet}</div>
                    `;
                } else {
                    // v1
                    let band6gSupport = !!wlBandSeq["6g1"];
                    let band5g2Support = !!wlBandSeq["5g2"];
                    radioBandSnippet += `<option value="0" ${v1Type === "0" ? "selected" : ""}><#wl_securitylevel_0#></option>`;
                    radioBandSnippet += `<option value="1" ${v1Type === "1" ? "selected" : ""}>${
                        band6gSupport || band5g2Support ? "<#smart_connect_tri#>" : "<#smart_connect_dual#>"
                    }</option>`;

                    // due to SMART CONNECT v1 does not have 2556 model
                    if (band6gSupport) {
                        radioBandSnippet += `<option value="3" ${v1Type === "3" ? "selected" : ""}>2.4 GHz/5 GHz Smart Connect</option>`;
                    } else if (band5g2Support) {
                        radioBandSnippet += `<option value="2" ${v1Type === "2" ? "selected" : ""}>5 GHz Smart Connect</option>`;
                    }

                    code += `
                        <select id="smart_connect_x" class="input_option" onchange="enableSmartConnect(this.value)">
                            ${radioBandSnippet}
                        </select>
                    `;
                }

                document.querySelector("#smart_connect_field").innerHTML = `
                    <div class="info-block">
                        <div class="info-title"><#smart_connect#></div>
                        ${code}
                    </div>
                `;
            }

            function generateWireless() {
                let { smartConnect, wlBandSeq, aMesh } = systemManipulable;
                let { support, version, v1Type, smartConnectEnable, smartConnectReferenceIndex } = smartConnect;
                let { dwbMode, dwbBand } = aMesh;
                let code = "";
                if (support && smartConnectEnable) {
                    let { authMethodValue, wlModeValue } = wlBandSeq[smartConnectReferenceIndex];
                    let smartConnectSnippet = "";
                    smartConnectSnippet += generateSSID("smart_connect");
                    smartConnectSnippet += generateAuthenticationMethod("smart_connect");
                    smartConnectSnippet += generateWpaEncryption("smart_connect");
                    smartConnectSnippet += generateWpaKey("smart_connect");
                    if (wlModeValue === "2") {
                        smartConnectSnippet += generateWepEncryption("smart_connect");
                        smartConnectSnippet += generateWepKey("smart_connect");
                    }

                    code += `
                        <div class="unit-block">
                            <div class="division-block">Smart Connect</div>
                            ${smartConnectSnippet}
                        </div>
                    `;
                }

                for (let wlObject of Object.values(wlBandSeq)) {
                    let { prefixNvram, name, joinSmartConnect, wlModeValue } = wlObject;
                    let backhaulString = "";
                    let displayFlag = (() => {
                        if (dwbMode === "1" && dwbBand === prefixNvram) {
                            backhaulString = "(Backhaul)";
                            return "";
                        }

                        return smartConnectEnable && joinSmartConnect ? "none" : "";
                    })();

                    let wlSnippet = "";
                    wlSnippet += generateSSID(prefixNvram);
                    wlSnippet += generateAuthenticationMethod(prefixNvram);
                    wlSnippet += generateWpaEncryption(prefixNvram);
                    wlSnippet += generateWpaKey(prefixNvram);
                    if (wlModeValue === "2") {
                        wlSnippet += generateWepEncryption(prefixNvram);
                        wlSnippet += generateWepKey(prefixNvram);
                    }

                    code += `
                        <div class="unit-block" style="display:${displayFlag}">
                            <div class="division-block">${name} ${backhaulString}</div>
                            ${wlSnippet}
                        </div>
                    `;
                }

                document.querySelector("#wl_settings_field").innerHTML = code;
            }

            function generateSSID(prefix) {
                let { wlBandSeq, smartConnect } = systemManipulable;
                let { smartConnectReferenceIndex } = smartConnect;
                let prefixNvram = prefix === "smart_connect" ? smartConnectReferenceIndex : prefix;
                let { ssidValue } = wlBandSeq[prefixNvram];
                return `
                    <div class="info-block">
                        <div class="info-title"><#QIS_finish_wireless_item1#></div>
                        <div>
                            <input id="${prefix}_ssid" type="text" class="input-size-25" maxlength="32" value="${ssidValue}" onkeypress="validator.isString(this, event)" autocomplete="off" autocapitalize="off" />
                        </div>
                    </div>
                `;
            }

            function generateAuthenticationMethod(prefix) {
                let {
                    wlBandSeq,
                    newWiFiCertSupport,
                    wifiLogoSupport,
                    isKRSku,
                    currentOPMode,
                    aMeshSupport,
                    aMeshRouterSupport,
                    smartConnect,
                    aMesh,
                } = systemManipulable;
                let { smartConnectEnable, v2Band, smartConnectReferenceIndex } = smartConnect;
                let prefixNvram = prefix === "smart_connect" ? smartConnectReferenceIndex : prefix;
                let { wlModeValue, authMethod, authMethodValue, joinSmartConnect } = wlBandSeq[prefixNvram];

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
                    <div class="info-block">
                        <div class="info-title"><#WLANConfig11b_AuthenticationMethod_itemname#></div>
                        <select id="${prefix}_auth_method" class="input_option" onChange="authenticationMethodChange(this.value, '${prefix}')">${authMethodSnippet}</select>
                    </div>
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
                    <div id="${prefix}_wpa_encrypt_field" class="info-block" style="display:${displayFlag}">
                        <div class="info-title"><#WLANConfig11b_WPAType_itemname#></div>
                        <select id="${prefix}_wpa_encrypt" class="input_option" onchange="">${wpaEncryptSnippet}</select>
                    </div>
                `;
            }

            function generateWpaKey(prefix) {
                let { wlBandSeq, smartConnect } = systemManipulable;
                let { smartConnectEnable, smartConnectReferenceIndex } = smartConnect;
                let prefixNvram = prefix === "smart_connect" ? smartConnectReferenceIndex : prefix;
                let { authMethodValue, wpaKeyValue } = wlBandSeq[prefixNvram];
                let displayFlag = (() => {
                    if (
                        authMethodValue === "psk" ||
                        authMethodValue === "psk2" ||
                        authMethodValue === "sae" ||
                        authMethodValue === "pskpsk2" ||
                        authMethodValue === "psk2sae"
                    ) {
                        // let { dwbMode, dwbBand } = aMesh;
                        // if (dwbMode === "1" && dwbBand === prefix) {
                        //     return "";
                        // }

                        if (prefix === "smart_connect") {
                            return smartConnectEnable ? "" : "none";
                        }

                        // if (smartConnectEnable && joinSmartConnect) {
                        //     return "none";
                        // }

                        return "";
                    }

                    return "none";
                })();

                return `
                    <div id="${prefix}_wpa_key_field" class="info-block" style="display:${displayFlag}">
                        <div class="info-title"><#WPA-PSKKey#></div>
                        <input id="${prefix}_wpa_key" type="password" class="input-size-25" value="${wpaKeyValue}" onfocus="plainPasswordSwitch(this, 'focus')" onblur="plainPasswordSwitch(this, 'blur')" />
                    </div>
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
                    <div class="info-block" id='${prefix}_wep_encrypt_field'  style="display:${displayFlag}">
                        <div class="info-title"><#WLANConfig11b_WEPType_itemname#></div>
                        <select id="${prefix}_wep_encrypt" class="input_option" onChange="wepEncryptionChange(this.value, '${prefix}')">${wepEncryptSnippet}</select>
                    </div>
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
                    <div class="info-block" id="${prefix}_wep_key_index_field"  style="display:${displayFlag}">
                        <div class="info-title"><#WLANConfig11b_WEPDefaultKey_itemname#></div>
                        <select id="${prefix}_key" class="input_option" onChange="">
                            <option value="1" ${wepKeyIndexValue === "1" ? "selected" : ""}>1</option>
                            <option value="2" ${wepKeyIndexValue === "2" ? "selected" : ""}>2</option>
                            <option value="3" ${wepKeyIndexValue === "3" ? "selected" : ""}>3</option>
                            <option value="4" ${wepKeyIndexValue === "4" ? "selected" : ""}>4</option>
                        </select>
                    </div>

                    <div id="${prefix}_wep_key1_field" class="info-block" style="display:${displayFlag}">
                        <div class="info-title"><#WLANConfig11b_WEPKey1_itemname#></div>
                        <input id="${prefix}_key1" type="password" class="input-size-25" value="${wepKey1Value}" onfocus="plainPasswordSwitch(this, 'focus')" onblur="plainPasswordSwitch(this, 'blur')" autocorrect="off" autocapitalize="off" />
                    </div>

                    <div id="${prefix}_wep_key2_field" class="info-block" style="display:${displayFlag}">
                        <div class="info-title"><#WLANConfig11b_WEPKey2_itemname#></div>
                        <input id="${prefix}_key2" type="password" class="input-size-25" value="${wepKey2Value}" onfocus="plainPasswordSwitch(this, 'focus')" onblur="plainPasswordSwitch(this, 'blur')" autocorrect="off" autocapitalize="off" />
                    </div>

                    <div id="${prefix}_wep_key3_field" class="info-block" style="display:${displayFlag}">
                        <div class="info-title"><#WLANConfig11b_WEPKey3_itemname#></div>
                        <input id="${prefix}_key3" type="password" class="input-size-25" value="${wepKey3Value}" onfocus="plainPasswordSwitch(this, 'focus')" onblur="plainPasswordSwitch(this, 'blur')" autocorrect="off" autocapitalize="off" />
                    </div>

                    <div id="${prefix}_wep_key4_field" class="info-block" style="display:${displayFlag}">
                        <div class="info-title"><#WLANConfig11b_WEPKey4_itemname#></div>
                        <input id="${prefix}_key4" type="password" class="input-size-25" value="${wepKey4Value}" onfocus="plainPasswordSwitch(this, 'focus')" onblur="plainPasswordSwitch(this, 'blur')" autocorrect="off" autocapitalize="off" />
                    </div>

                    <div id="${prefix}_pass_phrase_field" class="info-block" style="display:${displayFlag}">
                        <div class="info-title"><#WLANConfig11b_x_Phrase_itemname#></div>
                        <input id="${prefix}_pass_phrase" type="password" class="input-size-25" value="${wepPassPhraseValue}" onfocus="plainPasswordSwitch(this, 'focus')" onblur="plainPasswordSwitch(this, 'blur')" autocorrect="off" autocapitalize="off" />
                    </div>
                `;
            }

            function smartConnectRadioChange(check, prefix) {
                if (system.mloEnabled) mloHint();

                systemManipulable.wlBandSeq[prefix].joinSmartConnect = check;
                let { wlBandSeq } = systemManipulable;
                let { joinSmartConnect } = wlBandSeq[prefix];
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

                generateWireless();
            }

            function enableSmartConnect(value) {
                if (system.mloEnabled) mloHint();

                let { smartConnect, wlBandSeq } = systemManipulable;
                let { support, version, v1Type, smartConnectEnable } = smartConnect;
                systemManipulable.smartConnect.smartConnectEnable = value !== "0";
                if (version === "v2") {
                    document.querySelector("#smart_connect_mode_field").style.display = `${value !== "0" ? "" : "none"}`;
                } else if (version === "v1") {
                    systemManipulable.smartConnect.v1Type = value;
                    for (let wlIfIndex of Object.keys(wlBandSeq)) {
                        if (value === "1") {
                            // all bands smart connect
                            systemManipulable.wlBandSeq[wlIfIndex].joinSmartConnect = true;
                            systemManipulable.smartConnect.smartConnectReferenceIndex = "2g1";
                        } else if (value === "2") {
                            // 5 GHZ smart connect
                            let joined = wlIfIndex === "5g1" || wlIfIndex === "5g2";
                            systemManipulable.wlBandSeq[wlIfIndex].joinSmartConnect = joined;
                            systemManipulable.smartConnect.smartConnectReferenceIndex = "5g1";
                        } else if (value === "3") {
                            // 2.4 GHz & 5 GHz smart connect without 6 GHz
                            let joined = wlIfIndex === "2g1" || wlIfIndex === "5g1" || wlIfIndex === "5g2";
                            systemManipulable.wlBandSeq[wlIfIndex].joinSmartConnect = joined;
                            systemManipulable.smartConnect.smartConnectReferenceIndex = "2g1";
                        } else if (value === "0") {
                            systemManipulable.wlBandSeq[wlIfIndex].joinSmartConnect = false;
                        }
                    }
                }

                generateWireless();
            }

            function authenticationMethodChange(authMethodValue, prefix) {
                let { smartConnect, mloEnabled } = systemManipulable;
                let { smartConnectEnable, smartConnectReferenceIndex } = smartConnect;
                let prefixNvram = prefix === "smart_connect" ? smartConnectReferenceIndex : prefix;
                let { beSupport, wifi7ModeEnabled, authMethodValue: authMethodValueOri } = systemManipulable.wlBandSeq[prefixNvram];
                if (systemManipulable.wlBandSeq[prefixNvram]) {
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
                                        return `0`;
                                    })(),
                                    note_display_flag: 0,
                                });
                            }
                        }
                    }
                }

                generateWireless();
            }

            function apply() {
                let { isMTKplatform, isQCAplatform, smartConnect, wlBandSeq, aMesh, isKRSku } = systemManipulable;
                let { smartConnectEnable, radioSeqArray, version } = smartConnect;
                let { dwbBand, dwbMode } = aMesh;
                let postObject = {};
                let commonStringError = {};
                let validateErrorCount = 0;
                let restartTime = 10;
                if (isMTKplatform) {
                    restartTime = 25;
                } else if (isQCAplatform) {
                    restartTime = 30;
                }

                // var rc_time = httpApi.hookGet("get_default_reboot_time", true);
                // if (!rc_flag) {
                //     rc_flag = "restart_wireless";
                //     rc_time = 10;
                // }
                // handle data wants to post
                postObject = {
                    action_mode: "apply",
                    rc_service: "restart_wireless",
                };

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
                    let { joinSmartConnect, wlModeValue } = value;

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

                    // WPA Key
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
                        authMethodValue === "wpa2pwa3"
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
                        authMethodValue === "wpa2pwa3"
                    ) {
                        let { mfpValue } = value;
                        postObject[`${key}_mfp`] = (() => {
                            let mfp = "0";
                            if (
                                authMethodValue === "owe" ||
                                authMethodValue === "openowe" ||
                                authMethodValue === "sae" ||
                                authMethodValue === "wpa3" ||
                                authMethodValue === "suite-b"
                            ) {
                                mfp = "2";
                            } else if (authMethodValue === "psk2sae" || authMethodValue === "wpa2pwa3") {
                                mfp = "1";
                            } else if (authMethodValue === "pskpsk2" || authMethodValue === "wpawpa2") {
                                mfp = "0";
                            }

                            return mfp;
                        })();
                    }

                    // WEP
                    if (wlModeValue === "2") {
                        let wepEncryptValue = document.getElementById(`${key}_wep_encrypt`).value;
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
                    }
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
                    parent.showLoading(restartTime);
                    setTimeout(function () {
                        location.reload();
                    }, restartTime * 1000);
                });
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
                if (systemManipulable.wlBandSeq[prefixNvram]) {
                    systemManipulable.wlBandSeq[prefixNvram].wepEncryptValue = wepEncryptValue;
                }

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

            function switchTab(id) {
                let pageObject = {
                    wireless_tab: "router.asp",
                    status_tab: "router_status.asp",
                    light_effect_tab: "router_light_effect.asp",
                };
                let path = window.location.pathname.split("/").pop();
                let targetPath = pageObject[id];
                if (targetPath === path) {
                    return false;
                }

                location.href = targetPath;
            }

            function gotoSiteSurvey() {
                if (sw_mode == 2) parent.location.href = "/QIS_wizard.htm?flag=sitesurvey_rep&band=" + wl_unit;
                else parent.location.href = "/QIS_wizard.htm?flag=sitesurvey_mb";
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
        <div class="main-block">
            <div class="display-flex flex-a-center">
                <div id="wireless_tab" class="tab-block tab-click" onclick="switchTab(this.id)"><#menu5_1#></div>
                <div id="status_tab" class="tab-block" onclick="switchTab(this.id)"><#Status_Str#></div>
                <div id="light_effect_tab" class="tab-block" style="display: none" onclick="switchTab(this.id)">Aura RGB</div>
            </div>

            <!-- Media Bridge site survey -->
            <div id="media_bridge_site_survey_field" class="unit-block" style="display: none">
                <div class="info-title"><#APSurvey_action_search_again_hint2#></div>
                <div class="button-right">
                    <input type="button" class="button_gen" value="<#QIS_rescan#>" onclick="gotoSiteSurvey();" />
                </div>
            </div>

            <!-- ASSASSIN MODE -->
            <div id="assassin_mode" class="unit-block" style="display: none">
                <div class="display-flex flex-a-center flex-j-spaceB">
                    <div>刺客模式</div>
                    <div>
                        <div class="left" id="assassin_mode_enable"></div>
                        <div class="clear"></div>
                        <script>
                            $("#assassin_mode_enable").iphoneSwitch(
                                systemManipulable.assassinMode.assassinModeEnable,
                                function () {
                                    systemManipulable.locationCode = "XX";

                                    // apply("reboot");
                                },
                                function () {
                                    systemManipulable.locationCode = "CN";

                                    // apply("reboot");
                                }
                            );
                        </script>
                    </div>
                </div>
            </div>

            <!-- SMART CONNECT -->
            <div id="smart_connect_field" class="unit-block">
                <!-- v2 -->
                <div class="info-block">
                    <div class="info-title"><#smart_connect#></div>
                    <select class="input_option" id="smartConnectSwitch" onchange="enableSmartConnect(this.value)">
                        <option value="0"><#CTL_close#></option>
                        <option value="1"><#CTL_Enabled#></option>
                    </select>

                    <div id="smart_connect_mode_field">
                        <input id="smart_connect_check_0" type="checkbox" onchange="updateSmartConnect(0, this.checked)" />2.4 GHz
                        <input id="smart_connect_check_1" type="checkbox" onchange="updateSmartConnect(0, this.checked)" />5 GHz
                        <input id="smart_connect_check_2" type="checkbox" onchange="updateSmartConnect(0, this.checked)" />6 GHz-1
                        <input id="smart_connect_check_3" type="checkbox" onchange="updateSmartConnect(0, this.checked)" />6 GHz-2
                    </div>
                </div>

                <!-- v1 -->
                <div class="info-block">
                    <div class="info-title"><#smart_connect#></div>
                    <div>
                        <select id="smart_connect_x" class="input_option" onchange="updateVariable(this.id, value)">
                            <option value="0"><#wl_securitylevel_0#></option>
                            <option value="1"><#smart_connect_dual#></option>
                            <option value="2">5 GHz Smart Connect</option>
                            <option value="3">2.4 GHz/5 GHz Smart Connect</option>
                        </select>
                    </div>
                </div>
            </div>

            <!-- WiFi Setting field -->
            <div id="wl_settings_field">
                <div id="wl_ready" class="wl-ready" style="display: none">Wireless is setting...</div>
                <div class="unit-block">
                    <div class="division-block">2.4 GHz</div>
                    <!-- <div class="dwb_hint">6 GHz <#AiMesh_backhaul_band_5GHz-2_desc1#></div>
                    <div class="dwb_hint">5 GHz-2 <#AiMesh_backhaul_band_5GHz-2_desc1#></div>
                    <div class="dwb_hint"><#AiMesh_backhaul_band_5GHz-2_desc2#></div> -->

                    <!-- SSID -->
                    <div class="info-block">
                        <div class="info-title"><#QIS_finish_wireless_item1#></div>
                        <div>
                            <input type="text" class="input-size-25" oninput="" maxlength="33" autocomplete="off" autocapitalize="off" />
                        </div>
                    </div>

                    <!-- Authentication Method -->
                    <div class="info-block">
                        <div class="info-title"><#WLANConfig11b_AuthenticationMethod_itemname#></div>
                        <div><select id="" class="input_option" onchange=""></select></div>
                        <!-- <span id="" style="color: #fc0"><#WLANConfig11b_AuthenticationMethod_EOT_desc#></span> -->
                    </div>
                    <!-- <div id="" class="wpa3_hint" style="">
                        <span><#AiMesh_confirm_msg10#> <a id="" class="faq-link" target="_blank" href="">FAQ</a></span>
                    </div> -->

                    <!-- WPA Encryption -->
                    <div class="info-block">
                        <div class="info-title"><#WLANConfig11b_WPAType_itemname#></div>
                        <div><select id="" class="input_option" onchange=""></select></div>
                    </div>

                    <!-- WPA Key -->
                    <div class="info-block">
                        <div class="info-title"><#WPA-PSKKey#></div>
                        <div>
                            <input type="password" class="input-size-25" oninput="" onfocus="" onblur="" />
                        </div>
                    </div>

                    <!-- WEP Encryption -->
                    <div class="info-block">
                        <div class="info-title"><#WLANConfig11b_WEPType_itemname#></div>
                        <div>
                            <select id="" class="input_option" onchange=""></select>
                        </div>
                    </div>

                    <!-- WEP Key -->
                    <div class="info-block">
                        <div class="info-title"><#WLANConfig11b_WEPKey_itemname#></div>
                        <div>
                            <input type="text" class="input-size-25" oninput="" />
                        </div>
                    </div>
                </div>
            </div>
            <div id="apply_button" class="button">
                <input type="button" class="button_gen" value="<#CTL_apply#>" onclick="apply();" />
            </div>
        </div>
    </body>
</html>
