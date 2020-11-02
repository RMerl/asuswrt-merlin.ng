/*
 * WLC LTE Coex module API definition
 * Broadcom 802.11abg Networking Device Driver
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
 * $Id: wlc_ltecx.h 708017 2017-06-29 14:11:45Z $
 */

#ifndef _wlc_ltecx_h_
#define _wlc_ltecx_h_

/* LTECX - WCI2 Default baud rate */
#define LTECX_WCI2_INVALID_BAUD		0
#define LTECX_WCI2_DEFAULT_BAUD		3

/* ltecxflg interface mask */
#define	LTECX_LOOKAHEAD_MASK	0x00FFF
#define	LTECX_BAUDRATE_MASK		0x0F000
#define	LTECX_TX_IND_MASK		0x10000

/* LTE B40 parameters */
#define LTECX_NVRAM_PARAM_MAX			3
#define LTECX_NVRAM_WLANRX_PROT			0
#define LTECX_NVRAM_LTERX_PROT			1
#define LTECX_NVRAM_SCANJOIN_PROT		2
#define LTECX_NVRAM_RSSI_THRESH_20MHZ	5
#define LTECX_NVRAM_RSSI_THRESH_10MHZ	6
#define LTECX_NVRAM_MAX_CHANNELS		13
#define LTECX_NVRAM_GET_PROT_MASK		4
#define LTECX_NVRAM_20M_RSSI_2390		0
#define LTECX_NVRAM_20M_RSSI_2385		1
#define LTECX_NVRAM_20M_RSSI_2380		2
#define LTECX_NVRAM_20M_RSSI_2375		3
#define LTECX_NVRAM_20M_RSSI_2370		4
#define LTECX_NVRAM_10M_RSSI_2395		0
#define LTECX_NVRAM_10M_RSSI_2390		1
#define LTECX_NVRAM_10M_RSSI_2385		2
#define LTECX_NVRAM_10M_RSSI_2380		3
#define LTECX_NVRAM_10M_RSSI_2375		4
#define LTECX_NVRAM_10M_RSSI_2370		5

#define LTE_CHANNEL_BW_20MHZ	20000
#define LTE_CHANNEL_BW_10MHZ	10000
#define LTE_BAND40_MAX_FREQ		2400
#define LTE_BAND40_MIN_FREQ		2300
#define LTE_20MHZ_INIT_STEP		10
#define LTE_10MHZ_INIT_STEP		5
#define LTE_RSSI_THRESH_LMT		2
#define LTE_FREQ_STEP_SIZE		5
#define LTE_FREQ_STEP_MAX		8
#define LTE_MAX_FREQ_DEVIATION	2
#define LTECX_LOOKAHEAD_SHIFT	0
#define LTECX_BAUDRATE_SHIFT	12
#define LTECX_TX_IND_SHIFT		16

#define LTECX_MIN_CH_MASK		0xF

typedef enum shm_ltecx_hflags_e {
	C_LTECX_HOST_COEX_EN	= 0,	/* 1: Enable Lte Coex */
	C_LTECX_HOST_RX_ALWAYS,			/* 1: WLAN Rx not affected by LTE Tx */
	C_LTECX_HOST_TX_NEGEDGE,		/* 1: LTE_Tx lookahead de-asserts
									 *  at actual LTE_Tx end
									 */
	C_LTECX_HOST_PROT_TXRX,			/* 1: Enable LTE simultaneous TxRx protection */
	C_LTECX_HOST_TX_ALWAYS	= 4,	/* 1: WLAN Tx does not affect LTE Rx */
	C_LTECX_HOST_ASSOC_PROG,		/* 1: Association in progress */
	C_LTECX_HOST_ASSOC_STATE,		/* 1: Client STA associated */
	C_LTECX_HOST_PROT_TYPE_NONE_TMP,	/* bit updated by firmware */
	C_LTECX_HOST_PROT_TYPE_PM_CTS = 8,	/* bit updated by firmware */
	C_LTECX_HOST_PROT_TYPE_NONE,		/* bit updated by ucode */
	C_LTECX_HOST_PROT_TYPE_CTS,		/* 0: Use PM packets, 1: Use CTS2SELF */
	C_LTECX_HOST_PROT_TYPE_AUTO,
	C_LTECX_HOST_RX_ACK	= 12,		/* 0: Cant receive Ack during LTE_Tx */
	C_LTECX_HOST_TXIND,
	C_LTECX_HOST_SCANJOIN_PROT,
	C_LTECX_HOST_INTERFACE = 15		/* 0: WCI2, 1: ERCX Interface */
} shm_ltecx_hflags_t;

