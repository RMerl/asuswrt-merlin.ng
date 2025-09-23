﻿/* menuTree_ROG.js */
define(function(){
	var menuTree = {
		list: [
			/*
			{
				menuName: "Title",
				index: "Assign an index to this menu, it is also used for filtering",
				tab: [
					{
						url: "Put url here", 
						tabName: "
								Assign a title for this tab, leave this field empty to bypass this tab, 
								fill in '__HIDE__' to hide the tab switcher,
								fill in '__INHERIT__' to inhert tab index from referred page.
						"
					}
				]
			}
			*/
			{
				menuName: "<#QIS#>",
				index: "menu_QIS", 
				tab: [
					{url: "QIS_wizard.htm", tabName: "__HIDE__"},
					{url: "NULL", tabName: "__INHERIT__"}
				] 
			},
			/* ============================================================================================================ */
			{
				menuName: "<#menu5_1_1#>",
				index: "menu_Split", 
				tab: [
					{url: "NULL", tabName: "__HIDE__"}
				] 
			},
			{
				menuName: "<#AiProtection_title_Dashboard_title#>",
				index: "menu_Dashboard",
				tab: [
					{url: "GameDashboard.asp", tabName: "__HIDE__"},
					{url: "NULL", tabName: "__INHERIT__"}
				]
			},
			{
				menuName: "AiMesh",
				index: "menu_AiMesh", 
				tab: [
					{url: "AiMesh.asp", tabName: "AiMesh"},
					{url: "NULL", tabName: "__INHERIT__"}
				]
			},
			{
				menuName: "<#AiProtection_title_Dashboard_title#>",
				index: "menu_NewDashboard",
				tab: [
					{url: "index.html", tabName: "__HIDE__"},
					{url: "NULL", tabName: "__INHERIT__"}
				]
			},
			{
				menuName: "<#AiProtection_title#>",
				index: "menu_AiProtection", 
				tab: [
					{url: "AiProtection_HomeProtection.asp", tabName: "<#AiProtection_Home#>"},
					{url: "AiProtection_MaliciousSitesBlocking.asp", tabName: "<#AiProtection_sites_blocking#>"},
					{url: "AiProtection_IntrusionPreventionSystem.asp", tabName: "<#AiProtection_two-way_IPS#>"},
					{url: "AiProtection_InfectedDevicePreventBlock.asp", tabName: "<#AiProtection_detection_blocking#>"},
					{url: "AiProtection_AdBlock.asp", tabName: "Ad Blocking"},
					{url: "AiProtection_Key_Guard.asp", tabName: "Key Guard"},
					{url: "NULL", tabName: "__INHERIT__"}
				] 
			},
			{
				menuName: "<#Parental_Control#>",
				index: "menu_ParentalControl", 
				tab: [
					{url: "AiProtection_WebProtector.asp", tabName: "<#AiProtection_filter#>"},
					{url: "ParentalControl.asp", tabName: "<#Time_Scheduling#>"},
					{url: "YandexDNS.asp", tabName: "<#YandexDNS#>"},
					{url: "adGuard_DNS.asp", tabName: "AdGuard"},
					{url: "NULL", tabName: "__INHERIT__"}
				] 
			},
			{
				menuName: "<#Game_acceleration#>",
				index: "menu_GameBoost",
				tab: [
					{url: "GameBoost.asp", tabName: "<#Game_acceleration#>"},
					{url: "QoS_EZQoS.asp", tabName: "<#menu5_3_2#>"},
					{url: "QoS_Stats.asp", tabName: "Classification"},
					{url: "Advanced_QOSUserPrio_Content.asp", tabName: "__INHERIT__"},
					{url: "Advanced_QOSUserRules_Content.asp", tabName: "__INHERIT__"},
					{url: "Advanced_WTFast_Content.asp", tabName: "WTFast"},
					{url: "GearAccelerator.asp", tabName: "<#Gear_Accelerator#>"},
					{url: "AdaptiveQoS_InternetSpeed.asp", tabName: "<#InternetSpeed#>"},
					{url: "NULL", tabName: "__INHERIT__"}
				] 
			},
			{
				menuName: "Open NAT",
				index: "menu_GameProfile", 
				tab: [
					{url: "GameProfile.asp", tabName: "Open NAT"},
					{url: "NULL", tabName: "__INHERIT__"}
				] 
			},
			{
				menuName: "<#AiProtection_title_Radar#>",
				index: "menu_GameRadar", 
				tab: [
					{url: "Main_GameServer_Content.asp", tabName: "<#AiProtection_title_Radar#>"},
					{url: "NULL", tabName: "__INHERIT__"}
				] 
			},
			{
				menuName: "<#WiFi_radar#>",
				index: "menu_WifiRadar", 
				tab: [
					{url: "WiFi_Insight.asp", tabName: "<#WiFi_radar#>"},
					{url: "NULL", tabName: "__INHERIT__"}
				] 
			},
			{
				menuName: "VPN",
				index: "menu_VPN", 
				tab: [
					{url: "Advanced_VPNStatus.asp", tabName: "VPN Status"},
					{url: "Advanced_VPNDirector.asp", tabName: "VPN Director"},
					{url: "Advanced_VPN_OpenVPN.asp", tabName: "<#BOP_isp_heart_item#>"},
					{url: "Advanced_VPNServer_Content.asp", tabName: "__INHERIT__"},
					{url: "Advanced_OpenVPNClient_Content.asp", tabName: "<#vpnc_title#>"},
					{url: "Advanced_VPNClient_Content.asp", tabName: "__INHERIT__"},
					{url: "Advanced_TOR_Content.asp", tabName: "TOR"},
					{url: "Advanced_Instant_Guard.asp", tabName: "<#Instant_Guard_title#>"},
					{url: "Advanced_WireguardClient_Content.asp", tabName: "__INHERIT__"},
					{url: "NULL", tabName: "__INHERIT__"}
				]
			},
			{
				menuName: "<#Traffic_Analyzer#>",
				index: "menu_TrafficAnalyzer", 
				tab: [
					{url: "AdaptiveQoS_Bandwidth_Monitor.asp", tabName: "<#Bandwidth_monitor#>"},
					{url: "Main_TrafficMonitor_realtime.asp", tabName: "<#traffic_monitor#>"},
					{url: "Main_TrafficMonitor_last24.asp", tabName: "__INHERIT__"},
					{url: "Main_TrafficMonitor_daily.asp", tabName: "__INHERIT__"},
					{url: "Main_TrafficMonitor_monthly.asp", tabName: "__INHERIT__"},
					{url: "TrafficAnalyzer_Statistic.asp", tabName: "<#Statistic#>"},
					{url: "AdaptiveQoS_TrafficLimiter.asp", tabName: "Traffic Limiter"},
					{url: "AdaptiveQoS_WebHistory.asp", tabName: "<#Adaptive_History#>"},
					{url: "NULL", tabName: "__INHERIT__"}
				] 
			},
			{
				menuName: "Tools",
				index: "menu_Tools",
				tab: [
					{url: "Tools_Sysinfo.asp", tabName: "Sysinfo"},
					{url: "Tools_OtherSettings.asp", tabName: "Other Settings"},
					{url: "NULL", tabName: "__INHERIT__"}
				] 
			},

			/* ============================================================================================================ */

			{
				menuName: "<#menu5#>",
				index: "menu_Split",
				tab: [
					{url: "NULL", tabName: "__HIDE__"}
				] 
			},
			{
				menuName: "<#menu1#>",
				index: "menu_Index",
				tab: [
					{url: "<% networkmap_page(); %>", tabName: "__HIDE__"},
					{url: "NULL", tabName: "__INHERIT__"}
				]
			}, 
			{
				menuName: "<#menu5_1#>",
				index: "menu_Wireless",
				tab: [
					{url: "Advanced_Wireless_Content.asp", tabName: "<#menu5_1_1#>"},
					{url: "Advanced_WWPS_Content.asp", tabName: "<#menu5_1_2#>"},
					{url: "Advanced_WMode_Content.asp", tabName: "WDS"},
					{url: "Advanced_ACL_Content.asp", tabName: "<#menu5_1_4#>"},
					{url: "Advanced_WSecurity_Content.asp", tabName: "<#menu5_1_5#>"},
					{url: "Advanced_WAdvanced_Content.asp", tabName: "<#menu5_1_6#>"},
					{url: "Advanced_WProxy_Content.asp", tabName: "<#WiFi_Proxy_item#>"},
					{url: "Advanced_Roaming_Block_Content.asp", tabName: "<#WiFi_Roaming_Block_List#>"},
					{url: "MLO.asp", tabName: `MLO`},
					{url: "NULL", tabName: "__INHERIT__"}
				] 
			},
			{
				menuName: Guest_Network_naming,
				index: "menu_GuestNetwork",
				tab: [
					{url: (isSupport("mtlancfg") ? "SDN.asp" : "Guest_network.asp"), tabName: Guest_Network_naming},
					{url: "Captive_Portal.asp", tabName: "Free WiFi"},
					{url: "Captive_Portal_Advanced.asp", tabName: "<#Captive_Portal#>"},
					{url: "Guest_network_fbwifi.asp", tabName: "Facebook WiFi"},
					{url: "NULL", tabName: "__INHERIT__"}
				]
			},
			{
				menuName: "<#menu5_2#>",
				index: "menu_LAN",
				tab: [
					{url: "Advanced_LAN_Content.asp", tabName: "<#menu5_2_1#>"},
					{url: "Advanced_DHCP_Content.asp", tabName: "<#menu5_2_2#>"},
					{url: "DNSDirector.asp", tabName: "DNS Director"},
					{url: "Advanced_MultiSubnet_Content.asp", tabName: "<#menu5_2_2#>"},
					{url: "Advanced_GWStaticRoute_Content.asp", tabName: "<#menu5_2_3#>"},
					{url: "Advanced_IPTV_Content.asp", tabName: "IPTV"},
					{url: "Advanced_SwitchCtrl_Content.asp", tabName: "<#Switch_itemname#>"},
					{url: "Advanced_VLAN_Switch_Content.asp", tabName: "VLAN"},
					{url: "Advanced_VLAN_Profile_Content.asp", tabName: "__INHERIT__"},
					{url: "NULL", tabName: "__INHERIT__"}
				] 
			},
			{
				menuName: "<#menu5_3#>",
				index: "menu_WAN",
				tab: [
					{url: "Advanced_WAN_Content.asp", tabName: "<#menu5_3_1#>"},
					{url: "Advanced_DSL_Content.asp", tabName: "<#menu5_3_1#>"},
					{url: "Advanced_Modem_Content.asp", tabName: "__INHERIT__"},
					{url: "Advanced_MobileBroadband_Content.asp", tabName: "__INHERIT__"},
					{url: "Advanced_WANPort_Content.asp", tabName: "<#dualwan#>"},
					{url: "Advanced_PortTrigger_Content.asp", tabName: "<#menu5_3_3#>"},
					{url: "Advanced_VirtualServer_Content.asp", tabName: "<#menu5_3_4#>"},
					{url: "Advanced_Exposed_Content.asp", tabName: "<#menu5_3_5#>"},
					{url: "Advanced_ASUSDDNS_Content.asp", tabName: "<#menu5_3_6#>"},
					{url: "Advanced_NATPassThrough_Content.asp", tabName: "<#NAT_passthrough_itemname#>"},
					{url: "NULL", tabName: "__INHERIT__"}
				]
			},
			{
				menuName: "<#Menu_usb_application#>",
				index: "menu_APP", 
				tab: [
					{url: "APP_Installation.asp", tabName: "__HIDE__"},
					{url: "aidisk.asp", tabName: "__INHERIT__"},
					{url: "mediaserver.asp", tabName: "<#UPnPMediaServer#>"},
					{url: "Advanced_AiDisk_samba.asp", tabName: "<#menu5_4_1#>".concat(WebDav_support?" / <#Cloud_Disk#>":"")},
					{url: "Advanced_AiDisk_NFS.asp", tabName: "NFS Exports"},
					{url: "Advanced_AiDisk_ftp.asp", tabName: "<#menu5_4_2#>"},
					{url: "PrinterServer.asp", tabName: "__INHERIT__"},
					{url: "Advanced_Modem_Content.asp", tabName: "__INHERIT__"},
					{url: "Advanced_TimeMachine.asp", tabName: "__INHERIT__"},
					{url: "fileflex.asp", tabName: "__INHERIT__"},
					{url: "NULL", tabName: "__INHERIT__"}
				] 
			},
			{
				menuName: "<#AiCloud_Title#>",
				index: "menu_AiCloud", 
				tab: [
					{url: "cloud_main.asp", tabName: "AiCloud 2.0"},
					{url: "cloud_sync.asp", tabName: "<#smart_sync#>"},
					{url: "cloud_router_sync.asp", tabName: "<#Server_Sync#>"},
					{url: "cloud_settings.asp", tabName: "<#Settings#>"},
					{url: "cloud_syslog.asp", tabName: "<#Log#>"},
					{url: "NULL", tabName: "__INHERIT__"}
				] 
			},
			{
				menuName: "Alexa & IFTTT",
				index: "menu_Alexa_IFTTT",
				tab: [
					{url: "Advanced_Smart_Home_Alexa.asp", tabName: "__INHERIT__"},
					{url: "Advanced_Smart_Home_IFTTT.asp", tabName: "__INHERIT__"},
					{url: "NULL", tabName: "__INHERIT__"}
				]
			},
			{
				menuName: "IPv6",
				index: "menu_IPv6",
				tab: [
					{url: "Advanced_IPv6_Content.asp", tabName: "IPv6"},
					{url: "Advanced_IPv61_Content.asp", tabName: "__INHERIT__"},
					{url: "NULL", tabName: "__INHERIT__"}
				]
			},
			{
				menuName: "<#menu5_5#>",
				index: "menu_Firewall",
				tab: [
					{url: "Advanced_BasicFirewall_Content.asp", tabName: "<#menu5_1_1#>"},
					{url: "Advanced_URLFilter_Content.asp", tabName: "<#menu5_5_2#>"},
					{url: "Advanced_KeywordFilter_Content.asp", tabName: "<#menu5_5_5#>"},
					{url: "Advanced_Firewall_Content.asp", tabName: "<#menu5_5_4#>"},			
					{url: "NULL", tabName: "__INHERIT__"}
				]
			},
			{
				menuName: "<#menu5_6#>",
				index: "menu_Setting",
				tab: [
					{url: "Advanced_OperationMode_Content.asp", tabName: "<#menu5_6_1#>"},
					{url: "Advanced_System_Content.asp", tabName: "<#menu5_6_2#>"},
					{url: "Advanced_FirmwareUpgrade_Content.asp", tabName: "<#menu5_6_3#>"},
					{url: "Advanced_SettingBackup_Content.asp", tabName: "<#menu5_6_4#>"},
					{url: "Advanced_PerformanceTuning_Content.asp", tabName: "Temperature"},
					{url: "Advanced_ADSL_Content.asp", tabName: "<#menu_dsl_setting#>"},
					{url: "Advanced_Feedback.asp", tabName: "<#menu_feedback#>"},
					{url: "Feedback_Info.asp", tabName: "__INHERIT__"},
					{url: "Advanced_SNMP_Content.asp", tabName: "SNMP"},
					{url: "Advanced_TR069_Content.asp", tabName: "TR-069"},
					{url: "Advanced_Notification_Content.asp", tabName: "Notification"},
					{url: "Advanced_Privacy.asp", tabName: "<#menu_privacy#>"},
					{url: "NULL", tabName: "__INHERIT__"}
				]
			},
			{
				menuName: "<#System_Log#>",
				index: "menu_Log",
				tab: [
					{url: "Main_LogStatus_Content.asp", tabName: "<#menu5_7_2#>"},
					{url: "Main_WStatus_Content.asp", tabName: "<#menu5_7_4#>"},
					{url: "Main_DHCPStatus_Content.asp", tabName: "<#menu5_7_3#>"},
					{url: "Main_IPV6Status_Content.asp", tabName: "IPv6"},
					{url: "Main_RouteStatus_Content.asp", tabName: "<#menu5_7_6#>"},
					{url: "Main_IPTStatus_Content.asp", tabName: "<#menu5_7_5#>"},
					{url: "Main_AdslStatus_Content.asp", tabName: "<#menu_dsl_log#>"},
					{url: "Main_ConnStatus_Content.asp", tabName: "<#Connections#>"},
					/* {url: "###Main_ConnStatus_Content.asp", tabName: "Captive Portal Connection Log"}, */
					{url: "NULL", tabName: "__INHERIT__"}
				]
			},
			{
				menuName: "<#Network_Tools#>",
				index: "menu_NekworkTool",
				tab: [
					{url: "Main_Analysis_Content.asp", tabName: "<#Network_Analysis#>"},
					{url: "Main_Netstat_Content.asp", tabName: "Netstat"},
					{url: "Main_WOL_Content.asp", tabName: "<#NetworkTools_WOL#>"},
					{url: "Advanced_Smart_Connect.asp", tabName: "<#smart_connect_rule#>"},
					{url: "Advanced_Wireless_Survey.asp", tabName: "Site Survey"},
					{url: "NULL", tabName: "__INHERIT__"}
				]
			}
		],

		exclude: {
			menus: function(){
				var retArray = [];

				if(!wifiRadar_support){
					retArray.push("menu_WifiRadar");
				}

				if(!nfsd_support){
					retArray.push("Advanced_AiDisk_NFS.asp");
				}

				if(!dnsfilter_support){
					retArray.push("DNSDirector.asp");
				}


				if(!multissid_support){
					retArray.push("menu_GuestNetwork");
				}

				if(!dashboard_support){
					retArray.push("menu_NewDashboard");
				}

				if(!isSupport("adaptive_qos") && !isSupport("bandwidth_monitor") && !isSupport("dns_dpi")){	
					for(i=0; i<menuTree.list.length; i++){
						if(menuTree.list[i].menuName == '<#Adaptive_QoS#>'){
							menuTree.list[i].menuName = '<#menu5_3_2#>';
						}
					}
				}

				if(!isSupport("dpi_mals") && !isSupport("dpi_cc") && !isSupport("dpi_vp")){
					retArray.push("menu_AiProtection");
				}
				
				if(!isSupport("traffic_analyzer") && !isSupport("dns_dpi")){
					retArray.push("menu_TrafficAnalyzer");
				}

				if(!usb_support){
					retArray.push("menu_APP");
				}

				if((!cloudsync_support && !aicloudipk_support) || nocloudsync_support || isSupport("BUSINESS")){
					retArray.push("menu_AiCloud");
				}

				if(!ifttt_support && !alexa_support){
					retArray.push("menu_Alexa_IFTTT");
				}

				if(!IPv6_support){
					retArray.push("menu_IPv6");
				}

				if(!networkTool_support){
					retArray.push("menu_NekworkTool");
				}
				
				if(!pptpd_support && !openvpnd_support && !vpnc_support){
					retArray.push("menu_VPN");
				}

				if(!tagged_based_vlan){
					retArray.push("menu_VLAN");
				}

				if(!wtfast_support) {
					retArray.push("menu_Wtfast");
				}

				if(!amesh_support)
					retArray.push("menu_AiMesh");
				else{
					if(ameshRouter_support){
						if(!isSwMode("rt") && !isSwMode("ap"))
							retArray.push("menu_AiMesh");
					}
					else if(ameshNode_support)
						retArray.push("menu_AiMesh");
				}

				/* Operation Mode */
				if(isSwMode("re")){
					retArray.push("menu_GameBoost");
					retArray.push("menu_GuestNetwork");
					retArray.push("menu_AccessControl");
					retArray.push("menu_TrafficAnalyzer");
					retArray.push("menu_Wtfast");
					retArray.push("menu_GameProfile");
					retArray.push("menu_QoS");
					retArray.push("menu_BandwidthMonitor");
					retArray.push("menu_AiProtection");
					retArray.push("menu_WAN");
					retArray.push("menu_IPv6");
					retArray.push("menu_VPN");
					retArray.push("menu_VLAN");
					retArray.push("menu_Firewall");
					retArray.push("menu_ParentalControl");
					retArray.push("menu_Wireless");

					if(ifttt_support || alexa_support){
						retArray.push("menu_Alexa_IFTTT");
					}
				}
				else if(isSwMode("ap")){
					retArray.push("menu_GameBoost");
					retArray.push("menu_AccessControl");
					retArray.push("menu_TrafficAnalyzer");
					retArray.push("menu_Wtfast");
					retArray.push("menu_GameProfile");
					retArray.push("menu_QoS");
					retArray.push("menu_BandwidthMonitor");
					retArray.push("menu_AiProtection");
					retArray.push("menu_WAN");
					retArray.push("menu_IPv6");
					retArray.push("menu_VPN");
					retArray.push("menu_VLAN");
					retArray.push("menu_Firewall");
					retArray.push("menu_ParentalControl");

					if(ifttt_support || alexa_support){
						retArray.push("menu_Alexa_IFTTT");
					}
				}
				else if(isSwMode("mb")){
					retArray.push("menu_GameBoost");
					retArray.push("menu_GuestNetwork");
					retArray.push("menu_AccessControl");
					retArray.push("menu_Wtfast");
					retArray.push("menu_GameProfile");
					retArray.push("menu_WifiRadar");
					retArray.push("menu_TrafficAnalyzer");
					retArray.push("menu_QoS");
					retArray.push("menu_BandwidthMonitor");
					retArray.push("menu_AiProtection");
					retArray.push("menu_Wireless");
					retArray.push("menu_WAN");
					retArray.push("menu_IPv6");
					retArray.push("menu_VPN");
					retArray.push("menu_VLAN");
					retArray.push("menu_Firewall");
					retArray.push("menu_ParentalControl");

					if(ifttt_support || alexa_support){
						retArray.push("menu_Alexa_IFTTT");
					}
				}

				return retArray;
			},

			tabs: function(){
				var retArray = [];

				/* By RC Support */
				if(!wifiRadar_support){
					retArray.push("WiFi_Insight.asp");
				}

				if(!isSupport("dpi_mals")){
					retArray.push("AiProtection_MaliciousSitesBlocking.asp");
				}

				if(!isSupport("dpi_cc")){
					retArray.push("AiProtection_InfectedDevicePreventBlock.asp");
				}

				if(!isSupport("dpi_vp")){
					retArray.push("AiProtection_IntrusionPreventionSystem.asp");
				}

				if(!isSupport("webs_filter")){
					retArray.push("AiProtection_WebProtector.asp");
					var index = -1;
					for(i=0;i<menuTree.list.length;i++){
						if(menuTree.list[i].menuName == '<#AiProtection_title#>'){
							for(j=0;j<menuTree.list[i].tab.length;j++){	
								if(menuTree.list[i].tab[j].url == 'AiProtection_WebProtector.asp'){
									index = j;
								}

								if(menuTree.list[i].tab[j].url == 'ParentalControl.asp'){
									menuTree.list[i].tab[j].tabName = '<#Parental_Control#>';
									break;
								}		
							}

							menuTree.list[i].tab.splice(index, 1);
						}
					}
				}

				if(!isSupport("web_history")){
					retArray.push("AdaptiveQoS_WebHistory.asp");
				}

				if(!isSupport("bandwidth_monitor") && !isSupport("dns_dpi")){
					retArray.push("AdaptiveQoS_Bandwidth_Monitor.asp");
				}

				if(!isSupport("traffic_analyzer") && !isSupport("dns_dpi")){
					retArray.push("TrafficAnalyzer_Statistic.asp");		

					for(i=0;i<menuTree.list.length;i++){
						if(menuTree.list[i].menuName == '<#Adaptive_QoS#>'){
							menuTree.list[i].menuName = '<#Menu_TrafficManager#>';
							var index = menuTree.list[i].tab.length-1;
							menuTree.list[i].tab[index] = {url: "Main_TrafficMonitor_realtime.asp", tabName: "<#traffic_monitor#>"};
							menuTree.list[i].tab.push({url: "Main_TrafficMonitor_last24.asp", tabName: "__INHERIT__"});
							menuTree.list[i].tab.push({url: "Main_TrafficMonitor_daily.asp", tabName: "__INHERIT__"});
							menuTree.list[i].tab.push({url: "NULL", tabName: "__INHERIT__"});
						}
						else if(menuTree.list[i].menuName == '<#menu5_3_2#>'){
							var index = menuTree.list[i].tab.length-1;
							menuTree.list[i].tab[index] = {url: "Main_TrafficMonitor_realtime.asp", tabName: "<#traffic_monitor#>"};
							menuTree.list[i].tab.push({url: "Main_TrafficMonitor_last24.asp", tabName: "__INHERIT__"});
							menuTree.list[i].tab.push({url: "Main_TrafficMonitor_daily.asp", tabName: "__INHERIT__"});
							menuTree.list[i].tab.push({url: "NULL", tabName: "__INHERIT__"});
						}
					}
				}

				if(!isSupport("traffic_limiter")){
					retArray.push("AdaptiveQoS_TrafficLimiter.asp");		
				}

				if(downsize_4m_support){
					retArray.push("Main_ConnStatus_Content.asp");
					retArray.push("Main_TrafficMonitor_realtime.asp");
				}
				
				if(!pptpd_support){
					retArray.push("Advanced_VPN_PPTP.asp");
				}

				if(!openvpnd_support){
					retArray.push("Advanced_VPN_OpenVPN.asp");
					retArray.push("Advanced_OpenVPNClient_Content.asp");
					retArray.push("Advanced_VPNDirector.asp");
				}	

				if(!ipsec_srv_support){
					retArray.push("Advanced_VPN_IPSec.asp");
				}

				if(!vpnc_support){
					retArray.push("Advanced_VPNClient_Content.asp");
				}

				if(!isSupport("Instant_Guard"))
					retArray.push("Advanced_Instant_Guard.asp");

				if(!ParentalCtrl2_support){
					retArray.push("ParentalControl.asp");
				}

				if(!yadns_support){
					retArray.push("YandexDNS.asp");
				}

				if(!isSupport("adguard_dns")){
					retArray.push("adGuard_DNS.asp");
				}

				if(!frs_feedback_support) {		
					retArray.push("Advanced_Feedback.asp");
					retArray.push("Feedback_Info.asp");
				}

				if(noftp_support){
					retArray.push("Advanced_AiDisk_ftp.asp");
				}
				
				if(!dualWAN_support){
					retArray.push("Advanced_WANPort_Content.asp");
				}
				else{
					if(!dualwan_enabled && usb_index == 0){
						retArray.push("Advanced_WAN_Content.asp");
						if(!gobi_support)
							retArray.push("Advanced_MobileBroadband_Content.asp");
						else
							retArray.push("Advanced_Modem_Content.asp");

						if(!gobi_support){
							replaceTabNameByUrl("menu_WAN", "Advanced_Modem_Content.asp", "<#menu5_3_1#>");
						}
						else{
							replaceTabNameByUrl("menu_WAN", "Advanced_MobileBroadband_Content.asp", "<#menu5_3_1#>");
						}
					}

					if(usb_index < 0){
						retArray.push("Advanced_MobileBroadband_Content.asp");
						retArray.push("Advanced_Modem_Content.asp");
					}
					else{
						if(!gobi_support)
							retArray.push("Advanced_MobileBroadband_Content.asp");
						else
							retArray.push("Advanced_Modem_Content.asp");
					}
				}

				if(!SwitchCtrl_support){
					retArray.push("Advanced_SwitchCtrl_Content.asp");
				}

				if(!tr069_support){
					retArray.push("Advanced_TR069_Content.asp");
				}

				if(!snmp_support){
					retArray.push("Advanced_SNMP_Content.asp");
				}

				if(!nt_center_support){
					retArray.push("Advanced_Notification_Content.asp");
				}

				if(!smart_connect_support && !smart_connect_v2_support){
					retArray.push("Advanced_Smart_Connect.asp");
				}
				
				if(!adBlock_support){
					retArray.push("AiProtection_AdBlock.asp");
				}
				if(!keyGuard_support){
					retArray.push("AiProtection_Key_Guard.asp");
				}

				if(!tor_support){
					retArray.push("Advanced_TOR_Content.asp");
				}

				if(!dsl_support) {
					retArray.push("Advanced_DSL_Content.asp");
					retArray.push("Advanced_ADSL_Content.asp");
					retArray.push("Main_AdslStatus_Content.asp");
					retArray.push("Main_Spectrum_Content.asp");
				}
				else{
					retArray.push("Advanced_WAN_Content.asp");
					retArray.push("Advanced_OperationMode_Content.asp");			
					if(!spectrum_support)
						retArray.push("Main_Spectrum_Content.asp");
				}

				if(hwmodeSwitch_support){
					retArray.push("Advanced_OperationMode_Content.asp");		
				}

				if(noiptv_support){
					retArray.push("Advanced_IPTV_Content.asp");
				}


				if(!media_support || nomedia_support){
					retArray.push("mediaserver.asp");
				}

				if(!rog_support){
					retArray.push("AdaptiveQoS_ROG.asp");
				}

				if(!alexa_support){
					retArray.push("Advanced_Smart_Home_Alexa.asp");
				}

				if(!ifttt_support){
					retArray.push("Advanced_Smart_Home_IFTTT.asp");
				}

				if(!IPv6_support){
					retArray.push("Main_IPV6Status_Content.asp");
				}

				if(!fbwifi_support){
					retArray.push("Guest_network_fbwifi.asp");
				}

				if(!mtlancfg_support || isSupport("SMART_HOME_MASTER_UI")){
					retArray.push("Advanced_VLAN_Switch_Content.asp");
					retArray.push("Advanced_VLAN_Profile_Content.asp");
				}

				if(!tagged_based_vlan){
					retArray.push("Advanced_TagBasedVLAN_Content.asp");
					retArray.push("Advanced_MultiSubnet_Content.asp");
				}
				else
					retArray.push("Advanced_DHCP_Content.asp");

				if(!wifiproxy_support || !concurrep_support || !isSwMode("re")){
					retArray.push("Advanced_WProxy_Content.asp");
				}
				
				if(!captivePortal_support) {
					retArray.push("Captive_Portal.asp");
					retArray.push("Captive_Portal_Advanced.asp");
				}
				else {
					if(!cp_freewifi_support)
						retArray.push("Captive_Portal.asp");
					if(!cp_advanced_support)
						retArray.push("Captive_Portal_Advanced.asp");
				}

				if(!cooler_support){
					retArray.push("Advanced_PerformanceTuning_Content.asp");
				}

				if(!rrsut_support)
					retArray.push("cloud_router_sync.asp");

				if(!amesh_support)
					retArray.push("Advanced_Roaming_Block_Content.asp");
				else{
					if(ameshRouter_support){
						if(!isSwMode("rt") && !isSwMode("ap"))
							retArray.push("Advanced_Roaming_Block_Content.asp");
					}
					else if(ameshNode_support)
						retArray.push("Advanced_Roaming_Block_Content.asp");
				}

				if(!fileflex_support)
					retArray.push("fileflex.asp");

				if(!dnsfilter_support)
					retArray.push("DNSDirector.asp");

				if(isSupport("mtlancfg")){
					retArray.push("Captive_Portal.asp");
					retArray.push("Captive_Portal_Advanced.asp");
					retArray.push("Guest_network_fbwifi.asp");
				}

				if(!isSupport("mtlancfg") || !isSupport("mlo")){
					retArray.push("MLO.asp");
				}

				if(isSupport("BUSINESS")){
					retArray.push("APP_Installation.asp");
					retArray.push("aidisk.asp");
					retArray.push("PrinterServer.asp");
					retArray.push("Advanced_Modem_Content.asp");
					retArray.push("Advanced_TimeMachine.asp");
					retArray.push("fileflex.asp");
				}

				if(isSupport("wifi7")){
					retArray.push("cloud_sync.asp");
					retArray.push("cloud_router_sync.asp");
				}

				/* Operation Mode */
				if(isSwMode("re")){
					retArray.push("GameBoost_ROG.asp");
					retArray.push("TrafficAnalyzer_Statistic.asp");
					retArray.push("Advanced_DHCP_Content.asp");
					retArray.push("Advanced_MultiSubnet_Content.asp");
					retArray.push("Advanced_GWStaticRoute_Content.asp");
					retArray.push("Advanced_IPTV_Content.asp");
					retArray.push("Advanced_VLAN_Switch_Content.asp");
					retArray.push("Advanced_VLAN_Profile_Content.asp");
					retArray.push("Main_DHCPStatus_Content.asp");
					retArray.push("Main_IPV6Status_Content.asp");
					retArray.push("Main_RouteStatus_Content.asp");
					retArray.push("Main_IPTStatus_Content.asp");
					retArray.push("Main_ConnStatus_Content.asp");
					retArray.push("Advanced_Smart_Connect.asp");
					retArray.push("DNSDirector.asp");

					if(userRSSI_support){
						retArray.push("Advanced_ACL_Content.asp");
						if(!concurrep_support){
							retArray.push("Advanced_Wireless_Content.asp");
						}
						retArray.push("Advanced_WWPS_Content.asp");
						retArray.push("Advanced_WMode_Content.asp");
						retArray.push("Advanced_WSecurity_Content.asp");
					}
				}
				else if(isSwMode("ap")){
					retArray.push("GameBoost_ROG.asp");
					retArray.push("TrafficAnalyzer_Statistic.asp");	
					if(!dhcp_override_support){
						retArray.push("Advanced_DHCP_Content.asp");
					}
					retArray.push("Advanced_MultiSubnet_Content.asp");
					retArray.push("Advanced_GWStaticRoute_Content.asp");
					retArray.push("Advanced_IPTV_Content.asp");
					retArray.push("Main_DHCPStatus_Content.asp");
					retArray.push("Main_IPV6Status_Content.asp");
					retArray.push("Main_RouteStatus_Content.asp");
					retArray.push("Main_IPTStatus_Content.asp");
					retArray.push("Main_ConnStatus_Content.asp");
					//short term solution for only router mode support Captive Portal
					retArray.push("Captive_Portal.asp");
					retArray.push("Captive_Portal_Advanced.asp");
					//short term solution for only router mode support Facebook Wi-Fi
					retArray.push("Guest_network_fbwifi.asp");
					retArray.push("DNSDirector.asp");
				}
				else if(isSwMode("mb")){
					retArray.push("GameBoost_ROG.asp");
					retArray.push("TrafficAnalyzer_Statistic.asp");
					retArray.push("Advanced_DHCP_Content.asp");
					retArray.push("Advanced_MultiSubnet_Content.asp");
					retArray.push("Advanced_GWStaticRoute_Content.asp");
					retArray.push("Advanced_IPTV_Content.asp");
					retArray.push("Advanced_VLAN_Switch_Content.asp");
					retArray.push("Advanced_VLAN_Profile_Content.asp");
					retArray.push("Main_DHCPStatus_Content.asp");
					retArray.push("Main_IPV6Status_Content.asp");
					retArray.push("Main_RouteStatus_Content.asp");
					retArray.push("Main_IPTStatus_Content.asp");
					retArray.push("Main_ConnStatus_Content.asp");
					retArray.push("Advanced_Smart_Connect.asp");
					retArray.push("DNSDirector.asp");
				}

				/* System Status Changed */
				// --

				/* MODELDEP */
				if(based_modelid == "RT-N10U"){
					retArray.push("Advanced_WMode_Content.asp");
				}
				else if(based_modelid == "RT-AC87U" && '<% nvram_get("wl_unit"); %>' == '1'){
					retArray.push("Advanced_WSecurity_Content.asp");
				}
				else if(based_modelid == "RT-N300"){
					retArray.push("Advanced_WMode_Content.asp");
					retArray.push("Advanced_IPTV_Content.asp");
				}
				if(based_modelid === '4G-AC55U' || based_modelid === '4G-AC68U'){
					retArray.push("Advanced_Modem_Content.asp");
				}
				else{
					retArray.push("Advanced_MobileBroadband_Content.asp");
				}

				if(!internetSpeed_support && !internetSpeed_lite_support){
					retArray.push("AdaptiveQoS_InternetSpeed.asp");
				}

				if(outfox_support || !wtfast_support)
					retArray.push("Advanced_WTFast_Content.asp");

				return retArray;
			}
		}
	}

	function removeTabByUrl(idx, url){
		for(var i in menuTree.list){
			if(menuTree.list[i].index == idx){
				for(var j in menuTree.list[i].tab){
					if(menuTree.list[i].tab[j].url == url){
						menuTree.list[i].tab.splice(j, 1);
					}
				}
			}
		}
	}

	function replaceTabNameByUrl(idx, url, tabName){
		for(var i in menuTree.list){
			if(menuTree.list[i].index == idx){
				for(var j in menuTree.list[i].tab){
					if(menuTree.list[i].tab[j].url == url){
						menuTree.list[i].tab[j].tabName = tabName;
					}
				}
			}
		}
	}

	if(usb_index !== -1){
		menuTree.list.filter(function(item, index, array){
			if(item.index == "menu_APP"){
				item.tab.filter(function(item2, index2, array2){
					if(item2.url == "Advanced_Modem_Content.asp")
						item.tab.splice(index2, 1);
				});
			}
		});
	}

	return menuTree;
});
