/*
 * Common (OS-independent) definitions for
 * Broadcom 802.11abg Networking Device Driver
 *
 * Copyright (C) 2020, Broadcom. All Rights Reserved.
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 * SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
 * OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 *
 * <<Broadcom-WL-IPTag/Open:>>
 *
 * $Id: wlc_pub.h 787796 2020-06-11 23:04:24Z $
 */

#ifndef _wlc_pub_h_
#define _wlc_pub_h_

#include <wlc_types.h>
#include <siutils.h>
#include <802.11.h>
#include <802.11ax.h>
#include <bcmevent.h>
#include <bcmutils.h>
#include <wlc_utils.h>
#include <wlc_channel.h>
#include <wlc_iocv_types.h>
#include <wlc_iocv_desc.h>
#include <wlc_iocv_reg.h>
#include <wlc_rate.h>
#include <wlc_cfg.h>

/* Relocate attach symbols to save-restore region to increase pre-reclaim heap size. */
#ifdef SR_ATTACH_MOVE
	#undef BCMATTACHDATA
	#undef BCMATTACHFN

	#define BCMATTACHDATA(_data)	BCM_SRM_ATTACH_DATA(_data)
	#define BCMATTACHFN(_fn)	BCM_SRM_ATTACH_FN(_fn)
#endif /* SR_ATTACH_MOVE */

#ifdef HNDCTF
#include <ctf/hndctf.h>
#endif /* HNDCTF */

/* max # wlc timers. Each TDLS connection may dynamically allocate upto 2 timers */
#define	MAX_TIMERS	(34 + WLC_MAXMFPS + WLC_MAXDLS_TIMERS)
#define	MAXMULTILIST	32	/**< max # multicast addresses */

/* channel bandwidth */
#define WLC_10_MHZ	10	/**< 10Mhz channel bandwidth */
#define WLC_20_MHZ	20	/**< 20Mhz channel bandwidth */
#define WLC_40_MHZ	40	/**< 40Mhz channel bandwidth */
#define WLC_80_MHZ	80	/**< 80Mhz channel bandwidth */
#define WLC_160_MHZ	160	/**< 160Mhz channel bandwidth */

/* This macro is used for ratespec related initialization.
 */
#define CHSPEC_WLC_BW(chanspec)(\
				(CHSPEC_IS160(chanspec) ||\
				CHSPEC_IS8080(chanspec)) ? WLC_160_MHZ :\
				CHSPEC_IS80(chanspec) ? WLC_80_MHZ :\
				CHSPEC_IS40(chanspec) ? WLC_40_MHZ :\
							WLC_20_MHZ)

#define	WLC_RSSI_MINVAL		-200	/**< Low value, e.g. for forcing roam */
#define	WLC_RSSI_MINVAL_INT8	-128	/**< Low value fit for 8-bits */
#define	WLC_RSSI_NO_SIGNAL	-91	/**< NDIS RSSI link quality cutoffs */
#define	WLC_RSSI_VERY_LOW	-80	/**< Very low quality cutoffs */
#define	WLC_RSSI_LOW		-70	/**< Low quality cutoffs */
#define	WLC_RSSI_GOOD		-68	/**< Good quality cutoffs */
#define	WLC_RSSI_VERY_GOOD	-58	/**< Very good quality cutoffs */

#define	PREFSZ			160	/**< prefetch size */
#ifdef PKTC
#define WLPREFHDRS(h, sz)
#else
#define WLPREFHDRS(h, sz)	OSL_PREF_RANGE_ST((h), (sz))
#endif // endif

#ifndef LINUX_POSTMOGRIFY_REMOVAL

/* a large TX Power as an init value to factor out of MIN() calculations,
 * keep low enough to fit in an int8, units are .25 dBm
 */
#define WLC_TXPWR_MAX		(127)	/**< ~32 dBm = 1,500 mW */
#define ARPT_MODULES_MINTXPOWER	7	/**< Min txpower target for all X & M modules in dBm */
#define BCM94360_MINTXPOWER 1   /**< Min txpower target for 4360 boards in dBm */
#define WLC_NUM_TXCHAIN_MAX	4	/**< max number of chains supported by common code */

/* parameter struct for wlc_get_last_txpwr() */
typedef struct wlc_txchain_pwr {
	int8 chain[WLC_NUM_TXCHAIN_MAX];	/**< quarter dBm signed pwr for each chain */
} wlc_txchain_pwr_t;

/* legacy rx Antenna diversity for SISO rates */
#define	ANT_RX_DIV_FORCE_0		0	/**< Use antenna 0 */
#define	ANT_RX_DIV_FORCE_1		1	/**< Use antenna 1 */
#define	ANT_RX_DIV_START_1		2	/**< Choose starting with 1 */
#define	ANT_RX_DIV_START_0		3	/**< Choose starting with 0 */
#define	ANT_RX_DIV_ENABLE		3	/**< APHY bbConfig Enable RX Diversity */
#define ANT_RX_DIV_DEF		ANT_RX_DIV_START_0	/**< default antdiv setting */

#define	ANT_TX_DIV_FORCE_0		0	/* Use antenna 0 */
#define	ANT_TX_DIV_FORCE_1		1	/* Use antenna 1 */
#define	ANT_TX_DIV_START_1		2	/* Choose starting with 1 */
#define	ANT_TX_DIV_START_0		3	/* Choose starting with 0 */
#define ANT_TX_DIV_DEF		ANT_TX_DIV_START_0	/* default txant setting */

/* legacy rx Antenna diversity for SISO rates */
#define ANT_TX_FORCE_0		0	/**< Tx on antenna 0, "legacy term Main" */
#define ANT_TX_FORCE_1		1	/**< Tx on antenna 1, "legacy term Aux" */
#define ANT_TX_LAST_RX		3	/**< Tx on phy's last good Rx antenna */
#define ANT_TX_DEF			3	/**< driver's default tx antenna setting */

#define TXCORE_POLICY_ALL	0x1	/**< use all available core for transmit */

/* Tx Chain values */
#define TXCHAIN_DEF		0x1	/**< def bitmap of txchain */
#define TXCHAIN_DEF_NPHY	0x3	/**< default bitmap of tx chains for nphy */
#define TXCHAIN_DEF_ACPHY	0x7	/**< default bitmap of tx chains for acphy */
#define TXCHAIN_DEF_AC2PHY	0xf	/**< default bitmap of tx chains for ac2phy */
#define RXCHAIN_DEF		0x1	/**< def bitmap of rxchain */
#define RXCHAIN_DEF_NPHY	0x3	/**< default bitmap of rx chains for nphy */
#define RXCHAIN_DEF_ACPHY	0x7	/**< default bitmap of rx chains for acphy */
#define RXCHAIN_DEF_AC2PHY	0xf	/**< default bitmap of rx chains for ac2phy */

#define ANTSWITCH_NONE		0	/**< no antenna switch */
#define ANTSWITCH_TYPE_1	1	/**< antenna switch on 4321CB2, 2of3 */
#define ANTSWITCH_TYPE_2	2	/**< antenna switch on 4321MPCI, 2of3 */
#define ANTSWITCH_TYPE_3	3	/**< antenna switch on 4322, 2of3, SW only */
#define ANTSWITCH_TYPE_4	4	/**< antenna switch on 43234, 1of2, Core 1, SW only */
#define ANTSWITCH_TYPE_5	5	/**< antenna switch on 4322, 2of3, SWTX + HWRX */
#define ANTSWITCH_TYPE_6	6	/**< antenna switch on 43234, 1of2, SWTX + HWRX */
#define ANTSWITCH_TYPE_7	7	/**< antenna switch on 5356C0, 1of2, Core 0, SW only */

#define RXBUFSZ		PKTBUFSZ
#ifndef RXBUFSZ_STS
#define RXBUFSZ_STS     sizeof(d11phystshdr_t)
#endif // endif
#endif /* LINUX_POSTMOGRIFY_REMOVAL */

/* forward declare and use the struct notation so we don't have to
 * have it defined if not necessary.
 */
struct wlc_info;
struct wlc_hw_info;
struct wlc_bsscfg;
struct wlc_if;
struct wlc_d11rxhdr;

typedef struct wlc_tunables {
	int ntxd; /**< size of tx descriptor table for DMAs with 512 descriptor table support */
	int nrxd; /**< size of rx descriptor table for DMAs with 512 descriptor table support */
	int rxbufsz;			/**< size of rx buffers to post */
	int nrxbufpost;			/**< # of rx buffers to post */
	int maxscb;			/**< max # of STAs allowed to join */
	int ampdunummpdu2streams;	/**< max number of mpdu in an ampdu for 2 streams */
	int ampdunummpdu3streams;	/**< max number of mpdu in an ampdu for 3+ streams */
	int maxpktcb;			/**< max # of packet callbacks */
	int maxucodebss;		/**< max # of BSS handled in ucode bcn/prb */
	int maxucodebss4;		/**< max # of BSS handled in sw bcn/prb */
	int maxbss;			/**< max # of bss info elements in scan list */
	int datahiwat;			/**< data msg txq hiwat mark */
	int ampdudatahiwat;		/**< AMPDU msg txq hiwat mark */
	int rxbnd;			/**< max # of rx bufs to process before deferring to dpc */
	int txsbnd;			/**< max # tx status to process in wlc_txstatus() */
	int pktcbnd;			/**< max # of packets to chain */
	int dngl_mem_restrict_rxdma;	/**< memory limit for BMAC's rx dma */
	int pkt_maxsegs;
	int maxscbcubbies;		/**< max # of scb cubbies */
	int maxbsscfgcubbies;		/**< max # of bsscfg cubbies */
	int max_notif_servers;		/**< max # of notification servers. */
	int max_notif_clients;		/**< max # of notification clients. */
	int max_mempools;		/**< max # of memory pools. */
	int amsdu_resize_buflen;	/**< threshold for right-size amsdu subframes */
	int ampdu_pktq_size;		/**< max # of pkt from same precedence in an ampdu */
	int maxpcbcds;			/**< max # of packet class callback descriptors */
	int wlfcfifocreditac0;		/**< FIFO credits offered to host for AC 0 */
	int wlfcfifocreditac1;		/**< FIFO credits offered to host for AC 1 */
	int wlfcfifocreditac2;		/**< FIFO credits offered to host for AC 2 */
	int wlfcfifocreditac3;		/**< FIFO credits offered to host for AC 3 */
	int wlfcfifocreditbcmc;		/**< FIFO credits offered to host for BCMC */
	int wlfcfifocreditother;	/**< FIFO credits offered to host for Other */
	int wlfc_fifo_cr_pending_thresh_ac_bk;	/**< max AC_BK credits pending for push to host */
	int wlfc_fifo_cr_pending_thresh_ac_be;	/**< max AC_BE credits pending for push to host */
	int wlfc_fifo_cr_pending_thresh_ac_vi;	/**< max AC_VI credits pending for push to host */
	int wlfc_fifo_cr_pending_thresh_ac_vo;	/**< max AC_VO credits pending for push to host */
	int wlfc_fifo_cr_pending_thresh_bcmc;	/**< max BCMC credits pending for push to host */
	int wlfc_trigger;		/**< status/credit inidication trigger */
	int wlfc_fifo_bo_cr_ratio;	/**< total credits to max pending credits ratio */
	int wlfc_comp_txstatus_thresh;	/**< max pending compressed tx status count */

	int ntxd_large; /**< size of tx descriptor table for DMAs w/ 4096 descr table support */
	int nrxd_large; /**< size of rx descriptor table for DMAs w/ 4096 descr table support */
	int ntxd_bcmc;	/**< nr of bcmc tx descriptors */
	int ntxd_trig_max;	/**< Max size of tx descriptor table for Trigger DMAs queue */
	int ntxd_trig_min;	/**< Min size of rx descriptor table for Trigger DMAs queue */
	int maxubss;			/**< for dongle: limit for user (stored) scans list */
	int ampdunummpdu1stream;	/**< max number of mpdu in an ampdu for 2 streams */
	int max_keys;			/**< maximum number of (s/w) keys */
	int max_ie_build_cbs;		/**< max # IE-build callbacks */
	int max_vs_ie_build_cbs;	/**< max # VS IE-build callbacks */
	int max_ie_parse_cbs;		/**< max # IE-parse callbacks */
	int max_vs_ie_parse_cbs;	/**< max # VS IE-parse callbacks */
	int max_ie_regs;		/**< max # IE registries */
	int num_rxivs;
	int maxbestn;
	int maxmscan;
	int ntxd_lfrag;			/**< ntxd when lfrags are getting programmed */
	int nrxbufpost_fifo1;		/**< no of descriptors to be posted into fifo1 */
	int nrxbufpost_fifo2;		/**< no of descriptors to be posted into fifo2 */
	int nrxd_fifo1;			/**< no of rxd for fifo-1 */
	int maxroamthresh;	/**< max roam threshold tunable */
	int copycount;		/**< copycount for mode 3 and mode 4 */
	int bufpost_classified_fifo;	/**< no of descriptors to be posted into fifo1 */
	int nrxd_classified_fifo;		/**< no of rxd for fifo-1 */
	uint scan_settle_time;		/**< time (watchdog ticks) to block after a scan/roam */
	int min_scballoc_mem;		/**< min memory to allow scb alloc */
	int amsdu_rxpost_threshold;	/**< min rxpost required to enable rx_amsdu by default */
	int rpool_disabled_ampdu_mpdu;	/* ampdu_mpdu value when rpool is disabled */
	int max_wait_for_ctxt_delete;	/**< max wait to delete a stale chanenl context */
	int txmr;		/**< no. of outstanding reads */
	int txpft;		/**< tx prefetch threshold */
	int txpfc;		/**< tx prefetch control */
	int txblen;		/**< tx burst len */
	int rxpft;		/**< rx prefetch threshold */
	int rxpfc;		/**< rx prefetch threshold */
	int rxblen;		/**< rx burst len */
	int mrrs;		/**< max read request size */
	int evpool_maxdata;	/* max data size for events from the event pool */
	int evpool_size;	/* max pool size for the priority events */
	int max_assoc_scan_results;	/* Limit the max bss allocation in assoc/roam scan. */
	int max_scancache_results; /* Max scancache limit */
	int monrxbufsz;		/**< max size of rx buffers to recieve in monitor mode */
	int nrxd_sts;
	int bufpost_sts;
	int rxbufsz_sts;
	int max_maclist;	/**< Max size maclist array used to allow/deny stations */
} wlc_tunables_t;

#if defined(STA) && defined(DBG_BCN_LOSS)
struct wlc_scb_dbg_bcn {
	uint	last_rx;
	int	last_rx_rssi;
	int	last_bcn_rssi;
	uint	last_tx;
};
#endif /* defined(STA) && defined(DBG_BCN_LOSS) */

#ifndef LINUX_POSTMOGRIFY_REMOVAL
/*
 * buffer length needed for wlc_format_ssid
 * 32 SSID chars, max of 4 chars for each SSID char "\xFF", plus NULL.
 */
#ifndef SSID_FMT_BUF_LEN
#define SSID_FMT_BUF_LEN	((4 * DOT11_MAX_SSID_LEN) + 1)
#endif /* SSID_FMT_BUF_LEN */

#define RSN_FLAGS_SUPPORTED		0x1	/**< Flag for rsn_parms */
#define RSN_FLAGS_PREAUTH		0x2	/**< Flag for WPA2 rsn_parms */
#define RSN_FLAGS_FBT			0x4	/**< Flag for Fast BSS Transition */
#define RSN_FLAGS_MFPC			0x8	/**< Flag for MFP enabled */
#define RSN_FLAGS_MFPR			0x10	/**< Flag for MFP required */
#define RSN_FLAGS_SHA256		0x20	/**< Flag for MFP required */
#define RSN_FLAGS_PEER_KEY_ENAB		0x40	/**< Flags for Peer Key Enabled */
#define RSN_FLAGS_PMKID_COUNT_PRESENT	0x80	/**< PMKID count present */

/* All the HT-specific default advertised capabilities (including AMPDU)
 * should be grouped here at one place
 */
/* For MODE4 AMPDU_DEF_MPDU_DENSITY should be 8us when HWDEAGG is disabled. */
#ifndef AMPDU_DEF_MPDU_DENSITY
#define AMPDU_DEF_MPDU_DENSITY	AMPDU_DENSITY_4_US	/**< default mpdu density */
#endif /* AMPDU_DEF_MPDU_DENSITY */

/* defaults for the HT (MIMO) bss */
#define HT_CAP	((HT_CAP_MIMO_PS_OFF << HT_CAP_MIMO_PS_SHIFT) | HT_CAP_40MHZ | \
		  HT_CAP_GF | HT_CAP_MAX_AMSDU | HT_CAP_DSSS_CCK)
#endif /* LINUX_POSTMOGRIFY_REMOVAL */

/* WLC packet type is a void * */
typedef void *wlc_pkt_t;

/* Event data type */
struct wlc_event {
	wlc_event_t *next;		/**< enables ordered list of pending events */
	struct ether_addr *addr;	/**< used to keep a trace of the potential present of
					 * an address in wlc_event_msg_t
					 */
	wlc_if_t *wlcif;		/**< pointer to wlcif */
	void *data;			/**< used to hang additional data on an event */
	wl_event_msg_t event;		/**< encapsulated event */
};

/**
 * Stores scan result information (so information on an AP or IBSS), extracted from a beacon or
 * probe response. Information is not related to this local entity. AP information that is specific
 * to this local entity (e.g. association state) should be stored in struct wlc_bsscfg instead.
 */
struct wlc_bss_info {
	struct ether_addr BSSID;	/**< network BSSID */
	uint16		flags;		/**< flags for internal attributes */
	uint16		flags2;		/**< additional flags for internal attributes */
	uint16		flags3;		/**< additional flags for internal attributes */
	uint8		bcnflags;	/**< additional flags re: the beacon */
	uint8		SSID_len;	/**< the length of SSID */
	uint8		SSID[32];	/**< SSID string */
	int16		RSSI;		/**< receive signal strength (in dBm) */
	int16		SNR;		/**< receive signal SNR in dB */
	uint16		beacon_period;	/**< units are Kusec */
	uint16		atim_window;	/**< units are Kusec */
	chanspec_t	chanspec;	/**< Channel num, bw, ctrl_sb and band */
	int8		bss_type;	/**< DOT11_BSSTYPE_XXX except DOT11_BSSTYPE_ANY */
	uint8		accessnet;	/**< from beacon interwork IE (if bcnflags) */
	uint16		bcnlen;		/**< Length of bcn part of merged bcn_prb */
	wlc_rateset_t	rateset;	/**< supported rates */
	uint8		dtim_period;	/**< DTIM period */
	int8		phy_noise;	/**< noise right after tx (in dBm) */
	uint16		capability;	/**< Capability information */

	/* WLSCANCACHE */
	uint32		timestamp;	/**< in ms since boot, OSL_SYSUPTIME() */

	struct dot11_bcn_prb *bcn_prb;	/**< beacon/probe response frame (ioctl na) */
	uint16		bcn_prb_len;	/**< beacon/probe response frame length (ioctl na) */

	uint8		wme_qosinfo;	/**< QoS Info from WME IE; valid if WLC_BSS_WME flag set */

	struct rsn_parms wpa;
	struct rsn_parms wpa2;

	/* WLP2P || WLMCHAN */
	uint32		rx_tsf_l;	/**< usecs, rx time in local TSF */

	uint16		qbss_load_aac;	/**< qbss load available admission capacity */
	/* qbss_load_chan_free <- (0xff - channel_utilization of qbss_load_ie_t) */
	uint8		qbss_load_chan_free;	/**< indicates how free the channel is */

	uint8		rwan_links;	/**< reduced wan metrics: uplink and downlink */

	uint8		mcipher;	/**< multicast cipher */
	uint8		wpacfg;		/**< wpa config index */
	uint16		mdid;		/**< mobility domain id */

	/* WLTDLS */
	uint32		ext_cap_flags;
	uint8		ti_type;
	uint8		oper_mode;
	uint32		ti_val;
	uint8		anonce[32];
	uint8		snonce[32];
	uint8		mic[16];

	uint32		vht_capabilities;
	uint16		vht_rxmcsmap;
	uint16		vht_txmcsmap;
	uint16		vht_txmcsmap_prop;

	uint16		WPA_auth_support; /**< supporting WPA AKMs */
	/* Mesh protocol specific variables */
	uint8		activ_path_sel_prot_id;
	uint8		activ_path_sel_metric_id;
	uint8		cong_ctl_mode_id;
	uint8		sync_method_id;
	uint8		auth_prot_id;
	uint16		he_sup_bw80_tx_mcs;	/**< supported HE (11ax) 80Mhz tx mcs map */
	uint16		he_sup_bw80_rx_mcs;	/**< supported HE (11ax) 80Mhz rx mcs map */
	uint16		he_sup_bw160_tx_mcs;	/**< supported HE (11ax) 160Mhz tx mcs map */
	uint16		he_sup_bw160_rx_mcs;	/**< supported HE (11ax) 160Mhz rx mcs map */
	uint16		he_sup_bw80p80_tx_mcs;	/**< supported HE (11ax) 80+80Mhz tx mcs map */
	uint16		he_sup_bw80p80_rx_mcs;	/**< supported HE (11ax) 80+80Mhz rx mcs map */
	uint16		he_neg_bw80_tx_mcs;	/**< negotiated HE (11ax) 80Mhz tx mcs map */
	uint16		he_neg_bw80_rx_mcs;	/**< negotiated HE (11ax) 80Mhz rx mcs map */
	uint16		he_neg_bw160_tx_mcs;	/**< negotiated HE (11ax) 160Mhz tx mcs map */
	uint16		he_neg_bw160_rx_mcs;	/**< negotiated HE (11ax) 160Mhz rx mcs map */
	uint16		he_neg_bw80p80_tx_mcs;	/**< negotiated HE (11ax) 80+80Mhz tx mcs map */
	uint16		he_neg_bw80p80_rx_mcs;	/**< negotiated HE (11ax) 80+80Mhz rx mcs map */
	/* CCFS2 in recent most HT Info IE IEEE 802.11 REVmc D8.0 :: Table 9-168 */
	uint8		ht_ccfs2;

	uint8		eat_frac;	/**< estimated air time fraction ESP IE */
};

/* NDIS compatibility macro */
#define WLC_BSS_TYPE(bi)	(bi)->bss_type

/* wlc_bss_info flag bit values */
#define WLC_BSS_54G             0x0001  /**< BSS is a legacy 54g BSS */
#define WLC_BSS_RSSI_ON_CHANNEL 0x0002  /**< RSSI measurement was from the same channel as BSS */
#define WLC_BSS_WME             0x0004  /**< BSS is WME capable */
#define WLC_BSS_BRCM            0x0008  /**< BSS is BRCM */
#define WLC_BSS_WPA             0x0010  /**< BSS is WPA capable */
#define WLC_BSS_HT              0x0020  /**< BSS is HT (MIMO) capable */
#define WLC_BSS_40MHZ           0x0040  /**< BSS is 40MHZ capable */
#define WLC_BSS_WPA2            0x0080  /**< BSS is WPA2 capable */
#define WLC_BSS_BEACON          0x0100  /**< bss_info was derived from a beacon */
#define WLC_BSS_40INTOL         0x0200  /**< BSS is forty intolerant */
#define WLC_BSS_SGI_20          0x0400  /**< BSS supports 20MHz SGI */
#define WLC_BSS_SGI_40          0x0800  /**< BSS supports 40MHz SGI */
/**< 0x1000 is unused */
#define WLC_BSS_CACHE           0x2000  /**< bss_info was collected from scan cache */
#define WLC_BSS_FBT             0x8000  /**< BSS is FBT capable */

/* additional wlc_bss_info flag bit values (flags2 field) */
#define WLC_BSS_OVERDS_FBT      0x0001  /**< BSS is Over-the-DS FBT capable */
#define WLC_BSS_VHT             0x0002  /**< BSS is VHT (802.11ac) capable */
#define WLC_BSS_80MHZ           0x0004  /**< BSS is VHT 80MHz capable */
#define WLC_BSS_SGI_80		0x0008	/**< BSS supports 80MHz SGI */
#define WLC_BSS_HS20		0x0010	/**< BSS is hotspot 2.0 capable */
#define WLC_BSS_RSSI_INVALID	0x0020	/**< BSS contains invalid RSSI */
#define WLC_BSS_RSSI_INACCURATE	0x0040	/**< BSS contains inaccurate RSSI */
#define WLC_BSS_8080MHZ		0x0080  /**< BSS is VHT 80+80 MHz capable */
#define WLC_BSS_160MHZ		0x0100  /**< BSS is VHT 160 MHz capable */
#define WLC_BSS_SGI_160		0x0200  /**< BSS supports 160MHz SGI */
#define WLC_BSS_MAX_STA_CNT	0x0400	/**< BSS load IE has max sta count */
#define WLC_BSS_OPER_MODE	0x0800  /* BSS supports operating mode notification */
#define WLC_BSS_OCE_ASSOC_REJD  0x1000  /* OCE Assoc Rejected in last received Assoc Resp */

/* bit values for bcnflags field */
#define WLC_BSS_INTERWORK_PRESENT	0x01 /* interwork IE was present in last bcn recvd */
#define WLC_BSS_MBO_CAPABLE		0x02 /* MBO AP Capability in last bcn/probe resp recvd */
#define WLC_BSS_MBO_ASSOC_DISALLOWED	0x04 /* MBO Assoc Disallowed in last bcn/probe resp recvd */

/* additional wlc_bss_info flag bit values (flags3 field) */
#define WLC_BSS3_HE		0x0001	/* BSS is HE (802.11ax) capable */

#define BAR0_INVALID		(1 << 0)
#define VENDORID_INVALID	(1 << 1)
#define NOCARD_PRESENT		(1 << 2)
#define PHY_PLL_ERROR		(1 << 3)
#define DEADCHIP_ERROR		(1 << 4)
#define MACSPEND_TIMOUT		(1 << 5)
#define MACSPEND_WOWL_TIMOUT	(1 << 6)
#define DMATX_ERROR		(1 << 7)
#define DMARX_ERROR		(1 << 8)
#define DESCRIPTOR_ERROR	(1 << 9)
#define CARD_NOT_POWERED	(1 << 10)

#define WL_HEALTH_LOG(w, s)	do {} while (0)

/* watchdog/init/down callback function proto's */
typedef void (*watchdog_fn_t)(void *handle);
typedef int (*up_fn_t)(void *handle);
typedef int (*down_fn_t)(void *handle);

#define MU_FEATURES_MUTX	(1 << 0)
#define MU_FEATURES_MURX	(1 << 1)
#define MU_FEATURES_AUTO	(1 << 15)
#define MU_FEATURES_OFF		0

#ifdef PKTC_TBL
struct wl_pktc_tbl;
#endif // endif
/*
 * Public portion of "common" os-independent state structure.
 * The wlc handle points at this.
 */
