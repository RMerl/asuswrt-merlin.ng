/*
 * Common (OS-independent) definitions for
 * Broadcom 802.11abg Networking Device Driver
 *
 * Copyright (C) 2017, Broadcom. All Rights Reserved.
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
 * $Id: wlc_pub.h 671038 2016-11-18 14:45:14Z $
 */

#ifndef _wlc_pub_h_
#define _wlc_pub_h_

#include <wlc_types.h>
#include <wlc_utils.h>
#include "proto/802.11.h"
#include "proto/bcmevent.h"
#include <bcmutils.h>
#include <bcmdevs.h>
#ifdef WL11ULB_NIC
#include <siutils.h>
#endif

#ifndef PAD
#define _PADLINE(line)  pad ## line
#define _XSTR(line)     _PADLINE(line)
#define PAD             _XSTR(__LINE__)
#endif

/* max # wlc timers. Each TDLS connection may dynamically allocate upto 2 timers */
#define	MAX_TIMERS	(34 + WLC_MAXMFPS + WLC_MAXDLS_TIMERS)
#define	WLC_NUMRATES	16	/* max # of rates in a rateset */
#define	MAXMULTILIST	32	/* max # multicast addresses */
#define	D11_PHY_HDR_LEN	6	/* Phy header length - 6 bytes */

/* phy types */
#define	PHY_TYPE_A	0	/* Phy type A */
#define	PHY_TYPE_G	2	/* Phy type G */
#define	PHY_TYPE_N	4	/* Phy type N */
#define	PHY_TYPE_LP	5	/* Phy type Low Power A/B/G */
#define	PHY_TYPE_SSN	6	/* Phy type Single Stream N */
#define	PHY_TYPE_HT	7	/* Phy type 3-Stream N */
#define	PHY_TYPE_LCN	8	/* Phy type Single Stream N */
#define	PHY_TYPE_LCNXN	9	/* Phy type 2-stream N */

/* channel bandwidth */
#define WLC_10_MHZ	10	/* 10Mhz channel bandwidth */
#define WLC_20_MHZ	20	/* 20Mhz channel bandwidth */
#define WLC_40_MHZ	40	/* 40Mhz channel bandwidth */
#define WLC_80_MHZ	80	/* 80Mhz channel bandwidth */
#define WLC_160_MHZ	160	/* 160Mhz channel bandwidth */

/* This macro is used for ratespec related initialization. Since for ULB mode the ratespec
 * values are same as that of 20MHz BW, hence default value returned has been changed
 */
#ifdef WL11ULB
#define CHSPEC_WLC_BW(chanspec)(\
				(CHSPEC_IS160(chanspec) ||\
				CHSPEC_IS8080(chanspec)) ? WLC_160_MHZ :\
				CHSPEC_IS80(chanspec) ? WLC_80_MHZ :\
				CHSPEC_IS40(chanspec) ? WLC_40_MHZ :\
							WLC_20_MHZ)
#else /* WL11ULB */
#define CHSPEC_WLC_BW(chanspec)(\
				(CHSPEC_IS160(chanspec) ||\
				CHSPEC_IS8080(chanspec)) ? WLC_160_MHZ :\
				CHSPEC_IS80(chanspec) ? WLC_80_MHZ :\
				CHSPEC_IS40(chanspec) ? WLC_40_MHZ :\
				CHSPEC_IS20(chanspec) ? WLC_20_MHZ :\
							WLC_10_MHZ)
#endif /* WL11ULB */

#define	WLC_RSSI_MINVAL		-200	/* Low value, e.g. for forcing roam */
#define	WLC_RSSI_MINVAL_INT8	-128	/* Low value fit for 8-bits */
#define	WLC_RSSI_NO_SIGNAL	-91	/* NDIS RSSI link quality cutoffs */
#define	WLC_RSSI_VERY_LOW	-80	/* Very low quality cutoffs */
#define	WLC_RSSI_LOW		-70	/* Low quality cutoffs */
#define	WLC_RSSI_GOOD		-68	/* Good quality cutoffs */
#define	WLC_RSSI_VERY_GOOD	-58	/* Very good quality cutoffs */
#define	WLC_RSSI_EXCELLENT	-57	/* Excellent quality cutoffs */

#define	PREFSZ			160
#ifdef PKTC
#define WLPREFHDRS(h, sz)
#else
#define WLPREFHDRS(h, sz)	OSL_PREF_RANGE_ST((h), (sz))
#endif

#ifndef LINUX_POSTMOGRIFY_REMOVAL
#define WLC_PHYTYPE(_x) (_x) /* macro to perform WLC PHY -> D11 PHY TYPE, currently 1:1 */

#define MA_WINDOW_SZ		8	/* moving average window size */

#define WLC_SNR_INVALID		0	/* invalid SNR value */
#define WLC_SNR_EXCELLENT	25

#define WLC_NOISE_INVALID	0

/* a large TX Power as an init value to factor out of MIN() calculations,
 * keep low enough to fit in an int8, units are .25 dBm
 */
#define WLC_TXPWR_MAX		(127)	/* ~32 dBm = 1,500 mW */
#define BCM94331X19_MINTXPOWER	5	/* Min txpower target for X19 board in dBm */
#define ARPT_MODULES_MINTXPOWER	7	/* Min txpower target for all X & M modules in dBm */
#define BCM94360_MINTXPOWER 1   /* Min txpower target for 4360 boards in dBm */
#define WLC_NUM_TXCHAIN_MAX	4	/* max number of chains supported by common code */

/* parameter struct for wlc_get_last_txpwr() */
typedef struct wlc_txchain_pwr {
	int8 chain[WLC_NUM_TXCHAIN_MAX];	/* quarter dBm signed pwr for each chain */
} wlc_txchain_pwr_t;

/* legacy rx Antenna diversity for SISO rates */
#define	ANT_RX_DIV_FORCE_0		0	/* Use antenna 0 */
#define	ANT_RX_DIV_FORCE_1		1	/* Use antenna 1 */
#define	ANT_RX_DIV_START_1		2	/* Choose starting with 1 */
#define	ANT_RX_DIV_START_0		3	/* Choose starting with 0 */
#define	ANT_RX_DIV_ENABLE		3	/* APHY bbConfig Enable RX Diversity */
#define ANT_RX_DIV_DEF		ANT_RX_DIV_START_0	/* default antdiv setting */

/* legacy rx Antenna diversity for SISO rates */
#define ANT_TX_FORCE_0		0	/* Tx on antenna 0, "legacy term Main" */
#define ANT_TX_FORCE_1		1	/* Tx on antenna 1, "legacy term Aux" */
#define ANT_TX_LAST_RX		3	/* Tx on phy's last good Rx antenna */
#define ANT_TX_DEF			3	/* driver's default tx antenna setting */

#define TXCORE_POLICY_ALL	0x1	/* use all available core for transmit */

/* Tx Chain values */
#define TXCHAIN_DEF		0x1	/* def bitmap of txchain */
#define TXCHAIN_DEF_NPHY	0x3	/* default bitmap of tx chains for nphy */
#define TXCHAIN_DEF_HTPHY	0x7	/* default bitmap of tx chains for htphy */
#define TXCHAIN_DEF_ACPHY	0x7	/* default bitmap of tx chains for acphy */
#define TXCHAIN_DEF_AC2PHY	0xf	/* default bitmap of tx chains for ac2phy */
#define RXCHAIN_DEF		0x1	/* def bitmap of rxchain */
#define RXCHAIN_DEF_NPHY	0x3	/* default bitmap of rx chains for nphy */
#define RXCHAIN_DEF_HTPHY	0x7	/* default bitmap of rx chains for htphy */
#define RXCHAIN_DEF_ACPHY	0x7	/* default bitmap of rx chains for acphy */
#define RXCHAIN_DEF_AC2PHY	0xf	/* default bitmap of rx chains for ac2phy */
#define ANTSWITCH_NONE		0	/* no antenna switch */
#define ANTSWITCH_TYPE_1	1	/* antenna switch on 4321CB2, 2of3 */
#define ANTSWITCH_TYPE_2	2	/* antenna switch on 4321MPCI, 2of3 */
#define ANTSWITCH_TYPE_3	3	/* antenna switch on 4322, 2of3, SW only */
#define ANTSWITCH_TYPE_4	4	/* antenna switch on 43234, 1of2, Core 1, SW only */
#define ANTSWITCH_TYPE_5	5	/* antenna switch on 4322, 2of3, SWTX + HWRX */
#define ANTSWITCH_TYPE_6	6	/* antenna switch on 43234, 1of2, SWTX + HWRX */
#define ANTSWITCH_TYPE_7	7	/* antenna switch on 5356C0, 1of2, Core 0, SW only */

#define RXBUFSZ		PKTBUFSZ
#ifndef AIDMAPSZ
#define AIDMAPSZ	(ROUNDUP(MAXSCB, NBBY)/NBBY)	/* aid bitmap size in bytes */
#endif /* AIDMAPSZ */
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
	int ntxd;	/* size of tx descriptor table for DMAs with 512 descriptor table support */
	int nrxd;	/* size of rx descriptor table for DMAs with 512 descriptor table support */
	int rxbufsz;			/* size of rx buffers to post */
	int nrxbufpost;			/* # of rx buffers to post */
#if defined(TXQ_MUX)
	int txq_highwater;		/* usec TxQ highwater mark - stop filling TxQ */
	int txq_lowwater;		/* usec TxQ lowwater mark - start filling TxQ */
#endif /* TXQ_MUX */
	int maxscb;			/* # of SCBs supported */
	int ampdunummpdu2streams;	/* max number of mpdu in an ampdu for 2 streams */
	int ampdunummpdu3streams;	/* max number of mpdu in an ampdu for 3+ streams */
	int maxpktcb;			/* max # of packet callbacks */
	int maxucodebss;		/* max # of BSS handled in ucode bcn/prb */
	int maxucodebss4;		/* max # of BSS handled in sw bcn/prb */
	int maxbss;			/* max # of bss info elements in scan list */
	int datahiwat;			/* data msg txq hiwat mark */
	int ampdudatahiwat;		/* AMPDU msg txq hiwat mark */
	int rxbnd;			/* max # of rx bufs to process before deferring to dpc */
	int txsbnd;			/* max # tx status to process in wlc_txstatus() */
	int pktcbnd;			/* max # of packets to chain */
	int dngl_mem_restrict_rxdma;	/* memory limit for BMAC's rx dma */
	int rpctxbufpost;
	int pkt_maxsegs;
	int maxscbcubbies;		/* max # of scb cubbies */
	int maxbsscfgcubbies;		/* max # of bsscfg cubbies */
	int max_notif_servers;		/* max # of notification servers. */
	int max_notif_clients;		/* max # of notification clients. */
	int max_mempools;		/* max # of memory pools. */
	int maxtdls;			/* max # of tdls links */
	int amsdu_resize_buflen;	/* threshold for right-size amsdu subframes */
	int ampdu_pktq_size;		/* max # of pkt from same precedence in an ampdu */
	int ampdu_pktq_fav_size;	/* max # of favored pkt from same precedence in an ampdu */
	int maxpcbcds;			/* max # of packet class callback descriptors */
	int wlfcfifocreditac0;		/* FIFO credits offered to host for AC 0 */
	int wlfcfifocreditac1;		/* FIFO credits offered to host for AC 1 */
	int wlfcfifocreditac2;		/* FIFO credits offered to host for AC 2 */
	int wlfcfifocreditac3;		/* FIFO credits offered to host for AC 3 */
	int wlfcfifocreditbcmc;		/* FIFO credits offered to host for BCMC */
	int wlfcfifocreditother;	/* FIFO credits offered to host for Other */
	int wlfc_fifo_cr_pending_thresh_ac_bk;	/* max AC_BK credits pending for push to host */
	int wlfc_fifo_cr_pending_thresh_ac_be;	/* max AC_BE credits pending for push to host */
	int wlfc_fifo_cr_pending_thresh_ac_vi;	/* max AC_VI credits pending for push to host */
	int wlfc_fifo_cr_pending_thresh_ac_vo;	/* max AC_VO credits pending for push to host */
	int wlfc_fifo_cr_pending_thresh_bcmc;	/* max BCMC credits pending for push to host */
	int wlfc_trigger;		/* status/credit inidication trigger */
	int wlfc_fifo_bo_cr_ratio;	/* total credits to max pending credits ratio */
	int wlfc_comp_txstatus_thresh;	/* max pending compressed tx status count */

	int ntxd_large;	/* size of tx descriptor table for DMAs w/ 4096 descriptor table support */
	int nrxd_large;	/* size of rx descriptor table for DMAs w/ 4096 descriptor table support */
	int maxubss;			/* for dongle: limit for user (stored) scans list */
	int ampdunummpdu1stream;	/* max number of mpdu in an ampdu for 2 streams */
	int max_keys;			/* maximum number of (s/w) keys */
	int max_ie_build_cbs;		/* max # IE-build callbacks */
	int max_vs_ie_build_cbs;	/* max # VS IE-build callbacks */
	int max_ie_parse_cbs;		/* max # IE-parse callbacks */
	int max_vs_ie_parse_cbs;	/* max # IE-parse callbacks */
	int max_ie_regs;		/* max # IE registries */
	int num_rxivs;
	int maxbestn;
	int maxmscan;
	int ntxd_lfrag;			/* ntxd when lfrags are getting programmed */
	int nrxbufpost_fifo1;		/* no of descriptors to be posted into fifo1 */
	int nrxbufpost_fifo2;		/* no of descriptors to be posted into fifo2 */
	int nrxd_fifo1;			/* no of rxd for fifo-1 */
	int maxroamthresh;	/* max roam threshold tunable */
	int copycount;		/* copycount for mode 3 and mode 4 */
	int bufpost_classified_fifo;	/* no of descriptors to be posted into fifo1 */
	int nrxd_classified_fifo;		/* no of rxd for fifo-1 */
	uint scan_settle_time;		/* time (watchdog ticks) to block after a scan/roam */
	int min_scballoc_mem;		/* min memory to allow scb alloc */
	int txmr;		/* no. of outstanding reads */
	int txpft;		/* tx prefetch threshold */
	int txpfc;		/* tx prefetch control */
	int txblen;		/* tx burst len */
	int rxpft;		/* rx prefetch threshold */
	int rxpfc;		/* rx prefetch threshold */
	int rxblen;		/* rx burst len */
	int mrrs;		/* max read request size */
} wlc_tunables_t;

#if defined(STA) && defined(DBG_BCN_LOSS)
struct wlc_scb_dbg_bcn {
	uint	last_rx;
	int	last_rx_rssi;
	int	last_bcn_rssi;
	uint	last_tx;
};
#endif

#ifndef LINUX_POSTMOGRIFY_REMOVAL
/*
 * buffer length needed for wlc_format_ssid
 * 32 SSID chars, max of 4 chars for each SSID char "\xFF", plus NULL.
 */
#define SSID_FMT_BUF_LEN	((4 * DOT11_MAX_SSID_LEN) + 1)


#define RSN_FLAGS_SUPPORTED		0x1	/* Flag for rsn_parms */
#define RSN_FLAGS_PREAUTH		0x2	/* Flag for WPA2 rsn_parms */
#define RSN_FLAGS_FBT			0x4	/* Flag for Fast BSS Transition */
#define RSN_FLAGS_MFPC			0x8	/* Flag for MFP enabled */
#define RSN_FLAGS_MFPR			0x10	/* Flag for MFP required */
#define RSN_FLAGS_SHA256		0x20	/* Flag for MFP required */
#define RSN_FLAGS_PEER_KEY_ENAB		0x40	/* Flags for Peer Key Enabled */
#define RSN_FLAGS_PMKID_COUNT_PRESENT	0x80	/* PMKID count present */

/* All the HT-specific default advertised capabilities (including AMPDU)
 * should be grouped here at one place
 */
#define AMPDU_DEF_MPDU_DENSITY	AMPDU_DENSITY_4_US	/* default mpdu density */

/* defaults for the HT (MIMO) bss */
#define HT_CAP	((HT_CAP_MIMO_PS_OFF << HT_CAP_MIMO_PS_SHIFT) | HT_CAP_40MHZ | \
		  HT_CAP_GF | HT_CAP_MAX_AMSDU | HT_CAP_DSSS_CCK)
#endif /* LINUX_POSTMOGRIFY_REMOVAL */

/* WLC packet type is a void * */
typedef void *wlc_pkt_t;

/* Event data type */
typedef struct wlc_event {
	wl_event_msg_t	event;		/* encapsulated event */
	struct ether_addr *addr;	/* used to keep a trace of the potential present of
					 * an address in wlc_event_msg_t
					 */
	struct wlc_if	*wlcif;		/* pointer to wlcif */
	void		*data;		/* used to hang additional data on an event */
	struct wlc_event *next;		/* enables ordered list of pending events */
} wlc_event_t;

/**
 * Stores scan result information (so information on an AP or IBSS), extracted from a beacon or
 * probe response. Information is not related to this local entity. AP information that is specific
 * to this local entity (e.g. association state) should be stored in struct wlc_bsscfg instead.
 */
struct wlc_bss_info {
	struct ether_addr BSSID;	/* network BSSID */
	uint16		flags;		/* flags for internal attributes */
	uint8		SSID_len;	/* the length of SSID */
	uint8		SSID[32];	/* SSID string */
	int16		RSSI;		/* receive signal strength (in dBm) */
	int16		SNR;		/* receive signal SNR in dB */
	uint16		beacon_period;	/* units are Kusec */
	uint16		atim_window;	/* units are Kusec */
	chanspec_t	chanspec;	/* Channel num, bw, ctrl_sb and band */
	int8		infra;		/* 0=IBSS, 1=infrastructure, 2=unknown */
	wlc_rateset_t	rateset;	/* supported rates */
	uint8		dtim_period;	/* DTIM period */
	int8		phy_noise;	/* noise right after tx (in dBm) */
	uint16		capability;	/* Capability information */

	/* WLSCANCACHE */
	uint32		timestamp;	/* in ms since boot, OSL_SYSUPTIME() */

	struct dot11_bcn_prb *bcn_prb;	/* beacon/probe response frame (ioctl na) */
	uint16		bcn_prb_len;	/* beacon/probe response frame length (ioctl na) */
	uint8		wme_qosinfo;	/* QoS Info from WME IE; valid if WLC_BSS_WME flag set */
	struct rsn_parms wpa;
	struct rsn_parms wpa2;

	/* BCMCCX || WLP2P || WLMCHAN */
	uint32		rx_tsf_l;	/* usecs, rx time in local TSF */

