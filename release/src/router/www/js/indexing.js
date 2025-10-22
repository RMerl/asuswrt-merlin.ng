const searchInput = document.getElementById('router-assistant-input');
const resultsDiv = document.getElementById('router-assistant-response');
let dictData = {};
let usageIndex = {};
let qaPairData = {};
let currentLang = httpApi.nvramGet(["preferred_lang"]).preferred_lang || 'EN';
let menuTree = [
    {
        menuName: "<#AiProtection_title_Dashboard_title#>",
        tab: [
            {url: "index.html?page=dashboard", tabName: ``}
        ]
    },
    {
        menuName: "AiMesh",
        tab: [
            {url: "index.html?page=aimesh", tabName: ``}
        ]
    },
    {
        menuName: "Clients",
        tab: [
            {url: "index.html?page=clients", tabName: ``}
        ]
    },
    {
        menuName: "<#AiProtection_title_Dashboard_title#>",
        tab: [
            {url: "index.html?page=dashboard", tabName: ``}
        ]
    },
    {
        menuName: "Adaptive QoE",
        tab: [
            {url: "index.html?page=qoe", tabName: ``}
        ]
    },    
    {
        menuName: "<#Network#>",
        tab: [
            {url: "index.html?page=sdn", tabName: ``}
        ]
    },    
    {
        menuName: "VPN",
        tab: [
            {url: "index.html?page=vpn", tabName: ``}
        ]
    },    
    {
        menuName: "<#Traffic_Analyzer#>",
        tab: [
            {url: "index.html?page=trafficanalyzer", tabName: ``}
        ]
    },    
    {
        menuName: "<#AiProtection_title#>",
        tab: [
            {url: "index.html?page=aiprotection", tabName: ``}
        ]
    },
    {
        menuName: "<#Parental_Control#>",
        tab: [
            {url: "index.html?page=parentalcontrol", tabName: ``}
        ]
    },
    {
        menuName: "AI Board",
        tab: [
            {url: "index.html?page=aiboard", tabName: ``}
        ]
    },
    {
        menuName: "<#traffic_monitor#>",
        tab: [
            {url: "index.html?page=trafficmonitor", tabName: ``}
        ]
    },

    {
        menuName: "<#menu5_1#>",
        index: "menu_Wireless",
        tab: [
            {url: "Advanced_Wireless_Content.asp", tabName: "<#menu5_1_1#>"},
            {url: "Advanced_WWPS_Content.asp", tabName: "<#menu5_1_2#>"},
            {url: "Advanced_WMode_Content.asp", tabName: "WDS"},
            {url: "Advanced_ACL_Content.asp", tabName: "<#menu5_1_4#>"},
            {url: "Advanced_WSecurity_Content.asp", tabName: "<#menu5_1_5#>"},
            {url: "Advanced_WAdvanced_Content.asp", tabName: "<#menu5_1_6#>"},
            {url: "Advanced_WProxy_Content.asp", tabName: "<#WiFi_Proxy_item#>"},
            {url: "Advanced_Roaming_Block_Content.asp", tabName: "<#WiFi_Roaming_Block_List#>"},
            {url: "MLO.asp", tabName: `MLO`}
        ]
    },
    {
        menuName: "<#menu5_2#>",
        index: "menu_LAN",
        tab: [
            {url: "Advanced_LAN_Content.asp", tabName: "<#menu5_2_1#>"},
            {url: "Advanced_DHCP_Content.asp", tabName: "<#menu5_2_2#>"},
            {url: "Advanced_MultiSubnet_Content.asp", tabName: "<#menu5_2_2#>"},
            {url: "Advanced_GWStaticRoute_Content.asp", tabName: "<#menu5_2_3#>"},
            {url: "Advanced_IPTV_Content.asp", tabName: "IPTV"},
            {url: "Advanced_SwitchCtrl_Content.asp", tabName: "<#Switch_itemname#>"},
            {url: "Advanced_VLAN_Switch_Content.asp", tabName: "VLAN"},
            {url: "Advanced_VLAN_Profile_Content.asp", tabName: "__INHERIT__"}
        ]
    },
    {
        menuName: "<#menu5_3#>",
        index: "menu_WAN",
        tab: [
            {url: "Advanced_WAN_Content.asp", tabName: "<#menu5_3_1#>"},
            {url: "Advanced_DSL_Content.asp", tabName: "<#menu5_3_1#>"},
            {url: "Advanced_VDSL_Content.asp", tabName: "<#menu5_3_1#>"},
            {url: "Advanced_Modem_Content.asp", tabName: "__INHERIT__"},
            {url: "Advanced_MobileBroadband_Content.asp", tabName: "__INHERIT__"},
            {url: "Advanced_WANPort_Content.asp", tabName: "<#dualwan#>"},
            {url: "MultiGroup_WAN.asp", tabName: "Multi-WAN"},
            {url: "Advanced_PortTrigger_Content.asp", tabName: "<#menu5_3_3#>"},
            {url: "Advanced_VirtualServer_Content.asp", tabName: "<#menu5_3_4#>"},
            {url: "Advanced_Exposed_Content.asp", tabName: "<#menu5_3_5#>"},
            {url: "Advanced_ASUSDDNS_Content.asp", tabName: "<#menu5_3_6#>"},
            {url: "Advanced_NATPassThrough_Content.asp", tabName: "<#NAT_passthrough_itemname#>"}
        ]
    },
    {
        menuName: "AiMesh",
        index: "menu_AiMesh",
        tab: [
            {url: "AiMesh.asp", tabName: "AiMesh"}
        ]
    },
    {
        menuName: `<#Network#>`,
        index: "menu_GuestNetwork",
        tab: [
            {url: "SDN.asp", tabName: `<#Network#>`}
        ]
    },
    {
        menuName: "<#AiProtection_title_Dashboard_title#>",
        index: "menu_NewDashboard",
        tab: [
            {url: "index.html", tabName: "__HIDE__"}
        ]
    },
    {
        menuName: "<#AiProtection_title#>",
        index: "menu_AiProtection",
        tab: [
            {url: "AiProtection_HomeProtection.asp", tabName: "<#AiProtection_Home#>"},
            {url: "AiProtection_MaliciousSitesBlocking.asp", tabName: "<#AiProtection_sites_blocking#>"},
            {url: "AiProtection_IntrusionPreventionSystem.asp", tabName: "<#AiProtection_two-way_IPS#>"},
            {
                url: "AiProtection_InfectedDevicePreventBlock.asp",
                tabName: "<#AiProtection_detection_blocking#>"
            },
            {url: "AiProtection_AdBlock.asp", tabName: "Ad Blocking"},
            {url: "AiProtection_Key_Guard.asp", tabName: "Key Guard"}
        ]
    },
    {
        menuName: "<#Menu_TrafficManager#>",
        index: "menu_QoS",
        tab: [
            {url: "QoS_EZQoS.asp", tabName: "<#menu5_3_2#>"},
            {url: "Main_TrafficMonitor_realtime.asp", tabName: "<#traffic_monitor#>"},
            {url: "Main_TrafficMonitor_last24.asp", tabName: "__INHERIT__"},
            {url: "Main_TrafficMonitor_daily.asp", tabName: "__INHERIT__"},
            {url: "Main_Spectrum_Content.asp", tabName: "<#Spectrum_title#>"},
            {url: "AdaptiveQoS_TrafficLimiter.asp", tabName: "Traffic Limiter"},
            {url: "Advanced_QOSUserPrio_Content.asp", tabName: "__INHERIT__"},
            {url: "Advanced_QOSUserRules_Content.asp", tabName: "__INHERIT__"},
        ]
    },
    {
        menuName: "<#Parental_Control#>",
        index: "menu_ParentalControl",
        tab: [
            {url: "AiProtection_WebProtector.asp", tabName: "<#AiProtection_filter#>"},
            {url: "AiProtection_ContentFilter.asp", tabName: "<#AiProtection_filter#>"},
            {url: "ParentalControl.asp", tabName: "<#Time_Scheduling#>"},
            {url: "YandexDNS.asp", tabName: "<#YandexDNS#>"},
            {url: "adGuard_DNS.asp", tabName: "AdGuard"}
        ]
    },
    {
        menuName: "<#Adaptive_QoS#>",
        index: "menu_BandwidthMonitor",
        tab: [
            {url: "AdaptiveQoS_Bandwidth_Monitor.asp", tabName: "<#Bandwidth_monitor#>"},
            {url: "QoS_EZQoS.asp", tabName: "<#menu5_3_2#>"},
            {url: "AdaptiveQoS_WebHistory.asp", tabName: "<#Adaptive_History#>"},
            {url: "Main_Spectrum_Content.asp", tabName: "<#Spectrum_title#>"},
            {url: "Advanced_QOSUserPrio_Content.asp", tabName: "__INHERIT__"},
            {url: "Advanced_QOSUserRules_Content.asp", tabName: "__INHERIT__"},
            {url: "AdaptiveQoS_InternetSpeed.asp", tabName: "<#InternetSpeed#>"}
        ]
    },
    {
        menuName: "<#Traffic_Analyzer#>",
        index: "menu_TrafficAnalyzer",
        tab: [
            {url: "TrafficAnalyzer_Statistic.asp", tabName: "<#Statistic#>"},
            {url: "Main_TrafficMonitor_realtime.asp", tabName: "<#traffic_monitor#>"},
            {url: "Main_TrafficMonitor_last24.asp", tabName: "__INHERIT__"},
            {url: "Main_TrafficMonitor_daily.asp", tabName: "__INHERIT__"},
            {url: "AdaptiveQoS_TrafficLimiter.asp", tabName: "Traffic Limiter"}
        ]
    },
    {
        menuName: "<#Game_Boost#>",
        index: "menu_GameBoost",
        tab: [
            {url: "GameBoost.asp", tabName: "<#Game_Boost#>"}
        ]
    },
    {
        menuName: "Open NAT",
        index: "menu_OpenNAT",
        tab: [
            {url: "GameProfile.asp", tabName: "Open NAT"}
        ]
    },
    {
        menuName: "腾讯网游加速器",
        index: "menu_TencentAcceleration",
        tab: [
            {url: "GameBoost_Tencent.asp", tabName: "Tencent Game Acceleration"}
        ]
    },
    {
        menuName: "<#Menu_usb_application#>",
        index: "menu_APP",
        tab: [
            {url: "mediaserver.asp", tabName: "<#UPnPMediaServer#>"},
            {url: "Advanced_AiDisk_samba.asp", tabName: "<#menu5_4_1#>"},
            {url: "Advanced_AiDisk_ftp.asp", tabName: "<#menu5_4_2#>"},
            {url: "PrinterServer.asp", tabName: "<#Network_Printer_Server#>"},
            {url: "Advanced_Modem_Content.asp", tabName: "<#menu5_4_4#>"}
        ]
    },
    {
        menuName: "IPv6",
        index: "menu_IPv6",
        tab: [
            {url: "Advanced_IPv6_Content.asp", tabName: "IPv6"},
            {url: "Advanced_IPv61_Content.asp", tabName: "__INHERIT__"}
        ]
    },
    {
        menuName: "<#menu5_5#>",
        index: "menu_Firewall",
        tab: [
            {url: "Advanced_BasicFirewall_Content.asp", tabName: "<#menu5_1_1#>"},
            {url: "Advanced_URLFilter_Content.asp", tabName: "<#menu5_5_2#>"},
            {url: "Advanced_KeywordFilter_Content.asp", tabName: "<#menu5_5_5#>"},
            {url: "Advanced_Firewall_Content.asp", tabName: "<#menu5_5_4#>"}
        ]
    },
    {
        menuName: "<#menu5_6#>",
        index: "menu_Setting",
        tab: [
            {url: "Advanced_OperationMode_Content.asp", tabName: "<#menu5_6_1#>"},
            {url: "Advanced_System_Content.asp", tabName: "<#menu5_6_2#>"},
            {url: "Advanced_FirmwareUpgrade_Content.asp", tabName: "<#menu5_6_3#>"},
            {url: "Advanced_SettingBackup_Content.asp", tabName: "<#menu5_6_4#>"},
            {url: "Advanced_PerformanceTuning_Content.asp", tabName: "<#fan_tuning#>"},
            {url: "Advanced_ADSL_Content.asp", tabName: "<#menu_dsl_setting#>"},
            {url: "Advanced_Feedback.asp", tabName: "<#menu_feedback#>"},
            {url: "Feedback_Info.asp", tabName: "__INHERIT__"},
            {url: "Advanced_SNMP_Content.asp", tabName: "SNMP"},
            {url: "Advanced_TR069_Content.asp", tabName: "TR-069"},
            {url: "Advanced_OAM_Content.asp", tabName: "OAM"},
            {url: "Advanced_Notification_Content.asp", tabName: "Notification"},
            {url: "Advanced_Privacy.asp", tabName: "<#menu_privacy#>"},
            {url: "Advanced_MultiFuncBtn.asp", tabName: "Multi-Function Button"}
        ]
    },
    {
        menuName: "<#System_Log#>",
        index: "menu_Log",
        tab: [
            {url: "Main_LogStatus_Content.asp", tabName: "<#menu5_7_2#>"},
            {url: "Main_WStatus_Content.asp", tabName: "<#menu5_7_4#>"},
            {url: "Main_DHCPStatus_Content.asp", tabName: "<#menu5_7_3#>"},
            {url: "Main_IPV6Status_Content.asp", tabName: "IPv6"},
            {url: "Main_RouteStatus_Content.asp", tabName: "<#menu5_7_6#>"},
            {url: "Main_IPTStatus_Content.asp", tabName: "<#menu5_7_5#>"},
            {url: "Main_AdslStatus_Content.asp", tabName: "<#menu_dsl_log#>"},
            {url: "Main_ConnStatus_Content.asp", tabName: "<#Connections#>"},
            {url: "Main_Security_Change_Notification.asp", tabName: "Security Update Notification"},
            /* {url: "###Main_ConnStatus_Content.asp", tabName: "Captive Portal Connection Log"}, */
            {url: "NULL", tabName: "__INHERIT__"}
        ]
    },
    {
        menuName: "<#Network_Tools#>",
        index: "menu_NekworkTool",
        tab: [
            {url: "Main_Analysis_Content.asp", tabName: "<#Network_Analysis#>"},
            {url: "Main_Netstat_Content.asp", tabName: "Netstat"},
            {url: "Main_WOL_Content.asp", tabName: "<#NetworkTools_WOL#>"},
            // {url: "Main_ChkSta_Content.asp", tabName: "<#NetworkTools_ChkSta#>"},
            {url: "Advanced_Smart_Connect.asp", tabName: "<#smart_connect_rule#>"}
        ]
    },
    {
        menuName: "ASUS Site Manager",
        index: "menu_SiteManager",
        tab: [
            {url: "Advanced_Site_PinCode.asp", tabName: "PIN Code"},
            {url: "Advanced_Site_Manager.asp", tabName: "<#Pincode_Title#>"}
        ]
    }
];

