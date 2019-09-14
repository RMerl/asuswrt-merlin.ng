#ifndef __CFG_ONBOARDING_H__
#define __CFG_ONBOARDING_H__

#define ONBOARDING_FILE_LOCK	"onboarding"
#define ONBOARDING_LIST_JSON_PATH	"/tmp/onboarding.json"
#define ONBOARDING_STATUS_PATH	"/tmp/obstatus"
#define ONBOARDING_VSIE_PATH	"/tmp/obvsie"
#define WPS_CHECK_TIME		5
#define WPS_TIMEOUT		120		//  minutes
#define WPS_WAIT_TIMEOUT		150		// 3 minutes
#define ONBOARDING_CHECK_TIME		5
#define ONBOARDING_DEF_TIMEOUT	300
#define ONBOARDING_AVAILABLE_CHECK_TIME		5
#define ONBOARDING_SELECTION_CHECK_TIME		1
#define ONBOARDING_AVAILABLE_TIMEOUT		120	// 2 minutes
#define ONBOARDING_AVAILABLE_SEL_TIMEOUT	30	//sec
#define UNDEF_RE_MAC	"FF:FF:FF:FF:FF:FF"

enum onboardingType {
	OB_TYPE_OFF = 1,
	OB_TYPE_AVAILABLE,
	OB_TYPE_REQ,
	OB_TYPE_LOCKED
};

enum onboardingStatus {
	OB_STATUS_REQ = 0,
	OB_STATUS_START,
	OB_STATUS_SUCCESS,
	OB_STATUS_WPS_SUCCESS,
	OB_STATUS_WPS_FAIL,
	OB_STATUS_TERMINATE,
	OB_STATUS_AVALIABLE_TIMEOUT,
	OB_STATUS_CANCEL_SELECTION
};

enum onboardingFailResult {
	OB_FAIL_NONE = 0,			/* init result for nothing */
	OB_TIMEOUT_FAIL = 1,		/* fail for onboarding timeout */
	OB_WIFI_TIMEOUT_FAIL,		/* fail for wifi connection timeout */
	OB_WIRED_TIMEOUT_FAIL,	/* fail for wired connection timeout */
	OB_TRAFFIC_TIMEOUT_FAIL,	/* fail for traffic tiemout */
	OB_WPS_UNKNOWN_FAIL,	/* fail for wps unknow */
	OB_WPS_TIMEOUT_FAIL,		/* fail for wps timeout */
	OB_WPS_OVERLAP_FAIL,		/* fail for wps overlapping */
	OB_SELECT_RE_FAIL	/* fail for no RE be selected */
};

enum onboardingStage {
	OB_INIT_STAGE = 0,		/* init stage for nothging */
	OB_REBOOT_STAGE,		/* reboot stage */
	OB_CONNECTION_STAGE,	/* connection stage */
	OB_TRAFFIC_STAGE		/* traffic stage */
};

enum vsieType {
	VSIE_TYPE_STATUS = 1,
	VSIE_TYPE_COST,
	VSIE_TYPE_ID,
	VSIE_TYPE_RE_MAC,
	VSIE_TYPE_MODEL_NAME,
	VSIE_TYPE_RSSI,
	VSIE_TYPE_TIMESTAMP,
	VSIE_TYPE_REBOOT_TIME = 15,
	VSIE_TYPE_CONN_TIMEOUT = 16,
	VSIE_TYPE_TRAFFIC_TIMEOUT = 17
};

extern int cm_isOnboardingAvailable();
extern void cm_processOnboardingEvent(char *inData);
extern void cm_processOnboardingList(char *msg);
extern void cm_stopWps();
extern void cm_stopOnboardingMonitor();
extern void cm_stopOnboardingAvailable();
extern void cm_updateOnboardingListStatus(char *reMac, char *newReMac, int obStatus);
extern int cm_updateOnboardingSuccess(unsigned char *msg);
extern int cm_checkOnboardingNewReValid(unsigned char *msg);
extern void cm_initOnboardingStatus();
extern void cm_updateOnboardingStatus(int obStatus, char *obVsie);
extern int cm_obtainOnboardingStatusFromFile();
extern void cm_updateOnboardingVsie(int obStatus);
extern void cm_updateOnboardingResult(int obResult, char *newReMac);
extern int cm_selectOnboardingPath(char *newReMac);
extern int cm_getOnboardingPath();
extern void cm_setOnboardingPath(int obPath);
extern void cm_processEthOnboardingStatus(unsigned char *data);
extern void cm_updateOnboardingFailResult(int result);
extern void cm_updateOnboardingStage(int stage);

#endif /* __CFG_ONBOARDING_H__ */
/* End of cfg_onboarding.h */
