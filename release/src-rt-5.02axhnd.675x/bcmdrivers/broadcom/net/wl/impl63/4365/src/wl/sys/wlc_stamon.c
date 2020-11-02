/*
 * STA monitor implementation
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
 * $Id: wlc_stamon.c 782025 2019-12-09 10:23:05Z $
 */

/**
 * @file
 * @brief
 * This is an AP/router specific feature.
 */

/**
 * @file
 * @brief
 * XXX Twiki: [STASniffingModeOnAP]
 */

#include <wlc_cfg.h>
#include <typedefs.h>
#include <bcmdefs.h>
#include <osl.h>
#include <bcmendian.h>
#include <bcmutils.h>
#include <siutils.h>
#include <wlioctl.h>
#include <d11.h>
#include <wlc_rate.h>
#include <wlc_pub.h>
#include <wlc_key.h>
#include <wlc.h>
#include <wl_export.h>
#include <wlc_hw.h>
#include <wlc_hw_priv.h>
#include <wlc_bmac.h>
#include <wlc_stamon.h>
#ifdef WL_STA_MONITOR_COMP
#include <wlc_addrmatch.h>
#include <wlc_macfltr.h>
#include <wlc_keymgmt.h>
#include <wlc_assoc.h>

/* Maximum STAs to monitor */
#define DFLT_STA_MONITOR_ENRTY 4
#define MAX_STA_MONITOR_ENRTY 8
#endif /* WL_STA_MONITOR_COMP */

/* wlc access macros */
#define WLCUNIT(x) ((x)->wlc->pub->unit)
#define WLCPUB(x) ((x)->wlc->pub)
#define WLCOSH(x) ((x)->wlc->osh)
#define WLCHW(x) ((x)->wlc->hw)
#define WLC(x) ((x)->wlc)
#ifdef WL_STA_MONITOR_COMP
#else
#define WLCREGS(x) ((x)->wlc->regs)
#define WLCCLK(x) ((x)->wlc->clk)
/* other helper macros */
#define STAMON_SUPPORT(x)	((x)->support)
#define RCM_CTL(x) (WLCREGS((x))->u_rcv.d11regs.rcm_ctl)
#define RCM_COND_DLY(x) (WLCREGS((x))->u_rcv.d11regs.rcm_cond_dly)
#endif /* WL_STA_MONITOR_COMP */
#define STAMON_MAX_OFFCHAN_TIME	50 /* in ms */
#define STAMON_ONCHAN_TIME	110 /* in ms */
#define STAMON_DISABLE_TIMER_RUNNING (1 << 0)
#define STAMON_OFFCHAN_TIMER_RUNNING (1 << 1)
#define STAMON_UPDATE_MAC_LIST_TIMER_RUNNING	(1 << 2)
#define STA_DELETE_FROM_STAMON		(1 << 0)
#define STAMON_WAIT_TIME_TO_REMOVE_ENTRY	200 /* wait(msec) to let wl module to handle
						   * all stamon packets graciously
						   */

struct wlc_stamon_sta_cfg_entry {
	struct ether_addr ea;
	bool sniff_enab; /* TRUE means this station is currently is being sniffed by ucode */
#ifdef WL_STA_MONITOR_COMP
	int8  rcmta_idx; /* index of RCMTA/AMT tables */
#endif /* WL_STA_MONITOR_COMP */
	int rssi;	/* rssi of monitored sta */
	chanspec_t chanspec;
	uint32 offchan_time;	/* Time (ms) for which off-channel STA's are monitored. */
	uint32 flags;
	uint32 delete_time;
};

struct wlc_stamon_info {
	wlc_info_t *wlc;
	struct wlc_stamon_sta_cfg_entry *stacfg;
	uint32 stacfg_memsize; /* memory size of the stacfg array */

	bool support;		/* STA monitor feature support or not */
	uint16 stacfg_num; /* Total number of currently configured STAs */
	uint16 amt_start_idx;
	uint16 amt_max_idx;
#ifdef WL_STA_MONITOR_COMP
	stamon_cnts_t stamon_cnts; /* rx total monitored data, mngt and ctl pkts of STAs */
	uint32 monitor_time;	/* Time (ms) for which STA's are monitored. */
	struct wl_timer *stamon_disable_timer;	/* STA monitor disabling timer */
	uint8 timer_status;
	struct wl_timer *offchan_timer;
	uint8 cfg_idx;
	chanspec_t chspec_return;
	struct wl_timer *update_stamon_mac_list_timer;
#endif /* WL_STA_MONITOR_COMP */
};
#define STACFG_MAX_ENTRY(_stamon_info_) \
	((_stamon_info_)->stacfg_memsize/sizeof(struct wlc_stamon_sta_cfg_entry))

/* IOVar table */
enum {
	IOV_STA_MONITOR = 0,	/* Add/delete the monitored STA's MAC addresses. */
#ifdef WL_STA_MONITOR_COMP
	IOV_STA_MONITOR_CNT = 1,
#endif /* WL_STA_MONITOR_COMP */
	IOV_LAST
};

static const bcm_iovar_t stamon_iovars[] = {
	{"sta_monitor", IOV_STA_MONITOR,
	(IOVF_SET_UP), IOVT_BUFFER, sizeof(wlc_stamon_sta_config_t)
	},
#ifdef WL_STA_MONITOR_COMP
	{"sta_monitor_cnt", IOV_STA_MONITOR_CNT, (0), IOVT_BUFFER, sizeof(stamon_cnts_t)},
#endif /* WL_STA_MONITOR_COMP */
	{NULL, 0, 0, 0, 0 }
};

/* **** Private Functions Prototypes *** */

/* Forward declarations for functions registered for this module */
static int wlc_stamon_doiovar(
		    void                *hdl,
		    const bcm_iovar_t   *vi,
		    uint32              actionid,
		    const char          *name,
		    void                *p,
		    uint                plen,
		    void                *a,
		    int                 alen,
		    int                 vsize,
		    struct wlc_if       *wlcif);
static int wlc_stamon_send_sta_info(wlc_stamon_info_t *ctxt, void* param,
	int paramlen, void* bptr, int len);
static int wlc_stamon_process_get_cmd_options(wlc_stamon_info_t *ctxt, void* param,
	int paramlen, void* bptr, int len);
#ifdef WL_STA_MONITOR_COMP
static int wlc_stamon_up(void *handle);
static int wlc_stamon_down(void *handle);
static void wlc_stamon_delete_all_stations(wlc_stamon_info_t *ctxt);
static void wlc_stamon_disable_all_stations(wlc_stamon_info_t *ctxt);
static void wlc_stamon_sta_ucode_sniff_enab(wlc_stamon_info_t *ctxt,
	int8 idx, bool enab);
static int wlc_stamon_sta_get(wlc_stamon_info_t *stamon_ctxt,
	struct maclist *ml);
static int wlc_stamon_rcmta_slots_reserve(wlc_stamon_info_t *ctxt);
static void wlc_stamon_timer_add(wlc_stamon_info_t *ctxt);
static void wlc_stamon_timer_delete(wlc_stamon_info_t *ctxt);
static void wlc_stamon_disable_timer(void *arg);
static void wlc_stamon_offchan_timer(void *arg);
static void wlc_offchan_timer_start(wlc_stamon_info_t *ctxt, int8 cfg_idx);
static void wlc_offchan_timer_delete(wlc_stamon_info_t *ctxt);
static void wlc_stamon_add_timer_update_mac_list(wlc_stamon_info_t *ctxt);
static void wlc_stamon_delete_timer_update_mac_list(wlc_stamon_info_t *ctxt);
static void wlc_update_stamon_mac_list(void *context);
#else
static bool wlc_stamon_chip_supported(wlc_stamon_info_t *stamon_ctxt);
static bool wlc_stamon_is_pre_ac_chip(wlc_stamon_info_t *stamon_ctxt);
#endif /* WL_STA_MONITOR_COMP */
/* This includes the auto generated ROM IOCTL/IOVAR patch handler C source file (if auto patching is
 * enabled). It must be included after the prototypes and declarations above (since the generated
 * source file may reference private constants, types, variables, and functions).
 */
#include <wlc_patch.h>

/* **** Public Functions *** */
/*
 * Initialize the sta monitor private context and resources.
 * Returns a pointer to the sta monitor private context, NULL on failure.
 */
