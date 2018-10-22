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
<link rel="stylesheet" type="text/css" href="other.css">
<script type="text/javascript" src="state.js"></script>
<script type="text/javascript" src="general.js"></script>
<script type="text/javascript" src="popup.js"></script>
<script type="text/javascript" src="help.js"></script>
<script type="text/javascript" src="validator.js"></script>
<script type="text/javascript" src="js/jquery.js"></script>
<script type="text/javascript" src="/js/httpApi.js"></script>
<style>
.contentM_connection{
	position:absolute;
	-webkit-border-radius: 5px;
	-moz-border-radius: 5px;
	border-radius: 5px;
	z-index:500;
	background-color:#2B373B;
	margin-left: 34%;
	margin-top: -90px;
	width:650px;
	display:none;
	box-shadow: 3px 3px 10px #000;
}

.addBtn{
 	text-align:right;
 	background-color:#4D595D;
 	width:100%;
 	margin-top:10px;
}
</style>

<script>
var wan_interfaces = "wan lan1 lan2 lan3 lan4 usb";
var wan_interfaces_array = wan_interfaces.split(" ");

<% login_state_hook(); %>

var Wan_Setting = [
			{"interface": "wan", "wan_enable": "0","wan_nat_x": "1", "wan_upnp_enable": "1", "proto": "dhcp", "wan_dhcpenable_x": "1", "dnsenable_x": "1", "pppoe_username": "", "pppoe_passwd": "", "pppoe_idletime": "0", "pppoe_mtu": "1492", "pppoe_mru": "1492", "pppoe_service": "", "pppoe_ac": "", "pppoe_options_x": "", "ipaddr_x": "0.0.0.0", "netmask_x": "0.0.0.0", "gateway_x": "0.0.0.0", "dns1_x": "", "dns2_x": "", "auth_x": "", "wan_heartbeat_x": "", "wan_vpndhcp": "1", "wan_hostname": "", "wan_hwaddr_x": "", "dhcpc_mode": "1", "ttl_inc_enable": "0"}, 
			{"interface": "lan1", "wan_enable": "0","wan_nat_x": "0", "wan_upnp_enable": "0", "proto": "pppoe", "wan_dhcpenable_x": "0", "dnsenable_x": "0", "pppoe_username": "test1", "pppoe_passwd": "1234", "pppoe_idletime": "0", "pppoe_mtu": "1492", "pppoe_mru": "1492", "pppoe_service": "", "pppoe_ac": "", "pppoe_options_x": "", "ipaddr_x": "0.0.0.0", "netmask_x": "0.0.0.0", "gateway_x": "0.0.0.0", "dns1_x": "", "dns2_x": "", "auth_x": "", "wan_heartbeat_x": "", "wan_vpndhcp": "1", "wan_hostname": "", "wan_hwaddr_x": "", "dhcpc_mode": "1", "ttl_inc_enable": "0"}
	];
function initial(){
	show_menu();
	// https://www.asus.com/support/FAQ/1011715/
	httpApi.faqURL("1011715", function(url){document.getElementById("faq").href=url;});	

}

function show_wan_table(){
	var wan_interface;
	var connection_type;
	var ipaddr;
	if(Object.keys(Wan_Setting).length == 0)
		code +='<tr><td style="color:#FFCC00;" colspan="4">No WAN Interface</td></tr>';
	else{
		//Object.keys(Wan_Setting).forEach(function(key){

		//}
	}
}

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
		var ip_num = inet_network(ip_obj.value);

		if(obj_flag == "DNS" && ip_num == -1){ //DNS allows to input nothing
			return true;
		}
		
		if(obj_flag == "GW" && ip_num == -1){ //GW allows to input nothing
			return true;
		}
		
		if(ip_num > A_class_start && ip_num < A_class_end){
		   obj_name.value = ipFilterZero(ip_obj.value);
			return true;
		}
		else if(ip_num > B_class_start && ip_num < B_class_end){
			alert(ip_obj.value+" <#JS_validip#>");
			ip_obj.focus();
			ip_obj.select();
			return false;
		}
		else if(ip_num > C_class_start && ip_num < C_class_end){
			obj_name.value = ipFilterZero(ip_obj.value);
			return true;
		}
		else{
			alert(ip_obj.value+" <#JS_validip#>");
			ip_obj.focus();
			ip_obj.select();
			return false;
		}	
}