typedef enum {
	C_LTECX_ST_PROT_REQ	= 0,		/* 1: LTECX Protection Requested */
	C_LTECX_ST_IDLE,				/* 1: LTE is idle */
	C_LTECX_ST_ACTUAL_TX,			/* 1: LTE Tx On */
	C_LTECX_ST_TX_PREV,				/* Previous LTE Tx (with lookahead) */
	C_LTECX_ST_WLAN_PRIO = 4,		/* 1: WLAN in critical */
	C_LTECX_ST_PRQ_ACTIVE,			/* Probe request sent */
	C_LTECX_ST_PROT_PND,			/* 1: LTECX Protection Pending */
	C_LTECX_ST_PROT_REQ_CTS,		/* 1: LTECX Protection Requested CTS2SELF */
	C_LTECX_ST_RESEND_GCI_BITS = 8,	/* 1: Indicate the status to the MWS. */
	C_LTECX_ST_TYPE3_INFINITE_STATE,	/* 1: TYPE 3 MSG with infinite duration. */
	C_LTECX_ST_CRTI_DEBUG_MODE,		/* 1: CRTI DEBUG MODE Enabled */
	C_LTECX_ST_CRTI_DEBUG_MODE_TMP
} shm_ltecx_state_t;

/* LTE coex definitions */
typedef enum mws_wlanrx_prot_e {
	C_LTECX_MWS_WLANRX_PROT_NONE	= 0,
	C_LTECX_MWS_WLANRX_PROT_CTS,
	C_LTECX_MWS_WLANRX_PROT_PM,
	C_LTECX_MWS_WLANRX_PROT_AUTO
} mws_wlanrx_prot_t;

/* LTE Flags bits */
typedef enum {
	C_LTECX_FLAGS_LPBKSRC	= 0,
	C_LTECX_FLAGS_LPBKSINK
} shm_ltecx_flags_t;

typedef enum {
	C_LTECX_DATA_TYPE_INT16,
	C_LTECX_DATA_TYPE_UINT32
} ltecx_arr_datatype_t;

#define LTECX_FLAGS_LPBKSRC_MASK (1 << C_LTECX_FLAGS_LPBKSRC)
#define LTECX_FLAGS_LPBKSINK_MASK (1 << C_LTECX_FLAGS_LPBKSINK)
#define LTECX_FLAGS_LPBK_MASK ((LTECX_FLAGS_LPBKSRC_MASK) | (LTECX_FLAGS_LPBKSINK_MASK))

/* LTE coex data structures */
typedef struct {
	uint8 loopback_type;
	uint8 packet;
	uint16 repeat_ct;
} wci2_loopback_t;

typedef struct {
	uint16 nbytes_tx;
	uint16 nbytes_rx;
	uint16 nbytes_err;
} wci2_loopback_rsp_t;

