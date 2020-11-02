/*
 * Key Management Module Implementation - replay/iv trace window support
 * Copyright (c) 2013 Broadcom Corporation, All rights reserved.
 * $Id: km_ivtw.c 778965 2019-09-16 13:29:05Z $
 */

#include "km.h"
#include "km_key.h"

#ifdef BRCMAPIVTW

#include "km_ivtw.h"

typedef unsigned long long km_uint64_t;

#define IVTW_VALID(_ivtw) ((_ivtw) != NULL)
#define IVTW_LOG(args) KM_LOG(args)
#define IVTW_NONE(args) KM_NONE(args)
#define IVTW_ERR(args) KM_ERR(args)
#define IVTW_WLC(_ivtw) ((_ivtw)->wlc)
#define IVTW_UNIT(_ivtw) WLCWLUNIT(IVTW_WLC(_ivtw))
#define IVTW_KM(_ivtw) (IVTW_WLC(_ivtw)->keymgmt)
#define IVTW_OSH(_ivtw) IVTW_WLC(_ivtw)->osh
#define IVTW_PUB(_ivtw) IVTW_WLC(_ivtw)->pub
#define IVTW_NUM_RX_SEQ(_ivtw) (IVTW_PUB(_ivtw)->tunables->num_rxivs)

#if defined(DONGLEBUILD) && !defined(BCMROMBUILD)
/* #define IVTW_DONGLE_DEBUGGING */
#endif /* DONGLEBUILD  && !BCMROMBUILD */

#ifdef IVTW_DONGLE_DEBUGGING
#undef IVTW_ERR
#define IVTW_ERR(args) printf args
#undef IVTW_LOG
#define IVTW_LOG(args) printf args
#endif /* IVTW_DONGLE_DEBUGGING */

#define IVTW_SEQ_SIZE 6
#define IVTW_TWSIZE	BRCMAPIVTW

#define IVTW_SEQ_TO_U64(_seq) ((km_uint64_t)(uint32)ltoh32_ua(_seq) | \
	((km_uint64_t)(ltoh16_ua((uint8 *)(_seq) + 4)) << 32))

#define U64_TO_IVTW_SEQ(_val, _seq) {\
	htol32_ua_store((km_uint64_t)(_val) & 0xffffffff, (uint8 *)(_seq)); \
	htol16_ua_store((((km_uint64_t)(_val) & 0x0000ffff00000000ULL) >> 32), \
		(uint8 *)(_seq) + 4); \
}

/* ivtw sequence info - contains a window of IVTW_TWSIZE, starting at
 * and including lb - the lower bound.
 */
struct ivtw_seq_info {
	km_uint64_t		wstart;					/* window start */
	uint8			window[CEIL(IVTW_TWSIZE, NBBY)];
};
typedef struct ivtw_seq_info ivtw_seq_info_t;

struct ivtw_key_info {
	ivtw_seq_info_t seq_info[1];
};
typedef struct ivtw_key_info ivtw_key_info_t;

#define SIZEOF_IVTW_KEY_INFO(_ivtw) (OFFSETOF(ivtw_key_info_t, seq_info) +\
	IVTW_NUM_RX_SEQ(_ivtw) * sizeof(ivtw_seq_info_t))

struct km_ivtw {
	wlc_info_t			*wlc;
	int					mode; 	/* ON, OFF or AUTO */
	uint16				max_keys;
	ivtw_key_info_t		**key_info; /* 0..max_keys - 1 */
};
typedef struct km_ivtw ivtw_t;

static bool
ivtw_is_replay(ivtw_t *ivtw, ivtw_seq_info_t *seq_info, const uint8 *rx_seq)
{
	bool replay = FALSE;
	km_uint64_t lb = seq_info->wstart;
	km_uint64_t hb = lb + IVTW_TWSIZE - 1;
	km_uint64_t rx = IVTW_SEQ_TO_U64(rx_seq);

	if (rx < lb) { /* rx before window */
		replay = TRUE;
		goto done;
	}

	if (rx <= hb) { /* in window */
		if (isset(seq_info->window, (rx - lb))) { /* already seen */
			replay = TRUE;
			goto done;
		}
	}

done:
	IVTW_LOG(("wl%d: %s: replay %d - rx %d.%d lb %d.%d\n",
		IVTW_UNIT(ivtw), __FUNCTION__, replay,
		(uint32)((rx >> 32) & 0xffffffff), (uint32)(rx & 0xffffffff),
		(uint32)((lb >> 32) & 0xffffffff), (uint32)(lb & 0xffffffff)));
	return replay;
}

