#ifdef RTCONFIG_LIBASUSLOG
#include <libasuslog.h>
#endif

#define RAST_COUNT_RSSI_SENSITIVE	2
#define RAST_PERIOD_IDLE_SENSITIVE 	10
#define RAST_DFT_IDLE_RATE_SENSITIVE 	100	/* Kbps */

#define RAST_COUNT_RSSI_NORMAL		3
#define RAST_PERIOD_IDLE_NORMAL 	15
#define RAST_DFT_IDLE_RATE_NORMAL 	20	/* Kbps */

#define RAST_COUNT_RSSI_LAZY		6
#define RAST_PERIOD_IDLE_LAZY		20
#define RAST_DFT_IDLE_RATE_LAZY		10	/* Kbps */

#ifdef RTCONFIG_ADV_RAST
#define RAST_POLL_INTV_DETECT 1
#define RAST_EVENT_TIMEOUT 3		/* timeout of cfgsyn daemon to deal STAMON event */
#define RAST_EVENT_INTERVAL_MAX 5	/* maximum additional time for next event trigger */
#define RAST_EVENT_FREEZE 10		/* event of specific will be freezed once event is triggered over this number */
#define RAST_OBVS_RSSI_DELTA 3		/* condition of rssi for obvious moving */
#define RAST_DFT_WEAK_RSSI_DIFF 10	/* rssi delta allow to roam the station which stamon result is not better than trigger criteria */
#define RAST_DFT_RSSI_VIDEO_CALL -80	/* rssi thresold to change idle rate weighting scheme */
#define WL_NBAND_2G 2
#define WL_NBAND_5G 1
#endif

#define RAST_SUPPORT_K_PASSIVE_SCAN	0x1
#define RAST_SUPPORT_V	0x2

#define RAST_POLL_INTV_NORMAL 5
#if defined(RTCONFIG_RALINK)  /* Remove dead STA from assoclist */
#define RAST_TIMEOUT_STA 10
#define RAST_MTK_DATARATE 128 /* 1024=1k, 8bit=1Byte */
#else
#define RAST_TIMEOUT_STA 10
#endif
#define	MAX_IF_NUM 3
#define MAX_SUBIF_NUM 4
#define MAX_STA_COUNT 128
#define ETHER_ADDR_STR_LEN 18
#define MACF_UP	"%02X:%02X:%02X:%02X:%02X:%02X"
#define MACF	"%02x:%02x:%02x:%02x:%02x:%02x"
#if defined(RTCONFIG_RALINK) || defined(RTCONFIG_QCA) || defined(RTCONFIG_LANTIQ)
#define ETHERP_TO_MACF(ea)	((struct ether_addr *) (ea))->ether_addr_octet[0], \
				((struct ether_addr *) (ea))->ether_addr_octet[1], \
				((struct ether_addr *) (ea))->ether_addr_octet[2], \
				((struct ether_addr *) (ea))->ether_addr_octet[3], \
				((struct ether_addr *) (ea))->ether_addr_octet[4], \
				((struct ether_addr *) (ea))->ether_addr_octet[5]
#define ETHER_TO_MACF(ea)	(ea).ether_addr_octet[0], \
				(ea).ether_addr_octet[1], \
				(ea).ether_addr_octet[2], \
				(ea).ether_addr_octet[3], \
				(ea).ether_addr_octet[4], \
				(ea).ether_addr_octet[5]
#else
#define ETHERP_TO_MACF(ea)	((struct ether_addr *) (ea))->octet[0], \
				((struct ether_addr *) (ea))->octet[1], \
				((struct ether_addr *) (ea))->octet[2], \
				((struct ether_addr *) (ea))->octet[3], \
				((struct ether_addr *) (ea))->octet[4], \
				((struct ether_addr *) (ea))->octet[5]
#define ETHER_TO_MACF(ea)	(ea).octet[0], \
				(ea).octet[1], \
				(ea).octet[2], \
				(ea).octet[3], \
				(ea).octet[4], \
				(ea).octet[5]
#endif

#define RAST_DEBUG "/tmp/RAST_DEBUG"

