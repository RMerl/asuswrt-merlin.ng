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
<script type="text/javascript" src="/state.js"></script>
<script type="text/javascript" src="/help.js"></script>
<script type="text/javascript" src="/general.js"></script>
<script type="text/javascript" src="/popup.js"></script>
<script type="text/javascript" src="/md5.js"></script>
<script type="text/javascript" src="/validator.js"></script>
<script type="text/javascript" src="/js/jquery.js"></script>
<script type="text/javascript" src="/switcherplugin/jquery.iphone-switch.js"></script>
<script type="text/javascript" src="/js/httpApi.js"></script>
<script><% wl_get_parameter(); %>

$(function () {
	if(amesh_support && (isSwMode("rt") || isSwMode("ap")) && ameshRouter_support) {
		addNewScript('/require/modules/amesh.js');
	}
});

wl_channel_list_2g = '<% channel_list_2g(); %>';
wl_channel_list_5g = '<% channel_list_5g(); %>';
wl_channel_list_5g_2 = '<% channel_list_5g_2(); %>';
wl_channel_list_60g = '<% channel_list_60g(); %>';
var wl_unit_value = '<% nvram_get("wl_unit"); %>';
var wl_subunit_value = '<% nvram_get("wl_subunit"); %>';
var wlc_band_value = '<% nvram_get("wlc_band"); %>';
var cur_control_channel = [<% wl_control_channel(); %>][0];
var cur_edmg_channel = [<% wl_edmg_channel(); %>][0];
var wlc0_ssid = '<% nvram_get("wlc0_ssid"); %>';
var wlc1_ssid = '<% nvram_get("wlc1_ssid"); %>';
var wifison_ready = httpApi.nvramGet(["wifison_ready"]).wifison_ready;
var wl_bw_160 = '<% nvram_get("wl_bw_160"); %>';
var enable_bw_160 = (wl_bw_160 == 1) ? true : false;

function initial(){
	show_menu();
	if (he_frame_support) {
		$("#he_mode_field").show();
		$("#he_mode_faq_link")  //for string tag: WLANConfig11b_HE_Frame_Mode_faq
            .attr('target', '_blank')
            .attr('style', 'color:#FC0;text-decoration:underline;')
            .attr('href', 'https://www.asus.com/support/FAQ/1037422/');
	}

	if(vht160_support){
		$("#enable_160mhz").attr("checked", enable_bw_160);
	}

	wireless_mode_change(document.form.wl_nmode_x);
	regen_band(document.form.wl_unit);
	regen_5G_mode(document.form.wl_nmode_x, wl_unit);
	if(lantiq_support){
		checkWLReady();
	}
	
	if((isSwMode("re") || isSwMode("mb")) && (wl_unit_value == wlc_band_value) && wl_subunit_value != '1' && !concurrep_support){
		_change_wl_unit(wl_unit_value);
	}
	
	if(isSwMode("re") && concurrep_support && wl_subunit_value != '1'){
		_change_wl_unit(wl_unit_value);
	}
	
	if(isSwMode("ew")){
		if(wlc_express == "1"){
			document.form.wl_unit.innerHTML = '<option class="content_input_fd" value="1" selected="">5 GHz</option>';
			if(wl_unit_value != 1) _change_wl_unit();
		}
		else if(wlc_express == "2"){
			document.form.wl_unit.innerHTML = '<option class="content_input_fd" value="0" selected="">2.4 GHz</option>';
			if(wl_unit_value != 0) _change_wl_unit();
		}
	}

	if(band5g_support && band5g_11ac_support && document.form.wl_unit.value >= 1){
		document.getElementById('wl_mode_desc').onclick=function(){return openHint(1, 5)};
	}else if(band5g_support && document.form.wl_unit.value >= 1){
		document.getElementById('wl_mode_desc').onclick=function(){return openHint(1, 4)};
	}

	if(!(band5g_support && band5g_11ac_support && document.form.wl_unit.value >= 1)){
		document.form.wl_nmode_x.remove(3); //remove "N/AC Mixed" for NON-AC router and NOT in 5G
	}

	if(vht160_support && wl_unit_value != '0' && wl_unit_value != '3'){
		document.getElementById('enable_160_field').style.display = "";
	}

	if('<% nvram_get("wl_nmode_x"); %>' == "2")
			inputCtrl(document.form.wl_bw, 0);

	if(wl_unit_value == '0')
		check_channel_2g();
	else if(wl_unit_value == '3')
		insertChannelOption_60g();
	else
		insertExtChannelOption_5g();

	if(isSwMode("re")){
		if((wlc0_ssid != "" && wl_unit_value == "0") || (wlc1_ssid != "" && wl_unit_value == "1")){
			document.getElementById('wl_bw_field').style.display = "none";
			document.getElementById('wl_channel_field').style.display = "none";
			document.getElementById('wl_nctrlsb_field').style.display = "none";
		}
	}

	limit_auth_method();	
	wl_auth_mode_change(1);
	//mbss_display_ctrl();
	if(optimizeXbox_support){
		document.getElementById("wl_optimizexbox_span").style.display = "";
		document.form.wl_optimizexbox_ckb.checked = ('<% nvram_get("wl_optimizexbox"); %>' == 1) ? true : false;
	}
	
	document.form.wl_ssid.value = decodeURIComponent('<% nvram_char_to_ascii("", "wl_ssid"); %>');
	document.form.wl_wpa_psk.value = decodeURIComponent('<% nvram_char_to_ascii("", "wl_wpa_psk"); %>');
	document.form.wl_key1.value = decodeURIComponent('<% nvram_char_to_ascii("", "wl_key1"); %>');
	document.form.wl_key2.value = decodeURIComponent('<% nvram_char_to_ascii("", "wl_key2"); %>');
	document.form.wl_key3.value = decodeURIComponent('<% nvram_char_to_ascii("", "wl_key3"); %>');
	document.form.wl_key4.value = decodeURIComponent('<% nvram_char_to_ascii("", "wl_key4"); %>');
	document.form.wl_phrase_x.value = decodeURIComponent('<% nvram_char_to_ascii("", "wl_phrase_x"); %>');
	document.form.wl_channel.value = document.form.wl_channel_orig.value;
	if (band60g_support && document.form.wl_unit.value == '3')
		document.form.wl_edmg_channel.value = document.form.wl_edmg_channel_orig.value;
	
	if(document.form.wl_wpa_psk.value.length <= 0)
		document.form.wl_wpa_psk.value = "<#wireless_psk_fillin#>";

	if(document.form.wl_unit[0].selected == true)
		document.getElementById("wl_gmode_checkbox").style.display = "";

	if(document.form.wl_nmode_x.value=='1'){
		document.form.wl_gmode_check.checked = false;
		document.getElementById("wl_gmode_check").disabled = true;
	}
	else{
		document.form.wl_gmode_check.checked = true;
		document.getElementById("wl_gmode_check").disabled = false;
	}
	if(document.form.wl_gmode_protection.value == "auto")
		document.form.wl_gmode_check.checked = true;
	else
		document.form.wl_gmode_check.checked = false;

	if(!band5g_support)	
		document.getElementById("wl_unit_field").style.display = "none";

	handle_11ac_80MHz();
	genBWTable('<% nvram_get("wl_unit"); %>');

	if(isSwMode("re") || isSwMode("mb"))
		document.form.wl_subunit.value = (wl_unit_value == wlc_band_value) ? 1 : -1;	
	
	document.getElementById('WPS_hideSSID_hint').innerHTML = "<#WPS_hideSSID_hint#>";	
	if("<% nvram_get("wl_closed"); %>" == 1 && (isSwMode("rt") || isSwMode("ap"))){
		document.getElementById('WPS_hideSSID_hint').style.display = "";	
	}

	if(band60g_support && wl_unit_value == '3'){//60G, remove unsupported items and show wigig items
		document.getElementById("wl_closed_field").style.display = "none";
		inputCtrl(document.form.wl_nmode_x, 0);
		inputCtrl(document.form.wl_nctrlsb, 0);
		if(he_frame_support){
			$("#he_mode_field").hide();
		}	

		document.getElementById("wl_edmg_field").style.display = "";

		if (document.form.wl_edmg_channel.value == '0' && cur_edmg_channel && cur_edmg_channel[wl_unit_value] != '0'){
			ajax_wl_edmg_channel();
			document.getElementById("auto_edmg_channel").style.display = "";
			document.getElementById("auto_edmg_channel").innerHTML = "Current EDMG channel: " + cur_edmg_channel[wl_unit_value];
		}
	}else{
		document.getElementById("wl_edmg_field").style.display = "none";
	}
	
	if(document.form.wl_channel.value == '0' && cur_control_channel){
		ajax_wl_channel();
		document.getElementById("auto_channel").style.display = "";
		document.getElementById("auto_channel").innerHTML = "Current control channel: " + cur_control_channel[wl_unit_value];
	}

	if(concurrep_support && (isSwMode("re") || isSwMode("ew"))){
		inputCtrl(document.form.wl_nmode_x, 0);
		document.form.wl_subunit.disabled = false;
		document.form.wl_subunit.value = 1;
	}

	var skip_channel_2g = '<% nvram_get("skip_channel_2g"); %>';
	var skip_channel_5g = '<% nvram_get("skip_channel_5g"); %>';

	if(skip_channel_2g == "CH13" && wl_unit_value == "0"){
		document.getElementById("acs_ch13_checkbox").style = "";
	}

	if(skip_channel_5g == "band1" && wl_unit_value == "1"){
		document.getElementById("acs_band1_checkbox").style = "";
	}
	else if(skip_channel_5g == "band3" && wl_unit_value == "1"){
		document.getElementById("acs_band3_checkbox").style = "";
	}
	else if(skip_channel_5g == "band23" && wl_unit_value == "1"){
		document.getElementById("dfs_checkbox").style = "";
	}

	if(Qcawifi_support && document.form.wl_channel.value  == '0'){
		if((wl_unit == '1' && has_dfs_channel(wl_channel_list_5g)) || (wl_unit == '2' && has_dfs_channel(wl_channel_list_5g_2))){
			document.getElementById('dfs_checkbox').style.display = "";
			check_DFS_support(document.form.acs_dfs_checkbox);
		}
	}

	if(smart_connect_support && (isSwMode("rt") || isSwMode("ap"))){
		var flag = '<% get_parameter("flag"); %>';		
		var smart_connect_flag_t;
		document.getElementById("smartcon_enable_field").style.display = "";
		if(flag == '')
			smart_connect_flag_t = '<% nvram_get("smart_connect_x"); %>';
		else
			smart_connect_flag_t = flag;	

		document.form.smart_connect_x.value = smart_connect_flag_t;
		if(smart_connect_flag_t == 0)
			document.form.smart_connect_t.value = 1;
		else    
			document.form.smart_connect_t.value = smart_connect_flag_t;

		enableSmartCon(smart_connect_flag_t);
	}

	if(wifison_ready == "1")
		document.getElementById("wl_unit_field").style.display = "none";

	if(is_RU_sku){
		var ch_orig = parseInt(document.form.wl_channel_orig.value);
		var _ch = '0';
		var _array = [36, 44, 52, 60, 100, 108, 116, 124, 132, 140, 149, 157];
		if(document.form.wl_nmode_x.value == 0 || document.form.wl_nmode_x.value == 8){    // Auto or N/AC mixed
			if(document.form.wl_bw.value == 3){    // 80 MHz		
				for(i=0; i<_array.length; i+=2){
					if(ch_orig >= _array[i] && ch_orig <= (_array[i]+12)){
						_ch = _array[i];
					}
				}
			}
			else if(document.form.wl_bw.value == 2){    // 40 MHz
				for(i=0; i<_array.length; i++){
					if(ch_orig >= _array[i] && ch_orig <= (_array[i]+4)){
						_ch = _array[i];
					}
				}
			}
			
			document.form.wl_channel.value = _ch;
		}
	}	
}