static void
ivtw_update(ivtw_t *ivtw, ivtw_seq_info_t *seq_info, const uint8 *rx_seq)
{
	bool in_tw;
	km_uint64_t lb = seq_info->wstart;
	km_uint64_t hb = lb + IVTW_TWSIZE - 1;
	km_uint64_t rx = IVTW_SEQ_TO_U64(rx_seq);

	/* already replay checked */
	KM_DBG_ASSERT(rx >=  lb);

	in_tw = (rx <= hb) ? TRUE: FALSE;
	if (!in_tw) { /* adjust window to include rx */
		km_uint64_t new_hb = rx;
		km_uint64_t new_lb = new_hb - (IVTW_TWSIZE - 1);
		km_uint64_t ovl_off;
		km_uint64_t ovl_len;
		int i;

		/* no overlap with current window */
		if (new_lb > hb) {
			memset(seq_info->window, 0, sizeof(seq_info->window));
			goto upd_tw;
		}

		/* skip received for new window */
		ovl_off = new_lb - lb;
		ovl_len = hb - new_lb + 1;
		KM_ASSERT((ovl_off + ovl_len) <= IVTW_TWSIZE);
		while (ovl_len > 0 && isset(seq_info->window, ovl_off)) {
			++ovl_off; --ovl_len;
			++new_lb;
		}

		KM_ASSERT((ovl_off + ovl_len) <= IVTW_TWSIZE);
		new_hb = new_lb + IVTW_TWSIZE - 1;

		/* no overlap with current window */
		if (!ovl_len) {
			memset(seq_info->window, 0, sizeof(seq_info->window));
			goto upd_tw;
		}

		/* move overlap to new window */
		for (i = 0; i < IVTW_TWSIZE; ++i) {
			if ((i < ovl_len) && isset(seq_info->window, ovl_off + i))
				setbit(seq_info->window, i);
			else
				clrbit(seq_info->window, i);
		}

upd_tw:
		seq_info->wstart = new_lb;
		lb = new_lb;
	}

	KM_ASSERT((rx - lb) < IVTW_TWSIZE);
	setbit(seq_info->window, rx - lb);

	IVTW_LOG(("wl%d: %s: rx %d.%d lb %d.%d\n",
		IVTW_UNIT(ivtw), __FUNCTION__,
		(uint32)((rx >> 32) & 0xffffffff), (uint32)(rx & 0xffffffff),
		(uint32)((lb >> 32) & 0xffffffff), (uint32)(lb & 0xffffffff)));
}

int
km_ivtw_set(km_ivtw_t *ivtw, wlc_key_info_t *key_info, int ins,
	const uint8 *rx_seq, size_t seq_len) {

	ivtw_key_info_t *ivtw_key_info;
	ivtw_seq_info_t *seq_info;
	km_uint64_t lb;
	km_uint64_t rx;

	KM_DBG_ASSERT(IVTW_VALID(ivtw));
	KM_DBG_ASSERT(key_info != NULL);
	KM_DBG_ASSERT(key_info->key_idx < ivtw->max_keys);
	KM_DBG_ASSERT(ins <  IVTW_NUM_RX_SEQ(ivtw));

	ivtw_key_info = ivtw->key_info[key_info->key_idx];
	if (!ivtw_key_info) {
		return BCME_BADKEYIDX;
	}

	seq_info = &ivtw_key_info->seq_info[ins];
	lb = seq_info->wstart;
	rx = IVTW_SEQ_TO_U64(rx_seq);
	if (!rx) {
		return BCME_OK;
	}
	if (rx < lb) {
		return BCME_REPLAY;
	}

	km_ivtw_update(ivtw, key_info, ins, rx_seq, seq_len);

	return BCME_OK;
}

