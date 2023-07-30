﻿<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01 Transitional//EN"
"http://www.w3.org/TR/html4/loose.dtd">
<head>
<meta http-equiv="Content-Type" content="text/html; charset=utf-8" />
<meta HTTP-EQUIV="Pragma" CONTENT="no-cache">
<meta HTTP-EQUIV="Expires" CONTENT="-1">
<link rel="shortcut icon" href="images/favicon.png">
<link rel="icon" href="images/favicon.png">
<link rel="stylesheet" type="text/css" href="/NM_style.css" />
<link rel="stylesheet" type="text/css" href="/form_style.css" />
<link rel="stylesheet" type="text/css" href="/index_style.css" />
<link rel="stylesheet" type="text/css" href="/device-map/amesh.css" />
<link rel="stylesheet" type="text/css" href="/device-map/device-map.css" />
<title>device-map/amesh.asp</title>
<script type="text/javascript" src="/state.js"></script>
<script type="text/javascript" src="/help.js"></script>
<script type="text/javascript" src="/js/jquery.js"></script>
<script type="text/javascript" src="/client_function.js"></script>
<script type="text/javascript" src="../js/httpApi.js"></script>
<script type="text/javascript" src="../js/sorterApi.js"></script>
<script>

if(parent.location.pathname.search("index") === -1) top.location.href = "../"+'<% networkmap_page(); %>';

var id = '<% get_parameter("id"); %>';
var processCount = 0;
var checkCloudIconExist = new Array();
var interval_ajax_onboarding_status;
var interval_ajax_get_onboardinglist_status;
var interval_restore_pairing_status;
var interval_ajax_AiMesh_node_clients_status;
var checkCloudIconErrorTimes = new Array();
var onboarding_flag = false;
var search_result_fail = "init";
var aimesh_node_client_list = new Array();
var aimesh_node_client_list_colspan = 4;
var aimesh_node_client_info_width = (top.isIE8) ? ["", "40%", "40%", "20%"] : ["15", "40%", "30%", "15%"];
var aimesh_node_client_upload_icon = new Array();
var aimesh_select_new_re_mac = "";
var AUTOLOGOUT_MAX_MINUTE_ORI = 0;
var restore_autologout = false;
var faq_href = "https://nw-dlcdnet.asus.com/support/forward.html?model=&type=Faq&lang="+ui_lang+"&kw=&num=149";
var aimesh_href = "https://nw-dlcdnet.asus.com/support/forward.html?model=&type=AiMesh&lang="+ui_lang+"&kw=&num=";
var led_control = {
	"status": function(_node_info){
		var result = {"support": 0, "value": 0};
		var node_capability = httpApi.aimesh_get_node_capability(_node_info);
		if(node_capability.central_led) {
			result.support = "central_led";
			if("config" in _node_info) {//led value
				if("central_led" in _node_info.config) {
					if("bc_ledLv" in _node_info.config.central_led)
						result.value = parseInt(_node_info.config.central_led.bc_ledLv);
				}
			}
		}
		else if(node_capability.lp55xx_led) {
			result.support = "lp55xx_led";
			if("lp55xx_led" in _node_info.config) {
				if("lp55xx_lp5523_user_enable" in _node_info.config.lp55xx_led)
					result.value = parseInt(_node_info.config.lp55xx_led.lp55xx_lp5523_user_enable);
			}
		}
		else if(node_capability.led_on_off) {
			result.support = "ctrl_led";
			if("ctrl_led" in _node_info.config) {
				if("led_val" in _node_info.config.ctrl_led)
					result.value = parseInt(_node_info.config.ctrl_led.led_val);
			}
		}
		if(isNaN(result.value))
			result.value = 0;

		return result;
	},
	"component": function(_supportType) {
		var html = "";
		switch(_supportType) {
			case "central_led" :
				html += "<div style='margin-top:10px;'>";
				html += "<div class='aimesh_node_setting_info_title'><#LED_Brightness#></div>";
				html += "<div class='aimesh_node_setting_info_content'>";
				html += "<div id='led_slider' class='led_slider'></div>";
				html += "<div id='led_text'></div>";
				html += "</div>";
				html += "<div class='clear_both'></div>";
				html += "</div>";
				break;
			case "lp55xx_led" :
			case "ctrl_led" :
				html += "<div style='margin-top:10px;'>";
				html += "<div class='aimesh_node_setting_info_title'><#BoostKey_LED#></div>";
				html += "<div class='aimesh_node_setting_info_content'>";
				html += "<div align='center' style='float:left;cursor:pointer;' id='led_radio'></div>";
				html += "</div>";
				html += "<div class='clear_both'></div>";
				html += "</div>";
				break;
		}
		return html;
	}
};

function initial(){
	if(parent.$('link[rel=stylesheet][href~="/device-map/amesh.css"]').length > 0)
		parent.$('link[rel=stylesheet][href~="/device-map/amesh.css"]').remove();

	var add_css_status = setInterval(add_css, 300);
	function add_css() {
		if(parent.$('link[rel=stylesheet][href~="/device-map/amesh.css"]').length > 0)
			clearInterval(add_css_status);
		else
			parent.addNewCSS("/device-map/amesh.css");
	}

	ajax_onboarding();
	interval_ajax_onboarding_status = setInterval(ajax_onboarding, 5000);

	initial_amesh_obj();

	//reset_NM_height
	var NM_table_height = parseInt(parent.$("#NM_table").css("height"));
	parent.$("#statusframe").css("height",  NM_table_height - 50);
	parent.$(".NM_radius_bottom_container").css("height", NM_table_height - 45);
}
function ajax_onboarding() {
	$.ajax({
		url: '/ajax_onboarding.asp',
		dataType: 'script',
		success: function() {
			//check cfg server ready or not
			if(get_onboardingstatus.cfg_obstatus == "")
				return;

			/* Update ready_onBoarding_block */
			var list_status = Object.keys(get_onboardinglist).length;
			if(list_status > 0) {
				if(get_onboardingstatus.cfg_obstatus != "1") {
					$('#ready_onBoarding_table').css("display", "");
					gen_ready_onboardinglist(get_onboardinglist);
					search_result_fail = false;
				}
			}
			else {
				if(!onboarding_flag) {
					$('#ready_onBoarding_block').empty();
					$('#ready_onBoarding_table').css("display", "none");
					if(typeof parent.show_AMesh_status !== 'undefined' && $.isFunction(parent.show_AMesh_status))
						parent.show_AMesh_status(0, 0);

					if(get_onboardingstatus.cfg_obstatus == "1") {
						if(search_result_fail == true) {
							show_search_fail_result();
							search_result_fail = "init";
						}
						else if(search_result_fail == false)
							search_result_fail = "init";
					}
				}
				if(parent.$("#amesh_connect_msg").length)
					parent.$("#amesh_connect_msg").remove();
			}

			if(onboarding_flag) {
				$("#searchReadyOnBoarding").css("display", "none");
				$("#amesh_loadingIcon").css("display", "none");
				search_result_fail = false;
				if(!restore_autologout) {
					AUTOLOGOUT_MAX_MINUTE_ORI = parent.AUTOLOGOUT_MAX_MINUTE;
					parent.AUTOLOGOUT_MAX_MINUTE = 0;
					restore_autologout = true;
				}
			}
			else {
				if(get_onboardingstatus.cfg_obstatus == "2") {
					$("#searchReadyOnBoarding").css("display", "none");
					$("#amesh_loadingIcon").css("display", "");
					if(search_result_fail == "init")
						search_result_fail = true;
					if(!restore_autologout) {
						AUTOLOGOUT_MAX_MINUTE_ORI = parent.AUTOLOGOUT_MAX_MINUTE;
						parent.AUTOLOGOUT_MAX_MINUTE = 0;
						restore_autologout = true;
					}
				}
				else {
					$("#searchReadyOnBoarding").css("display", "");
					$("#amesh_loadingIcon").css("display", "none");
					aimesh_select_new_re_mac = "";
					if(restore_autologout) {
						parent.AUTOLOGOUT_MAX_MINUTE = AUTOLOGOUT_MAX_MINUTE_ORI;
						restore_autologout = false;
						AUTOLOGOUT_MAX_MINUTE_ORI = 0;
					}
				}
			}
			/* Update ready_onBoarding_block end */

			/* Update onBoarding_block */
			var list_status = get_cfg_clientlist.length;
			if(list_status > 1) {
				if($("#onBoarding_block").children(".amesh_no_data").length > 0)
					$("#onBoarding_block").children(".amesh_no_data").remove();
				gen_current_onboardinglist(get_cfg_clientlist, get_wclientlist, get_wiredclientlist, get_allclientlist);
			}
			else {
				$('#onBoarding_block').empty();
				$('#onBoarding_block').append("<div class='amesh_no_data'><#IPConnection_VSList_Norule#></div>");
				if(typeof parent.show_AMesh_status !== 'undefined' && $.isFunction(parent.show_AMesh_status))
					parent.show_AMesh_status(0, 1);
			}
			/* Update onBoarding_block end */

			//restore pairing status
			if(get_onboardingstatus.cfg_obresult != "" && get_onboardingstatus.cfg_newre != "" && get_onboardingstatus.cfg_obstatus == "4" && get_onboardingstatus.cfg_obstart != "" && get_onboardingstatus.cfg_obtimeout != "") {
				if(!onboarding_flag) {
					onboarding_flag = true;
					var device_id = get_onboardingstatus.cfg_newre.replace(/:/g, "").toUpperCase();
					if($('#ready_onBoarding_block').find('#' + device_id + '').length == 1) {
						$('#ready_onBoarding_block').find("#" + device_id + "").find(".loading-container").css("display", "");
						$('#ready_onBoarding_block').find("#" + device_id + "").find(".amesh_each_router_icon_bg").css("display", "none");
						processCount = set_process_percentage(get_onboardingstatus.cfg_obstart, get_onboardingstatus.cfg_obcurrent, get_onboardingstatus.cfg_obtimeout , 100);
						switch(parseInt(get_onboardingstatus.cfg_obresult)) {
							case 0 : //Init
							case 1 : //Start
							case 3 : //WPS Success
								if(processCount >= 99)
									processCount = 99;
								break;
							case 2 : //Success
							case 4 : //WPS Fail
							case 5 : //Terminate
								if(processCount >= 100)
									processCount = 100;
								break;
						}
						$('#ready_onBoarding_block').find("#" + device_id + "").find(".processText").html("" + processCount + " %");
						ajax_get_onboardinglist_status();
						interval_ajax_get_onboardinglist_status = setInterval(ajax_get_onboardinglist_status, 1000);
						$('#ready_onBoarding_block').find("#" + device_id + "").find(".amesh_rotate").addClass("connect");
						$('#ready_onBoarding_block').find("#" + device_id + "").find(".amesh_line_run").addClass("connect");
						$("#searchReadyOnBoarding").css("display", "none");
						$("#amesh_loadingIcon").css("display", "none");
					}
				}
			}

			/* Handle auto onboarding start */
			if(id != "" && (Session.get("AiMesh_id") == undefined || Session.get("AiMesh_id") != id) ) {
				Session.set("AiMesh_id", id);
				Session.set("AiMesh_id_status", "0");
				var mac = formatMAC(id);
				var re_exist = false;
				for (var idx in get_cfg_clientlist) {
					if(get_cfg_clientlist.hasOwnProperty(idx)) {
						if(idx != 0) {
							if(get_cfg_clientlist[idx].mac.toUpperCase() == mac) {
								re_exist = true;
								break;
							}
						}
					}
				}
				if(re_exist) {
					id = "";
				}
				else {
					if(get_onboardingstatus.cfg_obstatus == "2") {
						var onboarding_exist = false; 
						Object.keys(get_onboardinglist).forEach(function(key) {
							var reMac = key;
							var newReMacArray = get_onboardinglist[reMac];
							Object.keys(newReMacArray).forEach(function(key) {
								var newReMac = key;
								if(newReMac == mac) {
									var model_name = newReMacArray[newReMac].model_name;
									var ui_model_name = newReMacArray[newReMac].ui_model_name;
									var rssi = newReMacArray[newReMac].rssi;
									var source = newReMacArray[newReMac].source;
									show_connect_msg(reMac, newReMac, model_name, ui_model_name, rssi, source);
									onboarding_exist = true;
									document.onboardingLED_form.new_re_mac.disabled = false;
									document.onboardingLED_form.new_re_mac.value = newReMac;
									document.onboardingLED_form.ob_path.disabled = false;
									document.onboardingLED_form.ob_path.value = source;
									document.onboardingLED_form.submit();
								}
							});
						});
						if(onboarding_exist)
							id = "";
						else
							show_connect_msg("", mac, "New Node", "", "-1");
					}
					else
						id = "";
				}
			}
			/* Handle auto onboarding end */

			/* Update show_connect_msg start */
			if(parent.$("#amesh_connect_msg").length) {
				var classArray = parent.$("#amesh_connect_msg").attr("class").split(" ");
				var mac = "";
				if(classArray[1] != undefined)
					mac = formatMAC(classArray[1]);
				
				Object.keys(get_onboardinglist).forEach(function(key) {
					var reMac = key;
					var newReMacArray = get_onboardinglist[reMac];
					Object.keys(newReMacArray).forEach(function(key) {
						var newReMac = key;
						if(newReMac == mac) {
							var reMac = key;
							var model_name = newReMacArray[newReMac].model_name;
							var ui_model_name = newReMacArray[newReMac].ui_model_name;
							var rssi = newReMacArray[newReMac].rssi;
							var device_info = handle_ui_model_name(model_name, ui_model_name) + "<br>";
							var labelMac = newReMac;
							httpApi.getAiMeshLabelMac(model_name, newReMac, 
								function(_callBackMac){
									labelMac = _callBackMac;
								}
							);
							device_info += labelMac;
							var source = newReMacArray[newReMac].source;
							parent.$("#amesh_connect_msg").find(".amesh_hint_text.amesh_device_info").html(device_info);
							parent.$("#amesh_connect_msg").find(".amesh_hint_text.amesh_device_info").css("display", "");
							parent.$("#amesh_connect_msg").find(".wait_search").css("display", "none");

							if((parseInt(rssi) < parseInt(get_onboardingstatus.cfg_wifi_quality)) && (source != "2")) {
								parent.$("#amesh_connect_msg").find(".quality_ok").css("display", "none");
								parent.$("#amesh_connect_msg").find(".quality_weak").css("display", "");
							}
							else {
								parent.$("#amesh_connect_msg").find(".quality_ok").css("display", "");
								parent.$("#amesh_connect_msg").find(".quality_weak").css("display", "none");
							}
							parent.$("#amesh_connect_msg").find(".amesh_hint_text.amesh_hint_title").html("<#AiMesh_Node_AddConfirm#>");
						}
					});
				});
			}
			/* Update show_connect_msg end */
		}
	});
}
function gen_ready_onboardinglist(_onboardingList) {
	Object.keys(_onboardingList).forEach(function(key) {
		var reMac = key;
		var newReMacArray = _onboardingList[reMac];
		Object.keys(newReMacArray).forEach(function(key) {
			var newReMac = key;
			var model_name = newReMacArray[newReMac].model_name;
			var ui_model_name = newReMacArray[newReMac].ui_model_name;
			var icon_model_name = "";
			var rssi = newReMacArray[newReMac].rssi;
			var onboarding_device_id = newReMac.replace(/:/g, "");
			var source = newReMacArray[newReMac].source;
			var tcode = newReMacArray[newReMac].tcode;
			var type = newReMacArray[newReMac].type;
			var cobrand = httpApi.aimesh_get_misc_info(newReMacArray[newReMac]).cobrand;
			var model_info = {"model_name": model_name, "tcode": tcode, "cobrand": cobrand, "icon_model_name": icon_model_name};
			var cloudModelName = httpApi.transformCloudModelName(model_info);
			model_info.cloudModelName = cloudModelName;
			if($('#ready_onBoarding_block').find('#' + onboarding_device_id + '').length == 0) {
				if(checkCloudIconExist[cloudModelName] == undefined)
					checkCloudIconExist[cloudModelName] = false;

				var code = "";
				code += "<div id='" + onboarding_device_id + "'>";
					code += "<div class='amesh_rotate'>";
					code += "<div class='amesh_line'><i class='amesh_line_run'></i></div>";
					code += "<div class='amesh_line'><i class='amesh_line_run'></i></div>";
					code += "<div class='amesh_line'><i class='amesh_line_run'></i></div>";
					code += "<div class='amesh_line'><i class='amesh_line_run'></i></div>";
					code += "<div class='amesh_router_icon_bg'>";
						code += "<div class='amesh_each_router_icon_bg'></div>";
						if(checkCloudIconExist[cloudModelName])
							code += "<div class='amesh_each_router_icon_bg amesh_router_image_web'></div>";
						else
							code += "<div class='amesh_each_router_icon_bg amesh_router_icon'></div>";
						code += "<div class='loading-container' style='display:none;'>";
						code += "<div class='loadingAnimation'></div>";
						code += "<div class='processText'>100 %</div>";
						code += "</div>";
					code += "</div>";
					code += "<div class='vertical_line'></div>";
					code += "<div class='amesh_router_info_bg'>";
						code += "<div class='amesh_router_info_text model_name'>";
						code += handle_ui_model_name(model_name, ui_model_name);
						code += "</div>";
						code += "<div class='horizontal_line'></div>";
						code += "<div style='position:relative;height:20px;'>";
							code += "<div class='amesh_router_info_text'>";
							var labelMac = newReMac;
							httpApi.getAiMeshLabelMac(model_name, newReMac, 
								function(_callBackMac){
									labelMac = _callBackMac;
								}
							);
							code += labelMac;
							code += "</div>";
							code += "<div class='amesh_interface_icon'>";
							if(source == 2){
								if(type != undefined && type == "65536")
									code += "<div class='radioIcon radio_plc'></div>";
								else if(type != undefined && type == "131072")
									code += "<div class='radioIcon radio_moca'></div>";
								else
									code += "<div class='radioIcon radio_wired'></div>";
							}
							else
								code += "<div class='radioIcon radio_" + client_convRSSI(rssi) + "'></div>";
							code += '</div>';
						code += "</div>";
					code += "</div>";
					code += "</div>";
				code += "</div>";
				$('#ready_onBoarding_block').append(code);

				$('#ready_onBoarding_block').find('#' + onboarding_device_id + '').find('.amesh_rotate').unbind('click');
				$('#ready_onBoarding_block').find('#' + onboarding_device_id + '').find('.amesh_rotate').click(
					function() {
						show_connect_msg(reMac, newReMac, model_name, ui_model_name, rssi, source);
					}
				);

				if(isNaN(parseInt(checkCloudIconErrorTimes[model_name])))
					checkCloudIconErrorTimes[model_name] = 0;
				download_cloud_icon(model_info, onboarding_device_id, "ready_onBoarding_block");

				if(typeof parent.show_AMesh_status !== 'undefined' && $.isFunction(parent.show_AMesh_status))
					parent.show_AMesh_status($('#ready_onBoarding_block').children().length, 0);
			}
			else {
				$('#ready_onBoarding_block').find('#' + onboarding_device_id + '').find('.amesh_rotate').unbind('click');
				$('#ready_onBoarding_block').find('#' + onboarding_device_id + '').find('.amesh_rotate').click(
					function() {
						show_connect_msg(reMac, newReMac, model_name, ui_model_name, rssi, source);
					}
				);
				if(newReMac != aimesh_select_new_re_mac){
					if(source == 2){
						if(type != undefined && type == "65536")
							$('#ready_onBoarding_block').find('#' + onboarding_device_id + '').children().find('.radioIcon').removeClass().addClass('radioIcon radio_plc');
						else if(type != undefined && type == "131072")
							$('#ready_onBoarding_block').find('#' + onboarding_device_id + '').children().find('.radioIcon').removeClass().addClass('radioIcon radio_moca');
						else
							$('#ready_onBoarding_block').find('#' + onboarding_device_id + '').children().find('.radioIcon').removeClass().addClass('radioIcon radio_wired');
					}
					else
						$('#ready_onBoarding_block').find('#' + onboarding_device_id + '').children().find('.radioIcon').removeClass().addClass('radioIcon radio_' + client_convRSSI(rssi) + '');
				}
				$('#ready_onBoarding_block').find('#' + onboarding_device_id + '').children().find('.amesh_router_info_text.model_name').html(handle_ui_model_name(model_name, ui_model_name));

				if(isNaN(parseInt(checkCloudIconErrorTimes[cloudModelName])))
					checkCloudIconErrorTimes[cloudModelName] = 0;
				download_cloud_icon(model_info, onboarding_device_id, "ready_onBoarding_block");
			}
		});
	});
}
var setAMeshAttr = function(){
	this.alias = "";
	this.ip = "";
	this.rssi = "";
};

