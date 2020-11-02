/*
 * Common (OS-independent) portion of
 * Broadcom Station Prioritization Module
 *
 * This module is used to differentiate STA type (Video STA or Data Station) by setting
 * scb priority for each scb.
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
 * $Id: wlc_staprio.c 708017 2017-06-29 14:11:45Z $
 */

/* XXX: Define wlc_cfg.h to be the first header file included as some builds
 * get their feature flags thru this file.
 */
#include <wlc_cfg.h>
#include <osl.h>
#include <typedefs.h>
#include <bcmutils.h>
#include <siutils.h>
#include <wlioctl.h>
#include <wlc_key.h>
#include <wlc_pub.h>
#include <d11.h>
#include <bcmendian.h>
#include <wlc_bsscfg.h>
#include <wlc.h>
#include <wlc_scb.h>
#include <wlc_staprio.h>

/* IOVar table */
enum {
	IOV_STAPRIO = 0,	/* get/set scb prio  */
	IOV_STAPRIO_MODE = 1,	/* staprio feature enable/disable */
	IOV_LAST		/* In case of a need to check max ID number */
};

static const bcm_iovar_t staprio_iovars[] = {
	{"staprio", IOV_STAPRIO, (0), IOVT_BUFFER, sizeof(wl_staprio_cfg_t) },
	{"staprio_mode", IOV_STAPRIO_MODE, (0), IOVT_BOOL, 0 },
	{NULL, 0, 0, 0, 0 }
};

/*
 * only one byte data (u8 prio) for now. This structure will expand to include more data
 * for media STB
 */
typedef struct staprio_scb_info {
	uint8	prio;	/* scb priority */
} staprio_scb_info_t;

typedef struct staprio_scb_cubby {
	staprio_scb_info_t *staprio_scb_info;
} staprio_scb_cubby_t;

/*
 * Structure to cache the requested mac address priority. A linked list of these is
 * attached to the wlc_staprio_info struct, entries are added through the iovar and
 * settings applied if a matching SCB exists or is created.
 * The list is preserved unless it is manually flushed.
 */
typedef struct wlc_staprio_settings {
	struct wlc_staprio_settings *next;	/* Forward link */
	struct	ether_addr ea;			/* Ether address - key */
	uint8	prio;				/* Defined priority */
} wlc_staprio_settings_t;

struct wlc_staprio_info {
	wlc_info_t	*wlc;	/* pointer to main wlc structure */
	wlc_pub_t	*pub;	/* public common code handler */
	osl_t		*osh;	/* OSL handler */
	int		scb_handle; /* scb cubby handle */
	bool		mode;	/* TRUE/FALSE: enable/disable */
	struct wlc_staprio_settings *per_ea_settings;
};

#define STAPRIO_SCB_CUBBY(staprio, scb) (staprio_scb_cubby_t *)SCB_CUBBY(scb, (staprio->scb_handle))
#define STAPRIO_SCB_INFO(staprio, scb) (STAPRIO_SCB_CUBBY(staprio, scb))->staprio_scb_info