/* public interface */
km_ivtw_t*
BCMATTACHFN(km_ivtw_attach)(wlc_info_t *wlc, wlc_keymgmt_t *km)
{
	ivtw_t *ivtw;
	int err = BCME_OK;

	KM_DBG_ASSERT(wlc != NULL && km != NULL);

	ivtw = (ivtw_t *)MALLOCZ(wlc->osh, sizeof(ivtw_t));
	if (!ivtw) {
		err = BCME_NORESOURCE;
		goto done;
	}

	ivtw->wlc = wlc;
	ivtw->max_keys = km_get_max_keys(km) & 0xffff;
	/* default is mode auto, but not enabled */
	ivtw->mode = AUTO;
	ivtw->key_info = (ivtw_key_info_t **)MALLOCZ(wlc->osh,
		(sizeof(ivtw_key_info_t *) * ivtw->max_keys));
	if (!ivtw->key_info) {
		err = BCME_NORESOURCE;
		goto done;
	}

done:
	if (err != BCME_OK) {
		km_ivtw_detach(&ivtw);
		KM_DBG_ASSERT(ivtw == NULL);
	}

	IVTW_LOG(("wl%d: %s: allocated ivtw@%p status %d\n", WLCWLUNIT(wlc), __FUNCTION__,
		ivtw, err));
	return ivtw;
}

void
BCMATTACHFN(km_ivtw_detach)(km_ivtw_t **in_ivtw)
{
	ivtw_t *ivtw;
	if (!in_ivtw || !(*in_ivtw))
		return;

	ivtw = *in_ivtw;
	*in_ivtw = NULL;

	if (ivtw->key_info) {
		int i;
		for (i = 0; i < ivtw->max_keys; ++i) {
			ivtw_key_info_t *ivtw_key_info;
			ivtw_key_info = ivtw->key_info[i];
			if (!ivtw_key_info)
				continue;
			MFREE(IVTW_OSH(ivtw), ivtw_key_info, SIZEOF_IVTW_KEY_INFO(ivtw));
		}
		MFREE(IVTW_OSH(ivtw), ivtw->key_info,
			(sizeof(ivtw_key_info_t *) * ivtw->max_keys));
	}
	MFREE(IVTW_OSH(ivtw), ivtw, sizeof(ivtw_t));
}

int km_ivtw_get_mode(km_ivtw_t *ivtw)
{
	KM_DBG_ASSERT(IVTW_VALID(ivtw));
	return ivtw->mode;
}

int km_ivtw_set_mode(km_ivtw_t *ivtw, int val)
{
	KM_DBG_ASSERT(IVTW_VALID(ivtw));
	if (val != AUTO && val != OFF && val != ON)
		return BCME_RANGE;
	ivtw->mode = val;
	return BCME_OK;
}

int
km_ivtw_enable(km_ivtw_t *ivtw, wlc_key_index_t key_idx, bool enable)
{
	ivtw_key_info_t **ivtw_key_info;
	bool is_enabled = FALSE;
	int err = BCME_OK;
	int i;

	KM_DBG_ASSERT(IVTW_VALID(ivtw));

	if (key_idx >= ivtw->max_keys) {
		err = BCME_BADKEYIDX;
		goto done;
	}

	switch (ivtw->mode) {
	case ON:
		enable = TRUE; break;
	case OFF:
		enable = FALSE; break;
	case AUTO:
		break;
	}

	ivtw_key_info = &ivtw->key_info[key_idx];
	is_enabled = ((*ivtw_key_info) != NULL);

	if (enable == is_enabled)
		goto done;
	else if (!enable) {
		KM_ASSERT(*ivtw_key_info != NULL);
		if (!*ivtw_key_info)
			goto done;
		MFREE(IVTW_OSH(ivtw), *ivtw_key_info, SIZEOF_IVTW_KEY_INFO(ivtw));
		*ivtw_key_info = NULL;
		goto done;
	}

	/* need to enable */
	KM_ASSERT(*ivtw_key_info == NULL);
	*ivtw_key_info = (ivtw_key_info_t *)MALLOCZ(IVTW_OSH(ivtw), SIZEOF_IVTW_KEY_INFO(ivtw));
	if (!(*ivtw_key_info)) {
		err = BCME_NOMEM;
		goto done;
	}

	/* init tw for all seq/iv */
	for (i = 0; i < IVTW_NUM_RX_SEQ(ivtw); ++i) {
		ivtw_seq_info_t *seq_info = &(*ivtw_key_info)->seq_info[i];
		seq_info->wstart = 1;
	}

done:
	IVTW_LOG(("wl%d: %s: key index %d status %d enable %d was enabled %d\n",
		IVTW_UNIT(ivtw), __FUNCTION__, key_idx, err, enable, is_enabled));
	return err;
}