typedef struct wlc_pub {
	wlc_info_t *wlc;
	struct wlc_pub_cmn *cmn; /**< Common PUB variables across WLCs. */
	struct ether_addr	cur_etheraddr;	/**< our local ethernet addr, must be aligned */
	uint		unit;			/**< device instance number */
	uint		corerev;		/**< core revision */
	osl_t		*osh;			/**< pointer to os handle */
	bool		up;			/**< interface up and running */
	bool		hw_off;			/**< HW is off */
	wlc_tunables_t *tunables;		/**< tunables: ntxd, nrxd, maxscb, etc. */
	bool		hw_up;		/**< one time hw up/down(from boot or hibernation) */
	bool		_piomode;	/**< true if pio mode */ /* BMAC_NOTE: NEED In both */
	uint		now;			/**< # elapsed seconds */
	uint		_bandmask;		/**< bitmask indicating supported bands */
	bool		promisc;		/**< promiscuous destination address */
	bool		delayed_down;		/**< down delayed */
	/* TOE */
	bool		_toe;			/**< TOE mode enabled */
	/* ARPOE */
	bool		_arpoe;			/**< Arp agent offload enabled */
	bool		_ap;			/**< AP mode enabled */
	bool		_apsta;			/**< simultaneous AP/STA mode enabled */
	bool		_apcs;			/**< APCS enabled */
	bool		_assoc_recreate;	/**< association recreation on up transitions */
	int		_wme;			/**< WME QoS mode */
	uint8		_mbss_mode;		/**< MBSS mode */
	bool		_mbss_support;		/**< MBSS is supported or not */
	/* WLDLS */
	bool		_dls;			/**< dls enabled or not */
	/* WLMCNX */
	bool		_mcnx;			/**< multiple connection ucode ucode used or not */
	/* WLP2P */
	bool		_p2p;			/**< p2p enabled or not */
	/* WLMCHAN */
	bool		_mchan;			/**< multi channel enabled or not */
	bool		_mchan_active;		/**< multi channel active or not */
	/* PSTA */
	int8		_psta;			/**< Proxy STA mode: disabled, proxy, repeater */
	int8		_pktc;			/**< Packet chaining enabled or not */
	int8		_pktc_tx;		/**< Packet chaining enabled or not in WL for TX */
	bool		associated;		/**< true:part of [I]BSS, false: not */
						/**< (union of stas_associated, aps_associated) */
	bool            phytest_on;             /**< whether a PHY test is running */
	bool		bf_preempt_4306;	/**< True to enable 'darwin' mode */
	/* GTKOE */
	bool		_gtkoe;			/* GTK Offload enable */
	/* WOWL */
	bool		_wowl;			/**< wowl enabled or not (in sw) */
	bool		_wowl_active;		/**< Is Wake mode actually active
						 * (used during transition)
						 */
	/* WLAMPDU */
	bool		_ampdu_tx;		/**< ampdu_tx enabled for HOST, UCODE or HW aggr */
	bool		_ampdu_rx;		/**< ampdu_rx enabled for HOST, UCODE or HW aggr */
	/* WLAMSDU || WLAMSDU_TX */
	bool		_amsdu_tx;		/**< true if currently amsdu agg is enabled */
	bool		_ccx;			/**< enable: ccx */
	/* WL11H */
	uint		_spect_management;	/**< 11h spectrum management */
	/* WL11K */
	bool		_rrm;			/**< 11k radio resource measurement */
	/* WLBSSLOAD */
	bool		_bssload;		/**< bss load IE */
	bool		_mfp;			/**< mfp enabled or not */
	uint8		_n_enab;		/**< bitmap of 11N + HT support */
	bool		_n_reqd;		/**< N support required for clients */
	uint8		_vht_enab;		/**< VHT (11AC) support */
	int8		_coex;			/**< 20/40 MHz BSS Management AUTO, ENAB, DISABLE */
	bool		_priofc;		/**< Priority-based flowcontrol */
	bool		phy_bw40_capable;	/**< PHY 40MHz capable */
	bool		phy_bw80_capable;	/**< PHY 80MHz capable */
	bool		phy_bw160_capable;	/**< PHY 160MHz capable */
	bool		phy_bw8080_capable;	/**< PHY 80+80MHz capable */

#ifdef HNDCTF
	ctf_brc_hot_t *brc_hot;			/**< hot ctf bridge cache entry */
#else
	void		*PAD;			/**< increased ROM compatibility */
#endif /* HNDCTF */

	uint32		wlfeatureflag;		/**< Flags to control sw features from registry */

	int             psq_pkts_total;         /**< total num of ps pkts */

	uint16		txmaxpkts;	/**< max number of large pkts allowed to be pending */

	/* s/w decryption counters */
	uint32		swdecrypt;		/**< s/w decrypt attempts */

	int 		bcmerror;		/**< last bcm error */

	mbool		radio_disabled;		/**< bit vector for radio disabled reasons */
	mbool		last_radio_disabled;	/**< radio disabled reasons for previous state */
	bool		radio_active;		/**< radio on/off state */
	/* WME_PER_AC_TX_PARAMS */
	bool		_per_ac_maxrate;	/**< Max TX rate per AC */

	/* BCM_BLOG */
	bool		fcache;			/**< 1-- enable; 0--disable */

	/* STA */
	bool		align_wd_tbtt;		/**< Align watchdog with tbtt indication
						 * handling. This flag is cleared by default
						 * and is set by per port code explicitly and
						 * you need to make sure the OSL_SYSUPTIME()
						 * is implemented properly in osl of that port
						 * when it enables this Power Save feature.
						 */

	uint16		boardrev;		/**< version # of particular board */
	uint8		sromrev;		/**< version # of the srom */
	uint32		boardflags;		/**< Board specific flags from srom */
	uint32		boardflags2;		/**< More board flags if sromrev >= 4 */
	uint32		boardflags4;		/**< More board flags if sromrev >= 12 */
	/* WLCNT */
	wl_cnt_wlc_t	*_cnt;			/**< monolithic counters struct */
	reinit_rsns_t	*reinitrsn;
	void		*_mcst_cnt;		/**< macstat count from SHM.
						 * To be cast to either wl_cnt_ge40mcst_v1_t or
						 * wl_cnt_lt40mcst_v1_t based on corerev
						 */
	/* WL_EAP_STATS */
	wl_cnt_eap_t   *_eap_cnt;		/**< EAP counters struct */
	wl_wme_cnt_t	*_wme_cnt;		/**< Counters for WMM */

	bool		_pkt_filter;		/**< pkt filter enable bool */
	bool		_fbt;			/* Fast Bss Transition active */
	bool		_assoc_mgr;		/**< enable/disable fine grained assoc control */
	/* WLPLT */
	bool		_plt;			/**< PLT module included and enabled */

	pktpool_t	*pktpool;		/**< use the pktpool for buffers */
	uint8		_ampdumac;	/**< mac assist ampdu enabled or not */

	/* IBSS_PEER_GROUP_KEY */
	bool		_ibss_peer_group_key;
	/* IBSS_PEER_DISCOVERY_EVENT */
	bool		_ibss_peer_discovery_event;
	/* IBSS_PEER_MGMT */
	bool		_ibss_peer_mgmt;
	/* WLNOEIND */
	bool		_wleind;
	/* WLRXOV */
	bool		_rxov;
	/* BCMAUTH_PSK */
	bool		_bcmauth_psk;
	/* PROP_TXSTATUS */
	bool		_proptxstatus;
	/* BCMSUP_PSK */
	bool		_sup_enab;
	/* WL11U */
	bool		_11u;
	/* BPRESET */
	bool		_bpreset;
	/* WLPROBRESP_SW */
	bool		_probresp_sw;
	bool		_wapi_hw_wpi;	/**< HW WAPI_WPI enabled or not */

	uint32		ht_features;            /**< 802.11n proprietary rates */

	uint8       max_modules;
	/* WLNDOE */
	bool		_ndoe;		/**< Neighbor Advertisement Offload */
	/* WOWLPF */
	bool		_wowlpf;			/**< wowlpf enabled or not (in sw) */
	bool		_wowlpf_active;		/**< Is Wake mode actually active
						 * (used during transition)
						 */
	uint8		d11tpl_phy_hdr_len;	/**< PLCP len for templates */
	int		max_addrma_idx;
	uint16		m_amt_info_blk;		/**< base address of amt info block */
	uint16		m_coremask_blk;
	uint16		m_coremask_blk_wowl;
	/* WET_TUNNEL */
	bool		wet_tunnel;	/**< true if wet_tunnel is enable */
	int		_ol;			/**< Offloads */

	/* WL11AC */
	uint16		vht_features;		/**< BRCM proprietary VHT features  bitmap */
	/* WLAMPDU_HOSTREORDER */
	bool		_ampdu_hostreorder;

	si_t		*sih;			/**< SB handle (cookie for siutils calls) */
	char		*vars;			/**< "environment" name=value */
	uint		vars_size;		/**< size of vars, free vars on detach */

	/* ARP offload supported. */
	bool		_arpoe_support;
	/* Flag to runtime enable/disable LPC */
	bool            _lpc_algo;

	bool		_l2_filter;		/**< L2 filter enabled or not */

	bool		_rmc;		/**< reliable multicast GO enabled */
	bool		_rmc_support;	/**< reliable multicast GC enabled */
	uint		bcn_tmpl_len;		/**< len for beacon template */

	bool		_p2po;			/**< p2po enabled or not */
	bool		_anqpo;			/**< anqpo enabled or not */

	bool		_d0_filter;		/**< pkt filter enable bool */
	bool		_olpc;			/**< Open Loop Phy Calibration enable/disable */
	bool		_staprio;		/**< sta prio enable bool */
	/**
	 * Since code contained in ROM must be able to dynamically use SRVSDB functionality, ROM
	 * code checks this member variable. Variable is written once at startup/attach, its value
	 * does not change after that.
	 */
	bool		_wlsrvsdb;		/**< hardware assisted vsdb enable/disable */
	bool		_stamon;		/**< STA monitor feature. */
	bool		_wl_rxearlyrc;
	bool		_tiny_pktjoin;
	pktpool_t	*pktpool_lfrag;		/**< use the pktpool for buffers */
	bool		_fmc;			/**< WLFMC_ENAB enabled or not */
	bool		_rcc;			/**< WLRCC_ENAB enabled or not */

	/* WL_BEAMFORMING */
	bool		_txbf;

	pktpool_t	*pktpool_rxlfrag;		/**< use the pktpool for rx  buffers */
	pktpool_t	*pktpool_utxd;		/**< use the pktpool for utxd  */
	bool		_wlstats;		/**< wlc statistics */
	bool		_amsdu_tx_support;	/**< true if amsdu tx agg is supported */
	bool		_clear_restricted_chan;	/**< clear restricted channels on rx frames */
	bool		scanresults_on_evtcplt; /**< scan results tag with scan cplt event */
	bool		_evdata_bssinfo;	/**< WLC event data bssinfo enable */
	bool		_net_detect;		/**< NetDetect enable/disable */
	bool		_prot_obss;		/**< OBSS protection */
	bool		_obss_dynbw;		/**< OBSS protection */
	bool		_pwrstats;		/**< additional power stats supported */
	uint            pending_now;            /**< To correct delayed now due to scan */
	bool		_proxd;			/**< proximity detection enabled or not */
	bool            _rm;                    /**< enable radio measurement */
	bool		_abt;			/**< Adaptive Beacon Timeout */
	bool		_scan_ps;		/**< pwr optimization for scan */
	bool		_nfc;			/**< Secure WiFi through NFC */
	bool		_bssload_report;	/**< Get/report bssload from beacon */
	bool		_wloverthruster;	/**< Overthruster enable/disable */
	bool		_ltecx;			/**< LTE coex enabled */
	bool		_ltecxgci;		/**< LTE Coex GCI Support Present */
	bool		_ccx_aka;		/**< CCX AP_KEEP_ALIVE supported */
	bool		_osen;			/**< hotspot OSEN */
	bool		_bwte;			/**< true if bt wlan tunnel engine supported */
	bool		_wfds;			/**< wfds enabled or not */
	bool		_ltr_support;	/**< LTR supported by platform */
	bool		_ltr;			/**< LTR cap enabled/disabled */
	bool		_ht_prop_rates_capable;	/**< 802.11n proprietary rates */
	bool		_tbow;			/**< true if tunnel bt over wlan supported */
	bool		_modesw; 		/**< modeswitch */
	bool		_metadata_to_host; /**< To create TLVs to be sent to host */
	bool		_credit_info_update; /**< To enable credit related processing */
	bool		_rxmetadata_to_host; /**< TLVs to be sent to host for rxed pkts */
	bool		_txmetadata_to_host; /**< TLVs to be sent to host for txed pkts */
	bool		_wlfc_ctrl_to_host;  /**< TLVs to be sent to host for flow control info */
	bool		_wlfc_info_to_bus; /**< TLVs/byte streams to be sent to bus for wlfcinfo */
	bool		_dynbw;			/**< Dynamic bandwidth enabled or not */
	bool		_bcntrim;		/**< Beacon Trim */
	bool		_wnm_brcm;      /**< brcm-proprietary 11v capabilities signaling */
	bool		_smfs;
	bool		_pkt_filter2;	/**< pkt filter2 enable bool */
	bool		_link_stats;		/**< link layer statistics */
	uint32		wake_event_enable;	/**< bitwise event flag */
	uint32		wake_event_status;	/**< bitwise event status(reason for the wake) */
	bool		_mesh;	/* Mesh support enable/disable flag */
	bool		_ucodedump;	/* enable/disable u code dump */
	bool		_ulp;
	bool		_cxnoa;         /* Extend NoA feature for synchronous BT traffic */
	bool		_bdo_support;	/* bdo supported */
	bool		_bdo;		/* bdo enabled or not */
	bool		_fccpwrlimit2g; /* FCC power limit control on ch12/13 */
	bool		_btcdyn;	/* BTCOEX Dynamic control */
	bool		_auth_shared_open;	/**< Open/Shared-Key Authentication Method */
	bool		_randmac;			/* randmac method enable/disable */
	wl_rxfifo_cnt_t *_rxfifo_cnt;	/**< Counters for packets in RXFIFOs */
	bool            _asdb;          /*  ASDB enabled or not */
	bool		_nap;           /* Napping supported */
	bool		_pspretend;		/* PSPRETEND enabled */

	/* WL11AX */
	bool		_he_enab;		/**< HE (11AX) support */
	uint8		_he_fragtx;             /**< HE fragment TX enable */
	uint32		he_features;            /**< 11ax proprietary features */

	/* WLTWT */
	bool		_twt;			/**< 11ax/11ah Target Wake Time support */

	bool		_mac_clkgating;		/* Enable MAC clockgating */
	pktpool_t	*pktpool_resv_lfrag;	/* use this pktpool for resv tx buffers */
	resv_info_t	*resv_pool_info;	/* resv pool info for (scbs or tx_pkts) */
	bool		_bsstrans_ext; /* 802.11 K/V/R Extension support */
	bool		_bsstrans_ext_active;	/* set to true when product policy is enabled */
	bool		_rsdb_pm_modesw;	/* Dual MAC modeswitch */
	bool		_swdiv;			/* enable/disable SW diversity */
	bool		_txpwrcap;		/* enable/disable TXpowercap */
	bool		_tsync;			/* Timesync enabled/disabled */
	bool		_stf_arb;		/* STF Arbitrator */
	bool        _stf_mimops;	/* MIMOPS */
	bool		_ocl;
	bool		_mon;			/* monitor mode */
	void		*_mcxst_cnt;		/* mcx count from SHM X. */
	uint32		mu_features;
	bool		_dyn160;		/* DYN160 capability - dynamic 160/80p80 <-> 80 */
	uint32		_dyn160_active;		/* DYN160 active - dynamic 160/80p80 <-> 80 */
	struct poolreorg_info_s *poolreorg_info; /* info used by pkt forwarding offload features */
	bool		_dts;			/* SmartAmpdu (DTS tx suppression) */
	bool		_bgdfs;			/* Background DFS with radar scan core */
	bool            _amsdu_2g;              /* AMSDU 2G dynamic control */
	bool		_chanim;		/* enable/disable chanim */
	/* APF */
	bool		_apf_filter;
	bool            _pkt_filter6;		/* pkt filter6 enable bool */
	bool		_cca_stats;		/* enable/disable cca_stats */
	bool		_deltastats;		/* enable/disable deltastats */
	uint8		corerev_minor;		/**< core minor revision */
	bool _ndoe_support;	/* NDOE offload supported. */
	bool		_vasip;			/* enable/disable vasip */
	bool            _smc;                   /* enable/disable smc */
	bool            _icmp;                  /* icmp enable */
	bool		_ucm;			/* enable/disable UCM */
	bool		_leakyapstats;		/* LeakyAPStats enabled */
	bool		_psmx;			/* true if PSMX feature is enabled */
	bool		_auxpmq;		/* true if AUXPMQ feature is enabled */
	bool		_ratelinkmem;		/* RateLinkMem enabled */
	bool        _tvpm;			/**< TVPM mitigation supported */
	bool		_tsync_active;		/* Timesync active/inactive */
	bool		_tdmtx;			/* TDM Tx enable flag */
	bool            _heb;                   /* HEB block support */
	bool		_ops;			/* ops */
#if defined(PKTC_TBL) || defined(BCM_PKTFWD)
	union {
#ifdef PKTC_TBL
		struct wl_pktc_tbl *pktc_tbl;	/* PKTC_TBL Table for cached Packet Chaining info */
#endif // endif
#ifdef BCM_PKTFWD
		void    *d3lut;		/* BCM_PKTFWD && BCM_D3LUT */
#endif // endif
	};
#endif /* PKTC_TBL || BCM_PKTFWD */
#if defined(BCM_AWL)
	void		*awl_cb;		/* Archer Wireless Control Block */
#endif /* BCM_AWL */
	bool		_psmr1;			/* true if PSMR1 is enabled */
	uint32		_cevent;		/* cevent state */
	bool		_sts_rxen;		/**< PhyRxStatus over D11RX FIFO 3 */
	bool		_sts_offld_rxen;	/**< PhyRxStatus over M2M/BME */
	uint32		_dis_ch_grp_conf;	/* config bitmap of ch groups disabled by nvram */
	uint32		_dis_ch_grp_user;	/* user bitmap of ch groups disabled by iovar */

	bool		_fbt_cap;		/* Fast Bss Transition capable */
	bool		_traffic_thresh;	/* txfail threshold events disabled by iovar */
	int32		_bw_switch_160;		/* BW switch 80/160Mhz control by IOVAR */
	bool		_cfp;			/* CFP enabled */
	bool		_bgdfs_2g;		/* Background DFS radar scan on 2G */
	bool		_dtpc;			/**< dynamic tx power control enable flag */
	bool		_mbss_ign_mac_valid;	/* Ignore validation/auto-generation of MBSS MACs */
	bool		_wrr_adaptive;			/* Adaptive round robin weights for uCode */
	bool            _csimon;                /* Channel State Info Monitor feature */

	/* ULOFDMA */
	pktpool_t	*utxd_pool;	/* utxd buf pool */
} wlc_pub_t;

/** Shared portion of wlc_pub structure across WLC's in case of RSDB. */
typedef struct wlc_pub_cmn {
	uint		driverrev;		/**< to make wlc_get_revision_info rommable */
#ifdef WLRSDB
	bool		_isrsdb;		/**< Set to true if this chip is RSDB capable. */
	bool		_rsdb_active;	/**< Set to true when both MAC's have
								 * active connections.
								 */
#endif /* WLRSDB */
	bool            _mu_rx;         /* true if phy is MU-MIMO receive capable and
					 * MU RX is enabled by configuration
					 */
	/* WL11H */
	bool		_11h;
	/* WL11D */
	bool		_11d;
	/* WLCNTRY */
	bool		_autocountry;
	bool	_rsdb_policy; /* host provided policy support. configured through rsdb_config */
	bool		_shub;		/* sensor hub interface */
	bool		 _wlpfn;		/* WLPFN */
	bool		 _gscan;		/* gscan */
	/* WLTDLS */
	bool		_tdls;			/**< tdls enabled or not */
	bool		_natoe;         /* enable/disable NATOE */
	bool		_natoe_active;  /* set to true when all the NATOE ioctls received */
	bool            _radar;         /* Radar is supported or not */
	bool            _wldfs;         /* DFS for AP is supported or not */
	bool            _rsdb_dfs;      /* DFS support for RSDB is supported or not */
	bool		_mbo;       /* mbo */
	bool            _isrsdb_cmn_bandstate; /* Bandstate shared for RSDB chips */
	/* WL_OKC */
	bool		_okc;
	uint8		_rsdb_scan; /* Downgraded RSDB Scan feature enabled or not */
	bool		_rmon;		/* rssi monitor */
	bool		_cac;			/**< 802.11e CAC enabled */
	bool		_scancache_support;
	uint32		drvrev_major;	/* drvrev: major */
	uint32		drvrev_minor;	/* drvrev: minor */
	uint32		drvrev_rc;	/* drvrev: rc */
	uint32		drvrev_rc_inc;	/* drvrev: rc inc */
	bool		_fils;		/* FILS for OCE */
	bool		_keep_alive;	/* enable/disable keep-alive */
	bool		_oce;		/* oce */
	bool		_scan_sumevt;	/* Scan_summary statistics */
	bool		_nan;		/**< NAN enabled */
	bool		_mbo_oce;       /* mbo-oce enabled or not */
	/* WLWNM */
	bool		_wnm;		/**< 11v wireless netwrk management */
	bool		_pm_bcnrx;	/**< PM single core beacon rx */
	bool		_bam_enable;
	bool		_health_check;	/* capture health check enable state */
	bool		_rpsnoa;	/* Radio power save with NOA */
	bool		_esp;		/* esp enabled or not */
	bool		_proxd_ucode_tsync; /* tsync based proxd */
} wlc_pub_cmn_t;

/* status per error RX pkt */
#define WL_RXS_CRC_ERROR	0x00000001 /**< CRC Error in packet */
#define WL_RXS_RUNT_ERROR	0x00000002 /**< Runt packet */
#define WL_RXS_ALIGN_ERROR	0x00000004 /**< Misaligned packet */
#define WL_RXS_OVERSIZE_ERROR	0x00000008 /**< packet bigger than RX_LENGTH (usually 1518) */
#define WL_RXS_WEP_ICV_ERROR	0x00000010 /**< Integrity Check Value error */
#define WL_RXS_WEP_ENCRYPTED	0x00000020 /**< Encrypted with WEP */
#define WL_RXS_PLCP_SHORT	0x00000040 /**< Short PLCP error */
#define WL_RXS_DECRYPT_ERR	0x00000080 /**< Decryption error */
#define WL_RXS_OTHER_ERR	0x80000000 /**< Other errors */

/* phy type */
#define WL_RXS_PHY_A			0x00000000 /**< A phy type */
#define WL_RXS_PHY_B			0x00000001 /**< B phy type */
#define WL_RXS_PHY_G			0x00000002 /**< G phy type */
#define WL_RXS_PHY_N			0x00000004 /**< N phy type */

/* encoding */
#define WL_RXS_ENCODING_UNKNOWN		0x00000000
#define WL_RXS_ENCODING_DSSS_CCK	0x00000001 /**< DSSS/CCK encoding (1, 2, 5.5, 11) */
#define WL_RXS_ENCODING_OFDM		0x00000002 /**< OFDM encoding */
#define WL_RXS_ENCODING_HT		0x00000003 /**< HT encoding */
#define WL_RXS_ENCODING_VHT		0x00000004 /**< VHT encoding */
#define WL_RXS_ENCODING_HE		0x00000005 /**< HE encoding */

/* preamble */
#define WL_RXS_UNUSED_STUB		0x0		/**< stub to match with wlc_ethereal.h */
#define WL_RXS_PREAMBLE_SHORT		0x00000001	/**< Short preamble */
#define WL_RXS_PREAMBLE_LONG		0x00000002	/**< Long preamble */
#define WL_RXS_PREAMBLE_HT_MM		0x00000003	/**< HT mixed mode preamble */
#define WL_RXS_PREAMBLE_HT_GF		0x00000004	/**< HT green field preamble */

/* htflags */
#define WL_RXS_HTF_BW_MASK		0x07
#define WL_RXS_HTF_40			0x01
#define WL_RXS_HTF_20L			0x02
#define WL_RXS_HTF_20U			0x04
#define WL_RXS_HTF_SGI			0x08
#define WL_RXS_HTF_STBC_MASK		0x30
#define WL_RXS_HTF_STBC_SHIFT		4
#define WL_RXS_HTF_LDPC			0x40

#ifdef WLTXMONITOR
/* reuse bw-bits in ht for vht */
#define WL_RXS_VHTF_BW_MASK		0x87
#define WL_RXS_VHTF_40			0x01
#define WL_RXS_VHTF_20L			WL_RXS_VHTF_20LL
#define WL_RXS_VHTF_20U			WL_RXS_VHTF_20LU
#define WL_RXS_VHTF_80			0x02
#define WL_RXS_VHTF_20LL		0x03
#define WL_RXS_VHTF_20LU		0x04
#define WL_RXS_VHTF_20UL		0x05
#define WL_RXS_VHTF_20UU		0x06
#define WL_RXS_VHTF_40L			0x07
#define WL_RXS_VHTF_40U			0x80
#endif /* WLTXMONITOR */

/* vhtflags */
#define WL_RXS_VHTF_STBC		0x01
#define WL_RXS_VHTF_TXOP_PS		0x02
#define WL_RXS_VHTF_SGI			0x04
#define WL_RXS_VHTF_SGI_NSYM_DA		0x08
#define WL_RXS_VHTF_LDPC_EXTRA		0x10
#define WL_RXS_VHTF_BF			0x20
#define WL_RXS_VHTF_DYN_BW_NONHT 	0x40
#define WL_RXS_VHTF_CODING_LDCP		0x01

#define WL_RXS_VHT_BW_20		0
#define WL_RXS_VHT_BW_40		1
#define WL_RXS_VHT_BW_20L		2
#define WL_RXS_VHT_BW_20U		3
#define WL_RXS_VHT_BW_80		4
#define WL_RXS_VHT_BW_40L		5
#define WL_RXS_VHT_BW_40U		6
#define WL_RXS_VHT_BW_20LL		7
#define WL_RXS_VHT_BW_20LU		8
#define WL_RXS_VHT_BW_20UL		9
#define WL_RXS_VHT_BW_20UU		10
#define WL_RXS_VHT_BW_160		11
#define WL_RXS_VHT_BW_80L		12
#define WL_RXS_VHT_BW_80U		13
#define WL_RXS_VHT_BW_40LL		14
#define WL_RXS_VHT_BW_40LU		15
#define WL_RXS_VHT_BW_40UL		16
#define WL_RXS_VHT_BW_40UU		17
#define WL_RXS_VHT_BW_20LLL		18
#define WL_RXS_VHT_BW_20LLU		19
#define WL_RXS_VHT_BW_20LUL		20
#define WL_RXS_VHT_BW_20LUU		21
#define WL_RXS_VHT_BW_20ULL		22
#define WL_RXS_VHT_BW_20ULU		23
#define WL_RXS_VHT_BW_20UUL		24
#define WL_RXS_VHT_BW_20UUU		25

#define WL_RXS_NFRM_AMPDU_FIRST		0x00000001 /**< first MPDU in A-MPDU */
#define WL_RXS_NFRM_AMPDU_SUB		0x00000002 /**< subsequent MPDU(s) in A-MPDU */
#define WL_RXS_NFRM_AMSDU_FIRST		0x00000004 /**< first MSDU in A-MSDU */
#define WL_RXS_NFRM_AMSDU_SUB		0x00000008 /**< subsequent MSDU(s) in A-MSDU */

#define WL_TXS_TXF_FAIL		0x01	/**< TX failed due to excessive retries */
#define WL_TXS_TXF_CTS		0x02	/**< TX used CTS-to-self protection */
#define WL_TXS_TXF_RTSCTS 	0x04	/**< TX used RTS/CTS */

#ifndef LINUX_POSTMOGRIFY_REMOVAL
/* Structure for Pkttag area in a packet.
 * CAUTION: Please carefully consider your design before adding any new fields to the pkttag
 * The size is limited to 32 bytes which on 64-bit machine allows only 4 fields.
 * If adding a member, be sure to check if wlc_pkttag_info_move should transfer it.
 */
