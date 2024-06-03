let _nvram = httpApi.nvramGet(
    [
        "TM_EULA",
        "TM_EULA_time",
        "wrs_protect_enable",
        "wrs_mals_enable",
        "wrs_vp_enable",
        "wrs_cc_enable",
        "wps_enable",
        "dmz_ip",
        "autofw_enable_x",
        "vts_enable_x",
        "misc_http_x",
        "misc_ping_x",
        "st_ftp_mode",
        "st_samba_mode",
        "wrs_protect_enable",
        "wrs_mals_enable",
        "wrs_vp_enable",
        "wrs_cc_enable",
        "wans_dualwan",
        "wan0_upnp_enable",
        "wan1_upnp_enable",
        "dslx_transmode",
        "dsl0_upnp_enable",
        "dsl8_upnp_enable",
        "wrs_mals_t",
        "wrs_vp_t",
        "wrs_cc_t",
        "wrs_mail_bit",
        "PM_SMTP_AUTH_USER",
        "PM_SMTP_SERVER",
        "PM_SMTP_PORT",
        "PM_MY_EMAIL",
        "ctf_disable",
        "ctf_fa",
    ],
    true
);
(function () {
    genFeatureDesc();
    genRouterScan();
    setTimeout(function () {
        genMalsTopClient();
        genVPTopClient();
        genCCTopClient();

        genMalsChart();
        genVPChart();
        genCCChart();

        genMalsDetailTable();
        genVPDetailTable();
        genCCDetailTable();
    }, 100);

    getWhitelist();
})();

function genFeatureDesc() {
    let { wrs_protect_enable, wrs_mals_enable, wrs_vp_enable, wrs_cc_enable, TM_EULA, TM_EULA_time } = _nvram;
    let code = "",
        faq_href = "https://nw-dlcdnet.asus.com/support/forward.html?model=&type=Faq&lang=" + system.language.currentLang + "&kw=&num=139";

    code += `<h5 class="card-header"><#AiProtection_title#></h5>`;
    code += `
        <div class="card-body">
            <div class="d-flex justify-content-between">
                <div>
                    <div class="mb-3">
                        <#AiProtection_HomeDesc2#>
                    </div>
                    <a href="${faq_href}" target="_blank" class="text-decoration-underline"><#AiProtection_title#> FAQ</a>
                </div>
                <img src="images/New_ui/tm_logo.png" class="ms-5 tm-logo-smartHome" />
            </div>
            <div class="text-center pt-2 bg-img">
                <img src="images/New_ui/Home_Protection_Scenario.svg" class="aiProtection-scenario" />
            </div>
            <div class="d-flex justify-content-between align-items-end">
                <div>
                    <div class="my-3">
                        <div class="d-flex align-items-center">
                            <img src="images/New_ui/AiProtection_01.svg" class="aiProtection-step" />
                            <span class="ms-2"><#AiProtection_scan#></span>
                        </div>
                    </div>
                    <div class="my-3">
                        <div class="d-flex align-items-center justify-content-around mb-2">
                            <img src="images/New_ui/AiProtection_02.svg" class="aiProtection-step" />
                            <span class="ms-2"><#AiProtection_sites_blocking#></span>
                            <div class="form-check form-switch ms-auto">
                                <input class="form-check-input form-switch-large" type="checkbox" id="mals_switch"
                                ${wrs_protect_enable === "0" ? "" : wrs_mals_enable === "1" ? "checked" : ""}  />
                            </div>
                        </div>
                        <div class="ps-3 pe-2 feature-desc">
                            <#AiProtection_sites_block_desc#>
                        </div>
                    </div>
                    <div class="my-3">
                        <div class="d-flex align-items-center justify-content-around mb-2">
                            <img src="images/New_ui/AiProtection_02.svg" class="aiProtection-step" />
                            <span class="ms-2"><#AiProtection_two-way_IPS#></span>
                            <div class="form-check form-switch ms-auto">
                                <input class="form-check-input form-switch-large" type="checkbox" id="vp_switch"
                                ${wrs_protect_enable === "0" ? "" : wrs_vp_enable === "1" ? "checked" : ""} />
                            </div>
                        </div>
                        <div class="ps-3 pe-2 feature-desc">
                            <#AiProtection_two-way_IPS_desc#>
                        </div>
                    </div>
                    <div class="my-3">
                        <div class="d-flex align-items-center justify-content-around mb-2">
                            <img src="images/New_ui/AiProtection_03.svg" class="aiProtection-step" />
                            <span class="ms-2"><#AiProtection_detection_blocking#></span>
                            <div class="form-check form-switch ms-auto">
                                <input class="form-check-input form-switch-large" type="checkbox" id="cc_switch"
                                ${wrs_protect_enable === "0" ? "" : wrs_cc_enable === "1" ? "checked" : ""} />
                            </div>
                        </div>
                        <div class="ps-3 pe-2 feature-desc">
                            <#AiProtection_detection_block_desc#>
                        </div>
                    </div>
                </div>
            </div>
            <div class="text-end">
                <img src="images/New_ui/TrendMirco_logo.svg" class="tm-logo" />
            </div>
        </div>
    `;

    document.getElementById("feature_desc").innerHTML = code;
    document.getElementById("mals_switch").addEventListener("change", function () {
        let value = this.checked ? "1" : "0";
        if (TM_EULA === "0" || TM_EULA_time === "") {
            showTMEula("mals");
        } else {
            let applyObj = {
                action_mode: "apply",
                rc_service: "restart_wrs;restart_firewall",
                wrs_protect_enable: "1",
                wrs_mals_enable: value,
                action_time: 4,
            };

            let { ctf_disable, ctf_fa_mode } = _nvram;
            if ((ctf_disable == 0 && ctf_fa_mode == 2) || system.modelName === "MAP-AC1750") {
                applyObj.rc_service = "reboot";
                applyObj.action_time = httpApi.hookGet("get_default_reboot_time");
            }

            httpApi.nvramSet(applyObj, function () {
                setTimeout(function () {
                    location.reload();
                }, applyObj.action_time * 1000);
            });
        }
    });

    document.getElementById("vp_switch").addEventListener("change", function () {
        let value = this.checked ? "1" : "0";
        if (TM_EULA === "0" || TM_EULA_time === "") {
            showTMEula("vp");
        } else {
            let applyObj = {
                action_mode: "apply",
                rc_service: "restart_wrs;restart_firewall",
                wrs_protect_enable: "1",
                wrs_vp_enable: value,
                action_time: 4,
            };

            let { ctf_disable, ctf_fa_mode } = _nvram;
            if ((ctf_disable == 0 && ctf_fa_mode == 2) || system.modelName === "MAP-AC1750") {
                applyObj.rc_service = "reboot";
                applyObj.action_time = httpApi.hookGet("get_default_reboot_time");
            }

            httpApi.nvramSet(applyObj, function () {
                setTimeout(function () {
                    location.reload();
                }, applyObj.action_time * 1000);
            });
        }
    });

    document.getElementById("cc_switch").addEventListener("change", function () {
        let value = this.checked ? "1" : "0";
        if (TM_EULA === "0" || TM_EULA_time === "") {
            showTMEula("cc");
        } else {
            let applyObj = {
                action_mode: "apply",
                rc_service: "restart_wrs;restart_firewall",
                wrs_protect_enable: "1",
                wrs_cc_enable: value,
                action_time: 4,
            };

            let { ctf_disable, ctf_fa_mode } = _nvram;
            if ((ctf_disable == 0 && ctf_fa_mode == 2) || system.modelName === "MAP-AC1750") {
                applyObj.rc_service = "reboot";
                applyObj.action_time = httpApi.hookGet("get_default_reboot_time");
            }

            httpApi.nvramSet(applyObj, function () {
                setTimeout(function () {
                    location.reload();
                }, applyObj.action_time * 1000);
            });
        }
    });
}

