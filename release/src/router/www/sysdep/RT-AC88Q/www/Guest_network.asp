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
<title><#Web_Title#> - <#Guest_Network#></title>
<link rel="stylesheet" type="text/css" href="index_style.css"> 
<link rel="stylesheet" type="text/css" href="form_style.css">
<link rel="stylesheet" type="text/css" href="usp_style.css">
<link href="other.css"  rel="stylesheet" type="text/css">
<link rel="stylesheet" type="text/css" href="/device-map/device-map.css">
<link rel="stylesheet" type="text/css" href="Guest_network.css">
<script type="text/javascript" src="/state.js"></script>
<script language="JavaScript" type="text/javascript" src="/js/jquery.js"></script>
<script language="JavaScript" type="text/javascript" src="/client_function.js"></script>
<script type="text/javascript" src="/help.js"></script>
<script type="text/javascript" src="/general.js"></script>
<script type="text/javascript" src="/popup.js"></script>
<script type="text/javascript" src="/md5.js"></script>
<script type="text/javascript" src="/validator.js"></script>
<script type="text/javascript" src="switcherplugin/jquery.iphone-switch.js"></script>
<script type="text/javascript" src="form.js"></script>
<style>
</style>
<script>
if(!Qcawifi_support)
{
	var radio_2 = '<% nvram_get("wl0_radio"); %>';
	var radio_5 = '<% nvram_get("wl1_radio"); %>';
}
<% radio_status(); %>

var wl1_nmode_x = '<% nvram_get("wl1_nmode_x"); %>';
var wl0_nmode_x = '<% nvram_get("wl0_nmode_x"); %>';
if(wl_info.band5g_2_support){
	var wl2_nmode_x = '<% nvram_get("wl2_nmode_x"); %>';
}

<% wl_get_parameter(); %>

wl_channel_list_2g = '<% channel_list_2g(); %>';
wl_channel_list_5g = '<% channel_list_5g(); %>';

var QoS_enable_orig = '<% nvram_get("qos_enable"); %>';
var QoS_type_orig = '<% nvram_get("qos_type"); %>';
var ctf_disable_orig = '<% nvram_get("ctf_disable"); %>';

var gn_array = gn_array_2g;
var wl_maclist_x_array = gn_array[0][16];
var g_maxsta = 50;

var captive_portal_used_wl_array = new Array();

window.onresize = function() {
	if(document.getElementById("gnset_block").style.display == "block") {
		cal_panel_block("gnset_block", 0.15);
	}
}

var lan_num = 8;
var wl0_vifnames = '<% nvram_get("wl0_vifnames"); %>';
var wireless_num = wl0_vifnames.split(' ').length + 1;
var vlan_enable = parseInt('<% nvram_get("vlan_enable"); %>');
var vlan_rulelist = decodeURIComponent("<% nvram_char_to_ascii("","vlan_rulelist"); %>");
var vlan_rulelist_row = vlan_rulelist.split('<');
var vlanrule_inf_array = new Array(); //[[vlan1, lan, 2g, 5g], [vlan2, lan, 2g, 5g], ...]

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
	show_menu();

	if(Qcawifi_support)
	{
		radio_2 = '<% nvram_get("wl0_radio"); %>';
		radio_5 = '<% nvram_get("wl1_radio"); %>';
	}

	//insertExtChannelOption();
	if(tagged_based_vlan && vlan_enable)
		parse_vlan_rulelist();

	if(downsize_4m_support || downsize_8m_support)
		document.getElementById("guest_image").parentNode.parentNode.removeChild(document.getElementById("guest_image").parentNode);

	gen_gntable();

	if(radio_2 != 1){
		document.getElementById('2g_radio_hint').style.display = "";
	}
	if(radio_5 != 1){
		document.getElementById('5g_radio_hint').style.display = "";
	}

	if(document.form.preferred_lang.value == "JP"){    //use unique font-family for JP
		document.getElementById('2g_radio_hint').style.fontFamily = "MS UI Gothic,MS P Gothic";
		document.getElementById('5g_radio_hint').style.fontFamily = "MS UI Gothic,MS P Gothic";
	}	
	
	if("<% get_parameter("af"); %>" == "wl_NOnly_note"){
		var childsel=document.createElement("div");
		childsel.setAttribute("id","wl_NOnly_note");
		childsel.style.color="#FFCC00";
		document.getElementById('gn_desc').parentNode.appendChild(childsel);
		document.getElementById("wl_NOnly_note").innerHTML="* Please change the guest network authentication to WPA2 Personal AES.";	
	}

	setTimeout("showDropdownClientList('setClientmac', 'mac', 'wl', 'WL_MAC_List_Block', 'pull_arrow', 'all');", 1000);	
	updateLanaccess();

	//When redirect page from Free WiFi or Captive Portal, auto go to anchor tag
	var gn_idx = cookie.get("captive_portal_gn_idx");
	if(gn_idx != "" && gn_idx != null) {
		var type_hint = "";
		switch(parseInt(gn_idx)) {
			case 5 :
				type_hint = "Captive Portal";
				break;
			case 6 :
				type_hint = "Free W-Fi";
				break;
		}
		window.location.hash = "guest_block_anchor_" + gn_idx;
		setTimeout(function(){
			alert("Guest Network – " + gn_idx + " will used by " + type_hint);
		}, 100);
		cookie.unset("captive_portal_gn_idx");
	}
}

function change_wl_expire_radio(){
	load_expire_selection(document.form.wl_expire_day, option_expire_day, optval_expire_day);	
	
	if(document.form.wl_expire.value > 0){
		document.form.wl_expire_day.value = Math.floor(document.form.wl_expire.value/86400);
		document.form.wl_expire_hr.value = Math.floor((document.form.wl_expire.value%86400)/3600);
		document.form.wl_expire_min.value  = Math.floor((document.form.wl_expire.value%3600)/60);
		document.form.wl_expire_radio[0].checked = 0;
		document.form.wl_expire_radio[1].checked = 1;
	}
	else{	
		document.form.wl_expire_hr.value = "";
		document.form.wl_expire_min.value = "";
		document.form.wl_expire_radio[0].checked = 1;
		document.form.wl_expire_radio[1].checked = 0;
	}
}

function change_wl_lanaccess(){
	if(document.form.wl_lanaccess.value == "on") {
		document.form.wl_lanaccess[0].checked = 1;
		document.form.wl_lanaccess[1].checked = 0;
	}
	else{	
		document.form.wl_lanaccess[0].checked = 0;
		document.form.wl_lanaccess[1].checked = 1;
	}
}

option_expire_day = new Array("0", "1", "2", "3", "4", "5", "6", "7", "8", "9", "10", 
			"11", "12", "13", "14", "15", "16", "17", "18", "19", "20", 
			"21", "22", "23", "24", "25", "26", "27", "28", "29", "30");
optval_expire_day = new Array(0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 
			11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 
			21, 22, 23, 24, 25, 26, 27, 28, 29, 30);


function load_expire_selection(obj, opt, val){
	free_options(obj);
	for(i=0; i<opt.length; i++){
		if(opt[i].length > 0){
			obj.options[i] = new Option(opt[i], val[i]);
		}
	}
}	

function translate_auth(flag){
	if(flag == "open")
		return "Open System";
	else if(flag == "shared")
		return "Shared Key";
	else if(flag == "psk")
		return "WPA-Personal";
	else if(flag == "psk2")
 		return "WPA2-Personal";
	else if(flag == "pskpsk2")
		return "WPA-Auto-Personal";
	else if(flag == "wpa")
		return "WPA-Enterprise";
	else if(flag == "wpa2")
		return "WPA2-Enterprise";
	else if(flag == "wpawpa2")
		return "WPA-Auto-Enterprise";
	else if(flag == "radius")
		return "Radius with 802.1x";
	else
		return "unknown Auth";
}

function _change_wl_unit_status(__unit){
	document.titleForm.current_page.value = "Advanced_WAdvanced_Content.asp?af=wl_radio";
	document.titleForm.next_page.value = "Advanced_WAdvanced_Content.asp?af=wl_radio";
	change_wl_unit_status(__unit);
}

