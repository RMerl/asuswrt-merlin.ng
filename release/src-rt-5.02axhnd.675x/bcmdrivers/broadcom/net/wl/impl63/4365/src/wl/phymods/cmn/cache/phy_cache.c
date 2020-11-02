/*
 * CACHE module implementation
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
#include <bcmutils.h>
#include <bcmwifi_channels.h>
#include <phy_dbg.h>
#include <phy_mem.h>
#include <phy_api.h>
#include <phy_chanmgr_notif.h>
#include "phy_cache_cfg.h"
#include <phy_type_cache.h>

/* cache registry entry */
typedef struct {
	phy_cache_init_fn_t init;
	phy_cache_save_fn_t save;
	phy_cache_restore_fn_t restore;
	phy_cache_dump_fn_t dump;
	phy_cache_ctx_t *ctx;
	uint32 offset;
	uint32 flags;	/* See PHY_CACHE_FLAG_XXXX */
} phy_cache_reg_t;

/* cache control entry */
typedef struct {
	uint8 chan;	/* control channel # */
	uint8 state;
	uint16 ttdel;	/* time to delete */
} phy_cache_ctl_t;

/* state */
#define CTL_ST_USED	(1<<0)

/* module private states */
struct phy_cache_info {
	phy_info_t *pi;
	phy_type_cache_fns_t	*fns;	/* PHY specific function ptrs */

	/* control index */
	uint8 ctl_cur;

	/* cache registry */
	uint8 reg_sz;
	uint8 reg_cnt;
	phy_cache_reg_t *reg;
	/* entry size */
	uint32 bufsz;
	/* control flags */
	uint16 *flags;

	/* cache control */
	uint8 ctl_sz;
	phy_cache_ctl_t *ctl;

	/* cache entries */
	uint8 **bufp;
};

/* control index */
#define CTL_CUR_INV	255		/* invalid index */

/* control flags */
#define CTL_FLAG_VAL	(1<<0)	/* valid flag */

/* module private states memory layout */
typedef struct {
	phy_cache_info_t info;
	phy_cache_reg_t reg[PHY_CACHE_REG_SZ];
	uint16 flags[PHY_CACHE_REG_SZ * PHY_CACHE_SZ];
	phy_cache_ctl_t ctl[PHY_CACHE_SZ];
	uint8 *bufp[PHY_CACHE_SZ];
} phy_cache_mem_t;

/* local function declaration */
static int phy_cache_init(phy_init_ctx_t *ctx);
static int phy_cache_chspec_notif(phy_chanmgr_notif_ctx_t *ctx, phy_chanmgr_notif_data_t *data);
#if defined(BCMDBG) || defined(BCMDBG_DUMP)
static int phy_cache_dump(void *ctx, struct bcmstrbuf *b);
#endif // endif

/* attach/detach */
phy_cache_info_t *
BCMATTACHFN(phy_cache_attach)(phy_info_t *pi)
{
	phy_cache_info_t *info;
	uint16 events = (PHY_CHANMGR_NOTIF_OPCHCTX_OPEN | PHY_CHANMGR_NOTIF_OPCHCTX_CLOSE |
	                 PHY_CHANMGR_NOTIF_OPCH_CHG | PHY_CHANMGR_NOTIF_CH_CHG);

	PHY_TRACE(("%s\n", __FUNCTION__));

	/* allocate attach info storage */
	if ((info = phy_malloc(pi, sizeof(phy_cache_mem_t))) == NULL) {
		PHY_ERROR(("%s: phy_malloc failed\n", __FUNCTION__));
		goto fail;
	}
	info->pi = pi;

	info->reg_sz = PHY_CACHE_REG_SZ;
	info->reg = ((phy_cache_mem_t *)info)->reg;

	info->flags = ((phy_cache_mem_t *)info)->flags;

	info->ctl_sz = PHY_CACHE_SZ;
	info->ctl = ((phy_cache_mem_t *)info)->ctl;

	info->bufp = ((phy_cache_mem_t *)info)->bufp;

	info->ctl_cur = CTL_CUR_INV;

	/* register init fn */
	if (phy_init_add_init_fn(pi->initi, phy_cache_init, info, PHY_INIT_CACHE) != BCME_OK) {
		PHY_ERROR(("%s: phy_init_add_init_fn failed\n", __FUNCTION__));
		goto fail;
	}

	/* register chspec notification callback */
	if (phy_chanmgr_notif_add_interest(pi->chanmgr_notifi,
	                phy_cache_chspec_notif, info, PHY_CHANMGR_NOTIF_ORDER_CACHE,
	                events) != BCME_OK) {
		PHY_ERROR(("%s: phy_chanmgr_notif_add_interest failed\n", __FUNCTION__));
		goto fail;
	}

#if defined(BCMDBG) || defined(BCMDBG_DUMP)
	/* register dump callback */
	phy_dbg_add_dump_fn(pi, "phycache", (phy_dump_fn_t)phy_cache_dump, info);
#endif // endif

	return info;

	/* error */
fail:
	phy_cache_detach(info);
	return NULL;
}

