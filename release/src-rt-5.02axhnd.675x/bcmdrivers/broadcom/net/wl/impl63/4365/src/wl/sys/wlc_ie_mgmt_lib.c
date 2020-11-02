/**
 * @file
 * This file contains a collection of low level IE management functions to:
 *
 * 1. manipulate IE management related callback tables
 * 2. register callbacks
 * 3. invoke callbacks
 *
 * The user provides callbacks table's storage.
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
 * $Id: wlc_ie_mgmt_lib.c 708017 2017-06-29 14:11:45Z $
 */

#include <wlc_cfg.h>
#include <typedefs.h>
#include <bcmdefs.h>
#include <bcmutils.h>
#include <wl_dbg.h>
#if defined(BCMDBG) || defined(BCMDBG_ERR)
#include <osl.h>
#include <siutils.h>
#include <d11.h>
#include <wlioctl.h>
#include <wlc_pub.h>
#include <wlc_bsscfg.h>
#include <wlc_rate.h>
#include <wlc.h>
#endif /* BCMDBG || BCMDBG_ERR */
#include <wlc_ie_mgmt_dbg.h>
#include <wlc_ie_mgmt_lib.h>

#ifdef BCMDBG
/* default message level */
uint iem_msg_level = 0;
#endif /* BCMDBG */

/* Sort entries in a callback table 'cbe_tag' + 'cbe_tbl' based on the order
 * of the tags in the tag table 'tag'. 'cbe_cnt' tells the size of the callback
 * table. 'cnt' indicates the size of the tag table. 'tmp_cbe' is a storage
 * provided by the caller to temporarily save an callback entry. 'cbe_sz' is
 * the callback entry's size in bytes.
 */
static void
BCMINITFN(wlc_iem_sort_tbl)(uint8 *cbe_tag, void *cbe_tbl, uint16 cbe_cnt,
	void *tmp_cbe, uint cbe_sz, const uint8 *tag, uint16 cnt)
{
	uint16 ti;
	uint16 first;	/* the first entry after the sorted ones */
	uint8 *ep;

	IEM_TRACE(("%s: tag %p cb %p sz %u tplt %p cnt %u\n",
	           __FUNCTION__, cbe_tag, cbe_tbl, cbe_cnt, tag, cnt));

	ASSERT(cbe_tag != NULL);
	ASSERT(cbe_tbl != NULL);
	ASSERT(tmp_cbe != NULL);
	ASSERT(tag != NULL);

	/* use the tags order in the tags table as a guide to "sort" callbacks table */
	for (ti = 0, first = 0; ti < cnt && first < cbe_cnt; ti ++) {
		uint8 t = tag[ti];
		int l, m, r;

		IEM_TRACE(("%s: tag %u\n", __FUNCTION__, t));

		/* binary search for 'tag' in the callbacks table */
		l = first;
		r = cbe_cnt - 1;
		while (l <= r) {
			m = (l + r) / 2;

			IEM_TRACE(("%s: left %u, mid %u, right %u\n", __FUNCTION__, l, m, r));

			if (t < cbe_tag[m]) {
				r = m - 1;
				continue;
			}
			else if (t > cbe_tag[m]) {
				l = m + 1;
				continue;
			}

			/* found the callback */
			IEM_TRACE(("%s: found %u\n", __FUNCTION__, m));

			/* move only when the found entry is
			 * after the first entry past the sorted ones
			 */
			if (m > first) {
				/* move it to the front */
				IEM_TRACE(("%s: insert %u\n", __FUNCTION__, first));
				ep = (uint8 *)cbe_tbl + cbe_sz * m;
				bcopy(ep, tmp_cbe, cbe_sz);
				ep = (uint8 *)cbe_tbl + cbe_sz * first;
				memmove(ep + cbe_sz, ep, cbe_sz * (m - first));
				bcopy(tmp_cbe, ep, cbe_sz);
				memmove(&cbe_tag[first + 1], &cbe_tag[first],
				        sizeof(uint8) * (m - first));
				cbe_tag[first] = t;
			}

			/* move to next entry */
			first ++;

			/* done with this callback */
			break;
		}

		/* nothing is found */
		if (l > r) {
			IEM_TRACE(("%s: not found\n", __FUNCTION__));
		}
	}
}

/* Sort calc_len/build callback table (build_tag + build_cb) entries based on
 * the tags order in the tag table 'tag'.
 */
void
BCMINITFN(wlc_ieml_sort_cbtbl)(uint8 *build_tag, wlc_iem_cbe_t *build_cb, uint16 build_cnt,
	const uint8 *tag, uint16 cnt)
{
	wlc_iem_cbe_t tmp;

	IEM_TRACE(("%s: tag %p cb %p sz %u tplt %p cnt %u\n",
	           __FUNCTION__, build_tag, build_cb, build_cnt, tag, cnt));

	wlc_iem_sort_tbl(build_tag, build_cb, build_cnt, &tmp, sizeof(tmp), tag, cnt);
}

/* Add a callback entry 'cbe' in the callback table 'cbe_tag' + 'cbe_tbl'.
 * 'cbe_cnt' is the current callback table size before adding the entry.
 * 'cbe_sz' specifies the callback entry size.
 */
