/*
 * BSD shared include file
 *
 * Copyright 2019 Broadcom
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
 * $Id: bsd.h 398225 2013-04-23 22:33:56Z $
 */

#ifndef _bsd_h_
#define _bsd_h_

#include <ethernet.h>
#include <bcmeth.h>
#include <bcmevent.h>
#include <802.11.h>

#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <signal.h>

#include <assert.h>
#include <typedefs.h>
#include <bcmnvram.h>
#include <bcmutils.h>
#include <bcmparams.h>
#include <bcmtimer.h>
#include <bcmendian.h>

#include <shutils.h>
#include <bcmendian.h>
#include <bcmwifi_channels.h>
#include <wlioctl.h>
#include <wlutils.h>

#include <security_ipc.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <net/if.h>
#include <arpa/inet.h>

#ifdef BCM_WBD
#include "bsd_wbd.h"
#endif /* BCM_WBD */

#include "appeventd.h"
#include "appeventd_bsd.h"
#include "bcm_steering.h"

#define BSD_OK	0
#define BSD_FAIL -1

/* default polling interval */
#define BSD_POLL_INTERVAL		1
/* default sta_info poll interval */
#define BSD_STATUS_POLL_INTV	5
#define BSD_COUNTER_POLL_INTV	0
/* default interval to cleanup assoclist */
#define BSD_STA_TIMEOUT			120
/* default sta dwell time after channel band */
#define BSD_STEER_TIMEOUT		15
/* default probe list timeout */
#define BSD_PROBE_TIMEOUT	3600
/* time intv of last probe seen */
#define BSD_PROBE_GAP		30
/* default probe list timeout */
#define BSD_MACLIST_TIMEOUT	30
/* STA maclist timeout after SUCC */
#define BSD_STA_TIMEOUT_AFTER_SUCC 60
/* interval for getting the valid BSSID */
#define BSD_GET_VALID_BSSID_TIMEOUT	1
#define BSD_WBD_GET_VALID_BSSID_TIMEOUT	(10 * 1000)

/* Number of retry for getting valid BSSID */
#define BSD_GET_VALID_BSSID_RETRY_COUNT	10
#define BSD_WBD_GET_VALID_BSSID_RETRY_COUNT	500

#define BSD_BUFSIZE_4K	4096
#define BSD_BUFSIZE_1K	1024

typedef enum {
	BSD_MODE_DISABLE = 0,
	BSD_MODE_MONITOR = 1,
	BSD_MODE_STEER = 2,
	BSD_MODE_MAX = 3
} bsd_mode_t;

typedef enum {
	BSD_ROLE_NONE = 0,
	BSD_ROLE_PRIMARY = 1,
	BSD_ROLE_HELPER = 2,
	BSD_ROLE_STANDALONE = 3,
	BSD_ROLE_MAX = 4
} bsd_role_t;

typedef enum {
	BSD_CMD_CONFIG_INFO = 0,
	BSD_CMD_STA_INFO = 1,
	BSD_CMD_STA_CONFIG_HDLR = 2,
	BSD_CMD_STEER_LOG = 3,
	BSD_CMD_STA_STATS = 4,
	BSD_CMD_RADIO_STATS = 5,
	BSD_CMD_CHANGE_TTY_CONSOLE = 6
} bsd_cmd_id_t;

#define BSD_BSS_PRIO_DISABLE	0xff

#define BSD_VERSION		1
#define BSD_DFLT_FD		-1

/* BSD and WBD enabled flags */
#define BSD_FLAG_ENABLED	0x0001
#define BSD_FLAG_WBD_ENABLED	0x0002

/* Debug Print */
extern int bsd_msglevel;
#define BSD_DEBUG_ERROR		0x000001
#define BSD_DEBUG_WARNING	0x000002
#define BSD_DEBUG_INFO		0x000004
#define BSD_DEBUG_TO		0x000008
#define BSD_DEBUG_STEER		0x000010
#define BSD_DEBUG_EVENT		0x000020
#define BSD_DEBUG_HISTO		0x000040
#define BSD_DEBUG_CCA		0x000080
#define BSD_DEBUG_AT		0x000100
#define BSD_DEBUG_RPC		0x000200
#define BSD_DEBUG_RPCD		0x000400
#define BSD_DEBUG_RPCEVT	0x000800
#define BSD_DEBUG_MULTI_RF	0x001000
#define BSD_DEBUG_BOUNCE	0x002000
#define BSD_DEBUG_WBD		0x004000
#define BSD_DEBUG_APPEVENT	0x008000

#define BSD_DEBUG_DUMP		0x100000
#define BSD_DEBUG_PROBE		0x400000
#define BSD_DEBUG_ALL		0x800000

