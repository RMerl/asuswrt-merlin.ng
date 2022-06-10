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

<title><#Web_Title#> - <#menu5_1_1#></title>
<link rel="stylesheet" type="text/css" href="index_style.css"> 
<link rel="stylesheet" type="text/css" href="form_style.css">
<link rel="stylesheet" type="text/css" href="usp_style.css">
<link href="other.css"  rel="stylesheet" type="text/css">
<script type="text/javascript" src="/js/jquery.js"></script>
<script type="text/javascript" src="/js/httpApi.js"></script>
<script type="text/javascript" src="/state.js"></script>
<script type="text/javascript" src="/js/device.js"></script>
<script type="text/javascript" src="/help.js"></script>
<script type="text/javascript" src="/general.js"></script>
<script type="text/javascript" src="/popup.js"></script>
<script type="text/javascript" src="/md5.js"></script>
<script type="text/javascript" src="/chanspec.js"></script>
<script type="text/javascript" src="/validator.js"></script>

<script type="text/javascript" src="/switcherplugin/jquery.iphone-switch.js"></script>


<script>
<% wl_get_parameter(); %>
$(function () {
	if(amesh_support && (isSwMode("rt") || isSwMode("ap")) && ameshRouter_support) {
		addNewScript('/require/modules/amesh.js');
	}
});
wl_channel_list_2g = <% channel_list_2g(); %>;
wl_channel_list_5g = <% channel_list_5g(); %>;
wl_channel_list_5g_2 = JSON.parse('<% channel_list_5g_2(); %>');
//var meshBackhaulAutoSupport = based_modelid === 'XT8PRO' ? true : false;
var meshBackhaulAutoSupport = false;
var unii4Support = (function(){
	for(item of wl_channel_list_5g){
		if(parseInt(item) > 165){
			return true;
		}
	}

	for(item of wl_channel_list_5g_2){
		if(parseInt(item) > 165){
			return true;
		}
	}

	return false;
})();
var current_control_channel = [<% wl_control_channel(); %>][0];
var reboot_needed_time = eval("<% get_default_reboot_time(); %>");
var wl_unit = <% nvram_get("wl_unit"); %>;
var country = '';
if(wl_unit == '1')
	country = '<% nvram_get("wl1_country_code"); %>';
else		
	country = '<% nvram_get("wl0_country_code"); %>';

var wl_bw_160 = '<% nvram_get("wl_bw_160"); %>';
var enable_bw_160 = (wl_bw_160 == 1) ? true : false;
var wl_reg_mode = '<% nvram_get("wl_reg_mode"); %>';
var faq_href = "https://nw-dlcdnet.asus.com/support/forward.html?model=&type=Faq&lang="+ui_lang+"&kw=&num=151";

function initial(){
	show_menu();
	handle_bandwidth('0', '<% nvram_get("wl0_bw"); %>');
	handle_bandwidth('1', '<% nvram_get("wl1_bw"); %>');
	handle_bandwidth('2', '<% nvram_get("wl2_bw"); %>');
	separateEnable_160MHz(document.getElementById('band1_160'))

	handle_channel('0', '<% nvram_get("wl0_chanspec"); %>');
	handle_channel('1', '<% nvram_get("wl1_chanspec"); %>');
	handle_channel('2', '<% nvram_get("wl2_chanspec"); %>');

	auth_method_change('0', '<% nvram_get("wl0_auth_mode_x"); %>', 'init');
	auth_method_change('1', '<% nvram_get("wl1_auth_mode_x"); %>', 'init');
	auth_method_change('2', '<% nvram_get("wl2_auth_mode_x"); %>', 'init');

	handle_smart_connect('<% nvram_get("smart_connect_x"); %>', 'init');

	gen_fronthaul_ap('<% nvram_get("smart_connect_x"); %>');

	document.form.band0_ssid.value = decodeURIComponent('<% nvram_char_to_ascii("", "wl0_ssid"); %>');
	document.form.band01_ssid.value = decodeURIComponent('<% nvram_char_to_ascii("", "wl0_ssid"); %>');
	document.form.band012_ssid.value = decodeURIComponent('<% nvram_char_to_ascii("", "wl0_ssid"); %>');
	document.form.band1_ssid.value = decodeURIComponent('<% nvram_char_to_ascii("", "wl1_ssid"); %>');
	document.form.band2_ssid.value = decodeURIComponent('<% nvram_char_to_ascii("", "wl2_ssid"); %>');

	if(band6g_support){
		$("#psc_faq_link")  //for string tag: PSC_Faq
			.attr("target", "_blank")
			.attr("style", "color:#FC0;text-decoration:underline;")
			.attr("href", faq_href);
	}

	var re_count = httpApi.hookGet("get_cfg_clientlist", true).length;
	if(re_count > 1){
		$(".select_auth_mode option[value='wpa2']").remove();
		$(".select_auth_mode option[value='wpawpa2']").remove();
	}

	controlHideSSIDHint();
	ajax_wl_channel();

	if(meshBackhaulAutoSupport){
		if(document.form.fh_ap_enabled.value === '1'){
			document.form.fh_ap_enabled.value = '0';
			document.querySelector('#fh_connection_hint').style.display = '';
			document.querySelector('#fh_connection_hint_checkbox').checked = true;
		}
		else if(document.form.fh_ap_enabled.value === '0'){
			document.querySelector('#fh_connection_hint').style.display = '';
			document.querySelector('#fh_connection_hint_checkbox').checked = false;
		}
		else if(document.form.fh_ap_enabled.value === '2'){
			document.querySelector('#fh_connection_hint').style.display = 'none';
			document.querySelector('#fh_connection_hint_checkbox').checked = false;
		}
	}

	if(unii4Support){
		document.querySelector('#acs_unii4_field').style.display = '';
	}
}

function cal_panel_block(obj){
	var blockmarginLeft;
	if (window.innerWidth)
		winWidth = window.innerWidth;
	else if ((document.body) && (document.body.clientWidth))
		winWidth = document.body.clientWidth;

	if (document.documentElement  && document.documentElement.clientHeight && document.documentElement.clientWidth){
		winWidth = document.documentElement.clientWidth;
	}

	if(winWidth >1050){
		winPadding = (winWidth-1050)/2;
		winWidth = 1105;
		blockmarginLeft= (winWidth*0.2)+winPadding;
	}
	else if(winWidth <=1050){
		blockmarginLeft= (winWidth)*0.2 + document.body.scrollLeft;
	}

	document.getElementById(obj).style.marginLeft = blockmarginLeft+"px";
}

function change_wl_nmode(o){
	if(Bcmwifi_support) {
		if(o.value == '2')
			inputCtrl(document.form.wl_gmode_check, 1);
		else {
			inputCtrl(document.form.wl_gmode_check, 0);
			document.form.wl_gmode_check.checked = true;
		}
	}
	else {
		if(o.value=='1') /* Jerry5: to be verified */
			inputCtrl(document.form.wl_gmode_check, 0);
		else
			inputCtrl(document.form.wl_gmode_check, 1);
	}

	if(he_frame_support){
		if(o.value == '0'){
			if (based_modelid != 'RT-AX92U' || (wl_unit != '0' && wl_unit != '1')) {
				$("#he_mode_field").show();
			}
		}
		else if(o.value == '9'){		// AX only
			$("#he_mode_field").show();
		}
		else{
			$("#he_mode_field").hide();
		}
	}

	limit_auth_method();
	if(o.value == "3"){
		document.form.wl_wme.value = "on";
	}

	wl_chanspec_list_change();
	genBWTable(wl_unit);
	controlAXOnlyHint();
}

function genBWTable(_unit){
	cur = '<% nvram_get("wl_bw"); %>';
	var bws = new Array();
	var bwsDesc = new Array();
	if(document.form.wl_nmode_x.value == 2){
		bws = [1];
		bwsDesc = ["20 MHz"];
		if(tmo_support && _unit == 0){		// for 2.4G B/G Mixed
			inputCtrl(document.form.wl_bw,0);
		}
		else{
			inputCtrl(document.form.wl_bw,1);
		}
	}
	else if(_unit == 0){
		bws = [0, 1, 2];
		bwsDesc = ["20/40 MHz", "20 MHz", "40 MHz"];
		if(tmo_support){
			if(document.form.wl_nmode_x.value == 6 || document.form.wl_nmode_x.value == 5){		// B only or G only
				inputCtrl(document.form.wl_bw,0);				
			}
			else{
				inputCtrl(document.form.wl_bw,1);
			}
		}
	}
	else{
		if(tmo_support){
			if(document.form.wl_nmode_x.value == 7){		// A only
				inputCtrl(document.form.wl_bw,0);	
			}
			else{
				inputCtrl(document.form.wl_bw,1);
			
				if(document.form.wl_nmode_x.value == 0 || document.form.wl_nmode_x.value == 3){			// Auto or AC only
					bws = [0, 1, 2, 3];
					bwsDesc = ["20/40/80 MHz", "20 MHz", "40 MHz", "80 MHz"];		
				}
				else{			// N only or A/N Mixed
					bws = [0, 1, 2];
					bwsDesc = ["20/40 MHz", "20 MHz", "40 MHz"];			
				}
			}
		}
		else{
			if (!band5g_11ac_support){	//for RT-N66U SDK 6.x
				bws = [0, 1, 2];
				bwsDesc = ["20/40 MHz", "20 MHz", "40 MHz"];		
			}
			else if (based_modelid == "RT-AC87U"){
				if(document.form.wl_nmode_x.value == 1){
					bws = [1, 2];
					bwsDesc = ["20 MHz", "40 MHz"];
				}else{
					bws = [1, 2, 3];
					bwsDesc = ["20 MHz", "40 MHz", "80 MHz"];
				}
			}
			else if (based_modelid == "GT-AC9600"){
				if(document.form.wl_nmode_x.value == 1){
					bws = [1, 2];
					bwsDesc = ["20 MHz", "40 MHz"];
				}else{
					bws = [1, 2, 3, 5];
					bwsDesc = ["20 MHz", "40 MHz", "80 MHz", "160 MHz"];
				}
			}			
			else if((based_modelid == "DSL-AC68U" || based_modelid == "RT-AC68U" || based_modelid == "RT-AC68A" || based_modelid == "4G-AC68U" ||
				based_modelid == "RT-AC56U" || based_modelid == "RT-AC56S" || 
				based_modelid == "RT-AC66U" || 
				based_modelid == "RT-AC3200" || 
				based_modelid == "RT-AC3100" || based_modelid == "RT-AC88U" || based_modelid == "RT-AX88U" || based_modelid == "RT-AC86U" || based_modelid == "GT-AC2900" ||
				based_modelid == "RT-AC5300" || based_modelid == "GT-AC5300" || based_modelid == "GT-AX11000" || based_modelid == "RT-AX92U" || based_modelid == "RT-AX95Q" || based_modelid == "XT8PRO" || based_modelid == "XT8_V2" || based_modelid == "RT-AX56_XD4" || based_modelid == "XD4PRO" || based_modelid == "CT-AX56_XD4" || based_modelid == "RT-AX58U" || based_modelid == "RT-AX58U_V2" || based_modelid == "RT-AX3000N" || based_modelid == "TUF-AX3000" || based_modelid == "TUF-AX3000_V2" || based_modelid == "RT-AXE7800" || based_modelid == "DSL-AX82U" || based_modelid == "RT-AX82U" || based_modelid == "RT-AX82U_V2" || based_modelid == "RT-AX56U" || based_modelid == "GT-AXE11000" || based_modelid == "GS-AX3000" || based_modelid == "GS-AX5400" || based_modelid == "GT-AX6000" || based_modelid == "GT10" || based_modelid == "GT-AX11000_PRO" || based_modelid == "ET12" || based_modelid == "XT12" || based_modelid == "GT-AXE16000" ||
				based_modelid == "RT-AC53U") && document.form.wl_nmode_x.value == 1){		//N only
				bws = [0, 1, 2];
				bwsDesc = ["20/40 MHz", "20 MHz", "40 MHz"];
				
			}
			else{
				if(no_vht_support){		//Hide 11AC/80MHz from GUI
					bws = [0, 1, 2];
					bwsDesc = ["20/40 MHz", "20 MHz", "40 MHz"];
				}
				else if(band5g_11ax_support || bw_160_support){
					if(enable_bw_160){
						if( (wl_unit == 1 && wl1.channel_160m == '') ||(wl_unit == 2 && wl2.channel_160m == '')){
							bws = [0, 1, 2, 3];
							bwsDesc = ["20/40/80 MHz", "20 MHz", "40 MHz", "80 MHz"];
						}
						else{
							bws = [0, 1, 2, 3, 5];
							bwsDesc = ["20/40/80/160 MHz", "20 MHz", "40 MHz", "80 MHz", "160 MHz"];
						}	
					}
					else{
						bws = [0, 1, 2, 3];
						bwsDesc = ["20/40/80 MHz", "20 MHz", "40 MHz", "80 MHz"];
					}					
				}
				else{
					bws = [0, 1, 2, 3];
					bwsDesc = ["20/40/80 MHz", "20 MHz", "40 MHz", "80 MHz"];
				}
			}
		}		
	}

	add_options_x2(document.form.wl_bw, bwsDesc, bws, cur);
	wl_chanspec_list_change();
}

function mbss_display_ctrl(){
	// generate options
	if(multissid_support){
		for(var i=1; i<multissid_count+1; i++)
			add_options_value(document.form.wl_subunit, i, '<% nvram_get("wl_subunit"); %>');
	}	
	else
		document.getElementById("wl_subunit_field").style.display = "none";

	if(document.form.wl_subunit.value != 0){
		document.getElementById("wl_bw_field").style.display = "none";
		document.getElementById("wl_channel_field").style.display = "none";
		document.getElementById("wl_nctrlsb_field").style.display = "none";
	}
	else
		document.getElementById("wl_bss_enabled_field").style.display = "none";
}

function add_options_value(o, arr, orig){
	if(orig == arr)
		add_option(o, "mbss_"+arr, arr, 1);
	else
		add_option(o, "mbss_"+arr, arr, 0);
}

function detect_qtn_ready(){
	if(qtn_state_t != "1")
		setTimeout('detect_qtn_ready();', 1000);
	else
		document.form.submit();		
}

function applyRule(){var postObj = new Object();
	var rc_time = 10;
	var variable = new Object();
	var _smart_connect_x_ori = '<% nvram_get("smart_connect_x"); %>';

	// handle data wants to post
	postObj = {
		'action_mode': 'apply',
		'rc_service': 'restart_wireless',
		'psc6g': document.getElementById('band2_psc6g_checkbox').checked ? '1' : '0'
	}

	if(!validateInput()){
		return false;
	}

	/*var auth_mode = document.form.wl_auth_mode_x.value;
	var auth_mode_ori = '<% nvram_get("wl_auth_mode_x"); %>';*/
	
	/*if(document.form.wl_wpa_psk.value == "<#wireless_psk_fillin#>"){
		document.form.wl_wpa_psk.value = "";
	}*/


	//if(validForm()){
		//showLoading();
		
			//document.form.submit();

	//}
	if(_smart_connect_enable == '0'){
		variable = {
			"smart_connect_x": '0', 
			"wl0_ssid": document.form.band0_ssid.value,
			"wl0_bw": document.form.band0_bw.value,
			"wl1_ssid": document.form.band1_ssid.value,
			"wl1_bw": document.form.band1_bw.value,
			"wl2_ssid": document.form.band2_ssid.value,
			"wl2_bw": document.form.band2_bw.value,
			"wl1_chanspec": document.form.band1_channel.value,
			"wl2_chanspec": document.form.band2_channel.value,
			"wl0_auth_mode_x": document.form.band0_auth_mode_x.value,
			"wl1_auth_mode_x": document.form.band1_auth_mode_x.value,
			"wl2_auth_mode_x": document.form.band2_auth_mode_x.value,
			"wl1_bw_160": band1_enable_bw_160,
			"wl2_bw_160": band2_enable_bw_160,
			"wl0_closed": document.form.wl0_closed.value,
			"wl1_closed": document.form.wl1_closed.value,
			"wl2_closed": document.form.wl2_closed.value
		}

		if(document.form.band0_bw.value == '1'){
			variable['wl0_chanspec'] = document.form.band0_channel.value;
		}
		else{
			variable['wl0_chanspec'] = document.form.band0_channel.value + document.form.band0_extChannel.value;
		}

		if(document.form.band1_channel.value == '0'){
			variable['acs_dfs'] = (document.getElementById('band1_acsDFS_checkbox').checked) ? '1' : '0';
		}

		if(document.form.band0_auth_mode_x.value == 'psk2'
		|| document.form.band0_auth_mode_x.value == 'sae'
		|| document.form.band0_auth_mode_x.value == 'pskpsk2'
		|| document.form.band0_auth_mode_x.value == 'psk2sae'){
			variable['wl0_crypto'] = document.form.band0_crypto.value;
			variable['wl0_wpa_psk'] = document.form.band0_wpa_psk.value;
			variable['wl0_mfp'] = document.form.band0_mfp.value;
			variable['wl0_wpa_gtk_rekey'] = document.form.band0_wpa_gtk_rekey.value;
		}
		else if(document.form.band0_auth_mode_x.value == 'wpa2'
			 || document.form.band0_auth_mode_x.value == 'wpawpa2'){
				variable['wl0_crypto'] = document.form.band0_crypto.value;
				variable['wl0_radius_ipaddr'] = document.form.band0_radius_ipaddr.value;
				variable['wl0_radius_port'] = document.form.band0_radius_port.value;
				variable['wl0_radius_key'] = document.form.band0_radius_key.value;
				variable['wl0_mfp'] = document.form.band0_mfp.value;
				variable['wl0_wpa_gtk_rekey'] = document.form.band0_wpa_gtk_rekey.value;
		}

		if(document.form.band1_auth_mode_x.value == 'psk2'
		|| document.form.band1_auth_mode_x.value == 'sae'
		|| document.form.band1_auth_mode_x.value == 'pskpsk2'
		|| document.form.band1_auth_mode_x.value == 'psk2sae'){
			variable['wl1_crypto'] = document.form.band1_crypto.value;
			variable['wl1_wpa_psk'] = document.form.band1_wpa_psk.value;
			variable['wl1_mfp'] = document.form.band1_mfp.value;
			variable['wl1_wpa_gtk_rekey'] = document.form.band1_wpa_gtk_rekey.value;
		}
		else if(document.form.band1_auth_mode_x.value == 'wpa2'
			 || document.form.band1_auth_mode_x.value == 'wpawpa2'){
				variable['wl1_crypto'] = document.form.band1_crypto.value;
				variable['wl1_radius_ipaddr'] = document.form.band1_radius_ipaddr.value;
				variable['wl1_radius_port'] = document.form.band1_radius_port.value;
				variable['wl1_radius_key'] = document.form.band1_radius_key.value;
				variable['wl1_mfp'] = document.form.band1_mfp.value;
				variable['wl1_wpa_gtk_rekey'] = document.form.band1_wpa_gtk_rekey.value;
		}

		if(document.form.band2_auth_mode_x.value == 'sae'){
			variable['wl2_crypto'] = document.form.band2_crypto.value;
			variable['wl2_wpa_psk'] = document.form.band2_wpa_psk.value;
			variable['wl2_mfp'] = document.form.band2_mfp.value;
			variable['wl2_wpa_gtk_rekey'] = document.form.band2_wpa_gtk_rekey.value;
		}
		else if(document.form.band2_auth_mode_x.value == 'owe'){
			variable['wl2_crypto'] = document.form.band2_crypto.value;
			variable['wl2_mfp'] = document.form.band2_mfp.value;
			variable['wl2_wpa_gtk_rekey'] = document.form.band2_wpa_gtk_rekey.value;
		}
	}
	else if(_smart_connect_enable == '1'){
		if(document.form.smart_connect_t.value == '1'){
			variable = {
				"smart_connect_x": '1', 
				"wl0_ssid": document.form.band012_ssid.value,
				"wl1_ssid": document.form.band012_ssid.value,
				"wl2_ssid": document.form.band012_ssid.value,
				"wl0_bw": document.form.band0_bw.value,		
				"wl1_bw": document.form.band1_bw.value,	
				"wl2_bw": document.form.band2_bw.value,
				"wl1_chanspec": document.form.band1_channel.value,
				"wl2_chanspec": document.form.band2_channel.value,
				"wl0_auth_mode_x": document.form.band01_auth_mode_x.value,
				"wl1_auth_mode_x": document.form.band01_auth_mode_x.value,
				"wl2_auth_mode_x": document.form.band2_auth_mode_x.value,
				"wl1_bw_160": band1_enable_bw_160,
				"wl2_bw_160": band2_enable_bw_160,
				"wl0_closed": document.form.wl256_closed.value
			}

			if(document.form.band0_bw.value == '1'){
				variable['wl0_chanspec'] = document.form.band0_channel.value;
			}
			else{
				variable['wl0_chanspec'] = document.form.band0_channel.value + document.form.band0_extChannel.value;
			}

			if(document.form.band1_channel.value == '0'){
				variable['acs_dfs'] = (document.getElementById('band1_acsDFS_checkbox').checked) ? '1' : '0';
			}

			if(document.form.band01_auth_mode_x.value == 'psk2'
			|| document.form.band01_auth_mode_x.value == 'sae'
			|| document.form.band01_auth_mode_x.value == 'pskpsk2'
			|| document.form.band01_auth_mode_x.value == 'psk2sae'){
				variable['wl0_crypto'] = document.form.band01_crypto.value;
				variable['wl1_crypto'] = document.form.band01_crypto.value;
				variable['wl0_wpa_psk'] = document.form.band01_wpa_psk.value;
				variable['wl1_wpa_psk'] = document.form.band01_wpa_psk.value;
				variable['wl0_mfp'] = document.form.band01_mfp.value;
				variable['wl1_mfp'] = document.form.band01_mfp.value;
				variable['wl0_wpa_gtk_rekey'] = document.form.band01_wpa_gtk_rekey.value;			
				variable['wl1_wpa_gtk_rekey'] = document.form.band01_wpa_gtk_rekey.value;				
			}
			else if(document.form.band01_auth_mode_x.value == 'wpa2'
				|| document.form.band01_auth_mode_x.value == 'wpawpa2'){
					variable['wl0_crypto'] = document.form.band01_crypto.value;
					variable['wl1_crypto'] = document.form.band01_crypto.value;
					variable['wl0_radius_ipaddr'] = document.form.band01_radius_ipaddr.value;
					variable['wl1_radius_ipaddr'] = document.form.band01_radius_ipaddr.value;
					variable['wl0_radius_port'] = document.form.band01_radius_port.value;
					variable['wl1_radius_port'] = document.form.band01_radius_port.value;
					variable['wl0_radius_key'] = document.form.band01_radius_key.value;
					variable['wl1_radius_key'] = document.form.band01_radius_key.value;
					variable['wl0_mfp'] = document.form.band01_mfp.value;
					variable['wl1_mfp'] = document.form.band01_mfp.value;
					variable['wl0_wpa_gtk_rekey'] = document.form.band01_wpa_gtk_rekey.value;
					variable['wl1_wpa_gtk_rekey'] = document.form.band01_wpa_gtk_rekey.value;
			}

			if(document.form.band2_auth_mode_x.value == 'sae'){
				variable['wl2_crypto'] = document.form.band2_crypto.value;
				variable['wl2_wpa_psk'] = document.form.band2_wpa_psk.value;
				variable['wl2_mfp'] = document.form.band2_mfp.value;
				variable['wl2_wpa_gtk_rekey'] = document.form.band2_wpa_gtk_rekey.value;
			}		
		}
		else if(document.form.smart_connect_t.value == '3'){
			variable = {
				"smart_connect_x": '3', 
				"wl0_ssid": document.form.band01_ssid.value,
				"wl1_ssid": document.form.band01_ssid.value,
				"wl2_ssid": document.form.band2_ssid.value,
				"wl0_bw": document.form.band0_bw.value,		
				"wl1_bw": document.form.band1_bw.value,	
				"wl2_bw": document.form.band2_bw.value,
				"wl1_chanspec": document.form.band1_channel.value,
				"wl2_chanspec": document.form.band2_channel.value,
				"wl0_auth_mode_x": document.form.band01_auth_mode_x.value,
				"wl1_auth_mode_x": document.form.band01_auth_mode_x.value,
				"wl2_auth_mode_x": document.form.band2_auth_mode_x.value,
				"wl1_bw_160": band1_enable_bw_160,
				"wl2_bw_160": band2_enable_bw_160,
				"wl0_closed": document.form.wl25_closed.value,				
				"wl2_closed": document.form.wl2_closed.value				
			}

			if(document.form.band0_bw.value == '1'){
				variable['wl0_chanspec'] = document.form.band0_channel.value;
			}
			else{
				variable['wl0_chanspec'] = document.form.band0_channel.value + document.form.band0_extChannel.value;
			}

			if(document.form.band1_channel.value == '0'){
				variable['acs_dfs'] = (document.getElementById('band1_acsDFS_checkbox').checked) ? '1' : '0';
			}

			if(document.form.band01_auth_mode_x.value == 'psk2'
			|| document.form.band01_auth_mode_x.value == 'sae'
			|| document.form.band01_auth_mode_x.value == 'pskpsk2'
			|| document.form.band01_auth_mode_x.value == 'psk2sae'){
				variable['wl0_crypto'] = document.form.band01_crypto.value;
				variable['wl1_crypto'] = document.form.band01_crypto.value;
				variable['wl0_wpa_psk'] = document.form.band01_wpa_psk.value;
				variable['wl1_wpa_psk'] = document.form.band01_wpa_psk.value;
				variable['wl0_mfp'] = document.form.band01_mfp.value;
				variable['wl1_mfp'] = document.form.band01_mfp.value;
				variable['wl0_wpa_gtk_rekey'] = document.form.band01_wpa_gtk_rekey.value;			
				variable['wl1_wpa_gtk_rekey'] = document.form.band01_wpa_gtk_rekey.value;		
			}
			else if(document.form.band01_auth_mode_x.value == 'wpa2'
				|| document.form.band01_auth_mode_x.value == 'wpawpa2'){
					variable['wl0_crypto'] = document.form.band01_crypto.value;
					variable['wl1_crypto'] = document.form.band01_crypto.value;
					variable['wl0_radius_ipaddr'] = document.form.band01_radius_ipaddr.value;
					variable['wl1_radius_ipaddr'] = document.form.band01_radius_ipaddr.value;
					variable['wl0_radius_port'] = document.form.band01_radius_port.value;
					variable['wl1_radius_port'] = document.form.band01_radius_port.value;
					variable['wl0_radius_key'] = document.form.band01_radius_key.value;
					variable['wl1_radius_key'] = document.form.band01_radius_key.value;
					variable['wl0_mfp'] = document.form.band01_mfp.value;
					variable['wl1_mfp'] = document.form.band01_mfp.value;
					variable['wl0_wpa_gtk_rekey'] = document.form.band01_wpa_gtk_rekey.value;
					variable['wl1_wpa_gtk_rekey'] = document.form.band01_wpa_gtk_rekey.value;
			}

			if(document.form.band2_auth_mode_x.value == 'sae'){
				variable['wl2_crypto'] = document.form.band2_crypto.value;
				variable['wl2_wpa_psk'] = document.form.band2_wpa_psk.value;
				variable['wl2_mfp'] = document.form.band2_mfp.value;
				variable['wl2_wpa_gtk_rekey'] = document.form.band2_wpa_gtk_rekey.value;
			}
		}
	}

	if(isSupport("amas_fronthaul_network")){
		if($("#fh_ap_enabled").val() != undefined && $("#fh_ap_enabled").val() != "")
			variable['fh_ap_enabled'] = $("#fh_ap_enabled").val();
	}

	if(isSupport("triband") && dwb_info.mode == "1"){
		var ssid_same_flag = false;
		var dwb_ssid = "";
		if(_smart_connect_enable == "1" && document.form.smart_connect_t.value == "1"){
			dwb_ssid = httpApi.nvramGet(["wl" + dwb_info.band + "_ssid"])["wl" + dwb_info.band + "_ssid"];
			delete variable["wl" + dwb_info.band + "_ssid"];
			if(isSupport("amas_fronthaul_network"))
				variable["wl" + dwb_info.band + "_closed"] = "1";
		}
		else
			dwb_ssid = variable["wl" + dwb_info.band + "_ssid"];

		for(var i = 0; i < wl_nband_array.length; i++){
			if(i == dwb_info.band)
				continue;
			var current_ssid = variable["wl" + i + "_ssid"];
			if(current_ssid == dwb_ssid){
				ssid_same_flag = true;
				break;
			}
		}
		if(ssid_same_flag){
			var _str = '<#Wireless_SSID_hint1#>';
			var _temp = _str.replace('%@', wl_nband_title[dwb_info.band]);
			alert(_temp);
			return false;
		}
	}

	if(meshBackhaulAutoSupport){
		if(document.form.fh_ap_enabled.value === '0' && document.querySelector('#fh_connection_hint_checkbox').checked){
			variable['fh_ap_enabled'] = '1';
		}
	}

	if(unii4Support){
		if(document.querySelector('#acs_unii4_checkbox').checked){
			variable['acs_unii4'] = '1';
		}
		else{
			variable['acs_unii4'] = '0';
		}		
	}

	postObj = Object.assign(postObj, variable);
	httpApi.nvramSet(postObj, function(){
		showLoading(rc_time);
		setTimeout(function(){
			location.reload();
		}, rc_time*1000);
	});
}

