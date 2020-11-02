/*
 * ESCAND server include file
 *
 * Copyright 2020 Broadcom
 *
 * This program is the proprietary software of Broadcom and/or
 * its licensors, and may only be used, duplicated, modified or distributed
 * pursuant to the terms and conditions of a separate, written license
 * agreement executed between you and Broadcom (an "Authorized License").
 * Except as set forth in an Authorized License, Broadcom grants no license
 * (express or implied), right to use, or waiver of any kind with respect to
 * the Software, and Broadcom expressly reserves all rights in and to the
 * Software and all intellectual property rights therein.  IF YOU HAVE NO
 * AUTHORIZED LICENSE, THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY
 * WAY, AND SHOULD IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE ALL USE OF
 * THE SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1. This program, including its structure, sequence and organization,
 * constitutes the valuable trade secrets of Broadcom, and you shall use
 * all reasonable efforts to protect the confidentiality thereof, and to
 * use this information only in connection with your use of Broadcom
 * integrated circuit products.
 *
 * 2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED
 * "AS IS" AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES,
 * REPRESENTATIONS OR WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR
 * OTHERWISE, WITH RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY
 * DISCLAIMS ANY AND ALL IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY,
 * NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE, LACK OF VIRUSES,
 * ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION OR
 * CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING
 * OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL
 * BROADCOM OR ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL,
 * SPECIAL, INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR
 * IN ANY WAY RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN
 * IF BROADCOM HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii)
 * ANY AMOUNT IN EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF
 * OR U.S. $1, WHICHEVER IS GREATER. THESE LIMITATIONS SHALL APPLY
 * NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
 *
 * $Id: escand_svr.h 769837 2018-11-28 06:31:29Z $
 */

#ifndef _escand_srv_h_
#define _escand_srv_h_

#include "escand.h"

#define ESCAND_OK	0
#define ESCAND_FAIL -1
#define ESCAND_ERR_NO_FRM			-2
#define ESCAND_ERR_NOT_ACTVS		-3
#define ESCAND_ERR_NOT_DCSREQ		-4

#define ESCAND_IFNAME_SIZE		16
#define ESCAND_MAX_INTERFACES		3
#define ESCAND_MAX_IF_NUM ESCAND_MAX_INTERFACES
#define ESCAND_EXCLUDE_IFACE_LIST		16

#define ESCAND_DFLT_POLL_INTERVAL 1  /* default polling interval */

#define DCS_CSA_COUNT		20
#define ESCAND_CSA_COUNT		2

#define WL_CHSPEC_DEF_5G_L	0xD024		/* Default 5GL chanspec from Channel 36 */
#define WL_CHSPEC_DEF_5G_H	0xD095		/* Default 5GH chanspec from Channel 149 */
#define WL_RADIO_CHAN_5GL	0x0018
#define WL_RADIO_CHAN_5GH	0x0007
#define BAND_5G(band) (band == WLC_BAND_5G)
#define ESCAND_SM_BUF_LEN  1024
#define ESCAND_CS_MAX_2G_CHAN	CH_MAX_2G_CHANNEL	/* max channel # in 2G band */
#define ESCAND_SRSLT_BUF_LEN (32*1024)
/* escand config flags */
#define ESCAND_FLAGS_INTF_THRES_CCA	0x1
#define ESCAND_FLAGS_INTF_THRES_BGN	0x2
#define ESCAND_FLAGS_NOBIAS_11B		0x4
#define ESCAND_FLAGS_LASTUSED_CHK		0x8  /* check connectivity for cs scan */
#define ESCAND_FLAGS_CI_SCAN		0x10 /* run ci scan constantly */
#define ESCAND_FLAGS_FAST_DCS		0x20 /* fast channel decision based on updated ci */
#define ESCAND_FLAGS_SCAN_TIMER_OFF	0x40 /* do not check scan timer */

