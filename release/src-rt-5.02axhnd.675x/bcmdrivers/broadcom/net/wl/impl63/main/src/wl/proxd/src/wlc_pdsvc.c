/*
 * Proximity detection service layer implementation for Broadcom 802.11 Networking Driver
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
 * $Id: wlc_pdsvc.c 788034 2020-06-18 14:32:28Z $
 */

#include <typedefs.h>
#include <bcmdefs.h>
#include <bcmdevs.h>
#include <osl.h>
#include <sbchipc.h>
#include <bcmutils.h>
#include <siutils.h>
#include <bcmendian.h>
#include <802.11.h>
#include <wlioctl.h>
#include <bcmwpa.h>
#include <d11.h>
#include <wlc_cfg.h>
#include <wlc_pub.h>
#include <wlc_rate.h>
#include <wlc_key.h>
#include <wlc_channel.h>
#include <wlc_bsscfg.h>
#include <wlc.h>
#include <wlc_scb.h>
#include <wl_export.h>
#include <wlc_bmac.h>
#include <wlc_hw.h>
#include <wlc_hw_priv.h>
#include <hndpmu.h>
#include <wlc_pcb.h>
#include <wlc_event_utils.h>
#include <wlc_hrt.h>

#include <wlc_pdsvc.h>
#include <wlc_pddefs.h>
#include <wlc_pdmthd.h>
#ifdef WL_RANDMAC
#include <wlc_randmac.h>
#endif /* WL_RANDMAC */
#include "pdsvc.h"
#include "pdftm.h"
#include "pdburst.h"

#include <phy_tof_api.h>
#ifdef WLSLOTTED_BSS
#include <wlc_slotted_bss.h>
#endif /* WLSLOTTED_BSS */
#include <wlc_dump.h>

#if defined(WLRSDB) && defined(WL_RANGE_SEQ)
#include<wlc_rsdb.h>
#endif /* WLRSDB && WL_RANGE_SEQ */
#include <wlc_ratelinkmem.h>
#include <wlc_rate_sel.h>

#define PROXD_NAME "proxd"

#define ELEM_SWAP(a,b) {uint32 t = (a); (a) = (b); (b) = t;}

typedef struct wlc_pdsvc_config {
	uint16	mode;
	void *method_params; /* points to current method params structure */
	struct ether_addr mcastaddr;	/* Multicast address */
	struct ether_addr bssid;	/* BSSID */
} wlc_pdsvc_config_t;

/* This is the mainstructure of proximity detection service */
struct wlc_pdsvc_info {
	uint32 signature;
	wlc_info_t *wlc;
	wlc_bsscfg_t *bsscfg;
	int cfgh;				/* bsscfg cubby handle */
	uint16 method;
	uint16 state;
	wlc_pdsvc_config_t config;
	pdsvc_funcs_t funcs;
	pdmthd_if_t *cur_mif;  /* current method interface */
	pdsvc_payload_t payload;
	uint32 fvco;
	uint32 pllreg;
	proxd_method_create  method_create_fn[PROXD_MAX_METHOD_NUM];
	uint32 ki;
	uint32 kt;
	void * ranging;
	pdftm_t *ftm;
	wl_proxd_params_tof_tune_t *tunep;
	uint64	clkfactor; /* clock factor */
	uint16	pwrflag;
};

/* Proximity specific private datas in bsscfg */
/* XXX allocate the struct and reserve a pointer to the struct in the bsscfg
 * as the bsscfg cubby when this structure grows larger ...
 */
typedef struct {
	mbool flags;	/* flags for proximity detection */
} bss_proxd_cubby_t;

#define PROXD_FLAG_TXPWR_OVERRIDE	0x1	/* override tx power of the transmit frames */

#define PROXD_BSSCFG_CUBBY(pdsvc_info, cfg) \
	((bss_proxd_cubby_t *)BSSCFG_CUBBY(cfg, (pdsvc_info)->cfgh))

/* IOVAR declarations */
enum {
	/*
	 IOV: IOV_PROXD
	 Purpose: This IOVAR enables/disables proximity detection and sets mode.
	*/
	IOV_PROXD		= 0,
	/*
	 IOV: IOV_PROXD_PARAMS
	 Purpose: This IOVAR sets/gets the parameters for the specific method
	*/
	IOV_PROXD_PARAMS	= 1,
	/*
	 IOV: IOV_PROXD_BSSID
	 Purpose: This IOVAR sets/gets BSSID of proximity detection frames
	*/
	IOV_PROXD_BSSID		= 2,
	/*
	 IOV: IOV_PROXD_MCASTADDR
	 Purpose: This IOVAR sets/gets multicast address of proximity detection frames
	*/
	IOV_PROXD_MCASTADDR	= 3,
	/*
	 IOV: IOV_PROXD_FIND
	 Purpose: Start proximity detection
	*/
	IOV_PROXD_FIND		= 4,
	/*
	 IOV: IOV_PROXD_STOP
	 Purpose: Stop proximity detection
	*/
	IOV_PROXD_STOP		= 5,
	/*
	 IOV: IOV_PROXD_STATUS
	 Purpose: Get proximity detection status
	*/
	IOV_PROXD_STATUS	= 6,
	/*
	 IOV: IOV_PROXD_MONITOR
	 Purpose: Start proximity detection monitor mode
	*/
	IOV_PROXD_MONITOR	= 7,
	/*
	 IOV: IOV_PROXD_PAYLOAD
	 Purpose: Get/Set proximity detection payload content
	*/
	IOV_PROXD_PAYLOAD	= 8,
	/*
	 IOV: IOV_PROXD_COLLECT
	*/
	IOV_PROXD_COLLECT = 9,
	/*
	 IOV: IOV_PROXD_TUNE
	*/
	IOV_PROXD_TUNE = 10,
	/*
		Minimum time required between two consecutive measurement frames (for target)
	*/
	IOV_FTM_PERIOD = 11,
	/*
		REPORT
	*/
	IOV_PROXD_REPORT = 12,
	/*
	 IOV: IOV_AVB_LOCAL_TIME = 13
	*/
	IOV_AVB_LOCAL_TIME = 13,
	/*
		SEQ
	*/
	IOV_TOF_SEQ = 14
};

/* Iovars */
static const bcm_iovar_t  wlc_proxd_iovars[] = {
	{"proxd", IOV_PROXD, 0, 0, IOVT_BUFFER, sizeof(uint16)*2},
#ifdef TOF_DBG
	{"proxd_collect", IOV_PROXD_COLLECT, (IOVF_GET_UP | IOVF_SET_UP | IOVF_GET_CLK),
	IOVT_BUFFER, sizeof(wl_proxd_collect_data_t)},
#endif /* TOF_DBG */
	{"proxd_tune", IOV_PROXD_TUNE, (IOVF_GET_UP | IOVF_SET_UP),
	0, IOVT_BUFFER, sizeof(wl_proxd_params_iovar_t)},
#ifdef WL_PROXD_AVB_TS
	{"avb_local_time", IOV_AVB_LOCAL_TIME, 0, IOVT_BUFFER, sizeof(uint32)},
#endif /* WL_PROXD_AVB_TS */
	{NULL, 0, 0, 0, 0, 0}
};

#ifdef BCMDBG
static void wlc_proxd_bsscfg_cubby_dump(void *ctx, wlc_bsscfg_t *cfg, struct bcmstrbuf *b);
#else
#define wlc_proxd_bsscfg_cubby_dump NULL
#endif // endif

#ifdef WL_FTM
static void proxd_bss_updown(void *ctx, bsscfg_up_down_event_data_t *evt);
static wl_proxd_params_tof_tune_t *proxd_init_tune(wlc_info_t *wlc);
static int pdburst_get_tune(wlc_info_t *, wl_proxd_params_tof_tune_t *, void *, uint);
static int pdburst_set_tune(wl_proxd_params_tof_tune_t *tune, void *pbuf, uint len);
static int wlc_proxd_wlc_up(void *context);
static int wlc_proxd_wlc_down(void *context);
#endif /* WL_FTM */

/* This includes the auto generated ROM IOCTL/IOVAR patch handler C source file (if auto patching is
 * enabled). It must be included after the prototypes and declarations above (since the generated
 * source file may reference private constants, types, variables, and functions).
 */
#include <wlc_patch.h>

