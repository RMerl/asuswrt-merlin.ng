var apply = {};

apply.welcome = function(){
	if($("#tosCheckbox").is(":visible")){
		if(systemVariable.isDefault && !$("#tosCheckbox").prop("checked") && isSupport("ForceBWDPI")){
			$("html, body").animate({ scrollTop: $(document).height()}, 800);
			$(".tos").fadeOut(500).fadeIn(500).fadeOut(500).fadeIn(500);
			return false;
		}
	}

	systemVariable.advSetting = false;

	if(!systemVariable.forceChangePw){
		if(isOriginSwMode("RP")){
			goTo.rpMode();
		}
		else if(isOriginSwMode("AP")){
			goTo.apMode();
		}
		else if(isOriginSwMode("MB")){
			goTo.mbMode();
		}
		else if(isOriginSwMode("Mesh")){
			goTo.meshMode();
		}
		else{
			goTo.autoWan();
		}
	}
	else{
		goTo.Login();
	}
};

apply.upload = function(){
	var $settingPofile = $("#setting_profile").val();
	var $settingPofile_name = $settingPofile.toUpperCase();

	if($settingPofile_name == ""){
		$("#settingProfileBar").showSelectorHint("<#JS_fieldblank#>");
		return false;
	}
	
	if( 
		$settingPofile_name.length < 6 ||
		$settingPofile_name.lastIndexOf(".CFG")  < 0 || 
		$settingPofile_name.lastIndexOf(".CFG") != ($settingPofile_name.length)-4)
	{
		$("#settingProfileBar").showSelectorHint("<#Setting_upload_hint#>");
		return false;
	}

	httpApi.uploadFile($("#setting_profile").prop('files')[0]);
	goTo.Upload();
}

apply.changeOpMode = function(){
	postDataModel.insert(opModeObj);

	if(!systemVariable.forceChangePw)
		goTo.opMode();
	else
		goTo.Login();
}

apply.login = function(){
	var isValidInputs = function(){
		String.prototype.getTrimString = function(){
			var tmpString = this + '';
			for (var startIndex=0;startIndex<tmpString.length && tmpString.substring(startIndex,startIndex+1) == ' ';startIndex++);
			for (var endIndex=tmpString.length-1; endIndex>startIndex && tmpString.substring(endIndex,endIndex+1) == ' ';endIndex--);
			return tmpString.substring(startIndex,endIndex+1);
		};

		var httpUserInput = $("#http_username");
		var httpPassInput = $("#http_passwd");
		var httpPassConfirmInput = $("#http_passwd_confirm");

		if(hasBlank([httpUserInput, httpPassInput, httpPassConfirmInput])) return false;

		/* check user name */
		var isValidHostName = validator.hostNameString(httpUserInput.val())
		if(isValidHostName.isError){
			httpUserInput.showTextHint(isValidHostName.errReason);
			return false;
		}

		if(httpUserInput.val() == "root" || httpUserInput.val() == "guest" || httpUserInput.val() == "anonymous"){
			httpUserInput.showTextHint("<#USB_Application_account_alert#>");
			return false;
		}

		/* check user name */
		if(isSku("KR")){
			var isValidKRSkuPwd = validator.KRSkuPwd(httpPassInput.val())
			if(isValidKRSkuPwd.isError){
				httpPassInput.showTextHint(isValidKRSkuPwd.errReason);
				return false;
			}
		}

		if(httpPassInput.val() != httpPassConfirmInput.val()){
			httpPassInput.showTextHint("<#File_Pop_content_alert_desc7#>");
			return false;
		}

		if(httpPassInput.val() == systemVariable.default_http_passwd){
			httpPassInput.showTextHint("<#QIS_adminpass_confirm0#>");
			return false;
		}	
		else if(httpPassInput.val().length < 5){
			httpPassInput.showTextHint("<#JS_short_password#>");
			return false;
		}

		var isValidChar = validator.invalidChar(httpPassInput.val())
		if(isValidChar.isError){
			httpPassInput.showTextHint(isValidChar.errReason);
			return false;
		}	

		if(isWeakString(httpPassInput.val(), "httpd_password")){
			if(!confirm("<#JS_common_passwd#>")){
				httpPassInput.showTextHint("<#AiProtection_scan_note11#>");
				return false;
			}
		}

		return true;
	}

	if(isValidInputs()){
		qisPostData.http_username = $("#http_username").val();
		qisPostData.http_passwd = $("#http_passwd").val();

		if(systemVariable.forceChangePwInTheEnd){
			if(isSwMode("RP"))
				transformWLToGuest();

			if( (isSwMode("RP") && !isSupport("concurrep")) || isSwMode("MB") )
				transformWLCObj();

			$(".btn_login_apply").html(Get_Component_btnLoading);
			apply.submitQIS();
		}
		else{
			if(qisPostData.hasOwnProperty("sw_mode")){
				goTo.opMode();
			}
			else if(isOriginSwMode("RP")){
				goTo.rpMode();
			}
			else if(isOriginSwMode("AP")){
				goTo.apMode();
			}
			else if(isOriginSwMode("MB")){
				goTo.mbMode();
			}
			else if(isOriginSwMode("Mesh")){
				goTo.meshMode();
			}
			else{
				goTo.autoWan();
			}
		}
	}
};

apply.manual = function(){
	systemVariable.manualWanSetup = true;
	goTo.WAN()
};

apply.dhcp = function(){
	if($("#iptv_checkbox").is(":checked")){
		goTo.IPTV();
	}
	else if(hadPlugged("modem")){
		goTo.Modem();
	}
	else{
		if(isWANChanged()){
			httpApi.nvramSet((function(){
				qisPostData.action_mode = "apply";
				qisPostData.rc_service = "restart_wan_if 0";
				return qisPostData;
			})());

			updateOriginWan();
		}

		goTo.Wireless();
	}
};

apply.pppoe = function(){
	if(qisPostData.wan_proto === "l2tp" || qisPostData.wan_proto === "pptp"){
		if(hasBlank([$("#wan_pppoe_username"), $("#wan_pppoe_passwd"), $("#wan_heartbeat_x")])) return false;

		qisPostData.wan_pppoe_username = $("#wan_pppoe_username").val();
		qisPostData.wan_pppoe_passwd = $("#wan_pppoe_passwd").val();
		qisPostData.wan_heartbeat_x = $("#wan_heartbeat_x").val();

		goTo.GetIp();
	}
	else{
		if(qisPostData.hasOwnProperty("wan_pppoe_username")){
			if(hasBlank([$("#wan_pppoe_username"), $("#wan_pppoe_passwd")])) return false;		
			qisPostData.wan_pppoe_username = $("#wan_pppoe_username").val();
			qisPostData.wan_pppoe_passwd = $("#wan_pppoe_passwd").val();
		}

		if($("#iptv_checkbox").is(":checked")){
			goTo.IPTV();
		}
		else if(hadPlugged("modem")){
			goTo.Modem();
		}
		else{
			if(isWANChanged() || window.pppAuthFailChecked){
				httpApi.nvramSet((function(){
					qisPostData.action_mode = "apply";
					qisPostData.rc_service = "restart_wan_if 0";
					return qisPostData;
				})());

				updateOriginWan();
			}
	
			goTo.Wireless();
		}
	}
};

apply.static = function(){
	if(qisPostData.hasOwnProperty("wan_ipaddr_x")){
		if(hasBlank([
			$("#static_ipaddr"), 
			$("#static_subnet"), 
			$("#static_gateway"),
			$("#static_dns1")
		])) return false;

		if($("#static_ipaddr").val() == $("#static_gateway").val()){
			$("#static_ipaddr").showTextHint("<#IPConnection_warning_WANIPEQUALGatewayIP#>");
			return false;
		}

		qisPostData.wan_dhcpenable_x = "0";
		qisPostData.wan_ipaddr_x = $("#static_ipaddr").val()
		qisPostData.wan_netmask_x = $("#static_subnet").val()
		qisPostData.wan_gateway_x = $("#static_gateway").val()
		qisPostData.wan_dnsenable_x = "0";
		qisPostData.wan_dns1_x = $("#static_dns1").val();
		qisPostData.wan_dns2_x = $("#static_dns2").val();
	}

	if($("#iptv_checkbox").is(":checked")){
		goTo.IPTV();
	}
	else if(hadPlugged("modem")){
		goTo.Modem();
	}
	else{
		if(isWANChanged()){
			httpApi.nvramSet((function(){
				qisPostData.action_mode = "apply";
				qisPostData.rc_service = "restart_wan_if 0";
				return qisPostData;
			})());

			updateOriginWan();
		}
	
		goTo.Wireless();
	}
};

apply.iptv = function(){
	if(qisPostData.switch_wantag == "movistar"){
		if(hasBlank([
			$("#iptv_ipaddr"), 
			$("#iptv_netmask"), 
			$("#iptv_gateway"),
			$("#iptv_dns1")
		])) return false;

		qisPostData.wan10_ipaddr_x = $("#iptv_ipaddr").val();
		qisPostData.wan10_netmask_x = $("#iptv_netmask").val();
		qisPostData.wan10_gateway_x = $("#iptv_gateway").val();
		qisPostData.wan10_dns1_x = $("#iptv_dns1").val();
		qisPostData.wan10_dns2_x = $("#iptv_dns2").val();
		qisPostData.wan10_auth_x = $("#iptv_auth_mode").val();
		qisPostData.wan10_pppoe_username = $("#iptv_pppoe_username").val();
		qisPostData.wan10_pppoe_passwd = $("#iptv_pppoe_passwd").val();
	}
	else if(qisPostData.switch_wantag == "manual"){
		if(hasBlank([
			$("#internet_vid"), 
			$("#internet_prio"), 
			$("#stb_vid"),
			$("#stb_prio"),
			$("#voip_vid"),
			$("#voip_prio")
		])) return false;

		qisPostData.switch_wan0tagid = $("#internet_vid").val();
		qisPostData.switch_wan0prio = $("#internet_prio").val();
		qisPostData.switch_wan1tagid = $("#stb_vid").val();
		qisPostData.switch_wan1prio = $("#stb_prio").val();
		qisPostData.switch_wan2tagid = $("#voip_vid").val();
		qisPostData.switch_wan2prio = $("#voip_prio").val();
	}

	if(hadPlugged("modem")){
		goTo.Modem();
	}
	else{
		goTo.Wireless();
	}
};

