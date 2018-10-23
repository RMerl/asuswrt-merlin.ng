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
<title><#Web_Title#> - Subnet Management</title>
<link rel="stylesheet" type="text/css" href="index_style.css"> 
<link rel="stylesheet" type="text/css" href="form_style.css">
<link rel="stylesheet" type="text/css" href="device-map/device-map.css">
<script language="JavaScript" type="text/javascript" src="state.js"></script>
<script language="JavaScript" type="text/javascript" src="general.js"></script>
<script language="JavaScript" type="text/javascript" src="popup.js"></script>
<script language="JavaScript" type="text/javascript" src="help.js"></script>
<script language="JavaScript" type="text/javascript" src="client_function.js"></script>
<script language="JavaScript" type="text/javascript" src="validator.js"></script>
<script type="text/javaScript" src="/js/jquery.js"></script>
<script type="text/javascript" src="switcherplugin/jquery.iphone-switch.js"></script>
<script type="text/javaScript" src="js/subnet_rule.js"></script>
<style>
.contentM_connection{
	position:absolute;
	-webkit-border-radius: 5px;
	-moz-border-radius: 5px;
	border-radius: 5px;
	z-index:500;
	background-color:#2B373B;
	margin-left: 34%;
	margin-top: 10px;
	width:650px;
	display:none;
	box-shadow: 3px 3px 10px #000;
}
</style>

<script>
<% wanlink(); %>
var subnet_rulelist_array = new Array();
var default_lanip = '<% nvram_get("lan_ipaddr"); %>';
var default_dhcp_gateway_x = '<% nvram_get("dhcp_gateway_x"); %>';
var default_gateway = (default_dhcp_gateway_x == "")? default_lanip:default_dhcp_gateway_x;
var default_netmask = '<% nvram_get("lan_netmask"); %>';
var default_dhcp_enable = '<% nvram_get("dhcp_enable_x"); %>';
var default_dhcp_start = '<% nvram_get("dhcp_start"); %>';
var default_dhcp_end = '<% nvram_get("dhcp_end"); %>';
var default_dhcp_lease = '<% nvram_get("dhcp_lease"); %>';
var default_domain = '<% nvram_get("lan_domain"); %>';
var default_dns1 = '<% nvram_get("dhcp_dns1_x"); %>';
var default_wins_x = '<% nvram_get("dhcp_wins_x"); %>';
var default_dhcp_static = '<% nvram_get("dhcp_static_x"); %>';
if(captivePortal_support){
	var cp_net = '<% nvram_get("cp_net"); %>';
	var cp_ipaddr = cp_net.split('/')[0];
	var cp_netmsk_bits = parseInt(cp_net.split('/')[1]);
	var cp_netmask = bits_to_netmask(cp_netmsk_bits);
	var cp_lease = '<% nvram_get("cp_lease"); %>';
	var cp_ipaddr_array = cp_ipaddr.split('.');
	if(cp_ipaddr_array[3] == '0'){
		cp_ipaddr_array[3] = "1";
	}
	var cp_gateway = cp_ipaddr_array.join('.');
}

if(cp_freewifi_support){
	var chilli_net = '<% nvram_get("chilli_net"); %>';
	var chilli_ipaddr = chilli_net.split('/')[0];
	var chilli_netmsk_bits = parseInt(chilli_net.split('/')[1]);
	var chilli_netmask = bits_to_netmask(chilli_netmsk_bits);
	var chilli_lease = '<% nvram_get("chilli_lease"); %>';
	var chilli_ipaddr_array = chilli_ipaddr.split('.');
	if(chilli_ipaddr_array[3] == '0'){
		chilli_ipaddr_array[3] = "1";
	}
	var chilli_gateway = chilli_ipaddr_array.join('.');
}

if(ipsec_srv_support){
	var ipsec_subnet_array = [];
	var ipsec_profile_array = [];
	var ipsec_profile = "";
	var ipsec_subnet_string = "";
	var ipsec_ipaddr = "";
	var ipsec_netmask = "";
	var subnet_element = [];
	ipsec_profile = decodeURIComponent('<% nvram_char_to_ascii("", "ipsec_profile_1"); %>');
	if(ipsec_profile != "")
		ipsec_profile_array.push(ipsec_profile);
	ipsec_profile = decodeURIComponent('<% nvram_char_to_ascii("", "ipsec_profile_2"); %>');
	if(ipsec_profile != "")
		ipsec_profile_array.push(ipsec_profile);
	ipsec_profile = decodeURIComponent('<% nvram_char_to_ascii("", "ipsec_profile_3"); %>');
	if(ipsec_profile != "")
		ipsec_profile_array.push(ipsec_profile);
	ipsec_profile = decodeURIComponent('<% nvram_char_to_ascii("", "ipsec_profile_4"); %>');
	if(ipsec_profile != "")
		ipsec_profile_array.push(ipsec_profile);
	ipsec_profile = decodeURIComponent('<% nvram_char_to_ascii("", "ipsec_profile_5"); %>');
	if(ipsec_profile != "")
		ipsec_profile_array.push(ipsec_profile);

	Object.keys(ipsec_profile_array).forEach(function(key){
		var profile_array = ipsec_profile_array[key].split('>');
		if(profile_array[0] == '4'){
			ipsec_subnet_string = profile_array[14];
			ipsec_ipaddr = ipsec_subnet_string.split('/')[0];
			ipsec_netmask = bits_to_netmask(ipsec_subnet_string.split('/')[1]);
			subnet_element.push(ipsec_ipaddr);
			subnet_element.push(ipsec_netmask)
			ipsec_subnet_array.push(subnet_element);
		}
	});
}

