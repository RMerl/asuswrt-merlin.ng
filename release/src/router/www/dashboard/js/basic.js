var Session = Session || (function(){
	var win = window.top || window;
	try{
		var store = (win.name ? JSON.parse(win.name) : {});
		function Save() {
			win.name = JSON.stringify(store);
		}

		if (window.addEventListener) window.addEventListener("unload", Save, false);
		else if (window.attachEvent) window.attachEvent("onunload", Save);
		else window.onunload = Save;

		return {
			set: function(name, value) {
				store[name] = value;
			},
			get: function(name) {
				return (store[name] ? store[name] : undefined);
			},
			clear: function() { store = {}; },
			dump: function() { return JSON.stringify(store); }
		};
	}
	catch(e){
		win.name = ""; /* reset cache */
		return {
			set: function(){},
			get: function(){},
			clear: function(){},
			dump: function(){}
		};
	}
})();

function genArchitecture() {
	if(navigator.userAgent.match(/ASUSMultiSiteManager/) || navigator.userAgent.match(/ASUSExpertSiteManager/) || (/^aae-sgrst001-\w+\.asuscomm\.com$/.test(window.location.hostname))) {
		genHeader(["logo","model-name"]);
	}else{
		genHeader();
	}
	genNavMenu();
	genMidHeader();
}

function genHeader(filter = ["logo", "router-assistant", "model-name", "time", "notification", "logout", "reboot", "language", "more"]) {
    let header = document.getElementsByTagName("header")[0];
    if (header) {
        let code = "";

        // LOGO, MODEL NAME
        if(filter.includes("logo")) {
            code += genLogoModelName(filter);
        }

        if(filter.includes("router-assistant")) { 
            code += genRouterAssistant();
        }

        // NOTIFICATION
        if (filter.includes("notification")) {
            code += genNotification();
        }

        // LOGOUT
        if(filter.includes("logout")) {
            code += genLogout();
        }

        // REBOOT
        if(filter.includes("reboot")) {
            code += genReboot();
        }

        // LANGUAGE MENU
        if(filter.includes("language")) {
            code += genLanguageList();
        }

        // MORE
        if(filter.includes("more")) {
            code += `
                <div role="more" class="d-flex justify-content-end d-lg-none">
                    <a href="#" class="nav-link dropdown-toggle rounded-pill p-1 lh-1 pe-1 pe-md-2 d-flex align-items-center justify-content-center" aria-expanded="true" data-bs-toggle="dropdown" data-bs-auto-close="outside">
                        <div role="icon" class="icon-size-28 icon-more-vert mx-2"></div>
                     </a>
                     <div class="dropdown-menu mt-0 p-0 dropdown-menu-end overflow-hidden">
                        <div role="" class="dropdown-item d-flex align-items-center p-3 border-bottom" onclick="logout();">
                            <div role="icon" class="icon-size-24 icon-logout"></div>
                            <div role="" class="ps-2 logout-title"><#t1Logout#></div>
                        </div>
                        <div role="t" class="dropdown-item d-flex align-items-center p-3" onclick="reboot();">
                            <div role="icon" class="icon-size-24 icon-reboot"></div>
                            <div role="" class="ps-2 reboot-title"><#BTN_REBOOT#></div>
                        </div>
                             
                     </div>
                </div>
            `;
        }


        header.innerHTML = code;
        setTimeout(checkNotification, 2000);
        import('./indexing.js').catch(e => { console.error('Failed to load indexing.js', e); });
        
        if(filter.includes("router-assistant")) {
            setTimeout(checkRouterAssistantVisibility, 1000);
        }
    }
}

function genRouterAssistant() {
    let code = "";
	var searchIconClass = isSupport("ai_board_slm") ? "icon-AiBoard" : "icon-search";
	var searchText = isSupport("ai_board_slm") ? "Ask Router Assistant..." : "<#CTL_search#>";
	
    code += `
        <div role="router-assistant" class="router-assistant-container flex-grow-1 mx-3" style="position: relative; display: flex; justify-content: center;z-index: 1999;">
            <div id="router-assistant-wrapper" style="position:relative; width:100%;">
                <div class="position-absolute" style="left: 12px; top: 50%; transform: translateY(-50%); z-index: 10; pointer-events: none;">
                    <div role="icon" class="icon-size-20 ${searchIconClass}" style="opacity: 0.6;"></div>
                </div>
                <textarea id="router-assistant-input" 
                          class="form-control" 
                          placeholder="${searchText}" 
                          rows="1"
                          style="width:100%; resize:none; overflow-y:hidden; min-height:38px; max-height:120px; transition: height 0.2s ease; line-height: 1.4; padding-left: 40px;"
                          onkeydown="handleRouterAssistantInput(event)"
                          oninput="autoResizeTextarea(this)"></textarea>
            </div>
            <div id="router-assistant-response" 
                 class="position-absolute bg-white shadow rounded p-3" 
                 style="font-size: 1rem; top:100%; width:100%; max-height:500px; overflow-y:auto; display:none; z-index:1050; border:1px solid #dee2e6;">
            </div>
        </div>
    `;

    return code;
}

function checkRouterAssistantVisibility() {
    const container = document.querySelector('[role="router-assistant"]');
    const wrapper = document.getElementById('router-assistant-wrapper');
    
    if (container && wrapper) {
        const updateSize = () => {
            const wasHidden = container.style.display === 'none';
            if (wasHidden) {
                container.style.display = 'flex';
                container.style.visibility = 'hidden';
            }
            
            const containerRect = container.getBoundingClientRect();
            const availableWidth = containerRect.width;
            
            if (availableWidth < 200) {
                container.style.display = 'none';
                container.style.visibility = 'visible';
            } else {
                container.style.display = 'flex';
                container.style.visibility = 'visible';
                
                const targetWidth = Math.min(availableWidth - 40, 500);
                wrapper.style.width = targetWidth + 'px';
                
                const responseDiv = document.getElementById('router-assistant-response');
                if (responseDiv) {
                    responseDiv.style.width = targetWidth + 'px';
                }
            }
        };
        
        window.addEventListener('resize', updateSize);
        
        const observer = new ResizeObserver(entries => {
            updateSize();
        });
        
        observer.observe(container.parentElement || document.body);
        
        updateSize();
    }
}