static void
BCMATTACHFN(wlc_iem_add_cb)(uint8 *cbe_tag, void *cbe_tbl, uint16 cbe_cnt,
	void *cbe, uint cbe_sz, uint8 tag)
{
	uint16 i;
	uint8 *ep;

	ASSERT(cbe_tag != NULL);
	ASSERT(cbe_tbl != NULL);
	ASSERT(cbe != NULL);

	IEM_TRACE(("%s: tag %p cb %p sz %u ent %u\n",
	           __FUNCTION__, cbe_tag, cbe_tbl, cbe_cnt, tag));

	/* linear search the registered callbacks table */
	for (i = 0; i < cbe_cnt; i ++) {
		/* insert it right at here */
		if (tag == cbe_tag[i] ||
		    ((i == 0 || tag > cbe_tag[i - 1]) && tag <= cbe_tag[i])) {
			IEM_TRACE(("%s: insert %u\n", __FUNCTION__, i));
			break;
		}
	}
	/* insert at after all existing entries when i == cbe_cnt */
	if (i == cbe_cnt) {
		IEM_TRACE(("%s: append %u\n", __FUNCTION__, i));
	}

	/* move entries from index 'i' down by 1 */
	ep = (uint8 *)cbe_tbl + cbe_sz * i;
	if (cbe_cnt > i) {
		IEM_TRACE(("%s: move %u\n", __FUNCTION__, i));
		memmove(ep + cbe_sz, ep, cbe_sz * (cbe_cnt - i));
		memmove(&cbe_tag[i + 1], &cbe_tag[i], sizeof(uint8) * (cbe_cnt - i));
	}
	/* insert it at index 'i' */
	bcopy(cbe, ep, cbe_sz);
	cbe_tag[i] = tag;
}

/* Register 'calc_len'/'build' callback pair for tag 'tag' */
void
BCMATTACHFN(wlc_ieml_add_build_fn)(uint8 *build_tag, wlc_iem_cbe_t *build_cb, uint16 build_cnt,
	wlc_iem_calc_fn_t calc_fn, wlc_iem_build_fn_t build_fn, void *ctx,
	uint8 tag)
{
	wlc_iem_cbe_t build;

	build.calc = calc_fn;
	build.build = build_fn;
	build.ctx = ctx;

	wlc_iem_add_cb(build_tag, build_cb, build_cnt, &build, sizeof(build), tag);
}

/* User supplied IEs list processing */
typedef struct {
	uint l;
	wlc_iem_cbe_t *cbe;
	union {
		wlc_iem_calc_data_t *calc;
		wlc_iem_build_data_t *build;
	} cbdata;
} wlc_iem_uiel_cbparm_t;

/* Write all IEs with the tag value 'tag' */
static int
wlc_iem_ins_uiel(wlc_bsscfg_t *cfg, uint16 ft, uint8 tag, bool is_tag,
	wlc_iem_uiel_t *uiel, wlc_iem_cbparm_t *cbparm,
	int (*ins)(wlc_iem_uiel_cbparm_t *ucbp, uint8 *ie, uint ie_len),
	wlc_iem_uiel_cbparm_t *ucbp)
{
	uint8 *ies;
	uint ies_len;
	uint8 *ie;
	uint ie_len;
	wlc_iem_cbvsie_data_t vsie;
	int err;

	ASSERT(uiel != NULL);
	if (!is_tag)
		ASSERT(uiel->vsie_fn != NULL);
	ASSERT(ins != NULL);

	IEM_TRACE(("%s: ins %u is_tag %d\n", __FUNCTION__, tag, is_tag));

	ies = uiel->ies;
	ies_len = uiel->ies_len;

	vsie.cbparm = cbparm;
	vsie.cfg = cfg;
	vsie.ft = ft;

	while ((ie = ies) != NULL &&
	       (ie_len = TLV_HDR_LEN + ie[TLV_LEN_OFF]) <= ies_len) {
		uint8 ie_tag = ie[TLV_TAG_OFF];

		IEM_TRACE(("%s: tag %u\n", __FUNCTION__, ie_tag));

		/* handle Vendor Specific IE when !is_tag */
		if (!is_tag) {
			vsie.ie = ie;
			vsie.ie_len = ie_len;
			/* 'ie_tag' value is now the prio */
			ie_tag = (uiel->vsie_fn)(uiel->ctx, &vsie);
			IEM_TRACE(("%s: vsie %u\n", __FUNCTION__, ie_tag));
		}

		/* insert the IE with matching tag */
		if (ie_tag == tag) {
			IEM_TRACE(("%s: tag %u len %u\n", __FUNCTION__, ie_tag, ie_len));
			if ((err = (ins)(ucbp, ie, ie_len)) != BCME_OK) {
				IEM_TRACE(("%s: err %d\n", __FUNCTION__, err));
				return err;
			}
		}

		/* next IE */
		ies += ie_len;
		ies_len -= ie_len;
	}

	return BCME_OK;
}