function genRouterScan() {
    let {
        wps_enable,
        dmz_ip,
        autofw_enable_x,
        vts_enable_x,
        misc_http_x,
        misc_ping_x,
        st_ftp_mode,
        st_samba_mode,
        wrs_protect_enable,
        wrs_mals_enable,
        wrs_vp_enable,
        wrs_cc_enable,
        wans_dualwan,
        dslx_transmode,
        wan0_upnp_enable,
        wan1_upnp_enable,
        dsl0_upnp_enable,
        dsl8_upnp_enable,
    } = _nvram;

    let wpsEnable = wps_enable === "1" ? true : false,
        dmzEnable = dmz_ip !== "" ? true : false,
        portTriggerEnable = autofw_enable_x === "1" ? true : false,
        portForwardingEnable = vts_enable_x === "1" ? true : false,
        accessFromWAN = misc_http_x === "1" ? true : false,
        pingFromWAN = misc_ping_x === "1" ? true : false,
        ftpAnonymous = st_ftp_mode === "1" ? true : false,
        sambaAnonymous = st_samba_mode === "1" ? true : false,
        malsEnable = wrs_protect_enable === "1" && wrs_mals_enable === "1" ? true : false,
        vpEnable = wrs_protect_enable === "1" && wrs_vp_enable === "1" ? true : false,
        ccEnable = wrs_protect_enable === "1" && wrs_cc_enable === "1" ? true : false;

    //check default username/password
    let defaultNamePasswd = httpApi.hookGet("check_acorpw") === "1" ? true : false;

    // password strength of wireless
    let score = httpApi.hookGet("check_passwd_strength-wl_key"),
        level = Math.floor(score / 20);
    const passwdStrengthDesc = [
        "<#PASS_score0#>",
        "<#PASS_score1#>",
        "<#PASS_score2#>",
        "<#PASS_score3#>",
        "<#PASS_score4#>",
        "<#PASS_score4#>",
    ];

    /*
        check wireless auth, 
        - Personal: WPA2, WPA/WPA2, WAP3, WPA2/WPA3.
        - Enterprise: WPA2, WPA/WPA2, WPA3, WPA2/WPA3, Suite-b,
        - Enhanced Open, Enhanced Open Transition
    */
    let wlEncryptEnough = httpApi.hookGet("check_wireless_encryption") === "1" ? true : false;

    // check UPNP risk
    let redirectPage = "";
    let uPnpHasRisk = (function () {
        let dualWANIfArray = wans_dualwan.split(" ");
        let dslEnable = dualWANIfArray.find((value) => value === "dsl") === "dsl" ? true : false;

        if (dslEnable && dslx_transmode === "atm" && dsl0_upnp_enable === "1") {
            redirectPage = "Advanced_DSL_Content.asp";
            return true;
        } else if (dslEnable && dslx_transmode === "ptm" && dsl8_upnp_enable === "1") {
            redirectPage = "Advanced_VDSL_Content.asp";
            return true;
        } else if (
            ((dualWANIfArray[0] === "wan" || dualWANIfArray[0] === "lan") && wan0_upnp_enable === "1") ||
            ((dualWANIfArray[1] === "wan" || dualWANIfArray[1] === "lan") && wan1_upnp_enable === "1")
        ) {
            redirectPage = "Advanced_WAN_Content.asp";
            return true;
        }

        return false;
    })();

    let code = "";
    code += `
        <h5 class="card-header"><#AiProtection_scan#></h5>
        <div class="card-body">
            <div><#AiProtection_scan_desc#></div>
            <div class="mt-3">
                <div><#HSDPAConfig_Password_itemname#></div>
                <div class="d-flex">
                    <div class="d-flex align-items-start justify-content-between my-2 w-50">
                        <div class="feature-desc">
                            <#AiProtection_scan_item1#>
                        </div>
                        <button
                            type="button"
                            class="btn btn-sm mx-3 router-scan-button button-danger">                           
                            ${
                                defaultNamePasswd
                                    ? "<a href='Advanced_System_Content.asp' target='_blank'><#checkbox_No#></a>"
                                    : "<#checkbox_Yes#>"
                            }
                        </button>
                    </div>
                    <div class="d-flex align-items-start justify-content-between my-2 w-50">
                        <div class="feature-desc">
                            <#AiProtection_scan_item2#>
                        </div>
                        <button
                            type="button"
                            class="btn btn-sm mx-3 router-scan-button 
                            ${level < 2 ? "button-danger" : ""} 
                            ${level === 2 ? "button-warning" : ""} 
                            ${level > 2 ? "button-safe" : ""}
                            ">
                            ${passwdStrengthDesc[level]}
                        </button>
                    </div>
                </div>
            </div>

            <div class="mt-3">
                <div><#menu5_3#> / <#menu5_2#></div>
                <div class="d-flex flex-wrap">
                    <div class="d-flex align-items-start justify-content-between my-2 w-50">
                        <div class="feature-desc">
                            <#AiProtection_scan_item3#>
                        </div>
                        <button
                            type="button"
                            class="btn btn-sm mx-3 router-scan-button 
                            ${wlEncryptEnough ? "button-safe" : "button-danger"}"> 

                            ${
                                wlEncryptEnough
                                    ? " <#PASS_score3#>"
                                    : "<a href='Advanced_Wireless_Content.asp' target='_blank'><#PASS_score1#></a>"
                            }
                        </button>
                    </div>
                    <div class="d-flex align-items-start justify-content-between my-2 w-50">
                        <div class="feature-desc">
                            <#WPS_disabled#>
                        </div>
                        <button
                            type="button"
                            class="btn btn-sm mx-3 router-scan-button
                            ${!wpsEnable ? "button-safe" : "button-warning"}">
                                                                                                     
                            ${
                                !wpsEnable ? "<#checkbox_Yes#>" : "<a href='Advanced_WWPS_Content.asp' target='_blank'><#checkbox_No#></a>"
                            }                            
                        </button>
                    </div>

                    <div class="d-flex align-items-start justify-content-between my-2 w-50">
                        <div class="feature-desc">
                            <#AiProtection_scan_item4#>
                        </div>
                        <button
                            type="button"
                            class="btn btn-sm mx-3 router-scan-button 
                            ${uPnpHasRisk ? "button-warning" : "button-safe"}">
                            ${
                                uPnpHasRisk ? "<a href='" + redirectPage + "' target='_blank'><#checkbox_No#></a>" : "<#checkbox_Yes#>"
                            }                            
                        </button>
                    </div>

                    <div class="d-flex align-items-start justify-content-between my-2 w-50">
                        <div class="feature-desc">
                            <#AiProtection_scan_item7#>
                        </div>
                        <button
                            type="button"
                            class="btn btn-sm mx-3 router-scan-button
                            ${dmzEnable ? "button-warning" : "button-safe"}">
                            ${
                                dmzEnable
                                    ? "<a href='Advanced_Exposed_Content.asp' target='_blank'><#checkbox_No#></a>"
                                    : "<#checkbox_Yes#>"
                            }                            
                        </button>
                    </div>
                    <div class="d-flex align-items-start justify-content-between my-2 w-50">
                        <div class="feature-desc">
                            <#AiProtection_scan_item8#>
                        </div>
                        <button
                            type="button"
                            class="btn btn-sm mx-3 router-scan-button 
                            ${portTriggerEnable ? "button-warning" : "button-safe"}">
                            ${
                                portTriggerEnable
                                    ? "<a href='Advanced_PortTrigger_Content.asp' target='_blank'><#checkbox_No#></a>"
                                    : "<#checkbox_Yes#>"
                            }                            
                        </button>
                    </div>
                    <div class="d-flex align-items-start justify-content-between my-2 w-50">
                        <div class="feature-desc">
                            <#AiProtection_scan_item9#>
                        </div>
                        <button
                            type="button"
                            class="btn btn-sm mx-3 router-scan-button 
                            ${portForwardingEnable ? "button-warning" : "button-safe"}">
                            ${
                                portForwardingEnable
                                    ? "<a href='Advanced_VirtualServer_Content.asp' target='_blank'><#checkbox_No#></a>"
                                    : "<#checkbox_Yes#>"
                            }
                        </button>
                    </div>
                    <div class="d-flex align-items-start justify-content-between my-2 w-50">
                        <div class="feature-desc">
                            <#AiProtection_scan_item5#>
                        </div>
                        <button
                            type="button"
                            class="btn btn-sm mx-3 router-scan-button
                            ${accessFromWAN ? "button-warning" : "button-safe"}">
                            ${
                                accessFromWAN
                                    ? "<a href='Advanced_System_Content.asp' target='_blank'><#checkbox_No#></a>"
                                    : "<#checkbox_Yes#>"
                            }
                        </button>
                    </div>
                    <div class="d-flex align-items-start justify-content-between my-2 w-50">
                        <div class="feature-desc">
                            <#AiProtection_scan_item6#>
                        </div>
                        <button
                            type="button"
                            class="btn btn-sm mx-3 router-scan-button
                            ${pingFromWAN ? "button-warning" : "button-safe"}">
                            ${
                                pingFromWAN
                                    ? "<a href='Advanced_BasicFirewall_Content.asp' target='_blank'><#checkbox_No#></a>"
                                    : "<#checkbox_Yes#>"
                            }                            
                        </button>
                    </div>
                    <div class="d-flex align-items-start justify-content-between my-2 w-50">
                        <div class="feature-desc">
                            <#AiProtection_scan_item10#>
                        </div>
                        <button
                            type="button"
                            class="btn btn-sm mx-3 router-scan-button 
                            ${ftpAnonymous ? "button-warning" : "button-safe"}">                            
                            ${ftpAnonymous ? "<a href='Advanced_AiDisk_ftp.asp' target='_blank'><#checkbox_No#></a>" : "<#checkbox_Yes#>"}
                        </button>
                    </div>
                    <div class="d-flex align-items-start justify-content-between my-2 w-50">
                        <div class="feature-desc">
                            <#AiProtection_scan_item11#>
                        </div>
                        <button
                            type="button"
                            class="btn btn-sm mx-3 router-scan-button 
                            ${sambaAnonymous ? "button-warning" : "button-safe"}">
                            ${
                                sambaAnonymous
                                    ? "<a href='Advanced_AiDisk_samba.asp' target='_blank'><#checkbox_No#></a>"
                                    : "<#checkbox_Yes#>"
                            }                            
                        </button>
                    </div>
                </div>
            </div>

            <div class="mt-3">
                <div><#AiProtection_title#></div>
                <div class="d-flex flex-wrap">
                    <div class="d-flex align-items-start justify-content-between my-2 w-50">
                        <div class="feature-desc">
                            <#AiProtection_scan_item12#>
                        </div>
                        <button
                            type="button"
                            class="btn btn-sm mx-3 router-scan-button
                            ${malsEnable ? "button-safe" : "button-warning"}">
                            ${malsEnable ? "<#checkbox_Yes#>" : "<#checkbox_No#>"}                            
                        </button>
                    </div>
                    <div class="d-flex align-items-start justify-content-between my-2 w-50">
                        <div class="feature-desc">
                            <#AiProtection_scan_item13#>
                        </div>
                        <button
                            type="button"
                            class="btn btn-sm mx-3 router-scan-button
                            ${vpEnable ? "button-safe" : "button-warning"}">
                            ${vpEnable ? "<#checkbox_Yes#>" : "<#checkbox_No#>"}                            
                        </button>
                    </div>
                    <div class="d-flex align-items-start justify-content-between my-2 w-50">
                        <div class="feature-desc">
                            <#AiProtection_detection_blocking#>
                        </div>
                        <button
                            type="button"
                            class="btn btn-sm mx-3 router-scan-button
                            ${ccEnable ? "button-safe" : "button-warning"}">
                            ${ccEnable ? "<#checkbox_Yes#>" : "<#checkbox_No#>"}                        
                        </button>
                    </div>
                </div>
            </div>
        </div>    
    `;

    document.getElementById("router_scan").innerHTML = code;
}

