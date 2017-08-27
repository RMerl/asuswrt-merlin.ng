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
<title><#Web_Title#> - <#menu5_4_4#></title>
<link rel="stylesheet" type="text/css" href="index_style.css"> 
<link rel="stylesheet" type="text/css" href="form_style.css">
<link rel="stylesheet" type="text/css" href="other.css">
<style>
.contentM_qis{
	position:absolute;
	-webkit-border-radius: 5px;
	-moz-border-radius: 5px;
	border-radius: 5px;
	z-index:200;
	background-color:#2B373B;
	display:none;
	margin-left: 30%;
	margin-top: 10px;
	width:650px;
}
#ClientList_Block_PC{
	border:1px outset #999;
	background-color:#576D73;
	position:absolute;
	*margin-top:26px;	
	margin-left:2px;
	*margin-left:-189px;
	width:181px;
	text-align:left;	
	height:auto;
	overflow-y:auto;
	z-index:200;
	padding: 1px;
	display:none;
}
#ClientList_Block_PC div{
	background-color:#576D73;
	height:auto;
	*height:20px;
	line-height:20px;
	text-decoration:none;
	font-family: Lucida Console;
	padding-left:2px;
}

#ClientList_Block_PC a{
	background-color:#EFEFEF;
	color:#FFF;
	font-size:12px;
	font-family:Arial, Helvetica, sans-serif;
	text-decoration:none;	
}
#ClientList_Block_PC div:hover, #ClientList_Block a:hover{
	background-color:#3366FF;
	color:#FFFFFF;
	cursor:default;
}
.contentM_qis{
	position:absolute;
	-webkit-border-radius: 5px;
	-moz-border-radius: 5px;
	border-radius: 5px;
	z-index:200;
	background-color:#2B373B;
	display:none;
	margin-left: 30%;
	margin-top: 10px;
	width:650px;
}	
</style>	
<script type="text/javascript" src="state.js"></script>
<script type="text/javascript" src="general.js"></script>
<script type="text/javascript" src="popup.js"></script>
<script type="text/javascript" src="help.js"></script>
<script type="text/javascript" src="validator.js"></script>
<script type="text/javascript" src="wcdma_list.js"></script>
<script type="text/javaScript" src="js/jquery.js"></script>
<script type="text/javascript" src="switcherplugin/jquery.iphone-switch.js"></script>
<script>

<% login_state_hook(); %>
var wireless = [<% wl_auth_list(); %>];	// [[MAC, associated, authorized], ...]
var orig_modem_enable = '<% nvram_get("modem_enable"); %>';
var country = '<% nvram_get("modem_country"); %>';
var isp = '<% nvram_get("modem_isp"); %>';
var apn = '<% nvram_get("modem_apn"); %>';
var dialnum = '<% nvram_get("modem_dialnum"); %>';
var user = '<% nvram_get("modem_user"); %>';
var pass = '<% nvram_get("modem_pass"); %>';
var modem_limit_unit = '<% nvram_get("modem_limit_unit"); %>';
var modem_warning_unit = '<% nvram_get("modem_warning_unit"); %>';

var stopCheck = 0;
//var modem_act_hwver = '<% nvram_get("usb_modem_act_hwver"); %>';
var modem_act_imei = '<% nvram_get("usb_modem_act_imei"); %>';
var modem_act_imsi = '<% nvram_get("usb_modem_act_imsi"); %>';
var modem_act_iccid = '<% nvram_get("usb_modem_act_iccid"); %>';
var modem_operation ='<% nvram_get("usb_modem_act_operation"); %>';
var modem_act_rssi = '<% nvram_get("usb_modem_act_rssi"); %>';
var modem_act_sinr = '<% nvram_get("usb_modem_act_sinr"); %>';
var modem_act_rsrp = '<% nvram_get("usb_modem_act_rsrp"); %>';
var modem_act_rsrq = '<% nvram_get("usb_modem_act_rsrq"); %>';
var modem_act_cellid = '<% nvram_get("usb_modem_act_cellid"); %>';
var modem_act_lac = '<% nvram_get("usb_modem_act_lac"); %>';
var orig_modem_lte_band = '<% nvram_get("modem_lte_band"); %>';
var modem_isp = '<% nvram_get("modem_isp"); %>';
var modem_spn = '<% nvram_get("usb_modem_auto_spn"); %>';
var modem_act_provider = '<% nvram_get("usb_modem_act_provider"); %>';
var g3err_pin = '<% nvram_get("g3err_pin"); %>';
var pin_remaining_count = '<% nvram_get("usb_modem_act_auth_pin"); %>';
var puk_remaining_count = '<% nvram_get("usb_modem_act_auth_puk"); %>';
var tx_rate = parseFloat('<% nvram_get("usb_modem_act_tx"); %>');
var rx_rate = parseFloat('<% nvram_get("usb_modem_act_rx"); %>');
var total_bytes = 0;
var simact_result = "";
var simact_flag = 0;
var modemuptime = parseInt('<% get_modemuptime(); %>');
var modem_autoapn_imsi = '<% nvram_get("modem_autoapn_imsi"); %>';

var modemlist = new Array();
var countrylist = new Array();
var protolist = new Array();
var isplist = new Array();
var apnlist = new Array();
var daillist = new Array();
var userlist = new Array();
var passlist = new Array();

var KBytes = 1024;
var MBytes = 1024*1024;
var GBytes = 1024*1024*1024;
var orig_usage_date = '<% nvram_get("modem_bytes_data_cycle"); %>';

var mobile_state = -1;
var mobile_sbstate = -1;
var mobile_auxstate = -1;
var usb_modem_act_auth = '<% nvram_get("usb_modem_act_auth"); %>';
var modem_pincode = '<% nvram_get("modem_pincode"); %>';

/* start of DualWAN */ 
var wans_dualwan = '<% nvram_get("wans_dualwan"); %>';
var old_sim_state = '<% nvram_get("usb_modem_act_sim"); %>';
<% wan_get_parameter(); %>
var wans_hotstandby = '<% nvram_get("wans_standby"); %>';

/* Data usage*/
var old_sim_order = '';

var orig_simdetect = '<% nvram_get("usb_modem_act_simdetect"); %>';


if(dualWAN_support && wans_dualwan.search("usb") >= 0 ){
	var wan_type_name = wans_dualwan.split(" ")[<% nvram_get("wan_unit"); %>];
	wan_type_name = wan_type_name.toUpperCase();
	switch(wan_type_name){
		case "DSL":
			location.href = "Advanced_DSL_Content.asp";
			break;
		case "WAN":
			location.href = "Advanced_WAN_Content.asp";
			break;
		case "LAN":
			location.href = "Advanced_WAN_Content.asp";
			break;	
		default:
			break;	
	}
}

var confirmState;
var show_wstatus = 0;

function genWANSoption(){
	for(i=0; i<wans_dualwan.split(" ").length; i++){
	var wans_dualwan_NAME = wans_dualwan.split(" ")[i].toUpperCase();
		//MODELDEP: DSL-N55U, DSL-N55U-B, DSL-AC68U, DSL-AC68R
		if(wans_dualwan_NAME == "LAN" && 
			(productid == "DSL-N55U" || productid == "DSL-N55U-B" || productid == "DSL-AC68U" || productid == "DSL-AC68R"))	
			wans_dualwan_NAME = "Ethernet WAN";
		else if(wans_dualwan_NAME == "LAN")
			wans_dualwan_NAME = "Ethernet LAN";
		if(wans_dualwan_NAME == "USB" && (based_modelid == "4G-AC55U" || based_modelid == "4G-AC68U"))
			wans_dualwan_NAME = "<#Mobile_title#>";
		document.form.wan_unit.options[i] = new Option(wans_dualwan_NAME, i);
	}
	document.form.wan_unit.selectedIndex = '<% nvram_get("wan_unit"); %>';
}
/* end of DualWAN */

var usb_modem_act_swver = '<% nvram_get("usb_modem_act_swver"); %>';
var usb_lte_version = '<% chk_lte_fw(); %>';
var lte_update_status = '';

function initial(){
	var data_usage = 0;
	var remaining_data = 0;
	var limit_val = parseFloat(document.form.modem_bytes_data_limit.value);
	var warning_val = parseFloat(document.form.modem_bytes_data_warning.value);

	show_menu();

	if( modem_limit_unit == '0' ) //GByes
		document.getElementById("data_limit").value = Math.round(limit_val/GBytes*1000)/1000;
	else if( modem_limit_unit == '1' ) //MByes
		document.getElementById("data_limit").value = Math.round(limit_val/MBytes*1000)/1000;		

	if( modem_warning_unit == '0' ) //GByes
		document.getElementById("data_warning").value = Math.round(warning_val/GBytes*1000)/1000;		
	else if( modem_warning_unit == '1' ) //MByes
		document.getElementById("data_warning").value = Math.round(warning_val/MBytes*1000)/1000;	

	data_usage = tx_bytes + rx_bytes;

	if(!isNaN(data_usage)){
		if(data_usage < KBytes)
			document.getElementById("data_usage").innerHTML = data_usage + "&nbsp;Bytes";
		else if(data_usage < MBytes)
			document.getElementById("data_usage").innerHTML = Math.round(data_usage/KBytes*1000)/1000 + "&nbsp;KBytes";
		else if(data_usage < GBytes)
			document.getElementById("data_usage").innerHTML = Math.round(data_usage/MBytes*1000)/1000 + "&nbsp;MBytes";
		else
			document.getElementById("data_usage").innerHTML = Math.round(data_usage/GBytes*1000)/1000 + "&nbsp;GBytes";

		document.getElementById("starting_date").innerHTML = "(<#Mobile_starting_day#> : " + orig_usage_date + ")";
	}

	if(data_usage > 0)
		document.getElementById("reset_usage_btn").style.display = "";

	if( limit_val > 0 ){
		remaining_data = limit_val - data_usage;
		if(remaining_data < 0)
			remaining_data = 0;

		if(!isNaN(remaining_data)){
			if(Math.abs(remaining_data) < KBytes)
				document.getElementById("remaining_data").innerHTML = remaining_data + "&nbsp;Bytes";
			else if(Math.abs(remaining_data) < MBytes)
				document.getElementById("remaining_data").innerHTML = Math.round(remaining_data/KBytes*1000)/1000 + "&nbsp;KBytes";
			else if(Math.abs(remaining_data) < GBytes)
				document.getElementById("remaining_data").innerHTML = Math.round(remaining_data/MBytes*1000)/1000 + "&nbsp;MBytes";
			else
				document.getElementById("remaining_data").innerHTML = Math.round(remaining_data/GBytes*1000)/1000 + "&nbsp;GBytes";
		}
	}
	else
		document.getElementById("remaining_data_tr").style.display = "none";

	if(dualWAN_support && '<% nvram_get("wans_dualwan"); %>'.search("none") < 0){
		genWANSoption();
	}
	else{
		document.form.wan_unit.disabled = true;
		document.getElementById("WANscap").style.display = "none";	
	}

	switch_modem_mode('<% nvram_get("modem_enable"); %>');
	gen_country_list();
	reloadProfile();

	if(!dualWAN_support){
		document.getElementById("_APP_Installation").innerHTML = '<table><tbody><tr><td><div class="AiProtection_HomeSecurity"></div></td><td><div style="width:120px;"><#Menu_usb_application#></div></td></tr></tbody></table>';
		document.getElementById("_APP_Installation").className = "menu_clicked";
	}

	if(sim_state == "1"){
		if(usb_modem_act_auth == "1" || usb_modem_act_auth == "2"){
			document.form.pin_verify[0].selected = true;
			document.getElementById("pin_modify_tr").style.display = "";
		}
		else if(usb_modem_act_auth == "3"){
			document.form.pin_verify[1].selected = true;
			document.getElementById("pin_modify_tr").style.display = "none";		
		}
		else if(usb_modem_act_auth == ""){
			getSimAuth();
		}
	}

	if(modem_pincode != "")
		document.form.pincode.value = modem_pincode;

	if(modem_pincode != "")
		document.form.save_pin_ckb.checked = true;
	else
		document.form.save_pin_ckb.checked = false;

	document.getElementById("pin_remaining").innerHTML = '<#Mobile_remaining_num#>: ';
	document.getElementById("pin_remaining").innerHTML += pin_remaining_count;

	change_apn_mode();
	show_dateList();
	show_phone(document.form.modem_sms_limit.value);
	check_sim_state();
	check_connect_status();

	if(usb_lte_version != ""){
		document.getElementById("new_version_tr").style.display = "";
		document.getElementById("new_version").innerHTML = usb_lte_version;
		document.getElementById("lte_update_note").style.display = "";
	}

}

