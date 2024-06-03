/* this header file is uesed to define the message between wapp and 1905 deamon
 */
#ifndef __INTERFACE_H__
#define __INTERFACE_H__

#include <linux/if_ether.h>
#include <net/if.h>
#include "common.h"
#ifndef GNU_PACKED
#define GNU_PACKED  __attribute__ ((packed))
#endif /* GNU_PACKED */

#define MAX_SSID_LEN 32
#define MAX_CH_NUM 13
#define MAX_ELEMNT_NUM 4
#define TLV_BUFFER_OFFSET 4
#define MAX_HE_MCS_LEN 12
#define WSC_APCLI_CONFIG_MSG_USE_SAVED_PROFILE 998
#define WSC_APCLI_CONFIG_MSG_SAVE_PROFILE 999

/*WAPP EVENT*/
enum {
	WAPP_MAP_MIN_CMD=0x0,
	/*add new mapcmd below this line*/

	WAPP_MAP_BH_READY=0x01,
	WAPP_WPS_CONFIG_STATUS,
	WAPP_RADIO_BASIC_CAP,
	WAPP_CHANNLE_PREFERENCE,
	WAPP_RADIO_OPERATION_RESTRICTION,
	WAPP_DISCOVERY,
	WAPP_CLIENT_NOTIFICATION,
	WAPP_AP_CAPABILITY,
	WAPP_AP_HT_CAPABILITY,
	WAPP_AP_VHT_CAPABILITY,
	WAPP_AP_OP_BSS=0x0C,
	WAPP_ASSOC_CLI,
	WAPP_CHN_SEL_RSP,
	WAPP_TOPOQUERY,
	WAPP_OPERBSS_REPORT,
	WAPP_CLI_CAPABILITY_REPORT,
	WAPP_1905_CMDU_REQUEST,
	WAPP_CLI_STEER_BTM_REPORT,
	WAPP_STEERING_COMPLETED,
	WAPP_1905_READ_BSS_CONF_REQUEST,  /* for wts controller bss_info ready case */
	WAPP_1905_READ_BSS_CONF_AND_RENEW,
	WAPP_AP_METRICS_INFO,
	WAPP_GET_WSC_CONF,
	WAPP_1905_READ_1905_TLV_REQUEST, /* for wts send 1905 data ready case */
	WAPP_BACKHAUL_STEER_RSP,
	WAPP_ALL_ASSOC_STA_TRAFFIC_STATS,
	WAPP_ONE_ASSOC_STA_TRAFFIC_STATS,
	WAPP_ALL_ASSOC_STA_LINK_METRICS,
	WAPP_ONE_ASSOC_STA_LINK_METRICS,
	WAPP_TX_LINK_STATISTICS,
	WAPP_RX_LINK_STATISTICS,
	WAPP_UNASSOC_STA_LINK_METRICS,
	WAPP_OPERATING_CHANNEL_REPORT,
	WAPP_BEACON_METRICS_REPORT,
	WAPP_1905_REQ,
	WAPP_AP_LINK_METRIC_REQ,
	WAPP_OPERATING_CHANNEL_INFO,
	WAPP_AP_CAPABLILTY_QUERY=0x61,
	WAPP_CLI_CAPABLILTY_QUERY,
	WAPP_CH_SELECTION_REQUEST,
	WAPP_CLI_STEER_REQUEST,
	WAPP_CH_PREFER_QUERY,
	WAPP_POLICY_CONFIG_REQUEST,
	WAPP_CLI_ASSOC_CNTRL_REQUEST,
	WAPP_OP_CHN_RPT=0x8F,
	WAPP_STA_RSSI,
	WAPP_STA_STAT,
	WAPP_NAC_INFO,
	WAPP_STA_BSSLOAD,
	WAPP_TXPWR_CHANGE,
	WAPP_CSA_INFO,
	WAPP_APCLI_UPLINK_RSSI,
	WAPP_BSS_STAT_CHANGE,
	WAPP_BSS_LOAD_CROSSING,
	WAPP_APCLI_ASSOC_STAT_CHANGE,
	WAPP_STA_CNNCT_REJ_INFO,
	WAPP_UPDATE_PROBE_INFO,
	WAPP_WIRELESS_INF_INFO,
	WAPP_SCAN_RESULT,
	WAPP_SCAN_DONE,
	WAPP_MAP_VEND_IE_CHANGED,
	WAPP_ALL_ASSOC_TP_METRICS,
	WAPP_AIR_MONITOR_REPORT,
	WAPP_MAP_BH_CONFIG,
	WAPP_DEVICE_STATUS,
	WAPP_BRIDGE_IP,
	WAPP_SET_BH_TYPE,
	WAPP_OFF_CH_SCAN_REPORT,
	WAPP_NET_OPT_SCAN_REPORT,
#ifdef AUTOROLE_NEGO
	WAPP_MAP_NEGO_ROLE_RESP,
#endif // AUTOROLE_NEGO
	WAPP_AP_HE_CAPABILITY,
#ifdef MAP_R2
	WAPP_SCAN_CAPAB,
	WAPP_CHANNEL_SCAN_REPORT,
	WAPP_ASSOC_STATUS_NOTIFICATION,
	WAPP_TUNNELED_MESSAGE,
#ifdef DFS_CAC_R2
	WAPP_CAC_CAPAB,
#endif
#endif
	WAPP_CAC_COMPLETION_REPORT,
#ifdef MAP_R2
#ifdef DFS_CAC_R2
	WAPP_CAC_STATUS_REPORT,
#endif
	WAPP_METRIC_REP_INTERVAL_CAP,
	WAPP_DISASSOC_STATS_EVT,
	WAPP_ALL_ASSOC_STA_EXTENDED_LINK_METRICS,
	WAPP_ONE_ASSOC_STA_EXTENDED_LINK_METRICS,
	WAPP_RADIO_METRICS_INFO,
	WAPP_R2_AP_CAP,
#endif
	WAPP_CH_LIST_DFS_INFO,
#ifdef MAP_R2
	WAPP_MBO_STA_PREF_CH_LIST,
#endif
	WAPP_CAC_PERIOD_ENABLE,
	WAPP_SEND_DPP_MSG,
	WAPP_MAP_RESET	,
	WAPP_WTS_CONFIG,
	/*add new map event before this line*/
	WAPP_MAP_MAX_EVENT,
};

/*wapp user req command*/
enum {
	SET_REGITER_EVENT = 0x01,

	WAPP_USER_MIN_CMD = 0x80,
	/*add wapp user cmd below this line*/

