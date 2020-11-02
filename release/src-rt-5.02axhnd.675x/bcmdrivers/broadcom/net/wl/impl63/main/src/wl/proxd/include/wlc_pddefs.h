/*
 * Required external functions and definitions for proximity detection
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
 * $Id: wlc_pddefs.h 788031 2020-06-18 14:10:58Z $
 */
#ifndef _wlc_pddefs_h
#define _wlc_pddefs_h

/******************************************************************************************
 * This Header (internal) file contains all the messges and its declacations that are used by
 * proximity detection.
 * The first section contains  RSSI based proximity detection messages.
*******************************************************************************************
*/
#define CHECK_SIGNATURE(obj, value)       ASSERT((obj)->signature == (uint32)(value))
#define ASSIGN_SIGNATURE(obj, value)      ((obj)->signature = value)

/* Singature declarations for service and each methods */
#define WLC_PDSVC_SIGNATURE		0x80706041
#define PDSVC_RSSI_MTHD_SIGNATURE	0x80706042
#define PDSVC_TOF_MTHD_SIGNATURE	0x80706043

#ifdef	TOF_DEBUG
#define PROXD_TRACE(x)			printf x
#define FUNC_ENTER			printf("----------> %s \n", __FUNCTION__)
#define FUNC_EXIT			printf("<---------- %s \n", __FUNCTION__)
#else
#define PROXD_TRACE(x)
#define FUNC_ENTER
#define FUNC_EXIT
#endif // endif

#ifdef WL_RANGE_SEQ
/* Need ATTACH_ERROR() defintion only for dongle builds.
 * So define it to WL_ERROR for other builds
 */
#define WL_PROXD_ATTACH_ERROR(args)	WL_ERROR(args)
#endif /* WL_RANGE_SEQ */

#define FTM_PROTO_REV 1   /*  TOF extended protocol version */

/* PROXD_MAX_METHOD_NUM should be updated when new methtod enum is added */
#define PROXD_MAX_METHOD_NUM PROXD_TOF_METHOD

/* RSSI Proximity method configuration parameters  definition */
#define PROXD_DEFAULT_CHANSPEC		CH80MHZ_CHSPEC(42, WL_CHANSPEC_CTL_SB_UU) /* 44/80 */
#define PROXD_DEFAULT_INTERVAL		100			/* 100 TU */
#define PROXD_DEFAULT_DURATION		10			/* 10 TU */
#define PROXD_DEFAULT_RSSI_THRESH	-30			/* -30 dBm */
#define PROXD_DEFAULT_TX_POWER		14			/* 14 dBm */
#define PROXD_DEFAULT_TX_RATE		12			/* 6 Mbps */
#define PROXD_DEFAULT_TIMEOUT		20			/* 20 Ms */
#define PROXD_DEFAULT_RETRY_CNT		6			/* retry 6 times */
#define PROXD_LOW_RSSI_VALUE		-127			/* lowest rssi */
/* Debug RSSI threshold */
#define DEBUG_RSSI_THERSH		255			/* rssi theshold */

/* TOF proximity specific default params  */
#define PROXD_DEFAULT_FTM_COUNT		6	/* target: num of ftm frames to send */
#define PROXD_DEFAULT_FTM_SEQ_COUNT	1

#define PROXD_NO_GDADJ			0
#define PROXD_GD_NADJ			1

#define PROXD_MEASUREMENT_PKTID		0x80000000
#define PROXD_FTM_PACKET_TAG		((DOT11_ACTION_CAT_UWNM << 24) |\
						(DOT11_UWNM_ACTION_TIMING_MEASUREMENT << 16))

#ifdef RSSI_REFINE
#define TOF_MAXSCALE			30
#endif // endif