function show_sim_settings(show_flag){
	if(show_flag == 1){
		document.getElementById("connection_table").style.display = '';
		document.getElementById("traffic_table").style.display = '';
		document.getElementById("apn_table").style.display = '';
		change_apn_mode();
		inputCtrl(document.form.modem_mode, 1);
		inputCtrl(document.form.modem_pdp, 1);
		document.form.modem_bytes_data_limit.disabled = false;
		document.form.modem_bytes_data_warning.disabled = false;
		document.form.modem_limit_unit.disabled = false;
		document.form.modem_warning_unit.disabled = false;
		document.getElementById("modem_roaming_tr").style.display = "";
		inputCtrl(document.form.modem_roaming, 1);
		ShowRoamingOpt(document.form.modem_roaming.value);
		document.form.modem_roaming_isp.disabled = false;
	}
	else{
		document.getElementById("connection_table").style.display = 'none';
		document.getElementById("traffic_table").style.display = 'none';
		document.getElementById("apn_table").style.display = 'none';
		inputCtrl(document.form.modem_mode, 0);
		inputCtrl(document.form.modem_pdp, 0);
		document.form.modem_bytes_data_limit.disabled = true;
		document.form.modem_bytes_data_warning.disabled = true;
		document.form.modem_limit_unit.disabled = true;
		document.form.modem_warning_unit.disabled = true;		
		inputCtrl(document.form.modem_roaming, 0);
		inputCtrl(document.form.modem_roaming_isp, 0);
	}

}

function reloadProfile(){
	if(document.form.modem_enable.value == 0)
		return 0;
	gen_list();
	show_ISP_list();
	show_APN_list();
}

function switch_modem_mode(mode){
	document.form.modem_enable.value = mode;
	if(mode == '0'){
		document.getElementById("connection_table").style.display = 'none';
		document.getElementById("traffic_table").style.display = 'none';
		document.getElementById("apn_table").style.display = 'none';
		document.getElementById("sim_mgnt_table").style.display = 'none';
		inputCtrl(document.form.modem_pdp, 0);
		document.form.modem_bytes_data_limit.disabled = true;
		document.form.modem_bytes_data_warning.disabled = true;
		document.form.modem_limit_unit.disabled = true;
		document.form.modem_warning_unit.disabled = true;
		inputCtrl(document.form.modem_roaming, 0);
		inputCtrl(document.form.modem_roaming_isp, 0);
		//inputCtrl(document.form.modem_enable_option, 0);
		inputCtrl(document.form.modem_country, 0);
		inputCtrl(document.form.modem_isp, 0);
		inputCtrl(document.form.modem_apn, 0);
		inputCtrl(document.form.modem_dialnum, 0);
		inputCtrl(document.form.modem_user, 0);
		inputCtrl(document.form.modem_pass, 0);
	}
	else{
		if(sim_state == "1"){
			show_sim_settings(1);
			if(modem_autoapn_imsi != "")
				inputCtrl(document.form.modem_isp, 0);		
		}
		else{
			show_sim_settings(0);
		}

		document.getElementById("sim_mgnt_table").style.display = '';
	}
}

function modem_enable_act(enable){
	var confirm_str_off = "<#Mobile_disable_hint#>";

	if(enable == "1")
		switch_modem_mode(document.form.modem_enable_option.value);
	else{
		if(wans_hotstandby == "1" && confirm(confirm_str_off)){
			document.form.wans_standby.value = "0";
		}	
	}

	reloadProfile();
}

function show_ISP_list(){
	var removeItem = 0;
	free_options(document.form.modem_isp);
	document.form.modem_isp.options.length = isplist.length;

	for(var i = 0; i < isplist.length; i++){
	  if(protolist[i] == 4 && !wimax_support){
			document.form.modem_isp.options.length = document.form.modem_isp.options.length - 1;

			if(document.form.modem_isp.options.length > 0)
				continue;
			else{
				alert('<#HSDPAConfig_Country_hint#>');
				document.form.modem_country.focus();
				document.form.modem_country.selectedIndex = countrylist.length-1;
				break;
			}
		}
		else
			document.form.modem_isp.options[i] = new Option(isplist[i], isplist[i]);

		if(isplist[i] == isp)
			document.form.modem_isp.options[i].selected = 1;
	}
}

function show_APN_list(){
	var ISPlist = document.form.modem_isp.value;
	var Countrylist = document.form.modem_country.value;

	var isp_order = -1;
	for(isp_order = 0; isp_order < isplist.length; ++isp_order){
		if(isplist[isp_order] == ISPlist)
			break;
		else if(isp_order == isplist.length-1){
			isp_order = -1;
			break;
		}
	}

	if(isp_order == -1){
		alert("system error");
		return;
	}
	
	/* use manual or location */
	if(document.form.modem_country.value == ""){
		inputHideCtrl(document.form.modem_isp, 0);
	}
	else{
		inputHideCtrl(document.form.modem_isp, 1);
		if(protolist[isp_order] == "")
			protolist[isp_order] = 1;
	}

	if(Countrylist == ""){
		if(orig_modem_enable == document.getElementById('modem_enable_option').value){
			document.getElementById("modem_apn").value = apn;
			document.getElementById("modem_dialnum").value = dialnum;
			document.getElementById("modem_user").value = user;
			document.getElementById("modem_pass").value = pass;
		}
		else{
			document.getElementById("modem_apn").value = apnlist[isp_order];
			document.getElementById("modem_dialnum").value = daillist[isp_order];
			document.getElementById("modem_user").value = userlist[isp_order];
			document.getElementById("modem_pass").value = passlist[isp_order];
		}
	}
	else if(protolist[isp_order] != "4"){
		if(ISPlist == isp && Countrylist == country && (apn != "" || dialnum != "" || user != "" || pass != "")){
			if(typeof(apnlist[isp_order]) == 'object' && apnlist[isp_order].constructor == Array){
				document.getElementById("pull_arrow").style.display = '';
				showLANIPList(isp_order);
			}
			else{
				document.getElementById("pull_arrow").style.display = 'none';
				document.getElementById('ClientList_Block_PC').style.display = 'none';
			}

			document.getElementById("modem_apn").value = apn;
			document.getElementById("modem_dialnum").value = dialnum;
			document.getElementById("modem_user").value = user;
			document.getElementById("modem_pass").value = pass;
		}
		else{
			if(typeof(apnlist[isp_order]) == 'object' && apnlist[isp_order].constructor == Array){
				document.getElementById("pull_arrow").style.display = '';
				showLANIPList(isp_order);
			}
			else{
				document.getElementById("pull_arrow").style.display = 'none';
				document.getElementById('ClientList_Block_PC').style.display = 'none';
				document.getElementById("modem_apn").value = apnlist[isp_order];
			}

			document.getElementById("modem_dialnum").value = daillist[isp_order];
			document.getElementById("modem_user").value = userlist[isp_order];
			document.getElementById("modem_pass").value = passlist[isp_order];
		}
	}
	else{
		document.getElementById("modem_apn").value = "";
		document.getElementById("modem_dialnum").value = "";

		if(ISPlist == isp	&& (user != "" || pass != "")){
			document.getElementById("modem_user").value = user;
			document.getElementById("modem_pass").value = pass;
		}
		else{
			document.getElementById("modem_user").value = userlist[isp_order];
			document.getElementById("modem_pass").value = passlist[isp_order];
		}
	}

	if(document.form.modem_country.value != ""){
		document.form.modem_enable.value = protolist[isp_order];
		switch_modem_mode(document.form.modem_enable.value);
	}
}

function applyRule(){
	if(document.form.modem_limit_unit.value == '0')//GBytes
		document.form.modem_bytes_data_limit.value = document.form.data_limit.value*GBytes;
	else if(document.form.modem_limit_unit.value == '1')
		document.form.modem_bytes_data_limit.value = document.form.data_limit.value*MBytes;

	if(document.form.modem_warning_unit.value == '0')//GBytes
		document.form.modem_bytes_data_warning.value = document.form.data_warning.value*GBytes;
	else if(document.form.modem_warning_unit.value == '1')
		document.form.modem_bytes_data_warning.value = document.form.data_warning.value*MBytes;

	if(document.form.modem_bytes_data_limit.value != modem_bytes_data_limit){
		cookie.unset(keystr);
	}

	if(document.form.modem_country.value == ""){
		var valueStr="";
		document.form.modem_isp.disabled = false;
		document.form.modem_isp.options.length = 1;
		document.form.modem_isp.options[0] = new Option(valueStr, valueStr, false, true);
	}

	if(orig_modem_lte_band != document.form.modem_lte_band.value)
		document.form.action_wait.value = "30";

	if(document.form.modem_roaming.value == "1"){
		if(document.form.modem_roaming_isp.value == "")
			document.form.modem_roaming_mode.value = "0";
		else
			document.form.modem_roaming_mode.value = "1";
	}
	else{
		document.form.modem_roaming_mode.value = "0";
		document.form.modem_roaming_isp.value = "";
	}

	showLoading();
	document.form.submit();
}

/*------------ Mouse event of fake LAN IP select menu {-----------------*/
function setClientIP(apnAddr){
	document.form.modem_apn.value = apnAddr;
	hideClients_Block();
	over_var = 0;
}

function showLANIPList(isp_order){
	var code = "";
	var show_name = "";

	for(var i = 0; i < apnlist[isp_order].length; i++){
		var apnlist_col = apnlist[isp_order][i].split('&&');
		code += '<a><div onmouseover="over_var=1;" onmouseout="over_var=0;" onclick="setClientIP(\''+apnlist_col[1]+'\');"><strong>'+apnlist_col[0]+'</strong></div></a>';

		if(i == 0)
			document.form.modem_apn.value = apnlist_col[1];
	}
	code +='<!--[if lte IE 6.5]><iframe class="hackiframe2"></iframe><![endif]-->';	
	document.getElementById("ClientList_Block_PC").innerHTML = code;
}

function pullLANIPList(obj){
	
	if(isMenuopen == 0){		
		obj.src = "/images/arrow-top.gif"
		document.getElementById("ClientList_Block_PC").style.display = 'block';		
		document.form.modem_apn.focus();		
		isMenuopen = 1;
	}
	else
		hideClients_Block();
}

var over_var = 0;
var isMenuopen = 0;
function hideClients_Block(){
	document.getElementById("pull_arrow").src = "/images/arrow-down.gif";
	document.getElementById('ClientList_Block_PC').style.display='none';
	isMenuopen = 0;
}
/*----------} Mouse event of fake LAN IP select menu-----------------*/

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

	if(obj.options[obj.selectedIndex].text == "WAN" ||	obj.options[obj.selectedIndex].text == "Ethernet LAN"){
		document.form.current_page.value = "Advanced_WAN_Content.asp";
	}else	if(obj.options[obj.selectedIndex].text == "USB") {
		return false;
	}

	FormActions("apply.cgi", "change_wan_unit", "", "");
	document.form.target = "";
	document.form.submit();
}

function done_validating(action){
	refreshpage();
}

