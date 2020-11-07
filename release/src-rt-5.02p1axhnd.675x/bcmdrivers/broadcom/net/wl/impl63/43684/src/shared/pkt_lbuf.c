/*
 * Network interface packet buffer routines. These are used internally by the
 * driver to allocate/de-allocate packet buffers, and manipulate the buffer
 * contents and attributes.
 *
 * This implementation is specific to LBUF (linked buffer) packet buffers.
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
 * $Id: pkt_lbuf.c 467150 2014-04-02 17:30:43Z $
 */

/* ---- Include Files ---------------------------------------------------- */

#include "typedefs.h"
#include "bcmdefs.h"
#include "osl.h"
#include "pkt_lbuf.h"
#include "lbuf.h"
#include <stdlib.h>

/* ---- Public Variables ------------------------------------------------- */
/* ---- Private Constants and Types -------------------------------------- */

#define PKT_LBUF_LOG(a)	printf a

/* ---- Private Variables ------------------------------------------------ */

static lbuf_info_t *g_lbuf_info;

/* ---- Private Function Prototypes -------------------------------------- */
/* ---- Functions -------------------------------------------------------- */

/* ----------------------------------------------------------------------- */
pkt_handle_t*
pkt_init(osl_t *osh)
{
	lbuf_info_t	*info;

	if (!(info = (lbuf_info_t *)MALLOC(osh, sizeof(lbuf_info_t)))) {
		PKT_LBUF_LOG(("malloc failed for lbuf_info_t\n"));
		ASSERT(0);
	}
	memset(info, 0, sizeof(lbuf_info_t));
	info->osh = osh;

	if (!lbuf_alloc_list(info, &(info->txfree), NTXBUF)) {
		PKT_LBUF_LOG(("lbuf_alloc_list for TX fail \n"));
		ASSERT(0);
	}

	if (!lbuf_alloc_list(info, &(info->rxfree), NRXBUF)) {
		PKT_LBUF_LOG(("lbuf_alloc_list for RX fail \n"));
		ASSERT(0);
	}

	g_lbuf_info = info;

	return ((pkt_handle_t*)info);
}

/* ----------------------------------------------------------------------- */
void
pkt_deinit(osl_t *osh, pkt_handle_t *pkt_info)
{
	lbuf_info_t	*info = (lbuf_info_t*)pkt_info;

	lbuf_free_list(info, &(info->rxfree));
	lbuf_free_list(info, &(info->txfree));

	MFREE(osh, info, sizeof(lbuf_info_t));
}

/* ----------------------------------------------------------------------- */
/* Converts a native (network interface) packet to driver packet.
 * Allocates a new lbuf and copies the contents
 */
void *
pkt_frmnative(osl_t *osh, void *native_pkt, int len)
{
	struct lbuf* lb;
	lbuf_info_t *info = g_lbuf_info;
	struct lbfree *list = &info->txfree;

	ASSERT(osh);

	/* Alloc driver packet (lbuf) */
	if ((lb = lbuf_get(list)) == NULL)
		return (NULL);

	/* Adjust for the head room requested */
	ASSERT(list->size > list->headroom);
	lb->data += list->headroom;
	lb->tail += list->headroom;

	/* Copy data from native to driver packet. */
	pkt_get_native_pkt_data(osh, native_pkt, PKTDATA(osh, lb), PKTTAILROOM(osh, lb));
	pktsetlen(osh, lb, len);

	/* Save pointer to native packet. This will be freed later by pktfree(). */
	lb->native_pkt = native_pkt;

	/* Return driver packet. */
	return ((void *)lb);
}

/* ----------------------------------------------------------------------- */
/* Converts a driver packet to a native (network interface) packet.
 */
void* pkt_tonative(osl_t *osh, void *drv_pkt)
{
	void *native_pkt;
	struct lbuf* lb = (struct lbuf*) drv_pkt;

	ASSERT(osh);

	/* Allocate network interface (native) packet. */
	native_pkt = pkt_alloc_native(osh, PKTLEN(osh, drv_pkt));

	/* Copy data from driver to network interface packet */
	pkt_set_native_pkt_data(osh, native_pkt, PKTDATA(osh, lb), PKTLEN(osh, lb));

	return (native_pkt);
}

/* ----------------------------------------------------------------------- */
void*
pktget(osl_t *osh, uint len, bool send)
{
	struct lbuf	*lb;
	lbuf_info_t *info = g_lbuf_info;

	ASSERT(osh);

	if (len > LBUFSZ)
		return (NULL);

	if (send)
		lb = lbuf_get(&info->txfree);
	else
		lb = lbuf_get(&info->rxfree);

	if (lb)
		lb->len = len;

	return ((void*) lb);
}

/* ----------------------------------------------------------------------- */
void
pktfree(osl_t *osh, struct lbuf	*lb, bool send)
{
	struct lbuf *next;
	void *native_pkt;

	native_pkt = lb->native_pkt;

	ASSERT(osh);
	ASSERT(lb);

	while (lb) {
		next = lb->next;
		lb->next = NULL;

		lbuf_put(lb->list, lb);

		lb = next;
	}

	/* Free network interface (native) packet. */
	if (native_pkt != NULL)
		pkt_free_native(osh, native_pkt);
}

/* ----------------------------------------------------------------------- */
void pkt_set_headroom(osl_t *osh, bool tx, unsigned int headroom)
{
	struct lbfree *list;

	if (tx)
		list = &g_lbuf_info->txfree;
	else
		list = &g_lbuf_info->rxfree;

	list->headroom = headroom;
}