	WAPP_USER_GET_CLI_CAPABILITY_REPORT = 0x81,
	WAPP_USER_SET_WIRELESS_SETTING,
	WAPP_USER_GET_RADIO_BASIC_CAP,
	WAPP_USER_GET_AP_CAPABILITY,
	WAPP_USER_GET_AP_HT_CAPABILITY,
	WAPP_USER_GET_AP_VHT_CAPABILITY,
	WAPP_USER_GET_SUPPORTED_SERVICE,
	WAPP_USER_GET_SEARCHED_SERVICE,
	WAPP_USER_GET_ASSOCIATED_CLIENT,
	WAPP_USER_GET_RA_OP_RESTRICTION,
	WAPP_USER_GET_CHANNEL_PREFERENCE,
	WAPP_USER_SET_CHANNEL_SETTING,
	WAPP_USER_GET_OPERATIONAL_BSS,
	WAPP_USER_SET_STEERING_SETTING,
	WAPP_USER_SET_ASSOC_CNTRL_SETTING,
	WAPP_USER_SET_LOCAL_STEER_DISALLOW_STA,
	WAPP_USER_SET_BTM_STEER_DISALLOW_STA,
	WAPP_USER_SET_RADIO_CONTROL_POLICY,
	WAPP_USER_GET_AP_METRICS_INFO,
	WAPP_USER_MAP_CONTROLLER_FOUND,
	WAPP_USER_SET_BACKHAUL_STEER,
	WAPP_USER_SET_METIRCS_POLICY,
	WAPP_USER_GET_ASSOC_STA_TRAFFIC_STATS,
	WAPP_USER_GET_ONE_ASSOC_STA_TRAFFIC_STATS,
	WAPP_USER_GET_ASSOC_STA_LINK_METRICS,
	WAPP_USER_GET_ONE_ASSOC_STA_LINK_METRICS,
	WAPP_USER_GET_TX_LINK_STATISTICS,
	WAPP_USER_GET_RX_LINK_STATISTICS,
	WAPP_USER_GET_UNASSOC_STA_LINK_METRICS,
	WAPP_USER_SET_RADIO_TEARED_DOWN,
	WAPP_USER_SET_BEACON_METRICS_QRY,
	WAPP_USER_SET_TOPOLOGY_INFO,
	WAPP_USER_SET_RADIO_RENEW,
	WAPP_USER_GET_OPERATING_CHANNEL_INFO,
	WAPP_USER_GET_AP_HE_CAPABILITY,
	WAPP_USER_FLUSH_ACL,
	WAPP_USER_GET_BSSLOAD,
	WAPP_USER_GET_RSSI_REQ,
	WAPP_USER_SET_WHPROBE_REQ,
	WAPP_USER_SET_NAC_REQ,
	WAPP_USER_SET_DEAUTH_STA,
	WAPP_USER_SET_VENDOR_IE,
	WAPP_USER_GET_APCLI_RSSI_REQ,
	WAPP_USER_GET_WIRELESS_INF_INFO,
	WAPP_USER_SET_ADDITIONAL_BH_ASSOC,
	WAPP_USER_ISSUE_SCAN_REQ,
	WAPP_USER_GET_SCAN_RESULT,
	WAPP_USER_SEND_NULL_FRAMES,
	WAPP_USER_GET_ALL_ASSOC_TP_METRICS,
	WAPP_USER_SET_AIR_MONITOR_REQUEST,
	WAPP_USER_SET_ENROLLEE_BH,
	WAPP_USER_SET_BSS_ROLE,
	WAPP_USER_TRIGGER_WPS,
	WAPP_USER_ISSUE_APCLI_DISCONNECT,
	WAPP_USER_SET_BH_WIRELESS_SETTING,
	WAPP_USER_SET_BH_PRIORITY,
	WAPP_USER_SET_BH_CONNECT_REQUEST,
	WAPP_USER_GET_BH_WIRELESS_SETTING,
	WAPP_USER_SET_GARP_REQUEST,
	WAPP_USER_SET_DHCP_CTL_REQUEST,
	WAPP_USER_GET_BRIDGE_IP_REQUEST,
	WAPP_USER_SET_BRIDGE_DEFAULT_IP_REQUEST,
	WAPP_USER_SET_TX_POWER_PERCENTAGE,
	WAPP_USER_SET_OFF_CH_SCAN_REQ,
	WAPP_USER_SET_NET_OPT_SCAN_REQ,
#ifdef AUTOROLE_NEGO
	WAPP_NEGOTIATE_ROLE,
#endif // AUTOROLE_NEGO
	WAPP_UPDATE_MAP_DEVICE_ROLE,
	WAPP_USER_SET_AVOID_SCAN_CAC,
#ifdef MAP_R2
	WAPP_USER_GET_SCAN_CAP, //MAP_R2
	WAPP_USER_SET_CHANNEL_SCAN_REQ, //MAP_R2
	WAPP_USER_SET_CHANNEL_SCAN_POLICY, //MAP_R2
#ifdef DFS_CAC_R2
	WAPP_USER_SET_CAC_REQ, //MAP_R2
	WAPP_USER_SET_CAC_TERMINATE_REQ, //MAP_R2
	WAPP_USER_GET_CAC_CAP, //MAP_R2
	WAPP_USER_GET_CAC_STATUS,
#endif
	WAPP_USER_GET_METRIC_REP_INTERVAL_CAP,
	WAPP_USER_GET_ASSOC_STA_EXTENDED_LINK_METRICS,
	WAPP_USER_GET_ONE_ASSOC_STA_EXTENDED_LINK_METRICS,
	WAPP_USER_GET_RADIO_METRICS_INFO, //MAP_R2
	WAPP_USER_SET_UNSUCCESSFUL_ASSOC_POLICY, //MAP_R2
	WAPP_USER_GET_R2_AP_CAP, //MAP_R2
	WAPP_USER_SET_TRAFFIC_SEPARATION_SETTING, //MAP_R2
	WAPP_USER_SET_TRANSPARENT_VLAN_SETTING, //MAP_R2
#endif
	WAPP_USER_GET_WTS_CONFIG,
#ifdef ACL_CTRL
	WAPP_USER_SET_ACL_CNTRL_SETTING,
#endif
	/*add wapp user cmd above this line*/
	WAPP_USER_MAX_CMD,
};

typedef enum {
	SHOW_1905_REQ,
	SET_1905_DBG_LV_REQ,
} WAPP_1905_REQ_ID;

typedef enum {
	SYNC,
	ASYNC,
} CMD_EVENT_TYPE;
#define STATUS_BHSTA_UNCONFIGURED		0
#define STATUS_BHSTA_BH_READY			1
#define STATUS_BHSTA_CONFIGURED			2
#define STATUS_BHSTA_CONFIG_PENDING		3
#define STATUS_BHSTA_WPS_TRIGGERED		4
#define STATUS_BHSTA_WPS_FAILED			5
#define STATUS_BHSTA_WPS_NORMAL_CONFIGURED		6

#define STATUS_FHBSS_UNCONFIGURED		0
#define STATUS_FHBSS_WPS_TRIGGERED		1
#define STATUS_FHBSS_CONFIGURED			2
#define STATUS_FHBSS_WPS_FAILED			3
#define STATUS_FHBSS_WPS_SUCCESSFULL	4

typedef struct GNU_PACKED _wapp_device_status {
	unsigned int status_fhbss;
	unsigned int status_bhsta;
} wapp_device_status;

typedef enum {
	WMODE_INVALID = 0,
	WMODE_A = 1 << 0,
	WMODE_B = 1 << 1,
	WMODE_G = 1 << 2,
	WMODE_GN = 1 << 3,
	WMODE_AN = 1 << 4,
	WMODE_AC = 1 << 5,
	WMODE_AX_24G = 1 << 6,
	WMODE_AX_5G = 1 << 7,
	WMODE_COMP = 8, /* total types of supported wireless mode, add this value once yow add new type */
} WIFI_MODE;

struct GNU_PACKED cmd_to_wapp
{
	unsigned short type;
	unsigned char role;             /*0-indicate this message is from agent, 1-indicate this message is from controller*/
	unsigned short length;
	unsigned char band;
	unsigned char bssAddr[6];
	unsigned char staAddr[6];
	unsigned char body[0];
};

struct GNU_PACKED msg
{
	unsigned short type;
	unsigned short length;
	unsigned char buffer[0];
};

struct _map_cmd_event
{
	unsigned short cmd;
	unsigned short event;
	CMD_EVENT_TYPE type;
	unsigned char (*match_func)(struct msg*, struct cmd_to_wapp* );
};


struct GNU_PACKED wapp_1905_req
{
	unsigned char  id;
	unsigned short value;
};

typedef struct GNU_PACKED _inf_info{
	unsigned char ifname[IFNAMSIZ];
	unsigned char mac_addr[ETH_ALEN];
} inf_info;
/**
  *@type: backhaul type, 0-eth; 1-wifi
  *@ifname: backhaul inf name (with link ready)
  *@mac_addr: mac addr of the backhaul interface
  */
