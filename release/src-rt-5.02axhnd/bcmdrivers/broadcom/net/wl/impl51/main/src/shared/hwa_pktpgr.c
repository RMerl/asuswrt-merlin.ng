#ifdef RTE_MODEL
// Need to decide whether to create a new hwa_pp.c or simply use hwa_lib.c
// Need to figure what moves from attach/preinit to init/deinit disable/enable
#include <rte_wl.h>
#endif // endif
// +-------------------------  cut here --------------------------------------+

/*
 * HWA Packet Pager Library routines
 * - response ring handlers
 * - attach, config, preinit, init, status and debug dump
 *
 * Copyright 2019 Broadcom
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
 * <<Broadcom-WL-IPTag/Proprietary:>>
 *
 * $Id$
 *
 * vim: set ts=4 noet sw=4 tw=80:
 * -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
 */

#include <typedefs.h>
#include <osl.h>
#include <bcmutils.h>
#include <bcmpcie.h>
#include <hwa_lib.h>

#if defined(HWA_PKTPGR_BUILD)

/**
 * ---------------------------------------------------------------------------+
 * XXX Miscellaneous Packet Pager related registers|fields in other blocks:
 *
 * Block : Registers with Packet Pager related fields.
 * ----- : -------------------------------------------------------------------+
 * common: module_clk_enable, module_clkgating_enable, module_enable,
 *         module_reset, module_clkavail, module_idle
 *         dmatxsel, dmarxsel, hwa2hwcap, pageinstatus, pageinmask
 * rx    : rxpsrc_ring_hwa2cfg,
 *         d11bdest_threshold_l1l0, d11bdest_threshold_l2,
 *         hwa_rx_d11bdest_ring_wrindex_dir,
 *         pagein_status, recycle_status
 * txdma : pp_pagein_cfg, pp_pageout_cfg, pp_pagein_sts, pp_pageout_sts
 *
 * ---------------------------------------------------------------------------+
 */

/**
 * +--------------------------------------------------------------------------+
 *  Section: Handlers invoked to process Packet Pager Responses
 *  Parameter "hwa_pp_cmd_t *" is treated as a const pointer.
 * +--------------------------------------------------------------------------+
 */
// Forward declarations of all response ring handlers
#define HWA_PKTPGR_HANDLER_DECLARE(resp) \
static int hwa_pktpgr_ ## resp(void *ctx, hwa_dev_t *dev, hwa_pp_cmd_t *pp_cmd)

	                                            // PAGEIN response handlers
HWA_PKTPGR_HANDLER_DECLARE(pagein_rxprocess);   //  HWA_PKTPGR_PAGEIN_RXPROCESS
HWA_PKTPGR_HANDLER_DECLARE(pagein_txstatus);    //  HWA_PKTPGR_PAGEIN_TXSTATUS
HWA_PKTPGR_HANDLER_DECLARE(pagein_txpost);      //  HWA_PKTPGR_PAGEIN_TXPOST
	                                            // PAGEOUT response handlers
HWA_PKTPGR_HANDLER_DECLARE(pageout_pktlist);    //  HWA_PKTPGR_PAGEOUT_PKTLIST
HWA_PKTPGR_HANDLER_DECLARE(pageout_local);      //  HWA_PKTPGR_PAGEOUT_LOCAL
	                                            // PAGEMGR response handlers
HWA_PKTPGR_HANDLER_DECLARE(pagemgr_alloc_rx);   //  HWA_PKTPGR_PAGEMGR_ALLOC_RX
HWA_PKTPGR_HANDLER_DECLARE(pagemgr_alloc_rph);  //  HWA_PKTPGR_PAGEMGR_ALLOC_RPH
HWA_PKTPGR_HANDLER_DECLARE(pagemgr_alloc_tx);   //  HWA_PKTPGR_PAGEMGR_ALLOC_TX
HWA_PKTPGR_HANDLER_DECLARE(pagemgr_push);       //  HWA_PKTPGR_PAGEMGR_PUSH
HWA_PKTPGR_HANDLER_DECLARE(pagemgr_pull);       //  HWA_PKTPGR_PAGEMGR_PULL

static int
hwa_pktpgr_response(hwa_dev_t *dev, hwa_ring_t *ring, hwa_callback_t callback);

int // HWA_PKTPGR_PAGEIN_RXPROCESS handler
hwa_pktpgr_pagein_rxprocess(void *ctx, hwa_dev_t *dev, hwa_pp_cmd_t *pp_cmd)
{
	hwa_pp_pagein_rsp_rxprocess_t *rxprocess = &pp_cmd->pagein_rsp_rxprocess;

	HWA_FTRACE(HWApp);
	HWA_ASSERT(rxprocess->trans == HWA_PP_PAGEIN_RXPROCESS);

	// XXX --- process pktlist

	return rxprocess->trans_id;

}   // hwa_pktpgr_pagein_rxprocess()

int // HWA_PKTPGR_PAGEIN_TXSTATUS handler
hwa_pktpgr_pagein_txstatus(void *ctx, hwa_dev_t *dev, hwa_pp_cmd_t *pp_cmd)
{
	hwa_pp_pagein_rsp_txstatus_t *txstatus = &pp_cmd->pagein_rsp_txstatus;

	HWA_FTRACE(HWApp);
	HWA_ASSERT(txstatus->trans == HWA_PP_PAGEIN_TXSTATUS);

	// XXX --- process pktlist

	return txstatus->trans_id;

}   // hwa_pktpgr_pagein_txstatus()

int // HWA_PKTPGR_PAGEIN_TXPOST handler
hwa_pktpgr_pagein_txpost(void *ctx, hwa_dev_t *dev, hwa_pp_cmd_t *pp_cmd)
{
	hwa_pp_pagein_rsp_txpost_t *txpost = &pp_cmd->pagein_rsp_txpost;

	HWA_FTRACE(HWApp);
	HWA_ASSERT((txpost->trans == HWA_PP_PAGEIN_TXPOST_WITEMS) ||
	           (txpost->trans == HWA_PP_PAGEIN_TXPOST_WITEMS_FRC));

	// XXX --- process pktlist

	return txpost->trans_id;

}   // hwa_pktpgr_pagein_txpost()

int // HWA_PKTPGR_PAGEOUT_PKTLIST handler
hwa_pktpgr_pageout_pktlist(void *ctx, hwa_dev_t *dev, hwa_pp_cmd_t *pp_cmd)
{
	hwa_pp_pageout_rsp_pktlist_t *pktlist = &pp_cmd->pageout_rsp_pktlist;

	HWA_FTRACE(HWApp);
	HWA_ASSERT(pktlist->trans == HWA_PP_PAGEOUT_PKTLIST_WR);

	// XXX --- if nothing to do, register default noop handler instead.

	return pktlist->trans_id;

}   // hwa_pktpgr_pageout_pktlist()