function check_connect_status(){
	 $.ajax({
    	url: '/ajax_simconnect.asp',
    	dataType: 'script', 

    	error: function(xhr){
      		check_connect_status();
    	},
    	success: function(response){
    		if( usb_index == 0 ){
				mobile_state = first_wanstate;
				mobile_sbstate = first_wansbstate;
				mobile_auxstate = first_wanauxstate;
			}
			else if(usb_index == 1){
				mobile_state = second_wanstate;
				mobile_sbstate = second_wansbstate;
				mobile_auxstate = second_wanauxstate;
			}

			if(mobile_state == 2 && mobile_sbstate == 0 && mobile_auxstate == 0){
				document.getElementById("connection_status").innerHTML = "<#Connected#>";
				document.getElementById("mconnect_status").innerHTML = "<#Connected#>";
			}
			else{
				var sim_status = parseInt(sim_state);
				if(sim_status == 2){
					document.getElementById("connection_status").innerHTML = "<#Mobile_need_pin#>";
					document.getElementById("mconnect_status").innerHTML = " <#Mobile_need_pin#>";
				}
				else if(sim_status == 3){
					document.getElementById("connection_status").innerHTML = "<#Mobile_sim_lock#> <#Mobile_need_puk#>";
					document.getElementById("mconnect_status").innerHTML = "<#Mobile_sim_lock#> <#Mobile_need_puk#>";
				}
				else if(sim_status == 4){
					document.getElementById("connection_status").innerHTML = "<#Mobile_need_pin2#>";
					document.getElementById("mconnect_status").innerHTML = "<#Mobile_need_pin2#>";
				}
				else if(sim_status == 5){
					document.getElementById("connection_status").innerHTML = "<#Mobile_sim_lock#> <#Mobile_need_puk2#>";
					document.getElementById("mconnect_status").innerHTML = "<#Mobile_sim_lock#> <#Mobile_need_puk2#>";
				}
				else if(sim_status == 6){
					document.getElementById("connection_status").innerHTML = "<#Mobile_wait_sim#>";
					document.getElementById("mconnect_status").innerHTML = "<#Mobile_wait_sim#>";
				}
				else if(sim_status == -1){
					document.getElementById("connection_status").innerHTML = "<#Mobile_sim_miss#>";
					document.getElementById("mconnect_status").innerHTML = "<#Mobile_sim_miss#>";
				}
				else if(scan_end == "1" || scan_end == "2" || scan_end == "3"){
					document.getElementById("connection_status").innerHTML = "Scanning";/*untranslated*/
					document.getElementById("mconnect_status").innerHTML = "Scanning";
				}
				else{
					document.getElementById("connection_status").innerHTML = "<#Connecting_str#>";
					document.getElementById("mconnect_status").innerHTML = "<#Connecting_str#>";
				}
			}

			if( sim_state == '1')
				setTimeout("check_connect_status();",3000);
       }
   });
}

function showUpDownRate(){
	var Kbits = 1024;
	var Mbits = 1024*1024;
	var Gbits = 1024*1024*1024;

	if(!isNaN(tx_rate)){
		if(tx_rate < Kbits)
			document.getElementById("upRate").innerHTML = tx_rate + "&nbsp;bps";
		else if(tx_rate < Mbits)
			document.getElementById("upRate").innerHTML = Math.round(tx_rate/Kbits*1000)/1000 + "&nbsp;Kbps";
		else if(tx_rate < Gbits)
			document.getElementById("upRate").innerHTML = Math.round(tx_rate/Mbits*1000)/1000 + "&nbsp;Mbps";
		else
			document.getElementById("upRate").innerHTML = Math.round(tx_rate/Gbits*1000)/1000 + "&nbsp;Gbps";
	}

	if(!isNaN(rx_rate)){
		if(rx_rate < Kbits)
			document.getElementById("downRate").innerHTML = rx_rate + "&nbsp;bps";
		else if(rx_rate < Mbits)
			document.getElementById("downRate").innerHTML = Math.round(rx_rate/Kbits*1000)/1000 + "&nbsp;Kbps";
		else if(rx_rate < Gbits)
			document.getElementById("downRate").innerHTML = Math.round(rx_rate/Mbits*1000)/1000 + "&nbsp;Mbps";
		else
			document.getElementById("downRate").innerHTML = Math.round(rx_rate/Gbits*1000)/1000 + "&nbsp;Gbps";
	}
}

function check_simact_result(flag){ // 1: Unblock PIN  2: configure PIN  3: modify PIN
	var result_no = "";

	$.ajax({
		url: '/simact_result.asp',
		dataType: 'script',
		error: function(xhr){
			check_simact_result();		
		},
		success: function(response){
			if(simact_result.indexOf("done") >= 0){
				if(flag == 1)
					setTimeout("check_sim_state(1);", 2000);
				else if(flag == 2)
					setTimeout("check_sim_state(2);", 2000);
				else if(flag == 3){
					show_sim_table(0);
					document.getElementById("pin_modify_result").innerHTML = "<#Mobile_succeed_changePIN#>";
					document.getElementById("pin_modify_result").style.display="";
				}
				else if(flag ==4)
					location.reload();
				simact_flag = 0;
			}
			else{
				result_no = simact_result.substring(0,2);
				if(flag == 1 || flag == 2)
					check_sim_state(flag);
				else if(flag == 3){//Modify PIN
					show_sim_table(0);				
					document.getElementById("pin_modify_result").innerHTML = simact_result_str(result_no);
					document.getElementById("pin_modify_result").style.display="";
				}
				else if(flag == 4){//PIN Verification
					if(usb_modem_act_auth == "1" || usb_modem_act_auth == "2"){
						document.form.pin_verify[0].selected = true;
						document.getElementById("pin_modify_tr").style.display = "";
					}
					else if(usb_modem_act_auth == "3"){
						document.form.pin_verify[1].selected = true;
						document.getElementById("pin_modify_tr").style.display = "none";	
					}
					document.getElementById("pin_verify_result").innerHTML = simact_result_str(result_no);
					document.getElementById("pin_verify_result").style.display="";
				}
			}
		}
	});	
}

function simact_result_str(result_no){
	var result_string = "";
	switch(result_no){
		case "25":
			result_string = "Fail to unlock the SIM.";
			break;
		case "28":
			result_string = "Fail to unlock the SIM PIN.";		
			break;
		case "30":
			result_string = "<#Mobile_sim_lock#>";
			break;																	
		case "34":
			result_string = "Fail to enable PIN Verification.";
			break;
		case "35":
			result_string = "Fail to disable PIN Verification.";
			break;
		case "38":
			result_string = "<#Mobile_fail_changePIN#>";
			break;
	}

	return result_string;
}

function check_sim_state(flag){
	$.ajax({
    	url: '/ajax_simstate.asp',
    	dataType: 'script', 

    	error: function(xhr){
      		setTimeout("check_sim_state();", 1000);
    	},
    	success: function(response){
			switch(sim_state){
				case '1':	
					if(flag == 1 && $("#sim_input").css("display") == "block")
						show_sim_table(0);
					document.getElementById("usim_status").innerHTML = "<#Mobile_sim_ready#>";
					break;
				case '2':
					if(g3err_pin == '1' && pin_remaining_count < 3){
						document.getElementById("usim_status").innerHTML = "<#Mobile_wrong_pin#>";	
						if( pin_remaining_count == 0)
							check_sim_state(2);									
					}
					else if(!simact_flag){
						document.getElementById("usim_status").innerHTML = "<#Mobile_need_pin#>";
					}
					document.getElementById("pin_remaining").innerHTML = '<#Mobile_remaining_num#>: ';
					document.getElementById("pin_remaining").innerHTML += pin_remaining_count;							
					break;	
				case '3':
					document.getElementById("usim_status").innerHTML = "<#Mobile_need_puk#>";	
					if(flag == 1 && $("#sim_input").css("display") == "block")
						document.getElementById("puk_remaining").innerHTML = puk_remaining_count;		
					break;
				case '4':
					document.getElementById("usim_status").innerHTML = "<#Mobile_need_pin2#>";
					break;
				case '5':
					document.getElementById("usim_status").innerHTML = "<#Mobile_need_puk2#>";					
					break;
				case '6':
					document.getElementById("usim_status").innerHTML = "<#Mobile_wait_sim#>";				
					break;					
				case '-1':
					document.getElementById("usim_status").innerHTML = "<#Mobile_sim_miss#>";
					break;	
				case '-2':
				case '-10':
					document.getElementById("usim_status").innerHTML = "<#Mobile_sim_fail#>";
					break;
				default:
					break;	
			}

			if(document.form.pin_verify[0].selected != true && document.form.pin_verify[1].selected != true){
				if(usb_modem_act_auth == "1" || usb_modem_act_auth == "2")
					document.form.pin_verify[0].selected = true;
				else if(usb_modem_act_auth == "3")
					document.form.pin_verify[1].selected = true;
			}

			if(sim_state == '1'){
				document.getElementById("pin_verify_tr").style.display = "";
				document.getElementById("unblock_btn").style.display = "none";
				if(usb_modem_act_auth == "1" || usb_modem_act_auth == "2")
					document.getElementById("pin_modify_tr").style.display = "";
				document.form.pincode.value="";
				document.getElementById("pin_code_tr").style.display="none";		
			}
			else if(sim_state == '3' || sim_state == '5'){
				document.getElementById("pin_verify_tr").style.display = "none";
				document.getElementById("pin_modify_tr").style.display = "none";
				document.form.pincode.value="";
				document.getElementById("pin_code_tr").style.display="none";
				document.getElementById("unblock_btn").style.display = "";				
			}
			else{
				if(sim_state == '2'){
					document.getElementById("pin_code_tr").style.display="";
				}
				document.getElementById("pin_verify_tr").style.display = "none";
				document.getElementById("pin_modify_tr").style.display = "none";
				document.getElementById("unblock_btn").style.display = "none";					
			}			

			if(flag == 1){
				if($("#sim_input").css("display") == "block"){
					document.getElementById("loadingIcon_sim").style.display="none";
					document.getElementById("sim_ok_button").style.display = "";
					document.getElementById("sim_cancel_btn").style.display = "";	
				}
			}	
			else if(flag == 2){			
				document.getElementById("loadingIcon_pin").style.display = "none";
				document.getElementById("save_pin_btn").style.display = "";
				document.getElementById("save_pin_ckb_span").style.display="";	
			}
			
			if(!simact_flag)
				setTimeout("check_sim_state();", 2000);

			if(old_sim_state != sim_state){
				if(sim_state == '1'){
					if(document.form.modem_enable.value != '0')
						check_connect_status();
						show_sim_settings(1);
				}
				else{
					show_sim_settings(0);
				}
			}

			old_sim_state = sim_state;

			if((modem_sim_order != old_sim_order) && (modem_sim_order == "-1")){
				var confirm_str = "<#Mobile_record_limit_warning#>";
				if(confirm(confirm_str)){
					document.simact_form.action_mode.value = "restart_sim_del";
					document.simact_form.sim_order.value = "1";
					document.simact_form.submit();							
				}
			}

			old_sim_order = modem_sim_order;
        }
   });
}	

