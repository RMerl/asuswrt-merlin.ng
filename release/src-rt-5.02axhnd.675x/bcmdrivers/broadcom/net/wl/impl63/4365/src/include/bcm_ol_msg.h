/*
 * Broadcom 802.11 Message infra (pcie<-> CR4) used for RX offloads
 *
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
 * $Id: bcm_ol_msg.h chandrum $
 */

#ifndef _BCM_OL_MSG_H_
#define _BCM_OL_MSG_H_

#include <epivers.h>
#include <typedefs.h>
#include <wlc_utils.h>
#ifdef BCM_OL_DEV
#include <proto/ethernet.h>
#include <bcmcrypto/tkhash.h>
#include <proto/802.11.h>
#endif // endif
#include <proto/eapol.h>
#include <bcmcrypto/tkhash.h>
#include <bcmdevs.h>
#include <proto/bcmip.h>
#include <proto/bcmipv6.h>
#include <proto/802.1d.h>
#include <wlc_phy_hal.h>
#include <bcmwpa.h>
#include <wlc_key.h>

#include <wlc_keymgmt.h>

#define OLMSG_RW_MAX_ENTRIES		2

/* Dongle indecies */
#define OLMSG_READ_DONGLE_INDEX		0
#define OLMSG_WRITE_DONGLE_INDEX	1

/* Host indecies */
#define OLMSG_READ_HOST_INDEX		1
#define OLMSG_WRITE_HOST_INDEX		0
#define OLMSG_BUF_SZ			0x10000 /* 64k */
#define OLMSG_HOST_BUF_SZ		0x7800 /* 30k */
#define OLMSG_DGL_BUF_SZ		0x7800 /* 30k */

/* Maximum IE id for non vendor specific IE */
#define OLMSG_BCN_MAX_IE		222
#define MAX_VNDR_IE			50 /* Needs to be looked into */
#define MAX_IE_LENGTH_BUF		2048
#define MAX_STAT_ENTRIES		16
#define NEXT_STAT(x)			((x + 1) & ((MAX_STAT_ENTRIES) - 1))
#define IEMASK_SZ			CEIL((OLMSG_BCN_MAX_IE+1), 8)

#define MARKER_BEGIN			0xA5A5A5A5
#define MARKER_END			~MARKER_BEGIN

#define MAX_OL_EVENTS			16
#define	MAX_OL_SCAN_CONFIG		9
#define	MAX_OL_SCAN_BSS			5

#define RSSI_THRESHOLD_2G_DEF		-80
#define RSSI_THRESHOLD_5G_DEF		-75

/* Events among various offload modules */
enum {
	BCM_OL_E_WOWL_START,
	BCM_OL_E_WOWL_COMPLETE,
	BCM_OL_E_TIME_SINCE_BCN,
	BCM_OL_E_BCN_LOSS,
	BCM_OL_E_DEAUTH,
	BCM_OL_E_DISASSOC,
	BCM_OL_E_RETROGRADE_TSF,
	BCM_OL_E_RADIO_HW_DISABLED,
	BCM_OL_E_PME_ASSERTED,
	BCM_OL_E_UNASSOC,
	BCM_OL_E_SCAN_BEGIN,
	BCM_OL_E_SCAN_END,
	BCM_OL_E_PREFSSID_FOUND,
	BCM_OL_E_CSA,
	BCM_OL_E_MAX
};

