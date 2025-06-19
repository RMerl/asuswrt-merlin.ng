
#ifndef __nt_eInfo_h__
#define __nt_eInfo_h__

#define APP_ACTION_AIHOME      0x01
#define APP_ACTION_AICLOUD     0x02
#define APP_ACTION_AIEXTEND    0x04
#define APP_ACTION_AIPLAYER    0x08

#define APP_SID_AIHOME         "1001"
#define APP_SID_AICLOUD        "1001.01"
#define APP_SID_AIEXTEND       "1001.02"
#define APP_SID_AIPLAYER       "1001.03"
#define MAX_APPS_NUM           4

#define ACT(var) APP_ACTION_##var
#define SID(var) APP_SID_##var

/* NOTIFY CENTER EVENT MAPPING INFO STRUCTURE
---------------------------------*/
struct eInfo {
	char    *eName;
	int      value;
	int      eType;
	int      appsid;
	int      action;
	int      ispush;
	EVENT_PRI_T ePri;
};
enum {
	TYPE_OF_RSV=0,
	TYPE_OF_TURN_OFF,
	TYPE_OF_IMPORTANT,
	TYPE_OF_TIPS,
	TYPE_OF_TOTAL
};


struct app {
	char    *sid;
	int      appx;
};

struct app appInfo[] =
{
	{SID(AIHOME)    ,ACT(AIHOME)   },
	{SID(AICLOUD)   ,ACT(AICLOUD)  },
	{SID(AIEXTEND)  ,ACT(AIEXTEND) },
	{SID(AIPLAYER)  ,ACT(AIPLAYER) },
	{0,0}
};