/* Modify and write all IEs with the tag value 'tag' */
static int
wlc_iem_mod_uiel(wlc_bsscfg_t *cfg, uint16 ft, uint8 tag, bool is_tag,
	wlc_iem_uiel_t *uiel, wlc_iem_cbparm_t *cbparm,
	int (*mod)(wlc_iem_uiel_cbparm_t *ucbp, uint8 *ie, uint ie_len),
	wlc_iem_uiel_cbparm_t *ucbp)
{
	uint8 *ies;
	uint ies_len;
	uint8 *ie;
	uint ie_len;
	wlc_iem_cbvsie_data_t vsie;
	int err;

	ASSERT(uiel != NULL);
	if (!is_tag)
		ASSERT(uiel->vsie_fn != NULL);
	ASSERT(mod != NULL);

	IEM_TRACE(("%s: mod %u is_tag %d\n", __FUNCTION__, tag, is_tag));

	ies = uiel->ies;
	ies_len = uiel->ies_len;

	vsie.cbparm = cbparm;
	vsie.cfg = cfg;
	vsie.ft = ft;

	while ((ie = ies) != NULL &&
	       (ie_len = TLV_HDR_LEN + ie[TLV_LEN_OFF]) <= ies_len) {
		uint8 ie_tag = ie[TLV_TAG_OFF];

		IEM_TRACE(("%s: tag %u\n", __FUNCTION__, ie_tag));

		/* handle Vendor Specific IE when !is_tag */
		if (!is_tag) {
			vsie.ie = ie;
			vsie.ie_len = ie_len;
			/* 'ie_tag' value is now the prio */
			ie_tag = (uiel->vsie_fn)(uiel->ctx, &vsie);
			IEM_TRACE(("%s: vsie %u\n", __FUNCTION__, ie_tag));
		}

		/* insert the IE with matching tag */
		if (ie_tag == tag) {
			IEM_TRACE(("%s: tag %u len %u\n", __FUNCTION__, ie_tag, ie_len));
			if ((err = (mod)(ucbp, ie, ie_len)) != BCME_OK) {
				IEM_TRACE(("%s: err %d\n", __FUNCTION__, err));
				return err;
			}
		}

		/* next IE */
		ies += ie_len;
		ies_len -= ie_len;
	}

	return BCME_OK;
}

/* Insert IEs that don't have any registered callbacks with users approval, and
 * insert IEs that have registered callbacks but the user has decided to write
 * them out as is, or insert modified IEs that have registered callbacks and the
 * user has decided to modify them.
 */
static int
wlc_iem_proc_uiel(wlc_bsscfg_t *cfg, uint16 ft,
	uint8 *nocbvec, int16 prev, uint8 tag, bool is_tag,
	wlc_iem_uiel_t *uiel, wlc_iem_cbparm_t *cbparm,
	int (*ins_fn)(wlc_iem_uiel_cbparm_t *ucbp, uint8 *ie, uint ie_len),
	int (*mod_fn)(wlc_iem_uiel_cbparm_t *ucbp, uint8 *ie, uint ie_len),
	wlc_iem_uiel_cbparm_t *ucbp, bool *done)
{
	uint8 *ies;
	uint ies_len;
	uint8 *tie = NULL;
	uint tie_len = 0;
	uint8 *ie;
	uint ie_len;
	wlc_iem_ins_data_t ins;
	wlc_iem_mod_data_t mod;
	wlc_iem_cbvsie_data_t vsie;
	int err;

	ASSERT(done != NULL);
	*done = FALSE;

	/* Initialize variables for user supplied IEs list processing */
	if (uiel != NULL) {
		IEM_TRACE(("%s: prev %d tag %u is_tag %d\n", __FUNCTION__, prev, tag, is_tag));

		ASSERT(uiel->ies != NULL);
		ASSERT(uiel->ies_len > 0);
		ASSERT(uiel->ins_fn != NULL);
		ASSERT(uiel->mod_fn != NULL);
		ASSERT(uiel->vsie_fn != NULL);
		ies = uiel->ies;
		ies_len = uiel->ies_len;
		ins.cbparm = cbparm;
		ins.cfg = cfg;
		ins.ft = ft;
		ins.prev = prev;
		ins.tag = tag;
		ins.is_tag = is_tag;
		mod.cbparm = cbparm;
		mod.cfg = cfg;
		mod.ft = ft;
		vsie.cbparm = cbparm;
		vsie.cfg = cfg;
		vsie.ft = ft;
	}
	else {
		ies = NULL;
		ies_len = 0;
	}

	/* walk all IEs and insert them if user approves so */
	while ((ie = ies) != NULL &&
	       (ie_len = TLV_HDR_LEN + ie[TLV_LEN_OFF]) <= ies_len) {
		uint8 ie_tag = ie[TLV_TAG_OFF];

		/* skip non applicable IEs based on 'is_tag' value */
		if ((is_tag ? TRUE : FALSE) == (ie_tag == DOT11_MNG_VS_ID)) {
			IEM_TRACE(("%s: ie_tag %u\n", __FUNCTION__, ie_tag));
			goto n_ie;
		}

		IEM_TRACE(("%s: tag %u\n", __FUNCTION__, ie_tag));

		/* handle Vendor Specific IE when !is_tag */
		if (!is_tag && ie_tag == DOT11_MNG_VS_ID) {
			vsie.ie = ie;
			vsie.ie_len = ie_len;
			/* 'ie_tag' value is now the prio */
			ie_tag = (uiel->vsie_fn)(uiel->ctx, &vsie);
			IEM_TRACE(("%s: vsie %u\n", __FUNCTION__, ie_tag));
		}

		/* insert the IE if the 'insert_ie?' callback says Yes */
		if (isset(nocbvec, ie_tag) &&
		    (ins.ie = ie, ins.ie_len = ie_len,
		     (uiel->ins_fn)(uiel->ctx, &ins))) {
			IEM_TRACE(("%s: ins %u\n", __FUNCTION__, ie_tag));
			if ((err = wlc_iem_ins_uiel(cfg, ft, ie_tag, is_tag,
			                uiel, cbparm, ins_fn, ucbp)) != BCME_OK) {
				IEM_TRACE(("%s: err %d\n", __FUNCTION__, err));
				return err;
			}
			/* done for IEs with this tag */
			clrbit(nocbvec, ie_tag);
		}

		/* also find the IE that matches the registered tag. */
		/* !point to the first one! */
		if (tie == NULL &&
		    ie_tag == tag) {
			IEM_TRACE(("%s: found %u\n", __FUNCTION__, ie_tag));
			tie = ie;
			tie_len = ie_len;
		}

		/* next IE */
	n_ie:	ies += ie_len;
		ies_len -= ie_len;
	}

	/* insert the user supplied IE that matches the registered tag */
	if (tie != NULL) {
		/* insert IEs as is */
		if ((mod.ie = tie, mod.ie_len = tie_len,
		     !(uiel->mod_fn)(uiel->ctx, &mod))) {
			IEM_TRACE(("%s: ins %u\n", __FUNCTION__, tag));
			if ((err = wlc_iem_ins_uiel(cfg, ft, tag, is_tag,
			                uiel, cbparm, ins_fn, ucbp)) != BCME_OK) {
				IEM_TRACE(("%s: err %d\n", __FUNCTION__, err));
				return err;
			}
		}
		/* insert the modified IEs */
		else {
			IEM_TRACE(("%s: mod %u\n", __FUNCTION__, tag));
			if ((err = wlc_iem_mod_uiel(cfg, ft, tag, is_tag,
			                uiel, cbparm, mod_fn, ucbp)) != BCME_OK) {
				IEM_TRACE(("%s: err %d\n", __FUNCTION__, err));
				return err;
			}
		}
		*done = TRUE;
	}

	return BCME_OK;
}

