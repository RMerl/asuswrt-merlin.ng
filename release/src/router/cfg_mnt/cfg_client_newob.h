#ifndef __CFG_CLIENT_H__
#define __CFG_CLIENT_H__

typedef struct securityInfo_t
{
	unsigned char *publicKey;	// public key
	unsigned char *masterKey;	// master key
	unsigned char *serverNounce;	// server nounce
	unsigned char *clientNounce;	// client nounce
	//unsigned char *sessionKey;
	size_t publicKeyLen;		// the length of public key
	size_t masterKeyLen;		// the length of master key
	size_t serverNounceLen;		// the length of server nounce
	size_t clientNounceLen;		// the length of client nounce
	//size_t sessionKeyLen;
} securityInfo;

struct packetHandler
{
    int type;
    int (*func)(int sock, CM_CTRL *pCtrlBK, TLV_Header tlv, securityInfo *keyInfo, unsigned char *packetMsg);
};

extern int cm_processRES_KU(int sock, CM_CTRL *pCtrlBK, TLV_Header tlv, securityInfo *keyInfo, unsigned char *packetMsg);
extern int cm_processRES_NC(int sock, CM_CTRL *pCtrlBK, TLV_Header tlv, securityInfo *keyInfo, unsigned char *packetMsg);
extern int cm_processRSP_CHK(int sock, CM_CTRL *pCtrlBK, TLV_Header tlv, securityInfo *keyInfo, unsigned char *packetMsg);
#ifdef SYNC_WCHANNEL
extern int cm_processRSP_RPT(int sock, CM_CTRL *pCtrlBK, TLV_Header tlv, securityInfo *keyInfo, unsigned char *packetMsg);
#endif
extern int cm_processACK_OK(int sock, CM_CTRL *pCtrlBK, TLV_Header tlv, securityInfo *keyInfo, unsigned char *packetMsg);
extern int cm_processREQ_NTF(int sock, CM_CTRL *pCtrlBK, TLV_Header tlv, securityInfo *keyInfo, unsigned char *packetMsg);
extern int cm_processRSP_GKEY(int sock, CM_CTRL *pCtrlBK, TLV_Header tlv, securityInfo *keyInfo, unsigned char *packetMsg);
#ifdef SYNC_WCHANNEL
extern int cm_processRSP_CHANSYNC(int sock, CM_CTRL *pCtrlBK, TLV_Header tlv, securityInfo *keyInfo, unsigned char *packetMsg);
#endif
extern int cm_processRSP_JOIN(int sock, CM_CTRL *pCtrlBK, TLV_Header tlv, securityInfo *keyInfo, unsigned char *packetMsg);
extern int cm_processRSP_COST(int sock, CM_CTRL *pCtrlBK, TLV_Header tlv, securityInfo *keyInfo, unsigned char *packetMsg);
#ifdef ONBOARDING
extern int cm_processRSP_GROUPID(int sock, CM_CTRL *pCtrlBK, TLV_Header tlv, securityInfo *keyInfo, unsigned char *packetMsg);
#endif
extern int cm_processRSP_SREKEY(int sock, CM_CTRL *pCtrlBK, TLV_Header tlv, securityInfo *keyInfo, unsigned char *packetMsg);
extern int cm_processRSP_GREKEY(int sock, CM_CTRL *pCtrlBK, TLV_Header tlv, securityInfo *keyInfo, unsigned char *packetMsg);
extern int cm_processRSP_TOPOLOGY(int sock, CM_CTRL *pCtrlBK, TLV_Header tlv, securityInfo *keyInfo, unsigned char *packetMsg);
extern int cm_processRSP_RELIST(int sock, CM_CTRL *pCtrlBK, TLV_Header tlv, securityInfo *keyInfo, unsigned char *packetMsg);
#ifdef RTCONFIG_BCN_RPT
extern int cm_processRSP_APLIST(int sock, CM_CTRL *pCtrlBK, TLV_Header tlv, securityInfo *keyInfo, unsigned char *packetMsg);
#endif
#ifdef DUAL_BAND_DETECTION
extern int cm_processRSP_DBLIST(int sock, CM_CTRL *pCtrlBK, TLV_Header tlv, securityInfo *keyInfo, unsigned char *packetMsg);
#endif
#ifdef RTCONFIG_FRONTHAUL_DWB
extern int cm_processRSP_BACKHUALSTATUS(int sock, CM_CTRL *pCtrlBK, TLV_Header tlv, securityInfo *keyInfo, unsigned char *packetMsg);
#endif
extern int cm_processRSP_LEVEL(int sock, CM_CTRL *pCtrlBK, TLV_Header tlv, securityInfo *keyInfo, unsigned char *packetMsg);