enum {
	BCM_OL_UNUSED,	/* 0 */
	BCM_OL_BEACON_ENABLE,
	BCM_OL_BEACON_DISABLE,
	BCM_OL_RSSI_INIT,
	BCM_OL_LOW_RSSI_NOTIFICATION,
	BCM_OL_ARP_ENABLE,
	BCM_OL_ARP_SETIP,
	BCM_OL_ARP_DISABLE,
	BCM_OL_ND_ENABLE,
	BCM_OL_ND_SETIP,
	BCM_OL_ND_DISABLE,
	BCM_OL_PKT_FILTER_ENABLE,
	BCM_OL_PKT_FILTER_ADD,
	BCM_OL_PKT_FILTER_DISABLE,
	BCM_OL_WOWL_ENABLE_START,
	BCM_OL_WOWL_ENABLE_COMPLETE,
	BCM_OL_WOWL_ENABLE_COMPLETED,
	BCM_OL_ARM_TX,
	BCM_OL_ARM_TX_DONE,
	BCM_OL_RESET,
	BCM_OL_FIFODEL,
	BCM_OL_MSG_TEST,
	BCM_OL_MSG_IE_NOTIFICATION_FLAG,
	BCM_OL_MSG_IE_NOTIFICATION,
	BCM_OL_SCAN_ENAB,
	BCM_OL_SCAN,
	BCM_OL_SCAN_RESULTS,
	BCM_OL_SCAN_CONFIG,
	BCM_OL_SCAN_BSS,
	BCM_OL_SCAN_QUIET,
	BCM_OL_SCAN_VALID2G,
	BCM_OL_SCAN_VALID5G,
	BCM_OL_SCAN_CHANSPECS,
	BCM_OL_SCAN_BSSID,
	BCM_OL_MACADDR,
	BCM_OL_SCAN_TXRXCHAIN,
	BCM_OL_SCAN_COUNTRY,
	BCM_OL_SCAN_PARAMS,
	BCM_OL_SSIDS,
	BCM_OL_PREFSSIDS,
	BCM_OL_PFN_LIST,
	BCM_OL_PFN_ADD,
	BCM_OL_PFN_DEL,
	BCM_OL_ULP,
	BCM_OL_CURPWR,
	BCM_OL_SARLIMIT,
	BCM_OL_TXCORE,
	BCM_OL_SCAN_DUMP,
	BCM_OL_MSGLEVEL,
	BCM_OL_MSGLEVEL2,
	BCM_OL_DMA_DUMP,
	BCM_OL_BCNS_PROMISC,
	BCM_OL_SETCHANNEL,
	BCM_OL_L2KEEPALIVE_ENABLE,
	BCM_OL_GTK_ENABLE,
	BCM_OL_LTR,
	BCM_OL_TCP_KEEP_CONN,
	BCM_OL_TCP_KEEP_TIMERS,
	BCM_OL_EL_START,
	BCM_OL_EL_SEND_REPORT,
	BCM_OL_EL_REPORT,
	BCM_OL_PANIC,
	BCM_OL_CONS,
	BCM_OL_MSG_MAX /* Keep this last */
};

/* L2 Keepalive feature flags */
enum {
	BCM_OL_KEEPALIVE_RX_SILENT_DISCARD = 1<<0,
	BCM_OL_KEEPALIVE_PERIODIC_TX = 1<<1,
	BCM_OL_KEEPALIVE_PERIODIC_TX_QOS = 1<<2
};

#include <packed_section_start.h>

typedef BWL_PRE_PACKED_STRUCT struct ol_iv {
	uint8		buf[WOWL_TSCPN_SIZE];
	uint16		PAD;
} BWL_POST_PACKED_STRUCT ol_iv_t;

typedef BWL_PRE_PACKED_STRUCT struct olmsg_arp_stats_t {
	struct ipv4_addr src_ip;
	struct ipv4_addr dest_ip;
	uint8 suppressed;
	uint8 is_request;
	uint8 resp_sent;
	uint8 armtx;
} BWL_POST_PACKED_STRUCT olmsg_arp_stats;

typedef BWL_PRE_PACKED_STRUCT struct olmsg_nd_stats_t {
	struct ipv6_addr dest_ip;
	uint8 suppressed;
	uint8 is_request;
	uint8 resp_sent;
	uint8 armtx;
} BWL_POST_PACKED_STRUCT olmsg_nd_stats;

typedef BWL_PRE_PACKED_STRUCT struct olmsg_pkt_filter_stats_t {
	uint8	suppressed;
	uint8	pkt_filtered;
	uint8	matched_pattern;
	uint8	matched_magic;
	uint32	pattern_id;
} BWL_POST_PACKED_STRUCT olmsg_pkt_filter_stats;

typedef BWL_PRE_PACKED_STRUCT struct olmsg_txpkt_status_t {
	uint32	tottxpkt;
	uint32	datafrm;
	uint32	nullfrm;
	uint32	pspoll;
	uint32	probereq;
	uint32	txacked;
	uint32	txsupressed;
} BWL_POST_PACKED_STRUCT olmsg_txpkt_status;

typedef BWL_PRE_PACKED_STRUCT struct olmsg_rxpkt_status_t {
	uint32	totrxpkt;
	uint32	mgmtfrm;
	uint32	datafrm;
	uint32	scanprocessfrm;
	uint32	unassfrmdrop;
	uint32	badfcs;
	uint32	badrxchan;
	uint32	badfrmlen;
} BWL_POST_PACKED_STRUCT olmsg_rxpkt_status;

