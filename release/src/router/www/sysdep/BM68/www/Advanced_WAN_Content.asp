<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Transitional//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd">
<html xmlns="http://www.w3.org/1999/xhtml">
<html xmlns:v>
<head>
<meta http-equiv="X-UA-Compatible" content="IE=Edge"/>
<meta http-equiv="Content-Type" content="text/html; charset=utf-8" />
<meta HTTP-EQUIV="Pragma" CONTENT="no-cache">
<meta HTTP-EQUIV="Expires" CONTENT="-1">
<link rel="shortcut icon" href="images/favicon.png">
<link rel="icon" href="images/favicon.png">
<title><#Web_Title#> - <#menu5_3_1#></title>
<link rel="stylesheet" type="text/css" href="index_style.css">
<link rel="stylesheet" type="text/css" href="form_style.css">
<link rel="stylesheet" type="text/css" href="/RWD_UI/rwd_component.css">
<script type="text/javascript" src="/state.js"></script>
<script type="text/javascript" src="/general.js"></script>
<script type="text/javascript" src="/popup.js"></script>
<script type="text/javascript" src="/help.js"></script>
<script type="text/javascript" src="/validator.js"></script>
<script type="text/javascript" src="/js/jquery.js"></script>
<script type="text/javascript" src="/js/httpApi.js"></script>
<script type="text/javascript" src="/RWD_UI/rwd_component.js"></script>
<style>
.FormTable{
	margin-top:10px;
}

.addBtn{
	display: flex;
	justify-content: center;
	align-items: center;
	width: 108px;
	background: #248DFF;
	border-radius: 8px;
	margin-left: 10px;
	cursor: pointer;
}