/* bsscfg cubby */
static int
wlc_proxd_bsscfg_cubby_init(void *ctx, wlc_bsscfg_t *cfg)
{
	wlc_pdsvc_info_t *pdsvc = (wlc_pdsvc_info_t *)ctx;
	wlc_info_t *wlc = pdsvc->wlc;
	bss_proxd_cubby_t *proxd_bsscfg_cubby;
	uint8 gmode = GMODE_AUTO;
	int err;

	ASSERT(cfg != NULL);

	proxd_bsscfg_cubby = (bss_proxd_cubby_t *)PROXD_BSSCFG_CUBBY(pdsvc, cfg);

	bzero((void *)proxd_bsscfg_cubby, sizeof(*proxd_bsscfg_cubby));

	if (!BSS_PROXD_ENAB(wlc, cfg)) {
		return BCME_OK;
	}

	if (BAND_ENABLED(wlc, BAND_2G_INDEX)) {
		gmode = wlc->bandstate[BAND_2G_INDEX]->gmode;
	}
	if (gmode == GMODE_LEGACY_B) {
		WL_ERROR(("wl%d: %s: gmode cannot be GMODE_LEGACY_B\n", wlc->pub->unit,
			__FUNCTION__));
		err = BCME_BADRATESET;
		goto exit;
	}

	if ((err = wlc_bsscfg_rateset_init(wlc, cfg, WLC_RATES_OFDM,
			WL_BW_CAP_40MHZ(wlc->band->bw_cap) ? CHSPEC_WLC_BW(wlc->home_chanspec) : 0,
			BSS_N_ENAB(wlc, cfg))) != BCME_OK) {
		WL_ERROR(("wl%d: %s: failed rateset int\n", wlc->pub->unit, __FUNCTION__));
		goto exit;
	}

	/* set bsscfg to IBSS */
	cfg->current_bss->bss_type = DOT11_BSSTYPE_INDEPENDENT;

	/* Do not particiate in mchan scheduler since
	   proxd has its own scheduler for channel access
	*/
	/* bsscfg->flags |= WLC_BSSCFG_MCHAN_DISABLE; */

	/* Initialize default flags if needed */

	/* Set BSSID for this bsscfg */
	bcopy(&pdsvc->config.bssid, &cfg->BSSID, ETHER_ADDR_LEN);

	/* if the driver is not up, return here.
	 * BSSID to AMT will be set during the driver up later.
	 * This would fall into one of the following two cases.
	 *  1) wl is down from Host
	 *  2) radio is down due to mpc
	 */
	if (!wlc->pub->up)
		goto exit;

	ASSERT(wlc->clk);

	/* Set BSSID to AMT (or RCMTA) */
	wlc_set_bssid(cfg);

exit:
	return err;
}

static void
wlc_proxd_bsscfg_cubby_deinit(void *ctx, wlc_bsscfg_t *cfg)
{
}

#ifdef BCMDBG
static void
wlc_proxd_bsscfg_cubby_dump(void *ctx, wlc_bsscfg_t *cfg, struct bcmstrbuf *b)
{
	wlc_pdsvc_info_t *pdsvc = (wlc_pdsvc_info_t *)ctx;
	bss_proxd_cubby_t *proxd_bsscfg_cubby;

	ASSERT(pdsvc != NULL);
	ASSERT(cfg != NULL);

	proxd_bsscfg_cubby = (bss_proxd_cubby_t *)PROXD_BSSCFG_CUBBY(pdsvc, cfg);

	bcm_bprintf(b, "proxd bss flags: %x\n", proxd_bsscfg_cubby->flags);
}
#endif // endif

#ifdef WL_FTM
/* wl proxd_tune get command */
static int
pdburst_get_tune(wlc_info_t *wlc, wl_proxd_params_tof_tune_t *tune, void *pbuf, uint len)
{
	wl_proxd_params_tof_tune_t *tunep = pbuf;
	uint32 *kip = NULL, *ktp = NULL;

	if (len < sizeof(wl_proxd_params_tof_tune_t)) {
		return BCME_BUFTOOSHORT;
	}

	memcpy(pbuf, tune, sizeof(wl_proxd_params_tof_tune_t));
	if (!tunep->Ki)
		kip = &tunep->Ki;
	if (!tunep->Kt)
		ktp = &tunep->Kt;
	if (kip || ktp) {
		wl_proxd_session_flags_t flags;
		const pdburst_config_t *configp;
		/* get the default burst config */
		configp = pdftm_get_burst_config(wlc->pdsvc_info->ftm, NULL, &flags);
		if (configp) {
			ratespec_t ackrspec;
			if (!tune->vhtack) {
				ackrspec = LEGACY_RSPEC(PROXD_DEFAULT_TX_RATE);
			} else {
				ackrspec = LEGACY_RSPEC(PROXD_DEFAULT_TX_RATE) |
				        WL_RSPEC_ENCODE_VHT;
			}
			wlc_phy_kvalue(WLC_PI(wlc), configp->chanspec,
				proxd_get_ratespec_idx(configp->ratespec, ackrspec),
				kip, ktp,
				((flags & WL_PROXD_SESSION_FLAG_SEQ_EN) ? WL_PROXD_SEQEN : 0));
		}
	}
	return BCME_OK;
}

/* wl proxd_tune set command */
static int
pdburst_set_tune(wl_proxd_params_tof_tune_t *tune, void *pbuf, uint len)
{
	wl_proxd_params_tof_tune_t *tunep = pbuf;

	if (len < sizeof(wl_proxd_params_tof_tune_t)) {
		return BCME_BUFTOOSHORT;
	}

	memcpy(tune, pbuf, sizeof(wl_proxd_params_tof_tune_t));
	if (!(tunep->setflags & WL_PROXD_SETFLAG_K)) {
		tune->Ki = tune->Kt = 0;
	}
	tunep->setflags &= ~WL_PROXD_SETFLAG_K;

	return BCME_OK;
}
#endif /* WL_FTM */