typedef BWL_PRE_PACKED_STRUCT struct olmsg_rssi_t {
	uint8	rssi;			/* This is where we will keep caluculated RSSI Average */
	uint8	noise_avg;		/* This is where we will keep caluculated Noise Average */
	uint8	rxpwr[WL_RSSI_ANT_MAX];	/* 8 bit value for multiple Antennas */
	uint8	PAD[2];
} BWL_POST_PACKED_STRUCT olmsg_rssi;

typedef BWL_PRE_PACKED_STRUCT struct olmsg_dump_stats_info_t {
	uint32 	rxoe_bcncnt;
	uint32 	rxoe_bcntohost;
	uint32 	rxoe_bcndelcnt;
	uint32 	rxoe_obssidcnt;
	uint32 	rxoe_capchangedcnt;
	uint32 	rxoe_bcnintchangedcnt;
	uint32	rxoe_bcnlosscnt;
	uint16	rxoe_iechanged[OLMSG_BCN_MAX_IE];
	uint16	rxoe_arp_statidx;
	uint16	PAD;
	uint32 	rxoe_totalarpcnt;
	uint32	rxoe_arpcnt;	/* arp received for US */
	uint32 	rxoe_arpsupresscnt;
	uint32 	rxoe_arpresponsecnt;
	olmsg_arp_stats rxoe_arp_stats[MAX_STAT_ENTRIES];
	uint16 	rxoe_nd_statidx;
	uint16	PAD;
	uint32 	rxoe_totalndcnt;
	uint32	rxoe_nscnt;
	uint32 	rxoe_nssupresscnt;
	uint32 	rxoe_nsresponsecnt;
	olmsg_nd_stats	rxoe_nd_stats[MAX_STAT_ENTRIES];
	uint16	rxoe_pkt_filter_statidx;
	uint16	PAD;
	uint32 	rxoe_total_pkt_filter_cnt;
	uint32 	rxoe_total_matching_pattern_cnt;
	uint32 	rxoe_total_matching_magic_cnt;
	uint32 	rxoe_pkt_filter_supresscnt;
	olmsg_pkt_filter_stats rxoe_pkt_filter_stats[MAX_STAT_ENTRIES];
	olmsg_rxpkt_status	rxoe_rxpktcnt;
	olmsg_txpkt_status	rxoe_txpktcnt;
} BWL_POST_PACKED_STRUCT olmsg_dump_stats;

typedef BWL_PRE_PACKED_STRUCT struct vndriemask_info_t {
	union {
		struct ouidata {
		uint8	id[3];
		uint8	type;
		} b;
		uint32  mask;
	} oui;
} BWL_POST_PACKED_STRUCT vndriemask_info;

/* Read/Write Context */
typedef BWL_PRE_PACKED_STRUCT struct {
	uint32	offset;
	uint32	size;
	uint32	rx;
	uint32	wx;
} BWL_POST_PACKED_STRUCT volatile olmsg_buf_info;

/* TBD: Should be a  packed structure */
typedef BWL_PRE_PACKED_STRUCT struct olmsg_header_t {
	uint32 type;
	uint32 seq;
	uint32 len;
} BWL_POST_PACKED_STRUCT olmsg_header;

typedef BWL_PRE_PACKED_STRUCT struct olmsg_test_t {
	olmsg_header hdr;
	uint32 data;
} BWL_POST_PACKED_STRUCT olmsg_test;

typedef BWL_PRE_PACKED_STRUCT struct olmsg_bmac_up_t {
	olmsg_header hdr;
} BWL_POST_PACKED_STRUCT olmsg_bmac_up;

typedef BWL_PRE_PACKED_STRUCT struct olmsg_reset_t {
	olmsg_header hdr;
} BWL_POST_PACKED_STRUCT olmsg_reset;

typedef BWL_PRE_PACKED_STRUCT struct olmsg_fifodel_t {
	olmsg_header hdr;
	uint8 enable;
} BWL_POST_PACKED_STRUCT olmsg_fifodel;

