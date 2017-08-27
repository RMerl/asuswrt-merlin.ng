<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Transitional//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd">
<html xmlns="http://www.w3.org/1999/xhtml">
<head>
<meta http-equiv="Content-Type" content="text/html; charset=utf-8" />
<meta HTTP-EQUIV="Pragma" CONTENT="no-cache">
<meta HTTP-EQUIV="Expires" CONTENT="-1">
<link rel="shortcut icon" href="images/favicon.png">
<link rel="icon" href="images/favicon.png">
<title></title>
<link href="/NM_style.css" rel="stylesheet" type="text/css" />
<link href="/form_style.css" rel="stylesheet" type="text/css" />
<script type="text/javascript" src="/general.js"></script>
<script type="text/javascript" src="/state.js"></script>
<script type="text/javascript" src="/help.js"></script>
<script type="text/javascript" src="/js/jquery.js"></script>
<script type="text/javascript" src="/validator.js"></script>
<script type="text/javascript" src="/switcherplugin/jquery.iphone-switch.js"></script>
<script>
if(parent.location.pathname !== '<% abs_networkmap_page(); %>' && parent.location.pathname !== "/") top.location.href = "../"+'<% networkmap_page(); %>';

<% wl_get_parameter(); %>
var flag = '<% get_parameter("flag"); %>';
var wireless_unit = cookie.get("wireless_unit");
var wireless_subunit = cookie.get("wireless_subunit");
var smart_connect_flag_t;
var wl_unit = '<% nvram_get("wl_unit"); %>';


if(yadns_support){
	var yadns_enable = '<% nvram_get("yadns_enable_x"); %>';
	var yadns_mode = '<% nvram_get("yadns_mode"); %>';
	var yadns_clients = [ <% yadns_clients(); %> ];
}

var lan_num = 8;
var wl0_vifnames = '<% nvram_get("wl0_vifnames"); %>';
var wireless_num = wl0_vifnames.split(' ').length + 1;
var vlan_enable = parseInt('<% nvram_get("vlan_enable"); %>');
var vlan_rulelist = decodeURIComponent("<% nvram_char_to_ascii("","vlan_rulelist"); %>");
var vlan_rulelist_row = vlan_rulelist.split('<');
var vlanrule_inf_array = new Array(); //[[vlan1, lan, 2g, 5g], [vlan2, lan, 2g, 5g], ...]
var captive_portal_used_wl_array = new Array();

function parse_vlan_rulelist(){
	var eth_inf = new Array();
	var wl2g_inf = new Array();
	var wl5g_inf = new Array();

	for(var i = 1; i < vlan_rulelist_row.length; i ++) {
		var vlan_rulelist_col = vlan_rulelist_row[i].split('>');
		var inf_value = 0;
		var vlan_inf = "";

		eth_inf = [];
		inf_value = parseInt(vlan_rulelist_col[4].slice(-4), 16)
		if(inf_value != 0){
			for(var k = 0; k < lan_num; k++){
				var isMember = (inf_value >> k) & 1;
				if(isMember){
					vlan_inf = "lan" + (k+1).toString();
					eth_inf.push(vlan_inf);
				}
			}
		}

		wl2g_inf = [];
		inf_value = parseInt(vlan_rulelist_col[5].slice(-2), 16)
		if(inf_value != 0){
			for(var k = 0; k < wireless_num; k++){
				var isMember = (inf_value >> k) & 1;
				if(isMember){
					if( k == 0 )
						vlan_inf = "wl0";
					else
						vlan_inf = "wl0." + k.toString();
					wl2g_inf.push(vlan_inf);
				}
			}
		}

		wl5g_inf = [];
		inf_value = parseInt(vlan_rulelist_col[6].slice(-2), 16);
		if(inf_value != 0){
			for(var k = 0; k < wireless_num; k++){
				var isMember = (inf_value >> k) & 1;
				if(isMember){
					if( k == 0 )
						vlan_inf = "wl1";
					else
						vlan_inf = "wl1." + k.toString();

					wl5g_inf.push(vlan_inf);
				}
			}
		}

		vlanrule_inf_array[i-1] = [vlan_rulelist_col[1], eth_inf, wl2g_inf, wl5g_inf];
	}
}

function get_wl_vlan_id(wireless_unit, wireless_subunit){
	var wl_unit_int = parseInt(wireless_unit);
	var wl_inf = "wl"+ wireless_unit;
	var vlan_id = "";

	if( wireless_subunit != "-1")
		wl_inf = wl_inf + "." + wireless_subunit;

	Object.keys(vlanrule_inf_array).forEach(function(key){
		var wl_inf_array = vlanrule_inf_array[key][wl_unit_int+2];

		Object.keys(wl_inf_array).forEach(function(inf_key){
			if(wl_inf_array[inf_key] == wl_inf){
				if(vlan_id !=  "")
					vlan_id += ', ';
				vlan_id += vlanrule_inf_array[key][0];
			}
		});
	});

	return vlan_id;
}

