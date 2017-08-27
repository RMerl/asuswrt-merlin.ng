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
<style type="text/css">
.ipsec_profile_panel {
	width: 650px;
	height: auto;
	top:165px;
	margin-left: 226px;
	position: absolute;
	-webkit-border-radius: 5px;
	-moz-border-radius: 5px;
	border-radius: 5px;
	z-index: 200;
	background-color: #4D595D;
	box-shadow: 3px 3px 10px #000;
	display: none;
	overflow: auto;
}
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
.ipsec_add_profile_bg {
	height: 30px;
	line-height: 30px;
	margin: 10px 0;
}
.ipsec_add_profile_icon {
	width: 30px;
	height: 30px;
	cursor: pointer;
	float: left;
	margin-right: 10px;
	background-color: #00b0ff;
	border-radius: 50%;
	box-shadow: 0px 4px 10px 0px rgba(0, 0, 0, 0.20), 0px 2px 10px 0px rgba(0, 0, 0, 0.16);
	text-align: center;
	font-size: 20px;
	font-family: Roboto, sans-serif, Arial, Helvetica, sans-serif;
	font-weight: bold;
}
.ipsec_add_profile_title {
	cursor: pointer;
}
</style>
<script>
var subnetIP_support_IPv6 = false;
<% wanlink(); %>
<% secondary_wanlink(); %>
var ipsec_profile_1 = decodeURIComponent('<% nvram_char_to_ascii("","ipsec_profile_1"); %>');
var ipsec_profile_2 = decodeURIComponent('<% nvram_char_to_ascii("","ipsec_profile_2"); %>');
var ipsec_profile_3 = decodeURIComponent('<% nvram_char_to_ascii("","ipsec_profile_3"); %>');
var ipsec_profile_4 = decodeURIComponent('<% nvram_char_to_ascii("","ipsec_profile_4"); %>');
var ipsec_profile_5 = decodeURIComponent('<% nvram_char_to_ascii("","ipsec_profile_5"); %>');

var ipsec_client_list_1 = decodeURIComponent('<% nvram_char_to_ascii("","ipsec_client_list_1"); %>');
var ipsec_client_list_2 = decodeURIComponent('<% nvram_char_to_ascii("","ipsec_client_list_2"); %>');
var ipsec_client_list_3 = decodeURIComponent('<% nvram_char_to_ascii("","ipsec_client_list_3"); %>');
var ipsec_client_list_4 = decodeURIComponent('<% nvram_char_to_ascii("","ipsec_client_list_4"); %>');
var ipsec_client_list_5 = decodeURIComponent('<% nvram_char_to_ascii("","ipsec_client_list_5"); %>');

var ipsec_ca_1 = decodeURIComponent('<% nvram_char_to_ascii("","ipsec_ca_1"); %>');
var ipsec_ca_2 = decodeURIComponent('<% nvram_char_to_ascii("","ipsec_ca_2"); %>');
var ipsec_ca_3 = decodeURIComponent('<% nvram_char_to_ascii("","ipsec_ca_3"); %>');
var ipsec_ca_4 = decodeURIComponent('<% nvram_char_to_ascii("","ipsec_ca_4"); %>');
var ipsec_ca_5 = decodeURIComponent('<% nvram_char_to_ascii("","ipsec_ca_5"); %>');

var ipsec_client_list_array = "";
var control_profile_flag = true;
var all_profile_subnet_list = "";
var ipsec_server_enable = 0;
var ipsec_type_array = new Array();
window.onresize = function() {
	if(document.getElementById("edit_ipsec_profile_panel").style.display == "block") {
		cal_panel_block("edit_ipsec_profile_panel", 0.25);
	}
} 

/* dotted-quad IP to integer */
function IPv4_dotquadA_to_intA(strbits) {
	var split = strbits.split( '.', 4 );
	var maskConvertInt = (
		parseFloat( split[0] * 16777216 )/* 2^24 */
		+ parseFloat( split[1] * 65536 )/* 2^16 */
		+ parseFloat( split[2] * 256 )/* 2^8  */
		+ parseFloat( split[3] )
	);
	var tempInt = maskConvertInt;

	var maskConvertResult = "";
	for(var i = 31; i >= 0; i --) {
		var result = tempInt - Math.pow(2,i);
		if(result == 0) {
			maskConvertResult = 32 - i;
			break;
		}
		tempInt = result;
	}
	return maskConvertResult;
}

//CIDR to netmask converion
function createNetmaskAddr(bitCount) {
	var mask = [];
	for(var i = 0; i < 4; i += 1) {
		var n = Math.min(bitCount, 8);
		mask.push(256 - Math.pow(2, 8-n));
		bitCount -= n;
	}
	return mask.join('.');
}