int // HWA_PKTPGR_PAGEOUT_LOCAL handler
hwa_pktpgr_pageout_local(void *ctx, hwa_dev_t *dev, hwa_pp_cmd_t *pp_cmd)
{
	hwa_pp_pageout_rsp_pktlist_t *pktlocal = &pp_cmd->pageout_rsp_pktlist;

	HWA_FTRACE(HWApp);
	HWA_ASSERT(pktlocal->trans == HWA_PP_PAGEOUT_PKTLOCAL);

	// XXX --- free locally allocated TxLfrag

	return pktlocal->trans_id;

}   // hwa_pktpgr_pageout_local()

int // HWA_PKTPGR_PAGEMGR_ALLOC_RX handler
hwa_pktpgr_pagemgr_alloc_rx(void *ctx, hwa_dev_t *dev, hwa_pp_cmd_t *pp_cmd)
{
	hwa_pp_pagemgr_rsp_alloc_t *alloc_rx = &pp_cmd->pagemgr_rsp_alloc;

	HWA_FTRACE(HWApp);
	HWA_ASSERT(alloc_rx->trans == HWA_PP_PAGEMGR_ALLOC_RX);

	// XXX --- use allocated rxlfrags

	return alloc_rx->trans_id;

}   // hwa_pktpgr_pagemgr_alloc_rx()

int // HWA_PKTPGR_PAGEMGR_ALLOC_RPH handler
hwa_pktpgr_pagemgr_alloc_rph(void *ctx, hwa_dev_t *dev, hwa_pp_cmd_t *pp_cmd)
{
	hwa_pp_pagemgr_rsp_alloc_t *alloc_rph = &pp_cmd->pagemgr_rsp_alloc;

	HWA_FTRACE(HWApp);
	HWA_ASSERT(alloc_rph->trans == HWA_PP_PAGEMGR_ALLOC_RX_RPH);

	// XXX --- use allocated rxpost host info

	return alloc_rph->trans_id;

}   // hwa_pktpgr_pagemgr_alloc_rph()

int // HWA_PKTPGR_PAGEMGR_ALLOC_TX handler
hwa_pktpgr_pagemgr_alloc_tx(void *ctx, hwa_dev_t *dev, hwa_pp_cmd_t *pp_cmd)
{
	hwa_pp_pagemgr_rsp_alloc_t *alloc_tx = &pp_cmd->pagemgr_rsp_alloc;

	HWA_FTRACE(HWApp);
	HWA_ASSERT(alloc_tx->trans == HWA_PP_PAGEMGR_ALLOC_TX);

#ifdef RTE_MODEL
	wl_pmprocess(ctx, alloc_tx->trans, alloc_tx->pkt_count,
	             (hwa_pp_lbuf_t*)alloc_tx->pktlist_head,
	             (hwa_pp_lbuf_t*)alloc_tx->pktlist_tail);
#endif // endif
	return alloc_tx->trans_id;

}   // hwa_pktpgr_pagemgr_alloc_tx()

int // HWA_PKTPGR_PAGEMGR_PUSH handler
hwa_pktpgr_pagemgr_push(void *ctx, hwa_dev_t *dev, hwa_pp_cmd_t *pp_cmd)
{
	hwa_pp_pagemgr_rsp_push_t *push = &pp_cmd->pagemgr_rsp_push;

	HWA_FTRACE(HWApp);
	HWA_ASSERT(push->trans == HWA_PP_PAGEMGR_PUSH);

	// XXX --- if nothing to do, register default noop handler

	return push->trans_id;

}   // hwa_pktpgr_pagemgr_push()

int // HWA_PKTPGR_PAGEMGR_PULL handler
hwa_pktpgr_pagemgr_pull(void *ctx, hwa_dev_t *dev, hwa_pp_cmd_t *pp_cmd)
{
	hwa_pp_pagemgr_rsp_pull_t *pull = &pp_cmd->pagemgr_rsp_pull;

	HWA_FTRACE(HWApp);
	HWA_ASSERT(pull->trans == HWA_PP_PAGEMGR_PULL);

	// XXX --- use fetched packet list

	return pull->trans_id;

}   // hwa_pktpgr_pagemgr_pull()

/**
 * +--------------------------------------------------------------------------+
 *  Generic Packet Pager consumer loop. Each element of the Response ring is
 *  processed by invoking the handler identified by the transaction command type
 *
 *  All elements from RD to WR index are processed, unbounded. Each response
 *  element is holding onto packets in dongle's packet storage, and needs to be
 *  promptly processed to completion.
 *  Bounding algorithms should be enforced during requests.
 *
 *  There are no response rings corresponding to Free pkt/rph/d11b requests.
 * +--------------------------------------------------------------------------+
 */
int
hwa_pktpgr_response(hwa_dev_t *dev, hwa_ring_t *rsp_ring,
	hwa_callback_t callback)
{
	uint8          trans_cmd;
	uint8          trans_id;        // last processed transaction's id
	uint32         elem_ix;         // location of next element to read
	hwa_pp_cmd_t  *rsp_cmd;         // pointer to command in h2s response ring
	hwa_handler_t *rsp_handler;     // response handler
	hwa_pktpgr_t  *pktpgr;          // pktpgr local state

	HWA_FTRACE(HWApp);
	HWA_AUDIT_DEV(dev);

	pktpgr = &dev->pktpgr;
	trans_id = ~0;

	hwa_ring_cons_get(rsp_ring);    // fetch response ring's WR index once

	while ((elem_ix = hwa_ring_cons_upd(rsp_ring)) != BCM_RING_EMPTY) {
		rsp_cmd     = HWA_RING_ELEM(hwa_pp_cmd_t, rsp_ring, elem_ix);
		trans_cmd   = rsp_cmd->u8[HWA_PP_CMD_TRANS];
		trans_id    = rsp_cmd->u8[HWA_PP_CMD_TRANS_ID];
		rsp_handler = &dev->handlers[callback + trans_cmd];

		(*rsp_handler->callback)(rsp_handler->context, dev, rsp_cmd);
	}

	return trans_id;

}    // hwa_pktpgr_response()

/**
 * +--------------------------------------------------------------------------+
 *  Generic API to post a Request command into one of the S2H interfaces
 *  to the Packet Pager. Caller must compose/marshall/pack a Packet Pager
 *  command, and pass to this requst API. This API copies the 16B command into
 *  a slot in the request interface. If the request ring is full a HWA_FAILURE
 *  is returned, otherwise a transaction id is returned.
 *
 *  Transaction ID is a 8bit incrementing unsigned char. As a request ring may
 *  be deeper than 256 elements, a transaction Id does not necessarily identify
 *  a unique RD index in the ring.
 * +--------------------------------------------------------------------------+
 */

