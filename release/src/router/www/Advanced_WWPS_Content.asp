﻿<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Transitional//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd">
<html xmlns="http://www.w3.org/1999/xhtml">
<html xmlns:v>
<head>
<meta http-equiv="X-UA-Compatible" content="IE=Edge"/>
<meta http-equiv="Content-Type" content="text/html; charset=utf-8" />
<meta HTTP-EQUIV="Pragma" CONTENT="no-cache">
<meta HTTP-EQUIV="Expires" CONTENT="-1">
<link rel="shortcut icon" href="images/favicon.png">
<link rel="icon" href="images/favicon.png">
<title><#Web_Title#> - <#menu5_1_2#></title>
<link rel="stylesheet" type="text/css" href="index_style.css"> 
<link rel="stylesheet" type="text/css" href="form_style.css">
<script type="text/javascript" src="/js/jquery.js"></script>
<script type="text/javascript" src="/state.js"></script>
<script type="text/javascript" src="/general.js"></script>
<script type="text/javascript" src="/help.js"></script>
<script type="text/javascript" src="/popup.js"></script>
<script type="text/javascript" src="/js/httpApi.js"></script>
<script type="text/javascript" src="/switcherplugin/jquery.iphone-switch.js"></script>
<script type="text/javascript" src="/form.js"></script>
<style>
#wpa3_not_support_hint{
	margin-left:7px;
}
</style>
<script><% wl_get_parameter(); %>
$(function () {
	if(amesh_support && ((isSwMode("RT") || isSwMode("WISP")) || isSwMode("ap")) && ameshRouter_support) {
		addNewScript('/require/modules/amesh.js');
	}
});

const nvram = httpApi.nvramGet(["wsc_config_state", "wps_enable", "wps_band_x", "wl_wps_mode", "productid", "wl_closed", "wl0_radio", "wl1_radio"]);
var wsc_config_state_old = nvram.wsc_config_state;
var wps_enable_old = nvram.wps_enable;
var wl_wps_mode_old = nvram.wl_wps_mode;
var secs;
var timerID = null;
var timerRunning = false;
var timeout = 2000;
var delay = 1000;
var productid= nvram.productid;
var wl_ssid_closed = "<% nvram_get("wl_closed"); %>";
let main_fh_status = {};
var curState = nvram.wps_enable;
var radio_2 = nvram.wl0_radio;
var radio_5 = nvram.wl1_radio;
var band_string = "";
var faq_href_fail_WPA3P = "https://nw-dlcdnet.asus.com/support/forward.html?model=&type=Faq&lang="+ui_lang+"&kw=&num=204";

var current_page = window.location.pathname.split("/").pop();
var faq_index_tmp = get_faq_index(FAQ_List, current_page, 1);
let cur_wps_band = nvram.wps_band_x;

function reject_wps(auth_mode, wep){
	return (auth_mode == "open" && wep != "0") || auth_mode == "shared" || auth_mode == "psk" || auth_mode == "wpa" || auth_mode == "wpa2" || auth_mode == "wpawpa2" || auth_mode == "radius";
}

function get_band_str(band){
	if(band == 0)
		return "2.4 GHz";
	else if(band == 1){
		if(!wl_info.band5g_2_support)
			return "5 GHz";	
		else{
			return (band6g_support) ? '5 GHz' : '5 GHz-1';
		}
	}
	else if(band == 2){
		return '5 GHz-2';
	}
		
	return "";
}

function initial(){
	show_menu();
	if(!band5g_support){
		document.getElementById("wps_band_tr").style.display = "none";		
	}else{		
		if(wl_info.band5g_2_support || wl_info.band6g_support){	//Tri-band, RT-AC3200
			if(band6g_support){
				document.getElementById("wps_opt1").innerHTML = '5 GHz';
				// document.getElementById("wps_opt2").innerHTML = '6 GHz';
				document.getElementById("wps_opt2").remove();
			}
			
			document.getElementById("wps_switch").style.display = "none";
			document.form.wps_unit.value = cur_wps_band;
			document.getElementById("wps_select").style.display = "";
		}								
		
		document.getElementById("wps_band_tr").style.display = "";
		if(!wps_multiband_support || document.form.wps_multiband.value == "0") {
			document.getElementById("wps_band_word").innerHTML = get_band_str(document.form.wps_band.value);
			var band = get_band_str(document.form.wps_band.value);
			var band_prefix = '';
			switch (band){
				case "2.4 GHz":
					band_prefix = '2g1';
					break;

				case "5 GHz-1":
					band_prefix = '5g1';
					break;

				case "5 GHz-2":
					band_prefix = '5g2';
					break;

				case "6 GHz-1":
					band_prefix = '6g1';
					break;

				case "6 GHz-2":
					band_prefix = '6g2';
					break;
			}

			var auth = httpApi.nvramGet([band_prefix + '_auth_mode_x'])[band_prefix + '_auth_mode_x'];
			if(auth == 'sae' || auth == 'wpa3' || auth == 'suite-b'){
				document.getElementById('wpa3_not_support_hint').innerHTML = `<#wireless_JS_WPS_fail_WPA3P#> <#wireless_JS_WPS_fail_WPA3P_faq#>`;
				document.getElementById("faq_link").href=faq_href_fail_WPA3P;
				document.getElementById('wpa3_not_support_hint').style.display = "";
			}
		}

		if((wps_multiband_support && document.form.wps_multiband.value == "1") 
		|| document.form.wps_dualband.value == "1"){
			var rej0 = reject_wps(document.form.wl0_auth_mode_x.value, document.form.wl0_wep_x.value);
			var rej1 = reject_wps(document.form.wl1_auth_mode_x.value, document.form.wl1_wep_x.value);
			band0 = get_band_str(0);
			band1 = get_band_str(1);
			if (rej0)
				band0 = "<del>" + band0 + "</del>";
			if (rej1)
				band1 = "<del>" + band1 + "</del>";
			
			document.getElementById("wps_band_word").innerHTML = band0 + " / " + band1;
			var auth = httpApi.nvramGet(['2g1_auth_mode_x', '5g1_auth_mode_x']);
			var wl0_auth = auth['2g1_auth_mode_x'];
			var wl1_auth = auth['5g1_auth_mode_x'];
			if(wl0_auth == 'sae' || wl0_auth == 'wpa3' || wl0_auth == 'suite-b'
			|| wl1_auth == 'sae' || wl1_auth == 'wpa3' || wl1_auth == 'suite-b'){
				document.getElementById('wpa3_not_support_hint').innerHTML = `<#wireless_JS_WPS_fail_WPA3P#> <#wireless_JS_WPS_fail_WPA3P_faq#>`;
				document.getElementById("faq_link").href=faq_href_fail_WPA3P;
				document.getElementById('wpa3_not_support_hint').style.display = "";
			}
		}
	}

	if(lantiq_support){
		checkWLReady();
	}
	
	if(!ValidateChecksum(document.form.wps_sta_pin.value) || document.form.wps_sta_pin.value == "00000000"){
		document.form.wps_method[0].checked = true;
		changemethod(0);
	}
	else{
		document.form.wps_method[1].checked = true;		
		changemethod(1);
	}

	loadXML();
	document.getElementById('WPS_hideSSID_hint').innerHTML = "<#FW_note#> <#WPS_hideSSID_hint#>";
	if("<% nvram_get("wl_closed"); %>" == 1){
		document.getElementById('WPS_hideSSID_hint').style.display = "";
	}
	main_fh_status = get_mainfh_by_wl_unit($('select[name="wps_unit"]').val());
}