// get IPS data
let ips = {
    mals: { count: 0, chart: [], topClient: [], detail: [] },
    vp: { count: 0, chart: { high: [], medium: [], low: [] }, topClient: [], detail: [] },
    cc: { count: 0, chart: [], topClient: [], detail: [] },
};
$.ajax({
    url: "/getAiProtectionEvent.asp",
    dataType: "script",
    error: function (xhr) {},
    success: function (response) {
        ips.mals.count = event_count.mals_n;
        ips.vp.count = event_count.vp_n;
        ips.cc.count = event_count.cc_n;
    },
});
$.ajax({
    url: "/getIPSEvent.asp?type=mals&event=mac",
    dataType: "script",
    error: function (xhr) {},
    success: function (response) {
        if (data != "") {
            let data_array = JSON.parse(data);
            ips.mals.topClient = data_array.map((x) => x);
            ips.mals.topClient.sort(function (a, b) {
                return parseInt(b[1]) - parseInt(a[1]);
            });
        }
    },
});
$.ajax({
    url: "/getIPSEvent.asp?type=vp&event=mac",
    dataType: "script",
    error: function (xhr) {},
    success: function (response) {
        if (data != "") {
            let data_array = JSON.parse(data);
            ips.vp.topClient = data_array.map((x) => x);
            ips.vp.topClient.sort(function (a, b) {
                return parseInt(b[1]) - parseInt(a[1]);
            });
        }
    },
});
$.ajax({
    url: "/getIPSEvent.asp?type=cc&event=mac",
    dataType: "script",
    error: function (xhr) {},
    success: function (response) {
        if (data != "") {
            let data_array = JSON.parse(data);
            ips.cc.topClient = data_array.map((x) => x);
            ips.cc.topClient.sort(function (a, b) {
                return parseInt(b[1]) - parseInt(a[1]);
            });
        }
    },
});

let date = new Date().getTime().toString().substring(0, 10);
$.ajax({
    url: "/getNonIPSChart.asp?type=mals&date=" + date,
    dataType: "script",
    error: function (xhr) {},
    success: function (response) {
        for (let item of data) {
            ips.mals.chart = item.map((x) => x);
        }
    },
});
$.ajax({
    url: "/getIPSChart.asp?type=vp&date=" + date,
    dataType: "script",
    error: function (xhr) {},
    success: function (response) {
        ips.vp.chart.high = data[0].map((x) => x);
        ips.vp.chart.medium = data[1].map((x) => x);
        ips.vp.chart.low = data[2].map((x) => x);
    },
});
$.ajax({
    url: "/getNonIPSChart.asp?type=cc&date=" + date,
    dataType: "script",
    error: function (xhr) {},
    success: function (response) {
        for (let item of data) {
            ips.cc.chart = item.map((x) => x);
        }
    },
});

$.ajax({
    url: "/getIPSDetailEvent.asp?type=mals&event=all",
    dataType: "script",
    error: function (xhr) {},
    success: function (response) {
        if (data != "") {
            let data_array = JSON.parse(data);
            ips.mals.detail = data_array.map((x) => x);
        }
    },
});
$.ajax({
    url: "/getIPSDetailEvent.asp?type=vp&event=all",
    dataType: "script",
    error: function (xhr) {},
    success: function (response) {
        if (data != "") {
            let data_array = JSON.parse(data);
            ips.vp.detail = data_array.map((x) => x);
        }
    },
});
$.ajax({
    url: "/getIPSDetailEvent.asp?type=cc&event=all",
    dataType: "script",
    error: function (xhr) {},
    success: function (response) {
        if (data != "") {
            let data_array = JSON.parse(data);
            ips.cc.detail = data_array.map((x) => x);
        }
    },
});
function genMalsTopClient() {
    let { wrs_mals_t } = _nvram;
    let code = "";

    code += `<h5 class="card-header"><#AiProtection_sites_blocking#></h5>`;
    code += `
        <div class="card-body">
            <div class="row h-100">
                <div class="dns-content-default">
                    <span><#AiProtection_event#></span>
                    <div class="d-flex align-items-center justify-content-center flex-column">
                        <div class="fs-4">${ips.mals.count}</div>
                        <div class="fs-7"><#AiProtection_scan_rHits#></div>
                    </div>
                    <div class="d-flex justify-content-between pb-2 card-data-title">
                        <div><#AiProtection_event_Source#></div>
                        <div><#NetworkTools_Count#></div>
                    </div>
                    <div>
    `;

    if (ips.mals.count === "0") {
        code += `
            <div class="d-flex justify-content-center align-items-center my-4">
                <div class="fs-5"><#AiProtection_eventnodetected#></div>
            </div>
        `;
    } else {
        let denom = ips.mals.count,
            client = system.client.detail;

        for (let item of ips.mals.topClient) {
            let mac = item[0],
                num = item[1],
                percentage = Math.ceil((num / denom) * 100),
                name = client[mac] ? client[mac].name : mac;

            if (percentage > 100) {
                percentage = 100;
            } else if (percentage < 0) {
                percentage = 1;
            }

            code += `
                <div>
                    <div class="row my-3">
                        <div class="col-12 col-md-12 col-xl-4 text-truncate dns-text" title="${name}">${name}</div>
                        <div class="col-8 col-xl-4 my-auto">
                            <div class="progress">
                                <div
                                    class="progress-bar progress-active bar-percent-${percentage}"
                                    role="progressbar"                                   
                                ></div>
                            </div>
                        </div>
                        <div class="col-4 col-xl-4 text-end card-data-value">${num}</div>
                    </div>
                </div>                   
            `;
        }
    }

    code += `
                    </div>
                </div>
            </div>
        </div>   
    `;

    code += `
        <div class="card-footer d-flex justify-content-between align-items-center">
            <div class="fs-7">${transferTimeFormat(wrs_mals_t * 1000)}</div>
            <div role="icon" class="icon-size-24 icon-delete me-3" id="mals_recount"></div>
        </div>
    `;

    document.getElementById("mals_topClient").innerHTML = code;
    document.getElementById("mals_recount").addEventListener("click", function () {
        let timestamp = new Date().getTime(),
            time = parseInt(timestamp / 1000);

        httpApi.nvramSet(
            {
                action_mode: "apply",
                rc_service: "",
                wrs_mals_t: time,
            },
            function () {
                setTimeout(function () {
                    location.reload();
                }, 1000);
            }
        );
    });
}