typedef BWL_PRE_PACKED_STRUCT struct olmsg_bcn_enable_t {
	olmsg_header    hdr;
	/* Deferral count to inform ucode */
	uint32	defcnt;

	/* BSSID beacon length */
	uint32	bcn_length;
	/* BSSID to support per interface */
	struct  ether_addr      BSSID;          /* Associated with BSSID */
	struct  ether_addr      cur_etheraddr;  /* Current Ethernet Address */

	/* beacon interval  */
	uint16	bi; /* units are Kusec */

	/* Beacon capability */
	uint16 capability;

	/* Beacon received channel */
	uint32	rxchannel;

	/* Control channel */
	uint32	ctrl_channel;

	/* association aid */
	uint16	aid;

	uint8 	frame_del;

	uint8	iemask[IEMASK_SZ];

	vndriemask_info	vndriemask[MAX_VNDR_IE];

	uint32	iedatalen;

	uint8	iedata[1];			/* Elements */
} BWL_POST_PACKED_STRUCT olmsg_bcn_enable;

typedef BWL_PRE_PACKED_STRUCT struct olmsg_bcn_disable_t {
	olmsg_header hdr;
	struct  ether_addr      BSSID;          /* Associated with BSSID */
} BWL_POST_PACKED_STRUCT olmsg_bcn_disable;

typedef BWL_PRE_PACKED_STRUCT struct olmsg_rssi_init_t {
	olmsg_header hdr;
	uint8	enabled;		/* Flag to indicate RSSI enabled */
	int8	low_threshold;		/* Low RSSI Threshold value */
	int8	roam_off;		/* Low RSSI roam enabled */
	uint8	mode;			/* Mode: MAX, MIN, AVG */
	uint8	phyrxchain;		/* Antenna chain */
	int8	phy_rssi_gain_error[WL_RSSI_ANT_MAX]; /* RSSI gain error */
	uint16	current_temp;		/* current temperature */
	uint16	raw_tempsense;		/* temperature from ROM */
	uint16	radio_chanspec;		/* Radio channel spec */
} BWL_POST_PACKED_STRUCT olmsg_rssi_init;

typedef BWL_PRE_PACKED_STRUCT struct ol_key_info_t {
	wlc_key_info_t	info;
	wlc_key_hw_index_t hw_idx;
	uint8		data[DOT11_MAX_KEY_SIZE];	/* key data */
	ol_iv_t		txiv;		/* Tx IV */
	ol_iv_t		rxiv[WLC_KEY_BASE_RX_SEQ];	/* Rx IV (one per TID) */
} BWL_POST_PACKED_STRUCT ol_key_info;

#define OL_SEC_F_NONE           0x00000000
#define OL_SEC_F_QOS            0x00000001
#define OL_SEC_F_IBSS           0x00000002
#define OL_SEC_F_WIN7PLUS       0x00000004
#define OL_SEC_F_WAKEON1MICERR  0x00000008
#define OL_SEC_F_LEGACY_AES     0x00000010
#define OL_SEC_F_MFP            0x00000020

#define OL_IGTK_IDX_POS(key_id) ((key_id) - WLC_KEY_ID_IGTK_1)

typedef BWL_PRE_PACKED_STRUCT struct ol_sec_info_t {
	uint32			flags;
	uint32			wsec;
	uint32			WPA_auth;
	uint8			wsec_restrict;
	uint8			bss_tx_key_id;
	ol_key_info 	scb_key_info;
	ol_key_info 	bss_key_info[WLC_KEYMGMT_NUM_GROUP_KEYS];
	ol_key_info 	igtk_key_info[WLC_KEYMGMT_NUM_BSS_IGTK];
	uint8			key_rot_id_mask;	/* key id mask of group keys updated */
	uint8			PAD[1];
} BWL_POST_PACKED_STRUCT ol_sec_info;

typedef BWL_PRE_PACKED_STRUCT struct ol_tx_info_t {
	struct  ether_addr	BSSID;          /* Associated with BSSID */
	struct  ether_addr	cur_etheraddr;  /* Current Ethernet Address */
	ol_sec_info			sec_info;
	uint16			rate;
	uint16			chanspec;
	uint16			aid;		/* association aid */
	uint16			PhyTxControlWord_0;
	uint16			PhyTxControlWord_1;
	uint16			PhyTxControlWord_2;
	uint8			replay_counter[EAPOL_KEY_REPLAY_LEN]; /* out of sec info for now */
	uint8			PAD[2];
} BWL_POST_PACKED_STRUCT ol_tx_info;

typedef BWL_PRE_PACKED_STRUCT struct olmsg_arp_enable_t {
	olmsg_header		hdr;
	struct ether_addr	host_mac;
	struct ipv4_addr	host_ip;
	struct  ether_addr  BSSID;
	ol_sec_info			sec_info;
	uint8				PAD[3];
} BWL_POST_PACKED_STRUCT olmsg_arp_enable;