if (isSupport("UI4")) {
    menuTree = menuTree.filter(menu => menu.index !== "menu_TrafficAnalyzer");
    menuTree = menuTree.filter(menu => menu.index !== "menu_AiProtection");
    menuTree = menuTree.filter(menu => menu.index !== "menu_AiMesh");
    menuTree = menuTree.filter(menu => menu.index !== "menu_GuestNetwork");
    menuTree = menuTree.filter(menu => menu.index !== "menu_ParentalControl");
}
if (isSupport("gtbooster") && isSupport("ark_qoe")) {
    menuTree = menuTree.filter(menu => menu.index !== "menu_QoS");
    menuTree = menuTree.filter(menu => menu.index !== "menu_BandwidthMonitor");
}
if (!isSupport("newsite_provisioning")) {
    menuTree = menuTree.filter(menu => menu.index !== "menu_SiteManager");
}
if (!isSupport("tencent_qmacc") || (!isSwMode("RT") && !isSwMode("WISP"))) {
    menuTree = menuTree.filter(menu => menu.index !== "menu_TencentAcceleration");
}
if (!isSupport("gameMode") || isSupport("UI4")) {
    if (!isSupport("gu_accel") || isSupport("UI4")) {
        menuTree = menuTree.filter(menu => menu.index !== "menu_GameBoost");
    }
}
if (!isSupport("usbX")) {
    menuTree = menuTree.filter(menu => menu.index !== "menu_APP");
}
if (!isSupport("ipv6")) {
    menuTree = menuTree.filter(menu => menu.index !== "menu_IPV6");
}