#define CI_SCAN(c_info) ((c_info)->flags & ESCAND_FLAGS_CI_SCAN)
#define SCAN_TIMER_ON(c_info) (((c_info)->flags & ESCAND_FLAGS_SCAN_TIMER_OFF) == 0)

#define ESCAND_NOT_ALIGN_WT	2

#define ESCAND_MIN_SCORE NBITVAL(31)
#define ESCAND_BGNOISE_BASE	-95

#define ESCAND_BW_20	0
#define ESCAND_BW_40	1
#define ESCAND_BW_80	2
#define ESCAND_BW_160	3
#define ESCAND_BW_8080	4
#define ESCAND_BW_MAX	5
#define ESCAND_BW_160_OP	6

#define ESCAND_BSS_TYPE_11G	1
#define ESCAND_BSS_TYPE_11B	2
#define ESCAND_BSS_TYPE_11A	3
#define ESCAND_BSS_TYPE_11N	8

#define LOW_POWER_CHANNEL_MAX	64

/* scan parameter */
#define ESCAND_CS_SCAN_DWELL	70 /* ms */
#define ESCAND_CS_HOME_DWELL	5000 /* ms */
#define ESCAND_CI_SCAN_DWELL	70  /* ms */
#define ESCAND_CI_HOME_DWELL	5000 /* ms */
#define ESCAND_CS_SCAN_DWELL_ACTIVE 250 /* ms */
#define ESCAND_CI_SCAN_WINDOW	5   /* sec: how often for ci scan */
#define ESCAND_CS_SCAN_TIMER_MIN	60  /* sec */
#define ESCAND_DFLT_CS_SCAN_TIMER	900  /* sec */
#define ESCAND_DFLT_CI_SCAN_TIMER  5 /* sec */
#define ESCAND_CS_SCAN_MIN_RSSI -80 /* dBm */
#define ESCAND_CI_SCAN_MIN_RSSI -80 /* dBm */
#define ESCAND_CI_SCAN_EXPIRE	300  /* sec: how long to expire an scan entry */
#define ESCAND_SCS_DFS_SCAN_DEFAULT 0 /* creating default value for escand_scs_dfs_scan */

/* scan channel flags */
#define ESCAND_CI_SCAN_CHAN_PREF	0x01  /* chan preferred flag */
#define ESCAND_CI_SCAN_CHAN_EXCL	0x02  /* chan exclude flag */

/* Scan running status */
#define ESCAND_CI_SCAN_RUNNING_PREF	0x01
#define ESCAND_CI_SCAN_RUNNING_NORM	0x02

#define ESCAND_NUMCHANNELS	64

/* mode define */
#define ESCAND_MODE_DISABLE	0
#define ESCAND_MODE_MONITOR	1
#define ESCAND_MODE_LAST	1

#define ESCAND_STATUS_POLL	5
#define ESCAND_ASSOCLIST_POLL	30

#define ESCAND_DYN160_CENTER_CH	50 /* on 160MHz with dyn160 enabled, use this center chanspec */
#define ESCAND_DYN160_CENTER_CH_80	58 /* with dyn160 enabled, use this center chanspec on 80MHz */

#define ESCAND_BW_DWNGRD_ITERATIONS	2 /* No of iterations to check before downgrading BW */

#define ESCAND_MAX_VECTOR_LEN			(ESCAND_NUMCHANNELS * 6) /* hex format */
#define ESCAND_MAX_LIST_LEN			ESCAND_NUMCHANNELS

#define ESCAND_TXDELAY_PERIOD			1
#define ESCAND_TXDELAY_CNT			1
#define ESCAND_START_ON_NONDFS			0
#define ESCAND_TXDELAY_RATIO			30
/* escand_dfs settings: disabled, enabled, reentry */
#define ESCAND_DFS_DISABLED			0
#define ESCAND_DFS_ENABLED			1
#define ESCAND_DFS_REENTRY			2
#define ESCAND_FAR_STA_RSSI			-75
#define ESCAND_NOFCS_LEAST_RSSI			-60
#define ESCAND_CHAN_DWELL_TIME			30
#define ESCAND_TX_IDLE_CNT			10		/* around 3.5Mbps */
#define ESCAND_CI_SCAN_TIMEOUT			15		/* seconds */
#define ESCAND_BOOT_ONLY_DEFAULT		0