wlc_stamon_info_t *
BCMATTACHFN(wlc_stamon_attach)(wlc_info_t *wlc)
{
	wlc_pub_t *pub = wlc->pub;
	wlc_stamon_info_t *stamon_ctxt;
	uint32 i = 0;
	uint32 malloc_size, stacfg_memsize;
#ifdef WL_STA_MONITOR_COMP
	char *val;
	uint8 stamon_entry = DFLT_STA_MONITOR_ENRTY;

	/*
	 * override max_stamon_entry if nvram is configured, for
	 * dynamically sta_monitor entry configuration
	 */
	val = getvar(wlc->pub->vars, "max_stamon_entry");
	if (val && (*val != '\0')) {
		stamon_entry = (uint8)bcm_atoi(val);
		if (stamon_entry > MAX_STA_MONITOR_ENRTY) {
			WL_ERROR(("wl%d: max_stamon_entry %d exceed max available "
			"entry number %d, override to %d\n", wlc->pub->unit,
			stamon_entry, MAX_STA_MONITOR_ENRTY, MAX_STA_MONITOR_ENRTY));
			stamon_entry = MAX_STA_MONITOR_ENRTY;
		}
	}

	stacfg_memsize = (sizeof(struct wlc_stamon_sta_cfg_entry) *
		stamon_entry);

	malloc_size = sizeof(wlc_stamon_info_t) + stacfg_memsize;

	stamon_ctxt = (wlc_stamon_info_t*)MALLOCZ(pub->osh, malloc_size);
	if (stamon_ctxt == NULL) {
		WL_ERROR(("wl%d: %s: stamon_ctxt MALLOCZ failed; total "
			"mallocs %d bytes\n",
			wlc->pub->unit,
			__FUNCTION__, MALLOCED(wlc->osh)));
		return NULL;
	}

	/* Initializing stamon_ctxt structure */
	stamon_ctxt->wlc = wlc;
	stamon_ctxt->stacfg_memsize = stacfg_memsize;
	stamon_ctxt->stacfg =
		(struct wlc_stamon_sta_cfg_entry*)(((uint8*)stamon_ctxt) +
		sizeof(wlc_stamon_info_t));

	stamon_ctxt->stacfg_num = 0;
	for (i = 0; i < STACFG_MAX_ENTRY(stamon_ctxt); i++) {
		bcopy(&ether_null, &stamon_ctxt->stacfg[i].ea, ETHER_ADDR_LEN);
		stamon_ctxt->stacfg[i].rcmta_idx = -1;
	}

	/* Initialize stamon disable timer */
	if (!(stamon_ctxt->stamon_disable_timer = wl_init_timer(wlc->wl, wlc_stamon_disable_timer,
		stamon_ctxt, "stamon_disable"))) {
		WL_ERROR(("wl%d: wlc_stamon_disable_timer init failed\n", wlc->pub->unit));
		return NULL;
	}

	if (!(stamon_ctxt->offchan_timer = wl_init_timer(wlc->wl, wlc_stamon_offchan_timer,
		stamon_ctxt, "stamon_offchannel"))) {
		WL_ERROR(("wl%d: wlc_stamon_offchan_timer init failed\n", wlc->pub->unit));
		return NULL;
	}
	if (!(stamon_ctxt->update_stamon_mac_list_timer = wl_init_timer(wlc->wl,
		wlc_update_stamon_mac_list, stamon_ctxt, "stamon_mac_list_update_timer"))) {
		WL_ERROR(("wl%d: wlc_stamon_offchan_timer init failed\n", wlc->pub->unit));
		return NULL;
	}
	/* register module */
	if (wlc_module_register(
			    wlc->pub,
			    stamon_iovars,
			    "sta_monitor",
			    stamon_ctxt,
			    wlc_stamon_doiovar,
			    NULL,
			    wlc_stamon_up,
			    wlc_stamon_down)) {
				WL_ERROR(("wl%d: %s wlc_module_register() failed\n",
				    WLCUNIT(stamon_ctxt),
				    __FUNCTION__));

				goto fail;
			    }
#else
	stacfg_memsize = (sizeof(struct wlc_stamon_sta_cfg_entry) *
		AMT_MAX_STA_MONITOR);
	malloc_size = sizeof(wlc_stamon_info_t) + stacfg_memsize;

	stamon_ctxt = (wlc_stamon_info_t*)MALLOCZ(pub->osh, malloc_size);
	if (stamon_ctxt == NULL) {
		WL_ERROR(("wl%d: %s: stamon_ctxt MALLOCZ failed; total "
			"mallocs %d bytes\n",
			wlc->pub->unit,
			__FUNCTION__, MALLOCED(wlc->osh)));
		return NULL;
	}

	/* Initializing stamon_ctxt structure */
	stamon_ctxt->wlc = wlc;
	stamon_ctxt->stacfg_memsize = stacfg_memsize;
	stamon_ctxt->stacfg =
		(struct wlc_stamon_sta_cfg_entry*)(((uint8*)stamon_ctxt) +
		sizeof(wlc_stamon_info_t));

	stamon_ctxt->amt_max_idx = AMT_MAXIDX_STA_MONITOR + 1;
	stamon_ctxt->amt_start_idx = stamon_ctxt->amt_max_idx -
		AMT_MAX_STA_MONITOR;
	stamon_ctxt->stacfg_num = 0;

	for (i = 0; i < STACFG_MAX_ENTRY(stamon_ctxt); i++) {
		bcopy(&ether_null, &stamon_ctxt->stacfg[i].ea, ETHER_ADDR_LEN);
		stamon_ctxt->stacfg[i].sniff_enab = FALSE;
	}

	/* Check whether current mac core revision supports STA monitor feature */
	stamon_ctxt->support = wlc_stamon_chip_supported(stamon_ctxt);

	/* register module */
	if (wlc_module_register(
			    wlc->pub,
			    stamon_iovars,
			    "sta_monitor",
			    stamon_ctxt,
			    wlc_stamon_doiovar,
			    NULL,
			    NULL,
			    NULL)) {
				WL_ERROR(("wl%d: %s wlc_module_register() failed\n",
				    WLCUNIT(stamon_ctxt),
				    __FUNCTION__));

				goto fail;
			    }

	/* Self enabling the STA monitor feature */
	wlc->pub->_stamon = TRUE;
#endif /* WL_STA_MONITOR_COMP */

	return stamon_ctxt;

fail:
	MFREE(pub->osh, stamon_ctxt, malloc_size);

	return NULL;
}

/*
 * Release net detect private context and resources.
 */
#ifdef WL_STA_MONITOR_COMP
void
BCMATTACHFN(wlc_stamon_detach)(wlc_stamon_info_t *ctxt)
{
	if (ctxt != NULL) {

		if (ctxt->stamon_disable_timer != NULL) {
			wl_free_timer(ctxt->wlc->wl, ctxt->stamon_disable_timer);
		}
		if (ctxt->offchan_timer != NULL) {
			wl_free_timer(ctxt->wlc->wl, ctxt->offchan_timer);
		}
		if (ctxt->update_stamon_mac_list_timer != NULL) {
			wl_free_timer(ctxt->wlc->wl, ctxt->update_stamon_mac_list_timer);
		}
		if (ctxt->stacfg_num) {
			/* Clearing all existing monitoring STA */
			wlc_stamon_delete_all_stations(ctxt);
		}

		if (STAMON_ENAB(WLCPUB(ctxt))) {
			/* Free RCMTA resource */
			wlc_stamon_rcmta_slots_free(ctxt);
			/* Disabling the STA monitor feature */
			WLCPUB(ctxt)->_stamon = FALSE;
		}
		/* Unregister the module */
		wlc_module_unregister(WLCPUB(ctxt), "sta_monitor", ctxt);

		/* Free the all context memory */
		MFREE(WLCOSH(ctxt), ctxt,
			(sizeof(wlc_stamon_info_t) + ctxt->stacfg_memsize));
	}
}
#else
void
BCMATTACHFN(wlc_stamon_detach)(wlc_stamon_info_t *stamon_ctxt)
{
	if (stamon_ctxt != NULL) {
		/* Disabling the STA monitor feature */
		WLCPUB(stamon_ctxt)->_stamon = FALSE;

		/* Unregister the module */
		wlc_module_unregister(WLCPUB(stamon_ctxt), "sta_monitor", stamon_ctxt);

		/* Free the all context memory */
		MFREE(WLCOSH(stamon_ctxt), stamon_ctxt,
			(sizeof(wlc_stamon_info_t) + stamon_ctxt->stacfg_memsize));
	}
}
#endif /* WL_STA_MONITOR_COMP */

#ifdef WL_STA_MONITOR_COMP
/*
 * Enable or disable sniffing of specified STA.
 *
 * Parameters:
 * ea - MAC address of STA to enable/disable sniffing.
 * enab - FALSE/TRUE - disable/enable sniffing.
 *
 * Return values:
 * BCME_UNSUPPORTED - feature is not supported.
 * BCME_BADARG - invalid argument;
 * BCME_ERROR - feature is supported but not enabled
 * BCME_NOTFOUND - entry not found.
 * BCME_OK - succeed.
 */