#define TOF_SHIFT		15
/*
	FTM frame period for target, it's chip specific
	and can be configured via "proxd_ftmperiod" iovar,
	the following were calculated for 4335b0:
*/
#define TOF_DEFAULT_SW_ADJ	1
#define TOF_DEFAULT_HW_ADJ	0
#define TOF_DEFAULT_SEQ_EN	0
#define TOF_DEFAULT_VHTACK	1
#define TOF_DEFAULT_CORE_SELECTION 255 /* auto core selection */
#define TOF_DEFAULT_TDCS_EN     FALSE
#define TOF_DEFAULT_LONGWIN_EN     TRUE
#define TOF_DEFAULT_LONGWIN_EN_SEQ     FALSE

/* #define TOF_DEFAULT_USE_MULTICORE */

#define TOF_DEFAULT_THRESHOLD_SCALE_2G	5
#define TOF_DEFAULT_THRESHOLD_LOG2_2G	4
#define TOF_DEFAULT_THRESHOLD_SCALE_20M	5
#define TOF_DEFAULT_THRESHOLD_LOG2_20M	6
#define TOF_DEFAULT_THRESHOLD_SCALE_40M	3
#define TOF_LEGACY_THRESHOLD_SCALE_40M	5
#define TOF_DEFAULT_THRESHOLD_LOG2_40M	7
#define TOF_LEGACY_THRESHOLD_LOG2_40M	6
#define TOF_DEFAULT_THRESHOLD_SCALE_80M	3
#define TOF_LEGACY_THRESHOLD_SCALE_80M	5
#define TOF_DEFAULT_THRESHOLD_LOG2_80M	7
#define TOF_LEGACY_THRESHOLD_LOG2_80M	6
#define TOF_DEFAULT_TX_THRESHOLD_SCALE	7
#define TOF_DEFAULT_TX_THRESHOLD_SCALE_5G_20M 7
#define TOF_DEFAULT_TX_THRESHOLD_SCALE_2G_20M 7
#define TOF_DEFAULT_TX_THRESHOLD_LOG2	7
#define TOF_DEFAULT_TX_THRESHOLD_LOG2_5G_20M 7
#define TOF_DEFAULT_TX_THRESHOLD_LOG2_2G_20M 6
#define TOF_DEFAULT_RX_THRESHOLD_SCALE	3
#define TOF_DEFAULT_RX_THRESHOLD_SCALE_5G_20M 3
#define TOF_DEFAULT_RX_THRESHOLD_SCALE_2G_20M 6
#define TOF_DEFAULT_RX_THRESHOLD_LOG2	7
#define TOF_DEFAULT_RX_THRESHOLD_LOG2_5G_20M 7
#define TOF_DEFAULT_RX_THRESHOLD_LOG2_2G_20M 6
#define TOF_DEFAULT_RX_THRESHOLD_BITFLIP	10
#define TOF_DEFAULT_RX_THRESHOLD_SNR		0
#define TOF_DEFAULT_RX_THRESHOLD_ACS_RSSI	-60
#define TOF_DEFAULT_RX_THRESHOLD_ACS_DELTA_RSSI	12
#define TOF_DEFAULT_RX_SMOOTH_WIN_EN		1
#define TOF_DEFAULT_WINDOW_LEN_5G_20 25
#define TOF_DEFAULT_WINDOW_OFFSET_5G_20 16
#define TOF_DEFAULT_WINDOW_LEN_2G 25
#define TOF_DEFAULT_WINDOW_OFFSET_2G 16
#define TOF_MIN_CORR_PEAK_PWR 4000
#define TOF_DEFAULT_FTMCNT_20M		12
#define TOF_DEFAULT_FTMCNT_40M		10
#define TOF_DEFAULT_FTMCNT_80M		6
#define TOF_DEFAULT_FTMCNT_SEQ		3
#define TOF_DEFAULT_EMU_DELAY		0
#define TOF_DEFAULT_CORE_MASK		0xFF
#define TOF_REQ_RETRY_CNT	6
#define TOF_SEQ_SHIFT		9
#define TOF_VHT_ACK_SHIFT	8
#define TOF_HT_ACK_SHIFT	10
#define TOF_CMD_MASK		0xff
#define TOF_RSP_MASK		0xff
#define TOF_RX			1
#define TOF_RESET		0
#define TOF_SUCCESS		0x88