	uint16		qbss_load_aac;	/* qbss load available admission capacity */
	/* qbss_load_chan_free <- (0xff - channel_utilization of qbss_load_ie_t) */
	uint8		qbss_load_chan_free;	/* indicates how free the channel is */
	uint8		mcipher;	/* multicast cipher */
	uint8		wpacfg;		/* wpa config index */
	uint16		mdid;		/* mobility domain id */
	uint16		flags2;		/* additional flags for internal attributes */

	/* WLTDLS */
	uint32		ext_cap_flags;
	uint8		ti_type;
	uint8		oper_mode;
	uint32		ti_val;
	uint8 		anonce[32];
	uint8 		snonce[32];
	uint8 		mic[16];

	uint32		vht_capabilities;
	uint16		vht_rxmcsmap;
	uint16		vht_txmcsmap;
	uint16		vht_txmcsmap_prop;

#ifdef BCMCCX
	uint8		ckip_support;	/* Aironet IE byte with CKIP negotiation bits */
	uint8		ccx_qos_params_cnt; /* CCX QOS params IE update count */
	uint8		ccx_version;	/* CCX version */
	uint8		ccx_phy_type;	/* CCX phy_type for radio measurements */
	bool		aironet_ie_rx;	/* present of Aironet IE in beacon/probe */
#endif /* BCMCCX */

#ifdef BCMWAPI_WAI
	struct rsn_parms wapi;
#endif /* BCMWAPI_WAI */

};

/* wlc_bss_info flag bit values */
#define WLC_BSS_54G             0x0001  /* BSS is a legacy 54g BSS */
#define WLC_BSS_RSSI_ON_CHANNEL 0x0002  /* RSSI measurement was from the same channel as BSS */
#define WLC_BSS_WME             0x0004  /* BSS is WME capable */
#define WLC_BSS_BRCM            0x0008  /* BSS is BRCM */
#define WLC_BSS_WPA             0x0010  /* BSS is WPA capable */
#define WLC_BSS_HT              0x0020  /* BSS is HT (MIMO) capable */
#define WLC_BSS_40MHZ           0x0040  /* BSS is 40MHZ capable */
#define WLC_BSS_WPA2            0x0080  /* BSS is WPA2 capable */
#define WLC_BSS_BEACON          0x0100  /* bss_info was derived from a beacon */
#define WLC_BSS_40INTOL         0x0200  /* BSS is forty intolerant */
#define WLC_BSS_SGI_20          0x0400  /* BSS supports 20MHz SGI */
#define WLC_BSS_SGI_40          0x0800  /* BSS supports 40MHz SGI */
#ifdef WLBTAMP
#define WLC_BSS_11E             0x1000  /* BSS is 802.11e capable (BT-AMP QoS) */
#endif /* WLBTAMP */
#define WLC_BSS_CACHE           0x2000  /* bss_info was collected from scan cache */
#ifdef BCMWAPI_WAI
#define WLC_BSS_WAPI            0x4000  /* BSS is WAPI capable */
#endif /* BCMWAPI_WAI */
#define WLC_BSS_FBT             0x8000  /* BSS is FBT capable */

/* additional wlc_bss_info flag bit values (flags2 field) */
#define WLC_BSS_OVERDS_FBT      0x0001  /* BSS is Over-the-DS FBT capable */
#define WLC_BSS_VHT             0x0002  /* BSS is VHT (802.11ac) capable */
#define WLC_BSS_80MHZ           0x0004  /* BSS is VHT 80MHz capable */
#define WLC_BSS_SGI_80		0x0008	/* BSS supports 80MHz SGI */
#define WLC_BSS_HS20		0x0010	/* BSS is hotspot 2.0 capable */
#define WLC_BSS_RSSI_INVALID	0x0020	/* BSS contains invalid RSSI */
#define WLC_BSS_RSSI_INACCURATE	0x0040	/* BSS contains inaccurate RSSI */
#define WLC_BSS_8080MHZ           0x0080  /* BSS is VHT 80+80 MHz capable */
#define WLC_BSS_160MHZ           0x0100  /* BSS is VHT 160 MHz capable */
#define WLC_BSS_SGI_160          0x0200  /* BSS supports 160MHz SGI */
#define WLC_BSS_MAX_STA_CNT	0x0400	/* BSS load IE has max sta count */
#define WLC_BSS_ULB_10_CAP	0x1000	/* BSS is 10MHz ULB Capable */
#define WLC_BSS_ULB_5_CAP	0x2000	/* BSS is 5MHz ULB Capable */
#define WLC_BSS_ULB_2P5_CAP	0x4000	/* BSS is 2.5MHz ULB Capable */
#define WLC_BSS_OPER_MODE	0x8000  /* BSS supports operating mode notification */

/* wlc_ioctl error codes */
#define WLC_ENOIOCTL	1 /* No such Ioctl */
#define WLC_EINVAL	2 /* Invalid value */
#define WLC_ETOOSMALL	3 /* Value too small */
#define WLC_ETOOBIG	4 /* Value too big */
#define WLC_ERANGE	5 /* Out of range */
#define WLC_EDOWN	6 /* Down */
#define WLC_EUP		7 /* Up */
#define WLC_ENOMEM	8 /* No Memory */
#define WLC_EBUSY	9 /* Busy */

/* IOVar flags for common error checks */
#define IOVF_BSSCFG_STA_ONLY	(1<<0)	/* flag for BSSCFG_STA() only iovars */
#define IOVF_BSSCFG_AP_ONLY	(1<<1)	/* flag for BSSCFG_AP() only iovars */
#define IOVF_BSS_SET_DOWN (1<<2)	/* set requires BSS to be down */

#define IOVF_MFG	(1<<3)  /* flag for mfgtest iovars */
#define IOVF_WHL	(1<<4)	/* value must be whole (0-max) */
#define IOVF_NTRL	(1<<5)	/* value must be natural (1-max) */

#define IOVF_SET_UP	(1<<6)	/* set requires driver be up */
#define IOVF_SET_DOWN	(1<<7)	/* set requires driver be down */
#define IOVF_SET_CLK	(1<<8)	/* set requires core clock */
#define IOVF_SET_BAND	(1<<9)	/* set requires fixed band */

#define IOVF_GET_UP	(1<<10)	/* get requires driver be up */
#define IOVF_GET_DOWN	(1<<11)	/* get requires driver be down */
#define IOVF_GET_CLK	(1<<12)	/* get requires core clock */
#define IOVF_GET_BAND	(1<<13)	/* get requires fixed band */
#define IOVF_OPEN_ALLOW	(1<<14)	/* set allowed iovar for opensrc */
#define IOVF_RSDB_SET 	(1<<15)  /* set iovar across all WLCs */

/* IOVAR flag used by ROM patch tables. Indicates that the IOVAR has been removed from the latest
 * code. Or may be unsupported based upon #ifdefs. (Note that all of the IOVF_xxx bits have been
 * used. Since some of the bits are mutually exclusive, it is impossible for all bits to be set in a
 * real IOVAR table. Therefore IOVF_REMOVED can be set to 0xffff, instead of increasing the width
 * of the flags bit-field).
 */
#define IOVF_REMOVED	(0xffff)


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

#ifdef MACOSX
#define WL_HEALTH_LOG(w, s)	((w)->pub->health |= s)
#else
#define WL_HEALTH_LOG(w, s)	do {} while (0)
#endif

/* watchdog down and dump callback function proto's */
typedef void (*watchdog_fn_t)(void *handle);
typedef int (*up_fn_t)(void *handle);
typedef int (*down_fn_t)(void *handle);
typedef int (*dump_fn_t)(void *handle, struct bcmstrbuf *b);

/* IOVar handler
 *
 * handle - a pointer value registered with the function
 * vi - iovar_info that was looked up
 * actionid - action ID, calculated by IOV_GVAL() and IOV_SVAL() based on varid.
 * name - the actual iovar name
 * params/plen - parameters and length for a get, input only.
 * arg/len - buffer and length for value to be set or retrieved, input or output.
 * vsize - value size, valid for integer type only.
 * wlcif - interface context (wlc_if pointer)
 *
 * All pointers may point into the same buffer.
 */
typedef int (*iovar_fn_t)(void *handle, const bcm_iovar_t *vi, uint32 actionid,
	const char *name, void *params, uint plen, void *arg, int alen,
	int vsize, struct wlc_if *wlcif);

/* IOCTL flags for common error checks */
#define WLC_IOCF_BSSCFG_STA_ONLY	(1<<0)	/* flag for BSSCFG_STA() only ioctls */
#define WLC_IOCF_BSSCFG_AP_ONLY	(1<<1)	/* flag for BSSCFG_AP() only ioctls */

#define WLC_IOCF_MFG	(1<<2)  /* flag for mfgtest ioctls */

#define WLC_IOCF_DRIVER_UP	(1<<3)	/* requires driver be up */
#define WLC_IOCF_DRIVER_DOWN	(1<<4)	/* requires driver be down */
#define WLC_IOCF_CORE_CLK	(1<<5)	/* requires core clock */
#define WLC_IOCF_FIXED_BAND	(1<<6)	/* requires fixed band */
#define WLC_IOCF_OPEN_ALLOW	(1<<7)	/* allowed ioctl for opensrc */

/* IOCTL handler
 *
 * handle - a pointer value registered with the function
 * cmd - ioctl command
 * arg/len - buffer and length for value to be set or retrieved, input or output.
 * wlcif - interface context (wlc_if pointer)
 *
 */
typedef int (*wlc_ioctl_fn_t)(void *handle, int cmd, void *arg, int len, struct wlc_if *wlcif);

struct wlc_ioctl_cmd {
	uint16 cmd;			/* IOCTL command */
	uint16 flags;			/* IOCTL command flags */
	int min_len;			/* IOCTL command minimum argument len (in bytes) */
};

/*
 * Public portion of "common" os-independent state structure.
 * The wlc handle points at this.
 */
typedef struct wlc_pub {
	void		*wlc;
	struct wlc_pub_cmn *cmn; /* Common PUB variables across WLCs. */
	struct ether_addr	cur_etheraddr;	/* our local ethernet addr, must be aligned */

	/* WLC_HIGH || WLOFFLD || BCM_OL_DEV */
	uint		unit;			/* device instance number */

#ifdef WLC_HIGH
	uint		corerev;		/* core revision */
	osl_t		*osh;			/* pointer to os handle */
	bool		up;			/* interface up and running */
	bool		hw_off;			/* HW is off */
#endif
	wlc_tunables_t *tunables;		/* tunables: ntxd, nrxd, maxscb, etc. */
	bool		hw_up;			/* one time hw up/down(from boot or hibernation) */
	bool		_piomode;		/* true if pio mode */ /* BMAC_NOTE: NEED In both */
	uint		now;			/* # elapsed seconds */
#ifdef WLC_HIGH
	uint		_nbands;		/* # bands supported */

	bool		promisc;		/* promiscuous destination address */
	bool		delayed_down;		/* down delayed */

	/* TOE */
	bool		_toe;			/* TOE mode enabled */
	/* ARPOE */
	bool		_arpoe;			/* Arp agent offload enabled */
	/* TRAFFIC_MGMT */
	bool		_traffic_mgmt;	        /* Traffic management enabled */
	/* TRAFFIC_MGMT_DWM */
	bool		_traffic_mgmt_dwm;	    /* Traffic management DWM enabled */

	bool		_ap;			/* AP mode enabled */
	bool		_apsta;			/* simultaneous AP/STA mode enabled */
	bool		_apcs;			/* APCS enabled */
	bool		_assoc_recreate;	/* association recreation on up transitions */
	int		_wme;			/* WME QoS mode */
	uint8		_mbss_mode;		/* MBSS mode */
	bool		_mbss_support;		/* MBSS is supported or not */
	/* WLTDLS */
	bool		_tdls;			/* tdls enabled or not */
	/* WLDLS */
	bool		_dls;			/* dls enabled or not */
	/* WLMCNX */
	bool		_mcnx;			/* multiple connection ucode ucode used or not */
	/* WLP2P */
	bool		_p2p;			/* p2p enabled or not */
	/* WLMCHAN */
	bool		_mchan;			/* multi channel enabled or not */
	bool		_mchan_active;		/* multi channel active or not */
	/* PSTA */
	int8		_psta;			/* Proxy STA mode: disabled, proxy, repeater */
	int8		_pktc;			/* Packet chaining enabled or not */
	/* WLBTAMP */
	bool		_bta;			/* bta enabled or not */

	bool		associated;		/* true:part of [I]BSS, false: not */
						/* (union of stas_associated, aps_associated) */
	bool            phytest_on;             /* whether a PHY test is running */
	bool		bf_preempt_4306;	/* True to enable 'darwin' mode */

	/* WOWL */
	bool		_wowl;			/* wowl enabled or not (in sw) */
	bool		_wowl_active;		/* Is Wake mode actually active
						 * (used during transition)
						 */
	/* WLAMPDU */
	bool		_ampdu_tx;		/* ampdu_tx enabled for HOST, UCODE or HW aggr */
	bool		_ampdu_rx;		/* ampdu_rx enabled for HOST, UCODE or HW aggr */
	/* WLAMSDU || WLAMSDU_TX */
	bool		_amsdu_tx;		/* true if currently amsdu agg is enabled */
	bool		_cac;			/* 802.11e CAC enabled */
#ifdef BCMCCX
	bool		_ccx;			/* enable: ccx */
#else
	bool		PAD;			/* increased ROM compatibility */
#endif
	/* WL11H */
	uint		_spect_management;	/* 11h spectrum management */
	/* WL11K */
	bool		_rrm;			/* 11k radio resource measurement */
	/* WLWNM */
	bool		_wnm;			/* 11v wireless netwrk management */
	/* WLBSSLOAD */
	bool		_bssload;		/* bss load IE */

	bool		_mfp;			/* mfp enabled or not */

	uint8		_n_enab;		/* bitmap of 11N + HT support */
	bool		_n_reqd;		/* N support required for clients */

	uint8		_vht_enab;		/* VHT (11AC) support */

	int8		_coex;			/* 20/40 MHz BSS Management AUTO, ENAB, DISABLE */
	bool		_priofc;		/* Priority-based flowcontrol */
	bool		phy_bw40_capable;	/* PHY 40MHz capable */
	bool		phy_bw80_capable;	/* PHY 80MHz capable */
	bool		phy_bw160_capable;	/* PHY 160MHz capable */
	bool		phy_bw8080_capable;	/* PHY 80+80MHz capable */

#ifdef HNDCTF
	ctf_brc_hot_t *brc_hot;			/* hot ctf bridge cache entry */
#else
	void		*PAD;			/* increased ROM compatibility */
#endif

	uint32		wlfeatureflag;		/* Flags to control sw features from registry */
	int			psq_pkts_total;		/* total num of ps pkts */

	uint16		txmaxpkts;		/* max number of large pkts allowed to be pending */

	/* s/w decryption counters */
	uint32		swdecrypt;		/* s/w decrypt attempts */

	int 		bcmerror;		/* last bcm error */

	mbool		radio_disabled;		/* bit vector for radio disabled reasons */
	mbool		last_radio_disabled;	/* radio disabled reasons for previous state */
	bool		radio_active;		/* radio on/off state */
	/* WME_PER_AC_TX_PARAMS */
	bool		_per_ac_maxrate;	/* Max TX rate per AC */

	/* DSLCPE && DSLCPE_BLOG */
	bool		fcache;			/* 1-- enable; 0--disable */

	/* STA */
	uint16		roam_time_thresh;	/* Max. # millisecs. of not hearing beacons
						 * before roaming.
						 */
	bool		align_wd_tbtt;		/* Align watchdog with tbtt indication
						 * handling. This flag is cleared by default
						 * and is set by per port code explicitly and
						 * you need to make sure the OSL_SYSUPTIME()
						 * is implemented properly in osl of that port
						 * when it enables this Power Save feature.
						 */

	uint16		boardrev;		/* version # of particular board */
	uint8		sromrev;		/* version # of the srom */
	uint32		boardflags;		/* Board specific flags from srom */
	uint32		boardflags2;		/* More board flags if sromrev >= 4 */
	uint32		boardflags4;		/* More board flags if sromrev >= 12 */
#endif /* WLC_HIGH */

	/* WLCNT */
	wl_cnt_wlc_t	*_cnt;			/* monolithic counters struct */
	wl_wme_cnt_t	*_wme_cnt;		/* Counters for WMM */

	bool		_extsta;		/* EXT_STA flag */
	bool		_pkt_filter;		/* pkt filter enable bool */
	bool		phy_11ncapable;		/* the PHY/HW is capable of 802.11N */
	bool		_fbt;			/* Fast Bss Transition active */
	/* WLPLT */
	bool		_plt;			/* PLT module included and enabled */

	pktpool_t	*pktpool;		/* use the pktpool for buffers */
	uint8		_ampdumac;	/* mac assist ampdu enabled or not */

	/* IBSS_PEER_GROUP_KEY */
	bool		_ibss_peer_group_key;
	/* IBSS_PEER_DISCOVERY_EVENT */
	bool		_ibss_peer_discovery_event;
	/* IBSS_PEER_MGMT */
	bool		_ibss_peer_mgmt;
	/* WLNOEIND */
	bool		_wleind;

	bool		PAD;			/* increased ROM compatibility */
	bool		PAD;			/* increased ROM compatibility */

	/* DMATXRC */
	bool		_dmatxrc;
	/* WLRXOV */
	bool		_rxov;
	/* WLPFN */
	bool		_wlpfn;
	/* BCMAUTH_PSK */
	bool		_bcmauth_psk;
	/* PROP_TXSTATUS */
	bool		_proptxstatus;
	/* BCMSUP_PSK */
	bool		_sup_enab;
	/*	WL_MULTIQUEUE */
	bool		_mqueue;
	/* BPRESET */
	bool		_bpreset;

	/* WL11H */
	bool		_11h;
	/* WL11D */
	bool		_11d;
	/* WLCNTRY */
	bool		_autocountry;
	/* WL11U */
	bool		_11u;
	/* WLPROBRESP_SW */
	bool		_probresp_sw;
#ifdef BCMWAPI_WPI
	bool		_wapi_hw_wpi;	/* HW WAPI_WPI enabled or not */
#else
	bool		PAD;	/* increased ROM compatibility */
#endif

	uint32		health;
	uint32		ht_features;            /* 802.11n proprietary rates */

	uint8       max_modules;
	/* WLNDOE */
	bool		_ndoe;		/* Neighbor Advertisement Offload */
	/* WOWLPF */
	bool		_wowlpf;			/* wowlpf enabled or not (in sw) */
	bool		_wowlpf_active;		/* Is Wake mode actually active
						 * (used during transition)
						 */
	/* NWOE */
	bool            _nwoe;

	uint8		d11tpl_phy_hdr_len; /* PLCP len for templates */
	uint		max_addrma_idx;
	uint16		m_coremask_blk;
	uint16		m_coremask_blk_wowl;
	/* WET_TUNNEL */
	bool		wet_tunnel;	/* true if wet_tunnel is enable */
	int		_ol;			/* Offloads */
	bool		_bmon;		/* BSSID monitor feature */
	/* WLPKTDLYSTAT_IND */
	txdelay_params_t *txdelay_params;
	/* WL11AC */
	uint16		vht_features;		/* BRCM proprietary VHT features  bitmap */
	/* WLAMPDU_HOSTREORDER */
	bool		_ampdu_hostreorder;
	/* WLTDLS */
	bool		_tdls_support;		/* tdls supported or not */

	si_t		*sih;			/* SB handle (cookie for siutils calls) */
	char		*vars;			/* "environment" name=value */
	uint		vars_size;		/* size of vars, free vars on detach */

	/* ARP offload supported. */
	bool		_arpoe_support;
	/* Flag to runtime enable/disable LPC */
	bool            _lpc_algo;

	bool		_l2_filter;		/* L2 filter enabled or not */

	bool		_rmc;		/* reliable multicast GO enabled */
	bool		_rmc_support;	/* reliable multicast GC enabled */
	uint		bcn_tmpl_len;		/* len for beacon template */

	bool		_p2po;			/* p2po enabled or not */
	bool		_anqpo;			/* anqpo enabled or not */

#ifdef WL_OFFLOADSTATS
	uint32		offld_cnt_received[4];
	uint32		offld_cnt_consumed[4];
#endif /* WL_OFFLOADSTATS */
#ifdef WL_INTERRUPTSTATS
	uint32		intr_cnt[32];
#endif /* WL_INTERRUPTSTATS */
	/* TCPKAOE */
	bool		_icmpoe;		/* ICMP agent offload enabled */
	bool		_tcp_keepalive;		/* tcp keepalive enable bool */

	/* WL_OKC */
	bool 		_okc;

	bool		_d0_filter;		/* pkt filter enable bool */
	bool		_olpc;			/* Open Loop Phy Calibration enable/disable */
	bool		_staprio;		/* sta prio enable bool */
	/**
	 * Since code contained in ROM must be able to dynamically use SRVSDB functionality, ROM
	 * code checks this member variable. Variable is written once at startup/attach, its value
	 * does not change after that.
	 */
	bool		_wlsrvsdb;		/* hardware assisted vsdb enable/disable */
	bool		_stamon;		/* STA monitor feature. */
	bool		_wl_rxearlyrc;
	bool		_tiny_pktjoin;
	pktpool_t	*pktpool_lfrag;		/* use the pktpool for buffers */
	bool		_scancache_support;
	bool		_fmc;			/* WLFMC_ENAB enabled or not */
	bool		_rcc;			/* WLRCC_ENAB enabled or not */
	/* WLAIBSS */
	bool		_aibss;			/* Advanced IBSS */

#ifdef WLC_HIGH_ONLY
	uint8		ampdu_ba_rx_wsize;	/* ampdu block-ack receive window size */
	uint32      wowl_gpio;
	bool        wowl_gpiopol;
#endif  /* WLC_HIGH_ONLY */

	/* WL_BEAMFORMING */
	bool		_txbf;

	bool		_wlfcts;		/* PROP TX status time stamp */
	pktpool_t	*pktpool_rxlfrag;		/* use the pktpool for rx  buffers */
	bool		_wlstats;		/* wlc statistics */
	bool		is_ss;			/* true if rpc bus running at superspeed */
	bool		_amsdu_tx_support;	/* true if amsdu tx agg is supported */
	bool		_awdl;			/* awdl enabled or not */
	bool		_awdl_support;		/* awdl supported or not */
	bool		_clear_restricted_chan;	/* clear restricted channels on rx frames */
	bool		scanresults_on_evtcplt; /* scan results tag with scan cplt event */
	bool		_evdata_bssinfo;	/* WLC event data bssinfo enable */
	bool		_net_detect;		/* NetDetect enable/disable */
	bool		_prot_obss;		/* OBSS protection */
	bool		_obss_dynbw;		/**< OBSS protection */
	bool		_pwrstats;		/* additional power stats supported */
	bool		_ipfo;			/* Multi-hop IP forwarding supported */
	bool		_frwd_reorder;
	uint            pending_now;            /* To correct delayed now due to scan */
	bool		_proxd;			/* proximity detection enabled or not */
	bool		_pm_bcnrx;		/* PM single core beacon rx */
	bool            _rm;                    /* enable radio measurement */
	bool		_abt;			/* Adaptive Beacon Timeout */
	bool		_scan_ps;		/* pwr optimization for scan */
	bool		_nfc;			/* Secure WiFi through NFC */
	bool		_bssload_report;	/* Get/report bssload from beacon */
	bool		_wloverthruster;	/* Overthruster enable/disable */
	bool		_ltecx;			/* LTE coex enabled */
	bool		_ltecxgci;		/* LTE Coex GCI Support Present */
	bool		_ccx_aka;		/* CCX AP_KEEP_ALIVE supported */
	bool		_osen;			/* hotspot OSEN */
	bool		_bwte;			/* true if bt wlan tunnel engine supported */
	bool		_wfds;			/* wfds enabled or not */
	bool		_nan;			/* NAN enabled */
	bool		_ltr_support;	/* LTR supported by platform */
	bool		_ltr;			/* LTR cap enabled/disabled */
	bool		_ht_prop_rates_capable;	/* 802.11n proprietary rates */
	bool		_tbow;			/* true if tunnel bt over wlan supported */
	bool		_modesw; 		/* modeswitch */
	bool		_metadata_to_host; /* To create TLVs to be sent to host */
	bool		_credit_info_update; /* To enable credit related processing */
	bool		_aibss_ps;		/* AIBSS PS */
	bool		_rxmetadata_to_host; /* TLVs to be sent to host for rxed pkts */
	bool		_txmetadata_to_host; /* TLVs to be sent to host for txed pkts */
	bool		_wlfc_ctrl_to_host;  /* TLVs to be sent to host for flow control info */
	bool		_wlfc_info_to_bus;   /* TLVs/byte streams to be sent to bus for wlfcinfo */
	bool		_dynbw;			/* Dynamic bandwidth enabled or not */
	bool		_bcntrim;		/* Beacon Trim */
	void		*_mcst_cnt;		/* macstat count from SHM.
						 * To be cast to either wl_cnt_ge40mcst_v1_t or
						 * wl_cnt_lt40mcst_v1_t based on corerev
						 */
#if defined(WL_PSMX)
	void		*_mcxst_cnt;		/* mcx count from SHM X. */
#else
	void		*PAD;			/* increased ROM compatibility */
#endif

#ifdef BGDFS
	bool		_bgdfs;			/* Background DFS with radar scan core */
#else
	bool		PAD;			/* increased ROM compatibility */
#endif /* BGDFS */
	uint16		m_amt_info_blk;		/* base address of amt info block */
	bool		_ulb;			/* ULB Mode supported */
	bool            _taf;                   /* TAF support */
#define MU_FEATURES_MUTX	(1 << 0)
#define MU_FEATURES_MURX	(1 << 1)
#define MU_FEATURES_AUTO	(1 << 15)
	uint32		mu_features;
	uint8		max_muclients;		/* Maximum MU clients */
	bool		_dyn160;		/* DYN160 capability - dynamic 160/80p80 <-> 80 */
	uint32		_dyn160_active;		/* DYN160 active - dynamic 160/80p80 <-> 80 */
	bool		_fbt_cap;		/* Fast Bss Transition capable */
	bool		_mbss_ign_mac_valid;	/* Ignore validation/auto-generation of MBSS MACs */
} wlc_pub_t;