function initial(){
	var wl_subunit = '<% nvram_get("wl_subunit"); %>';

	if(tagged_based_vlan && vlan_enable)
		parse_vlan_rulelist();

	if(lyra_hide_support){
		document.getElementById("t0").style.display = "";
		document.getElementById("span0").innerHTML = "<#tm_wireless#>";
		document.getElementById("t0").className = "tabclick_NW";
		inputCtrl(document.form.wl_auth_mode_x, 0);
		document.getElementById("wl_wpa_psk_title").innerHTML = "<#Network_key#>";
		document.getElementById("t_status").style.display = "none";
	}
	else{
		if(band5g_support){
			document.getElementById("t0").style.display = "";
			document.getElementById("t1").style.display = "";
			if(wl_info.band5g_2_support){
				document.getElementById("t2").style.display = "";
				tab_reset(0);
			}
			if(wl_info.band60g_support) {
				document.getElementById("t3").style.display = "";
				tab_reset(0);
			}

			// disallow to use the other band as a wireless AP
			if(parent.isSwMode("mb") && !localAP_support){
				for(var x=0; x<wl_info.wl_if_total;x++){
					if(x != '<% nvram_get("wlc_band"); %>')
						document.getElementById('t'+parseInt(x)).style.display = 'none';
				}
			}
		}
		else{
			document.getElementById("t0").style.display = "";
		}
	}
	if(parent.sw_mode == 2){
		if(parent.wlc_express != 0){
			if(parent.wlc_express == 1){
				document.getElementById("t0").style.display = "none";
				if(wl_subunit != '1' || wl_unit != '1'){
					tabclickhandler(1);
				}
			}
			else if(parent.wlc_express == 2){
				document.getElementById("t1").style.display = "none";
				if(wl_subunit != '1' || wl_unit != '0'){
					tabclickhandler(0);
				}
			}
		}
		else{
			if(!parent.concurrep_support){
				if(wl_unit == '<% nvram_get("wlc_band"); %>' && wl_subunit != '1'){
					tabclickhandler(wl_unit);
				}
				else if(wl_unit != '<% nvram_get("wlc_band"); %>' && wl_subunit == '1'){
					tabclickhandler(wl_unit);
				}
			}
			else{
				if(wl_subunit != '1') tabclickhandler(wl_unit);
			}
		}
	}
	else if(parent.sw_mode == 4){
		if(wl_unit != '<% nvram_get("wlc_band"); %>'){
			tabclickhandler('<% nvram_get("wlc_band"); %>');
		}

		document.getElementById("WLnetworkmap").style.display = "none";
		document.getElementById("applySecurity").style.display = "none";
		document.getElementById("WLnetworkmap_re").style.display = "";
	}
	else{
		//beacuse enable guestnetwork will reset wl_unit = -1
		if(wl_unit == "-1" || wl_subunit != "-1"){
			if(wireless_unit != "" && wireless_unit != null) {
				tabclickhandler(parseInt(wireless_unit));
			}
			else {
				tabclickhandler(0);
			}
		}
	}

	// modify wlX.1_ssid(SSID to end clients) under repeater mode
	if((parent.sw_mode == 2 || parent.sw_mode == 4) && '<% nvram_get("wlc_band"); %>' == wl_unit)
		document.form.wl_subunit.value = 1;
	else {
		//for guestnetwork
		if(wireless_subunit != "" && wireless_subunit != null && wireless_subunit != "main") {
			document.form.wl_subunit.value = parseInt(wireless_subunit);
		}
		else {
			document.form.wl_subunit.value = -1;
			document.getElementById("tr_guestnetwork_control").style.display = "none";
		}
	}

	// modify wlX.1_ssid(SSID to end clients) under concurrent repeater mode
	if(parent.sw_mode == 2 && parent.concurrep_support)
		document.form.wl_subunit.value = 1;

	if(smart_connect_support && (parent.isSwMode("rt") || parent.isSwMode("ap"))){
		var smart_connect_x = '<% nvram_get("smart_connect_x"); %>';
		if(based_modelid == "RT-AC5300" ||
			based_modelid == "GT-AC5300" || 
			based_modelid == "RT-AC3200" || 
			based_modelid == "RT-AC88U" ||
			based_modelid == "RT-AC86U" ||
			based_modelid == "AC2900" ||
			based_modelid == "RT-AC3100"){
			var value = new Array();
			var desc = new Array();
				
			if(based_modelid == "RT-AC5300" || based_modelid == "GT-AC5300"){
				desc = ["none", "Tri-Band Smart Connect", "5GHz Smart Connect"];
				value = ["0", "1", "2"];
				add_options_x2(document.form.smart_connect_t, desc, value, smart_connect_x);
			}
			else if(based_modelid == "RT-AC3200"){
				desc = ["none", "Tri-Band Smart Connect"];
				value = ["0", "1"];
				add_options_x2(document.form.smart_connect_t, desc, value, smart_connect_x);						
			}
			else if(based_modelid == "RT-AC88U" || based_modelid == "RT-AC86U" || based_modelid == "AC2900" || based_modelid == "RT-AC3100"){
				desc = ["none", "Dual-Band Smart Connect"];
				value = ["0", "1"];
				add_options_x2(document.form.smart_connect_t, desc, value, smart_connect_x);						
			}
		
			if(smart_connect_x !=0)
				document.getElementById("smart_connect_field").style.display = '';
		}else{
			document.getElementById("smartcon_enable_field").style.display = '';
			document.getElementById("smartcon_enable_line").style.display = '';
		}
	}
	
	/*if(document.getElementById("t1").className == "tabclick_NW" && 	parent.Rawifi_support)	//no exist Rawifi
		document.getElementById("wl_txbf_tr").style.display = "";		//Viz Add 2011.12 for RT-N56U Ralink*/ 			

	var wireless_profile = new Array();
	
	if(document.form.wl_subunit.value == -1 || parent.sw_mode == 2 || parent.sw_mode == 4) {
		wireless_unit = decodeURIComponent('<% nvram_char_to_ascii("", "wl_unit"); %>');
		wireless_subunit = document.form.wl_subunit.value;
		wireless_profile.ssid = decodeURIComponent('<% nvram_char_to_ascii("", "wl_ssid"); %>');
		wireless_profile.wl_auth_mode_x = decodeURIComponent('<% nvram_char_to_ascii("", "wl_auth_mode_x"); %>');
		wireless_profile.wl_crypto = decodeURIComponent('<% nvram_char_to_ascii("", "wl_crypto"); %>');
		wireless_profile.wl_wpa_psk = decodeURIComponent('<% nvram_char_to_ascii("", "wl_wpa_psk"); %>');
		wireless_profile.wl_wep_x = decodeURIComponent('<% nvram_char_to_ascii("", "wl_wep_x"); %>');
		wireless_profile.wl_key = decodeURIComponent('<% nvram_char_to_ascii("", "wl_key"); %>');
		wireless_profile.wl_key1 = decodeURIComponent('<% nvram_char_to_ascii("", "wl_key1"); %>');
		wireless_profile.wl_key2 = decodeURIComponent('<% nvram_char_to_ascii("", "wl_key2"); %>');
		wireless_profile.wl_key3 = decodeURIComponent('<% nvram_char_to_ascii("", "wl_key3"); %>');
		wireless_profile.wl_key4 = decodeURIComponent('<% nvram_char_to_ascii("", "wl_key4"); %>');
		//main wireless not need
		document.form.wl_mbss.disabled = true;
		document.form.wl_bss_enabled.disabled = true;
	}
	else {
		if(wireless_unit == null || wireless_unit == "") {
			wireless_unit = decodeURIComponent('<% nvram_char_to_ascii("", "wl_unit"); %>');
		}
		var gn_array = "";
		switch(parseInt(wireless_unit)) {
			case 0:
				gn_array = gn_array_2g;
				break;
			case 1:			
				gn_array = gn_array_5g;
				break;
			case 2:
				gn_array = gn_array_5g_2;
				break;
		}
		wireless_subunit = document.form.wl_subunit.value;
		var array_idx = (document.form.wl_subunit.value - 1);
		wireless_profile.wl_bss_enabled = gn_array[array_idx][0];
		wireless_profile.ssid = gn_array[array_idx][1];
		wireless_profile.wl_auth_mode_x = gn_array[array_idx][2];
		wireless_profile.wl_crypto = gn_array[array_idx][3];
		wireless_profile.wl_wpa_psk = gn_array[array_idx][4];
		wireless_profile.wl_wep_x = gn_array[array_idx][5];
		wireless_profile.wl_key = gn_array[array_idx][6];
		wireless_profile.wl_key1 = gn_array[array_idx][7];
		wireless_profile.wl_key2 = gn_array[array_idx][8];
		wireless_profile.wl_key3 = gn_array[array_idx][9];
		wireless_profile.wl_key4 = gn_array[array_idx][10];
		//Guestnetwork not need
		document.form.wl_radio.disabled = true;
		document.form.wl_nmode_x.disabled = true;
		document.form.wps_enable.disabled = true;
		document.form.wps_band.disabled = true;
		document.form.wl_txbf.disabled = true;
		document.form.wl_mumimo.disabled = true;
		$('#radio_gn_enable').iphoneSwitch(gn_array[array_idx][0], 
			function() {
				document.form.wl_bss_enabled.value = "1";
			},
			function() {			
				document.form.wl_bss_enabled.value = "0";
			}
		);
	}
	document.form.wl_bss_enabled.value = wireless_profile.wl_bss_enabled;
	document.form.wl_ssid.value = wireless_profile.ssid;
	document.form.wl_auth_mode_x.value = wireless_profile.wl_auth_mode_x;
	document.form.wl_crypto.value = wireless_profile.wl_crypto;
	document.form.wl_wpa_psk.value = wireless_profile.wl_wpa_psk;
	document.form.wl_wep_x.value = wireless_profile.wl_wep_x;
	document.form.wl_key.value =  wireless_profile.wl_key;
	document.form.wl_key1.value =  wireless_profile.wl_key1;
	document.form.wl_key2.value = wireless_profile.wl_key2;
	document.form.wl_key3.value =  wireless_profile.wl_key3;
	document.form.wl_key4.value = wireless_profile.wl_key4;

	/* Viz banned 2012.06
	if(document.form.wl_wpa_psk.value.length <= 0)
		document.form.wl_wpa_psk.value = "<#wireless_psk_fillin#>";
	*/
	
	limit_auth_method();
	wl_auth_mode_change(1);
	show_LAN_info();

	if(smart_connect_support && (parent.isSwMode("rt") || parent.isSwMode("ap"))){

		if(flag == '')
			smart_connect_flag_t = '<% nvram_get("smart_connect_x"); %>';
		else
			smart_connect_flag_t = flag;	

			document.form.smart_connect_x.value = smart_connect_flag_t;		
			change_smart_connect(smart_connect_flag_t);	
	}

	if(parent.sw_mode == 4){
		var wlc_auth_mode = '<% nvram_get("wlc_auth_mode"); %>';
		if(wlc_auth_mode == "") wlc_auth_mode = '<% nvram_get("wlc0_auth_mode"); %>';
		if(wlc_auth_mode == "") wlc_auth_mode = '<% nvram_get("wlc1_auth_mode"); %>';
		if(wlc_auth_mode == "") wlc_auth_mode = 'unknown';
	}

	flash_button();	

	if(history.pushState != undefined) history.pushState("", document.title, window.location.pathname);

	if(!lyra_hide_support)
		change_tabclick();

	//short term solution for only router mode support Captive Portal
	if(isSwMode("rt")) {
		//captive portal used wl if
		
		var parse_wl_list = function(_profile, _wl_list_idx, _item, _cpa_used_array) {
			var _profile_row = _profile.split("<");
			for(var i = 0; i < _profile_row.length; i += 1) {
				if(_profile_row[i] != "") {
					var _wl_list = _profile_row[i].split(">")[_wl_list_idx];
					var _wl_if = _wl_list.split("wl");
					while(_wl_if.length) {
						if(_wl_if[0] != "") {
							_cpa_used_array["wl" + _wl_if[0]] = _item;
						}
						_wl_if.shift();
					}
				}
			}
		};
		var _enable_flag = "";
		var _profile_list = "";
		//check free wi-fi
		_enable_flag = '<% nvram_get("captive_portal_enable"); %>';
		if(_enable_flag == "on") {
			_profile_list = decodeURIComponent('<% nvram_char_to_ascii("","captive_portal"); %>');
			parse_wl_list(_profile_list, 5, "Free Wi-Fi", captive_portal_used_wl_array);
		}
		//check captive portal adv
		_enable_flag = '<% nvram_get("captive_portal_adv_enable"); %>';
		if(_enable_flag == "on") {
			_profile_list = decodeURIComponent('<% nvram_char_to_ascii("","captive_portal_adv_profile"); %>');
			parse_wl_list(_profile_list, 5, "Captive Portal Wi-Fi", captive_portal_used_wl_array);
		}
		
		//check fb wi-fi
		_enable_flag = '<% nvram_get("fbwifi_enable"); %>';
		if(_enable_flag == "on") {
			var fbwifi_2g = '<% nvram_get("fbwifi_2g"); %>';
			if(fbwifi_2g != "off") {
				captive_portal_used_wl_array[fbwifi_2g] = "Facebook Wi-Fi";
			}
			if(wl_info.band5g_support) {
				var fbwifi_5g = '<% nvram_get("fbwifi_5g"); %>';
				if(fbwifi_5g != "off") {
					captive_portal_used_wl_array[fbwifi_5g] = "Facebook Wi-Fi";
				}
			}
			if(wl_info.band5g_2_support) {
				var fbwifi_5g_2 = '<% nvram_get("fbwifi_5g_2"); %>';
				if(fbwifi_5g_2 != "off") {
					captive_portal_used_wl_array[fbwifi_5g_2] = "Facebook Wi-Fi";
				}
			}
		}
		var _wl_unit_subunit = "wl" + wireless_unit + "."  + wireless_subunit;
		if(captive_portal_used_wl_array[_wl_unit_subunit] != undefined) {
			document.getElementById("tr_guestnetwork_control").style.display = "none";
			document.getElementById("wl_captive_portal_tr").style.display = "";
			document.getElementById("wl_captive_portal_text").innerHTML = "Used by " + captive_portal_used_wl_array[_wl_unit_subunit];
		}
	}
	set_NM_height();
}