function gen_current_onboardinglist(_onboardingList, _wclientlist, _wiredclientlist, _allclientlist) {
	originData.fromNetworkmapd[0] = httpApi.hookGet("get_clientlist", true);
	genClientList();

	$('#onBoarding_block').empty();
	for (var idx in _onboardingList) {
		if(_onboardingList.hasOwnProperty(idx)) {
			if(idx != 0) {
				var model_name = _onboardingList[idx].model_name;
				var ui_model_name = _onboardingList[idx].ui_model_name;
				var icon_model_name = _onboardingList[idx].icon_model_name;
				var fwver = _onboardingList[idx].fwver;
				var mac = _onboardingList[idx].mac.toUpperCase();
				var device_id = mac.replace(/:/g, "");
				var ipAddr = _onboardingList[idx].ip;
				var pap2g = _onboardingList[idx].pap2g;
				var pap5g = _onboardingList[idx].pap5g;
				var rssi2g = _onboardingList[idx].rssi2g;
				var rssi5g = _onboardingList[idx].rssi5g;
				var rssi6g = _onboardingList[idx].rssi6g;
				var online = _onboardingList[idx].online;
				var connect_type = handle_re_path(_onboardingList[idx].re_path);
				var tcode = _onboardingList[idx].tcode;
				var cobrand = httpApi.aimesh_get_misc_info(_onboardingList[idx]).cobrand;
				var wireless_band = 0;
				var wireless_band_array = ["2.4G", "5G", "6G"];
				var wireless_rssi = 4;
				var alias = "Home";
				var model_info = {"model_name": model_name, "tcode": tcode, "cobrand": cobrand, "icon_model_name": icon_model_name};
				var cloudModelName = httpApi.transformCloudModelName(model_info);
				model_info.cloudModelName = cloudModelName;

				if("config" in _onboardingList[idx]) {
					if("misc" in _onboardingList[idx].config) {
						if("cfg_alias" in _onboardingList[idx].config.misc) {
							if(_onboardingList[idx].config.misc.cfg_alias != "")
								alias = _onboardingList[idx].config.misc.cfg_alias;
						}
					}
				}
				var location_text = "<#AiMesh_NodeLocation01#>";
				var specific_location = aimesh_location_arr.filter(function(item, index, _array){
					return (item.value == alias);
				})[0];
				if(specific_location != undefined)
					location_text = specific_location.text;
				else
					location_text = alias;

				if($("script[src='../calendar/jquery-ui.js']").length == 0) {
					if(led_control.status(_onboardingList[idx]).support == "central_led")
						addNewScript("../calendar/jquery-ui.js");
				}
				if(parent.$("script[src='/calendar/jquery-ui.js']").length == 0) {
					if(led_control.status(_onboardingList[idx]).support == "central_led")
						parent.addNewScript("/calendar/jquery-ui.js");
				}
				if($("script[src='../switcherplugin/jquery.iphone-switch.js']").length == 0) {
					if(led_control.status(_onboardingList[idx]).support == "lp55xx_led" || led_control.status(_onboardingList[idx]).support == "ctrl_led")
						addNewScript("../switcherplugin/jquery.iphone-switch.js");
				}

				if(connect_type == "1" || connect_type == "16" || connect_type == "32" || connect_type == "64"){
					var real_port_type = gen_real_port_type(_onboardingList[idx]);
					if(real_port_type.type == "eth")
						wireless_rssi = "wired";
					else if(real_port_type.type == "plc")
						wireless_rssi = "plc";
					else if(real_port_type.type == "moca")
						wireless_rssi = "moca";
				}
				else if(connect_type == "2") {
					wireless_band = 0;
					wireless_rssi = client_convRSSI(rssi2g);
				}
				else if(connect_type == "128") {
					wireless_band = 2;
					wireless_rssi = client_convRSSI(rssi6g);
				}
				else if(connect_type <= 0) {
					online = 0;
					wireless_rssi = "wired";
				}
				else {
					wireless_band = 1;
					wireless_rssi = client_convRSSI(rssi5g);
				}

				if($('#ready_onBoarding_block').find('#' + device_id + '').length == 0 && 
					$('#onBoarding_block').find('#' + device_id + '').length == 0
				) {
					if(checkCloudIconExist[cloudModelName] == undefined)
						checkCloudIconExist[cloudModelName] = false;

					var code = "";
					code += "<div id='" + device_id + "' class='amesh_pairing_bg' >";
						code += "<div class='amesh_router_icon_bg pairing'>";
							if(checkCloudIconExist[cloudModelName])
								code += "<div class='amesh_each_router_icon_bg amesh_router_image_web' style='background-image:url(\"" + checkCloudIconExist[cloudModelName]  + "\");'></div>";
							else
								code += "<div class='amesh_each_router_icon_bg amesh_router_icon'></div>";
						code += "</div>";
						code += "<div class='vertical_line pairing'></div>";
						code += "<div class='amesh_router_info_bg'>";
							code += "<div class='amesh_router_info_title'>";
							var display_model_name = handle_ui_model_name(model_name, ui_model_name);
							code += "<div class='amesh_model_name' title='" + display_model_name + "'>" + display_model_name + "</div>";
							code += "<div class='device_reset' onclick='reset_re_device(\"" + mac + "\", \"" + model_name + "\", \"" + ui_model_name + "\", event, \"" + online + "\");'></div>";
							code += "</div>";
							code += "<div class='horizontal_line'></div>";
							code += "<div style='position:relative;'>";
								code += "<div class='amesh_router_info_text location' title='" + htmlEnDeCode.htmlEncode(location_text) + "'>";
								var node_location_text = location_text;
								if(location_text.length > 22) {
									node_location_text = location_text.substring(0, 20) + "..";
								}
								code += "<span class='amesh_node_content'>" + htmlEnDeCode.htmlEncode(node_location_text) + "</span>";
								code += "</div>";
								code += "<div class='amesh_router_info_text'><#Full_Clients#>: ";
								var re_client_num = 0;
								if(online == "1" && AiMeshTotalClientNum[mac] != undefined) {
									re_client_num = AiMeshTotalClientNum[mac];
								}
								code += "<span class='amesh_node_content'>" + re_client_num + "</span>";
								code += "</div>";
								if(online == "1") {
									code += "<div class='amesh_interface_icon'>";
									code += "<div class='radioIcon radio_" + wireless_rssi + "'></div>";
									if(connect_type != "1" && connect_type != "16" && connect_type != "32" && connect_type != "64") {
										var bandClass = (navigator.userAgent.toUpperCase().match(/CHROME\/([\d.]+)/)) ? "band_txt_chrome" : "band_txt";
										code += "<div class='band_block'><span class=" + bandClass + ">" + wireless_band_array[wireless_band] + "</span></div>";
									}
									code += "</div>";
								}
								else {
									code += "<div class='amesh_title offline' onclick='show_offline_msg(event);'><#Clientlist_OffLine#></div>";
								}
							code += "</div>";
						code += "</div>";
					code += "</div>";

					$('#onBoarding_block').append(code);

					$('#onBoarding_block').find('#' + device_id + '').click(
						{"node_info" : _onboardingList[idx], "wl_client" : _wclientlist[mac], "wired_client" :  _wiredclientlist[mac]},
						popAMeshClientListEditTable);

					if(isNaN(parseInt(checkCloudIconErrorTimes[cloudModelName])))
						checkCloudIconErrorTimes[cloudModelName] = 0;
					if(!checkCloudIconExist[cloudModelName])
						download_cloud_icon(model_info, device_id, "onBoarding_block");
				}
			}
		}
	}

	/* Update ameshNumber start */
	if($('#onBoarding_block').children().length == 0) {
		$('#onBoarding_block').append("<div class='amesh_no_data'><#IPConnection_VSList_Norule#></div>");
		if(typeof parent.show_AMesh_status !== 'undefined' && $.isFunction(parent.show_AMesh_status))
			parent.show_AMesh_status(0, 1);
	}
	else {
		if(typeof parent.show_AMesh_status !== 'undefined' && $.isFunction(parent.show_AMesh_status))
			parent.show_AMesh_status($('#onBoarding_block').children().length, 1);
	}
	/* Update ameshNumber end */
}

function getAiMeshOnboardinglist(_onboardingList){
	var jsonArray = [];
	var profile = function(){
		this.name = "";
		this.ui_model_name = "";
		this.signal = "";
		this.rssi = "";
		this.source = "";
		this.mac = "";
		this.pap_mac = "";
		this.id = "";
	};
	var convRSSI = function(val) {
		var result = 1;
		val = parseInt(val);
		if(val >= -50) result = 4;
		else if(val >= -80) result = Math.ceil((24 + ((val + 80) * 26)/10)/25);
		else if(val >= -90) result = Math.ceil((((val + 90) * 26)/10)/25);
		else return 1;

		if(result == 0) result = 1;
		return result;
	};

	Object.keys(_onboardingList).forEach(function(key) {
		var papMac = key;
		var newReMacArray = _onboardingList[papMac];
		Object.keys(newReMacArray).forEach(function(key) {
			var newReMac = key;
			var node_info  = new profile();
			node_info.name = newReMacArray[newReMac].model_name;
			node_info.ui_model_name = newReMacArray[newReMac].ui_model_name;
			node_info.signal = convRSSI(newReMacArray[newReMac].rssi);
			node_info.rssi = newReMacArray[newReMac].rssi;
			node_info.source = newReMacArray[newReMac].source;
			node_info.mac = newReMac;
			node_info.pap_mac = papMac;
			node_info.id = newReMac.replace(/:/g, "");
			jsonArray.push(node_info);
		});
	});

	return jsonArray;
}

