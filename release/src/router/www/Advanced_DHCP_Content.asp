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

<title><#Web_Title#> - <#menu5_2_2#></title>
<link rel="stylesheet" type="text/css" href="index_style.css"> 
<link rel="stylesheet" type="text/css" href="form_style.css">
<link rel="stylesheet" type="text/css" href="device-map/device-map.css">
<script type="text/javascript" src="/js/jquery.js"></script>
<script type="text/javascript" src="/js/httpApi.js"></script>
<script language="JavaScript" type="text/javascript" src="/state.js"></script>
<script language="JavaScript" type="text/javascript" src="/general.js"></script>
<script language="JavaScript" type="text/javascript" src="/popup.js"></script>
<script type="text/javascript" language="JavaScript" src="/help.js"></script>
<script type="text/javascript" language="JavaScript" src="/validator.js"></script>
<script language="JavaScript" type="text/javascript" src="/client_function.js"></script>
<style>
.sort_border{
	position: relative;
	cursor: pointer;
}
.sort_border:before{
	content: "";
	position: absolute;
	width: 100%;
	left: 0;
	border-top: 1px solid #FC0;
	top: 0;
}
.sort_border.decrease:before{
	bottom: 0;
	top: initial;
}
</style>
<script>
$(function () {
	if(amesh_support && (isSwMode("rt") || isSwMode("ap")) && ameshRouter_support) {
		addNewScript('/require/modules/amesh.js');
	}
});
const dhcp_manual_and_vpnc_policy_attr = function(){
	this.activate = "";
	this.mac = "";
	this.ip = "";
	this.dest_ip = "";
	this.vpnc_idx = "";
	this.dns = "";
	this.hostname = "";
	this.brifname = "";
};
let dhcp_manual_and_vpnc_policy = [];
let dhcp_manual_and_vpnc_policy_ori_data = {};
function init_dhcp_manual_and_vpnc_policy(){
	dhcp_manual_and_vpnc_policy = [];
	const dhcp_staticlist = decodeURIComponent(httpApi.nvramCharToAscii(["dhcp_staticlist"], true).dhcp_staticlist);
	dhcp_manual_and_vpnc_policy_ori_data.dhcp_staticlist = dhcp_staticlist;
	if(dhcp_staticlist != "") {
		const dhcp_staticlist_row = decodeURIComponent(dhcp_staticlist).split("<");
		let i, len = dhcp_staticlist_row.length
		for(i = 1; i < len; i += 1) {
			const dhcp_staticlist_col = dhcp_staticlist_row[i].split('>');
			let profile = new dhcp_manual_and_vpnc_policy_attr();
			profile.mac = dhcp_staticlist_col[0].toUpperCase();
			profile.ip = dhcp_staticlist_col[1];
			profile.dns = (dhcp_staticlist_col[2] == undefined) ? "" : dhcp_staticlist_col[2];
			profile.hostname = (dhcp_staticlist_col[3] == undefined) ? "" : dhcp_staticlist_col[3];
			dhcp_manual_and_vpnc_policy.push(JSON.parse(JSON.stringify(profile)));
		}
	}
	if(isSupport("vpn_fusion")){
		const vpnc_dev_policy_list = decodeURIComponent(httpApi.nvramCharToAscii(["vpnc_dev_policy_list"], true).vpnc_dev_policy_list);
		dhcp_manual_and_vpnc_policy_ori_data.vpnc_dev_policy_list = vpnc_dev_policy_list;
		const each_profile = vpnc_dev_policy_list.split("<");
		$.each(each_profile, function(index, value){
			if(value != ""){
				const profile_data = value.split(">");
				if(profile_data.length >= 4){
					const ip = profile_data[1];
					const specific_profile = dhcp_manual_and_vpnc_policy.find(item => item.ip == ip);
					if((specific_profile != undefined) || (profile_data[4] == "br1" || profile_data[4] == "br2")){
						let profile = (specific_profile == undefined) ? new dhcp_manual_and_vpnc_policy_attr() : specific_profile;
						profile.activate = profile_data[0];
						profile.ip = profile_data[1];
						profile.dest_ip = profile_data[2];
						profile.vpnc_idx = profile_data[3];
						profile.brifname = ((profile_data[4] == undefined) ? "" : profile_data[4]);
						if(specific_profile == undefined)
							dhcp_manual_and_vpnc_policy.push(JSON.parse(JSON.stringify(profile)));
					}
				}
			}
		});
	}
}
if(pptpd_support){
	var pptpd_clients = '<% nvram_get("pptpd_clients"); %>';
	var pptpd_clients_subnet = pptpd_clients.split(".")[0]+"."
				+pptpd_clients.split(".")[1]+"."
				+pptpd_clients.split(".")[2]+".";

	var pptpd_clients_start_ip = parseInt(pptpd_clients.split(".")[3].split("-")[0]);
	var pptpd_clients_end_ip = parseInt(pptpd_clients.split("-")[1]);
}

var dhcp_enable = '<% nvram_get("dhcp_enable_x"); %>';
var pool_start = '<% nvram_get("dhcp_start"); %>';
var pool_end = '<% nvram_get("dhcp_end"); %>';
var pool_subnet = pool_start.split(".")[0]+"."+pool_start.split(".")[1]+"."+pool_start.split(".")[2]+".";
var pool_start_end = parseInt(pool_start.split(".")[3]);
var pool_end_end = parseInt(pool_end.split(".")[3]);

var lan_domain_ori = '<% nvram_get("lan_domain"); %>';
var dhcp_gateway_ori = '<% nvram_get("dhcp_gateway_x"); %>';
var dhcp_dns1_ori = '<% nvram_get("dhcp_dns1_x"); %>';
var dhcp_dns2_ori = '<% nvram_get("dhcp_dns2_x"); %>';
var dhcp_wins_ori = '<% nvram_get("dhcp_wins_x"); %>';

var static_enable = '<% nvram_get("dhcp_static_x"); %>';
var dhcp_staticlists = '<% nvram_get("dhcp_staticlist"); %>';
var staticclist_row = dhcp_staticlists.split('&#60');

