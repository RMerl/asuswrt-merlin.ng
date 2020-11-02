/*
 * Implementation of a bcm_rpc_tp based on the
 * bcm_rpc_char simple character device bus API
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
 * $Id: bcm_rpc_char.c 708017 2017-06-29 14:11:45Z $
 */

#include <osl.h>
#include <typedefs.h>

#include <bcm_rpc_tp.h>
#include <bcmendian.h>
#include <bcm_rpc_char.h>

static void bcm_rpc_tp_rx(void *ctx, char *buf, uint len);

#define RPC_BUS_TX_OVERHEAD sizeof(uint32)

struct rpc_transport_info {
	osl_t *osh;
	rpc_osl_t *rpc_osh;
	chardev_bus_t *cbus;

	uint bufalloc;
	int buf_cnt;
	uint tx_cnt;
	uint txerr_cnt;
	uint rx_cnt;
	uint rxdrop_cnt;

	rpc_tx_complete_fn_t tx_complete;
	void* tx_context;

	rpc_rx_fn_t rx_pkt;
	void* rx_context;
};

struct rpc_buf {
	char *buf;
	uint buflen;
	char *data;
	uint len;
};

rpc_tp_info_t *
bcm_rpc_tp_attach(osl_t * osh, void *bus)
{
	rpc_tp_info_t *rpcb;

	rpcb = (rpc_tp_info_t*)MALLOC(osh, sizeof(rpc_tp_info_t));
	if (rpcb == NULL) {
		printf("%s: rpc_tp_info_t malloc failed\n", __FUNCTION__);
		return NULL;
	}

	memset(rpcb, 0, sizeof(rpc_tp_info_t));

	rpcb->osh = osh;
	rpcb->cbus = (chardev_bus_t*)bus;

	chardev_register_callback(rpcb->cbus, bcm_rpc_tp_rx, rpcb);

	return rpcb;
}

void
bcm_rpc_tp_detach(rpc_tp_info_t * rpcb)
{
	ASSERT(rpcb);
	MFREE(rpcb->osh, rpcb, sizeof(rpc_tp_info_t));
}

static void
bcm_rpc_tp_rx(void *ctx, char *buf, uint len)
{
	rpc_tp_info_t *rpcb = (rpc_tp_info_t*)ctx;
	rpc_buf_t *b;

	rpcb->rx_cnt++;

	if (rpcb->rx_pkt == NULL) {
		printf("%s: no rpc rx fn, dropping\n", __FUNCTION__);
		rpcb->rxdrop_cnt++;
		return;
	}

	b = bcm_rpc_tp_buf_alloc(rpcb, len);
	if (b == NULL)
		return;

	memcpy(b->data, buf, len);

	printf("%s: calling rpc rx fn\n", __FUNCTION__);
	(rpcb->rx_pkt)(rpcb->rx_context, b);
}

void
bcm_rpc_tp_register_cb(rpc_tp_info_t * rpcb,
                               rpc_tx_complete_fn_t txcmplt, void* tx_context,
                               rpc_rx_fn_t rxpkt, void* rx_context,
                               rpc_osl_t *rpc_osh)
{
	rpcb->tx_complete = txcmplt;
	rpcb->tx_context = tx_context;
	rpcb->rx_pkt = rxpkt;
	rpcb->rx_context = rx_context;
	rpcb->rpc_osh = rpc_osh;

	/* ?? need this ?? ASSERT(rpc_osh); */
}

void
bcm_rpc_tp_deregister_cb(rpc_tp_info_t * rpcb)
{
	rpcb->tx_complete = NULL;
	rpcb->tx_context = NULL;
	rpcb->rx_pkt = NULL;
	rpcb->rx_context = NULL;
	rpcb->rpc_osh = NULL;
}

int
bcm_rpc_tp_buf_send(rpc_tp_info_t * rpcb, rpc_buf_t *b)
{
	int err;

	/* initialize the bus packet length */
	htol32_ua_store(b->len, (uint8*)b->buf);

	err = chardev_send(rpcb->cbus, b->buf, b->len + RPC_BUS_TX_OVERHEAD);
	if (err) {
		rpcb->txerr_cnt++;
	} else {
		bcm_rpc_tp_buf_free(rpcb, b);
	}

	rpcb->tx_cnt++;
	return err;
}

/* Buffer manipulation */

rpc_buf_t *
bcm_rpc_tp_buf_alloc(rpc_tp_info_t * rpcb, int len)
{
	uint alloc_len;
	rpc_buf_t * b;

	alloc_len = sizeof(rpc_buf_t) + RPC_BUS_TX_OVERHEAD + len;
	b = (rpc_buf_t*)MALLOC(rpcb->osh, alloc_len);

	if (b != NULL) {
		memset(b, 0, alloc_len);
		b->buf = (char*)(b + 1);
		b->buflen = RPC_BUS_TX_OVERHEAD + len;
		b->data = b->buf + RPC_BUS_TX_OVERHEAD;
		b->len = len;

		rpcb->bufalloc++;
		rpcb->buf_cnt++;
	}

	return b;
}

void
bcm_rpc_tp_buf_free(rpc_tp_info_t * rpcb, rpc_buf_t *b)
{
	uint alloc_len;

	ASSERT(b);

	alloc_len = sizeof(rpc_buf_t) + b->buflen;

	MFREE(rpcb->osh, b, alloc_len);
	rpcb->buf_cnt--;
}

int
bcm_rpc_buf_len_get(rpc_tp_info_t * rpcb, rpc_buf_t* b)
{
	return b->len;
}

int
bcm_rpc_buf_len_set(rpc_tp_info_t * rpcb, rpc_buf_t* b, uint len)
{
	ASSERT(b->data + len <= b->buf + b->buflen);

	if (b->data + len > b->buf + b->buflen)
		return 1;

	b->len = len;

	return 0;
}

unsigned char*
bcm_rpc_buf_data(rpc_tp_info_t * rpcb, rpc_buf_t* b)
{
	return (unsigned char *)(b->data);
}

unsigned char*
bcm_rpc_buf_push(rpc_tp_info_t * rpcb, rpc_buf_t* b, uint bytes)
{
	ASSERT((b->data - bytes) >= b->buf);

	b->data -= bytes;
	b->len += bytes;

	return (unsigned char *)(b->data);
}

unsigned char*
bcm_rpc_buf_pull(rpc_tp_info_t * rpcb, rpc_buf_t* b, uint bytes)
{
	ASSERT(b->len >= bytes);

	b->data += bytes;
	b->len -= bytes;

	return (unsigned char *)(b->data);
}