/* Iovar processing: Each proximity method is created, deleted, and changes it state by iovars */
static int
wlc_proxd_doiovar(void *ctx, uint32 actionid,
	void *params, uint p_len, void *arg, uint a_len, uint val_size, struct wlc_if *wlcif)
{
	wlc_pdsvc_info_t *pdsvc_info = (wlc_pdsvc_info_t *)ctx;
	uint16 method = 0;
	wlc_info_t *wlc;
	int err = BCME_OK;
#ifdef WL_PROXD_AVB_TS
	uint32 tx, rx;
	wlc_hw_info_t *wlc_hw;
	osl_t *osh;
	d11regs_t *regs;
	uint32 clkst;
	uint32 macctrl1;
#endif /* WL_PROXD_AVB_TS */
#ifdef WL_FTM
	uint16 iov_version = 0;
#endif // endif

	ASSERT(pdsvc_info != NULL);
	CHECK_SIGNATURE(pdsvc_info, WLC_PDSVC_SIGNATURE);
	ASSERT(pdsvc_info->wlc != NULL);
	wlc = pdsvc_info->wlc;

	/* Process IOVARS */

#ifdef WL_FTM
	/* handle/dispatch new API - this needs cleanup */
	do {
		wlc_bsscfg_t *bsscfg;
		wl_proxd_iov_t *iov;
		uint iov_len;
		wl_proxd_cmd_t iov_cmd;
		wl_proxd_method_t iov_method;
		wl_proxd_session_id_t iov_sid;
		wl_proxd_iov_t *rsp_iov;
		uint rsp_tlvs_len;

		if (IOV_ID(actionid) != IOV_PROXD)
			break;

		if (p_len < WL_PROXD_IOV_HDR_SIZE)
			break;

		iov = (wl_proxd_iov_t *)params;
		iov_version = ltoh16_ua(&iov->version);

		if (iov_version < WL_PROXD_API_MIN_VERSION)
			break;

		/* check length - iov_len includes ver and len */
		iov_len = ltoh16_ua(&iov->len);
		if (p_len < iov->len) {
			err = BCME_BADLEN;
			break;
		}

		/* all other commands except get version need exact match on version */
		iov_cmd = (wl_proxd_cmd_t) ltoh16_ua(&iov->cmd);

		if (iov_version != WL_PROXD_API_VERSION && iov_cmd != WL_PROXD_CMD_GET_VERSION) {
			err = BCME_UNSUPPORTED;
			break;
		}

		iov_method = (wl_proxd_method_t) ltoh16_ua(&iov->method);
		iov_sid = (wl_proxd_session_id_t)ltoh16_ua(&iov->sid);

		/* set up the result */
		rsp_iov = (wl_proxd_iov_t *)arg;
		if (a_len < WL_PROXD_IOV_HDR_SIZE) {
			err = BCME_BUFTOOSHORT;
			break;
		}

		if (iov_len < WL_PROXD_IOV_HDR_SIZE) {
			err = BCME_BUFTOOSHORT;
			break;
		}

		/* init response - length may be adjusted later */
		memcpy(rsp_iov, iov, WL_PROXD_IOV_HDR_SIZE);
		htol16_ua_store(WL_PROXD_IOV_HDR_SIZE, &rsp_iov->len);

		bsscfg = wlc_bsscfg_find_by_wlcif(wlc, wlcif);
		ASSERT(bsscfg != NULL);

		switch (iov_method) {
		case WL_PROXD_METHOD_NONE:	/* handled by svc */
			/* session id not used - perhaps ignore/log warning? */
			if (iov_sid != WL_PROXD_SESSION_ID_GLOBAL) {
				err = BCME_BADARG;
				break;
			}
			break;
		case WL_PROXD_METHOD_FTM:	/* handled by method */
			if (IOV_ISSET(actionid)) {
				err = pdftm_set_iov(pdsvc_info->ftm, bsscfg,
					iov_cmd, iov_sid, iov->tlvs,
					iov_len - WL_PROXD_IOV_HDR_SIZE);
			} else {
				/* TODO: EXCAST to (int *) has to be removed
				* when int changes are made in pdftm_get_iov
				*/
				err = pdftm_get_iov(pdsvc_info->ftm, bsscfg,
					iov_cmd, iov_sid, iov->tlvs,
					iov_len - WL_PROXD_IOV_HDR_SIZE,
					a_len - WL_PROXD_IOV_HDR_SIZE,
					rsp_iov->tlvs, (int *)&rsp_tlvs_len);
				ASSERT((rsp_tlvs_len + WL_PROXD_IOV_HDR_SIZE) <= a_len);
				htol16_ua_store(rsp_tlvs_len + WL_PROXD_IOV_HDR_SIZE,
					&rsp_iov->len);
			}

			if (err != BCME_OK)
				break;

			break;
		default:
			err = BCME_UNSUPPORTED;
			break;

		}

		/* indicate errors that are not BCME_... , loses detail. */
		if (err != BCME_OK) {
			 if (!VALID_BCMERROR(err))
				err = BCME_ERROR;
		}
	} while (0);

	if (iov_version == WL_PROXD_API_VERSION)
		goto done;

#endif /* WL_FTM */

	switch (actionid) {
#if defined(TOF_DBG) && defined(WL_FTM)
	case IOV_GVAL(IOV_PROXD_COLLECT):
	case IOV_SVAL(IOV_PROXD_COLLECT):
		/* proximity detection should be in idle state */
		if (pdsvc_info->state != 0)
			err = BCME_EPERM;
		else if (p_len < (uint)sizeof(wl_proxd_collect_query_t))
			err = BCME_BADARG;
		else {
			uint16 len;
			wl_proxd_collect_query_t quety;
			bcopy(params, &quety, sizeof(quety));

			if (pdsvc_info->cur_mif) {
			} else if (pdsvc_info->ftm) {
				err = pdburst_collection(wlc, NULL, &quety, arg, a_len, &len);
			} else
				err = BCME_NOTREADY;
		}
		break;
#endif /* TOF_DBG */
	case IOV_GVAL(IOV_PROXD_TUNE):
		if (p_len >= (uint)sizeof(method))
			bcopy(params, &method, sizeof(method));

		if (p_len < sizeof(wl_proxd_params_iovar_t)) {
			return BCME_BUFTOOSHORT;
		}

		if (pdsvc_info->cur_mif) {
		} else if (pdsvc_info->ftm && pdsvc_info->tunep) {
			err = pdburst_get_tune(wlc, pdsvc_info->tunep,
				((uint8*) arg + OFFSETOF(wl_proxd_params_iovar_t, u.tof_tune)),
				p_len - OFFSETOF(wl_proxd_params_iovar_t, u.tof_tune));
		}
		else
			err = BCME_NOTREADY;
		break;
	case IOV_SVAL(IOV_PROXD_TUNE):
		if (p_len >= (uint)sizeof(method))
			bcopy(params, &method, sizeof(method));

		if (pdsvc_info->cur_mif) {
		} else if (pdsvc_info->ftm && pdsvc_info->tunep) {
			err = pdburst_set_tune(pdsvc_info->tunep,
				((uint8*) params + OFFSETOF(wl_proxd_params_iovar_t, u.tof_tune)),
				p_len - OFFSETOF(wl_proxd_params_iovar_t, u.tof_tune));
		}
		else
			err = BCME_NOTREADY;
		break;
#ifdef WL_PROXD_AVB_TS
	case IOV_GVAL(IOV_AVB_LOCAL_TIME):
		if (p_len < sizeof(uint32))
			return BCME_BUFTOOSHORT;

		/* proximity detection should be in idle state */
		if (pdsvc_info->state != 0)
			return BCME_EPERM;

		wlc_hw = wlc->hw;
		osh = wlc_hw->osh;
		regs = wlc_hw->regs;

		/* Read the clock state and MAC control registers */
		wlc_get_avb_timer_reg(wlc->hw, &clkst, &macctrl1);
		wlc_hw->machwcap1 = R_REG(wlc_hw->osh, D11_MacHWCap1(wlc_hw));
		WL_ERROR(("%s clkst 0x%x,  macctrl1 0x%x, machwcap1 0x%x \n",
			__FUNCTION__, clkst, macctrl1, wlc_hw->machwcap1));
		wlc_enable_avb_timer(wlc->hw, TRUE);
		wlc_enable_avb_timer_war(wlc->hw, TRUE);
		wlc_get_avb_timestamp(wlc->hw, &tx, &rx);
		WL_ERROR(("%s AVBTxTimeStamp %u   AVBRxTimeStamp %u \n", __FUNCTION__, tx, rx));

		((uint32 *)arg)[0] = tx;
		break;
#endif /* WL_PROXD_AVB_TS */

	default:
		err = BCME_UNSUPPORTED;
		break;
	}

#ifdef WL_FTM
done:
#endif /* WL_FTM */
	return err;
}