int
wlc_stamon_sta_sniff_enab(wlc_stamon_info_t *ctxt,
	struct ether_addr *ea, bool enab)
{
	int8 cfg_idx = 0;

#if defined(BCMDBG) || defined(WLMSG_ERROR)
	char eabuf [ETHER_ADDR_STR_LEN];
#endif /* BCMDBG || WLMSG_ERROR */

	if (ctxt == NULL)
		return BCME_UNSUPPORTED;

	if (!STAMON_ENAB(WLCPUB(ctxt)))
		return BCME_ERROR;

	if (ea == NULL) {
		WL_ERROR(("wl%d: %s: Invalid mac address pointer.\n",
			WLCUNIT(ctxt), __FUNCTION__));
		return BCME_BADARG;
					}

	/* Check MAC address validity.
	 * The MAC address must be valid unicast.
					 */
	if (ETHER_ISNULLADDR(ea) || ETHER_ISMULTI(ea)) {
#if defined(BCMDBG) || defined(WLMSG_ERROR)
		WL_ERROR(("wl%d: %s: Invalid MAC address %s.\n",
			WLCUNIT(ctxt), __FUNCTION__,
			bcm_ether_ntoa(ea, eabuf)));
#endif /* BCMDBG || WLMSG_ERROR */
		return BCME_BADARG;
					}

	/* Searching for corresponding entry in the STAs list. */
	cfg_idx = wlc_stamon_sta_find(ctxt, ea);
	if (cfg_idx < 0) {
#if defined(BCMDBG) || defined(WLMSG_ERROR)
		WL_ERROR(("wl%d: %s: STA %s not found.\n",
			WLCUNIT(ctxt), __FUNCTION__,
			bcm_ether_ntoa(ea, eabuf)));
#endif /* BCMDBG || WLMSG_INFORM */
		return BCME_NOTFOUND;
	}

	/* programming the ucode */
	wlc_stamon_sta_ucode_sniff_enab(ctxt, cfg_idx, enab);

	return BCME_OK;
}
#else
/*
 * ea:  MAC address of STA to enable/disable sniffing.
 *      NULL means enable sniffing for all configured STAs.
 * enable: FALSE/TRUE - disable/enable sniffing.
 *
 * NOTE: currently only one STA is supported.
 */
int
wlc_stamon_sta_sniff_enab(wlc_stamon_info_t *stamon_ctxt,
	struct ether_addr *ea, bool enab)
{
	uint16 attr = 0;
	uint8 i = 0;
	bool cmd_apply = FALSE;
#if defined(BCMDBG) || defined(WLMSG_INFORM)
	char eabuf [ETHER_ADDR_STR_LEN];
#endif /* BCMDBG || WLMSG_INFORM */

	if (stamon_ctxt == NULL)
		return BCME_UNSUPPORTED;

	if (!STAMON_SUPPORT(stamon_ctxt)) {
		WL_ERROR(("wl%d: %s: STA monitor feature is not supported.\n",
			WLCUNIT(stamon_ctxt), __FUNCTION__));
		return BCME_UNSUPPORTED;
	}

	for (i = 0; i < STACFG_MAX_ENTRY(stamon_ctxt); i++) {
		cmd_apply = FALSE;
		if (ea != NULL) {
			if (bcmp(ea, &stamon_ctxt->stacfg[i].ea, ETHER_ADDR_LEN) == 0)
				cmd_apply = TRUE;
		} else {
			cmd_apply = TRUE;
		}

		if (cmd_apply) {
			if (enab) {
				if (wlc_stamon_is_pre_ac_chip(stamon_ctxt)) {
					wlc_bmac_set_rxe_addrmatch(WLCHW(stamon_ctxt),
						RCM_F_BSSID_0_OFFSET, &stamon_ctxt->stacfg[i].ea);
					/* fix missing ucode inits */
					if (WLCCLK(stamon_ctxt)) {
						W_REG(WLCOSH(stamon_ctxt),
							&RCM_CTL(stamon_ctxt), 0xb);
						W_REG(WLCOSH(stamon_ctxt),
							&RCM_COND_DLY(stamon_ctxt),
							0x020b);
					}
					WL_INFORM(("wl%d: %s: Sniffing enabled for STA:[%s]\n",
						WLCUNIT(stamon_ctxt), __FUNCTION__,
						bcm_ether_ntoa(&stamon_ctxt->stacfg[i].ea, eabuf)));
				} else {
					attr = (AMT_ATTR_VALID | AMT_ATTR_A2);
					wlc_bmac_write_amt(WLCHW(stamon_ctxt),
						(stamon_ctxt->amt_start_idx + i),
						&stamon_ctxt->stacfg[i].ea, attr);

					/* XXX In order for the ucode to realize
					 * this is the "monitored" entry, the driver
					 * needs to write the AMT index into shm
					 * location (0x12 * 2).
					 *
					 * TODO: This MUST be changed if we will
					 * support multiple STA monitor feature.
					 */
					wlc_set_shm(WLC(stamon_ctxt), M_STA_MONITOR_N,
						(stamon_ctxt->amt_start_idx + i),
						sizeof(stamon_ctxt->amt_start_idx));

					stamon_ctxt->stacfg[i].sniff_enab = TRUE;

					WL_INFORM(("wl%d: %s: Sniffing enabled for "
						"STA:[%s] AMT:[attr:%04X idx:%d].\n",
						WLCUNIT(stamon_ctxt), __FUNCTION__,
						bcm_ether_ntoa(&stamon_ctxt->stacfg[i].ea, eabuf),
						attr, (stamon_ctxt->amt_start_idx + i)));
				}
			} else { /* disable */
				if (wlc_stamon_is_pre_ac_chip(stamon_ctxt)) {
					wlc_bmac_set_rxe_addrmatch(WLCHW(stamon_ctxt),
						RCM_F_BSSID_0_OFFSET, &ether_null);
					/* fix missing ucode inits */
					if (WLCCLK(stamon_ctxt)) {
						W_REG(WLCOSH(stamon_ctxt),
							&RCM_CTL(stamon_ctxt),
							0xb);
						W_REG(WLCOSH(stamon_ctxt),
							&RCM_COND_DLY(stamon_ctxt),
							0x020b);
					}
					WL_INFORM(("wl%d: %s: Sniffing disabled for STA:[%s]\n",
						WLCUNIT(stamon_ctxt), __FUNCTION__,
						bcm_ether_ntoa(&stamon_ctxt->stacfg[i].ea, eabuf)));
				} else {
					wlc_bmac_write_amt(WLCHW(stamon_ctxt),
						(stamon_ctxt->amt_start_idx + i), &ether_null, 0);

					/* XXX Disable monitored STA feature in microcode.
					* TODO: This MUST be changed if multiple STA monitor
					* feature would be supported.
					*/
					wlc_set_shm(WLC(stamon_ctxt), M_STA_MONITOR_N, 0xffff,
						sizeof(stamon_ctxt->amt_start_idx));

					stamon_ctxt->stacfg[i].sniff_enab = FALSE;

					WL_INFORM(("wl%d: %s: Sniffing disabled for "
						"STA:[%s] AMT:[attr:%04X idx:%d].\n",
						WLCUNIT(stamon_ctxt), __FUNCTION__,
						bcm_ether_ntoa(&stamon_ctxt->stacfg[i].ea, eabuf),
						attr, (stamon_ctxt->amt_start_idx + i)));
				}
			}
		}
	}

	return BCME_OK;
}
#endif /* WL_STA_MONITOR_COMP */

#ifdef WL_STA_MONITOR_COMP
/*
 * The function processing the corresponding commands passed via
 * cfg->cmd parameter.
 *
 * STAMON_CFG_CMD_ENB command enable STA monitor feature
 * in runtime.
 *
 * STAMON_CFG_CMD_DSB command disable STA monitor feature
 * in runtime.
 *
 * STAMON_CFG_CMD_ADD command adding given MAC address of the STA
 * into the STA list and automatically start sniffing. The address
 * must be valid unicast.
 *
 * STAMON_CFG_CMD_DEL command removes given MAC address of the STA
 * from the STA list and automatically stop sniffing. The address
 * must be valid unicast.
 * The BROADCAST mac address has special meaning. It indicates to
 * clear all existing stations in the list.
 *
 * Return values:
 * BCME_UNSUPPORTED - feature is not supported.
 * BCME_BADARG - invalid argument;
 * BCME_NORESOURCE - no more entry in the STA list.
 * BCME_ERROR - feature is supported but not enabled
 * BCME_NOTFOUND - entry not found.
 * BCME_OK - succeed.
 */
