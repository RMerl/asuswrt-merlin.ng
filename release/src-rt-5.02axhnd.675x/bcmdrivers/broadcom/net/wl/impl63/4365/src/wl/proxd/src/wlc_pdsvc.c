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
 * $Id: wlc_pdsvc.c 491282 2014-07-15 19:26:17Z $
 */

#include <typedefs.h>
#include <bcmdefs.h>
#include <bcmdevs.h>
#include <osl.h>
#include <sbchipc.h>
#include <bcmutils.h>
#include <siutils.h>
#include <bcmendian.h>
#include <proto/802.11.h>
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
#include <wl_export.h>
#include <wlc_hw.h>
#include <wlc_hw_priv.h>
#include <hndpmu.h>
#include <wlc_pcb.h>

#include <wlc_pdsvc.h>
#include <wlc_pddefs.h>
#include <wlc_pdmthd.h>

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
	uint8 rptlistnum;
	struct ether_addr *rptlist;
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
		DEBUG
	*/
	IOV_TOF_SEQ = 13
};

/* Iovars */
static const bcm_iovar_t  wlc_proxd_iovars[] = {
	{"proxd", IOV_PROXD, 0, IOVT_BUFFER, sizeof(uint16)*2},
	{"proxd_params", IOV_PROXD_PARAMS, 0, IOVT_BUFFER, sizeof(wl_proxd_params_iovar_t)},
	{"proxd_bssid", IOV_PROXD_BSSID, 0, IOVT_BUFFER, ETHER_ADDR_LEN},
	{"proxd_mcastaddr", IOV_PROXD_MCASTADDR, 0, IOVT_BUFFER, ETHER_ADDR_LEN},
	{"proxd_find", IOV_PROXD_FIND, 0, IOVT_VOID, 0},
	{"proxd_stop", IOV_PROXD_STOP, 0, IOVT_VOID, 0},
	{"proxd_status", IOV_PROXD_STATUS, 0, IOVT_BUFFER, sizeof(wl_proxd_status_iovar_t)},
	{"proxd_monitor", IOV_PROXD_MONITOR, 0, IOVT_BUFFER, ETHER_ADDR_LEN},
	{"proxd_payload", IOV_PROXD_PAYLOAD, 0, IOVT_BUFFER, 0},
	{"proxd_collect", IOV_PROXD_COLLECT, 0, IOVT_BUFFER, sizeof(wl_proxd_collect_data_t)},
	{"proxd_ftmperiod", IOV_FTM_PERIOD, 0, IOVT_UINT32, 0},
	{"proxd_tune", IOV_PROXD_TUNE, 0, IOVT_BUFFER, sizeof(wl_proxd_params_iovar_t)},
	{"proxd_report", IOV_PROXD_REPORT, 0, IOVT_BUFFER, 0},
#ifdef TOF_DBG
	{"tof_seq", IOV_TOF_SEQ,  0, IOVT_UINT32, 0},
#endif // endif
	{NULL, 0, 0, 0, 0}
};

/* Proximity Default BSSID and Default Multicast address */
STATIC CONST struct ether_addr proxd_default_bssid = {{0x00, 0x90, 0x4c, 0x02, 0x17, 0x03}};
STATIC CONST struct ether_addr proxd_default_mcastaddr = {{0x01, 0x90, 0x4c, 0x02, 0x17, 0x03}};

#ifdef BCMDBG
static void wlc_proxd_bsscfg_cubby_dump(void *ctx, wlc_bsscfg_t *cfg, struct bcmstrbuf *b);
#else
#define wlc_proxd_bsscfg_cubby_dump NULL
#endif // endif

/* Initialize the RSSI method configuration parameters */
static void
BCMATTACHFN(wlc_pdsvc_init)(wlc_pdsvc_info_t *pdsvc)
{
	/* Setting common default parametrs */
	memcpy(&pdsvc->config.bssid, &proxd_default_bssid, ETHER_ADDR_LEN);
	memcpy(&pdsvc->config.mcastaddr, &proxd_default_mcastaddr, ETHER_ADDR_LEN);

	/* get rrtcal from nvram */
	pdsvc->ki = getintvararray(pdsvc->wlc->pub->vars, "rrtcal", 0);
	pdsvc->kt = getintvararray(pdsvc->wlc->pub->vars, "rrtcal", 1);
}