#define ESCAND_AP_CFG		1
#define ESCAND_STA_CFG		2
#define ESCAND_MAC_CFG		3
#define ESCAND_INTF_CFG		4
#define ESCAND_BSS_CFG		5
#define ESCAND_AC_BE		0
#define ESCAND_AC_BK		1
#define ESCAND_AC_VI		2
#define ESCAND_AC_VO		3
#define ESCAND_AC_TO		4
#define ESCAND_TRF_AE		0x10
#define ESCAND_IGNORE_TXFAIL		0
#define ESCAND_TRAFFIC_THRESH_ENABLE	0
#define ESCAND_MAX_VIFNAMES		8

#define ESCAND_IGNORE_TXFAIL_ON_FAR_STA		0
/* ESCAND TOA suport for video stas */
#define ESCAND_MAX_VIDEO_STAS			5
#define ESCAND_STA_EA_LEN			18
#define ESCAND_MAX_STA_INFO_BUF			8192

#define INACT_TIME_OFFSET	24 /* bit offset to inactivity time */
#define INACT_TIME_MASK		(0xffu<<INACT_TIME_OFFSET)

#define GET_INACT_TIME(status) ((status >> INACT_TIME_OFFSET) & 0xff)

/* any previous move in progress will be cancelled (both radar scan and move are cancelled) */
#define DFS_AP_MOVE_CANCEL	-1
/* any previous move in progress will be stunted (scan finishes but channel switch is avoided) */
#define DFS_AP_MOVE_STUNT	-2

#define ESCAND_TICK_DISPLAY_INTERVAL		30	/* display ticks every 30 seconds */

/* default values that nvram might override */

#define ESCAND_RECENT_CHANSWITCH_TIME		240	/* Recent channel switch time */

#define ESCAND_11H(CI)	((CI)->rs_info.reg_11h)

#define ESCAND_CHINFO_IS_INACTIVE(CHINFO) (((CHINFO) & WL_CHAN_INACTIVE) != 0 || \
		((CHINFO) & INACT_TIME_MASK) != 0) /* DFS channel marked inactive due to radar */
#define ESCAND_CHINFO_IS_UNCLEAR(CHINFO) (((CHINFO) & WL_CHAN_PASSIVE) != 0) /* not-cleared DFS ch */
#define ESCAND_CHINFO_IS_CLEARED(CHINFO) (((CHINFO) & WL_CHAN_PASSIVE) == 0) /* cleared channel */

#define ESCAN_EVENTS_BUFFER_SIZE 2048
#define WL_CS_SCAN_TIMEOUT 10 /* Timeout in second for CS scan event from driver */
#define WL_CI_SCAN_TIMEOUT 500000 /* Timeout in millisecond for CI scan event from driver */
#define ESCAND_ESCAN_DEFAULT  0 /* Escan disabled by default */

#define ESCAND_WPS_RUNNING	(nvram_match("wps_proc_status", "1") || \
	nvram_match("wps_proc_status", "9"))

#define ESCAND_CAP_STRING_DYN160			"dyn160 "	/* dyn160 in `wl cap` */

#define ESCAND_OP_BW(op) ((op) & 0xf)
#define ESCAND_OP_BW_IS_160_80p80(op)	(ESCAND_OP_BW(op) == ESCAND_BW_160_OP)
#define ESCAND_OP_BW_IS_80(op)		(ESCAND_OP_BW(op) == ESCAND_BW_80)
#define ESCAND_OP_BW_IS_40(op)		(ESCAND_OP_BW(op) == ESCAND_BW_40)
#define ESCAND_OP_BW_IS_20(op)		(ESCAND_OP_BW(op) == ESCAND_BW_20)
#define ESCAND_OP_2NSS_80			0x112
#define ESCAND_OP_2NSS_160			0x116
#define ESCAND_OP_4NSS_80			0x132
/* Other APP can request to change the channel via escand, in that
 * case proper reason will be provided by requesting APP, For ESCAND
 * ESCAND_USE_DEF_METHOD: ESCAND's own default method to set channel
 */