function genBWTable(_unit){
	if (!Rawifi_support && !Qcawifi_support && based_modelid != "BLUECAVE")
		return;

	cur = '<% nvram_get("wl_bw"); %>';
	var bws = new Array();
	var bwsDesc = new Array();
	var array_80m = new Array();
	var array_160m = new Array();

	if(document.form.wl_nmode_x.value == 2){
		if(based_modelid == "BLUECAVE"){
			bws = [1];
			bwsDesc = ["20 MHz"];
		}
		else{
			bws = [0];
			bwsDesc = ["20 MHz"];			
		}

		inputCtrl(document.form.wl_bw,1);
		document.getElementById("wl_bw_field").style.display = "none";
	}
	else if(_unit == 0 || (_unit != 0 && document.form.wl_nmode_x.value == 1)){// 2G or 5G N only
		if(based_modelid == "BLUECAVE"){
			bws = [0, 1, 2];
			bwsDesc = ["20/40 MHz", "20 MHz", "40 MHz"];
		}
		else{
			bws = [1, 0, 2];
			bwsDesc = ["20/40 MHz", "20 MHz", "40 MHz"];			
		}

		document.getElementById("wl_bw_field").style.display = "";
	}
	else if (band60g_support && _unit == 3){
		var ary = [], auto = [1], autoDesc = ["2.16"];
		var bws = [6], bwsDesc = ["2.16 GHz"];
		var ch_list = eval('<% channel_list_60g(); %>');

		/* Generate all possible bandwidth */
		for (var i = 7; i <= max_band60g_wl_bw; ++i) {
			if ((wigig_bw = wl_bw_to_wigig_bw(i)) <= 2160)
				continue;
			ary = filter_60g_edmg_channel_by_bw(ch_list, wigig_bw);
			if (!ary.length)
				continue;
			bws.push(i);
			bwsDesc.push((wigig_bw / 1000) + " GHz");
			autoDesc[0] = autoDesc[0] + "/" + (wigig_bw / 1000);
		}
		autoDesc[0] += " GHz";
		if (bws.length > 1) {
			bws = auto.concat(bws);
			bwsDesc = autoDesc.concat(bwsDesc);
		}

		if (bws.indexOf(parseInt(cur)) == -1)
			cur = bws[0];
		document.getElementById("wl_bw_field").style.display = "";
	}
	else{
		if(based_modelid == "BLUECAVE"){
			bws = [0, 1, 2, 3];
			bwsDesc = ["20/40/80 MHz", "20 MHz", "40 MHz", "80 MHz"];
		}
		else{
			bws = [1, 0, 2, 3];
			bwsDesc = [document.form.wl_bw[0].text, "20 MHz", "40 MHz", "80 MHz"];
		}

		if(document.form.wl_nmode_x.value == 8 || (_unit != 0 && document.form.wl_nmode_x.value == 0)){// N/AC mixed or 5G Auto
			if(isArray(wl_channel_list_5g)){
				array_80m = filter_5g_channel_by_bw(wl_channel_list_5g, 80);
				array_160m = filter_5g_channel_by_bw(wl_channel_list_5g, 160);
			}else{
				start = wl_channel_list_5g.lastIndexOf("[");
				end = wl_channel_list_5g.indexOf("]");
				if (end == -1)
					end = wl_channel_list_5g.length;
				ch = wl_channel_list_5g.slice(start + 1, end);
				array_80m = filter_5g_channel_by_bw(ch.split(","), 80);
				array_160m = filter_5g_channel_by_bw(ch.split(","), 160);
			}
			
			if(vht80_80_support && array_80m.length/4 >= 2){
				bws.push(4);
				bwsDesc.push("80+80 MHz");
			}
			if(vht160_support && array_160m.length/4 >= 1 && enable_bw_160){
				bwsDesc[0] = "20/40/80/160 MHz";
				bws.push(5);
				bwsDesc.push("160 MHz");
			}
		}

		document.getElementById("wl_bw_field").style.display = "";
	}

	add_options_x2(document.form.wl_bw, bwsDesc, bws, cur);
	if (band60g_support && _unit == 3)
		insertChannelOption_60g();
}