apply.modem = function(){
	if($("#modem_autoapn").val() == "0"){
		if(hasBlank([
			$("#modem_dialnum")
		])) return false;	
	}

	qisPostData.wans_dualwan = "wan usb";
	qisPostData.modem_enable = $("#modem_enable").val();
	qisPostData.modem_android = $("#modem_android").val();
	qisPostData.modem_autoapn = $("#modem_autoapn").val();
	qisPostData.modem_country = $("#modem_country").val();
	qisPostData.modem_isp = $("#modem_isp").val();
	qisPostData.modem_apn = $("#modem_apn").val();
	qisPostData.modem_dialnum = $("#modem_dialnum").val();
	qisPostData.modem_pincode = $("#modem_pincode").val();
	qisPostData.modem_user = $("#modem_user").val();
	qisPostData.modem_pass = $("#modem_pass").val();
	qisPostData.modem_ttlsid = $("#modem_ttlsid").val();
	qisPostData.modem_authmode = $("#modem_authmode").val();
	qisPostData.modem_mtu = $("#modem_mtu").val();
	qisPostData.Dev3G = $("#Dev3G").val();

	goTo.Wireless();
};

apply.lanStatic = function(){
	if(qisPostData.hasOwnProperty("lan_proto")){
		if(hasBlank([
			$("#static_lan_ipaddr"), 
			$("#static_lan_subnet"), 
			$("#static_lan_gateway"),
			$("#static_lan_dns1")
		])) return false;

		qisPostData.lan_proto = "static";
		qisPostData.lan_ipaddr = $("#static_lan_ipaddr").val()
		qisPostData.lan_netmask = $("#static_lan_subnet").val()
		qisPostData.lan_gateway = $("#static_lan_gateway").val()
		qisPostData.lan_dnsenable_x = "0";
		qisPostData.lan_dns1_x = $("#static_lan_dns1").val();
		qisPostData.lan_dns2_x = $("#static_lan_dns2").val();
	}
	
	if(isSwMode("MB")){
		postDataModel.insert(generalObj);

		if(systemVariable.forceChangePwInTheEnd){
			goTo.changePwInTheEnd();
		}
		else{
			transformWLCObj();

			httpApi.nvramSet((function(){
				qisPostData.action_mode = "apply";
				qisPostData.rc_service = getRestartService();
				return qisPostData;
			})(), goTo.Finish);
		}
	}
	else{
		goTo.Wireless();
	}
};

apply.wlcKey = function(){
	var unit = "";
	var isManual = $("#wlc_ssid_manual").is(":visible");
	var isWepAuthMode = false;
	if(isManual){
		unit = $("#wlc_band_manual").val().toString();
		if(hasBlank([$("#wlc_ssid_manual")])) return false;
		if(!validator.stringSSID(document.getElementById("wlc_ssid_manual"))) return false;

		qisPostData["wlc" + unit + "_band"] = $("#wlc_band_manual").val();
		qisPostData["wlc" + unit + "_ssid"] = $("#wlc_ssid_manual").val();

		qisPostData["wlc" + unit + "_auth_mode"] = "";
		qisPostData["wlc" + unit + "_crypto"] = "";
		qisPostData["wlc" + unit + "_wpa_psk"] = "";
		qisPostData["wlc" + unit + "_wep"] = "";
		qisPostData["wlc" + unit + "_wep_key"] = "";
		qisPostData["wlc" + unit + "_key"] = "";

		var auth_mode = $("#wlc_auth_mode_manual").val();
		qisPostData["wlc" + unit + "_auth_mode"] = auth_mode;
		var wep = $("#wlc_wep_manual").val();
		if(auth_mode == "open"){
			qisPostData["wlc" + unit + "_wep"] = wep;
			if(wep != "0"){
				isWepAuthMode = true;
			}
		}
		else if(auth_mode == "shared"){
			qisPostData["wlc" + unit + "_wep"] = wep;
			isWepAuthMode = true;
		}
		else if(auth_mode == "psk" || auth_mode == "psk2"){
			qisPostData["wlc" + unit + "_crypto"] = $("#wlc_crypto_manual").val();
		}
	}
	else{
		unit = systemVariable.selectedAP.unit.toString();
		isWepAuthMode = (qisPostData["wlc" + unit + "_auth_mode"] == "open" && qisPostData["wlc" + unit + "_wep"] == "1");
	}

	if($("#wlc_wifiKey").is(":visible")){
		if(hasBlank([$("#wlc_wifiKey")])) return false;

		if(isWepAuthMode){
			if(checkWepKey($("#wlc_wifiKey"), wep)) return false;
			var wepKey = $("#wlc_wifiKey").val();
			if(isManual){
				qisPostData["wlc" + unit + "_key"] = $("#wlc_key_index_manual").val();
				qisPostData["wlc" + unit + "_wep"] = wep;
			}
			else{
				qisPostData["wlc" + unit + "_key"] = 1;
				qisPostData["wlc" + unit + "_wep"] = (wepKey.length < 11) ? "1" : "2";
			}
			qisPostData["wlc" + unit + "_wep_key"] = wepKey;
		}
		else{
			if(!validator.psk(document.getElementById("wlc_wifiKey"))) return false;
			qisPostData["wlc" + unit + "_wpa_psk"] = $("#wlc_wifiKey").val();
		}
	}

	systemVariable.multiPAP.wlcOrder.push(unit);
	systemVariable.multiPAP.wlcStatus["wlc" + unit + "_checked"] = true;
	if(isManual)
		systemVariable.multiPAP.wlcStatus["wlc" + unit + "_manual"] = true;

	goTo.lanIP_papList();
}

apply.wireless = function(){
	var wirelessValidator = function(band){
		if(hasBlank([$("#wireless_ssid_" + band), $("#wireless_key_" + band)])) return false;
		if(!validator.stringSSID(document.getElementById("wireless_ssid_" + band))) return false;
		if(!validator.psk(document.getElementById("wireless_key_" + band))) return false;	
		if(isWeakString($("#wireless_key_" + band).val(), "wpa_key")){
			if(!confirm("<#JS_common_passwd#>")){
				$("#wireless_key_" + band).showTextHint("<#AiProtection_scan_note11#>"); 
				return false;
			}
		}

		return true;
	}

	if(qisPostData.hasOwnProperty("wl0_ssid")){
		if($("#wireless_ssid_0").length){if(!wirelessValidator(0)) return false;}

		qisPostData.wl0_ssid = $("#wireless_ssid_0").val();
		qisPostData.wl0_wpa_psk = $("#wireless_key_0").val();
		qisPostData.wl0_auth_mode_x = "psk2";
		qisPostData.wl0_crypto = "aes";
	}

	if(qisPostData.hasOwnProperty("wl1_ssid")){
		if($("#wireless_ssid_1").length){if(!wirelessValidator(1)) return false;}

		qisPostData.wl1_ssid = ($("#wireless_ssid_1").length) ? $("#wireless_ssid_1").val() : qisPostData.wl0_ssid;
		qisPostData.wl1_wpa_psk = ($("#wireless_key_1").length) ? $("#wireless_key_1").val() : qisPostData.wl0_wpa_psk;
		qisPostData.wl1_auth_mode_x = "psk2";
		qisPostData.wl1_crypto = "aes";
	}

	if(qisPostData.hasOwnProperty("wl2_ssid")){
		if($("#wireless_ssid_2").length){if(!wirelessValidator(2)) return false;}

		qisPostData.wl2_ssid = ($("#wireless_ssid_2").length) ? $("#wireless_ssid_2").val() : qisPostData.wl0_ssid;
		qisPostData.wl2_wpa_psk = ($("#wireless_key_2").length) ? $("#wireless_key_2").val() : qisPostData.wl0_wpa_psk;
		qisPostData.wl2_auth_mode_x = "psk2";
		qisPostData.wl2_crypto = "aes";
	}

	if(systemVariable.forceChangePwInTheEnd){
		goTo.changePwInTheEnd();
	}
	else{
		if(isSwMode("RP")) {
			transformWLToGuest();
			if(!isSupport("concurrep"))
				transformWLCObj();
		}

		$(".btn_wireless_apply").html(Get_Component_btnLoading);
		apply.submitQIS();
	}
};

apply.submitQIS = function(){
	function updateChanges(){
		if(systemVariable.isNewFw == 0){
			setTimeout(function(){
				if(isSupport("lantiq")){
					/* Make sure BLUECAVE had updated wireless setting. */
					var waveReady = httpApi.nvramGet(["wave_ready"], true).wave_ready;
					if(waveReady == "0"){
						setTimeout(arguments.callee, 1000);
						return false;
					}
				}

				httpApi.nvramSet((function(){
					qisPostData.action_mode = "apply";
					if(!(isSupport("yadns") && isSwMode("RT")))
						qisPostData.rc_service = getRestartService();
					return qisPostData;
				})(), (isSupport("yadns") && isSwMode("RT")) ? goTo.Yadns : goTo.Finish);
			}, 500);
		}
		else{
			httpApi.nvramSet((function(){
				qisPostData.action_mode = "apply";
				return qisPostData;
			})(), (isSupport("yadns") && isSwMode("RT")) ? goTo.Yadns : goTo.Update);
		}
	}

	var linkInternet = httpApi.isConnected();
	var pppoeAuthFail = httpApi.isPppAuthFail();

	if(pppoeAuthFail){
		$(".btn_wireless_apply").html("<#CTL_apply#>");
		$(".btn_login_apply").html("<#CTL_apply#>");
		$("#wan_pppoe_passwd").showTextHint("<#QKSet_Internet_Setup_fail_reason2#>");
		goTo.loadPage("pppoe_setting", true);
	}
	else if(linkInternet && isSupport("fupgrade")){
		var errCount = 0;

		setTimeout(function(){
			if(systemVariable.newFwVersion == "" && errCount < 10){
				errCount++;
				setTimeout(arguments.callee, 1000);
			}
			else{
				updateChanges();
			}
		}, 500);
	}
	else{
		updateChanges();
	}
}

apply.update = function(){
	httpApi.nvramSet({"action_mode": "apply", "rc_service":"start_webs_upgrade"}, goTo.Upgrading);
};

apply.amasNode = function(){
	systemVariable.meshRole = "meshNode";

	if(!systemVariable.isDefault){
		if(confirm("<#AiMesh_restore_desc_confirm#>")){
			goTo.amasRestore();
		}
	}
	else{
		goTo.Conncap();
	}
};

apply.amasbundle = function(){
	location.href = "/cfg_onboarding.cgi?flag=AMesh&id=donot_search";
};

apply.yadnsEnable = function(){
	postDataModel.insert(yandexObj);
	qisPostData.yadns_enable_x = "1";
	qisPostData.yadns_mode = "0";
	apply.yadnsSetting();
};