/* Shared portion of wlc_pub structure across WLC's in case of RSDB. */
typedef struct wlc_pub_cmn {
	uint		driverrev;		/* to make wlc_get_revision_info rommable */
#ifdef WLRSDB
	bool		_isrsdb;		/* Set to true if this chip is RSDB capable. */
	bool _rsdb_active;			/* Set to true when both MAC's have
								 * active connections.
								 */
#endif /* WLRSDB */
} wlc_pub_cmn_t;

/* wl_monitor rx status per packet */
typedef struct	wl_rxsts {
	uint	pkterror;		/* error flags per pkt */
	uint	phytype;		/* 802.11 A/B/G /N  */
	chanspec_t chanspec;		/* channel spec */
	uint16	datarate;		/* rate in 500kbps */
	uint8	mcs;			/* MCS for HT frame */
	uint8	htflags;		/* HT modulation flags */
	uint	antenna;		/* antenna pkts received on */
	uint	pktlength;		/* pkt length minus bcm phy hdr */
	uint32	mactime;		/* time stamp from mac, count per 1us */
	uint	sq;			/* signal quality */
	int32	signal;			/* in dBm */
	int32	noise;			/* in dBm */
	uint	preamble;		/* Unknown, short, long */
	uint	encoding;		/* Unknown, CCK, PBCC, OFDM, HT, VHT */
	uint	nfrmtype;		/* special 802.11n frames(AMPDU, AMSDU) */
	struct wl_if *wlif;		/* wl interface */
#ifdef WL11AC
	uint8	nss;			/* Number of spatial streams for VHT frame */
	uint8   coding;
	uint16	aid;			/* Partial AID for VHT frame */
	uint8	gid;			/* Group ID for VHT frame */
	uint8   bw;			/* Bandwidth for VHT frame */
	uint16	vhtflags;		/* VHT modulation flags */
	uint8	bw_nonht;		/* non-HT bw advertised in rts/cts */
	uint32	ampdu_counter;		/* AMPDU counter for sniffer */
#endif /* WL11AC */
} wl_rxsts_t;

/* wl_tx_monitor tx status per packet */
typedef struct	wl_txsts {
	uint	pkterror;		/* error flags per pkt */
	uint	phytype;		/* 802.11 A/B/G /N */
	chanspec_t chanspec;		/* channel spec */
	uint16	datarate;		/* rate in 500kbps */
	uint8	mcs;			/* MCS for HT frame */
	uint8	htflags;		/* HT modulation flags */
	uint	antenna;		/* antenna pkt transmitted on */
	uint	pktlength;		/* pkt length minus bcm phy hdr */
	uint32	mactime;		/* ? time stamp from mac, count per 1us */
	uint	preamble;		/* Unknown, short, long */
	uint	encoding;		/* Unknown, CCK, PBCC, OFDM, HT */
	uint	nfrmtype;		/* special 802.11n frames(AMPDU, AMSDU) */
	uint	txflags;		/* As defined in radiotap field 15 */
	uint	retries;		/* Number of retries */
	struct wl_if *wlif;		/* wl interface */
} wl_txsts_t;

/* status per error RX pkt */
#define WL_RXS_CRC_ERROR		0x00000001 /* CRC Error in packet */
#define WL_RXS_RUNT_ERROR		0x00000002 /* Runt packet */
#define WL_RXS_ALIGN_ERROR		0x00000004 /* Misaligned packet */
#define WL_RXS_OVERSIZE_ERROR		0x00000008 /* packet bigger than RX_LENGTH (usually 1518) */
#define WL_RXS_WEP_ICV_ERROR		0x00000010 /* Integrity Check Value error */
#define WL_RXS_WEP_ENCRYPTED		0x00000020 /* Encrypted with WEP */
#define WL_RXS_PLCP_SHORT		0x00000040 /* Short PLCP error */
#define WL_RXS_DECRYPT_ERR		0x00000080 /* Decryption error */
#define WL_RXS_OTHER_ERR		0x80000000 /* Other errors */

/* phy type */
#define WL_RXS_PHY_A			0x00000000 /* A phy type */
#define WL_RXS_PHY_B			0x00000001 /* B phy type */
#define WL_RXS_PHY_G			0x00000002 /* G phy type */
#define WL_RXS_PHY_N			0x00000004 /* N phy type */

/* encoding */
#define WL_RXS_ENCODING_UNKNOWN		0x00000000
#define WL_RXS_ENCODING_DSSS_CCK	0x00000001 /* DSSS/CCK encoding (1, 2, 5.5, 11) */
#define WL_RXS_ENCODING_OFDM		0x00000002 /* OFDM encoding */
#define WL_RXS_ENCODING_HT		0x00000003 /* HT encoding */
#define WL_RXS_ENCODING_VHT		0x00000004 /* VHT encoding */

/* preamble */
#define WL_RXS_UNUSED_STUB		0x0		/* stub to match with wlc_ethereal.h */
#define WL_RXS_PREAMBLE_SHORT		0x00000001	/* Short preamble */
#define WL_RXS_PREAMBLE_LONG		0x00000002	/* Long preamble */
#define WL_RXS_PREAMBLE_HT_MM		0x00000003	/* HT mixed mode preamble */
#define WL_RXS_PREAMBLE_HT_GF		0x00000004	/* HT green field preamble */

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

#define WL_RXS_NFRM_AMPDU_FIRST		0x00000001 /* first MPDU in A-MPDU */
#define WL_RXS_NFRM_AMPDU_SUB		0x00000002 /* subsequent MPDU(s) in A-MPDU */
#define WL_RXS_NFRM_AMSDU_FIRST		0x00000004 /* first MSDU in A-MSDU */
#define WL_RXS_NFRM_AMSDU_SUB		0x00000008 /* subsequent MSDU(s) in A-MSDU */

#define WL_TXS_TXF_FAIL		0x01	/* TX failed due to excessive retries */
#define WL_TXS_TXF_CTS		0x02	/* TX used CTS-to-self protection */
#define WL_TXS_TXF_RTSCTS 	0x04	/* TX used RTS/CTS */


#ifndef LINUX_POSTMOGRIFY_REMOVAL
/* Structure for Pkttag area in a packet.
 * CAUTION: Please carefully consider your design before adding any new fields to the pkttag
 * The size is limited to 32 bytes which on 64-bit machine allows only 4 fields.
 * If adding a member, be sure to check if wlc_pkttag_info_move should transfer it.
 */
typedef struct {
	uint32		flags;		/* Describe various packet properties */
	uint16		seq;		/* preassigned seqnum for AMPDU */
	uint8		flags2;		/* Describe various packet properties */
	uint8		flags3;		/* Describe various packet properties */
	union {
		uint8	callbackidx;	/* Index into pkt_callback tables for callback function */
		uint8	rxchannel;	/* Control channel of the received packet */
	};

	int8		_bsscfgidx;	/* Index of bsscfg for this frame */
	union {
		struct {
			int8	snr;	/* SNR for the recvd. packet */
			int8	rssi;	/* RSSI for the recvd. packet */
		} misc;
		union {
			uint16 pkt_len;
			uint16 pkt_time;
		} atf;
#ifdef WLTAF
		struct {
			uint16 index:2;
			uint16 units:14;
		} taf;
#endif
	} pktinfo;
	union {
		uint32		exptime;	/* Time of expiry for the packet */
#if defined(BCMPKTIDMAP)
		struct wlc_d11rxhdr * wrxh; /* ampdu responder saves wrxh temporarily */
#endif /* BCMPKTIDMAP */
		struct {
			uint8	ampdu_flow_id;
			uint8	cur_idx;
			uint8	exp_idx;
			uint8	flags;
		} ampdu_info_to_host;
	} u;
	struct scb	*_scb;		/* Pointer to SCB for associated ea */
	uint32		rspec;		/* Phy rate for received packet */
	union {
		uint32		packetid;
#ifdef WLFCTS
		uint32		rx_tstamp;	/* 32-bit TSF based timestamp for RX pkt */
		uint32		tx_entry_tstamp;	/* TSF based timestamp for TX entry */
#endif /* WLFCTS */
#ifdef WLBTAMP
		struct {
			uint16	llh;		/* Logical Link Handle */
			uint8	unused[2];
		} bta;
#endif
#if defined(WLPKTDLYSTAT) || defined(WL11K)
		uint32		enqtime;	/* Time when packet was enqueued into the FIFO */
#endif
	} shared;
#if defined(PROP_TXSTATUS)
	/* proptxtstatus uses this field for host specified seqnum and other flags */
	uint32		wl_hdr_information;
#endif /* PROP_TXSTATUS */
#ifdef WLAWDL_LATENCY_SUPPORT
	uint32	ts[4];
#endif
#ifdef BCM_OL_DEV
	uint16 frameptr;
#endif
} wlc_pkttag_t;

#define WLPKTTAG(p) ((wlc_pkttag_t*)PKTTAG(p))

#define WLPKTTAGCLEAR(p) bzero((char *)PKTTAG(p), sizeof(wlc_pkttag_t))

/* Flags used in wlc_pkttag_t.
 * If adding a flag, be sure to check if WLPKTTAG_FLAG_MOVE should transfer it.
 */
#define WLF_PSMARK		0x00000001	/* PKT marking for PSQ aging */
#define WLF_PSDONTQ		0x00000002	/* PS-Poll response don't queue flag */
#define WLF_MPDU		0x00000004	/* Set if pkt is a PDU as opposed to MSDU */
#define WLF_NON8023		0x00000008	/* original pkt is not 8023 */
#define WLF_8021X		0x00000010	/* original pkt is 802.1x */
#define WLF_APSD		0x00000020	/* APSD delivery frame */
#define WLF_AMSDU		0x00000040	/* pkt is aggregated msdu */
/* we're out of space, so reusing this rx bit on tx side */
#define WLF_HWAMSDU		0x00000080	/* Rx: HW/ucode has deaggregated this A-MSDU */
#define WLF_TXHDR		0x00000080	/* Tx: pkt is 802.11 MPDU with plcp and txhdr */
#define WLF_FIFOPKT		0x00000100	/* Used by WL_MULTIQUEUE module if pkt recovered *
						 * from FIFO
						 */
#define WLF_HOST_PKT		0x00000100	/* pkt coming from host/bridge; overload
						 * WLF_FIFOPKT. set when pkt
						 * leaves per port layer and
						 * clear in wl layer.
						 */
#define WLF_EXPTIME		0x00000200	/* pkttag has a valid expiration time for the pkt */
#define WLF_AMPDU_MPDU		0x00000400	/* mpdu in a ampdu */
#define WLF_MIMO		0x00000800	/* mpdu has a mimo rate */
#define WLF_RIFS		0x00001000	/* frameburst with RIFS separated */
#define WLF_VRATE_PROBE		0x00002000	/* vertical rate probe mpdu */
#define WLF_BSS_DOWN		0x00004000	/* The BSS associated with the pkt has gone down */
#ifdef BCM_OL_DEV
#define WLF_NULLPKT		0x00004000	/* offloads code overrides the flag
						 * to identify NULL pkt
						 */