function change_tabclick(){
	document.getElementById("t" + wl_unit + "").className = "tabclick_NW";
}

function tabclickhandler(wl_unit){
	if(wl_unit == 'status'){
		location.href = "router_status.asp";
	}
	else{
		if(parent.sw_mode == 2 && parent.wlc_express != 0){
			document.form.wl_subunit.value = 1;
		}
		else if((parent.sw_mode == 2 || parent.sw_mode == 4) && '<% nvram_get("wlc_band"); %>' == wl_unit){
			document.form.wl_subunit.value = 1;
		}
		else if (parent.sw_mode == 2 && parent.concurrep_support){
			document.form.wl_subunit.value = 1;
		}
		else{
			document.form.wl_subunit.value = -1;
		}

		document.form.wl_unit.value = wl_unit;
		cookie.set("wireless_unit", wl_unit, 30);

		if(smart_connect_support && (parent.isSwMode("rt") || parent.isSwMode("ap"))){
			var smart_connect_flag = document.form.smart_connect_x.value;
			document.form.current_page.value = "device-map/router.asp?flag=" + smart_connect_flag;
		}else{
			document.form.current_page.value = "device-map/router.asp?time=" + Math.round(new Date().getTime()/1000);
		}
		FormActions("/apply.cgi", "change_wl_unit", "", "");
		document.form.target = "hidden_frame";
		document.form.submit();
	}
}

function disableAdvFn(){
	for(var i=8; i>=1; i--)
		document.getElementById("WLnetworkmap").deleteRow(i);
}

function UIunderRepeater(){
	document.form.wl_auth_mode_x.disabled = true;
	document.form.wl_wep_x.disabled = true;
	document.form.wl_key.disabled = true;
	document.form.wl_asuskey1.disabled = true;
	document.form.wl_wpa_psk.disabled = true;
	document.form.wl_crypto.disabled = true;

	var ssidObj=document.getElementById("wl_ssid");
	ssidObj.name="wlc_ure_ssid";
}

function wl_auth_mode_change(isload){
	var mode = document.form.wl_auth_mode_x.value;

	change_wep_type(mode);
	change_wpa_type(mode);
}

function change_wpa_type(mode){
	var opts = document.form.wl_auth_mode_x.options;
	var new_array;
	var cur_crypto;
	/* enable/disable crypto algorithm */
	if(lyra_hide_support)
		inputCtrl(document.form.wl_crypto, 0);
	else if(mode == "wpa" || mode == "wpa2" || mode == "wpawpa2" || mode == "psk" || mode == "psk2" || mode == "pskpsk2")
		inputCtrl(document.form.wl_crypto, 1);
	else
		inputCtrl(document.form.wl_crypto, 0);
	
	/* enable/disable psk passphrase */
	if(mode == "psk" || mode == "psk2" || mode == "pskpsk2")
		inputCtrl(document.form.wl_wpa_psk, 1);
	else
		inputCtrl(document.form.wl_wpa_psk, 0);
	
	/* update wl_crypto */
	for(var i = 0; i < document.form.wl_crypto.length; ++i)
		if(document.form.wl_crypto[i].selected){
			cur_crypto = document.form.wl_crypto[i].value;
			break;
		}
	
	/* Reconstruct algorithm array from new crypto algorithms */
	if(mode == "psk" || mode == "psk2" || mode == "pskpsk2" || mode == "wpa" || mode == "wpa2" || mode == "wpawpa2"){
		/* Save current crypto algorithm */
			if(opts[opts.selectedIndex].text == "WPA-Personal" || opts[opts.selectedIndex].text == "WPA-Enterprise")
				new_array = new Array("TKIP");
			else if(opts[opts.selectedIndex].text == "WPA2-Personal" || opts[opts.selectedIndex].text == "WPA2-Enterprise")
				new_array = new Array("AES");
			else
				new_array = new Array("AES", "TKIP+AES");
		
		free_options(document.form.wl_crypto);
		for(var i = 0; i < new_array.length; i++){
			document.form.wl_crypto[i] = new Option(new_array[i], new_array[i].toLowerCase());
			document.form.wl_crypto[i].value = new_array[i].toLowerCase();
			if(new_array[i].toLowerCase() == cur_crypto)
				document.form.wl_crypto[i].selected = true;
		}
	}
}

function change_wep_type(mode){

	var cur_wep = document.form.wl_wep_x.value;
	var wep_type_array;
	var value_array;
	var show_wep_x = 0;
	
	free_options(document.form.wl_wep_x);
	
	//if(mode == "shared" || mode == "radius"){ //2009.03 magic
	if(mode == "shared"){ //2009.03 magic
		wep_type_array = new Array("WEP-64bits", "WEP-128bits");
		value_array = new Array("1", "2");
		show_wep_x = 1;
	}
	else if(based_modelid == "RP-AC66"){
		wep_type_array = new Array("None");
		value_array = new Array("0");
		cur_wep = "0";
	}
	else if(mode == "open" && document.form.wl_nmode_x.value == 2){
		wep_type_array = new Array("None", "WEP-64bits", "WEP-128bits");
		value_array = new Array("0", "1", "2");
		show_wep_x = 1;
	}
	else {
		wep_type_array = new Array("None");
		value_array = new Array("0");
		cur_wep = "0";
	}

	add_options_x2(document.form.wl_wep_x, wep_type_array, value_array, cur_wep);
	inputCtrl(document.form.wl_wep_x, show_wep_x);


	change_wlweptype(document.form.wl_wep_x);
}