for(var i = 1; i < subnet_rulelist_row.length; i ++) {
	var  subnet_rulelist_col = subnet_rulelist_row[i].split('>');
	if( i == 1){
		subnet_rulelist_array[0] = [default_gateway, default_netmask, default_dhcp_enable, default_dhcp_start, default_dhcp_end, default_dhcp_lease, default_domain, default_dns1, default_wins_x, default_dhcp_static, subnet_rulelist_col[10]];
	}
	else
		subnet_rulelist_array[i-1] = [subnet_rulelist_col[0], subnet_rulelist_col[1], subnet_rulelist_col[2], subnet_rulelist_col[3], subnet_rulelist_col[4],subnet_rulelist_col[5], subnet_rulelist_col[6], subnet_rulelist_col[7], subnet_rulelist_col[8], subnet_rulelist_col[9], subnet_rulelist_col[10]];
}
var selected_row = -1;
var old_tGatewayIP = "";
var old_subnetMask = "";
var manually_dhcp_list_array = new Array();
var dhcp_staticlist = "";
var MAX_SUBNET_NUM = 8;

var vlan_rulelist = decodeURIComponent("<% nvram_char_to_ascii("","vlan_rulelist"); %>");
var vlan_rulelist_row = vlan_rulelist.split('<');
var vlan_rulelist_array = new Array();
for(var i = 1; i < vlan_rulelist_row.length; i ++) {
	var  vlan_rulelist_col = vlan_rulelist_row[i].split('>');
	vlan_rulelist_array[i-1] = [vlan_rulelist_col[0], vlan_rulelist_col[1], vlan_rulelist_col[2], vlan_rulelist_col[3], vlan_rulelist_col[4],vlan_rulelist_col[5], vlan_rulelist_col[6], vlan_rulelist_col[7], vlan_rulelist_col[8], vlan_rulelist_col[9]];
}

var restart_net_and_phy = 0; //0: just save nvram  1: restart service

function initial(){
	show_menu();
	show_subnet_list();
	parse_LanToLanRoute_to_object();
}

function show_subnet_list(){
	var code = "";
	var wid = [19, 19, 19, 19, 10, 7, 7];
	var ip_range = [];

	code +='<table width="98%" align="center" cellpadding="4" cellspacing="0"  class="list_table" id="subnet_list_table">';

	if(subnet_rulelist_array.length == 0)
		code +='<tr><td style="color:#FFCC00;" colspan="8">No subnet rules</td>';
	else{
		Object.keys(subnet_rulelist_array).forEach(function(key){
			code += '<tr id="row'+key+'">';
			code += '<td width="'+wid[0]+'%">'+ subnet_rulelist_array[key][0] +'</td>';
			code += '<td width="'+wid[1]+'%">'+ subnet_rulelist_array[key][1] +'</td>';
			code += '<td width="'+wid[2]+'%">'+ subnet_rulelist_array[key][3] +'</td>';
			code += '<td width="'+wid[3]+'%">'+ subnet_rulelist_array[key][4] +'</td>';
			code += '<td width="'+wid[4]+'%">'+ subnet_rulelist_array[key][5] +'</td>';
			if(key == 0)
				code += '<td colspan = "2">'+'<input class="edit_btn" onclick="edit_subnet(this)" /></td>';
			else{
				code += '<td width="'+wid[5]+'%">'+'<input class="edit_btn" onclick="edit_subnet(this)" /></td>';
				code += '<td width="'+wid[6]+'%">'+'<input class="remove_btn" onclick="del_subnet(this);" /></td>';
			}
			code += '</tr>';
		});
	}

	/* Show subnet information of Captive Portal */
	if(captivePortal_support){
		ip_range = calculatorIPPoolRange(cp_gateway, cp_netmask).split('>');
		code += '<tr id="cp_subnet">';
		code += '<td width="'+wid[0]+'%">'+ cp_gateway +'</td>';
		code += '<td width="'+wid[1]+'%">'+ cp_netmask +'</td>';
		code += '<td width="'+wid[2]+'%">'+ ip_range[0] +'</td>';
		code += '<td width="'+wid[3]+'%">'+ ip_range[1] +'</td>';
		code += '<td width="'+wid[4]+'%">'+ cp_lease +'</td>';
		code += '<td colspan = "2">'+'<#Captive_Portal#>'+'</td>';
		code += '</tr>';
	}

	/* Show subnet information of Free WIFI */
	if(cp_freewifi_support){
		ip_range = calculatorIPPoolRange(chilli_gateway, chilli_netmask).split('>');
		code += '<tr id="cp_subnet">';
		code += '<td width="'+wid[0]+'%">'+ chilli_gateway +'</td>';
		code += '<td width="'+wid[1]+'%">'+ chilli_netmask +'</td>';
		code += '<td width="'+wid[2]+'%">'+ ip_range[0] +'</td>';
		code += '<td width="'+wid[3]+'%">'+ ip_range[1] +'</td>';
		code += '<td width="'+wid[4]+'%">'+ chilli_lease +'</td>';
		code += '<td colspan = "2"> Free Wi-Fi </td>';
		code += '</tr>';
	}

	/* Show subnet information of IPSec Server */
	if(ipsec_srv_support){
		Object.keys(ipsec_subnet_array).forEach(function(key){
			ip_range = calculatorIPPoolRange(ipsec_subnet_array[key][0], ipsec_subnet_array[key][1]).split('>');
			code += '<tr id="ipsec_subnet_' + key + '">';
			code += '<td width="'+wid[0]+'%">'+ '-' +'</td>';
			code += '<td width="'+wid[1]+'%">'+ ipsec_subnet_array[key][1] +'</td>';
			code += '<td width="'+wid[2]+'%">'+ ip_range[0] +'</td>';
			code += '<td width="'+wid[3]+'%">'+ ip_range[1] +'</td>';
			code += '<td width="'+wid[4]+'%">'+ '-' +'</td>';
			code += '<td colspan = "2"> IPSec Server</td>';
			code += '</tr>';
		});
	}

	code +='</table>';
	document.getElementById('subnet_list_Block').innerHTML = code;

	if(subnet_rulelist_array.length < MAX_SUBNET_NUM){
		document.getElementById("add_subnet_btn").className = "add_btn";
		document.getElementById("add_subnet_btn").disabled = false;
	}
	else{
		document.getElementById("add_subnet_btn").className = "add_btn_disabled";
		document.getElementById("add_subnet_btn").disabled = true;
	}

}