#else
#define WLF_BYPASS_TXC		0x00004000	/* bypass txc lookup and update.
						 * assuming only used by data packet
						 * hence it won't conflict
						 * with WLF_BSS_DOWN used by MBSS beacon.
						 */
#endif /* BCM_OL_DEV */
#define WLF_TXCMISS		0x00008000	/* Packet missed tx cache lkup (overloaded below) */
#define WLF_RX_KM       	0x00008000      /* ICV trimmed off Rx pkt: pktfetch case */
#define WLF_EXEMPT_MASK		0x00030000	/* mask for encryption exemption (Vista) */
#define WLF_WME_NOACK		0x00040000	/* pkt use WME No ACK policy */
#ifdef DMATXRC
#define WLF_PHDR		0x00080000      /* pkt hdr */
#endif
#ifdef PROP_TXSTATUS
#define WLF_CREDITED		0x00100000	/* AC credit for this pkt has been provided to
						 * the host
						 */
#endif
#if (defined(BCMCCX) && defined(CCX_SDK))
#define WLF_IHV_TX_PKT		0x00100000	/* tx pkt from IHV */
#define WLF_TX_PKT_IDX_MASK	0x00600000	/* tx pkt index for SDK interface(IHV_TX_Q_SIZE) */
#define WLF_TX_PKT_IDX_SHIFT	21
#endif /* BCMCCX && CCX_SDK */
#define WLF_TDLS_TYPE		0x00800000	/* pkt is of TDLS type */
#define WLF_TDLS_DIRECT		0x01000000	/* pkt will use direct TDLS path */
#define WLF_TDLS_APPATH         0x02000000      /* pkt will use AP path */
#define WLF_USERTS		0x04000000	/* protect the packet with RTS/CTS */
#define WLF_RATE_AUTO		0x08000000	/* pkt uses rates from the rate selection module */
#define WLF_PROPTX_PROCESSED    0x10000000	/* marks pkt proptx processed  */
#ifdef WLBTAMP
#define WLF_BTA_SRM		0x20000000	/* xmit the pkt using BTAMP target power */
#endif
#define WLF_MFP			0x20000000	/* pkt is MFP */
#define WLF_DATA		0x40000000	/* pkt is pure data */
#ifdef BCMWAPI_WAI
#define WLF_WAI			0x80000000	/* original pkt is WAI */
#endif

/* Flags2 used in wlc_pkttag_t (8 bits). */
#define WLF2_PCB1_MASK		0x0f	/* see pcb1 definitions */
#define WLF2_PCB1_SHIFT		0
#define WLF2_PCB2_MASK		0x30	/* see pcb2 definitions */
#define WLF2_PCB2_SHIFT		4
#define WLF2_PCB3_MASK		0x40	/* see pcb3 definitions */
#define WLF2_PCB3_SHIFT		6
#define WLF2_PCB4_MASK		0x80	/* see pcb4 definitions */
#define WLF2_PCB4_SHIFT		7
#define WLF2_HOSTREORDERAMPDU_INFO 0x20 /* packet has info to help host reorder agg packets  */

/* Flags3 used in wlc_pkttag_t (8 bits). */
#define WLF3_SUPR		0x01	/* pkt was suppressed due to PM 1 or NoA ABS */
#define WLF3_FAVORED		0x02	/* pkt is favored. Non-favored can be dropped */
#define WLF3_NO_PMCHANGE	0x04	/* don't touch pm state for this, e.g. keep_alive */
#define WLF3_TDLS_BYPASS_AMPDU	0x08	/* TDLS signalling packet to bypass AMPDU */
#define WLF3_AWDL_TX			0x08	/* overload with TDLS, AWDL/TDLS should not coex */
#define WLF3_TXQ_SHORT_LIFETIME	0x10	/* pkt in TXQ is short-lived, for channel switch case */
#define WLF3_DATA_TCP_ACK	0x20	/* pkt is TCP_ACK data */
#define WLF3_AMPDU_REGMPDU	0x40	/* rempdu in ampdu module */
#define WLF3_DATA_WOWL_PKT	0x80
/* Using the same flag as WOWL as its not used in Rx path */
#define WLF3_REORDERDATA_PROCESSED 0x80

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
#define WLF2_PCB1_STA_PRB	1	/* wlc_ap_sta|wds_probe_complete */
#define WLF2_PCB1_PSP		2	/* wlc_sendpspoll_complete */
#define WLF2_PCB1_AF		3	/* wlc_actionframetx_complete */
#ifdef WLBTAMP
/* also used to identify HCI ACL packet type */
#define WLF2_PCB1_BTA_HCI_ACL	4	/* wlc_bta_tx_hcidata_complete */
#endif
#define WLF2_PCB1_TKIP_CM	5	/* wlc_tkip_countermeasure */
#define WLF2_PCB1_RATE_PRB	6	/* wlc_rateprobe_complete */
#define WLF2_PCB1_PM_NOTIF	8	/* wlc_pm_notif_complete */
#define WLF2_PCB1_NAR		9	/* nar callback when pkt freed */
#ifdef WL_RELMCAST
#define WLF2_PCB1_RMC		10	/* rmc callback when pkt freed */
#endif
#ifdef WLOLPC
#define WLF2_PCB1_OLPC		11	/* olpc calibration pkts */
#endif /* WLOLPC */
#define WLF2_PCB1_NAN		12	/* nan_mac_txstatus */
#define WLF2_PCB1_WLFC		13	/* wl_hostpkt_callback */
#define WLF2_PCB1_AIBSS_DATA	14	/* AIBSS TX packets */



/* macros to access the pkttag.flags2.pcb1 field */
#define WLF2_PCB1(p)		((WLPKTTAG(p)->flags2 & WLF2_PCB1_MASK) >> WLF2_PCB1_SHIFT)
#define WLF2_PCB1_REG(p, cb)	(WLPKTTAG(p)->flags2 &= ~WLF2_PCB1_MASK, \
				 WLPKTTAG(p)->flags2 |= (cb) << WLF2_PCB1_SHIFT)
#define WLF2_PCB1_UNREG(p)	(WLPKTTAG(p)->flags2 &= ~WLF2_PCB1_MASK)

/* pcb2 */
#define WLF2_PCB2_APSD		1	/* wlc_apps_apsd_complete */
#define WLF2_PCB2_PSP_RSP	2	/* wlc_apps_psp_resp_complete */
#define WLF2_PCB2_AIBSS_CTRL	3	/* wlc_aibss_ctrl_pkt_txstatus */

/* macros to access the pkttag.flags2.pcb2 field */
#define WLF2_PCB2(p)		((WLPKTTAG(p)->flags2 & WLF2_PCB2_MASK) >> WLF2_PCB2_SHIFT)
#define WLF2_PCB2_REG(p, cb)	(WLPKTTAG(p)->flags2 &= ~WLF2_PCB2_MASK, \
				 WLPKTTAG(p)->flags2 |= (cb) << WLF2_PCB2_SHIFT)
#define WLF2_PCB2_UNREG(p)	(WLPKTTAG(p)->flags2 &= ~WLF2_PCB2_MASK)

/* pcb3 */
#define WLF2_PCB3_AMSDU		1	/* wlc_amsdu_pkt_freed */

/* macros to access the pkttag.flags2.pcb3 field */
#define WLF2_PCB3(p)		((WLPKTTAG(p)->flags2 & WLF2_PCB3_MASK) >> WLF2_PCB3_SHIFT)
#define WLF2_PCB3_REG(p, cb)	(WLPKTTAG(p)->flags2 &= ~WLF2_PCB3_MASK, \
				 WLPKTTAG(p)->flags2 |= (cb) << WLF2_PCB3_SHIFT)
#define WLF2_PCB3_UNREG(p)	(WLPKTTAG(p)->flags2 &= ~WLF2_PCB3_MASK)

/* pcb4 */
#define WLF2_PCB4_AMPDU		1	/* wlc_ampdu_pkt_freed */

/* macros to access the pkttag.flags2.pcb4 field */
#define WLF2_PCB4(p)		((WLPKTTAG(p)->flags2 & WLF2_PCB4_MASK) >> WLF2_PCB4_SHIFT)
#define WLF2_PCB4_REG(p, cb)	(WLPKTTAG(p)->flags2 &= ~WLF2_PCB4_MASK, \
				 WLPKTTAG(p)->flags2 |= (cb) << WLF2_PCB4_SHIFT)
#define WLF2_PCB4_UNREG(p)	(WLPKTTAG(p)->flags2 &= ~WLF2_PCB4_MASK)

#include <packed_section_start.h>
typedef BWL_PRE_PACKED_STRUCT struct
{
	uint16 rate;
	int8   rssi;
	uint8  channel;
}
BWL_POST_PACKED_STRUCT rx_ctxt_t;
#include <packed_section_end.h>

/* EXT_STA */
typedef struct {
	uint16 exempt_type;	/* Ethertype allowed to be received unencrypted */
	uint16 exempt_action;	/* 1 - always (802.1x), 2 - key unavailable (WPA) */
	uint16 exempt_pkttype;	/* 1 - ucast, 2 - mcast, 3 - both */
} exempt_t;

typedef struct {
	uint entries;		/* total exempt entries */
	exempt_t exempt[1];	/* exempt list */
} exempt_list_t;

typedef enum {
	WLC_EXEMPT_ALWAYS,
	WLC_EXEMPT_NO_KEY,
	WLC_EXEMPT_ACTION_MAX
} exempt_action_t;

typedef enum {
	WLC_EXEMPT_UNICAST,
	WLC_EXEMPT_MULTICAST,
	WLC_EXEMPT_BOTH,
	WLC_EXEMPT_PKTTYPE_MAX
} exempt_pkttype_t;

#define WLC_EXEMPT_LIST_LEN(entries) (OFFSETOF(exempt_list_t, exempt) + \
	(sizeof(exempt_t) * entries))

#ifdef WLAMPDU
#define WLPKTFLAG_AMPDU(pkttag)	((pkttag)->flags & WLF_AMPDU_MPDU)
#else
#define WLPKTFLAG_AMPDU(pkttag)	FALSE
#endif

#ifdef WLAMSDU
#define WLPKTFLAG_AMSDU(pkttag)	((pkttag)->flags & WLF_AMSDU)
#else
#define WLPKTFLAG_AMSDU(pkttag)	FALSE
#endif

#if (defined(BCMCCX) && defined(CCX_SDK)) || defined(MFP)
/* flag for .11w Protected Management Frames(PMF) and ccx Management Frame Protection(MFP) */
#define WLPKTFLAG_PMF(pkttag)	((pkttag)->flags & WLF_MFP)
#else
#define WLPKTFLAG_PMF(pkttag)	FALSE
#endif /* BCMCCX && CCX_SDK */

/* AWDL Support */
#define AWDL_SUPPORT(pub)		(0)
#define AWDL_ENAB(pub)			(0)

/* LTE coex support */
#ifdef BCMLTECOEX
#if defined(WL_ENAB_RUNTIME_CHECK)
#define BCMLTECOEX_ENAB(pub)	((pub)->_ltecx)
#define BCMLTECOEXGCI_ENAB(pub)	((pub)->_ltecxgci)
#elif defined(BCMLTECOEX_DISABLED)
#define BCMLTECOEX_ENAB(pub)	(0)
#define BCMLTECOEXGCI_ENAB(pub)	(0)
#else
#define BCMLTECOEX_ENAB(pub)	((pub)->_ltecx)
#define BCMLTECOEXGCI_ENAB(pub)	((pub)->_ltecxgci)
#endif
#else
#define BCMLTECOEX_ENAB(pub)	(0)
#define BCMLTECOEXGCI_ENAB(pub)	(0)
#endif /* BCMLTECOEX */

#ifdef WLBTAMP
#define WLPKTFLAG_BTA_HCI_ACL(pkttag)	(((pkttag)->flags2 & WLF2_PCB1_MASK) == \
					  WLF2_PCB1_BTA_HCI_ACL << WLF2_PCB1_SHIFT)
#endif

#define WLPKTFLAG_RIFS(pkttag)	((pkttag)->flags & WLF_RIFS)

#define WLPKTFLAG_BSS_DOWN_GET(pkttag) ((pkttag)->flags & WLF_BSS_DOWN)
#define WLPKTFLAG_BSS_DOWN_SET(pkttag, val) (pkttag)->flags |= ((val) ? WLF_BSS_DOWN : 0)

#define WLPKTFLAG_EXEMPT_GET(pkttag) (((pkttag)->flags & WLF_EXEMPT_MASK) >> 16)
#define WLPKTFLAG_EXEMPT_SET(pkttag, val) ((pkttag)->flags = \
			((pkttag)->flags & ~WLF_EXEMPT_MASK) | (val << 16));
#define WLPKTFLAG_NOACK(pkttag)	((pkttag)->flags & WLF_WME_NOACK)

/* API for accessing BSSCFG index in WLPKTTAG */
#define BSSCFGIDX_ISVALID(bsscfgidx) (((bsscfgidx >= 0)&&(bsscfgidx < WLC_MAXBSSCFG)) ? 1 : 0)

static INLINE int8
wlc_pkttag_bsscfg_get(void *p)
{
	int8 idx = WLPKTTAG(p)->_bsscfgidx;
#ifdef BCMDBG
	ASSERT(BSSCFGIDX_ISVALID(idx));
#endif
	return idx;
}

#define WLPKTTAGBSSCFGGET(p) (wlc_pkttag_bsscfg_get(p))
#define WLPKTTAGBSSCFGSET(p, bsscfgidx) (WLPKTTAG(p)->_bsscfgidx = bsscfgidx)

/* Raw get of bss idx from pkt tag without error checking */
#define WLPKTTAG_BSSIDX_GET(pkttag) ((pkttag)->_bsscfgidx)

/* API for accessing SCB pointer in WLPKTTAG */
#define WLPKTTAGSCBGET(p)	(WLPKTTAG(p)->_scb)
#define WLPKTTAGSCBSET(p, scb)	(WLPKTTAG(p)->_scb = scb)
#define WLPKTTAGSCBCLR(p)	(WLPKTTAG(p)->_scb = NULL)
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

/* WL_ENAB_RUNTIME_CHECK may be set based upon the #define below (for ROM builds). It may also
 * be defined via makefiles (e.g. ROM auto abandon unoptimized compiles).
 */
#if defined(BCMROMBUILD)
	#ifndef WL_ENAB_RUNTIME_CHECK
		#define WL_ENAB_RUNTIME_CHECK
	#endif
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
	#if defined(WL_ENAB_RUNTIME_CHECK) || !defined(DONGLEBUILD)
		#define APCS_ENAB(pub) ((pub)->_apcs)
	#elif defined(APCS_DISABLED)
		#define APCS_ENAB(pub) (0)
	#else
		#define APCS_ENAB(pub) (1)
	#endif
#else /* !APCS */
	#define APCS_ENAB(pub) (0)
#endif /* APCS */

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
#endif

#if defined(DONGLEBUILD) && defined(WET)
#error "Use WET_DONGLE iso WET for donglebuilds"
#endif

#ifdef WET
#define WET_ENAB(wlc)		(wlc->wet)
#else
#define WET_ENAB(wlc)		(0)
#endif

#ifdef WET_DONGLE
#define WET_DONGLE_ENAB(wlc)	(wlc->wet_dongle)
#else
#define WET_DONGLE_ENAB(wlc)	(0)
#endif

#if defined(PKTC) || defined(PKTC_DONGLE)
#define PKTC_ENAB(pub)	((pub)->_pktc)
#else
#define PKTC_ENAB(pub)	(0)
#endif

/* TX Beamforming Support */
#if defined(WL_BEAMFORMING)
	#if defined(WL_ENAB_RUNTIME_CHECK) || !defined(DONGLEBUILD)
		#define TXBF_ENAB(pub)		((pub)->_txbf)
	#elif defined(WLTXBF_DISABLED)
		#define TXBF_ENAB(pub)		(0)
	#else
		#define TXBF_ENAB(pub)		(1)
	#endif
#else
	#define TXBF_ENAB(pub)			(0)
#endif /* WL_BEAMFORMING */

/* STA Priority Feature Support  */
#ifdef WL_STAPRIO
	#if defined(WL_ENAB_RUNTIME_CHECK)
		#define STAPRIO_ENAB(pub)		((pub)->_staprio)
	#elif defined(WL_STAPRIO_DISABLED)
		#define STAPRIO_ENAB(pub)		(0)
	#else
		#define STAPRIO_ENAB(pub)		(1)
	#endif
#else
	#define STAPRIO_ENAB(pub)			(0)
#endif /* WL_STAPRIO */

#ifdef WL_STA_MONITOR
#define STAMON_ENAB(pub) ((pub)->_stamon)
#else
#define STAMON_ENAB(pub) (0)
#endif /* WL_STA_MONITOR */

/* Some useful combinations */
#define STA_ONLY(pub)	(!AP_ENAB(pub))
#define AP_ONLY(pub)	(AP_ENAB(pub) && !APSTA_ENAB(pub))

/* proptxstatus feature dynamic enable, compile time enable or disable */
#if defined(PROP_TXSTATUS)
	#if defined(WL_ENAB_RUNTIME_CHECK) || !defined(DONGLEBUILD)
		#define PROP_TXSTATUS_ENAB(pub)		((pub)->_proptxstatus)
	#elif defined(PROP_TXSTATUS_ENABLED)
		#define PROP_TXSTATUS_ENAB(pub)		1
	#else
		#define PROP_TXSTATUS_ENAB(pub)		0
	#endif
#else
	#define PROP_TXSTATUS_ENAB(pub)		0
#endif /* PROP_TXSTATUS */

/* Send Rxed packet information to host through TLV's */
#if defined(PROP_TXSTATUS)
	#if defined(WL_ENAB_RUNTIME_CHECK) || !defined(DONGLEBUILD)
		#define RXMETADATA_TO_HOST_ENAB(pub)	((pub)->_rxmetadata_to_host)
	#elif defined(RXMETADATA_TO_HOST_DISABLED)
		#define RXMETADATA_TO_HOST_ENAB(pub)	0
	#else
		#define RXMETADATA_TO_HOST_ENAB(pub)	((pub)->_rxmetadata_to_host)
	#endif
#else
	#define RXMETADATA_TO_HOST_ENAB(pub)	0
#endif /* PROP_TXSTATUS */

/* Send txed packet information to host through TLV's */
#if defined(PROP_TXSTATUS)
	#if defined(WL_ENAB_RUNTIME_CHECK) || !defined(DONGLEBUILD)
		#define TXMETADATA_TO_HOST_ENAB(pub)	((pub)->_txmetadata_to_host)
	#elif defined(TXMETADATA_TO_HOST_DISABLED)
		#define TXMETADATA_TO_HOST_ENAB(pub)	0
	#else
		#define TXMETADATA_TO_HOST_ENAB(pub)	((pub)->_txmetadata_to_host)
	#endif
