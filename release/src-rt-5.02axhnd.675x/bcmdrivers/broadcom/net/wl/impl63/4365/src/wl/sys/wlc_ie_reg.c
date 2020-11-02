/**
 * @file
 * @brief
 * IE management module interface - this is a top-level function
 * managing IE build and/or parse in all frames other than those
 * listed in wlc_ie_mgmt.c (e.g. it can be used in Action frame or
 * or Wrapper IE build/parse).
 *
 * It is used to decouple server modules who need to deal with
 * IEs owned by other features/modules and client modules who
 * 'own' these IEs.
 *
 * These steps briefly describe what modules should to do:
 * - servers create IE registries (at attach time)
 * - clients register IEs' build and/or parse callbacks
 *   with the registries
 * - servers invoke clients' callbacks.
 * - clients build/parse their IEs
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
 * $Id: wlc_ie_reg.c 708017 2017-06-29 14:11:45Z $
 */

/* XXX: Define wlc_cfg.h to be the first header file included as some builds
 * get their feature flags thru this file.
 */
#include <wlc_cfg.h>
#include <typedefs.h>
#include <bcmdefs.h>
#include <osl.h>
#include <bcmutils.h>
#include <siutils.h>
#include <wlioctl.h>
#include <d11.h>
#include <wlc_rate.h>
#include <wlc_pub.h>
#include <wlc.h>
#include <wl_dbg.h>
#include <wlc_ie_mgmt_lib.h>
#include <wlc_ie_mgmt_dbg.h>
#include <wlc_ie_reg.h>

/*
 * module info
 */
/* module private states - fixed portion */
struct wlc_ier_info {
	wlc_info_t *wlc;
	uint16 af_info_len;
	uint16 regs_cnt;
	uint16 regs_offset;
};

/* module private states - entire structure */
/* typedef struct {
 *	wlc_ier_info_t iem -- module private states
 *	wlc_ier_reg_t *reg[max_ie_regs] -- registry pointers
 * } wlc_ier_mem_t;
 */

/* module info access macros */
#define IE_REG(iem, idx) ((wlc_ier_reg_t **)((uintptr)(iem) + (iem)->regs_offset))[idx]

/*
 * registry info
 */
/* registry private states - fixed portion */
struct wlc_ier_reg {
	wlc_info_t *wlc;
	bool init;
	wlc_ier_info_t *iem;
	wlc_iem_upp_t *upp;
	uint16 af_reg_len;
	uint16 build_cnt;
	uint16 build_tag_offset;
	uint16 build_cb_offset;
	uint16 build_tt_offset;
	uint16 parse_cnt;
	uint16 parse_tag_offset;
	uint16 parse_cb_offset;
	uint16 parse_tt_offset;
};

/* registry structure */
/* typedef struct {
 *	uint8 build_tag[build_cnt]; -- 'build' tag table
 *	wlc_iem_cbe_t build_cb[build_cnt] -- 'build' callback table
 *	uint16 build_tt[IE_NUM_TT + 1]; -- 'build' tag type table
 *	uint8 parse_tag[parse_cnt]; -- 'parse' tag table
 *	wlc_iem_pe_t parse_cb[parse_cnt] -- 'parse' callback table
 *	uint16 parse_tt[IE_NUM_TT + 1]; -- 'parse' tag type table
 * } wlc_ier_reg_mem_t;
 * Notes:
 * - parallel tables
 *   parallel tables 'tag'/'prio' save a bit memory by eliminating padding
 * - 'build' and 'parse' tables relations
 *   tt table     tag table callback table
 *   +------+     +-------+ +-------+--
 *   | tag  | --> | tag a | | cb... |
 *   +------+     +-------+ +-------+--
 *   | vs   | -   | ...   | |  ...  |
 *   +------+  |  +-------+ +-------+--
 *             |  | tab x | | cb... |
 *             |  +-------+ +-------+--
 *              > | vs a  | |  ...  |
 *                +-------+ +-------+--
 *                | ...   | | cb... |
 *                +-------+ +-------+--
 *                | vs x  | | cb... |
 *                +-------+ +-------+--
 */

/* # of tag types (regular IE tag + VS IE prio/id */
#define IE_NUM_TT	2

/* tag type */
#define IE_TT_IE	0	/* regular IE tag */
#define IE_TT_PRIO	1	/* VS IE prio/id */