/* bsscfg cubby */
static int
wlc_proxd_bsscfg_cubby_init(void *ctx, wlc_bsscfg_t *cfg)
{
	wlc_pdsvc_info_t *pdsvc = (wlc_pdsvc_info_t *)ctx;
	bss_proxd_cubby_t *proxd_bsscfg_cubby;

	ASSERT(pdsvc != NULL);
	ASSERT(cfg != NULL);

	proxd_bsscfg_cubby = (bss_proxd_cubby_t *)PROXD_BSSCFG_CUBBY(pdsvc, cfg);

	bzero((void *)proxd_bsscfg_cubby, sizeof(*proxd_bsscfg_cubby));

	return BCME_OK;
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

/* Initialize the bsscfg */
static int
wlc_proxd_bsscfg_init(wlc_pdsvc_info_t *pdsvc, wlc_bsscfg_t *bsscfg)
{
	wlc_info_t *wlc = pdsvc->wlc;
	uint8 gmode = GMODE_AUTO;
	int err = BCME_OK;

	pdsvc->bsscfg = bsscfg;

	if ((err = wlc_bsscfg_init(wlc, bsscfg)) != BCME_OK) {
		WL_ERROR(("wl%d: %s: cannot init bsscfg\n", wlc->pub->unit, __FUNCTION__));
		goto exit;
	}

	if (!IS_SINGLEBAND_5G(wlc->deviceid)) {
		gmode = wlc->bandstate[BAND_2G_INDEX]->gmode;
	}
	if (gmode == GMODE_LEGACY_B) {
		WL_ERROR(("wl%d: %s: gmode cannot be GMODE_LEGACY_B\n", wlc->pub->unit,
			__FUNCTION__));
		err = BCME_BADRATESET;
		goto exit;
	}

	if ((err = wlc_bsscfg_rateset_init(wlc, bsscfg, WLC_RATES_OFDM,
		WL_BW_CAP_40MHZ(wlc->band->bw_cap) ? CHSPEC_WLC_BW(wlc->home_chanspec) : 0,
		BSS_N_ENAB(wlc, bsscfg))) != BCME_OK) {
		WL_ERROR(("wl%d: %s: failed rateset int\n", wlc->pub->unit, __FUNCTION__));
		goto exit;
	}

	/* set bsscfg to IBSS */
	bsscfg->BSS = 0;
	bsscfg->current_bss->infra = 0;

	/* Do not particiate in mchan scheduler since
	   proxd has its own scheduler for channel access
	*/
	//bsscfg->flags |= WLC_BSSCFG_MCHAN_DISABLE;

	/* Initialize default flags if needed */

	/* Set BSSID for this bsscfg */
	bcopy(&pdsvc->config.bssid, &bsscfg->BSSID, ETHER_ADDR_LEN);

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
	wlc_set_bssid(bsscfg);

exit:
	return err;
}

/* Enabling the proximity interface */
static int
wlc_proxd_ifadd(wlc_pdsvc_info_t *pdsvc, struct ether_addr *addr)
{
	wlc_info_t *wlc = pdsvc->wlc;
	wlc_bsscfg_t *bsscfg = NULL;
	int idx;
	int err = BCME_OK;

	/* Get the free id to create bsscfg */
	if ((idx = wlc_bsscfg_get_free_idx(wlc)) == -1) {
		WL_ERROR(("wl%d: %s: no free bsscfg\n", wlc->pub->unit, __FUNCTION__));
		err = BCME_NORESOURCE;
		goto exit;
	}
	if ((bsscfg = wlc_bsscfg_alloc(wlc, idx, WLC_BSSCFG_NOIF, NULL, FALSE)) == NULL) {
		WL_ERROR(("wl%d: %s: cannot create bsscfg\n", wlc->pub->unit, __FUNCTION__));
		err = BCME_NOMEM;
		goto exit;
	}

	if (addr)
		bcopy(addr, &bsscfg->cur_etheraddr, ETHER_ADDR_LEN);

	if (wlc_bsscfg_type_init(wlc, bsscfg, BSSCFG_TYPE_PROXD) != BCME_OK) {
		WL_ERROR(("wl%d: %s: wlc_pd_bsscfg_init failed \n",
			wlc->pub->unit, __FUNCTION__));
		goto exit;
	}

	return BCME_OK;

exit:
	if (bsscfg != NULL)
		wlc_bsscfg_free(wlc, bsscfg);
	pdsvc->bsscfg = NULL;

	return err;
}

/* Deleting the proximity interface */
static void
wlc_proxd_ifdel(wlc_pdsvc_info_t *pdsvc)
{
	wlc_bsscfg_t *bsscfg;

	ASSERT(pdsvc != NULL);
	bsscfg = pdsvc->bsscfg;

	if (bsscfg != NULL) {
		if (bsscfg->enable)
			wlc_bsscfg_disable(pdsvc->wlc, bsscfg);
		wlc_bsscfg_free(pdsvc->wlc, bsscfg);
	}

	pdsvc->bsscfg = NULL;
}

static int
wlc_proxd_stop(void *ctx)
{
	wlc_pdsvc_info_t *pdsvc = (wlc_pdsvc_info_t *)ctx;

	ASSERT(pdsvc != NULL);

	/* Stop proximity detection.
	   It should be safe to call stop even if it is not active.
	*/
	if (pdsvc->cur_mif && pdsvc->cur_mif->mstart) {
		(*pdsvc->cur_mif->mstart)(pdsvc->cur_mif, FALSE);
	}

	return BCME_OK;
}

/* Iovar processing: Each proximity method is created, deleted, and changes it state by iovars */
static int
wlc_proxd_doiovar(void *ctx, const bcm_iovar_t *vi, uint32 actionid, const char *name,
	void *params, uint p_len, void *arg, int a_len, int val_size, struct wlc_if *wlcif)
{
	wlc_pdsvc_info_t *pdsvc_info = (wlc_pdsvc_info_t *)ctx;
	wl_proxd_status_iovar_t *proxd_status_iovar_p;
	uint16 method = 0, mode = 0;
	bool is_active = FALSE, is_wlup = FALSE;
	wlc_info_t *wlc;
	int err = BCME_OK;

	ASSERT(pdsvc_info != NULL);
	CHECK_SIGNATURE(pdsvc_info, WLC_PDSVC_SIGNATURE);
	ASSERT(pdsvc_info->wlc != NULL);
	wlc = pdsvc_info->wlc;

	/* Process IOVARS */
	switch (actionid) {
	/* wl proxd [0|1] [neutral | initiator | target] */
	case IOV_GVAL(IOV_PROXD):
		*((uint16*)arg) = pdsvc_info->method;
		if (pdsvc_info->method > 0)
			*((uint16*)(arg + sizeof(method))) = pdsvc_info->config.mode;
		else
			*((uint16*)(arg + sizeof(method))) = 0;
		break;

	/* wl proxd [0|1] [neutral | initiator | target] */
	case IOV_SVAL(IOV_PROXD):
		if (p_len >= (uint)sizeof(method))
			bcopy(params, &method, sizeof(method));

		if (p_len >= (uint)(sizeof(method) + sizeof(mode)))
			bcopy(params + sizeof(method), &mode, sizeof(mode));

		/* RSSI method removed */
		if ((mode == WL_PROXD_MODE_NEUTRAL) ||
			(method == PROXD_RSSI_METHOD))
			return BCME_UNSUPPORTED;

		if (method > PROXD_MAX_METHOD_NUM)
			return BCME_BADARG;

		if (method != PROXD_UNDEFINED_METHOD) {
			/* Return BCME_BUSY if it is already enabled */
			if (pdsvc_info->method != PROXD_UNDEFINED_METHOD) {
				return BCME_BUSY;
			}
			if (!wlc->clk)
				return BCME_NOCLK;

			/* Initialize the Transmit call back */
			ASSERT(pdsvc_info->bsscfg == NULL);
			if (ETHER_ISNULLADDR(&wlc->cfg->cur_etheraddr)) {
				WL_ERROR(("wl%d: %s: Primary interface ethernet address is NULL \n",
					wlc->pub->unit, __FUNCTION__));
				return BCME_BADADDR;
			}
			if (mode == WL_PROXD_MODE_INITIATOR || method != PROXD_TOF_METHOD) {
				if ((err = wlc_proxd_ifadd(pdsvc_info, &wlc->cfg->cur_etheraddr))
					!= BCME_OK) {
					WL_ERROR(("wl%d: %s: wlc_proxd_ifadd failed \n",
						wlc->pub->unit, __FUNCTION__));
					break;
				}
			}

			/*   create & init method object   */
			ASSERT(pdsvc_info->cur_mif == NULL);

			/* Create a proximity method */
			ASSERT(pdsvc_info->method_create_fn[method-1] != NULL);

			if (!( pdsvc_info->cur_mif =	/* if Ok returns *mth iface obj */
				(*pdsvc_info->method_create_fn[method-1])(
					wlc, mode, &pdsvc_info->funcs, NULL,
					&pdsvc_info->payload))) {
				WL_ERROR(("wl%d: %s: Create method:%d failed \n",
					wlc->pub->unit, __FUNCTION__, method));
				if (pdsvc_info->bsscfg)
					wlc_proxd_ifdel(pdsvc_info);
				break;
			}

			/* Configure  created method */
			if (pdsvc_info->cur_mif->mconfig) {
				/* Call the method configuration */
				(*pdsvc_info->cur_mif->mconfig)(pdsvc_info->cur_mif, mode,
					pdsvc_info->bsscfg);
			}

			/* FIXME: Need to check return value of *pdsvc_info->cur_mif->mconfig
			          Set pdsvc_info->method and mode only if success
			*/
			pdsvc_info->method = method;
			pdsvc_info->config.mode = mode;

			wlc->pub->_proxd = TRUE;
		} else /*  == PROXD_UNDEFINED_METHOD) */ {
			/* Disable proxmity deteciton if it is enabled */
			if (pdsvc_info->method != 0) {
				int ret = 0;
				ASSERT(pdsvc_info->cur_mif != NULL);
				/* Delete method */
				if (pdsvc_info->cur_mif) {
					/* Stop proximity */
					(void) wlc_proxd_stop((void *)pdsvc_info);

					/* Release the method */
					ret = (*pdsvc_info->cur_mif->mrelease)(pdsvc_info->cur_mif);
					pdsvc_info->cur_mif = NULL;
				}

				/* Delete the proxd interface */
				if (pdsvc_info->bsscfg && !ret) {
					wlc_proxd_ifdel(pdsvc_info);
					pdsvc_info->bsscfg = NULL;
				}
				/* Initiatize all the parameters to NULL */
				pdsvc_info->method = PROXD_UNDEFINED_METHOD;
				pdsvc_info->config.mode = WL_PROXD_MODE_DISABLE;
				wlc->pub->_proxd = FALSE;
			}
		}
		break;

	case IOV_GVAL(IOV_PROXD_PARAMS):
		if (!pdsvc_info->cur_mif)
			return BCME_BADARG;

		if (p_len >= (uint)sizeof(method))
			bcopy(params, &method, sizeof(method));

		/* must be a vallid method and == to created method  */
		if (method == 0 ||
			method > PROXD_MAX_METHOD_NUM ||
			pdsvc_info->method != method)
			return BCME_BADARG;

		/*  read params into the buffer */
		ASSERT(pdsvc_info->cur_mif->rw_params != NULL);
		*(uint8 *)arg = pdsvc_info->method; /* current method */
		err = pdsvc_info->cur_mif->rw_params(pdsvc_info->cur_mif,
			arg+sizeof(method), p_len, 0);

		break;
	case IOV_SVAL(IOV_PROXD_PARAMS):
		/* proximity detection should be in idle state */
		if (pdsvc_info->state != 0)
			return BCME_EPERM;

		if (p_len >= (uint)sizeof(method))
			bcopy(params, &method, sizeof(method));

		/* must be a vallid method and == to created method  */
		if (method == 0 || method > PROXD_MAX_METHOD_NUM ||
			pdsvc_info->method != method) {
			PROXD_TRACE((" cmd mth:%d,  svc cur:%d\n", method, pdsvc_info->method));
			return BCME_BADARG;
		}

		/*  write params into method module  */
		ASSERT(pdsvc_info->cur_mif->rw_params != NULL);
		err = pdsvc_info->cur_mif->rw_params(pdsvc_info->cur_mif,
			params + sizeof(method), p_len, 1);

		break;

	case IOV_GVAL(IOV_PROXD_BSSID):
		bcopy(&pdsvc_info->config.bssid, arg, ETHER_ADDR_LEN);
		break;

	case IOV_SVAL(IOV_PROXD_BSSID):
		/* Don't check if NULL so that to allow clearing bssid */
		bcopy(params, &pdsvc_info->config.bssid, ETHER_ADDR_LEN);

		if (pdsvc_info->bsscfg != NULL) {
			/* Update BSSID */
			bcopy(&pdsvc_info->config.bssid, &pdsvc_info->bsscfg->BSSID,
			      ETHER_ADDR_LEN);
		}
		break;

	case IOV_GVAL(IOV_PROXD_MCASTADDR):
		bcopy(&pdsvc_info->config.mcastaddr, arg, ETHER_ADDR_LEN);
		break;

	case IOV_SVAL(IOV_PROXD_MCASTADDR):
		if (!ETHER_ISMULTI(params))
			return BCME_BADADDR;

		bcopy(params, &pdsvc_info->config.mcastaddr, ETHER_ADDR_LEN);
		break;

	case IOV_SVAL(IOV_PROXD_FIND):
		/* proximity detection should have been enabled to start */
		if (pdsvc_info->method == 0 || !pdsvc_info->cur_mif)
			return BCME_EPERM;

		/* proxd_find and proxd_stop have a dependency on the driver up state.
		 * They are allowed only when the driver is up.
		 * wlc_down_for_mpc() check is required to differentiate it from
		 *  wl down due to MPC
		 */
		is_wlup = wlc->pub->up || wlc_down_for_mpc(wlc);
		if (!is_wlup)
			return BCME_NOTUP;

		/* Check if it is already active */
		if ((*pdsvc_info->cur_mif->mstatus)(pdsvc_info->cur_mif, &is_active, NULL) ==
			BCME_ERROR)
			return BCME_ERROR;

		/* Call start only when it is not active */
		if (!is_active)
			return (*pdsvc_info->cur_mif->mstart)(pdsvc_info->cur_mif, TRUE);
		else
			return BCME_BUSY;
		break;

	case IOV_SVAL(IOV_PROXD_STOP):
		/* proximity detection should have been enabled to stop */
		if (pdsvc_info->method == 0 || !pdsvc_info->cur_mif)
			return BCME_EPERM;

		/* proxd_find and proxd_stop have a dependency on the driver up state.
		 * They are allowed only when the driver is up.
		 * wlc_down_for_mpc() check is required to differentiate it from
		 *  wl down due to MPC
		 */
		is_wlup = wlc->pub->up || wlc_down_for_mpc(wlc);
		if (!is_wlup)
			return BCME_NOTUP;

		(void) wlc_proxd_stop((void *)pdsvc_info);
		break;

	case IOV_GVAL(IOV_PROXD_STATUS):
		/* proximity detection should have been enabled to start */
		if (pdsvc_info->method == 0 || !pdsvc_info->cur_mif)
			return BCME_EPERM;

		if (p_len < sizeof(wl_proxd_status_iovar_t))
			return BCME_BUFTOOSHORT;

		if (pdsvc_info->cur_mif->mstatus) {
			proxd_status_iovar_p = (wl_proxd_status_iovar_t *)arg;
			proxd_status_iovar_p->method = pdsvc_info->method;
			(*pdsvc_info->cur_mif->mstatus)(pdsvc_info->cur_mif, &is_active,
				proxd_status_iovar_p);
		}
		break;

	case IOV_SVAL(IOV_PROXD_MONITOR):
		/* proximity detection goes to monitor mode */
		if (pdsvc_info->method == 0 || !pdsvc_info->cur_mif)
			return BCME_EPERM;

		/* Check if it is already active */
		if ((*pdsvc_info->cur_mif->mstatus)(pdsvc_info->cur_mif, &is_active, NULL) ==
			BCME_ERROR)
			return BCME_ERROR;

		/* Call start only when it is not active */
		if (!is_active)
			(*pdsvc_info->cur_mif->mmonitor)(pdsvc_info->cur_mif, params);
		else
			return BCME_BUSY;
		break;

	case IOV_GVAL(IOV_PROXD_PAYLOAD):
		*((uint16*)arg) = pdsvc_info->payload.len;
		if (pdsvc_info->payload.len > 0)
			memcpy(arg + sizeof(uint16), pdsvc_info->payload.data,
				pdsvc_info->payload.len);
		break;

	case IOV_SVAL(IOV_PROXD_PAYLOAD):
		if (pdsvc_info->payload.data)
			MFREE(wlc->osh, pdsvc_info->payload.data, pdsvc_info->payload.len);
		pdsvc_info->payload.data = NULL;
		if (p_len > 0) {
			pdsvc_info->payload.data = MALLOC(wlc->osh, p_len);
			if (!pdsvc_info->payload.data)
				return BCME_NOMEM;
			memcpy(pdsvc_info->payload.data, arg, p_len);
		}
		pdsvc_info->payload.len = p_len;
		break;

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

			/* must be a vallid method and == to created method  */
			if (quety.method == 0 || quety.method > PROXD_MAX_METHOD_NUM ||
				pdsvc_info->method != quety.method) {
				return BCME_BADARG;
			}

			if (pdsvc_info->cur_mif->collect == NULL)
				return BCME_UNSUPPORTED;

			err = pdsvc_info->cur_mif->collect(pdsvc_info->cur_mif,
				&quety, arg, a_len, &len);
		}
		break;

	case IOV_GVAL(IOV_PROXD_TUNE):
		if (p_len >= (uint)sizeof(method))
			bcopy(params, &method, sizeof(method));

		/* must be a vallid method and == to created method  */
		if (method == 0 ||
			method > PROXD_MAX_METHOD_NUM ||
			pdsvc_info->method != method)
			return BCME_BADARG;

		if (method == PROXD_TOF_METHOD)
			err = wlc_pdtof_get_tune(pdsvc_info->cur_mif,
				arg+sizeof(method), p_len);
		else
			err = BCME_UNSUPPORTED;
		break;

	case IOV_SVAL(IOV_PROXD_TUNE):
		/* proximity detection should be in idle state */
		if (pdsvc_info->state != 0)
			return BCME_EPERM;

		if (p_len >= (uint)sizeof(method))
			bcopy(params, &method, sizeof(method));

		/* must be a vallid method and == to created method  */
		if (method == 0 || method > PROXD_MAX_METHOD_NUM ||
			pdsvc_info->method != method) {
			PROXD_TRACE((" cmd mth:%d,	svc cur:%d\n", method, pdsvc_info->method));
			return BCME_BADARG;
		}

		if (method == PROXD_TOF_METHOD)
			err = wlc_pdtof_set_tune(pdsvc_info->cur_mif,
				params+sizeof(method), p_len);
		else
			err = BCME_UNSUPPORTED;
		break;

	case IOV_GVAL(IOV_FTM_PERIOD):
		if (pdsvc_info->method == PROXD_TOF_METHOD) {
			int val = wlc_pdtof_get_ftmperiod(pdsvc_info->cur_mif);
			if (val >= 0) {
				*((uint32 *)arg) = (uint32)val;
				break;
			}
		}
		err = BCME_UNSUPPORTED;
		break;

	case IOV_SVAL(IOV_FTM_PERIOD):
		if (pdsvc_info->method == PROXD_TOF_METHOD) {
			uint32 val = 0;
			if (p_len >= sizeof(uint32))
				bcopy(params, &val, sizeof(val));
			if (wlc_pdtof_set_ftmperiod(pdsvc_info->cur_mif,
				val) == BCME_OK)
				break;
		}
		err = BCME_UNSUPPORTED;
		break;
#ifdef TOF_DBG
	case IOV_GVAL(IOV_TOF_SEQ):
		if (pdsvc_info->method == PROXD_TOF_METHOD)
			wlc_tof_seq_iov(pdsvc_info->cur_mif, 0, (int*)arg);
		else
			err = BCME_UNSUPPORTED;
		break;
	case IOV_SVAL(IOV_TOF_SEQ):
		if (pdsvc_info->method == PROXD_TOF_METHOD)
			err = wlc_tof_seq_iov(pdsvc_info->cur_mif, *((int32*)params), NULL);
		else
			err = BCME_UNSUPPORTED;
		break;
#endif // endif
	case IOV_GVAL(IOV_PROXD_REPORT):
		bzero(arg, ETHER_ADDR_LEN * WL_PROXD_MAXREPORT);
		if (pdsvc_info->rptlistnum)
			bcopy(pdsvc_info->rptlist, arg, ETHER_ADDR_LEN * pdsvc_info->rptlistnum);
		break;

	case IOV_SVAL(IOV_PROXD_REPORT):
		if (pdsvc_info->rptlist)
			MFREE(wlc->osh, pdsvc_info->rptlist,
				pdsvc_info->rptlistnum * ETHER_ADDR_LEN);
		pdsvc_info->rptlist = NULL;
		pdsvc_info->rptlistnum = 0;
		if (p_len > 0) {
			if (ETHER_ISNULLADDR(arg))
				break;
			pdsvc_info->rptlist = MALLOC(wlc->osh, p_len);
			if (!pdsvc_info->rptlist)
				return BCME_NOMEM;
			bcopy(arg, pdsvc_info->rptlist, p_len);
			pdsvc_info->rptlistnum = p_len / ETHER_ADDR_LEN;
		}
		break;
	default:
		err = BCME_UNSUPPORTED;
		break;
	}

	return err;
}

