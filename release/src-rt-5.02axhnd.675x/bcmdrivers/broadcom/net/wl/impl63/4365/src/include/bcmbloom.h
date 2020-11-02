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
 * $Id: bcmbloom.h 708017 2017-06-29 14:11:45Z $
 */

#ifndef _bcmbloom_h_
#define _bcmbloom_h_

#include <typedefs.h>
#ifdef BCMDRIVER
#include <osl.h>
#else
#include <stddef.h>  /* For size_t */
#endif // endif

struct bcm_bloom_filter;
typedef struct bcm_bloom_filter bcm_bloom_filter_t;

typedef void* (*bcm_bloom_alloc_t)(void *ctx, uint size);
typedef void (*bcm_bloom_free_t)(void *ctx, void *buf);
typedef uint (*bcm_bloom_hash_t)(void* ctx, uint idx, const uint8 *tag, uint len);

/* create/allocate a bloom filter. filter size can be 0 for validate only filters */
int bcm_bloom_create(bcm_bloom_alloc_t alloc_cb,
	bcm_bloom_free_t free_cb, void *callback_ctx, uint max_hash,
	uint filter_size /* bytes */, bcm_bloom_filter_t **bloom);

/* destroy bloom filter */
int bcm_bloom_destroy(bcm_bloom_filter_t **bloom, bcm_bloom_free_t free_cb);

/* add a hash function to filter, return an index */
int bcm_bloom_add_hash(bcm_bloom_filter_t *filter, bcm_bloom_hash_t hash, uint *idx);

/* remove the hash function at index from filter */
int bcm_bloom_remove_hash(bcm_bloom_filter_t *filter, uint idx);

/* check if given tag is member of the filter. If buf is NULL and/or buf_len is 0
 * then use the internal state. BCME_OK if member, BCME_NOTFOUND if not,
 * or other error (e.g. BADARG)
 */
bool bcm_bloom_is_member(bcm_bloom_filter_t *filter,
	const uint8 *tag, uint tag_len, const uint8 *buf, uint buf_len);

/* add a member to the filter. invalid for validate_only filters */
int bcm_bloom_add_member(bcm_bloom_filter_t *filter, const uint8 *tag, uint tag_len);

/* no support for remove member */

/* get the filter data from state. BCME_BUFTOOSHORT w/ required length in buf_len
 * if supplied size is insufficient
 */
int bcm_bloom_get_filter_data(bcm_bloom_filter_t *filter,
	uint buf_size, uint8 *buf, uint *buf_len);

#endif /* _bcmbloom_h_ */
