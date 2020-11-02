/*
 * 11g/n shared protection module private APIs
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
 * $Id: wlc_prot_priv.h 708017 2017-06-29 14:11:45Z $
 */

#ifndef _wlc_prot_priv_h_
#define _wlc_prot_priv_h_

#include <wlc_prot.h>

/* module private states */
typedef struct {
	wlc_info_t *wlc;
	uint16 cfg_offset;	/* bss_prot_cfg_t offset in bsscfg cubby client */
} wlc_prot_info_priv_t;

/* module states layout */
typedef struct {
	wlc_prot_info_t pub;
	wlc_prot_info_priv_t priv;
} wlc_prot_t;
/* module states size */
#define WLC_PROT_SIZE	(sizeof(wlc_prot_t))
/* moudle states location */
extern uint16 wlc_prot_info_priv_offset;
#define WLC_PROT_INFO_PRIV(prot) ((wlc_prot_info_priv_t *) \
				  ((uintptr)(prot) + wlc_prot_info_priv_offset))

/* bsscfg private states */
typedef	struct {
	int8	overlap;		/* Overlap BSS/IBSS protection for both 11g and 11n */
} bss_prot_cfg_t;

/* bsscfg states layout */
typedef struct {
	wlc_prot_cfg_t pub;
	bss_prot_cfg_t priv;
} bss_prot_t;
/* bsscfg states size */
#define BSS_PROT_SIZE	(sizeof(bss_prot_t))
/* bsscfg states location */
#define BSS_PROT_CUBBY(prot, cfg) ((bss_prot_t *) \
				   BSSCFG_CUBBY(cfg, WLC_PROT_INFO_CFGH(prot)))
#define BSS_PROT_CFG(prot, cfg) ((bss_prot_cfg_t *) \
				 ((uintptr)BSS_PROT_CUBBY(prot, cfg) +	\
				  WLC_PROT_INFO_PRIV(prot)->cfg_offset))

/* update configuration */
extern void wlc_prot_cfg_upd(wlc_prot_info_t *prot, wlc_bsscfg_t *cfg, uint idx, int val);
/* wlc_prot_cfg_upd() idx */
#define WLC_PROT_OVERLAP	1
#define WLC_PROT_SHORTPREAMBLE	2

/* configuration init */
extern void wlc_prot_cfg_init(wlc_prot_info_t *prot, wlc_bsscfg_t *cfg);

/* protection init */
extern void wlc_prot_init(wlc_prot_info_t *prot, wlc_bsscfg_t *cfg);

/* propagate condition */
extern void wlc_prot_cond_set(wlc_info_t *wlc, wlc_bsscfg_t *cfg, uint coff, bool set,
	uint8 *(*cb)(void *prot, wlc_bsscfg_t *cfg, uint coff), void *prot);

/* check if associated scbs have the flags set */
extern bool wlc_prot_scb_scan(wlc_info_t *wlc, wlc_bsscfg_t *cfg,
	uint32 flagmask, uint32 flagvalue);

#endif /* _wlc_prot_priv_h_ */