int // HWApp: Post a request command
hwa_pktpgr_request(hwa_dev_t *dev, hwa_pktpgr_ring_t pp_ring, void *pp_cmd)
{
	uint8_t       trans_id;         // per s2h_ring incrementing transaction id
	hwa_ring_t   *req_ring;         // sw to hw request ring
	hwa_pp_cmd_t *req_cmd;          // location in s2h ring to place the cmd
	hwa_pktpgr_t *pktpgr;           // pktpgr local state

	HWA_FTRACE(HWApp);
	HWA_ASSERT(dev != (struct hwa_dev *)NULL);
	HWA_ASSERT(pp_ring < hwa_pktpgr_req_ring_max);

	pktpgr = &dev->pktpgr;
	req_ring = pktpgr->req_ring[pp_ring];

	if (hwa_ring_is_full(req_ring))
		goto failure;

	trans_id = pktpgr->trans_id[pp_ring]++; // increment transaction id
	req_cmd  = HWA_RING_PROD_ELEM(hwa_pp_cmd_t, req_ring);

	*req_cmd = *((hwa_pp_cmd_t*)pp_cmd);    // copy 16 byte command into ring
	req_cmd->u8[HWA_PP_CMD_TRANS_ID] = trans_id; // overwrite trans_id

	hwa_ring_prod_upd(req_ring, 1, TRUE);	// update/commit WR

	return trans_id;

failure:
	HWA_TRACE(("%s req ring %u pp_cmd %u failure\n", HWApp,
		pp_ring, ((hwa_pp_cmd_t*)pp_cmd)->u8[HWA_PP_CMD_TRANS_ID]));

	return HWA_FAILURE;

}   // hwa_pktpgr_request()

// +--------------------------------------------------------------------------+

hwa_ring_t * // Initialize SW and HW ring contexts.
BCMATTACHFN(hwa_pktpgr_ring_init)(hwa_ring_t *hwa_ring,
	const char *ring_name, uint8 ring_dir, uint8 ring_num,
	uint16 depth, void *memory,
	hwa_regs_t *regs, hwa_pp_ring_t *hwa_pp_ring)
{
	uint32 v32;
	hwa_ring_init(hwa_ring, ring_name, HWA_PKTPGR_ID, ring_dir, ring_num,
		depth, memory, &hwa_pp_ring->wr_index, &hwa_pp_ring->rd_index);

	v32 = HWA_PTR2UINT(memory);
	HWA_WR_REG_ADDR(ring_name, &hwa_pp_ring->addr, v32);

	HWA_ASSERT(depth == HWA_PKTPGR_INTERFACE_DEPTH);
	v32 = BCM_SBF(depth, HWA_PAGER_PP_RING_CFG_DEPTH);
	if (ring_dir == HWA_RING_S2H) {
		v32 |= BCM_SBF(HWA_PKTPGR_REQ_WAITTIME,
		               HWA_PAGER_PP_REQ_RING_CFG_WAITTIME);
		if (ring_num == HWA_PKTPGR_PAGEIN_S2H_RINGNUM) {
			v32 |= BCM_SBF(HWA_PKTPGR_RX_BESTEFFORT,
			               HWA_PAGER_PP_PAGEIN_REQ_RING_CFG_PPINRXPROCESSBE);
		}
	}
	HWA_WR_REG_ADDR(ring_name, &hwa_pp_ring->cfg, v32);

	v32 = BCM_SBF(HWA_PKTPGR_INTRAGGR_COUNT,
	              HWA_PAGER_PP_RING_LAZYINT_CFG_AGGRCOUNT)
	    | BCM_SBF(HWA_PKTPGR_INTRAGGR_TMOUT,
	              HWA_PAGER_PP_RING_LAZYINT_CFG_AGGRTIMER);
	HWA_WR_REG_ADDR(ring_name, &hwa_pp_ring->lazyint_cfg, v32);

	return hwa_ring;

}   // hwa_pktpgr_ring_init()