/* Calculate user supplied IE's length - when write the IE as is */
static int
wlc_iem_calc_ins_uie_len(wlc_iem_uiel_cbparm_t *ucbp, uint8 *ie, uint ie_len)
{
	ASSERT(ucbp != NULL);

	IEM_TRACE(("%s: tag %u len %u\n", __FUNCTION__, ie[TLV_TAG_OFF], ie_len));

	ucbp->l += ie_len;

	return BCME_OK;
}

/* Calculate user supplied IE's length - when use the IE as a template and modify it */
static int
wlc_iem_calc_mod_uie_len(wlc_iem_uiel_cbparm_t *ucbp, uint8 *ie, uint ie_len)
{
	wlc_iem_calc_data_t *calc;
	wlc_iem_cbe_t *cbe;
	uint l;

	ASSERT(ucbp != NULL);
	ASSERT(ucbp->cbe != NULL);
	ASSERT(ucbp->cbdata.calc != NULL);
	ASSERT(ie != NULL);

	cbe = ucbp->cbe;
	calc = ucbp->cbdata.calc;

	calc->tag = ie[TLV_TAG_OFF];
	calc->ie = ie;

	ASSERT(cbe->calc != NULL);
	l = (cbe->calc)(cbe->ctx, calc);

	IEM_TRACE(("%s: tag %u len %u\n", __FUNCTION__, ie[TLV_TAG_OFF], l));

	ucbp->l += l;

	return BCME_OK;
}

static uint
_wlc_iem_calc_ie_len(wlc_bsscfg_t *cfg, uint16 ft,
	uint8 tag, bool is_tag, int16 prev, wlc_iem_cbe_t *cbe,
	wlc_iem_uiel_t *uiel, wlc_iem_cbparm_t *cbparm, uint8 *nocbvec)
{
	uint l;
	uint ll = 0;
	int err;
	wlc_iem_calc_data_t calc;

	calc.cbparm = cbparm;
	calc.cfg = cfg;
	calc.ft = ft;

	/* insert user supplied IEs with user's approval before the current tag */
	if (uiel != NULL) {
		wlc_iem_uiel_cbparm_t ucbp;
		bool done;

		IEM_TRACE(("%s: tag %u\n", __FUNCTION__, tag));

		bzero(&ucbp, sizeof(ucbp));
		ucbp.cbe = cbe;
		ucbp.cbdata.calc = &calc;

		err = wlc_iem_proc_uiel(cfg, ft, nocbvec,
		                        prev, tag, is_tag, uiel, cbparm,
		                        wlc_iem_calc_ins_uie_len, wlc_iem_calc_mod_uie_len,
		                        &ucbp, &done);
		if (err != BCME_OK) {
			WL_ERROR(("%s: err %d\n", __FUNCTION__, err));
			return 0;
		}

		l = ucbp.l;
		ll += l;
		if (l > 0) {
			IEM_TRACE(("%s: uiel %u\n", __FUNCTION__, l));
		}

		if (done)
			goto exit;
	}

	/* insert a new IE for the current tag */
	calc.tag = tag;
	calc.ie = NULL;
	calc.ie_len = 0;
	ASSERT(cbe->calc != NULL);
	l = (cbe->calc)(cbe->ctx, &calc);
	ll += l;
	if (l > 0) {
		IEM_TRACE(("%s: len %u\n", __FUNCTION__, l));
	}

exit:
	return ll;
}

/* Calculate IEs' length by calling each 'calc_len' callbacks in the callback table.
 * Parallel table 'build_tag' is the 'IE tag' column of the callback table when param
 * 'is_tag' value is TRUE; it is the 'Vendor Specific IE prio' column of the callback
 * table otherwise. 'build_cnt' param is the callback table's size.
 */