function genVPTopClient() {
    let { wrs_vp_t } = _nvram;
    let code = "";

    code += `
        <h5 class="card-header"><#AiProtection_two-way_IPS#></h5>
        <div class="card-body">
            <div class="row h-100">
                <div class="dns-content-default">
                    <span><#AiProtection_event#></span>
                    <div class="d-flex align-items-center justify-content-center flex-column">
                        <div class="fs-4">${ips.vp.count}</div>
                        <div class="fs-7"><#AiProtection_scan_rHits#></div>
                    </div>
                    <div class="d-flex justify-content-between pb-2 card-data-title">
                        <div><#AiProtection_event_Source#></div>
                        <div><#NetworkTools_Count#></div>
                    </div>
                    <div>
    `;

    if (ips.vp.count === "0") {
        code += `
            <div class="d-flex justify-content-center align-items-center my-4">
                <div class="fs-5"><#AiProtection_eventnodetected#></div>
            </div>
        `;
    } else {
        let denom = ips.vp.count;
        let client = system.client.detail;
        for (let item of ips.vp.topClient) {
            let mac = item[0],
                num = item[1];
            let percentage = Math.ceil((num / denom) * 100);
            if (percentage > 100) {
                percentage = 100;
            } else if (percentage < 0) {
                percentage = 1;
            }

            let name = client[mac] ? client[mac].name : mac;
            code += `
            <div>
                <div class="row my-3">
                    <div class="col-12 col-md-12 col-xl-4 text-truncate dns-text" title="${name}">${name}</div>
                    <div class="col-8 col-xl-4 my-auto">
                        <div class="progress">
                            <div
                                class="progress-bar progress-active bar-percent-${percentage}"
                                role="progressbar"
                            ></div>
                        </div>
                    </div>
                    <div class="col-4 col-xl-4 text-end card-data-value">${num}</div>
                </div>
            </div>                    
        `;
        }
    }

    code += `
                    </div>
                </div>
            </div>
        </div>
        <div class="card-footer d-flex justify-content-between align-items-center">
            <div class="fs-7">${transferTimeFormat(wrs_vp_t * 1000)}</div>
            <div role="icon" class="icon-size-24 icon-delete me-3" id="vp_recount"></div>
        </div>    
    `;

    document.getElementById("vp_topClient").innerHTML = code;
    document.getElementById("vp_recount").addEventListener("click", function () {
        let timestamp = new Date().getTime();
        let time = parseInt(timestamp / 1000);

        httpApi.nvramSet(
            {
                action_mode: "apply",
                rc_service: "",
                wrs_vp_t: time,
            },
            function () {
                setTimeout(function () {
                    location.reload();
                }, 1000);
            }
        );
    });
}

function genCCTopClient() {
    let { wrs_cc_t } = _nvram;
    let code = "";

    code += `
        <h5 class="card-header"><#AiProtection_detection_blocking#></h5>
        <div class="card-body">
            <div class="row h-100">
                <div class="dns-content-default">
                    <span><#AiProtection_event#></span>
                    <div class="d-flex align-items-center justify-content-center flex-column">
                        <div class="fs-4">${ips.cc.count}</div>
                        <div class="fs-7"><#AiProtection_scan_rHits#></div>
                    </div>
                    <div class="d-flex justify-content-between pb-2 card-data-title">
                        <div><#AiProtection_event_Source#></div>
                        <div><#NetworkTools_Count#></div>
                    </div>
                    <div>
    `;

    if (ips.cc.count === "0") {
        code += `
            <div class="d-flex justify-content-center align-items-center my-4">
                <div class="fs-5"><#AiProtection_eventnodetected#></div>
            </div>        
        `;
    } else {
        let denom = ips.cc.count;
        let client = system.client.detail;
        for (let item of ips.cc.topClient) {
            let mac = item[0],
                num = item[1];
            let percentage = Math.ceil((num / denom) * 100);
            if (percentage > 100) {
                percentage = 100;
            } else if (percentage < 0) {
                percentage = 1;
            }

            let name = client[mac] ? client[mac].name : mac;
            code += `
                <div>
                    <div class="row my-3">
                        <div class="col-12 col-md-12 col-xl-4 text-truncate dns-text" title="${name}">${name}</div>
                        <div class="col-8 col-xl-4 my-auto">
                            <div class="progress">
                                <div
                                    class="progress-bar progress-active bar-percent-${percentage}"
                                    role="progressbar"
                                ></div>
                            </div>
                        </div>
                        <div class="col-4 col-xl-4 text-end card-data-value">${num}</div>
                    </div>
                </div>            
            `;
        }
    }

    code += `
                    </div>
                </div>
            </div>
        </div>
        <div class="card-footer d-flex justify-content-between align-items-center">
            <div class="fs-7">${transferTimeFormat(wrs_cc_t * 1000)}</div>
            <div role="icon" class="icon-size-24 icon-delete me-3" id="cc_recount"></div>
        </div>   
    `;

    document.getElementById("cc_topClient").innerHTML = code;
    document.getElementById("cc_recount").addEventListener("click", function () {
        let timestamp = new Date().getTime();
        let time = parseInt(timestamp / 1000);

        httpApi.nvramSet(
            {
                action_mode: "apply",
                rc_service: "",
                wrs_cc_t: time,
            },
            function () {
                setTimeout(function () {
                    location.reload();
                }, 1000);
            }
        );
    });
}

function genMalsChart() {
    let timestamp = new Date().getTime();
    let labelTag = [];
    let length = 7;

    for (i = 0; i < length; i++) {
        let time = new Date(timestamp);
        let month = time.getMonth() + 1;
        let date = time.getDate();

        labelTag.unshift(month + "/" + date);
        timestamp -= 86400000;
    }

    let datasets = [];
    let cpuLineColor = getComputedStyle(document.querySelector(":root")).getPropertyValue("--chart-color-1");
    let ramLineColor = getComputedStyle(document.querySelector(":root")).getPropertyValue("--chart-color-2");

    datasets.push({
        label: "",
        data: ips.mals.chart,
        backgroundColor: [`rgba(${cpuLineColor}, 0.3)`],
        borderWidth: 1,
        borderColor: `rgb(${cpuLineColor})`,
        fill: true,
        yAxisID: "y",
        tension: 0.2,
    });

    let ctx_vp = document.getElementById("mals_chart").getContext("2d");
    let labelColor = getComputedStyle(document.querySelector(":root")).getPropertyValue("--text-regular-color");
    malsChart = new Chart(ctx_vp, {
        type: "line",
        data: {
            labels: labelTag,
            datasets: datasets,
        },
        options: {
            plugins: {
                legend: {
                    display: false,
                },
            },
            scales: {
                y: {
                    ticks: {
                        color: labelColor,
                        beginAtZero: true,
                    },
                    min: 0,
                },
                x: {
                    ticks: {
                        color: labelColor,
                    },
                },
            },
            responsive: true,
            maintainAspectRatio: false,
        },
    });
}

function genVPChart() {
    let timestamp = new Date().getTime();
    let labelTag = [];
    let length = 7;

    for (i = 0; i < length; i++) {
        let time = new Date(timestamp);
        let month = time.getMonth() + 1;
        let date = time.getDate();

        labelTag.unshift(month + "/" + date);
        timestamp -= 86400000;
    }

    let datasets = [];
    let cpuLineColor = getComputedStyle(document.querySelector(":root")).getPropertyValue("--chart-color-1");
    let highColor = "237,28,36";
    let mediumColor = "215,215,0";
    datasets.push(
        {
            label: "High",
            data: ips.vp.chart.high,
            backgroundColor: [`rgba(${highColor}, 0.3)`],
            borderWidth: 1,
            borderColor: `rgb(${highColor})`,
            fill: true,
            yAxisID: "y",
            tension: 0.2,
        },
        {
            label: "Medium",
            data: ips.vp.chart.medium,
            backgroundColor: [`rgba(${mediumColor}, 0.3)`],
            borderWidth: 1,
            borderColor: `rgb(${mediumColor})`,
            fill: true,
            yAxisID: "y",
            tension: 0.2,
        },
        {
            label: "Low",
            data: ips.vp.chart.low,
            backgroundColor: [`rgba(${cpuLineColor}, 0.3)`],
            borderWidth: 1,
            borderColor: `rgb(${cpuLineColor})`,
            fill: true,
            yAxisID: "y",
            tension: 0.2,
        }
    );

    let ctx_vp = document.getElementById("vp_chart").getContext("2d");
    let labelColor = getComputedStyle(document.querySelector(":root")).getPropertyValue("--text-regular-color");
    malsChart = new Chart(ctx_vp, {
        type: "line",
        data: {
            labels: labelTag,
            datasets: datasets,
        },
        options: {
            plugins: {
                legend: {
                    display: false,
                },
            },
            scales: {
                y: {
                    ticks: {
                        color: labelColor,
                        beginAtZero: true,
                        stepSize: 1,
                    },
                    min: 0,
                },
                x: {
                    ticks: {
                        color: labelColor,
                    },
                },
            },
            responsive: true,
            maintainAspectRatio: false,
        },
    });
}

