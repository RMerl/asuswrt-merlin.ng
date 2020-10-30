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
#define PRELINK_FILE_LOCK	"prelink"
#define PRELINK_LIST_JSON_PATH	"/tmp/prelink.json"
#ifdef ONBOARDING_VIA_VIF
#define TIMEOUT_FOR_VIF_CHECK		60	//sec
#define TIMEOUT_CONFIG_SYNC		60	//sec
#endif

enum onboardingType {
	OB_TYPE_OFF = 1,
	OB_TYPE_AVAILABLE,
	OB_TYPE_REQ,
	OB_TYPE_LOCKED,
#ifdef ONBOARDING_VIA_VIF
	OB_TYPE_VIF_CHECK = 5,
#endif
	OB_TYPE_MAX
};

enum onboardingStatus {
	OB_STATUS_REQ = 0,
	OB_STATUS_START,
	OB_STATUS_SUCCESS,
	OB_STATUS_WPS_SUCCESS,
	OB_STATUS_WPS_FAIL,
	OB_STATUS_TERMINATE,
	OB_STATUS_AVALIABLE_TIMEOUT,
	OB_STATUS_CANCEL_SELECTION,
#ifdef ONBOARDING_VIA_VIF
	OB_STATUS_REPORT_VIF_STATUS = 8,
#endif
	OB_STATUS_MAX
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
	OB_SELECT_RE_FAIL,	/* fail for no RE be selected */
#ifdef ONBOARDING_VIA_VIF
	OB_VIF_CHECK_FAIL = 9,		/* fail for vif check */
#endif
	OB_FAIL_MAX
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
	VSIE_TYPE_TRAFFIC_TIMEOUT = 17,
    VSIE_TYPE_AP_LAST_BYTE = 18,
    VSIE_TYPE_CAP_ROLE = 19,
	VSIE_TYPE_BUNDLE_KEY = 20,
	VSIE_TYPE_INF_TYPE = 21,
	VSIE_TYPE_TCODE = 22,
#if defined(RTCONFIG_AMAS_QCA_WDS) && defined(RTCONFIG_BHCOST_OPT)
	VSIE_TYPE_WDS =23
#endif		
};

enum prelinkStage {
	PRELINK_INIT = 0,
	PRELINK_GID_REQUEST,
	PRELINK_GID_RESPONSE,
	PRELINK_GID_ACK,
	PRELINK_JOIN
};

enum obVifAction {
	OB_VIF_DOWN = 0,
	OB_VIF_UP
};


extern int cm_isOnboardingAvailable();
extern void cm_processOnboardingEvent(char *inData);
extern void cm_processOnboardingList(char *msg);
extern void cm_stopWps();
extern void cm_stopOnboardingMonitor();
extern void cm_stopOnboardingAvailable();
extern void cm_updateOnboardingListStatus(char *reMac, char *newReMac, int obStatus);
extern int cm_updateOnboardingSuccess(int keyType, unsigned char *msg);
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
#ifdef PRELINK
extern void cm_updatePrelinkStatus(char *reMac, int status);
#endif
extern void update_vsie_info();
#ifdef ONBOARDING_VIA_VIF
extern int cm_checkOnboardingViaVif(char *mac);
extern int cm_waitOnboardingVifReady(char *mac);
extern void *cm_upOnboardingVif(void *args);
extern void cm_computeVifDownTimeout(int rTime, int cTimeout, int tTimeout);
extern void cm_updateVifUpStatus(int status);
extern int cm_obVifDownUp(int action);
#endif

#endif /* __CFG_ONBOARDING_H__ */
/* End of cfg_onboarding.h */