uint
wlc_ieml_calc_len(wlc_bsscfg_t *cfg, uint16 ft,
	uint8 *build_tag, bool is_tag, wlc_iem_cbe_t *build_cb, uint16 build_cnt,
	wlc_iem_uiel_t *uiel, wlc_iem_cbparm_t *cbparm)
{
	uint16 i;
	uint l = 0;
	uint ll = 0;
	int16 prev = -1;
	uint8 nocbvec[CEIL(NBITVAL(NBBY), NBBY)];

	ASSERT(build_tag != NULL);
	ASSERT(build_cb != NULL);

	IEM_TRACE(("%s: ft 0x%04x is_tag %d\n", __FUNCTION__, ft, is_tag));

	/* figure out IEs that don't have any registered callbacks - 1's represent
	 * those IEs' that don't have any registered callbacks and whose indices
	 * are the IEs' tags.
	 */
	if (uiel != NULL) {
		memset(nocbvec, 0xff, sizeof(nocbvec));
		for (i = 0; i < build_cnt; i ++)
			clrbit(nocbvec, build_tag[i]);
	}

	/* walk through callbacks list and invoke them and sum IEs' length. */
	for (i = 0; i < build_cnt; i ++) {
		/* targeted IE */
		uint8 tag = build_tag[i];	/* this is 'prio' when 'is_tag' is FALSE */

		l = _wlc_iem_calc_ie_len(cfg, ft, tag, is_tag, prev, &build_cb[i],
		                         uiel, cbparm, nocbvec);
		ll += l;

		prev = tag;
	}

	IEM_TRACE(("%s: len %u\n", __FUNCTION__, ll));

	return ll;
}

/*
 * Invoke to calculate a specific non Vendor Specific IE's length.
 *
 * 'build_tag', 'build_cb', and 'build_cnt' are the calc_len/build pair callback table
 * and size.
 * 'is_tag' indicates if the tag table 'build_tag' contains IE tags or VS IE PRIOs.
 * 'cfg' and 'ft' are the BSS and frame type that the function is called for, and are
 * passed to the 'calc_len' callbacks as is.
 * 'tag' is the specific IE's tag.
 *
 * Note - doesn't work for User Supplied IE that doesn't have a registered callback
 * for its tag!
 */
uint
wlc_ieml_calc_ie_len(wlc_bsscfg_t *cfg, uint16 ft,
	uint8 *build_tag, bool is_tag, wlc_iem_cbe_t *build_cb, uint16 build_cnt,
	uint8 tag, wlc_iem_uiel_t *uiel, wlc_iem_cbparm_t *cbparm)
{
	uint16 i;
	uint l = 0;
	uint ll = 0;
	int16 prev = -1;
	uint8 nocbvec[CEIL(NBITVAL(NBBY), NBBY)];

	ASSERT(build_tag != NULL);
	ASSERT(build_cb != NULL);

	IEM_TRACE(("%s: ft 0x%04x is_tag %d\n", __FUNCTION__, ft, is_tag));

	/* figure out IEs that don't have any registered callbacks - 1's represent
	 * those IEs' that don't have any registered callbacks and whose indices
	 * are the IEs' tags.
	 */
	if (uiel != NULL) {
		memset(nocbvec, 0xff, sizeof(nocbvec));
		for (i = 0; i < build_cnt; i ++)
			clrbit(nocbvec, build_tag[i]);
	}

	/* walk through callbacks list and invoke the one expected */
	for (i = 0; i < build_cnt; i ++) {
		if (build_tag[i] == tag) {
			l = _wlc_iem_calc_ie_len(cfg, ft, tag, is_tag, prev, &build_cb[i],
			                         uiel, cbparm, nocbvec);
			ll += l;
		}
		prev = tag;
	}

	IEM_TRACE(("%s: len %u\n", __FUNCTION__, ll));

	return ll;
}

/* Write user supplied IE */
static int
wlc_iem_build_ins_uie(wlc_iem_uiel_cbparm_t *ucbp, uint8 *ie, uint ie_len)
{
	wlc_iem_build_data_t *build;

	ASSERT(ucbp != NULL);
	ASSERT(ucbp->cbdata.build != NULL);
	ASSERT(ie != NULL);

	build = ucbp->cbdata.build;

	IEM_TRACE(("%s: tag %u len %u\n", __FUNCTION__, ie[TLV_TAG_OFF], ie_len));

	if (build->buf_len < ie_len) {
		WL_ERROR(("%s: buf %u ie %u\n", __FUNCTION__, build->buf_len, ie_len));
		return BCME_BUFTOOSHORT;
	}

	bcopy(ie, build->buf, ie_len);
	build->buf += ie_len;
	build->buf_len -= ie_len;

	return BCME_OK;
}