/* registry access macros */
#define BUILD_TAG_TBL(reg) (uint8 *)((uintptr)(reg) + (reg)->build_tag_offset)
#define BUILD_CB_TBL(reg) (wlc_iem_cbe_t *)((uintptr)(reg) + (reg)->build_cb_offset)
#define BUILD_TT_TBL(reg) (uint16 *)((uintptr)(reg) + (reg)->build_tt_offset)
#define PARSE_TAG_TBL(reg) (uint8 *)((uintptr)(reg) + (reg)->parse_tag_offset)
#define PARSE_CB_TBL(reg) (wlc_iem_pe_t *)((uintptr)(reg) + (reg)->parse_cb_offset)
#define PARSE_TT_TBL(reg) (uint16 *)((uintptr)(reg) + (reg)->parse_tt_offset)

/* registry entries total # */
#define TBL_TTL_GET(tttbl) ((tttbl)[IE_NUM_TT] - (tttbl)[0])
/* registry entries positions for tag type 'tt' */
#define TBL_POS_GET(tttbl, tt, cur, next) {	\
		ASSERT(tt < IE_NUM_TT);		\
		cur = (tttbl)[tt];		\
		next = (tttbl)[tt + 1];		\
	}
/* shift registry entries positions from 'next' (for tag type 'tt' + 1 onwards) down by 1 */
#define TBL_POS_SHIFT(ttbl, fntbl, cbs, next, tttbl, tt) {		\
		if ((next) < (cbs)) {					\
			uint mentries = (cbs) - (next);			\
			memmove(&(ttbl)[(next) + 1], &(ttbl)[next],	\
			        sizeof((ttbl)[0]) * mentries);		\
			memmove(&(fntbl)[(next) + 1], &(fntbl)[next],	\
			        sizeof((fntbl)[0]) * mentries);		\
		}							\
		{							\
		uint i;							\
		for (i = tt + 1; i <= IE_NUM_TT; i ++) {		\
			(tttbl)[i] ++;					\
		}							\
		}							\
	}

/* debugging */
#define WLCUNIT(reg) (reg)->wlc->pub->unit

/*
 * local functions
 */

/* debugging */
#if defined(BCMDBG) || defined(BCMDBG_DUMP)
static int wlc_ier_dump(void *ctx, struct bcmstrbuf *b);
static int wlc_ier_reg_dump(wlc_ier_reg_t *reg, struct bcmstrbuf *b);
#endif // endif

/* Vendor Specific IE proxy */
static uint wlc_ier_vs_calc_len_cb(void *ctx, wlc_iem_calc_data_t *data);
static int wlc_ier_vs_build_cb(void *ctx, wlc_iem_build_data_t *data);
static int wlc_ier_vs_parse_cb(void *ctx, wlc_iem_parse_data_t *data);

/*
 * module attach/detach
 */
wlc_ier_info_t *
BCMATTACHFN(wlc_ier_attach)(wlc_info_t *wlc)
{
	wlc_ier_info_t *iem;
	uint16 len;

	/* module private data length (fixed struct + variable structs) */
	len = (uint16)sizeof(wlc_ier_info_t);	/* fixed */
#if defined(BCMDBG) || defined(BCMDBG_DUMP)
	len = (uint16)ALIGN_SIZE(len, sizeof(void *));	/* registry pointers */
	len += (uint16)(sizeof(void *) * wlc->pub->tunables->max_ie_regs);
#endif // endif

	/* allocate module private data */
	if ((iem = MALLOC(wlc->osh, len)) == NULL) {
		WL_ERROR(("wl%d: %s: out of mem, malloced %d bytes\n",
		          wlc->pub->unit, __FUNCTION__, MALLOCED(wlc->osh)));
		goto fail;
	}
	bzero(iem, len);
	iem->wlc = wlc;
	iem->af_info_len = len;

	/* suballocate variable size arrays */
	len = (uint16)sizeof(wlc_ier_info_t);	/* fixed */
#if defined(BCMDBG) || defined(BCMDBG_DUMP)
	len = (uint16)ALIGN_SIZE(len, sizeof(void *));	/* registry pointers */
	iem->regs_offset = len;
	len += (uint16)(sizeof(void *) * wlc->pub->tunables->max_ie_regs);
#endif // endif

#if defined(BCMDBG) || defined(BCMDBG_DUMP)
	/* register dump routine */
	wlc_dump_register(wlc->pub, "ier", wlc_ier_dump, (void *)iem);
#endif // endif
	return iem;

fail:	/* error handling */
	wlc_ier_detach(iem);
	return NULL;
}