function connectingDevice(_reMac, _newReMac, delay) {
	var device_id = _newReMac.replace(/:/g, "").toUpperCase();
	$('#ready_onBoarding_block').find("#" + device_id + "").find(".loading-container").css("display", "");
	$('#ready_onBoarding_block').find("#" + device_id + "").find(".amesh_each_router_icon_bg").css("display", "none");
	processCount = 0;
	$('#ready_onBoarding_block').find("#" + device_id + "").find(".processText").html("" + processCount + " %");
	interval_ajax_get_onboardinglist_status = setInterval(ajax_get_onboardinglist_status, 1000);
	$('#ready_onBoarding_block').find("#" + device_id + "").find(".amesh_rotate").addClass("connect");
	$('#ready_onBoarding_block').find("#" + device_id + "").find(".amesh_line_run").addClass("connect");
	$("#searchReadyOnBoarding").css("display", "none");
	$("#amesh_loadingIcon").css("display", "none");
	onboarding_flag = true;

	var onboardingSearch = function(){
		httpApi.nvramSet({"action_mode": "onboarding"})

		setTimeout(function(){
			var obList = getAiMeshOnboardinglist(httpApi.hookGet("get_onboardinglist", true));
			var got = false;

			obList.forEach(function(nodeInfo){
				if(nodeInfo.mac == _newReMac){
					got = true;

					httpApi.nvramSet({
						"action_mode": "ob_selection", 
						"new_re_mac": nodeInfo.mac, 
						"ob_path": nodeInfo.source
					}, function(){
						setTimeout(function(){
							httpApi.nvramSet({
								"action_mode": "onboarding",
								"re_mac": nodeInfo.pap_mac,
								"new_re_mac": nodeInfo.mac
							});
						}, 3000);
					}());
				}
			})

			if(!got){
				setTimeout(arguments.callee, 1000);
			}
		}, 2000);
	}

	if(delay){
		setTimeout(onboardingSearch, parseInt(delay)*1000)
	}
	else{
		document.form.re_mac.disabled = false;
		document.form.new_re_mac.disabled = false;
		document.form.re_mac.value = _reMac;
		document.form.new_re_mac.value = _newReMac;
		document.form.submit();	
	}
}
function ajax_get_onboardinglist_status() {
	var accelerate_count = function(_device_id) {
		if(interval_ajax_get_onboardinglist_status) {
			clearInterval(interval_ajax_get_onboardinglist_status);
			interval_ajax_get_onboardinglist_status = false;
		}

		var process_accumulate_status = setInterval(process_accumulate, 100);
		function process_accumulate() {
			if(processCount >= 100) {
				processCount = 100;
				clearInterval(process_accumulate_status);
				ajax_get_onboardinglist_status();
			}
			else {
				processCount++;
				$('#ready_onBoarding_block').find("#" + _device_id + "").find(".processText").html("" + processCount + " %");
			}
		}
	};
	$.ajax({
		url: '/ajax_onboarding.asp',
		dataType: 'script',
		success: function() {
			onboarding_flag = true;
			var device_id = get_onboardingstatus.cfg_newre.replace(/:/g, "").toUpperCase();
			var cfg_obresult = get_onboardingstatus.cfg_obresult;
			if(Object.keys(get_onboardinglist).length > 0) {
				if(cfg_obresult != "" && get_onboardingstatus.cfg_newre != "" && get_onboardingstatus.cfg_obstatus == "4" && get_onboardingstatus.cfg_obstart != "" && get_onboardingstatus.cfg_obtimeout != "") {
					processCount = set_process_percentage(get_onboardingstatus.cfg_obstart, get_onboardingstatus.cfg_obcurrent, get_onboardingstatus.cfg_obtimeout , 100);
					switch(parseInt(cfg_obresult)) {
						case 0 : //Init
						case 1 : //Start
						case 3 : //WPS Success
							if(processCount >= 99)
								processCount = 99;
							break;
						case 2 : //Success
						case 4 : //WPS Fail
						case 5 : //Terminate
							if(processCount >= 100)
								processCount = 100;
							break;
					}
				}
				else if(get_onboardingstatus.cfg_obstatus == "1") {//for onboarding abnormal
					if(processCount < 100) {
						accelerate_count(device_id);
						cfg_obresult = 4;
					}
				}
			}
			else {
				//for onboarding finish
				if(get_onboardingstatus.cfg_obstatus == "1" && (get_onboardingstatus.cfg_obresult == "2" || get_onboardingstatus.cfg_obresult == "4" || get_onboardingstatus.cfg_obresult == "5") && get_onboardingstatus.cfg_newre != "") {
					if(processCount < 100)
						accelerate_count(device_id);
				}
			}

			$('#ready_onBoarding_block').find("#" + device_id + "").find(".processText").html("" + processCount + " %");

			if(processCount >= 100) {
				if(interval_ajax_get_onboardinglist_status) {
					clearInterval(interval_ajax_get_onboardinglist_status);
					interval_ajax_get_onboardinglist_status = false;
				}
				show_connect_result(cfg_obresult, get_onboardingstatus.cfg_newre, get_onboardingstatus.cfg_obmodel, get_onboardingstatus.cfg_ui_obmodel);
			}
		}
	});
}
var idx = 1;
var clickFlag = false;
var interval;
function set_slider() {
	parent.$('.amesh_dot').removeClass("click");
	var left_pos = ["", "0px", "-570px", "-1140px"];
	if(idx == 3) {
		parent.$('#amesh_scenario_slider').animate({left: left_pos[idx]}, 1000);
		parent.$('#amesh_dot_' + idx + '').addClass("click");
		idx = 1;
	}
	else {
		parent.$('#amesh_scenario_slider').animate({left: left_pos[idx]}, 1000);
		parent.$('#amesh_dot_' + idx + '').addClass("click");
		idx ++;
	}
}
function set_slider_step(_idx) {
	idx = _idx;
	parent.$('.amesh_dot').removeClass("click");
	var left_pos = ["", "0px", "-570px", "-1140px"];
	parent.$('#amesh_scenario_slider').animate({left: left_pos[idx]}, 100);
	parent.$('#amesh_dot_' + idx + '').addClass("click");
	clearInterval(interval);
	interval = setInterval(set_slider, 15000);
}
function scenario() {
	idx = 2;

	initial_amesh_obj();

	var $scenarioHtml = $('<div>');
	$scenarioHtml.attr({"id" : "amesh_scenario"});
	$scenarioHtml.addClass("amesh_popup_bg amesh_scenario");
	$scenarioHtml.css("display", "none");
	$scenarioHtml.attr({"onselectstart" : "return false"});
	$scenarioHtml.appendTo(parent.$('body'));

	var $closeHtml = $('<div>');
	$closeHtml.addClass("amesh_popup_close");
	$closeHtml.click(
		function() {
			initial_amesh_obj();
			clearInterval(interval);
		}
	);
	$scenarioHtml.append($closeHtml);

	var $clearHtml = $('<div>');
	$clearHtml.css("clear", "both");
	$scenarioHtml.append($clearHtml);

	var $amesh_scenario_slider_bg = $('<div>');
	$amesh_scenario_slider_bg.addClass("amesh_scenario_slider_bg");
	$scenarioHtml.append($amesh_scenario_slider_bg);

	var $amesh_scenario_slider = $('<div>');
	$amesh_scenario_slider.attr({"id" : "amesh_scenario_slider"});
	$amesh_scenario_slider.addClass("amesh_scenario_slider");
	$amesh_scenario_slider_bg.append($amesh_scenario_slider);
	
	var gen_each_step_content = function(_description, _idx) {
		var $amesh_scenario_step_bg = $('<div>');
		$amesh_scenario_step_bg.addClass("amesh_scenario_bg");
		$amesh_scenario_slider.append($amesh_scenario_step_bg);

		var $amesh_left_bg = $('<div>');
		$amesh_left_bg.addClass("scenario_left_bg");
		$amesh_scenario_step_bg.append($amesh_left_bg);

		var $amesh_icon = $('<div>');
		$amesh_icon.addClass("description_icon scenario");
		$amesh_left_bg.append($amesh_icon);

		var $amesh_description = $('<div>');
		$amesh_description.html(_description);
		$amesh_left_bg.append($amesh_description);

		var $amesh_right_bg = $('<div>');
		$amesh_right_bg.addClass("scenario_left_bg amesh_scenario_img step" + _idx + "");
		$amesh_scenario_step_bg.append($amesh_right_bg);
	};

	var description = "";
	description = "<#AiMesh_Desc#>";
	gen_each_step_content(description, 1);
	description = "<#AiMesh_Desc2#>";
	gen_each_step_content(description, 2);
	description = "<#AiMesh_Desc3#>";
	description += "<br>";
	description += "<#AiMesh_Desc31#>";
	description += "<br>";
	description += "<#AiMesh_Desc32#>";
	description += "<br>";
	description += "<a id='aimesh_link' style='font-weight:bolder;text-decoration:underline;color:#FC0;' href='' target='_blank'><#AiMesh_Desc3_note#></a>";
	gen_each_step_content(description, 3);

	interval = setInterval(set_slider, 15000);

	var $amesh_dot_bg = $('<div>');
	$amesh_dot_bg.addClass("amesh_dot_bg");
	$scenarioHtml.append($amesh_dot_bg);

	var gen_each_step_dot = function(_idx) {
		var default_dot = 1;
		var $amesh_dot = $('<div>');
		$amesh_dot.addClass("amesh_dot");
		if(default_dot == _idx)
			$amesh_dot.addClass("click");
		$amesh_dot.attr({"id" : "amesh_dot_" + _idx + ""});
		$amesh_dot_bg.append($amesh_dot);
		$amesh_dot.html(_idx);
		$amesh_dot.click(
			function() {
				set_slider_step(_idx);
			}
		);
	};
	var dot_num = 3;
	for(var i = 1; i <= dot_num; i += 1)
		gen_each_step_dot(i);

	parent.$("#amesh_scenario").fadeIn(300);
	parent.cal_panel_block("amesh_scenario", 0.2);
	parent.adjust_panel_block_top("amesh_scenario", 170);
	parent.$("#aimesh_link").attr({"href": aimesh_href});
}
function show_connect_msg(_reMac, _newReMac, _model_name, _ui_model_name, _rssi, _ob_path) {
	aimesh_select_new_re_mac = _newReMac;
	$.ajax({
		url: '/ajax_onboarding.asp',
		dataType: 'script',
		error: function(xhr) {
			setTimeout(function(){
				show_connect_msg(_reMac, _newReMac, _model_name, _ui_model_name, _rssi, _ob_path);
			}, 3000);
		},
		success: function() {
			if(_newReMac == "")
				return;

			initial_amesh_obj();

			if(get_onboardingstatus.cfg_obstatus == "4" && (get_onboardingstatus.cfg_obresult == "1" || get_onboardingstatus.cfg_obresult == "3") && onboarding_flag) {
				var $connectHtml = $('<div>');
				$connectHtml.attr({"id" : "amesh_connect_msg"});
				$connectHtml.addClass("amesh_popup_bg");
				$connectHtml.css("display", "none");
				$connectHtml.attr({"onselectstart" : "return false"});
				$connectHtml.appendTo(parent.$('body'));

				var result_text = "<#AiMesh_Pairing#>";
				
				var $amesh_hint_text = $('<div>');
				$amesh_hint_text.addClass("amesh_hint_text");
				$amesh_hint_text.css("text-align", "center");
				$amesh_hint_text.html(result_text);
				$connectHtml.append($amesh_hint_text);

				var $amesh_action_bg = $('<div>');
				$amesh_action_bg.addClass("amesh_action_bg");
				$connectHtml.append($amesh_action_bg);

				var $amesh_cancel = $('<input/>');
				$amesh_cancel.addClass("button_gen");
				$amesh_cancel.attr({"type" : "button", "value" : "<#CTL_ok#>"});
				$amesh_action_bg.append($amesh_cancel);
				$amesh_cancel.click(
					function() {
						initial_amesh_obj();
					}
				);

				parent.$("#amesh_connect_msg").fadeIn(300);
				parent.cal_panel_block("amesh_connect_msg", 0.2);
				parent.adjust_panel_block_top("amesh_connect_msg", 170);
			}
			else {
				var nodeNotReady = false;
				if(_reMac == "" && _model_name == "New Node" && _rssi == "-1" && Session.get("AiMesh_id") != "")
					nodeNotReady = true;

				var labelMac = _newReMac;
				httpApi.getAiMeshLabelMac(_model_name, _newReMac, 
					function(_callBackMac){
						labelMac = _callBackMac;
					}
				);
				var $connectHtml = $('<div>');
				$connectHtml.attr({"id" : "amesh_connect_msg"});
				$connectHtml.addClass("amesh_popup_bg");
				$connectHtml.addClass(_newReMac.replace(/:/g, ""));
				$connectHtml.css("display", "none");
				$connectHtml.attr({"onselectstart" : "return false"});
				$connectHtml.appendTo(parent.$('body'));

				var $clearHtml = $('<div>');
				$clearHtml.css("clear", "both");
				$connectHtml.append($clearHtml);

				var $amesh_hint_text = $('<div>');
				$amesh_hint_text.addClass("amesh_hint_text amesh_hint_title");
				var hint_text = "<#AiMesh_Node_AddConfirm#>";
				if(nodeNotReady)
					hint_text = "<#AiMesh_Node_WaitReady#>";
				$amesh_hint_text.html(hint_text);
				if(nodeNotReady)
					$amesh_hint_text.find(".amesh_device_info").html("(" + labelMac + ")");
				$connectHtml.append($amesh_hint_text);

				var $amesh_device_info = $('<div>');
				$amesh_device_info.addClass("amesh_hint_text amesh_device_info");
				var device_info = handle_ui_model_name(_model_name, _ui_model_name) + "<br>";
				device_info += labelMac;
				$amesh_device_info.html(device_info);
				if(nodeNotReady)
					$amesh_device_info.css("display", "none");
				$connectHtml.append($amesh_device_info);

				var $amesh_quality_text = $('<div>');
				$amesh_quality_text.addClass("amesh_hint_text amesh_quality_text quality_weak");
				$amesh_quality_text.html("<#AiMesh_info_weak#>");
				$connectHtml.append($amesh_quality_text);

				var $amesh_action_bg = $('<div>');
				$amesh_action_bg.addClass("amesh_action_bg");
				$connectHtml.append($amesh_action_bg);

				var $amesh_wait_search_text = $('<div>');
				$amesh_wait_search_text.addClass("amesh_hint_text amesh_quality_text wait_search");
				$amesh_wait_search_text.css("display", "none");
				$amesh_wait_search_text.html("<#AiMesh_info_waiting#>");
				$amesh_wait_search_text.find("#newReMac").html(labelMac);
				$amesh_action_bg.append($amesh_wait_search_text);

				var enable_wps_flag = false;
				var wps_enable = httpApi.nvramGet(["wps_enable"])["wps_enable"];
				if(_ob_path == "1" && wps_enable == "0"){
					if(isSupport("wps_method_ob")){
						var $amesh_wps_text = $('<div>');
						$amesh_wps_text.addClass("amesh_hint_text");
						var hint_text = "<#WiFi_temp_unavailable#>";
						$amesh_wps_text.html(hint_text);
						$connectHtml.find(".amesh_action_bg").before($amesh_wps_text);
					}
					else
						enable_wps_flag = true;
				}

				var $amesh_cancel = $('<input/>');
				$amesh_cancel.addClass("button_gen quality_ok cancel");
				$amesh_cancel.css("margin-right", "5px");
				$amesh_cancel.attr({"type" : "button", "value" : "<#CTL_Cancel#>"});
				$amesh_action_bg.append($amesh_cancel);
				$amesh_cancel.click(
					function() {
						initial_amesh_obj();
						aimesh_select_new_re_mac = "";
						document.onboardingLED_form.ob_path.disabled = true;
						document.onboardingLED_form.new_re_mac.disabled = true;
						document.onboardingLED_form.submit();
					}
				);

				var $amesh_apply = $('<input/>');
				$amesh_apply.addClass("button_gen quality_ok");
				$amesh_apply.attr({"type" : "button", "value" : "<#CTL_apply#>"});
				$amesh_action_bg.append($amesh_apply);
				$amesh_apply.click(
					function() {
						var re_isAX_model = (_model_name.toUpperCase().indexOf("AX") >= 0 || _model_name.toUpperCase().indexOf("ZENWIFI_X") >= 0 || _model_name.toUpperCase().indexOf("ZENWIFI_E") >= 0 || _model_name.toUpperCase().indexOf("GT6") >= 0);
						var auth_flag = false;
						var postData = {};
						var band6g = 4;
						var auth_list = [];
						auth_list["psk2"] = "WPA2-Personal";
						auth_list["psk2sae"] = "WPA2/WPA3-personal";
						auth_list["wpa2"] = "WPA2-Enterprise";
						auth_list["wpawpa2"] = "WPA/WPA2-Enterprise";
						auth_list["sae"] = "WPA3-Personal";

						var current_auth = "";
						var changeTo_auth = "";
						var set_replace_str = function(_str, _auth, _idx){
							var result = "";
							if(_str != "")
								result += ", ";
							result += auth_list[_auth];
							result += " (" + wl_nband_title[_idx] + ")";
							return result;
						};
						$.each(wl_nband_array, function(index, value){
							var authMode = httpApi.nvramGet(["wl" + index + "_auth_mode_x"], true)["wl" + index + "_auth_mode_x"];
							if(value == band6g)
								return true;
							//case 1, WPA2-Enterprise or WPA/WPA2-Enterprise
							if(authMode == "wpa2" || authMode == "wpawpa2"){
								postData["wl" + index + "_auth_mode_x"] = "psk2";
								current_auth += set_replace_str(current_auth, authMode, index);
								changeTo_auth += set_replace_str(changeTo_auth, "psk2", index);
								auth_flag = true;
							}
							//case 2, WPA3-personal and not AX model
							if(authMode == "sae" && !re_isAX_model){
								postData["wl" + index + "_auth_mode_x"] = "psk2sae";
								postData["wl" + index + "_mfp"] = 1;
								current_auth += set_replace_str(current_auth, authMode, index);
								changeTo_auth += set_replace_str(changeTo_auth, "psk2sae", index);
								auth_flag = true;
							}
						});

						if(enable_wps_flag){
							postData["wps_enable"] = "1";
						}

						if(Object.keys(postData).length){
							if(auth_flag){
								var $amesh_wpa3_text = $('<div>');
								$amesh_wpa3_text.addClass("amesh_hint_text");
								var auth_change_str = "* <#ADSL_FW_note#> <#AiMesh_confirm_msg11#>".replace("WPA3-personal", current_auth).replace("WPA2/WPA3-personal", changeTo_auth);
								$amesh_wpa3_text.append($("<div>").html(auth_change_str).css("color", "#FC0"));
								$amesh_wpa3_text.append($("<div>").html("<#AiMesh_confirm_msg12#>"));
								$amesh_wpa3_text.find("#wpa3FaqLink").attr("target", "_blank").css({"color": "#FC0", "text-decoration": "underline"});
								$amesh_wpa3_text.find("#wpa3FaqLink").attr("href", faq_href);
								$connectHtml.find(".amesh_action_bg").before($amesh_wpa3_text);
							}

							if(enable_wps_flag){
								var $amesh_wps_text = $('<div>');
								$amesh_wps_text.addClass("amesh_hint_text");
								var hint_text = "<#AiMesh_FindNode_confirm_WPS#>";
								hint_text += "<br>";
								hint_text += "<#WiFi_temp_unavailable#>";
								$amesh_wps_text.html(hint_text);
								$connectHtml.find(".amesh_action_bg").before($amesh_wps_text);
							}

							$amesh_apply.unbind("click");
							$amesh_apply.click(
								function() {
									initial_amesh_obj();
									postData.action_mode = "apply";
									postData.rc_service = "restart_wireless";
									httpApi.nvramSet(postData, function(){
										connectingDevice(_reMac, _newReMac, 10);
									})
								}
							);
						}
						else{
							initial_amesh_obj();
							connectingDevice(_reMac, _newReMac);
						}
					}
				);

				var $amesh_ok = $('<input/>');
				$amesh_ok.addClass("button_gen quality_weak");
				$amesh_ok.css("margin-right", "5px");
				$amesh_ok.attr({"type" : "button", "value" : "<#CTL_ok#>"});
				$amesh_action_bg.append($amesh_ok);
				$amesh_ok.click(
					function() {
						initial_amesh_obj();
						aimesh_select_new_re_mac = "";
						document.onboardingLED_form.ob_path.disabled = true;
						document.onboardingLED_form.new_re_mac.disabled = true;
						document.onboardingLED_form.submit();
					}
				);

				if((parseInt(_rssi) < parseInt(get_onboardingstatus.cfg_wifi_quality)) && (_ob_path != "2") ) {
					$connectHtml.find(".quality_ok").css("display", "none");
				}
				else {
					$connectHtml.find(".quality_weak").css("display", "none");
				}

				if(nodeNotReady) {
					$amesh_action_bg.find(".button_gen").css("display", "none");
					$amesh_action_bg.find(".button_gen.cancel").css("display", "");
					$amesh_action_bg.find(".amesh_hint_text.amesh_quality_text.wait_search").css("display", "");
				}
				else {
					document.onboardingLED_form.new_re_mac.disabled = false;
					document.onboardingLED_form.new_re_mac.value = _newReMac;
					document.onboardingLED_form.ob_path.disabled = false;
					document.onboardingLED_form.ob_path.value = _ob_path;
					document.onboardingLED_form.submit();
				}

				parent.$("#amesh_connect_msg").fadeIn(300);
				parent.cal_panel_block("amesh_connect_msg", 0.2);
				parent.adjust_panel_block_top("amesh_connect_msg", 170);
			}
		}
	});	
}
function show_connect_result(_status, _newReMac, _model_name, _ui_model_name) {
	initial_amesh_obj();

	var labelMac = _newReMac;
	httpApi.getAiMeshLabelMac(_model_name, _newReMac, 
		function(_callBackMac){
			labelMac = _callBackMac;
		}
	);

	var $connectResultHtml = $('<div>');
	$connectResultHtml.attr({"id" : "amesh_connect_result"});
	$connectResultHtml.addClass("amesh_popup_bg");
	$connectResultHtml.css("display", "none");
	$connectResultHtml.attr({"onselectstart" : "return false"});
	$connectResultHtml.appendTo(parent.$('body'));

	if(_status == "2") {
		var result_text = "";
		var $successResult1 = $('<div>');
		$successResult1.addClass("amesh_successResult");
		$successResult1.attr({"id" : "amesh_successResult_1"});
		$connectResultHtml.append($successResult1);
		result_text = "";
		result_text += "<#AiMesh_Node_AddDescA#>";
		result_text += "<br>";
		result_text += "<#AiMesh_Node_AddDescB#>";
		result_text += "<br>";
		result_text += "1. <#AiMesh_Node_AddDesc1#>";
		result_text += "<br>";
		result_text += "2. <#AiMesh_Node_AddDesc2#>";
		$successResult1.addClass("amesh_hint_text");
		$successResult1.html(result_text);
		$successResult1.find(".amesh_device_info").html(handle_ui_model_name(_model_name, _ui_model_name) + " (" + labelMac + ")");

		var $successResult2 = $('<div>');
		$successResult2.addClass("amesh_successResult");
		$successResult2.attr({"id" : "amesh_successResult_2"});
		$connectResultHtml.append($successResult2);

		var $successResult2_text = $('<div>');
		result_text = "";
		result_text = "<#AiMesh_Node_AddDiagram#>";
		$successResult2_text.addClass("amesh_hint_text");
		$successResult2_text.html(result_text);
		$successResult2.append($successResult2_text);

		var $aimesh_illustration_bg = $('<div>');
		$aimesh_illustration_bg.addClass("aimesh_illustration_bg");
		$successResult2.append($aimesh_illustration_bg);

		var gen_icon = function(_class) {
			var $iconBg = $('<div>');
			$iconBg.addClass("aimesh_illustration_icon");
			$iconBg.addClass(_class);
			return $iconBg;
		};
		var gen_text = function(_text, _class) {
			var $textBg = $('<div>');
			$textBg.addClass("aimesh_illustration_text");
			$textBg.addClass(_class);
			$textBg.html(_text);
			return $textBg;
		};

		if(!dsl_support){
			$aimesh_illustration_bg.append(gen_icon("modem"));
			$aimesh_illustration_bg.append(gen_icon("ethernet"));
			$aimesh_illustration_bg.append(gen_icon("router_back"));
			$aimesh_illustration_bg.append(gen_icon("router_back_right"));
			$aimesh_illustration_bg.append(gen_icon("ethernet_lan"));
			$aimesh_illustration_bg.append(gen_icon("ethernet_wan"));
			$aimesh_illustration_bg.append(gen_text("LAN - WAN", "lan_to_wan"));
			$aimesh_illustration_bg.append(gen_text("Modem", "modem"));
			$aimesh_illustration_bg.append(gen_text("<#AiMesh_Router#>", "backhaul_router"));
			$aimesh_illustration_bg.append(gen_text("<#AiMesh_Node#>", "backhaul_node"));
		}
		else{
			$aimesh_illustration_bg.append(gen_icon("router_back for_dsl"));
			$aimesh_illustration_bg.append(gen_icon("router_back_right for_dsl"));
			$aimesh_illustration_bg.append(gen_icon("ethernet_lan for_dsl"));
			$aimesh_illustration_bg.append(gen_icon("ethernet_wan for_dsl"));
			$aimesh_illustration_bg.append(gen_text("LAN - WAN", "lan_to_wan for_dsl"));
			$aimesh_illustration_bg.append(gen_text("AiMesh xDSL modem router", "backhaul_router for_dsl"));	/*Untranslated*/
			$aimesh_illustration_bg.append(gen_text("<#AiMesh_Node#>", "backhaul_node for_dsl"));
		}
		
		var $amesh_clear_bg = $('<div>');
		$amesh_clear_bg.css("clear", "both");
		$aimesh_illustration_bg.append($amesh_clear_bg);
	}
	else {
		//var fail_status = ["Init", "Start", "Success", "WPS Success", "WPS Fail", "Terminate"];
		var result_text = "";
		result_text += "<#AiMesh_info_unabled#>";
		result_text += "<ol style='margin:0;padding-left:17px;'>";
		result_text += "<li><#AiMesh_info_unabled1#></li>";
		result_text += "<li><#AiMesh_info_unabled2#></li>";
		result_text += "<li><#AiMesh_OfflineTips1#></li>";
		result_text += "<li><#AiMesh_info_unabled4#></li>";
		result_text += "<li><#AiMesh_FindNode_Not_advA3#></li>";
		result_text += "</ol>";
		//result_text += "<br><br>Status: " + fail_status[parseInt(_status)];
		var $amesh_hint_text = $('<div>');
		$amesh_hint_text.addClass("amesh_hint_text");
		$amesh_hint_text.html(result_text);
		$amesh_hint_text.find(".amesh_device_info").html(handle_ui_model_name(_model_name, _ui_model_name) + " (" + labelMac + ")");
		$connectResultHtml.append($amesh_hint_text);
	}
	
	var $amesh_action_bg = $('<div>');
	$amesh_action_bg.addClass("amesh_action_bg");
	$connectResultHtml.append($amesh_action_bg);

	var $amesh_cancel = $('<input/>');
	$amesh_cancel.addClass("button_gen");
	$amesh_cancel.attr({"type" : "button", "value" : "<#CTL_ok#>"});
	$amesh_action_bg.append($amesh_cancel);
	$amesh_cancel.click(
		function() {
			initial_amesh_obj();
		}
	);

	parent.$("#amesh_connect_result").fadeIn(300);
	parent.cal_panel_block("amesh_connect_result", 0.2);
	parent.adjust_panel_block_top("amesh_connect_result", 170);
	$('#ready_onBoarding_block').empty();
	$('#ready_onBoarding_table').css("display", "none");
	$("#searchReadyOnBoarding").css("display", "");
	$("#amesh_loadingIcon").css("display", "none");
	onboarding_flag = false;
}
function show_search_fail_result() {
	initial_amesh_obj();

	var $searchFailHtml = $('<div>');
	$searchFailHtml.attr({"id" : "amesh_search_fail_result"});
	$searchFailHtml.addClass("amesh_popup_bg");
	$searchFailHtml.css("display", "none");
	$searchFailHtml.attr({"onselectstart" : "return false"});
	$searchFailHtml.appendTo(parent.$('body'));

	var result_text = "";
	result_text = "<#AiMesh_FindNode_Not#>";
	result_text += "<br>";
	if(ameshRouter_support && ameshNode_support)
		result_text += "a. ";
	if(ameshRouter_support) {
		result_text += "<#AiMesh_FindNode_Not_advA#>";
		result_text += "<div style='margin-left: 20px;'>1. <#AiMesh_FindNode_Not_advA1#></div>";
		result_text += "<div style='margin-left: 20px;'>2. <#AiMesh_FindNode_Not_advA2#></div>";
		result_text += "<div style='margin-left: 20px;'>3. <#AiMesh_FindNode_Not_advA3#></div>";
		result_text += "<div style='margin-left: 20px;'>4. <#AiMesh_FindNode_Not_advA4#></div>";
	}
	if(ameshRouter_support && ameshNode_support)
		result_text += "b. ";
	if(ameshNode_support){
		result_text += "<#AiMesh_FindNode_Not_advB#>";
		result_text += "<div style='margin-left: 20px;'>1. <#AiMesh_FindNode_Not_advB1#></div>";
		result_text += "<div style='margin-left: 20px;'>2. <#AiMesh_FindNode_Not_advB2#></div>";
		result_text += "<div style='margin-left: 20px;'>3. <#AiMesh_FindNode_Not_advB3#></div>";
	}

	var $amesh_hint_text = $('<div>');
	$amesh_hint_text.addClass("amesh_hint_text");
	$amesh_hint_text.html(result_text);
	$searchFailHtml.append($amesh_hint_text);

	var $aimesh_illustration_bg = $('<div>');
	$aimesh_illustration_bg.addClass("aimesh_illustration_bg");
	$searchFailHtml.append($aimesh_illustration_bg);

	var gen_icon = function(_class) {
			var $iconBg = $('<div>');
			$iconBg.addClass("aimesh_illustration_icon");
			$iconBg.addClass(_class);
			return $iconBg;
		};
	var gen_text = function(_text, _class) {
		var $textBg = $('<div>');
		$textBg.addClass("aimesh_illustration_text");
		$textBg.addClass(_class);
		$textBg.html(_text);
		return $textBg;
	};

	if(!dsl_support){
		$aimesh_illustration_bg.append(gen_icon("modem"));
		$aimesh_illustration_bg.append(gen_icon("ethernet"));
		$aimesh_illustration_bg.append(gen_icon("router"));
		$aimesh_illustration_bg.append(gen_icon("router_right"));
		$aimesh_illustration_bg.append(gen_icon("wifi"));
		$aimesh_illustration_bg.append(gen_text("Modem", "modem"));
		$aimesh_illustration_bg.append(gen_text("<#AiMesh_Router#>", "router"));
		$aimesh_illustration_bg.append(gen_text("<#AiMesh_Node#>", "node"));
	}
	else{
		$aimesh_illustration_bg.append(gen_icon("router for_dsl"));
		$aimesh_illustration_bg.append(gen_icon("router_right for_dsl"));
		$aimesh_illustration_bg.append(gen_icon("wifi for_dsl"));
		$aimesh_illustration_bg.append(gen_text("AiMesh xDSL modem router", "router for_dsl"));    /*Untranslated*/	
		$aimesh_illustration_bg.append(gen_text("<#AiMesh_Node#>", "node for_dsl"));
	}
	
	var $amesh_clear_bg = $('<div>');
	$amesh_clear_bg.css("clear", "both");
	$aimesh_illustration_bg.append($amesh_clear_bg);

	var $amesh_action_bg = $('<div>');
	$amesh_action_bg.addClass("amesh_action_bg");
	$searchFailHtml.append($amesh_action_bg);

	var $amesh_cancel = $('<input/>');
	$amesh_cancel.addClass("button_gen");
	$amesh_cancel.attr({"type" : "button", "value" : "<#CTL_ok#>"});
	$amesh_action_bg.append($amesh_cancel);
	$amesh_cancel.click(
		function() {
			initial_amesh_obj();
		}
	);

	parent.$("#amesh_search_fail_result").fadeIn(300);
	parent.cal_panel_block("amesh_search_fail_result", 0.2);
	parent.adjust_panel_block_top("amesh_search_fail_result", 170);
}
function download_cloud_icon(model_info, device_id, parent_bg_id) {
	var cloudModelName = "";
	if(model_info.cloudModelName != undefined && model_info.cloudModelName != "")
		cloudModelName = model_info.cloudModelName;
	else
		cloudModelName = httpApi.transformCloudModelName(model_info);

	var set_default_router_icon = function(_parent_bg_id, _device_id) {
		if($('#' + _parent_bg_id + '').find('#' + _device_id + '').length != 0) {
			$ ('#' + _parent_bg_id + '').find('#' + _device_id + '').children().find('.amesh_each_router_icon_bg').addClass('amesh_router_icon');
		}
	};
	var set_cloud_router_icon = function(_parent_bg_id, _device_id) {
		if($('#' + _parent_bg_id + '').find('#' + _device_id + '').length != 0) {
			$('#' + _parent_bg_id + '').find('#' + _device_id + '').children().find('.amesh_each_router_icon_bg').css('background-image', 'url(' + checkCloudIconExist[cloudModelName] + ')');
			$('#' + _parent_bg_id + '').find('#' + _device_id + '').children().find('.amesh_each_router_icon_bg').addClass('amesh_router_image_web');
			$('#' + _parent_bg_id + '').find('#' + _device_id + '').children().find('.amesh_each_router_icon_bg').removeClass('amesh_router_icon');
		}
	};

	if('<% nvram_get("x_Setting"); %>' == '1' && parent.wanConnectStatus && checkCloudIconErrorTimes[cloudModelName] < 5 && !checkCloudIconExist[cloudModelName]) {
		httpApi.checkCloudModelIcon(
			model_info,
			function(src){
				checkCloudIconExist[cloudModelName] = src;
				set_cloud_router_icon(parent_bg_id, device_id);
				checkCloudIconErrorTimes[cloudModelName] = 0;
			},
			function(){
				checkCloudIconExist[cloudModelName] = false;
				set_default_router_icon(parent_bg_id, device_id);
				checkCloudIconErrorTimes[cloudModelName] = parseInt(checkCloudIconErrorTimes[cloudModelName]) + 1;
			}
		);
	}
	else {
		if(checkCloudIconExist[cloudModelName])
			set_cloud_router_icon(parent_bg_id, device_id);
		else
			set_default_router_icon(parent_bg_id, device_id);
	}
}

