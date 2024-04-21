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
<link rel="stylesheet" type="text/css" href="device-map/device-map.css">
<style>
.FormTable{
 	margin-top:10px;
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
.assign_dns{
	color:#FFF;
	opacity: 0.7;
	max-width:330px;
}
#dns_list_Block{
	font-family: Arial, Helvetica, MS UI Gothic, MS P Gothic, Microsoft Yahei UI, sans-serif;
	width:825px;
	border: 1px solid #FFF;
}

#aftr_Block_PC{
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
	display:none;
}
#aftr_Block_PC div{
	background-color:#576D73;
	height:auto;
	*height:20px;
	line-height:20px;
	text-decoration:none;
	font-family: Lucida Console;
	padding-left:2px;
}

#aftr_Block_PC a{
	background-color:#EFEFEF;
	color:#FFF;
	font-size:12px;
	font-family:Arial, Helvetica, sans-serif;
	text-decoration:none;	
}
#aftr_Block_PC div:hover{
	background-color:#3366FF;
	color:#FFFFFF;
	cursor:default;
}	
</style>
<script type="text/javascript" src="/state.js"></script>
<script type="text/javascript" src="/general.js"></script>
<script type="text/javascript" src="/popup.js"></script>
<script type="text/javascript" src="/help.js"></script>
<script type="text/javascript" src="/validator.js"></script>
<script type="text/javascript" src="/js/jquery.js"></script>
<script type="text/javascript" src="/js/httpApi.js"></script>
<script type="text/javascript" src="/js/searchIspNameProfile.js"></script>
<script>

var wans_dualwan = '<% nvram_get("wans_dualwan"); %>';
var wans_lanport = '<% nvram_get("wans_lanport"); %>';
var wans_extwan = '<% nvram_get("wans_extwan"); %>';
var nowWAN = '<% get_parameter("flag"); %>';
var original_switch_wantag = '<% nvram_get("switch_wantag"); %>';
var original_switch_stb_x = '<% nvram_get("switch_stb_x"); %>';
var original_wan_dot1q = '<% nvram_get("wan_dot1q"); %>';
var original_wan_vid = '<% nvram_get("wan_vid"); %>';
if(wan_bonding_support){
	var orig_bond_wan = httpApi.nvramGet(["bond_wan"], true).bond_wan;
	var orig_wanports_bond = httpApi.nvramGet(["wanports_bond"], true).wanports_bond;
}

if(dualWAN_support && ( wans_dualwan.search("wan") >= 0 || wans_dualwan.search("lan") >= 0)){
	var wan_type_name = wans_dualwan.split(" ")[<% nvram_get("wan_unit"); %>].toUpperCase();
	switch(wan_type_name){
		case "DSL":
			location.href = "Advanced_DSL_Content.asp";
			break;
		case "USB":
			if(based_modelid == "4G-AC53U" || based_modelid == "4G-AC55U" || based_modelid == "4G-AC68U")
				location.href = "Advanced_MobileBroadband_Content.asp";
			else{
				if(based_modelid != "BRT-AC828"){
					location.href = "Advanced_Modem_Content.asp";
				}
			}
			break;
		default:
			break;	
	}
}
<% login_state_hook(); %>
<% wan_get_parameter(); %>

var wan_proto_orig = '<% nvram_get("wan_proto"); %>';
var original_wan_type = wan_proto_orig;
var original_wan_dhcpenable = parseInt('<% nvram_get("wan_dhcpenable_x"); %>');
var original_dnsenable = parseInt('<% nvram_get("wan_dnsenable_x"); %>');
var original_ppp_echo = parseInt('<% nvram_get("wan_ppp_echo"); %>');
var default_ppp_echo = parseInt('<% nvram_default_get("wan_ppp_echo"); %>');
var wan_unit_flag = '<% nvram_get("wan_unit"); %>';

var ipv6_service_orig = '<% nvram_get("ipv6_service"); %>';

if(yadns_support){
	var yadns_enable = '<% nvram_get("yadns_enable_x"); %>';
	var yadns_mode = '<% nvram_get("yadns_mode"); %>';
}

if(dnspriv_support){
	var dot_servers_array = [];
	var dnspriv_rulelist_array = '<% nvram_get("dnspriv_rulelist"); %>';
}

if(dslite_support){
	var s46_dslite_svc_str = httpApi.nvramGet(["wan0_s46_dslite_svc"], true).wan0_s46_dslite_svc;
}

var pppoe_username = decodeURIComponent('<% nvram_char_to_ascii("", "wan_pppoe_username"); %>');
var pppoe_password = decodeURIComponent('<% nvram_char_to_ascii("", "wan_pppoe_passwd"); %>');

var faq_href1 = "https://nw-dlcdnet.asus.com/support/forward.html?model=&type=Faq&lang="+ui_lang+"&kw=&num=125";
var faq_href2 = "https://nw-dlcdnet.asus.com/support/forward.html?model=&type=Faq&lang="+ui_lang+"&kw=&num=126";
var faq_href3 = "https://nw-dlcdnet.asus.com/support/forward.html?model=&type=Faq&lang="+ui_lang+"&kw=&num=127";

var eth_wan_list = httpApi.hookGet("get_ethernet_wan_list", true);

function initial(){
	if (dnspriv_support) {
		if (dnsfilter_support)
			var dnsfilter_enable = '<% nvram_get("dnsfilter_enable_x"); %>';

		var dhcp_dns1 = '<% nvram_get("dhcp_dns1_x"); %>';
		var dhcp_dns2 = '<% nvram_get("dhcp_dns2_x"); %>';
		var lan_ipaddr = '<% nvram_get("lan_ipaddr"); %>';
	}

	if(!dualWAN_support){
		if(wan_unit_flag == 1){	
			document.wanUnit_form.wan_unit.value = 0;
			document.wanUnit_form.target = "";
			document.wanUnit_form.submit();
		}
	}else{
		if('<% nvram_get("wan_unit"); %>' == usb_index){
			change_notusb_unit();
		}
	}
	
	show_menu();

	document.getElementById("faq_nat").href=faq_href1;
	document.getElementById("faq").href=faq_href2;
	
	if(!Softwire46_support){
		$("#wan_proto_menu option[value='lw4o6']").remove();
		$("#wan_proto_menu option[value='map-e']").remove();
		$("#wan_proto_menu option[value='v6plus']").remove();
		$("#wan_proto_menu option[value='ocnvc']").remove();
		$("#wan_proto_menu option[value='dslite']").remove();
	}
	else{
		$("#wan_proto_menu option[value='lw4o6']").remove();
		$("#wan_proto_menu option[value='map-e']").remove();
		if(!ocnvc_support){
			$("#wan_proto_menu option[value='ocnvc']").remove();
		}
		if(!dslite_support){
			$("#wan_proto_menu option[value='dslite']").remove();
		}
		if(dualWAN_support && wan_unit_flag == 1){
			$("#wan_proto_menu option[value='v6plus']").remove();
			$("#wan_proto_menu option[value='ocnvc']").remove();
			$("#wan_proto_menu option[value='dslite']").remove();
		}

		httpApi.faqURL("1050137", function(url){document.getElementById("s46reset_faq").href = url;});
	}

	updatDNSListOnline();
	change_wan_type(document.form.wan_proto.value, 0);	
	fixed_change_wan_type(document.form.wan_proto.value);
	genWANSoption();

	var wan_type = document.form.wan_proto.value;
	if(wan_type == "pppoe" || wan_type == "pptp" || wan_type == "l2tp" ||
	   document.form.wan_auth_x.value != ""){
		document.form.wan_pppoe_username.value = pppoe_username;
		document.form.wan_pppoe_passwd.value = pppoe_password;
	}

	if(dnspriv_support){
		inputCtrl(document.form.dnspriv_enable, 1);
		change_dnspriv_enable(document.form.dnspriv_enable.value);
		var dhcp_dns1 = '<% nvram_get("dhcp_dns1_x"); %>';
		var dhcp_dns2 = '<% nvram_get("dhcp_dns2_x"); %>';
		var lan_ipaddr = '<% nvram_get("lan_ipaddr"); %>';

		if (dnsfilter_support && dnsfilter_enable == 1)
			document.getElementById("dnsfilter_hint_dnspriv").style.display = "";

		if ((dhcp_dns1 != "" && dhcp_dns1 != lan_ipaddr) 
			|| (dhcp_dns2 != "" && dhcp_dns2 != lan_ipaddr)
		){

			$("#dhcpdns_hint_dnspriv").html("<span><#DHCPdns_Hint_Dnspriv#></span>");
			$("#dhcpdns_hint_dnspriv a").attr("href", "Advanced_DHCP_Content.asp")
							.css("color", "#FFCC00")
							.css("text-decoration", "underline");
			$("#dhcpdns_hint_dnspriv").show();	
		}

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
/*
			$.getJSON("http://nw-dlcdnet.asus.com/plugin/js/dot-servers.json",
				function(cloud_data){
					if(JSON.stringify(local_data) != JSON.stringify(cloud_data)){
						if(Object.keys(cloud_data).length > 0){
							gen_dotPresets(cloud_data);
						}
					}
				}
			);
*/
		});
	}
	else{
		inputCtrl(document.form.dnspriv_enable, 0);
		change_dnspriv_enable(0);
	}

	if(dnssec_support){
		document.getElementById("dnssec_tr").style.display = "";
		showhide("dnssec_strict_tr", "<% nvram_get("dnssec_enable"); %>" == "1" ? 1 : 0);
	}

	if(dnssec_support){
		document.getElementById("dnssec_tr").style.display = "";
		showhide("dnssec_strict_tr", "<% nvram_get("dnssec_enable"); %>" == "1" ? 1 : 0);
	}

	change_nat(<% nvram_get("wan_nat_x"); %>);

	if(yadns_support){
		if(yadns_enable != 0 && yadns_mode != -1){
			document.getElementById("yadns_hint").style.display = "";
			document.getElementById("yadns_hint").innerHTML = "<span><#YandexDNS_settings_hint#></span>";
			if(dnspriv_support){
				document.getElementById("yadns_hint_dnspriv").style.display = "";
				document.getElementById("yadns_hint_dnspriv").innerHTML = "<span><#YandexDNS_settings_hint#></span>";
			}
		}
	}
	display_upnp_options();

	if(gobi_support){
		document.getElementById("page_title").innerHTML = "<#WAN_page_desc#>";
		document.getElementById("wan_inf_th").innerHTML = "<#WAN_Interface_Title#>";
	}

	if(dsl_support)      //MODELDEP: DSL-AC68U,DSL-AC68R,DSL-AX82U
		showhide("dot1q_setting",1);
	else
		showhide("dot1q_setting",0);

	if(productid == "BRT-AC828" || productid == "RT-AD7200"){      //MODELDEP: BRT-AC828, RT-AD7200
		var wan_type_name = wans_dualwan.split(" ")[<% nvram_get("wan_unit"); %>].toUpperCase();
		if((original_switch_wantag == "none" && original_switch_stb_x != "0") ||
		   (original_switch_wantag != "none") || (wan_type_name != "WAN" && wan_type_name != "WAN2")){
			document.form.wan_dot1q.value = "0";
			showhide("wan_dot1q_setting",0);
		}else{
			showhide("wan_dot1q_setting",1);
		}
	}else{
		document.form.wan_dot1q.value = "0";
		showhide("wan_dot1q_setting",0);
	}

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
	}

	if(wan_bonding_support){
		document.getElementById("wanAgg_faq").href=faq_href3;
	}

	$.getJSON("/ajax/dns_db.json",
		function(data){
			var dns_db_translation_mapping = [
				{tag:"#ADGUARD_1",text:"<#IPConnection_x_DNS_DB_ADGUARD_1#>"},
				{tag:"#ADGUARD_2",text:"<#IPConnection_x_DNS_DB_ADGUARD_2#>"},
				{tag:"#CLOUDFLARE_1",text:"<#IPConnection_x_DNS_DB_CLOUDFLARE_1#>"},
				{tag:"#CLOUDFLARE_2",text:"<#IPConnection_x_DNS_DB_CLOUDFLARE_2#>"},
				{tag:"#CLOUDFLARE_3",text:"<#IPConnection_x_DNS_DB_CLOUDFLARE_3#>"}
			];
			Object.keys(data).forEach(function(dns_item) {
				var dns_name = data[dns_item].name;
				var dns_list = data[dns_item].server;
				var dns_desc = data[dns_item].desc;
				var dns_translation = data[dns_item].translation;
				Object.keys(dns_list).forEach(function(idx) {
					var dns_ip = dns_list[idx];
					var $dns_item_bg = $("<a>");
					$dns_item_bg.appendTo($("#dns_server_list1"));
					if(dns_desc != "")
						$dns_item_bg.attr("title", dns_desc);
					if(dns_translation != "") {
						var specific_translation = dns_db_translation_mapping.filter(function(item, index, _array){
							return (item.tag == dns_translation);
						})[0];
						if(specific_translation != undefined)
							$dns_item_bg.attr("title",  specific_translation.text);
					}
					var $dns_item = $("<div>");
					$dns_item.appendTo($dns_item_bg);
					$dns_item.unbind("click");
					$dns_item.click(function(e) {
						e = e || event;
						e.stopPropagation();
						var click_dns_ip = $(this).children("strong").attr("dns_ip");
						var idx = $(this).closest(".dns_server_list_dropdown").attr("id").replace("dns_server_list", "");
						$("input[name='wan_dns" + idx + "_x']").val(click_dns_ip);
						$(".dns_pull_arrow").attr("src","/images/arrow-down.gif");
						$(".dns_server_list_dropdown").hide();
					});
					var $dns_text = $("<strong>");
					$dns_text.appendTo($dns_item);
					$dns_text.html("" + dns_name + " ( " + dns_ip +  " )");
					$dns_text.attr("dns_ip", dns_ip);
				});
			});
			$("#dns_server_list1").children().clone(true).appendTo($("#dns_server_list2"));
			$(".dns_pull_arrow").show();
		}
	);
	$("body").click(function() {
		$(".dns_pull_arrow").attr("src","/images/arrow-down.gif");
		$(".dns_server_list_dropdown").hide();
	});

	if(dslite_support){
		showaftrList();
		if(wan_type != "dslite"){
			update_info(0);
		}
		else{
			update_info(1);
		}
	}
}