function validForm(){
	var wan_type = document.form.wan_proto_now.value;

	if(!document.form.wan_dhcpenable_x_now[0].checked){// Set IP address by userself
		if(!valid_IP(document.form.wan_ipaddr_x_now, "")) return false;  //WAN IP
		if(!valid_IP(document.form.wan_gateway_x_now, "GW"))return false;  //Gateway IP		

		if(document.form.wan_gateway_x_now.value == document.form.wan_ipaddr_x_now.value){
			document.form.wan_ipaddr_x_now.focus();
			alert("<#IPConnection_warning_WANIPEQUALGatewayIP#>");
			return false;
		}
		
		// test if netmask is valid.
		var default_netmask = "";
		var wrong_netmask = 0;
		var netmask_obj = document.form.wan_netmask_x_now;
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
	
	if(document.form.wan_dnsenable_x_now[1].checked == true && document.form.wan_proto_now.value != "dhcp" && document.form.wan_dns1_x_now.value == "" && document.form.wan_dns2_x_now.value == ""){
		document.form.wan_dns1_x_now.focus();
		alert("<#IPConnection_x_DNSServer_blank#>");
		return false;
	}
	
	if(!document.form.wan_dnsenable_x_now[0].checked){
		if(!valid_IP(document.form.wan_dns1_x_now, "DNS")) return false;  //DNS1
		if(!valid_IP(document.form.wan_dns2_x_now, "DNS")) return false;  //DNS2
	}
	
	if(wan_type == "pppoe" || wan_type == "pptp" || wan_type == "l2tp" ||
	   document.form.wan_auth_x_now.value != ""){
		if(!validator.string(document.form.wan_pppoe_username_now)
				|| !validator.string(document.form.wan_pppoe_passwd_now)
				)
			return false;
	}
	
	if(wan_type == "pppoe" || wan_type == "pptp" || wan_type == "l2tp"){
		if(!validator.numberRange(document.form.wan_pppoe_idletime_now, 0, 4294967295))
			return false;
	}
	
	if(wan_type == "pppoe"){
		if(!validator.numberRange(document.form.wan_pppoe_mtu_now, 576, 1492)
				|| !validator.numberRange(document.form.wan_pppoe_mru_now, 576, 1492))
			return false;
		
		if(!validator.string(document.form.wan_pppoe_service_now)
				|| !validator.string(document.form.wan_pppoe_ac_now))
			return false;
	}
	
	if(document.form.wan_hostname_now.value.length > 0){
		var alert_str = validator.hostName(document.form.wan_hostname_now);
	
		if(alert_str != ""){
			showtext(document.getElementById("alert_msg1"), alert_str);
			document.getElementById("alert_msg1").style.display = "";
			document.form.wan_hostname_now.focus();
			document.form.wan_hostname_now.select();
			return false;
		}else{
			document.getElementById("alert_msg1").style.display = "none";
  	}

		document.form.wan_hostname_now.value = trim(document.form.wan_hostname_now.value);
	}	
	
	if(document.form.wan_hwaddr_x_now.value.length > 0)
			if(!check_macaddr(document.form.wan_hwaddr_x_now,check_hwaddr_flag(document.form.wan_hwaddr_x_now))){
					document.form.wan_hwaddr_x_now.select();
					document.form.wan_hwaddr_x_now.focus();
		 	return false;
			}		 	
	
	if(document.form.wan_heartbeat_x_now.value.length > 0)
		 if(!validator.string(document.form.wan_heartbeat_x_now))
		 	return false;
	return true;
}

function applyRule(){

	showLoading();
	document.form.submit();
}

function save_connection_settings(){
	if(validForm()){
		$("#connection_settings_table").fadeOut(300);
		save_settings();
	}
}

function hide_connection_settings(){
	$("#connection_settings_table").fadeOut(300);
}

var curr_index = 0;
var original_wan_proto_now;
var original_wan_dhcpenable_now;
var original_dnsenable_now;
function configure_wans(r){
	var i = r.parentNode.parentNode.rowIndex - 2;
	load_settings(i);
	curr_index = i;
	change_wan_type(document.form.wan_proto_now.value);
	$("#connection_settings_table").fadeIn(300);
}

function load_settings(wan_index){
		document.form.wan_enable_now.value = Wan_Setting[wan_index].wan_enable;
		document.form.wan_nat_x_now.value = Wan_Setting[wan_index].wan_nat_x;
		document.form.wan_upnp_enable_now.value = Wan_Setting[wan_index].wan_upnp_enable;
		original_wan_proto_now = Wan_Setting[wan_index].proto;
		document.form.wan_proto_now.value = Wan_Setting[wan_index].proto;
		original_wan_dhcpenable_now = parseInt(Wan_Setting[wan_index].wan_dhcpenable_x);
		document.form.wan_dhcpenable_x_now.value = Wan_Setting[wan_index].wan_dhcpenable_x;
		original_dnsenable_now = parseInt(Wan_Setting[wan_index].dnsenable_x);
		document.form.wan_dnsenable_x_now.value = Wan_Setting[wan_index].dnsenable_x;	
		document.form.wan_pppoe_username_now.value = Wan_Setting[wan_index].pppoe_username;
		document.form.wan_pppoe_passwd_now.value = Wan_Setting[wan_index].pppoe_passwd;
		document.form.wan_pppoe_idletime_now.value = Wan_Setting[wan_index].pppoe_idletime;
		document.form.wan_pppoe_mtu_now.value = Wan_Setting[wan_index].pppoe_mtu;
		document.form.wan_pppoe_mru_now.value = Wan_Setting[wan_index].pppoe_mru;
		document.form.wan_pppoe_service_now.value = Wan_Setting[wan_index].pppoe_service;
		document.form.wan_pppoe_ac_now.value = Wan_Setting[wan_index].pppoe_ac;
		document.form.wan_pppoe_options_x_now.value = Wan_Setting[wan_index].pppoe_options_x;
		document.form.wan_ipaddr_x_now.value = Wan_Setting[wan_index].ipaddr_x;
		document.form.wan_netmask_x_now.value = Wan_Setting[wan_index].netmask_x;
		document.form.wan_gateway_x_now.value = Wan_Setting[wan_index].gateway_x;
		document.form.wan_dns1_x_now.value = Wan_Setting[wan_index].dns1_x;
		document.form.wan_dns2_x_now.value = Wan_Setting[wan_index].dns2_x;
		document.form.wan_auth_x_now.value = Wan_Setting[wan_index].auth_x;
		document.form.wan_heartbeat_x_now.value = Wan_Setting[wan_index].wan_heartbeat_x;
		document.form.wan_vpndhcp_now.value = Wan_Setting[wan_index].wan_vpndhcp;
		document.form.wan_hostname_now.value = Wan_Setting[wan_index].wan_hostname;
		document.form.wan_hwaddr_x_now.value = Wan_Setting[wan_index].wan_hwaddr_x;
		document.form.dhcpc_mode_now.value = Wan_Setting[wan_index].dhcpc_mode;
		document.form.ttl_inc_enable_now.value = Wan_Setting[wan_index].ttl_inc_enable;
}

function save_settings(){
		Wan_Setting[curr_index].wan_enable = document.form.wan_enable_now.value;
		Wan_Setting[curr_index].wan_nat_x = document.form.wan_nat_x_now.value;
		Wan_Setting[curr_index].wan_upnp_enable = document.form.wan_upnp_enable_now.value;	
		Wan_Setting[curr_index].proto = document.form.wan_proto_now.value;
		Wan_Setting[curr_index].wan_dhcpenable_x = document.form.wan_dhcpenable_x_now.value;
		Wan_Setting[curr_index].dnsenable_x = document.form.wan_dnsenable_x_now.value;
		Wan_Setting[curr_index].pppoe_username = document.form.wan_pppoe_username_now.value;
		Wan_Setting[curr_index].pppoe_passwd = document.form.wan_pppoe_passwd_now.value;
		Wan_Setting[curr_index].pppoe_idletime = document.form.wan_pppoe_idletime_now.value;
		Wan_Setting[curr_index].pppoe_mtu = document.form.wan_pppoe_mtu_now.value;
		Wan_Setting[curr_index].pppoe_mru = document.form.wan_pppoe_mru_now.value;
		Wan_Setting[curr_index].pppoe_service = document.form.wan_pppoe_service_now.value;
		Wan_Setting[curr_index].pppoe_ac = document.form.wan_pppoe_ac_now.value;
		Wan_Setting[curr_index].pppoe_options_x = document.form.wan_pppoe_options_x_now.value;
		Wan_Setting[curr_index].ipaddr_x = document.form.wan_ipaddr_x_now.value;
		Wan_Setting[curr_index].netmask_x = document.form.wan_netmask_x_now.value;
		Wan_Setting[curr_index].gateway_x = document.form.wan_gateway_x_now.value;
		Wan_Setting[curr_index].dns1_x = document.form.wan_dns1_x_now.value;
		Wan_Setting[curr_index].dns2_x = document.form.wan_dns2_x_now.value;
		Wan_Setting[curr_index].auth_x = document.form.wan_auth_x_now.value;
		Wan_Setting[curr_index].wan_heartbeat_x = document.form.wan_heartbeat_x_now.value;
		Wan_Setting[curr_index].wan_vpndhcp = document.form.wan_vpndhcp_now.value;
		Wan_Setting[curr_index].wan_hostname = document.form.wan_hostname_now.value;
		Wan_Setting[curr_index].wan_hwaddr_x = document.form.wan_hwaddr_x_now.value;
		Wan_Setting[curr_index].dhcpc_mode = document.form.dhcpc_mode_now.value;
		Wan_Setting[curr_index].ttl_inc_enable = document.form.ttl_inc_enable_now.value;		
}

function change_wan_type(wan_type){
	if(wan_type == "pppoe"){
		document.getElementById("wan_dhcp_tr").style.display="";
		document.getElementById("dnsenable_tr").style.display = "";		
		if(original_wan_proto_now == document.form.wan_proto_now.value){
			document.form.wan_dhcpenable_x_now[0].checked = original_wan_dhcpenable_now;
			document.form.wan_dhcpenable_x_now[1].checked = !original_wan_dhcpenable_now;
		}
		else{
			document.form.wan_dhcpenable_x_now[0].checked = 1;
			document.form.wan_dhcpenable_x_now[1].checked = 0;
		}
		document.getElementById('IPsetting').style.display = "";		
		
		if(original_wan_proto_now == document.form.wan_proto_now.value){
			document.form.wan_dnsenable_x_now[0].checked = original_dnsenable_now;
			document.form.wan_dnsenable_x_now[1].checked = !original_dnsenable_now;
		}
		else{
			document.form.wan_dnsenable_x_now[0].checked = 1;
			document.form.wan_dnsenable_x_now[1].checked = 0;
		}

		inputCtrl(document.form.wan_auth_x_now, 0);
		inputCtrl(document.form.wan_pppoe_username_now, 1);
		document.getElementById('tr_pppoe_password').style.display = "";
		document.form.wan_pppoe_passwd_now.disabled = false;
		inputCtrl(document.form.wan_pppoe_idletime_now, 1);
		inputCtrl(document.form.wan_pppoe_idletime_check, 1);
		inputCtrl(document.form.wan_pppoe_mtu_now, 1);
		inputCtrl(document.form.wan_pppoe_mru_now, 1);
		inputCtrl(document.form.wan_pppoe_service_now, 1);
		inputCtrl(document.form.wan_pppoe_ac_now, 1);
		
		inputCtrl(document.form.wan_pppoe_options_x_now, 1);
		inputCtrl(document.form.wan_pptp_options_x_now, 0);

		inputCtrl(document.form.wan_heartbeat_x_now, 0);
		document.getElementById("vpn_dhcp").style.display = "";
		inputCtrl(document.form.dhcpc_mode_now, 0);

	}
	else if(wan_type == "pptp" || wan_type == "l2tp"){
		document.getElementById("wan_dhcp_tr").style.display="";
		document.getElementById("dnsenable_tr").style.display = "";		
		if(original_wan_proto_now == document.form.wan_proto_now.value){
			document.form.wan_dhcpenable_x_now[0].checked = original_wan_dhcpenable_now;
			document.form.wan_dhcpenable_x_now[1].checked = !original_wan_dhcpenable_now;
		}
		else{
			document.form.wan_dhcpenable_x_now[0].checked = 0;
			document.form.wan_dhcpenable_x_now[1].checked = 1;
		}
		document.getElementById('IPsetting').style.display = "";

		if(original_wan_proto_now == document.form.wan_proto_now.value){
			document.form.wan_dnsenable_x_now[0].checked = original_dnsenable_now;
			document.form.wan_dnsenable_x_now[1].checked = !original_dnsenable_now;
		}
		else{
			document.form.wan_dnsenable_x_now[0].checked = 0;
			document.form.wan_dnsenable_x_now[1].checked = 1;
		}

		inputCtrl(document.form.wan_auth_x_now, 0);
		inputCtrl(document.form.wan_pppoe_username_now, 1);
		document.getElementById('tr_pppoe_password').style.display = "";
		document.form.wan_pppoe_passwd_now.disabled = false;
		inputCtrl(document.form.wan_pppoe_mtu_now, 0);
		inputCtrl(document.form.wan_pppoe_mru_now, 0);
		inputCtrl(document.form.wan_pppoe_service_now, 0);
		inputCtrl(document.form.wan_pppoe_ac_now, 0);
		inputCtrl(document.form.wan_pppoe_options_x_now, 1);
	
		if(wan_type == "pptp"){
			inputCtrl(document.form.wan_pppoe_idletime_now, 1);
			inputCtrl(document.form.wan_pppoe_idletime_check, 1);
			inputCtrl(document.form.wan_pptp_options_x_now, 1);
		}
		else if(wan_type == "l2tp"){
			inputCtrl(document.form.wan_pppoe_idletime_now, 0);
			inputCtrl(document.form.wan_pppoe_idletime_check, 0);
			inputCtrl(document.form.wan_pptp_options_x_now, 0);
		}

		inputCtrl(document.form.wan_heartbeat_x_now, 1);
		document.getElementById("vpn_dhcp").style.display = "none";
		inputCtrl(document.form.dhcpc_mode_now, 0);
	}
	else if(wan_type == "static"){
		document.getElementById("wan_dhcp_tr").style.display="none";
		document.getElementById("dnsenable_tr").style.display = "none";	

		document.form.wan_dhcpenable_x_now[0].checked = 0;
		document.form.wan_dhcpenable_x_now[1].checked = 1;
		document.getElementById('IPsetting').style.display = "";

		inputCtrl(document.form.wan_auth_x_now, 1);
		inputCtrl(document.form.wan_pppoe_username_now, (document.form.wan_auth_x_now.value != ""));
		document.getElementById('tr_pppoe_password').style.display = (document.form.wan_auth_x_now.value != "") ? "" : "none";
		document.form.wan_pppoe_passwd_now.disabled = (document.form.wan_auth_x_now.value != "") ? false : true;
		inputCtrl(document.form.wan_pppoe_idletime_now, 0);
		inputCtrl(document.form.wan_pppoe_idletime_check, 0);
		inputCtrl(document.form.wan_pppoe_mtu_now, 0);
		inputCtrl(document.form.wan_pppoe_mru_now, 0);
		inputCtrl(document.form.wan_pppoe_service_now, 0);
		inputCtrl(document.form.wan_pppoe_ac_now, 0);
		
		inputCtrl(document.form.wan_pppoe_options_x_now, 0);
		inputCtrl(document.form.wan_pptp_options_x_now, 0);

		document.form.wan_dnsenable_x_now[0].checked = 0;
		document.form.wan_dnsenable_x_now[1].checked = 1;

		inputCtrl(document.form.wan_heartbeat_x_now, 0);
		document.getElementById("vpn_dhcp").style.display = "none";
		inputCtrl(document.form.dhcpc_mode_now, 0);
	}
	else{	// Automatic IP or 802.11 MD or ""
		document.getElementById("IPsetting").style.display="none";
		document.getElementById("dnsenable_tr").style.display = "";

		document.form.wan_dhcpenable_x_now[0].checked = 1;
		document.form.wan_dhcpenable_x_now[1].checked = 0;

		if(original_wan_proto_now == document.form.wan_proto_now.value){
			document.form.wan_dnsenable_x_now[0].checked = original_dnsenable_now;
			document.form.wan_dnsenable_x_now[1].checked = !original_dnsenable_now;
		}
		else{
			document.form.wan_dnsenable_x_now[0].checked = 1;
			document.form.wan_dnsenable_x_now[1].checked = 0;
		}

		inputCtrl(document.form.wan_auth_x_now, 1);	
		
		inputCtrl(document.form.wan_pppoe_username_now, (document.form.wan_auth_x_now.value != ""));
		document.getElementById('tr_pppoe_password').style.display = (document.form.wan_auth_x_now.value != "") ? "" : "none";
		document.form.wan_pppoe_passwd_now.disabled = (document.form.wan_auth_x_now.value != "") ? false : true;
		
		inputCtrl(document.form.wan_pppoe_idletime_now, 0);
		inputCtrl(document.form.wan_pppoe_idletime_check, 0);
		inputCtrl(document.form.wan_pppoe_mtu_now, 0);
		inputCtrl(document.form.wan_pppoe_mru_now, 0);
		inputCtrl(document.form.wan_pppoe_service_now, 0);
		inputCtrl(document.form.wan_pppoe_ac_now, 0);
		
		inputCtrl(document.form.wan_pppoe_options_x_now, 0);
		inputCtrl(document.form.wan_pptp_options_x_now, 0);

		inputCtrl(document.form.wan_heartbeat_x_now, 0);
		document.getElementById("vpn_dhcp").style.display = "none";
		inputCtrl(document.form.dhcpc_mode_now, 1);
	}

	var wan_dhcpenable = document.form.wan_dhcpenable_x_now[0].checked;
	inputCtrl(document.form.wan_ipaddr_x_now, !wan_dhcpenable);
	inputCtrl(document.form.wan_netmask_x_now, !wan_dhcpenable);
	inputCtrl(document.form.wan_gateway_x_now, !wan_dhcpenable);

	var dnsenable = document.form.wan_dnsenable_x_now[0].checked;
	inputCtrl(document.form.wan_dns1_x_now, !dnsenable);
	inputCtrl(document.form.wan_dns2_x_now, !dnsenable);

	if(wan_type != "static"){
		if(document.form.wan_dhcpenable_x_now[0].checked){
			inputCtrl(document.form.wan_dnsenable_x_now[0], 1);
			inputCtrl(document.form.wan_dnsenable_x_now[1], 1);
		}
		else{
			inputCtrl(document.form.wan_dnsenable_x_now[0], 0);
			inputCtrl(document.form.wan_dnsenable_x_now[1], 1);
		}
	}

}

function change_wan_dhcp_enable(wan_dhcpenable_flag){
	inputCtrl(document.form.wan_ipaddr_x_now, !wan_dhcpenable_flag);
	inputCtrl(document.form.wan_netmask_x_now, !wan_dhcpenable_flag);
	inputCtrl(document.form.wan_gateway_x_now, !wan_dhcpenable_flag);
	if(!wan_dhcpenable_flag){
		document.form.wan_dnsenable_x_now[0].checked = 0;
		document.form.wan_dnsenable_x_now[1].checked = 1;
		inputCtrl(document.form.wan_dns1_x_now, 1);
		inputCtrl(document.form.wan_dns2_x_now, 1);
		inputCtrl(document.form.wan_dnsenable_x_now[0], 0);
		inputCtrl(document.form.wan_dnsenable_x_now[1], 1);
	}
	else{
		inputCtrl(document.form.wan_dnsenable_x_now[0], 1);
		inputCtrl(document.form.wan_dnsenable_x_now[1], 1);
	}
}

function change_dnsenable(dnseanble_flag){
	inputCtrl(document.form.wan_dns1_x_now, !dnseanble_flag);
	inputCtrl(document.form.wan_dns2_x_now, !dnseanble_flag);
}

/* password item show or not */
function pass_checked(obj){
	switchType(obj, document.form.show_pass_1.checked, true);
}

function showMAC(){
	var tempMAC = "";
	document.form.wan_hwaddr_x_now.value = login_mac_str();
	document.form.wan_hwaddr_x_now.focus();
}
</script>
</head>

<body onload="initial();" onunLoad="return unload_body();">
<div id="TopBanner"></div>
<div id="hiddenMask" class="popup_bg">
	<table cellpadding="5" cellspacing="0" id="dr_sweet_advise" class="dr_sweet_advise" align="center">
		<tr>
		<td>
			<div class="drword" id="drword" style="height:110px;"><#Main_alert_proceeding_desc4#> <#Main_alert_proceeding_desc1#>...
				<br/>
				<br/>
	    </div>
		  <div class="drImg"><img src="images/alertImg.png"></div>
			<div style="height:70px;"></div>
		</td>
		</tr>
	</table>
<!--[if lte IE 6.5]><iframe class="hackiframe"></iframe><![endif]-->
</div>

<div id="Loading" class="popup_bg"></div>

<iframe name="hidden_frame" id="hidden_frame" src="" width="0" height="0" frameborder="0"></iframe>

<form method="post" name="form" id="ruleForm" action="/start_apply.htm" target="hidden_frame">
<input type="hidden" name="productid" value="<% nvram_get("productid"); %>">
<input type="hidden" name="current_page" value="Advanced_MultiWAN_Content.asp">
<input type="hidden" name="next_page" value="Advanced_MultiWAN_Content.asp">
<input type="hidden" name="group_id" value="">
<input type="hidden" name="modified" value="0">
<input type="hidden" name="action_mode" value="apply">
<input type="hidden" name="action_script" value="reboot">
<input type="hidden" name="action_wait" value="10">
<input type="hidden" name="preferred_lang" id="preferred_lang" value="<% nvram_get("preferred_lang"); %>">
<input type="hidden" name="firmver" value="<% nvram_get("firmver"); %>">
<!---- connection settings start  ---->
<div id="connection_settings_table"  class="contentM_connection">
	<table border="0" align="center" cellpadding="5" cellspacing="5">
		<tr>
			<td align="left">
			<span id="con_settings_title" class="formfonttitle">Connection Settings</span>
			<div style="width:630px; height:2px;overflow:hidden;position:relative;left:0px;top:5px;" class="splitLine"></div>
			<div></div>
			</td>
		</tr>
		<tr>
			<td>
				<table width="100%" border="1" align="center" cellpadding="4" cellspacing="0" bordercolor="#6b8fa3" class="FormTable">
					<tr>
						<th><#Layer3Forwarding_x_ConnectionType_itemname#></th>
						<td align="left">
							<select id="wan_proto_menu" class="input_option" name="wan_proto_now" onchange="change_wan_type(this.value);">
								<option value="dhcp"><#BOP_ctype_title1#></option>
								<option value="static"><#BOP_ctype_title5#></option>
								<option value="pppoe">PPPoE</option>
								<option value="pptp">PPTP</option>
								<option value="l2tp">L2TP</option>
							</select>
		  				</td>
					</tr>
					<tr>
						<th><#Enable_WAN#></th>
						<td>
							<input type="radio" id="wan_enable_now1" name="wan_enable_now" class="input" value="1" <% nvram_match("wan_enable", "1", "checked"); %>><label for="wan_enable_now1"><#checkbox_Yes#></label>
							<input type="radio" id="wan_enable_now2" name="wan_enable_now" class="input" value="0" <% nvram_match("wan_enable", "0", "checked"); %>><label for="wan_enable_now2"><#checkbox_No#></label>
						</td>
					</tr>
					<tr>
						<th><a class="hintstyle" href="javascript:void(0);" onClick="openHint(7,22);"><#Enable_NAT#></a></th>
						<td>
							<input type="radio" id="wan_nat_x_now1" name="wan_nat_x_now" class="input" value="1" <% nvram_match("wan_nat_x", "1", "checked"); %>><label for="wan_nat_x_now1"><#checkbox_Yes#></label>
							<input type="radio" id="wan_nat_x_now2" name="wan_nat_x_now" class="input" value="0" <% nvram_match("wan_nat_x", "0", "checked"); %>><label for="wan_nat_x_now2"><#checkbox_No#></label>
						</td>
					</tr>
					<tr>
						<th><a class="hintstyle" href="javascript:void(0);" onClick="openHint(7,23);"><#BasicConfig_EnableMediaServer_itemname#></a>&nbsp&nbsp&nbsp&nbsp&nbsp&nbsp<a id="faq" href="" target="_blank" style="font-family:Lucida Console;text-decoration:underline;">UPnP&nbspFAQ</a></th>
						<td>
							<input type="radio" id="wan_upnp_enable_now1" name="wan_upnp_enable_now" class="input" value="1" onclick="return change_common_radio(this, 'LANHostConfig', 'wan_upnp_enable', '1')" <% nvram_match("wan_upnp_enable", "1", "checked"); %>><label for="wan_upnp_enable_now1"><#checkbox_Yes#></label>
							<input type="radio" id="wan_upnp_enable_now2" name="wan_upnp_enable_now" class="input" value="0" onclick="return change_common_radio(this, 'LANHostConfig', 'wan_upnp_enable', '0')" <% nvram_match("wan_upnp_enable", "0", "checked"); %>><label for="wan_upnp_enable_now2"><#checkbox_No#></label>
						</td>
					</tr>
		 		</table>
	  		</td>
		</tr>
		<tr id="IPsetting">
			<td>
				<table width="100%" border="1" align="center" cellpadding="4" cellspacing="0" bordercolor="#6b8fa3" class="FormTable">
				<thead>
					<tr>
						<td colspan="2"><#IPConnection_ExternalIPAddress_sectionname#></td>
					</tr>
				</thead>
				<tr id="wan_dhcp_tr">
					<th><#Layer3Forwarding_x_DHCPClient_itemname#></th>
					<td>
						<input type="radio" id="wan_dhcpenable_x_now1" name="wan_dhcpenable_x_now" class="input" value="1" onclick="change_wan_dhcp_enable(1);"><label for="wan_dhcpenable_x_now1"><#checkbox_Yes#></label>
						<input type="radio" id="wan_dhcpenable_x_now2" name="wan_dhcpenable_x_now" class="input" value="0" onclick="change_wan_dhcp_enable(0);"><label for="wan_dhcpenable_x_now2"><#checkbox_No#></label>
					</td>
				</tr>
				<tr>
					<th><a class="hintstyle" href="javascript:void(0);" onClick="openHint(7,1);"><#IPConnection_ExternalIPAddress_itemname#></a></th>
					<td><input type="text" name="wan_ipaddr_x_now" maxlength="15" class="input_15_table" value="" onKeyPress="return validator.isIPAddr(this, event);" ></td>
				</tr>
				<tr>
					<th><a class="hintstyle" href="javascript:void(0);" onClick="openHint(7,2);"><#IPConnection_x_ExternalSubnetMask_itemname#></a></th>
					<td><input type="text" name="wan_netmask_x_now" maxlength="15" class="input_15_table" value="" onKeyPress="return validator.isIPAddr(this, event);" ></td>
				</tr>
				<tr>
					<th><a class="hintstyle" href="javascript:void(0);" onClick="openHint(7,3);"><#IPConnection_x_ExternalGateway_itemname#></a></th>
					<td><input type="text" name="wan_gateway_x_now" maxlength="15" class="input_15_table" value="" onKeyPress="return validator.isIPAddr(this, event);" ></td>
				</tr>
				</table>
			</td>
		</tr>
		<tr id="DNSsetting">
			<td>
				<table width="100%" border="1" align="center" cellpadding="4" cellspacing="0" bordercolor="#6b8fa3"  class="FormTable">
          		<thead>
            	<tr>
             	<td colspan="2"><#IPConnection_x_DNSServerEnable_sectionname#></td>
            	</tr>
          		</thead>
         		<tr id="dnsenable_tr">
            		<th><a class="hintstyle" href="javascript:void(0);" onClick="openHint(7,12);"><#IPConnection_x_DNSServerEnable_itemname#></a></th>
					<td>
  					<input type="radio" id="wan_dnsenable_x_now1" name="wan_dnsenable_x_now" class="input" value="1" onclick="change_dnsenable(1);"/><label for="wan_dnsenable_x_now1"><#checkbox_Yes#></label>
  					<input type="radio" id="wan_dnsenable_x_now2" name="wan_dnsenable_x_now" class="input" value="0" onclick="change_dnsenable(0);"/><label for="wan_dnsenable_x_now2"><#checkbox_No#></label>
					<div id="yadns_hint" style="display:none;"></div>
					</td>
          		</tr>
          		<tr>
            		<th><a class="hintstyle" href="javascript:void(0);" onClick="openHint(7,13);"><#IPConnection_x_DNSServer1_itemname#></a></th>
            		<td><input type="text" maxlength="15" class="input_15_table" name="wan_dns1_x_now" value="" onkeypress="return validator.isIPAddr(this, event)" ></td>
          		</tr>
          		<tr>
            		<th><a class="hintstyle" href="javascript:void(0);" onClick="openHint(7,14);"><#IPConnection_x_DNSServer2_itemname#></a></th>
            		<td><input type="text" maxlength="15" class="input_15_table" name="wan_dns2_x_now" value="" onkeypress="return validator.isIPAddr(this, event)" ></td>
          		</tr>
        		</table>
	  		</td>	
		</tr>
		<tr id="PPPsetting" >
			<td>
		  		<table width="100%" border="1" align="center" cellpadding="4" cellspacing="0" bordercolor="#6b8fa3"  class="FormTable">
            	<thead>
            		<tr>
              			<td colspan="2"><#PPPConnection_UserName_sectionname#></td>
            		</tr>
            	</thead>
            	<tr>
					<th><a class="hintstyle" href="javascript:void(0);" onClick="openHint(7,29);"><#PPPConnection_Authentication_itemname#></a></th>
					<td align="left">
					    <select class="input_option" name="wan_auth_x_now" onChange="change_wan_type(document.form.wan_proto_now.value);">
						    <option value=""><#wl_securitylevel_0#></option>
						    <option value="8021x-md5">802.1x MD5</option>
					    </select></td>
				</tr>
            	<tr>
             	 	<th><a class="hintstyle" href="javascript:void(0);" onClick="openHint(7,4);"><#Username#></a></th>
              		<td><input type="text" maxlength="64" class="input_32_table" name="wan_pppoe_username_now" value="" onkeypress="return validator.isString(this, event)"></td>
            	</tr>
            	<tr id="tr_pppoe_password">
              		<th><a class="hintstyle" href="javascript:void(0);" onClick="openHint(7,5);"><#PPPConnection_Password_itemname#></a></th>
              		<td>
					<div style="margin-top:2px;"><input type="password" autocapitalization="off" maxlength="64" class="input_32_table" id="wan_pppoe_passwd_now" name="wan_pppoe_passwd_now" value=""></div>
					<div style="margin-top:1px;"><input type="checkbox" name="show_pass_1" onclick="pass_checked(document.form.wan_pppoe_passwd_now);"><#QIS_show_pass#></div>
					</td>
            	</tr>
				<tr style="display:none">
              		<th><a class="hintstyle" href="javascript:void(0);" onClick="openHint(7,6);"><#PPPConnection_IdleDisconnectTime_itemname#></a></th>
              		<td>
                		<input type="text" maxlength="10" class="input_12_table" name="wan_pppoe_idletime_now" value="" onKeyPress="return validator.isNumber(this,event);" />
                		<input type="checkbox" style="margin-left:30;display:none;" name="wan_pppoe_idletime_check" value="" />
              		</td>
            	</tr>
            	<tr>
              		<th><a class="hintstyle" href="javascript:void(0);" onClick="openHint(7,7);"><#PPPConnection_x_PPPoEMTU_itemname#></a></th>
              		<td><input type="text" maxlength="5" name="wan_pppoe_mtu_now" class="input_6_table" value="" onKeyPress="return validator.isNumber(this,event);"/></td>
            	</tr>
            	<tr>
              		<th><a class="hintstyle" href="javascript:void(0);" onClick="openHint(7,8);"><#PPPConnection_x_PPPoEMRU_itemname#></a></th>
              		<td><input type="text" maxlength="5" name="wan_pppoe_mru_now" class="input_6_table" value="" onKeyPress="return validator.isNumber(this,event);"/></td>
            	</tr>
            	<tr>
              		<th><a class="hintstyle" href="javascript:void(0);" onClick="openHint(7,9);"><#PPPConnection_x_ServiceName_itemname#></a></th>
              		<td><input type="text" maxlength="32" class="input_32_table" name="wan_pppoe_service_now" value="" onkeypress="return validator.isString(this, event)"/></td>
            	</tr>
            	<tr>
              		<th><a class="hintstyle" href="javascript:void(0);" onClick="openHint(7,10);"><#PPPConnection_x_AccessConcentrator_itemname#></a></th>
              		<td><input type="text" maxlength="32" class="input_32_table" name="wan_pppoe_ac_now" value="" onkeypress="return validator.isString(this, event)"/></td>
            	</tr>
				<tr>
					<th><a class="hintstyle" href="javascript:void(0);" onClick="openHint(7,17);"><#PPPConnection_x_PPTPOptions_itemname#></a></th>
					<td>
						<select name="wan_pptp_options_x_now" class="input_option">
							<option value="" <% nvram_match("wan_pptp_options_x_now", "","selected"); %>><#Auto#></option>
							<option value="-mppc" <% nvram_match("wan_pptp_options_x_now", "-mppc","selected"); %>><#No_Encryp#></option>
							<option value="+mppe-40" <% nvram_match("wan_pptp_options_x_now", "+mppe-40","selected"); %>>MPPE 40</option>
							<option value="+mppe-128" <% nvram_match("wan_pptp_options_x_now", "+mppe-128","selected"); %>>MPPE 128</option>
						</select>
					</td>
				</tr>
				<tr>
					<th><a class="hintstyle" href="javascript:void(0);" onClick="openHint(7,18);"><#PPPConnection_x_AdditionalOptions_itemname#></a></th>
					<td><input type="text" name="wan_pppoe_options_x_now" value="<% nvram_get("wan_pppoe_options_x_now"); %>" class="input_32_table" maxlength="255" onKeyPress="return validator.isString(this, event)" onBlur="validator.string(this)"></td>
				</tr>
          </table>
        </td>
    </tr>
    <tr>
    	<td>
			<table width="100%" border="1" align="center" cellpadding="4" cellspacing="0" bordercolor="#6b8fa3"  class="FormTable">
			  	<thead>
				<tr>
			        <td colspan="2"><#PPPConnection_x_HostNameForISP_sectionname#></td>
				</tr>
				</thead>
				<tr>
			      	<th><a class="hintstyle" href="javascript:void(0);" onClick="openHint(7,19);"><#BOP_isp_heart_item#></a></th>
			      	<td>
			      	<input type="text" name="wan_heartbeat_x_now" class="input_32_table" maxlength="256" value="" onKeyPress="return validator.isString(this, event)" autocorrect="off" autocapitalize="off"></td>
				</tr>
				<tr id="vpn_dhcp">
					<th><!--a class="hintstyle" href="javascript:void(0);" onClick="openHint(7,);"--><#PPPConnection_x_vpn_dhcp_itemname#><!--/a--></th><!-- Enable VPN + DHCP Connection -->
					<td><input type="radio" name="wan_vpndhcp_now" class="input" value="1" onclick="return change_common_radio(this, 'IPConnection', 'wan_vpndhcp', 1)" <% nvram_match("wan_vpndhcp", "1", "checked"); %> /><#checkbox_Yes#>
				    <input type="radio" name="wan_vpndhcp_now" class="input" value="0" onclick="return change_common_radio(this, 'IPConnection', 'wan_vpndhcp', 0)" <% nvram_match("wan_vpndhcp", "0", "checked"); %> /><#checkbox_No#>
					</td>
			    </tr>
			    <tr>
			      	<th><a class="hintstyle" href="javascript:void(0);" onClick="openHint(7,15);"><#PPPConnection_x_HostNameForISP_itemname#></a></th>
			      	<td>
			      		<div><input type="text" name="wan_hostname_now" class="input_32_table" maxlength="32" value="" onkeypress="return validator.isString(this, event)" autocorrect="off" autocapitalize="off"><br/><span id="alert_msg1" style="color:#FC0;"></span></div>
			      	</td>
			    </tr>
			    <tr>
			      	<th ><a class="hintstyle" href="javascript:void(0);" onClick="openHint(7,16);"><#PPPConnection_x_MacAddressForISP_itemname#></a></th>
					<td>
						<input type="text" name="wan_hwaddr_x_now" class="input_20_table" maxlength="17" value="" onKeyPress="return validator.isHWAddr(this,event)" autocorrect="off" autocapitalize="off">
						<input type="button" class="button_gen" onclick="showMAC();" value="<#BOP_isp_MACclone#>">
					</td>
			    </tr>
				<tr>
					<th><a class="hintstyle" href="javascript:void(0);" onClick="openHint(7,30);"><#DHCP_query_freq#></a></th>
					<td>
					<select name="dhcpc_mode_now" class="input_option">
						<option value="0"><#DHCPnormal#></option>
						<option value="1"><#DHCPaggressive#></option>
					</select>
					</td>
				</tr>
				<tr>
					<th><a class="hintstyle" href="javascript:void(0);" onClick=""><#Extend_TTL_Value#></a></th>
					<td>
						<input type="radio" id="ttl_inc_enable_now1" name="ttl_inc_enable_now" class="input" value="1"><label for="ttl_inc_enable_now1"><#checkbox_Yes#></label>
						<input type="radio" id="ttl_inc_enable_now2" name="ttl_inc_enable_now" class="input" value="0"><label for="ttl_inc_enable_now2"><#checkbox_No#></label>
					</td>
				</tr>
			</table>
    	</td>
    </tr>
	</table>
	<div id="wan_interfaces_Block"></div>

	<div style="margin-top:5px;padding-bottom:10px;width:100%;text-align:center;">
		<input class="button_gen" type="button" onclick="hide_connection_settings();" value="<#CTL_Cancel#>">
		<input class="button_gen" type="button" onclick="save_connection_settings();" value="<#CTL_ok#>">
	</div>
</div>
<!---- connection settings end  ---->

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
	  		<table width="760px" border="0" cellpadding="5" cellspacing="0" class="FormTitle" id="FormTitle">
				<tbody>
					<tr>
						<td bgcolor="#4D595D" valign="top"  >
							<div>&nbsp;</div>
							<div class="formfonttitle"><#menu5_3#> - <#menu5_3_1#></div>
							<div style="margin:10px 0 10px 5px;" class="splitLine"></div>
							<table width="98%" border="1" align="center" cellpadding="4" cellspacing="0" class="FormTable_table" id="vlan_list_table" style="margin-top:30px;">
								<thead>
					   				<tr>
					   					<td colspan="8"><#menu5_3_1#></td><!--untranslated-->
					   				</tr>
					  			</thead>
				    			<tr>
				    				<th style="width:13%">Enable/Disable</th><!--untranslated-->
				    				<th style="width:13%">State</th><!--untranslated-->
				      				<th><#WAN_Interface_Title#></th>
				    				<th style="width:20%"><#Connectiontype#></th>
				      				<th style="width:22%">IP Address</th><!--untranslated-->
				      				<th style="width:7%"><#pvccfg_edit#></th>
				      			</tr>
				      			<tr>
									<td></td>				      			
				      				<td>Connected</td>
				      				<td>Ethernet WAN</td>
				    				<td style="width:15%">DHCP</td>
				      				<td style="width:17%">192.168.123.112</td>
				      				<td style="width:7%"><input class="edit_btn" onclick="configure_wans(this);" value=""/></td>
				    			</tr>
				      			<tr>
									<td></td>				      			
				      				<td>Disconnected</td>
				      				<td>Lan1</td>
				    				<td style="width:15%">PPPOE</td>
				      				<td style="width:17%">N/A</td>
				      				<td style="width:7%"><input class="edit_btn" onclick="configure_wans(this);" value=""/></td>
				    			</tr>				      			
							</table>
							<div id="wan_list_Block"></div>
							<div class="apply_gen">
								<input class="button_gen" onclick="applyRule()" type="button" value="<#CTL_apply#>"/>
							</div>
						</td>
					</tr>	
				</tbody>
			</table>
			<table>
			</table>
	  	</td>
	</tr>
</table>		
</td>	
</tr>
</table>
<div id="footer"></div>
</form>
</body>
</html>