function reset_re_device(_reMac, _reModelName, _reUiModelName, _evt, _online) {
	_evt.stopPropagation();
	initial_amesh_obj();

	var $resetHtml = $('<div>');
	$resetHtml.attr({"id" : "amesh_reset_msg"});
	$resetHtml.addClass("amesh_popup_bg");
	$resetHtml.css("display", "none");
	$resetHtml.attr({"onselectstart" : "return false"});
	$resetHtml.appendTo(parent.$('body'));

	var $amesh_hint_text = $('<div>');
	$amesh_hint_text.addClass("amesh_hint_text");
	$amesh_hint_text.html("<#AiMesh_Node_RemoveConfirm#>");
	$resetHtml.append($amesh_hint_text);

	var $amesh_device_info = $('<div>');
	$amesh_device_info.addClass("amesh_hint_text amesh_device_info");
	var device_info = handle_ui_model_name(_reModelName, _reUiModelName) + "<br>";
	var labelMac = _reMac;
	httpApi.getAiMeshLabelMac(_reModelName, _reMac, 
		function(_callBackMac){
			labelMac = _callBackMac;
		}
	);
	device_info += labelMac;
	$amesh_device_info.html(device_info);
	$resetHtml.append($amesh_device_info);

	var $amesh_hint_text = $('<div>');
	$amesh_hint_text.addClass("amesh_hint_text");
	$amesh_hint_text.html("<#AiMesh_Node_RemoveDesc#>");
	$resetHtml.append($amesh_hint_text);

	if(_online == "0"){
		var $amesh_hint_text = $('<div>');
		$amesh_hint_text.addClass("amesh_hint_text amesh_quality_text");
		$amesh_hint_text.html("* <#AiMesh_Node_RemoveDesc01#>");
		$resetHtml.append($amesh_hint_text);
	}

	var $amesh_action_bg = $('<div>');
	$amesh_action_bg.addClass("amesh_action_bg");
	$resetHtml.append($amesh_action_bg);

	var $amesh_cancel = $('<input/>');
	$amesh_cancel.addClass("button_gen");
	$amesh_cancel.css("margin-right", "5px");
	$amesh_cancel.attr({"type" : "button", "value" : "<#CTL_Cancel#>"});
	$amesh_action_bg.append($amesh_cancel);
	$amesh_cancel.click(
		function() {
			initial_amesh_obj();
		}
	);

	var $amesh_apply = $('<input/>');
	$amesh_apply.addClass("button_gen");
	$amesh_apply.attr({"type" : "button", "value" : "<#CTL_ok#>"});
	$amesh_action_bg.append($amesh_apply);
	$amesh_apply.click(
		function() {
			initial_amesh_obj();
			document.reset_form.slave_mac.value = _reMac.toUpperCase();
			document.reset_form.submit();
			ajax_onboarding()
		}
	);

	parent.$("#amesh_reset_msg").fadeIn(300);
	parent.cal_panel_block("amesh_reset_msg", 0.2);
	parent.adjust_panel_block_top("amesh_reset_msg", 170);
}
function searchReadyOnBoarding() {
	var get_onboardingstatus = httpApi.hookGet("get_onboardingstatus", true);
	var cfg_re_maxnum = parseInt(get_onboardingstatus.cfg_re_maxnum);
	var cfg_recount = parseInt(get_onboardingstatus.cfg_recount);
	if(cfg_recount >= cfg_re_maxnum){
		initial_amesh_obj();
		var $maxnumHtml = $('<div>');
		$maxnumHtml.attr({"id" : "amesh_maxnum_msg"});
		$maxnumHtml.addClass("amesh_popup_bg");
		$maxnumHtml.css("display", "none");
		$maxnumHtml.attr({"onselectstart" : "return false"});
		$maxnumHtml.appendTo(parent.$('body'));

		var $amesh_hint_text = $('<div>');
		$amesh_hint_text.addClass("amesh_hint_text");
		var hint = "";
		hint += "<#AiMesh_Unable_Add_Nodes#>";
		hint += "<br>";
		hint += "<#AiMesh_Limit_RE_Maximum#> <#AiMesh_Reached_Maximum#>".replace("#RE_MAXNUM", cfg_re_maxnum);
		hint += "<br>";
		hint += "<#AiMesh_Want_Add_Node_Hint#>";
		$amesh_hint_text.html(hint);
		$maxnumHtml.append($amesh_hint_text);

		var $amesh_action_bg = $('<div>');
		$amesh_action_bg.addClass("amesh_action_bg");
		$maxnumHtml.append($amesh_action_bg);

		var $amesh_ok = $('<input/>');
		$amesh_ok.addClass("button_gen");
		$amesh_ok.attr({"type" : "button", "value" : "<#CTL_ok#>"});
		$amesh_action_bg.append($amesh_ok);
		$amesh_ok.click(
			function() {
				initial_amesh_obj();
			}
		);

		parent.$("#amesh_maxnum_msg").fadeIn(300);
		parent.cal_panel_block("amesh_maxnum_msg", 0.2);
		parent.adjust_panel_block_top("amesh_maxnum_msg", 170);
	}
	else{
		$.ajax({
			url: '/ajax_onboarding.asp',
			dataType: 'script',
			error: function(xhr) {
				setTimeout(function(){
					searchReadyOnBoarding();
				}, 3000);
			},
			success: function() {
				if(get_onboardingstatus.cfg_obstatus == "") {
					$("#searchReadyOnBoarding").css("display", "none");
					$("#amesh_loadingIcon").css("display", "");
					setTimeout(function(){
						searchReadyOnBoarding();
					}, 3000);
				}
				else {
					if(get_onboardingstatus.cfg_obstatus == "1") {
						if(interval_ajax_onboarding_status) {
							clearInterval(interval_ajax_onboarding_status);
							interval_ajax_onboarding_status = false;
						}
						$("#searchReadyOnBoarding").css("display", "none");
						$("#amesh_loadingIcon").css("display", "");
						document.form.re_mac.disabled = true;
						document.form.new_re_mac.disabled = true;
						document.form.submit();
						interval_ajax_onboarding_status = setInterval(ajax_onboarding, 5000);
					}
				}
			}
		});
	}
}