if(yadns_support){
	var yadns_enable = '<% nvram_get("yadns_enable_x"); %>';
	var yadns_mode = '<% nvram_get("yadns_mode"); %>';
}
var ipv6_proto_orig = httpApi.nvramGet(["ipv6_service"]).ipv6_service;
var MaxRule_extend_limit = ((isSupport("MaxRule_extend_limit") != "") ? isSupport("MaxRule_extend_limit") : 64);
var manually_dhcp_sort_type = 0;//0:increase, 1:decrease

var faq_href = "https://nw-dlcdnet.asus.com/support/forward.html?model=&type=Faq&lang="+ui_lang+"&kw=&num=101";

vpn_fusion_support = false;

function initial(){
	show_menu();
	document.getElementById("faq").href=faq_href;

	//Viz 2011.10{ for LAN ip in DHCP pool or Static list
	showtext(document.getElementById("LANIP"), '<% nvram_get("lan_ipaddr"); %>');
	if((inet_network(document.form.lan_ipaddr.value)>=inet_network(document.form.dhcp_start.value))&&(inet_network(document.form.lan_ipaddr.value)<=inet_network(document.form.dhcp_end.value))){
			document.getElementById('router_in_pool').style.display="";
	}else if(dhcp_staticlists != ""){
			for(var i = 1; i < staticclist_row.length; i++){
					var static_ip = staticclist_row[i].split('&#62')[1];
					if(static_ip == document.form.lan_ipaddr.value){
								document.getElementById('router_in_pool').style.display="";
  				}
			}
	}
	//}Viz 2011.10
	setTimeout("showdhcp_staticlist();", 100);
	setTimeout("showDropdownClientList('setClientIP', 'mac>ip', 'all', 'ClientList_Block_PC', 'pull_arrow', 'all');", 1000);
	
	if(pptpd_support){	 
		var chk_vpn = check_vpn();
		if(chk_vpn == true){
	 		document.getElementById("VPN_conflict").style.display = "";
	 		document.getElementById("VPN_conflict_span").innerHTML = "<#vpn_conflict_dhcp#>"+pptpd_clients;
		}
	}	

	if(yadns_support){
		if(yadns_enable != 0 && yadns_mode != -1){
			document.getElementById("yadns_hint").style.display = "";
			document.getElementById("yadns_hint").innerHTML = "<span><#YandexDNS_settings_hint#></span>";
		}
	}

	document.form.sip_server.disabled = true;
	document.form.sip_server.parentNode.parentNode.style.display = "none";	

	if(IPv6_support && ipv6_proto_orig != "disabled"){
		document.form.ipv6_dns1_x.disabled = false;
		document.form.ipv6_dns1_x.parentNode.parentNode.style.display = "";
	}
	else{
		document.form.ipv6_dns1_x.disabled = true;
		document.form.ipv6_dns1_x.parentNode.parentNode.style.display = "none";
	}

	if(lyra_hide_support){
		$("#dhcpEnable").hide();
	}
	$("#GWStatic").html("<#LANHostConfig_ManualDHCPList_groupitemdesc#>&nbsp;(<#List_limit#>&nbsp;"+MaxRule_extend_limit+")");

	init_dhcp_manual_and_vpnc_policy();
}

function addRow_Group(){
	if(dhcp_enable != "1")
		document.form.dhcp_enable_x[0].checked = true;	
	if(static_enable != "1")
		document.form.dhcp_static_x[0].checked = true;
		
	const rule_num = dhcp_manual_and_vpnc_policy.filter(item => item.mac !== "" && item.ip !== "").length;
	if(rule_num >= MaxRule_extend_limit){
		alert("<#JS_itemlimit1#> " + MaxRule_extend_limit + " <#JS_itemlimit2#>");
		return false;	
	}			
		
	if(document.form.dhcp_staticmac_x_0.value==""){
		alert("<#JS_fieldblank#>");
		document.form.dhcp_staticmac_x_0.focus();
		document.form.dhcp_staticmac_x_0.select();
		return false;
	}else if(document.form.dhcp_staticip_x_0.value==""){
		alert("<#JS_fieldblank#>");
		document.form.dhcp_staticip_x_0.focus();
		document.form.dhcp_staticip_x_0.select();
		return false;
	}else if(check_macaddr(document.form.dhcp_staticmac_x_0, check_hwaddr_flag(document.form.dhcp_staticmac_x_0, 'inner')) == true &&
		 validator.validIPForm(document.form.dhcp_staticip_x_0,0) == true &&
		 validate_dhcp_range(document.form.dhcp_staticip_x_0) == true){
		
		if(document.form.dhcp_dnsip_x_0.value != "") {
			if(!validator.ipAddrFinal(document.form.dhcp_dnsip_x_0, 'dhcp_dns1_x'))
				return false;
		}
		//match(ip or mac) is not accepted

		const specific_mac = dhcp_manual_and_vpnc_policy.find(item => item.mac == document.form.dhcp_staticmac_x_0.value.toUpperCase());
		const specific_ip = dhcp_manual_and_vpnc_policy.find(item => item.ip == document.form.dhcp_staticip_x_0.value);
		if(specific_mac != undefined){
			alert("<#JS_duplicate#>");
			document.form.dhcp_staticmac_x_0.focus();
			document.form.dhcp_staticmac_x_0.select();
			return false;
		}
		if(specific_ip != undefined){
			alert("<#JS_duplicate#>");
			document.form.dhcp_staticip_x_0.focus();
			document.form.dhcp_staticip_x_0.select();
			return false;
		}

		var alert_str = "";
		if(document.form.dhcp_hostname_x_0.value.length > 0)
			alert_str = validator.host_name(document.form.dhcp_hostname_x_0);
		if(alert_str != ""){
			alert(alert_str);
			document.form.dhcp_hostname_x_0.focus();
			document.form.dhcp_hostname_x_0.select();
			return false;
		}

		let profile = new dhcp_manual_and_vpnc_policy_attr();
		profile.mac = document.form.dhcp_staticmac_x_0.value.toUpperCase();
		profile.ip = document.form.dhcp_staticip_x_0.value.toUpperCase();
		profile.dns = document.form.dhcp_dnsip_x_0.value;
		profile.hostname = document.form.dhcp_hostname_x_0.value;
		dhcp_manual_and_vpnc_policy.push(JSON.parse(JSON.stringify(profile)));

		document.form.dhcp_staticip_x_0.value = "";
		document.form.dhcp_staticmac_x_0.value = "";
		document.form.dhcp_dnsip_x_0.value = "";
		document.form.dhcp_hostname_x_0.value = "";
		showdhcp_staticlist();		
	}else{
		return false;
	}	
}