function check_channel_2g(){
	var wmode = document.form.wl_nmode_x.value;
	var CurrentCh = document.form.wl_channel_orig.value;
	if(is_high_power && auto_channel == 1){
		CurrentCh = document.form.wl_channel_orig.value = 0;
	}
	
	wl_channel_list_2g = eval('<% channel_list_2g(); %>');
	if(wl_channel_list_2g[0] != "<#Auto#>")
  		wl_channel_list_2g.splice(0,0,"0");
		
	var ch_v2 = new Array();
    for(var i=0; i<wl_channel_list_2g.length; i++){
        ch_v2[i] = wl_channel_list_2g[i];
    }
	
    if(ch_v2[0] == "0")
        wl_channel_list_2g[0] = "<#Auto#>";	

	add_options_x2(document.form.wl_channel, wl_channel_list_2g, ch_v2, CurrentCh);
	var option_length = document.form.wl_channel.options.length;	
	if(wmode == "0"||wmode == "1"){
		if((lantiq_support && document.form.wl_bw.value != "1") || (!lantiq_support && document.form.wl_bw.value != "0")){
			inputCtrl(document.form.wl_nctrlsb, 1);
			var x = document.form.wl_nctrlsb;
			var length = document.form.wl_nctrlsb.options.length;
			if (length > 1){
				x.selectedIndex = 1;
				x.remove(x.selectedIndex);
			}
			
			if ((CurrentCh >=1) && (CurrentCh <= 4)){
				x.options[0].text = "<#WLANConfig11b_EChannelAbove#>";
				x.options[0].value = "lower";
			}
			else if ((CurrentCh >= 5) && (CurrentCh <= 7)){
				x.options[0].text = "<#WLANConfig11b_EChannelAbove#>";
				x.options[0].value = "lower";
				add_option(document.form.wl_nctrlsb, "<#WLANConfig11b_EChannelBelow#>", "upper");
				if (document.form.wl_nctrlsb_old.value == "upper")
					document.form.wl_nctrlsb.options.selectedIndex=1;
					
				if(is_high_power && CurrentCh == 5) // for high power model, Jieming added at 2013/08/19
					document.form.wl_nctrlsb.remove(1);
				else if(is_high_power && CurrentCh == 7)
					document.form.wl_nctrlsb.remove(0);	
			}
			else if ((CurrentCh >= 8) && (CurrentCh <= 10)){
				x.options[0].text = "<#WLANConfig11b_EChannelBelow#>";
				x.options[0].value = "upper";
				if (option_length >=14){
					add_option(document.form.wl_nctrlsb, "<#WLANConfig11b_EChannelAbove#>", "lower");
					if (document.form.wl_nctrlsb_old.value == "lower")
						document.form.wl_nctrlsb.options.selectedIndex=1;
				}
			}
			else if (CurrentCh >= 11){
				x.options[0].text = "<#WLANConfig11b_EChannelBelow#>";
				x.options[0].value = "upper";
			}
			else{
				x.options[0].text = "<#Auto#>";
				x.options[0].value = "1";
			}
		}
		else{
			inputCtrl(document.form.wl_nctrlsb, 0);
		}
	}
	else{
		inputCtrl(document.form.wl_nctrlsb, 0);
	}
}