struct GNU_PACKED bh_link_info
{
	unsigned char type;
	unsigned char ifname[IFNAMSIZ]; /*  */
	unsigned char mac_addr[ETH_ALEN];
	unsigned char bssid[ETH_ALEN];
	unsigned char trigger_autconf;
};
/**
  *@type: channel info,
  *@ifname: backhaul inf name (with link ready)
  *@mac_addr: mac addr of the backhaul interface
  */
struct GNU_PACKED channel_bw_info
{
	unsigned char iface_addr[ETH_ALEN];
	unsigned char channel_bw;
	unsigned char channel_num;
};

/**
  *@type: backhaul type, 0-eth; 1-wifi
  */
#define MAP_BH_ETH		0
#define MAP_BH_WIFI		1
struct GNU_PACKED bh_type_info
{
	unsigned char type;
};

/**
  *@identifier: radio unique identifier of the radio requesting config settings
  *@band: 0x00-2.4g 0x01-5g
  *@dev_type: 1-wifi ap; 2-wifi sta
  *@num_of_inf: number of interface under this radio for getting conf
  *@inf_info: ifname, e.g ra0/ra1/apcli0; mac_addr: mac addr of this interface
  */

struct GNU_PACKED wps_get_config
{
	unsigned char identifier[ETH_ALEN];
	unsigned char band;
	unsigned char dev_type;
	unsigned char num_of_inf;
	inf_info      inf_data[0];
};
/**
  *@identifier: radio unique identifier of the radio for which capabilities are reported
  *@op_class:
  *@channel:
  *@power:
  */
struct GNU_PACKED ch_config_info
{
	unsigned char identifier[ETH_ALEN];
	unsigned char ifname[IFNAMSIZ];
	unsigned char op_class;
	unsigned char channel;
	signed char power;
	unsigned char reason_code;
};

struct GNU_PACKED channel_setting
{
	unsigned char almac[ETH_ALEN];
	unsigned char ch_set_num;
	struct ch_config_info chinfo[0];
};

struct GNU_PACKED ch_rep_info {
	unsigned char identifier[ETH_ALEN];
	unsigned char op_class;
	unsigned char channel;
	signed char tx_power;
};

struct GNU_PACKED channel_report
{
	unsigned char ch_rep_num;
	struct ch_rep_info info[0];
};

struct GNU_PACKED tx_power_percentage_setting {
	unsigned char bandIdx;
	unsigned char tx_power_percentage;
};

/**
  * this structure belongs to struct ap_radio_basic_cap
  * @op_class: specifying the global operating class in which the subsequent channel list is valid
  * @max_tx_pwr: maximum transmit power EIRP that this radio is capable of transmitting in the
  * current regulatory domain for the operating class
  * @non_operch_num: number of statically non-operable channels in the operating class
  * @non_operch_list: statically non-operable channels in the operating class
  */
struct GNU_PACKED radio_basic_cap
{
	unsigned char op_class;
	unsigned char max_tx_pwr;
	unsigned char non_operch_num;
	unsigned char non_operch_list[MAX_CH_NUM];
};
/**
  * @identifier: radio unique identifier of the radio for which capabilities are reported
  * @max_bss_num: maximum number of bss supported by this radio
  * @op_class_num: operation class number
  * @opcap: a pointer to struct radio_basic_cap
  */
struct GNU_PACKED ap_radio_basic_cap
{
	unsigned char identifier[ETH_ALEN];
	unsigned char max_bss_num;
	unsigned char band;
	int wireless_mode;
	unsigned char op_class_num;
	struct radio_basic_cap opcap[0];
};

/**
  * @bssid: MAC address of local interface(equal to BSSID) operating on the radio
  */
struct GNU_PACKED op_bss_cap
{
	unsigned char bssid[ETH_ALEN];
	unsigned char ssid_len;
	unsigned char ssid[33];
	unsigned char map_vendor_extension;
	unsigned int auth_mode;
	unsigned int enc_type;
	unsigned char key_len;
	unsigned char key[64 + 1];
};
/**
  * @identifier: radio unique identifier of a radio
  * @identifier: the band of the radio
  * @oper_bss_num: number of bss(802.11 local interfaces) currently operating on the radio
  */
struct GNU_PACKED oper_bss_cap
{
	unsigned char identifier[ETH_ALEN];
	unsigned char oper_bss_num;
	unsigned char band;
	struct op_bss_cap cap[0];
};

#define BIT_BH_STA BIT (7)
#define BIT_BH_BSS BIT (6)
#define BIT_FH_BSS BIT (5)
#define BIT_TEAR_DOWN BIT (4)

#define OPER_BSS_CAP_LEN 8
/**
  * @ifname: the radio interface name to be config
  * @num: number of wireless setting, can be 1~16
  * @Ssid: the ssid of the bss
  * @AuthMode:
  * @EncrypType:
  * @WPAKey:
  * @map_vendor_extension: map vendor extentsion in M2, 1 byte
  */
struct GNU_PACKED wireless_setting {
	unsigned char mac_addr[ETH_ALEN];
	unsigned char	Ssid[32 + 1];
	unsigned short	AuthMode;
	unsigned short	EncrypType;
	unsigned char	WPAKey[64 + 1];
	unsigned char map_vendor_extension;    /*store MAP controller's Muiti-AP Vendor Extesion value in M2*/
	unsigned char hidden_ssid;
};

struct GNU_PACKED wsc_config{
	unsigned char num;
	struct wireless_setting setting[0];
};
struct GNU_PACKED bh_priority{
	unsigned char priority;
	unsigned char bh_mac[6];
};

/**
  * @sta_report_on_cop(0 or 1): cap of unassociated sta link metrics reporting on the channels
  * its BSSs arecurrently operating on
  * @sta_report_not_cop(0 or 1): cap of unassociated sta link metrics reporting on the channels
  * its BSSs are not currently operating on
  * @rssi_steer(0 or 1): cap of Agent-initiated RSSI-based Steering
  */
struct GNU_PACKED ap_capability
{
	unsigned char sta_report_on_cop;
	unsigned char sta_report_not_cop;
	unsigned char rssi_steer;
};

/**
  * @identifier: radio unique identifier of the radio for witch HT capabilities are reported
  * @tx_stream: maximum number of supported Tx spatial streams
  * @rx_stream: maximum number of supported Rx spatial streams
  * @sgi_20: short GI support for 20 MHz
  * @sgi_40: short GI support for 40 MHz
  * @ht_40: HT support for 40MHz
  */
struct GNU_PACKED ap_ht_capability
{
	unsigned char identifier[ETH_ALEN];
	unsigned char tx_stream;
	unsigned char rx_stream;
	unsigned char sgi_20;
	unsigned char sgi_40;
	unsigned char ht_40;
	unsigned char band;
};

/**
  * @identifier: radio unique identifier of the radio for witch HT capabilities are reported
  * @vht_tx_mcs: supported VHT Tx MCS; set to Tx VHT MCS Map field
  * @vht_rx_mcs: supported VHT Rx MCS; set to Rx VHT MCS Map field
  * @tx_stream: maximum number of supported Tx spatial streams
  * @rx_stream: maximum number of supported Rx spatial streams
  * @sgi_80: short GI support for 80 MHz
  * @sgi_160: short GI support for 160 MHz and 80+80MHz
  * @vht_8080: VHT support for 80+80MHz
  * @vht_160: VHT support for 160MHz
  * @su_beamformer: SU Beamformer capable
  * @mu_beamformer: MU Beamformer capable
  */