function open_AiMesh_node_usb_app(_node_info) {
	var url = httpApi.aimesh_get_win_open_url(_node_info, "APP_Installation.asp");
	var window_width = 780;
	var window_height = 650;
	var window_top = screen.availHeight / 2 - window_height / 2;
	var window_left = screen.availWidth / 2 - window_width / 2;
	window.open(url, '_new' ,'width=' + window_width + ',height=' + window_height + ', top=' + window_top + ',left=' + window_left + ',menubar=no,scrollbars=yes,toolbar=no,resizable=no,status=no,location=no');
}

var aimesh_node_hide_flag = false;
function popAMeshClientListEditTable(event) {
	aimesh_node_hide_flag = false;
	var node_info = event.data.node_info;
	var wl_client = event.data.wl_client;
	var wired_client = event.data.wired_client;
	var re_path = handle_re_path(node_info.re_path);
	var cobrand = httpApi.aimesh_get_misc_info(event.data.node_info).cobrand;
	var node_capability = httpApi.aimesh_get_node_capability(node_info);
	initial_amesh_obj();
	var model_info = {"model_name": node_info.model_name, "tcode": node_info.tcode, "cobrand": cobrand, "icon_model_name": node_info.icon_model_name};
	var cloudModelName = httpApi.transformCloudModelName(model_info);
	var $popupBgHtml = $('<div>');
	$popupBgHtml.attr({"id" : "edit_amesh_client_block"});
	$popupBgHtml.addClass("amesh_popup_bg");
	$popupBgHtml.css("display", "none");
	$popupBgHtml.attr({"onselectstart" : "return false"});
	$popupBgHtml.appendTo(parent.$('body'));

	var code = "";
	code += "<div id='aimesh_node_close' class='aimesh_node_close_icon'></div>";
	code += "<div id='aimesh_node_title_name' class='aimesh_node_title_name'></div>";

	code += "<div class='aimesh_node_info_bg'>";
	code += "<div style='float:left;width:30%;height:92px;position:relative;'>";
	code += "<div class='amesh_router_icon_bg card'>";
	if(checkCloudIconExist[cloudModelName])
		code += "<div class='amesh_router_image_web card'></div>";
	else
		code += "<div class='amesh_router_icon card'></div>";
	code += "</div>";
	code += "</div>";
	code += "<div style='float:left;width:70%;margin-top:17px;'>";

	code += "<div class='aimesh_node_static_info_title'><#AiMesh_NodeLocation#></div>";
	code += "<div class='aimesh_node_static_info_content' style='width:55%;'>";
	code += "<div class='aimesh_node_select_text_combine'>";
	code += "<select id='aimesh_node_location_select' class='aimesh_node_select_text_combine_select'></select>";
	code += "<input id='aimesh_node_location_input' name='aimesh_node_location_input' class='aimesh_node_select_text_combine_text' type='text' value='' maxlength='32' autocorrect='off' autocapitalize='off'>";
	code += "</div>";
	code += "</div>";
	code += "<div class='aimesh_node_static_info_interface_bg'>";
	code += "<div class='amesh_interface_icon static_info'></div>";
	code += "</div>";
	code += "<div id='aimesh_node_location_hint' class='aimesh_node_location_hint'></div>";
	code += "<div class='clear_both'></div>";
	code += "<div class='aimesh_node_static_info_title'><#IPConnection_ExternalIPAddress_itemname#></div>";
	code += "<div id='aimesh_node_ipaddr' class='aimesh_node_static_info_content'></div>";
	code += "<div class='aimesh_node_static_info_title moreconfig'><span id='aimesh_node_moreconfig' class='text_hyperlink'></span></div>";
	code += "</div>";
	code += "<div class='clear_both'></div>";
	code += "</div>";

	code += "<div class='clientList_line aimesh_node_line_bg aimesh_node_setting_line'></div>";
	code += "<div class='aimesh_node_setting_bg'>";
	code += "<div>";
	code += "<div class='aimesh_node_setting_info_title'><#MAC_Address#></div>";
	code += "<div id='aimesh_node_macaddr' class='aimesh_node_setting_info_content'></div>";
	code += "<div class='clear_both'></div>";
	code += "</div>";

	code += "<div style='margin-top:10px;'>";
	code += "<div class='aimesh_node_setting_info_title'><#AiMesh_BackhaulConnPrio#></div>";
	code += "<select id='aimesh_node_connection_priority' class='aimesh_node_input_select'>";
	code += "<option value='3' class='aimesh_node_input_select_option'><#Auto#></option>";
	code += "<option value='2' class='aimesh_node_input_select_option'><#AiMesh_Node_ConnPrio_Eth_First_Title#></option>";
	code += "</select>";
	code += "<div id='aimesh_node_connection_priority_hint' class='aimesh_node_setting_info_content'><#CTL_nonsupported#></div>";
	code += "<div class='clear_both'></div>";
	code += "</div>";

	code += "<div style='margin-top:10px;'>";
	code += "<div class='aimesh_node_setting_info_title'><#Connectiontype#></div>";
	code += "<div class='aimesh_node_setting_info_content'><span id='aimesh_node_connection_type' class='text_hyperlink'></span></div>";
	code += "<div class='clear_both'></div>";
	code += "</div>";

	code += "<div style='margin-top:10px;'>";
	code += "<div class='aimesh_node_setting_info_title'><#FW_item2#></div>";
	code += "<div class='aimesh_node_setting_info_content'><span id='aimesh_node_fw_version' class='text_hyperlink'></span></div>";
	code += "<div class='clear_both'></div>";
	code += "</div>";

	code += "<div id='aimesh_node_usb_app_bg' style='margin-top:10px;'>";
	code += "<div class='aimesh_node_setting_info_title'><#menu5#></div>";
	code += "<div class='aimesh_node_setting_info_content'><span id='aimesh_node_usb_app' class='text_hyperlink'><#Menu_usb_application#></span></div>";
	code += "<div class='clear_both'></div>";
	code += "</div>";

	var led_status = led_control.status(node_info);
	if(led_status.support)
		code += led_control.component(led_status.support);

	code += "</div>";

	code += "<div class='clientList_line aimesh_node_line_bg'></div>";

	code += "<div class='aimesh_node_info_bg'>";
	code += "<table id='aimesh_node_client_tb' width='100%' border='1' align='center' cellpadding='0' cellspacing='0' class='FormTable_table'>";
	code += "<tr id='aimesh_node_client_header_tr' height='30px'>";
	code += "<th class='IE8HACK' width=" + aimesh_node_client_info_width[0] + "><#Client_Icon#></th>";
	code += "<th class='aimesh_node_client_item' width=" + aimesh_node_client_info_width[1] + " data-clickID='client_name'><#ParentalCtrl_username#></th>";
	code += "<th class='aimesh_node_client_item' width=" + aimesh_node_client_info_width[2] + " data-clickID='client_ip'><#vpn_client_ip#></th>";
	code += "<th class='aimesh_node_client_item' width=" + aimesh_node_client_info_width[3] + " data-clickID='client_interface'><#wan_interface#></th>";
	code += "</tr>";
	code += "</table>";
	code += "</div>";

	$popupBgHtml.html(code);

	/* handle node client start */
	aimesh_node_client_list = [];
	if(node_info.online == "1" && re_path > 0)
		aimesh_node_client_list = get_aimesh_node_client_list(wired_client, wl_client, node_info.mac);
	//set default sort property
	sorterApi.method = "increase";
	sorterApi.key = "name";
	sorterApi.type = "str";
	sorterApi.clickID = "client_name";
	aimesh_node_client_list = sorterApi.sortJson(aimesh_node_client_list, sorterApi.key, sorterApi.type);
	$popupBgHtml.find(".aimesh_node_info_bg").children("#aimesh_node_client_tb").find("#aimesh_node_client_header_tr").after(gen_AiMesh_node_client(aimesh_node_client_list));
	sorterApi.drawBorder($popupBgHtml.find(".aimesh_node_info_bg #aimesh_node_client_tb .aimesh_node_client_item"));
	/* handle node client end */

	/* settup value start */
	var alias = "Home";
	if("config" in node_info) {
		if("misc" in node_info.config) {
			if("cfg_alias" in node_info.config.misc) {
				if(node_info.config.misc.cfg_alias != "")
					alias = node_info.config.misc.cfg_alias;
			}
		}
	}
	var specific_location = aimesh_location_arr.filter(function(item, index, _array){
		return (item.value == alias);
	})[0];
	var location_text = "<#AiMesh_NodeLocation01#>";
	if(specific_location != undefined)
		location_text = specific_location.text;
	else
		location_text = alias;

	var title_name = handle_ui_model_name(node_info.model_name, node_info.ui_model_name) + " in " +  location_text;
	$popupBgHtml.find("#aimesh_node_title_name").html(htmlEnDeCode.htmlEncode(title_name));

	if(checkCloudIconExist[cloudModelName])
		$popupBgHtml.find(".amesh_router_image_web").css('background-image', 'url(' + checkCloudIconExist[cloudModelName] + ')');

	$popupBgHtml.find("#aimesh_node_ipaddr").html(node_info.ip);
	var labelMac = node_info.mac;
		httpApi.getAiMeshLabelMac(node_info.model_name, node_info.mac, 
			function(_callBackMac){
			labelMac = _callBackMac;
		}
	);
	$popupBgHtml.find("#aimesh_node_macaddr").html(labelMac);

	for(var i = 0; i < aimesh_location_arr.length; i += 1) {
		$popupBgHtml.find("#aimesh_node_location_select").append($('<option>', {
			value: aimesh_location_arr[i].value,
			text: aimesh_location_arr[i].text,
			class: "aimesh_node_input_select_option"
		}));
	}

	$popupBgHtml.find("#aimesh_node_location_input").val(location_text);

	if(specific_location != undefined){
		$popupBgHtml.find("#aimesh_node_location_select").val(specific_location.value);
		$popupBgHtml.find("#aimesh_node_location_input").attr("disabled", true);
		if(specific_location.value == "Custom")
			$popupBgHtml.find("#aimesh_node_location_input").attr("disabled", false);
	}
	else{
		$popupBgHtml.find("#aimesh_node_location_select").val("Custom");
		$popupBgHtml.find("#aimesh_node_location_input").attr("disabled", false);
	}

	$popupBgHtml.find("#aimesh_node_connection_type").html(get_connect_type(node_info).text);
	$popupBgHtml.find(".amesh_interface_icon.static_info").html(get_connect_type(node_info).icon);

	var backhalctrl_amas_ethernet = "3";
	var $conn_priority_select_obj = $popupBgHtml.find("#aimesh_node_connection_priority");
	$conn_priority_select_obj.attr("disabled", true);
	$conn_priority_select_obj.css("display", "none");

	if("config" in node_info) {
		if("backhalctrl" in node_info.config) {
			if("amas_ethernet" in node_info.config.backhalctrl) {
				backhalctrl_amas_ethernet = node_info.config.backhalctrl.amas_ethernet;
				$conn_priority_select_obj.attr("disabled", false);
				$conn_priority_select_obj.css("display", "");
				$popupBgHtml.find("#aimesh_node_connection_priority_hint").css("display", "none");
				$conn_priority_select_obj.children().remove().end();
				var eap_flag = false;
				var support_eth_num = 0;
				if(isSupport("amas_eap")){
					var amas_eap_bhmode = httpApi.nvramGet(["amas_eap_bhmode"]).amas_eap_bhmode;
					if(amas_eap_bhmode != "0" && amas_eap_bhmode != "")
						eap_flag = true;
				}

				var option_array = gen_conn_priority_select_option(node_info, eap_flag);
				option_array.forEach(function(item, index, array) {
					$conn_priority_select_obj.append($("<option/>").attr({"value": item.value, "conn_type": item.conn_type}).text(item.text));
					if(item.conn_type != "auto" && item.conn_type != "wifi")
						support_eth_num++;
				});

				if($conn_priority_select_obj.children("option[value="+backhalctrl_amas_ethernet+"]").length > 0)
					$conn_priority_select_obj.val(backhalctrl_amas_ethernet);
				else if(backhalctrl_amas_ethernet != "3"){
					if($conn_priority_select_obj.children("option[value=41]").length > 0)
						$conn_priority_select_obj.val("41");
				}

				if($conn_priority_select_obj.val() == null)//avoiding some value do not exist at select option
					$conn_priority_select_obj.val(3);


				if(!node_capability.wans_cap_wan){
					$conn_priority_select_obj.children("option[conn_type='eth']").remove();
					$conn_priority_select_obj.val(3);
				}

				if(eap_flag) {
					if(support_eth_num <= "1") {//only one eth
						$conn_priority_select_obj.attr("disabled", true);
						$conn_priority_select_obj.children("option").removeAttr("selected").filter("[conn_type=eth]:first").prop("selected", true);
					}
					else
						$conn_priority_select_obj.attr("disabled", false);
				}
			}
		}
	}

	$popupBgHtml.find("#aimesh_node_fw_version").html(node_info.fwver.split("-")[0]);
	$popupBgHtml.find("#aimesh_node_moreconfig").html("<#MoreConfig#>");

	if(!node_capability.usb)
		$popupBgHtml.find("#aimesh_node_usb_app_bg").remove();
	/* settup value end */

	/* set event start */
	$popupBgHtml.find("#aimesh_node_close").click(
		function() {
			initial_amesh_obj();
		}
	);
	$popupBgHtml.find("#aimesh_node_location_select").change(
		function() {
			var location_value = $(this).val();
			if(location_value != "Custom"){
				$popupBgHtml.find('#aimesh_node_location_input').attr("disabled", true);
				var specific_location = aimesh_location_arr.filter(function(item, index, _array){
					return (item.value == location_value);
				})[0];
				$popupBgHtml.find('#aimesh_node_location_input').val(specific_location.text);
				$popupBgHtml.find("#aimesh_node_location_hint").css("display", "none");
				$popupBgHtml.find("#aimesh_node_location_hint").val("");
				var data = new Object();
				data.cfg_alias = specific_location.value;
				var title_name = handle_ui_model_name(node_info.model_name, node_info.ui_model_name) + " in " +  specific_location.text;
				$popupBgHtml.find("#aimesh_node_title_name").html(htmlEnDeCode.htmlEncode(title_name));
				set_AiMesh_node_config(data, node_info.mac);
			}
			else{
				$popupBgHtml.find('#aimesh_node_location_input').attr("disabled", false);
				$popupBgHtml.find('#aimesh_node_location_input').focus();
				$popupBgHtml.find('#aimesh_node_location_input').select();
				$popupBgHtml.find('#aimesh_node_location_input').val("");
				if(getBrowser_info().ie != undefined || getBrowser_info().ie != null){
					setTimeout(function () {
						$popupBgHtml.find('#aimesh_node_location_input').focus();
						$popupBgHtml.find('#aimesh_node_location_input').select();
					}, 10);
				}
			}
		}
	);
	$popupBgHtml.find("#aimesh_node_location_input").blur(
		function() {
			var validAiMeshLocation = function() {
				var node_location_text = $.trim($popupBgHtml.find("#aimesh_node_location_input").val());
				$popupBgHtml.find("#aimesh_node_location_input").val(node_location_text);
				var show_valid_hint = function(_hint){
					$popupBgHtml.find("#aimesh_node_location_hint").css("display", "block");
					$popupBgHtml.find("#aimesh_node_location_hint").html(_hint);
					$popupBgHtml.find("#aimesh_node_location_input").focus();
					$popupBgHtml.find("#aimesh_node_location_input").select();
				};
				if(node_location_text.length == 0){
					show_valid_hint("<#JS_fieldblank#>");
					return false;
				}

				var block_chars_array = ["\""];
				var block_chars_hint = "";
				for(var i = 0; i < block_chars_array.length; i++) {
					if(node_location_text.indexOf(block_chars_array[i]) >= 0)
						block_chars_hint = block_chars_array[i] + " <#JS_invalid_chars#>";
				}
				if(block_chars_hint != "") {
					show_valid_hint(block_chars_hint);
					return false;
				}

				if(utf8_ssid_support){
					var len = parent.validator.lengthInUtf8(node_location_text);
					if(len > 32){
						show_valid_hint("The field cannot be greater than 32 characters.");/* untranslated */
						return false;
					}
				}
				else if(!parent.validator.haveFullWidthChar($obj[0])) {
					show_valid_hint("<#JS_validchar#>");
					return false;
				}
				$popupBgHtml.find("#aimesh_node_location_hint").css("display", "none");
				$popupBgHtml.find("#aimesh_node_location_hint").val("");
				return true;
			};
			if(validAiMeshLocation()) {
				var location_value = $(this).val();
				var location_text = "<#AiMesh_NodeLocation01#>";
				var specific_location = aimesh_location_arr.filter(function(item, index, _array){
					return (item.value == location_value);
				})[0];
				if(specific_location != undefined){
					location_value = specific_location.value;
					location_text = specific_location.text;
					$popupBgHtml.find("#aimesh_node_location_select").val(location_value);
					$popupBgHtml.find("#aimesh_node_location_input").attr("disabled", true);
					if(specific_location.value == "Custom")
						$popupBgHtml.find("#aimesh_node_location_input").attr("disabled", false);
				}
				else{
					location_value = location_text = location_value;
					$popupBgHtml.find("#aimesh_node_location_select").val("");
					$popupBgHtml.find("#aimesh_node_location_input").attr("disabled", false);
				}
				var data = new Object();
				data.cfg_alias = location_value;
				set_AiMesh_node_config(data, node_info.mac);
				var title_name = handle_ui_model_name(node_info.model_name, node_info.ui_model_name) + " in " +  location_text;
				$popupBgHtml.find("#aimesh_node_title_name").html(htmlEnDeCode.htmlEncode(title_name));
				$popupBgHtml.find("#aimesh_node_location_input").val(location_text);
			}
		}
	);

	if(node_capability.wans_cap_wan){
		$popupBgHtml.find("#aimesh_node_connection_type").click(
			function() {
				show_change_type_hint();
			}
		);
	}
	else
		$popupBgHtml.find("#aimesh_node_connection_type").removeClass("text_hyperlink");

	if($conn_priority_select_obj.css("display") != "none") {
		$conn_priority_select_obj.change(
			function() {
				var data = new Object();
				data.amas_ethernet = $conn_priority_select_obj.val();
				set_AiMesh_node_config(data, node_info.mac);
		});
	}
	$popupBgHtml.find("#aimesh_node_fw_version").click(
		function() {
			parent.window.location.href = "/Advanced_FirmwareUpgrade_Content.asp";
		}
	);
	$popupBgHtml.find("#aimesh_node_moreconfig").click(
		function() {
			if(node_info.online == "1" && re_path > 0) {
				var moreConfig = ($popupBgHtml.find(".aimesh_node_setting_bg").css("display") == "none") ? "<#CTL_close#>" : "<#MoreConfig#>";
				$popupBgHtml.find("#aimesh_node_moreconfig").html(moreConfig);
				var display_state = $popupBgHtml.find(".aimesh_node_setting_bg").css("display");
				if(display_state == "none") {
					$popupBgHtml.find(".aimesh_node_setting_bg").slideDown(200);
					$popupBgHtml.find(".aimesh_node_setting_line").slideDown(200);
				}
				else {
					$popupBgHtml.find(".aimesh_node_setting_bg").slideUp(200);
					$popupBgHtml.find(".aimesh_node_setting_line").slideUp(200);
				}
			}
			else
				alert("<#AiMesh_Features_Disabled_When_Offline#>");
		}
	);
	if(node_capability.usb){
		$popupBgHtml.find("#aimesh_node_usb_app").click(
			function() {
				open_AiMesh_node_usb_app(node_info);
			}
		);
	}
	$popupBgHtml.find("#aimesh_node_client_tb .aimesh_node_client_item[data-clickID='client_name']").click(
		function() {
			re_sort_AiMesh_node_client("name", "str", "client_name");
		}
	);
	$popupBgHtml.find("#aimesh_node_client_tb .aimesh_node_client_item[data-clickID='client_ip']").click(
		function() {
			re_sort_AiMesh_node_client("ip", "ip", "client_ip");
		}
	);
	$popupBgHtml.find("#aimesh_node_client_tb .aimesh_node_client_item[data-clickID='client_interface']").click(
		function() {
			re_sort_AiMesh_node_client("rssi", "num", "client_interface");
		}
	);

	if(led_status.support) {
		switch(led_status.support) {
			case "central_led":
				var color_table = ["#c6dafc", "#7baaf7", "#4285f4", "#3367d6"];
				var led_table = ["<#CTL_close#>", "<#Low#>", "<#Medium#>", "<#High#>"];
				parent.$("#edit_amesh_client_block").find("#led_text").html(led_table[led_status.value]);
				parent.$("#edit_amesh_client_block").find("#led_slider").slider({
					orientation: "horizontal",
					range: "min",
					min: 1,
					max: 4,
					value: (led_status.value + 1),
					slide: function(event, ui) {
						parent.$("#edit_amesh_client_block").find("#led_text").html(led_table[ui.value-1]);
						parent.$("#edit_amesh_client_block").find("#led_slider .ui-slider-range").css("background-color", color_table[ui.value-1]);
						parent.$("#edit_amesh_client_block").find("#led_slider .ui-slider-handle").css("border-color", color_table[ui.value-1]);
					},
					stop: function(event, ui) {
						var data = new Object();
						data.bc_ledLv = (ui.value - 1);
						set_AiMesh_node_config(data, node_info.mac);
					}
				});
				break;
			case "lp55xx_led":
			case "ctrl_led" :
				var led_value = 0;
				if(led_status.support == "lp55xx_led")
					led_value = (led_status.value == "0") ? 1 : 0;
				else if(led_status.support == "ctrl_led")
					led_value = (led_status.value == "1") ? 1 : 0;
				parent.$("#edit_amesh_client_block").find('#led_radio').iphoneSwitch(led_value,
					function(){
						var data = new Object();
						if(led_status.support == "lp55xx_led"){
							data.lp55xx_lp5523_user_enable = 0;
							data.lp55xx_lp5523_user_col = 0;
							data.lp55xx_lp5523_user_beh = 0;
						}
						else if(led_status.support == "ctrl_led")
							data.led_val = 1;

						set_AiMesh_node_config(data, node_info.mac);
					},
					function(){
						var data = new Object();
						if(led_status.support == "lp55xx_led"){
							data.lp55xx_lp5523_user_enable = 1;
							data.lp55xx_lp5523_user_col = 101;
							data.lp55xx_lp5523_user_beh = 300;
						}
						else if(led_status.support == "ctrl_led")
							data.led_val = 0;

						set_AiMesh_node_config(data, node_info.mac);
					}
				);
				break;
		}
	}
	/* set event end */

	parent.$("#edit_amesh_client_block").fadeIn(300);
	parent.cal_panel_block("edit_amesh_client_block", 0.23);
	parent.adjust_panel_block_top("edit_amesh_client_block", 120);

	if(interval_ajax_AiMesh_node_clients_status) {
		clearInterval(interval_ajax_AiMesh_node_clients_status);
		interval_ajax_AiMesh_node_clients_status = false;
	}
	if(interval_ajax_onboarding_status) {
		clearInterval(interval_ajax_onboarding_status);
		interval_ajax_onboarding_status = false;
	}
	interval_ajax_onboarding_status = setInterval(ajax_onboarding, 5000);
	interval_ajax_AiMesh_node_clients_status = setInterval( function() { ajax_AiMesh_node_clients(node_info.mac); }, 5000 );

	//register event to detect mouse click
	$popupBgHtml.click(
		function() {
			aimesh_node_hide_flag = false;
		}
	);
	parent.$("body").unbind('click');
	parent.$("body").click(
		function() {
			hide_aimesh_node_block();
		}
	);
	registerIframeClick("statusframe", hide_aimesh_node_block);

	if(node_info.online == "0") {
		$popupBgHtml.find("#aimesh_node_location_select").hide();
		$popupBgHtml.find("#aimesh_node_location_input").attr("disabled", true).css("border-right", "1px solid #4c5355");
	}
}
function hide_aimesh_node_block() {
	if(aimesh_node_hide_flag) {
		parent.$("body").unbind('click');
		if(parent.$("#edit_amesh_client_block").length > 0)
			parent.$("#edit_amesh_client_block").remove();
		if(parent.$("#aimesh_change_type_hint").length > 0)
			parent.$("#aimesh_change_type_hint").remove();
		removeIframeClick("statusframe", hide_aimesh_node_block);
	}
	aimesh_node_hide_flag = true;
}
function show_offline_msg(_evt) {
	_evt.stopPropagation();

	initial_amesh_obj();

	var $offlineHtml = $('<div>');
	$offlineHtml.attr({"id" : "amesh_offline_msg"});
	$offlineHtml.addClass("amesh_popup_bg");
	$offlineHtml.css("display", "none");
	$offlineHtml.attr({"onselectstart" : "return false"});
	$offlineHtml.appendTo(parent.$('body'));

	var $amesh_hint_text = $('<div>');
	$amesh_hint_text.addClass("amesh_hint_text");
	$amesh_hint_text.html("<#AiMesh_OfflineTips#> :");
	$offlineHtml.append($amesh_hint_text);

	var $amesh_hint_content = $('<div>');
	$amesh_hint_content.addClass("amesh_hint_text");
	$amesh_hint_content.css("margin-left", "auto");
	$offlineHtml.append($amesh_hint_content);

	var $msg_item =  $('<ol>');
	var msg_text = "<li><#AiMesh_OfflineTips1#></li>";
	msg_text += "<li><#AiMesh_OfflineTips2#></li>";
	msg_text += "<li><#AiMesh_OfflineTips3#></li>";
	msg_text += "<li><#AiMesh_OfflineTips4#></li>";
	msg_text += "<li><#AiMesh_OfflineTips5#></li>";
	$msg_item.html(msg_text);
	$amesh_hint_content.append($msg_item);


	var $amesh_action_bg = $('<div>');
	$amesh_action_bg.addClass("amesh_action_bg");
	$offlineHtml.append($amesh_action_bg);

	var $amesh_ok = $('<input/>');
	$amesh_ok.addClass("button_gen");
	$amesh_ok.attr({"type" : "button", "value" : "<#CTL_ok#>"});
	$amesh_action_bg.append($amesh_ok);
	$amesh_ok.click(
		function() {
			initial_amesh_obj();
		}
	);

	parent.$("#amesh_offline_msg").fadeIn(300);
	parent.cal_panel_block("amesh_offline_msg", 0.2);
	parent.adjust_panel_block_top("amesh_offline_msg", 170);
}
function initial_amesh_obj() {
	//initial amesh obj
	if(parent.$('.amesh_popup_bg').length > 0) {
		parent.$('.amesh_popup_bg').remove();
	}

	if(parent.$("#edit_amesh_client_block").children().find(".aimesh_node_client_tr").length > 0)
		parent.$("#edit_amesh_client_block").children().find(".aimesh_node_client_tr").remove();

	if(interval_ajax_AiMesh_node_clients_status) {
		clearInterval(interval_ajax_AiMesh_node_clients_status);
		interval_ajax_AiMesh_node_clients_status = false;
	}
}
function set_process_percentage(_start, _current, _timeout, _percentage) {
	var percentage = 0;
	var interval = parseInt(_current) - parseInt(_start);
	var denominator = parseInt(_timeout) / parseInt(_percentage);
	percentage = Math.round( interval / denominator );
	return percentage;
}
function formatMAC(_value) {
	var reg = /([a-f0-9]{2})([a-f0-9]{2})/i,
	str = _value.replace(/[^a-f0-9]/ig, "");
	while (reg.test(str)) {
		str = str.replace(reg, '$1' + ':' + '$2');
	}
	return str.slice(0, 17).toUpperCase();
}
function show_change_type_hint() {
	var $changeTypeHintHtml = $('<div>');
	$changeTypeHintHtml.attr({"id" : "aimesh_change_type_hint"});
	$changeTypeHintHtml.addClass("amesh_popup_bg");
	$changeTypeHintHtml.attr({"onselectstart" : "return false"});
	$changeTypeHintHtml.appendTo(parent.$('body'));

	var $changeTypeHintText = $('<div>');
	var result_text = "";
	result_text = "<#AiMesh_Node_AddDiagram#>";
	$changeTypeHintText.addClass("amesh_hint_text");
	$changeTypeHintText.html(result_text);
	$changeTypeHintHtml.append($changeTypeHintText);

	var $aimesh_illustration_bg = $('<div>');
	$aimesh_illustration_bg.addClass("aimesh_illustration_bg");
	$changeTypeHintHtml.append($aimesh_illustration_bg);

	var gen_icon = function(_class) {
		var $iconBg = $('<div>');
		$iconBg.addClass("aimesh_illustration_icon");
		$iconBg.addClass(_class);
		return $iconBg;
	};
	var gen_text = function(_text, _class) {
		var $textBg = $('<div>');
		$textBg.addClass("aimesh_illustration_text");
		$textBg.addClass(_class);
		$textBg.html(_text);
		return $textBg;
	};

	if(!dsl_support){
		$aimesh_illustration_bg.append(gen_icon("modem"));
		$aimesh_illustration_bg.append(gen_icon("ethernet"));
		$aimesh_illustration_bg.append(gen_icon("router_back"));
		$aimesh_illustration_bg.append(gen_icon("router_back_right"));
		$aimesh_illustration_bg.append(gen_icon("ethernet_lan"));
		$aimesh_illustration_bg.append(gen_icon("ethernet_wan"));
		$aimesh_illustration_bg.append(gen_text("LAN - WAN", "lan_to_wan"));
		$aimesh_illustration_bg.append(gen_text("Modem", "modem"));
		$aimesh_illustration_bg.append(gen_text("<#AiMesh_Router#>", "backhaul_router"));
		$aimesh_illustration_bg.append(gen_text("<#AiMesh_Node#>", "backhaul_node"));
	}
	else{
		$aimesh_illustration_bg.append(gen_icon("router_back for_dsl"));
		$aimesh_illustration_bg.append(gen_icon("router_back_right for_dsl"));
		$aimesh_illustration_bg.append(gen_icon("ethernet_lan for_dsl"));
		$aimesh_illustration_bg.append(gen_icon("ethernet_wan for_dsl"));
		$aimesh_illustration_bg.append(gen_text("LAN - WAN", "lan_to_wan for_dsl"));
		$aimesh_illustration_bg.append(gen_text("AiMesh xDSL modem router", "backhaul_router for_dsl"));    /*Untranslated*/
		$aimesh_illustration_bg.append(gen_text("<#AiMesh_Node#>", "backhaul_node for_dsl"));
	}

	var $amesh_clear_bg = $('<div>');
	$amesh_clear_bg.css("clear", "both");
	$aimesh_illustration_bg.append($amesh_clear_bg);

	var $amesh_action_bg = $('<div>');
	$amesh_action_bg.addClass("amesh_action_bg");
	$changeTypeHintHtml.append($amesh_action_bg);

	var $amesh_close = $('<input/>');
	$amesh_close.addClass("button_gen");
	$amesh_close.attr({"type" : "button", "value" : "<#CTL_close#>"});
	$amesh_action_bg.append($amesh_close);
	$amesh_close.click(
		function() {
			if(parent.$('#aimesh_change_type_hint').length == 1)
				parent.$('#aimesh_change_type_hint').remove();
		}
	);

	parent.$("#aimesh_change_type_hint").fadeIn(300);
	parent.cal_panel_block("aimesh_change_type_hint", 0.4);
	parent.adjust_panel_block_top("aimesh_change_type_hint", 280);
	$changeTypeHintHtml.click(
		function() {
			aimesh_node_hide_flag = false;
		}
	);
}
function ajax_AiMesh_node_clients(_nodeMac){
	$.ajax({
		url: '/ajax_AiMesh_node_clients.asp',
		dataType: 'script',
		success: function(response){
			if(parent.$("#edit_amesh_client_block").length > 0) {
				if(parent.$("#edit_amesh_client_block").children().find(".aimesh_node_client_tr").length > 0)
					parent.$("#edit_amesh_client_block").children().find(".aimesh_node_client_tr").remove();

				var getNodeInfo = function(_jsonArray, _idxArray) {
					var result = "";
					for (var idx in _jsonArray) {
						if (_jsonArray[idx].hasOwnProperty(_idxArray[0])) {
							if(_jsonArray[idx][_idxArray[0]] == _idxArray[1]) {
									result = _jsonArray[idx];
									break;
							}
						}
					}
					return result;
				};
				var node_info = getNodeInfo(get_cfg_clientlist, ["mac", _nodeMac]);
				var wired_client = get_wiredclientlist[_nodeMac];
				var wl_client = get_wclientlist[_nodeMac];
				var re_path = handle_re_path(node_info.re_path);
				aimesh_node_client_list = [];
				if(node_info.online == "1" && re_path > 0)
					aimesh_node_client_list = get_aimesh_node_client_list(wired_client, wl_client, _nodeMac);
				aimesh_node_client_list = sorterApi.sortJson(aimesh_node_client_list, sorterApi.key, sorterApi.type);
				parent.$("#edit_amesh_client_block").children().find("#aimesh_node_client_header_tr").after(gen_AiMesh_node_client(aimesh_node_client_list));

				var model_info = {"model_name": node_info.model_name, "tcode": node_info.tcode, "cobrand": node_info.cobrand, "icon_model_name":  node_info.icon_model_name};
				var cloudModelName = httpApi.transformCloudModelName(model_info);
				if(checkCloudIconExist[cloudModelName]) {
					parent.$("#edit_amesh_client_block .amesh_router_icon.card").css('background-image', 'url(' + checkCloudIconExist[cloudModelName] + ')');
					parent.$("#edit_amesh_client_block .amesh_router_icon.card").addClass('amesh_router_image_web');
					parent.$("#edit_amesh_client_block .amesh_router_icon.card").removeClass('amesh_router_icon');
				}

				parent.$("#edit_amesh_client_block #aimesh_node_connection_type").html(get_connect_type(node_info).text);
				parent.$("#edit_amesh_client_block .amesh_interface_icon.static_info").html(get_connect_type(node_info).icon);
			}
			else {
				if(interval_ajax_AiMesh_node_clients_status) {
					clearInterval(interval_ajax_AiMesh_node_clients_status);
					interval_ajax_AiMesh_node_clients_status = false;
				}
			}
		}
	});
}
function re_sort_AiMesh_node_client(_key, _sortType, _clickID) {
	if(parent.$("#aimesh_node_client_tb").children().find(".aimesh_node_client_tr").length > 0)
		parent.$("#aimesh_node_client_tb").children().find(".aimesh_node_client_tr").remove();

	sorterApi.method = (sorterApi.method == "increase") ? "decrease" : "increase";
	sorterApi.key = _key;
	sorterApi.type = _sortType;
	sorterApi.clickID = _clickID;

	aimesh_node_client_list = sorterApi.sortJson(aimesh_node_client_list, sorterApi.key, sorterApi.type);
	parent.$("#edit_amesh_client_block .aimesh_node_info_bg #aimesh_node_client_tb #aimesh_node_client_header_tr").after(gen_AiMesh_node_client(aimesh_node_client_list));
	sorterApi.drawBorder(parent.$("#edit_amesh_client_block .aimesh_node_info_bg #aimesh_node_client_tb .aimesh_node_client_item"));
}
function gen_AiMesh_node_client(_nodeClient_array) {
	var nodeClientHtml = "";
	if(_nodeClient_array.length == 0)
		nodeClientHtml = "<tr class='aimesh_node_client_tr'><td style='color:#FFCC00;' colspan='" + aimesh_node_client_list_colspan + "'><#IPConnection_VSList_Norule#></td></tr>";
	else {
		for(var i = 0; i < _nodeClient_array.length; i += 1) {
			var nodeClientObj = _nodeClient_array[i];
			if(nodeClientObj == undefined) //if client list no this mac.
				continue;
			nodeClientHtml += "<tr class='aimesh_node_client_tr' height='48px'>";
			var userIconBase64 = "NoIcon";
			var deviceTitle = (nodeClientObj.dpiDevice == "") ? nodeClientObj.vendor : nodeClientObj.dpiDevice;
			if(usericon_support) {
				if(aimesh_node_client_upload_icon[nodeClientObj.mac] == undefined) {
					var clientMac = nodeClientObj.mac.replace(/\:/g, "");
					userIconBase64 = getUploadIcon(clientMac);
					aimesh_node_client_upload_icon[nodeClientObj.mac] = userIconBase64;
				}
				else
					userIconBase64 = aimesh_node_client_upload_icon[nodeClientObj.mac];
			}
			nodeClientHtml += "<td class='IE8HACK' width='" + aimesh_node_client_info_width[0] + "' align='center' title='" + deviceTitle + "'>";
			if(userIconBase64 != "NoIcon") {
				nodeClientHtml += "<div class='aimesh_node_client_icon_userIcon' style='background-image:url(" + userIconBase64 + ");'></div>";
				nodeClientHtml += "";
			}
			else if( nodeClientObj.type != "0" || nodeClientObj.vendor == "") {
				var icon_type = "type" + nodeClientObj.type;
				nodeClientHtml += "<div class='clientIcon_no_hover " + icon_type + " aimesh_node_client_icon_default'></div>";
			}
			else if(nodeClientObj.vendor != "" ) {
				var venderIconClassName = getVenderIconClassName(nodeClientObj.vendor.toLowerCase());
				if(venderIconClassName != "" && !downsize_4m_support) {
					nodeClientHtml += "<div class='venderIcon_no_hover " + venderIconClassName + " aimesh_node_client_icon_vendor'></div>";
				}
				else {
					var icon_type = "type" + nodeClientObj.type;
					nodeClientHtml += "<div class='clientIcon_no_hover " + icon_type + " aimesh_node_client_icon_default'></div>";
				}
			}
			nodeClientHtml += "</td>";
			var client_name = (nodeClientObj.nickName == "") ? nodeClientObj.name : nodeClientObj.nickName;
			nodeClientHtml += "<td width='" + aimesh_node_client_info_width[1] + ";'style='word-wrap:break-word; word-break:break-all;'>" + client_name + "</td>";
			var nodeIP = (nodeClientObj.ip == "offline") ? "<#Distributing_IP#>" : nodeClientObj.ip;
			nodeClientHtml += "<td width='" + aimesh_node_client_info_width[2] + "'>" + nodeIP + "</td>";
			var rssi = 0;
			if(nodeClientObj.isWL == "0")
				rssi = "wired";
			else
				rssi = client_convRSSI(nodeClientObj.rssi);

			nodeClientHtml += "<td width='" + aimesh_node_client_info_width[3] + "' align='center'>";
			nodeClientHtml += "<div style='margin: auto;' class='radioIcon radio_" + rssi + "'></div>";
			if(nodeClientObj.isWL != "0") {
				var bandClass = (navigator.userAgent.toUpperCase().match(/CHROME\/([\d.]+)/)) ? "band_txt_chrome" : "band_txt";
				nodeClientHtml += "<div class='band_block' style='margin: auto;'><span class=" + bandClass + " style='color: #000000;'>" + isWL_map[nodeClientObj.isWL]["text"] + "</span></div>";
			}
			nodeClientHtml += "</td>";
			nodeClientHtml += "</tr>";
		}
		if(nodeClientHtml == "")
			nodeClientHtml = "<tr class='aimesh_node_client_tr'><td style='color:#FFCC00;' colspan='" + aimesh_node_client_list_colspan + "'><#IPConnection_VSList_Norule#></td></tr>";
	}
	return nodeClientHtml;
}
function set_AiMesh_node_config(_data, _nodeMac) {
	httpApi.nvramSet({
		"config" : JSON.stringify(_data),
		"re_mac" : _nodeMac,
		"action_mode": "config_changed"
	}, setTimeout(ajax_onboarding, 1000));
}
function get_aimesh_node_client_list(_wired_client, _wl_client, _node_mac) {
	var node_client_array = [];
	var node_client_mac = "";
	if(_wired_client != undefined) {
		for(var i = 0; i < _wired_client.length; i += 1) {
			node_client_mac = _wired_client[i];
			if(clientList[node_client_mac] != undefined) {
				if(clientList[node_client_mac].isOnline && clientList[node_client_mac].amesh_papMac == _node_mac)
					node_client_array.push(clientList[node_client_mac]);
			}
		}
	}

	for(var wc_idx in _wl_client) {
		if(_wl_client.hasOwnProperty(wc_idx)) {
			for(var i = 0; i < _wl_client[wc_idx].length; i += 1) {
				node_client_mac = _wl_client[wc_idx][i];
				if(clientList[node_client_mac] != undefined) {
					if(clientList[node_client_mac].isOnline && clientList[node_client_mac].amesh_papMac == _node_mac) {
						node_client_array.push(clientList[node_client_mac]);
					}
				}
			}
		}
	}

	return node_client_array;
}
function get_connect_type(_node_info) {
	var component = {"text":"", "icon":""};
	if(_node_info.online == "1" && handle_re_path(_node_info.re_path) > 0){
		if(_node_info.re_path == "1" || _node_info.re_path == "16" || _node_info.re_path == "32" || _node_info.re_path == "64") {
			var real_port_type = gen_real_port_type(_node_info);
			if(real_port_type.type == "eth"){
				component.text = "<#tm_wired#>";
				component.icon = "<div class='radioIcon radio_wired'></div>";
			}
			else if(real_port_type.type == "plc"){
				component.text = "<#Powerline#>";
				component.icon = "<div class='radioIcon radio_plc'></div>";
			}
			else if(real_port_type.type == "moca"){
				component.text = "MoCa";
				component.icon = "<div class='radioIcon radio_moca'></div>";
			}
		}
		else {
			component.text = "<#tm_wireless#>";
			var bandClass = (navigator.userAgent.toUpperCase().match(/CHROME\/([\d.]+)/)) ? "band_txt_chrome" : "band_txt";
			var wireless_band = 0;
			var wireless_band_array = ["2.4G", "5G", "6G"];
			var wireless_rssi = 4;
			if(_node_info.re_path == "2") {
				wireless_band = 0;
				wireless_rssi = client_convRSSI(_node_info.rssi2g);
			}
			else if(_node_info.re_path == "128") {
				wireless_band = 2;
				wireless_rssi = client_convRSSI(_node_info.rssi6g);
			}
			else {
				wireless_band = 1;
				wireless_rssi = client_convRSSI(_node_info.rssi5g);
			}
			component.icon = "<div class='radioIcon radio_" + wireless_rssi +"'></div>";
			component.icon += "<div class='band_block'><span class=" + bandClass + ">" + wireless_band_array[wireless_band]  + "</span></div>";
		}
	}
	else{
		component.text = "<#Disconnected#>";
		component.icon = "";
	}
	return component;
}
function gen_real_port_type(_node_info){
	var result = {type:"eth", speed:"G"};
	var interface_mapping = [{value:"1", type:"eth"}, {value:"2", type:"wifi"}, {value:"3", type:"plc"}, {value:"4", type:"moca"}];//Type
	var eth_rate_mapping = [{value:"1", type:"M"}, {value:"2", type:"M"}, {value:"3", type:"G"}, {value:"4", type:"F"}, {value:"5", type:"Q"},
		{value:"6", type:"T"}, {value:"7", type:"T"}];//SubType
	var eth_re_path_sequence = {"1":1, "16":2, "32":3, "64":4};
	var eth_mapping_idx = ((eth_re_path_sequence[_node_info.re_path] == undefined) ? 0 : (eth_re_path_sequence[_node_info.re_path]));
	var eth_current_idx = 1;
	if(_node_info.re_path == "1" || _node_info.re_path == "16" || _node_info.re_path == "32" || _node_info.re_path == "64"){
		if("capability" in _node_info){
			if("21" in _node_info.capability) {
				var multiple_uplink_port = _node_info.capability["21"];
				if(multiple_uplink_port["Ports"] != undefined) {
					$.each(multiple_uplink_port["Ports"], function( index, value ){
						var conn_type = "";
						var port_obj = value;
						var if_type = interface_mapping.filter(function(item, index, _array){
							return (item.value == port_obj.Type);
						})[0];
						if(if_type != undefined){
							var rate_type = "";
							switch(if_type.value){
								case "1":
									if(eth_current_idx == eth_mapping_idx){
										result.type = "eth";
										rate_type =  eth_rate_mapping.filter(function(item, index, _array){
											return (item.value == port_obj.SubType);
										})[0];
										if(rate_type != undefined && rate_type != "")
											result.speed = rate_type.type;
										return false;
									}
									eth_current_idx++;
									break;
								case "3":
									if(eth_current_idx == eth_mapping_idx){
										result.type = "plc";
										return false;
									}
									eth_current_idx++;
									break;
								case "4":
									if(eth_current_idx == eth_mapping_idx){
										result.type = "moca";
										return false;
									}
									eth_current_idx++;
									break;
							}
						}
					});
				}
			}
		}
	}
	return result;
}
function handle_ui_model_name(_model_name, _ui_model_name){
	var result = "";
	if(_ui_model_name == undefined || _ui_model_name == "")
		result = _model_name;
	else
		result = _ui_model_name;
	return result;
}
function gen_conn_priority_select_option(_node_info, _eap_flag){
	var option_array = [];
	var option_attr = function(){
		this.text = "";
		this.value = "";
		this.conn_type = "";
	};
	var gen_option_attr = function(_value, _text, _conn_type) {
		var option = new option_attr();
		option.value = _value;
		option.text = _text;
		option.conn_type = _conn_type;
		return JSON.parse(JSON.stringify(option));
	};

	option_array.push(gen_option_attr("3", ((_eap_flag) ? "<#Auto#> (<#AiMesh_Node_ConnPrio_Eth_Based_Title#>)" : "<#Auto#>"), "auto"));

	var port_mapping = [{value:"1", text:"WAN"}, {value:"2", text:"LAN"}];//Def
	var interface_mapping = [{value:"1", text:"Ethernet"}, {value:"2", text:"WiFi"}, {value:"3", text:"Powerline"}, {value:"4", text:"MoCa"}];//Type
	var eth_rate_mapping = [{value:"1", text:"10M"}, {value:"2", text:"100M"}, {value:"3", text:"1G"}, {value:"4", text:"2.5G"}, {value:"5", text:"5G"},
		{value:"6", text:"10G base-T"}, {value:"7", text:"10G SFP+"}];//SubType
	var wifi_rate_mapping = [{value:"1", text:"2.4GHz"}, {value:"2", text:"5GHz"}, {value:"3", text:"6GHz"}];//SubType

	if("capability" in _node_info){
		if("21" in _node_info.capability) {
			var multiple_uplink_port = _node_info.capability["21"];
			if(multiple_uplink_port["Ports"] != undefined) {
				$.each(multiple_uplink_port["Ports"], function( index, value ){
					var if_text = "";
					var rate_text = "";
					var conn_type = "";
					var port_obj = value;
					var port_idx = parseInt(port_obj.index);
					var if_type = interface_mapping.filter(function(item, index, _array){
						return (item.value == port_obj.Type);
					})[0];
					if(if_type != undefined){
						var rate_type = "";
						switch(if_type.value){
							case "1":
								var port_type = port_mapping.filter(function(item, index, _array){
									return (item.value == port_obj.Def);
								})[0];
								if(port_type != undefined)
									if_text = port_type.text;
								rate_type =  eth_rate_mapping.filter(function(item, index, _array){
									return (item.value == port_obj.SubType);
								})[0];
								conn_type = "eth";
								break;
							case "2":
								if_text = if_type.text;
								rate_type =  wifi_rate_mapping.filter(function(item, index, _array){
									return (item.value == port_obj.SubType);
								})[0];
								conn_type = "wifi";
								break;
							case "3":
								if_text = if_type.text;
								conn_type = "plc";
								break;
							case "4":
								if_text = if_type.text;
								conn_type = "moca";
								break;
						}
						if(rate_type != undefined && rate_type != "")
							rate_text = rate_type.text;
					}

					var option_value = port_obj.amas_ethernet;
					var conn_prio_type = rate_text;
					if(conn_type == "wifi"){
						if(port_idx >= 1){
							conn_prio_type += "-" + (port_idx + 1);
						}
						else if(port_idx == 0){
							if(rate_type.value == "2"){//5G
								if(_node_info.capability["22"] != undefined){
									var support_5G2 = (_node_info.capability["22"] & (1 << 2)) ? true : false;
									if(support_5G2)
										conn_prio_type += "-" + (port_idx + 1);//if support 5G-2, 5G need show 5G-1
								}
							}
						}
						conn_prio_type += " " + if_text;
					}
					else{
						conn_prio_type += " " + if_text;
						if(port_idx >= 1)
							conn_prio_type += port_idx;
					}
					var option_text = (_eap_flag) ? "<#AiMesh_BackhaulConnPrio_Only#>" : "<#AiMesh_BackhaulConnPrio_First#>";
					option_text = option_text.replace("#CONNPRIOTYPE", conn_prio_type);
					var option_conn_type = conn_type;
					if(_eap_flag && conn_type == "wifi")
						return true;
					if(option_value == "" || option_text == "" || option_conn_type == "")
						return true;
					option_array.push(gen_option_attr(option_value, option_text, option_conn_type));
				});
			}
		}
	}

	if(option_array.length == 1)
		option_array.push(gen_option_attr("2", ((_eap_flag) ? "<#AiMesh_Node_ConnPrio_Eth_Only_Title#>" : "<#AiMesh_Node_ConnPrio_Eth_First_Title#>"), "eth"));

	return option_array;
}
function handle_re_path(_re_path){
	var result = parseInt(_re_path);
	return ((isNaN(result)) ? 0 : result);
}
</script>
</head>

