if(typeof stringSafeGet != "function"){
	function stringSafeGet(str){
		return str.replace(new RegExp("&#39;", 'g'), "'");
	}
}
if(typeof Get_Component_PWD_Strength_Meter != "function"){
	function Get_Component_PWD_Strength_Meter(id){
		var postfix = (id == undefined)? "": ("_"+id);
		var $pwd_strength_container = $("<div>").addClass("pwd_strength_container").attr("id", "scorebarBorder"+postfix).attr("title", "<#LANHostConfig_x_Password_itemSecur#>");
		var $strength_text = $("<div>").addClass("strength_text").appendTo($pwd_strength_container).attr("id", "score"+postfix);
		var $strength_color = $("<div>").addClass("strength_color").appendTo($pwd_strength_container).attr("id", "scorebar"+postfix);
		return $pwd_strength_container;
	}
}
if(typeof get_wl_unit_by_band != "function"){
	function get_wl_unit_by_band(_band){
		if(_band == undefined) return "";

		_band = (_band).toString().toUpperCase();
		var wl_nband = "";

		switch(_band){
			case "2G":
				wl_nband = "2";
				break;
			case "5G":
			case "5G1":
			case "5G2":
				wl_nband = "1";
				break;
			case "6G":
			case "6G1":
			case "6G2":
				wl_nband = "4";
				break;
			case "60G":
				wl_nband = "6";
				break;
		}
		if(wl_nband == "") return "";

		var wl_unit = "";
		var suffix_num = _band.substr(_band.indexOf("G") + 1);
		var ordinal_num = (suffix_num == "") ? 1 : parseInt(suffix_num);
		var count = 1;
		var wl_nband_array = httpApi.hookGet("wl_nband_info");
		$.each(wl_nband_array, function(wlx, value){
			if(value == wl_nband){
				if(count == ordinal_num){
					wl_unit = wlx;
					return false;
				}
				else{
					count+=1;
				}
			}
		});
		return wl_unit.toString();
	}
}
if(typeof ui_lang != "string"){
	var ui_lang = httpApi.nvramGet(["preferred_lang"]).preferred_lang;
}

let terms_service_template = "Welcome to our Wi-Fi service (the \"Service\"). By accessing and using this Service, you represent and acknowledge that you are of legal age, and you have read and agree to be bound by the following terms and conditions:\n1. Accessing the Service\n1.1 The Service is provided by the store (the \"Store\") for your personal use. The Store reserves the right to change or terminate the Service or change this Terms and Conditions at any time. You are responsible for reviewing this Terms and Conditions each time you use the Service.\n1.2 Access to the Service may be blocked, suspended or terminated at any time for any reason including but not limited to your violation of this Terms and Conditions or applicable laws.\n1.3 The Store does not guarantee availability to the Internet, the minimum Internet connection speeds on the network, or that the Service will be stable, fault-free, timely, reliable, operational or suitable for supporting your intended use.\n1.4 The Store does not guarantee the quality of the information on the internet. It is your responsibility to determine the validity, quality, and relevance of the information obtained.\n1.5 The Service is provided on an open and public basis and the Store cannot guarantee the security of the Wi-Fi service, you acknowledge and agree that there are inherent security risks associated with using the Service and that you do so solely at your own risk.\n2. Restrictions\nYou acknowledge and agree that when using the Service you will comply with all applicable laws and that you will not;\na. use the Service for commercial purposes;\nb. use the Service to send unsolicited bulk emails;\nc. reveal or publish proprietary or confidential information;\nd. infringe the intellectual property rights of any person;\ne. collect or harvest any information or data from the Service or the servers used to provide the Service;\nf. attempt to intercept, interfere with or decipher any transmission to or from the servers used to provide the Service;\ng. connect to \"Peer to Peer\" file sharing networks, download large files, or run any programs, services, systems, processes, or servers that may substantially degrade network performance or accessibility;\nh. use the Service to transmit, send, upload, receive, download, publish, post, distribute, disseminate, encourage or solicit receipt of any material which is abusive, defamatory, harassing, indecent, offensive, obscene, menacing, racist, pornographic, threatening, unlawful or in breach of any right of any person;\ni. use the Service to transmit, store, publish or upload any electronic material, such as viruses, malware, Trojan horses, worms ,or time bombs, which will or are likely to cause damage to, or to destroy or limit the functionality of, any computer, software, hardware, electronic device or telecommunications equipment;\nj. obtain unauthorised access to any other person's computer, email account, bank account, computer network or equipment; or\nk. use the Service to invade the privacy of another person or to cause annoyance, inconvenience or anxiety to another person.\n3. Privacy\nYou acknowledge and agree that the Service will capture and process information about the web browser type and/or operating system information used by the enabled device to determine the more effective and/or customized means of displaying the requested website on your device. And the Store will collect the IP and MAC address of the enabled device that has accessed the Wi-Fi service, once the Terms and Conditions have been agreed to.\n4. Release and Indemnity\n4.1 The Store is not liable for loss of data due to service delays or interruptions or any kind of loss or damages you may sustain as a result of your use of the Service.\n4.2 You release and discharge the Store from any liability which might arise from your use of the Service, including liability in relation to defamatory or offensive material or any breach of copyright which may occur.\n4.3 You agree to indemnify and must defend and hold harmless the Store, its employees and agents, from and against all loss, damage, liability, charge, expense (including but not limited to attorneys' fees) of any nature or kind arising from your breach of these Terms and Conditions.";
var str_SDN_choose = stringSafeGet("<#GuestNetwork_SDN_choose#>");
var str_WiFi_sche = stringSafeGet("<#GuestNetwork_WiFi_Sche#>");
var str_Scheduled = stringSafeGet("<#Time_Scheduled#>");
var str_Scheduled_one_time = stringSafeGet("<#Time_Scheduled_one_time#>");
var str_noProfile_hint = stringSafeGet("<#GuestNetwork_noProfile_hint#>");
var str_GuestNetwork_WiFi_Sche_desc = stringSafeGet("<#GuestNetwork_WiFi_Sche_desc#>");
var str_Scenario_Classroom = stringSafeGet("<#Scenario_Classroom#>");
var str_Scenario_coffee = stringSafeGet("<#Scenario_coffee#>");
var str_Scenario_Study = stringSafeGet("<#Scenario_Study#>");
var str_Scenario_EV_charging = stringSafeGet("<#Scenario_EV_charging#>");
var str_Scenario_smartHome = stringSafeGet("<#Scenario_smartHome#>");
var str_Scenario_branchOffice = stringSafeGet("<#Scenario_branchOffice#>");
var str_Scenario_Hotel = stringSafeGet("<#Scenario_Hotel#>");
var str_Scenario_Mall = stringSafeGet("<#Scenario_Mall#>");
var str_Scenario_Friends = stringSafeGet("<#Scenario_Friends#>");
var str_GuestNetwork_Portal_desc2 = stringSafeGet("<#GuestNetwork_Portal_desc2#>");
var str_GuestNetwork_Wizard_type_desc0 = stringSafeGet("<#GuestNetwork_Wizard_type_desc0#>");
var str_GuestNetwork_Employee_recommend = stringSafeGet("<#GuestNetwork_Employee_recommend#>");
var sdn_maximum = ((isSupport("MaxRule_SDN") == "0") ? 6 : (parseInt(isSupport("MaxRule_SDN")) - 1));//default is sdn 0
const wifi_band_maximum = isSupport("SMART_HOME_MASTER_UI") ? 3 : 5;
const mlo_fh_enable_maximum = 1;
var selected_sdn_idx = "";
var selected_vlan_idx = "";
var sdn_all_rl_json = [];
var sdn_schedule = "";
var cfg_clientlist = httpApi.hookGet("get_cfg_clientlist");
const wifi_band_options = (()=>{
	const options = [
		{"text":"2.4GHz / 5GHz","value":"3"},
		{"text":"2.4GHz","value":"1"},
		{"text":"5GHz","value":"2"},
		{"text":"6GHz","value":"4"},
		{"text":"5GHz / 6GHz","value":"5"},
		{"text":"2.4GHz / 5GHz / 6GHz","value":"6"},
		{"text":"<#wl_securitylevel_0#>","value":"0"}
	];
	let support_2G = false, support_5G = false, support_6G = false;
	if(!isSupport("noWiFi")){
		//Check CAP supoort band
		if(get_wl_unit_by_band("2G") != "") support_2G = true;
		if(get_wl_unit_by_band("5G") != "") support_5G = true;
		if(get_wl_unit_by_band("6G") != "") support_6G = true;
	}
	$.each(cfg_clientlist, function(index, node_info){//Check RE supoort band
		if(index == 0) return true;//filter cap
		let wifi_band_info = httpApi.aimesh_get_node_wifi_band(node_info);
		if(!support_2G){
			let band_2G = wifi_band_info.find((item)=>{
				return (item.band == "1");
			});
			if(band_2G != undefined) support_2G = true;
		}
		if(!support_5G){
			let band_5G = wifi_band_info.find((item)=>{
				return (item.band == "2" || item.band == "4" || item.band == "8");
			});
			if(band_5G != undefined) support_5G = true;
		}
		if(!support_6G){
			let band_6G = wifi_band_info.find((item)=>{
				return (item.band == "16" || item.band == "32" || item.band == "64");
			});
			if(band_6G != undefined) support_6G = true;
		}
	});
	let result = [];
	if(support_2G){
		result = result.concat(options.find((el) => {
			return (el.value == "1");
		}));
	}
	if(support_5G){
		result = result.concat(options.find((el) => {
			return (el.value == "2");
		}));
	}
	if(support_2G && support_5G){
		result = [options.find((el) => {
			return (el.value == "3");
		})].concat(result);
	}
	if(support_6G){
		result = result.concat(options.find((el) => {
			return (el.value == "4");
		}));
	}
	if(support_6G && isSupport("mlo")){
		result = result.concat(options.filter((el) => {
			return (el.value == "5" || el.value == "6");
		}));
	}
	result = result.concat(options.find((el) => {
		return (el.value == "0");
	}));
	return result;
})();
var interval_AccTime = false;
var VLAN_Profile_select = [];
const is_Web_iframe = (($(parent.document).find("#mainMenu").length > 0) || (top.webWrapper)) ? true : false;
var page_container = window.location.pathname.split("/").pop();
var cap_mac = (httpApi.hookGet('get_lan_hwaddr')) ? httpApi.hookGet('get_lan_hwaddr') : '';
var is_QIS_flow = (function(){
	var result = false;
	if(typeof apply == "object"){
		if(typeof apply.SDN_Profile == "function"){
			result = true;
		}
	}
	return result;
})();
var is_cfg_ready = (cfg_clientlist.length > 0) ? true : false;
sdnRuleTable = [
	"idx",
	"sdn_name",
	"sdn_enable",
	"vlan_idx",
	"subnet_idx",
	"apg_idx",
	"vpnc_idx",
	"vpns_idx",
	"dns_filter_idx",
	"urlf_idx",
	"nwf_idx",
	"cp_idx",
	"gre_idx",
	"firewall_idx",
	"kill_switch",
	"access_host_service",
	"wan_unit",
	"pppoe-relay",
	"wan6_unit",
	"createby",
	"mtwan_idx",
	"mswan_idx"
];

vlanRuleTable = [
	"vlan_idx",
	"vid",
	"port_isolation"
];

apgRuleTable = [
	"mac",
	"wifiband",
	"lanport"
];

vlanTrunklistTable = [
	"mac",
	"port",
	"profile"
];

vlanBlklistTable = [
	"mac",
	"port",
	"profile"
];
var vlan_rl = decodeURIComponent(httpApi.nvramCharToAscii(["vlan_rl"]).vlan_rl);
var vlan_rl_json = convertRulelistToJson(vlanRuleTable, vlan_rl);

var vlan_rl_vid_array = [];
$.each(vlan_rl_json, function(idx, vlan) {
	vlan_rl_vid_array.push(vlan.vid);
});

var vlan_trunklist_orig = decodeURIComponent(httpApi.nvramCharToAscii(["vlan_trunklist"]).vlan_trunklist);
var vlan_trunklist_tmp = vlan_trunklist_orig+"_";	//easy to do replace
var vlan_trunklist_array = vlan_trunklist_orig.split("<");
var vlan_trunklist_port_array = [];
var vlan_trunklist_port_array_one_mac = [];	//2-dim by mac
var vlan_trunklist_port = "";
var vlan_trunklist_port_vid_array = [];
var vlan_rl_vid_array_tmp = [];

var vlan_trunklist_string = "";
var vlan_trunklist_json = [];
var vlan_trunklist_json_tmp = [];

if (vlan_trunklist_array.length > 1) {
	for (var b = 1; b < vlan_trunklist_array.length; b++) {	//DUT Macs
		vlan_trunklist_port_array_one_mac[b] = vlan_trunklist_array[b].split(">");
		for (var c = 0; c < vlan_trunklist_port_array_one_mac[b].length; c++) {	//binded LAN ports

			vlan_trunklist_port_array.push(vlan_trunklist_port_array_one_mac[b][c]);
			//collect each mac & port binding

			if (c > 0) {
				vlan_rl_vid_array_tmp = vlan_rl_vid_array;
				vlan_trunklist_port = vlan_trunklist_port_array_one_mac[b][c].split("#")[0];
				vlan_trunklist_port_vid_array = vlan_trunklist_port_array_one_mac[b][c].split("#")[1].split(",");
				vlan_rl_vid_array_tmp = vlan_trunklist_port_vid_array;
				
				//suppose 1st vid (vlan_rl_vid_array_tmp[0]) to be the only one vid binded
				vlan_trunklist_string += "<"+vlan_trunklist_port_array_one_mac[b][0]+">"+vlan_trunklist_port+">"+vlan_rl_vid_array_tmp[0];
			}
		}
	}
	vlan_trunklist_json = convertRulelistToJson(vlanTrunklistTable, vlan_trunklist_string);
	vlan_trunklist_json_tmp = convertRulelistToJson(vlanTrunklistTable, vlan_trunklist_string);
}

function arrayRemove(arr, value) {
	return arr.filter(function(ele) {
		return ele != value;
	});
}

var cp_type_support = {};
var cp_type_rl_json = (function(){
	if(!isSwMode("rt")) return [];

	var cp_type_json = [];
	var cp_type_rl_attr = function(){
		this.cp_idx = "";
		this.cp_type = "";
		this.cp_text = "";
		this.sdn_idx = "";
		this.profile = [];
	};

	var cp_text_map = [];
	if(!isSupport("SMART_HOME_MASTER_UI")){
		if(isSupport("captivePortal") && isSupport("cp_advanced"))
			cp_text_map.push({"id":"1", "text":"<#Captive_Portal#> WiFi"});
		if(isSupport("captivePortal") && isSupport("cp_freewifi")){
			cp_text_map.push({"id":"2", "text":"Free WiFi"});
			if(!isSupport("BUSINESS"))
				cp_text_map.push({"id":"4", "text":`<#Message_Board#>`});
		}
	}
	if(isSupport("fbwifi_sdn") && !is_QIS_flow)
		cp_text_map.push({"id":"3", "text":"Facebook WiFi"});

	var cp_type_rl = decodeURIComponent(httpApi.nvramCharToAscii(["cp_type_rl"]).cp_type_rl);
	var each_cp_type_rl = cp_type_rl.split("<");
	$.each(each_cp_type_rl, function(index, value){
		if(value != ""){
			var profile_data = value.split(">");
			var specific_cp_text = cp_text_map.filter(function(item, index, array){
				return (item.id == profile_data[1]);
			})[0];
			if(specific_cp_text != undefined){
				var cp_type = JSON.parse(JSON.stringify(new cp_type_rl_attr()));
				cp_type.cp_text = specific_cp_text.text;
				cp_type.cp_idx = profile_data[0];
				cp_type.cp_type = profile_data[1];
				cp_type_json.push(cp_type);
				cp_type_support["type_"+cp_type.cp_type+""] = true;
			}
		}
	});

	return cp_type_json;
})();
var vpns_rl_json = (function(){
	if(!isSwMode("rt")) return [];

	var vpns_type_text_map = [
		{"proto": "PPTP", "text": "PPTP"},
		{"proto": "OpenVPN", "text": "OpenVPN"},
		{"proto": "IPSec", "text": "IPSec VPN"},
		{"proto": "WireGuard", "text": "WireGuard VPN"}
	];

	var vpns_rl = decodeURIComponent(httpApi.nvramCharToAscii(["vpns_rl"]).vpns_rl);
	var vpn_type_json = convertRulelistToJson(["desc","proto","unit", "activate", "vpns_idx"], vpns_rl);
	if(!isSupport("pptpd"))
		vpn_type_json = remove_proto("PPTP");
	if(!isSupport("openvpnd"))
		vpn_type_json = remove_proto("OpenVPN");
	if(!isSupport("ipsec_srv"))
		vpn_type_json = remove_proto("IPSec");
	if(!isSupport("wireguard"))
		vpn_type_json = remove_proto("WireGuard");

	$.each(vpn_type_json, function(index, vpn_type){
		var spec_vpns_type = vpns_type_text_map.filter(function(item){
			return item.proto == vpn_type.proto;
		})[0];
		if(spec_vpns_type != undefined){
			vpn_type.text = spec_vpns_type.text;
		}
		else{
			vpn_type.text = vpn_type.proto;
		}
		if(vpn_type.unit > 1)
			vpn_type.text += " " + vpn_type.unit;
		if(vpn_type.proto == "WireGuard"){
			var activate = httpApi.nvramGet(["wgs"+vpn_type.unit+"_enable"])[["wgs"+vpn_type.unit+"_enable"]];
			if(activate == "1")
				vpn_type.activate = "1";
		}
	});

	return vpn_type_json

	function remove_proto(_proto) {
		return vpn_type_json.filter(function(item){
			if(item.proto == _proto){
				return false;
			}
			return true;
		});
	}
})();
let wizard_type_text = [
	{"type":"Employee", "text":"<#GuestNetwork_Employee#>", "desc":"<#GuestNetwork_Employee_desc#>"},
	{"type":"Portal", "text":"<#GuestNetwork_Portal#>", "desc":"<#GuestNetwork_Portal_desc2#>"},
	{"type":"Guest", "text":"<#Guest_Network#>", "desc":"<#GuestNetwork_Guests_desc#>"},
	{"type":"Kids", "text":"<#GuestNetwork_Kid#>", "desc":"<#GuestNetwork_Kid_desc#>"},
	{"type":"IoT", "text":"<#GuestNetwork_IoT#>", "desc":"<#GuestNetwork_IoT_desc#>"},
	{"type":"VPN", "text":"<#GuestNetwork_VPN#>", "desc":"<#GuestNetwork_VPN_desc#>"},
	{"type":"Customized", "text":"<#GuestNetwork_Customized#>", "desc":""},
	{"type":"MLO", "text":`<#WiFi_mlo_title#>`, "desc":`<#WiFi_mlo_Desc1#>`}
];
if(isSupport("BUSINESS")){
	var specific_wizard_type = wizard_type_text.filter(function(item, index, array){
		return (item.type == "Kids");
	})[0];
	if(specific_wizard_type != undefined){
		specific_wizard_type.type = "Sched";
		specific_wizard_type.text = "<#GuestNetwork_Sche_network#>";
		specific_wizard_type.desc = str_GuestNetwork_WiFi_Sche_desc;
	}
}
function Get_Component_Profile_Item_AddNew(){
	var $profile_item_container = $("<div>").addClass("profile_item_container addnew");
	$profile_item_container.unbind("click").click(function(e){
		e = e || event;
		e.stopPropagation();
		var rule_num = 0;
		if(rule_num >= sdn_maximum){
			show_customize_alert("<#weekSche_MAX_Num#>".replace("#MAXNUM", sdn_maximum));
			return false;
		}
		selected_sdn_idx = "";
		if($(".profile_setting_container").css("display") == "none"){
			$(".popup_element").css("display", "flex");
			$(".container").addClass("blur_effect");
			$(".popup_container.popup_element").empty().append(show_Get_Start("popup"));
		}
		else{
			$(this).closest("#profile_list_content").find(".profile_item_container").removeClass("selected");
			$(this).closest(".profile_item_container").addClass("selected");
			$(".profile_setting_container").empty().append(show_Get_Start());
			resize_iframe_height();
		}
	});
	var $item_text_container = $("<div>").addClass("item_text_container");
	$item_text_container.appendTo($profile_item_container);

	$("<div>").addClass("main_info").html("<#GuestNetwork_ProfileAdd#>").appendTo($item_text_container);

	return $profile_item_container;
}
function Get_Component_Profile_Item(_profile_data){
	var specific_data = sdn_all_rl_json.filter(function(item, index, array){
			return (item.sdn_rl.idx == _profile_data.sdn_idx);
		})[0];

	let sdn_type = (()=>{
		if(_profile_data.type == "LEGACY"){
			return "LEGACY";
		}
		else{
			let type_exist = wizard_type_text.some(function(item, index, array){
				return item.type === _profile_data.type;
			});
			return (type_exist) ? _profile_data.type : "Customized";
		}
	})();
	let $profile_item_container = $("<div>").addClass("profile_item_container profile_list " + sdn_type + "").attr("sdn_idx", _profile_data.sdn_idx);

	var $item_tag_cntr = $("<div>").addClass("item_tag_container").appendTo($profile_item_container);
	if(specific_data.sdn_rl.vpnc_idx != "0"){
		$("<div>").addClass("item_tag VPN").html("<#vpnc_title#>").appendTo($item_tag_cntr);
	}
	if(specific_data.sdn_rl.vpns_idx != "0"){
		$("<div>").addClass("item_tag VPN").html("<#BOP_isp_heart_item#>").appendTo($item_tag_cntr);
	}
	if(specific_data.sdn_rl.sdn_enable == "1" && specific_data.apg_rl.timesched == "2"){
		var utctimestamsp = parseInt(httpApi.hookGet("utctimestamp", true));
		var cur_time = (isNaN(utctimestamsp)) ? 0 : utctimestamsp;
		var end_expiretime = specific_data.apg_rl.expiretime.split(",")[1];
		if(end_expiretime != undefined && end_expiretime != "" && end_expiretime.length == 13){
			var end_time = parseInt(end_expiretime.substr(2,10));
			var remaining_time = ((end_time - cur_time) > 0) ? (end_time - cur_time) : 0;
			var HMS = secondsToHMS(remaining_time);
			$("<div>")
				.addClass("item_tag Timer")
				.attr({
					"title": "<#mssid_time_remaining#>",
					"end_time":end_time,"cur_time":cur_time,
					"data-container":"AccTime","access_time":(remaining_time > 0 ? true : false)
				})
				.html(HMS.hours + ":" + HMS.minutes + ":" + HMS.seconds)
				.appendTo($item_tag_cntr);
		}
	}

	if(get_cp_type_support("2") && specific_data.sdn_rl.cp_idx == "2"){
		$("<div>").addClass("item_tag no_icon").html("Free WiFi").appendTo($item_tag_cntr);
	}
	if(get_cp_type_support("4") && specific_data.sdn_rl.cp_idx == "4"){
		$("<div>").addClass("item_tag no_icon").html(`<#Message_Board#>`).appendTo($item_tag_cntr);
	}

	var $item_text_container = $("<div>").addClass("item_text_container").appendTo($profile_item_container)
	$item_text_container.unbind("click").click(function(e){
		e = e || event;
		e.stopPropagation();
		selected_sdn_idx = _profile_data.sdn_idx;
		if($(".profile_setting_container").css("display") == "none"){
			$(".popup_element").css("display", "flex");
			$(".container").addClass("blur_effect");
			$(".popup_container.popup_element").empty().append(Get_Component_Full_Setting("popup"));
			Update_Setting_Profile($(".popup_container.popup_element"), specific_data);
			adjust_popup_container_top($(".popup_container.popup_element"), 100);
		}
		else{
			$(this).closest("#profile_list_content").find(".profile_item_container").removeClass("selected");
			$(this).closest(".profile_item_container").addClass("selected");
			$(".profile_setting_container").empty().append(function(){
				return Get_Component_Full_Setting();
			});
			Update_Setting_Profile($(".profile_setting_container"), specific_data);
			resize_iframe_height();
		}
	});

	var $main_info = $("<div>").addClass("main_info").appendTo($item_text_container);
	$("<div>").addClass("main_info_text")
		.attr({"title":"<#QIS_finish_wireless_item1#>: " + _profile_data.name})
		.html(htmlEnDeCode.htmlEncode(_profile_data.name)).appendTo($main_info);
	let $status_info = $("<div>").addClass("status_info").appendTo($main_info);
	let $status_left_cntr = $("<div>").addClass("left_cntr").appendTo($status_info);
	let $status_right_cntr = $("<div>").addClass("right_cntr").appendTo($status_info);

	var conn_type_arr = get_conn_type(specific_data);
	$.each(conn_type_arr, function(index, item){
		if(index > 0) $("<div>").addClass("split_line").appendTo($status_left_cntr);
		$("<div>").addClass("conn_type " + item.type).attr({"title":item.text}).appendTo($status_left_cntr);
	});

	var wifi_band_arr = get_wifi_band(specific_data);
	$.each(wifi_band_arr, function(index, item){
		$("<div>").addClass("conn_type " + item.type).attr({"title":item.text}).appendTo($status_right_cntr);
	});

	$("<div>").addClass("icon_switch")
		.addClass((function(){
			return ((_profile_data.activate == "1") ? "on" : "off");
		})())
		.unbind("click").click(function(e){
			e = e || event;
			e.stopPropagation();
			let $this_icon_switch = $(this);
			$this_icon_switch.toggleClass("off on").promise().done(function(){
				var $profile_obj = $(this).closest(".profile_item_container");
				if($(this).hasClass("on")){
					var other_portal_enabled = false;
					if(get_cp_type_support(specific_data.sdn_rl.cp_idx)){
						$.each(sdn_all_rl_json, function(index, item) {
							if(item.sdn_rl.idx == specific_data.sdn_rl.idx)
								return true;
							if(item.sdn_rl.cp_idx != "0"){
								if(item.sdn_rl.sdn_enable == "1"){
									other_portal_enabled = true;
									return false;
								}
							}
						});
					}
					if(other_portal_enabled){
						$(this).toggleClass("off on");
						show_customize_alert("<#vpnc_conn_maxi_general#>".replace(/VPN/g, "<#GuestNetwork_Portal_type#>").replace(/2/g, "1"));
						return;
					}
					if(isSupport("mlo")){
						const is_mlo_type = sdn_all_rl_json.some(function(item, index, array){
							return (item.sdn_rl.sdn_name == "MLO" && item.sdn_rl.idx == specific_data.sdn_rl.idx && item.apg_rl.mlo == "2");
						});
						if(is_mlo_type){
							const mld_enable = httpApi.nvramGet(["mld_enable"]).mld_enable;
							if(mld_enable != "1"){
								$(this).toggleClass("off on");
								if(PAGE_CONTAINER != "mlo"){
									show_customize_confirm(`<#WiFi_mlo_enable_Hint#>`);
									const $confirm_obj = $(".popup_container.popup_customize_alert");
									$confirm_obj.find("[data-btn=ok]").html(`<#CTL_close#>`);
									$confirm_obj.find("[data-btn=cancel]").html(`<#btn_goSetting#>`)
										.click(()=>{
											if(is_Web_iframe){
												top.location.href = "/MLO.asp";
											}
											else{
												top.location.href = "/SDN/mlo.html" + ((typeof theme == "string" && theme != "") ? "?current_theme=" + theme + "" : "");
											}
										});
								}
								else{
									show_customize_alert(`<#WiFi_mlo_enable_Hint#>`);
								}
								return;
							}
							const mlo_enable_count = sdn_all_rl_json.filter(function(item, index, array){
								return (item.sdn_rl.sdn_name == "MLO" && item.sdn_rl.sdn_enable == "1" && item.apg_rl.mlo == "2");
							}).length;
							if(mlo_enable_count >= mlo_fh_enable_maximum){
								$(this).toggleClass("off on");
								show_customize_alert(`As the maximum number of the active MLO network is limited to 1. Please deactivate one existing MLO network profile before activating another.`);/* untranslated */
								return;
							}
						}
					}
					specific_data.sdn_rl.sdn_enable = "1";
					specific_data.apg_rl.enable = "1";
				}
				else{
					specific_data.sdn_rl.sdn_enable = "0";
					specific_data.apg_rl.enable = "0";
				}
				selected_sdn_idx = specific_data.sdn_rl.idx;
				var sdn_all_list = parse_JSONToStr_sdn_all_list();
				var nvramSet_obj = {"action_mode": "apply", "rc_service": "restart_wireless;restart_sdn " + selected_sdn_idx + ";"};
				if(httpApi.nvramGet(["qos_enable"]).qos_enable == "1"){
					nvramSet_obj.rc_service += "restart_qos;restart_firewall;";
				}
				if(specific_data.sdn_rl.cp_idx == "2" || specific_data.sdn_rl.cp_idx == "4"){
					nvramSet_obj.rc_service += "restart_chilli;restart_uam_srv;";
				}
				nvramSet_obj["sdn_rl"] = sdn_all_list.sdn_rl;
				nvramSet_obj["apg" + specific_data.apg_rl.apg_idx + "_enable"] = specific_data.apg_rl.enable;
				var showLoading_status = get_showLoading_status(nvramSet_obj.rc_service);
				const parent_cntr = (typeof PAGE_CONTAINER == "string") ? PAGE_CONTAINER : "sdn";
				showLoading();
				if(isWLclient()){
					showLoading(showLoading_status.time);
					setTimeout(function(){
						showWlHintContainer();
					}, showLoading_status.time*1000);
					check_isAlive_and_redirect({"page": "" + ((parent_cntr == "mlo") ? "MLO" : "SDN") + ".asp", "time": showLoading_status.time});
				}
				httpApi.nvramSet(nvramSet_obj, function(){
					if(isWLclient()) return;
					showLoading(showLoading_status.time);
					init_sdn_all_list();
					if(parent_cntr == "mlo")
						show_mlo_profilelist();
					else
						show_sdn_profilelist();
					close_popup_container("all");
					setTimeout(function(){
						if(!window.matchMedia('(max-width: 575px)').matches)
							$this_icon_switch.closest(".profile_item_container").find(".item_text_container").click();
					}, showLoading_status.time*1000);
					if(!isMobile()){
						if(showLoading_status.disconnect){
							check_isAlive_and_redirect({"page": "" + ((parent_cntr == "mlo") ? "MLO" : "SDN") + ".asp", "time": showLoading_status.time});
						}
					}
				});
			});
		}).appendTo($status_right_cntr);

	let $bottom_info = $("<div>").addClass("bottom_info").appendTo($item_text_container);
	if(!($.isEmptyObject(specific_data.vlan_rl))){
		$("<div>").html(`VLAN<span>${specific_data.vlan_rl.vid}</span>`).appendTo($bottom_info);
	}
	$("<div>").addClass("clients_num").attr({"title":"<#number_of_client#>"})
		.html(`<div class='icon_client'></div><span>${htmlEnDeCode.htmlEncode(_profile_data.client_num)}</span>`).appendTo($bottom_info);

	return $profile_item_container;

	function get_wifi_band(_sdn_data){
		let wifi_band = [];
		let wifi_band_arr = [];
		const is_mlo_type = (_sdn_data.apg_rl.mlo == "1" || _sdn_data.apg_rl.mlo == "2") ? true : false;
		if(is_mlo_type){
			wifi_band_arr.push("mlo");
		}
		else{
			let apg_dut_list_array = (_sdn_data.apg_rl.dut_list).split("<");
			$.each(apg_dut_list_array, function(index, dut_info){
				if(dut_info != ""){
					let dut_info_array = dut_info.split(">");
					const wifi_band = isNaN(parseInt(dut_info_array[1])) ? 0 : parseInt(dut_info_array[1]);
					if(wifi_band & 1){
						wifi_band_arr.push("2G");
					}
					if(wifi_band & 2 || wifi_band & 4 || wifi_band & 8){
						wifi_band_arr.push("5G");
					}
					if(wifi_band & 16 || wifi_band & 32 || wifi_band & 64){
						wifi_band_arr.push("6G");
					}
					return false;
				}
			});
		}

		let apg_sec_array = (_sdn_data.apg_rl.security).split("<");
		let cap_wifi_auth = "lock";
		if(apg_sec_array[1] != undefined && apg_sec_array[1] != ""){
			let cap_sec_array = apg_sec_array[1].split(">");
			let is_wifi_open = (cap_sec_array[1] == "open" || cap_sec_array[1] == "owe" || cap_sec_array[1] == "openowe") ? true : false;
			cap_wifi_auth = (is_wifi_open ? "" : "lock");
		}
		$.each(wifi_band_arr, function(index, band){
			let band_info = {"type":"", "text": ""};
			band_info.type = (cap_wifi_auth == "lock") ? "WiFi_"+band+"_lock" : "WiFi_"+band+"";
			if(band == "mlo")
				band_info.text = `<#WiFi_mlo_title#>`;
			else
				band_info.text = `${((band == "2G") ? "2.4" : band.substring(0,1))} GHz <#tm_wireless#>`;
			wifi_band.push(band_info);
		});
		return wifi_band;
	}
	function get_conn_type(_sdn_data){
		let conn_type = [];
		let is_wifi = true;
		const apg_dut_list_array = (_sdn_data.apg_rl.dut_list).split("<");
		$.each(apg_dut_list_array, function(index, dut_info){
			if(dut_info != ""){
				const dut_info_array = dut_info.split(">");
				const wifi_band = isNaN(parseInt(dut_info_array[1])) ? 0 : parseInt(dut_info_array[1]);
				if(wifi_band == 0) is_wifi = false;
				return false;
			}
		});

		if(!is_wifi){
			conn_type = [{"type": "LAN", "text": "<#tm_wired#>"}];
			return conn_type;
		}
		else{
			if(_sdn_data.sdn_rl.wifi7_onoff == "1") conn_type.push({"type": "WiFi7", "text": `WiFi7`});
			if(_sdn_data.sdn_rl.wifi_sched_on == "0"){
				conn_type.push({"type": "WiFi_offline", "text": "<#tm_wireless#> (<#Clientlist_OffLine#>)"});
			}
			else{
				conn_type.push({"type": "WiFi", "text": "<#tm_wireless#>"});
			}
			return conn_type;
		}
	}
}
function Get_Component_Wizard_Item(_parm){
	var $profile_item_container = $("<div>").addClass("profile_item_container wizard " + _parm.type + "");
	$profile_item_container.unbind("click").click(function(e){
		e = e || event;
		e.stopPropagation();
		show_popup_Wizard_Setting(_parm.type);
	});

	var $item_text_container = $("<div>").addClass("item_text_container");
	$item_text_container.appendTo($profile_item_container);

	$("<div>").addClass("main_info").html(htmlEnDeCode.htmlEncode(_parm.title)).appendTo($item_text_container);
	if(_parm.desc != "")
		$("<div>").addClass("sub_info").html(htmlEnDeCode.htmlEncode(_parm.desc)).appendTo($item_text_container);

	var $item_action_container = $("<div>").addClass("item_action_container").appendTo($profile_item_container);
	$("<div>").addClass("icon_add").appendTo($item_action_container);

	return $profile_item_container;
}
function Get_Component_Wizard_Item_Customized(){
	var $profile_item_container = $("<div>").addClass("profile_item_container wizard customized");
	$profile_item_container.unbind("click").click(function(e){
		e = e || event;
		e.stopPropagation();
		show_popup_Wizard_Setting("Customized");
	});
	var $item_text_container = $("<div>").addClass("item_text_container");
	$item_text_container.appendTo($profile_item_container);

	$("<div>").addClass("main_info").html("<#GuestNetwork_Customized#>").appendTo($item_text_container);

	var $item_action_container = $("<div>").addClass("item_action_container");
	$item_action_container.appendTo($profile_item_container);
	$("<div>").addClass("icon_add").appendTo($item_action_container);

	return $profile_item_container;
}
function Get_Component_Wizard_Item_Scenario(){
	var $profile_item_container = $("<div>").addClass("profile_item_container wizard scenario").attr({"data-container": "wizard_scenario"});
	var $item_text_container = $("<div>").addClass("item_text_container");
	$item_text_container.appendTo($profile_item_container);

	$("<div>").addClass("main_info").html("<#GuestNetwork_Scenarios_Exp#>").appendTo($item_text_container);

	var $item_action_container = $("<div>").addClass("item_action_container");
	$item_action_container.appendTo($profile_item_container);
	$("<div>").addClass("icon_add").appendTo($item_action_container);

	return $profile_item_container;
}
function Get_Component_Scenarios_Explorer(view_mode){
	var $container = $("<div>").addClass("setting_content_container");

	if(view_mode == "popup"){
		Get_Component_Popup_Profile_Title("<#GuestNetwork_Scenarios_Exp#>").addClass("Scenarios").appendTo($container)
			.find("#title_close_btn").unbind("click").click(function(e){
				e = e || event;
				e.stopPropagation();
				close_popup_container($container);
			});
	}

	var $content_container = $("<div>").appendTo($container).addClass("profile_setting");
	if(view_mode == "popup")
		$content_container.addClass("popup_content_container");
	else
		$content_container.addClass("no_popup_content_container");

	$content_container.addClass("Scenarios").appendTo($container);
	var $title = $("<div>").addClass("title btn_back_container").appendTo($content_container)
		.unbind("click").click(function(e){
			e = e || event;
			e.stopPropagation();
			var $parent_cntr = $container.parent();
			var $popup_container = $container.closest(".popup_container");
			var hide_title_flag = $popup_container.hasClass("hide_title_cntr");
			$parent_cntr.find(".setting_content_container").replaceWith(show_Get_Start(view_mode));
			if(hide_title_flag){
				$parent_cntr.find(".setting_content_container .popup_title_container").hide();
			}
			if(isMobile()){
				$popup_container.addClass("full_width");
			}
			resize_iframe_height();
		});
	$("<div>").addClass("icon_arrow_left").appendTo($title);
	$("<div>").html(htmlEnDeCode.htmlEncode("<#btn_Back#>")).appendTo($title);

	var $item_scenarios_cntr = $("<div>").addClass("item_scenarios_container").appendTo($content_container);
	$("<div>").addClass("title").html("<#GuestNetwork_SDN_desc#>").appendTo($item_scenarios_cntr);
	$.each(scenarios_list, function(index, scenariosItem){
		Get_Component_Scenarios(scenariosItem).appendTo($item_scenarios_cntr);
	});
	function Get_Component_Scenarios(_scenariosItem){
		var $scenarios_cntr = $("<div>").addClass("scenarios_container");
		var $scenarios_comp = $("<div>").addClass("scenarios_component").appendTo($scenarios_cntr).unbind("click").click(function(e){
			e = e || event;
			e.stopPropagation();
			if(_scenariosItem.type != undefined && _scenariosItem.type != "")
				show_popup_Wizard_Setting(_scenariosItem.type);
		});
		var $scenes = $("<div>").addClass("scenes").appendTo($scenarios_comp);
		if(_scenariosItem.source != undefined && _scenariosItem.source == "Cloud")
			$scenes.css("background-image", "url(" + _scenariosItem.scenes + ")");
		else
			$scenes.addClass(_scenariosItem.scenes);
		var $desc = $("<div>").addClass("desc").appendTo($scenarios_comp);
		$("<div>").html(_scenariosItem.text).appendTo($desc);
		return $scenarios_cntr;
	}

	return $container;
}
function Get_Component_Type_Scenarios(view_mode){
	var $container = $("<div>").addClass("setting_content_container");

	if(view_mode == "popup"){
		Get_Component_Popup_Profile_Title("<#GuestNetwork_Scenarios_Exp#>").addClass("Scenarios").appendTo($container)
			.find("#title_close_btn").unbind("click").click(function(e){
				e = e || event;
				e.stopPropagation();
				close_popup_container($container);
			});
	}

	var $content_container = $("<div>").appendTo($container).addClass("profile_setting");
	if(view_mode == "popup")
		$content_container.addClass("popup_content_container");
	else
		$content_container.addClass("no_popup_content_container");

	$content_container.addClass("Scenarios").appendTo($container);

	var wizard_type_list = [
		{"type":"Employee", "title":"<#GuestNetwork_Employee#>", "desc":str_GuestNetwork_Wizard_type_desc0+" "+str_GuestNetwork_Employee_recommend},
		{"type":"Portal", "title":"<#GuestNetwork_Portal#>", "desc":str_GuestNetwork_Portal_desc2},
		{"type":"Guest", "title":"<#Guest_Network#>", "desc":"<#GuestNetwork_Guests_desc#>"},
		{"type":"Sched", "title":"<#GuestNetwork_Sche_network#>", "desc":str_GuestNetwork_WiFi_Sche_desc},
		{"type":"IoT", "title":"<#GuestNetwork_IoT#>", "desc":"<#GuestNetwork_IoT_desc#>"}
	];

	var $wizard_list_bg = $("<div>").addClass("wizard_list_bg type_scenarios").appendTo($content_container);
	$.each(wizard_type_list, function(index, item){
		var $profile_item_container = $("<div>").addClass("profile_item_container wizard type_scenarios " + item.type + "")
			.appendTo($wizard_list_bg).unbind("click").click(function(e){
				e = e || event;
				e.stopPropagation();
				show_popup_Wizard_Setting(item.type);
			});
		var $item_text_container = $("<div>").addClass("item_text_container").appendTo($profile_item_container);
		$("<div>").addClass("main_info").html(htmlEnDeCode.htmlEncode(item.title)).appendTo($item_text_container);
		$("<div>").addClass("sub_info").html(htmlEnDeCode.htmlEncode(item.desc)).appendTo($item_text_container);

		var $item_scenarios_container = $("<div>").addClass("item_scenarios_container").appendTo($profile_item_container);
		$.each(scenarios_list, function(index, scenariosItem){
			if(item.type == scenariosItem.type)
				Get_Component_Scenarios(scenariosItem).appendTo($item_scenarios_container);
		});
	});

	return $container;

	function Get_Component_Scenarios(_scenariosItem){
		var $scenarios_cntr = $("<div>").addClass("scenarios_container");
		var $scenarios_comp = $("<div>").addClass("scenarios_component").appendTo($scenarios_cntr);
		var $scenes = $("<div>").addClass("scenes").appendTo($scenarios_comp);
		if(_scenariosItem.source != undefined && _scenariosItem.source == "Cloud")
			$scenes.css("background-image", "url(" + _scenariosItem.scenes + ")");
		else
			$scenes.addClass(_scenariosItem.scenes);
		var $desc = $("<div>").addClass("desc").appendTo($scenarios_comp);
		$("<div>").html(_scenariosItem.text).appendTo($desc);
		return $scenarios_cntr;
	}
}
function Get_Component_Bandwidth_Setting(_parm){
	var $container = $("<div>").addClass("profile_setting_item bandwidth_set");

	if(_parm.container_id != undefined)
		$container.attr("id", _parm.container_id);

	if(_parm.openHint != undefined){
		var hint_array = _parm.openHint.split("_");
		$("<a>").addClass("hintstyle").attr({"href":"javascript:void(0);"}).html(htmlEnDeCode.htmlEncode(_parm.title)).unbind("click").click(function(){
			openHint(hint_array[0], hint_array[1], "rwd_vpns");
		}).appendTo($("<div>").addClass("title").appendTo($container));
	}
	else
		$("<div>").addClass("title").html(htmlEnDeCode.htmlEncode(_parm.title)).appendTo($container);

	var set_value_1 = "";
	if(_parm.set_value_1 != undefined)
		set_value_1 = _parm.set_value_1;

	var set_value_2 = "";
	if(_parm.set_value_2 != undefined)
		set_value_2 = _parm.set_value_2;

	var $bandwidth_container = $("<div>").addClass("bandwidth_container").appendTo($container);

	var $download = $("<div>").addClass("bandwidth_item").appendTo($bandwidth_container);
	$("<div>").addClass("BW_icon icon_download").appendTo($download);
	var $input_1 = $("<input/>")
						.addClass("textInput")
						.attr({"id":_parm.id_1, "type":_parm.type_1, "maxlength":_parm.maxlength_1, "autocomplete":"off","autocorrect":"off","autocapitalize":"off","spellcheck":"false"})
						.val(set_value_1)
						.unbind("blur").blur(function(e){
							e = e || event;
							e.stopPropagation();
						}).on('click', function () {
							var target = this;
							setTimeout(function(){
								target.scrollIntoViewIfNeeded();
							},400);
						})
						.appendTo($download);
	if(_parm.need_check_1)
		$input_1.attr("need_check", true);

	var $upload = $("<div>").addClass("bandwidth_item").appendTo($bandwidth_container);
	$("<div>").addClass("BW_icon icon_upload").appendTo($upload);
	var $input_2 = $("<input/>")
						.addClass("textInput")
						.attr({"id":_parm.id_2, "type":_parm.type_2, "maxlength":_parm.maxlength_2, "autocomplete":"off","autocorrect":"off","autocapitalize":"off","spellcheck":"false"})
						.val(set_value_2)
						.unbind("blur").blur(function(e){
							e = e || event;
							e.stopPropagation();
						})
						.on('click', function () {
							var target = this;
							setTimeout(function(){
								target.scrollIntoViewIfNeeded();
							},400);
						})
						.appendTo($upload);

	if(_parm.need_check_2)
		$input_2.attr("need_check", true);

	return $container;
}
function Get_Component_Print_Btn(_parm){
	var $container = $("<div>").addClass("profile_setting_item nowrap switch_item");
	if(_parm.container_id != undefined)
		$container.attr("id", _parm.container_id);

	if(_parm.openHint != undefined){
		var hint_array = _parm.openHint.split("_");
		$("<a>").addClass("hintstyle").attr({"href":"javascript:void(0);"}).html(htmlEnDeCode.htmlEncode(_parm.title)).unbind("click").click(function(){
			openHint(hint_array[0], hint_array[1], "rwd_vpns");
		}).appendTo($("<div>").addClass("title").appendTo($container));
	}
	else
		$("<div>").addClass("title").html(htmlEnDeCode.htmlEncode(_parm.title)).appendTo($container);

	var $input_container = $("<div>").addClass("input_container").appendTo($container);
	var $icon_print = $("<div>").addClass("icon_print").appendTo($input_container);
	if(_parm.id != undefined)
		$icon_print.attr("id", _parm.id);

	return $container;
}
function Get_Container_Assign_DNS(_dns_list, _callback){
	var $container = $("<div>").addClass("setting_content_container");

	Get_Component_Popup_Profile_Title("<#IPConnection_x_DNS_List#>").appendTo($container)
		.find("#title_close_btn").unbind("click").click(function(e){
			e = e || event;
			e.stopPropagation();
			close_popup_container($container);
		});

	var $content_container = $("<div>").addClass("popup_content_container profile_setting").appendTo($container);

	var $DNS_List = Get_Component_DNS_List().appendTo($content_container);
	Get_Component_DNS_Manual().appendTo($DNS_List);

	var $action_container = $("<div>").addClass("action_container").appendTo($content_container);
	$("<div>").addClass("btn_container apply").html("<#CTL_ok#>").unbind("click").click(function(e){
		e = e || event;
		e.stopPropagation();
		if(validate_format($container)){
			var dns_list = {"dns1":"", "dns2":""};
			var $clicked_dns = $content_container.find("[data-container=DNS_List] .rwd_icon_radio.clicked");
			if($clicked_dns.attr("data-list-type") == "DB"){
				dns_list.dns1 = $clicked_dns.attr("data-dns1");
				dns_list.dns2 = $clicked_dns.attr("data-dns2");
			}
			else if($clicked_dns.attr("data-list-type") == "Manual"){
				$clicked_dns.attr({"data-dns1":$content_container.find("#dns1").val(), "data-dns2":$content_container.find("#dns2").val()});
				dns_list.dns1 = $clicked_dns.attr("data-dns1");
				dns_list.dns2 = $clicked_dns.attr("data-dns2");
			}
			_callback(dns_list);
			close_popup_container($container);
		}

		function validate_format(_obj){
			jQuery.fn.show_validate_hint_DNS = function(hintStr){
				$(this).closest(".profile_setting").find(".validate_hint").remove();

				$("<div>")
					.html(hintStr)
					.addClass("validate_hint")
					.insertAfter($(this).closest(".dns_list_container"));
					resize_iframe_height();
			}
			var valid_is_IP_format = function(str, type){
				var testResultPass = {
					'isError': false,
					'errReason': ''
				};
				var testResultFail = {
					'isError': true,
					'errReason': str + " <#JS_validip#>"
				};
				var format = new RegExp(ip_RegExp[type], "gi");
				if(format.test(str))
					return testResultPass;
				else
					return testResultFail;
			};
			$(_obj).find(".validate_hint").remove();
			var $clicked_dns = $(_obj).find("[data-container=DNS_List] .rwd_icon_radio.clicked");
			if($clicked_dns.attr("data-list-type") == "Manual"){
				var $dns1 = $(_obj).find("#dns1");
				var $dns2 = $(_obj).find("#dns2");
				$dns1.val($dns1.val().replace(/\s+/g, ''));//remove space
				$dns2.val($dns2.val().replace(/\s+/g, ''));//remove space
				if($dns1.val() != ""){
					var isValid_dns1 = valid_is_IP_format($dns1.val(), "IPv4");
					if(isValid_dns1.isError){
						$dns1.show_validate_hint_DNS(isValid_dns1.errReason);
						$dns1.focus();
						return false;
					}
				}
				if($dns2.val() != ""){
					var isValid_dns2 = valid_is_IP_format($dns2.val(), "IPv4");
					if(isValid_dns2.isError){
						$dns2.show_validate_hint_DNS(isValid_dns2.errReason);
						$dns2.focus();
						return false;
					}
				}
			}

			return true;
		}
	}).appendTo($action_container);

	if(_dns_list.dns1 != "" || _dns_list.dns2 != ""){
		var $specific_dns = $content_container.find("[data-dns1='"+_dns_list.dns1+"'][data-dns2='"+_dns_list.dns2+"']");
		if($specific_dns.length > 0){
			$specific_dns.addClass("clicked");
		}
		else{
			var $manual_dns = $content_container.find("[data-list-type='Manual']");
			$manual_dns.attr({"data-dns1":_dns_list.dns1, "data-dns2":_dns_list.dns2}).addClass("clicked");
			$content_container.find("#dns1").val(_dns_list.dns1);
			$content_container.find("#dns2").val(_dns_list.dns2);
		}
	}
	else{
		$content_container.find("[data-list-type=Default]").addClass("clicked");
	}

	return $container;

	function Get_Component_DNS_List(){
		var $container = $("<div>").attr({"data-container":"DNS_List"});
		Object.keys(dns_list_data).forEach(function(FilterMode){
			var FilterMode_arr = dns_list_data[FilterMode];
			if(FilterMode_arr.length > 0){
				var $dns_list_cntr = $("<div>").addClass("dns_list_container").appendTo($container);
				var $cate_title = $("<div>").addClass("category_title").html(htmlEnDeCode.htmlEncode(FilterMode)).appendTo($dns_list_cntr);
				var $item_title_cntr = $("<div>").addClass("item_container").appendTo($dns_list_cntr);
				$("<div>").addClass("radio").appendTo($item_title_cntr);
				$("<div>").addClass("service").html("<#qis_service#>").appendTo($item_title_cntr);
				$("<div>").addClass("server").html("<#HSDPAConfig_DNSServers_itemname#>").appendTo($item_title_cntr);
				$("<div>").addClass("desc").html("<#Description#>").appendTo($item_title_cntr);
				$.each(FilterMode_arr, function(index, dns_item){
					if(dns_item.confirmed == "Yes"){
						var $item_title_cntr = $("<div>").addClass("item_container dns_content").appendTo($dns_list_cntr);
						var $icon_radio = $("<div>").addClass("rwd_icon_radio")
							.attr({"data-dns1":dns_item.ServiceIP1, "data-dns2":dns_item.ServiceIP2, "data-list-type":"DB"})
							.unbind("click").click(function(e){
								e = e || event;
								e.stopPropagation();
								$(this).closest("[data-container=DNS_List]").find(".rwd_icon_radio").removeClass("clicked");
								$(this).addClass("clicked");
							});
						$("<div>").addClass("radio").append($icon_radio).appendTo($item_title_cntr);
						$("<div>").addClass("service").html(htmlEnDeCode.htmlEncode(dns_item.DNSService)).appendTo($item_title_cntr);
						$("<div>").addClass("server").html(htmlEnDeCode.htmlEncode(dns_item.ServiceIP1) + "<br>" + htmlEnDeCode.htmlEncode(dns_item.ServiceIP2)).appendTo($item_title_cntr);
						var desc = "";
						if(dns_item.Description != undefined){
							desc = htmlEnDeCode.htmlEncode(dns_item.Description);
						}
						if(dns_item.url != undefined){
							desc += "&nbsp;";
							desc += $("<a/>").attr({"href":dns_item.url, "target":"_blank"}).css({"text-decoration":"underline"}).html("<b><#Learn_more#></b>")[0].outerHTML;
						}
						$("<div>").addClass("desc").html(desc).appendTo($item_title_cntr);
					}
				});
			}
		});
		return $container
	}
	function Get_Component_DNS_Manual(){
		var $dns_list_cntr = $("<div>").addClass("dns_list_container manual_set");
		var $cate_title = $("<div>").addClass("category_title").html("<#Manual_Setting_btn#>").appendTo($dns_list_cntr);
		var $item_title_cntr = $("<div>").addClass("item_container").appendTo($dns_list_cntr);
		$("<div>").addClass("radio").appendTo($item_title_cntr);
		$("<div>").addClass("service").html("<#qis_service#>").appendTo($item_title_cntr);
		$("<div>").addClass("server").html("<#HSDPAConfig_DNSServers_itemname#>").appendTo($item_title_cntr);
		var $item_title_cntr = $("<div>").addClass("item_container dns_content").appendTo($dns_list_cntr);
		var $icon_radio = $("<div>").addClass("rwd_icon_radio")
			.attr({"data-dns1":"", "data-dns2":"", "data-list-type":"Manual"})
			.unbind("click").click(function(e){
				e = e || event;
				e.stopPropagation();
				$(this).closest("[data-container=DNS_List]").find(".rwd_icon_radio").removeClass("clicked");
				$(this).addClass("clicked");
			});
		$("<div>").addClass("radio").append($icon_radio).appendTo($item_title_cntr);
		var $service = $("<div>").addClass("service").appendTo($item_title_cntr);
		$("<div>").html("<#IPConnection_x_DNSServer1_itemname#>").appendTo($service);
		$("<div>").html("<#IPConnection_x_DNSServer2_itemname#>").appendTo($service);
		var $server = $("<div>").addClass("server").appendTo($item_title_cntr);
		$("<input/>")
			.addClass("textInput")
			.attr({"id":"dns1", "type":"text", "maxlength":15, "autocomplete":"off","autocorrect":"off","autocapitalize":"off","spellcheck":"false"})
			.on('click', function () {
				$icon_radio.click();
				var target = this;
				setTimeout(function(){
					target.scrollIntoViewIfNeeded();//for mobile view
				},400);
			})
			.unbind("keypress").keypress(function(){
				return validator.isIPAddr(this, event);
			})
			.appendTo($server);
		$("<input/>")
			.addClass("textInput")
			.attr({"id":"dns2", "type":"text", "maxlength":15, "autocomplete":"off","autocorrect":"off","autocapitalize":"off","spellcheck":"false"})
			.on('click', function () {
				$icon_radio.click();
				var target = this;
				setTimeout(function(){
					target.scrollIntoViewIfNeeded();//for mobile view
				},400);
			})
			.unbind("keypress").keypress(function(){
				return validator.isIPAddr(this, event);
			})
			.appendTo($server);

		var $default_item_title_cntr = $("<div>").addClass("item_container dns_content default").appendTo($dns_list_cntr);
		var $default_icon_radio = $("<div>").addClass("rwd_icon_radio")
			.attr({"data-dns1":"", "data-dns2":"", "data-list-type":"Default"})
			.unbind("click").click(function(e){
				e = e || event;
				e.stopPropagation();
				$(this).closest("[data-container=DNS_List]").find(".rwd_icon_radio").removeClass("clicked");
				$(this).addClass("clicked");
			});
		$("<div>").addClass("radio").append($default_icon_radio).appendTo($default_item_title_cntr);
		$("<div>").addClass("service").html("<#Setting_factorydefault_value#>").appendTo($default_item_title_cntr);

		return $dns_list_cntr;
	}
}
function Get_FWF_Change_BG_Container(){
	var $container = $("<div>").attr({"id":"FWF_Change_BG_Cntr"});

	var $btn_back_cntr = $("<div>").addClass("btn_back_container").appendTo($container)
		.unbind("click").click(function(e){
			e = e || event;
			e.stopPropagation();
			var $FWF_ui_items = $(this).closest("#FWF_ui_container").find("#FWF_Preview_Cntr, #FWF_Change_BG_Cntr");
			$FWF_ui_items.eq(0).show();
			$FWF_ui_items.eq(1).hide();
			resize_iframe_height();
		});
	$("<div>").addClass("icon_arrow_left").appendTo($btn_back_cntr);
	$("<div>").html(htmlEnDeCode.htmlEncode("<#btn_Back#>")).appendTo($btn_back_cntr);

	var $bg_template_cntr = $("<div>").addClass("bg_template_container").appendTo($container);
	$.each(FreeWiFi_template, function(index, value){
		$("<div>").addClass("bg_template")
			.css('background-image', 'url(' + value.image + ')')
			.unbind("click").click(function(e){
				e = e || event;
				e.stopPropagation();
				/*
				var $this_parent = $(this).parent(".bg_template_cntr");
				$(this).addClass("clicked").parent(".bg_template_cntr").siblings(".bg_template_cntr").children(".bg_template").not($this_parent).removeClass("clicked");
				*/
				var image = $(this).css('background-image').replace('url(','').replace(')','').replace(/\"/gi, "");
				var $FWF_ui_items = $(this).closest("#FWF_ui_container").find("#FWF_Preview_Cntr, #FWF_Change_BG_Cntr");
				$FWF_ui_items.eq(0).show().find("[data-component=FWF_bg]").css({"background-image": 'url(' + image + ')'});
				$FWF_ui_items.eq(1).hide();
			}).appendTo($("<div>").addClass("bg_template_cntr").appendTo($bg_template_cntr));
	});

	Get_Upload_Image_Container(function(_image){
		var $FWF_ui_items = $bg_template_cntr.closest("#FWF_ui_container").find("#FWF_Preview_Cntr, #FWF_Change_BG_Cntr");
		$FWF_ui_items.eq(0).show().find("[data-component=FWF_bg]").css({"background-image": 'url(' + _image + ')'});
		$FWF_ui_items.eq(1).hide();
	}).appendTo($("<div>").addClass("bg_template_cntr").appendTo($bg_template_cntr));

	return $container;
}
function Get_MB_Change_BG_Container(){
	var $container = $("<div>").attr({"id":"MB_Change_BG_Cntr"});

	var $btn_back_cntr = $("<div>").addClass("btn_back_container").appendTo($container)
		.unbind("click").click(function(e){
			e = e || event;
			e.stopPropagation();
			var $MB_ui_items = $(this).closest("#MB_ui_container").find("#MB_Preview_Cntr, #MB_Change_BG_Cntr");
			$MB_ui_items.eq(0).show();
			$MB_ui_items.eq(1).hide();
			resize_iframe_height();
		});
	$("<div>").addClass("icon_arrow_left").appendTo($btn_back_cntr);
	$("<div>").html(htmlEnDeCode.htmlEncode("<#btn_Back#>")).appendTo($btn_back_cntr);

	var $bg_template_cntr = $("<div>").addClass("bg_template_container MB").appendTo($container);
	$.each(MessageBoard_template, function(index, value){
		$("<div>").addClass("bg_template")
			.css('background-image', 'url(' + value.image + ')')
			.unbind("click").click(function(e){
				e = e || event;
				e.stopPropagation();
				var image = $(this).css('background-image').replace('url(','').replace(')','').replace(/\"/gi, "");
				var $MB_ui_items = $(this).closest("#MB_ui_container").find("#MB_Preview_Cntr, #MB_Change_BG_Cntr");
				$MB_ui_items.eq(0).show().find("[data-component=MB_bg]").css({"background-image": 'url(' + image + ')'});
				$MB_ui_items.eq(1).hide();
			}).appendTo($("<div>").addClass("bg_template_cntr").appendTo($bg_template_cntr));
	});

	Get_Upload_Image_Container(function(_image){
		var $MB_ui_items = $bg_template_cntr.closest("#MB_ui_container").find("#MB_Preview_Cntr, #MB_Change_BG_Cntr");
		$MB_ui_items.eq(0).show().find("[data-component=MB_bg]").css({"background-image": 'url(' + _image + ')'});
		$MB_ui_items.eq(1).hide();
	}).appendTo($("<div>").addClass("bg_template_cntr").appendTo($bg_template_cntr));

	return $container;
}
function Get_FWF_Preview_Container(){
	//default_FWF_template = FreeWiFi_template[4]
	let default_FWF_template = "data:image/jpeg;base64,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1l0TQJfzTIvhhLch+NJIH0+0kHc5t1lVd6+om3E0HGRPFDnQjG+sr6/MQzznwaBbyTwByRCTt9vdQBefzzzzzyx3g+TKsvVZJ6+Iqa6blPoIFiA9goupsDccHiZ/zHvIugQwx8DYaY7xCCMOKrbcWmmlocrNBTq3JJJRo0F8GtwR/Hs+FkgxgQn3cPS+dPM4ZBoywnzzzzzzzzzy2v8Aki/jodXZAp6Q8kQsEzXWbNjLhVFaXbSz/roTnF+88s4WQ1WqmuzO6wWnlZZFyKDw/oYX197ElfFDU7LTHXsLzDf9pr33MAkjzz4swdcgcB888888888888IHPtcHt2LfcfUheYtq6q1UX0AoQv78UNfKk47Lckc88s88U3uqNNU8X6np19Nwi3tmnuGnt7Iah5Agg4SberjPD/bwocAIkcqeugrfMcjTF38888888888888t2TPxxTOYTqSxccsxxLyx/h2wENdvXI2bfnNkA28888888MwbQwDv3nBBm5iG3rquYLXfM/8AI/fR6ML4WvCZm/2LDpjBICJHJGgnfM63DOfPPPPPPPPPPfPPLD7LHKFH9dwMKmZQwunb7+Wfx8opgMCX3ROBjJmnvPPPPPPPLBCSvuZGBBNc0jwIHHLw54Dw2/8AzD5L/LX/ABMU85hoME8lwcMgMWfXgmyNS88888889989888sYVU2gnxSv6Uvq9eSMogJ6e/Sv3DuEQDJYcjj862888888888s4P0quCN/NNRSfQAEIRxAzjJZCKCRLpM4wQP2Xn1Hp9IwM0q7q+iInw9V888888898888888EJ8r5Xz60TS40rvL97neZEWmeSGUkK1iPwgy+m8888888888888wSBWocY0U82Yw4wusMY2BNVCbzmNJ1tpnfFnMX+8R8O7X+YsI+nFz1888889889988998Yf8AvpB9/wB8KOhHFEwvAaiyYo6hgxT3XPTlN5gJzzzzzzzzzzzzzzyuLRgie0PP18RA5AAZ7Wa7NZI7FRF/DxhSiNk1iRLqr66T6q6KwfvHzzzzz333zz3333j24t6SNOt+0hDdyR+OpZwCMe90c2E1NbCTxzzzzzzzzzzzzzzzzzxSDzrBEokgPKTzvD5w557ab7dbla2GNMx72hjBafqro6I+Lhjjzuyjzzz33zz3z3333339g3rjNUzPmiXxQ/NNmubBsufvl11tdvtTTzzzzzzzzzzzzzzzzzxQ/tzWz3GdzzgNi5y360Ypn4kk3vstuiYY+/SA0mWwrrZZx5xhueU3Xz33z3z33333321thXPsg4DUzfVwgBuBaso4yWO/XHTJuGtvzzzzzzzzzzzzzzzzzzxCD9HFX46qLF0y125Jnn7LIZmcfzDjwQJzlkKJlEEm23m2IsmPgsyXv33333333333318SRwuCkL4jcvUULbIqD9vXCdcq01ltJMRtjTzzzzzzzzzzzzzzzzyK6BW0FGHHXX300VXHGFGEJ46pbp7Jpqa4646JjEXm33wjaeUv/hSGx/3333333333l/F7Gl7vT2ftKmV10Fc6QBu7oxVkn/yMQkl+xPLzzzzzzzzzzzzzzzyD3nEU33mFEEEEE0333x2gkEABTjZZZbqI4445Sz2027tA9oYWH6ejj/P33333333lfPZJxr/wxlBWVVk3kXVVEWeG9PWPfGyCAiDSCjMvzzzzzzzzzzzzzzxziF3X33EE30EE33T33X3GW1G1F5KrLLLJJrL57hADnltP1Df1Iej+yRNv9P3333ONCPSeskBx1MVSCRheQQ2A2UVbekkdbUdju/cwRwoKpDTzzzzzzzzzzzzygW133EE0UE13333330U000nY5LI7ba57765rK8L79trDilMwbz1MiNTfaO/utxBHyS/3FYkVwFSMLmf6SHAv2DBWPWa7iuG9/m+ySf8ALjs2488888888888sgZhRhV99995F199995xhBFZ2a2+mfKSuiBFi++GGfpA7/rZTptovRH6gug67r/zESn/APPg4Zrdl5zEALWKgB32LdBKs/jJ4+Ay87KOM6/kFmLsPPPPPPPPPJLeVeVffcUYUQcYYUYSTTXcT5louotaijgnomugrq06eZT0kJpC8/yh88GkT+Y6hkIawDakNd9VkHoTNf13WZGjiH/apKECyL62CuMBDzMbfxqjPNPPPPPPPHOJfXFPeQRQQQQSADXbfcRegjmqDjirRAgqnx300zl6sz0+4+uzQQPGKMwzwIIBzx/uX3G6Vcf7bCwbxt/gaAkkiw/Lpezgz/PawfD4G0sRqvXAw3kPPPPPPIKPfPPIQTfZDHDDHcffeR2gnqDjLOpgYAFoq2z8fJCxwnk8w863KsXUoZiUSgpjMHUhm1e7cAIWkseuwTOvTRWw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";
	var $container = $("<div>").attr({"id":"FWF_Preview_Cntr"});
	var $FWF_bg = $("<div>").attr({"data-component":"FWF_bg"}).addClass("FWF_bg").appendTo($container);
	$FWF_bg.css('background-image', 'url(' + default_FWF_template + ')');

	var $FWF_change_bg = $("<div>").addClass("FWF_change_bg").attr({"title":"<#Captive_Portal_Click_Image#>"}).appendTo($FWF_bg);
	$FWF_change_bg.unbind("click").click(function(e){
		e = e || event;
		e.stopPropagation();
		let $FWF_ui_items = $(this).closest("#FWF_ui_container").find("#FWF_Preview_Cntr, #FWF_Change_BG_Cntr");
		let $bg_template_cntr = $FWF_ui_items.eq(1).find(".bg_template_container");
		if(FreeWiFi_template.length > 0 && $FWF_ui_items.eq(1).find(".bg_template_container").children().length <= 1){
			$.each(FreeWiFi_template, function(index, value){
				$("<div>").addClass("bg_template")
					.css('background-image', 'url(' + value.image + ')')
					.unbind("click").click(function(e){
						e = e || event;
						e.stopPropagation();
						let image = $(this).css('background-image').replace('url(','').replace(')','').replace(/\"/gi, "");
						$FWF_ui_items.eq(0).show().find("[data-component=FWF_bg]").css({"background-image": 'url(' + image + ')'});
						$FWF_ui_items.eq(1).hide();
					}).appendTo($("<div>").addClass("bg_template_cntr").insertBefore($bg_template_cntr.children(".bg_template_cntr:last")));
			});
		}
		$FWF_ui_items.eq(0).hide();
		$FWF_ui_items.eq(1).show();
	});
	var $FWF_cntr = $("<div>").addClass("FWF_container").appendTo($FWF_bg);
	var $FWF_portal_cntr = $("<div>").addClass("FWF_portal_cntr").appendTo($FWF_cntr);
	var $FWF_text_info_cntr = $("<div>").addClass("FWF_text_info_cntr").appendTo($FWF_portal_cntr);
	$("<div>").attr({"data-component":"FWF_brand_name"}).addClass("brand_name").html("<#FreeWiFi_BrandName#>").appendTo($FWF_text_info_cntr);
	let $FWF_terms_service = $("<div>").attr({"data-component":"FWF_TS", "data-group":"FWF_TS"}).addClass("terms_service").html(
		htmlEnDeCode.htmlEncode(terms_service_template).replace(/(?:\r\n|\r|\n)/g, '<div style=height:6px;></div>')
	).appendTo($FWF_portal_cntr);

	var $action_info_cntr = $("<div>").addClass("action_info_cntr").appendTo($FWF_portal_cntr);
	var $cb_text_container = $("<div>").addClass("cb_text_container").attr({"data-group":"FWF_TS"}).appendTo($action_info_cntr);
	$("<div>").addClass("cb_icon clicked").appendTo($cb_text_container);
	$("<div>").addClass("cb_text").html("<#FreeWiFi_Agree_Terms_Service#>").appendTo($cb_text_container);
	$("<div>").attr({"data-component":"FWF_passcode"}).addClass("passcode_container").html("Enter Passcode").appendTo($action_info_cntr);/* untranslated */
	$("<div>").addClass("btn_container").html("<#FreeWiFi_Continue#>").appendTo($("<div>").addClass("action_container").appendTo($action_info_cntr))
	return $container;
}
function Get_MB_Preview_Container(){
	//default_MB_template = MessageBoard_template[0]
	let default_MB_template = "data:image/jpeg;base64,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";
	var $container = $("<div>").attr({"id":"MB_Preview_Cntr"});
	var $MB_bg = $("<div>").attr({"data-component":"MB_bg"}).addClass("MB_bg").appendTo($container);
	$MB_bg.css('background-image', 'url(' + default_MB_template + ')');

	var $MB_change_bg = $("<div>").addClass("MB_change_bg").attr({"title":"<#Captive_Portal_Click_Image#>"}).appendTo($MB_bg);
	$MB_change_bg.unbind("click").click(function(e){
		e = e || event;
		e.stopPropagation();
		let $MB_ui_items = $(this).closest("#MB_ui_container").find("#MB_Preview_Cntr, #MB_Change_BG_Cntr");
		let $bg_template_cntr = $MB_ui_items.eq(1).find(".bg_template_container");
		if(MessageBoard_template.length > 0 && $MB_ui_items.eq(1).find(".bg_template_container").children().length <= 1){
			$.each(MessageBoard_template, function(index, value){
				$("<div>").addClass("bg_template")
					.css('background-image', 'url(' + value.image + ')')
					.unbind("click").click(function(e){
						e = e || event;
						e.stopPropagation();
						let image = $(this).css('background-image').replace('url(','').replace(')','').replace(/\"/gi, "");
						$MB_ui_items.eq(0).show().find("[data-component=MB_bg]").css({"background-image": 'url(' + image + ')'});
						$MB_ui_items.eq(1).hide();
					}).appendTo($("<div>").addClass("bg_template_cntr").insertBefore($bg_template_cntr.children(".bg_template_cntr:last")));
			});
		}
		$MB_ui_items.eq(0).hide();
		$MB_ui_items.eq(1).show();
	});
	var $MB_cntr = $("<div>").addClass("MB_container").appendTo($MB_bg);
	var $MB_portal_cntr = $("<div>").addClass("MB_portal_cntr").appendTo($MB_cntr);
	var $MB_text_info_cntr = $("<div>").addClass("MB_text_info_cntr").appendTo($MB_portal_cntr);
	$("<div>").attr({"data-component":"MB_desc"}).addClass("MB_desc").html("<#Description#>").appendTo($MB_text_info_cntr);
	var $action_info_cntr = $("<div>").addClass("action_info_cntr").appendTo($MB_portal_cntr);
	$("<div>").addClass("btn_container").html("<#FreeWiFi_Continue#>").appendTo($("<div>").addClass("action_container").appendTo($action_info_cntr))
	return $container;
}
function Get_Upload_Image_Container(_callback){
	var $bg_upload_image = $("<div>").addClass("bg_upload_image").attr({"title":"<#FreeWiFi_Upload_Image#>"});
	$("<input/>").attr({"type":"file", "accept":".jpg,.jpeg,.png,.svg"}).addClass("bg_upload_file")
		.on("click", function(){
			$(this).val("");
		})
		.on("change", function(){
			var $file = $(this);
			if(!checkImageExtension($file.val())){
				show_customize_alert("<#Setting_upload_hint#>");
				return;
			}
			var fileReader = new FileReader();
			fileReader.onload = function(fileReader){
				var source_image_size = 0;
				if((fileReader.total != undefined) && (!isNaN(fileReader.total)))
					source_image_size = fileReader.total;
				if(Math.round(source_image_size / 1024) > 10240){
					show_customize_alert("<#FreeWiFi_Image_Size_Alert#>");
					return;
				}
				let $img = $("<img/>").attr("src", fileReader.target.result);
				$img.load(function(){
					let img_w = parseInt($(this)[0].naturalWidth);
					let img_h = parseInt($(this)[0].naturalHeight);
					let canvas_parm = {"w":1280, "h":720};
					let di_parm = {
						"sx":0, "sy":0, "sWidth":canvas_parm.w, "sHeight":canvas_parm.h,
						"dx":0, "dy":0, "dWidth":canvas_parm.w, "dHeight":canvas_parm.h,
						"set_s_parm": false
					};
					if(!isNaN(img_h) && !isNaN(img_w) && img_h > 0 && img_w > 0){
						if(img_h > img_w){
							di_parm.set_s_parm = true;
							di_parm.sWidth = img_w;
							di_parm.sHeight = parseInt(img_w/1.77);
							di_parm.sy = parseInt(img_h/2) - parseInt(di_parm.sHeight/2)
							if(di_parm.sy < 0)
								di_parm.sy = 0;
						}
						else{
							let ratio = Math.floor(img_w/img_h * 100) / 100;
							if(ratio != 1.77){
								canvas_parm.h = parseInt(canvas_parm.w / ratio);
							}
						}
					}
					let mimeType = $(this).attr("src").split(",")[0].split(":")[1].split(";")[0];
					let $canvas = $("<canvas/>").attr({"width":canvas_parm.w, "height":canvas_parm.h});
					let ctx = $canvas[0].getContext("2d");
					ctx.clearRect(0, 0, canvas_parm.w, canvas_parm.h);
					setTimeout(function(){
						if(di_parm.set_s_parm){
							ctx.drawImage(
								$img[0],
								di_parm.sx, di_parm.sy, di_parm.sWidth, di_parm.sHeight,
								di_parm.dx, di_parm.dy, di_parm.dWidth, di_parm.dHeight
							);
						}
						else{
							ctx.drawImage($img[0],0,0,canvas_parm.w, canvas_parm.h);
						}
						let dataURL = $canvas[0].toDataURL(mimeType);
						if(Math.round(dataURL.length / 1024) > 2048){
							show_customize_alert("<#FreeWiFi_Image_Size_Compressed_Alert#>");
							return;
						}
						else{
							if(typeof _callback == "function")
								_callback(dataURL);
						}
					}, 100);
				});
			}
			fileReader.readAsDataURL($file.prop("files")[0]);
	}).appendTo($bg_upload_image);

	return $bg_upload_image;
}
function show_feature_desc(){
	$(".container").addClass("blur_effect");
	if($(".popup_container.popup_element").css("display") == "flex"){
		$(".popup_container.popup_element").addClass("blur_effect");
	}
	$(".popup_element_second").css("display", "flex");
	$(".popup_container.popup_element_second").empty();

	$(".popup_container.popup_element_second").append(Get_Component_Feature_Desc());
	adjust_popup_container_top($(".popup_container.popup_element_second"), 100);
	resize_iframe_height();

	function Get_Component_Feature_Desc(){
		let wizard_type_list = [
			{"type":"Employee", "title":"<#GuestNetwork_Employee#>", "desc":str_GuestNetwork_Wizard_type_desc0+" "+str_GuestNetwork_Employee_recommend},
			{"type":"Portal", "title":"<#GuestNetwork_Portal#>", "desc":str_GuestNetwork_Portal_desc2+" "+stringSafeGet("<#GuestNetwork_Portal_desc3#>")},
			{"type":"Guest", "title":"<#Guest_Network#>", "desc":"<#GuestNetwork_Guests_desc#> "+stringSafeGet("<#GuestNetwork_Guests_desc2#>")},
			{"type":"Kids", "title":"<#GuestNetwork_Kid#>", "desc":str_GuestNetwork_WiFi_Sche_desc+" "+stringSafeGet("<#GuestNetwork_WiFi_Sche_desc2#>")},
			{
				"type":"IoT","title":`<#GuestNetwork_IoT#>`,
				"desc":`
					Easily separate a network for your IoT devices. Recommended for surveillance devices, voice assistance, light fixtures, doorbell cam, smart lock, and sensors.
					<br>
					To access and control the IoT devices within your home automation ecosystem,
					enable the "<#GuestNetwork_subnet_as_main#>" option. This is perfect for everyday control devices that require frequent monitoring and management.
					<br>
					To enhance security, disable the "<#GuestNetwork_subnet_as_main#>" option and establish a dedicated,
					isolated network that shields your IoT devices from external threats.
				`/* untranslated */
			},
			{"type":"VPN", "title":"<#GuestNetwork_VPN#>", "desc":stringSafeGet("<#GuestNetwork_VPN_desc2#>")},
			{"type":"MLO", "title":`<#WiFi_mlo_title#>`, "desc":`<#WiFi_mlo_Desc#>`}
		];
		if(isSwMode("ap")){
			wizard_type_list = wizard_type_list.filter(function(item, index, array){
				return (item.type == "Employee" || item.type == "Guest" || item.type == "Kids" || item.type == "MLO");
			});
		}
		if(isSupport("BUSINESS")){
			var specific_wizard_type = wizard_type_list.filter(function(item, index, array){
				return (item.type == "Kids");
			})[0];
			if(specific_wizard_type != undefined){
				specific_wizard_type.type = "Sched";
				specific_wizard_type.title = "<#GuestNetwork_Sche_network#>";
			}
		}
		else{
			wizard_type_list = wizard_type_list.filter(function(item, index, array){
				return (item.type != "Employee");
			});
		}
		if(!isSupport("mlo")){
			wizard_type_list = wizard_type_list.filter(function(item, index, array){
				return (item.type != "MLO");
			});
		}
		if(isSupport("SMART_HOME_MASTER_UI") || !(isSupport("captivePortal") && isSupport("cp_freewifi"))){
			wizard_type_list = wizard_type_list.filter(function(item, index, array){
				return (item.type != "Portal");
			});
		}
		var $container = $("<div>");

		var $popup_title_container = $("<div>").addClass("popup_title_container");
		$popup_title_container.appendTo($container);
		$("<div>").addClass("title").html("<#NewFeatureAbout#>").appendTo($popup_title_container);
		var $close_btn = $("<div>").addClass("vpn_icon_all_collect close_btn");
		$close_btn.appendTo($popup_title_container);
		$close_btn.unbind("click").click(function(e){
			e = e || event;
			e.stopPropagation();
			close_popup_container("all");
		});

		var $popup_content_container = $("<div>").addClass("popup_content_container");
		$popup_content_container.appendTo($container);

		var $feature_desc_cntr = $("<div>").addClass("feature_desc_container").appendTo($popup_content_container);
		$("<div>").addClass("title").html("<#NewFeatureDesc#>").appendTo($feature_desc_cntr);
		$("<div>").addClass("desc").html(
			stringSafeGet("<#SDN_feature_desc#>")+" "+stringSafeGet("<#SDN_feature_desc2#>")+" "+stringSafeGet("<#SDN_feature_desc3#>")
		).appendTo($feature_desc_cntr);

		var $wizard_list_bg = $("<div>").addClass("wizard_list_bg").appendTo($feature_desc_cntr);
		$.each(wizard_type_list, function(index, item){
			var $profile_item_container = $("<div>").addClass("profile_item_container wizard feature_desc " + item.type + "").appendTo($wizard_list_bg);
			var $item_text_container = $("<div>").addClass("item_text_container").appendTo($profile_item_container);
			$("<div>").addClass("main_info").append(item.title).appendTo($item_text_container);
			if(item.desc != "")
				$("<div>").addClass("sub_info").append(item.desc).appendTo($item_text_container);
		});

		return $container;
	}
}
function show_Get_Start(view_mode){
	var $container = $("<div>").addClass("setting_content_container");

	if(view_mode == "popup")
		$container.append(Get_Component_Popup_Profile_Title("<#QKSet_all_title#>"));

	var $content_container = $("<div>").addClass("profile_setting");
	if(view_mode == "popup")
		$content_container.addClass("popup_content_container");
	else
		$content_container.addClass("no_popup_content_container");

	$content_container.addClass("Get_Start").appendTo($container);

	let specific_wizard_type = "";
	let $employee_comp = "", $portal_comp = "", $guest_comp = "", $kids_sched_comp = "", $iot_comp = "", $vpn_comp = "",
		$scenario_comp = "", $customized_comp = "", $mlo_comp = "";
	if(isSupport("BUSINESS")){
		let employee_parm = {"type":"Employee", "title":"", "desc":""};
		specific_wizard_type = wizard_type_text.filter(function(item, index, array){
			return (item.type == "Employee");
		})[0];
		if(specific_wizard_type != undefined){
			employee_parm.title = specific_wizard_type.text
			employee_parm.desc = specific_wizard_type.desc;
		}
		$employee_comp = Get_Component_Wizard_Item(employee_parm);
	}
	if(isSwMode("rt")){
		if(cp_type_rl_json.length > 0){
			let portal_parm = {"type":"Portal", "title":"", "desc":""};
			specific_wizard_type = wizard_type_text.filter(function(item, index, array){
				return (item.type == "Portal");
			})[0];
			if(specific_wizard_type != undefined){
				portal_parm.title = specific_wizard_type.text
				portal_parm.desc = specific_wizard_type.desc;
			}
			$portal_comp = Get_Component_Wizard_Item(portal_parm);
		}
	}

	let guest_parm = {"type":"Guest", "title":"", "desc":""};
	specific_wizard_type = wizard_type_text.filter(function(item, index, array){
		return (item.type == "Guest");
	})[0];
	if(specific_wizard_type != undefined){
		guest_parm.title = specific_wizard_type.text
		guest_parm.desc = specific_wizard_type.desc;
	}
	$guest_comp = Get_Component_Wizard_Item(guest_parm);

	let kids_sched_parm = {"type":"Kids", "title":"", "desc":""};
	if(isSupport("BUSINESS")){
		kids_sched_parm = {"type":"Sched", "title":"", "desc":""};
	}
	specific_wizard_type = wizard_type_text.filter(function(item, index, array){
		return (item.type == "Kids" || item.type == "Sched");
	})[0];
	if(specific_wizard_type != undefined){
		kids_sched_parm.title = specific_wizard_type.text
		kids_sched_parm.desc = specific_wizard_type.desc;
	}
	$kids_sched_comp = Get_Component_Wizard_Item(kids_sched_parm);

	if(isSwMode("rt")){
		let iot_parm = {"type":"IoT", "title":"", "desc":""};
		specific_wizard_type = wizard_type_text.filter(function(item, index, array){
			return (item.type == "IoT");
		})[0];
		if(specific_wizard_type != undefined){
			iot_parm.title = specific_wizard_type.text
			iot_parm.desc = specific_wizard_type.desc;
		}
		$iot_comp = Get_Component_Wizard_Item(iot_parm);

		let vpn_parm = {"type":"VPN", "title":"", "desc":""};
		specific_wizard_type = wizard_type_text.filter(function(item, index, array){
			return (item.type == "VPN");
		})[0];
		if(specific_wizard_type != undefined){
			vpn_parm.title = specific_wizard_type.text
			vpn_parm.desc = specific_wizard_type.desc;
		}
		$vpn_comp = Get_Component_Wizard_Item(vpn_parm);

		if(isSupport("BUSINESS")){
			var $scenario_list_bg = $("<div>").addClass("wizard_list_bg").appendTo($content_container);
			$scenario_comp = Get_Component_Wizard_Item_Scenario()
			$scenario_comp.unbind("click").click(function(e){
				e = e || event;
				e.stopPropagation();
				var $parent_cntr = $container.parent();
				var $popup_cntr = $container.closest(".popup_container");
				var hide_title_flag = $popup_cntr.hasClass("hide_title_cntr");
				$parent_cntr.find(".setting_content_container").replaceWith(Get_Component_Scenarios_Explorer(view_mode));
				if(hide_title_flag){
					$parent_cntr.find(".setting_content_container .popup_title_container").hide();
				}
				if(isMobile()){
					$popup_cntr.addClass("full_width");
				}
				resize_iframe_height();
				resize_iframe_height();
				window.scrollTo({top: 0});
			});
		}
	}

	$customized_comp = Get_Component_Wizard_Item_Customized();

	if(isSupport("mlo")){
		let mlo_parm = {"type":"MLO", "title":"", "desc":""};
		specific_wizard_type = wizard_type_text.filter(function(item, index, array){
			return (item.type == "MLO");
		})[0];
		if(specific_wizard_type != undefined){
			mlo_parm.title = specific_wizard_type.text
			mlo_parm.desc = specific_wizard_type.desc;
		}
		$mlo_comp = Get_Component_Wizard_Item(mlo_parm)
	}

	if(isSupport("SMART_HOME_MASTER_UI")){
		$("<div>").addClass("title").html(htmlEnDeCode.htmlEncode(`<#Guest_Network#>`)).appendTo($content_container);
		let $gn_list_bg = $("<div>").addClass("wizard_list_bg").appendTo($content_container);
		if($guest_comp != "")
			$guest_comp.appendTo($gn_list_bg);

		$("<div>").addClass("title").html(htmlEnDeCode.htmlEncode(`SMART HOME MASTER`)).appendTo($content_container);/* untranslated */
		let $shm_list_bg = $("<div>").addClass("wizard_list_bg").appendTo($content_container);
		if($kids_sched_comp != "")
			$kids_sched_comp.appendTo($shm_list_bg);
		if($iot_comp != "")
			$iot_comp.appendTo($shm_list_bg);
		if($vpn_comp != "")
			$vpn_comp.appendTo($shm_list_bg);
		if($mlo_comp != "")
			$mlo_comp.appendTo($content_container);
	}
	else{//SDN, Guest Network PRO
		$("<div>").addClass("title").html(htmlEnDeCode.htmlEncode(str_SDN_choose)).appendTo($content_container);
		if($employee_comp != "")
			$employee_comp.appendTo($content_container);
		if($portal_comp != "")
			$portal_comp.appendTo($content_container);
		if($guest_comp != "")
			$guest_comp.appendTo($content_container);
		if($kids_sched_comp != "")
			$kids_sched_comp.appendTo($content_container);
		if($iot_comp != "")
			$iot_comp.appendTo($content_container);
		if($vpn_comp != "")
			$vpn_comp.appendTo($content_container);
		if($scenario_comp != "")
			$scenario_comp.appendTo($content_container);
		if($customized_comp != "")
			$customized_comp.appendTo($content_container);
		if($mlo_comp != "")
			$mlo_comp.appendTo($content_container);
	}

	return $container;
}
function show_popup_Wizard_Setting(_type){
	if(_type == undefined || _type == "")
		return;

	if(sdn_all_rl_json.length >= sdn_maximum){
		show_customize_alert("<#AiMesh_Binding_Rule_Maxi#>");
		return;
	}

	$(".container").addClass("blur_effect");
	$(".qis_container").addClass("blur_effect");
	if($(".popup_container.popup_element").css("display") == "flex"){
		$(".popup_container.popup_element").addClass("blur_effect");
	}
	$(".popup_element_second").css("display", "flex");
	$(".popup_container.popup_element_second").empty();

	switch(_type){
		case "Employee":
			$(".popup_container.popup_element_second").append(Get_Wizard_Employee("popup"));
			break;
		case "Guest":
			$(".popup_container.popup_element_second").append(Get_Wizard_Guest("popup"));
			break;
		case "Kids":
		case "Sched":
			$(".popup_container.popup_element_second").append(Get_Wizard_Kids("popup"));
			break;
		case "IoT":
			$(".popup_container.popup_element_second").append(Get_Wizard_IoT("popup"));
			break;
		case "VPN":
			$(".popup_container.popup_element_second").append(Get_Wizard_VPN("popup"));
			break;
		case "Customized":
			$(".popup_container.popup_element_second").append(Get_Wizard_Customized("popup"));
			break;
		case "Portal":
			$(".popup_container.popup_element_second").append(Get_Wizard_Portal("popup"));
			break;
		case "MLO":
			$(".popup_container.popup_element_second").append(Get_Wizard_MLO("popup"));
			break;
	}

	var $setting_content_cntr = $(".popup_container.popup_element_second .setting_content_container").attr({"data-wizard-type": _type});
	if(is_cfg_ready){
		update_aimesh_wifi_band_info();
		update_aimesh_wifi_band_full();
		if(_type != "MLO"){
			if(_type == "IoT"){
				set_value_Custom_Select($setting_content_cntr, "wifi_band", "1");
			}
			else{
				const support_6g = wifi_band_options.some((el) => {
					return (el.value == "4");
				});
				if(!(aimesh_wifi_band_full.all.band_2G) && !(aimesh_wifi_band_full.all.band_5G)){
					set_value_Custom_Select($setting_content_cntr, "wifi_band", "3");
				}
				else if(aimesh_wifi_band_full.all.band_2G && !(aimesh_wifi_band_full.all.band_5G)){
					set_value_Custom_Select($setting_content_cntr, "wifi_band", "2");
				}
				else if(!(aimesh_wifi_band_full.all.band_2G) && aimesh_wifi_band_full.all.band_5G){
					set_value_Custom_Select($setting_content_cntr, "wifi_band", "1");
				}
				else if(support_6g && !(aimesh_wifi_band_full.all.band_6G)){
					set_value_Custom_Select($setting_content_cntr, "wifi_band", "4");
				}
				else{
					set_value_Custom_Select($setting_content_cntr, "wifi_band", "3");
				}
			}
		}
		var $sel_wifi_band = $setting_content_cntr.find("#select_wifi_band");
		let $AiMesh_List = $setting_content_cntr.find("[data-container=AiMesh_List]");
		if(_type == "MLO"){
			Set_AiMesh_List_CB_MLO($AiMesh_List);
		}
		else{
			Set_AiMesh_List_CB($AiMesh_List, $sel_wifi_band.children(".selected").attr("value"));
		}
		if(!($AiMesh_List.find(".icon_checkbox").hasClass("clicked"))){
			$setting_content_cntr.find("#more_config").click();
		}
	}
	else{
		$setting_content_cntr.find("#wifi_band").closest(".profile_setting_item").remove();
		$setting_content_cntr.find("[data-container=wizard_aimesh_cntr]").remove();
	}

	var portal_status = {"support":false, "type":"0"};
	if(_type == "Portal"){
		portal_status.support = true;
		portal_status.type = "2";
	}
	else if(_type == "Kids"){
		if(get_cp_type_support("4")){
			portal_status.support = true;
			portal_status.type = "4";
		}
	}
	if(portal_status.support){
		$setting_content_cntr.find("[data-portal-type]").hide(0, function(){
			$(this).find(".profile_setting_item").hide();
		}).filter("[data-portal-type=0]").show(0, function(){
			$(this).find(".profile_setting_item").show();
		});
		var $select_portal_type = $setting_content_cntr.find("#select_portal_type");
		$.each(cp_type_rl_json, function(index, item){
			if(item.sdn_idx != ""){
				$select_portal_type.children("[value='"+item.cp_idx+"']")
					.addClass("disabled")
					.attr({"data-disabled":"true"})
					.html(htmlEnDeCode.htmlEncode(item.cp_text + " (<#Status_Used#>)"))
			}
		});
		if($select_portal_type.children("[value="+portal_status.type+"]").length > 0){
			var default_select_portal = {"type":"0", "show_hint":false}
			if(!$select_portal_type.children("[value="+portal_status.type+"]").hasClass("disabled")){//check same type
				default_select_portal.type = portal_status.type;
			}
			if(default_select_portal.type != "0"){
				$.each(sdn_all_rl_json, function(index, item) {
					if(item.sdn_rl.cp_idx != "0" && item.sdn_rl.cp_idx != default_select_portal.type){//check other type
						if(item.sdn_rl.sdn_enable == "1"){
							default_select_portal.type = "0";
							default_select_portal.show_hint = true;
							return false;
						}
					}
				});
			}
			if(default_select_portal.show_hint){
				$select_portal_type
					.closest(".custom_select_container")
					.attr("temp_disable", "disabled").addClass("temp_disable")
					.closest(".profile_setting_item")
					.after($("<div>").html("<#vpnc_conn_maxi_general#>".replace(/VPN/g, "<#GuestNetwork_Portal_type#>").replace(/2/g, "1")).addClass("item_hint"));
			}
			set_value_Custom_Select($setting_content_cntr, "portal_type", default_select_portal.type);
			$setting_content_cntr.find("[data-portal-type]").hide(0, function(){
				$(this).find(".profile_setting_item").hide();
			}).filter("[data-portal-type="+default_select_portal.type+"]").show(0, function(){
				$(this).find(".profile_setting_item").show();
			});
		}
	}

	adjust_popup_container_top($(".popup_container.popup_element_second"), 100);
	resize_iframe_height();

	function Get_Wizard_Employee(view_mode){
		var _set_apply_btn_status = function(_obj){
			var $btn_container_apply = $(_obj).find(".action_container .btn_container.apply");
			var isBlank = validate_isBlank($(_obj));
			if(isBlank){
				$btn_container_apply.removeClass("valid_fail").addClass("valid_fail").unbind("click");
			}
			else{
				$btn_container_apply.removeClass("valid_fail").unbind("click").click(function(e){
					e = e || event;
					e.stopPropagation();
					var wizard_type = $(this).closest(".setting_content_container").attr("data-wizard-type");
					if(validate_format_Wizard_Item($(_obj), wizard_type)){
						var sdn_obj = get_new_sdn_profile();
						var sdn_idx = Object.keys(sdn_obj);
						var sdn_profile = sdn_obj[sdn_idx];
						selected_sdn_idx = sdn_profile.sdn_rl.idx;
						var nvramSet_obj = {"action_mode": "apply", "rc_service": "restart_wireless;restart_sdn " + selected_sdn_idx + ";"};
						var rc_append = "";
						var wifi_band = (!is_cfg_ready) ? 3 : (parseInt($(_obj).find("#select_wifi_band").children(".selected").attr("value")));
						sdn_profile.sdn_rl.sdn_name = wizard_type;
						sdn_profile.apg_rl.ssid = $(_obj).find("#sdn_name").val();
						if(wifi_band > 0){
							var sec_option_id = $(_obj).find("#security_employee .switch_text_container").children(".selected").attr("data-option-id");
							var wifi_pwd = "";
							var wifi_auth = "psk2";
							let wifi_auth_6G = "sae";
							var wifi_crypto = "aes";
							if(sec_option_id == "radius"){
								sdn_profile.radius_rl = get_new_radius_rl();
								sdn_profile.radius_rl.radius_idx = sdn_profile.apg_rl.apg_idx;
								sdn_profile.radius_rl.auth_server_1 = $(_obj).find("#radius_ipaddr").val();
								sdn_profile.radius_rl.auth_port_1 = $(_obj).find("#radius_port").val();
								sdn_profile.radius_rl.auth_key_1 = $(_obj).find("#radius_key").val();
								wifi_auth = $(_obj).find("#select_wifi_auth_radius").children(".selected").attr("value");
								wifi_auth_6G = $(_obj).find("#select_wifi_auth_radius_6G").children(".selected").attr("value");
								if(wifi_auth == "suite-b")
									wifi_crypto = "suite-b";
							}
							else if(sec_option_id == "pwd"){
								wifi_auth = $(_obj).find("#select_wifi_auth").children(".selected").attr("value");
								wifi_auth_6G = $(_obj).find("#select_wifi_auth_6G").children(".selected").attr("value");
								wifi_pwd = $(_obj).find("#sdn_pwd").val();
							}
							if(wifi_band == 4 || wifi_band == 5 || wifi_band == 6){
								wifi_auth = wifi_auth_6G;
							}
							var radius_idx = sdn_profile.apg_rl.apg_idx;
							sdn_profile.apg_rl.security = "<3>" + wifi_auth + ">" + wifi_crypto + ">" + wifi_pwd + ">" + radius_idx + "";
							sdn_profile.apg_rl.security += "<13>" + wifi_auth + ">" + wifi_crypto + ">" + wifi_pwd + ">" + radius_idx + "";
							sdn_profile.apg_rl.security += "<16>" + wifi_auth_6G + ">" + wifi_crypto + ">" + wifi_pwd + ">" + radius_idx + "";
							sdn_profile.apg_rl.security += "<96>" + wifi_auth_6G + ">" + wifi_crypto + ">" + wifi_pwd + ">" + radius_idx + "";
							sdn_profile.apg_rl.sched = schedule_handle_data.json_array_to_string(wizard_schedule.Get_Value());
							if($(_obj).find("#wizard_schedule").hasClass("on")){
								sdn_profile.apg_rl.timesched = ((sdn_profile.apg_rl.sched == "") ? "0" : "1");
							}
							else{
								sdn_profile.apg_rl.timesched = "0";
							}
							if($(_obj).find("#bw_enabled").hasClass("on")){
								sdn_profile.apg_rl.bw_limit = "<1>" + ($(_obj).find("#bw_ul").val())*1024 + ">" + ($(_obj).find("#bw_dl").val())*1024;
								nvramSet_obj.qos_enable = "1";
								nvramSet_obj.qos_type = "2";
								rc_append += "restart_qos;restart_firewall;";
							}
							else
								sdn_profile.apg_rl.bw_limit = "<0>>";
						}
						var dut_list = "";
						if(is_cfg_ready){
							if(wifi_band > 0){
								$(_obj).find("[data-container=AiMesh_List]").find(".node_container").each(function(){
									if($(this).find(".icon_checkbox").hasClass("clicked")){
										var node_mac = $(this).attr("data-node-mac");
										var specific_node = cfg_clientlist.filter(function(item, index, array){
											return (item.mac == node_mac);
										})[0];
										if(specific_node != undefined){
											dut_list += get_specific_dut_list(wifi_band, specific_node);
										}
									}
								});
								dut_list += get_unChecked_dut_list(_obj);
							}
							else{
								dut_list = get_dut_list(wifi_band);
							}
						}
						else{
							dut_list = get_dut_list_by_mac("3", cap_mac);
						}
						sdn_profile.apg_rl.dut_list = dut_list;

						sdn_profile.sdn_access_rl = [];
						if($(_obj).find("#access_intranet").hasClass("on")){
							sdn_profile.sdn_access_rl.push({"access_sdn_idx": "0", "sdn_idx": sdn_profile.sdn_rl.idx});
						}
						sdn_profile.apg_rl.ap_isolate = "0";

						var sdn_all_rl_tmp = JSON.parse(JSON.stringify(sdn_all_rl_json));
						sdn_all_rl_tmp.push(JSON.parse(JSON.stringify(sdn_profile)));
						var vlan_rl_tmp = JSON.parse(JSON.stringify(vlan_rl_json));
						vlan_rl_tmp.push(JSON.parse(JSON.stringify(sdn_profile.vlan_rl)));
						var sdn_all_list = parse_JSONToStr_sdn_all_list({"sdn_all_rl":sdn_all_rl_tmp, "vlan_rl":vlan_rl_tmp});
						if(rc_append != "")
							nvramSet_obj.rc_service = nvramSet_obj.rc_service + rc_append;
						$.extend(nvramSet_obj, sdn_all_list);
						var apgX_rl = parse_apg_rl_to_apgX_rl(sdn_profile.apg_rl);
						$.extend(nvramSet_obj, apgX_rl);
						if(is_QIS_flow){
							nvramSet_obj.sdn_rc_service = nvramSet_obj.rc_service;
							delete nvramSet_obj["rc_service"];
							delete nvramSet_obj["action_mode"];
							$(this).replaceWith($("<img>").attr({"width": "30px", "src": "/images/InternetScan.gif"}));
							apply.SDN_Profile(nvramSet_obj);
							return;
						}
						var showLoading_status = get_showLoading_status(nvramSet_obj.rc_service);
						if(!httpApi.app_dataHandler){
							showLoading();
							close_popup_container("all");
							if(isWLclient()){
								showLoading(showLoading_status.time);
								setTimeout(function(){
									showWlHintContainer();
								}, showLoading_status.time*1000);
								check_isAlive_and_redirect({"page": "SDN.asp", "time": showLoading_status.time});
							}
						}
						httpApi.nvramSet(nvramSet_obj, function(){
							if(isWLclient()) return;
							showLoading(showLoading_status.time);
							setTimeout(function(){
								init_sdn_all_list();
								show_sdn_profilelist();
								if(!window.matchMedia('(max-width: 575px)').matches)
									$("#profile_list_content").find("[sdn_idx=" + selected_sdn_idx + "] .item_text_container").click();
							}, showLoading_status.time*1000);
							if(!isMobile()){
								if(showLoading_status.disconnect){
									check_isAlive_and_redirect({"page": "SDN.asp", "time": showLoading_status.time});
								}
							}
						});
					}
				});
			}
		};

		var $container = $("<div>").addClass("setting_content_container");

		if(view_mode == "popup"){
			Get_Component_Popup_Profile_Title("<#GuestNetwork_Employee#>").appendTo($container)
				.find("#title_close_btn").unbind("click").click(function(e){
					e = e || event;
					e.stopPropagation();
					close_popup_container($container);
				});
		}
		else
			Get_Component_Profile_Title("<#GuestNetwork_Employee#>").appendTo($container);

		var $content_container = $("<div>").addClass("popup_content_container profile_setting").appendTo($container);

		var sdn_name_parm = {"title":"<#QIS_finish_wireless_item1#>", "type":"text", "id":"sdn_name", "need_check":true, "maxlength":32, "openHint":"0_1"};
		Get_Component_Input(sdn_name_parm).appendTo($content_container)
			.find("#" + sdn_name_parm.id + "")
			.unbind("keypress").keypress(function(){
				return validator.isString(this, event);
			});

		var security_options = [{"text":"<#HSDPAConfig_Password_itemname#>","option_id":"pwd"}, {"text":"<#menu5_1_5#>","option_id":"radius"}];
		var security_parm = {"title":"<#Security#>", "options": security_options, "container_id":"security_employee"};
		Get_Component_Switch_Text(security_parm).attr({"data-group":"wifi_settings"}).appendTo($content_container)
			.find(".switch_text_container > .switch_text_item")
			.click(function(e){
				e = e || event;
				e.stopPropagation();
				var option_id = $(this).attr("data-option-id");
				$(this).closest(".profile_setting").find("[data-sec-option-id]").hide().next(".validate_hint").remove();
				$(this).closest(".profile_setting").find("[data-sec-option-id=" + option_id + "]").show();
				if(option_id == "pwd"){
					$(this).closest(".profile_setting").find("#sdn_pwd_strength").hide();
				}

				let $profile_setting = $(this).closest(".profile_setting");
				const wifi_band = $profile_setting.find("#select_wifi_band").children(".selected").attr("value");
				$profile_setting.find("[data-sec-option-id=" + option_id + "][data-wifi-auth]").hide()
					.filter((()=>{
						const sdn_type = $(this).closest(".setting_content_container").attr("sdn_type");
						return get_elem_attr_wifi_auth({"wifi_band_option_value":wifi_band, "sdn_type":sdn_type});
					})()).show();

				resize_iframe_height();
				_set_apply_btn_status($(this).closest(".profile_setting"));
				if(is_cfg_ready && isSupport("wpa3-e")){
					Set_AiMesh_List_CB(
						$(this).closest(".profile_setting").find("[data-container=AiMesh_List]"),
						$(this).closest(".profile_setting").find("#select_wifi_band").children(".selected").attr("value")
					);
				}
			});

		let wifi_auth_options = [
			{"text":"WPA2-Personal","value":"psk2"},
			{"text":"WPA/WPA2-Personal","value":"pskpsk2"},
			{"text":"WPA2/WPA3-Personal","value":"psk2sae"},
			{"text":`WPA3-Personal`,"value":"sae"}
		];
		var wifi_auth_options_parm = {"title": "<#WLANConfig11b_AuthenticationMethod_itemname#>", "id": "wifi_auth", "options": wifi_auth_options, "set_value": "psk2"};
		Get_Component_Custom_Select(wifi_auth_options_parm).attr({"data-sec-option-id":"pwd", "data-group":"wifi_settings","data-wifi-auth":"default"}).appendTo($content_container);

		let wifi_auth_options_6G = [{"text":`WPA3-Personal`,"value":"sae"}];
		let wifi_auth_options_6G_parm = {"title": `<#WLANConfig11b_AuthenticationMethod_itemname#>`, "id": "wifi_auth_6G", "options": wifi_auth_options_6G, "set_value": "sae"};
		Get_Component_Custom_Select(wifi_auth_options_6G_parm).attr({"data-sec-option-id":"pwd", "data-group":"wifi_settings","data-wifi-auth":"6G"}).appendTo($content_container);

		var wifi_auth_radius_options = [{"text":"WPA2-Enterprise","value":"wpa2"},{"text":"WPA/WPA2-Enterprise","value":"wpawpa2"}];
		if(isSupport("wpa3-e")){
			wifi_auth_radius_options.push({"text":"WPA3-Enterprise","value":"wpa3"});
			wifi_auth_radius_options.push({"text":"WPA2/WPA3-Enterprise","value":"wpa2wpa3"});
			wifi_auth_radius_options.push({"text":"WPA3-Enterprise 192-bit","value":"suite-b"});
		}
		var wifi_auth_radius_options_parm = {"title": "<#WLANConfig11b_AuthenticationMethod_itemname#>", "id": "wifi_auth_radius", "options": wifi_auth_radius_options, "set_value": "wpa2"};
		var $sel_wifi_auth_radius = Get_Component_Custom_Select(wifi_auth_radius_options_parm).attr({"data-sec-option-id":"radius","data-group":"wifi_settings","data-wifi-auth":"default"})
			.appendTo($content_container).find("#select_" + wifi_auth_radius_options_parm.id + "");
		if(is_cfg_ready && isSupport("wpa3-e")){
			$sel_wifi_auth_radius.children("div").click(function(e){
				Set_AiMesh_List_CB(
					$(this).closest(".profile_setting").find("[data-container=AiMesh_List]"),
					$(this).closest(".profile_setting").find("#select_wifi_band").children(".selected").attr("value")
				);
			});
		}

		let wifi_auth_radius_options_6G = [];
		if(isSupport("wpa3-e")){
			wifi_auth_radius_options_6G.push({"text":"WPA3-Enterprise","value":"wpa3"});
			wifi_auth_radius_options_6G.push({"text":"WPA3-Enterprise 192-bit","value":"suite-b"});
		}
		let wifi_auth_radius_options_6G_parm = {"title": "<#WLANConfig11b_AuthenticationMethod_itemname#>", "id": "wifi_auth_radius_6G", "options": wifi_auth_radius_options_6G, "set_value": "wpa3"};
		let $sel_wifi_auth_radius_6G = Get_Component_Custom_Select(wifi_auth_radius_options_6G_parm).attr({"data-sec-option-id":"radius","data-group":"wifi_settings","data-wifi-auth":"6G"})
			.appendTo($content_container).find("#select_" + wifi_auth_radius_options_6G_parm.id + "");
		if(is_cfg_ready && isSupport("wpa3-e")){
			$sel_wifi_auth_radius_6G.children("div").click(function(e){
				Set_AiMesh_List_CB(
					$(this).closest(".profile_setting").find("[data-container=AiMesh_List]"),
					$(this).closest(".profile_setting").find("#select_wifi_band").children(".selected").attr("value")
				);
			});
		}

		let sdn_pwd_parm = {"title":`<#QIS_finish_wireless_item2#>`, "type":"password", "id":"sdn_pwd", "need_check":true, "maxlength":64, "openHint":"0_7"};
		Get_Component_Input(sdn_pwd_parm).attr({"data-sec-option-id":"pwd", "data-group":"wifi_settings"}).appendTo($content_container)
			.find("#" + sdn_pwd_parm.id + "")
			.unbind("keyup").keyup(function(){
				chkPass($(this).val(), "rwd_vpn_pwd", $("#sdn_pwd_strength"));
				resize_iframe_height();
			})
			.unbind("blur").blur(function(){
				if($(this).val() == "")
					$("#" + sdn_pwd_parm.id + "_strength").hide();
			});
		$("<div>").attr({"id":"sdn_pwd_strength", "data-sec-option-id":"pwd", "data-group":"wifi_settings"}).append(Get_Component_PWD_Strength_Meter()).appendTo($content_container).hide();

		var radius_ipaddr_parm = {"title":"<#WLANAuthentication11a_ExAuthDBIPAddr_itemname#>", "type":"text", "id":"radius_ipaddr", "need_check":true, "maxlength":15, "openHint":"2_1"};
		Get_Component_Input(radius_ipaddr_parm).attr({"data-sec-option-id":"radius", "data-group":"wifi_settings"}).appendTo($content_container)
			.find("#" + radius_ipaddr_parm.id + "")
			.unbind("keypress").keypress(function(){
				return validator.isIPAddr(this, event);
			});

		var radius_port_parm = {"title":"<#WLANAuthentication11a_ExAuthDBPortNumber_itemname#>", "type":"text", "id":"radius_port", "need_check":true, "maxlength":5, "openHint":"2_2"};
		Get_Component_Input(radius_port_parm).attr({"data-sec-option-id":"radius", "data-group":"wifi_settings"}).appendTo($content_container)
			.find("#" + radius_port_parm.id + "")
			.unbind("keypress").keypress(function(){
				return validator.isNumber(this,event);
			});

		var radius_key_parm = {"title":"<#WLANAuthentication11a_ExAuthDBPassword_itemname#>", "type":"password", "id":"radius_key", "need_check":true, "maxlength":64, "openHint":"2_3"};
		Get_Component_Input(radius_key_parm).attr({"data-sec-option-id":"radius", "data-group":"wifi_settings"}).appendTo($content_container);

		$content_container.find("[data-sec-option-id=radius]").hide();
		$content_container.find("[data-wifi-auth=6G]").hide();

		var more_config_parm = {"title":"<#MoreConfig#>", "id":"more_config", "slide_target":"more_config_cntr"};
		Get_Component_Slide_Title(more_config_parm).appendTo($content_container);
		var $more_config_cntr = $("<div>").attr({"data-slide_target": "more_config_cntr"}).hide().appendTo($content_container);

		const band_options = wifi_band_options.filter((el) => {
			return (el.value != "5" && el.value != "6");
		});
		let wifi_band_options_parm = {"title": "WiFi <#Interface#>", "id": "wifi_band", "options": band_options, "set_value": "3"};
		var $sel_wifi_band = Get_Component_Custom_Select(wifi_band_options_parm).appendTo($more_config_cntr).find("#select_" + wifi_band_options_parm.id + "");
		$sel_wifi_band.children("div").click(function(e){
			var options = $(this).attr("value");
			var $profile_setting = $(this).closest(".profile_setting");
			$profile_setting.find("[data-container=AiMesh_List]").next(".validate_hint").remove();
			var $wifi_settings_objs = $profile_setting.find("[data-group=wifi_settings]");
			if(options == "0"){
				$wifi_settings_objs.hide().next(".validate_hint").remove();
				_set_apply_btn_status($profile_setting);
			}
			else{
				$wifi_settings_objs.show().filter("#wizard_aimesh").addClass("arrow_up");
				Set_AiMesh_List_CB($profile_setting.find("[data-container=AiMesh_List]"), options);
				if($wifi_settings_objs.find(".switch_text_container").length > 0)
					$wifi_settings_objs.find(".switch_text_container .switch_text_item.selected").click();
				if($wifi_settings_objs.find("#bw_enabled").length > 0){
					if($wifi_settings_objs.find("#bw_enabled").hasClass("on")){
						$wifi_settings_objs.filter("#container_bw").show();
					}
					else{
						$wifi_settings_objs.filter("#container_bw").hide();
					}
				}
				if($wifi_settings_objs.find("#wizard_schedule").length > 0){
					if($wifi_settings_objs.find("#wizard_schedule").hasClass("on"))
						$wifi_settings_objs.filter("#container_wizard_schedule").show();
					else
						$wifi_settings_objs.filter("#container_wizard_schedule").hide();
				}
				_set_apply_btn_status($profile_setting);
			}
			resize_iframe_height();
		});

		var aimesh_parm = {"title":"AiMesh", "id":"wizard_aimesh", "slide_target":"wizard_aimesh_cntr"};
		var $wizard_slide_aimesh = Get_Component_Slide_Title(aimesh_parm).attr({"data-container":"wizard_aimesh_cntr", "data-group":"wifi_settings"}).addClass("arrow_up").appendTo($more_config_cntr);
		var $wizard_aimesh_cntr = $("<div>").attr({"data-slide_target":"wizard_aimesh_cntr", "data-container":"wizard_aimesh_cntr", "data-group":"wifi_settings"}).show().appendTo($more_config_cntr);
		var $AiMesh_List = $("<div>").attr({"data-container":"AiMesh_List"}).append(Get_AiMesh_List_Container()).appendTo($wizard_aimesh_cntr);
		$AiMesh_List.find(".node_content_container").click(function(e){
			e = e || event;
			e.stopPropagation();
			_set_apply_btn_status($(this).closest(".profile_setting"));
		});

		var wizard_schedule = new schedule({
			show_timeset_viewport_callback: resize_iframe_height,
			icon_trash_callback: resize_iframe_height,
			btn_save_callback: resize_iframe_height,
			btn_cancel_callback: resize_iframe_height
		});
		var wizard_schedule_parm = {"title":str_WiFi_sche, "type":"switch", "id":"wizard_schedule", "set_value":"off"};
		Get_Component_Switch(wizard_schedule_parm).attr({"data-group":"wifi_settings"}).appendTo($more_config_cntr).find("#" + wizard_schedule_parm.id + "").click(function(e){
			e = e || event;
			e.stopPropagation();
			var $control_container = $(this).closest(".profile_setting").find("#container_wizard_schedule");
			if($(this).hasClass("on")){
				$control_container.empty().append(wizard_schedule.Get_UI());
				$control_container.show();
			}
			else{
				$control_container.hide();
			}
			resize_iframe_height();
		});
		$("<div>").addClass("profile_setting_item schedule_ui").attr({"id":"container_wizard_schedule", "data-group":"wifi_settings"}).append(wizard_schedule.Get_UI()).hide().appendTo($more_config_cntr);

		if(isSwMode("rt")){
			var bw_enabled_parm = {"title":"<#Bandwidth_Limiter#>", "type":"switch", "id":"bw_enabled", "set_value":"off"};
			Get_Component_Switch(bw_enabled_parm).attr({"data-group":"wifi_settings"}).appendTo($more_config_cntr)
				.find("#" + bw_enabled_parm.id + "").click(function(e){
					e = e || event;
					e.stopPropagation();
					var $control_container = $(this).closest(".profile_setting").find("#container_bw_dl, #container_bw_ul, #container_bw");
					if($(this).hasClass("on")){
						$control_container.show();
					}
					else{
						$control_container.hide().next(".validate_hint").remove();
					}
					resize_iframe_height();
					_set_apply_btn_status($(this).closest(".profile_setting"));
				});

			var bw_parm = {"title":"<#Bandwidth_Setting#> (Mb/s)", "container_id":"container_bw",
				"type_1":"text", "id_1":"bw_dl", "maxlength_1":12, "need_check_1":true,
				"type_2":"text", "id_2":"bw_ul", "maxlength_2":12, "need_check_2":true};
			Get_Component_Bandwidth_Setting(bw_parm).attr({"data-group":"wifi_settings"}).hide().appendTo($more_config_cntr);

			var access_intranet_parm = {"title":"<#Access_Intranet#>", "type":"switch", "id":"access_intranet", "openHint":"0_26"};
			Get_Component_Switch(access_intranet_parm).appendTo($more_config_cntr);
		}

		var $action_container = $("<div>").addClass("action_container").appendTo($content_container);
		$("<div>").addClass("btn_container apply").html("<#CTL_apply#>").appendTo($action_container);

		$content_container.find("[need_check=true]").keyup(function(){
			_set_apply_btn_status($content_container);
		});
		_set_apply_btn_status($content_container);

		return $container;
	}
	function Get_Wizard_Kids(view_mode){
		var _set_apply_btn_status = function(_obj){
			var $btn_container_apply = $(_obj).find(".action_container .btn_container.apply");
			var isBlank = validate_isBlank($(_obj));
			if(isBlank){
				$btn_container_apply.removeClass("valid_fail").addClass("valid_fail").unbind("click");
			}
			else{
				$btn_container_apply.removeClass("valid_fail").unbind("click").click(function(e){
					e = e || event;
					e.stopPropagation();
					var wizard_type = $(this).closest(".setting_content_container").attr("data-wizard-type");
					if(validate_format_Wizard_Item($(_obj), wizard_type)){
						var sdn_obj = get_new_sdn_profile();
						var sdn_idx = Object.keys(sdn_obj);
						var sdn_profile = sdn_obj[sdn_idx];
						selected_sdn_idx = sdn_profile.sdn_rl.idx;
						var nvramSet_obj = {"action_mode": "apply", "rc_service": "restart_wireless;restart_sdn " + selected_sdn_idx + ";"};
						var rc_append = "";
						var wifi_band = (!is_cfg_ready) ? 3 : (parseInt($(_obj).find("#select_wifi_band").children(".selected").attr("value")));
						sdn_profile.sdn_rl.sdn_name = wizard_type;
						sdn_profile.apg_rl.ssid = $(_obj).find("#sdn_name").val();
						if(wifi_band > 0){
							let wifi_auth = "psk2";
							let wifi_auth_6G = "sae";
							if(wifi_band == 4 || wifi_band == 5 || wifi_band == 6){
								wifi_auth = wifi_auth_6G;
							}
							var $sdn_pwd = $(_obj).find("#sdn_pwd");
							var radius_idx = sdn_profile.apg_rl.apg_idx;
							sdn_profile.apg_rl.security = "<3>" + wifi_auth + ">aes>" + $sdn_pwd.val() + ">" + radius_idx + "";
							sdn_profile.apg_rl.security += "<13>" + wifi_auth + ">aes>" + $sdn_pwd.val() + ">" + radius_idx + "";
							sdn_profile.apg_rl.security += "<16>" + wifi_auth_6G + ">aes>" + $sdn_pwd.val() + ">" + radius_idx + "";
							sdn_profile.apg_rl.security += "<96>" + wifi_auth_6G + ">aes>" + $sdn_pwd.val() + ">" + radius_idx + "";
							sdn_profile.apg_rl.sched = schedule_handle_data.json_array_to_string(wizard_schedule.Get_Value());
							if($(_obj).find("#wizard_schedule").hasClass("on")){
								sdn_profile.apg_rl.timesched = ((sdn_profile.apg_rl.sched == "") ? "0" : "1");
							}
							else{
								sdn_profile.apg_rl.timesched = "0";
							}
							if($(_obj).find("#bw_enabled").hasClass("on")){
								sdn_profile.apg_rl.bw_limit = "<1>" + ($(_obj).find("#bw_ul").val())*1024 + ">" + ($(_obj).find("#bw_dl").val())*1024;
								nvramSet_obj.qos_enable = "1";
								nvramSet_obj.qos_type = "2";
								rc_append += "restart_qos;restart_firewall;";
							}
							else
								sdn_profile.apg_rl.bw_limit = "<0>>";
						}
						var dut_list = "";
						if(is_cfg_ready){
							if(wifi_band > 0){
								$(_obj).find("[data-container=AiMesh_List]").find(".node_container").each(function(){
									if($(this).find(".icon_checkbox").hasClass("clicked")){
										var node_mac = $(this).attr("data-node-mac");
										var specific_node = cfg_clientlist.filter(function(item, index, array){
											return (item.mac == node_mac);
										})[0];
										if(specific_node != undefined){
											dut_list += get_specific_dut_list(wifi_band, specific_node);
										}
									}
								});
								dut_list += get_unChecked_dut_list(_obj);
							}
							else{
								dut_list = get_dut_list(wifi_band);
							}
						}
						else{
							dut_list = get_dut_list_by_mac("3", cap_mac);
						}
						sdn_profile.apg_rl.dut_list = dut_list;

						sdn_profile.sdn_access_rl = [];
						if($(_obj).find("#access_intranet").hasClass("on")){
							sdn_profile.sdn_access_rl.push({"access_sdn_idx": "0", "sdn_idx": sdn_profile.sdn_rl.idx});
						}
						sdn_profile.apg_rl.ap_isolate = "0";
						if(wizard_type == "Kids"){
							if(get_cp_type_support("4")){
								sdn_profile.sdn_rl.cp_idx = $(_obj).find("#select_portal_type").children(".selected").attr("value");
								if(sdn_profile.sdn_rl.cp_idx == "4"){
									if($.isEmptyObject(sdn_profile.cp_rl)){
										var idx_for_customized_ui = "";
										sdn_profile.cp_rl = JSON.parse(JSON.stringify(new cp_profile_attr()));
										var specific_cp_type_rl = cp_type_rl_json.filter(function(item, index, array){
											return (item.cp_idx == sdn_profile.sdn_rl.cp_idx);
										})[0];
										if(specific_cp_type_rl != undefined){
											if(specific_cp_type_rl.profile[0] != undefined){
												idx_for_customized_ui = specific_cp_type_rl.profile[0].idx_for_customized_ui;
											}
										}
										if(idx_for_customized_ui == ""){
											idx_for_customized_ui = $.now();
										}
										sdn_profile.cp_rl.idx_for_customized_ui = idx_for_customized_ui;
									}
									sdn_profile.cp_rl.cp_idx = sdn_profile.sdn_rl.cp_idx;
									sdn_profile.cp_rl.enable = "1";
									sdn_profile.cp_rl.conntimeout = 60*60;
									sdn_profile.cp_rl.redirecturl = "";
									sdn_profile.cp_rl.auth_type = "0";
									sdn_profile.cp_rl.term_of_service = "0";
									sdn_profile.cp_rl.NAS_ID = "";
									uploadFreeWiFi({
										"cp_idx": "4",
										"id": sdn_profile.cp_rl.idx_for_customized_ui,
										"MB_desc": $("#MB_desc").val(),
										"image": $(_obj).find("#MB_ui_container [data-component=MB_bg]").css('background-image').replace('url(','').replace(')','').replace(/\"/gi, ""),
										"auth_type": sdn_profile.cp_rl.auth_type
									});
									rc_append += "restart_chilli;restart_uam_srv;";
								}
							}
						}
						var sdn_all_rl_tmp = JSON.parse(JSON.stringify(sdn_all_rl_json));
						sdn_all_rl_tmp.push(JSON.parse(JSON.stringify(sdn_profile)));
						var vlan_rl_tmp = JSON.parse(JSON.stringify(vlan_rl_json));
						vlan_rl_tmp.push(JSON.parse(JSON.stringify(sdn_profile.vlan_rl)));
						var sdn_all_list = parse_JSONToStr_sdn_all_list({"sdn_all_rl":sdn_all_rl_tmp, "vlan_rl":vlan_rl_tmp});
						if(rc_append != "")
							nvramSet_obj.rc_service = nvramSet_obj.rc_service + rc_append;
						$.extend(nvramSet_obj, sdn_all_list);
						var apgX_rl = parse_apg_rl_to_apgX_rl(sdn_profile.apg_rl);
						$.extend(nvramSet_obj, apgX_rl);
						if(sdn_profile.sdn_rl.cp_idx == "4"){
							var cpX_rl = parse_cp_rl_to_cpX_rl(sdn_profile.cp_rl);
							$.extend(nvramSet_obj, cpX_rl);
						}
						if(is_QIS_flow){
							nvramSet_obj.sdn_rc_service = nvramSet_obj.rc_service;
							delete nvramSet_obj["rc_service"];
							delete nvramSet_obj["action_mode"];
							$(this).replaceWith($("<img>").attr({"width": "30px", "src": "/images/InternetScan.gif"}));
							apply.SDN_Profile(nvramSet_obj);
							return;
						}
						var showLoading_status = get_showLoading_status(nvramSet_obj.rc_service);
						if(!httpApi.app_dataHandler){
							showLoading();
							close_popup_container("all");
							if(isWLclient()){
								showLoading(showLoading_status.time);
								setTimeout(function(){
									showWlHintContainer();
								}, showLoading_status.time*1000);
								check_isAlive_and_redirect({"page": "SDN.asp", "time": showLoading_status.time});
							}
						}
						httpApi.nvramSet(nvramSet_obj, function(){
							if(isWLclient()) return;
							showLoading(showLoading_status.time);
							setTimeout(function(){
								init_sdn_all_list();
								show_sdn_profilelist();
								if(!window.matchMedia('(max-width: 575px)').matches)
									$("#profile_list_content").find("[sdn_idx=" + selected_sdn_idx + "] .item_text_container").click();
							}, showLoading_status.time*1000);
							if(!isMobile()){
								if(showLoading_status.disconnect){
									check_isAlive_and_redirect({"page": "SDN.asp", "time": showLoading_status.time});
								}
							}
						});
					}
				});
			}
		};

		var $container = $("<div>").addClass("setting_content_container");

		var profile_title = ((isSupport("BUSINESS")) ? "<#GuestNetwork_Sche_network#>" : "<#GuestNetwork_Kid#>");
		if(view_mode == "popup"){
			Get_Component_Popup_Profile_Title(profile_title).appendTo($container)
				.find("#title_close_btn").unbind("click").click(function(e){
					e = e || event;
					e.stopPropagation();
					close_popup_container($container);
				});
		}
		else
			Get_Component_Profile_Title(profile_title).appendTo($container);

		var $content_container = $("<div>").addClass("popup_content_container profile_setting").appendTo($container);

		var sdn_name_parm = {"title":"<#QIS_finish_wireless_item1#>", "type":"text", "id":"sdn_name", "need_check":true, "maxlength":32, "openHint":"0_1"};
		Get_Component_Input(sdn_name_parm).appendTo($content_container)
			.find("#" + sdn_name_parm.id + "")
			.unbind("keypress").keypress(function(){
				return validator.isString(this, event);
			});

		let sdn_pwd_parm = {"title":`<#QIS_finish_wireless_item2#>`, "type":"password", "id":"sdn_pwd", "need_check":true, "maxlength":64, "openHint":"0_7"};
		Get_Component_Input(sdn_pwd_parm).attr({"data-group":"wifi_settings"}).appendTo($content_container)
			.find("#" + sdn_pwd_parm.id + "")
			.unbind("keyup").keyup(function(){
				chkPass($(this).val(), "rwd_vpn_pwd", $("#sdn_pwd_strength"));
				resize_iframe_height();
			})
			.unbind("blur").blur(function(){
				if($(this).val() == "")
					$("#" + sdn_pwd_parm.id + "_strength").hide();
			});
		$("<div>").attr({"id":"sdn_pwd_strength"}).attr({"data-group":"wifi_settings"}).append(Get_Component_PWD_Strength_Meter()).appendTo($content_container).hide();

		var wizard_schedule = new schedule({
			data:schedule_handle_data.string_to_json_array("M13E17002100<M14116002200"),
			show_timeset_viewport_callback: resize_iframe_height,
			icon_trash_callback: resize_iframe_height,
			btn_save_callback: resize_iframe_height,
			btn_cancel_callback: resize_iframe_height
		});
		var wizard_schedule_parm = {"title":str_WiFi_sche, "type":"switch", "id":"wizard_schedule", "set_value":"on"};
		Get_Component_Switch(wizard_schedule_parm).attr({"data-group":"wifi_settings"}).appendTo($content_container).find("#" + wizard_schedule_parm.id + "").click(function(e){
			e = e || event;
			e.stopPropagation();
			var $control_container = $(this).closest(".profile_setting").find("#container_wizard_schedule");
			if($(this).hasClass("on")){
				$control_container.empty().append(wizard_schedule.Get_UI());
				$control_container.show();
			}
			else{
				$control_container.hide();
			}
			resize_iframe_height();
		});
		$("<div>").addClass("profile_setting_item schedule_ui").attr({"id":"container_wizard_schedule", "data-group":"wifi_settings"}).append(wizard_schedule.Get_UI()).appendTo($content_container);

		if(isSwMode("rt")){
			if(get_cp_type_support("4")){
				var portal_options = [{"text":"<#wl_securitylevel_0#>", "value":"0"}];
				cp_type_rl_json.forEach(function(item){
					if(item.cp_idx == "2")
						return;
					portal_options.push({"text":item.cp_text, "value":item.cp_idx});
				});
				var portal_options_parm = {"title": "<#GuestNetwork_Portal_type#>", "id": "portal_type", "options": portal_options};
				Get_Component_Custom_Select(portal_options_parm).appendTo($content_container)
					.find("#select_" + portal_options_parm.id + "").children("div").click(function(e){
						if($(this).attr("data-disabled") == "true")
							return false;
						var options = $(this).attr("value");
						$(this).closest(".profile_setting").find("[data-portal-type]").hide(0, function(){
							$(this).find(".profile_setting_item").hide();
						}).filter("[data-portal-type="+options+"]").show(0, function(){
							$(this).find(".profile_setting_item").show();
						});
						resize_iframe_height();
						_set_apply_btn_status($(this).closest(".profile_setting"));
					});

				var $message_board_cntr = $("<div>").attr({"data-portal-type":"4"}).appendTo($content_container);
				var MB_desc_parm = {"title":"<#Description#>", "type":"text", "id":"MB_desc", "need_check":true, "maxlength":80, "set_value":"<#Description#>"};
				Get_Component_Input(MB_desc_parm).appendTo($message_board_cntr)
						.find("#" + MB_desc_parm.id + "")
						.unbind("keypress").keypress(function(){
							return validator.isString(this, event);
						})
						.unbind("keyup").keyup(function(){
							$(this).closest(".profile_setting").find("#MB_Preview_Cntr [data-component=MB_desc]").html(htmlEnDeCode.htmlEncode($(this).val()));
						})
						.attr({"oninput": "trigger_keyup($(this))"});

				var $MB_ui_cntr = $("<div>").attr({"id":"MB_ui_container"}).addClass("profile_setting_item MB_ui").appendTo($message_board_cntr);
				Get_MB_Preview_Container().appendTo($MB_ui_cntr);
				Get_MB_Change_BG_Container().hide().appendTo($MB_ui_cntr);
			}
		}

		var more_config_parm = {"title":"<#MoreConfig#>", "id":"more_config", "slide_target":"more_config_cntr"};
		Get_Component_Slide_Title(more_config_parm).appendTo($content_container);
		var $more_config_cntr = $("<div>").attr({"data-slide_target": "more_config_cntr"}).hide().appendTo($content_container);

		const band_options = wifi_band_options.filter((el) => {
			return (el.value != "5");
		});
		let wifi_band_options_parm = {"title": "WiFi <#Interface#>", "id": "wifi_band", "options": band_options, "set_value": "3"};
		var $sel_wifi_band = Get_Component_Custom_Select(wifi_band_options_parm).appendTo($more_config_cntr).find("#select_" + wifi_band_options_parm.id + "");
		$sel_wifi_band.children("div").click(function(e){
			var options = $(this).attr("value");
			var $profile_setting = $(this).closest(".profile_setting");
			$profile_setting.find("[data-container=AiMesh_List]").next(".validate_hint").remove();
			var $wifi_settings_objs = $profile_setting.find("[data-group=wifi_settings]");
			if(options == "0"){
				$wifi_settings_objs.hide().next(".validate_hint").remove();
				_set_apply_btn_status($profile_setting);
			}
			else{
				$wifi_settings_objs.show().filter("#wizard_aimesh").addClass("arrow_up");
				Set_AiMesh_List_CB($profile_setting.find("[data-container=AiMesh_List]"), options);
				$wifi_settings_objs.find("#sdn_pwd").blur();
				if($wifi_settings_objs.find("#bw_enabled").length > 0){
					if($wifi_settings_objs.find("#bw_enabled").hasClass("on")){
						$wifi_settings_objs.filter("#container_bw").show();
					}
					else{
						$wifi_settings_objs.filter("#container_bw").hide();
					}
				}
				if($wifi_settings_objs.find("#wizard_schedule").length > 0){
					if($wifi_settings_objs.find("#wizard_schedule").hasClass("on"))
						$wifi_settings_objs.filter("#container_wizard_schedule").show();
					else
						$wifi_settings_objs.filter("#container_wizard_schedule").hide();
				}
				_set_apply_btn_status($profile_setting);
			}
			resize_iframe_height();
		});

		var aimesh_parm = {"title":"AiMesh", "id":"wizard_aimesh", "slide_target":"wizard_aimesh_cntr"};
		var $wizard_slide_aimesh = Get_Component_Slide_Title(aimesh_parm).attr({"data-container":"wizard_aimesh_cntr", "data-group":"wifi_settings"}).addClass("arrow_up").appendTo($more_config_cntr);
		var $wizard_aimesh_cntr = $("<div>").attr({"data-slide_target":"wizard_aimesh_cntr", "data-container":"wizard_aimesh_cntr", "data-group":"wifi_settings"}).show().appendTo($more_config_cntr);
		var $AiMesh_List = $("<div>").attr({"data-container":"AiMesh_List"}).append(Get_AiMesh_List_Container()).appendTo($wizard_aimesh_cntr);
		$AiMesh_List.find(".node_content_container").click(function(e){
			e = e || event;
			e.stopPropagation();
			_set_apply_btn_status($(this).closest(".profile_setting"));
		});

		if(isSwMode("rt")){
			if(!get_cp_type_support("4")){
				var bw_enabled_parm = {"title":"<#Bandwidth_Limiter#>", "type":"switch", "id":"bw_enabled", "set_value":"off"};
				Get_Component_Switch(bw_enabled_parm).attr({"data-group":"wifi_settings"}).appendTo($more_config_cntr)
					.find("#" + bw_enabled_parm.id + "").click(function(e){
						e = e || event;
						e.stopPropagation();
						var $control_container = $(this).closest(".profile_setting").find("#container_bw_dl, #container_bw_ul, #container_bw");
						if($(this).hasClass("on")){
							$control_container.show();
						}
						else{
							$control_container.hide().next(".validate_hint").remove();
						}
						resize_iframe_height();
						_set_apply_btn_status($(this).closest(".profile_setting"));
					});

				var bw_parm = {"title":"<#Bandwidth_Setting#> (Mb/s)", "container_id":"container_bw",
					"type_1":"text", "id_1":"bw_dl", "maxlength_1":12, "need_check_1":true,
					"type_2":"text", "id_2":"bw_ul", "maxlength_2":12, "need_check_2":true};
				Get_Component_Bandwidth_Setting(bw_parm).attr({"data-group":"wifi_settings"}).hide().appendTo($more_config_cntr);
			}

			var access_intranet_parm = {"title":"<#Access_Intranet#>", "type":"switch", "id":"access_intranet", "openHint":"0_26", "set_value":"off"};
			Get_Component_Switch(access_intranet_parm).appendTo($more_config_cntr);
		}

		var $action_container = $("<div>").addClass("action_container").appendTo($content_container);
		$("<div>").addClass("btn_container apply").html("<#CTL_apply#>").appendTo($action_container);

		$content_container.find("[need_check=true]").keyup(function(){
			_set_apply_btn_status($content_container);
		});
		_set_apply_btn_status($content_container);

		return $container;
	}
	function Get_Wizard_VPN(view_mode){
		var _set_apply_btn_status = function(_obj){
			var $btn_container_apply = $(_obj).find(".action_container .btn_container.apply");
			var isBlank = validate_isBlank($(_obj));
			if(isBlank){
				$btn_container_apply.removeClass("valid_fail").addClass("valid_fail").unbind("click");
			}
			else{
				$btn_container_apply.removeClass("valid_fail").unbind("click").click(function(e){
					e = e || event;
					e.stopPropagation();
					var wizard_type = $(this).closest(".setting_content_container").attr("data-wizard-type");
					if(validate_format_Wizard_Item($(_obj), wizard_type)){
						var sdn_obj = get_new_sdn_profile();
						var sdn_idx = Object.keys(sdn_obj);
						var sdn_profile = sdn_obj[sdn_idx];
						selected_sdn_idx = sdn_profile.sdn_rl.idx;
						var nvramSet_obj = {"action_mode": "apply", "rc_service": "restart_wireless;restart_sdn " + selected_sdn_idx + ";"};
						var rc_append = "";
						var wifi_band = parseInt($(_obj).find("#select_wifi_band").children(".selected").attr("value"));
						sdn_profile.sdn_rl.sdn_name = wizard_type;
						sdn_profile.sdn_rl.vpnc_idx = "0";
						sdn_profile.sdn_rl.vpns_idx = "0";
						if($(_obj).find("#vpn_enabled").hasClass("on")){
							var $selected_vpn = $(_obj).find("[data-container=VPN_Profiles] .rwd_icon_radio.clicked");
							if($selected_vpn.length){
								var vpn_type = $selected_vpn.attr("data-vpn-type");
								var vpn_idx = $selected_vpn.attr("data-idx");
								if(vpn_type == "vpnc"){
									sdn_profile.sdn_rl.vpnc_idx = vpn_idx;
								}
								else if(vpn_type == "vpns"){
									sdn_profile.sdn_rl.vpns_idx = vpn_idx;
								}
							}
						}
						sdn_profile.apg_rl.ssid = $(_obj).find("#sdn_name").val();
						if(wifi_band > 0){
							let wifi_auth = "psk2";
							let wifi_auth_6G = "sae";
							if(wifi_band == 4 || wifi_band == 5 || wifi_band == 6){
								wifi_auth = wifi_auth_6G;
							}
							var $sdn_pwd = $(_obj).find("#sdn_pwd");
							var radius_idx = sdn_profile.apg_rl.apg_idx;
							sdn_profile.apg_rl.security = "<3>" + wifi_auth + ">aes>" + $sdn_pwd.val() + ">" + radius_idx + "";
							sdn_profile.apg_rl.security += "<13>" + wifi_auth + ">aes>" + $sdn_pwd.val() + ">" + radius_idx + "";
							sdn_profile.apg_rl.security += "<16>" + wifi_auth_6G + ">aes>" + $sdn_pwd.val() + ">" + radius_idx + "";
							sdn_profile.apg_rl.security += "<96>" + wifi_auth_6G + ">aes>" + $sdn_pwd.val() + ">" + radius_idx + "";
							sdn_profile.apg_rl.sched = schedule_handle_data.json_array_to_string(wizard_schedule.Get_Value());
							if($(_obj).find("#wizard_schedule").hasClass("on")){
								sdn_profile.apg_rl.timesched = ((sdn_profile.apg_rl.sched == "") ? "0" : "1");
							}
							else{
								sdn_profile.apg_rl.timesched = "0";
							}
							if($(_obj).find("#bw_enabled").hasClass("on")){
								sdn_profile.apg_rl.bw_limit = "<1>" + ($(_obj).find("#bw_ul").val())*1024 + ">" + ($(_obj).find("#bw_dl").val())*1024;
								nvramSet_obj.qos_enable = "1";
								nvramSet_obj.qos_type = "2";
								rc_append += "restart_qos;restart_firewall;";
							}
							else
								sdn_profile.apg_rl.bw_limit = "<0>>";
						}
						var dut_list = "";
						if(wifi_band > 0){
							$(_obj).find("[data-container=AiMesh_List]").find(".node_container").each(function(){
								if($(this).find(".icon_checkbox").hasClass("clicked")){
									var node_mac = $(this).attr("data-node-mac");
									var specific_node = cfg_clientlist.filter(function(item, index, array){
										return (item.mac == node_mac);
									})[0];
									if(specific_node != undefined){
										dut_list += get_specific_dut_list(wifi_band, specific_node);
									}
								}
							});
							dut_list += get_unChecked_dut_list(_obj);
						}
						else{
							dut_list = get_dut_list(wifi_band);
						}
						sdn_profile.apg_rl.dut_list = dut_list;

						sdn_profile.sdn_access_rl = [];
						if($(_obj).find("#access_intranet").hasClass("on")){
							sdn_profile.sdn_access_rl.push({"access_sdn_idx": "0", "sdn_idx": sdn_profile.sdn_rl.idx});
						}
						sdn_profile.apg_rl.ap_isolate = "0";

						var sdn_all_rl_tmp = JSON.parse(JSON.stringify(sdn_all_rl_json));
						sdn_all_rl_tmp.push(JSON.parse(JSON.stringify(sdn_profile)));
						var vlan_rl_tmp = JSON.parse(JSON.stringify(vlan_rl_json));
						vlan_rl_tmp.push(JSON.parse(JSON.stringify(sdn_profile.vlan_rl)));
						var sdn_all_list = parse_JSONToStr_sdn_all_list({"sdn_all_rl":sdn_all_rl_tmp, "vlan_rl":vlan_rl_tmp});
						if(rc_append != "")
							nvramSet_obj.rc_service = nvramSet_obj.rc_service + rc_append;
						$.extend(nvramSet_obj, sdn_all_list);
						var apgX_rl = parse_apg_rl_to_apgX_rl(sdn_profile.apg_rl);
						$.extend(nvramSet_obj, apgX_rl);
						var showLoading_status = get_showLoading_status(nvramSet_obj.rc_service);
						if(!httpApi.app_dataHandler){
							showLoading();
							close_popup_container("all");
							if(isWLclient()){
								showLoading(showLoading_status.time);
								setTimeout(function(){
									showWlHintContainer();
								}, showLoading_status.time*1000);
								check_isAlive_and_redirect({"page": "SDN.asp", "time": showLoading_status.time});
							}
						}
						httpApi.nvramSet(nvramSet_obj, function(){
							if(isWLclient()) return;
							showLoading(showLoading_status.time);
							setTimeout(function(){
								init_sdn_all_list();
								show_sdn_profilelist();
								if(!window.matchMedia('(max-width: 575px)').matches)
									$("#profile_list_content").find("[sdn_idx=" + selected_sdn_idx + "] .item_text_container").click();
							}, showLoading_status.time*1000);
							if(!isMobile()){
								if(showLoading_status.disconnect){
									check_isAlive_and_redirect({"page": "SDN.asp", "time": showLoading_status.time});
								}
							}
						});
					}
				});
			}
		};

		var $container = $("<div>").addClass("setting_content_container");

		if(view_mode == "popup"){
			Get_Component_Popup_Profile_Title("VPN <#Network#>").appendTo($container)
				.find("#title_close_btn").unbind("click").click(function(e){
					e = e || event;
					e.stopPropagation();
					close_popup_container($container);
				});
		}
		else
			Get_Component_Profile_Title("VPN <#Network#>").appendTo($container);

		var $content_container = $("<div>").addClass("popup_content_container profile_setting").appendTo($container);

		var sdn_name_parm = {"title":"<#QIS_finish_wireless_item1#>", "type":"text", "id":"sdn_name", "need_check":true, "maxlength":32, "openHint":"0_1"};
		Get_Component_Input(sdn_name_parm).appendTo($content_container)
			.find("#" + sdn_name_parm.id + "")
			.unbind("keypress").keypress(function(){
				return validator.isString(this, event);
			});

		let sdn_pwd_parm = {"title":`<#QIS_finish_wireless_item2#>`, "type":"password", "id":"sdn_pwd", "need_check":true, "maxlength":64, "openHint":"0_7"};
		Get_Component_Input(sdn_pwd_parm).attr({"data-group":"wifi_settings"}).appendTo($content_container)
			.find("#" + sdn_pwd_parm.id + "")
			.unbind("keyup").keyup(function(){
				chkPass($(this).val(), "rwd_vpn_pwd", $("#sdn_pwd_strength"));
				resize_iframe_height();
			})
			.unbind("blur").blur(function(){
				if($(this).val() == "")
					$("#" + sdn_pwd_parm.id + "_strength").hide();
			});
		$("<div>").attr({"id":"sdn_pwd_strength", "data-group":"wifi_settings"}).append(Get_Component_PWD_Strength_Meter()).appendTo($content_container).hide();

		var vpn_enabled_parm = {"title":"VPN", "type":"switch", "id":"vpn_enabled"};
		Get_Component_Switch(vpn_enabled_parm).appendTo($content_container)
			.find("#" + vpn_enabled_parm.id + "").click(function(e){
				e = e || event;
				e.stopPropagation();
				var $control_container = $(this).closest(".profile_setting").find("[data-container=VPN_Profiles]");
				if($(this).hasClass("on")){
					$control_container.show();
				}
				else{
					$control_container.hide();
				}
				resize_iframe_height();
			});
		Get_Component_VPN_Profiles().appendTo($content_container);

		$.each(vpnc_profile_list, function(index, item){
			if(item.activate == "0"){
				$content_container.find("[data-container=VPN_Profiles] [data-vpn-type=vpnc][data-idx="+item.vpnc_idx+"]")
					.siblings("[data-component=icon_warning]").show();
			}
		});

		$.each(vpns_rl_json, function(index, item){
			if(item.activate == "0"){
				$content_container.find("[data-container=VPN_Profiles] [data-vpn-type=vpns][data-idx="+item.vpns_idx+"]")
					.siblings("[data-component=icon_warning]").show();
			}
			if(item.sdn_idx != ""){
				$content_container.find("[data-container=VPN_Profiles] [data-vpn-type=vpns][data-idx="+item.vpns_idx+"]").addClass("disabled").attr({"data-disabled":"true"})
					.siblings("[data-component=icon_error]").show();
			}
		});

		var more_config_parm = {"title":"<#MoreConfig#>", "id":"more_config", "slide_target":"more_config_cntr"};
		Get_Component_Slide_Title(more_config_parm).appendTo($content_container);
		var $more_config_cntr = $("<div>").attr({"data-slide_target": "more_config_cntr"}).hide().appendTo($content_container);

		const band_options = wifi_band_options.filter((el) => {
			return (el.value != "5");
		});
		let wifi_band_options_parm = {"title": "WiFi <#Interface#>", "id": "wifi_band", "options": band_options, "set_value": "3"};
		var $sel_wifi_band = Get_Component_Custom_Select(wifi_band_options_parm).appendTo($more_config_cntr).find("#select_" + wifi_band_options_parm.id + "");
		$sel_wifi_band.children("div").click(function(e){
			var options = $(this).attr("value");
			var $profile_setting = $(this).closest(".profile_setting");
			$profile_setting.find("[data-container=AiMesh_List]").next(".validate_hint").remove();
			var $wifi_settings_objs = $profile_setting.find("[data-group=wifi_settings]");
			if(options == "0"){
				$wifi_settings_objs.hide().next(".validate_hint").remove();
				_set_apply_btn_status($profile_setting);
			}
			else{
				$wifi_settings_objs.show().filter("#wizard_aimesh").addClass("arrow_up");
				Set_AiMesh_List_CB($profile_setting.find("[data-container=AiMesh_List]"), options);
				$wifi_settings_objs.find("#sdn_pwd").blur();
				if($wifi_settings_objs.find("#bw_enabled").length > 0){
					if($wifi_settings_objs.find("#bw_enabled").hasClass("on")){
						$wifi_settings_objs.filter("#container_bw").show();
					}
					else{
						$wifi_settings_objs.filter("#container_bw").hide();
					}
				}
				if($wifi_settings_objs.find("#wizard_schedule").length > 0){
					if($wifi_settings_objs.find("#wizard_schedule").hasClass("on"))
						$wifi_settings_objs.filter("#container_wizard_schedule").show();
					else
						$wifi_settings_objs.filter("#container_wizard_schedule").hide();
				}
				_set_apply_btn_status($profile_setting);
			}
			resize_iframe_height();
		});

		var aimesh_parm = {"title":"AiMesh", "id":"wizard_aimesh", "slide_target":"wizard_aimesh_cntr"};
		var $wizard_slide_aimesh = Get_Component_Slide_Title(aimesh_parm).attr({"data-container":"wizard_aimesh_cntr", "data-group":"wifi_settings"}).addClass("arrow_up").appendTo($more_config_cntr);
		var $wizard_aimesh_cntr = $("<div>").attr({"data-slide_target":"wizard_aimesh_cntr", "data-container":"wizard_aimesh_cntr", "data-group":"wifi_settings"}).show().appendTo($more_config_cntr);
		var $AiMesh_List = $("<div>").attr({"data-container":"AiMesh_List"}).append(Get_AiMesh_List_Container()).appendTo($wizard_aimesh_cntr);
		$AiMesh_List.find(".node_content_container").click(function(e){
			e = e || event;
			e.stopPropagation();
			_set_apply_btn_status($(this).closest(".profile_setting"));
		});

		var wizard_schedule = new schedule({
			show_timeset_viewport_callback: resize_iframe_height,
			icon_trash_callback: resize_iframe_height,
			btn_save_callback: resize_iframe_height,
			btn_cancel_callback: resize_iframe_height
		});
		var wizard_schedule_parm = {"title":str_WiFi_sche, "type":"switch", "id":"wizard_schedule", "set_value":"off"};
		Get_Component_Switch(wizard_schedule_parm).attr({"data-group":"wifi_settings"}).appendTo($more_config_cntr).find("#" + wizard_schedule_parm.id + "").click(function(e){
			e = e || event;
			e.stopPropagation();
			var $control_container = $(this).closest(".profile_setting").find("#container_wizard_schedule");
			if($(this).hasClass("on")){
				$control_container.empty().append(wizard_schedule.Get_UI());
				$control_container.show();
			}
			else{
				$control_container.hide();
			}
			resize_iframe_height();
		});
		$("<div>").addClass("profile_setting_item schedule_ui").attr({"id":"container_wizard_schedule", "data-group":"wifi_settings"}).append(wizard_schedule.Get_UI()).hide().appendTo($more_config_cntr);

		var bw_enabled_parm = {"title":"<#Bandwidth_Limiter#>", "type":"switch", "id":"bw_enabled", "set_value":"off"};
		Get_Component_Switch(bw_enabled_parm).attr({"data-group":"wifi_settings"}).appendTo($more_config_cntr)
			.find("#" + bw_enabled_parm.id + "").click(function(e){
				e = e || event;
				e.stopPropagation();
				var $control_container = $(this).closest(".profile_setting").find("#container_bw_dl, #container_bw_ul, #container_bw");
				if($(this).hasClass("on")){
					$control_container.show();
				}
				else{
					$control_container.hide().next(".validate_hint").remove();
				}
				resize_iframe_height();
				_set_apply_btn_status($(this).closest(".profile_setting"));
			});

		var bw_parm = {"title":"<#Bandwidth_Setting#> (Mb/s)", "container_id":"container_bw",
			"type_1":"text", "id_1":"bw_dl", "maxlength_1":12, "need_check_1":true,
			"type_2":"text", "id_2":"bw_ul", "maxlength_2":12, "need_check_2":true};
		Get_Component_Bandwidth_Setting(bw_parm).attr({"data-group":"wifi_settings"}).hide().appendTo($more_config_cntr);

		var access_intranet_parm = {"title":"<#Access_Intranet#>", "type":"switch", "id":"access_intranet", "openHint":"0_26"};
		Get_Component_Switch(access_intranet_parm).appendTo($more_config_cntr);

		var $action_container = $("<div>").addClass("action_container").appendTo($content_container);
		$("<div>").addClass("btn_container apply").html("<#CTL_apply#>").appendTo($action_container);

		$content_container.find("[need_check=true]").keyup(function(){
			_set_apply_btn_status($content_container);
		});
		_set_apply_btn_status($content_container);

		return $container;
	}
	function Get_Wizard_IoT(view_mode){
		var _set_apply_btn_status = function(_obj){
			var $btn_container_apply = $(_obj).find(".action_container .btn_container.apply");
			var isBlank = validate_isBlank($(_obj));
			if(isBlank){
				$btn_container_apply.removeClass("valid_fail").addClass("valid_fail").unbind("click");
			}
			else{
				$btn_container_apply.removeClass("valid_fail").unbind("click").click(function(e){
					e = e || event;
					e.stopPropagation();
					var wizard_type = $(this).closest(".setting_content_container").attr("data-wizard-type");
					if(validate_format_Wizard_Item($(_obj), wizard_type)){
						var sdn_obj = get_new_sdn_profile();
						var sdn_idx = Object.keys(sdn_obj);
						var sdn_profile = sdn_obj[sdn_idx];
						selected_sdn_idx = sdn_profile.sdn_rl.idx;
						var nvramSet_obj = {"action_mode": "apply", "rc_service": "restart_wireless;restart_sdn " + selected_sdn_idx + ";"};
						var rc_append = "";
						var wifi_band = (!is_cfg_ready) ? 1 : (parseInt($(_obj).find("#select_wifi_band").children(".selected").attr("value")));
						sdn_profile.sdn_rl.sdn_name = wizard_type;
						sdn_profile.apg_rl.ssid = $(_obj).find("#sdn_name").val();
						if(wifi_band > 0){
							let wifi_auth = "psk2";
							let wifi_auth_6G = "sae";
							if(wifi_band == 4 || wifi_band == 5 || wifi_band == 6){
								wifi_auth = wifi_auth_6G;
							}
							var $sdn_pwd = $(_obj).find("#sdn_pwd");
							var radius_idx = sdn_profile.apg_rl.apg_idx;
							sdn_profile.apg_rl.security = "<3>" + wifi_auth + ">aes>" + $sdn_pwd.val() + ">" + radius_idx + "";
							sdn_profile.apg_rl.security += "<13>" + wifi_auth + ">aes>" + $sdn_pwd.val() + ">" + radius_idx + "";
							sdn_profile.apg_rl.security += "<16>" + wifi_auth_6G + ">aes>" + $sdn_pwd.val() + ">" + radius_idx + "";
							sdn_profile.apg_rl.security += "<96>" + wifi_auth_6G + ">aes>" + $sdn_pwd.val() + ">" + radius_idx + "";
							sdn_profile.apg_rl.sched = schedule_handle_data.json_array_to_string(wizard_schedule.Get_Value());
							if($(_obj).find("#wizard_schedule").hasClass("on")){
								sdn_profile.apg_rl.timesched = ((sdn_profile.apg_rl.sched == "") ? "0" : "1");
							}
							else{
								sdn_profile.apg_rl.timesched = "0";
							}
						}
						sdn_profile.apg_rl.bw_limit = "<0>>";
						var dut_list = "";
						if(is_cfg_ready){
							if(wifi_band > 0){
								$(_obj).find("[data-container=AiMesh_List]").find(".node_container").each(function(){
									if($(this).find(".icon_checkbox").hasClass("clicked")){
										var node_mac = $(this).attr("data-node-mac");
										var specific_node = cfg_clientlist.filter(function(item, index, array){
											return (item.mac == node_mac);
										})[0];
										if(specific_node != undefined){
											dut_list += get_specific_dut_list(wifi_band, specific_node);
										}
									}
								});
								dut_list += get_unChecked_dut_list(_obj);
							}
							else{
								dut_list = get_dut_list(wifi_band);
							}
						}
						else{
							dut_list = get_dut_list_by_mac("1", cap_mac);
						}
						sdn_profile.apg_rl.dut_list = dut_list;

						sdn_profile.sdn_access_rl = [];
						sdn_profile.apg_rl.ap_isolate = "0";

						if($(_obj).find("#use_main_subnet").hasClass("on")){
							sdn_profile.subnet_rl = {};
							vlan_rl_json = vlan_rl_json.filter(function(item, index, array){
								return (item.vlan_idx != sdn_profile.vlan_rl.vlan_idx);
							});
							sdn_profile.vlan_rl = {};
							sdn_profile.sdn_rl.subnet_idx = "0";
							sdn_profile.sdn_rl.vlan_idx = "0";
						}

						var sdn_all_rl_tmp = JSON.parse(JSON.stringify(sdn_all_rl_json));
						sdn_all_rl_tmp.push(JSON.parse(JSON.stringify(sdn_profile)));
						var vlan_rl_tmp = JSON.parse(JSON.stringify(vlan_rl_json));
						vlan_rl_tmp.push(JSON.parse(JSON.stringify(sdn_profile.vlan_rl)));
						var sdn_all_list = parse_JSONToStr_sdn_all_list({"sdn_all_rl":sdn_all_rl_tmp, "vlan_rl":vlan_rl_tmp});
						if(rc_append != "")
							nvramSet_obj.rc_service = nvramSet_obj.rc_service + rc_append;
						$.extend(nvramSet_obj, sdn_all_list);
						var apgX_rl = parse_apg_rl_to_apgX_rl(sdn_profile.apg_rl);
						$.extend(nvramSet_obj, apgX_rl);
						if(is_QIS_flow){
							nvramSet_obj.sdn_rc_service = nvramSet_obj.rc_service;
							delete nvramSet_obj["rc_service"];
							delete nvramSet_obj["action_mode"];
							$(this).replaceWith($("<img>").attr({"width": "30px", "src": "/images/InternetScan.gif"}));
							apply.SDN_Profile(nvramSet_obj);
							return;
						}
						var showLoading_status = get_showLoading_status(nvramSet_obj.rc_service);
						if(!httpApi.app_dataHandler){
							showLoading();
							close_popup_container("all");
							if(isWLclient()){
								showLoading(showLoading_status.time);
								setTimeout(function(){
									showWlHintContainer();
								}, showLoading_status.time*1000);
								check_isAlive_and_redirect({"page": "SDN.asp", "time": showLoading_status.time});
							}
						}
						httpApi.nvramSet(nvramSet_obj, function(){
							if(isWLclient()) return;
							showLoading(showLoading_status.time);
							setTimeout(function(){
								init_sdn_all_list();
								show_sdn_profilelist();
								if(!window.matchMedia('(max-width: 575px)').matches)
									$("#profile_list_content").find("[sdn_idx=" + selected_sdn_idx + "] .item_text_container").click();
							}, showLoading_status.time*1000);
							if(!isMobile()){
								if(showLoading_status.disconnect){
									check_isAlive_and_redirect({"page": "SDN.asp", "time": showLoading_status.time});
								}
							}
						});
					}
				});
			}
		};

		var $container = $("<div>").addClass("setting_content_container");

		if(view_mode == "popup"){
			Get_Component_Popup_Profile_Title("<#GuestNetwork_IoT#>").appendTo($container)
				.find("#title_close_btn").unbind("click").click(function(e){
					e = e || event;
					e.stopPropagation();
					close_popup_container($container);
				});
		}
		else
			Get_Component_Profile_Title("<#GuestNetwork_IoT#>").appendTo($container);

		var $content_container = $("<div>").addClass("popup_content_container profile_setting").appendTo($container);

		var sdn_name_parm = {"title":"<#QIS_finish_wireless_item1#>", "type":"text", "id":"sdn_name", "need_check":true, "maxlength":32, "openHint":"0_1"};
		Get_Component_Input(sdn_name_parm).appendTo($content_container)
			.find("#" + sdn_name_parm.id + "")
			.unbind("keypress").keypress(function(){
				return validator.isString(this, event);
			});

		let sdn_pwd_parm = {"title":`<#QIS_finish_wireless_item2#>`, "type":"password", "id":"sdn_pwd", "need_check":true, "maxlength":64, "openHint":"0_7"};
		Get_Component_Input(sdn_pwd_parm).attr({"data-group":"wifi_settings"}).appendTo($content_container)
			.find("#" + sdn_pwd_parm.id + "")
			.unbind("keyup").keyup(function(){
				chkPass($(this).val(), "rwd_vpn_pwd", $("#sdn_pwd_strength"));
				resize_iframe_height();
			})
			.unbind("blur").blur(function(){
				if($(this).val() == "")
					$("#" + sdn_pwd_parm.id + "_strength").hide();
			});
		$("<div>").attr({"id":"sdn_pwd_strength", "data-group":"wifi_settings"}).append(Get_Component_PWD_Strength_Meter()).appendTo($content_container).hide();

		if(isSwMode("rt")){
			let use_main_subnet_parm = {"title":`<#GuestNetwork_subnet_as_main#>`, "type":"switch", "id":"use_main_subnet", "set_value":"on"};
			Get_Component_Switch(use_main_subnet_parm).appendTo($content_container).find("#" + use_main_subnet_parm.id + "").click(function(e){
				e = e || event;
				e.stopPropagation();
				const $profile_setting = $(this).closest(".profile_setting");
				$profile_setting.find("[data-container=AiMesh_List]").next(".validate_hint").remove();
				Set_AiMesh_List_CB(
					$profile_setting.find("[data-container=AiMesh_List]"),
					$profile_setting.find("#select_wifi_band").children(".selected").attr("value")
				);
				_set_apply_btn_status($profile_setting);
				resize_iframe_height();
			});
		}

		var more_config_parm = {"title":"<#MoreConfig#>", "id":"more_config", "slide_target":"more_config_cntr"};
		Get_Component_Slide_Title(more_config_parm).appendTo($content_container);
		var $more_config_cntr = $("<div>").attr({"data-slide_target": "more_config_cntr"}).hide().appendTo($content_container);

		const band_options = wifi_band_options.filter((el) => {
			return (el.value != "5");
		});
		let wifi_band_options_parm = {"title": "WiFi <#Interface#>", "id": "wifi_band", "options": band_options, "set_value": "1"};
		var $sel_wifi_band = Get_Component_Custom_Select(wifi_band_options_parm).appendTo($more_config_cntr).find("#select_" + wifi_band_options_parm.id + "");
		$sel_wifi_band.children("div").click(function(e){
			var options = $(this).attr("value");
			var $profile_setting = $(this).closest(".profile_setting");
			$profile_setting.find("[data-container=AiMesh_List]").next(".validate_hint").remove();
			var $wifi_settings_objs = $profile_setting.find("[data-group=wifi_settings]");
			if(options == "0"){
				$wifi_settings_objs.hide().next(".validate_hint").remove();
				_set_apply_btn_status($profile_setting);
			}
			else{
				$wifi_settings_objs.show().filter("#wizard_aimesh").addClass("arrow_up");
				Set_AiMesh_List_CB($profile_setting.find("[data-container=AiMesh_List]"), options);
				$wifi_settings_objs.find("#sdn_pwd").blur();
				if($wifi_settings_objs.find("#wizard_schedule").length > 0){
					if($wifi_settings_objs.find("#wizard_schedule").hasClass("on"))
						$wifi_settings_objs.filter("#container_wizard_schedule").show();
					else
						$wifi_settings_objs.filter("#container_wizard_schedule").hide();
				}
				_set_apply_btn_status($profile_setting);
			}
			resize_iframe_height();
		});

		var aimesh_parm = {"title":"AiMesh", "id":"wizard_aimesh", "slide_target":"wizard_aimesh_cntr"};
		var $wizard_slide_aimesh = Get_Component_Slide_Title(aimesh_parm).attr({"data-container":"wizard_aimesh_cntr", "data-group":"wifi_settings"}).addClass("arrow_up").appendTo($more_config_cntr);
		var $wizard_aimesh_cntr = $("<div>").attr({"data-slide_target":"wizard_aimesh_cntr", "data-container":"wizard_aimesh_cntr", "data-group":"wifi_settings"}).show().appendTo($more_config_cntr);
		var $AiMesh_List = $("<div>").attr({"data-container":"AiMesh_List"}).append(Get_AiMesh_List_Container()).appendTo($wizard_aimesh_cntr);
		$AiMesh_List.find(".node_content_container").click(function(e){
			e = e || event;
			e.stopPropagation();
			_set_apply_btn_status($(this).closest(".profile_setting"));
		});

		var wizard_schedule = new schedule({
			show_timeset_viewport_callback: resize_iframe_height,
			icon_trash_callback: resize_iframe_height,
			btn_save_callback: resize_iframe_height,
			btn_cancel_callback: resize_iframe_height
		});
		var wizard_schedule_parm = {"title":str_WiFi_sche, "type":"switch", "id":"wizard_schedule", "set_value":"off"};
		Get_Component_Switch(wizard_schedule_parm).attr({"data-group":"wifi_settings"}).appendTo($more_config_cntr).find("#" + wizard_schedule_parm.id + "").click(function(e){
			e = e || event;
			e.stopPropagation();
			var $control_container = $(this).closest(".profile_setting").find("#container_wizard_schedule");
			if($(this).hasClass("on")){
				$control_container.empty().append(wizard_schedule.Get_UI());
				$control_container.show();
			}
			else{
				$control_container.hide();
			}
			resize_iframe_height();
		});
		$("<div>").addClass("profile_setting_item schedule_ui").attr({"id":"container_wizard_schedule", "data-group":"wifi_settings"}).append(wizard_schedule.Get_UI()).hide().appendTo($more_config_cntr);

		var $action_container = $("<div>").addClass("action_container").appendTo($content_container);
		$("<div>").addClass("btn_container apply").html("<#CTL_apply#>").appendTo($action_container);

		$content_container.find("[need_check=true]").keyup(function(){
			_set_apply_btn_status($content_container);
		});
		_set_apply_btn_status($content_container);

		return $container;
	}
	function Get_Wizard_Guest(view_mode){
		var _set_apply_btn_status = function(_obj){
			var $btn_container_apply = $(_obj).find(".action_container .btn_container.apply");
			var isBlank = validate_isBlank($(_obj));
			if(isBlank){
				$btn_container_apply.removeClass("valid_fail").addClass("valid_fail").unbind("click");
			}
			else{
				$btn_container_apply.removeClass("valid_fail").unbind("click").click(function(e){
					e = e || event;
					e.stopPropagation();
					var wizard_type = $(this).closest(".setting_content_container").attr("data-wizard-type");
					if(validate_format_Wizard_Item($(_obj), wizard_type)){
						var sdn_obj = get_new_sdn_profile();
						var sdn_idx = Object.keys(sdn_obj);
						var sdn_profile = sdn_obj[sdn_idx];
						selected_sdn_idx = sdn_profile.sdn_rl.idx;
						selected_vlan_idx = sdn_profile.sdn_rl.vlan_idx;
						var nvramSet_obj = {"action_mode": "apply", "rc_service": "restart_wireless;restart_sdn " + selected_sdn_idx + ";"};
						var rc_append = "";
						var wifi_band = (!is_cfg_ready) ? 3 : (parseInt($(_obj).find("#select_wifi_band").children(".selected").attr("value")));
						sdn_profile.sdn_rl.sdn_name = wizard_type;
						sdn_profile.apg_rl.ssid = $(_obj).find("#sdn_name").val();
						if(wifi_band > 0){
							var sec_option_id = $(_obj).find("#security_guest .switch_text_container").children(".selected").attr("data-option-id");
							var wifi_pwd = "";
							var wifi_auth = "psk2";
							let wifi_auth_6G = "sae";
							if(sec_option_id == "open"){
								wifi_auth = "open";
								wifi_auth_6G = "owe";
								wifi_pwd = "";
							}
							else if(sec_option_id == "pwd"){
								wifi_auth = "psk2";
								wifi_pwd = $(_obj).find("#sdn_pwd").val();
							}
							if(wifi_band == 4 || wifi_band == 5 || wifi_band == 6){
								if(sec_option_id == "open")
									wifi_auth = "openowe";
								else{
									wifi_auth = wifi_auth_6G;
								}
							}
							var radius_idx = sdn_profile.apg_rl.apg_idx;
							sdn_profile.apg_rl.security = "<3>" + wifi_auth + ">aes>" + wifi_pwd + ">" + radius_idx + "";
							sdn_profile.apg_rl.security += "<13>" + wifi_auth + ">aes>" + wifi_pwd + ">" + radius_idx + "";
							sdn_profile.apg_rl.security += "<16>" + wifi_auth_6G + ">aes>" + wifi_pwd + ">" + radius_idx + "";
							sdn_profile.apg_rl.security += "<96>" + wifi_auth_6G + ">aes>" + wifi_pwd + ">" + radius_idx + "";
							sdn_profile.apg_rl.sched = schedule_handle_data.json_array_to_string(wizard_schedule.Get_Value());
							if($(_obj).find("#wizard_schedule").hasClass("on")){
								sdn_profile.apg_rl.timesched = wizard_schedule.Get_Value_Mode();
								if(sdn_profile.apg_rl.timesched == "1"){
									sdn_profile.apg_rl.timesched = ((sdn_profile.apg_rl.sched == "") ? "0" : "1");
								}
							}
							else{
								sdn_profile.apg_rl.timesched = "0";
							}
							if($(_obj).find("#bw_enabled").hasClass("on")){
								sdn_profile.apg_rl.bw_limit = "<1>" + ($(_obj).find("#bw_ul").val())*1024 + ">" + ($(_obj).find("#bw_dl").val())*1024;
								nvramSet_obj.qos_enable = "1";
								nvramSet_obj.qos_type = "2";
								rc_append += "restart_qos;restart_firewall;";
							}
							else
								sdn_profile.apg_rl.bw_limit = "<0>>";
						}
						var dut_list = "";
						if(is_cfg_ready){
							if(wifi_band > 0){
								$(_obj).find("[data-container=AiMesh_List]").find(".node_container").each(function(){
									if($(this).find(".icon_checkbox").hasClass("clicked")){
										var node_mac = $(this).attr("data-node-mac");
										var specific_node = cfg_clientlist.filter(function(item, index, array){
											return (item.mac == node_mac);
										})[0];
										if(specific_node != undefined){
											dut_list += get_specific_dut_list(wifi_band, specific_node);
										}
									}
								});
								dut_list += get_unChecked_dut_list(_obj);
							}
							else{
								dut_list = get_dut_list(wifi_band);
							}
						}
						else{
							dut_list = get_dut_list_by_mac("3", cap_mac);
						}
						sdn_profile.apg_rl.dut_list = dut_list;

						sdn_profile.sdn_access_rl = [];
						if($(_obj).find("#access_intranet").hasClass("on")){
							sdn_profile.sdn_access_rl.push({"access_sdn_idx": "0", "sdn_idx": sdn_profile.sdn_rl.idx});
						}
						sdn_profile.apg_rl.ap_isolate = isSwMode("rt") ? "1" : "0";

						if(rc_append != ""){
							nvramSet_obj.rc_service = nvramSet_obj.rc_service + rc_append;
						}
						var showLoading_status = get_showLoading_status(nvramSet_obj.rc_service);
						if(wifi_band > 0){
							if(sdn_profile.apg_rl.timesched == "2"){
								sdn_profile.apg_rl.expiretime = accesstime_handle_data.json_array_to_string(wizard_schedule.Get_Value_AccessTime(showLoading_status.time));
							}
						}
						var sdn_all_rl_tmp = JSON.parse(JSON.stringify(sdn_all_rl_json));
						sdn_all_rl_tmp.push(JSON.parse(JSON.stringify(sdn_profile)));
						var vlan_rl_tmp = JSON.parse(JSON.stringify(vlan_rl_json));
						vlan_rl_tmp.push(JSON.parse(JSON.stringify(sdn_profile.vlan_rl)));
						var sdn_all_list = parse_JSONToStr_sdn_all_list({"sdn_all_rl":sdn_all_rl_tmp, "vlan_rl":vlan_rl_tmp});
						$.extend(nvramSet_obj, sdn_all_list);
						var apgX_rl = parse_apg_rl_to_apgX_rl(sdn_profile.apg_rl);
						$.extend(nvramSet_obj, apgX_rl);
						if(is_QIS_flow){
							nvramSet_obj.sdn_rc_service = nvramSet_obj.rc_service;
							delete nvramSet_obj["rc_service"];
							delete nvramSet_obj["action_mode"];
							$(this).replaceWith($("<img>").attr({"width": "30px", "src": "/images/InternetScan.gif"}));
							apply.SDN_Profile(nvramSet_obj);
							return;
						}
						if(!httpApi.app_dataHandler){
							showLoading();
							close_popup_container("all");
							if(isWLclient()){
								showLoading(showLoading_status.time);
								setTimeout(function(){
									showWlHintContainer();
								}, showLoading_status.time*1000);
								check_isAlive_and_redirect({"page": "SDN.asp", "time": showLoading_status.time});
							}
						}
						httpApi.nvramSet(nvramSet_obj, function(){
							if(isWLclient()) return;
							showLoading(showLoading_status.time);
							setTimeout(function(){
								init_sdn_all_list();
								show_sdn_profilelist();
								if(!window.matchMedia('(max-width: 575px)').matches)
									$("#profile_list_content").find("[sdn_idx=" + selected_sdn_idx + "] .item_text_container").click();
							}, showLoading_status.time*1000);
							if(!isMobile()){
								if(showLoading_status.disconnect){
									check_isAlive_and_redirect({"page": "SDN.asp", "time": showLoading_status.time});
								}
							}
						});
					}
				});
			}
		};

		var $container = $("<div>").addClass("setting_content_container");

		if(view_mode == "popup"){
			Get_Component_Popup_Profile_Title("<#Guest_Network#>").appendTo($container)
				.find("#title_close_btn").unbind("click").click(function(e){
					e = e || event;
					e.stopPropagation();
					close_popup_container($container);
				});
		}
		else
			Get_Component_Profile_Title("<#Guest_Network#>").appendTo($container);

		var $content_container = $("<div>").addClass("popup_content_container profile_setting").appendTo($container);

		var sdn_name_parm = {"title":"<#QIS_finish_wireless_item1#>", "type":"text", "id":"sdn_name", "need_check":true, "maxlength":32, "openHint":"0_1"};
		Get_Component_Input(sdn_name_parm).appendTo($content_container)
			.find("#" + sdn_name_parm.id + "")
			.unbind("keypress").keypress(function(){
				return validator.isString(this, event);
			});

		var security_options = [{"text":"Open System","option_id":"open"}, {"text":"<#HSDPAConfig_Password_itemname#>","option_id":"pwd"}];
		var security_parm = {"title":"<#Security#>", "options": security_options, "container_id":"security_guest"};
		Get_Component_Switch_Text(security_parm).attr({"data-group":"wifi_settings"}).appendTo($content_container)
			.find(".switch_text_container > .switch_text_item")
			.click(function(e){
				e = e || event;
				e.stopPropagation();
				var option_id = $(this).attr("data-option-id");
				$(this).closest(".profile_setting").find("[data-sec-option-id]").hide().next(".validate_hint").remove();
				$(this).closest(".profile_setting").find("[data-sec-option-id=" + option_id + "]").show();
				if(option_id == "pwd"){
					$(this).closest(".profile_setting").find("#sdn_pwd_strength").hide();
				}
				resize_iframe_height();
				_set_apply_btn_status($(this).closest(".profile_setting"));
			});

		let sdn_pwd_parm = {"title":`<#QIS_finish_wireless_item2#>`, "type":"password", "id":"sdn_pwd", "need_check":true, "maxlength":64, "openHint":"0_7"};
		Get_Component_Input(sdn_pwd_parm).attr({"data-sec-option-id":"pwd", "data-group":"wifi_settings"}).appendTo($content_container)
			.find("#" + sdn_pwd_parm.id + "")
			.unbind("keyup").keyup(function(){
				chkPass($(this).val(), "rwd_vpn_pwd", $("#sdn_pwd_strength"));
				resize_iframe_height();
			})
			.unbind("blur").blur(function(){
				if($(this).val() == "")
					$("#" + sdn_pwd_parm.id + "_strength").hide();
			});
		$("<div>").attr({"id":"sdn_pwd_strength", "data-sec-option-id":"pwd", "data-group":"wifi_settings"}).append(Get_Component_PWD_Strength_Meter()).appendTo($content_container).hide();

		$content_container.find("[data-sec-option-id=pwd]").hide();

		var wizard_schedule = new schedule({
			AccTime_support: true,
			mode:2,
			show_timeset_viewport_callback: resize_iframe_height,
			icon_trash_callback: resize_iframe_height,
			btn_save_callback: resize_iframe_height,
			btn_cancel_callback: resize_iframe_height
		});
		var wizard_schedule_parm = {"title":str_WiFi_sche, "type":"switch", "id":"wizard_schedule", "set_value":"on"};
		Get_Component_Switch(wizard_schedule_parm).attr({"data-group":"wifi_settings"}).appendTo($content_container).find("#" + wizard_schedule_parm.id + "").click(function(e){
			e = e || event;
			e.stopPropagation();
			var $control_container = $(this).closest(".profile_setting").find("#container_wizard_schedule");
			if($(this).hasClass("on")){
				$control_container.empty().append(wizard_schedule.Get_UI());
				$control_container.show();
			}
			else{
				$control_container.hide();
			}
			resize_iframe_height();
		});
		$("<div>").addClass("profile_setting_item schedule_ui").attr({"id":"container_wizard_schedule", "data-group":"wifi_settings"}).append(wizard_schedule.Get_UI()).appendTo($content_container);

		var more_config_parm = {"title":"<#MoreConfig#>", "id":"more_config", "slide_target":"more_config_cntr"};
		Get_Component_Slide_Title(more_config_parm).appendTo($content_container);
		var $more_config_cntr = $("<div>").attr({"data-slide_target": "more_config_cntr"}).hide().appendTo($content_container);

		const band_options = wifi_band_options.filter((el) => {
			return (el.value != "5");
		});
		let wifi_band_options_parm = {"title": "WiFi <#Interface#>", "id": "wifi_band", "options": band_options, "set_value": "3"};
		var $sel_wifi_band = Get_Component_Custom_Select(wifi_band_options_parm).appendTo($more_config_cntr).find("#select_" + wifi_band_options_parm.id + "");
		$sel_wifi_band.children("div").click(function(e){
			var options = $(this).attr("value");
			var $profile_setting = $(this).closest(".profile_setting");
			$profile_setting.find("[data-container=AiMesh_List]").next(".validate_hint").remove();
			var $wifi_settings_objs = $profile_setting.find("[data-group=wifi_settings]");
			if(options == "0"){
				$wifi_settings_objs.hide().next(".validate_hint").remove();
				_set_apply_btn_status($profile_setting);
			}
			else{
				$wifi_settings_objs.show().filter("#wizard_aimesh").addClass("arrow_up");
				Set_AiMesh_List_CB($profile_setting.find("[data-container=AiMesh_List]"), options);
				if($wifi_settings_objs.find(".switch_text_container").length > 0){
					$wifi_settings_objs.find(".switch_text_container .switch_text_item.selected").click();
					if($wifi_settings_objs.filter("#security_guest").find("[data-option-id=open]").length > 0){
						$wifi_settings_objs.filter("#security_guest")
							.find("[data-option-id=open]")
							.html(()=>{return ((options == "4" || options == "5" || options == "6") ? `<#Wireless_Encryption_OWE#>` : `Open System`);});
					}
				}
				if($wifi_settings_objs.find("#bw_enabled").length > 0){
					if($wifi_settings_objs.find("#bw_enabled").hasClass("on")){
						$wifi_settings_objs.filter("#container_bw").show();
					}
					else{
						$wifi_settings_objs.filter("#container_bw").hide();
					}
				}
				if($wifi_settings_objs.find("#wizard_schedule").length > 0){
					if($wifi_settings_objs.find("#wizard_schedule").hasClass("on"))
						$wifi_settings_objs.filter("#container_wizard_schedule").show();
					else
						$wifi_settings_objs.filter("#container_wizard_schedule").hide();
				}
				_set_apply_btn_status($profile_setting);
			}
			resize_iframe_height();
		});

		var aimesh_parm = {"title":"AiMesh", "id":"wizard_aimesh", "slide_target":"wizard_aimesh_cntr"};
		var $wizard_slide_aimesh = Get_Component_Slide_Title(aimesh_parm).attr({"data-container":"wizard_aimesh_cntr", "data-group":"wifi_settings"}).addClass("arrow_up").appendTo($more_config_cntr);
		var $wizard_aimesh_cntr = $("<div>").attr({"data-slide_target":"wizard_aimesh_cntr", "data-container":"wizard_aimesh_cntr", "data-group":"wifi_settings"}).show().appendTo($more_config_cntr);
		var $AiMesh_List = $("<div>").attr({"data-container":"AiMesh_List"}).append(Get_AiMesh_List_Container()).appendTo($wizard_aimesh_cntr);
		$AiMesh_List.find(".node_content_container").click(function(e){
			e = e || event;
			e.stopPropagation();
			_set_apply_btn_status($(this).closest(".profile_setting"));
		});

		if(isSwMode("rt")){
			var bw_enabled_parm = {"title":"<#Bandwidth_Limiter#>", "type":"switch", "id":"bw_enabled", "set_value":"off"};
			Get_Component_Switch(bw_enabled_parm).attr({"data-group":"wifi_settings"}).appendTo($more_config_cntr)
				.find("#" + bw_enabled_parm.id + "").click(function(e){
					e = e || event;
					e.stopPropagation();
					var $control_container = $(this).closest(".profile_setting").find("#container_bw_dl, #container_bw_ul, #container_bw");
					if($(this).hasClass("on")){
						$control_container.show();
					}
					else{
						$control_container.hide().next(".validate_hint").remove();
					}
					resize_iframe_height();
					_set_apply_btn_status($(this).closest(".profile_setting"));
				});

			var bw_parm = {"title":"<#Bandwidth_Setting#> (Mb/s)", "container_id":"container_bw",
				"type_1":"text", "id_1":"bw_dl", "maxlength_1":12, "need_check_1":true,
				"type_2":"text", "id_2":"bw_ul", "maxlength_2":12, "need_check_2":true};
			Get_Component_Bandwidth_Setting(bw_parm).attr({"data-group":"wifi_settings"}).hide().appendTo($more_config_cntr);

			var access_intranet_parm = {"title":"<#Access_Intranet#>", "type":"switch", "id":"access_intranet", "openHint":"0_26", "set_value":"off"};
			Get_Component_Switch(access_intranet_parm).appendTo($more_config_cntr);
		}

		var $action_container = $("<div>").addClass("action_container").appendTo($content_container);
		$("<div>").addClass("btn_container apply").html("<#CTL_apply#>").appendTo($action_container);

		$content_container.find("[need_check=true]").keyup(function(){
			_set_apply_btn_status($content_container);
		});
		_set_apply_btn_status($content_container);

		return $container;
	}
	function Get_Wizard_Customized(view_mode){
		var _set_apply_btn_status = function(_obj){
			var $btn_container_apply = $(_obj).find(".action_container .btn_container.apply");
			var isBlank = validate_isBlank($(_obj));
			if(isBlank){
				$btn_container_apply.removeClass("valid_fail").addClass("valid_fail").unbind("click");
			}
			else{
				$btn_container_apply.removeClass("valid_fail").unbind("click").click(function(e){
					e = e || event;
					e.stopPropagation();
					var wizard_type = $(this).closest(".setting_content_container").attr("data-wizard-type");
					if(validate_format_Wizard_Item($(_obj), wizard_type)){
						var sdn_obj = get_new_sdn_profile();
						var sdn_idx = Object.keys(sdn_obj);
						var sdn_profile = sdn_obj[sdn_idx];
						selected_sdn_idx = sdn_profile.sdn_rl.idx;
						var nvramSet_obj = {"action_mode": "apply", "rc_service": "restart_wireless;restart_sdn " + selected_sdn_idx + ";"};
						var rc_append = "";
						var wifi_band = parseInt($(_obj).find("#select_wifi_band").children(".selected").attr("value"));
						sdn_profile.sdn_rl.sdn_name = wizard_type;
						sdn_profile.apg_rl.ssid = $(_obj).find("#sdn_name").val();
						var dut_list = "";
						if(wifi_band > 0){
							let wifi_auth = "psk2";
							let wifi_auth_6G = "sae";
							if(wifi_band == 4 || wifi_band == 5 || wifi_band == 6){
								wifi_auth = wifi_auth_6G;
							}
							var $sdn_pwd = $(_obj).find("#sdn_pwd");
							var radius_idx = sdn_profile.apg_rl.apg_idx;
							sdn_profile.apg_rl.security = "<3>" + wifi_auth + ">aes>" + $sdn_pwd.val() + ">" + radius_idx + "";
							sdn_profile.apg_rl.security += "<13>" + wifi_auth + ">aes>" + $sdn_pwd.val() + ">" + radius_idx + "";
							sdn_profile.apg_rl.security += "<16>" + wifi_auth_6G + ">aes>" + $sdn_pwd.val() + ">" + radius_idx + "";
							sdn_profile.apg_rl.security += "<96>" + wifi_auth_6G + ">aes>" + $sdn_pwd.val() + ">" + radius_idx + "";
							sdn_profile.apg_rl.sched = schedule_handle_data.json_array_to_string(wizard_schedule.Get_Value());
							if($(_obj).find("#wizard_schedule").hasClass("on")){
								sdn_profile.apg_rl.timesched = ((sdn_profile.apg_rl.sched == "") ? "0" : "1");
							}
							else{
								sdn_profile.apg_rl.timesched = "0";
							}
							if($(_obj).find("#bw_enabled").hasClass("on")){
								sdn_profile.apg_rl.bw_limit = "<1>" + ($(_obj).find("#bw_ul").val())*1024 + ">" + ($(_obj).find("#bw_dl").val())*1024;
								nvramSet_obj.qos_enable = "1";
								nvramSet_obj.qos_type = "2";
								rc_append += "restart_qos;restart_firewall;";
							}
							else
								sdn_profile.apg_rl.bw_limit = "<0>>";

							$(_obj).find("[data-container=AiMesh_List]").find(".node_container").each(function(){
								if($(this).find(".icon_checkbox").hasClass("clicked")){
									var node_mac = $(this).attr("data-node-mac");
									var specific_node = cfg_clientlist.filter(function(item, index, array){
										return (item.mac == node_mac);
									})[0];
									if(specific_node != undefined){
										dut_list += get_specific_dut_list(wifi_band, specific_node);
									}
								}
							});
							dut_list += get_unChecked_dut_list(_obj);
						}
						else{
							dut_list = get_dut_list(wifi_band);
						}
						sdn_profile.apg_rl.dut_list = dut_list;

						sdn_profile.sdn_access_rl = [];
						if($(_obj).find("#access_intranet").hasClass("on")){
							sdn_profile.sdn_access_rl.push({"access_sdn_idx": "0", "sdn_idx": sdn_profile.sdn_rl.idx});
						}
						sdn_profile.apg_rl.ap_isolate = "0";

						if($(_obj).find("#use_main_subnet").hasClass("on")){
							sdn_profile.subnet_rl = {};
							vlan_rl_json = vlan_rl_json.filter(function(item, index, array){
								return (item.vlan_idx != sdn_profile.vlan_rl.vlan_idx);
							});
							sdn_profile.vlan_rl = {};
							sdn_profile.sdn_rl.subnet_idx = "0";
							sdn_profile.sdn_rl.vlan_idx = "0";
						}

						var sdn_all_rl_tmp = JSON.parse(JSON.stringify(sdn_all_rl_json));
						sdn_all_rl_tmp.push(JSON.parse(JSON.stringify(sdn_profile)));
						var vlan_rl_tmp = JSON.parse(JSON.stringify(vlan_rl_json));
						vlan_rl_tmp.push(JSON.parse(JSON.stringify(sdn_profile.vlan_rl)));
						var sdn_all_list = parse_JSONToStr_sdn_all_list({"sdn_all_rl":sdn_all_rl_tmp, "vlan_rl":vlan_rl_tmp});
						if(rc_append != "")
							nvramSet_obj.rc_service = nvramSet_obj.rc_service + rc_append;
						$.extend(nvramSet_obj, sdn_all_list);
						var apgX_rl = parse_apg_rl_to_apgX_rl(sdn_profile.apg_rl);
						$.extend(nvramSet_obj, apgX_rl);
						var showLoading_status = get_showLoading_status(nvramSet_obj.rc_service);
						if(!httpApi.app_dataHandler){
							showLoading();
							close_popup_container("all");
							if(isWLclient()){
								showLoading(showLoading_status.time);
								setTimeout(function(){
									showWlHintContainer();
								}, showLoading_status.time*1000);
								check_isAlive_and_redirect({"page": "SDN.asp", "time": showLoading_status.time});
							}
						}
						httpApi.nvramSet(nvramSet_obj, function(){
							if(isWLclient()) return;
							showLoading(showLoading_status.time);
							setTimeout(function(){
								init_sdn_all_list();
								show_sdn_profilelist();
								if(!window.matchMedia('(max-width: 575px)').matches)
									$("#profile_list_content").find("[sdn_idx=" + selected_sdn_idx + "] .item_text_container").click();
							}, showLoading_status.time*1000);
							if(!isMobile()){
								if(showLoading_status.disconnect){
									check_isAlive_and_redirect({"page": "SDN.asp", "time": showLoading_status.time});
								}
							}
						});
					}
				});
			}
		};

		var $container = $("<div>").addClass("setting_content_container");

		if(view_mode == "popup"){
			Get_Component_Popup_Profile_Title("<#GuestNetwork_Customized#>").appendTo($container)
				.find("#title_close_btn").unbind("click").click(function(e){
					e = e || event;
					e.stopPropagation();
					close_popup_container($container);
				});
		}
		else
			Get_Component_Profile_Title("<#GuestNetwork_Customized#>").appendTo($container);

		var $content_container = $("<div>").addClass("popup_content_container profile_setting").appendTo($container);

		var sdn_name_parm = {"title":"<#QIS_finish_wireless_item1#>", "type":"text", "id":"sdn_name", "need_check":true, "maxlength":32, "openHint":"0_1"};
		Get_Component_Input(sdn_name_parm).appendTo($content_container)
			.find("#" + sdn_name_parm.id + "")
			.unbind("keypress").keypress(function(){
				return validator.isString(this, event);
			});

		let sdn_pwd_parm = {"title":`<#QIS_finish_wireless_item2#>`, "type":"password", "id":"sdn_pwd", "need_check":true, "maxlength":64, "openHint":"0_7"};
		Get_Component_Input(sdn_pwd_parm).attr({"data-group":"wifi_settings"}).appendTo($content_container)
			.find("#" + sdn_pwd_parm.id + "")
			.unbind("keyup").keyup(function(){
				chkPass($(this).val(), "rwd_vpn_pwd", $("#sdn_pwd_strength"));
				resize_iframe_height();
			})
			.unbind("blur").blur(function(){
				if($(this).val() == "")
					$("#" + sdn_pwd_parm.id + "_strength").hide();
			});
		$("<div>").attr({"id":"sdn_pwd_strength", "data-group":"wifi_settings"}).append(Get_Component_PWD_Strength_Meter()).appendTo($content_container).hide();

		var more_config_parm = {"title":"<#MoreConfig#>", "id":"more_config", "slide_target":"more_config_cntr"};
		Get_Component_Slide_Title(more_config_parm).appendTo($content_container);
		var $more_config_cntr = $("<div>").attr({"data-slide_target": "more_config_cntr"}).hide().appendTo($content_container);

		const band_options = wifi_band_options.filter((el) => {
			return (el.value != "5");
		});
		let wifi_band_options_parm = {"title": "WiFi <#Interface#>", "id": "wifi_band", "options": band_options, "set_value": "3"};
		var $sel_wifi_band = Get_Component_Custom_Select(wifi_band_options_parm).appendTo($more_config_cntr).find("#select_" + wifi_band_options_parm.id + "");
		$sel_wifi_band.children("div").click(function(e){
			var options = $(this).attr("value");
			var $profile_setting = $(this).closest(".profile_setting");
			$profile_setting.find("[data-container=AiMesh_List]").next(".validate_hint").remove();
			var $wifi_settings_objs = $profile_setting.find("[data-group=wifi_settings]");
			if(options == "0"){
				$wifi_settings_objs.hide().next(".validate_hint").remove();
				_set_apply_btn_status($profile_setting);
			}
			else{
				$wifi_settings_objs.show().filter("#wizard_aimesh").addClass("arrow_up");
				Set_AiMesh_List_CB($profile_setting.find("[data-container=AiMesh_List]"), options);
				$wifi_settings_objs.find("#sdn_pwd").blur();
				if($wifi_settings_objs.find("#bw_enabled").length > 0){
					if($wifi_settings_objs.find("#bw_enabled").hasClass("on")){
						$wifi_settings_objs.filter("#container_bw").show();
					}
					else{
						$wifi_settings_objs.filter("#container_bw").hide();
					}
				}
				if($wifi_settings_objs.find("#wizard_schedule").length > 0){
					if($wifi_settings_objs.find("#wizard_schedule").hasClass("on"))
						$wifi_settings_objs.filter("#container_wizard_schedule").show();
					else
						$wifi_settings_objs.filter("#container_wizard_schedule").hide();
				}
				_set_apply_btn_status($profile_setting);
			}
			resize_iframe_height();
		});

		var aimesh_parm = {"title":"AiMesh", "id":"wizard_aimesh", "slide_target":"wizard_aimesh_cntr"};
		var $wizard_slide_aimesh = Get_Component_Slide_Title(aimesh_parm).attr({"data-container":"wizard_aimesh_cntr", "data-group":"wifi_settings"}).addClass("arrow_up").appendTo($more_config_cntr);
		var $wizard_aimesh_cntr = $("<div>").attr({"data-slide_target":"wizard_aimesh_cntr", "data-container":"wizard_aimesh_cntr", "data-group":"wifi_settings"}).show().appendTo($more_config_cntr);
		var $AiMesh_List = $("<div>").attr({"data-container":"AiMesh_List"}).append(Get_AiMesh_List_Container()).appendTo($wizard_aimesh_cntr);
		$AiMesh_List.find(".node_content_container").click(function(e){
			e = e || event;
			e.stopPropagation();
			_set_apply_btn_status($(this).closest(".profile_setting"));
		});

		var wizard_schedule = new schedule({
			show_timeset_viewport_callback: resize_iframe_height,
			icon_trash_callback: resize_iframe_height,
			btn_save_callback: resize_iframe_height,
			btn_cancel_callback: resize_iframe_height
		});
		var wizard_schedule_parm = {"title":str_WiFi_sche, "type":"switch", "id":"wizard_schedule", "set_value":"off"};
		Get_Component_Switch(wizard_schedule_parm).attr({"data-group":"wifi_settings"}).appendTo($more_config_cntr).find("#" + wizard_schedule_parm.id + "").click(function(e){
			e = e || event;
			e.stopPropagation();
			var $control_container = $(this).closest(".profile_setting").find("#container_wizard_schedule");
			if($(this).hasClass("on")){
				$control_container.empty().append(wizard_schedule.Get_UI());
				$control_container.show();
			}
			else{
				$control_container.hide();
			}
			resize_iframe_height();
		});
		$("<div>").addClass("profile_setting_item schedule_ui").attr({"id":"container_wizard_schedule", "data-group":"wifi_settings"}).append(wizard_schedule.Get_UI()).hide().appendTo($more_config_cntr);

		if(isSwMode("rt")){
			var bw_enabled_parm = {"title":"<#Bandwidth_Limiter#>", "type":"switch", "id":"bw_enabled", "set_value":"off"};
			Get_Component_Switch(bw_enabled_parm).attr({"data-group":"wifi_settings"}).appendTo($more_config_cntr)
				.find("#" + bw_enabled_parm.id + "").click(function(e){
					e = e || event;
					e.stopPropagation();
					var $control_container = $(this).closest(".profile_setting").find("#container_bw_dl, #container_bw_ul, #container_bw");
					if($(this).hasClass("on")){
						$control_container.show();
					}
					else{
						$control_container.hide().next(".validate_hint").remove();
					}
					resize_iframe_height();
					_set_apply_btn_status($(this).closest(".profile_setting"));
				});

			var bw_parm = {"title":"<#Bandwidth_Setting#> (Mb/s)", "container_id":"container_bw",
				"type_1":"text", "id_1":"bw_dl", "maxlength_1":12, "need_check_1":true,
				"type_2":"text", "id_2":"bw_ul", "maxlength_2":12, "need_check_2":true};
			Get_Component_Bandwidth_Setting(bw_parm).attr({"data-group":"wifi_settings"}).hide().appendTo($more_config_cntr);

			var access_intranet_parm = {"title":"<#Access_Intranet#>", "type":"switch", "id":"access_intranet", "openHint":"0_26", "set_value":"off"};
			Get_Component_Switch(access_intranet_parm).appendTo($more_config_cntr);
		}

		if(isSwMode("rt")){
			let use_main_subnet_parm = {"title":`<#GuestNetwork_subnet_as_main#>`, "type":"switch", "id":"use_main_subnet", "set_value":"off"};
			Get_Component_Switch(use_main_subnet_parm).appendTo($more_config_cntr).find("#" + use_main_subnet_parm.id + "").click(function(e){
				e = e || event;
				e.stopPropagation();
				const $profile_setting = $(this).closest(".profile_setting");
				$profile_setting.find("[data-container=AiMesh_List]").next(".validate_hint").remove();
				Set_AiMesh_List_CB(
					$(this).closest(".profile_setting").find("[data-container=AiMesh_List]"),
					$(this).closest(".profile_setting").find("#select_wifi_band").children(".selected").attr("value")
				);
				_set_apply_btn_status($profile_setting);
				resize_iframe_height();
			});
		}

		var $action_container = $("<div>").addClass("action_container").appendTo($content_container);
		$("<div>").addClass("btn_container apply").html("<#CTL_apply#>").appendTo($action_container);

		$content_container.find("[need_check=true]").keyup(function(){
			_set_apply_btn_status($content_container);
		});
		_set_apply_btn_status($content_container);

		return $container;
	}
	function Get_Wizard_Portal(view_mode){
		var _set_apply_btn_status = function(_obj){
			var $btn_container_apply = $(_obj).find(".action_container .btn_container.apply");
			var isBlank = validate_isBlank($(_obj));
			if(isBlank){
				$btn_container_apply.removeClass("valid_fail").addClass("valid_fail").unbind("click");
			}
			else{
				$btn_container_apply.removeClass("valid_fail").unbind("click").click(function(e){
					e = e || event;
					e.stopPropagation();
					var wizard_type = $(this).closest(".setting_content_container").attr("data-wizard-type");
					if(validate_format_Wizard_Item($(_obj), wizard_type)){
						var rc_append = "";
						var sdn_obj = get_new_sdn_profile();
						var sdn_idx = Object.keys(sdn_obj);
						var sdn_profile = sdn_obj[sdn_idx];
						selected_sdn_idx = sdn_profile.sdn_rl.idx;
						var nvramSet_obj = {"action_mode": "apply", "rc_service": "restart_wireless;restart_sdn " + selected_sdn_idx + ";"};
						var rc_append = "";
						let wifi_band = parseInt($(_obj).find("#select_wifi_band").children(".selected").attr("value"));
						sdn_profile.sdn_rl.sdn_name = wizard_type;
						sdn_profile.apg_rl.ssid = $(_obj).find("#sdn_name").val();
						var wifi_pwd = "";
						var wifi_auth = "open";
						let wifi_auth_6G = "owe";
						if(wifi_band == 4 || wifi_band == 5 || wifi_band == 6){
							wifi_auth = "openowe";
						}
						var radius_idx = sdn_profile.apg_rl.apg_idx;
						sdn_profile.apg_rl.security = "<3>" + wifi_auth + ">aes>" + wifi_pwd + ">" + radius_idx + "";
						sdn_profile.apg_rl.security += "<13>" + wifi_auth + ">aes>" + wifi_pwd + ">" + radius_idx + "";
						sdn_profile.apg_rl.security += "<16>" + wifi_auth_6G + ">aes>" + wifi_pwd + ">" + radius_idx + "";
						sdn_profile.apg_rl.security += "<96>" + wifi_auth_6G + ">aes>" + wifi_pwd + ">" + radius_idx + "";
						var dut_list = "";
						if(is_cfg_ready){
							if(wifi_band > 0){
								$(_obj).find("[data-container=AiMesh_List]").find(".node_container").each(function(){
									if($(this).find(".icon_checkbox").hasClass("clicked")){
										var node_mac = $(this).attr("data-node-mac");
										var specific_node = cfg_clientlist.filter(function(item, index, array){
											return (item.mac == node_mac);
										})[0];
										if(specific_node != undefined){
											dut_list += get_specific_dut_list(wifi_band, specific_node);
										}
									}
								});
								dut_list += get_unChecked_dut_list(_obj);
							}
							else{
								dut_list = get_dut_list(wifi_band);
							}
						}
						else{
							dut_list = get_dut_list_by_mac("3", cap_mac);
						}
						sdn_profile.apg_rl.dut_list = dut_list;
						sdn_profile.apg_rl.sched = schedule_handle_data.json_array_to_string(wizard_schedule.Get_Value());
						if($(_obj).find("#wizard_schedule").hasClass("on")){
							sdn_profile.apg_rl.timesched = ((sdn_profile.apg_rl.sched == "") ? "0" : "1");
						}
						else{
							sdn_profile.apg_rl.timesched = "0";
						}
						if($(_obj).find("#bw_enabled").hasClass("on")){
							sdn_profile.apg_rl.bw_limit = "<1>" + ($(_obj).find("#bw_ul").val())*1024 + ">" + ($(_obj).find("#bw_dl").val())*1024;
							nvramSet_obj.qos_enable = "1";
							nvramSet_obj.qos_type = "2";
							rc_append += "restart_qos;restart_firewall;";
						}
						else
							sdn_profile.apg_rl.bw_limit = "<0>>";

						sdn_profile.sdn_access_rl = [];
						if($(_obj).find("#access_intranet").hasClass("on")){
							sdn_profile.sdn_access_rl.push({"access_sdn_idx": "0", "sdn_idx": sdn_profile.sdn_rl.idx});
						}
						sdn_profile.apg_rl.ap_isolate = isSwMode("rt") ? "1" : "0";

						sdn_profile.sdn_rl.cp_idx = $(_obj).find("#select_portal_type").children(".selected").attr("value");
						if(sdn_profile.sdn_rl.cp_idx == "2"){
							if($.isEmptyObject(sdn_profile.cp_rl)){
								var idx_for_customized_ui = "";
								sdn_profile.cp_rl = JSON.parse(JSON.stringify(new cp_profile_attr()));
								var specific_cp_type_rl = cp_type_rl_json.filter(function(item, index, array){
									return (item.cp_idx == sdn_profile.sdn_rl.cp_idx);
								})[0];
								if(specific_cp_type_rl != undefined){
									if(specific_cp_type_rl.profile[0] != undefined){
										idx_for_customized_ui = specific_cp_type_rl.profile[0].idx_for_customized_ui;
									}
								}
								if(idx_for_customized_ui == ""){
									idx_for_customized_ui = $.now();
								}
								sdn_profile.cp_rl.idx_for_customized_ui = idx_for_customized_ui;
							}
							sdn_profile.cp_rl.cp_idx = sdn_profile.sdn_rl.cp_idx;
							sdn_profile.cp_rl.enable = "1";
							sdn_profile.cp_rl.conntimeout = 60*60;
							sdn_profile.cp_rl.redirecturl = "";
							sdn_profile.cp_rl.auth_type = "0";
							sdn_profile.cp_rl.term_of_service = "1";
							sdn_profile.cp_rl.NAS_ID = "";
							uploadFreeWiFi({
								"cp_idx": "2",
								"id": sdn_profile.cp_rl.idx_for_customized_ui,
								"brand_name": $(_obj).find("#FWF_brand_name").val(),
								"image": $(_obj).find("#FWF_ui_container [data-component=FWF_bg]").css('background-image').replace('url(','').replace(')','').replace(/\"/gi, ""),
								"terms_service_enabled": "1",
								"terms_service": terms_service_template,
								"auth_type": sdn_profile.cp_rl.auth_type
							});
							rc_append += "restart_chilli;restart_uam_srv;";
						}
						var sdn_all_rl_tmp = JSON.parse(JSON.stringify(sdn_all_rl_json));
						sdn_all_rl_tmp.push(JSON.parse(JSON.stringify(sdn_profile)));
						var vlan_rl_tmp = JSON.parse(JSON.stringify(vlan_rl_json));
						vlan_rl_tmp.push(JSON.parse(JSON.stringify(sdn_profile.vlan_rl)));
						var sdn_all_list = parse_JSONToStr_sdn_all_list({"sdn_all_rl":sdn_all_rl_tmp, "vlan_rl":vlan_rl_tmp});
						if(rc_append != "")
							nvramSet_obj.rc_service = nvramSet_obj.rc_service + rc_append;
						$.extend(nvramSet_obj, sdn_all_list);
						var apgX_rl = parse_apg_rl_to_apgX_rl(sdn_profile.apg_rl);
						$.extend(nvramSet_obj, apgX_rl);
						if(sdn_profile.sdn_rl.cp_idx == "2"){
							var cpX_rl = parse_cp_rl_to_cpX_rl(sdn_profile.cp_rl);
							$.extend(nvramSet_obj, cpX_rl);
						}
						if(is_QIS_flow){
							nvramSet_obj.sdn_rc_service = nvramSet_obj.rc_service;
							delete nvramSet_obj["rc_service"];
							delete nvramSet_obj["action_mode"];
							$(this).replaceWith($("<img>").attr({"width": "30px", "src": "/images/InternetScan.gif"}));
							apply.SDN_Profile(nvramSet_obj);
							return;
						}
						var showLoading_status = get_showLoading_status(nvramSet_obj.rc_service);
						if(!httpApi.app_dataHandler){
							showLoading();
							close_popup_container("all");
							if(isWLclient()){
								showLoading(showLoading_status.time);
								setTimeout(function(){
									showWlHintContainer();
								}, showLoading_status.time*1000);
								check_isAlive_and_redirect({"page": "SDN.asp", "time": showLoading_status.time});
							}
						}
						httpApi.nvramSet(nvramSet_obj, function(){
							if(isWLclient()) return;
							showLoading(showLoading_status.time);
							setTimeout(function(){
								init_sdn_all_list();
								show_sdn_profilelist();
								if(!window.matchMedia('(max-width: 575px)').matches)
									$("#profile_list_content").find("[sdn_idx=" + selected_sdn_idx + "] .item_text_container").click();
							}, showLoading_status.time*1000);
							if(!isMobile()){
								if(showLoading_status.disconnect){
									check_isAlive_and_redirect({"page": "SDN.asp", "time": showLoading_status.time});
								}
							}
						});
					}
				});
			}
		};

		var $container = $("<div>").addClass("setting_content_container");

		if(view_mode == "popup"){
			Get_Component_Popup_Profile_Title("<#GuestNetwork_Portal#>").appendTo($container)
				.find("#title_close_btn").unbind("click").click(function(e){
					e = e || event;
					e.stopPropagation();
					close_popup_container($container);
				});
		}
		else
			Get_Component_Profile_Title("<#GuestNetwork_Portal#>").appendTo($container);

		var $content_container = $("<div>").addClass("popup_content_container profile_setting").appendTo($container);

		var sdn_name_parm = {"title":"<#QIS_finish_wireless_item1#>", "type":"text", "id":"sdn_name", "need_check":true, "maxlength":32, "openHint":"0_1"};
		Get_Component_Input(sdn_name_parm).appendTo($content_container)
			.find("#" + sdn_name_parm.id + "")
			.unbind("keypress").keypress(function(){
				return validator.isString(this, event);
			})
			.unbind("keyup").keyup(function(){
				if(get_cp_type_support("2")){
					var $FWF_brand_name_items = $content_container.find("#FWF_brand_name, #FWF_Preview_Cntr [data-component=FWF_brand_name]");
					if($FWF_brand_name_items.eq(0).attr("data-user-defined") != "true"){
						if($FWF_brand_name_items.eq(0).length > 0){
							$FWF_brand_name_items.eq(0).val($(this).val());
							$FWF_brand_name_items.eq(1).html(htmlEnDeCode.htmlEncode($(this).val()));
						}
					}
				}
			})
			.attr({"oninput": "trigger_keyup($(this))"});

		var portal_options = [{"text":"<#wl_securitylevel_0#>", "value":"0"}];
		cp_type_rl_json.forEach(function(item){
			if(item.cp_idx == "4")
				return;
			portal_options.push({"text":item.cp_text, "value":item.cp_idx});
		});
		var portal_options_parm = {"title": "<#GuestNetwork_Portal_type#>", "id": "portal_type", "options": portal_options};
		Get_Component_Custom_Select(portal_options_parm).appendTo($content_container)
			.find("#select_" + portal_options_parm.id + "").children("div").click(function(e){
				if($(this).attr("data-disabled") == "true")
					return false;
				var options = $(this).attr("value");
				$(this).closest(".profile_setting").find("[data-portal-type]").hide(0, function(){
					$(this).find(".profile_setting_item").hide();
				}).filter("[data-portal-type="+options+"]").show(0, function(){
					$(this).find(".profile_setting_item").show();
				});
				resize_iframe_height();
				_set_apply_btn_status($(this).closest(".profile_setting"));
			});

		if(get_cp_type_support("2")){
			var $freewifi_cntr = $("<div>").attr({"data-portal-type":"2"}).appendTo($content_container);
			var FWF_brand_name_parm = {"title":"<#FreeWiFi_BrandName#>", "type":"text", "id":"FWF_brand_name", "need_check":true, "maxlength":40};
			Get_Component_Input(FWF_brand_name_parm).appendTo($freewifi_cntr)
				.find("#" + FWF_brand_name_parm.id + "")
				.unbind("keypress").keypress(function(){
					return validator.isString(this, event);
				})
				.unbind("keyup").keyup(function(){
					$(this).attr({"data-user-defined":"true"});
					$(this).closest(".profile_setting").find("#FWF_Preview_Cntr [data-component=FWF_brand_name]").html(htmlEnDeCode.htmlEncode($(this).val()));
				})
				.attr({"oninput": "trigger_keyup($(this))"});

			var $FWF_ui_cntr = $("<div>").attr({"id":"FWF_ui_container"}).addClass("profile_setting_item FWF_ui").appendTo($freewifi_cntr);
			Get_FWF_Preview_Container().appendTo($FWF_ui_cntr).find("[data-component=FWF_passcode]").remove();
			Get_FWF_Change_BG_Container().hide().appendTo($FWF_ui_cntr);
			if(ui_lang != "EN" && ui_lang != ""){
				$.getJSON("/ajax/freewifi_tos.json", function(data){
					if(data[ui_lang] != ""){
						terms_service_template = data[ui_lang];
						$FWF_ui_cntr.find(`[data-component=FWF_TS]`)
							.html(htmlEnDeCode.htmlEncode(terms_service_template).replace(/(?:\r\n|\r|\n)/g, '<div style=height:6px;></div>'));
					}
				});
			}
		}

		if(isSupport("fbwifi_sdn")){
			var $fb_cntr = $("<div>").attr({"data-portal-type":"3"}).appendTo($content_container);
			var fb_btn_parm = {"id":"fb_btn", "text":"<#btn_goSetting#>"};
			Get_Component_Btn(fb_btn_parm).appendTo($fb_cntr)
				.find("#" + fb_btn_parm.id + "").click(function(e){
					e = e || event;
					e.stopPropagation();
					if(!httpApi.fbwifi.isAvailable()){
						show_customize_alert("<#Facebook_WiFi_disconnected#>");
						return false;
					}
					else{
						var url = httpApi.nvramGet(["fbwifi_cp_config_url"]).fbwifi_cp_config_url;
						window.open(url, '_blank', 'toolbar=no, location=no, menubar=no, top=0, left=0, width=700, height=600, scrollbars=1');
					}
				});
		}

		var more_config_parm = {"title":"<#MoreConfig#>", "id":"more_config", "slide_target":"more_config_cntr"};
		Get_Component_Slide_Title(more_config_parm).appendTo($content_container);
		var $more_config_cntr = $("<div>").attr({"data-slide_target": "more_config_cntr"}).hide().appendTo($content_container);

		const band_options = wifi_band_options.filter((el) => {
			return (el.value != "5");
		});
		let wifi_band_options_parm = {"title": "WiFi <#Interface#>", "id": "wifi_band", "options": band_options, "set_value": "3"};
		var $sel_wifi_band = Get_Component_Custom_Select(wifi_band_options_parm).appendTo($more_config_cntr).find("#select_" + wifi_band_options_parm.id + "");
		$sel_wifi_band.children("div").click(function(e){
			var options = $(this).attr("value");
			var $profile_setting = $(this).closest(".profile_setting");
			$profile_setting.find("[data-container=AiMesh_List]").next(".validate_hint").remove();
			var $wifi_settings_objs = $profile_setting.find("[data-group=wifi_settings]");
			if(options == "0"){
				$wifi_settings_objs.hide().next(".validate_hint").remove();
				_set_apply_btn_status($profile_setting);
			}
			else{
				$wifi_settings_objs.show().filter("#wizard_aimesh").addClass("arrow_up");
				Set_AiMesh_List_CB($profile_setting.find("[data-container=AiMesh_List]"), options);
				if($wifi_settings_objs.find("#bw_enabled").length > 0){
					if($wifi_settings_objs.find("#bw_enabled").hasClass("on")){
						$wifi_settings_objs.filter("#container_bw").show();
					}
					else{
						$wifi_settings_objs.filter("#container_bw").hide();
					}
				}
				if($wifi_settings_objs.find("#wizard_schedule").length > 0){
					if($wifi_settings_objs.find("#wizard_schedule").hasClass("on"))
						$wifi_settings_objs.filter("#container_wizard_schedule").show();
					else
						$wifi_settings_objs.filter("#container_wizard_schedule").hide();
				}
				_set_apply_btn_status($profile_setting);
			}
			resize_iframe_height();
		});

		var aimesh_parm = {"title":"AiMesh", "id":"wizard_aimesh", "slide_target":"wizard_aimesh_cntr"};
		var $wizard_slide_aimesh = Get_Component_Slide_Title(aimesh_parm).attr({"data-container":"wizard_aimesh_cntr", "data-group":"wifi_settings"}).addClass("arrow_up").appendTo($more_config_cntr);
		var $wizard_aimesh_cntr = $("<div>").attr({"data-slide_target":"wizard_aimesh_cntr", "data-container":"wizard_aimesh_cntr", "data-group":"wifi_settings"}).show().appendTo($more_config_cntr);
		var $AiMesh_List = $("<div>").attr({"data-container":"AiMesh_List"}).append(Get_AiMesh_List_Container()).appendTo($wizard_aimesh_cntr);
		$AiMesh_List.find(".node_content_container").click(function(e){
			e = e || event;
			e.stopPropagation();
			_set_apply_btn_status($(this).closest(".profile_setting"));
		});

		var wizard_schedule = new schedule({
			show_timeset_viewport_callback: resize_iframe_height,
			icon_trash_callback: resize_iframe_height,
			btn_save_callback: resize_iframe_height,
			btn_cancel_callback: resize_iframe_height
		});
		var wizard_schedule_parm = {"title":str_WiFi_sche, "type":"switch", "id":"wizard_schedule", "set_value":"off"};
		Get_Component_Switch(wizard_schedule_parm).attr({"data-group":"wifi_settings"}).appendTo($more_config_cntr).find("#" + wizard_schedule_parm.id + "").click(function(e){
			e = e || event;
			e.stopPropagation();
			var $control_container = $(this).closest(".profile_setting").find("#container_wizard_schedule");
			if($(this).hasClass("on")){
				$control_container.empty().append(wizard_schedule.Get_UI());
				$control_container.show();
			}
			else{
				$control_container.hide();
			}
			resize_iframe_height();
		});
		$("<div>").addClass("profile_setting_item schedule_ui").attr({"id":"container_wizard_schedule", "data-group":"wifi_settings"}).append(wizard_schedule.Get_UI()).hide().appendTo($more_config_cntr);
		/*
		var bw_enabled_parm = {"title":"<#Bandwidth_Limiter#>", "type":"switch", "id":"bw_enabled", "set_value":"off"};
		Get_Component_Switch(bw_enabled_parm).attr({"data-group":"wifi_settings"}).appendTo($more_config_cntr)
			.find("#" + bw_enabled_parm.id + "").click(function(e){
				e = e || event;
				e.stopPropagation();
				var $control_container = $(this).closest(".profile_setting").find("#container_bw_dl, #container_bw_ul, #container_bw");
				if($(this).hasClass("on")){
					$control_container.show();
				}
				else{
					$control_container.hide().next(".validate_hint").remove();
				}
				resize_iframe_height();
				_set_apply_btn_status($(this).closest(".profile_setting"));
			});

		var bw_parm = {"title":"<#Bandwidth_Setting#> (Mb/s)", "container_id":"container_bw",
			"type_1":"text", "id_1":"bw_dl", "maxlength_1":12, "need_check_1":true,
			"type_2":"text", "id_2":"bw_ul", "maxlength_2":12, "need_check_2":true};
		Get_Component_Bandwidth_Setting(bw_parm).attr({"data-group":"wifi_settings"}).hide().appendTo($more_config_cntr);
		*/
		var access_intranet_parm = {"title":"<#Access_Intranet#>", "type":"switch", "id":"access_intranet", "openHint":"0_26", "set_value":"off"};
		Get_Component_Switch(access_intranet_parm).appendTo($more_config_cntr);

		var $action_container = $("<div>").addClass("action_container").appendTo($content_container);
		$("<div>").addClass("btn_container apply").html("<#CTL_apply#>").appendTo($action_container);

		$content_container.find("[need_check=true]").keyup(function(){
			_set_apply_btn_status($content_container);
		});
		_set_apply_btn_status($content_container);

		return $container;
	}
}
function Get_Wizard_MLO(view_mode){
	let _set_apply_btn_status = function(_obj){
		let $btn_container_apply = $(_obj).find(".action_container .btn_container.apply");
		let isBlank = validate_isBlank($(_obj));
		if(isBlank){
			$btn_container_apply.removeClass("valid_fail").addClass("valid_fail").unbind("click");
		}
		else{
			$btn_container_apply.removeClass("valid_fail").unbind("click").click(function(e){
				e = e || event;
				e.stopPropagation();
				let wizard_type = $(this).closest(".setting_content_container").attr("data-wizard-type");
				if(validate_format_Wizard_Item($(_obj), wizard_type)){
					let sdn_obj = get_new_sdn_profile();
					let sdn_idx = Object.keys(sdn_obj);
					let sdn_profile = sdn_obj[sdn_idx];
					selected_sdn_idx = sdn_profile.sdn_rl.idx;
					let nvramSet_obj = {"action_mode": "apply", "rc_service": "restart_wireless;restart_sdn " + selected_sdn_idx + ";"};
					let rc_append = "";
					sdn_profile.sdn_rl.sdn_name = wizard_type;
					sdn_profile.apg_rl.ssid = $(_obj).find("#sdn_name").val();
					let dut_list = "";
					let $sdn_pwd = $(_obj).find("#sdn_pwd");
					let radius_idx = sdn_profile.apg_rl.apg_idx;
					sdn_profile.apg_rl.security = "<127>sae>aes>" + $sdn_pwd.val() + ">" + radius_idx + "";
					sdn_profile.apg_rl.sched = schedule_handle_data.json_array_to_string(wizard_schedule.Get_Value());
					if($(_obj).find("#wizard_schedule").hasClass("on")){
						sdn_profile.apg_rl.timesched = ((sdn_profile.apg_rl.sched == "") ? "0" : "1");
					}
					else{
						sdn_profile.apg_rl.timesched = "0";
					}
					if($(_obj).find("#bw_enabled").hasClass("on")){
						sdn_profile.apg_rl.bw_limit = "<1>" + ($(_obj).find("#bw_ul").val())*1024 + ">" + ($(_obj).find("#bw_dl").val())*1024;
						nvramSet_obj.qos_enable = "1";
						nvramSet_obj.qos_type = "2";
						rc_append += "restart_qos;restart_firewall;";
					}
					else
						sdn_profile.apg_rl.bw_limit = "<0>>";

					$(_obj).find("[data-container=AiMesh_List]").find(".node_container").each(function(){
						if($(this).find(".icon_checkbox").hasClass("clicked")){
							let node_mac = $(this).attr("data-node-mac");
							let specific_node = cfg_clientlist.filter(function(item, index, array){
								return (item.mac == node_mac);
							})[0];
							if(specific_node != undefined){
								dut_list += get_mlo_dut_list(aimesh_wifi_mlo_info.cap.mlo_band_num, specific_node);
							}
						}
					});
					dut_list += get_unChecked_dut_list(_obj);
					sdn_profile.apg_rl.dut_list = dut_list;

					sdn_profile.sdn_access_rl = [];
					if($(_obj).find("#access_intranet").hasClass("on")){
						sdn_profile.sdn_access_rl.push({"access_sdn_idx": "0", "sdn_idx": sdn_profile.sdn_rl.idx});
					}
					sdn_profile.apg_rl.ap_isolate = "0";
					if($(_obj).find("#use_main_subnet").hasClass("on")){
						sdn_profile.subnet_rl = {};
						vlan_rl_json = vlan_rl_json.filter(function(item, index, array){
							return (item.vlan_idx != sdn_profile.vlan_rl.vlan_idx);
						});
						sdn_profile.vlan_rl = {};
						sdn_profile.sdn_rl.subnet_idx = "0";
						sdn_profile.sdn_rl.vlan_idx = "0";
					}
					sdn_profile.apg_rl.mlo = "2";
					const mld_enable = httpApi.nvramGet(["mld_enable"]).mld_enable;
					const mlo_enable_count = sdn_all_rl_json.filter(function(item, index, array){
						return (item.sdn_rl.sdn_name == "MLO" && item.sdn_rl.sdn_enable == "1" && item.apg_rl.mlo == "2");
					}).length;
					if(mld_enable == "1" && mlo_enable_count < mlo_fh_enable_maximum){
						sdn_profile.sdn_rl.sdn_enable = "1";
						sdn_profile.apg_rl.enable = "1";
					}
					else{
						sdn_profile.sdn_rl.sdn_enable = "0";
						sdn_profile.apg_rl.enable = "0";
					}
					let sdn_all_rl_tmp = JSON.parse(JSON.stringify(sdn_all_rl_json));
					sdn_all_rl_tmp.push(JSON.parse(JSON.stringify(sdn_profile)));
					let vlan_rl_tmp = JSON.parse(JSON.stringify(vlan_rl_json));
					vlan_rl_tmp.push(JSON.parse(JSON.stringify(sdn_profile.vlan_rl)));
					let sdn_all_list = parse_JSONToStr_sdn_all_list({"sdn_all_rl":sdn_all_rl_tmp, "vlan_rl":vlan_rl_tmp});
					if(rc_append != "")
						nvramSet_obj.rc_service = nvramSet_obj.rc_service + rc_append;
					$.extend(nvramSet_obj, sdn_all_list);
					let apgX_rl = parse_apg_rl_to_apgX_rl(sdn_profile.apg_rl);
					$.extend(nvramSet_obj, apgX_rl);
					let showLoading_status = get_showLoading_status(nvramSet_obj.rc_service);
					const parent_cntr = (typeof PAGE_CONTAINER == "string") ? PAGE_CONTAINER : "sdn";
					if(!httpApi.app_dataHandler){
						showLoading();
						close_popup_container("all");
						if(isWLclient()){
							showLoading(showLoading_status.time);
							setTimeout(function(){
								showWlHintContainer();
							}, showLoading_status.time*1000);
							check_isAlive_and_redirect({"page": "" + ((parent_cntr == "mlo") ? "MLO" : "SDN") + ".asp", "time": showLoading_status.time});
						}
					}
					httpApi.nvramSet(nvramSet_obj, function(){
						if(isWLclient()) return;
						showLoading(showLoading_status.time);
						setTimeout(function(){
							init_sdn_all_list();
							if(parent_cntr == "mlo")
								show_mlo_profilelist();
							else
								show_sdn_profilelist();
							if(!window.matchMedia('(max-width: 575px)').matches)
								$("#profile_list_content").find("[sdn_idx='" + selected_sdn_idx + "'] .item_text_container").click();
						}, showLoading_status.time*1000);
						if(!isMobile()){
							if(showLoading_status.disconnect){
								check_isAlive_and_redirect({"page": "" + ((parent_cntr == "mlo") ? "MLO" : "SDN") + ".asp", "time": showLoading_status.time});
							}
						}
					});
				}
			});
		}
	};

	let $container = $("<div>").addClass("setting_content_container");

	if(view_mode == "popup"){
		Get_Component_Popup_Profile_Title(`<#WiFi_mlo_title#>`).appendTo($container)
			.find("#title_close_btn").unbind("click").click(function(e){
				e = e || event;
				e.stopPropagation();
				close_popup_container($container);
			});
	}
	else
		Get_Component_Profile_Title(`<#WiFi_mlo_title#>`).appendTo($container);

	let $content_container = $("<div>").addClass("popup_content_container profile_setting").appendTo($container);

	let sdn_name_parm = {"title":"<#QIS_finish_wireless_item1#>", "type":"text", "id":"sdn_name", "need_check":true, "maxlength":32, "openHint":"0_1"};
	Get_Component_Input(sdn_name_parm).appendTo($content_container)
		.find("#" + sdn_name_parm.id + "")
		.unbind("keypress").keypress(function(){
			return validator.isString(this, event);
		});

	let sdn_pwd_parm = {"title":"<#QIS_finish_wireless_item2#>", "type":"password", "id":"sdn_pwd", "need_check":true, "maxlength":32, "openHint":"0_7"};
	Get_Component_Input(sdn_pwd_parm).attr({"data-group":"wifi_settings"}).appendTo($content_container)
		.find("#" + sdn_pwd_parm.id + "")
		.unbind("keyup").keyup(function(){
			chkPass($(this).val(), "rwd_vpn_pwd", $("#sdn_pwd_strength"));
			resize_iframe_height();
		})
		.unbind("blur").blur(function(){
			if($(this).val() == "")
				$("#" + sdn_pwd_parm.id + "_strength").hide();
		});
	$("<div>").attr({"id":"sdn_pwd_strength", "data-group":"wifi_settings"}).append(Get_Component_PWD_Strength_Meter()).appendTo($content_container).hide();

	let more_config_parm = {"title":"<#MoreConfig#>", "id":"more_config", "slide_target":"more_config_cntr"};
	Get_Component_Slide_Title(more_config_parm).appendTo($content_container);
	let $more_config_cntr = $("<div>").attr({"data-slide_target": "more_config_cntr"}).hide().appendTo($content_container);

	const band_options = (()=>{
		const all_band_num = aimesh_wifi_mlo_info.cap.all_band_num;
		const mlo_band_info = mlo_band_num_mapping.find(function(item){
			return (item.all_band == all_band_num);
		});
		return (mlo_band_info != undefined) ? mlo_band_info.ui_option : [];
	})();
	let wifi_band_options_parm = {"title": "WiFi <#Interface#>", "id": "wifi_band", "options": band_options};
	let $sel_wifi_band = Get_Component_Custom_Select(wifi_band_options_parm).appendTo($more_config_cntr).find("#select_" + wifi_band_options_parm.id + "");
	$sel_wifi_band.children("div").unbind("click");

	let aimesh_parm = {"title":"AiMesh", "id":"wizard_aimesh", "slide_target":"wizard_aimesh_cntr"};
	let $wizard_slide_aimesh = Get_Component_Slide_Title(aimesh_parm).attr({"data-container":"wizard_aimesh_cntr", "data-group":"wifi_settings"}).addClass("arrow_up").appendTo($more_config_cntr);
	let $wizard_aimesh_cntr = $("<div>").attr({"data-slide_target":"wizard_aimesh_cntr", "data-container":"wizard_aimesh_cntr", "data-group":"wifi_settings"}).show().appendTo($more_config_cntr);
	let $AiMesh_List = $("<div>").attr({"data-container":"AiMesh_List"}).append(Get_AiMesh_List_Container()).appendTo($wizard_aimesh_cntr);
	$AiMesh_List.find(".node_content_container").click(function(e){
		e = e || event;
		e.stopPropagation();
		_set_apply_btn_status($(this).closest(".profile_setting"));
	});

	let wizard_schedule = new schedule({
		show_timeset_viewport_callback: resize_iframe_height,
		icon_trash_callback: resize_iframe_height,
		btn_save_callback: resize_iframe_height,
		btn_cancel_callback: resize_iframe_height
	});
	let wizard_schedule_parm = {"title":str_WiFi_sche, "type":"switch", "id":"wizard_schedule", "set_value":"off"};
	Get_Component_Switch(wizard_schedule_parm).attr({"data-group":"wifi_settings"}).appendTo($more_config_cntr).find("#" + wizard_schedule_parm.id + "").click(function(e){
		e = e || event;
		e.stopPropagation();
		let $control_container = $(this).closest(".profile_setting").find("#container_wizard_schedule");
		if($(this).hasClass("on")){
			$control_container.empty().append(wizard_schedule.Get_UI());
			$control_container.show();
		}
		else{
			$control_container.hide();
		}
		resize_iframe_height();
	});
	$("<div>").addClass("profile_setting_item schedule_ui").attr({"id":"container_wizard_schedule", "data-group":"wifi_settings"}).append(wizard_schedule.Get_UI()).hide().appendTo($more_config_cntr);

	if(isSwMode("rt")){
		let bw_enabled_parm = {"title":"<#Bandwidth_Limiter#>", "type":"switch", "id":"bw_enabled", "set_value":"off"};
		Get_Component_Switch(bw_enabled_parm).attr({"data-group":"wifi_settings"}).appendTo($more_config_cntr)
			.find("#" + bw_enabled_parm.id + "").click(function(e){
				e = e || event;
				e.stopPropagation();
				let $control_container = $(this).closest(".profile_setting").find("#container_bw_dl, #container_bw_ul, #container_bw");
				if($(this).hasClass("on")){
					$control_container.show();
				}
				else{
					$control_container.hide().next(".validate_hint").remove();
				}
				resize_iframe_height();
				_set_apply_btn_status($(this).closest(".profile_setting"));
			});

		let bw_parm = {"title":"<#Bandwidth_Setting#> (Mb/s)", "container_id":"container_bw",
			"type_1":"text", "id_1":"bw_dl", "maxlength_1":12, "need_check_1":true,
			"type_2":"text", "id_2":"bw_ul", "maxlength_2":12, "need_check_2":true};
		Get_Component_Bandwidth_Setting(bw_parm).attr({"data-group":"wifi_settings"}).hide().appendTo($more_config_cntr);

		let access_intranet_parm = {"title":"<#Access_Intranet#>", "type":"switch", "id":"access_intranet", "openHint":"0_26", "set_value":"off"};
		Get_Component_Switch(access_intranet_parm).appendTo($more_config_cntr);

		let use_main_subnet_parm = {"title":`<#GuestNetwork_subnet_as_main#>`, "type":"switch", "id":"use_main_subnet", "set_value":"off"};
		Get_Component_Switch(use_main_subnet_parm).appendTo($more_config_cntr);
	}

	let $action_container = $("<div>").addClass("action_container").appendTo($content_container);
	$("<div>").addClass("btn_container apply").html("<#CTL_apply#>").appendTo($action_container);

	$content_container.find("[need_check=true]").keyup(function(){
		_set_apply_btn_status($content_container);
	});
	_set_apply_btn_status($content_container);

	return $container;
}
function Get_Component_Full_Setting(view_mode){
	var $container = $("<div>").addClass("setting_content_container");

	if(view_mode == "popup"){
		var $popup_title_cntr = $("<div>").addClass("popup_title_container edit_mode").appendTo($container);
		var $title_cntr = $("<div>").addClass("title_container").appendTo($popup_title_cntr);
		$("<div>").addClass("title").attr({"data-container":"profile_title"}).html("<#DSL_profile#>").appendTo($title_cntr);
		$("<div>").attr({"id":"title_close_btn"}).addClass("close_btn").unbind("click").click(function(e){
			e = e || event;
			e.stopPropagation();
			close_popup_container($container);
		}).appendTo($popup_title_cntr);
	}
	else{
		var $title_cntr = $("<div>").addClass("profile_setting_title edit_mode").appendTo($container);
		$("<div>").addClass("title").attr({"data-container":"profile_title"}).html("<#DSL_profile#>").appendTo($title_cntr);
		var $profile_title_action = $("<div>").attr({"data-container":"profile_title_action"}).addClass("profile_title_action").appendTo($title_cntr);
		$("<div>").attr({"id":"title_del_btn"}).addClass("del_btn").appendTo($profile_title_action);
	}

	var $content_container = $("<div>").addClass("popup_content_container profile_setting").attr({"data-profile-mode":"edit"}).appendTo($container);

	var $subtab_cntr = Get_Setting_SubTab_Container().appendTo($content_container);

	var $subtab_container_net = $("<div>").attr({"subtab_container": "net"}).appendTo($content_container);
	var category_general = $("<div>").addClass("category_container").attr({"data-category-cntr":"general"}).appendTo($subtab_container_net);
	var general_parm = {"title":"<#menu5_1_1#>", "id":"category_general", "slide_target":"category_general_cntr"};
	Get_Component_Category_Slide_Title(general_parm).appendTo(category_general)
		.find(".title_cntr").unbind("click").click(function(e){
			e = e || event;
			e.stopPropagation();
			handle_category_container($(this), general_parm);
		});
	Get_Net_General_Container().hide().appendTo(category_general);

	var category_adv = $("<div>").addClass("category_container").attr({"data-category-cntr":"adv"}).appendTo($subtab_container_net);
	var adv_parm = {"title":"<#menu5#>", "id":"category_adv", "slide_target":"category_adv_cntr"};
	Get_Component_Category_Slide_Title(adv_parm).appendTo(category_adv)
		.find(".title_cntr").unbind("click").click(function(e){
			e = e || event;
			e.stopPropagation();
			handle_category_container($(this), adv_parm);
		});
	Get_Net_Adv_Container().hide().appendTo(category_adv);

	var category_aimesh = $("<div>").addClass("category_container").attr({"data-category-cntr":"aimesh","data-group":"wifi_settings"}).appendTo($subtab_container_net);
	var aimesh_parm = {"title":"AiMesh", "id":"category_aimesh", "slide_target":"category_aimesh_cntr"};/* untranslated */
	Get_Component_Category_Slide_Title(aimesh_parm).appendTo(category_aimesh)
		.find(".title_cntr").unbind("click").click(function(e){
			e = e || event;
			e.stopPropagation();
			handle_category_container($(this), aimesh_parm);
		});
	Get_Net_AiMesh_Container().hide().appendTo(category_aimesh);

	var $action_container = $("<div>").attr({"data-container":"action_container"}).addClass("action_container").appendTo($subtab_container_net);
	$("<div>").attr({"data-component":"del_btn"}).addClass("btn_container delete mobile").appendTo($action_container).html("<#CTL_del#>");
	$("<div>").attr({"data-component":"apply_btn"}).addClass("btn_container apply").appendTo($action_container).html("<#CTL_apply#>");

	//var $subtab_container_sec = $("<div>").attr({"subtab_container": "sec"}).appendTo($content_container);

	if(isSwMode("rt")){
		var $subtab_container_vpn = $("<div>").attr({"subtab_container": "vpn"}).appendTo($content_container);
		var vpn_enabled_parm = {"title":"VPN", "type":"switch", "id":"vpn_enabled"};
		Get_Component_Switch(vpn_enabled_parm).appendTo($subtab_container_vpn)
			.find("#" + vpn_enabled_parm.id + "").click(function(e){
				e = e || event;
				e.stopPropagation();
				var $control_container = $(this).closest(".profile_setting").find("[data-container=VPN_Profiles]");
				if($(this).hasClass("on")){
					$control_container.show();
				}
				else{
					$control_container.hide();
				}
				resize_iframe_height();
			});
		Get_Component_VPN_Profiles().appendTo($subtab_container_vpn);
	}

	var $subtab_container_client = $("<div>").attr({"subtab_container": "client"}).appendTo($content_container);
	var $client_info_cntr =  $("<div>").attr({"data-container":"client_info_container"}).addClass("client_info_container").appendTo($subtab_container_client);
	var client_tab_options = [{"text":"<#Clientlist_All_List#>","option_id":"all"}, {"text":"<#tm_wired#>","option_id":"wired"}, {"text":"<#tm_wireless#>","option_id":"wireless"}];
	var client_tab_parm = {"title":"<#ConnectedClient#> : 0 <#Clientlist_Online#>", "options": client_tab_options, "container_id":"client_tab"};
	Get_Component_Switch_Text(client_tab_parm).appendTo($client_info_cntr)
		.find(".switch_text_container > .switch_text_item")
		.click(function(e){
			e = e || event;
			e.stopPropagation();
			var sdn_idx = $client_info_cntr.closest(".setting_content_container").attr("sdn_idx");
			var option_id = $(this).attr("data-option-id");
			var $clientlist_container = $client_info_cntr.find("[data-container=clientlist_container]").empty();
			var $client_num = $client_info_cntr.find("#client_tab > .title");
			var sdn_clientlist_data = [];
			var get_clientlist = httpApi.hookGet("get_clientlist");
			$.each(get_clientlist, function(index, client){
				if(client.sdn_idx != undefined && client.sdn_idx == sdn_idx){
					if(client.isOnline == "1"){
						switch(option_id){
							case "all":
								sdn_clientlist_data.push(client);
								break;
							case "wired":
								if(client.isWL == "0"){
									sdn_clientlist_data.push(client);
								}
								break;
							case "wireless":
								if(client.isWL >= "1"){
									sdn_clientlist_data.push(client);
								}
								break;
						}
					}
				}
			});
			$client_num.html("<#ConnectedClient#> : " + sdn_clientlist_data.length + " <#Clientlist_Online#>");
			Get_Component_ClientList(sdn_clientlist_data).appendTo($clientlist_container);
			resize_iframe_height();
		});
	$("<div>").attr({"data-container":"clientlist_container"}).addClass("clientlist_container").appendTo($client_info_cntr);

	if(isSwMode("rt")){
		var $subtab_container_portal = $("<div>").attr({"subtab_container": "portal"}).appendTo($content_container);
		var portal_options = [{"text":"<#wl_securitylevel_0#>", "value":"0"}];
		cp_type_rl_json.forEach(function(item){
			portal_options.push({"text":item.cp_text, "value":item.cp_idx});
		});
		var portal_options_parm = {"title": "<#GuestNetwork_Portal_type#>", "id": "portal_type", "options": portal_options};
		Get_Component_Custom_Select(portal_options_parm).appendTo($subtab_container_portal)
			.find("#select_" + portal_options_parm.id + "").children("div").click(function(e){
				if($(this).attr("data-disabled") == "true")
					return false;
				var options = $(this).attr("value");
				$(this).closest(".profile_setting").find("[data-portal-type]").hide(0, function(){
					$(this).find(".profile_setting_item").hide();
				}).filter("[data-portal-type="+options+"]").show(0, function(){
					$(this).find(".profile_setting_item").show();
				});
				resize_iframe_height();
				set_apply_btn_status($(this).closest(".profile_setting"));
			});

		if(get_cp_type_support("2")){
			var $freewifi_cntr = $("<div>").attr({"data-portal-type":"2"}).appendTo($subtab_container_portal);
			var FWF_brand_name_parm = {"title":"<#FreeWiFi_BrandName#>", "type":"text", "id":"FWF_brand_name", "need_check":true, "maxlength":40};
				Get_Component_Input(FWF_brand_name_parm).appendTo($freewifi_cntr)
					.find("#" + FWF_brand_name_parm.id + "")
					.unbind("keypress").keypress(function(){
						return validator.isString(this, event);
					})
					.unbind("keyup").keyup(function(){
						$(this).closest(".profile_setting").find("#FWF_Preview_Cntr [data-component=FWF_brand_name]").html(htmlEnDeCode.htmlEncode($(this).val()));
					})
					.attr({"oninput": "trigger_keyup($(this))"});

			var $FWF_ui_cntr = $("<div>").attr({"id":"FWF_ui_container"}).addClass("profile_setting_item FWF_ui").appendTo($freewifi_cntr);
			Get_FWF_Preview_Container().appendTo($FWF_ui_cntr);
			Get_FWF_Change_BG_Container().hide().appendTo($FWF_ui_cntr);

			var FWF_conntimeout_parm = {"title":"<#FreeWiFi_timeout#> (<#Minute#>)", "type":"text", "id":"FWF_conntimeout", "need_check":true, "maxlength":3, "set_value":"60", "openHint":"31_3"};
			Get_Component_Input(FWF_conntimeout_parm).appendTo($freewifi_cntr)
				.find("#" + FWF_conntimeout_parm.id + "")
				.unbind("keypress").keypress(function(){
					return validator.isNumber(this,event);
				});
			var FWF_redirecturl_parm = {"title":"<#FreeWiFi_LandingPage#> (<#FreeWiFi_RedirectPage#>)", "type":"text", "id":"FWF_redirecturl", "maxlength":256};
			Get_Component_Input(FWF_redirecturl_parm).appendTo($freewifi_cntr);
			var FWF_passcode_enabled_parm = {"title":"<#FreeWiFi_Add_Passcode#>", "type":"switch", "id":"FWF_passcode_enabled", "set_value":"off"};
			Get_Component_Switch(FWF_passcode_enabled_parm).appendTo($freewifi_cntr)
				.find("#" + FWF_passcode_enabled_parm.id + "").click(function(e){
					e = e || event;
					e.stopPropagation();
					var $control_container = $(this).closest(".profile_setting").find("#container_FWF_passcode, #FWF_Preview_Cntr [data-component=FWF_passcode]");
					if($(this).hasClass("on")){
						$control_container.show();
					}
					else{
						$control_container.hide().next(".validate_hint").remove();
					}
					resize_iframe_height();
					set_apply_btn_status($(this).closest(".profile_setting"));
				});
			var FWF_passcode_parm = {"title":"<#FreeWiFi_Passcode#>", "type":"text", "id":"FWF_passcode", "need_check":true, "maxlength":64, "container_id":"container_FWF_passcode"};
			Get_Component_Input(FWF_passcode_parm).hide().appendTo($freewifi_cntr);

			var FWF_terms_service_enabled_parm = {"title":"<#FreeWiFi_OptionHint#>", "type":"switch", "id":"FWF_terms_service_enabled", "set_value":"on"};
			Get_Component_Switch(FWF_terms_service_enabled_parm).appendTo($freewifi_cntr)
				.find("#" + FWF_terms_service_enabled_parm.id + "").click(function(e){
					e = e || event;
					e.stopPropagation();
					var $control_container = $(this).closest(".profile_setting").find("#container_FWF_TS, #FWF_Preview_Cntr [data-group=FWF_TS]");
					if($(this).hasClass("on")){
						$control_container.show();
						$(this).closest(".profile_setting").find(".FWF_portal_cntr").removeClass("no_terms_service");
					}
					else{
						$control_container.hide().next(".validate_hint").remove();
						$(this).closest(".profile_setting").find(".FWF_portal_cntr").addClass("no_terms_service");
					}
					resize_iframe_height();
					set_apply_btn_status($(this).closest(".profile_setting"));
				});
			var parm_terms_service = terms_service_template;
			var FWF_terms_service_parm = {
				"title":"<#Terms_of_Service#>", "id":"FWF_terms_service", "rows":"8", "cols":"55", "need_check":true, "maxlength":"5120", "set_value":parm_terms_service,
				"container_id":"container_FWF_TS"
			};
			Get_Component_Textarea(FWF_terms_service_parm).appendTo($freewifi_cntr)
				.find("#" + FWF_terms_service_parm.id + "")
				.unbind("keypress").keypress(function(){
					return validator.isString(this, event);
				})
				.unbind("keyup").keyup(function(){
					$(this).closest(".profile_setting").find("#FWF_Preview_Cntr [data-component=FWF_TS]").html(htmlEnDeCode.htmlEncode($(this).val()).replace(/(?:\r\n|\r|\n)/g, '<div style=height:6px;></div>'));
				})
				.attr({"oninput": "trigger_keyup($(this))"});
		}
		if(get_cp_type_support("4")){
			var $message_board_cntr = $("<div>").attr({"data-portal-type":"4"}).appendTo($subtab_container_portal);
			var MB_desc_parm = {"title":"<#Description#>", "type":"text", "id":"MB_desc", "need_check":true, "maxlength":80};
				Get_Component_Input(MB_desc_parm).appendTo($message_board_cntr)
					.find("#" + MB_desc_parm.id + "")
					.unbind("keypress").keypress(function(){
						return validator.isString(this, event);
					})
					.unbind("keyup").keyup(function(){
						$(this).closest(".profile_setting").find("#MB_Preview_Cntr [data-component=MB_desc]").html(htmlEnDeCode.htmlEncode($(this).val()));
					})
					.attr({"oninput": "trigger_keyup($(this))"});

			var $MB_ui_cntr = $("<div>").attr({"id":"MB_ui_container"}).addClass("profile_setting_item MB_ui").appendTo($message_board_cntr);
			Get_MB_Preview_Container().appendTo($MB_ui_cntr);
			Get_MB_Change_BG_Container().hide().appendTo($MB_ui_cntr);
		}

		if(isSupport("fbwifi_sdn")){
			var $fb_cntr = $("<div>").attr({"data-portal-type":"3"}).appendTo($subtab_container_portal);
			var fb_btn_parm = {"id":"fb_btn", "text":"<#btn_goSetting#>"};
			Get_Component_Btn(fb_btn_parm).appendTo($fb_cntr)
				.find("#" + fb_btn_parm.id + "").click(function(e){
					e = e || event;
					e.stopPropagation();

					if(!httpApi.fbwifi.isAvailable()){
						return false;
					}
					else{
						var url = httpApi.nvramGet(["fbwifi_cp_config_url"]).fbwifi_cp_config_url;
						window.open(url, '_blank', 'toolbar=no, location=no, menubar=no, top=0, left=0, width=700, height=600, scrollbars=1');
					}
				});
		}
	}
	var $action_container = $("<div>").attr({"id":"action_container"}).addClass("action_container").appendTo($content_container);
	var $btn_container_delete_mobile = $("<div>").attr({"id":"del_btn"}).addClass("btn_container delete mobile").appendTo($action_container).html("<#CTL_del#>");
	var $btn_container_apply = $("<div>").attr({"id":"apply_btn"}).addClass("btn_container apply").appendTo($action_container).html("<#CTL_apply#>");

	$content_container.find("[need_check=true]").keyup(function(){
		set_apply_btn_status($content_container);
	});

	$subtab_cntr.find("[subtab=net]").click();
	$subtab_container_net.find("[data-category-cntr=general] .title_cntr").click();
	$action_container.hide();

	return $container;

	function Get_Setting_SubTab_Container(){
		var $container = $("<div>").addClass("setting_subtab_container");
		$("<div>").addClass("subtab_item").attr({"subtab":"net"}).html("<#Network#>").unbind("click").click(function(e){
			e = e || event;
			e.stopPropagation();
			$(this).closest(".Setting_SubTab_Container").find(".subtab_item").removeClass("selected");
			$(this).addClass("selected");
			var subtab = $(this).attr("subtab");
			$(this).closest(".profile_setting").find("[subtab_container]").hide();
			$(this).closest(".profile_setting").find("[subtab_container='" + subtab + "']").show();
			$(this).closest(".profile_setting").find("#action_container").hide();
			resize_iframe_height();
		}).appendTo($container);
		/*
		$("<div>").addClass("subtab_item").attr({"subtab":"sec"}).html("Security").unbind("click").click(function(e){
			e = e || event;
			e.stopPropagation();
			$(this).closest(".Setting_SubTab_Container").find(".subtab_item").removeClass("selected");
			$(this).addClass("selected");
			var subtab = $(this).attr("subtab");
			$(this).closest(".profile_setting").find("[subtab_container]").hide();
			$(this).closest(".profile_setting").find("[subtab_container='" + subtab + "']").show();
			$(this).closest(".profile_setting").find("#action_container").show();
			resize_iframe_height();
		}).appendTo($container);
		*/
		if(isSwMode("rt")){
			$("<div>").addClass("subtab_item").attr({"subtab":"vpn"}).html("VPN").unbind("click").click(function(e){
				e = e || event;
				e.stopPropagation();
				$(this).closest(".Setting_SubTab_Container").find(".subtab_item").removeClass("selected");
				$(this).addClass("selected");
				var subtab = $(this).attr("subtab");
				$(this).closest(".profile_setting").find("[subtab_container]").hide();
				$(this).closest(".profile_setting").find("[subtab_container='" + subtab + "']").show();
				$(this).closest(".profile_setting").find("#action_container").show();
				resize_iframe_height();
			}).appendTo($container);
		}
		$("<div>").addClass("subtab_item").attr({"subtab":"client"}).html("<#Full_Clients#>").unbind("click").click(function(e){
			e = e || event;
			e.stopPropagation();
			$(this).closest(".Setting_SubTab_Container").find(".subtab_item").removeClass("selected");
			$(this).addClass("selected");
			var subtab = $(this).attr("subtab");
			$(this).closest(".profile_setting").find("[subtab_container]").hide();
			$(this).closest(".profile_setting").find("[subtab_container='" + subtab + "']").show();
			$(this).closest(".profile_setting").find("#action_container").hide();
			resize_iframe_height();
		}).appendTo($container);
		if(isSwMode("rt")){
			if(cp_type_rl_json.length > 0){
				$("<div>").addClass("subtab_item").attr({"subtab":"portal"}).html(`Portal`).unbind("click").click(function(e){
					e = e || event;
					e.stopPropagation();
					$(this).closest(".Setting_SubTab_Container").find(".subtab_item").removeClass("selected");
					$(this).addClass("selected");
					var subtab = $(this).attr("subtab");
					$(this).closest(".profile_setting").find("[subtab_container]").hide();
					$(this).closest(".profile_setting").find("[subtab_container='" + subtab + "']").show();
					$(this).closest(".profile_setting").find("#action_container").show();
					resize_iframe_height();
				}).appendTo($container);
			}
		}

		return $container;
	}
	function Get_Net_General_Container(){
		var $net_mode_container = $("<div>").attr({"data-slide_target": "category_general_cntr"});

		var sdn_name_parm = {"title":"<#QIS_finish_wireless_item1#>", "type":"text", "id":"sdn_name", "need_check":true, "maxlength":32, "openHint":"0_1"};
		var $sdn_name_cntr = Get_Component_Input(sdn_name_parm).appendTo($net_mode_container)
		$sdn_name_cntr.find("#" + sdn_name_parm.id + "")
			.unbind("keypress").keypress(function(){
				return validator.isString(this, event);
			});
		$sdn_name_cntr.addClass("qrcode_item").find(".input_container").append($("<div>").attr({"id":"qrcode_btn"}).addClass("qrcode_btn"));

		let wifi_band_options_parm = {"title": "WiFi <#Interface#>", "id": "wifi_band", "options": wifi_band_options, "set_value": "3"};
		var $sel_wifi_band = Get_Component_Custom_Select(wifi_band_options_parm).appendTo($net_mode_container).find("#select_" + wifi_band_options_parm.id + "");
		$sel_wifi_band.children("div").click(function(e){
			var options = $(this).attr("value");
			var $profile_setting = $(this).closest(".profile_setting");
			$profile_setting.find("[data-container=AiMesh_List]").next(".validate_hint").remove();
			var $wifi_settings_objs = $profile_setting.find("[data-group=wifi_settings]");
			if(options == "0"){
				$wifi_settings_objs.hide().next(".validate_hint").remove();
				set_apply_btn_status($profile_setting);
			}
			else{
				$wifi_settings_objs.show().filter("#sdn_pwd_strength").hide();
				const sdn_type = $(this).closest(".setting_content_container").attr("sdn_type");
				if(sdn_type == "MLO"){
					Set_AiMesh_List_CB_MLO($profile_setting.find("[data-container=AiMesh_List]"));
				}
				else{
					Set_AiMesh_List_CB($profile_setting.find("[data-container=AiMesh_List]"), options);
				}
				if($wifi_settings_objs.find(".switch_text_container").length > 0){
					$wifi_settings_objs.find(".switch_text_container .switch_text_item.selected").click();
					if($wifi_settings_objs.filter("#security_guest").find("[data-option-id=open]").length > 0){
						$wifi_settings_objs.filter("#security_guest")
							.find("[data-option-id=open]")
							.html(()=>{return ((options == "4" || options == "5" || options == "6") ? `<#Wireless_Encryption_OWE#>` : `Open System`);});
					}
				}
				else{
					if($wifi_settings_objs.filter("[data-wifi-auth]").length > 0){
						$wifi_settings_objs.filter("[data-wifi-auth]").hide()
							.filter((()=>{
								return get_elem_attr_wifi_auth({"wifi_band_option_value":options, "sdn_type":sdn_type});
							})()).show();
					}
				}
				if($wifi_settings_objs.find("#bw_enabled").length > 0){
					if($wifi_settings_objs.find("#bw_enabled").hasClass("on")){
						$wifi_settings_objs.filter("#container_bw").show();
					}
					else{
						$wifi_settings_objs.filter("#container_bw").hide();
					}
				}
				if($wifi_settings_objs.find("#schedule").length > 0){
					if($wifi_settings_objs.find("#schedule").hasClass("on"))
						$wifi_settings_objs.filter("#container_schedule").show();
					else
						$wifi_settings_objs.filter("#container_schedule").hide();
				}

				set_apply_btn_status($profile_setting);
			}
			resize_iframe_height();
		});

		var security_options = [{"text":"Open System","option_id":"open"}, {"text":"<#HSDPAConfig_Password_itemname#>","option_id":"pwd"}];
		var security_parm = {"title":"<#Security#>", "options": security_options, "container_id":"security_guest"};
		Get_Component_Switch_Text(security_parm).attr({"data-group":"wifi_settings"}).appendTo($net_mode_container)
			.find(".switch_text_container > .switch_text_item")
			.click(function(e){
				e = e || event;
				e.stopPropagation();
				var option_id = $(this).attr("data-option-id");
				$(this).closest(".profile_setting").find("[data-sec-option-id]").hide();
				$(this).closest(".profile_setting").find("[data-sec-option-id=" + option_id + "]").show();
				if(option_id == "pwd"){
					$(this).closest(".profile_setting").find("#sdn_pwd_strength").hide();
					let $profile_setting = $(this).closest(".profile_setting");
					const wifi_band = $profile_setting.find("#select_wifi_band").children(".selected").attr("value");
					$profile_setting.find("[data-wifi-auth]").hide()
						.filter((()=>{
							const sdn_type = $(this).closest(".setting_content_container").attr("sdn_type");
							return get_elem_attr_wifi_auth({"wifi_band_option_value":wifi_band, "sdn_type":sdn_type});
						})()).show();
				}
				resize_iframe_height();
				set_apply_btn_status($net_mode_container.closest(".profile_setting"));
			});

		var security_radius_options = [{"text":"<#HSDPAConfig_Password_itemname#>","option_id":"pwd"}, {"text":"<#menu5_1_5#>","option_id":"radius"}];
		var security_radius_parm = {"title":"<#Security#>", "options": security_radius_options, "container_id":"security_employee"};
		Get_Component_Switch_Text(security_radius_parm).attr({"data-group":"wifi_settings"}).appendTo($net_mode_container)
			.find(".switch_text_container > .switch_text_item")
			.click(function(e){
				e = e || event;
				e.stopPropagation();
				var option_id = $(this).attr("data-option-id");
				$(this).closest(".profile_setting").find("[data-sec-option-id]").hide().next(".validate_hint").remove();
				$(this).closest(".profile_setting").find("[data-sec-option-id=" + option_id + "]").show();
				if(option_id == "pwd"){
					$(this).closest(".profile_setting").find("#sdn_pwd_strength").hide();
				}

				let $profile_setting = $(this).closest(".profile_setting");
				const wifi_band = $profile_setting.find("#select_wifi_band").children(".selected").attr("value");
				$profile_setting.find("[data-sec-option-id=" + option_id + "][data-wifi-auth]").hide()
					.filter((()=>{
						const sdn_type = $(this).closest(".setting_content_container").attr("sdn_type");
						return get_elem_attr_wifi_auth({"wifi_band_option_value":wifi_band, "sdn_type":sdn_type});
					})()).show();

				resize_iframe_height();
				set_apply_btn_status($net_mode_container.closest(".profile_setting"));
				if(isSupport("wpa3-e")){
					Set_AiMesh_List_CB(
						$(this).closest(".profile_setting").find("[data-container=AiMesh_List]"),
						$(this).closest(".profile_setting").find("#select_wifi_band").children(".selected").attr("value")
					);
				}
			});

		let wifi_auth_options = [
			{"text":"WPA2-Personal","value":"psk2"},
			{"text":"WPA/WPA2-Personal","value":"pskpsk2"},
			{"text":"WPA2/WPA3-Personal","value":"psk2sae"},
			{"text":`WPA3-Personal`,"value":"sae"}
		];
		var wifi_auth_options_parm = {"title": "<#WLANConfig11b_AuthenticationMethod_itemname#>", "id": "wifi_auth", "options": wifi_auth_options, "set_value": "psk2"};
		Get_Component_Custom_Select(wifi_auth_options_parm).attr({"data-sec-option-id":"pwd","data-group":"wifi_settings","data-wifi-auth":"default"}).appendTo($net_mode_container);

		let wifi_auth_options_6G = [{"text":`WPA3-Personal`,"value":"sae"}];
		let wifi_auth_options_6G_parm = {"title": `<#WLANConfig11b_AuthenticationMethod_itemname#>`, "id": "wifi_auth_6G", "options": wifi_auth_options_6G, "set_value": "sae"};
		Get_Component_Custom_Select(wifi_auth_options_6G_parm).attr({"data-sec-option-id":"pwd","data-group":"wifi_settings","data-wifi-auth":"6G"}).appendTo($net_mode_container);

		var wifi_auth_radius_options = [{"text":"WPA2-Enterprise","value":"wpa2"},{"text":"WPA/WPA2-Enterprise","value":"wpawpa2"}];
		if(isSupport("wpa3-e")){
			wifi_auth_radius_options.push({"text":"WPA3-Enterprise","value":"wpa3"});
			wifi_auth_radius_options.push({"text":"WPA2/WPA3-Enterprise","value":"wpa2wpa3"});
			wifi_auth_radius_options.push({"text":"WPA3-Enterprise 192-bit","value":"suite-b"});
		}
		var wifi_auth_radius_options_parm = {"title": "<#WLANConfig11b_AuthenticationMethod_itemname#>", "id": "wifi_auth_radius", "options": wifi_auth_radius_options, "set_value": "wpa2"};
		var $sel_wifi_auth_radius = Get_Component_Custom_Select(wifi_auth_radius_options_parm).attr({"data-sec-option-id":"radius","data-group":"wifi_settings","data-wifi-auth":"default"})
			.appendTo($net_mode_container).find("#select_" + wifi_auth_radius_options_parm.id + "");
		if(isSupport("wpa3-e")){
			$sel_wifi_auth_radius.children("div").click(function(e){
				Set_AiMesh_List_CB(
					$(this).closest(".profile_setting").find("[data-container=AiMesh_List]"),
					$(this).closest(".profile_setting").find("#select_wifi_band").children(".selected").attr("value")
				);
			});
		}

		let wifi_auth_radius_options_6G = [];
		if(isSupport("wpa3-e")){
			wifi_auth_radius_options_6G.push({"text":"WPA3-Enterprise","value":"wpa3"});
			wifi_auth_radius_options_6G.push({"text":"WPA3-Enterprise 192-bit","value":"suite-b"});
		}
		let wifi_auth_radius_options_6G_parm = {"title": "<#WLANConfig11b_AuthenticationMethod_itemname#>", "id": "wifi_auth_radius_6G", "options": wifi_auth_radius_options_6G, "set_value": "wpa3"};
		let $sel_wifi_auth_radius_6G = Get_Component_Custom_Select(wifi_auth_radius_options_6G_parm).attr({"data-sec-option-id":"radius","data-group":"wifi_settings","data-wifi-auth":"6G"})
			.appendTo($net_mode_container).find("#select_" + wifi_auth_radius_options_6G_parm.id + "");
		if(isSupport("wpa3-e")){
			$sel_wifi_auth_radius_6G.children("div").click(function(e){
				Set_AiMesh_List_CB(
					$(this).closest(".profile_setting").find("[data-container=AiMesh_List]"),
					$(this).closest(".profile_setting").find("#select_wifi_band").children(".selected").attr("value")
				);
			});
		}

		let sdn_pwd_parm = {"title":`<#QIS_finish_wireless_item2#>`, "type":"password", "id":"sdn_pwd", "need_check":true, "maxlength":64, "openHint":"0_7"};
		Get_Component_Input(sdn_pwd_parm).attr({"data-sec-option-id":"pwd","data-group":"wifi_settings"}).appendTo($net_mode_container)
			.find("#" + sdn_pwd_parm.id + "")
			.unbind("keyup").keyup(function(){
				chkPass($(this).val(), "rwd_vpn_pwd", $("#sdn_pwd_strength"));
				resize_iframe_height();
			})
			.unbind("blur").blur(function(){
				if($(this).val() == ""){
					$("#" + sdn_pwd_parm.id + "_strength").hide();
				}
			});
		$("<div>").attr({"id":"sdn_pwd_strength", "data-sec-option-id":"pwd", "data-group":"wifi_settings"}).append(Get_Component_PWD_Strength_Meter()).appendTo($net_mode_container).hide();

		var radius_ipaddr_parm = {"title":"<#WLANAuthentication11a_ExAuthDBIPAddr_itemname#>", "type":"text", "id":"radius_ipaddr", "need_check":true, "maxlength":15, "openHint":"2_1"};
		Get_Component_Input(radius_ipaddr_parm).attr({"data-sec-option-id":"radius","data-group":"wifi_settings"}).appendTo($net_mode_container)
			.find("#" + radius_ipaddr_parm.id + "")
			.unbind("keypress").keypress(function(){
				return validator.isIPAddr(this, event);
			});

		var radius_port_parm = {"title":"<#WLANAuthentication11a_ExAuthDBPortNumber_itemname#>", "type":"text", "id":"radius_port", "need_check":true, "maxlength":5, "openHint":"2_2"};
		Get_Component_Input(radius_port_parm).attr({"data-sec-option-id":"radius","data-group":"wifi_settings"}).appendTo($net_mode_container)
			.find("#" + radius_port_parm.id + "")
			.unbind("keypress").keypress(function(){
				return validator.isNumber(this,event);
			});

		var radius_key_parm = {"title":"<#WLANAuthentication11a_ExAuthDBPassword_itemname#>", "type":"password", "id":"radius_key", "need_check":true, "maxlength":64, "openHint":"2_3"};
		Get_Component_Input(radius_key_parm).attr({"data-sec-option-id":"radius","data-group":"wifi_settings"}).appendTo($net_mode_container);

		if(!isMobile()){
			var qrcode_btn_parm = {"title":`<#Print_QR_Code#>`, "id":"qrcode_print_btn"};
			Get_Component_Print_Btn(qrcode_btn_parm).appendTo($net_mode_container);
		}
		var bw_enabled_parm = {"title":"<#Bandwidth_Limiter#>", "type":"switch", "id":"bw_enabled"};
		Get_Component_Switch(bw_enabled_parm).attr({"data-group":"wifi_settings"}).appendTo($net_mode_container)
			.find("#" + bw_enabled_parm.id + "").click(function(e){
				e = e || event;
				e.stopPropagation();
				var $control_container = $(this).closest(".profile_setting").find("#container_bw_dl, #container_bw_ul, #container_bw");
				if($(this).hasClass("on")){
					$control_container.show();
				}
				else{
					$control_container.hide().next(".validate_hint").remove();
				}
				resize_iframe_height();
				set_apply_btn_status($net_mode_container.closest(".profile_setting"));
			});

		var bw_parm = {"title":"<#Bandwidth_Setting#> (Mb/s)", "container_id":"container_bw",
			"type_1":"text", "id_1":"bw_dl", "maxlength_1":12, "need_check_1":true,
			"type_2":"text", "id_2":"bw_ul", "maxlength_2":12, "need_check_2":true};
		Get_Component_Bandwidth_Setting(bw_parm).attr({"data-group":"wifi_settings"}).appendTo($net_mode_container);

		var access_intranet_parm = {"title":"<#Access_Intranet#>", "type":"switch", "id":"access_intranet", "openHint":"0_26"};
		Get_Component_Switch(access_intranet_parm).appendTo($net_mode_container);

		var schedule_parm = {"title":str_WiFi_sche, "type":"switch", "id":"schedule"};
		Get_Component_Switch(schedule_parm).attr({"data-group":"wifi_settings"}).appendTo($net_mode_container).find("#" + schedule_parm.id + "").click(function(e){
			e = e || event;
			e.stopPropagation();
			var $control_container = $(this).closest(".profile_setting").find("#container_schedule");
			if($(this).hasClass("on")){
				$control_container.empty().append(sdn_schedule.Get_UI());
				$control_container.show();
			}
			else{
				$control_container.hide();
			}
			resize_iframe_height();
		});

		$("<div>").addClass("profile_setting_item schedule_ui").attr({"id":"container_schedule","data-group":"wifi_settings"}).hide().appendTo($net_mode_container);

		 return $net_mode_container;
	}
	function Get_Net_Adv_Container(){
		var $net_mode_container = $("<div>").attr({"data-slide_target": "category_adv_cntr"});

		if(isSwMode("rt")){
			var dhcp_enable_parm = {"title":"<#LANHostConfig_DHCPServerConfigurable_itemname#>", "type":"switch", "id":"dhcp_enable", "openHint":"5_1"};
			Get_Component_Switch(dhcp_enable_parm).appendTo($net_mode_container);

			var ipaddr_parm = {"title":"<#LAN_IP#>", "type":"text", "id":"ipaddr", "need_check":true, "maxlength":15};
			Get_Component_Input(ipaddr_parm).appendTo($net_mode_container)
				.find("#" + ipaddr_parm.id + "")
				.unbind("keypress").keypress(function(){
					return validator.isIPAddr(this, event);
				});

			var netmask_options = [
				{"text":"255.255.255.0 (253 <#Full_Clients#>)","value":"255.255.255.0"},
				{"text":"255.255.255.128 (125 <#Full_Clients#>)","value":"255.255.255.128"},
				{"text":"255.255.255.192 (61 <#Full_Clients#>)","value":"255.255.255.192"},
				{"text":"255.255.255.224 (29 <#Full_Clients#>)","value":"255.255.255.224"},
				{"text":"255.255.255.240 (13 <#Full_Clients#>)","value":"255.255.255.240"},
			];
			var netmask_parm = {"title": "<#IPConnection_x_ExternalSubnetMask_itemname#>", "id": "netmask", "options": netmask_options, "set_value": "255.255.255.0", "openHint":"4_2"};
			Get_Component_Custom_Select(netmask_parm).appendTo($net_mode_container);

			const dhcp_lease_parm = {"title":`<#LANHostConfig_LeaseTime_itemname#> (<#Second#>)`, "type":"text", "id":"dhcp_lease", "need_check":true, "maxlength":6, "openHint":"5_5"};
			Get_Component_Input(dhcp_lease_parm).appendTo($net_mode_container)
				.find("#" + dhcp_lease_parm.id + "")
				.unbind("keypress").keypress(function(){
					return validator.isNumber(this,event);
				});
		}

		var vid_parm = {"title":"<#WANVLANIDText#>", "type":"text", "id":"vlan_id", "need_check":true, "maxlength":4};//1-4093
		if(is_Web_iframe){
			if(isSupport("SMART_HOME_MASTER_UI")){
				Get_Component_Input(vid_parm).appendTo($net_mode_container)
					.find("#" + vid_parm.id + "")
					.unbind("keypress").keypress(function(){
						return validator.isNumber(this,event);
					});
			}
			else{
				vid_parm["btn_text"] = "<#CTL_assign#>";
				Get_Component_Input_And_Btn(vid_parm).appendTo($net_mode_container)
					.find("#" + vid_parm.id + ", .profile_btn_container").each(function(){
						if($(this).filter("#" + vid_parm.id + "").length){
							$(this).filter("#" + vid_parm.id + "").unbind("keypress").keypress(function(){
								return validator.isNumber(this,event);
							})
						}
						if($(this).filter(".profile_btn_container").length){
							$(this).filter(".profile_btn_container").unbind("click").click(function(e){
								e = e || event;
								e.stopPropagation();
								top.location.href = "/Advanced_VLAN_Profile_Content.asp";
							});
						}
					});
			}
		}
		else{
			Get_Component_Input(vid_parm).appendTo($net_mode_container)
				.find("#" + vid_parm.id + "")
				.unbind("keypress").keypress(function(){
					return validator.isNumber(this,event);
				});
		}

		if(isSwMode("rt")){
			var hide_ssid_parm = {"title":"<#WLANConfig11b_x_BlockBCSSID_itemname#>", "type":"switch", "id":"hide_ssid"};
			Get_Component_Switch(hide_ssid_parm).attr({"data-group":"wifi_settings"}).appendTo($net_mode_container);

			var dns_parm = {
				"title":"<#HSDPAConfig_DNSServers_itemname#>",
				"id":"dns",
				"openHint":"5_7",
				"text":"<#Setting_factorydefault_value#>",
				"container_id":"container_dns",
				"btn_text":"<#CTL_assign#>"
			}
			var $dns_obj = Get_Component_Pure_Text_And_Btn(dns_parm).appendTo($net_mode_container);
			$dns_obj.find("#" + dns_parm.id + "").attr({"data-dns1":"", "data-dns2":""});
			$dns_obj.find(".profile_btn_container").unbind("click").click(function(e){
				e = e || event;
				e.stopPropagation();
				$(".container").addClass("blur_effect");
				if($(".popup_container.popup_element").css("display") == "flex"){
					$(".popup_container.popup_element").addClass("blur_effect");
				}
				$(".popup_element_second").css("display", "flex");
				$(".popup_container.popup_element_second").empty();
				var dns_list = {"dns1":$content_container.find("#dns").attr("data-dns1"), "dns2":$content_container.find("#dns").attr("data-dns2")};
				$(".popup_container.popup_element_second").append(Get_Container_Assign_DNS(dns_list, callback_assign_dns));
				$(".popup_container.popup_element_second").addClass("fit_width");
				adjust_popup_container_top($(".popup_container.popup_element_second"), 100);
				resize_iframe_height();
				function callback_assign_dns(_dns_list){
					var $dns = $content_container.find("#dns");
					$dns.attr({"data-dns1":_dns_list.dns1, "data-dns2":_dns_list.dns2});
					if(_dns_list.dns1 == "" && _dns_list.dns2 == ""){
						$dns.html("<#Setting_factorydefault_value#>");
					}
					else{
						var dns_text = "";
						if(_dns_list.dns1 != ""){
							dns_text += _dns_list.dns1;
						}
						if(_dns_list.dns2 != ""){
							if(dns_text != "")
								dns_text += ", ";
							dns_text += _dns_list.dns2;
						}
						$dns.html(htmlEnDeCode.htmlEncode(dns_text));
					}
				}
			});

			var ap_iso_parm = {"title":"<#WLANConfig11b_x_IsolateAP_itemname#>", "type":"switch", "id":"ap_isolate", "openHint":"3_5"};
			Get_Component_Switch(ap_iso_parm).attr({"data-group":"wifi_settings"}).appendTo($net_mode_container);
		}
		return $net_mode_container;
	}
	function Get_Net_AiMesh_Container(){
		var $net_mode_container = $("<div>").attr({"data-slide_target":"category_aimesh_cntr"});

		var $AiMesh_List = $("<div>").attr({"data-container":"AiMesh_List"}).append(Get_AiMesh_List_Container()).appendTo($net_mode_container);
		$AiMesh_List.find(".node_content_container").click(function(e){
			e = e || event;
			e.stopPropagation();
			set_apply_btn_status($(this).closest(".profile_setting"));
		});

		return $net_mode_container;
	}
	function handle_category_container(_obj, _parm){
		$subtab_container_net.find(".category_slide_title").filter(":not(#" + _parm.id + ")").removeClass("expand");
		$subtab_container_net.find("[data-slide_target]").hide();
		$parent_cntr = $(_obj).parent(".category_slide_title").toggleClass("expand");
		if($parent_cntr.hasClass("expand")){
			$subtab_container_net.find("[data-slide_target='" + _parm.slide_target + "']").show();
		}
		resize_iframe_height();
	}
}
function Update_Setting_Profile(_obj, _profile_data){
	const is_mlo_bh = check_is_mlo_bh(_profile_data.apg_rl);
	const is_mlo_fh = (_profile_data.sdn_rl.sdn_name == "MLO" && _profile_data.apg_rl.mlo == "2") ? true : false;
	const is_mlo_legacy = (_profile_data.sdn_rl.sdn_name == "LEGACY");
	if(is_mlo_bh){
		$(_obj).find(`.action_container .btn_container.apply`).remove();
	}
	if(is_mlo_legacy || is_mlo_bh){
		$(_obj).find("#title_del_btn, #action_container #del_btn, [data-container=action_container] > [data-component=del_btn]").remove();
	}
	var support_portal = (function(){
		if(isSwMode("rt")){
			if(_profile_data.sdn_rl.sdn_name == "Portal")
				return true;
			else if(_profile_data.sdn_rl.sdn_name == "Kids"){
				if(get_cp_type_support("4")){
					return true;
				}
			}
		}
		return false;
	})();
	if(support_portal){
		if(_profile_data.sdn_rl.sdn_name == "Portal"){
			$(_obj).find("[data-portal-type=4], #select_portal_type > div[value=4]").remove();
		}
		else if(_profile_data.sdn_rl.sdn_name == "Kids"){
			$(_obj).find("[data-portal-type=2], #select_portal_type > div[value=2]").remove();
		}
	}
	else{
		$(_obj).find("[subtab=portal], [subtab_container=portal]").remove();
	}
	$(_obj).find("[data-container=profile_title]").html("<#DSL_profile#>");
	const specific_wizard_type = wizard_type_text.find(function(item, index, array){
		return (item.type == _profile_data.sdn_rl.sdn_name);
	});
	if(specific_wizard_type != undefined || is_mlo_legacy){
		let profile_title_text = "";
		if(is_mlo_legacy){
			profile_title_text = `<#GuestNetwork_IoT_default#>`;
		}
		else{
			profile_title_text = specific_wizard_type.text;
			if($.isEmptyObject(_profile_data.subnet_rl)){
				if(_profile_data.sdn_rl.sdn_name == "MLO"){
					if(is_mlo_bh){
						profile_title_text = `<#AiMesh_WiFi_Backhaul#> (MLO)`;
					}
				}
				else{
					profile_title_text += ` (Main Subnet)`;/* untranslated */
				}
			}
		}
		if(profile_title_text != "")
			$(_obj).find("[data-container=profile_title]").html(htmlEnDeCode.htmlEncode(profile_title_text));
	}
	$(_obj).find(".setting_content_container, .profile_setting").attr({"sdn_idx":_profile_data.sdn_rl.idx, "sdn_type":_profile_data.sdn_rl.sdn_name});
	$(_obj).find("#sdn_name").val(_profile_data.apg_rl.ssid);
	var apg_sec_array = (_profile_data.apg_rl.security).split("<");
	var cap_wifi_auth = (_profile_data.sdn_rl.sdn_name == "Portal" || _profile_data.sdn_rl.sdn_name == "Guest") ? "open" : "psk2";
	var cap_wifi_pwd = "";
	if(apg_sec_array[1] != undefined && apg_sec_array[1] != ""){
		var cap_sec_array = apg_sec_array[1].split(">");
		cap_wifi_auth = cap_sec_array[1];
		cap_wifi_pwd = cap_sec_array[3];
	}
	$(_obj).find("#sdn_pwd").val(cap_wifi_pwd);

	let wifi_band_value = "0";
	let wifi_band_select = {"band_2g":false, "band_5g":false, "band_6g":false};
	var apg_dut_list_array = (_profile_data.apg_rl.dut_list).split("<");
	$.each(apg_dut_list_array, function(index, dut_info){
		if(dut_info != ""){
			let dut_info_array = dut_info.split(">");
			if(!validator.mac_addr(dut_info_array[0]) && !is_mlo_legacy)
				return false;
			const wifi_band = isNaN(parseInt(dut_info_array[1])) ? 0 : parseInt(dut_info_array[1]);
			if(wifi_band & 1){//2G
				wifi_band_select.band_2g = true;
			}
			if(wifi_band & 2 || wifi_band & 4 || wifi_band & 8){//5G
				wifi_band_select.band_5g = true;
			}
			if(wifi_band & 16 || wifi_band & 32 || wifi_band & 64){//6G
				wifi_band_select.band_6g = true;
			}
			return false;
		}
	});
	if(wifi_band_select.band_2g && !wifi_band_select.band_5g && !wifi_band_select.band_6g){
		wifi_band_value = "1";
	}
	else if(!wifi_band_select.band_2g && wifi_band_select.band_5g && !wifi_band_select.band_6g){
		wifi_band_value = "2";
	}
	else if(wifi_band_select.band_2g && wifi_band_select.band_5g && !wifi_band_select.band_6g){
		wifi_band_value = "3";
	}
	else if(!wifi_band_select.band_2g && !wifi_band_select.band_5g && wifi_band_select.band_6g){
		wifi_band_value = "4";
	}
	else if(!wifi_band_select.band_2g && wifi_band_select.band_5g && wifi_band_select.band_6g){
		wifi_band_value = "5";
	}
	else if(wifi_band_select.band_2g && wifi_band_select.band_5g && wifi_band_select.band_6g){
		wifi_band_value = "6";
	}

	const band_options = (() => {
		if(_profile_data.sdn_rl.sdn_name == "MLO"){
			const option_none = [wifi_band_options.find((el) => {
				return (el.value == "0");
			})];
			if(isSupport("mlo")){
				if(is_mlo_bh || is_mlo_legacy)
					return option_none;
				else{
					if(wifi_band_value == "0")
						return option_none;
					else{
						const all_band_num = aimesh_wifi_mlo_info.cap.all_band_num;
						const mlo_band_info = mlo_band_num_mapping.find(function(item){
							return (item.all_band == all_band_num);
						});
						if(mlo_band_info != undefined){
							return mlo_band_info.ui_option;
						}
						else
							return option_none;
					}
				}
			}
			else
				return option_none;
		}
		else{
			return wifi_band_options.filter((el) => {
				if(_profile_data.sdn_rl.sdn_name == "Employee")
					return (el.value != "5" && el.value != "6");//2G5G,2G,5G,6G,none
				else
					return (el.value != "5");//2G5G,2G,5G,6G,none,2G5G6G
			});
		}
	})();
	let band_option_is_exist = false;
	$(_obj).find("#select_wifi_band").children().each(function(){
		const option_value = $(this).attr('value');
		const option_exist = band_options.some((el) => {
			return (el.value == option_value);
		});
		if(!option_exist)
			$(this).remove();

		if(option_value == wifi_band_value){
			band_option_is_exist = true;
		}
	});
	if(!band_option_is_exist)
		wifi_band_value = "0";
	set_value_Custom_Select(_obj, "wifi_band", wifi_band_value);
	update_aimesh_wifi_band_info(_profile_data);
	update_aimesh_wifi_band_full();

	let is_wifi_open = (cap_wifi_auth == "open" || cap_wifi_auth == "owe" || cap_wifi_auth == "openowe") ? true : false;
	if(_profile_data.sdn_rl.sdn_name == "Guest" || _profile_data.sdn_rl.sdn_name == "Portal"){
		var wifi_auth = "psk2";
		if(is_wifi_open){
			$(_obj).find("#security_guest .switch_text_container").children("[data-option-id=open]").click();
			wifi_auth = "psk2";
		}
		else{
			$(_obj).find("#security_guest .switch_text_container").children("[data-option-id=pwd]").click();
			wifi_auth = cap_wifi_auth;
		}
		set_value_Custom_Select(_obj, "wifi_auth", wifi_auth);
	}
	else{
		$(_obj).find("#security_guest").remove();
	}

	if(_profile_data.sdn_rl.sdn_name == "Employee"){
		var wifi_auth = "psk2";
		var wifi_auth_radius = "wpa2";
		if(cap_wifi_auth == "wpa2" || cap_wifi_auth == "wpawpa2" ||
			cap_wifi_auth == "wpa3" || cap_wifi_auth == "wpa2wpa3" || cap_wifi_auth == "suite-b"
		){
			$(_obj).find("#radius_ipaddr").val(_profile_data.radius_rl.auth_server_1);
			$(_obj).find("#radius_port").val(_profile_data.radius_rl.auth_port_1);
			$(_obj).find("#radius_key").val(_profile_data.radius_rl.auth_key_1);
			$(_obj).find("#security_employee .switch_text_container").children("[data-option-id=radius]").click();
			wifi_auth = "psk2";
			wifi_auth_radius = cap_wifi_auth;
		}
		else{
			$(_obj).find("#security_employee .switch_text_container").children("[data-option-id=pwd]").click();
			wifi_auth = cap_wifi_auth;
			wifi_auth_radius = "wpa2";
		}
		set_value_Custom_Select(_obj, "wifi_auth", wifi_auth);
		set_value_Custom_Select(_obj, "wifi_auth_radius", wifi_auth_radius);
	}
	else{
		$(_obj).find("#security_employee, [data-sec-option-id=radius]").remove();
	}

	if(_profile_data.sdn_rl.sdn_name != "Guest" &&
		_profile_data.sdn_rl.sdn_name != "Employee" &&
		_profile_data.sdn_rl.sdn_name != "Portal")
	{
		set_value_Custom_Select(_obj, "wifi_auth", cap_wifi_auth);
	}

	var is_radius_server = (cap_wifi_auth == "wpa2" || cap_wifi_auth == "wpawpa2" || cap_wifi_auth == "wpa3" || cap_wifi_auth == "wpa2wpa3" || cap_wifi_auth == "suite-b");
	var is_eth_only = (wifi_band_value == "0") ? true : false;
	var is_wifi_off = (_profile_data.sdn_rl.wifi_sched_on == "0") ? true : false;
	if(is_radius_server || is_eth_only || is_wifi_off){
		$(_obj).find("#qrcode_btn").remove();
		$(_obj).find("#qrcode_print_btn").closest(".profile_setting_item").remove();
	}
	else{
		var qrstring = "WIFI:";
		qrstring += "S:" + encode_utf8(escape_string(_profile_data.apg_rl.ssid)) + ";";
		qrstring += "T:" + (is_wifi_open ? "nopass" : "WPA") + ";";
		qrstring += "P:" + (is_wifi_open ? "" : escape_string(cap_wifi_pwd)) + ";";
		if(_profile_data.apg_rl.hide_ssid == "1"){
			qrstring += "H:true;"
		}
		qrstring += ';';
		$(_obj).find("#qrcode_btn").unbind("click").click(function(e){
			e = e || event;
			e.stopPropagation();
			$(".container").addClass("blur_effect");
			if($(".popup_container.popup_element").css("display") == "flex"){
				$(".popup_container.popup_element").addClass("blur_effect");
			}
			$(".popup_element_second").css("display", "flex");
			$(".popup_container.popup_element_second").empty();
			$(".popup_container.popup_element_second").append(Get_Component_QRCode({"text": qrstring}));
			adjust_popup_container_top($(".popup_container.popup_element_second"), 100);

			function Get_Component_QRCode(_parm){
				var text = "https://www.asus.com/";
				if(_parm != undefined){
					if(_parm.text != undefined){
						text = _parm.text;
					}
				}
				var $container = $("<div>");
				var $popup_title_container = $("<div>").addClass("popup_title_container").appendTo($container);
				$("<div>").addClass("title").html("<#WiFi_Share#>").appendTo($popup_title_container);
				$("<div>").addClass("vpn_icon_all_collect close_btn").unbind("click").click(function(e){
					e = e || event;
					e.stopPropagation();
					close_popup_container($container);
				}).appendTo($popup_title_container);
				var $popup_content_container = $("<div>").addClass("popup_content_container").appendTo($container)
				var $wifi_share_cntr = $("<div>").addClass("wifi_share_container").appendTo($popup_content_container);
				var $qrcode_cntr = $("<div>").addClass("qrcode_container").appendTo($wifi_share_cntr);
				$("<div>").addClass("qr_code")
					.qrcode({text: text}).appendTo($qrcode_cntr)
					.find("canvas").css({"width":"100%", "height":"100%"});
				$("<div>").addClass("qr_text").html("<#Scan_QR_Code_To_Connect#>").appendTo($qrcode_cntr);

				var download_qrcode = {"support":false, "mode":""};
				if(isMobile()){
					if(typeof appWrapper != "undefined"){//APP RWD
						if(appWrapper === true){
							if(typeof app_action_support == "object"){
								download_qrcode.support = (app_action_support.download_qrcode == "1") ? true : false;
								download_qrcode.mode = ((download_qrcode.support) ? "postToApp" : "");
							}
						}
					}
				}
				else{
					download_qrcode.support = true;
					download_qrcode.mode = "download";
				}
				if(download_qrcode.support){
					var $export_btn_cntr = $("<div>").addClass("export_btn_container").appendTo($wifi_share_cntr);
					var $export_btn = $("<div>").addClass("export_btn").unbind("click").click(function(e){
						e = e || event;
						e.stopPropagation();
						var qrcode_image = "";
						var canvas = $qrcode_cntr.find("canvas");
						if(canvas.length == 1) {
							qrcode_image = canvas[0].toDataURL("image/png");
						}
						if(download_qrcode.mode == "download"){
							if(qrcode_image != ""){
								downloadURI("data:" + qrcode_image, "SSID_QRCode.png");
							}
							else{
								alert("<#weekSche_format_incorrect#>");
							}
						}
						else if(download_qrcode.mode == "postToApp"){
							postMessageToApp(
								{
									"app_action": "download_qrcode",
									"qrcode_text": qrstring,
									"qrcode_image": qrcode_image
								}
							);
						}

						function downloadURI(uri, name) {
							var link = document.createElement("a");
							link.download = name;
							link.href = uri;
							link.click();
						}
					}).appendTo($export_btn_cntr);
					$("<div>").addClass("btn_text").html(htmlEnDeCode.htmlEncode("QR Code")).appendTo($export_btn);/* untranslated */
				}
				return $container;
			}
		});

		if(!isMobile()){
			var is_portal_type = false;
			if(!($.isEmptyObject(_profile_data.cp_rl))){
				if(_profile_data.cp_rl.cp_idx == "2"){
					is_portal_type = true;
				}
			}
			if(is_portal_type){
				$(_obj).find("#qrcode_print_btn").unbind("click").click(function(e){
					e = e || event;
					e.stopPropagation();
					var $canvas = $("<div>").qrcode({text: qrstring}).find("canvas").css({"width":"100%", "height":"100%"});
					var canvas = $canvas;
					if(canvas.length == 1) {
						var data = canvas[0].toDataURL("image/png");
						var args = {
							"qrcode_title": $(_obj).find("#FWF_brand_name").val(),
							"qrcode": data,
							"ssid": _profile_data.apg_rl.ssid,
							"pwd": (is_wifi_open) ? "" : cap_wifi_pwd,
							"passcode": (_profile_data.cp_rl.auth_type == "1") ? _profile_data.cp_rl.local_auth_profile : ""
						};
						if(args.qrcode_title != ""){
							$("#print_html #print_wifi_info_desc").html(args.qrcode_title);
						}
						$("#print_html #print_qrcode_img").css("background-image", "url(" + args.qrcode + ")");
						$("#print_html #print_wifi_ssid").html(args.ssid);
						if(args.pwd != ""){
							$("#print_html #print_wifi_pwd_cntr").show().find("#print_wifi_pwd").html(args.pwd);
						}
						else{
							$("#print_html #print_wifi_pwd_cntr").hide();
						}
						if(args.passcode != ""){
							$("#print_html #print_wifi_passcode_cntr").show().find("#print_wifi_passcode").html(args.passcode);
						}
						else{
							$("#print_html #print_wifi_passcode_cntr").hide();
						}
						$("#print_html #print_wifi_info_desc").html(args.title);
						if(parent.webWrapper){
							$("#print_html .print_bottom_desc").html("Powered by ASUS ExpertWiFi");
						}
						else{
							$("#print_html .print_bottom_desc").html("Powered by ASUS");
						}

						var printWin = window.open('', '_blank', '');
						var $html_cntr = $("<html>");
						$("<meta>").attr({"charset":"UTF-8"}).appendTo($html_cntr);
						$("<meta>").attr({"name":"viewport", "content":"width=device-width, initial-scale=1.0, user-scalable=no"}).appendTo($html_cntr);
						$("<meta>").attr({"http-equiv":"X-UA-Compatible", "content":"ie=edge"}).appendTo($html_cntr);
						var $head_cntr = $("<head>").appendTo($html_cntr);
						$("<title>").html("Print").appendTo($head_cntr);
						$head_cntr.append($("#print_css").html());
						var $body_cntr = $("<body>").addClass("print_body").appendTo($html_cntr);
						$body_cntr.append($("#print_html").html());
						printWin.document.write($html_cntr[0].outerHTML);
						setTimeout(function(){
							printWin.print();
							printWin.close();
						}, 50);
					}
				});
			}
			else{
				$(_obj).find("#qrcode_print_btn").closest(".profile_setting_item").remove();
			}
		}

		function escape_string(_str){
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
	}
	const support_bw = (function(){
		if(isSwMode("rt")){
			if(support_portal)
				return false;
			else
				return (_profile_data.sdn_rl.sdn_name != "IoT") ? true : false;
		}
		else
			return false;
	})();
	if(support_bw){
		var bw_limit = _profile_data.apg_rl.bw_limit;
		$(_obj).find("#container_bw_dl, #container_bw_ul, #container_bw").hide();
		$(_obj).find("#bw_enabled").removeClass("off on").addClass((function(){
			var bw_enabled = bw_limit.substr(1,1);
			if(bw_enabled == "1"){
				var bw_list = bw_limit.substr(3).split(">");
				$(_obj).find("#bw_dl").val((bw_list[1]/1024));
				$(_obj).find("#bw_ul").val((bw_list[0]/1024));
				$(_obj).find("#container_bw_dl, #container_bw_ul,  #container_bw").show();
				return "on";
			}
			else{
				return "off";
			}
		})());
	}
	else{
		$(_obj).find("#bw_enabled").closest(".profile_setting_item").remove();
		$(_obj).find("#container_bw").remove();
	}

	var support_access_intranet = (function(){
		return (isSwMode("rt") && _profile_data.sdn_rl.sdn_name != "IoT") ? true : false;
	})();
	if(support_access_intranet){
		$(_obj).find("#access_intranet").removeClass("off on").addClass((function(){
			var result = "off";
			if(!($.isEmptyObject(_profile_data.sdn_access_rl))){
				var specific_data = _profile_data.sdn_access_rl.filter(function(item, index, array){
					return (item.sdn_idx == _profile_data.sdn_rl.idx && item.access_sdn_idx == "0");
				})[0];
				if(specific_data != undefined){
					result = "on";
				}
			}
			return result;
		})());
	}
	else{
		$(_obj).find("#access_intranet").closest(".profile_setting_item").remove();
	}

	$(_obj).find("#ap_isolate").removeClass("off on").addClass((function(){
		return ((_profile_data.apg_rl.ap_isolate == "1") ? "on" : "off");
	})());

	$(_obj).find("#schedule").removeClass("off on").addClass((function(){
		var schedule_config = {data:schedule_handle_data.string_to_json_array(_profile_data.apg_rl.sched)};
		if(_profile_data.sdn_rl.sdn_name == "Guest"){
			schedule_config["AccTime_support"] = true;
			schedule_config["AccTime_data"] = accesstime_handle_data.string_to_json_array(_profile_data.apg_rl.expiretime);
			schedule_config["mode"] = parseInt(_profile_data.apg_rl.timesched);
			schedule_config["AccTime_quickset_callback"] = resize_iframe_height;
			schedule_config["show_timeset_viewport_callback"] = resize_iframe_height;
			schedule_config["icon_trash_callback"] = resize_iframe_height;
			schedule_config["btn_save_callback"] = resize_iframe_height;
		}
		sdn_schedule = new schedule(schedule_config);
		$(_obj).find("#container_schedule").append(sdn_schedule.Get_UI());
		if(parseInt(_profile_data.apg_rl.timesched) > 0){
			$(_obj).find("#container_schedule").show();
			return "on";
		}
		else{
			$(_obj).find("#container_schedule").hide();
			return "off";
		}
	})());

	if(!($.isEmptyObject(_profile_data.subnet_rl))){
		var dns_arr = _profile_data.subnet_rl.dns.split(",");
		var dns_list = {"dns1":dns_arr[0], "dns2":dns_arr[1]};
		$(_obj).find("#dns").attr({"data-dns1":dns_list.dns1, "data-dns2":dns_list.dns2});
		var dns_text = "";
		if(dns_list.dns1 != ""){
			dns_text += dns_list.dns1;
		}
		if(dns_list.dns2 != ""){
			if(dns_text != "")
				dns_text += ", ";
			dns_text += dns_list.dns2;
		}
		if(dns_text != "")
			$(_obj).find("#dns").html(htmlEnDeCode.htmlEncode(dns_text));

		$(_obj).find("#dhcp_enable").removeClass("off on").addClass((function(){
			return ((_profile_data.subnet_rl.dhcp_enable == "1") ? "on" : "off");
		})());
		$(_obj).find("#ipaddr").val(_profile_data.subnet_rl.addr);
		set_value_Custom_Select(_obj, "netmask", _profile_data.subnet_rl.netmask);
		$(_obj).find("#dhcp_lease").val(_profile_data.subnet_rl.dhcp_lease);
	}
	else{
		$(_obj).find("#dhcp_enable").closest(".profile_setting_item").remove();
		$(_obj).find("#ipaddr").closest(".profile_setting_item").remove();
		$(_obj).find("#netmask").closest(".profile_setting_item").remove();
		$(_obj).find("#dhcp_lease").closest(".profile_setting_item").remove();
		$(_obj).find("#container_dns").remove();
	}
	if(!($.isEmptyObject(_profile_data.vlan_rl))){
		$(_obj).find("#vlan_id").val(_profile_data.vlan_rl.vid);
	}
	else{
		$(_obj).find("#vlan_id").closest(".profile_setting_item, .profile_setting_two_item").remove();
	}
	$(_obj).find("#hide_ssid").removeClass("off on").addClass((function(){
		return ((_profile_data.apg_rl.hide_ssid == "1") ? "on" : "off");
	})());

	var vpn_type = "";
	var vpn_idx = "";
	if(_profile_data.sdn_rl.vpnc_idx > "0"){
		vpn_type = "vpnc";
		vpn_idx = _profile_data.sdn_rl.vpnc_idx;
	}
	else if(_profile_data.sdn_rl.vpns_idx > "0"){
		vpn_type = "vpns";
		vpn_idx = _profile_data.sdn_rl.vpns_idx;
	}
	if($(_obj).find("[data-vpn-type="+vpn_type+"][data-idx="+vpn_idx+"]").length == 0){
		vpn_type = "";
		vpn_idx = "";
	}
	$(_obj).find("[data-container=VPN_Profiles]").hide();
	$(_obj).find("#vpn_enabled").removeClass("off on").addClass((function(){
		if(vpn_type == "")
			return "off";
		else{
			$(_obj).find("[data-container=VPN_Profiles]").show()
				.find(".rwd_icon_radio").removeClass("clicked").filter("[data-vpn-type="+vpn_type+"][data-idx="+vpn_idx+"]").addClass("clicked");
			return "on";
		}
	})());

	$.each(vpnc_profile_list, function(index, item){
		if(item.activate == "0"){
			$(_obj).find("[data-container=VPN_Profiles] [data-vpn-type=vpnc][data-idx="+item.vpnc_idx+"]").siblings("[data-component=icon_warning]").show();
		}
	});

	$.each(vpns_rl_json, function(index, item){
		if(item.activate == "0"){
			$(_obj).find("[data-container=VPN_Profiles] [data-vpn-type=vpns][data-idx="+item.vpns_idx+"]")
				.siblings("[data-component=icon_warning]").show();
		}
		if(item.sdn_idx != "" && item.vpns_idx != vpn_idx){
			$(_obj).find("[data-container=VPN_Profiles] [data-vpn-type=vpns][data-idx="+item.vpns_idx+"]").addClass("disabled").attr({"data-disabled":"true"})
				.siblings("[data-component=icon_error]").show();
		}
	});

	if(support_portal){
		set_value_Custom_Select(_obj, "portal_type", _profile_data.sdn_rl.cp_idx);
		var $select_portal_type = $(_obj).find("#select_portal_type");
		$.each(cp_type_rl_json, function(index, item){
			if(item.sdn_idx != "" && item.sdn_idx != _profile_data.sdn_rl.idx){
				$select_portal_type.children("[value='"+item.cp_idx+"']")
					.addClass("disabled")
					.attr({"data-disabled":"true"})
					.html(htmlEnDeCode.htmlEncode(item.cp_text + " (<#Status_Used#>)"))
			}
		});
		$select_portal_type.children("div").click(function(e){
			e = e || event;
			e.stopPropagation();
			var options = $(this).attr("value");
			if(options > "0"){
				var specific_cp_type_rl = cp_type_rl_json.filter(function(item, index, array){
					return (item.cp_idx == options);
				})[0];
				if(specific_cp_type_rl != undefined){
					if(specific_cp_type_rl.profile[0] != undefined){
						update_freewifi_settings(specific_cp_type_rl.profile[0]);
					}
				}
			}
		});
		if(_profile_data.sdn_rl.sdn_enable == "1"){
			if(_profile_data.sdn_rl.cp_idx == "0"){
				$.each(sdn_all_rl_json, function(index, item) {
					if(item.sdn_rl.idx == _profile_data.sdn_rl.idx)
						return true;
					if(item.sdn_rl.cp_idx != "0"){
						if(item.sdn_rl.sdn_enable == "1"){
							$select_portal_type
								.closest(".custom_select_container").attr("temp_disable", "disabled").addClass("temp_disable")
								.closest(".profile_setting_item")
								.after($("<div>").html("<#vpnc_conn_maxi_general#>".replace(/VPN/g, "<#GuestNetwork_Portal_type#>").replace(/2/g, "1")).addClass("item_hint"));
							return false;
						}
					}
				});
			}
		}

		function update_freewifi_settings(_cp_rl_data){
			if(_cp_rl_data.cp_idx == "2"){
				$(_obj).find("#FWF_conntimeout").val((parseInt(_cp_rl_data.conntimeout))/60);
				$(_obj).find("#FWF_redirecturl").val(_cp_rl_data.redirecturl);
				var response = httpApi.hookGet("get_customized_attribute-" + _cp_rl_data.idx_for_customized_ui + "");
				if(response != undefined && response != "NoData"){
					var settings = response.splash_page_setting;
					$(_obj).find("#FWF_brand_name").val(encode_decode_text(settings.brand_name, "decode"));
					$(_obj).find("#FWF_ui_container [data-component=FWF_brand_name]").html(htmlEnDeCode.htmlEncode($(_obj).find("#FWF_brand_name").val()));
					$(_obj).find("#FWF_ui_container [data-component=FWF_bg]").css('background-image', 'url(' + settings.image + ')');
					var terms_service_content = encode_decode_text(settings.terms_service_content, "decode");
					$(_obj).find("#FWF_terms_service").val(terms_service_content);
					terms_service_content = htmlEnDeCode.htmlEncode(terms_service_content).replace().replace(/(?:\r\n|\r|\n)/g, '<div style=height:6px;></div>');
					$(_obj).find("#FWF_ui_container [data-component=FWF_TS]").html(terms_service_content);
				}
				$(_obj).find("#FWF_passcode_enabled").removeClass("off on").addClass((function(){
					if(_cp_rl_data.auth_type == "1"){
						$(_obj).find("#container_FWF_passcode, #FWF_Preview_Cntr [data-component=FWF_passcode]").show();
						return "on";
					}
					else{
						$(_obj).find("#container_FWF_passcode, #FWF_Preview_Cntr [data-component=FWF_passcode]").hide();
						return "off";
					}
				})());
				$(_obj).find("#FWF_terms_service_enabled").removeClass("off on").addClass((function(){
					if(_cp_rl_data.term_of_service == "0"){
						$(_obj).find("#container_FWF_TS, #FWF_Preview_Cntr [data-group=FWF_TS]").hide();
						$(_obj).find(".FWF_portal_cntr").addClass("no_terms_service");
						return "off";
					}
					else{
						$(_obj).find("#container_FWF_TS, #FWF_Preview_Cntr [data-group=FWF_TS]").show();
						$(_obj).find(".FWF_portal_cntr").removeClass("no_terms_service");
						return "on";
					}
				})());
				$(_obj).find("#FWF_passcode").val(_cp_rl_data.local_auth_profile);
			}
			else if(_cp_rl_data.cp_idx == "4"){
				var response = httpApi.hookGet("get_customized_attribute-" + _cp_rl_data.idx_for_customized_ui + "");
				if(response != undefined && response != "NoData"){
					var settings = response.splash_page_setting;
					$(_obj).find("#MB_desc").val(encode_decode_text(settings.MB_desc, "decode"));
					$(_obj).find("#MB_ui_container [data-component=MB_desc]").html(htmlEnDeCode.htmlEncode($(_obj).find("#MB_desc").val()));
					$(_obj).find("#MB_ui_container [data-component=MB_bg]").css('background-image', 'url(' + settings.image + ')');
				}
			}
			set_apply_btn_status($(_obj).find(".profile_setting"));
		}

		$(_obj).find("[data-portal-type]").hide(0, function(){
			$(this).find(".profile_setting_item").hide();
		}).filter("[data-portal-type="+_profile_data.sdn_rl.cp_idx+"]").show(0, function(){
			$(this).find(".profile_setting_item").show();
		});
		if(_profile_data.sdn_rl.cp_idx == "2"){
			$(_obj).find("#FWF_brand_name").val(_profile_data.apg_rl.ssid);
			$(_obj).find("#FWF_ui_container [data-component=FWF_brand_name]").html(htmlEnDeCode.htmlEncode(_profile_data.apg_rl.ssid));
			if(!($.isEmptyObject(_profile_data.cp_rl))){
				if(_profile_data.cp_rl.cp_idx == "2"){
					update_freewifi_settings(_profile_data.cp_rl);
				}
			}
		}
		else if(_profile_data.sdn_rl.cp_idx == "4"){
			$(_obj).find("#MB_desc").val("<#Description#>");
			if(!($.isEmptyObject(_profile_data.cp_rl))){
				if(_profile_data.cp_rl.cp_idx == "4"){
					update_freewifi_settings(_profile_data.cp_rl);
				}
			}
		}
	}

	let $sel_wifi_band = $(_obj).find("#select_wifi_band");
	$sel_wifi_band.closest(".custom_select_container").click();
	$sel_wifi_band.find("div.selected").click();
	let $AiMesh_List = $(_obj).find("[data-container=AiMesh_List]");
	if(is_mlo_fh){
		Set_AiMesh_List_CB_MLO($AiMesh_List);
	}
	else{
		Set_AiMesh_List_CB($AiMesh_List, $sel_wifi_band.children(".selected").attr("value"));
	}
	$AiMesh_List.find(".icon_checkbox").removeClass("clicked");
	$.each(apg_dut_list_array, function(index, dut_info){
		if(dut_info != ""){
			var dut_info_array = dut_info.split(">");
			var node_mac = dut_info_array[0];
			var wifi_band = dut_info_array[1];
			var specific_node = cfg_clientlist.filter(function(item, index, array){
				return (item.mac == node_mac);
			})[0];
			var node_online = false;
			if(specific_node != undefined){
				node_online = (specific_node.online == "1") ? true : false;
			}
			var $node_container = $AiMesh_List.find("[data-node-mac='" + node_mac + "']");
			if($node_container.length > 0){
				if(wifi_band > "0"){
					if(aimesh_wifi_band_info[node_mac] != undefined && aimesh_wifi_band_info[node_mac].length > 0){
						$node_container.attr({"data-profile-setting":"true"}).find(".icon_checkbox").toggleClass("clicked").removeClass("closed");
					}
				}
			}
			else{
				Get_AiMesh_Offline_Container(dut_info_array).appendTo($AiMesh_List);
				$AiMesh_List.find("[data-node-mac='" + node_mac + "'] .icon_checkbox").toggleClass("clicked");
			}
		}
	});

	if(is_mlo_legacy){
		$(_obj).find("[subtab=vpn]").hide();
		$(_obj).find("#category_general, [data-category-cntr=aimesh], [data-category-cntr=adv]").hide();
		$(_obj).find("[data-group=wifi_settings]:visible").filter(":not([data-sec-option-id=pwd])").hide();
		$(_obj).find("#access_intranet").closest(".profile_setting_item").hide();
		$sel_wifi_band.closest(".custom_select_container").attr("temp_disable", "disabled").addClass("temp_disable");
		$(_obj).find("#select_wifi_auth").closest(".custom_select_container").attr("temp_disable", "disabled").addClass("temp_disable").closest(".profile_setting_item").show();
	}

	set_apply_btn_status($(_obj).find(".profile_setting"));

	$(_obj).find("#title_del_btn, #action_container #del_btn, [data-container=action_container] > [data-component=del_btn]").click(function(){
		$(".container").addClass("blur_effect");
		if($(".popup_container.popup_element").css("display") == "flex"){
			$(".popup_container.popup_element").addClass("blur_effect");
		}
		$(".popup_element_second").css("display", "flex");
		$(".popup_container.popup_element_second").empty();
		Get_Component_Del_Profile().appendTo($(".popup_container.popup_element_second")).find("[data-btn=del]").click(function(e){
			e = e || event;
			e.stopPropagation();
			var del_idx = "";
			$.each(sdn_all_rl_json, function(index, item) {
				if(item.sdn_rl.idx == _profile_data.sdn_rl.idx){
					del_idx = item.sdn_rl.idx;
					return false;
				}
			});

			if(del_idx !== ""){
				selected_sdn_idx = "";
				let sdn_all_rl_tmp = JSON.parse(JSON.stringify(sdn_all_rl_json));
				let del_sdn_rl = [sdn_all_rl_tmp.find(item => item.sdn_rl.idx == del_idx)];
				sdn_all_rl_tmp = sdn_all_rl_tmp.filter(item => item.sdn_rl.idx != del_idx);
				var del_sdn_all_rl = parse_JSONToStr_del_sdn_all_rl(del_sdn_rl);
				vlan_rl_json = vlan_rl_json.filter(function(item, index, array){
					return (item.vlan_idx != del_sdn_rl[0].vlan_rl.vlan_idx);
				});
				var vlan_rl_tmp = JSON.parse(JSON.stringify(vlan_rl_json));
				var sdn_all_list = parse_JSONToStr_sdn_all_list({"sdn_all_rl":sdn_all_rl_tmp, "vlan_rl":vlan_rl_tmp});

				//check apgX_dut_list with port binding
				//console.log( _profile_data.apg_rl.dut_list);
				var do_restart_net_and_phy = false;	// do restart_net_and_phy or not
				if(_profile_data.apg_rl.dut_list != undefined){
					var apgX_dut_list_arr = _profile_data.apg_rl.dut_list.split("<");

					$.each(apgX_dut_list_arr, function(index, dut_info){
						if(dut_info != ""){
							var del_idx_dut_list_array_by_mac = dut_info.split(">");
							if(del_idx_dut_list_array_by_mac[2] != ""){	//port binding
								do_restart_net_and_phy = true;
							}
						}
					});
				}

				//check vlan_trunklist existed
				if( vlan_trunklist_orig != "" && _profile_data.vlan_rl.vid > 1){	//vid :2~4093
					do_restart_net_and_phy = true;
				}

				if( do_restart_net_and_phy ){
					var nvramSet_obj = {"action_mode": "apply", "rc_service": "restart_net_and_phy;"};
				}
				else{
					var nvramSet_obj = {"action_mode": "apply", "rc_service": "start_sdn_del;restart_wireless;"};
				}

				//update vlan_trunklist
				if( vlan_trunklist_orig != "" && _profile_data.vlan_rl.vid > 1){	//vid :2~4093
					var updated_vlan_trunklist = rm_vid_from_vlan_trunklist( _profile_data.vlan_rl.vid );
					nvramSet_obj.vlan_trunklist = updated_vlan_trunklist;
					if(vlan_rl_tmp==""){	//without vid for vlan_trunklist
						nvramSet_obj.vlan_trunklist = "";
					}
				}

				nvramSet_obj["apg" + del_sdn_rl[0].apg_rl.apg_idx + "_enable"] = "0";
				if(httpApi.nvramGet(["qos_enable"]).qos_enable == "1"){
					nvramSet_obj.rc_service += "restart_qos;restart_firewall;";
				}
				$.extend(nvramSet_obj, sdn_all_list);
				$.extend(nvramSet_obj, del_sdn_all_rl);
				close_popup_container($(this));
				var showLoading_status = get_showLoading_status(nvramSet_obj.rc_service);
				const parent_cntr = (typeof PAGE_CONTAINER == "string") ? PAGE_CONTAINER : "sdn";
				if(!httpApi.app_dataHandler){
					showLoading();
					close_popup_container("all");
					if(isWLclient()){
						showLoading(showLoading_status.time);
						setTimeout(function(){
							showWlHintContainer();
						}, showLoading_status.time*1000);
						check_isAlive_and_redirect({"page": "" + ((parent_cntr == "mlo") ? "MLO" : "SDN") + ".asp", "time": showLoading_status.time});
					}
				}
				httpApi.nvramSet(nvramSet_obj, function(){
					if(isWLclient()) return;
					showLoading(showLoading_status.time);
					init_sdn_all_list();
					if(parent_cntr == "mlo")
						show_mlo_profilelist();
					else
						show_sdn_profilelist();
					setTimeout(function(){
						if(parent_cntr != "mlo"){
							const display_profile_num = $("#profile_list_content .profile_item_container:not(.addnew)").length;
							if(!window.matchMedia('(max-width: 575px)').matches){
								$("#profile_list_content .profile_item_container.addnew").click();
								if(display_profile_num <= 0){
									$(".popup_element").css("display", "flex");
									$(".container").addClass("blur_effect");
									$(".popup_container.popup_element").empty()
										.append(show_Get_Start("popup"))
										.addClass(function(){
											return "hide_title_cntr" + ((isMobile()) ? " full_width" : "");
										})
										.find(".popup_title_container").hide();
									resize_iframe_height();
								}
							}
							else{
								if(display_profile_num <= 0){
									$("#profile_list_content .profile_item_container.addnew").click();
									$(".popup_container.popup_element")
										.addClass("hide_title_cntr")
										.find(".popup_title_container").hide();
								}
							}
						}
					}, showLoading_status.time*1000);
					if(!isMobile()){
						if(showLoading_status.disconnect){
							check_isAlive_and_redirect({"page": "" + ((parent_cntr == "mlo") ? "MLO" : "SDN") + ".asp", "time": showLoading_status.time});
						}
					}
				});
			}
		});
		adjust_popup_container_top($(".popup_container.popup_element_second"), 100);
		resize_iframe_height();
	});

	var $clientlist_container = $(_obj).find("[data-container=client_info_container] [data-container=clientlist_container]");
	var $client_num = $(_obj).find("[data-container=client_info_container] #client_tab > .title");
	var sdn_clientlist_data = [];
	var get_clientlist = httpApi.hookGet("get_clientlist");
	$.each(get_clientlist, function(index, client){
		if(client.sdn_idx != undefined && client.sdn_idx == _profile_data.sdn_rl.idx){
			if(client.isOnline == "1"){
				sdn_clientlist_data.push(client);
			}
		}
	});
	$client_num.html("<#ConnectedClient#> : " + sdn_clientlist_data.length + " <#Clientlist_Online#>");
	Get_Component_ClientList(sdn_clientlist_data).appendTo($clientlist_container);
}
function set_apply_btn_status(_obj){
	var $btn_container_apply = $(_obj).find(".action_container .btn_container.apply");
	var isBlank = validate_isBlank($(_obj));
	if(isBlank){
		$btn_container_apply.removeClass("valid_fail").addClass("valid_fail").unbind("click");
	}
	else{
		$btn_container_apply.removeClass("valid_fail").unbind("click").click(function(e){
			e = e || event;
			e.stopPropagation();
			var sdn_idx = $(_obj).closest(".setting_content_container").attr("sdn_idx");
			if(validate_format_Wizard_Item($(_obj), "ALL")){
				var specific_data = sdn_all_rl_json.filter(function(item, index, array){
					return (item.sdn_rl.idx == sdn_idx);
				})[0];
				var sdn_profile = {};
				var isNewProfile = false;
				if(specific_data == undefined){
					var sdn_obj = get_new_sdn_profile();
					sdn_idx = Object.keys(sdn_obj);
					sdn_profile = sdn_obj[sdn_idx];
					sdn_profile.sdn_rl.sdn_name = "Customized";
					isNewProfile = true;
				}
				else{
					sdn_profile = specific_data;
				}

				selected_sdn_idx = sdn_profile.sdn_rl.idx;

				//check apgX_dut_list with port binding	
				var do_restart_net_and_phy = false;	// do restart_net_and_phy or not
				if(sdn_profile.apg_rl.dut_list != undefined){
					var update_idx_dut_list_array = sdn_profile.apg_rl.dut_list.split("<");

					$.each(update_idx_dut_list_array, function(index, dut_info){
						if(dut_info != ""){
							var update_idx_dut_list_array_by_mac = dut_info.split(">");
							if(update_idx_dut_list_array_by_mac[2] != ""){	//port binding
								do_restart_net_and_phy = true;
							}
						}
					});
				}

				//check vlan_trunklist with port binding
				if( vlan_trunklist_orig != "" && sdn_profile.vlan_rl.vid > 1 ){	//vid :2~4093
					$.each(vlan_trunklist_json_tmp, function(index, item){
						if(item.profile == sdn_profile.vlan_rl.vid){
							do_restart_net_and_phy = true;
						}
					});
				}

				if( do_restart_net_and_phy ){
					var nvramSet_obj = {"action_mode": "apply", "rc_service": "restart_net_and_phy;"};
				}
				else{
					var nvramSet_obj = {"action_mode": "apply", "rc_service": "restart_wireless;restart_sdn " + selected_sdn_idx + ";"};
				}

				var rc_append = "";
				var wifi_band = parseInt($(_obj).find("#select_wifi_band").children(".selected").attr("value"));
				sdn_profile.apg_rl.ssid = $(_obj).find("#sdn_name").val();
				const is_mlo_fh = (sdn_profile.sdn_rl.sdn_name == "MLO" && sdn_profile.apg_rl.mlo == "2") ? true : false;
				const is_mlo_bh = (sdn_profile.sdn_rl.sdn_name == "MLO" && sdn_profile.apg_rl.mlo == "1") ? true : false;
				const is_mlo_legacy = (sdn_profile.sdn_rl.sdn_name == "LEGACY") ? true : false;
				var ori_dut_list_json = convertRulelistToJson(["mac","wifiband","lanport"], sdn_profile.apg_rl.dut_list);
				var dut_list = "";
				if(wifi_band > 0 || is_mlo_fh){
					var sec_option_id = "pwd";
					if(sdn_profile.sdn_rl.sdn_name == "Guest" || sdn_profile.sdn_rl.sdn_name == "Portal")
						sec_option_id = $(_obj).find("#security_guest .switch_text_container").children(".selected").attr("data-option-id");
					else if(sdn_profile.sdn_rl.sdn_name == "Employee")
						sec_option_id = $(_obj).find("#security_employee .switch_text_container").children(".selected").attr("data-option-id");

					var wifi_pwd = "";
					var wifi_auth = "psk2";
					let wifi_auth_6G = "sae";
					var wifi_crypto = "aes";
					sdn_profile.radius_rl = {};
					if(sec_option_id == "open"){
						wifi_auth = "open";
						wifi_auth_6G = "owe";
						wifi_pwd = "";
					}
					else if(sec_option_id == "radius"){
						sdn_profile.radius_rl = get_new_radius_rl();
						sdn_profile.radius_rl.radius_idx = sdn_profile.apg_rl.apg_idx;
						sdn_profile.radius_rl.auth_server_1 = $(_obj).find("#radius_ipaddr").val();
						sdn_profile.radius_rl.auth_port_1 = $(_obj).find("#radius_port").val();
						sdn_profile.radius_rl.auth_key_1 = $(_obj).find("#radius_key").val();
						wifi_auth = $(_obj).find("#select_wifi_auth_radius").children(".selected").attr("value");
						if(wifi_auth == "suite-b")
							wifi_crypto = "suite-b";
					}
					else if(sec_option_id == "pwd"){
						wifi_auth = $(_obj).find("#select_wifi_auth").children(".selected").attr("value");
						wifi_auth_6G = $(_obj).find("#select_wifi_auth_6G").children(".selected").attr("value");
						wifi_pwd = $(_obj).find("#sdn_pwd").val();
					}
					if(wifi_band == 4 || wifi_band == 5 || wifi_band == 6){
						if(sec_option_id == "open")
							wifi_auth = "openowe";
						else{
							wifi_auth = wifi_auth_6G;
						}
					}
					var radius_idx = sdn_profile.apg_rl.apg_idx;
					if(is_mlo_fh){
						sdn_profile.apg_rl.security = "<127>sae>aes>" + wifi_pwd + ">" + radius_idx + "";
					}
					else if(is_mlo_legacy){
						sdn_profile.apg_rl.security = "<127>psk2>aes>" + wifi_pwd + ">" + radius_idx + "";
					}
					else{
						sdn_profile.apg_rl.security = "<3>" + wifi_auth + ">" + wifi_crypto + ">" + wifi_pwd + ">" + radius_idx + "";
						sdn_profile.apg_rl.security += "<13>" + wifi_auth + ">" + wifi_crypto + ">" + wifi_pwd + ">" + radius_idx + "";
						sdn_profile.apg_rl.security += "<16>" + wifi_auth_6G + ">" + wifi_crypto + ">" + wifi_pwd + ">" + radius_idx + "";
						sdn_profile.apg_rl.security += "<96>" + wifi_auth_6G + ">" + wifi_crypto + ">" + wifi_pwd + ">" + radius_idx + "";
					}

					sdn_profile.apg_rl.sched = schedule_handle_data.json_array_to_string(sdn_schedule.Get_Value());
					sdn_profile.apg_rl.expiretime = "";
					if($(_obj).find("#schedule").hasClass("on")){
						sdn_profile.apg_rl.timesched = sdn_schedule.Get_Value_Mode();
						if(sdn_profile.apg_rl.timesched == "1"){
							sdn_profile.apg_rl.timesched = ((sdn_profile.apg_rl.sched == "") ? "0" : "1");
						}
					}
					else{
						sdn_profile.apg_rl.timesched = "0";
					}
					if($(_obj).find("#bw_enabled").hasClass("on")){
						sdn_profile.apg_rl.bw_limit = "<1>" + ($(_obj).find("#bw_ul").val())*1024 + ">" + ($(_obj).find("#bw_dl").val())*1024;
						nvramSet_obj.qos_enable = "1";
						nvramSet_obj.qos_type = "2";
						rc_append += "restart_qos;restart_firewall;";
					}
					else{
						if(httpApi.nvramGet(["qos_enable"]).qos_enable == "1"){
							rc_append += "restart_qos;restart_firewall;";
						}
						sdn_profile.apg_rl.bw_limit = "<0>>";
					}
					if($(_obj).find("#ap_isolate").hasClass("on"))
						sdn_profile.apg_rl.ap_isolate = "1";
					else
						sdn_profile.apg_rl.ap_isolate = "0";

					if($(_obj).find("#hide_ssid").hasClass("on"))
						sdn_profile.apg_rl.hide_ssid = "1";
					else
						sdn_profile.apg_rl.hide_ssid = "0";

					if(!is_mlo_legacy){
						$(_obj).find("[data-container=AiMesh_List]").find(".node_container").each(function(){
							if($(this).find(".icon_checkbox").hasClass("clicked")){
								var node_mac = $(this).attr("data-node-mac");
								var specific_node = cfg_clientlist.filter(function(item, index, array){
									return (item.mac == node_mac);
								})[0];
								if(specific_node != undefined){
									if(is_mlo_fh)
										dut_list += get_mlo_dut_list(aimesh_wifi_mlo_info.cap.mlo_band_num, specific_node, ori_dut_list_json);
									else
										dut_list += get_specific_dut_list(wifi_band, specific_node, ori_dut_list_json);
								}
							}
						});

						var offline_dut_list = get_Offline_dut_list(_obj, wifi_band, ori_dut_list_json);
						if(offline_dut_list != ""){
							dut_list += offline_dut_list;
						}
						dut_list += get_unChecked_dut_list(_obj, ori_dut_list_json);
					}
				}
				else{
					const dut_list_is_star = get_dut_list_is_star(sdn_profile.apg_rl.dut_list);
					if(!is_mlo_bh && !dut_list_is_star){
						dut_list = get_dut_list(wifi_band, ori_dut_list_json);
					}
				}
				if(dut_list != ""){
					sdn_profile.apg_rl.dut_list = dut_list;
				}
				sdn_profile.sdn_access_rl = [];
				if($(_obj).find("#access_intranet").hasClass("on")){
					sdn_profile.sdn_access_rl.push({"access_sdn_idx": "0", "sdn_idx": sdn_profile.sdn_rl.idx});
				}

				sdn_profile.sdn_rl.vpnc_idx = "0";
				sdn_profile.sdn_rl.vpns_idx = "0";
				if($(_obj).find("#vpn_enabled").hasClass("on")){
					var $selected_vpn = $(_obj).find("[data-container=VPN_Profiles] .rwd_icon_radio.clicked");
					if($selected_vpn.length){
						var vpn_type = $selected_vpn.attr("data-vpn-type");
						var vpn_idx = $selected_vpn.attr("data-idx");
						if(vpn_type == "vpnc"){
							sdn_profile.sdn_rl.vpnc_idx = vpn_idx;
						}
						else if(vpn_type == "vpns"){
							sdn_profile.sdn_rl.vpns_idx = vpn_idx;
						}
					}
				}
				sdn_profile.sdn_rl.cp_idx = "0";
				var portal_type = $(_obj).find("#select_portal_type").children(".selected").attr("value");
				if(portal_type != undefined)
					sdn_profile.sdn_rl.cp_idx = portal_type;
				if(sdn_profile.sdn_rl.cp_idx == "2" || sdn_profile.sdn_rl.cp_idx == "4"){
					if($.isEmptyObject(sdn_profile.cp_rl)){
						var idx_for_customized_ui = "";
						sdn_profile.cp_rl = JSON.parse(JSON.stringify(new cp_profile_attr()));
						var specific_cp_type_rl = cp_type_rl_json.filter(function(item, index, array){
							return (item.cp_idx == sdn_profile.sdn_rl.cp_idx);
						})[0];
						if(specific_cp_type_rl != undefined){
							if(specific_cp_type_rl.profile[0] != undefined){
								idx_for_customized_ui = specific_cp_type_rl.profile[0].idx_for_customized_ui;
							}
						}
						if(idx_for_customized_ui == ""){
							idx_for_customized_ui = $.now();
						}
						sdn_profile.cp_rl.idx_for_customized_ui = idx_for_customized_ui;
					}
					sdn_profile.cp_rl.cp_idx = sdn_profile.sdn_rl.cp_idx;
					var cp_idx = sdn_profile.cp_rl.cp_idx;
					sdn_profile.cp_rl.enable = "1";
					sdn_profile.cp_rl.conntimeout = ((cp_idx == "2") ? (($(_obj).find("#FWF_conntimeout").val())*60).toString() : 60*60);
					sdn_profile.cp_rl.redirecturl = ((cp_idx == "2") ? $(_obj).find("#FWF_redirecturl").val() : "");
					sdn_profile.cp_rl.auth_type = ((cp_idx == "2") ? (($(_obj).find("#FWF_passcode_enabled").hasClass("on")) ? "1" : "0") : "0");
					sdn_profile.cp_rl.term_of_service = ((cp_idx == "2") ? (($(_obj).find("#FWF_terms_service_enabled").hasClass("on")) ? "1" : "0") : "0");
					sdn_profile.cp_rl.NAS_ID = "";
					sdn_profile.cp_rl.local_auth_profile = ((cp_idx == "2") ? (($(_obj).find("#FWF_passcode_enabled").hasClass("on")) ? $(_obj).find("#FWF_passcode").val() : "") : "");
					var upload_data = {"id": sdn_profile.cp_rl.idx_for_customized_ui, "cp_idx": cp_idx, "auth_type": sdn_profile.cp_rl.auth_type};
					if(cp_idx == "2"){
						upload_data["brand_name"] = $("#FWF_brand_name").val();
						upload_data["image"] = $(_obj).find("#FWF_ui_container [data-component=FWF_bg]").css('background-image').replace('url(','').replace(')','').replace(/\"/gi, "");
						upload_data["terms_service"] = $("#FWF_terms_service").val();
						upload_data["terms_service_enabled"] = sdn_profile.cp_rl.term_of_service;
					}
					else if(cp_idx == "4"){
						upload_data["MB_desc"] = $("#MB_desc").val();
						upload_data["image"] = $(_obj).find("#MB_ui_container [data-component=MB_bg]").css('background-image').replace('url(','').replace(')','').replace(/\"/gi, "");
					}
					uploadFreeWiFi(upload_data);
					rc_append += "restart_chilli;restart_uam_srv;";
				}
				if($(_obj).find("#container_dns").length > 0){
					sdn_profile.subnet_rl.dns = $(_obj).find("#dns").attr("data-dns1") + "," + $(_obj).find("#dns").attr("data-dns2");
				}

				if($(_obj).find("#dhcp_enable").length > 0){
					if($(_obj).find("#dhcp_enable").hasClass("on"))
						sdn_profile.subnet_rl.dhcp_enable = "1";
					else
						sdn_profile.subnet_rl.dhcp_enable = "0";
				}
				var $ipaddr = $(_obj).find("#ipaddr");
				if($ipaddr.length > 0){
					sdn_profile.subnet_rl.addr = $ipaddr.val();
					sdn_profile.subnet_rl.netmask = $(_obj).find("#select_netmask").children(".selected").attr("value");
					const ip_range = calculatorIPPoolRange(sdn_profile.subnet_rl.addr, sdn_profile.subnet_rl.netmask);
					sdn_profile.subnet_rl.dhcp_min = ip_range.start;
					sdn_profile.subnet_rl.dhcp_max = ip_range.end;
				}
				if($(_obj).find("#dhcp_lease").length > 0){
					sdn_profile.subnet_rl.dhcp_lease = $(_obj).find("#dhcp_lease").val();
				}
				if($(_obj).find("#vlan_id").length > 0){
					
					//update vlan_trunklist
					if( sdn_profile.vlan_rl.vid > 1){	//vid :2~4093
						var updated_vlan_trunklist = update_vlan_trunklist( sdn_profile.vlan_rl, $(_obj).find("#vlan_id").val());
						nvramSet_obj.vlan_trunklist = updated_vlan_trunklist;
					}

					//update vlan_rl
					sdn_profile.vlan_rl.vid = $(_obj).find("#vlan_id").val();
				}

				if(rc_append != ""){
					nvramSet_obj.rc_service = nvramSet_obj.rc_service + rc_append;
				}
				var showLoading_status = get_showLoading_status(nvramSet_obj.rc_service);
				const parent_cntr = (typeof PAGE_CONTAINER == "string") ? PAGE_CONTAINER : "sdn";
				if(wifi_band > 0){
					if(sdn_profile.apg_rl.timesched == "2"){
						sdn_profile.apg_rl.expiretime = accesstime_handle_data.json_array_to_string(sdn_schedule.Get_Value_AccessTime(showLoading_status.time));
					}
				}
				if(isNewProfile)
					sdn_all_rl_json.push(JSON.parse(JSON.stringify(sdn_profile)));
				var sdn_all_list = parse_JSONToStr_sdn_all_list();
				$.extend(nvramSet_obj, sdn_all_list);
				var apgX_rl = parse_apg_rl_to_apgX_rl(sdn_profile.apg_rl);
				$.extend(nvramSet_obj, apgX_rl);
				if(sdn_profile.sdn_rl.cp_idx == "2" || sdn_profile.sdn_rl.cp_idx == "4"){
					var cpX_rl = parse_cp_rl_to_cpX_rl(sdn_profile.cp_rl);
					$.extend(nvramSet_obj, cpX_rl);
				}
				if(!httpApi.app_dataHandler){
					showLoading();
					close_popup_container("all");
					if(isWLclient()){
						showLoading(showLoading_status.time);
						setTimeout(function(){
							showWlHintContainer();
						}, showLoading_status.time*1000);
						check_isAlive_and_redirect({"page": "" + ((parent_cntr == "mlo") ? "MLO" : "SDN") + ".asp", "time": showLoading_status.time});
					}
				}
				httpApi.nvramSet(nvramSet_obj, function(){
					if(isWLclient()) return;
					showLoading(showLoading_status.time);
					setTimeout(function(){
						init_sdn_all_list();
						if(parent_cntr == "mlo")
							show_mlo_profilelist();
						else
							show_sdn_profilelist();
						if(!window.matchMedia('(max-width: 575px)').matches)
							$("#profile_list_content").find("[sdn_idx=" + selected_sdn_idx + "] .item_text_container").click();
					}, showLoading_status.time*1000);
					if(!isMobile()){
						if(showLoading_status.disconnect){
							check_isAlive_and_redirect({"page": "" + ((parent_cntr == "mlo") ? "MLO" : "SDN") + ".asp", "time": showLoading_status.time});
						}
					}
				});
			}
		});
	}
}
function validate_isBlank(_obj){
	var isBlank = false;
	$(_obj).find("[need_check=true]").each(function(index){
		if($(this).closest(".profile_setting_item").css("display") == "none")
			return true;

		var value = $(this).val().replace(/\s+/g, '');//remove space
		if(value == ""){
			isBlank = true;
			return false;
		}
	});

	if($(_obj).find("#select_wifi_band").children(".selected").attr("value") != "0"){
		if($(_obj).find("[data-category-cntr=aimesh]").css("display") != "none"){
			if($(_obj).find("[data-container=AiMesh_List] .node_container .icon_checkbox").length > 0){
				if(!$(_obj).find("[data-container=AiMesh_List] .node_container .icon_checkbox:not(.disabled)").hasClass("clicked")){
					isBlank = true;
				}
			}
		}
	}

	if(isBlank)
		return true;
	else
		return false;
}
function validate_format_Wizard_Item(_obj, _type){
	$(_obj).find(".validate_hint").remove();
	var valid_block_chars = function(str, keywordArray){
		var testResult = {
			'isError': false,
			'errReason': ''
		};

		// bolck ascii code 32~126 first
		var invalid_char = "";
		for(var i = 0; i < str.length; ++i){
			if(str.charCodeAt(i) < '32' || str.charCodeAt(i) > '126'){
				invalid_char += str.charAt(i);
			}
		}
		if(invalid_char != ""){
			testResult.isError = true;
			testResult.errReason = '<#JS_validstr2#>" '+ invalid_char +'" !';
			return testResult;
		}

		// check if char in the specified array
		if(str){
			for(var i=0; i<keywordArray.length; i++){
				if(str.indexOf(keywordArray[i]) >= 0){
					testResult.isError = true;
					testResult.errReason = keywordArray + " <#JS_invalid_chars#>";
					return testResult;
				}
			}
		}

		return testResult;
	};
	var valid_psk = function(str){
		var testResult = {
			'isError': false,
			'errReason': '',
			'set_value': ''
		};
		var psk_length = str.length;
		var psk_length_trim = str.trim().length;
		if(psk_length < 8){
			testResult.isError = true;
			testResult.errReason = "<#JS_passzero#>";
			testResult.set_value = "00000000";
			return testResult;
		}
		if(psk_length > 64){
			testResult.isError = true;
			testResult.errReason = "<#JS_PSK64Hex#>";
			return testResult;
		}
		if(psk_length != psk_length_trim){
			testResult.isError = true;
			testResult.errReason = stringSafeGet("<#JS_PSK64Hex_whiteSpace#>");
			return testResult;
		}
		if(psk_length >= 8 && psk_length <= 63){
			if(str.charAt(0) == '"'){
				testResult.isError = true;
				testResult.errReason = stringSafeGet(`<#JS_validstr1#> ["]`);
				return testResult;
			}
			if(str.charAt(psk_length - 1) == '"'){
				testResult.isError = true;
				testResult.errReason = stringSafeGet(`<#JS_validstr3#> ["]`);
				return testResult;
			}
			let is_valid_chars = valid_block_chars(str, []);
			if(is_valid_chars.isError){
				testResult.isError = true;
				testResult.errReason = stringSafeGet(is_valid_chars.errReason);
				return testResult;
			}
		}
		if(psk_length == 64 && !check_is_hex(str)){
			testResult.isError = true;
			testResult.errReason = "<#JS_PSK64Hex#>";
			return testResult;
		}
		return testResult;
	};
	var valid_psk_KR = function(str){
		var testResult = {
			'isError': false,
			'errReason': ''
		};
		const psk_length = str.length;
		const psk_length_trim = str.trim().length;
		if(!/[A-Za-z]/.test(str) || !/[0-9]/.test(str) || psk_length < 8 || psk_length > 63 
				|| !/[\!\"\#\$\%\&\'\(\)\*\+\,\-\.\/\:\;\<\=\>\?\@\[\\\]\^\_\`\{\|\}\~]/.test(str)){
			testResult.isError = true;
			testResult.errReason = "<#JS_PSK64Hex_kr#> <#JS_validPWD#>";
			return testResult;
		}
		if(psk_length != psk_length_trim){
			testResult.isError = true;
			testResult.errReason = stringSafeGet("<#JS_PSK64Hex_whiteSpace#>");
			return testResult;
		}
		if(str.charAt(0) == '"'){
			testResult.isError = true;
			testResult.errReason = stringSafeGet(`<#JS_validstr1#> ["]`);
			return testResult;
		}
		if(str.charAt(psk_length - 1) == '"'){
			testResult.isError = true;
			testResult.errReason = stringSafeGet(`<#JS_validstr3#> ["]`);
			return testResult;
		}
		let is_valid_chars = valid_block_chars(str, []);
		if(is_valid_chars.isError){
			testResult.isError = true;
			testResult.errReason = stringSafeGet(is_valid_chars.errReason);
			return testResult;
		}
		return testResult;
	};
	var check_is_hex = function(str){
		var re = new RegExp("[^a-fA-F0-9]+","gi");
		if(re.test(str))
			return false;
		else
			return true;
	};
	var valid_SSID = function(str){
		var testResult = {
			'isError': false,
			'errReason': '',
			'set_value': ''
		};
		var c;
		var ssid = str;

		len = validator.lengthInUtf8(ssid);
		if(len > 32){
			testResult.isError = true;
			testResult.errReason = "<#JS_max_ssid#>";
			return testResult;
		}

		for(var i = 0; i < len; ++i){
			c = ssid.charCodeAt(i);
			if(!isSupport("utf8_ssid")){
				if(validator.ssidChar(c)){
					testResult.isError = true;
					testResult.errReason = '<#JS_validSSID1#> ' + ssid.charAt(i) + ' <#JS_validSSID2#>';
					return testResult;
				}
			}
		}

		return testResult;
	};
	var valid_isLegalIP = function(str){
		var testResult = {
			'isError': false,
			'errReason': ''
		};
		var A_class_start = inet_network("1.0.0.0");
		var A_class_end = inet_network("126.255.255.255");
		var B_class_start = inet_network("127.0.0.0");
		var B_class_end = inet_network("127.255.255.255");
		var C_class_start = inet_network("128.0.0.0");
		var C_class_end = inet_network("255.255.255.255");
		var ip_num = inet_network(str);
		if(ip_num > A_class_start && ip_num < A_class_end){
			return testResult;
		}
		else if(ip_num > B_class_start && ip_num < B_class_end){
			testResult.isError = true;
			testResult.errReason = str + " <#JS_validip#>";
			return testResult;
		}
		else if(ip_num > C_class_start && ip_num < C_class_end){
			return testResult;
		}
		else{
			testResult.isError = true;
			testResult.errReason = str + " <#JS_validip#>";
			return testResult;
		}
	};
	var valid_IP_CIDR = function(addr, type, mode){
		//mode, 0:IP, 1:IP/CIDR, 2:IP or IP/CIDR
		var testResultPass = {
			'isError': false,
			'errReason': ''
		};
		var testResultFail = {
			'isError': true,
			'errReason': addr + " <#JS_validip#>"
		};
		var IP = new RegExp(ip_RegExp[type],"gi");
		var IP_CIDR = new RegExp(ip_RegExp[type + "_CIDR"], "gi");
		if(mode == "0"){
			if(IP.test(addr))
				return testResultPass;
			else{
				testResultFail.errReason = testResultFail.errReason + ", IP Address without CIDR."
				return testResultFail;
			}
		}
		else if(mode == "1"){
			if(IP_CIDR.test(addr))
				return testResultPass;
			else{
				testResultFail.errReason = testResultFail.errReason + ", IP Address/CIDR"
				return testResultFail;
			}
		}
		else if(mode == "2"){
			if(IP_CIDR.test(addr) || IP.test(addr))
				return testResultPass;
			else{
				testResultFail.errReason = testResultFail.errReason + ", IP Address without CIDR or IP Address/CIDR."
				return testResultFail;
			}
		}
		else
			return testResultFail;
	};
	var valid_is_IP_format = function(str, type){
		var testResultPass = {
			'isError': false,
			'errReason': ''
		};
		var testResultFail = {
			'isError': true,
			'errReason': str + " <#JS_validip#>"
		};
		var format = new RegExp(ip_RegExp[type], "gi");
		if(format.test(str))
			return testResultPass;
		else
			return testResultFail;
	};
	var valid_isLegalMask = function(str){
		var testResult = {
			'isError': false,
			'errReason': ''
		};
		var wrong_netmask = 0;
		var netmask_num = inet_network(str);
		var netmask_reverse_num = 0;
		var test_num = 0;
		if(netmask_num != -1) {
			if(netmask_num == 0) {
				netmask_reverse_num = 0; //Viz 2011.07 : Let netmask 0.0.0.0 pass
			}
			else {
				netmask_reverse_num = ~netmask_num;
			}

			if(netmask_num < 0) {
				wrong_netmask = 1;
			}

			test_num = netmask_reverse_num;
			while(test_num != 0){
				if((test_num + 1) % 2 == 0) {
					test_num = (test_num + 1) / 2 - 1;
				}
				else{
					wrong_netmask = 1;
					break;
				}
			}
			if(wrong_netmask == 1){
				testResult.isError = true;
				testResult.errReason = str + " is not a valid Mask address!";
				return testResult;
			}
			else {
				return testResult;
			}
		}
		else { //null
			testResult.isError = true;
			testResult.errReason = "This is not a valid Mask address!";
			return testResult;
		}
	};
	var valid_bandwidth = function(str){
		var testResult = {
			'isError': false,
			'errReason': ''
		};
		if(!$.isNumeric(str)){
			testResult.isError = true;
			testResult.errReason = "<#QoS_invalid_period#>";
			return testResult;
		}
		if(str.split(".").length > 2 || parseFloat(str) < 0.1){
			testResult.isError = true;
			testResult.errReason = "<#min_bound#> : 0.1 Mb/s";
			return testResult;
		}
		return testResult;
	};
	var validate_dhcp_range = function(str, ipaddr, netmask){
		var testResult = {
			'isError': false,
			'errReason': ''
		};
		var ip_num = inet_network(str);
		var subnet_head, subnet_end;
		if(ip_num <= 0){
			testResult.isError = true;
			testResult.errReason = str + " <#JS_validip#>";
			return testResult;
		}
		subnet_head = getSubnet(ipaddr, netmask, "head");
		subnet_end = getSubnet(ipaddr, netmask, "end");
		if(ip_num <= subnet_head || ip_num >= subnet_end){
			testResult.isError = true;
			testResult.errReason = str + " <#JS_validip#>";
			return testResult;
		}
		return testResult;
	};
	var valid_num_range = function(str, mini, maxi){
		var testResult = {
			'isError': true,
			'errReason': '<#JS_validrange#> ' + mini + ' <#JS_validrange_to#> ' + maxi
		};
		if(isNaN(str))
			return testResult;
		else{
			var input_num = parseInt(str);
			var mini_num = parseInt(mini);
			var maxi_num = parseInt(maxi);
			if(input_num < mini_num || input_num > maxi_num)
				return testResult;
			else{
				testResult.isError = false;
				testResult.errReason = "";
				return testResult;
			}
		}
	};
	var valid_url = function(str){
		var testResult = {
			'isError': false,
			'errReason': ''
		};

		var urlregex = new RegExp("^(http|https|ftp)\://([a-zA-Z0-9\.\-]+(\:[a-zA-Z0-9\.&amp;%\$\-]+)*@)*((25[0-5]|2[0-4][0-9]|[0-1]{1}[0-9]{2}|[1-9]{1}[0-9]{1}|[1-9])\.(25[0-5]|2[0-4][0-9]|[0-1]{1}[0-9]{2}|[1-9]{1}[0-9]{1}|[1-9]|0)\.(25[0-5]|2[0-4][0-9]|[0-1]{1}[0-9]{2}|[1-9]{1}[0-9]{1}|[1-9]|0)\.(25[0-5]|2[0-4][0-9]|[0-1]{1}[0-9]{2}|[1-9]{1}[0-9]{1}|[0-9])|([a-zA-Z0-9\-]+\.)*[a-zA-Z0-9\-]+\.(com|edu|gov|int|mil|net|org|biz|arpa|info|name|pro|aero|coop|museum|[a-zA-Z]{2}))(\:[0-9]+)*(/($|[a-zA-Z0-9\.\,\?\'\\\+&amp;%\$#\=~_\-]+))*$");
		if(urlregex.test(str)){
			return testResult;
		}
		else{
			testResult.isError = true;
			testResult.errReason = "<#feedback_format_alert#> ex. http or https ://www.asus.com";
			return testResult;
		}
	};
	var isSku = function(_ptn){
		var ttc = httpApi.nvramGet(["territory_code"]).territory_code;
		return (ttc.search(_ptn) == -1) ? false : true;
	}

	var sdn_idx = $(_obj).attr("sdn_idx");
	var $sdn_name = $(_obj).find("#sdn_name");
	if($sdn_name.val() == ""){
		$sdn_name.show_validate_hint("<#JS_fieldblank#>");
		$sdn_name.focus();
		return false;
	}
	var isValid_sdn_name = valid_SSID($sdn_name.val());
	if(isValid_sdn_name.isError){
		$sdn_name.show_validate_hint(isValid_sdn_name.errReason);
		$sdn_name.focus();
		return false;
	}
	var specific_sdn_name = "";
	if(sdn_idx != undefined){
		specific_sdn_name = sdn_all_rl_json.filter(function(item, index, array){
			return (item.sdn_rl.idx != sdn_idx && item.apg_rl.ssid == $sdn_name.val());
		});
	}
	else{
		specific_sdn_name = sdn_all_rl_json.filter(function(item, index, array){
			return (item.apg_rl.ssid == $sdn_name.val());
		});
	}
	if(specific_sdn_name.length > 0){
		$sdn_name.show_validate_hint("<#JS_duplicate#>");
		$sdn_name.focus();
		return false;
	}

	var $sdn_pwd = $(_obj).find("#sdn_pwd");
	var $sdn_pwd_cntr = $sdn_pwd.closest(".profile_setting_item");
	if($sdn_pwd_cntr.length > 0 && $sdn_pwd_cntr.css("display") != "none"){
		$sdn_pwd.val($sdn_pwd.val().replace(/\s+/g, ''));//remove space
		if($sdn_pwd.val() == ""){
			$sdn_pwd.show_validate_hint("<#JS_fieldblank#>");
			$sdn_pwd.focus();
			return false;
		}
		var isValid_pwd = valid_block_chars($sdn_pwd.val(), ["<", ">"]);
		if(isValid_pwd.isError){
			$sdn_pwd.show_validate_hint(isValid_pwd.errReason);
			$sdn_pwd.focus();
			return false;
		}
		if(isSku("KR")){
			isValid_pwd = valid_psk_KR($sdn_pwd.val());
			if(isValid_pwd.isError){
				$sdn_pwd.show_validate_hint(isValid_pwd.errReason);
				$sdn_pwd.focus();
				return false;
			}
		}
		else{
			isValid_pwd = valid_psk($sdn_pwd.val());
			if(isValid_pwd.isError){
				$sdn_pwd.show_validate_hint(isValid_pwd.errReason);
				$sdn_pwd.focus();
				if(isValid_pwd.set_value)
					$sdn_pwd.val(isValid_pwd.set_value);
				return false;
			}
		}
		//confirm common string combination	#JS_common_passwd#
		var is_common_string = check_common_string($sdn_pwd.val(), "wpa_key");
		if(is_common_string){
			if(!confirm("<#JS_common_passwd#>")){
				$sdn_pwd.focus();
				return false;
			}
		}
	}

	var $radius_ipaddr = $(_obj).find("#radius_ipaddr");
	if($radius_ipaddr.length != 0){
		if($radius_ipaddr.closest(".profile_setting_item").css("display") != "none"){
			$radius_ipaddr.val($radius_ipaddr.val().replace(/\s+/g, ''));//remove space
			if(isSwMode("rt") && isSupport("ipv6")){
				var isValid_ipv4 = valid_is_IP_format($radius_ipaddr.val(), "IPv4");
				var isValid_ipv6 = valid_is_IP_format($radius_ipaddr.val(), "IPv6");
				if(isValid_ipv4.isError && isValid_ipv6.isError){
					$radius_ipaddr.show_validate_hint(($radius_ipaddr.val() + " <#JS_validip#>"));
					$radius_ipaddr.focus();
					return false;
				}
			}
			else{
				var isValid_ipv4 = valid_is_IP_format($radius_ipaddr.val(), "IPv4");
				if(isValid_ipv4.isError){
					$radius_ipaddr.show_validate_hint(($radius_ipaddr.val() + " <#JS_validip#>"));
					$radius_ipaddr.focus();
					return false;
				}
			}
		}
	}

	var $radius_port = $(_obj).find("#radius_port");
	if($radius_port.length != 0){
		if($radius_port.closest(".profile_setting_item").css("display") != "none"){
			$radius_port.val($radius_port.val().replace(/\s+/g, ''));//remove space
			if($radius_port.val() == ""){
				$radius_port.show_validate_hint("<#JS_fieldblank#>");
				$radius_port.focus();
				return false;
			}
			var isValid_radius_port = valid_num_range($radius_port.val(), 0, 65535);
			if(isValid_radius_port.isError){
				$radius_port.show_validate_hint(isValid_radius_port.errReason);
				$radius_port.focus();
				return false;
			}
		}
	}

	var $radius_key = $(_obj).find("#radius_key");
	if($radius_key.length != 0){
		if($radius_key.closest(".profile_setting_item").css("display") != "none"){
			var isValid_radius_key = valid_block_chars($radius_key.val(), ["\""]);
			if(isValid_radius_key.isError){
				$radius_key.show_validate_hint(isValid_radius_key.errReason);
				$radius_key.focus();
				return false;
			}
		}
	}

	var $bw_enabled = $(_obj).find("#bw_enabled");
	if($bw_enabled.length != 0){
		if($bw_enabled.closest(".profile_setting_item").css("display") != "none"){
			var $bw_dl = $(_obj).find("#bw_dl");
			var $bw_ul = $(_obj).find("#bw_ul");
			$bw_dl.val($bw_dl.val().replace(/\s+/g, ''));//remove space
			$bw_ul.val($bw_ul.val().replace(/\s+/g, ''));//remove space
			if($bw_enabled.hasClass("on")){
				if($bw_dl.val() == ""){
					$bw_dl.show_validate_hint("<#JS_fieldblank#>");
					$bw_dl.focus();
					return false;
				}
				else{
					var isValid_bw_dl = valid_bandwidth($bw_dl.val());
					if(isValid_bw_dl.isError){
						$bw_dl.show_validate_hint(isValid_bw_dl.errReason);
						$bw_dl.focus();
						return false;
					}
				}
				if($bw_ul.val() == ""){
					$bw_ul.show_validate_hint("<#JS_fieldblank#>");
					$bw_ul.focus();
					return false;
				}
				else{
					var isValid_bw_ul = valid_bandwidth($bw_ul.val());
					if(isValid_bw_ul.isError){
						$bw_ul.show_validate_hint(isValid_bw_ul.errReason);
						$bw_ul.focus();
						return false;
					}
				}
			}
		}
	}
	if(_type == "ALL"){
		if(sdn_idx != undefined){
			var $ipaddr = $(_obj).find("#ipaddr");
			if($ipaddr.length != 0){
				$ipaddr.val($ipaddr.val().replace(/\s+/g, ''));//remove space
				if($ipaddr.val() == ""){
					$ipaddr.show_validate_hint("<#JS_fieldblank#>");
					$ipaddr.focus();
					return false;
				}
				var isValid_ipaddr = valid_isLegalIP($ipaddr.val());
				if(isValid_ipaddr.isError){
					$ipaddr.show_validate_hint(isValid_ipaddr.errReason);
					$ipaddr.focus();
					return false;
				}
				var lan_ipaddr = httpApi.nvramGet(["lan_ipaddr"]).lan_ipaddr;
				if($ipaddr.val() == lan_ipaddr){
					$ipaddr.show_validate_hint("<#JS_conflict_LANIP#>");
					$ipaddr.focus();
					return false;
				}
				var ipaddr_substr = $ipaddr.val().substr(0, $ipaddr.val().lastIndexOf("."));
				var wan0_ipaddr_substr = httpApi.nvramGet(["wan0_ipaddr"]).wan0_ipaddr.substr(0, httpApi.nvramGet(["wan0_ipaddr"]).wan0_ipaddr.lastIndexOf("."));
				var wan1_ipaddr_substr = httpApi.nvramGet(["wan1_ipaddr"]).wan1_ipaddr.substr(0, httpApi.nvramGet(["wan1_ipaddr"]).wan1_ipaddr.lastIndexOf("."));
				if((wan0_ipaddr_substr == ipaddr_substr) || (wan1_ipaddr_substr == ipaddr_substr)){
					$ipaddr.show_validate_hint("<#JS_conflict_LANIP#>".replace("LAN", "WAN"));
					$ipaddr.focus();
					return false;
				}
				var specific_data = sdn_all_rl_json.filter(function(item, index, array){
					if(item.sdn_rl.idx != sdn_idx && !$.isEmptyObject(item.subnet_rl)){
						var compare_ipaddr = item.subnet_rl.addr.substr(0, item.subnet_rl.addr.lastIndexOf("."));
						return (compare_ipaddr == ipaddr_substr);
					}
				})[0];
				var lan_ipaddr_substr = lan_ipaddr.substr(0, lan_ipaddr.lastIndexOf("."));
				if(specific_data != undefined || (lan_ipaddr_substr == ipaddr_substr)){
					$ipaddr.show_validate_hint("<#vpn_openvpn_conflict#>");
					$ipaddr.focus();
					return false;
				}
			}

			const $dhcp_lease = $(_obj).find("#dhcp_lease");
			if($dhcp_lease.length != 0){
				$dhcp_lease.val($dhcp_lease.val().replace(/\s+/g, ''));//remove space
				if($dhcp_lease.val() == ""){
					$dhcp_lease.show_validate_hint("<#JS_fieldblank#>");
					$dhcp_lease.focus();
					return false;
				}
				var isValid_dhcp_lease = valid_num_range($dhcp_lease.val(), 120, 604800);
				if(isValid_dhcp_lease.isError){
					$dhcp_lease.show_validate_hint(isValid_dhcp_lease.errReason);
					$dhcp_lease.focus();
					return false;
				}
			}

			var $vlan_id = $(_obj).find("#vlan_id");
			if($vlan_id.length != 0){
				$vlan_id.val($vlan_id.val().replace(/\s+/g, ''));//remove space
				if($vlan_id.val() == ""){
					$vlan_id.show_validate_hint("<#JS_fieldblank#>");
					$vlan_id.focus();
					return false;
				}
				var isValid_vlan_id = valid_num_range($vlan_id.val(), 1, 4093);
				if(isValid_vlan_id.isError){
					$vlan_id.show_validate_hint(isValid_vlan_id.errReason);
					$vlan_id.focus();
					return false;
				}
				var specific_data = sdn_all_rl_json.filter(function(item, index, array){
					return (item.sdn_rl.idx != sdn_idx && item.vlan_rl.vid == $vlan_id.val());
				})[0];
				var specific_sdn_rl = sdn_all_rl_json.filter(function(item, index, array){
					return (item.sdn_rl.idx == sdn_idx);
				})[0];
				var specific_vlan_rl = vlan_rl_json.filter(function(item, index, array){
					return (item.vlan_idx != specific_sdn_rl.sdn_rl.vlan_idx && item.vid == $vlan_id.val());
				})[0];
				if(specific_data != undefined || specific_vlan_rl != undefined){
					$vlan_id.show_validate_hint("<#JS_duplicate#>");
					$vlan_id.focus();
					return false;
				}
			}

			var portal_type = $(_obj).find("#select_portal_type").children(".selected").attr("value");
			if(portal_type == "2"){
				var $FWF_conntimeout = $(_obj).find("#FWF_conntimeout");
				$FWF_conntimeout.val($FWF_conntimeout.val().replace(/\s+/g, ''));//remove space
				if($FWF_conntimeout.val() == ""){
					$FWF_conntimeout.show_validate_hint("<#JS_fieldblank#>");
					$FWF_conntimeout.focus();
					return false;
				}
				var isValid_FWF_conntimeout = valid_num_range($FWF_conntimeout.val(), 1, 999);
				if(isValid_FWF_conntimeout.isError){
					$FWF_conntimeout.show_validate_hint(isValid_FWF_conntimeout.errReason);
					$FWF_conntimeout.focus();
					return false;
				}

				var $FWF_redirecturl = $(_obj).find("#FWF_redirecturl");
				$FWF_redirecturl.val($FWF_redirecturl.val().replace(/\s+/g, ''));//remove space
				if($FWF_redirecturl.val() != ""){
					var isValid_FWF_redirecturl = valid_url($FWF_redirecturl.val());
					if(isValid_FWF_redirecturl.isError){
						$FWF_redirecturl.show_validate_hint(isValid_FWF_redirecturl.errReason);
						$FWF_redirecturl.focus();
						return false;
					}
				}

				var $FWF_passcode_enabled = $(_obj).find("#FWF_passcode_enabled");
				if($FWF_passcode_enabled.hasClass("on")){
					var $FWF_passcode = $(_obj).find("#FWF_passcode");
					$FWF_passcode.val($FWF_passcode.val().replace(/\s+/g, ''));//remove space
					if($FWF_passcode.val() == ""){
						$FWF_passcode.show_validate_hint("<#JS_fieldblank#>");
						$FWF_passcode.focus();
						return false;
					}
					var isValid_passcode = valid_block_chars($FWF_passcode.val(), ["<", ">"]);
					if(isValid_passcode.isError){
						$FWF_passcode.show_validate_hint(isValid_passcode.errReason);
						$FWF_passcode.focus();
						return false;
					}
					//confirm common string combination	#JS_common_passwd#
					var is_common_string = check_common_string($FWF_passcode.val(), "httpd_password");
					if(is_common_string){
						if(!confirm("<#JS_common_passwd#>")){
							$FWF_passcode.focus();
							return false;
						}
					}
				}
			}
		}
	}
	return true;
}
function show_sdn_profilelist(){
	//$("#profile_list_num").html(htmlEnDeCode.htmlEncode("" + (Object.keys(sdn_all_rl_json).length - 1) + "/" + sdn_maximum));
	$("#profile_list_content").empty();
	let display_sdn_list = [];
	$.each(sdn_all_rl_json, function(index, sdn_all_rl){
		if(sdn_all_rl.sdn_rl.idx == "0")
			return true;
		if(check_is_mlo_bh(sdn_all_rl.apg_rl))
			return true;
		var item = {
			"sdn_idx": sdn_all_rl.sdn_rl.idx,
			"type": sdn_all_rl.sdn_rl.sdn_name,
			"name": sdn_all_rl.apg_rl.ssid,
			"activate": sdn_all_rl.sdn_rl.sdn_enable,
			"client_num": sdn_all_rl.client_num
		};
		$("#profile_list_content").append(Get_Component_Profile_Item(item));
		display_sdn_list.push(sdn_all_rl)
	});
	if(display_sdn_list.length > 0)
		$("#profile_list_content").append($("<div>").addClass("horizontal_line"));

	var $AddNew_item = Get_Component_Profile_Item_AddNew().appendTo($("#profile_list_content"));
	if($(".profile_setting_container").css("display") != "none"){
		if(selected_sdn_idx == ""){
			$AddNew_item.click();
			if(display_sdn_list.length <= 0){
				$(".popup_element").css("display", "flex");
				$(".container").addClass("blur_effect");
				$(".popup_container.popup_element").empty()
					.append(show_Get_Start("popup"))
					.addClass(function(){
						return "hide_title_cntr" + ((isMobile()) ? " full_width" : "");
					})
					.find(".popup_title_container").hide();
				resize_iframe_height();
			}
		}
		else{
			$("#profile_list_content").find("[sdn_idx="+selected_sdn_idx+"] .item_text_container").click();
		}
	}
	else{
		$(".profile_setting_container").empty().append(show_Get_Start());
		if((window.matchMedia('(max-width: 575px)').matches)){//for mobile
			if(display_sdn_list.length <= 0){
				$AddNew_item.click();
				$(".popup_container.popup_element")
					.addClass("hide_title_cntr")
					.find(".popup_title_container").hide();
			}
		}
	}
	clearInterval(interval_AccTime);
	var $AccTime_list = $("#profile_list_content").find("[data-container=AccTime][access_time=true]");
	if($AccTime_list.length > 0){
		interval_AccTime = setInterval(function(){
			$.each($AccTime_list, function(index, item){
				var end_time = parseInt($(item).attr("end_time"));
				var cur_time = parseInt($(item).attr("cur_time")) + 1;
				var remaining_time = ((end_time - cur_time) > 0) ? (end_time - cur_time) : 0;
				var HMS = secondsToHMS(remaining_time);
				$(item).attr({"cur_time": cur_time}).html(HMS.hours + ":" + HMS.minutes + ":" + HMS.seconds);
				if(remaining_time <= 0){
					init_sdn_all_list();
					show_sdn_profilelist();
				}
			});
		},1000);
	}
}
var vpnc_profile_list = [];
/*
function Get_VPNC_Profile(){
	var vpnc_profile_attr = function(){
		this.desc = "";
		this.proto = "";
		this.vpnc_idx = "0";
		this.activate = "0";
	};
	var vpnc_clientlist = decodeURIComponent(httpApi.nvramCharToAscii(["vpnc_clientlist"]).vpnc_clientlist);
	var each_profile = vpnc_clientlist.split("<");
	$.each(each_profile, function(index, value){
		if(value != ""){
			var profile_data = value.split(">");
			var vpnc_profile = new vpnc_profile_attr();
			vpnc_profile.desc = profile_data[0];
			vpnc_profile.proto = profile_data[1];
			if(isSupport("vpn_fusion")){
				vpnc_profile.activate = profile_data[5];
				vpnc_profile.vpnc_idx = profile_data[6];
			}
			vpnc_profile_list.push(JSON.parse(JSON.stringify(vpnc_profile)));
		}
	});
}
*/

function Get_VPNC_Profile(){
        var vpnc_profile_attr = function(){
                this.desc = "";
                this.proto = "";
                this.vpnc_idx = "0";
                this.activate = "0";
        };
	var enabled_ovpn = httpApi.nvramGet(["vpn_clientx_eas"])[["vpn_clientx_eas"]];

	var vpnc_profile = new vpnc_profile_attr();

	for (var unit = 1; unit <= 5; unit++) {
		var prefix = "vpn_client" + unit + "_";
		if (httpApi.nvramGet([prefix + "addr"])[[prefix + "addr"]] != "") {
			vpnc_profile.desc = "OVPN" + unit + ": " + httpApi.nvramGet([prefix + "desc"])[[prefix + "desc"]];
			vpnc_profile.proto = "OpenVPN";
			vpnc_profile.vpnc_idx = unit + 5;	// Offset
			vpnc_profile.activate = (enabled_ovpn.indexOf("" + unit) >= 0 ? 1 : 0);
			vpnc_profile_list.push(JSON.parse(JSON.stringify(vpnc_profile)));
		}
	}

	for (unit = 1; unit <= 5; unit++) {
		prefix = "wgc" + unit + "_";
		if (httpApi.nvramGet([prefix + "ep_addr"])[[prefix + "ep_addr"]] != "") {
			vpnc_profile.desc = "WG" + unit + ": " + httpApi.nvramGet([prefix + "desc"])[[prefix + "desc"]];
			vpnc_profile.proto = "WireGuard";
			vpnc_profile.vpnc_idx = unit;
			vpnc_profile.activate = httpApi.nvramGet([prefix + "enable"])[[prefix + "enable"]];
			vpnc_profile_list.push(JSON.parse(JSON.stringify(vpnc_profile)));
		}
	}
}



let FreeWiFi_template = [];
function Get_FreeWiFi_template(){
	$.getJSON("/SDN/Captive_Portal/FreeWiFi_template.json", function(data){
		FreeWiFi_template = data;
	});
}
let MessageBoard_template = [];
function Get_MessageBoard_template(){
	$.getJSON("/SDN/Captive_Portal/MessageBoard_template.json", function(data){
		MessageBoard_template = data;
	});
}
var dns_list_data = [];
function Get_DNS_List_DB(){
	var dns_list = [];
	$.getJSON("/ajax/DNS_List.json", function(local_data){
		dns_list = Object.keys(local_data).map(function(e){
			return local_data[e];
		});
		category_DNS();

		setTimeout(function(){
			$.getJSON("https://nw-dlcdnet.asus.com/plugin/js/DNS_List.json",
				function(cloud_data){
					if(JSON.stringify(local_data) != JSON.stringify(cloud_data)){
						if(Object.keys(cloud_data).length > 0){
							dns_list = Object.keys(cloud_data).map(function(e){
								return cloud_data[e];
							});
							category_DNS();
						}
					}
				}
			);
		}, 1000);
	});

	function category_DNS(){
		dns_list_data = [];
		$.each(dns_list, function(index, item){
			if(item.FilterMode != undefined){
				if(dns_list_data[item.FilterMode] == undefined){
					dns_list_data[item.FilterMode] = [];
				}
				dns_list_data[item.FilterMode].push(item);
			}
		});
	}
}
var scenarios_list = [
	{
		"type":"Employee",
		"text":stringSafeGet("<#GuestNetwork_Employee#>"),
		"scenes":"office",
		"idx": 1
	},
	{
		"type":"Portal",
		"text":str_Scenario_Hotel,
		"scenes":"hotel",
		"idx": 4
	},
	{
		"type":"Portal",
		"text":str_Scenario_coffee,
		"scenes":"coffee_shop",
		"idx": 5
	},
	{
		"type":"Guest",
		"text":str_Scenario_Mall,
		"scenes":"shopping_mall",
		"idx": 6
	},
	{
		"type":(isSupport("BUSINESS") ? "Sched" : "Kids"),
		"text":str_Scenario_Study,
		"scenes":"remote_study",
		"idx": 9
	},
	{
		"type":(isSupport("BUSINESS") ? "Sched" : "Kids"),
		"text":str_Scenario_Classroom,
		"scenes":"classroom",
		"idx": 10
	},
	{
		"type":"IoT",
		"text":str_Scenario_smartHome,
		"scenes":"smart_home",
		"idx": 11
	},
	{
		"type":"IoT",
		"text":str_Scenario_EV_charging,
		"scenes":"ev_station",
		"idx": 13
	},
	{
		"type":"Guest",
		"text":str_Scenario_Friends,
		"scenes":"friends",
		"idx": 16
	}
];
function Get_Scenarios_List(){
	var sdn_scenarios_db_translation_mapping = [
		{tag:"#RESTAURANT",text:stringSafeGet("<#Scenario_Restaurant#>")},
		{tag:"#GYM",text:stringSafeGet("<#AiMesh_NodeLocation28#>")},
		{tag:"#CHILDREN",text:stringSafeGet("<#Scenario_Children#>")},
		{tag:"#VOICE_ASSISTANT",text:stringSafeGet("<#Scenario_Voice_assistance#>")},
		{tag:"#SURVEILLANCE_DEVICES",text:stringSafeGet("<#Scenario_Surveillance_devices#>")},
		{tag:"#WFH",text:stringSafeGet("<#Scenario_WFH#>")},
		{tag:"#BRANCH_OFFICE",text:stringSafeGet("<#Scenario_branchOffice#>")}
	];
	setTimeout(function(){
		$.getJSON("https://nw-dlcdnet.asus.com/plugin/js/extend_sdn_scenarios.json", function(data){
			$.each(data, function(index, item){
				if(item["type"] == "Kids" && isSupport("BUSINESS")){
					item["type"] = "Sched";
				}
				item["source"] = "Cloud";

				if(item.translation != "") {
					var specific_translation = sdn_scenarios_db_translation_mapping.filter(function(map_item){
						return (map_item.tag == item.translation);
					})[0];
					if(specific_translation != undefined){
						item.text = specific_translation.text;
					}
				}
			});
			if(data.length > 0){
				$.merge(scenarios_list, data);
				scenarios_list.sort(function(a, b){return parseInt(a.idx) - parseInt(b.idx);});
			}
		});
	},1000);
}

var sdn_all_rl_attr = function(){
	this.sdn_rl = {};
	this.vlan_rl = {};
	this.subnet_rl = {};
	this.radius_rl = {};
	this.apg_rl = {};
	this.cp_rl = {};
	this.sdn_access_rl = [];
};
var sdn_rl_attr = function(){
	this.idx = "0";
	this.sdn_name = "";
	this.sdn_enable = "1";
	this.vlan_idx = "0";
	this.subnet_idx = "0";
	this.apg_idx = "0";
	this.vpnc_idx = "0";
	this.vpns_idx = "0";
	this.dns_filter_idx = "0";
	this.urlf_idx = "0";
	this.nwf_idx = "0";
	this.cp_idx = "0";
	this.gre_idx = "0";
	this.firewall_idx = "0";
	this.kill_switch = "0";
	this.access_host_service = "0";
	this.wan_unit = "0";
	this.pppoe_relay = "0";
	this.wan6_unit = "0";
	this.createby = "WEB";
	this.mtwan_idx = "0";
	this.mswan_idx = "0";
};
var vlan_rl_attr = function(){
	this.vlan_idx = "";
	this.vid = "";
	this.port_isolation = "0";
};
var subnet_rl_attr = function(){
	this.subnet_idx = "";
	this.ifname = "";
	this.addr = "";
	this.netmask = "";
	this.dhcp_enable = "";
	this.dhcp_min = "";
	this.dhcp_max = "";
	this.dhcp_lease = "";
	this.domain_name = "";
	this.dns = "";
	this.wins = "";
	this.dhcp_static = "";
	this.dhcp_unit = "";
	this.ipv6_enable = "";
	this.autoconf = "";
	this.addr6 = "";
	this.dhcp6_start = "";
	this.dhcp6_end = "";
	this.dns6 = "";
	this.dot_enable = "";
	this.dot_tls = "";
};
var apg_rl_attr = function(){
	this.apg_idx = "";
	this.enable = "";
	this.ssid = "";
	this.hide_ssid = "";
	this.security = "";
	this.bw_limit = "";
	this.timesched = "";
	this.sched = "";
	this.expiretime = "";
	this.ap_isolate = "";
	this.macmode = "";
	this.mlo = "";
	this.maclist = "";
	this.iot_max_cmpt = "";
	this.dut_list = "";
};
var radius_rl_attr = function(){
	this.radius_idx = "";
	this.auth_server_1 = "";
	this.auth_port_1 = "";
	this.auth_key_1 = "";
	this.acct_server_1 = "";
	this.acct_port_1 = "";
	this.acct_key_1 = "";
	this.auth_server_2 = "";
	this.auth_port_2 = "";
	this.auth_key_2 = "";
	this.acct_server_2 = "";
	this.acct_port_2 = "";
	this.acct_key_2 = "";
};
var cp_profile_attr = function(){
	this.cp_idx = "";
	this.enable = "0";
	this.auth_type = "0";
	this.conntimeout = "0";
	this.idle_timeout = "0";
	this.auth_timeout = "0";
	this.redirecturl = "0";
	this.term_of_service = "1";
	this.bw_limit_ul = "0";
	this.bw_limit_dl = "0";
	this.NAS_ID = "0";
	this.idx_for_customized_ui = "";
	this.local_auth_profile  = "";
	this.radius_profile = "";
};
var sdn_access_rl_attr = function(){
	this.access_sdn_idx = "0";//default 0, is br0.
	this.sdn_idx = "";
};
const sdn0_rl = (function(){
	let result = "";
	let each_sdn_rl = decodeURIComponent(httpApi.nvramCharToAscii(["sdn_rl"]).sdn_rl).split("<");
	$.each(each_sdn_rl, function(index, value){
		if(value != ""){
			let profile_data = value.split(">");
			if(result == ""){
				if(profile_data[0] == "0"){
					result = "<" + value;
				}
			}
			else{
				return false;
			}
		}
	});
	if(result == ""){
		result = (httpApi.nvramDefaultGet(["sdn_rl"]).sdn_rl).replace(/&#60/g, "<").replace(/&#62/g, ">");
	}
	return result;
})();
function init_sdn_all_list(){
	if(isSwMode("rt")){
		httpApi.nvramGet(["qos_enable"], true);
	}
	var apg_wifi_sched_on = httpApi.hookGet("apg_wifi_sched_on", true);
	const get_apg_wifi7_onoff = (httpApi.hookGet("get_apg_wifi7_onoff", true) == undefined) ? [] : httpApi.hookGet("get_apg_wifi7_onoff");
	sdn_all_rl_json = [];
	cp_type_rl_json.forEach(function(item){
		item.sdn_idx = "";
		item.profile = [];
	});
	vpns_rl_json.forEach(function(item){
		item.sdn_idx = "";
	});
	var sdn_all_rl_info = httpApi.nvramCharToAscii(["sdn_rl", "vlan_rl", "subnet_rl", "radius_list", "sdn_access_rl"], true);
	var sdn_rl = decodeURIComponent(sdn_all_rl_info.sdn_rl);
	var vlan_rl = decodeURIComponent(sdn_all_rl_info.vlan_rl);
	var subnet_rl = decodeURIComponent(sdn_all_rl_info.subnet_rl);
	var radius_rl = decodeURIComponent(sdn_all_rl_info.radius_list);
	var sdn_access_rl = parse_StrToJSON_sdn_access_rl_list(decodeURIComponent(sdn_all_rl_info.sdn_access_rl));
	vlan_rl_json = parse_StrToJSON_vlan_rl_list(vlan_rl);
	var each_sdn_rl = sdn_rl.split("<");
	$.each(each_sdn_rl, function(index, value){
		if(value != ""){
			var sdn_all_rl = JSON.parse(JSON.stringify(new sdn_all_rl_attr()));
			var profile_data = value.split(">");
			var sdn_rl_profile = set_sdn_profile(profile_data);
			if(sdn_rl_profile.idx == "0")
				return;
			sdn_all_rl.sdn_rl = sdn_rl_profile;

			var specific_cp_type_rl = cp_type_rl_json.filter(function(item, index, array){
				return (item.cp_idx == sdn_all_rl.sdn_rl.cp_idx);
			})[0];
			if(specific_cp_type_rl != undefined){
				specific_cp_type_rl.sdn_idx = sdn_all_rl.sdn_rl.idx;
			}

			var specific_vpns_rl = vpns_rl_json.filter(function(item, index, array){
				return (item.vpns_idx == sdn_all_rl.sdn_rl.vpns_idx);
			})[0];
			if(specific_vpns_rl != undefined){
				specific_vpns_rl.sdn_idx = sdn_all_rl.sdn_rl.idx;
			}

			var specific_vlan = vlan_rl_json.filter(function(item, index, array){
				return (item.vlan_idx == sdn_rl_profile.vlan_idx);
			})[0];
			if(specific_vlan != undefined){
				sdn_all_rl.vlan_rl = specific_vlan;
			}

			var subnet_rl_list = parse_StrToJSON_subnet_rl_list(subnet_rl);
			var specific_subnet = subnet_rl_list.filter(function(item, index, array){
				return (item.subnet_idx == sdn_rl_profile.subnet_idx);
			})[0];
			if(specific_subnet != undefined){
				sdn_all_rl.subnet_rl = specific_subnet;
			}
			var apg_rl_list = get_apg_rl_list(sdn_rl_profile.apg_idx);
			var specific_apg = apg_rl_list.filter(function(item, index, array){
				return (item.apg_idx == sdn_rl_profile.apg_idx);
			})[0];
			if(specific_apg != undefined){
				sdn_all_rl.apg_rl = specific_apg;
				var radius_rl_list = parse_StrToJSON_radius_rl_list(radius_rl);
				var specific_radius = radius_rl_list.filter(function(item, index, array){
					return (item.radius_idx == specific_apg.apg_idx);
				})[0];
				if(specific_radius != undefined){
					sdn_all_rl.radius_rl = specific_radius;
				}
			}
			var cp_rl_list = get_cp_rl_list(sdn_rl_profile.cp_idx);
			var specific_cp = cp_rl_list.filter(function(item, index, array){
				return (item.cp_idx == sdn_rl_profile.cp_idx);
			})[0];
			if(specific_cp != undefined){
				sdn_all_rl.cp_rl = specific_cp;
				httpApi.hookGet("get_customized_attribute-" + specific_cp.idx_for_customized_ui + "", true);
			}
			var specific_acc_rl = sdn_access_rl.filter(function(item, index, array){
				return (item.sdn_idx == sdn_all_rl.sdn_rl.idx);
			});
			if(specific_acc_rl.length > 0){
				sdn_all_rl.sdn_access_rl = specific_acc_rl;
			}
			sdn_all_rl.client_num = 0;
			sdn_all_rl_json.push(sdn_all_rl);
		}
	});
	update_SDN_ClientList();
	init_aimesh_wifi_band_info();
	if(isSupport("mlo"))
		init_aimesh_wifi_mlo_info();
	$.each(cp_type_rl_json, function(index, item){
		var cp_rl_list = get_cp_rl_list(item.cp_idx);
		$.each(cp_rl_list, function(index, value){
			item.profile.push(value);
		});
	});

	function set_sdn_profile(profile_data){
		var sdn_profile = JSON.parse(JSON.stringify(new sdn_rl_attr()));
		sdn_profile.idx = profile_data[0];
		sdn_profile.sdn_name = profile_data[1];
		sdn_profile.sdn_enable = profile_data[2];
		sdn_profile.vlan_idx = profile_data[3];
		sdn_profile.subnet_idx = profile_data[4];
		sdn_profile.apg_idx = profile_data[5];
		sdn_profile.wifi_sched_on = (check_value_is_exist(apg_wifi_sched_on["apg" + profile_data[5] + ""]) ? apg_wifi_sched_on["apg" + profile_data[5] + ""] : "1");
		sdn_profile.wifi7_onoff =  (get_apg_wifi7_onoff[sdn_profile.apg_idx] == undefined) ? "0" : get_apg_wifi7_onoff[sdn_profile.apg_idx];
		sdn_profile.vpnc_idx = profile_data[6];
		sdn_profile.vpns_idx = profile_data[7];
		sdn_profile.dns_filter_idx = profile_data[8];
		sdn_profile.urlf_idx = profile_data[9];
		sdn_profile.nwf_idx = profile_data[10];
		sdn_profile.cp_idx = profile_data[11];
		sdn_profile.gre_idx = profile_data[12];
		sdn_profile.firewall_idx = profile_data[13];
		sdn_profile.kill_switch = profile_data[14];
		sdn_profile.access_host_service = profile_data[15];
		sdn_profile.wan_unit = (check_value_is_exist(profile_data[16]) ? profile_data[16] : "0");
		sdn_profile.pppoe_relay = (check_value_is_exist(profile_data[17]) ? profile_data[17] : "0");
		sdn_profile.wan6_unit = (check_value_is_exist(profile_data[18]) ? profile_data[18] : "0");
		sdn_profile.createby = (check_value_is_exist(profile_data[19]) ? profile_data[19] : "WEB");
		sdn_profile.mtwan_idx = (check_value_is_exist(profile_data[20]) ? profile_data[20] : "0");
		sdn_profile.mswan_idx = (check_value_is_exist(profile_data[21]) ? profile_data[21] : "0");
		return sdn_profile;
	}
	function parse_StrToJSON_vlan_rl_list(vlan_rl){
		var vlan_rl_list = [];
		var each_vlan_rl = vlan_rl.split("<");
		$.each(each_vlan_rl, function(index, value){
			if(value != ""){
				var profile_data = value.split(">");
				var vlan_profile = new vlan_rl_attr();
				vlan_profile.vlan_idx = profile_data[0];
				vlan_profile.vid = profile_data[1];
				vlan_profile.port_isolation = (check_value_is_exist(profile_data[2]) ? profile_data[2] : "0");
				vlan_rl_list.push(JSON.parse(JSON.stringify(vlan_profile)));
			}
		});
		vlan_rl_list.sort(function(a, b) {
			return parseInt(a.vlan_idx) - parseInt(b.vlan_idx);
		});
		return vlan_rl_list;
	}
	function parse_StrToJSON_subnet_rl_list(subnet_rl){
		var subnet_rl_list = [];
		var each_subnet_rl = subnet_rl.split("<");
		$.each(each_subnet_rl, function(index, value){
			if(value != ""){
				var profile_data = value.split(">");
				var subnet_profile = new subnet_rl_attr();
				subnet_profile.subnet_idx = profile_data[0];
				subnet_profile.ifname = profile_data[1];
				subnet_profile.addr = profile_data[2];
				subnet_profile.netmask = profile_data[3];
				subnet_profile.dhcp_enable = profile_data[4];
				subnet_profile.dhcp_min = profile_data[5];
				subnet_profile.dhcp_max = profile_data[6];
				subnet_profile.dhcp_lease = profile_data[7];
				subnet_profile.domain_name = profile_data[8];
				subnet_profile.dns = profile_data[9];
				subnet_profile.wins = profile_data[10];
				subnet_profile.dhcp_static = profile_data[11];
				subnet_profile.dhcp_unit = profile_data[12];
				subnet_profile.ipv6_enable = (check_value_is_exist(profile_data[13]) ? profile_data[13] : "");
				subnet_profile.autoconf = (check_value_is_exist(profile_data[14]) ? profile_data[14] : "");
				subnet_profile.addr6 = (check_value_is_exist(profile_data[15]) ? profile_data[15] : "");
				subnet_profile.dhcp6_start = (check_value_is_exist(profile_data[16]) ? profile_data[16] : "");
				subnet_profile.dhcp6_end = (check_value_is_exist(profile_data[17]) ? profile_data[17] : "");
				subnet_profile.dns6 = (check_value_is_exist(profile_data[18]) ? profile_data[18] : "");
				subnet_profile.dot_enable = (check_value_is_exist(profile_data[19]) ? profile_data[19] : "");
				subnet_profile.dot_tls = (check_value_is_exist(profile_data[20]) ? profile_data[20] : "");
				subnet_rl_list.push(JSON.parse(JSON.stringify(subnet_profile)));
			}
		});
		subnet_rl_list.sort(function(a, b) {
		    return parseInt(a.subnet_idx) - parseInt(b.subnet_idx);
		});
		return subnet_rl_list;
	}
	function parse_StrToJSON_radius_rl_list(radius_rl){
		var radius_rl_list = [];
		var each_radius_rl = radius_rl.split("<");
		$.each(each_radius_rl, function(index, value){
			if(value != ""){
				var profile_data = value.split(">");
				var radius_profile = new radius_rl_attr();
				radius_profile.radius_idx = profile_data[0];
				radius_profile.auth_server_1 = profile_data[1];
				radius_profile.auth_port_1 = profile_data[2];
				radius_profile.auth_key_1 = profile_data[3];
				radius_profile.acct_server_1 = profile_data[4];
				radius_profile.acct_port_1 = profile_data[5];
				radius_profile.acct_key_1 = profile_data[6];
				radius_profile.auth_server_2 = profile_data[7];
				radius_profile.auth_port_2 = profile_data[8];
				radius_profile.auth_key_2 = profile_data[9];
				radius_profile.acct_server_2 = profile_data[10];
				radius_profile.acct_port_2 = profile_data[11];
				radius_profile.acct_key_2 = profile_data[12];
				radius_rl_list.push(JSON.parse(JSON.stringify(radius_profile)));
			}
		});
		radius_rl_list.sort(function(a, b) {
			return parseInt(a.subnet_idx) - parseInt(b.subnet_idx);
		});
		return radius_rl_list;
	}
	function parse_StrToJSON_sdn_access_rl_list(sdn_access_rl){
		var sdn_access_rl_list = [];
		var each_sdn_access_rl = sdn_access_rl.split("<");
		$.each(each_sdn_access_rl, function(index, value){
			if(value != ""){
				var profile_data = value.split(">");
				var sdn_access_rl_profile = new sdn_access_rl_attr();
				sdn_access_rl_profile.access_sdn_idx = profile_data[0];
				sdn_access_rl_profile.sdn_idx = profile_data[1];
				sdn_access_rl_list.push(JSON.parse(JSON.stringify(sdn_access_rl_profile)));
			}
		});
		return sdn_access_rl_list;
	}
	function get_apg_rl_list(_apg_idx){
		var apg_rl_list = [];
		if(parseInt(_apg_idx) > 0){
			var apg_profile = new apg_rl_attr();
			let apg_info = httpApi.nvramCharToAscii(["apg" + _apg_idx + "_enable", "apg" + _apg_idx + "_ssid", "apg" + _apg_idx + "_hide_ssid", "apg" + _apg_idx + "_security",
				"apg" + _apg_idx + "_bw_limit", "apg" + _apg_idx + "_timesched", "apg" + _apg_idx + "_sched", "apg" + _apg_idx + "_expiretime","apg" + _apg_idx + "_ap_isolate",
				"apg" + _apg_idx + "_macmode", "apg" + _apg_idx + "_mlo", "apg" + _apg_idx + "_maclist", "apg" + _apg_idx + "_iot_max_cmpt", "apg" + _apg_idx + "_dut_list"], true);
			apg_profile.apg_idx = _apg_idx.toString();
			apg_profile.enable = apg_info["apg" + _apg_idx + "_enable"];
			apg_profile.ssid = decodeURIComponent(apg_info["apg" + _apg_idx + "_ssid"]);
			apg_profile.hide_ssid = apg_info["apg" + _apg_idx + "_hide_ssid"];
			apg_profile.security = decodeURIComponent(apg_info["apg" + _apg_idx + "_security"]);
			apg_profile.bw_limit = decodeURIComponent(apg_info["apg" + _apg_idx + "_bw_limit"]);
			apg_profile.timesched = apg_info["apg" + _apg_idx + "_timesched"];
			apg_profile.sched = decodeURIComponent(apg_info["apg" + _apg_idx + "_sched"]);
			apg_profile.expiretime = decodeURIComponent(apg_info["apg" + _apg_idx + "_expiretime"]);
			apg_profile.ap_isolate = apg_info["apg" + _apg_idx + "_ap_isolate"];
			apg_profile.macmode = decodeURIComponent(apg_info["apg" + _apg_idx + "_macmode"]);
			apg_profile.mlo = apg_info["apg" + _apg_idx + "_mlo"];
			apg_profile.maclist = decodeURIComponent(apg_info["apg" + _apg_idx + "_maclist"]);
			apg_profile.iot_max_cmpt = apg_info["apg" + _apg_idx + "_iot_max_cmpt"];
			apg_profile.dut_list = decodeURIComponent(apg_info["apg" + _apg_idx + "_dut_list"]);
			apg_rl_list.push(JSON.parse(JSON.stringify(apg_profile)));
		}
		return apg_rl_list;
	}
	function get_cp_rl_list(_cp_idx){
		var cp_rl_list = [];
		if(parseInt(_cp_idx) > 0){
			var cp_info = httpApi.nvramCharToAscii(["cp" + _cp_idx + "_profile", "cp" + _cp_idx + "_local_auth_profile", "cp" + _cp_idx + "_radius_profile"], true);
			var cp_profile = decodeURIComponent(cp_info["cp" + _cp_idx + "_profile"]);
			var each_cp_profile = cp_profile.split("<");
			$.each(each_cp_profile, function(index, value){
				if(value != ""){
					var profile_data = value.split(">");
					var cp_profile = new cp_profile_attr();
					cp_profile.cp_idx = _cp_idx.toString();
					cp_profile.enable = profile_data[0];
					cp_profile.auth_type = profile_data[1];
					cp_profile.conntimeout = profile_data[2];
					cp_profile.idle_timeout = profile_data[3];
					cp_profile.auth_timeout = profile_data[4];
					cp_profile.redirecturl = profile_data[5];
					cp_profile.term_of_service = profile_data[6];
					cp_profile.bw_limit_ul = profile_data[7];
					cp_profile.bw_limit_dl = profile_data[8];
					cp_profile.NAS_ID = profile_data[9];
					cp_profile.idx_for_customized_ui = profile_data[10];
					cp_profile.local_auth_profile = decodeURIComponent(cp_info["cp" + _cp_idx + "_local_auth_profile"]).substring(1);
					cp_profile.radius_profile = decodeURIComponent(cp_info["cp" + _cp_idx + "_radius_profile"]);
					cp_rl_list.push(JSON.parse(JSON.stringify(cp_profile)));
				}
			});
		}
		return cp_rl_list;
	}
	function update_SDN_ClientList(){
		httpApi.updateClientList();
		var get_clientlist = httpApi.hookGet("get_clientlist", true);
		$.each(get_clientlist, function(index, value){
			if(value.sdn_idx != undefined){
				if(value.isOnline == "1"){
					var specific_data = sdn_all_rl_json.filter(function(item, index, array){
						return (item.sdn_rl.idx == value.sdn_idx);
					})[0];
					if(specific_data != undefined){
						specific_data.client_num += 1;
					}
				}
			}
		});
	}
	function check_value_is_exist(val){
		let result = true;
		if(val == "undefined" || val == undefined || val == "")
			result = false;

		return result;
	}
}
var vlan_vid_add = "";
function get_new_sdn_profile(){
	var sdn_rl_profile = get_new_sdn_rl();
	var vlan_rl_profile = get_new_vlan_rl();
	var subnet_rl_profile = get_new_subnet_rl(sdn_rl_profile);
	var apg_rl_profile = get_new_apg_rl();
	sdn_rl_profile.vlan_idx = vlan_rl_profile.vlan_idx;
	sdn_rl_profile.subnet_idx = subnet_rl_profile.subnet_idx;
	sdn_rl_profile.apg_idx = apg_rl_profile.apg_idx;

	var dut_list = "";
	$.each(cfg_clientlist, function(index, node_info){
		var wifi_band_set = {"mac":"", "wifi_band":0};
		var wifi_band_info = httpApi.aimesh_get_node_wifi_band(node_info);
		$.each(wifi_band_info, function(index, band_info){
			if(band_info.no_used > 0){
				wifi_band_set.mac = node_info.mac;
				wifi_band_set.wifi_band += band_info.band;
			}
		});
		if(wifi_band_set.wifi_band != 0){
			dut_list += "<" + wifi_band_set.mac + ">" + wifi_band_set.wifi_band + ">";
		}
	});
	apg_rl_profile.dut_list = dut_list;
	var sdn_obj = {};
	sdn_obj["sdn_" + sdn_rl_profile.idx + ""] = JSON.parse(JSON.stringify(new sdn_all_rl_attr()));
	sdn_obj["sdn_" + sdn_rl_profile.idx + ""]["sdn_rl"] = JSON.parse(JSON.stringify(sdn_rl_profile));
	sdn_obj["sdn_" + sdn_rl_profile.idx + ""]["vlan_rl"] = vlan_rl_profile;
	sdn_obj["sdn_" + sdn_rl_profile.idx + ""]["subnet_rl"] = subnet_rl_profile;
	sdn_obj["sdn_" + sdn_rl_profile.idx + ""]["apg_rl"] = apg_rl_profile;
	return sdn_obj;

	function get_new_sdn_rl(){
		var idx = 1;
		idx = get_rl_new_idx(idx, "sdn");
		var sdn_profile = new sdn_rl_attr();
		sdn_profile.idx = idx.toString();
		return (JSON.parse(JSON.stringify(sdn_profile)));
	}
	function get_new_vlan_rl(){
		var vlan_idx = 1;
		vlan_idx = get_vlan_rl_new_idx(vlan_idx, "vlan");

		var vid = 52;
		vid = get_vlan_rl_new_vid(vid);

		vlan_vid_add = vid;

		var vlan_profile = new vlan_rl_attr();
		vlan_profile.vlan_idx = vlan_idx.toString();
		vlan_profile.vid = vid.toString();
		return (JSON.parse(JSON.stringify(vlan_profile)));

		function get_vlan_rl_new_idx(start_idx, category){
			var new_idx = parseInt(start_idx);
			for(new_idx; new_idx < (new_idx + sdn_maximum); new_idx+=1){
				var specific_data = vlan_rl_json.filter(function(item, index, array){
					return (item.vlan_idx == new_idx);
				})[0];
				if(specific_data == undefined){
					break;
				}
			}
			return new_idx;
		}
		function get_vlan_rl_new_vid(start_vid){
			var new_vid = start_vid;
			for(start_vid; start_vid < (start_vid + sdn_maximum); start_vid+=1){
				var specific_data = vlan_rl_json.filter(function(item, index, array){
					return (item.vid == start_vid);
				})[0];
				if(specific_data == undefined){
					new_vid = start_vid;
					break;
				}
			}
			return new_vid;
		}
	}
	function get_new_subnet_rl(sdn_profile){
		var subnet_idx = 1;
		subnet_idx = get_rl_new_idx(subnet_idx, "subnet");
		const dhcp_info = httpApi.nvramDefaultGet(["lan_ipaddr", "lan_netmask", "dhcp_enable_x", "dhcp_start", "dhcp_end", "dhcp_lease",
			"lan_domain", "dhcp_dns1_x", "dhcp_dns2_x", "dhcp_wins_x", "dhcp_static_x",
			"ipv6_service", "ipv6_autoconf_type", "ipv6_dns1", "ipv6_dns2", "ipv6_dns3", "dnspriv_enable", "dnspriv_profile"]);
		var ipaddr = get_subnet_rl_new_ipaddr();
		var ipaddr_substr = ipaddr.substr(0,ipaddr.lastIndexOf("."));
		var ipaddr_min = ipaddr_substr + "." + "2";
		var ipaddr_max = ipaddr_substr + "." + "254";

		var subnet_profile = new subnet_rl_attr();
		subnet_profile.subnet_idx = subnet_idx.toString();
		subnet_profile.ifname = "br" + (parseInt(sdn_profile.idx) + 51);
		subnet_profile.addr = ipaddr;
		subnet_profile.netmask = dhcp_info.lan_netmask;
		subnet_profile.dhcp_enable = dhcp_info.dhcp_enable_x;
		subnet_profile.dhcp_min = ipaddr_min;
		subnet_profile.dhcp_max = ipaddr_max;
		subnet_profile.dhcp_lease = dhcp_info.dhcp_lease;
		subnet_profile.domain_name = dhcp_info.lan_domain;
		subnet_profile.dns = dhcp_info.dhcp_dns1_x + "," + dhcp_info.dhcp_dns2_x;
		subnet_profile.wins = dhcp_info.dhcp_wins_x;
		subnet_profile.dhcp_static = dhcp_info.dhcp_static_x;
		subnet_profile.dhcp_unit = "";
		subnet_profile.ipv6_enable = dhcp_info.ipv6_service;
		subnet_profile.autoconf = dhcp_info.ipv6_autoconf_type;
		subnet_profile.addr6 = "1";
		subnet_profile.dhcp6_start = "1000";
		subnet_profile.dhcp6_end = "2000";
		subnet_profile.dns6 = dhcp_info.ipv6_dns1 + "," + dhcp_info.ipv6_dns2 + "," +  dhcp_info.ipv6_dns3;
		subnet_profile.dot_enable = dhcp_info.dnspriv_enable;
		subnet_profile.dot_tls = dhcp_info.dnspriv_profile;
		return (JSON.parse(JSON.stringify(subnet_profile)));

		function get_subnet_rl_new_ipaddr(){
			var exist_ipaddr_arr = [];
			exist_ipaddr_arr.push(get_network_num(httpApi.nvramGet(["lan_ipaddr"]).lan_ipaddr));
			exist_ipaddr_arr.push(get_network_num(httpApi.nvramGet(["wan0_ipaddr"]).wan0_ipaddr));
			exist_ipaddr_arr.push(get_network_num(httpApi.nvramGet(["wan1_ipaddr"]).wan1_ipaddr));
			$.each(sdn_all_rl_json, function(index, sdn_all_rl){
				if(!$.isEmptyObject(sdn_all_rl.subnet_rl)){
					exist_ipaddr_arr.push(get_network_num(sdn_all_rl.subnet_rl.addr));
				}
			});
			return (function(){
				var init_subnet = 52;
				var ipaddr_arr = httpApi.nvramDefaultGet(["lan_ipaddr"]).lan_ipaddr.split(".");
				var new_ipaddr = "";
				for(init_subnet; init_subnet < 255; init_subnet += 1){
					new_ipaddr = ipaddr_arr[0] + "." + ipaddr_arr[1] + "." + init_subnet;
					if($.inArray(new_ipaddr, exist_ipaddr_arr) == "-1") break;
				}
				return new_ipaddr + "." + ipaddr_arr[3];;
			})();
			function get_network_num(_ipaddr){
				return _ipaddr.substr(0, _ipaddr.lastIndexOf("."));
			}
		}
	}
	function get_new_apg_rl(){
		var apg_idx = 1;
		apg_idx = get_rl_new_idx(apg_idx, "apg");
		var apg_profile = new apg_rl_attr();
		apg_profile.apg_idx = apg_idx.toString();
		apg_profile.enable = "1";
		apg_profile.ssid = "ASUSTEST";
		apg_profile.security = "";
		apg_profile.hide_ssid = "0";
		apg_profile.bw_limit = "<0>>";
		apg_profile.timesched = "0";
		apg_profile.sched = "";
		apg_profile.expiretime = "";
		apg_profile.ap_isolate = "0";
		apg_profile.macmode = "0";
		apg_profile.mlo = "0";
		apg_profile.maclist = "";
		apg_profile.iot_max_cmpt = "";
		apg_profile.dut_list = "";
		return (JSON.parse(JSON.stringify(apg_profile)));
	}
	function get_rl_new_idx(start_idx, category){
		var new_idx = parseInt(start_idx);
		var compare_key = "idx";
		switch(category){
			case "sdn":
				compare_key = "idx";
				break;
			case "vlan":
				compare_key = "vlan_idx";
				break;
			case "subnet":
				compare_key = "subnet_idx";
				break;
			case "apg":
				compare_key = "apg_idx";
				break;
		}
		for(new_idx; new_idx < (new_idx + sdn_maximum); new_idx+=1){
			var specific_data = sdn_all_rl_json.filter(function(item, index, array){
				return (item.sdn_rl[compare_key] == new_idx);
			})[0];
			if(specific_data == undefined){
				break;
			}
		}
		return new_idx;
	}
}
function get_new_radius_rl(){
	var radius_profile = new radius_rl_attr();
	return (JSON.parse(JSON.stringify(radius_profile)));
}
function get_dut_list(_wifiband, _ori_dut_list_json){
	var dut_list = "";
	$.each(cfg_clientlist, function(index, node_info){
		var wifi_band_set = {"mac":"", "wifi_band":0, "lanport":""};
		wifi_band_set.mac = node_info.mac;
		var wifi_band_info = httpApi.aimesh_get_node_wifi_band(node_info);
		$.each(wifi_band_info, function(index, band_info){
			if(band_info.no_used > 0){
				const band = band_info.band;
				if(_wifiband == "1"){//2.4G
					if(band == "1")
						wifi_band_set.wifi_band += band;
				}
				else if(_wifiband == "2"){//5G
					if(band == "2" || band == "4" || band == "8")
						wifi_band_set.wifi_band += band;
				}
				else if(_wifiband == "3"){//2.4G&5G
					if(band == "1" || band == "2" || band == "4" || band == "8")
						wifi_band_set.wifi_band += band;
				}
				else if(_wifiband == "4"){//6G
					if(band == "16" || band == "32" || band == "64")
						wifi_band_set.wifi_band += band;
				}
				else if(_wifiband == "5"){//5G&6G
					if(band == "2" || band == "4" || band == "8" || band == "16" || band == "32" || band == "64")
						wifi_band_set.wifi_band += band;
				}
				else if(_wifiband == "6"){//2.4G&5G&6G
					if(band == "1" || band == "2" || band == "4" || band == "8" || band == "16" || band == "32" || band == "64")
						wifi_band_set.wifi_band += band;
				}
				else if(_wifiband == "0"){
					wifi_band_set.wifi_band = "0";
				}
			}
		});
		if(Array.isArray(_ori_dut_list_json)){
			var specific_node = _ori_dut_list_json.filter(function(item, index, array){
				return (item.mac == wifi_band_set.mac);
			})[0];
			if(specific_node != undefined)
				wifi_band_set.lanport = specific_node.lanport;
		}
		dut_list += "<" + wifi_band_set.mac + ">" + wifi_band_set.wifi_band + ">" + wifi_band_set.lanport;
	});
	return dut_list;
}
function get_specific_dut_list(_wifiband, _node_info, _ori_dut_list_json){
	var dut_list = "";
	var wifi_band_set = {"mac":"", "wifi_band":0, "lanport":""};
	wifi_band_set.mac = _node_info.mac;
	var wifi_band_info = httpApi.aimesh_get_node_wifi_band(_node_info);
	$.each(wifi_band_info, function(index, band_info){
		if(band_info.no_used > 0){
			const band = band_info.band;
			if(_wifiband == "1"){//2.4G
				if(band == "1")
					wifi_band_set.wifi_band += band;
			}
			else if(_wifiband == "2"){//5G
				if(band == "2" || band == "4")
					wifi_band_set.wifi_band += band;
			}
			else if(_wifiband == "3"){//2.4G&5G
				if(band == "1" || band == "2" || band == "4")
					wifi_band_set.wifi_band += band;
			}
			else if(_wifiband == "4"){//6G
				if(band == "16" || band == "64")
					wifi_band_set.wifi_band += band;
			}
			else if(_wifiband == "5"){//5G&6G
				if(band == "2" || band == "4" || band == "16" || band == "64")
					wifi_band_set.wifi_band += band;
			}
			else if(_wifiband == "6"){//2.4G&5G&6G
				if(band == "1" || band == "2" || band == "4" || band == "16" || band == "64")
					wifi_band_set.wifi_band += band;
			}
		}
	});
	if(Array.isArray(_ori_dut_list_json)){
		var specific_node = _ori_dut_list_json.filter(function(item, index, array){
			return (item.mac == wifi_band_set.mac);
		})[0];
		if(specific_node != undefined)
			wifi_band_set.lanport = specific_node.lanport;
	}
	dut_list = "<" + wifi_band_set.mac + ">" + wifi_band_set.wifi_band + ">" + wifi_band_set.lanport;
	return dut_list;
}
function get_mlo_dut_list(_mlo_band_num, _node_info, _ori_dut_list_json){
	const mlo_band_num = parseInt(_mlo_band_num);
	const node_mac = _node_info.mac;
	let dut_list = "";
	let wifi_band_set = {"mac":"", "wifi_band":0, "lanport":""};
	let wifi_band_info = [];
	if(aimesh_wifi_band_info[node_mac] != undefined && aimesh_wifi_band_info[node_mac].length > 0){
		wifi_band_info = aimesh_wifi_band_info[node_mac];
	}
	else{
		wifi_band_info = httpApi.aimesh_get_node_wifi_band(_node_info);
	}
	$.each(wifi_band_info, function(index, band_info){
		if(band_info.no_used > 0){
			wifi_band_set.mac = node_mac;
			const band = parseInt(band_info.band);
			if(band & mlo_band_num){
				wifi_band_set.wifi_band += band;
			}
		}
	});
	if(Array.isArray(_ori_dut_list_json)){
		var specific_node = _ori_dut_list_json.filter(function(item, index, array){
			return (item.mac == wifi_band_set.mac);
		})[0];
		if(specific_node != undefined)
			wifi_band_set.lanport = specific_node.lanport;
	}
	if(wifi_band_set.wifi_band != "0"){
		dut_list = "<" + wifi_band_set.mac + ">" + wifi_band_set.wifi_band + ">" + wifi_band_set.lanport;
	}

	return dut_list;
}
function get_Offline_dut_list(_obj, _wifiband, _ori_dut_list_json){
	var dut_list = "";
	var $node_list = $(_obj).find("[data-container=AiMesh_List] [data-node-status=offline]");
	$node_list.each(function(i, node_cntr){
		var wifi_band_match = false;
		var node_mac = $(node_cntr).attr("data-node-mac");
		var node_wifi_band = $(node_cntr).attr("data-wifi-band");
		var node_lanport = "";
		if(_wifiband == "1"){
			if(node_wifi_band == "1"){
				wifi_band_match = true;
			}
		}
		else if(_wifiband == "2"){//5G
			if(node_wifi_band == "2" || node_wifi_band == "4"){
				wifi_band_match = true;
			}
		}
		else if(_wifiband == "3"){//2.4G&5G
			if(node_wifi_band == "3" || node_wifi_band == "5"){
				wifi_band_match = true;
			}
		}
		else if(_wifiband == "4"){//6G
			if(node_wifi_band == "16" || node_wifi_band == "64"){
				wifi_band_match = true;
			}
		}
		else if(_wifiband == "5"){//5G&6G
			if(node_wifi_band == "2" || node_wifi_band == "4" || node_wifi_band == "16" || node_wifi_band == "64"){
				wifi_band_match = true;
			}
		}
		else if(_wifiband == "6"){//2.4G&5G&6G
			if(node_wifi_band == "1" || node_wifi_band == "2" || node_wifi_band == "4" || node_wifi_band == "16" || node_wifi_band == "64"){
				wifi_band_match = true;
			}
		}
		if(Array.isArray(_ori_dut_list_json)){
			var specific_node = _ori_dut_list_json.filter(function(item, index, array){
				return (item.mac == node_mac);
			})[0];
			if(specific_node != undefined)
				node_lanport = specific_node.lanport;
		}
		if(wifi_band_match)
			dut_list += "<" + node_mac + ">" + node_wifi_band + ">" + node_lanport;
		else
			dut_list += "<" + node_mac + ">0>" + node_lanport;
	});
	return dut_list;
}
function get_unChecked_dut_list(_obj, _ori_dut_list_json){
	var dut_list = "";
	$(_obj).find("[data-container=AiMesh_List]").find(".node_container[data-support-dutlist=true]").each(function(){
		if(!$(this).find(".icon_checkbox").hasClass("clicked")){
			var node_mac = $(this).attr("data-node-mac");
			var node_lanport = "";
			if(Array.isArray(_ori_dut_list_json)){
				var specific_node = _ori_dut_list_json.filter(function(item, index, array){
					return (item.mac == node_mac);
				})[0];
				if(specific_node != undefined)
					node_lanport = specific_node.lanport;
			}
			dut_list += "<" + node_mac + ">>" + node_lanport + "";
		}
	});
	return dut_list;
}
function get_dut_list_by_mac(_wifiband, _node_mac){
	var dut_list = "";
	_wifiband = (_wifiband != undefined && _wifiband != "") ? _wifiband : "3";
	_node_mac = (_node_mac != undefined && _node_mac != "") ? _node_mac : ((httpApi.hookGet('get_lan_hwaddr')) ? httpApi.hookGet('get_lan_hwaddr') : '');
	var wifi_band_set = {"mac":"", "wifi_band":0, "lanport":""};
	wifi_band_set.mac = _node_mac;
	var wifi_band_info = [];
	var get_node_wifi_band = httpApi.hookGet("get_node_wifi_band");
	if(get_node_wifi_band != undefined && get_node_wifi_band != ""){
		if(get_node_wifi_band[_node_mac] != undefined && get_node_wifi_band[_node_mac]["wifi_band"] != undefined){
			$.each(get_node_wifi_band[_node_mac]["wifi_band"], function(index, value){
				var band_data = value;
				var band_info = {};
				band_info.band = parseInt(value.band);
				band_info.count = parseInt(value.count);
				band_info.no_used = 0;
				$.each(band_data.vif, function(vif_if, vif_data){
					if(vif_data.type == "1")
						band_info.no_used += 1;
				});
				wifi_band_info.push(band_info);
			});
		}
	}
	$.each(wifi_band_info, function(index, band_info){
		if(band_info.no_used > 0){
			wifi_band_set.mac = _node_mac;
			if(_wifiband == "1"){//2.4G
				if(band_info.band == "1")
					wifi_band_set.wifi_band += band_info.band;
			}
			else if(_wifiband == "2"){//5G
				if(band_info.band == "2" || band_info.band == "4")
					wifi_band_set.wifi_band += band_info.band;
			}
			else if(_wifiband == "3"){//2.4G&5G
				if(band_info.band == "1" || band_info.band == "2" || band_info.band == "4")
					wifi_band_set.wifi_band += band_info.band;
			}
			else if(_wifiband == "16"){//6G
				if(band_info.band == "16" || band_info.band == "64")
					wifi_band_set.wifi_band += band_info.band;
			}
		}
	});
	dut_list = "<" + wifi_band_set.mac + ">" + wifi_band_set.wifi_band + ">" + wifi_band_set.lanport;
	return dut_list;
}
function get_dut_list_is_star(dut_list){
	let is_star = false;
	const dut_list_arr = dut_list.split("<");
	$.each(dut_list_arr, function(index, dut_info){
		if(dut_info != ""){
			let dut_info_arr = dut_info.split(">");
			if(dut_info_arr[0] == "*"){
				is_star = true;
				return false;
			}
		}
	});
	return is_star;
}
function check_is_mlo_bh(apg_rl){
	const mlo_is_bh = (apg_rl.mlo == "1") ? true : false;
	const dut_list_is_bh = get_dut_list_is_star(apg_rl.dut_list);
	return (mlo_is_bh && dut_list_is_bh);
}
function get_cp_type_support(_type){
	return ((cp_type_support["type_"+_type+""] == true) ? true : false);
}
function get_elem_attr_wifi_auth(_parm){
	let option_value = "0";
	let sdn_type = "";
	if(_parm.wifi_band_option_value != undefined)
		option_value = _parm.wifi_band_option_value;
	if(_parm.sdn_type != undefined)
		sdn_type = _parm.sdn_type;
	if(sdn_type == "MLO")
		return "[data-wifi-auth=6G]";
	else{
		if(option_value == "4" || option_value == "5" || option_value == "6")//6G, 5G6G, 2G5G6G
			return "[data-wifi-auth=6G]";
		else
			return "[data-wifi-auth=default]";
	}
}
function sortObject(obj){
    return Object.keys(obj).sort().reduce(function (result, key) {
        result[key] = obj[key];
        return result;
    }, {});
}
function parse_JSONToStr_sdn_all_list(_profile_data){
	var _sdn_all_rl_json = JSON.parse(JSON.stringify(sdn_all_rl_json));
	var _vlan_rl_json = JSON.parse(JSON.stringify(vlan_rl_json));
	var _vlan_rl_at_sdn_all_rl = [];
	if(_profile_data != undefined){
		if(_profile_data.sdn_all_rl != undefined && (typeof _profile_data.sdn_all_rl == "object")){
			_sdn_all_rl_json = ((_profile_data.sdn_all_rl.length > 0) ? JSON.parse(JSON.stringify(_profile_data.sdn_all_rl)) : []);
		}
		if(_profile_data.vlan_rl != undefined && _profile_data.vlan_rl != "" && (typeof _profile_data.vlan_rl == "object") && (_profile_data.vlan_rl.length > 0)){
			_vlan_rl_json = JSON.parse(JSON.stringify(_profile_data.vlan_rl));
		}
	}
	var result = {"sdn_rl":"", "vlan_rl":"", "subnet_rl":"", "radius_list":"", "sdn_access_rl":""};
	var sdn_rl = "", vlan_rl = "", subnet_rl = "", radius_rl = "", sdn_access_rl = "";
	if(typeof sdn0_rl == "string" && sdn0_rl != ""){
		sdn_rl = sdn0_rl;
	}
	else{
		sdn_rl = (httpApi.nvramDefaultGet(["sdn_rl"]).sdn_rl).replace(/&#60/g, "<").replace(/&#62/g, ">");
	}
	_sdn_all_rl_json.sort(function(a, b) {
		return parseInt(a.sdn_rl.idx) - parseInt(b.sdn_rl.idx);
	});
	$.each(_sdn_all_rl_json, function(index, sdn_all_rl){
		var sdn_profile = sdn_all_rl.sdn_rl;
		if(!$.isEmptyObject(sdn_profile)){
			if(sdn_profile.idx == "0")
				return;
			sdn_rl += "<";
			sdn_rl += sdn_profile.idx;
			sdn_rl += ">";
			sdn_rl += sdn_profile.sdn_name;
			sdn_rl += ">";
			sdn_rl += sdn_profile.sdn_enable;
			sdn_rl += ">";
			sdn_rl += sdn_profile.vlan_idx;
			sdn_rl += ">";
			sdn_rl += sdn_profile.subnet_idx;
			sdn_rl += ">";
			sdn_rl += sdn_profile.apg_idx;
			sdn_rl += ">";
			sdn_rl += sdn_profile.vpnc_idx;
			sdn_rl += ">";
			sdn_rl += sdn_profile.vpns_idx;
			sdn_rl += ">";
			sdn_rl += sdn_profile.dns_filter_idx;
			sdn_rl += ">";
			sdn_rl += sdn_profile.urlf_idx;
			sdn_rl += ">";
			sdn_rl += sdn_profile.nwf_idx;
			sdn_rl += ">";
			sdn_rl += sdn_profile.cp_idx;
			sdn_rl += ">";
			sdn_rl += sdn_profile.gre_idx;
			sdn_rl += ">";
			sdn_rl += sdn_profile.firewall_idx;
			sdn_rl += ">";
			sdn_rl += sdn_profile.kill_switch;
			sdn_rl += ">";
			sdn_rl += sdn_profile.access_host_service;
			sdn_rl += ">";
			sdn_rl += sdn_profile.wan_unit;
			sdn_rl += ">";
			sdn_rl += sdn_profile.pppoe_relay;
			sdn_rl += ">";
			sdn_rl += sdn_profile.wan6_unit;
			sdn_rl += ">";
			sdn_rl += sdn_profile.createby;
			sdn_rl += ">";
			sdn_rl += sdn_profile.mtwan_idx;
			sdn_rl += ">";
			sdn_rl += sdn_profile.mswan_idx;
		}

		var subnet_profile = sdn_all_rl.subnet_rl;
		if(!$.isEmptyObject(subnet_profile)){
			subnet_rl += "<";
			subnet_rl += subnet_profile.subnet_idx;
			subnet_rl += ">";
			subnet_rl += subnet_profile.ifname;
			subnet_rl += ">";
			subnet_rl += subnet_profile.addr;
			subnet_rl += ">";
			subnet_rl += subnet_profile.netmask;
			subnet_rl += ">";
			subnet_rl += subnet_profile.dhcp_enable;
			subnet_rl += ">";
			subnet_rl += subnet_profile.dhcp_min;
			subnet_rl += ">";
			subnet_rl += subnet_profile.dhcp_max;
			subnet_rl += ">";
			subnet_rl += subnet_profile.dhcp_lease;
			subnet_rl += ">";
			subnet_rl += subnet_profile.domain_name;
			subnet_rl += ">";
			subnet_rl += subnet_profile.dns;
			subnet_rl += ">";
			subnet_rl += subnet_profile.wins;
			subnet_rl += ">";
			subnet_rl += subnet_profile.dhcp_static;
			subnet_rl += ">";
			subnet_rl += subnet_profile.dhcp_unit;
			subnet_rl += ">";
			subnet_rl += subnet_profile.ipv6_enable;
			subnet_rl += ">";
			subnet_rl += subnet_profile.autoconf;
			subnet_rl += ">";
			subnet_rl += subnet_profile.addr6;
			subnet_rl += ">";
			subnet_rl += subnet_profile.dhcp6_start;
			subnet_rl += ">";
			subnet_rl += subnet_profile.dhcp6_end;
			subnet_rl += ">";
			subnet_rl += subnet_profile.dns6;
			subnet_rl += ">";
			subnet_rl += subnet_profile.dot_enable;
			subnet_rl += ">";
			subnet_rl += subnet_profile.dot_tls;
			subnet_rl += ">";
		}

		var radius_profile = sdn_all_rl.radius_rl;
		if(!$.isEmptyObject(radius_profile)){
			radius_rl += "<";
			radius_rl += radius_profile.radius_idx;
			radius_rl += ">";
			radius_rl += radius_profile.auth_server_1;
			radius_rl += ">";
			radius_rl += radius_profile.auth_port_1;
			radius_rl += ">";
			radius_rl += radius_profile.auth_key_1;
			radius_rl += ">";
			radius_rl += radius_profile.acct_server_1;
			radius_rl += ">";
			radius_rl += radius_profile.acct_port_1;
			radius_rl += ">";
			radius_rl += radius_profile.acct_key_1;
			radius_rl += ">";
			radius_rl += radius_profile.auth_server_2;
			radius_rl += ">";
			radius_rl += radius_profile.auth_port_2;
			radius_rl += ">";
			radius_rl += radius_profile.auth_key_2;
			radius_rl += ">";
			radius_rl += radius_profile.acct_server_2;
			radius_rl += ">";
			radius_rl += radius_profile.acct_port_2;
			radius_rl += ">";
			radius_rl += radius_profile.acct_key_2;
			radius_rl += ">";
		}

		if(!$.isEmptyObject(sdn_all_rl.vlan_rl)){
			_vlan_rl_at_sdn_all_rl.push(sdn_all_rl.vlan_rl);
		}

		var sdn_access_json = sdn_all_rl.sdn_access_rl;
		if(sdn_access_json.length > 0){
			$.each(sdn_access_json, function(index, sdn_access_profile){
				if(!$.isEmptyObject(sdn_access_profile)){
					sdn_access_rl += "<";
					sdn_access_rl += sdn_access_profile.access_sdn_idx;
					sdn_access_rl += ">";
					sdn_access_rl += sdn_access_profile.sdn_idx;
				}
			});
		}
	});

	$.each(_vlan_rl_at_sdn_all_rl, function(index, vlan_profile){
		if(!$.isEmptyObject(vlan_profile)){
			var specific_vlan = _vlan_rl_json.filter(function(item, index, array){
				return (item.vlan_idx == vlan_profile.vlan_idx);
			})[0];

			if(specific_vlan == undefined){
				_vlan_rl_json.push(vlan_profile);
			}
		}
	})
	_vlan_rl_json.sort(function(a, b) {
		return parseInt(a.vlan_idx) - parseInt(b.vlan_idx);
	});

	$.each(_vlan_rl_json, function(index, vlan_profile){
		if(!$.isEmptyObject(vlan_profile)){
			vlan_rl += "<";
			vlan_rl += vlan_profile.vlan_idx;
			vlan_rl += ">";
			vlan_rl += vlan_profile.vid;
			vlan_rl += ">";
			vlan_rl += vlan_profile.port_isolation;
			vlan_rl += ">";
		}
	});

	result.sdn_rl = sdn_rl;
	result.vlan_rl = vlan_rl;
	result.subnet_rl = subnet_rl;
	result.radius_list = radius_rl;
	result.sdn_access_rl = sdn_access_rl;
	return result;
}
function parse_JSONToStr_del_sdn_all_rl(_json_data){
	var json_data_tmp = JSON.parse(JSON.stringify(_json_data));
	var result = {"sdn_rl_x":"", "vlan_rl_x":"", "subnet_rl_x":"", "radius_list_x":""};
	var sdn_rl = "", vlan_rl = "", subnet_rl = "", radius_rl = "";
	json_data_tmp.sort(function(a, b) {
		return parseInt(a.sdn_rl.idx) - parseInt(b.sdn_rl.idx);
	});
	$.each(json_data_tmp, function(index, sdn_all_rl){
		var sdn_profile = sdn_all_rl.sdn_rl;
		if(!$.isEmptyObject(sdn_profile)){
			if(sdn_profile.idx == "0")
				return;
			sdn_rl += "<";
			sdn_rl += sdn_profile.idx;
			sdn_rl += ">";
			sdn_rl += sdn_profile.sdn_name;
			sdn_rl += ">";
			sdn_rl += sdn_profile.sdn_enable;
			sdn_rl += ">";
			sdn_rl += sdn_profile.vlan_idx;
			sdn_rl += ">";
			sdn_rl += sdn_profile.subnet_idx;
			sdn_rl += ">";
			sdn_rl += sdn_profile.apg_idx;
			sdn_rl += ">";
			sdn_rl += sdn_profile.vpnc_idx;
			sdn_rl += ">";
			sdn_rl += sdn_profile.vpns_idx;
			sdn_rl += ">";
			sdn_rl += sdn_profile.dns_filter_idx;
			sdn_rl += ">";
			sdn_rl += sdn_profile.urlf_idx;
			sdn_rl += ">";
			sdn_rl += sdn_profile.nwf_idx;
			sdn_rl += ">";
			sdn_rl += sdn_profile.cp_idx;
			sdn_rl += ">";
			sdn_rl += sdn_profile.gre_idx;
			sdn_rl += ">";
			sdn_rl += sdn_profile.firewall_idx;
			sdn_rl += ">";
			sdn_rl += sdn_profile.kill_switch;
			sdn_rl += ">";
			sdn_rl += sdn_profile.access_host_service;
			sdn_rl += ">";
			sdn_rl += sdn_profile.wan_unit;
			sdn_rl += ">";
			sdn_rl += sdn_profile.pppoe_relay;
			sdn_rl += ">";
			sdn_rl += sdn_profile.wan6_unit;
			sdn_rl += ">";
			sdn_rl += sdn_profile.createby;
			sdn_rl += ">";
			sdn_rl += sdn_profile.mtwan_idx;
			sdn_rl += ">";
			sdn_rl += sdn_profile.mswan_idx;
		}

		var vlan_profile = sdn_all_rl.vlan_rl;
		if(!$.isEmptyObject(vlan_profile)){
			vlan_rl += "<";
			vlan_rl += vlan_profile.vlan_idx;
			vlan_rl += ">";
			vlan_rl += vlan_profile.vid;
			vlan_rl += ">";
			vlan_rl += vlan_profile.port_isolation;
			vlan_rl += ">";
		}

		var subnet_profile = sdn_all_rl.subnet_rl;
		if(!$.isEmptyObject(subnet_profile)){
			subnet_rl += "<";
			subnet_rl += subnet_profile.subnet_idx;
			subnet_rl += ">";
			subnet_rl += subnet_profile.ifname;
			subnet_rl += ">";
			subnet_rl += subnet_profile.addr;
			subnet_rl += ">";
			subnet_rl += subnet_profile.netmask;
			subnet_rl += ">";
			subnet_rl += subnet_profile.dhcp_enable;
			subnet_rl += ">";
			subnet_rl += subnet_profile.dhcp_min;
			subnet_rl += ">";
			subnet_rl += subnet_profile.dhcp_max;
			subnet_rl += ">";
			subnet_rl += subnet_profile.dhcp_lease;
			subnet_rl += ">";
			subnet_rl += subnet_profile.domain_name;
			subnet_rl += ">";
			subnet_rl += subnet_profile.dns;
			subnet_rl += ">";
			subnet_rl += subnet_profile.wins;
			subnet_rl += ">";
			subnet_rl += subnet_profile.dhcp_static;
			subnet_rl += ">";
			subnet_rl += subnet_profile.dhcp_unit;
			subnet_rl += ">";
			subnet_rl += subnet_profile.ipv6_enable;
			subnet_rl += ">";
			subnet_rl += subnet_profile.autoconf;
			subnet_rl += ">";
			subnet_rl += subnet_profile.addr6;
			subnet_rl += ">";
			subnet_rl += subnet_profile.dhcp6_start;
			subnet_rl += ">";
			subnet_rl += subnet_profile.dhcp6_end;
			subnet_rl += ">";
			subnet_rl += subnet_profile.dns6;
			subnet_rl += ">";
			subnet_rl += subnet_profile.dot_enable;
			subnet_rl += ">";
			subnet_rl += subnet_profile.dot_tls;
			subnet_rl += ">";
		}

		var radius_profile = sdn_all_rl.radius_rl;
		if(!$.isEmptyObject(radius_profile)){
			radius_rl += "<";
			radius_rl += radius_profile.radius_idx;
			radius_rl += ">";
			radius_rl += radius_profile.auth_server_1;
			radius_rl += ">";
			radius_rl += radius_profile.auth_port_1;
			radius_rl += ">";
			radius_rl += radius_profile.auth_key_1;
			radius_rl += ">";
			radius_rl += radius_profile.acct_server_1;
			radius_rl += ">";
			radius_rl += radius_profile.acct_port_1;
			radius_rl += ">";
			radius_rl += radius_profile.acct_key_1;
			radius_rl += ">";
			radius_rl += radius_profile.auth_server_2;
			radius_rl += ">";
			radius_rl += radius_profile.auth_port_2;
			radius_rl += ">";
			radius_rl += radius_profile.auth_key_2;
			radius_rl += ">";
			radius_rl += radius_profile.acct_server_2;
			radius_rl += ">";
			radius_rl += radius_profile.acct_port_2;
			radius_rl += ">";
			radius_rl += radius_profile.acct_key_2;
			radius_rl += ">";
		}
	});
	result.sdn_rl_x = sdn_rl;
	result.vlan_rl_x = vlan_rl;
	result.subnet_rl_x = subnet_rl;
	result.radius_list_x = radius_rl;
	return result;
}
function parse_apg_rl_to_apgX_rl(_apg_rl){
	var result = {};
	if(!$.isEmptyObject(_apg_rl)){
		var apg_idx = _apg_rl.apg_idx;
		Object.keys(_apg_rl).forEach(function(key){
			if(key != "apg_idx"){
				result["apg" + apg_idx + "_" + key] = _apg_rl[key];
			}
		});
	}
	return result;
}
function parse_cp_rl_to_cpX_rl(_cp_rl){
	var result = {};
	if(!$.isEmptyObject(_cp_rl)){
		var cp_profile = "";
		cp_profile += "<";
		cp_profile += _cp_rl.enable;
		cp_profile += ">";
		cp_profile += _cp_rl.auth_type;
		cp_profile += ">";
		cp_profile += _cp_rl.conntimeout;
		cp_profile += ">";
		cp_profile += _cp_rl.idle_timeout;
		cp_profile += ">";
		cp_profile += _cp_rl.auth_timeout;
		cp_profile += ">";
		cp_profile += _cp_rl.redirecturl;
		cp_profile += ">";
		cp_profile += _cp_rl.term_of_service;
		cp_profile += ">";
		cp_profile += _cp_rl.bw_limit_ul;
		cp_profile += ">";
		cp_profile += _cp_rl.bw_limit_dl;
		cp_profile += ">";
		cp_profile += _cp_rl.NAS_ID;
		cp_profile += ">";
		cp_profile += _cp_rl.idx_for_customized_ui;
		result["cp" + _cp_rl.cp_idx + "_profile"] = cp_profile;
		result["cp" + _cp_rl.cp_idx + "_local_auth_profile"] =  (_cp_rl.auth_type == "1") ? ("<" + _cp_rl.local_auth_profile) : "";
		result["cp" + _cp_rl.cp_idx + "_radius_profile"] = _cp_rl.radius_profile;
	}
	return result;
}
var aimesh_wifi_band_info = [];
function init_aimesh_wifi_band_info(){
	aimesh_wifi_band_info = [];
	$.each(cfg_clientlist, function(index, node_info){
		var wifi_band_info = httpApi.aimesh_get_node_wifi_band(node_info);
		$.each(wifi_band_info, function(index, item){
			item["used"] = 0;
		});
		wifi_band_info.sort(function(a, b){
			return parseInt(a.band) - parseInt(b.band);
		});
		aimesh_wifi_band_info[node_info.mac] = JSON.parse(JSON.stringify(wifi_band_info));
	});
}
function update_aimesh_wifi_band_info(_except_sdn){
	init_aimesh_wifi_band_info();
	$.each(sdn_all_rl_json, function(index, sdn_all_rl){
		if(_except_sdn != undefined){
			if(sdn_all_rl.sdn_rl.idx == _except_sdn.sdn_rl.idx){
				return;
			}
		}
		if(sdn_all_rl.apg_rl.dut_list != undefined){
			var dut_list_arr = sdn_all_rl.apg_rl.dut_list.split("<");
			$.each(dut_list_arr, function(index, dut_info){
				if(dut_info != ""){
					var dut_info_arr = dut_info.split(">");
					var dut_mac = dut_info_arr[0];
					var dut_wifi_band = parseInt(dut_info_arr[1]);
					//1:2.4G, 2:5G, 4:5GL, 8:5GH, 16:6G, 32:6GL, 64:6GH
					if(aimesh_wifi_band_info[dut_mac] != undefined){
						$.each(aimesh_wifi_band_info[dut_mac], function(index, wifi_band){
							if(wifi_band.band & dut_wifi_band){
								wifi_band.used += 1;
							}
						});
					}
				}
			});
		}
	});
}
let aimesh_wifi_band_full = {"all":{"band_2G": true, "band_5G": true, "band_6G": true}, "node":[]};
function update_aimesh_wifi_band_full(){
	aimesh_wifi_band_full = {"all":{"band_2G": true, "band_5G": true, "band_6G": true}, "node":[]};
	Object.keys(aimesh_wifi_band_info).forEach(function(dut_mac){
		aimesh_wifi_band_full.node[dut_mac] = {"band_2G": false, "band_5G": false, "band_6G": false, "support_WPA3E": false, "support_6G": false, "support_wifi_band": false};
		$.each(aimesh_wifi_band_info[dut_mac], function(index, wifi_band){
			if(	wifi_band.band == 1 ||
				wifi_band.band == 2 || wifi_band.band == 4 || wifi_band.band == 8 ||
				wifi_band.band == 16 || wifi_band.band == 32 || wifi_band.band == 64
			){
				if(wifi_band.used >= wifi_band.no_used || wifi_band.used >= wifi_band_maximum){
					switch(wifi_band.band){
						case 1:
							aimesh_wifi_band_full.node[dut_mac].band_2G = true;
							break;
						case 2:
						case 4:
						case 8:
							aimesh_wifi_band_full.node[dut_mac].band_5G = true;
							break;
						case 16:
						case 32:
						case 64:
							aimesh_wifi_band_full.node[dut_mac].band_6G = true;
							break;
					}
				}
			}
		});
		var specific_node_data = cfg_clientlist.filter(function(item, index, array){
			return (item.mac == dut_mac);
		})[0];
		if(specific_node_data != undefined){
			var node_capability = httpApi.aimesh_get_node_capability(specific_node_data);
			if(node_capability.wpa3_enterprise)
				aimesh_wifi_band_full.node[dut_mac].support_WPA3E = true;
		}
		let band_6G = aimesh_wifi_band_info[dut_mac].filter((item)=>{
			return (item.band == "16" || item.band == "64");
		})[0];
		if(band_6G != undefined){
			aimesh_wifi_band_full.node[dut_mac].support_6G = true;
		}
    aimesh_wifi_band_full.node[dut_mac].support_wifi_band = aimesh_wifi_band_info[dut_mac].every((item)=>{
			return (item.mode == "2");
		});
		if(aimesh_wifi_band_full.node[dut_mac].band_2G == false)
			aimesh_wifi_band_full.all.band_2G = false;
		if(aimesh_wifi_band_full.node[dut_mac].band_5G == false)
			aimesh_wifi_band_full.all.band_5G = false;
		if(aimesh_wifi_band_full.node[dut_mac].band_6G == false)
			aimesh_wifi_band_full.all.band_6G = false;
	});
}
let aimesh_wifi_mlo_info = {"cap":{"all_band_num":0, "mlo_band_num":0},"node":[]};
const mlo_band_num_mapping = (()=>{
	//1:2.4G, 2:5G, 4:5GL, 8:5GH, 16:6G, 32:6GL, 64:6GH
	if(isSupport("mlo")){
		const ui_options_256 = [wifi_band_options.find((el) => {return (el.value == "6");})];
		const ui_options_56 = [wifi_band_options.find((el) => {return (el.value == "5");})];
		const ui_options_25 = [wifi_band_options.find((el) => {return (el.value == "3");})];
		return [
			{"all_band":(1+4+8+16), "mlo_band":(1+8+16), "ui_option":ui_options_256},//2556, mlo:2/5-2/6
			{"all_band":(1+2+32+64), "mlo_band":(1+2+32), "ui_option":ui_options_256},//2566, mlo:2/5/6-1
			{"all_band":(1+2+16), "mlo_band":(1+2+16), "ui_option":ui_options_256},//256, mlo:2/5/6
			{"all_band":(1+4+8), "mlo_band":(1+4+8), "ui_option":ui_options_25},//255
			{"all_band":(1+2), "mlo_band":(1+2), "ui_option":ui_options_25},//25
		];
	}
	else return [];
})();
function init_aimesh_wifi_mlo_info(){
	aimesh_wifi_mlo_info = {"cap":{"all_band_num":0, "mlo_band_num":0},"node":[]};
	if(cfg_clientlist[0] != undefined){
		if(!isSupport("mlo")) return;

		let node_mlo_info = get_node_mlo_info(cfg_clientlist[0]);
		node_mlo_info.support = true;
		aimesh_wifi_mlo_info.node.push(node_mlo_info);
		aimesh_wifi_mlo_info.cap.all_band_num = node_mlo_info.all_band_num;
		aimesh_wifi_mlo_info.cap.mlo_band_num = node_mlo_info.mlo_band_num;

		$.each(cfg_clientlist, function(index, node_info){
			if(index == 0) return;//filter cap
			const node_capability = httpApi.aimesh_get_node_capability(node_info);
			let node_mlo_info = get_node_mlo_info(node_info);
			if(	node_capability.mlo_fh &&
				node_mlo_info.all_band_num == aimesh_wifi_mlo_info.cap.all_band_num){
				node_mlo_info.support = true;
			}
			aimesh_wifi_mlo_info.node.push(node_mlo_info);
		});
	}
	function get_node_mlo_info(node_info){
		let obj_info = {"mac":"", "support":false, "all_band_num":0, "mlo_band_num":0};
		obj_info.mac = node_info.mac;
		const wifi_band_info = httpApi.aimesh_get_node_wifi_band(node_info);
		wifi_band_info.sort(function(a, b){
			return parseInt(a.band) - parseInt(b.band);
		});
		let all_band_num = 0;
		$.each(wifi_band_info, function(index, band_info){
			const band_num = parseInt(band_info.band);
			if(!Number.isNaN(band_num)){
				all_band_num += band_num;
			}
		});
		obj_info.all_band_num = all_band_num;
		const mlo_band_info = mlo_band_num_mapping.find(function(item){
			return (item.all_band == all_band_num);
		});
		if(mlo_band_info != undefined){
			obj_info.mlo_band_num = mlo_band_info.mlo_band;
		}
		return obj_info;
	}
}
function Get_AiMesh_List_Container(){
	var $container = $("<div>");
	$.each(cfg_clientlist, function(index, node_info){
		var $node_cntr = $("<div>").addClass("node_container").attr({"data-node-mac":node_info.mac, "data-support-dutlist":"true"}).appendTo($container);
		var $node_content_cntr = $("<div>").addClass("node_content_container").unbind("click").click(function(e){
			e = e || event;
			e.stopPropagation();
			if($(this).children(".icon_checkbox").hasClass("closed"))
				return;
			$(this).children(".icon_checkbox").toggleClass("clicked");
			var $AiMesh_List_cntr = $(this).closest("[data-container=AiMesh_List]");
			$AiMesh_List_cntr.next(".validate_hint").remove();
			if(!$AiMesh_List_cntr.find(".node_container .icon_checkbox:not(.disabled)").hasClass("clicked")){
				$("<div>").html("<#JS_fieldblank#>").addClass("validate_hint").insertAfter($AiMesh_List_cntr);
				resize_iframe_height();
			}
		}).appendTo($node_cntr);
		var isReNode = ((index != 0) ? true : false);
		var loc_info = get_location_info(node_info, isReNode);
		var node_name_loc = get_model_name(node_info) + " (" + loc_info.loc_text + ")";
		let sub_info_text = (node_info.online == "1") ? `<span>${node_info.ip}</span>` : `<span><#Clientlist_OffLine#> | ${node_info.mac}</span>`;
		let $icon_cb = $("<div>").addClass("icon_checkbox").appendTo($node_content_cntr);
		let $node_info = $("<div>").addClass("node_info").attr({"title":node_name_loc}).appendTo($node_content_cntr);
		$("<div>").addClass("node_main_info").html($("<span>").html(htmlEnDeCode.htmlEncode(get_model_name(node_info)))).appendTo($node_info);
		$("<div>").addClass("node_sub_info").html(sub_info_text).appendTo($node_info);
		let wifi_band_info = httpApi.aimesh_get_node_wifi_band(node_info);
		if(wifi_band_info.length == 0){
			$("<div>").addClass("icon_location " + loc_info.loc_css + "").appendTo($node_content_cntr);
			var hint = "* <#AiMesh_Modellist_Not_Support_Feature01#> <#AiMesh_Modellist_Not_Support_Feature03#>".replace("#MODELLIST", get_model_name(node_info));
			$("<div>").addClass("node_hint").html(hint).appendTo($node_cntr);
			$icon_cb.addClass("closed");
			$node_cntr.attr({"data-support-dutlist":"false"})
		}
		else{
			let sort_wifi_band_info = JSON.parse(JSON.stringify(wifi_band_info)).sort((a,b)=>{return a.band-b.band});
			let wifi_band_arr = [];
			$.each(sort_wifi_band_info, function(index, band_info){
				let wifi_band = "";
				switch(parseInt(band_info.band)){
					case 1:
						wifi_band = "2g";
						break;
					case 2:
					case 4:
					case 8:
						wifi_band = "5g";
						break;
					case 16:
					case 32:
					case 64:
						wifi_band = "6g";
						break;
					default:
						wifi_band = "";
						break;
				}
				if(wifi_band != ""){
					if($.inArray(wifi_band, wifi_band_arr) < 0) wifi_band_arr.push(wifi_band);
				}
			});
			$.each(wifi_band_arr, function(index, band){
				$("<div>").addClass("icon_wifi_band wifi_" + band + "").appendTo($node_content_cntr);
			})
		}
	});
	return $container;

	function get_location_info(_node_info, _isReNode){
		if(typeof aimesh_location_arr != "object"){
			var aimesh_location_arr = [
				{value:"Home",text:"<#AiMesh_NodeLocation01#>"}, {value:"Living Room",text:"<#AiMesh_NodeLocation02#>"}, {value:"Dining Room",text:"<#AiMesh_NodeLocation03#>"},
				{value:"Bedroom",text:"<#AiMesh_NodeLocation04#>"}, {value:"Office",text:"<#AiMesh_NodeLocation05#>"}, {value:"Stairwell",text:"<#AiMesh_NodeLocation06#>"},
				{value:"Hall",text:"<#AiMesh_NodeLocation07#>"}, {value:"Kitchen",text:"<#AiMesh_NodeLocation08#>"}, {value:"Attic",text:"<#AiMesh_NodeLocation09#>"},
				{value:"Basement",text:"<#AiMesh_NodeLocation10#>"}, {value:"Yard",text:"<#AiMesh_NodeLocation11#>"}, {value:"Master Bedroom",text:"<#AiMesh_NodeLocation12#>"},
				{value:"Guest Room",text:"<#AiMesh_NodeLocation13#>"}, {value:"Kids Room",text:"<#AiMesh_NodeLocation14#>"}, {value:"Study Room",text:"<#AiMesh_NodeLocation15#>"},
				{value:"Hallway",text:"<#AiMesh_NodeLocation16#>"}, {value:"Walk-in Closet",text:"<#AiMesh_NodeLocation17#>"}, {value:"Bathroom",text:"<#AiMesh_NodeLocation18#>"},
				{value:"First Floor",text:"<#AiMesh_NodeLocation26#>"}, {value:"Second Floor",text:"<#AiMesh_NodeLocation19#>"}, {value:"Third Floor",text:"<#AiMesh_NodeLocation20#>"},
				{value:"Storage",text:"<#AiMesh_NodeLocation21#>"}, {value:"Balcony",text:"<#AiMesh_NodeLocation22#>"}, {value:"Meeting Room",text:"<#AiMesh_NodeLocation23#>"},
				{value:"Garage",text:"<#AiMesh_NodeLocation25#>"}, {value:"Gaming Room",text:"<#AiMesh_NodeLocation27#>"}, {value:"Gym",text:"<#AiMesh_NodeLocation28#>"},
				{value:"Custom",text:"<#AiMesh_NodeLocation24#>"}
			];
		}
		var result = {"loc_text":"<#AiMesh_NodeLocation01#>", "loc_css":"Home"};
		var loc_text = "Home";
		if(_isReNode){
			if("config" in _node_info) {
				if("misc" in _node_info.config) {
					if("cfg_alias" in _node_info.config.misc) {
						if(_node_info.config.misc.cfg_alias != "")
							loc_text = _node_info.config.misc.cfg_alias;
					}
				}
			}
		}
		else{
			var alias = _node_info.alias;
			if(alias != _node_info.mac)
				loc_text = alias;
		}
		var specific_loc = aimesh_location_arr.filter(function(item, index, _array){
			return (item.value == loc_text);
		})[0];
		if(specific_loc != undefined){
			result.loc_css = specific_loc.value.replace(/\s/g,'_');
			result.loc_text = specific_loc.text;
		}
		else{
			result.loc_css = "Custom";
			result.loc_text = loc_text;
		}
		return result;
	}
	function get_model_name(_node_info){
		var result = "";
		if(_node_info.ui_model_name == undefined || _node_info.ui_model_name == "")
			result = _node_info.model_name;
		else
			result = _node_info.ui_model_name;
		return result;
	}
}
function Get_AiMesh_Offline_Container(_dut_info_arr){
	var node_mac = _dut_info_arr[0];
	var wifi_band = _dut_info_arr[1];
	var $container = $("<div>");

	var $node_cntr = $("<div>").addClass("node_container").attr({"data-node-mac":node_mac, "data-wifi-band":wifi_band, "data-node-status":"offline"}).appendTo($container);
	var $node_content_cntr = $("<div>").addClass("node_content_container").appendTo($node_cntr);
	var $icon_cb = $("<div>").addClass("icon_checkbox disabled").appendTo($node_content_cntr);
	var $node_info = $("<div>").addClass("node_info").appendTo($node_content_cntr);
	$("<div>").addClass("node_main_info").attr({"title":node_mac}).html($("<span>").html(htmlEnDeCode.htmlEncode("Offline AiMesh nodes"))).appendTo($node_info);
	$("<div>").addClass("node_sub_info").html(htmlEnDeCode.htmlEncode(node_mac)).appendTo($node_info);
	$("<div>").addClass("icon_location Home").appendTo($node_content_cntr);

	var hint = "* <#FW_note_AiMesh_offline#>";
	$("<div>").addClass("node_hint").html(hint).appendTo($node_cntr);

	return $container;
}
function Set_AiMesh_List_CB(_obj, _sel_wifi_bnad){
	var wifi_radius_is_WPA3E = (function(){
		var result = false;
		if(isSupport("wpa3-e")){
			var sec_option_id = $(_obj).closest(".profile_setting").find("#security_employee .switch_text_container").children(".selected").attr("data-option-id");
			if(sec_option_id == "radius"){
				var wifi_auth_radius = $(_obj).closest(".profile_setting").find("#select_wifi_auth_radius").children(".selected").attr("value");
				if(wifi_auth_radius == "wpa3" || wifi_auth_radius == "wpa2wpa3" || wifi_auth_radius == "suite-b")
					result = true;
			}
		}
		return result;
	})();
	const use_main_subnet = (()=>{
		//check use_main_subnet for new mode, check vlan_id for edit mode
		const is_edit_mode = ($(_obj).closest(".profile_setting").attr("data-profile-mode") == "edit") ? true : false;
		if(is_edit_mode){
			return ($(_obj).closest(".profile_setting").find("#vlan_id").length == 0) ? true : false;
		}
		else{
			return ($(_obj).closest(".profile_setting").find("#use_main_subnet").hasClass("on")) ? true : false;
		}
	})();
	Object.keys(aimesh_wifi_band_info).forEach(function(dut_mac){
		let band_2G_is_full = aimesh_wifi_band_full.node[dut_mac].band_2G;
		let band_5G_is_full = aimesh_wifi_band_full.node[dut_mac].band_5G;
		let band_6G_is_full = aimesh_wifi_band_full.node[dut_mac].band_6G;
		let support_WPA3E = aimesh_wifi_band_full.node[dut_mac].support_WPA3E;
		let support_6G = aimesh_wifi_band_full.node[dut_mac].support_6G;
		let support_wifi_band = aimesh_wifi_band_full.node[dut_mac].support_wifi_band;
		var $node_cntr = $(_obj).find("[data-node-mac='" + dut_mac + "']");
		if(aimesh_wifi_band_info[dut_mac].length > 0){//node support capability wifi_band
			$node_cntr.find(".node_hint").remove();
			$node_cntr.find(".icon_checkbox").removeClass().addClass("icon_checkbox");
			let select_band_is_enough = true;
			let select_band_is_support = {"status": true, "reason": ""};
			if(_sel_wifi_bnad == "3"){
				select_band_is_enough = (!band_2G_is_full && !band_5G_is_full) ? true : false;
			}
			else if(_sel_wifi_bnad == "1"){
				select_band_is_enough = (!band_2G_is_full) ? true : false;
			}
			else if(_sel_wifi_bnad == "2"){
				select_band_is_enough = (!band_5G_is_full) ? true : false;
			}
			else if(_sel_wifi_bnad == "4"){
				if(support_6G){
					select_band_is_enough = (!band_6G_is_full) ? true : false;
				}
				else{
					select_band_is_support.status = false;
					select_band_is_support.reason = `* This router does not support 6GHz.`;/* untranslated */
				}
			}
			else if(_sel_wifi_bnad == "5"){
				if(support_6G){
					select_band_is_enough = (!band_5G_is_full && !band_6G_is_full) ? true : false;
				}
				else{
					select_band_is_support.status = false;
					select_band_is_support.reason = `* This router does not support 6GHz.`;/* untranslated */
				}
			}
			else if(_sel_wifi_bnad == "6"){
				if(support_6G){
					select_band_is_enough = (!band_2G_is_full && !band_5G_is_full && !band_6G_is_full) ? true : false;
				}
				else{
					select_band_is_support.status = false;
					select_band_is_support.reason = `* This router does not support 6GHz.`;/* untranslated */
				}
			}
			if(use_main_subnet){
				if(!support_wifi_band){
					select_band_is_support.status = false;
					select_band_is_support.reason = `* This router does not support the function of using the same subnet as the main network.`;/* untranslated */
				}
			}

			if(select_band_is_support.status){
				if(select_band_is_enough){
					if(wifi_radius_is_WPA3E){
						if(support_WPA3E)
							$node_cntr.find(".icon_checkbox").addClass("clicked");
						else
							set_node_cb_closed_WPA3E();
					}
					else
						$node_cntr.find(".icon_checkbox").addClass("clicked");
				}
				else{
					set_node_cb_closed();
				}
			}
			else{
				set_node_cb_closed_noSupport(select_band_is_support.reason);
			}
		}
		function set_node_cb_closed(){
			$node_cntr.find(".icon_checkbox").addClass("closed");
			var wifi_text = "";
			if(band_2G_is_full)
				wifi_text += "2.4GHz";
			if(band_5G_is_full){
				if(wifi_text != "")
					wifi_text += " / ";
				wifi_text += "5GHz";
			}
			if(support_6G && band_6G_is_full){
				if(wifi_text != "")
					wifi_text += " / ";
				wifi_text += "6GHz";
			}
			$("<div>").addClass("node_hint")
				.html(htmlEnDeCode.htmlEncode("* The WiFi interface has reached the maximum. (" + wifi_text + ")"))/* untranslated */
				.appendTo($node_cntr);
		}
		function set_node_cb_closed_WPA3E(){
			$node_cntr.find(".icon_checkbox").addClass("closed");
			$("<div>").addClass("node_hint")
				.html(htmlEnDeCode.htmlEncode("* This router does not support WPA3-Enterprise."))/* untranslated */
				.appendTo($node_cntr);
		}
		function set_node_cb_closed_noSupport(reason){
			$node_cntr.find(".icon_checkbox").addClass("closed");
			$("<div>").addClass("node_hint")
				.html(htmlEnDeCode.htmlEncode(reason))
				.appendTo($node_cntr);
		}
	});
}
function Set_AiMesh_List_CB_MLO(_obj){
	const cap_mlo_band = aimesh_wifi_mlo_info.cap.mlo_band_num;
	Object.keys(aimesh_wifi_band_info).forEach(function(dut_mac){
		const $node_cntr = $(_obj).find("[data-node-mac='" + dut_mac + "']");
		$node_cntr.find(".node_hint").remove();
		$node_cntr.find(".icon_checkbox").removeClass().addClass("icon_checkbox");
		const node_mlo_info = aimesh_wifi_mlo_info.node.find(function(item){
			return item.mac == dut_mac;
		});
		let band_2G_is_full = false;
		let band_5G_is_full = false;
		let band_6G_is_full = false;
		if(node_mlo_info != undefined && node_mlo_info.support){
			let node_band_enough = true;
			const node_wifi_band_info = aimesh_wifi_band_info[dut_mac];
			$.each(node_wifi_band_info, function(index, wifi_band){
				const band = parseInt(wifi_band.band);
				if(band & cap_mlo_band){
					if(wifi_band.used >= wifi_band.no_used || wifi_band.used >= wifi_band_maximum){
						node_band_enough = false;
						switch(band){
							case 1:
								band_2G_is_full = true;
								break;
							case 2:
							case 4:
							case 8:
								band_5G_is_full = true;
								break;
							case 16:
							case 32:
							case 64:
								band_6G_is_full = true;
								break;
						}
					}
				}
			});
			if(node_band_enough){
				$node_cntr.find(".icon_checkbox").addClass("clicked");
			}
			else{
				set_node_cb_closed();
			}
		}
		else{
			set_node_cb_closed_noSupport();
		}

		function set_node_cb_closed(){
			$node_cntr.find(".icon_checkbox").addClass("closed");
			var wifi_text = "";
			if(band_2G_is_full)
				wifi_text += "2.4GHz";
			if(band_5G_is_full){
				if(wifi_text != "")
					wifi_text += " / ";
				wifi_text += "5GHz";
			}
			if(band_6G_is_full){
				if(wifi_text != "")
					wifi_text += " / ";
				wifi_text += "6GHz";
			}
			$("<div>").addClass("node_hint")
				.html(htmlEnDeCode.htmlEncode(`* The WiFi interface has reached the maximum. (${wifi_text})`))/* untranslated */
				.appendTo($node_cntr);
		}
		function set_node_cb_closed_noSupport(){
			$node_cntr.find(".icon_checkbox").addClass("closed");
			$("<div>").addClass("node_hint")
				.html(htmlEnDeCode.htmlEncode(`* <#CTL_nonsupported#> - MLO`))
				.appendTo($node_cntr);
		}
	});
}
function Get_Component_VPN_Profiles(){
	var $container = $("<div>").addClass("VPN_list_container").attr({"data-container": "VPN_Profiles"});

	var $vpnc_group = $("<div>").addClass("profile_group").appendTo($container);
	$("<div>").addClass("title").html("<#vpnc_title#>").appendTo($vpnc_group);
	var vpnc_options = [];
	vpnc_profile_list.forEach(function(item){
		vpnc_options.push({"type":"vpnc", "text":item.desc, "value":item.vpnc_idx, "proto":item.proto, "activate":item.activate});
	});
	if(vpnc_options.length > 0){
		var $content_cntr = $("<div>").addClass("profile_container").appendTo($vpnc_group);
		$.each(vpnc_options, function(index, value){
			Get_Component_VPN(value).appendTo($content_cntr);
		});
	}
	else{
		$("<div>").addClass("profile_hint").html(str_noProfile_hint).appendTo($vpnc_group);
	}
	var $btn_cntr = $("<div>").addClass("profile_btn_container").unbind("click").click(function(e){
		e = e || event;
		e.stopPropagation();
		if(is_Web_iframe){
			top.location.href = "/Advanced_OpenVPNClient_Content.asp";
		}
		else if(parent.webWrapper){
			top.location.href = "/index.html?url=vpnc&current_theme=white";
		}
		else{
			top.location.href = "/VPN/vpnc.html" + ((typeof theme == "string" && theme != "") ? "?current_theme=" + theme + "" : "");
		}
	}).appendTo($vpnc_group);
	$("<div>").html("<#btn_goSetting#>").appendTo($btn_cntr);
	$("<div>").addClass("icon_arrow_right").appendTo($btn_cntr);

	if(vpns_rl_json.length > 0){
		var $vpns_group = $("<div>").addClass("profile_group").appendTo($container);
		$("<div>").addClass("title").html("<#BOP_isp_heart_item#>").appendTo($vpns_group);
		var $profile_cntr = $("<div>").addClass("profile_container").appendTo($vpns_group);
		$.each(vpns_rl_json, function(index, vpn_type){
			Get_Component_VPN({"type":"vpns", "text":vpn_type.text, "value":vpn_type.vpns_idx, "activate":vpn_type.activate}).appendTo($profile_cntr);
		});
		var $btn_cntr = $("<div>").addClass("profile_btn_container").unbind("click").click(function(e){
			e = e || event;
			e.stopPropagation();
			if(is_Web_iframe){
				top.location.href = "/Advanced_VPNServer_Content.asp";
			}
			else if(parent.webWrapper){
				top.location.href = "/index.html?url=vpns&current_theme=white";
			}
			else{
				top.location.href = "/VPN/vpns.html" + ((typeof theme == "string" && theme != "") ? "?current_theme=" + theme + "" : "");
			}
		}).appendTo($vpns_group);
		$("<div>").html("<#btn_goSetting#>").appendTo($btn_cntr);
		$("<div>").addClass("icon_arrow_right").appendTo($btn_cntr);
	}
	if($container.find(".rwd_icon_radio.clicked").length == 0){
		$container.find(".rwd_icon_radio[data-vpn-type=vpnc]").first().addClass("clicked");
	}
	return $container;

	function Get_Component_VPN(_profile){
		var $profile_cntr = $("<div>").addClass("profile_item")
			.unbind("click").click(function(e){
				e = e || event;
				e.stopPropagation();
				if($(this).find(".rwd_icon_radio").attr("data-disabled") == "true")
					return false;
				$(this).closest("[data-container=VPN_Profiles]").find(".rwd_icon_radio").removeClass("clicked");
				$(this).find(".rwd_icon_radio").addClass("clicked");
			});
		var $text_container = $("<div>").addClass("text_container").appendTo($profile_cntr)
		$("<div>").html(htmlEnDeCode.htmlEncode(_profile.text)).appendTo($text_container);

		var $select_container = $("<div>").addClass("select_container").appendTo($profile_cntr);
		$("<div>").addClass("icon_warning_amber").attr({"data-component":"icon_warning"}).unbind("click").click(function(e){
			e = e || event;
			e.stopPropagation();
			$(this).closest("[data-container=VPN_Profiles]").find(".error_hint_container").remove();
			var error_hint = "<#GuestNetwork_VPN_errHint0#> <#GuestNetwork_VPN_errHint1#>";
			$(this).closest(".profile_item").append(Get_Component_Error_Hint({"text":error_hint}).show());
		}).hide().appendTo($select_container);

		$("<div>").addClass("icon_error_outline").attr({"data-component":"icon_error"}).unbind("click").click(function(e){
			e = e || event;
			e.stopPropagation();
			$(this).closest("[data-container=VPN_Profiles]").find(".error_hint_container").remove();
			var error_hint = "<#GuestNetwork_VPN_errHint2#>";
			$(this).closest(".profile_item").append(Get_Component_Error_Hint({"text":error_hint}).show());
		}).hide().appendTo($select_container);

		$("<div>").addClass("rwd_icon_radio").attr({"data-idx":_profile.value, "data-vpn-type":_profile.type}).appendTo($select_container);
		return $profile_cntr;
	}
}
var client_upload_icon_cache = [];
function Get_Component_ClientList(_dataArray){
	if(_dataArray.length == 0){
		return $("<div>").addClass("no_client_hint").html("<#AiMesh_No_Device_Connected#>");
	}
	else {
		var uploadIconMacList = getUploadIconList().replace(/\.log/g, "");
		var $clientlist_cntr = $("<div>");
		for(var i = 0; i < _dataArray.length; i += 1){
			var clientObj = _dataArray[i];
			var $client_cntr = $("<div>").addClass("client_container").appendTo($clientlist_cntr);
			var $client_icon_cntr = $("<div>").addClass("client_icon_container").appendTo($client_cntr);
			var userIconBase64 = "NoIcon";
			var deviceTitle = (clientObj.dpiDevice == "") ? clientObj.vendor : clientObj.dpiDevice;
			var isUserUpload = (uploadIconMacList.indexOf(clientObj.mac.toUpperCase().replace(/\:/g, "")) >= 0) ? true : false;
			if(isSupport("usericon")){
				if(client_upload_icon_cache[clientObj.mac] == undefined){
					var clientMac = clientObj.mac.replace(/\:/g, "");
					var queryString = "get_upload_icon()&clientmac=" + clientMac;
					$.ajax({
						url: '/appGet.cgi?hook=' + queryString,
						dataType: 'json',
						async: false,
						success: function(response){
							userIconBase64 = response.get_upload_icon;
						}
					});
					client_upload_icon_cache[clientObj.mac] = userIconBase64;
				}
				else
					userIconBase64 = client_upload_icon_cache[clientObj.mac];
			}
			if(userIconBase64 != "NoIcon"){
				if(isUserUpload) {
					$client_icon_cntr.addClass("user_icon").css("background-image", "url(" + userIconBase64 + ")");
				}else{
					$client_icon_cntr.addClass("user_icon").append($('<i>').addClass('type').attr('style','--svg:url(' + userIconBase64 + ')'));
				}
			}
			else if(clientObj.type != "0" || clientObj.vendor == ""){
				var icon_type = "type" + clientObj.type;
				$client_icon_cntr.addClass("default_icon");
				if($client_icon_cntr.find('i').length==0){
					$client_icon_cntr.append($('<i>').addClass(icon_type));
				}
				if(clientObj.type == "36"){
					$("<div>").addClass("flash").appendTo($client_icon_cntr);
				}
			}
			else if(clientObj.vendor != ""){
				var vendorIconClassName = getVendorIconClassName(clientObj.vendor.toLowerCase());
				if(vendorIconClassName != "" && !downsize_4m_support) {
					$client_icon_cntr.addClass("default_icon");
					$client_icon_cntr.append($('<i>').addClass("vendor-icon " + vendorIconClassName));
				}
				else {
					var icon_type = "type" + clientObj.type;
					$client_icon_cntr.addClass("default_icon");
					$client_icon_cntr.append($('<i>').addClass(icon_type));
				}
			}
			var $client_content_cntr = $("<div>").addClass("client_content_container").appendTo($client_cntr);
			var $main_info = $("<div>").addClass("main_info").appendTo($client_content_cntr);
			var client_name = (clientObj.nickName == "") ? clientObj.name : clientObj.nickName;
			if(client_name == "")
				client_name = clientObj.mac;
			$("<div>").attr("title", client_name).addClass("client_name").html(htmlEnDeCode.htmlEncode(client_name)).appendTo($main_info);
			var $sub_info = $("<div>").addClass("sub_info").appendTo($client_content_cntr);
			var $conn_type = $("<div>").addClass("conn_type").appendTo($sub_info);
			if(isSupport("mlo") && (clientObj.mlo == "1")){
				$conn_type.addClass("WiFi_MLO");
			}
			else{
				switch(parseInt(clientObj.isWL)){
					case 0:
						$conn_type.addClass("LAN");
						break;
					case 1:
						$conn_type.addClass("WiFi_2G");
						break;
					case 2:
					case 3:
						$conn_type.addClass("WiFi_5G");
						break;
					case 4:
						$conn_type.addClass("WiFi_6G");
						break;
				}
			}
			$("<div>").html(htmlEnDeCode.htmlEncode(clientObj.mac)).appendTo($sub_info);
			$("<div>").html(htmlEnDeCode.htmlEncode(clientObj.ip)).appendTo($sub_info);
		}
		return $clientlist_cntr;
	}
}
function checkImageExtension(imageFileObj){
	var picExtension = /\.(jpg|jpeg|png|svg)$/i;
	return ((picExtension.test(imageFileObj)) ? true : false);
}
function uploadFreeWiFi(_setting){
	var splash_page_setting = {
		'splash_page_setting':{
			'image': _setting.image,
			"continue_btn": encode_decode_text('<#FreeWiFi_Continue#>', 'encode'),
			"auth_type": encode_decode_text(_setting.auth_type, 'encode')
		}
	};
	if(_setting.cp_idx == "2"){
		splash_page_setting.splash_page_setting["portal_type"] = "2";
		splash_page_setting.splash_page_setting["brand_name"] = encode_decode_text(_setting.brand_name, "encode");
		splash_page_setting.splash_page_setting["terms_service_enabled"] = encode_decode_text(_setting.terms_service_enabled, "encode");
		splash_page_setting.splash_page_setting["terms_service_content"] = encode_decode_text(_setting.terms_service, "encode");
		splash_page_setting.splash_page_setting["terms_service_cb_text"] = encode_decode_text("<#FreeWiFi_Agree_Terms_Service#>", "encode");
		splash_page_setting.splash_page_setting["terms_service_title"] = encode_decode_text("<#Captive_Portal_Terms_Service#>", 'encode');
		splash_page_setting.splash_page_setting["agree_btn"] = encode_decode_text("<#CTL_Agree#>", 'encode');
		if(_setting.auth_type == "1"){
			splash_page_setting.splash_page_setting["passcode_blank"] = encode_decode_text('<#FreeWiFi_Passcode#> : <#JS_fieldblank#>', 'encode');
			splash_page_setting.splash_page_setting["passcode_placeholder"] = encode_decode_text('Enter Passcode', 'encode');
		}
	}
	else if(_setting.cp_idx == "4"){
		splash_page_setting.splash_page_setting["portal_type"] = "4";
		splash_page_setting.splash_page_setting["MB_desc"] = encode_decode_text(_setting.MB_desc, "encode");
	}
	var postData = {
		"splash_page_id": _setting.id,
		"splash_page_attribute": JSON.stringify(splash_page_setting)
	};
	httpApi.uploadFreeWiFi(postData);
}
function encode_decode_text(_string, _type) {
	var _string_temp = _string;
	switch(_type) {
		case "encode" :
			//escaped character
			 _string_temp = _string_temp.replace(/"/g, '\"').replace(/\\/g, "\\");
			//replace new line
			_string_temp = _string_temp.replace(/(?:\r\n|\r|\n)/g, '<br>');
			//encode ASCII
			_string_temp = encodeURIComponent(_string_temp).replace(/[!'()*]/g, escape);
			break;
		case "decode" :
			_string_temp = decodeURIComponent(_string_temp);
			_string_temp = _string_temp.replace(/\<br\>/g, "\n");
			break
	}
	return _string_temp;
}
function convertRulelistToJson(attrArray, rulelist){
	var rulelist_json = [];
	var each_rule = rulelist.split("<");
	var convertAtoJ = function(rule_array){
		var rule_json = {}
		$.each(attrArray, function(index, value){
			rule_json[value] = rule_array[index];
		});
		return rule_json;
	}
	$.each(each_rule, function(index, value){
		if(value != ""){
			var one_rule_array = value.split(">");
			var one_rule_json = convertAtoJ(one_rule_array);
			if(!one_rule_json.error) rulelist_json.push(one_rule_json);
		}
	});
	return rulelist_json;
}

//vlan_trunklist=<0C:9D:92:47:06:50>1#51,91>4#53
function update_vlan_trunklist(update_profile, _new_vid){
	
	var trunklist = "";
	var target_term11 = "#"+update_profile.vid+",";
	var target_term12 = "#"+update_profile.vid+">";
	var target_term13 = "#"+update_profile.vid+"<";
	var target_term14 = "#"+update_profile.vid+"_";
	var target_term21 = ","+update_profile.vid+",";
	var target_term22 = ","+update_profile.vid+">";
	var target_term23 = ","+update_profile.vid+"<";
	var target_term24 = ","+update_profile.vid+"_";
	var replace_term11 = "#"+_new_vid+",";
	var replace_term12 = "#"+_new_vid+">";
	var replace_term13 = "#"+_new_vid+"<";
	var replace_term14 = "#"+_new_vid+"_";
	var replace_term21 = ","+_new_vid+",";
	var replace_term22 = ","+_new_vid+">";
	var replace_term23 = ","+_new_vid+"<";
	var replace_term24 = ","+_new_vid+"_";

	trunklist = vlan_trunklist_tmp.replaceAll(target_term11, replace_term11);
	trunklist = trunklist.replaceAll(target_term12, replace_term12);
	trunklist = trunklist.replaceAll(target_term13, replace_term13);
	trunklist = trunklist.replaceAll(target_term14, replace_term14);
	trunklist = trunklist.replaceAll(target_term21, replace_term21);
	trunklist = trunklist.replaceAll(target_term22, replace_term22);
	trunklist = trunklist.replaceAll(target_term23, replace_term23);
	trunklist = trunklist.replaceAll(target_term24, replace_term24);
	if(trunklist[trunklist.length - 1] == "_"){	//remove last "_" char
		trunklist = trunklist.substring(0, trunklist.length - 1);
	}

	vlan_trunklist_tmp = trunklist+"_";	//eazy to replace
	return trunklist;
}

function rm_vid_from_vlan_trunklist(_rm_vid){

	var trunklist = "";
	//case: remove choosed vid
	$.each(vlan_trunklist_json, function(idx, items) {

		if(vlan_trunklist_json[idx].profile == _rm_vid){
			//alert(vlan_trunklist_json[idx].mac+"/"+vlan_trunklist_json[idx].port);		//matched mac & port

			for(var z=vlan_trunklist_port_array_one_mac.length-1; z>0; z--){	//by mac
				if(vlan_trunklist_json[idx].mac == vlan_trunklist_port_array_one_mac[z][0]){

					for(var x=vlan_trunklist_port_array_one_mac[z].length-1; x>0; x--){
						if(vlan_trunklist_port_array_one_mac[z][x].indexOf(vlan_trunklist_json[idx].port+"#") >= 0){
							vlan_trunklist_port_array_one_mac[z].splice(x, 1);
						}
					}
				}
			}

			vlan_trunklist_json_tmp.splice(idx, 1);
		}
	});
	vlan_trunklist_json = vlan_trunklist_json_tmp;	//sync

	vlan_trunklist_tmp = "";	//clean vlan_trunklist_tmp
	for(var t=1; t<vlan_trunklist_port_array_one_mac.length; t++){

		if(vlan_trunklist_port_array_one_mac[t].length >= 2){	//remove mac if no lan port binded
			for(var s=0; s<vlan_trunklist_port_array_one_mac[t].length; s++){
				vlan_trunklist_tmp += (s==0)?"<":">";
				vlan_trunklist_tmp += vlan_trunklist_port_array_one_mac[t][s];
			}
		}
	}

	vlan_trunklist_tmp += "_";	//eazy to replace

	var target_term11 = "#"+_rm_vid+",";
	var target_term12 = "#"+_rm_vid+">";
	var target_term13 = "#"+_rm_vid+"<";
	var target_term14 = "#"+_rm_vid+"_";
	var target_term21 = ","+_rm_vid+",";
	var target_term22 = ","+_rm_vid+">";
	var target_term23 = ","+_rm_vid+"<";
	var target_term24 = ","+_rm_vid+"_";	
	var replace_term11 = "#";
	var replace_term12 = ">";
	var replace_term13 = "<";
	var replace_term14 = "_";
	var replace_term21 = ",";
	var replace_term22 = ">";
	var replace_term23 = "<";
	var replace_term24 = "_";
	
	//case: remove only one vid 
	trunklist = vlan_trunklist_tmp.replaceAll(target_term11, replace_term11);
	for(var x=1; x<=8; x++){
		trunklist = trunklist.replaceAll(x+target_term12, replace_term12);
		trunklist = trunklist.replaceAll(x+target_term13, replace_term13);
		trunklist = trunklist.replaceAll(x+target_term14, replace_term14);
	}
	trunklist = trunklist.replaceAll(target_term21, replace_term21);
	trunklist = trunklist.replaceAll(target_term22, replace_term22);
	trunklist = trunklist.replaceAll(target_term23, replace_term23);
	trunklist = trunklist.replaceAll(target_term24, replace_term24);
	if(trunklist[trunklist.length - 1] == "_"){	//remove last "_" char
		trunklist = trunklist.substring(0, trunklist.length - 1);
	}

	vlan_trunklist_tmp = trunklist+"_";	//eazy to replace
	return trunklist;
}

function get_showLoading_status(_rc_service){
	var result = {"time": 20, "disconnect": false};//restart_net_and_phy will disconnect and logout
	if(_rc_service != undefined){
		if(_rc_service.indexOf("restart_net_and_phy;") >= 0){
			result.time = httpApi.hookGet("get_default_reboot_time");
			result.disconnect = true;
		}
		else if(_rc_service.indexOf("restart_chilli;restart_uam_srv;") >= 0){
			result.time = 30;
		}
	}
	var profile_sec = (((sdn_all_rl_json.length - 2) > 0) ? ((sdn_all_rl_json.length - 2) * 5) : 0);
	result.time += profile_sec;//profile
	return result;
}
function check_isAlive_and_redirect(_parm){
	var page = "";
	var time = (isWLclient()) ? 35 : 30;
	var interval_time = 2;
	if(_parm != undefined){
		if(_parm.page != undefined && _parm.page != "") page = _parm.page;
		if(_parm.time != undefined && _parm.time != "" && !isNaN(_parm.time)) time = parseInt(_parm.time);
	}
	if(parent.webWrapper){
		if(page == "SDN.asp")
			page = "sdn";
	}
	var lan_ipaddr = httpApi.nvramGet(["lan_ipaddr"]).lan_ipaddr;
	setTimeout(function(){
		var interval_isAlive = setInterval(function(){
			httpApi.isAlive("", lan_ipaddr,
				function(){
					clearInterval(interval_isAlive);
					if(parent.webWrapper){
						top.pageRedirect(page);
					}
					else{
						top.location.href = "/" + page + "";
					}
				});
		}, 1000*interval_time);
	}, 1000*(time - interval_time));
}
function secondsToHMS(sec){
	var sec = Number(sec);
	var h = Math.floor(sec / (60 * 60));
	var m = Math.floor(sec % (60 * 60) / 60);
	var s = Math.floor(sec % (60 * 60) % 60);
	var result = {"hours": 0, "minutes": 0, "seconds": 0};
	result.hours = h > 0 ? h : 0;
	result.minutes = m > 0 ? m : 0;
	result.seconds = s > 0 ? s : 0;
	result.hours = num_add_left_pad(result.hours, 2);
	result.minutes = num_add_left_pad(result.minutes, 2);
	result.seconds = num_add_left_pad(result.seconds, 2);
	return result;

	function num_add_left_pad(_num, _len){
		if(_len == undefined) _len = 2;
		return ("0".repeat(_len) + _num).slice(-_len);
	}
}
function trigger_keyup(inputObj){
	$(inputObj).keyup();
}
function calculatorIPPoolRange(ipaddr, mask){
	let ip_range = {"start": "", "end": ""};
	if(ipaddr == "" || mask == "" || ipaddr == undefined || mask == undefined)
		return ip_range;
	let ipaddr_substr = ipaddr.substr(0, ipaddr.lastIndexOf("."));
	ip_range.start = ipaddr_substr + "." + "2";
	ip_range.end = ipaddr_substr + "." + "254";

	let gatewayIPArray = ipaddr.split(".");
	let netMaskArray = mask.split(".");
	let ipPoolStartArray = [];
	let ipPoolEndArray = [];

	ipPoolStartArray[0] = (gatewayIPArray[0] & 0xFF) & (netMaskArray[0] & 0xFF);
	ipPoolStartArray[1] = (gatewayIPArray[1] & 0xFF) & (netMaskArray[1] & 0xFF);
	ipPoolStartArray[2] = (gatewayIPArray[2] & 0xFF) & (netMaskArray[2] & 0xFF);
	ipPoolStartArray[3] = (gatewayIPArray[3] & 0xFF) & (netMaskArray[3] & 0xFF);
	ipPoolStartArray[3] += 1;

	ipPoolEndArray[0] = (gatewayIPArray[0] & 0xFF) | (~netMaskArray[0] & 0xFF);
	ipPoolEndArray[1] = (gatewayIPArray[1] & 0xFF) | (~netMaskArray[1] & 0xFF);
	ipPoolEndArray[2] = (gatewayIPArray[2] & 0xFF) | (~netMaskArray[2] & 0xFF);
	ipPoolEndArray[3] = (gatewayIPArray[3] & 0xFF) | (~netMaskArray[3] & 0xFF);
	ipPoolEndArray[3] -= 1;

	ip_range.start = ipPoolStartArray[0] + "." + ipPoolStartArray[1] + "." + ipPoolStartArray[2] + "." + ipPoolStartArray[3];
	if(inet_network(ip_range.start) <= inet_network(ipaddr)) {
		ip_range.start = ipPoolStartArray[0] + "." + ipPoolStartArray[1] + "." + ipPoolStartArray[2] + "." + (parseInt(ipPoolStartArray[3]) + 1);
	}
	ip_range.end = ipPoolEndArray[0] + "." + ipPoolEndArray[1] + "." + ipPoolEndArray[2] + "." + ipPoolEndArray[3];

	return ip_range;
}