function genLogoModelName(filter = ["logo", "model-name", "time"]) {
	let code = "";
	code += `
		<div class="d-flex align-items-center me-auto">
			<div role="icon" class="d-block d-md-none icon-size-24 icon-menu ms-3 menu-header" id='mobile_menu'></div>
			${filter.includes('logo')?`<div class="d-none d-md-block icon-logo-asus ms-3"></div>`:``}
			${filter.includes('model-name')?`<div class="model-name mx-3"><#Web_Title2#></div>`:``}
			${filter.includes('time')?`
			<div role="time-banner" class="d-none px-2 py-1 d-lg-flex flex-row" onclick="goToSystem()">
				<div class="ps-1 banner-title d-flex flex-column me-2">
					<#General_x_SystemTime_itemname#>
					<small class="banner-time-meridiem">${system.time.weekday}</small>
				</div>
				<div class="font-weight-bold banner-time my-auto" id="sys_time">
					<span>${system.time.current}</span>
				</div>
			</div>`:``}
			
		</div>	
	`;

	return code;
}

function genNotification() {
	let code = "";
	code += `
		<div role="btn-notification" class="d-flex align-items-center mx-3">
			<div class="dropdown ms-2">
				<div role="icon" class="icon-size-24 icon-notification"
					data-bs-toggle="dropdown"
					aria-expanded="false"
				>
				</div>
				<ul class="dropdown-menu" id="notification-list"></ul>
			</div>
		</div>	
	`;

	return code;
}

function genLogout() {
	let code = "";
	code += `
		<div role="btn-logout" class="d-flex align-items-center mx-3" onclick="logout();">
			<div role="icon" class="icon-size-24 icon-logout"></div>
			<div role="info-text" class="ps-2 logout-title"><#t1Logout#></div>
		</div>	
	`;

	return code;
}

function genReboot() {
	let code = "";
	code += `	
		<div role="btn-reboot" class="d-flex align-items-center mx-3" onclick="reboot();">
			<div role="icon" class="icon-size-24 icon-reboot"></div>
			<div role="info-text" class="ps-2 reboot-title"><#BTN_REBOOT#></div>
		</div>
	`;

	return code;
}

function genLanguageList() {
	let code = "";
	let { currentLang, supportList } = system.language;
	code += `
		<div role="btn-language" class="d-none d-sm-flex align-items-center mx-3">
			<div role="icon" class="icon-size-24 icon-language"></div>
			<div class="dropdown ms-2">
				<button
					class="btn dropdown-toggle p-1 lang-title"
					type="button"
					id="dropdownLangMenu"
					data-bs-toggle="dropdown"
					aria-expanded="false"
				>
					${supportList[currentLang]}
				</button>
		`;
	code += `<ul class="dropdown-menu">`;

	for (let [key, value] of Object.entries(supportList)) {
		if (key === currentLang) continue;

		code += `
			<li onclick="changeLanguage('${key}')">
				<a class="dropdown-item lang-item" href="#">${value}</a>
			</li>
		`;
	}

	code += `</ul>`;
	code += `
			</div>
		</div>
	`;

	return code;
}
const menuSDNNaming = (() => {
	if (isSupport("sdn_mwl")) return `<#Network#>`;
	if (!isSupport("mtlancfg")) return `<#Guest_Network#>`;
	if (isSupport("BUSINESS")) return `<#GuestNetwork_SDN_title#>`;
	if (isSupport("SMART_HOME_MASTER_UI")) return `<#Guest_Network#>`;
	return `<#GuestNetwork_PRO_title#>`;
})();
var menuList = [
	{
		name: `<#QIS#>`,
		icon: "icon-qis",
		url: "QIS_wizard.htm",
		clicked: false,
		divide: false,
	},
	{
		name: `<#AiProtection_title_Dashboard_title#>`,
		icon: "icon-dashboard",
		url: "dashboard",
		clicked: false,
		divide: false,
	},

	{
		name: `AiMesh`,
		icon: "icon-network",
		url: "aimesh",
		clicked: false,
		divide: false,
	},
	{
		name: `Clients`,
		icon: "icon-client-list",
		url: "clients",
		clicked: false,
		divide: false,
	},
	{
		name: `Adaptive QoE`,
		icon: "icon-AdaptiveQoe",
		url: "qoe",
		clicked: false,
		divide: false,
	},
	{
		name: menuSDNNaming,
		icon: "icon-SDN",
		url: "sdn",
		clicked: false,
		divide: false,
	},
	{
		name: `VPN`,
		icon: "icon-VPN",
		url: "vpn",
		clicked: false,
		divide: false,
	},
	{
		name: `<#Traffic_Analyzer#>`,
		icon: "icon-TrafficAnalyzer",
		url: "trafficanalyzer",
		clicked: false,
		divide: false,
	},
	{
		name: `<#AiProtection_title#>`,
		icon: "icon-AiProtection",
		url: "aiprotection",
		clicked: false,
		divide: false,
	},
	{
		name: `<#Parental_Control#>`,
		icon: "icon-ParentalControl",
		url: "parentalcontrol",
		clicked: false,
		divide: false,
	},
    	{
		name: `<#Game_acceleration#>`,
		icon: "icon-GameAcceleration",
		url: "game_acceleration",
		clicked: false,
		divide: false,
	},
	{
		name: `AI Board`,
		icon: "icon-AiBoard",
		url: "aiboard",
		clicked: false,
		divide: false,
	},
	{
		name: `<#traffic_monitor#>`,
		icon: "icon-TrafficMonitor",
		url: "trafficmonitor",
		clicked: false,
		divide: false,
	},
	{
		name: "<#Settings#>",
		icon: "icon-advancedsettings",
		url: "settings",
		clicked: false,
		divide: false,
	}	 
	// {
	//	 name: "Help & Support",
	//	 icon: "icon-help",
	//	 url: "",
	//	 clicked: false,
	//	 divide: true,
	// },
];

let urlParameter = new URLSearchParams(window.location.search);
/* DECIDE THEME */
let theme = (function () {
	if (isSupport("rog"))
		return "rog";
	else if (isSupport("tuf"))
		return "tuf";
	else if (isSupport("BUSINESS"))
		return "business";
	else
		return "asus";
})();

if (isSupport("UI4")) {
	document.querySelector("html").setAttribute("data-asuswrt-theme", theme);
	document.querySelector("html").setAttribute("data-asuswrt-color", "light");
}
if(isSupport("ROG_UI")) {
    let asuswrtStyle = window.localStorage.getItem("asuswrt-style");
    if(asuswrtStyle) {
        document.querySelector("html").setAttribute("data-asuswrt-style", asuswrtStyle);
    }else {
        document.querySelector("html").setAttribute("data-asuswrt-style", "tech");
    }
}

if(!isSupport("UI4")){
	menuList = menuList.filter(function(item, index, array){
		return (item.url != "settings") && (item.url != "QIS_wizard.htm");
	});
}

if (!(isSupport("gtbooster") && isSupport("ark_qoe"))) {
	menuList = menuList.filter(function(item, index, array){
		return (item.url != "qoe");
	});
}

if(!isSupport("traffic_analyzer")){
	menuList = menuList.filter(function(item, index, array){
		return (item.url != "trafficanalyzer");
	});
}

if(!isSupport("bwdpi")){
	menuList = menuList.filter(function(item, index, array){
		return (item.url != "trafficanalyzer") && (item.url != "aiprotection");
	});
}

if(!isSupport("ai_support")){
	menuList = menuList.filter(function(item, index, array){
		return (item.url != "aiboard");
	});
}

if (!isSupport("rog")) {
    menuList = menuList.filter(function (item, index, array) {
        return (item.url != "game_acceleration");
    });
}

if(system.currentOPMode.id != "RT"){
	var menuList = [
		{
			name: `<#QIS#>`,
			icon: "icon-qis",
			url: "QIS_wizard.htm",
			clicked: false,
			divide: false,
		},
		{
			name: `<#AiProtection_title_Dashboard_title#>`,
			icon: "icon-dashboard",
			url: "dashboard",
			clicked: false,
			divide: false,
		},
		{
			name: `AiMesh`,
			icon: "icon-network",
			url: "aimesh",
			clicked: false,
			divide: false,
		},
		{
			name: menuSDNNaming,
			icon: "icon-SDN",
			url: "sdn",
			clicked: false,
			divide: false,
		},
		{
			name: `<#traffic_monitor#>`,
			icon: "icon-TrafficAnalyzer",
			url: "trafficmonitor",
			clicked: false,
			divide: false,
		},		
		{
			name: "<#Settings#>",
			icon: "icon-advancedsettings",
			url: "settings",
			clicked: false,
			divide: false,
		}	 
	]

	if(system.currentOPMode.id != "AP"){
		menuList = menuList.filter(function(item, index, array){
			return (item.url != "aimesh");
		});
	}
}

if (navigator.userAgent.match(/ASUSMultiSiteManager/) || navigator.userAgent.match(/ASUSExpertSiteManager/) || (/^aae-sgrst001-\w+\.asuscomm\.com$/.test(window.location.hostname))) {
	menuList = menuList.filter(item => item.url !== "QIS_wizard.htm");
}