function validateInput(){
	if(_smart_connect_enable == '0'){
		if(!validator.stringSSID(document.form.band0_ssid)){
			return false;
		}
		if(!validator.stringSSID(document.form.band1_ssid)){
			return false;
		}
		if(!validator.stringSSID(document.form.band2_ssid)){
			return false;
		}
		
		if(!check_wl_auth_support($("select[name=band0_auth_mode_x] option:selected"), 0))
			return false;
		if(!check_wl_auth_support($("select[name=band1_auth_mode_x] option:selected"), 1))
			return false;
		if(!check_wl_auth_support($("select[name=band2_auth_mode_x] option:selected"), 2))
			return false;

		if(document.form.band0_auth_mode_x.value == 'psk2'
		|| document.form.band0_auth_mode_x.value == 'sae'
		|| document.form.band0_auth_mode_x.value == 'pskpsk2'
		|| document.form.band0_auth_mode_x.value == 'psk2sae'){
			var is_common_string = check_common_string(document.form.band0_wpa_psk.value, "wpa_key");
			if(is_common_string){
				if(!confirm("<#JS_common_passwd#>")){
					document.form.band0_wpa_psk.focus();
					document.form.band0_wpa_psk.select();
					return false;	
				}
			}

			if(is_KR_sku && !validator.psk_KR(document.form.band0_wpa_psk)){
				return false;
			}
			else if(!validator.psk(document.form.band0_wpa_psk)){					
				return false;
			}
		}
		//document.form.band01_auth_mode_x.value == 'wpa2'
		if(document.form.band1_auth_mode_x.value == 'psk2'
		|| document.form.band1_auth_mode_x.value == 'sae'
		|| document.form.band1_auth_mode_x.value == 'pskpsk2'
		|| document.form.band1_auth_mode_x.value == 'psk2sae'){
			var is_common_string = check_common_string(document.form.band1_wpa_psk.value, "wpa_key");
			if(is_common_string){
				if(!confirm("<#JS_common_passwd#>")){
					document.form.band1_wpa_psk.focus();
					document.form.band1_wpa_psk.select();
					return false;	
				}
			}

			if(is_KR_sku && !validator.psk_KR(document.form.band1_wpa_psk)){
				return false;
			}
			else if(!validator.psk(document.form.band1_wpa_psk)){					
				return false;
			}
		}
		
		if(document.form.band2_auth_mode_x.value == 'psk2'
		|| document.form.band2_auth_mode_x.value == 'sae'
		|| document.form.band2_auth_mode_x.value == 'pskpsk2'
		|| document.form.band2_auth_mode_x.value == 'psk2sae'){
			var is_common_string = check_common_string(document.form.band2_wpa_psk.value, "wpa_key");
			if(is_common_string){
				if(!confirm("<#JS_common_passwd#>")){
					document.form.band2_wpa_psk.focus();
					document.form.band2_wpa_psk.select();
					return false;	
				}
			}

			if(is_KR_sku && !validator.psk_KR(document.form.band2_wpa_psk)){
				return false;
			}
			else if(!validator.psk(document.form.band2_wpa_psk)){					
				return false;
			}
		}

		if(document.form.band0_auth_mode_x.value == 'psk2'
		|| document.form.band0_auth_mode_x.value == 'sae'
		|| document.form.band0_auth_mode_x.value == 'pskpsk2'
		|| document.form.band0_auth_mode_x.value == 'psk2sae'
		|| document.form.band0_auth_mode_x.value == 'wpa2'
		|| document.form.band0_auth_mode_x.value == 'wpawpa2'){
			if(!validator.range(document.form.band0_wpa_gtk_rekey, 0, 2592000)){
				return false;
			}				
		}
		if(document.form.band1_auth_mode_x.value == 'psk2'
		|| document.form.band1_auth_mode_x.value == 'sae'
		|| document.form.band1_auth_mode_x.value == 'pskpsk2'
		|| document.form.band1_auth_mode_x.value == 'psk2sae'
		|| document.form.band1_auth_mode_x.value == 'wpa2'
		|| document.form.band1_auth_mode_x.value == 'wpawpa2'){
			if(!validator.range(document.form.band1_wpa_gtk_rekey, 0, 2592000)){
				return false;
			}				
		}
		if(document.form.band2_auth_mode_x.value == 'psk2'
		|| document.form.band2_auth_mode_x.value == 'sae'
		|| document.form.band2_auth_mode_x.value == 'pskpsk2'
		|| document.form.band2_auth_mode_x.value == 'psk2sae'
		|| document.form.band2_auth_mode_x.value == 'wpa2'
		|| document.form.band2_auth_mode_x.value == 'wpawpa2'){
			if(!validator.range(document.form.band2_wpa_gtk_rekey, 0, 2592000)){
				return false;
			}				
		}		
	}
	else{
		if(document.form.smart_connect_t.value == '1'){
			if(!validator.stringSSID(document.form.band012_ssid)){
				return false;
			}

			if(!check_wl_auth_support($("select[name=band01_auth_mode_x] option:selected"), 0))
				return false;

			if(document.form.band01_auth_mode_x.value == 'psk2'
			|| document.form.band01_auth_mode_x.value == 'sae'
			|| document.form.band01_auth_mode_x.value == 'pskpsk2'
			|| document.form.band01_auth_mode_x.value == 'psk2sae'){
				var is_common_string = check_common_string(document.form.band01_wpa_psk.value, "wpa_key");
				if(is_common_string){
					if(!confirm("<#JS_common_passwd#>")){
						document.form.band01_wpa_psk.focus();
						document.form.band01_wpa_psk.select();
						return false;	
					}
				}

				if(is_KR_sku && !validator.psk_KR(document.form.band01_wpa_psk)){
					return false;
				}
				else if(!validator.psk(document.form.band01_wpa_psk)){					
					return false;
				}
			}

			if(!check_wl_auth_support($("select[name=band2_auth_mode_x] option:selected"), 2))
				return false;

			if(document.form.band2_auth_mode_x.value == 'psk2'
			|| document.form.band2_auth_mode_x.value == 'sae'
			|| document.form.band2_auth_mode_x.value == 'pskpsk2'
			|| document.form.band2_auth_mode_x.value == 'psk2sae'){
				var is_common_string = check_common_string(document.form.band2_wpa_psk.value, "wpa_key");
				if(is_common_string){
					if(!confirm("<#JS_common_passwd#>")){
						document.form.band2_wpa_psk.focus();
						document.form.band2_wpa_psk.select();
						return false;	
					}
				}

				if(is_KR_sku && !validator.psk_KR(document.form.band2_wpa_psk)){
					return false;
				}
				else if(!validator.psk(document.form.band2_wpa_psk)){					
					return false;
				}
			}
			
			if(document.form.band01_auth_mode_x.value == 'psk2'
			|| document.form.band01_auth_mode_x.value == 'sae'
			|| document.form.band01_auth_mode_x.value == 'pskpsk2'
			|| document.form.band01_auth_mode_x.value == 'psk2sae'
			|| document.form.band01_auth_mode_x.value == 'wpa2'
			|| document.form.band01_auth_mode_x.value == 'wpawpa2'){
				if(!validator.range(document.form.band01_wpa_gtk_rekey, 0, 2592000)){
					return false;
				}				
			}
			if(document.form.band2_auth_mode_x.value == 'psk2'
			|| document.form.band2_auth_mode_x.value == 'sae'
			|| document.form.band2_auth_mode_x.value == 'pskpsk2'
			|| document.form.band2_auth_mode_x.value == 'psk2sae'
			|| document.form.band2_auth_mode_x.value == 'wpa2'
			|| document.form.band2_auth_mode_x.value == 'wpawpa2'){
				if(!validator.range(document.form.band2_wpa_gtk_rekey, 0, 2592000)){
					return false;
				}				
			}
		}
		else if(document.form.smart_connect_t.value == '3'){
			if(!validator.stringSSID(document.form.band01_ssid)){
				return false;
			}
			if(!validator.stringSSID(document.form.band2_ssid)){
				return false;
			}

			if(!check_wl_auth_support($("select[name=band01_auth_mode_x] option:selected"), 0))
				return false;

			if(document.form.band01_auth_mode_x.value == 'psk2'
			|| document.form.band01_auth_mode_x.value == 'sae'
			|| document.form.band01_auth_mode_x.value == 'pskpsk2'
			|| document.form.band01_auth_mode_x.value == 'psk2sae'){
				var is_common_string = check_common_string(document.form.band01_wpa_psk.value, "wpa_key");
				if(is_common_string){
					if(!confirm("<#JS_common_passwd#>")){
						document.form.band01_wpa_psk.focus();
						document.form.band01_wpa_psk.select();
						return false;	
					}
				}

				if(is_KR_sku && !validator.psk_KR(document.form.band01_wpa_psk)){
					return false;
				}
				else if(!validator.psk(document.form.band01_wpa_psk)){					
					return false;
				}
			}
			
			if(!check_wl_auth_support($("select[name=band2_auth_mode_x] option:selected"), 2))
				return false;

			if(document.form.band2_auth_mode_x.value == 'psk2'
			|| document.form.band2_auth_mode_x.value == 'sae'
			|| document.form.band2_auth_mode_x.value == 'pskpsk2'
			|| document.form.band2_auth_mode_x.value == 'psk2sae'){
				var is_common_string = check_common_string(document.form.band2_wpa_psk.value, "wpa_key");
				if(is_common_string){
					if(!confirm("<#JS_common_passwd#>")){
						document.form.band2_wpa_psk.focus();
						document.form.band2_wpa_psk.select();
						return false;	
					}
				}

				if(is_KR_sku && !validator.psk_KR(document.form.band2_wpa_psk)){
					return false;
				}
				else if(!validator.psk(document.form.band2_wpa_psk)){					
					return false;
				}
			}
			
			if(document.form.band01_auth_mode_x.value == 'psk2'
			|| document.form.band01_auth_mode_x.value == 'sae'
			|| document.form.band01_auth_mode_x.value == 'pskpsk2'
			|| document.form.band01_auth_mode_x.value == 'psk2sae'
			|| document.form.band01_auth_mode_x.value == 'wpa2'
			|| document.form.band01_auth_mode_x.value == 'wpawpa2'){
				if(!validator.range(document.form.band01_wpa_gtk_rekey, 0, 2592000)){
					return false;
				}				
			}
			if(document.form.band2_auth_mode_x.value == 'psk2'
			|| document.form.band2_auth_mode_x.value == 'sae'
			|| document.form.band2_auth_mode_x.value == 'pskpsk2'
			|| document.form.band2_auth_mode_x.value == 'psk2sae'
			|| document.form.band2_auth_mode_x.value == 'wpa2'
			|| document.form.band2_auth_mode_x.value == 'wpawpa2'){
				if(!validator.range(document.form.band2_wpa_gtk_rekey, 0, 2592000)){
					return false;
				}				
			}
		}
	}

	return true;
}

function validForm(){
	var auth_mode = document.form.wl_auth_mode_x.value;
	
	if(!validator.stringSSID(document.form.wl_ssid))
		return false;
	
	if(!check_NOnly_to_GN()){
		autoFocus('wl_nmode_x');
		return false;
	}
	
	if(document.form.wl_wep_x.value != "0")
		if(!validate_wlphrase('WLANConfig11b', 'wl_phrase_x', document.form.wl_phrase_x))
			return false;	
	if(auth_mode == "psk" || auth_mode == "psk2" || auth_mode == "pskpsk2" || auth_mode == "sae" || auth_mode == "psk2sae"){ //2008.08.04 lock modified
		if(is_KR_sku){
			if(!validator.psk_KR(document.form.wl_wpa_psk))
				return false;
		}
		else{
			if(!validator.psk(document.form.wl_wpa_psk))
				return false;
		}
		
		//confirm common string combination	#JS_common_passwd#
		var is_common_string = check_common_string(document.form.wl_wpa_psk.value, "wpa_key");
		if(is_common_string){
			if(!confirm("<#JS_common_passwd#>")){
				document.form.wl_wpa_psk.focus();
				document.form.wl_wpa_psk.select();
				return false;	
			}	
		}
		
		if(!validator.range(document.form.wl_wpa_gtk_rekey, 0, 2592000))
			return false;
	}
	else if(auth_mode == "wpa" || auth_mode == "wpa2" || auth_mode == "wpawpa2"){
		if(!validator.range(document.form.wl_wpa_gtk_rekey, 0, 2592000))
			return false;
	}
	else{
		var cur_wep_key = eval('document.form.wl_key'+document.form.wl_key.value);		
		if(auth_mode != "radius" && !validator.wlKey(cur_wep_key))
			return false;
	}	

	if(isSupport("triband") && dwb_info.mode) {
		var jsonPara = {};
		jsonPara["edit_wl_unit"] = wl_unit;
		jsonPara["edit_wl_ssid"] = document.form.wl_ssid.value;
		jsonPara["dwb_unit"] = dwb_info.band;
		jsonPara["smart_connect"] = document.form.smart_connect_x.value;
		var ssid_array = [];
		ssid_array.push(httpApi.nvramGet(["wl0_ssid"]).wl0_ssid);
		if(wl_info.band5g_support)
			ssid_array.push(httpApi.nvramGet(["wl1_ssid"]).wl1_ssid);
		if(wl_info.band5g_2_support || wl_info.band6g_support)
			ssid_array.push(httpApi.nvramGet(["wl2_ssid"]).wl2_ssid);
		jsonPara["current_ssid"] = ssid_array;
		if(!validator.dwb_check_wl_setting(jsonPara)) {
			alert("The fronthaul SSID is the same as the backhaul SSID.");/* untranslated */
			return false;
		}
	}

	return true;
}

function done_validating(action){
	refreshpage();
}

function validate_wlphrase(s, v, obj){
	if(!validator.string(obj)){
		is_wlphrase(s, v, obj);
		return false;
	}
	return true;
}

function disableAdvFn(){
	for(var i=18; i>=3; i--)
		document.getElementById("WLgeneral").deleteRow(i);
}

function _change_wl_unit(val){
	if(sw_mode == 2 || sw_mode == 4){
		if(concurrent_pap)
			document.form.wl_subunit.value = 1;
		else
			document.form.wl_subunit.value = (val == '<% nvram_get("wlc_band"); %>') ? 1 : -1;
	}

	if(smart_connect_support && (isSwMode("rt") || isSwMode("ap")))
		document.form.current_page.value = "Advanced_Wireless_Content.asp?flag=" + document.form.smart_connect_x.value;

	change_wl_unit();
}

function _change_smart_connect(val){
	current_band = wl_unit;
	document.getElementById("wl_unit_field").style.display = "";
	var band_desc = new Array();
	var band_value = new Array();
	if(val == 0){
		band_value = [0, 1, 2];
		if(band6g_support){
			band_desc = ['2.4 GHz', '5 GHz', '6 GHz'];
		}else{
			band_desc = ['2.4 GHz', '5 GHz-1', '5 GHz-2'];
		}		
	}else if(val == 1){
		if(dwb_info.mode) {
			band_value = [0, 2];
			if(band6g_support){
				band_desc = ['2.4 GHz, 5 GHz', '6 GHz'];
			}else{
				band_desc = ['2.4 GHz, 5 GHz-1', '5 GHz-2'];
			}				
		}
		else {
			document.getElementById("wl_unit_field").style.display = "none";
			band_value = [0];
			if(band6g_support){
				band_desc = ['2.4 GHz, 5 GHz and 6 GHz'];
			}else{
				band_desc = ['2.4 GHz, 5 GHz-1 and 5 GHz-2'];
			}
		}
	}else if(val == 2){
		band_value = [0, 1];
		if(band6g_support){
			band_desc = ['2.4 GHz', '5 GHz and 6 GHz'];
		}else{
			band_desc = ['2.4 GHz', '5 GHz-1 and 5 GHz-2'];
		}	
	}
	add_options_x2(document.form.wl_unit, band_desc, band_value, current_band);
}

function checkBW(){
	if(wifilogo_support)
		return false;

	if(document.form.wl_channel.value != 0 && document.form.wl_bw.value == 0){	//Auto but set specific channel
		if(document.form.wl_channel.value == "165")	// channel 165 only for 20MHz
			document.form.wl_bw.selectedIndex = 1;
		else if(wl_unit == 0)	//2.4GHz for 40MHz
			document.form.wl_bw.selectedIndex = 2;
		else{	//5GHz else for 80MHz
			if(band5g_11ac_support)
				document.form.wl_bw.selectedIndex = 3;
			else
				document.form.wl_bw.selectedIndex = 2;
				
			if (wl_channel_list_5g.getIndexByValue("165") >= 0 ) // rm option 165 if not Auto
				document.form.wl_channel.remove(wl_channel_list_5g.getIndexByValue("165"));			
		}
	}
}