static int
wlc_iem_build_mod_uie(wlc_iem_uiel_cbparm_t *ucbp, uint8 *ie, uint ie_len)
{
	wlc_iem_cbe_t *cbe;
	wlc_iem_build_data_t *build;
	wlc_iem_calc_data_t calc;
	int err;
	uint l;
	uint8 ie_tag;

	ASSERT(ucbp != NULL);
	ASSERT(ucbp->cbe != NULL);
	ASSERT(ucbp->cbdata.build != NULL);
	ASSERT(ie != NULL);

	cbe = ucbp->cbe;
	build = ucbp->cbdata.build;

	ie_tag = ie[TLV_TAG_OFF];

	IEM_TRACE(("%s: tag %u len %u\n", __FUNCTION__, ie_tag, ie_len));

	calc.cbparm = build->cbparm;
	calc.cfg = build->cfg;
	calc.ft = build->ft;
	calc.tag = ie_tag;
	calc.ie = ie;
	calc.ie_len = ie_len;
	ASSERT(cbe->calc != NULL);
	l = (cbe->calc)(cbe->ctx, &calc);
	if (build->buf_len < l) {
		WL_ERROR(("%s: buf %u ie %u\n", __FUNCTION__, build->buf_len, l));
		return BCME_BUFTOOSHORT;
	}

	build->tag = ie_tag;
	build->ie = ie;
	ASSERT(cbe->build != NULL);
	err = (cbe->build)(cbe->ctx, build);
	if (err != BCME_OK) {
		WL_ERROR(("%s: err %d\n", __FUNCTION__, err));
		return err;
	}
	build->buf += l;
	build->buf_len -= l;

	IEM_TRACE(("%s: len %u\n", __FUNCTION__, ie_len));

	return err;
}

/* Write a single IE in a buffer */
static int
_wlc_iem_build_ie(wlc_bsscfg_t *cfg, uint16 ft,
	uint8 tag, bool is_tag, int16 prev, wlc_iem_cbe_t *cbe,
	wlc_iem_uiel_t *uiel, wlc_iem_cbparm_t *cbparm, uint8 *nocbvec,
	uint8 *buf, uint *buf_len)
{
	uint l;
	uint ll = 0;
	int err;
	wlc_iem_calc_data_t calc;
	wlc_iem_build_data_t build;
	uint8 *buf1;
	uint buf_len1;
	uint32 tsf_l = IEM_T32D(cfg->wlc, 0);

	IEM_TRACE(("%s: cbe tag %u\n", __FUNCTION__, tag));

	build.cbparm = cbparm;
	build.cfg = cfg;
	build.ft = ft;
	build.buf = buf;
	build.buf_len = *buf_len;

	/* insert user supplied IEs with user's approval before the current tag */
	if (uiel != NULL) {
		wlc_iem_uiel_cbparm_t ucbp;
		bool done;

		IEM_TRACE(("%s: tag %u\n", __FUNCTION__, tag));

		bzero(&ucbp, sizeof(ucbp));
		ucbp.cbe = cbe;
		ucbp.cbdata.build = &build;

		/* save/advance the buffer pointer explicitly
		 * in case the callback also modifies it
		 */
		buf1 = build.buf;
		buf_len1 = build.buf_len;
		if ((err = wlc_iem_proc_uiel(cfg, ft, nocbvec,
		                             prev, tag, is_tag, uiel, cbparm,
		                             wlc_iem_build_ins_uie, wlc_iem_build_mod_uie,
		                             &ucbp, &done)) != BCME_OK) {
			WL_ERROR(("%s: err %d\n", __FUNCTION__, err));
			return err;
		}
		/* sanity check */
		ASSERT(buf_len1 - build.buf_len == build.buf - buf1);
		l = buf_len1 - build.buf_len;
		ll += l;
		if (l > 0) {
			IEM_TRACE(("%s: len %u\n", __FUNCTION__, l));
			if (IEM_DUMP_ON()) {
				char name[16];
				snprintf(name, sizeof(name), "bt%u", tag);
				prhex(name, buf1, l);
			}
		}

		if (done)
			goto exit;
	}

	/* insert a new IE for the current tag */
	calc.cbparm = cbparm;
	calc.cfg = cfg;
	calc.ft = ft;
	calc.tag = tag;
	calc.ie = NULL;
	calc.ie_len = 0;
	ASSERT(cbe->calc != NULL);
	l = (cbe->calc)(cbe->ctx, &calc);
	ll += l;
	if (l == 0) {
		IEM_INFO(("%s: skip tag %u\n", __FUNCTION__, tag));
		goto exit;
	}
	build.tag = tag;
	build.ie = NULL;
	build.ie_len = 0;
	/* save/advance the buffer pointer explicitly
	 * in case the callback also modifies it
	 */
	buf1 = build.buf;
	buf_len1 = build.buf_len;
	ASSERT(cbe->build != NULL);
	err = (cbe->build)(cbe->ctx, &build);
	if (err != BCME_OK) {
		WL_ERROR(("%s: err %d\n", __FUNCTION__, err));
		return err;
	}
	tsf_l = IEM_T32D(cfg->wlc, tsf_l);
	IEM_TRACE(("%s: len %u time %u\n", __FUNCTION__, l, tsf_l));
	if (IEM_DUMP_ON()) {
		char name[16];
		snprintf(name, sizeof(name), "tag%u", tag);
		prhex(name, buf1, l);
	}

exit:
	*buf_len = ll;
	return BCME_OK;
}

