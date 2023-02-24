#ifndef __CFG_OPTIMIZATION_H__
#define __CFG_OPTIMIZATION_H__

#define OPT_SITE_SURVEY_TIME	10	/* seconds */
#define OPT_SITE_SURVEY_TIMES	5	/* times */
#define OPT_SITE_SURVEY_BASE_TIME	30	/* seconds */
#define OPT_CONNECT_TIME	60	/* seconds */
#define OPT_RECONNECT_TIME	60	/* seconds */
#define OPT_SITE_SURVEY_NOTIFY_TIMES	3	/* times */
#define OPT_CONNECT_NOTIFY_TIMES	3	/* times */
#define RSSI_CUMULATIVE_MAX_TIMES	5	/* times */
#define RSSI_INFO_FILE_PATH		TEMP_CFG_MNT_PATH"/rssi_info"
#define ADS_PAIR_FILE_PATH		TEMP_CFG_MNT_PATH"/ads_pair"
#define OPT_RSSI_DIFF_MINUS		-12	/* dBm */
#define OPT_RSSI_DIFF_PLUS		12	/* dBm */

enum optStage {
	OPT_STAGE_NONE = 0,	/* none */
	OPT_STAGE_INIT = 1,	/* init */
	OPT_STAGE_NOTIFY_SITE_SURVEY = 2,	/* notify site survery */
	OPT_STAGE_COLLECT_DATA = 3,	/* collect date */
	OPT_STAGE_NOTIFY_CONNECT = 4,	/* notify connect */
	OPT_STAGE_ADS_FIND_PAIR = 5,	/* find ads pair */
	OPT_STAGE_ADS_NOTIFY_IPERF = 6,	/* notify iperf action */
	OPT_STAGE_ADS_NOTIFY_MEASURE = 7,	/* notify ds measure */
	OPT_STAGE_ADS_WAIT_MEASURE_RESULT = 8,	/* wait the result of ds measure */
	OPT_STAGE_ADS_NOTIFY_SWITCH = 9,	/* notify ds switch */
	OPT_STAGE_DONE = 10,	/* finish optimization */
	OPT_STAGE_MAX
};

enum optStatus {
	OPT_NONE = 0,
	OPT_SITE_SURVEY_START,
	OPT_SITE_SURVEY_DONE,
	OPT_CONNECT_START,
	OPT_CONNECT_DONE,
	OPT_MAX
};

enum connectNotify {
	CONNECT_NOTIFY_NONE = 0,
	CONNECT_NOTIFY_AGAIN,
	CONNECT_NOTIFY_END
};

enum optTrigger {
	OPT_TRIGGER_NONE = 0,
	OPT_TRIGGER_JOIN = 1,
	OPT_TRIGGER_UI = 2,
	OPT_TRIGGER_PERIODIC_TIME = 3,
	OPT_TRIGGER_ADS_FIXED_TIME = 4,
	OPT_TRIGGER_NOTIFY = 5,
	OPT_TRIGGER_5G_BACKHAUL_SWITCH = 6,
	OPT_TRIGGER_5G_RSSI_DIFF_12DBM = 7,
	OPT_TRIGER_MAX
};

struct optArgStruct {
	int newUpdate;
	int optTrigger;
	char mac[18];
}optArgs;

extern json_object *cm_getOptConnectionInfo(CM_CLIENT_TABLE *cfgTbl, json_object *ssrTbl, json_object *eciTbl);
extern int cm_isAllReSupportCentralOpt(CM_CLIENT_TABLE *cfgTbl);
extern void cm_updateOptFollowRule(CM_CLIENT_TABLE *cfgTbl, char *mac);
extern int cm_getIndexByBandUse(char *mac, int band);
extern int cm_updateRssiInfoByBssid(char *mac, char *bssid, char *bandIndex, int rssi);
extern int cm_computeAverageRssiByBssid(char *mac, char *bssid, char *bandIndex);
extern json_object *cm_getEthConnInfo(CM_CLIENT_TABLE *cfgTbl);

#endif /* __CFG_OPTIMIZATION_H__ */
/* End of cfg_optimization.h */