function SwitchBand(){
	if(wps_enable_old == "0"){
		var wps_band = document.form.wps_band.value;
		var wps_multiband = document.form.wps_multiband.value;
		// wps_multiband, wps_band: result
		// 0, 0: 2.4GHz
		// 0, 1: 5GHz
		// 1, X: 2.4GHz + 5GHz
		if(wps_multiband_support){
			if (wps_multiband == "1"){
				document.form.wps_multiband.value = 0;
				document.form.wps_band.value = 0;
			}
			else if (wps_multiband == "0" && wps_band == "0"){
				document.form.wps_multiband.value = 0;
				document.form.wps_band.value = 1;
			}
			else if (wps_multiband == "0" && wps_band == "1"){
				document.form.wps_multiband.value = 1;
				document.form.wps_band.value = 0;
			}
		}
		else{
			if(based_modelid == "RT-AC87U"){		//RT-AC87U dual band WPS, wps_band.value == 0 and wps_dualband == 1
				if(document.form.wps_dualband.value == "1"){
					document.form.wps_band.value = 0;
					document.form.wps_dualband.value = 0;
				}
				else{
					if(wps_band == "0"){
						document.form.wps_band.value = 1;
						document.form.wps_dualband.value = 0;
					}
					else{
						document.form.wps_band.value = 0;
						document.form.wps_dualband.value = 1;
					}
				}
			}
			else{		//single band  WPS 
				if(document.form.wps_band.value == "1")
					document.form.wps_band.value = 0;
				else
					document.form.wps_band.value = 1;		
			}
		}
	}
	else{
		document.getElementById("wps_band_hint").innerHTML = "* <#WLANConfig11b_x_WPSband_hint#>";
		return false;
	}

	FormActions("apply.cgi", "change_wps_unit", "", "");
	document.form.target = "";
	document.form.submit();
	applyRule();
}

function SelectBand(wps_band){
	if(wps_enable_old == "0"){
	var wps_band = document.form.wps_band.value;
	var wps_multiband = document.form.wps_multiband.value;
	if (!wps_multiband_support){
		if(document.form.wps_unit[0].selected)
			document.form.wps_band.value = 0;
		else if(document.form.wps_unit[1].selected)
			document.form.wps_band.value = 1;
		else if(document.form.wps_unit[2].selected)
			document.form.wps_band.value = 2;
		}
	}
	else{
		document.getElementById("wps_band_hint").innerHTML = "* <#WLANConfig11b_x_WPSband_hint#>";
	return false;
	}
	FormActions("apply.cgi", "change_wps_unit", "", "");
	document.form.target = "";
	document.form.submit();
	applyRule();
}

function done_validating(action){
	refreshpage();
}

function applyRule(){
	showLoading();
	stopFlag = 1;
	document.form.submit();
}

function enableWPS(){
	if(Qcawifi_support && amesh_support)
		document.form.action_script.value = "restart_wpsie";
	else
		document.form.action_script.value = "restart_wireless";
	document.form.action_mode.value = "apply_new";
	document.form.action_wait.value = "3";
	applyRule();
}

function configCommand(){
	var display = document.getElementById('wpa3_not_support_hint').style.display;
	if(display != 'none'){
		alert(`<#wireless_JS_WPS_fail_WPA3P#>`);
		return true;
	}

	if(lantiq_support && wave_ready != 1){
		alert(`<#Wireless_ready#>`);
		return false;
	}

	if(document.form.wps_method[1].checked == true){
		if(PIN_PBC_Check()){
			FormActions("apply.cgi", "wps_apply", "", "");
			document.form.target = "";
			applyRule();
		}
	}
	else{
		document.form.wps_sta_pin.value = "00000000";
		FormActions("apply.cgi", "wps_apply", "", "");
		document.form.target = "";
		applyRule();
	}
}

function resetWPS(){
	if(lantiq_support && wave_ready != 1){
		alert(`<#Wireless_ready#>`);
		return false;
	}

	var sec = 5;
	if(based_modelid == "BLUECAVE")
		sec = 30;
	if (Qcawifi_support)
		sec = 20;
	showLoading(sec);
	FormActions("apply.cgi", "wps_reset", "", sec.toString());
	document.form.submit();
	setTimeout('location.reload();', sec * 1000);
}

function resetTimer()
{
	if (stopFlag == 1)
	{
		stopFlag = 0;
		InitializeTimer();
	}
}

function ValidateChecksum(PIN){
	var accum = 0;

	accum += 3 * (parseInt(PIN / 10000000) % 10);
	accum += 1 * (parseInt(PIN / 1000000) % 10);
	accum += 3 * (parseInt(PIN / 100000) % 10);
	accum += 1 * (parseInt(PIN / 10000) % 10);
	accum += 3 * (parseInt(PIN / 1000) % 10);
	accum += 1 * (parseInt(PIN / 100) % 10);
	accum += 3 * (parseInt(PIN / 10) % 10);
	accum += 1 * (parseInt(PIN / 1) % 10);

	return ((accum % 10) == 0);
}