typedef struct {
	uint32		flags;		/**< Describe various packet properties */
	uint16		seq;		/**< preassigned seqnum for AMPDU */
	uint8		flags2;		/**< Describe various packet properties */
	uint8		flags3;		/**< Describe various packet properties */
	union {
#ifdef DONGLEBUILD
		uint8	callbackidx;	/**< Index into pkt_callback tables for callback function */
		uint8	rxchannel;	/**< Control channel of the received packet */
#else
		uint16	callbackidx;	/**< Index into pkt_callback tables for callback function */
		uint16	rxchannel;	/**< Control channel of the received packet */
#endif // endif
	};

	int8		_bsscfgidx;	/**< Index of bsscfg for this frame */
#ifndef DONGLEBUILD
	int8		pad0;
#endif // endif
	union {
		struct {
			int8	snr;	/**< SNR for the recvd. packet */
			int8	rssi;	/**< RSSI for the recvd. packet */
		} misc;
		union {
			uint16 pkt_len;
			uint16 pkt_time;
		} atf;
#ifdef WLTAF
		struct {
			union {
				struct {
					uint16 tagged:1;
					uint16 tag:15;
				} def;
				struct {
					uint16 index:3;
					uint16 units:13;
				} ias;
			};
		} taf;
#endif /* WLTAF */
	} pktinfo;
#ifndef DONGLEBUILD
	int8	pad1[6];
#endif // endif
	union {
		uint32		exptime;	/**< Time of expiry for the packet */
#if defined(BCMPKTIDMAP)
		struct wlc_d11rxhdr * wrxh; /**< ampdu responder saves wrxh temporarily */
#endif /* BCMPKTIDMAP */
		struct {
			uint8	ampdu_flow_id;
			uint8	cur_idx;
			uint8	exp_idx;
			uint8	flags;
		} ampdu_info_to_host;
	} u;
	uint16		scb_flowid;	/**< 12 bit SCB flowid + 2 bit incarnation + 2 bit radio */
	uint16		flags4;		/**< Unused */
	uint32		rspec;		/**< Phy rate for received packet */
	union {
		uint32		packetid;
#if defined(WLPKTDLYSTAT) || defined(WL11K)
		uint32		enqtime;	/**< Time when packet was enqueued into the FIFO */
#endif // endif
		uint32		pkt;	/* Pointer to reordered ofld pkt */
	} shared;
#ifdef DONGLEBUILD
	union {
#if defined(PROP_TXSTATUS)
		uint32 wl_hdr_information;
#endif /* PROP_TXSTATUS */
	struct {
		uint16 phystsbuf_idx;
		uint16 rsvd;
		};
        };
#else
	struct {
		uint16 phystsbuf_idx;
		uint16 rsvd[3];
	};
#endif /* DONGLEBUILD */
} wlc_pkttag_t;

typedef struct {
	uint32		flags;		/* Describe various packet properties */
	struct scb	*_scb;		/* Pointer to SCB for associated ea */
	int8		_bsscfgidx;	/* Index of bsscfg for this frame */
} wlc_rxpkttag_t;

#ifdef WLTAF
typedef struct {
	uint8		flags3;		/**< Describe various packet properties */
	uint8		index; /* window index */
	uint8		ref_cnt; /* reference count to utxd pkt */
	uint32		units; /* duration */
	uint32		req_trig_units; /* requested units */
	uint32		compl_trig_units; /* completed units */
	uint32		req_trig_bytes; /* requsted trigger bytes */
	uint32		trig_ts; /* timestamp */
} wlc_ulpkttag_t;

#define ULPKTTAG(osh, p) ((wlc_ulpkttag_t*) (PKTDATA(osh, p)))
#define WLULPKTTAG(p) ((wlc_ulpkttag_t*)(p))
#define WLULPKTTAGLEN (sizeof(wlc_ulpkttag_t))
#endif // endif

typedef struct wrr_info
{
	uint32 txframe;
	uint32 ncons[AC_COUNT];
} wrr_info_t;

#define WLPKTTAG(p) ((wlc_pkttag_t*)PKTTAG(p))
#define WLRXPKTTAG(p) ((wlc_rxpkttag_t*)RXPKTTAG(p))

#define WLPKTTAGCLEAR(p) bzero((char *)PKTTAG(p), sizeof(wlc_pkttag_t))
#define WLRXPKTTAGCLEAR(p) bzero((char *)RXPKTTAG(p), sizeof(wlc_rxpkttag_t))

/* Flags used in wlc_pkttag_t.
 * If adding a flag, be sure to check if WLPKTTAG_FLAG_MOVE should transfer it.
 */
#define WLF_PSMARK		0x00000001	/**< PKT marking for PSQ aging */
#define WLF_PSDONTQ		0x00000002	/**< PS-Poll response don't queue flag */
#define WLF_CTLMGMT		0x00000004	/**< Set if pkt is a CTL/MGMT frame */
#define WLF_NON8023		0x00000008	/**< original pkt is not 8023 */
#define WLF_8021X		0x00000010	/**< original pkt is 802.1x */
#define WLF_APSD		0x00000020	/**< APSD delivery frame */
#define WLF_AMSDU		0x00000040	/**< pkt is aggregated msdu */
/* we're out of space, so reusing this rx bit on tx side */
#define WLF_HWAMSDU		0x00000080	/**< Rx: HW/ucode has deaggregated this A-MSDU */
#define WLF_TXHDR		0x00000080	/**< Tx: pkt is 802.11 MPDU with plcp and txhdr */
#define WLF_FIFOPKT		0x00000100	/**< Used by WL_MULTIQUEUE module if pkt recovered *
						 * from FIFO
						 */
#define WLF_HOST_PKT		0x00000100	/**< pkt coming from host/bridge; overload
						 * WLF_FIFOPKT. set when pkt
						 * leaves per port layer and
						 * clear in wl layer.
						 */
#define WLF_EXPTIME		0x00000200 /**< pkttag has a valid expiration time for the pkt */
#define WLF_AMPDU_MPDU		0x00000400	/**< mpdu in a ampdu */
#define WLF_MIMO		0x00000800	/**< mpdu has a mimo rate */
#define WLF_RIFS		0x00001000	/**< frameburst with RIFS separated */
#define WLF_VRATE_PROBE		0x00002000	/**< vertical rate probe mpdu -- obselete/rev128 */
#define WLF_BSS_DOWN		0x00004000 /**< The BSS associated with the pkt has gone down */
#define WLF_BYPASS_TXC		0x00004000	/**< bypass txc lookup and update.
						 * assuming only used by data packet
						 * hence it won't conflict
						 * with WLF_BSS_DOWN used by MBSS beacon.
						 */
#define WLF_TXCMISS		0x00008000 /**< Packet missed tx cache lkup (overloaded below) */
#define WLF_RX_KM		0x00008000	/**< ICV trimmed off Rx pkt: pktfetch case */
#define WLF_EXEMPT_MASK		0x00030000	/**< mask for encryption exemption (Vista) */
#define WLF_EXEMPT_SHIFT	16
#define WLF_WME_NOACK		0x00040000	/**< pkt use WME No ACK policy */

#ifdef PROP_TXSTATUS
#define WLF_CREDITED		0x00100000	/**< AC credit for this pkt has been provided to
						 * the host
						 */
#endif /* PROP_TXSTATUS */

/* reuse for rx replay checking */
#define WLF_RX_PKTC_NOTFIRST	0x00200000  /* not the first packet in chanin */
#define WLF_RX_PKTC_NOTLAST		0x00400000  /* not the last packet in chain */

#define WLF_TDLS_TYPE		0x00800000	/**< pkt is of TDLS type */
#define WLF_TDLS_DIRECT		0x01000000	/**< pkt will use direct TDLS path */
#define WLF_TDLS_APPATH         0x02000000      /**< pkt will use AP path */
#define WLF_USERTS		0x04000000	/**< protect the packet with RTS/CTS */
#define WLF_RATE_AUTO		0x08000000 /**< pkt uses rates from the rate selection module */
#define WLF_PROPTX_PROCESSED    0x10000000	/**< marks pkt proptx processed  */

#define WLF_MFP			0x20000000	/**< pkt is MFP */
#define WLF_DATA		0x40000000	/**< pkt is pure data */

#define WLF_WAI			0x80000000	/**< original pkt is WAI */

/* re using wapi flag for mesh */
#define WLF_MESH_RETX		0x80000000	/* mesh pkt identifier */

#ifdef BCM_CSIMON
/* overload WLF_VRATE_PROBE that is deprecated for rev128 and CSI is applicable
 * to rev128++
 */
#define WLF_CSI_NULL_PKT    0x00002000    /**< CSI monitor  */
#endif /* BCM_CSIMON */

/* Flags2 used in wlc_pkttag_t (8 bits). */
#define WLF2_PCB1_MASK		0x0f	/**< see pcb1 definitions */
#define WLF2_PCB1_SHIFT		0
#define WLF2_PCB2_MASK		0x30	/**< see pcb2 definitions */
#define WLF2_PCB2_SHIFT		4
#define WLF2_PCB3_MASK		0x40	/**< see pcb3 definitions */
#define WLF2_PCB3_SHIFT		6
#define WLF2_PCB4_MASK		0x80	/**< see pcb4 definitions */
#define WLF2_PCB4_SHIFT		7
#define WLF2_HOSTREORDERAMPDU_INFO 0x20 /**< packet has info to help host reorder agg packets  */

/* Flags3 used in wlc_pkttag_t (8 bits). */
#define WLF3_SUPR		0x01	/**< pkt was suppressed due to PM 1 or NoA ABS */
#define WLF3_TAF_TAGGED		0x02	/**< pkt is taf tagged */
#define WLF3_NO_PMCHANGE	0x04	/**< don't touch pm state for this, e.g. keep_alive */
#define WLF3_BYPASS_AMPDU	0x08	/**< flag to bypass AMPDU queuing */
#define WLF3_TXQ_SHORT_LIFETIME	0x10	/**< pkt in TXQ is short-lived, for channel switch case */
#define WLF3_DATA_TCP_ACK	0x20	/**< pkt is TCP_ACK data */
#define WLF3_AMPDU_REGMPDU	0x40	/**< rempdu in ampdu module */
#define WLF3_DATA_WOWL_PKT	0x80

/*
 * pcb - packet class callback
 *
 * Packet class callbacks are registered by relevant modules up front. Each packet
 * that would like to invoke a callback sets an index in the packet tag. The value
 * of the index is:
 * - 0: reserved for no callback
 * - non 0: an index into the corresponding packet class callback table
 * - to simplify the implementation the index must be defined within the same byte
 * There are 3 tables defined so far:
 * - pcb1: wlc->pcb_cd[0].cb
 * - pcb2: wlc->pcb_cd[1].cb
 * - pcb3: wlc->pcb_cd[2].cb
 * - pcb4: wlc->pcb_cd[3].cb
 * Callbacks are invoked in the order of pcb1 and pcb2 (and so on so forth when more
 *   tables are added later)
 */

/* pcb1 */
#define WLF2_PCB1_STA_PRB	1	/**< wlc_ap_sta|wds_probe_complete */
#define WLF2_PCB1_PSP		2	/**< wlc_sendpspoll_complete */
#define WLF2_PCB1_AF		3	/**< wlc_actionframetx_complete */
#define WLF2_PCB1_PROBRESP	4	/* wlc_proberesp_cb in case WLPROBRESP_INTRANSIT_FILTER */
#define WLF2_PCB1_TKIP_CM	5	/**< wlc_tkip_countermeasure */
#define WLF2_PCB1_RATE_PRB	6	/**< wlc_rateprobe_complete */
#define WLF2_PCB1_TWT_AF	7	/**< wlc_twt_aftx_complete */
#define WLF2_PCB1_PM_NOTIF	8	/**< wlc_pm_notif_complete */
#define WLF2_PCB1_NAR		9	/* nar callback when pkt freed */
#ifdef WL_RELMCAST
#define WLF2_PCB1_RMC		10	/**< rmc callback when pkt freed */
#endif /* WL_RELMCAST */
#ifdef WLOLPC
#define WLF2_PCB1_OLPC		11	/**< olpc calibration pkts */
#endif /* WLOLPC */
#define WLF2_PCB1_NAN		12	/**< nan_mac_txstatus */
#define WLF2_PCB1_WLFC		13	/**< wl_hostpkt_callback */
#if defined(MBSS)
#define WLF2_PCB1_MBSS_BCMC	14	/** wlc_mbss_bcmc_free_cb */
#endif /* MBSS */

/* macros to access the pkttag.flags2.pcb1 field */
#define WLF2_PCB1(p)		((WLPKTTAG(p)->flags2 & WLF2_PCB1_MASK) >> WLF2_PCB1_SHIFT)
#define WLF2_PCB1_REG(p, cb)	(WLPKTTAG(p)->flags2 &= ~WLF2_PCB1_MASK, \
				 WLPKTTAG(p)->flags2 |= (cb) << WLF2_PCB1_SHIFT)
#define WLF2_PCB1_UNREG(p)	(WLPKTTAG(p)->flags2 &= ~WLF2_PCB1_MASK)

/* pcb2 */
#define WLF2_PCB2_APSD		1	/**< wlc_apps_apsd_complete */
#define WLF2_PCB2_PSP_RSP	2	/**< wlc_apps_psp_resp_complete */

/* macros to access the pkttag.flags2.pcb2 field */
#define WLF2_PCB2(p)		((WLPKTTAG(p)->flags2 & WLF2_PCB2_MASK) >> WLF2_PCB2_SHIFT)
#define WLF2_PCB2_REG(p, cb)	(WLPKTTAG(p)->flags2 &= ~WLF2_PCB2_MASK, \
				 WLPKTTAG(p)->flags2 |= (cb) << WLF2_PCB2_SHIFT)
#define WLF2_PCB2_UNREG(p)	(WLPKTTAG(p)->flags2 &= ~WLF2_PCB2_MASK)

/* pcb3 */
#define WLF2_PCB3_AMSDU		1	/**< wlc_amsdu_pkt_freed */

/* macros to access the pkttag.flags2.pcb3 field */
#define WLF2_PCB3(p)		((WLPKTTAG(p)->flags2 & WLF2_PCB3_MASK) >> WLF2_PCB3_SHIFT)
#define WLF2_PCB3_REG(p, cb)	(WLPKTTAG(p)->flags2 &= ~WLF2_PCB3_MASK, \
				 WLPKTTAG(p)->flags2 |= (cb) << WLF2_PCB3_SHIFT)
#define WLF2_PCB3_UNREG(p)	(WLPKTTAG(p)->flags2 &= ~WLF2_PCB3_MASK)

/* pcb4 */
#define WLF2_PCB4_AMPDU		1	/**< wlc_ampdu_pkt_freed */

/* macros to access the pkttag.flags2.pcb4 field */
#define WLF2_PCB4(p)		((WLPKTTAG(p)->flags2 & WLF2_PCB4_MASK) >> WLF2_PCB4_SHIFT)
#define WLF2_PCB4_REG(p, cb)	(WLPKTTAG(p)->flags2 &= ~WLF2_PCB4_MASK, \
				 WLPKTTAG(p)->flags2 |= (cb) << WLF2_PCB4_SHIFT)
#define WLF2_PCB4_UNREG(p)	(WLPKTTAG(p)->flags2 &= ~WLF2_PCB4_MASK)

#define	WLF_RESET_EXP_TIME(p)	((WLPKTTAG(p)->flags &= ~(WLF_EXPTIME)), \
					(WLPKTTAG(p)->u.exptime = 0))
#include <packed_section_start.h>
typedef BWL_PRE_PACKED_STRUCT struct
{
	uint16 rate;
	int8   rssi;
	uint8  channel;
}
BWL_POST_PACKED_STRUCT rx_ctxt_t;
#include <packed_section_end.h>

#ifdef WLAMPDU
#define WLPKTFLAG_AMPDU(pkttag)	((pkttag)->flags & WLF_AMPDU_MPDU)
#else
#define WLPKTFLAG_AMPDU(pkttag)	FALSE
#endif /* WLAMPDU */

#ifdef WLAMSDU
#define WLPKTFLAG_AMSDU(pkttag)	((pkttag)->flags & WLF_AMSDU)
#else
#define WLPKTFLAG_AMSDU(pkttag)	FALSE
#endif /* WLAMSDU */

#if defined(MFP)
/* flag for .11w Protected Management Frames(PMF) */
#define WLPKTFLAG_PMF(pkttag)	((pkttag)->flags & WLF_MFP)
#else
#define WLPKTFLAG_PMF(pkttag)	FALSE
#endif /* MFP */

/* LTE coex support */
#ifdef BCMLTECOEX
#if defined(ROM_ENAB_RUNTIME_CHECK)
#define BCMLTECOEX_ENAB(pub)	((pub)->_ltecx)
#define BCMLTECOEXGCI_ENAB(pub)	((pub)->_ltecxgci)
#elif defined(BCMLTECOEX_DISABLED)
#define BCMLTECOEX_ENAB(pub)	(0)
#define BCMLTECOEXGCI_ENAB(pub)	(0)
#else
#define BCMLTECOEX_ENAB(pub)	((pub)->_ltecx)
#define BCMLTECOEXGCI_ENAB(pub)	((pub)->_ltecxgci)
#endif /* ROM_ENAB_RUNTIME_CHECK */
#else
#define BCMLTECOEX_ENAB(pub)	(0)
#define BCMLTECOEXGCI_ENAB(pub)	(0)
#endif /* BCMLTECOEX */

/* Coex NoA support */
#ifdef BCMCOEXNOA
	#if defined(ROM_ENAB_RUNTIME_CHECK)
		#define BCMCOEXNOA_ENAB(pub)    ((pub)->_cxnoa)
	#elif defined(BCMBTCOEX_DISABLED)
		#define BCMCOEXNOA_ENAB(pub)    (0)
	#else
		#define BCMCOEXNOA_ENAB(pub)    ((pub)->_cxnoa)
	#endif
#else
	#define BCMCOEXNOA_ENAB(pub)    (0)
#endif /* BCMCOEXNOA */

/* CHANIM support */
#ifdef WLCHANIM
	#if defined(ROM_ENAB_RUNTIME_CHECK)
		#define WLC_CHANIM_ENAB(pub)	((pub)->_chanim)
	#elif defined(WLCHANIM_DISABLED)
		#define WLC_CHANIM_ENAB(pub)	(0)
	#else
		#define WLC_CHANIM_ENAB(pub)	(1)
	#endif
#else
	#define WLC_CHANIM_ENAB(pub)	(0)
#endif /* WLCHANIM */

/* CCA_STATS support */
#ifdef CCA_STATS
	#if defined(ROM_ENAB_RUNTIME_CHECK)
		#define WL_CCA_STATS_ENAB(pub)	((pub)->_cca_stats)
	#elif defined(CCA_STATS_DISABLED)
		#define WL_CCA_STATS_ENAB(pub)	(0)
	#else
		#define WL_CCA_STATS_ENAB(pub)	((pub)->_cca_stats)
	#endif
#else
	#define WL_CCA_STATS_ENAB(pub)	(0)
#endif /* CCA_STATS */

/* DELTASTATS support */
#ifdef DELTASTATS
	#if defined(ROM_ENAB_RUNTIME_CHECK)
		#define DELTASTATS_ENAB(pub)	((pub)->_deltastats)
	#elif defined(DELTASTATS_DISABLED)
		#define DELTASTATS_ENAB(pub)	(0)
	#else
		#define DELTASTATS_ENAB(pub)	((pub)->_deltastats)
	#endif
#else
	#define DELTASTATS_ENAB(pub)	(0)
#endif /* DELTASTATS */

/* RSSI MONITOR support */
#ifdef RSSI_MONITOR
	#if defined(ROM_ENAB_RUNTIME_CHECK)
		#define RMON_ENAB(pub)		((pub)->cmn->_rmon)
	#elif defined(RSSI_MONITOR_DISABLED)
		#define RMON_ENAB(pub)		(0)
	#else
		#define RMON_ENAB(pub)		((pub)->cmn->_rmon)
	#endif
#else
	#define RMON_ENAB(pub)		(0)
#endif /* RSSI_MONITOR */

/* HEALTH_CHECK support */
#ifdef HEALTH_CHECK
	#if defined(ROM_ENAB_RUNTIME_CHECK)
		#define WL_HEALTH_CHECK_ENAB(cmn)	((cmn)->_health_check)
	#elif defined(HEALTH_CHECK_DISABLED)
		#define WL_HEALTH_CHECK_ENAB(cmn)	(0)
	#else
		#define WL_HEALTH_CHECK_ENAB(cmn)	(1)
	#endif
#else
	#define WL_HEALTH_CHECK_ENAB(cmn)	(0)
#endif /* HEALTH_CHECK */

#define WLPKTFLAG_RIFS(pkttag)	((pkttag)->flags & WLF_RIFS)

#define WLPKTFLAG_BSS_DOWN_GET(pkttag) ((pkttag)->flags & WLF_BSS_DOWN)
#define WLPKTFLAG_BSS_DOWN_SET(pkttag, val) (pkttag)->flags |= ((val) ? WLF_BSS_DOWN : 0)

#define WLPKTFLAG_EXEMPT_GET(pkttag) (((pkttag)->flags & WLF_EXEMPT_MASK) >> WLF_EXEMPT_SHIFT)
#define WLPKTFLAG_EXEMPT_SET(pkttag, val) ((pkttag)->flags = \
			((pkttag)->flags & ~WLF_EXEMPT_MASK) | (val << WLF_EXEMPT_SHIFT));
#define WLPKTFLAG_NOACK(pkttag)	((pkttag)->flags & WLF_WME_NOACK)

/* API for accessing BSSCFG index in WLPKTTAG */
#define BSSCFGIDX_ISVALID(bsscfgidx) (((bsscfgidx >= 0)&&(bsscfgidx < WLC_MAXBSSCFG)) ? 1 : 0)

static INLINE int8
wlc_pkttag_bsscfg_get(void *p)
{
	int8 idx = WLPKTTAG(p)->_bsscfgidx;
#ifdef BCMDBG
	ASSERT(BSSCFGIDX_ISVALID(idx));
#endif /* BCMDBG */
	return idx;
}

#define WLPKTTAGBSSCFGGET(p) (wlc_pkttag_bsscfg_get(p))
#define WLPKTTAGBSSCFGSET(p, bsscfgidx) (WLPKTTAG(p)->_bsscfgidx = bsscfgidx)
#define WLRXPKTTAGBSSCFGGET(p) (WLRXPKTTAG(p)->_bsscfgidx)
#define WLRXPKTTAGBSSCFGSET(p, bsscfgidx) (WLRXPKTTAG(p)->_bsscfgidx = bsscfgidx)

/* Raw get of bss idx from pkt tag without error checking */
#define WLPKTTAG_BSSIDX_GET(pkttag) ((pkttag)->_bsscfgidx)

/* Raw get of scb flowid from pkttag without error checking */
#define WLPKTTAGSCBFLOWIDGET(p) (WLPKTTAG(p)->scb_flowid)
/* API for accessing SCB pointer in WLPKTTAG
 * Returns NULL for invalid ID <SCB_FLOWID_INVALID>
 * Returns scb from global scb lookup table for valid ID
 */
#define WLPKTTAGSCBGET(p)	(wlc_scb_flowid_global_lookup(WLPKTTAGSCBFLOWIDGET(p)))
#define WLPKTTAGSCBSET(p, _scb) \
	(WLPKTTAG(p)->scb_flowid = ((_scb) ? (SCB_FLOWID_GLOBAL((struct scb *)(_scb))) : \
		(SCB_FLOWID_INVALID)))

#define WLPKTTAGSCBCLR(p)	(WLPKTTAG(p)->scb_flowid = SCB_FLOWID_INVALID)

#define WLRXPKTTAGSCBGET(p)	(WLRXPKTTAG(p)->_scb)
#define WLRXPKTTAGSCBSET(p, scb)	(WLRXPKTTAG(p)->_scb = scb)
#define WLRXPKTTAGSCBCLR(p)	(WLRXPKTTAG(p)->_scb = NULL)

#endif /* LINUX_POSTMOGRIFY_REMOVAL */

/***********************************************
 * Feature-related macros to optimize out code *
 * *********************************************
 */
#ifdef BPRESET
#define BPRESET_ENAB(pub)	((pub)->_bpreset)
#else
#define BPRESET_ENAB(pub)	(0)
#endif /* BPRESET */

/* ROM_ENAB_RUNTIME_CHECK may be set based upon the #define below (for ROM builds). It may also
 * be defined via makefiles (e.g. ROM auto abandon unoptimized compiles).
 */
#if defined(BCMROMBUILD)
	#ifndef ROM_ENAB_RUNTIME_CHECK
		#define ROM_ENAB_RUNTIME_CHECK
	#endif /* ifndef ROM_ENAB_RUNTIME_CHECK */
#endif /* BCMROMBUILD */

/* AP Support (versus STA) */
#if defined(AP) && !defined(STA)
#define	AP_ENAB(pub)	(1)
#elif !defined(AP) && defined(STA)
#define	AP_ENAB(pub)	(0)
#else /* both, need runtime check */
#define	AP_ENAB(pub)	((pub)->_ap)
#endif /* defined(AP) && !defined(STA) */

#ifdef APCS
	#if defined(ROM_ENAB_RUNTIME_CHECK) || !defined(DONGLEBUILD)
		#define APCS_ENAB(pub) ((pub)->_apcs)
	#elif defined(APCS_DISABLED)
		#define APCS_ENAB(pub) (0)
	#else
		#define APCS_ENAB(pub) (1)
	#endif /* defined(ROM_ENAB_RUNTIME_CHECK) || !defined(DONGLEBUILD) */
#else /* !APCS */
	#define APCS_ENAB(pub) (0)
#endif /* APCS */

#ifdef PSPRETEND
	#if defined(ROM_ENAB_RUNTIME_CHECK) || !defined(DONGLEBUILD)
		#define PSPRETEND_ENAB(pub) ((pub)->_pspretend)
	#elif defined(PSPRETEND_DISABLED)
		#define PSPRETEND_ENAB(pub) (0)
	#else
		#define PSPRETEND_ENAB(pub) (1)
	#endif
#else /* !PSPRETEND */
	#define PSPRETEND_ENAB(pub) (0)
#endif /* PSPRETEND */

/* Macro to check if APSTA mode enabled */
#if defined(AP) && defined(STA)
#define APSTA_ENAB(pub)	((pub)->_apsta)
#else
#define APSTA_ENAB(pub)	(0)
#endif /* defined(AP) && defined(STA) */

#ifdef PSTA
#define PSTA_ENAB(pub)	((pub)->_psta)
#define	PSTA_IS_PROXY(wlc)	((wlc)->pub->_psta == PSTA_MODE_PROXY)
#define	PSTA_IS_REPEATER(wlc)	((wlc)->pub->_psta == PSTA_MODE_REPEATER)
#else /* PSTA */
#define PSTA_ENAB(pub)	(0)
#define	PSTA_IS_PROXY(wlc)	(0)
#define	PSTA_IS_REPEATER(wlc)	(0)
#endif /* PSTA */

#if defined(WET) && defined(WET_DONGLE)
#error "Don't use WET and WET_DONGLE in the same build"
#endif // endif

#if defined(DONGLEBUILD) && defined(WET)
#error "Use WET_DONGLE iso WET for donglebuilds"
#endif // endif

#ifdef WET
#define WET_ENAB(wlc)		(wlc->wet)
#else
#define WET_ENAB(wlc)		(0)
#endif // endif