function del_Row(obj){
	const mac = $(obj).attr("data-mac");
	let del_profile = dhcp_manual_and_vpnc_policy.find(item => item.mac == mac);
	if(vpn_fusion_support) {
		const policy_flag = (dhcp_manual_and_vpnc_policy_ori_data.vpnc_dev_policy_list.indexOf(del_profile.ip) >= 0) ? true : false;
		if(policy_flag){
			if(!confirm(stringSafeGet("<#VPN_Fusion_IP_Binding_Delete_And_Exception_Policy#>")))
				return false;
		}
	}

	dhcp_manual_and_vpnc_policy = dhcp_manual_and_vpnc_policy.filter(item => item.mac != mac);
	$("#dhcp_staticlist_table").find(`[data-component='tr_${mac}']`).remove();
	const dhcp_manual_profile = dhcp_manual_and_vpnc_policy.filter(item => item.mac !== "" && item.ip !== "");
	if(dhcp_manual_profile.length == 0)
		showdhcp_staticlist();
}

function edit_Row(obj){
	const mac = $(obj).attr("data-mac");
	const edit_profile = dhcp_manual_and_vpnc_policy.find(item => item.mac == mac);
	$("#edit_dhcp_manual_profile").find("#edit_dhcp_staticmac").val(edit_profile.mac);
	$("#edit_dhcp_manual_profile").find("#edit_dhcp_staticip").val(edit_profile.ip);
	$("#edit_dhcp_manual_profile").find("#edit_dhcp_dnsip").val(edit_profile.dns);
	$("#edit_dhcp_manual_profile").find("#edit_dhcp_hostname").val(edit_profile.hostname);
	$("#edit_dhcp_manual_profile").attr({"data-edit_mac":edit_profile.mac})
	$("#edit_dhcp_manual_profile").css({"top": ((top.webWrapper) ? "-110px": "40px")}).fadeIn(300);
}

function showdhcp_staticlist(){
	var code = "";
	var clientListEventData = [];
	const dhcp_manual_profile = dhcp_manual_and_vpnc_policy.filter(item => item.mac !== "" && item.ip !== "");
	code += '<table width="100%" cellspacing="0" cellpadding="4" align="center" class="list_table" id="dhcp_staticlist_table">';
	if(dhcp_manual_profile.length == 0)
		code += '<tr><td style="color:#FFCC00;"><#IPConnection_VSList_Norule#></td></tr>';
	else {
		//user icon
		var userIconBase64 = "NoIcon";
		var clientName, deviceType, deviceVender;
		const sortData = dhcp_manual_profile.sort(
			function(a, b){
				if(manually_dhcp_sort_type == 0)
					return inet_network(a.ip) - inet_network(b.ip);
				else if(manually_dhcp_sort_type == 1)
					return inet_network(b.ip) - inet_network(a.ip);
			}
		);
		$.each(sortData, function( index, value ) {
			const clientIP = value.ip;
			const clientMac = value.mac.toUpperCase();
			const clientDNS = (value.dns == "") ? `<#Setting_factorydefault_value#>` : value.dns;
			const clientHostname = value.hostname;
			const clientIconID = "clientIcon_" + clientMac.replace(/\:/g, "");
			if(clientList[clientMac]) {
				clientName = (clientList[clientMac].nickName == "") ? clientList[clientMac].name : clientList[clientMac].nickName;
				deviceType = clientList[clientMac].type;
				deviceVender = clientList[clientMac].vendor;
			}
			else {
				clientName = "New device";
				deviceType = 0;
				deviceVender = "";
			}
			code += `<tr data-component='tr_${clientMac}'><td width="30%" align="center">`;
			code += '<table style="width:100%;"><tr><td style="width:35%;height:56px;border:0px;">';
			if(clientList[clientMac] == undefined) {
				code += '<div id="' + clientIconID + '" class="clientIcon type0"></div>';
			}
			else {
				if(usericon_support) {
					userIconBase64 = getUploadIcon(clientMac.replace(/\:/g, ""));
				}
				if(userIconBase64 != "NoIcon") {
					code += '<div id="' + clientIconID + '" style="text-align:center;"><img class="imgUserIcon_card" src="' + userIconBase64 + '"></div>';
				}
				else if(deviceType != "0" || deviceVender == "") {
					code += '<div id="' + clientIconID + '" class="clientIcon type' + deviceType + '"></div>';
				}
				else if(deviceVender != "" ) {
					var venderIconClassName = getVenderIconClassName(deviceVender.toLowerCase());
					if(venderIconClassName != "" && !downsize_4m_support) {
						code += '<div id="' + clientIconID + '" class="venderIcon ' + venderIconClassName + '"></div>';
					}
					else {
						code += '<div id="' + clientIconID + '" class="clientIcon type' + deviceType + '"></div>';
					}
				}
			}
			code += '</td><td style="width:65%;border:0px;">';
			code += '<div>' + clientName + '</div>';
			code += '<div>' + clientMac + '</div>';
			code += '</td></tr></table>';
			code += '</td>';
			code += '<td width="20%">'+ clientIP +'</td>';
			code += '<td width="20%">'+ clientDNS +'</td>';
			code += '<td width="20%" style="word-break:break-all;">'+ clientHostname +'</td>';
			code += '<td width="10%">';
			const css_max_width = (top.webWrapper) ? "100%" : "66px";
			code += `<div style='display:flex;max-width:${css_max_width};min-width:66px;justify-content:center;'>
					<div class="edit_btn" onclick="edit_Row(this);" data-mac='${clientMac}'></div>
					<div class="remove_btn" onclick="del_Row(this);" data-mac='${clientMac}'></div>
				</div>`;
			code += '</td></tr>';
			if(validator.mac_addr(clientMac))
				clientListEventData.push({"mac" : clientMac, "name" : clientName, "ip" : clientIP, "callBack" : "DHCP"});
		});
	}
	code += '</table>';
	document.getElementById("dhcp_staticlist_Block").innerHTML = code;
	for(var i = 0; i < clientListEventData.length; i += 1) {
		var clientIconID = "clientIcon_" + clientListEventData[i].mac.replace(/\:/g, "");
		var clientIconObj = $("#dhcp_staticlist_Block").children("#dhcp_staticlist_table").find("#" + clientIconID + "")[0];
		var paramData = JSON.parse(JSON.stringify(clientListEventData[i]));
		paramData["obj"] = clientIconObj;
		$("#dhcp_staticlist_Block").children("#dhcp_staticlist_table").find("#" + clientIconID + "").click(paramData, popClientListEditTable);
	}
}