function check_NOnly_to_GN(){
	//var gn_array_2g = [["1", "ASUS_Guest1", "psk", "tkip", "1234567890", "0", "1", "", "", "", "", "0", "off", "0"], ["1", "ASUS_Guest2", "shared", "aes", "", "1", "1", "55555", "", "", "", "0", "off", "0"], ["1", "ASUS_Guest3", "open", "aes", "", "2", "4", "", "", "", "1234567890123", "0", "off", "0"]];
	//                    Y/N        mssid      auth    asyn    wpa_psk wl_wep_x wl_key k1	k2 k3 k4                                        
	//var gn_array_5g = [["1", "ASUS_5G_Guest1", "open", "aes", "", "0", "1", "", "", "", "", "0", "off", "0"], ["0", "ASUS_5G_Guest2", "open", "aes", "", "0", "1", "", "", "", "", "0", "off", ""], ["0", "ASUS_5G_Guest3", "open", "aes", "", "0", "1", "", "", "", "", "0", "off", ""]];
	// Viz add 2012.11.05 restriction for 'N Only' mode  ( start 	
	
	if(document.form.wl_nmode_x.value == "0" || document.form.wl_nmode_x.value == "1"){
		if(wl_unit == "1"){		//5G
			for(var i=0;i<gn_array_5g.length;i++){
				if(gn_array_5g[i][0] == "1" && (gn_array_5g[i][3] == "tkip" || gn_array_5g[i][5] == "1" || gn_array_5g[i][5] == "2")){
					if(document.form.wl_nmode_x.value == "0")
						document.getElementById('wl_NOnly_note').innerHTML = '<br>* <#WLANConfig11n_Auto_note#>';
					else{
						if(band5g_11ac_support)
							document.getElementById('wl_NOnly_note').innerHTML = '<br>* <#WLANConfig11n_NAC_note#>';
						else
							document.getElementById('wl_NOnly_note').innerHTML = '<br>* <#WLANConfig11n_NOnly_note#>';
					}	
						
					document.getElementById('wl_NOnly_note').style.display = "";
					return false;
				}
			}		
		}
		else if(wl_unit == "0"){		//2.4G
			for(var i=0;i<gn_array_2g.length;i++){
				if(gn_array_2g[i][0] == "1" && (gn_array_2g[i][3] == "tkip" || gn_array_2g[i][5] == "1" || gn_array_2g[i][5] == "2")){
					if(document.form.wl_nmode_x.value == "0")
						document.getElementById('wl_NOnly_note').innerHTML = '<br>* <#WLANConfig11n_Auto_note#>';
					else	
						document.getElementById('wl_NOnly_note').innerHTML = '<br>* <#WLANConfig11n_NOnly_note#>';
						
					document.getElementById('wl_NOnly_note').style.display = "";
					return false;
				}
			}	
		}	
	}
		
	document.getElementById('wl_NOnly_note').style.display = "none";
	return true;
//  Viz add 2012.11.05 restriction for 'N Only' mode  ) end		
}

function regen_5G_mode(obj,flag){	//please sync to initial() : //Change wireless mode help desc
	free_options(obj);
	if(flag == 1 || flag == 2){
		if(based_modelid == "RT-AC87U"){
			obj.options[0] = new Option("<#Auto#>", 0);
			obj.options[1] = new Option("N only", 1);			
		}
		else if(no_vht_support){	//Hide 11AC/80MHz from GUI
			obj.options[0] = new Option("<#Auto#>", 0);
			obj.options[1] = new Option("N only", 1);
			obj.options[2] = new Option("Legacy", 2);
		}
		else if(band5g_11ax_support){
			obj.options[0] = new Option("<#Auto#>", 0);
			if(based_modelid == "RT-AX92U" && flag == 1){
				obj.options[1] = new Option("N/AC mixed", 8);
				obj.options[2] = new Option("Legacy", 2);
			}
			else{
				obj.options[1] = new Option("AX only", 9);
				obj.options[2] = new Option("N/AC/AX mixed", 8);
				obj.options[3] = new Option("Legacy", 2);
			}			
		}
		else{
			obj.options[0] = new Option("<#Auto#>", 0);
			obj.options[1] = new Option("N/AC mixed", 8);
			obj.options[2] = new Option("Legacy", 2);
		}
	}
	else{
		obj.options[0] = new Option("<#Auto#>", 0);
		obj.options[1] = new Option("N only", 1);
		obj.options[2] = new Option("Legacy", 2);
	}
	
	obj.value = '<% nvram_get("wl_nmode_x"); %>';
}

function check_DFS_support(obj){
	if(obj.checked)
		document.form.acs_dfs.value = 1;
	else
		document.form.acs_dfs.value = 0;
}

function check_acs_band1_support(obj){
	if(obj.checked)
		document.form.acs_band1.value = 1;
	else
		document.form.acs_band1.value = 0;
}

function check_acs_band3_support(obj){
	if(obj.checked)
		document.form.acs_band3.value = 1;
	else
		document.form.acs_band3.value = 0;
}

function check_acs_ch13_support(obj){
	if(obj.checked)
		document.form.acs_ch13.value = 1;
	else
		document.form.acs_ch13.value = 0;
}

function tmo_wl_nmode(){
	var tmo2nmode = [["0",  "<#Auto#>"],["6",       "B Only"],["5", "G Only"],["1", "N Only"],["2",	"B/G Mixed"],["4", "G/N Mixed"]];
	var tmo5nmode = [["0",  "<#Auto#>"],["7",       "A Only"],["1", "N Only"],["3", "AC Only"],["4", "A/N Mixed"]];
	free_options(document.form.wl_nmode_x);
	if(wl_unit == "0"){               //2.4GHz
		for(var i = 0; i < tmo2nmode.length; i++){
			add_option(document.form.wl_nmode_x,tmo2nmode[i][1], tmo2nmode[i][0],(document.form.wl_nmode_x_orig.value == tmo2nmode[i][0]));
		}
	}
	else{           //5GHz
		for(var i = 0; i < tmo5nmode.length; i++){
			add_option(document.form.wl_nmode_x,tmo5nmode[i][1], tmo5nmode[i][0],(document.form.wl_nmode_x_orig.value == tmo5nmode[i][0]));
		}
	}
}

function enableSmartCon(val){
	document.form.smart_connect_x.value = val;
	var value = new Array();
	var desc = new Array();

	if(isSupport("triband") && dwb_info.mode) {
		desc = ["<#smart_connect_dual#> (2.4 GHz and 5 GHz)"];
		value = ["1"];
	}
	else {
		if(wl_info.band2g_support && wl_info.band5g_support && (wl_info.band5g_2_support || wl_info.band6g_support)){
			if(band6g_support){
				desc = ["<#smart_connect_tri#> (2.4 GHz, 5 GHz and 6 GHz)"];
				value = ["1"];
			}
			else{
				desc = ["<#smart_connect_tri#> (2.4 GHz, 5 GHz-1 and 5 GHz-2)", "5 GHz Smart Connect (5 GHz-1 and 5 GHz-2)"];
				value = ["1", "2"];
			}	
		}
		else if(wl_info.band2g_support && wl_info.band5g_support){
			desc = ["<#smart_connect_dual#> (2.4 GHz and 5 GHz)"];
			value = ["1"];
		}
	}

	add_options_x2(document.form.smart_connect_t, desc, value, val);
	$("#he_mode_field").hide();
	
	if(val == 0){
		document.getElementById("smart_connect_field").style.display = "none";
		document.getElementById("smartcon_rule_link").style.display = "none";

		if(wl_unit != 0){
			if(wl_info[wl_unit].bw_160_support){
				$("#enable_160_field").show();
				if((based_modelid == 'GT-AX11000' || based_modelid == 'RT-AX92U' || based_modelid == 'RT-AX95Q' || based_modelid == 'XT8PRO' || based_modelid == 'XT8_V2' || based_modelid == 'GT-AXE11000' || based_modelid == "GT-AX11000_PRO" || based_modelid == "ET12" || based_modelid == "XT12" || based_modelid == "GT-AXE16000") && wl2.channel_160m == '' && wl_unit == '2'){
					$("#enable_160_field").hide();
				}
			}

			if(wl_info[wl_unit].dfs_support){
				$("#dfs_checkbox").show();
			}
			else{
				$("#dfs_checkbox").hide();
			}	

			$('#acs_ch13_checkbox').hide();
		}
		else {
			if (!Qcawifi_support && !Rawifi_support) {
				if (document.form.wl_channel.value == '0' && wl_unit == '0' && document.form.wl_channel.length == '14'){
					$('#acs_ch13_checkbox').show();
				}
			}

			$("#dfs_checkbox").hide();
		}

		if(he_frame_support){
			if(based_modelid != 'RT-AX92U' || (wl_unit != '0' && wl_unit != '1')){
				$("#he_mode_field").show();
			}
		}

		$("#band_separate").hide();
		inputCtrl(document.form.wl_bw, 1);
		inputCtrl(document.form.wl_channel, 1);
		inputCtrl(document.form.wl_nctrlsb, 1);
		document.form.wl0_bw.disabled = true;
		document.form.wl1_bw.disabled = true;
		document.form.wl2_bw.disabled = true;
		document.form.wl0_chanspec.disabled = true;
		document.form.wl1_chanspec.disabled = true;
		document.form.wl2_chanspec.disabled = true;
		document.form.wl1_bw_160.disabled = true;
		document.form.wl2_bw_160.disabled = true;		

	}else if(val > 0){
		document.getElementById("smart_connect_field").style.display = "";
		document.getElementById("smartcon_rule_link").style.display = "table-cell";
		$("#enable_160_field").hide();
		if ((wl_unit == '0' && val == '2') || based_modelid == "RT-AC3200" || ((based_modelid == "RT-AX92U" || based_modelid == "RT-AX95Q" || based_modelid == "XT8PRO" || based_modelid == "XT8_V2") && country != "EU" && wl_unit == "1")) {
			$("#dfs_checkbox").hide();
		}
		else {
			$("#dfs_checkbox").show();
		}

		if((wl_unit == '0' && val == '2')){
			if (!Qcawifi_support && !Rawifi_support) {
				if (document.form.wl_channel.value == '0' && wl_unit == '0' && document.form.wl_channel.length == '14'){
					$('#acs_ch13_checkbox').show();
				}
			}
		}
		
		if(dwb_info.mode && wl_unit == dwb_info.band && wl_unit != 0 && bw_160_support) {
			$("#enable_160_field").show();
			if((based_modelid == 'GT-AX11000' || based_modelid == 'RT-AX92U' || based_modelid == 'RT-AX95Q' || based_modelid == 'XT8PRO' || based_modelid == 'XT8_V2' || based_modelid == 'GT-AXE11000' || based_modelid == "GT-AX11000_PRO" || based_modelid == "ET12" || based_modelid == "XT12" || based_modelid == "GT-AXE16000") && wl2.channel_160m == '' && wl_unit == '2'){
				$("#enable_160_field").hide();
			}
		}

		/*Separate Wireless Settings*/
		$("#band_separate").show();
		if(val == '1'){
			separateGenBWTable('0');
			separateGenBWTable('1');
			band0_channel = '<% nvram_get("wl0_chanspec"); %>';
			band1_channel = '<% nvram_get("wl1_chanspec"); %>';

			if(wl_info['1'].bw_160_support){
				$('#band1_160_field').show();
			}

			if(he_frame_support){
				if(based_modelid != 'RT-AX92U' || (wl_unit != '0' && wl_unit != '1')){
					$("#he_mode_field").show();
				}
				else if(based_modelid == 'RT-AX92U' && !dwb_info.mode){
					$("#he_mode_field").show();
				}
			}

			if (band0_channel == '0') {
				$('#band0_autoChannel').show();
				$('#band0_autoChannel').html('Current Control Channel: ' + cur_control_channel[0]);
			}

			if (band1_channel == '0') {
				$('#band1_autoChannel').show();
				$('#band1_autoChannel').html('Current Control Channel: ' + cur_control_channel[1]);
			}

			if (wl_info.band5g_2_support || wl_info.band6g_support) {
				band2_channel = '<% nvram_get("wl2_chanspec"); %>';
				separateGenBWTable('2');
				if(wl_info['2'].bw_160_support && !wl_info.band6g_support){
					$('#band2_160_field').show();
				}

				if (band2_channel == '0') {
					$('#band2_autoChannel').show();
					$('#band2_autoChannel').html('Current Control Channel: ' + cur_control_channel[2]);
				}
			}


			$('#band0_title_field').show();
			$('#band0_bandwidth_field').show();
			$('#band0_channel_field').show();
			$('#band0_extChannel_field').show();
			inputCtrl(document.form.wl_bw, 0);
			inputCtrl(document.form.wl_channel, 0);
			inputCtrl(document.form.wl_nctrlsb, 0);
			document.form.wl0_bw.disabled = false;
			document.form.wl1_bw.disabled = false;
			document.form.wl2_bw.disabled = false;
			document.form.wl0_chanspec.disabled = false;
			document.form.wl1_chanspec.disabled = false;
			document.form.wl2_chanspec.disabled = false;
			document.form.wl1_bw_160.disabled = false;
			document.form.wl2_bw_160.disabled = false;
			if(wl_info.band5g_2_support || wl_info.band6g_support){
				if(band6g_support){
					$('#5ghz_title').html('5 GHz');
				}
				else{
					$('#5ghz_title').html('5 GHz-1');
				}

				if(dwb_info.mode){
					$('#band2_title_field').hide();
					$('#band2_bandwidth_field').hide();
					$('#band2_channel_field').hide();
					$('#band2_extChannel_field').hide();
					if(wl_unit == '2'){
						inputCtrl(document.form.wl_bw, 1);
						inputCtrl(document.form.wl_channel, 1);
						inputCtrl(document.form.wl_nctrlsb, 1);
						document.form.wl0_bw.disabled = true;
						document.form.wl1_bw.disabled = true;
						document.form.wl2_bw.disabled = true;
						document.form.wl0_chanspec.disabled = true;
						document.form.wl1_chanspec.disabled = true;
						document.form.wl2_chanspec.disabled = true;
						document.form.wl1_bw_160.disabled = true;
						document.form.wl2_bw_160.disabled = true;
						$("#band_separate").hide();
					}
				}
				else{
					if(band6g_support){
						$('#5g2_title').html('6 GHz');
					}

					$('#band2_title_field').show();
					$('#band2_bandwidth_field').show();
					$('#band2_channel_field').show();
					$('#band2_extChannel_field').show();
				}
			}
		}
		else if(val == '2'){
			if(wl_unit == '0'){
				inputCtrl(document.form.wl_bw, 1);
				inputCtrl(document.form.wl_channel, 1);
				inputCtrl(document.form.wl_nctrlsb, 1);
				document.form.wl0_bw.disabled = true;
				document.form.wl1_bw.disabled = true;
				document.form.wl2_bw.disabled = true;
				document.form.wl0_chanspec.disabled = true;
				document.form.wl1_chanspec.disabled = true;
				document.form.wl2_chanspec.disabled = true;
				document.form.wl1_bw_160.disabled = true;
				document.form.wl2_bw_160.disabled = true;
				$("#band_separate").hide();
				if(he_frame_support){
					if(based_modelid != 'RT-AX92U' || (wl_unit != '0' && wl_unit != '1')){
						$("#he_mode_field").show();
					}
				}
			}
			else{
				separateGenBWTable('1');
				band1_channel = '<% nvram_get("wl1_chanspec"); %>';
				if (wl_info['1'].bw_160_support) {
					$('#band1_160_field').show();
				}

				if (band1_channel == '0') {
					$('#band1_autoChannel').show();
					$('#band1_autoChannel').html('Current Control Channel: ' + cur_control_channel[1]);
				}

				if (wl_info.band5g_2_support || wl_info.band6g_support) {
					band2_channel = '<% nvram_get("wl2_chanspec"); %>';
					separateGenBWTable('2');
					if (wl_info['2'].bw_160_support && !wl_info.band6g_support) {
						$('#band2_160_field').show();
					}

					if (band2_channel == '0') {
						$('#band2_autoChannel').show();
						$('#band2_autoChannel').html('Current Control Channel: ' + cur_control_channel[2]);
					}
				}

				if(he_frame_support){
					$("#he_mode_field").show();
				}

				inputCtrl(document.form.wl_bw, 0);
				inputCtrl(document.form.wl_channel, 0);
				inputCtrl(document.form.wl_nctrlsb, 0);
				document.form.wl0_bw.disabled = true;
				document.form.wl1_bw.disabled = false;
				document.form.wl2_bw.disabled = false;
				document.form.wl0_chanspec.disabled = true;
				document.form.wl1_chanspec.disabled = false;
				document.form.wl2_chanspec.disabled = false;
				document.form.wl1_bw_160.disabled = false;
				document.form.wl2_bw_160.disabled = false;
				$("#band_separate").show();
				$('#band0_title_field').hide();
				$('#band0_bandwidth_field').hide();
				$('#band0_channel_field').hide();
				$('#band0_extChannel_field').hide();
				$('#band1_title_field').show();
				$('#band1_bandwidth_field').show();
				$('#band1_channel_field').show();
				$('#band1_extChannel_field').show();
				if(band6g_support){
					$('#5g2_title').html('6 GHz');
				}

				$('#band2_title_field').show();
				$('#band2_bandwidth_field').show();
				$('#band2_channel_field').show();
				$('#band2_extChannel_field').show();
			}
		}

		$("#dfs_checkbox").hide();
		document.form.acs_dfs.value = 0;
	}

	if((val == 0 || (val == 2 && wl_unit == 0)) || (dwb_info.mode && wl_unit == dwb_info.band)){
		document.getElementById("wl_unit_field").style.display = "";
		document.form.wl_nmode_x.disabled = "";
		if(document.form.wl_unit[0].selected == true){
			document.getElementById("wl_gmode_checkbox").style.display = "";
		}
		if(band5g_11ac_support){
			regen_5G_mode(document.form.wl_nmode_x, wl_unit)		
		}else{
			free_options(document.form.wl_nmode_x);
			document.form.wl_nmode_x.options[0] = new Option("<#Auto#>", 0);
			document.form.wl_nmode_x.options[1] = new Option("N only", 1);
			document.form.wl_nmode_x.options[2] = new Option("Legacy", 2);
		}
		change_wl_nmode(document.form.wl_nmode_x);		
	}else{
		document.getElementById("wl_unit_field").style.display = "none";
		regen_auto_option(document.form.wl_nmode_x);
		document.getElementById("wl_gmode_checkbox").style.display = "none";
		regen_auto_option(document.form.wl_bw);
		regen_auto_option(document.form.wl_channel);
		regen_auto_option(document.form.wl_nctrlsb);			
	}
	
	if(wl_info.band2g_support && wl_info.band5g_support && (wl_info.band5g_2_support || wl_info.band6g_support))
		_change_smart_connect(val);

	controlHideSSIDHint();
}

function regen_auto_option(obj){
	free_options(obj);
	obj.options[0] = new Option("<#Auto#>", 0);
	obj.selectedIndex = 0;
}

function enable_160MHz(obj){
	cur = '<% nvram_get("wl_bw"); %>';
	var bws = new Array();
	var bwsDesc = new Array();

	if(obj.checked){
		bws = [0, 1, 2, 3, 5];
		bwsDesc = ["20/40/80/160 MHz", "20 MHz", "40 MHz", "80 MHz", "160 MHz"];
		enable_bw_160 = true;
		document.form.acs_dfs_checkbox.checked = true;
		check_DFS_support(document.form.acs_dfs_checkbox);
	}
	else{
		bws = [0, 1, 2, 3];
		bwsDesc = ["20/40/80 MHz", "20 MHz", "40 MHz", "80 MHz"];
		enable_bw_160 = false;
	}

	add_options_x2(document.form.wl_bw, bwsDesc, bws, cur);
	wl_chanspec_list_change();
	change_channel(document.form.wl_channel);
}

function he_frame_mode(obj) {
	if (obj.value == '0' && wl_unit != 0) {
		$("#enable_160mhz")[0].checked = false
		enable_160MHz($("#enable_160mhz")[0]);
		document.form.acs_dfs_checkbox.checked = false;
		document.form.acs_dfs.value = 0;		
	}
}

var band1_enable_bw_160 = '<% nvram_get("wl1_bw_160"); %>';
var band2_enable_bw_160 = '<% nvram_get("wl2_bw_160"); %>';
function separateGenBWTable(unit){
	var bws = new Array();
	var bwsDesc = new Array();
	var curChannel = '0';
	var curBandwidth = '0';
	if(unit == '0'){
		bws = [0, 1, 2];
		bwsDesc = ["20/40 MHz", "20 MHz", "40 MHz"];
		curBandwidth = '<% nvram_get("wl0_bw"); %>';
		curChannel = '<% nvram_get("wl0_chanspec"); %>';
		add_options_x2(document.form.band0_bw, bwsDesc, bws, curBandwidth);	
	}
	else if(unit == '1'){
		curBandwidth = '<% nvram_get("wl1_bw"); %>';
		curChannel = '<% nvram_get("wl1_chanspec"); %>';
		if (band5g_11ax_support) {
			if (band1_enable_bw_160 == '1') {
				if (wl1.channel_160m == '') {
					bws = [0, 1, 2, 3];
					bwsDesc = ["20/40/80 MHz", "20 MHz", "40 MHz", "80 MHz"];
				}
				else {
					bws = [0, 1, 2, 3, 5];
					bwsDesc = ["20/40/80/160 MHz", "20 MHz", "40 MHz", "80 MHz", "160 MHz"];
				}
			}
			else {
				bws = [0, 1, 2, 3];
				bwsDesc = ["20/40/80 MHz", "20 MHz", "40 MHz", "80 MHz"];
			}
		}
		else {
			bws = [0, 1, 2, 3];
			bwsDesc = ["20/40/80 MHz", "20 MHz", "40 MHz", "80 MHz"];
		}

		add_options_x2(document.form.band1_bw, bwsDesc, bws, curBandwidth);
	}
	else if(unit == '2'){
		curBandwidth = '<% nvram_get("wl2_bw"); %>';
		curChannel = '<% nvram_get("wl2_chanspec"); %>';
		if (band5g_11ax_support) {
			if (band2_enable_bw_160 == '1') {
				if (wl2.channel_160m == '') {
					bws = [0, 1, 2, 3];
					bwsDesc = ["20/40/80 MHz", "20 MHz", "40 MHz", "80 MHz"];
				}
				else {
					bws = [0, 1, 2, 3, 5];
					bwsDesc = ["20/40/80/160 MHz", "20 MHz", "40 MHz", "80 MHz", "160 MHz"];
				}
			}
			else {
				bws = [0, 1, 2, 3];
				bwsDesc = ["20/40/80 MHz", "20 MHz", "40 MHz", "80 MHz"];
			}
		}
		else {
			bws = [0, 1, 2, 3];
			bwsDesc = ["20/40/80 MHz", "20 MHz", "40 MHz", "80 MHz"];
		}

		add_options_x2(document.form.band2_bw, bwsDesc, bws, curBandwidth);
	}

	separateGenChannel(unit, curChannel, curBandwidth);
}
function separateEnable_160MHz(obj){
	if(obj.id == 'band1_160'){
		band1_enable_bw_160 = obj.checked ? '1' : '0';
		separateGenBWTable('1');
	}
	else if(obj.id == 'band2_160'){
		band2_enable_bw_160 = obj.checked ? '1' : '0';
		separateGenBWTable('2');
	}
}