function genNavMenu() {
	let menuListClicked = urlParameter.get("page");
	if(!menuListClicked){
		menuListClicked = window.localStorage.getItem("page") || "dashboard";
	}
	for(var i=0; i<menuList.length; i++){
		menuList[i].clicked = false;
		if(menuList[i].url.indexOf(menuListClicked) != -1) menuList[i].clicked = true;
	}

	let nav = document.getElementsByTagName("nav")[0];

	// for mobile layout, show the close icon
	let code = `
		<div role="menu-close">
			<div role="icon" class="icon-size-36 icon-close mt-2 mx-2" id="mobile_menu_close"></div>
		</div>
	`;

	for (let i = 0; i < menuList.length; i++) {
		let list = menuList[i];
		if (list.divide) {
			code += `<div class="ms-4 me-2 divide-menu"></div>`;
		}

		code += `
			<ul class="list-unstyled mb-0">
				<li role="menu">
					<button
						data-bs-toggle="tooltip" data-bs-placement="right" title="${list.name}"
						class="btn btn-toggle manu-${list.url} nav-menu ${list.clicked ? "menu-clicked" : ""}"
						onclick="pageRedirect('${list.url}')"
					>
						<div role="icon" class="icon-size-24 ${list.icon} icon-color-menu"></div>
						<div class="menu-name">${list.name}</div>
					</button>
				</li>
			</ul>
		`;
	}

	code += `
		<div class="mb-3 p-2 copyright">2022 ASUSTeK Computer Inc. All rights reserved.</div>
	`;

	nav.innerHTML = code;

	const tooltipTriggerList = document.querySelectorAll('[data-bs-toggle="tooltip"]')
	const tooltipList = [...tooltipTriggerList].map(tooltipTriggerEl => new bootstrap.Tooltip(tooltipTriggerEl, {trigger: 'hover', customClass:'tooltip-menu', container:'.sidebar'}));

	document.querySelectorAll(".menu-header").forEach((item) => {
		item.addEventListener("click", function () {
			document.querySelector("aside.sidebar").classList.add("sidebar-show");
		});
	});

	document.getElementById("mobile_menu_close").addEventListener("click", function () {
		document.querySelector("aside.sidebar").classList.remove("sidebar-show");
	});
}

var sysTimeTimer = 0;
function genMidHeader() {
	let banner = document.querySelector('[role~="info-banner"]');
	let code = "";

	code += genTimeBanner();
	code += genOpMode();
	code += genFirmwareVersion();
	code += genSSID();
	// code += genStatusBanner();

	banner.innerHTML = code;

	//trigger Time counting
	if(sysTimeTimer == 0){
		sysTimeTimer = setInterval(function () {
			system.time = getTime();
			if(document.getElementById("sys_time")) {
				document.getElementById("sys_time").innerHTML = `
				<span>${system.time.current}</span>
			`;
			}
		}, 1000);
	}
}

function genTimeBanner() {
	let code = "";
	code += `
		<div role="time-banner" class="mx-2 my-1 px-2 py-1">
			<div class="ps-1 banner-title"><#General_x_SystemTime_itemname#></div>
			<div class="font-weight-bold banner-time" id="sys_time">
				<span>${system.time.current}</span>
			</div>
		</div>
	`;

/*
	code += `
		<div role="time-banner" class="mx-2 my-1 px-2 py-1">
			<div class="ps-1 banner-title"><#General_x_SystemTime_itemname#></div>
			<div class="font-weight-bold banner-time" id="sys_time">
				<span>${system.time.current}</span>
				<small class="banner-time-meridiem">${system.time.weekday}</small>
			</div>
		</div>
	`;
*/
	return code;
}

function genOpMode() {
	let code = "";
	code += `
		<div role="op-mode-banner" class="mx-2 my-1 p-1">
			<div class="border-left-title ps-3 text-truncate banner-title"><#menu5_6_1_title#></div>
			<div class="font-weight-bold banner-text pt-2 ps-3 text-truncate">${system.currentOPMode.desc}</div>
		</div>
	`;

	return code;
}

function genFirmwareVersion() {
	let code = "";
	code += `
		<div role="fw-ver-banner" class="mx-2 my-1 p-1">
			<div class="border-left-title ps-3 text-truncate banner-title">
				<#General_x_FirmwareVersion_itemname#>
			</div>
			<div class="font-weight-bold banner-text pt-2 ps-3">${system.firmwareVer.number}</div>
		</div>
	`;

	return code;
}

function genSSID() {
	let code = "";
	code += `
		<div role="ssid-banner" class="mx-2 my-1 p-1">
			<div class="border-left-title ps-3 banner-title">SSID</div>
			<div class="d-flex font-weight-bold pt-2 ps-3 banner-text">
	`;

	for (let i = 0; i < Object.values(wireless).length; i++) {
		code += `<span class="text-truncate px-2">${Object.values(wireless)[i].ssid}</span>`;
	}

	code += `
			</div>
		</div>
	`;

	return code;
}

function genFooter() {}

function changeLanguage(lang) {
	httpApi.nvramSet(
		{
			action_mode: "apply",
			rc_service: "email_info",
			flag: "set_language",
			preferred_lang: lang,
		},
		function () {
			setTimeout(function () {
				location.reload();
			}, 10);
		}
	);
}

function reboot() {
	var rebootTime = httpApi.hookGet("get_default_reboot_time");
	var FbState = httpApi.nvramGet(["fb_state"], true).fb_state;
	var FbNote = `<#feedback_note5#>`;

	if (FbState == "0") {
		alert(FbNote);
	}
	else {
		if (confirm("<#Main_content_Login_Item7#>")) {
			applyLoading(rebootTime, "<#SAVE_restart_desc#>");
			setTimeout(function(){location.reload();}, rebootTime*1000);
			httpApi.nvramSet({ action_mode: "reboot" });
		}
	}
}

function logout() {
	if (confirm("<#JS_logout#>")) {
		setTimeout('location = "../Logout.asp";', 1);
	}
}

function goToSystem() {
	pageRedirect("settings", "Advanced_System_Content.asp");
}

function checkAiBoardInOtaOrRescue() {
    const _nvram = httpApi.nvramGet(['ai_prog_status', 'ai_fw_path', 'ai_rescue_ts']);
    const {ai_prog_status, ai_fw_path, ai_rescue_ts} = _nvram;

    let in_three_hours = false;
    if (ai_rescue_ts) {
        const currentTime = Math.floor(Date.now() / 1000);
        const timeDiff = currentTime - parseInt(ai_rescue_ts, 10);
        in_three_hours = timeDiff < 3 * 60 * 60; // 3 hours in seconds
    }
    const excludeStatuses = ["failed", "DOCKER_DONE", "DONE", ""];
    const includePaths = ["firmware/aisom_reset.swu", "firmware/aisom.swu"];
    return !excludeStatuses.includes(ai_prog_status) && includePaths.includes(ai_fw_path) && in_three_hours;
}

function checkAiBoardStatusAndRedirect() {
    if (checkAiBoardInOtaOrRescue() && !top.location.href.includes('aiboard')) {
        window.location.href = '?page=aiboard';
    }
}

checkAiBoardStatusAndRedirect();

function pageRedirect(target, lastPage) {

    checkAiBoardStatusAndRedirect();

	if (target !== "QIS_wizard.htm") {
		window.localStorage.setItem("page", target)
	}

	if(target){
		Session.set("target", target);
	}
	
	if(lastPage){
		Session.set("lastPage", lastPage);
	}

	if (target === "index.asp") {
		location.href = target;
	}
	else if (target == "QIS_wizard.htm") {
		location.href = "/QIS_wizard.htm";
	}
	else {
		var interval_id = window.setInterval(function(){}, 9999);
		for (var i = 1; i <= interval_id; i++) {if(i != sysTimeTimer)window.clearInterval(i);}
		setTimeout(checkNotification, 3000);
		$(`.nav-menu`).removeClass("menu-clicked");
		$(`.manu-${target}`).addClass("menu-clicked");
		$("main").load(`./pages/${target}.html`, function(){
			setTimeout(function(){
				const styleSheets = document.styleSheets;
				const retryLimit = 3;
				const retriedStylesheets = new Map();
				for (let i = 0; i < styleSheets.length; i++) {
					const stylesheet = styleSheets[i];
					if (stylesheet.href) {
						try {
							if (!stylesheet.cssRules) {
								throw new Error('CSS rules not accessible');
							}
						} catch (e) {
							const { pathname } = new URL(stylesheet.href);
							const retries = retriedStylesheets.get(stylesheet.href) || 0;
							if (retries < retryLimit) {
								retriedStylesheets.set(stylesheet.href, retries + 1);
								const link = document.createElement('link');
								link.rel = 'stylesheet';
								link.type = 'text/css';
								link.href = pathname;
								document.head.appendChild(link);
								if(typeof httpApi === "object")
									httpApi.log("CSS ERR_TOO_MANY_RETRIES", `[Append CSS again] Request: ${pathname}, Page: ${window.location.pathname}`);
							}
							else {
								if(typeof httpApi === "object")
									httpApi.log("CSS ERR_TOO_MANY_RETRIES", `[Retries fail: ${retryLimit}] Request: ${pathname}, Page: ${window.location.pathname}`);
							}
						}
					}
				}
			}, 1000);
		});
		document.querySelector("aside.sidebar").classList.remove("sidebar-show");
	}
}