void
BCMATTACHFN(wlc_ier_detach)(wlc_ier_info_t *iem)
{
	wlc_info_t *wlc;
	uint16 len;

	if (iem == NULL)
		return;

	wlc = iem->wlc;
	len = iem->af_info_len;

#if defined(BCMDBG) || defined(BCMDBG_DUMP)
	/* for debugging */
	{
	int idx;
	for (idx = 0; idx < wlc->pub->tunables->max_ie_regs; idx ++) {
		ASSERT(IE_REG(iem, idx) == NULL);
	}
	}
#endif /* BCMDBG || BCMDBG_DUMP */

	wlc_module_unregister(wlc->pub, "ier", iem);

	MFREE(wlc->osh, iem, len);
}

#if defined(BCMDBG) || defined(BCMDBG_DUMP)
static int
wlc_ier_dump(void *ctx, struct bcmstrbuf *b)
{
	wlc_ier_info_t *iem = (wlc_ier_info_t *)ctx;
	wlc_info_t *wlc = iem->wlc;
	int idx;

	bcm_bprintf(b, "registries: %u\n", iem->regs_cnt);
	for (idx = 0; idx < wlc->pub->tunables->max_ie_regs; idx ++) {
		wlc_ier_reg_t *reg = IE_REG(iem, idx);
		if (reg == NULL)
			continue;
		bcm_bprintf(b, "  registry: %u\n", idx);
		wlc_ier_reg_dump(reg, b);
	}

	return BCME_OK;
}
#endif /* BCMDBG || BCMDBG_DUMP */

/*
 * Create a registry to hold 'calc_len/build' and 'parse' callbacks.
 *
 * Inputs:
 * - build_cnt: # of 'calc_len/build' entries
 * - parse_cnt: # of 'parse' entries
 *
 * A NULL return value indicates an error.
 */