#ifdef RTCONFIG_AMAS_NEWOB
extern int cm_processRES_KU_NEWOB(int sock, CM_CTRL *pCtrlBK, TLV_Header tlv, securityInfo *keyInfo, unsigned char *packetMsg);
extern int cm_processRES_NC_NEWOB(int sock, CM_CTRL *pCtrlBK, TLV_Header tlv, securityInfo *keyInfo, unsigned char *packetMsg);
extern int cm_processACK_OK_NEWOB(int sock, CM_CTRL *pCtrlBK, TLV_Header tlv, securityInfo *keyInfo, unsigned char *packetMsg);
extern int cm_processACK_INFO(int sock, CM_CTRL *pCtrlBK, TLV_Header tlv, securityInfo *keyInfo, unsigned char *packetMsg);
extern int cm_processREQ_ADD(int sock, CM_CTRL *pCtrlBK, TLV_Header tlv, securityInfo *keyInfo, unsigned char *packetMsg);
#endif

struct packetHandler packetHandlers[] = {
	{ RES_KU,	cm_processRES_KU },
	{ RES_NC,	cm_processRES_NC },
	{ RSP_CHK,	cm_processRSP_CHK },
#ifdef SYNC_WCHANNEL
	{ RSP_RPT,	cm_processRSP_RPT },
#endif
	{ ACK_OK,	cm_processACK_OK },
	{ REQ_NTF,	cm_processREQ_NTF },
	{ RSP_GKEY,	cm_processRSP_GKEY },
#ifdef SYNC_WCHANNEL
	{ RSP_CHANSYNC,	cm_processRSP_CHANSYNC },
#endif
	{ RSP_JOIN, cm_processRSP_JOIN },
	{ RSP_COST, cm_processRSP_COST },
#ifdef ONBOARDING
	{ RSP_GROUPID, cm_processRSP_GROUPID },
#endif
	{ RSP_SREKEY,	cm_processRSP_SREKEY },
	{ RSP_GREKEY,	cm_processRSP_GREKEY },
	{ RSP_TOPOLOGY,	cm_processRSP_TOPOLOGY },
	{ RSP_RELIST,	cm_processRSP_RELIST },
#ifdef RTCONFIG_BCN_RPT
	{ RSP_APLIST,	cm_processRSP_APLIST },
#endif
#ifdef DUAL_BAND_DETECTION
	{ RSP_DBLIST,	cm_processRSP_DBLIST },
#endif
#ifdef RTCONFIG_FRONTHAUL_DWB
	{ RSP_BACKHUALSTATUS, cm_processRSP_BACKHUALSTATUS },
#endif
	{ RSP_LEVEL, cm_processRSP_LEVEL },
#ifdef RTCONFIG_AMAS_NEWOB
	{ RES_KU_NEWOB,	cm_processRES_KU_NEWOB },
	{ RES_NC_NEWOB,	cm_processRES_NC_NEWOB },
	{ ACK_OK_NEWOB,	cm_processACK_OK_NEWOB },
	{ ACK_INFO, cm_processACK_INFO },
	{ REQ_ADD, cm_processREQ_ADD },
#endif
	{ -1, NULL }
};