#define ESCAND_USE_DEF_METHOD		-1
#define BAND_2G(band) (band == WLC_BAND_2G)

#define ESCAND_DFS_REENTRY_EN 1

#define ESCAND_LOW_POW_LAST_FCC	144
#define ESCAND_LOW_POW_LAST_ETSI	64
#define ESCAND_IS_LOW_POW_CH(CH, IS_EU) ((IS_EU) ? ((CH) <= ESCAND_LOW_POW_LAST_ETSI) : \
	((CH) <= ESCAND_LOW_POW_LAST_FCC))
#define ESCAND_NON_WIFI_ENABLE	0

typedef struct {
	int min_val;
	int max_val;
} range_t;

typedef struct ifname_idx_map {
	char name[16];
    uint8 idx;
	bool in_use;
} ifname_idx_map_t;

typedef struct {
	uint num_cmds;  /* total incoming cmds from the client */
	uint valid_cmds; /* valid cmds */
	uint num_events; /* total event from the driver */
	uint valid_events; /* valid events */
} escand_stats_t;

struct escand_chaninfo;
typedef chanspec_t (*escand_selector_t)(struct escand_chaninfo *c_info, int bw);

/* a reduced version of wl_bss_info, keep it small, add items as needed */
typedef struct escand_bss_info_sm_s {
	struct ether_addr BSSID;
	uint8 SSID[32];
	uint8 SSID_len;
	chanspec_t chanspec;
	int16 RSSI;
	uint type;
} escand_bss_info_sm_t;

typedef struct escand_bss_info_entry_s {
	escand_bss_info_sm_t binfo_local;
	time_t timestamp;
	struct escand_bss_info_entry_s * next;
} escand_bss_info_entry_t;

typedef struct scan_chspec_elemt_s {
	chanspec_t chspec;
	uint32 chspec_info;
	uint32 flags;
} scan_chspec_elemt_t;

typedef struct escand_scan_chspec_s {
	uint8 count;
	uint8 idx;
	uint8 pref_count;	/* chan count of prefer chan list */
	uint8 excl_count;	/* chan count of exclusive chan list */
	uint8 ci_scan_running;	/* is ci_scan running */
	uint8 cs_scan_running;	/* is ci_scan running */
	bool ci_pref_scan_request; /* need to start ci scan for pref chan? */
	scan_chspec_elemt_t* chspec_list;
} escand_scan_chspec_t;

typedef struct escand_conf_chspec_s {
	uint16 count;
	chanspec_t clist[ESCAND_MAX_LIST_LEN];
} escand_conf_chspec_t;

typedef struct escand_toa_video_sta {
	struct ether_addr ea;
	char vid_sta_mac[ESCAND_STA_EA_LEN];
} escand_toa_video_sta_t;

/* radio setting info */
typedef struct escand_rsi {
	int band_type;
	int bw_cap;
	bool coex_enb;
	bool reg_11h;
	chanspec_t pref_chspec;
} escand_rsi_t;

#define ESCAND_STA_NONE            0               /* no assoc STA */
#define ESCAND_STA_EXIST_FAR       (1 << 0) /* existing FAR STA (low rssi) */
#define ESCAND_STA_EXIST_CLOSE     (1 << 1) /* exist close STA (high rssi) */

typedef struct escand_sta_info {
	struct ether_addr ea;
	int32 rssi;
} escand_sta_info_t;

typedef struct escand_assoclist {
	int count;
	escand_sta_info_t sta_info[1];
} escand_assoclist_t;