wlc_ier_reg_t *
BCMATTACHFN(wlc_ier_create_registry)(wlc_ier_info_t *iem, uint16 build_cnt, uint16 parse_cnt)
{
	wlc_ier_reg_t *reg = NULL;
	wlc_info_t *wlc = iem->wlc;
	uint16 len;
	uint16 tts;

	/* find an empty entry */
	if (iem->regs_cnt >= wlc->pub->tunables->max_ie_regs) {
		WL_ERROR(("wl%d: %s: too many registries\n",
		          wlc->pub->unit, __FUNCTION__));
		goto fail;
	}

	/* include an extra entry - vs callback proxy */
	if (build_cnt > 0)
		build_cnt ++;
	if (parse_cnt > 0)
		parse_cnt ++;

	/* include an extra tag type - after the last entry */
	tts = IE_NUM_TT;
	tts ++;

	/* module private data length (fixed struct + variable strcuts) */
	len = (uint16)sizeof(wlc_ier_reg_t);	/* fixed */
	len = (uint16)ALIGN_SIZE(len, sizeof(uint8));	/* build tag */
	len += (uint16)(sizeof(uint8) * build_cnt);
	len = (uint16)ALIGN_SIZE(len, sizeof(void *));	/* build cb */
	len += (uint16)(sizeof(wlc_iem_cbe_t) * build_cnt);
	len = (uint16)ALIGN_SIZE(len, sizeof(uint16));	/* build tt */
	len += (uint16)(sizeof(uint16) * tts);
	len = (uint16)ALIGN_SIZE(len, sizeof(uint8));	/* parse tag */
	len += (uint16)(sizeof(uint8) * parse_cnt);
	len = (uint16)ALIGN_SIZE(len, sizeof(void *));	/* parse cb */
	len += (uint16)(sizeof(wlc_iem_pe_t) * parse_cnt);
	len = (uint16)ALIGN_SIZE(len, sizeof(uint16));	/* parse tt */
	len += (uint16)(sizeof(uint16) * tts);

	/* allocate module private data */
	if ((reg = MALLOC(wlc->osh, len)) == NULL) {
		WL_ERROR(("wl%d: %s: out of mem, malloced %d bytes\n",
		          wlc->pub->unit, __FUNCTION__, MALLOCED(wlc->osh)));
		goto fail;
	}
	bzero(reg, len);
	reg->wlc = wlc;
	reg->iem = iem;
	reg->af_reg_len = len;

	/* suballocate variable size arrays */
	len = (uint16)sizeof(wlc_ier_reg_t);	/* fixed */
	len = (uint16)ALIGN_SIZE(len, sizeof(uint8));	/* build tag */
	reg->build_tag_offset = len;
	len += (uint16)(sizeof(uint8) * build_cnt);
	len = (uint16)ALIGN_SIZE(len, sizeof(void *));	/* build cb */
	reg->build_cb_offset = len;
	len += (uint16)(sizeof(wlc_iem_cbe_t) * build_cnt);
	len = (uint16)ALIGN_SIZE(len, sizeof(uint16));	/* build tt */
	reg->build_tt_offset = len;
	len += (uint16)(sizeof(uint16) * tts);
	len = (uint16)ALIGN_SIZE(len, sizeof(uint8));	/* parse tag */
	reg->parse_tag_offset = len;
	len += (uint16)(sizeof(uint8) * parse_cnt);
	len = (uint16)ALIGN_SIZE(len, sizeof(void *));	/* parse cb */
	reg->parse_cb_offset = len;
	len += (uint16)(sizeof(wlc_iem_pe_t) * parse_cnt);
	len = (uint16)ALIGN_SIZE(len, sizeof(uint16));	/* parse tt */
	reg->parse_tt_offset = len;
	len += (uint16)(sizeof(uint16) * tts);

	reg->build_cnt = build_cnt;
	reg->parse_cnt = parse_cnt;

#if defined(BCMDBG) || defined(BCMDBG_DUMP)
	/* for debugging */
	{
	int idx;
	for (idx = 0; idx < wlc->pub->tunables->max_ie_regs; idx ++) {
		if (IE_REG(iem, idx) == NULL) {
			wlc_ier_reg_t **regp = &IE_REG(iem, idx);
			*regp = reg;
			break;
		}
	}
	ASSERT(idx < wlc->pub->tunables->max_ie_regs);
	}
#endif /* BCMDBG || BCMDBG_DUMP */

	iem->regs_cnt ++;

	/* register Vendor Specific IE calc_len/build proxy */
	if (build_cnt > 0 &&
	    wlc_ier_add_build_fn(reg, DOT11_MNG_VS_ID,
	            wlc_ier_vs_calc_len_cb, wlc_ier_vs_build_cb, reg) < 0) {
		WL_ERROR(("wl%d: %s: wlc_iem_add_build_fn failed\n",
		          wlc->pub->unit, __FUNCTION__));
		goto fail;
	}
	/* register Vendor Specific IE parse proxy */
	if (parse_cnt > 0 &&
	    wlc_ier_add_parse_fn(reg, DOT11_MNG_VS_ID, wlc_ier_vs_parse_cb, reg) < 0) {
		WL_ERROR(("wl%d: %s: wlc_iem_add_parse_fn failed\n",
		          wlc->pub->unit, __FUNCTION__));
		goto fail;
	}

	return reg;

fail:	/* error handling */
	wlc_ier_destroy_registry(reg);
	return NULL;
}

/*
 * Free the registry.
 */
void
BCMATTACHFN(wlc_ier_destroy_registry)(wlc_ier_reg_t *reg)
{
	wlc_info_t *wlc;
	wlc_ier_info_t *iem;
	uint16 len;

	if (reg == NULL)
		return;

	wlc = reg->wlc;
	iem = reg->iem;
	len = reg->af_reg_len;

	ASSERT(iem->regs_cnt > 0);

#if defined(BCMDBG) || defined(BCMDBG_DUMP)
	/* for debugging */
	{
	int idx;
	for (idx = 0; idx < wlc->pub->tunables->max_ie_regs; idx ++) {
		if (IE_REG(iem, idx) == reg) {
			wlc_ier_reg_t **regp = &IE_REG(iem, idx);
			*regp = NULL;
			break;
		}
	}
	ASSERT(idx < wlc->pub->tunables->max_ie_regs);
	}
#endif /* BCMDBG || BCMDBG_DUMP */

	iem->regs_cnt --;

	MFREE(wlc->osh, reg, len);
}

/*
 * Sort the 'calc_len/build' part of the registry with the provided sorting order
 * table 'ies_tag'.
 */