#ifdef WET_DONGLE
#define WET_DONGLE_ENAB(wlc)	(wlc->wet_dongle)
#else
#define WET_DONGLE_ENAB(wlc)	(0)
#endif // endif

#if defined(PKTC) || defined(PKTC_DONGLE)
#define PKTC_ENAB(pub)	((pub)->_pktc)
#else
#define PKTC_ENAB(pub)	(0)
#endif /* defined(PKTC) || defined(PKTC_DONGLE) */

#if defined(PKTC_DONGLE)
#define PKTC_TX_ENAB(pub)	(1)
#elif defined(PKTC)
#define PKTC_TX_ENAB(pub)	((pub)->_pktc_tx)
#else /* !PKTC !PKTC_DONGLE */
#define PKTC_TX_ENAB(pub)	(0)
#endif /* PKTC_DONGLE */

/* 11AC MU-MIMO Tx Support macros */
#ifdef WL_MU_TX
/* If the MU Tx feature is defined, have dongle builds always force on or off,
 * but have NIC and ROM builds use the runtime check.
 * Dongle RAM code do not need the runtime config, they should be compiled
 * on way or the other.
 */
#if defined(ROM_ENAB_RUNTIME_CHECK) || !defined(DONGLEBUILD)
	#define MU_TX_ENAB(wlc)         ((wlc)->_mu_tx)
#elif defined(WL_MU_TX_DISABLED)
	#define MU_TX_ENAB(wlc)         (0)
#else
	#define MU_TX_ENAB(wlc)         ((wlc)->_mu_tx)
#endif // endif
#else /* WL_MU_TX */
#define MU_TX_ENAB(wlc)                 (0)
#endif /* WL_MU_TX */

/* MU-MIMO Rx Support macros */
#ifdef WL_MU_RX
/* If the MU Rx feature is defined, have dongle builds always force on or off,
 * but have NIC and ROM builds use the runtime check.
 * Dongle RAM code do not need the runtime config, they should be compiled
 * on way or the other.
 */
#if defined(ROM_ENAB_RUNTIME_CHECK) || !defined(DONGLEBUILD)
	#define MU_RX_ENAB(wlc)         ((wlc)->pub->cmn->_mu_rx)
#elif defined(WL_MU_RX_DISABLED)
	#define MU_RX_ENAB(wlc)         (0)
#else
	#define MU_RX_ENAB(wlc)         ((wlc)->pub->cmn->_mu_rx)
#endif // endif
#else /* WL_MU_RX */
#define MU_RX_ENAB(wlc)                 (0)
#endif /* WL_MU_RX */

/* BCM DMA Cut Through mode Support macros */
#ifdef BCM_DMA_CT
/* Check if hardware supports Cut Through Mode.
 * In the future check for Cut Through mode Capability bit when HW supports it.
 */
#define WLC_CT_HW_SUPPORTED(wlc) (((D11REV_GE((wlc)->pub->corerev, 65)) &&\
	(D11REV_LT((wlc)->pub->corerev, 80))) || (D11REV_GE((wlc)->pub->corerev, 128)))

/* If the BCM_DMA_CT feature is defined, have dongle builds always force on or off,
 * but have NIC and ROM builds use the runtime check.
 * Dongle RAM code do not need the runtime config, they should be compiled
 * on way or the other.
 */
#if defined(ROM_ENAB_RUNTIME_CHECK) || !defined(DONGLEBUILD)
	#define BCM_DMA_CT_ENAB(wlc)	((wlc)->_dma_ct)
#elif defined(BCM_DMA_CT_DISABLED)
	#define BCM_DMA_CT_ENAB(wlc)	(0)
#else
	#define BCM_DMA_CT_ENAB(wlc)	((wlc)->_dma_ct)
#endif // endif
#else /* BCM_DMA_CT */
#define BCM_DMA_CT_ENAB(wlc)		(0)
#endif /* BCM_DMA_CT */

/* TX Beamforming Support */
#if defined(WL_BEAMFORMING)
	#if defined(ROM_ENAB_RUNTIME_CHECK) || !defined(DONGLEBUILD)
		#define TXBF_ENAB(pub)		((pub)->_txbf)
	#elif defined(WLTXBF_DISABLED)
		#define TXBF_ENAB(pub)		(0)
	#else
		#define TXBF_ENAB(pub)		(1)
	#endif /* defined(ROM_ENAB_RUNTIME_CHECK) || !defined(DONGLEBUILD) */
#else
	#define TXBF_ENAB(pub)			(0)
#endif /* WL_BEAMFORMING */

/* STA Priority Feature Support  */
#ifdef WL_STAPRIO
	#if defined(ROM_ENAB_RUNTIME_CHECK)
		#define STAPRIO_ENAB(pub)		((pub)->_staprio)
	#elif defined(WL_STAPRIO_DISABLED)
		#define STAPRIO_ENAB(pub)		(0)
	#else
		#define STAPRIO_ENAB(pub)		(1)
	#endif /* defined(ROM_ENAB_RUNTIME_CHECK) */
#else
	#define STAPRIO_ENAB(pub)			(0)
#endif /* WL_STAPRIO */

#ifdef WL_STA_MONITOR
#define STAMON_ENAB(pub) ((pub)->_stamon)
#else
#define STAMON_ENAB(pub) (0)
#endif /* WL_STA_MONITOR */

#ifdef BCM_CSIMON
#define CSIMON_ENAB(pub) ((pub)->_csimon)
#else
#define CSIMON_ENAB(pub) (0)
#endif /* BCM_CSIMON */

/* Some useful combinations */
#define STA_ONLY(pub)	(!AP_ENAB(pub))
#define AP_ONLY(pub)	(AP_ENAB(pub) && !APSTA_ENAB(pub))

/* proptxstatus feature dynamic enable, compile time enable or disable */
#if defined(PROP_TXSTATUS)
	#if defined(ROM_ENAB_RUNTIME_CHECK) || !defined(DONGLEBUILD)
		#define PROP_TXSTATUS_ENAB(pub)		((pub)->_proptxstatus)
	#elif defined(PROP_TXSTATUS_ENABLED)
		#define PROP_TXSTATUS_ENAB(pub)		1
	#else
		#define PROP_TXSTATUS_ENAB(pub)		0
	#endif /* defined(ROM_ENAB_RUNTIME_CHECK) || !defined(DONGLEBUILD) */
#else
	#define PROP_TXSTATUS_ENAB(pub)		0
#endif /* PROP_TXSTATUS */

/* Send Flow control information to host through TLV's */
#if defined(PROP_TXSTATUS)
	#if defined(ROM_ENAB_RUNTIME_CHECK) || !defined(DONGLEBUILD)
		#define WLFC_CONTROL_SIGNALS_TO_HOST_ENAB(pub)	((pub)->_wlfc_ctrl_to_host)
	#elif defined(WLFC_CONTROL_TO_HOST_DISABLED)
		#define WLFC_CONTROL_SIGNALS_TO_HOST_ENAB(pub)	0
	#else
		#define WLFC_CONTROL_SIGNALS_TO_HOST_ENAB(pub)	1
	#endif /* defined(ROM_ENAB_RUNTIME_CHECK) || !defined(DONGLEBUILD) */
#else
	#define WLFC_CONTROL_SIGNALS_TO_HOST_ENAB(pub)	0
#endif /* PROP_TXSTATUS */

/* Send Rxed packet information to host through TLV's */
#if defined(PROP_TXSTATUS)
	#if defined(ROM_ENAB_RUNTIME_CHECK) || !defined(DONGLEBUILD)
		#define RXMETADATA_TO_HOST_ENAB(pub)	((pub)->_rxmetadata_to_host)
	#elif defined(RXMETADATA_TO_HOST_DISABLED)
		#define RXMETADATA_TO_HOST_ENAB(pub)	0
	#else
		#define RXMETADATA_TO_HOST_ENAB(pub)	((pub)->_rxmetadata_to_host)
	#endif /* defined(ROM_ENAB_RUNTIME_CHECK) || !defined(DONGLEBUILD) */
#else
	#define RXMETADATA_TO_HOST_ENAB(pub)	0
#endif /* PROP_TXSTATUS */

/* Send txed packet information to host through TLV's */
#if defined(PROP_TXSTATUS)
	#if defined(ROM_ENAB_RUNTIME_CHECK) || !defined(DONGLEBUILD)
		#define TXMETADATA_TO_HOST_ENAB(pub)	((pub)->_txmetadata_to_host)
	#elif defined(TXMETADATA_TO_HOST_DISABLED)
		#define TXMETADATA_TO_HOST_ENAB(pub)	0
	#else
		#define TXMETADATA_TO_HOST_ENAB(pub)	((pub)->_txmetadata_to_host)
	#endif /* defined(ROM_ENAB_RUNTIME_CHECK) || !defined(DONGLEBUILD) */
#else
	#define TXMETADATA_TO_HOST_ENAB(pub)	0
#endif /* PROP_TXSTATUS */

/* Send Flow control information to host through TLV's */
#if defined(PROP_TXSTATUS)
	#if defined(ROM_ENAB_RUNTIME_CHECK) || !defined(DONGLEBUILD)
		#define WLFC_CONTROL_SIGNALS_TO_HOST_ENAB(pub)	((pub)->_wlfc_ctrl_to_host)
	#elif defined(WLFC_CONTROL_TO_HOST_DISABLED)
		#define WLFC_CONTROL_SIGNALS_TO_HOST_ENAB(pub)	0
	#else
		#define WLFC_CONTROL_SIGNALS_TO_HOST_ENAB(pub)	1
	#endif /* defined(ROM_ENAB_RUNTIME_CHECK) || !defined(DONGLEBUILD) */
#else
	#define WLFC_CONTROL_SIGNALS_TO_HOST_ENAB(pub)	0
#endif /* PROP_TXSTATUS */

/* Send Flow control information to bus through TLV's, bus consumes FC info */
#if defined(PROP_TXSTATUS)
	#if defined(ROM_ENAB_RUNTIME_CHECK) || !defined(DONGLEBUILD)
		#define WLFC_INFO_TO_BUS_ENAB(pub)	((pub)->_wlfc_info_to_bus)
	#elif defined(WLFC_INFO_TO_BUS_DISABLED)
		#define WLFC_INFO_TO_BUS_ENAB(pub)	0
	#else
		#define WLFC_INFO_TO_BUS_ENAB(pub)	1
	#endif /* defined(ROM_ENAB_RUNTIME_CHECK) || !defined(DONGLEBUILD) */
#else
	#define WLFC_INFO_TO_BUS_ENAB(pub)	0
#endif /* PROP_TXSTATUS */

#ifndef LINUX_POSTMOGRIFY_REMOVAL
/* Primary MBSS enable check macro */
#define MBSS_OFF	0
#define MBSS_ENABLED	1 /* earlier MBSS16_ENABLED */
#if defined(MBSS)
	#if defined(ROM_ENAB_RUNTIME_CHECK) || !defined(DONGLEBUILD)
		#define MBSS_ENAB(pub)		((pub)->_mbss_mode)
		#define MBSS_SUPPORT(pub)	((pub)->_mbss_support)
		#define MBSS_IGN_MAC_VALID(pub)	((pub)->_mbss_ign_mac_valid && \
			(pub)->_mbss_mode)
	#elif defined(MBSS_DISABLED)
		#define MBSS_ENAB(pub)		(0)
		#define MBSS_SUPPORT(pub)	(0)
		#define MBSS_IGN_MAC_VALID(pub)	(0)
	#else
		#define MBSS_ENAB(pub)		((pub)->_mbss_mode)
		#define MBSS_SUPPORT(pub)	(1)
		#define MBSS_IGN_MAC_VALID(pub)	((pub)->_mbss_ign_mac_valid && \
			(pub)->_mbss_mode)
	#endif /* defined(ROM_ENAB_RUNTIME_CHECK) || !defined(DONGLEBUILD) */
#else /* !MBSS */
	#define MBSS_ENAB(pub)			(0)
	#define MBSS_SUPPORT(pub)		(0)
	#define MBSS_IGN_MAC_VALID(pub)		(0)
#endif /* MBSS */

#if defined(WME_PER_AC_TX_PARAMS)
/* With RATELINKMEM, there is a single rate-entry per SCB, so no per-AC support */
#define WME_PER_AC_TX_PARAMS_ENAB(pub)		(!RATELINKMEM_ENAB(pub))
#define WME_PER_AC_MAXRATE_ENAB(pub)		(!RATELINKMEM_ENAB(pub) && (pub)->_per_ac_maxrate)
#else /* WME_PER_AC_TX_PARAMS */
#define WME_PER_AC_TX_PARAMS_ENAB(pub)		(0)
#define WME_PER_AC_MAXRATE_ENAB(pub)		(0)
#endif /* WME_PER_AC_TX_PARAMS */

/* Shared/Open Authentication Method, Enable/Disable */
#if defined(WL_AUTH_SHARED_OPEN)
	#if defined(ROM_ENAB_RUNTIME_CHECK) || !defined(DONGLEBUILD)
		#define AUTH_SHARED_OPEN_ENAB(pub)	((pub)->_auth_shared_open)
	#elif defined(WL_AUTH_SHARED_OPEN_DISABLED)
		#define AUTH_SHARED_OPEN_ENAB(pub)	0
	#else
		#define AUTH_SHARED_OPEN_ENAB(pub)	1
	#endif
#else
	#define AUTH_SHARED_OPEN_ENAB(pub)		0
#endif /* WL_AUTH_SHARED_OPEN */

/* Dynamic Tx Power control */
#ifdef WLC_DTPC
	#if defined(ROM_ENAB_RUNTIME_CHECK) || !defined(DONGLEBUILD)
		#define DTPC_ENAB(pub)	((pub)->_dtpc)
	#elif defined(WLC_DTPC_DISABLED)
		#define DTPC_ENAB(pub)	0
	#else
		#define DTPC_ENAB(pub)	1
	#endif
#else
	#define DTPC_ENAB(pub)		0
#endif /* WLC_DTPC */

#define ENAB_1x1	0x01
#define ENAB_2x2	0x02
#define ENAB_3x3	0x04
#define ENAB_4x4	0x08
#define SUPPORT_11N	(ENAB_1x1|ENAB_2x2)
#define SUPPORT_HT	(ENAB_1x1|ENAB_2x2|ENAB_3x3)
/* WL11N Support */
#if ((defined(NCONF) && (NCONF != 0)) || (defined(LCN20CONF) && (LCN20CONF != 0)) || \
	(defined(ACCONF) && (ACCONF != 0)) || (defined(ACCONF2) && (ACCONF2 != 0)) || \
	(defined(ACCONF5) && (ACCONF5 != 0)))
#define N_ENAB(pub) ((pub)->_n_enab & SUPPORT_11N)
#define N_REQD(pub) ((pub)->_n_reqd)
#else
#define N_ENAB(pub)	((void)(pub), 0)
#define N_REQD(pub)	((void)(pub), 0)
#endif /* NCONF && ... */

#if defined(WL_MBO) && defined(MBO_AP)
	#if defined(ROM_ENAB_RUNTIME_CHECK)
		#define MBO_ENAB(pub)	((pub)->cmn->_mbo)
	#elif defined(WL_MBO_DISABLED)
		#define MBO_ENAB(pub)	(0)
	#else
		#define MBO_ENAB(pub)	((pub)->cmn->_mbo)
	#endif /* defined(ROM_ENAB_RUNTIME_CHECK) */
#else
	#define MBO_ENAB(pub)		(0)
#endif /* WL_MBO && MBO_AP */

	 #define NATOE_ENAB(pub)      (0)
	 #define NATOE_ACTIVE(pub)    (0)

/* WL11AC Support */
#if defined(WL11AC) && ((defined(ACCONF) && (ACCONF != 0)) || (defined(ACCONF2) && \
	(ACCONF2 != 0)) || (defined(ACCONF5) && (ACCONF5 != 0)))

/*
 * VHT features rates  bitmap.
 * Bits 0:7 is the same as the rate expansion field in the proprietary IE
 * Bit 0:		5G MCS 0-9 BW 160MHz
 * Bit 1:		5G MCS 0-9 support BW 80MHz
 * Bit 2:		5G MCS 0-9 support BW 20MHz
 * Bit 3:		2.4G MCS 0-9 support BW 20MHz
 * Bit 4:		Prop MCS 10-11 support
 * Bit 5:		1024QAM support
 * bit 6:		Advertise VHT IE in 2.4G
 * bit 7:		Reserved for further used
 * Bit 8:		VHT 5G support
 * Bit 9:		VHT 2.4G support
 * Bit 10:11	Allowed MCS map
 *		0 is MCS 0-7
 *		1 is MCS 0-8
 *		2 is MCS 0-9
 *		3 is Disabled
 * Bit 12:13	Allowed MCS map2
 * 		0 is MCS 10-11
 * 		1 not defined
 *		2 not defined
 *		3 is disabled
 */

#define WL_VHT_FEATURES_5G_160M		0x01
#define WL_VHT_FEATURES_5G_80M		0x02
#define WL_VHT_FEATURES_5G_20M		0x04
#define WL_VHT_FEATURES_2G_20M		0x08
#define WL_VHT_FEATURES_1024QAM		0x10
#define WL_VHT_FEATURES_2G_ADV		0x20
#define WL_VHT_FEATURES_5G		0x100
#define WL_VHT_FEATURES_2G		0x200

#define WL_VHT_FEATURES_MCS_S		10
#define WL_VHT_FEATURES_MCS_M		(VHT_CAP_MCS_MAP_M << WL_VHT_FEATURES_MCS_S)
#define WL_VHT_FEATURES_MCS_DEFAULT	(VHT_CAP_MCS_MAP_0_9 << WL_VHT_FEATURES_MCS_S)

#define WL_VHT_FEATURES_PROP_MCS_S		12
#define WL_VHT_FEATURES_PROP_MCS_M		(VHT_CAP_MCS_MAP_M << WL_VHT_FEATURES_PROP_MCS_S)
#define WL_VHT_FEATURES_PROP_MCS_DEFAULT	(VHT_PROP_MCS_MAP_NONE << \
						WL_VHT_FEATURES_PROP_MCS_S)

#define WL_VHT_FEATURES_RATEMASK	0x0F
#define WL_VHT_FEATURES_RATES_ALL	0x1F

/* 11ac std rates + prop 1024 qam rates */
#define WL_VHT_FEATURES_RATES_2G	(WL_VHT_FEATURES_2G_20M | WL_VHT_FEATURES_1024QAM)
#define WL_VHT_FEATURES_RATES_5G	(WL_VHT_FEATURES_5G_160M |\
					 WL_VHT_FEATURES_5G_80M |\
					 WL_VHT_FEATURES_5G_20M |\
					 WL_VHT_FEATURES_1024QAM)

/* 11ac std defined rates */
#define WL_VHT_FEATURES_STD_RATES_2G	(WL_VHT_FEATURES_2G_20M)
#define WL_VHT_FEATURES_STD_RATES_5G	(WL_VHT_FEATURES_5G_160M |\
					 WL_VHT_FEATURES_5G_80M |\
					 WL_VHT_FEATURES_5G_20M)

#define WL_VHT_FEATURES_DEFAULT		(WL_VHT_FEATURES_5G |\
					 WL_VHT_FEATURES_MCS_DEFAULT |\
					 WL_VHT_FEATURES_PROP_MCS_DEFAULT)
/*
 * Note (not quite triple-x):
 * Default enable 5G VHT operation, no extended rates
 * Right now there are no 2.4G only VHT devices,
 * if that changes the defaults will have to change.
 * Fixup WLC_VHT_FEATURES_DEFAULT() to setup the device
 * and band specific dependencies.
 * Macro should look at all the supported bands in wlc not just the
 * active band.
 */

#define SM(v, m, s) (((v) << (s)) & (m))
#define MS(x, m, s) (((x) & (m)) >> (s))
#define WLC_VHT_FEATURES_DEFAULT(pub, bandstate, nbands)\
			((pub)->vht_features = (uint16)WL_VHT_FEATURES_DEFAULT)

#define WLC_VHT_FEATURES_GET(x, mask) ((x)->vht_features & (mask))
#define WLC_VHT_FEATURES_SET(x, mask) ((x)->vht_features |= (mask))
#define WLC_VHT_FEATURES_CLR(x, mask) ((x)->vht_features &= ~(mask))
#define WLC_VHT_FEATURES_5G(x) WLC_VHT_FEATURES_GET((x), WL_VHT_FEATURES_5G)
#define WLC_VHT_FEATURES_2G(x) WLC_VHT_FEATURES_GET((x), WL_VHT_FEATURES_2G)
#define WLC_VHT_FEATURES_2G_ADV(x) WLC_VHT_FEATURES_GET((x), WL_VHT_FEATURES_2G_ADV)
#define WLC_VHT_FEATURES_1024QAM(x) WLC_VHT_FEATURES_GET((x), WL_VHT_FEATURES_1024QAM)

#define WLC_VHT_FEATURES_RATES_IS5G(x, m) \
		(WLC_VHT_FEATURES_GET((x), WL_VHT_FEATURES_RATES_5G) & (m))
#define WLC_VHT_FEATURES_RATES_IS2G(x, m) \
		(WLC_VHT_FEATURES_GET((x), WL_VHT_FEATURES_RATES_2G) & (m))
#define WLC_VHT_FEATURES_RATES(x) (WLC_VHT_FEATURES_GET((x), WL_VHT_FEATURES_RATES_ALL))
#define WLC_VHT_FEATURES_RATES_5G(x) (WLC_VHT_FEATURES_GET((x), WL_VHT_FEATURES_RATES_5G))
#define WLC_VHT_FEATURES_RATES_2G(x) (WLC_VHT_FEATURES_GET((x), WL_VHT_FEATURES_RATES_2G))

#define WLC_VHT_FEATURES_MCS_SET(x, v) \
		do { \
		WLC_VHT_FEATURES_CLR(x, WL_VHT_FEATURES_MCS_M); \
		WLC_VHT_FEATURES_SET(x, SM(v, WL_VHT_FEATURES_MCS_M, WL_VHT_FEATURES_MCS_S)); \
		} while (0)

#define WLC_VHT_FEATURES_MCS_GET(x) \
		MS((x)->vht_features, WL_VHT_FEATURES_MCS_M, WL_VHT_FEATURES_MCS_S)

#define WLC_VHT_FEATURES_PROP_MCS_SET(x, v) \
		do { \
		WLC_VHT_FEATURES_CLR(x, WL_VHT_FEATURES_PROP_MCS_M); \
		WLC_VHT_FEATURES_SET(x, SM(v, WL_VHT_FEATURES_PROP_MCS_M, \
			WL_VHT_FEATURES_PROP_MCS_S)); \
		} while (0)

#define WLC_VHT_FEATURES_PROP_MCS_GET(x) \
		MS((x)->vht_features, WL_VHT_FEATURES_PROP_MCS_M, WL_VHT_FEATURES_PROP_MCS_S)

/* Performs a check before based on band, used when processing
 * and updating the RF band info in wlc
 */
#define VHT_ENAB(pub)((pub)->_vht_enab)
#define VHT_ENAB_BAND(pub, band) ((VHT_ENAB((pub))) && (BAND_5G((band)) ?\
		(WLC_VHT_FEATURES_5G((pub))):((BAND_2G((band)) ?\
		(WLC_VHT_FEATURES_2G((pub))) : 0))))

#define VHT_ENAB_VHT_RATES(p, b, m) (VHT_ENAB_BAND(p, b) &&\
		(BAND_5G(b) ? WLC_VHT_FEATURES_RATES_IS5G(p, m):\
		(BAND_2G(b) ? WLC_VHT_FEATURES_RATES_IS2G(p, m) : 0)))

#define VHT_FEATURES_ENAB(p, b) (VHT_ENAB_VHT_RATES((p), (b), WLC_VHT_FEATURES_RATES((p))))
#else
#define VHT_ENAB_BAND(pub, band)  ((void)(pub), 0)
#define VHT_ENAB(pub)	((void)(pub), 0)
#define WLC_VHT_FEATURES_DEFAULT(pub, bandstate, nbands)
#define WLC_VHT_FEATURES_GET(x, mask) (0)
#define WLC_VHT_FEATURES_SET(x, mask) (0)
#define WLC_VHT_FEATURES_5G(x) (0)
#define WLC_VHT_FEATURES_2G(x) (0)
#define WLC_VHT_FEATURES_2G_ADV(x)	(0)
#define WLC_VHT_FEATURES_1024QAM(x) (0)
#define WLC_VHT_FEATURES_RATES_ALL(x) (0)
#define VHT_ENAB_VHT_RATES(p, b, m) (0)
#define VHT_FEATURES_ENAB(p, b) (0)
#define WLC_VHT_FEATURES_RATES(x) (0)
#define WLC_VHT_FEATURES_RATES_5G(x) (0)
#define WLC_VHT_FEATURES_RATES_2G(x) (0)
#define WLC_VHT_FEATURES_SET(x, mask) (0)
#define WLC_VHT_FEATURES_CLR(x, mask) (0)

#define WL_VHT_FEATURES_RATES_ALL (0)
#define WL_VHT_FEATURES_5G_160M		(0)
#define WL_VHT_FEATURES_5G_80M		(0)
#define WL_VHT_FEATURES_5G_20M		(0)
#define WL_VHT_FEATURES_2G_20M		(0)
#define WL_VHT_FEATURES_1024QAM		(0)
#define WL_VHT_FEATURES_5G		(0)
#define WL_VHT_FEATURES_2G		(0)
#define WL_VHT_FEATURES_2G_ADV	(0)

#define WL_VHT_FEATURES_RATEMASK	0x0F
#define WL_VHT_FEATURES_RATES_ALL	(0)

#define WL_VHT_FEATURES_RATES_2G	(0)
#define WL_VHT_FEATURES_RATES_5G	(0)
#define WL_VHT_FEATURES_STD_RATES_2G	(0)
#define WL_VHT_FEATURES_STD_RATES_5G	(0)

#define WLC_VHT_FEATURES_MCS_SET(x, val)	(0)
#define WLC_VHT_FEATURES_MCS_GET(x)		(0)

#define WLC_VHT_FEATURES_PROP_MCS_SET(x, val)	(0)
#define WLC_VHT_FEATURES_PROP_MCS_GET(x)		(0)

#endif /* defined(WL11AC) ... */

/* 11ax support */
#ifdef WL11AX
/*
 * HE features bitmap.
 * Bit 0:		HE 5G support
 * Bit 1:		HE 2G support
 * Bit 2:		HE DL-OFDMA support
 * Bit 3:		HE UL-OFDMA support
 */
#define WL_HE_FEATURES_5G		0x0001
#define WL_HE_FEATURES_2G		0x0002
#define WL_HE_FEATURES_DLOMU		0x0004
#define WL_HE_FEATURES_ULOMU		0x0008
#define WL_HE_FEATURES_DLMMU		0x0010

/* features convenience macros */
#define WLC_HE_FEATURES_GET(pub, mask) ((pub)->he_features & (mask))
#define WLC_HE_FEATURES_SET(pub, mask) ((pub)->he_features |= (mask))
#define WLC_HE_FEATURES_CLR(pub, mask) ((pub)->he_features &= ~(mask))

#define WLC_HE_FEATURES_5G(pub)		(WLC_HE_FEATURES_GET(pub, WL_HE_FEATURES_5G) != 0)
#define WLC_HE_FEATURES_2G(pub)		(WLC_HE_FEATURES_GET(pub, WL_HE_FEATURES_2G) != 0)
#define WLC_HE_FEATURES_DLOMU(pub)	(WLC_HE_FEATURES_GET(pub, WL_HE_FEATURES_DLOMU) != 0)
#define WLC_HE_FEATURES_ULOMU(pub)	(WLC_HE_FEATURES_GET(pub, WL_HE_FEATURES_ULOMU) != 0)
#define WLC_HE_FEATURES_DLMMU(pub)	(WLC_HE_FEATURES_GET(pub, WL_HE_FEATURES_DLMMU) != 0)