/* handle the wrap around while caclulating delta */
#define DELTA_FRAMES(t0, t1)    ((t1) >= (t0) ? ((t1) - (t0)) : (UINT32_MAX - (t0) + (t1)))

/* traffic information; tx & rx bytes and packets */
typedef struct escand_traffic_info {
	time_t timestamp;
	uint64 txbyte;
	uint64 rxbyte;
	uint32 txframe;
	uint32 rxframe;
} escand_traffic_info_t;

typedef struct escand_activity_info {
	escand_traffic_info_t prev_bss_traffic;		/* previous traffic info of the BSS */
	escand_traffic_info_t accu_diff_bss_traffic;	/* delta traffic info of the BSS */
	escand_traffic_info_t prev_diff_bss_traffic;	/* delta traffic info of the BSS */
	int num_accumulated;				/* number of diffs accumulated */
} escand_activity_info_t;

struct escan_bss {
	struct escan_bss *next;
	wl_bss_info_t bss[1];
};

/* escand escan structure  */
typedef struct escand_escaninfo {
	bool escand_use_escan; /* Escan toggle */
	bool escand_escan_inprogress;
	int scan_type;
	struct escan_bss *escan_bss_head; /* raw escan results */
	struct escan_bss *escan_bss_tail;
} escand_escaninfo_t;

/* escand main data structure */
typedef struct escand_chaninfo {
	char name[16];
	int mode;
	wl_country_t country;
	bool country_is_edcrs_eu;
	chanspec_t selected_chspec;
	chanspec_t cur_chspec, recent_prev_chspec;
	bool cur_is_dfs;
	bool cur_is_dfs_weather;
	escand_rsi_t rs_info;
	escand_scan_chspec_t scan_chspec_list;
	int c_count[ESCAND_BW_MAX];
	uint escand_cs_scan_timer;	/* cs scan timer */
	uint escand_ci_scan_timer; /* ci scan timer */
	wl_scan_results_t *scan_results; /* raw scan results */
	escand_bss_info_entry_t *escand_bss_info_q; /* up-to-date parsed scan result queue */
	escand_chan_bssinfo_t* ch_bssinfo;
	uint32 flags; /* config flags */
	uint32 escand_scan_entry_expire; /* sec: how long to expire an scan entry */
	escand_assoclist_t *escand_assoclist;
	uint16 sta_status;
	int switch_reason;
	int escand_boot_only;
	int escand_scan_promisc;
	escand_activity_info_t escand_activity; /* activity details */
	uint64 escand_prev_chan_at;	/* last channel swich time */
	int escand_cs_dfs_pref;		/* Customer knob for dfs preference */
	int escand_cs_high_pwr_pref;	/* Customer knob for channel pwr preference */
	int escand_scs_dfs_scan;		/* Enabling escand_scs_dfs_scan when SCS mode is on */
	bool is160_bwcap;
	bool is160_upgradable;		/* Can upgrade from 80MHz to 160MHz based on dyn metric */
	bool is160_downgradable;	/* Can downgrade from 160MHz to 80MHz based on metric */
	escand_escaninfo_t *escand_escan;
	bool is_mu_active;		/* true if MU mode is currently in use */
	bool dyn160_cap;		/* true if IOVAR cap include 'dyn160' */
	bool dyn160_enabled;		/* value fetched from IOVAR dyn160 */
	uint8 phy_dyn_switch;		/* value fetched form IOVAR phy_dyn_switch */
	escand_conf_chspec_t pref_chans; /* Prefer chan list */
	escand_conf_chspec_t excl_chans; /* Exclude chan list */
	uint32 timestamp_escand_scan;	/* timestamp of last scan */
	uint32 timestamp_tx_idle;	/* timestamp of last tx idle check */
	uint8 escand_ci_scan_count;	/* how many channel left for current ci_scan loop */
	uint32 escand_ci_scan_timeout;	/* start ci scan if ci_scan timeout */
	uint32 escand_txframe;			/* current txframe */

	int escand_far_sta_rssi;		/* rssi value for far sta */

	/* csa mode */
	uint8 escand_dcs_csa;
	/* toa video sta */
	bool escand_toa_enable;
	int video_sta_idx;
	escand_toa_video_sta_t vid_sta[ESCAND_MAX_VIDEO_STAS];
	uint32 cur_timestamp;
	uint32 timestamp;
	uint8 txop_channel_select;
	int ci_scan_txop_limit;		/* txop limit check to trigger channel change */
	char *vifnames[ESCAND_MAX_VIFNAMES];
	uint8 escand_txop_thresh;
	uint32 glitch_cnt;
	int wasdown;
} escand_chaninfo_t;