function check_sim_details(){
	if( stopCheck == 1 )
		return;

	$.ajax({
    	url: '/ajax_simstatus.asp',
    	dataType: 'script', 

    	error: function(xhr){
      		setTimeout("check_sim_details();", 1000);
    	},
    	success: function(response){

			//document.getElementById("modem_act_hwver").innerHTML = modem_act_hwver;	
			document.getElementById("modem_act_imei").innerHTML = modem_act_imei;
			document.getElementById("modem_act_imsi").innerHTML = modem_act_imsi;
			document.getElementById("modem_act_iccid").innerHTML = modem_act_iccid;

			if(modem_act_cellid.length > 0 && !isNaN(modem_act_cellid) && modem_act_cellid != "0"){
				document.getElementById("cellid_tr").style.display = "";
				document.getElementById("modem_cellid").innerHTML = modem_act_cellid;
				show_wstatus = 1;
			}

			if(modem_act_rssi.length > 0 && !isNaN(modem_act_rssi) && modem_act_rssi != "0"){
				document.getElementById("rssi_tr").style.display = "";
				document.getElementById("modem_rssi").innerHTML = modem_act_rssi;
				show_wstatus = 1;
			}

			if(modem_operation == "LTE" && modem_act_sinr.length > 0 && !isNaN(modem_act_sinr) && modem_act_sinr != "0"){
				document.getElementById("sinr_tr").style.display = "";
				document.getElementById("modem_sinr").innerHTML = modem_act_sinr;
				show_wstatus = 1;
			}

			if(modem_act_rsrp.length > 0 && !isNaN(modem_act_rsrp) && modem_act_rsrp != "0"){
				document.getElementById("rsrp_tr").style.display = "";
				document.getElementById("modem_rsrp").innerHTML = modem_act_rsrp;
				show_wstatus = 1;
			}

			if(modem_act_rsrq.length > 0 && !isNaN(modem_act_rsrq) && modem_act_rsrq != "0"){
				document.getElementById("rsrq_tr").style.display = "";
				document.getElementById("modem_rsrq").innerHTML = modem_act_rsrq;
				show_wstatus = 1;
			}

			if(modem_act_lac.length > 0 && !isNaN(modem_act_lac) && modem_act_lac != "0"){
				document.getElementById("lac_tr").style.display = "";
				document.getElementById("modem_lac").innerHTML = modem_act_lac;
				show_wstatus = 1;
			}

			if(show_wstatus)
				document.getElementById("wireless_status").style.display = "";
		
			if(sim_state == '1'){
				document.getElementById("msim_provider").innerHTML = modem_spn;
				document.getElementById("misp").innerHTML = '&nbsp;'+ modem_act_provider;
				switch(modem_operation)
				{
					case 'Edge':
						document.getElementById("msignalsys").innerHTML  = '<img src="/images/mobile/E.png">';
						break;
					case 'GPRS':
						document.getElementById("msignalsys").innerHTML = '<img src="/images/mobile/G.png">';
						break;
					case 'WCDMA':
					case 'CDMA':
					case 'EV-DO REV 0':
					case 'EV-DO REV A':
					case 'EV-DO REV B':
						document.getElementById("msignalsys").innerHTML = '<img src="/images/mobile/3G.png">';
						break;
					case 'HSDPA':
					case 'HSUPA':
						document.getElementById("msignalsys").innerHTML = '<img src="/images/mobile/H.png">';
						break;
					case 'HSDPA+':
					case 'DC-HSDPA+':
						document.getElementById("msignalsys").innerHTML = '<img src="/images/mobile/H+.png">';
						break;
					case 'LTE':
						document.getElementById("msignalsys").innerHTML = '<img src="/images/mobile/LTE.png">';
						break;
					case 'GSM':
					default:
						document.getElementById("msignalsys").innerHTML = '';
						break;
				}

				total_bytes = rx_bytes + tx_bytes;
				if(!isNaN(total_bytes)){
					if(total_bytes < KBytes)
						document.getElementById("totalTraffic").innerHTML = total_bytes + "&nbsp;Bytes";
					else if(total_bytes < MBytes)
						document.getElementById("totalTraffic").innerHTML = Math.round(total_bytes/KBytes*1000)/1000 + "&nbsp;KBytes";
					else if(total_bytes < GBytes)
						document.getElementById("totalTraffic").innerHTML = Math.round(total_bytes/MBytes*1000)/1000 + "&nbsp;MBytes";
					else
						document.getElementById("totalTraffic").innerHTML = Math.round(total_bytes/GBytes*1000)/1000 + "&nbsp;GBytes";
				}

				if(!isNaN(tx_bytes)){
					if(tx_bytes < KBytes)
						document.getElementById("upTraffic").innerHTML = tx_bytes + "&nbsp;Bytes";
					else if(tx_bytes < MBytes)
						document.getElementById("upTraffic").innerHTML = Math.round(tx_bytes/KBytes*1000)/1000 + "&nbsp;KBytes";
					else if(tx_bytes < GBytes)
						document.getElementById("upTraffic").innerHTML = Math.round(tx_bytes/MBytes*1000)/1000 + "&nbsp;MBytes";
					else
						document.getElementById("upTraffic").innerHTML = Math.round(tx_bytes/GBytes*1000)/1000 + "&nbsp;GBytes";
				}

				if(!isNaN(rx_bytes)){
					if(rx_bytes < KBytes)
						document.getElementById("downTraffic").innerHTML = rx_bytes + "&nbsp;Bytes";
					else if(rx_bytes < MBytes)
						document.getElementById("downTraffic").innerHTML = Math.round(rx_bytes/KBytes*1000)/1000 + "&nbsp;KBytes";
					else if(rx_bytes < GBytes)
						document.getElementById("downTraffic").innerHTML = Math.round(rx_bytes/MBytes*1000)/1000 + "&nbsp;MBytes";
					else
						document.getElementById("downTraffic").innerHTML = Math.round(rx_bytes/GBytes*1000)/1000 + "&nbsp;GBytes";
				}
			}
			else{
				document.getElementById("msignalsys").innerHTML = '';
				document.getElementById("msim_provider").innerHTML = '';
				document.getElementById("misp").innerHTML = '';
			}

			if(!isNaN(modemuptime)){
				document.getElementById("connect_days").innerHTML = Math.floor(modemuptime / (60*60*24));
				document.getElementById("connect_hours").innerHTML = Math.floor((modemuptime / 3600) % 24);
				document.getElementById("connect_minutes").innerHTML = Math.floor(modemuptime % 3600 / 60);
				document.getElementById("connect_seconds").innerHTML = Math.floor(modemuptime % 60);
			}

			//showUpDownRate();
			if(!isNaN(tx_rate))
				document.getElementById("upRate").innerHTML = tx_rate + "&nbsp;bps";

			if(!isNaN(rx_rate))
				document.getElementById("downRate").innerHTML = rx_rate + "&nbsp;bps";

			if(modem_autoapn_imsi != "")
				inputCtrl(document.form.modem_isp, 0);

			setTimeout("check_sim_details();", 1000);
       }
   });
}


function change_limit_value(){
	var data_usage = tx_bytes + rx_bytes;

	if(document.form.modem_limit_unit.value == '0')//GBytes
		data_limit_bytes = parseFloat(document.getElementById("data_limit").value)*GBytes;
	else if(document.form.modem_limit_unit.value == '1')
		data_limit_bytes = parseFloat(document.getElementById("data_limit").value)*MBytes;

	if(data_limit_bytes > 0 && data_limit_bytes < data_usage){
		alert("<#Mobile_limit_hint#>");
		document.form.data_limit.focus();
	}
	else
		update_usage_data();
}

function update_usage_data()
{
	var remaining_data = 0;
	var data_limit_bytes = 0;
	var data_usage = tx_bytes + rx_bytes;

	if(!isNaN(data_usage)){
		if(data_usage < KBytes)
			document.getElementById("data_usage").innerHTML = data_usage + "&nbsp;Bytes";
		else if(data_usage < MBytes)
			document.getElementById("data_usage").innerHTML = Math.round(data_usage/KBytes*1000)/1000 + "&nbsp;KBytes";
		else if(data_usage < GBytes)
			document.getElementById("data_usage").innerHTML = Math.round(data_usage/MBytes*1000)/1000 + "&nbsp;MBytes";
		else
			document.getElementById("data_usage").innerHTML = Math.round(data_usage/GBytes*1000)/1000 + "&nbsp;GBytes";
	}

	if(data_usage > 0)
		document.getElementById("reset_usage_btn").style.display = "";

	if(document.form.modem_limit_unit.value == '0')//GBytes
		data_limit_bytes = parseFloat(document.getElementById("data_limit").value)*GBytes;
	else if(document.form.modem_limit_unit.value == '1')
		data_limit_bytes = parseFloat(document.getElementById("data_limit").value)*MBytes;

	if(data_limit_bytes > 0){
		remaining_data = data_limit_bytes - data_usage;

		if(remaining_data < 0)
			remaining_data = 0;
		
		if(Math.abs(remaining_data) < KBytes)
			document.getElementById("remaining_data").innerHTML = remaining_data + "&nbsp;Bytes";
		else if(Math.abs(remaining_data) < MBytes)
			document.getElementById("remaining_data").innerHTML = Math.round(remaining_data/KBytes*1000)/1000 + "&nbsp;KBytes";
		else if(Math.abs(remaining_data) < GBytes)
			document.getElementById("remaining_data").innerHTML = Math.round(remaining_data/MBytes*1000)/1000 + "&nbsp;MBytes";
		else
			document.getElementById("remaining_data").innerHTML = Math.round(remaining_data/GBytes*1000)/1000 + "&nbsp;GBytes";

		document.getElementById("remaining_data_tr").style.display="";
	}
	else
		document.getElementById("remaining_data_tr").style.display="none";
}

function Show_status(){
	$("#mobile_status").fadeIn(300);
	stopCheck = 0;
	check_sim_details();
}

function hide_status(){
	$("#mobile_status").fadeOut(300);
	stopCheck = 1;
	update_usage_data();
	hide_wstatus();
}

function hide_wstatus(){
	show_wstatus = 0;
	document.getElementById("cellid_tr").style.display = "none";
	document.getElementById("rssi_tr").style.display = "none";
	document.getElementById("rsrp_tr").style.display = "none";
	document.getElementById("rsrq_tr").style.display = "none";
	document.getElementById("lac_tr").style.display = "none";
	document.getElementById("wireless_status").style.display = "none";
}

var scan_end = '<% nvram_get("usb_modem_act_scanning"); %>';
var ispstr = '<% get_isp_scan_results(); %>';
var ispList = "";
var orig_modem_roaming_isp = '<% nvram_get("modem_roaming_isp"); %>';
var orig_modem_mode = '<% nvram_get("modem_mode"); %>';
switch(orig_modem_mode)
{
	case '2':
		orig_modem_mode = "2G";
		break;
	case '3':
		orig_modem_mode = "3G";
		break;
	case '4':
		orig_modem_mode = "4G";
		break;
	default:
		break;
}

function ShowRoamingOpt(modem_roaming){
	var show = parseInt(modem_roaming);

	if(show){
		document.getElementById("roaming_isp").style.display = "";
		show_roaming_isp_list(ispstr);		
	}
	else{
		document.getElementById("roaming_isp").style.display = "none";
	}
}

function change_limit_unit(traffic_unit){
	var limit_val = parseFloat(document.getElementById("data_limit").value);

	if( traffic_unit == '0' ) //MBytes => GBytes
		document.getElementById("data_limit").value = Math.round(limit_val/1024*1000)/1000;
	else if( traffic_unit == '1' ) //Gbytes => MBytes
		document.getElementById("data_limit").value = Math.round(limit_val*1024);		

	update_usage_data();
}

function change_warning_unit(traffic_unit){
	var warning_val = parseFloat(document.getElementById("data_warning").value);

	if( traffic_unit == '0' ) //MBytes => GBytes
		document.getElementById("data_warning").value = Math.round(warning_val/1024*1000)/1000;
	else if( traffic_unit == '1' ) //Gbytes => MBytes
		document.getElementById("data_warning").value = Math.round(warning_val*1024);		
}

function show_dateList(){
	var valuestr = "";

	free_options(document.form.modem_bytes_data_cycle);
	document.form.modem_bytes_data_cycle.options.length = 31;	
	for(var i = 0; i < 31; i++){
		valuestr = (i+1).toString();
		document.form.modem_bytes_data_cycle.options[i] = new Option(valuestr, valuestr);
		if(orig_usage_date == valuestr)
			document.form.modem_bytes_data_cycle.selectedIndex = i;
	}
}

function show_roaming_isp_list(ispStr){
	var option, optionText, i = 0;
	ispList = ispStr.toArray();
	ispList.sort();

	if(ispList.length > 0){
		if(document.form.modem_roaming_isp.options.length > 0)
			free_options(document.form.modem_roaming_isp);

		option = new Option('<#Auto#>', "");
		document.form.modem_roaming_isp.add(option);
		if(orig_modem_roaming_isp == "")
			document.form.modem_roaming_isp.selectedIndex = 0;
		else{
			var match = 0;
			for(i = 0; i < ispList.length; i++){
				if(orig_modem_roaming_isp == ispList[i][0])
					match = 1;
			}
			if(match == 0){
				optionText = orig_modem_roaming_isp+' ('+orig_modem_mode+')';
				option = new Option(optionText, orig_modem_roaming_isp);
				document.form.modem_roaming_isp.add(option);
				document.form.modem_roaming_isp.selectedIndex = document.form.modem_roaming_isp.length - 1;
			}
		}

		for(i = 0; i < ispList.length; i++){
			optionText = ispList[i][0]+' ('+ispList[i][2]+')';
			option = new Option(optionText, ispList[i][0]);
			document.form.modem_roaming_isp.add(option);
			if(orig_modem_roaming_isp == ispList[i][0] && orig_modem_mode == ispList[i][2])
				document.form.modem_roaming_isp.selectedIndex = document.form.modem_roaming_isp.length - 1;
		}

		document.getElementById("modem_roaming_isp").style.display = "";
		document.getElementById("isp_scan_button").value = "<#QIS_rescan#>";
	}
	else if(orig_modem_roaming_isp != ""){
		if(document.form.modem_roaming_isp.options.length > 0)
			free_options(document.form.modem_roaming_isp);
		option = new Option('<#Auto#>', "");
		document.form.modem_roaming_isp.add(option);
		optionText = orig_modem_roaming_isp+' ('+orig_modem_mode+')';
		option = new Option(optionText, orig_modem_roaming_isp);
		document.form.modem_roaming_isp.add(option);
		document.form.modem_roaming_isp.selectedIndex = 1;
		document.getElementById("modem_roaming_isp").style.display = "";
		document.getElementById("isp_scan_button").value = "<#QIS_rescan#>";
	}
	else
		document.getElementById("modem_roaming_isp").style.display = "none";
}

