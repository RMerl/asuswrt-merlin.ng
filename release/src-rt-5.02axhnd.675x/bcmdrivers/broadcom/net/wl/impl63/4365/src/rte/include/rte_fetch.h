/*
 * HostFetch module interface
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
 * $Id: rte_fetch.h 683757 2017-02-09 01:58:16Z $
 */

#ifndef _rte_fetch_h_
#define _rte_fetch_h_

#include <typedefs.h>
#include <osl_decl.h>
#include <bcmutils.h>
#include <sbhnddma.h>

/* Private fetch_rqst flags
 * Make sure these flags are cleared before dispatching
 */
#define FETCH_RQST_IN_BUS_LAYER 0x01
#define FETCH_RQST_CANCELLED 0x02

/* External utility API */
#define FETCH_RQST_FLAG_SET(fr, bit) ((fr)->flags |= bit)
#define FETCH_RQST_FLAG_GET(fr, bit) ((fr)->flags & bit)
#define FETCH_RQST_FLAG_CLEAR(fr, bit) ((fr)->flags &= (~bit))

typedef struct fetch_rqst fetch_rqst_t;

typedef void (*fetch_cmplt_cb_t)(struct fetch_rqst *rqst, bool cancelled);

struct fetch_rqst {
	osl_t *osh;
	dma64addr_t haddr;
	uint16 size;
	uint8 flags;
	uint8 rsvd;
	uint8 *dest;
	fetch_cmplt_cb_t cb;
	void *ctx;
	struct fetch_rqst *next;
};

typedef int (*bus_dispatch_cb_t)(struct fetch_rqst *fr, void *arg);

struct fetch_module_info {
	struct pktpool *pool;
	bus_dispatch_cb_t cb;
	void *arg;
};

extern struct fetch_module_info *fetch_info;

void hnd_fetch_bus_dispatch_cb_register(bus_dispatch_cb_t cb, void *arg);
void hnd_fetch_rqst(struct fetch_rqst *fr);
void hnd_wake_fetch_rqstq(void);
bool hnd_fetch_rqstq_empty(void);
void hnd_flush_fetch_rqstq(void);
void hnd_dmadesc_avail_cb(void);
int hnd_cancel_fetch_rqst(struct fetch_rqst *fr);

void hnd_fetch_module_init(osl_t *osh);

#endif /* _rte_fetch_h_ */