function PIN_PBC_Check(){
	var array_temp = new Array();
	if(document.form.wps_sta_pin.value != ""){
		if(document.form.wps_sta_pin.value.indexOf(' ')!= -1){
			array_temp = document.form.wps_sta_pin.value.split(" ");
			document.form.wps_sta_pin.value = array_temp[0] + array_temp[1];
		}
		else if(document.form.wps_sta_pin.value.indexOf('-')!= -1){
			array_temp = document.form.wps_sta_pin.value.split("-");
			document.form.wps_sta_pin.value = array_temp[0] + array_temp[1];
		}
	
		if(document.form.wps_sta_pin.value.length != 4 || isNaN(document.form.wps_sta_pin.value/1)){		//new format, 4 digits and doesn't need to checksum
			if(document.form.wps_sta_pin.value.length != 8 || !ValidateChecksum(document.form.wps_sta_pin.value)){
				alert("<#JS_InvalidPIN#>");
				document.form.wps_sta_pin.focus();
				document.form.wps_sta_pin.select();
				return false;
			}
		}
	}	

	return true;
}

function InitializeTimer()
{
	if(!wps_multiband_support && reject_wps(document.form.wl_auth_mode_x.value, document.form.wl_wep_x.value))
		return;
	else if(wps_multiband_support &&
		(reject_wps(document.form.wl0_auth_mode_x.value, document.form.wl0_wep_x.value) ||
		 reject_wps(document.form.wl1_auth_mode_x.value, document.form.wl1_wep_x.value)))
		return;
	
	msecs = timeout;
	StopTheClock();
	StartTheTimer();
}

function StopTheClock()
{
	if(timerRunning)
		clearTimeout(timerID);
	timerRunning = false;
}

function StartTheTimer(){
	if(msecs == 0){
		StopTheClock();
		
		if(stopFlag == 1)
			return;
		
		updateWPS();
		msecs = timeout;
		StartTheTimer();
	}
	else{
		msecs = msecs-500;
		timerRunning = true;
		timerID = setTimeout("StartTheTimer();", delay);
	}
}

function updateWPS(){
	require(['/require/modules/makeRequest.js'], function(makeRequest) {
		makeRequest.start('/WPS_info.xml', refresh_wpsinfo, updateWPS);
	});
}

function loadXML(){
	updateWPS();
	InitializeTimer();
}

function refresh_wpsinfo(xhr){
	if(xhr.responseText.search("Main_Login.asp") !== -1) top.location.href = "/";

	var wpss = xhr.responseXML.getElementsByTagName("wps");
	if(wpss == null || wpss[0] == null){
		if (confirm('<#JS_badconnection#>'))
			;
		else
			stopFlag=1;
		
		return;
	}
	
	if (!wps_multiband_support){
		var wps_infos = wpss[0].getElementsByTagName("wps_info");
		show_wsc_status(wps_infos);
	}
	else if (wps_multiband_support && document.form.wps_multiband.value == "0"){
		var wps_infos;
		if (document.form.wps_band.value == "0")
			wps_infos = wpss[0].getElementsByTagName("wps_info0");
		else
			wps_infos = wpss[0].getElementsByTagName("wps_info1");
		show_wsc_status(wps_infos);
	}
	else{
		var wps_infos0 = wpss[0].getElementsByTagName("wps_info0");
		var wps_infos1 = wpss[0].getElementsByTagName("wps_info1");
		show_wsc_status2(wps_infos0, wps_infos1);
	}

	if(lyra_hide_support)
		document.getElementById("devicePIN_tr").style.display = "none";
}