#define BSD_ERROR(fmt, arg...) \
	do { if (bsd_msglevel & BSD_DEBUG_ERROR) \
		printf("BSD %lu %s(%d): "fmt, time(NULL), __FUNCTION__, __LINE__, ##arg); \
	} while (0)

#define BSD_WARNING(fmt, arg...) \
	do { if (bsd_msglevel & BSD_DEBUG_WARNING) \
		printf("BSD %lu %s(%d): "fmt, time(NULL), __FUNCTION__, __LINE__, ##arg); \
	} while (0)

#define BSD_INFO(fmt, arg...) \
	do { if (bsd_msglevel & BSD_DEBUG_INFO) \
		printf("BSD %lu %s(%d): "fmt, time(NULL), __FUNCTION__, __LINE__, ##arg); \
	} while (0)

#define BSD_TO(fmt, arg...) \
	do { if (bsd_msglevel & BSD_DEBUG_TO) \
		printf("BSD %lu %s(%d): "fmt, time(NULL), __FUNCTION__, __LINE__, ##arg); \
	} while (0)

#define BSD_STEER(fmt, arg...) \
	do { if (bsd_msglevel & BSD_DEBUG_STEER) \
		printf("BSD %lu %s(%d): "fmt, time(NULL), __FUNCTION__, __LINE__, ##arg); \
	} while (0)

#define BSD_EVENT(fmt, arg...) \
	do { if (bsd_msglevel & BSD_DEBUG_EVENT) \
		printf("BSD %lu %s(%d): "fmt, time(NULL), __FUNCTION__, __LINE__, ##arg); \
	} while (0)

#define BSD_HISTO(fmt, arg...) \
	do { if (bsd_msglevel & BSD_DEBUG_HISTO) \
		printf("BSD %lu %s(%d): "fmt, time(NULL), __FUNCTION__, __LINE__, ##arg); \
	} while (0)

#define BSD_CCA(fmt, arg...) \
	do { if (bsd_msglevel & BSD_DEBUG_CCA) \
		printf("BSD %lu %s(%d): "fmt, time(NULL),  __FUNCTION__, __LINE__, ##arg); \
	} while (0)

#define BSD_CCA_PLAIN(fmt, arg...) \
		do { if (bsd_msglevel & BSD_DEBUG_CCA) printf(fmt, ##arg); } while (0)

#define BSD_AT(fmt, arg...) \
	do { if (bsd_msglevel & BSD_DEBUG_AT) \
		printf("BSD %lu %s(%d): "fmt, time(NULL), __FUNCTION__, __LINE__, ##arg); \
	} while (0)

#define BSD_AT_PLAIN(fmt, arg...) \
		do { if (bsd_msglevel & BSD_DEBUG_AT) printf(fmt, ##arg); } while (0)

#define BSD_RPC(fmt, arg...) \
	do { if (bsd_msglevel & BSD_DEBUG_RPC) \
		printf("BSD %lu %s(%d): "fmt, time(NULL), __FUNCTION__, __LINE__, ##arg); \
	} while (0)

#define BSD_RPC_PLAIN(fmt, arg...) \
		do { if (bsd_msglevel & BSD_DEBUG_RPC) printf(fmt, ##arg); } while (0)

#define BSD_RPCD(fmt, arg...) \
	do { if (bsd_msglevel & BSD_DEBUG_RPCD) \
		printf("BSD %lu %s(%d): "fmt, time(NULL), __FUNCTION__, __LINE__, ##arg); \
	} while (0)

#define BSD_RPCD_PLAIN(fmt, arg...) \
		do { if (bsd_msglevel & BSD_DEBUG_RPCD) printf(fmt, ##arg); } while (0)

#define BSD_RPCEVT(fmt, arg...) \
	do { if (bsd_msglevel & BSD_DEBUG_RPCEVT) \
		printf("BSD %lu %s(%d): "fmt, time(NULL), __FUNCTION__, __LINE__, ##arg); \
	} while (0)

#define BSD_RPCEVT_PLAIN(fmt, arg...) \
		do { if (bsd_msglevel & BSD_DEBUG_RPCEVT) printf(fmt, ##arg); } while (0)

#define BSD_PROB(fmt, arg...) \
	do { if (bsd_msglevel & BSD_DEBUG_PROBE) \
		printf("BSD %lu %s(%d): "fmt, time(NULL), __FUNCTION__, __LINE__, ##arg); \
	} while (0)

#define BSD_MULTI_RF(fmt, arg...) \
	do { if (bsd_msglevel & BSD_DEBUG_MULTI_RF) \
		printf("BSD %lu %s(%d): "fmt, time(NULL), __FUNCTION__, __LINE__, ##arg); \
	} while (0)

#define BSD_BOUNCE(fmt, arg...) \
	do { if (bsd_msglevel & BSD_DEBUG_BOUNCE) \
			printf("BSD %lu %s(%d): "fmt, time(NULL), __FUNCTION__, __LINE__, ##arg); \
	} while (0)

#define BSD_WBD(fmt, arg...) \
	do { if (bsd_msglevel & BSD_DEBUG_WBD) \
			printf("BSD %lu %s(%d): "fmt, time(NULL), __FUNCTION__, __LINE__, ##arg); \
	} while (0)

#define BSD_APPEVENT(fmt, arg...) \
	do { if (bsd_msglevel & BSD_DEBUG_APPEVENT) \
		printf("BSD %lu %s(%d): "fmt, time(NULL), __FUNCTION__, __LINE__, ##arg); \
	} while (0)

#define BSD_ALL(fmt, arg...) \
	do { if (bsd_msglevel & BSD_DEBUG_ALL) \
		printf("BSD %lu %s(%d): "fmt, time(NULL), __FUNCTION__, __LINE__, ##arg); \
	} while (0)

#define BSD_PRINT(fmt, arg...) \
	do { printf("BSD %lu %s(%d): "fmt, time(NULL), __FUNCTION__, __LINE__, ##arg); \
	} while (0)

#define BSD_PRINT_PLAIN(fmt, arg...) \
		do { printf(fmt, ##arg); } while (0)

#define BSD_DUMP_ENAB	(bsd_msglevel & BSD_DEBUG_DUMP)
#define BSD_PROB_ENAB	(bsd_msglevel & BSD_DEBUG_PROBE)
#define BSD_STEER_ENAB	(bsd_msglevel & BSD_DEBUG_STEER)

#define tr() do { if (BSD_DUMP_ENAB) printf("%s@%d\n", __FUNCTION__, __LINE__); } while (0)
#define BSD_ENTER()	BSD_ALL("Enter...\n")
#define BSD_EXIT() 	BSD_ALL("Exit...\n")

#define BSD_RPC_ENAB	(bsd_msglevel & BSD_DEBUG_RPC)
#define BSD_RPCEVT_ENAB	(bsd_msglevel & BSD_DEBUG_RPCEVT)

#define BSD_EVTENTER()	BSD_EVENT("Enter...\n")
#define BSD_EVTEXIT() 	BSD_EVENT("Exit...\n")

#define BSD_CCAENTER()	BSD_CCA("Enter...\n")
#define BSD_CCAEXIT() 	BSD_CCA("Exit...\n")
#define BSD_CCA_ENAB	(bsd_msglevel & BSD_DEBUG_CCA)

#define BSD_ATENTER()	BSD_AT("Enter...\n")
#define BSD_ATEXIT()	BSD_AT("Exit...\n")
#define BSD_AT_ENAB		(bsd_msglevel & BSD_DEBUG_AT)

#define BCM_REFERENCE(data)	((void)(data))

#define BSD_MAX_PRIO		0x4
#define BSD_IFNAME_SIZE		16
#define BSD_DEFAULT_INTF_NUM	2	/* default dual-band router RF number  */
#define BSD_ATLAS_MAX_INTF		3	/* atlas ref. board 2 5G, 1 2.4G band max */
#define BSD_MAX_INTF			BSD_ATLAS_MAX_INTF

#define BSD_MAXBSSCFG		WL_MAXBSSCFG

#define BSD_SCHEME			0

typedef struct bsd_staprio_config {
	struct ether_addr addr;
	uint8 prio;			/* 1-Video STA, 0-data-STA */
	uint8 steerflag;	/* assoc'ed STAs can steer?  */
	struct bsd_staprio_config *next;
} bsd_staprio_config_t;

#define BSD_STA_STATE_PROBE			(1 << 0)
#define BSD_STA_STATE_AUTH			(1 << 1)
#define BSD_STA_STATE_ASSOC			(1 << 2)
#define BSD_STA_STATE_REASSOC		(1 << 3)
#define BSD_STA_STATE_CONNECTED		(1 << 4)
#define BSD_STA_STATE_DEAUTH		(1 << 5)
#define BSD_STA_STATE_DEASSOC		(1 << 6)
#define BSD_STA_STATE_STEER_FAIL	(1 << 7)

/* steer failure cnt bigger than this threshold is considered as picky sta */
#define BSD_STA_STEER_FAIL_PICKY_CNT 2
#define BSD_BAND_DETECT_MAX_TRY 3

typedef struct bsd_sta_info {
	time_t timestamp;	/* assoc timestamp */
	time_t active;		/* activity timestamp */

	struct ether_addr addr; /* mac addr */
	struct ether_addr paddr; /* psta intf */
	uint8 prio;			/* 1-Video STA, 0-data-STA */
	uint8 steerflag;	/* assoc'ed STAs can steer?  */

#ifdef BCM_WBD
	int wbd_sta_status;	/* Flag indicating Weak or DWDS STA. 0 Is Normal */
	time_t wbd_dwell_start;	/* Timestamp at which dwell time started */
	uint32 wbd_dwell_time;	/* Remaining dwell_time */
	int32 reported_rssi;	/* RCPI which is reported earlier */
#endif /* BCM_WBD */

	uint8 band;
	uint8 band_detect_cnt; /* send BSS transition request if dual-band is not detected yet */
	int32 rssi;		/* per antenna rssi */
	uint32 phy_rate;	/* unit Mbps */
	uint32 tx_rate;		/* unit Mbps */
	uint32 rx_rate;		/* unit Mbps */
	uint32 rx_pkts;	/* # of data packets recvd since last poll */
	uint32 tx_pkts;		/* # of packets transmitted since last poll */
	uint32 rx_bps;	/* txbps, in Kbps */
	uint32 tx_bps;	/* txbps, in Kbps */
	uint32 tx_failures;	/* tx retry */
	uint32 idle;		/* time since data pkt rx'd from sta */
	uint32 in;

	uint8 at_ratio;		/* airtime ratio */
	uint32 phyrate;		/* txsucc rate, in Mbps */
	uint32 datarate;	/* txsucc rate, in Kbps */

	uint32 mcs_phyrate;	/* sta_info's tx_rate, based on rspec */

	int32  score;			/* STA's score */
	uint32 rx_tot_pkts;	/* # of data packets recvd */
	uint64 rx_tot_bytes; /* bytes recvd */
	uint32 tx_tot_failures;	/* tx retry */
	uint32 tx_tot_pkts;		/* # of packets transmitted */
	struct bsd_sta_info *snext, *sprev;
	struct bsd_sta_info *next, *prev;
	struct bsd_bssinfo *bssinfo;

	uint32 flags;	/* sta flags from wl */
	struct bsd_bssinfo *to_bssinfo;
} bsd_sta_info_t;

#define BSD_INIT_ASSOC	(1 << 0)
#define BSD_INIT_BS_5G	(1 << 1)
#define BSD_INIT_BS_2G	(1 << 2)

#define BSD_BAND_2G	WLC_BAND_2G
#define BSD_BAND_5G	WLC_BAND_5G
#define BSD_BAND_ALL WLC_BAND_ALL

#define BSD_PROBEREQ_RSSI_VALID_PERIOD 10
typedef struct bsd_rssi {
	time_t timestamp;
	int32   rssi;
} bsd_rssi_t;

typedef struct bsd_maclist {
	struct ether_addr addr;
	time_t timestamp;		/* assoc timestamp */
	time_t active;			/* activity timestamp */
	uint8 band;
	uint8 states[BSD_MAX_INTF];	/* 802.11 sta probe/auth/assoc, RF histo bits per if */
	bsd_rssi_t rssi_info[BSD_MAX_INTF];	/* rssi log */
	/* for steer result */
	struct bsd_bssinfo *to_bssinfo;
	uint8 steer_state;
	struct bsd_maclist *next;
} bsd_maclist_t;

/* bit masks for stat_select_policy flags */
#define BSD_STA_SELECT_POLICY_FLAG_RULE			0x00000001	/* logic AND chk */
#define BSD_STA_SELECT_POLICY_FLAG_RSSI			0x00000002	/* RSSI */
#define BSD_STA_SELECT_POLICY_FLAG_VHT			0x00000004	/* VHT STA */
#define BSD_STA_SELECT_POLICY_FLAG_NON_VHT		0x00000008	/* NON VHT STA */
#define BSD_STA_SELECT_POLICY_FLAG_NEXT_RF		0x00000010	/* check next RF */
#define BSD_STA_SELECT_POLICY_FLAG_PHYRATE		0x00000020	/* phyrate */
#define BSD_STA_SELECT_POLICY_FLAG_LOAD_BAL		0x00000040	/* load balance */
#define BSD_STA_SELECT_POLICY_FLAG_SINGLEBAND	0x00000080	/* single band sta */
#define BSD_STA_SELECT_POLICY_FLAG_DUALBAND		0x00000100	/* dual band sta */
#define BSD_STA_SELECT_POLICY_FLAG_ACTIVE_STA	0x00000200	/* active sta */
#define BSD_STA_SELECT_POLICY_FLAG_SCORE		0x00000400	/* score compare */
#define BSD_STA_SELECT_POLICY_FLAG_SCORE2		0x00000800	/* for 2nd dest_if */

typedef struct bsd_sta_select_policy {
	int idle_rate;	/* data rate threshold to measure STA is idle */
	int rssi;		/* rssi threshold */
	uint32 phyrate;	/* phyrate threshold in Mbps */
	int wprio;		/* weight for prio */
	int wrssi;			/* weight for RSSI */
	int wphy_rate;		/* weight for phy_rate */
	int wtx_failures;	/* weight for tx retry */
	int wtx_rate;		/* weight for tx_rate */
	int wrx_rate;		/* weight for rx_rate */
	uint32 flags;		/* extension flags */
} bsd_sta_select_policy_t;

#define BSD_BSSCFG_NOTSTEER	(1 << 0)
#define BSD_STA_BSS_REJECT	(1 << 1)
#define BSD_STA_BSSTRANS_UNSUPPORTED	(1 << 2)

#define BSD_BSS_RESPONSE_ACCEPT	0
#define BSD_BSS_RESPONSE_REJECT	1
#define BSD_BSS_RESPONSE_UNKNOWN 2

#define BSD_POLICY_LOW_RSSI 0
#define BSD_POLICY_HIGH_RSSI 1
#define BSD_POLICY_LOW_PHYRATE 2
#define BSD_POLICY_HIGH_PHYRATE 3

/* forward declaration */
typedef struct bsd_if_bssinfo_list bsd_if_bssinfo_list_t;

#ifndef BCM_WBD
typedef struct bsd_bssinfo bsd_bssinfo_t;
#endif /* BCM_WBD */

struct bsd_bssinfo {
	bool valid;
	char ifnames[BSD_IFNAME_SIZE]; /* interface names */
	char prefix[BSD_IFNAME_SIZE];	/* Prefix name */
	char primary_prefix[BSD_IFNAME_SIZE];	/* primary interface prefix */
	char ssid[32];
	struct ether_addr bssid;
	chanspec_t chanspec;
	uint8 rclass;
	txpwr_target_max_t txpwr;

	int idx;
	uint8 prio;
	uint8 steerflag;	/* STAs can steer?  */
	struct bsd_bssinfo *steer_bssinfo;
	char steer_prefix[BSD_IFNAME_SIZE];

	bsd_if_bssinfo_list_t *to_if_bss_list;

	uint8 algo;
	uint8 policy;
	bsd_sta_select_policy_t sta_select_cfg;
	bool sta_select_policy_defined;
	struct bsd_intf_info *intf_info;
	uint8 assoc_cnt;	/* no of data-sta assoc-ed */
	bsd_sta_info_t *assoclist;
	bsd_maclist_t *maclist;
	int macmode;	/* deny, allow, disable */

	struct maclist *static_maclist;
	int static_macmode;

	/* hack:jhc */
	uint32 tx_tot_pkts, rx_tot_pkts;
	bool video_idle;

	uint32 throughput; /* for bsd radio stats */
#ifdef BCM_WBD
	bool rd_stats_again;
#endif // endif
	wl_wlif_hdl *wlif_hdl;	/* steering lib handle. */
};

struct bsd_if_bssinfo_list {
	struct bsd_if_bssinfo_list *next;

	struct bsd_bssinfo *bssinfo;
	int to_ifidx;
};

#define BSD_PROBE_STA_HASH	32
#define BSD_MAC_HASH(ea)		(((ea).octet[4]+(ea).octet[5])% BSD_PROBE_STA_HASH)

#define BSD_CHANIM_STATS_MAX	60
typedef struct bsd_chanim_stats {
	chanim_stats_t stats;
	uint8 valid;
} bsd_chanim_stats_t;

#define BSD_CHAN_STEER_MASK		0x80

typedef enum {
	BSD_CHAN_BUSY_UNKNOWN = 0,
	BSD_CHAN_BUSY = 1,
	BSD_CHAN_IDLE = 2,
	BSD_CHAN_NO_STATS = 3,
	BSD_CHAN_UTIL_MAX = 4
} bsd_chan_state_t;

#define LAN_IFNAMES_NVRAM		"lan_ifnames"
#define BSD_IFNAMES_NVRAM		"bsd_ifnames"
#define BSD_STEERING_POLICY_NVRAM	"bsd_steering_policy"
#define BSD_STA_SELECT_POLICY_NVRAM	"bsd_sta_select_policy"
#define BSD_IF_SELECT_POLICY_NVRAM	"bsd_if_select_policy"
#define BSD_IF_QUALIFY_POLICY_NVRAM	"bsd_if_qualify_policy"
#define BSD_BOUNCE_DETECT_NVRAM		"bsd_bounce_detect"
#define BSD_NON11V_DETECT_NVRAM		"bsd_non11v_detect"

/* steering_flags bits definition */
#define BSD_STEERING_POLICY_FLAG_RULE			0x00000001	/* logic AND chk */
#define BSD_STEERING_POLICY_FLAG_RSSI			0x00000002	/* RSSI */
#define BSD_STEERING_POLICY_FLAG_VHT			0x00000004	/* VHT STA */
#define BSD_STEERING_POLICY_FLAG_NON_VHT		0x00000008	/* NON VHT STA */
#define BSD_STEERING_POLICY_FLAG_NEXT_RF		0x00000010	/* check next RF */
#define BSD_STEERING_POLICY_FLAG_PHYRATE		0x00000020	/* phyrate */
#define BSD_STEERING_POLICY_FLAG_LOAD_BAL		0x00000040	/* load balance */
#define BSD_STEERING_POLICY_FLAG_STA_NUM_BAL	0x00000080	/* sta num balance */
/* use 2 record bits for steer result */
#define BSD_STEERING_RESULT_SUCC				0x00010000
#define BSD_STEERING_RESULT_FAIL				0x00020000
#define BSD_STEERING_POLICY_FLAG_CHAN_OVERSUB	0x80000000	/* over */

#define BSD_CHAN_BUSY_MIN		20	/* default min 20% */
#define BSD_CHAN_BUSY_MAX		80	/* default max 80% */
#define BSD_CHAN_BUSY_CNT		3	/* continuous sample cnt */
#define BSD_CHAN_BUSY_PERIOD	5	/* sample persiod. 5 sec */

typedef struct bsd_chan_util_info {
	bsd_chanim_stats_t rec[BSD_CHANIM_STATS_MAX];
	uint8 idx;
	uint8 ticks;
	uint32 chanim_version;
} bsd_chan_util_info_t;

typedef struct bsd_steering_policy {
	/* bsd_steering_policy config parameters */
	int period;			/* sample time in seconds */
	int cnt;			/* consecutive sampling count */
	int chan_busy_max;	/* oversubscription  bw util % threshold */

	int rssi;		/* STA steering rssi threshold, 0 means no check */
	uint32 phyrate;	/* phy rate threshold in Mbps, 0 means no check */
	uint32 flags;	/* extension flags */
} bsd_steering_policy_t;

#define BSD_QUALIFY_POLICY_RSSI_DEFAULT		-75			/* if qualify RSSI */

#define BSD_QUALIFY_POLICY_FLAG_RULE		0x00000001	/* logic AND chk */
#define BSD_QUALIFY_POLICY_FLAG_VHT			0x00000002	/* VHT STA */
#define BSD_QUALIFY_POLICY_FLAG_NON_VHT		0x00000004	/* NON VHT STA */
#define BSD_QUALIFY_POLICY_FLAG_PHYRATE		0x00000020	/* phyrate */
#define BSD_QUALIFY_POLICY_FLAG_LOAD_BAL	0x00000040	/* load balance */
#define BSD_QUALIFY_POLICY_FLAG_STA_BAL		0x00000080	/* sta balance */

/* bsd_if_qualify_policy config parameters */
typedef struct bsd_if_qualify_policy {
	int min_bw;		/* qualify STA to this RF if chan util bw % is less than */
	uint32 flags;	/* qualify flag */
	int rssi;		/* qualify RSSI to this RF */
} bsd_if_qualify_policy_t;

/* BSD bounce detection defaults */
/* 180 seconds detection window, consecutive count 2, and dwell time 3600 seconds */
#define BSD_BOUNCE_DETECT_WIN	180
#define BSD_BOUNCE_DETECT_CNT	2
#define BSD_BOUNCE_DETECT_DWELL	3600

#define BSD_BOUNCE_HASH_SIZE	BSD_PROBE_STA_HASH
#define BSD_BOUNCE_MAC_HASH		BSD_MAC_HASH

typedef enum {
	BSD_BOUNCE_INIT = 0,
	BSD_BOUNCE_WINDOW_STATE,
	BSD_BOUNCE_DWELL_STATE,
	BSD_BOUNCE_CLEAN_STATE
} bsd_bounce_state_t;

typedef struct bsd_bounce_detect {
	uint32 window;		/* window time in seconds */
	uint32 cnt;			/* counts */
	uint32 dwell_time;	/* dwell time in seconds */
} bsd_bounce_detect_t;

typedef struct bsd_sta_bounce_detect {
	time_t timestamp;	/* timestamp of first steering time */
	struct ether_addr addr;
	bsd_bounce_detect_t run; /* sta bounce detect running counts */
	bsd_bounce_state_t state;
	app_event_bsd_sta_t stats;
	time_t non11v_timestamp;	/* timestamp of first steering time */
	bsd_bounce_detect_t non11v_run; /* non11v sta detect running counts */
	bsd_bounce_state_t non11v_state; /* non11v sta state */
	struct bsd_sta_bounce_detect *next;
} bsd_sta_bounce_detect_t;

typedef struct bsd_intf_info {
	uint8 band;
	uint8 phytype;

	uint8 remote;	/* adapter is remote? */
	uint8 enabled;
	int idx;		/* this array index */
	int unit;		/* the actual interface index */

	uint32 steering_flags;	/* refer to BSD_STEERING_POLICY_FLAG_* bits defs */

	bsd_steering_policy_t steering_cfg;
	bsd_if_qualify_policy_t qualify_cfg;

	bsd_chan_util_info_t chan_util;
	bsd_chan_state_t state;

	bsd_bssinfo_t bsd_bssinfo[WL_MAXBSSCFG];
	int	bw_util; /* new for bsd radio stats */
} bsd_intf_info_t;

typedef enum {
	BSD_STA_INVALID = 0,
	BSD_STA_ASSOCLIST = 1,
	BSD_STA_AUTH = 2,
	BSD_STA_ASSOC = 3,
	BSD_STA_STEERED = 4,
	BSD_STA_DEAUTH = 5,
	BSD_STA_DISASSOC = 6,
	BSD_STA_STEERING = 7,
	BSD_STA_STEER_FAIL = 8,
	BSD_STA_STEER_SUCC = 9,
	BSD_STA_MAX = 10
} bsd_sta_state_t;

#define BSD_MAX_STEER_REC	128

typedef struct bsd_steer_record {
	struct ether_addr addr;
	time_t timestamp;
	chanspec_t from_chanspec;
	chanspec_t to_chanspec;
	uint32 reason;
} bsd_steer_record_t;

#define BSD_MAX_AT_SCB			5
#define BSD_VIDEO_AT_RATIO_BASE	5
#define BSD_SLOWEST_AT_RATIO	40
#define BSD_PHYRATE_DELTA		200

#ifndef BCM_WBD
typedef struct bsd_info bsd_info_t;
#endif /* BCM_WBD */

struct bsd_info {
	int version;
	int event_fd;
	int event_fd2;		/* special for bss response event */
	int rpc_listenfd;
	int rpc_eventfd, rpc_ioctlfd;
	int cli_listenfd;		/* fd of client socket */
	uint max_ifnum;		/* max wl interface number for band steering */
	int enable_flag;	/* BSD/WBD enabled or not */

	/* config info */
	bsd_role_t role;
	char helper_addr[32], primary_addr[32];
	int hport, pport;
	bsd_mode_t mode; /* monitor, or steer */
	uint poll_interval; /* polling interval */
	uint ticks;		/* number of polling intervals */
	uint8 status_poll;
	uint8 counter_poll, idle_rate;
	uint lb_idle_rate;      /* idle rate for load balance */
	uint probe_timeout, probe_gap;
	uint maclist_timeout;
	uint steer_timeout;
	uint sta_timeout;
	uint8 prefer_5g;
	uint8 scheme;

	/* v/(v+d) threshold. video_at_ratio[n] is threshold for n+1 data-stas */
	/* n data-sta actively assoc, v/(v+d) > video_at_ratio[n]. steer */
	uint32 video_at_ratio[BSD_MAX_AT_SCB];

	/* for all data-STA, if delta(phyrate) > phyrate_delat
	 * && at_time(lowest phyrate sta) > at_rati: steer
	 */
	/* slowest data-sta airtime ratio */
	uint32 slowest_at_ratio;
	/* data-sta phyrate Delat threshold */
	uint32 phyrate_delta;

	/* STA bounce detect config */
	bsd_bounce_detect_t bounce_cfg;

	/* non11v STA detect config */
	bsd_bounce_detect_t non11v_cfg;

	uint8 ifidx, bssidx;
	uint8 over; /* tmp var: 1: 5G oversubscription, 2: 5G undersubscription, 0:no steer */
	bsd_staprio_config_t *staprio;

	/* info/data for each intf */
	bsd_maclist_t *prbsta[BSD_PROBE_STA_HASH];
	bsd_sta_bounce_detect_t *sta_bounce_table[BSD_BOUNCE_HASH_SIZE];
	bsd_intf_info_t *intf_info;

#ifdef BCM_WBD
	bsd_wbd_info_t *wbd_info;	/* Information about WBD */
#endif /* BCM_WBD */
	uint8 steer_no_deauth; /* 1: NOT sent deauth to legady sta, 0 (default) send deauth */
};

/* Data structiure or rpc operation */
#define BSD_DEFT_HELPER_ADDR	"192.168.1.2"
#define BSD_DEFT_PRIMARY_ADDR	"192.168.1.1"
#define BSD_DEFAULT_SERVER_HOST	"127.0.0.1"

#define	HELPER_PORT		9877	/* Helper TCP Server port */
#define	PRIMARY_PORT	9878	/* Primary TCP Server port  */

typedef enum {
	BSD_RPC_ID_IOCTL = 0,
	BSD_RPC_ID_EVENT = 1,
	BSD_RPC_ID_NVRAM = 2,
	BSD_RPC_ID_MAX = 3
} bsd_rpc_id_t;

typedef  struct  bsd_rpc_cmd {
	int ret;
	char name[BSD_IFNAME_SIZE];
	int cmd;
	int len;
} bsd_rpc_cmd_t;

typedef struct {
	bsd_rpc_id_t  id;
	bsd_rpc_cmd_t cmd;
} bsd_rpc_pkt_t;

typedef struct bsd_cmdargs {
	int socket;
	const char *server_host;
	const char *server_port;
	bsd_cmd_id_t cmd_id;
} bsd_cmdargs_t;
#define BSD_RPC_HEADER_LEN	(sizeof(bsd_rpc_pkt_t) + 1)

#define BSD_IOCTL_MAXLEN 4096
extern char ioctl_buf[BSD_IOCTL_MAXLEN];
extern char ret_buf[BSD_IOCTL_MAXLEN];
extern char cmd_buf[BSD_IOCTL_MAXLEN];
extern char maclist_buf[BSD_IOCTL_MAXLEN];

#define DIV_QUO(num, div) ((num)/div)  /* Return the quotient of division to avoid floats */
#define DIV_REM(num, div) (((num%div) * 100)/div) /* Return the remainder of division */

#define BSDSTRNCPY(dst, src, len)	 \
	do { \
		if (strlen(src) < len) \
			strcpy(dst, src); \
		else {	\
			strncpy(dst, src, len -1); dst[len - 1] = '\0'; \
		} \
	} while (0)

/* Use a possible max data rate to filter bogus phyrate reading */
#define BSD_MAX_DATA_RATE  6934

#define BSD_OUTPUT_FILE_LOG "/tmp/bsd.log"
#define BSD_OUTPUT_FILE_INFO "/tmp/bsd.info"
#define BSD_OUTPUT_FILE_STA "/tmp/bsd.sta"
#define BSD_OUTPUT_FILE_TTY "/tmp/tty.info"

#define BSD_OUTPUT_FILE_LOG_TMP "/tmp/bsd.log.tmp"
#define BSD_OUTPUT_FILE_INFO_TMP "/tmp/bsd.info.tmp"
#define BSD_OUTPUT_FILE_STA_TMP "/tmp/bsd.sta.tmp"

#define BSD_OUTPUT_FILE_TIMEOUT 5000000
#define BSD_OUTPUT_FILE_INTERVAL 500000

extern int bs_safe_get_conf(char *outval, int outval_size, char *name);
extern void sleep_ms(const unsigned int ms);
extern void bsd_retrieve_config(bsd_info_t *info);
extern int bsd_info_init(bsd_info_t *bsd_info);
extern int bsd_intf_info_init(bsd_info_t *bsd_info);
extern void bsd_assoc_sta(bsd_info_t *info, char *ifname, uint8 remote, struct ether_addr *addr);
extern void bsd_auth_sta(bsd_info_t *info, char *ifname, uint8 remote, struct ether_addr *addr);
extern void bsd_deauth_sta(bsd_info_t *info, char *ifname, uint8 remote, struct ether_addr *addr);
extern void bsd_disassoc_sta(bsd_info_t *info, char *ifname, uint8 remote, struct ether_addr *addr);
extern void bsd_remove_sta_reason(bsd_info_t *info, char *ifname, uint8 remote,
	struct ether_addr *addr, bsd_sta_state_t reason);
extern void bsd_bssinfo_cleanup(bsd_info_t *info);
extern void bsd_update_psta(bsd_info_t *info, char *ifname, uint8 remote, struct ether_addr *addr,
	struct ether_addr *paddr);
extern void bsd_update_stainfo(bsd_info_t *info);

extern void bsd_update_stb_info(bsd_info_t *info);
extern void bsd_update_cca_stats(bsd_info_t *info);
extern void bsd_reset_chan_busy(bsd_info_t *info, int ifidx);

extern void bsd_add_prbsta(bsd_info_t *info, char *ifname, uint8 remote, struct ether_addr *addr);
extern void bsd_timeout_prbsta(bsd_info_t *info);
extern void bsd_timeout_maclist(bsd_info_t *info);
extern void bsd_dump_info(bsd_info_t *info);
extern void bsd_steer_sta(bsd_info_t *info, bsd_sta_info_t *sta, bsd_bssinfo_t *to_bss);
extern void bsd_check_steer(bsd_info_t *info);
extern int bsd_get_max_policy(bsd_info_t *info);
extern bsd_sta_select_policy_t *bsd_get_sta_select_cfg(bsd_bssinfo_t *bssinfo);
extern int bsd_get_max_algo(bsd_info_t *info);
extern int bsd_get_max_scheme(bsd_info_t *info);
extern void bsd_set_maclist(bsd_bssinfo_t *bssinfo);
extern void bsd_stamp_maclist(bsd_info_t *info, bsd_bssinfo_t *bssinfo, struct ether_addr *addr);
extern void bsd_remove_maclist(bsd_bssinfo_t *bssinfo, struct ether_addr *addr);
extern void bsd_addto_maclist(bsd_bssinfo_t *bssinfo, struct ether_addr *addr,
	bsd_bssinfo_t *to_bssinfo);
extern bsd_maclist_t *bsd_maclist_by_addr(bsd_bssinfo_t *bssinfo, struct ether_addr *addr);
extern void bsd_timeout_sta(bsd_info_t *info);
extern bool bsd_is_sta_dualband(bsd_info_t *info, struct ether_addr *addr);
extern int bsd_wl_ioctl(bsd_bssinfo_t *bssinfo, int cmd, void *buf, int len);
extern int bsd_do_wl_ioctl(char *ifname, int cmd, void *buf, int len, uint8 remote);
extern void bsd_rpc_dump(char *ptr, int len, int enab);
extern void bsd_proc_socket(bsd_info_t*info, struct timeval *tv);
extern int bsd_proc_socket2(bsd_info_t*info, struct timeval *tv, char *ifreq,
	uint8 token, struct ether_addr *bssid);
extern void bsd_close_rpc_eventfd(bsd_info_t*info);
extern void bsd_close_eventfd(bsd_info_t*info);
extern int bsd_open_rpc_eventfd(bsd_info_t*info);
extern int bsd_open_eventfd(bsd_info_t*info);

extern bool bsd_check_oversub(bsd_info_t *info);
extern int bsd_aclist_steerable(bsd_bssinfo_t *bssinfo, struct ether_addr *addr);
extern bsd_sta_info_t *bsd_select_sta(bsd_info_t *info);

extern bsd_chan_state_t bsd_update_chan_state(bsd_info_t *info,
	bsd_intf_info_t *intf_info, bsd_bssinfo_t *bssinfo);

extern void bsd_steer_scheme_policy(bsd_info_t *info);
extern bsd_sta_info_t *bsd_sta_select_policy(bsd_info_t *info, int ifidx, int to_ifidx);
extern void bsd_if_policy(bsd_info_t *info, bsd_sta_info_t *sta);

extern void dump_if_bssinfo_list(bsd_bssinfo_t *bssinfo);
extern void clean_if_bssinfo_list(bsd_if_bssinfo_list_t* list);
extern int bsd_get_steerable_bss(bsd_info_t *info, bsd_intf_info_t *intf_info);
extern bool bsd_check_if_oversub(bsd_info_t *info, bsd_intf_info_t *intf_info);

extern void bsd_init_sta_bounce_table();
extern void bsd_update_sta_bounce_table(bsd_info_t *info);
extern void bsd_add_sta_to_bounce_table(bsd_info_t *info, struct ether_addr *addr);
extern bool bsd_check_bouncing_sta(bsd_info_t *info, struct ether_addr *addr);
extern void bsd_cleanup_sta_bounce_table(bsd_info_t *info);

extern bool bsd_check_non11v_sta(bsd_info_t *info, struct ether_addr *addr);

extern bsd_sta_info_t *bsd_sta_select_5g(bsd_info_t *info, int ifidx, int to_ifidx);
extern void bsd_steer_scheme_5g(bsd_info_t *info);
extern void bsd_steer_scheme_balance(bsd_info_t *info);
extern void bsd_update_sta_state_transition(bsd_info_t *info,
	bsd_intf_info_t *intf_info, struct ether_addr *addr, uint32 state);
extern bool bsd_check_picky_sta(bsd_info_t *info, struct ether_addr *addr);
extern bool bsd_qualify_sta_rf(bsd_info_t *info,
	bsd_bssinfo_t *bss, bsd_bssinfo_t *to_bssi, struct ether_addr *addr);
extern void bsd_steering_record_display(void);
extern void bsd_dump_config_info(bsd_info_t *info);
extern void bsd_dump_sta_info(bsd_info_t *info);
extern void bsd_update_steering_record(bsd_info_t *info, struct ether_addr *addr,
	bsd_bssinfo_t *bssinfo, bsd_bssinfo_t *to_bssinfo, uint32 reason);
extern void bsd_check_steer_result(bsd_info_t *info, struct ether_addr *addr,
	bsd_bssinfo_t *to_bssinfo);
extern int bsd_check_steer_fail(bsd_info_t *info, bsd_maclist_t *sta, bsd_bssinfo_t *bssinfo);
extern bool bsd_bcm_special_sta_check(bsd_info_t *info, bsd_sta_info_t *sta);
bsd_maclist_t *bsd_prbsta_by_addr(bsd_info_t *info,
	struct ether_addr *addr, bool enable);
extern void bsd_log_rssi(bsd_info_t *info, bsd_sta_info_t *sta, bool reset);
extern void bsd_retrieve_staprio_config(bsd_info_t *info);
extern void bsd_set_staprio(bsd_info_t *info);
/* bsd to appeventd */
extern bsd_sta_info_t *bsd_add_assoclist(bsd_bssinfo_t *bssinfo, struct ether_addr *addr,
	bool enable);
extern app_event_bsd_sta_t * bsd_update_sta_stats(bsd_info_t *info, struct ether_addr *addr,
	char *src_ifname, char *dst_ifname, int steer_fail, uint32 tx_rate,
	uint8 at_ratio, int rssi, uint32 intf_steer_flags, uint8 sta_steer_response);
extern void bsd_steer_appevent(bsd_info_t *info, struct ether_addr *addr, int eventid, int status);
extern void bsd_query_sta_stats(bsd_info_t *info);
extern void bsd_query_radio_stats(bsd_info_t *info);
#define BSD_APPEVENT_STA_MAC "/tmp/appevent_bsd_sta"
#ifdef BCM_WBD
extern int bsd_wbd_send_block_client_req(bsd_info_t *info, bsd_bssinfo_t *tbssinfo,
	bsd_sta_info_t *sta, int *wbd_ptr);
#endif /* BCM_WBD */
extern int bsd_ioctl_callback_func(char *ifname, int cmd, void *buf, int len, void *data);
extern bool bsd_is_chan_passive(bsd_bssinfo_t *bssinfo);
extern void bsd_info_hdlr(void);
extern void bsd_log_hdlr(void);
extern void bsd_sta_hdlr(void);
extern void bsd_sta_config_hdlr(void);
extern void bsd_tty_hdlr(void);
extern void bsd_query_sta_stats_hdlr(void);
extern void bsd_query_radio_stats_hdlr(void);
extern int bsd_open_server_cli_fd(bsd_info_t *info);
extern void bsd_close_socket(int *socket);
extern int bsd_open_cli_fd(uint8 cmd_id);
#endif /*  _bsd_h_ */