#define PID_CM_CLIENT		"/var/run/cfg_client.pid"
#define INTERVAL_RETRY_CONNECT	5
#define CFG_JSON_FILE		"/tmp/cfg.json"
#define CHECK_TIME_INTERVAL	100
#define REPORT_TIME_INTERVAL	30
#define CHECK_WCHANNEL_INTERVAL	10
#define CHECK_KEY_INTERVAL	60
#define GET_TOPOLOGY_INTERVAL	30
#define PERIODIC_EXECUTION_TIME	(300 * 1000)	// 300 microsecond
#define PRIVATE_CFG_JSON_PATH			"/tmp/private_cfg.json"
#define CHANGED_CFG_JSON_PATH		"/tmp/changed_cfg.json"
#ifdef MASTER_DET
#define THRESHOLD_DISCONNECT_COUNT	3
#endif
#define SCHEDULER_KEEPALIVE_THRESHOLD		200	/* based on PERIODIC_EXECUTION_TIME, it will be 60 sec */
#define CHECK_KEEPALIVE_INTERVAL	5
#define CONNECT_KEEPALIVE_THRESHOLD	12	/* based on CHECK_KEEPALIVE_INTERVAL, it will be 60 sec */
#define MAX_SOCK_CONNECT_RETRY_COUNT	3
#define MAX_SOCK_CONNECT_RETRY_TIMEWAIT	1000		// 1 sec
#ifdef RTCONFIG_BH_SWITCH_ETH_FIRST
#define DISCONNECT_THRESHOLD	1
#else
#define DISCONNECT_THRESHOLD	3
#endif
#define PENDING_TIME	3
#ifdef RTCONFIG_AMAS_NEWOB
#define INFO_REPORT_TIME_INTERVAL 5
#define CHECK_INFO_REPORT_THRESHOLD	6	/* based on INFO_REPORT_TIME_INTERVAL, it will be 30 sec */
#define OB_REQ_MAX_FAIL_COUNT	3
#endif

enum states {
	START = 0,
	INIT,
	REKEY,
	DISCONN,
	PENDING,
	CONN,
	PERCHECK,	// periodic check
	IMMCHECK,	// check immediatly
	GREKEY,		/* group rekey */
	RESTART,
	MAX_STATE
};

/* feature list, last one must be SUBFT_END */
#if defined(RTCONFIG_WIFI_SON)
int common_ft_1905_control[] = {
	SUBFT_BASIC_BAND1,
	SUBFT_CHANNEL_BAND1,
	SUBFT_BASIC_BAND2,
	SUBFT_CHANNEL_BAND2,
	SUBFT_END};
#endif

int common_ft_list[] = {
	SUBFT_SMART_CONNECT,
	SUBFT_ROAMING_ASSISTANT,
	SUBFT_BSD_STEERING_POLICY_5G_X,
	SUBFT_BSD_STA_SELECT_POLICY_5G_X,
	SUBFT_BSD_IF_SELECT_5G_X,
	SUBFT_BSD_IF_QUALIFY_5G_X,
#ifdef SUPPORT_TRI_BAND
	SUBFT_BSD_STEERING_POLICY_5G1_X,
	SUBFT_BSD_STA_SELECT_POLICY_5G1_X,
	SUBFT_BSD_IF_SELECT_5G1_X,
	SUBFT_BSD_IF_QUALIFY_5G1_X,
#endif
	SUBFT_BSD_BOUNCE_DETECT,
	SUBFT_BSD_BOUNCE_DETECT_X,
	SUBFT_WPS,
	SUBFT_ROUTER_LOGIN,
	SUBFT_TIMEZONE,
	SUBFT_NTP_SERVER,
	SUBFT_TELNET_SERVER,
	SUBFT_SSH_SERVER,
	SUBFT_LOG_SERVER,
	SUBFT_FEEDBACK,
	SUBFT_MISCELLANEOUS,
#ifdef RTCONFIG_BHCOST_OPT
	SUBFT_BACKHAULCTRL_EAP,
#endif
	SUBFT_REBOOT_SCHEDULE,
#if defined(RTCONFIG_AMAS_WGN)
	SUBFT_VLAN_RULELIST,
	SUBFT_BW_LIMIT,
#endif	/* RTCONFIG_AMAS_WGN */		
#if defined(RTCONFIG_HND_ROUTER_AX)
	SUBFT_ACS_INCLUDE_DFS,
#endif
#if defined(RTCONFIG_BCN_RPT)
	SUBFT_RSSI_METHOD,
#endif
	SUBFT_CONNECTION_DIAGMOSTIC,
#if defined(STA_BIND_AP)
	SUBFT_STA_BIND_AP,
#endif
#if defined(RTCONFIG_FRONTHAUL_DWB)
	SUBFT_DWBCTRL_FRONTHAUL,
#endif
#if defined(RTCONFIG_FANCTRL)
	SUBFT_FANCTRL,
#endif
#ifdef RTCONFIG_QCA_PLC2
	SUBFT_PLC_MASTER,
#endif
#if defined(RTCONFIG_HTTPS)
	SUBFT_LOCAL_ACCESS,
#endif
	SUBFT_END
};