async function loadData(lang = 'EN') {
    var parseDict = function(text) {
        const lines = text.split(/\r?\n/);
        const data = {};
        lines.forEach(line => {
            if (line.includes('=')) {
                const parts = line.split('=', 2);
                const key = parts[0].trim();
                const value = parts[1].trim();
                if (key && value) {
                    data[key] = value
                        .replace(/ZVMODELVZ/g, httpApi.nvramGet(["odmpid"]).odmpid)
                        .replace(/ZVDOMAIN_NAMEVZ/g, httpApi.nvramGet(["local_domain"]).local_domain)             
                }
            }
        });
        return data;
    }

/*
    let dictText = localStorage.getItem(`dict.${lang}`);
    if (!dictText) {
        const dictResponse = await fetch(`https://nw-dlcdnet.asus.com/indexing/${lang}.dict`);
        dictText = await dictResponse.text();
        try {
            localStorage.setItem(`dict.${lang}`, dictText);
        } catch (error) {
            localStorage.clear();
            localStorage.setItem(`dict.${lang}`, dictText);
        }
    }
    dictData = parseDict(dictText);

    let usageJson = localStorage.getItem('usageIndex');
    if (!usageJson) {
        const usageResponse = await fetch('https://nw-dlcdnet.asus.com/indexing/indexing.json');
        usageJson = JSON.stringify(await usageResponse.json());

        try {
            localStorage.setItem('usageIndex', usageJson);
        } catch (error) {
            localStorage.clear();
            localStorage.setItem('usageIndex', usageJson);
        }

    }
    usageIndex = JSON.parse(usageJson);
*/

/* force to use cloud data - begin */
    let dictText = "";
    const dictResponse = await fetch(`https://nw-dlcdnet.asus.com/indexing/${lang}.dict`);
    dictText = await dictResponse.text();
    dictData = parseDict(dictText);

    let usageJson = "";
    const usageResponse = await fetch('https://nw-dlcdnet.asus.com/indexing/indexing.json');
    usageJson = JSON.stringify(await usageResponse.json());
    usageIndex = JSON.parse(usageJson);

    let qaPair = "";
    if(isSupport("ai_board_slm")){
        const qaPairResponse = await fetch('https://nw-dlcdnet.asus.com/indexing/qa_asuswrt.json');
        qaPair = JSON.stringify(await qaPairResponse.json());
        qaPairData = JSON.parse(qaPair);
    }

    // Filter out entries with id=10029 in the answer
    if (qaPairData && qaPairData.qa_pairs) {
        qaPairData.qa_pairs = qaPairData.qa_pairs.filter(qa => {
            return !(qa.answer && qa.answer.includes('id=10029'));
        });
    }
/* force to use cloud data - end */

    searchInput.addEventListener('input', performSearch);

    const style = document.createElement('style');
    style.textContent = `
        .result-item {
            margin-bottom: 15px;
            padding-bottom: 10px;
            border-bottom: 1px solid #eee;
        }
        .result-item strong {
            color: #d9534f;
        }
        .tab-name {
            color: #0056b3;
            font-size: 1.1em;
            font-weight: bold;
        }
        .found-string {
            color: #6c757d;
            font-size: 0.9em;
            padding-left: 20px;
        }
        .highlight {
            color: red;
            font-weight: bold;
        }
        .result-item ul {
            list-style-type: none;
            padding-left: 0;
        }
        .result-item li {
            padding: 5px 0;
        }
        .result-item a {
            color: #0275d8;
            text-decoration: none;
        }
        .result-item a:hover {
            text-decoration: underline;
        }
        .qa-section {
            margin-bottom: 15px;
            padding-bottom: 10px;
            border-bottom: 1px solid #eee;
        }
        .qa-section .tab-name {
            color: #0275d8;
            font-size: 1.1em;
            font-weight: bold;
            margin-bottom: 10px;
        }
        .qa-question {
            color: #0056b3;
            text-decoration: none;
            cursor: pointer;
            font-size: 0.9em;
            padding: 5px 0;
            display: block;
            padding-left: 20px;
        }
        .qa-question:hover {
            text-decoration: underline;
        }
    `;

    document.head.appendChild(style);
}

