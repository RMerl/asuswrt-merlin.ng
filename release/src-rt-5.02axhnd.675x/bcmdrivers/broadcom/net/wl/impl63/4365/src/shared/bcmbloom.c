/*
 * Bloom filter support
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
 * $Id: bcmbloom.c 708017 2017-06-29 14:11:45Z $
 */

#include <typedefs.h>
#include <bcmdefs.h>

#if defined(__FreeBSD__) || defined(__NetBSD__)
#include <machine/stdarg.h>
#else
#include <stdarg.h>
#endif /* __FreeBSD__ */

#ifdef BCMDRIVER
#include <osl.h>
#include <bcmutils.h>
#else /* !BCMDRIVER */
#include <stdio.h>
#include <string.h>
#ifndef ASSERT
#define ASSERT(exp)
#endif // endif
#endif /* !BCMDRIVER */
#include <bcmutils.h>

#include <bcmbloom.h>

#define BLOOM_BIT_LEN(_x) ((_x) << 3)

struct bcm_bloom_filter {
	void *cb_ctx;
	uint max_hash;
	bcm_bloom_hash_t *hash;	/* array of hash functions */
	uint filter_size; 		/* in bytes */
	uint8 *filter; 			/* can be NULL for validate only */
};

/* public interface */
int
bcm_bloom_create(bcm_bloom_alloc_t alloc_cb,
	bcm_bloom_free_t free_cb, void *cb_ctx, uint max_hash,
	uint filter_size, bcm_bloom_filter_t **bloom)
{
	int err = BCME_OK;
	bcm_bloom_filter_t *bp = NULL;

	if (!bloom || !alloc_cb || (max_hash == 0)) {
		err = BCME_BADARG;
		goto done;
	}

	bp = (*alloc_cb)(cb_ctx, sizeof(*bp));
	if (!bp) {
		err = BCME_NOMEM;
		goto done;
	}

	memset(bp, 0, sizeof(*bp));
	bp->cb_ctx = cb_ctx;
	bp->max_hash = max_hash;
	bp->hash = (*alloc_cb)(cb_ctx, sizeof(*bp->hash) * max_hash);
	if (!bp->hash) {
		err = BCME_NOMEM;
		goto done;
	}

	if (filter_size > 0) {
		bp->filter = (*alloc_cb)(cb_ctx, filter_size);
		if (!bp->filter) {
			err = BCME_NOMEM;
			goto done;
		}
		bp->filter_size = filter_size;
		memset(bp->filter, 0, filter_size);
	}

	*bloom = bp;

done:
	if (err != BCME_OK)
		bcm_bloom_destroy(&bp, free_cb);

	return err;
}

int
bcm_bloom_destroy(bcm_bloom_filter_t **bloom, bcm_bloom_free_t free_cb)
{
	int err = BCME_OK;
	bcm_bloom_filter_t *bp;

	if (!bloom || !*bloom || !free_cb)
		goto done;

	bp = *bloom;
	*bloom = NULL;

	if (bp->filter)
		(*free_cb)(bp->cb_ctx, bp->filter);
	if (bp->hash)
		(*free_cb)(bp->cb_ctx, bp->hash);
	(*free_cb)(bp->cb_ctx, bp);

done:
	return err;
}

int
bcm_bloom_add_hash(bcm_bloom_filter_t *bp, bcm_bloom_hash_t hash, uint *idx)
{
	uint i;

	if (!bp || !hash || !idx)
		return BCME_BADARG;

	for (i = 0; i < bp->max_hash; ++i) {
		if (bp->hash[i] == NULL)
			break;
	}

	if (i >= bp->max_hash)
		return BCME_NORESOURCE;

	bp->hash[i] = hash;
	*idx = i;
	return BCME_OK;
}

int
bcm_bloom_remove_hash(bcm_bloom_filter_t *bp, uint idx)
{
	if (!bp)
		return BCME_BADARG;

	if (idx >= bp->max_hash)
		return BCME_NOTFOUND;

	bp->hash[idx] = NULL;
	return BCME_OK;
}

bool
bcm_bloom_is_member(bcm_bloom_filter_t *bp,
	const uint8 *tag, uint tag_len, const uint8 *buf, uint buf_len)
{
	uint i;
	int err = BCME_OK;

	if (!bp->filter) {
		err = BCME_BADARG;
		goto done;

	}

	if (!tag || (tag_len == 0)) /* empty tag is always a member */
		goto done;

	/* use internal buffer if none was specified */
	if (!buf || (buf_len == 0)) {
		if (!bp->filter)	/* every one is a member of empty filter */
			goto done;

		buf = bp->filter;
		buf_len = bp->filter_size;
	}

	for (i = 0; i < bp->max_hash; ++i) {
		uint pos;
		if (!bp->hash[i])
			continue;
		pos = (*bp->hash[i])(bp->cb_ctx, i, tag, tag_len);

		/* all bits must be set for a match */
		if (isclr(buf, pos % BLOOM_BIT_LEN(buf_len))) {
			err = BCME_NOTFOUND;
			break;
		}
	}

done:
	return err;
}

int
bcm_bloom_add_member(bcm_bloom_filter_t *bp, const uint8 *tag, uint tag_len)
{
	uint i;

	if (!bp || !tag || (tag_len == 0))
		return BCME_BADARG;

	if (!bp->filter)		/* validate only */
		return BCME_UNSUPPORTED;

	for (i = 0; i < bp->max_hash; ++i) {
		uint pos;
		if (!bp->hash[i])
			continue;
		pos = (*bp->hash[i])(bp->cb_ctx, i, tag, tag_len);
		setbit(bp->filter, pos % BLOOM_BIT_LEN(bp->filter_size));
	}

	return BCME_OK;
}

int bcm_bloom_get_filter_data(bcm_bloom_filter_t *bp,
	uint buf_size, uint8 *buf, uint *buf_len)
{
	if (!bp)
		return BCME_BADARG;

	if (buf_len)
		*buf_len = bp->filter_size;

	if (buf_size < bp->filter_size)
		return BCME_BUFTOOSHORT;

	if (bp->filter && bp->filter_size)
		memcpy(buf, bp->filter, bp->filter_size);

	return BCME_OK;
}
