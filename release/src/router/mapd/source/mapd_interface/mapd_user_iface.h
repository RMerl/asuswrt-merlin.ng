#ifndef _MAPD_USER_IFACE_H_
#define _MAPD_USER_IFACE_H_

#include "common.h"
#include "interface.h"
#include <sys/un.h>
#ifdef SUPPORT_MULTI_AP
struct mapd_user_event
{
	int event_id;
	char event_body[0];
};
 /* struct mapd_interface_ctrl - Internal structure for control interface library
 *
 * This structure is used by the daemon control interface
 * library to store internal data. Programs using the library should not touch
 * this data directly. They can only use the pointer to the data structure as
 * an identifier for the control interface connection and use this as one of
 * the arguments for most of the control interface library functions.
 */


struct mapd_interface_user
{
	int s;
	struct sockaddr_un local;
	struct sockaddr_un dest;
};
#endif
#define WIRELESS_CLIENT_JOIN_NOTIF				1
#define WIRELESS_CLIENT_LEAVE_NOTIF				2
#ifdef SUPPORT_MULTI_AP
#define ETHERNET_CLIENT_JOIN_NOTIF				3
#define ETHERNET_CLIENT_LEAVE_NOTIF				4
#define CH_PREF_NOTIF							5
#define HIGHER_LAYER_PAYLOAD					6

#define ONBOARDING_STATUS_NOTIF				7

#ifdef VENDOR1_FEATURE_EXTEND
#define TRIGGER_MAP_STEERING_NOTIF				8
#define GET_BEACON_REPORT_NOTIF					9
#define GET_BTM_RESPONSE_NOTIF					10
#endif /*VENDOR1_FEATURE_EXTEND*/
struct mapd_user_iface_wireless_client_event
{
	unsigned char sta_mac[ETH_ALEN];
	unsigned char bssid[ETH_ALEN];
	char ssid[MAX_SSID_LEN + 1];
};

struct mapd_user_iface_eth_client_event
{
	unsigned char sta_mac[ETH_ALEN];
	unsigned char almac[ETH_ALEN];
};

struct mapd_user_iface_ch_pref_event
{
	unsigned char radio_id[ETH_ALEN];
	unsigned char almac[ETH_ALEN];
	unsigned char op_class;
	unsigned char ch_num;
	unsigned char ch_list[MAX_CH_NUM];
	unsigned char perference;
	unsigned char reason;
};

struct mapd_user_higher_layer_data_event
{
	unsigned int buf_len;
	unsigned char buf[4096];
};

#ifdef VENDOR1_FEATURE_EXTEND
struct mapd_user_iface_trigger_map_steering_event
{
	unsigned char band;
	unsigned char sta_mac[ETH_ALEN];
	char ssid[MAX_SSID_LEN + 1];
	unsigned char cap_11v;
	char ul_rssi;
#ifdef VENDOR1_FEATURE_EXTEND
	unsigned char vendor1_bssid[ETH_ALEN];	//own
	unsigned char vendor1_target_mac[ETH_ALEN];
	unsigned char vendor1_almac[ETH_ALEN];
	char vendor1_rssi_own;				//for vendor1 log - current ap downlink rssi from beacon response
	char vendor1_rssi_candidate;		//for vendor1 log - target ap downlink rssi from beacon response
#endif //VENDOR1_FEATURE_EXTEND
};

struct mapd_user_iface_get_beacon_report_event
{
	unsigned char band;
	unsigned char sta_mac[ETH_ALEN];
	char ssid[MAX_SSID_LEN + 1];
	unsigned char bssid[ETH_ALEN];
	char dl_rssi;
	unsigned char target_bssid[ETH_ALEN];
	char target_dl_rssi;
};

struct mapd_user_iface_get_btm_response_event
{
	unsigned char band;
	unsigned char sta_mac[ETH_ALEN];
	char ssid[MAX_SSID_LEN + 1];
	unsigned char status_code;
};
#endif /*VENDOR1_FEATURE_EXTEND*/
void mapd_user_eth_client_join(
	struct mapd_global *global,
	unsigned char *sta_mac, unsigned char *bssid);
#endif
void mapd_user_wireless_client_leave(
	struct mapd_global *global,
	unsigned char *sta_mac, unsigned char *bssid, char *ssid);
void mapd_user_wireless_client_join(
	struct mapd_global *global,
	unsigned char *sta_mac, unsigned char *bssid, char *ssid);
#ifdef SUPPORT_MULTI_AP
void send_user_ch_operable_results(struct own_1905_device *ctx);
#endif
#endif