/* Write IEs in a frame */
int
wlc_ieml_build_frame(wlc_bsscfg_t *cfg, uint16 ft,
	uint8 *build_tag, bool is_tag, wlc_iem_cbe_t *build_cb, uint16 build_cnt,
	wlc_iem_uiel_t *uiel, wlc_iem_cbparm_t *cbparm,
	uint8 *buf, uint buf_len)
{
	uint16 i;
	int err;
	int16 prev = -1;
	uint8 nocbvec[CEIL(NBITVAL(NBBY), NBBY)];
	uint8 *buf1;
	uint buf_len1;

	ASSERT(build_tag != NULL);
	ASSERT(build_cb != NULL);

	IEM_TRACE(("%s: ft 0x%04x is_tag %d\n", __FUNCTION__, ft, is_tag));

	/* figure out IEs that don't have any registered callbacks - 1's represent
	 * those IEs' that don't have any registered callbacks and whose indices
	 * are the IEs' tags.
	 */
	if (uiel != NULL) {
		memset(nocbvec, 0xff, sizeof(nocbvec));
		for (i = 0; i < build_cnt; i ++)
			clrbit(nocbvec, build_tag[i]);
	}

	/* walk through callbacks list and invoke them which then write IEs' to frame. */
	buf1 = buf;
	buf_len1 = buf_len;
	for (i = 0; i < build_cnt; i ++) {
		/* targeted IE */
		uint8 tag = build_tag[i];	/* this is 'prio' when 'is_tag' is FALSE */

		err = _wlc_iem_build_ie(cfg, ft, tag, is_tag, prev, &build_cb[i],
		                        uiel, cbparm, nocbvec, buf1, &buf_len1);
		if (err != BCME_OK)
			return err;

		buf1 += buf_len1;
		buf_len1 = buf_len - (uint)(buf1 - buf);

		prev = tag;
	}

	if (IEM_DUMP_ON()) {
		char name[16];
		snprintf(name, sizeof(name), "ft%04x", ft);
		prhex(name, buf, (uint)(buf1 - buf));
	}

	return BCME_OK;
}

/*
 * Invoke to write a specific IE into buffer
 *
 * 'build_tag', 'build_cb', and 'build_cnt' are the calc_len/build pair callback table
 * and size.
 * 'is_tag' indicates if the tag table 'build_tag' contains IE tags or VS IE PRIOs.
 * 'cfg' and 'ft' are the BSS and frame type that the function is called for, and are
 * passed to the 'build' callbacks as is.
 * 'buf' and 'buf_len' are the buffer and its length the IEs are written to.
 * 'tag' is the specific IE's tag.
 *
 * Note - doesn't work for User Supplied IE that doesn't have a registered callback
 * for its tag!
 */
int
wlc_ieml_build_ie(wlc_bsscfg_t *cfg, uint16 ft,
	uint8 *build_tag, bool is_tag, wlc_iem_cbe_t *build_cb, uint16 build_cnt, uint8 tag,
	wlc_iem_uiel_t *uiel, wlc_iem_cbparm_t *cbparm, uint8 *buf, uint buf_len)
{
	uint16 i;
	int err;
	int16 prev = -1;
	uint8 nocbvec[CEIL(NBITVAL(NBBY), NBBY)];
	uint8 *buf1;
	uint buf_len1;

	ASSERT(build_tag != NULL);
	ASSERT(build_cb != NULL);

	IEM_TRACE(("%s: ft 0x%04x is_tag %d\n", __FUNCTION__, ft, is_tag));

	/* figure out IEs that don't have any registered callbacks - 1's represent
	 * those IEs' that don't have any registered callbacks and whose indices
	 * are the IEs' tags.
	 */
	if (uiel != NULL) {
		memset(nocbvec, 0xff, sizeof(nocbvec));
		for (i = 0; i < build_cnt; i ++)
			clrbit(nocbvec, build_tag[i]);
	}

	/* walk through callbacks list and invoke them which then write IEs' to frame. */
	buf1 = buf;
	buf_len1 = buf_len;
	for (i = 0; i < build_cnt; i ++) {
		if (build_tag[i] == tag) {
			err = _wlc_iem_build_ie(cfg, ft, tag, is_tag, prev, &build_cb[i],
			                        uiel, cbparm, nocbvec, buf1, &buf_len1);
			if (err != BCME_OK)
				return err;

			buf1 += buf_len1;
			buf_len1 = buf_len - (uint)(buf1 - buf);
		}
		prev = build_tag[i];
	}

	if (IEM_DUMP_ON()) {
		char name[16];
		snprintf(name, sizeof(name), "ft%04x", ft);
		prhex(name, buf, (uint)(buf1 - buf));
	}

	return BCME_OK;
}

/* Register 'parse' callback for tag 'tag' */
void
BCMATTACHFN(wlc_ieml_add_parse_fn)(uint8 *parse_tag, wlc_iem_pe_t *parse_cb, uint16 parse_cnt,
	wlc_iem_parse_fn_t parse_fn, void *ctx,
	uint8 tag)
{
	wlc_iem_pe_t parse;

	parse.parse = parse_fn;
	parse.ctx = ctx;

	wlc_iem_add_cb(parse_tag, parse_cb, parse_cnt, &parse, sizeof(parse), tag);
}

/* Traverse IEs pointed by parse->buf pointer and call registered parse callbacks
 * for tags found in the callback registration table. For IEs without any parse
 * callbacks registered the 'notify' callback is invoked if it is specified by
 * the 'upp' parameter.
 * If the function is called for a Vendor Specific IE ('parse_tag' pointers to
 * a Vendor Specific IE ID table and 'is_tag' is FALSE) the 'vsie' callback is
 * invoked to query the Vendor Specific IE's ID first and above process is
 * applied then.
 * It is designed to expect one IE per IE tag (except Vendor Specific IEs) so
 * the registered callbacks should handle multiple IEs of the same tag themselves
 * by walking the IE buffer starting from the IE passed to them when invoked.
 */