.splitLine_dns{
	background: #32393B;
	background: -webkit-linear-gradient(#32393B 0%, #32393B 20%, #667881 80%, #667881 100%);
	background: -o-linear-gradient(#32393B 0%, #32393B 20%, #667881 80%, #667881 100%);
	background: linear-gradient(#32393B 0%, #32393B 20%, #667881 80%,#667881 100%);
	height: 2px;
	max-width:440px;
	margin: 1px 0;
}
.splitLine_dns_bussiness{
	background: #32393B;
	background: -webkit-linear-gradient(#32393B 0%, #32393B 20%, #667881 80%, #667881 100%);
	background: -o-linear-gradient(#32393B 0%, #32393B 20%, #667881 80%, #667881 100%);
	background: linear-gradient(#32393B 0%, #32393B 20%, #667881 80%,#667881 100%);
	height: 2px;
	max-width:600px;
	margin: 1px 0;
}
.assign_dns{
	color:#FFF;
	opacity: 0.7;
	max-width:330px;
}
.assign_dns_bussiness{
	color:#000;
	opacity: 0.7;
	max-width:600px;
}
#dns_list_Block{
	font-family: Arial, Helvetica, MS UI Gothic, MS P Gothic, Microsoft Yahei UI, sans-serif;
	border: 1px solid #FFF;
}
.dnslist_viewlist {
	position: absolute;
	-webkit-border-radius: 5px;
	-moz-border-radius: 5px;
	border-radius: 5px;
	z-index: 200;
	background-color:#444f53;
	margin-left: 10%;
	margin-top: -1500px;
	width: 80%;
	height:auto;
	box-shadow: 3px 3px 10px #000;
	display:block;
	overflow: auto;
}
.dnslist_viewlist_business{
	position: absolute;
	width: 70%;
    margin-left: 100px;
    margin-top: -920px;
    left: 15%;
	min-width: 950px;
    background: rgb(255, 255, 255);
    color: rgb(24, 24, 24);
	border: 1px solid rgba(255, 255, 255, 0.02);
    box-shadow: 0px 2px 4px 0px rgb(0 0 0 / 8%), 0px 1px 4px 0px rgb(60 60 60 / 10%);
    border-radius: 8px;
	z-index: 200;
}

.aftr_Block_PC{
	border:1px outset #999;
	background-color:#576D73;
	position:absolute;
	*margin-top:26px;	
	margin-left:2px;
	*margin-left:-353px;
	width:346px;
	text-align:left;	
	height:auto;
	overflow-y:auto;
	z-index:200;
	padding: 1px;
}
.aftr_Block_PC div{
	background-color:#576D73;
	height:auto;
	*height:20px;
	line-height:20px;
	text-decoration:none;
	font-family: Lucida Console;
	padding-left:2px;
}

.aftr_Block_PC a{
	background-color:#EFEFEF;
	color:#FFF;
	font-size:12px;
	font-family:Arial, Helvetica, sans-serif;
	text-decoration:none;	
}
.aftr_Block_PC div:hover{
	background-color:#3366FF;
	color:#FFFFFF;
	cursor:default;
}

.aftr_Block_PC_WHITE {
	background-color:#f5f5f5;
	border: solid 1px #f5f5f5;
	width: 60% !important;
	box-shadow: 0px 12px 12px rgb(0 0 0 / 10%);
}
.aftr_Block_PC_WHITE div {
	background-color:#f5f5f5;
	border: solid 1px #f5f5f5;
}

.aftr_Block_PC_WHITE a {
	background-color:#ffffff;
	color:#262626;
}
.aftr_Block_PC_WHITE a div:hover, .aftr_Block_PC_WHITE_expand:hover{
	background-color:#ffffff;
	color:#248dff;
	border: solid 1px #248dff;
	border-radius: 3px;
}

#autowan_hint_div{
	position: absolute;
	z-index: 1000;
	width: 600px;
	height: 550px;
	margin-left: 40%;
	background-color: #232629;
	box-shadow: 3px 3px 10px #000;
	border-radius: 4px;
	border: 2px solid #818181;
	font-size: 13px;
}

.port_img{
	position: absolute;
	width: 480px;
	height: 330px;
	background: url('images/model_port.png') no-repeat center;
}

.port_plugin_img{
	position: absolute;
	width: 480px;
	height: 330px;
	background: url('images/wanport_plugin.png') no-repeat center;
}

</style>
<script>
if(isSupport("BUSINESS")){
	$('link').last().after('<link group="extend_css" rel="stylesheet" type="text/css" href="/RWD_UI/rwd_component_WHITE.css">');
}
else if(isSupport("ROG_UI")){
	$('link').last().after('<link group="extend_css" rel="stylesheet" type="text/css" href="/RWD_UI/rwd_component_ROG.css">');
}

<% login_state_hook(); %>
<% wan_get_parameter(); %>

var wan_pvc_enabled = ["0", "0", "0", "0", "0", "0", "0", "0"];

var getUrlParameter = function getUrlParameter(param){
	var url_parm = parent.window.location.search.substring(1);
	var parm_array = url_parm.split("&");
	var key_value;

	for(var i = 0; i < parm_array.length; i += 1){
		key_value = parm_array[i].split("=");
		if (key_value[0] == param) {
			return typeof key_value[1] == "undefined" ? "" : decodeURIComponent(key_value[1]);
		}
	}
	return "";
};
var theme = getUrlParameter("current_theme").toUpperCase();

var wans_dualwan = '<% nvram_get("wans_dualwan"); %>';
var original_wan_dhcpenable = parseInt('<% nvram_get("wan_dhcpenable_x"); %>');
var original_dnsenable = parseInt('<% nvram_get("wan_dnsenable_x"); %>');
var original_ppp_echo = parseInt('<% nvram_get("wan_ppp_echo"); %>');
var default_ppp_echo = parseInt('<% nvram_default_get("wan_ppp_echo"); %>');
var orig_mtu = '<% nvram_get("wan_mtu"); %>';

var chg_pvc_unit_flag = '<% get_parameter("chg_pvc"); %>';

var load_wan_unit = '<% nvram_get("wan_unit"); %>';
if(load_wan_unit.length == 3)
	load_wan_unit=load_wan_unit.substring(1, 2);
var load_wan0_proto = '<% nvram_get("wan0_proto"); %>';
var load_wan1_proto = '<% nvram_get("wan1_proto"); %>';
var wan_proto_orig = '<% nvram_get("wan_proto"); %>';
var original_wan_type = wan_proto_orig;
var wan_unit_tmp="";
var rc_service_string = "restart_wan_if " + load_wan_unit;
var applyData = {"action_mode": "apply", "rc_service": rc_service_string };
var cloud_dns_list = [];
var original_switch_stb_x = '<% nvram_get("switch_stb_x"); %>';

var ipv6_service_orig = '<% nvram_get("ipv6_service"); %>';

if(wan_bonding_support){
	var orig_bond_wan = httpApi.nvramGet(["bond_wan"], true).bond_wan;
	var orig_wanports_bond = httpApi.nvramGet(["wanports_bond"], true).wanports_bond;
}

if(dnspriv_support){
	var dot_servers_array = [];
	var dnspriv_rulelist_array = '<% nvram_get("dnspriv_rulelist"); %>';
}
var default_reboot_time = parseInt("<% get_default_reboot_time(); %>") + 10;

var faq_href3 = "https://nw-dlcdnet.asus.com/support/forward.html?model=&type=Faq&lang="+ui_lang+"&kw=&num=127";

if(dslite_support){
	var s46_dslite_svc_str = httpApi.nvramGet(["wan0_s46_dslite_svc"], true).wan0_s46_dslite_svc;
}

if(dualWAN_support){
	var wan_type_name = wans_dualwan.split(" ")[load_wan_unit];
	wan_type_name = wan_type_name.toUpperCase();
	switch(wan_type_name){
		case "DSL":
			location.href = "Advanced_DSL_Content.asp";
			break;
		case "USB":
			location.href = "Advanced_Modem_Content.asp";
			break;
		default:
			break;
	}
}

function Check_IE_Version(){ //Edge: return 12
    var rv = -1; // Return value assumes failure.

    if (navigator.appName == 'Microsoft Internet Explorer'){

       var ua = navigator.userAgent,
           re  = new RegExp("MSIE ([0-9]{1,}[\\.0-9]{0,})");

       if (re.exec(ua) !== null){
         rv = parseFloat( RegExp.$1 );
       }
    }
    else if(navigator.appName == "Netscape"){                       
       /// in IE 11 the navigator.appVersion says 'trident'
       /// in Edge the navigator.appVersion does not say trident
       if(navigator.appVersion.indexOf('Trident') === -1 && navigator.appVersion.indexOf('Edge') >= 0) rv = 12;
       else rv = 11;
    }       

    return rv;          
}
var IE_Version = Check_IE_Version();

// get Ethernet WAN setting list from hook by wan_unit
// wan_proto, wan_dot1q, wan_vid
var MSWANList = [ <% get_MS_WAN_list(); %> ];
var wan_service_num = 0;
for(var i = 0; i < MSWANList.length; i++){
	if (MSWANList[i][0] != "0") {	//enabled unit
		wan_service_num++;
	}
}

function clean_applyData(){ //remove values of hidden settings from applyData
	Object.keys(applyData).forEach(function(key){
		if(key != "action_mode" && key != "rc_service"){
			var id = "#" + key;
			if($(id).is(":hidden")){
				delete applyData[key];
			}
		}
	});
}

function save_applyData(wan_unit){
	var pvc_shift = (load_wan_unit == 1)? "110":"100";	

	// find a available PVC
	var avail_pvc = 9;
	var found_pvc = false;
	if(wan_unit == undefined){
		for(var j = 1; j < MSWANList.length; j++){
			if (MSWANList[j][0] == "0") {
				found_pvc = true;
				avail_pvc = j + parseInt(pvc_shift);
				break;
			}
		}
		if (found_pvc == false) {
				// no empty pvc , return
				return;
		}
	}
	else
		avail_pvc = wan_unit;
	applyData["wan_unit"] = avail_pvc;

	$.each($(".input_container"), function(){ 
		if($(this).is(":visible")) {
			$(this).find("*").each(function(){
				if($(this).attr("id")){
					var id = $(this).attr("id");
					if($(this).hasClass("icon_switch")){
						if($(this).hasClass("on"))
							applyData[id] = "1";
						else
							applyData[id] = "0";
					}
					else if($(this).hasClass("textInput")){
						applyData[id] = $(this).val();
					}
					else if(id.indexOf("select_") != -1){
						var $selected_option = $(this).find(".selected");
						var setting_name = id.substring("select_".length);
						applyData[setting_name] = $selected_option.attr("value");
					}
				}	
			})
		}
	});

	if($("#wan_enable").is(":visible") == false)//user_defined wan1xx_eanble is default "1"
		applyData["wan_enable"] = "1";

	if (Softwire46_support && ipv6_service_orig != "ipv6pt" &&
			(applyData["wan_proto"] == "v6plus" || applyData["wan_proto"] == "ocnvc" || applyData["wan_proto"] == "dslite")){
				
				applyData["ipv6_service"] = "ipv6pt";
				applyData["rc_service"] += ";restart_net";
	}

	if(orig_bond_wan != applyData["bond_wan"] == "1"){
		applyData["rc_service"] = "reboot";
	}

	if((dnssec_support &&
		(applyData["dnssec_enable"] != '<% nvram_get("dnssec_enable"); %>') ||
		(applyData["dnssec_check_unsigned_x"] != '<% nvram_get("dnssec_check_unsigned_x"); %>')) ||
		(dnspriv_support &&
			(applyData["dns_priv_override"] == 0) &&
			(applyData["dnspriv_enable"] != '<% nvram_get("dnspriv_enable"); %>')) ||
			(applyData["dns_norebind"] != '<% nvram_get("dns_norebind"); %>') ||
			(applyData["dns_priv_override"] != '<% nvram_get("dns_priv_override"); %>') ||
			(applyData["dns_fwd_local"] != '<% nvram_get("dns_fwd_local"); %>'))
		applyData["rc_service"] += ";restart_dnsmasq";

	var autowan_conflict = false;
	if(isSupport("autowan")){
		var orig_autowan_enable = httpApi.nvramGet(["autowan_enable"]).autowan_enable;
		if(orig_autowan_enable == "1" && (applyData["bond_wan"] == "1" || applyData["wan_proto"] == "static" || applyData["wan_proto"] == "pptp" || applyData["wan_proto"] == "l2tp"))
			autowan_conflict = true;
	}

	if(autowan_conflict){
		var hint_str = "To ensure that there are no conflicts, when you enable %1$@, the WAN port will be change to %2$@ only. Please make sure that your WAN cable is correctly plugged into the %2$@. Are you sure to continue?"
		var msg = "";
		msg = hint_str.replace("%1$@", "<#WANAggregation#>").replaceAll("%2$@", get_default_wan_name());

		$("#autowan_hint").html(msg);
		$("#autowan_hint_div").show();
		if(check_file_exists('images/model_port.png') && check_file_exists('images/wanport_plugin.png')){
			setTimeout(function(){
					if($(".port_plugin_img").is(":visible"))
						$(".port_plugin_img").fadeOut(500);
					else
						setTimeout(function(){$(".port_plugin_img").fadeIn(500);}, 500);

					if($("#autowan_hint_div").is(":visible"))
						setTimeout(arguments.callee, 1500);
				}, 1500);
		}
		else{
			$("#schematic_diagram").hide();
			$("#autowan_hint_div").css("height", "auto");
			$("#autowan_hint").css("margin", "50px 0");
			$("#hint_action_div").css("margin-bottom", "30px");
		}
	}
	else{
		showLoading();
		httpApi.nvramSet(applyData, function(){
				setTimeout(function(){
					close_popup_container("all");
					refreshpage();
				}, (applyData["rc_service"] == "reboot")? (default_reboot_time*1000):20000);
			});
	}
}

function change_popup_settingsItem(wan_proto){
	var $content_container = $(".popup_edit_profile_container");

	set_value_Custom_Select($(".popup_container.popup_element"), "wan_proto", wan_proto);
	if(wan_proto == "bridge"){
		$("#nat_enable_container").hide();
		$("#dhcpfilter_enable_container").show();
		$("#dot1q_container").show();
		$("#wan_ip_container").hide();
		$("#dns_container").hide();
		$content_container.find("[data-sec-option-id=dhcp-only]").hide();
		$("#account_settings_container").hide();
		$("#special_isp_container").hide();
	}
	else if(wan_proto == "v6plus" || wan_proto == "ocnvc" || wan_proto == "dslite"){
		$("#dhcpfilter_enable_container").hide();
		$("#dot1q_container").hide();
		$("#wan_ip_container").hide();
		$("#dnsenable_container").show();
		$("#dns_container").show();
		$content_container.find("[data-sec-option-id=dhcp-only]").hide();
		$("#account_settings_container").hide();
		$("#vpndhcp_container").hide();
		$("#vpn_container").hide();
		$("#special_isp_container").show();
	}
	else if(wan_proto == "dhcp"){
		$("#dhcpfilter_enable_container").hide();
		$("#dot1q_container").show();
		$("#wan_ip_container").hide();
		$("#dnsenable_container").show();
		$("#dns_container").show();
		$content_container.find("[data-sec-option-id=dhcp-only]").show();
		$("#account_settings_container").hide();
		$("#vpndhcp_container").hide();
		$("#vpn_container").hide();
		$("#special_isp_container").show();
	}
	else if(wan_proto == "static"){
		$("#dhcpfilter_enable_container").hide();
		$("#dot1q_container").show();
		$("#dhcp_enabled_container").hide();
		$("#static_wanip_container").show();
		$("#wan_ip_container").show();
		$("#dnsenable_container").hide();
		$("#dns_server_container").show();
		$("#dns_container").show();
		$content_container.find("[data-sec-option-id=dhcp-only]").hide();
		$("#special_isp_container").show();
		$("#account_settings_container").hide();
		$("#vpndhcp_container").hide();
		$("#vpn_container").hide();
		$("#special_isp_container").show();
	}
	else if(wan_proto == "pppoe"){
		$("#dhcpfilter_enable_container").hide();
		$("#dot1q_container").show();
		$("#dhcp_enabled_container").show();
		$("#wan_ip_container").show();
		$("#dnsenable_container").show();
		$("#dns_container").show();
		$content_container.find("[data-sec-option-id=dhcp-only]").hide();
		$("#pptp_option_container").hide();
		$("#idletime_container").show();
		$content_container.find("[data-sec-option-id=pppoe-only]").show();
		$("#account_settings_container").show();
		$("#vpndhcp_container").show();
		$("#vpn_container").hide();
		$("#special_isp_container").show();
	}
	else if(wan_proto == "pptp"){
		$("#dhcpfilter_enable_container").hide();
		$("#dot1q_container").show();
		$("#dhcp_enabled_container").show();
		$("#wan_ip_container").show();
		$("#dnsenable_container").show();
		$("#dns_container").show();
		$content_container.find("[data-sec-option-id=dhcp-only]").hide();
		$("#pptp_option_container").show();
		$("#idletime_container").show();
		$content_container.find("[data-sec-option-id=pppoe-only]").hide();
		$("#account_settings_container").show();
		$("#vpndhcp_container").hide();
		$("#vpn_container").show();
		$("#special_isp_container").show();
	}
	else if(wan_proto == "l2tp"){
		$("#dhcpfilter_enable_container").hide();
		$("#dot1q_container").show();
		$("#dhcp_enabled_container").show();
		$("#wan_ip_container").show();
		$("#dnsenable_container").show();
		$("#dns_container").show();
		$content_container.find("[data-sec-option-id=dhcp-only]").hide();
		$("#pptp_option_container").hide();
		$("#idletime_container").hide();
		$content_container.find("[data-sec-option-id=pppoe-only]").hide();
		$("#account_settings_container").show();
		$("#vpndhcp_container").hide();
		$("#vpn_container").show();
		$("#special_isp_container").show();
	}

	if($("#wan_dhcpenable_x").is(":visible")){
		if($("#wan_dhcpenable_x").hasClass("on")){
			$("#static_wanip_container").hide();
		}
		else{
			$("#static_wanip_container").show();
		}
	}

	if($("#wan_dnsenable_x").is(":visible")){
		if($("#wan_dnsenable_x").hasClass("on")){
			$("#dns_server_container").hide();
		}
		else{
			$("#dns_server_container").show();
		}
	}
}

function Get_Component_Setting_Profile(type){//internet, user_defined
	var $settings_container = $("<div>").addClass("popup_edit_profile_container");


	$settings_container.append(Get_Component_Popup_Profile_Title("Profile"));
	var $content_container = $("<div>").addClass("profile_setting");
	$content_container.addClass("popup_content_container");
	$content_container.appendTo($settings_container);
/*
	var profile_name_parm = {"title":"<#vpn_ipsec_Profile_Name#>", "type":"text", "id":"profile_name", "need_check":true};
	$content_container.append(Get_Component_Input(profile_name_parm));
*/
	if(type == "user_defined")
		var connection_type_options = [{"text":"<#BOP_ctype_title1#>","value":"dhcp", "title":"DHCP"}, {"text":"<#BOP_ctype_title5#>","value":"static"},
		{"text":"PPPoE","value":"pppoe"}, {"text":"Bridge","value":"bridge"}];
	else{
		var connection_type_options = [{"text":"<#BOP_ctype_title1#>","value":"dhcp"}, {"text":"<#BOP_ctype_title5#>","value":"static"},
		{"text":"PPPoE","value":"pppoe"}, {"text":"PPTP","value":"pptp"}, {"text":"L2TP","value":"l2tp"}];
		if(Softwire46_support){
			connection_type_options.push({"text":"<#IPv6_plus#>","value":"v6plus"});
			if(ocnvc_support)
				connection_type_options.push({"text":"<#IPv6_ocnvc#>","value":"ocnvc"});
		}
	}
	var connection_type_parm = {"title": "<#Layer3Forwarding_x_ConnectionType_itemname#>", "id": "wan_proto", "options": connection_type_options, "set_value": "dhcp"}
	var $wan_proto_select = Get_Component_Custom_Select(connection_type_parm);
	$wan_proto_select.appendTo($content_container);
	var $item = $wan_proto_select.find("#select_" + connection_type_parm.id);
	$item.children("div")
		.click(function(e){
			var option = $(this).attr("value");
			change_popup_settingsItem(option);
		})

	var wan_enabled_parm = {"title":"<#Enable_WAN#>", "type":"switch", "id":"wan_enable"};
	Get_Component_Switch(wan_enabled_parm)
		.closest(".profile_setting_item").css("display", type == "user_defined"? "none":"flex")
		.appendTo($content_container)
		.find("#" + wan_enabled_parm.id + "").click(function(e){
		e = e || event;
		e.stopPropagation();
		if($(this).hasClass("on")){
			applyData.wan_enable = "1";
		}
		else{
			applyData.wan_enable = "0";
		}
	});

	var nat_enabled_parm = {"title":"<#Enable_NAT#>", "type":"switch", "id":"wan_nat_x", "openHint":"7_22", "container_id":"nat_enable_container"};
	Get_Component_Switch(nat_enabled_parm).appendTo($content_container)
		.find("#" + nat_enabled_parm.id + "").click(function(e){
			e = e || event;
			e.stopPropagation();
			if($(this).hasClass("on")){
				applyData.wan_nat_x = "1";
			}
			else{
				applyData.wan_nat_x = "0";
			}
		});

	var upnp_enabled_parm = {"title":"<#BasicConfig_EnableMediaServer_itemname#>", "type":"switch", "id":"wan_upnp_enable", "openHint":"7_23"};
	Get_Component_Switch(upnp_enabled_parm)
		.closest(".profile_setting_item").css("display", type == "user_defined"? "none":"flex")
		.appendTo($content_container)
		.find("#" + upnp_enabled_parm.id + "").click(function(e){
			e = e || event;
			e.stopPropagation();
			if($(this).hasClass("on")){
				applyData.wan_upnp_enable = "1";
			}
			else{
				applyData.wan_upnp_enable = "0";
			}
		});

	if(wan_bonding_support){
		var bond_wan_parm = {"title":"<#WANAggregation#>", "type":"switch", "id":"bond_wan"};
			Get_Component_Switch(bond_wan_parm)
				.closest(".profile_setting_item").css("display", type == "user_defined"? "none":"flex")
				.appendTo($content_container)
				.find("#" + bond_wan_parm.id + "").click(function(e){
					e = e || event;
					e.stopPropagation();
					if($(this).hasClass("on")){
						applyData.bond_wan = "1";
					}
					else{
						applyData.bond_wan = "0";
					}
				});
	}

	var dhcpfilter_enabled_parm = {"title":"<#BasicConfig_EnableMediaServer_itemname#>", "type":"switch", "id":"wan_dhcpfilter_enable", "container_id":"dhcpfilter_enable_container"};
	Get_Component_Switch(dhcpfilter_enabled_parm).appendTo($content_container)
		.find("#" + dhcpfilter_enabled_parm.id + "").click(function(e){
			e = e || event;
			e.stopPropagation();
			if($(this).hasClass("on")){
				applyData.wan_dhcpfilter_enable = "1";
			}
			else{
				applyData.wan_dhcpfilter_enable = "0";
			}
		});

	var $dot1q_container = $("<div>").attr({"id": "dot1q_container"});
	$dot1q_container.appendTo($content_container);
	$dot1q_container.append($("<div>").addClass("profile_title_item").attr({"id":"dit1q_div"}).append($("<span>").html("802.1Q")));

	var dot1q_enabled_parm = {"title":"<#WLANConfig11b_WirelessCtrl_button1name#>", "type":"switch", "id":"wan_dot1q"};
	Get_Component_Switch(dot1q_enabled_parm).appendTo($dot1q_container)
		.find("#" + dot1q_enabled_parm.id + "").click(function(e){
			e = e || event;
			e.stopPropagation();
			if($(this).hasClass("on")){
				applyData.wan_dot1q = "1";
			}
			else{
				applyData.wan_dot1q = "0";
			}
		});

	var vid_parm = {"title":"<#WANVLANIDText#> (2-4094)", "type":"text", "id":"wan_vid", "need_check":true, "maxlength":4};
	Get_Component_Input(vid_parm).appendTo($dot1q_container)
		.find("#" + vid_parm.id)
		.unbind("keypress").keypress(function(){
			return validator.isNumber(this,event);
		})
		.change(function(){
			return validator.rangeNull(this, 2, 4094, "");
		})

	var dot1p_parm = {"title":"802.1P (0-7)", "type":"text", "id":"wan_dot1p", "need_check":true, "maxlength":1};
	Get_Component_Input(dot1p_parm).appendTo($dot1q_container)
		.find("#" + dot1p_parm.id)
		.unbind("keypress").keypress(function(){
			return validator.isNumber(this,event);
		})
		.change(function(){
			return validator.rangeNull(this, 0, 7, "");
		})

	var $wan_ip_container = $("<div>").attr({"id": "wan_ip_container"})
	$wan_ip_container.appendTo($content_container);
	$wan_ip_container.append($("<div>").addClass("profile_title_item").append($("<span>").html("<#IPConnection_ExternalIPAddress_sectionname#>")));
	var dhcp_enabled_parm = {"title":"<#Layer3Forwarding_x_DHCPClient_itemname#>", "type":"switch", "id":"wan_dhcpenable_x", "container_id":"dhcp_enabled_container"};
	Get_Component_Switch(dhcp_enabled_parm).appendTo($wan_ip_container)
		.find("#" + dhcp_enabled_parm.id).click(function(e){
			e = e || event;
			e.stopPropagation();
			if($(this).hasClass("on")){
				applyData.wan_dhcpenable_x = "1";
				$("#static_wanip_container").hide();
			}
			else{
				applyData.wan_dhcpenable_x = "0";
				$("#static_wanip_container").show();
			}
		})

	var $static_wanip_container = $("<div>").attr({"id": "static_wanip_container"});
	$static_wanip_container.appendTo($wan_ip_container);
	var ip_parm = {"title":"<#IPConnection_ExternalIPAddress_itemname#>", "type":"text", "id":"wan_ipaddr_x", "need_check":true, "maxlength": 15, "openHint":"7_1"};
	Get_Component_Input(ip_parm).appendTo($static_wanip_container)
		.find("#" + ip_parm.id + "")
		.unbind("keypress").keypress(function(){
			return validator.isIPAddr(this, event);
		});

	var subnetmask_parm = {"title":"<#IPConnection_x_ExternalSubnetMask_itemname#>", "type":"text", "id":"wan_netmask_x", "need_check":true, "maxlength": 15, "openHint":"7_2"};
	Get_Component_Input(subnetmask_parm).appendTo($static_wanip_container)
		.find("#" + subnetmask_parm.id)
		.unbind("keypress").keypress(function(){
			return validator.isIPAddr(this, event);
		});

	var gateway_parm = {"title":"<#IPConnection_x_ExternalGateway_itemname#>", "type":"text", "id":"wan_gateway_x", "need_check":true, "maxlength": 15, "openHint":"7_3"};
	Get_Component_Input(gateway_parm).appendTo($static_wanip_container)
		.find("#" + gateway_parm.id)
		.unbind("keypress").keypress(function(){
			return validator.isIPAddr(this, event);
		});

	var $dns_container = $("<div>").attr({"id": "dns_container"});
	$dns_container.appendTo($content_container);
	$dns_container.append($("<div>").addClass("profile_title_item").append($("<span>").html("<#IPConnection_x_DNSServerEnable_sectionname#>")));
	var wandns_enabled_parm = {"title":"<#IPConnection_x_DNSServerEnable_itemname#>", "type":"switch", "id":"wan_dnsenable_x", "openHint":"7_12", "container_id": "dnsenable_container"};
	Get_Component_Switch(wandns_enabled_parm).appendTo($dns_container)
		.find("#" + wandns_enabled_parm.id).click(function(e){
			e = e || event;
			e.stopPropagation();
			if($(this).hasClass("on")){
				applyData.wan_dnsenable_x = "1";
				$("#dns_server_container").hide();
			}
			else{
				applyData.wan_dnsenable_x = "0";
				$("#dns_server_container").show();
			}
		});

	var $dns_server_container = $("<div>").attr({"id": "dns_server_container"});
	$dns_server_container.appendTo($dns_container);
	var dnsserver1_parm = {"title":"<#IPConnection_x_DNSServer1_itemname#>", "type":"text", "id":"wan_dns1_x", "need_check":true, "maxlength": 15, "openHint":"7_13"};
	Get_Component_Input(dnsserver1_parm).appendTo($dns_server_container)
		.find("#" + dnsserver1_parm.id)
		.unbind("keypress").keypress(function(){
			return validator.isIPAddr(this, event);
		});

	var dnsserver2_parm = {"title":"<#IPConnection_x_DNSServer2_itemname#>", "type":"text", "id":"wan_dns2_x", "need_check":true, "maxlength": 15, "openHint":"7_14"};
	Get_Component_Input(dnsserver2_parm).appendTo($dns_server_container)
		.find("#" + dnsserver2_parm.id)
		.unbind("keypress").keypress(function(){
			return validator.isIPAddr(this, event);
		});

	if(type == "internet"){
		var dns_fwd_local_parm = {"title":"<#WAN_Queries_Upstream_DNS#>", "type":"switch", "id":"dns_fwd_local", "openHint":"7_42"};
		Get_Component_Switch(dns_fwd_local_parm).appendTo($dns_container)
			.find("#" + dns_fwd_local_parm.id + "").click(function(e){
				e = e || event;
				e.stopPropagation();
				if($(this).hasClass("on")){
					applyData.dns_fwd_local = "1";
				}
				else{
					applyData.dns_fwd_local = "0";
				}
			});

		var dns_norebind_parm = {"title":"<#WAN_DNS_Rebind#>", "type":"switch", "id":"dns_norebind", "openHint":"7_43"};
		Get_Component_Switch(dns_norebind_parm).appendTo($dns_container)
			.find("#" + dns_norebind_parm.id + "").click(function(e){
				e = e || event;
				e.stopPropagation();
				if($(this).hasClass("on")){
					applyData.dns_norebind = "1";
				}
				else{
					applyData.dns_norebind = "0";
				}
			});
	}

	if(dnssec_support && type == "internet"){
		var dnssec_enabled_parm = {"title":"<#WAN_DNSSEC_Support#>", "type":"switch", "id":"dnssec_enable", "openHint":"7_44"};
		Get_Component_Switch(dnssec_enabled_parm).appendTo($dns_container)
			.find("#" + dnssec_enabled_parm.id + "").click(function(e){
				e = e || event;
				e.stopPropagation();
				if($(this).hasClass("on")){
					applyData.dnssec_enable = "1";
					$("#dnssec_check_unsigned_container").show();
				}
				else{
					applyData.dnssec_enable = "0";
					$("#dnssec_check_unsigned_container").hide();
				}
			});

		var dnssec_check_unsigned_parm = {"title":"<#WAN_Valid_Unsigned_DNSSEC#>", "type":"switch", "id":"dnssec_check_unsigned_x", "openHint":"7_45", "container_id":"dnssec_check_unsigned_container"};
		Get_Component_Switch(dnssec_check_unsigned_parm).css("display", "none").appendTo($dns_container)
			.find("#" + dnssec_check_unsigned_parm.id + "").click(function(e){
				e = e || event;
				e.stopPropagation();
				if($(this).hasClass("on")){
					applyData.dnssec_check_unsigned_x = "1";
				}
				else{
					applyData.dnssec_check_unsigned_x = "0";
				}
			});
	}

	if(type == "internet"){
		var dns_priv_override_options = [{"text":"<#Auto#>","value":"0"}, {"text":"<#checkbox_Yes#>","value":"1"}, {"text":"<#checkbox_No#>","value":"2"}];
		var dns_priv_override_parm = {"title":"<#WAN_Prevent_DoH#>", "id":"dns_priv_override", "options": dns_priv_override_options, "set_value": "0", "openHint":"7_46"};
		Get_Component_Custom_Select(dns_priv_override_parm).appendTo($dns_container);
	}

	if(dnspriv_support && type == "internet"){
		var dnspriv_options = [{"text":"<#wl_securitylevel_0#>","value":"0"}, {"text":"DNS-over-TLS (DoT)","value":"1"}];
		var dnspriv_parm = {"title":"<#WAN_DNS_Privacy#>", "id":"dnspriv_enable", "options": dnspriv_options, "set_value": "0", "openHint":"7_35"};
		Get_Component_Custom_Select(dnspriv_parm).appendTo($dns_container)
				.find("#select_" + dnspriv_parm.id).children("div").click(function(e){
					var option = $(this).attr("value");
					if(option == "1"){
						$("#dnspriv_profile_container").show();
					}
					else{
						$("#dnspriv_profile_container").hide();
					}
				})

		var dnspriv_profile_options = [{"text":"<#WAN_DNS_over_TLS_Strict#>","value":"1"}, {"text":"<#WAN_DNS_over_TLS_Opportunistic#>","value":"0"}];
		var dnspriv_profile_parm = {"title":"<#WAN_DNS_over_TLS#>", "id":"dnspriv_profile", "options": dnspriv_profile_options, "set_value": "1", "openHint":"7_36", "container_id":"dnspriv_profile_container"};
		Get_Component_Custom_Select(dnspriv_profile_parm).appendTo($dns_container)
	}

	var $dhcp_option_container = $("<div>").attr({"data-sec-option-id":"dhcp-only"});
	$dhcp_option_container.appendTo($content_container);
	$dhcp_option_container.append($("<div>").addClass("profile_title_item").append($("<span>").html("<#ipv6_6rd_dhcp_option#>")));
	var wan_vendorid_parm = {"title":"<#DHCPoption_Class#> (<#NetworkTools_option#> 60)", "type":"text", "id":"wan_vendorid", "need_check":true, "maxlength":126};
	Get_Component_Input(wan_vendorid_parm).appendTo($dhcp_option_container)
		.find("#" + wan_vendorid_parm.id)
		.unbind("keypress").keypress(function(){
			return validator.isString(this,event);
		});

	/* Client-identifier */
	var $client_id_container = $("<div>").addClass("profile_setting_item").css({"height": "100px"});
	$("<div>").addClass("title").html(htmlEnDeCode.htmlEncode("<#DHCPoption_Client#> (<#NetworkTools_option#> 61)")).appendTo($client_id_container);
	$client_id_container.appendTo($dhcp_option_container);
	var $id_settings_container = $("<div>").css({"flex-grow": "0.78"}).appendTo($client_id_container);
	var $iaid_checkbox_container = $("<div>").css({"display": "flex"}).appendTo($id_settings_container);
	$("<div>").addClass("icon_checkbox")
		.attr({"id":"wan_clientid_type", "title":"IAID/DUID"})
		.css({"margin-bottom": "10px"})
		.unbind("click").click(function(e){
			e = e || event;
			e.stopPropagation();
			if($(this).hasClass("closed"))
				return;
			$(this).toggleClass("clicked");
			if($(this).hasClass("clicked"))
				$("#wan_clientid").hide();
			else
				$("#wan_clientid").show();
		})
		.appendTo($iaid_checkbox_container);
	$("<div>")
		.css({"margin-left": "10px"})
		.html(htmlEnDeCode.htmlEncode("IAID/DUID"))
		.appendTo($iaid_checkbox_container);
	var $input_container = $("<div>").addClass("input_container").appendTo($id_settings_container);
	var $input = $("<input/>")
				.addClass("textInput")
				.attr({"id": "wan_clientid", "type": "text", "maxlength": 126, "autocomplete":"off","autocorrect":"off","autocapitalize":"off","spellcheck":"false"})
				.unbind("blur").blur(function(e){
					e = e || event;
					e.stopPropagation();
				})
				.unbind("keypress").keypress(function(){
					return validator.isString(this,event);
				})
				.on('click', function () {
					var target = this;
					setTimeout(function(){
						target.scrollIntoViewIfNeeded();//for mobile view
					},400);
				})
				.appendTo($input_container);

	var $account_settings_container = $("<div>").attr({"id": "account_settings_container"})
	$account_settings_container.appendTo($content_container);
	$account_settings_container.append($("<div>").addClass("profile_title_item").append($("<span>").html("<#PPPConnection_UserName_sectionname#>")));
	var username_parm = {"title":"<#Username#>", "type":"text", "id":"wan_pppoe_username", "need_check":true, "maxlength":64, "openHint":"7_4"};
	Get_Component_Input(username_parm).appendTo($account_settings_container)
		.find("#" + username_parm.id)
		.unbind("keypress").keypress(function(){
			return validator.isString(this, event);
		});

	var pwd_parm = {"title":"<#PPPConnection_Password_itemname#>", "type":"password", "id":"wan_pppoe_passwd", "need_check":true, "maxlength":64, "openHint":"7_5"};
	Get_Component_Input(pwd_parm).appendTo($account_settings_container)

	var ppp_auth_options = [{"text":"<#Auto#>","value":""}, {"text":"PAP","value":"pap"}, {"text":"CHAP","value":"chap"}];
	var ppp_auth_parm = {"title": "<#WANPPPAuthText#>", "id": "wan_pppoe_auth", "options": ppp_auth_options, "set_value": ""}
	$account_settings_container.append(Get_Component_Custom_Select(ppp_auth_parm));

	var pppoe_idletime_parm = {"title":"<#PPPConnection_IdleDisconnectTime_itemname#>", "type":"text", "id":"wan_pppoe_idletime", "need_check":true, "maxlength":10, "openHint":"7_6", "container_id": "idletime_container"};
	Get_Component_Input(pppoe_idletime_parm).appendTo($account_settings_container)
		.find("#" + pppoe_idletime_parm.id)
		.unbind("keypress").keypress(function(){
			return validator.isNumber(this, event);
		})
		.change(function(){
			return validator.numberRange(this, 0, 4294967295);
		})

	var pppoe_mtu_parm = {"title":"<#PPPConnection_x_PPPoEMTU_itemname#> (128 - 1492)", "type":"text", "id":"wan_pppoe_mtu", "need_check":true, "maxlength":5, "openHint":"7_7"};
	Get_Component_Input(pppoe_mtu_parm).attr({"data-sec-option-id":"pppoe-only"}).appendTo($account_settings_container)
		.find("#" + pppoe_mtu_parm.id)
		.unbind("keypress").keypress(function(){
			return validator.isNumber(this, event);
		})
		.change(function(){
			return validator.numberRange(this, 128, 1492);
		})

	var pppoe_mru_parm = {"title":"<#PPPConnection_x_PPPoEMRU_itemname#> (128 - 1492)", "type":"text", "id":"wan_pppoe_mru", "need_check":true, "maxlength":5, "openHint":"7_8"};
	Get_Component_Input(pppoe_mru_parm).attr({"data-sec-option-id":"pppoe-only"}).appendTo($account_settings_container)
		.find("#" + pppoe_mru_parm.id)
		.unbind("keypress").keypress(function(){
			return validator.isNumber(this, event);
		})
		.change(function(){
			return validator.numberRange(this, 128, 1492);
		})

	var pppoe_service_parm = {"title":"<#PPPConnection_x_ServiceName_itemname#>", "type":"text", "id":"wan_pppoe_service", "need_check":true, "maxlength":32, "openHint":"7_9"};
	Get_Component_Input(pppoe_service_parm).attr({"data-sec-option-id":"pppoe-only"}).appendTo($account_settings_container)
		.find("#" + pppoe_service_parm.id)
		.unbind("keypress").keypress(function(){
			return validator.isString(this, event);
		});

	var pppoe_ac_parm = {"title":"<#PPPConnection_x_AccessConcentrator_itemname#>", "type":"text", "id":"wan_pppoe_ac", "need_check":true, "maxlength":32, "openHint":"7_10"};
	Get_Component_Input(pppoe_ac_parm).attr({"data-sec-option-id":"pppoe-only"}).appendTo($account_settings_container)
		.find("#" + pppoe_ac_parm.id)
		.unbind("keypress").keypress(function(){
			return validator.isString(this, event);
		});

	var pppoe_hostuniq_parm = {"title":"Host-Uniq (<#Hexadecimal#>)", "type":"text", "id":"wan_pppoe_hostuniq", "need_check":true, "maxlength":256, "openHint":"7_18"};
	Get_Component_Input(pppoe_hostuniq_parm).attr({"data-sec-option-id":"pppoe-only"}).appendTo($account_settings_container)
		.find("#" + pppoe_hostuniq_parm.id)
		.unbind("keypress").keypress(function(){
			return validator.isString(this, event);
		});

	var pptp_options = [{"text":"<#Auto#>","value":"0"}, {"text":"<#No_Encryp#>","value":"-mppc"}, {"text":"MPPE 40","value":"+mppe-40"}, {"text":"MPPE 128","value":"+mppe-128"}];
	var pptp_option_parm = {"title": "<#PPPConnection_x_PPTPOptions_itemname#>", "id": "wan_pptp_options_x", "options": pptp_options, "set_value": "", "openHint":"7_17", "container_id":"pptp_option_container"}
	$account_settings_container.append(Get_Component_Custom_Select(pptp_option_parm));

	var ppp_echo_options = [{"text":"<#btn_disable#>","value":"0"}, {"text":"PPP Echo","value":"1"}, {"text":"DNS Probe","value":"2"}];
	var ppp_echo_parm = {"title": "<#PPPConnection_x_InternetDetection_itemname#>", "id": "wan_ppp_echo", "options": ppp_echo_options, "set_value": "0", "openHint":"7_31"}
	$account_settings_container.append(Get_Component_Custom_Select(ppp_echo_parm));

	var echo_interval_parm = {"title":"<#PPPConnection_x_PPPEcho_Interval#>", "type":"text", "id":"wan_ppp_echo_interval", "need_check":true, "maxlength":6, "openHint":"7_32"};
	Get_Component_Input(echo_interval_parm).appendTo($account_settings_container)
		.find("#" + echo_interval_parm.id)
		.unbind("keypress").keypress(function(){
			return validator.isNumber(this, event);
		});

	var echo_fail_parm = {"title":"<#PPPConnection_x_PPPEcho_Max_Failure#>", "type":"text", "id":"dns_delay_round", "need_check":true, "maxlength":6, "openHint":"7_34"};
	Get_Component_Input(echo_fail_parm).appendTo($account_settings_container)
		.find("#" + echo_fail_parm.id)
		.unbind("keypress").keypress(function(){
			return validator.isNumber(this, event);
		});

	var pppoe_options_parm = {"title":"<#PPPConnection_x_AdditionalOptions_itemname#>", "type":"text", "id":"wan_pppoe_options_x", "need_check":true, "maxlength":255, "openHint":"7_18"};
	Get_Component_Input(pppoe_options_parm).appendTo($account_settings_container)
		.find("#" + pppoe_options_parm.id + "")
		.unbind("keypress").keypress(function(){
			return validator.isString(this, event);
		});

	var ppp_conn_parm = {"title":"Number of PPP Connection", "type":"text", "id":"wan_ppp_conn", "need_check":true, "maxlength":1};
	Get_Component_Input(ppp_conn_parm).appendTo($account_settings_container)
		.find("#" + ppp_conn_parm.id + "")
		.unbind("keypress").keypress(function(){
			return validator.isNumber(this, event);
		});

	var $special_isp_container = $("<div>").attr({"id": "special_isp_container"})
	$special_isp_container.appendTo($content_container);
	$special_isp_container.append($("<div>").addClass("profile_title_item").append($("<span>").html("<#PPPConnection_x_HostNameForISP_sectionname#>")));
	var vpndhcp_parm = {"title":"<#PPPConnection_x_vpn_dhcp_itemname#>", "type":"switch", "id":"wan_vpndhcp", "container_id":"vpndhcp_container"};
	Get_Component_Switch(vpndhcp_parm).appendTo($special_isp_container)
		.find("#" + vpndhcp_parm.id + "").click(function(e){
			e = e || event;
			e.stopPropagation();
			if($(this).hasClass("on")){
				applyData.wan_vpndhcp = "1";
			}
			else{
				applyData.wan_vpndhcp = "0";
			}
		});

	var vpn_parm = {"title":"<#BOP_isp_heart_item#>", "type":"text", "id":"wan_heartbeat_x", "need_check":true, "maxlength":256, "openHint":"7_19", "container_id":"vpn_container"};
	Get_Component_Input(vpn_parm).appendTo($special_isp_container)
		.find("#" + vpn_parm.id + "")
		.unbind("keypress").keypress(function(){
			return validator.isString(this, event);
		});

	var host_name_parm = {"title":"<#PPPConnection_x_HostNameForISP_itemname#>", "type":"text", "id":"wan_hostname", "need_check":true, "maxlength":32, "openHint":"7_15"};
	Get_Component_Input(host_name_parm).appendTo($special_isp_container)
		.find("#" + host_name_parm.id)
		.unbind("keypress").keypress(function(){
			return validator.isString(this, event);
		});

	var $macaddr_container = $("<div>").addClass("profile_setting_item").css({"flex-wrap":"unset"});
	$macaddr_container.appendTo($special_isp_container);
	$("<a>").addClass("hintstyle").attr({"href":"javascript:void(0);"}).html(htmlEnDeCode.htmlEncode("<#PPPConnection_x_MacAddressForISP_itemname#>")).unbind("click").click(function(){
			openHint(7,16);
		}).appendTo($("<div>").addClass("title").appendTo($macaddr_container));
	var $mac_settings_container = $("<div>").css({"display":"flex", "flex-grow": "1"}).appendTo($macaddr_container);
	var $mac_input_container = $("<div>").addClass("input_container").appendTo($mac_settings_container);
	var $macaddr_input = $("<input/>")
				.addClass("textInput")
				.attr({"id": "wan_hwaddr_x", "type": "text", "maxlength": 17, "autocomplete":"off","autocorrect":"off","autocapitalize":"off","spellcheck":"false"})
				.unbind("blur").blur(function(e){
					e = e || event;
					e.stopPropagation();
				})
				.unbind("keypress").keypress(function(){
					return validator.isHWAddr(this,event);
				})
				.on('click', function () {
					var target = this;
					setTimeout(function(){
						target.scrollIntoViewIfNeeded();//for mobile view
					},400);
				})
				.appendTo($mac_input_container);
	var $mac_clone_button = $("<div>").addClass("addBtn")
									.html(htmlEnDeCode.htmlEncode("<#BOP_isp_MACclone#>"))
									.click(function(){
										$("#wan_hwaddr_x").val(login_mac_str());
										applyData.wan_hwaddr_x = $("#wan_hwaddr_x").val();
									})
									.appendTo($mac_settings_container);

	var wan_dhcp_qry_options = [{"text":"<#DHCPnormal#>","value":"0"}, {"text":"<#DHCPaggressive#>","value":"1"}, {"text":"<#Continuous_Mode#>","value":"2"}];
	var wan_dhcp_qry_parm = {"title": "<#DHCP_query_freq#>", "id": "wan_dhcp_qry", "options": wan_dhcp_qry_options, "set_value": "1", "openHint":"7_30"}
	Get_Component_Custom_Select(wan_dhcp_qry_parm).attr({"data-sec-option-id":"dhcp-only"}).appendTo($special_isp_container)

	if(type == "internet"){
		var ttl_inc_parm = {"title":"<#Extend_TTL_Value#>", "type":"switch", "id":"ttl_inc_enable", "set_value": "off"};
		Get_Component_Switch(ttl_inc_parm).appendTo($special_isp_container)
			.find("#" + ttl_inc_parm.id + "").click(function(e){
				e = e || event;
				e.stopPropagation();
				if($(this).hasClass("on")){
					applyData.ttl_inc_enable = "1";
				}
				else{
					applyData.ttl_inc_enable = "0";
				}
			});

		var ttl_spoof_parm = {"title":"<#Spoof_TTL_Value#>", "type":"switch", "id":"ttl_spoof_enable", "set_value": "off"};
		Get_Component_Switch(ttl_spoof_parm).appendTo($special_isp_container)
			.find("#" + ttl_spoof_parm.id + "").click(function(e){
				e = e || event;
				e.stopPropagation();
				if($(this).hasClass("on")){
					applyData.ttl_spoof_enable = "1";
				}
				else{
					applyData.ttl_spoof_enable = "0";
				}
			});
	}

	var wan_mtu_parm = {"title":"<#PPPConnection_x_PPPoEMTU_itemname#> (1280-1500)", "type":"text", "id":"wan_mtu", "need_check":true, "maxlength":4};
	Get_Component_Input(wan_mtu_parm).appendTo($special_isp_container)
		.find("#" + wan_mtu_parm.id + "")
		.unbind("keypress").keypress(function(){
			return validator.isNumber(this,event);
		})
		.change(function(){
			return validator.numberRange(this, 1280, 1500);
		})

	var $action_container = $("<div>").addClass("action_container").appendTo($content_container);
	var $btn_container_delete_mobile = $("<div>").addClass("btn_container delete mobile").appendTo($action_container).html("<#CTL_del#>");
	var $btn_container_apply = $("<div>").addClass("btn_container apply").attr({"id":"apply_btn"}).appendTo($action_container).html("<#CTL_apply1#>");
	$btn_container_apply.click(function(){
		save_applyData();
	});
	var $action_loading_container = $("<div>").addClass("action_container loading").appendTo($content_container);
	return $settings_container;
}

function load_profile_settings(wan_unit){
	var prefix = "wan" + wan_unit + "_";

	$.each($(".input_container"), function(){ 
		if($(this).is(":visible")) {
			$(this).find("*").each(function(){
				if($(this).attr("id") && $(this).attr("id").indexOf("select_") == -1){
					var id = $(this).attr("id");
					var nvram_name = id.replace("wan_", prefix);
					var nvram_val = httpApi.nvramGet([nvram_name])[nvram_name];

					if($(this).hasClass("icon_switch")){
						$(this).removeClass(nvram_val == "1"? "off": "on").addClass(nvram_val == "1"? "on": "off");
						if(id == "wan_dhcpenable_x"){
							if($("#wan_dhcpenable_x").hasClass("on"))
								$("#static_wanip_container").hide();
							else
								$("#static_wanip_container").show();
						}

						if(id == "wan_dnsenable_x"){
							if($("#wan_dnsenable_x").hasClass("on"))
								$("#dns_server_container").hide();
							else
								$("#dns_server_container").show();
						}

						if(id == "dnssec_enable"){
							if($("#dnssec_enable").hasClass("on"))
								$("#dnssec_check_unsigned_container").show();
							else
								$("#dnssec_check_unsigned_container").hide();
						}
					}
					else if($(this).hasClass("textInput")){
						$(this).val(nvram_val);
					}
					else if($(this).parent().hasClass("custom_select_container")){
						set_value_Custom_Select($(".popup_container.popup_element"), id, nvram_val);
					}

					if(id == "wan_proto"){
						change_popup_settingsItem(nvram_val);
					}
					else if(id == "dnspriv_enable"){
						if(nvram_val == "1"){
							$("#dnspriv_profile_container").show();
						}
						else{
							$("#dnspriv_profile_container").hide();
						}
					}
				}
			})
		}
	});

	$("#apply_btn").unbind('click')
					.click(function(){
						save_applyData(wan_unit);
					});
}

function show_popup(type, wan_unit){ //_type: new, edit
	$(".popup_element").css("display", "flex");
	$(".container").addClass("blur_effect");
	$(".popup_container.popup_element").empty();

	if(type == "new"){
		$(".popup_container.popup_element").append(Get_Component_Setting_Profile("user_defined"));
		change_popup_settingsItem("dhcp");
	}
	else if(type == "edit"){
		if(parseInt(wan_unit) < 100)
			$(".popup_container.popup_element").append(Get_Component_Setting_Profile("internet"));
		else
			$(".popup_container.popup_element").append(Get_Component_Setting_Profile("user_defined"));

		load_profile_settings(wan_unit);
	}

	//adjust_popup_container_top($(".popup_container.popup_element"), 100);
}

function chg_pvc_unit(pvc_to_chg) {
	show_popup("edit", pvc_to_chg);
}

function chg_pvc(pvc_to_chg) {	//pvc_to_chg: 0, 1, 101-109, 111-119

	var iptv_idx=0;
	disable_pvc_summary();
	enable_all_ctrl(pvc_to_chg);
	document.form.wan_unit.value = pvc_to_chg;
	
	if (pvc_to_chg != "0" && pvc_to_chg != "1") {
		if(!mswan_support){
			remove_item_from_select_bridge();
		}
		else{
			renew_wan_proto_options();
		}
	}
	else
	{
		iptv_row = 0;
		remove_bridge();	//remove beidge while edit Internet PVC
	}

	if((pvc_to_chg >= 111 && pvc_to_chg <= 119) || (pvc_to_chg >= 101 && pvc_to_chg <= 109)){
		iptv_idx = pvc_to_chg.substring(2, 3);
	}
	else{
		iptv_idx=0;	
	}
	
	change_wan_unit_idx(pvc_to_chg,iptv_idx);
	change_wan_proto_type(document.form.wan_proto.value);
	fixed_change_wan_proto_type(document.form.wan_proto.value);

	if (pvc_to_chg != "0" && pvc_to_chg != "1") {	//useless setup
		inputCtrl(document.form.wan_upnp_enable[0], 0);
		inputCtrl(document.form.wan_upnp_enable[1], 0);
	}
}

function remove_item_from_select_bridge() {
	free_options(document.form.wan_proto);
	var var_item = new Option("Bridge", "bridge");
	document.form.wan_proto.options.add(var_item);
}

function renew_wan_proto_options(){
	free_options(document.form.wan_proto);			//remove beidge while edit Internet PVC if not mswan_support
	var var_item0 = new Option("<#BOP_ctype_title1#>", "dhcp");
	var var_item1 = new Option("<#BOP_ctype_title5#>", "static");
	var var_item2 = new Option("PPPoE", "pppoe");
	var var_item3 = new Option("Bridge", "bridge");
	document.form.wan_proto.options.add(var_item0);
	document.form.wan_proto.options.add(var_item1);
	document.form.wan_proto.options.add(var_item2);
	document.form.wan_proto.options.add(var_item3);
}

function remove_bridge(){
	free_options(document.form.wan_proto);			//remove beidge while edit Internet PVC if not mswan_support
	var var_item0 = new Option("<#BOP_ctype_title1#>", "dhcp");
	var var_item1 = new Option("<#BOP_ctype_title5#>", "static");
	var var_item2 = new Option("PPPoE", "pppoe");
	var var_item3 = new Option("PPTP", "pptp");
	var var_item4 = new Option("L2TP", "l2tp");
	document.form.wan_proto.options.add(var_item0);
	document.form.wan_proto.options.add(var_item1);
	document.form.wan_proto.options.add(var_item2);
	document.form.wan_proto.options.add(var_item3);
	document.form.wan_proto.options.add(var_item4);
	if(Softwire46_support){
		var var_item5 = new Option("<#IPv6_plus#>", "v6plus");
		document.form.wan_proto.options.add(var_item5);
		if(ocnvc_support){
			var var_item6 = new Option("<#IPv6_ocnvc#>", "ocnvc");
			document.form.wan_proto.options.add(var_item6);
		}
		if(dslite_support){
			var var_item7 = new Option("DS-Lite", "dslite");
			document.form.wan_proto.options.add(var_item7);
		}
	}
}
function add_pvc() {
	var pvc_shift = (load_wan_unit==1)? "110":"100";	

	// find a available PVC
	var avail_pvc = 9;
	var found_pvc = false;
	for(var j = 1; j < MSWANList.length; j++){
			if (MSWANList[j][0] == "0") {
				found_pvc = true;
				avail_pvc = j+parseInt(pvc_shift);
				break;
			}
	}
	if (found_pvc == false) {
			// no empty pvc , return
			return;
	}

	document.form.wan_unit.value = avail_pvc; //avail_pvc.toString();	//101~109 || 111~119

	enable_all_ctrl(avail_pvc);
	if(!mswan_support){
		remove_item_from_select_bridge();
	}
	else{
		renew_wan_proto_options();
	}

	document.getElementById("t2BC").style.display = "";

	if (document.form.wan_proto.value != "bridge") {
		document.getElementById("IPsetting").style.display = "";
		document.getElementById("DNSsetting").style.display = "";
		document.getElementById("PPPsetting").style.display = "";
		document.getElementById("vpn_server").style.display = "";
	}
	else {
		document.getElementById("IPsetting").style.display = "none";
		document.getElementById("DNSsetting").style.display = "none";
		document.getElementById("PPPsetting").style.display = "none";
		document.getElementById("vpn_server").style.display = "none";
	}

	if(IE_Version == 12){	//for IE/Edge only
		setTimeout("disable_pvc_summary();",500);
	}
	else{
		disable_pvc_summary();
	}
	
	change_wan_unit_idx(avail_pvc, j);
	change_wan_proto_type(document.form.wan_proto.value);
	fixed_change_wan_proto_type(document.form.wan_proto.value);
	document.form.wan_enable[0].checked = true;	//Add to enable it

	//useless setup
	inputCtrl(document.form.wan_upnp_enable[0], 0);
	inputCtrl(document.form.wan_upnp_enable[1], 0);
}

function add_pvc_0() {
	disable_pvc_summary();
	enable_all_ctrl(load_wan_unit);
	
	remove_bridge();

	document.form.wan_proto.value = (load_wan_unit==1)? load_wan1_proto:load_wan0_proto;

	change_wan_unit_idx(load_wan_unit, 0);
	change_wan_proto_type(document.form.wan_proto.value);
	fixed_change_wan_proto_type(document.form.wan_proto.value);
	document.form.wan_enable[0].checked = true;	//Add to enable it
}

function del_pvc_sub(pvc_to_del) {
	var msg = "";
	var idx = "";
	if(pvc_to_del == 0 || pvc_to_del == 1){
		msg += "Internet, ";
		idx = 0;
	}
	else{
		idx = pvc_to_del.toString().substring(2, 3);
		msg += "IPTV "+idx.toString()+", ";
	}

	if(MSWANList[idx][2] != 0)
		msg += "VLAN ID = "+MSWANList[idx][3].toString();
	else
		msg += "802.1Q Disable";
	msg += "\n\n";
	msg += "<#pvc_del_alert_desc#>".replace("PVC", "profile");
	var answer = confirm (msg);
	if (answer == false)
		return;

	if (mtppp_support && pvc_to_del < 10) {
		document.form.wan_ppp_conn.disabled = false;
	}
	else {
		document.form.wan_ppp_conn.disabled = true;
	}

	document.form.wan_unit.value = pvc_to_del.toString();
	document.form.wan_enable.value = "0";
	del_pvc_submit();
}

function showMSWANList(){
	if(isSupport("is_ax5400_i1")){
		document.getElementById('MS_WAN_add_del').style.display = "none";
	}
	var addRow;
	var cell = new Array(8);

	if(wan_service_num == 0){
		if(!isSupport("is_ax5400_i1")){
			addRow = document.getElementById('MS_WAN_table').insertRow(2);	//0:thead 1:th 2:the 1st rule
			cell[0] = addRow.insertCell(0);
			cell[0].colSpan = "8";
			cell[0].style.color = "white";
			cell[0].innerHTML = '<center><input class="add_btn" onclick="add_pvc_0();" value=""/></center>';
		}
	}
	else{
		var row_count=0;
		var ms_pvc_shift = (load_wan_unit==1)? "110":"100";
		var ms_pvc=0;
		// wan_enable, wan_proto, wan_dot1q, wan_vid
		for(var i = 0; i < MSWANList.length; i++){			
			if (MSWANList[i][0] != "0") {
				ms_pvc = parseInt(i)+parseInt(ms_pvc_shift);
				addRow = document.getElementById('MS_WAN_table').insertRow(row_count+2);
				row_count++;
				cell[0] = addRow.insertCell(0);
				cell[0].innerHTML = "<center>"+i+"</center>";
				if(!parent.webWrapper) cell[0].style.color = "white";
				cell[1] = addRow.insertCell(1);
				if (MSWANList[i][2]=="1") cell[1].innerHTML = "<center><#checkbox_Yes#></center>";
				else cell[1].innerHTML = "<center><#checkbox_No#></center>";
				if(!parent.webWrapper) cell[1].style.color = "white";
				cell[2] = addRow.insertCell(2);
				cell[2].innerHTML = "<center>"+MSWANList[i][3]+"</center>";
				if(!parent.webWrapper) cell[2].style.color = "white";
				cell[3] = addRow.insertCell(3);
				if (MSWANList[i][1]=="pppoe") cell[3].innerHTML = "<center>PPPoE</center>";
				else if (MSWANList[i][1]=="dhcp") cell[3].innerHTML = "<center><#BOP_ctype_title1#></center>";
				else if (MSWANList[i][1]=="bridge") cell[3].innerHTML = "<center>Bridge</center>";
				else if (MSWANList[i][1]=="static") cell[3].innerHTML = "<center><#BOP_ctype_title5#></center>";
				else if (MSWANList[i][1]=="pptp") cell[3].innerHTML = "<center>PPTP</center>";
				else if (MSWANList[i][1]=="l2tp") cell[3].innerHTML = "<center>L2TP</center>";
				else if (MSWANList[i][1]=="v6plus") cell[3].innerHTML = "<center><#IPv6_plus#></center>";
				else if (MSWANList[i][1]=="ocnvc") cell[3].innerHTML = "<center><#IPv6_ocnvc#></center>";
				else if (MSWANList[i][1]=="dslite") cell[3].innerHTML = "<center>DS-Lite</center>";
				else cell[3].innerHTML = "<center>Unknown</center>";
				if(!parent.webWrapper) cell[3].style.color = "white";

				cell[4] = addRow.insertCell(4);
				if (i==0) {
					if(parent.webWrapper) 
						cell[4].innerHTML = "<center><img src=images/checked_24px_blue.svg border=0></center>";
					else
						cell[4].innerHTML = "<center><img src=images/checked_24px.svg border=0></center>";
				}
				else{
					cell[4].innerHTML = "";
				}
				if(!parent.webWrapper) cell[4].style.color = "white";

				cell[5] = addRow.insertCell(5);
				if(i==0)
					cell[5].innerHTML = '<center><span style="cursor:pointer;" onclick="chg_pvc_unit('+load_wan_unit+');"><input class="edit_btn"></span></center>';
				else
					cell[5].innerHTML = '<center><span style="cursor:pointer;" onclick="chg_pvc_unit('+ms_pvc+');"><input class="edit_btn"></span></center>';
				if(!parent.webWrapper) cell[5].style.color = "white";

				if(!isSupport("is_ax5400_i1")){
					cell[6] = addRow.insertCell(6);
					if(ms_pvc > parseInt(ms_pvc_shift))
						cell[6].innerHTML = '<center><input class="remove_btn" onclick="del_pvc_sub('+ms_pvc+');" value=""/></center>';
					if(!parent.webWrapper) cell[6].style.color = "white";
				}
			}
		}
/*
		if (row_count < 10) {
			if(!isSupport("is_ax5400_i1")){
				addRow = document.getElementById('MS_WAN_table').insertRow(row_count+2);
				cell[0] = addRow.insertCell(0);
				cell[0].colSpan = "8";
				cell[0].style.color = "white";
				if(MSWANList[0][0] != "0"){
					cell[0].innerHTML = '<center><input class="add_btn" onclick="add_pvc();" value=""/></center>';
				}
				else{
					cell[0].innerHTML = '<center><input class="add_btn" onclick="add_pvc_0();" value=""/></center>';
				}
			}
		}
*/
	}
}

function initial(){
	show_menu();

	document.form.wan_clientid.value = decodeURIComponent('<% nvram_char_to_ascii("", "wan_clientid"); %>');

	updatDNSListOnline();
	// WAN port
	genWANSoption();
	change_wan_unit(document.form.wan_unit_x);

	if(!dualWAN_support && !vdsl_support) {
		document.getElementById("WANscap").style.display = "none";
	}

	if(chg_pvc_unit_flag.length > 0 && 
		((chg_pvc_unit_flag >= 0 && chg_pvc_unit_flag <= 1) || ( chg_pvc_unit_flag >= 101 && chg_pvc_unit_flag <= 109 ) || (chg_pvc_unit_flag >= 111 && chg_pvc_unit_flag <= 119))
	){
		chg_pvc(chg_pvc_unit_flag);
	}
	else{
		if(wan_service_num > 1){
			showMSWANList();
			disable_all_ctrl();
		}
		else{
			chg_pvc(load_wan_unit);
		}
	}
	if(Softwire46_support){
		httpApi.faqURL("1050137", function(url){document.getElementById("s46reset_faq").href = url;});
	}

	var wan_type = document.form.wan_proto.value;
	if(wan_bonding_support){
		if(based_modelid == "RT-AX89U" || based_modelid == "GT-AXY16000"){
			var wan_name = wans_dualwan.split(" ")[<% nvram_get("wan_unit"); %>];
			if(typeof(wan_name) != 'undefined' && wan_name == "none")
				wan_name = wans_dualwan.split(" ")[0];
			if(typeof(wan_name) != 'undefined' && wan_name == "wan" && (wan_type == "dhcp" || wan_type == "static")){
				inputCtrl(document.form.bond_wan_radio[0], 1);
				inputCtrl(document.form.bond_wan_radio[1], 1);
				document.getElementById("wanports_bond_menu").style.display = "";
				document.form.wanports_bond.disabled = false;
			}else{
				inputCtrl(document.form.bond_wan_radio[0], 0);
				inputCtrl(document.form.bond_wan_radio[1], 0);
				document.form.bond_wan_radio.value = "0";
				document.getElementById("wanports_bond_menu").style.display = "none";
			}
		}

		change_wanAggre_desc();
		document.getElementById("wanAgg_faq").href = faq_href3;
	}

	if(dslite_support){
		showaftrList();
		if(wan_type != "dslite"){
			update_info(0);
		}
		else{
			update_info(1);
		}
	}

	if(dnspriv_support){
		$.getJSON("/dot-servers.json", function(local_data){
			var gen_dotPresets = function(data){
				$("#dotPresets").children().remove().end().append("<option value='-1'><#Select_menu_default#></option>");
				Object.keys(data).forEach(function(dns_group, dns_group_idx) {
					if(dns_group != "" && data[dns_group].length > 0) {
						var $optGroup = $("<optgroup/>");
						$optGroup.appendTo($("#dotPresets"));
						$optGroup.attr("label",dns_group);
						data[dns_group].forEach(function(item, index, array) {
							var opt_idx = "opt_" + dns_group_idx + "_" + index;
							dot_servers_array[opt_idx] = item;
							var $opt = $("<option/>");
							$opt.attr({"value": opt_idx}).text(item.dnspriv_label);
							$opt.appendTo($optGroup);
						});
					}
				});
				if($("#dotPresets option").length > 1 && $("#dnspriv_enable").val() == "1")
					$("#dot_presets_tr").show();
				else
					$("#dot_presets_tr").hide();
			};
			gen_dotPresets(local_data);
			$.getJSON("https://nw-dlcdnet.asus.com/plugin/js/dot-servers.json",
				function(cloud_data){
					if(JSON.stringify(local_data) != JSON.stringify(cloud_data)){
						if(Object.keys(cloud_data).length > 0){
							gen_dotPresets(cloud_data);
						}
					}
				}
			);
		});
	}

	if(dnssec_support){
		document.getElementById("dnssec_tr").style.display = "";
		showhide("dnssec_strict_tr", "<% nvram_get("dnssec_enable"); %>" == "1" ? 1 : 0);
	}

	if(parent.webWrapper){
		$("#DNS_Assign_splitLine").addClass("splitLine_dns_bussiness");
		$("#DNS_Assign_desc").addClass("assign_dns_bussiness");
		$("#DNS_Assign_button").css("margin", "-52px 0px 5px 630px");

	}
	else{
		$("#DNS_Assign_splitLine").addClass("splitLine_dns");
		$("#DNS_Assign_desc").addClass("assign_dns");
		$("#DNS_Assign_button").css("margin", "-38px 0 5px 0");//margin:-38px 0 5px 0;
	}
	
}

function change_wan_unit(obj){
	if(!dualWAN_support) return;

	if(obj.options[obj.selectedIndex].text == "WAN" 
		|| obj.options[obj.selectedIndex].text == "Ethernet LAN"
		|| obj.options[obj.selectedIndex].text == "Ethernet WAN"){
		
		if(load_wan_unit != obj.selectedIndex)
			document.form.current_page.value = "Advanced_WAN_Content.asp";
		else
			return;
	}
	else if(obj.options[obj.selectedIndex].text == "USB") {
		document.form.current_page.value = "Advanced_Modem_Content.asp";
	}
	else if(obj.options[obj.selectedIndex].text == "DSL"){
		document.form.current_page.value = "Advanced_DSL_Content.asp";
	}else{
		return;
	}
	document.form.wan_unit.value = obj.value;

	FormActions("apply.cgi", "change_wan_unit", "", "");
	document.form.target = "";
	document.form.submit();
}

function genWANSoption(){
	if(!dualWAN_support) return;

	for(i=0; i<wans_dualwan.split(" ").length; i++){
		var wans_dualwan_NAME = wans_dualwan.split(" ")[i].toUpperCase();
		//MODELDEP: DSL-N55U, DSL-N55U-B, DSL-AC68U, DSL-AC68R
		if(wans_dualwan_NAME == "LAN" && 
				(productid == "DSL-N55U" || productid == "DSL-N55U-B" || productid == "DSL-AC68U" || productid == "DSL-AC68R")) 
				wans_dualwan_NAME = "Ethernet WAN";
		else if(wans_dualwan_NAME == "LAN")
				wans_dualwan_NAME = "Ethernet LAN";
		if(wans_dualwan_NAME != "NONE")
			document.form.wan_unit_x.options[i] = new Option(wans_dualwan_NAME, i);
	}
	document.form.wan_unit_x.selectedIndex = load_wan_unit;
}

function change_wan_unit_idx(idx,iptv_row){

	// reset to old values
	if (idx == "0" || idx == "1") document.getElementById("pvc_sel").innerHTML = "Internet";
	else document.getElementById("pvc_sel").innerHTML = "IPTV #"+iptv_row.toString();

	wan_unit_tmp = idx.toString();
	document.form.wan_unit_x.value = load_wan_unit;
	document.form.wan_enable.value = MSWANList[iptv_row][0];
	document.form.wan_proto.value = (MSWANList[iptv_row][1]==0)?"dhcp":MSWANList[iptv_row][1];
	document.form.wan_dot1q.value = MSWANList[iptv_row][2];
	document.form.wan_vid.value = MSWANList[iptv_row][3];
	document.form.wan_dot1p.value = MSWANList[iptv_row][4];

	document.getElementById("t2BC").style.display = "";
	if (document.form.wan_proto.value != "bridge") {
		document.getElementById("IPsetting").style.display = "";
		document.getElementById("DNSsetting").style.display = "";
		document.getElementById("PPPsetting").style.display = "";
		document.getElementById("vpn_server").style.display = "";
	}
	else {
		document.getElementById("IPsetting").style.display = "none";
		document.getElementById("DNSsetting").style.display = "none";
		document.getElementById("PPPsetting").style.display = "none";
		document.getElementById("vpn_server").style.display = "none";
	}
}

function del_pvc_submit(){
	showLoading();
	document.form.submit();
}

function exit_to_main(){
	//enable_pvc_summary();
	//disable_all_ctrl();
	location.href = "Advanced_WAN_Content.asp";
}

var reboot_confirm=0;
function applyRule(){
	if(validForm()){

		if (document.form.wan_proto.value == "bridge") {
			document.getElementById('bridgePPPoE_relay').innerHTML = '<input type="hidden" name="fw_pt_pppoerelay" value="1"> ';
		}
		
		document.form.wan_unit.value = wan_unit_tmp;

		inputCtrl(document.form.wan_dhcpenable_x[0], 1);
		inputCtrl(document.form.wan_dhcpenable_x[1], 1);

		if (document.form.wan_proto.value == "static"){
			document.form.wan_dhcpenable_x.value = 0;
		}
		else if(document.form.wan_dhcpenable_x){
			if(document.form.wan_dhcpenable_x[0].checked == 1)
				document.form.wan_dhcpenable_x.value = 1;
			else
				document.form.wan_dhcpenable_x.value = 0;
		}

		if (Softwire46_support && ipv6_service_orig != "ipv6pt" &&
			(document.form.wan_proto.value == "v6plus" || document.form.wan_proto.value == "ocnvc" || document.form.wan_proto.value == "dslite"))
		{
				document.form.ipv6_service.disabled = false;
				document.form.ipv6_service.value = "ipv6pt";
				document.form.action_script.value += ";restart_net";
				document.form.action_wait.value = "30";
		}

		if( Softwire46_support && document.form.wan_proto.value == "dslite" ){
			if(document.form.wan_s46_dslite_mode.value == "1"){
				document.form.ipv6_s46_b4addr.disabled = false;
				document.form.ipv6_s46_b4addr.value = $("#ipv6_s46_b4addr_Select").val();
			}
		}

		if(wan_bonding_support){
			if (orig_bond_wan != document.form.bond_wan_radio.value || ((based_modelid == "RT-AX89U" || based_modelid == "GT-AXY16000") && orig_wanports_bond != $("#wanports_bond_menu").val())){
				document.form.bond_wan.disabled = false;
				document.form.bond_wan.value = document.form.bond_wan_radio.value;

				if(based_modelid == "RT-AX89U" || based_modelid == "GT-AXY16000"){
					document.form.wanports_bond.disabled = false;
				}
				reboot_confirm=1;
			}
		}

		var autowan_conflict = false;
		if(isSupport("autowan")){
			var orig_autowan_enable = httpApi.nvramGet(["autowan_enable"]).autowan_enable;
			if(orig_autowan_enable == "1"){
				if((wan_bonding_support && document.form.bond_wan_radio.value == "1") || document.form.wan_proto.value == "static" || document.form.wan_proto.value == "l2tp" || document.form.wan_proto.value == "pptp")
					autowan_conflict = true;
			}
		}

		if(dnspriv_support){
			if(document.form.dnspriv_enable.value == 1 && document.form.wan_unit.value < 100){
				var dnspriv_rulelist_value = "";
				for(k=0; k<document.getElementById('dnspriv_rulelist_table').rows.length; k++){
					for(j=0; j<document.getElementById('dnspriv_rulelist_table').rows[k].cells.length-1; j++){
						if(j == 0)
							dnspriv_rulelist_value += "<";
						else
							dnspriv_rulelist_value += ">";
						dnspriv_rulelist_value += document.getElementById('dnspriv_rulelist_table').rows[k].cells[j].innerHTML;
					}
				}
				document.form.dnspriv_rulelist.disabled = false;
				document.form.dnspriv_rulelist.value = dnspriv_rulelist_value;
			}
			document.form.action_script.value += ";restart_stubby";
		}

		if((dnssec_support && 
			(getRadioValue(document.form.dnssec_enable) != '<% nvram_get("dnssec_enable"); %>') ||
			(getRadioValue(document.form.dnssec_check_unsigned_x) != '<% nvram_get("dnssec_check_unsigned_x"); %>')) ||
			(dnspriv_support &&
				(document.form.dns_priv_override.value == 0) &&
				(document.form.dnspriv_enable.value != '<% nvram_get("dnspriv_enable"); %>')) ||
				(getRadioValue(document.form.dns_norebind) != '<% nvram_get("dns_norebind"); %>') ||
				(document.form.dns_priv_override.value != '<% nvram_get("dns_priv_override"); %>') ||
				(getRadioValue(document.form.dns_fwd_local) != '<% nvram_get("dns_fwd_local"); %>'))
			document.form.action_script.value += ";restart_dnsmasq";

		if(isSupport("autowan") && autowan_conflict){
			var hint_str = "To ensure that there are no conflicts, when you enable %1$@, the WAN port will be change to %2$@ only. Please make sure that your WAN cable is correctly plugged into the %2$@. Are you sure to continue?"
			var msg = "";
			msg = hint_str.replace("%1$@", "<#WANAggregation#>").replaceAll("%2$@", get_default_wan_name());

			$("#autowan_hint").html(msg);
			$("#autowan_hint_div").show();
			if(check_file_exists('images/model_port.png') && check_file_exists('images/wanport_plugin.png')){
				setTimeout(function(){
						if($(".port_plugin_img").is(":visible"))
							$(".port_plugin_img").fadeOut(500);
						else
							setTimeout(function(){$(".port_plugin_img").fadeIn(500);}, 500);

						if($("#autowan_hint_div").is(":visible"))
							setTimeout(arguments.callee, 1500);
					}, 1500);
			}
			else{
				$("#schematic_diagram").hide();
				$("#autowan_hint_div").css("height", "auto");
				$("#autowan_hint").css("margin", "50px 0");
				$("#hint_action_div").css("margin-bottom", "30px");
			}
		}
		else{
			if(reboot_confirm==1){
				if(confirm("<#AiMesh_Node_Reboot#>")){
					if((wan_proto_orig != "v6plus" && document.form.wan_proto.value == "v6plus") ||
						(wan_proto_orig != "ocnvc" && document.form.wan_proto.value == "ocnvc")){
						s46reset();	//map-e changed
					}

					FormActions("start_apply.htm", "apply", "reboot", "<% get_default_reboot_time(); %>");
					showLoading();
					document.form.submit();
				}
			}
			else{
				if((wan_proto_orig != "v6plus" && document.form.wan_proto.value == "v6plus") ||
					(wan_proto_orig != "ocnvc" && document.form.wan_proto.value == "ocnvc")){
					s46reset();	//map-e changed
				}

				showLoading();
				document.form.submit();
			}
		}
	}
}

// test if WAN IP & Gateway & DNS IP is a valid IP
// DNS IP allows to input nothing
function valid_IP(obj_name, obj_flag){
		// A : 1.0.0.0~126.255.255.255
		// B : 127.0.0.0~127.255.255.255 (forbidden)
		// C : 128.0.0.0~255.255.255.254
		var A_class_start = inet_network("1.0.0.0");
		var A_class_end = inet_network("126.255.255.255");
		var B_class_start = inet_network("127.0.0.0");
		var B_class_end = inet_network("127.255.255.255");
		var C_class_start = inet_network("128.0.0.0");
		var C_class_end = inet_network("255.255.255.255");
		
		var ip_obj = obj_name;
		var ip_num = inet_network(ip_obj.val());

		if(obj_flag == "DNS" && ip_num == -1){ //DNS allows to input nothing
			return true;
		}
		
		if(obj_flag == "GW" && ip_num == -1){ //GW allows to input nothing
			return true;
		}
		
		if(ip_num > A_class_start && ip_num < A_class_end){
		   obj_name.val( ipFilterZero(ip_obj.val()) );
			return true;
		}
		else if(ip_num > B_class_start && ip_num < B_class_end){
			alert(ip_obj.val()+" <#JS_validip#>");
			ip_obj.focus();
			return false;
		}
		else if(ip_num > C_class_start && ip_num < C_class_end){
			obj_name.val( ipFilterZero(ip_obj.val()) );
			return true;
		}
		else{
			alert(ip_obj.val()+" <#JS_validip#>");
			ip_obj.focus();
			return false;
		}	
}

function validForm(){
	var wan_type = document.form.wan_proto.value;

	if(!document.form.wan_dhcpenable_x[0].checked &&
		!(Softwire46_support && (wan_type == "lw4o6" || wan_type == "map-e" || wan_type == "v6plus" || wan_type == "ocnvc" || wan_type == "dslite"))){// Set IP address by userself
		if(!valid_IP($("#wan_ipaddr_x"), "")) return false;  //WAN IP
		if(!valid_IP($("#wan_gateway_x"), "GW"))return false;  //Gateway IP

		if(document.form.wan_gateway_x.value == document.form.wan_ipaddr_x.value){
			alert("<#IPConnection_warning_WANIPEQUALGatewayIP#>");
			return false;
		}

		// test if netmask is valid.
		var default_netmask = "";
		var wrong_netmask = 0;
		var netmask_obj = document.form.wan_netmask_x;
		var netmask_num = inet_network(netmask_obj.value);

		if(netmask_num==0){
			var netmask_reverse_num = 0;		//Viz 2011.07 : Let netmask 0.0.0.0 pass
		}else{
		var netmask_reverse_num = ~netmask_num;
		}

		if(netmask_num < 0) wrong_netmask = 1;

		var test_num = netmask_reverse_num;
		while(test_num != 0){
			if((test_num+1)%2 == 0)
				test_num = (test_num+1)/2-1;
			else{
				wrong_netmask = 1;
				break;
			}
		}
		if(wrong_netmask == 1){
			alert(netmask_obj.value+" <#JS_validip#>");
			netmask_obj.value = default_netmask;
			netmask_obj.focus();
			netmask_obj.select();
			return false;
		}
	}

	if( Softwire46_support && wan_type == "dslite" ){
		if(document.form.wan_s46_dslite_mode.value == "1"){
			if(!validator.isValidHost(document.form.ipv6_s46_aftr.value)) return false;  //AFTR Address: IPv6/domain
			if(!valid_IP($("#ipv6_s46_b4addr_Select"), "")) return false;  //B4 IPv4 Address: IPv4
		}
	}	

	if(document.form.wan_proto.value == "pppoe"	|| document.form.wan_proto.value == "pppoa" || 
		document.form.wan_proto.value == "pptp" || document.form.wan_proto.value == "l2tp")
	{
		if(!validator.string(document.form.wan_pppoe_username) || !validator.string(document.form.wan_pppoe_passwd))
			return false;

		if(!validator.numberRange(document.form.wan_pppoe_idletime, 0, 4294967295))
			return false;
	}

	if(document.form.wan_proto.value == "pppoe"){
		if(!validator.numberRange(document.form.wan_pppoe_mtu, 128, 1492)
			|| !validator.numberRange(document.form.wan_pppoe_mru, 128, 1492))
			return false;

		if(document.form.wan_mtu.value != "") {
			if(parseInt(document.form.wan_pppoe_mtu.value) + 8 > parseInt(document.form.wan_mtu.value)){
				document.form.wan_pppoe_mtu.value = parseInt(document.form.wan_mtu.value) - 8;
			}
		}

		if(!validator.string(document.form.wan_pppoe_service)
				|| !validator.string(document.form.wan_pppoe_ac))
			return false;

		//pppoe hostuniq
		if(!validator.hex(document.form.wan_pppoe_hostuniq)) {
			alert("Host-uniq should be hexadecimal digits.");
			document.form.wan_pppoe_hostuniq.focus();
			document.form.wan_pppoe_hostuniq.select();
			return false;
		}
	}

	if(document.form.wan_hwaddr_x.value.length > 0)
		if(!check_macaddr(document.form.wan_hwaddr_x, check_hwaddr_flag(document.form.wan_hwaddr_x))){
				document.form.wan_hwaddr_x.select();
				document.form.wan_hwaddr_x.focus();
				return false;
		}

	if(orig_mtu != "" || document.form.wan_mtu.value.length > 0) {
		if(!validator.numberRange(document.form.wan_mtu, 1280, 1500)) {
			document.form.wan_mtu.focus();
			document.form.wan_mtu.select();
			return false;
		}
	}

	if(wan_bonding_support){
		var msg_dualwan = "<#WANAggregation_disable_dualwan#>";
		var msg_both = "<#WANAggregation_disable_IPTVDualWAN#>";
		if(based_modelid == "RT-AX89U" || based_modelid == "GT-AXY16000" || based_modelid == "XT12" || based_modelid == "ET12"){
			var cur_wanports_bond = $("#wanports_bond_menu").val();
			var msg_iptv = "<#WANAggregation_PortConflict_hint2#>".replace(/LAN-*\D* 4/, wanAggr_p2_name(cur_wanports_bond));
		}
		else{
			var cur_wanports_bond = "";
			var msg_iptv = "<#WANAggregation_PortConflict_hint2#>";
		}

		if((orig_bond_wan != document.form.bond_wan_radio.value || ((based_modelid == "RT-AX89U" || based_modelid == "GT-AXY16000") && orig_wanports_bond != cur_wanports_bond))
		&& document.form.bond_wan_radio.value == "1"){
			if(wans_dualwan.indexOf("none") == -1 && wanAggr_p2_conflicts_w_stb_port(original_switch_stb_x, wanAggr_p2_num(cur_wanports_bond))){
				if(!confirm(msg_both)){
					document.form.bond_wan_radio.value = orig_bond_wan;
					return false;
				}
				else{
					document.form.wans_dualwan.disabled = false;
					document.form.wans_dualwan.value = "wan none";
					document.form.wans_mode.disabled = false;
					document.form.wans_mode.value = "fo";
					document.form.switch_wantag.disabled = false;
					document.form.switch_wantag.value = "none";
					document.form.switch_stb_x.disabled = false;
					document.form.switch_stb_x.value = "0";
				}
			}
			else if(wans_dualwan.indexOf("none") == -1){
				if(!confirm(msg_dualwan)){
					document.form.bond_wan_radio.value = orig_bond_wan;
					return false;
				}
				else{
					document.form.wans_dualwan.disabled = false;
					document.form.wans_dualwan.value = "wan none";
				}
			}
			else if(wanAggr_p2_conflicts_w_stb_port(original_switch_stb_x, wanAggr_p2_num(cur_wanports_bond))){
				if(!confirm(msg_iptv)){
					document.form.bond_wan_radio.value = orig_bond_wan;
					return false;
				}
				else{
					document.form.switch_wantag.disabled = false;
					document.form.switch_wantag.value = "none";
					document.form.switch_stb_x.disabled = false;
					document.form.switch_stb_x.value = "0";
				}
			}

			if((based_modelid == "RT-AX89U" || based_modelid == "GT-AXY16000") &&
				(document.form.wanports_bond.value.indexOf("1") != -1 || document.form.wanports_bond.value.indexOf("2") != -1)){
				// LAN1 or LAN2 is used in WAN aggregation, turn off LAN aggregation
				document.form.lacp_enabled.disabled = false;
				document.form.lacp_enabled.value = 0;
			}
		}
	}

	return true;
}

function done_validating(action){
	refreshpage();
}

function disable_pvc_summary() {
	document.getElementById("MS_WAN_table").style.display = "none";
}

function enable_pvc_summary() {
	document.getElementById("MS_WAN_table").style.display = "";
}

function disable_all_ctrl() {
	document.getElementById("desc_default").style.display = "";
	document.getElementById("desc_edit").style.display = "none";
	document.getElementById("t2BC").style.display = "none";
	document.getElementById("PPPsetting").style.display = "none";
	document.getElementById("DNSsetting").style.display = "none";
	document.getElementById("dot1q_setting").style.display = "none";
	document.getElementById("IPsetting").style.display = "none";
	document.getElementById("wan_DHCP_opt").style.display = "none";
	document.getElementById("vpn_server").style.display = "none";
	document.getElementById("btn_apply").style.display = "none";
}

function enable_all_ctrl(pvc) {
	document.getElementById("desc_default").style.display = "none";
	document.getElementById("desc_edit").style.display = "";
	document.getElementById("t2BC").style.display = "";
	document.getElementById("PPPsetting").style.display = "";
	document.getElementById("DNSsetting").style.display = "";
	document.getElementById("dot1q_setting").style.display = "";
	document.getElementById("IPsetting").style.display = "";
	document.getElementById("wan_DHCP_opt").style.display = "";
	document.getElementById("vpn_server").style.display = "";
	document.getElementById("btn_apply").style.display = "";

	if(dnspriv_support && pvc < 100){
		inputCtrl(document.form.dnspriv_enable, 1);
		change_dnspriv_enable(document.form.dnspriv_enable.value);
	}
	else{
		inputCtrl(document.form.dnspriv_enable, 0);
		change_dnspriv_enable(0);
	}

	if (mtppp_support && pvc < 10) {
		document.getElementById("ppp_conn_tr").style.display = "";
		document.form.wan_ppp_conn.disabled = false;
	}
	else {
		document.getElementById("ppp_conn_tr").style.display = "none";
		document.form.wan_ppp_conn.disabled = true;
	}

}

function change_wan_proto_type(proto_type){
	//change_dhcp_enable();
	change_wan_dhcp_enable();

	if(proto_type == "pppoe" || proto_type == "pppoa"){
		showhide("wan_DHCP_opt",0);
		inputCtrl(document.form.wan_vendorid, 0);
		inputCtrl(document.form.wan_clientid, 0);
		document.form.wan_clientid_type.disabled = true;

		inputCtrl(document.form.wan_auth_x, 0);
		inputCtrl(document.form.wan_pppoe_username, 1);
		inputCtrl(document.form.wan_pppoe_passwd, 1);
		inputCtrl(document.form.wan_pppoe_auth, 1);
		inputCtrl(document.form.wan_pppoe_idletime, 1);
		inputCtrl(document.form.wan_pppoe_idletime_check, 1);
		inputCtrl(document.form.wan_pppoe_mtu, 1);
		inputCtrl(document.form.wan_pppoe_mru, 1);
		inputCtrl(document.form.wan_pppoe_service, 1);
		inputCtrl(document.form.wan_pppoe_ac, 1);

		// 2008.03 James. patch for Oleg's patch. {
		inputCtrl(document.form.wan_pppoe_options_x, 1);
		inputCtrl(document.form.wan_pptp_options_x, 0);
		// 2008.03 James. patch for Oleg's patch. }
		showhide("PPPsetting",1);
		inputCtrl(document.form.wan_ppp_echo, 1);
		ppp_echo_control();
		inputCtrl(document.form.wan_heartbeat_x, 0);
		inputCtrl(document.form.wan_vpndhcp[0], 1);
		inputCtrl(document.form.wan_vpndhcp[1], 1);
		inputCtrl(document.form.wan_dhcp_qry, 0);
		if(mswan_support){
			inputCtrl(document.form.wan_dhcpfilter_enable[0], 0);
			inputCtrl(document.form.wan_dhcpfilter_enable[1], 0);
		}

		$("#s46reset_div").hide();
		$("#dslite_setting").hide();
		document.getElementById("dot1q_setting").style.display = "";
		if(wan_bonding_support){
			inputCtrl(document.form.bond_wan_radio[0], 0);
			inputCtrl(document.form.bond_wan_radio[1], 0);
			document.form.bond_wan_radio.value = "0";
		}
	}
	else if(proto_type == "pptp"){
		showhide("wan_DHCP_opt",0);
		inputCtrl(document.form.wan_vendorid, 0);
		inputCtrl(document.form.wan_clientid, 0);
		document.form.wan_clientid_type.disabled = true;
		
		inputCtrl(document.form.wan_auth_x, 0);
		inputCtrl(document.form.wan_pppoe_username, 1);
		inputCtrl(document.form.wan_pppoe_passwd, 1);
		inputCtrl(document.form.wan_pppoe_auth, 1);
		inputCtrl(document.form.wan_pppoe_idletime, 1);
		inputCtrl(document.form.wan_pppoe_idletime_check, 1);
		inputCtrl(document.form.wan_pppoe_mtu, 0);
		inputCtrl(document.form.wan_pppoe_mru, 0);
		inputCtrl(document.form.wan_pppoe_service, 0);
		inputCtrl(document.form.wan_pppoe_ac, 0);
		inputCtrl(document.form.wan_pppoe_hostuniq, 0);
		inputCtrl(document.form.wan_heartbeat_x, 1);
		inputCtrl(document.form.wan_vpndhcp[0], 0);
		inputCtrl(document.form.wan_vpndhcp[1], 0);
		inputCtrl(document.form.wan_dhcp_qry, 0);
		
		// 2008.03 James. patch for Oleg's patch. {
		inputCtrl(document.form.wan_pppoe_options_x, 1);
		inputCtrl(document.form.wan_pptp_options_x, 1);
		// 2008.03 James. patch for Oleg's patch. }
		inputCtrl(document.form.wan_ppp_echo, 1);
		ppp_echo_control();
		if(mswan_support){
			inputCtrl(document.form.wan_dhcpfilter_enable[0], 0);
			inputCtrl(document.form.wan_dhcpfilter_enable[1], 0);
		}

		$("#s46reset_div").hide();
		$("#dslite_setting").hide();
		document.getElementById("dot1q_setting").style.display = "";
		if(wan_bonding_support){
			inputCtrl(document.form.bond_wan_radio[0], 0);
			inputCtrl(document.form.bond_wan_radio[1], 0);
			document.form.bond_wan_radio.value = "0";
		}
	}
	else if(proto_type == "l2tp"){
		showhide("wan_DHCP_opt",0);
		inputCtrl(document.form.wan_vendorid, 0);
		inputCtrl(document.form.wan_clientid, 0);
		document.form.wan_clientid_type.disabled = true;
		
		inputCtrl(document.form.wan_auth_x, 0);
		inputCtrl(document.form.wan_pppoe_username, 1);
		inputCtrl(document.form.wan_pppoe_passwd, 1);
		inputCtrl(document.form.wan_pppoe_auth, 1);
		inputCtrl(document.form.wan_pppoe_idletime, 0);
		inputCtrl(document.form.wan_pppoe_idletime_check, 0);
		inputCtrl(document.form.wan_pppoe_mtu, 0);
		inputCtrl(document.form.wan_pppoe_mru, 0);
		inputCtrl(document.form.wan_pppoe_service, 0);
		inputCtrl(document.form.wan_pppoe_ac, 0);
		inputCtrl(document.form.wan_pppoe_hostuniq, 0);
		inputCtrl(document.form.wan_heartbeat_x, 1);
		inputCtrl(document.form.wan_vpndhcp[0], 0);
		inputCtrl(document.form.wan_vpndhcp[1], 0);
		inputCtrl(document.form.wan_dhcp_qry, 0);
		
		// 2008.03 James. patch for Oleg's patch. {
		inputCtrl(document.form.wan_pppoe_options_x, 1);
		inputCtrl(document.form.wan_pptp_options_x, 0);
		// 2008.03 James. patch for Oleg's patch. }
		inputCtrl(document.form.wan_ppp_echo, 1);
		ppp_echo_control();
		if(mswan_support){
			inputCtrl(document.form.wan_dhcpfilter_enable[0], 0);
			inputCtrl(document.form.wan_dhcpfilter_enable[1], 0);
		}

		$("#s46reset_div").hide();
		$("#dslite_setting").hide();
		document.getElementById("dot1q_setting").style.display = "";
		if(wan_bonding_support){
			inputCtrl(document.form.bond_wan_radio[0], 0);
			inputCtrl(document.form.bond_wan_radio[1], 0);
			document.form.bond_wan_radio.value = "0";
		}
	}
	else if(proto_type == "static"){
		showhide("wan_DHCP_opt",0);
		inputCtrl(document.form.wan_vendorid, 0);
		inputCtrl(document.form.wan_clientid, 0);
		document.form.wan_clientid_type.disabled = true;

		inputCtrl(document.form.wan_auth_x, 1);
		inputCtrl(document.form.wan_pppoe_username, (document.form.wan_auth_x.value != ""));
		inputCtrl(document.form.wan_pppoe_passwd, (document.form.wan_auth_x.value != ""));
		inputCtrl(document.form.wan_pppoe_auth, 0);
		inputCtrl(document.form.wan_pppoe_idletime, 0);
		inputCtrl(document.form.wan_pppoe_idletime_check, 0);
		inputCtrl(document.form.wan_pppoe_mtu, 0);
		inputCtrl(document.form.wan_pppoe_mru, 0);
		inputCtrl(document.form.wan_pppoe_service, 0);
		inputCtrl(document.form.wan_pppoe_ac, 0);
		// 2008.03 James. patch for Oleg's patch. {
		inputCtrl(document.form.wan_pppoe_options_x, 0);
		inputCtrl(document.form.wan_pptp_options_x, 0);
		// 2008.03 James. patch for Oleg's patch. }
		showhide("PPPsetting",0);
		inputCtrl(document.form.wan_ppp_echo, 0);
		ppp_echo_control(0);
		inputCtrl(document.form.wan_heartbeat_x, 0);
		inputCtrl(document.form.wan_vpndhcp[0], 0);
		inputCtrl(document.form.wan_vpndhcp[1], 0);
		inputCtrl(document.form.wan_dhcp_qry, 0);
		if(mswan_support){
			inputCtrl(document.form.wan_dhcpfilter_enable[0], 0);
			inputCtrl(document.form.wan_dhcpfilter_enable[1], 0);
		}

		$("#s46reset_div").hide();
		$("#dslite_setting").hide();
		document.getElementById("dot1q_setting").style.display = "";
		if(wan_bonding_support){
			if(based_modelid == "RT-AX89U" || based_modelid == "GT-AXY16000"){
				var wan_name = wans_dualwan.split(" ")[<% nvram_get("wan_unit"); %>];
				if(typeof(wan_name) != 'undefined' && wan_name == "none")
					wan_name = wans_dualwan.split(" ")[0];
				if(typeof(wan_name) != 'undefined' && wan_name == "wan"){
					inputCtrl(document.form.bond_wan_radio[0], 1);
					inputCtrl(document.form.bond_wan_radio[1], 1);
					document.form.bond_wan_radio.value = orig_bond_wan;
					document.getElementById("wanports_bond_menu").style.display = "";
				}else{
					inputCtrl(document.form.bond_wan_radio[0], 0);
					inputCtrl(document.form.bond_wan_radio[1], 0);
					document.getElementById("wanports_bond_menu").style.display = "none";
					document.form.bond_wan_radio.value = "0";
				}
			}else{
				inputCtrl(document.form.bond_wan_radio[0], 1);
				inputCtrl(document.form.bond_wan_radio[1], 1);
				document.form.bond_wan_radio.value = orig_bond_wan;
			}
		}
	}
	else if(Softwire46_support && (proto_type == "lw4o6" || proto_type == "map-e" || proto_type == "v6plus" || proto_type == "ocnvc" || proto_type == "dslite")){

		showhide("wan_DHCP_opt",0);
		inputCtrl(document.form.wan_auth_x, 0);
		inputCtrl(document.form.wan_pppoe_username, 0);
		inputCtrl(document.form.wan_pppoe_passwd, 0);
		inputCtrl(document.form.wan_pppoe_auth, 0);
		inputCtrl(document.form.wan_pppoe_idletime, 0);
		inputCtrl(document.form.wan_pppoe_idletime_check, 0);
		inputCtrl(document.form.wan_pppoe_mtu, 0);
		inputCtrl(document.form.wan_pppoe_mru, 0);
		inputCtrl(document.form.wan_pppoe_service, 0);
		inputCtrl(document.form.wan_pppoe_ac, 0);
		inputCtrl(document.form.wan_pppoe_hostuniq, 0);
		inputCtrl(document.form.wan_dhcp_qry, 0);

		// 2008.03 James. patch for Oleg's patch. {
		inputCtrl(document.form.wan_pppoe_options_x, 0);
		inputCtrl(document.form.wan_pptp_options_x, 0);
		// 2008.03 James. patch for Oleg's patch. }
		showhide("PPPsetting",0);
		inputCtrl(document.form.wan_heartbeat_x, 0);
		document.getElementById("vpn_dhcp").style.display = "none";
		inputCtrl(document.form.wan_ppp_echo, 0);
		ppp_echo_control(0);
		if(proto_type == "dslite"){
			$("#dslite_setting").show();
			change_dslite_mode(document.form.wan_s46_dslite_mode.value);
			update_ipv6_s46_b4addr_selector();
		}
		else
			$("#dslite_setting").hide();

		if((proto_type == "v6plus" && wan_proto_orig == "v6plus") || (proto_type == "ocnvc" && wan_proto_orig == "ocnvc"))
			$("#s46reset_div").css("display", "flex");
		else
			$("#s46reset_div").hide();

		document.getElementById("dot1q_setting").style.display = "none";
		if(wan_bonding_support){
			inputCtrl(document.form.bond_wan_radio[0], 0);
			inputCtrl(document.form.bond_wan_radio[1], 0);
			document.form.bond_wan_radio.value = orig_bond_wan;
		}
	}
	else if(proto_type == "dhcp"){
		showhide("wan_DHCP_opt",1);
		inputCtrl(document.form.wan_vendorid, 1);
		inputCtrl(document.form.wan_clientid, 1);
		document.form.wan_clientid_type.disabled = false;
		showDiableDHCPclientID(document.form.tmp_dhcp_clientid_type);

		inputCtrl(document.form.wan_auth_x, 1);
		inputCtrl(document.form.wan_pppoe_username, (document.form.wan_auth_x.value != ""));
		inputCtrl(document.form.wan_pppoe_passwd, (document.form.wan_auth_x.value != ""));
		inputCtrl(document.form.wan_pppoe_auth, 0);
		inputCtrl(document.form.wan_pppoe_idletime, 0);
		inputCtrl(document.form.wan_pppoe_idletime_check, 0);
		inputCtrl(document.form.wan_pppoe_mtu, 0);
		inputCtrl(document.form.wan_pppoe_mru, 0);
		inputCtrl(document.form.wan_pppoe_service, 0);
		inputCtrl(document.form.wan_pppoe_ac, 0);

		// 2008.03 James. patch for Oleg's patch. {
		inputCtrl(document.form.wan_pppoe_options_x, 0);
		inputCtrl(document.form.wan_pptp_options_x, 0);
		// 2008.03 James. patch for Oleg's patch. }
		showhide("PPPsetting",0);
		inputCtrl(document.form.wan_ppp_echo, 0);
		ppp_echo_control(0);
		inputCtrl(document.form.wan_heartbeat_x, 0);
		inputCtrl(document.form.wan_vpndhcp[0], 0);
		inputCtrl(document.form.wan_vpndhcp[1], 0);
		inputCtrl(document.form.wan_dhcp_qry, 1);
		if(mswan_support){
			inputCtrl(document.form.wan_dhcpfilter_enable[0], 0);
			inputCtrl(document.form.wan_dhcpfilter_enable[1], 0);
		}

		$("#s46reset_div").hide();
		$("#dslite_setting").hide();
		document.getElementById("dot1q_setting").style.display = "";
		if(wan_bonding_support){
			if(based_modelid == "RT-AX89U" || based_modelid == "GT-AXY16000"){
				var wan_name = wans_dualwan.split(" ")[<% nvram_get("wan_unit"); %>];
				if(typeof(wan_name) != 'undefined' && wan_name == "none")
					wan_name = wans_dualwan.split(" ")[0];
				if(typeof(wan_name) != 'undefined' && wan_name == "wan"){
					inputCtrl(document.form.bond_wan_radio[0], 1);
					inputCtrl(document.form.bond_wan_radio[1], 1);
					document.form.bond_wan_radio.value = orig_bond_wan;
					document.getElementById("wanports_bond_menu").style.display = "";
				}else{
					inputCtrl(document.form.bond_wan_radio[0], 0);
					inputCtrl(document.form.bond_wan_radio[1], 0);
					document.getElementById("wanports_bond_menu").style.display = "none";
					document.form.bond_wan_radio.value = "0";
				}
			}else{
				inputCtrl(document.form.bond_wan_radio[0], 1);
				inputCtrl(document.form.bond_wan_radio[1], 1);
				document.form.bond_wan_radio.value = orig_bond_wan;
			}
		}
	}
	else if(proto_type == "bridge") {
		showhide("wan_DHCP_opt",0);
		inputCtrl(document.form.wan_vendorid, 0);
		inputCtrl(document.form.wan_clientid, 0);
		document.form.wan_clientid_type.disabled = true;
		inputCtrl(document.form.wan_auth_x, 0);
		inputCtrl(document.form.wan_pppoe_username, 0);
		inputCtrl(document.form.wan_pppoe_passwd, 0);
		inputCtrl(document.form.wan_pppoe_auth, 0);
		inputCtrl(document.form.wan_pppoe_idletime, 0);
		inputCtrl(document.form.wan_pppoe_idletime_check, 0);
		inputCtrl(document.form.wan_pppoe_mtu, 0);
		inputCtrl(document.form.wan_pppoe_mru, 0);
		inputCtrl(document.form.wan_pppoe_service, 0);
		inputCtrl(document.form.wan_pppoe_ac, 0);

		// 2008.03 James. patch for Oleg's patch. {
		inputCtrl(document.form.wan_pppoe_options_x, 0);
		inputCtrl(document.form.wan_pptp_options_x, 0);
		// 2008.03 James. patch for Oleg's patch. }
		showhide("PPPsetting",0);
		inputCtrl(document.form.wan_ppp_echo, 0);
		ppp_echo_control(0);
		inputCtrl(document.form.wan_heartbeat_x, 1);
		inputCtrl(document.form.wan_vpndhcp[0], 1);
		inputCtrl(document.form.wan_vpndhcp[1], 1);
		inputCtrl(document.form.wan_dhcp_qry, 0);
		$("#s46reset_div").hide();
		$("#dslite_setting").hide();
		document.getElementById("dot1q_setting").style.display = "";
		if(mswan_support){
			inputCtrl(document.form.wan_dhcpfilter_enable[0], 1);
			inputCtrl(document.form.wan_dhcpfilter_enable[1], 1);
		}
		else{
			inputCtrl(document.form.wan_dhcpfilter_enable[0], 0);
			inputCtrl(document.form.wan_dhcpfilter_enable[1], 0);
		}
	}
	else {
		alert("error");
	}
}


function fixed_change_wan_proto_type(proto_type){
	if(!document.form.wan_dhcpenable_x[0].checked){
		if(document.form.wan_ipaddr_x.value.length == 0)
			document.form.wan_ipaddr_x.focus();
		else if(document.form.wan_netmask_x.value.length == 0)
			document.form.wan_netmask_x.focus();
		else if(document.form.wan_gateway_x.value.length == 0)
			document.form.wan_gateway_x.focus();
	}

	if(proto_type == "pppoe" || proto_type == "pppoa"){

		inputCtrl(document.form.wan_hwaddr_x, 1);
		inputCtrl(document.form.wan_enable[0], 1);
		inputCtrl(document.form.wan_enable[1], 1);
		inputCtrl(document.form.wan_nat_x[0], 1);
		inputCtrl(document.form.wan_nat_x[1], 1);
		inputCtrl(document.form.wan_upnp_enable[0], 1);
		inputCtrl(document.form.wan_upnp_enable[1], 1);

		showhide("IPsetting",1);
		showhide("DNSsetting",1);
		showhide("vpn_server",1);
		document.form.wan_ppp_echo.value = original_ppp_echo;
		ppp_echo_control();
	}
	else if(proto_type == "pptp" || proto_type == "l2tp"){

		showhide("IPsetting",1);
		showhide("PPPsetting",1);
		showhide("vpn_server",1);
		document.form.wan_ppp_echo.value = default_ppp_echo;
		ppp_echo_control();

	}
	else if(proto_type == "static"){
		
		inputCtrl(document.form.wan_hwaddr_x, 1);
		inputCtrl(document.form.wan_enable[0], 1);
		inputCtrl(document.form.wan_enable[1], 1);
		inputCtrl(document.form.wan_nat_x[0], 1);
		inputCtrl(document.form.wan_nat_x[1], 1);
		inputCtrl(document.form.wan_upnp_enable[0], 1);
		inputCtrl(document.form.wan_upnp_enable[1], 1);
		showhide("IPsetting",1);
		showhide("DNSsetting",1);
		showhide("vpn_server",1);
	}	
	else if(proto_type == "dhcp"){
		
		document.getElementById("IPsetting").style.display = "none";
		inputCtrl(document.form.wan_hwaddr_x, 1);
		inputCtrl(document.form.wan_enable[0], 1);
		inputCtrl(document.form.wan_enable[1], 1);
		inputCtrl(document.form.wan_nat_x[0], 1);
		inputCtrl(document.form.wan_nat_x[1], 1);
		inputCtrl(document.form.wan_upnp_enable[0], 1);
		inputCtrl(document.form.wan_upnp_enable[1], 1);
		showhide("DNSsetting",1);
		showhide("vpn_server",1);
	}
	else if(proto_type == "bridge"){

		inputCtrl(document.form.wan_hwaddr_x, 0);
		inputCtrl(document.form.wan_enable[0], 1);
		inputCtrl(document.form.wan_enable[1], 1);
		inputCtrl(document.form.wan_upnp_enable[0], 1);
		inputCtrl(document.form.wan_upnp_enable[1], 1);
		inputCtrl(document.form.wan_nat_x[0], 0);
		inputCtrl(document.form.wan_nat_x[1], 0);
		showhide("DNSsetting",0);
		showhide("vpn_server",0);
	}
}

function change_wan_dhcp_enable(flag){
	var wan_type = document.form.wan_proto.value;
	
	// 2008.03 James. patch for Oleg's patch. {
	if(wan_type == "pppoe"){
		if(flag == 1){
			if(wan_type == original_wan_type){
				document.form.wan_dhcpenable_x[0].checked = original_wan_dhcpenable;
				document.form.wan_dhcpenable_x[1].checked = !original_wan_dhcpenable;
			}
			else{
				document.form.wan_dhcpenable_x[0].checked = 1;
				document.form.wan_dhcpenable_x[1].checked = 0;
			}
		}
		
		document.getElementById('IPsetting').style.display = "";
		document.getElementById('S46setting').style.display = "none";
		document.getElementById('PPPsetting').style.display = "";
		inputCtrl(document.form.wan_dhcpenable_x[0], 1);
		inputCtrl(document.form.wan_dhcpenable_x[1], 1);
		
		var wan_dhcpenable = document.form.wan_dhcpenable_x[0].checked;
		
		inputCtrl(document.form.wan_ipaddr_x, !wan_dhcpenable);
		inputCtrl(document.form.wan_netmask_x, !wan_dhcpenable);
		inputCtrl(document.form.wan_gateway_x, !wan_dhcpenable);
	}
	// 2008.03 James. patch for Oleg's patch. }
	else if(wan_type == "pptp"|| wan_type == "l2tp"){
		if(flag == 1){
			if(wan_type == original_wan_type){
				document.form.wan_dhcpenable_x[0].checked = original_wan_dhcpenable;
				document.form.wan_dhcpenable_x[1].checked = !original_wan_dhcpenable;
			}
			else{
				document.form.wan_dhcpenable_x[0].checked = 0;
				document.form.wan_dhcpenable_x[1].checked = 1;
			}
		}
		
		document.getElementById('IPsetting').style.display = "";
		document.getElementById('S46setting').style.display = "none";
		document.getElementById('PPPsetting').style.display = "";
		inputCtrl(document.form.wan_dhcpenable_x[0], 1);
		inputCtrl(document.form.wan_dhcpenable_x[1], 1);
		
		var wan_dhcpenable = document.form.wan_dhcpenable_x[0].checked;
		
		inputCtrl(document.form.wan_ipaddr_x, !wan_dhcpenable);
		inputCtrl(document.form.wan_netmask_x, !wan_dhcpenable);
		inputCtrl(document.form.wan_gateway_x, !wan_dhcpenable);
	}
	else if(wan_type == "static"){
		document.form.wan_dhcpenable_x[0].checked = 0;
		document.form.wan_dhcpenable_x[1].checked = 1;
		
		inputCtrl(document.form.wan_dhcpenable_x[0], 0);
		inputCtrl(document.form.wan_dhcpenable_x[1], 0);
		
		document.getElementById('IPsetting').style.display = "";
		inputCtrl(document.form.wan_ipaddr_x, 1);
		inputCtrl(document.form.wan_netmask_x, 1);
		inputCtrl(document.form.wan_gateway_x, 1);
	}
	else{	// wan_type == "dhcp"
		document.form.wan_dhcpenable_x[0].checked = 1;
		document.form.wan_dhcpenable_x[1].checked = 0;
		
		inputCtrl(document.form.wan_dhcpenable_x[0], 0);
		inputCtrl(document.form.wan_dhcpenable_x[1], 0);
		
		inputCtrl(document.form.wan_ipaddr_x, 0);
		inputCtrl(document.form.wan_netmask_x, 0);
		inputCtrl(document.form.wan_gateway_x, 0);
		document.getElementById('IPsetting').style.display = "none";
	}
	
}

function showMAC(){
	var tempMAC = "";
	document.form.wan_hwaddr_x.value = login_mac_str();
}

function check_macaddr(obj,flag){ //control hint of input mac address
	if(flag == 1){
		var childsel=document.createElement("div");
		childsel.setAttribute("id","check_mac");
		childsel.style.color="#FFCC00";
		obj.parentNode.appendChild(childsel);
		document.getElementById("check_mac").innerHTML="<#LANHostConfig_ManualDHCPMacaddr_itemdesc#>";
		return false;
	}else if(flag ==2){
		var childsel=document.createElement("div");
		childsel.setAttribute("id","check_mac");
		childsel.style.color="#FFCC00";
		obj.parentNode.appendChild(childsel);
		document.getElementById("check_mac").innerHTML="<#IPConnection_x_illegal_mac#>";
		return false;
	}else{
		document.getElementById("check_mac") ? document.getElementById("check_mac").style.display="none" : true;
		return true;
	}
}

/* password item show or not */
function pass_checked(obj){
	switchType(obj, document.form.show_pass_1.checked, true);
}

function ppp_echo_control(flag){
	if (typeof(flag) == 'undefined')
		flag = document.form.wan_ppp_echo.value;
	var enable = (flag == 1) ? 1 : 0;
	inputCtrl(document.form.wan_ppp_echo_interval, enable);
	inputCtrl(document.form.wan_ppp_echo_failure, enable);
	enable = (flag == 2) ? 1 : 0;
	//inputCtrl(document.form.dns_probe_timeout, enable);
	inputCtrl(document.form.dns_delay_round, enable);
}

function change_dnspriv_enable(flag){
	if(flag == 1){
		inputCtrl(document.form.dnspriv_profile[0], 1);
		inputCtrl(document.form.dnspriv_profile[1], 1);
		document.getElementById("DNSPrivacy").style.display = "";
		document.getElementById("dnspriv_rulelist_Block").style.display = "";
		if($("#dotPresets option").length > 1)
			document.getElementById("dot_presets_tr").style.display = "";
		show_dnspriv_rulelist();
	}
	else{
		inputCtrl(document.form.dnspriv_profile[0], 0);
		inputCtrl(document.form.dnspriv_profile[1], 0);
		document.getElementById("DNSPrivacy").style.display = "none";
		document.getElementById("dnspriv_rulelist_Block").style.display = "none";
		document.getElementById("dot_presets_tr").style.display = "none";
	}
}

function addRow(obj, head){
	if(head == 1)
		dnspriv_rulelist_array += "&#60"
	else
		dnspriv_rulelist_array += "&#62"

	dnspriv_rulelist_array += obj.value;
	obj.value = "";
}

function addRow_Group(upper){
	var rule_num = document.getElementById('dnspriv_rulelist_table').rows.length;
	var item_num = document.getElementById('dnspriv_rulelist_table').rows[0].cells.length;		
	if(rule_num >= upper){
		alert("<#JS_itemlimit1#> " + upper + " <#JS_itemlimit2#>");
		return false;	
	}	

	if(document.form.dnspriv_server_0.value==""){
		alert("<#JS_fieldblank#>");
		document.form.dnspriv_server_0.focus();
		document.form.dnspriv_server_0.select();		
		return false;
	}
	else{
		if(document.form.dnspriv_hostname_0.value.indexOf("tls://") != -1){
			document.form.dnspriv_hostname_0.value = document.form.dnspriv_hostname_0.value.replace("tls://", "");
		}
		addRow(document.form.dnspriv_server_0, 1);
		addRow(document.form.dnspriv_port_0, 0);
		addRow(document.form.dnspriv_hostname_0, 0);
		addRow(document.form.dnspriv_spkipin_0, 0);
		show_dnspriv_rulelist();
	}
}

function edit_Row(r){ 	
	var i=r.parentNode.parentNode.rowIndex;
  	document.form.dnspriv_server_0.value = document.getElementById('dnspriv_rulelist_table').rows[i].cells[0].innerHTML;
	document.form.dnspriv_port_0.value = document.getElementById('dnspriv_rulelist_table').rows[i].cells[1].innerHTML; 
	document.form.dnspriv_hostname_0.value = document.getElementById('dnspriv_rulelist_table').rows[i].cells[2].innerHTML; 
	document.form.dnspriv_spkipin_0.value = document.getElementById('dnspriv_rulelist_table').rows[i].cells[3].innerHTML;

	del_Row(r);	
}

function del_Row(r){
	var i=r.parentNode.parentNode.rowIndex;
	document.getElementById('dnspriv_rulelist_table').deleteRow(i);

	var dnspriv_rulelist_value = "";
	for(k=0; k<document.getElementById('dnspriv_rulelist_table').rows.length; k++){
		for(j=0; j<document.getElementById('dnspriv_rulelist_table').rows[k].cells.length-1; j++){
			if(j == 0)
				dnspriv_rulelist_value += "&#60";
			else
				dnspriv_rulelist_value += "&#62";
			dnspriv_rulelist_value += document.getElementById('dnspriv_rulelist_table').rows[k].cells[j].innerHTML;
		}
	}

	dnspriv_rulelist_array = dnspriv_rulelist_value;
	if(dnspriv_rulelist_array == "")
		show_dnspriv_rulelist();
}

function show_dnspriv_rulelist(){
	var dnspriv_rulelist_row = dnspriv_rulelist_array.split('&#60');
	var code = "";

	code +='<table width="100%" border="1" cellspacing="0" cellpadding="4" align="center" class="list_table" id="dnspriv_rulelist_table">';
	if(dnspriv_rulelist_row.length == 1)
		code +='<tr><td style="color:#FFCC00;" colspan="5"><#IPConnection_VSList_Norule#></td></tr>';
	else{
		for(var i = 1; i < dnspriv_rulelist_row.length; i++){
			code +='<tr id="row'+i+'">';
			var dnspriv_rulelist_col = dnspriv_rulelist_row[i].split('&#62');
			var wid=[27, 10, 27, 27];
				for(var j = 0; j < dnspriv_rulelist_col.length; j++){
					code +='<td width="'+wid[j]+'%">'+ dnspriv_rulelist_col[j] +'</td>';
				}
				code +='<td width="9%"><!--input class="edit_btn" onclick="edit_Row(this);" value=""/-->';
				code +='<input class="remove_btn" onclick="del_Row(this);" value=""/></td></tr>';
		}
	}
	code +='</table>';
	document.getElementById("dnspriv_rulelist_Block").innerHTML = code;
}

var cur_bond_port = /LAN-*\D* 4/;
function change_wanAggre_desc(){
	var orig_desc = $("#wanAgg_desc").html();
	var selectedName = "";

	if(based_modelid == "RT-AXE7800")
		orig_desc = orig_desc.replace("#WAN", "1G WAN");
	else
		orig_desc = orig_desc.replace("#WAN", "WAN");

	if(based_modelid == "RT-AX89U" || based_modelid == "GT-AXY16000"){
		var selectedIndex = document.getElementById("wanports_bond_menu").selectedIndex;
		selectedName = document.getElementById("wanports_bond_menu").options[selectedIndex].text;
	}
	else if(based_modelid == "XT12" || based_modelid == "ET12"){
		orig_desc = orig_desc.replace("2Gbps", "5Gbps");
		selectedName = "2.5G/1G LAN";
	}
	else if(based_modelid == "RT-AXE7800"){
		selectedName = "LAN 2";
	}

	if(selectedName != "")
		orig_desc = orig_desc.replace(cur_bond_port, selectedName);

	$("#wanAgg_desc").html(orig_desc);

	cur_bond_port = selectedName;
}

function showDiableDHCPclientID(clientid_enable){
	if(clientid_enable.checked) {
		document.form.wan_clientid_type.value = "1";
		document.form.wan_clientid.value = "";
		document.form.wan_clientid.style.display = "none";
	}
	else {
		document.form.wan_clientid_type.value = "0";
		document.form.wan_clientid.style.display = "";
	}
}

function s46reset(){
	$.ajax({
		url: "/s46reset.cgi",
		success: function( response ) {
		}
	});
}
function update_map(){
	var msg = stringSafeGet("<#MAP_update_desc#>")+" <#AiMesh_confirm_msg0#>";
	if(confirm(msg)){
		$.ajax({
			url: "/s46reset.cgi",

			success: function( response ) {
				httpApi.nvramSet({
				    "action_mode": "apply",
				    "rc_service" : "restart_wan"
				});
				showLoading(10);
			}
		});
	}
	else
		return false;
}

function change_dslite_mode(flag){
	if(flag==0){	//Auto
		inputCtrl(document.form.ipv6_s46_aftr, 0);
		inputCtrl(document.form.ipv6_s46_b4addr_Select, 0);
		document.getElementById("ipv6_s46_aftr_r").style.display = "";
		document.getElementById("ipv6_s46_b4addr_r").style.display = "";
	}
	else{
		inputCtrl(document.form.ipv6_s46_aftr, 1);
		inputCtrl(document.form.ipv6_s46_b4addr_Select, 1);
		document.getElementById("ipv6_s46_aftr_r").style.display = "none";
		document.getElementById("ipv6_s46_b4addr_r").style.display = "none";
	}
}

function update_ipv6_s46_b4addr_selector(){
	$("#ipv6_s46_b4addr_Select").empty();
	var selectedValue = httpApi.nvramGet(["ipv6_s46_b4addr"]).ipv6_s46_b4addr;

	for (var i = 2; i <= 7; i++) {
		var option = document.createElement("option");
		option.value = "192.0.0." + i;
		option.text = "192.0.0." + i;
		if (option.value === selectedValue) {
			option.selected = true;
		}
		$("#ipv6_s46_b4addr_Select").append(option);
	}
}

var over_var = 0;
var isMenuopen = 0;
function hide_aftr_Block(){
	document.getElementById("pull_arrow").src = "/images/unfold_more.svg";
	document.getElementById('aftr_Block_PC').style.display='none';
	isMenuopen = 0;
}

function pullaftrList(obj){
	if(isMenuopen == 0){		
		obj.src = "/images/unfold_less.svg"
		document.getElementById("aftr_Block_PC").style.display = 'block';
		document.form.ipv6_s46_aftr.focus();		
		isMenuopen = 1;
	}
	else
		hide_aftr_Block();
}

var aftrArray = [
		["<#IPv6_xpass#> (dgw.xpass.jp)", "dgw.xpass.jp"], ["transix (gw.transix.jp)", "gw.transix.jp"]
	];

function showaftrList(){
	if(theme == "WHITE"){
		$("#aftr_Block_PC").removeClass("aftr_Block_PC").addClass("aftr_Block_PC_WHITE");
	}
	var code = "";
	for(var i = 0; i < aftrArray.length; i++){
		code += '<a><div onmouseover="over_var=1;" onmouseout="over_var=0;" onclick="setaftrIP(\''+aftrArray[i][1]+'\');"><strong>'+aftrArray[i][0]+'</strong></div></a>';
	}
	code +='<!--[if lte IE 6.5]><iframe class="hackiframe2"></iframe><![endif]-->';	
	document.getElementById("aftr_Block_PC").innerHTML = code;
}

function setaftrIP(ipaddr){
	document.form.ipv6_s46_aftr.value = ipaddr;
	hide_aftr_Block();
	over_var = 0;
}

/*------------ get IPv6 info Start -----------------*/
function update_info(flag){
	if(flag == 0)
		return false;
			
	$.ajax({
		url: '/update_IPv6state.asp',
		dataType: 'script',
		timeout: 1500,
		error: function(xhr){
			setTimeout("update_info();", 1500);
		},
		success: function(response){
			showInfo();
		}
	});
}

function showInfo(){
	if(document.form.wan_proto.value == wan_proto_orig){
		if(document.getElementById("ipv6_s46_aftr_r").style.display == ""){
			document.getElementById("ipv6_s46_aftr_span").innerHTML = state_ipv6_s46_aftr;
		}
		if(document.getElementById("ipv6_s46_b4addr_r").style.display == ""){
			document.getElementById("ipv6_s46_b4addr_span").innerHTML = state_ipv6_s46_b4addr;
		}
		if(s46_dslite_svc_str != "none" && s46_dslite_svc_str != ""){	//none,xpass,transix
			document.getElementById("wan_s46_dslite_svc_r").style.display = "";
			document.getElementById("wan_s46_dslite_svc_span").innerHTML = (s46_dslite_svc_str=="xpass")? "<#IPv6_xpass#>":s46_dslite_svc_str;
		}
		
		setTimeout("update_info();", 1500);
	}
}
/*------------- get IPv6 info end ----------------------------*/

function Assign_DNS_service(){

	$("#assign_button").attr('disabled', true);
	$("#dns_list_Block").remove();

	var divObj = document.createElement("div");
	divObj.setAttribute("id","dns_list_Block");
	if(parent.webWrapper){
		divObj.className = "dnslist_viewlist_business";
		document.body.appendChild(divObj);
		// cal_panel_block("dns_list_Block", 0.045);
		
	}
	else{
		divObj.className = "dnslist_viewlist";
		document.body.appendChild(divObj);
		// cal_panel_block("dns_list_Block", 0.045);
	}
	create_DNSlist_view();
}

var DNSList_view_hide_flag = false;
function hide_DNSList_view_block() {
	if(DNSList_view_hide_flag){closeDNSListView();}
	DNSList_view_hide_flag=true;
}
function show_DNSList_view_block() {
	DNSList_view_hide_flag = false;
}


function create_DNSlist_view(){
	//count FilterMode 
	var array_Ab=[], array_Fm=[], array_FD=[], array_Sf=[], array_Pr=[];
	for(var i=0;i<DNSService.length;i++){
		//"Ad block", "Family", "Fast DNS", "Safe", "Privacy-respecting"
		switch (DNSService[i].FilterMode){
			case "Ad block":
						array_Ab.push(i);
				break;
			case "Family":
						array_Fm.push(i);
				break;
			case "Fast DNS":
						array_FD.push(i);
				break;
			case "Safe":
						array_Sf.push(i);
				break;
			case "Privacy-respecting":
						array_Pr.push(i);
				break;
			default:
				break;

		}
	}

	//register event to detect mouse click
	document.body.onclick = function() {hide_DNSList_view_block();}
	DNSList_view_hide_flag = false;

	document.getElementById("dns_list_Block").onclick = function() {show_DNSList_view_block();}

	var DNSListTableIndex=[array_Ab, array_Fm, array_FD, array_Sf, array_Pr];
	var DNSListTableCategory=["<#IPConnection_x_DNS_List_adB#>", "<#IPConnection_x_DNS_List_Family#>", "<#IPConnection_x_DNS_List_Fast#>", "Safe", "<#IPConnection_x_DNS_List_Priv-resp#>"];

	var code="";

	code += "<div style='margin-top:15px;margin-left:15px;float:left;font-size:15px;color:#93A9B1;'><#IPConnection_x_DNS_List#></div>";
	code += "<div style='float:right;'><img src='/images/button-close.gif' style='width:30px;cursor:pointer' onclick='closeDNSListView();'></div>";
	code += "<table border='0' align='center' cellpadding='0' cellspacing='0' style='width:100%;padding:0 15px 15px 15px;'><tbody><tr><td>";

	code += "<table width='100%' border='1' align='center' cellpadding='0' cellspacing='0' class='FormTable_table' style='margin-top:15px;'>";
	code += "<thead><tr height='28px'><td id='td_all_list_title' colspan='4'><#CTL_Default#>";
	code += "</td></tr></thead>";
	code += "<tr id='tr_default_title' height='40px'>";
	code += "<th width='5%'></th>";
	code += "<th width='15%'><#qis_service#></th>";
	code += "<th width='80%'><#HSDPAConfig_DNSServers_itemname#></th></tr>";

	code += "<tr id='tr_auto_option' height='40px'>";
	code += "<td><input name='DNS_service_opt' id='dns_auto' type='radio' class='input'></td>";
	code += "<td>ISP</td>";
	code += "<td style='padding:10px;text-align:left;'><#IPConnection_x_DNSServer_auto#></td></tr>";
	code += "</table>";

	code += "</td></tr>";

	for (var j = 0; j < DNSListTableIndex.length; j++)
	{
		code += "<tr><td>";

		code += "<table width='100%' border='1' align='center' cellpadding='0' cellspacing='0' class='FormTable_table' style='margin-top:15px;'>";
		code += "<thead><tr height='28px'><td colspan='4'>";
		code += DNSListTableCategory[j];
		code += "</td></tr></thead>";
		code += "<tr id='tr_"+j+"_title' height='40px'>";
		code += "<th width='5%'></th>";
		code += "<th width='15%'><#qis_service#></th>";
		code += "<th width='15%'><#HSDPAConfig_DNSServers_itemname#></th>";
		code += "<th width='65%'><#Description#></th></tr>";

		DNSListTableIndex[j].forEach(function(idx) {
			if(DNSService[idx].confirmed == "Yes"){
				code += "<tr id='tr_"+idx+"_option' height='40px'>";
				code += "<td><input name='DNS_service_opt' id='dns_"+idx+"' type='radio' class='input'></td>";
				code += "<td>"+DNSService[idx].DNSService+"</td>";
				code += "<td style='padding:10px;text-align:left;'>"+DNSService[idx].ServiceIP1+"<br>"+DNSService[idx].ServiceIP2+"</td>";
				if(DNSService[idx].url){
					var description_temp = "<a href='"+DNSService[idx].url+"' target='_blank' style='text-decoration:underline;'> <b><#Learn_more#></b></a>";
					if(DNSService[idx].Description){
						description_temp = DNSService[idx].Description +" "+ description_temp;
					}
					
					code += "<td style='padding:10px;text-align:left;'>"+description_temp+"</td></tr>";
				}
				else{
					if(DNSService[idx].Description)
						code += "<td style='padding:10px;text-align:left;'>"+DNSService[idx].Description+"</td></tr>";
					else
						code += "<td style='padding:10px;text-align:left;'></td></tr>";
				}
			}
		});
		code += "</table>";

		code += "</td></tr>";
	}
	
	code += "<tr><td>";

	code += "<table width='100%' border='1' align='center' cellpadding='0' cellspacing='0' class='FormTable_table' style='margin-top:15px;'>";
	code += "<thead><tr height='28px'><td colspan='4'><#Manual_Setting_btn#>";
	code += "</td></tr></thead>";
	code += "<tr id='tr_manual_title' height='40px'>";
	code += "<th width='5%'></th>";
	code += "<th width='15%'><#qis_service#></th>";
	code += "<th width='80%'><#HSDPAConfig_DNSServers_itemname#></th></tr>";

	code += "<tr id='tr_manual_1' height='40px'>";
	code += "<td rowspan='2'><input name='DNS_service_opt' id='dns_manual' type='radio' class='input'></td>";
	code += "<td><#IPConnection_x_DNSServer1_itemname#></td>";
	code += "<td style='padding:10px;text-align:left;'><input type='text' maxlength='15' class='input_15_table' id='edit_wan_dns1_x' value='' onkeypress='return validator.isIPAddr(this, event)' autocorrect='off' autocapitalize='off'></td></tr>";
	code += "<tr id='tr_manual_2' height='40px'>";
	code += "<td><#IPConnection_x_DNSServer2_itemname#></td>";
	code += "<td style='padding:10px;text-align:left;'><input type='text' maxlength='15' class='input_15_table' id='edit_wan_dns2_x' value='' onkeypress='return validator.isIPAddr(this, event)' autocorrect='off' autocapitalize='off'></td></tr>";
	code += "</table>";

	code += "</td></tr></tbody></table>";

	code += "<div style='margin-top:10px;margin-bottom:20px;width:100%;text-align:center;'>";
	code += "<input id='button_dns_cancel' class='button_gen' type='button' onclick='closeDNSListView()' value='<#CTL_Cancel#>'>";
	code += "<input id='button_dns_ok' class='button_gen' type='button' onclick='Update_DNS_service()' style='margin-left:15px;' value='<#CTL_onlysave#>'>";
	code += "</div>";

	$("#dns_list_Block").html(code);

	if(parent.webWrapper){
		$("#dns_list_Block").find(".button_gen").css('display', 'inline');
	}

	if(document.form.wan_dnsenable_x.value == 1)	//Auto
	{
		if(document.form.wan_proto.value == "static"){
			$("#dns_auto").attr('checked', false)
						  .attr('disabled', true);
		}
		else{
			$("#dns_auto").attr('checked', true);
		}
	}
	else{
		if(document.form.wan_proto.value == "static"){
			$("#dns_auto").attr('checked', false)
						  .attr('disabled', true);
		}
		
		if(DNS_list_index >= 0){
			$("#dns_"+DNS_list_index).attr('checked', true);
		}
		else{
			$("#dns_manual").attr('checked', true);
			$("#edit_wan_dns1_x").val("<% nvram_get("wan_dns1_x"); %>");
			$("#edit_wan_dns2_x").val("<% nvram_get("wan_dns2_x"); %>");
		}
	}
	$("#dns_list_Block").fadeIn();
}

function Update_DNS_service(){
	if($('input:radio[name="DNS_service_opt"]:checked').length == 0){
		alert("Please set up the DNS server on the client device.");	/* Untranslated */
		return;
	}

	if($("#dns_auto").is(":checked")){
		document.form.wan_dnsenable_x.value = 1;
	}
	else {
		document.form.wan_dnsenable_x.value = 0;
		if($("#dns_manual").is(":checked")){

			if($("#edit_wan_dns1_x").val().length==0 && $("#edit_wan_dns2_x").val().length==0){
				alert("<#JS_fieldblank#>");
				$("#edit_wan_dns1_x").focus();
				return false;
			}
			if(!valid_IP($("#edit_wan_dns1_x"), "DNS")) return false;  //DNS1
			if(!valid_IP($("#edit_wan_dns2_x"), "DNS")) return false;  //DNS2

			DNS_list_index=DNSList_match($("#edit_wan_dns1_x").val(), $("#edit_wan_dns2_x").val());
			if(DNS_list_index >= 0){
				$("#dns_"+DNS_list_index).attr('checked', true);
			}
			document.form.wan_dns1_x.value = $("#edit_wan_dns1_x").val();
			document.form.wan_dns2_x.value = $("#edit_wan_dns2_x").val();
		}
		else{
			for(var m=0; m < DNSService.length; m++){
				if($("#dns_"+m).is(":checked")){
					document.form.wan_dns1_x.value = DNSService[m].ServiceIP1;
					document.form.wan_dns2_x.value = DNSService[m].ServiceIP2;
				}
			}
		}
	}
	
	httpApi.nvramSet({
		wan_unit: load_wan_unit,
		wan_dnsenable_x: document.form.wan_dnsenable_x.value,
		wan_dns1_x: document.form.wan_dns1_x.value,
		wan_dns2_x: document.form.wan_dns2_x.value,
		action_mode: "apply",
		rc_service: "restart_wan_dns "+load_wan_unit
	}, closeDNSListView());

	Update_DNS_status();
}

function cal_panel_block(obj){
	var blockmarginLeft;
	var blockmarginTop;

	if (window.innerWidth)
		winWidth = window.innerWidth;
	else if ((document.body) && (document.body.clientWidth))
		winWidth = document.body.clientWidth;

	if (window.innerHeight)
		winHeight = window.innerHeight;
	else if ((document.body) && (document.body.clientHeight))
		winHeight = document.body.clientHeight;

	if (document.documentElement && document.documentElement.clientHeight && document.documentElement.clientWidth){
		winWidth = document.documentElement.clientWidth;
		winHeight = document.documentElement.clientHeight;
	}

	if(winWidth >1050){
		winPadding = (winWidth-1050)/2;
		winWidth = 1105;
		blockmarginLeft= (winWidth*0.2)+winPadding;
	}
	else if(winWidth <=1050){
		blockmarginLeft= (winWidth)*0.2 + document.body.scrollLeft;
	}

	if(winHeight >660)
		winHeight = 660;

	blockmarginTop= winHeight*(-1.68);

	if(parent.webWrapper){
		blockmarginLeft = 100;
		blockmarginTop = 500;
	}

	document.getElementById(obj).style.marginLeft = blockmarginLeft+"px";
	document.getElementById(obj).style.marginTop = blockmarginTop+"px";
}

function closeDNSListView() {
	$("#assign_button").attr('disabled', false);
	$("#dns_list_Block").fadeOut();
}

var DNSService = new Object;
function updatDNSListOnline(){

	$.getJSON("/ajax/DNS_List.json", function(local_data){
		DNSService = Object.keys(local_data).map(function(e){
				return local_data[e];
		});
		Update_DNS_status();

		$.getJSON("https://nw-dlcdnet.asus.com/plugin/js/DNS_List.json",
			function(cloud_data){
				if(JSON.stringify(local_data) != JSON.stringify(cloud_data)){
					if(Object.keys(cloud_data).length > 0){
						DNSService = Object.keys(cloud_data).map(function(e){
							return cloud_data[e];
						});
						Update_DNS_status();
					}
				}
			}
		);
	});
}

var DNS_list_index="";
var DSN_status_info="";
function Update_DNS_status(){
	$("#DNS_status").empty();

	if(document.form.wan_dnsenable_x.value == 1){
		DSN_status_info="<b>Default status :</b> <#IPConnection_x_DNSServer_auto#>";
	}
	else{
		DNS_list_index=DNSList_match(document.form.wan_dns1_x.value, document.form.wan_dns2_x.value);
		if(DNS_list_index >= 0){
			DSN_status_info = "<b><#Filter_Mode#>:</b> "+DNSService[DNS_list_index].FilterMode;	/* Untranslated */
			DSN_status_info += "<br><b><#BM_UserList1#>:</b> "+DNSService[DNS_list_index].DNSService;	/* Untranslated */
			DSN_status_info += "<br><b><#PPPConnection_x_WANDNSServer_itemname#>:</b> "+document.form.wan_dns1_x.value+", "+document.form.wan_dns2_x.value;	/* Untranslated */
		}
		else{
			DSN_status_info = "<b><#PPPConnection_x_WANDNSServer_itemname#>:</b> "+document.form.wan_dns1_x.value;	/* Untranslated */
			if(document.form.wan_dns2_x.value!="")
				DSN_status_info += ", "+document.form.wan_dns2_x.value;
		}
	}
	$("#DNS_status").html(DSN_status_info);
}

function DNSList_match(ip1, ip2){
	var join_dns_x = ip1+"#"+ip2;
	for(x=0; x < DNSService.length; x++){
		if(join_dns_x == DNSService[x].ServiceIP1+"#"+DNSService[x].ServiceIP2
			|| join_dns_x == DNSService[x].ServiceIP2+"#"+DNSService[x].ServiceIP1
		){
			return x;
		}
	}
	return -1;
}

function close_autowan_hint(){
	$("#autowan_hint_div").hide();
}

function confirm_autowan_change(){
	$("#autowan_hint_div").hide();

	$('<input>').attr({
		type: 'hidden',
		name: "autowan_enable",
		value: "0"
	}).appendTo('form');

	$('<input>').attr({
			type: 'hidden',
			name: "wans_extwan",
			value: "0"
	}).appendTo('form');

	setTimeout(function(){
			if($(".popup_edit_profile_container").is(":visible")){
				applyData["autowan_enable"] = "0";
				applyData["wans_extwan"] = "0";
				showLoading();
				httpApi.nvramSet(applyData, function(){
						setTimeout(function(){
							close_popup_container("all");
							refreshpage();
						}, (applyData["rc_service"] == "reboot")? (default_reboot_time*1000):20000);
					});
			}
			else{
				if(reboot_confirm == 1){
					if(confirm("<#AiMesh_Node_Reboot#>")){
						if((wan_proto_orig != "v6plus" && document.form.wan_proto.value == "v6plus") ||
							(wan_proto_orig != "ocnvc" && document.form.wan_proto.value == "ocnvc")){
							s46reset();	//map-e changed
						}

						FormActions("start_apply.htm", "apply", "reboot", "<% get_default_reboot_time(); %>");
						showLoading();
						document.form.submit();
					}
				}
				else{
					if((wan_proto_orig != "v6plus" && document.form.wan_proto.value == "v6plus") ||
						(wan_proto_orig != "ocnvc" && document.form.wan_proto.value == "ocnvc")){
						s46reset();	//map-e changed
					}

					showLoading();
					document.form.submit();
				}
			}
		}, 100);
}

function isEmpty(obj)
{
	for (var name in obj){
		return false;
	}

	return true;
};

function get_default_wan_name(){
	var default_wan_name = "WAN";
	var eth_wan_list = httpApi.hookGet("get_ethernet_wan_list", true);

	if(!isEmpty(eth_wan_list)){
		default_wan_name = eth_wan_list["wan"].wan_name;
	}

	return default_wan_name;
}

function change_wizard(o, id){
	if (id == "dotPresets") {
		var i = o.value;
		if (i == -1) return;
		document.form.dnspriv_server_0.value = dot_servers_array[i].dnspriv_server;
		document.form.dnspriv_port_0.value = dot_servers_array[i].dnspriv_port;
		document.form.dnspriv_hostname_0.value = dot_servers_array[i].dnspriv_hostname;
		document.form.dnspriv_spkipin_0.value = dot_servers_array[i].dnspriv_spkipin;

		document.getElementById("dotPresets").selectedIndex = 0;
	}
}

</script>
</head>

<body onload="initial();" onunLoad="return unload_body();">
<div id="TopBanner"></div>
<div id="Loading" class="popup_bg"></div>
<div class="hidden_mask popup_element"></div>
<div class="hidden_mask popup_element_second"></div>
<div class="popup_container popup_element">
</div>
<div id="hiddenMask" class="popup_bg">
	<table cellpadding="5" cellspacing="0" id="dr_sweet_advise" class="dr_sweet_advise" align="center" style="height:100px;">
		<tr>
		<td>
			<div class="drword" id="drword" style=""></div>
		</td>
		</tr>
	</table>
<!--[if lte IE 6.5]><iframe class="hackiframe"></iframe><![endif]-->
</div>
<iframe name="hidden_frame" id="hidden_frame" src="" width="0" height="0" frameborder="0"></iframe>
<form method="post" name="form" action="/start_apply.htm" target="hidden_frame">
<input type="hidden" name="productid" value="<% nvram_get("productid"); %>">
<input type="hidden" name="current_page" value="Advanced_WAN_Content.asp">
<input type="hidden" name="next_page" value="Advanced_WAN_Content.asp">
<input type="hidden" name="modified" value="0">
<input type="hidden" name="action_mode" value="apply">
<input type="hidden" name="action_script" value="restart_wan_if">
<input type="hidden" name="action_wait" value="5">
<input type="hidden" name="preferred_lang" id="preferred_lang" value="<% nvram_get("preferred_lang"); %>">
<input type="hidden" name="firmver" value="<% nvram_get("firmver"); %>">
<input type="hidden" name="lan_ipaddr" value="<% nvram_get("lan_ipaddr"); %>" />
<input type="hidden" name="lan_netmask" value="<% nvram_get("lan_netmask"); %>" />
<input type="hidden" name="wan_unit" value="<% nvram_get("wan_unit"); %>">
<input type="hidden" name="wan_clientid_type" value="">
<input type="hidden" name="wan_dnsenable_x" value="<% nvram_get("wan_dnsenable_x"); %>">
<input type="hidden" name="wan_dns1_x" value="<% nvram_get("wan_dns1_x"); %>">
<input type="hidden" name="wan_dns2_x" value="<% nvram_get("wan_dns2_x"); %>">
<input type="hidden" name="dnspriv_rulelist" value="<% nvram_get("dnspriv_rulelist"); %>" disabled>
<input type="hidden" name="ipv6_service" value="<% nvram_get("ipv6_service"); %>" disabled>
<input type="hidden" name="bond_wan" value="<% nvram_get("bond_wan"); %>" disabled>
<input type="hidden" name="ipv6_s46_b4addr" value="<% nvram_get("ipv6_s46_b4addr"); %>" disabled>
<!--input type="hidden" name="wan_dhcpenable_x" value="<% nvram_get("wan_dhcpenable_x"); %>"-->
<!--===================================Beginning of Auto WAN Detection Confirm===========================================-->
<div id="autowan_hint_div" style="display: none;">
	<div style="width: 100%; height: 100%; display: flex; flex-direction: column; justify-content: space-evenly; align-items: center;">
		<div id="autowan_hint" style="width: 80%; line-height: 20px;"></div>
		<div id="schematic_diagram" style="width: 480px; height: 330px;">
			<div class="port_img"></div>
			<div class="port_plugin_img"></div>
		</div>
		<div id="hint_action_div" style="display: flex; width: 80%; justify-content: space-evenly; margin-bottom: 30px;">
			<input class="button_gen" type="button" value="<#CTL_Cancel#>" onclick="close_autowan_hint();">
			<input class="button_gen" type="button" value="<#CTL_ok#>" onclick="confirm_autowan_change();">
		</div>
	</div>
</div>
<!--===================================End of Auto WAN Detection Confirm===========================================-->
<span id="bridgePPPoE_relay"></span>
<table class="content" align="center" cellpadding="0" cellspacing="0">
	<tr>
		<td width="17">&nbsp;</td>
	<!--=====Beginning of Main Menu=====-->
		<td valign="top" width="202">
		  <div id="mainMenu"></div>
		  <div id="subMenu"></div>
		</td>

		<td height="430" valign="top">
			<div id="tabMenu" class="submenuBlock"></div>
		  <!--===================================Beginning of Main Content===========================================-->
			<table width="98%" border="0" align="left" cellpadding="0" cellspacing="0">
				<tr>
					<td valign="top">
						<table width="760px" border="0" cellpadding="5" cellspacing="0" class="FormTitle" id="FormTitle">
							<tbody>
								<tr>
								<td bgcolor="#4D595D" valign="top">
									<div>&nbsp;</div>
									<div class="formfonttitle"><#menu5_3#> - <#menu5_3_1#></div>
									<div style="margin: 10px 0 10px 5px;" class="splitLine"></div>
									<div id="desc_default" class="formfontdesc"><#dsl_wan_page_desc#></div>
									<div id="desc_edit" class="formfontdesc"><#Layer3Forwarding_x_ConnectionType_sectiondesc#></div>
									<div style="font-size: 13px; text-decoration: underline; margin-left: 5px; cursor: pointer;" onclick="show_popup('new');">Add Profile</div>
									<table id="WANscap" width="100%" border="1" align="center" cellpadding="4" cellspacing="0" bordercolor="#6b8fa3" class="FormTable">
										<thead>
										<tr>
											<td colspan="2"><#wan_index#></td>
										</tr>
										</thead>
										<tr>
											<th><#wan_type#></th>
											<td align="left">
												<select id="wan_unit_x" class="input_option" name="wan_unit_x" onchange="change_wan_unit(this);"></select>
											</td>
										</tr>
									</table>

									<table width="100%" border="1" align="center" cellpadding="4" cellspacing="0" bordercolor="#6b8fa3" class="FormTable" id="MS_WAN_table">
										<thead>
										<tr>
											<td colspan="8"><#DSL_multiserv_summary#></td>
										</tr>
										</thead>
											<tr>
												<th style="width:10%;"><center>Index</center></th>
												<th style="width:20%;"><center>802.1Q Enable</center></th>
												<th style="width:10%;"><center>VLAN ID</center></th>
												<th style="width:20%;"><center><#IPConnection_VServerProto_itemname#></center></th>
												<th style="width:10%;"><center><#Internet#></center></th>
												<th style="width:10%;"><center><#pvccfg_edit#></center></th>
												<th style="width:10%;" id="MS_WAN_add_del"><center><#CTL_del#></center></th>
											</tr>
									</table>

									<table id="t2BC" width="100%" border="1" align="center" cellpadding="4" cellspacing="0" bordercolor="#6b8fa3" class="FormTable">
										<thead>
										<tr>
											<td colspan="2">Internet Settings</td><!--untranslated-->
										</tr>
										</thead>
										<tr>
											<th>Profile</th>
											<td><span id="pvc_sel" style="color:white;"></span></td>
										</tr>
										<tr>
											<th><#Layer3Forwarding_x_ConnectionType_itemname#></th>
											<td align="left">
												<div style="display: flex; align-items: center;">
													<select class="input_option" name="wan_proto" onchange="change_wan_proto_type(this.value);fixed_change_wan_proto_type(this.value);">
														<option value="dhcp" <% nvram_match("wan_proto", "dhcp", "selected"); %>><#BOP_ctype_title1#></option>
														<option value="static" <% nvram_match("wan_proto", "static", "selected"); %>><#BOP_ctype_title5#></option>
														<option value="pppoe" <% nvram_match("wan_proto", "pppoe", "selected"); %>>PPPoE</option>
														<option value="bridge" <% nvram_match("wan_proto", "bridge", "selected"); %>>Bridge</option>
													</select>
													<div id="s46reset_div" style="display: flex; align-items: center;">
														<div id="s46btn_div" style="display: flex; align-items: center;">
															<div id="s46reset_btn" style="margin-left: 5px;"><input type="button" class="button_gen" onclick="update_map();" value="<#MAP_update#>"></div>
															<div id="s46reset_help" style="margin-left: 5px; cursor: pointer;"><a id = "s46reset_faq" href="" target="_blank"><img src="/images/New_ui/bottom_help.png"></a></div>
														</div>
													</div>
												</div>
											</td>
										</tr>
										<tr>
											<th><#Enable_WAN#></th>
											<td>
												<input type="radio" name="wan_enable" class="input" value="1" <% nvram_match("wan_enable", "1", "checked"); %>><#checkbox_Yes#>
												<input type="radio" name="wan_enable" class="input" value="0" <% nvram_match("wan_enable", "0", "checked"); %>><#checkbox_No#>
											</td>
										</tr>
										<tr>
											<th><a class="hintstyle" href="javascript:void(0);" onClick="openHint(7,22);"><#Enable_NAT#></a></th>
											<td>
												<input type="radio" name="wan_nat_x" class="input" value="1" <% nvram_match("wan_nat_x", "1", "checked"); %>><#checkbox_Yes#>
												<input type="radio" name="wan_nat_x" class="input" value="0" <% nvram_match("wan_nat_x", "0", "checked"); %>><#checkbox_No#>
											</td>
										</tr>
										<tr>
											<th><a class="hintstyle" href="javascript:void(0);" onClick="openHint(7,23);"><#BasicConfig_EnableMediaServer_itemname#></a></th>
											<td>
												<input type="radio" name="wan_upnp_enable" class="input" value="1" onclick="return change_common_radio(this, 'LANHostConfig', 'wan_upnp_enable', '1')" <% nvram_match("wan_upnp_enable", "1", "checked"); %>><#checkbox_Yes#>
												<input type="radio" name="wan_upnp_enable" class="input" value="0" onclick="return change_common_radio(this, 'LANHostConfig', 'wan_upnp_enable', '0')" <% nvram_match("wan_upnp_enable", "0", "checked"); %>><#checkbox_No#>
											</td>
										</tr>
										<tr style="display: none;">
											<th><a class="hintstyle" href="javascript:void(0);" onClick=""><#BasicConfig_EnableMediaServer_itemname#></a></th>
											<td>
												<input type="radio" name="wan_dhcpfilter_enable" class="input" value="1" <% nvram_match("wan_dhcpfilter_enable", "1", "checked"); %>><#checkbox_Yes#>
												<input type="radio" name="wan_dhcpfilter_enable" class="input" value="0" <% nvram_match("wan_dhcpfilter_enable", "0", "checked"); %>><#checkbox_No#>
											</td>
										</tr>
										<tr style="display:none;">
											<th><#WANAggregation_enable#></th>
											<td>
												<input type="radio" name="bond_wan_radio" class="input" value="1" onclick="return change_common_radio(this, 'LANHostConfig', 'bond_wan', '1')" <% nvram_match("bond_wan", "1", "checked"); %>><#checkbox_Yes#>
												<input type="radio" name="bond_wan_radio" class="input" value="0" onclick="return change_common_radio(this, 'LANHostConfig', 'bond_wan', '0')" <% nvram_match("bond_wan", "0", "checked"); %>><#checkbox_No#>
												<div id="wanAgg_desc" style="color:#FFCC00;"><#WANAggregation_desc#></div>
												<select id="wanports_bond_menu" class="input_option" style="display:none;" name="wanports_bond" onchange="change_wanAggre_desc();" disabled>
													<option value="0 1" <% find_word("wanports_bond", "1", "selected"); %>>LAN 1</option>
													<option value="0 2" <% find_word("wanports_bond", "2", "selected"); %>>LAN 2</option>
													<option value="0 30" <% find_word("wanports_bond", "30", "selected"); %>>10G base-T</option>
												</select>
											</td>
										</tr>
									</table>

									<table id="dot1q_setting" width="100%" border="1" align="center" cellpadding="4" cellspacing="0" bordercolor="#6b8fa3" class="FormTable">
									<thead><tr><td colspan="2">802.1Q</td></tr></thead>
									<tr>
										<th><#WLANConfig11b_WirelessCtrl_button1name#></th>
										<td>
											<input type="radio" name="wan_dot1q" class="input" value="1" onclick="return change_common_radio(this, 'IPConnection', 'wan_dot1q', 1);" <% nvram_match("wan_dot1q", "1", "checked"); %>><#checkbox_Yes#>
											<input type="radio" name="wan_dot1q" class="input" value="0" onclick="return change_common_radio(this, 'IPConnection', 'wan_dot1q', 0);" <% nvram_match("wan_dot1q", "0", "checked"); %>><#checkbox_No#>
										</td>
									</tr>
									<tr>
										<th>VLAN ID</th>
										<td>
											<input type="text" name="wan_vid" maxlength="4" class="input_6_table" value="<% nvram_get("wan_vid"); %>" onKeyPress="return validator.isNumber(this,event);"> ( 2 ~ 4094 )
										</td>
									</tr>
									<tr>
										<th>802.1P</th>
										<td>
											<input type="text" name="wan_dot1p" maxlength="4" class="input_6_table" value="<% nvram_get("wan_dot1p"); %>" onKeyPress="return validator.isNumber(this,event);"> ( 0 ~ 7 )
										</td>
									</tr>
									</table>

									<table id="IPsetting" width="100%" border="1" align="center" cellpadding="4" cellspacing="0" bordercolor="#6b8fa3" class="FormTable">
									<thead>
									<tr>
										<td colspan="2"><#IPConnection_ExternalIPAddress_sectionname#></td>
									</tr>
									</thead>
							
									<tr>
										<th><#Layer3Forwarding_x_DHCPClient_itemname#></th>
										<td>
											<input type="radio" name="wan_dhcpenable_x" class="input" value="1" onclick="change_wan_dhcp_enable(0);" <% nvram_match("wan_dhcpenable_x", "1", "checked"); %>><#checkbox_Yes#>
											<input type="radio" name="wan_dhcpenable_x" class="input" value="0" onclick="change_wan_dhcp_enable(0);" <% nvram_match("wan_dhcpenable_x", "0", "checked"); %>><#checkbox_No#>
										</td>
									</tr>
            
									<tr>
										<th><a class="hintstyle" href="javascript:void(0);" onClick="openHint(7,1);"><#IPConnection_ExternalIPAddress_itemname#></a></th>
										<td>
											<input type="text" id="wan_ipaddr_x" name="wan_ipaddr_x" maxlength="15" class="input_15_table" value="<% nvram_get("wan_ipaddr_x"); %>" onKeyPress="return validator.isIPAddr(this, event);" autocorrect="off" autocapitalize="off">
										</td>
									</tr>
							
									<tr>
										<th><a class="hintstyle" href="javascript:void(0);" onClick="openHint(7,2);"><#IPConnection_x_ExternalSubnetMask_itemname#></a></th>
										<td>
											<input type="text" name="wan_netmask_x" maxlength="15" class="input_15_table" value="<% nvram_get("wan_netmask_x"); %>" onKeyPress="return validator.isIPAddr(this, event);" autocorrect="off" autocapitalize="off">
										</td>
									</tr>
							
									<tr>
										<th><a class="hintstyle" href="javascript:void(0);" onClick="openHint(7,3);"><#IPConnection_x_ExternalGateway_itemname#></a></th>
										<td>
											<input type="text" id="wan_gateway_x" name="wan_gateway_x" maxlength="15" class="input_15_table" value="<% nvram_get("wan_gateway_x"); %>" onKeyPress="return validator.isIPAddr(this, event);" autocorrect="off" autocapitalize="off">
										</td>
									</tr>
									</table>

						<table id="dslite_setting" width="100%" border="1" align="center" cellpadding="4" cellspacing="0" bordercolor="#6b8fa3" class="FormTable" style="display:none">
						<thead>
							<tr>
								<td colspan="2"><#IPConnection_ExternalIPAddress_sectionname#></td>
							</tr>
						</thead>
							<tr>
								<th><#Connection_Type#></th>
								<td>
									<select id="wan_s46_dslite_mode" class="input_option" name="wan_s46_dslite_mode" onchange="change_dslite_mode(this.value)">
										<option value="0" <% nvram_match("wan_s46_dslite_mode", "0", "selected"); %>><#QKSet_detect_sanglass#></option>
										<option value="1" <% nvram_match("wan_s46_dslite_mode", "1", "selected"); %>><#Manual#></option>
								</td>
							</tr>
							<tr id="wan_s46_dslite_svc_r" style="display:none;">
								<th><#BM_UserList1#></th>
								<td>
									<div id="wan_s46_dslite_svc_span" name="wan_s46_dslite_svc_span" style="color:#FFFFFF;margin-left:8px;"></div>
								</td>
							</tr>
							<tr>
								<th>AFTR Address</th>
								<td>
									<input type="text" id="ipv6_s46_aftr" name="ipv6_s46_aftr" maxlength="39" class="input_32_table" value="<% nvram_get("ipv6_s46_aftr"); %>" onKeyPress="" autocorrect="off" autocapitalize="off">
									<img id="pull_arrow" height="14px;" src="/images/unfold_more.svg" style="position:absolute;*margin-left:-3px;*margin-top:1px;" onclick="pullaftrList(this);" title="<#select_service#>" onmouseover="over_var=1;" onmouseout="over_var=0;">
									<div id="aftr_Block_PC" class="aftr_Block_PC" style="display:none;"></div>
								</td>
							</tr>
							<tr id="ipv6_s46_aftr_r" style="display:none;">
								<th>AFTR Address</th>
								<td>
									<div id="ipv6_s46_aftr_span" name="ipv6_s46_aftr_span" style="color:#FFFFFF;margin-left:8px;"></div>
								</td>
							</tr>
							<tr>
								<th>B4 IPv4 Address</th>
								<td>
									<select id="ipv6_s46_b4addr_Select" name="ipv6_s46_b4addr_Select" class="input_option"></select>
								</td>
							</tr>
							<tr id="ipv6_s46_b4addr_r" style="display:none;">
								<th>B4 IPv4 Address</th>
								<td>
									<div id="ipv6_s46_b4addr_span" name="ipv6_s46_b4addr_span" style="color:#FFFFFF;margin-left:8px;"></div>
								</td>
							</tr>
						</table>

						<table id="S46setting" width="100%" border="1" align="center" cellpadding="4" cellspacing="0" bordercolor="#6b8fa3" class="FormTable" style="display:none">
							<thead>
							<tr>
								<td colspan="2"><#IPConnection_ExternalIPAddress_sectionname#></td>
							</tr>
							</thead>
							<tr>
								<th><#Layer3Forwarding_x_DHCPClient_itemname#></th>
								<td>
									<input type="radio" name="wan_s46_dhcpenable_x" class="input" value="1" onclick="change_wan_dhcp_enable(0);" <% nvram_match("wan_dhcpenable_x", "1", "checked"); %>><#checkbox_Yes#>
									<input type="radio" name="wan_s46_dhcpenable_x" class="input" value="0" onclick="change_wan_dhcp_enable(0);" <% nvram_match("wan_dhcpenable_x", "0", "checked"); %>><#checkbox_No#>
								</td>
							</tr>
							<tr>
								<th><a class="hintstyle" href="javascript:void(0);" onClick="openHint(-1,-1);">BR / AFTR IPv6 Address</a></th>
								<td><input type="text" name="wan_s46_peer_x" maxlength="39" class="input_32_table" value="<% nvram_get("wan_s46_peer_x"); %>"  autocorrect="off" autocapitalize="off"></td>
							</tr>
							<tr>
								<th>
									<a id="s46_prefix_hint" class="hintstyle" href="javascript:void(0);" onClick="openHint(-1,-1);">IP Prefix</a>
									<a id="s46_ipaddr_hint" class="hintstyle" href="javascript:void(0);" onClick="openHint(7,1);" style="display:none"><#IPConnection_ExternalIPAddress_itemname#></a>
								</th>
								<td><input type="text" name="wan_s46_prefix4_x" maxlength="15" class="input_15_table" value="<% nvram_get("wan_s46_prefix4_x"); %>" onKeyPress="return validator.isIPAddr(this, event);" autocorrect="off" autocapitalize="off"></td>
							</tr>
							<tr>
								<th><a class="hintstyle" href="javascript:void(0);" onClick="openHint(-1,-1);">IP Prefix Length</a></th>
								<td><input type="text" name="wan_s46_prefix4len_x" maxlength="2" class="input_3_table" value="<% nvram_get("wan_s46_prefix4len_x"); %>" autocorrect="off" autocapitalize="off"></td>
							</tr>
							<tr>
								<th><a class="hintstyle" href="javascript:void(0);" onClick="openHint(-1,-1);"><#ipv6_6rd_Prefix#></a></th>
								<td><input type="text" name="wan_s46_prefix6_x" maxlength="39" class="input_32_table" value="<% nvram_get("wan_s46_prefix6_x"); %>"  autocorrect="off" autocapitalize="off"></td>
							</tr>
							<tr>
								<th><a class="hintstyle" href="javascript:void(0);" onClick="openHint(-1,-1);"><#IPv6_Prefix_Length#></a></th>
								<td><input type="text" name="wan_s46_prefix6len_x" maxlength="3" class="input_3_table" value="<% nvram_get("wan_s46_prefix6len_x"); %>" autocorrect="off" autocapitalize="off"></td>
							</tr>
							<tr>
								<th><a class="hintstyle" href="javascript:void(0);" onClick="openHint(-1,-1);">EA-bit Length</a></th>
								<td><input type="text" name="wan_s46_ealen_x" maxlength="2" class="input_3_table" value="<% nvram_get("wan_s46_ealen_x"); %>" autocorrect="off" autocapitalize="off"></td>
							</tr>
							<tr>
								<th><a class="hintstyle" href="javascript:void(0);" onClick="openHint(-1,-1);">PSID Offset</a></th>
								<td><input type="text" name="wan_s46_offset_x" maxlength="2" class="input_3_table" value="<% nvram_get("wan_s46_offset_x"); %>" autocorrect="off" autocapitalize="off"></td>
							</tr>
							<tr>
								<th><a class="hintstyle" href="javascript:void(0);" onClick="openHint(-1,-1);">PSID Length</a></th>
								<td><input type="text" name="wan_s46_psidlen_x" maxlength="2" class="input_3_table" value="<% nvram_get("wan_s46_psidlen_x"); %>" autocorrect="off" autocapitalize="off"></td>
							</tr>
							<tr>
								<th><a class="hintstyle" href="javascript:void(0);" onClick="openHint(-1,-1);">PSID</a></th>
								<td><input type="text" name="wan_s46_psid_x" maxlength="5" class="input_3_table" value="<% nvram_get("wan_s46_psid_x"); %>" autocorrect="off" autocapitalize="off"></td>
							</tr>
						</table>

						<table id="DNSsetting" width="100%" border="1" align="center" cellpadding="4" cellspacing="0" bordercolor="#6b8fa3"  class="FormTable">
						<thead>
							<tr>
								<td colspan="2"><#IPConnection_x_DNSServerEnable_sectionname#></td>
							</tr>
						</thead>
						<tr>
							<th><a class="hintstyle" href="javascript:void(0);" onClick="openHint(7,12);"><#PPPConnection_x_WANDNSServer_itemname#></a></th>
							<td>
									<div id="DNS_status"></div>
									<div id="DNS_Assign_splitLine" style="margin:10px 0 5px 0;"></div>
									<div id="DNS_Assign_desc"><#DNS_Assign_desc#></div>
									<div id="DNS_Assign_button" style="text-align:right;"><input id="assign_button" type="button" class="button_gen" onclick="Assign_DNS_service()" value="<#CTL_assign#>"></div>
							</td>
						</tr>
						<tr>
							<th><a class="hintstyle" href="javascript:void(0);" onClick="openHint(7,42);"><#WAN_Queries_Upstream_DNS#></a></th>
							<td>
								<input type="radio" value="1" name="dns_fwd_local" <% nvram_match("dns_fwd_local", "1", "checked"); %> /><#checkbox_Yes#>
								<input type="radio" value="0" name="dns_fwd_local" <% nvram_match("dns_fwd_local", "0", "checked"); %> /><#checkbox_No#>
							</td>
						</tr>
						<tr>
							<th><a class="hintstyle" href="javascript:void(0);" onClick="openHint(7,43);"><#WAN_DNS_Rebind#></a></th>
							<td>
								<input type="radio" value="1" name="dns_norebind" <% nvram_match("dns_norebind", "1", "checked"); %> /><#checkbox_Yes#>
								<input type="radio" value="0" name="dns_norebind" <% nvram_match("dns_norebind", "0", "checked"); %> /><#checkbox_No#>
							</td>
						</tr>
						<tr id="dnssec_tr" style="display:none;">
							<th><a class="hintstyle" href="javascript:void(0);" onClick="openHint(7,44);"><#WAN_DNSSEC_Support#></a></th>
							<td>
								<input type="radio" value="1" name="dnssec_enable" onclick="showhide('dnssec_strict_tr',1);" <% nvram_match("dnssec_enable", "1", "checked"); %> /><#checkbox_Yes#>
								<input type="radio" value="0" name="dnssec_enable" onclick="showhide('dnssec_strict_tr',0);" <% nvram_match("dnssec_enable", "0", "checked"); %> /><#checkbox_No#>
							</td>
						</tr>
						<tr id="dnssec_strict_tr" style="display:none;">
								<th><a class="hintstyle" href="javascript:void(0);" onClick="openHint(7,45);"><#WAN_Valid_Unsigned_DNSSEC#></a></th>
								<td>
									<input type="radio" value="1" name="dnssec_check_unsigned_x" <% nvram_match("dnssec_check_unsigned_x", "1", "checked"); %> /><#checkbox_Yes#>
									<input type="radio" value="0" name="dnssec_check_unsigned_x" <% nvram_match("dnssec_check_unsigned_x", "0", "checked"); %> /><#checkbox_No#>
								</td>
						</tr>
						<tr id="dns_priv_override_tr">
								<th><a class="hintstyle" href="javascript:void(0);" onClick="openHint(7,46);"><#WAN_Prevent_DoH#></a></th>
								<td>
									<select id="dns_priv_override" class="input_option" name="dns_priv_override">
										<option value="0" <% nvram_match("dns_priv_override", "0", "selected"); %>><#Auto#></option>
										<option value="1" <% nvram_match("dns_priv_override", "1", "selected"); %>><#checkbox_Yes#></option>
										<option value="2" <% nvram_match("dns_priv_override", "2", "selected"); %>><#checkbox_No#></option>
									</select>
								</td>
						</tr>
						<tr style="display:none">
								<th>
									<a class="hintstyle" href="javascript:void(0);" onClick="openHint(7,35);"><#WAN_DNS_Privacy#></a>
								</th>
								<td align="left">
									<select id="dnspriv_enable" class="input_option" name="dnspriv_enable" onChange="change_dnspriv_enable(this.value);">
										<option value="0" <% nvram_match("dnspriv_enable", "0", "selected"); %>><#wl_securitylevel_0#></option>
										<option value="1" <% nvram_match("dnspriv_enable", "1", "selected"); %>>DNS-over-TLS (DoT)</option>
										<!--option value="2" <% nvram_match("dnspriv_enable", "2", "selected"); %>>DNS-over-HTTPS (DoH)</option-->
										<!--option value="3" <% nvram_match("dnspriv_enable", "3", "selected"); %>>DNS-over-TLS/HTTPS (DoT+DoH)</option-->
									</select>
									<div id="yadns_hint_dnspriv" style="display:none;"></div>
								</td>
						</tr>
						<tr style="display:none">
							<th>
								<a class="hintstyle" href="javascript:void(0);" onClick="openHint(7,36);"><#WAN_DNS_over_TLS#></a>
							</th>
							<td>
								<input type="radio" name="dnspriv_profile" class="input" value="1" onclick="return change_common_radio(this, 'IPConnection', 'dnspriv_profile', 1)" <% nvram_match("dnspriv_profile", "1", "checked"); %> /><#WAN_DNS_over_TLS_Strict#>
								<input type="radio" name="dnspriv_profile" class="input" value="0" onclick="return change_common_radio(this, 'IPConnection', 'dnspriv_profile', 0)" <% nvram_match("dnspriv_profile", "0", "checked"); %> /><#WAN_DNS_over_TLS_Opportunistic#>
							</td>
						</tr>
						<tr style="display:none" id="dot_presets_tr">
							<th><a class="hintstyle" href="javascript:void(0);" onClick="openHint(7,41);"><div class="table_text"><#WAN_DNS_dot_presets#></a></th>
							<td>
								<select name="dotPresets" id="dotPresets" class="input_option" onchange="change_wizard(this, 'dotPresets');">
							</td>
						</tr>
						</table>

									<table id="DNSPrivacy" width="100%" border="1" align="center" cellpadding="4" cellspacing="0" bordercolor="#6b8fa3" class="FormTable_table" style="display:none">
									<thead>
			  						<tr>
										<td colspan="5"><#WAN_DNS_over_TLS_server#>&nbsp;(<#List_limit#>&nbsp;8)</td>
			  						</tr>
									</thead>
									<tr>
										<th><a href="javascript:void(0);" onClick="openHint(7,37);"><div class="table_text"><div class="table_text"><#IPConnection_ExternalIPAddress_itemname#></div></a></th>
										<th><a href="javascript:void(0);" onClick="openHint(7,38);"><div class="table_text"><div class="table_text"><#WAN_DNS_over_TLS_server_port#></div></a></th>
										<th><a href="javascript:void(0);" onClick="openHint(7,39);"><div class="table_text"><div class="table_text"><#WAN_DNS_over_TLS_server_name#></div></a></th>
										<th><a href="javascript:void(0);" onClick="openHint(7,40);"><div class="table_text"><div class="table_text"><#WAN_DNS_over_TLS_server_SPKI#></div></a></th>
										<th><#list_add_delete#></th>
									</tr>
									<!-- server info -->
									<tr>
										<td width="27%"><input type="text" class="input_20_table" maxlength="64" name="dnspriv_server_0" onKeyPress="" autocorrect="off" autocapitalize="off"></td>
										<td width="10%"><input type="text" class="input_6_table" maxlength="5" name="dnspriv_port_0" onKeyPress="return validator.isNumber(this,event)" autocorrect="off" autocapitalize="off"></td>
										<td width="27%"><input type="text" class="input_20_table" maxlength="64" name="dnspriv_hostname_0" onKeyPress="" autocorrect="off" autocapitalize="off"></td>
										<td width="27%"><input type="text" class="input_20_table" maxlength="64" name="dnspriv_spkipin_0" onKeyPress="" autocorrect="off" autocapitalize="off"></td>
										<td width="9%"><div><input type="button" class="add_btn" onClick="addRow_Group(8);" value=""></div></td>
									</tr>
									</table>
									<!-- server block -->
									<div id="dnspriv_rulelist_Block"></div>

									<table id="wan_DHCP_opt" width="100%" border="1" align="center" cellpadding="4" cellspacing="0" bordercolor="#6b8fa3"  class="FormTable">
										<thead>
											<tr><td colspan="2"><#ipv6_6rd_dhcp_option#></td></tr>
										</thead>
										<tr>
											<th width="40%"><#DHCPoption_Class#> (<#NetworkTools_option#> 60):</th>
											<td>
												<input type="text" name="wan_vendorid" class="input_25_table" value="<% nvram_get("wan_vendorid"); %>" maxlength="126" autocapitalization="off" autocomplete="off">
											</td>
										</tr>
										<tr>
											<th width="40%"><#DHCPoption_Client#> (<#NetworkTools_option#> 61):</th>
											<td>
												<input type="checkbox" id="tmp_dhcp_clientid_type" name="tmp_dhcp_clientid_type" onclick="showDiableDHCPclientID(this);" <% nvram_match("wan_clientid_type", "1", "checked"); %>>IAID/DUID<br>
												<input type="text" name="wan_clientid" class="input_25_table" value="<% nvram_get("wan_clientid"); %>" maxlength="126" autocapitalization="off" autocomplete="off">
											</td>
										</tr>
									</table>

									<table id="PPPsetting" width="100%" border="1" align="center" cellpadding="4" cellspacing="0" bordercolor="#6b8fa3"  class="FormTable">
										<thead>
										<tr>
											<td colspan="2"><#PPPConnection_UserName_sectionname#></td>
										</tr>
										</thead>
										<tr>
											<th><a class="hintstyle" href="javascript:void(0);" onClick="openHint(7,29);"><#PPPConnection_Authentication_itemname#></a>
											</th>
											<td align="left">
							    				<select class="input_option" name="wan_auth_x" onChange="change_wan_type(document.form.wan_proto.value);">
							    				<option value="" <% nvram_match("wan_auth_x", "", "selected"); %>><#wl_securitylevel_0#></option>
							    				<option value="8021x-md5" <% nvram_match("wan_auth_x", "8021x-md5", "selected"); %>>802.1x MD5</option>
							    				</select>
							    			</td>
										</tr>

										<tr>
											<th>
												<a class="hintstyle" href="javascript:void(0);" onClick="openHint(7,4);"><#Username#></a>
											</th>
											<td>
												<input type="text" maxlength="64" class="input_32_table" name="wan_pppoe_username" value="<% nvram_get("wan_pppoe_username"); %>" onkeypress="return validator.isString(this, event)" onblur="" autocomplete="off" autocorrect="off" autocapitalize="off">
											</td>
										</tr>
										<tr>
											<th>
												<a class="hintstyle" href="javascript:void(0);" onClick="openHint(7,5);"><#PPPConnection_Password_itemname#></a>
											</th>
											<td>
												<div style="margin-top:2px;"><input type="password" maxlength="64" class="input_32_table" id="wan_pppoe_passwd" name="wan_pppoe_passwd" value="<% nvram_get("wan_pppoe_passwd"); %>" autocomplete="off" autocorrect="off" autocapitalize="off"></div>
												<div style="margin-top:1px;"><input type="checkbox" name="show_pass_1" onclick="pass_checked(document.form.wan_pppoe_passwd);"><#QIS_show_pass#></div>
											</td>
										</tr>
										<tr style="display:none">
											<th>
												<a class="hintstyle" href="javascript:void(0);" onClick="openHint(7,6);"><#WANPPPAuthText#></a>
											</th>
											<td>
												<select class="input_option" name="wan_pppoe_auth">
												<option value="" <% nvram_match("wan_pppoe_auth", "", "selected"); %>><#Auto#></option>
												<option value="pap" <% nvram_match("wan_pppoe_auth", "pap", "selected"); %>>PAP</option>
												<option value="chap" <% nvram_match("wan_pppoe_auth", "chap", "selected"); %>>CHAP</option>
												</select>
											</td>
										</tr>
										<tr>
											<th>
												<a class="hintstyle" href="javascript:void(0);" onClick="openHint(7,6);"><#PPPConnection_IdleDisconnectTime_itemname#></a>
											</th>
											<td>
												<input type="text" maxlength="10" class="input_12_table" name="wan_pppoe_idletime" value="<% nvram_get("wan_pppoe_idletime"); %>" onkeypress="return validator.isNumber(this,event)" autocorrect="off" autocapitalize="off"/>&nbsp<#Second#>
												<input type="checkbox" style="margin-left:30;display:none;" name="wan_pppoe_idletime_check" value="" />
											</td>
										</tr>
										<tr>
											<th>
												<a class="hintstyle" href="javascript:void(0);" onClick="openHint(7,7);"><#PPPConnection_x_PPPoEMTU_itemname#></a>
											</th>
											<td>
												<input type="text" maxlength="5" name="wan_pppoe_mtu" class="input_6_table" value="<% nvram_get("wan_pppoe_mtu"); %>" onKeyPress="return validator.isNumber(this,event);" autocorrect="off" autocapitalize="off"/>&nbsp;128 - 1492
											</td>
										</tr>
										<tr>
											<th>
												<a class="hintstyle" href="javascript:void(0);" onClick="openHint(7,8);"><#PPPConnection_x_PPPoEMRU_itemname#></a>
											</th>
											<td>
												<input type="text" maxlength="5" name="wan_pppoe_mru" class="input_6_table" value="<% nvram_get("wan_pppoe_mru"); %>" onKeyPress="return validator.isNumber(this,event);" autocorrect="off" autocapitalize="off"/>&nbsp;128 - 1492
											</td>
										</tr>
										<tr>
											<th>
												<a class="hintstyle" href="javascript:void(0);" onClick="openHint(7,9);"><#PPPConnection_x_ServiceName_itemname#></a>
											</th>
											<td>
												<input type="text" maxlength="32" class="input_32_table" name="wan_pppoe_service" value="<% nvram_get("wan_pppoe_service"); %>" onkeypress="return validator.isString(this, event)" onblur="" autocorrect="off" autocapitalize="off"/>
											</td>
										</tr>
										<tr>
											<th>
												<a class="hintstyle" href="javascript:void(0);" onClick="openHint(7,10);"><#PPPConnection_x_AccessConcentrator_itemname#></a>
											</th>
											<td>
												<input type="text" maxlength="32" class="input_32_table" name="wan_pppoe_ac" value="<% nvram_get("wan_pppoe_ac"); %>" onkeypress="return validator.isString(this, event)" autocorrect="off" autocapitalize="off"/>
											</td>
										</tr>
										<tr>
											<th>
												<a class="hintstyle" href="javascript:void(0);" onClick="openHint(7,18);">Host-Uniq (<#Hexadecimal#>)</a>
											</th>
											<td align="left">
												<input type="text" maxlength="256" class="input_32_table" name="wan_pppoe_hostuniq" value="<% nvram_get("wan_pppoe_hostuniq"); %>" onkeypress="return validator.isString(this, event);" autocorrect="off" autocapitalize="off"/>
											</td>
										</tr>

										<tr>
											<th><a class="hintstyle" href="javascript:void(0);" onClick="openHint(7,17);"><#PPPConnection_x_PPTPOptions_itemname#></a>
											</th>
											<td>
												<select name="wan_pptp_options_x" class="input_option">
												<option value="" <% nvram_match("wan_pptp_options_x", "","selected"); %>><#Auto#></option>
												<option value="-mppc" <% nvram_match("wan_pptp_options_x", "-mppc","selected"); %>><#No_Encryp#></option>
												<option value="+mppe-40" <% nvram_match("wan_pptp_options_x", "+mppe-40","selected"); %>>MPPE 40</option>
												<option value="+mppe-128" <% nvram_match("wan_pptp_options_x", "+mppe-128","selected"); %>>MPPE 128</option>
												</select>
											</td>
										</tr>

										<tr>
											<th><a class="hintstyle" href="javascript:void(0);" onClick="openHint(7,31);"><#PPPConnection_x_InternetDetection_itemname#></a></th>
											<td>
												<select name="wan_ppp_echo" class="input_option" onChange="ppp_echo_control();">
												<option value="0" <% nvram_match("wan_ppp_echo", "0","selected"); %>><#btn_disable#></option>
												<option value="1" <% nvram_match("wan_ppp_echo", "1","selected"); %>>PPP Echo</option>
												<option value="2" <% nvram_match("wan_ppp_echo", "2","selected"); %>>DNS Probe</option>
												</select>
											</td>
										</tr>
										<tr>
											<th><a class="hintstyle" href="javascript:void(0);" onClick="openHint(7,32);"><#PPPConnection_x_PPPEcho_Interval#></a></th>
											<td><input type="text" maxlength="6" class="input_6_table" name="wan_ppp_echo_interval" value="<% nvram_get("wan_ppp_echo_interval"); %>" onkeypress="return validator.isNumber(this, event)" autocorrect="off" autocapitalize="off"/></td>
										</tr>
										<tr>
											<th><a class="hintstyle" href="javascript:void(0);" onClick="openHint(7,33);"><#PPPConnection_x_PPPEcho_Max_Failure#></a></th>
											<td><input type="text" maxlength="6" class="input_6_table" name="wan_ppp_echo_failure" value="<% nvram_get("wan_ppp_echo_failure"); %>" onkeypress="return validator.isNumber(this,event);" autocorrect="off" autocapitalize="off"/></td>
										</tr>
										<!--tr>
											<th><a class="hintstyle" href="javascript:void(0);">DNS Probe Timeout</a></th><!--untranslated--\>
											<td><input type="text" maxlength="6" class="input_6_table" name="dns_probe_timeout" value="<% nvram_get("dns_probe_timeout"); %>" onkeypress="return validator.isNumber(this, event)" autocorrect="off" autocapitalize="off"/></td>
										</tr-->
										<tr>
											<th><a class="hintstyle" href="javascript:void(0);" onClick="openHint(7,34);">DNS Probe Max Failures</a></th><!--untranslated-->
											<td><input type="text" maxlength="6" class="input_6_table" name="dns_delay_round" value="<% nvram_get("dns_delay_round"); %>" onkeypress="return validator.isNumber(this,event);" autocorrect="off" autocapitalize="off"/></td>
										</tr>
										<!-- 2008.03 James. patch for Oleg's patch. { -->
										<tr>
											<th>
												<a class="hintstyle" href="javascript:void(0);" onClick="openHint(7,18);"><#PPPConnection_x_AdditionalOptions_itemname#></a>
											</th>
											<td>
												<input type="text" name="wan_pppoe_options_x" value="<% nvram_get("wan_pppoe_options_x"); %>" class="input_32_table" maxlength="255" onKeyPress="return validator.isString(this, event)" onBlur="validator.string(this)" autocorrect="off" autocapitalize="off">
											</td>
										</tr>
										<tr id="ppp_conn_tr" style="display:none;">
											<th>Number of PPP Connection</th>
											<td><input type="text" maxlength="1" name="wan_ppp_conn" class="input_3_table" value="<% nvram_get("wan_ppp_conn"); %>" onKeyPress="return validator.isNumber(this,event);" autocorrect="off" autocapitalize="off"/></td>
										</tr>
										<!-- 2008.03 James. patch for Oleg's patch. } -->
									</table>

									<table id="vpn_server" width="100%" border="1" align="center" cellpadding="4" cellspacing="0" bordercolor="#6b8fa3"  class="FormTable">
	  								<thead>
									<tr>
            							<td colspan="2"><#PPPConnection_x_HostNameForISP_sectionname#></td>
            						</tr>
									</thead>
									<tr>
          								<th><a class="hintstyle" href="javascript:void(0);" onClick="openHint(7,19);"><#BOP_isp_heart_item#></a></th>
          								<td>
          									<!-- 2008.03 James. patch for Oleg's patch. { -->
          									<input type="text" name="wan_heartbeat_x" class="input_32_table" maxlength="256" value="<% nvram_get("wan_heartbeat_x"); %>" onKeyPress="return validator.isString(this, event)" autocorrect="off" autocapitalize="off">
          								</td>
          									<!-- 2008.03 James. patch for Oleg's patch. } -->
        							</tr>
									<tr id="vpn_dhcp">
										<th><#PPPConnection_x_vpn_dhcp_itemname#></th>
										<td><input type="radio" name="wan_vpndhcp" class="input" value="1" onclick="return change_common_radio(this, 'IPConnection', 'wan_vpndhcp', 1)" <% nvram_match("wan_vpndhcp", "1", "checked"); %> /><#checkbox_Yes#>
		    								<input type="radio" name="wan_vpndhcp" class="input" value="0" onclick="return change_common_radio(this, 'IPConnection', 'wan_vpndhcp', 0)" <% nvram_match("wan_vpndhcp", "0", "checked"); %> /><#checkbox_No#>
										</td>
        							</tr>
        							<tr>
          								<th><a class="hintstyle" href="javascript:void(0);" onClick="openHint(7,15);"><#PPPConnection_x_HostNameForISP_itemname#></a></th>
          								<td>
          									<div><input type="text" name="wan_hostname" class="input_32_table" maxlength="32" value="<% nvram_get("wan_hostname"); %>" onkeypress="return validator.isString(this, event)" autocorrect="off" autocapitalize="off"><br/><span id="alert_msg1" style="color:#FC0;"></span></div>
          								</td>
        							</tr>
        							<tr>
          								<th><a class="hintstyle" href="javascript:void(0);" onClick="openHint(7,16);"><#PPPConnection_x_MacAddressForISP_itemname#></a></th>
										<td>
											<input type="text" name="wan_hwaddr_x" class="input_20_table" maxlength="17" value="<% nvram_get("wan_hwaddr_x"); %>" onKeyPress="return validator.isHWAddr(this,event)" autocorrect="off" autocapitalize="off">
											<input type="button" class="btn_subusage my-1 button_gen" onclick="showMAC();" value="<#BOP_isp_MACclone#>">
										</td>
        							</tr>

									<tr>
										<th><a class="hintstyle" href="javascript:void(0);" onClick="openHint(7,30);"><#DHCP_query_freq#></a></th>
										<td>
											<select name="wan_dhcp_qry" class="input_option">
											<option value="0" <% nvram_match("wan_dhcp_qry", "0","selected"); %>><#DHCPnormal#></option>
											<option value="1" <% nvram_match("wan_dhcp_qry", "1","selected"); %>><#DHCPaggressive#></option>
											<option value="2" <% nvram_match("wan_dhcp_qry", "2","selected"); %>><#Continuous_Mode#></option>
											</select>
										</td>
									</tr>
									<tr>
										<th><a class="hintstyle" href="javascript:void(0);" onClick=""><#Extend_TTL_Value#></a></th>
										<td>
											<input type="radio" name="ttl_inc_enable" class="input" value="1" <% nvram_match("ttl_inc_enable", "1", "checked"); %>><#checkbox_Yes#>
											<input type="radio" name="ttl_inc_enable" class="input" value="0" <% nvram_match("ttl_inc_enable", "0", "checked"); %>><#checkbox_No#>
										</td>
									</tr>	
									<tr>
										<th><a class="hintstyle" href="javascript:void(0);" onClick=""><#Spoof_TTL_Value#></a></th>
										<td>
											<input type="radio" name="ttl_spoof_enable" class="input" value="1" <% nvram_match("ttl_spoof_enable", "1", "checked"); %>><#checkbox_Yes#>
											<input type="radio" name="ttl_spoof_enable" class="input" value="0" <% nvram_match("ttl_spoof_enable", "0", "checked"); %>><#checkbox_No#>
										</td>
									</tr>	
									<tr>
										<th>
											<#PPPConnection_x_PPPoEMTU_itemname#>
										</th>
										<td>
											<input type="text" maxlength="5" name="wan_mtu" class="input_6_table" value="<% nvram_get("wan_mtu"); %>" onKeyPress="return validator.isNumber(this,event);" autocorrect="off" autocapitalize="off"/>&nbsp;1280 - 1500
										</td>
									</tr>
									</table>

									<div id="btn_apply" class="apply_gen" style="height:auto">
										<input class="actionButtonCancel button_gen" onclick="exit_to_main();" type="button" value="<#CTL_Cancel#>">
										<input class="button_gen" onclick="applyRule();" type="button" value="<#CTL_ok#>"/>
									</div>

								</td>
							</tr>
							</tbody>
						</table>
					</td>
				</form>
			</table>
		</td>
		<!--===================================Ending of Main Content===========================================-->
		<td width="10" align="center" valign="top">&nbsp;</td>
	</tr>
</table>
<div id="footer"></div>
<form method="post" name="chgpvc" action="/start_apply.htm" target="hidden_frame">
<input type="hidden" name="action_mode" value="apply">
<input type="hidden" name="flag" value="chg_pvc">
<input type="hidden" name="action_script" value="">
<input type="hidden" name="action_wait" value="2">
<input type="hidden" name="current_page" value="Advanced_WAN_Content.asp">
<input type="hidden" name="wan_unit" value="">
</form>
</body>
</html>
