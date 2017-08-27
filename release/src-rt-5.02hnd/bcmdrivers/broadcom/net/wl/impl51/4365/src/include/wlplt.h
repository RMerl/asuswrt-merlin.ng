/*
 * PLT code specific data structure
 * Broadcom 802.11abg Networking Device Driver
 *
 * Definitions subject to change without notice.
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
 * $Id: wlplt.h 305824 2012-01-03 00:26:26Z $
 */
#ifndef _wlplt_h_
#define _wlplt_h_

#define PLT_PARAMS_MAX		60	/* Maximum size of any PLT in/out structure */
#define PLT_ETHER_ADDR_LEN	6

#define PLT_SRATE_NONHTMODE	0
#define PLT_SRATE_HTMODE	1

#define PLT_SRATE_IGI_800ns	0
#define PLT_SRATE_IGI_400ns	1

#define PLT_PREAMBLE_LONG	0	/* legacy LONG / MIMO Mixed_mode */
#define PLT_PREAMBLE_SHORT	1	/* legacy SHORT / MIMO Greenfield */
#define PLT_PREAMBLE_AUTO	-1

#define PLT_TXPWR_CTRL_OPEN	0
#define PLT_TXPWR_CTRL_CLOSED	1

#define PLT_CSUPP_OFF		0
#define PLT_CSUPP_ON		1

#define PLT_SEQNU_OFF		0
#define PLT_SEQNU_ON		1

#define PLT_BTCX_RFACTIVE	0	/* BTCX signal numbers */
#define PLT_BTCX_TXCONF		1
#define PLT_BTCX_TXSTATUS	2

#define PLT_CBUCKMODE_PWM	0
#define PLT_CBUCKMODE_BURST	1
#define PLT_CBUCKMODE_LPPWM	2

/* In the PMUControl (Chipcommon Offset 0x600):
 *
 * LPOSelect (LS) bit0 	When this field is set to 1, the PMU selects the internal LPO clock.
 * When it is cleared to 0, the PMU selects an external LPO clock.
 *
 * But in the PMUStatus (Chipcommon Offset 0x608):
 * ExtLpoAvailable bit8 This read-only field contains a value of 0x1 if an external LPO clock
 * is available.
 *
 * So these two fields are opposite in definitions in above two registers!
 *
 * The following definitions apply to the register 0x608:
 */
#define PLT_SLEEPCLK_INTERNAL	0
#define PLT_SLEEPCLK_EXTERNAL	1
typedef struct wl_plt_srate {
	uint8		mode;		/* PLT_SRATE_NONHTMODE/HTMODE */
	uint8		igi;		/* PLT_SRATE_IGI_800ns/400ns */
	uint8		m_idx;		/* mcs index for HT mode */
	uint8		pad[1];
	uint32		tx_rate;	/* WLC_RATE_xxx */
} wl_plt_srate_t;

typedef struct wl_plt_continuous_tx {
	uint8 		band;		/* WLC_BAND_2G/5G */
	uint8		channel;
	int8		preamble;	/* PLT_PREAMBLE_SHORT/LONG */
	uint8		carrier_suppress; /* PLT_CSUPP_ON/OFF */
	uint8		pwrctl;		/* PLT_TXPWR_CTRL_OPEN/CLOSED loop */
	int8		power;
	int8		pad[2];
	uint32		ifs;		/* Inter frame spacing in usec */
	wl_plt_srate_t	srate;
} wl_plt_continuous_tx_t;

typedef struct wl_plt_txper_start {
	uint8 		band;		/* WLC_BAND_2G/5G */
	uint8		channel;
	uint8		preamble;	/* PLT_PREAMBLE_SHORT/LONG */
	uint8		seq_ctl;	/* PLT_SEQNU_OFF/ON */
	uint8		pwrctl;		/* PLT_TXPWR_CTRL_OPEN/CLOSED loop */
	uint8		pad[1];
	uint16		length;
	uint8		dest_mac[PLT_ETHER_ADDR_LEN];
	uint8		src_mac[PLT_ETHER_ADDR_LEN];
	uint32		nframes;
	wl_plt_srate_t	srate;
} wl_plt_txper_start_t;

typedef struct wl_plt_rxper_start {
	uint8		band;
	uint8		channel;
	uint8		seq_ctl;	/* PLT_SEQNU_OFF/ON */
	uint8		dst_mac[PLT_ETHER_ADDR_LEN];
	uint8		pad[3];
} wl_plt_rxper_start_t;

typedef struct wl_plt_rxper_results {
	uint32		frames;
	uint32		lost_frames;
	uint32		fcs_errs;
	uint32		plcp_errs;
	uint8		snr;
	uint8		rssi;
	uint8		pad[2];
} wl_plt_rxper_results_t;

typedef struct wl_plt_channel {
	uint8		band;
	uint8		channel;
} wl_plt_channel_t;

#define WLPLT_SUBCARRIER_MAX			56
#define WLPLT_SUBCARRIER_CENTRE			0
#define WLPLT_SUBCARRIER_LEFT_OF_CENTER		28
#define WLPLT_SUBCARRIER_FREQ_SPACING		312500

typedef struct wl_plt_tx_tone {
	wl_plt_channel_t	plt_channel;
	uint8			tone_type;
	uint8			sub_carrier_idx;
} wl_plt_tx_tone_t;

typedef struct wl_plt_desc {
	uchar		build_type[8];
	uchar		build_ver[32];
	uint		chipnum;
	uint		chiprev;
	uint		boardrev;
	uint		boardid;
	uint		ucoderev;
} wl_plt_desc_t;

#endif /* _wlplt_h_ */
