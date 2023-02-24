#ifndef __CFG_ADS_H__
#define __CFG_ADS_H__

#define ADS_DS_TEST_TIME	3	/* seconds */
#define ADS_DS_DELAY_TIME	3	/* seconds */
#define ADS_FIRST_DS_DELAY_TIME	10	/* seconds */
#define ADS_DS_WAIT_RESULT_TIME	30	/* seconds */
#define DS_DATA_RATE_FOR_JOIN	70	/* percent */
#define DS_DATA_RATE_FOR_UI	70	/*percent */
#define ADS_IPERF_NOTIFY_TIMES	3	/* times */
#define ADS_MEASURE_NOTIFY_TIMES	3	/* times */
#define ADS_SWITCH_NOTIFY_TIMES	3	/* times */
#define DS_SWITCH_RESULT_FILE	TEMP_CFG_MNT_PATH"/ds_switch_result"
#define ADS_MAX_RSSI_THRESHOLD	-85	/* dBm */
#define ADS_TRIGGER_FIXED_HOUR	3	/* AM 3:00 */
#define DS_UNAVAILABLE_FILE	TEMP_CFG_MNT_PATH"/ds_unavailable"
#define ADS_DS_WAIT_STA_RECONN_TIME	60	/* seconds */
#define ADS_PAIR_DONE_FILE	TEMP_CFG_MNT_PATH"/ads_pair_done"
#define ADS_PAIR_FILE_PATH		TEMP_CFG_MNT_PATH"/ads_pair"
#define ADS_MEASURE_TOLERANCE_TIME	5	/* seconds */

typedef struct ads_pair_s {
	char pMac[18];
	char cMac[18];
	char pModelName[33];
	char cModelName[33];
	int rssi5g;
	int cost;
	struct ads_pair_s *next;
} ads_pair_s;

enum dsSelection {
	DS_SELECTION_NONE = 0,
	DS_SELECTION_BY_RSSI = 1,
	DS_SELECTION_BY_DATA_RATE = 2,
	DS_SELECTION_MAX
};

extern json_object *cm_findAdsPair(unsigned int timestamp, CM_CLIENT_TABLE *p_client_tbl, int optTrigger, char *nMac);
extern int cm_getPairRelatedInfo(CM_CLIENT_TABLE *p_client_tbl, char *pMac, char *cMac, int pCap, int *pUnit5g, int *cUnit5g, int *cTblIndex, char *cStaMac, int cStaMacLen);
extern json_object *cm_createDpsCombination(unsigned int timestamp, char *mac, int adsCap, int bandUnit, int *dpNum, int *dpsCombNum);
extern void cm_updateUnavailableDsToFile(unsigned int timestamp, char *pMac, int pUnit, json_object *pDsObj, char *cMac, int cUnit, json_object *cDsObj);
extern int cm_isUnavailableDs(unsigned int timestamp, char *pMac, int pUnit, json_object *pDsObj, char *cMac, int cUnit, json_object *cDsObj);
extern void cm_updateDsToResult(unsigned int timestamp, char *resultFile, int seq, json_object *pDsObj, json_object *cDsObj);
extern void cm_saveFinalDsSwitch(unsigned int timestamp, char *mac, int bandUnit, json_object *dsObj);
extern void cm_removeFinalDsFromFile(unsigned int timestamp, char *mac, int bandUnit);
extern void cm_updateAdsPairDone(unsigned int timestamp, char *pMac, int pUnit, char *cMac, int cUnit);
extern int cm_isAdsPairDone(unsigned int timestamp, char *pMac, int pUnit, char *cMac, int cUnit);

#endif /* __CFG_ADS_H__ */
/* End of cfg_ads.h */