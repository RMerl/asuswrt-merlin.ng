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
<title><#Web_Title#> - IP Sec</title>
<link rel="stylesheet" type="text/css" href="index_style.css"> 
<link rel="stylesheet" type="text/css" href="form_style.css">

<script type="text/javascript" src="state.js"></script>
<script type="text/javascript" src="general.js"></script>
<script type="text/javascript" src="popup.js"></script>
<script type="text/javascript" src="help.js"></script>
<script type="text/javascript" src="js/jquery.js"></script>
<script language="JavaScript" type="text/javascript" src="form.js"></script>
<script language="JavaScript" type="text/javascript" src="validator.js"></script>
<script language="JavaScript" type="text/javascript" src="switcherplugin/jquery.iphone-switch.js"></script>
<script type="text/javascript" src="/js/httpApi.js"></script>
<style type="text/css">
.ipsec_view_log_panel {
	width: 720px;
	height: auto;
	margin-top: 65px;
	margin-left: 5px;
	position: absolute;
	-webkit-border-radius: 5px;
	-moz-border-radius: 5px;
	border-radius: 5px;
	z-index: 200;
	background-color: #2B373B;
	box-shadow: 3px 3px 10px #000;
	display: none;
	overflow: auto;
}
.ipsec_connect_status_title_bg {
	background: #C1C1C1;
	height: 25px;
	border-radius: 5px 5px 0 0;
	font-weight: bold;
	font-family: Verdana, Arial, Helvetica;
	color: rgb(119, 119, 119);
	font-size: x-small;
}
.ipsec_connect_status_title {
	float: left;
	width: 120px;
	text-align: center;
	margin: 5px 0;
}
.ipsec_connect_status_close {
	float: right;
	cursor: pointer;
}
.ipsec_connect_status_content_bg {
	height: 25px;
	color: #000000;
}
.ipsec_connect_status_content {
	font-family: Verdana, Arial, Helvetica;
	color: rgb(0, 0, 0);
	font-size: small;
	float: left;
	width: 120px;
	text-align: center;
	margin: 3px 0;
}
.ipsec_connect_status_panel {
	display: none;
	background: #E4E5E6;
	opacity:.95;
	border-radius: 5px;
	-webkit-border-radius: 5px;
	-moz-border-radius: 5px;
	border-radius: 5px;
	-moz-box-shadow: 3px 3px 4px #333;
	-webkit-box-shadow: 3px 3px 4px #333;
	box-shadow: 3px 3px 4px #000;
	-ms-filter: "progid:DXImageTransform.Microsoft.Shadow(Strength=4, Direction=135, Color='#333333')";
	filter: progid:DXImageTransform.Microsoft.Shadow(Strength=4, Direction=135, Color='#333333');
}
</style>
<script>
<% wanlink(); %>
var ipsec_profile_1 = decodeURIComponent('<% nvram_char_to_ascii("","ipsec_profile_1"); %>');
var ipsec_client_list_1 = decodeURIComponent('<% nvram_char_to_ascii("","ipsec_client_list_1"); %>');
var ipsec_client_list_array = ipsec_client_list_1;
var ipsec_server_enable = '<% nvram_get("ipsec_server_enable"); %>';
var ipsec_connect_status_array = new Array();
var ddns_enable_x = '<% nvram_get("ddns_enable_x"); %>';
var ddns_hostname_x = '<% nvram_get("ddns_hostname_x"); %>';

function initial(){
	show_menu();

	document.getElementById("ipsec_profile_1").value = ipsec_profile_1;
	document.getElementById("ipsec_client_list_1").value = ipsec_client_list_1;

	var vpn_server_array = { "PPTP" : ["PPTP", "Advanced_VPN_PPTP.asp"], "OpenVPN" : ["OpenVPN", "Advanced_VPN_OpenVPN.asp"], "IPSEC" : ["IPSec VPN", "Advanced_VPN_IPSec.asp"]};
	if(!pptpd_support) {
		delete vpn_server_array.PPTP;
	}
	if(!openvpnd_support) {
		delete vpn_server_array.OpenVPN;
	}
	if(!ipsec_srv_support) {
		delete vpn_server_array.IPSEC;
	}
	$('#divSwitchMenu').html(gen_switch_menu(vpn_server_array, "IPSEC"));

	ipsecShowAndHide(ipsec_server_enable);

	switchSettingsMode("1");

	while(document.form.ipsec_local_public_interface.options.length > 0){
		document.form.ipsec_local_public_interface.remove(0);
	}
	var wan_type_list = [];
	var option = ["wan", "<#dualwan_primary#>"];
	wan_type_list.push(option);
	if(dualWAN_support) {
		option = ["wan2", "<#dualwan_secondary#>"];
		wan_type_list.push(option);
	}

	for(var i = 0; i < wan_type_list.length; i += 1) {
		var option = document.createElement("option");
		option.value = wan_type_list[i][0];
		option.text = wan_type_list[i][1];
		document.form.ipsec_local_public_interface.add(option);
	}
	
	if(ipsec_profile_1 != "") {
		var editProfileArray = [];
		editProfileArray = ipsec_profile_1.split(">");
		editProfileArray.unshift("ipsec_profile_1");
		document.form.ipsec_local_public_interface.value = editProfileArray[5];
		document.form.ipsec_preshared_key.value = editProfileArray[8];
		document.form.ipsec_clients_start.value = editProfileArray[15];
		document.form.ipsec_dpd.value = editProfileArray[31];
		settingRadioItemCheck(document.form.ipsec_dead_peer_detection, editProfileArray[32]);
		var ipsec_samba_array = editProfileArray[37].split("<");
		document.form.ipsec_dns1.value = ipsec_samba_array[1];
		document.form.ipsec_dns2.value = ipsec_samba_array[2];
		document.form.ipsec_wins1.value = ipsec_samba_array[3];
		document.form.ipsec_wins2.value = ipsec_samba_array[4];
	}
	else {
		var ipsecLanIPAddr = "10.10.10.1";
		var ipsecLanNetMask = "255.255.255.0";
		var ipConflict;
		//1.check LAN IP
		ipConflict = checkIPConflict("LAN", ipsecLanIPAddr, ipsecLanNetMask);
		if(ipConflict.state) {
			document.form.ipsec_clients_start.value = "10.10.11";
		}
	}

	changeAdvDeadPeerDetection(document.form.ipsec_dead_peer_detection);
	setClientsEnd();

	showipsec_clientlist();

	if('<% nvram_get("ipsec_server_enable"); %>' == "1")
		update_connect_status();

	//check DUT is belong to private IP.
	setTimeout("show_warning_message();", 100);

	//set FAQ URL
	//	https://www.asus.com/support/FAQ/1033576
	httpApi.faqURL("1033576", function(url){document.getElementById("faq_windows").href=url;});
	//	https://www.asus.com/support/FAQ/1033575
	httpApi.faqURL("1033575", function(url){document.getElementById("faq_macOS").href=url;});
	//	https://www.asus.com/support/FAQ/1033574
	httpApi.faqURL("1033574", function(url){document.getElementById("faq_iPhone").href=url;});
	//	https://www.asus.com/support/FAQ/1033572
	httpApi.faqURL("1033572", function(url){document.getElementById("faq_android").href=url;});
}