int
BCMINITFN(wlc_ier_sort_cbtbl)(wlc_ier_reg_t *reg, const uint8 *ie_tags, uint16 ie_cnt)
{
	uint8 *build_tag = BUILD_TAG_TBL(reg);
	wlc_iem_cbe_t *build_cb = BUILD_CB_TBL(reg);
	uint16 *build_tt = BUILD_TT_TBL(reg);
	uint16 cur, next;

	if (reg->init)
		return BCME_UNSUPPORTED;

	reg->init = TRUE;

	TBL_POS_GET(build_tt, IE_TT_IE, cur, next);

	wlc_ieml_sort_cbtbl(build_tag, build_cb, next - cur, ie_tags, ie_cnt);

	return BCME_OK;
}

#if defined(BCMDBG) || defined(BCMDBG_DUMP)
static int
wlc_ier_reg_dump(wlc_ier_reg_t *reg, struct bcmstrbuf *b)
{
	uint8 *build_tag = BUILD_TAG_TBL(reg);
	wlc_iem_cbe_t *build_cb = BUILD_CB_TBL(reg);
	uint16 *build_tt = BUILD_TT_TBL(reg);
	uint8 *parse_tag = PARSE_TAG_TBL(reg);
	wlc_iem_pe_t *parse_cb = PARSE_CB_TBL(reg);
	uint16 *parse_tt = PARSE_TT_TBL(reg);
	uint16 cur, next;
	uint i;

	bcm_bprintf(b, "  build %u parse %u\n", reg->build_cnt, reg->parse_cnt);
	/* calc/build */
	TBL_POS_GET(build_tt, IE_TT_IE, cur, next);
	bcm_bprintf(b, "    calc/build %u\n", next - cur);
	for (i = cur; i < next; i ++) {
		bcm_bprintf(b, "      idx %u(%u): tag %u, calc %p, build %p, ctx %p\n",
		            i - cur, i, build_tag[i],
		            build_cb[i].calc, build_cb[i].build, build_cb[i].ctx);
	}
	TBL_POS_GET(build_tt, IE_TT_PRIO, cur, next);
	bcm_bprintf(b, "    vs calc/build %u\n", next - cur);
	for (i = cur; i < next; i ++) {
		bcm_bprintf(b, "      idx %u(%u): tag %u, calc %p, build %p, ctx %p\n",
		            i - cur, i, build_tag[i],
		            build_cb[i].calc, build_cb[i].build, build_cb[i].ctx);
	}
	/* parse */
	TBL_POS_GET(parse_tt, IE_TT_IE, cur, next);
	bcm_bprintf(b, "    parse %u\n", next - cur);
	for (i = cur; i < next; i ++) {
		bcm_bprintf(b, "      idx %u(%u): tag %u, parse %p, ctx %p\n",
		            i - cur, i, parse_tag[i],
		            parse_cb[i].parse, parse_cb[i].ctx);
	}
	TBL_POS_GET(parse_tt, IE_TT_PRIO, cur, next);
	bcm_bprintf(b, "    vs parse %u\n", next - cur);
	for (i = cur; i < next; i ++) {
		bcm_bprintf(b, "      idx %u(%u): tag %u, parse %p, ctx %p\n",
		            i - cur, i, parse_tag[i],
		            parse_cb[i].parse, parse_cb[i].ctx);
	}

	return BCME_OK;
}
#endif /* BCMDBG || BCMDBG_DUMP */

/*
 * Register a pair of 'calc_len'/'build' callbacks for an non Vendor Specific IE.
 *
 * Inputs:
 * - tag: IE tag as defined by DOT11_MNG_XXXX in 802.11.h
 *
 * A negative return value indicates an error.
 */
int
BCMATTACHFN(wlc_ier_add_build_fn)(wlc_ier_reg_t *reg, uint8 tag,
	wlc_iem_calc_fn_t calc_fn, wlc_iem_build_fn_t build_fn, void *ctx)
{
	uint8 *build_tag = BUILD_TAG_TBL(reg);
	wlc_iem_cbe_t *build_cb = BUILD_CB_TBL(reg);
	uint16 *build_tt = BUILD_TT_TBL(reg);
	uint16 cur, next;

	if (TBL_TTL_GET(build_tt) >= reg->build_cnt) {
		WL_ERROR(("wl%d: %s: too many entries\n", reg->wlc->pub->unit, __FUNCTION__));
		return BCME_NORESOURCE;
	}

	TBL_POS_GET(build_tt, IE_TT_IE, cur, next);

	IEM_TRACE(("wl%d: %s: cur %u, next %u\n", WLCUNIT(reg), __FUNCTION__, cur, next));

	/* move the entries of other frame types down by 1 */
	TBL_POS_SHIFT(build_tag, build_cb, TBL_TTL_GET(build_tt), next, build_tt, IE_TT_IE);

	/* point to the beginning of the frame type specific table */
	build_tag = &build_tag[cur];
	build_cb = &build_cb[cur];

	wlc_ieml_add_build_fn(build_tag, build_cb, next - cur, calc_fn, build_fn, ctx, tag);

	return BCME_OK;
}