function separateGenChannel(unit, channel, bandwidth){
	var channel_2g = JSON.parse('<% channel_list_2g(); %>');
	var channel_5g_1 = JSON.parse('<% channel_list_5g(); %>');
	var channel_5g_2 = JSON.parse('<% channel_list_5g_2(); %>');
	var channel_2g_val = JSON.parse('<% channel_list_2g(); %>');
	var channel_5g_1_val = new Array;
	var channel_5g_2_val = new Array;
	//var band0_curChannel = '<% nvram_get("wl0_chanspec"); %>';
	var band1_curChannel = '<% nvram_get("wl1_chanspec"); %>';
	var band2_curChannel = '<% nvram_get("wl2_chanspec"); %>';
	var curChannel = channel;
	var curBandwidth = bandwidth;
	if(unit == '0'){
		var extend_channel = new Array;
		var extend_channel_value = new Array;
		var band0_curCtrlChannel = 0;
		var band0_curExtChannel = 0;
		if (curChannel.search('[ul]') != -1) {
			band0_curExtChannel = curChannel.slice(-1);			//current control channel
			band0_curCtrlChannel = curChannel.split(band0_curExtChannel)[0];	//current extension channel direction
		}
		else {
			band0_curCtrlChannel = curChannel;
		}

		if(channel_2g.length == '11'){
			$('#band0_acs_ch13').hide();
			if(band0_curCtrlChannel == 0){
				extend_channel = ["<#Auto#>"];
				extend_channel_value = [""];
			}
			else if(band0_curCtrlChannel >= 1 && band0_curCtrlChannel <= 4){
				extend_channel = ["<#WLANConfig11b_EChannelAbove#>"];
				extend_channel_value = ["l"];
			}
			else if (band0_curCtrlChannel >= 5 && band0_curCtrlChannel <= 7) {
				extend_channel = ["<#WLANConfig11b_EChannelAbove#>", "<#WLANConfig11b_EChannelBelow#>"];
				extend_channel_value = ["l", "u"];
			}
			else if (band0_curCtrlChannel >= 8 && band0_curCtrlChannel <= 11) {
				extend_channel = ["<#WLANConfig11b_EChannelBelow#>"];
				extend_channel_value = ["u"];
			}
		}
		else if(channel_2g.length == '13'){
			$('#band0_acs_ch13').hide();
			if (band0_curCtrlChannel == 0) {
				extend_channel = ["<#Auto#>"];
				extend_channel_value = [""];
				$('#band0_acs_ch13').show();
			}
			else if (band0_curCtrlChannel >= 1 && band0_curCtrlChannel <= 4) {
				extend_channel = ["<#WLANConfig11b_EChannelAbove#>"];
				extend_channel_value = ["l"];
			}
			else if (band0_curCtrlChannel >= 5 && band0_curCtrlChannel <= 9) {
				extend_channel = ["<#WLANConfig11b_EChannelAbove#>", "<#WLANConfig11b_EChannelBelow#>"];
				extend_channel_value = ["l", "u"];
			}
			else if (band0_curCtrlChannel >= 10 && band0_curCtrlChannel <= 13) {
				extend_channel = ["<#WLANConfig11b_EChannelBelow#>"];
				extend_channel_value = ["u"];
			}
		}

		add_options_x2(document.form.band0_extChannel, extend_channel, extend_channel_value, band0_curExtChannel);
		if (curBandwidth == '0' || curBandwidth == '2') {
			$('#band0_extChannel_field').show();
		}
		else if (curBandwidth == '1') {
			$('#band0_extChannel_field').hide();
		}

		channel_2g.unshift('<#Auto#>');
		channel_2g_val.unshift('0');
		add_options_x2(document.form.band0_channel, channel_2g, channel_2g_val, band0_curCtrlChannel);		
	}
	else if(unit == '1'){
		if(curBandwidth == '0'){
			$('#band1_extChannel_field').show();
			if(amesh_support && httpApi.hasAiMeshNode() && !wl_info.band5g_2_support){
				var _wl_channel = new Array();
				channel_5g_1 = [];
				for(j=1; j<mesh_5g.auto.chanspec.length; j++){
					channel_5g_1.push(mesh_5g.auto.chanlist[j]);
					channel_5g_1_val.push(mesh_5g.auto.chanspec[j]);
				}
			}
			else{
				loop_auto: for (i = 0; i < channel_5g_1.length; i++) {
					var _cur_channel = parseInt(channel_5g_1[i]);
					
					if (band1_enable_bw_160 == '1') {
						for (j = 0; j < wl1.channel_160m.length; j++) {
							if (wl1.channel_160m[j].indexOf(_cur_channel) != -1) {
								channel_5g_1_val[i] = _cur_channel + "/160";
								continue loop_auto;
							}
						}
					}

					for (j = 0; j < wl1.channel_80m.length; j++) {
						if (wl1.channel_80m[j].indexOf(_cur_channel) != -1) {
							channel_5g_1_val[i] = _cur_channel + "/80";
							continue loop_auto;
						}
					}

					for (j = 0; j < wl1.channel_40m.length; j++) {
						if (wl1.channel_40m[j].indexOf(_cur_channel) != -1) {
							channel_5g_1_val[i] = wlextchannel_fourty(_cur_channel);
							continue loop_auto;
						}
					}

					for (j = 0; j < wl1.channel_20m.length; j++) {
						if (wl1.channel_20m[j].indexOf(_cur_channel) != -1) {
							channel_5g_1_val[i] = _cur_channel.toString();
							continue loop_auto;
						}
					}
				}
			}			
		}
		else if(curBandwidth == '5'){
			$('#band1_extChannel_field').show();
			var _wl_channel = new Array();
			if(amesh_support && httpApi.hasAiMeshNode() && !wl_info.band5g_2_support){
				channel_5g_1 = [];
				for(j=1; j<mesh_5g.chan_160m.chanspec.length; j++){
					channel_5g_1.push(mesh_5g.chan_160m.chanlist[j]);
					channel_5g_1_val.push(mesh_5g.chan_160m.chanspec[j]);					
				}
			}
			else{
				for (i = 0; i < channel_5g_1.length; i++) {
					var _cur_channel = parseInt(channel_5g_1[i]);
					var _reg = new RegExp("^" + _cur_channel);
					for (j = 0; j < wl1.channel_160m.length; j++) {
						if (wl1.channel_160m[j].match(_reg) != null) {
							_wl_channel.push(_cur_channel.toString());
							channel_5g_1_val.push(_cur_channel + "/160");
						}
					}
				}

				channel_5g_1 = _wl_channel;
			}			
		}
		else if(curBandwidth == '3'){
			$('#band1_extChannel_field').show();
			var _wl_channel = new Array();
			if(amesh_support && httpApi.hasAiMeshNode() && !wl_info.band5g_2_support){
				channel_5g_1 = [];
				for(j=1; j<mesh_5g.chan_80m.chanspec.length; j++){
					channel_5g_1.push(mesh_5g.chan_80m.chanlist[j]);
					channel_5g_1_val.push(mesh_5g.chan_80m.chanspec[j]);					
				}
			}
			else{
				for (i = 0; i < channel_5g_1.length; i++) {
					var _cur_channel = parseInt(channel_5g_1[i]);
					var _reg = new RegExp("^" + _cur_channel);
					for (j = 0; j < wl1.channel_80m.length; j++) {
						if (wl1.channel_80m[j].match(_reg) != null) {
							_wl_channel.push(_cur_channel.toString());
							channel_5g_1_val.push(_cur_channel + "/80");
						}
					}
				}

				channel_5g_1 = _wl_channel;
			}			
		}
		else if(curBandwidth == '2'){
			$('#band1_extChannel_field').show();
			_wl_channel = new Array();
			if(amesh_support && httpApi.hasAiMeshNode() && !wl_info.band5g_2_support){
				channel_5g_1 = [];
				for(j=1; j<mesh_5g.chan_40m.chanspec.length; j++){
					channel_5g_1.push(mesh_5g.chan_40m.chanlist[j]);
					channel_5g_1_val.push(mesh_5g.chan_40m.chanspec[j]);					
				}
			}
			else{
				for (i = 0; i < channel_5g_1.length; i++) {
					var _cur_channel = parseInt(channel_5g_1[i]);
					var _reg = new RegExp("^" + _cur_channel);
					for (j = 0; j < wl1.channel_40m.length; j++) {
						if (wl1.channel_40m[j].match(_reg) != null) {
							_wl_channel.push(_cur_channel.toString());
							channel_5g_1_val.push(wlextchannel_fourty(_cur_channel));
						}
					}
				}

				channel_5g_1 = _wl_channel;
			}			
		}
		else {
			$('#band1_extChannel_field').hide();
			_wl_channel = new Array();
			if(amesh_support && httpApi.hasAiMeshNode() && !wl_info.band5g_2_support){
				channel_5g_1 = [];
				for(j=1; j<mesh_5g.chan_20m.chanspec.length; j++){
					channel_5g_1.push(mesh_5g.chan_20m.chanlist[j]);
					channel_5g_1_val.push(mesh_5g.chan_20m.chanspec[j]);					
				}
			}
			else{
				for (i = 0; i < channel_5g_1.length; i++) {
					var _cur_channel = parseInt(channel_5g_1[i]);
					_wl_channel.push(_cur_channel.toString());
					channel_5g_1_val.push(_cur_channel);;
				}
			}			
		}
		
		channel_5g_1.unshift('<#Auto#>');
		channel_5g_1_val.unshift('0');
		add_options_x2(document.form.band1_channel, channel_5g_1, channel_5g_1_val, band1_curChannel);
		if (document.form.band1_channel.value == '0') {
			if(channel_5g_1.indexOf('56') != -1 || channel_5g_1.indexOf('100') != -1){
				$('#band1_acsDFS').show();
				if(curBandwidth == '5'){
					document.form.band1_acsDFS_checkbox.checked = true;
					document.form.band1_acsDFS_checkbox.disabled = true;
				}
				else{
					document.form.band1_acsDFS_checkbox.disabled = false;
				}
			}
			else{
				$('#band1_acsDFS').hide();
			}	
		}
		else {
			$('#band1_acsDFS').hide();
		}
	}
	else if(unit == '2'){
		if(band6g_support){		// due to GT-AXE11000 does not support
			if(document.getElementById('band2_psc6g_checkbox').checked){
				channel_5g_2 = ['37', '53', '69', '85', '101', '117', '133', '149', '165', '181', '197', '213'];
				if(is_EU_sku){
					channel_5g_2 = ['5', '21', '37', '53', '69', '85'];
				}
			}

			for(var i=channel_5g_2.length-1; i>=0; i--){
				var _channel = parseInt(channel_5g_2[i]);
				if(is_EU_sku){
					if(_channel > 221){
						channel_5g_2.splice(i, 1);
					}					
				}
				else if(_channel < 30 || _channel > 221){
					channel_5g_2.splice(i, 1);
				}
			}

			$('#band2_psc6g').show();
		}

		if (curBandwidth == '0') {
			$('#band2_extChannel_field').show();
			if(amesh_support && httpApi.hasAiMeshNode()){
				var _wl_channel = new Array();
				channel_5g_2 = [];
				for(j=1; j<mesh_5g2.auto.chanspec.length; j++){
					channel_5g_2.push(mesh_5g2.auto.chanlist[j]);
					channel_5g_2_val.push(mesh_5g2.auto.chanspec[j]);
				}
			}
			else{
				loop_auto: for (i = 0; i < channel_5g_2.length; i++) {
					var _cur_channel = parseInt(channel_5g_2[i]);
					if (band2_enable_bw_160 == '1') {
						for (j = 0; j < wl2.channel_160m.length; j++) {
							if (wl2.channel_160m[j].indexOf(_cur_channel) != -1) {
								if(band6g_support){
									channel_5g_2_val[i] = "6g" + _cur_channel + "/160";
								}
								else{
									channel_5g_2_val[i] = _cur_channel + "/160";
								}
								
								continue loop_auto;
							}
						}
					}

					for (j = 0; j < wl2.channel_80m.length; j++) {
						if (wl2.channel_80m[j].indexOf(_cur_channel) != -1) {
							if(band6g_support){
								channel_5g_2_val[i] = "6g" + _cur_channel + "/80";
							}
							else{
								channel_5g_2_val[i] = _cur_channel + "/80";
							}
							
							continue loop_auto;
						}
					}

					for (j = 0; j < wl2.channel_40m.length; j++) {
						if (wl2.channel_40m[j].indexOf(_cur_channel) != -1) {
							if(band6g_support){
								channel_5g_2_val[i] = "6g" + _cur_channel + "/40";
							}
							else{
								channel_5g_2_val[i] = wlextchannel_fourty(_cur_channel);
							}
							
							continue loop_auto;
						}
					}

					for (j = 0; j < wl2.channel_20m.length; j++) {
						if (wl2.channel_20m[j].indexOf(_cur_channel) != -1) {
							if(band6g_support){
								channel_5g_2_val[i] = "6g" + _cur_channel.toString();
							}
							else{
								channel_5g_2_val[i] = _cur_channel.toString();
							}
							
							continue loop_auto;
						}
					}
				}
			}			
		}
		else if (curBandwidth == '5') {
			$('#band2_extChannel_field').show();
			var _wl_channel = new Array();
			if(amesh_support && httpApi.hasAiMeshNode()){
				channel_5g_2 = [];
				for(j=1; j<mesh_5g2.chan_160m.chanspec.length; j++){
					channel_5g_2.push(mesh_5g2.chan_160m.chanlist[j]);
					channel_5g_2_val.push(mesh_5g2.chan_160m.chanspec[j]);
				}
			}
			else{
				for (i = 0; i < channel_5g_2.length; i++) {
					var _cur_channel = parseInt(channel_5g_2[i]);
					var _reg = new RegExp("^" + _cur_channel);					
					for (j = 0; j < wl2.channel_160m.length; j++) {
						if(band6g_support){
							if (wl2.channel_160m[j].includes('6g' + _cur_channel + '/160')) {
								_wl_channel.push(_cur_channel.toString());		
								channel_5g_2_val.push("6g" + _cur_channel + "/160");											
							}
						}
						else{
							if (wl2.channel_160m[j].match(_reg) != null) {
								_wl_channel.push(_cur_channel.toString());		
								channel_5g_2_val.push(_cur_channel + "/160");											
							}	
						}													
					}
				}

				channel_5g_2 = _wl_channel;
			}			
		}
		else if (curBandwidth == '3') {
			$('#band2_extChannel_field').show();
			var _wl_channel = new Array();
			if(amesh_support && httpApi.hasAiMeshNode()){
				channel_5g_2 = [];
				for(j=1; j<mesh_5g2.chan_80m.chanspec.length; j++){
					channel_5g_2.push(mesh_5g2.chan_80m.chanlist[j]);
					channel_5g_2_val.push(mesh_5g2.chan_80m.chanspec[j]);
				}
			}
			else{
				for (i = 0; i < channel_5g_2.length; i++) {
					var _cur_channel = parseInt(channel_5g_2[i]);
					var _reg = new RegExp("^" + _cur_channel);
					for (j = 0; j < wl2.channel_80m.length; j++) {
						if(band6g_support){
							if (wl2.channel_80m[j].includes('6g' + _cur_channel + '/80')) {
								_wl_channel.push(_cur_channel.toString());		
								channel_5g_2_val.push("6g" + _cur_channel + "/80");
							}						
						}
						else{
							if (wl2.channel_80m[j].match(_reg) != null) {
								_wl_channel.push(_cur_channel.toString());					
								channel_5g_2_val.push(_cur_channel + "/80");											
							}
						}					
					}
				}

				channel_5g_2 = _wl_channel;
			}			
		}
		else if (curBandwidth == '2') {
			$('#band2_extChannel_field').show();
			_wl_channel = new Array();
			if(amesh_support && httpApi.hasAiMeshNode()){
				channel_5g_2 = [];
				for(j=1; j<mesh_5g2.chan_40m.chanspec.length; j++){
					channel_5g_2.push(mesh_5g2.chan_40m.chanlist[j]);
					channel_5g_2_val.push(mesh_5g2.chan_40m.chanspec[j]);
				}
			}
			else{
				for (i = 0; i < channel_5g_2.length; i++) {
					var _cur_channel = parseInt(channel_5g_2[i]);
					var _reg = new RegExp("^" + _cur_channel);
					for (j = 0; j < wl2.channel_40m.length; j++) {
						if(band6g_support){
							if (wl2.channel_40m[j].includes('6g' + _cur_channel + '/40')) {
								_wl_channel.push(_cur_channel.toString());		
								channel_5g_2_val.push("6g" + _cur_channel + "/40");
							}						
						}
						else{
							if (wl2.channel_40m[j].match(_reg) != null) {
								_wl_channel.push(_cur_channel.toString());		
								channel_5g_2_val.push(wlextchannel_fourty(_cur_channel));												
							}
						}				
					}
				}

				channel_5g_2 = _wl_channel;
			}			
		}
		else{
			$('#band2_extChannel_field').hide();
			_wl_channel = new Array();
			if(amesh_support && httpApi.hasAiMeshNode()){
				channel_5g_2 = [];
				for(j=1; j<mesh_5g2.chan_20m.chanspec.length; j++){
					channel_5g_2.push(mesh_5g2.chan_20m.chanlist[j]);
					channel_5g_2_val.push(mesh_5g2.chan_20m.chanspec[j]);
				}
			}
			else{
				for (i = 0; i < channel_5g_2.length; i++) {
					var _cur_channel = parseInt(channel_5g_2[i]);
					_wl_channel.push(_cur_channel.toString());
					if(band6g_support){
						channel_5g_2_val.push("6g" + _cur_channel);
					}
					else{
						channel_5g_2_val.push(_cur_channel.toString());
					}				
				}
			}			
		}
		
		channel_5g_2.unshift('<#Auto#>');
		channel_5g_2_val.unshift('0');
		add_options_x2(document.form.band2_channel, channel_5g_2, channel_5g_2_val, band2_curChannel);
		if (document.form.band2_channel.value == '0') {
			if(channel_5g_2.indexOf('100') != -1){
				$('#band2_acsDFS').show();
				if(document.form.band2_bw.value == '5'){
					document.form.band2_acsDFS_checkbox.checked = true;
					document.form.band2_acsDFS_checkbox.disabled = true;
				}
				else{
					document.form.band2_acsDFS_checkbox.disabled = false;
				}
			}
			else{
				$('#band2_acsDFS').hide();
			}
		}
		else {
			$('#band2_acsDFS').hide();
		}
	}
}

function separateBWHandler(unit, bw){
	var curChannel = '0';

	if (unit == '0') {
		curChannel = '<% nvram_get("wl0_chanspec"); %>';
		if(bw == '0' || bw == '2'){
			$('#band0_extChannel_field').show();
		}
		else if(bw == '1'){
			$('#band0_extChannel_field').hide();
		}
	}
	else if(unit == '1'){
		curChannel = '<% nvram_get("wl1_chanspec"); %>';
		if (bw == '0' || bw == '2') {
			$('#band1_extChannel_field').show();
		}
		else if (bw == '1') {
			$('#band1_extChannel_field').hide();
		}
	}
	else if(unit == '2'){
		curChannel = '<% nvram_get("wl0_chanspec"); %>';
		if (bw == '0' || bw == '2') {
			$('#band2_extChannel_field').show();
		}
		else if (bw == '1') {
			$('#band2_extChannel_field').hide();
		}
	}

	separateGenChannel(unit, curChannel, bw);
}

function separateChannelHandler(unit, channel){
	var channel_2g = JSON.parse('<% channel_list_2g(); %>');
	var channel_5g_1 = JSON.parse('<% channel_list_5g(); %>');
	var channel_5g_2 = JSON.parse('<% channel_list_5g_2(); %>');
	var curCtrlChannel = channel;
	var extend_channel = new Array;
	var extend_channel_value = new Array;

	if(unit == '0'){
		$('#band0_acs_ch13').hide();
		if (channel_2g.length == '11') {		
			if (curCtrlChannel == 0) {
				extend_channel = ["<#Auto#>"];
				extend_channel_value = [""];
			}
			else if (curCtrlChannel >= 1 && curCtrlChannel <= 4) {
				extend_channel = ["<#WLANConfig11b_EChannelAbove#>"];
				extend_channel_value = ["l"];
			}
			else if (curCtrlChannel >= 5 && curCtrlChannel <= 7) {
				extend_channel = ["<#WLANConfig11b_EChannelAbove#>", "<#WLANConfig11b_EChannelBelow#>"];
				extend_channel_value = ["l", "u"];
			}
			else if (curCtrlChannel >= 8 && curCtrlChannel <= 11) {
				extend_channel = ["<#WLANConfig11b_EChannelBelow#>"];
				extend_channel_value = ["u"];
			}
		}
		else if (channel_2g.length == '13') {
			if (curCtrlChannel == 0) {
				extend_channel = ["<#Auto#>"];
				extend_channel_value = [""];
				$('#band0_acs_ch13').show();
			}
			else if (curCtrlChannel >= 1 && curCtrlChannel <= 4) {
				extend_channel = ["<#WLANConfig11b_EChannelAbove#>"];
				extend_channel_value = ["l"];
			}
			else if (curCtrlChannel >= 5 && curCtrlChannel <= 9) {
				extend_channel = ["<#WLANConfig11b_EChannelAbove#>", "<#WLANConfig11b_EChannelBelow#>"];
				extend_channel_value = ["l", "u"];
			}
			else if (curCtrlChannel >= 10 && curCtrlChannel <= 13) {
				extend_channel = ["<#WLANConfig11b_EChannelBelow#>"];
				extend_channel_value = ["u"];
			}
		}

		add_options_x2(document.form.band0_extChannel, extend_channel, extend_channel_value, curCtrlChannel);
	}
	else if (unit == '1') {
		if (curCtrlChannel == '0') {
			if(channel_5g_1.indexOf('56') != -1 || channel_5g_1.indexOf('100') != -1){
				$('#band1_acsDFS').show();
				if(document.form.band1_bw.value == '5'){
					document.form.band1_acsDFS_checkbox.checked = true;
					document.form.band1_acsDFS_checkbox.disabled = true;
				}
				else{
					document.form.band1_acsDFS_checkbox.disabled = false;
				}
			}
			else{
				$('#band1_acsDFS').hide();
			}
		}
		else {
			$('#band1_acsDFS').hide();
		}
	}
	else if (unit == '2') {
		if (curCtrlChannel == '0') {
			if(channel_5g_2.indexOf('100') != -1){
				$('#band2_acsDFS').show();
				if(document.form.band2_bw.value == '5'){
					document.form.band2_acsDFS_checkbox.checked = true;
					document.form.band2_acsDFS_checkbox.disabled = true;
				}
				else{
					document.form.band2_acsDFS_checkbox.disabled = false;
				}
			}
			else{
				$('#band2_acsDFS').hide();
			}
		}
		else {
			$('#band2_acsDFS').hide();
		}
	}
}
function controlHideSSIDHint() {
	var $hide_ssid_field = $('input:radio[name=wl' + dwb_info.band + '_closed]').closest("tr");
	$hide_ssid_field.find("#dwb_band_hide_hint").remove();
	if(dwb_info.mode){
		var sc_mode = ((_smart_connect_enable == "0") ? "0" : document.form.smart_connect_t.value);
		var is_hide_ssid = ($('input:radio[name=wl' + dwb_info.band + '_closed]:checked').val() == "1") ? true : false;
		if(sc_mode != "1" && is_hide_ssid)
			$hide_ssid_field.find("td").append($("<div>").attr({"id":"dwb_band_hide_hint"}).append($("<span>").html('<#AiMesh_dedicated_backhaul_band_hide_SSID#>')));
	}
}
function controlAXOnlyHint() {
	if(document.form.wl_nmode_x.value == "9")
		$("#wl_AXOnly_note").show();
	else
		$("#wl_AXOnly_note").hide();
}