void
BCMATTACHFN(phy_cache_detach)(phy_cache_info_t *info)
{
	phy_info_t *pi;
	uint i;

	PHY_TRACE(("%s\n", __FUNCTION__));

	if (info == NULL) {
		PHY_INFORM(("%s: null cache module\n", __FUNCTION__));
		return;
	}

	pi = info->pi;

	/* free cache entries */
	for (i = 0; i < info->ctl_sz; i ++) {
		if (info->bufp[i] == NULL)
			continue;
		phy_mfree(pi, info->bufp[i], info->bufsz);
	}

	phy_mfree(pi, info, sizeof(phy_cache_mem_t));
}

/* register phy type specific implementations */
int
BCMATTACHFN(phy_cache_register_impl)(phy_cache_info_t *ci, phy_type_cache_fns_t *fns)
{
	PHY_TRACE(("%s\n", __FUNCTION__));

	*ci->fns = *fns;
	return BCME_OK;
}

void
BCMATTACHFN(phy_cache_unregister_impl)(phy_cache_info_t *ci)
{
	PHY_TRACE(("%s\n", __FUNCTION__));
}

/*
 * Reserve cubby in cache entry and register operation callbacks for the cubby.
 */
int
BCMATTACHFN(phy_cache_reserve_cubby)(phy_cache_info_t *ci, phy_cache_init_fn_t init,
	phy_cache_save_fn_t save, phy_cache_restore_fn_t restore, phy_cache_dump_fn_t dump,
	phy_cache_ctx_t *ctx, uint32 size, uint32 flags, phy_cache_cubby_id_t *ccid)
{
	uint reg;
	uint32 offset;

	PHY_TRACE(("%s: size %u\n", __FUNCTION__, size));

	/* check registry occupancy */
	if (ci->reg_cnt == ci->reg_sz) {
		PHY_ERROR(("%s: too many cache control entries\n", __FUNCTION__));
		return BCME_NORESOURCE;
	}

	/* sanity check */
	ASSERT(save != NULL);
	ASSERT(restore != NULL);
	ASSERT(size > 0);

	reg = ci->reg_cnt;
	offset = ci->bufsz;

	/* use one registry entry */
	ci->reg[reg].init = init;
	ci->reg[reg].save = save;
	ci->reg[reg].restore = restore;
	ci->reg[reg].dump = dump;
	ci->reg[reg].ctx = ctx;
	ci->reg[reg].offset = offset;
	ci->reg[reg].flags = flags;

	/* account for the size and round it up to the next pointer */
	ci->bufsz += ROUNDUP(size, sizeof(void *));
	ci->reg_cnt ++;

#if defined(BCMDBG) || defined(BCMDBG_DUMP)
#endif /* BCMDBG || BCMDBG_DUMP */

	/* use the registry index as the cubby ID */
	*ccid = (phy_cache_cubby_id_t)reg;

	return BCME_OK;
}