function add_subnet(){
	selected_row = -1;
	document.form.radioDHCPEnable[0].checked = true;
	document.form.tLan_domain.value = "";
	document.form.tGatewayIP.value = "";
	old_tGatewayIP = "";
	old_subnetMask = "";
	document.form.tSubnetMask.value = "";
	document.form.tDHCPStart.value = "";
	document.form.tDHCPEnd.value = "";
	document.form.tLeaseTime.value = "86400";
	document.form.dhcp_dns.value = "";
	document.form.dhcp_wins.value = "";
	document.form.dhcp_static[1].checked = true;
	manually_dhcp_list_array = [];
	show_subnet_edit(1);
	get_LanToLanRoute();
}

function get_vlan_rule_index(gatewayIP){
	for(var i = 0; i < vlan_rulelist_array.length; i++){
		if(vlan_rulelist_array[i][7] != "default" ){
			var vlan_getwayIP =  vlan_rulelist_array[i][7].substring(0, vlan_rulelist_array[i][7].indexOf('/'));
			if(vlan_getwayIP == gatewayIP)
				return i;
		}
	}
	return -1;
}

function edit_subnet(r){
	var subnet_list_table = document.getElementById('subnet_list_table');
	var subnet_netmask = "";

	selected_row = r.parentNode.parentNode.rowIndex;

	if(subnet_rulelist_array[selected_row][2] == "1")
		document.form.radioDHCPEnable[0].checked = true;
	else
		document.form.radioDHCPEnable[1].checked = true;
	document.form.tGatewayIP.value = subnet_rulelist_array[selected_row][0];
	old_tGatewayIP = subnet_rulelist_array[selected_row][0];
	old_subnetMask = subnet_rulelist_array[selected_row][1];
	document.form.tSubnetMask.value = subnet_rulelist_array[selected_row][1];
	document.form.tDHCPStart.value = subnet_rulelist_array[selected_row][3];
	document.form.tDHCPEnd.value = subnet_rulelist_array[selected_row][4];
	document.form.tLeaseTime.value = subnet_rulelist_array[selected_row][5];
	document.form.tLan_domain.value = subnet_rulelist_array[selected_row][6];
	document.form.dhcp_dns.value = subnet_rulelist_array[selected_row][7];
	document.form.dhcp_wins.value = subnet_rulelist_array[selected_row][8];
	if(subnet_rulelist_array[selected_row][9] == "1")
		document.form.dhcp_static[0].checked = true;
	else
		document.form.dhcp_static[1].checked = true;
	var dhcp_staticlist_row = subnet_rulelist_array[selected_row][10].split(';');
	manually_dhcp_list_array = [];
	if(subnet_rulelist_array[selected_row][10] != ""){
		for(var i = 0; i < dhcp_staticlist_row.length; i += 1) {
			var dhcp_staticlist_col = dhcp_staticlist_row[i].split(' ');
			manually_dhcp_list_array[dhcp_staticlist_col[1]] = dhcp_staticlist_col[0];
		}
	}

	subnet_netmask = subnet_rulelist_array[selected_row][0] + '/' + netmask_to_bits(subnet_rulelist_array[selected_row][1]);
	get_LanToLanRoute(subnet_netmask);

	show_subnet_edit(1, 1);
}

function del_subnet(r){
	var subnet_list_table = document.getElementById('subnet_list_table');
	var index = r.parentNode.parentNode.rowIndex;
	var subnet_netmask = subnet_list_table.rows[index].cells[0].innerHTML + '/' + netmask_to_bits(subnet_list_table.rows[index].cells[1 ].innerHTML);

	var vlan_rule_index = get_vlan_rule_index(subnet_list_table.rows[index].cells[0].innerHTML);
	if(vlan_rule_index != -1){
		alert("<#MSubnet_Warning#>");
		return;
	}

	subnet_rulelist_array.splice(index, 1);
	subnet_list_table.deleteRow(index);
	rm_LanToLanRoute_rule(subnet_netmask);
	update_subnet_rulelist();

	if(subnet_rulelist_array.length < MAX_SUBNET_NUM){
		document.getElementById("add_subnet_btn").className = "add_btn";
		document.getElementById("add_subnet_btn").disabled = false;
	}
	else{
		document.getElementById("add_subnet_btn").className = "add_btn_disabled";
		document.getElementById("add_subnet_btn").disabled = true;
	}
}

function update_subnet_rulelist(){
	var subnet_rulelist_value = "";

	Object.keys(subnet_rulelist_array).forEach(function(key) {
			subnet_rulelist_value += "<" + subnet_rulelist_array[key][0] + ">" + subnet_rulelist_array[key][1] + ">" + subnet_rulelist_array[key][2]+ ">" + subnet_rulelist_array[key][3] + ">" + subnet_rulelist_array[key][4]+ ">" + subnet_rulelist_array[key][5]+ ">" + subnet_rulelist_array[key][6]+ ">" + subnet_rulelist_array[key][7]+ ">" + subnet_rulelist_array[key][8]+ ">" + subnet_rulelist_array[key][9]+ ">" + subnet_rulelist_array[key][10];
		});

	subnet_rulelist = subnet_rulelist_value;
	update_all_dhcp_staticlist();
}

function update_vlan_rulelist(){
	var vlan_rulelist_col = [];

	Object.keys(vlan_rulelist_array).forEach(function(key) {
		vlan_rulelist_col.push(vlan_rulelist_array[key].join('>'));
	});
	vlan_rulelist = '<'+vlan_rulelist_col.join('<');
}

function applyRule(){
		update_subnet_rulelist();
		update_vlan_rulelist();
		document.form.subnet_rulelist.value = subnet_rulelist;
		document.form.vlan_rulelist.value = vlan_rulelist;
		save_LanToLanRoute();
		document.form.subnet_rulelist_ext.value = subnet_rulelist_ext;
		showLoading();
		document.form.submit();
}

function show_subnet_edit(show, edit){
	if(show == 0){
		$("#subnet_div").fadeOut(300);
	}
	else{
		if(edit){
			document.getElementById("saveButton").onclick = function(){
				change_subnet_rule();
			};
		}
		else{
			document.getElementById("saveButton").onclick = function(){
				add_new_subnet();
			};
		}

		if(selected_row == 0){//default
			document.getElementById("max_staticIp_num").innerHTML = "64";
			document.getElementById("addStatic_Btn").onclick = function(){
				addRow_Group(64);
			};
		}	
		else{
			document.getElementById("max_staticIp_num").innerHTML = "8";
			document.getElementById("addStatic_Btn").onclick = function(){
				addRow_Group(8);
			};
		}

		setTimeout("showdhcp_staticlist();", 100);
		setTimeout("showDropdownClientList('setClientIP', 'mac>ip', 'all', 'ClientList_Block_PC', 'pull_arrow', 'all');", 1000);
		$("#subnet_div").fadeIn(300);
	}
}