function performSearch() {
    var getTabNameFromUrl = function(url) {
        for (const menu of menuTree) {
            let lastValidTabName = null;
            for (const tab of menu.tab) {
                if (tab.tabName !== "__HIDE__" && tab.tabName !== "__INHERIT__") {
                    lastValidTabName = tab.tabName;
                }
                if (tab.url === url) {
                    if (tab.tabName === "__HIDE__" || tab.tabName === "__INHERIT__") {
                        if (lastValidTabName) {
                            return `${menu.menuName} > ${lastValidTabName}`;
                        } else {
                            return menu.menuName;
                        }
                    }
                    return `${menu.menuName} > ${tab.tabName}`;
                }
            }
        }

        return null;
    }

    var handleQAButtonClick = function(question) {
        // Clear textarea and reset height
        const textarea = document.getElementById('router-assistant-input');
        textarea.value = '';
        if (typeof autoResizeTextarea === 'function') {
            autoResizeTextarea(textarea);
        }

        // Show loading in response area with user question
        showRouterAssistantResponse('loading', "");

        // Call the AI API
        setTimeout(() => {
            httpApi.ai_board_slm({"question": question}, function(response) {
                showResult(response, question);
            });
        }, 500);
    }

    const query = searchInput.value.trim().toLowerCase();
    resultsDiv.innerHTML = '';

    if (query.length < 2) {
        return;
    }

    // Search QA pairs first
    const matchingQuestions = [];
    if (qaPairData && qaPairData.qa_pairs) {
        qaPairData.qa_pairs.forEach(qa => {
            if (qa.question && qa.question.toLowerCase().includes(query) && qa.question.length <= 1000) {
                matchingQuestions.push(qa.question);
            }
        });
    }

    const resultsByTab = {};

    for (const flag in dictData) {
        const value = dictData[flag].toLowerCase();
        if (value.includes(query)) {
            const urls = usageIndex[flag];
            if (urls && urls.length > 0) {
                const validUrls = urls
                    .map(url => ({ url, tabName: getTabNameFromUrl(url) }))
                    .filter(item => item.tabName !== null);
                
                if (validUrls.length > 0) {
                    const uniqueTabNames = [...new Set(validUrls.map(item => item.tabName))];
                    uniqueTabNames.forEach(tabName => {
                        if (!resultsByTab[tabName]) {
                            resultsByTab[tabName] = [];
                        }
                        resultsByTab[tabName].push(dictData[flag]);
                    });
                }
            }
        }
    }

    let html = '';
    let sortedTabs = [];
    // Sort tabs according to menuTree order instead of alphabetical
    const getMenuOrder = (tabName) => {
        for (let i = 0; i < menuTree.length; i++) {
            const menu = menuTree[i];
            for (const tab of menu.tab) {
                const fullTabName = (tab.tabName === "__HIDE__" || tab.tabName === "__INHERIT__")
                    ? menu.menuName
                    : `${menu.menuName} > ${tab.tabName}`;
                if (fullTabName === tabName) {
                    return i; // Return menu index as order priority
                }
            }
        }
        return menuTree.length; // Put unknown tabs at the end
    };
    sortedTabs = Object.keys(resultsByTab).sort((a, b) => {
        const orderA = getMenuOrder(a);
        const orderB = getMenuOrder(b);
        if (orderA !== orderB) {
            return orderA - orderB;
        }
        // If same menu order, sort alphabetically as fallback
        return a.localeCompare(b);
    });
    const matched = sortedTabs.filter(tab => tab.toLowerCase().includes(query));
    const unmatched = sortedTabs.filter(tab => !tab.toLowerCase().includes(query));
    sortedTabs = [...matched, ...unmatched];

    // Display QA questions first if any matches found
    if (matchingQuestions.length > 0) {
        const highlightRegex = new RegExp(`(${query.replace(/[-\/\\^$*+?.()|[\]{}]/g, '\\$&')})`, 'gi');
        
        html += `
            <div class="qa-section">
                <p class="tab-name">Related Questions</p>
                <ul>
        `;
        
        matchingQuestions.forEach((question, index) => {
            const highlightedQuestion = question.replace(highlightRegex, '<span class="highlight">$1</span>');
            html += `
                <li><a class="qa-question" href="#" data-question="${encodeURIComponent(question)}" data-qa-index="${index}">"${highlightedQuestion}"</a></li>
            `;
        });
        
        html += `
                </ul>
            </div>
        `;
    }

    if (sortedTabs.length === 0 && matchingQuestions.length === 0) {
        resultsDiv.innerHTML = '';
        return;
    }

    const highlightRegex = new RegExp(`(${query.replace(/[-\/\\^$*+?.()|[\]{}]/g, '\\$&')})`, 'gi');

    sortedTabs.forEach(tabName => {
        const strings = [...new Set(resultsByTab[tabName])];

        let tabUrl = null;
        for (const menu of menuTree) {
            for (const tab of menu.tab) {
                const fullTabName = (tab.tabName === "__HIDE__" || tab.tabName === "__INHERIT__") ? menu.menuName : `${menu.menuName} > ${tab.tabName}`;
                if (fullTabName === tabName) {
                    tabUrl = tab.url;
                    break;
                }
            }
            if (tabUrl) break;
        }
        html += `
            <div class="result-item">
                <p class="tab-name"><a href="${tabUrl ? tabUrl : '#'}">${tabName}</a></p>
                <ul>
                    ${strings.map(str => {
                        const highlightedStr = str.replace(highlightRegex, '<span class="highlight">$1</span>');
                        return `<li class="found-string">"${highlightedStr}"</li>`;
                    }).join('')}
                </ul>
            </div>
        `;
    });
   
    resultsDiv.innerHTML = html;
    document.getElementById('router-assistant-response').style.display = '';
    
    // Add event listeners for QA questions
    const qaQuestions = resultsDiv.querySelectorAll('.qa-question');
    qaQuestions.forEach(link => {
        link.addEventListener('click', function(e) {
            e.preventDefault();
            const question = decodeURIComponent(this.getAttribute('data-question'));
            handleQAButtonClick(question);
        });
    });
}

loadData(currentLang);