function change_wlweptype(wep_type_obj){
	if(wep_type_obj.value == "0"){  //2009.03 magic
		inputCtrl(document.form.wl_key, 0);
		inputCtrl(document.form.wl_asuskey1, 0);
	}
	else{
		inputCtrl(document.form.wl_key, 1);
		inputCtrl(document.form.wl_asuskey1, 1);
	}
	
	wl_wep_change();
}

function wl_wep_change(){
	var mode = document.form.wl_auth_mode_x.value;
	var wep = document.form.wl_wep_x.value;
	if ((mode == "shared" || mode == "open") && wep != "0")
		show_key();
}

function show_key(){
	switchType(document.form.wl_asuskey1,true);

	var wep_type = document.form.wl_wep_x.value;
	var keyindex = document.form.wl_key.value;
	var cur_key_obj = eval("document.form.wl_key"+keyindex);
	var cur_key_length = cur_key_obj.value.length;
	
	if(wep_type == 1){
		if(cur_key_length == 5 || cur_key_length == 10)
			document.form.wl_asuskey1.value = cur_key_obj.value;
		else
			document.form.wl_asuskey1.value = ""; //0000000000
	}
	else if(wep_type == 2){
		if(cur_key_length == 13 || cur_key_length == 26)
			document.form.wl_asuskey1.value = cur_key_obj.value;
		else
			document.form.wl_asuskey1.value = ""; //00000000000000000000000000
	}
	else
		document.form.wl_asuskey1.value = "";
	
}

function show_LAN_info(v){
	if(tagged_based_vlan && vlan_enable){
		var vlan_id = get_wl_vlan_id(wireless_unit, wireless_subunit);
		if(vlan_id != ""){
			document.getElementById("vlan_tr").style.display = "";
			showtext(document.getElementById("vlanid"), vlan_id);
		}
		else
			document.getElementById("vlan_tr").style.display = "none";
	}
	else
		document.getElementById("vlan_tr").style.display = "none";

	var lan_ipaddr_t = '<% nvram_get("lan_ipaddr_t"); %>';
	if(lan_ipaddr_t != '')
		showtext(document.getElementById("LANIP"), lan_ipaddr_t);
	else	
		showtext(document.getElementById("LANIP"), '<% nvram_get("lan_ipaddr"); %>');

	if(yadns_support && parent.sw_mode == 1){
		var mode = (yadns_enable != 0) ? yadns_mode : -1;
		showtext(document.getElementById("yadns_mode"), get_yadns_modedesc(mode));
		for(var i = 0; i < 3; i++){
			var visible = (yadns_enable != 0 && i != mode && yadns_clients[i]) ? true : false;
			var modedesc = visible ? get_yadns_modedesc(i) + ": <#Full_Clients#> " + yadns_clients[i] : "";
			showtext2(document.getElementById("yadns_mode" + i), modedesc, visible);
		}
		if (!yadns_hideqis || yadns_enable != 0)
			document.getElementById("yadns_status").style.display = "";
	}

	showtext(document.getElementById("PINCode"), '<% nvram_get("secret_code"); %>');
	showtext(document.getElementById("MAC"), '<% nvram_get("lan_hwaddr"); %>');
	showtext(document.getElementById("MAC_wl2"), '<% nvram_get("wl0_hwaddr"); %>');
	if(document.form.wl_unit.value == '1')
		showtext(document.getElementById("MAC_wl5"), '<% nvram_get("wl1_hwaddr"); %>');
	else if(document.form.wl_unit.value == '2')
		showtext(document.getElementById("MAC_wl5_2"), '<% nvram_get("wl2_hwaddr"); %>');
	else if(document.form.wl_unit.value == '3')
		showtext(document.getElementById("MAC_wl60"), '<% nvram_get("wl3_hwaddr"); %>');

	if(document.form.wl_unit.value == '0'){
		document.getElementById("macaddr_wl5").style.display = "none";
		if(wl_info.band5g_2_support)
			document.getElementById("macaddr_wl5_2").style.display = "none";	
		document.getElementById("macaddr_wl60").style.display = "none";
		if(!band5g_support)
			document.getElementById("macaddr_wl2_title").style.display = "none";
	}
	else if (document.form.wl_unit.value == '1'){
		document.getElementById("macaddr_wl2").style.display = "none";
		document.getElementById("macaddr_wl5_2").style.display = "none";
		document.getElementById("macaddr_wl60").style.display = "none";
		if(wl_info.band5g_2_support)
			document.getElementById("macaddr_wl5_title").innerHTML = "5GHz-1 ";

	}
	else if (document.form.wl_unit.value == '2'){
		document.getElementById("macaddr_wl2").style.display = "none";
		document.getElementById("macaddr_wl5").style.display = "none";
		document.getElementById("macaddr_wl5_2").style.display = "";
		document.getElementById("macaddr_wl60").style.display = "none";
	}
	else if (document.form.wl_unit.value == '3'){
		document.getElementById("macaddr_wl2").style.display = "none";
		document.getElementById("macaddr_wl5").style.display = "none";
		document.getElementById("macaddr_wl5_2").style.display = "none";
		document.getElementById("macaddr_wl60").style.display = "";
	}
	if(smart_connect_support && (parent.isSwMode("rt") || parent.isSwMode("ap"))){
		if(v == '1'){
			showtext(document.getElementById("MAC_wl2"), '<% nvram_get("wl0_hwaddr"); %>');
			showtext(document.getElementById("MAC_wl5"), '<% nvram_get("wl1_hwaddr"); %>');
			showtext(document.getElementById("MAC_wl5_2"), '<% nvram_get("wl2_hwaddr"); %>');
			document.getElementById("macaddr_wl5_title").innerHTML = "5GHz-1 ";
			document.getElementById("macaddr_wl2").style.display = "";
			document.getElementById("macaddr_wl5").style.display = "";
			document.getElementById("macaddr_wl5_2").style.display = "";
			parent.document.getElementById("statusframe").height = 760;
		}else if(document.form.wl_unit.value != '0' && v == '2'){
			document.getElementById("macaddr_wl2").style.display = "none";
			showtext(document.getElementById("MAC_wl5"), '<% nvram_get("wl1_hwaddr"); %>');
			showtext(document.getElementById("MAC_wl5_2"), '<% nvram_get("wl2_hwaddr"); %>');
			document.getElementById("macaddr_wl5_title").innerHTML = "5GHz-1 ";
			document.getElementById("macaddr_wl5").style.display = "";
			document.getElementById("macaddr_wl5_2").style.display = "";
		}else{
			parent.document.getElementById("statusframe").height = 735;
		}
	}
}

var secs;
var timerID = null;
var timerRunning = false;
var timeout = 1000;
var delay = 500;
var stopFlag=0;

function detect_qtn_ready(){
	if(parent.qtn_state_t != "1")
		setTimeout('detect_qtn_ready();', 1000);
	else
		document.form.submit();
}