var MAX_RETRY_NUM = 5;
var external_ip_retry_cnt = MAX_RETRY_NUM;
function show_warning_message(){
	if(realip_support && (based_modelid == "BRT-AC828"|| wans_mode != "lb")){
		if(realip_state != "2" && external_ip_retry_cnt > 0){
			if( external_ip_retry_cnt == MAX_RETRY_NUM )
				get_real_ip();
			else
				setTimeout("get_real_ip();", 3000);
		}
		else if(realip_state != "2"){
			if(validator.isPrivateIP(wanlink_ipaddr())){
				document.getElementById("privateIP_notes").innerHTML = "<#vpn_privateIP_hint#>";
				document.getElementById("privateIP_notes").style.display = "";
				$(".general_server_addr").html("-");
				//	http://www.asus.com/support/FAQ/1033906
				httpApi.faqURL("1033906", function(url){document.getElementById("faq_port_forwarding").href=url;});	//this id is include in string : #vpn_privateIP_hint#
			}
			else {
				if(ddns_enable_x == "1" && ddns_hostname_x != "") {
					$(".general_server_addr").html(ddns_hostname_x);
				}
				else {
					$(".general_server_addr").html(wanlink_ipaddr() + ', ' + '<a href="../Advanced_ASUSDDNS_Content.asp" target="_blank" style="text-decoration: underline; font-family:Lucida Console;">please click here to set the DDNS.</a>');/*untranslated*/
				}
			}
		}
		else{
			if(!external_ip){
				document.getElementById("privateIP_notes").innerHTML = "<#vpn_privateIP_hint#>";
				document.getElementById("privateIP_notes").style.display = "";
				$(".general_server_addr").html("-");
				//	http://www.asus.com/support/FAQ/1033906
				httpApi.faqURL("1033906", function(url){document.getElementById("faq_port_forwarding").href=url;});	//this id is include in string : #vpn_privateIP_hint#
			}
			else {
				if(ddns_enable_x == "1" && ddns_hostname_x != "") {
					$(".general_server_addr").html(ddns_hostname_x);
				}
				else {
					$(".general_server_addr").html(wanlink_ipaddr() + ', ' + '<a href="../Advanced_ASUSDDNS_Content.asp" target="_blank" style="text-decoration: underline; font-family:Lucida Console;">please click here to set the DDNS.</a>');/*untranslated*/
				}
			}
		}
	}
	else if(validator.isPrivateIP(wanlink_ipaddr())){
		document.getElementById("privateIP_notes").innerHTML = "<#vpn_privateIP_hint#>";
		document.getElementById("privateIP_notes").style.display = "";
		$(".general_server_addr").html("-");
		//	http://www.asus.com/support/FAQ/1033906
		httpApi.faqURL("1033906", function(url){document.getElementById("faq_port_forwarding").href=url;});	//this id is include in string : #vpn_privateIP_hint#
	}
	else {
		if(ddns_enable_x == "1" && ddns_hostname_x != "") {
			$(".general_server_addr").html(ddns_hostname_x);
		}
		else {
			$(".general_server_addr").html(wanlink_ipaddr() + ', ' + '<a href="../Advanced_ASUSDDNS_Content.asp" target="_blank" style="text-decoration: underline; font-family:Lucida Console;">please click here to set the DDNS.</a>');/*untranslated*/
		}
	}
}