function change_notusb_unit(){
	document.wanUnit_form.wan_unit.value = (usb_index+1)%2;
	FormActions("apply.cgi", "change_wan_unit", "", "");
	document.wanUnit_form.target = "";
	document.wanUnit_form.submit();
	location.herf = document.wanUnit_form.current_page.value;
}

function display_upnp_options(){
	document.getElementById("upnp_range_int").style.display = (document.form.wan_upnp_enable[0].checked) ? "" : "none";
	document.getElementById("upnp_range_ext").style.display = (document.form.wan_upnp_enable[0].checked) ? "" : "none";
	document.getElementById("upnp_secure_tr").style.display = (document.form.wan_upnp_enable[0].checked) ? "" : "none";
	if (igd2_support) {
		document.getElementById("upnp_pinhole").style.display = (document.form.wan_upnp_enable[0].checked) ? "" : "none";
	} else {
		document.getElementById("upnp_pinhole").style.display = "none";
	}
}

var dsltmp_transmode = "<% nvram_get("dsltmp_transmode"); %>";
function change_wan_unit(obj){
	if(!dualWAN_support) return;
	
	if(obj.options[obj.selectedIndex].text == "DSL"){
		if(dsltmp_transmode == "atm")
			document.form.current_page.value = "Advanced_DSL_Content.asp";
		else //ptm
			document.form.current_page.value = "Advanced_VDSL_Content.asp";	
	}else if(document.form.dsltmp_transmode){
		document.form.dsltmp_transmode.style.display = "none";
	}
	
	if(obj.options[obj.selectedIndex].text == "USB") {
		document.form.current_page.value = "Advanced_Modem_Content.asp";
	}else if(obj.options[obj.selectedIndex].text == "WAN" 
			|| obj.options[obj.selectedIndex].text == "WAN2"
			|| obj.options[obj.selectedIndex].text == "Ethernet LAN"
			|| obj.options[obj.selectedIndex].text == "Ethernet WAN"){
		if((wans_dualwan == "wan lan" || wans_dualwan == "lan wan")
				 || (wans_dualwan == "wan2 wan" || wans_dualwan == "wan wan2")
				 || (wans_dualwan == "wan2 lan" || wans_dualwan == "lan wan2")){
			if(obj.selectedIndex != wan_unit_flag){
				document.form.wan_unit.value = obj.selectedIndex;
			}
			else{
				return false;
			}
		}
		else{
			return false;
		}
	}
	else if(obj.options[obj.selectedIndex].text == "<#Mobile_title#>"){
		document.form.current_page.value = "Advanced_MobileBroadband_Content.asp";
	}

	FormActions("apply.cgi", "change_wan_unit", "", "");
	document.form.target = "";
	document.form.submit();
}

function get_special_lan_name(wans_lanport_v){
	var special_lan_name = "";

	$.each(eth_wan_list, function(key) {
		var wan_obj = eth_wan_list[key];

		if(wan_obj.hasOwnProperty("wans_lanport")){
			if(wan_obj["wans_lanport"] == wans_lanport_v){
				special_lan_name = wan_obj.wan_name;
				return false;
			}
		}
		else if(wan_obj.hasOwnProperty("extra_settings")){
			var extra_settings = wan_obj.extra_settings;
			if(extra_settings.hasOwnProperty("wan_ifname_x")){
				if(extra_settings["wan_ifname_x"].substr(3, 1) == wans_lanport_v){
					special_lan_name = wan_obj.wan_name;
					return false;
				}
			}
		}
	});

	return special_lan_name;
}

function get_ethwan_name(){
	var wan_name = "";

	$.each(eth_wan_list, function(key) {
		var wan_obj = eth_wan_list[key];
		var setting_matched = true;

		if(wan_obj.hasOwnProperty("extra_settings")){
			var extra_settings = wan_obj.extra_settings;
			$.each(extra_settings, function(key2) {
				var value_x = extra_settings[key2];
				if(httpApi.nvramGet([key2], true)[key2] != value_x){
					setting_matched = false;
					return false;
				}
			});
		}

		if(setting_matched){
			wan_name = wan_obj.wan_name;
			return false;
		}
	});

	return wan_name;
}

function genWANSoption(){
	for(i=0; i<wans_dualwan.split(" ").length; i++){
		var wans_dualwan_NAME = wans_dualwan.split(" ")[i].toUpperCase();
		//MODELDEP: DSL-N55U, DSL-N55U-B, DSL-AC68U, DSL-AC68R
		if(wans_dualwan_NAME == "LAN" &&
				(productid == "DSL-N55U" || productid == "DSL-N55U-B" || productid == "DSL-AC68U" || productid == "DSL-AC68R"))
			wans_dualwan_NAME = "Ethernet WAN";
		else if(wans_dualwan_NAME == "LAN"){
			if((productid == "GT-AX6000" || productid == "RT-AX88U_PRO") && wans_lanport == "5"){
				wans_dualwan_NAME = "2.5G WAN";
			}
			else{
				var special_lan_name = get_special_lan_name(wans_lanport);
				if(special_lan_name != "")
					wans_dualwan_NAME = special_lan_name
				else
					wans_dualwan_NAME = "Ethernet LAN";
			}
		}
		else if(wans_dualwan_NAME == "WAN"){
			if((productid == "GT-AX6000" || productid == "RT-AX88U_PRO") && wans_extwan == "1")
				wans_dualwan_NAME = "2.5G WAN";
			else{
				var ethwan_name = get_ethwan_name();
				if(ethwan_name != "")
					wans_dualwan_NAME = ethwan_name;
			}

		}
		else if(wans_dualwan_NAME == "USB" && (based_modelid == "4G-AC53U" || based_modelid == "4G-AC55U" || based_modelid == "4G-AC68U"))
			wans_dualwan_NAME = "<#Mobile_title#>";
		document.form.wan_unit.options[i] = new Option(wans_dualwan_NAME, i);

		if(based_modelid == "GT-AXY16000" || based_modelid == "RT-AX89U"){
			if(wans_dualwan_NAME == "WAN2")
				document.form.wan_unit.options[i] = new Option("10G base-T", i);
			else if(wans_dualwan_NAME == "SFP+")
				document.form.wan_unit.options[i] = new Option("10G SFP+", i);
		}
	}

	document.form.wan_unit.selectedIndex = '<% nvram_get("wan_unit"); %>';
	if(wans_dualwan.search(" ") < 0 || wans_dualwan.split(" ")[1] == 'none' || !dualWAN_support)
		document.getElementById("WANscap").style.display = "none";
}