function add_new_subnet(){
	if(validSubnetForm()){
		var subnet_netmask = document.form.tGatewayIP.value + '/'+ netmask_to_bits(document.form.tSubnetMask.value);

		update_dhcp_staticlist();
		var new_subnet = [document.form.tGatewayIP.value, document.form.tSubnetMask.value, (document.form.radioDHCPEnable[0].checked)? "1":"0",
							document.form.tDHCPStart.value, document.form.tDHCPEnd.value, document.form.tLeaseTime.value, document.form.tLan_domain.value,
							document.form.dhcp_dns.value, document.form.dhcp_wins.value, (document.form.dhcp_static[0].checked)? "1":"0", dhcp_staticlist];
		subnet_rulelist_array.push(new_subnet);

		update_LanToLanRoute_array(subnet_netmask);
		show_subnet_list();
		show_subnet_edit(0);

		if(subnet_rulelist_array.length < MAX_SUBNET_NUM){
			document.getElementById("add_subnet_btn").className = "add_btn";
			document.getElementById("add_subnet_btn").disabled = false;
		}
		else{
			document.getElementById("add_subnet_btn").className = "add_btn_disabled";
			document.getElementById("add_subnet_btn").disabled = true;
		}
	}
}

function update_default_subnet(){
	var lan_ipaddr_array = default_lanip.split(".");
	var gateway_ipaddr_array = subnet_rulelist_array[0][0].split(".");
	var change = 0;

	document.form.dhcp_enable_x.value = subnet_rulelist_array[0][2];
	document.form.lan_domain.value = subnet_rulelist_array[0][6];
	if(default_dhcp_gateway_x != "")
		document.form.dhcp_gateway_x.value = subnet_rulelist_array[0][0];
	document.form.lan_netmask.value = subnet_rulelist_array[0][1];
	document.form.dhcp_start.value = subnet_rulelist_array[0][3];
	document.form.dhcp_end.value = subnet_rulelist_array[0][4];
	document.form.dhcp_lease.value = subnet_rulelist_array[0][5];
	document.form.dhcp_dns1_x.value = subnet_rulelist_array[0][7];
	document.form.dhcp_wins_x.value = subnet_rulelist_array[0][8];
	document.form.dhcp_static_x.value = subnet_rulelist_array[0][9];

	for(var i = 0; i < 3; i++){
		if(lan_ipaddr_array[i] != gateway_ipaddr_array[i]){
			change = 1;
			break;
		}
	}

	if(change){
		gateway_ipaddr_array[3] = "1";
		document.form.lan_ipaddr.value = gateway_ipaddr_array.join(".");
	}
}

function change_subnet_rule(){
	if(validSubnetForm()){
		var subnet_netmask = document.form.tGatewayIP.value + '/'+ netmask_to_bits(document.form.tSubnetMask.value);

		subnet_rulelist_array[selected_row][0] = document.form.tGatewayIP.value;
		subnet_rulelist_array[selected_row][1] = document.form.tSubnetMask.value;
		subnet_rulelist_array[selected_row][2] = (document.form.radioDHCPEnable[0].checked)? "1":"0";
		subnet_rulelist_array[selected_row][3] = document.form.tDHCPStart.value;
		subnet_rulelist_array[selected_row][4] = document.form.tDHCPEnd.value;
		subnet_rulelist_array[selected_row][5] = document.form.tLeaseTime.value;
		subnet_rulelist_array[selected_row][6] = document.form.tLan_domain.value;
		subnet_rulelist_array[selected_row][7] = document.form.dhcp_dns.value;
		subnet_rulelist_array[selected_row][8] = document.form.dhcp_wins.value;
		subnet_rulelist_array[selected_row][9] = (document.form.dhcp_static[0].checked)? "1":"0";
		
		update_dhcp_staticlist();
		subnet_rulelist_array[selected_row][10] = dhcp_staticlist;
		if(selected_row == 0){
			update_default_subnet();
		}

		//update vlan_rulelist
		if(old_tGatewayIP != "" && (old_tGatewayIP != document.form.tGatewayIP.value || old_subnetMask != document.form.tSubnetMask.value)){
			var vlan_rule_index = get_vlan_rule_index(old_tGatewayIP);
			if(vlan_rule_index != -1){
				vlan_rulelist_array[vlan_rule_index][7] = document.form.tGatewayIP.value + '/' + netmask_to_bits(document.form.tSubnetMask.value);
			}
		}

		update_LanToLanRoute_array(subnet_netmask);
		show_subnet_list();
		show_subnet_edit(0);
	}
}

var alertMsg = function (type, ipAddr, netStart, netEnd) {
		alert("*Conflict with " + type + " IP: " + ipAddr + ",\n" + "Network segment is " + netStart + " ~ " + netEnd);
};