#ifdef RTCONFIG_LIBASUSLOG
#define AMAS_DBG_LOG	"roamast.log"
#define RAST_INFO(fmt, arg...) \
	do {    \
		_dprintf("RAST %lu: "fmt, uptime(), ##arg); \
		if(rast_syslog || f_exists(RAST_DEBUG)) \
			asusdebuglog(LOG_INFO, AMAS_DBG_LOG, LOG_CUSTOM, LOG_SHOWTIME, 0, fmt, ##arg); \
		if(rast_force_syslog) \
 			logmessage("roamast", ""fmt, ##arg); \
	} while (0)
#define RAST_DBG(fmt, arg...) \
	do {    \
		if(rast_dbg || f_exists(RAST_DEBUG)) \
		_dprintf("RAST %lu: "fmt, uptime(), ##arg); \
		if(rast_syslog || f_exists(RAST_DEBUG)) \
			asusdebuglog(LOG_INFO, AMAS_DBG_LOG, LOG_CUSTOM, LOG_SHOWTIME, 0, fmt, ##arg); \
	    	if(rast_force_syslog) \
			logmessage("roamast", ""fmt, ##arg); \
	} while (0)
#define RAST_SYSLOG(fmt, arg...) \
        do {    \
                _dprintf("RAST %lu: "fmt, uptime(), ##arg); \
                logmessage("roamast", ""fmt, ##arg); \
		if(rast_syslog || f_exists(RAST_DEBUG)) \
			asusdebuglog(LOG_INFO, AMAS_DBG_LOG, LOG_CUSTOM, LOG_SHOWTIME, 0, fmt, ##arg); \
		if(rast_force_syslog) \
 			logmessage("roamast", ""fmt, ##arg); \
        } while (0)
#else
#define RAST_INFO(fmt, arg...) \
	do {    \
		_dprintf("RAST %lu: "fmt, uptime(), ##arg); \
		if(rast_syslog || f_exists(RAST_DEBUG)) \
		logmessage("RAST","[%s] "fmt, nvram_get("lan_hwaddr"), ##arg); \
	} while (0)
#define RAST_DBG(fmt, arg...) \
	do {    \
		if(rast_dbg || f_exists(RAST_DEBUG)) \
		_dprintf("RAST %lu: "fmt, uptime(), ##arg); \
		if(rast_syslog || f_exists(RAST_DEBUG)) \
		logmessage("RAST", "[%s] "fmt, nvram_get("lan_hwaddr"), ##arg); \
	} while (0)
#define RAST_SYSLOG(fmt, arg...) \
        do {    \
                _dprintf("RAST %lu: "fmt, uptime(), ##arg); \
                logmessage("roamast", ""fmt, ##arg); \
        } while (0)
#endif

#ifdef RTCONFIG_CONNDIAG
#include <sys/ipc.h>
#include <sys/shm.h>

#define TG_ROAMING_LOCK		"tg_roaming"
#define ROAMING_LOCK		"roaming"

#undef MAX_STA_COUNT
#define MAX_STA_COUNT 128
#undef MAC_LEN
#define MAC_LEN 18

#define KEY_TG_ROAMING_EVENT 34951
#define KEY_ROAMING_EVENT 34952

typedef struct _TG_ROAMING_TABLE {
	time_t tstamp[MAX_STA_COUNT];
	int user_low_rssi[MAX_STA_COUNT];
	int rssi_cnt[MAX_STA_COUNT];
	int idle_period[MAX_STA_COUNT];
	unsigned char sta[MAX_STA_COUNT][MAC_LEN];
	int sta_rssi[MAX_STA_COUNT];
	int idle_start[MAX_STA_COUNT];
	int total;
} TG_ROAMING_TABLE, *P_TG_ROAMING_TABLE;

typedef struct _ROAMING_TABLE {
	time_t tstamp[MAX_STA_COUNT];
	unsigned char sta[MAX_STA_COUNT][MAC_LEN];
	int sta_rssi[MAX_STA_COUNT];
	int candidate_rssi_criteria[MAX_STA_COUNT];
	unsigned char candidate[MAX_STA_COUNT][MAC_LEN];
	int candidate_rssi[MAX_STA_COUNT];
	int total;
} ROAMING_TABLE, *P_ROAMING_TABLE;
#endif

#if defined(RTCONFIG_RALINK)
#define xR_MAX  4
extern int xTxR;
#elif defined(RTCONFIG_QCA)
#endif

typedef struct rast_sta_info {
	time_t timestamp;
	time_t active;
	struct ether_addr addr;
	struct rast_sta_info *prev, *next;
	float datarate;	/* Kbps */
	time_t idle_start;
	uint8 idle_state;
	int32 rssi_hit_count;
	int32 rssi;
#if defined(RTCONFIG_RALINK)
	int32 rssi_xR[xR_MAX];
	int32 last_txrx_bytes;  // Kbps
	char mac_addr[18];
#elif defined(RTCONFIG_QCA)
	int32 last_txrx_bytes; /* bytes */
#elif defined(RTCONFIG_REALTEK)
	unsigned long long last_txrx_bytes;
#else //BRCM
#ifndef RTCONFIG_BCMARM
	uint32 prepkts;
#endif
	uint32 rx_tot_bytes;
	uint32 rx_bytes;
#endif

#ifdef RTCONFIG_ADV_RAST
	time_t trigger;			/* timestamp of STAMON event trigger */
	int next_trigger_interval;	/* interval of next STAMON event trigger */
	int stamon_event_count;		/* counter of STAMON event trigger */
	int32 previous_rssi;		/* save previous rssi for detecting sticky sta */
	uint32 wnm_cap;			/* WNM capability */
#ifdef RTCONFIG_BCN_RPT
	uint8 rrm_bcn_passive_cap;	/* RRM Beacon Passive Measurement capability */
#endif
#endif
#if defined(RTCONFIG_LANTIQ)
	unsigned long last_txrx_bytes;
#endif
	int32 tx_rate;
	int32 rx_rate;
}rast_sta_info_t;


#ifdef RTCONFIG_ADV_RAST
typedef struct rast_maclist {
        time_t timestamp;
	uint8 mesh_node;
        struct ether_addr addr;
        struct rast_maclist *next;
}rast_maclist_t;
#endif

typedef struct rast_bss_info {
	char wlif_name[32];
	char prefix[32];
	int band;
	int user_low_rssi;
	int32 rssi_cnt;
	int32 idle_period;
	int32 idle_rate;
	int sens_level;
	int bss_enable[MAX_SUBIF_NUM];
	rast_sta_info_t *assoclist[MAX_SUBIF_NUM];
	int upstream_if;
#ifdef RTCONFIG_ADV_RAST
	int rast_mode;
	rast_maclist_t *maclist[MAX_SUBIF_NUM];
	struct maclist *static_maclist[MAX_SUBIF_NUM];
	int static_macmode[MAX_SUBIF_NUM];
	int static_cli_enable;
	rast_maclist_t *static_client;
	char tmp_static_client_path[32];
#endif
}rast_bss_info_t;

#ifdef RTCONFIG_ADV_RAST
typedef enum {
	RAST_MODE_RSSI =0,
	RAST_MODE_LEGACY
} rast_mode_t;

typedef struct rast_adv_conf {
	uint32 aclist_timeout;
	uint8 weak_rssi_diff;
} rast_adv_conf_t;
#endif

rast_bss_info_t bssinfo[MAX_IF_NUM];
int rast_dbg;
int rast_syslog;
int rast_force_syslog;

#ifdef RTCONFIG_ADV_RAST
rast_adv_conf_t adv_conf;
extern int alarm_count;
char maclist_buf[4096];
#endif

#if defined(RTCONFIG_RALINK)

#if defined(RTCONFIG_RALINK_MT7621)
#ifndef __USE_GNU
#define __USE_GNU
#endif
#include <sched.h>

#ifdef __USE_GNU
/* Access macros for `cpu_set'.  */
#define CPU_SETSIZE __CPU_SETSIZE
#define CPU_SET(cpu, cpusetp)   __CPU_SET (cpu, cpusetp)
#define CPU_CLR(cpu, cpusetp)   __CPU_CLR (cpu, cpusetp)
#define CPU_ISSET(cpu, cpusetp) __CPU_ISSET (cpu, cpusetp)
#define CPU_ZERO(cpusetp)       __CPU_ZERO (cpusetp)


/* Set the CPU affinity for a task */
extern int sched_setaffinity (__pid_t __pid, size_t __cpusetsize,
                              __const cpu_set_t *__cpuset) __THROW;

/* Get the CPU affinity for a task */
extern int sched_getaffinity (__pid_t __pid, size_t __cpusetsize,
                              cpu_set_t *__cpuset) __THROW;
#endif 
#endif

extern int Set_RAST_CPU(void);

/* For ioctls that take a list of MAC addresses from src-rt/include/wlioctl.h (BCM platform's header) */

struct maclist {
        uint count;                     /* number of MAC addresses */
        struct ether_addr ea[1];        /* variable length array of MAC addresses */
};
#endif

extern char *strcat_safe(const char *s1, const char *s2);
extern void rast_init_bssinfo(void);
extern rast_sta_info_t *rast_add_to_assoclist(int bssidx, int vifidx, struct ether_addr *addr);

extern void get_wifi_ifname(char *wlif_name, int len, int bssidx, int vifidx);
extern void get_stainfo(int bssidx, int vifidx);
extern int rast_stamon_get_rssi(int bssidx, struct ether_addr *addr);
extern void rast_retrieve_static_maclist(int bssidx, int vifidx);
extern void rast_set_maclist(int bssidx, int vifidx);

#if defined(CONFIG_BCMWL5)
extern sta_info_t *wl_sta_info(char *ifname, struct ether_addr *ea);
extern int rast_send_bsstrans_req(int bssidx, int vifidx, struct ether_addr *sta_addr, struct ether_addr *nbr_bssid);
#endif

#if defined(RTCONFIG_BCMARM) || defined(RTCONFIG_BCMWL6)
extern void rast_retrieve_bs_data(int bssidx, int vifidx, int interval);
#endif

#ifndef CONFIG_BCMWL5
/* MAC list modes from src-rt/include/wlioctl.h (BCM platform's header) */
#define WLC_MACMODE_DISABLED    0       /* MAC list disabled */
#define WLC_MACMODE_ALLOW       1      /* Allow specified (i.e. deny unspecified) */
#define WLC_MACMODE_DENY        2      /* Deny specified (i.e. allow unspecified) */
#endif

#ifdef RTCONFIG_RAST_NONMESH_KVONLY
struct roaming_list_entry{
	int idx;
	int vidx;
	struct ether_addr sta;
	int rssi;
	time_t trigger_time;
	struct roaming_list_entry *next;
};
struct report_list_entry {
	struct ether_addr sta;
	struct ether_addr bssid;
	int8 rcpi;
	time_t recv_time;
	struct report_list_entry *next;
};
int add_to_roaming_list(int idx,int vidx ,struct ether_addr *sta,int rssi);
int remove_from_roaming_list(int idx,int vidx ,struct ether_addr *sta);

#endif //RTCONFIG_RAST_NONMESH_KVONLY