/* Find the control struct index (same as cache entry index) for 'chanspec' */
static int
phy_cache_find_ctl(phy_cache_info_t *ci, chanspec_t chanspec)
{
	int ctl;

	/* TODO: change to some faster search when necessary */

	for (ctl = 0; ctl < (int)ci->ctl_sz; ctl ++) {
		if ((ci->ctl[ctl].state & CTL_ST_USED) &&
		    ci->ctl[ctl].chan == wf_chspec_ctlchan(chanspec))
			return ctl;
	}

	return BCME_NOTFOUND;
}

/*
 * Create a cache entry for the 'chanspec' if one doesn't exist.
 */
static int
phy_cache_add_entry(phy_cache_info_t *ci, chanspec_t chanspec)
{
#if !PHY_CACHE_PREALLOC
	phy_info_t *pi = ci->pi;
#endif // endif
	int ctl;

	PHY_TRACE(("%s: chanspec 0x%x\n", __FUNCTION__, chanspec));

	if ((ctl = phy_cache_find_ctl(ci, chanspec)) >= 0) {
		ASSERT(ci->bufp[ctl] != NULL);
		goto init;
	}

	/* find an empty entry */
	for (ctl = 0; ctl < (int)ci->ctl_sz; ctl ++) {
		if (ci->ctl[ctl].state & CTL_ST_USED)
			continue;
#if !PHY_CACHE_PREALLOC
		if ((ci->bufp[ctl] = phy_malloc(pi, ci->bufsz)) == NULL) {
			PHY_ERROR(("%s: unable to allocate memory\n", __FUNCTION__));
			return BCME_NOMEM;
		}
#else
		ASSERT(ci->bufp[ctl] != NULL);
#endif // endif
		goto init;
	}

	return BCME_NORESOURCE;

init:
	ASSERT(ctl >= 0 && ctl < (int)ci->ctl_sz);
	ci->ctl[ctl].chan = wf_chspec_ctlchan(chanspec);
	ci->ctl[ctl].state |= CTL_ST_USED;

	return BCME_OK;
}

/*
 * Delete the cache entry for 'chanspec' if one exists.
 */
static int
phy_cache_del_entry(phy_cache_info_t *ci, chanspec_t chanspec)
{
#if !PHY_CACHE_PREALLOC
	phy_info_t *pi = ci->pi;
#endif // endif
	int ctl;

	PHY_TRACE(("%s: chanspec 0x%x\n", __FUNCTION__, chanspec));

	if ((ctl = phy_cache_find_ctl(ci, chanspec)) < 0)
		return ctl;

	ASSERT(ci->bufp[ctl] != NULL);
#if !PHY_CACHE_PREALLOC
	phy_mfree(pi, ci->bufp[ctl], ci->bufsz);
	ci->bufp[ctl] = NULL;
#endif // endif
	bzero(&ci->ctl[ctl], sizeof(ci->ctl[ctl]));
	bzero(&ci->flags[ctl * ci->reg_sz], sizeof(ci->flags[0]) * ci->reg_sz);

	return BCME_OK;
}

/* Invoke 'save' callback to save client states to cubby 'reg' of entry 'ctl' */
static int
_phy_cache_save(phy_cache_info_t *ci, uint ctl, uint reg)
{
	uint8 *p;
	int err;

	PHY_TRACE(("%s: ctl %u reg %u\n", __FUNCTION__, ctl, reg));

	ASSERT(ctl < ci->ctl_sz);
	ASSERT(reg < ci->reg_cnt);

	/* save one */
	ASSERT(ci->bufp[ctl] != NULL);
	p = ci->bufp[ctl] + ci->reg[reg].offset;
	ASSERT(ci->reg[reg].save != NULL);
	if ((err = (ci->reg[reg].save)(ci->reg[reg].ctx, p)) != BCME_OK) {
		PHY_ERROR(("%s: save %p err %d\n",
		           __FUNCTION__, ci->reg[reg].save, err));
		return err;
	}
	ci->flags[ctl * ci->reg_sz + reg] |= CTL_FLAG_VAL;

	return BCME_OK;
}

/* Automatically invoke all applicable 'save' callbacks when the cache entry 'ctl'
 * is made non-current.
 */