apply.yadnsSafe = function(){
	postDataModel.insert(yandexObj);
	qisPostData.yadns_enable_x = "1";
	qisPostData.yadns_mode = "1";
	apply.yadnsSetting();
};

apply.yadnsDisable = function(){
	postDataModel.insert(yandexObj);
	qisPostData.yadns_enable_x = "0";
	qisPostData.yadns_mode = "0";
	apply.yadnsSetting();
};
apply.yadnsSetting = function(){
	httpApi.nvramSet((function(){
		qisPostData.action_mode = "apply";
		qisPostData.rc_service = "restart_yadns";
		return qisPostData;
	})(), (systemVariable.isNewFw == 0) ? goTo.Finish : goTo.Update);
};

var abort = {};

abort.backToStartQIS = function(){
	if(systemVariable.meshRole){
		goTo.loadPage("amasrole_page", true);
	}
	else{
		goTo.loadPage("welcome", true);
	}
}

abort.advSetting = function(){
	goTo.loadPage("welcome", true);
}

abort.chooseRole = function(){
	delete systemVariable.meshRole;

	if(location.search == "?flag=amasrole_page"){
		window.history.back();
	}
	else{
		goTo.loadPage("amasintro_page", true);
	}
}

abort.login = function(){
	postDataModel.remove(userObj);

	if(systemVariable.forceChangePwInTheEnd){
		postDataModel.remove(userObj);
		if(isSwMode("MB")){
			if(qisPostData.lan_proto === "dhcp")
				goTo.loadPage("getLanIp_setting", true);
			else
				goTo.loadPage("lanStatic_setting", true);
		}
		else
			goTo.loadPage("wireless_setting", true);
	}
	else{
		if(qisPostData.hasOwnProperty("sw_mode")){
			postDataModel.remove(opModeObj);
			goTo.loadPage("advanced_setting", true);
		}
		else{
			abort.backToStartQIS();
		}
	}
};

abort.eula = function(){
	goTo.loadPage("welcome", true);
};

abort.opMode = function(){
	if(location.search == "?flag=manual"){
		window.history.back();
	}
	else{
		if(!systemVariable.forceChangePw){
			postDataModel.remove(opModeObj);
			goTo.loadPage("advanced_setting", true);
		}
		else{
			goTo.loadPage("login_name", true);	
		}
	}
}

abort.resetModem = function(){
	if(systemVariable.detwanResult.wanType === "RESETMODEM") httpApi.startAutoDet();
	systemVariable.manualWanSetup = false;

	setTimeout(function(){
		if(!isPage("waiting_page")) return false;

		if(systemVariable.detwanResult.wanType == "" || systemVariable.detwanResult.wanType == "CHECKING"){
			systemVariable.detwanResult = httpApi.detwanGetRet();
			if(isPage("waiting_page")) setTimeout(arguments.callee, 1000);
			return false;
		}

		if(!systemVariable.manualWanSetup) goTo.autoWan();	
	}, 2000);

	goTo.loadPage("waiting_page", true);
};

abort.wan = function(){
	goTo.loadPage("accountPrompt_setting", true);
}

abort.wanType = function(){
	postDataModel.remove(wanObj.all);
	delete systemVariable.manualWanType;

	if(qisPostData.hasOwnProperty("sw_mode") && !systemVariable.meshRole){
		goTo.loadPage("opMode_page", true);
	}
	else if(systemVariable.detwanResult.wanType == "NOWAN"){
		systemVariable.manualWanSetup = false;

		if(isSupport("modem")){
			setTimeout(function(){
				if(!isPage("noWan_page")) return false;

				if($("#noWanEth").is(":visible")){
					$("#noWanEth").fadeOut(500);	
					setTimeout(function(){$("#noWanUsb").fadeIn(500);}, 500);	
				}
				else{
					$("#noWanUsb").fadeOut(500);
					setTimeout(function(){$("#noWanEth").fadeIn(500);}, 500);	
				}

				if(isPage("noWan_page")) setTimeout(arguments.callee, 2000);
			}, 1)
		}

		setTimeout(function(){
			systemVariable.detwanResult = httpApi.detwanGetRet();

			switch(systemVariable.detwanResult.wanType){
				case "CONNECTED":
					if(systemVariable.manualWanSetup) goTo.Wireless();
					break;
				case "DHCP":
					if(systemVariable.manualWanSetup) goTo.DHCP();
					break;
				case "PPPoE":
					if(systemVariable.manualWanSetup) goTo.PPPoE();
					break;
				case "STATIC":
					if(systemVariable.manualWanSetup) goTo.Static();
					break;
				case "RESETMODEM":
					if(systemVariable.manualWanSetup) goTo.ResetModem();
					break;
				case "CHECKING":
					if(systemVariable.manualWanSetup) goTo.Waiting();
					break;
				default:
					if(isPage("noWan_page")) setTimeout(arguments.callee, 1000);
					break;
			}
		}, 500);

		goTo.loadPage("noWan_page", true);	
	}
	else if(systemVariable.detwanResult.wanType == "RESETMODEM"){
		setTimeout(function(){
			systemVariable.detwanResult = httpApi.detwanGetRet();
			if(!isPage("resetModem_page")) return false;

			if($("#resetModem").hasClass("unplug")){
				$("#resetModem").removeClass("unplug").addClass("plug")
			}
			else{
				$("#resetModem").removeClass("plug").addClass("unplug")
			}

			if(
				systemVariable.detwanResult.wanType != "" && 
				systemVariable.detwanResult.wanType != "CHECKING" &&
				systemVariable.detwanResult.wanType != "RESETMODEM"
			){
				goTo.autoWan();
			}

			if(isPage("resetModem_page")) setTimeout(arguments.callee, 1500);
		}, 1500);

		goTo.loadPage("resetModem_page", true);	
	}
	else if(systemVariable.detwanResult.wanType == "CHECKING"){
		systemVariable.manualWanSetup = false;

		setTimeout(function(){
			if(!isPage("waiting_page")) return false;

			if(systemVariable.detwanResult.wanType == "" || systemVariable.detwanResult.wanType == "CHECKING"){
				systemVariable.detwanResult = httpApi.detwanGetRet();
				if(isPage("waiting_page")) setTimeout(arguments.callee, 1000);
				return false;
			}

			if(!systemVariable.manualWanSetup) goTo.autoWan();	
		}, 1000);

		goTo.loadPage("waiting_page", true);	
	}
	else{
		if(!systemVariable.forceChangePw)
			abort.backToStartQIS();
		else
			goTo.loadPage("login_name", true);	
	}
};

abort.nowan = function(){
	if(!systemVariable.forceChangePw)
		abort.backToStartQIS();
	else
		goTo.loadPage("login_name", true);
};

abort.modem = function(){
	postDataModel.remove(modemObj);

	if($("#iptv_checkbox").is(":checked")){
		goTo.loadPage("iptv_setting", true);
	}
	else if(systemVariable.detwanResult.wanType == "MODEM" || systemVariable.detwanResult.wanType == "DHCP"){
		if(qisPostData.hasOwnProperty("http_username"))
			goTo.loadPage("login_name", true);
		else
			abort.backToStartQIS();
	}
	else if(qisPostData.hasOwnProperty("wan_heartbeat_x")){
		goTo.loadPage("getIp_setting", true);
	}
	else if(qisPostData.hasOwnProperty("wan_pppoe_username")){
		goTo.loadPage("pppoe_setting", true);
	}
	else if(qisPostData.hasOwnProperty("wan_ipaddr_x")){
		goTo.loadPage("static_setting", true);
	}
	else{
		goTo.loadPage("wan_setting", true);
	}

};

abort.pppoe = function(){
	postDataModel.remove(wanObj.all);

	$("#vpnServerContainer").hide();

	if(location.search == "?flag=pppoe"){
		window.history.back();
	}
	else if(systemVariable.detwanResult.wanType == "PPPoE"){
		if(!systemVariable.forceChangePw)
			abort.backToStartQIS();
		else
			goTo.loadPage("login_name", true);
	}
	else if(systemVariable.detwanResult.wanType == "PPPDHCP"){
		goTo.loadPage("wanTypePrompt_setting", true);
	}	
	else{									
		if(!isSupport("VPNCLIENT") && !isSupport("IPTV")){
			goTo.loadPage("accountPrompt_setting", true);
		}
		else{
			goTo.loadPage("wan_setting", true);
		}
	}
};

abort.getIp = function(){
	goTo.loadPage("pppoe_setting", true);
};

abort.static = function(){
	if(qisPostData.hasOwnProperty("wan_heartbeat_x")){
		postDataModel.remove(wanObj.staticIp);
		goTo.loadPage("getIp_setting", true);
	}
	else{
		postDataModel.remove(wanObj.all);
		goTo.loadPage("wan_setting", true);
	}
};

abort.iptv = function(){
	postDataModel.remove(iptvObj);
	postDataModel.remove(iptvManualObj);
	postDataModel.remove(iptvWanSettingObj);

	if(qisPostData.hasOwnProperty("wan_heartbeat_x")){
		goTo.loadPage("getIp_setting", true);
	}
	else if(qisPostData.hasOwnProperty("wan_pppoe_username")){
		goTo.loadPage("pppoe_setting", true);
	}
	else if(qisPostData.hasOwnProperty("wan_ipaddr_x")){
		goTo.loadPage("static_setting", true);
	}
	else{
		goTo.loadPage("wan_setting", true);
	}
};

abort.getLanIp = function(){
	postDataModel.remove(lanObj.general);
	postDataModel.remove(lanObj.staticIp);

	if(location.search == "?flag=lanip"){
		window.history.back();
	}
	else if(systemVariable.multiPAP.wlcOrder.length > 0){
		abort.backTo_papList_wlcKey();
	}
	else{
		if(isOriginSwMode("AP")){
			if(!systemVariable.forceChangePw)
				abort.backToStartQIS();
			else
				goTo.loadPage("login_name", true);	
		}
		else{
			goTo.loadPage("opMode_page", true);
		}
	}
};

abort.lanStatic = function(){
	postDataModel.remove(lanObj.staticIp);
	goTo.loadPage("getLanIp_setting", true);
};

abort.wlcKey = function(){
	$("#wlc_ssid_manual").val("");
	$("#wlc_wifiKey").val("");
	$("#wlc_wifiKey").showTextHint("");

	if(systemVariable.selectedAP.unit != undefined){//is not manual
		postDataModel.remove(wlcMultiObj["wlc" + systemVariable.selectedAP.unit]);
		systemVariable.selectedAP = [];
	}

	genPAPList(systemVariable.papList, systemVariable.multiPAP.wlcOrder);
	goTo.loadPage("papList_page", true);
}