function applyRule(){
	if(validForm()){
		let dhcp_staticlist = "";
		$.each(dhcp_manual_and_vpnc_policy, function(index, item){
			if(item.mac != "" && item.ip != "")
				dhcp_staticlist += "<" + item.mac + ">"  + item.ip + ">" + item.dns + ">" + item.hostname;
		});
		document.form.dhcp_staticlist.value = dhcp_staticlist;
               
		// Only restart the whole network if needed
               
		if ((document.form.dhcp_wins_x.value != dhcp_wins_ori) ||
		    (document.form.dhcp_dns1_x.value != dhcp_dns1_ori) ||
		    (document.form.dhcp_dns2_x.value != dhcp_dns2_ori) ||
		    (document.form.dhcp_gateway_x.value != dhcp_gateway_ori) ||
		    (document.form.lan_domain.value != lan_domain_ori)) {

			document.form.action_script.value = "restart_net_and_phy";
		} else {
			document.form.action_script.value = "restart_dnsmasq";
			document.form.action_wait.value = 5;
		}

		if(vpn_fusion_support) {
			let vpnc_dev_policy_list = "";
			$.each(dhcp_manual_and_vpnc_policy, function(index, item){
				if(item.vpnc_idx != "" || (item.brifname == "br1" || item.brifname == "br2")){
					if(vpnc_dev_policy_list != "") vpnc_dev_policy_list += "<";
					vpnc_dev_policy_list += `${item.activate}>${item.ip}>${item.dest_ip}>${item.vpnc_idx}>${item.brifname}`;
				}
			});
			if(vpnc_dev_policy_list != dhcp_manual_and_vpnc_policy_ori_data.vpnc_dev_policy_list) {
				var action_script_tmp = "restart_vpnc_dev_policy;" + document.form.action_script.value;
				document.form.action_script.value = action_script_tmp;
				document.form.vpnc_dev_policy_list.disabled = false;
				document.form.vpnc_dev_policy_list_tmp.disabled = false;
				document.form.vpnc_dev_policy_list_tmp.value = dhcp_manual_and_vpnc_policy_ori_data.vpnc_dev_policy_list
				document.form.vpnc_dev_policy_list.value = vpnc_dev_policy_list;
			}
		}

		if(based_modelid == "MAP-AC1300" || based_modelid == "MAP-AC2200" || based_modelid == "VZW-AC1300" || based_modelid == "MAP-AC1750")
			alert("By applying new LAN settings, please reboot all Lyras connected to main Lyra manually.");

		if(amesh_support && isSwMode("rt") && ameshRouter_support) {
			var radio_value = (document.form.dhcp_enable_x[0].checked) ? 1 : 0;
			if(!AiMesh_confirm_msg("DHCP_Server", radio_value))
				return false;
		}

		showLoading();
		document.form.submit();
	}
}

function validate_dhcp_range(ip_obj){
	var ip_num = inet_network(ip_obj.value);
	var subnet_head, subnet_end;
	
	if(ip_num <= 0){
		alert(ip_obj.value+" <#JS_validip#>");
		ip_obj.value = "";
		ip_obj.focus();
		ip_obj.select();
		return 0;
	}
	
	subnet_head = getSubnet(document.form.lan_ipaddr.value, document.form.lan_netmask.value, "head");
	subnet_end = getSubnet(document.form.lan_ipaddr.value, document.form.lan_netmask.value, "end");
	
	if(ip_num <= subnet_head || ip_num >= subnet_end){
		alert(ip_obj.value+" <#JS_validip#>");
		ip_obj.value = "";
		ip_obj.focus();
		ip_obj.select();
		return 0;
	}
	
	return 1;
}

function validForm(){	
	var re = new RegExp('^[a-zA-Z0-9][a-zA-Z0-9\.\-]*[a-zA-Z0-9]$','gi');
	if((!re.test(document.form.lan_domain.value) || document.form.lan_domain.value.indexOf("asuscomm.com") > 0) && document.form.lan_domain.value != ""){
      alert("<#JS_validchar#>");                
      document.form.lan_domain.focus();
      document.form.lan_domain.select();
	 	return false;
  }	
	
	if(!validator.ipAddrFinal(document.form.dhcp_gateway_x, 'dhcp_gateway_x') ||
			!validator.ipAddrFinal(document.form.dhcp_dns1_x, 'dhcp_dns1_x') ||
			!validator.ipAddrFinal(document.form.dhcp_wins_x, 'dhcp_wins_x'))
		return false;
		
	if(tmo_support && !validator.ipAddrFinal(document.form.sip_server, 'sip_server'))
		return false;	
	
	if(!validate_dhcp_range(document.form.dhcp_start)
			|| !validate_dhcp_range(document.form.dhcp_end))
		return false;
	
	var dhcp_start_num = inet_network(document.form.dhcp_start.value);
	var dhcp_end_num = inet_network(document.form.dhcp_end.value);
	
	if(dhcp_start_num > dhcp_end_num){
		var tmp = document.form.dhcp_start.value;
		document.form.dhcp_start.value = document.form.dhcp_end.value;
		document.form.dhcp_end.value = tmp;
	}
	
//Viz 2011.10 check if DHCP pool in default pool{
	var default_pool = new Array();
	default_pool =get_default_pool(document.form.lan_ipaddr.value, document.form.lan_netmask.value);
	if((inet_network(document.form.dhcp_start.value) < inet_network(default_pool[0])) || (inet_network(document.form.dhcp_end.value) > inet_network(default_pool[1]))){
			if(confirm("<#JS_DHCP3#>")){ //Acceptable DHCP ip pool : "+default_pool[0]+"~"+default_pool[1]+"\n
				document.form.dhcp_start.value=default_pool[0];
				document.form.dhcp_end.value=default_pool[1];
			}else{return false;}
	}
//} Viz 2011.10 check if DHCP pool in default pool
	
	
	if(!validator.range(document.form.dhcp_lease, 120, 604800))
		return false;
	
      	//Filtering ip address with leading zero
	document.form.dhcp_start.value = ipFilterZero(document.form.dhcp_start.value);
        document.form.dhcp_end.value = ipFilterZero(document.form.dhcp_end.value);

	if(IPv6_support && ipv6_proto_orig != "disabled"){
		if(document.form.ipv6_dns1_x.value != ""){
			if(!validator.isLegal_ipv6(document.form.ipv6_dns1_x)) return false;
		}
	}

	return true;
}