static int
phy_cache_auto_save(phy_cache_info_t *ci, uint ctl)
{
	uint reg;
	int err;

	PHY_TRACE(("%s: ctl %u\n", __FUNCTION__, ctl));

	/* Is this necessary? */
	ASSERT(ctl == ci->ctl_cur);

	/* save to 'auto save' cubbies */
	for (reg = 0; reg < ci->reg_cnt; reg ++) {
		if (!(ci->reg[reg].flags & PHY_CACHE_FLAG_AUTO_SAVE))
			continue;
		if ((err = _phy_cache_save(ci, ctl, reg)) != BCME_OK) {
			PHY_ERROR(("%s: auto save %u err %d\n", __FUNCTION__, reg, err));
			(void)err;
		}
	}

	return BCME_OK;
}

/*
 * Set the cache entry associated with 'chanspec' as the current and
 * restore client states through the registered 'restore' callbacks.
 */
static int
phy_cache_restore(phy_cache_info_t *ci, chanspec_t chanspec)
{
	int ctl;
	uint reg;
	uint8 *p;
	int err;

	PHY_TRACE(("%s: chanspec 0x%x\n", __FUNCTION__, chanspec));

	if ((ctl = phy_cache_find_ctl(ci, chanspec)) < 0)
		return ctl;

	/* perform auto save */
	if (ci->ctl_cur != CTL_CUR_INV)
		phy_cache_auto_save(ci, ci->ctl_cur);

	ci->ctl_cur = (uint8)ctl;

	/* restore all */
	for (reg = 0; reg < ci->reg_cnt; reg ++) {
		/* initialize client states before the cubby content is 'valid' */
		if (!(ci->flags[ctl * ci->reg_sz + reg] & CTL_FLAG_VAL)) {
			if (ci->reg[reg].init != NULL)
				(ci->reg[reg].init)(ci->reg[reg].ctx);
			continue;
		}
		/* restore client states */
		ASSERT(ci->bufp[ctl] != NULL);
		ASSERT(ci->reg[reg].offset < ci->bufsz);
		p = ci->bufp[ctl] + ci->reg[reg].offset;
		ASSERT(ci->reg[reg].restore != NULL);
		if ((err = (ci->reg[reg].restore)(ci->reg[reg].ctx, p)) != BCME_OK) {
			PHY_ERROR(("%s: restore %p err %d\n",
			           __FUNCTION__, ci->reg[reg].restore, err));
			(void)err;
		}
	}

	return BCME_OK;
}

/*
 * Save the client states to cache through the registered 'save' callback.
 */
int
phy_cache_save(phy_cache_info_t *ci, phy_cache_cubby_id_t ccid)
{
	uint ctl;
	uint reg;

	PHY_TRACE(("%s: ccid %u\n", __FUNCTION__, ccid));

	ctl = ci->ctl_cur;

	if (ctl == CTL_CUR_INV)
		return BCME_ERROR;

	ASSERT(ctl < ci->ctl_sz);

	/* 'ccid' is the registry index */
	reg = (uint)(uintptr)ccid;

	if (reg >= ci->reg_cnt)
		return BCME_BADARG;

	/* save one */
	return _phy_cache_save(ci, ctl, reg);
}

/*
 * Invalidate the current cache entry index (due to someone is
 * changing the radio chanspec without going through the cache).
 */
static void
phy_cache_inv(phy_cache_info_t *ci)
{
	PHY_TRACE(("%s\n", __FUNCTION__));

	/* perform auto save */
	if (ci->ctl_cur != CTL_CUR_INV)
		phy_cache_auto_save(ci, ci->ctl_cur);

	ci->ctl_cur = CTL_CUR_INV;
}