/* Provides the call back to method to transmit the action frames */
static int
wlc_proxd_transmitaf(wlc_pdsvc_info_t* pdsvc, wl_action_frame_t *af,
	ratespec_t rate_override, pkcb_fn_t fn, struct ether_addr *selfea)
{
	wlc_info_t *wlc;
	wlc_bsscfg_t *bsscfg;
	bss_proxd_cubby_t *proxd_bsscfg_cubby;
	struct scb *scb;
	struct ether_addr *bssid;
	uint16 method;
	chanspec_t chanspec;
	uint8* pbody;
	wlc_pkttag_t *pkttag;
	void *pkt;

	ASSERT(pdsvc != NULL);
	ASSERT(af != NULL);

	wlc = pdsvc->wlc;

	if (pdsvc->bsscfg) {
		bsscfg = pdsvc->bsscfg;
	} else {
		bsscfg = wlc_bsscfg_find_by_hwaddr(wlc, selfea);
	}

	if (ETHER_ISNULLADDR(&af->da))
		bcopy(&pdsvc->config.mcastaddr, &af->da, ETHER_ADDR_LEN);
	if (ETHER_ISNULLADDR(&bsscfg->BSSID))
		bssid = &pdsvc->config.bssid;
	else
		bssid = &bsscfg->BSSID;

	/* get allocation of action frame */
	if ((pkt = wlc_frame_get_action(wlc, FC_ACTION, &af->da, &bsscfg->cur_etheraddr,
		bssid, af->len, &pbody, DOT11_ACTION_CAT_VS)) == NULL) {
		return BCME_NOMEM;
	}

	pkttag = WLPKTTAG(pkt);
	pkttag->shared.packetid = af->packetId;
	WLPKTTAGBSSCFGSET(pkt, bsscfg->_idx);

	/* copy action frame payload */
	bcopy(af->data, pbody, af->len);

	/* Need to set a proper scb in action frame transmission so that lower layer
	   functions can have a correct reference to scb and bsscfg. If scb is not
	   provided on wlc_queue_80211_frag(), it internally uses the default scb
	   which points to a wrong bsscfg.
	*/
	method = pdsvc->method;
	ASSERT(method != 0 && method <= PROXD_MAX_METHOD_NUM);

	/* read chanspec from current method  */
	ASSERT(pdsvc->cur_mif->params_ptr != NULL);
	chanspec = pdsvc->cur_mif->params_ptr->chanspec;

	proxd_bsscfg_cubby = (bss_proxd_cubby_t *)PROXD_BSSCFG_CUBBY(pdsvc, bsscfg);

	if (method == PROXD_RSSI_METHOD)
		mboolset(proxd_bsscfg_cubby->flags, PROXD_FLAG_TXPWR_OVERRIDE);
	else
		mboolclr(proxd_bsscfg_cubby->flags, PROXD_FLAG_TXPWR_OVERRIDE);

	/* Getting bcmc scb for bsscfg */
	if (method == PROXD_RSSI_METHOD) {
		scb = bsscfg->bcmc_scb[CHSPEC_WLCBANDUNIT(chanspec)];
		ASSERT(scb != NULL);
	} else {
		scb = NULL;
	}

	if (fn) {
		wlc_pcb_fn_register(wlc->pcb, fn, pdsvc->cur_mif, pkt);
	}

	/* put into queue and then transmit */
	if (!wlc_queue_80211_frag(wlc, pkt, wlc->active_queue, scb, bsscfg, FALSE, NULL,
		rate_override))
		return BCME_ERROR;

	/* WLF2_PCB1_AF callback is not needed because the action frame was not
	 * initiated from Host. More importantly, queueing up WLC_E_ACTION_FRAME_COMPLETE event
	 * which would be done in the callback would keep the device from going into sleep.
	 */

	return BCME_OK;
}