function done_validating(action){
	refreshpage();
}

// Viz add 2011.10 default DHCP pool range{
function get_default_pool(ip, netmask){
	// --- get lan_ipaddr post set .xxx  By Viz 2011.10
	z=0;
	tmp_ip=0;
	for(i=0;i<document.form.lan_ipaddr.value.length;i++){
			if (document.form.lan_ipaddr.value.charAt(i) == '.')	z++;
			if (z==3){ tmp_ip=i+1; break;}
	}		
	post_lan_ipaddr = document.form.lan_ipaddr.value.substr(tmp_ip,3);
	// --- get lan_netmask post set .xxx	By Viz 2011.10
	c=0;
	tmp_nm=0;
	for(i=0;i<document.form.lan_netmask.value.length;i++){
			if (document.form.lan_netmask.value.charAt(i) == '.')	c++;
			if (c==3){ tmp_nm=i+1; break;}
	}		
	var post_lan_netmask = document.form.lan_netmask.value.substr(tmp_nm,3);
	
var nm = new Array("0", "128", "192", "224", "240", "248", "252");
	for(i=0;i<nm.length;i++){
				 if(post_lan_netmask==nm[i]){
							gap=256-Number(nm[i]);							
							subnet_set = 256/gap;
							for(j=1;j<=subnet_set;j++){
									if(post_lan_ipaddr < j*gap){
												pool_start=(j-1)*gap+1;
												pool_end=j*gap-2;
												break;						
									}
							}					
																	
							var default_pool_start = subnetPostfix(document.form.dhcp_start.value, pool_start, 3);
							var default_pool_end = subnetPostfix(document.form.dhcp_end.value, pool_end, 3);							
							var default_pool = new Array(default_pool_start, default_pool_end);
							return default_pool;
							break;
				 }
	}	
	//alert(document.form.dhcp_start.value+" , "+document.form.dhcp_end.value);//Viz
}
// } Viz add 2011.10 default DHCP pool range	

//Viz add 2012.02 DHCP client MAC { start

function setClientIP(macaddr, ipaddr){
	document.form.dhcp_staticmac_x_0.value = macaddr;
	document.form.dhcp_staticip_x_0.value = ipaddr;
	hideClients_Block();
}

function hideClients_Block(){
	document.getElementById("pull_arrow").src = "/images/arrow-down.gif";
	document.getElementById('ClientList_Block_PC').style.display='none';
}

function pullLANIPList(obj){
	var element = document.getElementById('ClientList_Block_PC');
	var isMenuopen = element.offsetWidth > 0 || element.offsetHeight > 0;
	if(isMenuopen == 0){		
		obj.src = "/images/arrow-top.gif"
		element.style.display = 'block';		
		document.form.dhcp_staticmac_x_0.focus();		
	}
	else
		hideClients_Block();
}

//Viz add 2012.02 DHCP client MAC } end 
function check_macaddr(obj,flag){ //control hint of input mac address

	if(flag == 1){		
		var childsel=document.createElement("div");
		childsel.setAttribute("id","check_mac");
		childsel.style.color = (top.webWrapper) ? "#ED4444" : "#FFCC00";
		obj.parentNode.appendChild(childsel);
		document.getElementById("check_mac").innerHTML="<#LANHostConfig_ManualDHCPMacaddr_itemdesc#>";		
		document.getElementById("check_mac").style.display = "";
		obj.focus();
		obj.select();
		return false;	
	}else if(flag == 2){
		var childsel=document.createElement("div");
		childsel.setAttribute("id","check_mac");
		childsel.style.color = (top.webWrapper) ? "#ED4444" : "#FFCC00";
		obj.parentNode.appendChild(childsel);
		document.getElementById("check_mac").innerHTML="<#IPConnection_x_illegal_mac#>";
		document.getElementById("check_mac").style.display = "";
		obj.focus();
		obj.select();
		return false;			
	}else{	
		document.getElementById("check_mac") ? document.getElementById("check_mac").style.display="none" : true;
		return true;
	}	
}

