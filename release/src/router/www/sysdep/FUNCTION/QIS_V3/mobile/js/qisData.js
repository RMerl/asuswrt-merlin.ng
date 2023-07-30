var qisPostData = {};

var opModeObj = {
	"sw_mode": "",
	"wlc_psta": "",
	"wlc_dpsta": ""
}

var generalObj = {
	"x_Setting": "1",
	"w_Setting": "1",
	"qis_Setting": "1"
}

var wanObj = {
	"general":{
		"wan_unit": "",
		"wan_proto": ""
	},

	"dhcp": {
		"wan_dhcpenable_x": "1",
		"wan_dnsenable_x": "1"
	},

	"pppoe": {
		"wan_dhcpenable_x": "1",
		"wan_dnsenable_x": "1",
		"wan_pppoe_username": "",
		"wan_pppoe_passwd": ""
	},

	"staticIp": {
		"wan_dhcpenable_x": "0",
		"wan_dnsenable_x": "0",
		"wan_ipaddr_x": "",
		"wan_netmask_x": "",
		"wan_gateway_x": "",
		"wan_dns1_x": "",
		"wan_dns2_x": ""
	},

	"v6plus": {
		"ipv6_service": "ipv6pt"
	},

	"ocnvc": {
		"ipv6_service": "ipv6pt"
	},

	"wan46": {
		"wan_proto": "",
		"ipv6_service": "ipv6pt"
	},
	
	"vpn": {
		"wan_heartbeat_x": ""
	},

	"all": {
		"wan_unit": "",
		"wan_proto": "",
		"wan_dhcpenable_x": "",
		"wan_dnsenable_x": "",
		"wan_pppoe_username": "",
		"wan_pppoe_passwd": "",
		"wan_dhcpenable_x": "",
		"wan_dnsenable_x": "",
		"wan_ipaddr_x": "",
		"wan_netmask_x": "",
		"wan_gateway_x": "",
		"wan_dns1_x": "",
		"wan_dns2_x": "",
		"wan_heartbeat_x": "",
		"ipv6_service": ""
	},

	"LWAN_2p5G": {
		"wans_extwan": "0"
	},

	"dualWan": {
		"wans_dualwan": ""
	}
}

var lanObj = {
	"general":{
		"lan_proto": "",
		"lan_dnsenable_x": ""
	},

	"staticIp": {
		"lan_ipaddr": "",
		"lan_gateway": "",
		"lan_netmask": "",
		"lan_dns2_x": "",
		"lan_dns1_x": ""
	}		
}

var wirelessObj = {
	"wl0": {
		"wl0_ssid": "",
		"wl0_wpa_psk": "",
		"wl0_auth_mode_x": "",
		"wl0_crypto": "",
		"wl0_mfp": ""
	},
	"wl1": {
		"wl1_ssid": "",
		"wl1_wpa_psk": "",
		"wl1_auth_mode_x": "",
		"wl1_crypto": "",
		"wl1_mfp": ""
	},
	"wl2": {
		"wl2_ssid": "",
		"wl2_wpa_psk": "",
		"wl2_auth_mode_x": "",
		"wl2_crypto": "",
		"wl2_mfp": ""
	},
	"wl3": {
		"wl3_ssid": "",
		"wl3_wpa_psk": "",
		"wl3_auth_mode_x": "",
		"wl3_crypto": ""
	}
}

var smartConnectObj = {
	"smart_connect_x": "1"
}

var fronthaulNetworkObj = {
	"fh_ap_enabled": "2",
	"acs_unii4": "0"
}

var wlcObj = {
	"wlc_band": "",
	"wlc_ssid": "",
	"wlc_auth_mode": "",
	"wlc_crypto": "",
	"wlc_wpa_psk": "",
	"wlc_wep": "",
	"wlc_wep_key": "",
	"wlc_key": "",
	"wlc_ap_mac": ""
}
var wlcMultiObj = {
	"wlc0" : {
		"wlc0_band": "",
		"wlc0_ssid": "",
		"wlc0_auth_mode": "",
		"wlc0_crypto": "",
		"wlc0_wpa_psk": "",
		"wlc0_wep": "",
		"wlc0_wep_key": "",
		"wlc0_key": "",
		"wlc0_ap_mac": ""
	},
	"wlc1" : {
		"wlc1_band": "",
		"wlc1_ssid": "",
		"wlc1_auth_mode": "",
		"wlc1_crypto": "",
		"wlc1_wpa_psk": "",
		"wlc1_wep": "",
		"wlc1_wep_key": "",
		"wlc1_key": "",
		"wlc1_ap_mac": ""
	},
	"wlc2" : {
		"wlc2_band": "",
		"wlc2_ssid": "",
		"wlc2_auth_mode": "",
		"wlc2_crypto": "",
		"wlc2_wpa_psk": "",
		"wlc2_wep": "",
		"wlc2_wep_key": "",
		"wlc2_key": "",
		"wlc2_ap_mac": ""
	}
}
var yandexObj = {
	"yadns_enable_x": "",
	"yadns_mode": ""	
}