function show_wsc_status(wps_infos){
	var wep;
	var currentBand = 0;
	if (document.form.wps_band.value == "0")
		wep = document.form.wl0_wep_x.value
	else
		wep = document.form.wl1_wep_x.value
	// enable button
	if(wps_enable_old == "1"){
		document.getElementById("wps_enable_word").innerHTML = "<#btn_Enabled#>";
		document.getElementById("enableWPSbtn").value = "<#btn_disable#>";
		document.getElementById("switchWPSbtn").style.display = "none";
		if(wl_info.band5g_2_support || wl_info.band6g_support){
			document.getElementById("wps_switch").style.display = "";	
			document.getElementById("wps_select").style.display = "none";
		}
	}
	else{
		document.getElementById("wps_enable_word").innerHTML = "<#btn_Disabled#>"
		document.getElementById("enableWPSbtn").value = "<#WLANConfig11b_WirelessCtrl_button1name#>";
		document.getElementById("switchWPSbtn").style.display = "";
	}

	if(based_modelid == "RT-AC87U" || based_modelid == "RT-AC87R"){
		document.getElementById("switchWPSbtn").style.display = "none";
	}
	else if(wps_infos[12]?.firstChild?.nodeValue == 0){
			document.getElementById("wps_band_word").innerHTML = "2.4 GHz";
			band_string = "2.4 GHz";
			currentBand = 0;
	}
	else if(wps_infos[12]?.firstChild?.nodeValue == 1){
		if(!wl_info.band5g_2_support){
			document.getElementById("wps_band_word").innerHTML = "5 GHz";
			band_string = "5 GHz";
		}else{
			document.getElementById("wps_band_word").innerHTML = "5 GHz-1";
			band_string = "5 GHz-1";		
		}
			
		currentBand = 1;
	}	
	else if(wps_infos[12]?.firstChild?.nodeValue == 2){
		document.getElementById("wps_band_word").innerHTML = "5 GHz-2";
		band_string = "5 GHz-2";
		currentBand = 1;
	}

	
	var controlDisplayItem = function () {
		document.getElementById("wps_state_tr").style.display = "none";
		document.getElementById("devicePIN_tr").style.display = "none";
		document.getElementById("wpsmethod_tr").style.display = "none";
		if (wps_multiband_support)
			document.getElementById("wps_band_word").innerHTML = "<del>" + document.getElementById("wps_band_word").innerHTML + "</del>";
	};
	// First filter whether turn on Wi-Fi or not
	if(currentBand == 0 && radio_2 != "1") {	//2.4GHz
		document.getElementById("wps_enable_hint").innerHTML = "* <#note_turn_wifi_on_WPS#> <a style='color:#FC0; text-decoration: underline; font-family:Lucida Console;cursor:pointer;' onclick=\"_change_wl_advanced_unit_status(" + htmlEnDeCode.htmlEncode(wps_infos[12]?.firstChild?.nodeValue) + ");\"><#btn_go#></a>"
		controlDisplayItem();
		return;
	}
	else if(currentBand == 1 && radio_5 != "1") {	//5GHz
		document.getElementById("wps_enable_hint").innerHTML = "* <#note_turn_wifi_on_WPS#> <a style='color:#FC0; text-decoration: underline; font-family:Lucida Console;cursor:pointer;' onclick=\"_change_wl_advanced_unit_status(" + htmlEnDeCode.htmlEncode(wps_infos[12]?.firstChild?.nodeValue) + ");\"><#btn_go#></a>"
		controlDisplayItem();
		return;
	}
	else if (reject_wps(wps_infos[11]?.firstChild?.nodeValue, wep)){ // Second filter the authentication method
		document.getElementById("wps_enable_hint").innerHTML = "<#WPS_weptkip_hint#><br><#wsc_mode_hint1#> <a style='color:#FC0; text-decoration: underline; font-family:Lucida Console;cursor:pointer;' onclick=\"_change_wl_unit_status(" + htmlEnDeCode.htmlEncode(wps_infos[12]?.firstChild?.nodeValue) + ");\"><#menu5_1_1#></a> <#wsc_mode_hint2#>"
		controlDisplayItem();

		return;
	}
	//document.getElementById("wps_enable_block").style.display = "";
	
	// WPS status
	if(wps_enable_old == "0"){
		document.getElementById("wps_state_tr").style.display = "";
		document.getElementById("wps_state_td").innerHTML = "<#wps_state_not_used#>";
		document.getElementById("WPSConnTble").style.display = "none";
		document.getElementById("wpsDesc").style.display = "none";
	}
	else{
		document.getElementById("wps_state_tr").style.display = "";
		document.getElementById("wps_state_td").innerHTML = htmlEnDeCode.htmlEncode(wps_infos[0]?.firstChild?.nodeValue);
		if(productid=="RT-AC55U" || productid=="RT-AC55UHP")
		{   
		   	if(document.form.wps_band.value =="0" && wlan0_radio_flag == "0")
	   			document.getElementById("wps_state_td").innerHTML += " (2.4G is disabled)";
		   	if(document.form.wps_band.value == "1" && wlan1_radio_flag == "0")
		      	      	document.getElementById("wps_state_td").innerHTML += " (5G is disabled)";
		}	
		document.getElementById("WPSConnTble").style.display = "";
		document.getElementById("wpsDesc").style.display = "";
	}
	
	// device's PIN code
	document.getElementById("devicePIN_tr").style.display = "";
	document.getElementById("devicePIN").value = wps_infos[7]?.firstChild?.nodeValue;
	
	// the input of the client's PIN code
	document.getElementById("wpsmethod_tr").style.display = "";
	if(wps_enable_old == "1"){
		inputCtrl(document.form.wps_sta_pin, 1);
		if(wps_infos[1]?.firstChild?.nodeValue == "Yes"){
			document.getElementById("Reset_OOB").style.display = "";
			document.getElementById("Reset_OOB_desc").style.display = "";
		}
		else{
			document.getElementById("Reset_OOB").style.display = "none";
			document.getElementById("Reset_OOB_desc").style.display = "none";
		}
	}
	else{
		inputCtrl(document.form.wps_sta_pin, 0);
		document.getElementById("Reset_OOB").style.display = "none";
		document.getElementById("Reset_OOB_desc").style.display = "none";
	}
	
	// show connecting btn
	/*
	if(wps_infos[0]?.firstChild?.nodeValue == "Idle" || wps_infos[0]?.firstChild?.nodeValue == "Configured"){
		show_method = 1;
	}
	else if(Rawifi_support){ //ralink solutions
		var wpsState = wps_infos[0]?.firstChild?.nodeValue;
		if(wpsState.search("Received M") != -1 || wpsState.search("Send M") != -1 || wpsState == "Success")
			show_method = 1;
	}

	if(show_method == 1) {
		document.getElementById("addEnrolleebtn_client").style.display = "";
		document.getElementById("WPSConnTble").style.display = "";
		document.getElementById("wpsDesc").style.display = "";
		document.form.wps_sta_pin.focus();
	}
	else{
		document.getElementById("addEnrolleebtn_client").style.display = "none";
		document.getElementById("WPSConnTble").style.display = "none";
		document.getElementById("wpsDesc").style.display = "none";
	}
	*/

	if(wps_infos[0]?.firstChild?.nodeValue == "Start WPS Process")
		document.getElementById("wps_pin_hint").style.display = "inline";
	else
		document.getElementById("wps_pin_hint").style.display = "none";
	

	if(wps_infos[1]?.firstChild?.nodeValue == "No")
		document.getElementById("wps_config_td").innerHTML = "<#btn_Disabled#>";
	else
		document.getElementById("wps_config_td").innerHTML = "<#btn_Enabled#>";
}