#else
	#define TXMETADATA_TO_HOST_ENAB(pub)	0
#endif /* PROP_TXSTATUS */

/* Send Flow control information to host through TLV's */
#if defined(PROP_TXSTATUS)
	#if defined(WL_ENAB_RUNTIME_CHECK) || !defined(DONGLEBUILD)
		#define WLFC_CONTROL_SIGNALS_TO_HOST_ENAB(pub)	((pub)->_wlfc_ctrl_to_host)
	#elif defined(WLFC_CONTROL_TO_HOST_DISABLED)
		#define WLFC_CONTROL_SIGNALS_TO_HOST_ENAB(pub)	0
	#else
		#define WLFC_CONTROL_SIGNALS_TO_HOST_ENAB(pub)	1
	#endif
#else
	#define WLFC_CONTROL_SIGNALS_TO_HOST_ENAB(pub)	0
#endif /* PROP_TXSTATUS */

/* Send Flow control information to bus through TLV's, bus consumes FC info */
#if defined(PROP_TXSTATUS)
	#if defined(WL_ENAB_RUNTIME_CHECK) || !defined(DONGLEBUILD)
		#define WLFC_INFO_TO_BUS_ENAB(pub)	((pub)->_wlfc_info_to_bus)
	#elif defined(WLFC_INFO_TO_BUS_DISABLED)
		#define WLFC_INFO_TO_BUS_ENAB(pub)	0
	#else
		#define WLFC_INFO_TO_BUS_ENAB(pub)	1
	#endif
#else
	#define WLFC_INFO_TO_BUS_ENAB(pub)	0
#endif /* PROP_TXSTATUS */

#ifndef LINUX_POSTMOGRIFY_REMOVAL
/* Primary MBSS enable check macro */
#define MBSS_OFF	0
#define MBSS4_ENABLED	1
#define MBSS16_ENABLED	2
#if defined(MBSS)
	#if defined(WL_ENAB_RUNTIME_CHECK) || !defined(DONGLEBUILD)
		#define MBSS_ENAB(pub)		((pub)->_mbss_mode)
		#define MBSS_ENAB4(pub)		((pub)->_mbss_mode == MBSS4_ENABLED)
		#define MBSS_ENAB16(pub)	((pub)->_mbss_mode == MBSS16_ENABLED)
		#define MBSS_SUPPORT(pub)	((pub)->_mbss_support)
		#define MBSS_IGN_MAC_VALID(pub)	((pub)->_mbss_ign_mac_valid && \
			(pub)->_mbss_mode)
	#elif defined(MBSS_DISABLED)
		#define MBSS_ENAB(pub)		(0)
		#define MBSS_ENAB4(pub)		(0)
		#define MBSS_ENAB16(pub)	(0)
		#define MBSS_SUPPORT(pub)	(0)
		#define MBSS_IGN_MAC_VALID(pub)	(0)
	#else
		#define MBSS_ENAB(pub)		((pub)->_mbss_mode)
		#define MBSS_ENAB4(pub)		((pub)->_mbss_mode == MBSS4_ENABLED)
		#define MBSS_ENAB16(pub)	((pub)->_mbss_mode == MBSS16_ENABLED)
		#define MBSS_SUPPORT(pub)	(1)
		#define MBSS_IGN_MAC_VALID(pub)	((pub)->_mbss_ign_mac_valid && \
			(pub)->_mbss_mode)
	#endif
#else /* !MBSS */
	#define MBSS_ENAB(pub)			(0)
	#define MBSS_ENAB4(pub)			(0)
	#define MBSS_ENAB16(pub)		(0)
	#define MBSS_SUPPORT(pub)		(0)
	#define MBSS_IGN_MAC_VALID(pub)		(0)
#endif /* MBSS */

#if defined(WME_PER_AC_TX_PARAMS)
#define WME_PER_AC_TX_PARAMS_ENAB(pub) (1)
#define WME_PER_AC_MAXRATE_ENAB(pub) ((pub)->_per_ac_maxrate)
#else /* WME_PER_AC_TX_PARAMS */
#define WME_PER_AC_TX_PARAMS_ENAB(pub) (0)
#define WME_PER_AC_MAXRATE_ENAB(pub) (0)
#endif /* WME_PER_AC_TX_PARAMS */

#define ENAB_1x1	0x01
#define ENAB_2x2	0x02
#define ENAB_3x3	0x04
#define ENAB_4x4	0x08
#define SUPPORT_11N	(ENAB_1x1|ENAB_2x2)
#define SUPPORT_HT	(ENAB_1x1|ENAB_2x2|ENAB_3x3)
/* WL11N Support */
#if defined(WL11N) && ((defined(NCONF) && (NCONF != 0)) || (defined(SSLPNCONF) && \
	(SSLPNCONF != 0)) || (defined(LCNCONF) && (LCNCONF != 0)) || (defined(LCN40CONF) && \
	(LCN40CONF != 0)) || (defined(ACCONF) && (ACCONF != 0)) || (defined(ACCONF2) && \
	(ACCONF2 != 0)) || (defined(HTCONF) && (HTCONF != 0)))
#define N_ENAB(pub) ((pub)->_n_enab & SUPPORT_11N)
#define N_REQD(pub) ((pub)->_n_reqd)
#else
#define N_ENAB(pub)	((void)(pub), 0)
#define N_REQD(pub)	((void)(pub), 0)
#endif

/* WL11AC Support */
#if defined(WL11AC) && ((defined(ACCONF) && (ACCONF != 0)) || (defined(ACCONF2) && \
	(ACCONF2 != 0)))

/*
 * VHT features rates  bitmap.
 * Bits 0:7 is the same as the rate expansion field in the proprietary IE
 * Bit 0:		5G MCS 0-9 BW 160MHz
 * Bit 1:		5G MCS 0-9 support BW 80MHz
 * Bit 2:		5G MCS 0-9 support BW 20MHz
 * Bit 3:		2.4G MCS 0-9 support BW 20MHz
 * Bit 4:		Prop MCS 10-11 support
 * Bits 5:7	Reserved for future use
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

#endif /* WL11AC Support */

/* Open Loop Phy Calibration support */
/* only supported for 11AC chips */
#if defined(WLOLPC) && defined(WL11AC)
#define OLPC_ENAB(wlc)  ((wlc)->pub->_olpc)
#else
#define OLPC_ENAB(wlc)  (FALSE)
#endif /* WLOLPC && WL11AC */

/* Block Ack Support */
#ifdef WLBA
#define WLBA_ENAB(pub) ((pub)->_ba)
#else
#define WLBA_ENAB(pub) 0
#endif /* WLBA */

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

/* Dongle Offload Feature bitmap. */

/* Bits used from iovar */
#define OL_BCN_IDX			0
#define OL_ARP_IDX			1
#define OL_ND_IDX			2
#define OL_IE_IDX			3

/* Bits used internally */
#define OL_PKT_FILTER_IDX	4
#define OL_WOWL_IDX			5

#define OL_BCN_ENAB	             (1 << OL_BCN_IDX)
#define OL_ARP_ENAB	             (1 << OL_ARP_IDX)
#define OL_ND_ENAB               (1 << OL_ND_IDX)
#define OL_IE_NOTIFICATION_ENAB	 (1 << OL_IE_IDX)
#define OL_PKT_FILTER_ENAB       (1 << OL_PKT_FILTER_IDX)
#define OL_WOWL_ENAB             (1 << OL_WOWL_IDX)

#ifdef WLOFFLD
#define WLOFFLD_CAP(wlc)        ((wlc)->ol != NULL)
#define WLOFFLD_ENAB(pub)       ((pub)->_ol)
#define WLOFFLD_BCN_ENAB(pub)   ((pub)->_ol & OL_BCN_ENAB)
#define WLOFFLD_ARP_ENAB(pub)   ((pub)->_ol & OL_ARP_ENAB)
#define WLOFFLD_ND_ENAB(pub)    ((pub)->_ol & OL_ND_ENAB)
#else
#define WLOFFLD_CAP(wlc)        0
#define WLOFFLD_ENAB(pub)       0
#define WLOFFLD_BCN_ENAB(pub)   0
#define WLOFFLD_ARP_ENAB(pub)   0
#define WLOFFLD_ND_ENAB(pub)    0
#endif /* WLOFFLD */

/* WLAMPDUMAC Support */
#ifdef WLAMPDU_MAC
	#if ((defined(WLAMPDU_HW) && defined(WLAMPDU_UCODE)) || (defined(WLAMPDU_HW) && \
	defined(WLAMPDU_AQM)) || (defined(WLAMPDU_UCODE) && defined(WLAMPDU_AQM)) || \
	(defined(WLC_HIGH) && !defined(WLC_LOW)) || defined(WL_ENAB_RUNTIME_CHECK) || \
	!defined(DONGLEBUILD))
		#define AMPDU_UCODE_ENAB(pub)	((pub)->_ampdumac == AMPDU_AGG_UCODE)
		#define AMPDU_HW_ENAB(pub)	((pub)->_ampdumac == AMPDU_AGG_HW)
		#define AMPDU_AQM_ENAB(pub)	((pub)->_ampdumac == AMPDU_AGG_AQM)
	#elif defined(WLAMPDU_UCODE) && defined(WLAMPDU_UCODE_ONLY)
		#define AMPDU_UCODE_ENAB(pub)	(1)
		#define AMPDU_HW_ENAB(pub)      (0)
		#define AMPDU_AQM_ENAB(pub)	(0)
	#elif defined(WLAMPDU_AQM)
		#define AMPDU_UCODE_ENAB(pub)   (0)
		#define AMPDU_HW_ENAB(pub)      (0)
		#define AMPDU_AQM_ENAB(pub)	(1)
	#elif defined(WLAMPDU_HW)
		#define AMPDU_UCODE_ENAB(pub)   (0)
		#define AMPDU_HW_ENAB(pub)      (1)
		#define AMPDU_AQM_ENAB(pub)	(0)
	#elif defined(WLAMPDU_UCODE)
		#define AMPDU_UCODE_ENAB(pub)	((pub)->_ampdumac == AMPDU_AGG_UCODE)
		#define AMPDU_HW_ENAB(pub)	(0)
		#define AMPDU_AQM_ENAB(pub)	(0)
	#endif /* various compile/run-time versions of AMPDU_xxx_ENAB() checks */
#else /* WLAMPDU_MAC */
	#define AMPDU_UCODE_ENAB(pub)		(0)
	#define AMPDU_HW_ENAB(pub)		(0)
	#define AMPDU_AQM_ENAB(pub)		(0)
#endif /* WLAMPDU_MAC */

#define AMPDU_MAC_ENAB(pub) (AMPDU_UCODE_ENAB(pub) || AMPDU_HW_ENAB(pub) || \
	AMPDU_AQM_ENAB(pub))
#define AMPDU_HOST_ENAB(pub) (!AMPDU_UCODE_ENAB(pub) && !AMPDU_HW_ENAB(pub) && \
	!AMPDU_AQM_ENAB(pub))

/* WOWL support */
#ifdef WOWL
#define WOWL_ENAB(pub) ((pub)->_wowl)
#define WOWL_ACTIVE(pub) ((pub)->_wowl_active)
#else
#define WOWL_ACTIVE(wlc) (0)
#define WOWL_ENAB(pub) (0)
#endif /* WOWL */

#ifdef WOWLPF
	#if defined(WL_ENAB_RUNTIME_CHECK) || !defined(DONGLEBUILD)
		#define WOWLPF_ENAB(pub) ((pub)->_wowlpf)
		#define WOWLPF_ACTIVE(pub) ((pub)->_wowlpf_active)
	#elif defined(WOWLPF_DISABLED)
		#define WOWLPF_ENAB(pub) (0)
		#define WOWLPF_ACTIVE(pub) (0)
	#else
		#define WOWLPF_ENAB(pub) (1)
		#define WOWLPF_ACTIVE(pub) ((pub)->_wowlpf_active)
	#endif
#else
	#define WOWLPF_ACTIVE(wlc) (0)
	#define WOWLPF_ENAB(pub) (0)
#endif /* WOWLPF */


/* WLFCTS or PROP_TXSTATUS_TIMESTAMP Support */
#ifdef WLFCTS
	#if defined(WL_ENAB_RUNTIME_CHECK)
		#define WLFCTS_ENAB(pub)		((pub)->_wlfcts)
	#elif defined(WLFCTS_DISABLED)
		#define WLFCTS_ENAB(pub)		(0)
	#else
		#define WLFCTS_ENAB(pub)		(1)
	#endif
#else
	#define WLFCTS_ENAB(pub)			(0)
#endif /* WLFCTS */

/* TDLS Support */
#ifdef WLTDLS
	#if defined(WL_ENAB_RUNTIME_CHECK)
		#define TDLS_SUPPORT(pub)	((pub)->_tdls_support)
		#define TDLS_ENAB(pub)		((pub)->_tdls)
	#elif defined(WLTDLS_DISABLED)
		#define TDLS_SUPPORT(pub)	(0)
		#define TDLS_ENAB(pub)		(0)
	#else
		#define TDLS_SUPPORT(pub)	(1)
		#define TDLS_ENAB(pub)		((pub)->_tdls)
	#endif
#else
	#define TDLS_SUPPORT(pub)		(0)
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
#define OKC_ENAB(pub) ((pub)->_okc)
#else
#define OKC_ENAB(pub) 0
#endif

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
		#endif
	#elif defined(WL_ENAB_RUNTIME_CHECK)
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
	#if defined(WL_ENAB_RUNTIME_CHECK) || !defined(DONGLEBUILD)
		#define P2P_ENAB(pub) ((pub)->_p2p)
	#elif defined(WLP2P_DISABLED)
		#define P2P_ENAB(pub)	((void)(pub), 0)
	#else
		#define P2P_ENAB(pub)	((void)(pub), 1)
	#endif
	#ifdef WLWFDS
		#if defined(WL_ENAB_RUNTIME_CHECK) || !defined(DONGLEBUILD)
			#define WFDS_ENAB(pub) ((pub)->_wfds)
		#elif defined(WLWFDS_DISABLED)
			#define WFDS_ENAB(pub) (0)
		#else
			#define WFDS_ENAB(pub) (1)
		#endif
	#else
		#define WFDS_ENAB(pub) (0)
	#endif /* WLWFDS */
#else
	#define P2P_ENAB(pub) ((void)(pub), 0)
	#define WFDS_ENAB(pub) 0
#endif /* WLP2P */

/* MFP Support */
#ifdef MFP
	#if defined(WL_ENAB_RUNTIME_CHECK) || !defined(DONGLEBUILD)
		#define WLC_MFP_ENAB(pub) ((pub)->_mfp)
	#elif defined(MFP_DISABLED)
		#define WLC_MFP_ENAB(pub)	((void)(pub), 0)
	#else
		#define WLC_MFP_ENAB(pub)	((void)(pub), 1)
	#endif
#else
	#define WLC_MFP_ENAB(pub) ((void)(pub), 0)
#endif /* MFP */

/* WLMCHAN Support */
#ifdef WLMCHAN
	#if defined(WL_ENAB_RUNTIME_CHECK) || !defined(DONGLEBUILD)
		#define MCHAN_ENAB(pub) ((pub)->_mchan)
		#define MCHAN_ACTIVE(pub) ((pub)->_mchan_active)
	#elif defined(WLMCHAN_DISABLED)
		#define MCHAN_ENAB(pub) (0)
		#define MCHAN_ACTIVE(pub) (0)
	#else
		#define MCHAN_ENAB(pub) ((pub)->_mchan)
		#define MCHAN_ACTIVE(pub) ((pub)->_mchan_active)
	#endif
#else
	#define MCHAN_ENAB(pub) (0)
	#define MCHAN_ACTIVE(pub) (0)
#endif /* WLMCHAN */

#ifdef WLRSDB
	#if defined(WL_ENAB_RUNTIME_CHECK) || !defined(DONGLEBUILD)
		#define RSDB_ENAB(pub) ((pub)->cmn->_isrsdb)
		#define RSDB_ACTIVE(pub) ((pub)->cmn->_rsdb_active)
	#elif defined(WLRSDB_DISABLED)
		#define RSDB_ENAB(pub) (0)
		#define RSDB_ACTIVE(pub) (0)
	#else
		#define RSDB_ENAB(pub) (1)
		#define RSDB_ACTIVE(pub) ((pub)->cmn->_rsdb_active)
	#endif
#else
	#define RSDB_ENAB(pub) (0)
	#define RSDB_ACTIVE(pub) (0)
#endif /* WLRSDB */

/* BG DFS Radar scan */
#ifdef BGDFS
	#if defined(WL_ENAB_RUNTIME_CHECK) || !defined(DONGLEBUILD)
		#define BGDFS_ENAB(pub)		((pub)->_bgdfs)
	#elif defined(BGDFS_DISABLED)
		#define BGDFS_ENAB(pub)		(0)
	#else
		#define BGDFS_ENAB(pub)		(1)
	#endif
#else
	#define BGDFS_ENAB(pub)		(0)
#endif /* BGDFS */

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
	#if defined(WL_ENAB_RUNTIME_CHECK) || !defined(DONGLEBUILD)
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
		#define DYN160_ENAB(pub)	((pub)->_dyn160)
		#define DYN160_ACTIVE(pub)	(DYN160_ENAB(pub) ? WL_VALID_DYN160(pub) : 0)
		#define DYN160_ACTIVE_SET(pub, val) ((pub)->_dyn160_active = \
				(DYN160_ENAB(pub) ? (val) : 0))
	#endif /* WL_ENAB_RUNTIME_CHECK || !DONGLEBUILD */
#else
	/* eg. NIC or dongle without -dyn160 target */
	#define DYN160_ENAB(pub)		(0)
	#define DYN160_ACTIVE(pub)		(0)
	#define DYN160_ACTIVE_SET(pub, val)	do {} while (0)
#endif /* DYN160 */

#ifdef WL_MULTIQUEUE
	#if defined(WL_ENAB_RUNTIME_CHECK)
		#define MQUEUE_ENAB(pub) ((pub)->_mqueue)
	#elif defined(WLMQUEUE_DISABLED)
		#define MQUEUE_ENAB(pub) (0)
	#else
		#define MQUEUE_ENAB(pub) (1)
	#endif
#else
	#define MQUEUE_ENAB(pub) (0)
#endif /* WL_MULTIQUEUE */