int
wlc_stamon_sta_config(wlc_stamon_info_t *ctxt, wlc_stamon_sta_config_t* cfg)
{
	int8 cfg_idx;
	wlc_info_t *wlc;

	if (ctxt == NULL)
		return BCME_UNSUPPORTED;

	ASSERT(cfg != NULL);

	wlc = WLC(ctxt);

	if (cfg->cmd == STAMON_CFG_CMD_DSB) {

		if (STAMON_ENAB(WLCPUB(ctxt))) {
			/* Cancel Stamon timer */
			wlc_stamon_timer_delete(ctxt);
			wlc_offchan_timer_delete(ctxt);
			/* Disable all existing monitoring STA's */
			wlc_stamon_disable_all_stations(ctxt);
			/* Delete stamon mac entry timer if running */
			wlc_stamon_delete_timer_update_mac_list(ctxt);
			/* Free RCMTA resource */
			wlc_stamon_rcmta_slots_free(ctxt);
			/* Disabling the STA monitor feature */
			WLCPUB(ctxt)->_stamon = FALSE;
		}
	} else if (cfg->cmd == STAMON_CFG_CMD_ENB) {
		if (!STAMON_ENAB(WLCPUB(ctxt))) {
			/* Reserve RCMTA resourse */
			wlc_stamon_rcmta_slots_reserve(ctxt);
			/* Enabling the STA monitor feature */
			WLCPUB(ctxt)->_stamon = TRUE;

			/* Start sniffing the frames of STA's in list */
			for (cfg_idx = 0; cfg_idx < ((int8)STACFG_MAX_ENTRY(ctxt)); cfg_idx++) {
				if (bcmp(&ether_null, &ctxt->stacfg[cfg_idx].ea,
					ETHER_ADDR_LEN) == 0) {
					continue;
				}
				wlc_stamon_sta_ucode_sniff_enab(ctxt, cfg_idx, TRUE);
			}

			wlc_stamon_timer_add(ctxt);
			/* Start offchan timer to check if any off channel sta added already */
			wlc_offchan_timer_start(ctxt, 0);
			/* Enable update mac entry timer database */
			if (ctxt->stacfg_num) {
				/* Delete stamon mac entry timer if running */
				wlc_stamon_add_timer_update_mac_list(ctxt);
			}
		}
	} else if (cfg->cmd == STAMON_CFG_CMD_SET_MONTIME) {

		if (STAMON_ENAB(WLCPUB(ctxt))) {
			return BCME_ERROR;
		}

		ctxt->monitor_time = cfg->monitor_time;

	} else if (cfg->cmd == STAMON_CFG_CMD_ADD) {

		/* Check MAC address validity. The MAC address must be valid unicast. */
		if (ETHER_ISNULLADDR(&(cfg->ea)) ||
				ETHER_ISMULTI(&(cfg->ea))) {
			WL_ERROR(("wl%d: %s: Invalid MAC address.\n",
				WLCUNIT(ctxt), __FUNCTION__));
			return BCME_BADARG;
		}
#ifdef ACKSUPR_MAC_FILTER
		/* check if there is duplicated entry in macfltr */
		if (WLC_ACKSUPR(WLC(ctxt)) &&
				wlc_macfltr_acksupr_is_duplicate(WLC(ctxt), &cfg->ea)) {
			return BCME_BADARG;
		}
#endif /* ACKSUPR_MAC_FILTER */

		if (cfg->chanspec) {
			if (!wlc_valid_chanspec(wlc->cmi, cfg->chanspec)) {
				WL_ERROR(("wl%d: %s: Invalid chspec :0x%x\n",
					WLCUNIT(ctxt), __FUNCTION__, cfg->chanspec));
				return BCME_BADARG;
			}
			if (cfg->offchan_time > STAMON_MAX_OFFCHAN_TIME) {
				WL_ERROR(("wl%d: %s: off-channel time %u, "
					"greater than Max allowed:%u\n",
					WLCUNIT(ctxt), __FUNCTION__,
					cfg->offchan_time, STAMON_MAX_OFFCHAN_TIME));
				return BCME_BADARG;
			}
		}
		/* Searching existing entry in the list. */
		cfg_idx = wlc_stamon_sta_find(ctxt, &cfg->ea);
		if (cfg_idx < 0) {
			/* Searching free entry in the list. */
			cfg_idx = wlc_stamon_sta_find(ctxt, &ether_null);
			if (cfg_idx < 0) {
				WL_ERROR(("wl%d: %s: No more entry in the list.\n",
					WLCUNIT(ctxt), __FUNCTION__));
				return BCME_NORESOURCE;
			}
			ctxt->stacfg_num++;
		}

		/* Adding MAC to the list */
		bcopy(&cfg->ea, &ctxt->stacfg[cfg_idx].ea, ETHER_ADDR_LEN);

		/* Update chanspec if specified and update its off-channel time */
		if (cfg->chanspec) {
			ctxt->stacfg[cfg_idx].chanspec = cfg->chanspec;
			ctxt->stacfg[cfg_idx].offchan_time = cfg->offchan_time;
		} else {
			ctxt->stacfg[cfg_idx].chanspec = WLC_BAND_PI_RADIO_CHANSPEC;
			ctxt->stacfg[cfg_idx].offchan_time = 0;
		}

		if (STAMON_ENAB(WLCPUB(ctxt))) {
			/* Start sniffing the frames */
			wlc_stamon_sta_ucode_sniff_enab(ctxt, cfg_idx, TRUE);
		}

		/* start off channel timer, only if the added STA is not in home channel */
		if (ctxt->stacfg[cfg_idx].chanspec != WLC_BAND_PI_RADIO_CHANSPEC) {
			wlc_offchan_timer_start(ctxt, cfg_idx);
		}

		ctxt->stacfg[cfg_idx].flags &= ~STA_DELETE_FROM_STAMON;

	} else if (cfg->cmd == STAMON_CFG_CMD_DEL) {

		uint32 dongle_time_ms = 0;
		if (!STAMON_ENAB(WLCPUB(ctxt)) && !(ctxt->stacfg_num)) {
			WL_ERROR(("wl%d: %s: Feature is not enabled.\n",
			WLCUNIT(ctxt), __FUNCTION__));
			return BCME_ERROR;
		}
		/* The broadcast mac address indicates to
		 * clear all existing stations in the list.
		 */
		if (ETHER_ISBCAST(&(cfg->ea))) {
			/* Cancel stamon timer */
			wlc_stamon_timer_delete(ctxt);
			wlc_offchan_timer_delete(ctxt);
			wlc_stamon_delete_all_stations(ctxt);
		} else {
			/* Searching specified MAC address in the list. */
			cfg_idx = wlc_stamon_sta_find(ctxt, &cfg->ea);
			if (cfg_idx < 0) {
				WL_ERROR(("wl%d: %s: Entry not found.\n",
					WLCUNIT(ctxt), __FUNCTION__));
				return BCME_NOTFOUND;
			}

			if (STAMON_ENAB(WLCPUB(ctxt))) {
				/* Stop sniffing frames from this STA */
				wlc_stamon_sta_ucode_sniff_enab(ctxt, cfg_idx, FALSE);
			}

			if (!(ctxt->stacfg[cfg_idx].flags & STA_DELETE_FROM_STAMON)) {
				/* Mark for deletion */
				ctxt->stacfg[cfg_idx].flags |= STA_DELETE_FROM_STAMON;
#if defined(BCMPCIEDEV) && defined(PROP_TXSTATUS)
				dongle_time_ms = hnd_get_reftime_ms();
#elif defined(OSL_SYSUPTIME_SUPPORT)
				dongle_time_ms = OSL_SYSUPTIME();
#endif /* BCMPCIEDEV && PROP_TXSTATUS */
				dongle_time_ms += STAMON_WAIT_TIME_TO_REMOVE_ENTRY;

				ctxt->stacfg[cfg_idx].delete_time = dongle_time_ms;
				wlc_stamon_add_timer_update_mac_list(ctxt);
			}
		}
	}
	else if (cfg->cmd == STAMON_CFG_CMD_RSTCNT) {
		ctxt->stamon_cnts.rxstamondata = 0;
		ctxt->stamon_cnts.rxstamonmngt = 0;
		ctxt->stamon_cnts.rxstamoncntl = 0;
	} else {
		return BCME_BADARG;
	}

	return BCME_OK;
}
#else
/* Add the MAC address of the STA into the STA list.
 * If "STA monitor mode" is enabled the sniffing will automatically started
 * for the frames which A2(transmitter) MAC address in 802.11 header will
 * match the specified MAC address. The new added MAC address would overwrite
 * previously added MAC address and sniffing will be continued for new MAC
 * address matches.
 * NOTE: It is allowed to add the STA MAC to STA list if "STA monitor mode"
 * is NOT enabled.
 */