var ntCallbackEvent = [];
function checkNotification(){
	import('/js/dashboard-nt.js')
		.then(({ getNotifications }) => {
			var notification = getNotifications();

			if(notification.length > 0){
				var code = "";
				for (var i in notification) {
					code += `
						<li class="notification-item" data-id="${notification[i].id}" onclick="ntCallbackEvent['${notification[i].id}']()">
							<div class="notification-item">
								<div role="icon" class="icon-${notification[i].id} notification-icon icon-event-default"></div>
								<div class="notification-content">
									<div class="app-name">${notification[i].title}</div>
									<div class="notification-text">${notification[i].description}</div>
								</div>
							</div>
						</li>
					`;
	
					ntCallbackEvent[notification[i].id] = notification[i].callback; 
				}

				var notificationBtn = document.querySelector('div[role="btn-notification"]');
				if (notificationBtn) {
					document.getElementById("notification-list").innerHTML = code;	
					notificationBtn.style.visibility = 'visible';
				}
			}
		})

	setTimeout(checkNotification, 3000);
}

function applyLoading(time, string, callback) {
	let template = `
		<div class="d-flex justify-content-center mt-5">
			<div class="card-float">
				<div class="card-header-float">${string}</div>
				<div class="card-body-float">
					<div class="d-flex align-items-center mt-3">
						<div class="spinner-border loader-circle-color spinner-border-lg" role="status" aria-hidden="true"></div>
						<div class="fs-4 fw-bold ms-4" id="loading_text"><#Main_alert_proceeding_desc1#>...</div>
					</div>										
				</div>	 
			</div>
		</div>
	`;

	let element = document.createElement("div");
	element.className = "shadow-bg modal-backdrop";
	element.innerHTML = template;
	document.body.appendChild(element);

	let count = 1;
	let counter = function (time) {
		let value = Math.round((count / time) * 100);
		document.getElementById("loading_text").innerHTML = value + "%";
		count++;

		if (value > 100) {
			if (callback) {callback();}
			document.body.removeChild(element);
		} else {
			setTimeout(function(){counter(time);}, 1000);
		}
	};

	counter(time);
}

function showConfirmDialog(message, options = {}) {
	const { onConfirm, onCancel } = options;

	const oldConfirm = document.getElementById('customConfirmDialog');
	if (oldConfirm) oldConfirm.parentNode.removeChild(oldConfirm);

	const element = document.createElement("div");
	element.id = 'customConfirmDialog';
	element.className = "shadow-bg modal-backdrop";
	element.setAttribute('role', 'dialog');
	element.setAttribute('aria-modal', 'true');
	element.setAttribute('aria-describedby', 'confirmDialogMessage');
	element.innerHTML = `
		<div class="card-float" style="position:absolute;left:50%;top:10%;transform:translateX(-50%);max-width:500px;width:90vw;">
			<div class="card-body-float">
				<div class="d-flex align-items-center justify-content-center mb-3">
					<div id="confirmDialogMessage" class="fs-6 w-100" role="alert">${message}</div>
				</div>
				<div class="d-flex justify-content-end gap-2">
					<button type="button" class="btn btn-outline-primary btn-sm" id="customConfirmNo" aria-label="<#CTL_Cancel#>"><#CTL_Cancel#></button>
					<button type="button" class="btn btn-primary btn-sm" id="customConfirmYes" aria-label="<#CTL_ok#>" autofocus><#CTL_ok#></button>
				</div>
			</div>
		</div>
	`;
	document.body.appendChild(element);

	const previousActiveElement = document.activeElement;
	const confirmButton = document.getElementById('customConfirmYes');
	const cancelButton = document.getElementById('customConfirmNo');

	element.addEventListener('keydown', function(e) {
		if (e.key === 'Tab') {
			e.preventDefault();
			if (e.shiftKey) {
				// Shift+Tab
				if (document.activeElement === confirmButton) {
					cancelButton.focus();
				} else {
					confirmButton.focus();
				}
			} else {
				// Tab
				if (document.activeElement === cancelButton) {
					confirmButton.focus();
				} else {
					cancelButton.focus();
				}
			}
		} else if (e.key === 'Escape') {
			// ESC
			if (onCancel) onCancel();
			document.body.removeChild(element);
			if (previousActiveElement) previousActiveElement.focus();
		}
	});

	confirmButton.onclick = () => {
		if (onConfirm) onConfirm();
		document.body.removeChild(element);
		if (previousActiveElement) previousActiveElement.focus();
	};

	cancelButton.onclick = () => {
		if (onCancel) onCancel();
		document.body.removeChild(element);
		if (previousActiveElement) previousActiveElement.focus();
	};
}

let oInactivityTimerId;
const http_autologout = parseInt(httpApi.nvramGet(['http_autologout']).http_autologout);
if (http_autologout != 0) {
	const resetInactivityTimer = () => {
		//if is asusrouter app, do not auto logout
		if(navigator.userAgent.search("asusrouter") != -1) return false;

        clearTimeout(oInactivityTimerId);
        oInactivityTimerId = setTimeout(() => {
            if (!checkAiBoardInOtaOrRescue()) {
                location.href = '/Logout.asp';
            }
        }, http_autologout * 60 * 1000);
    }

	document.addEventListener("DOMContentLoaded", () => {
		document.addEventListener("mousemove", resetInactivityTimer);
		document.addEventListener("keydown", resetInactivityTimer);
		resetInactivityTimer();
	});

	window.addEventListener('message', (event) => {
		if (event.data === 'iframeMouseMove') {
			resetInactivityTimer();
		}
	});
}

// Router Assistant functionality
function autoResizeTextarea(textarea) {
    // Store the current scroll position
    const scrollTop = textarea.scrollTop;
    
    // Temporarily set to auto to measure the natural height
    textarea.style.height = 'auto';
    
    // Get the natural content height
    const scrollHeight = textarea.scrollHeight;
    const minHeight = 38;
    const maxHeight = 120;
    
    // Calculate the appropriate height
    let newHeight;
    if (scrollHeight <= minHeight) {
        newHeight = minHeight;
    } else if (scrollHeight >= maxHeight) {
        newHeight = maxHeight;
    } else {
        newHeight = scrollHeight;
    }
    
    // Apply the calculated height
    textarea.style.height = newHeight + 'px';
    textarea.style.marginTop = (newHeight - 38) + 'px';
    
    // Restore scroll position
    textarea.scrollTop = scrollTop;
    
    // Handle overflow
    if (scrollHeight > maxHeight) {
        textarea.style.overflowY = 'auto';
    } else {
        textarea.style.overflowY = 'hidden';
    }
}