function ajax_wl_channel(){
	$.ajax({
		url: '/ajax_wl_channel.asp',
		dataType: 'script',	
		error: function(xhr) {
			setTimeout("ajax_wl_channel();", 1000);
		},
		success: function(response){
			// $("#auto_channel").html("<#wireless_control_channel#>: " + cur_control_channel[wl_unit]);
			$('#band0_autoChannel').html('Current Control Channel: ' + cur_control_channel[0]);
			$('#band1_autoChannel').html('Current Control Channel: ' + cur_control_channel[1]);
			$('#band2_autoChannel').html('Current Control Channel: ' + cur_control_channel[2]);
			setTimeout("ajax_wl_channel();", 5000);
		}
	});
}

function handleMFP(){
	if(mbo_support && document.form.wl_mbo_enable.value == '1' && document.form.wl_mfp.value == '0'){
		$('#mbo_notice').show();
	}
	else{
		$('#mbo_notice').hide();
	}
}

function auth_method_change(unit, value, flag){
	var _temp = '';
	var _temp_value = '';

	if(unit == '0'){
		if(value == 'open'){
			$('#band0_encrypt_field').hide();
			$('#band0_psk_field').hide();
			$('#band0_mfp_field').hide();
			$('#band0_gtk_field').hide();
			$('#band0_radius_ip_field').hide();
			$('#band0_radius_port_field').hide();
			$('#band0_radius_key_field').hide();		
		}
		else if(value == 'openowe'){
			$('#band0_encrypt_field').show();
			$('#band0_psk_field').hide();
			$('#band0_mfp_field').show();
			$('#band0_gtk_field').show();
			$('#band0_radius_ip_field').hide();
			$('#band0_radius_port_field').hide();
			$('#band0_radius_key_field').hide();
			_temp = ['AES'];
			_temp_value = ['aes'];
			add_options_x2(document.form.band0_crypto, _temp, _temp_value, 'aes');
		}
		else if(value == 'psk2' || value == 'pskpsk2' || value == 'sae' || value == 'psk2sae'){
			$('#band0_encrypt_field').show();
			$('#band0_psk_field').show();
			$('#band0_mfp_field').show();
			$('#band0_gtk_field').show();
			$('#band0_radius_ip_field').hide();
			$('#band0_radius_port_field').hide();
			$('#band0_radius_key_field').hide();
			if(value == 'pskpsk2'){
				var _crypto = '<% nvram_get("wl0_crypto"); %>';
				_temp = ['AES', 'TKIP+AES'];
				_temp_value = ['aes', 'tkip+aes'];
				add_options_x2(document.form.band0_crypto, _temp, _temp_value, _crypto);

				var _mfp = '<% nvram_get("wl0_mfp"); %>';
				_temp = ['<#WLANConfig11b_WirelessCtrl_buttonname#>', '<#WLANConfig11b_x_mfp_opt1#>'];
				_temp_value = ['0', '1'];
				add_options_x2(document.form.band0_mfp, _temp, _temp_value, _mfp);
			}
			else if(value == 'psk2'){
				_temp = ['AES'];
				_temp_value = ['aes'];
				add_options_x2(document.form.band0_crypto, _temp, _temp_value, 'aes');

				var _mfp = '<% nvram_get("wl0_mfp"); %>';
				_temp = ['<#WLANConfig11b_WirelessCtrl_buttonname#>', '<#WLANConfig11b_x_mfp_opt1#>', '<#WLANConfig11b_x_mfp_opt2#>'];
				_temp_value = ['0', '1', '2'];
				add_options_x2(document.form.band0_mfp, _temp, _temp_value, _mfp);
			}
			else if(value == 'sae'){
				_temp = ['AES'];
				_temp_value = ['aes'];
				add_options_x2(document.form.band0_crypto, _temp, _temp_value, 'aes');

				_temp = ['<#WLANConfig11b_x_mfp_opt2#>'];
				_temp_value = ['2'];
				add_options_x2(document.form.band0_mfp, _temp, _temp_value, 2);
			}
			else if(value == 'psk2sae'){
				_temp = ['AES'];
				_temp_value = ['aes'];
				add_options_x2(document.form.band0_crypto, _temp, _temp_value, 'aes');

				var _mfp = '<% nvram_get("wl0_mfp"); %>';
				_temp = ['<#WLANConfig11b_x_mfp_opt1#>', '<#WLANConfig11b_x_mfp_opt2#>'];
				_temp_value = ['1', '2'];
				add_options_x2(document.form.band0_mfp, _temp, _temp_value, _mfp);
			}
		}
		else if(value == 'wpa2' || value == 'wpawpa2'){
			$('#band0_encrypt_field').show();
			$('#band0_psk_field').hide();
			$('#band0_mfp_field').show();
			$('#band0_gtk_field').show();
			$('#band0_radius_ip_field').show();
			$('#band0_radius_port_field').show();
			$('#band0_radius_key_field').show();
			
			if(value == 'wpa2'){
				var _crypto = '<% nvram_get("wl0_crypto"); %>';
				_temp = ['AES'];
				_temp_value = ['aes'];
				add_options_x2(document.form.band0_crypto, _temp, _temp_value, 'aes');

				var _mfp = '<% nvram_get("wl0_mfp"); %>';
				_temp = ['<#WLANConfig11b_WirelessCtrl_buttonname#>', '<#WLANConfig11b_x_mfp_opt1#>', '<#WLANConfig11b_x_mfp_opt2#>'];
				_temp_value = ['0', '1', '2'];
				add_options_x2(document.form.band0_mfp, _temp, _temp_value, _mfp);
			}
			else if(value == 'wpawpa2'){
				var _crypto = '<% nvram_get("wl0_crypto"); %>';
				_temp = ['AES', 'TKIP+AES'];
				_temp_value = ['aes', 'tkip+aes'];
				add_options_x2(document.form.band0_crypto, _temp, _temp_value, _crypto);

				var _mfp = '<% nvram_get("wl0_mfp"); %>';
				_temp = ['<#WLANConfig11b_WirelessCtrl_buttonname#>', '<#WLANConfig11b_x_mfp_opt1#>'];
				_temp_value = ['0', '1'];
				add_options_x2(document.form.band0_mfp, _temp, _temp_value, _mfp);
			}	
		}
	}
	else if(unit == '1'){
		if(value == 'open'){
			$('#band1_encrypt_field').hide();
			$('#band1_psk_field').hide();
			$('#band1_mfp_field').hide();
			$('#band1_gtk_field').hide();
			$('#band1_radius_ip_field').hide();
			$('#band1_radius_port_field').hide();
			$('#band1_radius_key_field').hide();
		}
		else if(value == 'openowe'){
			$('#band1_encrypt_field').show();
			$('#band1_psk_field').hide();
			$('#band1_mfp_field').show();
			$('#band1_gtk_field').show();
			$('#band1_radius_ip_field').hide();
			$('#band1_radius_port_field').hide();
			$('#band1_radius_key_field').hide();
			_temp = ['AES'];
			_temp_value = ['aes'];
			add_options_x2(document.form.band1_crypto, _temp, _temp_value, 'aes');
		}
		else if(value == 'psk2' || value == 'pskpsk2' || value == 'sae' || value == 'psk2sae'){
			$('#band1_encrypt_field').show();
			$('#band1_psk_field').show();
			$('#band1_mfp_field').show();
			$('#band1_gtk_field').show();
			$('#band1_radius_ip_field').hide();
			$('#band1_radius_port_field').hide();
			$('#band1_radius_key_field').hide();
			if(value == 'pskpsk2'){
				var _crypto = '<% nvram_get("wl1_crypto"); %>';
				_temp = ['AES', 'TKIP+AES'];
				_temp_value = ['aes', 'tkip+aes'];
				add_options_x2(document.form.band1_crypto, _temp, _temp_value, _crypto);

				var _mfp = '<% nvram_get("wl1_mfp"); %>';
				_temp = ['<#WLANConfig11b_WirelessCtrl_buttonname#>', '<#WLANConfig11b_x_mfp_opt1#>'];
				_temp_value = ['0', '1'];
				add_options_x2(document.form.band1_mfp, _temp, _temp_value, _mfp);
			}
			else if(value == 'psk2'){
				_temp = ['AES'];
				_temp_value = ['aes'];
				add_options_x2(document.form.band1_crypto, _temp, _temp_value, 'aes');

				var _mfp = '<% nvram_get("wl1_mfp"); %>';
				_temp = ['<#WLANConfig11b_WirelessCtrl_buttonname#>', '<#WLANConfig11b_x_mfp_opt1#>', '<#WLANConfig11b_x_mfp_opt2#>'];
				_temp_value = ['0', '1', '2'];
				add_options_x2(document.form.band1_mfp, _temp, _temp_value, _mfp);
			}
			else if(value == 'sae'){
				_temp = ['AES'];
				_temp_value = ['aes'];
				add_options_x2(document.form.band1_crypto, _temp, _temp_value, 'aes');

				_temp = ['<#WLANConfig11b_x_mfp_opt2#>'];
				_temp_value = ['2'];
				add_options_x2(document.form.band1_mfp, _temp, _temp_value, 2);
			}
			else if(value == 'psk2sae'){
				_temp = ['AES'];
				_temp_value = ['aes'];
				add_options_x2(document.form.band1_crypto, _temp, _temp_value, 'aes');

				var _mfp = '<% nvram_get("wl1_mfp"); %>';
				_temp = ['<#WLANConfig11b_x_mfp_opt1#>', '<#WLANConfig11b_x_mfp_opt2#>'];
				_temp_value = ['1', '2'];
				add_options_x2(document.form.band1_mfp, _temp, _temp_value, _mfp);
			}
		}
		else if(value == 'wpa2' || value == 'wpawpa2'){
			$('#band1_encrypt_field').show();
			$('#band1_psk_field').hide();
			$('#band1_mfp_field').show();
			$('#band1_gtk_field').show();
			$('#band1_radius_ip_field').show();
			$('#band1_radius_port_field').show();
			$('#band1_radius_key_field').show();
			if(value == 'wpa2'){
				var _crypto = '<% nvram_get("wl1_crypto"); %>';
				_temp = ['AES'];
				_temp_value = ['aes'];
				add_options_x2(document.form.band1_crypto, _temp, _temp_value, 'aes');

				var _mfp = '<% nvram_get("wl1_mfp"); %>';
				_temp = ['<#WLANConfig11b_WirelessCtrl_buttonname#>', '<#WLANConfig11b_x_mfp_opt1#>', '<#WLANConfig11b_x_mfp_opt2#>'];
				_temp_value = ['0', '1', '2'];
				add_options_x2(document.form.band1_mfp, _temp, _temp_value, _mfp);
			}
			else if(value == 'wpawpa2'){
				var _crypto = '<% nvram_get("wl1_crypto"); %>';
				_temp = ['AES', 'TKIP+AES'];
				_temp_value = ['aes', 'tkip+aes'];
				add_options_x2(document.form.band1_crypto, _temp, _temp_value, _crypto);

				var _mfp = '<% nvram_get("wl1_mfp"); %>';
				_temp = ['<#WLANConfig11b_WirelessCtrl_buttonname#>', '<#WLANConfig11b_x_mfp_opt1#>'];
				_temp_value = ['0', '1'];
				add_options_x2(document.form.band1_mfp, _temp, _temp_value, _mfp);
			}
		}
	}
	else if(unit == '2'){
		if(value == 'owe'){
			$('#band2_psk_field').hide();
			/*if(document.form.smart_connect_t.value == '1'){
				if(document.form.band01_auth_mode_x.value != 'open'){
					document.form.band01_auth_mode_x.value = 'open';
					auth_method_change('01', 'open');
				}				
			}*/
			if(document.form.smart_connect_t.value == '1' && flag != 'init'){
				document.form.band01_auth_mode_x.value = 'openowe';
				auth_method_change('01', 'openowe');
			}
		}
		else{
			$('#band2_psk_field').show();
			/*if(document.form.smart_connect_t.value == '1'){
				if(document.form.band01_auth_mode_x.value != 'psk2sae'){
					document.form.band01_auth_mode_x.value = 'psk2sae';
					auth_method_change('01', 'psk2sae');
				}				
			}*/
			if(document.form.smart_connect_t.value == '1' && flag != 'init'){
				document.form.band01_auth_mode_x.value = 'psk2sae';
				auth_method_change('01', 'psk2sae');
			}
		}
	}
	else if(unit == '01'){
		if(value == 'open'){
			$('#band01_encrypt_field').hide();
			$('#band01_psk_field').hide();
			$('#band01_mfp_field').hide();
			$('#band01_gtk_field').hide();
			$('#band01_radius_ip_field').hide();
			$('#band01_radius_port_field').hide();
			$('#band01_radius_key_field').hide();
			/*if(document.form.smart_connect_t.value == '1'){
				if(document.form.band2_auth_mode_x.value != 'owe'){
					document.form.band2_auth_mode_x.value = 'owe';
					auth_method_change('2', 'owe');
				}				
			}*/
			if(band6g_support && document.form.smart_connect_t.value == '1' && flag != 'init'){
				document.form.band2_auth_mode_x.value = 'owe';
				$('#band2_psk_field').hide();
			}
		}
		else if(value == 'openowe'){
			$('#band01_encrypt_field').show();
			$('#band01_psk_field').hide();
			$('#band01_mfp_field').show();
			$('#band01_gtk_field').show();
			$('#band01_radius_ip_field').hide();
			$('#band01_radius_port_field').hide();
			$('#band01_radius_key_field').hide();
			_temp = ['AES'];
			_temp_value = ['aes'];
			add_options_x2(document.form.band01_crypto, _temp, _temp_value, 'aes');
			if(band6g_support && document.form.smart_connect_t.value == '1' && flag != 'init'){
				document.form.band2_auth_mode_x.value = 'owe';
				$('#band2_psk_field').hide();
			}
		}
		else if(value == 'psk2' || value == 'pskpsk2' || value == 'sae' || value == 'psk2sae'){
			$('#band01_encrypt_field').show();
			$('#band01_psk_field').show();
			$('#band01_mfp_field').show();
			$('#band01_gtk_field').show();
			$('#band01_radius_ip_field').hide();
			$('#band01_radius_port_field').hide();
			$('#band01_radius_key_field').hide();

			if(value == 'pskpsk2'){
				var _crypto = '<% nvram_get("wl0_crypto"); %>';
				_temp = ['AES', 'TKIP+AES'];
				_temp_value = ['aes', 'tkip+aes'];
				add_options_x2(document.form.band01_crypto, _temp, _temp_value, _crypto);

				var _mfp = '<% nvram_get("wl0_mfp"); %>';
				_temp = ['<#WLANConfig11b_WirelessCtrl_buttonname#>', '<#WLANConfig11b_x_mfp_opt1#>'];
				_temp_value = ['0', '1'];
				add_options_x2(document.form.band01_mfp, _temp, _temp_value, _mfp);
			}
			else if(value == 'psk2'){
				_temp = ['AES'];
				_temp_value = ['aes'];
				add_options_x2(document.form.band01_crypto, _temp, _temp_value, 'aes');

				var _mfp = '<% nvram_get("wl0_mfp"); %>';
				_temp = ['<#WLANConfig11b_WirelessCtrl_buttonname#>', '<#WLANConfig11b_x_mfp_opt1#>', '<#WLANConfig11b_x_mfp_opt2#>'];
				_temp_value = ['0', '1', '2'];
				add_options_x2(document.form.band01_mfp, _temp, _temp_value, _mfp);
			}
			else if(value == 'sae'){
				_temp = ['AES'];
				_temp_value = ['aes'];
				add_options_x2(document.form.band01_crypto, _temp, _temp_value, 'aes');

				_temp = ['<#WLANConfig11b_x_mfp_opt2#>'];
				_temp_value = ['2'];
				add_options_x2(document.form.band01_mfp, _temp, _temp_value, 2);
			}
			else if(value == 'psk2sae'){
				_temp = ['AES'];
				_temp_value = ['aes'];
				add_options_x2(document.form.band01_crypto, _temp, _temp_value, 'aes');

				var _mfp = '<% nvram_get("wl0_mfp"); %>';
				_temp = ['<#WLANConfig11b_x_mfp_opt1#>', '<#WLANConfig11b_x_mfp_opt2#>'];
				_temp_value = ['1', '2'];
				add_options_x2(document.form.band01_mfp, _temp, _temp_value, _mfp);
				if(document.form.smart_connect_t.value == '1' && flag != 'init'){
					if(document.form.band2_auth_mode_x.value != 'sae'){
						document.form.band2_auth_mode_x.value = 'sae';
						auth_method_change('2', 'sae');
					}					
				}
			}

			if(band6g_support && document.form.smart_connect_t.value == '1' && flag != 'init'){
				document.form.band2_auth_mode_x.value = 'sae';
				$('#band2_psk_field').show();
			}
		}
		else if(value == 'wpa2' || value == 'wpawpa2'){
			$('#band01_encrypt_field').show();
			$('#band01_psk_field').hide();
			$('#band01_mfp_field').show();
			$('#band01_gtk_field').show();
			$('#band01_radius_ip_field').show();
			$('#band01_radius_port_field').show();
			$('#band01_radius_key_field').show();
			
			if(value == 'wpa2'){
				var _crypto = '<% nvram_get("wl0_crypto"); %>';
				_temp = ['AES'];
				_temp_value = ['aes'];
				add_options_x2(document.form.band01_crypto, _temp, _temp_value, 'aes');

				var _mfp = '<% nvram_get("wl0_mfp"); %>';
				_temp = ['<#WLANConfig11b_WirelessCtrl_buttonname#>', '<#WLANConfig11b_x_mfp_opt1#>', '<#WLANConfig11b_x_mfp_opt2#>'];
				_temp_value = ['0', '1', '2'];
				add_options_x2(document.form.band01_mfp, _temp, _temp_value, _mfp);
			}
			else if(value == 'wpawpa2'){
				var _crypto = '<% nvram_get("wl0_crypto"); %>';
				_temp = ['AES', 'TKIP+AES'];
				_temp_value = ['aes', 'tkip+aes'];
				add_options_x2(document.form.band01_crypto, _temp, _temp_value, _crypto);

				var _mfp = '<% nvram_get("wl0_mfp"); %>';
				_temp = ['<#WLANConfig11b_WirelessCtrl_buttonname#>', '<#WLANConfig11b_x_mfp_opt1#>'];
				_temp_value = ['0', '1'];
				add_options_x2(document.form.band01_mfp, _temp, _temp_value, _mfp);
			}	
		}
	}
}

function handle_bandwidth(unit, bw){
	if(unit == '0'){
		if(bw == '1'){
			$('#band0_extChannel_field').hide();
		}
		else{
			$('#band0_extChannel_field').show();
		}

		channel_2g(bw);
	}
	else if(unit == '1'){
		if(bw == '1'){
			$('#band1_extChannel_field').hide();
		}
		else{
			$('#band1_extChannel_field').show();
		}

		channel_5g(bw);
	}
	else{
		if(bw == '1'){
			$('#band2_extChannel_field').hide();
		}
		else{
			$('#band2_extChannel_field').show();
		}

		channel_6g(bw);
	}
}

function handle_channel(unit, channel){
	if(unit == '0'){
		cur_control_channel = channel;
		_ch = '<% nvram_get("wl0_chanspec"); %>';
		cur_extend_channel = _ch.slice(-1);
		if(cur_control_channel == 0){
			extend_channel = ["<#Auto#>"];
			extend_channel_value = [""];
			add_options_x2(document.form.band0_extChannel, extend_channel, extend_channel_value, 1);	
		}
		else if(cur_control_channel >= 1 && cur_control_channel <= 4){
			extend_channel = ["<#WLANConfig11b_EChannelAbove#>"];
			add_options_x2(document.form.band0_extChannel, extend_channel, "l");							
		}
		else if(wl_channel_list_2g.length == 12){    // 1 ~ 11
			if(cur_control_channel >= 5 && cur_control_channel <= 7){
				extend_channel = ["<#WLANConfig11b_EChannelAbove#>", "<#WLANConfig11b_EChannelBelow#>"];
				extend_channel_value = ["l", "u"];
				add_options_x2(document.form.band0_extChannel, extend_channel, extend_channel_value, cur_extend_channel);							
			}
			else if(cur_control_channel >= 8 && cur_control_channel <= 11){
				extend_channel = ["<#WLANConfig11b_EChannelBelow#>"];
				extend_channel_value = ["u"];
				add_options_x2(document.form.band0_extChannel, extend_channel, extend_channel_value, cur_extend_channel);								
			}
		}
		else{		// 1 ~ 13
			if(cur_control_channel >= 5 && cur_control_channel <= 9){
				extend_channel = ["<#WLANConfig11b_EChannelAbove#>", "<#WLANConfig11b_EChannelBelow#>"];
				extend_channel_value = ["l", "u"];
				add_options_x2(document.form.band0_extChannel, extend_channel, extend_channel_value, cur_extend_channel);							
			}
			else if(cur_control_channel >= 10 && cur_control_channel <= 13){
				extend_channel = ["<#WLANConfig11b_EChannelBelow#>"];
				extend_channel_value = ["u"];
				add_options_x2(document.form.band0_extChannel, extend_channel, extend_channel_value, cur_extend_channel);								
			}			
		}

		if(_ch == '0' && cur_control_channel == '0'){
			$('#band0_autoChannel').show();
			$('#band0_autoChannel').html('Current Control Channel: ' + current_control_channel[0]);
		}
		else{
			$('#band0_autoChannel').hide();
		}

		if(wl_channel_list_2g.length > 12){
			$('#band0_acs_ch13').show();
		}
		
	}
	else if(unit == '1'){
		cur_control_channel = channel;
		_ch = '<% nvram_get("wl1_chanspec"); %>';
		if(cur_control_channel == '0'){
			$('#band1_acsDFS').show();
		}
		else{
			$('#band1_acsDFS').hide();
		}


		if(_ch == '0' && cur_control_channel == '0'){
			$('#band1_autoChannel').show();
			$('#band1_autoChannel').html('Current Control Channel: ' + current_control_channel[1]);
		}
		else{
			$('#band1_autoChannel').hide();
		}
	}
	else{
		_ch = '<% nvram_get("wl2_chanspec"); %>';
		if(_ch == '0' && cur_control_channel == '0'){
			$('#band2_autoChannel').show();
			$('#band2_autoChannel').html('Current Control Channel: ' + current_control_channel[2]);
		}
		else{
			$('#band2_autoChannel').hide();
		}
	}
}