function show_wsc_status2(wps_infos0, wps_infos1){
	var rej0 = reject_wps(wps_infos0[11]?.firstChild?.nodeValue, document.form.wl0_wep_x.value);
	var rej1 = reject_wps(wps_infos1[11]?.firstChild?.nodeValue, document.form.wl1_wep_x.value);
	// enable button
	if(wps_enable_old == "1"){
		document.getElementById("wps_enable_word").innerHTML = "<#btn_Enabled#>";
		document.getElementById("enableWPSbtn").value = "<#btn_disable#>";
		document.getElementById("switchWPSbtn").style.display = "none";
	}
	else{
		document.getElementById("wps_enable_word").innerHTML = "<#btn_Disabled#>"
		document.getElementById("enableWPSbtn").value = "<#WLANConfig11b_WirelessCtrl_button1name#>";

		band0 = get_band_str(wps_infos0[12]?.firstChild?.nodeValue);
		band1 = get_band_str(wps_infos1[12]?.firstChild?.nodeValue);

		if (rej0)
			band0 = "<del>" + band0 + "</del>";
		if (rej1)
			band1 = "<del>" + band1 + "</del>";

		document.getElementById("wps_band_word").innerHTML = band0 + " / " + band1;
		document.getElementById("switchWPSbtn").style.display = "";
	}

	var band_link = "";
	if(radio_2 !== "1" || radio_5 !== "1") {	// First filter whether turn on Wi-Fi or not
		if(radio_2 !== "1") {
			band_link += " <a style='color:#FC0; text-decoration: underline; font-family:Lucida Console;cursor:pointer;' onclick=\"_change_wl_advanced_unit_status(0);\"><#btn_go#> " + get_band_str(wps_infos0[12]?.firstChild?.nodeValue) + "</a>"
		}
		if(radio_5 !== "1") {
			band_link += " <a style='color:#FC0; text-decoration: underline; font-family:Lucida Console;cursor:pointer;' onclick=\"_change_wl_advanced_unit_status(1);\"><#btn_go#> " + get_band_str(wps_infos1[12]?.firstChild?.nodeValue) + "</a>"
		}

		document.getElementById("wps_enable_hint").innerHTML = "* <#note_turn_wifi_on_WPS#>" + band_link + "";

		if(radio_2 !== "1" && radio_5 !== "1") {
			document.getElementById("wps_state_tr").style.display = "none";
			document.getElementById("devicePIN_tr").style.display = "none";
			document.getElementById("wpsmethod_tr").style.display = "none";
			return;
		}
	}	
	else if(rej0 || rej1){	// Second filter the authentication method
		if(rej0)
			band_link += "<a style='color:#FC0; text-decoration: underline; font-family:Lucida Console;cursor:pointer;' onclick=\"_change_wl_unit_status(0);\"><#menu5_1_1#> " + get_band_str(wps_infos0[12]?.firstChild?.nodeValue) + "</a> ";
		if(rej1)
			band_link += "<a style='color:#FC0; text-decoration: underline; font-family:Lucida Console;cursor:pointer;' onclick=\"_change_wl_unit_status(1);\"><#menu5_1_1#> " + get_band_str(wps_infos1[12]?.firstChild?.nodeValue) + "</a> ";

		document.getElementById("wps_enable_hint").innerHTML = "<#WPS_weptkip_hint#><br><#wsc_mode_hint1#> " + band_link + " <#wsc_mode_hint2#>";

		if (rej0 && rej1){
			document.getElementById("wps_state_tr").style.display = "none";
			document.getElementById("devicePIN_tr").style.display = "none";
			document.getElementById("wpsmethod_tr").style.display = "none";
			return;
		}
	}

	// WPS status
	if(wps_enable_old == "0"){
		document.getElementById("wps_state_tr").style.display = "";
		if (!wps_multiband_support || document.form.wps_multiband.value == "0")
			document.getElementById("wps_state_td").innerHTML = "<#wps_state_not_used#>";
		else
			document.getElementById("wps_state_td").innerHTML = "<#wps_state_not_used#> / <#wps_state_not_used#>";
		document.getElementById("WPSConnTble").style.display = "none";
		document.getElementById("wpsDesc").style.display = "none";
	}
	else{
		document.getElementById("wps_state_tr").style.display = "";
		document.getElementById("wps_state_td").innerHTML = htmlEnDeCode.htmlEncode(wps_infos0[0]?.firstChild?.nodeValue);
		if(productid=="RT-AC55U" || productid=="RT-AC55UHP")
		   	if(wlan0_radio_flag == "0")
		   		document.getElementById("wps_state_td").innerHTML += " (2.4G is disabled)";

		document.getElementById("wps_state_td").innerHTML += " / " + htmlEnDeCode.htmlEncode(wps_infos1[0]?.firstChild?.nodeValue);
		if(productid=="RT-AC55U" || productid=="RT-AC55UHP")
		   	if( wlan1_radio_flag == "0")
	   			document.getElementById("wps_state_td").innerHTML += " (5G is disabled)";
		document.getElementById("WPSConnTble").style.display = "";
		document.getElementById("wpsDesc").style.display = "";
	}

	// device's PIN code
	document.getElementById("devicePIN_tr").style.display = "";
	document.getElementById("devicePIN").value = wps_infos0[7]?.firstChild?.nodeValue;

	// the input of the client's PIN code
	document.getElementById("wpsmethod_tr").style.display = "";
	if(wps_enable_old == "1"){
		inputCtrl(document.form.wps_sta_pin, 1);
		if(wps_infos0[1]?.firstChild?.nodeValue == "Yes" || wps_infos1[1]?.firstChild?.nodeValue == "Yes"){
			document.getElementById("Reset_OOB").style.display = "";
			document.getElementById("Reset_OOB_desc").style.display = "";
		}
		else{
			document.getElementById("Reset_OOB").style.display = "none";
			document.getElementById("Reset_OOB_desc").style.display = "none";
		}
	}
	else{
		inputCtrl(document.form.wps_sta_pin, 0);
		document.getElementById("Reset_OOB").style.display = "none";
		document.getElementById("Reset_OOB_desc").style.display = "none";
	}

	if(wps_infos0[0]?.firstChild?.nodeValue == "Start WPS Process" || wps_infos1[0]?.firstChild?.nodeValue == "Start WPS Process")
		document.getElementById("wps_pin_hint").style.display = "inline";
	else
		document.getElementById("wps_pin_hint").style.display = "none";

	band0 = "Yes"
	if(wps_infos0[1]?.firstChild?.nodeValue == "No")
		band0 = "No"

	band1 = "Yes"
	if(wps_infos1[1]?.firstChild?.nodeValue == "No")
		band1 = "No"

	document.getElementById("wps_config_td").innerHTML = band0 + " / " + band1;
}

function changemethod(wpsmethod){
	if(wpsmethod == 0){
		document.getElementById("starBtn").style.marginTop = "9px";
		document.getElementById("wps_sta_pin").style.display = "none";
	}
	else{
		document.getElementById("starBtn").style.marginTop = "5px";
		document.getElementById("wps_sta_pin").style.display = "";
	}
}

function _change_wl_unit_status(__unit){
	document.titleForm.current_page.value = "Advanced_Wireless_Content.asp?af=wl_auth_mode_x";
	document.titleForm.next_page.value = "Advanced_Wireless_Content.asp?af=wl_auth_mode_x";
	change_wl_unit_status(__unit);
}

function _change_wl_advanced_unit_status(__unit){
	document.titleForm.current_page.value = "Advanced_WAdvanced_Content.asp?af=wl_radio";
	document.titleForm.next_page.value = "Advanced_WAdvanced_Content.asp?af=wl_radio";
	change_wl_unit_status(__unit);
}