int wifi_band1_ft_list[] = {
	SUBFT_BASIC_BAND1,
	SUBFT_CHANNEL_BAND1,
	SUBFT_ADVANCED_BAND1,
	SUBFT_MACFILTER_BAND1,
	SUBFT_RADIUS_BAND1,
	SUBFT_ROAMING_ASSISTANT_BAND1,
	SUBFT_TIMESCHED_BAND1,
	SUBFT_TIMESCHEDV2_BAND1,
	SUBFT_BSD_STEERING_POLICY_BAND1,
	SUBFT_BSD_STA_SELECT_POLICY_BAND1,
	SUBFT_BSD_IF_SELECT_BAND1,
	SUBFT_BSD_IF_QUALIFY_BAND1,
	SUBFT_BASIC_BAND1_G1,
	SUBFT_BASIC_BAND1_G2,
	SUBFT_BASIC_BAND1_G3,
	SUBFT_ADVANCED_BAND1_G1,
	SUBFT_ADVANCED_BAND1_G2,
	SUBFT_ADVANCED_BAND1_G3,
#if defined(RTCONFIG_AMAS_WGN)
	SUBFT_GUEST_MISC_BAND1_G1,
	SUBFT_GUEST_MISC_BAND1_G2,
	SUBFT_GUEST_MISC_BAND1_G3,
	SUBFT_MACFILTER_BAND1_G1,
	SUBFT_MACFILTER_BAND1_G2,
	SUBFT_MACFILTER_BAND1_G3,
	SUBFT_BW_LIMIT_BAND1_G1,
	SUBFT_BW_LIMIT_BAND1_G2,
	SUBFT_BW_LIMIT_BAND1_G3,
#endif
#if defined(SUPPORT_11AX)
	SUBFT_11_AX_BAND1,
	SUBFT_OFDMA_BAND1,
#endif
#if defined(RTCONFIG_BW160M)
	SUBFT_BW_160_BAND1,
#endif
	SUBFT_END
};

int wifi_band2_ft_list[] = {
	SUBFT_BASIC_BAND2,
	SUBFT_CHANNEL_BAND2,
	SUBFT_ADVANCED_BAND2,
	SUBFT_MACFILTER_BAND2,
	SUBFT_RADIUS_BAND2,
	SUBFT_ROAMING_ASSISTANT_BAND2,
	SUBFT_TIMESCHED_BAND2,
	SUBFT_TIMESCHEDV2_BAND2,
	SUBFT_BSD_STEERING_POLICY_BAND2,
	SUBFT_BSD_STA_SELECT_POLICY_BAND2,
	SUBFT_BSD_IF_SELECT_BAND2,
	SUBFT_BSD_IF_QUALIFY_BAND2,
	SUBFT_BASIC_BAND2_G1,
	SUBFT_BASIC_BAND2_G2,
	SUBFT_BASIC_BAND2_G3,
	SUBFT_ADVANCED_BAND2_G1,
	SUBFT_ADVANCED_BAND2_G2,
	SUBFT_ADVANCED_BAND2_G3,
#if defined(RTCONFIG_AMAS_WGN)
	SUBFT_GUEST_MISC_BAND2_G1,
	SUBFT_GUEST_MISC_BAND2_G2,
	SUBFT_GUEST_MISC_BAND2_G3,
	SUBFT_MACFILTER_BAND2_G1,
	SUBFT_MACFILTER_BAND2_G2,
	SUBFT_MACFILTER_BAND2_G3,
	SUBFT_BW_LIMIT_BAND2_G1,
	SUBFT_BW_LIMIT_BAND2_G2,
	SUBFT_BW_LIMIT_BAND2_G3,
#endif
#if defined(SUPPORT_11AX)
	SUBFT_11_AX_BAND2,
	SUBFT_OFDMA_BAND2,
#endif
#if defined(RTCONFIG_BW160M)
	SUBFT_BW_160_BAND2,
#endif
	SUBFT_END
};

