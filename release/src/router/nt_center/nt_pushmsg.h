/* For iOS Push Notification Title */

struct pnsInfo
{
	int      event;
	int      argNum;
	char    *title;
	char    *ifttthook;
};
	
struct pnsInfo mapPushInfo[] =
{
	/* RESERVATION EVENT */
	{RESERVATION_MAIL_REPORT_EVENT               ,-1 ,"" ,""},
	{RESERVATION_MAIL_CONFIRM_EVENT              ,-1 ,"" ,""},
	/* ------------------------------
	   ### System ###
	---------------------------------*/
	/* WAN EVENT */
	{SYS_WAN_DISCONN_EVENT                       ,0  ,"Unable to connect to the Internet"                       ,"" },
	{SYS_WAN_BLOCK_EVENT                         ,0  ,"WAN Connection is paused"                                ,"" },
	{SYS_WAN_CABLE_UNPLUGGED_EVENT               ,0  ,"The network cable is unplugged"                          ,"" },
	{SYS_WAN_PPPOE_AUTH_FAILURE_EVENT            ,0  ,"PPPoE Authentication Failure"                            ,"" },
	{SYS_WAN_USB_MODEM_UNREADY_EVENT             ,0  ,"USB modem is not ready."                                 ,"" },
	{SYS_WAN_IP_CONFLICT_EVENT                   ,0  ,"IP conflict detected"                                    ,"" },
	{SYS_WAN_UNABLE_CONNECT_PARENT_AP_EVENT      ,0  ,"Unable to connect to Parent AP"                          ,"" },
	{SYS_WAN_MODEM_OFFLINE_EVENT                 ,0  ,"The modem appears to be offline."                        ,"" },
	{SYS_WAN_GOT_PROBLEMS_FROM_ISP_EVENT         ,0  ,"Your ISP appears to have problems."                      ,"" },
	{SYS_WAN_UNPUBLIC_IP_EVENT                   ,0  ,"External IP service is unavailable."                     ,"" },
	/* PASSWORD EVENT */
	{SYS_PASSWORD_SAME_WITH_LOGIN_WIFI_EVENT     ,0  ,"WiFi Password is the Same"                               ,"" },
	{SYS_PASSWORD_WIFI_WEAK_EVENT                ,0  ,"WiFi Password is Weak"                                   ,"" },
	{SYS_PASSWORD_LOGIN_STRENGTH_CHECK_EVENT     ,0  ,"Admin Password is Weak"                                  ,"" },
	/* GUEST NETWORK EVENT */
	{SYS_GUESTWIFI_ONE_ENABLE_EVENT              ,0  ,"Guest Network is available now!"                         ,"" },
	{SYS_GUESTWIFI_MORE_ENABLE_EVENT             ,0  ,"Guest Network is available now!"                         ,"" },
	/* RSSI EVENT */
	{SYS_RSSI_LOW_SIGNAL_EVENT                   ,0  ,"Get better signal!"                                      ,"" },
	{SYS_RSSI_LOW_SIGNAL_AGAIN_EVENT             ,0  ,"Get better signal!"                                      ,"" },
	/* DUALWAN EVENT */
	{SYS_DUALWAN_FAILOVER_EVENT                  ,0  ,"Failover Completed"                                      ,"" },
	{SYS_DUALWAN_FAILBACK_EVENT                  ,0  ,"Failback Completed"                                      ,"" },
	/* SYS DETECT EVENT */
	{SYS_SCAN_DLNA_PLAYER_EVENT                  ,0  ,"AiPlayer device is found"                                ,"" },
	{SYS_DETECT_ASUS_SSID_UNENCRYPT_EVENT        ,0  ,"ASUS router is found and need configuration"             ,"" },
	{SYS_ECO_MODE_EVENT                          ,0  ,"ECO Mode is on"                                          ,"" },
	{SYS_GAME_MODE_EVENT                         ,0  ,"Game mode is on"                                         ,"" },
	{SYS_NEW_DEVICE_WIFI_CONNECTED_EVENT         ,0  ,"New Device Connected"                                    ,"webhook_device_connect" },
	{SYS_WIFI_DEVICE_DISCONNECTED_EVENT          ,0  ,"Device Disconnected."                                    ,"" },
	{SYS_EXISTED_DEVICE_WIFI_CONNECTED_EVENT     ,0  ,"Manage Your Wireless Client"                             ,"webhook_device_connect" },
	/* FIRMWARE EVENT */
	{SYS_FW_NWE_VERSION_AVAILABLE_EVENT          ,0  ,"New Firmware Available"                                  ,"" },
	{SYS_NEW_SIGNATURE_UPDATED_EVENT             ,0  ,"Signature Updated"                                       ,"" },
	/* ------------------------------
	   ### Administration ###
	---------------------------------*/
	/* LOGIN EVENT */
	{ADMIN_LOGIN_FAIL_LAN_WEB_EVENT              ,0  ,"Unusual Router Sign-In"                                  ,"" },
	{ADMIN_LOGIN_FAIL_WAN_WEB_EVENT              ,0  ,"Unusual Router Sign-In"                                  ,"" },
	{ADMIN_LOGIN_FAIL_SSH_EVENT                  ,0  ,"Unusual Router Sign-In"                                  ,"" },
	{ADMIN_LOGIN_FAIL_TELNET_EVENT               ,0  ,"Unusual Router Sign-In"                                  ,"" },
	{ADMIN_LOGIN_FAIL_SSID_EVENT                 ,0  ,"Unusual Wireless Sign-In"                                ,"" },
	{ADMIN_LOGIN_DEVICE_DOUBLE_EVENT             ,0  ,"Multiple admin login"                                    ,"" },
	{ADMIN_LOGIN_ACCOUNT_DOBLE_EVENT             ,0  ,"Multiple admin login"                                    ,"" },
	{ADMIN_LOGIN_FAIL_AICLOUD_EVENT              ,0  ,"Unusual AiCloud Sign-In"                                 ,"" },
	{ADMIN_LOGIN_FAIL_VPNSERVER_EVENT            ,0  ,"Unusual VPN Server Sign-In"                              ,"" },
	/* ------------------------------
	   ### Security ###
	---------------------------------*/
	/* PROTECTION EVENT */
	{PROTECTION_INTO_MONITORMODE_EVENT           ,0  ,"Intrusion Alert"                                         ,"" },
	{PROTECTION_VULNERABILITY_EVENT              ,0  ,"Intrusion Prevention System Alert"                       ,"" },
	{PROTECTION_CC_EVENT                         ,0  ,"Infected Device Detected and Blocked"                    ,"" },
	{PROTECTION_DOS_EVENT                        ,0  ,"DoS Protection Alert"                                    ,"" },
	{PROTECTION_SAMBA_GUEST_ENABLE_EVENT         ,0  ,"Securtiy Risk - Samba"                                   ,"" },
	{PROTECTION_FTP_GUEST_ENABLE_EVENT           ,0  ,"Securtiy Risk - FTP "                                    ,"" },
	{PROTECTION_FIREWALL_DISABLE_EVENT           ,0  ,"Securtiy Risk  - Firewall Disable"                       ,"" },
	{PROTECTION_MALICIOUS_SITE_EVENT             ,0  ,"Malicious Site Access Blocked"                           ,"" },
	{PROTECTION_WEB_CROSS_SITE_EVENT             ,0  ,"Security Event Notice - Web Cross-site Scripting!"       ,"" },
	{PROTECTION_IIS_VULNERABILITY_EVENT          ,0  ,"Security Event Notice - Microsoft IIS Vulnerability!"    ,"" },
	{PROTECTION_DNS_AMPLIFICATION_ATTACK_EVENT   ,0  ,"Security Event Notice - DNS Amplification Attack!"       ,"" },
	{PROTECTION_SUSPICIOUS_HTML_TAG_EVNET        ,0  ,"Security Event Notice - Suspicious HTML Iframe tag!"     ,"" },
	{PROTECTION_BITCOIN_MINING_ACTIVITY_EVENT    ,0  ,"Security Event Notice - Bitcoin Mining Activity!"        ,"" },
	{PROTECTION_MALWARE_RANSOM_THREAT_EVENT      ,0  ,"Security Event Notice - Malware Ransomware Threat!"      ,"" },
	{PROTECTION_MALWARE_MIRAI_THREAT_EVENT       ,0  ,"Security Event Notice - Malware Mirai Threat!"           ,"" },
	/* ------------------------------
	   ### Parental Contorl ###
	---------------------------------*/
	/* PERMISSION REQUEST EVENT */
	{PERMISSION_FROM_BLOCKPAGE_EVENT             ,0  ,"Permission Request"                                      ,"" },
	{PERMISSION_FROM_TIME_SCHEDULE_EVENT         ,0  ,"Permission Request"                                      ,"" },
	/* ------------------------------
	   ### Traffic Management ###
	---------------------------------*/
	/* TRAFFIC METER EVENT */
	{TRAFFICMETER_ALERT_EVENT                    ,0  ,"Traffic Limiter Alert"                                   ,"" },
	{TRAFFICMETER_BW_LIMITER_EVENT               ,0  ,""                                                        ,"" },
	{TRAFFIC_REDUCE_LAG_EVENT                    ,0  ,"Reduce LAG in your Network!"                             ,"" },
	/* ------------------------------
	   ### USB Function ###
	---------------------------------*/
	/* USB EVENT */
	{USB_DM_TASK_FINISHED_EVENT                  ,0  ,"Download Master Notification"                            ,"" },
	{USB_DISK_SCAN_FAIL_EVENT                    ,0  ,"USB Disk Failed"                                         ,"" },
	{USB_DISK_EJECTED_FAIL_EVENT                 ,0  ,"External Device Not Ejected Properly"                    ,"" },
	{USB_DISK_PARTITION_FULL_EVENT               ,0  ,"USB Partition Full <1%>"                                 ,"" },
	{USB_DISK_FULL_EVENT                         ,0  ,"USB Disk Full <1%>"                                      ,"" },
	/* ------------------------------
	   ### Hint Item ###
	---------------------------------*/
	/* HINT EVENT */
	{HINT_USB_CHECK_EVENT                        ,0  ,"Get a Private Cloud!"                                    ,"" },
	{HINT_USB_FIRSTIME_CHECK_EVENT               ,0  ,""                                                        ,"" },
	{HINT_SAMBA_INLAN_EVENT                      ,0  ,""                                                        ,"" },
	{HINT_OSX_INLAN_EVENT                        ,0  ,"Time Machine"                                            ,"" },
	{HINT_DUALBAND_WITHOUT_5G_EVENT              ,0  ,""                                                        ,"" },
	{HINT_USB3_WITHOUT_ENHANCE_EVENT             ,0  ,"Enhance USB Transmission Speed!"                         ,"" },
	{HINT_XBOX_PS_EVENT                          ,0  ,"Adaptive QoS"                                            ,"" },
	{HINT_UPNP_RENDERER_EVENT                    ,0  ,""                                                        ,"" },
	{HINT_DETECT_GAME_SERVICE_EVENT              ,0  ,"Game Boost"                                              ,"" },
	{HINT_SUGGEST_ENABLE_DFS_CHANNEL_EVENT       ,0  ,"DFS Channel"                                             ,"" },
	{HINT_SUGGEST_IE_BROWSER_VERSION_EVENT       ,0  ,"Please Update Internet Explorer."                        ,"" },
	{HINT_INTERNET_USAGE_INSIGHT_EVENT           ,0  ,"Internet Usage Insight"                                  ,"" },
	/* The End */
	{0,0,0,0}
};
