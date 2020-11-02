/*
 * Packet ID to pointer mapping interface
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
 * $Id: $
 */

#ifndef _hnd_pktid_h_
#define _hnd_pktid_h_

#define PKT_INVALID_ID                  ((uint16)(~0))
#define PKT_NULL_ID                     ((uint16)(0))

#if defined(BCMPKTIDMAP)

#include <typedefs.h>
#include <osl_decl.h>
#include <hnd_lbuf.h>

/*
 * Compress a 32bit pkt pointer to 16bit unique ID to save memory in storing
 * pointers to packets. [In NIC mode (as opposed to dongle modes), there is no
 * memory constraint ... In NIC mode a pkt pointer may be 64bits. If a packet
 * pointer to packet ID suppression is desired in NIC mode, then the ability to
 * specify the maximum number of packets and a global pktptr_map is required.
 *
 * When a packet is first allocated from the heap, an ID is associated with it.
 * Packets allocated from the heap and filled into a pktpool will also get a
 * unique packet ID. When packets are allocated and freed into the pktpool, the
 * packet ID is preserved. In rte, lb_alloc() and lb_free_one() are the
 * only means to first instantiate (allocate from heap and initialize) a packet.
 * Packet ID management is performed in these functions.
 *
 * The lifetime of a packet ID is from the time it is allocated from the heap
 * until it is freed back to the heap.
 *
 * Packet ID 0 is reserved and never used and corresponds to NULL pktptr.
 */
#if !defined(PKT_MAXIMUM_ID)
#error "PKT_MAXIMUM_ID is not defined"
#endif  /* ! PKT_MAXIMUM_ID */

extern struct lbuf **hnd_pktptr_map;	/* exported pktid to pktptr map */

extern void hnd_pktid_init(osl_t *osh, uint32 pktids_total);

extern uint32 hnd_pktid_free_cnt(void);
extern uint32 hnd_pktid_fail_cnt(void);
extern void hnd_pktid_inc_fail_cnt(void);
extern uint16 hnd_pktid_allocate(const struct lbuf * pktptr);
extern void   hnd_pktid_release(const struct lbuf * pktptr, const uint16 pktid);
extern bool   hnd_pktid_sane(const struct lbuf * pktptr);

#else  /* ! BCMPKTIDMAP */

#if !defined(PKT_MAXIMUM_ID)
#define PKT_MAXIMUM_ID                  (0)
#endif  /* ! PKT_MAXIMUM_ID */

#define hnd_pktid_init(osh, pktids_total) do {} while (0)

#define hnd_pktid_free_cnt()         (1)
#define hnd_pktid_allocate(p)        (0)
#define hnd_pktid_release(p, i)      do {} while (0)
#define hnd_pktid_sane(p)            (TRUE)
#define hnd_pktid_fail_cnt()         (0)
#define hnd_pktid_inc_fail_cnt()     do {} while (0)

#endif /* ! BCMPKTIDMAP */

#endif /* _hnd_pktid_h_ */