function mbss_display_ctrl(){
	// generate options
	if(multissid_support){
		for(var i=1; i<multissid_count+1; i++)
			add_options_value(document.form.wl_subunit, i, wl_subunit_value);
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

function applyRule(){
	if(lantiq_support && wave_ready != 1){
		alert("Please wait a minute for wireless ready");
		return false;
	}

	var auth_mode = document.form.wl_auth_mode_x.value;
	if(document.form.wl_wpa_psk.value == "<#wireless_psk_fillin#>")
		document.form.wl_wpa_psk.value = "";

	if(validForm()){
		if(amesh_support && (isSwMode("rt") || isSwMode("ap")) && ameshRouter_support) {
			if(!check_wl_auth_support(auth_mode, $("select[name=wl_auth_mode_x] option:selected")))
				return false;
			else {
				var wl_parameter = {
					"original" : {
						"ssid" : decodeURIComponent('<% nvram_char_to_ascii("", "wl_ssid"); %>'),
						"psk" : decodeURIComponent('<% nvram_char_to_ascii("", "wl_wpa_psk"); %>')
					},
					"current": {
						"ssid" : document.form.wl_ssid.value,
						"psk" : document.form.wl_wpa_psk.value
					}
				};
				if(!AiMesh_confirm_msg("Wireless_SSID_PSK", wl_parameter))
					return false;
			}
			var radio_value = (document.form.wl_closed[0].checked) ? 1 : 0;
			if(document.form.wps_enable.value == 1) {
				if(radio_value) {
					if(!AiMesh_confirm_msg("Wireless_Hide_WPS", radio_value))
						return false;
					document.form.wps_enable.value = "0";
				}
			}
			else {
				if(!AiMesh_confirm_msg("Wireless_Hide", radio_value))
					return false;
			}
		}
		else {
			if(document.form.wl_closed[0].checked && document.form.wps_enable.value == 1 && (isSwMode("rt") || isSwMode("ap"))){
				if(confirm("<#wireless_JS_Hide_SSID#>"))
					document.form.wps_enable.value = "0";
				else
					return false;
			}
		}

		if(document.form.wps_enable.value == 1){		//disable WPS if choose WEP or WPA/TKIP Encryption
			if(wps_multiband_support && (document.form.wps_multiband.value == 1	|| document.form.wps_band.value == wl_unit_value)){		//Ralink, Qualcomm Atheros
				if(document.form.wl_auth_mode_x.value == "open" && document.form.wl_wep_x.value == "0"){
					if(!confirm("<#wireless_JS_WPS_open#>"))
						return false;		
				}
		
				if( document.form.wl_auth_mode_x.value == "shared"
				 ||	document.form.wl_auth_mode_x.value == "psk" || document.form.wl_auth_mode_x.value == "wpa"
				 || document.form.wl_auth_mode_x.value == "open" && (document.form.wl_wep_x.value == "1" || document.form.wl_wep_x.value == "2")){		//open wep case			
					if(confirm("<#wireless_JS_disable_WPS#>")){
						document.form.wps_enable.value = "0";	
					}
					else{	
						return false;	
					}			
				}
			}
			else{			//Broadcom 
				if(document.form.wl_auth_mode_x.value == "open" && document.form.wl_wep_x.value == "0"){
					if(!confirm("<#wireless_JS_WPS_open#>"))
						return false;		
				}
		
				if( document.form.wl_auth_mode_x.value == "shared"
				 ||	document.form.wl_auth_mode_x.value == "psk" || document.form.wl_auth_mode_x.value == "wpa"
				 || document.form.wl_auth_mode_x.value == "open" && (document.form.wl_wep_x.value == "1" || document.form.wl_wep_x.value == "2")){		//open wep case			
					if(confirm("<#wireless_JS_disable_WPS#>")){
						document.form.wps_enable.value = "0";	
					}
					else{	
						return false;	
					}			
				} 
			}
		}

		if(vht160_support){
			document.form.wl_bw_160.value = $("#enable_160mhz").prop("checked") ? 1 : 0;
		}

		showLoading();
		document.form.wps_config_state.value = "1";		
		if((auth_mode == "shared" || auth_mode == "wpa" || auth_mode == "wpa2"  || auth_mode == "wpawpa2" || auth_mode == "radius" ||
				((auth_mode == "open") && !(document.form.wl_wep_x.value == "0")))
				&& document.form.wps_mode.value == "enabled")
			document.form.wps_mode.value = "disabled";
		
		if(auth_mode == "wpa" || auth_mode == "wpa2" || auth_mode == "wpawpa2" || auth_mode == "radius"){
			if(based_modelid != "BRT-AC828"){
				document.form.next_page.value = "/Advanced_WSecurity_Content.asp";
			}
		}

		if(auth_mode == 'sae'){
			document.form.wl_mfp.value = '2';
		}
		else if(auth_mode == 'psk2sae' && document.form.wl_mfp.value == '0'){
			document.form.wl_mfp.value = '1';
		}

		if(Bcmwifi_support) {
			if(document.form.wl_nmode_x.value != "2" && wl_unit_value == "0")
				document.form.wl_gmode_protection.value = "auto";
		}
		else {
			if(document.form.wl_nmode_x.value == "1" && wl_unit_value == "0")
				document.form.wl_gmode_protection.value = "off";
		}

		/*  Viz 2012.08.15 seems ineeded
		inputCtrl(document.form.wl_crypto, 1);
		inputCtrl(document.form.wl_wpa_psk, 1);
		inputCtrl(document.form.wl_wep_x, 1);
		inputCtrl(document.form.wl_key, 1);
		inputCtrl(document.form.wl_key1, 1);
		inputCtrl(document.form.wl_key2, 1);
		inputCtrl(document.form.wl_key3, 1);
		inputCtrl(document.form.wl_key4, 1);
		inputCtrl(document.form.wl_phrase_x, 1);
		inputCtrl(document.form.wl_wpa_gtk_rekey, 1);*/
		
		if(isSwMode("re") || isSwMode("mb"))
			document.form.action_wait.value = "5";

		if (Qcawifi_support) {
			document.form.action_wait.value = "30";
		}
		else if (Rawifi_support) {
			document.form.action_wait.value = "20";
		}

		document.form.submit();
	}
}

function validForm(){
	var auth_mode = document.form.wl_auth_mode_x.value;
	
	if(!validator.stringSSID(document.form.wl_ssid))
		return false;
	if(sw_mode != 2){
		if(!check_NOnly_to_GN()){
			autoFocus('wl_nmode_x');
			return false;
		}
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
		return(false);
	}
	return true;
}

function disableAdvFn(){
	for(var i=18; i>=3; i--)
		document.getElementById("WLgeneral").deleteRow(i);
}

function _change_wl_unit(val){
	if (band60g_support && he_frame_support) {
		if (document.form.wl_unit.value == '3') {
			$("#he_mode_field").hide();
		}
		else {
			$("#he_mode_field").show();
		}
	}
	if(!concurrep_support && (isSwMode("re") || isSwMode("mb")) && val == wlc_band_value)
		document.form.wl_subunit.value = 1;
	else
		document.form.wl_subunit.value = -1;

	if(concurrep_support && (isSwMode("re") || isSwMode("mb") || isSwMode("ew")))
		document.form.wl_subunit.value = 1;

	change_wl_unit();
}

function clean_input(obj){
	if(obj.value == "<#wireless_psk_fillin#>")
			obj.value = "";
}

function check_NOnly_to_GN(){
	//var gn_array_2g = [["1", "ASUS_Guest1", "psk", "tkip", "1234567890", "0", "1", "", "", "", "", "0", "off", "0"], ["1", "ASUS_Guest2", "shared", "aes", "", "1", "1", "55555", "", "", "", "0", "off", "0"], ["1", "ASUS_Guest3", "open", "aes", "", "2", "4", "", "", "", "1234567890123", "0", "off", "0"]];
	//                    Y/N        mssid      auth    asyn    wpa_psk wl_wep_x wl_key k1	k2 k3 k4                                        
	//var gn_array_5g = [["1", "ASUS_5G_Guest1", "open", "aes", "", "0", "1", "", "", "", "", "0", "off", "0"], ["0", "ASUS_5G_Guest2", "open", "aes", "", "0", "1", "", "", "", "", "0", "off", ""], ["0", "ASUS_5G_Guest3", "open", "aes", "", "0", "1", "", "", "", "", "0", "off", ""]];
	// Viz add 2012.11.05 restriction for 'N Only' mode  ( start 	
	if(document.form.wl_nmode_x.value == "0" || document.form.wl_nmode_x.value == "1"){
		if(wl_unit_value == "1" || wl_unit_value == "2"){		//5G
			for(var i=0;i<gn_array_5g.length;i++){
				if(gn_array_5g[i][0] == "1" && (gn_array_5g[i][3] == "tkip" || gn_array_5g[i][5] == "1" || gn_array_5g[i][5] == "2")){
					if(document.form.wl_nmode_x.value == "0")
						document.getElementById('wl_NOnly_note').innerHTML = '<br>* <#WLANConfig11n_Auto_note#>';
					else{
						document.getElementById('wl_NOnly_note').innerHTML = '<br>* <#WLANConfig11n_NOnly_note#>';
					}	
						
					document.getElementById('wl_NOnly_note').style.display = "";
					return false;
				}
			}		
		}
		else if(wl_unit_value == "0"){		//2.4G
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

function high_power_auto_channel(){
	if(is_high_power){
		if(document.form.wl_channel.value == 1){
			if(confirm("<#WLANConfig11b_Channel_HighPower_desc1#>")){
				document.form.wl_channel.value = 2;
			}
			else if(!(confirm("<#WLANConfig11b_Channel_HighPower_desc2#>"))){
				document.form.wl_channel.value = 2;
			}
		}
		else if(document.form.wl_channel.value == 11){
			if(confirm("<#WLANConfig11b_Channel_HighPower_desc3#>")){
				document.form.wl_channel.value = 10;
			}
			else if(!(confirm("<#WLANConfig11b_Channel_HighPower_desc4#>"))){
				document.form.wl_channel.value = 10;
			}
		}	

		if(document.form.wl_channel.value == 0)
			document.form.AUTO_CHANNEL.value = 1;
		else
			document.form.AUTO_CHANNEL.value = 0;
	}
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

function enableSmartCon(val){
	document.form.smart_connect_x.value = val;
	var value = new Array();
	var desc = new Array();

	if(isSupport("triband") && dwb_info.mode) {
		desc = ["Dual-Band Smart Connect (2.4GHz and 5GHz)"];
		value = ["1"];
		add_options_x2(document.form.smart_connect_t, desc, value, val);
	}
	else {
		if(based_modelid=="RT-AC5300" || based_modelid=="GT-AC5300"){
			desc = ["Tri-Band Smart Connect (2.4GHz, 5GHz-1 and 5GHz-2)", "5GHz Smart Connect (5GHz-1 and 5GHz-2)"];
			value = ["1", "2"];
			add_options_x2(document.form.smart_connect_t, desc, value, val);
		}
		else if(based_modelid =="RT-AC3200" || based_modelid =="RT-AC95U"){
			desc = ["Tri-Band Smart Connect (2.4GHz, 5GHz-1 and 5GHz-2)"];
			value = ["1"];
			add_options_x2(document.form.smart_connect_t, desc, value, val);
		}
		else if(based_modelid == "RT-AC88U" || based_modelid == "RT-AC86U" || based_modelid == "GT-AC2900" || based_modelid == "RT-AC3100" || based_modelid == "BLUECAVE" || based_modelid == "MAP-AC1750" || based_modelid == "RT-AX89U" || based_modelid == "GT-AXY16000" || based_modelid.substring(0,7) == "RT-AC59"){
			desc = ["Dual-Band Smart Connect (2.4GHz and 5GHz)"];
			value = ["1"];
			add_options_x2(document.form.smart_connect_t, desc, value, val);
		}
	}
	
	
	if(based_modelid=="RT-AC5300" || 
		based_modelid=="GT-AC5300" || 
		based_modelid=="RT-AC3200" || 
		based_modelid=="RT-AC88U" ||
		based_modelid == "RT-AC86U" ||
		based_modelid == "GT-AC2900" ||
		based_modelid == "RT-AC3100" ||
		based_modelid == "RT-AC95U" ||
		based_modelid == "MAP-AC1750" ||
		based_modelid.substring(0,7) == "RT-AC59" ||
		based_modelid == "RT-AX89U" ||
		based_modelid == "GT-AXY16000" ||
		based_modelid == "BLUECAVE"){
		document.getElementById("smartcon_rule_link").style.display = "none";
		if(val == 0){
			document.getElementById("smart_connect_field").style.display = "none";
		}else if(val > 0){
			document.getElementById("smart_connect_field").style.display = "";
		}
	}

	if((val == 0 || (val == 2 && wl_unit == 0)) || (dwb_info.mode && wl_unit == dwb_info.band)){
		document.getElementById("wl_unit_field").style.display = "";
		document.form.wl_nmode_x.disabled = "";
		document.getElementById("wl_optimizexbox_span").style.display = "";
		if(document.form.wl_unit[0].selected == true){
			document.getElementById("wl_gmode_checkbox").style.display = "";
		}
		if(band5g_11ac_support){
			regen_5G_mode(document.form.wl_nmode_x, wl_unit);		
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
		document.getElementById("wl_optimizexbox_span").style.display = "none";
		document.getElementById("wl_gmode_checkbox").style.display = "none";
		if (Qcawifi_support)
			__regen_auto_option(document.form.wl_bw, 1);
		else
			regen_auto_option(document.form.wl_bw);
		regen_auto_option(document.form.wl_channel);
		regen_auto_option(document.form.wl_nctrlsb);			
	}
	
	if(based_modelid=="RT-AC5300" || based_modelid=="GT-AC5300" || based_modelid=="RT-AC3200")
		_change_smart_connect(val);
}

function enable_160MHz(obj){
	cur = '<% nvram_get("wl_bw"); %>';
	var bws = new Array();
	var bwsDesc = new Array();

	if(obj.checked){
		bws = [1, 0, 2, 3, 5];
		bwsDesc = ["20/40/80/160 MHz", "20 MHz", "40 MHz", "80 MHz", "160 MHz"];
		enable_bw_160 = true;
	}
	else{
		bws = [1, 0, 2, 3];
		bwsDesc = ["20/40/80 MHz", "20 MHz", "40 MHz", "80 MHz"];
		enable_bw_160 = false;
	}

	add_options_x2(document.form.wl_bw, bwsDesc, bws, cur);
	insertExtChannelOption();
}

function __regen_auto_option(obj,val){
	free_options(obj);
	obj.options[0] = new Option("<#Auto#>", val);
	obj.selectedIndex = 0;
}

function regen_auto_option(obj){
	__regen_auto_option(obj,0);
}

var wl_unit = <% nvram_get("wl_unit"); %>;
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
			obj.options[1] = new Option("N/AC/AX mixed", 8);
			obj.options[2] = new Option("Legacy", 2);
		}
		else{
			obj.options[0] = new Option("<#Auto#>", 0);
			obj.options[1] = new Option("N only", 1);
			obj.options[2] = new Option("N/AC mixed", 8);
			obj.options[3] = new Option("Legacy", 2);
		}
	}
	else{
		obj.options[0] = new Option("<#Auto#>", 0);
		obj.options[1] = new Option("N only", 1);
		obj.options[2] = new Option("Legacy", 2);
	}
	obj.value = '<% nvram_get("wl_nmode_x"); %>';
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

	if (he_frame_support) {
		if (o.value == '0' && !(band60g_support && document.form.wl_unit.value == '3')) {
			$("#he_mode_field").show();
		}
		else {
			$("#he_mode_field").hide();
		}
	}

	limit_auth_method();
	if(o.value == "3"){
		document.form.wl_wme.value = "on";
	}

	
	if(wl_unit == '0')
		check_channel_2g();
	else if(wl_unit == '3')
		insertChannelOption_60g();
	else
		insertExtChannelOption_5g();

	genBWTable(wl_unit);
}
function he_frame_mode(obj) {
	if (obj.value == "0" && wl_unit != 0) {
		$("#enable_160mhz")[0].checked = false
		enable_160MHz($("#enable_160mhz")[0]);
		document.form.acs_dfs_checkbox.checked = false;
		document.form.acs_dfs.value = 0;
	}
}

function ajax_wl_channel(){
	$.ajax({
		url: '/ajax_wl_channel.asp',
		dataType: 'script',	
		error: function(xhr) {
			setTimeout("ajax_wl_channel();", 1000);
		},
		success: function(response){
			$("#auto_channel").html("<#wireless_control_channel#>: " + cur_control_channel[wl_unit]);
			setTimeout("ajax_wl_channel();", 5000);
		}
	});
}

function ajax_wl_edmg_channel(){
	$.ajax({
		url: '/ajax_wl_edmg_channel.asp',
		dataType: 'script',	
		error: function(xhr) {
			setTimeout("ajax_wl_edmg_channel();", 1000);
		},
		success: function(response){
			$("#auto_edmg_channel").html("Current EDMG Channel: " + cur_edmg_channel[wl_unit]); /* untranslated */
			setTimeout("ajax_wl_edmg_channel();", 5000);
		}
	});
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

<iframe name="hidden_frame" id="hidden_frame" width="0" height="0" frameborder="0"></iframe>
<form method="post" name="form" action="/start_apply2.htm" target="hidden_frame">
<input type="hidden" name="productid" value="<% nvram_get("productid"); %>">


<input type="hidden" name="current_page" value="Advanced_Wireless_Content.asp">
<input type="hidden" name="next_page" value="Advanced_Wireless_Content.asp">
<input type="hidden" name="modified" value="0">
<input type="hidden" name="action_mode" value="apply_new">
<input type="hidden" name="action_script" value="restart_wireless">
<input type="hidden" name="action_wait" value="5">
<input type="hidden" name="preferred_lang" id="preferred_lang" value="<% nvram_get("preferred_lang"); %>">
<input type="hidden" name="wl_country_code" value="<% nvram_get("wl0_country_code"); %>" disabled>
<input type="hidden" name="firmver" value="<% nvram_get("firmver"); %>">
<input type="hidden" name="wps_mode" value="<% nvram_get("wps_mode"); %>">
<input type="hidden" name="wps_config_state" value="<% nvram_get("wps_config_state"); %>">
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
<input type="hidden" name="wl_nmode" value="<% nvram_get("wl_nmode"); %>">
<input type="hidden" name="wl_nctrlsb_old" value="<% nvram_get("wl_nctrlsb"); %>">
<input type="hidden" name="wl_key_type" value='<% nvram_get("wl_key_type"); %>'> <!--Lock Add 2009.03.10 for ralink platform-->
<input type="hidden" name="wl_channel_orig" value='<% nvram_get("wl_channel"); %>'>
<input type="hidden" name="wl_edmg_channel_orig" value='<% nvram_get("wl_edmg_channel"); %>' disabled>
<input type="hidden" name="AUTO_CHANNEL" value='<% nvram_get("AUTO_CHANNEL"); %>'>
<input type="hidden" name="wl_wep_x_orig" value='<% nvram_get("wl_wep_x"); %>'>
<input type="hidden" name="wl_optimizexbox" value='<% nvram_get("wl_optimizexbox"); %>'>
<input type="hidden" name="wl_bw_160" value='<% nvram_get("wl_bw_160"); %>'>
<input type="hidden" name="wl_subunit" value='-1'>
<input type="hidden" name="acs_dfs" value='<% nvram_get("acs_dfs"); %>'>
<input type="hidden" name="acs_band1" value='<% nvram_get("acs_band1"); %>'>
<input type="hidden" name="acs_band3" value='<% nvram_get("acs_band3"); %>'>
<input type="hidden" name="acs_ch13" value='<% nvram_get("acs_ch13"); %>'>
<input type="hidden" name="wps_enable" value="<% nvram_get("wps_enable"); %>">
<input type="hidden" name="wps_band" value="<% nvram_get("wps_band_x"); %>" disabled>
<input type="hidden" name="wps_multiband" value="<% nvram_get("wps_multiband"); %>" disabled>
<input type="hidden" name="w_Setting" value="1">
<input type="hidden" name="w_apply" value="1">
<input type="hidden" name="smart_connect_x" value="<% nvram_get("smart_connect_x"); %>">

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
      		<div style="margin:10px 0 10px 5px;" class="splitLine"></div>
      <div class="formfontdesc"><#adv_wl_desc#></div>
		<div id="lantiq_ready" style="display:none;color:#FC0;margin-left:5px;font-size:13px;">Wireless is setting...</div>
			<table width="99%" border="1" align="center" cellpadding="4" cellspacing="0" id="WLgeneral" class="FormTable">
					<tr id="smartcon_enable_field" style="display:none;">
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
										var smart_connect_flag_t;

									if(flag == '')
										smart_connect_flag_t = '<% nvram_get("smart_connect_x"); %>';
									else
										smart_connect_flag_t = flag;

										$('#radio_smartcon_enable').iphoneSwitch( smart_connect_flag_t > 0, 
										 function() {
											if(based_modelid != "RT-AC5300" && based_modelid != "GT-AC5300" && based_modelid !="RT-AC3200" && based_modelid != "RT-AC88U" && based_modelid != "RT-AC86U" && based_modelid != "GT-AC2900" && based_modelid != "RT-AC3100" && based_modelid != "BLUECAVE" && based_modelid != "MAP-AC1750"  && based_modelid != "RT-AC95U" && based_modelid != "RT-AX89U" && based_modelid != "GT-AXY16000")
												enableSmartCon(1);
											else{
												if(document.form.smart_connect_t.value)
													enableSmartCon(document.form.smart_connect_t.value);
												else
													enableSmartCon(smart_connect_flag_t);
											}
										 },
										 function() {
											enableSmartCon(0);
										 }
									);
								</script>
				  	  </td>
					</tr>	
					<tr id="smart_connect_field" style="display:none;">                     
						<th><#smart_connect#></th>                                            
						<td id="smart_connect_switch">
						<select name="smart_connect_t" class="input_option" onChange="enableSmartCon(this.value);">
							<option class="content_input_fd" value="1" >Tri-band Smart Connect (2.4GHz, 5GHz-1 and 5GHz-2)</optio>
							<option class="content_input_fd" value="2">5GHz Smart Connect (5GHz-1 and 5GHz-2)</option>
						</select>                       
						</td>
					</tr>

				<tr id="wl_unit_field">
					<th><#Interface#></th>
					<td>
						<select name="wl_unit" class="input_option" onChange="_change_wl_unit(this.value);">
							<option class="content_input_fd" value="0" <% nvram_match("wl_unit", "0","selected"); %>>2.4 GHz</option>
							<option class="content_input_fd" value="1" <% nvram_match("wl_unit", "1","selected"); %>>5 GHz</option>
							<option class="content_input_fd" value="1" <% nvram_match("wl_unit", "2","selected"); %>>5GHz-2</option>
						</select>			
					</td>
		  	</tr>

				<!--tr id="wl_subunit_field" style="display:none">
					<th>Multiple SSID index</th>
					<td>
						<select name="wl_subunit" class="input_option" onChange="change_wl_unit();">
							<option class="content_input_fd" value="0" <% nvram_match("wl_subunit", "0","selected"); %>>Primary</option>
						</select>			
						<select id="wl_bss_enabled_field" name="wl_bss_enabled" class="input_option" onChange="mbss_switch();">
							<option class="content_input_fd" value="0" <% nvram_match("wl_bss_enabled", "0","selected"); %>><#WLANConfig11b_WirelessCtrl_buttonname#></option>
							<option class="content_input_fd" value="1" <% nvram_match("wl_bss_enabled", "1","selected"); %>><#WLANConfig11b_WirelessCtrl_button1name#></option>
						</select>			
					</td>
		  	</tr-->

				<tr>
					<th><a class="hintstyle" href="javascript:void(0);" onClick="openHint(0, 1);"><#QIS_finish_wireless_item1#></a></th>
					<td>
						<input type="text" maxlength="32" class="input_32_table" id="wl_ssid" name="wl_ssid" value="<% nvram_get("wl_ssid"); %>" onkeypress="return validator.isString(this, event)" autocorrect="off" autocapitalize="off">
					</td>
		  	</tr>
			  
				<tr id="wl_closed_field">
					<th><a class="hintstyle" href="javascript:void(0);" onClick="openHint(0, 2);"><#WLANConfig11b_x_BlockBCSSID_itemname#></a></th>
					<td>
						<input type="radio" value="1" name="wl_closed" class="input" onClick="return change_common_radio(this, 'WLANConfig11b', 'wl_closed', '1')" <% nvram_match("wl_closed", "1", "checked"); %>><#checkbox_Yes#>
						<input type="radio" value="0" name="wl_closed" class="input" onClick="return change_common_radio(this, 'WLANConfig11b', 'wl_closed', '0')" <% nvram_match("wl_closed", "0", "checked"); %>><#checkbox_No#>
						<span id="WPS_hideSSID_hint" style="display:none;"></span>	
					</td>					
				</tr>
					  
			  <tr>
					<th><a id="wl_mode_desc" class="hintstyle" href="javascript:void(0);" onClick="openHint(0, 4);"><#WLANConfig11b_x_Mode_itemname#></a></th>
					<td>									
						<select name="wl_nmode_x" class="input_option" onChange="wireless_mode_change(this);">
							<option value="0" <% nvram_match("wl_nmode_x", "0","selected"); %>><#Auto#></option>
							<option value="1" <% nvram_match("wl_nmode_x", "1","selected"); %>>N Only</option>
							<option value="2" <% nvram_match("wl_nmode_x", "2","selected"); %>>Legacy</option>
							<option value="8" <% nvram_match("wl_nmode_x", "8","selected"); %>>N/AC Mixed</option>
						</select>
						<span id="wl_optimizexbox_span" style="display:none"><input type="checkbox" name="wl_optimizexbox_ckb" id="wl_optimizexbox_ckb" value="<% nvram_get("wl_optimizexbox"); %>" onclick="document.form.wl_optimizexbox.value=(this.checked==true)?1:0;"> <#WLANConfig11b_x_Mode_xbox#></input></span>
						<span id="wl_gmode_checkbox" style="display:none;"><input type="checkbox" name="wl_gmode_check" id="wl_gmode_check" value="" onClick="wl_gmode_protection_check();"> <#WLANConfig11b_x_Mode_protectbg#></input></span>
						<span id="wl_nmode_x_hint" style="display:none;"><br><#WLANConfig11n_automode_limition_hint#><br></span>
						<span id="wl_NOnly_note" style="display:none;"></span>
						<!-- [N only] is not compatible with current guest network authentication method(TKIP or WEP),  Please go to <a id="gn_link" href="/Guest_network.asp?af=wl_NOnly_note" target="_blank" style="color:#FFCC00;font-family:Lucida Console;text-decoration:underline;">guest network</a> and change the authentication method. -->
					</td>
			  </tr>
				<tr id="he_mode_field" style="display:none">
					<th>
						<a id="he_mode_text" class="hintstyle" href="javascript:void(0);" onClick=""><#WLANConfig11b_HE_Frame_Mode_itemname#></a>
					</th>
					<td>
						<div style="display:flex;align-items: center;">
							<select name="wl_11ax" class="input_option" onChange="he_frame_mode(this);">
								<option value="1" <% nvram_match("wl_11ax", "1" ,"selected"); %> ><#WLANConfig11b_WirelessCtrl_button1name#></option>
								<option value="0" <% nvram_match("wl_11ax", "0" ,"selected"); %> ><#WLANConfig11b_WirelessCtrl_buttonname#></option>
							</select>
							<span id="he_mode_faq" style="padding: 0 10px"><#WLANConfig11b_HE_Frame_Mode_faq#></span>
						</div>
					</td>
				</tr>
			 	<tr id="wl_bw_field">
			   	<th><a class="hintstyle" href="javascript:void(0);" onClick="openHint(0, 14);"><#WLANConfig11b_ChannelBW_itemname#></a></th>
			   	<td>				    			
						<select name="wl_bw" class="input_option" onChange="insertExtChannelOption();">
							<option class="content_input_fd" value="1" <% nvram_match("wl_bw", "1","selected"); %>>20/40/80 MHz</option>
							<option class="content_input_fd" value="0" <% nvram_match("wl_bw", "0","selected"); %>>20 MHz</option>
							<option class="content_input_fd" value="2" <% nvram_match("wl_bw", "2","selected"); %>>40 MHz</option>
							<option class="content_input_fd" value="3" <% nvram_match("wl_bw", "3","selected"); %>>80 MHz</option>
						</select>
						<span id="enable_160_field" style="display:none"><input type="checkbox" onclick="enable_160MHz(this);" id="enable_160mhz">Enable 160 MHz</span>			
			   	</td>
			 	</tr>			  
			  
				<tr id="wl_channel_field">
					<th><a id="wl_channel_select" class="hintstyle" href="javascript:void(0);" onClick="openHint(0, 3);"><#WLANConfig11b_Channel_itemname#></a></th>
					<td>
				 		<select name="wl_channel" class="input_option" onChange="high_power_auto_channel();insertExtChannelOption();"></select>
						<span id="auto_channel" style="display:none;margin-left:10px;"></span><br>
						
						<div style="margin-top:5px">
						<div><span id="dfs_checkbox" style="display:none"><input type="checkbox" onClick="check_DFS_support(this);" name="acs_dfs_checkbox" <% nvram_match("acs_dfs", "1", "checked"); %>><#WLANConfig11b_EChannel_dfs#></input></span></div>
						<div><span id="acs_band1_checkbox" style="display:none;"><input type="checkbox" onClick="check_acs_band1_support(this);" <% nvram_match("acs_band1", "1", "checked"); %>><#WLANConfig11b_EChannel_band1#></input></span></div>
						<div><span id="acs_band3_checkbox" style="display:none;"><input type="checkbox" onClick="check_acs_band3_support(this);" <% nvram_match("acs_band3", "1", "checked"); %>><#WLANConfig11b_EChannel_band3#></input></span></div>
						<div><span id="acs_ch13_checkbox" style="display:none;"><input type="checkbox" onClick="check_acs_ch13_support(this);" <% nvram_match("acs_ch13", "1", "checked"); %>><#WLANConfig11b_EChannel_acs_ch13#></input></span></div>
						</div>
					</td>
			  </tr>			 

				<tr id="wl_edmg_field" style="display:none">
					<th><a id="wl_edmg_select" class="hintstyle" href="javascript:void(0);">EDMG channel</a></th>
					<td>
						<select name="wl_edmg_channel" class="input_option">
							<option class="content_input_fd" value="0" <% nvram_match("wl_edmg_channel", "0","selected"); %>><#Auto#></option>
						</select>
						<span id="auto_edmg_channel" style="display:none;margin-left:10px;"></span><br>
					</td>
				</tr>

			  <tr id="wl_nctrlsb_field">
			  	<th><a class="hintstyle" href="javascript:void(0);" onClick="openHint(0, 15);"><#WLANConfig11b_EChannel_itemname#></a></th>
		   		<td>
					<select name="wl_nctrlsb" class="input_option">
						<option value="lower" <% nvram_match("wl_nctrlsb", "lower", "selected"); %>>lower</option>
						<option value="upper"<% nvram_match("wl_nctrlsb", "upper", "selected"); %>>upper</option>
					</select>
					</td>
		  	</tr>
			  
			  	<tr>
					<th><a class="hintstyle" href="javascript:void(0);" onClick="openHint(0, 5);"><#WLANConfig11b_AuthenticationMethod_itemname#></a></th>
					<td>
				  		<select name="wl_auth_mode_x" class="input_option" onChange="authentication_method_change(this);">
							<option value="open"    <% nvram_match("wl_auth_mode_x", "open",   "selected"); %>>Open System</option>
							<option value="shared"  <% nvram_match("wl_auth_mode_x", "shared", "selected"); %>>Shared Key</option>
							<option value="psk"     <% nvram_match("wl_auth_mode_x", "psk",    "selected"); %>>WPA-Personal</option>
							<option value="psk2"    <% nvram_match("wl_auth_mode_x", "psk2",   "selected"); %>>WPA2-Personal</option>
							<option value="sae"    <% nvram_match("wl_auth_mode_x", "sae",   "selected"); %>>WPA3-Personal</option>
							<option value="pskpsk2" <% nvram_match("wl_auth_mode_x", "pskpsk2","selected"); %>>WPA-Auto-Personal</option>
							<option value="psk2sae" <% nvram_match("wl_auth_mode_x", "psk2sae","selected"); %>>WPA2/WPA3-Personal</option>
							<option value="wpa"     <% nvram_match("wl_auth_mode_x", "wpa",    "selected"); %>>WPA-Enterprise</option>
							<option value="wpa2"    <% nvram_match("wl_auth_mode_x", "wpa2",   "selected"); %>>WPA2-Enterprise</option>
							<option value="wpawpa2" <% nvram_match("wl_auth_mode_x", "wpawpa2","selected"); %>>WPA-Auto-Enterprise</option>
							<option value="radius"  <% nvram_match("wl_auth_mode_x", "radius", "selected"); %>>Radius with 802.1x</option>
				  		</select>
					</td>
			  	</tr>
			  	
			  	<tr>
					<th><a class="hintstyle" href="javascript:void(0);" onClick="openHint(0, 6);"><#WLANConfig11b_WPAType_itemname#></a></th>
					<td>		
				  		<select name="wl_crypto" class="input_option">
								<option value="aes" <% nvram_match("wl_crypto", "aes", "selected"); %>>AES</option>
								<option value="tkip+aes" <% nvram_match("wl_crypto", "tkip+aes", "selected"); %>>TKIP+AES</option>
				  		</select>
					</td>
			  	</tr>
			  
			  	<tr>
					<th><a class="hintstyle" href="javascript:void(0);" onClick="openHint(0, 7);"><#WLANConfig11b_x_PSKKey_itemname#></a></th>
					<td>
						<input id="wl_wpa_psk" name="wl_wpa_psk" maxlength="64" class="input_32_table" type="password" autocapitalization="off" onBlur="switchType(this, false);" onFocus="switchType(this, true);" value="<% nvram_get("wl_wpa_psk"); %>" autocomplete="new-password" autocorrect="off" autocapitalize="off">
					</td>
			  	</tr>
			  		  
			  	<tr>
					<th><a class="hintstyle" href="javascript:void(0);" onClick="openHint(0, 9);"><#WLANConfig11b_WEPType_itemname#></a></th>
					<td>
				  		<select name="wl_wep_x" class="input_option" onChange="wep_encryption_change(this);">
								<option value="0" <% nvram_match("wl_wep_x", "0", "selected"); %>><#wl_securitylevel_0#></option>
								<option value="1" <% nvram_match("wl_wep_x", "1", "selected"); %>>WEP-64bits</option>
								<option value="2" <% nvram_match("wl_wep_x", "2", "selected"); %>>WEP-128bits</option>
				  		</select>
				  		<span name="key_des"></span>
					</td>
			  	</tr>
			  
			  	<tr>
					<th><a class="hintstyle" href="javascript:void(0);" onClick="openHint(0, 10);"><#WLANConfig11b_WEPDefaultKey_itemname#></a></th>
					<td>		
				  		<select name="wl_key" class="input_option"  onChange="wep_key_index_change(this);">
							<option value="1" <% nvram_match("wl_key", "1","selected"); %>>1</option>
							<option value="2" <% nvram_match("wl_key", "2","selected"); %>>2</option>
							<option value="3" <% nvram_match("wl_key", "3","selected"); %>>3</option>
							<option value="4" <% nvram_match("wl_key", "4","selected"); %>>4</option>
				  		</select>
					</td>
			  	</tr>
			  
			  	<tr>
					<th><a class="hintstyle" href="javascript:void(0);" onClick="openHint(0, 18);"><#WLANConfig11b_WEPKey1_itemname#></th>
					<td><input type="text" name="wl_key1" id="wl_key1" maxlength="27" class="input_25_table" value="<% nvram_get("wl_key1"); %>" onKeyUp="return change_wlkey(this, 'WLANConfig11b');" autocorrect="off" autocapitalize="off"></td>
			  	</tr>
			  
			  	<tr>
					<th><a class="hintstyle" href="javascript:void(0);" onClick="openHint(0, 18);"><#WLANConfig11b_WEPKey2_itemname#></th>
					<td><input type="text" name="wl_key2" id="wl_key2" maxlength="27" class="input_25_table" value="<% nvram_get("wl_key2"); %>" onKeyUp="return change_wlkey(this, 'WLANConfig11b');" autocorrect="off" autocapitalize="off"></td>
			  	</tr>
			  
			  	<tr>
					<th><a class="hintstyle" href="javascript:void(0);" onClick="openHint(0, 18);"><#WLANConfig11b_WEPKey3_itemname#></th>
					<td><input type="text" name="wl_key3" id="wl_key3" maxlength="27" class="input_25_table" value="<% nvram_get("wl_key3"); %>" onKeyUp="return change_wlkey(this, 'WLANConfig11b');" autocorrect="off" autocapitalize="off"></td>
			  	</tr>
			  
			  	<tr>
					<th><a class="hintstyle" href="javascript:void(0);" onClick="openHint(0, 18);"><#WLANConfig11b_WEPKey4_itemname#></th>
					<td><input type="text" name="wl_key4" id="wl_key4" maxlength="27" class="input_25_table" value="<% nvram_get("wl_key4"); %>" onKeyUp="return change_wlkey(this, 'WLANConfig11b');" autocorrect="off" autocapitalize="off"></td>
		  		</tr>

			  	<tr>
					<th><a class="hintstyle" href="javascript:void(0);" onClick="openHint(0, 8);"><#WLANConfig11b_x_Phrase_itemname#></a></th>
					<td>
				  		<input type="text" name="wl_phrase_x" maxlength="64" class="input_32_table" value="<% nvram_get("wl_phrase_x"); %>" onKeyUp="return is_wlphrase('WLANConfig11b', 'wl_phrase_x', this);" autocorrect="off" autocapitalize="off">
					</td>
			  	</tr>
				<tr >
					<th>
						<a class="hintstyle" href="javascript:void(0);"  onClick="openHint(2,1);">
					  	<#WLANAuthentication11a_ExAuthDBIPAddr_itemname#></a>
					</th>
					<td>
						<input type="text" maxlength="39" class="input_32_table" name="wl_radius_ipaddr" value='<% nvram_get("wl_radius_ipaddr"); %>' onKeyPress="return validator.isIPAddr(this, event)" autocorrect="off" autocapitalize="off">
					</td>
				</tr>
				<tr>
					<th>
						<a class="hintstyle" href="javascript:void(0);"  onClick="openHint(2,2);">
					  	<#WLANAuthentication11a_ExAuthDBPortNumber_itemname#></a>
					</th>
					<td>
						<input type="text" maxlength="5" class="input_6_table" name="wl_radius_port" value='<% nvram_get("wl_radius_port"); %>' onkeypress="return validator.isNumber(this,event)" autocorrect="off" autocapitalize="off"/>
					</td>
				</tr>
				<tr>
					<th >
						<a class="hintstyle" href="javascript:void(0);"  onClick="openHint(2,3);">
						<#WLANAuthentication11a_ExAuthDBPassword_itemname#></a>
					</th>
					<td>
						<input type="password" maxlength="64" class="input_32_table" name="wl_radius_key" value="<% nvram_get("wl_radius_key"); %>" autocorrect="off" autocapitalize="off">
					</td>
				</tr>
				<tr style="display:none">
					<th><#WLANConfig11b_x_mfp#></th>
					<td>
				  		<select name="wl_mfp" class="input_option" >
								<option value="0" <% nvram_match("wl_mfp", "0", "selected"); %>><#WLANConfig11b_WirelessCtrl_buttonname#></option>
								<option value="1" <% nvram_match("wl_mfp", "1", "selected"); %>><#WLANConfig11b_x_mfp_opt1#></option>
								<option value="2" <% nvram_match("wl_mfp", "2", "selected"); %>><#WLANConfig11b_x_mfp_opt2#></option>
				  		</select>
					</td>
			  	</tr>
			  	<tr>
					<th><a class="hintstyle" href="javascript:void(0);" onClick="openHint(0, 11);"><#WLANConfig11b_x_Rekey_itemname#></a></th>
					<td><input type="text" maxlength="7" name="wl_wpa_gtk_rekey" class="input_6_table"  value='<% nvram_get("wl_wpa_gtk_rekey"); %>' onKeyPress="return validator.isNumber(this,event);" autocorrect="off" autocapitalize="off"></td>
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
			change_wl_unit();
		}
	}
})();
</script>
</body>
</html>