function channel_2g(bw){
	var chanspec = '<% nvram_get("wl0_chanspec"); %>';
	cur_extend_channel = chanspec.slice(-1);
	if(chanspec.search('[ul]') != -1){
		cur_extend_channel = chanspec.slice(-1);			//current control channel
		cur_control_channel = chanspec.split(cur_extend_channel)[0];	//current extension channel direction
	}
	else{
		cur_control_channel = chanspec;
	}

	if(wl_channel_list_2g[0] != "0"){
		wl_channel_list_2g.splice(0,0,"0");
	}

	_temp = wl_channel_list_2g.slice(0);
	_temp[0] = '<#Auto#>';
	add_options_x2(document.form.band0_channel, _temp, wl_channel_list_2g, cur_control_channel);
	if(cur_control_channel == 0){
		extend_channel = ["<#Auto#>"];
		extend_channel_value = [""];
		add_options_x2(document.form.band0_extChannel, extend_channel, extend_channel_value, 1);	
	}
	else if(cur_control_channel >= 1 && cur_control_channel <= 4){
		extend_channel = ["<#WLANConfig11b_EChannelAbove#>"];
		add_options_x2(document.form.band0_extChannel, extend_channel, "l");							
	}
	else if(wl_channel_list_2g.length == 12){    // 1 ~ 11
		if(cur_control_channel >= 5 && cur_control_channel <= 7){
			extend_channel = ["<#WLANConfig11b_EChannelAbove#>", "<#WLANConfig11b_EChannelBelow#>"];
			extend_channel_value = ["l", "u"];
			add_options_x2(document.form.band0_extChannel, extend_channel, extend_channel_value, cur_extend_channel);							
		}
		else if(cur_control_channel >= 8 && cur_control_channel <= 11){
			extend_channel = ["<#WLANConfig11b_EChannelBelow#>"];
			extend_channel_value = ["u"];
			add_options_x2(document.form.band0_extChannel, extend_channel, extend_channel_value, cur_extend_channel);								
		}
	}
	else{		// 1 ~ 13
		if(cur_control_channel >= 5 && cur_control_channel <= 9){
			extend_channel = ["<#WLANConfig11b_EChannelAbove#>", "<#WLANConfig11b_EChannelBelow#>"];
			extend_channel_value = ["l", "u"];
			add_options_x2(document.form.band0_extChannel, extend_channel, extend_channel_value, cur_extend_channel);							
		}
		else if(cur_control_channel >= 10 && cur_control_channel <= 13){
			extend_channel = ["<#WLANConfig11b_EChannelBelow#>"];
			extend_channel_value = ["u"];
			add_options_x2(document.form.band0_extChannel, extend_channel, extend_channel_value, cur_extend_channel);								
		}			
	}
}

function channel_5g(bw){
	var chanspec = '<% nvram_get("wl1_chanspec"); %>';
	var nmode_x = '<% nvram_get("wl1_nmode_x"); %>';
	var enable_bw_160 = document.getElementById('band1_160').checked;
	if(chanspec.search('[ul]') != -1){
		cur_extend_channel = chanspec.slice(-1);			//current control channel
		cur_control_channel = chanspec.split(cur_extend_channel)[0];	//current extension channel direction
	}
	else{
		cur_control_channel = chanspec;
	}

	$('#band1_extChannel_field').show();
	if(bw == '0'){
		var _wl_channel = new Array();
		loop_auto: for(i=0; i<wl_channel_list_5g.length; i++){
			var _cur_channel = parseInt(wl_channel_list_5g[i]);
			if(nmode_x != 1 && wl_info[1].bw_160_support && enable_bw_160){
				for(j=0;j<wl1.channel_160m.length;j++){
					if(wl1.channel_160m[j].indexOf(_cur_channel) != -1){
						_wl_channel.push(_cur_channel+"/160");
						continue loop_auto;
					}
				}
			}
	
			if(band5g_11ac_support && nmode_x != 1){
				for(j=0;j<wl1.channel_80m.length;j++){
					if(wl1.channel_80m[j].indexOf(_cur_channel) != -1){
						_wl_channel.push(_cur_channel+"/80");
						continue loop_auto;
					}
				}
			}
	
			for(j=0;j<wl1.channel_40m.length;j++){
				if(wl1.channel_40m[j].indexOf(_cur_channel) != -1){
					_wl_channel.push(wlextchannel_fourty(_cur_channel));
					continue loop_auto;
				}
			}
	
			for(j=0;j<wl1.channel_20m.length;j++){
				if(wl1.channel_20m[j].indexOf(_cur_channel) != -1){
					_wl_channel.push(_cur_channel.toString());
					continue loop_auto;
				}
			}				
		}
	}
	else if(bw == '5'){
		var _wl_channel = new Array();
		/*if(amesh_support && httpApi.hasAiMeshNode() && !wl_info.band5g_2_support){
			for(j=1; j<mesh_5g.chan_160m.chanspec.length; j++){
				_wl_channel.push(mesh_5g.chan_160m.chanspec[j]);
			}
		}
		else{*/
			for(i=0;i<wl_channel_list_5g.length; i++){
				var _cur_channel = parseInt(wl_channel_list_5g[i]);
				var _reg = new RegExp("^" + _cur_channel);
				for(j=0;j<wl1.channel_160m.length;j++){
					if(wl1.channel_160m[j].match(_reg) != null){
						_wl_channel.push(_cur_channel+"/160");
					}
				}
			}
		//}

		if(is_RU_sku){
			_wl_channel = ["36/160"];
		}
	}
	else if(bw == '3'){
		var _wl_channel = new Array();
		for(i=0;i<wl_channel_list_5g.length; i++){
			var _cur_channel = parseInt(wl_channel_list_5g[i]);
			var _reg = new RegExp("^" + _cur_channel);
			for(j=0;j<wl1.channel_80m.length;j++){
				if(wl1.channel_80m[j].match(_reg) != null){
					_wl_channel.push(_cur_channel+"/80");
				}
			}
		}

		if(is_RU_sku){
			_wl_channel = ["36/80", "52/80", "132/80"];
			if(band5g2_support){
				_wl_channel = ["36/80", "52/80"];
			}
		}
	}
	else if(bw == '2'){
		var _wl_channel = new Array();
		for(i=0;i<wl_channel_list_5g.length; i++){
			var _cur_channel = parseInt(wl_channel_list_5g[i]);
			for(j=0;j<wl1.channel_40m.length;j++){
				var _l = wl1.channel_40m[j].split("l");
				var _u = wl1.channel_40m[j].split("u");
				if(_l.length > 1){
					if(_l[0] == _cur_channel){
						_wl_channel.push(wlextchannel_fourty(_cur_channel));
					}
				}
				else{
					if(_u[0] == _cur_channel){
						_wl_channel.push(wlextchannel_fourty(_cur_channel));
					}
				}
			}	
		}
		
		if(is_RU_sku){
			_wl_channel = ["36l", "44l", "52l", "60l", "132l", "140l"];
			if(band5g2_support){
				_wl_channel = ["36l", "44l", "52l", "60l"];
			}
		}
	}
	else{		// 20 MHz
		$('#band1_extChannel_field').hide();
		var _wl_channel = wl_channel_list_5g.slice(0);
	}

	var _wl_channel_string = new Array();
	for(var i =0;i<_wl_channel.length;i++){
		var _temp = '';
		if(_wl_channel[i].indexOf('/160') != -1){
			
			_temp = _wl_channel[i].split('/160')[0];
		}
		else if(_wl_channel[i].indexOf('/80') != -1){
			_temp = _wl_channel[i].split('/80')[0];
		}
		else if(_wl_channel[i].indexOf('u') != -1){
			_temp = _wl_channel[i].split('u')[0];
		}
		else if(_wl_channel[i].indexOf('l') != -1){
			_temp = _wl_channel[i].split('l')[0];
		}else{
			_temp = _wl_channel[i];
		}

		_wl_channel_string.push(_temp);
	}

	_wl_channel.unshift('0');
	_wl_channel_string.unshift('<#Auto#>');
	add_options_x2(document.form.band1_channel, _wl_channel_string, _wl_channel, chanspec);
}

function channel_6g(bw){
	var chanspec = '<% nvram_get("wl2_chanspec"); %>';
	var nmode_x = '<% nvram_get("wl2_nmode_x"); %>';
	if(document.getElementById('band2_psc6g_checkbox').checked){
		wl_channel_list_5g_2 = ['37', '53', '69', '85', '101', '117', '133', '149', '165', '181', '197', '213'];
		if(is_EU_sku){
			wl_channel_list_5g_2 = ['5', '21', '37', '53', '69', '85', '101', '117', '133', '149', '165', '181', '197', '213'];
		}
	}
	else{
		wl_channel_list_5g_2 = JSON.parse('<% channel_list_5g_2(); %>');
	}

	for(var i=wl_channel_list_5g_2.length-1; i>=0; i--){
		var _channel = parseInt(wl_channel_list_5g_2[i]);
		if(is_EU_sku){	// remove 225, 229, 233
			if(_channel > 221){
				wl_channel_list_5g_2.splice(i, 1);
			}			
		}
		else if(_channel < 30 || _channel > 221){	// remove 1, 5, 9, 13, 17, 21, 25, 29, 225, 229, 233
			wl_channel_list_5g_2.splice(i, 1);
		}
	}
	
	$('#band1_extChannel_field').show();
	if(bw == '0'){
		var _wl_channel = new Array();
		loop_auto: for(i=0; i<wl_channel_list_5g_2.length; i++){
			var _cur_channel = parseInt(wl_channel_list_5g_2[i]);
			if(nmode_x != 1){
				for(j=0;j<wl2.channel_160m.length;j++){
					if(wl2.channel_160m[j].indexOf("6g" + _cur_channel + "/160") != -1){
						_wl_channel.push("6g" + _cur_channel + "/160");
						continue loop_auto;
					}
				}
			}
	
			if(nmode_x != 1){
				for(j=0;j<wl2.channel_80m.length;j++){
					if(wl2.channel_80m[j].indexOf("6g" + _cur_channel + "/80") != -1){
						_wl_channel.push("6g" + _cur_channel + "/80");
						continue loop_auto;
					}
				}
			}
	
			for(j=0;j<wl2.channel_40m.length;j++){
				if(wl2.channel_40m[j].indexOf("6g" + _cur_channel + "/40") != -1){
					_wl_channel.push("6g" + _cur_channel + "/40");
					continue loop_auto;
				}
			}
	
			for(j=0;j<wl2.channel_20m.length;j++){
				if(wl2.channel_20m[j].indexOf("6g" +  _cur_channel) != -1){
					_wl_channel.push("6g" +  _cur_channel);
					continue loop_auto;
				}
			}				
		}
	}
	else if(bw == '5'){
		var _wl_channel = new Array();
		for(i=0;i<wl_channel_list_5g_2.length; i++){
			var _cur_channel = parseInt(wl_channel_list_5g_2[i]);
			var _reg = new RegExp("^" + _cur_channel);
			for(j=0;j<wl2.channel_160m.length;j++){
				if(wl2.channel_160m[j].indexOf("6g" + _cur_channel + "/160") != -1){
					_wl_channel.push("6g" + _cur_channel + "/160");
				}
			}	
		}

		if(is_RU_sku){
			_wl_channel = ["36/160"];
		}
	}
	else if(bw == '3'){
		var _wl_channel = new Array();
		for(i=0;i<wl_channel_list_5g_2.length; i++){
			var _cur_channel = parseInt(wl_channel_list_5g_2[i]);
			for(j=0;j<wl2.channel_80m.length;j++){
				if(wl2.channel_80m[j].indexOf("6g" + _cur_channel + "/80") != -1){
					_wl_channel.push("6g" + _cur_channel + "/80");
				}				
			}
		}

		if(is_RU_sku){
			_wl_channel = ["36/80", "52/80", "132/80"];
			if(band5g2_support){
				_wl_channel = ["36/80", "52/80"];
			}
		}
	}
	else if(bw == '2'){
		var _wl_channel = new Array();
		for(i=0;i<wl_channel_list_5g_2.length; i++){
			var _cur_channel = parseInt(wl_channel_list_5g_2[i]);
			for(j=0;j<wl2.channel_40m.length;j++){
				if(wl2.channel_40m[j].indexOf("6g" + _cur_channel + "/40") != -1){
					_wl_channel.push("6g" + _cur_channel + "/40");
				}
			}	
		}
		
		if(is_RU_sku){
			_wl_channel = ["36l", "44l", "52l", "60l", "132l", "140l"];
			if(band5g2_support){
				_wl_channel = ["36l", "44l", "52l", "60l"];
			}
		}
	}
	else{		// 20 MHz
		$('#band1_extChannel_field').hide();
		var _wl_channel = new Array();
		for(i=0;i<wl_channel_list_5g_2.length; i++){
			var _cur_channel = parseInt(wl_channel_list_5g_2[i]);
			for(j=0;j<wl2.channel_40m.length;j++){
				if((wl2.channel_20m[j].indexOf("6g" + _cur_channel) != -1) && wl2.channel_20m[j].length === ("6g" + _cur_channel).length){
					_wl_channel.push("6g" + _cur_channel);
				}
			}	
		}
	}

	var _wl_channel_string = new Array();
	for(var i =0;i<_wl_channel.length;i++){
		var _temp = '';
		if(_wl_channel[i].indexOf('/160') != -1){
			
			_temp = _wl_channel[i].split('6g')[1].split('/160')[0];
		}
		else if(_wl_channel[i].indexOf('/80') != -1){
			_temp = _wl_channel[i].split('6g')[1].split('/80')[0];
		}
		else if(_wl_channel[i].indexOf('/40') != -1){
			_temp = _wl_channel[i].split('6g')[1].split('/40')[0];
		}
		else{
			_temp = _wl_channel[i].split('6g')[1];
		}

		_wl_channel_string.push(_temp);
	}

	_wl_channel.unshift('0');
	_wl_channel_string.unshift('<#Auto#>');
	add_options_x2(document.form.band2_channel, _wl_channel_string, _wl_channel, chanspec);
}

function handle_smart_connect(value, flag){
	//var _smart_connect_x_ori = '<% nvram_get("smart_connect_x"); %>';
	if(value == '0'){
		document.getElementById("smart_connect_field").style.display = "none";
		document.getElementById("smartcon_rule_link").style.display = "none";
		$('#band012_ssid_field').hide();		
		auth_method_change('0', document.form.band0_auth_mode_x.value);
		auth_method_change('1', document.form.band1_auth_mode_x.value);
		auth_method_change('2', document.form.band2_auth_mode_x.value);
		$('#band01_title_field').hide();
		$('#band01_ssid_field').hide();
		$('#band01_auth_method_field').hide();
		$('#band01_encrypt_field').hide();
		$('#band01_psk_field').hide();
		$('#band01_radius_ip_field').hide();
		$('#band01_radius_port_field').hide();
		$('#band01_radius_key_field').hide();
		$('#band01_mfp_field').hide();
		$('#band01_gtk_field').hide();
		$('#band0_ssid_field').show();
		$('#band0_auth_method_field').show();
		$('#band1_ssid_field').show();
		$('#band1_auth_method_field').show();
		$('#band2_ssid_field').show();

		$('#band2_hide_ssid_field').show();
		$('#band5_hide_ssid_field').show();
		$('#band6_hide_ssid_field').show();
		$('#band25_hide_ssid_field').hide();
		$('#band256_hide_ssid_field').hide();

		if(wl_info['1'].bw_160_support){
			$("#band1_160_field").show();		
		}
		else{
			$("#band1_160_field").hide();	
		}

		if(wl_info['1'].dfs_support){
			$("#band1_acsDFS").show();
		}
		else{
			$("#band1_acsDFS").hide();
		}
	}
	else{
		document.getElementById("smart_connect_field").style.display = "";
		document.getElementById("smartcon_rule_link").style.display = "table-cell";
		if(value == '1'){
			$('#band012_ssid_field').show();
			
			if(flag != "init"){
				document.form.band01_auth_mode_x.value = 'psk2sae';
				auth_method_change('01', 'psk2sae');
				document.form.band2_auth_mode_x.value = 'sae';
				auth_method_change('2', 'sae');
			}
			
			$('#band01_title_field').show();
			$('#band01_ssid_field').hide();
			$('#band01_auth_method_field').show();
			$('#band0_ssid_field').hide();
			$('#band0_auth_method_field').hide();
			$('#band0_encrypt_field').hide();
			$('#band0_psk_field').hide();
			$('#band0_radius_ip_field').hide();
			$('#band0_radius_port_field').hide();
			$('#band0_radius_key_field').hide();
			$('#band0_mfp_field').hide();
			$('#band0_gtk_field').hide();	
			$('#band1_ssid_field').hide();
			$('#band1_auth_method_field').hide();
			$('#band1_encrypt_field').hide();
			$('#band1_psk_field').hide();
			$('#band1_radius_ip_field').hide();
			$('#band1_radius_port_field').hide();
			$('#band1_radius_key_field').hide();
			$('#band1_mfp_field').hide();
			$('#band1_gtk_field').hide();
			$('#band2_ssid_field').hide();

			$('#band2_hide_ssid_field').hide();
			$('#band5_hide_ssid_field').hide();
			$('#band6_hide_ssid_field').hide();
			$('#band25_hide_ssid_field').hide();
			$('#band256_hide_ssid_field').show();
		
			if(wl_info['1'].bw_160_support){
				$("#band1_160_field").show();		
			}
			else{
				$("#band1_160_field").hide();	
			}

			if(wl_info['1'].dfs_support){
				$("#band1_acsDFS").show();
			}
			else{
				$("#band1_acsDFS").hide();
			}
		}
		else if(value == '3'){
			if(flag != 'init'){
				document.form.band01_auth_mode_x.value = 'psk2';
				document.form.band01_mfp.value = '0';
				auth_method_change('01', document.form.band01_auth_mode_x.value);
				auth_method_change('2', document.form.band2_auth_mode_x.value);
			}
			
			$('#band01_title_field').show();
			$('#band012_ssid_field').hide();
			$('#band01_ssid_field').show();
			$('#band01_auth_method_field').show();
			$('#band0_ssid_field').hide();
			$('#band0_auth_method_field').hide();
			$('#band0_encrypt_field').hide();
			$('#band0_psk_field').hide();
			$('#band0_radius_ip_field').hide();
			$('#band0_radius_port_field').hide();
			$('#band0_radius_key_field').hide();
			$('#band0_mfp_field').hide();
			$('#band0_gtk_field').hide();
			$('#band1_ssid_field').hide();
			$('#band1_auth_method_field').hide();
			$('#band1_encrypt_field').hide();
			$('#band1_psk_field').hide();
			$('#band1_radius_ip_field').hide();
			$('#band1_radius_port_field').hide();
			$('#band1_radius_key_field').hide();
			$('#band1_mfp_field').hide();
			$('#band1_gtk_field').hide();
			$('#band2_ssid_field').show();
			
			$('#band2_hide_ssid_field').hide();
			$('#band5_hide_ssid_field').hide();
			$('#band6_hide_ssid_field').show();
			$('#band25_hide_ssid_field').show();
			$('#band256_hide_ssid_field').hide();

			if(document.form.band01_ssid.value == document.form.band2_ssid.value){
				document.form.band2_ssid.value = document.form.band01_ssid.value + '_6G';
			}

			if(wl_info['1'].bw_160_support){
				$("#band1_160_field").show();		
			}
			else{
				$("#band1_160_field").hide();	
			}

			if(wl_info['1'].dfs_support){
				$("#band1_acsDFS").show();
			}
			else{
				$("#band1_acsDFS").hide();
			}
		}
	}	

	if(isSupport("amas_fronthaul_network") && $(".fronthaul_ap").length > 0){
		$(".fronthaul_ap").hide();
		if(value == "1"){
			if(dwb_info.mode == "1")
				$(".fronthaul_ap").show();
			if($("#fh_ap_enabled option[value='2']").length > 0)
				$("#fh_ap_enabled").val(2);
		}
		else{
			if($("#fh_ap_enabled option[value='0']").length > 0)
				$("#fh_ap_enabled").val(0);
		}
	}
	controlHideSSIDHint();
}

function handleAiMeshBackhaul(value){
	var fh_ap_enabled = '<% nvram_get("fh_ap_enabled"); %>';
	if(value === '2'){
		if(meshBackhaulAutoSupport){
			document.querySelector('#fh_connection_hint').style.display = 'none';
			document.querySelector('#fh_ap_enabled').value = '2';
		}
		
		if(unii4Support){
			document.querySelector('#acs_unii4_checkbox').checked =  false;
		}
	}
	else if(value === '0'){
		if(meshBackhaulAutoSupport){
			document.querySelector('#fh_connection_hint').style.display = '';
		}
		
		if(unii4Support){
			if(fh_ap_enabled === '1'){
				document.querySelector('#acs_unii4_checkbox').checked =  false;
				if(meshBackhaulAutoSupport){
					document.querySelector('#fh_connection_hint_checkbox').checked = true;
				}
			}
			else{
				document.querySelector('#acs_unii4_checkbox').checked =  true;
				if(meshBackhaulAutoSupport){
					document.querySelector('#fh_connection_hint_checkbox').checked = false;
				}				
			}
		}	
	}
}

function handleFhConnectionHint(value){
	if(value){
		document.querySelector('#fh_ap_enabled').value = '0';
		if(unii4Support){
			document.querySelector('#acs_unii4_checkbox').checked =  false;
		}
	}
	else{
		document.querySelector('#fh_ap_enabled').value = '0';
		if(unii4Support){
			document.querySelector('#acs_unii4_checkbox').checked =  true;
		}
	}
}

function handleUNII4Hint(value){
	if(value){
		alert("<#WLANConfig11b_EChannel_U-NII-4_alert#>");
	}
}