int
wlc_stamon_sta_config(wlc_stamon_info_t *stamon_ctxt, wlc_stamon_sta_config_t* cfg)
{
	uint8 i;

	if (stamon_ctxt == NULL)
		return BCME_UNSUPPORTED;

	ASSERT(cfg != NULL);

	if (!STAMON_SUPPORT(stamon_ctxt)) {
		WL_ERROR(("wl%d: %s: STA monitor feature is not supported.\n",
			WLCUNIT(stamon_ctxt), __FUNCTION__));
		return BCME_UNSUPPORTED;
	}

	/* Check MAC address validity. The MAC address must be valid unicast. */
	if (ETHER_ISNULLADDR(&(cfg->ea)) ||
		!ETHER_ISUCAST(&(cfg->ea))) {
		WL_ERROR(("wl%d: %s: Attempt to configure invalid address of monitored STA.\n",
			WLCUNIT(stamon_ctxt), __FUNCTION__));
		return BCME_BADARG;
	}

	if (cfg->cmd == STAMON_CFG_CMD_ADD) {
		/* Add the MAC address of the monitored STA. */
		for (i = 0; i < STACFG_MAX_ENTRY(stamon_ctxt); i++) {
			if (bcmp(&ether_null, &stamon_ctxt->stacfg[i].ea, ETHER_ADDR_LEN) == 0) {
				/* Adding to the list */
				bcopy(&cfg->ea, &stamon_ctxt->stacfg[i].ea, ETHER_ADDR_LEN);
				stamon_ctxt->stacfg_num++;
				/* Start sniffing frames */
				wlc_stamon_sta_sniff_enab(stamon_ctxt,
					&stamon_ctxt->stacfg[i].ea, TRUE);
				break;
			}
		}
	} else if (cfg->cmd == STAMON_CFG_CMD_DEL) {
		/* Delete the MAC address of the monitored STA */
		for (i = 0; i < STACFG_MAX_ENTRY(stamon_ctxt); i++) {
			if (bcmp(&cfg->ea, &stamon_ctxt->stacfg[i].ea, ETHER_ADDR_LEN) == 0) {
				wlc_stamon_sta_sniff_enab(stamon_ctxt,
					&stamon_ctxt->stacfg[i].ea, FALSE);
				bcopy(&ether_null, &stamon_ctxt->stacfg[i].ea, ETHER_ADDR_LEN);
				stamon_ctxt->stacfg_num--;
				break;
			}
		}
	} else
		return BCME_BADARG;

	return BCME_OK;
}
#endif /* WL_STA_MONITOR_COMP */

#ifdef WL_STA_MONITOR_COMP
/* Copy configured MAC to maclist
 *
 * Input Parameters:
 * ml - mac list to copy the configured sta's MACs
 *		ml->count contains maximum number of MAC
 *		the list can carry.
 */
int
wlc_stamon_sta_get(wlc_stamon_info_t *ctxt, struct maclist *ml)
{
	uint i, j;

	if (ctxt == NULL)
		return BCME_UNSUPPORTED;

	ASSERT(ml != NULL);

	/* ml->count contains maximum number of MAC maclist can carry. */
	for (i = 0, j = 0; i < STACFG_MAX_ENTRY(ctxt) && j < ml->count; i++) {
		if (!ETHER_ISNULLADDR(&ctxt->stacfg[i].ea)) {
			bcopy(&ctxt->stacfg[i].ea, &ml->ea[j], ETHER_ADDR_LEN);
			j++;
		}
	}
	/* return the number of copied MACs to the maclist */
	ml->count = j;

	return BCME_OK;
}
#else
int
wlc_stamon_sta_get(wlc_stamon_info_t *stamon_ctxt, struct ether_addr *ea)
{
	if (stamon_ctxt == NULL)
		return BCME_UNSUPPORTED;

	ASSERT(ea != NULL);

	if (!STAMON_SUPPORT(stamon_ctxt)) {
		WL_ERROR(("wl%d: %s: STA monitor feature is not supported.\n",
			WLCUNIT(stamon_ctxt), __FUNCTION__));
		return BCME_UNSUPPORTED;
	}

	bcopy(&stamon_ctxt->stacfg[0].ea, ea, ETHER_ADDR_LEN);

	return BCME_OK;
}
#endif /* WL_STA_MONITOR_COMP */

int8
wlc_stamon_sta_find_index(wlc_stamon_info_t *stamon_ctxt, const struct ether_addr *ea)
{
	int8 i;

	if (stamon_ctxt == NULL || ea == NULL)
		return -1;

	for (i = 0; i < (int8)(STACFG_MAX_ENTRY(stamon_ctxt)); i++)
		if (bcmp(ea, &stamon_ctxt->stacfg[i].ea, ETHER_ADDR_LEN) == 0)
			return i;

	return -1;
}

#ifdef WL_STA_MONITOR_COMP
/* Searching the STA entry with given MAC address.
 * Return:
 * If entry is found return the index of the cfg entry.
 * If entry not found return -1.
 */
int8
wlc_stamon_sta_find(wlc_stamon_info_t *stamon_ctxt, const struct ether_addr *ea)
{
	return wlc_stamon_sta_find_index(stamon_ctxt, ea);
}
#else
bool
wlc_stamon_sta_find(wlc_stamon_info_t *stamon_ctxt, const struct ether_addr *ea)
{
	uint8 i = 0;

	if (stamon_ctxt == NULL)
		return FALSE;

	ASSERT(ea != NULL);

	if (!STAMON_SUPPORT(stamon_ctxt))
		return FALSE;

	for (i = 0; i < STACFG_MAX_ENTRY(stamon_ctxt); i++)
		if (bcmp(ea, &stamon_ctxt->stacfg[i].ea, ETHER_ADDR_LEN) == 0)
			return TRUE;

	return FALSE;
}
#endif /* WL_STA_MONITOR_COMP */
#ifdef ACKSUPR_MAC_FILTER
bool
wlc_stamon_acksupr_is_duplicate(wlc_info_t *wlc, struct ether_addr *ea)
{
	int i;
	wlc_stamon_info_t *stamon_ctxt;
	struct wlc_stamon_sta_cfg_entry *stacfg;
#ifdef BCMDBG_ERR
	char eabuf[20];
#endif // endif
	if (!WLC_ACKSUPR(wlc))
		return FALSE;

	stamon_ctxt = wlc->stamon_info;
	ASSERT(stamon_ctxt != NULL);

	stacfg = stamon_ctxt->stacfg;
	ASSERT(stacfg != NULL);

	for (i = 0; i < STACFG_MAX_ENTRY(stamon_ctxt); i++) {
		if (bcmp(ea, &stacfg[i].ea, ETHER_ADDR_LEN) == 0) {
			WL_ERROR(("wl%d: duplicated entry configured in"
				"stamon and macfilter for %s\n",
				wlc->pub->unit, bcm_ether_ntoa(ea, eabuf)));
			return TRUE;
		}
	}

	return FALSE;
}

bool
wlc_stamon_is_slot_reserved(wlc_info_t *wlc, int idx)
{
#ifdef WL_STA_MONITOR_COMP
	int i;
	wlc_stamon_info_t *stamon_ctxt;
	struct wlc_stamon_sta_cfg_entry *stacfg;

	if (idx < 0)
		return FALSE;

	stamon_ctxt = wlc->stamon_info;
	ASSERT(stamon_ctxt != NULL);

	stacfg = stamon_ctxt->stacfg;
	ASSERT(stacfg != NULL);

	for (i = 0; i < STACFG_MAX_ENTRY(stamon_ctxt); i++) {
		if ((stacfg[i].rcmta_idx >= 0) && (stacfg[i].rcmta_idx == idx)) {
			return TRUE;
		}
	}
#endif /* WL_STA_MONITOR_COMP */
	return FALSE;
}
#endif /* ACKSUPR_MAC_FILTER */
uint16
wlc_stamon_sta_num(wlc_stamon_info_t *stamon_ctxt)
{
	if (stamon_ctxt == NULL)
		return 0;

#ifndef WL_STA_MONITOR_COMP
	if (!STAMON_SUPPORT(stamon_ctxt))
		return 0;
#endif /* WL_STA_MONITOR_COMP */

	return stamon_ctxt->stacfg_num;
}