#ifdef BCMDBG
#define TOF_MCMD_TIMEOUT 800
#else
#define TOF_MCMD_TIMEOUT 400
#endif /* BCMDBG */

#define TOF_ACK_TS_TIMEOUT 40

#define TOF_REQ_START_RETRY_DUR 3000
#define TOF_REQ_START_DUR		10
#define CORE0_K_TOF_H_BITS	10
#define CHNSM_K_TOF_H_BITS	24
#define TOF_RTT_MINVAL		-10
#define TOF_RTT_MAXVAL		2000
#define TOF_TS_RANGE		300

/* All these parameters are related to CLK */
#define TOF_TICK_TO_NS(val, Tq)	(((uint64)(val)*(Tq)+ (1 << (TOF_SHIFT-1))) >> TOF_SHIFT)
/* Distance =  (C (speed of light) * Delta Time	(1 ns)) /2 = (3 * 10^8)* (10 ^ -9) /2
 * = 0.15 m
 * For 1 ns equals 0.15 meter, to keep the accuray, times 15 then divide by 100.
 * Distance expressed as Q4 number(i.e 16 => 1m)
 * We prefer to round down
*/
#define TOF_NS_TO_16THMETER(val, d)	((((val)*15)<<4)/((int32)d*100))

/**********************************************************
 * Function Purpose:
 * Call back interface for proximity detection notificaiton.
 * PARAMETERS:
 * wlc_info_t pointer
 * ether_addr pointer to ethernet addr
 * result
 * status
 * body
 * body_len
 * Return value:
 * If sucess it returns positive else negative.
*************************************************************
*/
typedef int (*notifypd)(void *ctx, struct ether_addr *ea, uint result, uint status,
	uint8 *body, uint body_len);

/***************************************************************************************
 * Proximity detection service supplies transmit funciton pointer to rssi pd method, so that,
 * once the action frame is ready it can pass it to this interface. The reason is that the
 * application may transport TLVs in the proxmity action frames. Application can pass the TLV
 * to pdsvc, and pdsvc can copy the TLVs in the action frames.
**************************************************************************************
*/
typedef int (*transmitaf)(wlc_pdsvc_info_t *const pdsvc, wl_action_frame_t *af,
	ratespec_t rate_override, pkcb_fn_t fn, struct ether_addr *addr);

/***************************************************************************************
 * Proximity detection service supplies AVB timer factor the factor = (2 * Divior * 1000000)/VCO.
 * The factor for 4335b0 and 4335c0 is 6.19834710... to keep good accuracy. Left Shift it 15 bit.
 * After calculation, right shift the result 15 bit.
**************************************************************************************
*/
typedef uint32 (*avbtimerfactorf)(wlc_pdsvc_info_t *const pdsvc, uint8 shift, uint32 *ki,
	uint32 *kt);

typedef int (*pdsvc_func_t)(wlc_info_t *wlc, uint8 action, chanspec_t chanspec,
	struct ether_addr *addr, int8 frmcnt, int8 retrycnt, int timeout, uint32 flags);

typedef struct pdsvc_funcs
{
	transmitaf	txaf;		/* action frame transmit function */
	notifypd	notify;		/* result notification function */
	avbtimerfactorf	clock_factor;	/* get avb timer clock factor function */
	void		*notifyptr;	/* notify function pointer */
} pdsvc_funcs_t;

typedef struct pdsvc_payload
{
	uint16	len;
	char *	data;
} pdsvc_payload_t;

pdsvc_func_t wlc_pdsvc_register(wlc_info_t *wlc, wlc_bsscfg_t *bsscfg, notifypd notify,
	void *notifyptr, int8 fmtcnt, struct ether_addr *allow_addr, bool setonly, uint32 flags);
int wlc_pdsvc_deregister(wlc_info_t *wlc, pdsvc_func_t funcp);

#endif /* _wlc_pddefs_h */