typedef BWL_PRE_PACKED_STRUCT struct olmsg_arp_disable_t {
	olmsg_header hdr;
} BWL_POST_PACKED_STRUCT olmsg_arp_disable;

typedef BWL_PRE_PACKED_STRUCT struct olmsg_arp_setip_t {
	olmsg_header 		hdr;
	struct	ipv4_addr	host_ip;
} BWL_POST_PACKED_STRUCT olmsg_arp_setip;

typedef BWL_PRE_PACKED_STRUCT struct olmsg_tcp_keep_conn_t {
	olmsg_header 		hdr;
	wl_mtcpkeep_alive_conn_pkt_t	tcp_keepalive_conn;
} BWL_POST_PACKED_STRUCT olmsg_tcp_keep_conn;

typedef BWL_PRE_PACKED_STRUCT struct olmsg_tcp_keep_timers_t {
	olmsg_header 		hdr;
	wl_mtcpkeep_alive_timers_pkt_t	tcp_keepalive_timers;
} BWL_POST_PACKED_STRUCT olmsg_tcp_keep_timers;

typedef BWL_PRE_PACKED_STRUCT struct olmsg_nd_enable_t {
	olmsg_header 		hdr;
	struct ether_addr	host_mac;
	struct ipv6_addr	host_ip;
	struct  ether_addr  BSSID;
	ol_sec_info			sec_info;
	uint8				PAD[3];
} BWL_POST_PACKED_STRUCT olmsg_nd_enable;

typedef BWL_PRE_PACKED_STRUCT struct olmsg_nd_disable_t {
	olmsg_header		hdr;
} BWL_POST_PACKED_STRUCT olmsg_nd_disable;

typedef BWL_PRE_PACKED_STRUCT struct olmsg_nd_setip_t {
	olmsg_header 		hdr;
	struct ipv6_addr	host_ip;
} BWL_POST_PACKED_STRUCT olmsg_nd_setip;

typedef BWL_PRE_PACKED_STRUCT struct olmsg_armtx_t {
	olmsg_header hdr;
	uint8 TX;
    ol_tx_info txinfo;
} BWL_POST_PACKED_STRUCT olmsg_armtx;

typedef BWL_PRE_PACKED_STRUCT struct olmsg_armtxdone_t {
	olmsg_header 	hdr;
	ol_tx_info 	txinfo;
} BWL_POST_PACKED_STRUCT olmsg_armtxdone;

/* Add IE NOTIFICATION STRUCT HERE */
typedef BWL_PRE_PACKED_STRUCT struct olmsg_ie_notification_enable_t {
	olmsg_header            hdr;            /* Message Header */
	struct  ether_addr      BSSID;          /* Associated with BSSID */
	struct  ether_addr      cur_etheraddr;  /* Current Ethernet Address */
	uint32                  id;             /* IE Mask for standard IE */
	uint32                  enable;         /* IE Mask enable/disable flag */

} BWL_POST_PACKED_STRUCT olmsg_ie_notification_enable;

typedef BWL_PRE_PACKED_STRUCT struct olmsg_scan32_t {
	olmsg_header hdr;
	uint32 count;
	uint32 list[10];
} BWL_POST_PACKED_STRUCT olmsg_scan32;

typedef BWL_PRE_PACKED_STRUCT struct olmsg_scanchspec_t {
	olmsg_header hdr;
	uint32 count;
	uint32 list[33];
} BWL_POST_PACKED_STRUCT olmsg_scanchspec;

typedef BWL_PRE_PACKED_STRUCT struct olmsg_chanvec_t {
	olmsg_header hdr;
	chanvec_t chanvec;
} BWL_POST_PACKED_STRUCT olmsg_chanvec;

typedef BWL_PRE_PACKED_STRUCT struct olmsg_country_t {
	olmsg_header hdr;
	char country[WLC_CNTRY_BUF_SZ];
} BWL_POST_PACKED_STRUCT olmsg_country;

typedef BWL_PRE_PACKED_STRUCT struct olmsg_addr_t {
	olmsg_header hdr;
	struct ether_addr addr;
} BWL_POST_PACKED_STRUCT olmsg_addr;

