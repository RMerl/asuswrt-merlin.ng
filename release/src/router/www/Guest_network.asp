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
<script type="text/javascript" src="/state.js"></script>
<script language="JavaScript" type="text/javascript" src="/js/jquery.js"></script>
<script language="JavaScript" type="text/javascript" src="/client_function.js"></script>
<script type="text/javascript" src="/help.js"></script>
<script type="text/javascript" src="/general.js"></script>
<script type="text/javascript" src="/popup.js"></script>
<script type="text/javascript" src="/md5.js"></script>
<script type="text/javascript" src="/validator.js"></script>
<style>
</style>
<script>
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

var captive_portal_used_wl_array = new Array();

var manually_maclist_list_array = new Array();
var all_gn_status = [];
Object.prototype.getKey = function(value) {
	for(var key in this) {
		if(this[key] == value) {
			return key;
		}
	}
	return null;
};

function initial(){
	show_menu();	

	if(Qcawifi_support){
		radio_2 = '<% nvram_get("wl0_radio"); %>';
		radio_5 = '<% nvram_get("wl1_radio"); %>';
	}
	
	//insertExtChannelOption();		
	if(downsize_4m_support || downsize_8m_support)
		document.getElementById("guest_image").parentNode.parentNode.removeChild(document.getElementById("guest_image").parentNode);

	mbss_display_ctrl();
	gen_gntable();
	guest_divctrl(0);
	if(document.form.wl_gmode_protection.value == "auto")
		document.form.wl_gmode_check.checked = true;
	else
		document.form.wl_gmode_check.checked = false;

	if(!band5g_support || no5gmssid_support)
		document.getElementById("guest_table5").style.display = "none";
	
	if(wl_info.band5g_2_support){
		document.getElementById("wl_opt1").innerHTML = "5GHz-1";
		document.getElementById("wl_opt2").style.display = "";
		document.getElementById("guest_table5_2").style.display = "";
	}

	if(wl_info.band60g_support) {
		document.getElementById("guest_table60").style.display = "";
	}

	if(radio_2 != 1){
		document.getElementById('2g_radio_hint').style.display ="";
	}
	if(radio_5 != 1){
		document.getElementById('5g_radio_hint').style.display ="";
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

	if(!fbwifi_support && !cp_freewifi_support && !cp_advanced_support){
		document.getElementById('FormTitle').style.webkitBorderRadius = "3px";
		document.getElementById('FormTitle').style.MozBorderRadius = "3px";
		document.getElementById('FormTitle').style.BorderRadius = "3px";        
	}

	if(!fbwifi_support) {
		document.getElementById("guest_tableFBWiFi").style.display = "none";
	}

	if(lyra_hide_support)
		document.getElementById("gn_index_tr").style.display = "none";

	if(ifttt_support || alexa_support){
		$("#smart_home_0").show();
		if(band5g_support)
			$("#smart_home_1").show();
		if(wl_info.band5g_2_support)
			$("#smart_home_2").show();
	}

	//When redirect page from Free WiFi or Captive Portal, auto go to anchor tag
	var gn_idx = cookie.get("captive_portal_gn_idx");
	if(gn_idx != "" && gn_idx != null) {
		var gn_idx_array = gn_idx.split(">");
		if(gn_idx_array.length == 2) {
			var gn_num = gn_idx_array[0];
			var type_hint = "";
			switch(gn_idx_array[1]) {
				case "captivePortal" :
					type_hint = "Captive Portal";
					break;
				case "freeWiFi" :
					type_hint = "Free W-Fi";
					break;
			}
			if(based_modelid == "BRT-AC828")
				window.location.hash = "guest_block_anchor_" + gn_num;
			else
				window.location.hash = "guest_block_anchor";
			setTimeout(function(){
				alert("Guest Network – " + gn_num + " will used by " + type_hint);
			}, 100);
		}
		cookie.unset("captive_portal_gn_idx");
	}
}

function change_wl_expire_radio(){
	load_expire_selection(document.form.wl_expire_day, option_expire_day, optval_expire_day);	
	
	if(document.form.wl_expire.value > 0){
		document.form.wl_expire_day.value = Math.floor(document.form.wl_expire.value/86400);
		document.form.wl_expire_hr.value = Math.floor((document.form.wl_expire.value%86400)/3600);
		document.form.wl_expire_min.value  = Math.floor((document.form.wl_expire.value%3600)/60);
		document.form.wl_expire_radio[0].checked = 1;
		document.form.wl_expire_radio[1].checked = 0;
	}
	else{	
		document.form.wl_expire_hr.value = "";
		document.form.wl_expire_min.value = "";	
		document.form.wl_expire_radio[0].checked = 0;
		document.form.wl_expire_radio[1].checked = 1;
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
	else if(flag == "sae"){
		return "WPA3-Personal";
	}	 
	else if(flag == "pskpsk2")
		return "WPA-Auto-Personal";
	else if(flag == "psk2sae"){
		return "WPA2/WPA3-Personal";	
	}	
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

function gen_gntable_tr(unit, gn_array, slicesb){	
	var GN_band = "";
	var htmlcode = "";
	if(unit == 0) 
		GN_band = 2;
	else
		GN_band = 5;
	
	htmlcode += '<table align="left" style="margin-left:-10px;border-collapse:collapse;width:720px;';
	if(slicesb > 0)
		htmlcode += 'margin-top:20px;';	
	htmlcode += '"><tr><th align="left" width="160px">';
	htmlcode += '<table id="GNW_'+GN_band+'G" class="gninfo_th_table" align="left" style="margin:auto;border-collapse:collapse;">';
	htmlcode += '<tr><th align="left" style="height:40px;"><#QIS_finish_wireless_item1#></th></tr>';
	if(!lyra_hide_support)
		htmlcode += "<tr><th align=\"left\" style=\"height:40px;\"><#WLANConfig11b_AuthenticationMethod_itemname#></th></tr>";
	htmlcode += '<tr><th align="left" style="height:40px;"><#Network_key#></th></tr>';
	htmlcode += '<tr><th align="left" style="height:40px;"><#mssid_time_remaining#></th></tr>';
	if(sw_mode != "3"){
			htmlcode += '<tr><th align="left" style="width:20%;height:28px;"><#Access_Intranet#></th></tr>';
	}
	htmlcode += '<tr><th align="left" style="height:40px;"></th></tr>';		
	htmlcode += '</table></th>';
	
	if(tmo_support)	//keep wlx.3 for usingg Passpoint
		var gn_array_length = gn_array.length-1;
	else	
		var gn_array_length = gn_array.length;
	for(var i=0; i<gn_array_length; i++){			
			var subunit = i+1+slicesb*4;
			var show_str;
			var unit_subunit = unit + "." + subunit;

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

			var control_setting_flag = false;
			if(captive_portal_used_wl_array["wl" + unit_subunit] == undefined || captive_portal_used_wl_array["wl" + unit_subunit] != "Facebook Wi-Fi")
				control_setting_flag = true;

			htmlcode += '<td><table id="GNW_'+GN_band+'G'+i+'" class="gninfo_table" align="center" style="margin:auto;border-collapse:collapse;">';			
			if(gn_array[i][0] == "1"){
				if(control_setting_flag) {
					htmlcode += '<tr><td align="center" class="gninfo_table_top"></td></tr>';
					show_str = decodeURIComponent(gn_array[i][1]);
					if(show_str.length >= 21)
						show_str = show_str.substring(0,17) + "...";
					show_str = handle_show_str(show_str);
					htmlcode += '<tr><td align="center" onclick="change_guest_unit('+ unit +','+ subunit +');">'+ show_str +'</td></tr>';
					if(!lyra_hide_support)
						htmlcode += '<tr><td align="center" onclick="change_guest_unit('+ unit +','+ subunit +');">'+ translate_auth(gn_array[i][2]) +'</td></tr>';
					
					if(gn_array[i][2].indexOf("wpa") >= 0 || gn_array[i][2].indexOf("radius") >= 0)
							show_str = "";
					else if(gn_array[i][2].indexOf("psk") >= 0 || gn_array[i][2].indexOf("sae") >= 0)
							show_str = gn_array[i][4];
					else if(gn_array[i][2] == "open" && gn_array[i][5] == "0")
							show_str = "None";
					else{
							var key_index = parseInt(gn_array[i][6])+6;
							show_str = gn_array[i][key_index];
					}

					show_str = decodeURIComponent(show_str);
					if(show_str.length >= 21)
						show_str = show_str.substring(0,17) + "...";
					show_str = handle_show_str(show_str);
					if(show_str.length <= 0)
						show_str = "&nbsp; ";
					htmlcode += '<tr><td align="center" onclick="change_guest_unit('+ unit +','+ subunit +');">'+ show_str +'</td></tr>';
					
					if(gn_array[i][11] == 0)
							htmlcode += '<tr><td align="center" onclick="change_guest_unit('+ unit +','+ subunit +');"><#Limitless#></td></tr>';
					else{
							var expire_day = Math.floor(gn_array[i][13]/86400);
							var expire_hr = Math.floor((gn_array[i][13]%86400)/3600);
							var expire_min = Math.floor((gn_array[i][13]%3600)/60);
							if(expire_day > 0)
									htmlcode += '<tr><td align="center" onclick="change_guest_unit('+ unit +','+ subunit +');"><b id="expire_day_'+i+'">'+ expire_day + '</b> <#Day#> <b id="expire_hr_'+i+'">'+ expire_hr + '</b> <#Hour#> <b id="expire_min_'+i+'">' + expire_min +'</b> <#Minute#></td></tr>';
							else if(expire_hr > 0)
									htmlcode += '<tr><td align="center" onclick="change_guest_unit('+ unit +','+ subunit +');"><b id="expire_hr_'+i+'">'+ expire_hr + '</b> <#Hour#> <b id="expire_min_'+i+'">' + expire_min +'</b> <#Minute#></td></tr>';
							else{
									if(expire_min > 0)
											htmlcode += '<tr><td align="center" onclick="change_guest_unit('+ unit +','+ subunit +');"><b id="expire_min_'+i+'">' + expire_min +'</b> <#Minute#></td></tr>';
									else
											htmlcode += '<tr><td align="center" onclick="change_guest_unit('+ unit +','+ subunit +');"><b id="expire_min_'+i+'">< 1</b> <#Minute#></td></tr>';
							}				
					}
				}
				else {
					htmlcode += '<tfoot><tr rowspan="3"><td align="center"><span style="color:#FFCC00;">Used by ' + captive_portal_used_wl_array["wl" + unit_subunit] + '</span></td></tr></tfoot>';
				}			
			}else{					
					htmlcode += '<tfoot><tr rowspan="3"><td align="center"><input type="button" class="button_gen" value="<#WLANConfig11b_WirelessCtrl_button1name#>" onclick="create_guest_unit('+ unit +','+ subunit +');"></td></tr></tfoot>';
			}														
			
			if(sw_mode != "3"){
					if(gn_array[i][0] == "1" && control_setting_flag){
						var status_Access_Intranet = (gn_array[i][12]=="on")?"<#WLANConfig11b_WirelessCtrl_button1name#>":"<#btn_disable#>";
						htmlcode += '<tr><td align="center" onclick="change_guest_unit('+ unit +','+ subunit +');">'+ status_Access_Intranet +'</td></tr>';
					}
			}
										
			if(gn_array[i][0] == "1" && control_setting_flag){
				if(captive_portal_used_wl_array["wl" + unit_subunit] == undefined) {
					htmlcode += '<tr><td align="center" class="gninfo_table_bottom"></td></tr>';
					htmlcode += '<tfoot><tr><td align="center"><input type="button" class="button_gen" value="<#btn_remove#>" onclick="close_guest_unit('+ unit +','+ subunit +');"></td></tr></tfoot>';
				}
				else {
					if(captive_portal_used_wl_array["wl" + unit_subunit] != "Facebook Wi-Fi")
						htmlcode += '<tfoot><tr rowspan="3"><td align="center"><span style="color:#FFCC00;">Used by ' + captive_portal_used_wl_array["wl" + unit_subunit] + '</span></td></tr></tfoot>';
				}
			}

			if(i == (gn_array_length-1)){
				htmlcode += '<tfoot><tr><td align="center"><div id="smart_home_'+unit+'" style="font-size: 12px;font-weight:bolder;color:rgb(255, 204, 0);position:absolute;margin:33px 0px 0px -20px;display:none"><#Guest_Network_AlexaIFTTT_setting#></div></td></tr></tfoot>';
			}
			htmlcode += '</table></td>';		

			all_gn_status.push({"idx" : unit_subunit, "enable" : (gn_array[i][0] == '1'), "bw_enabled" : (gn_array[i][18] == '1')});
	}	

	if(slicesb > 0){
		for(var td=0; td<(4-gn_array_length); td++)
			htmlcode += '<td style="width:135px"></td>';
	}			

	htmlcode += '</tr></table>';
	return htmlcode;
}

function _change_wl_unit_status(__unit){
	document.titleForm.current_page.value = "Advanced_WAdvanced_Content.asp?af=wl_radio";
	document.titleForm.next_page.value = "Advanced_WAdvanced_Content.asp?af=wl_radio";
	change_wl_unit_status(__unit);
}

function gen_gntable(){
	var htmlcode = ""; 
	var htmlcode5 = ""; 
	var htmlcode5_2 = ""; 
	var htmlcode60 = "";
	var gn_array_2g_tmp = gn_array_2g;
	var gn_array_5g_tmp = gn_array_5g;
	var gn_array_5g_2_tmp = gn_array_5g_2;
	var gn_array_60g_tmp = gn_array_60g;
	var band2sb = 0;
	var band5sb = 0;
	var band5sb_2 = 0;
	var band60sb = 0;

	if(gn_array_2g_tmp.length > 0){
		htmlcode += '<table style="margin-left:20px;margin-top:25px;" width="95%" align="center" cellpadding="4" cellspacing="0" class="gninfo_head_table" id="gninfo_table_2g">';
		htmlcode += '<tr id="2g_title"><td align="left" style="color:#5AD;font-size:16px; border-bottom:1px dashed #AAA;"><span>2.4GHz</span>';
		htmlcode += '<span id="2g_radio_hint" style="font-size: 14px;display:none;color:#FC0;margin-left:17px;">* <#GuestNetwork_Radio_Status#>	<a style="font-family:Lucida Console;color:#FC0;text-decoration:underline;cursor:pointer;" onclick="_change_wl_unit_status(0);"><#btn_go#></a></span></td></tr>';
		while(gn_array_2g_tmp.length > 4){
			htmlcode += '<tr><td>';
			htmlcode += gen_gntable_tr(0, gn_array_2g_tmp.slice(0, 4), band2sb);
			band2sb++;
			gn_array_2g_tmp = gn_array_2g_tmp.slice(4);
			htmlcode += '</td></tr>';
		}

		htmlcode += '<tr><td>';
		htmlcode += gen_gntable_tr(0, gn_array_2g_tmp, band2sb);
		htmlcode += '</td></tr>';
		htmlcode += '</table>';
		document.getElementById("guest_table2").innerHTML = htmlcode;
	}
	
	if(gn_array_5g_tmp.length > 0){
		htmlcode5 += '<table style="margin-left:20px;margin-top:25px;" width="95%" align="center" cellpadding="4" cellspacing="0" class="gninfo_head_table" id="gninfo_table_5g">';
		htmlcode5 += '<tr id="5g_title"><td align="left" style="color:#5AD; font-size:16px; border-bottom:1px dashed #AAA;">';
		if(wl_info.band5g_2_support)
			htmlcode5 += '<span>5GHz-1</span>';
		else
			htmlcode5 += '<span>5GHz</span>';
		htmlcode5 += '<span id="5g_radio_hint" style="font-size: 14px;display:none;color:#FC0;margin-left:17px;">* <#GuestNetwork_Radio_Status#>	<a style="font-family:Lucida Console;color:#FC0;text-decoration:underline;cursor:pointer;" onclick="_change_wl_unit_status(1);"><#btn_go#></a></span></td></tr>';

		while(gn_array_5g_tmp.length > 4){
			htmlcode5 += '<tr><td >';
			htmlcode5 += gen_gntable_tr(1, gn_array_5g_tmp.slice(0, 4), band5sb);
			band5sb++;
			gn_array_5g_tmp = gn_array_5g_tmp.slice(4);
			htmlcode5 += '</td></tr>';
		}

		htmlcode5 += '<tr><td>';
		htmlcode5 += gen_gntable_tr(1, gn_array_5g_tmp, band5sb);
		htmlcode5 += '</td></tr>';
		htmlcode5 += '</table>';
		document.getElementById("guest_table5").innerHTML = htmlcode5;
	}

  	if(wl_info.band5g_2_support && gn_array_5g_2_tmp.length > 0){
		htmlcode5_2 += '<table style="margin-left:20px;margin-top:25px;" width="95%" align="center" cellpadding="4" cellspacing="0" class="gninfo_head_table" id="gninfo_table_5g_2">';
		htmlcode5_2 += '<tr id="5g_2_title"><td align="left" style="color:#5AD; font-size:16px; border-bottom:1px dashed #AAA;"><span>5GHz-2</span>';
		htmlcode5_2 += '<span id="5g_2_radio_hint" style="font-size: 14px;display:none;color:#FC0;margin-left:17px;">* <#GuestNetwork_Radio_Status#>	<a style="font-family:Lucida Console;color:#FC0;text-decoration:underline;cursor:pointer;" onclick="_change_wl_unit_status(1);"><#btn_go#></a></span></td></tr>';
		while(gn_array_5g_2_tmp.length > 4){
			htmlcode5_2 += '<tr><td >';
			htmlcode5_2 += gen_gntable_tr(2, gn_array_5g_2_tmp.slice(0, 4), band5sb_2);
			band5sb++;
			gn_array_5g_2_tmp = gn_array_5g_2_tmp.slice(4);
			htmlcode5_2 += '</td></tr>';
		}

		htmlcode5_2 += '<tr><td>';
		htmlcode5_2 += gen_gntable_tr(2, gn_array_5g_2_tmp, band5sb_2);
		htmlcode5_2 += '</td></tr>';
		htmlcode5_2 += '</table>';
		document.getElementById("guest_table5_2").innerHTML = htmlcode5_2;
	}

	if(wl_info.band60g_support) {
		htmlcode60 += '<table style="margin-left:20px;margin-top:25px;" width="95%" align="center" cellpadding="4" cellspacing="0" class="gninfo_head_table" id="gninfo_table_60g">';
		htmlcode60 += '<tr id="60g_title"><td align="left" style="color:#5AD; font-size:16px; border-bottom:1px dashed #AAA;"><span>60GHz</span></td></tr>';
		htmlcode60 += '<tr><td><span style="font-size: 14px;color:#FC0;"><#CTL_nonsupported#></span></td></tr>';
		htmlcode60 += '</table>';
		document.getElementById("guest_table60").innerHTML = htmlcode60;
	}
}

function add_options_value(o, arr, orig){
	if(orig == arr)
		add_option(o, "mbss_"+arr, arr, 1);
	else
		add_option(o, "mbss_"+arr, arr, 0);
}

function applyRule(){
	var auth_mode = document.form.wl_auth_mode_x.value;
	if(document.form.wl_wpa_psk.value == "<#wireless_psk_fillin#>")
		document.form.wl_wpa_psk.value = "";

	if(validForm()){		
		updateMacList();		
		if(document.form.wl_expire_radio[0].checked)
			document.form.wl_expire.value = document.form.wl_expire_day.value*86400 + document.form.wl_expire_hr.value*3600 + document.form.wl_expire_min.value*60;
		else
			document.form.wl_expire.value = 0;
		
			
		if(based_modelid == "RT-AC87U") //MODELDEP: RT-AC87U need to extend waiting time to get new wl value
			document.form.action_wait.value = parseInt(document.form.action_wait.value)+5;
		
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
			if(ctf_disable_orig == '0' && !lantiq_support){	//brcm NAT Acceleration turned ON
				document.form.action_script.value = "saveNvram;reboot";
				document.form.action_wait.value = "<% get_default_reboot_time(); %>";
			}
			else{
				document.form.action_script.value = "restart_wireless;restart_qos;restart_firewall;";
			}
		}
		else if(unit_bw_enabled != document.form.wl_bw_enabled.value //bandwidth limiter settings changed OR re-enable mSSID with bandwidth limiter
				|| unit_bw_ul != document.form.wl_bw_ul.value 
				|| unit_bw_dl != document.form.wl_bw_dl.value
				|| (wl_x_y_bss_enabled == 1 && (document.form.bw_enabled_x.value == "1" || document.form.bw_enabled_x[0].checked)))	
		{	
			if(ctf_disable_orig == '0' && document.form.wl_bw_enabled.value == 1 && !lantiq_support){
				document.form.action_script.value = "saveNvram;reboot";
				document.form.action_wait.value = "<% get_default_reboot_time(); %>";
			}
			else{
				document.form.action_script.value = "restart_wireless;restart_qos;restart_firewall;";
			}
		}

		dis_qos_enable(document.form.wl_unit.value + "." + document.form.wl_subunit.value, document.form, "bw_enabled");

		var _unit_subunit = "wl" + document.form.wl_unit.value + "." + document.form.wl_subunit.value;
		if(captive_portal_used_wl_array[_unit_subunit] != undefined) {
			document.form.wl_key.disabled = true;
			document.form.wl_key1.disabled = true;
			document.form.wl_key2.disabled = true;
			document.form.wl_key3.disabled = true;
			document.form.wl_key4.disabled = true;
			document.form.wl_wep_x.disabled = true;
			document.form.wl_phrase_x.disabled = true;
			document.form.wl_lanaccess.disabled = true;
			document.form.wl_expire.disabled = true;
			document.form.wl_bw_enabled.disabled = true;
			document.form.wl_bw_dl.disabled = true;
			document.form.wl_bw_ul.disabled = true;	
			switch(captive_portal_used_wl_array[_unit_subunit]) {
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
	if(auth_mode == "psk" || auth_mode == "psk2" || auth_mode == "sae" || auth_mode == "pskpsk2" || auth_mode == "psk2sae"){ //2008.08.04 lock modified
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
	if(document.form.wl_expire_radio[0].checked){		
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

function disableAdvFn(){
	for(var i=16; i>=1; i--)
		document.getElementById("WLgeneral").deleteRow(i);
}

function guest_divctrl(flag){	
	if(flag == 1){
		document.getElementById("guest_table2").style.display = "none";
		if(band5g_support)
			document.getElementById("guest_table5").style.display = "none";
		
		if(wl_info.band5g_2_support)
			document.getElementById("guest_table5_2").style.display = "none";

		if(wl_info.band60g_support)
			document.getElementById("guest_table60").style.display = "none";

		if(fbwifi_support) {
			document.getElementById("guest_tableFBWiFi").style.display = "none";
		}

		document.getElementById("gnset_table").style.display = "";

		
		document.getElementById("applyButton").style.display = "";
	}
	else{
		document.getElementById("guest_table2").style.display = "";
		if(!band5g_support || no5gmssid_support){
			document.getElementById("guest_table5").style.display = "none";
			if(!wl_info.band5g_2_support)
				document.getElementById("guest_table5_2").style.display = "none";
		}
		else{
			document.getElementById("guest_table5").style.display = "";
		}		
		
		if(wl_info.band5g_2_support)
			document.getElementById("guest_table5_2").style.display = "";

		if(wl_info.band60g_support)
			document.getElementById("guest_table60").style.display = "";
		else
			document.getElementById("guest_table60").style.display = "none";

		if(!fbwifi_support) {
			document.getElementById("guest_tableFBWiFi").style.display = "none";
		}
		else {
			document.getElementById("guest_tableFBWiFi").style.display = "";
		}
		
		document.getElementById("gnset_table").style.display = "none";
		document.getElementById("applyButton").style.display = "none";
		document.getElementById("maclistMain").style.display = "none";
	}
}

function mbss_display_ctrl(){
	// generate options
	if(multissid_support){
		document.getElementById("wl_channel_field").style.display = "none";
		document.getElementById("wl_nctrlsb_field").style.display = "none";
		for(var i=1; i<multissid_count+1; i++)
			add_options_value(document.form.wl_subunit, i, '<% nvram_get("wl_subunit"); %>');
	}
	else{
		document.getElementById("gnset_table").style.display = "none";
		document.getElementById("guest_table2").style.display = "none";
		if(band5g_support)
			document.getElementById("guest_table5").style.display = "none";
		if(wl_info.band5g_2_support)
			document.getElementById("guest_table5_2").style.display = "none";
		if(wl_info.band60g_support)
			document.getElementById("guest_table60").style.display = "none";
		if(fbwifi_support) {
			document.getElementById("guest_tableFBWiFi").style.display = "none";
		}
		
		document.getElementById("applyButton").style.display = "none";
		document.getElementById("applyButton").innerHTML = "Not support!";
		document.getElementById("applyButton").style.fontSize = "25px";
		document.getElementById("applyButton").style.marginTop = "125px";
	}
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

	if(_setting == "0")
		dis_qos_enable(_unit + "." + _subunit, document.unitform, "enable");

	document.unitform.submit();
}

function close_guest_unit(_unit, _subunit){
	en_dis_guest_unit(_unit, _subunit, "0");
}

var unit_bw_enabled = "";
var unit_bw_ul = "";
var unit_bw_dl = "";
var edit_unit = "";
function change_guest_unit(_unit, _subunit){
	var idx;
	switch(_unit){
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
		case 3:
			edit_unit=3;
			gn_array = gn_array_60g;
			document.form.wl_nmode_x.value = wl3_nmode_x;
			break;
	}
	
	idx = _subunit - 1;
	if(gn_array[idx][22] == "1"){
		document.form.wl_closed[0].checked=true;
	}
	else{
		document.form.wl_closed[1].checked=true;
	}

	limit_auth_method(_unit);
	document.form.wl_unit.value = _unit;
	document.form.wl_subunit.value = _subunit;
	document.getElementById("wl_vifname").innerHTML = document.form.wl_subunit.value;
	document.form.wl_bss_enabled.value = decodeURIComponent(gn_array[idx][0]);
	document.form.wl_ssid.value = decodeURIComponent(gn_array[idx][1]);
	wl_x_y_bss_enabled = 1;
	
	if(lyra_hide_support)
		document.form.wl_auth_mode_x.value = "psk2";
	else
		document.form.wl_auth_mode_x.value = decodeURIComponent(gn_array[idx][2]);
	wl_auth_mode_change(1);
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

	wl_wep_change();
	change_wl_expire_radio();
	show_bandwidth(unit_bw_enabled);
	guest_divctrl(1);

	updateMacModeOption();

	if(captive_portal_used_wl_array["wl" + _unit + "." + _subunit] == "Free Wi-Fi" || captive_portal_used_wl_array["wl" + _unit + "." + _subunit] == "Captive Portal Wi-Fi") {
		$(".captive_portal_control_class").css("display", "none");
	}
	else {
		$(".captive_portal_control_class").css("display", "");
		if(isSwMode("ap")){
			inputCtrl(document.form.wl_lanaccess, 0);
			document.getElementById('bw_limiter_field').style.display = "none";
			document.getElementById('bw_limiter_settings').style.display = "none";
		}
	}

	if(lyra_hide_support){
		document.form.wl_crypto.value = "aes";
		document.getElementById("wl_auth_mode_tr").style.display = "none";
		document.getElementById("wl_crypt_tr").style.display = "none";
		document.getElementById("psk_title").innerHTML = "<#Network_key#>";
		inputCtrl(document.form.wl_macmode, 0);
		inputCtrl(document.form.wl_lanaccess, 1);
	}
}

function create_guest_unit(_unit, _subunit){
	switch(_unit){
		case 0:
			gn_array = gn_array_2g;
			break;
		case 1:
			gn_array = gn_array_5g;
			break;
		case 2:
			gn_array = gn_array_5g_2;
			break;						
		case 3:
			gn_array = gn_array_60g;
			break;
	}
	
	if(gn_array[_subunit-1][15] != "1"){
		change_guest_unit(_unit, _subunit);
		document.form.wl_bss_enabled.value = "1";
	}else{
		en_dis_guest_unit(_unit, _subunit, "1");
	}
}

function genBWTable(_unit){
	cur = '<% nvram_get("wl_bw"); %>';
	if(document.form.wl_nmode_x.value == 2){
		var bws = new Array("1");
		var bwsDesc = new Array("20 MHz");
	}
	else if(_unit == 0){
		var bws = new Array(0, 1, 2);
		var bwsDesc = new Array("20/40 MHz", "20 MHz", "40 MHz");
	}
	else{
		var bws = new Array(0, 1, 2, 3);
		var bwsDesc = new Array("20/40/80 MHz", "20 MHz", "40 MHz", "80 MHz");
	}

	document.form.wl_bw.length = bws.length;
	for (var i in bws) {
		document.form.wl_bw[i] = new Option(bwsDesc[i], bws[i]);
		document.form.wl_bw[i].value = bws[i];
		if (bws[i] == cur) {
			document.form.wl_bw[i].selected = true;
		}
	}
}

// mac filter
function updateMacModeOption(){
	wl_maclist_x_array = gn_array[document.form.wl_subunit.value-1][16];
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

	document.form.wl_macmode.value = gn_array[document.form.wl_subunit.value-1][14];
	document.form.wl_maclist_x.value = gn_array[document.form.wl_subunit.value-1][16];
	document.getElementById("maclistMain").style.display = (document.form.wl_macmode.value == "disabled") ? "none" : "";
}

function show_wl_maclist_x(){
	var code = "";
	var clientListEventData = [];
	code +='<table width="80%" border="1" cellspacing="0" cellpadding="4" align="center" class="list_table"  id="wl_maclist_x_table">'; 
	if(Object.keys(manually_maclist_list_array).length == 0)
		code +='<tr><td style="color:#FFCC00;"><#IPConnection_VSList_Norule#></td></tr>';
	else{
		//user icon
		var userIconBase64 = "NoIcon";
		var clientName, deviceType, deviceVender;
		Object.keys(manually_maclist_list_array).forEach(function(key) {
			var clientMac = key.toUpperCase();
			var clientIconID = "clientIcon_" + clientMac.replace(/\:/g, "");
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
			code += '<tr id="row_'+clientMac+'">';
			code += '<td width="80%" align="center">';
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
			code += '<td width="20%"><input type="button" class=\"remove_btn\" onclick=\"deleteRow(this, \'' + clientMac + '\');\" value=\"\"/></td></tr>';
			if(validator.mac_addr(clientMac))
				clientListEventData.push({"mac" : clientMac, "name" : "", "ip" : "", "callBack" : "GuestNetwork"});
		});
	}	
	
	code += '</table>';
	document.getElementById("wl_maclist_x_Block").innerHTML = code;
	for(var i = 0; i < clientListEventData.length; i += 1) {
		var clientIconID = "clientIcon_" + clientListEventData[i].mac.replace(/\:/g, "");
		var clientIconObj = $("#wl_maclist_x_Block").children("#wl_maclist_x_table").find("#" + clientIconID + "")[0];
		var paramData = JSON.parse(JSON.stringify(clientListEventData[i]));
		paramData["obj"] = clientIconObj;
		$("#wl_maclist_x_Block").children("#wl_maclist_x_table").find("#" + clientIconID + "").click(paramData, popClientListEditTable);
	}
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
	}else if(!check_macaddr(obj, check_hwaddr_flag(obj, 'inner'))){
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

function maclistMain_display(obj){	
	document.getElementById("maclistMain").style.display = (obj.value == "disabled") ? "none" : "";
}

function show_bandwidth(flag){	
	if(flag == "1"){
		document.form.bw_enabled_x[0].checked = true;
		var show_hint_content = "";
		if(ctf_disable_orig == '0'){	//brcm NAT Acceleration turned ON
			show_hint_content += "<br>NAT acceleration will be disable for more precise packet inspection.";	/* untranslated */			
		}

		if(QoS_enable_orig == "0"){
			show_hint_content += "<br>QoS function of traffic manager will be enable and set as Bandwidth Limiter mode by default.";	/* untranslated */
		}
		else if(QoS_type_orig != "2"){
			show_hint_content += "<br>QoS function of traffic manager will set as Bandwidth Limiter mode.";	/* untranslated */
		}

		if(show_hint_content.length <= 0){
			document.getElementById("QoS_hint").style.display = "none";
		}
		else{			
			document.getElementById("QoS_hint").innerHTML = show_hint_content;
			document.getElementById("QoS_hint").style.display = "";
		}	

		inputCtrl(document.form.wl_bw_dl_x, 1);
		inputCtrl(document.form.wl_bw_ul_x, 1);		
	}
	else{		
		document.form.bw_enabled_x[1].checked = true;
		document.getElementById("QoS_hint").style.display = "none";
		inputCtrl(document.form.wl_bw_dl_x, 0);
		inputCtrl(document.form.wl_bw_ul_x, 0);		
	}	
}

function dis_qos_enable(_wl_idx, _form_obj, _control_item){
	if(_wl_idx == "" || _wl_idx == undefined || _form_obj == "" || _form_obj == undefined || _control_item == "" || _control_item == undefined)
		return;

	if(!(all_gn_status.some(function(item, index, array){return (item.enable == true && item.bw_enabled == true)})))//if all gn bw disabled, not need disable qos
		return;

	var sw_mode_support = isSwMode("rt");
	var cp_wifi_not_used = (captive_portal_used_wl_array["wl" + _wl_idx] == undefined) ? true : false;
	if(sw_mode_support && cp_wifi_not_used){
		var specific_gn = all_gn_status.filter(function(item, index, array){
			return (item.idx == _wl_idx);
		})[0];

		if(_control_item == "enable")
			specific_gn.enable = false;
		else if(_control_item == "bw_enabled")
			specific_gn.bw_enabled = document.form.bw_enabled_x[0].checked;

		var all_gn_bw_dis = !(all_gn_status.some(function(item, index, array){return (item.enable == true && item.bw_enabled == true)}));
		var QoS_bw_rulelist_orig = '<% nvram_get("qos_bw_rulelist"); %>';
		var qos_can_dis = (QoS_enable_orig == "1" && QoS_type_orig == "2" && QoS_bw_rulelist_orig == "") ? true : false;
		if(all_gn_bw_dis && qos_can_dis){
			if(_form_obj.qos_enable == undefined){
				var qos_enable = document.createElement("input");
				qos_enable.type = "hidden";
				qos_enable.name = "qos_enable";
				qos_enable.value = "0";
				_form_obj.appendChild(qos_enable);
			}
			else
				_form_obj.qos_enable.value = "0";
		}
	}
}
</script>
</head>

<body onload="initial();" class="bg">
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
<input type="hidden" name="wl_maclist_x" value="<% nvram_get("wl_maclist_x"); %>">
<select name="wl_subunit" class="input_option" onChange="change_wl_unit();" style="display:none"></select>

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
					<td bgcolor="#4D595D" valign="top" id="table_height"  >
						<div>&nbsp;</div>
						<div class="formfonttitle"><#Guest_Network#></div>
						<div style="margin:10px 0 10px 5px;" class="splitLine"></div>
						<div>
							<table width="650px" style="margin:25px;">
								<tr>
									<td width="120px">
										<img id="guest_image" src="/images/New_ui/network_config.png">
									</td>
									<td>
										<div id="gn_desc" class="formfontdesc" style="font-style: italic;font-size: 14px;"><#GuestNetwork_desc#></div>
										
									</td>
								</tr>
							</table>
						</div>			
					<!-- info table -->
						<div id="guest_block_anchor"></div>
						<div id="guest_table2"></div>			
						<div id="guest_table5"></div>
						<div id="guest_table5_2"></div>
						<div id="guest_table60"></div>
						<div id="guest_tableFBWiFi">
							<table style="margin-left:20px;margin-top:25px;" width="95%" align="center" cellpadding="4" cellspacing="0" class="gninfo_head_table" id="gninfo_table_FBWiFi">
								<tr id="FBWiFi_title">
									<td align="left" style="color:#5AD; font-size:16px; border-bottom:1px dashed #AAA;" colspan="2">
										<span>Facebook WiFi</span>
									</td>
								</tr>
								<tr>
									<td width="70%">
										<span style="line-height: 20px;" >Facebook Wi-Fi lets customers check in to participating businesses on Facebok for free Wi-Fi access. When people check int to your Page, you can share offers and other announcements with them.
										</span>
									</td>
									<td width="30%" style="text-align:center;">
										<input name="button" type="button" class="button_gen" onclick="location.href='/Guest_network_fbwifi.asp'" value="<#btn_go#>"/>
									</td>
								</tr>
							</table>
						</div>
					<!-- setting table -->
						<table width="80%" border="1" align="center" style="margin-top:10px;display:none" cellpadding="4" cellspacing="0" id="gnset_table" class="FormTable">
							<tr id="wl_unit_field" style="display:none">
								<th><#Interface#></th>
								<td>
									<select name="wl_unit" class="input_option" onChange="change_wl_unit();" style="display:none">
										<option id="wl_opt0" class="content_input_fd" value="0" <% nvram_match("wl_unit", "0","selected"); %>>2.4GHz</option>
										<option id="wl_opt1" class="content_input_fd" value="1" <% nvram_match("wl_unit", "1","selected"); %>>5GHz</option>
										<option id="wl_opt2" class="content_input_fd" value="2" <% nvram_match("wl_unit", "2","selected"); %>>5GHz-2</option>
									</select>			
									<p id="wl_ifname">2.4GHz</p>
								</td>
							</tr>
							<tr style="display:none">
								<td>
									<span><span><input type="hidden" name="wl_wpa_gtk_rekey" value="<% nvram_get("wl_wpa_gtk_rekey"); %>" disabled></span></span>
								</td>
							</tr>
							<tr id="gn_index_tr">
								<th><#Guest_network_index#></th>
								<td>
									<p id="wl_vifname"></p>
								</td>
							</tr>
							<tr style="display:none">
								<th><#Guest_Network_enable#></th>
								<td>
									<select id="wl_bss_enabled_field" name="wl_bss_enabled" class="input_option">
										<option class="content_input_fd" value="0" <% nvram_match("wl_bss_enabled", "0","selected"); %>><#checkbox_No#></option>
										<option class="content_input_fd" value="1" <% nvram_match("wl_bss_enabled", "1","selected"); %>><#checkbox_Yes#></option>
									</select>			
								</td>
							</tr>
							<tr class="captive_portal_control_class">
								<th><#WLANConfig11b_x_BlockBCSSID_itemname#></th>
								<td>
									<input type="radio" value="1" name="wl_closed" class="content_input_fd" ><#checkbox_Yes#>
									<input type="radio" value="0" name="wl_closed" class="content_input_fd" ><#checkbox_No#>
								</td>
							</tr>
							<tr>
								<th><a class="hintstyle" href="javascript:void(0);" onClick="openHint(0, 1);"><#QIS_finish_wireless_item1#></a></th>
								<td>
									<input type="text" maxlength="32" class="input_32_table" name="wl_ssid" value="<% nvram_get("wl_ssid"); %>" onkeypress="return validator.isString(this, event)" autocorrect="off" autocapitalize="off">
								</td>
							</tr>	  
						<!-- Hidden and disable item, start -->
							<tr style="display:none">
								<th><a class="hintstyle" href="javascript:void(0);" onClick="openHint(0, 4);"><#WLANConfig11b_x_Mode_itemname#></a></th>
								<td>									
									<select name="wl_nmode_x" class="input_option" onChange="wireless_mode_change(this);" disabled>
										<option value="0" <% nvram_match("wl_nmode_x", "0","selected"); %>><#Auto#></option>
										<option value="1" <% nvram_match("wl_nmode_x", "1","selected"); %>>N Only</option>
										<option value="2" <% nvram_match("wl_nmode_x", "2","selected"); %>>Legacy</option>
									</select>
									<input type="checkbox" name="wl_gmode_check" id="wl_gmode_check" value="" onClick="wl_gmode_protection_check();"> b/g Protection</input>
									<!--span id="wl_nmode_x_hint" style="display:none"><#WLANConfig11n_automode_limition_hint#></span-->
								</td>
							</tr>
							<tr id="wl_channel_field">
								<th><a id="wl_channel_select" class="hintstyle" href="javascript:void(0);" onClick="openHint(0, 3);"><#WLANConfig11b_Channel_itemname#></a></th>
								<td>
									<select name="wl_channel" class="input_option" onChange="insertExtChannelOption();" disabled>
										<% select_channel("WLANConfig11b"); %>
									</select>
								</td>
							</tr>			  
							<tr id="wl_bw_field" style="display:none;">
								<th><a class="hintstyle" href="javascript:void(0);" onClick="openHint(0, 14);"><#WLANConfig11b_ChannelBW_itemname#></a></th>
								<td>				    			
									<select name="wl_bw" class="input_option" onChange="insertExtChannelOption();" disabled>
										<option class="content_input_fd" value="0" <% nvram_match("wl_bw", "0","selected"); %>>20 MHz</option>
										<option class="content_input_fd" value="1" <% nvram_match("wl_bw", "1","selected"); %>>20/40 MHz</option>
										<option class="content_input_fd" value="2" <% nvram_match("wl_bw", "2","selected"); %>>40 MHz</option>
									</select>				
								</td>
							</tr>
						<!-- Hidden and disable item, end -->
							<tr id="wl_nctrlsb_field">
								<th><a class="hintstyle" href="javascript:void(0);" onClick="openHint(0, 15);"><#WLANConfig11b_EChannel_itemname#></a></th>
								<td>
									<select name="wl_nctrlsb" class="input_option" disabled>
										<option value="lower" <% nvram_match("wl_nctrlsb", "lower", "selected"); %>>lower</option>
										<option value="upper"<% nvram_match("wl_nctrlsb", "upper", "selected"); %>>upper</option>
									</select>
								</td>
							</tr>
							<tr  id="wl_auth_mode_tr">
								<th><a class="hintstyle" href="javascript:void(0);" onClick="openHint(0, 5);"><#WLANConfig11b_AuthenticationMethod_itemname#></a></th>
								<td>
									<select name="wl_auth_mode_x" class="input_option" onChange="authentication_method_change(this);">
										<option value="open"    <% nvram_match("wl_auth_mode_x", "open",   "selected"); %>>Open System</option>
										<option value="shared"  <% nvram_match("wl_auth_mode_x", "shared", "selected"); %>>Shared Key</option>
										<option value="psk"     <% nvram_match("wl_auth_mode_x", "psk",    "selected"); %>>WPA-Personal</option>
										<option value="psk2"    <% nvram_match("wl_auth_mode_x", "psk2",   "selected"); %>>WPA2-Personal</option>
										<option value="sae"    <% nvram_match("wl_auth_mode_x", "psk2",   "selected"); %>>WPA3-Personal</option>
										<option value="pskpsk2" <% nvram_match("wl_auth_mode_x", "pskpsk2","selected"); %>>WPA-Auto-Personal</option>
										<option value="psk2sae" <% nvram_match("wl_auth_mode_x", "psk2sae","selected"); %>>WPA2/WPA3-Personal</option>
									</select>
									<br>
									<span id="wl_nmode_x_hint" style="display:none;"><#WLANConfig11n_automode_limition_hint#></span>
								</td>
							</tr>					
							<tr id="wl_crypt_tr">
								<th><a class="hintstyle" href="javascript:void(0);" onClick="openHint(0, 6);"><#WLANConfig11b_WPAType_itemname#></a></th>
								<td>		
									<select name="wl_crypto" class="input_option" onChange="authentication_method_change(this);">
										<option value="aes" <% nvram_match("wl_crypto", "aes", "selected"); %>>AES</option>
										<option value="tkip+aes" <% nvram_match("wl_crypto", "tkip+aes", "selected"); %>>TKIP+AES</option>
									</select>
								</td>
							</tr>	  
							<tr>
								<th><a id="psk_title" class="hintstyle" href="javascript:void(0);" onClick="openHint(0, 7);"><#WLANConfig11b_x_PSKKey_itemname#></a></th>
								<td>
									<input type="text" name="wl_wpa_psk" maxlength="64" class="input_32_table" value="<% nvram_get("wl_wpa_psk"); %>" autocorrect="off" autocapitalize="off">
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
								<td><input type="text" name="wl_key1" id="wl_key1" maxlength="32" class="input_32_table" value="<% nvram_get("wl_key1"); %>" onKeyUp="return change_wlkey(this, 'WLANConfig11b');" autocorrect="off" autocapitalize="off"></td>
							</tr>				  
							<tr>
								<th><a class="hintstyle" href="javascript:void(0);" onClick="openHint(0, 18);"><#WLANConfig11b_WEPKey2_itemname#></th>
								<td><input type="text" name="wl_key2" id="wl_key2" maxlength="32" class="input_32_table" value="<% nvram_get("wl_key2"); %>" onKeyUp="return change_wlkey(this, 'WLANConfig11b');" autocorrect="off" autocapitalize="off"></td>
							</tr>				  
							<tr>
								<th><a class="hintstyle" href="javascript:void(0);" onClick="openHint(0, 18);"><#WLANConfig11b_WEPKey3_itemname#></th>
								<td>
									<input type="text" name="wl_key3" id="wl_key3" maxlength="32" class="input_32_table" value="<% nvram_get("wl_key3"); %>" onKeyUp="return change_wlkey(this, 'WLANConfig11b');" autocorrect="off" autocapitalize="off">
								</td>
							</tr>				  
							<tr>
								<th><a class="hintstyle" href="javascript:void(0);" onClick="openHint(0, 18);"><#WLANConfig11b_WEPKey4_itemname#></th>
								<td>
									<input type="text" name="wl_key4" id="wl_key4" maxlength="32" class="input_32_table" value="<% nvram_get("wl_key4"); %>" onKeyUp="return change_wlkey(this, 'WLANConfig11b');" autocorrect="off" autocapitalize="off">
								</td>
							</tr>
							<tr style="display:none">
								<th><a class="hintstyle" href="javascript:void(0);" onClick="openHint(0, 8);"><#WLANConfig11b_x_Phrase_itemname#></a></th>
								<td>
									<input type="text" name="wl_phrase_x" maxlength="64" class="input_32_table" value="<% nvram_get("wl_phrase_x"); %>" onKeyUp="return is_wlphrase('WLANConfig11b', 'wl_phrase_x', this);" autocorrect="off" autocapitalize="off">
								</td>
							</tr>
							<tr class="captive_portal_control_class">
								<th><a class="hintstyle" href="javascript:void(0);" onClick="openHint(0, 25);"><#Access_Time#></a></th>
								<td>
									<input type="radio" value="1" name="wl_expire_radio" class="content_input_fd" onClick="">
									<select name="wl_expire_day" class="input_option"></select> <#Day#>
									<input type="text" maxlength="2" name="wl_expire_hr" class="input_3_table"  value="" onKeyPress="return validator.isNumber(this,event);" onblur="validator.timeRange(this, 0);" autocorrect="off" autocapitalize="off"> <#Hour#>
									<input type="text" maxlength="2" name="wl_expire_min" class="input_3_table"  value="" onKeyPress="return validator.isNumber(this,event);" onblur="validator.timeRange(this, 1);" autocorrect="off" autocapitalize="off"> <#Minute#>
									<br>	
									<input type="radio" value="0" name="wl_expire_radio" class="content_input_fd" onClick=""><#Limitless#>
								</td>
							</tr>
							
							<tr id="bw_limiter_field" class="captive_portal_control_class">
								<th><#Bandwidth_Limiter#></th>
								<td>
										<input type="radio" value="1" name="bw_enabled_x" class="content_input_fd" onClick="show_bandwidth(1);"><#checkbox_Yes#>
										<input type="radio" value="0" name="bw_enabled_x" class="content_input_fd" onClick="show_bandwidth(0);"><#checkbox_No#>
										<span id="QoS_hint" style="color:#FC0;display:none;"></span>
								</td>
							</tr>
							<tr id="bw_limiter_settings">
								<th><#Bandwidth_Setting#></th>
								<td>
										<#download_bandwidth#> <input type="text" id="wl_bw_dl_x" name="wl_bw_dl_x" maxlength="12" onkeypress="return validator.bandwidth_code(this, event);" class="input_12_table" value=""><label style="margin-left:2px;">Mb/s</label>
										<br><br>
										<#upload_bandwidth#> <input type="text" id="wl_bw_ul_x" name="wl_bw_ul_x" maxlength="12" onkeypress="return validator.bandwidth_code(this, event);" class="input_12_table" value=""><label style="margin-left:2px;">Mb/s</label>
								</td>
							</tr>
							
							<tr class="captive_portal_control_class">
								<th><a class="hintstyle" href="javascript:void(0);" onClick="openHint(0, 26);"><#Access_Intranet#></a></th>
								<td>
									<select name="wl_lanaccess" class="input_option">
										<option value="on" <% nvram_match("wl_lanaccess", "on","selected"); %>><#WLANConfig11b_WirelessCtrl_button1name#></option>
										<option value="off" <% nvram_match("wl_lanaccess", "off","selected"); %>><#btn_disable#></option>
									</select>
								</td>
							</tr>
							<tr>
								<th><#enable_macmode#></th>
								<td>
									<select name="wl_macmode" class="input_option">
										<option class="content_input_fd" value="disabled" <% nvram_match("wl_macmode", "disabled","selected"); %>><#btn_disable#></option>
										<option class="content_input_fd" value="allow" <% nvram_match("wl_macmode", "allow","selected"); %>><#FirewallConfig_MFMethod_item1#></option>
										<option class="content_input_fd" value="deny" <% nvram_match("wl_macmode", "deny","selected"); %>><#FirewallConfig_MFMethod_item2#></option>
									</select>
									<script>
									document.form.wl_macmode.onchange = function(){
										document.getElementById("maclistMain").style.display = (this.value == "disabled") ? "none" : "";
									}
									</script>
								</td>
							</tr>
						</table>

						<div id="maclistMain">
							<table id="maclistTable" width="80%" border="1" align="center" cellpadding="4" cellspacing="0" class="FormTable_table">
								<thead>
									<tr>
										<td colspan="2"><#FirewallConfig_MFList_groupitemname#>&nbsp;(<#List_limit#>&nbsp;16)</td>
									</tr>
								</thead>
									<tr>
										<th width="80%"><a class="hintstyle" href="javascript:void(0);" onClick="openHint(5,10);"><#Client_Name#> (<#PPPConnection_x_MacAddressForISP_itemname#>)</th> 
										<th width="20%"><#list_add_delete#></th>
									</tr>
									<tr>
										<td width="80%">
											<input type="text" maxlength="17" class="input_macaddr_table" name="wl_maclist_x_0" onKeyPress="return validator.isHWAddr(this,event)" onClick="hideClients_Block();" autocorrect="off" autocapitalize="off" placeholder="ex: <% nvram_get("lan_hwaddr"); %>" style="width:255px;">
											<img id="pull_arrow" height="14px;" src="/images/arrow-down.gif" style="position:absolute;display:none;" onclick="pullWLMACList(this);" title="<#select_wireless_MAC#>">
											<div id="WL_MAC_List_Block" class="clientlist_dropdown" style="margin-left:107px;"></div>
										</td>
										<td width="20%">	
											<input type="button" class="add_btn" onClick="addRow(document.form.wl_maclist_x_0, 16);" value="">
										</td>
									</tr>      		
							</table>
							<div id="wl_maclist_x_Block"></div>
						</div>

						<div class="apply_gen" id="applyButton" style="display:none;margin-top:20px">
							<input type="button" class="button_gen" value="<#CTL_Cancel#>" onclick="guest_divctrl(0);">
							<input type="button" class="button_gen" value="<#CTL_apply#>" onclick="applyRule();">
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
</body>
</html>
