
#ifndef _ark_common_h_
#define _ark_common_h_

#include "uthash.h"

/* parameters define */
#define TMP_ARK                 "/tmp/ark"
#define ARK_DEV_WAN             TMP_ARK"/dev_wan"
#define ARKCTL                  "arkctl"
#define USR_ARK                 "/usr/ark"
#define ARK_SIG                 TMP_ARK"/sig"
#define ARK_JFFS_SIG_D          "/jffs/signature"
#define ARK_JFFS_SIG_F          ARK_JFFS_SIG_D"/ark_rule.zip"
#define ARK_NORMAL_TIMEOUT      12
#define ARK_INI_TMP             TMP_ARK"/ark.ini"
#define ARK_INI_USR             USR_ARK"/ark.ini"
#define ARK_GMQ                 "gmq"
#define ARK_GHS                 "ghs"
#define ARK_GBUSD_SH            "gbusd.sh"
#define ARK_GBUSD               "gbusd"
#define ARK_GLOGER_SH           "gloger.sh"
#define ARK_GLOGER              "gloger"
#define ARK_GDRCTL              "gdrctl"
#define ARK_GDR                 "gdr"
#define ARK_SERVICE             "ark_service"
#define ARK_LOGD                "arklogd"

/* features */
#define ARK_BLOCK_ALL           nvram_get_int("ark_feature_block")  // will disable below features
#define ARK_QOE                 nvram_get_int("ark_qoe_enable")
#define ARK_IAM                 nvram_get_int("ark_iam_enable")
#define ARK_LIMITER             nvram_get_int("ark_limiter_enable")
#define ARK_LOG                 nvram_get_int("ark_log_enable")
#define ARK_HYQOS               nvram_get_int("ark_hyqos_enable")
#define ARK_MALS                nvram_get_int("ark_mals_enable")
#define ARK_ADBLOCK             nvram_get_int("ark_adblock_enable")
#define ARK_TRACKER             nvram_get_int("ark_tracker_enable")

/* function */
#define KMODULE_ARK             "/sys/module/ark/"
#define is_ARK_module()         (d_exists(KMODULE_ARK) ? 1 : 0)
#define is_ARK_pid()            (((pidof(ARK_SERVICE) > 0) && (pidof(ARK_LOGD) > 0)) ? 1 : 0)
#define is_ARK_service()        ((is_ARK_module() && is_ARK_pid()) ? 1 : 0)

/* db */
#define ARK_JFFS_SYS            "/jffs/.sys"
#define ARK_MON_DIR             ARK_JFFS_SYS"/ARKMON"
#define ARK_MON_DB              ARK_MON_DIR"/ArkMon.db"
#define ARK_MON_FLUSH           TMP_ARK"/ark_mon_flush"
#define ARK_MON_TIMEOUT         30
#define ARK_MON_COUNT           "1024"
#define ARK_LOG_DB_SIZE         3072
#define ARK_MON_DB_SIZE         3072
#define ARK_LOG_DIR             ARK_JFFS_SYS"/ARKLOG"
#define ARK_LOG_DB              ARK_LOG_DIR"/ArkLog.db"
#define ARK_GLOGER_LOG          "/tmp/iam.log"

/* eum : ARK MON type ID */
enum {
	ARK_MON_TYPE_NONE         = 0,
	ARK_MON_TYPE_MALS         = 1,
	ARK_MON_TYPE_ADBLOCK      = 2,
	ARK_MON_TYPE_TRACKER      = 3
};

typedef struct {
	int feature_id;
	int mon_type;
} FeatureMap;

static const FeatureMap feature_table[] = {
	{40, ARK_MON_TYPE_MALS},
	{41, ARK_MON_TYPE_MALS},
	{42, ARK_MON_TYPE_MALS},
	{43, ARK_MON_TYPE_MALS},
	{44, ARK_MON_TYPE_MALS},
	{45, ARK_MON_TYPE_MALS},
	{46, ARK_MON_TYPE_MALS},
	{47, ARK_MON_TYPE_MALS},
	{48, ARK_MON_TYPE_MALS},
	{10, ARK_MON_TYPE_ADBLOCK},
	{13, ARK_MON_TYPE_TRACKER},
};