hwa_pktpgr_t * // HWApp: Allocate all required memory for PktPgr
BCMATTACHFN(hwa_pktpgr_attach)(hwa_dev_t *dev)
{
	int ring;
	uint32 v32, mem_sz, depth;
	hwa_regs_t *regs;
	hwa_pager_regs_t *pager_regs;
	hwa_pktpgr_t *pktpgr;
	void *memory[hwa_pktpgr_ring_max], *ring_memory;

	HWA_FTRACE(HWApp);

	// Audit pre-conditions
	HWA_AUDIT_DEV(dev);

	// Setup locals
	regs = dev->regs;
	pager_regs = &regs->pager;
	pktpgr = &dev->pktpgr;

	// Verify PP block structures
	HWA_ASSERT(HWA_PP_LBUF_SZ ==
	           (HWA_PP_PKTCONTEXT_BYTES + HWA_PP_PKTDATABUF_BYTES));
	HWA_ASSERT(sizeof(hwa_pp_cmd_t) == HWA_PP_COMMAND_BYTES);

	mem_sz = sizeof(hwa_pp_lbuf_t) * HWA_DNGL_PKTS_MAX;
	pktpgr->dnglpktpool_mem = MALLOCZ(dev->osh, mem_sz);
	if (pktpgr->dnglpktpool_mem == NULL) {
		HWA_ERROR(("%s lbuf pool items<%u> mem_size<%u> failure\n",
			HWApp, HWA_DNGL_PKTS_MAX, mem_sz));
		HWA_ASSERT(pktpgr->dnglpktpool_mem != ((hwa_pp_lbuf_t *)NULL));
		goto lbuf_pool_failure;
	}
	pktpgr->dnglpktpool_max = HWA_DNGL_PKTS_MAX;

	// Allocate and initialize the S2H Request and H2S Response interfaces
	depth = HWA_PKTPGR_INTERFACE_DEPTH;
	mem_sz = depth * sizeof(hwa_pp_cmd_t);
	for (ring = 0; ring < hwa_pktpgr_ring_max; ++ring) {
		if ((memory[ring] = MALLOCZ(dev->osh, mem_sz)) == NULL) {
			HWA_ERROR(("%s ring %u size<%u> failure\n", HWApp, ring, mem_sz));
			HWA_ASSERT(memory[ring] != (void*)NULL);
			goto ring_failure;
		}
	}

	// Enable PP support in HWA 2.2. see also hwa_config
	v32 = HWA_RD_REG_NAME(HWApp, dev->regs, common, hwa2hwcap);
	v32 |= BCM_SBIT(HWA_COMMON_HWA2HWCAP_HWPPSUPPORT);
	HWA_WR_REG_NAME(HWApp, dev->regs, common, hwa2hwcap, v32);

	// Setup Rx Block with RxLfrag databuffer offset in words and to
	// use internal D11Bdest and default Rxblock to PAGER mode
	v32 = HWA_RD_REG_NAME(HWApp, regs, rx_core[0], rxpsrc_ring_hwa2cfg);
	v32 |= BCM_SBIT(HWA_RX_RXPSRC_RING_HWA2CFG_PP_INT_D11B)
	    | BCM_SBIT(HWA_RX_RXPSRC_RING_HWA2CFG_PP_PAGER_MODE);
	HWA_WR_REG_NAME(HWApp, regs, rx_core[0], rxpsrc_ring_hwa2cfg, v32);

	// Lbuf Context includes 4 FragInfo's Haddr64. In RxPath, only one fraginfo
	// is needed, and the memory of the remaining three are repurposed for
	// databuffer, allowing larger a 152 Byte databuffer.
	v32 = HWA_RD_REG_NAME(HWApp, regs, rx_core[0], pagein_status);
	v32 = BCM_CBF(v32, HWA_RX_PAGEIN_STATUS_PP_RXLFRAG_DATA_BUF_LEN)
		| BCM_SBF(HWA_PP_RXLFRAG_DATABUF_LEN_U32,
			HWA_RX_PAGEIN_STATUS_PP_RXLFRAG_DATA_BUF_LEN);
	v32 = BCM_CBF(v32, HWA_RX_PAGEIN_STATUS_PP_RXLFRAG_DATA_BUF_OFFSET)
		| BCM_SBF(HWA_PP_RXLFRAG_DATABUF_OFFSET_U32,
			HWA_RX_PAGEIN_STATUS_PP_RXLFRAG_DATA_BUF_OFFSET);
	HWA_WR_REG_NAME(HWApp, regs, rx_core[0], pagein_status, v32);

	v32 = BCM_SBF(HWA_PKTPGR_D11BDEST_TH0,
	              HWA_RX_D11BDEST_THRESHOLD_L1L0_PP_D11THRESHOLD_L0)
	    | BCM_SBF(HWA_PKTPGR_D11BDEST_TH1,
	              HWA_RX_D11BDEST_THRESHOLD_L1L0_PP_D11THRESHOLD_L1);
	HWA_WR_REG_NAME(HWApp, regs, rx_core[0], d11bdest_threshold_l1l0, v32);
	v32 = BCM_SBF(HWA_PKTPGR_D11BDEST_TH2,
	              HWA_RX_D11BDEST_THRESHOLD_L2_PP_D11THRESHOLD_L2);
	HWA_WR_REG_NAME(HWApp, regs, rx_core[0], d11bdest_threshold_l2, v32);

	// Turn on common::module_enable bit packet pager: see hwa_enable()

	// Describe Lbuf::Context and Lbuf::Databuffer size
	v32 = BCM_SBF(HWA_PP_PKTCONTEXT_BYTES,
	              HWA_PAGER_PP_PKTCTX_SIZE_PPPKTCTXSIZE);
	HWA_WR_REG_NAME(HWApp, regs, pager, pp_pktctx_size, v32);
	v32 = BCM_SBF(HWA_PP_PKTDATABUF_BYTES,
	              HWA_PAGER_PP_PKTBUF_SIZE_PPPKTBUFSIZE);
	HWA_WR_REG_NAME(HWApp, regs, pager, pp_pktbuf_size, v32);

	// Configure the DMA Descriptor template
	v32 = 0U
	    // All interface's ring memory is coherent and NotPCIe
	    | BCM_SBIT(HWA_PAGER_PP_DMA_DESCR_TEMPLATE_PPINREQNOTPCIE)
		| BCM_SBIT(HWA_PAGER_PP_DMA_DESCR_TEMPLATE_PPINREQCOHERENT)
		| BCM_SBIT(HWA_PAGER_PP_DMA_DESCR_TEMPLATE_PPINRSPNOTPCIE)
		| BCM_SBIT(HWA_PAGER_PP_DMA_DESCR_TEMPLATE_PPINRSPCOHERENT)
		| BCM_SBIT(HWA_PAGER_PP_DMA_DESCR_TEMPLATE_PPOUTREQNOTPCIE)
		| BCM_SBIT(HWA_PAGER_PP_DMA_DESCR_TEMPLATE_PPOUTREQCOHERENT)
		| BCM_SBIT(HWA_PAGER_PP_DMA_DESCR_TEMPLATE_PPOUTRSPNOTPCIE)
		| BCM_SBIT(HWA_PAGER_PP_DMA_DESCR_TEMPLATE_PPOUTRSPCOHERENT)
		| BCM_SBIT(HWA_PAGER_PP_DMA_DESCR_TEMPLATE_PPALLOCREQNOTPCIE)
		| BCM_SBIT(HWA_PAGER_PP_DMA_DESCR_TEMPLATE_PPALLOCREQCOHERENT)
		| BCM_SBIT(HWA_PAGER_PP_DMA_DESCR_TEMPLATE_PPALLOCRSPNOTPCIE)
		| BCM_SBIT(HWA_PAGER_PP_DMA_DESCR_TEMPLATE_PPALLOCRSPCOHERENT)
		| BCM_SBIT(HWA_PAGER_PP_DMA_DESCR_TEMPLATE_PPFREEREQNOTPCIE)
		| BCM_SBIT(HWA_PAGER_PP_DMA_DESCR_TEMPLATE_PPFREEREQCOHERENT)
		| BCM_SBIT(HWA_PAGER_PP_DMA_DESCR_TEMPLATE_PPFREERPHREQNOTPCIE)
		| BCM_SBIT(HWA_PAGER_PP_DMA_DESCR_TEMPLATE_PPFREERPHREQCOHERENT)

	// HostPktPool is "not" coherent as Router SoC never touches this
	// HostPktPool IS-PCIE, so need to comment out BCM_SBIT NOTPCIE
		| BCM_SBIT(HWA_PAGER_PP_DMA_DESCR_TEMPLATE_PPAPKTNOTPCIE)
		| BCM_SBIT(HWA_PAGER_PP_DMA_DESCR_TEMPLATE_PPAPKTCOHERENT)
		| BCM_SBIT(HWA_PAGER_PP_DMA_DESCR_TEMPLATE_PPRXAPKTNOTPCIE)
		| BCM_SBIT(HWA_PAGER_PP_DMA_DESCR_TEMPLATE_PPRXAPKTCOHERENT)

		| BCM_SBIT(HWA_PAGER_PP_DMA_DESCR_TEMPLATE_PPPUSHSANOTPCIE)
		| BCM_SBIT(HWA_PAGER_PP_DMA_DESCR_TEMPLATE_PPPUSHSACOHERENT)
		// | BCM_SBIT(HWA_PAGER_PP_DMA_DESCR_TEMPLATE_PPPUSHDANOTPCIE)
		// | BCM_SBIT(HWA_PAGER_PP_DMA_DESCR_TEMPLATE_PPPUSHDACOHERENT)
		// | BCM_SBIT(HWA_PAGER_PP_DMA_DESCR_TEMPLATE_PPPUSHDAWCPDESC)

		// | BCM_SBIT(HWA_PAGER_PP_DMA_DESCR_TEMPLATE_PPPULLSANOTPCIE)
		// | BCM_SBIT(HWA_PAGER_PP_DMA_DESCR_TEMPLATE_PPPULLSACOHERENT)
		| BCM_SBIT(HWA_PAGER_PP_DMA_DESCR_TEMPLATE_PPPULLDANOTPCIE)
		| BCM_SBIT(HWA_PAGER_PP_DMA_DESCR_TEMPLATE_PPPULLDACOHERENT)
		// | BCM_SBIT(HWA_PAGER_PP_DMA_DESCR_TEMPLATE_PPPULLDAWCPDESC)
		| 0U;
	HWA_WR_REG_NAME(HWApp, regs, pager, pp_dma_descr_template, v32);

	v32 = 0U;
	HWA_WR_REG_NAME(HWApp, regs, common, dmatxsel, v32);
	HWA_WR_REG_NAME(HWApp, regs, common, dmarxsel, v32);

	v32 = HWA_RD_REG_NAME(HWApp, regs, txdma, pp_pagein_cfg);
	v32 = v32
	    | BCM_SBIT(HWA_TXDMA_PP_PAGEIN_CFG_PP_PAGEIN_RDMA_COHERENT)
	    | BCM_SBIT(HWA_TXDMA_PP_PAGEIN_CFG_PP_PAGEIN_RDMA_NOTPCIE)
	    | BCM_SBIT(HWA_TXDMA_PP_PAGEIN_CFG_PP_PAGEIN_WDMA_WCPDESC)
	    | BCM_SBIT(HWA_TXDMA_PP_PAGEIN_CFG_PP_PAGEIN_WDMA_COHERENT)
	    | BCM_SBIT(HWA_TXDMA_PP_PAGEIN_CFG_PP_PAGEIN_WDMA_NOTPCIE)
	    | 0U;
	HWA_WR_REG_NAME(HWApp, regs, txdma, pp_pagein_cfg, v32);

	v32 = HWA_RD_REG_NAME(HWApp, regs, txdma, pp_pageout_cfg);
	v32 = v32
	    | BCM_SBIT(HWA_TXDMA_PP_PAGEOUT_CFG_PP_PAGEOUT_RDMA_COHERENT)
	    | BCM_SBIT(HWA_TXDMA_PP_PAGEOUT_CFG_PP_PAGEOUT_RDMA_NOTPCIE)
	    | BCM_SBIT(HWA_TXDMA_PP_PAGEOUT_CFG_PP_PAGEOUT_WDMA_WCPDESC)
	    | BCM_SBIT(HWA_TXDMA_PP_PAGEOUT_CFG_PP_PAGEOUT_WDMA_COHERENT)
	    | BCM_SBIT(HWA_TXDMA_PP_PAGEOUT_CFG_PP_PAGEOUT_WDMA_NOTPCIE)
	    | 0U;
	HWA_WR_REG_NAME(HWApp, regs, txdma, pp_pageout_cfg, v32);

	// Configure Dongle Packet Pool
	v32 = HWA_PTR2UINT(pktpgr->dnglpktpool_mem);
	HWA_WR_REG_ADDR(HWApp, &pager_regs->dnglpktpool.addr.lo, v32);
	HWA_WR_REG_ADDR(HWApp, &pager_regs->dnglpktpool.addr.hi, 0U);
	HWA_WR_REG_ADDR(HWApp, &pager_regs->dnglpktpool.size,
		pktpgr->dnglpktpool_max);
	v32 = (0U
		| BCM_SBF(HWA_PKTPGR_DNGLPKTPOOL_TH1,
		          HWA_PAGER_PP_DNGLPKTPOOL_INTR_TH_PPDDTH1)
		| BCM_SBF(HWA_PKTPGR_DNGLPKTPOOL_TH2,
		          HWA_PAGER_PP_DNGLPKTPOOL_INTR_TH_PPDDTH2));
	HWA_WR_REG_ADDR(HWApp, &pager_regs->dnglpktpool.intr_th, v32);
	v32 = BCM_SBIT(HWA_PAGER_PP_DNGLPKTPOOL_CTRL_PPDDENABLE);
	HWA_WR_REG_ADDR(HWApp, &pager_regs->dnglpktpool.ctrl, v32);

	// Host Packet Pool configured in hwa_config - after dhd::dongle handshake

	// Attach all Packet Pager Interfaces
	ring = 0;

	ring_memory = memory[ring++]; // pagein_req_ring + PPInRxProcessBE
	HWA_TRACE(("%s pagein_req +memory[%p,%u]\n", HWApp, ring_memory, mem_sz));
	pktpgr->req_ring[hwa_pktpgr_pagein_req_ring] =
		hwa_pktpgr_ring_init(&pktpgr->pagein_req_ring, "PI>",
			HWA_RING_S2H, HWA_PKTPGR_PAGEIN_S2H_RINGNUM,
			depth, ring_memory, regs, &pager_regs->pagein_req_ring);

	ring_memory = memory[ring++]; // pagein_rsp_ring
	HWA_TRACE(("%s pagein_rsp +memory[%p,%u]\n", HWApp, ring_memory, mem_sz));
	pktpgr->rsp_ring[hwa_pktpgr_pagein_rsp_ring] =
		hwa_pktpgr_ring_init(&pktpgr->pagein_rsp_ring, "PI<",
			HWA_RING_H2S, HWA_PKTPGR_PAGEIN_H2S_RINGNUM,
			depth, ring_memory, regs, &pager_regs->pagein_rsp_ring);

	ring_memory = memory[ring++]; // pageout_req_ring
	HWA_TRACE(("%s pageout_req +memory[%p,%u]\n", HWApp, ring_memory, mem_sz));
	pktpgr->req_ring[hwa_pktpgr_pageout_req_ring] =
		hwa_pktpgr_ring_init(&pktpgr->pageout_req_ring, "PO>",
			HWA_RING_S2H, HWA_PKTPGR_PAGEOUT_S2H_RINGNUM,
			depth, ring_memory, regs, &pager_regs->pageout_req_ring);

	ring_memory = memory[ring++]; // pageout_rsp_ring
	HWA_TRACE(("%s pageout_rsp +memory[%p,%u]\n", HWApp, ring_memory, mem_sz));
	pktpgr->rsp_ring[hwa_pktpgr_pageout_rsp_ring] =
		hwa_pktpgr_ring_init(&pktpgr->pageout_rsp_ring, "PO<",
			HWA_RING_H2S, HWA_PKTPGR_PAGEOUT_H2S_RINGNUM,
			depth, ring_memory, regs, &pager_regs->pageout_rsp_ring);

	ring_memory = memory[ring++]; // pagemgr_req_ring
	HWA_TRACE(("%s pagemgr_req +memory[%p,%u]\n", HWApp, ring_memory, mem_sz));
	pktpgr->req_ring[hwa_pktpgr_pagemgr_req_ring] =
		hwa_pktpgr_ring_init(&pktpgr->pagemgr_req_ring, "PM>",
			HWA_RING_S2H, HWA_PKTPGR_PAGEMGR_S2H_RINGNUM,
			depth, ring_memory, regs, &pager_regs->pagemgr_req_ring);

	ring_memory = memory[ring++]; // pagemgr_rsp_ring
	HWA_TRACE(("%s pagemgr_rsp +memory[%p,%u]\n", HWApp, ring_memory, mem_sz));
	pktpgr->rsp_ring[hwa_pktpgr_pagemgr_rsp_ring] =
		hwa_pktpgr_ring_init(&pktpgr->pagemgr_rsp_ring, "PM<",
			HWA_RING_H2S, HWA_PKTPGR_PAGEMGR_H2S_RINGNUM,
			depth, ring_memory, regs, &pager_regs->pagemgr_rsp_ring);

	ring_memory = memory[ring++]; // freepkt_req_ring
	HWA_TRACE(("%s freepkt_req +memory[%p,%u]\n", HWApp, ring_memory, mem_sz));
	pktpgr->req_ring[hwa_pktpgr_freepkt_req_ring] =
		hwa_pktpgr_ring_init(&pktpgr->freepkt_req_ring, "PKT",
			HWA_RING_S2H, HWA_PKTPGR_FREEPKT_S2H_RINGNUM,
			depth, ring_memory, regs, &pager_regs->freepkt_req_ring);

	ring_memory = memory[ring++]; // freerph_req_ring
	HWA_TRACE(("%s freerph_req +memory[%p,%u]\n", HWApp, ring_memory, mem_sz));
	pktpgr->req_ring[hwa_pktpgr_freerph_req_ring] =
		hwa_pktpgr_ring_init(&pktpgr->freerph_req_ring, "RPH",
			HWA_RING_S2H, HWA_PKTPGR_FREERPH_S2H_RINGNUM,
			depth, ring_memory, regs, &pager_regs->freerph_req_ring);

	HWA_ASSERT(ring == hwa_pktpgr_ring_max); // done all rings

	// Register process item callback for the PKTPGR H2S interfaces.
	hwa_register(dev, HWA_PKTPGR_PAGEIN_RXPROCESS, dev,
	             (hwa_callback_fn_t) hwa_pktpgr_pagein_rxprocess);
	hwa_register(dev, HWA_PKTPGR_PAGEIN_TXSTATUS, dev,
	             (hwa_callback_fn_t) hwa_pktpgr_pagein_txstatus);
	hwa_register(dev, HWA_PKTPGR_PAGEIN_TXPOST, dev,
	             (hwa_callback_fn_t) hwa_pktpgr_pagein_txpost);
	hwa_register(dev, HWA_PKTPGR_PAGEIN_TXPOST_FRC, dev,
	             (hwa_callback_fn_t) hwa_pktpgr_pagein_txpost);
	hwa_register(dev, HWA_PKTPGR_PAGEOUT_PKTLIST, dev,
	             (hwa_callback_fn_t) hwa_pktpgr_pageout_pktlist);
	hwa_register(dev, HWA_PKTPGR_PAGEOUT_LOCAL, dev,
	             (hwa_callback_fn_t) hwa_pktpgr_pageout_local);
	hwa_register(dev, HWA_PKTPGR_PAGEMGR_ALLOC_RX, dev,
	             (hwa_callback_fn_t) hwa_pktpgr_pagemgr_alloc_rx);
	hwa_register(dev, HWA_PKTPGR_PAGEMGR_ALLOC_RPH, dev,
	             (hwa_callback_fn_t) hwa_pktpgr_pagemgr_alloc_rph);
	hwa_register(dev, HWA_PKTPGR_PAGEMGR_ALLOC_TX, dev,
	             (hwa_callback_fn_t) hwa_pktpgr_pagemgr_alloc_tx);
	hwa_register(dev, HWA_PKTPGR_PAGEMGR_PUSH, dev,
	             (hwa_callback_fn_t) hwa_pktpgr_pagemgr_push);
	hwa_register(dev, HWA_PKTPGR_PAGEMGR_PULL, dev,
	             (hwa_callback_fn_t) hwa_pktpgr_pagemgr_pull);

	return pktpgr;

ring_failure:
	for (ring = 0; ring < hwa_pktpgr_ring_max; ++ring) {
		if (memory[ring] == NULL)
			break;
		MFREE(dev->osh, memory[ring], mem_sz);
	}

lbuf_pool_failure:
	hwa_pktpgr_detach(pktpgr);
	HWA_WARN(("%s attach failure\n", HWApp));

	return (hwa_pktpgr_t*) NULL;

}   // hwa_pktpgr_attach()

