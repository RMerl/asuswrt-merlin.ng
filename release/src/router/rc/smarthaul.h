#ifndef _SMARTHAUL_H_
#define _SMARTHAUL_H_

#define MAX_MLO_LINKS   4
#define SH_OK           0       /* Success */
#define SH_ERROR        -1      /* Error generic */
#define SH_NOMEM	    -27	    /* No Memory */

#include <libasuslog.h>
/* define debug log */
#define SH_DBG_LOG	"smarthaul.log"
#define SMARTHAUL_DEBUG_ERROR	0x0001
#define SMARTHAUL_DEBUG_WARNING	0x0002
#define SMARTHAUL_DEBUG_INFO	0x0004
#define SMARTHAUL_DEBUG_DETAIL	0x0008
#define SMARTHAUL_DEBUG_TRACE	0x0010
#define SMARTHAUL_DEBUG_PRINT	0x0020
#define SMARTHAUL_DEFAULT_LEVEL (SMARTHAUL_DEBUG_PRINT | SMARTHAUL_DEBUG_ERROR)
int smarthaul_debug_level = SMARTHAUL_DEFAULT_LEVEL;

#define SMARTHAUL_ERROR(fmt, arg...) \
	do { if (smarthaul_debug_level & SMARTHAUL_DEBUG_ERROR) \
		_dprintf("SMARTHAUL >>%s(%d): "fmt, __FUNCTION__, __LINE__, ##arg); } while (0)

#define SMARTHAUL_WARNING(fmt, arg...) \
	do { if (smarthaul_debug_level & SMARTHAUL_DEBUG_WARNING) \
		_dprintf("SMARTHAUL >>%s(%d): "fmt, __FUNCTION__, __LINE__, ##arg); } while (0)

#define SMARTHAUL_INFO(fmt, arg...) \
	do { if (smarthaul_debug_level & SMARTHAUL_DEBUG_INFO) \
		_dprintf("SMARTHAUL >>%s(%d): "fmt, __FUNCTION__, __LINE__, ##arg); } while (0)

#define SMARTHAUL_DEBUG(fmt, arg...) \
	do { if (smarthaul_debug_level & SMARTHAUL_DEBUG_DETAIL) \
		_dprintf("SMARTHAUL >>%s(%d): "fmt, __FUNCTION__, __LINE__, ##arg); } while (0)

#define SMARTHAUL_TRACE(fmt, arg...) \
	do { if (smarthaul_debug_level & SMARTHAUL_DEBUG_TRACE) \
		_dprintf("SMARTHAUL >>%s(%d): "fmt, __FUNCTION__, __LINE__, ##arg); } while (0)

#define SMARTHAUL_PRINT(fmt, arg...) \
	do { if (smarthaul_debug_level & SMARTHAUL_DEBUG_PRINT) \
		_dprintf("SMARTHAUL >>%s(%d): "fmt, __FUNCTION__, __LINE__, ##arg); } while (0)

#define SMARTHAUL_SYSLOG(fmt, arg...) \
    do { \
        logmessage("smarthaul: ", fmt, ##arg); \
		asusdebuglog(LOG_INFO, SH_DBG_LOG, LOG_CUSTOM, LOG_SHOWTIME, 0, fmt, ##arg); \
        _dprintf("SMARTHUAL >>%s(%d): "fmt, __FUNCTION__, __LINE__, ##arg); } while (0)

#define SMARTHAUL_DBGLOG(fmt, arg...) \
    do { \
		asusdebuglog(LOG_INFO, SH_DBG_LOG, LOG_CUSTOM, LOG_SHOWTIME, 0, fmt, ##arg); } while (0)

#define ASSERT(exp) do { \
		if (!(exp) && (smarthaul_debug_level & SMARTHAUL_DEBUG_INFO)) { \
			_dprintf("QOSMGMT >>%s(%d): assert(FALSE)\n", __FUNCTION__, __LINE__); \
			int *null = NULL; \
			*null = 0; \
		} \
	} while (0)

/* define ipc related parameters */
#define SH_IPC_SOCKET_PATH "/var/run/smarthaul_ipc_socket" 
#define SH_IPC_MAX_CONNECTION 128
#define EID_SH_STA_ADD 1
#define EID_SH_STA_UPD 2
#define EID_SH_STA_DEL 3
#define CFGMNT_IPC_SOCKET_PATH	"/var/run/cfgmnt_ipc_socket"

/* ------------------------------------------------------------ sh_utils.c ------------------------------------------------------------ */

typedef struct sh_tidmap {
	uint32 sh_idx;    // smarthaul interface index
	uint8 tids[MAX_MLO_LINKS];	// ordered by band, 6g, 5g, 2g, DC
} sh_tidmap_t;

typedef struct sh_mlo_link_info {
	struct ether_addr pap_bssid;
	uint8 wlc_unit;
	uint8 link_id;
	uint8 bandidx;  /* 2 5 6 */
} sh_mlo_link_info_t;

typedef struct sh_mlo_info {
	struct ether_addr self_mld_addr;
	uint8 mlo_num_links;
	sh_mlo_link_info_t mlo_link_info[MAX_MLO_LINKS];
} sh_mlo_info_t;

typedef struct sh_dev_info {
	struct ether_addr self_mac;
	sh_mlo_info_t mlo_info;
	/* BRCM: TBD: all it's BH AP related info */
} sh_dev_info_t;

typedef struct sh_repeater_node {
	struct sh_repeater_node *next;
	struct ether_addr self_mac;
	sh_mlo_info_t mlo_info;
	uint32 sh_idx;
} sh_repeater_node_t;

typedef struct sh_root_info {
	sh_repeater_node_t *rpt_list;
	int stats_interval;
	int num_repeaters;
	sh_dev_info_t local_dev;
} sh_root_info_t;

/* ------------------------------------------------------------ qm_main.c ------------------------------------------------------------ */

typedef struct smarthaul {
	int timeout;			/* default 5 sec */
	int maxretry;			/* default 3 retry */
	int num_entries;		/* total number of entries */
	uint16 pktid;
	uint8 policy_dialog_token;
	//actframe_entry_t *head;
	int eapd_fd;			/* to receive eapd events */
	int mesh_fd;			/* to receive HLE mesh messages */
	int mesh_ipc_fd;		/* to receive IPC mesh messages */
	int appevent_fd;		/* to receive APPEVENT messages */
	int cli_fd;			/* to receive cli message */
	//bcm_usched_handle *usched_hdl;  /* Handle to Micro Scheduler Module */
	uint multiap_mode;
	bool enable_smart_haul;
	void *dev_info;
	//smarthaul_iface_info_t iface_info_list[DEV_NUMIFS * WL_MAXBSSCFG];
} smarthaul_t;

/* ------------------------------------------------------------ sh_algorithm.c ------------------------------------------------------------ */

typedef int (*sh_ls_prep_func_t)(struct sh_mbh *mbh);
/* the benefit score if adding this specific link to the linkset */
typedef int (*sh_ls_expand_score_func_t)(struct sh_mbh *mbh, uint8 link_id);
/* the benefit score if removing this specific link from the linkset */
typedef int (*sh_ls_reduce_score_func_t)(struct sh_mbh *mbh, uint8 link_id);

/* mlo smarthaul link selection algorithms */
struct sh_ls_algo {
	int type;
	sh_ls_prep_func_t prep;
	sh_ls_expand_score_func_t expand_score;
	sh_ls_reduce_score_func_t reduce_score;
} sh_ls_algo_t;

//typedef int (*sh_tidmap_set_cb_t)(void *arg, sh_tidmap_t *tm);

/* internal representation of the stats */
typedef struct sh_link_stats {
	/* -- from chanim_stats */
	uint8 txop;
	uint8 obss;
	uint8 tx;
	uint8 inbss;
	uint8 nocat;
	uint8 nopkt;

	/* -- from txrx_summary */
	uint32 phy_rate; /* units in mbps */
	uint32 data_rate;
	uint32 airtime; /* airtime percentage */

	/* -- from mlo scb_stats */
	uint32 ntx;
	uint32 nrx;
	uint32 last_ntx;
	uint32 last_nrx;
	int rssi;

	/* -- from mlo tidmap */
	uint8 tidmap[MAX_MLO_LINKS];

	/* -- stats update state & time */
	uint flags;     /* valid bits: 0 (chanim), 1 (scb_stats), ... */

	/* from bs data*/
	float retries;
	bool retry_flag;
} sh_link_stats_t;

typedef struct sh_link {
	bool valid;		/* this link element is valid */
	uint8 link_id;
	uint8 wlc_unit;
	uint8 band_ix;		/* 0: 6G, 1: 5G, 2: 2G */
	uint16 chspec;
	struct sh_mbh *mbh; /* back pointer */

	/* dynamic from last stats */
	struct sh_link_stats stats;
} sh_link_t;

/* read from nvram by algo, expose it to utils */
typedef struct sh_algo_info {
	uint8 type;		/* type of algo, 0 means off */
	uint8 txop_hi;		/* thresh get out of congestion */
	uint8 txop_lo;		/* thresh fall into congestion */
	uint8 anchor_band;	/* globally xross all bh interfaces, FF means no */

	uint16 cycle_ms;	/* duration of stats collection cycle, units in ms */
	uint16 min_cycle;	/* min # of cycles has to stay b4 another change */
	uint16 min_cnt;
} sh_algo_info_t;

typedef struct sh_mbh {
	struct sh_mbh *next;	/* next AP backhaul interface */
	struct sh_mbh *bsta;	/* companion backhaul STA */
	struct sh_algo *algo;	/* back pointer */
	const sh_mlo_info_t *mlo_info;

	uint32 ix;	/* index of a BH */
	bool is_bap;
	uint8 flags;
	uint8 link_cnt;
	uint8 stats_cnt;

	uint8 cfg_linkset;	/* linkset user configed */
	uint8 anchor_link;	/* current anchor link */
	uint8 cur_linkset;	/* cur bitmap of links */
	uint8 can_linkset;	/* candidate bitmap of links */
	uint32 change_cycle;	/* cycle # of last linkset change */
	uint32 exp_cnt;
	uint32 red_cnt;

	uint32 stats_cycle;	/* cycle # of last stats update */
	struct sh_link link[MAX_MLO_LINKS];
} sh_mbh_t;

typedef struct sh_algo {
	struct sh_algo_info info;
	void *arg;
	//sh_tidmap_set_cb_t cb;

	uint32 cycle;		/* increase by 1 every cycle */
	uint8 mbh_cnt;		/* num of mlo bh interfaces managed */
	struct sh_mbh *mbh;
	const struct sh_ls_algo *ls_algo;	/* per bh interface algo */
} sh_algo_t;

void sh_ipc_socket_thread(smarthaul_t *smarthaul);
int sh_start_ipc_socket(void* arg);
void sh_ipc_receive(void* arg, int sockfd);
int sh_sta_add(void *arg, char *data);
int sh_sta_upd(void *arg, char *data);
int sh_sta_del(void *arg, char *data);
int sh_process_ipc_data(sh_dev_info_t *new_dev, char *data);
void sh_add_dev2list(smarthaul_t *smarthaul, sh_dev_info_t *new_dev);

int sh_activate_bh(smarthaul_t *smarthaul, sh_repeater_node_t *new_dev, sh_dev_info_t *local_dev);
void sh_deactivate_bh(smarthaul_t *smarthaul, sh_repeater_node_t *rpt_node);
static int sh_mbh_add_link(struct sh_mbh *mbh, int link_id, const sh_mlo_link_info_t *li);
static int sh_bh_add_links(struct sh_mbh *mbh);
int sh_add_bh(uint32 sh_idx, const sh_mlo_info_t *bap, const sh_mlo_info_t *bsta);
static void sh_mbh_del_link(struct sh_mbh *mbh, int link_id);
int sh_del_bh(uint32 sh_idx);
void sh_mbh_free(struct sh_mbh *mbh);

static void sh_mbh_fix_anchor_band(struct sh_mbh *mbh, uint8 band);
static struct sh_mbh *sh_algo_find_mbh_by_ix(int mbh_ix);

int sh_mbh_chanim_stats_upd(uint32 sh_idx);
static void sh_collect_stats(void);

int sh_add_timer(int cycle_ms);
int sh_remove_timer(void);

void dump_sh_rpt_list(sh_root_info_t *root_info);
void dump_sh_dev_info(sh_dev_info_t *dev);
void dump_sh(smarthaul_t *smarthaul);

static void sh_mbh_get_tidmap(struct sh_mbh *mbh, uint8 linkset, sh_tidmap_t *map);
int sh_tidmap_set (sh_tidmap_t *tm);
int sh_reset_tidmap(void);

static void sh_mbh_invalidate_stats(struct sh_mbh *mbh);
static int sh_algo_prep(struct sh_mbh *mbh);
static uint8 sh_algo_expand_linkset(struct sh_mbh *mbh);
static int sh_algo_expand_score(struct sh_mbh *mbh);
static uint8 sh_algo_reduce_linkset(struct sh_mbh *mbh);
static int sh_algo_reduce_score(struct sh_mbh *mbh);
static int sh_ls_prep_1(struct sh_mbh *mbh);
static int sh_ls_expand_score_1(struct sh_mbh *mbh, uint8 link_id);
static int sh_ls_reduce_score_1(struct sh_mbh *mbh, uint8 link_id);
static int sh_ls_prep_2(struct sh_mbh *mbh);
static int sh_ls_expand_score_2(struct sh_mbh *mbh, uint8 link_id);
static int sh_ls_reduce_score_2(struct sh_mbh *mbh, uint8 link_id);

int sh_init(smarthaul_t *smarthaul);
int sh_algo_init(void *arg);
int smarthaul_main(void);
sh_algo_info_t *sh_algo_info_get(void);
#endif	/* _SMARTHAUL_H_ */