struct GNU_PACKED ap_vht_capability
{
	unsigned char identifier[ETH_ALEN];
	unsigned short vht_tx_mcs;
	unsigned short vht_rx_mcs;
	unsigned char tx_stream;
	unsigned char rx_stream;
	unsigned char sgi_80;
	unsigned char sgi_160;
	unsigned char vht_8080;
	unsigned char vht_160;
	unsigned char su_beamformer;
	unsigned char mu_beamformer;
	unsigned char band;
};

struct GNU_PACKED ap_he_capability
{
	unsigned char identifier[ETH_ALEN];
	unsigned char he_mcs_len;
	unsigned char he_mcs[MAX_HE_MCS_LEN];
	unsigned char tx_stream;
	unsigned char rx_stream;
	unsigned char he_8080;
	unsigned char he_160;
	unsigned char su_bf_cap;
	unsigned char mu_bf_cap;
	unsigned char ul_mu_mimo_cap;
	unsigned char ul_mu_mimo_ofdma_cap;
	unsigned char dl_mu_mimo_ofdma_cap;
	unsigned char ul_ofdma_cap;
	unsigned char dl_ofdma_cap;
};

/**
  * this structure belongs to struct ch_prefer
  * @op_class: specifying the global operating class in which the subsequent channel list is valid
  * @ch_num: valid channel num belongs to op_class
  * @ch_list: channel list belongs to op_class
  * @perference: indicate a preference value for the channels in the channel list
  * @reason: indicate reason for the preference
  */
struct GNU_PACKED prefer_info
{
	unsigned char op_class;
	unsigned char ch_num;
	unsigned char ch_list[MAX_CH_NUM];
	unsigned char perference;
	unsigned char reason;
};
/**
  * @identifier: radio unique identifier of the radio for witch channel preferences are reported
  * @op_class_num: operation class number
  * @opinfo: a pointer to struct prefer_info
  */
struct GNU_PACKED ch_prefer
{
	unsigned char identifier[ETH_ALEN];
	unsigned char op_class_num;
	struct prefer_info opinfo[0];
};
/**
  * @identifier: radio unique identifier of the radio for witch channel preferences are reported
  * @op_class_num: operation class number
  * @opinfo: a pointer to struct prefer_info
  */
struct GNU_PACKED radar_notify
{
	unsigned char identifier[ETH_ALEN];
	unsigned char channel_number;
	unsigned char radar_present;
};

/**
  * this structure belongs to struct restriction
  * @op_class: specifying the global operating class in which the subsequent channel list is valid
  * @ch_num: valid channel num belongs to op_class
  * @ch_list: channel list belongs to op_class
  * @fre_separation: the minimum frequency separation(in multiples of 10 MHz) that this radio
  * would require when operationg on the above channel number between the center frequency of
  * that channel and the center operating requency of another radio(operating simultaneous TX/RX)
  * of the agent
  */
struct GNU_PACKED restrict_info
{
	unsigned char op_class;
	unsigned char ch_num;
	unsigned char ch_list[MAX_CH_NUM];
	unsigned char fre_separation[MAX_CH_NUM];
};
/**
  * @identifier: radio unique identifier of the radio for witch radio operation restrictions are reported
  * @op_class_num: operation class number
  * @opinfo: a pointer to struct restrict_info
  */
struct GNU_PACKED restriction
{
	unsigned char identifier[ETH_ALEN];
	unsigned char op_class_num;
	struct restrict_info opinfo[0];
};

/**
  * @sta_mac: the MAC address of the client
  * @bssid: the BSSID of the BSS operated by the MAP agent for which the event has occurred
  * @assoc_evt: 1 for client has joined the BSS; 0 for client has left the BSS
  * @assoc_time: the time of the 802.11 client's last association to this Multi-AP device. unit jiffies.
  * @assoc_req: association request frame body
  */
struct GNU_PACKED map_client_association_event
{
	unsigned char sta_mac[ETH_ALEN];
	unsigned char bssid[ETH_ALEN];
	unsigned char assoc_evt;
	unsigned int assoc_time;
	unsigned short assoc_req_len;
	unsigned char assoc_req[0];
};
/*In case of AX nss can be different for different supported BWs.
So map requires NSS of all the supported BWs as in the HE cap info*/
struct GNU_PACKED map_he_nss{
	u16 nss_80:2;
	u16 nss_160:2;
	u16 nss_8080:2;
};

struct GNU_PACKED map_priv_cli_cap
{
    unsigned short bw:2;
    unsigned short phy_mode:3;
    unsigned short nss:2;
    unsigned short btm_capable:1;
    unsigned short rrm_capable:1;
    unsigned short mbo_capable:1;
	struct map_he_nss nss_he;
};

struct GNU_PACKED client_association_event
{
    struct map_priv_cli_cap cli_caps;
    struct map_client_association_event map_assoc_evt;
};
struct GNU_PACKED map_client_association_event_local
{
	unsigned char sta_mac[ETH_ALEN];
	unsigned char bssid[ETH_ALEN];
	unsigned char assoc_evt;
	unsigned int assoc_time;
	unsigned char is_APCLI;
	unsigned short assoc_req_len;
	unsigned char assoc_req[0];
};

struct GNU_PACKED client_association_event_local
{
    struct map_priv_cli_cap cli_caps;
    struct map_client_association_event_local map_assoc_evt;
};

#ifdef MAP_R2
struct GNU_PACKED client_disassociation_stats_event
{
  	u8 mac_addr[ETH_ALEN];
	u16 reason_code;
};
#endif
/**
  * @bssid: the BSSID of a BSS
  * @sta_mac: the MAC address of the client
  */
struct GNU_PACKED client_info
{
	unsigned char bssid[ETH_ALEN];
	unsigned char sta_mac[ETH_ALEN];
};
/**
  * @result: result code for the client capability report message
  * @length: the length of frame body
  * @body: the frame body of the most recently received (re)association request frame
  * from this client. if result code is not equal to 0, this field is omitted
  */
struct GNU_PACKED client_capa_rep
{
	unsigned char result;
	unsigned short length;
	unsigned char body[0];
};


/**
  * @dest_al_mac: dest al mac
  * @type: 1905 frame type, e.g High layer Data Message 0x8018
  * @len: the length frame body, for High layer Data Message, it must not be 0,
  	       for other type of 1905 cmdu(e.g topology discovery, there's no frame content, it may be 0)
  * @body: frame body
  */
struct GNU_PACKED _1905_cmdu_request
{
	unsigned char dest_al_mac[ETH_ALEN];
	unsigned short type;
	unsigned short len;
	unsigned char body[0];
};

/**
  * @target_bssid: indicates a target BSSID for steering. wildcard BSSID is represented by ff:ff:ff:ff:ff:ff
  * @op_class: target BSS operating class
  * @channel: target BSS channel number for channel on which the target BSS is transmitting beacon frames
  * @sta_mac: sta mac address for which the steering request applies. if sta_count of struct steer_request
  * is 0, then this field is invalid
  */
struct GNU_PACKED target_bssid_info {
	unsigned char target_bssid[ETH_ALEN];
	unsigned char op_class;
	unsigned char channel;
	unsigned char sta_mac[ETH_ALEN];
#ifdef MAP_R2
	unsigned char reason;
#endif
};

