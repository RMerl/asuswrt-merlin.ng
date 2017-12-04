#ifndef __CFG_SLAVELIST_H__
#define __CFG_SLAVELIST_H__

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
typedef struct _CM_CLIENT_TABLE {
	char alias[CFG_CLIENT_NUM][ALIAS_LEN];
	unsigned char ipAddr[CFG_CLIENT_NUM][IP_LEN];
	unsigned char macAddr[CFG_CLIENT_NUM][MAC_LEN];
	unsigned char realMacAddr[CFG_CLIENT_NUM][MAC_LEN];
	time_t reportStartTime[CFG_CLIENT_NUM];
	unsigned char pap2g[CFG_CLIENT_NUM][MAC_LEN];
	unsigned char pap5g[CFG_CLIENT_NUM][MAC_LEN];
	int rssi2g[CFG_CLIENT_NUM];
	int rssi5g[CFG_CLIENT_NUM];
	unsigned char sta2g[CFG_CLIENT_NUM][MAC_LEN];
	unsigned char sta5g[CFG_CLIENT_NUM][MAC_LEN];
	unsigned char ap2g[CFG_CLIENT_NUM][MAC_LEN];
	unsigned char ap5g[CFG_CLIENT_NUM][MAC_LEN];
	unsigned char ap5g1[CFG_CLIENT_NUM][MAC_LEN];
	int level[CFG_CLIENT_NUM];
	char fwVer[CFG_CLIENT_NUM][FWVER_LEN];
	char newFwVer[CFG_CLIENT_NUM][FWVER_LEN];
	char modelName[CFG_CLIENT_NUM][MODEL_NAME_LEN];
	int activePath[CFG_CLIENT_NUM];
	int maxLevel;
	int count;
} CM_CLIENT_TABLE, *P_CM_CLIENT_TABLE;

#endif /* __CFG_SLAVELIST_H__ */
/* End of cfg_slavelist.h */