/*
 * Register a pair of 'calc_len'/build_frame' callbacks for an Vendor Specific IE.
 *
 * Inputs:
 * - prio: Relative position of Vendor Specific IE in a frame
 *
 * Some known priorities are defined by WLC_IEM_VS_IE_PRIO_XXXX (0 at the
 * first in the frame while 249 at the last).
 *
 * Position of Vendor Specific IEs are undefined with the same prio value.
 *
 * A negative return value indicates an error.
 */
int BCMATTACHFN(wlc_ier_vs_add_build_fn)(wlc_ier_reg_t *reg, uint8 prio,
	wlc_iem_calc_fn_t calc_fn, wlc_iem_build_fn_t build_fn, void *ctx)
{
	uint8 *build_tag = BUILD_TAG_TBL(reg);
	wlc_iem_cbe_t *build_cb = BUILD_CB_TBL(reg);
	uint16 *build_tt = BUILD_TT_TBL(reg);
	uint16 cur, next;

	if (TBL_TTL_GET(build_tt) >= reg->build_cnt) {
		WL_ERROR(("wl%d: %s: too many entries\n", reg->wlc->pub->unit, __FUNCTION__));
		return BCME_NORESOURCE;
	}

	TBL_POS_GET(build_tt, IE_TT_PRIO, cur, next);

	IEM_TRACE(("wl%d: %s: cur %u, next %u\n", WLCUNIT(reg), __FUNCTION__, cur, next));

	/* move the entries of other frame types down by 1 */
	TBL_POS_SHIFT(build_tag, build_cb, TBL_TTL_GET(build_tt), next, build_tt, IE_TT_PRIO);

	/* point to the beginning of the frame type specific table */
	build_tag = &build_tag[cur];
	build_cb = &build_cb[cur];

	wlc_ieml_add_build_fn(build_tag, build_cb, next - cur, calc_fn, build_fn, ctx, prio);

	return BCME_OK;
}

/*
 * Calculate expected IEs' length in a frame.
 *
 * Inputs:
 * - uiel: User supplied IEs list
 * - cbparm: Callback parameters
 *
 * Outputs:
 * - len: IEs' total length in bytes
 *
 * A negative return value indicates an error (BCME_XXXX).
 */
uint
wlc_ier_calc_len(wlc_ier_reg_t *reg, wlc_bsscfg_t *cfg, uint16 ft,
	wlc_iem_cbparm_t *cbparm)
{
	uint8 *build_tag = BUILD_TAG_TBL(reg);
	wlc_iem_cbe_t *build_cb = BUILD_CB_TBL(reg);
	uint16 *build_tt = BUILD_TT_TBL(reg);
	uint16 cur, next;

	TBL_POS_GET(build_tt, IE_TT_IE, cur, next);

	IEM_TRACE(("wl%d: %s: cur %u, next %u\n", WLCUNIT(reg), __FUNCTION__, cur, next));

	/* point to the beginning of the frame type specific table */
	build_tag = &build_tag[cur];
	build_cb = &build_cb[cur];

	return wlc_ieml_calc_len(cfg, ft, build_tag, TRUE, build_cb, next - cur,
	                         NULL, cbparm);
}

/* Calculate Vendor Specific IEs length in a frame */
static uint
wlc_ier_vs_calc_len_cb(void *ctx, wlc_iem_calc_data_t *data)
{
	wlc_ier_reg_t *reg = (wlc_ier_reg_t *)ctx;
	uint8 *build_prio = BUILD_TAG_TBL(reg);
	wlc_iem_cbe_t *build_cb = BUILD_CB_TBL(reg);
	uint16 *build_tt = BUILD_TT_TBL(reg);
	uint16 cur, next;

	TBL_POS_GET(build_tt, IE_TT_PRIO, cur, next);

	IEM_TRACE(("wl%d: %s: cur %u, next %u\n", WLCUNIT(reg), __FUNCTION__, cur, next));

	build_prio = &build_prio[cur];
	build_cb = &build_cb[cur];

	return wlc_ieml_calc_len(data->cfg, data->ft, build_prio, FALSE, build_cb, next - cur,
	                         NULL, data->cbparm);
}