/* This is a notify call back to the method to inform DHD on proximity detection */
static int
wlc_proxd_notify(void *ctx, struct ether_addr *ea, uint result, uint status,
	uint8 *body, int body_len)
{
	wlc_pdsvc_info_t* pdsvc = ctx;
	wlc_info_t *wlc;
	wlc_bsscfg_t *bsscfg;

	PROXD_TRACE(("%s result:%d status :%d\n",
		__FUNCTION__, result, status));

	ASSERT(pdsvc != NULL);

	wlc = pdsvc->wlc;
	bsscfg = pdsvc->bsscfg;

	wlc_bss_mac_event(wlc, bsscfg, WLC_E_PROXD, ea, result, status, 0,
		body, body_len);

	return BCME_OK;
}

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

	if ((CHIPID(pdsvc->wlc->pub->sih->chip)) == BCM4360_CHIP_ID ||
		(CHIPID(pdsvc->wlc->pub->sih->chip)) == BCM43460_CHIP_ID) {
		factor = ((pdsvc->pllreg * 1000) << shift);
	} else {
		factor = (((pdsvc->pllreg & PMU1_PLL0_PC1_M1DIV_MASK) * 1000 * 2) << shift);
	}
	factor = factor / pdsvc->fvco;
	if (ki)
		*ki = pdsvc->ki;
	if (kt)
		*kt = pdsvc->kt;

	PROXD_TRACE(("Shift:%d, pllreg:%x , fvco:%d, factor:%d\n",
		shift, pdsvc->pllreg, pdsvc->fvco, factor));

	return factor;
}

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
	/* Hook up the callback interfaces */
	pdsvc->funcs.txaf = wlc_proxd_transmitaf;
	pdsvc->funcs.notify = wlc_proxd_notify;
	pdsvc->funcs.clock_factor = wlc_proxd_AVB_clock_factor;
	pdsvc->funcs.notifyptr = pdsvc;

	/*  attach create_fn for currrently implemented PD methods  */
	pdsvc->method_create_fn[PROXD_RSSI_METHOD-1] = NULL;
	pdsvc->method_create_fn[PROXD_TOF_METHOD-1] =
		wlc_pdtof_create_method;
	/* TODO: pdsvc->method_create_fn[PROXD_AOA_METHOD-1] = wlc_pdaoa_create_method;  */

	/* LEGACY stuff, TODO: move rssi related init into the pdrssi module */
	wlc_pdsvc_init(pdsvc);

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
		wlc_proxd_doiovar, NULL, NULL, NULL);
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

	wlc_bsscfg_type_register(wlc, BSSCFG_TYPE_PROXD,
		(bsscfg_type_init_t)wlc_proxd_bsscfg_init, pdsvc);
	return pdsvc;