function initial(){
	show_menu();

	update_connect_status();
	show_profilelist();

	document.getElementById("ipsec_profile_1").value = ipsec_profile_1;
	document.getElementById("ipsec_profile_2").value = ipsec_profile_2;
	document.getElementById("ipsec_profile_3").value = ipsec_profile_3;
	document.getElementById("ipsec_profile_4").value = ipsec_profile_4;
	document.getElementById("ipsec_profile_5").value = ipsec_profile_5;

	document.getElementById("ipsec_client_list_1").value = ipsec_client_list_1;
	document.getElementById("ipsec_client_list_2").value = ipsec_client_list_2;
	document.getElementById("ipsec_client_list_3").value = ipsec_client_list_3;
	document.getElementById("ipsec_client_list_4").value = ipsec_client_list_4;
	document.getElementById("ipsec_client_list_5").value = ipsec_client_list_5;

	document.getElementById("ipsec_ca_1").value = ipsec_ca_1;
	document.getElementById("ipsec_ca_2").value = ipsec_ca_2;
	document.getElementById("ipsec_ca_3").value = ipsec_ca_3;
	document.getElementById("ipsec_ca_4").value = ipsec_ca_4;
	document.getElementById("ipsec_ca_5").value = ipsec_ca_5;

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

	if(document.general_form.ipsec_server_enable.value == "1") {
		showhide("tr_general_ike_isakmp", 1);
		showhide("tr_general_ike_isakmp_nat", 1);
		showhide("tr_general_log", 1);
		showhide("ipsec_add_profile_bg", 1);
		showhide("ipsec_server_profile_table", 1);
		showhide("ipsec_profilelist_block", 1);
		showhide("ipsec_apply", 1);
		ipsec_server_enable = 1;
	}
	else{
		showhide("tr_general_ike_isakmp", 0);
		showhide("tr_general_ike_isakmp_nat", 0);
		showhide("tr_general_log", 0);
		showhide("ipsec_add_profile_bg", 0);
		showhide("ipsec_server_profile_table", 0);
		showhide("ipsec_profilelist_block", 0);
		showhide("ipsec_apply", 0);
		ipsec_server_enable = 0;
	}
}
var ipsec_connect_status_array = new Array();
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
				ipsec_connect_status_array[ipsec_connect_status[i][0]] = ipsec_connect_status[i][1];
			}
			show_profilelist();
			setTimeout("update_connect_status();",3000);
		}
	});
}
function del_profile_list(obj) {
	if(!control_profile_flag) {
		alert("Please Deactivate all profile.");
		return true;
	}

	if(!confirm("Are you sure to delete this profile?"))/*untranslated*/
		return false;

	var i = obj.parentNode.parentNode.rowIndex;
	var delRowID = obj.parentNode.parentNode.id;
	if(delRowID == "tr_ipsec_profile_1") {
		ipsec_profile_1 = "";
		ipsec_client_list_1 = "";
		ipsec_ca_1 = "";
	}
	if(delRowID == "tr_ipsec_profile_2") {
		ipsec_profile_2 = "";
		ipsec_client_list_2 = "";
		ipsec_ca_2 = "";
	}
	if(delRowID == "tr_ipsec_profile_3") {
		ipsec_profile_3 = "";
		ipsec_client_list_3 = "";
		ipsec_ca_3 = "";
	}
	if(delRowID == "tr_ipsec_profile_4") {
		ipsec_profile_4 = "";
		ipsec_client_list_4 = "";
		ipsec_ca_4 = "";
	}
	if(delRowID == "tr_ipsec_profile_5") {
		ipsec_profile_5 = "";
		ipsec_client_list_5 = "";
		ipsec_ca_5 = "";
	}
	document.getElementById("ipsec_profilelist_table").deleteRow(i);

	document.list_form.ipsec_profile_1.value = ipsec_profile_1;
	document.list_form.ipsec_profile_2.value = ipsec_profile_2;
	document.list_form.ipsec_profile_3.value = ipsec_profile_3;
	document.list_form.ipsec_profile_4.value = ipsec_profile_4;
	document.list_form.ipsec_profile_5.value = ipsec_profile_5;

	document.list_form.ipsec_client_list_1.value = ipsec_client_list_1;
	document.list_form.ipsec_client_list_2.value = ipsec_client_list_2;
	document.list_form.ipsec_client_list_3.value = ipsec_client_list_3;
	document.list_form.ipsec_client_list_4.value = ipsec_client_list_4;
	document.list_form.ipsec_client_list_5.value = ipsec_client_list_5;

	document.list_form.ipsec_ca_1.value = ipsec_ca_1;
	document.list_form.ipsec_ca_2.value = ipsec_ca_2;
	document.list_form.ipsec_ca_3.value = ipsec_ca_3;
	document.list_form.ipsec_ca_4.value = ipsec_ca_4;
	document.list_form.ipsec_ca_5.value = ipsec_ca_5;

	document.form.ipsec_profile_1.value = ipsec_profile_1;
	document.form.ipsec_profile_2.value = ipsec_profile_2;
	document.form.ipsec_profile_3.value = ipsec_profile_3;
	document.form.ipsec_profile_4.value = ipsec_profile_4;
	document.form.ipsec_profile_5.value = ipsec_profile_5;

	document.form.ipsec_client_list_1.value = ipsec_client_list_1;
	document.form.ipsec_client_list_2.value = ipsec_client_list_2;
	document.form.ipsec_client_list_3.value = ipsec_client_list_3;
	document.form.ipsec_client_list_4.value = ipsec_client_list_4;
	document.form.ipsec_client_list_5.value = ipsec_client_list_5;

	document.form.ipsec_ca_1.value = ipsec_ca_1;
	document.form.ipsec_ca_2.value = ipsec_ca_2;
	document.form.ipsec_ca_3.value = ipsec_ca_3;
	document.form.ipsec_ca_4.value = ipsec_ca_4;
	document.form.ipsec_ca_5.value = ipsec_ca_5;

	document.list_form.submit();
	show_profilelist();
}
function convertVPNType(type) {
	var vpnType = "";
	switch (type) {
		case "1" :
			vpnType = "Net-to-Net VPN Server";/*untranslated*/
			break;
		case "2" :
			vpnType = "Net-to-Net VPN Client";/*untranslated*/
			break;
		case "3" :
			vpnType = "Net-to-Net VPN Peer";/*untranslated*/
			break;
		case "4" :
			vpnType = "Host-to-Net VPN Server";/*untranslated*/
			break;
	}
	return vpnType;
}
function connect_Row(rowdata, profileName, flag) {
	var orgProfile = "";
	var tempProfile = "";
	var state = 0;
	var actionScript = "ipsec_stop";
	if(flag == "active") {
		state = 1;
		actionScript = "ipsec_start";
	}

	switch (profileName) {
		case "ipsec_profile_1": 
			orgProfile = document.form.ipsec_profile_1.value;
			tempProfile = orgProfile.substring(0, (orgProfile.length - 2)) + ">" + state;
			ipsec_profile_1 = tempProfile;
			document.form.ipsec_profile_1.value = tempProfile;
			break;
		case "ipsec_profile_2": 
			orgProfile = document.form.ipsec_profile_2.value;
			tempProfile = orgProfile.substring(0, (orgProfile.length - 2)) + ">" + state;
			ipsec_profile_2 = tempProfile;
			document.form.ipsec_profile_2.value = tempProfile;
			break;
		case "ipsec_profile_3": 
			orgProfile = document.form.ipsec_profile_3.value;
			tempProfile = orgProfile.substring(0, (orgProfile.length - 2)) + ">" + state;
			ipsec_profile_3 = tempProfile;
			document.form.ipsec_profile_3.value = tempProfile;
			break;
		case "ipsec_profile_4": 
			orgProfile = document.form.ipsec_profile_4.value;
			tempProfile = orgProfile.substring(0, (orgProfile.length - 2)) + ">" + state;
			ipsec_profile_4 = tempProfile;
			document.form.ipsec_profile_4.value = tempProfile;
			break;
		case "ipsec_profile_5": 
			orgProfile = document.form.ipsec_profile_5.value;
			tempProfile = orgProfile.substring(0, (orgProfile.length - 2)) + ">" + state;
			ipsec_profile_5 = tempProfile;
			document.form.ipsec_profile_5.value = tempProfile;
			break;
	}
	document.form.action_script.value = actionScript;
	rowdata.parentNode.innerHTML = "<img src='/images/InternetScan.gif'>";
	document.form.submit();
}
function close_connect_status() {
	$("#connection_ipsec_profile_panel").fadeOut(300);
}
function showIPSecClients(profileName, e) {
	$("#connection_ipsec_profile_panel").fadeIn(300);
	$("#connection_ipsec_profile_panel").css("position", "absolute");
	$("#connection_ipsec_profile_panel").css("top", (e.pageY + 10) + "px");
	$("#connection_ipsec_profile_panel").css("left", (e.pageX - 50) + "px");
	
	var html = "";
	html += "<div class='ipsec_connect_status_title_bg'>";
	html += "<div class='ipsec_connect_status_title' style='width:240px;'>Remote IP</div>";/*untranslated*/
	html += "<div class='ipsec_connect_status_title'><#statusTitle_Client#></div>";
	html += "<div class='ipsec_connect_status_title'><#Access_Time#></div>";
	html += "<div class='ipsec_connect_status_title'>XAUTHUSER</div>";/*untranslated*/
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
function show_profilelist() {
	ipsec_type_array = {"type_4" : ["Host-to-Net VPN Server", "4"], "type_1" : ["Net-to-Net VPN Server", "1"]};//untranslated
	all_profile_subnet_list = "";
	control_profile_flag = true;
	var ipsec_profilelist_arraylist = new Array();
	var temp_array = [];

	var push_to_profilelist_array = function(oriArray, profileIndex) {
		temp_array = [];
		if(oriArray != "") {
			temp_array = oriArray.split(">");
			temp_array.unshift(profileIndex);
			ipsec_profilelist_arraylist.push(temp_array);
		}
	};
	push_to_profilelist_array(ipsec_profile_1, "ipsec_profile_1");
	push_to_profilelist_array(ipsec_profile_2, "ipsec_profile_2");
	push_to_profilelist_array(ipsec_profile_3, "ipsec_profile_3");
	push_to_profilelist_array(ipsec_profile_4, "ipsec_profile_4");
	push_to_profilelist_array(ipsec_profile_5, "ipsec_profile_5");

	var code = "";
	var arrayLength = ipsec_profilelist_arraylist.length;
	code +='<table width="100%" cellspacing="0" cellpadding="4" align="center" class="list_table" id="ipsec_profilelist_table">';
	if(arrayLength == 0)
		code +='<tr><td style="color:#FFCC00;" colspan="5"><#IPConnection_VSList_Norule#></td></tr>';
	else{
		for(var i = 0; i < arrayLength; i += 1) {
			//if(ipsec_profilelist_arraylist[i].length == 40) {
				code +='<tr id=tr_' + ipsec_profilelist_arraylist[i][0] + '>';
				if(ipsec_profilelist_arraylist[i][38] == 0) {
					code +='<td width="15%">-</td>';
				}
				else {
					if(ipsec_profilelist_arraylist[i][1] == "1" || ipsec_profilelist_arraylist[i][1] == "3" || ipsec_profilelist_arraylist[i][1] == "4") {	//Server status
						if(ipsec_connect_status_array[ipsec_profilelist_arraylist[i][2]]) {
							var connected_count = (ipsec_connect_status_array[ipsec_profilelist_arraylist[i][2]].split("<").length - 1);
							code +='<td width="15%"><a class="hintstyle2" href="javascript:void(0);" onClick="showIPSecClients(\''+ipsec_profilelist_arraylist[i][2]+'\', event);">';
							code +='<#btn_Enabled#>(' + connected_count + ')</a></td>';
						}
						else {
							code +='<td width="15%">-</td>';
						}
					}
					else { //Client status
						if(ipsec_connect_status_array[ipsec_profilelist_arraylist[i][2]]) {
							var connect_status = ipsec_connect_status_array[ipsec_profilelist_arraylist[i][2]].split("<")[1].split(">")[1];
							if(connect_status == "1") {
								code +='<td width="15%" title="<#Connected#>"><img src="/images/checked_parentctrl.png" style="width:25px;"></td>';
							}
							else {
								code +='<td width="15%" title="<#Connecting_str#>"><img src="/images/InternetScan.gif" style="width:25px;"></td>';
							}
						}
						else {
							code +='<td width="15%" title="<#ConnectionFailed#>"><img src="/images/button-close2.png" style="width:25px;cursor:pointer;" onclick="viewLog()"></td>';
						}
					}
				}
				code +='<td width="26%">' + ipsec_profilelist_arraylist[i][2] + '</td>';
				code +='<td width="25%">' + convertVPNType(ipsec_profilelist_arraylist[i][1]) + '</td>';
				delete ipsec_type_array["type_" + ipsec_profilelist_arraylist[i][1]];
				code +='<td width="7%"><input class="edit_btn" onclick="editIPSecProfile(\''+ ipsec_profilelist_arraylist[i][0] +'\');" value=""/></td>';
				code +='<td width="7%"><input class="remove_btn" onclick="del_profile_list(this);" value=""/></td>';
				code +='<td width="20%">';
				if(ipsec_profilelist_arraylist[i][38] == 0) {
					code += '<input class="button_gen" type="button" onClick="connect_Row(this, \''+ipsec_profilelist_arraylist[i][0]+'\', \'active\');" value="<#CTL_Activate#>">';
				}
				else {
					code += '<input class="button_gen" type="button" onClick="connect_Row(this, \''+ipsec_profilelist_arraylist[i][0]+'\', \'deactivate\');" value="<#CTL_Deactivate#>">';
					control_profile_flag = false;
				}
				code +='</td>';
				code +='</tr>';
			//}
			all_profile_subnet_list += ">" + ipsec_profilelist_arraylist[i][11];
		}
	}
	code +='</table>';
	document.getElementById("ipsec_profilelist_block").innerHTML = code;
}
function clear_subnet_input(_type) {
	var subnet_node = document.getElementById("td_net_" + _type + "_private_subnet");
	while (subnet_node.firstChild) {
		subnet_node.removeChild(subnet_node.firstChild);
	}
}
function gen_subnet_input(_type, _idx, _value) {
	var subnet_input_obj = document.createElement("input");
	subnet_input_obj.type = "text";
	subnet_input_obj.className = "input_25_table";
	subnet_input_obj.id = "ipsec_" + _type + "_subnet_" + _idx;
	subnet_input_obj.value = _value;
	if(subnetIP_support_IPv6)
		subnet_input_obj.maxLength = "39";
	else {
		subnet_input_obj.maxLength = "18";
		subnet_input_obj.onkeypress = function() {
			return validator.isIPAddrPlusNetmask(this,event);
		};
	}
	subnet_input_obj.style.marginTop = "4px";
	return subnet_input_obj;
}
function gen_subnet_add(_type) {
	var subnet_input_obj = document.createElement("input");
	subnet_input_obj.type = "text";
	subnet_input_obj.className = "add_btn";
	subnet_input_obj.style.height = "27px";
	subnet_input_obj.onclick = function() { add_subnet_item(this, _type);};
	return subnet_input_obj;
}
function gen_subnet_hint() {
	var subnet_input_obj = document.createElement("span");
	subnet_input_obj.innerHTML = " (ex.10.10.10.0/24)";
	subnet_input_obj.style.color = "#FC0";
	return subnet_input_obj;
}
function gen_subnet_del(_type) {
	var subnet_input_obj = document.createElement("input");
	subnet_input_obj.id = "btDelRemoteSubnet_" + _type;
	subnet_input_obj.type = "text";
	subnet_input_obj.className = "remove_btn";
	subnet_input_obj.style.height = "27px";
	subnet_input_obj.onclick = function() { del_subnet_item(this, _type);};
	return subnet_input_obj;
}
function initialIPSecProfile() {
	document.getElementById("ipsec_vpn_type_view").style.display = "none";
	document.form.ipsec_vpn_type.style.display = "";
	document.form.ipsec_profilename.value = "";
	settingRadioItemCheck(document.form.ipsec_remote_gateway_method, "0");
	document.form.ipsec_remote_gateway.value = "";
	document.form.ipsec_local_public_interface.value = "wan";
	settingRadioItemCheck(document.form.ipsec_auth_method, "1");
	document.form.edit_ipsec_ca.value = "";
	document.form.ipsec_preshared_key.value = "";
	settingRadioItemCheck(document.form.ipsec_exchange, "0");
	changeExchangeMode();
	document.form.ipsec_local_id.value = "";
	document.form.ipsec_remote_id.value = "";

	clear_subnet_input("local");
	document.getElementById("td_net_local_private_subnet").appendChild(gen_subnet_input("local", 1, ""));
	document.getElementById("td_net_local_private_subnet").appendChild(gen_subnet_add("local"));
	document.getElementById("td_net_local_private_subnet").appendChild(gen_subnet_hint());
	document.form.ipsec_local_port.value = "0";

	clear_subnet_input("remote");
	document.getElementById("td_net_remote_private_subnet").appendChild(gen_subnet_input("remote", 1, ""));
	document.getElementById("td_net_remote_private_subnet").appendChild(gen_subnet_add("remote"));
	document.getElementById("td_net_remote_private_subnet").appendChild(gen_subnet_hint());

	document.form.ipsec_remote_port.value = "0";
	settingRadioItemCheck(document.form.ipsec_virtual_ip, "1");
	document.form.ipsec_virtual_subnet.value = "";

	settingRadioItemCheck(document.form.ipsec_ike, "auto");

	document.form.ipsec_encryption_p1.value = "auto";
	document.form.ipsec_hash_p1.value = "auto";
	document.form.ipsec_keylife_p1.value = "172800";
	settingRadioItemCheck(document.form.ipsec_xauth, "0");
	document.form.ipsec_xauth_account.value = "";
	document.form.ipsec_xauth_password.value = "";
	document.form.ipsec_xauth_server_type.value = "eap-md5";
	settingRadioItemCheck(document.form.ipsec_traversal, "1");
	document.form.ipsec_ike_isakmp.value = "500";
	document.form.ipsec_ike_isakmp_nat.value = "4500";
	document.form.ipsec_dpd.value = "10";
	settingRadioItemCheck(document.form.ipsec_dead_peer_detection, "1");
	changeAdvDeadPeerDetection(document.form.ipsec_dead_peer_detection);
	document.form.ipsec_encryption_p2.value = "auto";
	document.form.ipsec_hash_p2.value = "auto";
	document.form.ipsec_keylife_p2.value = "3600";
	document.form.ipsec_keyingtries.value = "3";

	ipsec_client_list_array = "";
	document.getElementById("tbAccountList").style.display = "none";
	document.getElementById("ipsec_client_list_Block").style.display = "none";

	document.form.ipsec_client_list_username.value = "";
	document.form.ipsec_client_list_password.value = "";

	document.form.ipsec_dns1.value = "";
	document.form.ipsec_dns2.value = "";
	document.form.ipsec_wins1.value = "";
	document.form.ipsec_wins2.value = "";
}
function UpdatePSecProfile(array) {
	document.getElementById("ipsec_vpn_type_view").style.display = "";
	document.form.ipsec_vpn_type.style.display = "none";
	document.getElementById("ipsec_vpn_type_view").innerHTML = convertVPNType(array[1]);
	document.form.ipsec_profilename.value = array[2];
	settingRadioItemCheck(document.form.ipsec_remote_gateway_method, array[3]);
	document.form.ipsec_remote_gateway.value = array[4];
	document.form.ipsec_local_public_interface.value = array[5];
	settingRadioItemCheck(document.form.ipsec_auth_method, array[7]);

	if(array[7] == "0") {
		showhide("tr_importCA", 1);
		showhide("tr_editCA", 1);
		showhide("tr_presharedKey", 0);
	}
	else {
		showhide("tr_presharedKey", 1);
		document.form.ipsec_preshared_key.value = array[8];
	}
	switch (array[0]) {
		case "ipsec_profile_1": 
			document.form.edit_ipsec_ca.value = ipsec_ca_1;
			break;
		case "ipsec_profile_2": 
			document.form.edit_ipsec_ca.value = ipsec_ca_2;
			break;
		case "ipsec_profile_3": 
			document.form.edit_ipsec_ca.value = ipsec_ca_3;
			break;
		case "ipsec_profile_4": 
			document.form.edit_ipsec_ca.value = ipsec_ca_3;
			break;
		case "ipsec_profile_5": 
			document.form.edit_ipsec_ca.value = ipsec_ca_3;
			break;
	}
	
	settingRadioItemCheck(document.form.ipsec_exchange, array[20]);
	changeExchangeMode();
	document.form.ipsec_local_id.value = array[21];
	document.form.ipsec_remote_id.value = array[22];
	
	clear_subnet_input("local");
	if( array[9] != undefined) {
		var local_subnet = array[9].split("<");
		var local_subnet_idx = 1;
		for(var i = 0; i < local_subnet.length; i += 1) {
			if(local_subnet[i] != "") {
				document.getElementById("td_net_local_private_subnet").appendChild(gen_subnet_input("local", local_subnet_idx, local_subnet[i]));
				if(local_subnet_idx == 1) {
					document.getElementById("td_net_local_private_subnet").appendChild(gen_subnet_add("local"));
					document.getElementById("td_net_local_private_subnet").appendChild(gen_subnet_hint());
				}
				local_subnet_idx++;
			}
		}
		if(local_subnet_idx > 2) {
			document.getElementById("td_net_local_private_subnet").appendChild(gen_subnet_del("local"));
		}
	}
	document.form.ipsec_local_port.value = array[10];

	clear_subnet_input("remote");
	if( array[11] != undefined) {
		var remote_subnet = array[11].split("<");
		var remote_subnet_idx = 1;
		for(var i = 0; i < remote_subnet.length; i += 1) {
			if(remote_subnet[i] != "") {
				document.getElementById("td_net_remote_private_subnet").appendChild(gen_subnet_input("remote", remote_subnet_idx, remote_subnet[i]));
				if(remote_subnet_idx == 1) {
					document.getElementById("td_net_remote_private_subnet").appendChild(gen_subnet_add("remote"));
					document.getElementById("td_net_remote_private_subnet").appendChild(gen_subnet_hint());
				}
				remote_subnet_idx++;
			}
		}
		if(remote_subnet_idx > 2) {
			document.getElementById("td_net_remote_private_subnet").appendChild(gen_subnet_del("remote"));
		}
	}
	document.form.ipsec_remote_port.value = array[12];
	settingRadioItemCheck(document.form.ipsec_virtual_ip, array[14]);
	if(array[14] == "0") {
		showhide("tr_net_virtual_subnet", 0);
	}
	document.form.ipsec_virtual_subnet.value = array[15];

	settingRadioItemCheck(document.form.ipsec_ike, array[17]);
	if(array[1] == "3" || array[1] == "4") {
		if(array[17] == "1") {
			showhide("tr_adv_nat_xauth", 1);
			showhide("tr_adv_nat_xauth_server_type", 0);
			document.getElementById("th_xauth_title").innerHTML = "XAUTH";/*untranslated*/
			document.getElementById("th_xauth_account_title").innerHTML = "XAUTH account";/*untranslated*/
			document.getElementById("th_xauth_password_title").innerHTML = "XAUTH password";/*untranslated*/
		}
		else if(array[17] == "2") {
			showhide("tr_adv_nat_xauth", 1);
			if(getRadioItemCheck(document.form.ipsec_auth_method) == "1") {	//PSK
				showhide("tr_adv_nat_xauth_server_type", 0);
			}
			else {
				showhide("tr_adv_nat_xauth_server_type", 1);
			}
			document.getElementById("th_xauth_title").innerHTML = "User authentication role";/*untranslated*/
			document.getElementById("th_xauth_account_title").innerHTML = "User authentication account";/*untranslated*/
			document.getElementById("th_xauth_password_title").innerHTML = "User authentication password";/*untranslated*/
			document.getElementById("th_xauth_server_type_title").innerHTML = "User authentication method";/*untranslated*/
		}
	}
	changeIKEVersion();
	document.form.ipsec_encryption_p1.value = array[18];
	document.form.ipsec_hash_p1.value = array[19];
	document.form.ipsec_keylife_p1.value = array[23];
	settingRadioItemCheck(document.form.ipsec_xauth, array[24]);
	if(array[24] == "2" && array[17] != "auto") {
		showhide("tr_adv_nat_xauth_account", 1);
		showhide("tr_adv_nat_xauth_password", 1);
	}
	document.form.ipsec_xauth_account.value = array[25];
	document.form.ipsec_xauth_password.value = array[26];
	document.form.ipsec_xauth_server_type.value = array[27];
	settingRadioItemCheck(document.form.ipsec_traversal, array[28]);
	if(array[28] == "0") {
		showhide("tr_adv_ike_isakmp", 0);
		showhide("tr_adv_ike_isakmp_nat", 0);
	}
	document.form.ipsec_ike_isakmp.value = array[29];
	document.form.ipsec_ike_isakmp_nat.value = array[30];
	document.form.ipsec_dpd.value = (array[31] == "null") ? 10 : array[31];
	settingRadioItemCheck(document.form.ipsec_dead_peer_detection, (array[32] == "null") ? 1 : array[32]);
	if(array[32] == "0") {
		showhide("tr_adv_dpd_interval", 0);
	}
	document.form.ipsec_encryption_p2.value = array[33];
	document.form.ipsec_hash_p2.value = array[34]
	document.form.ipsec_keylife_p2.value = array[35];
	document.form.ipsec_keyingtries.value = array[36];

	if(array[1] == "4") {
		var ipsec_samba_array = array[37].split("<");
		document.form.ipsec_dns1.value = ipsec_samba_array[1];
		document.form.ipsec_dns2.value = ipsec_samba_array[2];
		document.form.ipsec_wins1.value = ipsec_samba_array[3];
		document.form.ipsec_wins2.value = ipsec_samba_array[4];
	}

	switch(document.form.ipsec_profile_item.value) {
		case "ipsec_profile_1" :
			ipsec_client_list_array = ipsec_client_list_1;
			break;
		case "ipsec_profile_2" :
			ipsec_client_list_array = ipsec_client_list_2;
			break;
		case "ipsec_profile_3" :
			ipsec_client_list_array = ipsec_client_list_3;
			break;
		case "ipsec_profile_4" :
			ipsec_client_list_array = ipsec_client_list_4;
			break;
		case "ipsec_profile_5" :
			ipsec_client_list_array = ipsec_client_list_5;
			break;
	}
	showipsec_clientlist();
}
function createIPSecProfile() {
	if(!control_profile_flag) {
		alert("Please Deactivate all profile.");
		return true;
	}
	if(document.getElementById("ipsec_profilelist_table").rows.length == 2) {
		alert("<#JS_itemlimit1#> 2 <#JS_itemlimit2#>");
		return true;
	}
	$("#edit_ipsec_profile_panel").fadeIn(300);
	cal_panel_block("edit_ipsec_profile_panel", 0.25);

	if(ipsec_profile_1 == "")
		document.form.ipsec_profile_item.value = "ipsec_profile_1";
	else if(ipsec_profile_2 == "")
		document.form.ipsec_profile_item.value = "ipsec_profile_2";
	else if(ipsec_profile_3 == "")
		document.form.ipsec_profile_item.value = "ipsec_profile_3";
	else if(ipsec_profile_4 == "")
		document.form.ipsec_profile_item.value = "ipsec_profile_4";
	else if(ipsec_profile_5 == "")
		document.form.ipsec_profile_item.value = "ipsec_profile_5";

	switch (document.form.ipsec_profile_item.value) {
		case "ipsec_profile_1":
			ipsec_client_list_array = ipsec_client_list_1;
			break;
		case "ipsec_profile_2":
			ipsec_client_list_array = ipsec_client_list_2;
			break;
		case "ipsec_profile_3":
			ipsec_client_list_array = ipsec_client_list_3;
			break;
		case "ipsec_profile_4":
			ipsec_client_list_array = ipsec_client_list_4;
			break;
		case "ipsec_profile_5":
			ipsec_client_list_array = ipsec_client_list_5;
			break;
	}
	free_options(document.form.ipsec_vpn_type);
	Object.keys(ipsec_type_array).forEach(function(key) {
		var type_text = ipsec_type_array[key][0];
		var type_value = ipsec_type_array[key][1];
		add_option(document.form.ipsec_vpn_type, type_text, type_value);
	});
	document.form.ipsec_vpn_type.selectedIndex = 0;
	switchMode(document.form.ipsec_vpn_type.value);
}
function editIPSecProfile(mode) {
	if(!control_profile_flag) {
		alert("Please Deactivate all profile.");
		return true;
	}
	$("#edit_ipsec_profile_panel").fadeIn(300);
	cal_panel_block("edit_ipsec_profile_panel", 0.25);
	
	var editProfileArray = [];
	switch (mode) {
		case "ipsec_profile_1" :
			editProfileArray = ipsec_profile_1.split(">");
			editProfileArray.unshift("ipsec_profile_1");
			document.form.ipsec_profile_item.value = "ipsec_profile_1";
			break;
		case "ipsec_profile_2" :
			editProfileArray = ipsec_profile_2.split(">");
			editProfileArray.unshift("ipsec_profile_2");
			document.form.ipsec_profile_item.value = "ipsec_profile_2";
			break;
		case "ipsec_profile_3" :
			editProfileArray = ipsec_profile_3.split(">");
			editProfileArray.unshift("ipsec_profile_3");
			document.form.ipsec_profile_item.value = "ipsec_profile_3";
			break;
		case "ipsec_profile_4" :
			editProfileArray = ipsec_profile_4.split(">");
			editProfileArray.unshift("ipsec_profile_4");
			document.form.ipsec_profile_item.value = "ipsec_profile_4";
			break;
		case "ipsec_profile_5" :
			editProfileArray = ipsec_profile_5.split(">");
			editProfileArray.unshift("ipsec_profile_5");
			document.form.ipsec_profile_item.value = "ipsec_profile_5";
			break;
	}
	var ipsec_type_array_edit = {"type_4" : ["Host-to-Net VPN Server", "4"], "type_1" : ["Net-to-Net VPN Server", "1"]};//untranslated
	free_options(document.form.ipsec_vpn_type);
	Object.keys(ipsec_type_array_edit).forEach(function(key) {
		var type_text = ipsec_type_array_edit[key][0];
		var type_value = ipsec_type_array_edit[key][1];
		add_option(document.form.ipsec_vpn_type, type_text, type_value);
	});
	switchMode(editProfileArray[1]);
	UpdatePSecProfile(editProfileArray);
}
function cancel_ipsec_profile_panel() {
	$("#edit_ipsec_profile_panel").fadeOut(300);
}

function checkGatewayIP(_lanIPAddr, _lanNetMask) {
	var lanIPAddr = _lanIPAddr;
	var lanNetMask = _lanNetMask;
	var ipConflict;
	var alertMsg = function (type, ipAddr, netStart, netEnd) {
		alert("*Conflict with " + type + " IP: " + ipAddr + ",\n" + "Network segment is " + netStart + " ~ " + netEnd);
	};

	//1.check Wan IP
	ipConflict = checkIPConflict("WAN", lanIPAddr, lanNetMask);
	if(ipConflict.state) {
		alertMsg("WAN", ipConflict.ipAddr, ipConflict.netLegalRangeStart, ipConflict.netLegalRangeEnd);
		return false;
	}

	//2.check PPTP
	if(pptpd_support) {
		ipConflict = checkIPConflict("PPTP", lanIPAddr, lanNetMask);
		if(ipConflict.state) {
			alertMsg("PPTP", ipConflict.ipAddr, ipConflict.netLegalRangeStart, ipConflict.netLegalRangeEnd);
			return false;
		}
	}

	//3.check OpenVPN
	if(openvpnd_support) {
		ipConflict = checkIPConflict("OpenVPN", lanIPAddr, lanNetMask);
		if(ipConflict.state) {
			alertMsg("OpenVPN", ipConflict.ipAddr, ipConflict.netLegalRangeStart, ipConflict.netLegalRangeEnd);
			return false;
		}
	}

	return true;
}

function validForm() {
	if(!validator.isEmpty(document.form.ipsec_profilename))
		return false;
	if(!Block_chars(document.form.ipsec_profilename, [">", "<"]))
		return false;
	if(!validator.isContainblanksStr(document.form.ipsec_profilename)) {
		return false;
	}
	var checkDuplicateProfileName = function() {
		var dup_flag = false;
		for(var i = 1; i < 6; i += 1) {
			if(document.form.ipsec_profile_item.value != ("ipsec_profile_" + i)) {
				var profileName = "";
				switch (("ipsec_profile_" + i)) {
					case "ipsec_profile_1" :
						if(ipsec_profile_1 != "")
							profileName = ipsec_profile_1.split(">")[1];
						break;
					case "ipsec_profile_2" :
						if(ipsec_profile_2 != "")
							profileName = ipsec_profile_2.split(">")[1];
						break;
					case "ipsec_profile_3" :
						if(ipsec_profile_3 != "")
							profileName = ipsec_profile_3.split(">")[1];
						break;
					case "ipsec_profile_4" :
						if(ipsec_profile_4 != "")
							profileName = ipsec_profile_4.split(">")[1];
						break;
					case "ipsec_profile_5" :
						if(ipsec_profile_5 != "")
							profileName = ipsec_profile_5.split(">")[1];
						break;
				}
				if(profileName == document.form.ipsec_profilename.value) {
					alert("<#JS_duplicate#>");
					document.form.ipsec_profilename.focus();
					document.form.ipsec_profilename.select();
					dup_flag = true;
					break;
				}
			}
		}
		return dup_flag;
	};
	if(checkDuplicateProfileName()) {
		return false;
	}
	
	if(getRadioItemCheck(document.form.ipsec_auth_method) == "1") {
		if(!validator.isEmpty(document.form.ipsec_preshared_key))
			return false;
		if(!Block_chars(document.form.ipsec_preshared_key, [">", "<", "#", "null"]))
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
	}

	if(document.form.ipsec_vpn_type.value != "4") {
		var valid_subnet = function(_type) {
			var existSubnetItem = document.getElementById("tr_net_" + _type + "_private_subnet").getElementsByClassName("input_25_table").length;
			var existSubnetItemList = "";
			var existSubnetObj = "";
			var is_ipv4 = false;
			var is_ipv6 = false;
			//var all_profile_subnet_list_array = all_profile_subnet_list.split(">");
			for(var i = 1 ; i <= existSubnetItem; i += 1) {
				existSubnetObj = document.getElementById("ipsec_" + _type + "_subnet_" + i);
				is_ipv4 = (existSubnetObj.value.indexOf(".") != -1) ? true : false;
				if(subnetIP_support_IPv6)
					is_ipv6 = (existSubnetObj.value.indexOf(":") != -1) ? true : false;
				if(!is_ipv4 && !is_ipv6) {
					alert(existSubnetObj.value + "<#JS_validip#>");
					existSubnetObj.focus();
					return false;
				}

				if(is_ipv4) {
					if(!validator.isLegalIPAndMask(existSubnetObj)) {
						return false;
					}

					var subnetIP = existSubnetObj.value.split("/")[0];
					var maskCIDR = parseInt(existSubnetObj.value.split("/")[1], 10);
					if (isNaN(maskCIDR) || maskCIDR != 24){
						alert("Mask address must be 24.");/*untranslated*/
						existSubnetObj.focus();
						existSubnetObj.select();
						return false;
					}
					var subnetMask = createNetmaskAddr(maskCIDR);
					if(!checkGatewayIP(subnetIP, subnetMask)) {
						existSubnetObj.focus();
						existSubnetObj.select();
						return false;
					}
				}
				else if(is_ipv6) {
					if(!validator.isLegal_ipv6(existSubnetObj)) {
						return false;
					}
				}
				
				if(existSubnetItemList.search(existSubnetObj.value.split("/")[0]) != -1) {
					alert("Conflict with other subnet.");/*untranslated*/
					existSubnetObj.focus();
					existSubnetObj.select();
					return false;
				}
				existSubnetItemList += document.getElementById("ipsec_" + _type + "_subnet_" + i).value + ">";
				
				/*
				for(var j = 0 ; j < all_profile_subnet_list_array.length; j += 1) {
					if(j != document.form.ipsec_profile_item.value.split("_")[2]) {
						if(all_profile_subnet_list_array[j].search(existSubnetObj.value.split("/")[0]) != -1) {
							alert("Conflict with profile "+j+" subnet.");
							existSubnetObj.focus();
							return false;
						}
					}
				}
				*/
			}
			return true;
		};

		if(!valid_subnet("local"))
			return false;

		if(!validator.numberRange(document.form.ipsec_local_port, 0, 65535)) {
			return false;
		}
		if(!validator.numberRange(document.form.ipsec_remote_port, 0, 65535)) {
			return false;
		}
		if(!validator.numberRange(document.form.ipsec_keylife_p1, 120, 172800)) {
			return false;
		}
		if(!validator.numberRange(document.form.ipsec_dpd, 10, 900)) {
			return false;
		}
		if(!validator.numberRange(document.form.ipsec_keylife_p2, 120, 172800)) {
			return false;
		}

		if(parseInt(document.form.ipsec_keylife_p2.value) > parseInt(document.form.ipsec_keylife_p1.value)) {
			alert("The phase 2 IKE keylift time can not be greater than phase 1 IKE keylift time");/*untranslated*/
			document.form.ipsec_keylife_p2.focus();
			return false;
		}

		if(!Block_chars(document.form.ipsec_local_id, [">", "<", "null"]))
			return false;
		if(!Block_chars(document.form.ipsec_remote_id, [">", "<", "null"]))
			return false;

		if(!valid_subnet("remote"))
			return false;
		
	}
	else {
		if(getRadioItemCheck(document.form.ipsec_virtual_ip) == "1") {
			if(!validator.isLegalIPAndMask(document.form.ipsec_virtual_subnet)) {
				return false;
			}
			var subnetIP = document.form.ipsec_virtual_subnet.value.split("/")[0];
			var maskCIDR = parseInt(document.form.ipsec_virtual_subnet.value.split("/")[1], 10);
			if (isNaN(maskCIDR) || maskCIDR != 24){
				alert("Mask address must be 24.");/*untranslated*/
				document.form.ipsec_virtual_subnet.focus();
				document.form.ipsec_virtual_subnet.select();
				return false;
			}
			var subnetMask = createNetmaskAddr(maskCIDR);
			if(!checkGatewayIP(subnetIP, subnetMask)) {
				document.form.ipsec_virtual_subnet.focus();
				document.form.ipsec_virtual_subnet.select();
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
		if(!validator.numberRange(document.form.ipsec_dpd, 10, 900)) {
			return false;
		}
	}
	if(document.form.ipsec_vpn_type.value == "2" || document.form.ipsec_vpn_type.value == "2") {
		if(!Block_chars(document.form.ipsec_remote_gateway, [">", "<"]))
			return false;
	}
	if(getRadioItemCheck(document.form.ipsec_xauth) == "2" && document.form.ipsec_vpn_type.value == "2") {
		if(!validator.isEmpty(document.form.ipsec_xauth_account))
			return false;
		if(!Block_chars(document.form.ipsec_xauth_account, [">", "<"]))
			return false;
		if(!validator.isEmpty(document.form.ipsec_xauth_password))
			return false;
		if(!Block_chars(document.form.ipsec_xauth_password, [">", "<"]))
			return false;
	}

	return true;
}
function save_ipsec_profile_panel() {

	if(validForm()) {
		$("#edit_ipsec_profile_panel").fadeOut(300);
		document.form.action = "/start_apply.htm";
		document.form.enctype = "application/x-www-form-urlencoded";
		document.form.encoding = "application/x-www-form-urlencoded";
		document.form.action_script.value = "saveNvram";
		document.form.file.value = "";

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

		var result = "";

		var accessible_networks = "null";
		var get_subnet_list = function(_type) {
			var subnet_list = "";
			var existSubnetItem = document.getElementById("tr_net_" + _type + "_private_subnet").getElementsByClassName("input_25_table").length;
			for(var i = 1 ; i <= existSubnetItem; i += 1) {
				subnet_list += "<" + document.getElementById("ipsec_" + _type + "_subnet_" + i).value;
			}
			return subnet_list;
		};
		var local_subnet_list = "";
		var remote_subnet_list = "";
		if(document.form.ipsec_vpn_type.value != "4") {
			local_subnet_list = get_subnet_list("local");
			remote_subnet_list = get_subnet_list("remote");
		}

		var local_public_ip = "";
		/*
		var wans_dualwan_array = '<% nvram_get("wans_dualwan"); %>'.split(" ");
		var wans_index = 0;
		for(var i = 0; i < wans_dualwan_array.length; i += 1) {
			if(wans_dualwan_array[i] == document.form.ipsec_local_public_interface.value) {
				wans_index = i + 1;
			}
		}
		switch(wans_index) {
			case 0 :
				local_public_ip = "0.0.0.0";
				break;
			case 1 :
				local_public_ip = wanlink_ipaddr();
				break;
			case 2 :
				local_public_ip = secondary_wanlink_ipaddr();
				break;
		}
		*/

		var auth_method_vaule = (getRadioItemCheck(document.form.ipsec_auth_method) == 1) ? document.form.ipsec_preshared_key.value : "cafile";
		var samba_list = "<" + document.form.ipsec_dns1.value + "<" + document.form.ipsec_dns2.value + "<" + document.form.ipsec_wins1.value + "<" + document.form.ipsec_wins2.value;
		var profile_array = [ "", 
			document.form.ipsec_vpn_type.value, document.form.ipsec_profilename.value, getRadioItemCheck(document.form.ipsec_remote_gateway_method), document.form.ipsec_remote_gateway.value, 
			document.form.ipsec_local_public_interface.value, local_public_ip, getRadioItemCheck(document.form.ipsec_auth_method), auth_method_vaule, 
			local_subnet_list, document.form.ipsec_local_port.value, remote_subnet_list, document.form.ipsec_remote_port.value, 
			"tunnel", getRadioItemCheck(document.form.ipsec_virtual_ip), document.form.ipsec_virtual_subnet.value, accessible_networks, 
			getRadioItemCheck(document.form.ipsec_ike), document.form.ipsec_encryption_p1.value, document.form.ipsec_hash_p1.value, getRadioItemCheck(document.form.ipsec_exchange), 
			document.form.ipsec_local_id.value, document.form.ipsec_remote_id.value, document.form.ipsec_keylife_p1.value, getRadioItemCheck(document.form.ipsec_xauth), 
			document.form.ipsec_xauth_account.value, document.form.ipsec_xauth_password.value, document.form.ipsec_xauth_server_type.value, getRadioItemCheck(document.form.ipsec_traversal), 
			document.form.ipsec_ike_isakmp.value, document.form.ipsec_ike_isakmp_nat.value, document.form.ipsec_dpd.value, getRadioItemCheck(document.form.ipsec_dead_peer_detection), 
			document.form.ipsec_encryption_p2.value, document.form.ipsec_hash_p2.value, document.form.ipsec_keylife_p2.value, document.form.ipsec_keyingtries.value, 
			samba_list, 0
		];

		switch (profile_array[1]) {
			case "1":
				result = profile_array[1] + ">" + profile_array[2] + ">" + "null" + ">" + "null" + ">" + 
						profile_array[5] + ">" + profile_array[6] +  ">" + profile_array[7] + ">" + profile_array[8] + ">" + 
						profile_array[9] + ">" + profile_array[10] + ">" + profile_array[11] + ">" + profile_array[12] + ">" + 
						profile_array[13] + ">" + "null" + ">" + "null" + ">" + "null" + ">" +
						profile_array[17] + ">" + profile_array[18] + ">" + profile_array[19] + ">" + profile_array[20] + ">" + 
						profile_array[21] + ">" + profile_array[22] + ">" + profile_array[23] + ">" + profile_array[24] + ">" + 
						"null" + ">" + "null" + ">" + profile_array[27] + ">" + profile_array[28] + ">" + 
						profile_array[29] + ">" + profile_array[30] + ">" + profile_array[31] + ">" + profile_array[32] + ">" + 
						profile_array[33] + ">" + profile_array[34] + ">" + profile_array[35] + ">" + profile_array[36] + ">" + 
						"null" + ">" + profile_array[38];
				break;
			case "2":
				result = profile_array[1] + ">" + profile_array[2] + ">" + profile_array[3] + ">" + profile_array[4] + ">" + 
						profile_array[5] + ">" + profile_array[6] + ">" + profile_array[7] + ">" + profile_array[8] + ">" + 
						profile_array[9] + ">" + profile_array[10] + ">" + profile_array[11] + ">" + profile_array[12] + ">" +
						profile_array[13] + ">" + "null" + ">" + "null" + ">" +"null"+ ">" +
						profile_array[17] + ">" + profile_array[18] + ">" + profile_array[19] + ">" + profile_array[20] + ">" + 
						profile_array[21] + ">" + profile_array[22] + ">" + profile_array[23] + ">" + profile_array[24] + ">" + 
						profile_array[25] + ">" + profile_array[26] + ">" + profile_array[27] + ">" + profile_array[28] + ">" + 
						profile_array[29] + ">" + profile_array[30] + ">" + profile_array[31] + ">" + profile_array[32] + ">" + 
						profile_array[33] + ">" + profile_array[34] + ">" + profile_array[35] + ">" + profile_array[36] + ">" + 
						"null" + ">" + profile_array[38];
				break;
			case "3":
				result = profile_array[1] + ">" + profile_array[2] + ">" + profile_array[3] + ">" + profile_array[4] + ">" + 
						profile_array[5] + ">" + profile_array[6] + ">" + profile_array[7] + ">" + profile_array[8] + ">" + 
						profile_array[9] + ">" + profile_array[10] + ">" + profile_array[11] + ">" + profile_array[12] + ">" + 
						profile_array[13] + ">" + "null" + ">" + "null" + ">" +"null"+ ">" +
						profile_array[17] + ">" + profile_array[18] + ">" + profile_array[19] + ">" + profile_array[20] + ">" + 
						profile_array[21] + ">" + profile_array[22] + ">" + profile_array[23] + ">" + profile_array[24] + ">" + 
						profile_array[25] + ">" + profile_array[26] + ">" + profile_array[27] + ">" + profile_array[28] + ">" + 
						profile_array[29] + ">" + profile_array[30] + ">" + profile_array[31] + ">" + profile_array[32] + ">" + 
						profile_array[33] + ">" + profile_array[34] + ">" + profile_array[35] + ">" + profile_array[36] + ">" + 
						"null" + ">" + profile_array[38];
				break;
			case "4":
				result = profile_array[1] + ">" + profile_array[2] + ">" + "null" + ">" + "null" + ">" + 
						profile_array[5] + ">" + profile_array[6] + ">" + profile_array[7] + ">" + profile_array[8] + ">" + 
						"null" + ">" + "null" + ">" + "null" + ">" + "null" + ">" + 
						"null" + ">" + profile_array[14] + ">" + profile_array[15] + ">" + profile_array[16] + ">" + 
						profile_array[17] + ">" + "null" + ">" + "null" + ">" + profile_array[20] + ">" + 
						"null" + ">" + "null" + ">" + "null" + ">" + profile_array[24] + ">" + 
						profile_array[25] + ">" + profile_array[26] + ">" + profile_array[27] + ">" + profile_array[28] + ">" + 
						profile_array[29] + ">" + profile_array[30] + ">"  + profile_array[31] + ">" + profile_array[32] + ">" + 
						"null" + ">" + "null" + ">" + "null" + ">" + "null" + ">" + 
						profile_array[37] + ">" + profile_array[38];
				break;

		}

		var ipsecClientList = "ipsec_client_list_" + document.form.ipsec_profile_item.value.split("_")[2];
		document.getElementById(document.form.ipsec_profile_item.value).value = result;

		var ipsec_ca = "ipsec_ca_" + document.form.ipsec_profile_item.value.split("_")[2];
		document.getElementById(ipsec_ca).value = document.getElementById('edit_ipsec_ca').value;

		document.getElementById(ipsecClientList).value = ipsec_client_list_array;

		document.form.submit();
		if(document.form.ipsec_profile_item.value == "ipsec_profile_1") {
			ipsec_profile_1 = result;
			ipsec_client_list_1 = ipsec_client_list_array;
			ipsec_ca_1 = document.form.edit_ipsec_ca.value;
		}
		if(document.form.ipsec_profile_item.value == "ipsec_profile_2") {
			ipsec_profile_2 = result;
			ipsec_client_list_2 = ipsec_client_list_array;
			ipsec_ca_2 = document.form.edit_ipsec_ca.value;
		}	
		if(document.form.ipsec_profile_item.value == "ipsec_profile_3") {
			ipsec_profile_3 = result;
			ipsec_client_list_3 = ipsec_client_list_array;
			ipsec_ca_3 = document.form.edit_ipsec_ca.value;
		}
		if(document.form.ipsec_profile_item.value == "ipsec_profile_4") {
			ipsec_profile_4 = result;
			ipsec_client_list_4 = ipsec_client_list_array;
			ipsec_ca_4 = document.form.edit_ipsec_ca.value;
		}
		if(document.form.ipsec_profile_item.value == "ipsec_profile_5") {
			ipsec_profile_5 = result;
			ipsec_client_list_5 = ipsec_client_list_array;
			ipsec_ca_5 = document.form.edit_ipsec_ca.value;
		}

		show_profilelist();
	}
}

function startImportCA() {
	document.getElementById('loadingiconCA').style.display = "";
	setTimeout(function(){document.getElementById('loadingiconCA').style.display = "none";},2000);
	document.form.action = "ipsecupload.cgi";
	document.form.enctype = "multipart/form-data";
	document.form.encoding = "multipart/form-data";
	document.form.submit();
	setTimeout("getImportCA();",2000);
}
function getImportCA() {
	$.ajax({
		url: '/ajax_ipsec.asp',
		dataType: 'script',
		timeout: 1500,
		error: function(xhr){
			setTimeout("getImportCA();",1000);
		},	
		success: function() {
			switch (document.form.ipsec_profile_item.value) {
				case "ipsec_profile_1": 
					document.getElementById('edit_ipsec_ca').value = ipsec_ca_1.replace(/&#10/g, "\n").replace(/&#13/g, "\r");
					break;
				case "ipsec_profile_2": 
					document.getElementById('edit_ipsec_ca').value = ipsec_ca_2.replace(/&#10/g, "\n").replace(/&#13/g, "\r");
					break;
				case "ipsec_profile_3": 
					document.getElementById('edit_ipsec_ca').value = ipsec_ca_3.replace(/&#10/g, "\n").replace(/&#13/g, "\r");
					break;
				case "ipsec_profile_4": 
					document.getElementById('edit_ipsec_ca').value = ipsec_ca_4.replace(/&#10/g, "\n").replace(/&#13/g, "\r");
					break;
				case "ipsec_profile_5": 
					document.getElementById('edit_ipsec_ca').value = ipsec_ca_5.replace(/&#10/g, "\n").replace(/&#13/g, "\r");
					break;
			}
			
		}
	});
}
function startExportCA() {
	var fileContents = document.getElementById("edit_ipsec_ca").value;
	var link = document.createElement('a');
	link.download = document.form.ipsec_profile_item.value + ".crt";
	link.href = 'data:,' + fileContents;
	link.click();
}

function switchMode(mode) {
	initialIPSecProfile();
	document.form.ipsec_vpn_type.value = mode;
	showhide("tr_SettingsMode", 1);
	document.getElementById("selSwitchMode").value = "1";
	switchSettingsMode(1);

	//Basic settings
	showhide("tr_remote_gateway_method", 1);
	showhide("tr_remote_gateway", 1);
	showhide("tr_adv_exchange_mode", 1);
	showhide("tr_importCA", 0);
	showhide("tr_editCA", 0);
	showhide("tr_presharedKey", 1);

	//Network
	showhide("tr_net_local_private_subnet", 1);
	showhide("tr_net_local_port", 1);
	showhide("tr_net_remote_private_subnet", 1);
	showhide("tr_net_remote_port", 1);
	showhide("tr_net_transport", 1);
	
	showhide("tr_net_virtual_ip", 0);
	showhide("tr_net_virtual_subnet", 0);

	//Advanced Settings
	showhide("ipsec_advanced_settings", 0);
	showhide("tr_adv_nat_xauth", 0);
	document.form.ipsec_xauth[0].disabled = false;
	document.form.ipsec_xauth[1].disabled = false;
	//document.form.ipsec_xauth[2].disabled = false;
	showhide("tr_adv_nat_xauth_server_type", 0);
	showhide("tr_adv_nat_xauth_account", 0);
	showhide("tr_adv_nat_xauth_password", 0);
	showhide("tr_adv_ike_isakmp", 0);
	showhide("tr_adv_ike_isakmp_nat", 0);
	document.form.ipsec_dead_peer_detection[0].disabled = false;
	document.form.ipsec_dead_peer_detection[1].disabled = false;
	document.form.ipsec_dead_peer_detection[2].disabled = false;
	document.form.ipsec_dead_peer_detection[3].disabled = false;
	document.getElementById("ipsec_dead_peer_detection_dis").style.display = "";
	document.getElementById("ipsec_dead_peer_detection_clear").style.display = "";
	document.getElementById("ipsec_dead_peer_detection_hold").style.display = "";
	document.getElementById("ipsec_dead_peer_detection_restart").style.display = "";
	document.getElementById("ipsec_dead_peer_detection_dis_label").style.display = "";
	document.getElementById("ipsec_dead_peer_detection_clear_label").style.display = "";
	document.getElementById("ipsec_dead_peer_detection_hold_label").style.display = "";
	document.getElementById("ipsec_dead_peer_detection_restart_label").style.display = "";
	showhide("tr_adv_encryption_p1", 1);
	showhide("tr_adv_hash_p1", 1);
	showhide("tr_adv_local_id", 1);
	showhide("tr_adv_remote_id", 1);
	showhide("tr_adv_keylife_time_p1", 1);
	showhide("tr_adv_dpd_interval", 1);
	showhide("tr_adv_dead_peer_detection", 1);
	showhide("tb_adv_phase2", 1);

	document.getElementById("ipsec_ike_v1").style.display = "";
	document.getElementById("ipsec_ike_v2").style.display = "";
	document.getElementById("ipsec_ike_auto").style.display = "";
	document.getElementById("ipsec_ike_v1_label").style.display = "";
	document.getElementById("ipsec_ike_v2_label").style.display = "";
	document.getElementById("ipsec_ike_auto_label").style.display = "";

	//User table
	document.getElementById("tbAccountList").style.display = "none";
	document.getElementById("ipsec_client_list_Block").style.display = "none";

	document.getElementById("ipsec_advanced_settings").style.display = "none";

	while(document.form.ipsec_local_public_interface.options.length > 0){
		document.form.ipsec_local_public_interface.remove(0);
	}
	var add_public_interface = function() {
		var wans_cap = '<% nvram_get("wans_cap"); %>'.split(" ");
		var wan_type_list = [];
		for(var i = 0; i < wans_cap.length; i += 1) {
			var option_value = "";
			var option_text = "";
			option_value = wans_cap[i];
			
			switch(wans_cap[i]) {
				case "wan" :
					option_text = "<#menu5_3#>";
					break;
				case "wan2" :
					option_text = wans_cap[i].toUpperCase();
					break;
				case "usb" :
					option_text = wans_cap[i].toUpperCase();
					break;
				case "lan" :
					option_text = "Ethernet LAN";
					break;
			}

			var option = [option_value, option_text];
			wan_type_list.push(option);
		}
		var selectobject = document.form.ipsec_local_public_interface;

		for(var i = 0; i < wan_type_list.length; i += 1) {
			var option = document.createElement("option");
			option.value = wan_type_list[i][0];
			option.text = wan_type_list[i][1];
			selectobject.add(option);
		}	
	};

	add_public_interface();

	showhide("tr_general_dns1", 0);
	showhide("tr_general_dns2", 0);
	showhide("tr_general_wins1", 0);
	showhide("tr_general_wins2", 0);

	switch(mode) {
		case "1":
			showhide("tr_remote_gateway_method", 0);
			showhide("tr_remote_gateway", 0);
			document.getElementById("td_network_title").innerHTML = "Network - Subnet";/*untranslated*/
			document.form.ipsec_xauth[1].disabled = true;
			settingRadioItemCheck(document.form.ipsec_xauth, "0");
			document.form.ipsec_dead_peer_detection[2].disabled = true;
			document.form.ipsec_dead_peer_detection[3].disabled = true;
			document.getElementById("ipsec_dead_peer_detection_hold").style.display = "none";
			document.getElementById("ipsec_dead_peer_detection_restart").style.display = "none";
			document.getElementById("ipsec_dead_peer_detection_hold_label").style.display = "none";
			document.getElementById("ipsec_dead_peer_detection_restart_label").style.display = "none";
			settingRadioItemCheck(document.form.ipsec_ike, "1");
			changeIKEVersion();
			document.getElementById("ipsec_ike_auto").style.display = "none";
			document.getElementById("ipsec_ike_auto_label").style.display = "none";
			break;
		case "2":
			document.getElementById("td_network_title").innerHTML = "Network - Subnet";/*untranslated*/
			document.form.ipsec_xauth[0].disabled = true;
			settingRadioItemCheck(document.form.ipsec_xauth, "0");
			break;
		case "3":
			document.getElementById("td_network_title").innerHTML = "Network - Subnet";/*untranslated*/
			settingRadioItemCheck(document.form.ipsec_xauth, "0");
			break;
		case "4":
			showhide("tr_remote_gateway_method", 0);
			showhide("tr_remote_gateway", 0);
			showhide("tr_adv_exchange_mode", 1);
			document.getElementById("td_network_title").innerHTML = "Network - Virtual IP";/*untranslated*/
			showhide("tr_net_local_private_subnet", 0);
			showhide("tr_net_local_port", 0);
			showhide("tr_net_remote_private_subnet", 0);
			showhide("tr_net_remote_port", 0);
			showhide("tr_net_transport", 0);
			showhide("tr_net_virtual_ip", 0);
			showhide("tr_net_virtual_subnet", 1);

			showhide("tr_adv_encryption_p1", 0);

			showhide("tr_adv_hash_p1", 0);
			showhide("tr_adv_local_id", 0);
			showhide("tr_adv_remote_id", 0);
			showhide("tr_adv_keylife_time_p1", 0);
			document.form.ipsec_dead_peer_detection[2].disabled = true;
			document.form.ipsec_dead_peer_detection[3].disabled = true;
			document.getElementById("ipsec_dead_peer_detection_hold").style.display = "none";
			document.getElementById("ipsec_dead_peer_detection_restart").style.display = "none";
			document.getElementById("ipsec_dead_peer_detection_hold_label").style.display = "none";
			document.getElementById("ipsec_dead_peer_detection_restart_label").style.display = "none";
			showhide("tb_adv_phase2", 0);
			settingRadioItemCheck(document.form.ipsec_ike, "1");
			changeIKEVersion();
			document.getElementById("ipsec_ike_v1").style.display = "none";
			document.getElementById("ipsec_ike_v2").style.display = "none";
			document.getElementById("ipsec_ike_auto").style.display = "none";
			document.getElementById("ipsec_ike_v2_label").style.display = "none";
			document.getElementById("ipsec_ike_auto_label").style.display = "none";
			showhide("tr_general_dns1", 1);
			showhide("tr_general_dns2", 1);
			showhide("tr_general_wins1", 1);
			showhide("tr_general_wins2", 1);
			break;	
	}
}

function changeIKEVersion() {
	var ike_version = getRadioItemCheck(document.form.ipsec_ike);
	switch(document.form.ipsec_vpn_type.value) {
		case "1" :
			switch(ike_version) {
				case "1" :
					showhide("tr_adv_exchange_mode", 1);
					break;
				case "2" :
					showhide("tr_adv_exchange_mode", 0);
					settingRadioItemCheck(document.form.ipsec_exchange, "0");
					changeExchangeMode();
					break;
			}
			break;
		case "4" :
			switch(ike_version) {
				case "1" :
					showhide("tr_adv_nat_xauth", 1);
					showhide("tr_adv_nat_xauth_server_type", 0);
					changeXAUTH();
					document.getElementById("th_xauth_title").innerHTML = "XAUTH";
					document.getElementById("th_xauth_account_title").innerHTML = "XAUTH account";
					document.getElementById("th_xauth_password_title").innerHTML = "XAUTH password";
					break;
				case "2" :
					showhide("tr_adv_nat_xauth", 1);
					if(getRadioItemCheck(document.form.ipsec_auth_method) == "1") {	//PSK
						showhide("tr_adv_nat_xauth_server_type", 0);
					}
					else {
						showhide("tr_adv_nat_xauth_server_type", 1);
					}
					changeXAUTH();
					document.getElementById("th_xauth_title").innerHTML = "User authentication role";
					document.getElementById("th_xauth_account_title").innerHTML = "User authentication account";
					document.getElementById("th_xauth_password_title").innerHTML = "User authentication password";
					document.getElementById("th_xauth_server_type_title").innerHTML = "User authentication method";
					break;
				case "auto" :
					showhide("tr_adv_nat_xauth", 0);
					showhide("tr_adv_nat_xauth_server_type", 0);
					showhide("tr_adv_nat_xauth_account", 0);
					showhide("tr_adv_nat_xauth_password", 0);
					document.getElementById("tbAccountList").style.display = "none";
					document.getElementById("ipsec_client_list_Block").style.display = "none";
					break;
			}
			break;
	}
}
function changeXAUTH() {
	var clickItem = getRadioItemCheck(document.form.ipsec_xauth);
	document.getElementById("tbAccountList").style.display = "none";
	document.getElementById("ipsec_client_list_Block").style.display = "none";
	showhide("tr_adv_nat_xauth_account", 0);
	showhide("tr_adv_nat_xauth_password", 0);

	switch (clickItem) {
		case "1" :
			document.getElementById("tbAccountList").style.display = "";
			document.getElementById("ipsec_client_list_Block").style.display = "";
			showipsec_clientlist();
			break;
		case "2" :
			showhide("tr_adv_nat_xauth_account", 1);
			showhide("tr_adv_nat_xauth_password", 1);
			break;
	}

}
function changeAdvTraversal(obj) {
	if(obj.value == "1") {
		showhide("tr_adv_ike_isakmp", 1);
		showhide("tr_adv_ike_isakmp_nat", 1);
	}
	else {
		showhide("tr_adv_ike_isakmp", 0);
		showhide("tr_adv_ike_isakmp_nat", 0);
	}
}
function changeVirtualIP(obj) {
	if(obj.value == "1") {
		showhide("tr_net_virtual_subnet", 1);
	}
	else {
		showhide("tr_net_virtual_subnet", 0);
	}
}
function add_subnet_item(obj, _type) {
	var existSubnetItem = obj.parentNode.getElementsByClassName("input_25_table").length;
	if(existSubnetItem > 3) {
		alert("<#JS_itemlimit1#> " + existSubnetItem + " <#JS_itemlimit2#>");
	}
	else {
		var code = "";
		var divObj = document.createElement("input");
		divObj.type = "text";
		divObj.className = "input_25_table";
		divObj.id = "ipsec_" + _type + "_subnet_" + (existSubnetItem + 1);
		if(subnetIP_support_IPv6)
			divObj.maxLength = "39";
		else {
			divObj.maxLength = "18";
			divObj.onkeypress = function() {
				return validator.isIPAddrPlusNetmask(this,event);
			};
		}
		divObj.style.marginTop = "4px";
		//divObj.onkeypress = function(){return validator.isIPAddr(this, event);};
		obj.parentNode.appendChild(divObj);

		var removeElement = function(element) {
			element && element.parentNode && element.parentNode.removeChild(element);
		};
		if(document.getElementById("btDelRemoteSubnet_" + _type) != null) {
			removeElement(document.getElementById("btDelRemoteSubnet_" + _type));
		}
		var divObj = document.createElement("input");
		divObj.id = "btDelRemoteSubnet_" + _type;
		divObj.type = "text";
		divObj.className = "remove_btn";
		divObj.style.height = "27px";
		divObj.onclick = function() { del_subnet_item(this, _type);};
		obj.parentNode.appendChild(divObj);
	}
}
function del_subnet_item(obj, _type) {

	var delIndex = obj.parentNode.getElementsByClassName('input_25_table').length;
	var removeElement = function(element) {
	    element && element.parentNode && element.parentNode.removeChild(element);
	};
	if(document.getElementById("ipsec_" + _type + "_subnet_" + delIndex) != null) {
		removeElement(document.getElementById("ipsec_" + _type + "_subnet_"+ delIndex));
	}
	if(delIndex == "2") {
		if(document.getElementById("btDelRemoteSubnet_" + _type) != null) {
			removeElement(document.getElementById("btDelRemoteSubnet_" + _type));
		}
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

	var validForm = function() {
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
		else if(!Block_chars(valid_password, ["<", ">", "&"])) {
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


	if(validForm()) {
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
function changeAuthMethod(obj) {
	if(obj.value == "0") {
		//showhide("tr_importCA", 1);
		//showhide("tr_editCA", 1);
		showhide("tr_presharedKey", 0);
	}
	else {
		//showhide("tr_importCA", 0);
		//showhide("tr_editCA", 0);
		showhide("tr_presharedKey", 1);
	}
}
function switchSettingsMode(mode){
	if(mode == "1") {
		document.getElementById("ipsec_basic_settings").style.display = "";
		document.getElementById("ipsec_network_settings").style.display = "";
		document.getElementById("ipsec_advanced_settings").style.display = "none";
		document.getElementById("tbAccountList").style.display = "none";
		document.getElementById("ipsec_client_list_Block").style.display = "none";
	}	
	else {
		document.getElementById("ipsec_basic_settings").style.display = "none";
		document.getElementById("ipsec_network_settings").style.display = "none";
		document.getElementById("ipsec_advanced_settings").style.display = "";
		if(getRadioItemCheck(document.form.ipsec_xauth) == "1") {
			document.getElementById("tbAccountList").style.display = "";
			document.getElementById("ipsec_client_list_Block").style.display = "";
			switch(document.form.ipsec_profile_item.value) {
				case "ipsec_profile_1" :
					ipsec_client_list_array = ipsec_client_list_1;
					break;
				case "ipsec_profile_2" :
					ipsec_client_list_array = ipsec_client_list_2;
					break;
				case "ipsec_profile_3" :
					ipsec_client_list_array = ipsec_client_list_3;
					break;
				case "ipsec_profile_4" :
					ipsec_client_list_array = ipsec_client_list_4;
					break;
				case "ipsec_profile_5" :
					ipsec_client_list_array = ipsec_client_list_5;
					break;
			}
			showipsec_clientlist();
		}
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
function applyRule() {
	document.general_form.force_change.value++;
	document.general_form.ipsec_server_enable.value = ipsec_server_enable;
	document.general_form.submit();
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
function changeAdvDeadPeerDetection (obj) {
	if(obj.value == "0") {
		showhide("tr_adv_dpd_interval", 0);
	}
	else {
		showhide("tr_adv_dpd_interval", 1);
	}
}
function ipsecShowAndHide(server_enable) {
	if(server_enable == "1") {
		ipsec_server_enable = 1;
	}
	else{
		ipsec_server_enable = 0;
	}

	document.general_form.ipsec_server_enable.value = ipsec_server_enable;
	showLoading();
	document.general_form.submit();
}
function changeExchangeMode() {
	var clickItem = getRadioItemCheck(document.form.ipsec_exchange);
	$("#exchange_mode_hint").css("display", "none");
	if(clickItem == "1") {
		$("#exchange_mode_hint").css("display", "");
	}
}
</script>
</head>

<body onload="initial();">
<div id="TopBanner"></div>
<div id="Loading" class="popup_bg"></div>

<iframe name="hidden_frame_general" id="hidden_frame" src="" width="0" height="0" frameborder="0"></iframe>
<form method="post" name="general_form" id="general_form" action="/start_apply.htm" target="hidden_frame">
	<input type="hidden" name="current_page" value="Advanced_VPN_IPSec.asp">
	<input type="hidden" name="next_page" value="Advanced_VPN_IPSec.asp">
	<input type="hidden" name="modified" value="0">
	<input type="hidden" name="action_mode" value="apply">
	<input type="hidden" name="action_wait" value="30">
	<input type="hidden" name="action_script" value="ipsec_set">
	<input type="hidden" name="ipsec_server_enable" value="<% nvram_get("ipsec_server_enable"); %>">
	<input type="hidden" name="force_change" value="<% nvram_get("force_change"); %>">
</form>
<iframe name="hidden_frame" id="hidden_frame" src="" width="0" height="0" frameborder="0"></iframe>
<form method="post" name="form" action="/start_apply.htm" target="hidden_frame">
<input type="hidden" name="productid" value="<% nvram_get("productid"); %>">
<input type="hidden" name="current_page" value="Advanced_VPN_IPSec.asp">
<input type="hidden" name="next_page" value="Advanced_VPN_IPSec.asp">
<input type="hidden" name="group_id" value="">
<input type="hidden" name="modified" value="0">
<input type="hidden" name="flag" value="background">
<input type="hidden" name="action_mode" value="apply">
<input type="hidden" name="action_script" value="saveNvram">
<input type="hidden" name="action_wait" value="3">
<input type="hidden" name="preferred_lang" id="preferred_lang" value="<% nvram_get("preferred_lang"); %>">

<input type="hidden" name="ipsec_profile_item" value="">
<input type="hidden" name="ipsec_profile_1" id="ipsec_profile_1" value="">
<input type="hidden" name="ipsec_profile_2" id="ipsec_profile_2" value="">
<input type="hidden" name="ipsec_profile_3" id="ipsec_profile_3" value="">
<input type="hidden" name="ipsec_profile_4" id="ipsec_profile_4" value="">
<input type="hidden" name="ipsec_profile_5" id="ipsec_profile_5" value="">

<input type="hidden" name="ipsec_client_list_1" id="ipsec_client_list_1" value="">
<input type="hidden" name="ipsec_client_list_2" id="ipsec_client_list_2" value="">
<input type="hidden" name="ipsec_client_list_3" id="ipsec_client_list_3" value="">
<input type="hidden" name="ipsec_client_list_4" id="ipsec_client_list_4" value="">
<input type="hidden" name="ipsec_client_list_5" id="ipsec_client_list_5" value="">

<input type="hidden" name="ipsec_ca_1" id="ipsec_ca_1" value="">
<input type="hidden" name="ipsec_ca_2" id="ipsec_ca_2" value="">
<input type="hidden" name="ipsec_ca_3" id="ipsec_ca_3" value="">
<input type="hidden" name="ipsec_ca_4" id="ipsec_ca_4" value="">
<input type="hidden" name="ipsec_ca_5" id="ipsec_ca_5" value="">

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
									<div style="margin-left:5px;margin-top:10px;margin-bottom:10px"><img src="/images/New_ui/export/line_export.png"></div>
									<div class="formfontdesc"></div>

									<table id="ipsec_general_setting" width="100%" border="1" align="center" cellpadding="4" cellspacing="0" bordercolor="#6b8fa3" class="FormTable">
										<thead>
										<tr>
											<td colspan="2"><#t2BC#></td>
										</tr>
										</thead>

										<tr>
											<th>Enable IPSec VPN Server<!--untranslated--></th>
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
										<tr id="tr_general_ike_isakmp">
											<th>IKE / ISAKMP Port<!--untranslated--></th>
											<td>500</td>
										</tr>
										<tr id="tr_general_ike_isakmp_nat">
											<th>IKE / ISAKMP NAT-T Port<!--untranslated--></th>
											<td>4500</td>
										</tr>										
										<tr id="tr_general_log">
											<th><#System_Log#></th>
											<td>
												<input class="button_gen" onclick="viewLog()" type="button" value="<#CTL_check_log#>"/>
											</td>
										</tr>
									</table>

									<div id="ipsec_add_profile_bg" class="ipsec_add_profile_bg" >
										<div class='ipsec_add_profile_icon' onclick="createIPSecProfile();">+</div>
										<span class='ipsec_add_profile_title' onclick="createIPSecProfile();"><#vpnc_step1#></span>
									</div>

									<table id="ipsec_server_profile_table" width="100%" align="center" cellpadding="4" cellspacing="0" class="FormTable_table">
										<thead>
										<tr>
											<td colspan="8">IPSec Server Profile<!--untranslated-->&nbsp;(<#List_limit#>&nbsp;2)</td>
										</tr>
										</thead>
										<tr>
											<th width="15%"><#PPPConnection_x_WANLink_itemname#></th>
											<th width="26%">Profile Name<!--untranslated--></th>
											<th width="25%">VPN type<!--untranslated--></th>
											<th width="7%"><#pvccfg_edit#></th>
											<th width="7%"><#CTL_del#></th>
											<th width="20%"><#Connecting#></th>
										</tr>
									</table>
									<div id="ipsec_profilelist_block"></div>
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


<div id="edit_ipsec_profile_panel" class="ipsec_profile_panel" style="padding:15px;">
	<!-- VPN Type table start-->
	<table width="100%" border="1" align="center" cellpadding="4" cellspacing="0" bordercolor="#6b8fa3"  class="FormTable">
		<thead>
		<tr>
			<td colspan="2">Quick Select<!--untranslated--></td>
		</tr>
		</thead>
		<tr>
			<th>VPN Type<!--untranslated--></th>
			<td>
				<select name="ipsec_vpn_type" onchange="switchMode(this.options[this.selectedIndex].value)" class="input_option"></select>
				<div id="ipsec_vpn_type_view" style="display:none;"></div>
			</td>
		</tr>
		<tr id="tr_SettingsMode">
			<th><#vpn_Adv#></th>
			<td>
				<select id="selSwitchMode" onchange="switchSettingsMode(this.options[this.selectedIndex].value)" class="input_option">
					<option value="1" selected><#menu5_1_1#></option>
					<option value="2"><#menu5#></option>
				</select>
			</td>
		</tr>
	</table>
	<!-- VPN Type table end-->
	<!-- Basic settings table start-->
	<table id="ipsec_basic_settings" width="100%" border="1" align="center" cellpadding="4" cellspacing="0" bordercolor="#6b8fa3"  class="FormTable" style="margin-top:15px;">
		<thead>
		<tr>
			<td colspan="2"><#t2BC#></td>
		</tr>
		</thead>
		<tr>
			<th>VPN Profile Name<!--untranslated--></th>
			<td>
				<input type="text" class="input_25_table" name="ipsec_profilename">
			</td>
		</tr>
		<tr id="tr_remote_gateway_method">
			<th>Remote Gateway Method<!--untranslated--></th>
			<td>
				<input type="radio" name="ipsec_remote_gateway_method" class="input" value="0" checked>Static IP Address<!--untranslated-->
				<input type="radio" name="ipsec_remote_gateway_method" class="input" value="1"><#LANHostConfig_x_LDNSServer1_itemname#>
			</td>
		</tr>
		<tr id="tr_remote_gateway">
			<th>Remote Gateway<!--untranslated--></th>
			<td>
				<input type="text" class="input_25_table" name="ipsec_remote_gateway">
			</td>
		</tr>
		<tr>
			<th>Local Public Interface</th>
			<td>
				<select name="ipsec_local_public_interface" class="input_option"></select>
			</td>
		</tr>
		<tr style="display:none;">
			<th><#WLANConfig11b_AuthenticationMethod_itemname#></th>
			<td>
				<input type="radio" name="ipsec_auth_method" id="ipsec_auth_method_rsa" class="input" value="0" onchange="changeAuthMethod(this);" style="display:none;">
				<label for='ipsec_auth_method_rsa' style="display:none;">RSA Signature<!--untranslated--></label>
				<input type="radio" name="ipsec_auth_method" id="ipsec_auth_method_psk" class="input" value="1" onchange="changeAuthMethod(this);" checked>
				<label for='ipsec_auth_method_psk'>Preshared Key<!--untranslated--></label>
			</td>
		</tr>
		<tr id="tr_presharedKey">
			<th>Preshared Key<!--untranslated--></th>
			<td>
				<input id="ipsec_preshared_key" name="ipsec_preshared_key" type="password" autocapitalization="off" onBlur="switchType(this, false);" onFocus="switchType(this, true);" class="input_25_table" maxlength="32" placeholder="Enter Preshared Key"><!--untranslated-->
			</td>
		</tr>
		<tr id="tr_importCA">
			<th><#vpn_openvpnc_importCA#></th>
			<td>
				<input id="importCA" class="button_gen" onclick="startImportCA();" type="button" value="<#CTL_upload#>" />
				<img id="loadingiconCA" style="margin-left:5px;display:none;" src="/images/InternetScan.gif">
				<input type="file" name="file" class="input" style="color:#FC0;*color:#000;">
			</td>
		</tr>
		<tr id="tr_editCA">
			<th><#Manual_Setting_btn#> CA</th>
			<td>
				<textarea rows="8" class="textarea_ssh_table" id="edit_ipsec_ca" name="edit_ipsec_ca" cols="65" maxlength="2999"></textarea>
				<input id="exportCA" class="button_gen" onclick="startExportCA();" type="button" value="<#btn_Export#>" />
			</td>
		</tr>
		<tr id="tr_adv_local_id">
			<th>Local Identity<!--untranslated--></th>
			<td>
				<input type="text" class="input_25_table" name="ipsec_local_id">
				<span style="color:#FC0">(Optional)<!--untranslated--></span>
			</td>
		</tr>
		<tr id="tr_adv_remote_id">
			<th>Remote Identity<!--untranslated--></th>
			<td>
				<input type="text" class="input_25_table" name="ipsec_remote_id">
				<span style="color:#FC0">(Optional)<!--untranslated--></span>
			</td>
		</tr>
	</table>
	<!-- Basic settings table end-->
	
	<!-- Network table start-->
	<table id="ipsec_network_settings" width="100%" border="1" align="center" cellpadding="4" cellspacing="0" bordercolor="#6b8fa3"  class="FormTable" style="margin-top:15px;">
		<thead>
		<tr>
			<td id="td_network_title" colspan="2"></td>
		</tr>
		</thead>
		<!--Subbet start-->
		<tr id="tr_net_local_private_subnet">
			<th>Local Private Subnet<!--untranslated--></th>
			<td id="td_net_local_private_subnet"></td>
		</tr>
		<tr id="tr_net_local_port">
			<th>Local Port number<!--untranslated--></th>
			<td>
				<input type="text" class="input_6_table" name="ipsec_local_port" maxlength="5" value="0" onKeyPress="return validator.isNumber(this,event)">
				<span style="color:#FC0">(0-65535)</span>
			</td>
		</tr>
		<tr id="tr_net_remote_private_subnet">
			<th>Remote Private Subnet<!--untranslated--></th>
			<td id="td_net_remote_private_subnet"></td>
		</tr>
		<tr id="tr_net_remote_port">
			<th>Remote Port number<!--untranslated--></th>
			<td>
				<input type="text" class="input_6_table" name="ipsec_remote_port" maxlength="5" value="0" onKeyPress="return validator.isNumber(this,event)">
				<span style="color:#FC0">(0-65535)</span>
			</td>
		</tr>
		<tr id="tr_net_transport">
			<th><#DSL_Mode#></th>
			<td>
				Tunnel<!--untranslated-->
				<!--select name="ipsec_transport" class="input_option">
					<option value="tunnel">Tunnel</option>
					<option value="transport">Transport</option>
					<option value="transport_proxy">Transport Proxy</option>
					<option value="passthrough">Passthrough</option>
					<option value="drop">Drop</option>
				</select-->
			</td>
		</tr>
		<!--Subbet end-->
		<!--Virtual IP start-->
		<tr id="tr_net_virtual_ip">
			<th>Virtual IP<!--untranslated--></th>
			<td>
				<input type="radio" name="ipsec_virtual_ip" class="input" value="1" checked onchange="changeVirtualIP(this);"><#btn_Enabled#>
				<input type="radio" name="ipsec_virtual_ip" class="input" value="0" onchange="changeVirtualIP(this);"><#btn_disable#>
			</td>
		</tr>

		<tr id="tr_net_virtual_subnet">
			<th>Virtual IP<!--untranslated--></th>
			<td>
				<input type="text" class="input_25_table" name="ipsec_virtual_subnet" maxlength="18" onKeyPress="return validator.isIPAddrPlusNetmask(this,event)" autocorrect="off" autocapitalize="off">
				<span style="color:#FC0"> (ex.10.10.10.0/24)</span>
			</td>
		</tr>
		<tr id="tr_net_virtual_accessible_networks" style="display:none;">
			<th>Accessible Networks<!--untranslated--></th>
			<td>
				<select name="ipsec_accessible_networks" class="input_option">
				</select>
			</td>
		</tr>
		<!--Virtual IP end-->
		<tr id="tr_general_dns1">
			<th><#IPConnection_x_DNSServer1_itemname#></th>
			<td>
				<input type="text" maxlength="15" class="input_15_table" name="ipsec_dns1"  onkeypress="return validator.isIPAddr(this, event)" >
				<span style="color:#FC0">(Optional)<!--untranslated--></span>
			</td>
		</tr>
		<tr id="tr_general_dns2">
			<th><#IPConnection_x_DNSServer2_itemname#></th>
			<td>
				<input type="text" maxlength="15" class="input_15_table" name="ipsec_dns2"  onkeypress="return validator.isIPAddr(this, event)" >
				<span style="color:#FC0">(Optional)<!--untranslated--></span>
			</td>
		</tr>
		<tr id="tr_general_wins1">
			<th><#IPConnection_x_WINSServer1_itemname#></th>
			<td>
				<input type="text" maxlength="15" class="input_15_table" name="ipsec_wins1"  onkeypress="return validator.isIPAddr(this, event)" >
				<span style="color:#FC0">(Optional)<!--untranslated--></span>
			</td>
		</tr>
		<tr id="tr_general_wins2">
			<th><#IPConnection_x_WINSServer2_itemname#></th>
			<td>
				<input type="text" maxlength="15" class="input_15_table" name="ipsec_wins2"  onkeypress="return validator.isIPAddr(this, event)" >
				<span style="color:#FC0">(Optional)<!--untranslated--></span>
			</td>
		</tr>
		
	</table>
	<!-- Network table end-->
	
	<!-- Advanced Settings table start-->
	<div id="ipsec_advanced_settings" style="display:none;">
		<table width="100%" border="1" align="center" cellpadding="4" cellspacing="0" bordercolor="#6b8fa3"  class="FormTable" style="margin-top:15px;">
			<thead>
			<tr>
				<td colspan="2">Advanced Settings - Phase 1 Negotiations<!--untranslated--></td>
			</tr>
			</thead>
			<tr id="tr_adv_ike_version">
				<th>IKE version<!--untranslated--></th>
				<td>
					<input type="radio" name="ipsec_ike" id="ipsec_ike_v1" class="input" value="1" onchange="changeIKEVersion()">
					<label for='ipsec_ike_v1' id="ipsec_ike_v1_label">v1<!--untranslated--></label>
					<input type="radio" name="ipsec_ike" id="ipsec_ike_v2" class="input" value="2" onchange="changeIKEVersion()">
					<label for='ipsec_ike_v2' id="ipsec_ike_v2_label">v2<!--untranslated--></label>
					<input type="radio" name="ipsec_ike" id="ipsec_ike_auto" class="input" value="auto" checked onchange="changeIKEVersion()">
					<label for='ipsec_ike_auto' id="ipsec_ike_auto_label"><#Auto#></label>
				</td>
			</tr>
			<tr id="tr_adv_encryption_p1">
				<th>Encryption<!--untranslated--></th>
				<td>
					<select name="ipsec_encryption_p1" class="input_option">
						<option value="auto"><#Auto#></option>
						<!--option value="des">DES</option-->
						<option value="3des">3DES<!--untranslated--></option>
						<option value="aes128">AES128<!--untranslated--></option>
						<!--option value="aes192">AES192</option-->
						<!--option value="aes256">AES256</option-->
					</select>
				</td>
			</tr>
			<tr id="tr_adv_hash_p1">
				<th>Hash</th>
				<td>
					<select name="ipsec_hash_p1" class="input_option">
						<option value="auto"><#Auto#></option>
						<!--option value="md5">MD5</option-->
						<option value="sha1">SHA1<!--untranslated--></option>
						<option value="sha256">SHA256<!--untranslated--></option>
						<!--option value="sha384">SHA384</option-->
						<!--option value="sha512">SHA512</option-->
					</select>
				</td>
			</tr>
				<tr id="tr_adv_exchange_mode">
				<th>Exchange Mode<!--untranslated--></th>
				<td>
					<input type="radio" name="ipsec_exchange" class="input" value="1" onchange="changeExchangeMode();"><#DHCPaggressive#>
					<input type="radio" name="ipsec_exchange" class="input" value="0" onchange="changeExchangeMode();" checked>Main Mode<!--untranslated-->
					<div id="exchange_mode_hint" style="color:#FC0;margin:5px 0px;">When you're using IKEv1 Aggressive mode, the authentication hash, PSK is transmitted as response to the initial packet of the VPN client that wants to establish an IPSec Tunnel. The hash PSK is not encrypted. An attacker can do offline dictionary and brute-force attacks on it to recover the PSK. Please try to avoid Aggressive Mode.<!--untranslated--></div>
				</td>
			</tr>
			<tr id="tr_adv_keylife_time_p1">
				<th>IKE keylife time<!--untranslated--></th>
				<td>
					<input type="text" class="input_6_table" name="ipsec_keylife_p1" maxlength="6" value="86400" onKeyPress="return validator.isNumber(this,event)">
					<span style="color:#FC0">(120~172800) <#Second#></span>
				</td>
			</tr>
			<tr id="tr_adv_nat_xauth">
				<th id="th_xauth_title">XAUTH<!--untranslated--></th>
				<td>
					<input type="radio" name="ipsec_xauth" class="input" value="1" onchange="changeXAUTH();"><#WLANConfig11b_WirelessCtrl_button1name#>
					<!--input type="radio" name="ipsec_xauth" class="input" value="2" onchange="changeXAUTH();">Client-->
					<input type="radio" name="ipsec_xauth" class="input" value="0" onchange="changeXAUTH();"><#WLANConfig11b_WirelessCtrl_buttonname#>
				</td>
			</tr>
			<tr id="tr_adv_nat_xauth_account">
				<th id="th_xauth_account_title">XAUTH account<!--untranslated--></th>
				<td>
					<input type="text" class="input_25_table" name="ipsec_xauth_account">
				</td>
			</tr>
			<tr id="tr_adv_nat_xauth_password">
				<th id="th_xauth_password_title">XAUTH password<!--untranslated--></th>
				<td>
					<input type="text" class="input_25_table" name="ipsec_xauth_password">
				</td>
			</tr>
			<tr id="tr_adv_nat_xauth_server_type">
				<th id="th_xauth_server_type_title">User Authentication<!--untranslated--></th>
				<td>
					<select name="ipsec_xauth_server_type" class="input_option">
						<option value="eap-md5">eap-md5<!--untranslated--></option>
						<option value="eap-mschapv2">eap-mschapv2<!--untranslated--></option>
						<option value="eap-peap">eap-peap<!--untranslated--></option>
						<option value="eap-tls">eap-tls<!--untranslated--></option>
					</select>
				</td>
			</tr>
			<tr id="tr_adv_traversal" style="display:none;">
				<th>NAT traversal<!--untranslated--></th>
				<td>
					<input type="radio" name="ipsec_traversal" class="input" value="1" checked onchange="changeAdvTraversal(this)"><#checkbox_Yes#>
					<input type="radio" name="ipsec_traversal" class="input" value="0" onchange="changeAdvTraversal(this)"><#checkbox_No#>
				</td>
			</tr>
			<tr id="tr_adv_ike_isakmp" style="display:none;">
				<th>IKE / ISAKMP Port<!--untranslated--></th>
				<td>
					<input type="text" class="input_6_table" name="ipsec_ike_isakmp" maxlength="3" value="500">
				</td>
			</tr>
			<tr id="tr_adv_ike_isakmp_nat" style="display:none;">
				<th>IKE / ISAKMP NAT-T Port<!--untranslated--></th>
				<td>
					<input type="text" class="input_6_table" name="ipsec_ike_isakmp_nat" maxlength="4" value="4500">
				</td>
			</tr>
			<tr id="tr_adv_dead_peer_detection">
				<th>Dead Peer Detection<!--untranslated--></th>
				<td>
					<input type="radio" name="ipsec_dead_peer_detection" id="ipsec_dead_peer_detection_clear" class="input" value="1" onchange="changeAdvDeadPeerDetection(this)">
					<label for='ipsec_dead_peer_detection_clear' id="ipsec_dead_peer_detection_clear_label"><#WLANConfig11b_WirelessCtrl_button1name#></label>
					<input type="radio" name="ipsec_dead_peer_detection" id="ipsec_dead_peer_detection_dis" class="input" value="0" onchange="changeAdvDeadPeerDetection(this)">
					<label for='ipsec_dead_peer_detection_dis' id="ipsec_dead_peer_detection_dis_label"><#WLANConfig11b_WirelessCtrl_buttonname#></label>
					<input type="radio" name="ipsec_dead_peer_detection" id="ipsec_dead_peer_detection_hold" class="input" value="2" onchange="changeAdvDeadPeerDetection(this)">
					<label for='ipsec_dead_peer_detection_hold' id="ipsec_dead_peer_detection_hold_label">Hold<!--untranslated--></label>
					<input type="radio" name="ipsec_dead_peer_detection" id="ipsec_dead_peer_detection_restart" class="input" value="3" onchange="changeAdvDeadPeerDetection(this)">
					<label for='ipsec_dead_peer_detection_restart' id="ipsec_dead_peer_detection_restart_label">Restart<!--untranslated--></label>
				</td>
			</tr>
			<tr id="tr_adv_dpd_interval">
				<th>DPD checking interval<!--untranslated--></th>
				<td>
					<input type="text" class="input_3_table" name="ipsec_dpd" maxlength="3" value="10" onKeyPress="return validator.isNumber(this,event)">
					<span style="color:#FC0">(10~900) <#Second#></span>
				</td>
			</tr>
		</table>
		<table id="tb_adv_phase2" width="100%" border="1" align="center" cellpadding="4" cellspacing="0" bordercolor="#6b8fa3"  class="FormTable" style="margin-top:15px;">
			<thead>
			<tr>
				<td colspan="2">Advanced Settings - Phase 2 Negotiations<!--untranslated--></td>
			</tr>
			</thead>
			<tr id="tr_adv_encryption_p2">
				<th>Encryption<!--untranslated--></th>
				<td>
					<select name="ipsec_encryption_p2" class="input_option">
						<option value="auto"><#Auto#></option>
						<!--option value="des">DES</option-->
						<option value="3des">3DES<!--untranslated--></option>
						<option value="aes128">AES128<!--untranslated--></option>
						<!--option value="aes192">AES192</option-->
						<!--option value="aes256">AES256</option-->
					</select>
				</td>
			</tr>
			<tr id="tr_adv_hash_p2">
				<th>Hash<!--untranslated--></th>
				<td>
					<select name="ipsec_hash_p2" class="input_option">
						<option value="auto"><#Auto#></option>
						<!--option value="md5">MD5</option-->
						<option value="sha1">SHA1<!--untranslated--></option>
						<option value="sha256">SHA256<!--untranslated--></option>
						<!--option value="sha384">SHA384</option-->
						<!--option value="sha512">SHA512</option-->
					</select>
				</td>
			</tr>
			<tr id="tr_adv_keylife_time_p2">
				<th>Keylife time<!--untranslated--></th>
				<td>
					<input type="text" class="input_6_table" name="ipsec_keylife_p2" maxlength="6" value="3600" onKeyPress="return validator.isNumber(this,event)">
					<span style="color:#FC0">(120~172800) <#Second#></span>
				</td>
			</tr>
			<tr id="tr_adv_keyingtries_p2">
				<th>Keyingtries<!--untranslated--></th>
				<td>
					<input type="text" class="input_6_table" name="ipsec_keyingtries" maxlength="2" value="3" onKeyPress="return validator.isNumber(this,event)">
				</td>
			</tr>
		</table>
		<div style="color:#FC0;margin:10px 0px;">Note: ASUS <#Web_Title2#> pre-configure the Diffie Hellman (DH) key change Group of phase 1 and phase 2 in auto mode, which support 2, 5, 14, 15, 16 and 18.<!--untranslated--></div>
	</div>
	<!-- Advanced Settings table end-->
	<!-- User table start-->
	<table id="tbAccountList" width="100%" border="1" align="center" cellpadding="4" cellspacing="0" class="FormTable_table" style="margin-top:15px;">
		<thead>
		<tr>
			<td colspan="3"><#Username_Pwd#>&nbsp;(<#List_limit#>&nbsp;8)</td>
		</tr>
		</thead>
		<tr>
			<th><#HSDPAConfig_Username_itemname#></th>
			<th><#HSDPAConfig_Password_itemname#></th>
			<th><#list_add_delete#></th>
		</tr>
		<tr>
			<td width="45%">
				<input type="text" class="input_25_table" maxlength="32" name="ipsec_client_list_username" onKeyPress="return validator.isString(this, event)">
			</td>
			<td width="45%">
				<input type="text" class="input_25_table" maxlength="32" name="ipsec_client_list_password" onKeyPress="return validator.isString(this, event)">
			</td>
			<td width="10%">
				<div><input type="button" class="add_btn" onClick="addRow_Group(8);" value=""></div>
			</td>
		</tr>
	</table>
	<div id="ipsec_client_list_Block"></div>
	<!-- User table end-->
	
	<div style="margin-top:15px;width:100%;text-align:center;">
		<input class="button_gen" type="button" onclick="cancel_ipsec_profile_panel();" value="<#CTL_Cancel#>">
		<input class="button_gen" type="button" onclick="save_ipsec_profile_panel();" value="<#CTL_onlysave#>">	
	</div>
</div>
<div id="connection_ipsec_profile_panel" class="ipsec_connect_status_panel"></div> 
</form>
<form method="post" name="list_form" id="list_form" action="/start_apply2.htm" target="hidden_frame">
	<input type="hidden" name="current_page" value="Advanced_VPN_IPSec.asp">
	<input type="hidden" name="next_page" value="Advanced_VPN_IPSec.asp">
	<input type="hidden" name="modified" value="0">
	<input type="hidden" name="flag" value="background">
	<input type="hidden" name="action_mode" value="apply">
	<input type="hidden" name="action_script" value="saveNvram">
	<input type="hidden" name="action_wait" value="1">
	<input type="hidden" name="ipsec_profile_1" value="">
	<input type="hidden" name="ipsec_profile_2" value="">
	<input type="hidden" name="ipsec_profile_3" value="">
	<input type="hidden" name="ipsec_profile_4" value="">
	<input type="hidden" name="ipsec_profile_5" value="">
	<input type="hidden" name="ipsec_client_list_1" value="">
	<input type="hidden" name="ipsec_client_list_2" value="">
	<input type="hidden" name="ipsec_client_list_3" value="">
	<input type="hidden" name="ipsec_client_list_4" value="">
	<input type="hidden" name="ipsec_client_list_5" value="">
	<input type="hidden" name="ipsec_ca_1" value="">
	<input type="hidden" name="ipsec_ca_2" value="">
	<input type="hidden" name="ipsec_ca_3" value="">
	<input type="hidden" name="ipsec_ca_4" value="">
	<input type="hidden" name="ipsec_ca_5" value="">
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
<input type="hidden" name="preferred_lang" id="preferred_lang" value="<% nvram_get("preferred_lang"); %>">
<input type="hidden" name="firmver" value="<% nvram_get("firmver"); %>">
<input type="hidden" name="clear_file_name" value="ipsec">
</form>
<div id="footer"></div>
</body>
</html>