function setRoamingModem_Mode(RoamingIsp){
	var modem_mode_str = RoamingIsp.substr(-3, 2);
	switch(modem_mode_str){
		case "2G":
			document.form.modem_mode.value = "2";
			break;
		case "3G":
			document.form.modem_mode.value = "3";
			break;
		case "4G":
			document.form.modem_mode.value = "4";
			break;
		default:
			document.form.modem_mode.value = "0";
			break;
	}
}

function detect_scan_result(){
	$.ajax({
		url: '/ajax_scanIsp.asp',
		dataType: 'script',
		
		error: function(xhr){
			detect_scan_result();
		},
		success: function(response){
			if( scan_end == '0'){
				if(ispstr.length > 0){
					show_roaming_isp_list(ispstr);
					document.getElementById("loadingIcon").style.display = "none";
					document.getElementById("isp_scan_button").style.display = "";
					document.getElementById("warning_states").style.display = "";
				}
				else
					setTimeout("detect_scan_result();", 5000);
			}
			else if( scan_end == '2' || scan_end == '1' || scan_end == '3'){
				setTimeout("detect_scan_result();", 5000);
			}
			else{ //Never scan
				document.getElementById("loadingIcon").style.display = "none";
				document.getElementById("isp_scan_button").value = "<#CTL_scan#>";
				document.getElementById("isp_scan_button").style.display = "";
				document.getElementById("warning_states").style.display = "";
			}
		}
	});
}

function scan_isp(){
	document.getElementById("loadingIcon").style.display = "";
	document.getElementById("isp_scan_button").style.display = "none";
	document.getElementById("modem_roaming_isp").style.display = "none";
	document.getElementById("warning_states").style.display = "none";
	document.getElementById("connection_status").innerHTML = "Scanning";/*untranslated*/
	document.getElementById("mconnect_status").innerHTML = "Scanning";
	setTimeout("detect_scan_result();", 10000);
	document.simact_form.action_mode.value = "scan_isp";
	document.simact_form.submit();
}

function cancel_action(){
	if(usb_modem_act_auth == "1" || usb_modem_act_auth == "2")
		document.form.pin_verify[0].selected = true;
	else if(usb_modem_act_auth == "3")
		document.form.pin_verify[1].selected = true;
	show_sim_table(0);
}

function set_verify_pin(){	
	if(document.form.sim_pincode.value !=""){
		if(document.form.sim_pincode.value.search(/^\d{4,8}$/)==-1){
			document.getElementById("verify_pincode_status").innerHTML='<#JS_InvalidPIN#>';
			document.getElementById("verify_pincode_status").style.display="";
			document.form.sim_pincode.select();
			document.form.sim_pincode.focus();
		}
		else{
			document.simact_form.sim_pincode.value = document.form.sim_pincode.value;
			if(document.form.pin_verify[0].selected == true)
				document.simact_form.action_mode.value = "start_lockpin";
			else if(document.form.pin_verify[1].selected == true)
				document.simact_form.action_mode.value = "stop_lockpin";
			document.simact_form.submit();
			show_sim_table(0);
		}
	}
	else{
		document.getElementById("verify_pincode_status").innerHTML='<#Mobile_pin_hint#>';
		document.getElementById("verify_pincode_status").style.display="";
	}

	showLoading(4);
	setTimeout("check_simact_result(4);", 3000);
}

function change_sim_pin(){
	var pin_check = 0;
	var newPin_check = 0;

	if(document.form.sim_pincode.value !=""){
		if(document.form.sim_pincode.value.search(/^\d{4,8}$/)==-1){
			document.getElementById("verify_pincode_status").innerHTML="<#JS_InvalidPIN#>";
			document.getElementById("verify_pincode_status").style.display="";
		}
		else{
			document.simact_form.sim_pincode.value = document.form.sim_pincode.value;
			pin_check = 1;
		}
	}	
	else{
		document.getElementById("verify_pincode_status").innerHTML="<#Mobile_input_pin#>";
		document.getElementById("verify_pincode_status").style.display="";
	}	

	if(document.form.sim_newpin.value !=""){
		if(document.form.sim_newpin.value.search(/^\d{4,8}$/)==-1){
				document.getElementById("new_pincode_status").innerHTML='<#JS_InvalidPIN#>';
				document.getElementById("new_pincode_status").style.display="";
		}
		else{
			document.simact_form.sim_newpin.value = document.form.sim_newpin.value;
			newPin_check = 1;
		}
	}	
	else{
		document.getElementById("new_pincode_status").innerHTML="<#Mobile_newpin_hint#>";
		document.getElementById("new_pincode_status").style.display="";
	}

	if(pin_check && newPin_check){
		document.simact_form.action_mode.value = "start_pwdpin";
		document.simact_form.submit();
		showLoading(3);
		setTimeout("check_simact_result(3);", 3500);	
	}
}

function unblock_pin(){
	var puk_check = 0;
	var newPin_check = 0;

	if(document.form.sim_puk.value != ""){
		document.simact_form.sim_puk.value = document.form.sim_puk.value;
		puk_check = 1;
	}	
	else{
		document.getElementById("puk_status").innerHTML="<#Mobile_input_puk#>";
		document.getElementById("puk_status").style.display="";
	}	

	if(document.form.sim_newpin.value !=""){
		if(document.form.sim_newpin.value.search(/^\d{4,8}$/)==-1){
				document.getElementById("new_pincode_status").innerHTML='<#JS_InvalidPIN#>';
				document.getElementById("new_pincode_status").style.display="";
		}
		else{
			document.simact_form.sim_newpin.value = document.form.sim_newpin.value;
			newPin_check = 1;
		}
	}	
	else{
		document.getElementById("new_pincode_status").innerHTML="<#Mobile_newpin_hint#>";
		document.getElementById("new_pincode_status").style.display="";
	}

	if(puk_check && newPin_check){
		document.getElementById("loadingIcon_sim").style.display="";
		document.getElementById("sim_ok_button").style.display = "none";
		document.getElementById("sim_cancel_btn").style.display = "none";		
		document.simact_form.action_mode.value = "start_simpuk";
		document.simact_form.submit();
		simact_flag = 1;
		setTimeout("check_simact_result(1);", 3000);	
	}
}

function show_sim_table(show, action){ //show: 1-show  0-hide   action: 1-pin verification  2:pin modification 3: unlock sim
	if(show == 1){
		$("#sim_input").fadeIn(300);
		if(action == 1){
			document.getElementById("sim_formtitle").innerHTML = "SIM <#Mobile_pin_management#> - <#Mobile_pin_verify#>";
			document.getElementById("sim_title_desc").innerHTML = "<#Mobile_pin_hint#>";
			document.form.sim_pincode.value = "";
			document.getElementById("sim_pincode_tr").style.display = "";
			document.getElementById("sim_pincode_hd").innerHTML = "<#PIN_code#>";
			document.getElementById("sim_newpin_tr").style.display = "none";
			document.getElementById("sim_puk_tr").style.display = "none";
			document.getElementById("puk_remaining_tr").style.display = "none";
			document.getElementById("table_pin_remaining").innerHTML = pin_remaining_count;
			document.getElementById("pin_remaining_tr").style.display = "";
			document.form.sim_pincode.focus();
			document.getElementById('sim_ok_button').onclick = function(){ 
				set_verify_pin(); 
			}; 
		}
		else if(action == 2){
			document.getElementById("sim_formtitle").innerHTML = "SIM <#Mobile_pin_management#> - <#Mobile_pin_modify#>";
			document.getElementById("sim_title_desc").innerHTML = "";
			document.form.sim_pincode.value = "";
			document.getElementById("sim_pincode_tr").style.display = "";
			document.getElementById("sim_pincode_hd").innerHTML = "<#Mobile_old_pin#>";
			document.form.sim_newpin.value = "";
			document.getElementById("sim_newpin_tr").style.display = "";
			document.getElementById("sim_puk_tr").style.display = "none";
			document.getElementById("puk_remaining_tr").style.display = "none";
			document.getElementById("table_pin_remaining").innerHTML = pin_remaining_count;
			document.getElementById("pin_remaining_tr").style.display = "";			
			document.getElementById('sim_ok_button').onclick = function(){ 
				change_sim_pin(); 
			}; 
		}
		else if(action == 3){
			document.getElementById("sim_formtitle").innerHTML = "SIM <#Mobile_pin_management#> - <#Mobile_unblock_sim#>";
			document.getElementById("sim_title_desc").innerHTML = "<#Mobile_puk_hint#>";
			document.form.sim_puk.value = "";			
			document.getElementById("sim_puk_tr").style.display = "";
			document.form.sim_newpin.value = "";			
			document.getElementById("sim_newpin_tr").style.display = "";
			document.getElementById("sim_pincode_tr").style.display = "none";	
			document.getElementById("puk_remaining").innerHTML = puk_remaining_count;	
			document.getElementById("puk_remaining_tr").style.display = "";
			document.getElementById("pin_remaining_tr").style.display = "none";
			document.getElementById('sim_ok_button').onclick = function(){ 
				unblock_pin(); 
			}; 
		}
	}
	else if(show == 0){
		$("#sim_input").fadeOut(300);
	}
}

function configure_pin(){
	if(document.form.pincode.value !=""){
		if(document.form.pincode.value.search(/^\d{4,8}$/)==-1){
			document.getElementById("pincode_status").innerHTML="<#JS_InvalidPIN#> ";
			document.getElementById("pincode_status").style.display="";
		}
		else{
			document.getElementById("save_pin_btn").style.display="none";
			document.getElementById("save_pin_ckb_span").style.display="none";
			document.getElementById("loadingIcon_pin").style.display="";			
			document.simact_form.sim_pincode.value = document.form.pincode.value;
			if(document.form.save_pin_ckb.checked == true)
				document.simact_form.save_pin.value = "1";
			else
				document.simact_form.save_pin.value = "0";
			document.getElementById("usim_status").innerHTML = "<#Mobile_configuring#>";
			document.simact_form.action_mode.value = "start_simpin";
			document.simact_form.submit();
			simact_flag = 1;
			setTimeout("check_simact_result(2);", 5000);	
		}
	}
	else{
		document.getElementById("pincode_status").innerHTML='<#Mobile_pin_hint#>';
		document.getElementById("pincode_status").style.display="";
	}		
}

function getSimAuth(){
	document.simact_form.action_mode.value = "restart_simauth";
	document.simact_form.submit();
}

function change_autoAPN(autoAPN){
	if(autoAPN == "0")
		inputCtrl(document.form.modem_enable_option, 1);
	else
		inputCtrl(document.form.modem_enable_option, 0);
}

function reset_usage(){
	cookie.unset(keystr);
	document.simact_form.action_mode.value = "restart_resetcount";
	document.getElementById("reset_usage_btn").style.display = "none";
	document.getElementById("loadingIcon_reset").style.display = "";
	document.simact_form.submit();
	setTimeout("finish_reset_usage();", 4000);
}

function finish_reset_usage(){
	document.getElementById("loadingIcon_reset").style.display = "none";
	update_usage_data();
}

function show_change_hint(){
	document.getElementById("change_day_hint").style.display="";
}