typedef struct {
	escand_chaninfo_t* chan_info[ESCAND_MAX_IF_NUM];
	ifname_idx_map_t escand_ifmap[ESCAND_MAX_IF_NUM];
	char *exclude_ifnames[ESCAND_EXCLUDE_IFACE_LIST];
} escand_info_t;

typedef struct escand_wksp_s {
	int version;
	char ifnames[ESCAND_IFNAME_SIZE*ESCAND_MAX_INTERFACES]; /* interface names */
	uint8 packet[ESCAND_BUFSIZE_4K];
	fd_set fdset;
	int fdmax;
	int event_fd;
	int listen_fd; /* server listen fd */
	char* cmd_buf; /* CLI buf */
	int conn_fd; /* client connection fd */
	uint poll_interval; /* polling interval */
	uint ticks;			/* number of polling intervals */
	escand_stats_t stats;
	escand_info_t *escand_info;
} escand_wksp_t;

typedef struct band_param {
	uint32 band;
	uint32 bw_cap;
} escand_param_info_t;

extern void escand_init_run(escand_info_t ** escand_info_p);
extern void escan_cleanup(escand_info_t ** escand_info_p);
#define escand_segment_allocate(x) (void)(x)
extern int escand_do_ci_update(uint ticks, escand_chaninfo_t *c_info);
extern int escand_update_status(escand_chaninfo_t * c_info);
extern int escand_update_assoc_info(escand_chaninfo_t * c_info);
extern int escand_run_escan(escand_chaninfo_t *c_info, uint8 scan_type);
extern int escand_run_normal_ci_scan(escand_chaninfo_t *c_info);
extern int escand_run_normal_cs_scan(escand_chaninfo_t *c_info);
extern int escand_request_normal_scan_data(escand_chaninfo_t *c_info);
extern int escand_request_escan_data(escand_chaninfo_t *c_info);
extern int escand_escan_prep_cs(escand_chaninfo_t *c_info, wl_scan_params_t *params, int *params_size);
extern int escand_escan_prep_ci(escand_chaninfo_t *c_info, wl_scan_params_t *params, int *params_size);
extern void escand_escan_free(struct escan_bss *node);
extern int escand_build_scanlist(escand_chaninfo_t *c_info);
extern int escand_run_cs_scan(escand_chaninfo_t *c_info);
extern int escand_idx_from_map(char *name);
extern int escand_request_data(escand_chaninfo_t *c_info);

extern chanspec_t escand_adjust_ctrl_chan(escand_chaninfo_t *c_info, chanspec_t chspec);
extern int escand_scan_timer_check(escand_chaninfo_t * c_info);
extern int escand_ci_scan_check(escand_chaninfo_t * c_info);
extern void escand_dump_scan_entry(escand_chaninfo_t *c_info);

extern int dcs_handle_request(char* ifname, wl_bcmdcs_data_t *dcs_data, uint8 mode,
	uint8 count, uint8 csa_mode);
extern int escand_proc_cmd(escand_wksp_t* d_info, char* buf, uint size,
	uint* resp_size);

extern int escand_activity_update(escand_chaninfo_t * c_info);