int wifi_band3_ft_list[] = {
	SUBFT_BASIC_BAND3,
	SUBFT_CHANNEL_BAND3,
	SUBFT_ADVANCED_BAND3,
	SUBFT_MACFILTER_BAND3,
	SUBFT_RADIUS_BAND3,
	SUBFT_ROAMING_ASSISTANT_BAND3,
	SUBFT_TIMESCHED_BAND3,
	SUBFT_TIMESCHEDV2_BAND3,
	SUBFT_BSD_STEERING_POLICY_BAND3,
	SUBFT_BSD_STA_SELECT_POLICY_BAND3,
	SUBFT_BSD_IF_SELECT_BAND3,
	SUBFT_BSD_IF_QUALIFY_BAND3,
	SUBFT_BASIC_BAND3_G1,
	SUBFT_BASIC_BAND3_G2,
	SUBFT_BASIC_BAND3_G3,
	SUBFT_ADVANCED_BAND3_G1,
	SUBFT_ADVANCED_BAND3_G2,
	SUBFT_ADVANCED_BAND3_G3,
#if defined(RTCONFIG_AMAS_WGN)
	SUBFT_GUEST_MISC_BAND3_G1,
	SUBFT_GUEST_MISC_BAND3_G2,
	SUBFT_GUEST_MISC_BAND3_G3,
	SUBFT_MACFILTER_BAND3_G1,
	SUBFT_MACFILTER_BAND3_G2,
	SUBFT_MACFILTER_BAND3_G3,
	SUBFT_BW_LIMIT_BAND3_G1,
	SUBFT_BW_LIMIT_BAND3_G2,
	SUBFT_BW_LIMIT_BAND3_G3,
#endif
#if defined(SUPPORT_11AX)
	SUBFT_11_AX_BAND3,
	SUBFT_OFDMA_BAND3,
#endif
#if defined(RTCONFIG_BW160M)
	SUBFT_BW_160_BAND3,
#endif
	SUBFT_END
};

int wifi_band4_ft_list[] = {
	SUBFT_BASIC_BAND4,
	SUBFT_CHANNEL_BAND4,
	SUBFT_ADVANCED_BAND4,
	SUBFT_MACFILTER_BAND4,
	SUBFT_RADIUS_BAND4,
	SUBFT_ROAMING_ASSISTANT_BAND4,
	SUBFT_TIMESCHED_BAND4,
	SUBFT_TIMESCHEDV2_BAND4,
	SUBFT_BSD_STEERING_POLICY_BAND4,
	SUBFT_BSD_STA_SELECT_POLICY_BAND4,
	SUBFT_BSD_IF_SELECT_BAND4,
	SUBFT_BSD_IF_QUALIFY_BAND4,
	SUBFT_BASIC_BAND4_G1,
	SUBFT_BASIC_BAND4_G2,
	SUBFT_BASIC_BAND4_G3,
	SUBFT_ADVANCED_BAND4_G1,
	SUBFT_ADVANCED_BAND4_G2,
	SUBFT_ADVANCED_BAND4_G3,
#if defined(RTCONFIG_AMAS_WGN)
	SUBFT_GUEST_MISC_BAND4_G1,
	SUBFT_GUEST_MISC_BAND4_G2,
	SUBFT_GUEST_MISC_BAND4_G3,
	SUBFT_MACFILTER_BAND4_G1,
	SUBFT_MACFILTER_BAND4_G2,
	SUBFT_MACFILTER_BAND4_G3,
	SUBFT_BW_LIMIT_BAND4_G1,
	SUBFT_BW_LIMIT_BAND4_G2,
	SUBFT_BW_LIMIT_BAND4_G3,
#endif
#if defined(SUPPORT_11AX)
	SUBFT_11_AX_BAND4,
	SUBFT_OFDMA_BAND4,
#endif
#if defined(RTCONFIG_BW160M)
	SUBFT_BW_160_BAND4,
#endif
	SUBFT_END
};


/* private feature list, last one must be SUBFT_END */
int private_ft_list[] = { SUBFT_LOCATION, SUBFT_BACKHAULCTRL,

#ifdef RTCONFIG_BHCOST_OPT
		SUBFT_FORCE_TOPOLOGY,
#endif
#if defined(RTCONFIG_LACP)
		SUBFT_LINK_AGGREGATION,
#endif
		SUBFT_END};

#endif /* __CFG_CLIENT_H__ */

/* End of cfg_client.h */
