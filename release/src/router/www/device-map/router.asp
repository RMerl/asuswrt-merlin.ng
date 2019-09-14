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
<script type="text/javascript" src="/js/httpApi.js"></script>
<script type="text/javascript" src="/general.js"></script>
<script type="text/javascript" src="/state.js"></script>
<script type="text/javascript" src="/js/jquery.js"></script>
<script type="text/javascript" src="/validator.js"></script>
<script type="text/javascript" src="/switcherplugin/jquery.iphone-switch.js"></script>
<style>
.cancel{
	border: 2px solid #898989;
	border-radius:50%;
	background-color: #898989;
}
.check{
	border: 2px solid #093;
	border-radius:50%;
	background-color: #093;
}
.cancel, .check{
	width: 22px;
	height: 22px;
	margin:0 auto;
	transition: .35s;
}
.cancel:active, .check:active{
	transform: scale(1.5,1.5);
	opacity: 0.5;
	transition: 0;
}
.all_enable{
	border: 1px solid #393;
	color: #393;
}
.all_disable{
	border: 1px solid #999;
	color: #999;
}
.ui-slider {
	position: relative;
	text-align: left;
}
.ui-slider .ui-slider-handle {
	position: absolute;
	width: 12px;
	height: 12px;
}
.ui-slider .ui-slider-range {
	position: absolute;
}
.ui-slider-horizontal {
	height: 6px;
}

.ui-widget-content {
	border: 1px solid #000;
	background-color:#000;
}
.ui-state-default,
.ui-widget-content .ui-state-default,
.ui-widget-header .ui-state-default {
	border: 1px solid ;
	background: #e6e6e6;
	margin-top:-4px;
	margin-left:-6px;
}

/* Corner radius */
.ui-corner-all,
.ui-corner-top,
.ui-corner-left,
.ui-corner-tl {
	border-top-left-radius: 4px;
}
.ui-corner-all,
.ui-corner-top,
.ui-corner-right,
.ui-corner-tr {
	border-top-right-radius: 4px;
}
.ui-corner-all,
.ui-corner-bottom,
.ui-corner-left,
.ui-corner-bl {
	border-bottom-left-radius: 4px;
}
.ui-corner-all,
.ui-corner-bottom,
.ui-corner-right,
.ui-corner-br {
	border-bottom-right-radius: 4px;
}

.ui-slider-horizontal .ui-slider-range {
	top: 0;
	height: 100%;
}

#slider .ui-slider-range {
	background: #3367d6;
	border-top-left-radius: 3px;
	border-top-right-radius: 1px;
	border-bottom-left-radius: 3px;
	border-bottom-right-radius: 1px;
}

#slider .ui-slider-handle { border-color: #3367d6; }	
.dwb_hint {
	color:#FC0;
	margin-left:5px;
	font-size:13px;
}
</style>
<script>
if(parent.location.pathname !== '<% abs_networkmap_page(); %>' && parent.location.pathname !== "/") top.location.href = "../"+'<% networkmap_page(); %>';

$(function () {
	var dynamic_include_js = function(_src) {
		$('<script>')
			.attr('type', 'text/javascript')
			.attr('src', _src)
			.appendTo('head');
	};
	if(parent.amesh_support && (parent.isSwMode("rt") || parent.isSwMode("ap")) && parent.ameshRouter_support)
		dynamic_include_js('/require/modules/amesh.js');
	if(parent.smart_connect_support && (parent.isSwMode("rt") || parent.isSwMode("ap")))
		dynamic_include_js('/switcherplugin/jquery.iphone-switch.js');
	if(parent.lantiq_support)
		dynamic_include_js('/calendar/jquery-ui.js');
});

<% wl_get_parameter(); %>
var flag = '<% get_parameter("flag"); %>';
var wl_unit = '<% nvram_get("wl_unit"); %>';

if(parent.yadns_support){
	var yadns_enable = '<% nvram_get("yadns_enable_x"); %>';
	var yadns_mode = '<% nvram_get("yadns_mode"); %>';
	var yadns_clients = [ <% yadns_clients(); %> ];
}