function submitForm(){
	var auth_mode = document.form.wl_auth_mode_x.value;

	if(document.form.wl_wpa_psk.value == "<#wireless_psk_fillin#>")
		document.form.wl_wpa_psk.value = "";
		
	if(!validator.stringSSID(document.form.wl_ssid))
		return false;
	
	stopFlag = 1;
	document.form.current_page.value = '<% abs_index_page(); %>';
	document.form.next_page.value = '<% abs_index_page(); %>';
	
	if(auth_mode == "psk" || auth_mode == "psk2" || auth_mode == "pskpsk2"){
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
	}
	else if(auth_mode == "wpa" || auth_mode == "wpa2" || auth_mode == "wpawpa2" || auth_mode == "radius"){
		document.form.target = "";
		if(based_modelid=="BRT-AC828"){
			document.form.next_page.value = "/Advanced_Wireless_Content.asp";
		}
		else{
			document.form.next_page.value = "/Advanced_WSecurity_Content.asp";
		}
	}
	else{
		if(!validator.wlKey(document.form.wl_asuskey1))
			return false;
	}
	
	var wep11 = eval('document.form.wl_key'+document.form.wl_key.value);
	wep11.value = document.form.wl_asuskey1.value;
	
	if((auth_mode == "shared" || auth_mode == "wpa" || auth_mode == "wpa2" || auth_mode == "wpawpa2" || auth_mode == "radius")
			&& document.form.wps_enable.value == "1"){
		document.form.wps_enable.value = "0";
	}
	document.form.wsc_config_state.value = "1";

	parent.showLoading();
	if(based_modelid == "RT-AC87U" && "<% nvram_get("wl_unit"); %>" == "1"){
		parent.stopFlag = '0';
		detect_qtn_ready();
	}else {
		document.form.fakepasswordremembered.disabled = true;
		document.form.wl_unit.value = wireless_unit;
		document.form.wl_subunit.value = wireless_subunit;
		if(wireless_subunit == "-1")
			wireless_subunit = "main";
		cookie.set("wireless_unit", wireless_unit, 30);
		cookie.set("wireless_subunit", wireless_subunit, 30);

		var _unit_subunit = "wl" + document.form.wl_unit.value + "." + document.form.wl_subunit.value;
		
		if(captive_portal_used_wl_array[_unit_subunit] != undefined) {
			document.form.wl_key.disabled = true;
			document.form.wl_key1.disabled = true;
			document.form.wl_key2.disabled = true;
			document.form.wl_key3.disabled = true;
			document.form.wl_key4.disabled = true;
			document.form.action_wait.value = 50;
			switch(captive_portal_used_wl_array[_unit_subunit]) {
				case "Free Wi-Fi" :
					document.form.action_script.value = "overwrite_captive_portal_ssid;restart_wireless;";
					break;
				case "Captive Portal Wi-Fi" :
					document.form.action_script.value = "overwrite_captive_portal_adv_ssid;restart_wireless;";
					break;
			}
		}

		document.form.submit();
	}	

	return true;
}

function clean_input(obj){
	if(obj.value == "<#wireless_psk_fillin#>")
			obj.value = "";
}

function gotoSiteSurvey(){
	if(sw_mode == 2)
		parent.location.href = '/QIS_wizard.htm?flag=sitesurvey_rep&band='+wl_unit;
	else
		parent.location.href = '/QIS_wizard.htm?flag=sitesurvey_mb';
}

function startPBCmethod(){
	return 0;
}

function wpsPBC(obj){
	return 0;
}

function manualSetup(){
	return 0;	
}

function tab_reset(v){
	var tab_array1 = document.getElementsByClassName("tab_NW");
	var tab_array2 = document.getElementsByClassName("tabclick_NW");

	var tab_width = Math.floor(270/(wl_info.wl_if_total+1));
	var i = 0;
	while(i < tab_array1.length){
		tab_array1[i].style.width=tab_width+'px';
		tab_array1[i].style.display = "";
	i++;
	}
	if(typeof tab_array2[0] != "undefined"){
		tab_array2[0].style.width=tab_width+'px';
		tab_array2[0].style.display = "";
	}
	if(v == 0){
		document.getElementById("span0").innerHTML = "2.4GHz";
		if(wl_info.band5g_2_support){
			document.getElementById("span1").innerHTML = "5GHz-1";
			document.getElementById("span2").innerHTML = "5GHz-2";
		}else{
			document.getElementById("span1").innerHTML = "5GHz";
			document.getElementById("t2").style.display = "none";
		}

		if(!wl_info.band60g_support){
			document.getElementById("t3").style.display = "none";
		}
	}else if(v == 1){	//Smart Connect
		if(based_modelid == "RT-AC5300" || based_modelid == "RT-AC3200" || based_modelid == "GT-AC5300")
			document.getElementById("span0").innerHTML = "2.4GHz, 5GHz-1 and 5GHz-2";
		else if(based_modelid == "RT-AC88U" || based_modelid == "RT-AC86U" || based_modelid == "AC2900" || based_modelid == "RT-AC3100")
			document.getElementById("span0").innerHTML = "2.4GHz and 5GHz";
		
		document.getElementById("t1").style.display = "none";
		document.getElementById("t2").style.display = "none";				
		document.getElementById("t0").style.width = (tab_width*wl_info.wl_if_total+10) +'px';
	}
	else if(v == 2){ //5GHz Smart Connect
		document.getElementById("span0").innerHTML = "2.4GHz";
		document.getElementById("span1").innerHTML = "5GHz-1 and 5GHz-2";
		document.getElementById("t2").style.display = "none";	
		document.getElementById("t1").style.width = "148px";
		document.getElementById("span1").style.padding = "5px 4px 5px 9px";
	}
}

