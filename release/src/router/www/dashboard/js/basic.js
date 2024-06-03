function genArchitecture() {
    genHeader();
    genNavMenu();
    genMidHeader();
}

function genHeader() {
    let header = document.getElementsByTagName("header")[0];
    let code = "";
    // LOGO, MODEL NAME
    code += genLogoModelName();

    // LOGOUT
    code += genLogout();

    // REBOOT
    code += genReboot();

    // LANGUAGE MENU
    code += genLanguageList();

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
    header.innerHTML = code;
}

function genLogoModelName() {
    let code = "";
    code += `
		<div class="d-flex align-items-center me-auto">
			<div role="icon" class="d-block d-md-none icon-size-24 icon-menu ms-3" id='mobile_menu'></div>
			<div role="icon" class="d-none d-md-block icon-size-logo-business icon-logo-business ms-3"></div>
			<div class="model-name mx-3"><#Web_Title2#></div>
			<div role="time-banner" class="d-none px-2 py-1 d-lg-flex flex-row" onclick="goToSystem()">
				<div class="ps-1 banner-title d-flex flex-column me-2">
                    <#General_x_SystemTime_itemname#>
                    <small class="banner-time-meridiem">${system.time.weekday}</small>
                </div>
				<div class="font-weight-bold banner-time my-auto" id="sys_time">
					<span>${system.time.current}</span>
				</div>
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

function isMultisiteApp() {
    let urlParameter = new URLSearchParams(window.location.search);
    return urlParameter.get("mapp") == "true";
}

var menuList = [
/*
	{
		name: "Home",
		icon: "icon-home",
		url: "index.asp",
		clicked: false,
		divide: false,
	},
*/
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
		name: `<#GuestNetwork_SDN_title#>`,
		icon: "icon-SDN",
		url: "sdn",
		clicked: false,
		divide: false,
	},
	{
		name: `<#BOP_isp_heart_item#>`,
		icon: "icon-VPN",
		url: "vpns",
		clicked: false,
		divide: false,
	},
	{
		name: `<#VPN_Fusion#>`,
		icon: "icon-VPN_client",
		url: "vpnc",
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
	//     name: "Help & Support",
	//     icon: "icon-help",
	//     url: "",
	//     clicked: false,
	//     divide: true,
	// },
];

if (isMultisiteApp()) {
    menuList = menuList.filter(item => item.url !== "QIS_wizard.htm");
}

let urlParameter = new URLSearchParams(window.location.search);
/* DECIDE THEME */
let theme = (function () {
    let array = ["dark", "white", "rog", "tuf"];
    let default_theme = (function () {
        if(isSupport("rog")) 
            return "rog";
        else if(isSupport("tuf")) 
            return "tuf";
        else if(isSupport("BUSINESS")) 
            return "white";
        else
            return "dark";
    })();

    let theme_string = (urlParameter.get("current_theme") || "").toLowerCase(); // handle null without parameter
    if(theme_string == "business") theme_string = "white";

    let matched = array.find((element) => element === theme_string);
    return matched === undefined ? default_theme : matched;
})();

document.querySelector("html").className = theme;

if (!urlParameter.get("url")){
    location.href = "/index.html?url=dashboard&current_theme=" + theme;
}

if (isSupport("BUSINESS") && theme != "white"){
    location.href = "/index.html?url=dashboard&current_theme=white";
}

if(!isSupport("BUSINESS") || theme != "white"){
    menuList = menuList.filter(function(item, index, array){
        return (item.url != "settings") && (item.url != "QIS_wizard.htm");
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
			name: `<#GuestNetwork_SDN_title#>`,
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

function genNavMenu() {
    /*
        {
            name: MENU名稱,
            icon: icon類型,
            url: 頁面URL名稱,
            clicked: [boolean] ,
            divide: [boolean] 分隔線,
        },
    
    */
	var menuListClicked = location.search.split("url=")[1].split("&")[0];
	for(var i=0; i<menuList.length; i++){
		menuList[i].clicked = false;
		if(menuList[i].url.indexOf(menuListClicked) != -1) menuList[i].clicked = true;
	}

    let nav = document.getElementsByTagName("nav")[0];

    // for mobile layout, show the close icon
    let code = `
        <div role="menu-close" class="d-flex justify-content-end">
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
                        class="btn btn-toggle d-flex flex-row flex-md-column align-items-center justify-content-start justify-content-md-center ps-2 pe-2 py-2 w-100 ${list.clicked ? "menu-clicked" : ""}"
                        onclick="pageRedirect('${list.url}')"
                    >
                        <div role="icon" class="icon-size-28 ${list.icon} icon-color-menu me-3 me-md-0"></div>
                        <div role="menu-text" class="ms-0">${list.name}</div>
                    </button>
                </li>
            </ul>
        `;
    }

    code += `
        <div class="mb-3 p-2 copyright">2022 ASUSTeK Computer Inc. All rights reserved.</div>
    `;

    nav.innerHTML = code;
    document.getElementById("mobile_menu").addEventListener("click", function () {
        let menu_hide = nav.classList.contains("nav-menu-hide");
        menu_hide ? nav.classList.remove("nav-menu-hide") : nav.classList.add("nav-menu-hide");
    });
    document.getElementById("mobile_menu_close").addEventListener("click", function () {
        let menu_hide = nav.classList.contains("nav-menu-hide");
        menu_hide ? nav.classList.remove("nav-menu-hide") : nav.classList.add("nav-menu-hide");
    });
}

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
    setInterval(function () {
        system.time = getTime();
        document.getElementById("sys_time").innerHTML = `
            <span>${system.time.current}</span>
        `;
/*
        document.getElementById("sys_time").innerHTML = `
            <span>${system.time.current}</span>
            <small class="banner-time-meridiem">${system.time.weekday}</small>
        `;
*/
    }, 1000);
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
	var FbNote = "Feedback is ongoing. Rebooting may cause the program to terminate and the feedback cannot be completed.\nPlease reboot later.";/*untranslated*/

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
	location.href = "/index.html?url=settings&current_theme=white&page=Advanced_System_Content.asp";
}

function pageRedirect(target) {
    if (target === "index.asp") {
        location.href = target;
    }
    else if (target == "QIS_wizard.htm") {
        location.href = "/QIS_wizard.htm";
    }
    else if (urlParameter.get("mapp") === "true") {
        location.href = `/index.html?url=${target}&current_theme=${theme}&mapp=true`;
    }
    else {
        location.href = "/index.html?url=" + target + "&current_theme=" + theme;
    }
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
	element.className = "shadow-bg";
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

let oInactivityTimerId;
const http_autologout = parseInt(httpApi.nvramGet(['http_autologout']).http_autologout);
if (http_autologout != 0) {
    const resetInactivityTimer = () => {
        clearTimeout(oInactivityTimerId);
        oInactivityTimerId = setTimeout(() => {
            location.href = '/Logout.asp';
        }, http_autologout * 10 * 1000);
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