#define MAX_SSID_CNT		16
typedef BWL_PRE_PACKED_STRUCT struct olmsg_ssids_t {
	olmsg_header hdr;
	wlc_ssid_t ssid[1];
} BWL_POST_PACKED_STRUCT olmsg_ssids;

typedef BWL_PRE_PACKED_STRUCT struct olmsg_pfn_t {
	olmsg_header hdr;
	pfn_olmsg_params params;
} BWL_POST_PACKED_STRUCT olmsg_pfn;

typedef BWL_PRE_PACKED_STRUCT struct olmsg_scanparams_t {
	olmsg_header hdr;
	scanol_params_t params;
} BWL_POST_PACKED_STRUCT olmsg_scanparams;

typedef BWL_PRE_PACKED_STRUCT struct olmsg_pkt_filter_enable_t {
	olmsg_header		hdr;
	struct ether_addr	host_mac;
} BWL_POST_PACKED_STRUCT olmsg_pkt_filter_enable;

typedef BWL_PRE_PACKED_STRUCT struct olmsg_pkt_filter_add_t {
	olmsg_header		hdr;
	wl_pkt_filter_t         pkt_filter;
} BWL_POST_PACKED_STRUCT olmsg_pkt_filter_add;

typedef BWL_PRE_PACKED_STRUCT struct olmsg_pkt_filter_disable_t {
	olmsg_header hdr;
} BWL_POST_PACKED_STRUCT olmsg_pkt_filter_disable;

/*
 * Message from host to ARM: Notification of start of WoWL enable.
 * Other messages can be sent from host after this message, but those
 * messages cannot require a handshake message from ARM after WoWL is
 * enabled.
 */

/* WOWL cfg data passed to ARM by host driver */
typedef BWL_PRE_PACKED_STRUCT struct wowl_cfg {
	uint8		wowl_enabled;   /* TRUE iff WoWL is enabled by host driver */
	uint32		wowl_flags;     /* WL_WOWL_Xxx flags defined in wlioctl.h */
	uint32		wowl_test;      /* Wowl test: seconds to sleep before waking */
	uint32		bcn_loss;       /* Threshold for bcn loss before waking host */
	uint32		associated;     /* Whether we are entering wowl in assoc mode */
	uint32		PM;             /* PM mode for wowl */
} BWL_POST_PACKED_STRUCT wowl_cfg_t;

typedef BWL_PRE_PACKED_STRUCT struct olmsg_wowl_enable_start_t {
	olmsg_header	hdr;
	wowl_cfg_t	wowl_cfg;
} BWL_POST_PACKED_STRUCT olmsg_wowl_enable_start;

/*
 * Message from host to ARM: Notification of end of WoWL enable.
 * This must be the last host message when WoWL is being enabled.
 */
typedef BWL_PRE_PACKED_STRUCT struct olmsg_wowl_enable_complete_t {
	olmsg_header	hdr;
} BWL_POST_PACKED_STRUCT olmsg_wowl_enable_complete;

typedef BWL_PRE_PACKED_STRUCT struct olmsg_l2keepalive_enable {
	olmsg_header	hdr;
	uint16	period_ms;
	uint8	prio;
	uint8	flags;
} BWL_POST_PACKED_STRUCT olmsg_l2keepalive_enable_t;

typedef BWL_PRE_PACKED_STRUCT struct rsn_rekey_params_t {
	uint8	kck[WPA_MIC_KEY_LEN];
	uint8	kek[WPA_ENCR_KEY_LEN];
	uint8	replay_counter[EAPOL_KEY_REPLAY_LEN];	/* replay counter */
} BWL_POST_PACKED_STRUCT rsn_rekey_params;

typedef BWL_PRE_PACKED_STRUCT struct olmsg_gtk_enable_t {
	olmsg_header		hdr;
	struct  ether_addr  	BSSID;
	struct  ether_addr  	cur_etheraddr;
	ol_sec_info         	sec_info;
	rsn_rekey_params	rekey;
	wlc_key_algo_t		gtk_algo;
	uint8	PAD[2];
	uint32			igtk_enabled;
} BWL_POST_PACKED_STRUCT olmsg_gtk_enable;

/*
 * Message from ARM to host: Notification of completion of WoWL enable.
 * This notification informs the host that the ARM is now ready to operate
 * in a low-power state and will not access host memory.
 */
typedef BWL_PRE_PACKED_STRUCT struct olmsg_wowl_enable_completed_t {
	olmsg_header	hdr;
} BWL_POST_PACKED_STRUCT olmsg_wowl_enable_completed;