function change_smart_connect(v){
	document.form.smart_connect_x.value = v;
	if(based_modelid=="RT-AC5300")
		document.form.smart_connect_t.value = v;

	show_LAN_info(v);
	switch(v){
		case '0':
				tab_reset(0);	
				break;
		case '1': 
				if(wl_unit != '0')
					tabclickhandler(0);
				else
					tab_reset(1);
				break;
		case '2': 
				if(!(wl_unit == '0' || wl_unit == '1'))
					tabclickhandler(1);
				else
					tab_reset(2);
				break;
	}
}
function limit_auth_method(){
	var auth_method_array = document.form.wl_auth_mode_x.value;
	if(sw_mode == 2){
		if((based_modelid == "RT-AC87U" && '<% nvram_get("wl_unit"); %>' == '1'))
			var auth_array = [["Open System", "open"], ["WPA2-Personal", "psk2"], ["WPA-Auto-Personal", "pskpsk2"]];
		else
			var auth_array = [["Open System", "open"], ["Shared Key", "shared"], ["WPA-Personal", "psk"], ["WPA2-Personal", "psk2"], ["WPA-Auto-Personal", "pskpsk2"]];
	}
	else if(document.form.wl_nmode_x.value != "2"){
		if((based_modelid == "RT-AC87U" && '<% nvram_get("wl_unit"); %>' == '1') || (based_modelid == "RT-AC87U" && g_unit))
			var auth_array = [["Open System", "open"], ["WPA2-Personal", "psk2"], ["WPA-Auto-Personal", "pskpsk2"]];
		else
			var auth_array = [["Open System", "open"], ["WPA2-Personal", "psk2"], ["WPA-Auto-Personal", "pskpsk2"], ["WPA2-Enterprise", "wpa2"], ["WPA-Auto-Enterprise", "wpawpa2"]];
	}
	else{		//Legacy
		if(wifi_logo_support)
			var auth_array = [["Open System", "open"], ["WPA2-Personal", "psk2"], ["WPA-Auto-Personal", "pskpsk2"], ["WPA-Enterprise", "wpa"], ["WPA2-Enterprise", "wpa2"], ["WPA-Auto-Enterprise", "wpawpa2"]];
		else
			var auth_array = [["Open System", "open"], ["Shared Key", "shared"], ["WPA-Personal", "psk"], ["WPA2-Personal", "psk2"], ["WPA-Auto-Personal", "pskpsk2"], ["WPA-Enterprise", "wpa"], ["WPA2-Enterprise", "wpa2"], ["WPA-Auto-Enterprise", "wpawpa2"], ["Radius with 802.1x", "radius"]];
	}

	if(wireless_subunit != "" && wireless_subunit != null && wireless_subunit != "-1") {
		var auth_array = [["Open System", "open"], ["WPA2-Personal", "psk2"], ["WPA-Auto-Personal", "pskpsk2"]];
	}

	if(is_KR_sku){	// MODELDEP by Territory_code
		auth_array.splice(0, 1); //remove Open System
	}	
	
	free_options(document.form.wl_auth_mode_x);
	for(i = 0; i < auth_array.length; i++){
		if(auth_method_array  == auth_array[i][1])
			add_option(document.form.wl_auth_mode_x, auth_array[i][0], auth_array[i][1], 1);
		else
			add_option(document.form.wl_auth_mode_x, auth_array[i][0], auth_array[i][1], 0);	
	}
		
	authentication_method_change(document.form.wl_auth_mode_x);
}
</script>
</head>
<body class="statusbody" onload="initial();">
<iframe name="hidden_frame" id="hidden_frame" width="0" height="0" frameborder="0"></iframe>
<form method="post" name="form" id="form" action="/start_apply2.htm">
<input type="hidden" name="current_page" value="device-map/router.asp">
<input type="hidden" name="next_page" value="">
<input type="hidden" name="action_mode" value="apply_new">
<input type="hidden" name="action_script" value="restart_wireless">
<input type="hidden" name="action_wait" value="8">
<input type="hidden" name="productid" value="<% nvram_get("productid"); %>">
<input type="hidden" name="wps_enable" value="<% nvram_get("wps_enable"); %>">
<input type="hidden" name="wsc_config_state" value="<% nvram_get("wsc_config_state"); %>">
<input type="hidden" name="wl_key1" value="">
<input type="hidden" name="wl_key2" value="">
<input type="hidden" name="wl_key3" value="">
<input type="hidden" name="wl_key4" value="">
<input type="hidden" name="wl_ssid_org" value="<% nvram_char_to_ascii("WLANConfig11b", "wl_ssid"); %>">
<input type="hidden" name="wlc_ure_ssid_org" value="<% nvram_char_to_ascii("WLANConfig11b", "wlc_ure_ssid"); %>" disabled>
<input type="hidden" name="wl_wpa_psk_org" value="<% nvram_char_to_ascii("WLANConfig11b", "wl_wpa_psk"); %>">
<input type="hidden" name="wl_auth_mode_orig" value="<% nvram_get("wl_auth_mode_x"); %>">
<input type="hidden" name="wl_wep_x_orig" value="<% nvram_get("wl_wep_x"); %>">
<input type="hidden" name="wl_key_type" value="<% nvram_get("wl_key_type"); %>"><!--Lock Add 1125 for ralink platform-->
<input type="hidden" name="wl_key_org" value="<% nvram_char_to_ascii("WLANConfig11b", "wl_key"); %>">
<input type="hidden" name="wl_key1_org" value="<% nvram_char_to_ascii("WLANConfig11b", "wl_key1"); %>">
<input type="hidden" name="wl_key2_org" value="<% nvram_char_to_ascii("WLANConfig11b", "wl_key2"); %>">
<input type="hidden" name="wl_key3_org" value="<% nvram_char_to_ascii("WLANConfig11b", "wl_key3"); %>">
<input type="hidden" name="wl_key4_org" value="<% nvram_char_to_ascii("WLANConfig11b", "wl_key4"); %>">
<input type="hidden" name="wl_nmode_x" value="<% nvram_get("wl_nmode_x"); %>"><!--Lock Add 20091210 for n only-->
<input type="hidden" name="wps_band" value="<% nvram_get("wps_band_x"); %>">
<input type="hidden" name="wl_unit" value="<% nvram_get("wl_unit"); %>">
<input type="hidden" name="wl_subunit" value="-1">
<input type="hidden" name="wl_radio" value="<% nvram_get("wl_radio"); %>">
<input type="hidden" name="wl_txbf" value="<% nvram_get("wl_txbf"); %>">
<input type="hidden" name="wl_mumimo" value="<% nvram_get("wl_mumimo"); %>">
<input type="hidden" name="smart_connect_x" value="<% nvram_get("smart_connect_x"); %>">
<input type="hidden" name="wl_bss_enabled" value="<% nvram_get("wl_bss_enabled"); %>">
<input type="hidden" name="wl_mbss" value="1">

<table border="0" cellpadding="0" cellspacing="0" id="rt_table">
<tr>
	<td>		
		<table width="100px" border="0" align="left" style="margin-left:8px;" cellpadding="0" cellspacing="0">
			<td>
				<div id="t0" class="tab_NW" align="center" style="font-weight: bolder;display:none; margin-right:2px; width:90px;" onclick="tabclickhandler(0)">
					<span id="span0" style="cursor:pointer;font-weight: bolder;">2.4GHz</span>
				</div>
			</td>
			<td>
				<div id="t1" class="tab_NW" align="center" style="font-weight: bolder;display:none; margin-right:2px; width:90px;" onclick="tabclickhandler(1)">
					<span id="span1" style="cursor:pointer;font-weight: bolder;">5GHz</span>
				</div>
			</td>
			<td>
				<div id="t2" class="tab_NW" align="center" style="font-weight: bolder;display:none; margin-right:2px; width:90px;" onclick="tabclickhandler(2)">
					<span id="span2" style="cursor:pointer;font-weight: bolder;">5GHz-2</span>
				</div>
			</td>
			<td>
				<div id="t3" class="tab_NW" align="center" style="font-weight: bolder;display:none; margin-right:2px; width:90px;" onclick="tabclickhandler(3)">
					<span id="span3" style="cursor:pointer;font-weight: bolder;">60GHz</span>
				</div>
			</td>
			<td>
				<div id="t_status" class="tab_NW" align="center" style="font-weight: bolder; margin-right:2px; width:90px;" onclick="tabclickhandler('status')">
					<span id="span_status" style="cursor:pointer;font-weight: bolder;">Status</span>
				</div>
			</td>
		</table>
	</td>
</tr>