/**
  * @assoc_bssid: unique identifier of the source BSS for whirch the steering request applies
  * (i.e. BSS that the STAs specified in the request are currently associated with)
  * @request_mode: 0: request is a steering opportunity; 1: request is a steering mandate to
  * trigger steering for specific client STA(s)
  * @btm_disassoc_immi: BTM Disassociation imminent bit
  * @btm_abridged: BTM Abridged bit
  * @steer_window: steering opportunity window. time period in seconds(from reception of the
  * steering request message)for whitch the request is valid.
  * @btm_disassoc_timer: BTM Disassociation timer. time period in TUs for disassociation timer
  * in BTM request
  * @sta_count: sta list count. k=0: steering request applies to all associated STAs in the BSS per
  * policy setting; k>0: steering request applies to specific STAs specific by STA MAC address(es)
  * @target_bssid_count: target BSSID list count. only valid when request_mode is set to 1.
  * m=1: the same target BSSID is indicated for all specified STAs;m=k: an individual target BSSID
  * is indicated for each apecified STA(in same order)
  * @steer_info: contains k sta mac address and m target bssid info
  */
struct GNU_PACKED steer_request {
	unsigned char assoc_bssid[ETH_ALEN];
	unsigned char request_mode;
	unsigned char btm_disassoc_immi;
	unsigned char btm_abridged;
	unsigned short steer_window;
	unsigned short btm_disassoc_timer;
	unsigned char sta_count;
	unsigned char target_bssid_count;
#ifdef MAP_R2
	unsigned char steering_type; //0-Legacy, 1-R2
#endif
	struct target_bssid_info info[0];
};

/**
  * @bssid: unique identifier of the source BSS for which the steering BTM report applies
  * @sta_mac: sta mac address for which the steering BTM report applies
  * @status: indicates the value of the BTM status code as reported by the STA in the BTM response
  * @tbssid: indicates the value of the target BSSID field(if present)in the BTM response received
  * from the STA. Note: this indicates the BSSID that the STA intends to roam to, whicg may not
  * align with the target BSSID specified in the BTM request.
  */
struct GNU_PACKED cli_steer_btm_event {
	unsigned char bssid[ETH_ALEN];
	unsigned char sta_mac[ETH_ALEN];
	unsigned char status;
	unsigned char tbssid[ETH_ALEN];
};

/**
  * @bssid: unique identifier of the BSS for which the client blocking request applies
  * @assoc_control: indicates if the request is to block or unblock the indicated STAs from associating
  * @valid_period: time period in seconds(from reception of the client association control request message)
  * for which a blocking request is valid
  * @sta_list_count: indicateing one or more STAs  for which the client association control request applies
  * @sta_mac: sta mac address for which he client association control request applies
  */
struct GNU_PACKED cli_assoc_control {
	unsigned char bssid[ETH_ALEN];
	unsigned char assoc_control;
	unsigned short valid_period;
	unsigned char sta_list_count;
	unsigned char sta_mac[0];
};

struct GNU_PACKED local_disallow_sta_head {
	unsigned char sta_cnt;
	unsigned char sta_list[0];
};
struct GNU_PACKED btm_disallow_sta_head {
	unsigned char sta_cnt;
	unsigned char sta_list[0];
};
struct GNU_PACKED radio_policy_head {
	unsigned char identifier[ETH_ALEN];
	unsigned char policy;
	unsigned char ch_ultil_thres;
	unsigned char rssi_thres;
};
struct GNU_PACKED radio_policy {
	unsigned char radio_cnt;
	struct radio_policy_head radio[0];
};

struct GNU_PACKED metric_policy_head {
	unsigned char identifier[ETH_ALEN];
	unsigned char rssi_thres;
	unsigned char hysteresis_margin;
	unsigned char ch_util_thres;
	unsigned char sta_stats_inclusion;
	unsigned char sta_metrics_inclusion;
};
struct GNU_PACKED metric_policy {
	unsigned char report_interval;
	unsigned char policy_cnt;
	struct metric_policy_head policy[0];
};


/**
  * @backhaul_mac: the mac address of associated backhaul station operated by the multi-ap agent
  * @target_bssid: the bssid of the target BSS
  * @oper_class: operating class
  * @channel: channel number on which beacon frames are being transmitted by the target BSS
  */
struct GNU_PACKED backhaul_steer_request {
	unsigned char backhaul_mac[ETH_ALEN];
	unsigned char target_bssid[ETH_ALEN];
	unsigned char oper_class;
	unsigned char channel;
};


struct GNU_PACKED backhaul_steer_request_extended {
	unsigned char target_ssid[MAX_SSID_LEN];
	unsigned char ssid_len;
	struct backhaul_steer_request request;
};
struct GNU_PACKED backhaul_connect_request {
	unsigned char backhaul_mac[ETH_ALEN];
	unsigned char target_bssid[ETH_ALEN];
	unsigned char target_ssid[33];
	unsigned short AuthType;
	unsigned short EncrType;
	unsigned char Key[64];
	unsigned char oper_class;
	unsigned char channel;
};

/**
  * @backhaul_mac: the mac address of associated backhaul station operated by the multi-ap agent
  * @target_bssid: the bssid of the target BSS
  * @status: status code
  */
struct GNU_PACKED backhaul_steer_rsp {
	unsigned char backhaul_mac[ETH_ALEN];
	unsigned char target_bssid[ETH_ALEN];
	unsigned char status;
	unsigned char error;
};

/*link metric collection*/
struct GNU_PACKED esp_info {
	unsigned char ac;
	unsigned char format;
	unsigned char ba_win_size;
	unsigned char e_air_time_fraction;
	unsigned char ppdu_dur_target;
};

/**
  * @bssid: bssid of a bss operated by the multi-ap agent for which the metrics are reported
  * @ch_util: channel utilization as mesured by the radio operating the bss
  * @assoc_sta_cnt: indicates the total number of STAs currently associated with the BSS
  * @esp: estimated service parameters information field.
  */
#ifdef MAP_R2
struct extended_ap_metrics {
	u32 uc_tx;
	u32 uc_rx;
	u32 mc_tx;
	u32 mc_rx;
	u32 bc_tx;
	u32 bc_rx;
};
#endif

struct GNU_PACKED ap_metrics_info {
	unsigned char bssid[ETH_ALEN];
	unsigned char ch_util;
	unsigned short assoc_sta_cnt;
	unsigned char valid_esp_count;
#ifdef MAP_R2
	struct extended_ap_metrics ext_ap_metric;
#endif
	struct esp_info esp[0];
};

#ifdef MAP_R2
struct GNU_PACKED radio_metrics_info {
	u8 ra_id[ETH_ALEN];
	u8 cu_noise;
	u8 cu_tx;
	u8 cu_rx;
	u8 cu_other;
	u32 edcca;
};
#endif

/**
  * @mac: MAC address of the associated STA
  * @bytes_sent: raw counter of the number of bytes sent to the associated STA
  * @bytes_received: raw counter of number of bytes received from the associated STA
  * @packets_sent: raw counter of the number of packets successfully sent to the associated STA
  * @packets_received: raw counter of the number of packets received from the associated STA
  * during the measurement window
  * @tx_packets_errors: raw counter of the number of packets which could not be transmitted to
  * the associated STA due to errors
  * @rx_packets_errors: raw counter of the number of packets which were received in error from
  * the associated STA
  * @retransmission_count: raw counter of the number of packets sent with the retry flag set to
  * the associated STA
  */
struct GNU_PACKED stat_info {
	unsigned char mac[ETH_ALEN];
	unsigned int bytes_sent;
	unsigned int bytes_received;
	unsigned int packets_sent;
	unsigned int packets_received;
	unsigned int tx_packets_errors;
	unsigned int rx_packets_errors;
	unsigned int retransmission_count;
	unsigned char is_APCLI; //Prakhar
};

/**
  * @identifier: radio unique identifier of the radio for which sta are associated
  * @sta_cnt: the total number of sta which traffic stats are reported
  * @stats: sta traffic stats info
  */
struct GNU_PACKED sta_traffic_stats {
	unsigned char identifier[ETH_ALEN];
	unsigned char sta_cnt;
	struct stat_info stats[0];
};