typedef BWL_PRE_PACKED_STRUCT struct olmsg_low_rssi_notification_t {
	olmsg_header	hdr;
	int8		rssi;
} BWL_POST_PACKED_STRUCT olmsg_low_rssi_notification;

typedef BWL_PRE_PACKED_STRUCT struct olmsg_eventlog_notification_t {
	olmsg_header	hdr;
	int8		num;
	int8		totalnum;
	int16		buflen;
	int8		buf[0];
} BWL_POST_PACKED_STRUCT olmsg_eventlog_notification;

#ifdef WL_LTR
typedef BWL_PRE_PACKED_STRUCT struct olmsg_ltr_t {
	olmsg_header	hdr;
	bool   _ltr;			/* LTR cap enabled/disabled */
	uint32 active_lat;		/* LTR active latency */
	uint32 active_idle_lat;	/* LTR active idle latency */
	uint32 sleep_lat;		/* LTR sleep latency */
	uint32 hi_wm;			/* FIFO watermark for LTR active transition */
	uint32 lo_wm;			/* FIFO watermark for LTR sleep transition */
} BWL_POST_PACKED_STRUCT olmsg_ltr;
#endif /* WL_LTR */

/*
 * The following structures define the format of the shared memory between
 * the ARM and the host driver.
 */
#ifndef ETHER_MAX_DATA
#define ETHER_MAX_DATA		    1500
#endif // endif

#define MAX_WAKE_PACKET_BYTES	    (DOT11_A3_HDR_LEN +			    \
				     DOT11_QOS_LEN +			    \
				     sizeof(struct dot11_llc_snap_header) + \
				     ETHER_MAX_DATA)

/* WOWL info returned by ARM firmware to host driver */
typedef BWL_PRE_PACKED_STRUCT struct wowl_wake_pkt_info {
	uint32		pattern_id;		/* ID of pattern that packet matched */
	uint32		original_packet_size;	/* Original size of packet */
	uint32		phy_type;
	uint32		channel;
	uint32		rate;
	uint32		rssi;
} BWL_POST_PACKED_STRUCT wowl_wake_info_t;

typedef BWL_PRE_PACKED_STRUCT struct scan_wake_pkt_info {
	wlc_ssid_t		ssid;			/* ssid that matched */
	uint16			capability;		/* Capability information */
	chanspec_t		chanspec;		/* Capability information */
	uint32			rssi;
	struct rsn_parms	wpa;
	struct rsn_parms	wpa2;
} BWL_POST_PACKED_STRUCT scan_wake_pkt_info_t;

#define WOWL_MIC_FAIL_F_MULTICAST 0x01 /* whether pkt is multicast */
#define WOWL_MIC_FAIL_F_GROUP_KEY 0x02 /* used group key to decode */

typedef BWL_PRE_PACKED_STRUCT struct wowl_mic_fail_info {
	uint32 fail_time;	/* now - fail_time when returned to host */
	uint8 flags;
	uint8 key_id;
	uint8 PAD[2];
} BWL_POST_PACKED_STRUCT wowl_mic_fail_info_t;

typedef BWL_PRE_PACKED_STRUCT struct wowl_mic_fail_pkt_info {
	uint8	fail_count;
	uint8	PAD[3];
	wowl_mic_fail_info_t fail_info[2];
} BWL_POST_PACKED_STRUCT wowl_mic_fail_pkt_info_t;

typedef BWL_PRE_PACKED_STRUCT struct wowl_host_info {
	uint32		wake_reason;	/* WL_WOWL_Xxx value */
	uint8		eventlog[MAX_OL_EVENTS];
	uint32		eventidx;
	union {
		wowl_wake_info_t			pattern_pkt_info;
		scan_wake_pkt_info_t		scan_pkt_info;
	} u;
	uint32          pkt_len;
	uchar           pkt[ETHER_MAX_DATA];
	olmsg_armtxdone	wake_tx_info;	/* Tx done settings returned to the host */
	wowl_mic_fail_pkt_info_t	mic_fail_info; /* can be returned on non-mic fail reason */
} BWL_POST_PACKED_STRUCT wowl_host_info_t;