/*
 * Write IEs in a frame.
 *
 * Inputs:
 * - uiel: User supplied IEs list
 * - cbparm: Callback parameters
 * - buf: IEs' buffer pointer
 * - buf_len: IEs' buffer length
 *
 * Outputs:
 * - buf: IEs written in the buffer
 *
 * A negative return value indicates an error (BCME_XXXX).
 */
int wlc_ier_build_frame(wlc_ier_reg_t *reg, wlc_bsscfg_t *cfg, uint16 ft,
	wlc_iem_cbparm_t *cbparm, uint8 *buf, uint buf_len)
{
	uint8 *build_tag = BUILD_TAG_TBL(reg);
	wlc_iem_cbe_t *build_cb = BUILD_CB_TBL(reg);
	uint16 *build_tt = BUILD_TT_TBL(reg);
	uint16 cur, next;

	TBL_POS_GET(build_tt, IE_TT_IE, cur, next);

	IEM_TRACE(("wl%d: %s: cur %u, next %u\n", WLCUNIT(reg), __FUNCTION__, cur, next));

	build_tag = &build_tag[cur];
	build_cb = &build_cb[cur];

	return wlc_ieml_build_frame(cfg, ft, build_tag, TRUE, build_cb, next - cur,
	                           NULL, cbparm, buf, buf_len);
}

/* Build Vendor Specific IEs in a frame */
static int
wlc_ier_vs_build_cb(void *ctx, wlc_iem_build_data_t *data)
{
	wlc_ier_reg_t *reg = (wlc_ier_reg_t *)ctx;
	uint8 *build_prio = BUILD_TAG_TBL(reg);
	wlc_iem_cbe_t *build_cb = BUILD_CB_TBL(reg);
	uint16 *build_tt = BUILD_TT_TBL(reg);
	uint16 cur, next;

	TBL_POS_GET(build_tt, IE_TT_PRIO, cur, next);

	IEM_TRACE(("wl%d: %s: cur %u, next %u\n", WLCUNIT(reg), __FUNCTION__, cur, next));

	build_prio = &build_prio[cur];
	build_cb = &build_cb[cur];

	return wlc_ieml_build_frame(data->cfg, data->ft, build_prio, FALSE, build_cb, next - cur,
	                            NULL, data->cbparm, data->buf, data->buf_len);
}

/*
 * Register 'parse' callback for a non Vendor Specific IE 'tag'.
 *
 * The callback is invoked when parsing a frame with the expected IE.
 *
 * Inputs:
 * - tag: IE tag as defined by DOT11_MNG_XXXX in 802.11.h
 *
 * A negative return value indicates an error.
 */
int
BCMATTACHFN(wlc_ier_add_parse_fn)(wlc_ier_reg_t *reg, uint8 tag,
	wlc_iem_parse_fn_t parse_fn, void *ctx)
{
	uint8 *parse_tag = PARSE_TAG_TBL(reg);
	wlc_iem_pe_t *parse_cb = PARSE_CB_TBL(reg);
	uint16 *parse_tt = PARSE_TT_TBL(reg);
	uint16 cur, next;

	if (TBL_TTL_GET(parse_tt) >= reg->parse_cnt) {
		WL_ERROR(("wl%d: %s: too many entries\n", reg->wlc->pub->unit, __FUNCTION__));
		return BCME_NORESOURCE;
	}

	TBL_POS_GET(parse_tt, IE_TT_IE, cur, next);

	IEM_TRACE(("wl%d: %s: cur %u, next %u\n", WLCUNIT(reg), __FUNCTION__, cur, next));

	/* move the entries of other frame types down by 1 */
	TBL_POS_SHIFT(parse_tag, parse_cb, TBL_TTL_GET(parse_tt), next, parse_tt, IE_TT_IE);

	/* point to the beginning of the frame type specific table */
	parse_tag = &parse_tag[cur];
	parse_cb = &parse_cb[cur];

	wlc_ieml_add_parse_fn(parse_tag, parse_cb, next - cur, parse_fn, ctx, tag);

	return BCME_OK;
}

/*
 * Register 'parse' callback for a Vendor Specific IE 'id'.
 *
 * The callback is invoked when parsing a frame with the expected IE.
 * A classifier callback must be provided through wlc_iem_parse_frame() API
 * to map the variable length OUI (and TYPE in most cases) to the 'id'.
 *
 * Inputs:
 * - id: Unique identify within the frame type
 *
 * A negative return value indicates an error.
 */