abort.siteSurvey = function(){
	if(location.search == "?flag=sitesurvey_mb" || location.search == "?flag=sitesurvey_rep"){
		window.history.back();
	}
	else if(systemVariable.advSetting){
		postDataModel.remove(opModeObj);
		goTo.loadPage("opMode_page", true);
	}
	else{
		if(!systemVariable.forceChangePw)
			goTo.loadPage("welcome", true);
		else
			goTo.loadPage("login_name", true);	
	}
}

abort.papList = function(){
	if(systemVariable.multiPAP.wlcOrder.length == 0) {
		if(systemVariable.advSetting){
			goTo.loadPage("opMode_page", true);
		}
		else{
			if(isSupport("RPMesh")){
				if(systemVariable.papListAiMesh.length > 0)
					goTo.loadPage("amas_option_page", true);
				else
					goTo.loadPage("welcome", true);
			}
			else
				abort.siteSurvey();
		}
	}
	else{
		abort.backTo_papList_wlcKey();
	}
}

abort.wireless = function(){
	postDataModel.remove(generalObj);
	postDataModel.remove(wirelessObj.wl0);
	postDataModel.remove(wirelessObj.wl1);
	postDataModel.remove(wirelessObj.wl2);
	postDataModel.remove(smartConnectObj);

	if($("#iptv_checkbox").is(":checked")){
		goTo.loadPage("iptv_setting", true);
	}
	else if(qisPostData.hasOwnProperty("modem_enable")){
		goTo.loadPage("modem_setting", true);
	}
	else if(qisPostData.hasOwnProperty("wan_heartbeat_x")){
		goTo.loadPage("getIp_setting", true);
	}
	else if(qisPostData.hasOwnProperty("wan_pppoe_username")){
		goTo.loadPage("pppoe_setting", true);
	}
	else if(qisPostData.hasOwnProperty("wan_ipaddr_x")){
		goTo.loadPage("static_setting", true);
	}
	else if(qisPostData.hasOwnProperty("lan_proto")){
		if(qisPostData.lan_proto === "dhcp")
			goTo.loadPage("getLanIp_setting", true);
		else
			goTo.loadPage("lanStatic_setting", true);
	}
	else if(systemVariable.manualWanType == "DHCP"){
		goTo.loadPage("wan_setting", true);
	}
	else if(systemVariable.detwanResult.wanType == "DHCP" || systemVariable.detwanResult.wanType == "CONNECTED"){
		if(systemVariable.meshRole == "meshRouter")
			goTo.loadPage("amasrole_page", true);
		else if(systemVariable.forceChangePw)
			goTo.loadPage("login_name", true);
		else
			abort.backToStartQIS();
	}
	else if(systemVariable.detwanResult.wanType == "PPPDHCP"){
		goTo.loadPage("wanTypePrompt_setting", true);
	}
	else{
		goTo.loadPage("wan_setting", true);
	}
};

abort.update = function(){
	systemVariable.isNewFw = 0;

	httpApi.nvramSet({
		"action_mode": "apply",
		"rc_service": getRestartService()
	}, goTo.Finish);
};

abort.connCap = function(){
	goTo.loadPage("amasnode_page", true);
};

abort.amasbundle = function(){
	location.href = "/";
};

abort.errRouterWifi = function(){
	goTo.loadPage("amasconncap_page", true);
}

abort.errManual = function(){
	goTo.loadPage("amasconncap_page", true);
};

abort.amasIntro = function(){
	goTo.loadPage("opMode_page", true);
}

abort.amasNode = function(){
	if(location.search == "?flag=amasnode_page")
		window.history.back();
	else{
		if(isSupport("RPMesh")){
			if(!systemVariable.advSetting){
				goTo.loadPage("amas_option_page", true);
			}
			else{
				goTo.loadPage("amasrole_page", true);
			}
		}
		else {
			if(!systemVariable.advSetting){
				$(".dailIP").hide();
				$(".autoIP").show();
				goTo.loadPage("wan_setting", true);
			}
			else if(!qisPostData.hasOwnProperty("sw_mode")){
				goTo.loadPage("advanced_setting", true);
			}
			else{
				goTo.loadPage("amasrole_page", true);
			}
		}
	}
}

abort.backTo_papList_wlcKey = function(){
	var last_unit = systemVariable.multiPAP.wlcOrder.slice(-1)[0];
	var manual = systemVariable.multiPAP.wlcStatus["wlc" + last_unit + "_manual"];
	var skip = systemVariable.multiPAP.wlcStatus["wlc" + last_unit + "_skip"];
	if(skip){
		systemVariable.multiPAP.wlcStatus["wlc" + last_unit + "_checked"] = false;
		systemVariable.multiPAP.wlcStatus["wlc" + last_unit + "_manual"] = false;
		systemVariable.multiPAP.wlcOrder.pop();
		genPAPList(systemVariable.papList, systemVariable.multiPAP.wlcOrder);
		postDataModel.remove(wlcMultiObj["wlc" + last_unit]);
		goTo.loadPage("papList_page", true);
	}
	else{
		if(manual){
			systemVariable.multiPAP.wlcStatus["wlc" + last_unit + "_checked"] = false;
			systemVariable.multiPAP.wlcStatus["wlc" + last_unit + "_manual"] = false;

			systemVariable.multiPAP.wlcOrder.pop();
			genWLBandOption();

			$(".manual_pap_setup").show();
			var band = qisPostData["wlc" + last_unit + "_band"];
			var ssid = qisPostData["wlc" + last_unit + "_ssid"];
			var auth_mode = qisPostData["wlc" + last_unit + "_auth_mode"];
			var crypto = qisPostData["wlc" + last_unit + "_crypto"];
			var wpa_psk = qisPostData["wlc" + last_unit + "_wpa_psk"];
			var wep = qisPostData["wlc" + last_unit + "_wep"];
			var wep_key = qisPostData["wlc" + last_unit + "_wep_key"];
			var key = qisPostData["wlc" + last_unit + "_key"];
			$("#wlc_band_manual  option[value=" + band + "]").prop("selected", true).change();
			$("#wlc_ssid_manual").val(ssid);
			$("#wlc_auth_mode_manual  option[value=" + auth_mode + "]").prop("selected", true).change();
			if(crypto == "")
				$("#wlc_crypto_manual  option:first").prop("selected", true).change();
			else
				$("#wlc_crypto_manual  option[value=" + crypto + "]").prop("selected", true).change();
			if(key == "")
				$("#wlc_key_index_manual  option:first").prop("selected", true).change();
			else
				$("#wlc_key_index_manual  option[value=" + key + "]").prop("selected", true).change();
			handleWLAuthModeItem();
			handleWLWepOption();
			if(wep == "")
				$("#wlc_wep_manual  option:first").prop("selected", true).change();
			else
				$("#wlc_wep_manual  option[value=" + wep + "]").prop("selected", true).change();

			if(wep == "1" || wep == "2")
				$("#wlc_wifiKey").val(wep_key);
			else
				$("#wlc_wifiKey").val(wpa_psk);

			postDataModel.remove(wlcMultiObj["wlc" + last_unit]);
			goTo.loadPage("wlcKey_setting", true);
		}
		else{
			var isOpenAuthMode = (qisPostData["wlc" + last_unit + "_auth_mode"] == "open" && qisPostData["wlc" + last_unit + "_wep"] == "0");
			var isWepAuthMode = (qisPostData["wlc" + last_unit + "_auth_mode"] == "open" && qisPostData["wlc" + last_unit + "_wep"] == "1");
			systemVariable.multiPAP.wlcStatus["wlc" + last_unit + "_checked"] = false;
			systemVariable.multiPAP.wlcStatus["wlc" + last_unit + "_skip"] = false;
			systemVariable.multiPAP.wlcOrder.pop();
			if(isOpenAuthMode){
				genPAPList(systemVariable.papList, systemVariable.multiPAP.wlcOrder);
				postDataModel.remove(wlcMultiObj["wlc" + last_unit]);
				goTo.loadPage("papList_page", true);
			}
			else{
				$(".manual_pap_setup").hide();
				$("#manual_pap_setup-key").show();
				if(isWepAuthMode){
					$("#wlc_wifiKey").val(qisPostData["wlc" + last_unit + "_wep_key"]);
					qisPostData["wlc" + last_unit + "_wep_key"] = "";
				}
				else{
					$("#wlc_wifiKey").val(qisPostData["wlc" + last_unit + "_wpa_psk"]);
					qisPostData["wlc" + last_unit + "_wpa_psk"] = "";
				}

				systemVariable.selectedAP = systemVariable.papList[qisPostData["wlc" + last_unit + "_ap_mac"]];

				goTo.loadPage("wlcKey_setting", true);
			}
		}
	}
};

var goTo = {};

goTo.Welcome = function(){
	systemVariable.historyPage = ["welcome"];

	if(isOriginSwMode("RT")){
		httpApi.startAutoDet();

		setTimeout(function(){
			systemVariable.detwanResult = httpApi.detwanGetRet();
			if(systemVariable.detwanResult.wanType == "CHECKING" || systemVariable.detwanResult.wanType == "" || systemVariable.isDefault){
				if(isPage("welcome") || isPage("login_name")) setTimeout(arguments.callee, 1000);
			}
		}, 500);
	}

	if(systemVariable.isDefault){
		setUpTimeZone();
	}

	$("#tosCheckbox")
		.change(function(){
			$(this).val($(this).is(":checked"));

			if($("#tosCheckbox").is(":checked")){
				postDataModel.insert(bwdpiObj);
			}
			else{
				postDataModel.remove(bwdpiObj);
			}
		});

	$("#tosTitle")
		.click(function(){
			$("#tosCheckbox")
				.prop('checked', !$("#tosCheckbox").prop("checked"))
				.change()
		})

	// start to detect in the background
	setTimeout(startLiveUpdate, 1000);
};

goTo.advSetting = function(){
	systemVariable.advSetting = true;
	goTo.loadPage("advanced_setting", false);
}

goTo.Login = function(){
	postDataModel.insert(userObj);

	$("#http_username")
		.keyup(function(e){
			if(e.keyCode == 13){
				$("#http_passwd").focus();
			}
		});

	$("#http_passwd")
		.keyup(function(e){
			if(e.keyCode == 13){
				$("#http_passwd_confirm").focus();
			}
		});

	$("#http_passwd_confirm")
		.keyup(function(e){
			if(e.keyCode == 13){
				apply.login();
			}
		});

	goTo.loadPage("login_name", false);
};