#if defined(WL_FTM)
/* Get AVB clock factor
 * AVB timer factor  = (2 * Divior * 1000000)/VCO.
 * The factor for 4335b0 and 4335c0 is 6.19834710... to keep good accuracy. Left Shift it 15 bit.
 * After calculation, right shift the result 15 bit.
*/
static uint32
wlc_proxd_AVB_clock_factor(wlc_pdsvc_info_t* pdsvc, uint8 shift, uint32 *ki, uint32 *kt)
{
	uint32 factor;

	ASSERT(pdsvc != NULL);
	if (CHIPID(pdsvc->wlc->pub->sih->chip) == BCM63178_CHIP_ID) {
		/* Considering the default settings for 63178/47622 avoidance mode */
		/* i_pdiv (pre-divider) = 1 */
		/* FVCO = xtal (50) * (integer + fractional) */
		/* Integer = i_ndiv_int [9:0] of PLL Control 2 = 58 */
		/* Fractional = i_ndiv_frac[19:0] hex2dec('23DD4')/2^20 = 0.14009476 */
		/* FVCO = 50 * 58.14009476 / 1 = 2907.00474MHz */
		/* mdiv = 12; 2907.005/12 = 242.2504 */
		/* factor = 1000*2^15/242.2504 */
		/** AVB Clock = 242.2504 */
		factor = 135265;
	} else if ((BCM43684_CHIP(pdsvc->wlc->pub->sih->chip))) {
		/* Considering the default settings for 43684 avoidance mode */
		/* i_pdiv (pre-divider) = 1 */
		/* FVCO = xtal (54) * (integer + fractional) */
		/* Integer = i_ndiv_int [9:0] of PLL Control 2 = 53 */
		/* Fractional = i_ndiv_frac[19:0] hex2dec('D55AC')/2^20 = 0.833415985 */
		/* FVCO = 54 * 53.833415985 / 1 = 2907.004463MHz */
		/* mdiv = 12; 2907.005/12 = 242.2504 */
		/* factor = 1000*2^15/242.2504 */
		/** AVB Clock = 242.2504 */
		factor = 135265;
	} else
	if ((CHIPID(pdsvc->wlc->pub->sih->chip)) == BCM4360_CHIP_ID ||
		(CHIPID(pdsvc->wlc->pub->sih->chip)) == BCM4352_CHIP_ID ||
		(CHIPID(pdsvc->wlc->pub->sih->chip)) == BCM43460_CHIP_ID ||
		(CHIPID(pdsvc->wlc->pub->sih->chip)) == BCM43602_CHIP_ID ||
		(CHIPID(pdsvc->wlc->pub->sih->chip)) == BCM4347_CHIP_ID ||
		D11REV_IS(pdsvc->wlc->pub->corerev, 82)) {

		factor = ((pdsvc->pllreg * 1000) << shift);
		factor = factor / pdsvc->fvco;
	} else {
		factor = (((pdsvc->pllreg & PMU1_PLL0_PC1_M1DIV_MASK) * 1000 * 2) << shift);
		factor = factor / pdsvc->fvco;
	}

	if (ki && pdsvc->ki)
		*ki = pdsvc->ki;
	if (kt && pdsvc->kt)
		*kt = pdsvc->kt;

	PROXD_TRACE(("Shift:%d, pllreg:%x , fvco:%d, factor:%d\n",
		shift, pdsvc->pllreg, pdsvc->fvco, factor));

	return factor;
}
#endif /* WL_FTM */
wlc_pdsvc_info_t *
BCMATTACHFN(wlc_proxd_attach)(wlc_info_t *wlc)
{
	wlc_pdsvc_info_t *pdsvc = NULL;
	int err;

	ASSERT(wlc != NULL);

	/* Allocate heap space for wlc_pdsvc_info_t */
	pdsvc = MALLOC(wlc->osh, sizeof(wlc_pdsvc_info_t));
	if (pdsvc == NULL) {
		WL_ERROR(("wl%d: %s: MALLOC allocation is failed %d bytes \n",
			wlc->pub->unit, __FUNCTION__, MALLOCED(wlc->pub->osh)));
		goto fail;
	}

	/* Proximity detection is disabled in default */
	wlc->pub->_proxd = FALSE;

	/* Clear the allocated space */
	bzero(pdsvc, sizeof(wlc_pdsvc_info_t));
	ASSIGN_SIGNATURE(pdsvc, WLC_PDSVC_SIGNATURE);

	/* save the wlc reference */
	pdsvc->wlc = wlc;
	/* reserve cubby in the bsscfg container for per-bsscfg private data */
	if ((pdsvc->cfgh = wlc_bsscfg_cubby_reserve(wlc, sizeof(bss_proxd_cubby_t),
		wlc_proxd_bsscfg_cubby_init, wlc_proxd_bsscfg_cubby_deinit,
		wlc_proxd_bsscfg_cubby_dump, pdsvc)) < 0) {
		WL_ERROR(("wl%d: %s: wlc_bsscfg_cubby_reserve() failed\n",
		    wlc->pub->unit, __FUNCTION__));
		goto fail;
	}

	/* Provide wlc_proxd_stop() for wl down callback, so that
	 * the proximity detection to be stopped upon diver down.
	 * This should be done along with PM mode implementation,
	 * otherwise the proximity detection will be stopped on
	 * entering sleep due to MPC.
	 */
	err = wlc_module_register(wlc->pub, wlc_proxd_iovars, PROXD_NAME, (void *)pdsvc,
		wlc_proxd_doiovar, NULL, wlc_proxd_wlc_up, wlc_proxd_wlc_down);

	if (err != BCME_OK) {
		WL_ERROR(("wl%d: %s: wlc_module_register() failed with status %d\n",
			wlc->pub->unit, __FUNCTION__, err));
		goto fail;
	}

	err = si_pmu_fvco_pllreg(wlc->hw->sih, &pdsvc->fvco, &pdsvc->pllreg);
	if (err != BCME_OK) {
		WL_ERROR(("wl%d: %s: get fvco failed with error %d\n",
			wlc->pub->unit, __FUNCTION__, err));
		goto fail;
	}

#ifdef WL_FTM
	pdsvc->ftm = pdftm_attach(wlc, pdsvc);
	if (!pdsvc->ftm)  /* callee logged failure */
		goto fail;

	/* wlc->pub->_proxd = TRUE; */

	err = wlc_bsscfg_updown_register(wlc, proxd_bss_updown, pdsvc);
	if (err != BCME_OK) {
		WL_ERROR(("wl%d: %s: bsscfg up/down register failed with error %d\n",
			wlc->pub->unit, __FUNCTION__, err));
		goto fail;
	}

	pdsvc->tunep = proxd_init_tune(wlc);
	if (!pdsvc->tunep) {
		WL_ERROR(("wl%d: %s: malloc tune failed\n", wlc->pub->unit, __FUNCTION__));
		goto fail;
	}

#if defined(BCMDBG)
	err = wlc_dump_register(wlc->pub, PROXD_NAME, (dump_fn_t)proxd_dump, pdsvc);
	if (err != BCME_OK) {
		WL_ERROR(("wl%d: %s: dump register failed with error %d\n",
			wlc->pub->unit, __FUNCTION__, err));
		goto fail;
	}
#endif // endif

#endif /* WL_FTM */

	return pdsvc;

fail:
	if (pdsvc != NULL) {
		if (pdsvc->ftm) {
			MODULE_DETACH(pdsvc->ftm, pdftm_detach);
		}
		(void)wlc_module_unregister(wlc->pub, PROXD_NAME, pdsvc);
		if (pdsvc->tunep)
			MFREE(wlc->osh, pdsvc->tunep, sizeof(wl_proxd_params_tof_tune_t));
		MFREE(wlc->osh, pdsvc, sizeof(wlc_pdsvc_info_t));
	}
	return NULL;
}

/* Detach the proximity service from WLC */
int
BCMATTACHFN(wlc_proxd_detach) (wlc_pdsvc_info_t *const pdsvc)
{
	int callbacks = 0;
	wlc_info_t *wlc;
	if (pdsvc == NULL)
		return callbacks;

	CHECK_SIGNATURE(pdsvc, WLC_PDSVC_SIGNATURE);
	wlc = pdsvc->wlc;

	ASSIGN_SIGNATURE(pdsvc, 0);

#ifdef WL_FTM
	MODULE_DETACH(pdsvc->ftm, pdftm_detach);
#endif /* WL_FTM */

	/* This is just to clean up the memory if unloading happens before disabling the method */
	if (pdsvc->cur_mif) {
		(*pdsvc->cur_mif->mrelease)(pdsvc->cur_mif);
	}
	wlc_module_unregister(wlc->pub, "proxd", pdsvc);

	if (pdsvc->tunep)
		MFREE(wlc->osh, pdsvc->tunep, sizeof(wl_proxd_params_tof_tune_t));
	MFREE(wlc->osh, pdsvc, sizeof(wlc_pdsvc_info_t));

#if defined(WLSLOTTED_BSS) && defined(WL_FTM)
	(void)wlc_slotted_bss_st_notif_unregister(wlc->sbi, pdftm_ext_sched_cb, NULL);
#endif // endif

	return ++callbacks;
}

/* wlc calls to receive the action frames */
int
wlc_proxd_recv_action_frames(wlc_info_t *wlc, wlc_bsscfg_t *bsscfg,
	struct dot11_management_header *hdr, uint8 *body, uint body_len,
	wlc_d11rxhdr_t *wrxh, uint32 rspec)
{
	wlc_pdsvc_info_t* pdsvc;
	uint8 action = 0;

	ASSERT(wlc != NULL);
	ASSERT(body != NULL);
	BCM_REFERENCE(bsscfg);
	BCM_REFERENCE(action);

	pdsvc = wlc->pdsvc_info;
#ifdef WL_PROXD_UCODE_TSYNC
	/* process ftm meas frames asap to clear the
	* ACK TS SHM blocks, else we may drop in between and
	* ACK block may not be cleared
	*/
	if (PROXD_ENAB_UCODE_TSYNC(wlc->pub)) {
		action = body[DOT11_ACTION_ACT_OFF];
		if (action == DOT11_PUB_ACTION_FTM) {
			pdburst_process_tx_rx_status(wlc,
				NULL, &wrxh->rxhdr, &hdr->sa);
		}
	}
#endif /* WL_PROXD_UCODE_TSYNC */

	if (!pdsvc) {
		return BCME_OK;
	}
#ifdef WL_FTM
	if (pdftm_is_ftm_action(pdsvc->ftm, hdr, body, body_len)) {
		if (bsscfg == NULL) {
			bsscfg = pdsvc->bsscfg;
		}

		(void)pdftm_rx(pdsvc->ftm, bsscfg, hdr, body, body_len, wrxh, rspec);
		goto done;
	}
	else if (pdftm_vs_is_ftm_action(pdsvc->ftm, hdr, body, body_len)) {
		(void) pdftm_vs_rx_frame(pdsvc->ftm, bsscfg == NULL ?  pdsvc->bsscfg : bsscfg,
			hdr, body, body_len, wrxh, rspec);
		goto done;
	}
#endif /* WL_FTM */

#ifdef WL_FTM
done:
#endif /* WL_FTM */

	return BCME_OK;
}

void wlc_proxd_set_pkttag_flags(wlc_info_t *wlc, wlc_pkttag_t *pkttag)
{
	wlc_pdsvc_info_t* pdsvc;
	ASSERT(wlc != NULL);
	pdsvc = wlc->pdsvc_info;

	if (pdsvc && pkttag) {
		pkttag->shared.packetid |= (PROXD_FTM_PACKET_TAG | PROXD_MEASUREMENT_PKTID);
		pkttag->flags |= WLF_USERTS;
	}
}

bool wlc_proxd_frame(wlc_info_t *wlc, wlc_pkttag_t *pkttag)
{
	wlc_pdsvc_info_t* pdsvc;

	ASSERT(wlc != NULL);

	pdsvc = wlc->pdsvc_info;

	if (pdsvc && pkttag &&
		((pkttag->shared.packetid & 0xffff0000) == (PROXD_FTM_PACKET_TAG |
		PROXD_MEASUREMENT_PKTID)))
	{
		return TRUE;
	}
	return FALSE;
}