struct eInfo mapInfo[] =
{
	/* RESERVATION EVENT */
	/* ------------------------------
	   ### System ###
	---------------------------------*/
	{"SYS_WAN_DISCONN_EVENT"                     ,SYS_WAN_DISCONN_EVENT                       ,TYPE_OF_TIPS       ,ACT(AIHOME)    ,0, 0, PRI_LOW },
	{"SYS_WAN_BLOCK_EVENT"                       ,SYS_WAN_BLOCK_EVENT                         ,TYPE_OF_TIPS       ,ACT(AIHOME)    ,0, 0, PRI_LOW },
	{"SYS_NEW_DEVICE_WIFI_CONNECTED_EVENT"       ,SYS_NEW_DEVICE_WIFI_CONNECTED_EVENT         ,TYPE_OF_IMPORTANT  ,ACT(AIHOME)    ,ACT_NOTIFY_DB, 1, PRI_LOW },
	{"SYS_NEW_DEVICE_ETH_CONNECTED_EVENT"        ,SYS_NEW_DEVICE_ETH_CONNECTED_EVENT          ,TYPE_OF_IMPORTANT  ,ACT(AIHOME)    ,ACT_NOTIFY_DB, 1, PRI_LOW },
	{"SYS_EXISTED_DEVICE_WIFI_CONNECTED_EVENT"   ,SYS_EXISTED_DEVICE_WIFI_CONNECTED_EVENT     ,TYPE_OF_IMPORTANT  ,ACT(AIHOME)    ,0, 0, PRI_LOW },
	{"SYS_WAN_CABLE_UNPLUGGED_EVENT"             ,SYS_WAN_CABLE_UNPLUGGED_EVENT               ,TYPE_OF_TIPS       ,ACT(AIHOME)    ,ACT_NOTIFY_DB, 0, PRI_LOW },
	{"SYS_WAN_PPPOE_AUTH_FAILURE_EVENT"          ,SYS_WAN_PPPOE_AUTH_FAILURE_EVENT            ,TYPE_OF_TIPS       ,ACT(AIHOME)    ,ACT_NOTIFY_DB, 0, PRI_LOW },
	{"SYS_WAN_USB_MODEM_UNREADY_EVENT"           ,SYS_WAN_USB_MODEM_UNREADY_EVENT             ,TYPE_OF_TIPS       ,ACT(AIHOME)    ,ACT_NOTIFY_DB, 0, PRI_LOW },
	{"SYS_WAN_IP_CONFLICT_EVENT"                 ,SYS_WAN_IP_CONFLICT_EVENT                   ,TYPE_OF_TIPS       ,ACT(AIHOME)    ,ACT_NOTIFY_DB, 0, PRI_LOW },
	{"SYS_WAN_UNABLE_CONNECT_PARENT_AP_EVENT"    ,SYS_WAN_UNABLE_CONNECT_PARENT_AP_EVENT      ,TYPE_OF_TIPS       ,ACT(AIHOME)    ,ACT_NOTIFY_DB, 0, PRI_LOW },
	{"SYS_WAN_MODEM_OFFLINE_EVENT"               ,SYS_WAN_MODEM_OFFLINE_EVENT                 ,TYPE_OF_TIPS       ,ACT(AIHOME)    ,ACT_NOTIFY_DB, 0, PRI_LOW },
	{"SYS_WAN_GOT_PROBLEMS_FROM_ISP_EVENT"       ,SYS_WAN_GOT_PROBLEMS_FROM_ISP_EVENT         ,TYPE_OF_TIPS       ,ACT(AIHOME)    ,ACT_NOTIFY_DB, 0, PRI_LOW },
	{"SYS_WAN_UNPUBLIC_IP_EVENT"                 ,SYS_WAN_UNPUBLIC_IP_EVENT                   ,TYPE_OF_TIPS       ,ACT(AIHOME)    ,ACT_NOTIFY_DB, 0, PRI_LOW },
	{"SYS_ALL_WIFI_TURN_OFF_EVENT"               ,SYS_ALL_WIFI_TURN_OFF_EVENT                 ,TYPE_OF_IMPORTANT  ,ACT(AIHOME)    ,ACT_NOTIFY_DB, 1, PRI_LOW },
	{"SYS_SCY_UNCONN_NOTICE_EVENT"               ,SYS_SCY_UNCONN_NOTICE_EVENT                 ,TYPE_OF_IMPORTANT  ,ACT(AIHOME)    ,ACT_NOTIFY_DB, 0, PRI_LOW },
	{"SYS_SCY_CONN_NOTICE_EVENT"                 ,SYS_SCY_CONN_NOTICE_EVENT                   ,TYPE_OF_IMPORTANT  ,ACT(AIHOME)    ,ACT_NOTIFY_DB, 0, PRI_LOW },
	{"SYS_FW_UPGRADE_OK_EVENT"                   ,SYS_FW_UPGRADE_OK_EVENT                     ,TYPE_OF_TIPS       ,ACT(AIHOME)    ,ACT_NOTIFY_APPUSH, 0, PRI_LOW },
	{"SYS_FW_UPGRADE_NOK_EVENT"                  ,SYS_FW_UPGRADE_NOK_EVENT                    ,TYPE_OF_TIPS       ,ACT(AIHOME)    ,ACT_NOTIFY_APPUSH, 0, PRI_LOW },
	{"SYS_REBOOT_OK_EVENT"                       ,SYS_REBOOT_OK_EVENT                         ,TYPE_OF_TIPS       ,ACT(AIHOME)    ,ACT_NOTIFY_APPUSH, 0, PRI_LOW },
	{"SYS_REBOOT_NOK_EVENT"                      ,SYS_REBOOT_NOK_EVENT                        ,TYPE_OF_TIPS       ,ACT(AIHOME)    ,ACT_NOTIFY_APPUSH, 0, PRI_LOW },
	{"SYS_LAN_PORT_IN_EVENT"                     ,SYS_LAN_PORT_IN_EVENT                       ,TYPE_OF_TIPS       ,ACT(AIHOME)    ,ACT_NOTIFY_DB | ACT_NOTIFY_APPUSH, 1, PRI_LOW },
	{"SYS_LAN_PORT_OUT_EVENT"                    ,SYS_LAN_PORT_OUT_EVENT                      ,TYPE_OF_TIPS       ,ACT(AIHOME)    ,ACT_NOTIFY_DB | ACT_NOTIFY_APPUSH, 1, PRI_LOW },
	{"SYS_AFC_COLD_REBOOT_SILENT_EVENT"          ,SYS_AFC_COLD_REBOOT_SILENT_EVENT            ,TYPE_OF_TIPS       ,ACT(AIHOME)    ,ACT_NOTIFY_DB | ACT_NOTIFY_APPUSH, 1, PRI_LOW },
	{"SYS_AFC_COLD_REBOOT_EVENT"                 ,SYS_AFC_COLD_REBOOT_EVENT                   ,TYPE_OF_TIPS       ,ACT(AIHOME)    ,ACT_NOTIFY_DB | ACT_NOTIFY_APPUSH, 1, PRI_LOW },
	{"SYS_FW_NEW_VERSION_AVAILABLE_EVENT"        ,SYS_FW_NEW_VERSION_AVAILABLE_EVENT          ,TYPE_OF_IMPORTANT  ,ACT(AIHOME)    ,ACT_NOTIFY_DB | ACT_NOTIFY_APPUSH, 1, PRI_LOW },
	/* ------------------------------
	   ### Administration ###
	---------------------------------*/
	{"ADMIN_REMOTE_LOGIN_EVENT"                  ,ADMIN_REMOTE_LOGIN_EVENT                    ,TYPE_OF_TIPS       ,ACT(AIHOME)    ,ACT_NOTIFY_DB | ACT_NOTIFY_APPUSH, 1, PRI_LOW },
	{"ADMIN_BOUND_DEV_EVENT"                     ,ADMIN_BOUND_DEV_EVENT                       ,TYPE_OF_TIPS       ,ACT(AIHOME)    ,ACT_NOTIFY_APPUSH, 0, PRI_LOW },
	{"ADMIN_UNBOUND_DEV_EVENT"                   ,ADMIN_UNBOUND_DEV_EVENT                     ,TYPE_OF_TIPS       ,ACT(AIHOME)    ,ACT_NOTIFY_APPUSH, 0, PRI_LOW },
	{"ADMIN_LOGIN_FAIL_SSH_EVENT"                ,ADMIN_LOGIN_FAIL_SSH_EVENT                  ,TYPE_OF_TIPS       ,ACT(AIHOME)    ,ACT_NOTIFY_DB | ACT_NOTIFY_APPUSH, 1, PRI_LOW },
	{"ADMIN_LOGIN_FAIL_TELNET_EVENT"             ,ADMIN_LOGIN_FAIL_TELNET_EVENT               ,TYPE_OF_TIPS       ,ACT(AIHOME)    ,ACT_NOTIFY_DB | ACT_NOTIFY_APPUSH, 1, PRI_LOW },
	{"ADMIN_LOGIN_FAIL_LAN_WEB_EVENT"            ,ADMIN_LOGIN_FAIL_LAN_WEB_EVENT              ,TYPE_OF_TIPS       ,ACT(AIHOME)    ,ACT_NOTIFY_DB | ACT_NOTIFY_APPUSH, 1, PRI_LOW },
	/* ------------------------------
	   ### Security ###
	---------------------------------*/
	{"PROTECTION_VULNERABILITY_EVENT"            ,PROTECTION_VULNERABILITY_EVENT              ,TYPE_OF_IMPORTANT  ,ACT(AIHOME)    ,ACT_NOTIFY_DB | ACT_NOTIFY_EMAIL, 0, PRI_LOW },
	{"PROTECTION_CC_EVENT"                       ,PROTECTION_CC_EVENT                         ,TYPE_OF_IMPORTANT  ,ACT(AIHOME)    ,ACT_NOTIFY_DB | ACT_NOTIFY_EMAIL, 0, PRI_LOW },
	{"PROTECTION_MALICIOUS_SITE_EVENT"           ,PROTECTION_MALICIOUS_SITE_EVENT             ,TYPE_OF_IMPORTANT  ,ACT(AIHOME)    ,ACT_NOTIFY_DB, 1, PRI_LOW },
	{"PROTECTION_INFECTED_DEVICE_EVENT"          ,PROTECTION_INFECTED_DEVICE_EVENT            ,TYPE_OF_IMPORTANT  ,ACT(AIHOME)    ,ACT_NOTIFY_DB, 1, PRI_LOW },
	/* ------------------------------
	   ### USB Function ###
	---------------------------------*/
	{"USB_TETHERING_EVENT"                       ,USB_TETHERING_EVENT                         ,TYPE_OF_IMPORTANT  ,ACT(AIHOME)    ,ACT_NOTIFY_DB | ACT_NOTIFY_APPUSH, 1, PRI_UHIGH },
	/* ------------------------------
	   ### GENERAL Event ###
	---------------------------------*/
	{"GENERAL_WIFI_DEV_ONLINE"                   ,GENERAL_WIFI_DEV_ONLINE                     ,TYPE_OF_TURN_OFF   ,-1             ,0, 0, PRI_LOW },
	{"GENERAL_WIFI_DEV_OFFLINE"                  ,GENERAL_WIFI_DEV_OFFLINE                    ,TYPE_OF_TURN_OFF   ,-1             ,0, 0, PRI_LOW },
	{"GENERAL_ETH_DEV_ONLINE"                    ,GENERAL_ETH_DEV_ONLINE                      ,TYPE_OF_TURN_OFF   ,-1             ,0, 0, PRI_LOW },
	{"GENERAL_ETH_DEV_OFFLINE"                   ,GENERAL_ETH_DEV_OFFLINE                     ,TYPE_OF_TURN_OFF   ,-1             ,0, 0, PRI_LOW },
	{"GENERAL_ETH_DEV_REFUSED"                   ,GENERAL_ETH_DEV_REFUSED                     ,TYPE_OF_TURN_OFF   ,-1             ,ACT_NOTIFY_GENERAL, 0, PRI_LOW },
	{"GENERAL_SYS_STATES"                        ,GENERAL_SYS_STATES                          ,TYPE_OF_TURN_OFF   ,-1             ,ACT_NOTIFY_GENERAL, 0, PRI_LOW },
	{"GENERAL_DEV_UPDATE"                        ,GENERAL_DEV_UPDATE                          ,TYPE_OF_TURN_OFF   ,-1             ,ACT_NOTIFY_GENERAL | ACT_NOTIFY_ALEXA, 0, PRI_LOW },
	{"GENERAL_DEV_DELETED"                       ,GENERAL_DEV_DELETED                         ,TYPE_OF_TURN_OFF   ,-1             ,ACT_NOTIFY_GENERAL | ACT_NOTIFY_ALEXA, 0, PRI_LOW },
	{"GENERAL_DEV_ACCESS_CHANGE"                 ,GENERAL_DEV_ACCESS_CHANGE                   ,TYPE_OF_TURN_OFF   ,-1             ,ACT_NOTIFY_GENERAL | ACT_NOTIFY_ALEXA, 0, PRI_LOW },
	{"GENERAL_QOS_UPDATE"                        ,GENERAL_QOS_UPDATE                          ,TYPE_OF_TURN_OFF   ,-1             ,ACT_NOTIFY_GENERAL | ACT_NOTIFY_ALEXA, 0, PRI_LOW },
	{"GENERAL_TOGGLE_STATES_UPDATE"              ,GENERAL_TOGGLE_STATES_UPDATE                ,TYPE_OF_TURN_OFF   ,-1             ,ACT_NOTIFY_GENERAL | ACT_NOTIFY_ALEXA, 0, PRI_LOW },
	/* ------------------------------
	   ### AiMesh Event ###
	---------------------------------*/
	{"AIMESH_ETH_OB_EVENT"                       ,AIMESH_ETH_OB_EVENT                         ,TYPE_OF_IMPORTANT  ,ACT(AIHOME)    ,ACT_NOTIFY_DB | ACT_NOTIFY_APPUSH, 1, PRI_LOW },
	/* The End */
	{0, 0, 0, 0, 0, 0, 0}
};

#endif