/* chspec notification callback */
static int
phy_cache_chspec_notif(phy_chanmgr_notif_ctx_t *ctx, phy_chanmgr_notif_data_t *data)
{
	phy_cache_info_t *ci = (phy_cache_info_t *)ctx;
	int status;

	PHY_TRACE(("%s\n", __FUNCTION__));

	switch (data->event) {
	case PHY_CHANMGR_NOTIF_OPCHCTX_OPEN:
		status = phy_cache_add_entry(ci, data->new);
		break;
	case PHY_CHANMGR_NOTIF_OPCHCTX_CLOSE:
		status = phy_cache_del_entry(ci, data->new);
		break;
	case PHY_CHANMGR_NOTIF_OPCH_CHG:
		status = phy_cache_restore(ci, data->new);
		break;
	case PHY_CHANMGR_NOTIF_CH_CHG:
		phy_cache_inv(ci);
		status = BCME_OK;
		break;
	default:
		status = BCME_ERROR;
		ASSERT(0);
		break;
	}

	return status;
}

/* allocate cache entries */
static int
WLBANDINITFN(phy_cache_init)(phy_init_ctx_t *ctx)
{
#if PHY_CACHE_PREALLOC
	phy_cache_info_t *ci = (phy_cache_info_t *)ctx;
	phy_info_t *pi = ci->pi;
	uint i;
#endif // endif

	PHY_TRACE(("%s\n", __FUNCTION__));

#if PHY_CACHE_PREALLOC
	/* allocate cache entries */
	for (i = 0; i < ci->ctl_sz; i ++) {
		if (ci->bufp[i] != NULL)
			continue;
		if ((ci->bufp[i] = phy_malloc(pi, ci->bufsz)) == NULL) {
			PHY_ERROR(("%s: unable to allocate memory\n", __FUNCTION__));
			return BCME_NOMEM;
		}
	}
#endif // endif

	return BCME_OK;
}

#if defined(BCMDBG) || defined(BCMDBG_DUMP)
static int
phy_cache_dump(void *ctx, struct bcmstrbuf *b)
{
	phy_cache_info_t *ci = (phy_cache_info_t *)ctx;
	uint i, j;
	uint8 *p;
	int err;

	bcm_bprintf(b, "cur: %u\n", ci->ctl_cur);
	bcm_bprintf(b, "reg: max %u cnt %u bufsz %u\n",
	            ci->reg_sz, ci->reg_cnt, ci->bufsz);
	for (i = 0; i < ci->reg_cnt; i ++) {
		bcm_bprintf(b, "  idx %u: init %p save %p restore %p dump %p ctx %p offset %u\n",
		            i, ci->reg[i].init, ci->reg[i].save, ci->reg[i].restore,
		            ci->reg[i].dump, ci->reg[i].ctx, ci->reg[i].offset);
	}

	bcm_bprintf(b, "ctl: sz %u\n", ci->ctl_sz);
	for (i = 0; i < ci->ctl_sz; i ++) {
		bcm_bprintf(b, "  idx %u: state 0x%x chan %u\n",
		            i, ci->ctl[i].state, ci->ctl[i].chan);
		for (j = 0; j < ci->reg_cnt; j ++) {
			bcm_bprintf(b, "    idx %u: flags 0x%x\n",
			            j, ci->flags[i * ci->reg_sz + j]);
		}
	}

	bcm_bprintf(b, "bufp: sz %u\n", ci->ctl_sz);
	for (i = 0; i < ci->ctl_sz; i ++) {
		bcm_bprintf(b, "  idx %u: bufp %p\n", i, ci->bufp[i]);
	}

	bcm_bprintf(b, "buf: sz %u\n", ci->ctl_sz);
	for (i = 0; i < ci->ctl_sz; i ++) {
		bcm_bprintf(b, "entry %u >\n", i);
		for (j = 0; j < ci->reg_cnt; j ++) {
			bcm_bprintf(b, "  idx %u: offset %u >\n", j, ci->reg[j].offset);
			if (ci->bufp[i] == NULL)
				continue;
			if (ci->reg[j].dump == NULL)
				continue;
			p = ci->bufp[i] + ci->reg[j].offset;
			err = (ci->reg[j].dump)(ci->reg[j].ctx, p, b);
			if (err != BCME_OK) {
				PHY_ERROR(("%s: dump %p err %d\n",
				           __FUNCTION__, ci->reg[j].dump, err));
			}
		}
	}

	return BCME_OK;
}
#endif /* BCMDBG || BCMDBG_DUMP */