function gen_fronthaul_ap(value){
	if(isSupport("amas_fronthaul_network")){
		$(".fronthaul_ap").remove();
		var get_cfg_clientlist = httpApi.hookGet("get_cfg_clientlist");
		if(get_cfg_clientlist[0] != undefined){
			var $selectObj = $("<select/>").attr("id","fh_ap_enabled").addClass("input_option");
			if(meshBackhaulAutoSupport || unii4Support) {
				$selectObj.attr('onChange', 'handleAiMeshBackhaul(this.value)');
			}

			var select_node_capability = httpApi.aimesh_get_node_capability(get_cfg_clientlist[0]);
			if(select_node_capability.fronthaul_ap_option_on)
				$selectObj.append($('<option>', {value: "2", text: "<#AiMesh_WiFi_Backhaul_both#>"}));
			if(select_node_capability.fronthaul_ap_option_auto)
				$selectObj.append($('<option>', {value: "1", text: "<#Auto#>"}));
			if(select_node_capability.fronthaul_ap_option_off)
				$selectObj.append($('<option>', {value: "0", text: "<#AiMesh_WiFi_Backhaul_dedicated_backhaul#>"}));

			if(meshBackhaulAutoSupport ){
				$selectObj.append($('<option>', {value: "1", text: "", hidden: true}));			
			}

			if($selectObj.find("option").length > 0){
				var $trObj = $("<tr>").addClass("fronthaul_ap");
				var $thObj = $("<th>").html("<#AiMesh_WiFi_Backhaul#>");
				var $tdObj = $("<td>");
				$trObj.append($thObj).append($tdObj.append($selectObj));
				if(meshBackhaulAutoSupport ){
					var fh_connect_obj ='<div id="fh_connection_hint" style="color:#FC0;"><input id="fh_connection_hint_checkbox" type="checkbox" onchange="handleFhConnectionHint(this.checked)"><#AiMesh_WiFi_Backhaul_release_fronthaul#></div>';
					$trObj.append($thObj).append($tdObj.append(fh_connect_obj));
				}

				$("#band" + dwb_info.band + "_ssid_field").after($trObj);
				var fh_ap_enabled = httpApi.nvramGet(["fh_ap_enabled"]).fh_ap_enabled;
				if(fh_ap_enabled != "" && $selectObj.children("option[value=" + fh_ap_enabled + "]").length > 0)
					$selectObj.val(fh_ap_enabled);
			}
			if(dwb_info.mode == "1" && value == "1")
				$(".fronthaul_ap").show();
			else
				$(".fronthaul_ap").hide();
		}
	}
}

function handle_auth(obj){
	var smart_connect = document.form.smart_connect_t.value;
	if(smart_connect == '1'){
		if(obj.name == 'band01_wpa_psk'){
			document.form.band2_wpa_psk.value = document.form.band01_wpa_psk.value;
		}
		else if(obj.name == 'band2_wpa_psk'){
			document.form.band01_wpa_psk.value = document.form.band2_wpa_psk.value;
		}		
	}
}
</script>
</head>

<body onload="initial();" onunLoad="return unload_body();" class="bg">
<div id="TopBanner"></div>
<div id="Loading" class="popup_bg"></div>
<div id="hiddenMask" class="popup_bg">
	<table cellpadding="4" cellspacing="0" id="dr_sweet_advise" class="dr_sweet_advise" align="center">
		<tr>
		<td>
			<div class="drword" id="drword"><#Main_alert_proceeding_desc4#> <#Main_alert_proceeding_desc1#>...
				<br/>
				<div id="disconnect_hint" style="display:none;"><#Main_alert_proceeding_desc2#></div>
				<br/>
		    </div>
			<div id="wireless_client_detect" style="margin-left:10px;position:absolute;display:none;width:400px;">
				<img src="images/loading.gif">
				<div style="margin:-55px 0 0 75px;"><#QKSet_Internet_Setup_fail_method1#></div>
			</div> 
			<div class="drImg"><img src="images/alertImg.png"></div>
			<div style="height:100px; "></div>
		</td>
		</tr>
	</table>
<!--[if lte IE 6.5]><iframe class="hackiframe"></iframe><![endif]-->
</div>
<script>

</script>
<iframe name="hidden_frame" id="hidden_frame" width="0" height="0" frameborder="0"></iframe>
<form method="post" name="autochannelform" action="/start_apply2.htm" target="hidden_frame">
<input type="hidden" name="productid" value="<% nvram_get("productid"); %>">
<input type="hidden" name="current_page" value="Advanced_Wireless_Content.asp">
<input type="hidden" name="next_page" value="Advanced_Wireless_Content.asp">
<input type="hidden" name="modified" value="0">
<input type="hidden" name="action_mode" value="apply_new">
<input type="hidden" name="action_script" value="restart_wireless">
<input type="hidden" name="action_wait" value="10">
<input type="hidden" name="wl_country_code" value="<% nvram_get("wl0_country_code"); %>" disabled>
<input type="hidden" name="firmver" value="<% nvram_get("firmver"); %>">
<input type="hidden" name="wl_chanspec" value="">
<input type="hidden" name="wl_unit" value="">
<input type="hidden" name="force_change" value="<% nvram_get("force_change"); %>">
</form>	
<form method="post" name="form" action="/start_apply2.htm" target="hidden_frame">
<input type="hidden" name="productid" value="<% nvram_get("productid"); %>">
<input type="hidden" name="current_page" value="Advanced_Wireless_Content.asp">
<input type="hidden" name="next_page" value="Advanced_Wireless_Content.asp">
<input type="hidden" name="modified" value="0">
<input type="hidden" name="action_mode" value="apply_new">
<input type="hidden" name="action_script" value="restart_wireless">
<input type="hidden" name="action_wait" value="10">
<input type="hidden" name="preferred_lang" id="preferred_lang" value="<% nvram_get("preferred_lang"); %>">
<input type="hidden" name="wl_country_code" value="<% nvram_get("wl0_country_code"); %>" disabled>
<input type="hidden" name="firmver" value="<% nvram_get("firmver"); %>">
<input type="hidden" name="wps_mode" value="<% nvram_get("wps_mode"); %>">
<input type="hidden" name="wps_config_state" value="<% nvram_get("wps_config_state"); %>">
<input type="hidden" name="wl_ssid_org" value="<% nvram_char_to_ascii("WLANConfig11b", "wl_ssid"); %>">
<input type="hidden" name="wlc_ure_ssid_org" value="<% nvram_char_to_ascii("WLANConfig11b", "wlc_ure_ssid"); %>" disabled>
<input type="hidden" name="wl_wpa_psk_org" value="<% nvram_char_to_ascii("WLANConfig11b", "wl_wpa_psk"); %>">
<input type="hidden" name="wl_key1_org" value="<% nvram_char_to_ascii("WLANConfig11b", "wl_key1"); %>">
<input type="hidden" name="wl_key2_org" value="<% nvram_char_to_ascii("WLANConfig11b", "wl_key2"); %>">
<input type="hidden" name="wl_key3_org" value="<% nvram_char_to_ascii("WLANConfig11b", "wl_key3"); %>">
<input type="hidden" name="wl_key4_org" value="<% nvram_char_to_ascii("WLANConfig11b", "wl_key4"); %>">
<input type="hidden" name="wl_phrase_x_org" value="<% nvram_char_to_ascii("WLANConfig11b", "wl_phrase_x"); %>">
<input type="hidden" maxlength="15" size="15" name="x_RegulatoryDomain" value="<% nvram_get("x_RegulatoryDomain"); %>" readonly="1">
<input type="hidden" name="wl_gmode_protection" value="<% nvram_get("wl_gmode_protection"); %>">
<input type="hidden" name="wl_wme" value="<% nvram_get("wl_wme"); %>">
<input type="hidden" name="wl_mode_x" value="<% nvram_get("wl_mode_x"); %>">
<input type="hidden" name="wl_nmode_x_orig" value="<% nvram_get("wl_nmode_x"); %>">
<input type="hidden" name="wl_nctrlsb_old" value="<% nvram_get("wl_nctrlsb"); %>">
<input type="hidden" name="wl_key_type" value='<% nvram_get("wl_key_type"); %>'> <!--Lock Add 2009.03.10 for ralink platform-->
<input type="hidden" name="wl_channel_orig" value='<% nvram_get("wl_channel"); %>'>
<input type="hidden" name="wl_chanspec" value=''>
<input type="hidden" name="wl_wep_x_orig" value='<% nvram_get("wl_wep_x"); %>'>
<input type="hidden" name="wl_optimizexbox" value='<% nvram_get("wl_optimizexbox"); %>'>
<input type="hidden" name="wl_bw_160" value='<% nvram_get("wl_bw_160"); %>'>
<input type="hidden" name="wl0_bw" value='<% nvram_get("wl0_bw"); %>'>
<input type="hidden" name="wl1_bw" value='<% nvram_get("wl1_bw"); %>'>
<input type="hidden" name="wl2_bw" value='<% nvram_get("wl2_bw"); %>'>
<input type="hidden" name="wl0_chanspec" value='<% nvram_get("wl0_chanspec"); %>'>
<input type="hidden" name="wl1_chanspec" value='<% nvram_get("wl1_chanspec"); %>'>
<input type="hidden" name="wl2_chanspec" value='<% nvram_get("wl2_chanspec"); %>'>
<input type="hidden" name="wl1_bw_160" value='<% nvram_get("wl1_bw_160"); %>'>
<input type="hidden" name="wl2_bw_160" value='<% nvram_get("wl2_bw_160"); %>'>
<input type="hidden" name="wl2_auth_mode_x" value='<% nvram_get("wl2_auth_mode_x"); %>'>
<input type="hidden" name="wl2_crypto" value='<% nvram_get("wl2_crypto"); %>'>
<input type="hidden" name="wl2_wpa_psk" value='<% nvram_get("wl2_wpa_psk"); %>'>
<input type="hidden" name="wl2_mfp" value='<% nvram_get("wl2_mfp"); %>'>
<input type="hidden" name="wl2_wpa_gtk_rekey" value='<% nvram_get("wl2_wpa_gtk_rekey"); %>'>


