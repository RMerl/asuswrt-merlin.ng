#ifndef __CFG_EVENT_H__
#define __CFG_EVENT_H__

#define WEVENT_PREFIX	"wevent"
#define HTTPD_PREFIX	"httpd"
#define RC_PREFIX		"rc"
#define ETHEVENT_PREFIX	"ethevent"
#define EVENT_ID	"eid"
#define MAC_ADDR	"mac_addr"
#define IF_NAME		"if_name"
#define SLAVE_MAC	"slave_mac"
#define LOGIN_IP		"login_ip"
#define OB_STATUS	"ob_status"
#define OB_KEY	"ob_key"
#define RE_MAC		"re_mac"
#define NEW_RE_MAC	"new_re_mac"
#define VSIE		"vsie"
#define ASUS_OUI	"F832E4"
#define CONFIG	"config"
#define E_MODEL_NAME	"model_name"
#define E_ETHER_LIST	"ether_list"
#define E_OB_PATH	"ob_path"
#define MAC_LIST	"mac_list"
#define DATA		"data"
#define STA		"sta"
#define BLOCK_TIME	"block_time"
#define TARGET_AP	"target_ap"
#define WEVENT_GENERIC_MSG	 "{\""WEVENT_PREFIX"\":{\""EVENT_ID"\":\"%d\"}}"
#define WEVENT_MAC_IFNAME_MSG	 "{\""WEVENT_PREFIX"\":{\""EVENT_ID"\":\"%d\",\""MAC_ADDR"\":\"%s\",\""IF_NAME"\":\"%s\"}}"
#define WEVENT_VSIE_MSG	 "{\""WEVENT_PREFIX"\":{\""EVENT_ID"\":\"%d\",\""VSIE"\":\"%s\"}}"
#define HTTPD_GENERIC_MSG	 "{\""HTTPD_PREFIX"\":{\""EVENT_ID"\":\"%d\"}}"
#define HTTPD_SLAVE_MSG	"{\""HTTPD_PREFIX"\":{\""EVENT_ID"\":\"%d\",\""SLAVE_MAC"\":\"%s\"}}"
#define HTTPD_IP_MSG	"{\""HTTPD_PREFIX"\":{\""EVENT_ID"\":\"%d\",\""LOGIN_IP"\":\"%s\"}}"
#define HTTPD_OB_AVAILABLE_MSG	"{\""HTTPD_PREFIX"\":{\""EVENT_ID"\":\"%d\",\""OB_STATUS"\":%d}}"
#define HTTPD_OB_LOCK_MSG	"{\""HTTPD_PREFIX"\":{\""EVENT_ID"\":\"%d\",\""OB_STATUS"\":%d,\""RE_MAC"\":\"%s\",\""NEW_RE_MAC"\":\"%s\"}}"
#define HTTPD_OB_SELECTION_MSG	"{\""HTTPD_PREFIX"\":{\""EVENT_ID"\":\"%d\",\""OB_STATUS"\":%d,\""NEW_RE_MAC"\":\"%s\",\""E_OB_PATH"\":%d}}"
#define HTTPD_CONFIG_CHANGED_MSG	"{\""HTTPD_PREFIX"\":{\""EVENT_ID"\":\"%d\",\""RE_MAC"\":\"%s\",\""CONFIG"\":%s}}"
#define HTTPD_REBOOT_MSG	"{\""HTTPD_PREFIX"\":{\""EVENT_ID"\":\"%d\",\""MAC_LIST"\":%s}}"
#define HTTPD_ACTION_MSG	"{\""HTTPD_PREFIX"\":{\""EVENT_ID"\":\"%d\",\""MAC_LIST"\":%s,\""DATA"\":%s}}"
#define RC_GENERIC_MSG	 	"{\""RC_PREFIX"\":{\""EVENT_ID"\":\"%d\"}}"
#define RC_CONFIG_CHANGED_MSG	"{\""RC_PREFIX"\":{\""EVENT_ID"\":\"%d\",\""CONFIG"\":%s}}"
#define ETHEVENT_PROBE_MSG	 "{\""ETHEVENT_PREFIX"\":{\""EVENT_ID"\":\"%d\",\""E_ETHER_LIST"\":%s}}"
#define ETHEVENT_STATUS_MSG	 "{\""ETHEVENT_PREFIX"\":{\""EVENT_ID"\":\"%d\",\""OB_STATUS"\":%d,\""OB_KEY"\":\"%s\"}}"

/* source */
#define FROM_NONE	0x0
#define FROM_WIRELESS	0x1
#define FROM_ETHERNET	0x2

enum httpdEventType {
	EID_HTTPD_NONE = 0,
	EID_HTTPD_FW_CHECK = 1,
	EID_HTTPD_FW_UPGRADE,
	EID_HTTPD_REMOVE_SLAVE,
	EID_HTTPD_RESET_DEFAULT,
	EID_HTTPD_ONBOARDING,
	EID_HTTPD_CONFIG_CHANGED,
	EID_HTTPD_START_WPS,
#ifdef RTCONFIG_BHCOST_OPT
	EID_HTTPD_SELF_OPTIMIZE,
#endif
	EID_HTTPD_REBOOT,
	EID_HTTPD_RE_RECONNECT,
	EID_HTTPD_FORCE_ROAMING,
	EID_HTTPD_MAX
};

enum wEventType {
	EID_WEVENT_DEVICE_CONNECTED = 1,
	EID_WEVENT_DEVICE_DISCONNECTED,
	EID_WEVENT_DEVICE_PROBE_REQ,
	EID_WEVENT_DEVICE_RADAR_DETECTED
};

enum ethEventType {
	EID_ETHEVENT_DEVICE_PROBE_REQ = 1,
	EID_ETHEVENT_ONBOARDING_STATUS
};

enum rcEventType {
	EID_RC_WPS_STOP = 1,
	EID_RC_REPORT_PATH,
	EID_RC_GET_TOPOLOGY,
	EID_RC_FEEDBACK,
	EID_RC_RESTART_WIRELESS,
	EID_RC_CONFIG_CHANGED
};

#endif /* __CFG_EVENT_H__ */
/* End of cfg_event.h */
