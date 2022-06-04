/*
 * HND reserved packet pool operation
 *
 * Copyright (C) 2021, Broadcom. All Rights Reserved.
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 * SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
 * OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 * <<Broadcom-WL-IPTag/Proprietary:>>
 *
 * $Id:$
 */

#ifndef _hnd_resvpool_h_
#define _hnd_resvpool_h_

#include <osl_ext.h>

#ifdef __cplusplus
extern "C" {
#endif // endif

typedef enum _resv_mem_state {
	RESV_INVALID = 0,	/* RESERVED pool not in use and empty */
	RESV_PKTP = 1,		/* RESERVED pool in use */
	RESV_WAIT_FOR_PKTS = 2	/* Waiting for pkts to be freed to pool */
} resv_mem_state_t;

#define RESV_MAGIC 0x4B1DFACE

struct resv_info {
	uint magic;
	osl_t *osh;
	uint size;
	uint8 *mem;
	resv_mem_state_t state;
	/* pkt pool related */
	pktpool_t *pktp;
	uint8 *pkt_ptr;
	uint pkt_size;
	uint pktc;
};

void hnd_resv_dump(struct resv_info *ri);
struct resv_info* hnd_resv_pool_alloc(osl_t *osh);
void* hnd_resv_pool_init(struct resv_info *ri, int size);
int rsvpool_avail(pktpool_t *pktp);
void* rsvpool_get(pktpool_t *pktp);
void rsvpool_own(pktpool_t *pktp, int val);
int hnd_resv_pool_disable(struct resv_info *ri);
int hnd_resv_pool_enable(struct resv_info *ri);
int rsvpool_maxlen(pktpool_t *pktp);

#define RI_FROM_PKTP(p) ((struct resv_info*)((void*)(p)) - 1)

#ifdef __cplusplus
	}
#endif // endif

#endif /* _hnd_resvpool_h_ */