var color_table = ["#c6dafc", "#7baaf7", "#4285f4", "#3367d6"];
var led_table = ["Off", "Low", "Medium", "Highest"];
function initial(){
	var wl_subunit = '<% nvram_get("wl_subunit"); %>';

	if(parent.lantiq_support){
		$("#led_tr").show();
		checkWLReady();
		register_event();
		var led = httpApi.nvramGet(["bc_ledLv"]);
		if(led.bc_ledLv == 3){
			translated_value = 100;
		}
		else if(led.bc_ledLv == 2){
			translated_value = 67;
		}
		else if(led.bc_ledLv == 1){
			translated_value = 33;
		}
		else{		// led.bc_ledLv is 0
			translated_value = 0;
		}

		document.getElementById('slider').children[0].style.width = translated_value + "%";
		document.getElementById('slider').children[1].style.left = translated_value + "%";
		//$("#color_pad").css("background-color", color_table[led.bc_ledLv]);
		$("#color_pad").html(led_table[led.bc_ledLv]);
		$("#slider .ui-slider-range").css("background-color", color_table[led.bc_ledLv]);
		$("#slider .ui-slider-handle").css("border-color", color_table[led.bc_ledLv]);
	}

	if(wifison_ready == "1" && parent.sw_mode == "1"){
		document.getElementById("t0").style.display = "";
		document.getElementById("span0").innerHTML = "<#tm_wireless#>";
		document.getElementById("t0").className = "tabclick_NW";
		document.getElementById("wl_wpa_psk_title").innerHTML = "<#Network_key#>";
		document.getElementById("t3").style.display = "none";
		document.getElementById("pincode").style.display = "none";
	}
	else{
		if(parent.band5g_support){
			document.getElementById("t0").style.display = "";
			document.getElementById("t1").style.display = "";
			if(parent.wl_info.band5g_2_support){
				document.getElementById("t2").style.display = "";
				tab_reset(0);
			}
			if(parent.wl_info.band60g_support) {
				document.getElementById("t3").style.display = "";
				tab_reset(0);
			}

			// disallow to use the other band as a wireless AP
			if(parent.isSwMode("mb") && !localAP_support){
				for(var x=0; x<parent.wl_info.wl_if_total;x++){
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
		if(wl_unit == "-1" || wl_subunit != "-1"){
			tabclickhandler(0);
		}
	}

	// modify wlX.1_ssid(SSID to end clients) under repeater mode
	if((parent.sw_mode == 2 || parent.sw_mode == 4) && '<% nvram_get("wlc_band"); %>' == wl_unit)
		document.form.wl_subunit.value = 1;
	else
		document.form.wl_subunit.value = -1;

	// modify wlX.1_ssid(SSID to end clients) under concurrent repeater mode
	if(parent.sw_mode == 2 && parent.concurrep_support)
		document.form.wl_subunit.value = 1;

	if(parent.smart_connect_support && (parent.isSwMode("rt") || parent.isSwMode("ap"))){
		var value = new Array();
		var desc = new Array();
		if(isSupport("triband") && dwb_info.mode) {
			desc = ["none", "Dual-Band Smart Connect"];
			value = ["0", "1"];
		}
		else {
			if(wl_info.band2g_support && wl_info.band5g_support && wl_info.band5g_2_support){
				desc = ["none", "Tri-Band Smart Connect", "5GHz Smart Connect"];
				value = ["0", "1", "2"];
			}
			else if(wl_info.band2g_support && wl_info.band5g_support){
				desc = ["none", "Dual-Band Smart Connect"];
				value = ["0", "1"];
			}
		}
		add_options_x2(document.form.smart_connect_t, desc, value, document.form.smart_connect_x.value);

		if(document.form.smart_connect_x.value !=0)
			document.getElementById("smart_connect_field").style.display = '';
	}

	if(wifison_ready != "1" || parent.sw_mode != "1")
		change_tabclick();

	document.form.wl_ssid.value = decodeURIComponent('<% nvram_char_to_ascii("", "wl_ssid"); %>');
	document.form.wl_wpa_psk.value = decodeURIComponent('<% nvram_char_to_ascii("", "wl_wpa_psk"); %>');
	document.form.wl_key1.value = decodeURIComponent('<% nvram_char_to_ascii("", "wl_key1"); %>');
	document.form.wl_key2.value = decodeURIComponent('<% nvram_char_to_ascii("", "wl_key2"); %>');
	document.form.wl_key3.value = decodeURIComponent('<% nvram_char_to_ascii("", "wl_key3"); %>');
	document.form.wl_key4.value = decodeURIComponent('<% nvram_char_to_ascii("", "wl_key4"); %>');

	/* Viz banned 2012.06
	if(document.form.wl_wpa_psk.value.length <= 0)
		document.form.wl_wpa_psk.value = "<#wireless_psk_fillin#>";
	*/
	
	limit_auth_method();
	wl_auth_mode_change(1);
	show_LAN_info();

	if(parent.smart_connect_support && (parent.isSwMode("rt") || parent.isSwMode("ap"))){

		document.form.smart_connect_t.value = (flag=='')?document.form.smart_connect_x.value:flag;

		change_smart_connect(document.form.smart_connect_t.value);

		$('#radio_smartcon_enable').iphoneSwitch(document.form.smart_connect_t.value>0,
			function() {
				change_smart_connect(document.form.smart_connect_t.value);
			},
			function() {
				change_smart_connect('0');
			}
		);
	}

	if(parent.sw_mode == 4){
		var wlc_auth_mode = '<% nvram_get("wlc_auth_mode"); %>';
		if(wlc_auth_mode == "") wlc_auth_mode = '<% nvram_get("wlc0_auth_mode"); %>';
		if(wlc_auth_mode == "") wlc_auth_mode = '<% nvram_get("wlc1_auth_mode"); %>';
		if(wlc_auth_mode == "") wlc_auth_mode = 'unknown';

		parent.show_middle_status(wlc_auth_mode, 0);
	}
	else
		parent.show_middle_status(document.form.wl_auth_mode_x.value, parseInt(document.form.wl_wep_x.value));

	if(history.pushState != undefined) history.pushState("", document.title, window.location.pathname);

	var table_height = document.getElementById("rt_table").clientHeight;
	if(table_height != "0" || table_height != "")
		set_NM_height(table_height);
	else {
		document.body.style.overflow = "hidden";
		var errorCount = 0;
		var readyStateCheckInterval = setInterval(function() {
			table_height = document.getElementById("rt_table").clientHeight;
			if (table_height != "0" || table_height != "") {
				clearInterval(readyStateCheckInterval);
				set_NM_height(table_height);
			}
			else {
				if(errorCount > 5) {
					clearInterval(readyStateCheckInterval);
					table_height = parent.document.getElementById("NM_table").style.height;
					set_NM_height(table_height);
				}
				errorCount++;
			}
		}, 10);
	}

	$("#tr_dwb_info").css("display", "none");
	if(isSupport("triband") && dwb_info.mode) {
		if(dwb_info.band == wl_unit) {
			$("#tr_wl_info").css("display", "none");
			$("#apply_tr").css("display", "none");
			$("#tr_dwb_info").css("display", "");
			var dwb_hint = "";
			dwb_hint += wl_nband_title[wl_unit];
			dwb_hint += " is now used as dedicated WiFi backhaul under AiMesh mode.";
			dwb_hint += "<br><br>";
			dwb_hint += "If you want to change wireless settings, please go to <span id='redirect_link'>here</span>.";/* untranslated */
			$("#tr_dwb_info").find(".dwb_hint").html(dwb_hint);
			$("#tr_dwb_info").find(".dwb_hint #redirect_link").css({"text-decoration":"underline", "cursor":"pointer"});
			$("#tr_dwb_info").find(".dwb_hint #redirect_link").click(function(){
				parent.document.titleForm.current_page.value = "Advanced_Wireless_Content.asp";
				parent.document.titleForm.next_page.value = "Advanced_Wireless_Content.asp";
				parent.change_wl_unit_status(2);
			});
		}
	}
}

function register_event(){
	$(function() {
		$( "#slider" ).slider({
			orientation: "horizontal",
			range: "min",
			min: 1,
			max: 4,
			value: 4,
			slide:function(event, ui){
				//$("#color_pad").css("background-color", color_table[ui.value-1]);
				$("#color_pad").html(led_table[ui.value-1]);
				$("#slider .ui-slider-range").css("background-color", color_table[ui.value-1]);
				$("#slider .ui-slider-handle").css("border-color", color_table[ui.value-1]);
			},
			stop:function(event, ui){
				set_led(ui.value);	  
			}
		}); 
	});
}

function set_led(value){
	var obj = {
		"action_mode": "apply",
		"rc_service": "reset_led",
	}

	obj.bc_ledLv = value-1;
	httpApi.nvramSet(obj);
}

function change_tabclick(){
	document.getElementById("t" + wl_unit + "").className = "tabclick_NW";
}

function tabclickhandler(wl_unit){
	if(wl_unit == 'status'){
		location.href = "router_status.asp";
	}
	/*else if (wl_unit == "compatibility") {
		location.href = "compatibility.asp";
	}*/
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

		if(parent.smart_connect_support && (parent.isSwMode("rt") || parent.isSwMode("ap"))){
			document.form.current_page.value = "device-map/router.asp?flag=" + document.form.smart_connect_x.value;
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
	if(mode == "wpa" || mode == "wpa2" || mode == "wpawpa2" || mode == "psk" || mode == "psk2" || mode == "sae" || mode == "pskpsk2" || mode == "psk2sae")
		inputCtrl(document.form.wl_crypto, 1);
	else
		inputCtrl(document.form.wl_crypto, 0);
	
	/* enable/disable psk passphrase */
	if(mode == "psk" || mode == "psk2" || mode == "sae" || mode == "pskpsk2" || mode == "psk2sae")
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
	if(mode == "psk" || mode == "psk2" || mode == "sae" || mode == "pskpsk2" || mode == "psk2sae" || mode == "wpa" || mode == "wpa2" || mode == "wpawpa2"){
		/* Save current crypto algorithm */
			if(opts[opts.selectedIndex].text == "WPA-Personal" || opts[opts.selectedIndex].text == "WPA-Enterprise")
				new_array = new Array("TKIP");
			else if(opts[opts.selectedIndex].text == "WPA2-Personal" || opts[opts.selectedIndex].text == "WPA3-Personal" || opts[opts.selectedIndex].text == "WPA2/WPA3-Personal" || opts[opts.selectedIndex].text == "WPA2-Enterprise")
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
	else if(parent.based_modelid == "RP-AC66"){
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
	var lan_ipaddr_t = '<% nvram_get("lan_ipaddr_t"); %>';
	if(lan_ipaddr_t != '')
		showtext(document.getElementById("LANIP"), lan_ipaddr_t);
	else	
		showtext(document.getElementById("LANIP"), '<% nvram_get("lan_ipaddr"); %>');

	if(parent.yadns_support && parent.sw_mode == 1){
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
	showtext(document.getElementById("MAC"), '<% get_lan_hwaddr(); %>');
	showtext(document.getElementById("MAC_wl2"), '<% nvram_get("wl0_hwaddr"); %>');
	if(document.form.wl_unit.value == '1')
		showtext(document.getElementById("MAC_wl5"), '<% nvram_get("wl1_hwaddr"); %>');
	else if(document.form.wl_unit.value == '2')
		showtext(document.getElementById("MAC_wl5_2"), '<% nvram_get("wl2_hwaddr"); %>');
	else if(document.form.wl_unit.value == '3')
		showtext(document.getElementById("MAC_wl60"), '<% nvram_get("wl3_hwaddr"); %>');

	if(document.form.wl_unit.value == '0'){
		document.getElementById("macaddr_wl5").style.display = "none";
		if(parent.wl_info.band5g_2_support)
			document.getElementById("macaddr_wl5_2").style.display = "none";	
		document.getElementById("macaddr_wl60").style.display = "none";
		if(!parent.band5g_support)
			document.getElementById("macaddr_wl2_title").style.display = "none";
	}
	else if (document.form.wl_unit.value == '1'){
		document.getElementById("macaddr_wl2").style.display = "none";
		document.getElementById("macaddr_wl5_2").style.display = "none";
		document.getElementById("macaddr_wl60").style.display = "none";
		if(parent.wl_info.band5g_2_support)
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
	if(parent.smart_connect_support && (parent.isSwMode("rt") || parent.isSwMode("ap"))){
		var macaddr_wl5_title = (parent.wl_info.band5g_2_support) ? "5GHz-1 " : "5GHz ";
		if(v == '1'){
			showtext(document.getElementById("MAC_wl2"), '<% nvram_get("wl0_hwaddr"); %>');
			showtext(document.getElementById("MAC_wl5"), '<% nvram_get("wl1_hwaddr"); %>');
			document.getElementById("macaddr_wl5_title").innerHTML = macaddr_wl5_title;
			document.getElementById("macaddr_wl2").style.display = "";
			document.getElementById("macaddr_wl5").style.display = "";
			if(parent.wl_info.band5g_2_support) {
				showtext(document.getElementById("MAC_wl5_2"), '<% nvram_get("wl2_hwaddr"); %>');
				document.getElementById("macaddr_wl5_2").style.display = "";
			}
			parent.document.getElementById("statusframe").height = 760;
		}else if(document.form.wl_unit.value != '0' && v == '2'){
			document.getElementById("macaddr_wl2").style.display = "none";
			showtext(document.getElementById("MAC_wl5"), '<% nvram_get("wl1_hwaddr"); %>');
			document.getElementById("macaddr_wl5_title").innerHTML = macaddr_wl5_title;
			document.getElementById("macaddr_wl5").style.display = "";
			if(parent.wl_info.band5g_2_support) {
				showtext(document.getElementById("MAC_wl5_2"), '<% nvram_get("wl2_hwaddr"); %>');
				document.getElementById("macaddr_wl5_2").style.display = "";
			}
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
	if(parent.lantiq_support && wave_ready != 1){
		alert("Please wait a minute for wireless ready");
		return false;
	}

	var auth_mode = document.form.wl_auth_mode_x.value;

	if(document.form.wl_wpa_psk.value == "<#wireless_psk_fillin#>")
		document.form.wl_wpa_psk.value = "";
		
	if(!parent.validator.stringSSID(document.form.wl_ssid))
		return false;
	
	stopFlag = 1;
	document.form.current_page.value = '/';
	document.form.next_page.value = '/';
	
	if(auth_mode == "psk" || auth_mode == "psk2" || auth_mode == "sae" || auth_mode == "pskpsk2" || auth_mode == "psk2sae"){
		if(is_KR_sku){
			if(!parent.validator.psk_KR(document.form.wl_wpa_psk))
				return false;
		}
		else{
			if(!parent.validator.psk(document.form.wl_wpa_psk))
				return false;
		}
		
		//confirm common string combination	#JS_common_passwd#
		var is_common_string = parent.check_common_string(document.form.wl_wpa_psk.value, "wpa_key");
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
		document.form.next_page.value = "/Advanced_WSecurity_Content.asp";
	}
	else{
		if(!parent.validator.wlKey(document.form.wl_asuskey1))
			return false;
	}
	
	var wep11 = eval('document.form.wl_key'+document.form.wl_key.value);
	wep11.value = document.form.wl_asuskey1.value;
	
	if((auth_mode == "shared" || auth_mode == "wpa" || auth_mode == "wpa2" || auth_mode == "wpawpa2" || auth_mode == "radius")
			&& document.form.wps_enable.value == "1"){
		document.form.wps_enable.value = "0";
	}
	document.form.wsc_config_state.value = "1";

	if(auth_mode == 'sae'){
		document.form.wl_mfp.value = '2';
	}
	else if(auth_mode == 'psk' || auth_mode == 'psk2' || auth_mode == 'pskpsk2' || auth_mode == 'psk2sae' || auth_mode == 'wpa' || auth_mode == 'wpa2' || auth_mode == 'wpawpa2'){
		document.form.wl_mfp.value = '1';
	}

	if(parent.amesh_support && (parent.isSwMode("rt") || parent.isSwMode("ap")) && parent.ameshRouter_support) {
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
		if(wl_info.band5g_2_support)
			ssid_array.push(httpApi.nvramGet(["wl2_ssid"]).wl2_ssid);
		jsonPara["current_ssid"] = ssid_array;
		if(!validator.dwb_check_wl_setting(jsonPara)) {
			alert("The fronthaul SSID is the same as the backhaul SSID.");/* untranslated */
			return false;
		}
	}

	if (Qcawifi_support) {
		document.form.action_wait.value = "30";
	}
	else if (Rawifi_support) {
		document.form.action_wait.value = "20";
	}

	parent.showLoading();
	if(parent.based_modelid == "RT-AC87U" && "<% nvram_get("wl_unit"); %>" == "1"){
		parent.stopFlag = '0';
		detect_qtn_ready();
	}else {
		document.form.fakepasswordremembered.disabled = true;
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
	var tab_width = Math.floor(270/(parent.wl_info.wl_if_total+1));

	/*if (Bcmwifi_support && band5g_11ax_support) {
		tab_width = "60";
	}*/

	var i = 0;

	while(i < tab_array1.length){
		tab_array1[i].style.width = tab_width + 'px';
		tab_array1[i].style.display = "";
		i++;
	}

	if(typeof tab_array2[0] != "undefined"){
		tab_array2[0].style.width = tab_width + 'px';
		tab_array2[0].style.display = "";
	}
	if(v == 0){
		document.getElementById("span0").innerHTML = "2.4GHz";
		if(parent.wl_info.band5g_2_support){
			document.getElementById("span1").innerHTML = "5GHz-1";
			document.getElementById("span2").innerHTML = "5GHz-2";
		}else{
			document.getElementById("span1").innerHTML = "5GHz";
			document.getElementById("t2").style.display = "none";
		}

		if(!parent.wl_info.band60g_support){
			document.getElementById("t3").style.display = "none";
		}
	}else if(v == 1){	//Smart Connect
		if(isSupport("triband") && dwb_info.mode) {
			document.getElementById("span0").innerHTML = "2.4GHz and 5GHz-1";
			document.getElementById("span0").style.padding = "5px 0px 0px 8px";
			document.getElementById("span2").innerHTML = "5GHz-2";
			document.getElementById("t0").style.width = "142px";
			document.getElementById("t1").style.display = "none";
			document.getElementById("t3").style.display = "none";
		}
		else {
			if(parent.wl_info.band2g_support && parent.wl_info.band5g_support && parent.wl_info.band5g_2_support)
				document.getElementById("span0").innerHTML = "2.4GHz, 5GHz-1 and 5GHz-2";
			else if(parent.wl_info.band2g_support && parent.wl_info.band5g_support)
				document.getElementById("span0").innerHTML = "2.4GHz and 5GHz";

			document.getElementById("t1").style.display = "none";
			document.getElementById("t2").style.display = "none";
			document.getElementById("t3").style.display = "none";
			document.getElementById("t0").style.width = (tab_width*parent.wl_info.wl_if_total+10) +'px';
		}
	}
	else if(v == 2){ //5GHz Smart Connect
		document.getElementById("span0").innerHTML = "2.4GHz";
		document.getElementById("span1").innerHTML = "5GHz-1 and 5GHz-2";
		document.getElementById("t3").style.display = "none";
		document.getElementById("t2").style.display = "none";	
		document.getElementById("t1").style.width = "148px";
		document.getElementById("span1").style.padding = "5px 4px 5px 9px";
	}
}

function change_smart_connect(v){
	document.form.smart_connect_x.value = v;

	show_LAN_info(v);
	switch(v){
		case '0':
				tab_reset(0);	
				break;
		case '1': 
				if(isSupport("triband") && dwb_info.mode) {
					if(wl_unit == dwb_info.band)
						tab_reset(1);
					else {
						if(wl_unit != '0')
							tabclickhandler(0);
						else
							tab_reset(1);
					}
				}
				else {
					if(wl_unit != '0')
						tabclickhandler(0);
					else
						tab_reset(1);
				}
				break;
		case '2': 
				if(!(wl_unit == '0' || wl_unit == '1'))
					tabclickhandler(1);
				else
					tab_reset(2);
				break;
	}
}

function checkWLReady(){
	$.ajax({
	    url: '/ajax_wl_ready.asp',
	    dataType: 'script',	
	    error: function(xhr) {
			setTimeout("checkWLReady();", 2000);
	    },
	    success: function(response){
	    	if(wave_ready != 1){
	    		$("#lantiq_ready").show();
	    		setTimeout("checkWLReady();", 2000);
	    	}
	    	else{
	    		$("#lantiq_ready").hide();
	    		setTimeout("checkWLReady();", 2000);
	    	}
	    }
  	});
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
<input type="hidden" name="wl_mfp" value="<% nvram_get("wl_mfp"); %>">
<input type="hidden" name="wl_subunit" value="-1">
<input type="hidden" name="smart_connect_x" value="<% nvram_get("smart_connect_x"); %>">
<table width="100%" border="0" cellpadding="0" cellspacing="0" id="rt_table">
<tr>
	<td>		
		<table width="100px" border="0" align="left" style="margin-left:8px;" cellpadding="0" cellspacing="0">
			<td>
				<div id="t0" class="tab_NW" align="center" style="font-weight: bolder;display:none; margin-right:2px;" onclick="tabclickhandler(0)">
					<span id="span0" style="cursor:pointer;font-weight: bolder;">2.4GHz</span>
				</div>
			</td>
			<td>
				<div id="t1" class="tab_NW" align="center" style="font-weight: bolder;display:none; margin-right:2px;" onclick="tabclickhandler(1)">
					<span id="span1" style="cursor:pointer;font-weight: bolder;">5GHz</span>
				</div>
			</td>
			<td>
				<div id="t2" class="tab_NW" align="center" style="font-weight: bolder;display:none; margin-right:2px;" onclick="tabclickhandler(2)">
					<span id="span2" style="cursor:pointer;font-weight: bolder;">5GHz-2</span>
				</div>
			</td>
			<td>
				<div id="t3" class="tab_NW" align="center" style="font-weight: bolder;display:none; margin-right:2px;" onclick="tabclickhandler(3)">
					<span id="span3" style="cursor:pointer;font-weight: bolder;">60GHz</span>
				</div>
			</td>
			<!--td>
				<div id="t_compatibility" class="tab_NW" align="center" style="font-weight: bolder; margin-right:2px;"
				 onclick="tabclickhandler('compatibility')">
					<span style="cursor:pointer;font-weight: bolder;">Compatibility</span>
				</div>
			</td-->
			<td>
				<div id="t_status" class="tab_NW" align="center" style="font-weight: bolder; margin-right:2px;" onclick="tabclickhandler('status')">
					<span id="span_status" style="cursor:pointer;font-weight: bolder;"><#Status_Str#></span>
				</div>
			</td>
		</table>
	</td>
</tr>

<tr id="tr_wl_info">
	<td>
		<table width="95%" border="1" align="center" cellpadding="4" cellspacing="0" class="table1px" id="WLnetworkmap_re" style="display:none">
		  <tr>
		    <td height="50" style="padding:10px 15px 0px 15px;">
		    	<p class="formfonttitle_nwm" style="float:left;"><#APSurvey_action_search_again_hint2#></p>
					<br />
			  	<input type="button" class="button_gen" onclick="gotoSiteSurvey();" value="<#QIS_rescan#>" style="float:right;">
		    </td>
		  </tr>
		</table>

		<table width="95%" border="1" align="center" cellpadding="4" cellspacing="0" class="table1px" id="WLnetworkmap">
               <tr id="smart_connect_field" style="display:none">
                       <td style="padding:5px 10px 0px 10px; *padding:1px 10px 0px 10px;">
                               <p class="formfonttitle_nwm" >Smart Connect</p>
                               <select style="*margin-top:-7px;" name="smart_connect_t" class="input_option" onchange="change_smart_connect(this.value);"></select>                               
                           <div style="margin-top:5px; *margin-top:-10px;" class="line_horizontal"></div>
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
					</td>
				</tr></table></div>
			</td>
		</tr>
  		<tr id="smartcon_enable_line" style="display:none">
  			<td>
  				<div style="*margin-top:-10px;" class="line_horizontal"></div>
  			</td>
  		</tr>
  		<tr>
    			<td style="padding:5px 10px 0px 10px; ">
    			<div id="lantiq_ready" style="display:none;color:#FC0;margin-left:3px;font-size:13px;">Wireless is setting...</div>
  	  			<p class="formfonttitle_nwm" ><#QIS_finish_wireless_item1#></p>
      			<input style="*margin-top:-7px; width:260px;" id="wl_ssid" type="text" name="wl_ssid" value="<% nvram_get("wl_ssid"); %>" maxlength="32" size="22" class="input_25_table" autocomplete="off" autocorrect="off" autocapitalize="off">
      				<div style="margin-top:5px; *margin-top:-10px;" class="line_horizontal"></div>
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
							<option value="sae"    <% nvram_match("wl_auth_mode_x", "sae",   "selected"); %>>WPA3-Personal</option>
							<option value="pskpsk2" <% nvram_match("wl_auth_mode_x", "pskpsk2","selected"); %>>WPA-Auto-Personal</option>
							<option value="psk2sae" <% nvram_match("wl_auth_mode_x", "psk2sae","selected"); %>>WPA2/WPA3-Personal</option>
							<option value="wpa"     <% nvram_match("wl_auth_mode_x", "wpa",    "selected"); %>>WPA-Enterprise</option>
							<option value="wpa2"    <% nvram_match("wl_auth_mode_x", "wpa2",   "selected"); %>>WPA2-Enterprise</option>
							<option value="wpawpa2" <% nvram_match("wl_auth_mode_x", "wpawpa2","selected"); %>>WPA-Auto-Enterprise</option>
							<option value="radius"  <% nvram_match("wl_auth_mode_x", "radius", "selected"); %>>Radius with 802.1x</option>
				  		</select>
							<img style="display:none;margin-top:-30px;margin-left:185px;cursor:pointer;" id="wl_nmode_x_hint" src="/images/alertImg.png" width="30px" onClick="parent.overlib(parent.helpcontent[0][24], parent.FIXX, 870, parent.FIXY, 350);" onMouseOut="parent.nd();">
	  					<div style="margin-top:5px; *margin-top:-10px;" class="line_horizontal"></div>
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
	  			<div style="margin-top:5px; *margin-top:-10px;" class="line_horizontal"></div>
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
	  				<div style="margin-top:5px; *margin-top:-10px;" class="line_horizontal"></div>
    			</td>
  		</tr>
  		<tr id='asus_wep_key'>
    			<td style="padding:5px 10px 0px 10px; ">
	    			<p class="formfonttitle_nwm" ><#WLANConfig11b_WEPKey_itemname#>
						</p>
							<input id="wl_asuskey1" name="wl_asuskey1" style="width:260px;*margin-top:-7px;" type="password" onBlur="switchType(this, false);" onFocus="switchType(this, true);" onKeyUp="return change_wlkey(this, 'WLANConfig11b');" value="" maxlength="27" class="input_25_table" autocorrect="off" autocapitalize="off">
      				<div style="margin-top:5px; *margin-top:-10px;" class="line_horizontal"></div>
    			</td>
  		</tr>
  		<tr id='wl_crypto' style='display:none;'>
			<td style="padding:5px 10px 0px 10px; *padding:1px 10px 0px 10px;">
	  			<p class="formfonttitle_nwm" ><#WLANConfig11b_WPAType_itemname#></p>
	  			<select style="*margin-top:-7px;" name="wl_crypto" class="input_option" onchange="wl_auth_mode_change(0);">
					<option value="aes" <% nvram_match("wl_crypto", "aes", "selected"); %>>AES</option>
					<option value="tkip+aes" <% nvram_match("wl_crypto", "tkip+aes", "selected"); %>>TKIP+AES</option>
	  			</select>	  			
	  			<div style="margin-top:5px; *margin-top:-10px;" class="line_horizontal"></div>	
			</td>
  		</tr>
  		<tr id='wl_wpa_psk_tr' style='display:none'>
    			<td style="padding:5px 10px 0px 10px;">
					<p id="wl_wpa_psk_title" class="formfonttitle_nwm" ><#WPA-PSKKey#></p>
							<input id="wl_wpa_psk" name="wl_wpa_psk" style="width:260px;*margin-top:-7px;" type="password" onBlur="switchType(this, false);" onFocus="switchType(this, true);" value="" maxlength="64" class="input_25_table" autocomplete="off" autocorrect="off" autocapitalize="off"/>
      						<!-- fake fields are a workaround for chrome autofill getting the wrong fields -->
      						<input style="display:none" type="password" name="fakepasswordremembered"/>
      				<div style="margin-top:5px; *margin-top:-10px;" class="line_horizontal"></div>
    			</td>
		</tr>
 		</table>		
  	</td>
</tr>

<tr>
	<td> 			
 		<table width="95%" border="1" align="center" cellpadding="4" cellspacing="0" class="table1px">
  		<tr id="apply_tr">
				<td id="bottom_border" style="text-align:center">
					<input id="applySecurity" type="button" class="button_gen" value="<#CTL_apply#>" onclick="submitForm();" >
    			</td>
  		</tr>
		<tr id="tr_dwb_info" style="display:none;">
			<td style="padding:5px 10px 5px 10px;border-bottom:5px #000 solid;">
				<div class="dwb_hint" ></div>
			</td>
		</tr>
		<tr id="led_tr" style="display:none">
			<td>
				<div>
					<table>
						<tr>
							<td>
								<div style="font-family: Arial, Helvetica, sans-serif;font-weight: bolder;"><#LED_Brightness#></div>
							</td>
						</tr>
						<tr>
							<td style="border:0px;padding-left:0px;">
								<div id="slider" style="width:180px;"></div>
							</td>
							<td style="border:0px;">
								<div id="color_pad" style="margin-left: 20px;font-weight: bolder;">100%</div>
							</td>				
						</tr>
					</table>
				</div>
				<div style="margin-top:5px; *margin-top:-10px;" class="line_horizontal"></div>
			</td>
		</tr>
  		<tr>
    			<td style="padding:10px 10px 0px 10px;">
    				<p class="formfonttitle_nwm" ><#LAN_IP#></p>
    				<p class="tab_info_bg" style="padding-left:10px; margin-top:3px; *margin-top:-5px; margin-right:10px; line-height:20px;" id="LANIP"></p>
      				<div style="margin-top:5px; *margin-top:-10px;" class="line_horizontal"></div>
    			</td>
  		</tr>  
		<tr id="pincode">
    			<td style="padding:5px 10px 0px 10px;">
    				<p class="formfonttitle_nwm" ><#PIN_code#></p>
    				<p class="tab_info_bg" style="padding-left:10px; margin-top:3px; *margin-top:-5px; margin-right:10px;line-height:20px;" id="PINCode"></p>
      				<div style="margin-top:5px; *margin-top:-10px;" class="line_horizontal"></div>
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
      				<div style="margin-top:5px; *margin-top:-10px;" class="line_horizontal"></div>
    			</td>
  		</tr>
  		<tr>
    			<td style="padding:5px 10px 0px 10px;">
    				<p class="formfonttitle_nwm" >LAN <#MAC_Address#></p>
    				<p class="tab_info_bg" style="padding-left:10px; margin-top:3px; *margin-top:-5px; padding-bottom:3px; margin-right:10px;line-height:20px;" id="MAC"></p>
    				<div style="margin-top:5px; *margin-top:-10px;" class="line_horizontal"></div>
    			</td>
  		</tr>     
  		<tr id="macaddr_wl2">
    			<td style="padding:5px 10px 0px 10px;">
    				<p class="formfonttitle_nwm" >Wireless <span id="macaddr_wl2_title">2.4GHz </span><#MAC_Address#></p>
    				<p class="tab_info_bg" style="padding-left:10px; margin-bottom:5px; margin-top:3px; *margin-top:-5px; padding-bottom:3px; margin-right:10px;line-height:20px;" id="MAC_wl2"></p>
    				<div style="margin-top:5px; *margin-top:-10px;" class="line_horizontal"></div>
    			</td>
  		</tr>     
  		<tr id="macaddr_wl5">
    			<td style="padding:5px 10px 0px 10px;">
    				<p class="formfonttitle_nwm" >Wireless <span id="macaddr_wl5_title">5GHz </span><#MAC_Address#></p>
    				<p class="tab_info_bg" style="padding-left:10px; margin-bottom:5px; margin-top:3px; *margin-top:-5px; padding-bottom:3px; margin-right:10px;line-height:20px;" id="MAC_wl5"></p>
    				<div style="margin-top:5px; *margin-top:-10px;" class="line_horizontal"></div>
    			</td>
  		</tr>
  		<tr id="macaddr_wl5_2" style="display:none;">
    			<td style="padding:5px 10px 0px 10px;">
    				<p class="formfonttitle_nwm" >Wireless <span id="macaddr_wl5_2_title">5GHz-2 </span><#MAC_Address#></p>
    				<p class="tab_info_bg" style="padding-left:10px; margin-bottom:5px; margin-top:3px; *margin-top:-5px; padding-bottom:3px; margin-right:10px;line-height:20px;" id="MAC_wl5_2"></p>
    				<div style="margin-top:5px; *margin-top:-10px;" class="line_horizontal"></div>
    			</td>
  		</tr>  
		<tr id="macaddr_wl60" style="display:none;">
			<td style="padding:5px 10px 0px 10px;">
				<p class="formfonttitle_nwm" >Wireless <span id="macaddr_wl60_title">60GHz </span><#MAC_Address#></p>
				<p class="tab_info_bg" style="padding-left:10px; margin-bottom:5px; margin-top:3px; *margin-top:-5px; padding-bottom:3px; margin-right:10px;line-height:20px;" id="MAC_wl60"></p>
				<div style="margin-top:5px; *margin-top:-10px;" class="line_horizontal"></div>
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