int
wlc_ieml_parse_frame(wlc_bsscfg_t *cfg, uint16 ft,
	uint8 *parse_tag, bool is_tag, wlc_iem_pe_t *parse_cb, uint16 parse_cnt,
	wlc_iem_upp_t *upp, wlc_iem_pparm_t *pparm,
	uint8 *buf, uint buf_len)
{
	uint8 cbvec[CEIL(NBITVAL(NBBY), NBBY)];
	uint i;
	uint8 *ie;
	uint ie_len;
	wlc_iem_nhdlr_data_t ntie;
	wlc_iem_pvsie_data_t vsie;
	wlc_iem_parse_data_t parse;
	int err;

	IEM_TRACE(("%s: ft 0x%04x is_tag %d\n", __FUNCTION__, ft, is_tag));

	if (IEM_DUMP_ON()) {
		char name[16];
		snprintf(name, sizeof(name), "ft%04x", ft);
		prhex(name, buf, buf_len);
	}

	ASSERT(parse_tag != NULL);
	ASSERT(parse_cb != NULL);
	if (!is_tag) {
		ASSERT(upp != NULL);
		ASSERT(upp->vsie_fn != NULL);
	}

	/* figure out callbacks that haven't been called (i.e. IEs are not in the frames) */
	bzero(cbvec, sizeof(cbvec));
	for (i = 0; i < parse_cnt; i ++)
		setbit(cbvec, parse_tag[i]);

	ntie.pparm = pparm;
	ntie.cfg = cfg;
	ntie.ft = ft;
	vsie.pparm = pparm;
	vsie.cfg = cfg;
	vsie.ft = ft;

	parse.pparm = pparm;
	parse.cfg = cfg;
	parse.ft = ft;
	parse.buf = buf;
	parse.buf_len = buf_len;

	/* traverse all IEs */
	while ((ie = buf) != NULL && buf_len >= TLV_HDR_LEN &&
	       (ie_len = TLV_HDR_LEN + ie[TLV_LEN_OFF]) <= buf_len) {
		uint8 ie_tag = ie[TLV_TAG_OFF];
		int l, m, r;

		/* skip the IE if we have handled the tag earlier */
		if (is_tag &&
		    isclr(cbvec, ie_tag)) {
			IEM_TRACE(("%s: skip %u\n", __FUNCTION__, ie_tag));
			goto n_ie;
		}

		IEM_TRACE(("%s: tag %u\n", __FUNCTION__, ie_tag));

		/* handle Vendor Specific IE when !is_tag */
		if (!is_tag) {
			vsie.ie = ie;
			vsie.ie_len = ie_len;
			/* 'ie_tag' value is now the id */
			ie_tag = (upp->vsie_fn)(upp->ctx, &vsie);
			IEM_TRACE(("%s: vsie %u\n", __FUNCTION__, ie_tag));
		}

		/* binary search in the registered callbacks table */
		l = 0;
		r = parse_cnt - 1;
		while (l <= r) {
			m = (l + r) / 2;

			IEM_TRACE(("%s: left %u, mid %u, right %u\n", __FUNCTION__, l, m, r));

			if (ie_tag < parse_tag[m]) {
				r = m - 1;
				continue;
			}
			else if (ie_tag > parse_tag[m]) {
				l = m + 1;
				continue;
			}

			/* found one callback */
			IEM_TRACE(("%s: cbe %u tag %u\n", __FUNCTION__, m, ie_tag));

			/* invoke all callbacks for the tag and handle error... */
			while (m > 0 && m >= l && ie_tag == parse_tag[m - 1])
				m --;
			while (m <= r && ie_tag == parse_tag[m]) {
				if (IEM_DUMP_ON()) {
					char name[16];
					snprintf(name, sizeof(name), "tag%u", ie_tag);
					prhex(name, ie, ie_len);
				}
				parse.ie = ie;
				parse.ie_len = ie_len;
				ASSERT(parse_cb[m].parse != NULL);
				err = (parse_cb[m].parse)(parse_cb[m].ctx, &parse);
				if (err != BCME_OK) {
					WL_ERROR(("%s: err %d\n", __FUNCTION__, err));
					return err;
				}
				m ++;
			}

			/* mark that the callback was called */
			clrbit(cbvec, ie_tag);

			/* done with this IE */
			break;
		}

		/* no callback is found */
		if (l > r) {
			IEM_TRACE(("%s: tag %u not found\n", __FUNCTION__, ie_tag));
			if (upp != NULL && upp->notif_fn != NULL) {
				ntie.ie = ie;
				ntie.ie_len = ie_len;
				(upp->notif_fn)(upp->ctx, &ntie);
			}
		}

		/* next IE */
	n_ie:	buf += ie_len;
		buf_len -= ie_len;
	}

	/* sanity check */
	if (buf_len != 0) {
		WL_INFORM(("%s: wrong length\n", __FUNCTION__));
	}

	/* XXX Do we need to do this before processing presented IEs...which requires
	 * some extra code to figure out which callback hasn't been called...
	 */
	/* notify the callbacks that the IEs of interests are not found in the frame... */
	for (i = 0; i < parse_cnt; i ++) {
		/* the callback was invoked before... */
		if (isclr(cbvec, parse_tag[i]))
			continue;
		IEM_TRACE(("%s: cbe %u\n", __FUNCTION__, i));
		/* notify and handle error... */
		parse.ie = NULL;
		parse.ie_len = 0;
		ASSERT(parse_cb[i].parse != NULL);
		err = (parse_cb[i].parse)(parse_cb[i].ctx, &parse);
		if (err != BCME_OK) {
			WL_ERROR(("%s: err %d\n", __FUNCTION__, err));
			return err;
		}
	}

	return BCME_OK;
}