fail:
	if (pdsvc != NULL) {
		(void)wlc_module_unregister(wlc->pub, PROXD_NAME, pdsvc);
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

	/* Stop proximity */
	(void) wlc_proxd_stop((void *)pdsvc);

	/* This is just to clean up the memory if unloading happens before disabling the method */
	if (pdsvc->cur_mif) {
		(*pdsvc->cur_mif->mrelease)(pdsvc->cur_mif);
	}
	if (pdsvc->bsscfg) {
		wlc_proxd_ifdel(pdsvc);
	}
	wlc_module_unregister(wlc->pub, "proxd", pdsvc);
	MFREE(wlc->osh, pdsvc, sizeof(wlc_pdsvc_info_t));
	return ++callbacks;
}

/* wlc calls to receive the action frames */
int
wlc_proxd_recv_action_frames(wlc_info_t *wlc, wlc_bsscfg_t *bsscfg,
	struct dot11_management_header *hdr, uint8 *body, int body_len,
	wlc_d11rxhdr_t *wrxh, uint32 rspec)
{
	wlc_pdsvc_info_t* pdsvc;

	ASSERT(wlc != NULL);
	ASSERT(body != NULL);
	BCM_REFERENCE(bsscfg);

	pdsvc = wlc->pdsvc_info;
	/* Push the frame to method */
	if (pdsvc->cur_mif && pdsvc->cur_mif->mpushaf)
		(*pdsvc->cur_mif->mpushaf)(pdsvc->cur_mif, &hdr->sa, &hdr->da, wrxh,
			body, body_len, rspec);

	return 1;
}