void // HWApp: Cleanup/Free resources used by PktPgr block
BCMATTACHFN(hwa_pktpgr_detach)(hwa_pktpgr_t *pktpgr)
{
	hwa_dev_t *dev;

	HWA_FTRACE(HWApp);

	if (pktpgr == (hwa_pktpgr_t*)NULL)
		return;

	// Audit pre-conditions
	dev = HWA_DEV(pktpgr);

	if (pktpgr->dnglpktpool_mem != (hwa_pp_lbuf_t*)NULL) {
		void * memory = (void*)pktpgr->dnglpktpool_mem;
		uint32 mem_sz = pktpgr->dnglpktpool_max * HWA_PP_LBUF_SZ;
		HWA_TRACE(("%s lbuf pool items<%u> mem_size<%u> free\n",
			HWApp, HWA_DNGL_PKTS_MAX, mem_sz));
		MFREE(dev->osh, memory, mem_sz);
		pktpgr->dnglpktpool_mem = (hwa_pp_lbuf_t*)NULL;
		pktpgr->dnglpktpool_max = 0;
	}

}   // hwa_pktpgr_detach()

void // HWApp: Init PktPgr block AFTER DHD handshake pcie_ipc initialized.
hwa_pktpgr_preinit(hwa_pktpgr_t *pktpgr)
{
	uint32 v32;
	hwa_dev_t *dev;
	hwa_pager_regs_t *pager_regs;

	HWA_FTRACE(HWApp);

	// Audit pre-conditions
	dev = HWA_DEV(pktpgr);
	HWA_ASSERT(dev->pcie_ipc != (pcie_ipc_t*)NULL);
	HWA_ASSERT(!HADDR64_IS_ZERO(dev->pcie_ipc->host_mem_haddr64));
	// XXX FIXME Fabian ... need a scheme to have multiple users of host_mem
	HWA_ASSERT(dev->pcie_ipc->host_mem_len >=
		(HWA_HOST_PKTS_MAX * HWA_PP_LBUF_SZ));

	pager_regs = &dev->regs->pager; // Setup locals

	// Configure Host Packet Pool
	HADDR64_SET(pktpgr->hostpktpool_haddr64, dev->pcie_ipc->host_mem_haddr64);
	HADD64_LTOH(pktpgr->hostpktpool_haddr64);
	pktpgr->hostpktpool_max =
		ltoh16(dev->pcie_ipc->host_mem_len) / HWA_PP_LBUF_SZ;

	v32 = HADDR64_LO(pktpgr->hostpktpool_haddr64);
	HWA_WR_REG_ADDR(HWApp, &pager_regs->hostpktpool.addr.lo, v32);
	v32 = HADDR64_HI(pktpgr->hostpktpool_haddr64);
	HWA_WR_REG_ADDR(HWApp, &pager_regs->hostpktpool.addr.hi, v32);
	HWA_WR_REG_ADDR(HWApp, &pager_regs->hostpktpool.size,
		pktpgr->hostpktpool_max);
	v32 = (0U
		| BCM_SBF(HWA_PKTPGR_HOSTPKTPOOL_TH1,
		          HWA_PAGER_PP_HOSTPKTPOOL_INTR_TH_PPHDTH1)
		| BCM_SBF(HWA_PKTPGR_HOSTPKTPOOL_TH2,
		          HWA_PAGER_PP_HOSTPKTPOOL_INTR_TH_PPHDTH2));
	HWA_WR_REG_ADDR(HWApp, &pager_regs->hostpktpool.intr_th, v32);
	v32 = BCM_SBIT(HWA_PAGER_PP_HOSTPKTPOOL_CTRL_PPHDENABLE);
	HWA_WR_REG_ADDR(HWApp, &pager_regs->hostpktpool.ctrl, v32);

	// Enable Packet Pager
	v32 = BCM_SBIT(HWA_PAGER_PP_PAGER_CFG_PAGER_EN);
	HWA_WR_REG_NAME(HWApp, dev->regs, pager, pp_pager_cfg, v32);

}   // hwa_pktpgr_preinit()