void wlc_proxd_tx_conf(wlc_info_t *wlc, uint16 *phyctl, uint16 *mch, wlc_pkttag_t *pkttag)
{
	wlc_pdsvc_info_t* pdsvc;

	ASSERT(wlc != NULL);

	pdsvc = wlc->pdsvc_info;

	if (pdsvc && pkttag &&
		((pkttag->shared.packetid & 0x7fff0000) == PROXD_FTM_PACKET_TAG))
	{
		uint16 mask;
		uint8 core_shift = (pdsvc->tunep->core == 255) ? 0 : pdsvc->tunep->core;
		if (D11REV_GE(wlc->pub->corerev, 80)) {
			mask = (1 << core_shift);
			*phyctl = (*phyctl & ~(D11_REV80_PHY_TXC_CORE_MASK)) | mask;
		} else {
			mask = (1 << core_shift) << D11AC_PHY_TXC_CORE_SHIFT;
			*phyctl = (*phyctl & ~D11AC_PHY_TXC_CORE_MASK) | mask;
		}

		if (pkttag->shared.packetid & PROXD_MEASUREMENT_PKTID) {
			/* signal ucode to enable timestamping for this frame */
			*mch |= D11AC_TXC_TOF;
		}
	}
}

uint16 wlc_proxd_get_tx_subband(wlc_info_t * wlc, chanspec_t chanspec)
{
	uint16 subband = WL_CHANSPEC_CTL_SB_LLL;
	uint16 ichan, tchan;

	if (CHSPEC_IS80(WLC_BAND_PI_RADIO_CHANSPEC)) {
		/* Target is 80M band */
		tchan = CHSPEC_CHANNEL(WLC_BAND_PI_RADIO_CHANSPEC);
		ichan = CHSPEC_CHANNEL(chanspec);
		if (CHSPEC_IS20(chanspec)) {
			/* Initiator is 20 band */
			if (ichan + 2 == tchan)
				subband = WL_CHANSPEC_CTL_SB_LLU;
			else if (ichan == tchan + 2)
				subband = WL_CHANSPEC_CTL_SB_LUL;
			else if (ichan == tchan + 6)
				subband = WL_CHANSPEC_CTL_SB_LUU;
		}
		else if (CHSPEC_IS40(chanspec)) {
			subband = chanspec & WL_CHANSPEC_CTL_SB_MASK;
		}
	} else if (CHSPEC_IS40(WLC_BAND_PI_RADIO_CHANSPEC)) {
		tchan = CHSPEC_CHANNEL(WLC_BAND_PI_RADIO_CHANSPEC);
		if (CHSPEC_IS20(chanspec)) {
			ichan = CHSPEC_CHANNEL(chanspec);
			if (ichan == tchan + 2)
				subband = WL_CHANSPEC_CTL_SB_LLU;
		}
	}
	return subband >> WL_CHANSPEC_CTL_SB_SHIFT;
}

void wlc_proxd_tx_conf_subband(wlc_info_t *wlc, uint16 *phyctl, wlc_pkttag_t *pkttag)
{
	wlc_pdsvc_info_t* pdsvc;

	ASSERT(wlc != NULL);

	pdsvc = wlc->pdsvc_info;

	if (pdsvc && pkttag &&
		((pkttag->shared.packetid & 0x7fff0000) == PROXD_FTM_PACKET_TAG)) {
		uint16 subband;
		chanspec_t chspec = pkttag->shared.packetid & 0xffff;
		subband = wlc_proxd_get_tx_subband(wlc, chspec) & D11AC_PHY_TXC_PRIM_SUBBAND_MASK;
		if (D11REV_GE(wlc->pub->corerev, 80)) {
			*phyctl = (*phyctl & ~D11_REV80_PHY_TXC_SB_MASK) |
				(subband << D11_REV80_PHY_TXC_SB_SHIFT);
		} else {
			*phyctl = (*phyctl & ~WL_CHANSPEC_CTL_SB_MASK) |
				(subband << WL_CHANSPEC_CTL_SB_SHIFT);
		}
	}
}

uint32 wlc_pdsvc_sqrt(uint32 x)
{
	int i;
	uint32 answer = 0, old = 1;
		i = 0;
	while (i < 100) {
		answer = (old + (x / old)) >> 1;
		if (answer == old-1 || answer == old)
			break;
		old = answer;
		i++;
	}
	return answer;
}

int32 wlc_pdsvc_average(int32 *arr, int n)
{
	int total;
	int i;
	int32 ret;

	if (n > 1) {
		i = 1;
		total = 0;
		while (i < n) {
			total += (arr[i++]-arr[0]);
		}
		total = total*100/n;
		ret = total/100+arr[0];
		if (total%100 >= 50) {
			ret++;
		}
	} else if (n == 1) {
		ret = arr[0];
	} else {
		ret = 0;
	}

	return ret;
}

uint32 wlc_pdsvc_deviation(int32 *arr, int32 mean, int n, uint8 decimaldigits)
{
	uint32 sum = 0;
	int i, diff;

	if (n == 0) {
		return 0;
	}
	if (decimaldigits > 3) {
		decimaldigits = 3;
	}
	for (i = 0; i < n; i++) {
		diff = arr[i] - mean;
		sum += diff * diff;
	}
	for (i = 0; i < decimaldigits; i++) {
		sum *= 100;
	}

	return wlc_pdsvc_sqrt(sum/n);
}

#ifdef WL_PROXD_OUTLIER_FILTERING
void
wlc_pdsvc_sortasc(int32 *arr, uint16 arr_size)
{
	uint16 i, j;
	int32 temp;

	/* Sort the data in ascending order */
	for (i = 0u; i < arr_size; i++) {
		for (j = 0u; j < arr_size - 1u; j++) {
			if (arr[j + 1u] < arr[j]) {
				temp = arr[j];
				arr[j] = arr[j + 1u];
				arr[j + 1u] = temp;
			}
		}
	}
}

int32
wlc_pdsvc_median(int32 *arr, uint16 arr_size)
{
	int32 median;

	/* median = (arr_size + 1)/2 th sample in the sorted set */
	if (arr_size % 2u == 0) {
		median = (arr[(arr_size / 2u) - 1u] + arr[arr_size / 2u]) / 2u;
	} else {
		median = arr[(arr_size + 1u)/2u - 1u];
	}

	return median;
}
#endif /* WL_PROXD_OUTLIER_FILTERING */

/* function to determine if the proxd is supported by the radio card */
bool wlc_is_proxd_supported(wlc_info_t *wlc)
{
	wlc_hw_info_t *wlc_hw;

	ASSERT(wlc != NULL);
	wlc_hw = wlc->hw;

	if (D11REV_IS(wlc_hw->corerev, 65) ||
	    D11REV_IS(wlc_hw->corerev, 129) ||
	    D11REV_IS(wlc_hw->corerev, 130)) {
		return TRUE;
	} else {
		return FALSE;
	}
}

uint32 proxd_get_ratespec_idx(ratespec_t rspec, ratespec_t ackrspec)
{
	uint32 idx = 0;

	if (RSPEC_ISLEGACY(rspec)) {
		if (RSPEC2RATE(rspec) == WLC_RATE_6M) {
			idx = WL_PROXD_RATE_6M;
		} else {
			idx = WL_PROXD_RATE_LEGACY;
		}
	} else if (RSPEC_ISHT(rspec)) {
		if (wlc_ratespec_mcs(rspec) > 0) {
			idx = WL_PROXD_RATE_MCS;
		} else {
			idx = WL_PROXD_RATE_MCS_0;
		}
	}

	if (RSPEC_ISLEGACY(ackrspec)) {
		if (RSPEC2RATE(ackrspec) == WLC_RATE_6M) {
			idx |= WL_PROXD_RATE_6M << WL_RSPEC_ACKIDX_SHIFT;
		} else {
			idx |= WL_PROXD_RATE_LEGACY << WL_RSPEC_ACKIDX_SHIFT;
		}
	} else if (RSPEC_ISHT(ackrspec)) {
		if (wlc_ratespec_mcs(ackrspec) > 0) {
			idx |= WL_PROXD_RATE_MCS << WL_RSPEC_ACKIDX_SHIFT;
		} else {
			idx |= WL_PROXD_RATE_MCS_0 << WL_RSPEC_ACKIDX_SHIFT;
		}
	}

	return idx;
}

#if defined(WL_FTM)

/* internal interface */
wlc_bsscfg_t *
pdsvc_get_bsscfg(wlc_pdsvc_info_t *pdsvc)
{
	wlc_info_t *wlc;

	ASSERT(pdsvc != NULL);
	wlc = pdsvc->wlc;
	if (!pdsvc->bsscfg) {
		pdsvc->bsscfg = wlc->cfg;
	}
	return pdsvc->bsscfg;
}