var userObj = {
	"http_username": "",
	"http_passwd": ""
}

var bwdpiObj = {
	"wrs_protect_enable": "1",
	"wrs_mals_t": "0",
	"wrs_cc_t": "0",
	"wrs_vp_t": "0",
	"TM_EULA": "1"
}

var timeObj = {
	"time_zone": "",
	"time_zone_dst": ""
}

var iptvObj = {
	"switch_stb_x": "",
	"switch_wantag": ""
}

var wanDhcpOptionObj = {
	"wan_vendorid": "",
	"wan_clientid_type": "",
	"wan_clientid": ""
}

var iptvManualObj = {
	"switch_wan0tagid": "",
	"switch_wan0prio": "",
	"switch_wan1tagid": "",
	"switch_wan1prio": "",
	"switch_wan2tagid": "",
	"switch_wan2prio": ""
}

var iptvWanSettingObj = {
	"wan10_proto": "",
	"wan11_proto": "",
	"wan10_ipaddr_x": "",
	"wan10_netmask_x": "",
	"wan10_gateway_x": "",
	"wan10_dns1_x": "",
	"wan10_dns2_x": "",
	"wan10_auth_x": "",
	"wan10_pppoe_username": "",
	"wan10_pppoe_passwd": ""
}

var modemObj = {
	"wans_dualwan": "wan usb",
	"modem_enable": "",
	"modem_android": "",
	"modem_autoapn": "",
	"modem_country": "",
	"modem_isp": "",
	"modem_apn": "",
	"modem_dialnum": "",
	"modem_pincode": "",
	"modem_user": "",
	"modem_pass": "",
	"modem_ttlsid": "",
	"Dev3G": "",
	"modem_authmode": "",
	"modem_mtu": ""
}

var systemVariable = {
	"originWanType": "", /* ToDo: Secondary WAN support */
	"originPppAccount":{
		"username": "",
		"password": ""
	},
	"maxPasswordLen": 32,
	"originWansExtwan": "",
	"originWansDualwan": "",
	"manualWanSetup": false,
	"wanDnsenable": "",
	"detwanResult": {
		"wanType": "",
		"isIPConflict": false,
	},
	"detwan46Result": {
		"wan46State": ""
	},
	"isNewFw": 0,
	"newFwVersion": "",
	"forceLevel": "",
	"papList": [],
	"papListAiMesh": [],
	"selectedAP": {},
	"opMode": "",
	"originOpMode": "",
	"productid": "",
	"rcSupport": "",
	"isDefault": "",
	"uiLanguage": "",
	"territoryCode": "",
	"default_http_passwd": "admin",
	"forceChangePw": false,
	"forceChangePwInTheEnd": false,
	"rebootTime": "",
	"multiPAP" : {
		"wlcOrder": [],
		"wlcStatus": {}
	},
	"wanOption": false,
	"interval_status": false,
	"modelCloudIcon": [],
	"onboardingInfo": {},
	"skipAiMeshOptionPage": false,
	"amas_newWindow_addNode": false,
	"authModePostData": {},
	"eth_wan_list":{},
}

var aimeshObj = {
	"cfg_master": "1"
}

var boostKeyObj = {
	"turbo_mode": "1"
}

var axModeObj = {
	"wl0_11ax": "1",
	"wl1_11ax": "1",
	"wl2_11ax": "1"
}