function check_vpn(){		//true: (DHCP ip pool & static ip ) conflict with VPN clients

		if(pool_subnet == pptpd_clients_subnet
					&& ((pool_start_end >= pptpd_clients_start_ip && pool_start_end <= pptpd_clients_end_ip)								
								|| (pool_end_end >= pptpd_clients_start_ip && pool_end_end <= pptpd_clients_end_ip)								
								|| (pptpd_clients_start_ip >= pool_start_end && pptpd_clients_start_ip <= pool_end_end)
								|| (pptpd_clients_end_ip >= pool_start_end && pptpd_clients_end_ip <= pool_end_end))
					){
						return true;				
		}
		
		if(dhcp_staticlists != ""){
			for(var i = 1; i < staticclist_row.length; i++){
					var static_subnet ="";
					var static_end ="";					
					var static_ip = staticclist_row[i].split('&#62')[1];
					static_subnet = static_ip.split(".")[0]+"."+static_ip.split(".")[1]+"."+static_ip.split(".")[2]+".";
					static_end = parseInt(static_ip.split(".")[3]);
					if(static_subnet == pptpd_clients_subnet 
  						&& static_end >= pptpd_clients_start_ip 
  						&& static_end <= pptpd_clients_end_ip){
								return true;  							
  				}				
			}
	}

	return false;	
}
function sortClientIP(){
	manually_dhcp_sort_type
	if($(".sort_border").hasClass("decrease")){
		$(".sort_border").removeClass("decrease");
		manually_dhcp_sort_type = 0;
	}
	else{
		$(".sort_border").addClass("decrease");
		manually_dhcp_sort_type = 1;
	}

	showdhcp_staticlist();
}
function edit_dhcp_manual_cancel(){
	$("#edit_dhcp_manual_profile").fadeOut(300);

}
function edit_dhcp_manual_ok(){
	const edit_mac = $("#edit_dhcp_manual_profile").attr("data-edit_mac");
	let edit_profile = dhcp_manual_and_vpnc_policy.find(item => item.mac == edit_mac);
	const not_edit_profile_list = dhcp_manual_and_vpnc_policy.filter(item => item.mac != edit_mac);

	const $dhcpMac = $("#edit_dhcp_staticmac");
	const $dhcpIP = $("#edit_dhcp_staticip");
	const $dhcpDNS = $("#edit_dhcp_dnsip");
	const $dhcpHostName = $("#edit_dhcp_hostname");

	if($dhcpMac.val() == ""){
		alert("<#JS_fieldblank#>");
		$dhcpMac.focus().select();
		return false;
	}

	if($dhcpIP.val() == ""){
		alert("<#JS_fieldblank#>");
		$dhcpIP.focus().select();
		return false;
	}

	if($dhcpDNS.val() !== ""){
		if(!validator.ipAddrFinal($dhcpDNS[0], 'dhcp_dns1_x')){
			return false;
		}
	}

	if($dhcpHostName.val().trim().length > 0){
		const alert_str = validator.host_name($dhcpHostName[0]);
		if(alert_str !== ""){
			alert(alert_str);
			$dhcpHostName.focus().select();
			return false;
		}
	}

	if(check_macaddr($dhcpMac[0], check_hwaddr_flag($dhcpMac[0], 'inner')) == true &&
		 validator.validIPForm($dhcpIP[0],0) == true &&
		 validate_dhcp_range($dhcpIP[0]) == true){

		const specific_mac = not_edit_profile_list.find(item => item.mac == $dhcpMac.val().toUpperCase());
		const specific_ip = not_edit_profile_list.find(item => item.ip == $dhcpIP.val());
		if(specific_mac != undefined){
			alert("<#JS_duplicate#>");
			$dhcpMac.focus();
			$dhcpMac.select();
			return false;
		}
		if(specific_ip != undefined){
			alert("<#JS_duplicate#>");
			$dhcpIP.focus();
			$dhcpIP.select();
			return false;
		}

		edit_profile.mac = $dhcpMac.val().toUpperCase();
		edit_profile.ip = $dhcpIP.val().toUpperCase();
		edit_profile.dns = $dhcpDNS.val();
		edit_profile.hostname = $dhcpHostName.val();

		showdhcp_staticlist();
		edit_dhcp_manual_cancel();
	}
	else{
		return false;
	}
	
}
</script>
</head>

<body onload="initial();" onunLoad="return unload_body();" class="bg">
<div id="TopBanner"></div>

<div id="Loading" class="popup_bg"></div>

<iframe name="hidden_frame" id="hidden_frame" src="" width="0" height="0" frameborder="0"></iframe>

<form method="post" name="form" id="ruleForm" action="/start_apply.htm" target="hidden_frame">
<input type="hidden" name="productid" value="<% nvram_get("productid"); %>">
<input type="hidden" name="current_page" value="Advanced_DHCP_Content.asp">
<input type="hidden" name="next_page" value="Advanced_GWStaticRoute_Content.asp">
<input type="hidden" name="modified" value="0">
<input type="hidden" name="action_mode" value="apply_new">
<input type="hidden" name="action_wait" value="30">
<input type="hidden" name="action_script" value="restart_net_and_phy">
<input type="hidden" name="first_time" value="">
<input type="hidden" name="preferred_lang" id="preferred_lang" value="<% nvram_get("preferred_lang"); %>">
<input type="hidden" name="firmver" value="<% nvram_get("firmver"); %>">
<input type="hidden" name="lan_ipaddr" value="<% nvram_get("lan_ipaddr"); %>">
<input type="hidden" name="lan_netmask" value="<% nvram_get("lan_netmask"); %>">
<input type="hidden" name="dhcp_staticlist" value="">
<input type="hidden" name="vpnc_dev_policy_list" value="" disabled>
<input type="hidden" name="vpnc_dev_policy_list_tmp" value="" disabled>