#if defined(BCMDBG)
void
proxd_dump(wlc_pdsvc_info_t *pdsvc, struct bcmstrbuf *b)
{
	/* tbd */
	ASSERT(pdsvc != NULL && pdsvc->ftm != NULL);
	pdftm_dump(pdsvc->ftm, b);
}
#endif // endif

static void
proxd_bss_updown(void *ctx, bsscfg_up_down_event_data_t *evt)
{
	wlc_pdsvc_info_t *pdsvc = (wlc_pdsvc_info_t *)ctx;

	ASSERT(pdsvc != NULL && pdsvc->ftm != NULL);
	ASSERT(evt != NULL && evt->bsscfg != NULL);
	ASSERT(WL_PROXD_EVENT_NONE == 0);

	pdftm_notify(pdsvc->ftm, evt->bsscfg,
		(evt->up ? PDFTM_NOTIF_BSS_UP : PDFTM_NOTIF_BSS_DOWN), NULL);
}

void
proxd_init_event(wlc_pdsvc_info_t *pdsvc, wl_proxd_event_type_t type,
	wl_proxd_method_t method, wl_proxd_session_id_t sid, wl_proxd_event_t *event)
{
	ASSERT(pdsvc != NULL);
	ASSERT(event != NULL);

	BCM_REFERENCE(pdsvc);

	event->version = htol16(WL_PROXD_API_VERSION);
	event->len = htol16(OFFSETOF(wl_proxd_event_t, tlvs));
	event->type = htol16(type);
	event->method = htol16(method);
	event->sid = htol16(sid);
	bzero(event->pad, sizeof(event->pad));
}