<input type="hidden" name="wl_subunit" value='-1'>
<input type="hidden" name="wl1_dfs" value='<% nvram_get("wl1_dfs"); %>'>
<input type="hidden" name="acs_dfs" value='<% nvram_get("acs_dfs"); %>'>
<input type="hidden" name="acs_band1" value='<% nvram_get("acs_band1"); %>'>
<input type="hidden" name="acs_band3" value='<% nvram_get("acs_band3"); %>'>
<input type="hidden" name="acs_ch13" value='<% nvram_get("acs_ch13"); %>'>
<input type="hidden" name="psc6g" value='<% nvram_get("psc6g"); %>'>
<input type="hidden" name="acs_unii4" value='<% nvram_get("acs_unii4"); %>'>
<input type="hidden" name="wps_enable" value="<% nvram_get("wps_enable"); %>">
<input type="hidden" name="wps_band" value="<% nvram_get("wps_band_x"); %>">
<input type="hidden" name="wps_dualband" value="<% nvram_get("wps_dualband"); %>">
<input type="hidden" name="smart_connect_x" value="<% nvram_get("smart_connect_x"); %>">
<input type="hidden" name="wl1_80211h" value="<% nvram_get("wl1_80211h"); %>" >
<input type="hidden" name="w_Setting" value="1">
<input type="hidden" name="w_apply" value="1">
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
	<td align="left" valign="top" >
	  <table width="760px" border="0" cellpadding="4" cellspacing="0" class="FormTitle" id="FormTitle">
		<tbody>
		<tr>
		  <td bgcolor="#4D595D" valign="top">
		  <div>&nbsp;</div>
		  <div class="formfonttitle"><#menu5_1#> - <#menu5_1_1#></div>
      <div style="margin: 10px 0 10px 5px" class="splitLine"></div>
      <div class="formfontdesc"><#adv_wl_desc#></div>
		
			<table width="100%" border="1" align="center" cellpadding="4" cellspacing="0" id="WLgeneral" class="FormTable">
			<tr id="smartcon_enable_field" style="">
				<th width="30%"><a class="hintstyle" href="javascript:void(0);" onClick="openHint(0,27);"><#smart_connect_enable#></a></th>
			  	<td>
			    	<div id="smartcon_enable_block" style="display:none;">
			    		<span style="color:#FFF;" id="smart_connect_enable_word">&nbsp;&nbsp;</span>
			    		<input type="button" name="enableSmartConbtn" id="enableSmartConbtn" value="" class="button_gen" onClick="enableSmartCon();">
			    		<br>
			    	</div>
						
			    	<div id="radio_smartcon_enable" class="left" style="width: 94px;display:table-cell;"></div><div id="smartcon_rule_link" style="display:table-cell; vertical-align: middle;"><a href="Advanced_Smart_Connect.asp" style="font-family:Lucida Console;color:#FC0;text-decoration:underline;cursor:pointer;"><#smart_connect_rule#></a></div>
						<div class="clear"></div>					
						<script type="text/javascript">
								var flag = '<% get_parameter("flag"); %>';
								var smart_connect_flag_t = (flag=='')?document.form.smart_connect_x.value:flag;
								var _smart_connect_enable = ('<% nvram_get("smart_connect_x"); %>' != '0' ? '1' : '0');
								$('#radio_smartcon_enable').iphoneSwitch( smart_connect_flag_t > 0, 
								 function() {
									_smart_connect_enable = 1;
									if(document.form.smart_connect_t.value)
										handle_smart_connect(document.form.smart_connect_t.value);
									else
										handle_smart_connect(smart_connect_flag_t);
								 },
								 function() {
									if(document.form.band0_auth_mode_x.value == 'psk2sae'){
										document.form.band0_auth_mode_x.value = 'psk2';
										document.form.band0_mfp.value = '0';
									}
									if(document.form.band1_auth_mode_x.value == 'psk2sae'){
										document.form.band1_auth_mode_x.value = 'psk2';
										document.form.band1_mfp.value = '0';
									}
									_smart_connect_enable = 0;
									handle_smart_connect(0);
								 }
							);
						</script>
		  	  </td>
			</tr>
				<tr id="smart_connect_field" style="display:none;">                     
					<th><#smart_connect#></th>                                            
					<td id="smart_connect_switch">
					<select name="smart_connect_t" class="input_option" onChange="handle_smart_connect(this.value);">
						<option class="content_input_fd" value="1" <% nvram_match("smart_connect_x", "1","selected"); %> >Tri-band Smart Connect (2.4 GHz, 5 GHz and 6 GHz)</optio>
						<option class="content_input_fd" value="3" <% nvram_match("smart_connect_x", "3","selected"); %> >Dual-band Smart Connect (2.4 GHz and 5 GHz)</option>
					</select>                       
					</td>
				</tr>

				<tr id="band256_hide_ssid_field">
					<th><a class="hintstyle" href="javascript:void(0);" onClick="openHint(0, 2);"><#WLANConfig11b_x_BlockBCSSID_itemname#></a></th>
					<td>
						<input type="radio" value="1" name="wl256_closed" class="input" <% nvram_match("wl0_closed", "1", "checked"); %>><#checkbox_Yes#>
						<input type="radio" value="0" name="wl256_closed" class="input" <% nvram_match("wl0_closed", "0", "checked"); %>><#checkbox_No#>
					</td>
				</tr>

				<tr id="band012_ssid_field">
					<th><a class="hintstyle" href="javascript:void(0);" onClick="openHint(0, 1);"><#QIS_finish_wireless_item1#></a></th>
					<td>
						<input type="text" maxlength="32" class="input_32_table" id="band012_ssid" name="band012_ssid" value="<% nvram_get("wl0_ssid"); %>" onkeypress="return validator.isString(this, event)" autocorrect="off" autocapitalize="off">
					</td>
		  		</tr>
				<tr style="display:none">
					<th>
						<a class="hintstyle" href="javascript:void(0);"  onClick="openHint(2,1);"><#WLANAuthentication11a_ExAuthDBIPAddr_itemname#></a>
					</th>
					<td>
						<input type="text" maxlength="39" class="input_32_table" name="band012_radius_ipaddr" value='<% nvram_get("wl0_radius_ipaddr"); %>' onKeyPress="return validator.isIPAddr(this, event)" autocorrect="off" autocapitalize="off">
					</td>
				</tr>
				<tr style="display:none">
					<th>
						<a class="hintstyle" href="javascript:void(0);"  onClick="openHint(2,2);">
					  	<#WLANAuthentication11a_ExAuthDBPortNumber_itemname#></a>
					</th>
					<td>
						<input type="text" maxlength="5" class="input_6_table" name="band012_radius_port" value='<% nvram_get("wl0_radius_port"); %>' onkeypress="return validator.isNumber(this,event)" autocorrect="off" autocapitalize="off"/>
					</td>
				</tr>
				<tr style="display:none">
					<th >
						<a class="hintstyle" href="javascript:void(0);"  onClick="openHint(2,3);">
						<#WLANAuthentication11a_ExAuthDBPassword_itemname#></a>
					</th>
					<td>
						<input type="password" maxlength="64" class="input_32_table" name="band012_radius_key" value="<% nvram_get("wl0_radius_key"); %>" autocorrect="off" autocapitalize="off">
					</td>
				</tr>			  	
			  </table>

			<table id="band_separate" width="100%" border="1" align="center" cellpadding="4" cellspacing="0" bordercolor="#6b8fa3" class="FormTable">
				<thead>
					<tr id="band01_title_field">
						<td colspan="2">2.4 / 5 GHz</td>
					</tr>
					
				</thead>

				<tr id="band25_hide_ssid_field">
					<th><a class="hintstyle" href="javascript:void(0);" onClick="openHint(0, 2);"><#WLANConfig11b_x_BlockBCSSID_itemname#></a></th>
					<td>
						<input type="radio" value="1" name="wl25_closed" class="input" <% nvram_match("wl0_closed", "1", "checked"); %>><#checkbox_Yes#>
						<input type="radio" value="0" name="wl25_closed" class="input" <% nvram_match("wl0_closed", "0", "checked"); %>><#checkbox_No#>
					</td>
				</tr>

				<tr id="band01_ssid_field">
					<th><a class="hintstyle" href="javascript:void(0);" onClick="openHint(0, 1);"><#QIS_finish_wireless_item1#></a></th>
					<td>
						<input type="text" maxlength="32" class="input_32_table" name="band01_ssid" value="<% nvram_get("wl0_ssid"); %>" onkeypress="return validator.isString(this, event)" autocorrect="off" autocapitalize="off">
					</td>
				</tr>

				<tr id="band01_auth_method_field">
					<th>
						<a class="hintstyle" href="javascript:void(0);" onClick="openHint(0, 5);"><#WLANConfig11b_AuthenticationMethod_itemname#></a>
					</th>
					<td>
						<select name="band01_auth_mode_x" class="input_option select_auth_mode" onChange="auth_method_change('01', this.value);">
							<option value="open"    <% nvram_match("wl0_auth_mode_x", "open",   "selected"); %>>Open System</option>
							<option value="openowe" <% nvram_match("wl0_auth_mode_x", "openowe", "selected"); %>>Enhanced Open Transition</option>
							<option value="psk2"    <% nvram_match("wl0_auth_mode_x", "psk2",   "selected"); %>>WPA2-Personal</option>
							<option value="sae"    <% nvram_match("wl0_auth_mode_x", "sae",   "selected"); %>>WPA3-Personal</option>
							<option value="pskpsk2" <% nvram_match("wl0_auth_mode_x", "pskpsk2","selected"); %>>WPA/WPA2-Personal</option>
							<option value="psk2sae" <% nvram_match("wl0_auth_mode_x", "psk2sae","selected"); %>>WPA2/WPA3-Personal</option>
							<option value="wpa2"    <% nvram_match("wl0_auth_mode_x", "wpa2",   "selected"); %>>WPA2-Enterprise</option>
							<option value="wpawpa2" <% nvram_match("wl0_auth_mode_x", "wpawpa2","selected"); %>>WPA/WPA2-Enterprise</option>
						</select>
					</td>
				</tr>
				<tr id="band01_encrypt_field">
					<th><a class="hintstyle" href="javascript:void(0);" onClick="openHint(0, 6);"><#WLANConfig11b_WPAType_itemname#></a></th>
					<td>		
				  		<select name="band01_crypto" class="input_option">
							<option value="aes" <% nvram_match("wl0_crypto", "aes", "selected"); %>>AES</option>
							<option value="tkip+aes" <% nvram_match("wl0_crypto", "tkip+aes", "selected"); %>>TKIP+AES</option>
				  		</select>
					</td>
				</tr>
				<tr id="band01_psk_field">
					<th><a class="hintstyle" href="javascript:void(0);" onClick="openHint(0, 7);"><#WLANConfig11b_x_PSKKey_itemname#></a></th>
					<td>
				  		<input type="text" name="band01_wpa_psk" maxlength="64" class="input_32_table" oninput="handle_auth(this);" value="<% nvram_get("wl0_wpa_psk"); %>"  autocorrect="off" autocapitalize="off">
					</td>
				</tr>
				  
				<tr id="band01_radius_ip_field" style="display:none">
					<th>
						<a class="hintstyle" href="javascript:void(0);"  onClick="openHint(2,1);"><#WLANAuthentication11a_ExAuthDBIPAddr_itemname#></a>
					</th>
					<td>
						<input type="text" maxlength="39" class="input_32_table" name="band01_radius_ipaddr" value='<% nvram_get("wl0_radius_ipaddr"); %>' onKeyPress="return validator.isIPAddr(this, event)" autocorrect="off" autocapitalize="off">
					</td>
				</tr>
				<tr id="band01_radius_port_field" style="display:none">
					<th>
						<a class="hintstyle" href="javascript:void(0);"  onClick="openHint(2,2);"><#WLANAuthentication11a_ExAuthDBPortNumber_itemname#></a>
					</th>
					<td>
						<input type="text" maxlength="5" class="input_6_table" name="band01_radius_port" value='<% nvram_get("wl0_radius_port"); %>' onkeypress="return validator.isNumber(this,event)" autocorrect="off" autocapitalize="off"/>
					</td>
				</tr>
				<tr id="band01_radius_key_field" style="display:none">
					<th >
						<a class="hintstyle" href="javascript:void(0);"  onClick="openHint(2,3);"><#WLANAuthentication11a_ExAuthDBPassword_itemname#></a>
					</th>
					<td>
						<input type="password" maxlength="64" class="input_32_table" name="band01_radius_key" value="<% nvram_get("wl0_radius_key"); %>" autocorrect="off" autocapitalize="off">
					</td>
				</tr>
			
				<tr id="band01_mfp_field">
					<th><#WLANConfig11b_x_mfp#></th>
					<td>
				  		<select name="band01_mfp" class="input_option" onchange="">
							<option value="0" <% nvram_match("wl0_mfp", "0", "selected"); %>><#WLANConfig11b_WirelessCtrl_buttonname#></option>
							<option value="1" <% nvram_match("wl0_mfp", "1", "selected"); %>><#WLANConfig11b_x_mfp_opt1#></option>
							<option value="2" <% nvram_match("wl0_mfp", "2", "selected"); %>><#WLANConfig11b_x_mfp_opt2#></option>
						</select>
					</td>
				</tr>
				
				<tr id="band01_gtk_field">
					<th><a class="hintstyle" href="javascript:void(0);" onClick="openHint(0, 11);"><#WLANConfig11b_x_Rekey_itemname#></a></th>
					<td><input type="text" maxlength="7" name="band01_wpa_gtk_rekey" class="input_6_table"  value="<% nvram_get("wl0_wpa_gtk_rekey"); %>" onKeyPress="return validator.isNumber(this,event);" autocorrect="off" autocapitalize="off"></td>
				</tr>
				  







				<thead>
					<tr id="band0_title_field">
						<td colspan="2">2.4 GHz</td>
					</tr>
				</thead>

				<tr id="band2_hide_ssid_field">
					<th><a class="hintstyle" href="javascript:void(0);" onClick="openHint(0, 2);"><#WLANConfig11b_x_BlockBCSSID_itemname#></a></th>
					<td>
						<input type="radio" value="1" name="wl0_closed" class="input" <% nvram_match("wl0_closed", "1", "checked"); %>><#checkbox_Yes#>
						<input type="radio" value="0" name="wl0_closed" class="input" <% nvram_match("wl0_closed", "0", "checked"); %>><#checkbox_No#>
					</td>
				</tr>
				
				<tr id="band0_ssid_field">
					<th><a class="hintstyle" href="javascript:void(0);" onClick="openHint(0, 1);"><#QIS_finish_wireless_item1#></a></th>
					<td>
						<input type="text" maxlength="32" class="input_32_table" name="band0_ssid" value="<% nvram_get("wl0_ssid"); %>" onkeypress="return validator.isString(this, event)" autocorrect="off" autocapitalize="off">
					</td>
		  		</tr>
				<tr id="band0_bandwidth_field">
					<th><#WLANConfig11b_ChannelBW_itemname#></th>
					<td>
						<select name="band0_bw" class="input_option" onChange="handle_bandwidth(0, this.value)">
							<option value="0" <% nvram_match("wl0_bw", "0","selected"); %>>20/40 MHz</option>
							<option value="1" <% nvram_match("wl0_bw", "1","selected"); %>>20 MHz</option>
							<option value="2" <% nvram_match("wl0_bw", "2","selected"); %>>40 MHz</option>
						</select>
					</td>
				</tr>
				<tr id="band0_channel_field">
					<th><#WLANConfig11b_Channel_itemname#></th>
					<td>
						<select name="band0_channel" class="input_option" onChange="handle_channel(0, this.value);"></select>
						<span id="band0_autoChannel" style="display:none;margin-left:10px;">Current Control Channel</span><br>
						<span id="band0_acs_ch13"  style="display:none;"><input id="band0_acs_ch13_checkbox" type="checkbox" <% nvram_match("acs_ch13", "1" , "checked" ); %>><#WLANConfig11b_EChannel_acs_ch13#></span>
					</td>
				</tr>
				<tr id="band0_extChannel_field">
					<th>
						<a class="hintstyle" href="javascript:void(0);" onClick="openHint(0, 15);"><#WLANConfig11b_EChannel_itemname#></a>
					</th>
					<td>
						<select name="band0_extChannel" class="input_option"></select>
					</td>
				</tr>
				
				<tr id="band0_auth_method_field">
					<th>
						<a class="hintstyle" href="javascript:void(0);" onClick="openHint(0, 5);"><#WLANConfig11b_AuthenticationMethod_itemname#></a>
					</th>
					<td>
						<select name="band0_auth_mode_x" class="input_option select_auth_mode" onChange="auth_method_change('0', this.value);">
							<option value="open"    <% nvram_match("wl0_auth_mode_x", "open",   "selected"); %>>Open System</option>
							<option value="openowe" <% nvram_match("wl0_auth_mode_x", "openowe", "selected"); %>>Enhanced Open Transition</option>
							<option value="psk2"    <% nvram_match("wl0_auth_mode_x", "psk2",   "selected"); %>>WPA2-Personal</option>
							<option value="sae"    <% nvram_match("wl0_auth_mode_x", "sae",   "selected"); %>>WPA3-Personal</option>
							<option value="pskpsk2" <% nvram_match("wl0_auth_mode_x", "pskpsk2","selected"); %>>WPA/WPA2-Personal</option>
							<option value="psk2sae" <% nvram_match("wl0_auth_mode_x", "psk2sae","selected"); %>>WPA2/WPA3-Personal</option>
							<option value="wpa2"    <% nvram_match("wl0_auth_mode_x", "wpa2",   "selected"); %>>WPA2-Enterprise</option>
							<option value="wpawpa2" <% nvram_match("wl0_auth_mode_x", "wpawpa2","selected"); %>>WPA/WPA2-Enterprise</option>
						</select>
					</td>
				</tr>

				<tr id="band0_encrypt_field">
					<th><a class="hintstyle" href="javascript:void(0);" onClick="openHint(0, 6);"><#WLANConfig11b_WPAType_itemname#></a></th>
					<td>		
				  		<select name="band0_crypto" class="input_option">
							<option value="aes" <% nvram_match("wl0_crypto", "aes", "selected"); %>>AES</option>
							<option value="tkip+aes" <% nvram_match("wl0_crypto", "tkip+aes", "selected"); %>>TKIP+AES</option>
				  		</select>
					</td>
				</tr>
				  
				<tr id="band0_psk_field">
					<th><a class="hintstyle" href="javascript:void(0);" onClick="openHint(0, 7);"><#WLANConfig11b_x_PSKKey_itemname#></a></th>
					<td>
				  		<input type="text" name="band0_wpa_psk" maxlength="64" class="input_32_table" value="<% nvram_get("wl0_wpa_psk"); %>" autocorrect="off" autocapitalize="off">
					</td>
				</tr>
				  
				<tr id="band0_radius_ip_field">
					<th>
						<a class="hintstyle" href="javascript:void(0);"  onClick="openHint(2,1);"><#WLANAuthentication11a_ExAuthDBIPAddr_itemname#></a>
					</th>
					<td>
						<input type="text" maxlength="39" class="input_32_table" name="band0_radius_ipaddr" value='<% nvram_get("wl0_radius_ipaddr"); %>' onKeyPress="return validator.isIPAddr(this, event)" autocorrect="off" autocapitalize="off">
					</td>
				</tr>
				<tr id="band0_radius_port_field">
					<th>
						<a class="hintstyle" href="javascript:void(0);"  onClick="openHint(2,2);"><#WLANAuthentication11a_ExAuthDBPortNumber_itemname#></a>
					</th>
					<td>
						<input type="text" maxlength="5" class="input_6_table" name="band0_radius_port" value='<% nvram_get("wl0_radius_port"); %>' onkeypress="return validator.isNumber(this,event)" autocorrect="off" autocapitalize="off"/>
					</td>
				</tr>
				<tr id="band0_radius_key_field">
					<th >
						<a class="hintstyle" href="javascript:void(0);"  onClick="openHint(2,3);"><#WLANAuthentication11a_ExAuthDBPassword_itemname#></a>
					</th>
					<td>
						<input type="password" maxlength="64" class="input_32_table" name="band0_radius_key" value="<% nvram_get("wl0_radius_key"); %>" autocorrect="off" autocapitalize="off">
					</td>
				</tr>
			
				<tr id="band0_mfp_field">
					<th><#WLANConfig11b_x_mfp#></th>
					<td>
				  		<select name="band0_mfp" class="input_option" onchange="">
							<option value="0" <% nvram_match("wl0_mfp", "0", "selected"); %>><#WLANConfig11b_WirelessCtrl_buttonname#></option>
							<option value="1" <% nvram_match("wl0_mfp", "1", "selected"); %>><#WLANConfig11b_x_mfp_opt1#></option>
							<option value="2" <% nvram_match("wl0_mfp", "2", "selected"); %>><#WLANConfig11b_x_mfp_opt2#></option>
						</select>
					</td>
				</tr>
				
				<tr id="band0_gtk_field">
					<th><a class="hintstyle" href="javascript:void(0);" onClick="openHint(0, 11);"><#WLANConfig11b_x_Rekey_itemname#></a></th>
					<td><input type="text" maxlength="7" name="band0_wpa_gtk_rekey" class="input_6_table"  value="<% nvram_get("wl0_wpa_gtk_rekey"); %>" onKeyPress="return validator.isNumber(this,event);" autocorrect="off" autocapitalize="off"></td>
			  	</tr>

				<!-- 5 GHz -->
				<thead>
					<tr>
						<td id="5ghz_title" colspan="2">5 GHz</td>
					</tr>
				</thead>

				<tr id="band5_hide_ssid_field">
					<th><a class="hintstyle" href="javascript:void(0);" onClick="openHint(0, 2);"><#WLANConfig11b_x_BlockBCSSID_itemname#></a></th>
					<td>
						<input type="radio" value="1" name="wl1_closed" class="input" <% nvram_match("wl1_closed", "1", "checked"); %>><#checkbox_Yes#>
						<input type="radio" value="0" name="wl1_closed" class="input" <% nvram_match("wl1_closed", "0", "checked"); %>><#checkbox_No#>
					</td>
				</tr>

				<tr id="band1_ssid_field">
					<th><a class="hintstyle" href="javascript:void(0);" onClick="openHint(0, 1);"><#QIS_finish_wireless_item1#></a></th>
					<td>
						<input type="text" maxlength="32" class="input_32_table" name="band1_ssid" value="<% nvram_get("wl1_ssid"); %>" onkeypress="return validator.isString(this, event)" autocorrect="off" autocapitalize="off">
					</td>
		  		</tr>
				<tr>
					<th><#WLANConfig11b_ChannelBW_itemname#></th>
					<td>
						<select name="band1_bw" class="input_option" onChange="handle_bandwidth(1, this.value)">
							<option value="0" <% nvram_match("wl1_bw", "0", "selected"); %>>20/40/80/160 MHz</option>
							<option value="1" <% nvram_match("wl1_bw", "1", "selected"); %>>20 MHz</option>
							<option value="2" <% nvram_match("wl1_bw", "2", "selected"); %>>40 MHz</option>
							<option value="3" <% nvram_match("wl1_bw", "3", "selected"); %>>80 MHz</option>
							<option value="5" <% nvram_match("wl1_bw", "5", "selected"); %>>160 MHz</option>
						</select>
						<span id="band1_160_field" style="display:none"><input id="band1_160" type="checkbox" onClick="separateEnable_160MHz(this);" <% nvram_match("wl1_bw_160", "1" , "checked" ); %>><#WLANConfig11b_ChannelBW_Enable160M#></span>
					</td>
				</tr>
				<tr>
					<th><#WLANConfig11b_Channel_itemname#></th>
					<td>
						<select name="band1_channel" class="input_option" onChange="handle_channel('1', this.value);"></select>
						<span id="band1_autoChannel" style="display:none;margin-left:10px;">Current Control Channel</span><br>
						<span id="band1_acsDFS"><input id="band1_acsDFS_checkbox" type="checkbox" <% nvram_match("acs_dfs", "1" , "checked" ); %>><#WLANConfig11b_EChannel_dfs#></span>
						<div><span id="acs_unii4_field" style="display:none;"><input id="acs_unii4_checkbox" type="checkbox" onClick="handleUNII4Hint(this.checked)" <% nvram_match("acs_unii4", "1", "checked"); %>><#WLANConfig11b_EChannel_U-NII-4#></span></div>
					</td>
				</tr>
				<tr id="band1_extChannel_field">
					<th>
						<a class="hintstyle" href="javascript:void(0);" onClick="openHint(0, 15);"><#WLANConfig11b_EChannel_itemname#></a>
					</th>
					<td>
						<select class="input_option" name="band1_extChannel">
							<option value=""><#Auto#></option>
						</select>
					</td>
				</tr>

				<tr id="band1_auth_method_field">
					<th>
						<a class="hintstyle" href="javascript:void(0);" onClick="openHint(0, 5);"><#WLANConfig11b_AuthenticationMethod_itemname#></a>
					</th>
					<td>
						<select name="band1_auth_mode_x" class="input_option select_auth_mode" onChange="auth_method_change('1', this.value);">
							<option value="open"    <% nvram_match("wl1_auth_mode_x", "open",   "selected"); %>>Open System</option>
							<option value="openowe" <% nvram_match("wl1_auth_mode_x", "openowe", "selected"); %>>Enhanced Open Transition</option>
							<option value="psk2"    <% nvram_match("wl1_auth_mode_x", "psk2",   "selected"); %>>WPA2-Personal</option>
							<option value="sae"    <% nvram_match("wl1_auth_mode_x", "sae",   "selected"); %>>WPA3-Personal</option>
							<option value="pskpsk2" <% nvram_match("wl1_auth_mode_x", "pskpsk2","selected"); %>>WPA/WPA2-Personal</option>
							<option value="psk2sae" <% nvram_match("wl1_auth_mode_x", "psk2sae","selected"); %>>WPA2/WPA3-Personal</option>
							<option value="wpa2"    <% nvram_match("wl1_auth_mode_x", "wpa2",   "selected"); %>>WPA2-Enterprise</option>
							<option value="wpawpa2" <% nvram_match("wl1_auth_mode_x", "wpawpa2","selected"); %>>WPA/WPA2-Enterprise</option>
						</select>
					</td>
				</tr>

				<tr id="band1_encrypt_field">
					<th><a class="hintstyle" href="javascript:void(0);" onClick="openHint(0, 6);"><#WLANConfig11b_WPAType_itemname#></a></th>
					<td>		
				  		<select name="band1_crypto" class="input_option">
							<option value="aes" <% nvram_match("wl1_crypto", "aes", "selected"); %>>AES</option>
							<option value="tkip+aes" <% nvram_match("wl1_crypto", "tkip+aes", "selected"); %>>TKIP+AES</option>
				  		</select>
					</td>
				</tr>
				  
				<tr id="band1_psk_field">
					<th><a class="hintstyle" href="javascript:void(0);" onClick="openHint(0, 7);"><#WLANConfig11b_x_PSKKey_itemname#></a></th>
					<td>
				  		<input type="text" name="band1_wpa_psk" maxlength="64" class="input_32_table" value="<% nvram_get("wl1_wpa_psk"); %>" autocorrect="off" autocapitalize="off">
					</td>
				</tr>
				  
				<tr id="band1_radius_ip_field">
					<th>
						<a class="hintstyle" href="javascript:void(0);"  onClick="openHint(2,1);"><#WLANAuthentication11a_ExAuthDBIPAddr_itemname#></a>
					</th>
					<td>
						<input type="text" maxlength="39" class="input_32_table" name="band1_radius_ipaddr" value='<% nvram_get("wl1_radius_ipaddr"); %>' onKeyPress="return validator.isIPAddr(this, event)" autocorrect="off" autocapitalize="off">
					</td>
				</tr>
				<tr id="band1_radius_port_field">
					<th>
						<a class="hintstyle" href="javascript:void(0);"  onClick="openHint(2,2);"><#WLANAuthentication11a_ExAuthDBPortNumber_itemname#></a>
					</th>
					<td>
						<input type="text" maxlength="5" class="input_6_table" name="band1_radius_port" value='<% nvram_get("wl1_radius_port"); %>' onkeypress="return validator.isNumber(this,event)" autocorrect="off" autocapitalize="off"/>
					</td>
				</tr>
				<tr id="band1_radius_key_field">
					<th >
						<a class="hintstyle" href="javascript:void(0);"  onClick="openHint(2,3);"><#WLANAuthentication11a_ExAuthDBPassword_itemname#></a>
					</th>
					<td>
						<input type="password" maxlength="64" class="input_32_table" name="band1_radius_key" value="<% nvram_get("wl1_radius_key"); %>" autocorrect="off" autocapitalize="off">
					</td>
				</tr>

				
				<tr id="band1_mfp_field">
					<th><#WLANConfig11b_x_mfp#></th>
					<td>
				  		<select name="band1_mfp" class="input_option" onchange="">
							<option value="2" <% nvram_match("wl1_mfp", "2", "selected"); %>><#WLANConfig11b_x_mfp_opt2#></option>
						</select>
					</td>
				</tr>
				
				<tr id="band1_gtk_field">
					<th><a class="hintstyle" href="javascript:void(0);" onClick="openHint(0, 11);"><#WLANConfig11b_x_Rekey_itemname#></a></th>
					<td><input type="text" maxlength="7" name="band1_wpa_gtk_rekey" class="input_6_table"  value="<% nvram_get("wl1_wpa_gtk_rekey"); %>" onKeyPress="return validator.isNumber(this,event);" autocorrect="off" autocapitalize="off"></td>
			  	</tr>
				<!-- 6 GHz -->
				<thead>
					<tr id="band2_title_field" style="">
						<td id="5g2_title" colspan="2">6 GHz</td>
					</tr>
				</thead>

				<tr id="band6_hide_ssid_field">
					<th><a class="hintstyle" href="javascript:void(0);" onClick="openHint(0, 2);"><#WLANConfig11b_x_BlockBCSSID_itemname#></a></th>
					<td>
						<input type="radio" value="1" name="wl2_closed" class="input" <% nvram_match("wl2_closed", "1", "checked"); %>><#checkbox_Yes#>
						<input type="radio" value="0" name="wl2_closed" class="input" <% nvram_match("wl2_closed", "0", "checked"); %>><#checkbox_No#>
					</td>
				</tr>

				<tr id="band2_ssid_field">
					<th><a class="hintstyle" href="javascript:void(0);" onClick="openHint(0, 1);"><#QIS_finish_wireless_item1#></a></th>
					<td>
						<input type="text" maxlength="32" class="input_32_table" name="band2_ssid" value="<% nvram_get("wl2_ssid"); %>" onkeypress="return validator.isString(this, event)" autocorrect="off" autocapitalize="off">
					</td>
		  		</tr>
				<tr id="band2_bandwidth_field" style="">
					<th><#WLANConfig11b_ChannelBW_itemname#></th>
					<td>
						<select name="band2_bw" class="input_option" onChange="handle_bandwidth(2, this.value);">
							<option value="0" <% nvram_match("wl2_bw", "0", "selected"); %>>20/40/80/160 MHz</option>
							<option value="1" <% nvram_match("wl2_bw", "1", "selected"); %>>20 MHz</option>
							<option value="2" <% nvram_match("wl2_bw", "2", "selected"); %>>40 MHz</option>
							<option value="3" <% nvram_match("wl2_bw", "3", "selected"); %>>80 MHz</option>
							<option value="5" <% nvram_match("wl2_bw", "5", "selected"); %>>160 MHz</option>
						</select>
					</td>
				</tr>
				<tr id="band2_channel_field" style="">
					<th><#WLANConfig11b_Channel_itemname#></th>
					<td>
						<select name="band2_channel" class="input_option" onChange="handle_channel('2', this.value);"></select>
						<span id="band2_autoChannel" style="display:none;margin-left:10px;">Current Control Channel</span><br>
						<span id="band2_psc6g" style="">
							<input id="band2_psc6g_checkbox" type="checkbox" onclick="separateGenChannel('2', document.form.band2_channel.value, document.form.band2_bw.value);" <% nvram_match("psc6g", "1" , "checked" ); %>><#Enable_PSC_Hint#> <#PSC_Faq#>
						</span>						
					</td>
				</tr>
				<tr id="band2_extChannel_field" style="">
					<th>
						<a class="hintstyle" href="javascript:void(0);" onClick="openHint(0, 15);"><#WLANConfig11b_EChannel_itemname#></a>
					</th>
					<td>
						<select class="input_option" name="band2_extChannel">
							<option value=""><#Auto#></option>
						</select>
					</td>
				</tr>

				<tr id="band2_auth_method_field">
					<th>
						<a class="hintstyle" href="javascript:void(0);" onClick="openHint(0, 5);"><#WLANConfig11b_AuthenticationMethod_itemname#></a>
					</th>
					<td>
						<select name="band2_auth_mode_x" class="input_option select_auth_mode" onChange="auth_method_change('2', this.value);">
							<option value="owe" <% nvram_match("wl2_auth_mode_x", "owe",   "selected"); %>><#Wireless_Encryption_OWE#></option>
							<option value="sae" <% nvram_match("wl2_auth_mode_x", "sae",   "selected"); %>>WPA3-Personal</option>
						</select>
					</td>
				</tr>

				<tr id="band2_encrypt_field">
					<th><a class="hintstyle" href="javascript:void(0);" onClick="openHint(0, 6);"><#WLANConfig11b_WPAType_itemname#></a></th>
					<td>		
				  		<select name="band2_crypto" class="input_option">
							<option value="aes" <% nvram_match("wl2_crypto", "aes", "selected"); %>>AES</option>
				  		</select>
					</td>
				</tr>
				  
				<tr id="band2_psk_field">
					<th><a class="hintstyle" href="javascript:void(0);" onClick="openHint(0, 7);"><#WLANConfig11b_x_PSKKey_itemname#></a></th>
					<td>
				  		<input type="text" name="band2_wpa_psk" maxlength="64" class="input_32_table" oninput="handle_auth(this);" value="<% nvram_get("wl2_wpa_psk"); %>" autocorrect="off" autocapitalize="off">
					</td>
			  	</tr>
				
				<tr id="band2_mfp_field">
					<th><#WLANConfig11b_x_mfp#></th>
					<td>
				  		<select name="band2_mfp" class="input_option" onchange="">
							<option value="2" <% nvram_match("wl2_mfp", "2", "selected"); %>><#WLANConfig11b_x_mfp_opt2#></option>
						</select>
					</td>
				</tr>
				
				<tr id="band2_gtk_field">
					<th><a class="hintstyle" href="javascript:void(0);" onClick="openHint(0, 11);"><#WLANConfig11b_x_Rekey_itemname#></a></th>
					<td><input type="text" maxlength="7" name="band2_wpa_gtk_rekey" class="input_6_table"  value="<% nvram_get("wl2_wpa_gtk_rekey"); %>" onKeyPress="return validator.isNumber(this,event);" autocorrect="off" autocapitalize="off"></td>
			  	</tr>



			</table>
			  
				<div class="apply_gen">
					<input type="button" id="applyButton" class="button_gen" value="<#CTL_apply#>" onclick="applyRule();">
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
	<td width="10" align="center" valign="top"></td>
  </tr>
</table>

<div id="footer"></div>
<script>
(function() {
	// special case after modifing GuestNetwork
	// case 1 is after enable GuestNetwork, case 2 is after disable GuestNetwork
	if(isSwMode("rt") || isSwMode("ap")) {
		if('<% nvram_get("wl_unit"); %>' == "-1" || '<% nvram_get("wl_subunit"); %>' != "-1") {
			httpApi.nvramSet({
				'action_mode': 'apply',
				'rc_service': 'restart_wireless',
				'wl_unit': '0',
				'wl_subunit': '-1'
			}, function(){
				location.reload();
			});
		}

	}
})();
</script>
</body>
</html>