var reboot_confirm=0;
function applyRule(){
	if(ctf.dhcpToPppoe() && ctf.getLevel() == 2){
		if(confirm("<#ctf_confirm#>")){
			document.form.ctf_disable_force.value = 0;
			document.form.ctf_fa_mode.value = 0;
			reboot_confirm=1;
		}
		else{				
			return false;
		}
	}
	
	if(productid != "DSL-AC68U" && productid != "DSL-AC68R"){      //MODELDEP: DSL-AC68U,DSL-AC68R
		document.form.ewan_dot1q[0].disabled = true;
		document.form.ewan_dot1q[1].disabled = true;
		document.form.ewan_vid.disabled = true;
		document.form.ewan_dot1p.disabled = true;
	}

	if(productid == "BRT-AC828" || productid == "RT-AD7200"){	//MODELDEP: BRT-AC828,RT-AD7200
		if(original_wan_dot1q != document.form.wan_dot1q.value || original_wan_vid != document.form.wan_vid.value){
			reboot_confirm=1;
		}
	}else{
		document.form.wan_dot1q[0].disabled = true;
		document.form.wan_dot1q[1].disabled = true;
		document.form.wan_vid.disabled = true;
	}

	if(validForm()){
		
		inputCtrl(document.form.wan_dhcpenable_x[0], 1);
		inputCtrl(document.form.wan_dhcpenable_x[1], 1);
		if(!document.form.wan_dhcpenable_x[0].checked){
			inputCtrl(document.form.wan_ipaddr_x, 1);
			inputCtrl(document.form.wan_netmask_x, 1);
			inputCtrl(document.form.wan_gateway_x, 1);
			if(Softwire46_support){
				inputCtrl(document.form.wan_s46_peer_x, 1);
				inputCtrl(document.form.wan_s46_prefix4_x, 1);
				inputCtrl(document.form.wan_s46_prefix4len_x, 1);
				inputCtrl(document.form.wan_s46_prefix6_x, 1);
				inputCtrl(document.form.wan_s46_prefix6len_x, 1);
				inputCtrl(document.form.wan_s46_ealen_x, 1);
				inputCtrl(document.form.wan_s46_offset_x, 1);
				inputCtrl(document.form.wan_s46_psidlen_x, 1);
				inputCtrl(document.form.wan_s46_psid_x, 1);
			}
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

		if(dnspriv_support){
			if(document.form.dnspriv_enable.value == 1){
				var dnspriv_rulelist_value = "";
				for(k=0; k<document.getElementById('dnspriv_rulelist_table').rows.length; k++){
					for(j=0; j<document.getElementById('dnspriv_rulelist_table').rows[k].cells.length-1; j++){
						if(j == 0)
							dnspriv_rulelist_value += "<";
						else
							dnspriv_rulelist_value += ">";
						if (document.getElementById('dnspriv_rulelist_table').rows[k].cells[j].innerHTML.lastIndexOf("...") < 0)
							dnspriv_rulelist_value += document.getElementById('dnspriv_rulelist_table').rows[k].cells[j].innerHTML;
						else
							dnspriv_rulelist_value += document.getElementById('dnspriv_rulelist_table').rows[k].cells[j].title;
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

		if(reboot_confirm==1){

			if(confirm("<#AiMesh_Node_Reboot#>")){
				if((wan_proto_orig != "v6plus" && document.form.wan_proto.value == "v6plus") ||
					(wan_proto_orig != "ocnvc" && document.form.wan_proto.value == "ocnvc"))
					s46reset();

				FormActions("start_apply.htm", "apply", "reboot", "<% get_default_reboot_time(); %>");
				showLoading();
				document.form.submit();
			}
		}
		else{
			if((wan_proto_orig != "v6plus" && document.form.wan_proto.value == "v6plus") ||
				(wan_proto_orig != "ocnvc" && document.form.wan_proto.value == "ocnvc"))
				s46reset();
			showLoading();
			document.form.submit();	
		}
	}
}

var ctf = {
	disable_force: '<% nvram_get("ctf_disable"); %>',
	fa_mode: '<% nvram_get("ctf_fa_mode"); %>',

	dhcpToPppoe: function(){
		if((document.form.wan_proto.value == 'dhcp' || document.form.wan_proto.value == 'static') && 
		   (wan_proto_orig == "pppoe" || wan_proto_orig == "pptp" || wan_proto_orig == "l2tp"))
			return false;
		else if((document.form.wan_proto.value == "pppoe" || document.form.wan_proto.value == "pptp" || document.form.wan_proto.value == "l2tp") && 
		   (wan_proto_orig == 'dhcp' || wan_proto_orig == 'static'))
			return true;

		return false;
	},

	getLevel: function(){
		var curVal;

		if(ctf.disable_force == '0' && ctf.fa_mode == '0')
			curVal = 1;
		else if(ctf.disable_force == '0' && ctf.fa_mode == '2')
			curVal = 2;
		else
			curVal = 0;		

		return curVal;
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
			document.form.wan_ipaddr_x.focus();
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


	if(document.form.wan_dnsenable_x.value == 1 && wan_type == "static"){
		alert("Please set up the DNS server on the client device.");	/* Untranslated */
		Assign_DNS_service();
		return false;
	}
	
	if(dnspriv_support) {
		if(document.form.dnspriv_enable.value == 1 && dnspriv_rulelist_array == ""){
			alert("<#WAN_DNS_Privacy_SelectOne#>");
			return false;
		}
	}

	if(wan_type == "pppoe" || wan_type == "pptp" || wan_type == "l2tp" ||
	   document.form.wan_auth_x.value != ""){
	   	
		if(document.form.wan_pppoe_username.value.length <= 0){
			alert("<#WANJS9Text#>");
			document.form.wan_pppoe_username.focus();
			return false;
		}
		if(!validator.string(document.form.wan_pppoe_username)){
			document.form.wan_pppoe_username.focus();
			return false;
		}
		if(document.form.wan_pppoe_passwd.value.length <= 0){
			alert("<#WANJS9Text#>");
			document.form.wan_pppoe_passwd.focus();
			return false;
		}
		if(!validator.string(document.form.wan_pppoe_passwd)){
			document.form.wan_pppoe_passwd.focus();
			return false;
		}
	}
	
	if(wan_type == "pppoe" || wan_type == "pptp" || wan_type == "l2tp"){
		if(!validator.numberRange(document.form.wan_pppoe_idletime, 0, 4294967295))
			return false;
	}
	
	if(wan_type == "pppoe"){
		if(!validator.numberRange(document.form.wan_pppoe_mtu, 576, 1500)
				|| !validator.numberRange(document.form.wan_pppoe_mru, 576, 1500))
			return false;
		
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

        if((document.form.wan_proto.value == "dhcp")
		|| (document.form.wan_proto.value == "static")){
			if(document.form.wan_mtu.value != "" &&
			   !validator.numberRange(document.form.wan_mtu, 576, 9000))
				return false;
	}

	if(productid == "DSL-AC68U" || productid == "DSL-AC68R"){      //MODELDEP: DSL-AC68U,DSL-AC68R
		if(document.form.ewan_dot1q.value == 1) {
			if(!validator.range(document.form.ewan_vid, 1, 4094)) {
				document.form.ewan_vid.focus();
				return false;
			}
			if((document.form.ewan_vid.value >= 1 && document.form.ewan_vid.value <= 3) ||
				(document.form.ewan_vid.value >= 3880 && document.form.ewan_vid.value <= 3887)){
				alert("VLAN ID " + document.form.ewan_vid.value + " is reserved for internal usage. Please change to another one."); /* untranslated */
				document.form.ewan_vid.focus();
				return false;
			}
			if(!validator.range(document.form.ewan_dot1p, 0, 7)) {
				document.form.ewan_dot1p.focus();
				return false;
			}
		}
	}
	
	if(document.form.wan_hostname.value.length > 0){
		var alert_str = validator.domainName(document.form.wan_hostname);
	
		if(alert_str != ""){
			showtext(document.getElementById("alert_msg1"), alert_str);
			document.getElementById("alert_msg1").style.display = "";
			document.form.wan_hostname.focus();
			document.form.wan_hostname.select();
			return false;
		}else{
			document.getElementById("alert_msg1").style.display = "none";
  	}

		document.form.wan_hostname.value = trim(document.form.wan_hostname.value);
	}	
	
	if(document.form.wan_hwaddr_x.value.length > 0)
			if(!check_macaddr(document.form.wan_hwaddr_x,check_hwaddr_flag(document.form.wan_hwaddr_x,'inner'))){
					document.form.wan_hwaddr_x.select();
					document.form.wan_hwaddr_x.focus();
		 	return false;
			}		 	

	if (document.form.wan_upnp_enable[0].checked) {
		if((!validator.numberRange(document.form.upnp_min_port_int, 1, 65535))
			|| (!validator.numberRange(document.form.upnp_max_port_int, 1, 65535))
			|| (!validator.numberRange(document.form.upnp_min_port_ext, 1, 65535))
			|| (!validator.numberRange(document.form.upnp_max_port_ext, 1, 65535))) {
				return false;
		}
		if((parseInt(document.form.upnp_max_port_int.value) < parseInt(document.form.upnp_min_port_int.value))
			|| (parseInt(document.form.upnp_max_port_ext.value) < parseInt(document.form.upnp_min_port_ext.value))) {
				alert("Invalid UPNP ports!  First port must be lower than last port value.");
	                        return false;
		}
	}
	
	if(document.form.wan_heartbeat_x.value.length > 0)
		 if(!validator.string(document.form.wan_heartbeat_x))
		 	return false;


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

function change_wan_type(wan_type, flag){
	if(typeof(flag) != "undefined")
		change_wan_dhcp_enable(flag);
	else
		change_wan_dhcp_enable(1);
	
	if(wan_type == "pppoe"){
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
		inputCtrl(document.form.wan_pppoe_hostuniq, 1);
		inputCtrl(document.form.wan_dhcp_qry, 0);
		inputCtrl(document.form.wan_mtu, 0);
		
		// 2008.03 James. patch for Oleg's patch. {
		inputCtrl(document.form.wan_pppoe_options_x, 1);
		inputCtrl(document.form.wan_pptp_options_x, 0);
		// 2008.03 James. patch for Oleg's patch. }
		inputCtrl(document.form.wan_heartbeat_x, 0);
		document.getElementById("vpn_dhcp").style.display = "";
		inputCtrl(document.form.wan_ppp_echo, 1);
		ppp_echo_control();

		$("#s46reset_div").hide();
		$("#dslite_setting").hide();

		if(wan_bonding_support){
			inputCtrl(document.form.bond_wan_radio[0], 0);
			inputCtrl(document.form.bond_wan_radio[1], 0);
			document.form.bond_wan_radio.value = "0";
		}
	}
	else if(wan_type == "pptp"){
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
		inputCtrl(document.form.wan_dhcp_qry, 0);
		inputCtrl(document.form.wan_mtu, 0);
		
		// 2008.03 James. patch for Oleg's patch. {
		inputCtrl(document.form.wan_pppoe_options_x, 1);
		inputCtrl(document.form.wan_pptp_options_x, 1);
		// 2008.03 James. patch for Oleg's patch. }
		inputCtrl(document.form.wan_heartbeat_x, 1);
		document.getElementById("vpn_dhcp").style.display = "none";
		inputCtrl(document.form.wan_ppp_echo, 1);
		ppp_echo_control();

		$("#s46reset_div").hide();
		$("#dslite_setting").hide();

		if(wan_bonding_support){
			inputCtrl(document.form.bond_wan_radio[0], 0);
			inputCtrl(document.form.bond_wan_radio[1], 0);
			document.form.bond_wan_radio.value = "0";
		}
	}
	else if(wan_type == "l2tp"){
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
		inputCtrl(document.form.wan_dhcp_qry, 0);
		inputCtrl(document.form.wan_mtu, 0);
		
		// 2008.03 James. patch for Oleg's patch. {
		inputCtrl(document.form.wan_pppoe_options_x, 1);
		inputCtrl(document.form.wan_pptp_options_x, 0);
		// 2008.03 James. patch for Oleg's patch. }
		inputCtrl(document.form.wan_heartbeat_x, 1);
		document.getElementById("vpn_dhcp").style.display = "none";
		inputCtrl(document.form.wan_ppp_echo, 1);
		ppp_echo_control();

		$("#s46reset_div").hide();
		$("#dslite_setting").hide();

		if(wan_bonding_support){
			inputCtrl(document.form.bond_wan_radio[0], 0);
			inputCtrl(document.form.bond_wan_radio[1], 0);
			document.form.bond_wan_radio.value = "0";
		}
	}
	else if(wan_type == "static"){
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
		inputCtrl(document.form.wan_pppoe_hostuniq, 0);
		inputCtrl(document.form.wan_dhcp_qry, 0);
		inputCtrl(document.form.wan_mtu, 1);
		
		// 2008.03 James. patch for Oleg's patch. {
		inputCtrl(document.form.wan_pppoe_options_x, 0);
		inputCtrl(document.form.wan_pptp_options_x, 0);
		// 2008.03 James. patch for Oleg's patch. }
		inputCtrl(document.form.wan_heartbeat_x, 0);
		document.getElementById("vpn_dhcp").style.display = "none";
		inputCtrl(document.form.wan_ppp_echo, 0);
		ppp_echo_control(0);

		$("#s46reset_div").hide();
		$("#dslite_setting").hide();

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
	else if(Softwire46_support && (wan_type == "lw4o6" || wan_type == "map-e" || wan_type == "v6plus" || wan_type == "ocnvc" || wan_type == "dslite")){
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
		inputCtrl(document.form.wan_heartbeat_x, 0);
		document.getElementById("vpn_dhcp").style.display = "none";
		inputCtrl(document.form.wan_ppp_echo, 0);
		ppp_echo_control(0);
		if(wan_type == "dslite"){
			$("#dslite_setting").show();
			change_dslite_mode(document.form.wan_s46_dslite_mode.value);
			update_ipv6_s46_b4addr_selector();
		}
		else
			$("#dslite_setting").hide();

		if((wan_type == "v6plus" && wan_proto_orig == "v6plus") || (wan_type == "ocnvc" && wan_proto_orig == "ocnvc"))
			$("#s46reset_div").css("display", "flex");
		else
			$("#s46reset_div").hide();

		if(wan_bonding_support){
			inputCtrl(document.form.bond_wan_radio[0], 0);
			inputCtrl(document.form.bond_wan_radio[1], 0);
			document.form.bond_wan_radio.value = orig_bond_wan;
		}
	}
	else{	// Automatic IP or 802.11 MD or ""		
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
		inputCtrl(document.form.wan_pppoe_hostuniq, 0);
		inputCtrl(document.form.wan_dhcp_qry, 1);
		inputCtrl(document.form.wan_mtu, 1);
		
		// 2008.03 James. patch for Oleg's patch. {
		inputCtrl(document.form.wan_pppoe_options_x, 0);
		inputCtrl(document.form.wan_pptp_options_x, 0);
		// 2008.03 James. patch for Oleg's patch. }
		inputCtrl(document.form.wan_heartbeat_x, 0);
		document.getElementById("vpn_dhcp").style.display = "none";
		inputCtrl(document.form.wan_ppp_echo, 0);
		ppp_echo_control(0);

		$("#s46reset_div").hide();
		$("#dslite_setting").hide();

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
}

function fixed_change_wan_type(wan_type){
	var flag = false;
	
	if(!document.form.wan_dhcpenable_x[0].checked){
		if(document.form.wan_ipaddr_x.value.length == 0)
			document.form.wan_ipaddr_x.focus();
		else if(document.form.wan_netmask_x.value.length == 0)
			document.form.wan_netmask_x.focus();
		else if(document.form.wan_gateway_x.value.length == 0)
			document.form.wan_gateway_x.focus();
		else
			flag = true;
	}else
		flag = true;
	
	if(wan_type == "pppoe"){
		if(wan_type == original_wan_type){
			document.form.wan_ppp_echo.value = original_ppp_echo;
		}
		else{
			document.form.wan_ppp_echo.value = default_ppp_echo;
		}		
	}else if(wan_type == "pptp" || wan_type == "l2tp"){
		if(wan_type == original_wan_type){
			document.form.wan_ppp_echo.value = original_ppp_echo;
		}
		else{
			document.form.wan_ppp_echo.value = default_ppp_echo;
			
		}
	}
	ppp_echo_control();

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
		document.getElementById('S46setting').style.display = "none";
		document.getElementById('PPPsetting').style.display = "";
		inputCtrl(document.form.wan_ipaddr_x, 1);
		inputCtrl(document.form.wan_netmask_x, 1);
		inputCtrl(document.form.wan_gateway_x, 1);
	}
	else if(Softwire46_support && (wan_type == "lw4o6" || wan_type == "map-e" || wan_type == "v6plus" || wan_type == "ocnvc" || wan_type == "dslite")){
		if(flag == 1){
			if(wan_type == original_wan_type){
				document.form.wan_dhcpenable_x[0].checked = original_wan_dhcpenable;
				document.form.wan_dhcpenable_x[1].checked = !original_wan_dhcpenable;
			}
			else{
				document.form.wan_dhcpenable_x[0].checked = 1;
				document.form.wan_dhcpenable_x[1].checked = 0;
			}
			document.form.wan_s46_dhcpenable_x[0].checked = document.form.wan_dhcpenable_x[0].checked;
			document.form.wan_s46_dhcpenable_x[1].checked = document.form.wan_dhcpenable_x[1].checked;
		}else{
			document.form.wan_dhcpenable_x[0].checked = document.form.wan_s46_dhcpenable_x[0].checked;
			document.form.wan_dhcpenable_x[1].checked = document.form.wan_s46_dhcpenable_x[1].checked;
		}
		
		document.getElementById('s46_prefix_hint').style.display = (wan_type == "lw4o6") ? "none" : "";
		document.getElementById('s46_ipaddr_hint').style.display = (wan_type == "lw4o6") ? "" : "none";
		
		document.getElementById('IPsetting').style.display = "none";
		//document.getElementById('S46setting').style.display = "";
		document.getElementById('S46setting').style.display = "none";
		document.getElementById('PPPsetting').style.display = "none";
		
		var wan_dhcpenable = document.form.wan_dhcpenable_x[0].checked;
		
		inputCtrl(document.form.wan_s46_peer_x, !wan_dhcpenable);
		inputCtrl(document.form.wan_s46_prefix4_x, !wan_dhcpenable);
		inputCtrl(document.form.wan_s46_prefix4len_x, !wan_dhcpenable && wan_type != "lw4o6");
		inputCtrl(document.form.wan_s46_prefix6_x, !wan_dhcpenable);
		inputCtrl(document.form.wan_s46_prefix6len_x, !wan_dhcpenable);
		inputCtrl(document.form.wan_s46_ealen_x, !wan_dhcpenable && wan_type != "lw4o6");
		inputCtrl(document.form.wan_s46_offset_x, !wan_dhcpenable);
		inputCtrl(document.form.wan_s46_psidlen_x, !wan_dhcpenable);
		inputCtrl(document.form.wan_s46_psid_x, !wan_dhcpenable);
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
		document.getElementById('S46setting').style.display = "none";
		document.getElementById('dslite_setting').style.display = "none";
		document.getElementById('PPPsetting').style.display = "";
	}
	
}

function showMAC(){
	var tempMAC = "";
	document.form.wan_hwaddr_x.value = login_mac_str();
	document.form.wan_hwaddr_x.focus();
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

	var is_ipv4 = (document.form.dnspriv_server_0.value.indexOf(".") != -1) ? true : false;
	var is_ipv6 = (document.form.dnspriv_server_0.value.indexOf(":") != -1) ? true : false;
	if(!is_ipv4 && !is_ipv6) {
		alert(document.form.dnspriv_server_0.value + "<#JS_validip#>");
		document.form.dnspriv_server_0.focus();
		return false;
	}
	if ((is_ipv4 && !validator.isLegalIP(document.form.dnspriv_server_0)) ||
	    (is_ipv6 && !validator.isLegal_ipv6(document.form.dnspriv_server_0)))
		return false;

	addRow(document.form.dnspriv_server_0, 1);
	addRow(document.form.dnspriv_port_0, 0);
	addRow(document.form.dnspriv_hostname_0, 0);
	addRow(document.form.dnspriv_spkipin_0, 0);
	show_dnspriv_rulelist();
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

			if (document.getElementById('dnspriv_rulelist_table').rows[k].cells[j].innerHTML.lastIndexOf("...") < 0)
				dnspriv_rulelist_value += document.getElementById('dnspriv_rulelist_table').rows[k].cells[j].innerHTML;
			else
				dnspriv_rulelist_value += document.getElementById('dnspriv_rulelist_table').rows[k].cells[j].title;
		}
	}

	dnspriv_rulelist_array = dnspriv_rulelist_value;
	if(dnspriv_rulelist_array == "")
		show_dnspriv_rulelist();
}

function show_dnspriv_rulelist(){
	var dnspriv_rulelist_row = dnspriv_rulelist_array.split('&#60');
	var code = "";
	var overlib_str;

	code +='<table width="100%" border="1" cellspacing="0" cellpadding="4" align="center" class="list_table" id="dnspriv_rulelist_table">';
	if(dnspriv_rulelist_row.length == 1)
		code +='<tr><td style="color:#FFCC00;" colspan="5"><#IPConnection_VSList_Norule#></td></tr>';
	else{
		for(var i = 1; i < dnspriv_rulelist_row.length; i++){
			code +='<tr id="row'+i+'">';
			var dnspriv_rulelist_col = dnspriv_rulelist_row[i].split('&#62');
			var wid=[27, 10, 27, 27];
				for(var j = 0; j < dnspriv_rulelist_col.length; j++){

					if (dnspriv_rulelist_col[j].length > 25) {
						overlib_str = dnspriv_rulelist_col[j];
						dnspriv_rulelist_col[j] = dnspriv_rulelist_col[j].substring(0, 22)+"...";
						code +='<td width="'+wid[j]+'%" title="' + overlib_str + '">'+ dnspriv_rulelist_col[j] +'</td>';
					} else {
						code +='<td width="'+wid[j]+'%">'+ dnspriv_rulelist_col[j] +'</td>';
					}
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

function pullDNSList(_this) {
	event.stopPropagation();
	var idx = $(_this).attr("id").replace("dns_pull_arrow", "");
	$(".dns_pull_arrow:not(#dns_pull_arrow" + idx + ")").attr("src","/images/arrow-down.gif");
	$(".dns_server_list_dropdown:not(#dns_server_list" + idx + ")").hide();
	var $element = $("#dns_server_list" + idx + "");
	var isMenuopen = $element[0].offsetWidth > 0 || $element[0].offsetHeight > 0;
	if(isMenuopen == 0) {
		$(_this).attr("src","/images/arrow-top.gif");
		$element.show();
	}
	else {
		$(_this).attr("src","/images/arrow-down.gif");
		$element.hide();
	}
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

function change_nat(state) {
	if (isSupport("bcm_kf_netfilter") && isSupport("fullcone")) {
		document.getElementById("nat_type_tr").style.display = (state ? "" : "none");
	}
}

function Assign_DNS_service(){

	$("#apply_btn").attr('disabled', true);
	$("#dns_list_Block").remove();

	var divObj = document.createElement("div");
	divObj.setAttribute("id","dns_list_Block");
	divObj.className = "clientlist_viewlist";
	document.body.appendChild(divObj);
	cal_panel_block("dns_list_Block", 0.045);

	create_DNSlist_view();
}

var DNSList_view_hide_flag = false;
function hide_DNSList_view_block() {

	if(DNSList_view_hide_flag)
	{
		closeDNSListView();
		//removeIframeClick("statusframe", hide_DNSList_view_block);
	}
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
	code += "<th class='IE8HACK' width='5%'></th>";
	code += "<th class='IE8HACK' width='15%'><#qis_service#></th>";
	code += "<th class='IE8HACK' width='80%'><#HSDPAConfig_DNSServers_itemname#></th></tr>";

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
		code += "<th class='IE8HACK' width='5%'></th>";
		code += "<th class='IE8HACK' width='15%'><#qis_service#></th>";
		code += "<th class='IE8HACK' width='15%'><#HSDPAConfig_DNSServers_itemname#></th>";
		code += "<th class='IE8HACK' width='65%'><#Description#></th></tr>";

		DNSListTableIndex[j].forEach(function(idx) {
			if(DNSService[idx].confirmed == "Yes"){
				code += "<tr id='tr_"+idx+"_option' height='40px'>";
				code += "<td><input name='DNS_service_opt' id='dns_"+idx+"' type='radio' class='input'></td>";
				code += "<td>"+DNSService[idx].DNSService+"</td>";
				code += "<td style='padding:10px;text-align:left;'>"+DNSService[idx].ServiceIP1+"<br>"+DNSService[idx].ServiceIP2+"</td>";
				if(DNSService[idx].url){
					var description_temp = DNSService[idx].Description+" <a href='"+DNSService[idx].url+"' target='_blank' style='text-decoration:underline;'> <b><#Learn_more#></b></a>";
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
	code += "<th class='IE8HACK' width='5%'></th>";
	code += "<th class='IE8HACK' width='15%'><#qis_service#></th>";
	code += "<th class='IE8HACK' width='80%'><#HSDPAConfig_DNSServers_itemname#></th></tr>";

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
	code += "<input class='button_gen' type='button' onclick='closeDNSListView()' value='<#CTL_Cancel#>'>";
	code += "<input class='button_gen' type='button' onclick='Update_DNS_service()' style='margin-left:15px;' value='<#CTL_onlysave#>'>";
	code += "</div>";

	$("#dns_list_Block").html(code);

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
			console.log($("#edit_wan_dns1_x").val()+" | "+$("#edit_wan_dns2_x").val());
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
		wan_unit: wan_unit_flag,
		wan_dnsenable_x: document.form.wan_dnsenable_x.value,
		wan_dns1_x: document.form.wan_dns1_x.value,
		wan_dns2_x: document.form.wan_dns2_x.value,
		action_mode: "apply",
		rc_service: "restart_wan_dns "+wan_unit_flag
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

	document.getElementById(obj).style.marginLeft = blockmarginLeft+"px";
	document.getElementById(obj).style.marginTop = blockmarginTop+"px";
}

function closeDNSListView() {
	$("#apply_btn").attr('disabled', false);
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
	document.getElementById("pull_arrow").src = "/images/arrow-down.gif";
	document.getElementById('aftr_Block_PC').style.display='none';
	isMenuopen = 0;
}

function pullaftrList(obj){
	if(isMenuopen == 0){		
		obj.src = "/images/arrow-top.gif"
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

</script>
</head>

<body onload="initial();" onunLoad="return unload_body();" class="bg">
<script>
	if(sw_mode == 3){
		alert("<#page_not_support_mode_hint#>");
		location.href = "/";
	}
</script>
<div id="TopBanner"></div>
<div id="Loading" class="popup_bg"></div>
<iframe name="hidden_frame" id="hidden_frame" src="" width="0" height="0" frameborder="0"></iframe>
<form method="post" name="wanUnit_form" action="/apply.cgi" target="hidden_frame">
<input type="hidden" name="current_page" value="Advanced_WAN_Content.asp">
<input type="hidden" name="next_page" value="Advanced_WAN_Content.asp">
<input type="hidden" name="action_mode" value="change_wan_unit">
<input type="hidden" name="action_script" value="">
<input type="hidden" name="action_wait" value="">
<input type="hidden" name="wan_unit" value="">
</form>
<form method="post" name="form" id="ruleForm" action="/start_apply.htm" target="hidden_frame">
<input type="hidden" name="productid" value="<% nvram_get("productid"); %>">
<input type="hidden" name="support_cdma" value="<% nvram_get("support_cdma"); %>">
<input type="hidden" name="current_page" value="Advanced_WAN_Content.asp">
<input type="hidden" name="next_page" value="Advanced_WAN_Content.asp">
<input type="hidden" name="group_id" value="">
<input type="hidden" name="modified" value="0">
<input type="hidden" name="action_mode" value="apply">
<input type="hidden" name="action_script" value="restart_wan_if">
<input type="hidden" name="action_wait" value="5">
<input type="hidden" name="first_time" value="">
<input type="hidden" name="preferred_lang" id="preferred_lang" value="<% nvram_get("preferred_lang"); %>">
<input type="hidden" name="firmver" value="<% nvram_get("firmver"); %>">
<input type="hidden" name="lan_ipaddr" value="<% nvram_get("lan_ipaddr"); %>">
<input type="hidden" name="lan_netmask" value="<% nvram_get("lan_netmask"); %>">
<input type="hidden" name="ctf_fa_mode" value="<% nvram_get("ctf_fa_mode"); %>">
<input type="hidden" name="ctf_disable_force" value="<% nvram_get("ctf_disable_force"); %>">
<input type="hidden" name="wans_dualwan" value="<% nvram_get("wans_dualwan"); %>" disabled>
<input type="hidden" name="bond_wan" value="<% nvram_get("bond_wan"); %>" disabled>
<input type="hidden" name="switch_wantag" value="<% nvram_get("switch_wantag"); %>" disabled>
<input type="hidden" name="switch_stb_x" value="<% nvram_get("switch_stb_x"); %>" disabled>
<input type="hidden" name="lacp_enabled" value="<% nvram_get("lacp_enabled"); %>" disabled>
<input type="hidden" name="wan_clientid_type" value="">
<input type="hidden" name="wan_dnsenable_x" value="<% nvram_get("wan_dnsenable_x"); %>">
<input type="hidden" name="wan_dns1_x" value="<% nvram_get("wan_dns1_x"); %>">
<input type="hidden" name="wan_dns2_x" value="<% nvram_get("wan_dns2_x"); %>">
<input type="hidden" name="dnspriv_rulelist" value="" disabled>
<input type="hidden" name="ipv6_service" value="<% nvram_get("ipv6_service"); %>" disabled>
<input type="hidden" name="wans_mode" value="<% nvram_get("wans_mode"); %>" disabled>
<input type="hidden" name="ipv6_s46_b4addr" value="<% nvram_get("ipv6_s46_b4addr"); %>" disabled>

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
		  			<div style="margin:10px 0 10px 5px;" class="splitLine"></div>
		  			<div id="page_title" class="formfontdesc" style="margin-bottom:0px;"><#Layer3Forwarding_x_ConnectionType_sectiondesc#></div>

						<table id="WANscap" width="100%" border="1" align="center" cellpadding="4" cellspacing="0" bordercolor="#6b8fa3" class="FormTable">
							<thead>
							<tr>
								<td colspan="2"><#wan_index#></td>
							</tr>
							</thead>							
							<tr>
								<th id="wan_inf_th"><#wan_type#></th>
								<td align="left">
									<select class="input_option" name="wan_unit" onchange="change_wan_unit(this);"></select>
								</td>
							</tr>
						</table>
						
						<div id="basic_setting_desc" class="formfontdesc" style="margin-bottom:0px; margin-top: 15px;"><#WAN_Cfg_Ethernet_WAN#></div>
						<table id="t2BC" width="100%" border="1" align="center" cellpadding="4" cellspacing="0" bordercolor="#6b8fa3"  class="FormTable">
						  <thead>
						  <tr>
							<td colspan="2"><#t2BC#></td>
						  </tr>
						  </thead>

							<tr>
								<th><#Layer3Forwarding_x_ConnectionType_itemname#></th>
								<td align="left">
									<div style="display: flex; align-items: center;">
									<select id="wan_proto_menu" class="input_option" name="wan_proto" onchange="change_wan_type(this.value);fixed_change_wan_type(this.value);">
										<option value="dhcp" <% nvram_match("wan_proto", "dhcp", "selected"); %>><#BOP_ctype_title1#></option>
										<option value="static" <% nvram_match("wan_proto", "static", "selected"); %>><#BOP_ctype_title5#></option>
										<option value="pppoe" <% nvram_match("wan_proto", "pppoe", "selected"); %>>PPPoE</option>
										<option value="pptp" <% nvram_match("wan_proto", "pptp", "selected"); %>>PPTP</option>
										<option value="l2tp" <% nvram_match("wan_proto", "l2tp", "selected"); %>>L2TP</option>
										<option value="lw4o6" <% nvram_match("wan_proto", "lw4o6", "selected"); %>>LW 4over6</option>
										<option value="map-e" <% nvram_match("wan_proto", "map-e", "selected"); %>>MAP-E</option>
										<option value="v6plus" <% nvram_match("wan_proto", "v6plus", "selected"); %>><#IPv6_plus#></option>
										<option value="ocnvc" <% nvram_match("wan_proto", "ocnvc", "selected"); %>><#IPv6_ocnvc#></option>
										<option value="dslite" <% nvram_match("wan_proto", "dslite", "selected"); %>>DS-Lite</option>
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
									<input type="radio" name="wan_nat_x" class="input" value="1" onclick="change_nat(1);" <% nvram_match("wan_nat_x", "1", "checked"); %>><#checkbox_Yes#>
									<input type="radio" name="wan_nat_x" class="input" value="0" onclick="change_nat(0);" <% nvram_match("wan_nat_x", "0", "checked"); %>><#checkbox_No#>
								</td>
							</tr>

							<tr id="nat_type_tr" style="display:none;">
								<th><#OpenNAT_TYPE#>&nbsp&nbsp&nbsp&nbsp&nbsp&nbsp<a id="faq_nat" href="" target="_blank" style="font-family:Lucida Console;text-decoration:underline;">FAQ</a></th> 
								<td>
									<select name="nat_type" class="input_option">
										<option value="0" <% nvram_match("nat_type", "0", "selected"); %>><#OpenNAT_TYPE_Symmetric#></option>
										<option value="1" <% nvram_match("nat_type", "1", "selected"); %>><#OpenNAT_TYPE_Fullcone#></option>
									</select>
								</td>
							</tr>

							<tr>
								<th><a class="hintstyle" href="javascript:void(0);" onClick="openHint(7,23);"><#BasicConfig_EnableMediaServer_itemname#></a>&nbsp&nbsp&nbsp&nbsp&nbsp&nbsp<a id="faq" href="" target="_blank" style="font-family:Lucida Console;text-decoration:underline;">UPnP&nbspFAQ</a></th>
								<td>
									<input type="radio" name="wan_upnp_enable" class="input" value="1" onclick="display_upnp_options();return change_common_radio(this, 'LANHostConfig', 'wan_upnp_enable', '1')" <% nvram_match("wan_upnp_enable", "1", "checked"); %>><#checkbox_Yes#>
									<input type="radio" name="wan_upnp_enable" class="input" value="0" onclick="display_upnp_options();return change_common_radio(this, 'LANHostConfig', 'wan_upnp_enable', '0')" <% nvram_match("wan_upnp_enable", "0", "checked"); %>><#checkbox_No#>
								</td>
							</tr>
							<tr id="upnp_pinhole">
								<th><a class="hintstyle" href="javascript:void(0);" onClick="openHint(50,5);">Enable IGDv2 (IPv6 pinhole support)</a></th>
								<td>
									<input type="radio" name="upnp_pinhole_enable" class="input" value="1" onclick="display_upnp_options();" <% nvram_match("upnp_pinhole_enable", "1", "checked"); %>><#checkbox_Yes#>
									<input type="radio" name="upnp_pinhole_enable" class="input" value="0" onclick="display_upnp_options();" <% nvram_match("upnp_pinhole_enable", "0", "checked"); %>><#checkbox_No#>
								</td>
							</tr>
							<tr id="upnp_secure_tr">
								<th><a class="hintstyle" href="javascript:void(0);" onClick="openHint(50,3);">Enable secure UPnP mode</a></th>
								<td>
									<input type="radio" name="upnp_secure" class="input" value="1" <% nvram_match_x("", "upnp_secure", "1", "checked"); %>><#checkbox_Yes#>
									<input type="radio" name="upnp_secure" class="input" value="0" <% nvram_match_x("", "upnp_secure", "0", "checked"); %>><#checkbox_No#>
								</td>
							</tr>
							<tr id="upnp_range_int">
								<th>UPNP: Allowed internal port range</th>
									<td>
										<input type="text" maxlength="5" name="upnp_min_port_int" class="input_6_table" value="<% nvram_get("upnp_min_port_int"); %>" onkeypress="return validator.isNumber(this,event);">
											to
										<input type="text" maxlength="5" name="upnp_max_port_int" class="input_6_table" value="<% nvram_get("upnp_max_port_int"); %>" onkeypress="return validator.isNumber(this,event);">
									</td>
							</tr>
							<tr id="upnp_range_ext">

								<th>UPNP: Allowed external port range</th>
									<td>
										<input type="text" maxlength="5" name="upnp_min_port_ext" class="input_6_table" value="<% nvram_get("upnp_min_port_ext"); %>" onkeypress="return validator.isNumber(this,event);">
											to
										<input type="text" maxlength="5" name="upnp_max_port_ext" class="input_6_table" value="<% nvram_get("upnp_max_port_ext"); %>" onkeypress="return validator.isNumber(this,event);">
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
								<input type="radio" name="ewan_dot1q" class="input" value="1" onclick="change_dsl_dhcp_enable();" <% nvram_match("ewan_dot1q", "1", "checked"); %>><#checkbox_Yes#>
								<input type="radio" name="ewan_dot1q" class="input" value="0" onclick="change_dsl_dhcp_enable();" <% nvram_match("ewan_dot1q", "0", "checked"); %>><#checkbox_No#>
							</td>
						</tr>
						<tr>
							<th>VLAN ID</th>
							<td>
								<input type="text" name="ewan_vid" maxlength="4" class="input_6_table" value="<% nvram_get("ewan_vid"); %>" onKeyPress="return validator.isNumber(this,event);"> ( 1 ~ 4094 )
							</td>
						</tr>
						<tr>
							<th>802.1P</th>
							<td>
								<input type="text" name="ewan_dot1p" maxlength="4" class="input_6_table" value="<% nvram_get("ewan_dot1p"); %>" onKeyPress="return validator.isNumber(this,event);"> ( 0 ~ 7 )
							</td>
						</tr>
						</table>

						<table id="wan_dot1q_setting" width="100%" border="1" align="center" cellpadding="4" cellspacing="0" bordercolor="#6b8fa3" class="FormTable">
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
								<td><input type="text" id="wan_ipaddr_x" name="wan_ipaddr_x" maxlength="15" class="input_15_table" value="<% nvram_get("wan_ipaddr_x"); %>" onKeyPress="return validator.isIPAddr(this, event);" autocorrect="off" autocapitalize="off"></td>
							</tr>
							
							<tr>
								<th><a class="hintstyle" href="javascript:void(0);" onClick="openHint(7,2);"><#IPConnection_x_ExternalSubnetMask_itemname#></a></th>
								<td><input type="text" name="wan_netmask_x" maxlength="15" class="input_15_table" value="<% nvram_get("wan_netmask_x"); %>" onKeyPress="return validator.isIPAddr(this, event);" autocorrect="off" autocapitalize="off"></td>
							</tr>
							
							<tr>
								<th><a class="hintstyle" href="javascript:void(0);" onClick="openHint(7,3);"><#IPConnection_x_ExternalGateway_itemname#></a></th>
								<td><input type="text" id="wan_gateway_x" name="wan_gateway_x" maxlength="15" class="input_15_table" value="<% nvram_get("wan_gateway_x"); %>" onKeyPress="return validator.isIPAddr(this, event);" autocorrect="off" autocapitalize="off"></td>
							</tr>
						</table>

						<table id="dslite_setting" width="100%" border="1" align="center" cellpadding="4" cellspacing="0" bordercolor="#6b8fa3" class="FormTable" style="display:none">
						<thead>
							<tr>
								<td colspan="2"><#IPConnection_ExternalIPAddress_sectionname#></td>
							</tr>
						</thead>
							<tr>
								<th>Configuration Type</th>
								<td>
									<select id="wan_s46_dslite_mode" class="input_option" name="wan_s46_dslite_mode" onchange="change_dslite_mode(this.value)">
										<option value="0" <% nvram_match("wan_s46_dslite_mode", "0", "selected"); %>><#Auto#></option>
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
									<img id="pull_arrow" height="14px;" src="/images/arrow-down.gif" style="position:absolute;*margin-left:-3px;*margin-top:1px;" onclick="pullaftrList(this);" title="<#select_service#>" onmouseover="over_var=1;" onmouseout="over_var=0;">
									<div id="aftr_Block_PC" class="aftr_Block_PC"></div>
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
						<div style="margin:10px 0 5px 5px;" class="splitLine_dns"></div>
						<div class="assign_dns"><#DNS_Assign_desc#></div>
						<div style="margin:-38px 0 5px 0;text-align:right;"><input type="button" class="button_gen" onclick="Assign_DNS_service()" value="<#CTL_assign#>"></div>
					</td>
          		</tr>
			<tr>
					<th><a class="hintstyle" href="javascript:void(0);" onClick="openHint(7,42);">Forward local domain queries to upstream DNS</a></th><!-- untranslated -->
				<td>
					<input type="radio" value="1" name="dns_fwd_local" <% nvram_match("dns_fwd_local", "1", "checked"); %> /><#checkbox_Yes#>
					<input type="radio" value="0" name="dns_fwd_local" <% nvram_match("dns_fwd_local", "0", "checked"); %> /><#checkbox_No#>
				</td>
			</tr>
			<tr>
				<tr>
					<th><a class="hintstyle" href="javascript:void(0);" onClick="openHint(7,43);">Enable DNS Rebind protection</a></th><!-- untranslated -->
				<td>
					<input type="radio" value="1" name="dns_norebind" <% nvram_match("dns_norebind", "1", "checked"); %> /><#checkbox_Yes#>
					<input type="radio" value="0" name="dns_norebind" <% nvram_match("dns_norebind", "0", "checked"); %> /><#checkbox_No#>
				</td>
			</tr>
			<tr id="dnssec_tr" style="display:none;">
				<th><a class="hintstyle" href="javascript:void(0);" onClick="openHint(7,44);">Enable DNSSEC support</a></th><!-- untranslated -->
				<td>
					<input type="radio" value="1" name="dnssec_enable" onclick="showhide('dnssec_strict_tr',1);" <% nvram_match("dnssec_enable", "1", "checked"); %> /><#checkbox_Yes#>
					<input type="radio" value="0" name="dnssec_enable" onclick="showhide('dnssec_strict_tr',0);" <% nvram_match("dnssec_enable", "0", "checked"); %> /><#checkbox_No#>
				</td>
			</tr>
			<tr id="dnssec_strict_tr" style="display:none;">
					<th><a class="hintstyle" href="javascript:void(0);" onClick="openHint(7,45);">Validate unsigned DNSSEC replies</a></th><!-- untranslated -->
				<td>
					<input type="radio" value="1" name="dnssec_check_unsigned_x" <% nvram_match("dnssec_check_unsigned_x", "1", "checked"); %> /><#checkbox_Yes#>
					<input type="radio" value="0" name="dnssec_check_unsigned_x" <% nvram_match("dnssec_check_unsigned_x", "0", "checked"); %> /><#checkbox_No#>
				</td>
			</tr>
			<tr id="dns_priv_override_tr">
				<th><a class="hintstyle" href="javascript:void(0);" onClick="openHint(7,46);">Prevent client auto DoH</a></th><!-- untranslated -->
				<td>
					<select id="dns_priv_override" class="input_option" name="dns_priv_override">
						<option value="0" <% nvram_match("dns_priv_override", "0", "selected"); %>>Auto</option>
						<option value="1" <% nvram_match("dns_priv_override", "1", "selected"); %>>Yes</option>
						<option value="2" <% nvram_match("dns_priv_override", "2", "selected"); %>>No</option>
					</select>
				</td>
			</tr>

			<tr style="display:none">
				<th><a class="hintstyle" href="javascript:void(0);" onClick="openHint(7,35);"><#WAN_DNS_Privacy#></a></th>
				<td align="left">
					<select id="dnspriv_enable" class="input_option" name="dnspriv_enable" onChange="change_dnspriv_enable(this.value);">
					<option value="0" <% nvram_match("dnspriv_enable", "0", "selected"); %>><#wl_securitylevel_0#></option>
					<option value="1" <% nvram_match("dnspriv_enable", "1", "selected"); %>>DNS-over-TLS (DoT)</option>
					<!--option value="2" <% nvram_match("dnspriv_enable", "2", "selected"); %>>DNS-over-HTTPS (DoH)</option-->
					<!--option value="3" <% nvram_match("dnspriv_enable", "3", "selected"); %>>DNS-over-TLS/HTTPS (DoT+DoH)</option-->
					</select>
					<div id="yadns_hint_dnspriv" style="display:none;"></div>
					<div id="dhcpdns_hint_dnspriv" style="display:none;"></div>
					<div id="dnsfilter_hint_dnspriv" style="display:none;"><span><a style="text-decoration:underline; color:#FFCC00;" href="DNSDirector.asp">DNS Director</a> is enabled - anything configured there to something other than No Redirection or Router will bypass DNS Privacy servers.</span></div>
				</td>
			</tr>
			<tr style="display:none">
				<th><a class="hintstyle" href="javascript:void(0);" onClick="openHint(7,36);"><#WAN_DNS_over_TLS#></a></th>
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
					<th><a href="javascript:void(0);" onClick="openHint(7,37);"><div class="table_text"><#IPConnection_ExternalIPAddress_itemname#></div></a></th>
					<th><a href="javascript:void(0);" onClick="openHint(7,38);"><div class="table_text"><#WAN_DNS_over_TLS_server_port#></div></a></th>
					<th><a href="javascript:void(0);" onClick="openHint(7,39);"><div class="table_text"><#WAN_DNS_over_TLS_server_name#></div></a></th>
					<th><a href="javascript:void(0);" onClick="openHint(7,40);"><div class="table_text"><#WAN_DNS_over_TLS_server_SPKI#></div></a></th>
					<th><#list_add_delete#></th>
				</tr>
				<!-- server info -->
				<tr>
					<td width="27%"><input type="text" class="input_18_table" maxlength="64" id="dnspriv_server_0" name="dnspriv_server_0" onKeyPress="" autocorrect="off" autocapitalize="off"></td>
					<td width="10%"><input type="text" class="input_6_table" maxlength="5" name="dnspriv_port_0" onKeyPress="return validator.isNumber(this,event)" autocorrect="off" autocapitalize="off"></td>
					<td width="27%"><input type="text" class="input_20_table" maxlength="64" name="dnspriv_hostname_0" onKeyPress="" autocorrect="off" autocapitalize="off"></td>
					<td width="27%"><input type="text" class="input_20_table" maxlength="64" name="dnspriv_spkipin_0" onKeyPress="" autocorrect="off" autocapitalize="off"></td>
					<td width="9%">
						<div> 
							<input type="button" class="add_btn" onClick="addRow_Group(8);" value="">
						</div>
					</td>
				</tr>
			</table>
			<!-- server block -->
			<div id="dnspriv_rulelist_Block" style="word-break:break-all;"></div>

			<table id="wan_DHCP_opt" width="100%" border="1" align="center" cellpadding="4" cellspacing="0" bordercolor="#6b8fa3"  class="FormTable">
			<thead>
				<tr><td colspan="2"><#ipv6_6rd_dhcp_option#></td></tr>
			</thead>
				<tr>
					<th width="40%"><#DHCPoption_Class#> (<#NetworkTools_option#> 60):</th>
					<td><input type="text" name="wan_vendorid" class="input_25_table" value="<% nvram_get("wan_vendorid"); %>" maxlength="126" autocapitalization="off" autocomplete="off">
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
							<th><a class="hintstyle" href="javascript:void(0);" onClick="openHint(7,29);"><#PPPConnection_Authentication_itemname#></a></th>
							<td align="left">
							    <select class="input_option" name="wan_auth_x" onChange="change_wan_type(document.form.wan_proto.value);">
							    <option value="" <% nvram_match("wan_auth_x", "", "selected"); %>><#wl_securitylevel_0#></option>
							    <option value="8021x-md5" <% nvram_match("wan_auth_x", "8021x-md5", "selected"); %>>802.1x</option>
							    <option value="telenet" <% nvram_match("wan_auth_x", "telenet", "selected"); %>>Кабinet</option>
							    </select></td>
							</tr>
				<tr>
					<th><a class="hintstyle" href="javascript:void(0);" onClick="openHint(7,4);"><#Username#></a></th>
					<td><input type="text" maxlength="64" class="input_32_table" name="wan_pppoe_username" value="" autocomplete="off" onkeypress="return validator.isString(this, event)" autocorrect="off" autocapitalize="off"></td>
				</tr>
				<tr>
					<th><a class="hintstyle" href="javascript:void(0);" onClick="openHint(7,5);"><#PPPConnection_Password_itemname#></a></th>
					<td>
						<input type="password" maxlength="64" class="input_32_table" style="margin-top:2px;" id="wan_pppoe_passwd" name="wan_pppoe_passwd" value="" autocomplete="off" autocorrect="off" autocapitalize="off">
						<div style="margin-top:1px;"><input type="checkbox" name="show_pass_1" onclick="pass_checked(document.form.wan_pppoe_passwd);"><#QIS_show_pass#></div>
					</td>
				</tr>
				<tr style="display:none">
					<th><a class="hintstyle" href="javascript:void(0);" onClick="openHint(7,6);"><#WANPPPAuthText#></a></th>
					<td align="left">
						<select class="input_option" name="wan_pppoe_auth">
						<option value="" <% nvram_match("wan_pppoe_auth", "", "selected"); %>><#Auto#></option>
						<option value="pap" <% nvram_match("wan_pppoe_auth", "pap", "selected"); %>>PAP</option>
						<option value="chap" <% nvram_match("wan_pppoe_auth", "chap", "selected"); %>>CHAP</option>
						</select>
					</td>
				</tr>
				<tr style="display:none">
              	<th><a class="hintstyle" href="javascript:void(0);" onClick="openHint(7,6);"><#PPPConnection_IdleDisconnectTime_itemname#></a></th>
              	<td>
                	<input type="text" maxlength="10" class="input_12_table" name="wan_pppoe_idletime" value="<% nvram_get("wan_pppoe_idletime"); %>" onKeyPress="return validator.isNumber(this,event);" autocorrect="off" autocapitalize="off"/>
                	<input type="checkbox" style="margin-left:30;display:none;" name="wan_pppoe_idletime_check" value="" />
              	</td>
            	</tr>
            	<tr>
              	<th><a class="hintstyle" href="javascript:void(0);" onClick="openHint(7,7);"><#PPPConnection_x_PPPoEMTU_itemname#></a></th>
              	<td><input type="text" maxlength="5" name="wan_pppoe_mtu" class="input_6_table" value="<% nvram_get("wan_pppoe_mtu"); %>" onKeyPress="return validator.isNumber(this,event);" autocorrect="off" autocapitalize="off"/></td>
            	</tr>
            	<tr>
              	<th><a class="hintstyle" href="javascript:void(0);" onClick="openHint(7,8);"><#PPPConnection_x_PPPoEMRU_itemname#></a></th>
              	<td><input type="text" maxlength="5" name="wan_pppoe_mru" class="input_6_table" value="<% nvram_get("wan_pppoe_mru"); %>" onKeyPress="return validator.isNumber(this,event);" autocorrect="off" autocapitalize="off"/></td>
            	</tr>
            	<tr>
              	<th><a class="hintstyle" href="javascript:void(0);" onClick="openHint(7,9);"><#PPPConnection_x_ServiceName_itemname#></a></th>
              	<td><input type="text" maxlength="32" class="input_32_table" name="wan_pppoe_service" value="<% nvram_get("wan_pppoe_service"); %>" onkeypress="return validator.isString(this, event)" autocorrect="off" autocapitalize="off"/></td>
            	</tr>
            	<tr>
              	<th><a class="hintstyle" href="javascript:void(0);" onClick="openHint(7,10);"><#PPPConnection_x_AccessConcentrator_itemname#></a></th>
              	<td><input type="text" maxlength="32" class="input_32_table" name="wan_pppoe_ac" value="<% nvram_get("wan_pppoe_ac"); %>" onkeypress="return validator.isString(this, event)" autocorrect="off" autocapitalize="off"/></td>
            	</tr>
				<tr>
					<th><a class="hintstyle" href="javascript:void(0);" onClick="openHint(7,18);">Host-Uniq (<#Hexadecimal#>)</a></th>
					<td><input type="text" maxlength="256" class="input_32_table" name="wan_pppoe_hostuniq" value="<% nvram_get("wan_pppoe_hostuniq"); %>" onkeypress="return validator.isString(this, event);" autocorrect="off" autocapitalize="off"/></td>
				</tr>
            	<!-- 2008.03 James. patch for Oleg's patch. { -->
		<tr>
		<th><a class="hintstyle" href="javascript:void(0);" onClick="openHint(7,17);"><#PPPConnection_x_PPTPOptions_itemname#></a></th>
		<td>
		<select name="wan_pptp_options_x" class="input_option">
			<option value="" <% nvram_match("wan_pptp_options_x", "","selected"); %>><#Auto#></option>
			<option value="-mppc" <% nvram_match("wan_pptp_options_x", "-mppc","selected"); %>><#No_Encryp#></option>
			<option value="+mppe-40" <% nvram_match("wan_pptp_options_x", "+mppe-40","selected"); %>>MPPE 40</option>
			<!--option value="+mppe-56" <% nvram_match("wan_pptp_options_x", "+mppe-56","selected"); %>>MPPE 56</option-->
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
			<th><a class="hintstyle" href="javascript:void(0);" onClick="openHint(7,34);"><#PPPConnection_x_DNSProbe_Max_Failure#></a></th>
			<td><input type="text" maxlength="6" class="input_6_table" name="dns_delay_round" value="<% nvram_get("dns_delay_round"); %>" onkeypress="return validator.isNumber(this,event);" autocorrect="off" autocapitalize="off"/></td>
		</tr>
		<tr>
			<th><a class="hintstyle" href="javascript:void(0);" onClick="openHint(7,18);"><#PPPConnection_x_AdditionalOptions_itemname#></a></th>
			<td><input type="text" name="wan_pppoe_options_x" value="<% nvram_get("wan_pppoe_options_x"); %>" class="input_32_table" maxlength="255" onKeyPress="return validator.isString(this, event)" onBlur="validator.string(this)" autocorrect="off" autocapitalize="off"></td>
		</tr>
          </table>

      <table width="100%" border="1" align="center" cellpadding="4" cellspacing="0" bordercolor="#6b8fa3"  class="FormTable">
	  	<thead>
		<tr>
            	<td colspan="2"><#PPPConnection_x_HostNameForISP_sectionname#></td>
            	</tr>
		</thead>
		<tr>
          	<th><a class="hintstyle" href="javascript:void(0);" onClick="openHint(7,19);"><#BOP_isp_heart_item#></a></th>
          	<td>
          	<!-- 2008.03 James. patch for Oleg's patch. { -->
          	<input type="text" name="wan_heartbeat_x" class="input_32_table" maxlength="256" value="<% nvram_get("wan_heartbeat_x"); %>" onKeyPress="return validator.isString(this, event)" autocorrect="off" autocapitalize="off"></td>
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
                <th>WAN MTU</th>
                <td><input type="text" maxlength="5" name="wan_mtu" class="input_6_table" value="<% nvram_get("wan_mtu"); %>" onKeyPress="return validator.isNumber(this,event);"/></td>
                </tr>
        	<tr>
          	<th ><a class="hintstyle" href="javascript:void(0);" onClick="openHint(7,16);"><#PPPConnection_x_MacAddressForISP_itemname#></a></th>
				<td>
					<input type="text" name="wan_hwaddr_x" class="input_20_table" maxlength="17" value="<% nvram_get("wan_hwaddr_x"); %>" onKeyPress="return validator.isHWAddr(this,event)" autocorrect="off" autocapitalize="off">
					<input type="button" class="button_gen" onclick="showMAC();" value="<#BOP_isp_MACclone#>">
				</td>
        	</tr>

        <tr>
		<th><a class="hintstyle" href="javascript:void(0);" onClick="openHint(7,30);"><#DHCP_query_freq#></a></th>
		<td>
		<select name="wan_dhcp_qry" class="input_option">
			<option value="0" <% nvram_match(" wan_dhcp_qry", "0","selected"); %>><#DHCPnormal#></option>
			<option value="1" <% nvram_match(" wan_dhcp_qry", "1","selected"); %>><#DHCPaggressive#></option>
			<option value="2" <% nvram_match(" wan_dhcp_qry", "2","selected"); %>><#Continuous_Mode#></option>
		</select>
		</td>
		</tr>

		<tr>
		<tr>
			<th><#Extend_TTL_Value#></th>
				<td>
					<input type="radio" name="ttl_inc_enable" class="input" value="1" <% nvram_match("ttl_inc_enable", "1", "checked"); %>><#checkbox_Yes#>
					<input type="radio" name="ttl_inc_enable" class="input" value="0" <% nvram_match("ttl_inc_enable", "0", "checked"); %>><#checkbox_No#>
				</td>
		</tr>	
		<tr>
			<th><#Spoof_TTL_Value#></th>
				<td>
					<input type="radio" name="ttl_spoof_enable" class="input" value="1" <% nvram_match("ttl_spoof_enable", "1", "checked"); %>><#checkbox_Yes#>
					<input type="radio" name="ttl_spoof_enable" class="input" value="0" <% nvram_match("ttl_spoof_enable", "0", "checked"); %>><#checkbox_No#>
				</td>
		</tr>	
		</table>
		<div class="apply_gen" style="height:auto">
			<input class="button_gen" id="apply_btn" onclick="applyRule();" type="button" value="<#CTL_apply#>"/>
		</div>

                    </td>
                    </tr>

      	  </td>
      	  </tr>
</tbody>
</table>
</td>
</form>
				</tr>
			</table>

		</td>
		<!--===================================Ending of Main Content===========================================-->
    <td width="10" align="center" valign="top">&nbsp;</td>
	</tr>
</table>

<div id="footer"></div>

</body>
</html>