/* main ENAB check macros */
#define HE_ENAB(pub)			((pub)->_he_enab)
#define HE_ENAB_BAND(pub, band) \
	(HE_ENAB(pub) && \
	 (BAND_6G(band) || \
	 (BAND_5G6G(band) ? WLC_HE_FEATURES_5G(pub) : \
	  BAND_2G(band) ? WLC_HE_FEATURES_2G(pub) : \
	  0)))

#define HE_SU_FRAGTX_ENAB(pub)            ((pub)->_he_fragtx & 0x1)
#define HE_MU_FRAGTX_ENAB(pub)            ((pub)->_he_fragtx & 0x2)
/* 11ax MU-MIMO/OFDMA support */
#define HE_DLMU_ENAB(pub)		(HE_ENAB(pub) && WLC_HE_FEATURES_DLOMU(pub))
#define HE_ULMU_ENAB(pub)		(HE_ENAB(pub) && WLC_HE_FEATURES_ULOMU(pub))
#define HE_DLMMU_ENAB(pub)		(HE_ENAB(pub) && WLC_HE_FEATURES_DLMMU(pub))
#define HE_MMU_ENAB(pub)		(HE_ENAB(pub) && WLC_HE_FEATURES_DLMMU(pub))

#if defined(WLWRR) && defined(BCMDBG)
	#if defined(DONGLEBUILD)
		#define WLC_WRR_ENAB(pub)		(TRUE)
	#else
		/* NIC Mode
		 * Enable WRR on NIC for corerev 0x81 (129) or greater
		 * Corerev 0x81 is 43684B1
		 */
		#define WLC_WRR_ENAB(pub)		(D11REV_GE((pub)->corerev, 129))
	#endif /* DONGLEBUILD */
#else
#define WLC_WRR_ENAB(pub)		(FALSE)
#endif /* WLWRR && BCMDBG */

#else
#define HE_ENAB(pub)			(0)
#define HE_ENAB_BAND(pub, band)		(FALSE)
#define HE_DLMU_ENAB(pub)		(FALSE)
#define HE_ULMU_ENAB(pub)		(FALSE)
#define HE_DLMMU_ENAB(pub)		(FALSE)
#define HE_MMU_ENAB(pub)		(FALSE)
#define WLC_WRR_ENAB(pub)		(FALSE)
#define HE_SU_FRAGTX_ENAB(pub)		(FALSE)
#define HE_MU_FRAGTX_ENAB(pub)		(FALSE)
#endif /* WL11AX */

/* 11ax/11ah TWT support */
#ifdef WLTWT
	#if defined(ROM_ENAB_RUNTIME_CHECK)
		#define TWT_ENAB(pub)	((pub)->_twt)
	#elif defined(WLTWT_DISABLED)
		#define TWT_ENAB(pub)	(FALSE)
	#else
		#define TWT_ENAB(pub)	((pub)->_twt)
	#endif
#else
	#define TWT_ENAB(pub)	(FALSE)
#endif	/* WLTWT */

/* HEB block support */
#ifdef WLHEB
	#if defined(ROM_ENAB_RUNTIME_CHECK)
		#define HEB_ENAB(pub)	((pub)->_heb)
	#elif defined(WLHEB_DISABLED)
		#define HEB_ENAB(pub)	(FALSE)
	#else
		#define HEB_ENAB(pub)	((pub)->_heb)
	#endif
#else
	#define HEB_ENAB(pub)	(FALSE)
#endif	/* WLHEB */

/* Open Loop Phy Calibration support */
/* only supported for 11AC chips */
#if defined(WLOLPC) && defined(WL11AC)
	#if defined(ROM_ENAB_RUNTIME_CHECK)
		#define OLPC_ENAB(wlc)  ((wlc)->pub->_olpc)
	#elif defined(WLOLPC_DISABLED)
		#define OLPC_ENAB(wlc)  (FALSE)
	#else
		#define OLPC_ENAB(wlc)  ((wlc)->pub->_olpc)
	#endif
#else
	#define OLPC_ENAB(wlc)  (FALSE)
#endif /* WLOLPC && WL11AC */

/* Constants for set/get iovar "ampdu_aggmode" */
#define AMPDU_AGGMODE_AUTO	-1
#define AMPDU_AGGMODE_HOST	1
#define AMPDU_AGGMODE_MAC	2

/* Constants for get iovar "ampdumac" */
#define AMPDU_AGG_OFF		0
#define AMPDU_AGG_UCODE		1
#define AMPDU_AGG_HW		2
#define AMPDU_AGG_AQM		3

/* WLAMPDU Support */
#ifdef WLAMPDU
#define AMPDU_ENAB(pub) ((pub)->_ampdu_tx && (pub)->_ampdu_rx)
#else
#define AMPDU_ENAB(pub) 0
#endif /* WLAMPDU */

/* WLAMPDUMAC Support */
#if ((defined(WLAMPDU_UCODE) && defined(WLAMPDU_AQM)) || \
	defined(ROM_ENAB_RUNTIME_CHECK) || !defined(DONGLEBUILD))
	#define AMPDU_UCODE_ENAB(pub)	((pub)->_ampdumac == AMPDU_AGG_UCODE)
	#define AMPDU_AQM_ENAB(pub)	((pub)->_ampdumac == AMPDU_AGG_AQM)
#elif defined(WLAMPDU_UCODE) && defined(WLAMPDU_UCODE_ONLY)
	#define AMPDU_UCODE_ENAB(pub)	(1)
	#define AMPDU_AQM_ENAB(pub)	(0)
#elif defined(WLAMPDU_AQM)
	#define AMPDU_UCODE_ENAB(pub)   (0)
	#define AMPDU_AQM_ENAB(pub)	(1)
#elif defined(WLAMPDU_UCODE)
	#define AMPDU_UCODE_ENAB(pub)	((pub)->_ampdumac == AMPDU_AGG_UCODE)
	#define AMPDU_AQM_ENAB(pub)	(0)
#endif /* various compile/run-time versions of AMPDU_xxx_ENAB() checks */

#define AMPDU_MAC_ENAB(pub) (AMPDU_UCODE_ENAB(pub) || AMPDU_AQM_ENAB(pub))
#define AMPDU_HOST_ENAB(pub) (!AMPDU_UCODE_ENAB(pub) && !AMPDU_AQM_ENAB(pub))

/* WOWL support */
#ifdef WOWL
#define WOWL_ENAB(pub) ((pub)->_wowl)
#define WOWL_ACTIVE(pub) ((pub)->_wowl_active)
#else
#define WOWL_ACTIVE(wlc) (0)
#define WOWL_ENAB(pub) (0)
#endif /* WOWL */

#ifdef WOWLPF
	#if defined(ROM_ENAB_RUNTIME_CHECK) || !defined(DONGLEBUILD)
		#define WOWLPF_ENAB(pub) ((pub)->_wowlpf)
		#define WOWLPF_ACTIVE(pub) ((pub)->_wowlpf_active)
	#elif defined(WOWLPF_DISABLED)
		#define WOWLPF_ENAB(pub) (0)
		#define WOWLPF_ACTIVE(pub) (0)
	#else
		#define WOWLPF_ENAB(pub) (1)
		#define WOWLPF_ACTIVE(pub) ((pub)->_wowlpf_active)
	#endif /* defined(ROM_ENAB_RUNTIME_CHECK) || !defined(DONGLEBUILD) */
#else
	#define WOWLPF_ACTIVE(wlc) (0)
	#define WOWLPF_ENAB(pub) (0)
#endif /* WOWLPF */

/* TDLS Support */
#ifdef WLTDLS
	#if defined(ROM_ENAB_RUNTIME_CHECK)
		#define TDLS_ENAB(pub)		((pub)->cmn->_tdls)
	#elif defined(WLTDLS_DISABLED)
		#define TDLS_ENAB(pub)		(0)
	#else
		#define TDLS_ENAB(pub)		((pub)->cmn->_tdls)
	#endif /* defined(ROM_ENAB_RUNTIME_CHECK) */
#else
	#define TDLS_ENAB(pub)			(0)
#endif /* WLTDLS */

/* DLS Support */
#ifdef WLDLS
#define WLDLS_ENAB(pub) ((pub)->_dls)
#else
#define WLDLS_ENAB(pub) 0
#endif /* WLDLS */
/* OKC Support */
#ifdef WL_OKC
#define OKC_ENAB(pub) ((pub)->cmn->_okc)
#else
#define OKC_ENAB(pub) 0
#endif /* WL_OKC */

#ifdef WLBSSLOAD
#define WLBSSLOAD_ENAB(pub)	((pub)->_bssload)
#else
#define WLBSSLOAD_ENAB(pub)	(0)
#endif /* WLBSSLOAD */

/* WLMCNX Support */
#ifdef WLMCNX
	#if !defined(DONGLEBUILD)
		#ifndef WLP2P_UCODE_ONLY
			#define MCNX_ENAB(pub) ((pub)->_mcnx)
		#else
			#define MCNX_ENAB(pub)	(1)
		#endif /* ifndef WLP2P_UCODE_ONLY */
	#elif defined(ROM_ENAB_RUNTIME_CHECK)
		#define MCNX_ENAB(pub) ((pub)->_mcnx)
	#elif !defined(WLMCNX_DISABLED)
		#define MCNX_ENAB(pub)	(1)
	#else
		#define MCNX_ENAB(pub)	(0)
	#endif /* !DONGLEBUILD */
#else
	#define MCNX_ENAB(pub) 0
#endif /* WLMCNX */

/* WLP2P Support */
#ifdef WLP2P
	#if defined(ROM_ENAB_RUNTIME_CHECK) || !defined(DONGLEBUILD)
		#define P2P_ENAB(pub) ((pub)->_p2p)
	#elif defined(WLP2P_DISABLED)
		#define P2P_ENAB(pub)	((void)(pub), 0)
	#else
		#define P2P_ENAB(pub)	((void)(pub), 1)
	#endif /* defined(ROM_ENAB_RUNTIME_CHECK) || !defined(DONGLEBUILD) */
	#ifdef WLWFDS
		#if defined(ROM_ENAB_RUNTIME_CHECK) || !defined(DONGLEBUILD)
			#define WFDS_ENAB(pub) ((pub)->_wfds)
		#elif defined(WLWFDS_DISABLED)
			#define WFDS_ENAB(pub) (0)
		#else
			#define WFDS_ENAB(pub) (1)
		#endif /* ROM_ENAB_RUNTIME_CHECK || !DONGLEBUILD */
	#else
		#define WFDS_ENAB(pub) (0)
	#endif /* WLWFDS */
#else
	#define P2P_ENAB(pub) ((void)(pub), 0)
	#define WFDS_ENAB(pub) 0
#endif /* WLP2P */

/* MFP Support */
#ifdef MFP
	#if defined(ROM_ENAB_RUNTIME_CHECK) || !defined(DONGLEBUILD)
		#define WLC_MFP_ENAB(pub) ((pub)->_mfp)
	#elif defined(MFP_DISABLED)
		#define WLC_MFP_ENAB(pub)	((void)(pub), 0)
	#else
		#define WLC_MFP_ENAB(pub)	((void)(pub), 1)
	#endif /* defined(ROM_ENAB_RUNTIME_CHECK) || !defined(DONGLEBUILD) */
#else
	#define WLC_MFP_ENAB(pub) ((void)(pub), 0)
#endif /* MFP */

/* WLMCHAN Support */
#ifdef WLMCHAN
	#if defined(ROM_ENAB_RUNTIME_CHECK) || !defined(DONGLEBUILD)
		#define MCHAN_ENAB(pub) ((pub)->_mchan)
		#define MCHAN_ACTIVE(pub) ((pub)->_mchan_active)
	#elif defined(WLMCHAN_DISABLED)
		#define MCHAN_ENAB(pub) (0)
		#define MCHAN_ACTIVE(pub) (0)
	#else
		#define MCHAN_ENAB(pub) ((pub)->_mchan)
		#define MCHAN_ACTIVE(pub) ((pub)->_mchan_active)
	#endif /* defined(ROM_ENAB_RUNTIME_CHECK) || !defined(DONGLEBUILD) */
#else
	#define MCHAN_ENAB(pub) (0)
	#define MCHAN_ACTIVE(pub) (0)
#endif /* WLMCHAN */

#ifdef WLRSDB
	#if defined(ROM_ENAB_RUNTIME_CHECK) || !defined(DONGLEBUILD)
		#define RSDB_ENAB(pub) ((pub)->cmn->_isrsdb)
		#define RSDB_ACTIVE(pub) ((pub)->cmn->_rsdb_active)
	#elif defined(WLRSDB_DISABLED)
		#define RSDB_ENAB(pub) (0)
		#define RSDB_ACTIVE(pub) (0)
	#else
		#define RSDB_ENAB(pub) (1)
		#define RSDB_ACTIVE(pub) ((pub)->cmn->_rsdb_active)
	#endif /* defined(ROM_ENAB_RUNTIME_CHECK) || !defined(DONGLEBUILD) */

	#if defined(ROM_ENAB_RUNTIME_CHECK) || !defined(DONGLEBUILD)
		#define RSDB_CMN_BANDSTATE_ENAB(pub) ((pub)->cmn->_isrsdb_cmn_bandstate)
	#elif defined(RSDB_CMN_BANDSTATE_DISABLED)
		#define RSDB_CMN_BANDSTATE_ENAB(pub) (0)
	#else
		#define RSDB_CMN_BANDSTATE_ENAB(pub) (1)
	#endif
#else
	#define RSDB_ENAB(pub) (0)
	#define RSDB_ACTIVE(pub) (0)
	#define RSDB_CMN_BANDSTATE_ENAB(pub) (0)
#endif /* WLRSDB */

/* BG DFS Radar scan */
#ifdef BGDFS
	#if defined(ROM_ENAB_RUNTIME_CHECK) || !defined(DONGLEBUILD)
		#define BGDFS_ENAB(pub)		((pub)->_bgdfs)
	#elif defined(BGDFS_DISABLED)
		#define BGDFS_ENAB(pub)		(0)
	#else
		#define BGDFS_ENAB(pub)		(1)
	#endif
	#define BGDFS_2G_ENAB(pub)	((pub)->_bgdfs_2g)
#else
	#define BGDFS_ENAB(pub)		(0)
	#define BGDFS_2G_ENAB(pub)	(0)
#endif /* BGDFS */

/* 160MHz mode is available only for 4365/4366 corerev 0x41 (c0) */
#define WL_HAS_DYN160(wlc)	D11REV_IS(wlc->pub->corerev, 0x41)

/* default values for dyn160_active state */
#define WL_DEFAULT_DYN160_INVALID		(-1) /* invalid; not configured by IOVAR yet */
#define WL_DEFAULT_DYN160_STA			(1)  /* enable by default on STA (when capable) */
#define WL_DEFAULT_DYN160_AP			(0)  /* disable by default on AP (when capable) */

/* get default value by cfg; all non-APs are treated the same as STA */
#define WL_DYN160_DEFAULT(cfg) (((cfg) != NULL && BSSCFG_AP(cfg)) ? \
		WL_DEFAULT_DYN160_AP : WL_DEFAULT_DYN160_STA)

/* get dyn160_active if valid or return default based on primary cfg */
#define WL_VALID_DYN160(pub) ((pub)->_dyn160_active == WL_DEFAULT_DYN160_INVALID ? \
		WL_DYN160_DEFAULT(((wlc_info_t *)(pub)->wlc)->cfg) : (pub)->_dyn160_active)

/* DYN160 - dynamic 160/80p80 at half NSS to 80 at full NSS */
#ifdef DYN160
	#if defined(ROM_ENAB_RUNTIME_CHECK) || !defined(DONGLEBUILD)
		/* eg. NIC with -dyn160 target or dongle with runtime check */
		#define DYN160_ENAB(pub)	((pub)->_dyn160)
		#define DYN160_ACTIVE(pub)	(DYN160_ENAB(pub) ? WL_VALID_DYN160(pub) : 0)
		#define DYN160_ACTIVE_SET(pub, val) ((pub)->_dyn160_active = \
				(DYN160_ENAB(pub) ? (val) : 0))
	#elif defined(DYN160_DISABLED)
		/* dongle with dyn160 target but disabled with -DDYN160_DISABLED */
		#define DYN160_ENAB(pub)	(0)
		#define DYN160_ACTIVE(pub)	(0)
		#define DYN160_ACTIVE_SET(pub, val)	do {} while (0)
	#else
		/* eg. dongle without runtime check and without DYN160_DISABLED */
		#define DYN160_ENAB(pub)	(1)
		#define DYN160_ACTIVE(pub)	(DYN160_ENAB(pub) ? WL_VALID_DYN160(pub) : 0)
		#define DYN160_ACTIVE_SET(pub, val) ((pub)->_dyn160_active = \
				(DYN160_ENAB(pub) ? (val) : 0))
	#endif /* ROM_ENAB_RUNTIME_CHECK || !DONGLEBUILD */
#else
	/* eg. NIC or dongle without -dyn160 target */
	#define DYN160_ENAB(pub)		(0)
	#define DYN160_ACTIVE(pub)		(0)
	#define DYN160_ACTIVE_SET(pub, val)	do {} while (0)
#endif /* DYN160 */

/* PIO Mode Support */
#ifdef WLPIO
#define PIO_ENAB(pub) ((pub)->_piomode)
#else
#define PIO_ENAB(pub) 0
#endif /* WLPIO */

/* Call Admission Control support */
#ifdef WLCAC
	#if defined(ROM_ENAB_RUNTIME_CHECK) || !defined(DONGLEBUILD)
		#define CAC_ENAB(pub)	((pub)->cmn->_cac)
	#elif defined(WLCAC_DISABLED)
		#define CAC_ENAB(pub)	0
	#else
		#define CAC_ENAB(pub)	((pub)->cmn->_cac)
	#endif /* defined(ROM_ENAB_RUNTIME_CHECK) || !defined(DONGLEBUILD) */
#else
	#define CAC_ENAB(pub) 0
#endif /* WLCAC */

#ifdef BCMSPACE
#define	RXIQEST_ENAB(pub)	(1)
#else
#define	RXIQEST_ENAB(pub)	(0)
#endif /* BCMSPACE */

#define QOS_ENAB(pub) (WME_ENAB(pub) || N_ENAB(pub))

#define PRIOFC_ENAB(pub) ((pub)->_priofc)

#ifdef WL_MONITOR
	#if defined(ROM_ENAB_RUNTIME_CHECK) || !defined(DONGLEBUILD)
		#define MONITOR_ENAB(wlc)	((wlc)->pub->_mon && (wlc)->monitor != 0)
	#elif defined(WL_MONITOR_DISABLED)
		#define MONITOR_ENAB(wlc) (0)
	#else
		#define MONITOR_ENAB(wlc) ((wlc)->monitor != 0)
	#endif
#else
	#define MONITOR_ENAB(wlc) (0)
#endif /* WL_MONITOR */

#if defined(WL_PROMISC)
#define PROMISC_ENAB(wlc_pub)	(wlc_pub)->promisc
#else
#define PROMISC_ENAB(wlc_pub)	(bcmspace && (wlc_pub)->promisc)
#endif /* defined(WL_PROMISC) */

#ifdef WLCSO
#define WLC_TSO_HDR_LEN(w, tso_hdr) ((w)->toe_bypass ? 0 : wlc_tso_hdr_length(tso_hdr))
#elif defined(WLAMPDU)
#define WLC_TSO_HDR_LEN(w, tso_hdr) TSO_HEADER_PASSTHROUGH_LENGTH
#else
#define WLC_TSO_HDR_LEN(w, tso_hdr) 0
#endif /* WLCSO */

#ifdef TOE
	#if defined(ROM_ENAB_RUNTIME_CHECK)
		#define TOE_ENAB(pub)		((pub)->_toe)
	#elif defined(TOE_DISABLED)
		#define TOE_ENAB(pub)		(0)
	#else
		#define TOE_ENAB(pub)		((pub)->_toe)
	#endif /* defined(ROM_ENAB_RUNTIME_CHECK) */
#else
	#define TOE_ENAB(pub)			(0)
#endif /* TOE */

#ifdef ARPOE
	#if defined(ROM_ENAB_RUNTIME_CHECK)
		#define ARPOE_SUPPORT(pub)	((pub)->_arpoe_support)
		#define ARPOE_ENAB(pub)		((pub)->_arpoe)
	#elif defined(ARPOE_DISABLED)
		#define ARPOE_SUPPORT(pub)	(0)
		#define ARPOE_ENAB(pub)		(0)
	#else
		#define ARPOE_SUPPORT(pub)	(1)
		#define ARPOE_ENAB(pub)		((pub)->_arpoe)
	#endif /* defined(ROM_ENAB_RUNTIME_CHECK) */
#else
	#define ARPOE_SUPPORT(pub)		(0)
	#define ARPOE_ENAB(pub)			(0)
#endif /* ARPOE */

#ifdef L2_FILTER
	#if defined(ROM_ENAB_RUNTIME_CHECK)
		#define L2_FILTER_ENAB(pub)	((pub)->_l2_filter)
	#elif defined(L2_FILTER_DISABLED)
		#define L2_FILTER_ENAB(pub)	(0)
	#else
		#define L2_FILTER_ENAB(pub)	(1)
	#endif /* defined(ROM_ENAB_RUNTIME_CHECK) */
#else
	#define L2_FILTER_ENAB(pub)		(0)
#endif /* L2_FILTER */

#ifdef PACKET_FILTER
	#if defined(ROM_ENAB_RUNTIME_CHECK)
		#define PKT_FILTER_ENAB(pub)	((pub)->_pkt_filter)
	#elif defined(PKT_FLT_DISABLED)
		#define PKT_FILTER_ENAB(pub)	(0)
	#else
		#define PKT_FILTER_ENAB(pub)	(1)
	#endif /* defined(ROM_ENAB_RUNTIME_CHECK) */
#else
	#define PKT_FILTER_ENAB(pub)		(0)
#endif /* PACKET_FILTER */

#ifdef APF
	#if defined(ROM_ENAB_RUNTIME_CHECK)
		#define APF_ENAB(pub)		((pub)->_apf_filter)
	#elif defined(APF_DISABLED)
		#define APF_ENAB(pub)		(0)
	#else
		#define APF_ENAB(pub)		(1)
	#endif
#else
	#define APF_ENAB(pub)			(0)
#endif /* APF */

#ifdef PACKET_FILTER2
	#if defined(ROM_ENAB_RUNTIME_CHECK)
		#define PKT_FILTER2_ENAB(pub)	((pub)->_pkt_filter2)
	#elif defined(PKT_FLT2_DISABLED)
		#define PKT_FILTER2_ENAB(pub)	(0)
	#else
		#define PKT_FILTER2_ENAB(pub)	(1)
	#endif /* defined(ROM_ENAB_RUNTIME_CHECK) */
#else
	#define PKT_FILTER2_ENAB(pub)		(0)
#endif /* PACKET_FILTER2 */

#ifdef PACKET_FILTER6
	#if defined(ROM_ENAB_RUNTIME_CHECK)
		#define PKT_FILTER6_ENAB(pub)   ((pub)->_pkt_filter6)
	#elif defined(PKT_FLT6_DISABLED)
		#define PKT_FILTER6_ENAB(pub)   (0)
	#else
		#define PKT_FILTER6_ENAB(pub)   (1)
	#endif
#else
	#define PKT_FILTER6_ENAB(pub)           (0)
#endif /* PACKET_FILTER6 */

#ifdef D0_COALESCING
	#if defined(ROM_ENAB_RUNTIME_CHECK) || !defined(DONGLEBUILD)
		#define D0_FILTER_ENAB(pub)	((pub)->_d0_filter)
	#elif defined(D0_FILTER_DISABLED)
		#define D0_FILTER_ENAB(pub)	(0)
	#else
		#define D0_FILTER_ENAB(pub)	(1)
	#endif /* defined(ROM_ENAB_RUNTIME_CHECK) || !defined(DONGLEBUILD) */
#else
	#define D0_FILTER_ENAB(pub)	0
#endif /* D0_COALESCING */

#ifdef P2PO
	#if defined(ROM_ENAB_RUNTIME_CHECK) || !defined(DONGLEBUILD)
		#define P2PO_ENAB(pub) ((pub)->_p2po)
	#elif defined(P2PO_DISABLED)
		#define P2PO_ENAB(pub)	(0)
	#else
		#define P2PO_ENAB(pub)	(1)
	#endif /* defined(ROM_ENAB_RUNTIME_CHECK) || !defined(DONGLEBUILD) */
#else
	#define P2PO_ENAB(pub) 0
#endif /* P2PO */

#ifdef ANQPO
	#if defined(ROM_ENAB_RUNTIME_CHECK) || !defined(DONGLEBUILD)
		#define ANQPO_ENAB(pub) ((pub)->_anqpo)
	#elif defined(ANQPO_DISABLED)
		#define ANQPO_ENAB(pub)	(0)
	#else
		#define ANQPO_ENAB(pub)	(1)
	#endif /* defined(ROM_ENAB_RUNTIME_CHECK) || !defined(DONGLEBUILD) */
#else
	#define ANQPO_ENAB(pub) 0
#endif /* ANQPO */

#ifdef BDO
	#if defined(ROM_ENAB_RUNTIME_CHECK)
		#define BDO_SUPPORT(pub)	((pub)->_bdo_support)
		#define BDO_ENAB(pub)		((pub)->_bdo)
	#elif defined(BDO_DISABLED)
		#define BDO_SUPPORT(pub)	(0)
		#define BDO_ENAB(pub)		(0)
	#else
		#define BDO_SUPPORT(pub)	(1)
		#define BDO_ENAB(pub)		((pub)->_bdo)
	#endif
#else
	#define BDO_SUPPORT(pub)		(0)
	#define BDO_ENAB(pub)			(0)
#endif /* BDO */

#ifdef ICMP
	#if defined(ROM_ENAB_RUNTIME_CHECK) || !defined(DONGLEBUILD)
		#define ICMP_ENAB(pub) ((pub)->_icmp)
	#elif defined(ICMP_DISABLED)
		#define ICMP_ENAB(pub)	(0)
	#else
		#define ICMP_ENAB(pub)	(1)
	#endif
#else
	#define ICMP_ENAB(pub) 0
#endif /* ICMP */

#ifdef WL_ASSOC_RECREATE
	#if defined(ROM_ENAB_RUNTIME_CHECK) || !defined(DONGLEBUILD)
		#define ASSOC_RECREATE_ENAB(pub) ((pub)->_assoc_recreate)
	#elif defined(ASSOC_RECREATE_DISABLED)
		#define ASSOC_RECREATE_ENAB(pub)	(0)
	#else
		#define ASSOC_RECREATE_ENAB(pub)	(1)
	#endif /* defined(ROM_ENAB_RUNTIME_CHECK) || !defined(DONGLEBUILD) */
#else
#define ASSOC_RECREATE_ENAB(pub) 0
#endif /* WL_ASSOC_RECREATE */

