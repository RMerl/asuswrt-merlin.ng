<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01 Transitional//EN"
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
var wps_enable_status = '<% nvram_get("wps_enable"); %>';
var aimesh_node_client_list = new Array();
var aimesh_node_client_list_colspan = 4;
var aimesh_node_client_info_width = (top.isIE8) ? ["", "40%", "40%", "20%"] : ["15", "40%", "30%", "15%"];
var aimesh_node_client_upload_icon = new Array();
var aimesh_select_new_re_mac = "";
var AUTOLOGOUT_MAX_MINUTE_ORI = 0;
var restore_autologout = false;
var led_control = {
	"status": function(_node_info){
		var result = {"support": 0, "value": 0};
		var firstCheck = false;
		var secondCheck = false;

		if("capability" in _node_info) {
			if("1" in _node_info.capability) {
				result.support = _node_info.capability["1"];
				firstCheck = true;
			}
		}

		if("config" in _node_info) {
			if("central_led" in _node_info.config) {
				if("bc_ledLv" in _node_info.config.central_led) {
					result.value = parseInt(_node_info.config.central_led.bc_ledLv);
					secondCheck = true;
				}
			}
			else if("lp55xx_led" in _node_info.config) {
				if("lp55xx_lp5523_user_enable" in _node_info.config.lp55xx_led) {
					result.value = parseInt(_node_info.config.lp55xx_led.lp55xx_lp5523_user_enable);
					secondCheck = true;
				}
			}
		}

		if(isNaN(result.value))
			result.value = 0;
		if(!firstCheck || !secondCheck) {
			result.support = 0;
			result.value = 0;
		}
		return result;
	},
	"component": function(_supportType) {
		var html = "";
		switch(parseInt(_supportType)) {
			case 1 :
				html += "<div style='margin-top:10px;'>";
				html += "<div class='aimesh_node_setting_info_title'><#LED_Brightness#></div>";
				html += "<div class='aimesh_node_setting_info_content'>";
				html += "<div id='led_slider' class='led_slider'></div>";
				html += "<div id='led_text'></div>";
				html += "</div>";
				html += "<div class='clear_both'></div>";
				html += "</div>";
				break;
			case 2 :
				html += "<div style='margin-top:10px;'>";
				html += "<div class='aimesh_node_setting_info_title'>LED</div>";
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
									var rssi = newReMacArray[newReMac].rssi;
									var source = newReMacArray[newReMac].source;
									show_connect_msg(reMac, newReMac, model_name, rssi, source);
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
							show_connect_msg("", mac, "New Node", "-1");
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
							var rssi = newReMacArray[newReMac].rssi;
							var device_info = model_name + "<br>"; 
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
			var rssi = newReMacArray[newReMac].rssi;
			var onboarding_device_id = newReMac.replace(/:/g, "");
			var source = newReMacArray[newReMac].source;
			if($('#ready_onBoarding_block').find('#' + onboarding_device_id + '').length == 0) {
				if(checkCloudIconExist[model_name] == undefined)
					checkCloudIconExist[model_name] = false;

				var code = "";
				code += "<div id='" + onboarding_device_id + "'>";
					code += "<div class='amesh_rotate'>";
					code += "<div class='amesh_line'><i class='amesh_line_run'></i></div>";
					code += "<div class='amesh_line'><i class='amesh_line_run'></i></div>";
					code += "<div class='amesh_line'><i class='amesh_line_run'></i></div>";
					code += "<div class='amesh_line'><i class='amesh_line_run'></i></div>";
					code += "<div class='amesh_router_icon_bg'>";
						code += "<div class='amesh_each_router_icon_bg'></div>";
						if(checkCloudIconExist[model_name])
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
						code += model_name;
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
							if(source == 2)
								code += "<div class='radioIcon radio_wired'></div>";
							else
								code += "<div class='radioIcon radio_" + convRSSI(rssi) + "'></div>";
							code += '</div>';
						code += "</div>";
					code += "</div>";
					code += "</div>";
				code += "</div>";
				$('#ready_onBoarding_block').append(code);

				$('#ready_onBoarding_block').find('#' + onboarding_device_id + '').find('.amesh_rotate').unbind('click');
				$('#ready_onBoarding_block').find('#' + onboarding_device_id + '').find('.amesh_rotate').click(
					function() {
						show_connect_msg(reMac, newReMac, model_name, rssi, source);
					}
				);

				if(isNaN(parseInt(checkCloudIconErrorTimes[model_name])))
					checkCloudIconErrorTimes[model_name] = 0;
				download_cloud_icon(model_name, onboarding_device_id, "ready_onBoarding_block");

				if(typeof parent.show_AMesh_status !== 'undefined' && $.isFunction(parent.show_AMesh_status))
					parent.show_AMesh_status($('#ready_onBoarding_block').children().length, 0);
			}
			else {
				$('#ready_onBoarding_block').find('#' + onboarding_device_id + '').find('.amesh_rotate').unbind('click');
				$('#ready_onBoarding_block').find('#' + onboarding_device_id + '').find('.amesh_rotate').click(
					function() {
						show_connect_msg(reMac, newReMac, model_name, rssi, source);
					}
				);
				if(newReMac != aimesh_select_new_re_mac){
					if(source == 2)
						$('#ready_onBoarding_block').find('#' + onboarding_device_id + '').children().find('.radioIcon').removeClass().addClass('radioIcon radio_wired');
					else
						$('#ready_onBoarding_block').find('#' + onboarding_device_id + '').children().find('.radioIcon').removeClass().addClass('radioIcon radio_' + convRSSI(rssi) + '');
				}
				$('#ready_onBoarding_block').find('#' + onboarding_device_id + '').children().find('.amesh_router_info_text.model_name').html(model_name);

				if(isNaN(parseInt(checkCloudIconErrorTimes[model_name])))
					checkCloudIconErrorTimes[model_name] = 0;
				download_cloud_icon(model_name, onboarding_device_id, "ready_onBoarding_block");
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
				var fwver = _onboardingList[idx].fwver;
				var mac = _onboardingList[idx].mac.toUpperCase();
				var device_id = mac.replace(/:/g, "");
				var ipAddr = _onboardingList[idx].ip;
				var pap2g = _onboardingList[idx].pap2g;
				var pap5g = _onboardingList[idx].pap5g;
				var rssi2g = _onboardingList[idx].rssi2g;
				var rssi5g = _onboardingList[idx].rssi5g;
				var online = _onboardingList[idx].online;
				var connect_type = _onboardingList[idx].re_path;
				var wireless_band = 0;
				var wireless_band_array = ["2.4 G", "5 G"];
				var wireless_rssi = 4;
				var alias = "My Home";
				if("config" in _onboardingList[idx]) {
					if("misc" in _onboardingList[idx].config) {
						if("cfg_alias" in _onboardingList[idx].config.misc) {
							if(_onboardingList[idx].config.misc.cfg_alias != "")
								alias = _onboardingList[idx].config.misc.cfg_alias;
						}
					}
				}

				if($("script[src='../calendar/jquery-ui.js']").length == 0) {
					if(led_control.status(_onboardingList[idx]).support == 1)
						addNewScript("../calendar/jquery-ui.js");
				}
				if(parent.$("script[src='/calendar/jquery-ui.js']").length == 0) {
					if(led_control.status(_onboardingList[idx]).support == 1)
						parent.addNewScript("/calendar/jquery-ui.js");
				}
				if($("script[src='../switcherplugin/jquery.iphone-switch.js']").length == 0) {
					if(led_control.status(_onboardingList[idx]).support == 2)
						addNewScript("../switcherplugin/jquery.iphone-switch.js");
				}

				if(connect_type == "1")
					wireless_rssi = "wired";
				else if(connect_type == "2") {
					wireless_band = 0;
					wireless_rssi = convRSSI(rssi2g);
				}
				else {
					wireless_band = 1;
					wireless_rssi = convRSSI(rssi5g);
				}

				if($('#ready_onBoarding_block').find('#' + device_id + '').length == 0 && 
					$('#onBoarding_block').find('#' + device_id + '').length == 0
				) {
					if(checkCloudIconExist[model_name] == undefined)
						checkCloudIconExist[model_name] = false;

					var code = "";
					code += "<div id='" + device_id + "' class='amesh_pairing_bg' >";
						code += "<div class='amesh_router_icon_bg pairing'>";
							if(checkCloudIconExist[model_name])
								code += "<div class='amesh_each_router_icon_bg amesh_router_image_web' style='background-image:url(\"" + checkCloudIconExist[model_name]  + "\");'></div>";
							else
								code += "<div class='amesh_each_router_icon_bg amesh_router_icon'></div>";
						code += "</div>";
						code += "<div class='vertical_line pairing'></div>";
						code += "<div class='amesh_router_info_bg'>";
							code += "<div class='amesh_router_info_title'>";
							code += model_name;
							code += "<div class='device_reset' onclick='reset_re_device(\"" + mac + "\", \"" + model_name + "\", event);'></div>";
							code += "</div>";
							code += "<div class='horizontal_line'></div>";
							code += "<div style='position:relative;'>";
								code += "<div class='amesh_router_info_text location' title='" + alias + "'>";
								var location = alias;
								if(alias.length > 22) {
									location = alias.substring(0, 20) + "..";
								}
								code += "<span class='amesh_node_content'>" + location + "</span>";
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
									if(connect_type != "1") {
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

					if(isNaN(parseInt(checkCloudIconErrorTimes[model_name])))
						checkCloudIconErrorTimes[model_name] = 0;
					if(!checkCloudIconExist[model_name])
						download_cloud_icon(model_name, device_id, "onBoarding_block");			
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

function connectingDevice(_reMac, _newReMac) {
	document.form.re_mac.disabled = false;
	document.form.new_re_mac.disabled = false;
	document.form.re_mac.value = _reMac;
	document.form.new_re_mac.value = _newReMac;
	document.form.submit();
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
				show_connect_result(cfg_obresult, get_onboardingstatus.cfg_newre, get_onboardingstatus.cfg_obmodel);
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
	description += "<a style='font-weight:bolder;text-decoration:underline;color:#FC0;' href='https://www.asus.com/AiMesh/' target='_blank'><#AiMesh_Desc3_note#></a>";
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
}
function show_connect_msg(_reMac, _newReMac, _model_name, _rssi, _ob_path) {
	aimesh_select_new_re_mac = _newReMac;
	$.ajax({
		url: '/ajax_onboarding.asp',
		dataType: 'script',
		error: function(xhr) {
			setTimeout(function(){
				show_connect_msg(_reMac, _newReMac, _model_name, _rssi, _ob_path);
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
				var device_info = _model_name + "<br>"; 
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
						initial_amesh_obj();
						connectingDevice(_reMac, _newReMac);
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
function show_connect_result(_status, _newReMac, _model_name) {
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
		$successResult1.find(".amesh_device_info").html(_model_name + " (" + labelMac + ")");

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
		$amesh_hint_text.find(".amesh_device_info").html(_model_name + " (" + labelMac + ")");
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
function download_cloud_icon(model_name, device_id, parent_bg_id) {
	var set_default_router_icon = function(_parent_bg_id, _device_id) {
		if($('#' + _parent_bg_id + '').find('#' + _device_id + '').length != 0) {
			$ ('#' + _parent_bg_id + '').find('#' + _device_id + '').children().find('.amesh_each_router_icon_bg').addClass('amesh_router_icon');
		}
	};
	var set_cloud_router_icon = function(_parent_bg_id, _device_id) {
		if($('#' + _parent_bg_id + '').find('#' + _device_id + '').length != 0) {
			$('#' + _parent_bg_id + '').find('#' + _device_id + '').children().find('.amesh_each_router_icon_bg').css('background-image', 'url(' + checkCloudIconExist[model_name] + ')');
			$('#' + _parent_bg_id + '').find('#' + _device_id + '').children().find('.amesh_each_router_icon_bg').addClass('amesh_router_image_web');
			$('#' + _parent_bg_id + '').find('#' + _device_id + '').children().find('.amesh_each_router_icon_bg').removeClass('amesh_router_icon');
		}
	};

	if('<% nvram_get("x_Setting"); %>' == '1' && parent.wanConnectStatus && checkCloudIconErrorTimes[model_name] < 5 && !checkCloudIconExist[model_name]) {
		httpApi.checkCloudModelIcon(
			model_name,
			function(src){
				checkCloudIconExist[model_name] = src;
				set_cloud_router_icon(parent_bg_id, device_id);
				checkCloudIconErrorTimes[model_name] = 0;
			},
			function(){
				checkCloudIconExist[model_name] = false;
				set_default_router_icon(parent_bg_id, device_id);
				checkCloudIconErrorTimes[model_name] = parseInt(checkCloudIconErrorTimes[model_name]) + 1;
			}
		);
	}
	else {
		if(checkCloudIconExist[model_name])
			set_cloud_router_icon(parent_bg_id, device_id);
		else
			set_default_router_icon(parent_bg_id, device_id);
	}
}
function convRSSI(val) {
	val = parseInt(val);
	if(val >= -50) return 4;
	else if(val >= -80)	return Math.ceil((24 + ((val + 80) * 26)/10)/25);
	else if(val >= -90)	return Math.ceil((((val + 90) * 26)/10)/25);
	else return 1;
}

function reset_re_device(_reMac, _reModelName, _evt) {
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
	var device_info = _reModelName + "<br>";
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
	$amesh_apply.attr({"type" : "button", "value" : "<#CTL_apply#>"});
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
	if(wps_enable_status == "1") {
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
	else {
		var confirm_flag = confirm("<#AiMesh_FindNode_confirm_WPS#>\n<#WiFi_temp_unavailable#>");
		if(confirm_flag) {
			wps_enable_status = "1"
			document.wps_form.submit();
			searchReadyOnBoarding();
		}
	}
}
var aimesh_node_hide_flag = false;
function popAMeshClientListEditTable(event) {
	aimesh_node_hide_flag = false;
	var node_info = event.data.node_info;
	var wl_client = event.data.wl_client;
	var wired_client = event.data.wired_client;
	initial_amesh_obj();

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
	if(checkCloudIconExist[node_info.model_name])
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
	code += "<div class='aimesh_node_setting_info_title'><#AiMesh_Node_ConnPrio#></div>";
	code += "<select id='aimesh_node_connection_priority' class='aimesh_node_input_select'>";
	code += "<option value='3' class='aimesh_node_input_select_option'><#Auto#></option>";
	code += "<option value='2' class='aimesh_node_input_select_option'><#wan_ethernet#></option>";
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
	if(node_info.online == "1")
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
	var alias = "My Home";
	if("config" in node_info) {
		if("misc" in node_info.config) {
			if("cfg_alias" in node_info.config.misc) {
				if(node_info.config.misc.cfg_alias != "")
					alias = node_info.config.misc.cfg_alias;
			}
		}
	}
	var title_name = node_info.model_name + " in " +  alias;
	$popupBgHtml.find("#aimesh_node_title_name").html(title_name);

	if(checkCloudIconExist[node_info.model_name])
		$popupBgHtml.find(".amesh_router_image_web").css('background-image', 'url(' + checkCloudIconExist[node_info.model_name] + ')');

	$popupBgHtml.find("#aimesh_node_ipaddr").html(node_info.ip);
	var labelMac = node_info.mac;
		httpApi.getAiMeshLabelMac(node_info.model_name, node_info.mac, 
			function(_callBackMac){
			labelMac = _callBackMac;
		}
	);
	$popupBgHtml.find("#aimesh_node_macaddr").html(labelMac);

	var location_array = ["Living Room", "Dining Room", "Bedroom", "Office", "Aisle", "Stairwell", "Hall", "Kitchen", "Attic", "Basement", "Yard", "Garage"];

	for(var i = 0; i < location_array.length; i += 1) {
		$popupBgHtml.find("#aimesh_node_location_select").append($('<option>', {
			value: location_array[i],
			text: location_array[i],
			class: "aimesh_node_input_select_option"
		}));
	}

	$popupBgHtml.find("#aimesh_node_location_input").val(alias);

	if($popupBgHtml.find("#aimesh_node_location_select option[value='" + alias + "']").length > 0)
		$popupBgHtml.find("#aimesh_node_location_select").val($popupBgHtml.find("#aimesh_node_location_input").val());
	else
		$popupBgHtml.find("#aimesh_node_location_select").val("");

	var connect_info = {"re_path" : node_info.re_path, "rssi2g" : node_info.rssi2g, "rssi5g" : node_info.rssi5g};
	$popupBgHtml.find("#aimesh_node_connection_type").html(get_connect_type(connect_info).text);
	$popupBgHtml.find(".amesh_interface_icon.static_info").html(get_connect_type(connect_info).icon);

	var backhalctrl_amas_ethernet = "3";
	$popupBgHtml.find("#aimesh_node_connection_priority").attr("disabled", true);
	$popupBgHtml.find("#aimesh_node_connection_priority").css("display", "none");

	if("config" in node_info) {
		if("backhalctrl" in node_info.config) {
			if("amas_ethernet" in node_info.config.backhalctrl) {
				backhalctrl_amas_ethernet = node_info.config.backhalctrl.amas_ethernet;
				$popupBgHtml.find("#aimesh_node_connection_priority").attr("disabled", false);
				$popupBgHtml.find("#aimesh_node_connection_priority").css("display", "");
				$popupBgHtml.find("#aimesh_node_connection_priority_hint").css("display", "none");
			}
		}
	}

	$popupBgHtml.find("#aimesh_node_connection_priority").val(backhalctrl_amas_ethernet);
	$popupBgHtml.find("#aimesh_node_fw_version").html(node_info.fwver.split("-")[0]);
	$popupBgHtml.find("#aimesh_node_moreconfig").html("<#MoreConfig#>");
	/* settup value end */

	/* set event start */
	$popupBgHtml.find("#aimesh_node_close").click(
		function() {
			initial_amesh_obj();
		}
	);
	$popupBgHtml.find("#aimesh_node_location_select").change(
		function() {
			$popupBgHtml.find('#aimesh_node_location_input').val($(this).val());
			$popupBgHtml.find("#aimesh_node_location_hint").css("display", "none");
			$popupBgHtml.find("#aimesh_node_location_hint").val("");
			var data = new Object();
			data.cfg_alias = $popupBgHtml.find("#aimesh_node_location_input").val();
			var title_name = node_info.model_name + " in " +  $popupBgHtml.find("#aimesh_node_location_input").val();
			$popupBgHtml.find("#aimesh_node_title_name").html(title_name);
			set_AiMesh_node_config(data, node_info.mac)
		}
	);
	$popupBgHtml.find("#aimesh_node_location_input").blur(
		function() {
			var validAiMeshLocation = function() {
				var location = $popupBgHtml.find("#aimesh_node_location_input").val();
				if(location.length == 0){
					$popupBgHtml.find("#aimesh_node_location_hint").css("display", "block");
					$popupBgHtml.find("#aimesh_node_location_hint").html("<#File_Pop_content_alert_desc1#>");
					$popupBgHtml.find("#aimesh_node_location_input").focus();
					$popupBgHtml.find("#aimesh_node_location_input").select();
					return false;
				}
				else if(!parent.validator.haveFullWidthChar($popupBgHtml.find("#aimesh_node_location_input")[0])) {
					$popupBgHtml.find("#aimesh_node_location_hint").css("display", "block");
					$popupBgHtml.find("#aimesh_node_location_hint").html("<#JS_validchar#>");
					$popupBgHtml.find("#aimesh_node_location_input").focus();
					$popupBgHtml.find("#aimesh_node_location_input").select();
					return false;
				}
				$popupBgHtml.find("#aimesh_node_location_hint").css("display", "none");
				$popupBgHtml.find("#aimesh_node_location_hint").val("");
				return true;
			};
			if(validAiMeshLocation()) {
				if($popupBgHtml.find("#aimesh_node_location_select option[value='" + $(this).val() + "']").length > 0)
					$popupBgHtml.find("#aimesh_node_location_select").val($(this).val());
				else
					$popupBgHtml.find("#aimesh_node_location_select").val("");
				var data = new Object();
				data.cfg_alias = $popupBgHtml.find("#aimesh_node_location_input").val();
				var title_name = "AiMesh node in " + $popupBgHtml.find("#aimesh_node_location_input").val();
				$popupBgHtml.find("#aimesh_node_title_name").html(title_name);
				set_AiMesh_node_config(data, node_info.mac);
			}
		}
	);
	$popupBgHtml.find("#aimesh_node_connection_type").click(
		function() {
			show_change_type_hint();
		}
	);
	if($popupBgHtml.find("#aimesh_node_connection_priority").css("display") != "none") {
		$popupBgHtml.find("#aimesh_node_connection_priority").change(
			function() {
				var data = new Object();
				data.amas_ethernet = $popupBgHtml.find("#aimesh_node_connection_priority").val();
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
	);
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
			case 1:
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
			case 2:
				var led_status = (led_status.value == "0") ? 1 : 0;
				parent.$("#edit_amesh_client_block").find('#led_radio').iphoneSwitch(led_status,
					function(){
						var data = new Object();
						data.lp55xx_lp5523_user_enable = 0;
						data.lp55xx_lp5523_user_col = 0;
						data.lp55xx_lp5523_user_beh = 0;
						set_AiMesh_node_config(data, node_info.mac);
					},
					function(){
						var data = new Object();
						data.lp55xx_lp5523_user_enable = 1;
						data.lp55xx_lp5523_user_col = 101;
						data.lp55xx_lp5523_user_beh = 300;
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

				var getNodeStatus = function(_jsonArray, _idxArray, item) {
					var status = 0;
					for (var idx in _jsonArray) {
						if (_jsonArray[idx].hasOwnProperty(_idxArray[0])) {
							if(_jsonArray[idx][_idxArray[0]] == _idxArray[1]) {
								if (_jsonArray[idx].hasOwnProperty(item)) {
									status = _jsonArray[idx][item];
									break;
								}
							}
						}
					}
					return status;
				};
				var node_online = getNodeStatus(get_cfg_clientlist, ["mac", _nodeMac], "online");
				var node_model_name = getNodeStatus(get_cfg_clientlist, ["mac", _nodeMac], "model_name");
				var node_re_path = getNodeStatus(get_cfg_clientlist, ["mac", _nodeMac], "re_path");
				var node_rssi2g = getNodeStatus(get_cfg_clientlist, ["mac", _nodeMac], "rssi2g");
				var node_rssi5g = getNodeStatus(get_cfg_clientlist, ["mac", _nodeMac], "rssi5g");
				var wired_client = get_wiredclientlist[_nodeMac];
				var wl_client = get_wclientlist[_nodeMac];
				aimesh_node_client_list = [];
				if(node_online == "1")
					aimesh_node_client_list = get_aimesh_node_client_list(wired_client, wl_client, _nodeMac);
				aimesh_node_client_list = sorterApi.sortJson(aimesh_node_client_list, sorterApi.key, sorterApi.type);
				parent.$("#edit_amesh_client_block").children().find("#aimesh_node_client_header_tr").after(gen_AiMesh_node_client(aimesh_node_client_list));

				if(checkCloudIconExist[node_model_name]) {
					parent.$("#edit_amesh_client_block .amesh_router_icon.card").css('background-image', 'url(' + checkCloudIconExist[node_model_name] + ')');
					parent.$("#edit_amesh_client_block .amesh_router_icon.card").addClass('amesh_router_image_web');
					parent.$("#edit_amesh_client_block .amesh_router_icon.card").removeClass('amesh_router_icon');
				}

				var connect_info = {"re_path" : node_re_path, "rssi2g" : node_rssi2g, "rssi5g" : node_rssi5g};
				parent.$("#edit_amesh_client_block #aimesh_node_connection_type").html(get_connect_type(connect_info).text);
				parent.$("#edit_amesh_client_block .amesh_interface_icon.static_info").html(get_connect_type(connect_info).icon);
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
				if(nodeClientObj.type == "0") {
					icon_type = "type0_viewMode";
				}
				nodeClientHtml += "<div class='clientIcon_no_hover " + icon_type + " aimesh_node_client_icon_default'></div>";
			}
			else if(nodeClientObj.vendor != "" ) {
				var venderIconClassName = getVenderIconClassName(nodeClientObj.vendor.toLowerCase());
				if(venderIconClassName != "" && !downsize_4m_support) {
					nodeClientHtml += "<div class='venderIcon_no_hover " + venderIconClassName + " aimesh_node_client_icon_vendor'></div>";
				}
				else {
					var icon_type = "type" + nodeClientObj.type;
					if(nodeClientObj.type == "0") {
						icon_type = "type0_viewMode";
					}
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
				rssi = convRSSI(nodeClientObj.rssi);

			nodeClientHtml += "<td width='" + aimesh_node_client_info_width[3] + "' align='center'>";
			nodeClientHtml += "<div style='margin: auto;' class='radioIcon radio_" + rssi + "'></div>";
			if(nodeClientObj.isWL != "0") {
				var wireless_band_array = ["", "2.4 G", "5 G", "5 G-2"];
				var bandClass = (navigator.userAgent.toUpperCase().match(/CHROME\/([\d.]+)/)) ? "band_txt_chrome" : "band_txt";
				nodeClientHtml += "<div class='band_block' style='margin: auto;'><span class=" + bandClass + " style='color: #000000;'>" + wireless_band_array[nodeClientObj.isWL] + "</span></div>";
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
	}, ajax_onboarding);
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
	if(_node_info.re_path == "1") {
		component.text = "<#tm_wired#>";
		component.icon = "<div class='radioIcon radio_wired'></div>";
	}
	else {
		component.text = "<#tm_wireless#>";
		var bandClass = (navigator.userAgent.toUpperCase().match(/CHROME\/([\d.]+)/)) ? "band_txt_chrome" : "band_txt";
		var wireless_band = 0;
		var wireless_band_array = ["2.4 G", "5 G"];
		var wireless_rssi = 4;
		if(_node_info.re_path == "2") {
			wireless_band = 0;
			wireless_rssi = convRSSI(_node_info.rssi2g);
		}
		else {
			wireless_band = 1;
			wireless_rssi = convRSSI(_node_info.rssi5g);
		}
		component.icon = "<div class='radioIcon radio_" + wireless_rssi +"'></div>";
		component.icon += "<div class='band_block'><span class=" + bandClass + ">" + wireless_band_array[wireless_band]  + "</span></div>";
	}
	return component;
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
<iframe name="wpsFrame" id="wpsFrame" src="" width="0" height="0" frameborder="0" scrolling="no"></iframe>
<form method="post" name="wps_form" id="wps_form" action="/start_apply.htm" target="wpsFrame">
<input type="hidden" name="productid" value="<% nvram_get("productid"); %>">
<input type="hidden" name="action_mode" value="apply_new">
<input type="hidden" name="action_script" value="restart_wireless">
<input type="hidden" name="action_wait" value="3">
<input type="hidden" name="current_page" value="device-map/amesh.asp">
<input type="hidden" name="next_page" value="device-map/amesh.asp">
<input type="hidden" name="wps_enable" value='1'>
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
