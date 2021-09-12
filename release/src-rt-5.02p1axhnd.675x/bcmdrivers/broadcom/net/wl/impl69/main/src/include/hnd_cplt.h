/*
 * This is specific to pcie full dongle for now.
 * handles data structures and functions for rx completion operation
 * Shared utils between bus layer and WL layer
 *
 * Copyright (C) 2020, Broadcom. All Rights Reserved.
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
 * $Id$
 */

#ifndef _hnd_cplt_h_
#define _hnd_cplt_h_
#include <osl.h>

/* RXCPL definitions */
/* MAX_HOST_RXBUFS limited to 2048 */
#define BCM_MAC_RXCPL_IDX_BITS			11
#define BCM_MAX_RXCPL_IDX_INVALID		0
/* Max interfaces supported are 32. Inline with HWA_RXCPLE_IFID */
#define BCM_MAC_RXCPL_IFIDX_BITS		5
#define BCM_MAC_RXCPL_DOT11_BITS		1
#define BCM_MAX_RXCPL_IFIDX			((1 << BCM_MAC_RXCPL_IFIDX_BITS) - 1)
#define BCM_MAC_RXCPL_FLAG_BITS			4
#define BCM_RXCPL_FLAGS_IN_TRANSIT		0x1
#define BCM_RXCPL_FLAGS_FIRST_IN_FLUSHLIST	0x2
#define BCM_RXCPL_FLAGS_RXCPLVALID		0x4
#define BCM_RXCPL_FLAGS_PKT_FETCHD		0x8

#define BCM_RXCPL_CONSECUTIVE_FAIL_COUNT_THRESH		10

/* RXCPL Macros */
#define BCM_RXCPL_SET_PKT_FETCHD(a)     ((a)->rxcpl_id.flags |= BCM_RXCPL_FLAGS_PKT_FETCHD)
#define BCM_RXCPL_CLR_PKT_FETCHD(a)     ((a)->rxcpl_id.flags &= ~BCM_RXCPL_FLAGS_PKT_FETCHD)
#define BCM_RXCPL_IS_PKT_FETCHD(a)      ((a)->rxcpl_id.flags & BCM_RXCPL_FLAGS_PKT_FETCHD)

#define BCM_RXCPL_SET_IN_TRANSIT(a)	((a)->rxcpl_id.flags |= BCM_RXCPL_FLAGS_IN_TRANSIT)
#define BCM_RXCPL_CLR_IN_TRANSIT(a)	((a)->rxcpl_id.flags &= ~BCM_RXCPL_FLAGS_IN_TRANSIT)
#define BCM_RXCPL_IN_TRANSIT(a)		((a)->rxcpl_id.flags & BCM_RXCPL_FLAGS_IN_TRANSIT)

#define BCM_RXCPL_SET_FRST_IN_FLUSH(a)	((a)->rxcpl_id.flags |= BCM_RXCPL_FLAGS_FIRST_IN_FLUSHLIST)
#define BCM_RXCPL_CLR_FRST_IN_FLUSH(a)	((a)->rxcpl_id.flags &= ~BCM_RXCPL_FLAGS_FIRST_IN_FLUSHLIST)
#define BCM_RXCPL_FRST_IN_FLUSH(a)	((a)->rxcpl_id.flags & BCM_RXCPL_FLAGS_FIRST_IN_FLUSHLIST)

#define BCM_RXCPL_SET_VALID_INFO(a)	((a)->rxcpl_id.flags |= BCM_RXCPL_FLAGS_RXCPLVALID)
#define BCM_RXCPL_CLR_VALID_INFO(a)	((a)->rxcpl_id.flags &= ~BCM_RXCPL_FLAGS_RXCPLVALID)
#define BCM_RXCPL_VALID_INFO(a) (((a)->rxcpl_id.flags & BCM_RXCPL_FLAGS_RXCPLVALID) ? TRUE : FALSE)

#define UP_TABLE_MAX	((IPV4_TOS_DSCP_MASK >> IPV4_TOS_DSCP_SHIFT) + 1)	/* 64 max */

/* Rx completion cookie data structures */
struct reorder_rxcpl_id_list {
	uint16 head;
	uint16 tail;
	uint32 cnt;
};

typedef struct reorder_rxcpl_id_list reorder_rxcpl_id_list_t;

typedef struct rxcpl_id {
	uint32		idx : BCM_MAC_RXCPL_IDX_BITS;
	uint32		next_idx : BCM_MAC_RXCPL_IDX_BITS;
	uint32		ifidx : BCM_MAC_RXCPL_IFIDX_BITS;
	uint32		dot11 : BCM_MAC_RXCPL_DOT11_BITS;
	uint32		flags : BCM_MAC_RXCPL_FLAG_BITS;
} rxcpl_idx_id_t;

typedef struct rxcpl_data_len {
	uint32		metadata_len_w : 6;
	uint32		dataoffset: 10;
	uint32		datalen : 16;
} rxcpl_data_len_t;

typedef struct rxcpl_info {
	rxcpl_idx_id_t		rxcpl_id;
	uint32			host_pktref;
	union {
		rxcpl_data_len_t	rxcpl_len;
		struct rxcpl_info	*free_next;
	};
} rxcpl_info_t;

/* rx completion list */
typedef struct bcm_rxcplid_list {
	uint32			max;
	uint32			avail;
	rxcpl_info_t		*rxcpl_ptr;
	rxcpl_info_t		*free_list;
} bcm_rxcplid_list_t;

/* rxcpl function definitions */
extern bool BCMATTACHFN(bcm_alloc_rxcplid_list)(osl_t *osh, uint32 max);
extern rxcpl_info_t * bcm_alloc_rxcplinfo(void);
extern void bcm_free_rxcplinfo(rxcpl_info_t *ptr);
extern void bcm_chain_rxcplid(uint16 first,  uint16 next);
extern rxcpl_info_t *bcm_id2rxcplinfo(uint16 id);
extern uint16 bcm_rxcplinfo2id(rxcpl_info_t *ptr);
extern rxcpl_info_t *bcm_rxcpllist_end(rxcpl_info_t *ptr, uint32 *count);
extern void bcm_rxcpllist_dump(uint16 head);
extern bcm_rxcplid_list_t *g_rxcplid_list;
#endif /* _hnd_cplt_h_ */