function genCCChart() {
    let timestamp = new Date().getTime();
    let labelTag = [];
    let length = 7;
    for (i = 0; i < length; i++) {
        let time = new Date(timestamp);
        let month = time.getMonth() + 1;
        let date = time.getDate();
        labelTag.unshift(month + "/" + date);
        timestamp -= 86400000;
    }

    let datasets = [];
    let cpuLineColor = getComputedStyle(document.querySelector(":root")).getPropertyValue("--chart-color-1");
    datasets.push({
        label: "",
        data: ips.cc.chart,
        backgroundColor: [`rgba(${cpuLineColor}, 0.3)`],
        borderWidth: 1,
        borderColor: `rgb(${cpuLineColor})`,
        fill: true,
        yAxisID: "y",
        tension: 0.2,
    });

    let ctx_cc = document.getElementById("cc_chart").getContext("2d");
    let labelColor = getComputedStyle(document.querySelector(":root")).getPropertyValue("--text-regular-color");
    ccChart = new Chart(ctx_cc, {
        type: "line",
        data: {
            labels: labelTag,
            datasets: datasets,
        },
        options: {
            plugins: {
                legend: {
                    display: false,
                },
            },
            scales: {
                y: {
                    ticks: {
                        color: labelColor,
                        beginAtZero: true,
                    },
                    min: 0,
                },
                x: {
                    ticks: {
                        color: labelColor,
                    },
                },
            },
            responsive: true,
            maintainAspectRatio: false,
        },
    });
}

let catId = {
    2: "Illegal",
    39: "Proxy Avoidance",
    73: "Malicious Software",
    74: "Spyware",
    75: "Phishing",
    76: "Spam",
    77: "Adware",
    78: "Malware Accomplic",
    79: "Disease Vector",
    80: "Cookies",
    81: "Dialers",
    82: "Hacking",
    83: "Joke Program",
    84: "Password Cracking Apps",
    85: "Remote Access",
    86: "Made for AdSense sites",
    91: "C&C Server",
    92: "Malicious Domain",
    94: "Scam",
    95: "Ransomware",
};

let whitelist = {};
function getWhitelist() {
    $.ajax({
        url: "/wrs_wbl.cgi?action=get&type=0",
        type: "POST",
        success: function (response) {
            let res = JSON.parse(response).data;
            for (let item of res) {
                whitelist[item] = true;
            }
        },
    });
}
function genMalsDetailTable() {
    let code = "";
    let duplicate = {};
    code += `
        <h5 class="card-header d-flex align-items-center justify-content-between">
            <#AiProtection_sites_blocking#> - <#AiProtection_eventdetails#>
        </h5>
        <div class="card-body">
            <div class="table-responsive detail-table-height">
                <table class="table">
                    <thead>
                        <tr>
                            <th><#diskUtility_time#></th>
                            <th><#AiProtection_event_Threat#></th>
                            <th><#AiProtection_event_Source#></th>
                            <th><#AiProtection_event_Destination#></th>
                            <th></th>
                        </tr>
                    </thead>
                    <tbody>
    `;
    if (ips.mals.detail.length === 0) {
        code += `<tr><td colspan="5" class="text-center"><#IPConnection_VSList_Norule#></td></tr>`;
    } else {
        for (let item of ips.mals.detail) {
            let name = (function () {
                let mac = item[2];
                let client = system.client.detail;
                return client[mac] ? client[mac].name : mac;
            })();

            let compareString = item[1] + "_" + item[2] + "_" + item[3];
            if (duplicate[compareString] === undefined) {
                code += `
                    <tr class="align-middle">
                        <td>${item[0]}</td>
                        <td>${catId[item[1]]}</td>
                        <td>${name}</td>
                        <td>${item[3]}</td>
                        <td><div role="icon" class="icon-size-24 icon-add-circle me-3" data-bind="add_white_list" data-key=${
                            item[3]
                        }></div></td>
                    </tr>
                `;

                duplicate[compareString] = true;
            }
        }
    }

    code += `                    
                    </tbody>
                </table>
            </div>
        </div>
        <div class="card-footer d-flex justify-content-end align-items-center">            
            <div role="icon" class="icon-size-24 icon-delete me-3" title="<#CTL_del#>" id="mals_db_erase"></div>
            <div role="icon" class="icon-size-24 icon-edit-document me-3" title="Manage Whitelist" id="manage_whiteList"></div>
            <div role="icon" class="icon-size-24 icon-export-file me-3" title="<#CTL_onlysave#>" id="mals_db_export"></div>
        </div>
    `;

    document.getElementById("mals_detail").innerHTML = code;
    for (let target of document.querySelectorAll("[data-bind='add_white_list']")) {
        target.addEventListener("click", function () {
            let url = this.getAttribute("data-key");
            if (whitelist[url]) {
                return false;
            }

            $.ajax({
                url: "/wrs_wbl.cgi?action=add&type=0&url_type=url&url=" + url,
                type: "POST",
                success: function (response) {
                    whitelist[url] = true;
                },
            });
        });
    }

    document.getElementById("mals_db_erase").addEventListener("click", function () {
        eraseDB("mals_db");
    });

    document.getElementById("manage_whiteList").addEventListener("click", function () {
        manageWhiteList();
    });

    document.getElementById("mals_db_export").addEventListener("click", function () {
        let aTag = document.createElement("a");
        let content = (function () {
            let data = "Time,Threat,Source,Destination\n";
            for (let item of ips.mals.detail) {
                data += item.join(",");
                data += "\n";
            }

            return data;
        })();
        let fileName = "MaliciousSitesBlocking.csv";
        let mimeType = "data:text/csv;charset=utf-8";
        if (navigator.msSaveBlob) {
            // IE10
            return navigator.msSaveBlob(new Blob([content], { type: mimeType }), fileName);
        } else if ("download" in aTag) {
            //html5 A[download]
            aTag.href = "data:" + mimeType + "," + encodeURIComponent(content);
            aTag.setAttribute("download", fileName);
            document.getElementById("mals_db_export").appendChild(aTag);
            setTimeout(function () {
                document.getElementById("mals_db_export").removeChild(aTag);
                aTag.click();
            }, 66);
            return true;
        } else {
            //do iframe dataURL download (old ch+FF):
            let f = document.createElement("iframe");
            document.getElementById("mals_db_export").appendChild(f);
            f.src = "data:" + mimeType + "," + encodeURIComponent(content);
            setTimeout(function () {
                document.getElementById("mals_db_export").removeChild(f);
            }, 333);
            return true;
        }
    });
}

function eraseDB(type) {
    let template = `
        <div class="d-flex justify-content-center mt-5">
            <div class="card-float">
                <div class="card-header-float"><#Web_Title2#></div>
                <div class="card-body-float">
                    <#AiProtection_event_del_confirm#>
                </div>
                <div class="d-flex justify-content-end card-footer-float">
                    <div class="text-center btn-regular" id="db_erase_cancel">
                        <div><#CTL_Cancel#></div>
                    </div>
                    <div class="text-center btn-confirm" id="db_erase_confirm">
                        <div><#CTL_ok#></div>
                    </div>
                </div>
            </div>
        </div>
    `;

    let element = document.createElement("div");
    element.className = "shadow-bg";
    element.innerHTML = template;
    document.body.appendChild(element);

    let rcString = {
        mals_db: "reset_mals_db",
        vp_db: "reset_vp_db",
        cc_db: "reset_cc_db",
    };
    document.getElementById("db_erase_cancel").addEventListener("click", function () {
        document.body.removeChild(element);
    });
    document.getElementById("db_erase_confirm").addEventListener("click", function () {
        let timestamp = new Date().getTime();
        let time = parseInt(timestamp / 1000);
        httpApi.nvramSet(
            {
                action_mode: "apply",
                rc_service: rcString[type],
                wrs_mals_t: time,
            },
            function () {
                setTimeout(function () {
                    location.reload();
                }, 100);
            }
        );
        document.body.removeChild(element);
    });
}

