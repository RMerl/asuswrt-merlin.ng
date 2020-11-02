/*
 * RADIO control module implementation.
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
 * $Id$
 */

#include <phy_cfg.h>
#include <typedefs.h>
#include <bcmdefs.h>
#include <bcmutils.h>
#include <phy_dbg.h>
#include <phy_mem.h>
#include <phy_init.h>
#include "phy_type_radio.h"
#include <phy_radio_api.h>
#include <phy_radio.h>
#include "phy_utils_reg.h"

/* module private states */
struct phy_radio_info {
	phy_info_t *pi;
	phy_type_radio_fns_t *fns;
};

/* module private states memory layout */
typedef struct {
	phy_radio_info_t info;
	phy_type_radio_fns_t fns;
/* add other variable size variables here at the end */
} phy_radio_mem_t;

/* local function declaration */
static int phy_radio_on(phy_init_ctx_t *ctx);
#if ((defined(BCMDBG) || defined(BCMDBG_DUMP)) && defined(DBG_PHY_IOV)) || \
	defined(BCMDBG_PHYDUMP)
static int phy_radio_dump(void *ctx, struct bcmstrbuf *b);
#endif // endif

/* attach/detach */
phy_radio_info_t *
BCMATTACHFN(phy_radio_attach)(phy_info_t *pi)
{
	phy_radio_info_t *info;
	phy_type_radio_fns_t *fns;

	PHY_TRACE(("%s\n", __FUNCTION__));

	/* allocate attach info storage */
	if ((info = phy_malloc(pi, sizeof(phy_radio_mem_t))) == NULL) {
		PHY_ERROR(("%s: phy_malloc failed\n", __FUNCTION__));
		goto fail;
	}
	info->pi = pi;

	fns = &((phy_radio_mem_t *)info)->fns;
	info->fns = fns;

	/* register radio on fn */
	if (phy_init_add_init_fn(pi->initi, phy_radio_on, info, PHY_INIT_RADIO) != BCME_OK) {
		PHY_ERROR(("%s: phy_init_add_init_fn failed\n", __FUNCTION__));
		goto fail;
	}

#if ((defined(BCMDBG) || defined(BCMDBG_DUMP)) && defined(DBG_PHY_IOV)) || \
	defined(BCMDBG_PHYDUMP)
	/* register dump callback */
	phy_dbg_add_dump_fn(pi, "radioreg", (phy_dump_fn_t)phy_radio_dump, info);
#endif // endif

	return info;

	/* error */
fail:
	phy_radio_detach(info);
	return NULL;
}

void
BCMATTACHFN(phy_radio_detach)(phy_radio_info_t *info)
{
	phy_info_t *pi;

	PHY_TRACE(("%s\n", __FUNCTION__));

	if (info == NULL) {
		PHY_INFORM(("%s: null radio module\n", __FUNCTION__));
		return;
	}

	pi = info->pi;

	phy_mfree(pi, info, sizeof(phy_radio_mem_t));
}

/* switch the radio on/off */
void
phy_radio_switch(phy_info_t *pi, bool on)
{
	phy_radio_info_t *ri = pi->radioi;
	phy_type_radio_fns_t *fns = ri->fns;
	uint mc;

	PHY_TRACE(("%s: on %d\n", __FUNCTION__, on));

	/* Return if running on QT */
	if (NORADIO_ENAB(pi->pubpi))
		return;

	mc = R_REG(pi->sh->osh, &pi->regs->maccontrol);
	if (mc & MCTL_EN_MAC) {
		PHY_ERROR(("wl%d: %s: maccontrol 0x%x has EN_MAC set\n",
		           pi->sh->unit, __FUNCTION__, mc));
	}

	if (!on) {
		wlapi_update_bt_chanspec(pi->sh->physhim, 0,
			SCAN_INPROG_PHY(pi), RM_INPROG_PHY(pi));
	}
	else {
		wlapi_update_bt_chanspec(pi->sh->physhim, pi->radio_chanspec,
			SCAN_INPROG_PHY(pi), RM_INPROG_PHY(pi));
	}

	ASSERT(fns->ctrl != NULL);
	(fns->ctrl)(fns->ctx, on);
}