function show_phone(show){
	if(show == "1")
		document.getElementById("sms_phone_tr").style.display = "";
	else if( show == "0")
		document.getElementById("sms_phone_tr").style.display = "none";
}

/*
var curState = '<% nvram_get("usb_modem_act_simdetect"); %>';
function set_simdetect(enable_flag){
	var confirm_str = "Change SIM Card etector setting will reboot the router. Are you sure you want to change it?";
	if(confirm(confirm_str)){
		document.simact_form.simdetect.value = enable_flag;
		document.simact_form.action_mode.value = "start_simdetect";
		showLoading();
		document.simact_form.submit();
	}
	else{
		if(enable_flag == "1"){
			curState = "0";
			$('#simdetect_switch').find('.iphone_switch').animate({backgroundPosition: -37}, "slow");
		}
		else{
			curState = "1";
			$('#simdetect_switch').find('.iphone_switch').animate({backgroundPosition: 0}, "slow");
		}
	}
	
}
*/

function change_apn_mode(){
	if(document.form.modem_autoapn.value == "1"){//Automatic
		inputCtrl(document.form.modem_country, 0);
		inputCtrl(document.form.modem_isp, 0);
		inputCtrl(document.form.modem_apn, 0);
		inputCtrl(document.form.modem_dialnum, 0);
		inputCtrl(document.form.modem_user, 0);
		inputCtrl(document.form.modem_pass, 0);
		document.getElementById("modem_apn_div_tr").style.display = "";
		document.getElementById("modem_dialnum_div_tr").style.display = "";
		document.getElementById("modem_user_div_tr").style.display = "";
		document.getElementById("modem_pass_div_tr").style.display = "";
		document.getElementById("modem_apn_div").innerHTML = apn;
		document.getElementById("modem_dialnum_div").innerHTML = dialnum;
		document.getElementById("modem_user_div").innerHTML = user;
		document.getElementById("modem_pass_div").innerHTML = pass;
	}
	else{//Manual
		inputCtrl(document.form.modem_country, 1);
		if(document.form.modem_country.value == "")
			inputCtrl(document.form.modem_isp, 0);
		else
			inputCtrl(document.form.modem_isp, 1);
		inputCtrl(document.form.modem_apn, 1);
		inputCtrl(document.form.modem_dialnum, 1);
		inputCtrl(document.form.modem_user, 1);
		inputCtrl(document.form.modem_pass, 1);
		document.getElementById("modem_apn_div_tr").style.display = "none";
		document.getElementById("modem_dialnum_div_tr").style.display = "none";
		document.getElementById("modem_user_div_tr").style.display = "none";
		document.getElementById("modem_pass_div_tr").style.display = "none";
	}
}

function check_update(){
	$.ajax({
		url: '/ajax_lte_info.asp',
		dataType: 'script',
		
		error: function(xhr){
			check_update();
		},
		success: function(response){
			var lte_update_val = parseInt(lte_update_status);

			if( lte_update_status == '')
				setTimeout("check_update();", 10000);
			else{
				if(lte_update_val == 0)
					setTimeout("check_update();", 1000);
				else if(lte_update_val >= 1){
					if(lte_update_val > 1)
						document.getElementById("update_msg").innerHTML = "Fail to upgrade software of LTE module.";
					else{
						document.getElementById("usb_modem_act_swver").innerHTML = usb_modem_act_swver;
						document.getElementById("update_msg").innerHTML = "Succeed to upgrade software of LTE module.";
					}
				}
			}
		}
	});
}