static int wlc_staprio_doiovar(
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

static int wlc_staprio_dump(wlc_staprio_info_t *staprio, struct bcmstrbuf *b);

/*
 * wlc_staprio_settings_find() - locate the remanent settings for a specific MAC address.
 *
 * Inputs:
 *	staprio	- address of module common context structure.
 *	ea	- pointer to mac address to look up.
 *
 * Returns:
 *	The return value is a pointer to the settings structure, or NULL if none exists.
 */
static wlc_staprio_settings_t *
wlc_staprio_settings_find(wlc_staprio_info_t *staprio, struct ether_addr *ea)
{
	wlc_staprio_settings_t *settings;

	for (settings = staprio->per_ea_settings; settings; settings = settings->next) {
		if (memcmp(ea, &settings->ea, sizeof(*ea)) == 0) {
			return settings;
		}
	}
	return NULL;
}

/*
 * wlc_staprio_settings_get_prio() - find the priority setting for a given MAC address.
 *
 * Inputs:
 *	staprio	- address of module common context structure.
 *	ea	- pointer to mac address to look up.
 *	pprio	- pointer to a uint8 to return the priority.
 *
 * Returns:
 *	Returns BCME_OK if a matching prio setting was found, BCME_NOTFOUND if none was found,
 *	or another BCME_xxx error code in case of failure.
 */
static int
wlc_staprio_settings_get_prio(wlc_staprio_info_t *staprio, struct ether_addr *ea, uint8 *pprio)
{
	wlc_staprio_settings_t *settings;

	if (!staprio || !ea) {
		return BCME_BADARG;
	}

	settings = wlc_staprio_settings_find(staprio, ea);
	if (!settings) {
		return BCME_NOTFOUND;
	}

	if (pprio) {
		*pprio = settings->prio;
	}
	return BCME_OK;
}

/*
 * wlc_staprio_settings_set_prio() - set or update the priority setting for a given MAC address.
 *
 * Inputs:
 *	staprio	- address of module common context structure.
 *	ea	- pointer to mac address to look up.
 *	prio	- the priority value to set.
 *
 * Returns:
 *	Returns BCME_OK if a matching prio setting was found and updated, or another BCME_xxx
 *	error code in case of failure.
 */
static int
wlc_staprio_settings_set_prio(wlc_staprio_info_t *staprio, struct ether_addr *ea, uint8 prio)
{
	wlc_staprio_settings_t *settings;

	if (!staprio || !ea) {
		return BCME_BADARG;
	}

	settings = wlc_staprio_settings_find(staprio, ea);
	if (!settings) {
		settings = MALLOC(staprio->osh, sizeof(*settings));
		if (!settings) {
			return BCME_NOMEM;
		}
		memcpy(&settings->ea, ea, sizeof(*ea));
		settings->next = staprio->per_ea_settings;
		staprio->per_ea_settings = settings;
	}

	settings->prio = prio;
	return BCME_OK;
}

/* staprio scb cubby_init function */
static int staprio_scb_init(void *ctx, struct scb *scb)
{
	wlc_staprio_info_t *staprio = (wlc_staprio_info_t *)ctx;
	staprio_scb_cubby_t *staprio_scb_cubby = STAPRIO_SCB_CUBBY(staprio, scb);
	staprio_scb_info_t *staprio_scb_info;

	staprio_scb_info = (staprio_scb_info_t *)MALLOC(staprio->osh, sizeof(staprio_scb_info_t));
	if (!staprio_scb_info) {
		return BCME_NOMEM;
	}

	memset((void *)staprio_scb_info, 0, sizeof(staprio_scb_info_t));

	/* Get the settings, if some were defined before the scb was associated. */
	(void)wlc_staprio_settings_get_prio(staprio, &scb->ea, &staprio_scb_info->prio);

	staprio_scb_cubby->staprio_scb_info = staprio_scb_info;

	return BCME_OK;
}

/* staprio scb cubby_deinit fucntion */
static void staprio_scb_deinit(void *ctx, struct scb *scb)
{
	wlc_staprio_info_t *staprio = (wlc_staprio_info_t *)ctx;
	staprio_scb_cubby_t *staprio_scb_cubby = STAPRIO_SCB_CUBBY(staprio, scb);
	staprio_scb_info_t *staprio_scb_info = STAPRIO_SCB_INFO(staprio, scb);

	if (staprio_scb_info)
		MFREE(staprio->osh, staprio_scb_info, sizeof(staprio_scb_info_t));

	staprio_scb_cubby->staprio_scb_info = NULL;
}

/* staprio scb cubby_dump fucntion */
static void staprio_scb_dump(void *ctx, struct scb *scb, struct bcmstrbuf *b)
{
	wlc_staprio_info_t *staprio = (wlc_staprio_info_t *)ctx;
	staprio_scb_info_t *staprio_scb_info = STAPRIO_SCB_INFO(staprio, scb);

	bcm_bprintf(b, "SCB Prio: 0x%x\n", staprio_scb_info->prio);
}

/*
 * Initialize staprio module private context and resources.
 * Returns a pointer to the module private context, NULL on failure.
 */
wlc_staprio_info_t *
BCMATTACHFN(wlc_staprio_attach)(wlc_info_t *wlc)
{
	wlc_staprio_info_t *staprio;

	if (!(staprio = (wlc_staprio_info_t *)MALLOC(wlc->osh, sizeof(wlc_staprio_info_t)))) {
		WL_ERROR(("wl%d: %s: out of mem, malloced %d bytes\n",
			wlc->pub->unit, __FUNCTION__, MALLOCED(wlc->osh)));
		return NULL;
	}

	memset((void *)staprio, 0, sizeof(wlc_staprio_info_t));
	staprio->wlc = wlc;
	staprio->pub = wlc->pub;
	staprio->osh = wlc->osh;

	/* register module */
	if (wlc_module_register(wlc->pub, staprio_iovars, "staprio",
		staprio, wlc_staprio_doiovar, NULL, NULL, NULL)) {
		WL_ERROR(("wl%d: txbf wlc_module_register() failed\n", wlc->pub->unit));
		MFREE(wlc->osh, (void *)staprio, sizeof(wlc_staprio_info_t));
		return NULL;
	}
	wlc_dump_register(wlc->pub, "staprio", (dump_fn_t)wlc_staprio_dump, (void *)staprio);

	staprio->scb_handle = wlc_scb_cubby_reserve(wlc, sizeof(struct staprio_scb_cubby),
		staprio_scb_init, staprio_scb_deinit, staprio_scb_dump, (void *)staprio);
	if (staprio->scb_handle < 0) {
		WL_ERROR(("wl%d: wlc_scb_cubby_reserve() failed\n", wlc->pub->unit));
		wlc_module_unregister(wlc->pub, "staprio", staprio);
		MFREE(wlc->osh, (void *)staprio, sizeof(wlc_staprio_info_t));
		return NULL;
	}

	return staprio;
}

/* Release staprio module private context and resources. */
void
BCMATTACHFN(wlc_staprio_detach)(wlc_staprio_info_t *staprio)
{
	wlc_staprio_settings_t *settings, *next;

	if (!staprio)
		return;
	wlc_module_unregister(staprio->pub, "staprio", staprio);
	for (settings = staprio->per_ea_settings; settings; settings = next) {
		next = settings->next;
		MFREE(staprio->osh, settings, sizeof(*settings));
	}

	MFREE(staprio->osh, (void *)staprio, sizeof(wlc_staprio_info_t));
	return;
}

/* Get/Set Sta Priority */
static int
wlc_proc_staprio(wlc_staprio_info_t *staprio, wlc_bsscfg_t *bsscfg,
	wl_staprio_cfg_t *staprio_cfg, bool set)
{
	struct ether_addr *ea = &staprio_cfg->ea;
	uint8 *prio = &staprio_cfg->prio;

	wlc_info_t *wlc = staprio->wlc;
	struct scb *scb;
	staprio_scb_info_t *staprio_scb_info;

	ASSERT(staprio != NULL);
	ASSERT(bsscfg != NULL);

	if (!bsscfg || !staprio || ETHER_ISMULTI(ea)) {
		return BCME_ERROR;
	}

	if ((scb = wlc_scbfind(wlc, bsscfg, ea)) == NULL) {
		/* No matching SCB, get or update the settings */
		return (set) ?
			wlc_staprio_settings_set_prio(staprio, ea, *prio) :
			wlc_staprio_settings_get_prio(staprio, ea, prio);
	}

	staprio_scb_info = STAPRIO_SCB_INFO(staprio, scb);

	if (set) {
		staprio_scb_info->prio = *prio;
		wlc_staprio_settings_set_prio(staprio, ea, *prio); /* make settings permanent */
	} else {
		*prio = staprio_scb_info->prio;
	}
	return BCME_OK;
}

/* Handle staprio related iovars */
static int
wlc_staprio_doiovar(
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
	wlc_staprio_info_t	*staprio = hdl;
	wlc_bsscfg_t	*bsscfg;

	uint32 *ret_uint_ptr;
	int err = 0;
	wl_staprio_cfg_t *staprio_cfg;

	BCM_REFERENCE(vi);
	BCM_REFERENCE(name);
	BCM_REFERENCE(vsize);

	if (!staprio)
		return BCME_ERROR;

	ret_uint_ptr = (uint32 *)a;

	/* Resolve the bsscfg for this request. */
	bsscfg = wlc_bsscfg_find_by_wlcif(staprio->wlc, wlcif);
	ASSERT(bsscfg != NULL);

	switch (actionid) {
		case IOV_GVAL(IOV_STAPRIO_MODE):
			*ret_uint_ptr = (uint32)staprio->pub->_staprio;
			break;
		case IOV_SVAL(IOV_STAPRIO_MODE):
			staprio->pub->_staprio = (*ret_uint_ptr != 0);
			break;

		case IOV_GVAL(IOV_STAPRIO):
			if (alen < sizeof(wl_staprio_cfg_t))
			    return BCME_BUFTOOSHORT;

			staprio_cfg = (wl_staprio_cfg_t *)a;

			memcpy(staprio_cfg, p, sizeof(wl_staprio_cfg_t));
			err = wlc_proc_staprio(staprio, bsscfg, staprio_cfg, FALSE);
			break;

		case IOV_SVAL(IOV_STAPRIO):
			staprio_cfg = (wl_staprio_cfg_t *)a;
			err = wlc_proc_staprio(staprio, bsscfg, staprio_cfg, TRUE);
			break;

	    default:
			err = BCME_UNSUPPORTED;
			break;
	}

	return err;
}

/* module dump function */
static int
wlc_staprio_dump(wlc_staprio_info_t *staprio, struct bcmstrbuf *b)
{
	wlc_staprio_settings_t *settings;

	bcm_bprintf(b, "%17s %4s\n", "Station", "Prio");
	for (settings = staprio->per_ea_settings; settings; settings = settings->next) {
		bcm_bprintf(b, ""MACF" %4d\n",  ETHERP_TO_MACF(&settings->ea),
			settings->prio);
	}

	return BCME_OK;
}

/* Read Sta Priority */
bool wlc_get_scb_prio(wlc_staprio_info_t *staprio, struct scb *scb, uint8 *prio)
{
	staprio_scb_info_t *staprio_scb_info;

	if (!staprio || !scb)
		return FALSE;

	staprio_scb_info = STAPRIO_SCB_INFO(staprio, scb);

	*prio = staprio_scb_info->prio;

	return TRUE;
}
