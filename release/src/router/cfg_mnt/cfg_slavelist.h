#ifndef __CFG_SLAVELIST_H__
#define __CFG_SLAVELIST_H__

#include <json.h>

#define REPORT_TIME_INTERVAL	30
#define OFFLINE_THRESHOLD	(REPORT_TIME_INTERVAL * 3)

#define CFG_FILE_LOCK		"cfg_mnt"
#define KEY_SHM_CFG		2001
#define CFG_CLIENT_NUM		16
#define MAC_LIST_JSON_FILE	"/tmp/maclist.json"
#define ALIAS_LEN			33
#define IP_LEN				4
#define MAC_LEN				6
#define FWVER_LEN			33
#define MODEL_NAME_LEN		33
#define TERRITORY_CODE_LEN	33
#define RE_LIST_JSON_FILE	"/tmp/relist.json"
#define MAX_RELIST_COUNT	9
#define SSID_LEN				33
#define LLDP_STAT_LEN       128
#define RE_LIST_MAX_LEN		(MAX_RELIST_COUNT * 128)
enum reListAction {
	RELIST_ADD,
	RELIST_DEL,
	RELIST_UPDATE
};

typedef struct _CM_CLIENT_TABLE {
	char alias[CFG_CLIENT_NUM][ALIAS_LEN];
	unsigned char ipAddr[CFG_CLIENT_NUM][IP_LEN];
	unsigned char macAddr[CFG_CLIENT_NUM][MAC_LEN];
	unsigned char realMacAddr[CFG_CLIENT_NUM][MAC_LEN];
	time_t reportStartTime[CFG_CLIENT_NUM];
	unsigned char pap2g[CFG_CLIENT_NUM][MAC_LEN];
	unsigned char pap5g[CFG_CLIENT_NUM][MAC_LEN];
	char pap2g_ssid[CFG_CLIENT_NUM][SSID_LEN];
	char pap5g_ssid[CFG_CLIENT_NUM][SSID_LEN];
	int rssi2g[CFG_CLIENT_NUM];
	int rssi5g[CFG_CLIENT_NUM];
	unsigned char sta2g[CFG_CLIENT_NUM][MAC_LEN];
	unsigned char sta5g[CFG_CLIENT_NUM][MAC_LEN];
	unsigned char ap2g[CFG_CLIENT_NUM][MAC_LEN];
	unsigned char ap5g[CFG_CLIENT_NUM][MAC_LEN];
	unsigned char ap5g1[CFG_CLIENT_NUM][MAC_LEN];
	unsigned char apDwb[CFG_CLIENT_NUM][MAC_LEN];
	char ap2g_ssid[CFG_CLIENT_NUM][SSID_LEN];
	char ap5g_ssid[CFG_CLIENT_NUM][SSID_LEN];
	char ap5g1_ssid[CFG_CLIENT_NUM][SSID_LEN];
	int level[CFG_CLIENT_NUM];
	char fwVer[CFG_CLIENT_NUM][FWVER_LEN];
	char newFwVer[CFG_CLIENT_NUM][FWVER_LEN];
	char modelName[CFG_CLIENT_NUM][MODEL_NAME_LEN];
	char territoryCode[CFG_CLIENT_NUM][TERRITORY_CODE_LEN];
	int activePath[CFG_CLIENT_NUM];
	int bandnum[CFG_CLIENT_NUM];
	int online[CFG_CLIENT_NUM];
	int maxLevel;
	int count;
    char lldp_wlc_stat[CFG_CLIENT_NUM][LLDP_STAT_LEN];
    char lldp_eth_stat[CFG_CLIENT_NUM][LLDP_STAT_LEN];
#ifdef RTCONFIG_FRONTHAUL_DWB
	int BackhualStatus[CFG_CLIENT_NUM]; // bits 0(update or not) 0(reserved) 0(reserved) 0(used or not)
#endif
#ifdef RTCONFIG_BHCOST_OPT
	unsigned int joinTime[CFG_CLIENT_NUM];
#endif
} CM_CLIENT_TABLE, *P_CM_CLIENT_TABLE;

extern int cm_checkReListExist(char *Mac);
extern int cm_checkReListUpdate(char *newReMac, char *sta2gMac, char *sta5gMac);
extern void cm_updateReList(char *newReMac, char *sta2gMac, char *sta5gMac, int action);
extern void cm_handleReListUpdate(unsigned char *decodeMsg);
extern int cm_isReWifiUpstreamMac(char *staMac);
extern int cm_prepareReListMsg(char *msg, int msgLen);
extern void cm_generateReList();
extern void cm_updateReListTimestamp(unsigned char *decodeMsg);
extern int cm_isSlaveOnline(time_t startTime);
#ifdef RTCONFIG_BCN_RPT
extern void cm_handleAPListUpdate(unsigned char *decodeMsg);
extern int cm_prepareAPListMsg(char *msg, int msgLen);
#endif
extern void cm_updateTribandReList(const char *newReMac, int bandNum, char *modelName, int action, int commit);
#ifdef RTCONFIG_BHCOST_OPT
extern json_object *cm_recordReListArray(CM_CLIENT_TABLE *clientTbl, char *reMac);
#endif
extern void cm_setReOffline(time_t *startTime);
extern int cm_getReMacByIp(CM_CLIENT_TABLE *clientTbl, char *reIp, char *reMac, int macLen);
extern int cm_getReTrafficMacByIp(CM_CLIENT_TABLE *clientTbl, char *reIp, char *reMac, int macLen);
extern int cm_getReIpByReMac(CM_CLIENT_TABLE *clientTbl, char *reMac, char *reIp, int ipLen);
extern int cm_getReTrafficMacByReMac(CM_CLIENT_TABLE *clientTbl, char *reMac, char *reTrafficMac, int macLen);
extern int cm_getReMacBy2gMac(CM_CLIENT_TABLE *clientTbl, char *mac2g, char *reMac, int macLen);
#ifdef ONBOARDING_VIA_VIF
extern int cm_checkObVifReListUpdate(char *newReMac);
extern int cm_getObVifReByNewReMac(char *newReMac, char *obReMac, int macLen);
extern void cm_updateObVifReList(char *newReMac, char *obReMac, int action);
#endif

#endif /* __CFG_SLAVELIST_H__ */
/* End of cfg_slavelist.h */