<table class="content" align="center" cellpadding="0" cellspacing="0">
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
	  <table width="760" border="0" cellpadding="4" cellspacing="0" class="FormTitle" id="FormTitle">		
		<tbody>
		<tr>
		  <td bgcolor="#4D595D" valign="top">
		  <div>&nbsp;</div>
		  <div class="formfonttitle"><#menu5_2#> - <#menu5_2_2#></div>
		  <div style="margin:10px 0 10px 5px;" class="splitLine"></div>
      <div class="formfontdesc"><#LANHostConfig_DHCPServerConfigurable_sectiondesc#></div>
      <div id="router_in_pool" class="formfontdesc" class="hint-color" style="display:none;"><#LANHostConfig_DHCPServerConfigurable_sectiondesc2#><span id="LANIP"></span></div>	
      <div id="VPN_conflict" class="formfontdesc" class="hint-color" style="display:none;"><span id="VPN_conflict_span"></span></div>
			<div class="formfontdesc" style="margin-top:-10px;">
				<a id="faq" href="" target="_blank" style="font-family:Lucida Console;text-decoration:underline;"><#LANHostConfig_ManualDHCPList_groupitemdesc#>&nbsp;FAQ</a>
			</div>
  
			<table width="100%" border="1" align="center" cellpadding="4" cellspacing="0" bordercolor="#6b8fa3" class="FormTable">
			  <thead>
			  <tr>
				<td colspan="2"><#t2BC#></td>
			  </tr>
			  </thead>		

			  <tr id="dhcpEnable">
				<th><a class="hintstyle" href="javascript:void(0);" onClick="openHint(5,1);"><#LANHostConfig_DHCPServerConfigurable_itemname#></a></th>
				<td>
				  <input type="radio" value="1" name="dhcp_enable_x" class="content_input_fd" onClick="return change_common_radio(this, 'LANHostConfig', 'dhcp_enable_x', '1')" <% nvram_match("dhcp_enable_x", "1", "checked"); %>><#checkbox_Yes#>
				  <input type="radio" value="0" name="dhcp_enable_x" class="content_input_fd" onClick="return change_common_radio(this, 'LANHostConfig', 'dhcp_enable_x', '0')" <% nvram_match("dhcp_enable_x", "0", "checked"); %>><#checkbox_No#>
				</td>
			  </tr>
			  
			  <tr>
				<th><a class="hintstyle" href="javascript:void(0);" onClick="openHint(5,2);"><#LANHostConfig_DomainName_itemname#></a></th>
				<td>
				  <input type="text" maxlength="32" class="input_25_table" name="lan_domain" value="<% nvram_get("lan_domain"); %>" autocorrect="off" autocapitalize="off">
				</td>
			  </tr>
			  
			  <tr>
			  <th><a class="hintstyle" href="javascript:void(0);" onClick="openHint(5,3);"><#LANHostConfig_MinAddress_itemname#></a></th>
			  <td>
				<input type="text" maxlength="15" class="input_15_table" name="dhcp_start" value="<% nvram_get("dhcp_start"); %>" onKeyPress="return validator.isIPAddr(this,event);" autocorrect="off" autocapitalize="off">
			  </td>
			  </tr>
			  
			  <tr>
            <th><a class="hintstyle" href="javascript:void(0);" onClick="openHint(5,4);"><#LANHostConfig_MaxAddress_itemname#></a></th>
            <td>
              <input type="text" maxlength="15" class="input_15_table" name="dhcp_end" value="<% nvram_get("dhcp_end"); %>" onKeyPress="return validator.isIPAddr(this,event)" autocorrect="off" autocapitalize="off">
            </td>
			  </tr>
			  
			  <tr>
            <th><a class="hintstyle" href="javascript:void(0);" onClick="openHint(5,5);"><#LANHostConfig_LeaseTime_itemname#></a></th>
            <td>
              <input type="text" maxlength="6" name="dhcp_lease" class="input_15_table" value="<% nvram_get("dhcp_lease"); %>" onKeyPress="return validator.isNumber(this,event)" autocorrect="off" autocapitalize="off">
            </td>
			  </tr>
			  
			  <tr>
            <th><a class="hintstyle" href="javascript:void(0);" onClick="openHint(5,6);"><#IPConnection_x_ExternalGateway_itemname#></a></th>
            <td>
              <input type="text" maxlength="15" class="input_15_table" name="dhcp_gateway_x" value="<% nvram_get("dhcp_gateway_x"); %>" onKeyPress="return validator.isIPAddr(this,event)" autocorrect="off" autocapitalize="off">
            </td>
			  </tr>

			  <tr>
            <th>Sip Server</th>
            <td>
              <input type="text" maxlength="15" class="input_15_table" name="sip_server" value="<% nvram_get("sip_server"); %>" onKeyPress="return validator.isIPAddr(this,event)" autocorrect="off" autocapitalize="off">
            </td>
			  </tr>			  
			</table>
			<table width="100%" border="1" align="center" cellpadding="4" cellspacing="0" bordercolor="#6b8fa3"  class="FormTable" style="margin-top:8px">
			  <thead>
			  <tr>
				<td colspan="2"><#LANHostConfig_x_LDNSServer1_sectionname#></td>
			  </tr>
			  </thead>		
			  
			  <tr>
				<th width="200"><a class="hintstyle" href="javascript:void(0);" onClick="openHint(5,7);"><#LANHostConfig_x_LDNSServer1_itemname#> 1</a></th>
				<td>
				  <input type="text" maxlength="15" class="input_15_table" name="dhcp_dns1_x" value="<% nvram_get("dhcp_dns1_x"); %>" onKeyPress="return validator.isIPAddr(this,event)" autocorrect="off" autocapitalize="off">
				  <div id="yadns_hint" style="display:none;"></div>
				</td>
			  </tr>
			<tr>
				<th width="200"><a class="hintstyle" href="javascript:void(0);" onClick="openHint(5,7);"><#LANHostConfig_x_LDNSServer1_itemname#> 2</a></th>
				<td>
					<input type="text" maxlength="15" class="input_15_table" name="dhcp_dns2_x" value="<% nvram_get("dhcp_dns2_x"); %>" onKeyPress="return validator.isIPAddr(this,event)">
				</td>
			</tr>
			<tr style="display:none;">
				<th width="200"><#ipv6_dns_serv#></th>
				<td>
					<input type="text" maxlength="39" class="input_32_table" name="ipv6_dns1_x" value="<% nvram_get("ipv6_dns1_x"); %>" autocorrect="off" autocapitalize="off">
				</td>
			</tr>

			<tr>
				<th>Advertise router's IP in addition to user-specified DNS</th>
				<td>
					<input type="radio" value="1" name="dhcpd_dns_router" class="content_input_fd" onClick="return change_common_radio(this, 'LANHostConfig', 'dhcpd_dns_router', '1')" <% nvram_match("dhcpd_dns_router", "1", "checked"); %>><#checkbox_Yes#>
					<input type="radio" value="0" name="dhcpd_dns_router" class="content_input_fd" onClick="return change_common_radio(this, 'LANHostConfig', 'dhcpd_dns_router', '0')" <% nvram_match("dhcpd_dns_router", "0", "checked"); %>><#checkbox_No#>
				</td>
			</tr>
			
			  <tr>
				<th><a class="hintstyle" href="javascript:void(0);" onClick="openHint(5,8);"><#LANHostConfig_x_WINSServer_itemname#></a></th>
				<td>
				  <input type="text" maxlength="15" class="input_15_table" name="dhcp_wins_x" value="<% nvram_get("dhcp_wins_x"); %>" onkeypress="return validator.isIPAddr(this,event)" autocorrect="off" autocapitalize="off"/>
				</td>
			  </tr>
			</table>

			<table width="100%" border="1" align="center" cellpadding="4" cellspacing="0" class="FormTable" style="margin-top:8px;" >
		  	<thead>
		  		<tr>
					<td colspan="3"><#LANHostConfig_ManualDHCPEnable_title#></td>
		  		</tr>
		  	</thead>

		  	<tr>
     			<th><a class="hintstyle" href="javascript:void(0);" onClick="openHint(5,9);"><#LANHostConfig_ManualDHCPEnable_itemname#></a></th>
				<td colspan="2" style="text-align:left;">
					<input type="radio" value="1" name="dhcp_static_x"  onclick="return change_common_radio(this, 'LANHostConfig', 'dhcp_static_x', '1')" <% nvram_match("dhcp_static_x", "1", "checked"); %> /><#checkbox_Yes#>
					<input type="radio" value="0" name="dhcp_static_x"  onclick="return change_common_radio(this, 'LANHostConfig', 'dhcp_static_x', '0')" <% nvram_match("dhcp_static_x", "0", "checked"); %> /><#checkbox_No#>
				</td>
			  </tr>
			</table>

			<div style="position: relative;">
				<table width="100%" border="1" align="center" cellpadding="4" cellspacing="0" class="FormTable_table" style="margin-top:8px;position: relative;">
					<thead>
						<tr>
							<td colspan="5" id="GWStatic"></td>
						</tr>
					</thead>
					<tr>
						<th><a class="hintstyle" href="javascript:void(0);" onClick="openHint(5,10);"><#Client_Name#> (<#PPPConnection_x_MacAddressForISP_itemname#>)</a></th>
						<th class="sort_border"  onClick="sortClientIP()"><#IPConnection_ExternalIPAddress_itemname#></th>
						<th><#LANHostConfig_x_LDNSServer1_itemname#> (Optional)</th>
						<th><#LANHostConfig_x_DDNSHostNames_itemname#> (Optional)</th>
						<th><#list_add_delete#></th>
					</tr>
					<tr>
						<!-- client info -->
						<td width="30%">
							<input type="text" class="input_20_table" maxlength="17" name="dhcp_staticmac_x_0" style="margin-left:-20px;width:190px;" onKeyPress="return validator.isHWAddr(this,event)" onClick="hideClients_Block();" autocorrect="off" autocapitalize="off" placeholder="ex: <% nvram_get("lan_hwaddr"); %>">
							<img id="pull_arrow" height="14px;" src="/images/arrow-down.gif" style="position:absolute;*margin-left:-3px;*margin-top:1px;" onclick="pullLANIPList(this);" title="<#select_MAC#>">
							<div id="ClientList_Block_PC" class="clientlist_dropdown" style="margin-left:-1px;"></div>
						</td>
						<td width="20%">
							<input type="text" class="input_15_table" maxlength="15" name="dhcp_staticip_x_0" onkeypress="return validator.isIPAddr(this,event)" autocorrect="off" autocapitalize="off">
						</td>
						<td width="20%">
							<input type="text" class="input_15_table" maxlength="15" name="dhcp_dnsip_x_0" onkeypress="return validator.isIPAddr(this,event)" autocorrect="off" autocapitalize="off">
						</td>
						<td width="20%">
							<input type="text" class="input_15_table" maxlength="32" name="dhcp_hostname_x_0" onkeypress="return validator.isString(this, event)" autocorrect="off" autocapitalize="off">
						</td>
						<td width="10%">
							<div>
								<input type="button" class="add_btn" onClick="addRow_Group();" value="">
							</div>
						</td>
					</tr>
				</table>

				<div id="edit_dhcp_manual_profile"  class="pop_div_bg pop_div_container">
					<table width="95%" border="1" align="center" cellpadding="4" cellspacing="0" class="FormTable" style="margin-top:8px;">
						<thead>
							<tr>
								<td colspan="2"><#VPN_Fusion_Editor#></td>
							</tr>
						</thead>
						<tbody>
							<tr>
								<th width="30%"><#PPPConnection_x_MacAddressForISP_itemname#></th>
								<td>
									<input type="text" class="input_20_table" maxlength="17" id="edit_dhcp_staticmac" onKeyPress="return validator.isHWAddr(this,event)" onClick="hideClients_Block();" autocorrect="off" autocapitalize="off" placeholder="ex: <% nvram_get("lan_hwaddr"); %>">
								</td>
							</tr>
							<tr>
								<th width="30%"><#IPConnection_ExternalIPAddress_itemname#></th>
								<td>
									<input type="text" class="input_20_table" maxlength="15" id="edit_dhcp_staticip" onkeypress="return validator.isIPAddr(this,event)" autocorrect="off" autocapitalize="off">
								</td>
							</tr>
							<tr>
								<th width="30%"><#LANHostConfig_x_LDNSServer1_itemname#> (Optional)</th>
								<td>
									<input type="text" class="input_20_table" maxlength="15" id="edit_dhcp_dnsip" onkeypress="return validator.isIPAddr(this,event)" autocorrect="off" autocapitalize="off">
								</td>
							</tr>
							<tr>
								<th width="30%"><#LANHostConfig_x_DDNSHostNames_itemname#> (Optional)</th>
								<td>
									<input type="text" class="input_20_table" maxlength="32" id="edit_dhcp_hostname" onkeypress="return validator.isString(this, event)" autocorrect="off" autocapitalize="off">
								</td>
							</tr>
						</tbody>
					</table>
					<div style="margin:10px 0;text-align:center;display:flex;justify-content: space-evenly;">
						<input class="button_gen" type="button" onclick="edit_dhcp_manual_cancel();" value="Cancel">
						<input class="button_gen" type="button" onclick="edit_dhcp_manual_ok();" value="OK">
					</div>
				</div>
			</div>
			  <div id="dhcp_staticlist_Block"></div>
        			
        	<!-- manually assigned the DHCP List end-->		
           	<div class="apply_gen">
           		<input type="button" name="button" class="button_gen" onclick="applyRule();" value="<#CTL_apply#>"/>
            	</div>

      	  </td>
		</tr>
		</tbody>
		
	  </table>		
	</td>
</form>
  </tr>
</table>
		<!--===================================Ending of Main Content===========================================-->		
	</td>
	
    <td width="10" align="center" valign="top">&nbsp;</td>
  </tr>
</table>

<div id="footer"></div>
</body>
</html>