function checkWLReady(){
	$.ajax({
	    url: '/ajax_wl_ready.asp',
	    dataType: 'script',	
	    error: function(xhr) {
			setTimeout("checkWLReady();", 1000);
	    },
	    success: function(response){
	    	if(wave_ready != 1){
	    		$("#lantiq_ready").show();
	    		setTimeout("checkWLReady();", 1000);
	    	}
	    	else{
	    		$("#lantiq_ready").hide();
	    	}
			
	    }
  	});
}
function get_mainfh_by_wl_unit(_wl_unit){
	let mainfh_info = {};
	if(isSupport("sdn_mainfh")){
		const wifi_band_mainfh = (()=>{
			const options = [
				{"band":"2G","value":"1"},
				{"band":"5G","value":"2"},
				{"band":"5G1","value":"4"},
				{"band":"5G2","value":"8"},
				{"band":"6G","value":"16"},
				{"band":"6G1","value":"32"},
				{"band":"6G2","value":"64"}
			];
			let result = [];
			if(isSupport("noWiFi")){
				return result;
			}
			if(get_wl_unit_by_band("2G") != ""){
				result = result.concat(options.find(el => el.value === "1"));
			}
			if(get_wl_unit_by_band("5G2") != ""){
				result = result.concat(options.filter(el => ["4", "8"].includes(el.value)));
			}
			else if(get_wl_unit_by_band("5G") != ""){
				result = result.concat(options.find(el => el.value === "2"));
			}
			if(get_wl_unit_by_band("6G2") != ""){
				result = result.concat(options.filter(el => ["32", "64"].includes(el.value)));
			}
			else if(get_wl_unit_by_band("6G") != ""){
				result = result.concat(options.find(el => el.value === "16"));
			}
			return result;
		})();
		let specific_mainfh_band = 0;
		$.each(wifi_band_mainfh, function(index, item){
			const wl_unit = get_wl_unit_by_band(item.band);
			if(wl_unit == _wl_unit){
				specific_mainfh_band = parseInt(item.value);
				return false;
			}
		});
		let mainfh_info_list = [];
		const mainfh_sdn_rl = decodeURIComponent(httpApi.nvramCharToAscii(["sdn_rl"]).sdn_rl).split("<").filter(item => item.includes("MAINFH"));
		mainfh_sdn_rl.forEach(item=>{
			if(item != ""){
				const apmIdx = item.split(">")[5];
				const apm_ssid = decodeURIComponent(httpApi.nvramCharToAscii([`apm${apmIdx}_ssid`])[`apm${apmIdx}_ssid`]);
				const apm_enable = decodeURIComponent(httpApi.nvramCharToAscii([`apm${apmIdx}_enable`])[`apm${apmIdx}_enable`]);
				const apm_hide_ssid = decodeURIComponent(httpApi.nvramCharToAscii([`apm${apmIdx}_hide_ssid`])[`apm${apmIdx}_hide_ssid`]);
				const apm_dut_list = decodeURIComponent(httpApi.nvramCharToAscii([`apm${apmIdx}_dut_list`])[`apm${apmIdx}_dut_list`]).split("<");
				let apm_band = "0";
				if(apm_dut_list[1] != undefined && apm_dut_list[1] != ""){
					const dut_list_arr = apm_dut_list[1].split(">");
					apm_band = dut_list_arr[1];
				}
				mainfh_info_list.push({"ssid":apm_ssid, "band":apm_band, "enable":apm_enable, "hide_ssid":apm_hide_ssid});
			}
		});
		if(mainfh_info_list.length == 0){
			mainfh_info_list = [{"ssid":"", "band":"", "enable":"0", "hide_ssid":"1"}];
		}
		$.each(mainfh_info_list, function(index, item){
			const band = parseInt(item.band);
			if(band & specific_mainfh_band){
				mainfh_info = item;
				return false;
			}
		});
	}
	return mainfh_info;
}
</script>
</head>

<body onload="initial();" onunLoad="return unload_body();" class="bg">
<div id="TopBanner"></div>
<div id="Loading" class="popup_bg"></div>
<iframe name="hidden_frame" id="hidden_frame" src="" width="0" height="0" frameborder="0"></iframe>
<form method="POST" name="form" id="ruleForm" action="/start_apply.htm" target="hidden_frame">
<table class="content" align="center" cellpadding="0" cellspacing="0">
	<tr>
		<td width="17">&nbsp;</td>		
		<td valign="top" width="202">
		<div  id="mainMenu"></div>
		<div  id="subMenu"></div>	
		</td>
		<td valign="top">
	<div id="tabMenu" class="submenuBlock"></div>
		<!--===================================Beginning of Main Content===========================================-->
<input type="hidden" name="current_page" value="Advanced_WWPS_Content.asp">
<input type="hidden" name="next_page" value="Advanced_WWPS_Content.asp">
<input type="hidden" name="modified" value="0">
<input type="hidden" name="action_mode" value="">
<input type="hidden" name="action_script" value="">
<input type="hidden" name="action_wait" value="3">
<input type="hidden" name="first_time" value="">
<input type="hidden" name="preferred_lang" id="preferred_lang" value="<% nvram_get("preferred_lang"); %>">
<input type="hidden" name="firmver" value="<% nvram_get("firmver"); %>">
<input type="hidden" name="wps_enable" value="<% nvram_get("wps_enable"); %>">
<input type="hidden" name="wl_wps_mode" value="<% nvram_get("wl_wps_mode"); %>" disabled>
<input type="hidden" name="wl_auth_mode_x" value="<% nvram_get("wl_auth_mode_x"); %>">
<input type="hidden" name="wl_wep_x" value="<% nvram_get("wl_wep_x"); %>">
<input type="hidden" name="wps_band" value="<% nvram_get("wps_band_x"); %>">
<input type="hidden" name="wps_dualband" value="<% nvram_get("wps_dualband"); %>">
<input type="hidden" name="wl_crypto" value="<% nvram_get("wl_crypto"); %>">
<input type="hidden" name="wps_multiband" value="<% nvram_get("wps_multiband"); %>">
<input type="hidden" name="wl0_auth_mode_x" value="<% nvram_get("wl0_auth_mode_x"); %>">
<input type="hidden" name="wl0_wep_x" value="<% nvram_get("wl0_wep_x"); %>">
<input type="hidden" name="wl1_auth_mode_x" value="<% nvram_get("wl1_auth_mode_x"); %>">
<input type="hidden" name="wl1_wep_x" value="<% nvram_get("wl1_wep_x"); %>">
<input type="hidden" name="wl2_auth_mode_x" value="<% nvram_get("wl2_auth_mode_x"); %>">
<input type="hidden" name="wl2_wep_x" value="<% nvram_get("wl2_wep_x"); %>">
<input type="hidden" name="wl0_radio" value="<% nvram_get("wl0_radio"); %>">
<input type="hidden" name="wl1_radio" value="<% nvram_get("wl1_radio"); %>">
<input type="hidden" name="wl2_radio" value="<% nvram_get("wl2_radio"); %>">
<table width="98%" border="0" align="left" cellpadding="0" cellspacing="0">
	<tr>
		<td valign="top" >
		