struct wlc_ltecx_info {
	wlc_info_t	*wlc;
	bool		ltecx_enabled;	/* LTECX enabled/disabled in ucode */
	bool		ltecx_idle;		/* LTE signalling IDLE */
	bool		mws_lterx_prot;
	bool		mws_lterx_prot_prev;	/* To detect change in mws_lterx_prot */
	bool		mws_im3_prot;
	bool		mws_im3_prot_prev;	/* To detect change in mws_lterx_prot */
	bool		mws_ltecx_txind;
	bool		mws_ltecx_txind_prev; /* To detect change in mws_ltecx_txind */
	bool		mws_wlan_rx_ack_prev; /* To detect change in rx_ack bit */
	bool		mws_rx_aggr_off;	/* 1: Rx Aggregation disabled by LTECX */
	bool		mws_elna_bypass;		/* 1: elna bypassed 0: elna enabled */
	uint8		mws_wlanrx_prot;
	uint8		mws_wlanrx_prot_prev;	/* Previous protection mode */
	uint8		baud_rate;		/* SECI uart baud rate */
	uint8		ltecx_rssi_thresh_lmt_nvram;
	uint8		mws_ltecx_rssi_thresh_lmt; /* rssi threshold hysteresis loop limit */
	uint8		mws_wlanrx_prot_min_ch;
	uint8		mws_lterx_prot_min_ch;
	uint8		mws_scanjoin_prot_min_ch;
	uint8		mws_lte_freq_index;
	uint16		ltecx_chmap;	/* per-ch ltecx bm (iovar "mws_coex_bitmap") */
	uint16		ltetx_adv;
	uint16		ltetx_adv_prev;	/* To detect change in ltetx_adv */
	uint16		adv_tout_prev;
	uint16		scanjoin_prot;
	uint16		scanjoin_prot_prev; /* To detect change in scanjoin_prot */
	uint16		lte_center_freq_prev;
	uint16		lte_channel_bw_prev;
	uint16		mws_debug_mode;
	uint16		mws_debug_mode_prev; /* Used to optimize shmem access */
	uint16		ltecx_shm_addr;
	int16		mws_wifi_sensi_prev;
	int16		mws_ltecx_wifi_sensitivity;
	int16		mws_elna_rssi_thresh; /* elna bypass RSSI threshold */
	int16		ltecx_rssi_thresh_20mhz[LTECX_NVRAM_RSSI_THRESH_20MHZ]
				[LTECX_NVRAM_MAX_CHANNELS]; /* elna rssi threshold for 20MHz BW */
	int16		ltecx_rssi_thresh_10mhz[LTECX_NVRAM_RSSI_THRESH_10MHZ]
				[LTECX_NVRAM_MAX_CHANNELS]; /* elna rssi threshold for 10MHz BW */
	uint32		ltecx_flags;
	uint32		ltecxmux;	/* LTECX Configuration */
	uint32		ltecxpadnum;
	uint32		ltecxfnsel;
	uint32		ltecxgcigpio;
	uint32		ltecx_20mhz_modes[LTECX_NVRAM_PARAM_MAX];
					/* wlanrx_prot, lterx_prot, scanjoin_prot */
	uint32		ltecx_10mhz_modes[LTECX_NVRAM_PARAM_MAX];
					/* wlanrx_prot, lterx_prot, scanjoin_prot */
	mws_wci2_msg_t	mws_wci2_msg;
	mws_params_t	mws_params;
	wci2_config_t	wci2_config;
};

#ifdef BCMLTECOEX
/* LTE coex functions */
extern wlc_ltecx_info_t *wlc_ltecx_attach(wlc_info_t *wlc);
extern void wlc_ltecx_detach(wlc_ltecx_info_t *ltecx);
extern void wlc_ltecx_init(wlc_ltecx_info_t *ltecx);

extern void wlc_ltecx_update_all_states(wlc_ltecx_info_t *ltecx);
extern void wlc_ltecx_check_chmap(wlc_ltecx_info_t *ltecx);
extern void wlc_ltecx_set_wlanrx_prot(wlc_ltecx_info_t *ltecx);
extern void wlc_ltecx_update_ltetx_adv(wlc_ltecx_info_t *ltecx);
extern void wlc_ltecx_update_lterx_prot(wlc_ltecx_info_t *ltecx);
extern void wlc_ltecx_update_im3_prot(wlc_ltecx_info_t *ltecx);
extern void wlc_ltecx_scanjoin_prot(wlc_ltecx_info_t *ltecx);
extern void wlc_ltetx_indication(wlc_ltecx_info_t *ltecx);
extern bool wlc_ltecx_get_lte_status(wlc_ltecx_info_t *ltecx);
extern bool wlc_ltecx_turnoff_rx_aggr(wlc_ltecx_info_t *ltecx);
extern bool wlc_ltecx_turnoff_tx_aggr(wlc_ltecx_info_t *ltecx);
extern bool wlc_ltecx_get_lte_map(wlc_ltecx_info_t *ltecx);
extern void wlc_ltecx_update_wl_rssi_thresh(wlc_ltecx_info_t *ltecx);
extern void wlc_ltecx_update_wlanrx_ack(wlc_ltecx_info_t *ltecx);
extern int wlc_ltecx_chk_elna_bypass_mode(wlc_ltecx_info_t * ltecx);
extern void wlc_ltecx_update_status(wlc_ltecx_info_t *ltecx);
extern void wlc_ltecx_wifi_sensitivity(wlc_ltecx_info_t *ltecx);
extern void wlc_ltecx_update_debug_msg(wlc_ltecx_info_t *wlc);
extern void wlc_ltecx_update_debug_mode(wlc_ltecx_info_t *wlc);
#ifdef WLRSDB
extern void wlc_ltecx_update_coex_iomask(wlc_ltecx_info_t *ltecx);
#endif /* WLRSDB */
#endif /* BCMLTECOEX */

#endif /* _wlc_ltecx_h_ */