/* WLBTAMP Support */
#ifdef WLBTAMP
	#if defined(WL_ENAB_RUNTIME_CHECK) || !defined(DONGLEBUILD)
		#define BTA_ENAB(pub) ((pub)->_bta)
	#elif defined(WLBTAMP_DISABLED)
		#define BTA_ENAB(pub) ((void)(pub), 0)
	#else
		#define BTA_ENAB(pub) ((void)(pub), 1)
	#endif
#else
	#define BTA_ENAB(pub) ((void)(pub), 0)
#endif /* WLBTAMP */

/* PIO Mode Support */
#ifdef WLPIO
#define PIO_ENAB(pub) ((pub)->_piomode)
#else
#define PIO_ENAB(pub) 0
#endif /* WLPIO */

/* Call Admission Control support */
#ifdef WLCAC
	#if defined(WL_ENAB_RUNTIME_CHECK) || !defined(DONGLEBUILD)
		#define CAC_ENAB(pub)	((pub)->_cac)
	#elif defined(WLCAC_DISABLED)
		#define CAC_ENAB(pub)	0
	#else
		#define CAC_ENAB(pub)	((pub)->_cac)
	#endif
#else
	#define CAC_ENAB(pub) 0
#endif /* WLCAC */

#ifdef BCMSPACE
#define	RXIQEST_ENAB(pub)	(1)
#else
#define	RXIQEST_ENAB(pub)	(0)
#endif

#ifdef WLBTAMP
#define EDCF_ENAB(pub) (WME_ENAB(pub) || BTA_ENAB(pub))
#define QOS_ENAB(pub) (WME_ENAB(pub) || N_ENAB(pub) || BTA_ENAB(pub))
#else
#define EDCF_ENAB(pub) (WME_ENAB(pub))
#define QOS_ENAB(pub) (WME_ENAB(pub) || N_ENAB(pub))
#endif /* WLBTAMP */

#define PRIOFC_ENAB(pub) ((pub)->_priofc)

#if defined(WL_MONITOR) || (defined(BCMCCX) && defined(CCX_SDK))
#define MONITOR_ENAB(wlc)	((wlc)->monitor != 0)
#else
#define MONITOR_ENAB(wlc)	(bcmspace && ((wlc)->monitor != 0))
#endif

#if defined(WL_PROMISC)
#define PROMISC_ENAB(wlc_pub)	(wlc_pub)->promisc
#else
#define PROMISC_ENAB(wlc_pub)	(bcmspace && (wlc_pub)->promisc)
#endif

#if defined(EXT_STA) || defined(MACOSX)
#define WLC_SENDUP_MGMT_ENAB(cfg)	((cfg)->sendup_mgmt)
#else
#define WLC_SENDUP_MGMT_ENAB(cfg)	0
#endif

#ifdef TOE
	#if defined(WL_ENAB_RUNTIME_CHECK)
		#define TOE_ENAB(pub)		((pub)->_toe)
	#elif defined(TOE_DISABLED)
		#define TOE_ENAB(pub)		(0)
	#else
		#define TOE_ENAB(pub)		((pub)->_toe)
	#endif
#else
	#define TOE_ENAB(pub)			(0)
#endif /* TOE */

#ifdef ARPOE
	#if defined(WL_ENAB_RUNTIME_CHECK)
		#define ARPOE_SUPPORT(pub)	((pub)->_arpoe_support)
		#define ARPOE_ENAB(pub)		((pub)->_arpoe)
	#elif defined(ARPOE_DISABLED)
		#define ARPOE_SUPPORT(pub)	(0)
		#define ARPOE_ENAB(pub)		(0)
	#else
		#define ARPOE_SUPPORT(pub)	(1)
		#define ARPOE_ENAB(pub)		((pub)->_arpoe)
	#endif
#else
	#define ARPOE_SUPPORT(pub)		(0)
	#define ARPOE_ENAB(pub)			(0)
#endif /* ARPOE */


#ifdef NWOE
	#if defined(WL_ENAB_RUNTIME_CHECK)
		#define NWOE_ENAB(pub)		((pub)->_nwoe)
	#elif defined(NWOE_DISABLED)
		#define NWOE_ENAB(pub)		(0)
	#else
		#define NWOE_ENAB(pub)		((pub)->_nwoe)
	#endif
#else
	#define NWOE_ENAB(pub)			(0)
#endif /* NWOE */


#ifdef TRAFFIC_MGMT
#define TRAFFIC_MGMT_ENAB(pub) ((pub)->_traffic_mgmt)
	#define TRAFFIC_MGMT_ENAB(pub) ((pub)->_traffic_mgmt)
	#ifdef TRAFFIC_MGMT_DWM
		#define TRAFFIC_MGMT_DWM_ENAB(pub) (TRAFFIC_MGMT_ENAB(pub) && \
				(pub)->_traffic_mgmt_dwm)
	#else
		#define TRAFFIC_MGMT_DWM_ENAB(pub) 0
	#endif
#else
	#define TRAFFIC_MGMT_ENAB(pub) 0
#endif /* TRAFFIC_MGMT */

#ifdef NET_DETECT
#define NET_DETECT_ENAB(pub) ((pub)->_net_detect)
#else
#define NET_DETECT_ENAB(pub) 0
#endif

#ifdef L2_FILTER
	#if defined(WL_ENAB_RUNTIME_CHECK)
		#define L2_FILTER_ENAB(pub)	((pub)->_l2_filter)
	#elif defined(L2_FILTER_DISABLED)
		#define L2_FILTER_ENAB(pub)	(0)
	#else
		#define L2_FILTER_ENAB(pub)	(1)
	#endif
#else
	#define L2_FILTER_ENAB(pub)		(0)
#endif /* L2_FILTER */


#ifdef PACKET_FILTER
#define PKT_FILTER_ENAB(pub) 	((pub)->_pkt_filter)
#else
#define PKT_FILTER_ENAB(pub)	0
#endif

#ifdef D0_COALESCING
#define D0_FILTER_ENAB(pub) 	((pub)->_d0_filter)
#else
#define D0_FILTER_ENAB(pub)	0
#endif

#ifdef P2PO
	#if defined(WL_ENAB_RUNTIME_CHECK) || !defined(DONGLEBUILD)
		#define P2PO_ENAB(pub) ((pub)->_p2po)
	#elif defined(P2PO_DISABLED)
		#define P2PO_ENAB(pub)	(0)
	#else
		#define P2PO_ENAB(pub)	(1)
	#endif
#else
	#define P2PO_ENAB(pub) 0
#endif /* P2PO */

#ifdef ANQPO
	#if defined(WL_ENAB_RUNTIME_CHECK) || !defined(DONGLEBUILD)
		#define ANQPO_ENAB(pub) ((pub)->_anqpo)
	#elif defined(ANQPO_DISABLED)
		#define ANQPO_ENAB(pub)	(0)
	#else
		#define ANQPO_ENAB(pub)	(1)
	#endif
#else
	#define ANQPO_ENAB(pub) 0
#endif /* ANQPO */

#ifdef WL_ASSOC_RECREATE
#define ASSOC_RECREATE_ENAB(pub) ((pub)->_assoc_recreate)
#else
#define ASSOC_RECREATE_ENAB(pub) 0
#endif

#ifdef WLFBT
	#if defined(WL_ENAB_RUNTIME_CHECK) || !defined(DONGLEBUILD)
		#define WLFBT_CAP(pub) ((pub)->_fbt_cap)
		#define WLFBT_ENAB(pub) ((pub)->_fbt)
	#elif defined(WLFBT_DISABLED)
		#define WLFBT_CAP(pub) ((void)(pub), 0)
		#define WLFBT_ENAB(pub) ((void)(pub), 0)
	#else
		#define WLFBT_CAP(pub) ((pub)->_fbt_cap)
		#define WLFBT_ENAB(pub) ((pub)->_fbt)
	#endif
#else
	#define WLFBT_CAP(pub) ((void)(pub), 0)
	#define WLFBT_ENAB(pub) ((void)(pub), 0)
#endif /* WLFBT */

#ifdef WLNDOE
#define NDOE_ENAB(pub) ((pub)->_ndoe)
#else
#define NDOE_ENAB(pub) 0
#endif

#ifdef EXT_STA
	#if defined(WL_ENAB_RUNTIME_CHECK) || !defined(DONGLEBUILD)
		#define WLEXTSTA_ENAB(pub)	((pub)->_extsta)
	#elif defined(EXT_STA_DISABLED)
		#define WLEXTSTA_ENAB(pub)	(0)
	#else
		#define WLEXTSTA_ENAB(pub)	(1)
	#endif
#else
	#define WLEXTSTA_ENAB(pub)		(0)
#endif /* EXT_STA */

#ifdef IBSS_PEER_GROUP_KEY
	#if defined(WL_ENAB_RUNTIME_CHECK) || !defined(DONGLEBUILD)
		#define IBSS_PEER_GROUP_KEY_ENAB(pub) ((pub)->_ibss_peer_group_key)
	#elif defined(IBSS_PEER_GROUP_KEY_DISABLED)
		#define IBSS_PEER_GROUP_KEY_ENAB(pub) (0)
	#else
		#define IBSS_PEER_GROUP_KEY_ENAB(pub) (1)
	#endif
#else
	#define IBSS_PEER_GROUP_KEY_ENAB(pub) (0)
#endif /* !IBSS_PEER_GROUP_KEY */

#ifdef IBSS_PEER_DISCOVERY_EVENT
	#if defined(WL_ENAB_RUNTIME_CHECK) || !defined(DONGLEBUILD)
		#define IBSS_PEER_DISCOVERY_EVENT_ENAB(pub) ((pub)->_ibss_peer_discovery_event)
	#elif defined(IBSS_PEER_DISCOVERY_EVENT_DISABLED)
		#define IBSS_PEER_DISCOVERY_EVENT_ENAB(pub) (0)
	#else
		#define IBSS_PEER_DISCOVERY_EVENT_ENAB(pub) (1)
	#endif
#else
	#define IBSS_PEER_DISCOVERY_EVENT_ENAB(pub) (0)
#endif /* !IBSS_PEER_DISCOVERY_EVENT */

#ifdef IBSS_PEER_MGMT
	#if defined(WL_ENAB_RUNTIME_CHECK) || !defined(DONGLEBUILD)
		#define IBSS_PEER_MGMT_ENAB(pub) ((pub)->_ibss_peer_mgmt)
	#elif defined(IBSS_PEER_MGMT_DISABLED)
		#define IBSS_PEER_MGMT_ENAB(pub) (0)
	#else
		#define IBSS_PEER_MGMT_ENAB(pub) (1)
	#endif
#else
	#define IBSS_PEER_MGMT_ENAB(pub) (0)
#endif /* !IBSS_PEER_MGMT */

#if !defined(WLNOEIND)
	#if defined(WL_ENAB_RUNTIME_CHECK) || !defined(DONGLEBUILD)
		#define WLEIND_ENAB(pub) ((pub)->_wleind)
	#elif defined(WLEIND_DISABLED)
		#define WLEIND_ENAB(pub) (0)
	#else
		#define WLEIND_ENAB(pub) (1)
	#endif
#else
	#define WLEIND_ENAB(pub) (0)
#endif /* ! WLNOEIND */

#ifdef BCMCCX
	#if defined(WL_ENAB_RUNTIME_CHECK) || !defined(DONGLEBUILD)
		#define CCX_ENAB(pub) ((pub)->_ccx)
	#elif defined(BCMCCX_DISABLED)
		#define CCX_ENAB(pub) ((void)(pub), 0)
	#else
		#define CCX_ENAB(pub) ((pub)->_ccx)
	#endif
	#if defined(WL_ENAB_RUNTIME_CHECK) || !defined(DONGLEBUILD)
		#define CCX_AP_KEEP_ALIVE_ENAB(pub) ((pub)->_ccx_aka)
	#elif defined(CCX_AP_KEEP_ALIVE_DISABLED)
		#define CCX_AP_KEEP_ALIVE_ENAB(pub) (0)
	#else
		#define CCX_AP_KEEP_ALIVE_ENAB(pub) (1)
	#endif
#else
	#define CCX_ENAB(pub) ((void)(pub), 0)
#endif  /* BCMCCX */

#ifdef WLRM
	#if defined(WL_ENAB_RUNTIME_CHECK) || !defined(DONGLEBUILD)
		#define WLRM_ENAB(pub) ((pub)->_rm)
	#elif defined(WLRM_DISABLED)
		#define WLRM_ENAB(pub) ((void)(pub), 0)
	#else
		#define WLRM_ENAB(pub) ((void)(pub), 1)
	#endif
#else
	#define WLRM_ENAB(pub) ((void)(pub), 0)
#endif  /* WLRM */

#ifdef BCMAUTH_PSK
	#if defined(WL_ENAB_RUNTIME_CHECK) || !defined(DONGLEBUILD)
		#define BCMAUTH_PSK_ENAB(pub) ((pub)->_bcmauth_psk)
	#elif defined(BCMAUTH_PSK_DISABLED)
		#define BCMAUTH_PSK_ENAB(pub) (0)
	#else
		#define BCMAUTH_PSK_ENAB(pub) (1)
	#endif
#else
	#define BCMAUTH_PSK_ENAB(pub) 0
#endif /* BCMAUTH_PSK */

#ifdef BCMSUP_PSK
	#if defined(WL_ENAB_RUNTIME_CHECK) || !defined(DONGLEBUILD)
		#define SUP_ENAB(pub)	((pub)->_sup_enab)
	#elif defined(BCMSUP_PSK_DISABLED)
		#define SUP_ENAB(pub)	((void)(pub), 0)
	#else
		#define SUP_ENAB(pub)	((void)(pub), 1)
	#endif
#else
	#define SUP_ENAB(pub)	((void)(pub), 0)
#endif /* BCMSUP_PSK */

#ifdef WL11K
	#if defined(WL_ENAB_RUNTIME_CHECK) || !defined(DONGLEBUILD)
		#define WL11K_ENAB(pub)	((pub)->_rrm)
	#elif defined(WL11K_DISABLED)
		#define WL11K_ENAB(pub)	((void)(pub), 0)
	#else
		#define WL11K_ENAB(pub)	((void)(pub), 1)
	#endif
#else
	#define WL11K_ENAB(pub)	((void)(pub), 0)
#endif /* WL11K */

#ifdef WLWNM
	#if defined(WL_ENAB_RUNTIME_CHECK) || !defined(DONGLEBUILD)
		#define WLWNM_ENAB(pub)	((pub)->_wnm)
	#elif defined(WLWNM_DISABLED)
		#define WLWNM_ENAB(pub)	(0)
	#else
		#define WLWNM_ENAB(pub)	(1)
	#endif
#else
	#define WLWNM_ENAB(pub)	(0)
#endif /* WLWNM */

/* WLNINTENDO2 Support */
	#define WLNIN2_ENAB(pub) 0

/* DMA early txreclaim */
#ifdef DMATXRC
	#if defined(WL_ENAB_RUNTIME_CHECK) || !defined(DONGLEBUILD)
		#define DMATXRC_ENAB(pub)	((pub)->_dmatxrc)
	#elif defined(DMATXRC_DISABLED)
		#define DMATXRC_ENAB(pub) 	(0)
	#else
		#define	DMATXRC_ENAB(pub) 	((pub)->_dmatxrc)
	#endif
#else
	#define DMATXRC_ENAB(pub) 		(0)
#endif /* DMATXRC */

#ifdef WLOVERTHRUSTER
	#if defined(WL_ENAB_RUNTIME_CHECK) || !defined(DONGLEBUILD)
		#define OVERTHRUST_ENAB(pub)	((pub)->_wloverthruster)
	#elif defined(WLOVERTHRUSTER_DISABLED)
		#define OVERTHRUST_ENAB(pub)	(0)
	#else
		#define OVERTHRUST_ENAB(pub)	(1)
	#endif
#else
	#define OVERTHRUST_ENAB(pub) 	(0)
#endif /* WLOVERTHRUSTER */

#ifdef TINY_PKTJOIN
	#if defined(WL_ENAB_RUNTIME_CHECK) || !defined(DONGLEBUILD)
		#define TINY_PKTJOIN_ENAB(pub)	((pub)->_tiny_pktjoin)
	#elif defined(TINY_PKTJOIN_DISABLED)
		#define TINY_PKTJOIN_ENAB(pub) 	(0)
	#else
		#define	TINY_PKTJOIN_ENAB(pub) 	(1)
	#endif
#else
	#define TINY_PKTJOIN_ENAB(pub) 		(0)
#endif /* TINY_PKTJOIN */

#ifdef WL_RXEARLYRC
	#if defined(WL_ENAB_RUNTIME_CHECK) || !defined(DONGLEBUILD)
		#define WL_RXEARLYRC_ENAB(pub)	((pub)->_wl_rxearlyrc)
	#elif defined(WL_RXEARLYRC_DISABLED)
		#define WL_RXEARLYRC_ENAB(pub) 	(0)
	#else
		#define	WL_RXEARLYRC_ENAB(pub) 	(1)
	#endif
#else
	#define WL_RXEARLYRC_ENAB(pub) 		(0)
#endif /* WL_RXEARLYRC */

/* rxfifo overflow handler */
#ifdef WLRXOV
	#if defined(WL_ENAB_RUNTIME_CHECK) || !defined(DONGLEBUILD)
		#define WLRXOV_ENAB(pub)	((pub)->_rxov)
	#elif defined(WLRXOV_DISABLED)
		#define WLRXOV_ENAB(pub)	(0)
	#else
		#define	WLRXOV_ENAB(pub)	(1)
	#endif
#else
	#define WLRXOV_ENAB(pub)		(0)
#endif /* WLRXOV */

#ifdef WLPFN
	#if defined(WL_ENAB_RUNTIME_CHECK) || !defined(DONGLEBUILD)
		#define WLPFN_ENAB(pub) ((pub)->_wlpfn)
	#elif defined(WLPFN_DISABLED)
		#define WLPFN_ENAB(pub) (0)
	#else
		#define WLPFN_ENAB(pub) (1)
	#endif
#else
	#define WLPFN_ENAB(pub) (0)
#endif /* WLPFN */

#ifdef WL_PROXDETECT
	#if defined(WL_ENAB_RUNTIME_CHECK)
		#define PROXD_ENAB(pub)	((pub)->_proxd)
	#elif defined(WL_PROXDETECT_DISABLED)
		#define PROXD_ENAB(pub)	(0)
	#else
		#define PROXD_ENAB(pub)	((pub)->_proxd)
	#endif
#else
	#define PROXD_ENAB(pub)		(0)
#endif /* WL_PROXDETECT */

	#define NAN_ENAB(pub)		(0)