function validSubnetForm() {
	var re = new RegExp('^[a-zA-Z0-9][a-zA-Z0-9\.\-]*[a-zA-Z0-9]$','gi');
	if((!re.test(document.form.tLan_domain.value) || document.form.tLan_domain.value.indexOf("asuscomm.com") > 0) && document.form.tLan_domain.value != ""){
      alert("<#JS_validchar#>");
      document.form.tLan_domain.focus();
      document.form.tLan_domain.select();
	 	return false;
  	}

	if(old_tGatewayIP != document.form.tGatewayIP.value){
		if(!validator.isLegalIP(document.form.tGatewayIP)) {
			document.form.tGatewayIP.focus();
			document.form.tGatewayIP.select();
			return false;
		}

		if(!checkGatewayIP()) {
			document.form.tGatewayIP.focus();
			document.form.tGatewayIP.select();
			return false;
		}
	}

	if(document.form.tSubnetMask.value == "") {
		alert("<#JS_fieldblank#>");
		document.form.tSubnetMask.focus();
		document.form.tSubnetMask.select();
		return false;
	}
	else if(!validator.isLegalMask(document.form.tSubnetMask)) {
		return false;
	}

	if(!validate_dhcp_range(document.form.tDHCPStart)
			|| !validate_dhcp_range(document.form.tDHCPEnd))
		return false;

	if(!validator.numberRange(document.form.tLeaseTime, 120, 604800)) {
		return false;
	}

	/* Check overlap of DHCP Range with other subnets */
	var lanIPAddr = document.form.tGatewayIP.value;
	var lanNetMask = document.form.tSubnetMask.value;
	for(var i = 0; i < subnet_rulelist_array.length; i++) {
		if( i != selected_row){
			var ipConflict = checkIPConflict("", lanIPAddr, lanNetMask, subnet_rulelist_array[i][0], subnet_rulelist_array[i][1]);
			if(ipConflict.state) {
				alertMsg("Subnet", ipConflict.ipAddr, ipConflict.netLegalRangeStart, ipConflict.netLegalRangeEnd);
				return false;
			}
		}
	}

	/* Check overlap of DHCP Range with Captive Portal, Free WiFi, and IPSec Server */
	if(captivePortal_support){
		var ipConflict_cp = checkIPConflict("", lanIPAddr, lanNetMask, cp_gateway, cp_netmask);
		if(ipConflict_cp.state) {
			alertMsg("<#Captive_Portal#>", ipConflict_cp.ipAddr, ipConflict_cp.netLegalRangeStart, ipConflict_cp.netLegalRangeEnd);
			return false;
		}
	}

	if(cp_freewifi_support){
		var ipConflict_chilli = checkIPConflict("", lanIPAddr, lanNetMask, chilli_gateway, chilli_netmask);
		if(ipConflict_chilli.state) {
			alertMsg("Free Wi-Fi", ipConflict_chilli.ipAddr, ipConflict_chilli.netLegalRangeStart, ipConflict_chilli.netLegalRangeEnd);
			return false;
		}
	}

	if(ipsec_srv_support){
		var ipsec_conflict = false;
		Object.keys(ipsec_subnet_array).forEach(function(key){
			var ipConflict_ipsec = checkIPConflict("", lanIPAddr, lanNetMask, ipsec_subnet_array[key][0], ipsec_subnet_array[key][1]);
			if(ipConflict_ipsec.state) {
				alertMsg("IPSec Server", ipConflict_ipsec.ipAddr, ipConflict_ipsec.netLegalRangeStart, ipConflict_ipsec.netLegalRangeEnd);
				ipsec_conflict = true;
			}
		});

		if(ipsec_conflict)
			return false;
	}

	return true;
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
	
	subnet_head = getSubnet(document.form.tGatewayIP.value, document.form.tSubnetMask.value, "head");
	subnet_end = getSubnet(document.form.tGatewayIP.value, document.form.tSubnetMask.value, "end");
	
	if(ip_num <= subnet_head || ip_num >= subnet_end){
		alert(ip_obj.value+" <#JS_validip#>");
		ip_obj.value = "";
		ip_obj.focus();
		ip_obj.select();
		return 0;
	}
	
	return 1;
}

function calculatorIPPoolRange(ipaddr, netmask) {
	var gatewayIPArray = ipaddr.split(".");
	var netMaskArray = netmask.split(".");
	var ipPoolStartArray  = new Array();
	var ipPoolEndArray  = new Array();
	var ipPoolStart = "";
	var ipPoolEnd = "";

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

	ipPoolStart = ipPoolStartArray[0] + "." + ipPoolStartArray[1] + "." + ipPoolStartArray[2] + "." + ipPoolStartArray[3];
	if(inet_network(ipPoolStart) <= inet_network(ipaddr)) {
		ipPoolStart = ipPoolStartArray[0] + "." + ipPoolStartArray[1] + "." + ipPoolStartArray[2] + "." + (parseInt(ipPoolStartArray[3]) + 1);
	}
	ipPoolEnd = ipPoolEndArray[0] + "." + ipPoolEndArray[1] + "." + ipPoolEndArray[2] + "." + ipPoolEndArray[3];

	return ipPoolStart + ">" + ipPoolEnd;
}

function checkIPLegality() {
	//check IP legal
	if(document.form.tGatewayIP.value !== "") {
		if(!validator.isLegalIP(document.form.tGatewayIP)) {
			document.form.tGatewayIP.focus();
			document.form.tGatewayIP.select();
			return false;
		}
	}

	//setting IP pool range
	if(document.form.tGatewayIP.value !== "" && document.form.tSubnetMask.value !== "") {
		var ipPoolRangeArray = calculatorIPPoolRange(document.form.tGatewayIP.value, document.form.tSubnetMask.value).split(">");
		document.form.tDHCPStart.value = ipPoolRangeArray[0];
		document.form.tDHCPEnd.value = ipPoolRangeArray[1];
	}
}

function checkMaskLegality() {
	//check IP legal
	if(document.form.tSubnetMask.value !== "") {
		if(!validator.isLegalMask(document.form.tSubnetMask)) {
			return false;
		}
	}
	
	//setting IP pool range
	if(document.form.tGatewayIP.value !== "" && document.form.tSubnetMask.value !== "") {
		var ipPoolRangeArray = calculatorIPPoolRange(document.form.tGatewayIP.value, document.form.tSubnetMask.value).split(">");
		document.form.tDHCPStart.value = ipPoolRangeArray[0];
		document.form.tDHCPEnd.value = ipPoolRangeArray[1];
	}
}