function handleRouterAssistantInput(event) {
	if(!isSupport("ai_board_slm")) return false;

	const textarea = event.target;

	// Handle Enter key - but allow Shift+Enter for new lines
	if (event.key === 'Enter' && !event.shiftKey) {
		event.preventDefault(); // Prevent default new line behavior

		const inputValue = textarea.value.trim();
		if (!inputValue) return;

		// Show loading in response area with user question
		showRouterAssistantResponse('loading', inputValue);

		// Call the AI API
		setTimeout(() => {
			httpApi.ai_board_slm({"question": inputValue}, function(response) {
				showResult(response, inputValue);
			});
		}, 500);

		// Clear textarea and reset height
		textarea.value = '';
		autoResizeTextarea(textarea);
	}
	// Allow other keys to work normally, including Shift+Enter for new lines
}
const metadataList = [
	{ id: "10001", desc: "11b_enable", preActionSentence: "About to enable 802.11b mode. Do you want to do it now?" },
	{ id: "10002", desc: "11b_disable", preActionSentence: "About to disable 802.11b mode. Do you want to do it now?" },
	{ id: "10003", desc: "reboot", preActionSentence: "About to reboot the router. Do you want to do it now?" },
	{ id: "10004", desc: "qos_game_mode_on", preActionSentence: "About to enable Game Mode. Do you want to do it now?" },
	{ id: "10005", desc: "qos_game_mode_off", preActionSentence: "About to disable Game Mode. Do you want to do it now?" },
	{ id: "10006", desc: "qos_media_mode_on", preActionSentence: "About to enable Media Mode. Do you want to do it now?" },
	{ id: "10007", desc: "qos_media_mode_off", preActionSentence: "About to disable Media Mode. Do you want to do it now?" },
	{ id: "10008", desc: "system_led_on", preActionSentence: "About to turn on the LED light. Do you want to do it now?" },
	{ id: "10009", desc: "system_led_off", preActionSentence: "About to turn off the LED light. Do you want to do it now?" },
	{ id: "10010", desc: "enable_portal", preActionSentence: "About to enable guest portal. Do you want to do it now?" },
	{ id: "10011", desc: "disable_portal", preActionSentence: "About to disable guest portal. Do you want to do it now?" },
	{ id: "10012", desc: "get_client_num", preActionSentence: "Current number of connected devices:" },
	{ id: "10013", desc: "firmware_update", preActionSentence: "Redirecting to firmware update page:" },
	{ id: "10014", desc: "pause_internet", preActionSentence: "About to pause the internet. Do you want to do it now?" },
	{ id: "10015", desc: "resume_internet", preActionSentence: "About to resume the internet. Do you want to do it now?" },
	{ id: "10016", desc: "get_cpu_ram_info", preActionSentence: "CPU/RAM info:" },
	{ id: "10017", desc: "get_wan_type", preActionSentence: "Query WAN type:" },
	{ id: "10018", desc: "get_wan_ip", preActionSentence: "Query WAN IP:" },
	{ id: "10019", desc: "get_system_time", preActionSentence: "Query router time:" },
	{ id: "10020", desc: "get_firmware_version", preActionSentence: "Query firmware version:" },
	{ id: "10021", desc: "feedback", preActionSentence: "About to enter Feedback page. Do you want to do it now?" },
	{ id: "10022", desc: "get_system_status", preActionSentence: "System status:" },
	{ id: "10023", desc: "share_wifi", preActionSentence: "About to generate Wi-Fi sharing QR code. Do you want to do it now?" },
	{ id: "10024", desc: "wifi_setting", preActionSentence: "Entering Wi-Fi settings page" },
	{ id: "10025", desc: "vpn_setting", preActionSentence: "Entering VPN settings page" },
	{ id: "10026", desc: "op_mode", preActionSentence: "Entering operation mode" },
	{ id: "10027", desc: "wan_setting", preActionSentence: "Entering WAN settings page" },
	{ id: "10028", desc: "wifi_txop_status", preActionSentence: "Here are the WiFi status:" },
	{ id: "10031", desc: "qos_wfh_mode_on", preActionSentence: "Enable WFH Mode. Do you want to do it now?" },
	{ id: "10032", desc: "qos_wfh_mode_off", preActionSentence: "Disable WFH Mode. Do you want to do it now?" },
	{ id: "10033", desc: "get_aiboard_ip", preActionSentence: "Here is the IP address:" }
];
function extractMetadata(content) {
	const regex = /\[metadata\]id=(\d+),desc=([^\]]+)\[\/metadata\]/g;
	const result = [];
	let match;
	while ((match = regex.exec(content)) !== null) {
		const meta = metadataList.find(item => item.id === match[1]);
		if (meta) {
			result.push(meta);
		} else {
			result.push({ id: match[1], desc: match[2], preActionSentence: "" });
		}
	}
	return result;
}
function showResult(response, userQuestion) {
	let responseText = "";
	if (response.data) {
		responseText = response.data;
	}
	else {
		responseText = `Unable to connect at the moment. Please refresh or check your network and try again. If problem persist, please try to reboot the router.`;
	}
	showRouterAssistantResponse(responseText, userQuestion);
	const responseDiv = document.querySelector('[data-component="router-assistant-response"]');
	const actionBtns = document.querySelectorAll('[data-group="router-assistant-action"]');
	actionBtns.forEach(btn => {
		btn.style.display = 'none';
	});
	const cancelBtn = document.querySelector('[data-group="router-assistant-action"][data-component="cancelBtn"]');
	const okBtn = document.querySelector('[data-group="router-assistant-action"][data-component="okBtn"]');
	let filteredContent = responseText;
	const isLowPercent = (function(str) {
		const match = str.match(/\((\d+(?:\.\d+)?)%\)/);
		const percent = match ? parseInt(match[1], 10) : null;
		return percent !== null && percent < 20;
	})(filteredContent);
	const metadataList = extractMetadata(responseText);
	if (typeof filteredContent === 'string') {
		filteredContent = filteredContent.replace(/\[metadata\][^\[]*?\[\/metadata\]/g, '');
		filteredContent = filteredContent.replace(/\s*\([^\)]*%\)\s*$/g, '');
		if (metadataList.length > 0) {
			const preActionHtml = metadataList
				.filter(meta => meta.preActionSentence && meta.preActionSentence.trim() !== "")
				.map(meta => `<div class=\"router-assistant-preaction\">${meta.preActionSentence}</div>`)
				.join('');
			filteredContent += preActionHtml;
		}
		responseDiv.innerHTML = filteredContent;
	}

	if (isLowPercent) {
		responseDiv.innerHTML = `Sorry, unable to answer the question.`;
		if (cancelBtn) {
			cancelBtn.style.display = '';
			cancelBtn.textContent = `<#CTL_ok#>`;
		}
	}
	else if (metadataList.length == 0) {
		if (cancelBtn) {
			cancelBtn.style.display = '';
			cancelBtn.textContent = `<#CTL_ok#>`;
		}
	}
	else {
		metadataList.forEach(meta => {
			switch (meta.id) {
				case "10001":
				case "10002": {
					// // 11b_enable, 11b_disable
					if (cancelBtn) {
						cancelBtn.style.display = '';
					}
					if (okBtn) {
						okBtn.style.display = '';
						okBtn.textContent = `<#btn_go#>`;
						okBtn.onclick = function() {
							pageRedirect('sdn', 'sdn.html');
							closeRouterAssistantResponse();
						};
					}
					break;
				}
				case "10003": {
					// reboot
					if (cancelBtn) {
						cancelBtn.style.display = '';
					}
					if (okBtn) {
						okBtn.style.display = '';
						okBtn.onclick = function() {
							const rebootTime = httpApi.hookGet("get_default_reboot_time");
							const FbState = httpApi.nvramGet(["fb_state"], true).fb_state;
							const FbNote = `<#feedback_note5#>`;
							if (FbState == "0") {
								alert(FbNote);
							}
							else {
								closeRouterAssistantResponse();
								showConfirmDialog(`<#Main_content_Login_Item7#>`, {
									onConfirm: () => {
										applyLoading(rebootTime, "<#SAVE_restart_desc#>");
										setTimeout(function(){location.reload();}, rebootTime*1000);
										httpApi.nvramSet({ action_mode: "reboot" });
									}
								});
							}
						};
					}
					break;
				}
				case "10004":
				case "10006":
				case "10031":{
					// qos_game_mode_on, qos_media_mode_on, qos_wfh_mode_on
					let profile = "ai";
					if (meta.id === "10004") profile = "gaming";
					if (meta.id === "10006") profile = "streaming";
					if (meta.id === "10031") profile = "wfh";
					if (cancelBtn) {
						cancelBtn.style.display = '';
					}
					if (okBtn) {
						okBtn.style.display = '';
						okBtn.onclick = function() {
							let apply_obj = {
								"action_mode": "apply",
								"rc_service": "restart_ark",
								"ark_qoe_enable": "1",
								"ark_qoe_profile": profile,
							};
							const support_ark_qoe_profile = [
								{"name": "ai", "value": 0},
								{"name": "gaming", "value": meta.id === "10004" ? 10 : 0},
								{"name": "streaming", "value": meta.id === "10006" ? 10 : 0},
								{"name": "wfh", "value": meta.id === "10031" ? 10 : 0},
								{"name": "office", "value": 0}
							];
							apply_obj["ark_qoe_psettings"] = support_ark_qoe_profile.map(profile => profile.value).join("<");
							httpApi.nvramSet(apply_obj);
							closeRouterAssistantResponse();
							showLoading(3);
							setTimeout(() => {
								let reloadInterval = setInterval(function(){
									if (typeof businessLoader !== 'undefined' && !businessLoader.isShow()) {
										pageRedirect('qoe', 'qoe.html');
										clearInterval(reloadInterval);
									}
								}, 10);
							}, 3000);
						};
					}
					break;
				}
				case "10005":
				case "10007":
				case "10032": {
					// qos_game_mode_off, qos_media_mode_off, qos_wfh_mode_off
					if (cancelBtn) {
						cancelBtn.style.display = '';
					}
					if (okBtn) {
						okBtn.style.display = '';
						okBtn.onclick = function() {
							let apply_obj = {
								"action_mode": "apply",
								"rc_service": "restart_ark",
								"ark_qoe_enable": "0",
							}
							httpApi.nvramSet(apply_obj);
							closeRouterAssistantResponse();
							showLoading(3);
							setTimeout(() => {
								let reloadInterval = setInterval(function(){
									if (typeof businessLoader !== 'undefined' && !businessLoader.isShow()) {
										pageRedirect('qoe', 'qoe.html');
										clearInterval(reloadInterval);
									}
								}, 10);
							}, 3000);
						};
					}
					break;
				}
				case "10008":
				case "10009": {
					// system_led_off, system_led_on
					if (cancelBtn) {
						cancelBtn.style.display = '';
					}
					if (okBtn) {
						okBtn.style.display = '';
						okBtn.onclick = function() {
							const system_settings_led = {"led": false};
							const get_cfg_clientlist = httpApi.hookGet("get_cfg_clientlist");
							const capObj = get_cfg_clientlist[0];
							if(capObj != undefined){
								const node_capability = httpApi.aimesh_get_node_capability(capObj);
								if(node_capability?.central_led_on_off){
									system_settings_led.led = capObj?.config?.ctrl_led?.led_val !== undefined;
								}
							}
							if(system_settings_led.led){
								const node_led_count = (()=>{
									let result = {"led_on": 0, "led_off": 0, "led_night_mode_on": 0, "led_night_mode_off": 0, "not_support_list": []};
									get_cfg_clientlist.forEach(node => {
										if(node.config?.ctrl_led?.led_val !== undefined){
											if(node.config.ctrl_led.led_val === "1")
												result.led_on++;
											else
												result.led_off++;
										}
										if(node.config?.misc?.single_led_night_mode !== undefined){
											if(node.config.misc.single_led_night_mode === "1")
												result.led_night_mode_on++;
											else
												result.led_night_mode_off++;
										}
										const node_capability = httpApi.aimesh_get_node_capability(node);
										if(node_capability.central_led_on_off === false){
											const node_led_status = {
												"mac": node.mac,
												"led_on_off": node_capability.led_on_off && node?.config?.ctrl_led?.led_val !== undefined
											};
											result.not_support_list.push(node_led_status);
										}
									});
									return result;
								})();
								const switchLedStatus = meta.id === "10008" ? "1" : "0";
								node_led_count.not_support_list.forEach((node) => {
									if (node.led_on_off) {
										if (node.mac != "") {
											httpApi.nvramSet({
												"config": JSON.stringify({ "led_val": switchLedStatus }),
												"re_mac": node.mac,
												"action_mode": "config_changed"
											});
										}
									}
								});
								setTimeout(() => {
									const postData = {
										"led_val": switchLedStatus,
										"rc_service": "no_service",
										"action_mode": "apply"
									};
									httpApi.nvramSet(postData);
								}, 500);
							}
							closeRouterAssistantResponse();
						};
					}
					break;
				}
				case "10010": {
					// enable_portal
					if (cancelBtn) {
						cancelBtn.style.display = '';
					}
					if (okBtn) {
						okBtn.style.display = '';
						okBtn.textContent = `<#btn_go#>`;
						okBtn.onclick = function() {
							pageRedirect('sdn', 'sdn.html');
							closeRouterAssistantResponse();
						};
					}
					break;
				}
				case "10011": {
					// disable_portal
					if (cancelBtn) {
						cancelBtn.style.display = '';
					}
					if (okBtn) {
						okBtn.style.display = '';
						okBtn.textContent = `<#btn_go#>`;
						okBtn.onclick = function() {
							pageRedirect('sdn', 'sdn.html');
							closeRouterAssistantResponse();
						};
					}
					break;
				}
				case "10012": {
					// get_client_num
					let nmp = httpApi.hookGet("get_clientlist", true);
					let totalClient = 0;
					for (let [key, value] of Object.entries(nmp)) {
						if (key === "maclist" || key === "ClientAPILevel") {
							continue;
						}
						if (nmp[key] !== undefined) {
							if (nmp[key].isOnline === "1" && nmp[key].amesh_isRe !== "1" && nmp[key].isAiBoard === "0") {
								totalClient++;
							}
						}
					}
					responseDiv.textContent += ` ${totalClient}`;
					if (cancelBtn) {
						cancelBtn.style.display = '';
						cancelBtn.textContent = `<#CTL_ok#>`;
					}
					break;
				}
				case "10013": {
					// firmware_update
					if (cancelBtn) {
						cancelBtn.style.display = '';
					}
					if (okBtn) {
						okBtn.style.display = '';
						okBtn.textContent = `<#btn_go#>`;
						okBtn.onclick = function() {
							pageRedirect('settings', 'Advanced_FirmwareUpgrade_Content.asp');
							closeRouterAssistantResponse();
						};
					}
					break;
				}
				case "10014": {
					// pause_internet
					if (cancelBtn) {
						cancelBtn.style.display = '';
					}
					if (okBtn) {
						okBtn.style.display = '';
						okBtn.textContent = `<#btn_go#>`;
						okBtn.onclick = function() {
							pageRedirect('sdn', 'sdn.html');
							closeRouterAssistantResponse();
						};
					}
					break;
				}
				case "10015": {
					// resume_internet
					if (cancelBtn) {
						cancelBtn.style.display = '';
					}
					if (okBtn) {
						okBtn.style.display = '';
						okBtn.textContent = `<#btn_go#>`;
						okBtn.onclick = function() {
							pageRedirect('sdn', 'sdn.html');
							closeRouterAssistantResponse();
						};
					}
					break;
				}
				case "10016": {
					// get_cpu_ram_info
					httpApi.get_diag_content_data(
						{
							ts: Date.now(),
							duration: "60",
							point: "2",
							db: "sys_detect",
							content: "cpu_usage;mem_usage",
							filter: "node_mac>txt>" + system.labelMac + ">0",
						},
						(e) => {
							let cpuData = [],
								ramData = [];
							for (let item of e.contents) {
								cpuData.push(item[0]);
								ramData.push(item[1]);
							}
							let cpuRamInfo = "";
							cpuRamInfo += `CPU : ${cpuData[0] || 0}%, RAM : ${ramData[0] || 0}%`;
							responseDiv.innerText += ` ${cpuRamInfo}`;
						}
					);
					if (cancelBtn) {
						cancelBtn.style.display = '';
						cancelBtn.textContent = `<#CTL_ok#>`;
					}
					break;
				}
				case "10017":
				case "10018": {
					// get_wan_type, get_wan_ip
					const wans_dualwan = httpApi.nvramGet(["wans_dualwan"]).wans_dualwan;
					const dualwan_enabled = (isSupport("dualwan") && wans_dualwan.search("none") == -1) ? 1 : 0;
					let wanTypeText = "";
					let wanIp = "";
					const wanInfo0 = httpApi.getWanInfo(0);
					if (dualwan_enabled) {
						const wanInfo1 = httpApi.getWanInfo(1);
						wanTypeText = `
							<#dualwan_primary#> : ${(wanInfo0.proto_text !== '') ? wanInfo0.proto_text : wanInfo0.status_text}
							<#dualwan_secondary#> : ${(wanInfo1.proto_text !== '') ? wanInfo1.proto_text : wanInfo1.status_text}
						`;
						wanIp = `
							<#dualwan_primary#> : ${wanInfo0.ipaddr || "--"}
							<#dualwan_secondary#> : ${wanInfo1.ipaddr || "--"}
						`;
					}
					else {
						wanTypeText = (wanInfo0.proto_text !== '') ? wanInfo0.proto_text : wanInfo0.status_text;
						wanIp = wanInfo0.ipaddr|| "--";
					}
					if (meta.id === "10017") {
						responseDiv.innerText += ` ${wanTypeText}`;
					}
					else if (meta.id === "10018") {
						responseDiv.innerText += ` ${wanIp}`;
					}
					if (cancelBtn) {
						cancelBtn.style.display = '';
						cancelBtn.textContent = `<#CTL_ok#>`;
					}
					break;
				}
				case "10019": {
					// get_system_time
					const utctimestamsp = parseInt(httpApi.hookGet("utctimestamp", true));
					let systemTime = "";
					if (isNaN(utctimestamsp)) {
						systemTime = new Date();
					} else {
						systemTime = new Date(utctimestamsp * 1000);
					}
					responseDiv.textContent += ` ${systemTime.toLocaleString()}`;
					if (cancelBtn) {
						cancelBtn.style.display = '';
						cancelBtn.textContent = `<#CTL_ok#>`;
					}
					break;
				}
				case "10020": {
					// get_firmware_version
					const fwInfo = httpApi.nvramGet(['firmver', 'buildno', 'extendno']);
					const firmver = fwInfo.firmver;
					const buildno = fwInfo.buildno;
					const extendno = fwInfo.extendno;
					const FWString = `${firmver}.${buildno}_${extendno}`;
					responseDiv.textContent += ` ${FWString}`;
					if (cancelBtn) {
						cancelBtn.style.display = '';
						cancelBtn.textContent = `<#CTL_ok#>`;
					}
					break;
				}
				case "10021": {
					// feedback
					if (cancelBtn) {
						cancelBtn.style.display = '';
					}
					if (okBtn) {
						okBtn.style.display = '';
						okBtn.textContent = `<#btn_go#>`;
						okBtn.onclick = function() {
							pageRedirect('settings', 'Advanced_Feedback.asp');
							closeRouterAssistantResponse();
						};
					}
					break;
				}
				case "10022": {
					// get_system_status
					// 1. get_system_time
					const utctimestamsp = parseInt(httpApi.hookGet("utctimestamp", true));
					let systemTime = "";
					if (isNaN(utctimestamsp)) {
						systemTime = new Date();
					} else {
						systemTime = new Date(utctimestamsp * 1000);
					}

					// 2. get_wan_ip & get_wan_type
					const wans_dualwan = httpApi.nvramGet(["wans_dualwan"]).wans_dualwan;
					const dualwan_enabled = (isSupport("dualwan") && wans_dualwan.search("none") == -1) ? 1 : 0;
					let wanTypeText = "";
					let wanIp = "";
					const wanInfo0 = httpApi.getWanInfo(0);
					if (dualwan_enabled) {
						const wanInfo1 = httpApi.getWanInfo(1);
						wanTypeText = `
							<#dualwan_primary#> : ${(wanInfo0.proto_text !== '') ? wanInfo0.proto_text : wanInfo0.status_text}
							<#dualwan_secondary#> : ${(wanInfo1.proto_text !== '') ? wanInfo1.proto_text : wanInfo1.status_text}
						`;
						wanIp = `
							<#dualwan_primary#> : ${wanInfo0.ipaddr || "--"}
							<#dualwan_secondary#> : ${wanInfo1.ipaddr || "--"}
						`;
					}
					else {
						wanTypeText = (wanInfo0.proto_text !== '') ? wanInfo0.proto_text : wanInfo0.status_text;
						wanIp = wanInfo0.ipaddr|| "--";
					}

					// 3. get_cpu_ram_info
					httpApi.get_diag_content_data(
						{
							ts: Date.now(),
							duration: "60",
							point: "2",
							db: "sys_detect",
							content: "cpu_usage;mem_usage",
							filter: "node_mac>txt>" + system.labelMac + ">0",
						},
						(e) => {
							let cpuData = [],
								ramData = [];
							for (let item of e.contents) {
								cpuData.push(item[0]);
								ramData.push(item[1]);
							}
							let cpuRamInfo = `CPU : ${cpuData[0] || 0}%, RAM : ${ramData[0] || 0}%`;
							const cpuRamSpan = document.querySelector('[data-component="cpu-ram-status"]');
							if(cpuRamSpan) cpuRamSpan.textContent = cpuRamInfo;
						}
					);

					// 4. get_client_num
					let nmp = httpApi.hookGet("get_clientlist", true);
					let clientNum = 0;
					for (let [key, value] of Object.entries(nmp)) {
						if (key === "maclist" || key === "ClientAPILevel") {
							continue;
						}
						if (nmp[key] !== undefined) {
							if (nmp[key].isOnline === "1" && nmp[key].amesh_isRe !== "1" && nmp[key].isAiBoard === "0") {
								clientNum++;
							}
						}
					}

					// 5. get_firmware_version
					const fwInfo = httpApi.nvramGet(['firmver', 'buildno', 'extendno']);
					const firmver = fwInfo.firmver;
					const buildno = fwInfo.buildno;
					const extendno = fwInfo.extendno;
					const FWString = `${firmver}.${buildno}_${extendno}`;

					let statusHtml = '';
					statusHtml += `<b><#General_x_SystemTime_itemname#> : </b> ${systemTime.toLocaleString()}<br>`;
					statusHtml += `<b><#WAN_IP#> : </b> ${wanIp}<br>`;
					statusHtml += `<b><#wan_type#> : </b> ${wanTypeText}<br>`;
					statusHtml += `<b><#Status_CPU#>/<#Status_RAM#> : </b> <span data-component="cpu-ram-status"></span><br>`;
					statusHtml += `<b><#Full_Clients#> : </b> ${clientNum}<br>`;
					statusHtml += `<b><#FW_item2#> : </b> ${FWString}`;
					responseDiv.innerHTML += statusHtml;
					if (cancelBtn) {
						cancelBtn.style.display = '';
						cancelBtn.textContent = `<#CTL_ok#>`;
					}
					break;
				}
				case "10023": {
					// share_wifi
					function escape_string(_str){
						if (typeof _str !== 'string') _str = (_str === undefined || _str === null) ? '' : String(_str);
						_str = _str.replace(/\\/g, "\\\\");
						_str = _str.replace(/\"/g, "\\\"");
						_str = _str.replace(/;/g, "\\;");
						_str = _str.replace(/:/g, "\\:");
						_str = _str.replace(/,/g, "\\,");
						return _str;
					}
					function encode_utf8(s){
						return unescape(encodeURIComponent(s));
					}
					function get_sdn_main_fh_info(){
						let main_fh_info = [];
						if(isSupport("sdn_mainfh")){
							const mainFH = decodeURIComponent(httpApi.nvramCharToAscii(["sdn_rl"],true).sdn_rl).split("<").filter(item => item.includes("MAINFH"));
							mainFH.forEach(item=>{
								if(item != ""){
									const apmIdx = item.split(">")[5];
									const apm_config = httpApi.nvramCharToAscii([`apm${apmIdx}_ssid`, `apm${apmIdx}_security`], true);
									const apm_ssid = decodeURIComponent(apm_config[`apm${apmIdx}_ssid`]);
									const apm_security = decodeURIComponent(apm_config[`apm${apmIdx}_security`]).split("<");
									let apm_auth = "open";
									let apm_psk = "";
									if(apm_security[1] != undefined && apm_security[1] != ""){
										const sec_arr = apm_security[1].split(">");
										apm_auth = sec_arr[1];
										apm_psk = sec_arr[3];
									}
									main_fh_info.push({"ssid":apm_ssid, "auth":apm_auth, "psk":apm_psk});
								}
							});
						}
						if(main_fh_info.length == 0){
							main_fh_info = [{"ssid":"", "auth":"open", "psk":""}];
						}
						return main_fh_info;
					}
					const mainFH = get_sdn_main_fh_info()[0];
					let qrText = "WIFI:";
					qrText += "S:" + encode_utf8(escape_string(mainFH.ssid)) + ";";
					qrText += "T:" + (mainFH.auth === "open" ? "nopass" : "WPA") + ";";
					qrText += "P:" + (mainFH.auth === "open" ? "" : escape_string(mainFH.psk)) + ";";
					qrText += ';';
					responseDiv.innerHTML += `
						<div style="display: flex; flex-direction: column; align-items: center; margin: 18px 0 8px 0;">
							<div id="wifi-qrcode"
								style="display: flex;
								justify-content: center;
								align-items: center;
								padding: 18px;
								background: #fff;
								border-radius: 16px;
								box-shadow: 0 2px 12px 0 rgba(0,0,0,0.08);">
							</div>
						</div>
					`;
					function renderJqueryQRCode() {
						if (window.jQuery && typeof jQuery.fn.qrcode === 'function') {
							$('#wifi-qrcode').empty().qrcode({
								text: qrText,
								width: 180,
								height: 180,
								background: "#ffffff",
								foreground: "#000000"
							});
							const okBtn = document.querySelector('[data-group="router-assistant-action"][data-component="okBtn"]');
							if (okBtn) {
								okBtn.style.display = '';
								okBtn.textContent = `<#option_download#> QR Code`;
								okBtn.onclick = function() {
									const qrDiv = document.getElementById('wifi-qrcode');
									let canvas = qrDiv.querySelector('canvas');
									if (!canvas) {
										const img = qrDiv.querySelector('img');
										if (img) {
											const a = document.createElement('a');
											a.href = img.src;
											a.download = 'wifi_qrcode.png';
											a.click();
										}
										return;
									}
									const a = document.createElement('a');
									a.href = canvas.toDataURL('image/png');
									a.download = 'wifi_qrcode.png';
									a.click();
								};
							}
						}
					}
					if (window.jQuery && typeof jQuery.fn.qrcode === 'function') {
						renderJqueryQRCode();
					} else {
						let script = document.createElement('script');
						script.src = '/js/qrcode/jquery.qrcode.min.js';
						script.onload = renderJqueryQRCode;
						document.body.appendChild(script);
					}
					if (cancelBtn) {
						cancelBtn.style.display = '';
						cancelBtn.textContent = `<#CTL_ok#>`;
					}
					break;
				}
				case "10024": {
					// wifi_setting
					if (cancelBtn) {
						cancelBtn.style.display = '';
					}
					if (okBtn) {
						okBtn.style.display = '';
						okBtn.textContent = `<#btn_go#>`;
						okBtn.onclick = function() {
							pageRedirect('sdn', 'sdn.html');
							closeRouterAssistantResponse();
						};
					}
					break;
				}
				case "10025": {
					// vpn_setting
					if (cancelBtn) {
						cancelBtn.style.display = '';
					}
					if (okBtn) {
						okBtn.style.display = '';
						okBtn.textContent = `<#btn_go#>`;
						okBtn.onclick = function() {
							pageRedirect('vpn', 'vpn.html');
							closeRouterAssistantResponse();
						};
					}
					break;
				}
				case "10026": {
					// op_mode
					if (cancelBtn) {
						cancelBtn.style.display = '';
					}
					if (okBtn) {
						okBtn.style.display = '';
						okBtn.textContent = `<#btn_go#>`;
						okBtn.onclick = function() {
							pageRedirect('settings', 'Advanced_OperationMode_Content.asp');
							closeRouterAssistantResponse();
						};
					}
					break;
				}
				case "10027": {
					// wan_setting
					if (cancelBtn) {
						cancelBtn.style.display = '';
					}
					if (okBtn) {
						okBtn.style.display = '';
						okBtn.textContent = `<#btn_go#>`;
						okBtn.onclick = function() {
							pageRedirect('settings', 'Advanced_WAN_Content.asp');
							closeRouterAssistantResponse();
						};
					}
					break;
				}
				case "10028": {
					// wifi_txop_status
					if (cancelBtn) {
						cancelBtn.style.display = '';
						cancelBtn.textContent = `<#CTL_ok#>`;
					}
					break;
				}
				case "10029": {
					// sensitive_words
					if (cancelBtn) {
						cancelBtn.style.display = '';
						cancelBtn.textContent = `<#CTL_ok#>`;
					}
					break;
				}
				case "10030": {
					// get_link_internet
					const wans_dualwan = httpApi.nvramGet(["wans_dualwan"]).wans_dualwan;
					const dualwan_enabled = (isSupport("dualwan") && wans_dualwan.search("none") == -1) ? 1 : 0;
					let wanStatusText = "";
					const wanInfo0 = httpApi.getWanInfo(0);
					const connectedText = `Router is connected to the internet.`;
					const disconnectedText = `Router lost connection to internet, please check ISP mode, Ethernet cable.`;
					if (dualwan_enabled) {
						const wanInfo1 = httpApi.getWanInfo(1);
						wanStatusText = `
							<#dualwan_primary#> : ${(wanInfo0.status === "connected") ? connectedText : disconnectedText}
							<#dualwan_secondary#> : ${(wanInfo1.status === "connected") ? connectedText : disconnectedText}
						`;
					}
					else {
						wanStatusText = (wanInfo0.status === "connected") ? connectedText : disconnectedText;
					}
					responseDiv.innerHTML += wanStatusText;
					if (cancelBtn) {
						cancelBtn.style.display = '';
						cancelBtn.textContent = `<#CTL_ok#>`;
					}
					break;
				}
				case "10033": {
					// get_aiboard_ip
					const ai_sys_ip_address = httpApi.nvramGet(['ai_sys_ip_address']).ai_sys_ip_address;
					responseDiv.textContent += ` ${ai_sys_ip_address}`;
					if (cancelBtn) {
						cancelBtn.style.display = '';
						cancelBtn.textContent = `<#CTL_ok#>`;
					}
					break;
				}
			}
		});
		const allActionBtns = document.querySelectorAll('[data-group="router-assistant-action"]');
		let foundBtnVisible = false;
		allActionBtns.forEach(btn => {
			if (btn.style.display !== 'none') {
				foundBtnVisible = true;
			}
		});
		if (!foundBtnVisible) {
			const cancelBtn = document.querySelector('[data-group="router-assistant-action"][data-component="cancelBtn"]');
			if (cancelBtn) {
				cancelBtn.style.display = '';
				cancelBtn.textContent = `<#CTL_ok#>`;
			}
		}
	}
}