function manageWhiteList() {
    let whitelist_length = Object.keys(whitelist).length;
    const WHITELIST_MAX_LENGTH = 64;
    let template = `
        <div class="d-flex justify-content-center mt-5">
            <div class="card-float">
                <div class="card-header-float"><#Web_Title2#></div>
                <div class="card-body-float">
                    <div><#AiProtection_sites_trust#></div>
                    <div class="d-flex align-items-center py-2">
                        <div class="me-auto"><#Domain_Name#></div>
                        <div class="card-content-float">
                            <div class="input-group">
                                <input type="text" class="form-control" id="whitelist_domain_name" value="" />
                                <button class="btn btn-input-icon" type="button">
                                    <div role="icon" class="icon-size-24 icon-add-circle" id="addWhitelist"></div>
                                </button>
                            </div>
                        </div>                    
                    </div>
                    
                    <div class="mt-3">
                        <div><#WhiteList#>: <span id="whitelist_length">${whitelist_length}</span>/${WHITELIST_MAX_LENGTH} (<#List_limit#> ${WHITELIST_MAX_LENGTH})</div>
                        <div class="table-responsive detail-table-height">
                            <table class="table">
                                <thead>
                                    <tr>
                                        <th><#Domain_Name#></th>
                                        <th><#btn_remove#></th>
                                    </tr>
                                </thead>
                                <tbody data-table="whitelist_tbody">
        `;

    for (let item of Object.keys(whitelist)) {
        template += `
            <tr>
                <td>${item}</td>
                <td><div role="icon" class="icon-size-24 icon-delete-circle me-3" data-bind="delete_white_list" data-key="${item}"></div></td>
            </tr>
        `;
    }

    template += `
                                </tbody>
                            </table>
                        </div>
                    </div>
                   
                </div>
                <div class="d-flex justify-content-end card-footer-float">
                    <div class="text-center btn-regular" id="whitelist_close">
                        <div><#CTL_close#></div>
                    </div>
                </div>
            </div>
        </div>
    `;

    let element = document.createElement("div");
    element.className = "shadow-bg";
    element.innerHTML = template;
    document.body.appendChild(element);

    document.getElementById("addWhitelist").addEventListener("click", function () {
        let url = document.getElementById("whitelist_domain_name").value;
        whitelist_length = Object.keys(whitelist).length;
        let hintElement = document.getElementById("whitelist_domain_name").parentNode.nextElementSibling;
        let ipformat =
            /^(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\.(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\.(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\.(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)$/;

        let domainNameFormat = /^((?:(?:(?:\w[\.\-\+]?)*)\w)+)((?:(?:(?:\w[\.\-\+]?){0,62})\w)+)\.(\w{2,6})$/;
        if (hintElement) {
            hintElement.remove();
        }

        if (
            whitelist[url] ||
            whitelist_length >= WHITELIST_MAX_LENGTH ||
            url === "" ||
            (!domainNameFormat.test(url) && !ipformat.test(url))
        ) {
            let text = "";
            if (whitelist[url]) {
                text = "<#AiProtection_ErrMsg_duplicate#>";
            } else if (whitelist_length >= WHITELIST_MAX_LENGTH) {
                text = "<#AiProtection_ErrMsg_full#>";
            } else if (url === "") {
                text = "<#AiProtection_ErrMsg_blank#>";
            } else if (!domainNameFormat.test(url) && !ipformat.test(url)) {
                text = "<#AiProtection_ErrMsg_wrong_format#>";
            }

            let element = document.createElement("div");
            element.className = "mt-2 hint-text";
            element.innerHTML = text;
            document.getElementById("whitelist_domain_name").parentNode.insertAdjacentElement("afterend", element);
            return false;
        }

        let target = document.querySelector('[data-table="whitelist_tbody"]');
        $.ajax({
            url: "/wrs_wbl.cgi?action=add&type=0&url_type=url&url=" + url,
            type: "POST",
            success: function (response) {
                whitelist[url] = true;
                let trElement = document.createElement("tr");
                let code = `
                    <td>${url}</td>
                    <td><div role="icon" class="icon-size-24 icon-delete-circle me-3" data-bind="delete_white_list" data-key="${url}"></div></td>
                `;

                trElement.innerHTML = code;
                target.appendChild(trElement);
                whitelist_length = Object.keys(whitelist).length;
                document.getElementById("whitelist_length").innerHTML = whitelist_length;
                document.getElementById("whitelist_domain_name").value = "";
                document.querySelector("[data-key='" + url + "']").addEventListener("click", function () {
                    delete whitelist[url];
                    trElement.remove();
                    whitelist_length = Object.keys(whitelist).length;
                    document.getElementById("whitelist_length").innerHTML = whitelist_length;
                });
            },
        });
    });

    document.getElementById("whitelist_close").addEventListener("click", function () {
        document.body.removeChild(element);
    });

    for (let target of document.querySelectorAll("[data-bind='delete_white_list']")) {
        let trElement = target.parentNode.parentNode;
        target.addEventListener("click", function () {
            let url = this.getAttribute("data-key");
            if (whitelist[url] === undefined) {
                return false;
            }

            $.ajax({
                url: "/wrs_wbl.cgi?action=del&type=0&url_type=url&url=" + url,
                type: "POST",
                success: function (response) {
                    delete whitelist[url];
                    trElement.remove();
                    document.getElementById("whitelist_length").innerHTML = Object.keys(whitelist).length;
                },
            });
        });
    }
}

function genVPDetailTable() {
    let code = "";
    let duplicate = {};
    let direct_type = ["", "Device Infected", "External Attacks"];
    code += `
        <h5 class="card-header d-flex align-items-center justify-content-between">
            <#AiProtection_two-way_IPS#> - <#AiProtection_eventdetails#>
        </h5>
        <div class="card-body">
            <div class="table-responsive">
                <table class="table">
                    <thead>
                        <tr>
                            <th><#diskUtility_time#></th>
                            <th><#AiProtection_level_th#></th>
                            <th>Type</th>
                            <th><#AiProtection_event_Source#></th>
                            <th><#AiProtection_event_Destination#></th>
                            <th><#AiProtection_event_Threat#></th>
                        </tr>
                    </thead>
                    <tbody>
    `;
    if (ips.vp.detail.length === 0) {
        code += `<tr><td colspan="6" class="text-center"><#IPConnection_VSList_Norule#></td></tr>`;
    } else {
        for (let item of ips.vp.detail) {
            let compareString = item[2] + "_" + item[3] + "_" + item[4];
            let queryString = (function () {
                let string1 = item[4].split(" ")[0] + "_" + item[3];
                let string2 = item[4].split(" ").slice(0, 3).join("+");
                return [string1, string2];
            })();

            if (duplicate[compareString] === undefined) {
                code += `
                    <tr class="align-middle">
                        <td>${item[0]}</td>
                        <td>${item[1]}</td>
                        <td>${direct_type[item[5]]}</td>
                        <td>${item[2]}</td>
                        <td>${item[3]}</td>
                        <td>
                            <a href="#" title="<#AiProtection_DetailTable_Click_Title#>" class="text-decoration-underline" 
                            onclick="threatQuery('${queryString[0]}', '${queryString[1]}')">${item[4]}</a>
                        </td>
                    </tr>
                `;

                duplicate[compareString] = true;
            }
        }
    }

    code += `                    
                    </tbody>
                </table>
            </div>
        </div>
        <div class="card-footer d-flex justify-content-end align-items-center">
            <div role="icon" class="icon-size-24 icon-delete me-3" id="vp_db_erase"></div>
            <div role="icon" class="icon-size-24 icon-export-file me-3" title="<#CTL_onlysave#>" id="vp_db_export"></div>
        </div>
    `;

    document.getElementById("vp_detail").innerHTML = code;
    document.getElementById("vp_db_erase").addEventListener("click", function () {
        eraseDB("vp_db");
    });

    document.getElementById("vp_db_export").addEventListener("click", function () {
        let aTag = document.createElement("a");
        let content = (function () {
            let data = "Time,Level,Type,Source,Destination,Threat\n";
            for (let item of ips.vp.detail) {
                data += item[0] + "," + item[1];
                data += "," + direct_type[item[5]];
                data += "," + item[2] + "," + item[3] + "," + item[4];
                data += "\n";
            }

            return data;
        })();

        let fileName = "IntrusionPreventionSystem.csv";
        let mimeType = "data:text/csv;charset=utf-8";
        if (navigator.msSaveBlob) {
            // IE10
            return navigator.msSaveBlob(new Blob([content], { type: mimeType }), fileName);
        } else if ("download" in aTag) {
            //html5 A[download]
            aTag.href = "data:" + mimeType + "," + encodeURIComponent(content);
            aTag.setAttribute("download", fileName);
            document.getElementById("vp_db_export").appendChild(aTag);
            setTimeout(function () {
                document.getElementById("vp_db_export").removeChild(aTag);
                aTag.click();
            }, 66);
            return true;
        } else {
            //do iframe dataURL download (old ch+FF):
            let f = document.createElement("iframe");
            document.getElementById("vp_db_export").appendChild(f);
            f.src = "data:" + mimeType + "," + encodeURIComponent(content);
            setTimeout(function () {
                document.getElementById("vp_db_export").removeChild(f);
            }, 333);
            return true;
        }
    });
}
let cat_id_index = {
    39: "Proxy Avoidance",
    73: "Malicious Software",
    74: "Spyware",
    75: "Phishing",
    76: "Spam",
    77: "Adware",
    78: "Malware Accomplic",
    79: "Disease Vector",
    80: "Cookies",
    81: "Dialers",
    82: "Hacking",
    83: "Joke Program",
    84: "Password Cracking Apps",
    85: "Remote Access",
    86: "Made for AdSense sites",
    91: "C&C Server",
    92: "Malicious Domain",
    94: "Scam",
    95: "Ransomware",
};
function genCCDetailTable() {
    let code = "";
    let duplicate = {};
    // let direct_type = ["", "Device Infected", "External Attacks"];
    code += `
        <h5 class="card-header d-flex align-items-center justify-content-between">
            <#AiProtection_detection_blocking#> - <#AiProtection_eventdetails#>
        </h5>
        <div class="card-body">
            <div class="table-responsive">
                <table class="table">
                    <thead>
                        <tr>
                            <th><#diskUtility_time#></th>
                            <th><#AiProtection_event_Threat#></th>            
                            <th><#AiProtection_event_Source#></th>
                            <th><#AiProtection_event_Destination#></th>
                        </tr>
                    </thead>
                    <tbody>
    `;
    if (ips.cc.detail.length === 0) {
        code += `<tr><td colspan="4" class="text-center"><#IPConnection_VSList_Norule#></td></tr>`;
    } else {
        for (let item of ips.cc.detail) {
            let name = (function () {
                let mac = item[2];
                let client = system.client.detail;
                return client[mac] ? client[mac].name : mac;
            })();

            let compareString = item[2] + "_" + item[3] + "_" + item[4];
            if (duplicate[compareString] === undefined) {
                code += `
                    <tr class="align-middle">
                        <td>${item[0]}</td>
                        <td>${cat_id_index[item[1]]}</td>                       
                        <td>${name}</td>                
                        <td>
                            <a href="#" title="<#AiProtection_DetailTable_Click_Title#>" class="text-decoration-underline" 
                            onclick="TMEvent()">${item[3]}</a>
                        </td>
                    </tr>
                `;

                duplicate[compareString] = true;
            }
        }
    }

    code += `                    
                    </tbody>
                </table>
            </div>
        </div>
        <div class="card-footer d-flex justify-content-end align-items-center">
            <div role="icon" class="icon-size-24 icon-delete me-3" id="cc_db_erase"></div>
            <div role="icon" class="icon-size-24 icon-export-file me-3" title="<#CTL_onlysave#>" id="cc_db_export"></div>
        </div>
    `;

    document.getElementById("cc_detail").innerHTML = code;
    document.getElementById("cc_db_erase").addEventListener("click", function () {
        eraseDB("cc_db");
    });

    document.getElementById("cc_db_export").addEventListener("click", function () {
        let aTag = document.createElement("a");
        let content = (function () {
            let data = "Time,Threat,Source,Destination\n";
            for (let item of ips.cc.detail) {
                data += item[0];
                data += "," + cat_id_index[item[1]];
                data += "," + item[2] + "," + item[3];
                data += "\n";
            }

            return data;
        })();

        let fileName = "InfectedDevicePreventBlock.csv";
        let mimeType = "data:text/csv;charset=utf-8";
        if (navigator.msSaveBlob) {
            // IE10
            return navigator.msSaveBlob(new Blob([content], { type: mimeType }), fileName);
        } else if ("download" in aTag) {
            //html5 A[download]
            aTag.href = "data:" + mimeType + "," + encodeURIComponent(content);
            aTag.setAttribute("download", fileName);
            document.getElementById("vp_db_export").appendChild(aTag);
            setTimeout(function () {
                document.getElementById("vp_db_export").removeChild(aTag);
                aTag.click();
            }, 66);
            return true;
        } else {
            //do iframe dataURL download (old ch+FF):
            let f = document.createElement("iframe");
            document.getElementById("vp_db_export").appendChild(f);
            f.src = "data:" + mimeType + "," + encodeURIComponent(content);
            setTimeout(function () {
                document.getElementById("vp_db_export").removeChild(f);
            }, 333);
            return true;
        }
    });
}

function threatQuery(id, keyword) {
    let url = "https://nw-dlcdnet.asus.com/trend/" + id + "?q=" + keyword;
    window.open(url, "_blank");
}

function TMEvent() {
    var url = "https://esupport.trendmicro.com/en-us/home/pages/technical-support/smart-home-network/1123020.aspx";
    window.open(url, "_blank");
}
function transferTimeFormat(time) {
    if (time == 0) {
        return "";
    }

    let t = new Date();
    t.setTime(time);
    let year = t.getFullYear();
    let month = t.getMonth() + 1;
    if (month < 10) {
        month = "0" + month;
    }

    let date = t.getDate();
    if (date < 10) {
        date = "0" + date;
    }

    let hour = t.getHours();
    if (hour < 10) {
        hour = "0" + hour;
    }

    let minute = t.getMinutes();
    if (minute < 10) {
        minute = "0" + minute;
    }

    return "Since " + year + "/" + month + "/" + date + " " + hour + ":" + minute;
}

function showTMEula(type) {
    let template = `
        <div class="d-flex justify-content-center mt-5">
            <div class="card-float">
                <div class="card-header-float"><#lyra_TrendMicro_agreement#></div>
                <div class="card-body-float">
                    <div class="mb-3"><#TM_eula_desc1#></div>

                    <div class="mb-3"><#TM_eula_desc2#></div>
                    <div><#TM_privacy_policy#></div>
                    <div><#TM_data_collection#></div>
                    
                    <div class="mt-3"><#TM_eula_desc3#></div>
                </div>
                <div class="d-flex justify-content-end card-footer-float">
                    <div class="text-center btn-regular" id="tm_eula_cancel">
                        <div><#CTL_Cancel#></div>
                    </div>
                    <div class="text-center btn-confirm" id="tm_eula_confirm">
                        <div><#CTL_ok#></div>
                    </div>
                </div>
            </div>
        </div>
    `;

    let element = document.createElement("div");
    element.className = "shadow-bg";
    element.innerHTML = template;
    document.body.appendChild(element);
    document.getElementById("tm_eula_cancel").addEventListener("click", function () {
        document.body.removeChild(element);
        if (type === "mals") {
            document.getElementById("mals_switch").checked = false;
        } else if (type === "vp") {
            document.getElementById("vp_switch").checked = false;
        } else if (type === "cc") {
            document.getElementById("cc_switch").checked = false;
        }
    });
    document.getElementById("tm_eula_confirm").addEventListener("click", function () {
        let applyObj = {
            action_mode: "apply",
            rc_service: "restart_wrs;restart_firewall",
            wrs_protect_enable: "1",
            TM_EULA: "1",
            action_time: 4,
        };

        let { ctf_disable, ctf_fa_mode } = _nvram;
        if ((ctf_disable == 0 && ctf_fa_mode == 2) || system.modelName === "MAP-AC1750") {
            applyObj.rc_service = "reboot";
            applyObj.action_time = httpApi.hookGet("get_default_reboot_time");
        }

        document.body.removeChild(element);
        applyLoading(applyObj.action_time);
        if (type === "mals") {
            applyObj["wrs_mals_enable"] = "1";
        } else if (type === "vp") {
            applyObj["wrs_vp_enable"] = "1";
        } else if (type === "cc") {
            applyObj["wrs_cc_enable"] = "1";
        }

        httpApi.nvramSet(applyObj, function () {
            setTimeout(function () {
                location.reload();
            }, applyObj.action_time * 1000);
        });
    });

    let tm_eula = "https://nw-dlcdnet.asus.com/support/forward.html?model=&type=TMeula&lang=" + system.language.currentLang + "&kw=&num=",
        tm_privacy = "https://nw-dlcdnet.asus.com/trend/tm_privacy",
        tm_data_collection = "https://nw-dlcdnet.asus.com/trend/tm_pdcd";

    // document.getElementById("eula_url").setAttribute("href", tm_eula);
    document.getElementById("tm_eula_url").setAttribute("href", tm_privacy);
    document.getElementById("tm_disclosure_url").setAttribute("href", tm_data_collection);
}

function applyLoading(time, callback) {
    let template = `
        <div class="d-flex justify-content-center mt-5">
            <div class="card-float">
                <div class="card-header-float">套用設定中</div>
                <div class="card-body-float">
                    <div class="d-flex align-items-center mt-3">
                        <div class="spinner-border loader-circle-color spinner-border-lg" role="status" aria-hidden="true"></div>
                        <div class="fs-4 fw-bold ms-4" id="loading_text">Loading...</div>
                    </div>                                        
                </div>     
            </div>
        </div>
    `;

    let shadowBgElement = document.querySelector(".shadow-bg");
    if (shadowBgElement) {
        document.body.removeChild(element);
    }

    let element = document.createElement("div");
    element.className = "shadow-bg";
    element.innerHTML = template;
    document.body.appendChild(element);

    let count = 1;
    let counter = function (time) {
        let value = Math.round((count / time) * 100);
        document.getElementById("loading_text").innerHTML = value + "%";
        count++;

        if (value > 100) {
            if (callback) {
                callback();
            }

            document.body.removeChild(element);
        } else {
            setTimeout(function () {
                counter(time);
            }, 1000);
        }
    };

    counter(time);
}

function alertPreference() {
    let template = `
        <div class="d-flex justify-content-center mt-5">
            <div class="card-float">
                <div class="card-header-float"><#AiProtection_alert_pref#></div>
                <div class="card-body-float">

                    <div class="d-sm-flex align-items-center py-2">
                        <div class="me-auto"><#Provider#></div>
                        <div class="card-content-float">
                            <div class="input-group">
                                <select class="form-select w-auto" id="mail_provider">
                                    <option value="GOOGLE">Google</option>
                                    <option value="AOL">AOL</option>
                                    <option value="QQ">QQ</option>
                                    <option value="163">163</option>
                                </select>
                            </div>
                        </div>
                    </div>

                    <div class="d-sm-flex align-items-center py-2">
                        <div class="me-auto"><#AiProtection_WebProtector_EMail#></div>
                        <div class="card-content-float">
                            <div class="input-group">
                                <input type="type" class="form-control" value="" maxlength="32" id="mail_account" />
                            </div>
                        </div>
                    </div>

                    <div class="d-sm-flex align-items-center py-2">
                        <div class="me-auto"><#PPPConnection_Password_itemname#></div>
                        <div class="card-content-float">
                            <div class="input-group">
                                <input type="password" class="form-control" value="" maxlength="64" id="mail_password" />
                            </div>
                        </div>
                    </div>

                    <div class="d-sm-flex align-items-center py-2">
                        <div class="me-auto"><#Notification_Item#></div>
                        <div class="card-content-float">
                            <div class="form-check">
                                <input class="form-check-input form-check-large" type="checkbox" value="" id="mals_alert_check">
                                <label class="form-check-label" for="mals_alert_check"><#AiProtection_sites_blocking#></label>
                            </div>
                            <div class="form-check">
                                <input class="form-check-input form-check-large" type="checkbox" value="" id="vp_alert_check">
                                <label class="form-check-label" for="vp_alert_check"><#AiProtection_two-way_IPS#></label>
                            </div>
                            <div class="form-check">
                                <input class="form-check-input form-check-large" type="checkbox" value="" id="cc_alert_check">
                                <label class="form-check-label" for="cc_alert_check"><#AiProtection_detection_blocking#></label>
                            </div>
                        </div>
                    </div>

                </div>
                <div class="d-flex justify-content-end card-footer-float">
                    <div class="text-center btn-regular" id="alert_preference_cancel">
                        <div><#CTL_Cancel#></div>
                    </div>
                    <div class="text-center btn-confirm" id="alert_preference_confirm">
                        <div><#CTL_apply#></div>
                    </div>
                </div>
            </div>
        </div>
    `;

    let element = document.createElement("div");
    element.className = "shadow-bg";
    element.innerHTML = template;
    document.body.appendChild(element);
    let { PM_MY_EMAIL, PM_SMTP_SERVER, wrs_mail_bit } = _nvram;
    let smtpServerList = {
        GOOGLE: { smtpServer: "smtp.gmail.com", smtpPort: "587", smtpDomain: "gmail.com" },
        AOL: { smtpServer: "smtp.aol.com", smtpPort: "587", smtpDomain: "aol.com" },
        QQ: { smtpServer: "smtp.qq.com", smtpPort: "587", smtpDomain: "qq.com" },
        163: { smtpServer: "smtp.163.com", smtpPort: "25", smtpDomain: "163.com" },
    };
    const MAIL_BITS_MALS = 1 << 0;
    const MAIL_BITS_VP = 1 << 1;
    const MAIL_BITS_CC = 1 << 2;

    // select mail provider
    for (let key of Object.keys(smtpServerList)) {
        let server = smtpServerList[key].smtpServer;
        if (server === PM_SMTP_SERVER) {
            document.getElementById("mail_provider").value = key;
            break;
        }
    }

    document.getElementById("mail_account").value = PM_MY_EMAIL;
    // set NOtification item
    let mailBits = parseInt(wrs_mail_bit);
    if ((mailBits & MAIL_BITS_MALS) !== 0) {
        document.getElementById("mals_alert_check").checked = true;
    }

    if ((mailBits & MAIL_BITS_VP) !== 0) {
        document.getElementById("vp_alert_check").checked = true;
    }

    if ((mailBits & MAIL_BITS_CC) !== 0) {
        document.getElementById("cc_alert_check").checked = true;
    }

    document.getElementById("alert_preference_cancel").addEventListener("click", function () {
        document.body.removeChild(element);
    });
    document.getElementById("alert_preference_confirm").addEventListener("click", function () {
        let mailProvider = document.getElementById("mail_provider").value;
        let mailAddress = "",
            smtpServer = "",
            smtpServerPort = "";
        let account = (function () {
            let acc = document.getElementById("mail_account").value.split("@");
            // include server domain
            if (acc[1] && acc[1] !== "") {
                let domain = acc[1].toLowerCase();
                if (domain !== "gmail.com" && domain !== "aol.com" && domain !== "qq.com" && domain !== "163.com") {
                    alert("Wrong mail domain");
                    document.getElementById("mail_account").focus();
                    return false;
                }
            }

            mailAddress = `${acc[0]}@${smtpServerList[mailProvider].smtpDomain}`;
            smtpServer = smtpServerList[mailProvider].smtpServer;
            smtpServerPort = smtpServerList[mailProvider].smtpPort;
            return acc[0];
        })();

        let hintElement = "";
        hintElement = document.getElementById("mail_account").parentNode.nextElementSibling;
        if (hintElement) {
            hintElement.remove();
        }

        if (account === "" || account.indexOf("`") !== -1) {
            let element = document.createElement("div");
            element.className = "mt-2 hint-text";
            if (account === "") {
                element.innerHTML = "<#JS_fieldblank#>";
            } else if (account.indexOf("`") !== -1) {
                element.innerHTML = "` " + " <#JS_validchar#>";
            }

            document.getElementById("mail_account").parentNode.insertAdjacentElement("afterend", element);
            return false;
        }

        let password = document.getElementById("mail_password").value;
        hintElement = document.getElementById("mail_password").parentNode.nextElementSibling;
        if (hintElement) {
            hintElement.remove();
        }

        if (password === "") {
            let element = document.createElement("div");
            element.className = "mt-2 hint-text";
            element.innerHTML = "<#JS_fieldblank#>";
            document.getElementById("mail_password").parentNode.insertAdjacentElement("afterend", element);
            return false;
        }

        wrs_mail_bit = (function () {
            let mals_check = document.getElementById("mals_alert_check").checked,
                vp_check = document.getElementById("vp_alert_check").checked,
                cc_check = document.getElementById("cc_alert_check").checked,
                bits = 0;

            bits += mals_check ? MAIL_BITS_MALS : 0;
            bits += vp_check ? MAIL_BITS_VP : 0;
            bits += cc_check ? MAIL_BITS_CC : 0;
            return bits;
        })();

        let applyObj = {
            action_mode: "apply",
            rc_service: "restart_wrs;restart_firewall;email_conf;send_confirm_mail",
            PM_SMTP_SERVER: smtpServer,
            PM_SMTP_PORT: smtpServerPort,
            PM_MY_EMAIL: mailAddress,
            PM_SMTP_AUTH_USER: account,
            PM_SMTP_AUTH_PASS: password,
            wrs_mail_bit,
        };

        httpApi.nvramSet(applyObj, function () {
            document.body.removeChild(element);
            applyLoading(4, function () {
                location.reload();
            });
        });
    });
}
