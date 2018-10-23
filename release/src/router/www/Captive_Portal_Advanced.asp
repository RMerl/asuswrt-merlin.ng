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
<title><#Web_Title#> - <#Captive_Portal#></title>
<link rel="stylesheet" type="text/css" href="index_style.css"> 
<link rel="stylesheet" type="text/css" href="form_style.css">
<link rel="stylesheet" type="text/css" href="other.css">
<link rel="stylesheet" type="text/css" href="Captive_Portal_Advanced.css">
<script type="text/javascript" src="state.js"></script>
<script type="text/javascript" src="general.js"></script>
<script type="text/javascript" src="popup.js"></script>
<script type="text/javascript" src="help.js"></script>
<script type="text/javascript" src="validator.js"></script>
<script type="text/javascript" src="js/jquery.js"></script>
<script type="text/javascript" src="calendar/jquery-ui.js"></script>
<script type="text/javascript" src="form.js"></script>
<script type="text/javascript" src="switcherplugin/jquery.iphone-switch.js"></script>
<script type="text/javascript" src="jscolor/jscolor.js"></script>
<script type="text/javascript" src="Captive_Portal_Advanced_template.js"></script>
<script>
// disable auto log out
AUTOLOGOUT_MAX_MINUTE = 0;
var captive_portal_adv_profile_list = decodeURIComponent('<% nvram_char_to_ascii("","captive_portal_adv_profile"); %>');
var captive_portal_adv_local_clientlist = decodeURIComponent('<% nvram_char_to_ascii("","captive_portal_adv_local_clientlist"); %>');
var local_list_current = "";
var captive_portal_adv_enable = '<% nvram_get("captive_portal_adv_enable"); %>';
var profile_id = "";
var captive_portal_adv_edit_idx = 1; 
var component_click = "";
var toolbar_click = "";
var edit_background_base64 = "";
var edit_image_base64_array = new Array();
var component_array = new Array();
var component_array_default = new Array();
var template_click = "";
var template_page_click = 1;
var used_profile_wl_if = new Array();
var splash_page_toolbar_unit_help_flag = false;
var component_attribute = function() {
	this.style_left = "0px";
	this.style_top = "0px";
	this.style_height = "510px";
	this.style_width = "340px";
	this.style_background_color = "#4D595D";
	this.style_z_index = 0;
	this.style_opacity = 1;
	this.style_color = "#000000";
};
var jscolor_attr = "{closable:true, closeText:\"<#CTL_close#>\", onFineChange:\"edit_component_color_callback(this)\"}";
window.onresize = function() {
	if(document.getElementById("captive_portal_adv_log_panel").style.display == "block") {
		cal_panel_block("captive_portal_adv_log_panel", 0.05);
	}
};
function initial(){
	show_menu();
	show_profilelist();

	if(captive_portal_adv_enable == "on") {
		captivePortalAdvShowAndHide(1);
	}
	else {
		captivePortalAdvShowAndHide(0);
	}

	if(captive_portal_adv_profile_list == "") {
		editProfile("new");
	}
}
function captivePortalAdvShowAndHide(_flag) {
	if(_flag == 1) {
		captive_portal_adv_enable = "on";
	}
	else {
		captive_portal_adv_enable = "off";
	}
}
function deleteProfile(r, _profile_id) {
	if(!validator.numberRange($('#captive_portal_adv_idle_timeout')[0], 60, 800)){
		$("#captive_portal_adv_idle_timeout").focus();
		return false;
	}

	var del_idx = r.parentNode.parentNode.rowIndex;
	document.getElementById(r.parentNode.parentNode.parentNode.parentNode.id).deleteRow(del_idx);
	
	var rc_service_wl_en = "";
	var rc_service_wl_dis = "";
	var captive_portal_list_temp = "";
	var captive_portal_list_row = captive_portal_adv_profile_list.split("<");
	for(var i = 0; i < captive_portal_list_row.length; i += 1) {
		if(captive_portal_list_row[i] != "") {
			var captive_portal_list_col = captive_portal_list_row[i].split(">");
			if(captive_portal_list_col[0] == _profile_id) {
				rc_service_wl_en = "";
				rc_service_wl_dis = re_gen_wl_if(captive_portal_list_col[5]);
				continue;
			}
			else {
				captive_portal_list_temp += "<" + captive_portal_list_row[i];
			}
		}
	}
	captive_portal_adv_profile_list = captive_portal_list_temp;

	var local_clientlist_temp = "";
	var local_clientlist_row = captive_portal_adv_local_clientlist.split("<");
	for(var i = 0; i < local_clientlist_row.length; i += 1) {
		if(local_clientlist_row[i] != "") {
			var local_clientlist_col = local_clientlist_row[i].split(">");
			if(local_clientlist_col[0] == _profile_id) {
				continue;
			}
			else {
				local_clientlist_temp += "<" + local_clientlist_row[i];
			}
		}
	}
	captive_portal_adv_local_clientlist = local_clientlist_temp;

	if(document.form.captive_portal_adv_enable.value == "off" && captive_portal_adv_enable == "off") {
		document.form.action_script.value = "saveNvram";
		document.form.captive_portal_adv_wl_en.value = "";
		document.form.captive_portal_adv_wl_dis.value = "";
		showLoading(3);
	}
	else {
		document.form.action_script.value = "set_captive_portal_adv_wl;restart_wireless;restart_CP;restart_uam_srv";
		document.form.captive_portal_adv_wl_en.value = rc_service_wl_en;
		document.form.captive_portal_adv_wl_dis.value = rc_service_wl_dis;
		showLoading();
	}

	document.form.captive_portal_adv_profile.value = captive_portal_adv_profile_list;
	document.form.captive_portal_adv_enable.value = captive_portal_adv_enable;
	document.form.captive_portal_adv_idle_timeout.value = $("#captive_portal_adv_idle_timeout").val();
	document.form.captive_portal_adv_local_clientlist.value = captive_portal_adv_local_clientlist;
	
	document.splash_page_form_del.splash_page_id_del.value = _profile_id;
	document.splash_page_form_del.submit();
}
function show_profilelist() {
	var captive_portal_list_row = captive_portal_adv_profile_list.split("<");
	var auth_array = ["<#btn_disable#>", "Account/Password", "Account/Password", "Account/Password"];/*untranslated*/
	var code = "";
	var arrayLength = (captive_portal_list_row.length == 1) ? 0 : captive_portal_list_row.length;
	used_profile_wl_if = [];
	code += '<table width="100%" cellspacing="0" cellpadding="4" align="center" class="list_table" id="captive_portal_adv_profile_table">';
	if(arrayLength == 0)
		code += '<tr><td style="color:#FFCC00;" colspan="5"><#IPConnection_VSList_Norule#></td></tr>';
	else{
		for(var i = 0; i < arrayLength; i += 1) {
			if(captive_portal_list_row[i] != "") {
				var captive_portal_list_col = captive_portal_list_row[i].split(">");
				code += '<tr>';
				code += '<td width="30%">' + captive_portal_list_col[1] + '</td>';
				code += '<td width="20%">' + auth_array[captive_portal_list_col[7]] + '</td>';
				code += '<td id="td_wl_name" width="30%"><#wl_securitylevel_0#></td>';
				used_profile_wl_if[captive_portal_list_col[0]] = captive_portal_list_col[5];
				code += '<td width="10%"><input class="edit_btn" type="button" onclick="editProfile(\'' + captive_portal_list_col[0] + '\');" value="" /></td>';
				code += '<td width="10%"><input class="remove_btn" type="button" onclick="deleteProfile(this, \'' + captive_portal_list_col[0] + '\');"" value="" /></td>';
				code += '</tr>';
			}
		}
	}
	code += '</table>';
	document.getElementById("captive_portal_adv_profile_block").innerHTML = code;

	var get_wl_name = function(_gn_array, _if) {
		var wl_name = "";
		var idx = _if.split(".")[1];
		wl_name =  _gn_array[idx - 1][1];
		return wl_name;
	};
	//replace wifi name
	var replace_wl_name = "";
	Object.keys(used_profile_wl_if).forEach(function(key) {
		var used_profile_wl_if_id = key;
		var used_profile_wl_if_array = used_profile_wl_if[key].split("wl");
		var captive_portal_adv_ssid = "";
		
		for(var i = 0; i < used_profile_wl_if_array.length; i += 1) {
			if(used_profile_wl_if_array[i] != "") {
				var wl_band = used_profile_wl_if_array[i].split(".")[0];
				switch(wl_band) {
					case "0" :
						captive_portal_adv_ssid = decodeURIComponent('<% nvram_char_to_ascii("", "captive_portal_adv_2g_ssid"); %>');
						if(captive_portal_adv_ssid != "")
							replace_wl_name += captive_portal_adv_ssid;
						break;
					case "1" :
						if(replace_wl_name != "") {
							replace_wl_name += "<br>";
						}
						captive_portal_adv_ssid = decodeURIComponent('<% nvram_char_to_ascii("", "captive_portal_adv_5g_ssid"); %>');
						if(captive_portal_adv_ssid != "")
							replace_wl_name += captive_portal_adv_ssid;
						break;
					case "2" :
						if(replace_wl_name != "") {
							replace_wl_name += "<br>";
						}
						captive_portal_adv_ssid = decodeURIComponent('<% nvram_char_to_ascii("", "captive_portal_adv_5g_2_ssid"); %>');
						if(captive_portal_adv_ssid != "")
							replace_wl_name += captive_portal_adv_ssid;
						break;
				}
			}
		}
	});
	if(replace_wl_name != "")
		$("#td_wl_name").html(replace_wl_name);
}
function set_tab_and_action_btn() {
	$('#captive_portal_adv_step_1').attr("class","step_bg");
	$('#captive_portal_adv_step_2').attr("class","step_bg");
	$('#captive_portal_adv_step_3').attr("class","step_bg");

	$('#captive_portal_adv_step_1_content').css("display", "none");
	$('#captive_portal_adv_step_2_content').css("display", "none");
	$('#captive_portal_adv_step_3_content').css("display", "none");

	$("#captive_portal_adv_pre").css("display", "none");
	$("#captive_portal_adv_next").css("display", "none");
	$("#captive_portal_adv_finsih").css("display", "none");

	$('#captive_portal_adv_step_' + captive_portal_adv_edit_idx + '').attr("class","step_bg current");
	$('#captive_portal_adv_step_' + captive_portal_adv_edit_idx + '_content').css("display", "");

	switch(captive_portal_adv_edit_idx) {
		case 1 :
			$("#captive_portal_adv_next").css("display", "");
			break;
		case 2 :
			if(captive_portal_adv_profile_list == "") {
				if(!splash_page_toolbar_unit_help_flag)
					$(".splash_page_toolbar_unit_help").click();
			}	
			$("#captive_portal_adv_pre").css("display", "");
			$("#captive_portal_adv_next").css("display", "");
			break;
		case 3 :
			$("#captive_portal_adv_pre").css("display", "");
			$("#captive_portal_adv_finsih").css("display", "");

			$("#account_settings_hint").css("display", "");

			if(component_array["component_account"]) {
				$("#account_settings_hint").css("display", "none");
			}
			break;
	}
}
function edit_cancel(_flag) {
	var _confirm_flag = true;
	if(_flag) {
		_confirm_flag = confirm("<#Captive_Portal_Profile_Exit#>");
	}

	if(_confirm_flag) {
		$('#captive_portal_adv_setting').fadeOut();
		captive_portal_adv_edit_idx = 1;
	}
}
function preview(event) {
	window.open('Captive_Portal_Advanced_Preview.asp', '<#FreeWiFi_title#>', 'toolbar=no,scrollbars=no,resizable=yes,location=no,menubar=no,width=476,height=714');
	stop_propagation(event);
}
function help_close() {
	$('#captive_portal_adv_tool_help_bg').empty();
	$('#captive_portal_adv_tool_help_bg').fadeOut();
	$(document).unbind("keyup", keyUpFunc);
}
function keyUpFunc(e) {
	if (e.keyCode === 27) {
		if($('#captive_portal_adv_tool_help_bg').css("display") == "block") {
			help_close();
		}
	}
}
function switch_help(_action, event) {
	if(_action == "next") {
		help_idx++ ;
		if(help_idx == 7) {
			help_close();
		}
	}
	else
		help_idx-- ;

	$(".help_step").css("display", "none");
	$("#help_desc_action_close").css("display", "none");
	$("#help_desc_action_next").css("display", "none");
	$("#help_desc_action_pre").css("display", "none");
	switch(help_idx) {
		case 1 :
			$("#help_desc_action_next").css("display", "block");
			break;
		case 6 :
			$("#help_desc_action_close").css("display", "block");
			$("#help_desc_action_pre").css("display", "block");
			break;
		default :
			$("#help_desc_action_next").css("display", "block");
			$("#help_desc_action_pre").css("display", "block");
			break;
	}
	$(".help_desc_action_idx").html("" + help_idx + " / 6");
	$("#help_step_" + help_idx + "").css("display", "block");
	stop_propagation(event);
}
var help_idx = 1;
function toolbar_unit_help(event) {
	splash_page_toolbar_unit_help_flag = true;
	help_idx = 1;
	var code = "";

	code += "<div class='captive_portal_adv_log_close'><img src='/images/button-close.gif' style='width:35px;' onclick='help_close();'></div>";

	code += "<div id='help_step_1' class='help_step'>";
	code += "<div class='help_frame step1'>";
	code += "<div id='toolbar_template' title='<#Captive_Portal_Template#>' class='splash_page_toolbar_unit_bg_right splash_page_toolbar_unit_template' style='cursor:default;'>";
	code += "<div class='splash_page_toolbar_unit_title'><#Captive_Portal_Template#></div>";
	code += "</div>";
	code += "</div>";
	code += "<div class='help_desc step1'>";
	code += "<div class='help_desc_title'>1. <#FreeWiFi_Temp_Select#></div>";
	code += "<div class='help_desc_text'><#FreeWiFi_Temp_Select_desc#></div>";
	code += "</div>";
	code += "</div>";
	code += "<div id='help_step_2' class='help_step' style='display:none;'>";
	code += "<div class='help_frame step2'>";
	code += "<div class='help_desc_title'>2. <#FreeWiFi_Temp_WebLayout#></div>";
	code += "<div class='help_desc_text'><#FreeWiFi_Temp_WebLayout_desc#></div>";
	code += "</div>";
	code += "</div>";

	code += "<div id='help_step_3' class='help_step' style='display:none;'>";
	code += "<div class='help_frame step3'>";
	code += "<div id='toolbar_unit_image' title='<#CTL_add#> <#Captive_Portal_Image#>' class='splash_page_toolbar_unit_bg_left splash_page_toolbar_unit_image' style='cursor:default;'></div>";
	code += "<div id='toolbar_unit_text' title='<#CTL_add#> Text' class='splash_page_toolbar_unit_bg_left splash_page_toolbar_unit_text' style='cursor:default;'></div>";
	code += "<div id='toolbar_unit_account' title='<#CTL_add#> <#PPPConnection_Authentication_itemname#>' class='splash_page_toolbar_unit_bg_left splash_page_toolbar_unit_account' style='cursor:default;'></div>";
	code += "<div id='toolbar_unit_eula' title='<#CTL_add#> Eula' class='splash_page_toolbar_unit_bg_left splash_page_toolbar_unit_eula' style='cursor:default;'></div>";
	code += "</div>";
	code += "<div class='help_desc step3'>";
	code += "<div class='help_desc_title'>3. <#FreeWiFi_Temp_Add#></div>";
	code += "<div class='help_desc_text'><#FreeWiFi_Temp_Add_desc1#></div>";
	code += "<div class='help_desc_text'><#FreeWiFi_Temp_Add_desc2#></div>";
	code += "</div>";
	code += "</div>";

	code += "<div id='help_step_4' class='help_step' style='display:none;'>";
	code += "<div class='help_frame step4'>";
	code += "<div id='toolbar_background'  title='<#btn_Background#>' class='splash_page_toolbar_unit_bg_right splash_page_toolbar_unit_background' style='cursor:default;'>";
	code += "<div class='splash_page_toolbar_unit_title'><#btn_Background#></div>";
	code += "</div>";
	code += "<div id='toolbar_editor' title='<#Captive_Portal_Editor#>' class='splash_page_toolbar_unit_bg_right splash_page_toolbar_unit_editor' style='cursor:default;'>";
	code += "<div class='splash_page_toolbar_unit_title'><#Captive_Portal_Editor#></div>";
	code += "</div>";
	code += "</div>";
	code += "<div class='help_desc step4'>";
	code += "<div class='help_desc_title'>4. <#FreeWiFi_Temp_Edit#></div>";
	code += "<div class='help_desc_text'><#FreeWiFi_Temp_Edit_desc1#></div>";
	code += "<div class='help_desc_text'><#FreeWiFi_Temp_Edit_desc2#></div>";
	code += "</div>";
	code += "</div>";

	code += "<div id='help_step_5' class='help_step' style='display:none;'>";
	code += "<div class='help_frame step5'>";
	code += "<div title='<#btn_Preview#>' class='splash_page_toolbar_unit_preview' style='cursor:default;'></div>";
	code += "</div>";
	code += "<div class='help_desc step5'>";
	code += "<div class='help_desc_title'>5. <#FreeWiFi_Temp_Preview#></div>";
	code += "<div class='help_desc_text'><#FreeWiFi_Temp_Preview_desc1#></div>";
	code += "<div class='help_desc_text'><#FreeWiFi_Temp_Preview_desc2#></div>";
	code += "</div>";
	code += "</div>";

	code += "<div id='help_step_6' class='help_step' style='display:none;'>";
	code += "<div class='help_frame step6'>";
	code += "<input class='button_gen' type='button' style='cursor:default;' value='<#CTL_Reset_OOB#>' disabled/>";
	code += "</div>";
	code += "<div class='help_desc step6'>";
	code += "<div class='help_desc_title'>6. <#FreeWiFi_Temp_Reset#></div>";
	code += "<div class='help_desc_text'><#FreeWiFi_Temp_Reset_desc#></div>";
	code += "</div>";
	code += "</div>";

	code += "<div class='help_desc_action_bg'>";
	code += "<div class='help_desc_action_idx'>1 / 6</div>";
	code += "<div id='help_desc_action_close' class='help_desc_action' style='display:none;' onclick='help_close();'><#CTL_close#></div>";
	code += "<div id='help_desc_action_next' class='help_desc_action' onclick='switch_help(\"next\", event);'><#CTL_next#></div>";
	code += "<div id='help_desc_action_pre' class='help_desc_action' style='display:none;' onclick='switch_help(\"pre\", event);'><#CTL_prev#></div>";
	code += "</div>";

	$('#captive_portal_adv_tool_help_bg').html(code);
	$('#captive_portal_adv_tool_help_bg').fadeIn();

	$(document).keyup(keyUpFunc);

	stop_propagation(event);
}
function gen_tab_menu() {
	var code = "";
	code += "<div style='height:50px;'>"
	code += "<div id='captive_portal_adv_step_1' onclick='switch_tab(1)'>1. <#Configuration#></div>";
	code += "<div id='captive_portal_adv_step_2' onclick='switch_tab(2)'>2. <#FreeWiFi_title#></div>";
	code += "<div id='captive_portal_adv_step_3' onclick='switch_tab(3)'>3. <#PPPConnection_UserName_sectionname#></div>";
	code += "</div>";
	return code;
}
function gen_action_button() {
	var code = "";
	code += "<div class='captive_portal_adv_action_bg'>";
	code += "<div class='captive_portal_adv_line'></div>";
	code += "<input id='captive_portal_adv_pre' class='button_gen' style='display:none;' onclick='preRule()' type='button' value='<#CTL_prev#>'/>";
	code += "<input id='captive_portal_adv_next' class='button_gen' style='display:none;' onclick='nextRule()' type='button' value='<#CTL_next#>'/>";
	code += "<input id='captive_portal_adv_finsih' class='button_gen' style='display:none;' type='button' value='<#CTL_finish#>'/>";
	code += "</div>";
	return code;
}
function clearHTML(objName) {
	if($("#" + objName + "").length > 0) {
		$("#" + objName + "").empty();
	}
}
function gen_captive_portal_adv_edit_content() {
	clearHTML("captive_portal_adv_step_1_content");
	clearHTML("captive_portal_adv_step_2_content");
	clearHTML("captive_portal_adv_step_3_content");

	$("#captive_portal_adv_step_1_content").append(gen_captive_portal_adv_content("1"));
	$("#captive_portal_adv_step_2_content").append(gen_captive_portal_adv_content("2"));
	$("#captive_portal_adv_step_3_content").append(gen_captive_portal_adv_content("3"));
}
function change_wl_input_status(_idx) {
	if($("#cb_wl_" + _idx + "").prop("checked")) {
		if($("input[name=wl_" + _idx + "]").hasClass("wl_input_disabled")) {
			$("input[name=wl_" + _idx + "]").removeClass("wl_input_disabled");
			$("input[name=wl_" + _idx + "]").attr('disabled', false);
		}
	}
	else {
		$("input[name=wl_" + _idx + "]").addClass("wl_input_disabled");
		$("input[name=wl_" + _idx + "]").attr('disabled', true);
	}
}
function auto_fill_wl_name() {
	var brand_name = $("input[name=cpa_profile_name]").val();
	if(brand_name.length > 32) {
		if(wl_info.band2g_support) {
			$("input[name=wl_0]").val(brand_name);
		}
		if(wl_info.band5g_support) {
			$("input[name=wl_1]").val(brand_name);
		}
		if(wl_info.band5g_2_support) {
			$("input[name=wl_2]").val(brand_name);
		}
	}
	else {
		var brand_name_temp = "";
		if(wl_info.band2g_support) {
			brand_name_temp = brand_name;
			$("input[name=wl_0]").val(brand_name_temp);
		}
		if(wl_info.band5g_support) {
			brand_name_temp = brand_name.substring(0,29) + "_5G";
			$("input[name=wl_1]").val(brand_name_temp);
		}
		if(wl_info.band5g_2_support) {
			brand_name_temp = brand_name.substring(0,27) + "_5G-2";
			$("input[name=wl_2]").val(brand_name_temp);
		}
	}
}
function gen_basic_settings() {
	var code = "";
	code += "<table width='100%' border='1' align='center' cellpadding='4' cellspacing='0' bordercolor='#6b8fa3' class='FormTable'>";
	code += "<thead>";
	code += "<tr>";
	code += "<td colspan='2'><#t2BC#></td>";
	code += "</tr>";
	code += "</thead>";
	code += "<tr>";
	code += "<th><#FreeWiFi_BrandName#></th>";
	code += "<td>";
	code += "<input style='margin-left:0px;' type='text' name='cpa_profile_name' value='Brand Name' maxlength='32' class='input_25_table' autocomplete='off' autocorrect='off' autocapitalize='off' onkeyup='auto_fill_wl_name();'>";
	code += "</td>";
	code += "</tr>";

	var guest_group_num = gn_array_2g.length;
	var gen_wl_interface = function(_unit, _gn_array) {
		var _default_wl_name = ["Brand Name", "Brand Name_5G", "Brand Name_5G-2"];
		var code = "";
		code += "<tr>";
		code += "<th>" + wl_nband_title[_unit] + " <#QIS_finish_wireless_item1#></th>";
		code += "<td>";
		code += "<input type='checkbox' name='cb_wl_" + _unit + "' id='cb_wl_" + _unit + "' onclick='change_wl_input_status(" + _unit + ");' checked>";
		code += "<input name='wl_" + _unit + "' class='input_25_table' value='" + _default_wl_name[_unit] + "' type='text' maxlength='32' autocorrect='off' autocapitalize='off'>";
		code += "</td>";
		code += "</tr>";
		return code;
	};

	//2.4G
	code += gen_wl_interface(0, gn_array_2g);
	//5G
	if(wl_info.band5g_support && !no5gmssid_support) {
		code += gen_wl_interface(1, gn_array_5g);
	}
	//5G-2
	if(wl_info.band5g_2_support) {
		code += gen_wl_interface(2, gn_array_5g_2);
	}

	code += "</table>";

	//Access Policy
	code += "<table width='100%' border='1' align='center' cellpadding='4' cellspacing='0' bordercolor='#6b8fa3' class='FormTable' style='margin-top:15px;'>";
	code += "<thead>";
	code += "<tr>";
	code += "<td colspan='2'><#FreeWiFi_AccessPolicy#></td>";
	code += "</tr>";
	code += "</thead>";

	code += "<tr>";
	code += "<th><a class='hintstyle' href='javascript:void(0);' onClick='openHint(31, 2);'><#FreeWiFi_Idle#></a></th>";
	code += "<td>";
	code += "<input type='radio' name='cpa_away_timeout' id='cpa_away_timeout_time'>";
	code += "<input type='text' name='cpa_away_set' value='1' maxlength='3' class='input_6_table' onKeyPress='return validator.isNumber(this,event)' autocomplete='off' autocorrect='off' autocapitalize='off'>";
	code += "&nbsp;&nbsp;<select name='cpa_away_timeout_unit' class='input_option'>";
	code += "<option value='hour'><#Hour#></option>";
	code += "<option value='minute'><#Minute#></option>";
	code += "</select>";
	code += "&nbsp;&nbsp;<input type='radio' name='cpa_away_timeout' id='cpa_away_timeout_unlimited' checked><label for='cpa_away_timeout_unlimited'><#Limitless#></label>";
	code += "</td>";
	code += "</tr>";

	code += "<tr>";
	code += "<th><a class='hintstyle' href='javascript:void(0);' onClick='openHint(31, 3);'><#FreeWiFi_timeout#></a></th>";
	code += "<td>";
	code += "<input type='radio' name='cpa_session_timeout' id='cpa_session_timeout_time'>";
	code += "<input type='text' name='cpa_session_set' value='1' maxlength='3' class='input_6_table' onKeyPress='return validator.isNumber(this,event)' autocomplete='off' autocorrect='off' autocapitalize='off'>";
	code += "&nbsp;&nbsp;<select name='cpa_session_timeout_unit' class='input_option'>";
	code += "<option value='day'><#Day#></option>";
	code += "<option value='hour'><#Hour#></option>";
	code += "<option value='minute'><#Minute#></option>";
	code += "</select>";
	code += "&nbsp;&nbsp;<input type='radio' name='cpa_session_timeout' id='cpa_session_timeout_unlimited' checked><label for='cpa_session_timeout_unlimited'><#Limitless#></label>";
	code += "</td>";
	code += "</tr>";

	code += "<tr>";
	code += "<th><#Captive_Portal_DL_Speed#></th>";
	code += "<td>";
	code += "<input type='radio' name='cpa_bw_dl' id='cpa_bw_dl_limited'>";
	code += "<input name='cpa_bw_dl_set' class='input_15_table' value='1' type='text' maxlength='12' autocorrect='off' autocapitalize='off' onKeyPress='return validator.bandwidth_code(this, event);'>";
	code += "&nbsp;&nbsp;Mb/s";
	code += "&nbsp;&nbsp;<input type='radio' name='cpa_bw_dl' id='cpa_bw_dl_unlimited' checked><label for='cpa_bw_dl_unlimited'><#Limitless#></label>";
	code += "</td>";
	code += "</tr>";

	code += "<tr>";
	code += "<th><#Captive_Portal_UL_Speed#></th>";
	code += "<td>";
	code += "<input type='radio' name='cpa_bw_ul' id='cpa_bw_ul_limited'>";
	code += "<input name='cpa_bw_ul_set' class='input_15_table' value='1' type='text' maxlength='12' autocorrect='off' autocapitalize='off' onKeyPress='return validator.bandwidth_code(this, event);'>";
	code += "&nbsp;&nbsp;Mb/s";
	code += "&nbsp;&nbsp;<input type='radio' name='cpa_bw_ul' id='cpa_bw_ul_unlimited' checked><label for='cpa_bw_ul_unlimited'><#Limitless#></label>";
	code += "</td>";
	code += "</tr>";

	code += "<tr>";
	code += "<th><#FreeWiFi_LandingPage#> (<#FreeWiFi_RedirectPage#>)</th>";
	code += "<td>";
	code += "<input style='margin-left:0px;' type='text' name='cpa_landing_page' value='' class='input_25_table' maxlength='64' autocomplete='off' autocorrect='off' autocapitalize='off'>";
	code += "<span style='margin-left:5px;'>ex. http <#Captive_Portal_OR#> https ://www.asus.com</span>";
	code += "</td>";
	code += "</tr>";

	code += "</table>";
	return code;
}
function edit_del_component(_obj, event) {
	var del_component_id = "component_" + _obj;
	$("#" + del_component_id).remove();

	if (_obj.indexOf("image") >= 0 || _obj.indexOf("text") >= 0) {
		var removeType = _obj.split("_")[0];
		var removeItem = _obj.split("_")[1];
		if(component_array["component_" + removeType + ""]) {
			if(component_array["component_" + removeType + ""][removeItem])
				delete component_array["component_" + removeType + ""][removeItem];
		}
	}
	else {
		delete component_array[del_component_id];
	}

	if(component_click == del_component_id) {
		var code = "<div class='edit_component_item_hint'><#Captive_Portal_Select_Element#></div>";
		$("#splash_page_container_edit").html(code);
	}
	reset_toolbar_component_css();
	stop_propagation(event);
}
function set_component_attribute(obj) {
	var component_set_attribute = new component_attribute();	
	component_set_attribute.style_left = $("#" + obj + "").css("left");
	component_set_attribute.style_top = $("#" + obj + "").css("top");
	component_set_attribute.style_height = parseInt($("#" + obj + "").css("height"), 10) + "px";
	component_set_attribute.style_width = parseInt($("#" + obj + "").css("width"), 10) + "px";
	component_set_attribute.style_background_color = $("#" + obj + "").css("background-color");
	component_set_attribute.style_z_index = $("#" + obj + "").css("z-index");
	component_set_attribute.style_opacity = $("#" + obj + "").css("opacity");
	component_set_attribute.style_color = $("#" + obj + "").css("color");
	return component_set_attribute;
}
function reset_toolbar_component_css() {
	var reset_css = function(_component_type, _limit_num) {
		var _component_num = 0;
		if(_component_type == "image" || _component_type == "text") {
			_component_num = $(".multi_" + _component_type + "").length;
		}
		else {
			_component_num = $("#component_" + _component_type + "").length;
		}
		if(_component_num < _limit_num) {
			if($("#toolbar_unit_" + _component_type + "").hasClass("splash_page_toolbar_unit_" + _component_type + "_disabled")) {
				$("#toolbar_unit_" + _component_type + "").removeClass("splash_page_toolbar_unit_" + _component_type + "_disabled");
			}
			$("#toolbar_unit_" + _component_type + "").addClass("splash_page_toolbar_unit_" + _component_type + "");			
		}
		else {
			if($("#toolbar_unit_" + _component_type + "").hasClass("splash_page_toolbar_unit_" + _component_type + "")) {
				$("#toolbar_unit_" + _component_type + "").removeClass("splash_page_toolbar_unit_" + _component_type + "");
			}
			$("#toolbar_unit_" + _component_type + "").addClass("splash_page_toolbar_unit_" + _component_type + "_disabled");
		}
	};

	reset_css("image", 5);
	reset_css("text", 5);
	reset_css("eula", 1);
	reset_css("account", 1);
}
function reset_toolbar_css() {
	var selected_toolbar = toolbar_click;
	var removeCss = function(_obj) {
		if($("#" + _obj + "").hasClass("selected")) {
			$("#" + _obj + "").removeClass("selected");
			$("#" + _obj + "").find(".splash_page_toolbar_unit_title").removeClass("selected");
		}
	};

	removeCss("toolbar_template");
	removeCss("toolbar_background");
	removeCss("toolbar_editor");
	removeCss("toolbar_customized");
	removeCss("toolbar_help");

	$("#" + selected_toolbar + "").addClass("selected");
	$("#" + selected_toolbar + "").find(".splash_page_toolbar_unit_title").addClass("selected");
}
function reset_component_css() {
	var removeCss = function(_obj, _component_type) {

		if($("#" + _obj + "").hasClass("component_bg_click")) {
			$("#" + _obj + "").removeClass( "component_bg_click" );
			$("#" + _obj + "").find(".component_title").removeClass("component_title_click");
			$("#" + _obj + "").find(".component_del").css({"display" : "none"});

			//remove draggable and resizable event
			if(_component_type != "background") {
				$("#" + _obj + "").draggable("destroy");
				$("#" + _obj + "").resizable("destroy");
			}
		}

		if(_component_type == "text" || _component_type == "image") {
			if(component_array["component_" + _component_type + ""] != undefined) {
				var _idx = _obj.split("_")[2];
				if(component_array["component_" + _component_type + ""][_idx] != undefined) {
					$("#" + _obj + "").addClass("component_bg_edited");
				}
			}
		}
		else {
			if(component_array[_obj] != undefined) {
				$("#" + _obj + "").addClass("component_bg_edited");
			}
		}
	};

	var removeMultiComponent = function(_component_type) {
		for(var i = 0; i < $(".multi_" + _component_type + "").length; i += 1) {
			var _component_id = $(".multi_" + _component_type + "")[i].id;
			removeCss(_component_id, _component_type);	
		}
	};

	removeMultiComponent("text");
	removeMultiComponent("image");
	removeCss("component_eula", "eula");
	removeCss("component_account", "account");
	removeCss("component_button", "button");
	removeCss("component_background", "background");
}
function set_component_click_attr(_obj) {
	$("#" + _obj + "").addClass("component_bg_click");
	$("#" + _obj + "").find(".component_title").addClass("component_title_click");
	$("#" + _obj + "").removeClass("component_bg_edited");
	$("#" + _obj + "").find(".component_del").css({"display" : "block"});

	set_drop_resize_event(_obj);
}
function edit_clear_content(_obj) {
	var _component_attribute_before = "";
	var save_component_attribute = function(_component_id) {
		var component_attribute = function() {
			this.style_left = "0px";
			this.style_top = "0px";
			this.style_height = "510px";
			this.style_width = "340px";
			this.style_z_index = 0;
		};

		var _component_attribute_temp = new component_attribute();
		_component_attribute_temp.style_left = $("#" + _component_id + "").css("left");
		_component_attribute_temp.style_top = $("#" + _component_id + "").css("top");
		_component_attribute_temp.style_width = $("#" + _component_id + "").css("width");
		_component_attribute_temp.style_height = $("#" + _component_id + "").css("height");
		_component_attribute_temp.style_z_index = $("#" + _component_id + "").css("z-index");
		return _component_attribute_temp;
	};
	var set_component_attribute = function(_old_component_attribute, _new_component_id) {
		$("#" + _new_component_id + "").css({"left" : _old_component_attribute.style_left});
		$("#" + _new_component_id + "").css({"top" : _old_component_attribute.style_top});
		$("#" + _new_component_id + "").css({"width" : _old_component_attribute.style_width});
		$("#" + _new_component_id + "").css({"height" : _old_component_attribute.style_height});
		$("#" + _new_component_id + "").css({"z-index" : _old_component_attribute.style_z_index});
	};
	var _component_type = _obj;
	if (_component_type.indexOf("image") >= 0) {
		_component_type = "image";
	}
	if (_component_type.indexOf("text") >= 0) {
		_component_type = "text";
	}
	switch(_component_type) {
		case "text" :
			_component_attribute_before = save_component_attribute("component_" + _obj + "");
			$("#component_" + _obj + "").remove();
			var removeItem = _obj.split("_")[1];
			if(component_array["component_text"]) {
				if(component_array["component_text"][removeItem] != undefined) {
					delete component_array["component_text"][removeItem];
				}
			}
			var newItem = $.now();
			$("#splash_page_layout").append(gen_splash_page_component("text", newItem));
			edit_component_text(newItem);
			set_component_attribute(_component_attribute_before, "component_text_" + newItem + "");
			break;
		case "image" :
			_component_attribute_before = save_component_attribute("component_" + _obj + "");
			$("#component_" + _obj + "").remove();
			var removeItem = _obj.split("_")[1];
			if(component_array["component_image"]) {
				if(component_array["component_image"][removeItem] != undefined) {
					delete component_array["component_image"][removeItem];
					delete edit_image_base64_array["image_" + removeItem + ""];
				}
			}
			var newItem = $.now();
			$("#splash_page_layout").append(gen_splash_page_component("image", newItem));
			edit_component_image(newItem);
			set_component_attribute(_component_attribute_before, "component_image_" + newItem + "");
			break;
		case "eula" :
			_component_attribute_before = save_component_attribute("component_eula");
			$("#component_eula").remove();
			if(component_array["component_eula"]) {
				_component_attribute_temp = component_array["component_eula"].attribute;
				delete component_array["component_eula"];
			}
			$("#splash_page_layout").append(gen_splash_page_component("eula"));
			edit_component_eula();
			set_component_attribute(_component_attribute_before, "component_eula");
			edit_update_content_eula();
			break;
		case "account" :
			_component_attribute_before = save_component_attribute("component_account");
			$("#component_account").remove();
			delete component_array["component_account"];
			$("#splash_page_layout").append(gen_splash_page_component("account"));
			edit_component_account();
			set_component_attribute(_component_attribute_before, "component_account");
			edit_update_content_account();
			$("#component_account").css("max-height", "66px");
			$("#component_account").css("height", "66px");
			break;
		case "button" :
			_component_attribute_before = save_component_attribute("component_button");
			$("#component_button").remove();
			delete component_array["component_button"];
			$("#splash_page_layout").append(gen_splash_page_component("button"));
			edit_component_button();
			set_component_attribute(_component_attribute_before, "component_button");
			edit_update_content_button();
			break;
		case "background" :
			delete edit_image_base64_array["background"];
			$("#edit_redraw_canvas")[0].getContext("2d").clearRect(0, 0, 1152, 864);
			$("#component_background").remove();
			delete component_array["component_background"];
			$("#splash_page_layout").append(gen_splash_page_component("background"));
			edit_component_background();
			settingRadioItemCheck($("input[name=edit_background_type]"), "color");
			showHideBackgroundType();
			edit_update_content_background();
			break;
	}
}
function edit_update_content_eula() {
	remove_hint_msg();
	$("input[name=edit_eula_terms_service_title]").val($.trim($("input[name=edit_eula_terms_service_title]").val()));
	var eula_terms_service_title = $("input[name=edit_eula_terms_service_title]").val();
	var eula_terms_service = $('textarea#edit_eula_terms_service').val().trim();
	var font_color = "#" + $("#edit_eula_label_color_set").val();
	var terms_font_color = "#" + $("#edit_eula_terms_color_set").val();
	var terms_background_color = "#" + $("#edit_eula_terms_background_color_set").val();
	$("#component_eula").css({"color" : font_color});

	$("input[name=edit_eula_text_label]").val($.trim($("input[name=edit_eula_text_label]").val()));
	var eula_text = $('input[name="edit_eula_text_label"]').val();
	$("input[name=edit_eula_hyperlink_label]").val($.trim($("input[name=edit_eula_hyperlink_label]").val()));
	var eula_hyperlink = $('input[name="edit_eula_hyperlink_label"]').val();
	var hyperlink_color = "#" + $("#edit_eula_hyperlink_label_color_set").val();

	if(eula_terms_service_title == "") {
		add_hint_msg($('input[name="edit_eula_terms_service_title"]'), "<#JS_fieldblank#>");
		$('input[name="edit_eula_terms_service_title"]').focus();
		return;
	}

	if(eula_terms_service == "") {
		add_hint_msg($('textarea#edit_eula_terms_service'), "<#JS_fieldblank#>");
		$('textarea#edit_eula_terms_service').focus();
		return;
	}

	if(eula_hyperlink == "") {
		add_hint_msg($('input[name="edit_eula_hyperlink_label"]'), "<#JS_fieldblank#>");
		$('input[name="edit_eula_hyperlink_label"]').focus();
		return;
	}

	var code = "<div style='height:100%;width:100%;'>";
	code += "<div style='float:left;'>";
	code += "<input type='checkbox' name='eula_check' disabled>";
	code += "</div>";
	code += "<div>";
	code += "<span>" + htmlEnDeCode.htmlEncode(eula_text) + "</span>&nbsp;<span class='edit_eula_terms_service_hyperlink'>" + htmlEnDeCode.htmlEncode(eula_hyperlink) + "</span>";
	code += "</div>";
	code += "</div>";

	$("#component_eula div").first().replaceWith(code);
	$("#component_eula").find(".edit_eula_terms_service_hyperlink").css({"color" : hyperlink_color});

	component_array["component_eula"] = [ { 
		"attribute" : set_component_attribute("component_eula"), 
		"eula_terms_service_title" : encode_decode_text(eula_terms_service_title, "encode"),
		"eula_terms_service" : encode_decode_text($('textarea#edit_eula_terms_service').val(), "encode"),
		"eula_terms_service_font_color" : terms_font_color,
		"eula_terms_service_background_color" : terms_background_color,
		"eula_terms_service_text": encode_decode_text(eula_text, "encode"),
		"eula_terms_service_hyperlink": encode_decode_text(eula_hyperlink, "encode"),
		"eula_terms_service_hyperlink_color": hyperlink_color
	 } ];
}
function auto_add_onblur_event(_obj) {
	var item_length = $(".auto_add_onblur").length;
	switch(_obj) {
		case "image" :
			$(".auto_add_onblur").blur(function() {
				var component_click_id = component_click.replace("component_image_", "");
				edit_update_content_image(component_click_id);
			});
			break;
		case "text" :
			$(".auto_add_onblur").blur(function() {
				var component_click_id = component_click.replace("component_text_", "");
				edit_update_content_text(component_click_id);
			});
			break;
		case "eula" :
			$(".auto_add_onblur").blur(function() {
				edit_update_content_eula();
			});
			break;
		case "account" :
			$(".auto_add_onblur").blur(function() {
				edit_update_content_account();
			});
			break;
		case "button" :
			$(".auto_add_onblur").blur(function() {
				edit_update_content_button();
			});
			break;
		case "background" :
			$(".auto_add_onblur").blur(function() {
				edit_update_content_background();
			});
			break;
	}
}
function edit_component_eula() {
	component_click = "component_eula";
	toolbar_click = "toolbar_editor";

	reset_component_css();
	reset_toolbar_css();

	set_component_click_attr(component_click);

	var code = "";
	$("#splash_page_container_edit").html(code);

	code += "<div class='edit_component_item_title'><#Captive_Portal_Text_Color#></div>";
	code += "<div class='edit_component_item_content'>";
	code += "<input name='edit_eula_label_color_set' id='edit_eula_label_color_set' class='jscolor " + jscolor_attr + " input_12_table auto_add_onblur' style='cursor:pointer;' value='C9C9C9' type='text' maxlength='6' autocorrect='off' autocapitalize='off'>";
	code += "&nbsp;&nbsp;";
	code += "<input name='edit_eula_text_label' class='input_22_table auto_add_onblur' value='I have read and agree to' type='text' maxlength='32' autocomplete='off' autocorrect='off' autocapitalize='off'>";
	code += "</div>";
	code += "<br>";
	code += "<div class='edit_component_item_content'>";
	code += "<input name='edit_eula_hyperlink_label_color_set' id='edit_eula_hyperlink_label_color_set' class='jscolor " + jscolor_attr + " input_12_table auto_add_onblur' style='cursor:pointer;' value='00B0FF' type='text' maxlength='6' autocorrect='off' autocapitalize='off'>";
	code += "&nbsp;&nbsp;";
	code += "<input name='edit_eula_hyperlink_label' class='input_22_table auto_add_onblur' value='the Terms of Service' type='text' maxlength='32' autocomplete='off' autocorrect='off' autocapitalize='off'>";
	code += "</div>";

	code += "<div class='captive_portal_adv_line'></div>";

	code += "<div class='edit_component_item_title'><#Captive_Portal_Terms_Service#> <span>(* <#Captive_Portal_Terms_Dialog#>)</span></div>";
	code += "<div class='edit_component_item_content'>";
	code += "<div class='edit_component_item_title' style='margin-top:5px;'><#Captive_Portal_Terms_Service#> title</div>";/*untranslated*/
	code += "<input name='edit_eula_terms_service_title' class='input_30_table auto_add_onblur' value='Welcome to Captive Portal Wi-Fi!' type='text' maxlength='128' autocorrect='off' autocapitalize='off'>";
	code += "</div>";

	code += "<div class='edit_component_item_content' style='margin-top:5px;'>";
	code += "<div class='edit_component_item_title' style='margin-top:5px;'><#Terms_of_Service#></div>";
	code += "<textarea name='edit_eula_terms_service' id='edit_eula_terms_service' class='edit_eula_textarea auto_add_onblur' rows='8' cols='40' maxlength='2048'>";
	code += "By using Captive Portal Wi-Fi internet service, you hereby expressly acknowledge and agree that there are significant security, privacy and confidentiality risks inherent in accessing or transmitting information through the internet, whether the connection is facilitated through wired or wireless technology. Security issues include, without limitation, interception of transmissions, loss of data, and the introduction or viruses and other programs that can corrupt or damage your computer.\n\nAccordingly, you agree that the owner and/or provider of this network is NOT liable for any interception or transmissions, computer worms or viruses, loss of data, file corruption, hacking or damage to your computer or other devices that result from the transmission or download of information or materials through the internet service provided.\n\nUse of the wireless network is subject to the general restrictions outlined below. If abnormal, illegal, or unauthorized behavior is detected, including heavy consumption of bandwidth, the network provider reserves the right to permanently disconnect the offending device from the wireless network.";
	code += "</textarea>";
	code += "</div>";

	code += "<div style='width:49%;float:left;'>";
	code += "<div class='edit_component_item_title'><#Captive_Portal_Text_Color#></div>";
	code += "<div class='edit_component_item_content'>";
	code += "<input name='edit_eula_terms_color_set' id='edit_eula_terms_color_set' class='jscolor " + jscolor_attr + " input_12_table auto_add_onblur' style='cursor:pointer;' value='E8E8E8' type='text' maxlength='6' autocorrect='off' autocapitalize='off'>";
	code += "</div>";
	code += "</div>";

	code += "<div style='width:49%;float:left;'>";
	code += "<div class='edit_component_item_title'><#Captive_Portal_Dialog_Color#></div>";
	code += "<div class='edit_component_item_content'>";
	code += "<input name='edit_eula_terms_background_color_set' id='edit_eula_terms_background_color_set' class='jscolor " + jscolor_attr + " input_12_table auto_add_onblur' style='cursor:pointer;' value='232E32' type='text' maxlength='6' autocorrect='off' autocapitalize='off'>";
	code += "</div>";
	code += "</div>";
	code += "<div style='clear:both;'></div>";

	code += gen_sort_level("component_eula");

	code += "<div class='edit_action_bg'>";
	code += "<div class='captive_portal_adv_line'></div>";
	code += "<input class='button_gen' onclick='edit_clear_content(\"eula\");' type='button' value='<#CTL_clear#>'/>";
	//code += "<input class='button_gen' onclick='edit_update_content_eula();' type='button' value='<#CTL_apply#>'/>";
	code += "</div>";

	$("#splash_page_container_edit").html(code);

	if(component_array["component_eula"]) {
		var font_color = component_array["component_eula"][0].attribute.style_color;
		var terms_font_color = component_array["component_eula"][0].eula_terms_service_font_color;
		var terms_background_color = component_array["component_eula"][0].eula_terms_service_background_color;
		$("input[name=edit_eula_label_color_set]").val(font_color);
		$("input[name=edit_eula_terms_service_title]").val(encode_decode_text(component_array["component_eula"][0].eula_terms_service_title, "decode"));
		$('textarea#edit_eula_terms_service').val(encode_decode_text(component_array["component_eula"][0].eula_terms_service, "decode"));
		$("#edit_component_level").html(component_array["component_eula"][0].attribute.style_z_index);
		$("input[name=edit_eula_terms_color_set]").val(terms_font_color);
		$("input[name=edit_eula_terms_background_color_set]").val(terms_background_color);

		var eula_text = "I have read and agree to";
		var eula_hyperlink = "the Terms of Service";
		if(component_array["component_eula"][0].eula_terms_service_text == undefined)
			component_array["component_eula"][0].eula_terms_service_text = encode_decode_text(eula_text, "encode");
		if(component_array["component_eula"][0].eula_terms_service_hyperlink == undefined)
			component_array["component_eula"][0].eula_terms_service_hyperlink = encode_decode_text(eula_hyperlink, "encode");
		if(component_array["component_eula"][0].eula_terms_service_hyperlink_color == undefined)
			component_array["component_eula"][0].eula_terms_service_hyperlink_color = "#00B0FF";
		$("input[name=edit_eula_text_label]").val(encode_decode_text(component_array["component_eula"][0].eula_terms_service_text, "decode"));
		$("input[name=edit_eula_hyperlink_label]").val(encode_decode_text(component_array["component_eula"][0].eula_terms_service_hyperlink, "decode"));
		$("input[name=edit_eula_hyperlink_label_color_set]").val(component_array["component_eula"][0].eula_terms_service_hyperlink_color);
	}

	auto_add_onblur_event("eula");

	jsc.register();
}
function edit_update_content_account() {
	remove_hint_msg();
	$("input[name=edit_account_username_label]").val($.trim($("input[name=edit_account_username_label]").val()));
	$("input[name=edit_account_password_label]").val($.trim($("input[name=edit_account_password_label]").val()));
	var account_auth_type = "1";

	var account_username = $('input[name="edit_account_username_label"]').val();
	var account_password = $('input[name="edit_account_password_label"]').val();

	var font_color = transformHEXtoRGB("#" + $("#edit_account_label_color_set").val());
	font_color = font_color.replace(')', ', 0.8)').replace('rgb', 'rgba');

	var input_box_background_color = transformHEXtoRGB("#" + $("#edit_account_box_color_set").val());
	var input_box_opacity = parseFloat($("#edit_account_opacity").attr('title')) / 100;
	input_box_background_color = input_box_background_color.replace(')', ', ' + input_box_opacity + ')').replace('rgb', 'rgba');

	var code = "";
	if(account_username == "") {
		add_hint_msg($('input[name="edit_account_username_label"]'), "<#JS_fieldblank#>");
		$('input[name="edit_account_username_label"]').focus();
		return;
	}
	else if(account_password == "") {
		add_hint_msg($('input[name="edit_account_password_label"]'), "<#JS_fieldblank#>");
		$('input[name="edit_account_password_label"]').focus();
		return;
	}
	else {
		code += "<div style='height:100%;width:100%;'>";

		code += "<div style='background-color:" + input_box_background_color + ";color:" + font_color + ";";
		code += "height:30px;border-radius:4px;padding-left:2%;line-height:30px;font-style:oblique;'>";
		code += "" + htmlEnDeCode.htmlEncode(account_username) + "</div>";

		code += "<div style='background-color:" + input_box_background_color + ";color:" + font_color + ";";
		code += "height:30px;border-radius:4px;padding-left:2%;line-height:30px;font-style:oblique;margin-top:6px;'>";
		code += "" + htmlEnDeCode.htmlEncode(account_password) + "</div>";

		code += "</div>";
	}
		
	component_array["component_account"] = [ { 
		"attribute" : set_component_attribute("component_account"), 
		"account_auth_type" : account_auth_type, 
		"account_username" : encode_decode_text(account_username, "encode"),
		"account_password" : encode_decode_text(account_password, "encode"),
		"account_input_box_background_color" : $("#edit_account_box_color_set").val(),
		"account_input_box_font_color" : $("#edit_account_label_color_set").val(),
		"account_input_box_opacity" : input_box_opacity
	 } ];

	$("#component_account div").first().replaceWith(code);
}
function edit_component_account() {
	component_click = "component_account";
	toolbar_click = "toolbar_editor";

	reset_component_css();
	reset_toolbar_css();

	set_component_click_attr(component_click);

	var code = "";
	$("#splash_page_container_edit").html(code);

	code += "<div id='edit_account_accpwd_bg'>";
	code += "<div class='edit_component_item_title'><#Captive_Portal_Username_Hint#></div>";
	code += "<div class='edit_component_item_content'>";
	code += "<input name='edit_account_username_label' class='input_30_table auto_add_onblur' value='Enter Account' type='text' maxlength='32' autocorrect='off' autocapitalize='off'>";
	code += "</div>";

	code += "<div class='edit_component_item_title'><#Captive_Portal_Password_Hint#></div>";
	code += "<div class='edit_component_item_content'>";
	code += "<input name='edit_account_password_label' class='input_30_table auto_add_onblur' value='Enter Password' type='text' maxlength='32' autocorrect='off' autocapitalize='off'>";
	code += "</div>";
	code += "</div>";

	code += "<div class='captive_portal_adv_line'></div>";

	code += "<div class='edit_component_item_title'><#Captive_Portal_Input_Text_Color#></div>";
	code += "<div class='edit_component_item_content'>";
	code += "<input name='edit_account_label_color_set' id='edit_account_label_color_set' class='jscolor " + jscolor_attr + " input_12_table auto_add_onblur' style='cursor:pointer;' value='000000' type='text' maxlength='6' autocorrect='off' autocapitalize='off'>";
	code += "</div>";

	code += "<div class='edit_component_item_title'><#Captive_Portal_Input_Box_Color#></div>";
	code += "<div class='edit_component_item_content'>";
	code += "<input name='edit_account_box_color_set' id='edit_account_box_color_set' class='jscolor " + jscolor_attr + " input_12_table auto_add_onblur' style='cursor:pointer;' value='FFFFFF' type='text' maxlength='6' autocorrect='off' autocapitalize='off'>";
	code += "</div>";
	
	code += gen_slider("account");

	code += gen_sort_level("component_account");

	code += "<div class='edit_action_bg'>";
	code += "<div class='captive_portal_adv_line'></div>";
	code += "<input class='button_gen' onclick='edit_clear_content(\"account\");' type='button' value='<#CTL_clear#>'/>";
	//code += "<input class='button_gen' onclick='edit_update_content_account();' type='button' value='<#CTL_apply#>'/>";
	code += "</div>";

	$("#splash_page_container_edit").html(code);

	if(component_array["component_account"]) {
		var font_color = component_array["component_account"][0].account_input_box_font_color;
		$("input[name=edit_account_label_color_set]").val(font_color);
		var background_color = component_array["component_account"][0].account_input_box_background_color;
		$("input[name=edit_account_box_color_set]").val(background_color);
		var input_box_opacity = parseFloat(component_array["component_account"][0].account_input_box_opacity) * 100;
		$("#edit_account_opacity").attr('title', input_box_opacity);
		$( "#edit_account_opacity_text" ).html(input_box_opacity + "%");
		$("#edit_component_level").html(component_array["component_account"][0].attribute.style_z_index);

		if(component_array["component_account"][0].account_username == "")
			component_array["component_account"][0].account_username = encode_decode_text("Enter Account", "encode");
		if(component_array["component_account"][0].account_password == "")
			component_array["component_account"][0].account_password = encode_decode_text("Enter Password", "encode");
		$("input[name=edit_account_username_label]").val(encode_decode_text(component_array["component_account"][0].account_username, "decode"));
		$("input[name=edit_account_password_label]").val(encode_decode_text(component_array["component_account"][0].account_password, "decode"));

		$("#component_account").css("height", "66px");
		$("#component_account").css("max-height", "66px");

		edit_update_content_account();
	}

	jsc.register();

	auto_add_onblur_event("account");

	register_slider("account");
}
function edit_update_content_button() {
	remove_hint_msg();
	var background_color = "#" + $("#edit_button_color_set").val();
	var font_color = "#" + $("#edit_button_label_color_set").val();

	$("input[name=edit_button_label]").val($.trim($("input[name=edit_button_label]").val()));
	$("input[name=edit_button_work_progress]").val($.trim($("input[name=edit_button_work_progress]").val()));
	$("input[name=edit_button_denied]").val($.trim($("input[name=edit_button_denied]").val()));

	var botton_label = $('input[name="edit_button_label"]').val();
	var button_work_progress = $('input[name="edit_button_work_progress"]').val();
	var button_denied = $('input[name="edit_button_denied"]').val();

	if(botton_label == "") {
		add_hint_msg($('input[name="edit_button_label"]'), "<#JS_fieldblank#>");
		$('input[name="edit_button_label"]').focus();
		return;
	}
	else {
		$("#component_button").css({"background-color" : background_color});
		$("#component_button").css({"color" : font_color});
		
		component_array["component_button"] = [ { 
			"attribute" : set_component_attribute("component_button"), 
			"button_label" : encode_decode_text(botton_label, "encode"),
			"button_work_progress" : encode_decode_text(button_work_progress, "encode"),
			"button_denied" : encode_decode_text(button_denied, "encode")
		 } ];
		
		var button_height = component_array["component_button"][0].attribute.style_height;
		var code = "<div style='height:100%;width:100%;'>";
		code += "<div style='height:" + button_height + ";line-height:" + button_height + ";text-align:center;overflow:hidden;'>";
		code += htmlEnDeCode.htmlEncode($('input[name="edit_button_label"]').val());
		code += "</div>";
		code += "</div>";

		$("#component_button div").first().replaceWith(code);
	}
}
function edit_component_button() {
	component_click = "component_button";
	toolbar_click = "toolbar_editor";
	
	reset_component_css();
	reset_toolbar_css();

	set_component_click_attr(component_click);

	var code = "";
	$("#splash_page_container_edit").html(code);

	code += "<div class='edit_component_item_title'><#Captive_Portal_Button_Label#></div>";
	code += "<div class='edit_component_item_content'>";
	code += "<input name='edit_button_label' class='input_30_table auto_add_onblur' value='Connect' type='text' maxlength='16' autocorrect='off' autocapitalize='off'>";
	code += "</div>";

	code += "<div class='edit_component_item_title'><#Captive_Portal_Button_Color#></div>";
	code += "<div class='edit_component_item_content'>";
	code += "<input name='edit_button_color_set' id='edit_button_color_set' class='jscolor " + jscolor_attr + " input_12_table auto_add_onblur' style='cursor:pointer;' value='00b0ff' type='text' maxlength='6' autocorrect='off' autocapitalize='off'>";
	code += "</div>";

	code += "<div class='edit_component_item_title'><#Captive_Portal_Button_Text_Color#></div>";
	code += "<div class='edit_component_item_content'>";
	code += "<input name='edit_button_label_color_set' id='edit_button_label_color_set' class='jscolor " + jscolor_attr + " input_12_table auto_add_onblur' style='cursor:pointer;' value='ffffff' type='text' maxlength='6' autocorrect='off' autocapitalize='off'>";
	code += "</div>";

	code += gen_sort_level("component_button");

	code += "<div class='captive_portal_adv_line'></div>";

	code += "<div class='edit_component_item_title'><#Captive_Portal_Work_Text#></div>";
	code += "<div class='edit_component_item_content'>";
	code += "<input name='edit_button_work_progress' class='input_30_table auto_add_onblur' value='Connecting, please be patient.' type='text' maxlength='128' autocorrect='off' autocapitalize='off'>";
	code += "</div>";

	code += "<div class='edit_component_item_title'><#Captive_Portal_Denied_Text#></div>";
	code += "<div class='edit_component_item_content'>";
	code += "<input name='edit_button_denied' class='input_30_table auto_add_onblur' value='Error: Invalid credentials. please contact your system administrator' type='text' maxlength='128' autocorrect='off' autocapitalize='off'>";
	code += "</div>";

	code += "<div class='edit_action_bg'>";
	code += "<div class='captive_portal_adv_line'></div>";
	code += "<input class='button_gen' onclick='edit_clear_content(\"button\");' type='button' value='<#CTL_clear#>'/>";
	//code += "<input class='button_gen' onclick='edit_update_content_button();' type='button' value='<#CTL_apply#>'/>";
	code += "</div>";

	$("#splash_page_container_edit").html(code);

	if(component_array["component_account"]) {
		$("input[name=edit_button_denied]").val("Error: Invalid credentials. please contact your system administrator.");
	}

	if(component_array["component_button"]) {
		var background_color = component_array["component_button"][0].attribute.style_background_color;
		if(background_color.search("rgb") != -1) {
			background_color = transformRGBtoHEX(background_color);
		}
		var font_color = component_array["component_button"][0].attribute.style_color;
		$("input[name=edit_button_color_set]").val(background_color);
		$("input[name=edit_button_label]").val(encode_decode_text(component_array["component_button"][0].button_label, "decode"));
		$("input[name=edit_button_label_color_set]").val(font_color);
		$("input[name=edit_button_work_progress]").val(encode_decode_text(component_array["component_button"][0].button_work_progress, "decode"));
		$("input[name=edit_button_denied]").val(encode_decode_text(component_array["component_button"][0].button_denied, "decode"));
		$("#edit_component_level").html(component_array["component_button"][0].attribute.style_z_index);
	}

	jsc.register();

	auto_add_onblur_event("button");
}
function showHideBackgroundType() {
	var radio_value = $("input[name=edit_background_type]:checked").val();
	$("#edit_background_image_bg").css("display", "none");
	$("#edit_background_color_bg").css("display", "none");
	if(radio_value == "image") {
		$("#edit_background_image_bg").css("display", "");
	}
	else {
		$("#edit_background_color_bg").css("display", "");
	}
}
function edit_upload_file_trigger(_component) {
	$( "#edit_upload_file").unbind("change");
	$("#edit_upload_file").change( function() {
		edit_upload_file_event($("#edit_upload_file")[0], _component);
	});
	$("#edit_upload_file").val('');
	$("#edit_upload_file").click();
}
function edit_upload_file_event(_obj, _component) {
	var _mimeType = _obj.files[0].type;
	//1.check image
	if (!checkImageMimeType(_mimeType)) {
		alert("<#Setting_upload_hint#>");
		_obj.focus();
	}
	else {
		var canvas_width = 1152;
		var canvas_height = 864;
		if (_component.indexOf("image") >= 0) {
			//canvas_width = parseInt($("#component_" + _component + "").css("width"), 10);
			//canvas_height = parseInt($("#component_" + _component + "").css("height"), 10);
			canvas_width = 300;
			canvas_height = 300;
		}
		$("#edit_redraw_canvas")[0].width = canvas_width;
		$("#edit_redraw_canvas")[0].height = canvas_height;

		//2.Re-drow image
		var fileReader = new FileReader(); 
		fileReader.onload = function (fileReader) {
			var source_image_size = 0;
			if( (fileReader.total != undefined) && (!isNaN(fileReader.total)) )
				source_image_size = fileReader.total;
			if(Math.round(source_image_size / 1024) > 10240) {
				alert('Warning: The upload file size exceeds the allowable 10MB limit. Please select another image.');/*untranslated*/
				return false;
			}

			var img = document.createElement("img");
			img.src = fileReader.target.result;
			var mimeType = img.src.split(",")[0].split(":")[1].split(";")[0];
			var canvas = document.getElementById("edit_redraw_canvas");
			var ctx = canvas.getContext("2d");
			ctx.clearRect(0, 0, canvas_width, canvas_height);
			setTimeout(function() {
				ctx.drawImage(img, 0, 0, canvas_width, canvas_height);
				var dataURL = canvas.toDataURL(mimeType);
				if(Math.round(dataURL.length / 1024) > 2048) {
					alert('Warning: The upload file size exceeds the allowable 2MB limit. Please select another image.\nNote: Sometime the smaller PNG image will take fewer megabytes and exceed 2MB limit after resizing the image.');/*untranslated*/
					return false;
				}
				else {
					edit_image_base64_array[_component] = dataURL;
					if (_component.indexOf("image") >= 0) {
						var _component_id = _component.split("_")[1];
						edit_update_content_image(_component_id);
					}
					else
						edit_update_content_background();
				}
			}, 100); //for firefox FPS(Frames per Second) issue need delay
		}
		fileReader.readAsDataURL(_obj.files[0]);
	}
}
function edit_update_content_image(_component_id) {
	remove_hint_msg();
	var image_opacity = parseInt($( "#edit_image_opacity" ).attr('title')) / 100;

	var image_attr_array = function() {
		this.attribute = "";
		this.image_base64 = "";
		this.image_type = "";
		this.image_walled_garden = "";
	};

	if(component_array["component_image"] == undefined)
		component_array["component_image"] = [];

	$("input[name=edit_image_walled_garden]").val($.trim($("input[name=edit_image_walled_garden]").val()));
	var image_walled_garden = $('input[name="edit_image_walled_garden"]').val();
	if(image_walled_garden != "") {
		var validIPFlag = false;
		if(image_walled_garden.split(".").length == 4) {
			var part = image_walled_garden.split(".");
			if(!isNaN(part[0]) && !isNaN(part[1]) && !isNaN(part[2]) && !isNaN(part[3]))
				validIPFlag = true;
		}
		if(validIPFlag) {
			if(!validator.ipv4_addr(image_walled_garden)) {
				add_hint_msg($('input[name="edit_image_walled_garden"]'), "" + image_walled_garden + " <#JS_validip#>");
				$("input[name=edit_image_walled_garden]").focus();
				return;
			}
		}
		else {
			if(!validator.domainName_flag(image_walled_garden)) {
				add_hint_msg($('input[name="edit_image_walled_garden"]'), "" + image_walled_garden + " is invalid Domain Name");/*untranslated*/
				$("input[name=edit_image_walled_garden]").focus();
				return;
			}
		}
	}

	if($("input[name=edit_image_type]:checked").val() == "image") {
		if(edit_image_base64_array["image_" + _component_id]) {
			var _edit_css = "background-image:url(" + edit_image_base64_array["image_" + _component_id] + ");";
			_edit_css += "background-repeat: no-repeat;";
			_edit_css += "background-size: 100% 100%;";
			_edit_css += "opacity: " + image_opacity + ";";
			var html = "<div style='height:100%;width:100%;overflow:auto;" + _edit_css + ";border-radius:8px;'></div>";
			$("#component_image_" + _component_id  + " div").first().replaceWith(html);

			component_array["component_image"][_component_id] = new image_attr_array();
			component_array["component_image"][_component_id].attribute = set_component_attribute("component_image_" + _component_id  + "");
			component_array["component_image"][_component_id].attribute.style_opacity = image_opacity;
			component_array["component_image"][_component_id].image_base64 = edit_image_base64_array["image_" + _component_id];
			component_array["component_image"][_component_id].image_type = "image";
			component_array["component_image"][_component_id].image_walled_garden = encode_decode_text(image_walled_garden, "encode");
		}
	}
	else {
		delete edit_image_base64_array["image_" + _component_id];
		var background_color = "#" + $("#edit_image_color_set").val();
		var _edit_css = "background-color: " + background_color + ";";
		_edit_css += "opacity: " + image_opacity + ";";
		var html = "<div style='height:100%;width:100%;overflow:auto;" + _edit_css + ";border-radius:8px;'></div>";
		$("#component_image_" + _component_id  + " div").first().replaceWith(html);

		component_array["component_image"][_component_id] = new image_attr_array();
		component_array["component_image"][_component_id].attribute = set_component_attribute("component_image_" + _component_id  + "");
		component_array["component_image"][_component_id].attribute.style_background_color = transformHEXtoRGB(background_color);
		component_array["component_image"][_component_id].attribute.style_opacity = image_opacity;
		component_array["component_image"][_component_id].image_base64 = "";
		component_array["component_image"][_component_id].image_type = "color";
		component_array["component_image"][_component_id].image_walled_garden = encode_decode_text(image_walled_garden, "encode");
	}
}
function edit_update_content_background() {
	var background_opacity = parseInt($( "#edit_background_opacity" ).attr('title')) / 100;

	if($("input[name=edit_background_type]:checked").val() == "image") {
		if(edit_image_base64_array["background"]) {
			var _edit_css = "background-image:url(" + edit_image_base64_array["background"]  + ");";
			_edit_css += "background-repeat: no-repeat;";
			_edit_css += "background-size: 250%;";
			_edit_css += "background-position: 50% 50%;";
			_edit_css += "opacity: " + background_opacity + ";";
			var code = "<div style='border-radius:6px;height:100%;width:100%;overflow:auto;" + _edit_css + "'></div>";
			$("#component_background").html(code);

			component_array["component_background"] = [ { "attribute" : set_component_attribute("component_background"), "image_type" : "image", "image_base64" : edit_image_base64_array["background"] } ];
			component_array["component_background"][0].attribute.style_opacity = background_opacity;
		}
	}
	else {
		delete edit_image_base64_array["background"];
		var background_color = "#" + $("#edit_background_color_set").val();
		var _edit_css = "background-color: " + background_color + ";";
		_edit_css += "opacity: " + background_opacity + ";";
		var code = "<div style='border-radius:6px;height:100%;width:100%;overflow:auto;" + _edit_css + "'></div>";
		$("#component_background").html(code);

		component_array["component_background"] = [ { "attribute" : set_component_attribute("component_background"), "image_type" : "color", "image_base64" : "" } ];
		component_array["component_background"][0].attribute.style_background_color = transformHEXtoRGB(background_color);
		component_array["component_background"][0].attribute.style_opacity = background_opacity;
	}
}
function edit_component_color_callback(jscolor) {
	var _component_type = component_click;
	var _component_type_id = "";
	if (_component_type.indexOf("image") >= 0) {
		_component_type = "image";
		_component_type_id = component_click.replace("component_image_", "");
	}
	else if (_component_type.indexOf("text") >= 0) {
		_component_type = "text";
		_component_type_id = component_click.replace("component_text_", "");
	}
	else {
		_component_type = component_click.replace("component_", "");
	}
	switch(_component_type) {
		case "image" :
			edit_update_content_image(_component_type_id);
			break;
		case "text" :
			edit_update_content_text(_component_type_id);
			break;
		case "background" :
			edit_update_content_background();
			break;
		case "button" :
			edit_update_content_button();
			break;
		case "account" :
			edit_update_content_account();
			break;
		case "eula" :
			edit_update_content_eula();
			break;
	}
}
function edit_component_background() {
	component_click = "component_background";
	toolbar_click = "toolbar_background";

	reset_component_css();
	reset_toolbar_css();

	set_component_click_attr(component_click);
	
	var code = "";
	$("#splash_page_container_edit").html(code);

	code += "<div class='edit_component_item_title'>";
	code += "<input type='radio' name='edit_background_type' id='edit_background_image' value='image' onclick='showHideBackgroundType();' checked><label for='edit_background_image'><#Captive_Portal_Background_Image#></label>";
	code += "<input type='radio' name='edit_background_type' id='edit_background_color' value='color' onclick='showHideBackgroundType();'><label for='edit_background_color'><#Captive_Portal_Background_Color#></label>";
	code += "</div>";

	code += "<div class='captive_portal_adv_line'></div>";
	
	code += "<div id='edit_background_image_bg' class='edit_background_bg'>";
	if(isSupportFileReader() && isSupportCanvas()) {
		code += "<div id='edit_drag_drop_bg' class='edit_drag_drop_bg' onclick='edit_upload_file_trigger(\"background\");'>";
		code += "<div class='edit_drag_drop_text'><#Captive_Portal_Upload_Image#><br><#Captive_Portal_OR#><br><#Captive_Portal_Click_Image#></div>";
		code += "</div>";
		code += "<div class='edit_drag_drop_hint'><#FreeWiFi_ImageSize#>: < 10MB</div>";
		code += "<div class='edit_drag_drop_hint'><#FreeWiFi_RecommendType#>: jpg, png</div>";
		code += "<div class='edit_drag_drop_hint'><#FreeWiFi_RecommendResolution#>: 1152 x 864 px <#Captive_Portal_OR#> <#Captive_Portal_Above#></div>";
	}
	else {
		code += "<div id='edit_drag_drop_bg' class='edit_drag_drop_bg'>";
		code += "<div class='edit_drag_drop_text'>Your browser doesn't support html5</div>";/*untranslated*/
		code += "</div>";
	}
	code += "<canvas id='edit_redraw_canvas' style='display:none;'></canvas>";
	//code += "<input type='file' name='edit_upload_file' id='edit_upload_file' class='edit_upload_file' onchange='edit_upload_file_event(this, \"background\");'/>";
	code += "</div>";

	code += "<div id='edit_background_color_bg' class='edit_background_bg' style='display:none;'>";
		code += "<div class='edit_component_item_title'><#Captive_Portal_Selected_Color#></div>";
		code += "<div class='edit_component_item_content'><input name='edit_background_color_set' id='edit_background_color_set' class='jscolor " + jscolor_attr + " input_12_table auto_add_onblur' style='cursor:pointer;' value='616161' type='text' maxlength='6' autocorrect='off' autocapitalize='off'></div>";
	code += "</div>";

	code += gen_slider("background");

	code += "<div class='edit_action_bg'>";
	code += "<div class='captive_portal_adv_line'></div>";
	code += "<input class='button_gen' onclick='edit_clear_content(\"background\");' type='button' value='<#CTL_clear#>'/>";
	code += "</div>";

	$("#splash_page_container_edit").html(code);
	
	if(component_array["component_background"]) {
		if(component_array["component_background"][0].image_type == "image") {
			edit_image_base64_array["background"] = component_array["component_background"][0].image_base64;
			settingRadioItemCheck($("input[name=edit_background_type]"), "image");
			showHideBackgroundType();
			
		}
		else {
			var background_color = component_array["component_background"][0].attribute.style_background_color;
			if(background_color.search("rgb") != -1) {
				background_color = transformRGBtoHEX(background_color);
			}
			$("input[name=edit_background_color_set]").val(background_color);
			settingRadioItemCheck($("input[name=edit_background_type]"), "color");
			showHideBackgroundType();
		}
	}

	register_drag_drop("background");

	jsc.register();

	auto_add_onblur_event("background");

	register_slider("background");
}
function showHideImageType() {
	var radio_value = $("input[name=edit_image_type]:checked").val();
	$("#edit_image_bg").css("display", "none");
	$("#edit_color_bg").css("display", "none");
	if(radio_value == "image") {
		$("#edit_image_bg").css("display", "");
	}
	else {
		$("#edit_color_bg").css("display", "");
	}
}
function edit_component_image(_idx) {
	component_click = "component_image_" + _idx;
	toolbar_click = "toolbar_editor";

	reset_component_css();
	reset_toolbar_css();

	set_component_click_attr(component_click);
	
	var code = "";
	$("#splash_page_container_edit").html(code);

	code += "<div class='edit_component_item_title'>";
	code += "<input type='radio' name='edit_image_type' id='edit_image_image' value='image' onclick='showHideImageType();' checked><label for='edit_image_image'><#Captive_Portal_Image#></label>";
	code += "<input type='radio' name='edit_image_type' id='edit_image_color' value='color' onclick='showHideImageType();'><label for='edit_image_color'><#Captive_Portal_Color#></label>";
	code += "</div>";

	code += "<div class='captive_portal_adv_line'></div>";

	code += "<div id='edit_image_bg' class='edit_background_bg'>";
	if(isSupportFileReader() && isSupportCanvas()) {
		code += "<div id='edit_drag_drop_bg' class='edit_drag_drop_bg edit_drag_drop_bg_for_image' onclick='edit_upload_file_trigger(\"image_" + _idx + "\");'>";
		code += "<div class='edit_drag_drop_text edit_drag_drop_text_for_image'><#Captive_Portal_Upload_Image#><br><#Captive_Portal_OR#><br><#Captive_Portal_Click_Image#></div>";
		code += "</div>";
	}
	else {
		code += "<div id='edit_drag_drop_bg' class='edit_drag_drop_bg edit_drag_drop_bg_for_image'>";
		code += "<div class='edit_drag_drop_text edit_drag_drop_text_for_image'>Your browser doesn't support html5</div>";/*untranslated*/
		code += "</div>";
	}
	code += "<canvas id='edit_redraw_canvas' style='display:none;'></canvas>";
	//code += "<input type='file' name='edit_upload_file' id='edit_upload_file' class='edit_upload_file' onchange='edit_upload_file_event(this, \"image_" + _idx + "\");'/>";
	code += "</div>";

	code += "<div id='edit_color_bg' class='edit_background_bg' style='display:none;'>";
		code += "<div class='edit_component_item_title'><#Captive_Portal_Selected_Color#></div>";
		code += "<div class='edit_component_item_content'><input name='edit_image_color_set' id='edit_image_color_set' class='jscolor " + jscolor_attr + " input_12_table auto_add_onblur' style='cursor:pointer;' value='FFFFFF' type='text' maxlength='6' autocorrect='off' autocapitalize='off'></div>";
	code += "</div>";

	code += "<div class='edit_component_item_title'><a style='cursor:help;color:#D0D6D8' href='javascript:void(0);' onClick='openHint(31, 5);'><#Captive_Portal_Web_Address#></a></div>";
	code += "<div class='edit_component_item_content'>";
	code += "<span class='edit_component_item_title'>http://</span><input name='edit_image_walled_garden' class='input_30_table auto_add_onblur' style='width:284px' value='' type='text' maxlength='64' autocorrect='off' autocapitalize='off'>";
	code += "<span class='input_item_hint'>ex. www.asus.com</span>";

	code += gen_slider("image");

	code += gen_sort_level("component_image_" + _idx);

	$("#splash_page_container_edit").html(code);
	
	if(component_array["component_image"]) {
		if(component_array["component_image"][_idx]) {
			if(component_array["component_image"][_idx].image_type == "image") {
				edit_image_base64_array["image_" + _idx + ""] = component_array["component_image"][_idx].image_base64;
				settingRadioItemCheck($("input[name=edit_image_type]"), "image");
				showHideImageType();
			}
			else {
				var background_color = component_array["component_image"][_idx].attribute.style_background_color;
				if(background_color.search("rgb") != -1) {
					background_color = transformRGBtoHEX(background_color);
				}
				$("input[name=edit_image_color_set]").val(background_color);
				settingRadioItemCheck($("input[name=edit_image_type]"), "color");
				showHideImageType();
			}
			$("#edit_component_level").html(component_array["component_image"][_idx].attribute.style_z_index);
			if(component_array["component_image"][_idx].image_walled_garden) {
				$('input[name="edit_image_walled_garden"]').val(encode_decode_text(component_array["component_image"][_idx].image_walled_garden, "decode"));
			}
		}
	}

	register_drag_drop("image_" + _idx);

	jsc.register();

	auto_add_onblur_event("image");

	register_slider("image_" + _idx);
}
function edit_update_content_text(_component_id) {
	remove_hint_msg();
	var text_attr_array = function() {
		this.attribute = "";
		this.text_content = "";
		this.text_font_weight = "initial";
		this.text_font_style = "initial";
		this.text_font_family = "Arial";
		this.text_font_size = "12";
		this.text_font_color = "#000000";
		this.text_walled_garden = "";
	};

	var text_font_weight = ($("#edit_text_tool_bolder").hasClass("click")) ? "bolder" : "initial" ;
	var text_font_style = ($("#edit_text_tool_oblique").hasClass("click")) ? "oblique" : "initial" ;
	var text_font_family = $("#edit_text_tool_font_family").val();
	var text_font_size = $("#edit_text_tool_combine_input").val();
	var text_font_color = "#" + $("#edit_text_color_set").val();
	$("input[name=edit_text_walled_garden]").val($.trim($("input[name=edit_text_walled_garden]").val()));
	var text_walled_garden = $('input[name="edit_text_walled_garden"]').val();
	if(text_walled_garden != "") {
		var validIPFlag = false;
		if(text_walled_garden.split(".").length == 4) {
			var part = text_walled_garden.split(".");
			if(!isNaN(part[0]) && !isNaN(part[1]) && !isNaN(part[2]) && !isNaN(part[3]))
				validIPFlag = true;
		}
		if(validIPFlag) {
			if(!validator.ipv4_addr(text_walled_garden)) {
				add_hint_msg($('input[name="edit_text_walled_garden"]'), "" + text_walled_garden + " <#JS_validip#>");
				$("input[name=edit_text_walled_garden]").focus();
				return;
			}
		}
		else {
			if(!validator.domainName_flag(text_walled_garden)) {
				add_hint_msg($('input[name="edit_text_walled_garden"]'), "" + text_walled_garden + " is invalid Domain Name");/*untranslated*/
				$("input[name=edit_text_walled_garden]").focus();
				return;
			}
		}
	}
	var text_decoration = (text_walled_garden == "") ? "none" : "underline";

	$("#component_text_" + _component_id  + "").css({"color" : text_font_color});

	var text_content = $("#edit_text_content").val();
	text_content = htmlEnDeCode.htmlEncode(text_content);
	text_content = text_content.replace(/ /g, '&nbsp;').replace(/(?:\r\n|\r|\n)/g, '<br>');

	if(component_array["component_text"] == undefined)
		component_array["component_text"] = [];

	var html = "<div style='height:100%;width:100%;overflow:auto;font-size:" + text_font_size + "px;font-weight:" + text_font_weight  + ";font-style:" + text_font_style + ";font-family:" + text_font_family + ";'>";
	html +='<div style=\'text-decoration:' + text_decoration + ';\'>';
	html += text_content;
	html +='</div>';
	html += "</div>";
	$("#component_text_" + _component_id  + " div").first().replaceWith(html);

	component_array["component_text"][_component_id] = new text_attr_array();
	component_array["component_text"][_component_id].attribute = set_component_attribute("component_text_" + _component_id  + "");
	component_array["component_text"][_component_id].text_content = encode_decode_text($("#edit_text_content").val(), "encode");
	component_array["component_text"][_component_id].text_font_weight = text_font_weight;
	component_array["component_text"][_component_id].text_font_style = text_font_style
	component_array["component_text"][_component_id].text_font_family = text_font_family;
	component_array["component_text"][_component_id].text_font_size = text_font_size;
	component_array["component_text"][_component_id].text_walled_garden = encode_decode_text(text_walled_garden, "encode");
}
function edit_text_tool_combine_select_change(_component_id) {
	$('#edit_text_tool_combine_input').val($('#edit_text_tool_combine_select').val());
	edit_update_content_text(_component_id);
}
function edit_text_tool_combine_input_blur(_component_id) {
	if(!validator.numberRange($('#edit_text_tool_combine_input')[0], 12, 72)){
		return false;
	}
	$('#edit_text_tool_combine_select').val($('#edit_text_tool_combine_input').val());
	edit_update_content_text(_component_id);
}
function edit_text_tool_font_family_change(_component_id) {
	edit_update_content_text(_component_id);
}
function edit_text_tool_style(_type, _component_id) {
	$("#edit_text_tool_" + _type + "").removeClass("component_bg_click");

	if($("#edit_text_tool_" + _type + "").hasClass("click")) {
		$("#edit_text_tool_" + _type + "").removeClass("click");
	}
	else {
		$("#edit_text_tool_" + _type + "").addClass("click");
	}
	edit_update_content_text(_component_id);
}
function edit_component_text(_idx) {
	component_click = "component_text_" + _idx;
	toolbar_click = "toolbar_editor";

	reset_component_css();
	reset_toolbar_css();

	set_component_click_attr(component_click);
	
	var code = "";
	$("#splash_page_container_edit").html(code);

	code += "<div class='edit_component_item_title'>";
	code += "<#Captive_Portal_Font_Style#>";
	code += "</div>";

	code += "<div class='edit_component_item_content'>";
	code += "<div id='edit_text_tool_bolder' class='edit_text_tool edit_text_tool_bolder' title='Boldface' onclick='edit_text_tool_style(\"bolder\", \"" + _idx + "\");'>B</div>";/*untranslated*/
	code += "<div id='edit_text_tool_oblique' class='edit_text_tool edit_text_tool_oblique' title='Italics' onclick='edit_text_tool_style(\"oblique\", \"" + _idx + "\");'>I</div>";/*untranslated*/
	code += "<div class='edit_text_tool_family'>";
	code += "<select id='edit_text_tool_font_family' class='edit_text_tool_font_family' onchange='edit_text_tool_font_family_change(\"" + _idx + "\");'>";
	var font_family_array = ["Arial", "Tahoma", "Helvetica", "Comic Sans MS", "Georgia", "Time New Roman", "serif", "sans-serif", "cursive", "fantasy", "monospace"];
	for(var i = 0; i < font_family_array.length; i += 1) {
		code += "<option value='" + font_family_array[i] + "'>" + font_family_array[i] + "</option>";
	}
	code += "</select>";
	code += "</div>";
	code += "<div class='edit_text_tool_combine'>";
	code += "<select id='edit_text_tool_combine_select' class='edit_text_tool_combine_select' onchange='edit_text_tool_combine_select_change(\"" + _idx + "\");'>";
	var font_size_array = [12, 14, 16, 18, 20, 22, 24, 26, 28, 36, 48, 72];
	for(var i = 0; i < font_size_array.length; i += 1) {
		code += "<option value='" + font_size_array[i] + "'>" + font_size_array[i] + "</option>";
	}
	code += "</select>";
	code += "<input id='edit_text_tool_combine_input' name='edit_text_tool_combine_input' class='edit_text_tool_combine_input' type='text' value='12' maxlength='2' onkeypress='return validator.isNumber(this,event)' autocorrect='off' autocapitalize='off' onblur='edit_text_tool_combine_input_blur(\"" + _idx + "\");'>";
	code += "</div>";
	code += "<textarea id='edit_text_content' class='edit_text_content auto_add_onblur' onkeyup='edit_update_content_text(\"" + _idx + "\");' maxlength='1024'></textarea>";
	code += "</div>";

	code += "<div class='edit_component_item_title'><a style='cursor:help;color:#D0D6D8' href='javascript:void(0);' onClick='openHint(31, 5);'><#Captive_Portal_Web_Address#></a></div>";
	code += "<div class='edit_component_item_content'>";
	code += "<span class='edit_component_item_title'>http://</span><input name='edit_text_walled_garden' class='input_30_table auto_add_onblur' style='width:284px' value='' type='text' maxlength='64' autocorrect='off' autocapitalize='off'>";
	code += "<span class='input_item_hint'>ex. www.asus.com</span>";

	code += "<div class='edit_component_item_title'><#Captive_Portal_Text_Color#></div>";
	code += "<div class='edit_component_item_content'>";
	code += "<input name='edit_text_color_set' id='edit_text_color_set' class='jscolor " + jscolor_attr + " input_12_table auto_add_onblur' style='cursor:pointer;' value='000000' type='text' maxlength='6' autocorrect='off' autocapitalize='off'>";
	code += "</div>";

	code += gen_sort_level("component_text_" + _idx);

	code += "<div class='edit_action_bg'>";
	code += "<div class='captive_portal_adv_line'></div>";
	code += "<input class='button_gen' onclick='edit_clear_content(\"text_" + _idx + "\");' type='button' value='<#CTL_clear#>'/>";
	//code += "<input class='button_gen' onclick='edit_update_content_text(\"" + _idx + "\");' type='button' value='<#CTL_apply#>'/>";
	code += "</div>";

	$("#splash_page_container_edit").html(code);
	
	if(component_array["component_text"]) {
		if(component_array["component_text"][_idx]) {
			var background_color = component_array["component_text"][_idx].attribute.style_background_color;
			if(background_color.search("rgb") != -1) {
				background_color = transformRGBtoHEX(background_color);
			}
			var font_color = component_array["component_text"][_idx].attribute.style_color;
			if(font_color.search("rgb") != -1) {
				font_color = transformRGBtoHEX(font_color);
			}
			var text_content = encode_decode_text(component_array["component_text"][_idx].text_content, "decode");
			$("#edit_text_content").val(text_content);
			$("#edit_component_level").html(component_array["component_text"][_idx].attribute.style_z_index);
			if(component_array["component_text"][_idx].text_font_weight == "bolder") {
				$("#edit_text_tool_bolder").addClass("click");
			}
			if(component_array["component_text"][_idx].text_font_style == "oblique") {
				$("#edit_text_tool_oblique").addClass("click");
			}
			$("#edit_text_tool_font_family").val(component_array["component_text"][_idx].text_font_family);
			$("#edit_text_tool_combine_input").val(component_array["component_text"][_idx].text_font_size);
			$("#edit_text_tool_combine_select").val(component_array["component_text"][_idx].text_font_size);
			$("input[name=edit_text_color_set]").val(font_color);
			if(component_array["component_text"][_idx].text_walled_garden) {
				$('input[name="edit_text_walled_garden"]').val(encode_decode_text(component_array["component_text"][_idx].text_walled_garden, "decode"));
			}
		}
	}

	jsc.register();

	auto_add_onblur_event("text");
}
function gen_splash_page_component(_component, _idx) {
	var code = "";
	switch(_component) {
		case "text" :
			code += "<div id='component_text_" + _idx + "' class='component_bg multi_text' style='left:10px;top:120px;position:absolute;height:180px;width:320px;min-width:100px;min-height:20px;z-index:1;' onclick='edit_component_text(" + _idx + ");'>";
			code += "<div class='component_title'><span>Text</span></div>";/*untranslated*/
			code += "<div class='component_del' onclick='edit_del_component(\"text_" + _idx + "\", event);' title='Delete this pattern.'></div>";/*untranslated*/
			code += "</div>";
			break;
		case "image" :
			code += "<div id='component_image_" + _idx + "' class='component_bg multi_image' style='left:10px;top:10px;position:absolute;height:100px;width:320px;min-width:60px;min-height:60px;z-index:1;' onclick='edit_component_image(" + _idx + ");'>";
			code += "<div class='component_title'><span><#Captive_Portal_Image#></span></div>";
			code += "<div class='component_del' onclick='edit_del_component(\"image_" + _idx + "\" , event);' title='Delete this pattern.'></div>";/*untranslated*/
			code += "</div>";
			break;
		case "eula" :
			code += "<div id='component_eula' class='component_bg' style='left:20px;top:310px;position:absolute;height:40px;width:300px;min-width:160px;min-height:40px;max-height:40px;z-index:1;' onclick='edit_component_eula();'>";
			code += "<div class='component_title'><span>EULA Check box</span></div>";/*untranslated*/
			code += "<div class='component_del' onclick='edit_del_component(\"eula\" , event);' title='Delete this pattern.'></div>";
			code += "</div>";
			break;
		case "account" :
			code += "<div id='component_account' class='component_bg' style='position:absolute;left:10px;top:360px;height:66px;width:320px;min-width:160px;min-height:30px;max-height:66px;max-width:340px;z-index:1;' onclick='edit_component_account();'>";
			code += "<div class='component_title'><span>Account / Password</span></div>";/*untranslated*/
			code += "<div class='component_del' onclick='edit_del_component(\"account\" , event);' title='Delete this pattern.'></div>";
			code += "</div>";
			break;
		case "button" :
			code += "<div id='component_button' class='component_bg' style='position:absolute;left:105px;top:450px;height:40px;width:130px;min-width:60px;min-height:40px;max-height:40px;max-width:340px;z-index:2;border-radius:8px;' onclick='edit_component_button();'>";
			code += "<div class='component_title'><span>Button</span></div>";/*untranslated*/
			code += "</div>";
			break;
		case "background" :
			code += "<div id='component_background' class='component_bg component_background' style='height:510px;width:340px;' onclick='edit_component_background();'></div>";
			break;
	}
	return code;
}
function register_drag_drop(_component) {
	//check browswer support File Reader and Canvas or not.
	if(isSupportFileReader() && isSupportCanvas()) {
		//Setting drop event
		var holder = document.getElementById("edit_drag_drop_bg");
		holder.ondragover = function () { return false; };
		holder.ondragend = function () { return false; };
		holder.ondrop = function (e) {
			e.preventDefault();
			var file = e.dataTransfer.files[0];

			//check image
			if(checkImageMimeType(file.type)) {
				var source_image_size = 0;
				if( (file.size != undefined) && (!isNaN(file.size)) )
					source_image_size = file.size;
				if(Math.round(source_image_size / 1024) > 10240) {
					alert('Warning: The upload file size exceeds the allowable 10MB limit. Please select another image.');/*untranslated*/
					return false;
				}
				var canvas_width = 1152;
				var canvas_height = 864;
				if (_component.indexOf("image") >= 0) {
					//canvas_width = parseInt($("#component_" + _component + "").css("width"), 10);
					//canvas_height = parseInt($("#component_" + _component + "").css("height"), 10);
					canvas_width = 300;
					canvas_height = 300;
				}
				$("#edit_redraw_canvas")[0].width = canvas_width;
				$("#edit_redraw_canvas")[0].height = canvas_height;

				var reader = new FileReader();
				reader.onload = function (event) {
					var img = document.createElement("img");
					img.src = event.target.result;

					var mimeType = img.src.split(",")[0].split(":")[1].split(";")[0];
					var canvas = document.getElementById("edit_redraw_canvas");
					var ctx = canvas.getContext("2d");
					ctx.clearRect(0, 0, canvas_width, canvas_height);
					setTimeout(function() {
						ctx.drawImage(img, 0, 0, canvas_width, canvas_height);
						var dataURL = canvas.toDataURL(mimeType);
						if(Math.round(dataURL.length / 1024) > 2048) {
							alert('Warning: The upload file size exceeds the allowable 2MB limit. Please select another image.\nNote: Sometime the smaller PNG image will take fewer megabytes and exceed 2MB limit after resizing the image.');/*untranslated*/
							return false;
						}
						else {
							edit_image_base64_array[_component] = dataURL;
							if (_component.indexOf("image") >= 0) {
								var _component_id = _component.split("_")[1];
								edit_update_content_image(_component_id);
							}
							else
								edit_update_content_background();
						}
					}, 100); //for firefox FPS(Frames per Second) issue need delay
				};
				reader.readAsDataURL(file);
				return false;
			}
			else {
				alert("<#Setting_upload_hint#>");
				return false;
			}
		};
	}
}
function edit_component_add(_obj, event) {
	if(_obj == "image" || _obj == "text") {
		if($(".multi_" + _obj + "").length < 5) {
			var _idx = $.now();
			$("#splash_page_layout").append(gen_splash_page_component(_obj, _idx));

			switch(_obj) {
				case "image" :
					edit_component_image(_idx);
					break;
				case "text" :
					edit_component_text(_idx);
					break;
			}
		}
		else {
			alert("<#JS_itemlimit1#> 5 <#JS_itemlimit2#>");
		}
	}
	else {
		if($("#component_" + _obj).length == 0) {
			$("#splash_page_layout").append(gen_splash_page_component(_obj));

			switch(_obj) {
				case "eula" :
					edit_component_eula();
					edit_update_content_eula();
					break;
				case "account" :
					edit_component_account();
					edit_update_content_account();
					break;
			}
		}
		else {
			alert("<#JS_itemlimit1#> 1 <#JS_itemlimit2#>");
		}
	}

	reset_toolbar_component_css();
	stop_propagation(event);
}
function edit_component_help() {
	component_click = "";
	toolbar_click = "toolbar_help";

	reset_component_css();
	reset_toolbar_css();
	
	var code = "";
	$("#splash_page_container_edit").html(code);

	code += "<div class='edit_component_item_title'>Introduce</div>";/*untranslated*/
	code += "<div class='edit_component_item_title'>The page will be pop up once the client successful access the Wi-Fi network you designated in the previous step. ASUS BERT-AC828 provide you plenty of pre-customized template.</div>";/*untranslated*/

	code += "<div class='edit_component_item_title'><#Captive_Portal_Editor#></div>";
	code += "<div class='edit_component_item_title'>EThe splash editor allows you to build a self-defined HTML web page with easiest way. The editor supports several web components, which includes Image, Text, EULA and Authentication unit. Just have few of clicks and fill the retired content. At the left hand size, the field is the mobile view that allow user directly position the unit of the web page. After the unit be pitched, you can edit the contents at the right hand size editing field.</div>";/*untranslated*/

	code += "<div class='edit_component_item_title'>Notice</div>";/*untranslated*/
	code += "<div class='edit_component_item_title'>A single web container supports up to 10 Mbytes. Once the self-defined web size exceed limitation, please downsize or redesign the web page.</div>";/*untranslated*/
	
	$("#splash_page_container_edit").html(code);
}