function create_guest_unit(_unit, _subunit){
	edit_guest_unit(_unit, _subunit);
	document.form.wl_bss_enabled.value = "1";
}
function copyOtherSetting(_unit, _subunit) {
	var idx;
	idx = _subunit - 1;
	switch(parseInt(_unit)){
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
	document.form.wl_ssid.value = decodeURIComponent(gn_array[idx][1]);
	document.form.wl_auth_mode_x.value = decodeURIComponent(gn_array[idx][2]);
	wl_auth_mode_change(1); //setting wl_crypto, wl_wpa_psk item display mode
	document.form.wl_crypto.value = decodeURIComponent(gn_array[idx][3]);
	document.form.wl_wpa_psk.value = decodeURIComponent(gn_array[idx][4]);
	document.form.wl_wep_x.value = decodeURIComponent(gn_array[idx][5]);
	document.form.wl_key.value = decodeURIComponent(gn_array[idx][6]);
	document.form.wl_key1.value = decodeURIComponent(gn_array[idx][7]);
	document.form.wl_key2.value = decodeURIComponent(gn_array[idx][8]);
	document.form.wl_key3.value = decodeURIComponent(gn_array[idx][9]);
	document.form.wl_key4.value = decodeURIComponent(gn_array[idx][10]);
	document.form.wl_phrase_x.value = decodeURIComponent(gn_array[idx][17]);
	document.form.wl_expire.value = decodeURIComponent(gn_array[idx][11]);
	document.form.wl_lanaccess.value = decodeURIComponent(gn_array[idx][12]);
	if(decodeURIComponent(gn_array[idx][18]) == 1)
		document.form.bw_enabled_x[0].checked = true;
	else
		document.form.bw_enabled_x[1].checked = true;
	unit_bw_enabled = decodeURIComponent(gn_array[idx][18]);
	document.form.wl_bw_dl_x.value = decodeURIComponent(gn_array[idx][19])/1024;
	unit_bw_dl = decodeURIComponent(gn_array[idx][19]);
	document.form.wl_bw_ul_x.value = decodeURIComponent(gn_array[idx][20])/1024;
	unit_bw_ul = decodeURIComponent(gn_array[idx][20]);
	document.form.wl_guest_num.value = (decodeURIComponent(gn_array[idx][21]) == "") ? 10 : decodeURIComponent(gn_array[idx][21]);

	wl_wep_change();

	change_wl_expire_radio();
	show_bandwidth(unit_bw_enabled);
	change_wl_lanaccess();

	updateMacModeOption(gn_array[idx]);

}

var unit_bw_enabled = "";
var unit_bw_ul = "";
var unit_bw_dl = "";
var edit_unit = "";
function edit_guest_unit(_unit, _subunit) {
	var wl_vifnames = '<% nvram_get("wl0_vifnames"); %>';
	var nr_mssid = wl_vifnames.split(" ").length + 1;
	var columns_count = 0;
	var auto_add_br = function() {
		columns_count++;
		if(columns_count == 4) {
			edit_gn_html += "<br>";
			columns_count = 0;
		}
	};
	var unit_subunit = _unit + "." + _subunit;
	document.getElementById("gn_interfce").innerHTML = "<#Guest_Network#> " + _subunit + " - " +   wl_nband_title[_unit];
	document.getElementById("gn_interfce").innerHTML += "<br>SET UP";

	var guest_group_num = gn_array_2g.length;
	var edit_gn_html = "";
	var gn_unit_subunit = "";
	for(var i = 1; i <= guest_group_num; i += 1) {
		if(tmo_support && i == 3)	{ //keep wlx.3 for usingg Passpoint
			break;
		}
		//2.4G
		if(gn_array_2g[i - 1][0] == "1") {
			gn_unit_subunit = "0." + i;
			if(gn_unit_subunit != unit_subunit) {
				//auto_add_br();
				edit_gn_html += "<div class='gnset_copySettingContent_list'><input type='radio' name='radio_gn_list' id='radio_gn_0_" + i + "' onclick='copyOtherSetting(0, " + i + ");'><label class='gnset_copySettingContent_item' for='radio_gn_0_" + i + "'><#Guest_Network#> " + i + " - " + wl_nband_title[0] + "</label></div>";
			}
		}
		//5G
		if(wl_info.band5g_support && !no5gmssid_support) {
			
			if(gn_array_5g[i - 1][0] == "1" ) {
				gn_unit_subunit = "1." + i;
				if(gn_unit_subunit != unit_subunit) {
					//auto_add_br();
					edit_gn_html += "<div class='gnset_copySettingContent_list'><input type='radio' name='radio_gn_list' id='radio_gn_1_" + i + "' onclick='copyOtherSetting(1, " + i + ");'><label class='gnset_copySettingContent_item' for='radio_gn_1_" + i + "'><#Guest_Network#> " + i + " - " + wl_nband_title[1] + "</label></div>";
				}
			}
		}
		//5G-2
		if(wl_info.band5g_2_support) {
			if(gn_array_5g_2[i - 1][0] == "1" ) {
				gn_unit_subunit = "2." + i;
				if(gn_unit_subunit != unit_subunit) {
					//auto_add_br();
					edit_gn_html += "<div class='gnset_copySettingContent_list'><input type='radio' name='radio_gn_list' id='radio_gn_2_" + i + "' onclick='copyOtherSetting(2, " + i + ");'><label class='gnset_copySettingContent_item' for='radio_gn_2_" + i + "'><#Guest_Network#> " + i + " - " + wl_nband_title[2] + "</label></div>";
				}
			}
		}
	}
	if(edit_gn_html == "") { //if not edited guestnetwork, hide list 
		document.getElementById("edited_list").style.display = "none";
		document.getElementById("gn_copyOtherSetting").style.display = "none";
	}
	else {
		document.getElementById("edited_list").style.display = "none";
		document.getElementById("gn_copyOtherSetting").style.display = "";
		//auto_add_br();
		edit_gn_html += "<div class='gnset_copySettingContent_list'><input type='radio' name='radio_gn_list' id='radio_gn_default' onclick='copyOtherSetting(" + _unit + ", " + _subunit + ");'><label class='gnset_copySettingContent_item' for='radio_gn_default'><#Setting_upload_itemname#> </label></div>";
		document.getElementById("edited_guestnetwork_list").innerHTML = edit_gn_html;
		document.getElementById("copyOtherSettingIcon").className = "gnset_copySettingContent_icon";
		document.getElementById("copyOtherSettingIcon").onclick = function() {ShowHideOtherSetting();};
	}
	
	var idx;
	switch(parseInt(_unit)){
		case 0:
			edit_unit=0;
			gn_array = gn_array_2g;
			document.form.wl_nmode_x.value = wl0_nmode_x;
			break;
		case 1:			
			edit_unit=1;
			gn_array = gn_array_5g;
			document.form.wl_nmode_x.value = wl1_nmode_x;
			break;
		case 2:
			edit_unit=2;
			gn_array = gn_array_5g_2;
			document.form.wl_nmode_x.value = wl2_nmode_x;
			break;
	}
	idx = _subunit - 1;
	limit_auth_method(_unit); //setting wl_auth_mode_x options
	document.form.wl_unit.value = _unit;
	document.form.wl_subunit.value = _subunit;
	document.form.wl_bss_enabled.value = decodeURIComponent(gn_array[idx][0]);
	document.form.wl_ssid.value = decodeURIComponent(gn_array[idx][1]);
	wl_x_y_bss_enabled = 1;

	document.form.wl_auth_mode_x.value = decodeURIComponent(gn_array[idx][2]);
	wl_auth_mode_change(1); //setting wl_crypto, wl_wpa_psk item display mode
	document.form.wl_crypto.value = decodeURIComponent(gn_array[idx][3]);
	document.form.wl_wpa_psk.value = decodeURIComponent(gn_array[idx][4]);
	document.form.wl_wep_x.value = decodeURIComponent(gn_array[idx][5]);
	document.form.wl_key.value = decodeURIComponent(gn_array[idx][6]);
	document.form.wl_key1.value = decodeURIComponent(gn_array[idx][7]);
	document.form.wl_key2.value = decodeURIComponent(gn_array[idx][8]);
	document.form.wl_key3.value = decodeURIComponent(gn_array[idx][9]);
	document.form.wl_key4.value = decodeURIComponent(gn_array[idx][10]);
	document.form.wl_phrase_x.value = decodeURIComponent(gn_array[idx][17]);
	document.form.wl_expire.value = decodeURIComponent(gn_array[idx][11]);
	document.form.wl_lanaccess.value = decodeURIComponent(gn_array[idx][12]);
	if(decodeURIComponent(gn_array[idx][18]) == 1)
		document.form.bw_enabled_x[0].checked = true;
	else
		document.form.bw_enabled_x[1].checked = true;
	unit_bw_enabled = decodeURIComponent(gn_array[idx][18]);
	document.form.wl_bw_dl_x.value = decodeURIComponent(gn_array[idx][19])/1024;
	unit_bw_dl = decodeURIComponent(gn_array[idx][19]);
	document.form.wl_bw_ul_x.value = decodeURIComponent(gn_array[idx][20])/1024;
	unit_bw_ul = decodeURIComponent(gn_array[idx][20]);

	if(tagged_based_vlan && vlan_enable){
		var vlan_id = get_wl_vlan_id(_unit, _subunit);
		if(vlan_id != ""){
			document.getElementById("gnset_wl_vlanTag").style.display = "";
			document.getElementById("vlan_id").innerHTML = vlan_id;
		}
		else
			document.getElementById("gnset_wl_vlanTag").style.display = "none";
	}
	else
		document.getElementById("gnset_wl_vlanTag").style.display = "none";

	wl_wep_change();

	change_wl_expire_radio();
	show_bandwidth(unit_bw_enabled);
	change_wl_lanaccess();

	updateMacModeOption(gn_array[idx]);


	if(sw_mode == "3") {
		document.getElementById("gnset_wl_lanaccess").style.display = "none";
	}

	if (Rawifi_support)
		g_maxsta = 32;
	else if (Qcawifi_support){
		if (based_modelid == "BRT-AC828") //MODELDEP: Max. number of clients of QCA99xx is 511.
			g_maxsta = 511 / nr_mssid;
		else
			g_maxsta = 127 / nr_mssid;
	}
	g_maxsta -= g_maxsta % 10;

	//setting total guest num
	for(var i = 0; i < g_maxsta; i += 1) {
		document.form.wl_guest_num[i] = new Option((i+1), (i+1));
	}
	document.form.wl_guest_num.value = (decodeURIComponent(gn_array[idx][21]) == "") ? 10 : decodeURIComponent(gn_array[idx][21]);

	document.getElementById("unused_item").style.display = "none";

	//control captive portal item
	if(captive_portal_used_wl_array["wl" + unit_subunit] != undefined) {
		document.getElementById("gnset_wl_captive_portal").style.display = "";
		document.getElementById("gnset_captive_portal_profile").innerHTML = captive_portal_used_wl_array["wl" + unit_subunit];
		document.getElementById("gnset_wl_guest_num").style.display = "none";
		document.getElementById("gnset_wl_expire").style.display = "none";
		document.getElementById("gnset_wl_lanaccess").style.display = "none";
		document.getElementById("gnset_wl_bw_enabled").style.display = "none";
		document.getElementById("gnset_wl_bw_setting").style.display = "none";
	}
	else {
		document.getElementById("gnset_wl_captive_portal").style.display = "none";
		document.getElementById("gnset_wl_guest_num").style.display = "";
		document.getElementById("gnset_wl_expire").style.display = "";
		document.getElementById("gnset_wl_lanaccess").style.display = "";
		document.getElementById("gnset_wl_bw_enabled").style.display = "";
		document.getElementById("gnset_wl_bw_setting").style.display = "";
	}

	$('#full_screen_bg').fadeIn();
	$('#gnset_block').fadeIn();
	var scrollTop = $(document).scrollTop();
	$("#gnset_block").css({ top: (scrollTop + 20) + "px" });
	cal_panel_block("gnset_block", 0.15);
}
function ShowHideOtherSetting() {

	if(document.getElementById("edited_list").style.display == "none") {
		$("#edited_list").slideDown("normal");
		document.getElementById("copyOtherSettingIcon").className = "gnset_copySettingContent_icon_hover";
	}
	else {
		$("#edited_list").slideUp("normal");
		document.getElementById("copyOtherSettingIcon").className = "gnset_copySettingContent_icon";
		var allRadios = document.getElementsByName('radio_gn_list');
		var x = 0;
		for(x = 0; x < allRadios.length; x++) {
			allRadios[x].checked = false;
		}
	}
}
function gnset_cancel() {
	$('#full_screen_bg').fadeOut();
	$('#gnset_block').fadeOut();
}

function showHideContent(objnmae, thisObj) {
	var state = document.getElementById(objnmae).style.display;
	if(state == "none") {
		$("#" + objnmae + "").slideDown("normal");
		$("#" + objnmae + "_expand").html("-");
		$("#" + objnmae + "_expand").css("line-height", "16px");
	}
	else {
		$("#" + objnmae + "").slideUp("normal");
		$("#" + objnmae + "_expand").html("+");
		$("#" + objnmae + "_expand").css("line-height", "20px");
	}
}

function gen_gntable(){
	//wl_info.band5g_2_support = false;
	var gen_new_guest = function(_unit, _subunit) {
		var html = "";
		if(wl_info.band5g_2_support)
			html += "<div class='newGuestNetwork_block_tri_band'>";
		else
			html += "<div class='newGuestNetwork_block'>";
		html += "<div class='newGuestNetwork_titleName_unedit'>" + wl_nband_title[_unit] + "</div>";
		html += "<div class='newGuestNetwork_box_decoration' onclick='create_guest_unit(" + _unit + ", " + _subunit + ")'>";
		if(wl_info.band5g_2_support)
			html += "<div class='newGuestNetwork_icon_block_tri_band'>";
		else 
			html += "<div class='newGuestNetwork_icon_block'>";
		html += "<div class='newGuestNetwork_icon'></div>";
		html += "</div>";
		html += "<div class='newGuestNetwork_txt'>";
		html += "<div class='newGuestNetwork_txt_big'>New</div>"; /*untranslated*/
		html += "<div class='newGuestNetwork_txt_small'>A Guest Network</div>"; /*untranslated*/
		html += "</div>";
		
		html += "</div>";
		html += "</div>";
		return html;
	};

	var gen_had_enable_guest = function(_gn_array, _unit, _index) {
		var html = "";
		var show_str = "";
		var _gn_unit_subunit = "wl" + _unit + "." + _index;
		//html += "<table style='width:100%'><tr><td>";
		if(wl_info.band5g_2_support)
			html += "<div class='editedGuestNetwork_block_tri_band'>";
		else
			html += "<div class='editedGuestNetwork_block'>";
		html += "<div style='height:32px;'>";
		html += "<div style='float:left;width:50%;'>";
		html += "<div style='float:left;line-height:32px;' class='guestNetwork_name_edited'>" + wl_nband_title[_unit] + "</div>";
		html += "</div>";
		
		html += "<div style='float:right;width:50%;'>";
		if(captive_portal_used_wl_array[_gn_unit_subunit] == undefined) {
			html += "<div class='left' style='float:right;' id='radio_guest_enable_" + _unit + "_" + _index + "'></div>";
		}
		html += "<div class='internetTimeEdit' style='float:right;cursor:pointer;' onclick='edit_guest_unit(" + _unit + ", " + _index + ");' ></div>";
		html += "</div>";
		
		//html += "</td></tr></table>";
		html += "</div>";
		html += "<div class='guest_line'></div>";
		html += "<div class='guestNetwork_titleName_edited'><#QIS_finish_wireless_item1#></div>";
		show_str = decodeURIComponent(_gn_array[1]);
		//show_str = handle_show_str(show_str);
		if(show_str.length > 25) {
			var show_str_temp = show_str;
			show_str_temp = show_str_temp.substr(0, 23) + "..";
			html += "<div class='guestNetwork_content_edited' title='" + htmlEnDeCode.htmlEncode(show_str) + "'>" + htmlEnDeCode.htmlEncode(show_str_temp) + "</div>";
		}
		else {
			html += "<div class='guestNetwork_content_edited'>" + htmlEnDeCode.htmlEncode(show_str) + "</div>";
		}
		html += "<div class='guest_line'></div>";
		html += "<div  class='guestNetwork_titleName_edited'><#WLANConfig11b_AuthenticationMethod_itemname#></div>";
		html += "<div class='guestNetwork_content_edited'>" + translate_auth(_gn_array[2]) + "</div>";
		html += "<div class='guest_line'></div>";
		html += "<div  class='guestNetwork_titleName_edited'><#Network_key#></div>";
		
		if(_gn_array[2].indexOf("wpa") >= 0 || _gn_array[2].indexOf("radius") >= 0)
			show_str = "";
		else if(_gn_array[2].indexOf("psk") >= 0)
			show_str = _gn_array[4];
		else if(_gn_array[2] == "open" && _gn_array[5] == "0")
			show_str = "None";
		else {
			var key_index = parseInt(_gn_array[6]) + 6;
			show_str = _gn_array[key_index];
		}
		show_str = decodeURIComponent(show_str);
		if(show_str.length <= 0)
			show_str = "&nbsp; ";
		if(show_str.length > 25) {
			var show_str_temp = show_str;
			show_str_temp = show_str_temp.substr(0, 23) + "..";
			html += "<div class='guestNetwork_content_edited' title='" + htmlEnDeCode.htmlEncode(show_str) + "'>" + htmlEnDeCode.htmlEncode(show_str_temp) + "</div>";
		}
		else {
			html += "<div class='guestNetwork_content_edited'>" + htmlEnDeCode.htmlEncode(show_str) + "</div>";
		}
		if(captive_portal_used_wl_array[_gn_unit_subunit] == undefined) {
			html += "<div class='guest_line'></div>";
			html += "<div class='guestNetwork_titleName_edited'><#mssid_time_remaining#></div>";
			if(_gn_array[11] == 0)
				html += "<div class='guestNetwork_content_edited'><#Limitless#></div>";
			else {
				var expire_day = Math.floor(_gn_array[13]/86400);
				var expire_hr = Math.floor((_gn_array[13]%86400)/3600);
				var expire_min = Math.floor((_gn_array[13]%3600)/60);
				if(expire_day > 0)
					html += "<div class='guestNetwork_content_edited'><b>"+ expire_day + "</b> <#Day#> <b>"+ expire_hr + "</b> <#Hour#> <b>" + expire_min +"</b> <#Minute#></div>";
				else if(expire_hr > 0)
					html += "<div class='guestNetwork_content_edited'><b>"+ expire_hr + "</b> <#Hour#> <b>" + expire_min +"</b> <#Minute#></div>";
				else {
					if(expire_min > 0)
						html += "<div class='guestNetwork_content_edited'><b>" + expire_min +"</b> <#Minute#></div>";
					else
						html += "<div class='guestNetwork_content_edited'><b>< 1</b> <#Minute#></div>";
				}
			}
		}
		if(sw_mode != "3") {
			if(captive_portal_used_wl_array[_gn_unit_subunit] == undefined) {
				html += "<div class='guest_line'></div>";
				html += "<div class='guestNetwork_titleName_edited'><#Access_Intranet#></div>";
				html += "<div class='guestNetwork_content_edited'>" + _gn_array[12] + "</div>";
			}
		}

		if(captive_portal_used_wl_array[_gn_unit_subunit] != undefined) {
			html += "<div class='guest_line'></div>";
			html += "<div class='guestNetwork_titleName_edited'><#Access_Control_title#></div>";
			html += "<div class='guestNetwork_content_edited'>Used by " + captive_portal_used_wl_array[_gn_unit_subunit] + "</div>";
		}
		html += "<div class='guest_line'></div>";
		html += "</div>";
		return html;
	};

	var htmlcode = ""; 
	var guest_group_num = gn_array_2g.length;
	//no5gmssid_support =true;

	//short term solution for only router mode support Captive Portal
	if(isSwMode("rt")) {
		//captive portal used wl if
		if(captivePortal_support) {
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
			if(cp_freewifi_support) {
				_enable_flag = '<% nvram_get("captive_portal_enable"); %>';
				if(_enable_flag == "on") {
					_profile_list = decodeURIComponent('<% nvram_char_to_ascii("","captive_portal"); %>');
					parse_wl_list(_profile_list, 5, "Free Wi-Fi", captive_portal_used_wl_array);
				}
			}
			//check captive portal adv
			if(cp_advanced_support) {
				_enable_flag = '<% nvram_get("captive_portal_adv_enable"); %>';
				if(_enable_flag == "on") {
					_profile_list = decodeURIComponent('<% nvram_char_to_ascii("","captive_portal_adv_profile"); %>');
					parse_wl_list(_profile_list, 5, "Captive Portal Wi-Fi", captive_portal_used_wl_array);
				}
			}
		}
		
		//check fb wi-fi
		if(fbwifi_support) {
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
		}
	}

	var gn_unit_subunit = "";
	var captive_portal_used_interface_array = new Array();
	//short term solution for only router mode support Captive Portal
	if(isSwMode("rt")) {
		if(fbwifi_support)
			captive_portal_used_interface_array[get_captive_portal_wl_idx("facebookWiFi")] = "Facebook Wi-Fi";/*untranslated*/
		if(captivePortal_support) {
			if(cp_advanced_support)
				captive_portal_used_interface_array[get_captive_portal_wl_idx("captivePortal")] = "Captive Portal";/*untranslated*/
			if(cp_freewifi_support)
				captive_portal_used_interface_array[get_captive_portal_wl_idx("freeWiFi")] = "Free W-Fi";/*untranslated*/
		}
	}
	var td_width = (wl_info.band5g_2_support) ? "33%" : (wl_info.band5g_support && !no5gmssid_support) ? "50%" : "100%";
	for(var i = 1; i <= guest_group_num; i += 1) {

		if(tmo_support && i == 3)	{ //keep wlx.3 for usingg Passpoint
			break;
		}

		htmlcode += "<div class='guest_line' style='margin-bottom:23px;'></div>";
		htmlcode += "<div class='capitial_GuestNetwork_title'>";

		var width_size = "100%";
		if(smart_connect_support)
			width_size = "70%";
		htmlcode += "<div style='float:left;width:" + width_size + ";'>";
		htmlcode += "<div class='capitial_GuestNetworkName' onclick='showHideContent(\"guest_block" + i + "\", this);'>";
		htmlcode += "<div id='guest_block" + i + "_expand' class='capitial_GuestNetwork_expand'>-</div>";
		htmlcode += "<div id='guest_block_anchor_" + i + "'></div>";
		htmlcode += "<div><#Guest_Network#> - " + i;
		if(captive_portal_used_interface_array[i] != undefined) {
			htmlcode += "  (Default used by " + captive_portal_used_interface_array[i] + ")";
		}
		htmlcode += "</div>"
		htmlcode += "</div>";
		htmlcode += "</div>";

		if(smart_connect_support) {
			htmlcode += "<div style='float:right;width:30%;'>";
			htmlcode += "<div style='float:left;width:55%'>";
			htmlcode += "<div class='capitial_GuestNetworkName_IconName'>Smart Connect</div>";/*untranslated*/
			htmlcode += "</div>";
			htmlcode += "<div class='left' style='margin-top:4px;margin-right:24px;float:right' id='radio_guest_smartconnnect_" + i + "'></div>";
			
			htmlcode += "</div>";
		}

		htmlcode += "</div>";
		
		htmlcode += "<div id='guest_block" + i + "'>";
		htmlcode += "<table style='width:100%;' cellpadding='0' cellspacing='0'><tr>";
		
		//2.4G
		htmlcode += "<td style='width:" + td_width + ";vertical-align:top;'>";
		gn_unit_subunit = "wl0." + i;
		if(gn_array_2g[i - 1][0] != "1") {
			htmlcode += gen_new_guest("0", i);
		}
		else {
			htmlcode += gen_had_enable_guest(gn_array_2g[i - 1], "0", i);
		}
		htmlcode += "</td>";
		//5G
		if(wl_info.band5g_support && !no5gmssid_support) {
			htmlcode += "<td style='width:" + td_width + ";vertical-align:top;'>";
			gn_unit_subunit = "wl1." + i;
			if(gn_array_5g[i - 1][0] != "1") {
				htmlcode += gen_new_guest("1", i);
			}
			else {
				htmlcode += gen_had_enable_guest(gn_array_5g[i - 1], "1", i);
			}
			htmlcode += "</td>";
		}
		//5G-2
		if(wl_info.band5g_2_support) {
			htmlcode += "<td style='width:" + td_width + ";vertical-align:top;'>";
			gn_unit_subunit = "wl2." + i;
			if(gn_array_5g_2[i - 1][0] != "1") {
				htmlcode += gen_new_guest("2", i);
			}
			else {
				htmlcode += gen_had_enable_guest(gn_array_5g_2[i - 1], "2", i);
			}
			htmlcode += "</td>";
		}
		htmlcode += "</tr></table>";
		htmlcode += "</div>";
	}

	document.getElementById("guest_table").innerHTML = htmlcode;


	$('#radio_guest_smartconnnect_1').iphoneSwitch(0,
		function(){
		
		},
		function(){

		}
	);
	$('#radio_guest_smartconnnect_2').iphoneSwitch(0,
		function(){
		
		},
		function(){

		}
	);
	$('#radio_guest_smartconnnect_3').iphoneSwitch(0,
		function(){
		
		},
		function(){

		}
	);

	//Register Switch Function
	var settingSwitchFun = function(_unit, _subunit, _state) {
		$('#radio_guest_enable_' + _unit + '_' + _subunit + '').iphoneSwitch(_state,
			function(){
				en_dis_guest_unit(_unit, _subunit, 1);
			},
			function(){
				en_dis_guest_unit(_unit, _subunit, 0);
			}
		);
	};
	for(var i = 1; i <= guest_group_num; i += 1) {
		//2.4G
		settingSwitchFun(0, i, gn_array_2g[i - 1][0]);
		//5G
		if(wl_info.band5g_support) {
			settingSwitchFun(1, i, gn_array_5g[i - 1][0]);
		}
		//5G-2
		if(wl_info.band5g_2_support) {
			settingSwitchFun(2, i, gn_array_5g_2[i - 1][0]);
		}
	}

}

function applyRule(){
	var auth_mode = document.form.wl_auth_mode_x.value;
	if(document.form.wl_wpa_psk.value == "<#wireless_psk_fillin#>")
		document.form.wl_wpa_psk.value = "";

	if(validForm()){
		updateMacList();
		if(document.form.wl_expire_radio[1].checked)
			document.form.wl_expire.value = document.form.wl_expire_day.value*86400 + document.form.wl_expire_hr.value*3600 + document.form.wl_expire_min.value*60;
		else
			document.form.wl_expire.value = 0;

		if(based_modelid == "RT-AC87U") //MODELDEP: RT-AC87U need to extend waiting time to get new wl value
			document.form.action_wait.value = parseInt(document.form.action_wait.value)+5;
		if(based_modelid == "BRT-AC828")
			document.form.action_wait.value = 50;

		if(auth_mode == "wpa" || auth_mode == "wpa2" || auth_mode == "wpawpa2" || auth_mode == "radius") {
			document.form.next_page.value = "/Advanced_WSecurity_Content.asp";
			document.form.gwlu.value =  document.form.wl_unit.value;
			document.form.gwlu.disabled = false;
		}

		if(document.form.bw_enabled_x[0].checked)
			document.form.wl_bw_enabled.value = 1;
		else
			document.form.wl_bw_enabled.value = 0;	
		document.form.wl_bw_dl.value = document.form.wl_bw_dl_x.value*1024;
		document.form.wl_bw_ul.value = document.form.wl_bw_ul_x.value*1024;		
				
		if((QoS_enable_orig == "0" || QoS_type_orig != "2") && (document.form.bw_enabled_x.value == "1" || document.form.bw_enabled_x[0].checked))
		{
			document.form.qos_enable.value = 1;
			document.form.qos_type.value = 2;
			if(confirm("QoS function of traffic manager will be enable and set as bandwidth limiter mode by default.")){	/* Untranslated */
				if(ctf_disable_orig == '0'){	//brcm NAT Acceleration turned ON
					document.form.action_script.value = "saveNvram;reboot";
					document.form.action_wait.value = "<% get_default_reboot_time(); %>";
				}
				else{
					document.form.action_script.value = "restart_wireless;restart_qos;restart_firewall;";
				}	
			}
			else{
				return;	
			}		
		}
		else if(unit_bw_enabled != document.form.wl_bw_enabled.value //bandwidth limiter settings changed OR re-enable mSSID with bandwidth limiter
				|| unit_bw_ul != document.form.wl_bw_ul.value 
				|| unit_bw_dl != document.form.wl_bw_dl.value
				|| (wl_x_y_bss_enabled == 1 && (document.form.bw_enabled_x.value == "1" || document.form.bw_enabled_x[0].checked)))	
		{	
			if(ctf_disable_orig == '0' && document.form.wl_bw_enabled.value == 1){
				document.form.action_script.value = "saveNvram;reboot";
				document.form.action_wait.value = "<% get_default_reboot_time(); %>";
			}
			else{
				document.form.action_script.value = "restart_wireless;restart_qos;restart_firewall;";
			}
		}

		var _unit_subunit = "wl" + document.form.wl_unit.value + "." + document.form.wl_subunit.value;
		if(captive_portal_used_wl_array[_unit_subunit] != undefined) {
			document.form.wl_key.disabled = true;
			document.form.wl_key1.disabled = true;
			document.form.wl_key2.disabled = true;
			document.form.wl_key3.disabled = true;
			document.form.wl_key4.disabled = true;
			document.form.wl_wep_x.disabled = true;
			document.form.wl_phrase_x.disabled = true;
			document.getElementById("lanaccess_on").disabled = true;
			document.getElementById("lanaccess_off").disabled = true;
			document.form.wl_expire.disabled = true;
			document.form.wl_guest_num.disabled = true;
			document.form.wl_bw_enabled.disabled = true;
			document.form.wl_bw_dl.disabled = true;
			document.form.wl_bw_ul.disabled = true;	
			switch(captive_portal_used_wl_array[_unit_subunit]) {
				case "Facebook Wi-Fi" :
					document.form.action_script.value = "overwrite_fbwifi_ssid;" + document.form.action_script.value;
					break;
				case "Free Wi-Fi" :
					document.form.action_script.value = "overwrite_captive_portal_ssid;" + document.form.action_script.value;
					break;
				case "Captive Portal Wi-Fi" :
					document.form.action_script.value = "overwrite_captive_portal_adv_ssid;" + document.form.action_script.value;
					break;
			}
		}

		inputCtrl(document.form.wl_crypto, 1);
		inputCtrl(document.form.wl_wpa_psk, 1);
		inputCtrl(document.form.wl_wep_x, 1);
		inputCtrl(document.form.wl_key, 1);
		inputCtrl(document.form.wl_key1, 1);
		inputCtrl(document.form.wl_key2, 1);
		inputCtrl(document.form.wl_key3, 1);
		inputCtrl(document.form.wl_key4, 1);
		inputCtrl(document.form.wl_phrase_x, 1);

		showLoading();
		document.form.submit();
	}
}

function validForm(){
	var auth_mode = document.form.wl_auth_mode_x.value;
	
	if(!validator.stringSSID(document.form.wl_ssid))
		return false;
	
	if(document.form.wl_wep_x.value != "0")
		if(!validate_wlphrase('WLANConfig11b', 'wl_phrase_x', document.form.wl_phrase_x))
			return false;	
	if(auth_mode == "psk" || auth_mode == "psk2" || auth_mode == "pskpsk2"){ //2008.08.04 lock modified
		if(is_KR_sku){
			if(!validator.psk_KR(document.form.wl_wpa_psk, document.form.wl_unit.value))
                                return false;
		}
		else{
			if(!validator.psk(document.form.wl_wpa_psk, document.form.wl_unit.value))
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
	else{
		var cur_wep_key = eval('document.form.wl_key'+document.form.wl_key.value);		
		if(auth_mode != "radius" && !validator.wlKey(cur_wep_key))
			return false;
	}	
	
	//confirm expire time not allow zero
	if(document.form.wl_expire_radio[1].checked){
		if(document.form.wl_expire_day.value==0 && (document.form.wl_expire_hr.value=="" || document.form.wl_expire_hr.value==0) & (document.form.wl_expire_min.value=="" || document.form.wl_expire_min.value==0)){
			alert("<#JS_fieldblank#>");
			document.form.wl_expire_min.focus();
			return false;
		}	
	}

	//bandwidth limiter
	if(document.form.bw_enabled_x[0].checked){
		
		if(document.form.wl_bw_dl_x.value == ""){
			alert("<#JS_fieldblank#>");
			document.form.wl_bw_dl_x.focus();
			return false;
		}
	
		if(document.form.wl_bw_dl_x.value.split(".").length > 2 || document.form.wl_bw_dl_x.value < 0.1){
			alert("<#min_bound#> : 0.1 Mb/s");
			document.form.wl_bw_dl_x.focus();
			return false;
		}
		
		if(document.form.wl_bw_ul_x.value == ""){
			alert("<#JS_fieldblank#>");
			document.form.wl_bw_ul_x.focus();
			return false;
		}
	
		if(document.form.wl_bw_ul_x.value.split(".").length > 2 || document.form.wl_bw_ul_x.value < 0.1){
			alert("<#min_bound#> : 0.1 Mb/s");
			document.form.wl_bw_ul_x.focus();
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

var wl_x_y_bss_enabled = 0;
function en_dis_guest_unit(_unit, _subunit, _setting){
	var NewInput = document.createElement("input");
	NewInput.type = "hidden";
	NewInput.name = "wl"+ _unit + "." + _subunit +"_bss_enabled";
	NewInput.value = _setting;
	wl_x_y_bss_enabled = _setting;	
	document.unitform.appendChild(NewInput);
	document.unitform.wl_unit.value = _unit;
	document.unitform.wl_subunit.value = _subunit;
	if(based_modelid == "BRT-AC828")
		document.form.action_wait.value = 50; //MODELDEP: BRT-AC828
	document.unitform.submit();
}
function updateLanaccess() {
	//document.getElementById("gnset_wl_vlanTag").style.display = "none";
	document.getElementById("gnset_wl_clientIsolation").style.display = "none";
	if(document.form.wl_lanaccess.value == "vlan") {
	//	document.getElementById("gnset_wl_vlanTag").style.display = "";
		document.getElementById("gnset_wl_clientIsolation").style.display = "";
	}
}

// mac filter
function updateMacModeOption(gn_array){
	wl_maclist_x_array = gn_array[16];
	var wl_maclist_x_row = wl_maclist_x_array.split('&#60');
	var clientName = "New device";
	manually_maclist_list_array = [];
	for(var i = 1; i < wl_maclist_x_row.length; i += 1) {
		if(clientList[wl_maclist_x_row[i]]) {
			clientName = (clientList[wl_maclist_x_row[i]].nickName == "") ? clientList[wl_maclist_x_row[i]].name : clientList[wl_maclist_x_row[i]].nickName;
		}
		else {
			clientName = "New device";
		}
		manually_maclist_list_array[wl_maclist_x_row[i]] = clientName;
	}
	show_wl_maclist_x();

	document.form.wl_macmode.value = gn_array[14];
	document.form.wl_maclist_x.value = gn_array[16];

	var _unit_subunit = document.form.wl_unit.value + "." + document.form.wl_subunit.value
	document.getElementById("maclistMain").style.display = (document.form.wl_macmode.value == "disabled") ? "none" : "";
}

function show_wl_maclist_x(){
	var code = "";
	code +='<table width="100%" border="1" cellspacing="0" cellpadding="4" align="center" class="list_table"  id="wl_maclist_x_table">'; 
	if(Object.keys(manually_maclist_list_array).length == 0)
		code +='<tr><td style="color:#FFCC00;"><#IPConnection_VSList_Norule#></td>';
	else{
		//user icon
		var userIconBase64 = "NoIcon";
		var clientName, deviceType, deviceVender;
		Object.keys(manually_maclist_list_array).forEach(function(key) {
			var clientMac = key;
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
			code +='<tr id="row_'+clientMac+'">';
			code +='<td width="80%" align="center">';
			code += '<table style="width:100%;"><tr><td style="width:40%;height:56px;border:0px;float:right;">';
			if(clientList[clientMac] == undefined) {
				code += '<div class="clientIcon type0" onClick="popClientListEditTable(\'' + clientMac + '\', this, \'\', \'\', \'GuestNetwork\')"></div>';
			}
			else {
				if(usericon_support) {
					userIconBase64 = getUploadIcon(clientMac.replace(/\:/g, ""));
				}
				if(userIconBase64 != "NoIcon") {
					code += '<div style="text-align:center;" onClick="popClientListEditTable(\'' + clientMac + '\', this, \'\', \'\', \'GuestNetwork\')"><img class="imgUserIcon_card" src="' + userIconBase64 + '"></div>';
				}
				else if(deviceType != "0" || deviceVender == "") {
					code += '<div class="clientIcon type' + deviceType + '" onClick="popClientListEditTable(\'' +clientMac + '\', this, \'\', \'\', \'GuestNetwork\')"></div>';
				}
				else if(deviceVender != "" ) {
					var venderIconClassName = getVenderIconClassName(deviceVender.toLowerCase());
					if(venderIconClassName != "" && !downsize_4m_support) {
						code += '<div class="venderIcon ' + venderIconClassName + '" onClick="popClientListEditTable(\'' + clientMac + '\', this, \'\', \'\', \'GuestNetwork\')"></div>';
					}
					else {
						code += '<div class="clientIcon type' + deviceType + '" onClick="popClientListEditTable(\'' + clientMac + '\', this, \'\', \'\', \'GuestNetwork\')"></div>';
					}
				}
			}
			code += '</td><td style="width:60%;border:0px;">';
			code += '<div>' + clientName + '</div>';
			code += '<div>' + clientMac + '</div>';
			code += '</td></tr></table>';
			code += '</td>';
			code +='<td width="20%"><input type="button" class=\"remove_btn\" onclick=\"deleteRow(this, \'' + clientMac + '\');\" value=\"\"/></td></tr>';		
		});
	}	
	
  	code +='</tr></table>';
	document.getElementById("wl_maclist_x_Block").innerHTML = code;
}

function deleteRow(r, delMac){
	var i = r.parentNode.parentNode.rowIndex;
	delete manually_maclist_list_array[delMac];
	document.getElementById('wl_maclist_x_table').deleteRow(i);

	if(Object.keys(manually_maclist_list_array).length == 0)
		show_wl_maclist_x();
}

function addRow(obj, upper){
	var rule_num = document.getElementById('wl_maclist_x_table').rows.length;
	var item_num = document.getElementById('wl_maclist_x_table').rows[0].cells.length;
	var mac = obj.value.toUpperCase();

	if(rule_num >= upper){
		alert("<#JS_itemlimit1#> " + upper + " <#JS_itemlimit2#>");
		return false;	
	}	
	
	if(mac==""){
		alert("<#JS_fieldblank#>");
		obj.focus();
		obj.select();			
		return false;
	}else if(!check_macaddr(obj, check_hwaddr_flag(obj))){
		obj.focus();
		obj.select();	
		return false;	
	}
		
		//Viz check same rule
	for(i=0; i<rule_num; i++){
		for(j=0; j<item_num-1; j++){	
			if(manually_maclist_list_array[mac] != null){
				alert("<#JS_duplicate#>");
				return false;
			}	
		}		
	}		
	
	if(clientList[mac]) {
		manually_maclist_list_array[mac] = (clientList[mac].nickName == "") ? clientList[mac].name : clientList[mac].nickName;
	}
	else {
		manually_maclist_list_array[mac] = "New device";
	}

	obj.value = ""
	show_wl_maclist_x();
}

function updateMacList(){
	var rule_num = document.getElementById('wl_maclist_x_table').rows.length;
	var item_num = document.getElementById('wl_maclist_x_table').rows[0].cells.length;
	var tmp_value = "";

	Object.keys(manually_maclist_list_array).forEach(function(key) {
		tmp_value += "<" + key;
	});

	if(tmp_value == "<"+"<#IPConnection_VSList_Norule#>" || tmp_value == "<")
		tmp_value = "";	

	document.form.wl_maclist_x.value = tmp_value;
}

function change_wl_unit(){
	FormActions("apply.cgi", "change_wl_unit", "", "");
	document.form.target = "";
	document.form.submit();
}

function check_macaddr(obj,flag){ //control hint of input mac address
	if(flag == 1){
		var childsel=document.createElement("div");
		childsel.setAttribute("id","check_mac");
		childsel.style.color="#FFCC00";
		obj.parentNode.appendChild(childsel);
		document.getElementById("check_mac").innerHTML="<#LANHostConfig_ManualDHCPMacaddr_itemdesc#>";		
		document.getElementById("check_mac").style.display = "";
		return false;
	}else if(flag ==2){
		var childsel=document.createElement("div");
		childsel.setAttribute("id","check_mac");
		childsel.style.color="#FFCC00";
		obj.parentNode.appendChild(childsel);
		document.getElementById("check_mac").innerHTML="<#IPConnection_x_illegal_mac#>";
		document.getElementById("check_mac").style.display = "";
		return false;		
	}else{	
		document.getElementById("check_mac") ? document.getElementById("check_mac").style.display="none" : true;
		return true;
	}	
}

//Viz add 2013.01 pull out WL client mac START
function pullWLMACList(obj){
	var element = document.getElementById('WL_MAC_List_Block');
	var isMenuopen = element.offsetWidth > 0 || element.offsetHeight > 0;	
	if(isMenuopen == 0){		
		obj.src = "/images/arrow-top.gif"
		element.style.display = "block";
		document.form.wl_maclist_x_0.focus();		
	}
	else
		hideClients_Block();
}

function hideClients_Block(){
	document.getElementById("pull_arrow").src = "/images/arrow-down.gif";
	document.getElementById("WL_MAC_List_Block").style.display="none";
}

function setClientmac(macaddr){
	document.form.wl_maclist_x_0.value = macaddr;
	hideClients_Block();
}
// end

function show_bandwidth(flag){	
	if(flag == "1"){
		document.form.bw_enabled_x[0].checked = true;
		if(ctf_disable_orig == '0'){	//brcm NAT Acceleration turned ON
			document.getElementById("QoS_hint").innerHTML = "NAT acceleration will be disable for more precise packet inspection.";	/* untranslated */
			document.getElementById("QoS_hint").style.display = "";	
		}
		else{
			document.getElementById("QoS_hint").style.display = "none";	
		}
		document.getElementById("gnset_wl_bw_setting").style.display = "";
	}
	else{		
		document.form.bw_enabled_x[1].checked = true;
		document.getElementById("QoS_hint").style.display = "none";
		document.getElementById("gnset_wl_bw_setting").style.display = "none";
	}	
}

function bandwidth_code(o,event){
	var keyPressed = event.keyCode ? event.keyCode : event.which;
	var target = o.value.split(".");
	
	if (validator.isFunctionButton(event))
		return true;	
		
	if((keyPressed == 46) && (target.length > 1))
		return false;

	if((target.length > 1) && (target[1].length > 0))
		return false;	
		
	if ((keyPressed == 46) || (keyPressed > 47 && keyPressed < 58))
		return true;
	else
		return false;		
}
</script>
</head>

<body onload="initial();">
<div id="full_screen_bg" class="full_screen_bg" onselectstart="return false;"></div>
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
<form method="post" name="unitform" action="/start_apply2.htm" target="hidden_frame">
<input type="hidden" name="current_page" value="Guest_network.asp">
<input type="hidden" name="next_page" value="Guest_network.asp">
<input type="hidden" name="wl_unit" value="<% nvram_get("wl_unit"); %>">
<input type="hidden" name="wl_subunit" value="<% nvram_get("wl_subunit"); %>">
<input type="hidden" name="action_mode" value="apply_new">
<input type="hidden" name="action_script" value="restart_wireless;restart_qos;restart_firewall;">
<input type="hidden" name="action_wait" value="15">
<input type="hidden" name="wl_mbss" value="1">
</form>
<form method="post" name="form" action="/start_apply2.htm" target="hidden_frame">
<input type="hidden" name="productid" value="<% nvram_get("productid"); %>">
<input type="hidden" name="current_page" value="Guest_network.asp">
<input type="hidden" name="next_page" value="Guest_network.asp">
<input type="hidden" name="gwlu" value="" disabled>
<input type="hidden" name="modified" value="0">
<input type="hidden" name="action_mode" value="apply_new">
<input type="hidden" name="action_script" value="restart_wireless">
<input type="hidden" name="action_wait" value="15">
<input type="hidden" name="preferred_lang" id="preferred_lang" value="<% nvram_get("preferred_lang"); %>">
<input type="hidden" name="wl_country_code" value="<% nvram_get("wl0_country_code"); %>" disabled>
<input type="hidden" name="firmver" value="<% nvram_get("firmver"); %>">
<input type="hidden" name="wl_ssid_org" value="<% nvram_char_to_ascii("WLANConfig11b",  "wl_ssid"); %>">
<input type="hidden" name="wl_wpa_psk_org" value="<% nvram_char_to_ascii("WLANConfig11b", "wl_wpa_psk"); %>">
<input type="hidden" name="wl_key1_org" value="<% nvram_char_to_ascii("WLANConfig11b", "wl_key1"); %>">
<input type="hidden" name="wl_key2_org" value="<% nvram_char_to_ascii("WLANConfig11b", "wl_key2"); %>">
<input type="hidden" name="wl_key3_org" value="<% nvram_char_to_ascii("WLANConfig11b", "wl_key3"); %>">
<input type="hidden" name="wl_key4_org" value="<% nvram_char_to_ascii("WLANConfig11b", "wl_key4"); %>">
<input type="hidden" name="wl_phrase_x_org" value="<% nvram_char_to_ascii("WLANConfig11b", "wl_phrase_x"); %>">
<input type="hidden" name="x_RegulatoryDomain" value="<% nvram_get("x_RegulatoryDomain"); %>" readonly="1">
<input type="hidden" name="wl_wme" value="<% nvram_get("wl_wme"); %>" disabled>
<input type="hidden" name="wl_nctrlsb_old" value="<% nvram_get("wl_nctrlsb"); %>">
<input type="hidden" name="wl_key_type" value='<% nvram_get("wl_key_type"); %>'> <!--Lock Add 2009.03.10 for ralink platform-->
<input type="hidden" name="wl_channel_orig" value='<% nvram_get("wl_channel"); %>'>
<input type="hidden" name="wl_expire" value='<% nvram_get("wl_expire"); %>'>
<input type="hidden" name="qos_enable" value='<% nvram_get("qos_enable"); %>'>
<input type="hidden" name="qos_type" value='<% nvram_get("qos_type"); %>'>
<input type="hidden" name="wl_bw_enabled" value="">
<input type="hidden" name="wl_bw_dl" value="">
<input type="hidden" name="wl_bw_ul" value="">
<input type="hidden" name="wl_mbss" value="1">
<input type="hidden" name="wl_gmode_protection" value="<% nvram_get("wl_gmode_protection"); %>" disabled>
<input type="hidden" name="wl_mode_x" value="<% nvram_get("wl_mode_x"); %>" disabled>
<input type="hidden" name="wl_unit" value="<% nvram_get("wl_unit"); %>">
<input type="hidden" name="wl_subunit" value="<% nvram_get("wl_subunit"); %>">
<input type="hidden" name="wl_maclist_x" value="<% nvram_get("wl_maclist_x"); %>">

<!-- setting table -->
<div id="gnset_block" class="gnset_edit_bg">
	<div style="height:54px;width:100%">
		<div class='gnset_close' onclick="gnset_cancel();"></div>
	</div>
	<div style="height:48px;">
		<div id="gn_interfce" style="width:70%;float:left;" class="gnset_edit_titleName"></div>
		<div id="gn_copyOtherSetting" style="width:30%;float:right;" >
			<div id="copyOtherSettingIcon" class='gnset_copySettingContent_icon'></div>
			<div class="gnset_copySettingContent_txt">
				COPY OTHER<!--untranslated-->
				<br>
				<#Settings#>
			</div>
			
		</div>
	</div>
	<div id="edited_list" class="gnset_copySettingContent_bg">
		<div class="gnset_copySettingContent_title">Using the Same Settings With...<!--untranslated--></div>
		
		<div id="edited_guestnetwork_list" class="gnset_copySettingContent_list_bg"></div>
	</div>
	<div style="margin-top:20px;">
		<div class="gnset_setting_item_bg">
			<div class="gnset_setting_item_titleName">
				<span class="gnset_hintstyle" href="javascript:void(0);" onClick="openHint(0, 1);" onmouseout="return nd();"><#QIS_finish_wireless_item1#></span>
			</div>
			<div class='gnset_setting_item_content'>
				<input type="text" maxlength="32" class="gnset_setting_input_text_full" name="wl_ssid" value="<% nvram_get("wl_ssid"); %>" onkeypress="return validator.isString(this, event)" autocorrect="off" autocapitalize="off">
			</div>
		</div>
		<div class="gnset_setting_item_bg">
			<div class='gnset_setting_item_titleName'>
				<span class="gnset_hintstyle" href="javascript:void(0);" onClick="openHint(0, 5);" onmouseout="return nd();"><#WLANConfig11b_AuthenticationMethod_itemname#></span>
			</div>
			<div class='gnset_setting_item_content'>
				<select name="wl_auth_mode_x" class="gnset_setting_input_option" onChange="authentication_method_change(this);"></select>
			</div>
		</div>
		<div class="gnset_setting_item_bg">
			<div class='gnset_setting_item_titleName'>
				<span class="gnset_hintstyle" href="javascript:void(0);" onClick="openHint(0, 6);" onmouseout="return nd();"><#WLANConfig11b_WPAType_itemname#></span>
			</div>
			<div class='gnset_setting_item_content'>
				<select name="wl_crypto" class="gnset_setting_input_option" onChange="authentication_method_change(this);">
					<option value="aes" <% nvram_match("wl_crypto", "aes", "selected"); %>>AES</option>
					<option value="tkip+aes" <% nvram_match("wl_crypto", "tkip+aes", "selected"); %>>TKIP+AES</option>
				</select>
			</div>
		</div>
		<div class="gnset_setting_item_bg">
			<div class='gnset_setting_item_titleName'>
				<span class="gnset_hintstyle" href="javascript:void(0);" onClick="openHint(0, 9);" onmouseout="return nd();"><#WLANConfig11b_WEPType_itemname#></span>
			</div>
			<div class='gnset_setting_item_content'>
				<select name="wl_wep_x" class="gnset_setting_input_text_autoWidth" onChange="wep_encryption_change(this);">
					<option value="0" <% nvram_match("wl_wep_x", "0", "selected"); %>><#wl_securitylevel_0#></option>
					<option value="1" <% nvram_match("wl_wep_x", "1", "selected"); %>>WEP-64bits</option>
					<option value="2" <% nvram_match("wl_wep_x", "2", "selected"); %>>WEP-128bits</option>
				</select>
				<span name="key_des"></span>
			</div>
		</div>
		<div class="gnset_setting_item_bg">
			<div class='gnset_setting_item_titleName'>
				<span class="gnset_hintstyle" href="javascript:void(0);" onClick="openHint(0, 10);" onmouseout="return nd();"><#WLANConfig11b_WEPDefaultKey_itemname#></span>
			</div>
			<div class='gnset_setting_item_content'>
				<select name="wl_key" class="gnset_setting_input_text_autoWidth"  onChange="wep_key_index_change(this);">
					<option value="1" <% nvram_match("wl_key", "1","selected"); %>>1</option>
					<option value="2" <% nvram_match("wl_key", "2","selected"); %>>2</option>
					<option value="3" <% nvram_match("wl_key", "3","selected"); %>>3</option>
					<option value="4" <% nvram_match("wl_key", "4","selected"); %>>4</option>
				</select>
			</div>
		</div>
		<div class="gnset_setting_item_bg">
			<div class='gnset_setting_item_titleName'>
				<span class="gnset_hintstyle" href="javascript:void(0);" onClick="openHint(0, 18);" onmouseout="return nd();"><#WLANConfig11b_WEPKey1_itemname#></span>
			</div>
			<div class='gnset_setting_item_content'>
				<input type="text" name="wl_key1" id="wl_key1" maxlength="32" class="gnset_setting_input_text_full" value="<% nvram_get("wl_key1"); %>" onKeyUp="return change_wlkey(this, 'WLANConfig11b');" autocorrect="off" autocapitalize="off">
			</div>
		</div>
		<div class="gnset_setting_item_bg">
			<div class='gnset_setting_item_titleName'>
				<span class="gnset_hintstyle" href="javascript:void(0);" onClick="openHint(0, 18);" onmouseout="return nd();"><#WLANConfig11b_WEPKey2_itemname#></span>
			</div>
			<div class='gnset_setting_item_content'>
				<input type="text" name="wl_key2" id="wl_key2" maxlength="32" class="gnset_setting_input_text_full" value="<% nvram_get("wl_key2"); %>" onKeyUp="return change_wlkey(this, 'WLANConfig11b');" autocorrect="off" autocapitalize="off">
			</div>
		</div>
		<div class="gnset_setting_item_bg">
			<div class='gnset_setting_item_titleName'>
				<span class="gnset_hintstyle" href="javascript:void(0);" onClick="openHint(0, 18);" onmouseout="return nd();"><#WLANConfig11b_WEPKey3_itemname#></span>
			</div>
			<div class='gnset_setting_item_content'>
				<input type="text" name="wl_key3" id="wl_key3" maxlength="32" class="gnset_setting_input_text_full" value="<% nvram_get("wl_key3"); %>" onKeyUp="return change_wlkey(this, 'WLANConfig11b');" autocorrect="off" autocapitalize="off">
			</div>
		</div>
		<div class="gnset_setting_item_bg">
			<div class='gnset_setting_item_titleName'>
				<span class="gnset_hintstyle" href="javascript:void(0);" onClick="openHint(0, 18);" onmouseout="return nd();"><#WLANConfig11b_WEPKey4_itemname#></span>
			</div>
			<div class='gnset_setting_item_content'>
				<input type="text" name="wl_key4" id="wl_key4" maxlength="32" class="gnset_setting_input_text_full" value="<% nvram_get("wl_key4"); %>" onKeyUp="return change_wlkey(this, 'WLANConfig11b');" autocorrect="off" autocapitalize="off">
			</div>
		</div>
		<div class="gnset_setting_item_bg">
			<div class='gnset_setting_item_titleName'>
				<span class="gnset_hintstyle" href="javascript:void(0);" onClick="openHint(0, 8);" onmouseout="return nd();"><#WLANConfig11b_x_Phrase_itemname#></span>
			</div>
			<div class='gnset_setting_item_content'>
				<input type="text" name="wl_phrase_x" maxlength="64" class="gnset_setting_input_text_full" value="<% nvram_get("wl_phrase_x"); %>" onKeyUp="return is_wlphrase('WLANConfig11b', 'wl_phrase_x', this);" autocorrect="off" autocapitalize="off">
			</div>
		</div>
		<div id="gnset_wl_guest_num" class="gnset_setting_item_bg">
			<div class='gnset_setting_item_titleName'>
				Total Guest<!--untranslated-->
			</div>
			<div class='gnset_setting_item_content'>
				<div style="float:left;width:50%">
					<select name="wl_guest_num" class="gnset_setting_input_text_autoWidth"></select>
					<span class="gnset_des">guests</span><!--untranslated-->
				</div>
				<!--div class="gnset_setting_note">UP TO 50 GUESTS</div-->
			</div>
		</div>
		<div class="gnset_setting_item_bg">
			<div class='gnset_setting_item_titleName'>
				<span class="gnset_hintstyle" href="javascript:void(0);" onClick="openHint(0, 7);" onmouseout="return nd();"><#WLANConfig11b_x_PSKKey_itemname#></span>
			</div>
			<div class='gnset_setting_item_content'>
				<input type="text" name="wl_wpa_psk" maxlength="64" class="gnset_setting_input_text_full" value="<% nvram_get("wl_wpa_psk"); %>" autocorrect="off" autocapitalize="off">
			</div>
		</div>
		<div id="gnset_wl_expire" class="gnset_setting_item_bg">
			<div class='gnset_setting_item_titleName'>
				<span class="gnset_hintstyle" href="javascript:void(0);" onClick="openHint(0, 25);" onmouseout="return nd();"><#Access_Time#></span>
			</div>
			<div class='gnset_setting_item_content'>
				<div class="gnset_setting_content_bg">
					<input type="radio" value="0" name="wl_expire_radio" id="wl_expire_radio" class="gnset_setting_input_radio" onClick="">
					<label for="wl_expire_radio" class="gnset_setting_content"><#Limitless#></label>
				</div>
				<div class="gnset_setting_content_bg">
					<input type="radio" value="1" name="wl_expire_radio" class="gnset_setting_input_radio" onClick="">
					<select name="wl_expire_day" class="gnset_setting_input_text_autoWidth"></select> <#Day#>
					<input type="text" maxlength="2" name="wl_expire_hr" class="gnset_setting_input_text_short"  value="" onKeyPress="return validator.isNumber(this,event);" onblur="validator.timeRange(this, 0)" autocorrect="off" autocapitalize="off"> 
					<span class="gnset_setting_content"><#Hour#></span>
					<input type="text" maxlength="2" name="wl_expire_min" class="gnset_setting_input_text_short"  value="" onKeyPress="return validator.isNumber(this,event);" onblur="validator.timeRange(this, 1)" autocorrect="off" autocapitalize="off"> 
					<span class="gnset_setting_content"><#Minute#></span>
					<br>	
				</div>
			</div>
		</div>
		<div id="gnset_wl_bw_enabled" class="gnset_setting_item_bg">
			<div class='gnset_setting_item_titleName'>
				Enable Bandwidth Limiter<!-- Untranslated -->
			</div>
			<div class='gnset_setting_item_content'>
				<div class="gnset_setting_content_bg">
					<input type="radio" name="bw_enabled_x" id="bw_enabled_x_on" value="1" onchange="show_bandwidth(1);">
					<label for="bw_enabled_x_on" class="gnset_setting_content"><#checkbox_Yes#></label>
				</div>
				<div class="gnset_setting_content_bg">
					<input type="radio" name="bw_enabled_x" id="bw_enabled_x_off" value="0" onchange="show_bandwidth(0);">
					<label for="bw_enabled_x_off" class="gnset_setting_content"><#checkbox_No#></label>
				</div>
				<span id="QoS_hint" style="color:#FC0;display:none;"></span>
			</div>
		</div>
		<div id="gnset_wl_bw_setting" class="gnset_setting_item_bg">
			<div class='gnset_setting_item_titleName'>
				<#Bandwidth_Limiter#>
			</div>
			<div class='gnset_setting_item_content'>
				<div class="gnset_setting_content_bg">
					<span class="gnset_setting_content"><#option_download#></span>
					<input type="text" id="wl_bw_dl_x" name="wl_bw_dl_x" maxlength="12" onkeypress="return bandwidth_code(this, event);" class="gnset_setting_input_text_short" value="">
					<span class="gnset_setting_content">Mb/s</span>
				</div>
				<div class="gnset_setting_content_bg">
					<span class="gnset_setting_content"><#option_upload#></span>
					<input type="text" id="wl_bw_ul_x" name="wl_bw_ul_x" maxlength="12" onkeypress="return bandwidth_code(this, event);" class="gnset_setting_input_text_short" value="">
					<span class="gnset_setting_content">Mb/s</span>
				</div>
			</div>
		</div>
		<div id="gnset_wl_lanaccess" class="gnset_setting_item_bg">
			<div class='gnset_setting_item_titleName'>
				Access limit<!-- Untranslated -->
			</div>
			<div class='gnset_setting_item_content'>
				<div class="gnset_setting_content_bg">
					<input type="radio" name="wl_lanaccess" id="lanaccess_on" value="on" onchange="updateLanaccess()">
					<label for="lanaccess_on" class="gnset_setting_content"><#Limitless#></label>
				</div>
				<div class="gnset_setting_content_bg">
					<input type="radio" name="wl_lanaccess" id="lanaccess_off" value="off" onchange="updateLanaccess()">
					<label for="lanaccess_off" class="gnset_setting_content">Only access internet<!-- Untranslated --></label>
				</div>
				<!--div class="gnset_setting_content_bg">
					<input type="radio" name="wl_lanaccess" id="lanaccess_vlan" value="vlan" onchange="updateLanaccess()">
					<label for="lanaccess_vlan" class="gnset_setting_content">VLAN</label>
				</div-->
			</div>
		</div>
		<div id="gnset_wl_vlanTag" class="gnset_setting_item_bg">
			<div class='gnset_setting_item_titleName'>
				VLAN ID<!--untranslated-->
			</div>
			<div class='gnset_setting_item_content'>
					<div class="gnset_setting_content" id="vlan_id"></div>
			</div>
		</div>
		<div id="gnset_wl_clientIsolation" class="gnset_setting_item_bg">
			<div class='gnset_setting_item_titleName'>
				Client isolation<!--untranslated-->
			</div>
			<div class='gnset_setting_item_content'>
				<div class="gnset_setting_content_bg">
					<input type="radio" name="wl_clientIsolation" id="wl_clientIsolation_disable" value="disabled" checked>
					<label for="wl_clientIsolation_disable" class="gnset_setting_content"><#btn_disable#></label>
				</div>
				<div class="gnset_setting_content_bg">
					<input type="radio" name="wl_clientIsolation" id="wl_clientIsolation_enable" value="enable">
					<label for="wl_clientIsolation_enable" class="gnset_setting_content"><#btn_Enabled#></label>
				</div>
			</div>
		</div>
		<div id="gnset_wl_captive_portal" class="gnset_setting_item_bg">
			<div class='gnset_setting_item_titleName'>
				<#Access_Control_title#>
			</div>
			<div class='gnset_setting_item_content'>
				<div id="gnset_captive_portal_profile" class="gnset_setting_content"></div>
			</div>
		</div>
		<div id="gnset_wl_macmode" class="gnset_setting_item_bg">
			<div class='gnset_setting_item_titleName'>
				<#enable_macmode#>
			</div>
			<div class='gnset_setting_item_content'>
				<select name="wl_macmode" class="gnset_setting_input_text_autoWidth">
					<option class="content_input_fd" value="disabled" <% nvram_match("wl_macmode", "disabled","selected"); %>><#btn_disable#></option>
					<option class="content_input_fd" value="allow" <% nvram_match("wl_macmode", "allow","selected"); %>><#FirewallConfig_MFMethod_item1#></option>
					<option class="content_input_fd" value="deny" <% nvram_match("wl_macmode", "deny","selected"); %>><#FirewallConfig_MFMethod_item2#></option>
				</select>
				<script>
				document.form.wl_macmode.onchange = function(){
					document.getElementById("maclistMain").style.display = (this.value == "disabled") ? "none" : "";
				}
				</script>
			</div>
		</div>
		<div id="maclistMain" style="padding-left:15px;padding-right:15px;">
			<table id="maclistTable" border="1" align="center" cellpadding="4" cellspacing="0" class="FormTable_table" style="width:100%;">
				<thead>
					<tr>
						<td colspan="2"><#FirewallConfig_MFList_groupitemname#>&nbsp;(<#List_limit#>&nbsp;64)</td>
					</tr>
				</thead>
					<tr>
						<th width="80%"><a class="hintstyle" href="javascript:void(0);" onClick="openHint(5,10);">Client's MAC address<!--untranslated--></th> 
						<th width="20%"><#list_add_delete#></th>
					</tr>
					<tr>
						<td width="80%">
							<input type="text" maxlength="17" class="input_macaddr_table" name="wl_maclist_x_0" onKeyPress="return validator.isHWAddr(this,event)" onClick="hideClients_Block();" autocorrect="off" autocapitalize="off" placeholder="ex: <% nvram_get("lan_hwaddr"); %>" style="width:255px;">
							<img id="pull_arrow" height="14px;" src="/images/arrow-down.gif" style="position:absolute;display:none;" onclick="pullWLMACList(this);" title="<#select_wireless_MAC#>" onmouseover="over_var=1;" onmouseout="over_var=0;">
							<div id="WL_MAC_List_Block" class="WL_MAC_Block"></div>
						</td>
						<td width="20%">
							<input type="button" class="add_btn" onClick="addRow(document.form.wl_maclist_x_0, 64);" value="">
						</td>
					</tr>
			</table>
			<div id="wl_maclist_x_Block"></div>
		</div>

		<!--unuse item-->
		<div id="unused_item" style="display:none;">
			<table width="80%" border="1" align="center" style="margin-top:10px;margin-bottom:20px;display:none" cellpadding="4" cellspacing="0" id="gnset_table" class="FormTable">
				<tr style="display:none">
					<td>
						<span><span><input type="hidden" name="wl_wpa_gtk_rekey" value="<% nvram_get("wl_wpa_gtk_rekey"); %>" disabled></span></span>
					</td>
				</tr>
				<tr id="wl_unit_field" style="display:none">
					<th><#Guest_Network_enable#></th>
					<td>
						<select id="wl_bss_enabled_field" name="wl_bss_enabled" class="input_option">
							<option class="content_input_fd" value="0" <% nvram_match("wl_bss_enabled", "0","selected"); %>><#checkbox_No#></option>
							<option class="content_input_fd" value="1" <% nvram_match("wl_bss_enabled", "1","selected"); %>><#checkbox_Yes#></option>
						</select>
					</td>
				</tr>
				<tr style="display:none">
					<th><a class="hintstyle" href="javascript:void(0);" onClick="openHint(0, 4);"><#WLANConfig11b_x_Mode_itemname#></a></th>
					<td>
						<select name="wl_nmode_x" class="input_option" onChange="wireless_mode_change(this);" disabled>
							<option value="0" <% nvram_match("wl_nmode_x", "0","selected"); %>><#Auto#></option>
							<option value="1" <% nvram_match("wl_nmode_x", "1","selected"); %>>N Only</option>
							<option value="2" <% nvram_match("wl_nmode_x", "2","selected"); %>>Legacy</option>
						</select>
						<input type="checkbox" name="wl_gmode_check" id="wl_gmode_check" value="" onClick="wl_gmode_protection_check();"> b/g Protection</input>
					</td>
				</tr>
				<tr>
					<th><a id="wl_channel_select" class="hintstyle" href="javascript:void(0);" onClick="openHint(0, 3);"><#WLANConfig11b_Channel_itemname#></a></th>
					<td>
						<select name="wl_channel" class="input_option" onChange="insertExtChannelOption();" disabled>
							<% select_channel("WLANConfig11b"); %>
						</select>
					</td>
				</tr>
				<tr style="display:none;">
					<th><a class="hintstyle" href="javascript:void(0);" onClick="openHint(0, 14);"><#WLANConfig11b_ChannelBW_itemname#></a></th>
					<td>
						<select name="wl_bw" class="input_option" onChange="insertExtChannelOption();" disabled>
							<option class="content_input_fd" value="0" <% nvram_match("wl_bw", "0","selected"); %>>20 MHz</option>
							<option class="content_input_fd" value="1" <% nvram_match("wl_bw", "1","selected"); %>>20/40 MHz</option>
							<option class="content_input_fd" value="2" <% nvram_match("wl_bw", "2","selected"); %>>40 MHz</option>
						</select>
					</td>
				</tr>
				<tr>
					<th><a class="hintstyle" href="javascript:void(0);" onClick="openHint(0, 15);"><#WLANConfig11b_EChannel_itemname#></a></th>
					<td>
						<select name="wl_nctrlsb" class="input_option" disabled>
							<option value="lower" <% nvram_match("wl_nctrlsb", "lower", "selected"); %>>lower</option>
							<option value="upper"<% nvram_match("wl_nctrlsb", "upper", "selected"); %>>upper</option>
						</select>
					</td>
				</tr>
			</table>
		</div>
	</div>
	<div class="gnset_setting_apply_bg">
		<input type="button" class="button_gen" value="<#CTL_Cancel#>" onclick="gnset_cancel();">
		<input type="button" class="button_gen" value="<#CTL_apply#>" onclick="applyRule();">
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
			<div id="tabMenu" class="submenuBlock" style="*margin-top:-155px;"></div>

<!--===================================Beginning of Main Content===========================================-->
			<table width="98%" border="0" align="left" cellpadding="0" cellspacing="0">
				<tr>
					<td align="left" valign="top" >
						<table width="760px" border="0" cellpadding="4" cellspacing="0" class="FormTitle" style="-webkit-border-radius:3px;-moz-border-radius:3px;border-radius:3px;" id="FormTitle">
							<tbody>
							<tr>
								<td bgcolor="#4D595D" valign="top" id="table_height"  >
									<div>&nbsp;</div>
									<div class="formfonttitle"><#Guest_Network#></div>
									<div class='guest_line' style="margin-left:5px;margin-right:5px;"></div>
									<div style="height:130px;">
										<div class='guest_page_intro_icon'></div>
										<div>
											<div id="gn_desc" class="guest_page_intro_txt" ><#GuestNetwork_desc#></div>
											<div style="float:left">
												<span id="2g_radio_hint" style="font-size: 14px;display:none;color:#FC0;margin-left:25px;">* <#GuestNetwork_Radio_Status#>
													<a style="font-family:Lucida Console;color:#FC0;text-decoration:underline;cursor:pointer;" onclick="_change_wl_unit_status(0);"><#btn_go#></a>
													2.4 GHZ
													<br>
												</span>
												<span id="5g_radio_hint" style="font-size: 14px;display:none;color:#FC0;margin-left:25px;">* <#GuestNetwork_Radio_Status#>
													<a style="font-family:Lucida Console;color:#FC0;text-decoration:underline;cursor:pointer;" onclick="_change_wl_unit_status(1);"><#btn_go#></a>
													5 GHZ
												</span>
											</div>
										</div>
									</div>			
								<!-- info table -->
									<div id="guest_table" style="margin-left:5px;margin-right:5px;"></div>		
									  	
								</td>
							</tr>
							</tbody>		
						</table>
					</td>
	</tr>
</table>
<!--===================================Ending of Main Content===========================================-->
		</td>	
		<td width="10" align="center" valign="top"></td>
	</tr>
</table>
</form>
<div id="footer"></div>

</body>
</html>