function checkGatewayIP() {
	var lanIPAddr = document.form.tGatewayIP.value;
	var lanNetMask = document.form.tSubnetMask.value;
	var ipConflict;

	//1.check Wan IP
	ipConflict = checkIPConflict("WAN", lanIPAddr, lanNetMask);
	if(ipConflict.state) {
		alertMsg("WAN", ipConflict.ipAddr, ipConflict.netLegalRangeStart, ipConflict.netLegalRangeEnd);
		return false;
	}

	//2.check Lan IP  Default Subnet is subnet_rulelist_array[0]
	if(selected_row != 0){
		ipConflict = checkIPConflict("", lanIPAddr, lanNetMask, subnet_rulelist_array[0][0], subnet_rulelist_array[0][1]);
		if(ipConflict.state) {
			alertMsg("LAN", ipConflict.ipAddr, ipConflict.netLegalRangeStart, ipConflict.netLegalRangeEnd);
			return false;
		}
	}

	//3.check PPTP
	if(pptpd_support) {
		ipConflict = checkIPConflict("PPTP", lanIPAddr, lanNetMask);
		if(ipConflict.state) {
			alertMsg("PPTP", ipConflict.ipAddr, ipConflict.netLegalRangeStart, ipConflict.netLegalRangeEnd);
			return false;
		}
	}

	//4.check OpenVPN
	if(openvpnd_support) {
		ipConflict = checkIPConflict("OpenVPN", lanIPAddr, lanNetMask);
		if(ipConflict.state) {
			alertMsg("OpenVPN", ipConflict.ipAddr, ipConflict.netLegalRangeStart, ipConflict.netLegalRangeEnd);
			return false;
		}
	}

	//5.check existed Subnet IP address except default subnet (LAN)
	for(var i = 1; i < subnet_rulelist_array.length; i++) {
		if( i != selected_row){
			ipConflict = checkIPConflict("", lanIPAddr, lanNetMask, subnet_rulelist_array[i][0], subnet_rulelist_array[i][1]);
			if(ipConflict.state) {
				alertMsg("Subnet", ipConflict.ipAddr, ipConflict.netLegalRangeStart, ipConflict.netLegalRangeEnd);
				return false;
			}
		}
	}

	return true;
}