function change_template(_idx) {
	template_page_click = _idx;
	edit_component_template();
}
function edit_component_template() {
	component_click = "";
	toolbar_click = "toolbar_template";

	reset_component_css();
	reset_toolbar_css();
	
	var template_num = template.length;
	var float_css = "";
	var code = "";
	code += "<div class='edit_component_template_bg'>";
	var start_idx = 1 + (template_page_click -1) * 4;
	var end_idx = 4 + (template_page_click -1) * 4;
	for(start_idx; start_idx <= end_idx; start_idx += 1) {
		if(start_idx == template_num) {
			float_css = (start_idx % 2 == 1) ? "left" : "right";
			code += "<div id='edit_component_template_0' class='edit_component_template_content template_0 " + float_css + "' onclick='set_template_layout(0);'></div>";
			break;
		}
		else {
			float_css = (start_idx % 2 == 1) ? "left" : "right";
			code += "<div id='edit_component_template_" + start_idx + "' class='edit_component_template_content template_" + start_idx + " " + float_css + "' onclick='set_template_layout(" + start_idx + ");'></div>";
		}
	}
	code += "<div class='edit_component_template_page_bg'>";
	code += "<div class='captive_portal_adv_line'></div>";
	var template_page_num = Math.ceil(template.length / 4);
	for(var i = 1; i <= template_page_num; i += 1) {
		if(template_page_click == i)
			code += "<div id='edit_component_template_page_item_" + i + "' class='edit_component_template_page_item selected' onclick='change_template(" + i + ");'>" + i + "</div>";
		else 
			code += "<div id='edit_component_template_page_item_" + i + "' class='edit_component_template_page_item' onclick='change_template(" + i + ");'>" + i + "</div>";
	}
	code += "</div>";
	code += "</div>";
	$("#splash_page_container_edit").html(code);

	reset_template_css();
}
function edit_component_editor() {
	toolbar_click = "toolbar_editor";

	reset_component_css();
	reset_toolbar_css();

	if(component_click == "" || component_click == "component_background") {
		var code = "<div class='edit_component_item_hint'>Select web component at the left side.</div>";/*untranslated*/
		$("#splash_page_container_edit").html(code);
	}
}
function edit_component_customized() {
	component_click = "";
	toolbar_click = "toolbar_customized";

	reset_component_css();
	reset_toolbar_css();

	var code = "Template TBD";
	$("#splash_page_container_edit").html(code);
}
function reset_template_css() {
	var template_length = template.length;
	for(var i = 0; i < template_length; i += 1) {
		if($("#edit_component_template_" + i + "").hasClass("selected")) {
			$("#edit_component_template_" + i + "").removeClass("selected");
		}
	}
	$("#edit_component_template_" + template_click + "").addClass("selected");
}
function reset_gen_splash_layout() {
	var set_component_attribute = function(_component_id, _component_attr) {
		$("#" + _component_id + "").css({"left" : _component_attr.style_left});
		$("#" + _component_id + "").css({"top" : _component_attr.style_top});
		$("#" + _component_id + "").css({"width" : _component_attr.style_width});
		$("#" + _component_id + "").css({"height" : _component_attr.style_height});
		$("#" + _component_id + "").css({"z-index" : _component_attr.style_z_index});
		$("#" + _component_id + "").css({"color" : _component_attr.style_color});
		if(_component_id.indexOf("image") == -1 && _component_id.indexOf("background") == -1) {
			$("#" + _component_id + "").css({"background-color" : _component_attr.style_background_color});
			$("#" + _component_id + "").css({"opacity" : _component_attr.style_opacity});
		}
	};

	while ($(".multi_image").length > 0) {
		$("#" + $(".multi_image")[0].id).remove();
	}

	while ($(".multi_text").length > 0) {
		$("#" + $(".multi_text")[0].id).remove();
	}

	if(component_array["component_image"]) {
		var _component_obj_array = Object.keys(component_array["component_image"]);
		var _component_obj_length = Object.keys(component_array["component_image"]).length;
		if(_component_obj_length > 0) {
			for(var i = 0; i < _component_obj_length; i += 1) {
				var _idx = _component_obj_array[i];
				$("#splash_page_layout").append(gen_splash_page_component("image", _idx));
			
				var _image_opacity = component_array["component_image"][_idx].attribute.style_opacity;
				if(component_array["component_image"][_idx].image_type == "image") {
					var _edit_css = "background-image:url(" + component_array["component_image"][_idx].image_base64 + ");";
					_edit_css += "background-repeat: no-repeat;";
					_edit_css += "background-size: 100% 100%;";
					_edit_css += "opacity: " + _image_opacity + ";";
					var html = "<div style='height:100%;width:100%;overflow:auto;" + _edit_css + ";border-radius:8px;'></div>";
					$("#component_image_" + _idx  + " div").first().replaceWith(html);
					set_component_attribute("component_image_" + _idx  + "", component_array["component_image"][_idx].attribute);
				}
				else {
					var _background_color = component_array["component_image"][_idx].attribute.style_background_color;
					var _edit_css = "background-color: " + _background_color + ";";
					_edit_css += "opacity: " + _image_opacity + ";";
					var html = "<div style='height:100%;width:100%;overflow:auto;" + _edit_css + ";border-radius:8px;'></div>";
					$("#component_image_" + _idx  + " div").first().replaceWith(html);
					set_component_attribute("component_image_" + _idx  + "", component_array["component_image"][_idx].attribute);
				}
			}
		}
	}

	if(component_array["component_text"]) {
		var _component_obj_array = Object.keys(component_array["component_text"]);
		var _component_obj_length = Object.keys(component_array["component_text"]).length;
		if(_component_obj_length > 0) {
			for(var i = 0; i < _component_obj_length; i += 1) {
				var _idx = _component_obj_array[i];

				$("#splash_page_layout").append(gen_splash_page_component("text", _idx));

				var text_font_weight = component_array["component_text"][_idx].text_font_weight;
				var text_font_style =  component_array["component_text"][_idx].text_font_style;
				var text_font_family =  component_array["component_text"][_idx].text_font_family;
				var text_font_size =  component_array["component_text"][_idx].text_font_size;
				var text_walled_garden = "";
				if(component_array["component_text"][_idx].text_walled_garden) {
					text_walled_garden = encode_decode_text(component_array["component_text"][_idx].text_walled_garden, "decode");
				}
				var text_decoration = (text_walled_garden == "") ? "none" : "underline";

				var code = "<div style='height:100%;width:100%;overflow:auto;font-size:" + text_font_size + "px;font-weight:" + text_font_weight  + ";font-style:" + text_font_style + ";font-family:" + text_font_family + ";'>";
				var text_content = encode_decode_text(component_array["component_text"][_idx].text_content, "decode");
				text_content = htmlEnDeCode.htmlEncode(text_content);
				text_content = text_content.replace(/ /g, '&nbsp;').replace(/(?:\r\n|\r|\n)/g, '<br>');
				code +='<div style=\'text-decoration:' + text_decoration + ';\'>';
				code += text_content;
				code +='</div>';
				code += "</div>";
				$("#component_text_" + _idx  + " div").first().replaceWith(code);

				var background_color = component_array["component_text"][_idx].attribute.style_background_color;
				$("#component_text_" + _idx  + "").css({"background-color" : background_color});

				var font_color = component_array["component_text"][_idx].attribute.style_color;
				$("#component_text_" + _idx  + "").css({"color" : font_color});

				set_component_attribute("component_text_" + _idx  + "", component_array["component_text"][_idx].attribute);
			}
		}
	}
	$("#component_eula").remove();
	$("#component_account").remove();

	if(component_array["component_eula"]) {
		var eula_text = "I have read and agree to";
		var eula_hyperlink = "the Terms of Service";
		if(component_array["component_eula"][0].eula_terms_service_text == undefined)
			component_array["component_eula"][0].eula_terms_service_text = encode_decode_text(eula_text, "encode");
		if(component_array["component_eula"][0].eula_terms_service_hyperlink == undefined)
			component_array["component_eula"][0].eula_terms_service_hyperlink = encode_decode_text(eula_hyperlink, "encode");
		if(component_array["component_eula"][0].eula_terms_service_hyperlink_color == undefined)
			component_array["component_eula"][0].eula_terms_service_hyperlink_color = "#00B0FF";
		var code = "<div style='height:100%;width:100%;'>";
		code += "<div style='float:left;'>";
		code += "<input type='checkbox' name='eula_check' disabled>";
		code += "</div>";
		code += "<div>";
		code += "<span>" + htmlEnDeCode.htmlEncode(encode_decode_text(component_array["component_eula"][0].eula_terms_service_text, "decode")) + "</span>&nbsp;<span class='edit_eula_terms_service_hyperlink'>" +  htmlEnDeCode.htmlEncode(encode_decode_text(component_array["component_eula"][0].eula_terms_service_hyperlink, "decode")) + "</span>";
		code += "</div>";
		code += "</div>";

		$("#splash_page_layout").append(gen_splash_page_component("eula"));

		$("#component_eula div").first().replaceWith(code);
		set_component_attribute("component_eula", component_array["component_eula"][0].attribute);
		$("#component_eula").find(".edit_eula_terms_service_hyperlink").css({"color" : component_array["component_eula"][0].eula_terms_service_hyperlink_color});
	}
	else {
		$("#component_eula").remove();
	}

	if(component_array["component_account"]) {
		var code = "";

		var _font_color = transformHEXtoRGB("#" + component_array["component_account"][0].account_input_box_font_color);
		_font_color = _font_color.replace(')', ', 0.8)').replace('rgb', 'rgba');

		var _input_box_background_color = transformHEXtoRGB("#" + component_array["component_account"][0].account_input_box_background_color);
		var _input_box_opacity = component_array["component_account"][0].account_input_box_opacity;
		_input_box_background_color = _input_box_background_color.replace(')', ', ' + _input_box_opacity + ')').replace('rgb', 'rgba');
		if(component_array["component_account"][0].account_username == "")
			component_array["component_account"][0].account_username = encode_decode_text("Enter Account", "encode");
		if(component_array["component_account"][0].account_password == "")
			component_array["component_account"][0].account_password = encode_decode_text("Enter Password", "encode");
		component_array["component_account"][0].attribute.style_height = "66px";
		component_array["component_account"][0].account_auth_type = "1";

		code += "<div style='height:100%;width:100%;'>";

		code += "<div style='background-color:" + _input_box_background_color + ";color:" + _font_color + ";";
		code += "height:30px;border-radius:4px;padding-left:2%;line-height:30px;font-style:oblique;'>";
		code += "" + htmlEnDeCode.htmlEncode(encode_decode_text(component_array["component_account"][0].account_username, "decode")) + "</div>";

		code += "<div style='background-color:" + _input_box_background_color + ";color:" + _font_color + ";";
		code += "height:30px;border-radius:4px;padding-left:2%;line-height:30px;font-style:oblique;margin-top:6px;'>";
		code += "" + htmlEnDeCode.htmlEncode(encode_decode_text(component_array["component_account"][0].account_password, "decode")) + "</div>";

		code += "</div>";

		$("#splash_page_layout").append(gen_splash_page_component("account"));

		$("#component_account div").first().replaceWith(code);
		set_component_attribute("component_account", component_array["component_account"][0].attribute);
	}
	else {
		$("#component_account").remove();
	}

	if(component_array["component_button"]) {
		$("#component_button").css({"background-color" : component_array["component_button"][0].attribute.style_background_color});
		$("#component_button").css({"color" : component_array["component_button"][0].attribute.style_color});

		var button_height = component_array["component_button"][0].attribute.style_height;
		var code = "<div style='height:100%;width:100%;'>";
		code += "<div style='height:" + button_height + ";line-height:" + button_height + ";text-align:center;overflow:hidden;'>";
		code += htmlEnDeCode.htmlEncode(encode_decode_text(component_array["component_button"][0].button_label, "decode"));
		code += "</div>";
		code += "</div>";

		$("#component_button div").first().replaceWith(code);
		set_component_attribute("component_button", component_array["component_button"][0].attribute);
	}
	else {
		$("#component_button").remove();
		$("#splash_page_layout").append(gen_splash_page_component("button"));
	}
	if(component_array["component_background"]) {
		var _background_opacity = component_array["component_background"][0].attribute.style_opacity;
		if(component_array["component_background"][0].image_type == "image") {
			var _edit_css = "background-image:url(" + component_array["component_background"][0].image_base64  + ");";
			_edit_css += "background-repeat: no-repeat;";
			_edit_css += "background-size: 250%;";
			_edit_css += "background-position: 50% 50%;";
			_edit_css += "opacity: " + _background_opacity + ";";
			var code = "<div style='border-radius:6px;height:100%;width:100%;overflow:auto;" + _edit_css + "'></div>";
			$("#component_background").html(code);
		}
		else {
			var _background_color = component_array["component_background"][0].attribute.style_background_color;
			var _edit_css = "background-color: " + _background_color + ";";
			_edit_css += "opacity: " + _background_opacity + ";";
			var code = "<div style='border-radius:6px;height:100%;width:100%;overflow:auto;" + _edit_css + "'></div>";
			$("#component_background").html(code);
		}
	}
	else {
		$("#component_background").remove();
		$("#splash_page_layout").append(gen_splash_page_component("background"));
	}

	reset_editing_area_css();
}
function set_template_layout(_template_idx) {
	toolbar_click = "toolbar_template";
	component_array = [];
	component_array = set_template_array(_template_idx);
	reset_gen_splash_layout();
	reset_template_css();
	reset_toolbar_component_css();
}
function set_template_array(_template_idx) {
	var _temp_array = new Array();
	var clone_template_array = function(_template_array) {
		Object.keys(_template_array).forEach(function(key) {
			if(key == "component_text" || key == "component_image") {
				var _component_type = key;
				var _multi_array = 	_template_array[_component_type];
				var _component_array = "";
				if(_temp_array[_component_type] == undefined)
					_temp_array[_component_type] = [];
				Object.keys(_multi_array).forEach(function(_idx) {
					_component_array = JSON.parse( JSON.stringify( _multi_array[_idx] ) );
					_temp_array[_component_type][_component_array.idx] = _component_array;
					
				});
			}
			else {
				var _component_type = key;
				var _component_array = [_template_array[_component_type]];
				var _new_component_array = "";
				var _new_component_array = JSON.parse( JSON.stringify( _component_array ) );
				if(_new_component_array != "") {
					_temp_array[_component_type] = _new_component_array;
				}
			}
		});
	};

	clone_template_array(template[_template_idx]);
	template_click = _template_idx;

	return _temp_array;	
}
function reset_splash_page_layout() {
	component_array = [];
	//edited default
	if(Object.keys(component_array_default).length > 0) {
		template_click = "";
		Object.keys(component_array_default).forEach(function(key) {
			if(key == "component_text" || key == "component_image") {
				var _component_type = key;
				var _multi_array = 	component_array_default[_component_type];
				var _component_array = "";
				if(component_array[_component_type] == undefined)
					component_array[_component_type] = [];
				Object.keys(_multi_array).forEach(function(_idx) {
					_component_array = JSON.parse( JSON.stringify( _multi_array[_idx] ) );
					component_array[_component_type][_idx] = _component_array;	
				});
			}
			else {
				var _component_type = key;
				var _component_array = component_array_default[_component_type];
				var _new_component_array = "";
				var _new_component_array = JSON.parse( JSON.stringify( _component_array ) );
				if(_new_component_array != "") {
					component_array[_component_type] = _new_component_array;
				}
			}
		});
	}
	else { // template
		component_array = set_template_array(template_click);
	}	

	reset_gen_splash_layout();
	reset_template_css();
	reset_toolbar_component_css();
}
function gen_splash_page() {
	var code = "";

	code += "<div style='height:600px;margin-top:10px;'>";

	//tool bar left
	code += "<div id='splash_page_container_layout' class='splash_page_container_left'>";
	code += "<div class='splash_page_toolbar_bg_left'>";
	code += "<div id='toolbar_unit_image' title='<#CTL_add#> Image' class='splash_page_toolbar_unit_bg_left splash_page_toolbar_unit_image' onclick='edit_component_add(\"image\", event);'></div>";
	code += "<div id='toolbar_unit_text' title='<#CTL_add#> Text' class='splash_page_toolbar_unit_bg_left splash_page_toolbar_unit_text' onclick='edit_component_add(\"text\" , event);'></div>";
	code += "<div id='toolbar_unit_account' title='<#CTL_add#> <#PPPConnection_Authentication_itemname#>' class='splash_page_toolbar_unit_bg_left splash_page_toolbar_unit_account' onclick='edit_component_add(\"account\" , event);'></div>";
	code += "<div id='toolbar_unit_eula' title='<#CTL_add#> Eula' class='splash_page_toolbar_unit_bg_left splash_page_toolbar_unit_eula' onclick='edit_component_add(\"eula\" , event);'></div>";
	code += "<div title='<#btn_Preview#>' class='splash_page_toolbar_unit_preview' onclick='preview(event);'></div>";
	code += "</div>";

	//component layout left
	code += "<div id='splash_page_layout_bg' class='splash_page_layout_bg' onclick='stop_propagation(event);'>";
	code += "<div id='splash_page_layout' class='splash_page_layout'>";
	//image
	//code += gen_splash_page_component("image", $.now());
	//text
	//code += gen_splash_page_component("text", $.now());
	//eula
	//code += gen_splash_page_component("eula");
	//account
	//code += gen_splash_page_component("account");
	//Button
	code += gen_splash_page_component("button");
	//Background
	code += gen_splash_page_component("background");
	code += "</div>";
	code += "</div>";
	code += "<div class='captive_portal_adv_reset'>";
	code += "<input id='captive_portal_adv_reset' class='button_gen' onclick='reset_splash_page_layout()' type='button' value='<#CTL_Reset_OOB#>'/>";
	code += "<div id='splash_page_toolbar_unit_help' title='<#CTL_help#>' class='splash_page_toolbar_unit_help' onclick='toolbar_unit_help(event);'></div>";
	code +="</div>";
	code += "</div>";

	//toolbar right
	code += "<div class='splash_page_toolbar_bg_right' onclick='stop_propagation(event);'>";
	code += "<div id='toolbar_template' title='<#Captive_Portal_Template#>' class='splash_page_toolbar_unit_bg_right splash_page_toolbar_unit_template' onclick='edit_component_template();'>";
	code += "<div class='splash_page_toolbar_unit_title'><#Captive_Portal_Template#></div>";
	code += "</div>";
	code += "<div id='toolbar_background'  title='<#btn_Background#>' class='splash_page_toolbar_unit_bg_right splash_page_toolbar_unit_background' onclick='edit_component_background();'>";
	code += "<div class='splash_page_toolbar_unit_title'><#btn_Background#></div>";
	code += "</div>";
	code += "<div id='toolbar_editor' title='<#Captive_Portal_Editor#>' class='splash_page_toolbar_unit_bg_right splash_page_toolbar_unit_editor' onclick='edit_component_editor();'>";
	code += "<div class='splash_page_toolbar_unit_title'><#Captive_Portal_Editor#></div>";
	code += "</div>";
	//code += "<div id='toolbar_customized' title='Customized' class='splash_page_toolbar_unit_bg_right splash_page_toolbar_unit_customized' onclick='edit_component_customized();'>";
	//code += "<div class='splash_page_toolbar_unit_title'>Customized</div>";/*untranslated*/
	//code += "</div>";
	//code += "<div id='toolbar_help' title='<#Help#>' class='splash_page_toolbar_unit_bg_right splash_page_toolbar_unit_help' onclick='edit_component_help();'>";
	//code += "<div class='splash_page_toolbar_unit_title'><#Help#></div>";
	//code += "</div>";
	code += "</div>";

	//component edit right
	code += "<div id='splash_page_container_edit' class='splash_page_container_right' onclick='stop_propagation(event);'>";
	code += "</div>";

	code += "</div>";
	
	return code;
}
function showHideVerification() {
	$("#verification_local_bg").css("display", "none");
	$("#verification_radius_bg").css("display", "none");
	if($("input[name=cpa_verification]:checked").val() == "1") {
		$("#verification_radius_bg").css("display", "");
	}
	else {
		$("#verification_local_bg").css("display", "");
	}
}
function gen_account_settings() {
	var code = "";
	code += "<div id='account_settings_hint' class='account_settings_hint'>* You didn't have the requirement for both account numbers and ID in the second step, so this step can be ignored. Please directly press the saving button.</div>";
	code += "<div><#FreeWiFi_AccountSetting_desc1#></div>";
	code += "<table width='100%' border='1' align='center' cellpadding='4' cellspacing='0' bordercolor='#6b8fa3' class='FormTable' style='margin-top:15px;'>";
	code += "<thead>";
	code += "<tr>";
	code += "<td colspan='2'><#FreeWiFi_AccountSetting_Server#></td>";
	code += "</tr>";
	code += "</thead>";
	code += "<tr>";
	code += "<th><#FreeWiFi_AccountSetting_Verification#></th>";
	code += "<td>";
	code += "<input type='radio' name='cpa_verification' id='cpa_verification_local' value='0' onclick='showHideVerification();' checked><label for='cpa_verification_local'><#Captive_Portal_Local#></label>";
	code += "<input type='radio' name='cpa_verification' id='cpa_verification_radius' value='1' onclick='showHideVerification();'><label for='cpa_verification_radius'>RADIUS</label>";/*untranslated*/
	code += "</td>";
	code += "</tr>";
	code += "</table>";

	code += "<div id='verification_local_bg' style='margin-top:15px;'>";
	code += "<div><#FreeWiFi_AccountSetting_desc2#></div>";
	code += "<table width='100%' border='1' align='center' cellpadding='4' cellspacing='0' class='FormTable_table' style='margin-top:15px;'>";
	code += "<thead>";
	code += "<tr>";
	code += "<td colspan='3'><#Username_Pwd#>&nbsp;(<#List_limit#>&nbsp;32)</td>";
	code += "</tr>";
	code += "</thead>";
	code += "<tr>";
	code += "<th><#Username#></th>";
	code += "<th><#HSDPAConfig_Password_itemname#></th>";
	code += "<th><#list_add_delete#></th>";
	code += "</tr>";
	code += "<tr>";
	code += "<td width='40%'>";
	code += "<input type='text' class='input_22_table' maxlength='64' name='cpa_local_username' id='cpa_local_username' onKeyPress='return validator.isString(this, event)' autocorrect='off' autocapitalize='off'>";
	code += "</td>";
	code += "<td width='40%'>";
	code += "<input type='text' class='input_22_table' maxlength='64' name='cpa_local_password' id='cpa_local_password' onKeyPress='return validator.isString(this, event)' autocorrect='off' autocapitalize='off'>";
	code += "</td>";
	code += "<td width='20%'>";
	code += "<div><input type='button' class='add_btn' onClick='addRow_local(32);' value=''></div>";
	code += "</td>";
	code += "</tr>";
	code += "</table>";

	code += "<div id='verification_local_clientlist_block'></div>";
	code += "</div>";

	code += "<div id='verification_radius_bg' style='display:none;'>";
	code += "<table width='100%' border='1' align='center' cellpadding='4' cellspacing='0' bordercolor='#6b8fa3' class='FormTable' style='margin-top:15px;'>";
	code += "<thead>";
	code += "<tr>";
	code += "<td colspan='2'><#t2BC#></td>";
	code += "</tr>";
	code += "</thead>";

	code += "<tr>";
	code += "<th><a class='hintstyle' href='javascript:void(0);' onClick='openHint(2, 1);'><#WLANAuthentication11a_ExAuthDBIPAddr_itemname#></a></th>";
	code += "<td>";
	code += "<input style='margin-left:0px;' type='text' maxlength='15' name='cpa_radius_server_ipaddr' value='' class='input_15_table' onKeyPress='return validator.isIPAddr(this, event)' autocomplete='off' autocorrect='off' autocapitalize='off'>";
	code += "</td>";
	code += "</tr>";

	code += "<tr>";
	code += "<th><a class='hintstyle' href='javascript:void(0);' onClick='openHint(2, 2);'><#WLANAuthentication11a_ExAuthDBPortNumber_itemname#></a></th>";
	code += "<td>";
	code += "<input style='margin-left:0px;' type='text' maxlength='5' name='cpa_radius_server_port' value='1812' class='input_6_table' onkeypress='return validator.isNumber(this,event)' autocomplete='off' autocorrect='off' autocapitalize='off'>";
	code += "</td>";
	code += "</tr>";

	code += "<tr>";
	code += "<th><a class='hintstyle' href='javascript:void(0);' onClick='openHint(2, 3);'><#WLANAuthentication11a_ExAuthDBPassword_itemname#></a></th>";
	code += "<td>";
	code += "<input style='margin-left:0px;' type='password' maxlength='64' name='cpa_radius_key' value='' class='input_32_table' autocomplete='off' autocorrect='off' autocapitalize='off'>";
	code += "</td>";
	code += "</tr>";

	code += "<tr>";
	code += "<th><a class='hintstyle' href='javascript:void(0);' onClick='openHint(31, 4);'>NAS ID</a></th>";/*untranslated*/
	code += "<td>";
	code += "<input style='margin-left:0px;' type='text' name='cpa_radius_nas_id' value='' class='input_32_table' autocomplete='off' autocorrect='off' autocapitalize='off'>";
	code += "</td>";
	code += "</tr>";

	code += "</table>";
	code += "</div>";

	return code;
}
function gen_captive_portal_adv_content(step) {
	var code = "";
	switch(step) {
		case "1" :
			code += gen_basic_settings();
			break;
		case "2" :
			code += gen_splash_page();
			break;
		case "3" :
			code += gen_account_settings();
			break;
	}
	return code;
}
function editProfile(_profile_id) {
	template_page_click = 1;
	if(_profile_id != "new") {
		profile_id = _profile_id
		template_click = "";
	}
	else {
		var rule_num = $("#captive_portal_adv_profile_table")[0].rows.length;
		if(rule_num >= 1 && captive_portal_adv_profile_list != "") {
			alert("<#JS_itemlimit1#>1<#JS_itemlimit2#>");
			return false;
		}
		component_array = [];
		profile_id = $.now();
		template_click = 1;
		component_array_default = [];
	}

	var edit_profile_content = [];
	var used_profile_wl_if = [];
	var captive_portal_list_row = captive_portal_adv_profile_list.split("<");
	for(var i = 0; i < captive_portal_list_row.length; i += 1) {
		var captive_portal_list_col = captive_portal_list_row[i].split(">");
		if(captive_portal_list_col != "") {
			if(captive_portal_list_col[0] == profile_id) {
				edit_profile_content = captive_portal_list_col;
			}
			used_profile_wl_if[captive_portal_list_col[0]] = captive_portal_list_col[5];
		}
	}

	local_list_current = "";
	var local_clientlist_row = captive_portal_adv_local_clientlist.split("<");
	for(var i = 0; i < local_clientlist_row.length; i += 1) {
		if(local_clientlist_row[i] != "") {
			var local_clientlist_col = local_clientlist_row[i].split(">");
			if(local_clientlist_col[0] == profile_id) {
				local_list_current = local_clientlist_col[1];
				break;
			}
		}
	}

	captive_portal_adv_edit_idx = 1;
	var code = "";

	//title
	code += "<div class='captive_portal_adv_log_close'><img src='/images/button-close.gif' style='width:35px;' onclick='edit_cancel(1);'></div>";
	
	//step tab
	code += gen_tab_menu();

	//step 1 content
	code += "<div id='captive_portal_adv_step_1_content' class='captive_portal_adv_content_bg' style='display:none;'></div>";

	//step 2 content
	code += "<div id='captive_portal_adv_step_2_content' style='display:none;'></div>";

	//step 3 content
	code += "<div id='captive_portal_adv_step_3_content' class='captive_portal_adv_content_bg' style='display:none;'></div>";

	//action button
	code += gen_action_button();

	$('#captive_portal_adv_setting').html(code);
	$('#captive_portal_adv_setting').fadeIn();
	set_tab_and_action_btn();

	gen_captive_portal_adv_edit_content();

	if(_profile_id == "new") {
		$("#captive_portal_adv_finsih").click( function(){ finishRule("new"); } );
		edit_component_template();
		reset_toolbar_component_css();
		set_template_layout(1);
	}
	else {
		$("#captive_portal_adv_finsih").click( function(){ finishRule(_profile_id); } );
		get_splash_page_attribute(edit_profile_content[0]);
		$("input[name=cpa_profile_name]").val(edit_profile_content[1]);
		
		var set_time = function(_seconds, _value_obj, _unit_obj) {
			var isInteger = function(_value) {
				return (_value | 0) === _value;
			};
			var _minutes = parseInt(_seconds) / 60;
			var is_hour = _minutes / 60;
			if(isInteger(is_hour)) {
				var is_day = is_hour / 24;
				if(isInteger(is_day)) {
					_value_obj.val(is_day);
					_unit_obj.val("day");
				}
				else {
					_value_obj.val(is_hour);
					_unit_obj.val("hour");
				}
			}
			else {
				_value_obj.val(_minutes);
				_unit_obj.val("minute");
			}
		};
		if(edit_profile_content[2] == 0) {
			$("#cpa_away_timeout_unlimited").prop("checked", true);
		}
		else {
			$("#cpa_away_timeout_time").prop("checked", true);
			set_time(edit_profile_content[2], $("input[name=cpa_away_set]"), $("select[name=cpa_away_timeout_unit]"));
		}
		if(edit_profile_content[3] == 0) {
			$("#cpa_session_timeout_unlimited").prop("checked", true);
		}
		else {
			$("#cpa_session_timeout_time").prop("checked", true);
			set_time(edit_profile_content[3], $("input[name=cpa_session_set]"), $("select[name=cpa_session_timeout_unit]"));
		}

		$("input[name=cpa_landing_page]").val(edit_profile_content[4]);

		var _wl_if_group = "";
		var set_checkbox_status = function(_unit, _status) {
			if($("input[name=cb_wl_" + _unit + "]").length) {
				$("#cb_wl_" + _unit + "").prop("checked", _status);
			}
		};

		set_checkbox_status(0, false);
		change_wl_input_status(0);
		var captive_portal_adv_ssid = "";
		captive_portal_adv_ssid = decodeURIComponent('<% nvram_char_to_ascii("", "captive_portal_adv_2g_ssid"); %>');
		if(captive_portal_adv_ssid != "")
			$("input[name=wl_0]").val(captive_portal_adv_ssid);
		if(wl_info.band5g_support) {
			set_checkbox_status(1, false);
			change_wl_input_status(1);
			captive_portal_adv_ssid = decodeURIComponent('<% nvram_char_to_ascii("", "captive_portal_adv_5g_ssid"); %>');
			if(captive_portal_adv_ssid != "")
				$("input[name=wl_1]").val(captive_portal_adv_ssid);
		}
		if(wl_info.band5g_2_support) {
			set_checkbox_status(2, false);
			change_wl_input_status(2);
			captive_portal_adv_ssid = decodeURIComponent('<% nvram_char_to_ascii("", "captive_portal_adv_5g_2_ssid"); %>');
			if(captive_portal_adv_ssid != "")
				$("input[name=wl_2]").val(captive_portal_adv_ssid);
		}
		var wl_if_array = edit_profile_content[5].split("wl");
		for(var i = 0; i < wl_if_array.length; i += 1) {
			if(wl_if_array[i] != "") {
				var wl_band = wl_if_array[i].split(".")[0];
				_wl_if_group = wl_if_array[i].split(".")[1];
				if($("input[name=cb_wl_" + wl_band + "]").length) {
					set_checkbox_status(wl_band, true);
					change_wl_input_status(wl_band);
				}
			}
		}
	}

	var verification = edit_profile_content[8];
	$("input[name=cpa_radius_server_ipaddr]").val("");
	$("input[name=cpa_radius_server_port]").val("1812");
	$("input[name=cpa_radius_key]").val("");
	$("input[name=cpa_radius_nas_id]").val("");
	settingRadioItemCheck($("input[name=cpa_verification]"), "0");
	if(verification == "1") {
		settingRadioItemCheck($("input[name=cpa_verification]"), "1");
		$("input[name=cpa_radius_server_ipaddr]").val(edit_profile_content[9]);
		$("input[name=cpa_radius_server_port]").val(edit_profile_content[10]);
		$("input[name=cpa_radius_key]").val(edit_profile_content[11]);
		$("input[name=cpa_radius_nas_id]").val(edit_profile_content[12]);
	}
	showHideVerification();

	if(edit_profile_content[14] == undefined || edit_profile_content[14] == 0) {
		$("#cpa_bw_dl_unlimited").prop("checked", true);
	}
	else {
		$("#cpa_bw_dl_limited").prop("checked", true);
		$("input[name=cpa_bw_dl_set]").val(edit_profile_content[14]/1024);
	}

	if(edit_profile_content[15] == undefined || edit_profile_content[15] == 0) {
		$("#cpa_bw_ul_unlimited").prop("checked", true);
	}
	else {
		$("#cpa_bw_ul_limited").prop("checked", true);
		$("input[name=cpa_bw_ul_set]").val(edit_profile_content[15]/1024);
	}

	show_local_clientlist();
}
function switch_tab(_idx) {
	captive_portal_adv_edit_idx = _idx;
	set_tab_and_action_btn();
}
function preRule() {
	captive_portal_adv_edit_idx--;
	set_tab_and_action_btn();
}
function nextRule() {
	captive_portal_adv_edit_idx++;
	set_tab_and_action_btn();
}
function get_splash_page_attribute(_profile_id) {
	var result = "NoData";
	$.ajax({
		url: '/ajax_captive_portal.asp?profile_id=' + _profile_id,
		dataType: 'json',
		error: function(xhr){
			setTimeout("get_splash_page_attribute('" + _profile_id + "');", 1000);
		},
		success: function(response) {
			component_array = [];
			component_array_default = [];
			var component_array_element = "";
			var component_array_default_element = "";
			var set_component_attribute = function(_component_id, _component_attr) {
				$("#" + _component_id + "").css({"left" : _component_attr.style_left});
				$("#" + _component_id + "").css({"top" : _component_attr.style_top});
				$("#" + _component_id + "").css({"width" : _component_attr.style_width});
				$("#" + _component_id + "").css({"height" : _component_attr.style_height});
				$("#" + _component_id + "").css({"z-index" : _component_attr.style_z_index});
				$("#" + _component_id + "").css({"color" : _component_attr.style_color});
				if(_component_id.indexOf("image") == -1 && _component_id.indexOf("background") == -1) {
					$("#" + _component_id + "").css({"background-color" : _component_attr.style_background_color});
					$("#" + _component_id + "").css({"opacity" : _component_attr.style_opacity});
				}
			};

			while ($(".multi_image").length > 0) {
				$("#" + $(".multi_image")[0].id).remove();
			}

			while ($(".multi_text").length > 0) {
				$("#" + $(".multi_text")[0].id).remove();
			}

			if(response.component_image) {
				component_array_element = JSON.parse( JSON.stringify( response.component_image ) );
				component_array_default_element = JSON.parse( JSON.stringify( response.component_image ) );
				for(var i = 0; i < component_array_element.length; i += 1) {
					var _idx = component_array_element[i].idx;
					if(component_array["component_image"] == undefined)
						component_array["component_image"] = [];
					component_array["component_image"][_idx] = component_array_element[i];
					delete component_array["component_image"][_idx].idx;
					if(component_array_default["component_image"] == undefined)
						component_array_default["component_image"] = [];
					component_array_default["component_image"][_idx] = component_array_default_element[i];

					$("#splash_page_layout").append(gen_splash_page_component("image", _idx));
					
					var _image_opacity = component_array["component_image"][_idx].attribute.style_opacity;
					if(component_array["component_image"][_idx].image_type == "image") {
						var _edit_css = "background-image:url(" + component_array["component_image"][_idx].image_base64 + ");";
						_edit_css += "background-repeat: no-repeat;";
						_edit_css += "background-size: 100% 100%;";
						_edit_css += "opacity: " + _image_opacity + ";";
						var html = "<div style='height:100%;width:100%;overflow:auto;" + _edit_css + ";border-radius:8px;'></div>";
						$("#component_image_" + _idx  + " div").first().replaceWith(html);
						set_component_attribute("component_image_" + _idx  + "", component_array["component_image"][_idx].attribute);
					}
					else {
						var _background_color = component_array["component_image"][_idx].attribute.style_background_color;
						var _edit_css = "background-color: " + _background_color + ";";
						_edit_css += "opacity: " + _image_opacity + ";";
						var html = "<div style='height:100%;width:100%;overflow:auto;" + _edit_css + ";border-radius:8px;'></div>";
						$("#component_image_" + _idx  + " div").first().replaceWith(html);
						set_component_attribute("component_image_" + _idx  + "", component_array["component_image"][_idx].attribute);
					}
				}
			}
			if(response.component_text) {
				component_array_element = JSON.parse( JSON.stringify( response.component_text ) );
				component_array_default_element = JSON.parse( JSON.stringify( response.component_text ) );
				for(var i = 0; i < component_array_element.length; i += 1) {
					var _idx = component_array_element[i].idx;
					if(component_array["component_text"] == undefined)
						component_array["component_text"] = [];
					component_array["component_text"][_idx] = component_array_element[i];
					delete component_array["component_text"][_idx].idx;
					if(component_array_default["component_text"] == undefined)
						component_array_default["component_text"] = [];
					component_array_default["component_text"][_idx] = component_array_default_element[i];

					$("#splash_page_layout").append(gen_splash_page_component("text", _idx));

					var text_font_weight = component_array["component_text"][_idx].text_font_weight;
					var text_font_style =  component_array["component_text"][_idx].text_font_style;
					var text_font_family =  component_array["component_text"][_idx].text_font_family;
					var text_font_size =  component_array["component_text"][_idx].text_font_size;
					var text_walled_garden = "";
					if(component_array["component_text"][_idx].text_walled_garden) {
						text_walled_garden = encode_decode_text(component_array["component_text"][_idx].text_walled_garden, "decode");
					}
					var text_decoration = (text_walled_garden == "") ? "none" : "underline";

					var code = "<div style='height:100%;width:100%;overflow:auto;font-size:" + text_font_size + "px;font-weight:" + text_font_weight  + ";font-style:" + text_font_style + ";font-family:" + text_font_family + ";'>";
					var text_content = encode_decode_text(component_array["component_text"][_idx].text_content, "decode");
					text_content = htmlEnDeCode.htmlEncode(text_content);
					text_content = text_content.replace(/ /g, '&nbsp;').replace(/(?:\r\n|\r|\n)/g, '<br>');
					code +='<div style=\'text-decoration:' + text_decoration + ';\'>';
					code += text_content;
					code +='</div>';
					code += "</div>";
					$("#component_text_" + _idx  + " div").first().replaceWith(code);

					var background_color = component_array["component_text"][_idx].attribute.style_background_color;
					$("#component_text_" + _idx  + "").css({"background-color" : background_color});

					var font_color = component_array["component_text"][_idx].attribute.style_color;
					$("#component_text_" + _idx  + "").css({"color" : font_color});

					set_component_attribute("component_text_" + _idx  + "", component_array["component_text"][_idx].attribute);
				}
			}
			if(response.component_eula) {
				component_array_element = JSON.parse( JSON.stringify( response.component_eula ) );
				component_array_default_element = JSON.parse( JSON.stringify( response.component_eula ) );
				component_array["component_eula"] = [component_array_element];
				component_array_default["component_eula"] = [component_array_default_element];
				var eula_text = 'I have read and agree to';
				var eula_hyperlink = 'the Terms of Service';
				var eula_hyperlink_color = '#00B0FF';
				if(response.component_eula.eula_terms_service_text != undefined)
					eula_text = response.component_eula.eula_terms_service_text;
				if(response.component_eula.eula_terms_service_hyperlink != undefined)
					eula_hyperlink = response.component_eula.eula_terms_service_hyperlink;
				if(response.component_eula.eula_terms_service_hyperlink_color != undefined)
					eula_hyperlink_color = response.component_eula.eula_terms_service_hyperlink_color;
				var code = "<div style='height:100%;width:100%;'>";
				code += "<div style='float:left;'>";
				code += "<input type='checkbox' name='eula_check' disabled>";
				code += "</div>";
				code += "<div>";
				code += "<span>" + htmlEnDeCode.htmlEncode(decodeURIComponent(eula_text)) + "</span>&nbsp;<span class='edit_eula_terms_service_hyperlink'>" + htmlEnDeCode.htmlEncode(decodeURIComponent(eula_hyperlink)) + "</span>";
				code += "</div>";
				code += "</div>";

				$("#splash_page_layout").append(gen_splash_page_component("eula"));

				$("#component_eula div").first().replaceWith(code);
				set_component_attribute("component_eula", component_array["component_eula"][0].attribute);
				$("#component_eula").find(".edit_eula_terms_service_hyperlink").css({"color" : eula_hyperlink_color});
			}
			else {
				$("#component_eula").remove();
			}
			if(response.component_account) {
				component_array_element = JSON.parse( JSON.stringify( response.component_account ) );
				component_array_default_element = JSON.parse( JSON.stringify( response.component_account ) );
				var code = "";
				component_array["component_account"] = [component_array_element];
				component_array_default["component_account"] = [component_array_default_element];

				var _font_color = transformHEXtoRGB("#" + component_array["component_account"][0].account_input_box_font_color);
				_font_color = _font_color.replace(')', ', 0.8)').replace('rgb', 'rgba');

				var _input_box_background_color = transformHEXtoRGB("#" + component_array["component_account"][0].account_input_box_background_color);
				var _input_box_opacity = component_array["component_account"][0].account_input_box_opacity;
				_input_box_background_color = _input_box_background_color.replace(')', ', ' + _input_box_opacity + ')').replace('rgb', 'rgba');
				if(component_array["component_account"][0].account_username == "")
					component_array["component_account"][0].account_username = encode_decode_text("Enter Account", "encode");
				if(component_array["component_account"][0].account_password == "")
					component_array["component_account"][0].account_password = encode_decode_text("Enter Password", "encode");
				component_array["component_account"][0].attribute.style_height = "66px";
				component_array["component_account"][0].account_auth_type = "1";
				code += "<div style='height:100%;width:100%;'>";

				code += "<div style='background-color:" + _input_box_background_color + ";color:" + _font_color + ";";
				code += "height:30px;border-radius:4px;padding-left:2%;line-height:30px;font-style:oblique;'>";
				code += "" + htmlEnDeCode.htmlEncode(encode_decode_text(component_array["component_account"][0].account_username, "decode")) + "</div>";

				code += "<div style='background-color:" + _input_box_background_color + ";color:" + _font_color + ";";
				code += "height:30px;border-radius:4px;padding-left:2%;line-height:30px;font-style:oblique;margin-top:6px;'>";
				code += "" + htmlEnDeCode.htmlEncode(encode_decode_text(component_array["component_account"][0].account_password, "decode")) + "</div>";

				code += "</div>";
				$("#splash_page_layout").append(gen_splash_page_component("account"));

				$("#component_account div").first().replaceWith(code);
				set_component_attribute("component_account", component_array["component_account"][0].attribute);
			}
			else {
				$("#component_account").remove();
			}
			if(response.component_button) {
				component_array_element = JSON.parse( JSON.stringify( response.component_button ) );
				component_array_default_element = JSON.parse( JSON.stringify( response.component_button ) );
				component_array["component_button"] = [component_array_element];
				component_array_default["component_button"] = [component_array_default_element];
				
				$("#component_button").css({"background-color" : component_array["component_button"][0].attribute.style_background_color});
				$("#component_button").css({"color" : component_array["component_button"][0].attribute.style_color});

				var button_height = component_array["component_button"][0].attribute.style_height;
				var code = "<div style='height:100%;width:100%;'>";
				code += "<div style='height:" + button_height + ";line-height:" + button_height + ";text-align:center;overflow:hidden;'>";
				code +=  htmlEnDeCode.htmlEncode(encode_decode_text(component_array["component_button"][0].button_label, "decode"));
				code += "</div>";
				code += "</div>";

				$("#component_button div").first().replaceWith(code);
				set_component_attribute("component_button", component_array["component_button"][0].attribute);
			}
			if(response.component_background) {
				component_array_element = JSON.parse( JSON.stringify( response.component_background ) );
				component_array_default_element = JSON.parse( JSON.stringify( response.component_background ) );
				component_array["component_background"] = [component_array_element];
				component_array_default["component_background"] = [component_array_default_element];

				var _background_opacity = component_array["component_background"][0].attribute.style_opacity;
				if(component_array["component_background"][0].image_type == "image") {
					var _edit_css = "background-image:url(" + component_array["component_background"][0].image_base64  + ");";
					_edit_css += "background-repeat: no-repeat;";
					_edit_css += "background-size: 250%;";
					_edit_css += "background-position: 50% 50%;";
					_edit_css += "opacity: " + _background_opacity + ";";
					var code = "<div style='height:100%;width:100%;overflow:auto;" + _edit_css + "'></div>";
					$("#component_background").html(code);
				}
				else {
					var _background_color = component_array["component_background"][0].attribute.style_background_color;
					var _edit_css = "background-color: " + _background_color + ";";
					_edit_css += "opacity: " + _background_opacity + ";";
					var code = "<div style='height:100%;width:100%;overflow:auto;" + _edit_css + "'></div>";
					$("#component_background").html(code);
				}
			}
			edit_component_template();
			reset_toolbar_component_css();
		}
	});
}
function call_back_to_save_config(_splash_page_status) {
	if(_splash_page_status) {
		document.form.submit();
		show_profilelist();
		if($("#captive_portal_adv_setting").css('display') == "block")
			edit_cancel(0);
	}
	else {
		alert('<#ALERT_OF_ERROR_System4#>');
	}
}
function finishRule(flag) {
	//save profile setting
	var first_time_entry_flag = false;
	if(flag == "new") {
		if(captive_portal_adv_profile_list == "")
			first_time_entry_flag = true;
	}

	var captive_portal_adv_current_edit_array = new Array();
	captive_portal_adv_current_edit_array.push(profile_id);
	if($("input[name=cpa_profile_name]").val().trim() == "") {
		captive_portal_adv_edit_idx = 1;
		set_tab_and_action_btn();
		alert("<#JS_fieldblank#>");
		$("input[name=cpa_profile_name]").focus();
		return false;
	}
	else if(!Block_chars($("input[name=cpa_profile_name]")[0], ["<", ">"])) {
		captive_portal_adv_edit_idx = 1;
		set_tab_and_action_btn();
		$("input[name=cpa_profile_name]").focus();
		return false;
	}

	captive_portal_adv_current_edit_array.push($("input[name=cpa_profile_name]").val().trim());

	var _away_set_sec = 0; 
	if($("input[name=cpa_away_timeout]:checked")[0].id == "cpa_away_timeout_unlimited") {
		captive_portal_adv_current_edit_array.push(0);
	}
	else {
		if(!validator.numberRange($("input[name=cpa_away_set]")[0], 1, 999)) {
			captive_portal_adv_edit_idx = 1;
			set_tab_and_action_btn();
			$("input[name=cpa_away_set]").focus();
			return false;
		}
		switch($("select[name=cpa_away_timeout_unit]").val()) {
			case "hour" :
				_away_set_sec = parseInt($("input[name=cpa_away_set]").val()) * 60 * 60;
				break;
			case "minute" :
				_away_set_sec = parseInt($("input[name=cpa_away_set]").val()) * 60;
				break;
		}
		if(_away_set_sec > 65536) {
			alert("A maximum of 18 hours.");/*untranslated*/
			captive_portal_adv_edit_idx = 1;
			set_tab_and_action_btn();
			$("input[name=cpa_away_set]").focus();
			return false;
		}
		captive_portal_adv_current_edit_array.push(_away_set_sec);
	}
	var _session_set_sec = 0; 
	if($("input[name=cpa_session_timeout]:checked")[0].id == "cpa_session_timeout_unlimited") {
		captive_portal_adv_current_edit_array.push(0);
	}
	else {
		if(!validator.numberRange($("input[name=cpa_session_set]")[0], 1, 999)) {
			captive_portal_adv_edit_idx = 1;
			set_tab_and_action_btn();
			$("input[name=cpa_session_set]").focus();
			return false;
		}
		
		switch($("select[name=cpa_session_timeout_unit]").val()) {
			case "day" :
				_session_set_sec = parseInt($("input[name=cpa_session_set]").val()) * 60 * 60 * 24;
				break;
			case "hour" :
				_session_set_sec = parseInt($("input[name=cpa_session_set]").val()) * 60 * 60;
				break;
			case "minute" :
				_session_set_sec = parseInt($("input[name=cpa_session_set]").val()) * 60;
				break;
		}
		if(_session_set_sec > 31536000) {
			alert("A maximum of 365 days.");/*untranslated*/
			captive_portal_adv_edit_idx = 1;
			set_tab_and_action_btn();
			$("input[name=cpa_session_set]").focus();
			return false;
		}
		captive_portal_adv_current_edit_array.push(_session_set_sec);
	}

	if( (_session_set_sec < _away_set_sec) && (_session_set_sec != 0) ) {
		alert("Idle Timeout must be less than Connection Timeout.");/*untranslated*/
		captive_portal_adv_edit_idx = 1;
		set_tab_and_action_btn();
		$("input[name=cpa_away_set]").focus();
		return false;
	}

	var cpa_landing_page = "";
	if($("input[name=cpa_landing_page]").val() != "") {
		cpa_landing_page = $("input[name=cpa_landing_page]").val().trim();
		if(!validator.isValidURL(cpa_landing_page)) {
			captive_portal_adv_edit_idx = 1;
			set_tab_and_action_btn();
			$("input[name=cpa_landing_page]").focus();
			return false;
		}
	}

	captive_portal_adv_current_edit_array.push(cpa_landing_page);
	
	var _at_least_wl_if_flag = false;
	if(!validator.stringSSID($("input[name=wl_0]")[0]))
		return false;
	if($("#cb_wl_0").prop("checked")) {
		_at_least_wl_if_flag = true;
	}
	document.form.captive_portal_adv_2g_ssid.value = $("input[name=wl_0]").val().trim();
	if(wl_info.band5g_support) {
		if(!validator.stringSSID($("input[name=wl_1]")[0]))
			return false;
		if($("#cb_wl_1").prop("checked")) {
			_at_least_wl_if_flag = true;
		}
		document.form.captive_portal_adv_5g_ssid.value = $("input[name=wl_1]").val().trim();
	}
	if(wl_info.band5g_2_support) {
		if(!validator.stringSSID($("input[name=wl_2]")[0]))
			return false;
		if($("#cb_wl_2").prop("checked")) {
			_at_least_wl_if_flag = true;
		}
		document.form.captive_portal_adv_5g_2_ssid.value = $("input[name=wl_2]").val().trim();
	}
	if(!_at_least_wl_if_flag) {
		captive_portal_adv_edit_idx = 1;
		set_tab_and_action_btn();
		alert("Please select at least one <#QIS_finish_wireless_item1#>.");/*untranslated*/
		return false;
	}
	var wl_if = "";
	var empty_wl_idx = get_captive_portal_wl_idx("captivePortal");
	var captive_portal_adv_2g_if = "off";
	var captive_portal_adv_5g_if = "off";
	var captive_portal_adv_5g_2_if = "off";

	var check_used_profile_wl_if_status = function(_unit, _subunit) {
		var _wl_if_status = false;
		if(Object.keys(used_profile_wl_if).length > 0) {
			if(used_profile_wl_if[profile_id].search("wl" + _unit + "." + _subunit) != "-1")
				_wl_if_status = true;
		}
		return _wl_if_status;
	};

	var gn_overwrite_hint = "";
	if(empty_wl_idx != "") {
		if($("#cb_wl_0").prop("checked")) {
			wl_if += "wl0." + empty_wl_idx;
			if(check_used_profile_wl_if_status(0, empty_wl_idx ))
				captive_portal_adv_2g_if = "wl0." + empty_wl_idx;
			if(!check_gn_if_status(empty_wl_idx, gn_array_2g) && captive_portal_adv_2g_if == "off")
				gn_overwrite_hint += "<#Guest_Network#> " + empty_wl_idx + " - " + wl_nband_title[0] + " will be overwrite.\n\n";
		}
		if(wl_info.band5g_support) {
			if($("#cb_wl_1").prop("checked")) {
				wl_if += "wl1." + empty_wl_idx;
				if(check_used_profile_wl_if_status(1, empty_wl_idx ))
					captive_portal_adv_5g_if = "wl1." + empty_wl_idx;
				if(!check_gn_if_status(empty_wl_idx, gn_array_5g) && captive_portal_adv_5g_if == "off")
					gn_overwrite_hint += "<#Guest_Network#> " + empty_wl_idx + " - " + wl_nband_title[1] + " will be overwrite.\n\n";
			}
		}
		if(wl_info.band5g_2_support) {
			if($("#cb_wl_2").prop("checked")) {
				wl_if += "wl2." + empty_wl_idx;
				if(check_used_profile_wl_if_status(2, empty_wl_idx ))
					captive_portal_adv_5g_2_if = "wl2." + empty_wl_idx;
				if(!check_gn_if_status(empty_wl_idx, gn_array_5g_2) && captive_portal_adv_5g_2_if == "off")
					gn_overwrite_hint += "<#Guest_Network#> " + empty_wl_idx + " - " + wl_nband_title[2] + " will be overwrite.";
			}
		}
	}
	captive_portal_adv_current_edit_array.push(wl_if);

	captive_portal_adv_current_edit_array.push("");
	
	//0: disable, 1: Account/password
	if(component_array["component_account"]) {
		captive_portal_adv_current_edit_array.push("1");
	 }
	 else {
	 	captive_portal_adv_current_edit_array.push("0");
	 }


	if($("input[name=cpa_verification]:checked").val() == "0") { //local
		captive_portal_adv_current_edit_array.push("0");
		captive_portal_adv_current_edit_array.push("");//cpa_radius_server_ipaddr
		captive_portal_adv_current_edit_array.push("");//cpa_radius_server_port
		captive_portal_adv_current_edit_array.push("");//cpa_radius_key
		captive_portal_adv_current_edit_array.push("");//cpa_radius_nas_id
	}
	else {
		captive_portal_adv_current_edit_array.push("1");

		if(!validator.isLegalIP($("input[name=cpa_radius_server_ipaddr]")[0], "")) {
			return false;
		}
		captive_portal_adv_current_edit_array.push($("input[name=cpa_radius_server_ipaddr]").val());

		if(!validator.range($("input[name=cpa_radius_server_port]")[0], 0, 65535))
			return false;
		captive_portal_adv_current_edit_array.push($("input[name=cpa_radius_server_port]").val());

		if(!validator.string($("input[name=cpa_radius_key]")[0]))
			return false;
		captive_portal_adv_current_edit_array.push($("input[name=cpa_radius_key]").val());

		if(!validator.string($("input[name=cpa_radius_nas_id]")[0]))
			return false;
		captive_portal_adv_current_edit_array.push($("input[name=cpa_radius_nas_id]").val());
	}

	//profile active (1) or deactivate (0)
	captive_portal_adv_current_edit_array.push(1);

	//bandwidth limiter
	if($("input[name=cpa_bw_dl]:checked")[0].id == "cpa_bw_dl_unlimited") {
		captive_portal_adv_current_edit_array.push(0);
	}
	else {
		if(($("input[name=cpa_bw_dl_set]").val().split(".").length > 2 || $("input[name=cpa_bw_dl_set]").val() < 0.1) || isNaN(parseFloat($("input[name=cpa_bw_dl_set]").val()))) {
			captive_portal_adv_edit_idx = 1;
			set_tab_and_action_btn();
			alert("<#min_bound#> : 0.1 Mb/s");
			$("input[name=cpa_bw_dl_set]").focus();
			return false;
		}
		captive_portal_adv_current_edit_array.push($("input[name=cpa_bw_dl_set]").val().trim()*1024);
	}

	if($("input[name=cpa_bw_ul]:checked")[0].id == "cpa_bw_ul_unlimited") {
		captive_portal_adv_current_edit_array.push(0);
	}
	else {
		if(($("input[name=cpa_bw_ul_set]").val().split(".").length > 2 || $("input[name=cpa_bw_ul_set]").val() < 0.1) || isNaN(parseFloat($("input[name=cpa_bw_ul_set]").val()))) {
			captive_portal_adv_edit_idx = 1;
			set_tab_and_action_btn();
			alert("<#min_bound#> : 0.1 Mb/s");
			$("input[name=cpa_bw_ul_set]").focus();
			return false;
		}
		captive_portal_adv_current_edit_array.push($("input[name=cpa_bw_ul_set]").val().trim()*1024);
	}

	//save splash page setting
	var check_pattern = function(obj) {
		switch(obj) {
			case "component_button" :
				edit_component_button();
				alert("Button is not set!");
				break;
			case "component_background" :
				edit_component_background();
				alert("Background is not set!");
				break;
		}
		event.stopPropagation();
	};
	
	var set_attribute = function(obj) {
		var html = "{";
		html += '"style_width": "' + obj.style_width + '",';
		html += '"style_height": "' + obj.style_height + '",';
		html += '"style_top": "' + obj.style_top + '",';
		html += '"style_left": "' + obj.style_left + '",';
		html += '"style_z_index": "' + obj.style_z_index + '",';
		html += '"style_background_color": "' + obj.style_background_color + '",';
		html += '"style_opacity": "' + obj.style_opacity + '",';
		html += '"style_color": "' + obj.style_color + '"';
		html += '}';
		return html;
	};

	var walled_garden_list = "";
	var _component_obj = [];
	var splash_page_adv_attribute = ''; 
	splash_page_adv_attribute += '{\n'; //start
	if(component_array["component_image"]) {
		var _component_obj_array = Object.keys(component_array["component_image"]);
		var _component_obj_length = Object.keys(component_array["component_image"]).length;
		if(_component_obj_length > 0) {
			splash_page_adv_attribute += '"component_image":\n';
			splash_page_adv_attribute += '[\n';
			for(var i = 0; i < _component_obj_length; i += 1) {
				var _idx = _component_obj_array[i];
					_component_obj = component_array["component_image"][_idx];
					splash_page_adv_attribute += '{\n';
					splash_page_adv_attribute += '"idx": "' + _idx + '",\n';
					splash_page_adv_attribute += '"attribute": ' + set_attribute(_component_obj.attribute) + ',\n';
					splash_page_adv_attribute += '"image_type": "' + _component_obj.image_type + '",\n';
					splash_page_adv_attribute += '"image_base64": "' + _component_obj.image_base64 + '",\n';
					var image_walled_garden = "";
					if(_component_obj.image_walled_garden != undefined)
						image_walled_garden = _component_obj.image_walled_garden;
					splash_page_adv_attribute += '"image_walled_garden": "' + image_walled_garden + '"\n';
					if(image_walled_garden != "") {
						walled_garden_list += encode_decode_text(image_walled_garden, "decode") + ",";
					}
					if( i  == (_component_obj_length - 1)) {
						splash_page_adv_attribute += '}\n';
					}
					else {
						splash_page_adv_attribute += '},\n';
					}
			}
			splash_page_adv_attribute += '],\n';
		}
	}
	if(component_array["component_text"]) {
		var _component_obj_array = Object.keys(component_array["component_text"]);
		var _component_obj_length = Object.keys(component_array["component_text"]).length;
		if(_component_obj_length > 0) {
			splash_page_adv_attribute += '"component_text":\n';
			splash_page_adv_attribute += '[\n';
			for(var i = 0; i < _component_obj_length; i += 1) {
				var _idx = _component_obj_array[i];
					_component_obj = component_array["component_text"][_idx];
					splash_page_adv_attribute += '{\n';
					splash_page_adv_attribute += '"idx": "' + _idx + '",\n';
					splash_page_adv_attribute += '"attribute": ' + set_attribute(_component_obj.attribute) + ',\n';
					splash_page_adv_attribute += '"text_content": "' + _component_obj.text_content + '",\n';
					splash_page_adv_attribute += '"text_font_weight": "' + _component_obj.text_font_weight + '",\n';
					splash_page_adv_attribute += '"text_font_style": "' + _component_obj.text_font_style + '",\n';
					splash_page_adv_attribute += '"text_font_family": "' + _component_obj.text_font_family + '",\n';
					splash_page_adv_attribute += '"text_font_size": "' + _component_obj.text_font_size + '",\n';
					var text_walled_garden = "";
					if(_component_obj.text_walled_garden != undefined)
						text_walled_garden = _component_obj.text_walled_garden;
					splash_page_adv_attribute += '"text_walled_garden": "' + text_walled_garden + '"\n';
					if(text_walled_garden != "") {
						walled_garden_list += encode_decode_text(text_walled_garden, "decode") + ",";
					}
					if( i  == (_component_obj_length - 1)) {
						splash_page_adv_attribute += '}\n';
					}
					else {
						splash_page_adv_attribute += '},\n';
					}
			}
			splash_page_adv_attribute += '],\n';
		}
	}
	if(component_array["component_eula"]) {
		_component_obj = component_array["component_eula"][0];
		splash_page_adv_attribute += '"component_eula": {\n';
		splash_page_adv_attribute += '"attribute": ' + set_attribute(_component_obj.attribute) + ',\n';
		splash_page_adv_attribute += '"eula_terms_service_title": "' + _component_obj.eula_terms_service_title + '",\n';
		splash_page_adv_attribute += '"eula_terms_service": "' + _component_obj.eula_terms_service + '",\n';
		splash_page_adv_attribute += '"eula_terms_service_font_color": "' + _component_obj.eula_terms_service_font_color + '",\n';
		splash_page_adv_attribute += '"eula_terms_service_background_color": "' + _component_obj.eula_terms_service_background_color + '",\n';
		splash_page_adv_attribute += '"eula_terms_service_text": "' + _component_obj.eula_terms_service_text + '",\n';
		splash_page_adv_attribute += '"eula_terms_service_hyperlink": "' + _component_obj.eula_terms_service_hyperlink + '",\n';
		splash_page_adv_attribute += '"eula_terms_service_hyperlink_color": "' + _component_obj.eula_terms_service_hyperlink_color + '"\n';
		splash_page_adv_attribute += '},\n';
	 }
	if(component_array["component_account"]) {
		_component_obj = component_array["component_account"][0];
		splash_page_adv_attribute += '"component_account": {\n';
		splash_page_adv_attribute += '"attribute": ' + set_attribute(_component_obj.attribute) + ',\n';
		splash_page_adv_attribute += '"account_auth_type": "' + _component_obj.account_auth_type + '",\n';
		splash_page_adv_attribute += '"account_username": "' + _component_obj.account_username + '",\n';
		splash_page_adv_attribute += '"account_password": "' + _component_obj.account_password + '",\n';
		splash_page_adv_attribute += '"account_input_box_background_color": "' + _component_obj.account_input_box_background_color + '",\n';
		splash_page_adv_attribute += '"account_input_box_font_color": "' + _component_obj.account_input_box_font_color + '",\n';
		splash_page_adv_attribute += '"account_input_box_opacity": "' + _component_obj.account_input_box_opacity + '"\n';
		splash_page_adv_attribute += '},\n';
	 }
	if(component_array["component_button"]) {
		_component_obj = component_array["component_button"][0];
		splash_page_adv_attribute += '"component_button": {\n';
		splash_page_adv_attribute += '"attribute": ' + set_attribute(_component_obj.attribute) + ',\n';
		splash_page_adv_attribute += '"button_label": "' + _component_obj.button_label + '",\n';
		splash_page_adv_attribute += '"button_work_progress": "' + _component_obj.button_work_progress + '",\n';
		splash_page_adv_attribute += '"button_denied": "' + _component_obj.button_denied + '"\n';
		splash_page_adv_attribute += '},\n';
	}
	else {
		captive_portal_adv_edit_idx = 2;
		set_tab_and_action_btn();
		check_pattern("component_button");
		return false;
	}
	if(component_array["component_background"]) {
		_component_obj = component_array["component_background"][0];
		splash_page_adv_attribute += '"component_background": {\n';
		splash_page_adv_attribute += '"attribute": ' + set_attribute(_component_obj.attribute) + ',\n';
		splash_page_adv_attribute += '"image_type": "' + _component_obj.image_type + '",\n';
		splash_page_adv_attribute += '"image_base64": "' + _component_obj.image_base64 + '"\n';
		splash_page_adv_attribute += '}\n';
	}
	else {
		captive_portal_adv_edit_idx = 2;
		set_tab_and_action_btn();
		check_pattern("component_background");
		return false;
	}
	splash_page_adv_attribute += '}\n'; //end

	var splash_page_adv_css = "";
	splash_page_adv_css += ".component_container {\n";
	splash_page_adv_css += "position: absolute;\n";
	splash_page_adv_css += "top: 0;\n";
	splash_page_adv_css += "right: 0;\n";
	splash_page_adv_css += "bottom: 0;\n";
	splash_page_adv_css += "left: 0;\n";
	splash_page_adv_css += "}\n";
	splash_page_adv_css += "p {\n";
	splash_page_adv_css += "margin-top: 0;\n";
	splash_page_adv_css += "margin-bottom: 0;\n";
	splash_page_adv_css += "}\n";
	splash_page_adv_css += ".component_background {\n";
	splash_page_adv_css += "width: 100%;\n";
	splash_page_adv_css += "height: 100%;\n";
	splash_page_adv_css += "position: absolute;\n";
	splash_page_adv_css += "top: 0;\n";
	splash_page_adv_css += "left: 0;\n";
	splash_page_adv_css += "}\n";
	if(component_array["component_eula"]) {
		splash_page_adv_css += ".terms_service_hyperlink {\n";
		splash_page_adv_css += "font-weight: bolder;\n";
		splash_page_adv_css += "text-decoration: underline;\n";
		splash_page_adv_css += "color: #00B0FF;\n";
		splash_page_adv_css += "cursor: pointer;\n";
		splash_page_adv_css += "}\n";
		splash_page_adv_css += ".terms_service {\n";
		splash_page_adv_css += "position: fixed;\n";
		splash_page_adv_css += "width: 80vw;\n";
		splash_page_adv_css += "height: 68vh;\n";
		splash_page_adv_css += "left: 7vw;\n";
		splash_page_adv_css += "top: 13vh;\n";
		splash_page_adv_css += "z-index: 200;\n";
		splash_page_adv_css += "padding: 3vh 3vw;\n";
		splash_page_adv_css += "border-radius: 2vw;\n";
		splash_page_adv_css += "display: none;\n";
		splash_page_adv_css += "box-shadow: 0 19px 38px rgba(0,0,0,0.30), 0 15px 12px rgba(0,0,0,0.22);\n";
		splash_page_adv_css += "}\n";
		splash_page_adv_css += ".term_service_title {\n";
		splash_page_adv_css += "font-weight: bolder;\n";
		splash_page_adv_css += "text-align: center;\n";
		splash_page_adv_css += "font-family: Microsoft JhengHei;\n";
		splash_page_adv_css += "border-bottom: 1px solid;\n";
		splash_page_adv_css += "padding-bottom: 2%;\n";
		splash_page_adv_css += "}\n";
		splash_page_adv_css += ".term_service_text {\n";
		splash_page_adv_css += "position: absolute;\n";
		splash_page_adv_css += "bottom: 2%;\n";
		splash_page_adv_css += "top: 2%;\n";
		splash_page_adv_css += "height: 96%;\n";
		splash_page_adv_css += "overflow: auto;\n";
		splash_page_adv_css += "overflow-x: hidden;\n";
		splash_page_adv_css += "font-family: Arial, Helvetica, sans-serif;\n";
		splash_page_adv_css += "white-space: normal;\n";
		splash_page_adv_css += "word-break: break-all;\n";
		splash_page_adv_css += "}\n";
		splash_page_adv_css += ".terms_service_close {\n";
		splash_page_adv_css += "width: 5vmin;\n";
		splash_page_adv_css += "height: 5vmin;\n";
		splash_page_adv_css += "cursor: pointer;\n";
		splash_page_adv_css += "position: absolute;\n";
		splash_page_adv_css += "right: 1vw;\n";
		splash_page_adv_css += "top: 1vmin;\n";
		splash_page_adv_css += "}\n";
	}

	var splash_page_adv_html = "";
	splash_page_adv_html += "<!DOCTYPE html PUBLIC '-//W3C//DTD XHTML 1.0 Transitional//EN' 'http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd'>\n";
	splash_page_adv_html += "<html xmlns='http://www.w3.org/1999/xhtml'>\n";
	splash_page_adv_html += "<html xmlns:v>\n";
	splash_page_adv_html += "<head>\n";
	splash_page_adv_html += "<meta name='viewport' content='width=device-width, initial-scale=1.0, user-scalable=no'>\n";
	splash_page_adv_html += "<meta http-equiv='X-UA-Compatible' content='IE=Edge'/>\n";
	splash_page_adv_html += "<meta http-equiv='Content-Type' content='text/html; charset=utf-8' />\n";
	splash_page_adv_html += "<meta HTTP-EQUIV='Pragma' CONTENT='no-cache'>\n";
	splash_page_adv_html += "<meta HTTP-EQUIV='Expires' CONTENT='-1'>\n";
	splash_page_adv_html += "<title><#FreeWiFi_title#></title>\n";
	splash_page_adv_html += "<link rel='stylesheet' type='text/css' href='Uam.css'>\n";
	splash_page_adv_html += "<_INCLUDE_JQUERY_>\n";
	splash_page_adv_html += "<_INCLUDE_UAM_>\n";

	splash_page_adv_html += "<_TAG_START_>\n";
	splash_page_adv_html += "var htmlEnDeCode = (function() {\n";
	splash_page_adv_html += "var charToEntityRegex,\n";
	splash_page_adv_html += "entityToCharRegex,\n";
	splash_page_adv_html += "charToEntity,\n";
	splash_page_adv_html += "entityToChar;\n";

	splash_page_adv_html += "function resetCharacterEntities() {\n";
	splash_page_adv_html += "charToEntity = {};\n";
	splash_page_adv_html += "entityToChar = {};\n";

	splash_page_adv_html += "addCharacterEntities({\n";
	splash_page_adv_html += "'&amp;'     :   '&',\n";
	splash_page_adv_html += "'&gt;'      :   '>',\n";
	splash_page_adv_html += "'&lt;'      :   '<',\n";
	splash_page_adv_html += "'&quot;'    :   '\"',\n";
	splash_page_adv_html += "'&#39;'     :   '\\''\n";
	splash_page_adv_html += "});\n";
	splash_page_adv_html += "}\n";

	splash_page_adv_html += "function addCharacterEntities(newEntities) {\n";
	splash_page_adv_html += "var charKeys = [],\n";
	splash_page_adv_html += "entityKeys = [],\n";
	splash_page_adv_html += "key, echar;\n";
	splash_page_adv_html += "for (key in newEntities) {\n";
	splash_page_adv_html += "echar = newEntities[key];\n";
	splash_page_adv_html += "entityToChar[key] = echar;\n";
	splash_page_adv_html += "charToEntity[echar] = key;\n";
	splash_page_adv_html += "charKeys.push(echar);\n";
	splash_page_adv_html += "entityKeys.push(key);\n";
	splash_page_adv_html += "}\n";
	splash_page_adv_html += "charToEntityRegex = new RegExp('(' + charKeys.join('|') + ')', 'g');\n";
	splash_page_adv_html += "entityToCharRegex = new RegExp('(' + entityKeys.join('|') + '|&#[0-9]{1,5};' + ')', 'g');\n";
	splash_page_adv_html += "}\n";

	splash_page_adv_html += "function htmlEncode(value){\n";
	splash_page_adv_html += "var htmlEncodeReplaceFn = function(match, capture) {\n";
	splash_page_adv_html += "return charToEntity[capture];\n";
	splash_page_adv_html += "};\n";

	splash_page_adv_html += "return (!value) ? value : String(value).replace(charToEntityRegex, htmlEncodeReplaceFn);\n";
	splash_page_adv_html += "}\n";

	splash_page_adv_html += "function htmlDecode(value) {\n";
	splash_page_adv_html += "var htmlDecodeReplaceFn = function(match, capture) {\n";
	splash_page_adv_html += "return (capture in entityToChar) ? entityToChar[capture] : String.fromCharCode(parseInt(capture.substr(2), 10));\n";
	splash_page_adv_html += "};\n";

	splash_page_adv_html += "return (!value) ? value : String(value).replace(entityToCharRegex, htmlDecodeReplaceFn);\n";
	splash_page_adv_html += "}\n";

	splash_page_adv_html += "resetCharacterEntities();\n";

	splash_page_adv_html += "return {\n";
	splash_page_adv_html += "htmlEncode: htmlEncode,\n";
	splash_page_adv_html += "htmlDecode: htmlDecode\n";
	splash_page_adv_html += "};\n";
	splash_page_adv_html += "})();\n";
	splash_page_adv_html += "window.moveTo(0,0);\n";
	splash_page_adv_html += "var windw_width = screen.width;\n";
	splash_page_adv_html += "var windw_height = screen.height;\n";
	splash_page_adv_html += "if(isMobile()) {\n";
	splash_page_adv_html += "var supportsOrientationChange = 'onorientationchange' in window,\n";
	splash_page_adv_html += "orientationEvent = supportsOrientationChange ? 'orientationchange' : 'resize';\n";

	splash_page_adv_html += "window.addEventListener(orientationEvent, function() {\n";
	splash_page_adv_html += "if(window.orientation == undefined) {\n";
	splash_page_adv_html += "setTimeout(function() {\n";
	splash_page_adv_html += "windw_width = screen.width;\n";
	splash_page_adv_html += "windw_height = screen.height;\n";
	splash_page_adv_html += "re_tune_size();\n";
	splash_page_adv_html += "},100);\n";
	splash_page_adv_html += "}\n";
	splash_page_adv_html += "else {\n";
	splash_page_adv_html += "switch(window.orientation) {\n";
	splash_page_adv_html += "case -90 :\n";
	splash_page_adv_html += "case 90 :\n";
	splash_page_adv_html += "windw_width = (screen.height > screen.width) ? screen.height : screen.width;\n";
	splash_page_adv_html += "windw_height = (screen.height > screen.width) ? screen.width : screen.height;\n";
	splash_page_adv_html += "break; \n";
	splash_page_adv_html += "default :\n";
	splash_page_adv_html += "windw_width = (screen.height < screen.width) ? screen.height : screen.width;\n";
	splash_page_adv_html += "windw_height = (screen.height < screen.width) ? screen.width : screen.height;\n";
	splash_page_adv_html += "break; \n";
	splash_page_adv_html += "}\n";
	splash_page_adv_html += "re_tune_size();\n";
	splash_page_adv_html += "}\n";
	splash_page_adv_html += "}, false);\n";
	splash_page_adv_html += "}\n";
	splash_page_adv_html += "window.onresize = function() {\n";
	splash_page_adv_html += "if(!isMobile()) {\n";
	splash_page_adv_html += "initial();\n";
	splash_page_adv_html += "}\n";
	splash_page_adv_html += "};\n";
	splash_page_adv_html += "function re_tune_size() {\n";
	splash_page_adv_html += "if(isMobile()) {\n";
	splash_page_adv_html += "$('meta[name=viewport]').attr('content', 'initial-scale=1.0,maximum-scale=1.0,minimum-scale=1.0,user-scalable=no,width=' + windw_width + ',height=' + windw_height);\n";
	splash_page_adv_html += "var mobile_ratio_size = ((windw_width / 340));\n";
	splash_page_adv_html += "document.getElementById('component_background').style.width = windw_width + 'px';\n";
	splash_page_adv_html += "if(windw_width > windw_height) {\n";
	splash_page_adv_html += "windw_width = windw_height;\n";
	splash_page_adv_html += "mobile_ratio_size =  ((windw_width / 340));\n";
	splash_page_adv_html += "windw_height = windw_width * 1.5;\n";
	splash_page_adv_html += "}\n";
	splash_page_adv_html += "document.getElementById('component_container').style.width = (340 * mobile_ratio_size) + 'px';\n";
	splash_page_adv_html += "document.getElementById('component_container').style.height = (510 * mobile_ratio_size) + 'px';\n";
	splash_page_adv_html += "document.getElementById('component_container').style.margin = '0 auto';\n";
	splash_page_adv_html += "var _component_background_height = ((510 * mobile_ratio_size) > windw_height) ? (510 * mobile_ratio_size) : windw_height;\n";
	splash_page_adv_html += "document.getElementById('component_background').style.height = _component_background_height + 'px';\n";
	splash_page_adv_html += "gen_component_html(mobile_ratio_size, mobile_ratio_size);\n";
	splash_page_adv_html += "}\n";
	splash_page_adv_html += "else {\n";
	splash_page_adv_html += "windw_width = window.innerWidth || document.documentElement.clientWidth || document.body.clientWidth;\n";
	splash_page_adv_html += "windw_height = window.innerHeight || document.documentElement.clientHeight || document.body.clientHeight;\n";
	splash_page_adv_html += "$('meta[name=viewport]').attr('content', 'initial-scale=1.0,maximum-scale=1.0,minimum-scale=1.0,user-scalable=no,width=' + windw_width + ',height=' + windw_height);\n";
	splash_page_adv_html += "document.getElementById('component_container').style.width = (340 * 1.4) + 'px';\n";
	splash_page_adv_html += "document.getElementById('component_container').style.height = (510 * 1.4) + 'px';\n";
	splash_page_adv_html += "document.getElementById('component_container').style.margin = 'auto';\n";
	splash_page_adv_html += "document.getElementById('component_background').style.width = windw_width + 'px';\n";
	splash_page_adv_html += "document.getElementById('component_background').style.height = windw_height + 'px';\n";
	splash_page_adv_html += "gen_component_html(1.4, 1.4);\n";
	splash_page_adv_html += "}\n";
	splash_page_adv_html += "}\n";
	splash_page_adv_html += "var splash_page_preview_array = new Array();\n";
	splash_page_adv_html += "function get_splash_page_attribute() {\n";
	splash_page_adv_html += "var result = 'NoData';\n";
	splash_page_adv_html += "$.ajax({\n";
	splash_page_adv_html += "url: '/Uam.json',\n";
	splash_page_adv_html += "dataType: 'json',\n";
	splash_page_adv_html += "error: function(xhr){\n";
	splash_page_adv_html += "setTimeout('get_splash_page_attribute();', 1000);\n";
	splash_page_adv_html += "},\n";
	splash_page_adv_html += "success: function(response) {\n";
	splash_page_adv_html += "splash_page_preview_array = [];\n";
	splash_page_adv_html += "var component_array_element = '';\n";
	splash_page_adv_html += "if(response.component_image) {\n";
	splash_page_adv_html += "component_array_element = JSON.parse( JSON.stringify( response.component_image ) );\n";
	splash_page_adv_html += "for(var i = 0; i < component_array_element.length; i += 1) {\n";
	splash_page_adv_html += "var _idx = component_array_element[i].idx;\n";
	splash_page_adv_html += "if(splash_page_preview_array['component_image'] == undefined)\n";
	splash_page_adv_html += "splash_page_preview_array['component_image'] = [];\n";
	splash_page_adv_html += "splash_page_preview_array['component_image'][_idx] = component_array_element[i];\n";
	splash_page_adv_html += "delete splash_page_preview_array['component_image'][_idx].idx;\n";
	splash_page_adv_html += "}\n";
	splash_page_adv_html += "}\n";
	splash_page_adv_html += "if(response.component_text) {\n";
	splash_page_adv_html += "component_array_element = JSON.parse( JSON.stringify( response.component_text ) );\n";
	splash_page_adv_html += "for(var i = 0; i < component_array_element.length; i += 1) {\n";
	splash_page_adv_html += "var _idx = component_array_element[i].idx;\n";
	splash_page_adv_html += "if(splash_page_preview_array['component_text'] == undefined)\n";
	splash_page_adv_html += "splash_page_preview_array['component_text'] = [];\n";
	splash_page_adv_html += "splash_page_preview_array['component_text'][_idx] = component_array_element[i];\n";
	splash_page_adv_html += "delete splash_page_preview_array['component_text'][_idx].idx;\n";
	splash_page_adv_html += "}\n";
	splash_page_adv_html += "}\n";
	splash_page_adv_html += "if(response.component_eula) {\n";
	splash_page_adv_html += "component_array_element = JSON.parse( JSON.stringify( response.component_eula ) );\n";
	splash_page_adv_html += "splash_page_preview_array['component_eula'] = [component_array_element];\n";
	splash_page_adv_html += "}\n";
	splash_page_adv_html += "if(response.component_account) {\n";
	splash_page_adv_html += "component_array_element = JSON.parse( JSON.stringify( response.component_account ) );\n";
	splash_page_adv_html += "splash_page_preview_array['component_account'] = [component_array_element];\n";
	splash_page_adv_html += "}\n";
	splash_page_adv_html += "if(response.component_button) {\n";
	splash_page_adv_html += "component_array_element = JSON.parse( JSON.stringify( response.component_button ) );\n";
	splash_page_adv_html += "splash_page_preview_array['component_button'] = [component_array_element];\n";
	splash_page_adv_html += "}\n";
	splash_page_adv_html += "if(response.component_background) {\n";
	splash_page_adv_html += "component_array_element = JSON.parse( JSON.stringify( response.component_background ) );\n";
	splash_page_adv_html += "splash_page_preview_array['component_background'] = [component_array_element];\n";
	splash_page_adv_html += "}\n";
	splash_page_adv_html += "re_tune_size();\n";
	splash_page_adv_html += "}\n";
	splash_page_adv_html += "});\n";
	splash_page_adv_html += "}\n";
	splash_page_adv_html += "function initial() {\n";
	splash_page_adv_html += "var mql = window.matchMedia('(orientation: portrait)');\n";
	splash_page_adv_html += "if(mql.matches) {\n";  
	splash_page_adv_html += "windw_width = (screen.height < screen.width) ? screen.height : screen.width;\n";
	splash_page_adv_html += "windw_height = (screen.height < screen.width) ? screen.width : screen.height;\n";
	splash_page_adv_html += "}\n";
	splash_page_adv_html += "else {  \n";
	splash_page_adv_html += "windw_width = (screen.height > screen.width) ? screen.height : screen.width;\n";
	splash_page_adv_html += "windw_height = (screen.height > screen.width) ? screen.width : screen.height;\n";
	splash_page_adv_html += "}\n";
	splash_page_adv_html += "get_splash_page_attribute();\n";
	splash_page_adv_html += "}\n";
	splash_page_adv_html += "function open_walled_garden(_url) {\n";
	splash_page_adv_html += "if(_url != '') {\n";
	splash_page_adv_html += "window.open('http://' + _url + '');\n";
	splash_page_adv_html += "}\n";
	splash_page_adv_html += "}\n";
	if(component_array["component_eula"]) {
		splash_page_adv_html += "function open_term_service() {\n";
		splash_page_adv_html += "if(isMobile()) {\n";
		splash_page_adv_html += "$('#terms_service').css('display', 'block');\n";
		splash_page_adv_html += "$('#component_container').css('display', 'none');\n";
		splash_page_adv_html += "}\n";
		splash_page_adv_html += "else {\n";
		splash_page_adv_html += "$('#terms_service').fadeIn(300);\n";
		splash_page_adv_html += "$('#component_container').fadeOut(100);\n";
		splash_page_adv_html += "}\n";
		splash_page_adv_html += "}\n";
		splash_page_adv_html += "function close_term_service() {\n";
		splash_page_adv_html += "if(isMobile()) {\n";
		splash_page_adv_html += "$('#terms_service').css('display', 'none');\n";
		splash_page_adv_html += "$('#component_container').css('display', 'block');\n";
		splash_page_adv_html += "}\n";
		splash_page_adv_html += "else {\n";
		splash_page_adv_html += "$('#terms_service').fadeOut(300);\n";
		splash_page_adv_html += "$('#component_container').fadeIn(100);\n";
		splash_page_adv_html += "}\n";
		splash_page_adv_html += "}\n";
	}
	splash_page_adv_html += "function continue_action() {\n";
	splash_page_adv_html += "var valid_account_flag = false;\n";
	splash_page_adv_html += "if(splash_page_preview_array['component_account']) {\n";
	splash_page_adv_html += "valid_account_flag = true;\n";
	splash_page_adv_html += "}\n";
	splash_page_adv_html += "var valid_form = function() {\n";
	splash_page_adv_html += "var _default_account_username = decodeURIComponent(splash_page_preview_array['component_account'][0].account_username).replace(/\"/g, '&#34;').replace(/'/g, '&#39;').toLowerCase();\n";
	splash_page_adv_html += "var _default_account_password = decodeURIComponent(splash_page_preview_array['component_account'][0].account_password).replace(/\"/g, '&#34;').replace(/'/g, '&#39;').toLowerCase();\n";
	splash_page_adv_html += "_default_account_username = htmlEnDeCode.htmlDecode(_default_account_username);\n";
	splash_page_adv_html += "_default_account_password = htmlEnDeCode.htmlDecode(_default_account_password);\n";
	splash_page_adv_html += "if(document.getElementById('splash_page_account').value.trim() == '' || document.getElementById('splash_page_account').value.trim().toLowerCase() == _default_account_username) {\n";
	splash_page_adv_html += "document.getElementById('splash_page_account').value = '';\n";
	splash_page_adv_html += "document.getElementById('splash_page_account').focus();\n";
	splash_page_adv_html += "alert('<#JS_fieldblank#>');\n";
	splash_page_adv_html += "return false;\n";
	splash_page_adv_html += "}\n";
	splash_page_adv_html += "if(document.getElementById('splash_page_password').value.trim() == '' || document.getElementById('splash_page_password').value.trim().toLowerCase() == _default_account_password) {\n";
	splash_page_adv_html += "document.getElementById('splash_page_password').value = '';\n";
	splash_page_adv_html += "document.getElementById('splash_page_password').focus();\n";
	splash_page_adv_html += "alert('<#JS_fieldblank#>');\n";
	splash_page_adv_html += "return false;\n";
	splash_page_adv_html += "}\n";
	splash_page_adv_html += "return true;\n";
	splash_page_adv_html += "};\n";
	splash_page_adv_html += "if(splash_page_preview_array['component_eula']) {\n";
	splash_page_adv_html += "var _obj = document.getElementById('eula_check');\n";
	splash_page_adv_html += "if(_obj.checked) {\n";

	if(component_array["component_account"]) {
		splash_page_adv_html += "if(valid_account_flag) {\n";
		splash_page_adv_html += "if(valid_form())\n";
		splash_page_adv_html += "formSubmit(1);\n";
		splash_page_adv_html += "}\n";
	}
	else
		splash_page_adv_html += "formSubmit(0);\n";

	splash_page_adv_html += "}\n";
	splash_page_adv_html += "else {\n";
	splash_page_adv_html += "alert('You must agree to the terms of service before continuing.');\n";
	splash_page_adv_html += "}\n";
	splash_page_adv_html += "}\n";
	splash_page_adv_html += "else {\n";

	if(component_array["component_account"]) {
		splash_page_adv_html += "if(valid_account_flag) {\n";
		splash_page_adv_html += "if(valid_form())\n";
		splash_page_adv_html += "formSubmit(1);\n";
	splash_page_adv_html += "}\n";
	}
	else
		splash_page_adv_html += "formSubmit(0);\n";

	splash_page_adv_html += "}\n";
	splash_page_adv_html += "}\n";
	if(component_array['component_account']) {
		splash_page_adv_html += "function simulate_placeholder_focus(_obj, _type, _fontColor) {\n";
		splash_page_adv_html += "if(!_obj.haswriting) {\n";
		splash_page_adv_html += "_obj.type = _type;\n";
		splash_page_adv_html += "_obj.style.fontStyle = 'initial';\n";
		splash_page_adv_html += "_obj.style.fontWeight = 'bolder';\n";
		splash_page_adv_html += "_obj.style.color = _fontColor;\n";
		splash_page_adv_html += "_obj.value = '';\n";
		splash_page_adv_html += "}\n";
		splash_page_adv_html += "}\n";
		splash_page_adv_html += "function simulate_placeholder_blur(_obj, _type, _fontColor, _placeholderText) {\n";
		splash_page_adv_html += "if(!_obj.value) {\n";
		splash_page_adv_html += "_obj.type = 'text';\n";
		splash_page_adv_html += "_obj.style.color = transformHEXtoRGB(_fontColor).replace(')', ', 0.8)').replace('rgb', 'rgba');\n";
		splash_page_adv_html += "_obj.style.fontStyle = 'oblique';\n";
		splash_page_adv_html += "_obj.style.fontWeight = 'initial';\n";
		splash_page_adv_html += "_obj.value = decodeURIComponent(_placeholderText);\n";
		splash_page_adv_html += "_obj.haswriting = false;\n";
		splash_page_adv_html += "}\n";
		splash_page_adv_html += "else {\n";
		splash_page_adv_html += "_obj.haswriting = true;\n";
		splash_page_adv_html += "}\n";
		splash_page_adv_html += "}\n";
	}
	splash_page_adv_html += "function gen_component_obj(_component_attr_array, _ratio_width, _ratio_height, _component_type) {\n";
	splash_page_adv_html += "var font_size = (13 * _ratio_width) + 'px';\n";
	splash_page_adv_html += "var _component_width = (parseInt(_component_attr_array.attribute.style_width) * _ratio_width) + 'px';\n";
	splash_page_adv_html += "var _component_height = (parseInt(_component_attr_array.attribute.style_height) * _ratio_height) + 'px';\n";
	splash_page_adv_html += "var _component_top = (parseInt(_component_attr_array.attribute.style_top) * _ratio_height) + 'px';\n";
	splash_page_adv_html += "var _component_left = (parseInt(_component_attr_array.attribute.style_left) * _ratio_width) + 'px';\n";
	splash_page_adv_html += "var _component_zIndex = _component_attr_array.attribute.style_z_index;\n";
	splash_page_adv_html += "var _component_backgroundColor = transformRGBtoHEX(_component_attr_array.attribute.style_background_color);\n";
	splash_page_adv_html += "var _component_opacity = _component_attr_array.attribute.style_opacity;\n";
	splash_page_adv_html += "var _component_color = transformRGBtoHEX(_component_attr_array.attribute.style_color);\n";
	splash_page_adv_html += "var _component_common_attr = 'position:absolute;width:' + _component_width + ';height:' + _component_height + ';top:' + _component_top + ';left:' + _component_left + ';z-index:' + _component_zIndex + ';opacity:' + _component_opacity + ';color:' + _component_color + ';';\n";
	splash_page_adv_html += "var code = '';\n";
	splash_page_adv_html += "switch(_component_type) {\n";
	if(component_array['component_image']) {
		splash_page_adv_html += "case 'image' :\n";
		splash_page_adv_html += "var image_walled_garden = '';\n";
		splash_page_adv_html += "if(_component_attr_array.image_walled_garden) {\n";
		splash_page_adv_html += "image_walled_garden = decodeURIComponent(_component_attr_array.image_walled_garden);\n";
		splash_page_adv_html += "}\n";
		splash_page_adv_html += "var image_cursor = (image_walled_garden == '') ? 'default' : 'pointer';\n";
		splash_page_adv_html += "if(_component_attr_array.image_type == 'image') {\n";
		splash_page_adv_html += 'code += \"<div style=\'\" + _component_common_attr + \"background-image:url(\" + _component_attr_array.image_base64 + \");background-repeat:no-repeat;background-size:\" + _component_width + \" \" + _component_height + \";border-radius:8px;cursor:\" + image_cursor + \";\' onclick=open_walled_garden(\\"\" + image_walled_garden + \"\\");></div>\";\n';
		splash_page_adv_html += "}\n";
		splash_page_adv_html += "else {\n";
		splash_page_adv_html += 'code += \"<div style=\'\" + _component_common_attr + \"background-color:\" + _component_backgroundColor + \";border-radius:8px;cursor:\" + image_cursor + \";\' onclick=\'open_walled_garden(\\"\" + image_walled_garden + \"\\");\'></div>\";\n';
		splash_page_adv_html += "}\n";
		splash_page_adv_html += "break;\n";
	}
	if(component_array['component_text']) {
		splash_page_adv_html += "case 'text' :\n";
		splash_page_adv_html += "var text_font_weight = _component_attr_array.text_font_weight;\n";
		splash_page_adv_html += "var text_font_style =  _component_attr_array.text_font_style;\n";
		splash_page_adv_html += "var text_font_family =  _component_attr_array.text_font_family;\n";
		splash_page_adv_html += "var text_font_size = (parseInt(_component_attr_array.text_font_size) * _ratio_width) + 'px';\n";
		splash_page_adv_html += "var text_walled_garden = '';\n";
		splash_page_adv_html += "if(_component_attr_array.text_walled_garden) {\n";
		splash_page_adv_html += "text_walled_garden = decodeURIComponent(_component_attr_array.text_walled_garden);\n";
		splash_page_adv_html += "}\n";
		splash_page_adv_html += "var text_decoration = (text_walled_garden == '') ? 'none' : 'underline';\n";
		splash_page_adv_html += "var text_cursor = (text_walled_garden == '') ? 'default' : 'pointer';\n";
		splash_page_adv_html += "code += '<div style=' + _component_common_attr + ';overflow:auto;font-size:' + text_font_size + ';font-weight:' + text_font_weight  + ';font-style:' + text_font_style + ';font-family:' + text_font_family + ';>';\n";
		splash_page_adv_html += "var text_content = _component_attr_array.text_content;\n";
		splash_page_adv_html += "text_content = decodeURIComponent(text_content);\n";
		splash_page_adv_html += "text_content = text_content.replace(/\<br\>/g, '\\n');\n";
		splash_page_adv_html += "text_content = htmlEnDeCode.htmlEncode(text_content);\n";
		splash_page_adv_html += "text_content = text_content.replace(/ /g, '&nbsp;').replace(/(?:\\r\\n|\\r|\\n)/g, '<br>');\n";
		splash_page_adv_html += "code +='<div style=text-decoration:\' + text_decoration + \';cursor:\' + text_cursor + \'; onclick=open_walled_garden(\"' + text_walled_garden + '\");>';\n";
		splash_page_adv_html += "code += text_content;\n";
		splash_page_adv_html += "code +='</div>';\n";
		splash_page_adv_html += "code +='</div>';\n";
		splash_page_adv_html += "break;\n";
	}
	if(component_array['component_account']) {
		splash_page_adv_html += "case 'account' :\n";
		splash_page_adv_html += "code += '<div style=' + _component_common_attr + 'font-size:' + font_size + ';>';\n";
		splash_page_adv_html += "var _account_input_box_background_color = transformHEXtoRGB('#' + _component_attr_array.account_input_box_background_color);\n";
		splash_page_adv_html += "var _account_input_box_opacity = _component_attr_array.account_input_box_opacity;\n";
		splash_page_adv_html += "_account_input_box_background_color = _account_input_box_background_color.replace(')', ',' + _account_input_box_opacity + ')').replace('rgb', 'rgba');\n";
		splash_page_adv_html += "var _account_font_color = '#' + _component_attr_array.account_input_box_font_color;\n";
		splash_page_adv_html += "var _account_font_color_rgba = transformHEXtoRGB(_account_font_color).replace(')', ',0.8)').replace('rgb', 'rgba');\n";
		splash_page_adv_html += "var _account_height = Math.floor(parseInt(_component_height) / 2) + 'px';\n";
		splash_page_adv_html += "var _account_imput_height = (Math.floor(parseInt(_component_height) / 2) - 10) + 'px';\n";
		splash_page_adv_html += "var _account_username_ASCII = _component_attr_array.account_username;\n";
		splash_page_adv_html += "var _account_password_ASCII = _component_attr_array.account_password;\n";
		splash_page_adv_html += "var _account_username = htmlEnDeCode.htmlEncode(decodeURIComponent(_account_username_ASCII));\n";
		splash_page_adv_html += "var _account_password = htmlEnDeCode.htmlEncode(decodeURIComponent(_account_password_ASCII));\n";
		splash_page_adv_html += "var _common_css = 'padding-left:2%;width:98%;border:none;border-radius:4px;font-style:oblique;background-color:' + _account_input_box_background_color + ';height:' + _account_imput_height + ';font-size:' + font_size + ';color:' + _account_font_color_rgba + '';\n";
		splash_page_adv_html += "var _margin_top = (6 * _ratio_width) + 'px';\n";
		splash_page_adv_html += "code += '<input id=splash_page_account style=' + _common_css + '; type=text maxlength=64 autocorrect=off autocapitalize=off value=\"' + _account_username + '\" onfocus=simulate_placeholder_focus(this,\"text\",\"' + _account_font_color + '\"); onblur=simulate_placeholder_blur(this,\"text\",\"' + _account_font_color + '\",\"' + _account_username_ASCII + '\"); >';\n";
		splash_page_adv_html += "code += '<input id=splash_page_password style=' + _common_css + ';margin-top:' + _margin_top + '; type=text maxlength=64 autocorrect=off autocapitalize=off value=\"' + _account_password + '\" onfocus=simulate_placeholder_focus(this,\"password\",\"' + _account_font_color + '\"); onblur=simulate_placeholder_blur(this,\"password\",\"' + _account_font_color + '\",\"' + _account_password_ASCII + '\"); >';\n";
		splash_page_adv_html += "code += '</div>';\n";
		splash_page_adv_html += "break;\n";
	}
	if(component_array['component_eula']) {
		splash_page_adv_html += "case 'eula' :\n";
		splash_page_adv_html += "var eula_text = 'I have read and agree to';\n";
		splash_page_adv_html += "var eula_hyperlink = 'the Terms of Service';\n";
		splash_page_adv_html += "var eula_hyperlink_color = '#00B0FF';\n";
		splash_page_adv_html += "if(_component_attr_array.eula_terms_service_text != undefined)\n";
		splash_page_adv_html += "eula_text = _component_attr_array.eula_terms_service_text;\n";
		splash_page_adv_html += "if(_component_attr_array.eula_terms_service_hyperlink != undefined)\n";
		splash_page_adv_html += "eula_hyperlink = _component_attr_array.eula_terms_service_hyperlink;\n";
		splash_page_adv_html += "if(_component_attr_array.eula_terms_service_hyperlink_color != undefined)\n";
		splash_page_adv_html += "eula_hyperlink_color = _component_attr_array.eula_terms_service_hyperlink_color;\n";
		splash_page_adv_html += "code += '<div style=' + _component_common_attr + 'font-size:' + font_size + ';>';\n";
		splash_page_adv_html += "code += '<div style=float:left;>';\n";
		splash_page_adv_html += "code += '<input type=checkbox name=eula_check id=eula_check style=height:' + font_size + ';width:' + font_size + '; onclick=control_bt_status();>';\n";
		splash_page_adv_html += "code += '</div>';\n";
		splash_page_adv_html += "code += '<div>';\n";
		splash_page_adv_html += "code += '<span>' + htmlEnDeCode.htmlEncode(decodeURIComponent(eula_text)) + '</span>&nbsp;<span class=terms_service_hyperlink style=color:' + eula_hyperlink_color + '; onclick=open_term_service();>' + htmlEnDeCode.htmlEncode(decodeURIComponent(eula_hyperlink)) + '</span>';\n";
		splash_page_adv_html += "code += '</div>';\n";
		splash_page_adv_html += "code += '</div>';\n";
		splash_page_adv_html += "break;\n";
	}
	splash_page_adv_html += "case 'button' :\n";
	splash_page_adv_html += "code += '<div id=component_button style=' + _component_common_attr + 'cursor:pointer;text-align:center;font-size:' + font_size + ';line-height:' + _component_height + ';background-color:' + _component_backgroundColor + ';border-radius:8px; onclick=continue_action();>';\n";
	splash_page_adv_html += "code += '' + htmlEnDeCode.htmlEncode(decodeURIComponent(_component_attr_array.button_label)) + '';\n";
	splash_page_adv_html += "code += '</div>';\n";
	splash_page_adv_html += "break;\n";
	splash_page_adv_html += "}\n";
	splash_page_adv_html += "return code;\n";
	splash_page_adv_html += "}\n";
	splash_page_adv_html += "function gen_component_html(_ratio_width, _ratio_height) {\n";
	splash_page_adv_html += "document.getElementById('component_container').innerHTML = '';\n";
	splash_page_adv_html += "var _component_attr_array = [];\n";
	if(component_array['component_image']) {
		splash_page_adv_html += "if(splash_page_preview_array['component_image']) {\n";
		splash_page_adv_html += "var _component_obj_length = Object.keys(splash_page_preview_array['component_image']).length;\n";
		splash_page_adv_html += "for(var i = 0; i < _component_obj_length; i += 1) {\n";
		splash_page_adv_html += "var _idx = Object.keys(splash_page_preview_array['component_image'])[i];\n";
		splash_page_adv_html += "_component_attr_array = splash_page_preview_array['component_image'][_idx];\n";
		splash_page_adv_html += "document.getElementById('component_container').innerHTML += gen_component_obj(_component_attr_array, _ratio_width, _ratio_height, 'image');\n";
		splash_page_adv_html += "}\n";
		splash_page_adv_html += "}\n";
	}
	if(component_array['component_text']) {
		splash_page_adv_html += "if(splash_page_preview_array['component_text']) {\n";
		splash_page_adv_html += "var _component_obj_length = Object.keys(splash_page_preview_array['component_text']).length;\n";
		splash_page_adv_html += "for(var i = 0; i < _component_obj_length; i += 1) {\n";
		splash_page_adv_html += "var _idx = Object.keys(splash_page_preview_array['component_text'])[i];\n";
		splash_page_adv_html += "_component_attr_array = splash_page_preview_array['component_text'][_idx];\n";
		splash_page_adv_html += "document.getElementById('component_container').innerHTML += gen_component_obj(_component_attr_array, _ratio_width, _ratio_height, 'text');\n";
		splash_page_adv_html += "}\n";
		splash_page_adv_html += "}\n";
	}
	if(component_array['component_eula']) {
		splash_page_adv_html += "if(splash_page_preview_array['component_eula']) {\n";
		splash_page_adv_html += "_component_attr_array = splash_page_preview_array['component_eula'][0];\n";
		splash_page_adv_html += "document.getElementById('component_container').innerHTML += gen_component_obj(_component_attr_array, _ratio_width, _ratio_height, 'eula');\n";
		splash_page_adv_html += "document.getElementById('terms_service').style.color = _component_attr_array.eula_terms_service_font_color;\n";
		splash_page_adv_html += "document.getElementById('terms_service').style.backgroundColor = _component_attr_array.eula_terms_service_background_color;\n";
		splash_page_adv_html += "var code = '';\n";
		splash_page_adv_html += "code += '<div style=\"display:table;width:100%;height:100%;border-collapse:collapse;\">';\n";
		splash_page_adv_html += "code += '<div style=\"display:table-row;\">';\n";
		splash_page_adv_html += "code += '<div class=\"term_service_title\" style=\"display:table-cell;vertical-align:middle;font-size:' + (13 * _ratio_width) + 'px;\" >';\n";
		splash_page_adv_html += "var eula_terms_service_title =_component_attr_array.eula_terms_service_title;\n";
		splash_page_adv_html += "eula_terms_service_title = decodeURIComponent(eula_terms_service_title);\n";
		splash_page_adv_html += "eula_terms_service_title = eula_terms_service_title.replace(/\<br\>/g, '\\n');\n";
		splash_page_adv_html += "eula_terms_service_title = htmlEnDeCode.htmlEncode(eula_terms_service_title);\n";
		splash_page_adv_html += "eula_terms_service_title = eula_terms_service_title.replace(/ /g, '&nbsp;').replace(/(?:\\r\\n|\\r|\\n)/g, '<br>');\n";
		splash_page_adv_html += "code += eula_terms_service_title;\n";
		splash_page_adv_html += "code += '</div>';\n";
		splash_page_adv_html += "code += '</div>';\n";
		splash_page_adv_html += "code += '<div style=\"display:table-row;height:90%;\">';\n";
		splash_page_adv_html += "code += '<div style=\"display:table-cell;\">';\n";
		splash_page_adv_html += "code += '<div style=\"width:100%;max-width:100%;height:100%;max-height:100%;position:relative;\">';\n";
		splash_page_adv_html += "code += '<div class=\"term_service_text\" style=\"font-size:' + (12 * _ratio_width) + 'px;\">';\n";
		splash_page_adv_html += "var eula_terms_service =_component_attr_array.eula_terms_service;\n";
		splash_page_adv_html += "eula_terms_service = decodeURIComponent(eula_terms_service);\n";
		splash_page_adv_html += "eula_terms_service = eula_terms_service.replace(/\<br\>/g, '\\n');\n";
		splash_page_adv_html += "eula_terms_service = htmlEnDeCode.htmlEncode(eula_terms_service);\n";
		splash_page_adv_html += "eula_terms_service = eula_terms_service.replace(/ /g, '&nbsp;').replace(/(?:\\r\\n|\\r|\\n)/g, '<br>');\n";
		splash_page_adv_html += "code += eula_terms_service;\n";
		splash_page_adv_html += "code += '</div>';\n";
		splash_page_adv_html += "code += '</div>';\n";
		splash_page_adv_html += "code += '</div>';\n";
		splash_page_adv_html += "code += '</div>';\n";
		splash_page_adv_html += "code += '</div>';\n";
		splash_page_adv_html += "code += '<div class=\"terms_service_close\" onclick=\"close_term_service();\">';\n";
		splash_page_adv_html += "code += '<svg version=\"1.1\" id=\"terms_service_close_icon\" xmlns=\"http://www.w3.org/2000/svg\" xmlns:xlink=\"http://www.w3.org/1999/xlink\" x=\"0px\" y=\"0px\" viewBox=\"0 0 64 64\" enable-background=\"new 0 0 64 64\" xml:space=\"preserve\">';\n";
		splash_page_adv_html += "code += '<line fill=\"none\" stroke=\"' + _component_attr_array.eula_terms_service_font_color + '\" stroke-width=\"3\" stroke-linecap=\"round\" stroke-linejoin=\"round\" stroke-miterlimit=\"10\" x1=\"21.1\" y1=\"21.1\" x2=\"42.9\" y2=\"42.9\"/>';\n";
		splash_page_adv_html += "code += '<line fill=\"none\" stroke=\"' + _component_attr_array.eula_terms_service_font_color + '\" stroke-width=\"3\" stroke-linecap=\"round\" stroke-linejoin=\"round\" stroke-miterlimit=\"10\" x1=\"42.9\" y1=\"21.1\" x2=\"21.1\" y2=\"42.9\"/>';\n";
		splash_page_adv_html += "code += '<circle fill=\"none\" stroke=\"' + _component_attr_array.eula_terms_service_font_color + '\" stroke-width=\"2.5\" stroke-linecap=\"round\" stroke-linejoin=\"round\" stroke-miterlimit=\"10\" cx=\"32\" cy=\"32.1\" r=\"25.2\"/>';\n";
		splash_page_adv_html += "code += '</svg>';\n";
		splash_page_adv_html += "code += '</div>';\n";
		splash_page_adv_html += "$('#terms_service').html(code);\n";
		splash_page_adv_html += "}\n";
	}
	if(component_array['component_account']) {
		splash_page_adv_html += "if(splash_page_preview_array['component_account']) {\n";
		splash_page_adv_html += "_component_attr_array = splash_page_preview_array['component_account'][0];\n";
		splash_page_adv_html += "document.getElementById('component_container').innerHTML += gen_component_obj(_component_attr_array, _ratio_width, _ratio_height, 'account');\n";
		splash_page_adv_html += "}\n";
	}
	splash_page_adv_html += "if(splash_page_preview_array['component_button']) {\n";
	splash_page_adv_html += "_component_attr_array = splash_page_preview_array['component_button'][0];\n";
	splash_page_adv_html += "document.getElementById('component_container').innerHTML += gen_component_obj(_component_attr_array, _ratio_width, _ratio_height, 'button');\n";
	splash_page_adv_html += "}\n";
	splash_page_adv_html += "if(splash_page_preview_array['component_background']) {\n";
	splash_page_adv_html += "_component_attr_array = splash_page_preview_array['component_background'][0];\n";
	splash_page_adv_html += "document.getElementById('component_background').style.opacity = _component_attr_array.attribute.style_opacity;\n";
	splash_page_adv_html += "if(_component_attr_array.image_type == 'image') {\n";
	splash_page_adv_html += "document.getElementById('component_background').style.backgroundImage = 'url(' + _component_attr_array.image_base64 + ')';\n";
	splash_page_adv_html += "document.getElementById('component_background').style.backgroundRepeat = 'no-repeat';\n";
	splash_page_adv_html += "document.getElementById('component_background').style.backgroundAttachment = 'fixed';\n";
	splash_page_adv_html += "document.getElementById('component_background').style.backgroundSize = 'cover';\n";
	splash_page_adv_html += "document.getElementById('component_background').style.backgroundPosition = '50% 50%';\n";
	splash_page_adv_html += "}\n";
	splash_page_adv_html += "else {\n";
	splash_page_adv_html += "document.getElementById('component_background').style.backgroundColor =  _component_attr_array.attribute.style_background_color;\n";
	splash_page_adv_html += "}\n";
	splash_page_adv_html += "}\n";
	splash_page_adv_html += "if(splash_page_preview_array['component_eula']) {\n";
	splash_page_adv_html += "control_bt_status();\n";
	splash_page_adv_html += "}\n";
	splash_page_adv_html += "}\n";
	if(component_array['component_eula']) {
		splash_page_adv_html += "function control_bt_status() {\n";
		splash_page_adv_html += "var _obj = document.getElementById('eula_check');\n";
		splash_page_adv_html += "if(_obj.checked) {\n";
		splash_page_adv_html += "document.getElementById('component_button').style.opacity = 1;\n";
		splash_page_adv_html += "}\n";
		splash_page_adv_html += "else {\n";
		splash_page_adv_html += "document.getElementById('component_button').style.opacity = 0.5;\n";
		splash_page_adv_html += "}\n";
		splash_page_adv_html += "}\n";
	}
	splash_page_adv_html += "function isMobile() {\n";
	splash_page_adv_html += "var userAgentString = navigator.userAgent.toLowerCase();\n";
	splash_page_adv_html += "var mobile = ['iphone','ipad','ipod','android','blackberry','nokia','opera mini','windows mobile','windows phone','iemobile','mobile safari','bb10; touch', 'tablet'];\n";
	splash_page_adv_html += "for (var i in mobile) if (userAgentString.indexOf(mobile[i]) > 0) return true;\n";
	splash_page_adv_html += "return false;\n";
	splash_page_adv_html += "}\n";
	splash_page_adv_html += "function transformRGBtoHEX(_rgb) {\n";
	splash_page_adv_html += "var rgb_array = _rgb.replace(/\\s/g,'').replace('rgb', '').replace('(', '').replace(')', '').split(',');\n";
	splash_page_adv_html += "var hex = (rgb_array && rgb_array.length === 3) ? '#' +\n";
	splash_page_adv_html += "('0' + parseInt(rgb_array[0],10).toString(16)).slice(-2) +\n";
	splash_page_adv_html += "('0' + parseInt(rgb_array[1],10).toString(16)).slice(-2) +\n";
	splash_page_adv_html += "('0' + parseInt(rgb_array[2],10).toString(16)).slice(-2) : '#FFFFFF';\n";
	splash_page_adv_html += "return hex;\n";
	splash_page_adv_html += "}\n";
	splash_page_adv_html += "function transformHEXtoRGB(_hex) {\n";
	splash_page_adv_html += "var result = /^#?([a-f\\d]{2})([a-f\\d]{2})([a-f\\d]{2})$/i.exec(_hex);\n";
	splash_page_adv_html += "return result ? 'rgb(' + parseInt(result[1], 16) + ',' + parseInt(result[2], 16) + ',' + parseInt(result[3], 16) + ')' : 'rgb(255,255,255)';\n";
	splash_page_adv_html += "}\n";
	splash_page_adv_html += "<_TAG_END_>\n";
	splash_page_adv_html += "</head>\n";
	splash_page_adv_html += "<body id='component_body' onload='initial();' style='overflow-x:hidden;overflow-y:auto;;background-color:#21333E;font-family:Arial,Helvetica,sans-serif;'>\n";
	if(component_array['component_eula'])
		splash_page_adv_html += "<div id='terms_service' class='terms_service'></div>\n";
	splash_page_adv_html += "<div id='component_container' class='component_container'></div>\n";
	splash_page_adv_html += "<div id='component_background' class='component_background'></div>\n";
	splash_page_adv_html += "</body>\n";
	splash_page_adv_html += "</html>\n";

	var rc_service_wl_en = "";
	var rc_service_wl_dis = "";
	
	var captive_portal_adv_profile_item_temp = "";
	for(var idx = 0; idx < captive_portal_adv_current_edit_array.length; idx += 1) {
		if(idx == 0) {
			captive_portal_adv_profile_item_temp += "<" + captive_portal_adv_current_edit_array[idx];
		}
		else {
			if(idx == 6) { //walled garden
				captive_portal_adv_profile_item_temp += ">" + walled_garden_list;
			}
			else 
				captive_portal_adv_profile_item_temp += ">" + captive_portal_adv_current_edit_array[idx];
		}
	}
	var captive_portal_adv_profile_item = captive_portal_adv_profile_item_temp;

	if(flag == "new") {
		captive_portal_adv_profile_list += captive_portal_adv_profile_item;
		rc_service_wl_en = re_gen_wl_if(captive_portal_adv_profile_item.split(">")[5]);
		rc_service_wl_dis = "";
		captive_portal_adv_local_clientlist += "<" + profile_id + ">" + local_list_current;
	}
	else {
		var captive_portal_adv_profile_list_temp = "";
		var captive_portal_list_row = captive_portal_adv_profile_list.split("<");
		for(var i = 0; i < captive_portal_list_row.length; i += 1) {
			if(captive_portal_list_row[i] != "") {
				var captive_portal_list_col = captive_portal_list_row[i].split(">");
				if(flag == captive_portal_list_col[0]) {
					rc_service_wl_en = re_gen_wl_if(captive_portal_adv_profile_item.split(">")[5]);
					rc_service_wl_dis = re_gen_wl_if(captive_portal_list_col[5]);
					captive_portal_adv_profile_list_temp += captive_portal_adv_profile_item;
				}
				else {
					captive_portal_adv_profile_list_temp += "<" + captive_portal_list_row[i];
				}
			}
		}
		captive_portal_adv_profile_list = captive_portal_adv_profile_list_temp;

		var adv_local_clientlist_temp = "";
		var local_clientlist_row = captive_portal_adv_local_clientlist.split("<");
		for(var i = 0; i < local_clientlist_row.length; i += 1) {
			if(local_clientlist_row[i] != "") {
				var captive_portal_list_col = local_clientlist_row[i].split(">");
				if(flag == captive_portal_list_col[0]) {
					adv_local_clientlist_temp += "<" + profile_id + ">" + local_list_current;
				}
				else {
					adv_local_clientlist_temp += "<" + local_clientlist_row[i];
				}
			}
		}
		captive_portal_adv_local_clientlist = adv_local_clientlist_temp;
		if(captive_portal_adv_local_clientlist == "")
			captive_portal_adv_local_clientlist = "<" + profile_id + ">";
	}

	if(gn_overwrite_hint != "")
		alert(gn_overwrite_hint);

	captive_portal_adv_enable = "on";
	document.form.force_change.value++;
	document.form.action_script.value = "set_captive_portal_adv_wl;restart_wireless;restart_CP;restart_uam_srv";

	//if(based_modelid == "BRT-AC828") {
		var captive_portal_adv_enable_ori = '<% nvram_get("captive_portal_adv_enable"); %>';
		if(captive_portal_adv_enable_ori == "off") {
			cookie.set("captive_portal_gn_idx", get_captive_portal_wl_idx("captivePortal") + ">captivePortal", 1);
			document.form.next_page.value = "Guest_network.asp";
		}
	//}

	document.form.captive_portal_adv_wl_en.value = rc_service_wl_en;
	document.form.captive_portal_adv_wl_dis.value = rc_service_wl_dis;

	document.splash_page_form.splash_page_id.value = profile_id;
	document.splash_page_form.splash_page_attribute.value = splash_page_adv_attribute;
	document.splash_page_form.splash_page_html.value = splash_page_adv_html;
	document.splash_page_form.splash_page_css.value = splash_page_adv_css;
	showLoading();
	document.splash_page_form.submit();

	document.form.captive_portal_adv_profile.value = captive_portal_adv_profile_list;
	document.form.captive_portal_adv_enable.value = captive_portal_adv_enable;
	document.form.captive_portal_adv_idle_timeout.disabled = true;
	document.form.captive_portal_adv_local_clientlist.value = captive_portal_adv_local_clientlist;
}
function show_local_clientlist() {
	var local_client_list_row = local_list_current.split(',');
	var code = "";
	var local_user_name = "";
	var local_user_pwd = "";

	code += '<table width="100%" cellspacing="0" cellpadding="4" align="center" class="list_table" id="cpa_local_client_list_table">';
	if(local_client_list_row.length == 1)
		code += '<tr><td style="color:#FFCC00;" colspan="5"><#IPConnection_VSList_Norule#></td></tr>';
	else{
		for(var i = 1; i < local_client_list_row.length; i++){
			code += '<tr>';
			var local_client_list_col = local_client_list_row[i].split(':');
			for(var j = 0; j < local_client_list_col.length; j++) {
				if(j == 0) {
					local_user_name = local_client_list_col[0];
					if(local_user_name.length > 32) {
						local_user_name = local_user_name.substring(0, 30) + "...";	
					}
					code += '<td width="40%" title="' + local_client_list_col[0] + '">' + local_user_name +'</td>';
				}
				else if(j == 1) {
					local_user_pwd = local_client_list_col[1];
					if(local_user_pwd.length > 32) {
						local_user_pwd = local_user_pwd.substring(0, 30) + "...";
					}
					code += '<td width="40%" title="' + local_client_list_col[1] + '">' + local_user_pwd +'</td>';
				} 
			}
			
			code += '<td width="20%"><input class="remove_btn" onclick="delRow_local(this);" value=""/></td>';
			code += '</tr>';
		}
	}
	
	code += '</table>';
	$("#verification_local_clientlist_block")[0].innerHTML = code;
}
function addRow_local(_upper) {
	var username_obj = $("#cpa_local_username")[0];
	var	password_obj = $("#cpa_local_password")[0];

	var rule_num = $("#cpa_local_client_list_table")[0].rows.length;
	var item_num = $("#cpa_local_client_list_table")[0].rows[0].cells.length;

	if(rule_num >= _upper) {
		alert("<#JS_itemlimit1#> " + _upper + " <#JS_itemlimit2#>");
		return false;
	}

	var validForm = function() {
		if(username_obj.value == "") {
			alert("<#JS_fieldblank#>");
			username_obj.focus();
			return false;
		}
		else if(!Block_chars(username_obj, [" ", "@", "*", "+", "|", ":", "?", "<", ">", ",", ".", "/", ";", "[", "]", "\\", "=", "\"", "&" ])) {
			return false;
		}

		if(password_obj.value == "") {
			alert("<#JS_fieldblank#>");
			password_obj.focus();
			return false;
		}
		else if(!Block_chars(password_obj, [" ", "@", "*", "+", "|", ":", "?", "<", ">", ",", ".", "/", ";", "[", "]", "\\", "=", "\"", "&" ])) {
			return false;
		}
		else if(password_obj.value.length > 0 && password_obj.value.length < 5) {
			alert("* <#JS_short_password#>");
			password_obj.focus();
			return false;
		}

		//confirm common string combination	#JS_common_passwd#
		var is_common_string = check_common_string(password_obj.value, "httpd_password");
		if(password_obj.value.length > 0 && is_common_string){
			if(!confirm("<#JS_common_passwd#>")){
				password_obj.focus();
				password_obj.select();
				return false;
			}
		}
			
		return true;
	};

	if(validForm()) {
		if(item_num >= 2) {
			for(var i = 0; i < rule_num; i +=1 ) {
				if(username_obj.value == $("#cpa_local_client_list_table")[0].rows[i].cells[0].title) {
					alert("<#JS_duplicate#>");
					username_obj.focus();
					username_obj.select();
					return false;
				}	
			}
		}
		
		var addRow = function(obj, head) {
			if(head == 1)
				local_list_current += ",";
			else	
				local_list_current += ":";
					
			local_list_current += obj.value;
			obj.value = "";
		}

		addRow(username_obj ,1);
		addRow(password_obj, 0);
		show_local_clientlist();
	}
}
function delRow_local(rowdata) {
	var i = rowdata.parentNode.parentNode.rowIndex;
	$("#cpa_local_client_list_table")[0].deleteRow(i);
	var temp_list = "";
	var rowLength = $("#cpa_local_client_list_table")[0].rows.length;
	
	for(var k = 0; k < rowLength; k += 1) {
		for(var j = 0; j < $("#cpa_local_client_list_table")[0].rows[k].cells.length - 1; j += 1) {
			if(j == 0)
				temp_list += ",";
			else {
				temp_list += $("#cpa_local_client_list_table")[0].rows[k].cells[0].title;
				temp_list += ":";
				temp_list += $("#cpa_local_client_list_table")[0].rows[k].cells[1].title;
			}
		}
	}

	local_list_current = temp_list;

	if(local_list_current == "")
		show_local_clientlist();
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
function update_component_attribute(obj) {
	var _obj_array = obj.split("_");
	var _component_type = _obj_array[0] + "_" + _obj_array[1];
	var _component_id = "";
	if(_component_type == "component_text" || _component_type == "component_image") {
		_component_id = _obj_array[2];
	}

	if(_component_id != "") { //text, image
		if(component_array[_component_type]) {
			if(component_array[_component_type][_component_id]) {
				component_array[_component_type][_component_id].attribute.style_left = $("#" + obj + "").css("left");
				component_array[_component_type][_component_id].attribute.style_top = $("#" + obj + "").css("top");
				component_array[_component_type][_component_id].attribute.style_height = $("#" + obj + "").css("height");
				component_array[_component_type][_component_id].attribute.style_width = $("#" + obj + "").css("width");
				component_array[_component_type][_component_id].attribute.style_z_index = $("#" + obj + "").css("z-index");
			}
		}
	}
	else {
		if(component_array[_component_type]) {
			component_array[_component_type][0].attribute.style_left = $("#" + obj + "").css("left");
			component_array[_component_type][0].attribute.style_top = $("#" + obj + "").css("top");
			component_array[_component_type][0].attribute.style_height = $("#" + obj + "").css("height");
			component_array[_component_type][0].attribute.style_width = $("#" + obj + "").css("width");
			component_array[_component_type][0].attribute.style_z_index = $("#" + obj + "").css("z-index");
		}
	}
}
function set_drop_resize_event(obj) {
	var set_event = function(_component_id) {
		$( "#"+ _component_id + "" ).resizable({
			containment: $("#splash_page_layout_bg"),
			stop: function(event, ui) {
				update_component_attribute(_component_id);
			}
		});

		$("#"+ _component_id + "").draggable({
			containment: $("#splash_page_layout_bg"),
			stop: function(event,ui) {
				update_component_attribute(_component_id);
			}
		});
	};

	if(obj != "component_background")
		set_event(obj);

	if(obj == "component_button" || obj == "component_account") {
		$("#"+ obj + "").children('.ui-resizable-s').remove();
		$("#"+ obj + "").children('.ui-resizable-se').remove();
	}

	if(obj == "component_eula") {
		$("#"+ obj + "").children('.ui-resizable-s').remove();
		$("#"+ obj + "").children('.ui-resizable-se').remove();
		//$("#"+ obj + "").children('.ui-resizable-e').remove();
	}
}
function transformRGBtoHEX(_rgb) {
	var rgb_array = _rgb.replace(/\s/g,'').replace("rgb", "").replace("(", "").replace(")", "").split(",");
	var hex = (rgb_array && rgb_array.length === 3) ? "#" +
	("0" + parseInt(rgb_array[0],10).toString(16)).slice(-2) +
	("0" + parseInt(rgb_array[1],10).toString(16)).slice(-2) +
	("0" + parseInt(rgb_array[2],10).toString(16)).slice(-2) : "#FFFFFF";
	return hex;
}
function transformHEXtoRGB(_hex) {
	var result = /^#?([a-f\d]{2})([a-f\d]{2})([a-f\d]{2})$/i.exec(_hex);
	return result ? "rgb(" + parseInt(result[1], 16) + ", " + parseInt(result[2], 16) + ", " + parseInt(result[3], 16) + ")" : "rgb(255, 255, 255)";
}
function cpa_apply() {
	var rc_service_wl_en = "";
	var rc_service_wl_dis = "";
	var _wl_list = "";
	var _captive_portal_adv_profile_row = captive_portal_adv_profile_list.split("<");
	var _captive_portal_adv_profile_col = [];
	for(var i = 0; i < _captive_portal_adv_profile_row.length; i += 1) {
		if(_captive_portal_adv_profile_row[i] != "") {
			_wl_list = _captive_portal_adv_profile_row[i].split(">")[5];
			_captive_portal_adv_profile_col = _captive_portal_adv_profile_row[i].split(">");
		}
	}
	if(document.form.captive_portal_adv_enable.value == "on") {
		if(captive_portal_adv_enable == "off") {
			rc_service_wl_en = "";
			rc_service_wl_dis = re_gen_wl_if(_wl_list);
		}
	}
	else {
		if(captive_portal_adv_enable == "on") {
			rc_service_wl_en = re_gen_wl_if(_wl_list);
			rc_service_wl_dis = "";
			var gn_overwrite_hint = "";

			var _wl_band_array = [];
			var _wl_if_array = _wl_list.split("wl");
			for(var i = 0; i < _wl_if_array.length; i += 1) {
				if(_wl_if_array[i] != "") {
					var _wl_unit = _wl_if_array[i].split(".")[0];
					var _wl_subunit = _wl_if_array[i].split(".")[1];
					switch(_wl_unit) {
						case "0" :
							if(!check_gn_if_status(_wl_subunit, gn_array_2g))
								gn_overwrite_hint += "<#Guest_Network#> " + _wl_subunit + " - " + wl_nband_title[0] + " will be overwrite.\n\n";
							break;
						case "1" :
							if(!check_gn_if_status(_wl_subunit, gn_array_5g))
								gn_overwrite_hint += "<#Guest_Network#> " + _wl_subunit + " - " + wl_nband_title[1] + " will be overwrite.\n\n";
							break;
						case "2" :
							if(!check_gn_if_status(_wl_subunit, gn_array_5g_2))
								gn_overwrite_hint += "<#Guest_Network#> " + _wl_subunit + " - " + wl_nband_title[2] + " will be overwrite.";
							break;
					}
				}
			}

			if(gn_overwrite_hint != "")
				alert(gn_overwrite_hint);				
		}
	}

	document.form.action_script.value = "set_captive_portal_adv_wl;restart_wireless;restart_CP;restart_uam_srv";
	document.form.captive_portal_adv_enable.value = captive_portal_adv_enable;
	if(!validator.numberRange($('#captive_portal_adv_idle_timeout')[0], 60, 800)){
		$("#captive_portal_adv_idle_timeout").focus();
		return false;
	}
	document.form.captive_portal_adv_idle_timeout.value = $("#captive_portal_adv_idle_timeout").val();
	document.form.captive_portal_adv_profile.value = captive_portal_adv_profile_list;
	document.form.captive_portal_adv_local_clientlist.value = captive_portal_adv_local_clientlist;
	document.form.captive_portal_adv_wl_en.value = rc_service_wl_en;
	document.form.captive_portal_adv_wl_dis.value = rc_service_wl_dis;
	showLoading();
	document.form.submit();
}
function close_eventLog() {
	$("#captive_portal_adv_log_panel").fadeOut(300);
}
function drawSwitchEventMode(mode) {
	$("#switchEventMode").empty();
	var drawSwitchModeHtml = "";
	if(mode) {
		drawSwitchModeHtml += "<div class='block_filter_pressed auth_pass'>";
		drawSwitchModeHtml += "<div class='block_filter_name' style='color:#93A9B1;'>Authenticated Client</div>";/*untranslated*/
		drawSwitchModeHtml += "</div>";
		drawSwitchModeHtml += "<div class='block_filter auth_fail' style='cursor:pointer' onclick='changeEventLogMode();'>";
		drawSwitchModeHtml += "<div class='block_filter_name'>failed Authenticated Client</div>";/*untranslated*/
		drawSwitchModeHtml += "</div>";
	}
	else {							
		drawSwitchModeHtml += "<div class='block_filter auth_pass' style='cursor:pointer' onclick='changeEventLogMode();'>";
		drawSwitchModeHtml += "<div class='block_filter_name'>Authenticated Client</div>";/*untranslated*/
		drawSwitchModeHtml += "</div>";
		drawSwitchModeHtml += "<div class='block_filter_pressed auth_fail'>";
		drawSwitchModeHtml += "<div class='block_filter_name' style='color:#93A9B1;'>failed Authenticated Client</div>";/*untranslated*/
		drawSwitchModeHtml += "</div>";
	}
	$("#switchEventMode").append(drawSwitchModeHtml);
}
function changeEventLogMode() {
	$("#switchEventMode").empty();
	$("#auth_pass_block").css("display", "none");
	$("#auth_failed_block").css("display", "none");
	if(event_log_mode) {
		event_log_mode = false;
		drawSwitchEventMode(event_log_mode);
		$("#auth_failed_block").css("display", "");
	}
	else {
		event_log_mode = true;
		drawSwitchEventMode(event_log_mode);
		$("#auth_pass_block").css("display", "");
	}
}
function formatBytes(bytes,decimals) {
	if(bytes == 0) return '0 Byte';
	var k = 1000;
	var dm = decimals + 1 || 3;
	var sizes = ['Bytes', 'KB', 'MB', 'GB', 'TB', 'PB', 'EB', 'ZB', 'YB'];
	var i = Math.floor(Math.log(bytes) / Math.log(k));
	return parseFloat((bytes / Math.pow(k, i)).toFixed(dm)) + ' ' + sizes[i];
}
function seconds2time (seconds) {
	var hours   = Math.floor(seconds / 3600);
	var minutes = Math.floor((seconds - (hours * 3600)) / 60);
	var seconds = seconds - (hours * 3600) - (minutes * 60);
	var time = "";

	if (hours != 0) {
		time = hours+":";
	}
	if (minutes != 0 || time !== "") {
		minutes = (minutes < 10 && time !== "") ? "0"+minutes : String(minutes);
		time += minutes+":";
	}
	if (time === "") {
		time = seconds+"s";
	}
	else {
		time += (seconds < 10) ? "0"+seconds : String(seconds);
	}
	return time;
}
function transform_sec_to_date(_seconds) {
	var _time_date = "";
	var _time_format_obj = new Date(1000 * _seconds);
	_time_date = _time_format_obj.getFullYear() + "/" + ("0" + (_time_format_obj.getMonth() + 1)).slice(-2) + "/" + ("0" + _time_format_obj.getDate()).slice(-2) + "  " +
	checkTime(_time_format_obj.getHours()) + ":" + checkTime(_time_format_obj.getMinutes()) + ":" + checkTime(_time_format_obj.getSeconds());
	return _time_date;
}
function gen_event_log(_array, _connect_type) {
	var _colspan_num = {"pass" : 9, "failed" : 5};
	var _td_width = {"pass" : ["10%", "10%", "10%", "10%", "10%", "10%", "20%", "10%", "10%"], "failed" : ["20%", "20%", "20%", "20%", "20%"]};
	var code = "";
	code += '<table width="100%" cellspacing="0" cellpadding="4" align="center" class="FormTable_table" id="connect_client_auth_' + _connect_type + '_table">';
	if(_array.length == 0)
		code += '<tr><td style="color:#FFCC00;" colspan=' + _colspan_num[_connect_type] + '><#IPConnection_VSList_Norule#></td></tr>';
	else{
		for(var i = 0; i < _array.length; i += 1) {
			code += '<tr style="height:30px;">';
			switch(_connect_type) {
				case "pass" :
					code += '<td width=' + _td_width[_connect_type][0] + '>' + _array[i].macAddress + '</td>';
					code += '<td width=' + _td_width[_connect_type][1] + '>' + _array[i].ipAddress + '</td>';
					code += '<td width=' + _td_width[_connect_type][2] + '>' + _array[i].userName + '</td>';
					code += '<td width=' + _td_width[_connect_type][3] + '>' + _array[i].auth + '</td>';
					code += '<td width=' + _td_width[_connect_type][4] + '>' + seconds2time(_array[i].session) + '</td>';
					code += '<td width=' + _td_width[_connect_type][5] + '>' + seconds2time(_array[i].awayTimeout) + '</td>';
					code += '<td width=' + _td_width[_connect_type][6] + '>' + transform_sec_to_date(_array[i].startTime) + '</td>';
					code += '<td width=' + _td_width[_connect_type][7] + '>' + formatBytes(_array[i].receviedBytes) + '</td>';
					code += '<td width=' + _td_width[_connect_type][8] + '>' + formatBytes(_array[i].transmittedBytes) + '</td>';
					break;
				case "failed" :
					code += '<td width=' + _td_width[_connect_type][0] + '>' + _array[i].macAddress + '</td>';
					code += '<td width=' + _td_width[_connect_type][1] + '>' + _array[i].ipAddress + '</td>';
					code += '<td width=' + _td_width[_connect_type][2] + '>' + _array[i].userName + '</td>';
					code += '<td width=' + _td_width[_connect_type][3] + '>' + _array[i].auth + '</td>';
					code += '<td width=' + _td_width[_connect_type][4] + '>' + transform_sec_to_date(_array[i].startTime) + '</td>';
					break;
			}
			code += '</tr>';
		}
	}
	code += '</table>';
	$("#connect_client_auth_" + _connect_type + "_block")[0].innerHTML = code;
}
var connect_status_sorter = {
	"auth_pass_click_item" : "startTime",
	"sortingMethod_auth_pass" : "increase",
	"auth_failed_click_item" : "startTime",
	"sortingMethod_auth_failed" : "increase", 

	"sorter_type" : function(_sort_item) {
		var _type = "str";
		switch(_sort_item) {
			case "macAddress" :
			case "userName" :
			case "auth" :
				_type = "str";
				break;
			case "ipAddress" :
				_type = "ip";
				break;
			case "session" :
			case "awayTimeout" :
			case "startTime" :
			case "receviedBytes" :
			case "transmittedBytes" :
				_type = "num";
				break;
		}
		return _type
	},	
	"addBorder" : function(_connect_type, _obj) {
		var _sort_item = _obj.id.split("_")[1];
		switch(_connect_type) {
			case "pass" :
				connect_status_sorter.sortingMethod_auth_pass = (connect_status_sorter.sortingMethod_auth_pass == "increase") ? "decrease" : "increase";
				connect_status_sorter.auth_pass_click_item = _sort_item;
				break;
			case "failed" :
				connect_status_sorter.sortingMethod_auth_failed = (connect_status_sorter.sortingMethod_auth_failed == "increase") ? "decrease" : "increase";
				connect_status_sorter.auth_failed_click_item = _sort_item;
				break;
		}
	},
	"drawDefaultBorder" : function(_connect_type) {
		$("#tr_auth_" + _connect_type + "").find("th").css("border-top", "1px solid #222");
		$("#tr_auth_" + _connect_type + "").find("th").css("border-bottom", "1px solid #222");
	},
	"doSorter" : function(_connect_type) {
		var _array = [];
		var _sort_item = "";
		var _sort_type = "";
		var _sort_method = "";
		connect_status_sorter.drawDefaultBorder(_connect_type);
		switch(_connect_type) {
			case "pass" :
				_sort_item = connect_status_sorter.auth_pass_click_item;
				_sort_type = connect_status_sorter.sorter_type(_sort_item);
				_sort_method = connect_status_sorter.sortingMethod_auth_pass;
				_array = sort_connect_status_auth_pass;
				break;
			case "failed" :
				_sort_item = connect_status_sorter.auth_failed_click_item;
				_sort_type = connect_status_sorter.sorter_type(_sort_item);
				_sort_method = connect_status_sorter.sortingMethod_auth_failed;
				_array = sort_connect_status_auth_failed;
				break;
		}

		if(_sort_method == "increase") {
			$("#tr_auth_" + _connect_type + "").find("#th_" + _sort_item + "_" + _connect_type + "").css("border-top", "1px solid #FC0");
		}
		else {
			$("#tr_auth_" + _connect_type + "").find("#th_" + _sort_item + "_" + _connect_type + "").css("border-bottom", "1px solid #FC0");
		}
		
		switch(_sort_type + "_" + _sort_method) {
			case "str_increase" :
				_array = _array.sort(function(a, b) {
					return (a[_sort_item] > b[_sort_item]) ? 1 : ((a[_sort_item] < b[_sort_item]) ? -1 : 0);
				});
				break;
			case "str_decrease" :
				_array = _array.sort(function(a, b) {
					return (b[_sort_item] > a[_sort_item]) ? 1 : ((b[_sort_item] < a[_sort_item]) ? -1 : 0);
				});
				break;
			case "ip_increase" :
				_array = _array.sort(function(a, b) {
					var a_num = 0, b_num = 0;
					a_num = inet_network(a[_sort_item]);
					b_num = inet_network(b[_sort_item]);
					return parseInt(a_num) - parseInt(b_num);
				});
				break;
			case "ip_decrease" :
				_array = _array.sort(function(a, b) {
					var a_num = 0, b_num = 0;
					a_num = inet_network(a[_sort_item]);
					b_num = inet_network(b[_sort_item]);
					return parseInt(b_num) - parseInt(a_num);
				});
				break;
			case "num_increase" :
				_array = _array.sort(function(a, b) {
					return parseInt(a[_sort_item]) - parseInt(b[_sort_item]);
				});
				break;
			case "num_decrease" :
				_array = _array.sort(function(a, b) {
					return parseInt(b[_sort_item]) - parseInt(a[_sort_item]);
				});
				break;
		}
		gen_event_log(_array, _connect_type);
	}
};
var sort_connect_status_auth_pass = [];
var sort_connect_status_auth_failed = [];
function get_connection_event_log() {
	var result = "NoData";
	$.ajax({
		url: '/ajax_captive_portal_log.asp',
		dataType: 'json',
		error: function(xhr){
			setTimeout("get_connection_event_log();", 1000);
		},
		success: function(response) {
			sort_connect_status_auth_pass = [];
			sort_connect_status_auth_failed = [];
			var connect_status_attr = function() {
				this.macAddress = "";
				this.ipAddress = "";
				this.userName = "";
				this.auth = "";
				this.session = "";
				this.awayTimeout = "";
				this.startTime = "";
				this.receviedBytes = "";
				this.transmittedBytes = "";
			};

			for(var i = 0; i < response.pass.length; i += 1) {
				if(response.pass[i] != "") {
					var connect_status_array = [];
					connect_status_array = new connect_status_attr();
					connect_status_array.macAddress = response.pass[i].macAddress;
					connect_status_array.ipAddress = response.pass[i].ipAddress;
					connect_status_array.userName = response.pass[i].session.userName;
					connect_status_array.auth = "Local";
					connect_status_array.session = response.pass[i].accounting.sessionTime;
					connect_status_array.awayTimeout = response.pass[i].accounting.idleTime;
					connect_status_array.startTime = response.pass[i].session.startTime;
					connect_status_array.receviedBytes = response.pass[i].accounting.inputOctets;
					connect_status_array.transmittedBytes = response.pass[i].accounting.outputOctets;
					sort_connect_status_auth_pass.push(connect_status_array);
				}
			}

			for(var i = 0; i < response.failed.length; i += 1) {
				if(response.failed[i] != "") {
					var connect_status_array = [];
					connect_status_array = new connect_status_attr();
					connect_status_array.macAddress = response.failed[i].macAddress;
					connect_status_array.ipAddress = response.failed[i].ipAddress;
					connect_status_array.userName = response.failed[i].session.userName;
					connect_status_array.auth = i;
					connect_status_array.startTime = response.failed[i].session.startTime;
					sort_connect_status_auth_failed.push(connect_status_array);
				}
			}
			connect_status_sorter.doSorter("pass");
			connect_status_sorter.doSorter("failed");
		}
	});
}
function update_connect_status() {
	get_connection_event_log();
}
function exportConnectEventLog() {
	var data = [];
	if(event_log_mode) {
		data = [["<#MAC_Address#>", "<#IPConnection_ExternalIPAddress_itemname#>", "<#Username#>", "<#PPPConnection_Authentication_itemname#>", "<#FreeWiFi_timeout#>", "<#FreeWiFi_Idle#>", "<#Captive_Portal_Start_Time#>", "<#Captive_Portal_Recevied_Bytes#>", "<#Captive_Portal_Transmitted_Bytes#>"]];
	}
	else {
		data = [["<#MAC_Address#>", "<#IPConnection_ExternalIPAddress_itemname#>", "<#Username#>", "<#PPPConnection_Authentication_itemname#>", "Failure Time"]];
	}
	var tempArray = new Array();
	var setArray = function(array) {
		for(var i = 0; i < array.length; i += 1) {
			tempArray = [];
			if(event_log_mode) {
				tempArray[0] = array[i].macAddress;
				tempArray[1] = array[i].ipAddress;
				tempArray[2] = array[i].userName;
				tempArray[3] = array[i].auth;
				tempArray[4] = seconds2time(array[i].session);
				tempArray[5] = seconds2time(array[i].awayTimeout);
				tempArray[6] = transform_sec_to_date(array[i].startTime);
				tempArray[7] = formatBytes(array[i].receviedBytes);
				tempArray[8] = formatBytes(array[i].transmittedBytes);
			}
			else {
				tempArray[0] = array[i].macAddress;
				tempArray[1] = array[i].ipAddress;
				tempArray[2] = array[i].userName;
				tempArray[3] = array[i].auth;
				tempArray[4] = transform_sec_to_date(array[i].startTime);
			}
			data.push(tempArray);
		}
	};
	if(event_log_mode) {
		setArray(sort_connect_status_auth_pass);
	}
	else {
		setArray(sort_connect_status_auth_failed);
	}

	var csvContent = '';
	data.forEach(function (infoArray, index) {
		dataString = infoArray.join(',');
		csvContent += index < data.length ? dataString + '\n' : dataString;
	});

	var download = function(content, fileName, mimeType) {
		var a = document.createElement('a');
		mimeType = mimeType || 'application/octet-stream';

		if (navigator.msSaveBlob) { // IE10
			return navigator.msSaveBlob(new Blob([content], { type: mimeType }), fileName);
		} 
		else if ('download' in a) { //html5 A[download]
			a.href = 'data:' + mimeType + ',' + encodeURIComponent(content);
			a.setAttribute('download', fileName);
			document.getElementById("captive_portal_adv_log_panel").appendChild(a);
			setTimeout(function() {
				a.click();
				document.getElementById("captive_portal_adv_log_panel").removeChild(a);
			}, 66);
			return true;
		} 
		else { //do iframe dataURL download (old ch+FF):
			var f = document.createElement('iframe');
			document.getElementById("captive_portal_adv_log_panel").appendChild(f);
			f.src = 'data:' + mimeType + ',' + encodeURIComponent(content);

			setTimeout(function() {
				document.getElementById("captive_portal_adv_log_panel").removeChild(f);
				}, 333);
			return true;
		}
	};

	var _export_file_name = (event_log_mode) ? "ConnectEventLog_Pass.csv" : "ConnectEventLog_Failed.csv";
	download(csvContent, _export_file_name, 'data:text/csv;charset=utf-8');
}
var event_log_mode = true;
function show_event_log() {
	var code = "";
	code += "<div id='switchEventMode' style='display:none;'></div>";
	code += "<div class='captive_portal_adv_log_close'><img src='/images/button-close.gif' style='width:35px;' onclick='close_eventLog();'></div>";
	//code += "<div class='captive_portal_adv_line'></div>";

	code += "<div id='auth_pass_block' class='auth_block' style='display:none;'>";
	code += "<table width='100%' align='center' cellpadding='0' cellspacing='0' class='FormTable_table'>";
	code += "<thead><tr style='height:30px;'><td colspan='10'><#Captive_Portal_Connected_Clients#>";
	code += "</td></tr></thead>";
	code += "<tr id='tr_auth_pass' style='cursor:pointer;height:40px;'>";
	code += "<th id='th_macAddress_pass' width='10%' onclick='connect_status_sorter.addBorder(\"pass\", this);connect_status_sorter.doSorter(\"pass\");' style='cursor:pointer;'><#PPPConnection_x_MacAddressForISP_itemname#></th>";
	code += "<th id='th_ipAddress_pass' width='10%' onclick='connect_status_sorter.addBorder(\"pass\", this);connect_status_sorter.doSorter(\"pass\");'><#IPConnection_ExternalIPAddress_itemname#></th>";
	code += "<th id='th_userName_pass' width='10%' onclick='connect_status_sorter.addBorder(\"pass\", this);connect_status_sorter.doSorter(\"pass\");'><#Username#></th>";
	code += "<th id='th_auth_pass' width='10%' onclick='connect_status_sorter.addBorder(\"pass\", this);connect_status_sorter.doSorter(\"pass\");'><#PPPConnection_Authentication_itemname#></th>";
	code += "<th id='th_session_pass' width='10%' onclick='connect_status_sorter.addBorder(\"pass\", this);connect_status_sorter.doSorter(\"pass\");'><#FreeWiFi_timeout#></th>";
	code += "<th id='th_awayTimeout_pass' width='10%' onclick='connect_status_sorter.addBorder(\"pass\", this);connect_status_sorter.doSorter(\"pass\");'><#FreeWiFi_Idle#></th>";
	code += "<th id='th_startTime_pass' width='20%' onclick='connect_status_sorter.addBorder(\"pass\", this);connect_status_sorter.doSorter(\"pass\");'><#Captive_Portal_Start_Time#></th>";
	code += "<th id='th_receviedBytes_pass' width='10%' onclick='connect_status_sorter.addBorder(\"pass\", this);connect_status_sorter.doSorter(\"pass\");'><#Captive_Portal_Recevied_Bytes#></th>";
	code += "<th id='th_transmittedBytes_pass' width='10%' onclick='connect_status_sorter.addBorder(\"pass\", this);connect_status_sorter.doSorter(\"pass\");'><#Captive_Portal_Transmitted_Bytes#></th>";
	code += "</tr>";
	code += "</table>";
	code += "<div id='connect_client_auth_pass_block'></div>";
	code += "</div>";

	code += "<div id='auth_failed_block' class='auth_block' style='display:none;'>";

	code += "<table width='100%' border='1' align='center' cellpadding='0' cellspacing='0' class='FormTable_table' style='margin-top:0px;'>";
	code += "<thead><tr style='height:30px;'><td colspan='5'><#Captive_Portal_Connected_Clients#>";
	code += "</td></tr></thead>";
	code += "<tr id='tr_auth_failed' style='cursor:pointer;height:40px;'>";
	code += "<th id='th_macAddress_failed' width='20%' onclick='connect_status_sorter.addBorder(\"failed\", this);connect_status_sorter.doSorter(\"failed\");'><#PPPConnection_x_MacAddressForISP_itemname#></th>";
	code += "<th id='th_ipAddress_failed' width='20%' onclick='connect_status_sorter.addBorder(\"failed\", this);connect_status_sorter.doSorter(\"failed\");'><#IPConnection_ExternalIPAddress_itemname#></th>";
	code += "<th id='th_userName_failed' width='20%' onclick='connect_status_sorter.addBorder(\"failed\", this);connect_status_sorter.doSorter(\"failed\");'><#Username#></th>";
	code += "<th id='th_auth_failed' width='20%' onclick='connect_status_sorter.addBorder(\"failed\", this);connect_status_sorter.doSorter(\"failed\");'><#PPPConnection_Authentication_itemname#></th>";
	code += "<th id='th_startTime_failed' width='20%' onclick='connect_status_sorter.addBorder(\"failed\", this);connect_status_sorter.doSorter(\"failed\");'>Failure Time</th>";/*untranslated*/
	code += "</tr>";
	code += "</table>";
	code += "<div id='connect_client_auth_failed_block'></div>";

	code += "</div>";

	code += "<div class='captive_portal_adv_log_action_bg'>";
	code += "<div class='captive_portal_adv_line'></div>";
	code += "<input class='button_gen' onclick='exportConnectEventLog();' type='button' value='<#btn_Export#>'/>";
	code += "<input class='button_gen' onclick='update_connect_status();' type='button' value='<#CTL_refresh#>'/>";
	code += "</div>";

	$("#captive_portal_adv_log_panel").html(code);

	drawSwitchEventMode(event_log_mode);
	if(event_log_mode)
		$("#auth_pass_block").css("display", "");
	else
		$("#auth_failed_block").css("display", "");

	$("#captive_portal_adv_log_panel").fadeIn(300);
	cal_panel_block("captive_portal_adv_log_panel", 0.05);

	get_connection_event_log();
}
function checkImageMimeType(_mimeType) {
	var  mimeTypeList = /image\/(jpg|jpeg|gif|png|bmp|x-icon)$/i;
	if (mimeTypeList.test(_mimeType)) 
		return true;
	else
		return false;
}
function register_slider(_component_type) {
	var call_back_fun = function(_component_type, _component_id) {
		switch(_component_type) {
			case "image" :
				edit_update_content_image(_component_id);
				break;
			case "background" :
				edit_update_content_background();
				break;
			case "account" :
				edit_update_content_account();
				break;
		}
	};
	var _setted_opacity = 100;
	var _type = _component_type;
	var _idx = "";
	if (_component_type.indexOf("image") >= 0) {
		_type = _component_type.split("_")[0];
		_idx =  _component_type.split("_")[1];
	}
	if(component_array["component_" + _type]) {
		switch(_type) {
			case "image" :
				if(component_array["component_" + _type][_idx])
					_setted_opacity = Math.round(parseFloat(component_array["component_" + _type][_idx].attribute.style_opacity) * 100);
				break;
			case "background" :
				_setted_opacity = Math.round(parseFloat(component_array["component_" + _type][0].attribute.style_opacity) * 100);
				break;
			case "account" :
				_setted_opacity = Math.round(parseFloat(component_array["component_" + _type][0].account_input_box_opacity) * 100);
				break;
		}
	}

	$( "#edit_" + _type + "_opacity" ).attr('title', _setted_opacity);
	$( "#edit_" + _type + "_opacity_text" ).html(_setted_opacity + "%");

	$( "#edit_" + _type + "_opacity" ).slider({
		orientation: "horizontal",
		range: "min",
		min: 1,
		max: 100,
		value: _setted_opacity,
		step: 1,
		slide: function(event, ui) {
			$( "#edit_" + _type + "_opacity" ).attr('title', ui.value);
			$( "#edit_" + _type + "_opacity_text" ).html(ui.value + "%");
			call_back_fun(_type, _idx);
		},
		stop: function(event, ui) {
			$( "#edit_" + _type + "_opacity" ).attr('title', ui.value);
			$( "#edit_" + _type + "_opacity_text" ).html(ui.value + "%");
			call_back_fun(_type, _idx);
		}
	});
}
function gen_slider(_component_type) {
	var code = "";
	code += "<div class='edit_component_item_title'><#Captive_Portal_Opacity#></div>";
	code += "<div id='edit_" + _component_type + "_opacity' class='slider' title='100'></div>";
	code += "<div id='edit_" + _component_type + "_opacity_text' class='slider_text'>100%</div>";
	code += "<div style='clear:both;'></div>";
	return code;
}
function edit_sort_level(_component_id, _method) {
	var z_index_temp = $("#"+ _component_id + "").css("z-index");
	switch(_method) {
		case "increase" :
			z_index_temp++;
			break;
		case "decrease" :
			z_index_temp--;
			if(z_index_temp == 0)
				z_index_temp = 1;
			break;
	}

	$("#"+ _component_id + "").css("z-index", z_index_temp);
	$("#edit_component_level").html(z_index_temp);
	
	update_component_attribute(_component_id);
}
function gen_sort_level(_component_id) {
	var code = "";
	code += "<div class='edit_component_item_title' style='margin-bottom:8px;'><#Captive_Portal_Layer#>: <span id='edit_component_level' class='edit_component_level'>" + $("#"+ _component_id + "").css("z-index") + "</span></div>";/*untranslated*/
	code += "<div class='edit_level edit_level_up' title='increase' onclick='edit_sort_level(\"" + _component_id + "\", \"increase\");'></div>";
	code += "<div class='edit_level edit_level_down' title='decrease' onclick='edit_sort_level(\"" + _component_id + "\", \"decrease\");'></div>";
	code += "<div style='clear:both;'></div>";
	return code;
}
function reset_editing_area_css() {
	if(captive_portal_adv_edit_idx == "2") {
		reset_component_css();
		reset_toolbar_css();
		switch(toolbar_click) {
			case "toolbar_background" :
				edit_component_background();
				break;
			case "toolbar_editor" :
				component_click = "";
				var code = "<div class='edit_component_item_hint'><#Captive_Portal_Select_Element#></div>";
				$("#splash_page_container_edit").html(code);
				break;
		}
	}
}
function stop_propagation(event) {
	event.stopPropagation();
}
function encode_decode_text(_string, _type) {
	var _string_temp = _string;
	switch(_type) {
		case "encode" :
			//escaped character
			 _string_temp = _string_temp.replace(/"/g, '\"').replace(/\\/g, "\\");
			//replace new line
			_string_temp = _string_temp.replace(/(?:\r\n|\r|\n)/g, '<br>');
			//encode ASCII
			_string_temp = encodeURIComponent(_string_temp).replace(/[!'()*]/g, escape);
			break;
		case "decode" :
			_string_temp = decodeURIComponent(_string_temp);
			_string_temp = _string_temp.replace(/\<br\>/g, "\n");
			break
	}
	
	return _string_temp;
}
function re_gen_wl_if(_wl_list) {
	var _new_wl_list = "";
	var _unit_subunit_array = _wl_list.split("wl");
	for(var i = 0; i < _unit_subunit_array.length; i += 1) {
		if(_unit_subunit_array[i] != "") {
			_new_wl_list += "wl" + _unit_subunit_array[i] + ">";
		}
	}
	return _new_wl_list;
}
function find_empty_gn_group() {
	var _empty_wl_idx = "";
	var _empty_flag = false;
	var _gn_count = multissid_support;
	for(_gn_count; _gn_count > 0; _gn_count -= 1) {
		_empty_flag = (gn_array_2g[(_gn_count - 1)][0] == "0") ? true : false;
		if(!_empty_flag)
			continue;
		if(wl_info.band5g_support) {
			_empty_flag = (gn_array_5g[(_gn_count - 1)][0] == "0") ? true : false;
			if(!_empty_flag)
				continue;
		}
		if(wl_info.band5g_2_support) {
			_empty_flag = (gn_array_5g_2[(_gn_count - 1)][0] == "0") ? true : false;
			if(!_empty_flag)
				continue;
		}
		if(_empty_flag) {
			_empty_wl_idx = _gn_count;
		}
		break;
	}
	return _empty_wl_idx;
}
function check_gn_status(_subunit) {
	var _gn_status = false;

	_gn_status = (gn_array_2g[(_subunit - 1)][0] == "0") ? true : false;
	if(!_gn_status)
		return _gn_status;
	if(wl_info.band5g_support) {
		_gn_status = (gn_array_5g[(_subunit - 1)][0] == "0") ? true : false;
		if(!_gn_status)
			return _gn_status;
	}
	if(wl_info.band5g_2_support) {
		_gn_status = (gn_array_5g_2[(_subunit - 1)][0] == "0") ? true : false;
		if(!_gn_status)
			return _gn_status;
	}

	return _gn_status;
}
function check_gn_if_status(_subunit, _gn_array) {
	var _gn_status = false;
	_gn_status = (_gn_array[(_subunit - 1)][0] == "0") ? true : false;
	return _gn_status;
}
function add_hint_msg(_$obj, _hintMsg) {
	if(_$obj.siblings('.hint_msg').length) {
		_$obj.siblings('.hint_msg').remove();
	}
	var $hintHtml = $('<div>');
	$hintHtml.addClass("hint_msg");
	$hintHtml.html(_hintMsg);
	_$obj.after($hintHtml);
}
function remove_hint_msg() {
	if($(".hint_msg").length) {
		$(".hint_msg").remove();
	}
}
</script>

</head>

<body onload="initial();" onunLoad="return unload_body();">
<div id="captive_portal_adv_log_panel" class="captive_portal_adv_log_panel"></div>
<div id="TopBanner"></div>
<div id="Loading" class="popup_bg"></div>
<iframe name="hidden_frame" id="hidden_frame" src="" width="0" height="0" frameborder="0"></iframe>
<form method="post" name="form" id="form" action="/start_apply2.htm" target="hidden_frame">
<input type="hidden" name="productid" value="<% nvram_get("productid"); %>">
<input type="hidden" name="current_page" value="Captive_Portal_Advanced.asp">
<input type="hidden" name="next_page" value="Captive_Portal_Advanced.asp">
<input type="hidden" name="group_id" value="">
<input type="hidden" name="modified" value="0">
<input type="hidden" name="action_mode" value="apply_new">
<input type="hidden" name="action_script" value="restart_wireless;restart_CP;restart_uam_srv;">
<input type="hidden" name="action_wait" value="50">
<input type="hidden" name="preferred_lang" id="preferred_lang" value="<% nvram_get("preferred_lang"); %>">
<input type="hidden" name="firmver" value="<% nvram_get("firmver"); %>">
<input type="hidden" name="captive_portal_adv_enable" value="<% nvram_get("captive_portal_adv_enable"); %>">
<input type="hidden" name="captive_portal_adv_profile" value="<% nvram_get("captive_portal_adv_profile"); %>">
<input type="hidden" name="captive_portal_adv_local_clientlist" value="<% nvram_get("captive_portal_adv_local_clientlist"); %>">
<input type="hidden" name="force_change" value="<% nvram_get("force_change"); %>">
<input type="hidden" name="captive_portal_adv_wl_en" value="">
<input type="hidden" name="captive_portal_adv_wl_dis" value="">
<input type="hidden" name="captive_portal_adv_2g_ssid" value='<% nvram_get("captive_portal_adv_2g_ssid"); %>'>
<input type="hidden" name="captive_portal_adv_5g_ssid" value='<% nvram_get("captive_portal_adv_5g_ssid"); %>'>
<input type="hidden" name="captive_portal_adv_5g_2_ssid" value='<% nvram_get("captive_portal_adv_5g_2_ssid"); %>'>
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
			<div id="captive_portal_adv_setting" class="captive_portal_adv_setting" onclick="reset_editing_area_css();"></div>
			<div id="captive_portal_adv_tool_help_bg" class="captive_portal_adv_tool_help_bg" onclick="switch_help('next', event)"></div>
			<input type='file' name='edit_upload_file' id='edit_upload_file' class='edit_upload_file' />
			<table width="98%" border="0" align="left" cellpadding="0" cellspacing="0">
				<tr>
					<td align="left" valign="top">
						
						<table width="760px" border="0" cellpadding="5" cellspacing="0" class="FormTitle" id="FormTitle">
							<tbody>
							<tr>
								<td bgcolor="#4D595D" valign="top">
									<div>&nbsp;</div>
									<div class="formfonttitle"><#Guest_Network#> - <#Captive_Portal#></div>
									<div style="margin:10px 0 10px 5px;" class="splitLine"></div>
									<div class="captive_portal_adv_intro_icon"></div>
									<div style='float:left;width:80%;'>
									<div class="captive_portal_adv_intro_txt" style="color:#FC0;"><#Captive_Portal_desc1#></div>
									<div class="captive_portal_adv_intro_txt"><#Captive_Portal_desc2#></div>
									</div>
									<div style="clear:both;"></div>
									<div style="margin:10px 0 10px 5px;" class="splitLine"></div>
									<table width="100%" border="1" align="center" cellpadding="4" cellspacing="0" bordercolor="#6b8fa3"  class="FormTable">
										<thead>
										<tr>
											<td colspan="2"><#t2BC#></td>
										</tr>
										</thead>
										<tr>
											<th><#Captive_Portal_enable#></th>
											<td>
												<div align="center" class="left" style="float:left;cursor:pointer;" id="radio_captive_portal_enable"></div>
												<div class="iphone_switch_container" style="height:32px; width:74px; position: relative; overflow: hidden;"></div>
												<script type="text/javascript">
												var captivePortalAdvEnable = 0;
												if(document.form.captive_portal_adv_enable.value == "on") {
													captivePortalAdvEnable = 1;
												}
												$('#radio_captive_portal_enable').iphoneSwitch(captivePortalAdvEnable,
													function() {
														captivePortalAdvShowAndHide(1);
													},
													function() {
														captivePortalAdvShowAndHide(0);
													},
													{
														switch_on_container_path: '/switcherplugin/iphone_switch_container_off.png'
													}
												);
											</script>
											</td>
										</tr>
										<tr id="tr_idle_timeout">
											<th><a class="hintstyle" href="javascript:void(0);" onClick="openHint(31, 1);"><#Captive_Portal_timeout#></a></th>
											<td>
												<input type="text" maxlength="3" class="input_6_table" id="captive_portal_adv_idle_timeout" name="captive_portal_adv_idle_timeout" value="<% nvram_get("captive_portal_adv_idle_timeout"); %>" onkeypress="return validator.isNumber(this,event)" autocorrect="off" autocapitalize="off">
												<span style="color:#FFCC00;">(<#Captive_Portal_Range#>: 60 - 800, <#Setting_factorydefault_value#>: 300 <#Second#>)</span>
											</td>
										</tr>
										<tr id="tr_event_log">
											<th><#Captive_Portal_event#></th>
											<td>
												<input class='button_gen' onclick='show_event_log();' type='button' value='<#liveupdate#>'/>
											</td>
										</tr>
									</table>

									<div id="captive_portal_adv_profile_bg">
										<div class="captive_portal_adv_create_bg" >
											<div class='captive_portal_adv_add' onclick="editProfile('new')"><div style='margin-left:1px;'>+</div></div>
											<span class='captive_portal_adv_add_title' onclick="editProfile('new')"><#vpnc_step1#></span>
										</div>

										<table width="100%" align="center" cellpadding="4" cellspacing="0" class="FormTable_table">
											<thead>
											<tr>
												<td id="td_profile_table_title" colspan="5"><#Captive_Portal_Profile#> &nbsp;(<#List_limit#>&nbsp;1)</td>
											</tr>
											</thead>
											<tr>
												<th width="30%"><#FreeWiFi_BrandName#></th>
												<th width="20%"><#PPPConnection_Authentication_itemname#></th>
												<th width="30%"><#QIS_finish_wireless_item1#></th>
												<th width="10%"><#pvccfg_edit#></th>
												<th width="10%"><#CTL_del#></th>
											</tr>
										</table>
										<div id="captive_portal_adv_profile_block"></div>
									</div>
									<div class='apply_content'>
										<input class='button_gen' onclick='cpa_apply();' type='button' value='<#CTL_apply#>'/>
									</div>
								</td>
							</tr>
							</tbody>
						</table>
					</td>
				</tr>
			</table>
		</td>
		<td width="10" align="center" valign="top">&nbsp;</td>
	</tr>
</table>
<!--===================================End of Main Content===========================================-->
</form>
<iframe name="hidden_frame_save" id="hidden_frame_save" src="" width="0" height="0" frameborder="0"></iframe>
<form method="post" name="splash_page_form" action="splash_page.cgi" target="hidden_frame_save" enctype="multipart/form-data">
<input type="hidden" name="current_page" value="Captive_Portal.asp">
<input type="hidden" name="next_page" value="Captive_Portal.asp">
<input type="hidden" name="preferred_lang" value="<% nvram_get("preferred_lang"); %>">
<input type="hidden" name="firmver" value="<% nvram_get("firmver"); %>">
<input type="hidden" name="splash_page_id" value="">
<input type="hidden" name="splash_page_attribute" value="">
<input type="hidden" name="splash_page_html" value="">
<input type="hidden" name="splash_page_css" value="">
<input type="hidden" name="splash_page_error" value="" disabled>
</form>
<iframe name="hidden_frame_del" id="hidden_frame_del" src="" width="0" height="0" frameborder="0"></iframe>
<form method="post" name="splash_page_form_del" action="splash_page_del.cgi" target="hidden_frame_save" enctype="multipart/form-data">
<input type="hidden" name="current_page" value="Captive_Portal.asp">
<input type="hidden" name="next_page" value="Captive_Portal.asp">
<input type="hidden" name="preferred_lang" value="<% nvram_get("preferred_lang"); %>">
<input type="hidden" name="firmver" value="<% nvram_get("firmver"); %>">
<input type="hidden" name="splash_page_id_del" value="">
</form>
<div id="footer"></div>
</body>
</html>