/* **** Private Functions *** */

static int
wlc_stamon_doiovar(
	void                *hdl,
	const bcm_iovar_t   *vi,
	uint32              actionid,
	const char          *name,
	void                *p,
	uint                plen,
	void                *a,
	int                 alen,
	int                 vsize,
	struct wlc_if       *wlcif)
{
	wlc_stamon_info_t	*stamon_ctxt = hdl;
	int			err = BCME_OK;

	BCM_REFERENCE(vi);
	BCM_REFERENCE(name);
	BCM_REFERENCE(vsize);

	switch (actionid) {
	case IOV_GVAL(IOV_STA_MONITOR):
		{

			/* Process sta_monitor and sta_monitor stats MAC command */
			err = wlc_stamon_process_get_cmd_options(stamon_ctxt, p, plen, a, alen);
			break;
		}
	case IOV_SVAL(IOV_STA_MONITOR):
		{
			wlc_stamon_sta_config_t *stamon_cfg = (wlc_stamon_sta_config_t *)a;

			if (stamon_cfg->version != STAMON_STACONFIG_VER) {
				return BCME_VERSION;
			}

			if ((alen >= STAMON_STACONFIG_LENGTH) &&
				(stamon_cfg->length >= STAMON_STACONFIG_LENGTH)) {
				err = wlc_stamon_sta_config(stamon_ctxt, stamon_cfg);
			} else {
				return BCME_BUFTOOSHORT;
			}

			break;
		}
#ifdef WL_STA_MONITOR_COMP
	case IOV_GVAL(IOV_STA_MONITOR_CNT):
		{
			stamon_cnts_t *cntrs = (stamon_cnts_t *)a;
			stamon_ctxt->stamon_cnts.version = STAMON_CNTR_VER;
			stamon_ctxt->stamon_cnts.length = sizeof(stamon_ctxt->stamon_cnts);
			if (alen < stamon_ctxt->stamon_cnts.length)
				return BCME_BUFTOOSHORT;
			*cntrs = stamon_ctxt->stamon_cnts;
			WL_ERROR(("wl%d: dt: %u, mn: %u, ct: %u\n", WLCUNIT(stamon_ctxt),
				cntrs->rxstamondata, cntrs->rxstamonmngt, cntrs->rxstamoncntl));
			break;
		}
#endif /* WL_STA_MONITOR_COMP */
	default:
		err = BCME_UNSUPPORTED;
		break;
	}

	return err;
}

#ifdef WL_STA_MONITOR_COMP
/*
 * Interface going up.
 */
static int
wlc_stamon_up(void *handle)
{
	wlc_stamon_info_t *ctxt = (wlc_stamon_info_t*)handle;

	if (ctxt == NULL)
		return BCME_UNSUPPORTED;

	if (STAMON_ENAB(WLCPUB(ctxt))) {
		/* Reserve RCMTA resourse */
		wlc_stamon_rcmta_slots_reserve(ctxt);
	}

	return BCME_OK;
}

/*
 * Interface going down.
 */
static int
wlc_stamon_down(void *handle)
{
	wlc_stamon_info_t *ctxt = (wlc_stamon_info_t*)handle;

	if (ctxt == NULL)
		return BCME_UNSUPPORTED;

	if (ctxt->stacfg_num) {
		/* Clearing all existing monitoring STA */
		wlc_stamon_delete_all_stations(ctxt);
	}

	if (STAMON_ENAB(WLCPUB(ctxt))) {
		/* Cancel Stamon timer */
		wlc_stamon_timer_delete(ctxt);
		wlc_offchan_timer_delete(ctxt);
		/* Free RCMTA resource */
		wlc_stamon_rcmta_slots_free(ctxt);
	}
	return BCME_OK;
}

/* Helper procedure function to program to
 * start/stop sniffing of the specified STA
 * in ucode.
 *
 * Parameters:
 * idx - Valid entry in the STAs list.
 * Caller must make validation.
 */
static void
wlc_stamon_sta_ucode_sniff_enab(wlc_stamon_info_t *ctxt,
	int8 idx, bool enab)
{
	uint16 amt_attr, bitmap;
	const struct ether_addr *shm_ea = NULL;
	struct wlc_stamon_sta_cfg_entry *cfg;

	cfg = &ctxt->stacfg[idx];

	/* For NULL addresses or for invalid rcmta_idx,
	 * don't issue start/stop sniff command to ucode.
	 */
	if ((bcmp(&ether_null, &cfg->ea, ETHER_ADDR_LEN) == 0) ||
		(cfg->rcmta_idx < 0)) {
		return;
	}

	/* Preparing target MAC address for the RCMTA or AMT table entry */
	shm_ea = enab ? &cfg->ea : &ether_null;

	if (D11REV_GE(WLCPUB(ctxt)->corerev, 40)) {
		amt_attr = enab ? (AMT_ATTR_VALID | AMT_ATTR_A2) : 0;
		bitmap = enab ? ADDR_STAMON_NBIT : 0;
		wlc_set_addrmatch(WLC(ctxt), cfg->rcmta_idx,
			shm_ea, amt_attr);
		wlc_write_amtinfo_by_idx(WLC(ctxt),
			cfg->rcmta_idx, bitmap);
	} else {
		amt_attr = enab ? (AMT_ATTR_VALID | AMT_ATTR_A2) : 0;
		bitmap = enab ? SKL_STAMON_NBIT : 0;
		wlc_set_addrmatch(WLC(ctxt), cfg->rcmta_idx, shm_ea, amt_attr);
		wlc_write_shm(WLC(ctxt), (D11_PRE40_M_SECKINDXALGO_BLK +
			((cfg->rcmta_idx + WLC_KEYMGMT_NUM_GROUP_KEYS) * 2)), bitmap);
	}
}

/* Clearing all monitored stations from the list.
 * Disabling STA sniffing if it is active for any
 * of the STAs from the list.
 */
static void
wlc_stamon_delete_all_stations(wlc_stamon_info_t *ctxt)
{
	int8 i;
	uint32 dongle_time_ms = 0;

	for (i = 0; i < ((int8)STACFG_MAX_ENTRY(ctxt)); i++) {
		if (ETHER_ISNULLADDR(&(ctxt->stacfg[i].ea)))
			continue;
#if defined(BCMPCIEDEV) && defined(PROP_TXSTATUS)
		dongle_time_ms = hnd_get_reftime_ms();
#elif defined(OSL_SYSUPTIME_SUPPORT)
		dongle_time_ms = OSL_SYSUPTIME();
#endif /* BCMPCIEDEV && PROP_TXSTATUS */
		/* Stop STA sniffing */
		if (STAMON_ENAB(WLCPUB(ctxt))) {
			wlc_stamon_sta_ucode_sniff_enab(ctxt, i, FALSE);
		}
		dongle_time_ms += STAMON_WAIT_TIME_TO_REMOVE_ENTRY;
		ctxt->stacfg[i].flags |= STA_DELETE_FROM_STAMON;
		ctxt->stacfg[i].delete_time = dongle_time_ms;
	}
	wlc_stamon_add_timer_update_mac_list(ctxt);
}

/* Reserve dummy slots in wlc->wsec_keys key array per
 * STA config entry.
 * By this way the key management engine would consider
 * these slot as used and wouldn't use the corresponding
 * RCMTA entries.
 * NOTE: The order of rcmta slots is not required to be
 * consecutive.
 * RCMTA indexes are stored in sta config entries.
 *
 * Return:
 * BCME_NORESOURCE - unable to allocate dummy key entry.
 * BCME_OK - succeeded.
 */
static int
wlc_stamon_rcmta_slots_reserve(wlc_stamon_info_t *ctxt)
{
	uint cfg_max_idx = STACFG_MAX_ENTRY(ctxt);
	uint cfg_idx;
	int err = BCME_OK;
	for (cfg_idx = 0; cfg_idx < cfg_max_idx; cfg_idx++) {
		ctxt->stacfg[cfg_idx].rcmta_idx =
			(int8)wlc_keymgmt_alloc_amt(WLC(ctxt)->keymgmt);

		if (ctxt->stacfg[cfg_idx].rcmta_idx < 0) {
			WL_ERROR(("wl%d %s: out of amt slots\n",
				WLCUNIT(ctxt), __FUNCTION__));
			err = BCME_NORESOURCE;
			break;
		}
	}
#ifdef ACKSUPR_MAC_FILTER
	/* update amt_info and re-locate amt table */
	if (WLC_ACKSUPR(WLC(ctxt))) {
		wlc_macfltr_addrmatch_move(WLC(ctxt));
	}
#endif /* ACKSUPR_MAC_FILTER */
	wlc_stamon_rxcounters_update(WLC(ctxt), NULL, TRUE);
	return err;
}