#ifdef WLFBT
	#if defined(ROM_ENAB_RUNTIME_CHECK) || !defined(DONGLEBUILD)
		#define WLFBT_CAP(pub) ((pub)->_fbt_cap)
		#define WLFBT_ENAB(pub) ((pub)->_fbt)
	#elif defined(WLFBT_DISABLED)
		#define WLFBT_CAP(pub) ((void)(pub), 0)
		#define WLFBT_ENAB(pub) ((void)(pub), 0)
	#else
		#define WLFBT_CAP(pub) ((pub)->_fbt_cap)
		#define WLFBT_ENAB(pub) ((pub)->_fbt)
	#endif /* defined(ROM_ENAB_RUNTIME_CHECK) || !defined(DONGLEBUILD) */
#else
	#define WLFBT_CAP(pub) ((void)(pub), 0)
	#define WLFBT_ENAB(pub) ((void)(pub), 0)
#endif /* WLFBT */

#if defined(WL_ASSOC_MGR)
	#if defined(ROM_ENAB_RUNTIME_CHECK) || !defined(DONGLEBUILD)
		#define WL_ASSOC_MGR_ENAB(pub) ((pub)->_assoc_mgr)
	#elif defined(WL_ASSOC_MGR_DISABLED)
		#define WL_ASSOC_MGR_ENAB(pub) ((void)(pub), 0)
	#else
		#define WL_ASSOC_MGR_ENAB(pub) ((void)(pub), 1)
	#endif
#else
	#define WL_ASSOC_MGR_ENAB(pub) ((void)(pub), 0)
#endif /* defined(WL_ASSOC_MGR) */

#ifdef WLNDOE
	#if defined(ROM_ENAB_RUNTIME_CHECK) || !defined(DONGLEBUILD)
		#define NDOE_SUPPORT(pub)	((pub)->_ndoe_support)
		#define NDOE_ENAB(pub) ((pub)->_ndoe)
	#elif defined(WLNDOE_DISABLED)
		#define NDOE_SUPPORT(pub)	0
		#define NDOE_ENAB(pub) 0
	#else
		#define NDOE_SUPPORT(pub)	1
		#define NDOE_ENAB(pub) ((pub)->_ndoe)
	#endif /* defined(ROM_ENAB_RUNTIME_CHECK) || !defined(DONGLEBUILD) */
#else
	#define NDOE_SUPPORT(pub) 0
	#define NDOE_ENAB(pub) 0
#endif /* WLNDOE */

#ifdef GTKOE
	#if defined(ROM_ENAB_RUNTIME_CHECK) || !defined(DONGLEBUILD)
		#define GTKOE_ENAB(pub) ((pub)->_gtkoe)
	#elif defined(GTKOE_DISABLED)
		#define GTKOE_ENAB(pub) 0
	#else
		#define GTKOE_ENAB(pub) 1
	#endif /* defined(ROM_ENAB_RUNTIME_CHECK) || !defined(DONGLEBUILD) */
#endif /* GTKOE */

	#define IBSS_PEER_GROUP_KEY_ENAB(pub) (0)

	#define IBSS_PEER_DISCOVERY_EVENT_ENAB(pub) (0)

	#define IBSS_PEER_MGMT_ENAB(pub) (0)

#if !defined(WLNOEIND)
	#if defined(ROM_ENAB_RUNTIME_CHECK) || !defined(DONGLEBUILD)
		#define WLEIND_ENAB(pub) ((pub)->_wleind)
	#elif defined(WLEIND_DISABLED)
		#define WLEIND_ENAB(pub) (0)
	#else
		#define WLEIND_ENAB(pub) (1)
	#endif /* defined(ROM_ENAB_RUNTIME_CHECK) || !defined(DONGLEBUILD) */
#else
	#define WLEIND_ENAB(pub) (0)
#endif /* ! WLNOEIND */

#ifdef WLRM
	#if defined(ROM_ENAB_RUNTIME_CHECK) || !defined(DONGLEBUILD)
		#define WLRM_ENAB(pub) ((pub)->_rm)
	#elif defined(WLRM_DISABLED)
		#define WLRM_ENAB(pub) ((void)(pub), 0)
	#else
		#define WLRM_ENAB(pub) ((void)(pub), 1)
	#endif /* defined(ROM_ENAB_RUNTIME_CHECK) || !defined(DONGLEBUILD) */
#else
	#define WLRM_ENAB(pub) ((void)(pub), 0)
#endif  /* WLRM */

#ifdef BCMAUTH_PSK
	#if defined(ROM_ENAB_RUNTIME_CHECK) || !defined(DONGLEBUILD)
		#define BCMAUTH_PSK_ENAB(pub) ((pub)->_bcmauth_psk)
	#elif defined(BCMAUTH_PSK_DISABLED)
		#define BCMAUTH_PSK_ENAB(pub) (0)
	#else
		#define BCMAUTH_PSK_ENAB(pub) (1)
	#endif /* defined(ROM_ENAB_RUNTIME_CHECK) || !defined(DONGLEBUILD) */
#else
	#define BCMAUTH_PSK_ENAB(pub) 0
#endif /* BCMAUTH_PSK */

#ifdef BCMSUP_PSK
	#if defined(ROM_ENAB_RUNTIME_CHECK) || !defined(DONGLEBUILD)
		#define SUP_ENAB(pub)	((pub)->_sup_enab)
	#elif defined(BCMSUP_PSK_DISABLED)
		#define SUP_ENAB(pub)	((void)(pub), 0)
	#else
		#define SUP_ENAB(pub)	((void)(pub), 1)
	#endif /* defined(ROM_ENAB_RUNTIME_CHECK) || !defined(DONGLEBUILD) */
#else
	#define SUP_ENAB(pub)	((void)(pub), 0)
#endif /* BCMSUP_PSK */

#ifdef WL11K
	#if defined(ROM_ENAB_RUNTIME_CHECK) || !defined(DONGLEBUILD)
		#define WL11K_ENAB(pub)	((pub)->_rrm)
	#elif defined(WL11K_DISABLED)
		#define WL11K_ENAB(pub)	((void)(pub), 0)
	#else
		#define WL11K_ENAB(pub)	((void)(pub), 1)
	#endif /* defined(ROM_ENAB_RUNTIME_CHECK) || !defined(DONGLEBUILD) */
#else
	#define WL11K_ENAB(pub)	((void)(pub), 0)
#endif /* WL11K */

#ifdef WLWNM
	#if defined(ROM_ENAB_RUNTIME_CHECK) || !defined(DONGLEBUILD)
		#define WLWNM_ENAB(pub)	((pub)->cmn->_wnm)
	#elif defined(WLWNM_DISABLED)
		#define WLWNM_ENAB(pub)	(0)
	#else
		#define WLWNM_ENAB(pub)	(1)
	#endif /* defined(ROM_ENAB_RUNTIME_CHECK) || !defined(DONGLEBUILD) */
#else
	#define WLWNM_ENAB(pub)	(0)
#endif /* WLWNM */

#ifdef WLWNM_BRCM
	#if defined(ROM_ENAB_RUNTIME_CHECK) || !defined(DONGLEBUILD)
		#define WLWNM_BRCM_ENAB(pub)	((pub)->_wnm_brcm)
	#elif defined(WLWNM_BRCM_DISABLED)
		#define WLWNM_BRCM_ENAB(pub)	(0)
	#else
		#define WLWNM_BRCM_ENAB(pub)	(1)
	#endif /* defined(ROM_ENAB_RUNTIME_CHECK) || !defined(DONGLEBUILD) */
#else
	#define WLWNM_BRCM_ENAB(pub)	(0)
#endif /* WLWNM_BRCM */

#ifdef WNM_BSSTRANS_EXT
	#if defined(ROM_ENAB_RUNTIME_CHECK) || !defined(DONGLEBUILD)
		#define WBTEXT_ENAB(pub)	((pub)->_bsstrans_ext)
		#define WBTEXT_ACTIVE(pub)	((pub)->_bsstrans_ext_active)
	#elif defined(WNM_BSSTRANS_EXT_DISABLED)
		#define WBTEXT_ENAB(pub)	(0)
		#define WBTEXT_ACTIVE(pub)	(0)
	#else
		#define WBTEXT_ENAB(pub)	((pub)->_bsstrans_ext)
		#define WBTEXT_ACTIVE(pub)	((pub)->_bsstrans_ext_active)
	#endif
#else
	#define WBTEXT_ENAB(pub)	(0)
	#define WBTEXT_ACTIVE(pub)	(0)
#endif /* WNM_BSSTRANS_EXT */

#ifdef WLOVERTHRUSTER
	#if defined(ROM_ENAB_RUNTIME_CHECK) || !defined(DONGLEBUILD)
		#define OVERTHRUST_ENAB(pub)	((pub)->_wloverthruster)
	#elif defined(WLOVERTHRUSTER_DISABLED)
		#define OVERTHRUST_ENAB(pub)	(0)
	#else
		#define OVERTHRUST_ENAB(pub)	(1)
	#endif /* defined(ROM_ENAB_RUNTIME_CHECK) || !defined(DONGLEBUILD) */
#else
	#define OVERTHRUST_ENAB(pub) 	(0)
#endif /* WLOVERTHRUSTER */

#ifdef TINY_PKTJOIN
	#if defined(ROM_ENAB_RUNTIME_CHECK) || !defined(DONGLEBUILD)
		#define TINY_PKTJOIN_ENAB(pub)	((pub)->_tiny_pktjoin)
	#elif defined(TINY_PKTJOIN_DISABLED)
		#define TINY_PKTJOIN_ENAB(pub) 	(0)
	#else
		#define	TINY_PKTJOIN_ENAB(pub) 	(1)
	#endif /* defined(ROM_ENAB_RUNTIME_CHECK) || !defined(DONGLEBUILD) */
#else
	#define TINY_PKTJOIN_ENAB(pub) 		(0)
#endif /* TINY_PKTJOIN */

#ifdef WL_RXEARLYRC
	#if defined(ROM_ENAB_RUNTIME_CHECK) || !defined(DONGLEBUILD)
		#define WL_RXEARLYRC_ENAB(pub)	((pub)->_wl_rxearlyrc)
	#elif defined(WL_RXEARLYRC_DISABLED)
		#define WL_RXEARLYRC_ENAB(pub) 	(0)
	#else
		#define	WL_RXEARLYRC_ENAB(pub) 	(1)
	#endif /* defined(ROM_ENAB_RUNTIME_CHECK) || !defined(DONGLEBUILD) */
#else
	#define WL_RXEARLYRC_ENAB(pub) 		(0)
#endif /* WL_RXEARLYRC */

/* rxfifo overflow handler */
#ifdef WLRXOV
	#if defined(ROM_ENAB_RUNTIME_CHECK) || !defined(DONGLEBUILD)
		#define WLRXOV_ENAB(pub)	((pub)->_rxov)
	#elif defined(WLRXOV_DISABLED)
		#define WLRXOV_ENAB(pub)	(0)
	#else
		#define	WLRXOV_ENAB(pub)	(1)
	#endif /* defined(ROM_ENAB_RUNTIME_CHECK) || !defined(DONGLEBUILD) */
#else
	#define WLRXOV_ENAB(pub)		(0)
#endif /* WLRXOV */

#ifdef WLPFN
	#if defined(ROM_ENAB_RUNTIME_CHECK) || !defined(DONGLEBUILD)
		#define WLPFN_ENAB(pub) ((pub)->cmn->_wlpfn)
	#elif defined(WLPFN_DISABLED)
		#define WLPFN_ENAB(pub) (0)
	#else
		#define WLPFN_ENAB(pub) (1)
	#endif /* defined(ROM_ENAB_RUNTIME_CHECK) || !defined(DONGLEBUILD) */
#else
	#define WLPFN_ENAB(pub) (0)
#endif /* WLPFN */

#ifdef WL_PROXDETECT
	#if defined(ROM_ENAB_RUNTIME_CHECK)
		#define PROXD_ENAB(pub)	((pub)->_proxd)
	#elif defined(WL_PROXDETECT_DISABLED)
		#define PROXD_ENAB(pub)	(0)
	#else
		#define PROXD_ENAB(pub)	((pub)->_proxd)
	#endif /* defined(ROM_ENAB_RUNTIME_CHECK) */
	#ifdef WL_PROXD_UCODE_TSYNC
		#if defined(ROM_ENAB_RUNTIME_CHECK)
			#define PROXD_ENAB_UCODE_TSYNC(pub)	((pub)->cmn->_proxd_ucode_tsync)
		#elif defined(WL_PROXDETECT_DISABLED)
			#define PROXD_ENAB_UCODE_TSYNC(pub)	(0)
		#else
			#define PROXD_ENAB_UCODE_TSYNC(pub)	((pub)->cmn->_proxd_ucode_tsync)
		#endif
	#else
		#define PROXD_ENAB_UCODE_TSYNC(pub) (0)
	#endif /* WL_PROXD_UCODE_TSYNC */
#else
	#define PROXD_ENAB(pub)		(0)
	#define PROXD_ENAB_UCODE_TSYNC(pub)	(0)
#endif /* WL_PROXDETECT */

#ifdef WL_BTCDYN
	#if defined(ROM_ENAB_RUNTIME_CHECK)
		#define BTCDYN_ENAB(pub)        ((pub)->_btcdyn)
	#elif defined(WL_BTCDYN_DISABLED)
		#define BTCDYN_ENAB(pub)        (0)
	#else
		#define BTCDYN_ENAB(pub)        ((pub)->_btcdyn)
	#endif
	#else
		#define BTCDYN_ENAB(pub)                (0)
#endif /* WL_BTCDYN */

	#define WLC_NAN_SET_ENAB(wlc, enable)
	#define NAN_ENAB(pub)		(0)

#ifdef WLSCANCACHE
	#if defined(ROM_ENAB_RUNTIME_CHECK) || !defined(DONGLEBUILD)
		#define SCANCACHE_SUPPORT(wlc)	((wlc)->pub->cmn->_scancache_support)
	#elif defined(WLSCANCACHE_DISABLED)
		#define SCANCACHE_SUPPORT(wlc)	(0)
	#else
		#define SCANCACHE_SUPPORT(wlc)	(1)
	#endif /* defined(ROM_ENAB_RUNTIME_CHECK) || !defined(DONGLEBUILD) */
#else
	#define SCANCACHE_SUPPORT(wlc)		(0)
#endif /* WLSCANCACHE */

#ifdef WLFMC
	#if defined(ROM_ENAB_RUNTIME_CHECK) || !defined(DONGLEBUILD)
		#define WLFMC_ENAB(pub) ((pub)->_fmc)
	#elif defined(WLFMC_DISABLED)
		#define WLFMC_ENAB(pub) (0)
	#else
		#define WLFMC_ENAB(pub) (1)
	#endif /* defined(ROM_ENAB_RUNTIME_CHECK) || !defined(DONGLEBUILD) */
#else
	#define WLFMC_ENAB(pub) 0
#endif /* WLFMC */

#ifdef WLRCC
	#if defined(ROM_ENAB_RUNTIME_CHECK) || !defined(DONGLEBUILD)
		#define WLRCC_ENAB(pub) ((pub)->_rcc)
	#elif defined(WLRCC_DISABLED)
		#define WLRCC_ENAB(pub) (0)
	#else
		#define WLRCC_ENAB(pub) (1)
	#endif /* defined(ROM_ENAB_RUNTIME_CHECK) || !defined(DONGLEBUILD) */
#else
	#define WLRCC_ENAB(pub) 0
#endif /* WLRCC */

/* Adaptive Beacon Timeout */
#ifdef WLABT
	#if defined(ROM_ENAB_RUNTIME_CHECK) || !defined(DONGLEBUILD)
		#define WLABT_ENAB(pub) ((pub)->_abt)
	#elif defined(WLABT_DISABLED)
		#define WLABT_ENAB(pub) (0)
	#else
		#define WLABT_ENAB(pub) (1)
	#endif /* defined(ROM_ENAB_RUNTIME_CHECK) || !defined(DONGLEBUILD) */
#else
	#define WLABT_ENAB(pub) 0
#endif /* WLABT */

#if defined(WLAMPDU_HOSTREORDER)
	#if defined(ROM_ENAB_RUNTIME_CHECK)
		#define AMPDU_HOST_REORDER_ENAB(pub)		(pub->_ampdu_hostreorder)
	#elif defined(AMPDU_HOSTREORDER_DISABLED)
		#define AMPDU_HOST_REORDER_ENAB(pub)		FALSE
	#else
		#define AMPDU_HOST_REORDER_ENAB(pub)		(pub->_ampdu_hostreorder)
	#endif /* defined(ROM_ENAB_RUNTIME_CHECK) */
#else
	#define AMPDU_HOST_REORDER_ENAB(pub)			FALSE
#endif /* WLAMPDU_HOSTREORDER */

#ifdef WLAMSDU_TX
	#if defined(ROM_ENAB_RUNTIME_CHECK)
		#define AMSDU_TX_SUPPORT(pub)	((pub)->_amsdu_tx_support)
		#define AMSDU_TX_ENAB(pub)	((pub)->_amsdu_tx)
		#define AMSDU_2G_ENAB(pub)	((pub)->_amsdu_2g)
	#elif defined(WLAMSDU_TX_DISABLED)
		#define AMSDU_TX_SUPPORT(pub)	(0)
		#define AMSDU_TX_ENAB(pub)	(0)
		#define AMSDU_2G_ENAB(pub)	(0)
	#else
		#define AMSDU_TX_SUPPORT(pub)	(1)
		#define AMSDU_TX_ENAB(pub)	((pub)->_amsdu_tx)
		#define AMSDU_2G_ENAB(pub)	((pub)->_amsdu_2g)
	#endif /* defined(ROM_ENAB_RUNTIME_CHECK) */
	#define	AMSDU_TX_AC_ENAB(ami, tid)\
		(wlc_amsdu_chk_priority_enable((ami), (tid)))
#else
	#define AMSDU_TX_SUPPORT(pub)	(0)
	#define AMSDU_TX_ENAB(pub)	(0)
	#define AMSDU_2G_ENAB(pub)	(0)
	#define AMSDU_TX_AC_ENAB(ami, tid) (0)
#endif /* WLAMSDU_TX */

#ifdef WLNFC
	#if defined(ROM_ENAB_RUNTIME_CHECK) || !defined(DONGLEBUILD)
		#define NFC_ENAB(pub) ((pub)->_nfc)
	#elif defined(WLNFC_DISABLED)
		#define NFC_ENAB(pub)	(0)
	#else
		#define NFC_ENAB(pub) ((pub)->_nfc)
	#endif /* defined(ROM_ENAB_RUNTIME_CHECK) || !defined(DONGLEBUILD) */
#else
	#define NFC_ENAB(pub) (0)
#endif /* WLNFC */

#ifdef GSCAN
/* XXX Gscan refers to Google's specific requirements and extensions to BRCM's current
 * implementation of PNO.
 */
	#if defined(ROM_ENAB_RUNTIME_CHECK) || !defined(DONGLEBUILD)
		#define GSCAN_ENAB(pub) ((pub)->cmn->_gscan)
	#elif defined(WL_GSCAN_DISABLED)
		#define GSCAN_ENAB(pub) (0)
	#else
		#define GSCAN_ENAB(pub) (1)
	#endif /* defined(ROM_ENAB_RUNTIME_CHECK) || !defined(DONGLEBUILD) */
#else
	#define GSCAN_ENAB(pub) (0)
#endif /* GSCAN */

#ifdef FCC_PWR_LIMIT_2G
	#if defined(ROM_ENAB_RUNTIME_CHECK) || !defined(DONGLEBUILD)
		#define FCC_PWR_LIMIT_2G_ENAB(pub) ((pub)->_fccpwrlimit2g)
	#elif defined(FCC_PWR_LIMIT_2G_DISABLED)
		#define FCC_PWR_LIMIT_2G_ENAB(pub) (0)
	#else
		#define FCC_PWR_LIMIT_2G_ENAB(pub) ((pub)->_fccpwrlimit2g)
	#endif
#else
	#define FCC_PWR_LIMIT_2G_ENAB(pub) (0)
#endif /* FCC_PWR_LIMIT_2G */

#ifdef WL_DTS
	#if defined(ROM_ENAB_RUNTIME_CHECK)
		#define DTS_ENAB(pub)		((pub)->_dts)
	#elif defined(WL_DTS_DISABLED)
		#define DTS_ENAB(pub)		(0)
	#else
		#define DTS_ENAB(pub)		((pub)->_dts)
	#endif
#else
	#define DTS_ENAB(pub)			(0)
#endif /* WL_DTS */

#if defined(WL_PRQ_RAND_SEQ)
	#if defined(ROM_ENAB_RUNTIME_CHECK) || !defined(DONGLEBUILD)
		#define PRQ_RAND_SEQ_ENAB(pub) ((pub)->_prq_rand_seq)
	#elif defined(WL_RAND_SEQ_DISABLED)
		#define PRQ_RAND_SEQ_ENAB(pub) (0)
	#else
		#define PRQ_RAND_SEQ_ENAB(pub) ((pub)->_prq_rand_seq)
	#endif
#else
	#define PRQ_RAND_SEQ_ENAB(pub) (0)
#endif /* WL_PRQ_RAND_SEQ */

/* PM2 Receive Throttle Duty Cycle */
#if defined(WL_PM2_RCV_DUR_LIMIT)
#define WLC_PM2_RCV_DUR_MIN		(10)	/**< 10% of beacon interval */
#define WLC_PM2_RCV_DUR_MAX		(80)	/**< 80% of beacon interval */
#define PM2_RCV_DUR_ENAB(cfg) ((cfg)->pm->pm2_rcv_percent > 0)
#else
#define PM2_RCV_DUR_ENAB(cfg) 0
#endif /* WL_PM2_RCV_DUR_LIMIT */

/* Default PM2 Return to Sleep value, in ms */
#define PM2_SLEEP_RET_MS_DEFAULT 200

/* PM2 mode with periodic PSPoll More data bit behaviour */
#define PM2_MD_SLEEP_EXT_DISABLED	0
#define PM2_MD_SLEEP_EXT_USE_SHORT_FRTS	1
#define PM2_MD_SLEEP_EXT_USE_BCN_FRTS	2

#ifdef PROP_TXSTATUS
#define WLFC_PKTAG_INFO_MOVE(pkt_from, pkt_to)	\
	do { \
		WLPKTTAG(pkt_to)->wl_hdr_information = WLPKTTAG(pkt_from)->wl_hdr_information; \
		WLPKTTAG(pkt_from)->wl_hdr_information = 0; \
		WLPKTTAG(pkt_to)->seq = WLPKTTAG(pkt_from)->seq; \
	} while (0)
#else
#define WLFC_PKTAG_INFO_MOVE(pkt_from, pkt_to)	do {} while (0)
#endif /* PROP_TXSTATUS */

#ifdef WL_RANDMAC
	#if defined(ROM_ENAB_RUNTIME_CHECK)
		#define RANDMAC_ENAB(pub)	((pub)->_randmac)
	#elif defined(WL_RANDMAC_DISABLED)
		#define RANDMAC_ENAB(pub)	(0)
	#else
		#define RANDMAC_ENAB(pub)	((pub)->_randmac)
	#endif
#else
	#define RANDMAC_ENAB(pub)		(0)
#endif /* WL_RANDMAC */

extern void wlc_pkttag_info_move(wlc_info_t *wlc, void *pkt_from, void *pkt_to);
extern void wlc_pkttag_info_frag_cpy(wlc_info_t *wlc, void *pkt_from, void *pkt_to);

#define	WLC_PREC_COUNT		16 /**< Max precedence level implemented */

/* pri is PKTPRIO encoded in the packet. This maps the Packet priority to
 * enqueue precedence as defined in wlc_prec_map
 */
extern const uint8 wlc_prio2prec_map[];
#define WLC_PRIO_TO_PREC(pri)	wlc_prio2prec_map[(pri) & 7]

extern const uint8 wlc_prec2prio_map[];
#define WLC_PREC_TO_PRIO(prec)  wlc_prec2prio_map[(prec) & 15]

/* This maps priority to one precedence higher - Used by PS-Poll response packets to
 * simulate enqueue-at-head operation, but still maintain the order on the queue
 */
#define WLC_PRIO_TO_HI_PREC(pri)	MIN(WLC_PRIO_TO_PREC(pri) + 1, WLC_PREC_COUNT - 1)

#define WME_PRIO2AC(prio)	prio2ac[(prio)]

extern const uint8 wme_fifo2ac[NFIFO_LEGACY];
extern const uint8 fifo2prio[NFIFO_LEGACY];

#endif /* LINUX_POSTMOGRIFY_REMOVAL */

extern const uint8 tid_service_order[NUMPRIO];

/* Mask to describe all precedence levels */
#define WLC_PREC_BMP_ALL		MAXBITVAL(WLC_PREC_COUNT)

/* Define a bitmap of precedences comprised by each AC */
#define WLC_PREC_BMP_AC_BE	(NBITVAL(WLC_PRIO_TO_PREC(PRIO_8021D_BE)) |	\
				NBITVAL(WLC_PRIO_TO_HI_PREC(PRIO_8021D_BE)) |	\
				NBITVAL(WLC_PRIO_TO_PREC(PRIO_8021D_EE)) |	\
				NBITVAL(WLC_PRIO_TO_HI_PREC(PRIO_8021D_EE)))
#define WLC_PREC_BMP_AC_BK	(NBITVAL(WLC_PRIO_TO_PREC(PRIO_8021D_BK)) |	\
				NBITVAL(WLC_PRIO_TO_HI_PREC(PRIO_8021D_BK)) |	\
				NBITVAL(WLC_PRIO_TO_PREC(PRIO_8021D_NONE)) |	\
				NBITVAL(WLC_PRIO_TO_HI_PREC(PRIO_8021D_NONE)))
#define WLC_PREC_BMP_AC_VI	(NBITVAL(WLC_PRIO_TO_PREC(PRIO_8021D_CL)) |	\
				NBITVAL(WLC_PRIO_TO_HI_PREC(PRIO_8021D_CL)) |	\
				NBITVAL(WLC_PRIO_TO_PREC(PRIO_8021D_VI)) |	\
				NBITVAL(WLC_PRIO_TO_HI_PREC(PRIO_8021D_VI)))
#define WLC_PREC_BMP_AC_VO	(NBITVAL(WLC_PRIO_TO_PREC(PRIO_8021D_VO)) |	\
				NBITVAL(WLC_PRIO_TO_HI_PREC(PRIO_8021D_VO)) |	\
				NBITVAL(WLC_PRIO_TO_PREC(PRIO_8021D_NC)) |	\
				NBITVAL(WLC_PRIO_TO_HI_PREC(PRIO_8021D_NC)))

/* WME Support */
#ifdef WME
#define WME_ENAB(pub) ((pub)->_wme != OFF)
#define WME_AUTO(wlc) ((wlc)->pub->_wme == AUTO)
#else
#define WME_ENAB(pub) ((void)(pub), 0)
#define WME_AUTO(wlc) (0)
#endif /* WME */

/* Auto Country */
#ifdef WLCNTRY
#define WLC_AUTOCOUNTRY_ENAB(wlc) ((wlc)->pub->cmn->_autocountry)
#else
#define WLC_AUTOCOUNTRY_ENAB(wlc) FALSE
#endif /* WLCNTRY */

/* Regulatory Domain -- 11D Support */
#ifdef WL11D
#define WL11D_ENAB(wlc)	((wlc)->pub->cmn->_11d)
#else
#define WL11D_ENAB(wlc)	((void)(wlc), FALSE)
#endif /* WL11D */

/* Spectrum Management -- 11H Support */
#ifdef WL11H
#define WL11H_ENAB(wlc)	((wlc)->pub->cmn->_11h)
#else
#define WL11H_ENAB(wlc)	((void)(wlc), FALSE)
#endif /* WL11H */