#ifdef CENT_STR
struct GNU_PACKED sta_traffic_tlv {
	unsigned char identifier[ETH_ALEN];
	unsigned char sta_cnt;
	struct stat_info stats[0];
};
#endif
/**
  * @mac: MAC address of the associated STA
  * @bssid: BSSID of the BSS for which the STA is associated
  * @time_delta: The time delta in ms between the time at which the earliest measurement that
  * contributed to the data rate estimates were made, and the time at which this report was sent
  * @erate_downlink: Estimated MAC Data Rate in downlink (in Mb/s)
  * @erate_uplink: Estimated MAC Data Rate in uplink (in Mb/s)
  * @rssi_uplink: Measured uplink RSSI for STA (dBm)
  */
#ifdef MAP_R2
struct GNU_PACKED ext_link_metrics {
	unsigned char mac[ETH_ALEN];
	unsigned char bssid[ETH_ALEN];
	u32 last_data_ul_rate;
	u32 last_data_dl_rate;
	u32 utilization_rx;
	u32 utilization_tx;
};

struct GNU_PACKED ext_sta_link_metrics {
	unsigned char identifier[ETH_ALEN];
	unsigned char sta_cnt;
	struct ext_link_metrics info[0];
};
#endif

struct GNU_PACKED link_metrics {
	unsigned char mac[ETH_ALEN];
	unsigned char bssid[ETH_ALEN];
	unsigned int time_delta;
	unsigned int erate_downlink;
	unsigned int erate_uplink;
	unsigned char rssi_uplink;
	unsigned char is_APCLI; //Prkahar
};

/**
  * @identifier: radio unique identifier of the radio for which sta are associated
  * @sta_cnt: the total number of sta which link metrics are reported
  * @info: sta link metrics info
  */
struct GNU_PACKED sta_link_metrics {
	unsigned char identifier[ETH_ALEN];
	unsigned char sta_cnt;
	struct link_metrics info[0];
};

/**
  * a struct to query tx or rx link metrics info of 1905
  * @media_type: wifi or eth interface
  * @local_if: MAC address of an interface in the receiving IEEE 1905.1 AL, which connects to
  * an interface in the neighbor IEEE 1905.1 AL
  * @neighbor_if: MAC address of an IEEE 1905.1 interface in a neighbor IEEE 1905.1 device
  * which connects to an IEEE 1905.1 interface in the receiving IEEE 1905.1 device
  */
struct GNU_PACKED link_stat_query {
	unsigned char media_type;
	unsigned char local_if[ETH_ALEN];
	unsigned char neighbor_if[ETH_ALEN];
};

/**
  * @pkt_errs: wifi or eth interface
  * @local_if: Estimated number of lost packets on the transmit side of the link during the
  * measurement period
  * @tx_pkts: Estimated number of packets transmitted by the Transmitter of the link on
  * the same measurement period
  * @mac_tp_cap: The maximum MAC throughput of the Link estimated at the transmitter
  * and expressed in Mb/s
  * @link_avail: The estimated average percentage of time that the link is available for data
  * transmission
  * @phyrate: If the media type of the link is IEEE 802.3, then IEEE 1901 or MoCA 1.1
  * This value is the PHY rate estimated at the transmitter of the link expressed in Mb/s;
  * otherwise, it is set to 0xFFFF.
  */
struct GNU_PACKED tx_link_stat_rsp {
	unsigned int pkt_errs;
	unsigned int tx_pkts;
	unsigned short mac_tp_cap;
	unsigned short link_avail;
	unsigned short phyrate;
	unsigned int tx_tp;
};

/**
  * @pkt_errs: wifi or eth interface
  * @local_if: Estimated number of lost packets on the receive side of the link during the
  * measurement period
  * @rx_pkts: Estimated number of packets transmitted by the Transmitter of the link on
  * the same measurement period
  * @rssi: If the media type of the link is IEEE 802.11, then this value is the estimated RSSI
  * in dB at the receive side of the Link expressed in dB; otherwise, it is set to 0xFF.
  */
struct GNU_PACKED rx_link_stat_rsp {
	unsigned int pkt_errs;
	unsigned int rx_pkts;
	unsigned char rssi;
	unsigned int rx_tp;
};

struct GNU_PACKED unlink_metrics_query {
	unsigned char oper_class;
	unsigned char ch_num;
	unsigned char ch_list[MAX_CH_NUM];
	unsigned char sta_num;
	unsigned char sta_list[0];
};

struct GNU_PACKED unlink_rsp_sta {
	unsigned char mac[ETH_ALEN];
	unsigned char ch;
	unsigned int time_delta;
	signed char uplink_rssi;
};

struct GNU_PACKED unlink_metrics_rsp {
	unsigned char oper_class;
	unsigned char sta_num;
	struct unlink_rsp_sta info[0];
};

struct GNU_PACKED ap_chn_rpt {
	unsigned char ch_rpt_len;
	unsigned char oper_class;
	unsigned char ch_list[MAX_CH_NUM];
};

struct GNU_PACKED beacon_metrics_query {
	unsigned char sta_mac[ETH_ALEN];
	unsigned char oper_class;
	unsigned char ch;
	unsigned char bssid[ETH_ALEN];
	unsigned char rpt_detail_val;
	unsigned char ssid_len;
	unsigned char ssid[33];
	unsigned char elemnt_num;
	unsigned char elemnt_list[MAX_ELEMNT_NUM];
	unsigned char ap_ch_rpt_num;
	struct ap_chn_rpt rpt[0];
};


struct GNU_PACKED beacon_metrics_rsp {
	unsigned char sta_mac[ETH_ALEN];
	unsigned char reserved;
	unsigned char bcn_rpt_num;
	unsigned short rpt_len;
	unsigned char rpt[0];
};

typedef struct GNU_PACKED _rrm_beacon_rep_info {
	unsigned char regulatory_Class;
	unsigned char ch_number;
	unsigned long long actual_measure_start_time;
	unsigned short measure_duration;
	unsigned char rep_frame_Info;
	unsigned char rcpi;
	unsigned char rsni;
	unsigned char bssid[ETH_ALEN];
	unsigned char annta_dd;
	unsigned int parent_tsf;
	unsigned char option[0];
} rrm_beacon_rep_info, *prrm_beacon_rep_info;


struct GNU_PACKED topo_info {
	unsigned char almac[ETH_ALEN];
	unsigned char bssid_num;
	unsigned char bssid[0];
};

#define PREQ_IE_LEN 128
typedef struct GNU_PACKED _wapp_probe_info {
	uint8_t mac_addr[ETH_ALEN];
	uint8_t channel;
	uint8_t rssi;
	uint8_t preq_len;
	uint8_t preq[PREQ_IE_LEN];
} wapp_probe_info;

typedef enum {
	WAPP_AUTH = 0,
	WAPP_ASSOC,
	WAPP_EAPOL
} WAPP_CNNCT_STAGE;

typedef struct GNU_PACKED _WAPP_CONNECT_FAILURE_REASON {
	uint8_t  connect_stage;
	uint16_t reason;
} WAPP_CONNECT_FAILURE_REASON;

typedef struct GNU_PACKED _wapp_sta_auth_rejected_info {
	uint32_t interface_index;
	unsigned char sta_mac[ETH_ALEN];
	unsigned char bssid[ETH_ALEN];
	WAPP_CONNECT_FAILURE_REASON cnnct_fail;
#ifdef MAP_R2
	u16 assoc_status_code;
	u16 assoc_reason_code;
	u16 send_failed_assoc_frame;
#endif
} wapp_sta_cnnct_rej_info;

struct GNU_PACKED csa_info_rsp {
	unsigned char ruid[ETH_ALEN];
	unsigned char new_ch;
};