void
hwa_pktpgr_init(hwa_pktpgr_t *pktpgr)
{
}   // hwa_pktpgr_init()

void // HWApp: Deinit PktPgr block
hwa_pktpgr_deinit(hwa_pktpgr_t *pktpgr)
{
}   // hwa_pktpgr_deinit()

/**
 * Single debug interface to read all registers carrying packet pager "status"
 * Uses a wrapper _hwa_pktpgr_ring_status() to dump S2H and H2S interface.
 */
static void _hwa_pktpgr_ring_status(const char *ring_name,
                                    hwa_pp_ring_t *hwa_pp_ring_regs);
static void
_hwa_pktpgr_ring_status(const char *ring_name, hwa_pp_ring_t *hwa_pp_ring_regs)
{
	uint32 v32, wr_index, rd_index;
	wr_index = HWA_RD_REG_ADDR(HWApp, &hwa_pp_ring_regs->wr_index);
	rd_index = HWA_RD_REG_ADDR(HWApp, &hwa_pp_ring_regs->rd_index);
	if (wr_index != rd_index)
		HWA_PRINT("\t %s [wr,rd] = [%u, %u]\n", ring_name, wr_index, rd_index);
	v32 = HWA_RD_REG_ADDR(HWApp, &hwa_pp_ring_regs->debug);
	if (v32) HWA_PRINT("\t %s debug<0x%08x>\n", ring_name, v32);

}   // _hwa_pktpgr_ring_status()