/* SLAVE RADAR Support */
#if defined(BAND5G) && defined(SLAVE_RADAR)
#define WL11H_STA_ENAB(wlc)	(BSSCFG_STA((wlc)->cfg) && WL11H_ENAB(wlc))
#else
#define WL11H_STA_ENAB(wlc)	0
#endif /* BAND5G && SLAVE_RADAR */

/* Interworking -- 11u Support */
#ifdef WL11U
	#if defined(ROM_ENAB_RUNTIME_CHECK)
		#define WL11U_ENAB(wlc)	((wlc)->pub->_11u)
	#elif defined(WL11U_DISABLED)
		#define WL11U_ENAB(wlc)	((void)(wlc), 0)
	#else
		#define WL11U_ENAB(wlc)	((void)(wlc), 1)
	#endif
#else
	#define WL11U_ENAB(wlc)		((void)(wlc), 0)
#endif /* WL11U */

/* SW probe response Support */
#ifdef WLPROBRESP_SW
	#if defined(ROM_ENAB_RUNTIME_CHECK) || !defined(DONGLEBUILD)
		#define WLPROBRESP_SW_ENAB(wlc)	((wlc)->pub->_probresp_sw)
	#elif defined(WLPROBRESP_SW_DISABLED)
		#define WLPROBRESP_SW_ENAB(wlc)	(0)
	#else
		#define WLPROBRESP_SW_ENAB(wlc)	((wlc)->pub->_probresp_sw)
	#endif
#else
	#define WLPROBRESP_SW_ENAB(wlc)		(0)
#endif /* WLPROBRESP_SW */

/* Link Power Control Support */
#ifdef WL_LPC
#define LPC_ENAB(wlc)	((wlc)->pub->_lpc_algo)
#else
#define LPC_ENAB(wlc)	(FALSE)
#endif /* WL_LPC */

/* Reliable Multicast Support */
#ifdef WL_RELMCAST
#if defined(ROM_ENAB_RUNTIME_CHECK)
#define RMC_SUPPORT(pub)	((pub)->_rmc_support)
#define RMC_ENAB(pub)		((pub)->_rmc)
#elif defined(WL_RELMCAST_DISABLED)
#define RMC_SUPPORT(pub)	(0)
#define RMC_ENAB(pub)		(0)
#else
#define RMC_SUPPORT(pub)	(1)
#define RMC_ENAB(pub)		((pub)->_rmc)
#endif /* ROM_ENAB_RUNTIME_CHECK */
#else
#define RMC_SUPPORT(pub)		(0)
#define RMC_ENAB(pub)			(0)
#endif /* WL_RELMCAST */

#ifdef WL_STATS
	#if defined(ROM_ENAB_RUNTIME_CHECK)
		#define WL_STATS_ENAB(pub)	((pub)->_wlstats)
	#elif defined(WL_STATS_DISABLED)
		#define WL_STATS_ENAB(pub)	(0)
	#else
		#define WL_STATS_ENAB(pub)	(1)
	#endif /* defined(ROM_ENAB_RUNTIME_CHECK) */
#else
	#define WL_STATS_ENAB(pub)		(0)
#endif /* WL_STATS */

/* Sensor hub interface APIs */
#ifdef WL_SHIF
	#if defined(ROM_ENAB_RUNTIME_CHECK)
		#define SHIF_ENAB(pub)   ((pub)->cmn->_shub)
	#elif defined(WL_SHIF_DISABLED)
		#define SHIF_ENAB(pub)	(0)
	#else
		#define SHIF_ENAB(pub)	(1)
	#endif
#else
	#define SHIF_ENAB(pub)		(0)
#endif /* WL_SHIF */

/* PM single core beacon rx
 * Allow MIMO chips to save power by enabling
 * only RX core while receiving beacons
 * To use this feature, need to add support in ucode
 * first before enabling it in driver
 */

#ifdef WLPM_BCNRX
#if defined(ROM_ENAB_RUNTIME_CHECK)
	#define PM_BCNRX_ENAB(pub)	((pub)->cmn->_pm_bcnrx)
#elif defined(WLPM_BCNRX_DISABLED)
	#define PM_BCNRX_ENAB(pub)	(0)
#else
	#define PM_BCNRX_ENAB(pub)	((pub)->cmn->_pm_bcnrx)
#endif // endif
#else
	#define PM_BCNRX_ENAB(pub)	(0)
#endif /* WLPM_BCNRX */

/* power efficient scanning (optimization) */
#ifdef WLSCAN_PS
#if defined(ROM_ENAB_RUNTIME_CHECK)
	#define WLSCAN_PS_ENAB(pub)   ((pub)->_scan_ps)
#elif defined(WLSCAN_PS_DISABLED)
	#define WLSCAN_PS_ENAB(pub)   (0)
#else
	#define WLSCAN_PS_ENAB(pub)   (1)
#endif /* defined(ROM_ENAB_RUNTIME_CHECK) */
#else
	#define WLSCAN_PS_ENAB(pub)   (0)
#endif /* WLSCAN_PS */

/* hotspot OSEN */
#ifdef WLOSEN
#if defined(ROM_ENAB_RUNTIME_CHECK)
	#define WLOSEN_ENAB(pub)   ((pub)->_osen)
#elif defined(WLOSEN_DISABLED)
	#define WLOSEN_ENAB(pub)   (0)
#else
	#define WLOSEN_ENAB(pub)   (1)
#endif /* defined(ROM_ENAB_RUNTIME_CHECK) */
#else
	#define WLOSEN_ENAB(pub)   (0)

#endif	/* WLOSEN */

/* SW Diversity Support */
#ifdef WLC_SW_DIVERSITY
#if defined(ROM_ENAB_RUNTIME_CHECK)
	#define WLSWDIV_ENAB(wlc)   ((wlc)->pub->_swdiv)
#elif defined(WLC_SW_DIVERSITY_DISABLED)
	#define WLSWDIV_ENAB(wlc)   (0)
#else
	#define WLSWDIV_ENAB(wlc)   ((wlc)->pub->_swdiv)
#endif // endif
#else
	#define WLSWDIV_ENAB(wlc)   (0)
#endif	/* WLC_SW_DIVERSITY */

/* TXPowercap Support */
#ifdef WLC_TXPWRCAP
#if defined(ROM_ENAB_RUNTIME_CHECK)
	#define WLTXPWRCAP_ENAB(wlc)   ((wlc)->pub->_txpwrcap)
#elif defined(WLC_TXPWRCAP_DISABLED)
	#define WLTXPWRCAP_ENAB(wlc)   (0)
#else
	#define WLTXPWRCAP_ENAB(wlc)   (1)
#endif // endif
#else
	#define WLTXPWRCAP_ENAB(wlc)   (0)
#endif	/* WLC_TXPWRCAP */

#define WLC_USE_COREFLAGS	0xffffffff	/**< invalid core flags, use the saved coreflags */

#ifdef WLCNT
#define WLC_UPDATE_STATS(wlc)	1	/**< Stats support */
#define WLCNTINCR(a)		((a)++)	/**< Increment by 1 */
#define WLCNTDECR(a)		((a)--)	/**< Decrement by 1 */
#define WLCNTADD(a,delta)	((a) += (delta)) /* Increment by specified value */
#define WLCNTSET(a,value)	((a) = (value)) /* Set to specific value */
#define WLCNTVAL(a)		(a)	/**< Return value */
#define WLCNTINCR_MIN(a)        ((a)++) /* Increment by 1 */
#define WLCNTCONDINCR(c, a)	do { if (c) (a)++; } while (0)	/**< Conditionally incr by 1 */
#define WLCNTCONDADD(c, a, delta)  /* Conditionally Increment by specified value */\
	                       do { if (c) (a) += (delta); } while (0)

/* variable in macstat struct based on corerev */
#define MCSTVAR(pub, varname)					\
	(D11REV_GE(pub->corerev, 40) ?				\
	((wl_cnt_ge40mcst_v1_t *)pub->_mcst_cnt)->varname :	\
	((wl_cnt_lt40mcst_v1_t *)pub->_mcst_cnt)->varname)

/* Set macstat variable to specific value */
#define WLCNTMCSTSET(pub, varname, value)				\
	do {								\
		if (D11REV_GE(pub->corerev, 40))			\
			((wl_cnt_ge40mcst_v1_t *)pub->_mcst_cnt)->	\
			varname = value;				\
		else							\
			((wl_cnt_lt40mcst_v1_t *)pub->_mcst_cnt)->	\
			varname = value;				\
	} while (0)
#else /* WLCNT */
#define WLC_UPDATE_STATS(wlc)	0	/* No stats support */
#define WLCNTINCR(a)			/* No stats support */
#define WLCNTDECR(a)			/* No stats support */
#define WLCNTADD(a,delta)		/* No stats support */
#define WLCNTSET(a,value)		/* No stats support */
#define WLCNTVAL(a)		0	/* No stats support */
#define WLCNTINCR_MIN(a)
#define WLCNTCONDINCR(c, a)		/* No stats support */
#define WLCNTCONDADD(c, a, delta)	/* No stats support */
#define MCSTVAR(a, b)		0	/* No stats support */
#define WLCNTMCSTSET(a, b, c)		/* No stats support */
#endif /* WLCNT */

#if !defined(RXCHAIN_PWRSAVE) && !defined(RADIO_PWRSAVE)
#define WLPWRSAVERXFADD(wlc, v)
#define WLPWRSAVERXFINCR(wlc)
#define WLPWRSAVETXFADD(wlc, v)
#define WLPWRSAVETXFINCR(wlc)
#define WLPWRSAVERXFVAL(wlc)	0
#define WLPWRSAVETXFVAL(wlc)	0
#endif /* !defined(RXCHAIN_PWRSAVE) && !defined(RADIO_PWRSAVE) */

/* dpc info */
struct wlc_dpc_info {
	uint processed;
};

/* BT WLAN Tunnel Engine */
#ifdef WL_BWTE
	#if defined(ROM_ENAB_RUNTIME_CHECK)
		#define BWTE_ENAB(pub)   ((pub)->_bwte)
	#elif defined(WL_BWTE_DISABLED)
		#define BWTE_ENAB(pub)   (0)
	#else
		#define BWTE_ENAB(pub)   (1)
	#endif /* defined(ROM_ENAB_RUNTIME_CHECK) */
#else
	#define BWTE_ENAB(pub)      (0)
#endif /* WL_BWTE */

/* Tunnel BT Over Wlan */
#ifdef WL_TBOW
	#if defined(ROM_ENAB_RUNTIME_CHECK)
		#define TBOW_ENAB(pub)   ((pub)->_tbow)
	#elif defined(WL_TBOW_DISABLED)
		#define TBOW_ENAB(pub)   (0)
	#else
		#define TBOW_ENAB(pub)   (1)
	#endif /* defined(ROM_ENAB_RUNTIME_CHECK) */
#else
	#define TBOW_ENAB(pub)      (0)
#endif /* WL_TBOW */

#if D11CONF_GE(128)
#ifndef WL_RATELINKMEM
#error "D11 revs 128 (and greater) require WL_RATELINKMEM (and HNDBME) support"
#endif /* !WL_RATELINKMEM */
#endif /* D11CONF_GE(128) */

/* RateLinkMemory support */
#ifdef WL_RATELINKMEM
	#ifdef ROM_ENAB_RUNTIME_CHECK
		#define RATELINKMEM_ENAB(pub)	((pub)->_ratelinkmem)
	#elif defined(WL_RATELINKMEM_DISABLED)
		#define RATELINKMEM_ENAB(pub)	(0)
	#else
		#define RATELINKMEM_ENAB(pub)	(D11REV_GE((pub)->corerev, 128))
	#endif
#else
	#define RATELINKMEM_ENAB(pub)	(0)
#endif /* WL_RATELINKMEM */

/* CFP support */
#ifdef WLCFP
	#if defined(ROM_ENAB_RUNTIME_CHECK) || !defined(DONGLEBUILD)
		#define CFP_ENAB(pub)	((pub)->_cfp)
	#elif defined(WLCFP_DISABLED)
		#define CFP_ENAB(pub)	(0)
	#else
		#define CFP_ENAB(pub)	(1)
	#endif
#else /* ! WLCFP */
	#define CFP_ENAB(pub)	(0)
#endif /* ! WLCFP */

#ifdef WL_REUSE_KEY_SEQ
#ifndef BCM_DHDHDR
#error "WL_REUSE_KEY_SEQ require BCM_DHDHDR support"
#endif /* !BCM_DHDHDR */
#endif /* WL_REUSE_KEY_SEQ */

/* common functions for every port */
extern void *wlc_attach(void *wl, uint16 vendor, uint16 device, uint unit, uint iomode,
	osl_t *osh, volatile void *regsva, uint bustype, void *btparam, void *objr, uint *perr);
extern uint wlc_detach(struct wlc_info *wlc);
extern int  wlc_up(struct wlc_info *wlc);
extern uint wlc_down(struct wlc_info *wlc);

extern bool wlc_chipmatch(uint16 vendor, uint16 device);
extern void wlc_init(struct wlc_info *wlc);
extern void wlc_reset(struct wlc_info *wlc);
extern int wlc_getrand(struct wlc_info *wlc, uint8 *buf, int len);

extern void wlc_intrson(struct wlc_info *wlc);
extern uint32 wlc_intrsoff(struct wlc_info *wlc);
extern void wlc_intrs_deassert(wlc_info_t *wlc);
extern void wlc_intrsrestore(struct wlc_info *wlc, uint32 macintmask);
extern bool wlc_intrsupd(struct wlc_info *wlc);
extern bool wlc_isr(struct wlc_info *wlc, bool *wantdpc);
extern bool wlc_dpc(struct wlc_info *wlc, bool bounded, struct wlc_dpc_info *dpc);
#if defined(WLC_OFFLOADS_TXSTS)
extern bool wlc_isr_txs_offload_ch1(struct wlc_info *wlc, bool *wantdpc);
#if defined(BCMQT)
extern bool wlc_int_is_m2m_irq(wlc_info_t *wlc);
#endif /* BCMQT */
#endif /* WLC_OFFLOADS_TXSTS */
extern bool wlc_sendpkt(struct wlc_info *wlc, void *sdu, struct wlc_if *wlcif);
extern bool wlc_send80211_specified(wlc_info_t *wlc, void *sdu, uint32 rspec, struct wlc_if *wlcif);
/* helper functions */
extern void wlc_statsupd(struct wlc_info *wlc);
#ifdef WLCNT
extern int wlc_get_all_cnts(wlc_info_t *wlc, void *buf, int buflen);
#endif /* WLCNT */

extern wlc_pub_t *wlc_pub(void *wlc);
extern void wlc_get_override_vendor_dev_id(void *wlc, uint16 *vendorid, uint16 *devid);

#ifndef LINUX_POSTMOGRIFY_REMOVAL
/* common functions for every port */
extern int wlc_bmac_up_prep(struct wlc_hw_info *wlc_hw);
extern int wlc_bmac_up_finish(struct wlc_hw_info *wlc_hw);
extern int wlc_bmac_set_ctrl_SROM(struct wlc_hw_info *wlc_hw);
extern int wlc_bmac_down_prep(struct wlc_hw_info *wlc_hw);
extern int wlc_bmac_down_finish(struct wlc_hw_info *wlc_hw);

extern uint32 wlc_reg_read(struct wlc_info *wlc, void *r, uint size);
extern void wlc_reg_write(struct wlc_info *wlc, void *r, uint32 v, uint size);
extern void wlc_corereset(struct wlc_info *wlc, uint32 flags);
extern void wlc_mhf(struct wlc_info *wlc, uint8 idx, uint16 mask, uint16 val, int bands);
extern uint16 wlc_mhf_get(struct wlc_info *wlc, uint8 idx, int bands);
extern uint wlc_ctrupd(wlc_info_t *wlc, uint macstat_offset);
extern uint32 wlc_delta_txfunfl(struct wlc_info *wlc, int fifo);
extern void wlc_rate_lookup_init(struct wlc_info *wlc, wlc_rateset_t *rateset);
extern void wlc_default_rateset(struct wlc_info *wlc, wlc_rateset_t *rs);

/* wlc_phy.c helper functions */
extern bool wlc_scan_inprog(struct wlc_info *wlc);
extern bool wlc_rminprog(struct wlc_info *wlc);
#ifdef STA
extern bool wlc_associnprog(struct wlc_info *wlc);
#endif /* STA */
extern bool wlc_scan_inprog(struct wlc_info *wlc);
extern void *wlc_cur_phy(struct wlc_info *wlc);
extern void wlc_set_ps_ctrl(struct wlc_bsscfg *cfg);
extern void wlc_set_wake_ctrl(struct wlc_info *wlc);
extern void wlc_mctrl(struct wlc_info *wlc, uint32 mask, uint32 val);
extern void wlc_mctrlx(struct wlc_info *wlc, uint32 mask, uint32 val);
#endif /* LINUX_POSTMOGRIFY_REMOVAL */

#if defined(WLC_PATCH_IOCTL)
/* 'wlc_module_register_ex()' supports IOVAR patch handlers. Callers continue to use the
 * legacy API 'wlc_module_register()' to register modules.
 */
#define wlc_module_register(pub, iovars, name, hdl, iovar_fn, watchdog_fn, up, down) \
	wlc_module_register_ex(pub, iovars, name, hdl, iovar_fn, watchdog_fn, up, down, \
	                       IOV_PATCH_TBL, IOV_PATCH_FN)
int wlc_module_register_ex(wlc_pub_t *pub, const bcm_iovar_t *iovars,
	const char *name, void *hdl, wlc_iov_disp_fn_t iovar_fn,
	watchdog_fn_t watchdog_fn, up_fn_t up_fn, down_fn_t down_fn,
	const bcm_iovar_t *patch_iovt, wlc_iov_disp_fn_t patch_disp_fn);
#else  /* !WLC_PATCH_IOCTL */
int wlc_module_register(wlc_pub_t *pub, const bcm_iovar_t *iovars,
	const char *name, void *hdl, wlc_iov_disp_fn_t iovar_fn,
	watchdog_fn_t watchdog_fn, up_fn_t up_fn, down_fn_t down_fn);
#endif /* WLC_PATCH_IOCTL */
extern int wlc_module_unregister(wlc_pub_t *pub, const char *name, void *hdl);

#ifndef LINUX_POSTMOGRIFY_REMOVAL
extern uint wlc_txpktcnt(struct wlc_info *wlc);
extern uint wlc_txpktcnt_bss(struct wlc_info *wlc, wlc_bsscfg_t *bsscfg);

extern void wlc_suspend_mac_and_wait(struct wlc_info *wlc);
extern void wlc_enable_mac(struct wlc_info *wlc);
extern uint16 wlc_rate_shm_offset(struct wlc_info *wlc, uint8 rate);

extern int wlc_get_last_txpwr(wlc_info_t *wlc, wlc_txchain_pwr_t *last_pwr);

/* helper functions */
extern bool wlc_radio_monitor_stop(struct wlc_info *wlc);
extern bool wlc_radio_monitor_start(struct wlc_info *wlc);

#ifdef STA
#ifdef BCMSUP_PSK
extern bool wlc_sup_mic_error(struct wlc_bsscfg *cfg, bool group);
#endif /* BCMSUP_PSK */
extern void wlc_pmkid_build_cand_list(struct wlc_bsscfg *cfg, bool check_SSID);
#endif /* STA */

#define BAND_2G_NAME		"2.4G"
#define BAND_5G_NAME		"5G"

#endif /* LINUX_POSTMOGRIFY_REMOVAL */

/* per interface stats counters */
extern void wlc_wlcif_stats_get(wlc_info_t *wlc, wlc_if_t *wlcif, wl_if_stats_t *wlif_stats);
extern wlc_if_t *wlc_wlcif_get_by_index(wlc_info_t *wlc, uint idx);

/* value for # replay counters currently supported */
#ifdef WOWL
#define WLC_REPLAY_CNTRS_VALUE	WPA_CAP_4_REPLAY_CNTRS
#else
#define WLC_REPLAY_CNTRS_VALUE	WPA_CAP_16_REPLAY_CNTRS
#endif /* WOWL */

/* priority to replay counter (Rx IV) entry index mapping. */
/*
 * It is one-to-one mapping when there are 16 replay counters.
 * Otherwise it is many-to-one mapping when there are only 4
 * counters which are one-to-one mapped to 4 ACs.
 */
#if WLC_REPLAY_CNTRS_VALUE == WPA_CAP_16_REPLAY_CNTRS
#define PRIO2IVIDX(prio)	(prio)
#elif WLC_REPLAY_CNTRS_VALUE == WPA_CAP_4_REPLAY_CNTRS
#define PRIO2IVIDX(prio)	WME_PRIO2AC(prio)
#else
#error "Neither WPA_CAP_4_REPLAY_CNTRS nor WPA_CAP_16_REPLAY_CNTRS is used"
#endif /* WLC_REPLAY_CNTRS_VALUE == WPA_CAP_16_REPLAY_CNTRS */

#define GPIO_2_PA_CTRL_5G_0		0x4 /**< bit mask of 2nd pin */

#if defined(CONFIG_WL) || defined(CONFIG_WL_MODULE) || defined(BCA_HNDROUTER)
#define WL_RTR()	TRUE
#else
#define WL_RTR()	FALSE
#endif /* defined(CONFIG_WL) || defined(CONFIG_WL_MODULE) */

/**
 * XXX: Note: A prerequisite is that for ROM builds supporting the SRHWVSDB
 * feature, ROM_ENAB_RUNTIME_CHECK is defined.
 */
#if defined(SRHWVSDB)
	#if defined(ROM_ENAB_RUNTIME_CHECK) || !defined(DONGLEBUILD)
		/* rom builds & high/nic driver builds */
		#define SRHWVSDB_ENAB(pub) ((pub)->_wlsrvsdb)
	#elif defined(SRHWVSDB_DISABLED)
		/* rom offload build, feature disabled */
		#define SRHWVSDB_ENAB(pub) (0)
	#else
		/* rom offload build, feature enabled */
		#define SRHWVSDB_ENAB(pub) (1)
	#endif /* defined(ROM_ENAB_RUNTIME_CHECK) || !defined(DONGLEBUILD) */
#else
	#define SRHWVSDB_ENAB(pub) (0)
#endif /* SRHWVSDB */

#if defined(WLPROPRIETARY_11N_RATES)
	#if defined(ROM_ENAB_RUNTIME_CHECK) || !defined(DONGLEBUILD)
		/* dongle builds OR rom builds */
		#define WLPROPRIETARY_11N_RATES_ENAB(pub) ((pub)->_ht_prop_rates_capable)
	#elif defined(WLPROPRIETARY_11N_RATES_DISABLED)
		/* rom offload build, feature disabled */
		#define WLPROPRIETARY_11N_RATES_ENAB(pub) (0)
	#else
		/* rom offload build, feature enabled */
		#define WLPROPRIETARY_11N_RATES_ENAB(pub) (1)
	#endif /* defined(ROM_ENAB_RUNTIME_CHECK) || !defined(DONGLEBUILD) */
#else
	#define WLPROPRIETARY_11N_RATES_ENAB(pub) (0)
#endif /* WLPROPRIETARY_11N_RATES */

#define WAPI_HW_WPI_ENAB(_pub) FALSE

/* Convert restricted channel on receiving beacon or probe response */
#define CLEAR_RESTRICTED_CHANNEL_ENAB(pub) ((pub)->_clear_restricted_chan)

/* Add BSS info to WLC event data */
#if defined(WL_EVDATA_BSSINFO)
	#if defined(ROM_ENAB_RUNTIME_CHECK) || !defined(DONGLEBUILD)
		#define EVDATA_BSSINFO_ENAB(pub) ((pub)->_evdata_bssinfo)
	#elif defined(WL_EVDATA_BSSINFO_DISABLED)
		#define EVDATA_BSSINFO_ENAB(pub) (0)
	#else
		#define EVDATA_BSSINFO_ENAB(pub) (1)
	#endif /* defined(ROM_ENAB_RUNTIME_CHECK) || !defined(DONGLEBUILD) */
#else
	#define EVDATA_BSSINFO_ENAB(pub) 0
#endif /* WL_EVDATA_BSSINFO */

/* Power Stats */
#ifdef WL_PWRSTATS
	#if defined(ROM_ENAB_RUNTIME_CHECK)
		#define PWRSTATS_ENAB(pub)		((pub)->_pwrstats)
	#elif defined(WL_PWRSTATS_DISABLED)
		#define PWRSTATS_ENAB(pub)		(0)
	#else
		#define PWRSTATS_ENAB(pub)		(1)
	#endif /* defined(ROM_ENAB_RUNTIME_CHECK) */
#else
	#define PWRSTATS_ENAB(pub)			(0)
#endif /* WL_PWRSTATS */

#if defined(SCB_BS_DATA)
/* Macros to update the named counter, provided the structure pointer is non-NULL */
#define SCB_BS_DATA_CONDFIND(counters, wlc, scb) \
	do { (counters) = wlc_bs_data_counters(wlc, scb); } while (0)
#define SCB_BS_DATA_CONDINCR(counters, counter_name) \
	do { if (counters) { (counters)->counter_name += 1; } } while (0)
#define SCB_BS_DATA_CONDADD(counters, counter_name, delta) \
	do { if (counters) { (counters)->counter_name += (delta); } } while (0)
#else /* not SCB_BS_DATA */
#define SCB_BS_DATA_CONDFIND(c,w,s)	/* no SCB_BS_DATA support */
#define SCB_BS_DATA_CONDINCR(c, a)	/* no SCB_BS_DATA support */
#define SCB_BS_DATA_CONDADD(c, a, d)	/* no SCB_BS_DATA support */
#endif /* defined(SCB_BS_DATA) */

#ifdef PKTQ_LOG
/* Macros to update the named counter, provided the structure pointer if non-NULL */
#define SCB_RX_REPORT_COND_FIND(counters, wlc, scb, tid) \
	do { (counters) = wlc_rx_report_counters_tid(wlc, scb, tid); } while (0)
#define SCB_RX_REPORT_DATA_COND_INCR(counters, counter_name) \
	do { if (counters) { (counters)->counter_name += 1; } } while (0)
#define SCB_RX_REPORT_COND_ADD(counters, counter_name, delta) \
	do { if (counters) { (counters)->counter_name += (delta); } } while (0)
#else /* PKTQ_LOG */
#define SCB_RX_REPORT_COND_FIND(c, w, s, t)	/* no SCB_RX_REPORT support */
#define SCB_RX_REPORT_DATA_COND_INCR(c, a)	/* no SCB_RX_REPORT support */
#define SCB_RX_REPORT_COND_ADD(c, a, d)	/* no SCB_RX_REPORT support */
#endif /* PKTQ_LOG */

#ifdef WLDURATION
#define WLDURATION_ENTER(ww, idx) do { wlc_duration_enter((ww), (idx)); } while (0)
#define WLDURATION_EXIT(ww, idx)  do { wlc_duration_exit((ww), (idx)); } while (0)
#else
#define WLDURATION_ENTER(ww, idx)
#define WLDURATION_EXIT(ww, idx)
#endif /* WLDURATION */

/* OBSS Protection Support */
#ifdef WL_PROT_OBSS
	#if defined(ROM_ENAB_RUNTIME_CHECK) || !defined(DONGLEBUILD)
		#define WLC_PROT_OBSS_ENAB(pub) ((pub)->_prot_obss)
	#elif defined(WL_PROT_OBSS_DISABLED)
		#define WLC_PROT_OBSS_ENAB(pub) (0)
	#else
		#define WLC_PROT_OBSS_ENAB(pub) ((pub)->_prot_obss)
	#endif /* defined(ROM_ENAB_RUNTIME_CHECK) || !defined(DONGLEBUILD) */