<tr>
	<td>
		<table width="95%" border="1" align="center" cellpadding="4" cellspacing="0" class="table1px" id="WLnetworkmap_re" style="display:none">
		  <tr>
		    <td height="50" style="padding:10px 15px 0px 15px;">
		    	<p class="formfonttitle_nwm" style="float:left;"><#APSurvey_action_search_again_hint2#></p>
					<br />
			  	<input type="button" class="button_gen" onclick="gotoSiteSurvey();" value="<#QIS_rescan#>" style="float:right;">
			  	<!--input type="button" class="button_gen" onclick="manualSetup();" value="<#Manual_Setting_btn#>" style="float:right;"-->
     			<img style="margin-top:5px; *margin-top:-10px; visibility:hidden;" src="/images/New_ui/networkmap/linetwo2.png">
		    </td>
		  </tr>
		</table>

		<table width="95%" border="1" align="center" cellpadding="4" cellspacing="0" class="table1px" id="WLnetworkmap">
               <tr id="smart_connect_field" style="display:none">
                       <td style="padding:5px 10px 0px 10px; *padding:1px 10px 0px 10px;">
                               <p class="formfonttitle_nwm" >Smart Connect</p>
                               <select style="*margin-top:-7px;" name="smart_connect_t" class="input_option" onchange="change_smart_connect(this.value);"></select>                               
                               <img style="margin-top:5px; *margin-top:-10px;"src="/images/New_ui/networkmap/linetwo2.png">
                       </td>
               </tr>
  		<tr id="smartcon_enable_field" style="display:none">
			  	<td>
			  	<div><table><tr>
			  		<td style="padding:8px 5px 0px 0px;">
			  			<p class="formfonttitle_nwm" >Smart Connect: </p>
			  		</td>
			  		<td>
					<div id="smartcon_enable_block" style="display:none;">
			    		<span style="color:#FFF;" id="smart_connect_enable_word">&nbsp;&nbsp;</span>
			    		<input type="button" name="enableSmartConbtn" id="enableSmartConbtn" value="" class="button_gen" onClick="change_smart_connect();">
			    		<br>
			    	</div>
			    		<div class="left" style="width: 94px;" id="radio_smartcon_enable"></div>
						<div class="clear"></div>					
						<script type="text/javascript">
								var flag = '<% get_parameter("flag"); %>';

							if(flag == '')
								smart_connect_flag_t = '<% nvram_get("smart_connect_x"); %>';
							else
								smart_connect_flag_t = flag;

								$('#radio_smartcon_enable').iphoneSwitch(smart_connect_flag_t>0, 
								 function() {
									change_smart_connect('1');
								 },
								 function() {
									change_smart_connect('0');
								 }
							);
						</script>			  			
			  		</td>
			  	</tr></table></div>
		  	  </td>			
  		</tr>  		
  		<tr id="smartcon_enable_line" style="display:none"><td><img style="margin-top:-2px; *margin-top:-10px;"src="/images/New_ui/networkmap/linetwo2.png"></td></tr>
  		<tr id="tr_guestnetwork_control">
  			<td>
  				<table>
  					<tr>
  						<td style="padding:8px 5px 0px 2px;">
  							<p class="formfonttitle_nwm" >Guest Status: </p>
  						</td>
  						<td>
  							<div class="left" style="width:94px;" id="radio_gn_enable"></div>
  						</td>
  					</tr>
  					<tr>
  						<td colspan="2">
  							<img style="margin-left:2px;" src="/images/New_ui/networkmap/linetwo2.png">
  						</td>
  					</tr>
  				</table>
  			</td>
  		</tr>
  		<tr>
    			<td style="padding:5px 10px 0px 10px; ">
  	  			<p class="formfonttitle_nwm" ><#QIS_finish_wireless_item1#></p>
      			<input style="*margin-top:-7px; width:260px;" id="wl_ssid" type="text" name="wl_ssid" value="<% nvram_get("wl_ssid"); %>" maxlength="32" size="22" class="input_25_table" autocomplete="off" autocorrect="off" autocapitalize="off">
      			<img style="margin-top:5px; *margin-top:-10px;"src="/images/New_ui/networkmap/linetwo2.png">
    			</td>
  		</tr>  
  		<tr>
    			<td style="padding:5px 10px 0px 10px; *padding:1px 10px 0px 10px;">
    					<p class="formfonttitle_nwm" ><#WLANConfig11b_AuthenticationMethod_itemname#></p>
				  		<select style="*margin-top:-7px;" name="wl_auth_mode_x" class="input_option" onChange="authentication_method_change(this);">
							<option value="open"    <% nvram_match("wl_auth_mode_x", "open",   "selected"); %>>Open System</option>
							<option value="shared"  <% nvram_match("wl_auth_mode_x", "shared", "selected"); %>>Shared Key</option>
							<option value="psk"     <% nvram_match("wl_auth_mode_x", "psk",    "selected"); %>>WPA-Personal</option>
							<option value="psk2"    <% nvram_match("wl_auth_mode_x", "psk2",   "selected"); %>>WPA2-Personal</option>
							<option value="pskpsk2" <% nvram_match("wl_auth_mode_x", "pskpsk2","selected"); %>>WPA-Auto-Personal</option>
							<option value="wpa"     <% nvram_match("wl_auth_mode_x", "wpa",    "selected"); %>>WPA-Enterprise</option>
							<option value="wpa2"    <% nvram_match("wl_auth_mode_x", "wpa2",   "selected"); %>>WPA2-Enterprise</option>
							<option value="wpawpa2" <% nvram_match("wl_auth_mode_x", "wpawpa2","selected"); %>>WPA-Auto-Enterprise</option>
							<option value="radius"  <% nvram_match("wl_auth_mode_x", "radius", "selected"); %>>Radius with 802.1x</option>
				  		</select>
							<img style="display:none;margin-top:-30px;margin-left:185px;cursor:pointer;" id="wl_nmode_x_hint" src="/images/alertImg.png" width="30px" onClick="parent.overlib(parent.helpcontent[0][24], parent.FIXX, 870, parent.FIXY, 350);" onMouseOut="parent.nd();">
	  					<img style="margin-top:5px; *margin-top:-10px;"src="/images/New_ui/networkmap/linetwo2.png">
    			</td>
  		</tr>
  		<tr id='all_related_wep' style='display:none;'>
			<td style="padding:5px 10px 0px 10px; *padding:1px 10px 0px 10px;">
				<p class="formfonttitle_nwm" ><#WLANConfig11b_WEPType_itemname#></p>
	  			<select style="*margin-top:-7px;" name="wl_wep_x" id="wl_wep_x" class="input_option" onchange="change_wlweptype(this);">
						<option value="0" <% nvram_match("wl_wep_x", "0", "selected"); %>><#wl_securitylevel_0#></option>
						<option value="1" <% nvram_match("wl_wep_x", "1", "selected"); %>>WEP-64bits</option>
						<option value="2" <% nvram_match("wl_wep_x", "2", "selected"); %>>WEP-128bits</option>
	  			</select>	  			
	  			<img style="margin-top:5px; *margin-top:-10px;"src="/images/New_ui/networkmap/linetwo2.png">
			</td>
  		</tr>
  		<tr id='all_wep_key' style='display:none;'>
    			<td style="padding:5px 10px 0px 10px; *padding:1px 10px 0px 10px;">
    				<p class="formfonttitle_nwm" ><#WLANConfig11b_WEPDefaultKey_itemname#></p>
      				<select style="*margin-top:-7px;" name="wl_key" class="input_option" onchange="show_key();">
					<option value="1" <% nvram_match("wl_key", "1", "selected"); %>>1</option>
					<option value="2" <% nvram_match("wl_key", "2", "selected"); %>>2</option>
					<option value="3" <% nvram_match("wl_key", "3", "selected"); %>>3</option>
					<option value="4" <% nvram_match("wl_key", "4", "selected"); %>>4</option>
      			</select>      			
	  			<img style="margin-top:5px; *margin-top:-10px;" src="/images/New_ui/networkmap/linetwo2.png">
    			</td>
  		</tr>
  		<tr id='asus_wep_key'>
    			<td style="padding:5px 10px 0px 10px; ">
	    			<p class="formfonttitle_nwm" ><#WLANConfig11b_WEPKey_itemname#>
						</p>
							<input id="wl_asuskey1" name="wl_asuskey1" style="width:260px;*margin-top:-7px;" type="password" onBlur="switchType(this, false);" onFocus="switchType(this, true);" onKeyUp="return change_wlkey(this, 'WLANConfig11b');" value="" maxlength="27" class="input_25_table" autocorrect="off" autocapitalize="off">
      			<img style="margin-top:5px; *margin-top:-10px;"src="/images/New_ui/networkmap/linetwo2.png">
    			</td>
  		</tr>
  		<tr id='wl_crypto' style='display:none;'>
			<td style="padding:5px 10px 0px 10px; *padding:1px 10px 0px 10px;">
	  			<p class="formfonttitle_nwm" ><#WLANConfig11b_WPAType_itemname#></p>
	  			<select style="*margin-top:-7px;" name="wl_crypto" class="input_option" onchange="wl_auth_mode_change(0);">
					<!--option value="tkip" <% nvram_match("wl_crypto", "tkip", "selected"); %>>TKIP</option-->
					<option value="aes" <% nvram_match("wl_crypto", "aes", "selected"); %>>AES</option>
					<option value="tkip+aes" <% nvram_match("wl_crypto", "tkip+aes", "selected"); %>>TKIP+AES</option>
	  			</select>	  			
	  			<img style="margin-top:5px; *margin-top:-10px;"src="/images/New_ui/networkmap/linetwo2.png">
			</td>
  		</tr>
  		<tr id='wl_wpa_psk_tr' style='display:none'>
    			<td style="padding:5px 10px 0px 10px;">
					<p id="wl_wpa_psk_title" class="formfonttitle_nwm" ><#WPA-PSKKey#></p>
							<input id="wl_wpa_psk" name="wl_wpa_psk" style="width:260px;*margin-top:-7px;" type="password" onBlur="switchType(this, false);" onFocus="switchType(this, true);" value="" maxlength="64" class="input_25_table" autocomplete="off" autocorrect="off" autocapitalize="off"/>
      						<!-- fake fields are a workaround for chrome autofill getting the wrong fields -->
      						<input style="display:none" type="password" name="fakepasswordremembered"/>
      			<img style="margin-top:5px; *margin-top:-10px;"src="/images/New_ui/networkmap/linetwo2.png">
    			</td>
  		</tr>

  		<tr id="wl_radio_tr" style="display:none">
			<td style="padding:5px 10px 0px 10px;">
	    			<p class="formfonttitle_nwm" style="float:left;"><#Wireless_Radio#></p>
				<div class="left" style="width:94px; float:right;" id="radio_wl_radio"></div>
				<div class="clear"></div>
				<script type="text/javascript">
					//
					$('#radio_wl_radio').iphoneSwitch('<% nvram_get("wl_radio"); %>', 
							 function() {
								document.form.wl_radio.value = "1";
							 },
							 function() {
								document.form.wl_radio.value = "0";
							 }
						);
				</script>
      			<img style="margin-top:5px; *margin-top:-10px;"src="/images/New_ui/networkmap/linetwo2.png">
			</td>
  		</tr>
  		<!--   Viz add 2011.12 for RT-N56U Ralink  start {{ -->
  		<tr id="wl_txbf_tr" style="display:none">
			<td style="padding:5px 10px 0px 10px;">
	    			<p class="formfonttitle_nwm" style="float:left;">AiRadar</p>
				<div class="left" style="width:94px; float:right;" id="radio_wl_txbf"></div>
				<div class="clear"></div>
				<script type="text/javascript">
					
					$('#radio_wl_txbf').iphoneSwitch('<% nvram_get("wl_txbf"); %>', 
							 function() {
								document.form.wl_txbf.value = "1";
								return true;
							 },
							 function() {
								document.form.wl_txbf.value = "0";
								return true;
							 }
						);
				</script>
      			<img style="margin-top:5px; *margin-top:-10px;"src="/images/New_ui/networkmap/linetwo2.png">
			</td>
  		</tr>  		
  		<!--   Viz add 2011.12 for RT-N56U Ralink   end  }} -->			
  		<tr id='wl_captive_portal_tr' style='display:none'>
			<td style="padding:5px 10px 0px 10px;">
				<p class="formfonttitle_nwm"><#Access_Control_title#></p>
				<p style="padding-left:10px; margin-top:3px; *margin-top:-5px; margin-right:10px; background-color:#444f53; line-height:20px;" id="wl_captive_portal_text"></p>
				<img style="margin-top:5px; *margin-top:-10px;"src="/images/New_ui/networkmap/linetwo2.png">
			</td>
		</tr>	
 		</table>		
  	</td>