/* Free dummy key slots in wlc->wsec_keys
 * allocated per STA config.
 */
int
wlc_stamon_rcmta_slots_free(wlc_stamon_info_t *ctxt)
{
	uint8 cfg_max_idx = (uint8)STACFG_MAX_ENTRY(ctxt);
	uint8 cfg_idx;
	for (cfg_idx = 0; cfg_idx < cfg_max_idx; cfg_idx++) {
		/* Check for valid RCMTA index */
		if (ctxt->stacfg[cfg_idx].rcmta_idx < 0)
			continue;

		/* Invalidate the rcmta index for this STA */
		wlc_keymgmt_free_amt(WLC(ctxt)->keymgmt,
			(uint8 *)&ctxt->stacfg[cfg_idx].rcmta_idx);
	}
	wlc_stamon_rxcounters_update(WLC(ctxt), NULL, TRUE);
	return BCME_OK;
}

/* Counter for receiving monitored data, mgt and ctl pkts of monitored
 * STA's which could be used as unit test of sta_monitor functionality
 * without tcpdump
 */
void
wlc_stamon_rxcounters_update(wlc_info_t *wlc, void *p, bool reset)
{
	wlc_stamon_info_t *ctxt = wlc->stamon_info;
	struct dot11_header *mac_header = NULL;
	uint16 fc;

	if (!reset) {
		mac_header = (struct dot11_header *)(PKTDATA(wlc->pub->osh, p) + D11_PHY_HDR_LEN);
		fc = ltoh16(mac_header->fc);

		if (FC_TYPE(fc) == FC_TYPE_DATA) {
			ctxt->stamon_cnts.rxstamondata++;
		} else if (FC_TYPE(fc) == FC_TYPE_MNG) {
			ctxt->stamon_cnts.rxstamonmngt++;
		} else if (FC_TYPE(fc) == FC_TYPE_CTL) {
			ctxt->stamon_cnts.rxstamoncntl++;
		} else {
			WL_ERROR(("wl%d: Invalid frame control type %u.\n",
					wlc->pub->unit, FC_TYPE(fc)));
		}
	} else {
		ctxt->stamon_cnts.rxstamondata = 0;
		ctxt->stamon_cnts.rxstamonmngt = 0;
		ctxt->stamon_cnts.rxstamoncntl = 0;
	}
}

#else
static bool
wlc_stamon_chip_supported(wlc_stamon_info_t *stamon_ctxt)
{
	/* Check whether current mac core revision supports STA monitor feature */
	if (D11REV_IS(WLCPUB(stamon_ctxt)->corerev, 24) ||
		D11REV_IS(WLCPUB(stamon_ctxt)->corerev, 28) ||
		D11REV_IS(WLCPUB(stamon_ctxt)->corerev, 29) ||
		D11REV_IS(WLCPUB(stamon_ctxt)->corerev, 30) ||
		D11REV_IS(WLCPUB(stamon_ctxt)->corerev, 40) ||
		D11REV_IS(WLCPUB(stamon_ctxt)->corerev, 42))
		return TRUE;
	return FALSE;
}

static bool
wlc_stamon_is_pre_ac_chip(wlc_stamon_info_t *stamon_ctxt)
{
	if (D11REV_IS(WLCPUB(stamon_ctxt)->corerev, 24) ||
		D11REV_IS(WLCPUB(stamon_ctxt)->corerev, 28) ||
		D11REV_IS(WLCPUB(stamon_ctxt)->corerev, 29) ||
		D11REV_IS(WLCPUB(stamon_ctxt)->corerev, 30))
		return TRUE;
	return FALSE;
}
#endif /* WL_STA_MONITOR_COMP */

int
wlc_stamon_stats_update(wlc_info_t *wlc, const struct ether_addr* ea, int value)
{
	int idx;
	wlc_stamon_info_t *ctxt = wlc->stamon_info;
	if ((idx = wlc_stamon_sta_find_index(ctxt, ea)) != -1) {
		if (TRUE
#ifdef WL_STA_MONITOR_COMP
		/* Avoid Ghost Frames RSSI, corrupting valid monitored RSSI */
		&& (ctxt->stacfg[idx].chanspec == WLC_BAND_PI_RADIO_CHANSPEC)
#endif /* WL_STA_MONITOR_COMP */
			) {
			/* update the stats at this index */
			ctxt->stacfg[idx].rssi = value;
			return BCME_OK;
		}
	}
	return BCME_ERROR;
}

static int
wlc_stamon_send_sta_info(wlc_stamon_info_t *ctxt, void* param, int paramlen, void* bptr, int len)
{
	uint i;
	int status = BCME_ERROR;
	stamon_info_t* stainfo = NULL;

	wlc_stamon_sta_config_t *arg = (wlc_stamon_sta_config_t *)param;
	if (ctxt == NULL)
		return BCME_UNSUPPORTED;

	/* At present only 1 Mac information is filled for user
	 * request
	 */

	stainfo = (stamon_info_t*)(bptr);
	ASSERT(stainfo != NULL);
	stainfo->version = STAMON_MODULE_VER;
	stainfo->count = 0;

	for (i = 0; i < STACFG_MAX_ENTRY(ctxt); i ++) {
		if (!ETHER_ISNULLADDR(&ctxt->stacfg[i].ea)) {
			if (!eacmp(&ctxt->stacfg[i].ea, &arg->ea)) {
				memcpy(&stainfo->sta_data[0].ea, &ctxt->stacfg[i].ea,
					ETHER_ADDR_LEN);
				stainfo->sta_data[0].rssi = ctxt->stacfg[i].rssi;
				stainfo->count = 1;
				status = BCME_OK;
				break;
			}
		}
	}
	if (stainfo->count == 0) {
		/* DHD copy the response if error is BCME_OK, in this
		 * case entry is not found, but that doesn't mean
		 * IOCTL operation failed
		 */
		status = BCME_NOTFOUND;
	}
	return status;
}

static int
wlc_stamon_process_get_cmd_options(wlc_stamon_info_t *ctxt, void* param,
	int paramlen, void* bptr, int len)
{
	int err = BCME_ERROR;
	wlc_stamon_sta_config_t *stamon_cfg = (wlc_stamon_sta_config_t *)param;

	if (stamon_cfg->cmd == STAMON_CFG_CMD_GET_STATS) {
		if (len < (int)((ctxt->stacfg_num * (sizeof(*(ctxt->stacfg))))
			+ sizeof(int))) {
			return BCME_BUFTOOSHORT;
		}
		err = wlc_stamon_send_sta_info(ctxt, param, paramlen, bptr, len);
	} else {
#ifdef WL_STA_MONITOR_COMP
		struct maclist *maclist;
		maclist = (struct maclist *) bptr;
		if (len < (int)(sizeof(maclist->count) +
				(ctxt->stacfg_num * sizeof(*(maclist->ea))))) {
			return BCME_BUFTOOSHORT;
		}
		err = wlc_stamon_sta_get(ctxt, maclist);
#else
		wlc_stamon_sta_config_t *stamon_cfg = (wlc_stamon_sta_config_t*)bptr;
		if (len < (int)sizeof(wlc_stamon_sta_config_t))
			return BCME_BUFTOOSHORT;

		err = wlc_stamon_sta_get(ctxt, &stamon_cfg->ea);
#endif /* WL_STA_MONITOR_COMP */
	}
	return err;
}

#ifdef WL_STA_MONITOR_COMP
static void
wlc_stamon_disable_all_stations(wlc_stamon_info_t *ctxt)
{
	int8 cfg_idx;

	for (cfg_idx = 0; cfg_idx < ((int8)STACFG_MAX_ENTRY(ctxt)); cfg_idx++) {
		/* Stop STA sniffing */
		if (STAMON_ENAB(WLCPUB(ctxt))) {
			wlc_stamon_sta_ucode_sniff_enab(ctxt, cfg_idx, FALSE);
		}
	}
}

static void
wlc_stamon_add_timer_update_mac_list(wlc_stamon_info_t *ctxt)
{
	wlc_info_t *wlc = ctxt->wlc;

	if ((ctxt->update_stamon_mac_list_timer != NULL) &&
		!(ctxt->timer_status & STAMON_UPDATE_MAC_LIST_TIMER_RUNNING)) {
			wl_add_timer(wlc->wl, ctxt->update_stamon_mac_list_timer,
				STAMON_WAIT_TIME_TO_REMOVE_ENTRY, TRUE);
			ctxt->timer_status |= STAMON_UPDATE_MAC_LIST_TIMER_RUNNING;

	}
}