/* turn radio on */
static int
WLBANDINITFN(phy_radio_on)(phy_init_ctx_t *ctx)
{
	phy_radio_info_t *ri = (phy_radio_info_t *)ctx;
	phy_type_radio_fns_t *fns = ri->fns;

	PHY_TRACE(("%s\n", __FUNCTION__));

	ASSERT(fns->on != NULL);
	(fns->on)(fns->ctx);

	return BCME_OK;
}

/* switch the radio off when switching band */
void
phy_radio_xband(phy_info_t *pi)
{
	phy_radio_info_t *ri = pi->radioi;
	phy_type_radio_fns_t *fns = ri->fns;

	PHY_TRACE(("%s\n", __FUNCTION__));

	/* Return if running on QT */
	if (NORADIO_ENAB(pi->pubpi))
		return;

	if (fns->bandx == NULL)
		return;

	(fns->bandx)(fns->ctx);
}

/* switch the radio off when initializing */
void
phy_radio_init(phy_info_t *pi)
{
	phy_radio_info_t *ri = pi->radioi;
	phy_type_radio_fns_t *fns = ri->fns;

	PHY_TRACE(("%s\n", __FUNCTION__));

	/* Return if running on QT */
	if (NORADIO_ENAB(pi->pubpi))
		return;

	if (fns->init == NULL)
		return;

	(fns->init)(fns->ctx);
}

/* query the radio idcode */
uint32
phy_radio_query_idcode(phy_radio_info_t *ri)
{
	phy_type_radio_fns_t *fns = ri->fns;
#ifdef BCMRADIOREV
	phy_info_t *pi = ri->pi;
#endif // endif
	uint32 idcode;

	PHY_TRACE(("%s\n", __FUNCTION__));

	ASSERT(fns->id != NULL);
	idcode = (fns->id)(fns->ctx);

#ifdef BCMRADIOREV
	/*
	 * Override the radiorev to a fixed value if running in QT/Sim.  This is to avoid needing
	 * a different QTDB build for each radio rev build (for the same chip).
	 * Note: if BCMRADIOREV is not known, then use whatever is read from the chip (i.e. no
	 *       override).
	 */
	(void)pi;
	if (ISSIM_ENAB(pi->sh->sih)) {
		if (ISACPHY(pi)) {
			idcode = (idcode & ~IDCODE_ACPHY_REV_MASK) |
			        (BCMRADIOREV << IDCODE_ACPHY_REV_SHIFT);
		} else {
			idcode = (idcode & ~IDCODE_REV_MASK) |
			        ((BCMRADIOREV & 0xF) << IDCODE_REV_SHIFT);
		}
	}
#endif	/* BCMRADIOREV */

	return idcode;
}

/* register phy type specific implementations */
int
BCMATTACHFN(phy_radio_register_impl)(phy_radio_info_t *ri, phy_type_radio_fns_t *fns)
{
	PHY_TRACE(("%s\n", __FUNCTION__));

	*ri->fns = *fns;

	return BCME_OK;
}

void
BCMATTACHFN(phy_radio_unregister_impl)(phy_radio_info_t *ri)
{
	PHY_TRACE(("%s\n", __FUNCTION__));
}

#if ((defined(BCMDBG) || defined(BCMDBG_DUMP)) && defined(DBG_PHY_IOV)) || \
	defined(BCMDBG_PHYDUMP)
static int
phy_radio_dump(void *ctx, struct bcmstrbuf *b)
{
	phy_radio_info_t *info = ctx;
	phy_type_radio_fns_t *fns = info->fns;
	phy_info_t *pi = info->pi;
	int ret = BCME_UNSUPPORTED;

	if (!pi->sh->clk)
		return BCME_NOCLK;

	if (fns->dump) {
		wlapi_suspend_mac_and_wait(pi->sh->physhim);
		phy_utils_phyreg_enter(pi);
		phy_utils_radioreg_enter(pi);
		ret = (fns->dump)(fns->ctx, b);
		phy_utils_radioreg_exit(pi);
		phy_utils_phyreg_exit(pi);
		wlapi_enable_mac(pi->sh->physhim);
	}

	return ret;
}
#endif // endif