void // HWApp: Query various interfaces and module for status and errors
hwa_pktpgr_status(hwa_dev_t *dev)
{
	uint32 v32;
	hwa_regs_t *regs;
	hwa_pager_regs_t *pager_regs;

	HWA_AUDIT_DEV(dev);
	regs = dev->regs;
	pager_regs = &regs->pager;

	// Ring debug status or processing stalled
	HWA_PRINT("%s Ring Status\n", HWApp);
	_hwa_pktpgr_ring_status("pagein_req ", &pager_regs->pagein_req_ring);
	_hwa_pktpgr_ring_status("pagein_rsp ", &pager_regs->pagein_rsp_ring);
	_hwa_pktpgr_ring_status("pageout_req", &pager_regs->pageout_req_ring);
	_hwa_pktpgr_ring_status("pageout_rsp", &pager_regs->pageout_rsp_ring);
	_hwa_pktpgr_ring_status("pagemgr_req", &pager_regs->pagemgr_req_ring);
	_hwa_pktpgr_ring_status("pagemgr_rsp", &pager_regs->pagemgr_rsp_ring);
	_hwa_pktpgr_ring_status("freepkt_req", &pager_regs->freepkt_req_ring);
	_hwa_pktpgr_ring_status("freerph_req", &pager_regs->freerph_req_ring);

	// Errors reported via Instatus
	HWA_PRINT("%s Ring Errors\n", HWApp);
	v32 = HWA_RD_REG_ADDR(HWApp, &pager_regs->pagein_int.status)
	    & HWA_PAGER_PAGEIN_ERRORS_MASK;
	if (v32) HWA_PRINT("\t pagein_errors<0x%08x>\n", v32);
	v32 = HWA_RD_REG_ADDR(HWApp, &pager_regs->pageout_int.status)
	    & HWA_PAGER_PAGEOUT_ERRORS_MASK;
	if (v32) HWA_PRINT("\t pageout_errors<0x%08x>\n", v32);
	v32 = HWA_RD_REG_ADDR(HWApp, &pager_regs->pagemgr_int.status)
	    & HWA_PAGER_PAGEMGR_ERRORS_MASK;
	if (v32) HWA_PRINT("\t pagemgr_errors<0x%08x>\n", v32);
	v32 = HWA_RD_REG_ADDR(HWApp, &pager_regs->pagerbm_int.status)
		& HWA_PAGER_PAGERBM_ERRORS_MASK;
	if (v32) HWA_PRINT("\t pagerbm_errors<0x%08x>\n", v32);

	HWA_PRINT("%s Transaction Id\n", HWApp);
	v32 = HWA_RD_REG_NAME(HWApp, regs, pager, rx_alloc_transaction_id);
	HWA_PRINT("\t rx_alloc<0x%08x>\n", v32);
	v32 = HWA_RD_REG_NAME(HWApp, regs, pager, rx_free_transaction_id);
	HWA_PRINT("\t rx_free<0x%08x>\n", v32);
	v32 = HWA_RD_REG_NAME(HWApp, regs, pager, tx_alloc_transaction_id);
	HWA_PRINT("\t tx_alloc<0x%08x>\n", v32);
	v32 = HWA_RD_REG_NAME(HWApp, regs, pager, tx_free_transaction_id);
	HWA_PRINT("\t tx_free<0x%08x>\n", v32);

	HWA_PRINT("%s Module Debug\n", HWApp);
	v32 = HWA_RD_REG_NAME(HWApp, regs, pager, pp_apkt_sts_dbg);
	if (v32) HWA_PRINT("\t alloc<0x%08x>\n", v32);
	v32 = HWA_RD_REG_NAME(HWApp, regs, pager, pp_rx_apkt_sts_dbg);
	if (v32) HWA_PRINT("\t rx_alloc<0x%08x>\n", v32);
	v32 = HWA_RD_REG_NAME(HWApp, regs, pager, pp_fpkt_sts_dbg);
	if (v32) HWA_PRINT("\t free<0x%08x>\n", v32);
	v32 = HWA_RD_REG_NAME(HWApp, regs, pager, pp_tb_sts_dbg);
	if (v32) HWA_PRINT("\t table<0x%08x>\n", v32);
	v32 = HWA_RD_REG_NAME(HWApp, regs, pager, pp_push_sts_dbg);
	if (v32) HWA_PRINT("\t push<0x%08x>\n", v32);
	v32 = HWA_RD_REG_NAME(HWApp, regs, pager, pp_pull_sts_dbg);
	if (v32) HWA_PRINT("\t pull<0x%08x>\n", v32);

	HWA_TXFIFO_EXPR({
		HWA_PRINT("%s Tx Status\n", HWApp);
		v32 = HWA_RD_REG_NAME(HWApp, regs, txdma, pp_pageout_sts);
		if (v32) HWA_PRINT("\t pp_pageout_sts<0x%08x>\n", v32);
		v32 = HWA_RD_REG_NAME(HWApp, regs, txdma, pp_pagein_sts);
		if (v32) HWA_PRINT("\t pp_pagein_sts<0x%08x>\n", v32);
	});

	HWA_RXFILL_EXPR({
		HWA_PRINT("%s Rx Status\n", HWApp);
		v32 = HWA_RD_REG_NAME(HWApp, regs, rx_core[0], pagein_status);
		if (v32) HWA_PRINT("\t pagein_status<0x%08x>\n", v32);
		v32 = HWA_RD_REG_NAME(HWApp, regs, rx_core[0], recycle_status);
		if (v32) HWA_PRINT("\t recycle_status<0x%08x>\n", v32);
	})

}   // hwa_pktpgr_status()