struct GNU_PACKED interface_info {
	unsigned char if_name[IFNAMSIZ];
	unsigned char if_role[6]; /* wiap,wista */
	unsigned char if_ch;      /* channel */
	unsigned char if_phymode[3];  /* n,ac */
	unsigned char if_mac_addr[ETH_ALEN];
	unsigned char identifier[ETH_ALEN];/*belong to which radio*/
};

struct GNU_PACKED interface_info_list_hdr {
	unsigned char interface_count;
	struct interface_info if_info[0];
};

struct GNU_PACKED bh_assoc_wireless_setting {
	unsigned char   bh_mac_addr[ETH_ALEN];
	unsigned char	target_ssid[33];
	unsigned char 	target_bssid[ETH_ALEN];
	unsigned short  auth_mode;
	unsigned short	encryp_type;
	unsigned char	wpa_key[65];
	unsigned char   target_channel;
};

typedef struct GNU_PACKED wapp_sta_activity_status {
	uint8_t status;
	unsigned char sta_mac[ETH_ALEN];
} wapp_sta_status_info;

struct GNU_PACKED map_vendor_ie
{
	unsigned char type;
	unsigned char subtype;
	unsigned char root_distance;
	unsigned char connectivity_to_controller;
	unsigned short uplink_rate;
	unsigned char uplink_bssid[ETH_ALEN];
	unsigned char bssid_5g[ETH_ALEN];
	unsigned char bssid_2g[ETH_ALEN];
};

struct GNU_PACKED wsc_apcli_config {
        char ssid[32 + 1];
        unsigned char SsidLen;
        unsigned short AuthType;
        unsigned short EncrType;
        unsigned char Key[64];
        unsigned short KeyLength;
        unsigned char KeyIndex;
        unsigned char bssid[6];
        unsigned char peer_map_role;
        unsigned char own_map_role;
};

struct GNU_PACKED wsc_apcli_config_msg {
	unsigned int profile_count;
	struct wsc_apcli_config apcli_config[0];
};

typedef struct GNU_PACKED _wdev_ht_cap {
	unsigned char	tx_stream;
	unsigned char	rx_stream;
	unsigned char	sgi_20;
	unsigned char	sgi_40;
	unsigned char	ht_40;
} wdev_ht_cap;

typedef struct GNU_PACKED _wdev_vht_cap {
	unsigned char	sup_tx_mcs[2];
	unsigned char	sup_rx_mcs[2];
	unsigned char	tx_stream;
	unsigned char	rx_stream;
	unsigned char	sgi_80;
	unsigned char	sgi_160;
	unsigned char	vht_160;
	unsigned char	vht_8080;
	unsigned char	su_bf;
	unsigned char	mu_bf;
} wdev_vht_cap;

typedef struct GNU_PACKED _wdev_he_cap {
        unsigned char he_mcs_len;
        unsigned char he_mcs[MAX_HE_MCS_LEN];
        unsigned char tx_stream;
        unsigned char rx_stream;
        unsigned char he_8080;
        unsigned char he_160;
        unsigned char su_bf_cap;
        unsigned char mu_bf_cap;
        unsigned char ul_mu_mimo_cap;
        unsigned char ul_mu_mimo_ofdma_cap;
        unsigned char dl_mu_mimo_ofdma_cap;
        unsigned char ul_ofdma_cap;
        unsigned char dl_ofdma_cap;
} wdev_he_cap;

struct GNU_PACKED bss_info {
	unsigned char Bssid[ETH_ALEN];
	unsigned char Channel;
	unsigned char CentralChannel;   /*Store the wide-band central channel for 40MHz.  .used in 40MHz AP. Or this is the same as Channel. */
	signed char Rssi;
	char MinSNR;
	unsigned char Privacy;/* Indicate security function ON/OFF. Don't mess up with auth mode. */
	unsigned char SsidLen;
	char Ssid[MAX_SSID_LEN];
	unsigned short AuthMode;
	unsigned short EncrypType;
	wdev_ht_cap ht_cap;
	wdev_vht_cap vht_cap;
	wdev_he_cap he_cap;
	unsigned char map_vendor_ie_found;
	struct map_vendor_ie map_info;
};

struct GNU_PACKED wapp_scan_info {
	unsigned int interface_index;
	unsigned char more_bss;
	unsigned char bss_count;
	struct bss_info bss[0];
};

struct GNU_PACKED tp_metrics {
	unsigned char mac[ETH_ALEN];
	unsigned char bssid[ETH_ALEN];
	unsigned int tx_tp;
	unsigned int rx_tp;
	unsigned char is_APCLI;
};

struct GNU_PACKED sta_tp_metrics {
	unsigned char identifier[ETH_ALEN];
	unsigned char sta_cnt;
	struct tp_metrics info[0];
};

struct GNU_PACKED enrollee_bh {
	unsigned char if_type;    				/*0-eth 1-wireless*/
	unsigned char mac_address[ETH_ALEN];    /*mac address of the interface*/
};

struct GNU_PACKED bss_role {
	unsigned char bssid[ETH_ALEN];			/*bssid*/
	unsigned char role;                     /*the bss type, could be BIT_BH_BSS, BIT_FH_BSS and BIT_BH_BSS|BIT_FH_BSS*/
};

struct GNU_PACKED trigger_wps_param {
	unsigned char if_mac[ETH_ALEN];         /*interface mac address on which the wps trggers on */
	unsigned char mode;                     /*WPS mode, 2-PBC */
};

typedef enum {
	WAPP_APCLI_DISASSOCIATED = 0,
	WAPP_APCLI_ASSOCIATED,
} WAPP_APCLI_ASSOC_STATE;

struct GNU_PACKED wapp_apcli_association_info {
	unsigned int interface_index;
	unsigned char apcli_assoc_state;
	char rssi;
	unsigned char current_channel;
	unsigned char peer_map_enable;
};

struct GNU_PACKED dhcp_ctl_req
{
	unsigned int dhcp_server_enable;
	unsigned int dhcp_client_enable;
};
#ifndef TS_MAX
#define TS_MAX 30
#endif
#define N_MAX 10
#define MAX_LEN_OF_SSID 32
#define MAX_CH_BW_LEN 10
#define MAX_TS_LEN 30

struct GNU_PACKED neighbor_info {
	u8 bssid[ETH_ALEN];
	u8 ssid_len;
	u8 ssid[MAX_LEN_OF_SSID];
	u8 RCPI;
	u8 ch_bw_len;
	u8 ch_bw[MAX_CH_BW_LEN];
	u8 cu_stacnt_present; //bit7 : CU, bit-6 Stacnt
	u8 cu;
	u16 sta_cnt;
};

struct GNU_PACKED off_ch_scan_result_event {
	u8 radio_id[ETH_ALEN];
	u8 channel;
	u8 utilization;
	u8 noise;
	u16 neighbor_num;
};

struct GNU_PACKED off_ch_scan_report_event {
	u8 scan_result_num;
	struct off_ch_scan_result_event scan_result[0];  //struct scan_result_tlv
};

#ifdef MAP_R2
struct GNU_PACKED timestamp_tlv {
	u8 tlvType;
	u16 tlvLen;
	u8 timestamp_len;
	u8 timestamp[TS_MAX];
};
#endif

struct GNU_PACKED net_opt_scan_result_event {
	u8 radio_id[ETH_ALEN];
	u8 oper_class;
	u8 channel;
	u8 scan_status;
	u8 timestamp_len;
	u8 timestamp[MAX_TS_LEN];
	u8 utilization;
	u8 noise;
	u32 agg_scan_duration;
	u8 scan_type;
	u16 neighbor_num;
#ifdef MAP_R2
	u32 rx_time;
	u32 tx_time;
	u32 obss_time;
	u32 edcca;
#endif
	struct neighbor_info nb_info[0];//struct neighbor_info *nb_info;
};
struct GNU_PACKED net_opt_scan_report_event {
#ifdef MAP_R2
	struct timestamp_tlv timestamp;
#endif
	u8 scan_result_num;
	struct net_opt_scan_result_event scan_result[0];
};