<table width="760px" border="0" cellpadding="4" cellspacing="0" class="FormTitle" id="FormTitle">

	<tbody>
	<tr>
		  <td bgcolor="#4D595D" valign="top">
		  <div class="container">

		  <div>&nbsp;</div>
		  <div class="formfonttitle"><#menu5_1#> - <#menu5_1_2#></div>
		  <div class="formfonttitle_help"><i onclick="show_feature_desc(`<#HOWTOSETUP#>`)" class="icon_help"></i></div>
		  <div style="margin:10px 0 10px 5px;" class="splitLine"></div>
		  <div class="formfontdesc"><#WLANConfig11b_display6_sectiondesc#></div>
		  <div id="lantiq_ready" style="display:none;color:#FC0;margin-left:5px;font-size:13px;">Wireless is setting...</div>
		  <div id="WPS_hideSSID_hint" class="hint-color formfontdesc" style="display:none;color:#FFCC00;"></div>		  

		<table width="100%" border="1" align="center" cellpadding="4" cellspacing="0"  class="FormTable">
			<tr>
			  	<th width="30%"><a class="hintstyle" href="javascript:void(0);" onClick="openHint(13,1);"><#WLANConfig11b_x_WPS_itemname#></a></th>
			  	<td>
			    	<div id="wps_enable_block" style="display:none;">
			    		<span style="color:#FFF;" id="wps_enable_word">&nbsp;&nbsp;</span>
			    		<input type="button" name="enableWPSbtn" id="enableWPSbtn" value="" class="button_gen" onClick="enableWPS();">
			    		<br>
			    	</div>
						
			    	<div class="left" style="width: 94px;" id="radio_wps_enable"></div>
						<div class="clear"></div>					
						<script type="text/javascript">
							$('#radio_wps_enable').iphoneSwitch(curState,
								function() {
									const show_wps_hint = (()=>{
										if(isSupport("sdn_mainfh")){
											if($.isEmptyObject(main_fh_status)){
												return `${band_string} Main Network has not been established yet`;
											}
											else if(main_fh_status.hide_ssid == "1"){
												return `${band_string} : <#note_hide_SSID_WPS#>`;
											}
											else
												return "";
										}
										else{
											return (wl_ssid_closed == 1) ? `${band_string} : <#note_hide_SSID_WPS#>` : "";
										}
									})();
									if(show_wps_hint != ""){
										alert(show_wps_hint);
										$('#iphone_switch').animate({backgroundPosition: -37}, "slow", function() {});
										return false;
									}

									if(( wps_multiband_support && document.form.wps_multiband.value == "1")
									  || document.form.wps_dualband.value == "1"){	//Ralink, Qualcomm Atheros, RT-AC87U WPS multiband case
										if(	document.form.wl0_auth_mode_x.value == "shared" ||	document.form.wl1_auth_mode_x.value == "shared"
										||	document.form.wl0_auth_mode_x.value == "psk"	||	document.form.wl0_auth_mode_x.value == "wpa"
										||	document.form.wl1_auth_mode_x.value == "psk"	||	document.form.wl1_auth_mode_x.value == "wpa"
										||	document.form.wl0_auth_mode_x.value == "wpa2"    ||	document.form.wl0_auth_mode_x.value == "wpa2"
										||	document.form.wl1_auth_mode_x.value == "wpawpa2"    ||	document.form.wl1_auth_mode_x.value == "wpawpa2"
										||	document.form.wl0_auth_mode_x.value == "open" && (document.form.wl0_wep_x.value == "1" || document.form.wl0_wep_x.value == "2")
										||	document.form.wl1_auth_mode_x.value == "open" && (document.form.wl1_wep_x.value == "1" || document.form.wl1_wep_x.value == "2")){
											alert("<#note_auth_wpa_WPS#>");
											$('#iphone_switch').animate({backgroundPosition: -37}, "slow", function() {});
											return false;									
										}							
									}
									else{		//Broadcom, Ralink normal case
										if(document.form.wps_band.value == 0){
											if(	document.form.wl0_auth_mode_x.value == "shared"
											||	document.form.wl0_auth_mode_x.value == "psk"
											||	document.form.wl0_auth_mode_x.value == "wpa" ||	document.form.wl0_auth_mode_x.value == "wpa2" || document.form.wl0_auth_mode_x.value == "wpawpa2"
											||	document.form.wl0_auth_mode_x.value == "open" && (document.form.wl0_wep_x.value == "1" || document.form.wl0_wep_x.value == "2")){
												alert(band_string + " : <#note_auth_wpa_WPS#>");
												$('#iphone_switch').animate({backgroundPosition: -37}, "slow", function() {});
												return false;									
											}
											
											if(document.form.wl0_radio.value == 0){
												alert("<#note_turn_wifi_on_WPS#>");
												$('#iphone_switch').animate({backgroundPosition: -37}, "slow", function() {});
												return false;											
											}
										}
										else if(document.form.wps_band.value == 1){			//5G
											if(	document.form.wl1_auth_mode_x.value == "shared"
											||	document.form.wl1_auth_mode_x.value == "psk"
											||	document.form.wl1_auth_mode_x.value == "wpa" ||	document.form.wl1_auth_mode_x.value == "wpa2" || document.form.wl1_auth_mode_x.value == "wpawpa2"
											||	document.form.wl1_auth_mode_x.value == "open" && (document.form.wl1_wep_x.value == "1" || document.form.wl1_wep_x.value == "2")){
												alert(band_string + " : <#note_auth_wpa_WPS#>");
												$('#iphone_switch').animate({backgroundPosition: -37}, "slow", function() {});
												return false;									
											}

											if(document.form.wl1_radio.value == 0){
												alert("<#note_turn_wifi_on_WPS#>");
												$('#iphone_switch').animate({backgroundPosition: -37}, "slow", function() {});
												return false;	
											
											}
										}
										else if(document.form.wps_band.value == 2){		//5G-2
											if(	document.form.wl2_auth_mode_x.value == "shared"
											||	document.form.wl2_auth_mode_x.value == "psk"
											||	document.form.wl2_auth_mode_x.value == "wpa" ||	document.form.wl2_auth_mode_x.value == "wpa2" || document.form.wl2_auth_mode_x.value == "wpawpa2"
											||	document.form.wl2_auth_mode_x.value == "open" && (document.form.wl2_wep_x.value == "1" || document.form.wl2_wep_x.value == "2")){
												alert(band_string + " : <#note_auth_wpa_WPS#>");
												$('#iphone_switch').animate({backgroundPosition: -37}, "slow", function() {});
												return false;									
											}
											
											if(document.form.wl2_radio.value == 0){
												alert("<#note_turn_wifi_on_WPS#>");
												$('#iphone_switch').animate({backgroundPosition: -37}, "slow", function() {});
												return false;	
											
											}
										}					
									}
									
									if( !SG_mode || (SG_mode && confirm(stringSafeGet("<#note_turn_on_WPS#>")))){
										document.form.wps_enable.value = "1";
										enableWPS();
									}
									else{
										$('#iphone_switch').animate({backgroundPosition: -37}, "slow", function() {});
										return false;
									}
								 },
								 function() {
									document.form.wps_enable.value = "0";
									enableWPS();
								 }
							);
						</script>
						<span id="wps_enable_hint"></span>
				</td>
			</tr>
			<tr id="wps_band_tr">
				<th width="30%"><a class="hintstyle" href="javascript:void(0);" onclick="openHint(13,5);"><#Current_band#></th>
				
				<td id="wps_switch">
						<span class="devicepin" style="color:#FFF;" id="wps_band_word"></span>&nbsp;&nbsp;
						<input type="button" class="button_gen" name="switchWPSbtn" id="switchWPSbtn" value="<#Switch_band#>" class="button" onClick="SwitchBand();">
						<br><span id="wps_band_hint"></span>
		  	</td>
				<td  id="wps_select" style="display:none">
						<select name="wps_unit" class="input_option" onChange="SelectBand();">
							<option id="wps_opt0" class="content_input_fd" value="0" <% nvram_match("wps_band_x", "0","selected"); %>>2.4 GHz</option>
							<option id="wps_opt1" class="content_input_fd" value="1" <% nvram_match("wps_band_x", "1","selected"); %>>5 GHz-1</option>
							<option id="wps_opt2" class="content_input_fd" value="2" <% nvram_match("wps_band_x", "2","selected"); %>>5 GHz-2</option>
						</select>			
				</td>
			</tr>
			
			<tr id="wps_state_tr">
				<th><#PPPConnection_x_WANLink_itemname#></th>
				<td width="300">
					<span id="wps_state_td" style="margin-left:5px;"></span>
					<img id="wps_pin_hint" style="display:none;" src="images/InternetScan.gif" />
				</td>
			</tr>

			<tr>
				<th><#wireless_configured#></th>
				<td>
					<div style="margin-left:-10px">
						<table ><tr>
							<td style="border:0px;" >
								<div class="devicepin" style="color:#FFF;" id="wps_config_td"></div>
							</td>
							<td style="border:0px">
								<input class="btn_subusage button_gen" type="button" onClick="resetWPS();" id="Reset_OOB" name="Reset_OOB" value="<#CTL_Reset_OOB#>" style="padding:0 0.3em 0 0.3em;" >
								<div id="Reset_OOB_desc"><#WLANConfig11b_x_ResetWPS_desc#></div>
							</td>
						</tr></table>
					</div>
				</td>
			</tr>
			
			<tr id="devicePIN_tr">
			  <th>
			  	<span id="devicePIN_name"><a class="hintstyle" href="javascript:void(0);" onclick="openHint(13,4);"><#WLANConfig11b_x_DevicePIN_itemname#></a></span>			  
			  </th>
			  <td>
			  	<input type="text" name="devicePIN" id="devicePIN" value="" class="input_15_table" readonly="1" style="float:left;" autocorrect="off" autocapitalize="off"></input>
			  </td>
			</tr>
		</table>

		<table id="WPSConnTble" width="100%" border="1" align="center" cellpadding="4" cellspacing="0"  class="FormTable" style="display:none;">	

			<div  class="formfontdesc" style="width:97%;padding-bottom:10px;padding-top:10px;display:none;" id="wpsDesc">
				<#WPS_add_client#>
			</div>
			
			<tr id="wpsmethod_tr">
				<th>
			  	<span id="wps_method"><a class="hintstyle" href="javascript:void(0);" onclick="openHint(13,2);"><#WLANConfig11b_x_WPSMode_itemname#></a></span>
			  </th>
				<td>
					<label><input type="radio" name="wps_method" onclick="changemethod(0);" value="0"><#WLANConfig11b_x_WPS_pushbtn#></label>
					<label><input type="radio" name="wps_method" onclick="changemethod(1);" value="1"><#WLANConfig11b_x_WPSPIN_itemname#></label>
					<input type="text" name="wps_sta_pin" id="wps_sta_pin" value="" size="9" maxlength="9" class="input_15_table" autocorrect="off" autocapitalize="off">
					<div id="starBtn" style="margin-top:10px;"><input class="button_gen" type="button" style="margin-left:5px;" onClick="configCommand();" id="addEnrolleebtn_client" name="addEnrolleebtn"  value="<#wps_start_btn#>"></div>
					<div style="color:#FC0;display:none" id="wpa3_not_support_hint"></div>
				</td>
			</tr>

		</table>

		</div>	<!-- for .container  -->
		<div class="popup_container popup_element_second"></div>

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