void wlc_proxd_tx_conf(wlc_info_t *wlc, uint16 *phyctl, uint16 *mch, wlc_pkttag_t *pkttag)
{
	wlc_pdsvc_info_t* pdsvc;

	ASSERT(wlc != NULL);

	pdsvc = wlc->pdsvc_info;

	if (pdsvc && pdsvc->method == PROXD_TOF_METHOD && pkttag &&
		((pkttag->shared.packetid & 0xffff0000) == PROXD_FTM_PACKET_TAG))
	{
		uint16 mask;

			/* measurement packet using one antenna to tx */
		mask = (wlc_pdtof_get_tx_mask(pdsvc->cur_mif) << D11AC_PHY_TXC_CORE_SHIFT);
		*phyctl = (*phyctl & ~D11AC_PHY_TXC_CORE_MASK) | mask;

		/* signal ucode to enable timestamping for this frame */
		*mch |= D11AC_TXC_TOF;
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

uint32 wlc_pdsvc_average(uint32 *arr, int n)
{
	int total;
	int i;
	uint32 ret;

	if (n > 1) {
		i = 1;
		total = 0;
		while (i < n)
			total += (arr[i++]-arr[0]);
		total = total*100/n;
		ret = total/100+arr[0];
		if (total%100 >= 50)
			ret++;
		return ret;
	} else if (n == 1)
		return arr[0];
	return 0;
}

uint32 wlc_pdsvc_deviation(uint32 *arr, int n, uint8 decimaldigits)
{
	uint32 sum = 0, mean;
	int i, diff;

	if (n == 0)
		return 0;
	if (decimaldigits > 3)
		decimaldigits = 3;
	mean = wlc_pdsvc_average(arr, n);
	for (i = 0; i < n; i++) {
		diff = arr[i] - mean;
		sum += diff * diff;
	}
	for (i = 0; i < decimaldigits; i++)
		sum *= 100;

	return wlc_pdsvc_sqrt(sum/n);
}

static int wlc_pdsvc_func(wlc_info_t *wlc, uint8 action, chanspec_t chanspec,
	struct ether_addr *addr, int8 frmcnt, int8 retrycnt, int timeout, uint32 flags)
{
	wlc_pdsvc_info_t* pdsvc;
	wl_proxd_params_tof_method_t tofparam;
	wl_proxd_params_tof_tune_t toftune;
	int ret = 0;

	ASSERT(wlc != NULL);
	ASSERT(wlc->pdsvc_info != NULL);

	pdsvc = wlc->pdsvc_info;
	if (pdsvc && pdsvc->cur_mif) {
		if (action == WL_PROXD_ACTION_START) {
			pdsvc->cur_mif->rw_params(pdsvc->cur_mif, &tofparam,
				sizeof(wl_proxd_params_tof_method_t), 0);
			tofparam.chanspec = chanspec;
			if (timeout != -1)
				tofparam.timeout = timeout;
			if (frmcnt != -1)
				tofparam.ftm_cnt = frmcnt;
			if (retrycnt != -1)
				tofparam.retry_cnt = retrycnt;
			bcopy(addr, &tofparam.tgt_mac, ETHER_ADDR_LEN);
			if (flags & WL_PROXD_FLAG_ONEWAY) {
				/* One side using 6M legacy rate */
				tofparam.tx_rate = 12;
				tofparam.vht_rate = WL_RSPEC_ENCODE_RATE >> 16;
			} else {
				tofparam.tx_rate = 1 << WL_RSPEC_VHT_NSS_SHIFT;
				tofparam.vht_rate = WL_RSPEC_ENCODE_VHT >> 16;
			}
			pdsvc->cur_mif->rw_params(pdsvc->cur_mif, &tofparam,
				sizeof(wl_proxd_params_tof_method_t), 1);
			wlc_pdtof_get_tune(pdsvc->cur_mif, &toftune,
				sizeof(wl_proxd_params_tof_tune_t));
			toftune.flags = flags;
			if (toftune.flags & WL_PROXD_FLAG_SEQ_EN)
			{
				toftune.seq_en = 1;
			} else {
				toftune.seq_en = 0;
			}

			wlc_pdtof_set_tune(pdsvc->cur_mif, &toftune,
				sizeof(wl_proxd_params_tof_tune_t));
		}

		(*pdsvc->cur_mif->mconfig)(pdsvc->cur_mif,
			WL_PROXD_MODE_INITIATOR, pdsvc->bsscfg);
		ret = (*pdsvc->cur_mif->mstart)(pdsvc->cur_mif, (action != WL_PROXD_ACTION_STOP));
	}
	return ret;
}

pdsvc_func_t wlc_pdsvc_register(wlc_info_t *wlc, wlc_bsscfg_t *bsscfg, notifypd notify,
	void *notifyptr, int8 fmtcnt, struct ether_addr *allow_addr, bool setonly, uint32 flags)
{
	wlc_pdsvc_info_t* pdsvc;
	wl_proxd_params_tof_method_t tofparam;
	wl_proxd_params_tof_tune_t toftune;

	ASSERT(wlc != NULL);
	ASSERT(wlc->pdsvc_info != NULL);

	pdsvc = wlc->pdsvc_info;
	pdsvc->bsscfg = bsscfg;
	if (notify) {
		pdsvc->funcs.notify = notify;
		pdsvc->funcs.notifyptr = notifyptr;
	}

	if (!setonly && !( pdsvc->cur_mif = (*pdsvc->method_create_fn[PROXD_TOF_METHOD-1])(
			wlc, WL_PROXD_MODE_TARGET, &pdsvc->funcs, NULL,
			&pdsvc->payload))) {
		WL_ERROR(("wl%d: %s: Create TOF method failed \n",
			wlc->pub->unit, __FUNCTION__));
		return NULL;
	}

	/* Configure  created method */
	if (!setonly && pdsvc->cur_mif->mconfig) {
		/* Call the method configuration */
		(*pdsvc->cur_mif->mconfig)(pdsvc->cur_mif,
			WL_PROXD_MODE_TARGET, pdsvc->bsscfg);
	}

	pdsvc->cur_mif->rw_params(pdsvc->cur_mif, &tofparam,
		sizeof(wl_proxd_params_tof_method_t), 0);
	if (fmtcnt != -1)
	tofparam.ftm_cnt = fmtcnt;
	tofparam.tx_rate = 1 << WL_RSPEC_VHT_NSS_SHIFT;
	tofparam.vht_rate = WL_RSPEC_ENCODE_VHT >> 16;
	pdsvc->cur_mif->rw_params(pdsvc->cur_mif, &tofparam,
		sizeof(wl_proxd_params_tof_method_t), 1);

	wlc_pdtof_get_tune(pdsvc->cur_mif, &toftune, sizeof(wl_proxd_params_tof_tune_t));
	toftune.vhtack = 1;
	if (fmtcnt != -1 && fmtcnt != 0)
		toftune.totalfrmcnt = fmtcnt+1; /* limit total frames */
	toftune.flags = flags;
	wlc_pdtof_set_tune(pdsvc->cur_mif, &toftune, sizeof(wl_proxd_params_tof_tune_t));

	wlc_pdtof_allowmac(pdsvc->cur_mif, allow_addr);

	pdsvc->method = PROXD_TOF_METHOD;
	pdsvc->config.mode = WL_PROXD_MODE_TARGET;

	wlc->pub->_proxd = TRUE;

	if (!setonly)
		(*pdsvc->cur_mif->mstart)(pdsvc->cur_mif, TRUE);

	return wlc_pdsvc_func;
}

int wlc_pdsvc_deregister(wlc_info_t *wlc, pdsvc_func_t funcp)
{
	wlc_pdsvc_info_t* pdsvc;

	ASSERT(wlc != NULL);
	ASSERT(wlc->pdsvc_info != NULL);

	pdsvc = wlc->pdsvc_info;
	if (pdsvc->cur_mif && funcp) {
		/* Stop proximity */
		wlc_proxd_stop((void *)pdsvc);

		/* Release the method */
		(*pdsvc->cur_mif->mrelease)(pdsvc->cur_mif);
		pdsvc->cur_mif = NULL;
	}

	/* Initiatize all the parameters to NULL */
	pdsvc->method = PROXD_UNDEFINED_METHOD;
	pdsvc->config.mode = WL_PROXD_MODE_DISABLE;
	wlc->pub->_proxd = FALSE;

	return 0;
}

/* function to determine if the proxd is supported by the radio card */
bool wlc_is_proxd_supported(wlc_info_t *wlc)
{
	ASSERT(wlc != NULL);
	if (si_pmu_fvco_pllreg(wlc->hw->sih, NULL, NULL))
		return FALSE;
	return TRUE;
}

/* function to get report mac address list */
struct ether_addr *wlc_pdsvc_report_list(wlc_info_t *wlc, int *cntptr)
{
	wlc_pdsvc_info_t* pdsvc;

	ASSERT(wlc != NULL);
	ASSERT(wlc->pdsvc_info != NULL);

	pdsvc = wlc->pdsvc_info;
	if (cntptr)
		*cntptr = pdsvc->rptlistnum;

	return pdsvc->rptlist;
}