function get_real_ip(){
	$.ajax({
		url: 'get_real_ip.asp',
		dataType: 'script',
		error: function(xhr){
			get_real_ip();
		},
		success: function(response){
			external_ip_retry_cnt--;
			show_warning_message();
		}
	});
}

function ipsecShowAndHide(server_enable) {
	if(server_enable == "1") {
		ipsec_server_enable = 1;
	}
	else {
		ipsec_server_enable = 0;
	}

	document.form.ipsec_server_enable.value = ipsec_server_enable;
	if(ipsec_server_enable == "1") {
		showhide("tr_general_server_addr", 1);
		showhide("tr_general_connection_status", 1);
		showhide("tr_general_log", 1);
		$("#ipsec_main_setting").css("display", "");
	}
	else{
		showhide("tr_general_server_addr", 0);
		showhide("tr_general_connection_status", 0);
		showhide("tr_general_log", 0);
		$("#ipsec_main_setting").css("display", "none");
	}
}
function switchSettingsMode(mode){
	if(mode == "1") {
		$(".tr_general").css("display", "");
		$(".tr_advanced").css("display", "none");
	}	
	else {
		$(".tr_general").css("display", "none");
		$(".tr_advanced").css("display", "");
	}
}
function showipsec_clientlist() {
	var ipsec_client_list_row = ipsec_client_list_array.split('<');
	var code = "";
	var ipsec_user_name = "";
	var ipsec_user_pwd = "";

	code +='<table width="100%" cellspacing="0" cellpadding="4" align="center" class="list_table" id="ipsec_client_list_table">';
	if(ipsec_client_list_row.length == 1)
		code +='<tr><td style="color:#FFCC00;" colspan="5"><#IPConnection_VSList_Norule#></td></tr>';
	else{
		for(var i = 1; i < ipsec_client_list_row.length; i++){
			code +='<tr>';
			var ipsec_client_list_col = ipsec_client_list_row[i].split('>');
			for(var j = 0; j < ipsec_client_list_col.length; j++) {
				if(j == 0) {
					if(ipsec_client_list_col[0].length > 28) {
						ipsec_user_name = ipsec_client_list_col[0].substring(0, 26) + "...";
						code +='<td width="45%" title="' + ipsec_client_list_col[0] + '">'+ ipsec_user_name +'</td>';
					}
					else
						code +='<td width="45%" title="' + ipsec_client_list_col[0] + '">' + ipsec_client_list_col[0] + '</td>';
				}
				else if(j == 1) {
					if(ipsec_client_list_col[1].length > 28) {
						ipsec_user_pwd = ipsec_client_list_col[1].substring(0, 26)+"...";
						code +='<td width="45%" title="' + ipsec_client_list_col[1] + '">'+ ipsec_user_pwd +'</td>';
					}
					else
						code +='<td width="45%" title="' + ipsec_client_list_col[1] + '">'+ ipsec_client_list_col[1] +'</td>';
				} 
			}
			
			code +='<td width="10%">';
			code +='<input class="remove_btn" onclick="del_Row(this);" value=""/></td>';
			code +='</tr>';
		}
	}
	
	code +='</table>';
	document.getElementById("ipsec_client_list_Block").innerHTML = code;
}
function addRow_Group(upper) {
	var username_obj = document.form.ipsec_client_list_username;
	var	password_obj = document.form.ipsec_client_list_password;

	var rule_num = document.getElementById("ipsec_client_list_table").rows.length;
	var item_num = document.getElementById("ipsec_client_list_table").rows[0].cells.length;		
	if(rule_num >= upper) {
		alert("<#JS_itemlimit1#> " + upper + " <#JS_itemlimit2#>");
		return false;	
	}

	var validAccount = function() {
		var valid_username = document.form.ipsec_client_list_username;
		var valid_password = document.form.ipsec_client_list_password;

		if(valid_username.value == "") {
			alert("<#JS_fieldblank#>");
			valid_username.focus();
			return false;
		}
		else if(!Block_chars(valid_username, [" ", "@", "*", "+", "|", ":", "?", "<", ">", ",", ".", "/", ";", "[", "]", "\\", "=", "\"", "&" ])) {
			return false;
		}

		if(valid_password.value == "") {
			alert("<#JS_fieldblank#>");
			valid_password.focus();
			return false;
		}
		else if(!Block_chars(valid_password, ["<", ">", "&", "\""])) {
			return false;
		}
		else if(valid_password.value.length > 0 && valid_password.value.length < 5) {
			alert("* <#JS_short_password#>");
			valid_password.focus();
			return false;
		}

		//confirm common string combination	#JS_common_passwd#
		var is_common_string = check_common_string(valid_password.value, "httpd_password");
		if(valid_password.value.length > 0 && is_common_string){
			if(!confirm("<#JS_common_passwd#>")){
				valid_password.focus();
				valid_password.select();
				return false;
			}
		}
	
		return true;
	};


	if(validAccount()) {
		if(item_num >= 2) {
			for(var i = 0; i < rule_num; i +=1 ) {
				if(username_obj.value == document.getElementById("ipsec_client_list_table").rows[i].cells[0].title) {
					alert("<#JS_duplicate#>");
					username_obj.focus();
					username_obj.select();
					return false;
				}	
			}
		}
		
		var addRow = function(obj, head) {
			if(head == 1)
				ipsec_client_list_array += "<" /*&#60*/
			else	
				ipsec_client_list_array += ">" /*&#62*/
					
			ipsec_client_list_array += obj.value;
			obj.value = "";
		}

		addRow(username_obj ,1);
		addRow(password_obj, 0);
		showipsec_clientlist();
	}
}
function del_Row(rowdata) {
	var i = rowdata.parentNode.parentNode.rowIndex;
	document.getElementById("ipsec_client_list_table").deleteRow(i);
	var ipsec_client_list_value = "";
	var rowLength = document.getElementById("ipsec_client_list_table").rows.length;
	
	for(var k = 0; k < rowLength; k += 1) {
		for(var j = 0; j < document.getElementById("ipsec_client_list_table").rows[k].cells.length - 1; j += 1) {
			if(j == 0)
				ipsec_client_list_value += "<";
			else {
				ipsec_client_list_value += document.getElementById("ipsec_client_list_table").rows[k].cells[0].title;
				ipsec_client_list_value += ">";
				ipsec_client_list_value += document.getElementById("ipsec_client_list_table").rows[k].cells[1].title;
			}
		}
	}

	ipsec_client_list_array = ipsec_client_list_value;

	if(ipsec_client_list_array == "")
		showipsec_clientlist();
}
function changeAdvDeadPeerDetection (obj) {
	if(obj.value == "0") {
		showhide("tr_adv_dpd_interval", 0);
	}
	else {
		showhide("tr_adv_dpd_interval", 1);
	}
}
function settingRadioItemCheck(obj, checkValue) {
	var radioLength = obj.length;
	for(var i = 0; i < radioLength; i += 1) {
		if(obj[i].value == checkValue) {
			obj[i].checked = true;
		}
	}
}
function getRadioItemCheck(obj) {
	var checkValue = "";
	var radioLength = obj.length;
	for(var i = 0; i < radioLength; i += 1) {
		if(obj[i].checked) {
			checkValue = obj[i].value;
			break;
		}
	}
	return 	checkValue;
}
function validForm() {
	if(ipsec_server_enable == "1") {
		if(!validator.isEmpty(document.form.ipsec_preshared_key))
			return false;
		if(!Block_chars(document.form.ipsec_preshared_key, [">", "<", "&", "\"", "null"]))
			return false;
		if(is_KR_sku){
			if(!validator.psk_KR(document.form.ipsec_preshared_key))
				return false;
		}
		else{
			if(!validator.psk(document.form.ipsec_preshared_key))
				return false;
		}
		//confirm common string combination	#JS_common_passwd#
		var is_common_string = check_common_string(document.form.ipsec_preshared_key.value, "wpa_key");
		if(is_common_string){
			if(!confirm("<#JS_common_passwd#>")){
				document.form.ipsec_preshared_key.focus();
				document.form.ipsec_preshared_key.select();
				return false;
			}
		}

		var ipAddr = document.form.ipsec_clients_start.value.trim() + ".1";
		var ipformat  = /^(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\.(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\.(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\.(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)$/;
		if(!(ipformat.test(ipAddr))) {
			alert(document.form.ipsec_clients_start.value+" <#JS_validip#>");
			document.form.ipsec_clients_start.focus();
			document.form.ipsec_clients_start.select();
			return false;
		}

		var ipsecLanIPAddr = ipAddr;
		var ipsecLanNetMask = "255.255.255.0";
		var ipConflict;
		//1.check LAN IP
		ipConflict = checkIPConflict("LAN", ipsecLanIPAddr, ipsecLanNetMask);
		if(ipConflict.state) {
			alert("Conflict with LAN IP: " + ipConflict.ipAddr + ",\n" + "Network segment is " + ipConflict.netLegalRangeStart + " ~ " + ipConflict.netLegalRangeEnd);
			document.form.ipsec_clients_start.focus();
			document.form.ipsec_clients_start.select();
			return false;
		}

		if(getRadioItemCheck(document.form.ipsec_dead_peer_detection) == "1") {
			if(!validator.numberRange(document.form.ipsec_dpd, 10, 900)) {
				return false;
			}
		}

		if(document.form.ipsec_dns1.value != "") {
			if(!Block_chars(document.form.ipsec_dns1, [">", "<"]))
				return false;
			if(!validator.isLegalIP(document.form.ipsec_dns1))
				return false
		}
		if(document.form.ipsec_dns2.value != "") {
			if(!Block_chars(document.form.ipsec_dns2, [">", "<"]))
				return false;
			if(!validator.isLegalIP(document.form.ipsec_dns2))
				return false
		}
		if(document.form.ipsec_wins1.value != "") {
			if(!Block_chars(document.form.ipsec_wins1, [">", "<"]))
				return false;
			if(!validator.isLegalIP(document.form.ipsec_wins1))
				return false
		}
		if(document.form.ipsec_wins2.value != "") {
			if(!Block_chars(document.form.ipsec_wins2, [">", "<"]))
				return false;
			if(!validator.isLegalIP(document.form.ipsec_wins2))
				return false
		}
	}

	return true;
}
function applyRule() {
	if(validForm()) {
		/* data structure
		1 vpn_type, profilename, remote_gateway_method, remote_gateway, 
		5 local_public_interface, local_public_ip, auth_method, auth_method_value, 
		9 local_subnet, local_port, remote_subnet, remote_port, 
		13 transport/tunnel type, virtual_ip, virtual_subnet, accessible_networks, 
		17 ike, encryption_p1, hash_p1, exchange,
		21 local id, remote id, keylife_p1, xauth,
		25 xauth_account, xauth_password, xauth_server_type, traversal,
		29 ike_isakmp, ike_isakmp_nat, ipsec_dpd, dead_peer_detection,
		33 encryption_p2, hash_p2, keylife_p2, keyingtries
		37 samba list, default Activate is 0;
		*/
		var oriProfileArray = [];
		oriProfileArray = ipsec_profile_1.split(">");
		oriProfileArray.unshift("ipsec_profile_1");
		var ipsec_dpd = oriProfileArray[31];
		var ipsec_dead_peer_detection = getRadioItemCheck(document.form.ipsec_dead_peer_detection);
		if(ipsec_dead_peer_detection == "1")
			ipsec_dpd = document.form.ipsec_dpd.value;
		var samba_list = "<" + document.form.ipsec_dns1.value + "<" + document.form.ipsec_dns2.value + "<" + document.form.ipsec_wins1.value + "<" + document.form.ipsec_wins2.value;
		var profile_array = [ "",
			"4", "Host-to-Net", "null", "null",
			document.form.ipsec_local_public_interface.value, "", "1", document.form.ipsec_preshared_key.value,
			"null", "null", "null", "null",
			"null", "1", document.form.ipsec_clients_start.value, "null",
			"1", "null", "null", "0",
			"null", "null", "null", "1",
			"", "", "eap-md5", "1",
			"500", "4500", ipsec_dpd, ipsec_dead_peer_detection,
			"null", "null", "null", "null",
			samba_list, ipsec_server_enable
		];

		var result = "";
		for(var i = 1; i < profile_array.length; i += 1) {
			result += profile_array[i] + ">";
		}
		result = result.slice(0, -1);

		
		if(ipsec_server_enable == "1") {
			document.form.ipsec_profile_1.value = result;
			document.form.ipsec_client_list_1.disabled = false;
			document.form.ipsec_client_list_1.value = ipsec_client_list_array;
		}
		else {
			if(document.form.ipsec_profile_1.value != "") {
				document.form.ipsec_profile_1.disabled = false;
				var orgProfile = document.form.ipsec_profile_1.value;
				var tempProfile = orgProfile.substring(0, (orgProfile.length - 2)) + ">0";
				document.form.ipsec_profile_1.value = tempProfile;
			}
			else {
				document.form.ipsec_profile_1.disabled = true;
			}
			document.form.ipsec_client_list_1.disabled = true;
		}

		document.form.ipsec_server_enable.value = ipsec_server_enable;
		var actionScript = "ipsec_stop";
		if(ipsec_server_enable == "1") {
			actionScript = "ipsec_start";
		}
		document.form.action_script.value = actionScript;
		document.form.submit();
	}
}
function viewLog() {
	update_ipsec_log();
	$("#ipsec_view_log_panel").fadeIn(300);
}
function cancel_viewLog() {
	$("#ipsec_view_log_panel").fadeOut(300);
}
function clear_viewLog() {
	document.clearLog.submit();
	document.getElementById("textarea").innerText = "";
}
function save_viewLog() {
	location.href = "ipsec.log";
}
function refresh_viewLog() {
	update_ipsec_log();
}
function update_ipsec_log() {
	$.ajax({
		url: '/ajax_ipsec_log.xml',
		dataType: 'xml',

		error: function(xml) {
			setTimeout("update_ipsec_log();", 1000);
		},

		success: function(xml) {
			var ipsecXML = xml.getElementsByTagName("ipsec");
			var ipsec_log = ipsecXML[0].firstChild.nodeValue;
			document.getElementById("textarea").innerText = ipsec_log;
		}
	});	
}
function setClientsEnd() {
	var ipAddrEnd = "";
	var ipAddr = document.form.ipsec_clients_start.value.trim() + ".1";
	var ipformat  = /^(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\.(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\.(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\.(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)$/;
	if((ipformat.test(ipAddr))) {
		ipAddrEnd = document.form.ipsec_clients_start.value.trim() + ".254";
		$("#ipsec_clients_end").html(ipAddrEnd);
	}
}

function update_connect_status() {
	$.ajax({
		url: '/ajax_ipsec.asp',
		dataType: 'script',
		timeout: 1500,
		error: function(xhr){
			setTimeout("update_connect_status();",1000);
		},	
		success: function() {
			ipsec_connect_status_array = [];
			for(var i = 0; i < ipsec_connect_status.length; i += 1) {
				ipsec_connect_status_array["Host-to-Net"] = ipsec_connect_status[i][1];
			}
			if(ipsec_connect_status_array["Host-to-Net"] != undefined) {
				var connected_count = (ipsec_connect_status_array["Host-to-Net"].split("<").length - 1);
				if(connected_count > 0) {
					var code = "";
					code +='<a class="hintstyle2" href="javascript:void(0);" onClick="showIPSecClients(\'Host-to-Net\', event);">';
					code +='<#btn_Enabled#>(' + connected_count + ')</a>';
					$(".general_connection_status").html(code);
				}
				else
					$(".general_connection_status").html("-");
			}
			else {
				$(".general_connection_status").html("-");
			}
			setTimeout("update_connect_status();",3000);
		}
	});
}
function close_connect_status() {
	$("#connection_ipsec_profile_panel").fadeOut(300);
}
function showIPSecClients(profileName, e) {
	$("#connection_ipsec_profile_panel").fadeIn(300);
	$("#connection_ipsec_profile_panel").css("position", "absolute");
	$("#connection_ipsec_profile_panel").css("top", "440px");
	$("#connection_ipsec_profile_panel").css("left", "225px");
	
	var html = "";
	html += "<div class='ipsec_connect_status_title_bg'>";
	html += "<div class='ipsec_connect_status_title' style='width:240px;'>Remote IP</div>";/*untranslated*/
	html += "<div class='ipsec_connect_status_title'><#statusTitle_Client#></div>";
	html += "<div class='ipsec_connect_status_title'><#Access_Time#></div>";
	html += "<div class='ipsec_connect_status_title'><#vpn_ipsec_XAUTH#> <#Permission_Management_Users#></div>";
	html += "<div class='ipsec_connect_status_title'>PSKRAUTHTIME</div>";/*untranslated*/
	html += "<div class='ipsec_connect_status_close'><a onclick='close_connect_status();'><img width='18px' height='18px' src=\"/images/button-close.png\" onmouseover='this.src=\"/images/button-close2.png\"' onmouseout='this.src=\"/images/button-close.png\"' border='0'></a></div>";
	html += "</div>";
	html += "<div style='clear:both;'></div>";

	var statusText = [[""], ["<#Connected#>"], ["<#Connecting_str#>"], ["<#Connecting_str#>"]]
	var profileName_array = ipsec_connect_status_array[profileName].split("<");
	for (i = 0; i < profileName_array.length; i += 1) {
		if(profileName_array[i] != "") {
			var profileName_col = profileName_array[i].split(">");
			html += "<div class='ipsec_connect_status_content_bg'>";
			html += "<div class='ipsec_connect_status_content' style='width:240px;word-wrap:break-word;word-break:break-all;'>" + profileName_col[0] + "</div>";
			html += "<div class='ipsec_connect_status_content'>" + statusText[profileName_col[1]] + "</div>";
			html += "<div class='ipsec_connect_status_content'>" + profileName_col[2] + "</div>";
			html += "<div class='ipsec_connect_status_content'>" + profileName_col[3] + "</div>";
			html += "<div class='ipsec_connect_status_content'>" + profileName_col[4] + "</div>";
			html += "</div>";
			html += "<div style='clear:both;'></div>";
		}	
	}
	$("#connection_ipsec_profile_panel").html(html);
}
</script>
</head>

<body onload="initial();">
<div id="TopBanner"></div>
<div id="Loading" class="popup_bg"></div>

<iframe name="hidden_frame" id="hidden_frame" src="" width="0" height="0" frameborder="0"></iframe>
<form method="post" name="form" action="/start_apply.htm" target="hidden_frame">
<input type="hidden" name="productid" value="<% nvram_get("productid"); %>">
<input type="hidden" name="current_page" value="Advanced_VPN_IPSec.asp">
<input type="hidden" name="next_page" value="Advanced_VPN_IPSec.asp">
<input type="hidden" name="modified" value="0">
<input type="hidden" name="action_mode" value="apply">
<input type="hidden" name="action_script" value="saveNvram">
<input type="hidden" name="action_wait" value="30">
<input type="hidden" name="preferred_lang" id="preferred_lang" value="<% nvram_get("preferred_lang"); %>">
<input type="hidden" name="ipsec_server_enable" value="<% nvram_get("ipsec_server_enable"); %>">
<input type="hidden" name="ipsec_profile_1" id="ipsec_profile_1" value="">
<input type="hidden" name="ipsec_client_list_1" id="ipsec_client_list_1" value="">

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
					<td valign="top" >
						<div id="ipsec_view_log_panel" class="ipsec_view_log_panel" style="padding:15px;">
							<textarea cols="63" rows="44" wrap="off" readonly="readonly" id="textarea" style="width:99%; font-family:'Courier New', Courier, mono; font-size:11px;background:#475A5F;color:#FFFFFF;resize:none;"><% nvram_dump("ipsec.log",""); %></textarea>
							<div style='text-align:center;margin-top:15px;'>
								<input class="button_gen" onclick="cancel_viewLog();" type="button" value="<#CTL_Cancel#>"/>
								<input class="button_gen" onclick="clear_viewLog();" type="button" value="<#CTL_clear#>"/>
								<input class="button_gen" onclick="save_viewLog();" type="button" value="<#CTL_onlysave#>"/>
								<input class="button_gen" onclick="refresh_viewLog();" type="button" value="<#CTL_refresh#>"/>
							</div>
						</div>
						<table width="760px" border="0" cellpadding="5" cellspacing="0" class="FormTitle" id="FormTitle">
						<tbody>
							<tr>
								<td bgcolor="#4D595D" valign="top">
									<div>&nbsp;</div>
									<div class="formfonttitle"><#BOP_isp_heart_item#> - IPSec VPN<!--untranslated--></div>
									<div id="divSwitchMenu" style="margin-top:-40px;float:right;"></div>
									<div style="margin:10px 0 10px 5px;" class="splitLine"></div>
									<div id="privateIP_notes" class="formfontdesc" style="display:none;color:#FFCC00;"></div>
									<div class="formfontdesc">
										<span style="color:#FC0"><#vpn_ipsec_note#></span>
									</div>

									<table id="ipsec_general_setting" width="100%" border="1" align="center" cellpadding="4" cellspacing="0" bordercolor="#6b8fa3" class="FormTable">
										<thead>
										<tr>
											<td colspan="2"><#t2BC#></td>
										</tr>
										</thead>
										<tr>
											<th><#vpn_ipsec_enable#></th>
											<td>
												<div align="center" class="left" style="float:left;cursor:pointer;" id="radio_ipsec_enable"></div>
												<div class="iphone_switch_container" style="height:32px; width:74px; position: relative; overflow: hidden;"></div>
												<script type="text/javascript">
												$('#radio_ipsec_enable').iphoneSwitch('<% nvram_get("ipsec_server_enable"); %>',
													function() {
														ipsecShowAndHide(1);
													},
													function() {
														ipsecShowAndHide(0);
													},
													{
														switch_on_container_path: '/switcherplugin/iphone_switch_container_off.png'
													}
												);
											</script>
											</td>
										</tr>
										<tr id="tr_general_server_addr">
											<th><#WLANAuthentication11a_ExAuthDBIPAddr_itemname#></th>
											<td class="general_server_addr">-</td>
										</tr>
										<tr id="tr_general_connection_status">
											<th><#PPPConnection_x_WANLink_itemname#></th>
											<td class="general_connection_status">-</td>
										</tr>
										<tr id="tr_general_log">
											<th><#System_Log#></th>
											<td>
												<input class="button_gen" onclick="viewLog()" type="button" value="<#CTL_check_log#>"/>
											</td>
										</tr>
									</table>

									<div id="ipsec_main_setting">
										<div class="formfontdesc" style="margin-top:8px;">
											How to setup IPSec VPN client
											<br>
											<ol>
												<li><a id="faq_windows" href="" target="_blank" style="text-decoration:underline;">Windows</a></li>
												<li><a id="faq_macOS" href="" target="_blank" style="text-decoration:underline;">Mac OS</a></li>
												<li><a id="faq_iPhone" href="" target="_blank" style="text-decoration:underline;">iOS</a></li>
												<li><a id="faq_android" href="" target="_blank" style="text-decoration:underline;">Android</a></li>
											<ol>
										</div>
										<!-- Quick Select table start-->
										<table width="100%" border="1" align="center" cellpadding="4" cellspacing="0" bordercolor="#6b8fa3"  class="FormTable" style="margin-top:15px;">
											<thead>
											<tr>
												<td colspan="2"><#vpn_ipsec_Quick_Select#></td>
											</tr>
											</thead>
											<tr id="tr_SettingsMode">
												<th><#vpn_Adv#></th>
												<td>
													<select id="selSwitchMode" onchange="switchSettingsMode(this.options[this.selectedIndex].value)" class="input_option" style="margin-left:2px;">
														<option value="1" selected><#menu5_1_1#></option>
														<option value="2"><#menu5#></option>
													</select>
												</td>
											</tr>
											<tr class="tr_general">
												<th><#vpn_ipsec_PreShared_Key#></th>
												<td>
													<input id="ipsec_preshared_key" name="ipsec_preshared_key" type="text" class="input_25_table" maxlength="32" placeholder="<#vpn_preshared_key_hint#>" autocomplete="off" autocorrect="off" autocapitalize="off">
												</td>
											</tr>
											<tr class="tr_advanced">
												<th><#vpn_ipsec_IKE_ISAKMP_Port#></th>
												<td>500</td>
											</tr>
											<tr class="tr_advanced">
												<th><#vpn_ipsec_IKE_ISAKMP_NAT_Port#></th>
												<td>4500</td>
											</tr>
											<tr class="tr_advanced">
												<th><#vpn_client_ip#></th>
												<td>
													<input type="text" maxlength="11" class="input_12_table" name="ipsec_clients_start" onBlur="setClientsEnd();" value="10.10.10" autocomplete="off" autocorrect="off" autocapitalize="off"/>
													<span style="font-family: Lucida Console;color: #FFF;">.1 ~ </span>
													<span id="ipsec_clients_end" style="font-family: Lucida Console;color: #FFF;">10.10.10.254</span>
												</td>
											</tr>
											<tr class="tr_advanced">
												<th><#IPConnection_x_DNSServer1_itemname#></th>
												<td>
													<input type="text" maxlength="15" class="input_15_table" name="ipsec_dns1" onkeypress="return validator.isIPAddr(this, event)" autocomplete="off" autocorrect="off" autocapitalize="off">
													<span style="color:#FC0"><#feedback_optional#></span>
												</td>
											</tr>
											<tr class="tr_advanced">
												<th><#IPConnection_x_DNSServer2_itemname#></th>
												<td>
													<input type="text" maxlength="15" class="input_15_table" name="ipsec_dns2" onkeypress="return validator.isIPAddr(this, event)" autocomplete="off" autocorrect="off" autocapitalize="off">
													<span style="color:#FC0"><#feedback_optional#></span>
												</td>
											</tr>
											<tr class="tr_advanced">
												<th><#IPConnection_x_WINSServer1_itemname#></th>
												<td>
													<input type="text" maxlength="15" class="input_15_table" name="ipsec_wins1" onkeypress="return validator.isIPAddr(this, event)" autocomplete="off" autocorrect="off" autocapitalize="off">
													<span style="color:#FC0"><#feedback_optional#></span>
												</td>
											</tr>
											<tr class="tr_advanced">
												<th><#IPConnection_x_WINSServer2_itemname#></th>
												<td>
													<input type="text" maxlength="15" class="input_15_table" name="ipsec_wins2" onkeypress="return validator.isIPAddr(this, event)" autocomplete="off" autocorrect="off" autocapitalize="off">
													<span style="color:#FC0"><#feedback_optional#></span>
												</td>
											</tr>
											<tr id="tr_localPublicInterface" style="display:none;">
												<th><#vpn_ipsec_Local_Interface#></th>
												<td>
													<select name="ipsec_local_public_interface" class="input_option"></select>
												</td>
											</tr>
										</table>
										<!-- User table start-->
										<table id="tbAccountList" width="100%" border="1" align="center" cellpadding="4" cellspacing="0" class="FormTable_table tr_general" style="margin-top:15px;">
											<thead>
											<tr>
												<td colspan="3"><#Username_Pwd#>&nbsp;(<#List_limit#>&nbsp;8)</td>
											</tr>
											</thead>
											<tr>
												<th><#Username#></th>
												<th><#HSDPAConfig_Password_itemname#></th>
												<th><#list_add_delete#></th>
											</tr>
											<tr>
												<td width="45%">
													<input type="text" class="input_25_table" maxlength="32" name="ipsec_client_list_username" onKeyPress="return validator.isString(this, event)" autocomplete="off" autocorrect="off" autocapitalize="off">
												</td>
												<td width="45%">
													<input type="text" class="input_25_table" maxlength="32" name="ipsec_client_list_password" onKeyPress="return validator.isString(this, event)" autocomplete="off" autocorrect="off" autocapitalize="off">
												</td>
												<td width="10%">
													<div><input type="button" class="add_btn" onClick="addRow_Group(8);" value=""></div>
												</td>
											</tr>
										</table>
										<div id="ipsec_client_list_Block" class="tr_general"></div>
										<!-- User table end-->
										<!-- Quick Select table end-->
										<!-- Advanced Settings table start-->
										<div id="ipsec_advanced_settings" class="tr_advanced">
											<table width="100%" border="1" align="center" cellpadding="4" cellspacing="0" bordercolor="#6b8fa3"  class="FormTable" style="margin-top:15px;">
												<thead>
												<tr>
													<td colspan="2"><#menu5#> - <#vpn_ipsec_Phase_1_Negotiations#></td>
												</tr>
												</thead>
												<tr id="tr_adv_ike_version">
													<th><#vpn_ipsec_IKE_Version#></th>
													<td>v1</td>
												</tr>
												<tr id="tr_adv_exchange_mode">
													<th><#vpn_ipsec_Exchange_Mode#></th>
													<td><#vpn_ipsec_Main_Mode#></td>
												</tr>
												<tr id="tr_adv_dead_peer_detection">
													<th><#vpn_ipsec_DPD#></th>
													<td>
														<input type="radio" name="ipsec_dead_peer_detection" id="ipsec_dead_peer_detection_en" class="input" value="1" onchange="changeAdvDeadPeerDetection(this)" checked>
														<label for='ipsec_dead_peer_detection_en' id="ipsec_dead_peer_detection_en_label"><#WLANConfig11b_WirelessCtrl_button1name#></label>
														<input type="radio" name="ipsec_dead_peer_detection" id="ipsec_dead_peer_detection_dis" class="input" value="0" onchange="changeAdvDeadPeerDetection(this)">
														<label for='ipsec_dead_peer_detection_dis' id="ipsec_dead_peer_detection_dis_label"><#WLANConfig11b_WirelessCtrl_buttonname#></label>
													</td>
												</tr>
												<tr id="tr_adv_dpd_interval">
													<th><#vpn_ipsec_DPD_Checking_Interval#></th>
													<td>
														<input type="text" class="input_3_table" name="ipsec_dpd" maxlength="3" value="10" onKeyPress="return validator.isNumber(this,event)" autocomplete="off" autocorrect="off" autocapitalize="off">
														<span style="color:#FC0">(10~900) <#Second#></span>
													</td>
												</tr>
											</table>
										</div>
										<!-- Advanced Settings table end-->
									</div>

									<div id="ipsec_apply" class="apply_gen">
										
										<input class="button_gen" onclick="applyRule()" type="button" value="<#CTL_apply#>"/>
									</div>
								</td>
							</tr>
						</tbody>
						</table>
					</td>
				</tr>
			</table>
			<!--===================================End of Main Content===========================================-->
		</td>
		<td width="10" align="center" valign="top">&nbsp;</td>
	</tr>
</table>
<div id="connection_ipsec_profile_panel" class="ipsec_connect_status_panel"></div> 
</form>
<form method="post" name="clearLog" action="clear_file.cgi" target="hidden_frame">
<input type="hidden" name="current_page" value="Advanced_VPN_IPSec.asp">
<input type="hidden" name="next_page" value="Advanced_VPN_IPSec.asp">
<input type="hidden" name="group_id" value="">
<input type="hidden" name="modified" value="0">
<input type="hidden" name="action_mode" value="">
<input type="hidden" name="action_wait" value="">
<input type="hidden" name="first_time" value="">
<input type="hidden" name="action_script" value="">
<input type="hidden" name="preferred_lang" value="<% nvram_get("preferred_lang"); %>">
<input type="hidden" name="firmver" value="<% nvram_get("firmver"); %>">
<input type="hidden" name="clear_file_name" value="ipsec">
</form>
<div id="footer"></div>
</body>
</html>