</tr>

<tr>
	<td> 			
 		<table width="95%" border="1" align="center" cellpadding="4" cellspacing="0" class="table1px">
  		<tr id="apply_tr">
    			<td style="border-bottom:5px #2A3539 solid;padding:5px 10px 5px 10px;">
    				<input id="applySecurity" type="button" class="button_gen" value="<#CTL_apply#>" onclick="submitForm();" style="margin-left:90px;">
    			</td>
  		</tr>
  		<tr id="vlan_tr" style="display: none;">
    			<td style="padding:10px 10px 0px 10px;">
    				<p class="formfonttitle_nwm" >VLAN ID</p>
    				<p style="padding-left:10px; margin-top:3px; *margin-top:-5px; margin-right:10px; background-color:#444f53; line-height:20px;" id="vlanid"></p>
      			<img style="margin-top:5px; *margin-top:-10px;" src="/images/New_ui/networkmap/linetwo2.png">
    			</td>
  		</tr>
  		<tr>
    			<td style="padding:10px 10px 0px 10px;">
    				<p class="formfonttitle_nwm" ><#LAN_IP#></p>
    				<p class="tab_info_bg" style="padding-left:10px; margin-top:3px; *margin-top:-5px; margin-right:10px; line-height:20px;" id="LANIP"></p>
      			<img style="margin-top:5px; *margin-top:-10px;" src="/images/New_ui/networkmap/linetwo2.png">
    			</td>
  		</tr>  
  		<tr>
    			<td style="padding:5px 10px 0px 10px;">
    				<p class="formfonttitle_nwm" ><#PIN_code#></p>
    				<p class="tab_info_bg" style="padding-left:10px; margin-top:3px; *margin-top:-5px; margin-right:10px;line-height:20px;" id="PINCode"></p>
      			<img style="margin-top:5px; *margin-top:-10px;" src="/images/New_ui/networkmap/linetwo2.png">
    			</td>
  		</tr>
  		<tr id="yadns_status" style="display:none;">
    			<td style="padding:5px 10px 0px 10px;">
    				<p class="formfonttitle_nwm" ><#YandexDNS#></p>
    				<a href="/YandexDNS.asp" target="_parent">
    				<p class="tab_info_bg" style="padding-left:10px; margin-top:3px; *margin-top:-5px; margin-right:10px;line-height:20px;" id="yadns_mode"></p>
    				<p class="tab_info_bg" style="padding-left:10px; margin-top:3px; *margin-top:-5px; margin-right:10px;line-height:20px;" id="yadns_mode0"></p>
    				<p class="tab_info_bg" style="padding-left:10px; margin-top:3px; *margin-top:-5px; margin-right:10px;line-height:20px;" id="yadns_mode1"></p>
    				<p class="tab_info_bg" style="padding-left:10px; margin-top:3px; *margin-top:-5px; margin-right:10px;line-height:20px;" id="yadns_mode2"></p>
    				</a>
      			<img style="margin-top:5px; *margin-top:-10px;" src="/images/New_ui/networkmap/linetwo2.png">
    			</td>
  		</tr>
  		<tr>
    			<td style="padding:5px 10px 0px 10px;">
    				<p class="formfonttitle_nwm" >LAN <#MAC_Address#></p>
    				<p class="tab_info_bg" style="padding-left:10px; margin-top:3px; *margin-top:-5px; padding-bottom:3px; margin-right:10px;line-height:20px;" id="MAC"></p>
    				<img style="margin-top:5px; *margin-top:-10px;" src="/images/New_ui/networkmap/linetwo2.png">
    			</td>
  		</tr>     
  		<tr id="macaddr_wl2">
    			<td style="padding:5px 10px 0px 10px;">
    				<p class="formfonttitle_nwm" >Wireless <span id="macaddr_wl2_title">2.4GHz </span><#MAC_Address#></p>
    				<p class="tab_info_bg" style="padding-left:10px; margin-bottom:5px; margin-top:3px; *margin-top:-5px; padding-bottom:3px; margin-right:10px;line-height:20px;" id="MAC_wl2"></p>
    			</td>
  		</tr>     
  		<tr id="macaddr_wl5">
    			<td style="padding:5px 10px 0px 10px;">
    				<p class="formfonttitle_nwm" >Wireless <span id="macaddr_wl5_title">5GHz </span><#MAC_Address#></p>
    				<p class="tab_info_bg" style="padding-left:10px; margin-bottom:5px; margin-top:3px; *margin-top:-5px; padding-bottom:3px; margin-right:10px;line-height:20px;" id="MAC_wl5"></p>
    			</td>
  		</tr>
  		<tr id="macaddr_wl5_2" style="display:none;">
    			<td style="padding:5px 10px 0px 10px;">
    				<p class="formfonttitle_nwm" >Wireless <span id="macaddr_wl5_2_title">5GHz-2 </span><#MAC_Address#></p>
    				<p class="tab_info_bg" style="padding-left:10px; margin-bottom:5px; margin-top:3px; *margin-top:-5px; padding-bottom:3px; margin-right:10px;line-height:20px;" id="MAC_wl5_2"></p>
    			</td>
  		</tr>  
		<tr id="macaddr_wl60" style="display:none;">
			<td style="padding:5px 10px 0px 10px;">
				<p class="formfonttitle_nwm" >Wireless <span id="macaddr_wl60_title">60GHz </span><#MAC_Address#></p>
				<p class="tab_info_bg" style="padding-left:10px; margin-bottom:5px; margin-top:3px; *margin-top:-5px; padding-bottom:3px; margin-right:10px;line-height:20px;" id="MAC_wl60"></p>
			</td>
		</tr>
		</table>
	</td>
</tr>
</table>			
</form>
<form method="post" name="WPSForm" id="WPSForm" action="/stawl_apply.htm">
<input type="hidden" name="current_page" value="">
<input type="hidden" name="action_script" value="">
<input type="hidden" name="flag" value="">
</form>

<form method="post" name="stopPINForm" id="stopPINForm" action="/start_apply.htm" target="hidden_frame">
<input type="hidden" name="current_page" value="">
<input type="hidden" name="group_id" value="">
<input type="hidden" name="action_mode" value="">
<input type="hidden" name="wsc_config_command" value="<% nvram_get("wsc_config_command"); %>">
</form>
</body>
</html>