static void
wlc_stamon_delete_timer_update_mac_list(wlc_stamon_info_t *ctxt)
{
	wlc_info_t *wlc = ctxt->wlc;

	if ((ctxt->update_stamon_mac_list_timer != NULL) &&
		(ctxt->timer_status & STAMON_UPDATE_MAC_LIST_TIMER_RUNNING)) {
		wl_del_timer(wlc->wl, ctxt->update_stamon_mac_list_timer);
	}
	ctxt->timer_status &= ~STAMON_UPDATE_MAC_LIST_TIMER_RUNNING;
}
static void
wlc_stamon_timer_add(wlc_stamon_info_t *ctxt)
{
	wlc_info_t *wlc = ctxt->wlc;

	if ((ctxt->stamon_disable_timer != NULL) &&
		!(ctxt->timer_status & STAMON_DISABLE_TIMER_RUNNING)) {
		if (ctxt->monitor_time > 0) {
			wl_add_timer(wlc->wl, ctxt->stamon_disable_timer,
				ctxt->monitor_time, FALSE);
			ctxt->timer_status |= STAMON_DISABLE_TIMER_RUNNING;
		}
	}
}

static void
wlc_stamon_timer_delete(wlc_stamon_info_t *ctxt)
{
	wlc_info_t *wlc = ctxt->wlc;

	ctxt->monitor_time = 0;

	if (ctxt->timer_status & STAMON_DISABLE_TIMER_RUNNING) {
		if (ctxt->stamon_disable_timer != NULL) {
			wl_del_timer(wlc->wl, ctxt->stamon_disable_timer);
		}
		ctxt->timer_status &= ~STAMON_DISABLE_TIMER_RUNNING;
	}
}

static void
wlc_stamon_disable_timer(void *arg)
{
	wlc_stamon_info_t *ctxt = (wlc_stamon_info_t *) arg;

	if (ctxt->stacfg_num) {
		/* Disable all existing monitoring STA's */
		wlc_stamon_disable_all_stations(ctxt);
	}

	if (STAMON_ENAB(WLCPUB(ctxt))) {
		/* Free RCMTA resource */
		wlc_stamon_rcmta_slots_free(ctxt);
		/* Disable STA monitor feature */
		WLCPUB(ctxt)->_stamon = FALSE;
	}
	ctxt->monitor_time = 0;
	ctxt->timer_status &= ~STAMON_DISABLE_TIMER_RUNNING;
}

static void
wlc_offchan_timer_start(wlc_stamon_info_t *ctxt, int8 cfg_idx)
{
	if ((ctxt->offchan_timer != NULL) &&
		!(ctxt->timer_status & STAMON_OFFCHAN_TIMER_RUNNING)) {
		ctxt->cfg_idx = cfg_idx;
		wlc_stamon_offchan_timer(ctxt);
	}
}

static void
wlc_offchan_timer_delete(wlc_stamon_info_t *ctxt)
{
	wlc_info_t *wlc = ctxt->wlc;

	if ((ctxt->offchan_timer != NULL) &&
		(ctxt->timer_status & STAMON_OFFCHAN_TIMER_RUNNING)) {
		wl_del_timer(wlc->wl, ctxt->offchan_timer);
	}
	ctxt->timer_status &= ~STAMON_OFFCHAN_TIMER_RUNNING;

	/* Make sure AP is back in home channel when deleting timer */
	if (ctxt->chspec_return) {
		wlc_bmac_suspend_mac_and_wait(wlc->hw);
		wlc_bmac_set_chanspec(wlc->hw, ctxt->chspec_return,
			(wlc_quiet_chanspec(wlc->cmi, ctxt->chspec_return) != 0), NULL);
		wlc_bmac_enable_mac(wlc->hw);
		ctxt->chspec_return = 0;
	}
}

static void
wlc_stamon_offchan_timer(void *arg)
{
	wlc_stamon_info_t *ctxt = (wlc_stamon_info_t *) arg;
	wlc_info_t *wlc = ctxt->wlc;
	uint8 i;

	/* chspec_return is set when AP goes off channel. Go back to home channel if it is set */
	if (ctxt->chspec_return) {
		wlc_bmac_suspend_mac_and_wait(wlc->hw);
		wlc_bmac_set_chanspec(wlc->hw, ctxt->chspec_return,
			(wlc_quiet_chanspec(wlc->cmi, ctxt->chspec_return) != 0), NULL);
		wlc_bmac_enable_mac(wlc->hw);

		/* Stay in home channel for at leaset ONCHAN_TIME, before next off channel */
		ctxt->chspec_return = 0;
		wl_add_timer(wlc->wl, ctxt->offchan_timer, STAMON_ONCHAN_TIME, FALSE);
		ctxt->timer_status |= STAMON_OFFCHAN_TIMER_RUNNING;
		return;
	}

	/* Now, timer need to run only if AP goes off channel */
	ctxt->timer_status &= ~STAMON_OFFCHAN_TIMER_RUNNING;

	if (!STAMON_ENAB(WLCPUB(ctxt)) || !(ctxt->stacfg_num)) {
		return;
	}

	/* Start from index 0, if stamon has gone through all STAs in list */
	if (ctxt->cfg_idx >= ctxt->stacfg_num) {
		ctxt->cfg_idx = 0;
	}

	/* loop thorugh all STAs only once starting from current index */
	i = ctxt->cfg_idx;
	do {
		/* Check if off channel required and allowed */
		if (ctxt->stacfg[i].chanspec != WLC_BAND_PI_RADIO_CHANSPEC &&
			ctxt->stacfg[i].offchan_time != 0 &&
			wlc_mac_request_entry(wlc, NULL, WLC_ACTION_SCAN) == BCME_OK) {
			/* Going off channel. Record home chanspec */
			ctxt->chspec_return = WLC_BAND_PI_RADIO_CHANSPEC;

			wlc_bmac_suspend_mac_and_wait(wlc->hw);
			wlc_bmac_set_chanspec(wlc->hw, ctxt->stacfg[i].chanspec, 1, NULL);
			wlc_bmac_enable_mac(wlc->hw);

			/* Start off channel timer */
			wl_add_timer(wlc->wl, ctxt->offchan_timer,
				ctxt->stacfg[i].offchan_time, FALSE);
			ctxt->timer_status |= STAMON_OFFCHAN_TIMER_RUNNING;
			ctxt->stacfg[i].offchan_time = 0;
			ctxt->cfg_idx = i + 1;
			break;
		}
		if (++i == ctxt->stacfg_num) {
			 i = 0;
		}
		if (i == ctxt->cfg_idx) {
			break;
		}
	} while (1);
}

static void
wlc_update_stamon_mac_list(void *context)
{
	uint32 idx = 0;
	uint32 current_msec = 0;
	bool stop_update_mac_list_timer = TRUE;
	wlc_stamon_info_t *ctxt = (wlc_stamon_info_t*)context;

	if ((!ctxt->stacfg) || !(ctxt->stacfg_num)) {
		return;
	}
#if defined(BCMPCIEDEV) && defined(PROP_TXSTATUS)
	current_msec = hnd_get_reftime_ms();
#elif defined(OSL_SYSUPTIME_SUPPORT)
	current_msec = OSL_SYSUPTIME();
#endif /* BCMPCIEDEV && PROP_TXSTATUS */
	for (idx = 0; idx < STACFG_MAX_ENTRY(ctxt); idx++) {
		if (ctxt->stacfg[idx].flags & STA_DELETE_FROM_STAMON) {
			if (current_msec >= ctxt->stacfg[idx].delete_time) {
				bcopy(&ether_null, &ctxt->stacfg[idx].ea, ETHER_ADDR_LEN);
				ctxt->stacfg[idx].rssi = 0;
				ctxt->stacfg_num--;
				ctxt->stacfg[idx].flags &=  ~STA_DELETE_FROM_STAMON;
			}
		}
	}
	/* stop the timer in case no more sta marked for deletion */
	for (idx = 0; idx < STACFG_MAX_ENTRY(ctxt); idx++) {
		if (ctxt->stacfg[idx].flags & STA_DELETE_FROM_STAMON) {
			stop_update_mac_list_timer = FALSE;
			break;
		}
	}

	if ((ctxt->stacfg_num == 0) || stop_update_mac_list_timer) {
		/* stop the timer */
		wlc_stamon_delete_timer_update_mac_list(ctxt);
	}
}
#endif /* WLC_STA_MONITOR_COMP */