#ifdef WLSCANCACHE
	#if defined(WL_ENAB_RUNTIME_CHECK) || !defined(DONGLEBUILD)
		#define SCANCACHE_SUPPORT(pub)	((pub)->_scancache_support)
	#elif defined(WLSCANCACHE_DISABLED)
		#define SCANCACHE_SUPPORT(pub)	(0)
	#else
		#define SCANCACHE_SUPPORT(pub)	(1)
	#endif
#else
	#define SCANCACHE_SUPPORT(pub)		(0)
#endif /* WLSCANCACHE */

#ifdef WLFMC
	#if defined(WL_ENAB_RUNTIME_CHECK) || !defined(DONGLEBUILD)
		#define WLFMC_ENAB(pub) ((pub)->_fmc)
	#elif defined(WLFMC_DISABLED)
		#define WLFMC_ENAB(pub) (0)
	#else
		#define WLFMC_ENAB(pub) (1)
	#endif
#else
	#define WLFMC_ENAB(pub) 0
#endif /* WLFMC */

#ifdef WLRCC
	#if defined(WL_ENAB_RUNTIME_CHECK) || !defined(DONGLEBUILD)
		#define WLRCC_ENAB(pub) ((pub)->_rcc)
	#elif defined(WLRCC_DISABLED)
		#define WLRCC_ENAB(pub) (0)
	#else
		#define WLRCC_ENAB(pub) (1)
	#endif
#else
	#define WLRCC_ENAB(pub) 0
#endif /* WLRCC */

/* Adaptive Beacon Timeout */
#ifdef WLABT
	#if defined(WL_ENAB_RUNTIME_CHECK) || !defined(DONGLEBUILD)
		#define WLABT_ENAB(pub) ((pub)->_abt)
	#elif defined(WLABT_DISABLED)
		#define WLABT_ENAB(pub) (0)
	#else
		#define WLABT_ENAB(pub) (1)
	#endif
#else
	#define WLABT_ENAB(pub) 0
#endif /* WLABT */

/* Advanced IBSS Support */
#ifdef WLAIBSS
	#if defined(WL_ENAB_RUNTIME_CHECK)
		#define AIBSS_ENAB(pub)		((pub)->_aibss)
	#elif defined(WLAIBSS_DISABLED)
		#define AIBSS_ENAB(pub)		(0)
	#else
		#define AIBSS_ENAB(pub)		((pub)->_aibss)
	#endif
#else
	#define AIBSS_ENAB(pub)			(0)
#endif /* WLAIBSS */

/* Multi-hop IP forwarding offload support */
#ifdef WLIPFO
	#if defined(WL_ENAB_RUNTIME_CHECK)
		#define IPFO_ENAB(pub)		((pub)->_ipfo)
	#elif defined(WLIPFO_DISABLED)
		#define IPFO_ENAB(pub)		(0)
	#else
		#define IPFO_ENAB(pub)		((pub)->_ipfo)
	#endif
#else
	#define IPFO_ENAB(pub)			(0)
#endif /* WLIPFO */

#ifdef WL_FRWD_REORDER
	#if defined(WL_ENAB_RUNTIME_CHECK)
		#define FRWD_REORDER_ENAB(pub)		((pub)->_frwd_reorder)
	#elif defined(WL_FRWD_REORDER_DISABLED)
		#define FRWD_REORDER_ENAB(pub)		(0)
	#else
		#define FRWD_REORDER_ENAB(pub)		(1)
	#endif
#else
	#define FRWD_REORDER_ENAB(pub)			(0)
#endif /* WL_FRWD_REORDER */

#if defined(WLAMPDU_HOSTREORDER)
	#if defined(WL_ENAB_RUNTIME_CHECK)
		#define AMPDU_HOST_REORDER_ENAB(pub)		(pub->_ampdu_hostreorder)
	#elif defined(AMPDU_HOSTREORDER_DISABLED)
		#define AMPDU_HOST_REORDER_ENAB(pub)		FALSE
	#else
		#define AMPDU_HOST_REORDER_ENAB(pub)		(pub->_ampdu_hostreorder)
	#endif
#else
	#define AMPDU_HOST_REORDER_ENAB(pub)			FALSE
#endif /* WLAMPDU_HOSTREORDER */

#ifdef WLAMSDU_TX
	#if defined(WL_ENAB_RUNTIME_CHECK)
		#define AMSDU_TX_SUPPORT(pub)	((pub)->_amsdu_tx_support)
		#define AMSDU_TX_ENAB(pub)	((pub)->_amsdu_tx)
	#elif defined(WLAMSDU_TX_DISABLED)
		#define AMSDU_TX_SUPPORT(pub)	(0)
		#define AMSDU_TX_ENAB(pub)	(0)
	#else
		#define AMSDU_TX_SUPPORT(pub)	(1)
		#define AMSDU_TX_ENAB(pub)	((pub)->_amsdu_tx)
	#endif

	#ifdef DISABLE_AMSDUTX_FOR_VI
		#define	AMSDU_TX_AC_ENAB(ami, tid)\
			(wlc_amsdu_chk_priority_enable((ami), (tid)))
	#else
		#define AMSDU_TX_AC_ENAB(ami, tid)	(TRUE)
	#endif
#else
	#define AMSDU_TX_SUPPORT(pub)	(0)
	#define AMSDU_TX_ENAB(pub)	(0)
#endif /* WLAMSDU_TX */

#ifdef WLNFC
	#if defined(WL_ENAB_RUNTIME_CHECK) || !defined(DONGLEBUILD)
		#define NFC_ENAB(pub) ((pub)->_nfc)
	#elif defined(WLNFC_DISABLED)
		#define NFC_ENAB(pub)	(0)
	#else
		#define NFC_ENAB(pub) ((pub)->_nfc)
	#endif
#else
	#define NFC_ENAB(pub) (0)
#endif /* WLNFC */

/* PM2 Receive Throttle Duty Cycle */
#if defined(WL_PM2_RCV_DUR_LIMIT)
#define WLC_PM2_RCV_DUR_MIN		(10)	/* 10% of beacon interval */
#define WLC_PM2_RCV_DUR_MAX		(80)	/* 80% of beacon interval */
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
#endif

extern void wlc_pkttag_info_move(wlc_info_t *wlc, void *pkt_from, void *pkt_to);

#define	WLC_PREC_COUNT		16 /* Max precedence level implemented */

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

extern const uint8 wme_fifo2ac[];
#define WME_PRIO2AC(zprio)	wme_fifo2ac[prio2fifo[(zprio)]]

extern const uint8 fifo2prio[];

#endif /* LINUX_POSTMOGRIFY_REMOVAL */

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
#define WLC_AUTOCOUNTRY_ENAB(wlc) ((wlc)->pub->_autocountry)
#else
#define WLC_AUTOCOUNTRY_ENAB(wlc) FALSE
#endif

/* Regulatory Domain -- 11D Support */
#ifdef WL11D
#define WL11D_ENAB(wlc)	((wlc)->pub->_11d)
#else
#define WL11D_ENAB(wlc)	((void)(wlc), FALSE)
#endif

/* Spectrum Management -- 11H Support */
#ifdef WL11H
#define WL11H_ENAB(wlc)	((wlc)->pub->_11h)
#else
#define WL11H_ENAB(wlc)	((void)(wlc), FALSE)
#endif

/* SLAVE RADAR Support */
#if defined(BAND5G) && defined(SLAVE_RADAR)
#define WL11H_STA_ENAB(wlc)	(BSSCFG_STA((wlc)->cfg) && WL11H_ENAB(wlc))
#else
#define WL11H_STA_ENAB(wlc)	0
#endif /* BAND5G && SLAVE_RADAR */

/* Interworking -- 11u Support */
#ifdef WL11U
#define WL11U_ENAB(wlc)	((wlc)->pub->_11u)
#else
#define WL11U_ENAB(wlc)	FALSE
#endif

/* SW probe response Support */
#ifdef WLPROBRESP_SW
#define WLPROBRESP_SW_ENAB(wlc)	((wlc)->pub->_probresp_sw)
#else
#define WLPROBRESP_SW_ENAB(wlc)	FALSE
#endif

/* Link Power Control Support */
#ifdef WL_LPC
#define LPC_ENAB(wlc)	((wlc)->pub->_lpc_algo)
#else
#define LPC_ENAB(wlc)	(FALSE)
#endif

/* Reliable Multicast Support */
#ifdef WL_RELMCAST
#if defined(WL_ENAB_RUNTIME_CHECK)
#define RMC_SUPPORT(pub)	((pub)->_rmc_support)
#define RMC_ENAB(pub)		((pub)->_rmc)
#elif defined(WL_RELMCAST_DISABLED)
#define RMC_SUPPORT(pub)	(0)
#define RMC_ENAB(pub)		(0)
#else
#define RMC_SUPPORT(pub)	(1)
#define RMC_ENAB(pub)		((pub)->_rmc)
#endif /* WL_ENAB_RUNTIME_CHECK */
#else
#define RMC_SUPPORT(pub)		(0)
#define RMC_ENAB(pub)			(0)
#endif /* WL_RELMCAST */

#ifdef WL_STATS
	#if defined(WL_ENAB_RUNTIME_CHECK)
		#define WL_STATS_ENAB(pub)	((pub)->_wlstats)
	#elif defined(WL_STATS_DISABLED)
		#define WL_STATS_ENAB(pub)	(0)
	#else
		#define WL_STATS_ENAB(pub)	(1)
	#endif
#else
	#define WL_STATS_ENAB(pub)		(0)
#endif /* WL_STATS */

/* PM single core beacon rx
 * Allow MIMO chips to save power by enabling
 * only RX core while receiving beacons
 * To use this feature, need to add support in ucode
 * first before enabling it in driver
 */
#ifdef WLPM_BCNRX
	#define PM_BCNRX_ENAB(pub)		((pub)->_pm_bcnrx)
#else
	#define PM_BCNRX_ENAB(pub)		(0)
#endif /* WLPM_BCNRX */

/* power optimization */
#ifdef WLSCAN_PS
#if defined(WL_ENAB_RUNTIME_CHECK)
	#define WLSCAN_PS_ENAB(pub)   ((pub)->_scan_ps)
#elif defined(WLSCAN_PS_DISABLED)
	#define WLSCAN_PS_ENAB(pub)   (0)
#else
	#define WLSCAN_PS_ENAB(pub)   (1)
#endif
#else
	#define WLSCAN_PS_ENAB(pub)   (0)
#endif /* WLSCAN_PS */

/* hotspot OSEN */
#ifdef WLOSEN
#if defined(WL_ENAB_RUNTIME_CHECK)
	#define WLOSEN_ENAB(pub)   ((pub)->_osen)
#elif defined(WLOSEN_DISABLED)
	#define WLOSEN_ENAB(pub)   (0)
#else
	#define WLOSEN_ENAB(pub)   (1)
#endif
#else
	#define WLOSEN_ENAB(pub)   (0)

#endif	/* WLOSEN */

#define WLC_USE_COREFLAGS	0xffffffff	/* invalid core flags, use the saved coreflags */

#ifdef WLCNT
#define WLC_UPDATE_STATS(wlc)	1	/* Stats support */
#define WLCNTINCR(a)		((a)++)	/* Increment by 1 */
#define WLCNTDECR(a)		((a)--)	/* Decrement by 1 */
#define WLCNTADD(a,delta)	((a) += (delta)) /* Increment by specified value */
#define WLCNTSET(a,value)	((a) = (value)) /* Set to specific value */
#define WLCNTVAL(a)		(a)	/* Return value */
#define WLCNTINCR_MIN(a)        ((a)++) /* Increment by 1 */
#define WLCNTCONDINCR(c, a)	do { if (c) (a)++; } while (0)	/* Conditionally incr by 1 */
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
#define WLPWRSAVETXFINCR(wlc)
#define WLPWRSAVERXFVAL(wlc)	0
#define WLPWRSAVETXFVAL(wlc)	0
#endif

#ifdef WDS
#define WLWDS_CAP(wlc)        ((wlc)->mwds != NULL)
#else
#define WLWDS_CAP(wlc)        0
#endif /* WDS */

/* dpc info */
struct wlc_dpc_info {
	uint processed;
};

/* BT WLAN Tunnel Engine */
#ifdef WL_BWTE
	#if defined(WL_ENAB_RUNTIME_CHECK)
		#define BWTE_ENAB(pub)   ((pub)->_bwte)
	#elif defined(WL_BWTE_DISABLED)
		#define BWTE_ENAB(pub)   (0)
	#else
		#define BWTE_ENAB(pub)   (1)
	#endif
#else
	#define BWTE_ENAB(pub)      (0)
#endif /* WL_BWTE */

/* Tunnel BT Over Wlan */
#ifdef WL_TBOW
	#if defined(WL_ENAB_RUNTIME_CHECK)
		#define TBOW_ENAB(pub)   ((pub)->_tbow)
	#elif defined(WL_TBOW_DISABLED)
		#define TBOW_ENAB(pub)   (0)
	#else
		#define TBOW_ENAB(pub)   (1)
	#endif
#else
	#define TBOW_ENAB(pub)      (0)
#endif /* WL_TBOW */

/* common functions for every port */
extern void *wlc_attach(void *wl, uint16 vendor, uint16 device, uint unit, uint iomode,
	osl_t *osh, void *regsva, uint bustype, void *btparam, void *objr, uint *perr);
extern uint wlc_detach(struct wlc_info *wlc);
extern int  wlc_up(struct wlc_info *wlc);
extern uint wlc_down(struct wlc_info *wlc);
#ifdef WLC_HIGH_ONLY
extern int  wlc_sleep(struct wlc_info *wlc);
extern int  wlc_resume(struct wlc_info *wlc);
#endif

extern int wlc_set(struct wlc_info *wlc, int cmd, int arg);
extern int wlc_get(struct wlc_info *wlc, int cmd, int *arg);
extern int wlc_iovar_getint(struct wlc_info *wlc, const char *name, int *arg);
extern int wlc_iovar_setint(struct wlc_info *wlc, const char *name, int arg);
extern bool wlc_chipmatch(uint16 vendor, uint16 device);
extern void wlc_init(struct wlc_info *wlc);
extern void wlc_reset(struct wlc_info *wlc);
extern int wlc_getrand(struct wlc_info *wlc, uint8 *buf, int len);

#ifdef MCAST_REGEN
extern int32 wlc_mcast_reverse_translation(struct ether_header *eh);
#endif /* MCAST_REGEN */

extern void wlc_intrson(struct wlc_info *wlc);
extern uint32 wlc_intrsoff(struct wlc_info *wlc);
extern void wlc_intrs_deassert(wlc_info_t *wlc);
extern void wlc_intrsrestore(struct wlc_info *wlc, uint32 macintmask);
extern bool wlc_intrsupd(struct wlc_info *wlc);
extern bool wlc_isr(struct wlc_info *wlc, bool *wantdpc);
extern bool wlc_dpc(struct wlc_info *wlc, bool bounded, struct wlc_dpc_info *dpc);

extern bool wlc_sendpkt(struct wlc_info *wlc, void *sdu, struct wlc_if *wlcif);
extern bool wlc_send80211_specified(wlc_info_t *wlc, void *sdu, uint32 rspec, struct wlc_if *wlcif);
extern int wlc_iovar_op(struct wlc_info *wlc, const char *name, void *params, int p_len, void *arg,
	int len, bool set, struct wlc_if *wlcif);
extern int wlc_ioctl(struct wlc_info *wlc, int cmd, void *arg, int len, struct wlc_if *wlcif);
/* helper functions */
extern void wlc_statsupd(struct wlc_info *wlc);
#ifdef WLCNT
extern int wlc_get_all_cnts(wlc_info_t *wlc, void *buf, int buflen);
#endif /* WLCNT */

extern wlc_pub_t *wlc_pub(void *wlc);
extern void wlc_get_override_vendor_dev_id(void *wlc, uint16 *vendorid, uint16 *devid);

#if defined(BCM_OL_DEV) || defined(WLOFFLD)
/* SHM access synchronization functions as per Peterson's algorithm */
extern void tcm_sem_enter(wlc_info_t *wlc);
extern void tcm_sem_exit(wlc_info_t *wlc);
extern void tcm_sem_cleanup(wlc_info_t *wlc);
#endif /* BCM_OL_DEV || WLOFFLD */

#ifndef LINUX_POSTMOGRIFY_REMOVAL
/* common functions for every port */
extern int wlc_bmac_up_prep(struct wlc_hw_info *wlc_hw);
extern int wlc_bmac_up_finish(struct wlc_hw_info *wlc_hw);
extern int wlc_bmac_set_ctrl_ePA(struct wlc_hw_info *wlc_hw);
extern int wlc_bmac_set_ctrl_SROM(struct wlc_hw_info *wlc_hw);
extern int wlc_bmac_down_prep(struct wlc_hw_info *wlc_hw);
extern int wlc_bmac_down_finish(struct wlc_hw_info *wlc_hw);
extern int wlc_bmac_set_ctrl_bt_shd0(struct wlc_hw_info *wlc_hw, bool enable);
extern int wlc_bmac_set_btswitch(struct wlc_hw_info *wlc_hw, int8 state);
extern int wlc_bmac_4331_epa_init(struct wlc_hw_info *wlc_hw);

extern int wlc_nin_ioctl(struct wlc_info *wlc, int cmd, void *arg, int len, struct wlc_if *wlcif);
extern bool wlc_nin_process_sendup(struct wlc_info *wlc, void * p);
void wlc_nin_create_iapp_ind(struct wlc_info *wlc, void *p, int len);
void
wlc_nin_sendup(wlc_info_t *wlc, void *pkt, struct wlc_if *wlcif);

extern uint32 wlc_reg_read(struct wlc_info *wlc, void *r, uint size);
extern void wlc_reg_write(struct wlc_info *wlc, void *r, uint32 v, uint size);
extern void wlc_corereset(struct wlc_info *wlc, uint32 flags);
extern void wlc_mhf(struct wlc_info *wlc, uint8 idx, uint16 mask, uint16 val, int bands);
extern uint16 wlc_mhf_get(struct wlc_info *wlc, uint8 idx, int bands);
extern uint wlc_ctrupd(wlc_info_t *wlc, uint macstat_offset);
extern uint32 wlc_delta_txfunfl(struct wlc_info *wlc, int fifo);
extern void wlc_rate_lookup_init(struct wlc_info *wlc, wlc_rateset_t *rateset);
extern void wlc_default_rateset(struct wlc_info *wlc, wlc_rateset_t *rs);
#ifdef STA
extern void wlc_join_bss_prep(struct wlc_bsscfg *cfg);
extern void wlc_join_BSS(struct wlc_bsscfg *cfg, wlc_bss_info_t* bi);
#endif /* STA */
extern chanspec_t wlc_get_home_chanspec(struct wlc_bsscfg *cfg);
extern wlc_bss_info_t *wlc_get_current_bss(struct wlc_bsscfg *cfg);
#if defined(STA) && defined(DBG_BCN_LOSS)
extern int wlc_get_bcn_dbg_info(struct wlc_bsscfg *cfg, struct ether_addr *addr,
	struct wlc_scb_dbg_bcn *dbg_bcn);