int
km_ivtw_reset(km_ivtw_t *ivtw,  wlc_key_index_t key_idx)
{
	int err = BCME_OK;
	ivtw_key_info_t *ivtw_key_info;
	bool enabled = FALSE;
	int i;

	KM_DBG_ASSERT(IVTW_VALID(ivtw));
	if (key_idx >= ivtw->max_keys) {
		err = BCME_BADKEYIDX;
		goto done;
	}

	ivtw_key_info = ivtw->key_info[key_idx];
	enabled = (ivtw_key_info != NULL);
	if (!enabled)
		goto done;

	memset(ivtw_key_info, 0,  SIZEOF_IVTW_KEY_INFO(ivtw));
	for (i = 0; i < IVTW_NUM_RX_SEQ(ivtw); ++i) {
		ivtw_seq_info_t *seq_info = &ivtw_key_info->seq_info[i];
		seq_info->wstart = 1;
	}

done:
	IVTW_LOG(("wl%d: %s: key index %d status %d enabled %d\n",
		IVTW_UNIT(ivtw), __FUNCTION__, key_idx, err, enabled));
	return err;
}

bool
km_ivtw_is_replay(km_ivtw_t *ivtw, wlc_key_info_t *key_info, int ins,
	uint8 *key_seq, const uint8 *rx_seq, size_t seq_len)
{
	ivtw_key_info_t *ivtw_key_info;
	ivtw_seq_info_t *seq_info;

	KM_DBG_ASSERT(IVTW_VALID(ivtw));
	KM_DBG_ASSERT(key_info != NULL);
	KM_DBG_ASSERT(key_info->key_idx < ivtw->max_keys);
	KM_DBG_ASSERT(ins <  IVTW_NUM_RX_SEQ(ivtw));

	ivtw_key_info = ivtw->key_info[key_info->key_idx];
	if (!ivtw_key_info) {
		IVTW_LOG(("wl%d: %s: key index %d no ivtw key info or not enabled\n",
			IVTW_UNIT(ivtw), __FUNCTION__, key_info->key_idx));
		key_info->flags &= ~WLC_KEY_FLAG_USE_IVTW;
		return !km_key_seq_less(key_seq, rx_seq, seq_len);
	}

	seq_info = &ivtw_key_info->seq_info[ins];
	return ivtw_is_replay(ivtw, seq_info, rx_seq);
}

void
km_ivtw_update(km_ivtw_t *ivtw, wlc_key_info_t *key_info, int ins,
	const uint8 *rx_seq, size_t seq_len)
{
	ivtw_key_info_t *ivtw_key_info;
	ivtw_seq_info_t *seq_info;

	KM_DBG_ASSERT(IVTW_VALID(ivtw));
	KM_DBG_ASSERT(key_info != NULL);
	KM_DBG_ASSERT(key_info->key_idx < ivtw->max_keys);
	KM_DBG_ASSERT(ins <  IVTW_NUM_RX_SEQ(ivtw));

	ivtw_key_info = ivtw->key_info[key_info->key_idx];
	if (ivtw_key_info) {
		seq_info = &ivtw_key_info->seq_info[ins];
		ivtw_update(ivtw, seq_info, rx_seq);
	}
}

#if defined(BCMDBG) || defined(BCMDBG_DUMP)
void
km_ivtw_dump(km_ivtw_t *ivtw, struct bcmstrbuf *b)
{
	int i, j, k;

	KM_DBG_ASSERT(IVTW_VALID(ivtw));

	bcm_bprintf(b, "begin wl%d ivtww dump: mode %d, max keys %d\n",
		IVTW_UNIT(ivtw), ivtw->mode, ivtw->max_keys);
	for (i = 0; i < ivtw->max_keys; ++i) {
		const ivtw_key_info_t *ivtw_key_info = ivtw->key_info[i];
		if (!ivtw_key_info)
			continue;
		bcm_bprintf(b, "\tivtw key info: key index %d\n", i);
		for (j = 0; j < IVTW_NUM_RX_SEQ(ivtw); ++j) {
			const ivtw_seq_info_t *seq_info = &ivtw_key_info->seq_info[j];
			bcm_bprintf(b, "\t\tivtw seq info: seq index %d, lb %d.%d\n",
				j, seq_info->wstart >> 32, seq_info->wstart & 0xffffffff);
			bcm_bprintf(b, "\t\t\twindow: 0x");
			for (k = sizeof(seq_info->window) - 1; k >= 0; --k) {
				bcm_bprintf(b, "%02x", seq_info->window[k]);
			}
			bcm_bprintf(b, "\n");
		}
	}
	bcm_bprintf(b, "end ivtw dump\n");
}
#endif /* BCMDBG || BCMDBG_DUMP */

#endif /* BRCMAPIVTW */