function update_lte_fw(){
	if(sim_state != "-1"){
		alert("Please remove SIM card before starting update.");
		return;
	}
	else{
		document.simact_form.action_mode.value = "update_lte_fw";
		document.simact_form.submit();
		showLoadingBar(390);
		setTimeout("check_update();", 390000);
	}
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

<div id="LoadingBar" class="popup_bar_bg">
<table cellpadding="5" cellspacing="0" id="loadingBarBlock" class="loadingBarBlock" align="center">
	<tr>
		<td height="80">
		<div id="loading_block1" class="Bar_container">
			<span id="proceeding_img_text"></span>
			<div id="proceeding_img"></div>
		</div>
		<div id="loading_block2" style="margin:5px auto; width:85%;">LTE module software is upgrading. Please wait about 6 minutes. <#Main_alert_proceeding_desc5#></div>
		<div id="loading_block3" style="margin:5px auto;width:85%; font-size:12pt;"></div>
		</td>
	</tr>
</table>
<!--[if lte IE 6.5]><iframe class="hackiframe"></iframe><![endif]-->
</div>

<div id="Loading" class="popup_bg"></div>

<iframe name="hidden_frame" id="hidden_frame" src="" width="0" height="0" frameborder="0"></iframe>

<form method="post" name="form" id="ruleForm" action="/start_apply.htm" target="hidden_frame" autocomplete="off">
<input type="hidden" name="productid" value="<% nvram_get("productid"); %>">
<input type="hidden" name="current_page" value="Advanced_MobileBroadband_Content.asp">
<input type="hidden" name="next_page" value="Advanced_MobileBroadband_Content.asp">
<input type="hidden" name="modified" value="0">
<input type="hidden" name="action_mode" value="apply">
<input type="hidden" name="action_script" value="restart_wan_if">
<input type="hidden" name="action_wait" value="10">
<input type="hidden" name="preferred_lang" id="preferred_lang" value="<% nvram_get("preferred_lang"); %>">
<input type="hidden" name="firmver" value="<% nvram_get("firmver"); %>">
<input type="hidden" name="modem_enable" value="<% nvram_get("modem_enable"); %>">
<input type="hidden" name="modem_bytes_data_limit" id="modem_bytes_data_limit" value="<% nvram_get("modem_bytes_data_limit"); %>">
<input type="hidden" name="modem_bytes_data_warning" id="modem_bytes_data_warning" value="<% nvram_get("modem_bytes_data_warning"); %>">
<input type="hidden" name="g3err_pin" value="<% nvram_get("g3err_pin"); %>">
<input type="hidden" name="wans_standby" value="<% nvram_get("wans_standby"); %>">
<input type="hidden" name="simdetect" value="">
<input type="hidden" name="modem_roaming_mode" value="<% nvram_get("modem_roaming_mode"); %>">

<!---- connect status start  ---->
<div id="mobile_status"  class="contentM_qis" style="box-shadow: 3px 3px 10px #000;">
	<table class="QISform_wireless" border=0 align="center" cellpadding="5" cellspacing="5">
		<tr>
			<td align="left">
			<span class="formfonttitle"><#menu5_3#> - <#Mobile_status_title#></span>
			<div style="width:600px; height:15px;overflow:hidden;position:relative;left:0px;top:5px;"><img src="/images/New_ui/export/line_export.png"></div>
			<div><#Mobile_status_desc1#></div>
			</td>
		</tr>
		<tr>
			<td>
				<div id="product_info">
				<table width="100%" border="1" align="center" cellpadding="4" cellspacing="0" class="FormTable">
					<thead>
					<tr>
						<td colspan="2"><#Product_information#></td>
					</tr>
					</thead>
			 		<tr><th><#Modelname#></th><td><% nvram_get("productid"); %></td></tr>
		  			<!--tr><th><#Hardware_version#></th><td><div id="modem_act_hwver"><% nvram_get("usb_modem_act_hwver"); %></div></td></tr-->
		  			<!--tr><th>LTE Modem Version</th><td><div id="usb_modem_act_swver"><% nvram_get("usb_modem_act_swver"); %></div></td></tr-->
		  			<!--tr><th>IMEI</th><td><div id="modem_act_imei"><% nvram_get("usb_modem_act_imei"); %></div></td></tr-->
					<tr><th>IMSI</th><td><div id="modem_act_imsi"><% nvram_get("usb_modem_act_imsi"); %></div></td></tr>
					<tr><th>ICCID</th><td><div id="modem_act_iccid"><% nvram_get("usb_modem_act_iccid"); %></div></td></tr>
		 		</table>
		 		</div>
	  		</td>
		</tr>

		<tr>
			<td>
				<div id="wireless_status" style="display:none;">
				<table width="100%" border="1" align="center" cellpadding="4" cellspacing="0" class="FormTable">
					<thead>
					<tr>
						<td colspan="2">Wireless Status</td>
					</tr>
					</thead>
					<tr id="cellid_tr" style="display:none;"><th>Cell ID</th><td><div id="modem_cellid"><% nvram_get("usb_modem_act_cellid"); %></div></td></tr>
		  			<tr id="rssi_tr" style="display:none;"><th>RSSI</th><td><span id="modem_rssi" style="color:#FFF;"><% nvram_get("usb_modem_act_rssi"); %></span>&nbsp;dBm</td></tr>
					<tr id="sinr_tr" style="display:none;"><th>SINR</th><td><span id="modem_sinr" style="color:#FFF;"><% nvram_get("usb_modem_act_sinr"); %></span>&nbsp;dB</td></tr>
		  			<tr id="rsrp_tr" style="display:none;"><th>RSRP</th><td><span id="modem_rsrp" style="color:#FFF;"><% nvram_get("usb_modem_act_rsrp"); %></span>&nbsp;dBm</td></tr>
					<tr id="rsrq_tr" style="display:none;"><th>RSRQ</th><td><span id="modem_rsrq" style="color:#FFF;"><% nvram_get("usb_modem_act_rsrq"); %></span>&nbsp;dBm</td></tr>
					<tr id="lac_tr" style="display:none;"><th>LAC</th><td><div id="modem_lac"><% nvram_get("usb_modem_act_lac"); %></div></td></tr>
		 		</table>
		 		</div>
	  		</td>
		</tr>

		<tr>
			<td>
				<div id="internet_usage">
				<table width="100%" border="1" align="center" cellpadding="4" cellspacing="0" class="FormTable">
					<thead>
					<tr>
						<td colspan="2"><#Mobile_internet_usage#></td>
					</tr>
					</thead>
					<tr><th><#PPPConnection_x_WANLink_itemname#></th><td><span id="mconnect_status" style="color:#FFF;"></span></td></tr>
					<tr>
						<th>SIM Provider</th><!--untranslated-->
						<td><span id="msim_provider" style="color:#FFF;"></span></td>
					</tr>
					<tr>
						<th>Network Provider</th><!--untranslated-->
						<td><div id="msignalsys" style="cursor:auto;float:left;" class="img_wrap2"></div><div id="misp" style="float:left;margin-top:10px;"></div></td>
					</tr>
					<tr><th><#Mobile_data_usage#></th><td><span id="totalTraffic" style="color:#FFF;"></span></td></tr>
					<tr><th><#Uplink_traffic#></th><td><span id="upTraffic" style="color:#FFF;"></span></td></tr>
					<tr><th><#Downlink_traffic#></th><td><span id="downTraffic" style="color:#FFF;"></span></td></tr>
					<tr><th><#Uplink_rate#></th><td><span id="upRate" style="color:#FFF;"></span></td></tr>
					<tr><th><#Downlink_rate#></th><td><span id="downRate" style="color:#FFF;"></span></td></tr>
					<tr><th><#Connection_time#></th><td><span id="connect_days"></span> <#Day#> <span id="connect_hours"></span> <#Hour#> <span id="connect_minutes"></span> <#Minute#> <span id="connect_seconds"></span> <#Second#></td></span></td></tr>
				</table>
				</div>
			</td>
		</tr>
	</table>

	<div style="margin-top:5px;padding-bottom:10px;width:100%;text-align:center;">
		<input class="button_gen" type="button" onclick="hide_status();" value="<#CTL_close#>">	
	</div>
</div>
<!--===================================Ending of connect status ===========================================-->

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
	  <table width="760px" border="0" cellpadding="5" cellspacing="0" class="FormTitle" id="FormTitle" style="-webkit-border-radius: 3px;-moz-border-radius: 3px;border-radius:3px;">
		<tbody>
		<tr>
			<td bgcolor="#4D595D" valign="top" height="680px">
				<div>&nbsp;</div>
				<div style="width:730px">
					<table width="730px">
						<tr>
							<td align="left">
								<span class="formfonttitle"><#menu5_3#> - <#Mobile_title#></span>
							</td>
						</tr>
					</table>
				</div>
				<div style="margin:5px;"><img src="/images/New_ui/export/line_export.png"></div>
	      		<div class="formfontdesc"><#WAN_page_desc#></div>			  

					<table  width="100%" border="1" align="center" cellpadding="4" cellspacing="0" bordercolor="#6b8fa3" class="FormTable" id="WANscap">
						<thead>
						<tr>
							<td colspan="2"><#wan_index#></td>
						</tr>
						</thead>							
						<tr>
							<th id="wan_inf_th"><#WAN_Interface_Title#></th>
							<td align="left">
								<select class="input_option" name="wan_unit" onchange="change_wan_unit(this);">
								</select>
							</td>
						</tr>

						<tr>
							<th><#Mobile_enable#></th>
							<td>
								<select name="wan_enable" id="wan_enable" class="input_option" onchange="modem_enable_act(this.value);">
									<option value="1" <% nvram_match("wan_enable", "1","selected"); %>><#WLANConfig11b_WirelessCtrl_button1name#></option>
									<option value="0" <% nvram_match("wan_enable", "0","selected"); %>><#WLANConfig11b_WirelessCtrl_buttonname#></option>
								</select>
							</td>
						</tr>
					</table>

					<table width="100%" border="1" align="center" cellpadding="4" cellspacing="0" class="FormTable"  style="margin-top:8px">
						<thead>
						<tr>
							<td colspan="2">Mobile Broadband Modem Information</td>
						</tr>
						</thead>
			  			<!--tr><th>Modem hardware version</th><td><div id="modem_act_hwver"><% nvram_get("usb_modem_act_hwver"); %></div></td></tr-->
			  			<tr><th>Modem software version</th><td><div id="usb_modem_act_swver"><% nvram_get("usb_modem_act_swver"); %></div></td></tr>
			  			<tr id="new_version_tr" style="display:none;"><th>New software version</th><td><span id="new_version" style="color:#FFF;"></span><input id="lte_update_btn" name="lte_update_btn" class="button_gen" type="button" onclick="update_lte_fw();" style="margin-left:10px;" value="<#LANHostConfig_x_DDNSStatus_buttonname#>"/><div id="update_msg" style="color:#FC0;"></div></td></tr>
			  			<tr><th>IMEI</th><td><div id="modem_act_imei"><% nvram_get("usb_modem_act_imei"); %></div></td></tr>
			 		</table>
					 <div id="lte_update_note" style="color:#FFCC00; font-size:10px; display:none;">* Please remove SIM card before starting update and do not remove or unmount USB drive before update process is finished. </div>

					<div id="basic_setting_desc" class="formfontdesc" style="margin-bottom:0px; margin-top: 15px;"><#Mobile_desc1#></div>
					<!--table id="simdetect_table" width="100%" border="1" align="center" cellpadding="4" cellspacing="0" bordercolor="#6b8fa3"  class="FormTable" style="margin-top:8px">
						<thead>
						<tr>
							<td colspan="2"><#Mobile_SIM_Detector#></td>
						</tr>
						</thead>
						<tr>
							<th><a class="hintstyle" href="javascript:void(0);" onClick="openHint(28, 5);"><#Mobile_SIM_Detector#></a></th>
							<td>
					    		<div class="left" style="width:94px; float:left;" id="simdetect_switch"></div>
									<div class="iphone_switch_container" style="height:32px; width:74px; position: relative; overflow: hidden"></div>
									<script type="text/javascript">
										$('#simdetect_switch').iphoneSwitch('<% nvram_get("usb_modem_act_simdetect"); %>', 
											function() {
												curState = "1";
												set_simdetect("1");
												return true;
											},
											function() {
											 	curState = "0";
												set_simdetect("0");
												return true;
											 }
										);
									</script>
							</td>
						</tr>
					</table-->

				<table id="connection_table" width="100%" border="1" align="center" cellpadding="4" cellspacing="0" bordercolor="#6b8fa3"  class="FormTable" style="margin-top:8px">
				  	<thead>
				  	<tr>
						<td colspan="2"><#menu5_3_1#></td>
				  	</tr>
				  	</thead>		
				  
				  	<tr>
						<th width="200"><#ConnectionStatus#></th>
						<td>
							<span id="connection_status"></span>
							<div><img onclick="Show_status()" style="cursor:pointer;position:absolute;margin-left:450px;margin-top:-19px;" title="<#Mobile_inf#>" src="/images/New_ui/helpicon.png"></div>
						</td>		
				  	</tr>

					<tr id="newtork_type_tr">
						<th width="40%"><#Network_type#></th>
						<td>
							<select name="modem_mode" id="modem_mode" class="input_option">
								<option value="0" <% nvram_match("modem_mode", "0", "selected"); %>>Auto</option>
								<option value="43" <% nvram_match("modem_mode", "43", "selected"); %>>4G/3G</option>
								<option value="4" <% nvram_match("modem_mode", "4", "selected"); %>>4G only</option>
								<option value="3" <% nvram_match("modem_mode", "3", "selected"); %>>3G only</option>
								<option value="2" <% nvram_match("modem_mode", "2", "selected"); %>>2G only</option>
							</select>
						</td>
					</tr>

					<tr id="modem_pdp_tr" style="display:none">
						<th width="40%"><#Mobile_pdp_type#></th>
						<td>
							<select name="modem_pdp" id="modem_pdp" class="input_option">
								<option value="0" <% nvram_match("modem_pdp", "0", "selected"); %>>IPv4</option>
								<option value="1" <% nvram_match("modem_pdp", "1", "selected"); %>>PPP</option>
								<option value="2" <% nvram_match("modem_pdp", "2", "selected"); %>>IPv6</option>
								<option value="3" <% nvram_match("modem_pdp", "3", "selected"); %>>IPv4&IPv6</option>
							</select>
						</td>
					</tr>

					<tr>
						<th width="40%">LTE Band</th>
						<td>
							<select name="modem_lte_band" id="modem_lte_band" class="input_option">
								<option value="auto" <% nvram_match("modem_lte_band", "auto", "selected"); %>>Auto</option>
								<option value="B3" <% nvram_match("modem_lte_band", "B3", "selected"); %>>B3</option>
								<option value="B7" <% nvram_match("modem_lte_band", "B7", "selected"); %>>B7</option>
								<option value="B20" <% nvram_match("modem_lte_band", "B20", "selected"); %>>B20</option>
								<option value="B38" <% nvram_match("modem_lte_band", "B38", "selected"); %>>B38</option>
							</select>
						</td>
					</tr>

					<tr id="modem_roaming_tr" style="display:none">
						<th width="40%"><#Mobile_roaming#></th>
						<td>
							<select name="modem_roaming" id="modem_roaming" class="input_option" onchange="ShowRoamingOpt(this.value);">
								<option value="1" <% nvram_match("modem_roaming", "1","selected"); %>><#WLANConfig11b_WirelessCtrl_button1name#></option>
								<option value="0" <% nvram_match("modem_roaming", "0","selected"); %>><#WLANConfig11b_WirelessCtrl_buttonname#></option>
							</select>
						</td>
					</tr>

					<tr id="roaming_isp" style="display:none">
						<th width="40%"><#Mobile_roaming_isp#></th>
						<td>
							<select id="modem_roaming_isp" name="modem_roaming_isp" class="input_option" style="display:none;" onchange="setRoamingModem_Mode(this.options[this.selectedIndex].text);"></select>
							<input type="button" id = "isp_scan_button" name = "isp_scan_button" class="button_gen" onclick="scan_isp();" value="<#CTL_scan#>"/>
							<img id="loadingIcon" style="display:none;" src="/images/InternetScan.gif">
							<div id = "warning_states"><span>*<#Mobile_roaming_warning#></span></div>
						</td>
					</tr>
				</table>

				<table id="traffic_table" width="100%" border="1" align="center" cellpadding="4" cellspacing="0" bordercolor="#6b8fa3"  class="FormTable" style="margin-top:8px">
				  <thead>
				  <tr>
					<td colspan="2"><#Mobile_traffic_limit#></td>
				  </tr>
				  </thead>		
				  
				  <tr>
					<th width="200"><#Mobile_data_usage#></th>
					<td>
						<span id="data_usage" style="color:#FFF;"></span>
						<span id="starting_date"></span>
						<span><input id="reset_usage_btn" name="reset_usage_btn" class="button_gen" type="button" onclick="reset_usage();" style="margin-left:10px; display:none;" value="<#CTL_clear#>"/></span>
						<img id="loadingIcon_reset" style="margin-left:10px; display:none;" src="/images/InternetScan.gif">
					</td>
				  </tr>
				  
				  <tr id="remaining_data_tr">
					<th><#Mobile_remaining_data#></th>
					<td>
					  	<span id="remaining_data" style="color:#FFF;"></span>
					</td>
				  </tr>

				  <tr>
					<th><a class="hintstyle" href="javascript:void(0);" onClick="openHint(28, 1);"><#Mobile_reset_usage#></a></th>
					<td>
						<select id="modem_bytes_data_cycle" name="modem_bytes_data_cycle" class="input_option" onchange="show_change_hint();"></select>
						<span id="change_day_hint" style="display:none;">( <#Mobile_reset_day_hint#> )</span>
					</td>
				  </tr>

				  <tr>
					<th><a class="hintstyle" href="javascript:void(0);" onClick="openHint(28, 2);"><#Mobile_usage_limit#></a></th>
					<td>
					  <input type="text" maxlength="15" class="input_15_table" style="margin-left:0px;" id="data_limit" name="data_limit" value="" onkeypress="return validator.isNumberFloat(this,event)" onchange="change_limit_value();" autocorrect="off" autocapitalize="off"/>
					  	<span>
					  		<select name="modem_limit_unit" class="input_option" onchange="change_limit_unit(document.form.modem_limit_unit.value);">
					  			<option value="0" <% nvram_match("modem_limit_unit", "0", "selected"); %>>GBytes</option>
								<option value="1" <% nvram_match("modem_limit_unit", "1", "selected"); %>>MBytes</option>
							</select>
						</span>
						<span>(<#zero_disable#>)</span>
					</td>
				  </tr>		

				  <tr>
					<th><a class="hintstyle" href="javascript:void(0);" onClick="openHint(28, 3);"><#Mobile_usage_warning#></a></th>
					<td>
					  <input type="text" maxlength="15" class="input_15_table" style="margin-left:0px;" id="data_warning" name="data_warning" value="" onkeypress="return validator.isNumberFloat(this,event);" autocorrect="off" autocapitalize="off"/>
					  	<span>
					  		<select name="modem_warning_unit" class="input_option" onchange="change_warning_unit(document.form.modem_warning_unit.value);">
					  			<option value="0" <% nvram_match("modem_warning_unit", "0", "selected"); %>>GBytes</option>
								<option value="1" <% nvram_match("modem_warning_unit", "1", "selected"); %>>MBytes</option>
							</select>
						</span>
						<span>(<#zero_disable#>)</span>
					</td>
				  </tr>	
				  <tr id="sms_notif_tr">
					<th width="40%"><a class="hintstyle" href="javascript:void(0);" onClick="openHint(28, 4);"><#Mobile_SMS_Notification#></a></th>
					<td>
						<select name="modem_sms_limit" id="modem_sms_limit" class="input_option" onchange="show_phone(document.form.modem_sms_limit.value);">
							<option value="1" <% nvram_match("modem_sms_limit", "1","selected"); %>><#WLANConfig11b_WirelessCtrl_button1name#></option>
							<option value="0" <% nvram_match("modem_sms_limit", "0","selected"); %>><#WLANConfig11b_WirelessCtrl_buttonname#></option>
						</select>
					</td>
				  </tr>
				  <tr id="sms_phone_tr" style="display: none">
					<th width="40%"><#Mobile_phone_no#></th>
					<td>
						<input type="text" maxlength="12" class="input_15_table" id="modem_sms_phone" name="modem_sms_phone" value="<% nvram_get("modem_sms_phone"); %>" onkeypress="return validator.isNumber(this,event);" autocorrect="off" autocapitalize="off"/>
					</td>
				  </tr>
				</table>

			  	<table id="apn_table" width="100%" border="1" align="center" cellpadding="4" cellspacing="0" bordercolor="#6b8fa3"  class="FormTable" style="margin-top:8px">
					<thead>
					<tr>
						<td colspan="2"><#Mobile_apn_profile#></td>
					</tr>
					</thead>

					<tr>
						<th width="40%"><#APN_configuration#></th>
						<td>
							<select name="modem_autoapn" id="modem_autoapn" class="input_option" onchange="change_apn_mode();">
								<option value="1" <% nvram_match("modem_autoapn", "1","selected"); %>><#Auto#></option>
								<option value="0" <% nvram_match("modem_autoapn", "0","selected"); %>><#Manual_Setting_btn#></option>
							</select>
						</td>
					</tr>

					<tr>
          				<th><a class="hintstyle"  href="javascript:void(0);" onClick="openHint(21,9);"><#HSDPAConfig_Country_itemname#></a></th>
            			<td>
            				<select name="modem_country" class="input_option" onchange="switch_modem_mode(document.form.modem_enable_option.value);reloadProfile();"></select>
            				<!--div id="country_hint" style=""><span><#Mobile_country_hint#></span></div-->
						</td>
					</tr>
                                
			    	<tr>
			     		<th><a class="hintstyle"  href="javascript:void(0);" onClick="openHint(21,8);"><#HSDPAConfig_ISP_itemname#></a></th>
			    		<td><select name="modem_isp" class="input_option" onchange="show_APN_list();"></select></td>
			    	</tr>

					<tr style="display: none">
						<th width="40%">
							<a class="hintstyle" href="javascript:void(0);" onclick="openHint(21,1);"><#menu5_4_4#></a>
						</th>
						<td>
							<select name="modem_enable_option" id="modem_enable_option" class="input_option" onchange="switch_modem_mode(this.value);reloadProfile();">
								<option value="1" <% nvram_match("modem_enable", "1", "selected"); %>>WCDMA (UMTS)</option>
								<option value="2" <% nvram_match("modem_enable", "2", "selected"); %>>CDMA2000 (EVDO)</option>
								<option value="3" <% nvram_match("modem_enable", "3", "selected"); %>>TD-SCDMA</option>
							</select>
						</td>
					</tr>

          			<tr>
						<th><a class="hintstyle"  href="javascript:void(0);" onClick="openHint(21,3);"><#HSDPAConfig_private_apn_itemname#></a></th>
            		<td>
            			<input id="modem_apn" name="modem_apn" class="input_20_table" style="margin-left:0px;" type="text" value="" autocorrect="off" autocapitalize="off"/>
           				<img id="pull_arrow" height="14px;" src="/images/arrow-down.gif" style="position:absolute;*margin-left:-3px;*margin-top:1px;" onclick="pullLANIPList(this);" title="<#select_APN_service#>" onmouseover="over_var=1;" onmouseout="over_var=0;">
							<div id="ClientList_Block_PC" class="ClientList_Block_PC"></div>
					</td>
					</tr>

          			<tr id="modem_apn_div_tr" style="display:none;">
						<th><#HSDPAConfig_private_apn_itemname#></th>
	            		<td>
							<div id="modem_apn_div" style="color:#FFFFFF; margin-left:1px;"></div>
						</td>
					</tr>

					<tr>
						<th><#HSDPAConfig_DialNum_itemname#></th>
						<td>
							<input id="modem_dialnum" name="modem_dialnum" class="input_20_table" style="margin-left:0px;" type="text" value="" autocorrect="off" autocapitalize="off"/>
						</td>
					</tr>

          			<tr id="modem_dialnum_div_tr" style="display:none;">
						<th><#HSDPAConfig_DialNum_itemname#></th>
	            		<td>
							<div id="modem_dialnum_div" style="color:#FFFFFF; margin-left:1px;"></div>
						</td>
					</tr>					
                                
					<tr>
						<th><a class="hintstyle"  href="javascript:void(0);" onClick="openHint(21,11);"><#HSDPAConfig_Username_itemname#></a></th>
						<td>
						<input id="modem_user" name="modem_user" class="input_20_table" style="margin-left:0px;" type="text" value="<% nvram_get("modem_user"); %>" autocorrect="off" autocapitalize="off"/>
						</td>
					</tr>

          			<tr id="modem_user_div_tr" style="display:none;">
						<th><#HSDPAConfig_Username_itemname#></th>
	            		<td>
							<div id="modem_user_div" style="color:#FFFFFF; margin-left:1px;"></div>
						</td>
					</tr>

					<tr>
						<th><a class="hintstyle"  href="javascript:void(0);" onClick="openHint(21,12);"><#PPPConnection_Password_itemname#></a></th>
						<td>
							<input id="modem_pass" name="modem_pass" class="input_20_table"  style="margin-left:0px;" type="password" value="<% nvram_get("modem_pass"); %>" autocorrect="off" autocapitalize="off"/>
						</td>
					</tr>

          			<tr id="modem_pass_div_tr" style="display:none;">
						<th><#PPPConnection_Password_itemname#></th>
	            		<td>
							<div id="modem_pass_div" style="color:#FFFFFF; margin-left:1px;"></div>
						</td>
					</tr>

					<tr>
						<th><#PPPConnection_Authentication_itemname#></th>
						<td>
							<select name="modem_authmode" id="modem_authmode" class="input_option">
								<option value="0" <% nvram_match("modem_authmode", "0", "selected"); %>><#wl_securitylevel_0#></option>
								<option value="1" <% nvram_match("modem_authmode", "1", "selected"); %>>PAP</option>
								<option value="2" <% nvram_match("modem_authmode", "2", "selected"); %>>CHAP</option>
								<option value="3" <% nvram_match("modem_authmode", "3", "selected"); %>>PAP / CHAP</option>
							</select>
						</td>
					</tr>
				</table>

				<!--===================================Beginning of SIM Table ===========================================-->
				<div id="sim_input" style="box-shadow: 3px 3px 10px #000; position:absolute; background-color: #2B373B; margin-left:100px; margin-top: -50px; -webkit-border-radius: 5px;	-moz-border-radius: 5px; border-radius: 5px;display:none;"/>
					<table class="QISform_wireless" border=0 align="center" cellpadding="5" cellspacing="5">
						<tr>
							<td align="left">
							<span id="sim_formtitle" class="formfonttitle"></span>
							<div style="width:500px; height:15px;overflow:hidden;position:relative;left:0px;top:5px;"><img src="/images/New_ui/export/line_export.png"></div>
							<div id="sim_title_desc"></div>
							</td>
						</tr>
						<tr>
							<td>
								<div>
								<table width="100%" border="1" align="center" cellpadding="4" cellspacing="0" class="FormTable">
								  	<tr id="sim_puk_tr" style="display:none;">
										<th><#Mobile_puk#></th>
										<td>
					  						<input type="text" maxlength="8" class="input_20_table" name="sim_puk" autocapitalize="off" value="" onkeypress="return validator.isNumber(this,event)" autocorrect="off" autocapitalize="off"/>
					  						<br><span id="puk_status" style="display:none;"></span>
										</td>
				  					</tr>

							 		<tr id="sim_pincode_tr" style="display:none;">
							 			<th id="sim_pincode_hd"></th>
							 			<td>
							 				<input id="sim_pincode" name="sim_pincode" class="input_20_table" type="text" maxLength="8" value="<% nvram_get("modem_pincode"); %>" onkeypress="return validator.isNumber(this,event)" autocorrect="off" autocapitalize="off"/>
							 				<br><span id="verify_pincode_status" style="display:none;"></span>
							 			</td>
							 		</tr>
				  					<tr id="sim_newpin_tr" style="display:none;">
										<th><#Mobile_new_pin#></th>
											<td><input type="text" maxlength="8" class="input_20_table" name="sim_newpin" value=""  onkeypress="return validator.isNumber(this,event)" autocorrect="off" autocapitalize="off"/>
											<br><span id="new_pincode_status" style="display:none;"></span>
											</td>
				  					</tr>

				  					<tr id="pin_remaining_tr" style="display:none;">
										<th>PIN <#Mobile_remaining_num#></th>
										<td><span id="table_pin_remaining"></span></td>
				  					</tr>

				  					<tr id="puk_remaining_tr" style="display:none;">
										<th><#Mobile_remaining_num#></th>
										<td><span id="puk_remaining"></span></td>
				  					</tr>
						 		</table>
						 		</div>
					  		</td>
						</tr>
					</table>

					<div style="margin-top:5px;padding-bottom:10px;width:100%;text-align:center;">
						<input id="sim_cancel_btn" class="button_gen" type="button" onclick="cancel_action();" value="<#CTL_Cancel#>">
						<input id="sim_ok_button" class="button_gen" type="button" onclick="" value="<#CTL_ok#>">
						<img id="loadingIcon_sim" style="margin-left:10px; display:none;" src="/images/InternetScan.gif">
					</div>
				</div>
				<!--===================================End of SIM Table ===========================================-->

				<table id="sim_mgnt_table" width="100%" border="1" align="center" cellpadding="4" cellspacing="0" bordercolor="#6b8fa3"  class="FormTable" style="margin-top:8px">
				  <thead>
				  	<tr>
						<td colspan="2">SIM <#Mobile_pin_management#></td>
				  	</tr>
				  </thead>
				  
				  	<tr>
						<th width="200"><#Mobile_usim_status#></th>
						<td><span id="usim_status"></span><span ><input class="button_gen" id="unblock_btn" type="button" onclick="show_sim_table(1, 3);" style="margin-left:10px; display:none;" value="Unblock" ></span></td>
				  	</tr>
				  
				  	<tr id="pin_verify_tr" style="display:none;">
						<th width="40%"><#Mobile_pin_verify#></th>
						<td>
							<select name="pin_verify" id="pin_verify" class="input_option" onchange="show_sim_table(1, 1);">
								<option value="1"><#WLANConfig11b_WirelessCtrl_button1name#></option>
								<option value="0"><#WLANConfig11b_WirelessCtrl_buttonname#></option>
							</select>
							<span id="pin_verify_result" style="display: none;"></span>
						</td>
				  	</tr>

				  	<tr id="pin_modify_tr" style="display:none;">
						<th width="40%"><#Mobile_pin_modify#></th>
						<td>
							<input class="button_gen" type="button" onclick="show_sim_table(1, 2);" value="<#CTL_modify#>">	
							<span id="pin_modify_result" style="display: none"></span>
						</td>
				  	</tr>

					<tr id="pin_code_tr" style="display:none;">
						<th><a class="hintstyle"  href="javascript:void(0);" onClick="openHint(21,2);"><#PIN_code#></a></th>
						<td>
							<input id="pincode" name="pincode" class="input_20_table" type="text" maxLength="8" value="" onkeypress="return validator.isNumber(this,event)" autocorrect="off" autocapitalize="off"/>
							<span id="save_pin_ckb_span"><input type="checkbox" name="save_pin_ckb" id="save_pin_ckb" value="" onclick=""><#Mobile_save_pin#></input></span>
							<img id="loadingIcon_pin" style="margin-left:10px; display:none;" src="/images/InternetScan.gif">
							<span><input  id="save_pin_btn" name="save_pin_btn" class="button_gen" type="button" onclick="configure_pin();" style="margin-left:10px;" value="<#CTL_ok#>"></span>
							<br><span id="pincode_status" style="display:none;"></span><span id="pin_remaining"></span>
	
						</td>
					</tr>
				</table>
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
			<!--===================================End of Main Content===========================================-->
	</td>
  <td width="10" align="center" valign="top">&nbsp;</td>
	</tr>
</table>
</form>

<div id="footer"></div>

<form method="post" name="simact_form" action="/apply.cgi" target="hidden_frame">
<input type="hidden" name="action_mode" value="">
<input type="hidden" name="action_script" value="">
<input type="hidden" name="action_wait" value="">
<input type="hidden" name="sim_pincode" value="">
<input type="hidden" name="sim_newpin" value="">
<input type="hidden" name="sim_puk" value="">
<input type="hidden" name="save_pin" value="">
<input type="hidden" name="g3err_pin" value="0">
<input type="hidden" name="wan_unit" value="">
<input type="hidden" name="sim_order" value="">
<input type="hidden" name="simdetect" value="">
</form>
</body>
</html>
