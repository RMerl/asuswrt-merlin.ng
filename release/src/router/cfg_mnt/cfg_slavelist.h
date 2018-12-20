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
#define TERRITORY_CODE_LEN	33
#define RE_LIST_JSON_FILE	"/tmp/relist.json"
#define MAX_RELIST_COUNT		10
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
	int rssi2g[CFG_CLIENT_NUM];
	int rssi5g[CFG_CLIENT_NUM];
	unsigned char sta2g[CFG_CLIENT_NUM][MAC_LEN];
	unsigned char sta5g[CFG_CLIENT_NUM][MAC_LEN];
	unsigned char ap2g[CFG_CLIENT_NUM][MAC_LEN];
	unsigned char ap5g[CFG_CLIENT_NUM][MAC_LEN];
	unsigned char ap5g1[CFG_CLIENT_NUM][MAC_LEN];
	unsigned char apDwb[CFG_CLIENT_NUM][MAC_LEN];
	int level[CFG_CLIENT_NUM];
	char fwVer[CFG_CLIENT_NUM][FWVER_LEN];
	char newFwVer[CFG_CLIENT_NUM][FWVER_LEN];
	char modelName[CFG_CLIENT_NUM][MODEL_NAME_LEN];
	char territoryCode[CFG_CLIENT_NUM][TERRITORY_CODE_LEN];
	int activePath[CFG_CLIENT_NUM];
	int maxLevel;
	int count;
} CM_CLIENT_TABLE, *P_CM_CLIENT_TABLE;

extern int cm_checkReListExist(char *Mac);
extern int cm_checkReListUpdate(char *newReMac, char *sta2gMac, char *sta5gMac);
extern void cm_updateReList(char *newReMac, char *sta2gMac, char *sta5gMac, int action);
extern void cm_handleReListUpdate(unsigned char *decodeMsg);
extern int cm_prepareReListMsg(char *msg, int msgLen);
extern void cm_generateReList();
extern void cm_updateReListTimestamp(unsigned char *decodeMsg);
extern int cm_isSlaveOnline(time_t startTime);

#endif /* __CFG_SLAVELIST_H__ */
/* End of cfg_slavelist.h */