goTo.changePwInTheEnd = function(){
	$("#btn_login_apply").html("<#CTL_apply#>");
	goTo.Login();
}

goTo.autoWan = function(){
	systemVariable.opMode = "RT";
	postDataModel.remove(wanObj.all);

	if(isSupport("amas")){
		postDataModel.insert(aimeshObj);
		qisPostData.cfg_master = "1";
	}

	switch(systemVariable.detwanResult.wanType){
		case "DHCP":
			goTo.DHCP();
			break;
		case "PPPoE":
			goTo.PPPoE();
			break;
		case "STATIC":
			goTo.Static();
			break;
		case "NOWAN":
			goTo.NoWan();
			break;
		case "MODEM":
			goTo.Modem();
			break;
		case "CHECKING":
			goTo.Waiting();
			break;
		case "RESETMODEM":
			goTo.ResetModem();
			break;
		case "PPPDHCP":
			goTo.PPPDHCP();
			break;
		case "CONNECTED":
			goTo.Wireless();
			break;
		case "":
			goTo.Waiting();
			break;
		default:
			goTo.WAN();
			break;
	}
};

goTo.opMode = function(){
	postDataModel.insert(opModeObj);
	goTo.loadPage("opMode_page", false);
};

goTo.rtMode = function(){
	qisPostData.sw_mode = 1;
	qisPostData.wlc_psta = 0;
	qisPostData.wlc_dpsta = 0;
	systemVariable.opMode = "RT";
	if(isSupport("amas")){
		postDataModel.insert(aimeshObj);
		qisPostData.cfg_master = "1";
	}

	apply.manual();
};

goTo.rpMode = function(){
	if(isSupport("concurrep")){
		qisPostData.sw_mode = 3;
		qisPostData.wlc_psta = 2;
		qisPostData.wlc_dpsta = 1;
	}
	else{
		if(isSdk("7") || isSdk("9")){
			qisPostData.sw_mode = 3;
			qisPostData.wlc_psta = 2;
			qisPostData.wlc_dpsta = 0;
		}
		else{
			qisPostData.sw_mode = 2;
			qisPostData.wlc_psta = 0;
			qisPostData.wlc_dpsta = 0;
		}
	}
	systemVariable.opMode = "RP";
	if(isSupport("amas")){
		postDataModel.insert(aimeshObj);
		qisPostData.cfg_master = "0";
	}

	goTo.siteSurvey();
};

goTo.apMode = function(){
	qisPostData.sw_mode = 3;
	qisPostData.wlc_psta = 0;
	qisPostData.wlc_dpsta = 0;
	systemVariable.opMode = "AP";
	if(isSupport("amas")){
		postDataModel.insert(aimeshObj);
		qisPostData.cfg_master = "1";
	}

	goTo.GetLanIp();
};

goTo.mbMode = function(){
	if(isSupport("qcawifi") || isSupport("rawifi")){
		qisPostData.sw_mode = 2;
		qisPostData.wlc_psta = 1;
		qisPostData.wlc_dpsta = 0;
	}
	else{
		qisPostData.sw_mode = 3;
		qisPostData.wlc_psta = 1;
		qisPostData.wlc_dpsta = 0;
	}
	systemVariable.opMode = "MB";
	if(isSupport("amas")){
		postDataModel.insert(aimeshObj);
		qisPostData.cfg_master = "0";
	}

	goTo.siteSurvey();
};

goTo.meshMode = function(){
	systemVariable.opMode = "MESH";
	goTo.chooseRole();
};

goTo.WAN = function(){
	goTo.loadPage("accountPrompt_setting", false);
};

goTo.PPPDHCP = function(){
	goTo.loadPage("wanTypePrompt_setting", false);
};

goTo.dailIP = function(){
	handleSysDep();

	$(".dailIP").show();
	$(".autoIP").hide();

	if(!isSupport("VPNCLIENT") && !isSupport("IPTV")){
		goTo.PPPoE();
	}
	else{
		goTo.loadPage("wan_setting", false);
	}
}

goTo.autoIP = function(){
	$(".dailIP").hide();
	$(".autoIP").show();
	// $("#connTypeDesc").html("<#QIS_SmartConn_TypeAuto#>")

	goTo.loadPage("wan_setting", false);	
}

goTo.DHCP = function(){
	if(systemVariable.originWanType.toLowerCase() !== "dhcp"){
		postDataModel.remove(wanObj.all);

		postDataModel.insert(wanObj.general);
		postDataModel.insert(wanObj.dhcp);
		qisPostData.wan_proto = "dhcp";
	}

	if(systemVariable.manualWanSetup){
		systemVariable.manualWanType = 'DHCP';
	}

	apply.dhcp();
};

goTo.PPPoE = function(){
	postDataModel.remove(wanObj.all);
	postDataModel.insert(wanObj.general);
	postDataModel.insert(wanObj.pppoe);
	qisPostData.wan_proto = "pppoe";

	if(systemVariable.manualWanSetup){
		systemVariable.manualWanType = 'PPPoE';
	}

	var pppoeInfo = httpApi.nvramGet(["wan0_pppoe_username", "wan0_pppoe_passwd"]);

	$("#wan_pppoe_username")
		.val(pppoeInfo.wan0_pppoe_username)
		.unbind("keyup")
		.keyup(function(e){
			if(e.keyCode == 13){
				$("#wan_pppoe_passwd").focus();
			}
		});

	$("#wan_pppoe_passwd")
		.val(pppoeInfo.wan0_pppoe_passwd)
		.unbind("keyup")
		.keyup(function(e){
			if(e.keyCode == 13){
				apply.pppoe();
			}
		});

	$(".pppInput")
		.unbind("change")
		.change(function(){
			postDataModel.insert(wanObj.general);
			postDataModel.insert(wanObj.pppoe);
			qisPostData.wan_proto = "pppoe";
		})

	goTo.loadPage("pppoe_setting", false);
};

goTo.Static = function(){
	if(systemVariable.originWanType.toLowerCase() !== "static"){
		postDataModel.remove(wanObj.all);
		postDataModel.insert(wanObj.staticIp);
		postDataModel.insert(wanObj.general);
		qisPostData.wan_proto = "static";
	}

	if(systemVariable.manualWanSetup){
		systemVariable.manualWanType = 'STATIC';
	}

	var staticInfo = httpApi.nvramGet(["wan0_ipaddr_x", "wan0_netmask_x", "wan0_gateway_x", "wan0_dns1_x", "wan0_dns2_x"]);
	
	$("#static_ipaddr")
		.val(staticInfo.wan0_ipaddr_x.replace("0.0.0.0", ""))
		.keyup(function(e){
			if(e.keyCode == 13){
				$("#static_subnet").focus();
			}
		});

	$("#static_subnet")
		.val(staticInfo.wan0_netmask_x.replace("0.0.0.0", ""))
		.keyup(function(e){
			if(e.keyCode == 13){
				$("#static_gateway").focus();
			}
		});

	$("#static_gateway")
		.val(staticInfo.wan0_gateway_x.replace("0.0.0.0", ""))
		.keyup(function(e){
			if(e.keyCode == 13){
				$("#static_dns1").focus();
			}
		});

	$("#static_dns1")
		.val(staticInfo.wan0_dns1_x.replace("0.0.0.0", ""))
		.keyup(function(e){
			if(e.keyCode == 13){
				$("#static_dns2").focus();
			}
		});

	$("#static_dns2")
		.val(staticInfo.wan0_dns2_x.replace("0.0.0.0", ""))
		.keyup(function(e){
			if(e.keyCode == 13){
				apply.static();
			}
		});

	$(".staticInput")
		.change(function(){
			postDataModel.insert(wanObj.staticIp);
			postDataModel.insert(wanObj.general);
			qisPostData.wan_proto = "static";
		})

	goTo.loadPage("static_setting", false);		
};

goTo.PPTP = function(){
	postDataModel.insert(wanObj.general);
	qisPostData.wan_proto = "pptp";

	if(systemVariable.manualWanSetup){
		systemVariable.manualWanType = 'PPTP';
	}

	goTo.VPN();
};

goTo.L2TP = function(){
	postDataModel.insert(wanObj.general);
	qisPostData.wan_proto = "l2tp";

	if(systemVariable.manualWanSetup){
		systemVariable.manualWanType = 'L2TP';
	}

	goTo.VPN();
};

goTo.VPN = function(){
	postDataModel.insert(wanObj.pppoe);
	postDataModel.insert(wanObj.vpn);

	$("#vpnServerContainer").show();

	var pppoeInfo = httpApi.nvramGet(["wan0_pppoe_username", "wan0_pppoe_passwd", "wan0_heartbeat_x"]);

	$("#wan_pppoe_username")
		.val(pppoeInfo.wan0_pppoe_username)
		.unbind("keyup")
		.keyup(function(e){
			if(e.keyCode == 13){
				$("#wan_pppoe_passwd").focus();
			}
		});

	$("#wan_pppoe_passwd")
		.val(pppoeInfo.wan0_pppoe_passwd)
		.unbind("keyup")
		.keyup(function(e){
			if(e.keyCode == 13){
				$("#wan_heartbeat_x").focus();
			}
		});

	$("#wan_heartbeat_x")
		.val(pppoeInfo.wan0_heartbeat_x)
		.unbind("keyup")
		.keyup(function(e){
			if(e.keyCode == 13){
				apply.pppoe();
			}
		});

	$(".pppInput")
		.unbind("change")
		.change(function(){
			postDataModel.insert(wanObj.pppoe);
			postDataModel.insert(wanObj.vpn);
		})

	goTo.loadPage("pppoe_setting", false);
};

goTo.GetIp = function(){
	goTo.loadPage("getIp_setting", false);
};

goTo.vpnDHCP = function(){
	postDataModel.insert(wanObj.dhcp);
	apply.dhcp();
};