<body class="statusbody" onload="initial();">
<iframe name="applyFrame" id="applyFrame" src="" width="0" height="0" frameborder="0" scrolling="no"></iframe>
<form method="post" name="form" id="form" action="/applyapp.cgi" target="applyFrame">
<input type="hidden" name="productid" value="<% nvram_get("productid"); %>">
<input type="hidden" name="action_mode" value="onboarding">
<input type="hidden" name="action_wait" value="3">
<input type="hidden" name="current_page" value="device-map/amesh.asp">
<input type="hidden" name="next_page" value="device-map/amesh.asp">
<input type="hidden" name="re_mac" value=''>
<input type="hidden" name="new_re_mac" value=''>
</form>
<iframe name="resetFrame" id="resetFrame" src="" width="0" height="0" frameborder="0" scrolling="no"></iframe>
<form method="post" name="reset_form" id="reset_form" action="/applyapp.cgi" target="resetFrame">
<input type="hidden" name="productid" value="<% nvram_get("productid"); %>">
<input type="hidden" name="action_mode" value="reset_default">
<input type="hidden" name="action_wait" value="3">
<input type="hidden" name="current_page" value="device-map/amesh.asp">
<input type="hidden" name="next_page" value="device-map/amesh.asp">
<input type="hidden" name="slave_mac" value=''>
</form>
<iframe name="onboardingLEDFrame" id="onboardingLEDFrame" src="" width="0" height="0" frameborder="0" scrolling="no"></iframe>
<form method="post" name="onboardingLED_form" id="onboardingLED_form" action="/applyapp.cgi" target="onboardingLEDFrame">
<input type="hidden" name="productid" value="<% nvram_get("productid"); %>">
<input type="hidden" name="action_mode" value="ob_selection">
<input type="hidden" name="action_wait" value="3">
<input type="hidden" name="current_page" value="device-map/amesh.asp">
<input type="hidden" name="next_page" value="device-map/amesh.asp">
<input type="hidden" name="new_re_mac" value=''>
<input type="hidden" name="ob_path" value=''>
</form>
<table id="description_table" width="95%" border="0" align="center" cellpadding="4" cellspacing="0" class="description_table">
	<tr>
		<td>
			<div class="description_icon"></div>
		</td>
		<td>
			<div><#AiMesh_Desc#></div>
		</td>
	</tr>
	<tr>