// HWA Packet Pager Debug Support
#if defined(BCMDBG) || defined(HWA_DUMP)
void // Dump all SW and HWA Packet Pager state
hwa_pktpgr_dump(hwa_pktpgr_t *pktpgr, struct bcmstrbuf *b,
                bool verbose, bool dump_regs)
{
	HWA_BPRINT(b, "pktpgr dump\n");

#if defined(WLTEST) || defined(HWA_DUMP)
	if (dump_regs)
		hwa_pktpgr_regs_dump(pktpgr, b);
#endif // endif
}   // hwa_pktpgr_dump()

#if defined(WLTEST) || defined(HWA_DUMP)
// Dump HWA Packet Pager registers
void
hwa_pktpgr_regs_dump(hwa_pktpgr_t *pktpgr, struct bcmstrbuf *b)
{
	hwa_dev_t *dev;
	hwa_regs_t *regs;

	if (pktpgr == (hwa_pktpgr_t*)NULL)
		return;

	dev = HWA_DEV(pktpgr);
	regs = dev->regs;

#define HWA_BPR_PP_RING(b, SNAME) \
	({ \
		HWA_BPR_REG(b, pager, SNAME.addr); \
		HWA_BPR_REG(b, pager, SNAME.wr_index); \
		HWA_BPR_REG(b, pager, SNAME.rd_index); \
		HWA_BPR_REG(b, pager, SNAME.cfg); \
		HWA_BPR_REG(b, pager, SNAME.lazyint_cfg); \
		HWA_BPR_REG(b, pager, SNAME.debug); \
	})

#define HWA_BPR_PP_INT(b, SNAME) \
	({ \
		HWA_BPR_REG(b, pager, SNAME.status); \
		HWA_BPR_REG(b, pager, SNAME.mask); \
	})

#define HWA_BPR_PP_PKTPOOL(b, SNAME) \
	({ \
		HWA_BPR_REG(b, pager, SNAME.addr.lo); \
		HWA_BPR_REG(b, pager, SNAME.addr.hi); \
		HWA_BPR_REG(b, pager, SNAME.ctrl); \
		HWA_BPR_REG(b, pager, SNAME.size); \
		HWA_BPR_REG(b, pager, SNAME.intr_th); \
		HWA_BPR_REG(b, pager, SNAME.alloc_index); \
		HWA_BPR_REG(b, pager, SNAME.dealloc_index); \
		HWA_BPR_REG(b, pager, SNAME.dealloc_status); \
	})

	HWA_BPRINT(b, "%s registers[%p] offset[0x%04x]\n",
		HWApp, &regs->pager, (uint32)OFFSETOF(hwa_regs_t, pager));
	HWA_BPR_REG(b, pager, pp_pager_cfg);
	HWA_BPR_REG(b, pager, pp_pktctx_size);
	HWA_BPR_REG(b, pager, pp_pktbuf_size);
	HWA_BPR_PP_RING(b,    pagein_req_ring);
	HWA_BPR_PP_RING(b,    pagein_rsp_ring);
	HWA_BPR_PP_INT(b,     pagein_int);
	HWA_BPR_PP_RING(b,    pageout_req_ring);
	HWA_BPR_PP_RING(b,    pageout_rsp_ring);
	HWA_BPR_PP_RING(b,    pagemgr_req_ring);
	HWA_BPR_PP_RING(b,    pagemgr_rsp_ring);
	HWA_BPR_PP_INT(b,     pagemgr_int);
	HWA_BPR_PP_RING(b,    freepkt_req_ring);
	HWA_BPR_PP_RING(b,    freerph_req_ring);
	HWA_BPR_REG(b, pager, rx_alloc_transaction_id);
	HWA_BPR_REG(b, pager, rx_free_transaction_id);
	HWA_BPR_REG(b, pager, tx_alloc_transaction_id);
	HWA_BPR_REG(b, pager, tx_free_transaction_id);
	HWA_BPR_PP_PKTPOOL(b, hostpktpool);
	HWA_BPR_PP_PKTPOOL(b, dnglpktpool);
	HWA_BPR_PP_INT(b,     pagerbm_int);
	HWA_BPR_REG(b, pager, pp_dma_descr_template);
	HWA_BPR_REG(b, pager, pp_apkt_cfg);
	HWA_BPR_REG(b, pager, pp_rx_apkt_cfg);
	HWA_BPR_REG(b, pager, pp_fpkt_cfg);
	HWA_BPR_REG(b, pager, pp_phpl_cfg);
	HWA_BPR_REG(b, pager, pp_apkt_sts_dbg);
	HWA_BPR_REG(b, pager, pp_rx_apkt_sts_dbg);
	HWA_BPR_REG(b, pager, pp_fpkt_sts_dbg);
	HWA_BPR_REG(b, pager, pp_tb_sts_dbg);
	HWA_BPR_REG(b, pager, pp_push_sts_dbg);
	HWA_BPR_REG(b, pager, pp_pull_sts_dbg);

}   // hwa_pktpgr_regs_dump()

#endif // endif
#endif /* BCMDBG */

#endif /* HWA_PKTPGR_BUILD */