extern int escand_get_initial_traffic_stats(escand_chaninfo_t * c_info);
extern int escand_upgrade_to160(escand_chaninfo_t * c_info);
extern void escand_main_loop(struct timeval *tv);
extern uint32 escand_channel_info(escand_chaninfo_t *c_info, chanspec_t chspec);
extern void escand_retrieve_config(escand_chaninfo_t *c_info, char *prefix);
/* extern chanspec_t escand_pick_chanspec(escand_chaninfo_t *c_info, int bw); */
extern void escand_parse_chanspec(chanspec_t chanspec, escand_channel_t* chan_ptr);
extern int escand_idle_check(escand_chaninfo_t *c_info);
extern bool escand_check_assoc_scb(escand_chaninfo_t * c_info);
extern void escand_cleanup_scan_entry(escand_chaninfo_t *c_info);
/* is chanspec DFS channel */
extern bool escand_is_dfs_chanspec(escand_chaninfo_t *c_info, chanspec_t chspec);
extern bool escand_is_dfs_weather_chanspec(escand_chaninfo_t *c_info, chanspec_t chspec);
extern int escand_build_candidates(escand_chaninfo_t *c_info, int bw);
/* extern chanspec_t escand_pick_chanspec_common(escand_chaninfo_t *c_info, int bw, int score_type); */
extern bool escand_dfs_channel_is_usable(escand_chaninfo_t *c_info, chanspec_t chspec);
extern int escand_get_perband_chanspecs(escand_chaninfo_t *c_info, chanspec_t input, char *buf,
	int length);
extern int escand_get_per_chan_info(escand_chaninfo_t *c_info, chanspec_t sub_chspec, char *buf,
	int length);
extern int escand_set_scanresults_minrssi(escand_chaninfo_t *c_info, int minrssi);
extern int escand_set_escan_params(escand_chaninfo_t *c_info, wl_escan_params_t *params,
	int params_size);
extern int escand_set_chanspec(escand_chaninfo_t *c_info, chanspec_t chspec);
extern int escand_get_obss_coex_info(escand_chaninfo_t *c_info, int *coex);
extern int escand_get_bwcap_info(escand_chaninfo_t *c_info, escand_param_info_t *param, int param_len,
	char *buf, int buf_len);
extern int escand_get_cap_info(escand_chaninfo_t *c_info, uint32 *param, int param_len, char *cap_buf,
	int cap_len);
extern int escand_get_counters(char *ifname, char cntbuf[]);
extern int escand_get_phydyn_switch_status(char *name, int *phy_dyn_switch);
extern int escand_get_chanspec(escand_chaninfo_t *c_info, int *chanspec);
extern int escand_get_isup(escand_chaninfo_t *c_info, int *isup);
extern int escand_get_stainfo(char *name, struct ether_addr *ea, int ether_len, char *stabuf,
	int buf_len);
extern int escand_set_noise_metric(char *name, uint8 knoise);
extern int escand_get_scb_probe(char *ifname, wl_scb_probe_t *scb_probe, int size);
extern int escand_set_scb_probe(char *ifname, wl_scb_probe_t *scb_probe, int size_probe);
extern int escand_get_country(escand_chaninfo_t * c_info);
extern int escand_ci_scan_finish_check(escand_chaninfo_t * c_info);
extern int escand_set_chan_table(char *channel_list, chanspec_t *chspec_list,
        unsigned int vector_size);
extern void escand_ci_scan_update_idx(escand_scan_chspec_t *chspec_q, uint8 increment);
/* look for str in capability (wl cap) and return true if found */
extern bool escand_check_cap(escand_chaninfo_t *c_info, char *str);
extern void escand_check_ifname_is_virtual(char **ifname);
extern int escand_start(char *name, escand_chaninfo_t *c_info);
extern int escand_set_far_sta_rssi(escand_chaninfo_t *c_info, int far_sta);
extern int escand_update_rssi(escand_chaninfo_t *c_info, unsigned char *addr);
extern bool escand_is_mode_check(char *osifname);

#endif  /* _escand_srv_h_ */