function showRouterAssistantResponse(content, userQuestion = '') {
	const responseDiv = document.getElementById('router-assistant-response');
	if (responseDiv) {
		let html = '';
		if (userQuestion) {
			html += `<div class="text-break mb-2 p-2" style="background-color: #f8f9fa; color: #6c757d; font-size: 0.9em; border-left: 5px solid #dee2e6;">
						<strong>${userQuestion}</strong>
						</div>`;
		}
		if (content === 'loading') {
			html += `<div class="text-break d-flex align-items-center mb-3">
						<div class="spinner-border spinner-border-sm me-2" role="status">
							<span class="visually-hidden">Loading...</span>
						</div>
						Thinking...
						</div>`;
		} else {
			html += `<div data-component="router-assistant-response" class="text-break mb-3">${content}</div>`;
		}
		html += `<div class="d-flex justify-content-end gap-2 pt-2 border-top">
				<button data-group="router-assistant-action" data-component="cancelBtn" type="button" class="btn btn-outline-primary btn-sm" style="display:none" onclick="closeRouterAssistantResponse()">Later</button>
				<button data-group="router-assistant-action" data-component="okBtn" type="button" class="btn btn-primary btn-sm" style="display:none">Do it now</button>
			</div>`;
		responseDiv.innerHTML = html;
		responseDiv.style.display = 'block';
	}
}

function closeRouterAssistantResponse() {
    const responseDiv = document.getElementById('router-assistant-response');
    if (responseDiv) {
        responseDiv.style.display = 'none';
    }
}

document.addEventListener('click', function(event) {
    const container = document.querySelector('.router-assistant-container');
    const responseDiv = document.getElementById('router-assistant-response');
    
    if (container && responseDiv && !container.contains(event.target)) {
        responseDiv.style.display = 'none';
    }
});