#define MDNS_DBASE_SZ	4096
typedef BWL_PRE_PACKED_STRUCT struct olmsg_shared_info {
	uint32		    marker_begin;
	uint32		    msgbufaddr_low;
	uint32		    msgbufaddr_high;
	uint32		    msgbuf_sz;
	uint32		    console_addr;
	/* flag[] is boolean array; and turn is an integer */
	uint32		    flag[2];
	uint32		    turn;
	uint32		    vars_size;
	uint8		    vars[MAXSZ_NVRAM_VARS];
	uint32		    mdns_len;
	uint8		    mdns_dbase[MDNS_DBASE_SZ];
	uint32		    dngl_watchdog_cntr;
	olmsg_rssi	    rssi_info;
	uint32		    eventlog_addr;
	olmsg_dump_stats    stats;
	wowl_host_info_t    wowl_host_info;
	uint32		    marker_end;
} BWL_POST_PACKED_STRUCT volatile olmsg_shared_info_t;

typedef BWL_PRE_PACKED_STRUCT struct olmsg_curpwr_t {
	olmsg_header hdr;
	uint8 curpwr[MAXCHANNEL];
} BWL_POST_PACKED_STRUCT olmsg_curpwr;

typedef BWL_PRE_PACKED_STRUCT struct olmsg_sarlimit_t {
	olmsg_header hdr;
	sar_limit_t sarlimit;
} BWL_POST_PACKED_STRUCT olmsg_sarlimit;

#define CMDLINESZ	80

typedef BWL_PRE_PACKED_STRUCT struct olmsg_ol_conscmd_t {
	olmsg_header hdr;
	uint8 cmdline[CMDLINESZ];
} BWL_POST_PACKED_STRUCT olmsg_ol_conscmd;

/* Message buffer start addreses is written at the end of
 * ARM memroy, 32 bytes additional.
 */
#define sharedsz		(sizeof(olmsg_shared_info_t) + 32)
#define OLMSG_SHARED_INFO_SZ	ROUNDUP(sharedsz, sizeof(uint32))

/* Modify below if the size changes */
#define OLMSG_SHARED_INFO_SZ_NUM  (15964)
#include <packed_section_end.h>

typedef struct olmsg_info_t {
	uchar	*msg_buff;
	uint32	len;
	olmsg_buf_info *write;
	olmsg_buf_info *read;
	uint32	next_seq;
} olmsg_info;

extern int
bcm_olmsg_create(uchar *buf, uint32 len);

/* Initialize message buffer */
extern int
bcm_olmsg_init(olmsg_info *ol_info, uchar *buf, uint32 len, uint8 rx, uint8 wx);

extern void
bcm_olmsg_deinit(olmsg_info *ol_info);

/* Copies the next message to be read into buf
	Updates the read pointer
	returns size of the message
	Pass NULL to retrieve the size of the message
 */
extern int
bcm_olmsg_getnext(olmsg_info *ol_info, char *buf, uint32 size);

/* same as bcm_olmsg_getnext, except that read pointer it not updated
 */
int
bcm_olmsg_peeknext(olmsg_info *ol_info, char *buf, uint32 size);

/* Writes the message to the shared buffer
	Updates the write pointer
	returns TRUE/FALSE depending on the availability of the space
	Pass NULL to retrieve the size of the message
 */
extern bool
bcm_olmsg_write(olmsg_info *ol_info, char *buf, uint32 size);

/*
 * Returns free space of the message buffer
 */
extern uint32
bcm_olmsg_avail(olmsg_info *ol_info);

extern bool
bcm_olmsg_is_writebuf_full(olmsg_info *ol_info);

extern bool
bcm_olmsg_is_writebuf_empty(olmsg_info *ol_info);

extern int
bcm_olmsg_writemsg(olmsg_info *ol, uchar *buf, uint16 len);

extern uint32
bcm_olmsg_bytes_to_read(olmsg_info *ol_info);

extern bool
bcm_olmsg_is_readbuf_empty(olmsg_info *ol_info);

extern uint32
bcm_olmsg_peekbytes(olmsg_info *ol, uchar *dst, uint32 len);

extern uint32
bcm_olmsg_readbytes(olmsg_info *ol, uchar *dst, uint32 len);

extern uint16
bcm_olmsg_peekmsg_len(olmsg_info *ol);

extern uint16
bcm_olmsg_readmsg(olmsg_info *ol, uchar *buf, uint16 len);

extern void
bcm_olmsg_dump_msg(olmsg_info *ol, olmsg_header *hdr);

extern void
bcm_olmsg_dump_record(olmsg_info *ol);

extern olmsg_shared_info_t *ppcie_shared;

#endif /* _BCM_OL_MSG_H_ */