goTo.vpnStatic = function(){
	var staticInfo = httpApi.nvramGet(["wan0_ipaddr_x", "wan0_netmask_x", "wan0_gateway_x", "wan0_dns1_x", "wan0_dns2_x"]);

	$("#static_ipaddr")
		.val(staticInfo.wan0_ipaddr_x.replace("0.0.0.0", ""))
		.keyup(function(e){
			if(e.keyCode == 13){
				$("#static_subnet").focus();
			}
		});

	$("#static_subnet")
		.val(staticInfo.wan0_netmask_x.replace("0.0.0.0", ""))
		.keyup(function(e){
			if(e.keyCode == 13){
				$("#static_gateway").focus();
			}
		});

	$("#static_gateway")
		.val(staticInfo.wan0_gateway_x.replace("0.0.0.0", ""))
		.keyup(function(e){
			if(e.keyCode == 13){
				$("#static_dns1").focus();
			}
		});

	$("#static_dns1")
		.val(staticInfo.wan0_dns1_x.replace("0.0.0.0", ""))
		.keyup(function(e){
			if(e.keyCode == 13){
				$("#static_dns2").focus();
			}
		});

	$("#static_dns2")
		.val(staticInfo.wan0_dns2_x.replace("0.0.0.0", ""))
		.keyup(function(e){
			if(e.keyCode == 13){
				apply.static();
			}
		});

	postDataModel.insert(wanObj.staticIp);
	goTo.loadPage("static_setting", false);		
};

goTo.IPTV = function(){
	postDataModel.insert(iptvObj);

	$("#iptv_auth_mode")
		.change(function(){$(".iptv_pppoe_setting").toggle($("#iptv_auth_mode").val() == "8021x-md5")})
		.trigger("change")

	$("#switch_wantag")
		.change(function(){
			var isp = $("#switch_wantag").val();
			var isp_profile = httpApi.getISPProfile(isp);

			if(isp_profile.iptv_port != "")
				$("#iptv_stb").show();
			else
				$("#iptv_stb").hide();

			if(isp_profile.voip_port != "")
				$("#iptv_voip").show();
			else
				$("#iptv_voip").hide();

			if(isp_profile.iptv_config == "1" || isp_profile.voip_config == "1"){
				$("#iptv_wanSetup").show();
				postDataModel.insert(iptvWanSettingObj);
				if(isp == "movistar"){
					qisPostData.wan10_proto = "static";
					qisPostData.wan11_proto = "dhcp";
				}
			}
			else{
				$("#iptv_wanSetup").hide();
				postDataModel.remove(iptvWanSettingObj);
			}

			if(isp == "manual"){
				$("#iptv_manual").show();
				postDataModel.insert(iptvManualObj);
			}
			else{
				$("#iptv_manual").hide();
				postDataModel.remove(iptvManualObj);
			}

			qisPostData.switch_wantag = isp;
			qisPostData.switch_stb_x = isp_profile.switch_stb_x;

			$("#iptv_voip_title").html(function(){
				return (isp == "vodafone") ? "IPTV STB Port" : "VoIP Port";
			});

			$("#iptv_stb_title").html(function(){
				return (isp == "vodafone" || isp == "meo") ? "Bridge Port" : "IPTV STB Port";
			});
		})
		.trigger("change")

	$("#iptv_checkbox")
		.change(function(){
			if($("#iptv_checkbox").is(":checked")){
				postDataModel.insert(iptvObj);
			}
			else{
				postDataModel.remove(iptvObj);
			}
		})

	if(isSku("TW")){
		$("#switch_wantag")
			.val("hinet")
			.trigger("change")
	}

	goTo.loadPage("iptv_setting", false);
};

goTo.GetLanIp = function(){
	postDataModel.insert(lanObj.general);
	goTo.loadPage("getLanIp_setting", false);
};

goTo.lanDHCP = function(){
	qisPostData.lan_proto = "dhcp";
	qisPostData.lan_dnsenable_x = "1";

	if(isSwMode("MB")){
		postDataModel.insert(generalObj);

		if(systemVariable.forceChangePwInTheEnd){
			goTo.changePwInTheEnd();
		}
		else{
			transformWLCObj();

			httpApi.nvramSet((function(){
				qisPostData.action_mode = "apply";
				qisPostData.rc_service = getRestartService();
				return qisPostData;
			})(), goTo.Finish);
		}
	}
	else{
		goTo.Wireless();
	}
};

goTo.lanStatic = function(){
	$("#static_lan_ipaddr")
		.keyup(function(e){
			if(e.keyCode == 13){
				$("#static_lan_subnet").focus();
			}
		});

	$("#static_lan_subnet")
		.keyup(function(e){
			if(e.keyCode == 13){
				$("#static_lan_gateway").focus();
			}
		});

	$("#static_lan_gateway")
		.keyup(function(e){
			if(e.keyCode == 13){
				$("#static_lan_dns1").focus();
			}
		});

	$("#static_lan_dns1")
		.keyup(function(e){
			if(e.keyCode == 13){
				$("#static_lan_dns2").focus();
			}
		});

	$("#static_lan_dns2")
		.keyup(function(e){
			if(e.keyCode == 13){
				apply.static();
			}
		});

	postDataModel.insert(lanObj.staticIp);
	goTo.loadPage("lanStatic_setting", false);		
};

goTo.siteSurvey = function(){
	$("#siteSurvey_search").show();
	$("#siteSurvey_fail").hide();
	systemVariable.papList = [];
	systemVariable.papListAiMesh = [];

	$("#siteSurveyLoading").html(Get_Component_Loading);

	var allWlArray =  getAllWlArray();
	for(var i = 0; i < allWlArray.length; i += 1){
		systemVariable.multiPAP.wlcStatus["wlc" + allWlArray[i].ifname + "_checked"] = false;
		systemVariable.multiPAP.wlcStatus["wlc" + allWlArray[i].ifname + "_manual"] = false;
		systemVariable.multiPAP.wlcStatus["wlc" + allWlArray[i].ifname + "_skip"] = false;
	}

	if(isSupport("concurrep"))
		$("#siteSurvey_page").find(".titleSub").html("<#WiFi_Network_List#>");
	else
		$("#siteSurvey_page").find(".titleSub").html("<#WLANConfig11b_RBRList_groupitemdesc#>");

	httpApi.nvramSet({"action_mode": "apply", "rc_service":"restart_wlcscan"}, function(){
		setTimeout(function(){
			var siteSurveyResult = {
				"isFinish": false,
				"aplist": []
			};

			siteSurveyResult.aplist = httpApi.hookGet("get_ap_info", true).sort(function(a, b){return parseInt(b[5])-parseInt(a[5]);});
			siteSurveyResult.isFinish = (httpApi.nvramGet(["wlc_scan_state"], true).wlc_scan_state == "5");

			systemVariable.papList = getPAPList(siteSurveyResult.aplist);
			systemVariable.papListAiMesh = getPAPList(siteSurveyResult.aplist, "AiMesh", "0");

			if(systemVariable.papList.length > 0 && siteSurveyResult.isFinish && isPage("siteSurvey_page")) {
				if(isSupport("RPMesh") && systemVariable.papListAiMesh.length > 0 && !systemVariable.advSetting && systemVariable.advSetting != undefined)
					goTo.AiMeshOption();
				else
					goTo.papList();
			}
			else if(systemVariable.papList.length == 0 && siteSurveyResult.isFinish && isPage("siteSurvey_page")) {
				$("#siteSurvey_search").hide();
				$("#siteSurvey_fail").show();
			}

			if(!siteSurveyResult.isFinish && isPage("siteSurvey_page")) setTimeout(arguments.callee, 1000);
		}, 1000);
	});

	goTo.loadPage("siteSurvey_page", false);
};
goTo.AiMeshOption = function(){
	$("#amas_option_page").find("#connectAiMesh")
		.unbind("click")
		.click(function(){goTo.asNode();});
	$("#amas_option_page").find("#connectOther")
		.unbind("click")
		.click(function(){goTo.papList();});
	goTo.loadPage("amas_option_page", false);
}

goTo.papList = function() {
	if(isSupport("RPMesh"))
		$("#papList_page").find(".titleSub").html("<#WiFi_Network_List#>");
	else
		$("#papList_page").find(".titleSub").html("<#WLANConfig11b_RBRList_groupitemdesc#>");

	genPAPList(systemVariable.papList, systemVariable.multiPAP.wlcOrder);
	goTo.loadPage("papList_page", false);
};
goTo.papSet = function() {
	var unit = systemVariable.selectedAP.unit.toString();
	var isOpenAuthMode = (qisPostData["wlc" + unit + "_auth_mode"] == "open" && qisPostData["wlc" + unit + "_wep"] == "0");
	if(isOpenAuthMode) {
		systemVariable.multiPAP.wlcOrder.push(unit);
		systemVariable.multiPAP.wlcStatus["wlc" + unit + "_checked"] = true;
		goTo.lanIP_papList();
	}
	else
		goTo.wlcKey();
};

goTo.wlcKey = function(){
	$(".manual_pap_setup").hide();
	$("#manual_pap_setup-key").show();
	$("#wlc_wifiKey").val("");
	$("#wlc_wifiKey").showTextHint("");

	goTo.loadPage("wlcKey_setting", false);
};
goTo.wlcManual = function(){
	systemVariable.selectedAP = [];
	$(".manual_pap_setup").show();
	genWLBandOption();
	$("#wlc_ssid_manual").val("");
	$("#wlc_band_manual option:first").prop("selected", true).change();
	$("#wlc_auth_mode_manual").val("psk2").change();
	$("#wlc_crypto_manual option:first").prop("selected", true).change();
	$("#wlc_wep_manual option:first").prop("selected", true).change();
	$("#wlc_key_index_manual option:first").prop("selected", true).change();
	$("#wlc_wifiKey").val("");
	$("#wlc_wifiKey").showTextHint("");

	$("#manual_pap_setup-nmode_hint").hide();
	$("#manual_pap_setup-wep").hide();
	$("#manual_pap_setup-key-index").hide();

	$("#wlc_auth_mode_manual")
		.change(function(){
			handleWLAuthModeItem();
			handleWLWepOption($(this).val());
		});

	$("#wlc_crypto_manual")
		.change(function(){
			handleWLAuthModeItem();
		});

	$("#wlc_wep_manual")
		.change(function(){
			handleWLAuthModeItem();
		});

	goTo.loadPage("wlcKey_setting", false);
};