#endif

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

/* ioctl */
extern int wlc_iovar_getint8(struct wlc_info *wlc, const char *name, int8 *arg);
extern int wlc_iovar_getbool(struct wlc_info *wlc, const char *name, bool *arg);
extern int wlc_iovar_check(wlc_info_t *wlc, const bcm_iovar_t *vi, void *arg, int len, bool set,
	wlc_if_t *wlcif);
#endif /* LINUX_POSTMOGRIFY_REMOVAL */

#if defined(WLC_PATCH_IOCTL)
	/* By default, IOVAR patch handling is disabled. These macros are overriden by the
	 * generated IOVAR patch handler file if patching is enabled.
	 */
	#define ROM_AUTO_IOCTL_PATCH_IOVARS	NULL
	#define ROM_AUTO_IOCTL_PATCH_DOIOVAR	NULL

	/* 'wlc_module_register_ex()' supports IOVAR patch handlers. Callers continue to use the
	 * legacy API 'wlc_module_register()' to register modules.
	 */
	#define WLC_MODULE_REGISTER		wlc_module_register_ex

	/* CSTYLED */
	#define WLC_MODULE_REG_PATCH_ARGS_DECL	, const bcm_iovar_t *patch_iovars, \
	                                          iovar_fn_t patch_iovar_fn

	#define wlc_module_register(pub, iovars, name, hdl, iovar_fn, watchdog_fn, up, down) \
		wlc_module_register_ex(pub, iovars, name, hdl, iovar_fn, watchdog_fn, up, down, \
				       ROM_AUTO_IOCTL_PATCH_IOVARS, ROM_AUTO_IOCTL_PATCH_DOIOVAR)

#else  /* !WLC_PATCH_IOCTL */
	#define WLC_MODULE_REGISTER		wlc_module_register
	#define WLC_MODULE_REG_PATCH_ARGS_DECL
#endif /* WLC_PATCH_IOCTL */


extern int WLC_MODULE_REGISTER(wlc_pub_t *pub, const bcm_iovar_t *iovars,
                               const char *name, void *hdl, iovar_fn_t iovar_fn,
                               watchdog_fn_t watchdog_fn, up_fn_t up_fn, down_fn_t down_fn
			       WLC_MODULE_REG_PATCH_ARGS_DECL);


extern int wlc_module_find(wlc_pub_t *pub, const char *name);
extern int wlc_module_unregister(wlc_pub_t *pub, const char *name, void *hdl);
extern int wlc_module_add_ioctl_fn(wlc_pub_t *pub, void *hdl,
                                   wlc_ioctl_fn_t ioctl_fn,
                                   int num_cmds, const wlc_ioctl_cmd_t *ioctls);
extern int wlc_module_remove_ioctl_fn(wlc_pub_t *pub, void *hdl);

#ifndef LINUX_POSTMOGRIFY_REMOVAL
#if defined(BCMDBG) || defined(BCMDBG_DUMP)
extern int wlc_iovar_dump(struct wlc_info *wlc, const char *params, int p_len,
                          char *out_buf, int out_len);
#endif /* BCMDBG */
extern int wlc_dump_register(wlc_pub_t *pub, const char *name, dump_fn_t dump_fn,
                             void *dump_fn_arg);

extern uint wlc_txpktcnt(struct wlc_info *wlc);

extern void wlc_event_if(struct wlc_info *wlc, struct wlc_bsscfg *cfg, wlc_event_t *e,
	const struct ether_addr *addr);
extern void wlc_suspend_mac_and_wait(struct wlc_info *wlc);
extern void wlc_enable_mac(struct wlc_info *wlc);
extern uint16 wlc_rate_shm_offset(struct wlc_info *wlc, uint8 rate);
extern uint32 wlc_get_rspec_history(struct wlc_bsscfg *cfg);
extern uint32 wlc_get_current_highest_rate(struct wlc_bsscfg *cfg);

extern int wlc_get_last_txpwr(wlc_info_t *wlc, wlc_txchain_pwr_t *last_pwr);


static INLINE int wlc_iovar_getuint(struct wlc_info *wlc, const char *name, uint *arg)
{
	return wlc_iovar_getint(wlc, name, (int*)arg);
}

static INLINE int wlc_iovar_getuint8(struct wlc_info *wlc, const char *name, uint8 *arg)
{
	return wlc_iovar_getint8(wlc, name, (int8*)arg);
}

static INLINE int wlc_iovar_setuint(struct wlc_info *wlc, const char *name, uint arg)
{
	return wlc_iovar_setint(wlc, name, (int)arg);
}

#ifdef WLC_HIGH
/* BMAC_NOTE: Hack away iovar helper functions until we know what to do with iovar support */
#if defined(BCMDBG) || defined(WLTEST) || defined(BCMDBG_ERR) || defined(BCMDBG_DUMP) \
	|| defined(DBG_PHY_IOV) || defined(BCMQT)
extern int wlc_iocregchk(struct wlc_info *wlc, uint band);
#endif 
#if defined(BCMDBG) || defined(WLTEST) || defined(STA)
extern int wlc_iocbandchk(struct wlc_info *wlc, int *arg, int len, uint *bands, bool clkchk);
#endif
#if defined(BCMDBG) || defined(BCMDBG_DUMP)
extern int wlc_iocpichk(struct wlc_info *wlc, uint phytype);
#endif 

#else /* !WLC_HIGH */
#define wlc_iocregchk(w, b)	(0)
#define wlc_iocbandchk(w, a, l, b, c)	(0)
#define wlc_iocpichk(w, p)	(0)
#endif /* WLC_HIGH */

/* helper functions */
struct scb;
extern void wlc_ps_on(struct wlc_info *wlc, struct scb *scb);
extern void wlc_ps_off(struct wlc_info *wlc, struct scb *scb, bool discard);
extern bool wlc_radio_monitor_stop(struct wlc_info *wlc);

#if defined(WLTINYDUMP) || defined(BCMDBG) || defined(WLMSG_ASSOC) || \
	defined(WLMSG_PRPKT) || defined(WLMSG_OID) || defined(BCMDBG_DUMP) || \
	defined(WLMSG_INFORM) || defined(WLMSG_WSEC) || defined(WLEXTLOG) || defined(WLTEST) || \
	defined(BCMDBG_MU)
extern int wlc_format_ssid(char* buf, const uchar ssid[], uint ssid_len);
#endif

#ifdef STA
#ifdef BCMSUP_PSK
extern bool wlc_sup_mic_error(struct wlc_bsscfg *cfg, bool group);
#endif /* BCMSUP_PSK */
extern void wlc_pmkid_build_cand_list(struct wlc_bsscfg *cfg, bool check_SSID);
#endif /* STA */

#define	MAXBANDS		2	/* Maximum #of bands */
/* bandstate array indices */
#define BAND_2G_INDEX		0	/* wlc->bandstate[x] index */
#define BAND_5G_INDEX		1	/* wlc->bandstate[x] index */

#define BAND_2G_NAME		"2.4G"
#define BAND_5G_NAME		"5G"

#if defined(BCMSDIO) || defined(WLC_HIGH_ONLY)
void wlc_device_removed(void *arg);
#endif
#endif /* LINUX_POSTMOGRIFY_REMOVAL */

/* BMAC RPC: 7 uint32 params: pkttotlen, fifo, commit, fid, txpktpend, pktflag, rpc_id */
#define WLC_RPCTX_PARAMS        32


/* per interface stats counters */
extern void wlc_wlcif_stats_get(wlc_info_t *wlc, wlc_if_t *wlcif, wl_if_stats_t *wlif_stats);
extern wlc_if_t *wlc_wlcif_get_by_index(wlc_info_t *wlc, uint idx);

#if defined(BCMDBG)
/* Performance statistics interfaces */
/* Mask for individual stats */
#define WLC_PERF_STATS_ISR			0x01
#define WLC_PERF_STATS_DPC			0x02
#define WLC_PERF_STATS_TMR_DPC		0x04
#define WLC_PERF_STATS_PRB_REQ		0x08
#define WLC_PERF_STATS_PRB_RESP		0x10
#define WLC_PERF_STATS_BCN_ISR		0x20
#define WLC_PERF_STATS_BCNS			0x40

void wlc_update_perf_stats(wlc_info_t *wlc, uint32 mask);
void wlc_update_isr_stats(wlc_info_t *wlc, uint32 macintstatus);
#endif /* BCMDBG */

/* value for # replay counters currently supported */
#if defined(WOWL) || defined(BCM_OL_DEV)
#define WLC_REPLAY_CNTRS_VALUE	WPA_CAP_4_REPLAY_CNTRS
#else
#define WLC_REPLAY_CNTRS_VALUE	WPA_CAP_16_REPLAY_CNTRS
#endif


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

#define GPIO_2_PA_CTRL_5G_0		0x4 /* bit mask of 2nd pin */

#if defined(CONFIG_WL) || defined(CONFIG_WL_MODULE)
#define WL_RTR()	TRUE
#else
#define WL_RTR()	FALSE
#endif /* CONFIG_WL */

#if defined(SRHWVSDB)
	#if defined(WL_ENAB_RUNTIME_CHECK) || !defined(DONGLEBUILD)
		/* rom builds & high/nic driver builds */
		#define SRHWVSDB_ENAB(pub) ((pub)->_wlsrvsdb)
	#elif defined(SRHWVSDB_DISABLED)
		/* rom offload build, feature disabled */
		#define SRHWVSDB_ENAB(pub) (0)
	#else
		/* rom offload build, feature enabled */
		#define SRHWVSDB_ENAB(pub) (1)
	#endif /* SRHWVSDB_DISABLED */
#else
	#define SRHWVSDB_ENAB(pub) (0)
#endif /* SRHWVSDB */

#if defined(WLPROPRIETARY_11N_RATES)
	#if defined(WL_ENAB_RUNTIME_CHECK) || !defined(DONGLEBUILD)
		/* dongle builds OR rom builds */
		#define WLPROPRIETARY_11N_RATES_ENAB(pub) ((pub)->_ht_prop_rates_capable)
	#elif defined(WLPROPRIETARY_11N_RATES_DISABLED)
		/* rom offload build, feature disabled */
		#define WLPROPRIETARY_11N_RATES_ENAB(pub) (0)
	#else
		/* rom offload build, feature enabled */

		/* 4349 family wants the feature to be enabled
		 * selectively for 2.4G 20Mhz chanspecs
		 */
		#define WLPROPRIETARY_11N_RATES_ENAB(pub) \
			(!BCM4349_CHIP((pub)->sih->chip) || (pub)->_ht_prop_rates_capable)
	#endif /* WLPROPRIETARY_11N_RATES */
#else
	#define WLPROPRIETARY_11N_RATES_ENAB(pub) (0)
#endif /* WLPROPRIETARY_11N_RATES */

#ifdef BCMWAPI_WPI
#define WAPI_HW_WPI_ENAB(_pub) ((_pub)->_wapi_hw_wpi)
#else
#define WAPI_HW_WPI_ENAB(_pub) FALSE
#endif /* BCMWAPI_WPI */

/* Convert restricted channel on receiving beacon or probe response */
#define CLEAR_RESTRICTED_CHANNEL_ENAB(pub) ((pub)->_clear_restricted_chan)

/* Add BSS info to WLC event data */
#if defined(WL_EVDATA_BSSINFO)
	#if defined(WL_ENAB_RUNTIME_CHECK) || !defined(DONGLEBUILD)
		#define EVDATA_BSSINFO_ENAB(pub) ((pub)->_evdata_bssinfo)
	#elif defined(WL_EVDATA_BSSINFO_DISABLED)
		#define EVDATA_BSSINFO_ENAB(pub) (0)
	#else
		#define EVDATA_BSSINFO_ENAB(pub) (1)
	#endif
#else
	#define EVDATA_BSSINFO_ENAB(pub) 0
#endif /* WL_EVDATA_BSSINFO */

/* Power Stats */
#ifdef WL_PWRSTATS
	#if defined(WL_ENAB_RUNTIME_CHECK)
		#define PWRSTATS_ENAB(pub)		((pub)->_pwrstats)
	#elif defined(WL_PWRSTATS_DISABLED)
		#define PWRSTATS_ENAB(pub)		(0)
	#else
		#define PWRSTATS_ENAB(pub)		(1)
	#endif
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
#endif /* SCB_BS_DATA not defined */

#ifdef WLDURATION
#define WLDURATION_ENTER(ww, idx) do { wlc_duration_enter((ww), (idx)); } while (0)
#define WLDURATION_EXIT(ww, idx)  do { wlc_duration_exit((ww), (idx)); } while (0)
#else
#define WLDURATION_ENTER(ww, idx)
#define WLDURATION_EXIT(ww, idx)
#endif

/* OBSS Protection Support */
#ifdef WL_PROT_OBSS
	#if defined(WL_ENAB_RUNTIME_CHECK) || !defined(DONGLEBUILD)
		#define WLC_PROT_OBSS_ENAB(pub) ((pub)->_prot_obss)
	#elif defined(WL_PROT_OBSS_DISABLED)
		#define WLC_PROT_OBSS_ENAB(pub) (0)
	#else
		#define WLC_PROT_OBSS_ENAB(pub) ((pub)->_prot_obss)
	#endif
#else
	#define WLC_PROT_OBSS_ENAB(pub) (0)
#endif /* WL_PROT_OBSS */

/* DYNBW  Support */
#ifdef WL_OBSS_DYNBW
	#if defined(WL_ENAB_RUNTIME_CHECK) || !defined(DONGLEBUILD)
		#define WLC_OBSS_DYNBW_ENAB(pub) ((pub)->_obss_dynbw)
	#elif defined(WL_OBSS_DYNBW_DISABLED)
		#define WLC_OBSS_DYNBW_ENAB(pub) (0)
	#else
		#define WLC_OBSS_DYNBW_ENAB(pub) ((pub)->_obss_dynbw)
	#endif
#else
	#define WLC_OBSS_DYNBW_ENAB(pub) (0)
#endif /* WL_OBSS_DYNBW */

/* mode switch Support */
#ifdef WL_MODESW
	#if defined(WL_ENAB_RUNTIME_CHECK) || !defined(DONGLEBUILD)
		#define WLC_MODESW_ENAB(pub) ((pub)->_modesw)
	#elif defined(WL_MODESW_DISABLED)
		#define WLC_MODESW_ENAB(pub) (0)
	#else
		#define WLC_MODESW_ENAB(pub) ((pub)->_modesw)
	#endif
#else
	#define WLC_MODESW_ENAB(pub) (0)
#endif /* WL_MODESW */

#if defined(WLBSSLOAD_REPORT)
	#if defined(WL_ENAB_RUNTIME_CHECK) || !defined(DONGLEBUILD)
		#define BSSLOAD_REPORT_ENAB(pub) ((pub)->_bssload_report)
	#elif defined(WLBSSLOAD_REPORT_DISABLED)
		#define BSSLOAD_REPORT_ENAB(pub) (0)
	#else
		#define BSSLOAD_REPORT_ENAB(pub) (1)
	#endif
#else
	#define BSSLOAD_REPORT_ENAB(pub) 0
#endif /* defined(WLBSSLOAD_REPORT) */

/* Macro to check if AIBSS PS modes enabled */
#if defined(WLAIBSS)
#define AIBSS_PS_ENAB(pub) ((pub)->_aibss_ps)
#define AIBSS_BSS_PS_ENAB(cfg) ((cfg)->current_bss->atim_window != 0)
#else
#define AIBSS_PS_ENAB(pub) (0)
#define AIBSS_BSS_PS_ENAB(cfg) (0)
#endif /* WLIBSSPS */

/* Beacon Trim Support */
#ifdef WL_BCNTRIM
	#if defined(WL_ENAB_RUNTIME_CHECK) || !defined(DONGLEBUILD)
		#define WLC_BCNTRIM_ENAB(pub) ((pub)->_bcntrim)
	#elif defined(WL_BCNTRIM_DISABLED)
		#define WLC_BCNTRIM_ENAB(pub) (0)
	#else
		#define WLC_BCNTRIM_ENAB(pub) ((pub)->_bcntrim)
	#endif
#else
	#define WLC_BCNTRIM_ENAB(pub) (0)
#endif /* WL_BCNTRIM */

extern void wlc_devpwrstchg_change(wlc_info_t *wlc, bool hostmem_access_enabled);

/* Latency Tolerance Reporting */
#if defined(WL_LTR)
	#if defined(WL_ENAB_RUNTIME_CHECK) || !defined(DONGLEBUILD)
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
	#endif /* WL_LTR_DISABLED */
#else
	#define LTR_SUPPORT(pub)	(0)
	#define LTR_ENAB(pub)		(0)
#endif /* WL_LTR */

/* AUXPMQ check on d11 rev. For x, use pub in high driver, wlc_hw in low */
#ifdef WL_AUXPMQ
#define AUXPMQ_ENAB(x)	D11REV_GE((x)->corerev, 64)
#else
#define AUXPMQ_ENAB(x)	(0)
#endif

extern void wlc_generate_pme_to_host(wlc_info_t *wlc, bool pme_on);

/* IO mode */
#define IOMODE_TYPE_PIO		1
#define IOMODE_TYPE_CTDMA	2

/* Ultra-Low Bandwidth (ULB) Mode support */
#ifdef WL11ULB
	#if defined(WL_ENAB_RUNTIME_CHECK)
		#define ULB_ENAB(pub)		((pub)->_ulb)
	#elif defined(WL11ULB_DISABLED)
		#define ULB_ENAB(pub)		(0)
	#else
		#define ULB_ENAB(pub)		((pub)->_ulb)
	#endif
#else
	#define ULB_ENAB(pub)			(0)
#endif /* WL11ULB */

/* TAF Mode support */
#ifdef WLTAF
	#if defined(WL_ENAB_RUNTIME_CHECK)
		#define WLTAF_ENAB(pub)		((pub)->_taf)
	#elif defined(WLTAF_DISABLED)
		#define WLTAF_ENAB(pub)		(0)
	#else
		#define WLTAF_ENAB(pub)		((pub)->_taf)
	#endif
#else
	#define WLTAF_ENAB(pub)			(0)
#endif /* WLTAF */

/* 160MHz mode is available only for 4365/4366 corerev 0x41 (c0) */
#define WL_HAS_DYN160(wlc)	D11REV_IS(wlc->pub->corerev, 0x41)

#endif /* _wlc_pub_h_ */