int BCMATTACHFN(wlc_ier_vs_add_parse_fn)(wlc_ier_reg_t *reg, uint8 id,
	wlc_iem_parse_fn_t parse_fn, void *ctx)
{
	uint8 *parse_tag = PARSE_TAG_TBL(reg);
	wlc_iem_pe_t *parse_cb = PARSE_CB_TBL(reg);
	uint16 *parse_tt = PARSE_TT_TBL(reg);
	uint16 cur, next;

	if (TBL_TTL_GET(parse_tt) >= reg->parse_cnt) {
		WL_ERROR(("wl%d: %s: too many entries\n", reg->wlc->pub->unit, __FUNCTION__));
		return BCME_NORESOURCE;
	}

	TBL_POS_GET(parse_tt, IE_TT_PRIO, cur, next);

	IEM_TRACE(("wl%d: %s: cur %u, next %u\n", WLCUNIT(reg), __FUNCTION__, cur, next));

	/* move the entries of other frame types down by 1 */
	TBL_POS_SHIFT(parse_tag, parse_cb, TBL_TTL_GET(parse_tt), next, parse_tt, IE_TT_PRIO);

	/* point to the beginning of the frame type specific table */
	parse_tag = &parse_tag[cur];
	parse_cb = &parse_cb[cur];

	wlc_ieml_add_parse_fn(parse_tag, parse_cb, next - cur, parse_fn, ctx, id);

	return BCME_OK;
}

/*
 * Parse IEs in a frame.
 *
 * Inputs:
 * - upp: User parse parameters
 * - pparm: Callback parameters
 * - buf: IEs' buffer pointer
 * - buf_len: IEs' buffer length
 *
 * A negative return value indicates an error (BCME_XXXX).
 */
int
wlc_ier_parse_frame(wlc_ier_reg_t *reg, wlc_bsscfg_t *cfg, uint16 ft,
	wlc_iem_upp_t *upp, wlc_iem_pparm_t *pparm, uint8 *buf, uint buf_len)
{
	uint8 *parse_tag = PARSE_TAG_TBL(reg);
	wlc_iem_pe_t *parse_cb = PARSE_CB_TBL(reg);
	uint16 *parse_tt = PARSE_TT_TBL(reg);
	uint16 cur, next;
	int err;

	TBL_POS_GET(parse_tt, IE_TT_IE, cur, next);

	IEM_TRACE(("wl%d: %s: cur %u, next %u\n", WLCUNIT(reg), __FUNCTION__, cur, next));

	parse_tag = &parse_tag[cur];
	parse_cb = &parse_cb[cur];

	/* Pass it to Vendor Specific IE proxy */
	ASSERT(reg->upp == NULL);
	reg->upp = upp;

	err = wlc_ieml_parse_frame(cfg, ft, parse_tag, TRUE, parse_cb, next - cur,
	                           upp, pparm, buf, buf_len);

	reg->upp = NULL;
	return err;
}

/* Parse Vendor Specific IE pointed by data->ie pointer */
static int
wlc_ier_vs_parse_cb(void *ctx, wlc_iem_parse_data_t *data)
{
	wlc_ier_reg_t *reg = (wlc_ier_reg_t *)ctx;
	uint8 *parse_id = PARSE_TAG_TBL(reg);
	wlc_iem_pe_t *parse_cb = PARSE_CB_TBL(reg);
	uint16 *parse_tt = PARSE_TT_TBL(reg);
	uint16 cur, next;
	uint8 *buf;
	uint buf_len;

	TBL_POS_GET(parse_tt, IE_TT_PRIO, cur, next);

	IEM_TRACE(("wl%d: %s: cur %u, next %u\n", WLCUNIT(reg), __FUNCTION__, cur, next));

	parse_id = &parse_id[cur];
	parse_cb = &parse_cb[cur];

	/* let's parse all IEs of the same tag at once starting from this one */
	if (data->ie != NULL) {
		buf = data->ie;
		buf_len = data->buf_len - (uint)(buf - data->buf);
	}
	else {
		buf = NULL;
		buf_len = 0;
	}

	return wlc_ieml_parse_frame(data->cfg, data->ft, parse_id, FALSE, parse_cb, next - cur,
	                            reg->upp, data->pparm, buf, buf_len);
}