void
proxd_send_event(wlc_pdsvc_info_t *pdsvc, wlc_bsscfg_t *bsscfg, wl_proxd_status_t status,
    const struct ether_addr *addr, wl_proxd_event_t *event, uint16 len)
{
#ifdef WL_RANGE_SEQ
	wlc_info_t *wlc;
#endif // endif
	ASSERT(pdsvc != NULL);
	ASSERT(event != NULL);
	ASSERT(len >= OFFSETOF(wl_proxd_event_t, tlvs));

#ifdef WL_RANGE_SEQ
	wlc = pdsvc->wlc;
	ASSERT(wlc != NULL);
#endif // endif

	if (bsscfg == NULL)
		bsscfg = pdsvc->bsscfg;

	event->len = htol16(len);
#ifdef WL_RANGE_SEQ
	wlc_bss_mac_event(wlc, bsscfg, WLC_E_PROXD, addr, WLC_E_STATUS_SUCCESS,
#else
	wlc_bss_mac_event(pdsvc->wlc, bsscfg, WLC_E_PROXD, addr, WLC_E_STATUS_SUCCESS,
#endif // endif
		status, 0 /* auth type */, (uint8 *)event, len);
}

void*
proxd_alloc_action_frame(pdsvc_t *pdsvc, wlc_bsscfg_t *bsscfg,
	const struct ether_addr *da, const struct ether_addr *sa,
	const struct ether_addr *bssid, uint body_len, uint8 **body,
	uint8 category, uint8 action)
{
	void *pkt;
	dot11_action_frmhdr_t *afhdr;
	scb_t *scb;
	wlc_info_t *wlc;
	enum wlc_bandunit bandunit;

	ASSERT(pdsvc != NULL);
	ASSERT(bsscfg != NULL);
	ASSERT(body != NULL);

	wlc = pdsvc->wlc;
	if (category == DOT11_ACTION_CAT_VS) {
		ASSERT(body_len >= OFFSETOF(dot11_action_vs_frmhdr_t, data));
	}
	else {
		ASSERT(body_len >= OFFSETOF(dot11_action_frmhdr_t, data));
	}

	if (bsscfg->associated)
		bandunit = CHSPEC_BANDUNIT(bsscfg->current_bss->chanspec);
	else
		bandunit = wlc->band->bandunit;

	/* note: non-bss members must set bssid in public action frame
	 * to wildcard i.e. all 1s - see 11mcd4.0 10.20
	 */
	scb = wlc_scbfindband(wlc, bsscfg, da, bandunit);

	if (scb && SCB_ASSOCIATED(scb)) {
		bssid = &bsscfg->BSSID;
	} else if (ETHER_ISNULLADDR(bssid)) {
		bssid = &ether_bcast;
	}

	pkt = wlc_frame_get_action(wlc, da, sa,
		bssid, body_len, body, category);
	if (!pkt) {
		goto done;
	}

	WLPKTTAGBSSCFGSET(pkt, WLC_BSSCFG_IDX(bsscfg));
	WLPKTTAGSCBSET(pkt, scb);

	/*
	 setup non-vendor specific action frame header.
	 Note, vendor-specific Action Frame header should be setup by caller
	*/
	if (category != DOT11_ACTION_CAT_VS) {
		afhdr = (dot11_action_frmhdr_t *) (*body);
		afhdr->category = category;
		afhdr->action = action;
	}

done:
	return pkt;
}

bool
proxd_tx(pdsvc_t *pdsvc, void *pkt, wlc_bsscfg_t *bsscfg, ratespec_t rspec, int status)
{
	wlc_txq_info_t * qi;
	struct scb *scb;
	wlc_pkttag_t *pkttag;
	bool ret;
	int err;
	ratesel_txparams_t ftm_rate;

	ASSERT(pdsvc != NULL);
	ASSERT(pkt != NULL);
	ASSERT(bsscfg != NULL);

	scb = WLPKTTAGSCBGET(pkt);
	pkttag = WLPKTTAG(pkt);

	pkttag->flags |= WLF_USERTS;
	if (status != WL_PROXD_E_SCAN_INPROCESS && scb && bsscfg->up &&
		(BSSCFG_AP(bsscfg) || bsscfg->associated))
		qi = bsscfg->wlcif->qi;
	else
		qi = pdsvc->wlc->active_queue;

	if (rspec && RATELINKMEM_ENAB(pdsvc->wlc->pub)) {
		/* one rate, no ACK, therefore no fallback rates */
		memset(&ftm_rate, 0, sizeof(ftm_rate));
		ftm_rate.num = 1;
		ftm_rate.rspec[0] = rspec;
		ftm_rate.antselid[0] = 1;

		scb = pdsvc->wlc->band->proxd_scb;
		scb->aid = pkttag->shared.packetid & 0xffff; /* save chanspec */

		err = wlc_ratelinkmem_update_rate_entry(pdsvc->wlc, scb, &ftm_rate, 0);
		if (err != BCME_OK) {
			WL_ERROR(("%s: ratelinkmem_update failed ret %d\n", __FUNCTION__, err));
			PKTFREE(pdsvc->wlc->osh, pkt, TRUE);
			return FALSE;
		}
	}

	ret = wlc_queue_80211_frag(pdsvc->wlc, pkt, qi, scb,
		bsscfg, FALSE, NULL, RATELINKMEM_ENAB(pdsvc->wlc->pub) ? 0 : rspec);

	return ret;
}

static wl_proxd_params_tof_tune_t *proxd_init_tune(wlc_info_t *wlc)
{
	wl_proxd_params_tof_tune_t *tunep;

	tunep = MALLOCZ(wlc->osh, sizeof(wl_proxd_params_tof_tune_t));
	if (tunep) {
		tunep->version = WL_PROXD_TUNE_VERSION_3;
		tunep->N_log2[TOF_BW_20MHZ_INDEX] = TOF_DEFAULT_THRESHOLD_LOG2_20M;
		tunep->N_log2[TOF_BW_40MHZ_INDEX] = TOF_DEFAULT_THRESHOLD_LOG2_40M;
		tunep->N_log2[TOF_BW_80MHZ_INDEX] = TOF_DEFAULT_THRESHOLD_LOG2_80M;
		tunep->N_log2[TOF_BW_SEQTX_INDEX] = TOF_DEFAULT_TX_THRESHOLD_LOG2;
		tunep->N_log2[TOF_BW_SEQRX_INDEX] = TOF_DEFAULT_RX_THRESHOLD_LOG2;
		tunep->N_scale[TOF_BW_20MHZ_INDEX] = TOF_DEFAULT_THRESHOLD_SCALE_20M;
		tunep->N_scale[TOF_BW_40MHZ_INDEX] = TOF_DEFAULT_THRESHOLD_SCALE_40M;
		tunep->N_scale[TOF_BW_80MHZ_INDEX] = TOF_DEFAULT_THRESHOLD_SCALE_80M;
		tunep->N_scale[TOF_BW_SEQTX_INDEX] = TOF_DEFAULT_TX_THRESHOLD_SCALE;
		tunep->N_scale[TOF_BW_SEQRX_INDEX] = TOF_DEFAULT_RX_THRESHOLD_SCALE;
		tunep->ftm_cnt[TOF_BW_20MHZ_INDEX] = TOF_DEFAULT_FTMCNT_20M;
		tunep->ftm_cnt[TOF_BW_40MHZ_INDEX] = TOF_DEFAULT_FTMCNT_40M;
		tunep->ftm_cnt[TOF_BW_80MHZ_INDEX] = TOF_DEFAULT_FTMCNT_80M;
		tunep->ftm_cnt[TOF_BW_SEQTX_INDEX] = TOF_DEFAULT_FTMCNT_SEQ;
		tunep->ftm_cnt[TOF_BW_SEQRX_INDEX] = TOF_DEFAULT_FTMCNT_SEQ;
		tunep->N_log2_2g = TOF_DEFAULT_THRESHOLD_LOG2_2G;
		tunep->N_scale_2g = TOF_DEFAULT_THRESHOLD_SCALE_2G;

		tunep->seq_5g20.N_tx_log2 = TOF_DEFAULT_TX_THRESHOLD_LOG2_5G_20M;
		tunep->seq_5g20.N_tx_scale = TOF_DEFAULT_TX_THRESHOLD_SCALE_5G_20M;
		tunep->seq_5g20.N_rx_log2 = TOF_DEFAULT_RX_THRESHOLD_LOG2_5G_20M;
		tunep->seq_5g20.N_rx_scale = TOF_DEFAULT_RX_THRESHOLD_SCALE_5G_20M;
		tunep->seq_5g20.w_len = TOF_DEFAULT_WINDOW_LEN_5G_20;
		tunep->seq_5g20.w_offset = TOF_DEFAULT_WINDOW_OFFSET_5G_20;

#ifdef WL_RANGE_SEQ
		tunep->seq_2g20.N_tx_log2 = TOF_DEFAULT_TX_THRESHOLD_LOG2_2G_20M;
		tunep->seq_2g20.N_tx_scale = TOF_DEFAULT_TX_THRESHOLD_SCALE_2G_20M;
		tunep->seq_2g20.N_rx_log2 = TOF_DEFAULT_RX_THRESHOLD_LOG2_2G_20M;
		tunep->seq_2g20.N_rx_scale = TOF_DEFAULT_RX_THRESHOLD_SCALE_2G_20M;
		tunep->seq_2g20.w_len = TOF_DEFAULT_WINDOW_LEN_2G;
		tunep->seq_2g20.w_offset = TOF_DEFAULT_WINDOW_OFFSET_2G;
#endif /* WL_RANGE_SEQ */

		tunep->sw_adj = TOF_DEFAULT_SW_ADJ;
		tunep->hw_adj = TOF_DEFAULT_HW_ADJ;
		tunep->seq_en = TOF_DEFAULT_SEQ_EN;
		tunep->vhtack = 0;
		tunep->core = TOF_DEFAULT_CORE_SELECTION;

		/* 80MHz */
		tunep->w_len[TOF_BW_80MHZ_INDEX] = 32;
		tunep->w_offset[TOF_BW_80MHZ_INDEX] = 10;
		/* 40MHz */
		tunep->w_len[TOF_BW_40MHZ_INDEX] = 16;
		tunep->w_offset[TOF_BW_40MHZ_INDEX] = 8;
		/* 20MHz */
		tunep->w_len[TOF_BW_20MHZ_INDEX] = 8;
		tunep->w_offset[TOF_BW_20MHZ_INDEX] = 4;

		/* default bitflip and snr thresholds */
		tunep->bitflip_thresh = TOF_DEFAULT_RX_THRESHOLD_BITFLIP;
		tunep->snr_thresh = TOF_DEFAULT_RX_THRESHOLD_SNR;
		tunep->emu_delay = TOF_DEFAULT_EMU_DELAY;
		tunep->core_mask = TOF_DEFAULT_CORE_MASK;
		/* default auto core select GD variance and RSSI thresholds */
		phy_tof_init_gdv_th(WLC_PI(wlc), &(tunep->acs_gdv_thresh));

		tunep->acs_rssi_thresh = TOF_DEFAULT_RX_THRESHOLD_ACS_RSSI;

		/* default smoothing window enable */
		tunep->smooth_win_en = TOF_DEFAULT_RX_SMOOTH_WIN_EN;

		/* default smoothing window enable */
		phy_tof_init_gdmm_th(WLC_PI(wlc), &(tunep->acs_gdmm_thresh));

		tunep->acs_delta_rssi_thresh = TOF_DEFAULT_RX_THRESHOLD_ACS_DELTA_RSSI;
	}
	return tunep;
}

wl_proxd_params_tof_tune_t *proxd_get_tunep(wlc_info_t *wlc, uint64 *tq)
{
	wlc_pdsvc_info_t* pdsvc;

	ASSERT(wlc != NULL);
	ASSERT(wlc->pdsvc_info != NULL);

	pdsvc = wlc->pdsvc_info;

	if (tq) {
		if (!pdsvc->clkfactor) {
			pdsvc->clkfactor = wlc_proxd_AVB_clock_factor(pdsvc, TOF_SHIFT,
				&pdsvc->tunep->Ki, &pdsvc->tunep->Kt);
		}
		*tq = pdsvc->clkfactor;
	}

	return pdsvc->tunep;
}

void proxd_enable(wlc_info_t *wlc, bool enable)
{
	uint32 gptime = 0;
	uint16 chip_id = CHIPID(wlc->pub->sih->chip);
	uint16 val, avb_cap = 0;

	WL_TRACE(("proxd_enable: enable %d, _proxd %d\n", enable, wlc->pub->_proxd));
	if (wlc->pub->_proxd != enable)
	{
		WL_TRACE(("proxd_enable: set _proxd to %d\n", enable));
		wlc->pub->_proxd = enable;

		if (!wlc_isup(wlc) || wlc->state == WLC_STATE_GOING_DOWN) {
			/*
			If the interface is down or going down, when it comes back up,
			it will automatically take care of the below functions
			*/
			return;
		}

		/* For supported AX corerevs that are < 129.2, a new ucode needs to be
		 * loaded to use PROXD
		 */
		if (D11REV_IS(wlc->pub->corerev, 129) &&
			D11MINORREV_LT(wlc->pub->corerev_minor, 2)) {

			WLCNTINCR(wlc->pub->_cnt->reinit);

			/* cache gptime out count */
			if (wlc->pub->up)
				gptime = wlc_hrt_gptimer_get(wlc);

			wlc->state = WLC_STATE_GOING_UP;
			wlc->hw->need_reinit =  WL_REINIT_RC_USER_FORCED;
			wlc->hw->ucode_loaded = 0;
			wl_init(wlc->wl);

			/* restore gptime out count after init
			 * (gptimer is reset due to wlc_reset
			 */
			if (gptime)
				wlc_hrt_gptimer_set(wlc, gptime);
		} else {
			/*
			If the ucode doesn't need to change and wl_init does not need to be done
			again, check the capability here (normally done at interface init time)
			*/

			if (D11REV_GE(wlc->pub->corerev, 129)) {
				avb_cap = wlc_read_shm(wlc, M_UCODE_CAP_H(wlc->hw)) & EAP_FTM_CAP;
			}

			if (((chip_id == BCM43684_CHIP_ID) ||
				(chip_id == BCM63178_CHIP_ID) ||
				(D11REV_GE(wlc->pub->corerev, 65) && avb_cap)) &&
				PROXD_ENAB(wlc->pub)) {
				WL_ERROR(("wl%d: %s: proxd ucode tsync enable\n",
					wlc->pub->unit, __FUNCTION__));
				wlc->pub->cmn->_proxd_ucode_tsync = TRUE;
				val = wlc_read_shm(wlc, M_HOST_FLAGS6(wlc->hw));
				wlc_write_shm(wlc, M_HOST_FLAGS6(wlc->hw), val | MHF6_TSYNC_EN);
			}
		}

		wlc_enable_avb_timer_war(wlc->hw, enable);
	}
}

uint16
proxd_get_tunep_idx(wlc_info_t *wlc, wl_proxd_session_flags_t flags,
	chanspec_t cspec, wl_proxd_params_tof_tune_t **tunepp)
{
	uint16 idx;

	ASSERT(tunepp != NULL);

	if (flags & WL_PROXD_SESSION_FLAG_SEQ_EN) {
		if (flags & WL_PROXD_SESSION_FLAG_INITIATOR)
			idx = TOF_BW_SEQRX_INDEX;
		else
			idx = TOF_BW_SEQTX_INDEX;
	} else if (CHSPEC_IS80(cspec))
		idx = TOF_BW_80MHZ_INDEX;
	else if (CHSPEC_IS40(cspec))
		idx = TOF_BW_40MHZ_INDEX;
	else
		idx = TOF_BW_20MHZ_INDEX;

	*tunepp = proxd_get_tunep(wlc, NULL);
	return idx;
}
#endif /* WL_FTM */

/* update N/S values based on using VHT ACK or not */
void
proxd_update_tunep_values(wl_proxd_params_tof_tune_t *tunep, chanspec_t cspec, bool vhtack)
{
	if (!tunep)
		return;
	if (!(tunep->setflags & WL_PROXD_SETFLAG_N))
	{
		/* the N value based on the channel and rspec for 80/40MHz */
		if (CHSPEC_IS80(cspec)) {
			if (vhtack)
				tunep->N_log2[TOF_BW_80MHZ_INDEX] = TOF_DEFAULT_THRESHOLD_LOG2_80M;
			else
				tunep->N_log2[TOF_BW_80MHZ_INDEX] = TOF_LEGACY_THRESHOLD_LOG2_80M;
		} else if (CHSPEC_IS40(cspec)) {
			if (vhtack)
				tunep->N_log2[TOF_BW_40MHZ_INDEX] = TOF_DEFAULT_THRESHOLD_LOG2_40M;
			else
				tunep->N_log2[TOF_BW_40MHZ_INDEX] = TOF_LEGACY_THRESHOLD_LOG2_40M;
		}
	}
	if (!(tunep->setflags & WL_PROXD_SETFLAG_S))
	{
		/* the S value based on the channel and rspec for 80/40 Mhz */
		if (CHSPEC_IS80(cspec)) {
			if (vhtack)
				tunep->N_scale[TOF_BW_80MHZ_INDEX] =
					TOF_DEFAULT_THRESHOLD_SCALE_80M;
			else
				tunep->N_scale[TOF_BW_80MHZ_INDEX] = TOF_LEGACY_THRESHOLD_SCALE_80M;
		} else if (CHSPEC_IS40(cspec)) {
			if (vhtack)
				tunep->N_scale[TOF_BW_40MHZ_INDEX] =
					TOF_DEFAULT_THRESHOLD_SCALE_40M;
			else
				tunep->N_scale[TOF_BW_40MHZ_INDEX] = TOF_LEGACY_THRESHOLD_SCALE_40M;
		}
	}
}

wlc_ftm_t*
wlc_ftm_get_handle(wlc_info_t *wlc)
{
	wlc_pdsvc_info_t* pdsvc;
	wlc_ftm_t *ftm = NULL;

	ASSERT(wlc != NULL);
	pdsvc = wlc->pdsvc_info;
	if (pdsvc)
		ftm = pdsvc->ftm;
	return ftm;
}

void proxd_power(wlc_info_t *wlc, uint8 id, bool on)
{
	wlc_pdsvc_info_t* pdsvc;

	ASSERT(wlc != NULL);
	pdsvc = wlc->pdsvc_info;
	if (!pdsvc || id >= sizeof(pdsvc->pwrflag))
		return;

	if (on) {
		if (!pdsvc->pwrflag) {
			wlc_user_wake_upd(wlc, WLC_USER_WAKE_REQ_FTM, TRUE);
			wlc_mpc_off_req_set(wlc, MPC_OFF_REQ_FTM_SESSION, MPC_OFF_REQ_FTM_SESSION);
		}
		pdsvc->pwrflag |= (1 << id);
	} else {
		pdsvc->pwrflag &= ~(1 << id);
		if (!pdsvc->pwrflag) {
			wlc_user_wake_upd(wlc, WLC_USER_WAKE_REQ_FTM, FALSE);
			wlc->mpc_delay_off = 0;
			wlc_mpc_off_req_set(wlc, MPC_OFF_REQ_FTM_SESSION, 0);
		}
	}
}

void proxd_undeaf_phy(wlc_info_t *wlc, bool acked)
{
	uint16 shmemptr = wlc_read_shm(wlc, M_TOF_BLK_PTR(wlc)) << 1;
	uint16 rspcmd, i;

	if (acked) {
		/* wait ucode finishing deafing the PHY */
		for (i = 0; i < TOF_MCMD_TIMEOUT; i++) {
			rspcmd = wlc_read_shm(wlc, shmemptr + M_TOF_RSP_OFFSET(wlc));
			if ((rspcmd & TOF_RSP_MASK) == TOF_SUCCESS)
				break;
			OSL_DELAY(1);
		}
	} else {
		/* No Ack, reset ucode state */
		wlc_hw_info_t *wlc_hw = wlc->hw;

		i = 0;
		/* Wait until last command completes */
		while ((R_REG(wlc_hw->osh, D11_MACCOMMAND(wlc_hw)) & MCMD_TOF) &&
			(i < TOF_MCMD_TIMEOUT)) {
			OSL_DELAY(1);
			i++;
		}

		if (R_REG(wlc_hw->osh, D11_MACCOMMAND(wlc_hw)) & MCMD_TOF) {
			WL_ERROR(("TOF ucode cmd timeout; maccommand: 0x%x\n",
				R_REG(wlc_hw->osh, D11_MACCOMMAND(wlc_hw))));
		}

		wlc_write_shm(wlc, shmemptr + M_TOF_CMD_OFFSET(wlc), TOF_RESET);

		W_REG(wlc_hw->osh, D11_MACCOMMAND(wlc_hw), MCMD_TOF);
	}

	/* Now undeaf the PHY */
	phy_tof_cmd(WLC_PI(wlc), FALSE, 0);
}

static int
wlc_proxd_wlc_up(void *context)
{
	wlc_pdsvc_info_t *pdsvc = (wlc_pdsvc_info_t *)context;
	wlc_info_t *wlc = pdsvc->wlc;
#ifdef WL_PROXD_UCODE_TSYNC
	uint16 val;
	uint16 avb_cap = 0;
	uint16 chip_id = CHIPID(wlc->pub->sih->chip);

	if (D11REV_GE(wlc->pub->corerev, 129)) {
		avb_cap = wlc_read_shm(wlc, M_UCODE_CAP_H(wlc->hw)) & EAP_FTM_CAP;
	}

	if (((chip_id == BCM43684_CHIP_ID) ||
		(chip_id == BCM63178_CHIP_ID) ||
		(D11REV_GE(wlc->pub->corerev, 65) && avb_cap)) && PROXD_ENAB(pdsvc->wlc->pub)) {
		wlc->pub->cmn->_proxd_ucode_tsync = TRUE;
		val = wlc_read_shm(wlc, M_HOST_FLAGS6(wlc->hw));
		wlc_write_shm(wlc, M_HOST_FLAGS6(wlc->hw), val | MHF6_TSYNC_EN);
	}
#endif /* WL_PROXD_UCODE_TSYNC */
	wlc_enable_avb_timer_war(wlc->hw, wlc->pub->_proxd);
	return BCME_OK;
}

static int
wlc_proxd_wlc_down(void *context)
{
#ifdef WL_PROXD_UCODE_TSYNC
	wlc_pdsvc_info_t *pdsvc = (wlc_pdsvc_info_t *)context;
	wlc_info_t *wlc = pdsvc->wlc;
	uint16 val;
	if (PROXD_ENAB_UCODE_TSYNC(wlc->pub)) {
		val = wlc_read_shm(wlc, M_HOST_FLAGS6(wlc->hw));
		wlc_write_shm(wlc, M_HOST_FLAGS6(wlc->hw), val & ~MHF6_TSYNC_EN);
	}
#endif /* WL_PROXD_UCODE_TSYNC */
	return BCME_OK;
}

#ifdef WL_PROXD_UCODE_TSYNC
void
wlc_proxd_process_tx_rx_status(wlc_info_t *wlc, tx_status_t *txs,
	d11rxhdr_t *rxh, struct ether_addr *peer)
{
	pdburst_process_tx_rx_status(wlc, txs, rxh, peer);
}
#endif /* WL_PROXD_UCODE_TSYNC */

struct ether_addr *
wlc_proxd_get_randmac(wlc_pdsvc_info_t *pdsvc, wlc_bsscfg_t *bsscfg)
{
	struct ether_addr  *random_addr = NULL;
#ifdef WL_RANDMAC
	if (RANDMAC_ENAB(pdsvc->wlc->pub)) {
		random_addr = wlc_randmac_request(pdsvc->wlc->randmac_info,
			bsscfg, WLC_RANDMAC_FTM, NULL);
	}
#endif /* WL_RANDMAC */
	return random_addr;
}

int
wlc_proxd_release_randmac(wlc_pdsvc_info_t *pdsvc, wlc_bsscfg_t *bsscfg)
{
	int err = BCME_OK;
#ifdef WL_RANDMAC
	if (RANDMAC_ENAB(pdsvc->wlc->pub)) {
		err = wlc_randmac_release(pdsvc->wlc->randmac_info,
			bsscfg, WLC_RANDMAC_FTM);
	}
#endif /* WL_RANDMAC */
	return err;
}