goTo.Wireless = function(){
	function genWirelessInputField(__wlArray){
		$("#wlInputField")
			.hide()
			.html(Get_Component_WirelessInput(__wlArray))
			.fadeIn()

		$(".wlInput")
			.keyup(function(e){
				if(!$("#wireless_sync_checkbox").is(":checked")) return false;

				var nvramName = this.id.split("_")[1];
				var referenceVal = this.value;

				if(this.id.indexOf("_0") !== -1){
					var wlArray = getAllWlArray();

					for(var idx=1; idx<wlArray.length; idx++){
						$("#wireless_" + nvramName + "_" + idx).val(referenceVal + ((nvramName == "ssid" && referenceVal.length < 27) ? wlArray[idx].suffix : ""));
					}
				}
			})
			.change(function(e){
				postDataModel.insert(wirelessObj.wl0);
				if(isSupport("dualband")) postDataModel.insert(wirelessObj.wl1);
				if(isSupport("triband")){
					postDataModel.insert(wirelessObj.wl1);
					postDataModel.insert(wirelessObj.wl2);
				}
			});

		$(".secureInput")
			.unbind("click")
			.click(checkPasswd);
	}

	postDataModel.insert(generalObj);

	$(".bandStreeringSupport").hide();

	if(isSupport("SMARTCONNECT")){
		postDataModel.insert(smartConnectObj);
		$("#wireless_checkbox").enableCheckBox(true);
		$("#wireless_checkbox").change();
		$("#wireless_sync_checkbox").enableCheckBox(true);
		if(isSwMode("RT") || isSwMode("AP")){
			$(".bandStreeringSupport").show();
			if(systemVariable.isDefault){
				if(!isSupport("separate_ssid")){
					$("#wireless_checkbox").enableCheckBox(false);
					$("#wireless_checkbox").change();
					$("#wireless_sync_checkbox").enableCheckBox(false);
				}
			}
			else{
				var smart_connect_x = httpApi.nvramGet(["smart_connect_x"]).smart_connect_x;
				if(smart_connect_x == "1"){
					$("#wireless_checkbox").enableCheckBox(false);
					$("#wireless_checkbox").change();
					$("#wireless_sync_checkbox").enableCheckBox(false);
				}
			}
		}
		qisPostData.smart_connect_x = $("#wireless_checkbox").prop("checked") ? "0" : "1";
	}
	else
		$("#wireless_sync_checkbox").enableCheckBox((isSupport("dualband") || isSupport("triband")));

	if(!$(".wlInput").length){
		var wlArray = (qisPostData.hasOwnProperty("smart_connect_x") && !$("#wireless_checkbox").prop("checked")) ? [{"title":"", "ifname":"0"}] : getAllWlArray();
		genWirelessInputField(wlArray);

		$("#wireless_checkbox").change(function(e){
			var curStatus = $(this).prop("checked");
			var __wlArray = [];

			if(curStatus){
				__wlArray = getAllWlArray()
				qisPostData.smart_connect_x = "0";
				$("#wireless_sync_checkbox").enableCheckBox(true);
			}
			else{
				__wlArray = [{"title":"", "ifname":"0"}];
				qisPostData.smart_connect_x = "1";
				$("#wireless_sync_checkbox").enableCheckBox(false);
			}

			genWirelessInputField(__wlArray);
		});

		if(systemVariable.multiPAP.wlcOrder.length > 0)
			$("#wireless_sync_checkbox").enableCheckBox(false);
	}
	else{
		if(systemVariable.multiPAP.wlcOrder.length > 0){
			$("#wireless_sync_checkbox").enableCheckBox(false);
			var wlArray = getAllWlArray();
			var autoStr = false;
			handleWirelessClientSSID(wlArray, autoStr);
		}
	}

	$.each($(".wlInput:password"), function(idx, input){
		if(input.value == ""){
			postDataModel.insert(wirelessObj.wl0);
			if(isSupport("dualband")) postDataModel.insert(wirelessObj.wl1);
			if(isSupport("triband")){
				postDataModel.insert(wirelessObj.wl1);
				postDataModel.insert(wirelessObj.wl2);
			}
		}
	});

	$("#wireless_sync_checkbox").change(function(e){
		var curStatus = $(this).prop("checked");
		var wlArray = getAllWlArray();

		if(systemVariable.multiPAP.wlcOrder.length > 0){
			handleWirelessClientSSID(wlArray, curStatus);
		}
		else{
			if(curStatus){
				for(var idx=1; idx<wlArray.length; idx++){
					$("#wireless_ssid_" + idx).val($("#wireless_ssid_0").val() + (($("#wireless_ssid_0").val().length < 27) ? wlArray[idx].suffix : ""));
					$("#wireless_key_" + idx).val($("#wireless_key_0").val());
				}
			}
			else{
				for(var idx=1; idx<wlArray.length; idx++){
					$("#wireless_ssid_" + idx).val("");
					$("#wireless_key_" + idx).val("");
				}
			}
		}
	})

	if(isSwModeChanged()){
		postDataModel.insert(wirelessObj.wl0);

		if(isSupport("dualband")) postDataModel.insert(wirelessObj.wl1);
		if(isSupport("triband")){
			postDataModel.insert(wirelessObj.wl1);
			postDataModel.insert(wirelessObj.wl2);
		}
	}
	else if(systemVariable.multiPAP.wlcOrder.length > 0){
		postDataModel.insert(wirelessObj.wl0);

		if(isSupport("dualband")) postDataModel.insert(wirelessObj.wl1);
		if(isSupport("triband")){
			postDataModel.insert(wirelessObj.wl1);
			postDataModel.insert(wirelessObj.wl2);
		}
	}

	if(systemVariable.forceChangePwInTheEnd){
		$("#btn_wireless_apply").html("<#CTL_next#>");
	}

	goTo.loadPage("wireless_setting", false);
};

goTo.Modem = function(){
	postDataModel.insert(modemObj);

	require(['/require/modules/modem.js'], function(modem) {
		$.each(modem.profile, function(country, ispProfile){
			$("#modem_country").append($('<option>', {
				"value": country,
				"text": ispProfile.countryName
			}))
		});

		$("#modem_android")
			.change(function(){
				$(".dongle").toggle($(this).val() == "0");
				if($(this).val() == "0")
					$("#modem_autoapn").trigger("change")
			});

		$("#modem_autoapn")
			.change(function(){
				$(".modem_manual").toggle($("#modem_autoapn").val() != "1")
			});

		$("#modem_country")
			.change(function(){
				var country = $(this).val();
				$("#modem_isp").html("");
				$.each(modem.profile[country].config, function(idx, ispConfig){
					$("#modem_isp").append($('<option>', {
						"value": ispConfig.isp,
						"text": ispConfig.isp
					}));		
				});

				$("#modem_isp_container").toggle($($("#modem_isp").html()).val() !== "");
				$("#modem_enable_container").toggle($($("#modem_isp").html()).val() === "");
				$("#modem_isp").trigger("change");
			})
			.trigger("change");

		$("#modem_isp")
			.change(function(){
				var config = modem.profile[$("#modem_country").val()].config[$("#modem_isp").prop('selectedIndex')]
				$("#modem_apn").val(config.apn.split(";")[0]);
				$("#modem_dialnum").val(config.dialnum);
				$("#modem_user").val(config.user);
				$("#modem_pass").val(config.pass);
				$("#modem_enable").val(config.proto).trigger("change");
			})
			.trigger("change");

		$("#modem_enable")
			.change(function(){
				$("#Dev3G").html("");
				var isWiMax = ($(this).val() === "4");
				var dongleList = (isWiMax) ? modem.wimaxDongle : modem.hsdpaDongle;
				$.each(dongleList, function(index, dongle){
					$("#Dev3G").append($('<option>', {
						"value": dongle,
						"text": dongle
					}))
				});

				$("#modem_apn_container").toggle(!isWiMax);
				$("#modem_dialnum_container").toggle(!isWiMax);					
				$("#modem_ttlsid_container").toggle(isWiMax);					
				$("#Dev3G").trigger("change")
			})
			.trigger("change");

		$("#Dev3G")
			.toggle(!isSupport("gobi"));
	});

	goTo.loadPage("modem_setting", false);		
};

goTo.Update = function(){
	var applyBtn = (systemVariable.isNewFw == 2) ? "<#CTL_UpgradeNow#>" : "<#CTL_Agree#>";
	var abortBtn = (systemVariable.isNewFw == 2) ? "<#CTL_UpgradeNight#>" : "<#CTL_Cancel#>";

	$("#newVersion").val(systemVariable.newFwVersion);
	$("#desktop_applyBtn_update").html(applyBtn);
	$("#desktop_abortBtn_update").html(abortBtn);
	$("#mobile_applyBtn_update").html(applyBtn);
	$("#mobile_abortBtn_update").html(abortBtn);

	goTo.loadPage("update_page", false);
};

goTo.Upgrading = function(){
	var errCount = 0;

	setTimeout(function(){
		var	fwUpgradeInfo = httpApi.nvramGet(["webs_state_upgrade", "webs_state_error"], true);

		if(fwUpgradeInfo.isError){
			if(errCount > 5){
				$("#upgradeBtn").show();
				$(".detectIcon").hide();
				$("#liveUpdateStatus").html("<#Congratulations#><br/><#is_latest#>");
			}
			else{
				errCount++;
				setTimeout(arguments.callee, 1000);			
			}
		}
		else if(fwUpgradeInfo.webs_state_upgrade == "0"){
			$("#liveUpdateStatus").html("<#fw_downloading#>...");
			setTimeout(arguments.callee, 1000);
		}
		else if(fwUpgradeInfo.webs_state_error != "0" && fwUpgradeInfo.webs_state_error != ""){
			$("#upgradeBtn").show();
			$(".detectIcon").hide();
			$("#liveUpdateStatus").html("<#FIRM_fail_desc#>");
		}
		else{
			$("#liveUpdateStatus").html("<#FIRM_ok_desc#>");
			setTimeout(arguments.callee, 1000);
		}
	}, 500);

	goTo.loadPage("upgrading_page", false);
};

goTo.Upload = function(){
	var width = 1;
	var timerId = setInterval(function(){
		if(width >= 1000){
			clearInterval(timerId);
			goTo.Finish();
		}
		else{
			width++;
			$("#profileLoadingBar")
				.css({"width": width/10 + '%'})
				.html(parseInt(width/10) + '%');
		}
	}, systemVariable.rebootTime);

	goTo.loadPage("uploading_page", false);
}

