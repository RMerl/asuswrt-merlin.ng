#ifndef __CFG_EVENT_H__
#define __CFG_EVENT_H__

#define WEVENT_PREFIX	"wevent"
#define HTTPD_PREFIX	"httpd"
#define RC_PREFIX		"rc"
#define EVENT_ID	"eid"
#define MAC_ADDR	"mac_addr"
#define IF_NAME		"if_name"
#define SLAVE_MAC	"slave_mac"
#define LOGIN_IP		"login_ip"
#define OB_STATUS	"ob_status"
#define RE_MAC		"re_mac"
#define NEW_RE_MAC	"new_re_mac"
#define VSIE		"vsie"
#define ASUS_OUI	"F832E4"
#define WEVENT_MAC_IFNAME_MSG	 "{\""WEVENT_PREFIX"\":{\""EVENT_ID"\":\"%d\",\""MAC_ADDR"\":\"%s\",\""IF_NAME"\":\"%s\"}}"
#define WEVENT_VSIE_MSG	 "{\""WEVENT_PREFIX"\":{\""EVENT_ID"\":\"%d\",\""VSIE"\":\"%s\"}}"
#define HTTPD_GENERIC_MSG	 "{\""HTTPD_PREFIX"\":{\""EVENT_ID"\":\"%d\"}}"
#define HTTPD_SLAVE_MSG	"{\""HTTPD_PREFIX"\":{\""EVENT_ID"\":\"%d\",\""SLAVE_MAC"\":\"%s\"}}"
#define HTTPD_IP_MSG	"{\""HTTPD_PREFIX"\":{\""EVENT_ID"\":\"%d\",\""LOGIN_IP"\":\"%s\"}}"
#define HTTPD_ONBOARDING_MSG	"{\""HTTPD_PREFIX"\":{\""EVENT_ID"\":\"%d\",\""OB_STATUS"\":%d,\""RE_MAC"\":\"%s\",\""NEW_RE_MAC"\":\"%s\"}}"
#define RC_GENERIC_MSG	 	"{\""RC_PREFIX"\":{\""EVENT_ID"\":\"%d\"}}"

enum httpdEventType {
	EID_HTTPD_FW_CHECK = 1,
	EID_HTTPD_FW_UPGRADE,
	EID_HTTPD_REMOVE_SLAVE,
	EID_HTTPD_RESET_DEFAULT,
	EID_HTTPD_ONBOARDING,
	EID_HTTPD_START_WPS
};

enum wEventType {
	EID_WEVENT_DEVICE_CONNECTED = 1,
	EID_WEVENT_DEVICE_DISCONNECTED,
	EID_WEVENT_DEVICE_PROBE_REQ
};

enum rcEventType {
	EID_RC_WPS_STOP = 1
};

#endif /* __CFG_EVENT_H__ */
/* End of cfg_event.h */