#ifdef MAP_R2
#define MAP_MAX_RADIO 3
#define OP_CLASS_PER_RADIO 6
#ifndef TS_MAX
#define TS_MAX 30
#endif
#define MAX_LEN_OF_SSID 32
#define MAX_CH_BW_LEN 10

struct GNU_PACKED channel_body {
	unsigned char oper_class;
	unsigned char ch_list_num;
	unsigned char ch_list[MAX_CH_NUM];
};

struct GNU_PACKED radio_scan_capab {
	unsigned char radio_id[ETH_ALEN];
	unsigned char boot_scan_only;
	unsigned char scan_impact;
	unsigned int min_scan_interval;
	unsigned char oper_class_num;
	struct channel_body ch_body[OP_CLASS_PER_RADIO];

};

struct GNU_PACKED channel_scan_capab {
	unsigned char radio_num;
	struct radio_scan_capab radio_scan_params[MAP_MAX_RADIO];
};
#endif
struct GNU_PACKED chnList {
	u8 channel;
	u8 pref;
	u16 cac_timer;
};

#ifdef MAP_R2
struct GNU_PACKED ap_r2_capability {
	unsigned char max_total_num_sp_rules;
	unsigned char reserved1;

	unsigned char rsv_bit0_3:4;
	unsigned char enhanced_sp_flag:1;
	unsigned char basic_sp_flag:1;
	unsigned char byte_counter_units:2; /*0: bytes, 1: kibibytes (KiB), 2: mebibytes (MiB), 3: reserved*/

	unsigned char max_total_num_vid;
};

struct GNU_PACKED scan_body {
	u8 radio_id[ETH_ALEN];
	u8 oper_class_num;
	struct channel_body ch_body[OP_CLASS_PER_RADIO];
};

struct GNU_PACKED channel_scan_req {
	u8 bw;
	u8 fresh_scan; // 8: perform fresh scan, 0: return stored result
	u8 radio_num;
	u8 neighbour_only;
	struct scan_body body[0];
};

#if 0
struct GNU_PACKED neighbor_info {
	u8 bssid[ETH_ALEN];
	u8 ssid_len;
	u8 ssid[MAX_LEN_OF_SSID];
	u8 RCPI;
	u8 ch_bw_len;
	u8 ch_bw[MAX_CH_BW_LEN];
	u8 cu_stacnt_present; //bit7 : CU, bit-6 Stacnt
	u8 cu;
	u16 sta_cnt;
};
#endif

struct GNU_PACKED scan_result_event {
	u8 radio_id[ETH_ALEN];
	u8 oper_class;
	u8 channel;
	u8 scan_status;
	u8 timestamp_len;
	u8 timestamp[TS_MAX];
	u8 utilization;
	u8 noise;
	u32 agg_scan_duration;
	u8 scan_type;
	u16 neighbor_num;
	struct neighbor_info nb_info[0];//struct neighbor_info *nb_info;
};


struct GNU_PACKED channel_scan_report_event {
	struct timestamp_tlv timestamp;
	u8 scan_result_num;
	struct scan_result_event scan_result[0];  //struct scan_result_tlv
};

struct GNU_PACKED status {
	u8 bssid[ETH_ALEN];
	u8 status;
};

struct GNU_PACKED notification_tlv {
	u8 bssid_num;
	struct status tlv_status[0];
};

struct GNU_PACKED assoc_notification {
	u8 notification_tlv_num;
	struct notification_tlv tlv[0];
};
struct GNU_PACKED tunneled_tlv {
	unsigned int payload_len;
	unsigned char payload[0];
 };

struct GNU_PACKED tunneled_msg {
	unsigned char sta_mac[ETH_ALEN];
	unsigned char proto_type;
	unsigned char num_tunneled_tlv;
	struct tunneled_tlv tlv[0];
};

#endif
struct GNU_PACKED cac_completion_opcap
{
        unsigned char op_class;    // This field shall be 0, if radar was not detected.
        unsigned char ch_num;       // This field shall be 0, if radar was not detected.
};

struct GNU_PACKED cac_completion_status
{
        unsigned char identifier[ETH_ALEN];
        unsigned char op_class;
        unsigned char channel;
        unsigned char cac_status;
        unsigned char op_class_num;    // This field shall be 0, if radar was not detected.
        struct cac_completion_opcap opcap[0];
};


struct GNU_PACKED cac_completion_report
{
        unsigned char radio_num;
        struct cac_completion_status cac_completion_status[0];
        //struct cac_report_opcap opcap[0];         // This field shall be 0, if radar was not detected.
};
#ifdef MAP_R2
#ifdef DFS_CAC_R2
struct GNU_PACKED cac_tlv {
	unsigned char identifier[ETH_ALEN];
	unsigned char op_class_num;
	unsigned char ch_num;
	unsigned char cac_method;
	unsigned char cac_action;
};

struct GNU_PACKED cac_request {
	unsigned char num_radio;
	struct cac_tlv tlv[0];
};


struct GNU_PACKED cac_term_tlv {
	unsigned char identifier[ETH_ALEN];
	unsigned char op_class_num;
	unsigned char ch_num;
};

struct GNU_PACKED cac_terminate {
	unsigned char num_radio;
	struct cac_term_tlv tlv[0];
};

struct GNU_PACKED cac_cap_opcap
{
        unsigned char op_class;
        unsigned char ch_num;
        unsigned char ch_list[0]; //MAX of 5G Channels
};

struct GNU_PACKED cac_cap_type
{
        unsigned char cac_mode;
        unsigned char cac_interval[3];
        unsigned char op_class_num;
        struct cac_cap_opcap opcap[0];
};


struct GNU_PACKED cac_cap_tlv
{
        unsigned char identifier[ETH_ALEN];
        unsigned char cac_type_num;
        struct cac_cap_type type[0];
};


struct GNU_PACKED cac_capability
{
        unsigned char country_code[2];
        unsigned char radio_num;
        struct cac_cap_tlv cap[0];
};

#define MAX_CLASS_CHANNEL 68 //Total Operating class/channel pairs possible
#define MAX_CLASS_CHAN_NON_ALLOWED 52 //Non Allowed Operating class - Channel possible
struct GNU_PACKED cac_allowed_ch
{
	u8 op_class;
	u8 ch_num;
	u16 cac_interval;// This field shall be 0, for Non-DFS Channels
};
struct GNU_PACKED cac_non_allowed_ch
{
	u8 op_class;
	u8 ch_num;
	u16 remain_interval;
};
struct GNU_PACKED cac_ongoing_ch
{
	u8 op_class;
	u8 ch_num;
	u32 remain_interval;
};
struct GNU_PACKED cac_status_report
{
	u8 allowed_channel_num;
	u8 non_allowed_channel_num;
	u8 ongoing_cac_channel_num;
	struct cac_allowed_ch allowed_channel[MAX_CLASS_CHANNEL];
	struct cac_non_allowed_ch non_allowed_channel[MAX_CLASS_CHAN_NON_ALLOWED];
	struct cac_ongoing_ch cac_ongoing_channel[0];
};
#endif
#endif //MAP_R2
#ifdef ACL_CTRL
struct GNU_PACKED acl_ctrl {
	unsigned char type;
	unsigned char cmd;
	unsigned char bssid[ETH_ALEN];
	unsigned char sta_list_count;
	unsigned char sta_mac[0];
};
#endif
#endif /*INTERFACE_H*/
