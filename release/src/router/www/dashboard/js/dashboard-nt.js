export function getNotifications() {
	// update item
	var webs_state_flag = httpApi.nvramGet(["webs_state_flag"], true).webs_state_flag;
	var get_cfg_clientlist = httpApi.hookGet("get_cfg_clientlist", true);
	var get_newob_onboardinglist = isSupport("AMAS_NEWOB") ? httpApi.hookGet("get_newob_onboardinglist", true) : {};
	var notification = [];

	var isNewOnboarding = (function(){
		return Object.keys(get_newob_onboardinglist).length > 0;
	})()

	var isNewFw = (function(){		
		var is_aimesh_fw_flag = false;
		for (var idx in get_cfg_clientlist) {
			if(get_cfg_clientlist.hasOwnProperty(idx)) {
				if(get_cfg_clientlist[idx].online == "1") {
					if(get_cfg_clientlist[idx].newfwver != "") {
						is_aimesh_fw_flag = true;
						break;
					}
				}
			}
		}
		return is_aimesh_fw_flag || webs_state_flag == 1;
	})()

	var isForceNewFw = (function(){		
		return webs_state_flag == 2;
	})()

	var isFtpAnonymous = (function(){		
		var enable_ftp = httpApi.nvramGet(["enable_ftp"]).enable_ftp == 1;
		var st_ftp_mode = httpApi.nvramGet(["st_ftp_mode"]).st_ftp_mode == 1;
		return enable_ftp && st_ftp_mode;
	})()

	var isSambaAnonymous = (function(){		
		var enable_samba = httpApi.nvramGet(["enable_samba"]).enable_samba == 1;
		var st_samba_mode = httpApi.nvramGet(["st_samba_mode"]).st_samba_mode == 1;
		return enable_samba && st_samba_mode;
	})()

	var isChtPppoe = (function(){
		var tcode = httpApi.nvramGet(["territory_code"]).territory_code.indexOf("TW") != -1;
		var autodet_state = httpApi.nvramGet(["autodet_state"]).autodet_state == 2;
		var autodet_auxstate = httpApi.nvramGet(["autodet_auxstate"]).autodet_auxstate == 6;
		var wan_proto = httpApi.nvramGet(["wan_proto"]).wan_proto != "pppoe";
		return tcode && autodet_state && autodet_auxstate && wan_proto;
	})()

	if(isNewOnboarding){
		notification.push({
			"id": "noti_amas_newob",
			"title": `AiMesh`,
			"description": `New AiMesh device connected via Ethernet`,
			"actionBtn": `<#btn_goSetting#>`,
			"callback": function(){pageRedirect("aimesh")}
		})
	}

	if(isNewFw){
		notification.push({
			"id": "noti_upgrade",
			"title": `<#menu5_6_3#>`,			
			"description": `<#ASUSGATE_note2#>`,
			"actionBtn": `<#ASUSGATE_act_update#>`,
			"callback": function(){pageRedirect("settings", "Advanced_FirmwareUpgrade_Content.asp?confirm_show=0")}
		})
	}

	if(isForceNewFw){
		notification.push({
			"id": "noti_force_upgrade",
			"title": `<#menu5_6_3#>`,			
			"description": `<#exist_new_force#>`,
			"actionBtn": `<#ASUSGATE_act_update#>`,
			"callback": function(){pageRedirect("settings", "Advanced_FirmwareUpgrade_Content.asp?confirm_show=0")}
		})
	}
	
	if(isFtpAnonymous){
		notification.push({
			"id": "noti_ftp",
			"title": `<#menu5_4_2#>`,			
			"description": `<#ASUSGATE_note4#>`,
			"actionBtn": ``,
			"callback": function(){pageRedirect("settings", "Advanced_AiDisk_ftp.asp")}
		})
	}

	if(isSambaAnonymous){
		notification.push({
			"id": "noti_ftp",
			"title": `<#menu5_4_1#>`,			
			"description": `<#ASUSGATE_note5#>`,
			"actionBtn": ``,
			"callback": function(){pageRedirect("settings", "Advanced_AiDisk_samba.asp")}
		})
	}

	if(isChtPppoe){
		notification.push({
			"id": "noti_pppoe_tw",
			"title": `<#menu5_3#>`,			
			"description": `<#CHT_ppp_notice_1#>`,
			"actionBtn": `<#CHT_ppp_notice_2#>`,
			"callback": function(){pageRedirect("settings", "Advanced_WAN_Content.asp?af=wan_proto")}
		})
	}

	return notification;
}