</table>
<table  width="95%" border="1" align="center" cellpadding="4" cellspacing="0" bordercolor="#6b8fa3" class="table1px" style="margin-bottom:5px;display:;">
	<tr>
		<td colspan="2">
			<div class="amesh_title"><#AiMesh_FindNode#></div>
			<div class="amesh_title amesh_content"><#AiMesh_FindNode_Desc#></div>
		</td>
	</tr>
	<tr id="ready_onBoarding_table" style="display:none;">
		<td colspan="2">
			<div id="ready_onBoarding_block"></div>
		</td>
	</tr>
	<tr>
		<td width="50%">
			<div class="amesh_title amesh_help" onclick="scenario();"><#AiMesh_FindNode_confirm#></div>
		</td>
		<td width="50%" height="32px;" style="text-align:center;">
			<input id="searchReadyOnBoarding" type="button" class="button_gen" value="<#CTL_search#>" onClick="searchReadyOnBoarding();">
			<div id="amesh_loadingIcon" class="amesh_loadingIcon" style="display:none;"></div>
		</td>
	</tr>
	<tr>
</table>
<div class="amesh_node_title">
	<#AiMesh_NodeList#>
</div>
<table width="95%" border="1" align="center" cellpadding="4" cellspacing="0" bordercolor="#6b8fa3" class="table1px">
	<tr>
		<td>
			<div id="onBoarding_block"><div class='amesh_no_data'><#IPConnection_VSList_Norule#></div></div>
		</td>
	</tr>
	<tr>
</table>
</body>
</html>