goTo.Finish = function(){
	if(
		!(getRestartService().indexOf("restart_wireless") != -1 && isWlUser) &&
		getRestartService().indexOf("restart_subnet") == -1 &&
		getRestartService().indexOf("reboot") == -1 &&
		systemVariable.isNewFw == 0 &&
		!isSupport("lantiq")
	){
		goTo.leaveQIS();
		return false;
	}

	if(!isSwMode("MB")){
		$("#wirelessFinishFiled").append($("#wlInputField"));
		$(".secureInput").hide();
		$(".wlInput").attr({
			"disabled": "true",
			"type": "text"
		});
		if(isSwMode("RP"))
			$("#summary_page").find("#stepText2").html("<#OP_MB_desc9#>");
	}
	else{
		$("#summary_page").find("#stepText1").html("<#OP_MB_desc7#> <#OP_MB_desc6#>");
		$("#summary_page").find("#stepText2").html("<#OP_MB_desc9#>");
	}

	if(isSupport("lantiq")){
		/* Make sure BLUECAVE had updated wireless setting. */
		setTimeout(function(){
			var waveReady = httpApi.nvramGet(["wave_ready"], true).wave_ready;
			if(waveReady == "0"){
				setTimeout(arguments.callee, 1000);
				return false;
			}

			setTimeout(function(){
				setInterval(function(){
					httpApi.isAlive("", updateSubnet(systemVariable.lanIpaddr), goTo.leaveQIS);
				}, 2000);
			}, 5000);
		}, 7000);
	}
	else{
		setTimeout(function(){
			setInterval(function(){
				httpApi.isAlive("", updateSubnet(systemVariable.lanIpaddr), goTo.leaveQIS);
			}, 2000);
		}, 8000);
	}

	goTo.loadPage("summary_page", false);
};

goTo.skipMesh = function(){
	systemVariable.skipMesh = true;	
	apply.welcome();
}

goTo.chooseRole = function(){
	/* catch offline data */
	$("#amasconncap_page").load("/mobile/pages/amasconncap_page.html");
	$("#amasrestore_page").load("/mobile/pages/amasrestore_page.html");
	$("#amaserr_connrouter_page").load("/mobile/pages/amaserr_connrouter_page.html");
	systemVariable.defaultIpAddr = httpApi.nvramDefaultGet(["lan_ipaddr"]).lan_ipaddr;
	systemVariable.macAddr = httpApi.nvramGet(["et0macaddr"]).et0macaddr;
	systemVariable.skipMesh = false;

	goTo.loadPage("amasrole_page", false);
	if(isSupport("amasRouter") && !isSupport("noRouter")) {
		var check_status = setInterval(function(){
			if($("#AiMesh_router").is(":hidden"))
				$("#AiMesh_router").show();

			if($("#AiMesh_router").is(":visible") || !isPage("amasrole_page"))
				clearInterval(check_status);
		}, 100);
	}
	if(isSupport("amasNode")) {
		var check_node_status = setInterval(function(){
			if($("#AiMesh_node").is(":hidden"))
				$("#AiMesh_node").show();

			if($("#AiMesh_node").is(":visible") || !isPage("amasrole_page"))
				clearInterval(check_node_status);
		}, 100);
	}
};

goTo.asRouter = function(){
	systemVariable.meshRole = "meshRouter";

	if(qisPostData.hasOwnProperty("http_username")){
		if(systemVariable.detwanResult.wanType == "NOWAN"){
			goTo.NoWan();
		}
		else{
			goTo.autoWan();
		}		
	}
	else{
		apply.welcome();
	}
}

goTo.asNode = function(){
	$("#amasconncap_page").load("/mobile/pages/amasconncap_page.html");
	$("#amasrestore_page").load("/mobile/pages/amasrestore_page.html");
	$("#amaserr_connrouter_page").load("/mobile/pages/amaserr_connrouter_page.html");
	systemVariable.defaultIpAddr = httpApi.nvramDefaultGet(["lan_ipaddr"]).lan_ipaddr;
	systemVariable.macAddr = httpApi.nvramGet(["et0macaddr"]).et0macaddr;
	systemVariable.skipMesh = false;	

	goTo.loadPage("amasnode_page", false);
}

goTo.amasIntro = function(){
	goTo.loadPage("amasintro_page", false);
}

goTo.amasRestore = function(){
	var width = 1;

	var timerId = setInterval(function(){
		if(width >= 1000){
			clearInterval(timerId);
			setTimeout(goTo.Conncap, 1000);
		}
		else{
			width++;
			$("#amasRestoreLoadingBar")
				.css({"width": width/10 + "%"})
				.html(parseInt(width/10) + "%");
		}
	}, systemVariable.rebootTime);

	setTimeout(function(){
		var iAmAlive = httpApi.isAlive("http://" + systemVariable.defaultIpAddr, systemVariable.defaultIpAddr, function(){
			width = 1000;
			$("#amasRestoreLoadingBar")
				.animate({"width": "100%"})
				.html("100%");
			});

		if(isPage("amasrestore_page")) setTimeout(arguments.callee, 2000);
	}, 40000);

	httpApi.nvramSet({"action_mode": "Restore"});
	goTo.loadPage("amasrestore_page", false);
}

goTo.Conncap = function(){
	if(systemVariable.isDefault) $("#cancelBtn").show();
	systemVariable.macAddr = httpApi.nvramGet(["et0macaddr"]).et0macaddr;

	setInterval(function(){
		httpApi.checkCap("http://router.asus.com", function(){
			setTimeout(function(){
				if(isPage("amasconncap_page")) window.location.href = "http://router.asus.com/cfg_onboarding.cgi?flag=AMesh&id=" + systemVariable.macAddr.split(":").join("");
			}, 3000);

			$("#loginCapAlert").fadeIn(500);
		});
	}, 5000);

	setInterval(function(){
		httpApi.isAlive("", updateSubnet(systemVariable.lanIpaddr), function(){
			$("#errConn_howToFind").fadeIn(300);
		});
	}, 10000);

	goTo.loadPage("amasconncap_page", false)
}

goTo.NoWan = function(){
	systemVariable.manualWanSetup = false;

	if(isSupport("modem")){
		setTimeout(function(){
			if(!isPage("noWan_page")) return false;

			if($("#noWanEth").is(":visible")){
				$("#noWanEth").fadeOut(500);	
				setTimeout(function(){$("#noWanUsb").fadeIn(500);}, 500);	
			}
			else{
				$("#noWanUsb").fadeOut(500);
				setTimeout(function(){$("#noWanEth").fadeIn(500);}, 500);	
			}

			if(isPage("noWan_page")) setTimeout(arguments.callee, 2000);
		}, 1)
	}
	else{
		if(!isSupport("lyra_hide"))
			$("#noWanEth").show();
	}

	setTimeout(function(){
		systemVariable.detwanResult = httpApi.detwanGetRet();		
		if(systemVariable.manualWanSetup) return false;

		switch(systemVariable.detwanResult.wanType){
			case "CONNECTED":
				goTo.Wireless();
				break;
			case "DHCP":
				goTo.DHCP();
				break;
			case "PPPoE":
				goTo.PPPoE();
				break;
			case "STATIC":
				goTo.Static();
				break;
			case "RESETMODEM":
				goTo.ResetModem();
				break;
			default:
				if(isPage("noWan_page")) setTimeout(arguments.callee, 1000);
				break;
		}
	}, 1000);

	goTo.loadPage("noWan_page", false);	
};

goTo.ResetModem = function(){
	if(!isSku("US") && !isSku("CA") && !isSku("EU") && !isSku("TW")){
		goTo.WAN();
		return false;
	}

	setTimeout(function(){
		systemVariable.detwanResult = httpApi.detwanGetRet();
		if(!isPage("resetModem_page")) return false;

		if($("#resetModem").hasClass("unplug")){
			$("#resetModem").removeClass("unplug").addClass("plug")
		}
		else{
			$("#resetModem").removeClass("plug").addClass("unplug")
		}

		if(
			systemVariable.detwanResult.wanType != "" && 
			systemVariable.detwanResult.wanType != "CHECKING" &&
			systemVariable.detwanResult.wanType != "RESETMODEM"
		){
			goTo.autoWan();			
		}

		if(isPage("resetModem_page")) setTimeout(arguments.callee, 1500);
	}, 1500);

	goTo.loadPage("resetModem_page", false);	
};

goTo.Waiting = function(){
	systemVariable.manualWanSetup = false;

	var errCount = 0;
	setTimeout(function(){
		if(systemVariable.manualWanSetup) return false;

		if(errCount > 10){
			goTo.WAN();
			return false;
		}

		if(systemVariable.detwanResult.wanType == "" || systemVariable.detwanResult.wanType == "CHECKING"){
			errCount++;
			systemVariable.detwanResult = httpApi.detwanGetRet();
			if(isPage("waiting_page")) setTimeout(arguments.callee, 1000);
			return false;
		}

		if(isPage("waiting_page")) goTo.autoWan();
	}, 1000);

	goTo.loadPage("waiting_page", false);	
};

goTo.leaveQIS = function(){
	if(isSupport("amas") && isSupport("amas_bdl") && (isSwMode("RT") || isSwMode("AP"))){
		goTo.loadPage("amasbundle_page", false);
	}
	else{
		location.href = "/";
	}
}

goTo.AsusPP = function(){
	goTo.loadPage("asuspp_page", false);
};

goTo.AsusToS = function(){
	goTo.loadPage("asustos_page", false);
};

goTo.TMToS = function(){
	goTo.loadPage("tmtos_page", false);
};

goTo.loadPage = function(page, _reverse){
	if(_reverse)
		systemVariable.historyPage.pop();
	else
		systemVariable.historyPage.push(page);

	var $obj = $("#"+page);
	if($obj.find($(".pageDesc")).length === 0) $obj.load("/mobile/pages/" + page + ".html", handleSysDep);
	$.mobile.changePage("#"+page, {transition: "slide", changeHash: false, reverse: _reverse});
};

goTo.skip_pap = function(){
	var available_band = Get_Value_Available_WL_Band();
	systemVariable.multiPAP.wlcStatus["wlc" + available_band[0] + "_checked"] = true;
	systemVariable.multiPAP.wlcStatus["wlc" + available_band[0] + "_skip"] = true;
	systemVariable.multiPAP.wlcOrder.push(available_band[0]);
	postDataModel.insert(wlcMultiObj["wlc" + available_band[0]]);
	var retData = wlcMultiObj["wlc" + available_band[0]];
	Object.keys(retData).forEach(function(key){
		qisPostData[key] = retData[key];
	});
	if(available_band.length == 1)
		goTo.GetLanIp();
	else{
		genPAPList(systemVariable.papList, systemVariable.multiPAP.wlcOrder);
		goTo.loadPage("papList_page", false);
	}
};

goTo.lanIP_papList = function(){
	var allPAPSet = true;
	if(isSwMode("RP") && isSupport("concurrep"))
		allPAPSet = isAllPAPSet();

	if(allPAPSet) {
		goTo.GetLanIp();
	}
	else {
		genPAPList(systemVariable.papList, systemVariable.multiPAP.wlcOrder);
		goTo.loadPage("papList_page", false);
	}
};

goTo.Yadns = function(){
	goTo.loadPage("yadns_page", false);
};