/* -- ARK -- */
/* DEBUG FUNCTION */
#define ARK_DBG_LOG             "/tmp/ARK_LOG"
#define ARK_DBG_LOG_FILE        "/tmp/ARK_LOG.log"
#define ARK_DEBUG               "/tmp/ARK_DEBUG"
#define ARK_DEBUG_JFFS          "/jffs/ARK_DEBUG"
#define ARK_DBG(fmt,args...) \
	if (f_exists(ARK_DEBUG) > 0 || f_exists(ARK_DEBUG_JFFS) > 0) { \
		_dprintf("[ARK][%s:(%d)] "fmt, __FUNCTION__, __LINE__, ##args); \
		if (f_exists(ARK_DBG_LOG) > 0) { \
			FILE *fp = fopen(ARK_DBG_LOG_FILE, "a"); \
			if (fp) { \
				fprintf(fp, "[ARK_LOG][%s:(%d)]" fmt, __FUNCTION__, __LINE__, ##args); \
				fclose(fp); \
			} \
		} \
	}

#define ARK_DB_DEBUG            "/tmp/ARK_DB_DEBUG"
#define ARK_DB_DEBUG_JFFS       "/jffs/ARK_DB_DEBUG"
#define ARK_DB_DBG(fmt,args...) \
	if (f_exists(ARK_DB_DEBUG) > 0 || f_exists(ARK_DB_DEBUG_JFFS) > 0) { \
		_dprintf("[ARK_DB][%s:(%d)] "fmt, __FUNCTION__, __LINE__, ##args); \
		if (f_exists(ARK_DBG_LOG) > 0) { \
			FILE *fp = fopen(ARK_DBG_LOG_FILE, "a"); \
			if (fp) { \
				fprintf(fp, "[ARK_LOG][%s:(%d)]" fmt, __FUNCTION__, __LINE__, ##args); \
				fclose(fp); \
			} \
		} \
	}

/* eum : BLOCKED REASON */
enum {
	ARK_BLOCKED_NONE            = 0, // 0 : normal, not to block
	ARK_BLOCKED_DEFAULT         = 1, // 1 : general block
	ARK_BLOCKED_DUALWAN_LB      = 2  // 2 : not support dualwan load-balance mode
};

enum {
	ARK_UI_DEFAULT              = 0,  // default, not to hidden
	ARK_UI_HIDDEN               = 1,  // ark UI hidden
	ARK_UI_VISIBLE              = 2   // ark UI visible
};

enum {
	ARK_FEATURE_DEFAULT         = 0,  // default, not to block feature
	ARK_FEATURE_BLOCK           = 1,  // block ark no matter enable or disable
	ARK_FEATURE_NONBLOCK        = 2   // not to block feature
};

/* IP / MAC / IP range */
enum {
	ARK_TYPE_UNKNOWN = -1,
	ARK_TYPE_IP = 0,
	ARK_TYPE_MAC,
	ARK_TYPE_IPRANGE,
	ARK_TYPE_MAX
};

/* ark signature structure */
typedef struct ARK_SIG_T ark_sig_t;
struct ARK_SIG_T {
	char *name;
	int type;
};

typedef struct ArkFeature ArkFeature;
typedef void (*Featureinit)(ArkFeature *self);
typedef int (*FeatureOnOff)(ArkFeature *self);
typedef void (*FeatureGenJS)(ArkFeature *self);
typedef void (*FeatureProfile)(ArkFeature *self);

struct ArkFeature {
	int flag;
	char *type;
	Featureinit init;
	FeatureOnOff on_off;
	FeatureGenJS gen_rule;
	FeatureProfile profile;
};

extern ArkFeature qoeFeature;
extern ArkFeature limiterFeature;

/* -- LIMITER -- */
#define ARK_LIMITER_MAX_BW      10*1024*1024 // 10 Gbps
#define ARK_LIMITER_MIN_BW      10           // 10 kbps

/* -- IAM -- */
#define ARK_IAM_JSON_CONF           TMP_ARK"/content_filter.json" // json rule of content filter
#define ARK_IAM_SUPPORT_ID_DEFAULT  "/www/ark-app-list.json"       // default ID support table
#define ARK_IAM_SUPPORT_ID_SIG      "/jffs/ark-app-list.json"      // upgrade ID support table
#define ARK_IAM_IS_CT           0
#define ARK_IAM_IS_APP          1
#define MAX_CATEGORIES          4                                // 4 kinds of category class
#define MAX_CT_ENTRIES          20                               // The maximum number of each category class allowed in a single rule list
#define MAX_APP_ENTRIES         256                              // The maximum number of applications allowed in a single rule list (IAM supports up to 256 rules currently)
#define IAM_CATEGORY_ID_MAX     1000
#define IAM_CATEGORY_ID_MIN     0
#define IAM_APP_ID_MAX          4000
#define IAM_APP_ID_MIN          0
#define IAM_MAX_APP_LIST        4                                // eq. nvram "ark_iam_app_list1", "ark_iam_app_list2", "ark_iam_app_list3" ...
#define IAM_GLOBAL_CT_INDEX     3                                // global category id will be stored into category[3]

typedef struct {
	char device[18];
	int categories[MAX_CATEGORIES][MAX_CT_ENTRIES];          // category ID for each category class, per profile
	int category_count[MAX_CATEGORIES];                      // record the number of category IDs for each category class

	int app[MAX_APP_ENTRIES];                                // app ID per profile
	int app_all_count;                                       // record the number of app IDs for each profile

	UT_hash_handle hh;                                       // uthash handle
} IAM_Profile;

/* Generic struct define for "category" & "app" */
typedef struct {
	int key;
	int index;

	UT_hash_handle hh;
} GenericEntry;

/* global category flag */
enum {
	FLAG_MALS = 1 << 0,
	FLAG_ADBLOCK = 1 << 1,
	FLAG_TRACKER = 1 << 2
};

// Feature name mapping
typedef struct {
	int flag;
	const char *name;
} FeatureCategoryMap;

enum {
	IAM_R_RULE_TYPE_PASS = 0,
	IAM_R_RULE_TYPE_LOG,
	IAM_R_RULE_TYPE_BLOCK
};

enum {
	IAM_RULE_DISABLE = 0,
	IAM_RULE_ENABLE
};

enum {
	IAM_D_RULE_TYPE_ALL_DEVICE = 0,
	IAM_D_RULE_TYPE_BY_DEVICE,
	IAM_D_RULE_TYPE_BY_IP = 3
};

enum {
	IAM_CATEGORY_MODE = 0,
	IAM_DOMAIN_NAME_KEYWORD_MODE,
	IAM_APP_ID_MODE = 4,
	IAM_APP_ID_CATEGORY_MODE = 5
};

enum {
	IAM_C_RULE_SCHEDULE_ONCE = 0,
	IAM_C_RULE_SCHEDULE_DAILY,
	IAM_C_RULE_SCHEDULE_WEEKLY,
	IAM_C_RULE_SCHEDULE_MONTHLY,
	IAM_C_RULE_SCHEDULE_WEEKDAY,
	IAM_C_RULE_SCHEDULE_WEEKEND,
	IAM_C_RULE_SCHEDULE_ALWAYS
};

/* ark_utils.c */
extern int check_ark_setting();
extern int ark_atoi_checked_range(const char *s, int *out, int min, int max);
extern int ark_atoi_checked_value(const char *s, int *out);

/* ark.c */
extern void stop_ark_engine();
extern void start_ark_engine();
extern int ark_check_wan_changed();
extern void wan_start_ark_engine(int wan_unit);
extern void exe_ark_qoe_service();
extern void exe_ark_limiter_service();
extern void exe_ark_iam_service();
extern void check_ark_alive_service();
extern void ark_sig_update_flow();

/* ark_db.c */
extern void exe_ark_protection();
extern void exe_ark_history();

/* ark_hook.c */
extern void ark_cgi_mon_to_json(char *type, char *start, char *end, FILE *stream);
extern void ark_cgi_mon_del_db(char *type, FILE *stream);
extern void ark_cgi_log_to_json(char *start, char *end, char *mac, char *page, char *num, FILE *stream);

#endif /* _ark_common_h_ */
