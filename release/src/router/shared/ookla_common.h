#ifndef _ookla_common_h_
#define _ookla_common_h_

/* definition */
#if defined(RTCONFIG_OOKLA_LITE)
#define OOKLA_FOLDER               "/tmp/OOKLA/"
#else
#define OOKLA_FOLDER               "/jffs/.sys/OOKLA/"
#endif
#define OOKLA_RESULT               OOKLA_FOLDER"ookla_result"
#define OOKLA_HISTORY              OOKLA_FOLDER"ookla_history"
#define OOKLA_HISTORY_T            OOKLA_FOLDER"ookla_history_t"
#define OOKLA_SERVERS              OOKLA_FOLDER"ookla_servers"
#define OOKLA_HISTORY_MAX 49
#define OOKLA_SESSION              OOKLA_FOLDER"ookla_session"
#define OOKLA_CONFIG_AXHND_A7      "https://config.speedtest.net/v1/embed/0ptd555qj7mb2ccz/config"
#define OOKLA_CONFIG_MT7986A       "https://config.speedtest.net/v1/embed/mqobpml236f7ufwv/config"
#define OOKLA_CONFIG_IPQ5322_64    "https://config.speedtest.net/v1/embed/pm779fkk686583sy/config"
#define OOKLA_CONFIG_IPQ5322_32    "https://config.speedtest.net/v1/embed/stkgib00tsvf03xy/config"
#define OOKLA_CONFIG_QCN550X       "https://config.speedtest.net/v1/embed/57h0m54vxv47vmj1/config"
#define OOKLA_CONFIG_MT7621A       "https://config.speedtest.net/v1/embed/kyffrencfr442ko8/config"
#define OOKLA_CONFIG_WIFI8_4916    "https://config.speedtest.net/v1/embed/zc58zg57fp5fek24/config"
#define OOKLA_CONFIG_ADDITIONAL    "https://config.speedtest.net/v1/embed/myrcev4cdcgx4o6e/config"
#define OOKLA_CONFIG_DEFAULT       "https://config.speedtest.net/v1/embed/4tiouex6gsnzq55o/config"

#if defined(RTCONFIG_ETH_MONITOR)
#define ETH_MONITOR_RESULT         OOKLA_FOLDER"ookla_data"
#endif

/* ookla_state */
enum{
		OOKLA_STATE_IDLE = 0,
		OOKLA_STATE_RUN,
		OOKLA_STATE_ERR_DISCON,
		OOKLA_STATE_ERR_TIMEOUT,
		OOKLA_STATE_ERR_TERMINATE,
		OOKLA_STATE_ERR_UNKNOWN
};

/* ookla command format */
struct ookla_cmd_profile {
	char *config;
	int use_broadcom_tcp_discard;
	int is_eos;
};

#define OOKLA_QUOTA_PHASE1_MAX   100   /* Phase 1 free-use limit (phase1_cnt) */
#define OOKLA_QUOTA_DAILY_MAX    3     /* Phase 2: max uses per 24-hour counting period */
#define OOKLA_QUOTA_WINDOW_SEC   86400 /* 24-hour counting period in seconds */

extern struct ookla_cmd_profile ookla_get_cmd_profile(void);

/* debug */
#define OOKLA_DEBUG       "/tmp/OOKLA_DEBUG"
#define OOKLA_DBG(fmt,args...) \
	if(f_exists(OOKLA_DEBUG) > 0) { \
		dbg("[OOKLA][%s:(%d)]"fmt, __FUNCTION__, __LINE__, ##args); \
	}

// shared function
extern int ookla_eos_support();

#endif /* _ookla_common_h_ */