var dsl_wanObj = {
	
	"PPPOEObj" : {
		"dsl_pppoe_username": "",
		"dsl_pppoe_passwd": "",
		"dsl_pppoe_idletime": "",
		"dsl_pppoe_mtu": "",
		"dsl_pppoe_mru": "",
		"dsl_pppoe_service": "",
		"dsl_pppoe_options": "",
		"dsl_pppoe_ac": "",
		"dsl_pppoe_relay": "",
		"dsl_DHCPClient": "1",
		"dsl_dnsenable": "1"
	},

	"atm_dslObj" : {
		"dsl_enable": "1",
		"dsl_unit": "0",
		"dsl_vpi": "",
		"dsl_vci": "",
		"dsl_proto": "",
		"dsl_encap": "",
		"dsl_dot1q": "",
	 	"dsl_vid": "",
 		"dsl_svc_cat": "",
	 	"dsl_pcr": "",
 		"dsl_scr": "",
	 	"dsl_mbs": ""
	},

	"ptm_dslObj" : {
		"dsl_enable": "1",
		"dsl_unit": "8",
		"dsl_proto": "",
		"dsl_dot1q": "",
		"dsl_vid": "",
	},

	"dslDHCPObj" : {
		"dsl_hwaddr": "",
		"dsl_DHCPClient": "1",
		"dsl_ipaddr": "",
		"dsl_netmask": "",
		"dsl_gateway": "",
		"dsl_dnsenable": "1",
		"dsl_dns1": "",
		"dsl_dns2": "",
		"dsl_dhcp_clientid": "",
		"dsl_dhcp_hostname": ""
	},

	"atm_all" : {
		"dsl_pppoe_username": "",
		"dsl_pppoe_passwd": "",
		"dsl_pppoe_idletime": "",
		"dsl_pppoe_mtu": "",
		"dsl_pppoe_mru": "",
		"dsl_pppoe_service": "",
		"dsl_pppoe_options": "",
		"dsl_pppoe_ac": "",
		"dsl_pppoe_relay": "",
		"dsl_enable": "",
		"dsl_unit": "",
		"dsl_vpi": "",
		"dsl_vci": "",
		"dsl_proto": "",
		"dsl_encap": "",
		"dsl_dot1q": "",
	 	"dsl_vid": "",
 		"dsl_svc_cat": "",
	 	"dsl_pcr": "",
 		"dsl_scr": "",
	 	"dsl_mbs": "",
	 	"dsl_hwaddr": "",
		"dsl_DHCPClient": "",
		"dsl_ipaddr": "",
		"dsl_netmask": "",
		"dsl_gateway": "",
		"dsl_dnsenable": "",
		"dsl_dns1": "",
		"dsl_dns2": "",
		"dsl_dhcp_clientid": "",
		"dsl_dhcp_hostname": ""
	},

	"ptm_all" : {
		"dsl_pppoe_username": "",
		"dsl_pppoe_passwd": "",
		"dsl_pppoe_idletime": "",
		"dsl_pppoe_mtu": "",
		"dsl_pppoe_mru": "",
		"dsl_pppoe_service": "",
		"dsl_pppoe_options": "",
		"dsl_pppoe_ac": "",
		"dsl_pppoe_relay": "",
		"dsl_enable": "",
		"dsl_proto": "",
		"dsl_dot1q": "",
		"dsl_vid": "",
		"dsl_hwaddr": "",
		"dsl_DHCPClient": "",
		"dsl_ipaddr": "",
		"dsl_netmask": "",
		"dsl_gateway": "",
		"dsl_dnsenable": "",
		"dsl_dns1": "",
		"dsl_dns2": "",
		"dsl_dhcp_clientid": "",
		"dsl_dhcp_hostname": ""
	}
}

var dsltmpQISObj = {
	"dslx_transmode": "",
	"dsltmp_transmode": "",
	"dsl_hwaddr": "",
	"dsl_nat": "1",
	"dsl_upnp_enable": "1"
}

var dslIPTVObj = {
	"dsltmp_cfg_iptv_enable": "0", 	//{1: for iptv enable, 0: for iptv disable}
	"dsltmp_cfg_iptv_rmvlan": "",
	"dsltmp_cfg_iptv_mr": "",
	"dsltmp_cfg_iptv_num_pvc": "0",
	"dsltmp_cfg_iptv_pvclist": "",
	"dsltmp_cfg_iptv_idx": "",
	"dsltmp_cfg_ispname": "",
	"dsltmp_cfg_country": ""
}