function showdhcp_staticlist(){
	var code = "";
	var clientListEventData = [];
	code += '<table width="100%" cellspacing="0" cellpadding="4" align="center" class="list_table" id="dhcp_staticlist_table">';
	if(Object.keys(manually_dhcp_list_array).length == 0)
		code += '<tr><td style="color:#FFCC00;"><#IPConnection_VSList_Norule#></td></tr>';
	else {
		//user icon
		var userIconBase64 = "NoIcon";
		var clientName, deviceType, deviceVender;
		Object.keys(manually_dhcp_list_array).forEach(function(key) {
			var clientMac = manually_dhcp_list_array[key].toUpperCase();
			var clientIconID = "clientIcon_" + clientMac.replace(/\:/g, "");
			var clientIP = key;
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
			code += '<tr><td width="60%" align="center">';
			code += '<table style="width:100%;"><tr><td style="width:40%;height:56px;border:0px;float:right;">';
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
			code += '</td><td style="width:60%;border:0px;">';
			code += '<div>' + clientName + '</div>';
			code += '<div>' + clientMac + '</div>';
			code += '</td></tr></table>';
			code += '</td>';
			code += '<td width="30%">'+ clientIP +'</td>';
			code += '<td width="10%">';
			code += '<input class="remove_btn" onclick="del_Row(this);" value=""/></td></tr>';
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

function del_Row(r){
	var i = r.parentNode.parentNode.rowIndex;
	var delIP = document.getElementById('dhcp_staticlist_table').rows[i].cells[1].innerHTML;

	delete manually_dhcp_list_array[delIP];
	document.getElementById('dhcp_staticlist_table').deleteRow(i);

	if(Object.keys(manually_dhcp_list_array).length == 0)
		showdhcp_staticlist();
}

function addRow_Group(upper){ //dhcp_staticlist
	var rule_num = document.getElementById('dhcp_staticlist_table').rows.length;
	var item_num = document.getElementById('dhcp_staticlist_table').rows[0].cells.length;
	var mac_duplicate = 0, ip_duplicate = 0;

	if(rule_num >= upper){
		alert("<#JS_itemlimit1#> " + upper + " <#JS_itemlimit2#>");
		return false;
	}
		
	if(document.form.dhcp_staticmac_x_0.value ==""){
		alert("<#JS_fieldblank#>");
		document.form.dhcp_staticmac_x_0.focus();
		document.form.dhcp_staticmac_x_0.select();
		return false;
	}else if(document.form.dhcp_staticip_x_0.value ==""){
		alert("<#JS_fieldblank#>");
		document.form.dhcp_staticip_x_0.focus();
		document.form.dhcp_staticip_x_0.select();
		return false;
	}else if(check_macaddr(document.form.dhcp_staticmac_x_0, check_hwaddr_flag(document.form.dhcp_staticmac_x_0)) == true &&
		 validator.validIPForm(document.form.dhcp_staticip_x_0,0) == true &&
		 validate_dhcp_range(document.form.dhcp_staticip_x_0) == true){

		Object.keys(manually_dhcp_list_array).forEach(function(key){
			if(!mac_duplicate && manually_dhcp_list_array[key] == document.form.dhcp_staticmac_x_0.value.toUpperCase()){
				mac_duplicate = 1;
			}

			if(!ip_duplicate && document.form.dhcp_staticip_x_0.value == key){
				ip_duplicate = 1;
			}
		});

		if(mac_duplicate || ip_duplicate){
			if(mac_duplicate){
				document.form.dhcp_staticmac_x_0.focus();
				document.form.dhcp_staticmac_x_0.select();
			}
			else if(ip_duplicate){
				document.form.dhcp_staticip_x_0.focus();
				document.form.dhcp_staticip_x_0.select();
			}

			alert("<#JS_duplicate#>");
			return false;
		}

		manually_dhcp_list_array[document.form.dhcp_staticip_x_0.value.toUpperCase()] = document.form.dhcp_staticmac_x_0.value;
		document.form.dhcp_staticip_x_0.value = "";
		document.form.dhcp_staticmac_x_0.value = "";
		showdhcp_staticlist();
	}else{
		return false;
	}
}

function update_dhcp_staticlist(){
	var num = Object.keys(manually_dhcp_list_array).length;
	dhcp_staticlist = "";

	Object.keys(manually_dhcp_list_array).forEach(function(key) {
		dhcp_staticlist += manually_dhcp_list_array[key] + " "  + key;
		num--;
		if(num > 0)
			dhcp_staticlist += ';';
	});
}

function update_all_dhcp_staticlist(){ //update value of nvram variable "dhcp_staticlist"
	var default_dhcp_staticlist = "";

	Object.keys(subnet_rulelist_array).forEach(function(key) {
		if(subnet_rulelist_array[key][9] == "1"){
			var staticlist_array = subnet_rulelist_array[key][10].split(';');
			Object.keys(staticlist_array).forEach(function(key) {
				var mac_ip_pair = staticlist_array[key].split(' ');
				default_dhcp_staticlist += "<" + mac_ip_pair[0] + ">" + mac_ip_pair[1];
			});
		}
	});

	document.form.dhcp_staticlist.value = default_dhcp_staticlist;
}

function check_macaddr(obj,flag){ //control hint of input mac address

	if(flag == 1){
		var childsel=document.createElement("div");
		childsel.setAttribute("id","check_mac");
		childsel.style.color="#FFCC00";
		obj.parentNode.appendChild(childsel);
		document.getElementById("check_mac").innerHTML="<#LANHostConfig_ManualDHCPMacaddr_itemdesc#>";
		document.getElementById("check_mac").style.display = "";
		obj.focus();
		obj.select();
		return false;
	}else if(flag == 2){
		var childsel=document.createElement("div");
		childsel.setAttribute("id","check_mac");
		childsel.style.color="#FFCC00";
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

function setClientIP(macaddr, ipaddr){
	document.form.dhcp_staticmac_x_0.value = macaddr;
	document.form.dhcp_staticip_x_0.value = ipaddr;
	hideClients_Block();
}

function hideClients_Block(){
	document.getElementById("pull_arrow").src = "images/arrow-down.gif";
	document.getElementById('ClientList_Block_PC').style.display='none';
}

function pullLANIPList(obj){
	var element = document.getElementById('ClientList_Block_PC');
	var isMenuopen = element.offsetWidth > 0 || element.offsetHeight > 0;
	if(isMenuopen == 0){
		obj.src = "images/arrow-top.gif"
		element.style.display = 'block';
		document.form.dhcp_staticmac_x_0.focus();
	}
	else
		hideClients_Block();
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
<input type="hidden" name="current_page" value="Advanced_MultiSubnet_Content.asp">
<input type="hidden" name="next_page" value="Advanced_MultiSubnet_Content.asp">
<input type="hidden" name="group_id" value="">
<input type="hidden" name="modified" value="0">
<input type="hidden" name="action_mode" value="apply">
<input type="hidden" name="action_script" value="restart_net_and_phy">
<input type="hidden" name="action_wait" value="15">
<input type="hidden" name="preferred_lang" id="preferred_lang" value="<% nvram_get("preferred_lang"); %>">
<input type="hidden" name="firmver" value="<% nvram_get("firmver"); %>">
<input type="hidden" name="subnet_rulelist" value="">
<input type="hidden" name="subnet_rulelist_ext" value='<% nvram_get("subnet_rulelist_ext"); %>'>
<input type="hidden" name="vlan_rulelist" value="">
<input type="hidden" name="dhcp_enable_x" value='<% nvram_get("dhcp_enable_x"); %>'>
<input type="hidden" name="lan_domain" value='<% nvram_get("lan_domain"); %>'>
<input type="hidden" name="dhcp_start" value='<% nvram_get("dhcp_start"); %>'>
<input type="hidden" name="dhcp_end" value='<% nvram_get("dhcp_end"); %>'>
<input type="hidden" name="dhcp_lease" value='<% nvram_get("dhcp_lease"); %>'>
<input type="hidden" name="lan_ipaddr" value='<% nvram_get("lan_ipaddr"); %>'>
<input type="hidden" name="dhcp_gateway_x" value='<% nvram_get("dhcp_gateway_x"); %>'>
<input type="hidden" name="lan_netmask" value='<% nvram_get("lan_netmask"); %>'>
<input type="hidden" name="dhcp_dns1_x" value='<% nvram_get("dhcp_dns1_x"); %>'>
<input type="hidden" name="dhcp_wins_x" value='<% nvram_get("dhcp_wins_x"); %>'>
<input type="hidden" name="dhcp_static_x" value='<% nvram_get("dhcp_static_x"); %>'>
<input type="hidden" name="dhcp_staticlist" value=''>
<div id="subnet_div" class="contentM_connection" style="z-index:600;">
	<table border="0" align="center" cellpadding="5" cellspacing="5">
		<tr>
			<td align="left">
			<span class="formfonttitle"><#TBVLAN_EditSubnetProfile#></span>
			<div style="width:630px; height:2px;overflow:hidden;position:relative;left:0px;top:5px;" class="splitLine"></div>
			</td>
		</tr>
		<tr>
			<td>
				<table width="100%" border="1" align="center" cellpadding="4" cellspacing="0" class="FormTable">
					<thead>
						<tr>
							<td colspan="2"><#t2BC#></td>
						</tr>
					</thead>
					<tr>
						<th><#LANHostConfig_DHCPServerConfigurable_itemname#></th>
						<td>
							<input type="radio" value="1" name="radioDHCPEnable" class="content_input_fd" checked><#checkbox_Yes#>
							<input type="radio" value="0" name="radioDHCPEnable" class="content_input_fd"><#checkbox_No#>
						</td>
					</tr>
					<tr>
					<th><a class="hintstyle" href="javascript:void(0);" onClick="openHint(5,2);"><#LANHostConfig_DomainName_itemname#></a></th>
					<td>
					  <input type="text" maxlength="32" class="input_25_table" name="tLan_domain" value="" autocorrect="off" autocapitalize="off">
					</td>
					</tr>
					<tr>
						<th><#IPConnection_x_ExternalGateway_itemname#></th>
						<td>
							<input type="text" maxlength="15" class="input_25_table" name="tGatewayIP" onchange="checkIPLegality();" onKeyPress="return validator.isIPAddr(this,event);"  autocorrect="off" autocapitalize="off">
						</td>
					</tr>
					<tr>
						<th><#IPConnection_x_ExternalSubnetMask_itemname#></th>
						<td>
							<input type="text" maxlength="15" class="input_25_table" name="tSubnetMask" onchange="checkMaskLegality();" onKeyPress="return validator.isIPAddr(this,event);" autocorrect="off" autocapitalize="off">
						</td>
					</tr>
					<tr>
						<th><#LANHostConfig_MinAddress_itemname#></th>
						<td>
							<input type="text" maxlength="15" class="input_25_table" name="tDHCPStart"  onKeyPress="return validator.isIPAddr(this,event);">
						</td>
					</tr>
					<tr>
						<th><#LANHostConfig_MaxAddress_itemname#></th>
						<td>
							<input type="text" maxlength="15" class="input_25_table" name="tDHCPEnd"  onKeyPress="return validator.isIPAddr(this,event);">
						</td>
					</tr>
					<tr>
						<th><#LANHostConfig_LeaseTime_itemname#></th>
						<td>
							<input type="text" maxlength="6" class="input_25_table" name="tLeaseTime" onKeyPress="return validator.isNumber(this,event);">
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
					<th width="200"><a class="hintstyle" href="javascript:void(0);" onClick="openHint(5,7);"><#LANHostConfig_x_LDNSServer1_itemname#></a></th>
					<td>
					  <input type="text" maxlength="15" class="input_15_table" name="dhcp_dns" value="" onKeyPress="return validator.isIPAddr(this,event)" autocorrect="off" autocapitalize="off">
					  <div id="yadns_hint" style="display:none;"></div>
					</td>
				  </tr>
				  
				  <tr>
					<th><a class="hintstyle" href="javascript:void(0);" onClick="openHint(5,8);"><#LANHostConfig_x_WINSServer_itemname#></a></th>
					<td>
					  <input type="text" maxlength="15" class="input_15_table" name="dhcp_wins" value="" onkeypress="return validator.isIPAddr(this,event)" autocorrect="off" autocapitalize="off"/>
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
						<input type="radio" value="1" name="dhcp_static" /><#checkbox_Yes#>
						<input type="radio" value="0" name="dhcp_static" /><#checkbox_No#>
					</td>
				  </tr>
				</table>
				<table width="100%" border="1" align="center" cellpadding="4" cellspacing="0" class="FormTable_table" style="margin-top:8px;">
				  	<thead>
				  		<tr>
							<td colspan="3" id="GWStatic"><#LANHostConfig_ManualDHCPList_groupitemdesc#>&nbsp;(<#List_limit#>&nbsp;<span id="max_staticIp_num" style="color:#FFF;">8</span>)</td>
				  		</tr>
				  	</thead>

				  	<tr>
			  			<th><a class="hintstyle" href="javascript:void(0);" onClick="openHint(5,10);"><#Client_Name#> (<#PPPConnection_x_MacAddressForISP_itemname#>)</a></th>
		        		<th><#IPConnection_ExternalIPAddress_itemname#></th>
		        		<th><#list_add_delete#></th>
				  	</tr>			  
				  	<tr>
			  			<!-- client info -->
	        			<td width="60%">
							<input type="text" class="input_20_table" maxlength="17" name="dhcp_staticmac_x_0" style="margin-left:-12px;width:255px;" onKeyPress="return validator.isHWAddr(this,event)" onClick="hideClients_Block();" autocorrect="off" autocapitalize="off" placeholder="ex: <% nvram_get("lan_hwaddr"); %>">
							<img id="pull_arrow" height="14px;" src="images/arrow-down.gif" style="position:absolute;*margin-left:-3px;*margin-top:1px;" onclick="pullLANIPList(this);" title="<#select_MAC#>">
							<div id="ClientList_Block_PC" class="clientlist_dropdown" style="margin-left:50px;"></div>	
						</td>
	        			<td width="30%">
	        				<input type="text" class="input_15_table" maxlength="15" name="dhcp_staticip_x_0" onkeypress="return validator.isIPAddr(this,event)" autocorrect="off" autocapitalize="off">
	        			</td>
	        			<td width="10%">
							<div> 
								<input id="addStatic_Btn" type="button" class="add_btn" onClick="addRow_Group(64);" value="">
							</div>
	        			</td>
				  	</tr>
				</table>
				<div id="dhcp_staticlist_Block"></div>
			</td>
		</tr>
	</table>
	<div style="margin-top:5px;padding-bottom:10px;width:100%;text-align:center;">
		<input class="button_gen" type="button" onclick="show_subnet_edit(0);" value="<#CTL_Cancel#>">
		<input id="saveButton" class="button_gen" type="button" onclick="" value="<#CTL_ok#>">
	</div>
</div>

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
							<div class="formfonttitle"><#menu5_2#> - <#Subnet#></div>
							<div style="margin:10px 0 10px 5px;" class="splitLine"></div>
							<div class="formfontdesc"><#MSubnet_desc#></div>
							<div style="margin-left:8px; margin-bottom:7px;"> <div style="font-size:12px; font-weight:800;display:table-cell;vertical-align:bottom;"><#MSubnet_ConfigTable_Title#>&nbsp;(<#List_limit#>&nbsp;8)</div><div style="display:table-cell;padding-left:6px;"><input id="add_subnet_btn" type="button" class="add_btn" onClick="add_subnet();" value=""></div></div>
							<table width="98%" border="1" align="center" cellpadding="4" cellspacing="0" class="FormTable_table" id="subnet_table" style="margin-top:5px;">
				    			<tr>
									<th style="width:19%"><#RouterConfig_GWStaticGW_itemname#></th>
				      				<th style="width:19%"><#IPConnection_x_ExternalSubnetMask_itemname#></th>
				      				<th style="width:19%"><#LANHostConfig_MinAddress_itemname#></th>
				      				<th style="width:19%"><#LANHostConfig_MaxAddress_itemname#></th>
				      				<th style="width:10%"><#LANHostConfig_LeaseTime_itemname#></th>
				      				<th style="width:7%"><#pvccfg_edit#></th>
				      				<th><#CTL_del#></th>
				    			</tr>
							</table>
					
							<div id="subnet_list_Block"></div>
							<div class="apply_gen">
								<input class="button_gen" onclick="applyRule()" type="button" value="<#CTL_apply#>"/>
							</div>
						</td>
					</tr>
				</tbody>
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