#else
	#define WLC_PROT_OBSS_ENAB(pub) (0)
#endif /* WL_PROT_OBSS */

/* DYNBW  Support */
#ifdef WL_OBSS_DYNBW
	#if defined(ROM_ENAB_RUNTIME_CHECK) || !defined(DONGLEBUILD)
		#define WLC_OBSS_DYNBW_ENAB(pub) ((pub)->_obss_dynbw)
	#elif defined(WL_OBSS_DYNBW_DISABLED)
		#define WLC_OBSS_DYNBW_ENAB(pub) (0)
	#else
		#define WLC_OBSS_DYNBW_ENAB(pub) ((pub)->_obss_dynbw)
	#endif /* defined(ROM_ENAB_RUNTIME_CHECK) || !defined(DONGLEBUILD) */
#else
	#define WLC_OBSS_DYNBW_ENAB(pub) (0)
#endif /* WL_OBSS_DYNBW */

/* mode switch Support */
#ifdef WL_MODESW
	#if defined(ROM_ENAB_RUNTIME_CHECK) || !defined(DONGLEBUILD)
		#define WLC_MODESW_ENAB(pub) ((pub)->_modesw)
	#elif defined(WL_MODESW_DISABLED)
		#define WLC_MODESW_ENAB(pub) (0)
	#else
		#define WLC_MODESW_ENAB(pub) ((pub)->_modesw)
	#endif /* defined(ROM_ENAB_RUNTIME_CHECK) || !defined(DONGLEBUILD) */
#else
	#define WLC_MODESW_ENAB(pub) (0)
#endif /* WL_MODESW */

/* mode switch Support */
#ifdef RSDB_PM_MODESW
	#if defined(ROM_ENAB_RUNTIME_CHECK) || !defined(DONGLEBUILD)
		#define WLC_RSDB_PM_MODESW_ENAB(pub) ((pub)->_rsdb_pm_modesw)
	#elif defined(RSDB_PM_MODESW_DISABLED)
		#define WLC_RSDB_PM_MODESW_ENAB(pub) (0)
	#else
		#define WLC_RSDB_PM_MODESW_ENAB(pub) (1)
	#endif
#else
	#define WLC_RSDB_PM_MODESW_ENAB(pub) (0)
#endif /* WL_MODESW */

#if defined(WLBSSLOAD_REPORT)
	#if defined(ROM_ENAB_RUNTIME_CHECK) || !defined(DONGLEBUILD)
		#define BSSLOAD_REPORT_ENAB(pub) ((pub)->_bssload_report)
	#elif defined(WLBSSLOAD_REPORT_DISABLED)
		#define BSSLOAD_REPORT_ENAB(pub) (0)
	#else
		#define BSSLOAD_REPORT_ENAB(pub) (1)
	#endif /* defined(ROM_ENAB_RUNTIME_CHECK) || !defined(DONGLEBUILD) */
#else
	#define BSSLOAD_REPORT_ENAB(pub) 0
#endif /* defined(WLBSSLOAD_REPORT) */

/* Beacon Trim Support */
#ifdef WL_BCNTRIM
	#if defined(ROM_ENAB_RUNTIME_CHECK) || !defined(DONGLEBUILD)
		#define WLC_BCNTRIM_ENAB(pub) ((pub)->_bcntrim)
	#elif defined(WL_BCNTRIM_DISABLED)
		#define WLC_BCNTRIM_ENAB(pub) (0)
	#else
		#define WLC_BCNTRIM_ENAB(pub) ((pub)->_bcntrim)
	#endif /* defined(ROM_ENAB_RUNTIME_CHECK) || !defined(DONGLEBUILD) */
#else
	#define WLC_BCNTRIM_ENAB(pub) (0)
#endif /* WL_BCNTRIM */

/* TVPM Support */
#ifdef WL_TVPM
	#if defined(ROM_ENAB_RUNTIME_CHECK) || !defined(DONGLEBUILD)
		#define TVPM_ENAB(pub) ((pub)->_tvpm)
	#elif defined(WL_TVPM_DISABLED)
		#define TVPM_ENAB(pub) (0)
	#else
		#define TVPM_ENAB(pub) ((pub)->_tvpm)
	#endif
#else
	#define TVPM_ENAB(pub) (0)
#endif /* WL_TVPM */

/* OPS */
#ifdef WL_OPS
#if defined(ROM_ENAB_RUNTIME_CHECK) || !defined(DONGLEBUILD)
	#define WL_OPS_ENAB(pub) ((pub)->_ops)
#elif defined(WL_OPS_DISABLED)
	#define WL_OPS_ENAB(pub) (0)
#else
	#define WL_OPS_ENAB(pub) ((pub)->_ops)
#endif // endif
#else
	#define WL_OPS_ENAB(pub) (0)
#endif /* OPS */

/* VASIP support */
#ifdef WLVASIP
	#if defined(ROM_ENAB_RUNTIME_CHECK) || !defined(DONGLEBUILD)
		#define VASIP_ENAB(pub) ((pub)->_vasip)
		#define VASIP_PRESENT(wlc_hw) wlc_vasip_present(wlc_hw)
	#elif defined(WLVASIP_DISABLED)
		#define VASIP_ENAB(pub) (0)
		#define VASIP_PRESENT(wlc_hw) (0)
	#else
		#define VASIP_ENAB(pub) ((pub)->_vasip)
		#define VASIP_PRESENT(wlc_hw) wlc_vasip_present(wlc_hw)
	#endif
#else
	#define VASIP_ENAB(pub) (0)
	#define VASIP_PRESENT(wlc_hw) (0)
#endif /* WLVASIP */

/* SMC support */
#ifdef WLSMC
	#if !defined(DONGLEBUILD)
		#define SMC_ENAB(pub) ((pub)->_smc)
	#elif defined(WLSMC_DISABLED)
		#define SMC_ENAB(pub) (0)
	#else
		#define SMC_ENAB(pub) ((pub)->_smc)
	#endif
#else
	#define SMC_ENAB(pub) (0)
#endif /* WLSMC */

extern void wlc_devpwrstchg_change(wlc_info_t *wlc, bool hostmem_access_enabled);
extern uint32 wlc_halt_device(wlc_info_t *wlc);

/* Latency Tolerance Reporting */
#if defined(WL_LTR)
	#if defined(ROM_ENAB_RUNTIME_CHECK) || !defined(DONGLEBUILD)
		/* rom builds & high/nic driver builds */
		#define LTR_SUPPORT(pub)	((pub)->_ltr_support)
		#define LTR_ENAB(pub)		((pub)->_ltr)
	#elif defined(WL_LTR_DISABLED)
		/* rom offload build, feature disabled */
		#define LTR_SUPPORT(pub)	(0)
		#define LTR_ENAB(pub)		(0)
	#else
		/* rom offload build, feature enabled */
		#define LTR_SUPPORT(pub)	(1)
		#define LTR_ENAB(pub)		(1)
	#endif /* defined(ROM_ENAB_RUNTIME_CHECK) || !defined(DONGLEBUILD) */
#else
	#define LTR_SUPPORT(pub)	(0)
	#define LTR_ENAB(pub)		(0)
#endif /* WL_LTR */

extern void wlc_generate_pme_to_host(wlc_info_t *wlc, bool pme_on);

/* Latency Tolerance Reporting */
#if defined(SMF_STATS)
	#if defined(ROM_ENAB_RUNTIME_CHECK)
		#define SMFS_ENAB(pub)	(pub)->_smfs
	#elif defined(SMF_STATS_DISABLED)
		#define SMFS_ENAB(pub)	0
	#else
		#define SMFS_ENAB(pub)	1
	#endif /* defined(ROM_ENAB_RUNTIME_CHECK) */
#else
	#define SMFS_ENAB(pub)	0
#endif /* SMF_STATS */

/* Link Layer Statistics - per rate counters and radio on time */
#if defined(WL_LINKSTAT)
	#if defined(ROM_ENAB_RUNTIME_CHECK)
		#define LINKSTAT_ENAB(pub)	(pub)->_link_stats
	#elif defined(WL_LINKSTAT_DISABLED)
		#define LINKSTAT_ENAB(pub)	0
	#else
		#define LINKSTAT_ENAB(pub)	1
	#endif /* defined(ROM_ENAB_RUNTIME_CHECK) */
#else
	#define LINKSTAT_ENAB(pub)	0
#endif /* WL_LINKSTAT */
/* Keep-alive offloading */
#if defined(KEEP_ALIVE)
	#if defined(ROM_ENAB_RUNTIME_CHECK) || !defined(DONGLEBUILD)
		#define KEEP_ALIVE_ENAB(pub)	(pub)->cmn->_keep_alive
	#elif defined(KEEP_ALIVE_DISABLED)
		#define KEEP_ALIVE_ENAB(pub)	0
	#else
		#define KEEP_ALIVE_ENAB(pub)	1
	#endif
#else
	#define KEEP_ALIVE_ENAB(pub)	0
#endif /* KEEP_ALIVE */

/* u-code dump support */
#if defined(ROM_ENAB_RUNTIME_CHECK)
	#define UCODEDUMP_ENAB(pub)	((pub)->_ucodedump)
#elif defined(UCODEDUMP_DISABLED)
	#define UCODEDUMP_ENAB(pub)	0
#else
	#define UCODEDUMP_ENAB(pub)	1
#endif /* defined(ROM_ENAB_RUNTIME_CHECK) */
#define MAC_CLKGATING_ENAB(pub)	((pub)->_mac_clkgating)

/* Napping support */
#ifdef WL_NAP
	#if defined(ROM_ENAB_RUNTIME_CHECK)
		#define NAP_ENAB(pub)		((pub)->_nap)
	#elif defined(WL_NAP_DISABLED)
		#define NAP_ENAB(pub)		(0)
	#else
		#define NAP_ENAB(pub)		((pub)->_nap)
	#endif
#else
	#define NAP_ENAB(pub)			(0)
#endif /* WL_NAP */

/* ASDB Support */
#ifdef WL_ASDB
	#if defined(ROM_ENAB_RUNTIME_CHECK) || !defined(DONGLEBUILD)
		#define ASDB_ENAB(pub) ((pub)->cmn->_asdb)
	#elif defined(ASDB_DISABLED)
		#define ASDB_ENAB(pub) (0)
	#else
		#define ASDB_ENAB(pub) (1)
	#endif
#else
	#define ASDB_ENAB(pub) (0)
#endif /* WL_ASDB */

#ifdef RADAR
	#if defined(ROM_ENAB_RUNTIME_CHECK) || !defined(DONGLEBUILD)
		#define RADAR_ENAB(pub) ((pub)->cmn->_radar)
	#elif defined(RADAR_DISABLED)
		#define RADAR_ENAB(pub) (0)
	#else
		#define RADAR_ENAB(pub) (1)
	#endif
#else
	#define RADAR_ENAB(pub) (0)
#endif /* RADAR */

#ifdef WLDFS
	#if defined(ROM_ENAB_RUNTIME_CHECK) || !defined(DONGLEBUILD)
		#define WLDFS_ENAB(pub) ((pub)->cmn->_wldfs)
	#elif defined(WLDFS_DISABLED)
		#define WLDFS_ENAB(pub) (0)
	#else
		#define WLDFS_ENAB(pub) (1)
	#endif
#else
	#define WLDFS_ENAB(pub) (0)
#endif /* WLDFS */

#ifdef RSDB_DFS_SCAN /* scan for radar on one of the two macs */
	#if defined(ROM_ENAB_RUNTIME_CHECK) || !defined(DONGLEBUILD)
		#define WLRSDBDFS_ENAB(pub) ((pub)->cmn->_rsdb_dfs)
	#elif defined(WLRSDB_DFS_SCAN_DISABLED)
		#define WLRSDBDFS_ENAB(pub) (0)
	#else
		#define WLRSDBDFS_ENAB(pub) (1)
	#endif
#else
	#define WLRSDBDFS_ENAB(pub) (0)
#endif /* RSDB_DFS_SCAN */

#ifdef WLRSDB_POLICY_MGR
	#if defined(ROM_ENAB_RUNTIME_CHECK) || !defined(DONGLEBUILD)
		#define WLRSDB_POLICY_MGR_ENAB(pub) ((pub)->cmn->_rsdb_policy)
	#elif defined(WLRSDB_POLICY_MGR_DISABLED)
		#define WLRSDB_POLICY_MGR_ENAB(pub) 0
	#else
		#define WLRSDB_POLICY_MGR_ENAB(pub) ((pub)->cmn->_rsdb_policy)
	#endif
#else
	#define WLRSDB_POLICY_MGR_ENAB(pub) 0
#endif /* WLRSDB_POLICY_MGR */

#ifdef OCL
	#if defined(ROM_ENAB_RUNTIME_CHECK)
		#define OCL_ENAB(pub)		((pub)->_ocl)
	#elif defined(OCL_DISABLED)
		#define OCL_ENAB(pub)		(0)
	#else
		#define OCL_ENAB(pub)		((pub)->_ocl)
	#endif
#else
	#define OCL_ENAB(pub)			(0)
#endif /* OCL */

#ifdef WL_STF_ARBITRATOR
	#if defined(ROM_ENAB_RUNTIME_CHECK) || !defined(DONGLEBUILD)
		#define WLC_STF_ARB_ENAB(pub)  ((pub)->_stf_arb)
	#elif defined(WL_STF_ARBITRATOR_DISABLED)
		#define WLC_STF_ARB_ENAB(pub)  (0)
	#else
		#define WLC_STF_ARB_ENAB(pub)  ((pub)->_stf_arb)
	#endif
#else
	#define STF_ARB_ENAB(pub)  (0)
#endif /* WL_STF_ARBITRATOR */

/* MIMOPS support */
#ifdef WL_MIMOPS
	#if defined(ROM_ENAB_RUNTIME_CHECK) || !defined(DONGLEBUILD)
		#define WLC_MIMOPS_ENAB(pub)  ((pub)->_stf_mimops)
	#elif defined(WL_MIMOPS_DISABLED)
		#define WLC_MIMOPS_ENAB(pub)  (0)
	#else
		#define WLC_MIMOPS_ENAB(pub)  ((pub)->_stf_mimops)
	#endif
#else
	#define WLC_MIMOPS_ENAB(pub)  (0)
#endif /* WL_MIMOPS */

#ifdef WLC_TSYNC
	#if defined(ROM_ENAB_RUNTIME_CHECK) || !defined(DONGLEBUILD)
		#define TSYNC_ENAB(pub) ((pub)->_tsync)
		#define TSYNC_ACTIVE(pub) ((pub)->_tsync_active)
	#elif defined(WLC_TSYNC_DISABLED)
		#define TSYNC_ENAB(pub) (0)
		#define TSYNC_ACTIVE(pub) (0)
	#else
		#define TSYNC_ENAB(pub) ((pub)->_tsync)
		#define TSYNC_ACTIVE(pub) ((pub)->_tsync_active)
	#endif
#else
	#define TSYNC_ENAB(pub) (0)
	#define TSYNC_ACTIVE(pub) (0)
#endif /* WLC_TSYNC */

/* send scan summary in chunks to the host to prevent buffer overflow */
#ifdef WLSCAN_SUMEVT
	#if defined(ROM_ENAB_RUNTIME_CHECK)
		#define SCAN_SUMEVT_ENAB(pub) ((pub)->cmn->_scan_sumevt)
	#elif defined(WLSCAN_SUMEVT_DISABLED)
		#define SCAN_SUMEVT_ENAB(pub) (0)
	#else
		#define SCAN_SUMEVT_ENAB(pub) (1)
	#endif
#else
	#define SCAN_SUMEVT_ENAB(pub) (0)
#endif /* WLSCAN_SUMEVT */

#if defined(WL_PSMX)
#if defined(ROM_ENAB_RUNTIME_CHECK) || !defined(DONGLEBUILD)
	#define PSMX_ENAB(pub)	((pub)->_psmx)
#elif defined(WL_PSMX_DISABLED)
	#define PSMX_ENAB(pub)	(0)
#else
	#define PSMX_ENAB(pub)	((pub)->_psmx)
#endif // endif

#else /* WL_PSMX */

#define PSMX_ENAB(pub)		(0)

#endif	/* WL_PSMX */

#if defined(WL_PSMR1)
#if defined(ROM_ENAB_RUNTIME_CHECK) || !defined(DONGLEBUILD)
	#define PSMR1_ENAB(hw)	(hw->_psmr1)
#elif defined(WL_PSMR1_DISABLED)
	#define PSMR1_ENAB(hw)	(0)
#else
	#define PSMR1_ENAB(hw)	(hw->_psmr1)
#endif // endif

#else /* WL_PSMR1 */

#define PSMR1_ENAB(pub)		(0)

#endif	/* WL_PSMX */
#if defined(WL_AUXPMQ)
#if defined(ROM_ENAB_RUNTIME_CHECK) || !defined(DONGLEBUILD)
	#define AUXPMQ_ENAB(pub)	((pub)->_auxpmq)
#elif defined(WL_AUXPMQ_DISABLED)
	#define AUXPMQ_ENAB(pub)	(0)
#else
	#define AUXPMQ_ENAB(pub)	((pub)->_auxpmq)
#endif // endif

#else /* WL_AUXPMQ */

#define AUXPMQ_ENAB(pub)		(0)

#endif	/* WL_AUXPMQ */

/**
 * STS_RX_ENAB() returns TRUE when the phyrxsts-over-d11rxfifo-3 mechanism is supported by hardware
 * and enabled by the driver.
 */
#if defined(STS_FIFO_RXEN)
#if defined(ROM_ENAB_RUNTIME_CHECK) || !defined(DONGLEBUILD)
	#define STS_RX_ENAB(pub)	((pub)->_sts_rxen)
#elif defined(STS_FIFO_RXEN_DISABLED)
	#define STS_RX_ENAB(pub)	(0)
#else
	#define STS_RX_ENAB(pub)	((pub)->_sts_rxen)
#endif // endif
#else
#define STS_RX_ENAB(pub)		(0)
#endif	/* STS_FIFO_RXEN */

/**
 * STS_RX_OFFLOAD_ENAB() returns TRUE when the phyrxsts-over-bme mechanism is supported by hardware
 * and enabled by the driver.
 */
#if defined(WLC_OFFLOADS_RXSTS)
#if defined(ROM_ENAB_RUNTIME_CHECK) || !defined(DONGLEBUILD)
	#define STS_RX_OFFLOAD_ENAB(pub)	((pub)->_sts_offld_rxen)
#elif defined(STS_FIFO_RXEN_DISABLED)
	#define STS_RX_OFFLOAD_ENAB(pub)	(0)
#else
	#define STS_RX_OFFLOAD_ENAB(pub)	((pub)->_sts_offld_rxen)
#endif // endif
#else
#define STS_RX_OFFLOAD_ENAB(pub)		(0)
#endif	/* WLC_OFFLOADS_RXSTS */

/* IO mode */
#define IOMODE_TYPE_PIO		1
#define IOMODE_TYPE_CTDMA	2

#ifdef WL_MBO
	#if defined(ROM_ENAB_RUNTIME_CHECK)
		#define MBO_ENAB(pub)	((pub)->cmn->_mbo)
	#elif defined(WL_MBO_DISABLED)
		#define MBO_ENAB(pub)	(0)
	#else
		#define MBO_ENAB(pub)	((pub)->cmn->_mbo)
	#endif /* defined(ROM_ENAB_RUNTIME_CHECK) */
#else
	#define MBO_ENAB(pub)		(0)
#endif /* WL_MBO */

#ifdef WL_OCE
	#if defined(ROM_ENAB_RUNTIME_CHECK)
		#define OCE_ENAB(pub)	((pub)->cmn->_oce)
	#elif defined(WL_OCE_DISABLED)
		#define OCE_ENAB(pub)	(0)
	#else
		#define OCE_ENAB(pub)	((pub)->cmn->_oce)
	#endif /* defined(ROM_ENAB_RUNTIME_CHECK) */
#else
	#define OCE_ENAB(pub)		(0)
#endif /* WL_OCE */

#ifdef RSDB_APSCAN
	#if defined(ROM_ENAB_RUNTIME_CHECK) || !defined(DONGLEBUILD)
		#define	RSDB_APSCAN_ENAB(pub)	\
			((pub)->cmn->_rsdb_scan & SCAN_DOWNGRADED_CH_PRUNE_ROAM)
	#elif defined(RSDB_APSCAN_DISABLED)
		#define	RSDB_APSCAN_ENAB(pub) 0
	#else
		#define	RSDB_APSCAN_ENAB(pub) 1
	#endif
#else
	#define	RSDB_APSCAN_ENAB(pub) 0
#endif /* RSDB_APSCAN */

#ifdef WL_UCM
	#if defined(ROM_ENAB_RUNTIME_CHECK)
		#define UCM_ENAB(pub) (pub->_ucm)
	#elif defined(WL_UCM_DISABLED)
		#define UCM_ENAB(pub) (FALSE)
	#else
		#define UCM_ENAB(pub) (pub->_ucm)
	#endif
#else
	#define UCM_ENAB(pub) (FALSE)
#endif /* WL_UCM */

#define UCPRS_DISABLED(pub)	\
	(D11REV_IS(pub->corerev, 128) ||\
	(D11REV_IS(pub->corerev, 129) &&\
	(D11MINORREV_IS(pub->corerev_minor, 0) ||\
	D11MINORREV_IS(pub->corerev_minor, 1))))

#ifdef WL_FILS
#if defined(ROM_ENAB_RUNTIME_CHECK) || !defined(DONGLEBUILD)
#define FILS_ENAB(pub)  ((pub)->cmn->_fils)
#elif defined(WL_FILS_DISABLED)
#define FILS_ENAB(pub)  (0)
#else
#define FILS_ENAB(pub)  (1)
#endif /* defined(ROM_ENAB_RUNTIME_CHECK) || !defined(DONGLEBUILD) */
#else
#define FILS_ENAB(pub)      (0)
#endif /* WL_FILS */

#ifdef WL_MBO_OCE
	#if defined(ROM_ENAB_RUNTIME_CHECK)
		#define MBO_OCE_ENAB(pub)	((pub)->cmn->_mbo_oce)
	#else
		#define MBO_OCE_ENAB(pub)	((pub)->cmn->_mbo_oce)
	#endif /* defined(ROM_ENAB_RUNTIME_CHECK) */
#else
	#define MBO_OCE_ENAB(pub)		(0)
#endif /* WL_MBO_OCE */

#ifdef WL_ESP
	#if defined(ROM_ENAB_RUNTIME_CHECK)
		#define ESP_ENAB(pub)	((pub)->cmn->_esp)
	#else
		#define ESP_ENAB(pub)	((pub)->cmn->_esp)
	#endif /* defined(ROM_ENAB_RUNTIME_CHECK) */
#else
	#define ESP_ENAB(pub)		(0)
#endif /* WL_ESP */

#ifdef WL_LEAKY_AP_STATS
	#if defined(ROM_ENAB_RUNTIME_CHECK)
		#define WL_LEAKYAPSTATS_ENAB(pub)       ((pub)->_leakyapstats)
	#elif defined(WL_LEAKY_AP_STATS_DISABLED)
		#define WL_LEAKAPSTATS_ENAB(pub)       (0)
	#else
		#define WL_LEAKYAPSTATS_ENAB(pub)       ((pub)->_leakyapstats)
	#endif
#else
	#define WL_LEAKYAPSTATS_ENAB(pub)       (0)
#endif /* WL_LEAKY_AP_STATS */

#ifdef WLSLOTTED_BSS
	#if defined(WLSLOTTED_BSS_DISABLED)
		#define SLOTTED_BSS_SUPPORT(pub) (0)
	#else
		#define SLOTTED_BSS_SUPPORT(pub) (1)
	#endif
#else
	#define SLOTTED_BSS_SUPPORT(pub) (0)
#endif /* WLSLOTTED_BSS */

#ifdef WLC_ADPS
	#if defined(ROM_ENAB_RUNTIME_CHECK) || !defined(DONGLEBUILD)
		#define ADPS_ENAB(pub)	((pub)->cmn->_adps_enable)
	#elif defined(WLC_ADPS_DISABLED)
		#define ADPS_ENAB(pub)	0
	#else
		#define ADPS_ENAB(pub)	1
	#endif
#else
#define ADPS_ENAB(pub)	0
#endif  /* WLC_ADPS */

/* BAD AP Manger */
#ifdef WLC_BAM
	#if defined(ROM_ENAB_RUNTIME_CHECK) || !defined(DONGLEBUILD)
		#define BAM_ENAB(pub)    ((pub)->cmn->_bam_enable)
	#elif defined(WLC_BAM_DISABLED)
		#define BAM_ENAB(pub)    (0)
	#else
		#define BAM_ENAB(pub)    (1)
	#endif
#else
	#define BAM_ENAB(pub)    (0)
#endif /* BAD AP Manager */

#ifdef TDMTX
	#if defined(ROM_ENAB_RUNTIME_CHECK)
		#define TDMTX_ENAB(pub) (pub->_tdmtx)
	#elif defined(TDMTX_DISABLED)
		#define TDMTX_ENAB(pub)        (0)
	#else
		#define TDMTX_ENAB(pub)	(pub->_tdmtx)
	#endif
#else
	#define TDMTX_ENAB(pub)	(0)
#endif /* TDMTX */

/* AP radio power save with NoA support */
#ifdef RADIONOA_PWRSAVE
	#if defined(ROM_ENAB_RUNTIME_CHECK) || !defined(DONGLEBUILD)
		#define RADIONOA_PWRSAVE_ENAB(pub)	 ((pub)->cmn->_rpsnoa)
	#elif defined(RADIONOA_PWRSAVE_DISABLED)
		#define RADIONOA_PWRSAVE_ENAB(pub)	 (0)
	#else
		#define RADIONOA_PWRSAVE_ENAB(pub)	 (1)
	#endif
#else
	#define RADIONOA_PWRSAVE_ENAB(pub)	 (0)
#endif /* RADIONOA_PWRSAVE */

#ifdef BCM_CEVENT
	#if defined(ROM_ENAB_RUNTIME_CHECK) || !defined(DONGLEBUILD)
		/* NIC with BCM_CEVENT flag or dongle with runtime check */
		#define CEVENT_STATE(pub)	((pub)->_cevent)
	#elif defined(BCM_CEVENT_DISABLED)
		/* dongle with -DBCM_CEVENT flag but disabled with -DBCM_CEVENT_DISABLED */
		#define CEVENT_STATE(pub)	(0)
	#else
		/* dongle without runtime check and without BCM_CEVENT_DISABLED */
		#define CEVENT_STATE(pub)	((pub)->_cevent)
	#endif /* ROM_ENAB_RUNTIME_CHECK || !DONGLEBUILD */
#else /* BCM_CEVENT */
	/* eg. NIC or dongle without BCM_CEVENT flag */
	#define CEVENT_STATE(pub)	(0)
#endif /* BCM_CEVENT */

/* Adaptive round robin weights for uCode */
#ifdef WLWRR
	#if defined(ROM_ENAB_RUNTIME_CHECK) || (!defined(DONGLEBUILD)) || \
	(!defined(BCMROMBUILD))
		#define WRR_ADAPTIVE(pub)	 ((pub)->_wrr_adaptive)
	#elif defined(WRR_ADAPTIVE_DISABLED)
		#define WRR_ADAPTIVE(pub)	 (0)
	#else
		#define WRR_ADAPTIVE(pub)	 (1)
	#endif
#else
	#define WRR_ADAPTIVE(pub)	 (0)
#endif /* WLWRR */

#endif /* _wlc_pub_h_ */
