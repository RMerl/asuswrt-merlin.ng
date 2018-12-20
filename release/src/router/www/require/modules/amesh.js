function updateAMeshCount() {
	if(lastName != "iconAMesh") {
		$.ajax({
			url: '/ajax_onboarding.asp',
			dataType: 'script', 
			success: function(response) {
				var get_cfg_clientlist_num = 0;
				if(get_cfg_clientlist.length > 1) {
					for (var idx in get_cfg_clientlist) {
						if(get_cfg_clientlist.hasOwnProperty(idx)) {
							if(idx != 0) {
								get_cfg_clientlist_num++;
							}
						}
					}
					show_AMesh_status(get_cfg_clientlist_num, 1);
				}
				else 
					show_AMesh_status(0, 1);
			}
		});
	}
}
function show_AMesh_status(num, flag) {
	document.getElementById("ameshNumber").innerHTML = "<#AiMesh_Node#>: <span>" + num + "</span>";
}
function initial_amesh_obj() {
	//initial amesh obj
	if($('link[rel=stylesheet][href~="/device-map/amesh.css"]').length == 1) {
		$('link[rel=stylesheet][href~="/device-map/amesh.css"]').remove();
	}
	if($('.amesh_popup_bg').length > 0) {
		$('.amesh_popup_bg').remove();
	}
}
function check_wl_auth_support(_wl_auth_mode_x, _obj) {
	var support_flag = false;
	var support_auth = ["psk2", "pskpsk2"];
	for (var idx in support_auth) {
		if (support_auth.hasOwnProperty(idx)) {
			if(_wl_auth_mode_x == support_auth[idx]) {
				support_flag = true;
				break;
			}
		}
	}
	if(!support_flag) {
		var auth_text = _obj.text();
		var confirm_msg = "When using " + auth_text + " Authentication, AiMesh system will become invalid.\nAre you sure to process?";/*untranslated*/
		support_flag = confirm(confirm_msg);
	}
	return support_flag;
}
function AiMesh_confirm_msg(_name, _value) {
	var check_operation_mode = function(_value) {
		switch(parseInt(_value)) {
			case 2 :
				return confirm("If you change to Repeater mode, it will disable AiMesh system.\nAre you sure to process?");/* untranslated */
				break;
			case 4 :
				return confirm("If you change to Media Bridge mode, it will disable AiMesh system.\nAre you sure to process?");/* untranslated */
				break;
			default :
				return true;
		}
	};
	var check_feature_status = function(_obj, _value) {
		if(_obj.value != _value)
			return confirm(_obj.text);
		else
			return true;
	};
	var check_wireless_ssid_psk = function(_value) {
		var current_ssid = _value["current"]["ssid"];
		var current_psk = _value["current"]["psk"];
		var original_ssid = _value["original"]["ssid"];
		var original_psk = _value["original"]["psk"];
		var current_node_count = [<% get_cfg_clientlist(); %>][0].length - 1;
		var total_node_count = [<% get_onboardingstatus(); %>][0]["cfg_obcount"];
		if(total_node_count != "" && current_node_count < total_node_count) {
			if(current_ssid == original_ssid && current_psk == original_psk)
				return true;
			else if(current_ssid != original_ssid && current_psk != original_psk)
				return confirm("If you change SSID and WPA Pre-Shared Key, it will affect the offline AiMesh node wifi connectivity.\nAre you sure to process?");/* untranslated */
			else if(current_ssid != original_ssid)
				return confirm("If you change SSID, it will affect the offline AiMesh node wifi connectivity.\nAre you sure to process?");/* untranslated */
			else if(current_psk != original_psk)
				return confirm("If you change WPA Pre-Shared Key, it will affect the offline AiMesh node wifi connectivity.\nAre you sure to process?");/* untranslated */
		}
		else
			return true;
	};
	var check_wireless_country_code = function() {
		return confirm("By changing country code, AiMesh might not work properly.\nAre you sure to process?");/* untranslated */
	};
	var feature_value = {
		"Wireless_Radio" : {
			"value" : 1,
			"text" : "If you disable Radio, it will affect the AiMesh wifi connectivity.\nAre you sure to process?" /* untranslated */
		},
		"Wireless_Hide" : {
			"value" : 0,
			"text" : "If you Hide SSID, it will affect the AiMesh wifi connectivity.\nAre you sure to process?" /* untranslated */
		},
		"Wireless_Hide_WPS" : {
			"value" : 0,
			"text" : "If you Hide SSID, it will disable the WPS function and affect the AiMesh wifi connectivity.\nAre you sure to process?" /* untranslated */
		},
		"DHCP_Server" : {
			"value" : 1,
			"text" : "If you disable DHCP Server, it will affect the AiMesh wifi connectivity.\nAre you sure to process?" /* untranslated */
		}
	};
	var confirm_flag = true;
	switch(_name) {
		case "Operation_Mode" :
			confirm_flag = check_operation_mode(_value);
			break;
		case "Wireless_Radio" :
		case "Wireless_Hide" :
		case "Wireless_Hide_WPS" :
		case "DHCP_Server" :
			confirm_flag = check_feature_status(feature_value[_name], _value);
			break;
		case "Wireless_SSID_PSK" :
			confirm_flag = check_wireless_ssid_psk(_value);
			break;
		case "Wireless_CountryCode" :
			confirm_flag = check_wireless_country_code();
			break;
	}
	return confirm_flag;
}
