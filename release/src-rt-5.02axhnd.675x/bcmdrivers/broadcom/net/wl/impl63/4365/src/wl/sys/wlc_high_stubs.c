/**
 * @file
 * @brief
 * Stub functions used for dispatching RPC call
 * Broadcom 802.11bang Networking Device Driver
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
 * $Id: wlc_high_stubs.c 708017 2017-06-29 14:11:45Z $
 */

/**
 * @file
 * @brief
 * Used for BMAC driver model. This file is used in firmware builds, not in host driver builds.
 */

/**
 * @file
 * @brief
 * XXX Twiki: [WlBmacRpcModule]
 */

/* XXX: Define wlc_cfg.h to be the first header file included as some builds
 * get their feature flags thru this file.
 */
#include <wlc_cfg.h>

#ifdef WLC_HIGH
#error "This file should not be included for WLC_HIGH"
#endif // endif
#ifndef WLC_LOW
#error "This file needs WLC_LOW"
#endif // endif

#include <typedefs.h>
#include <bcmdefs.h>
#include <bcmendian.h>
#include <osl.h>
#include <bcmutils.h>
#include <proto/802.11.h>
#include <bcmwifi_channels.h>
#include <proto/802.1d.h>
#include <proto/802.11.h>
#include <wlioctl.h>
#include <proto/bcmevent.h>
#include <sbhnddma.h>
#include <sbhndpio.h>
#include <d11.h>
#include <wlc_channel.h>
#include <siutils.h>
#include <hnddma.h>
#include <wlc_rate.h>
#include <wlc_pub.h>
#include <wlc_pio.h>
#include <wlc.h>
#include <wlc_hw_priv.h>
#include <wlc_stf.h>
#include <wlc_bmac.h>
#include <bcmsrom_fmt.h>
#include <bcm_xdr.h>
#include <bcm_rpc_tp.h>
#include <bcm_rpc.h>
#include <wlc_rpc.h>
#include <wlc_phy_hal.h>
#include <phy_radar_api.h>
#include <phy_antdiv_api.h>
#include <phy_utils_api.h>
#include <phy_rxgcrs_api.h>
#include <wlc_tx.h>
#ifdef AP
#include <wlc_apps.h>
#endif // endif
#include <wlc_extlog.h>
#ifdef WLMCNX
#include <wlc_mcnx.h>
#endif // endif
#ifdef WL_WOWL_MEDIA
#include <wl_export.h>
#endif // endif

#if defined(BCMDBG) || defined(BCMDBG_ERR)
const struct rpc_name_entry rpc_name_tbl[] = RPC_ID_TABLE;
#endif /* BCMDBG || BCMDBG_ERR */

/* Maximum bytes returned in response to WLRPC_WLC_BMAC_COPYFROM_VARS_ID */
#define MAX_VARS_BUFLEN 1900
#define HIST_SIZE 12
#define RPC_TP_ZLP_PAD 8
static uint32 histogram_txfifo[HIST_SIZE] = { 0 };

/* ============= BMAC->HOST =================== */

static INLINE int
wlc_rpc_call_return(rpc_info_t *rpc, rpc_buf_t *send)
{
	int err;

	ASSERT(send);
#if defined(BCMDBG) || defined(BCMDBG_ERR)
	wlc_rpc_id_t rpc_id = NULL;
	rpc_id = wlc_rpc_id_get(rpc, send);

	WL_TRACE(("%s: Returned Call id %s\n", __FUNCTION__,
	          WLC_RPC_ID_LOOKUP(rpc_name_tbl, rpc_id)));
#endif /* BCMDBG || BCMDBG_ERR */
	err =  bcm_rpc_call_return(rpc, send);

	if (err && send)
		bcm_rpc_buf_free(rpc, send);
	return err;
}

/* ============= HOST->BMAC =================== */

static void
wlc_rpc_dngl_reboot(wlc_hw_info_t *wlc_hw)
{
#if defined(USB4319)
#ifndef BCMUSB_NODISCONNECT
	si_watchdog_ms(wlc_hw->sih, 20);
#endif // endif
#else
	si_watchdog(wlc_hw->sih, 1);
#endif /* USB4319 */
}

void
wlc_rpc_bmac_dump_txfifohist(wlc_hw_info_t *wlc_hw, bool dump_clear)
{
	int i = 0;

	if (!dump_clear) {
		bzero(histogram_txfifo, HIST_SIZE * sizeof(uint32));
		return;
	}

	for (; i < HIST_SIZE; i++) {
		if (histogram_txfifo[i]) {
			printf("%d: %d ", i, histogram_txfifo[i]);
		}
	}
	printf("\n");
}

static void *
wlc_rpc_bmac_txfifo_buf_to_txpkt(rpc_info_t *rpc, osl_t *osh, bcm_xdr_buf_t *b,
                    rpc_buf_t *buf, uint totlen)
{
	struct lbuf *p;

#ifndef RPCPKTCOPY
	int resid;
	int pkt_cnt;
	void *iter_p;
	void *next_p;

	/* Here the assumption directly is rpc_buf IS lbuf. This could be perhaps fixed,
	 * but essentially based on what's already consumed, as can be inferred from
	 * bcm_xdr pointer, this will just fix up the relevent fields of lbuf
	 */

	p = (struct lbuf *)buf;

	WL_NONE(("PKTCONVERT: totlen %d p %p len %d data %p head %p end %p\n",
	         totlen, p, PKTLEN(osh, p), PKTDATA(osh, p), lb_head(p), lb_end(p)));

	WL_NONE(("PKTCONVERT: b %p b.buf %p b.origbuf %p b.size %d b.origsize %d\n",
	         b, b->buf, b->origbuf, b->size, b->origsize));

	/* Push the data pointer to match what's already consumed */
	PKTPULL(osh, p, b->buf - b->origbuf);

	iter_p = p;
	resid = totlen;

	/* Walk the packet chain until iter_p is the last packet in the chain.
	 * Calc the residual byte count as we step past packets.
	 */
	pkt_cnt = 1;
	while ((next_p = PKTNEXT(osh, iter_p)) != NULL) {
		if (resid < PKTLEN(osh, iter_p)) {
			WL_NONE(("resid = %d pktlen = %d next pktlen=%d\n", resid,
				PKTLEN(osh, iter_p), PKTLEN(osh, next_p)));
			/* handle the case of lbuf size is n * 512 and
			 * ZLP 8 bytes are appended as next pkt
			 */
			if (PKTLEN(osh, next_p) == RPC_TP_ZLP_PAD) {
				PKTSETNEXT(osh, iter_p, NULL);
				ASSERT(PKTNEXT(osh, next_p) == NULL);
				PKTFREE(osh, next_p, FALSE);
				pkt_cnt++;
				break;
			}
		}
		ASSERT(resid >= PKTLEN(osh, iter_p));
		resid -= PKTLEN(osh, iter_p);
		iter_p = next_p;
		pkt_cnt++;

		WL_NONE(("PKTCONVERT: resid %d p %p len %d data %p\n",
		         resid, p, PKTLEN(osh, p), PKTDATA(osh, p)));
	}
	/* If the final packet is longer than the residual bytes, trim the length(padding) */
	if (resid < PKTLEN(osh, iter_p)) {
		WL_NONE(("PKTCONVERT: trim to resid %d p %p len %d data %p\n",
		         resid, p, PKTLEN(osh, p), PKTDATA(osh, p)));
		/* Warning when residual bytes is >=6, 2 for WLC_BMAC_F_PAD2 at the front of
		 * packet, 3 for alignment padding at the end of packet.
		 */
		PKTSETLEN(osh, iter_p, resid);
	}

	/* Mark the buffer consumed */
	b->size = 0;

	/* adjust RPC TP buffer accounting since we are stealing the buffers */
	bcm_rpc_tp_buf_cnt_adjust(bcm_rpc_tp_get(rpc), -(pkt_cnt));

#else	/* old PKT copy version */

	int err;
	p = PKTGET(osh, totlen, FALSE);

	if (p == NULL)
		goto err;

	err = bcm_xdr_unpack_opaque_cpy(b, totlen, ((uint8*)PKTDATA(osh, p)));
	ASSERT(!err);

err:
	bcm_rpc_buf_free(rpc, buf);

#endif /* RPCPKTCOPY */

	return (void *)p;
}

/* CLIENT dongle driver RPC dispatch routine, called by bcm_rpc_buf_recv()
 *  Based on request, push to common driver or send back result
 */
void
wlc_rpc_bmac_dispatch(wlc_rpc_ctx_t *rpc_ctx, struct rpc_buf* buf)
{
	bcm_xdr_buf_t b;
	wlc_rpc_id_t rpc_id;
	int err;
	rpc_info_t *rpc = rpc_ctx->rpc;
	rpc_tp_info_t *rpc_th = bcm_rpc_tp_get(rpc);
	wlc_info_t *wlc = rpc_ctx->wlc;
	wlc_hw_info_t *wlc_hw = rpc_ctx->wlc->hw;
	bool free_buf = TRUE;

	ASSERT(rpc);

	rpc_id = (wlc_rpc_id_t)ltoh32(*(uint32 *)bcm_rpc_buf_data(rpc_th, buf));

	if (bcm_rpc_buf_len_get(rpc_th, buf) > sizeof(uint32))
		bcm_rpc_buf_pull(rpc_th, buf, sizeof(uint32));
	else {
		/* if head packet ends at the rpc header, free, advance to next packet in chain */
		rpc_buf_t *next_p;

		ASSERT(bcm_rpc_buf_len_get(rpc_th, buf) == sizeof(uint32));

		next_p = (rpc_buf_t*)PKTNEXT(wlc_hw->osh, buf);

		PKTSETNEXT(wlc_hw->osh, buf, NULL);
		bcm_rpc_tp_buf_free(rpc_th, buf);
		buf = next_p;
	}

	if (!buf)
		bcm_xdr_buf_init(&b, NULL, 0);
	else
		bcm_xdr_buf_init(&b, bcm_rpc_buf_data(rpc_th, buf),
		                 bcm_rpc_buf_len_get(rpc_th, buf));

#if defined(BCMDBG) || defined(BCMDBG_ERR)
	WL_TRACE(("%s: Dispatch id %s\n", __FUNCTION__, WLC_RPC_ID_LOOKUP(rpc_name_tbl, rpc_id)));
#endif /* BCMDBG || BCMDBG_ERR */

	switch (rpc_id) {

	case WLRPC_WLC_REG_READ_ID: {
		uint32 r;
		uint size;
		uint32 ret;
		bcm_xdr_buf_t retb;
		rpc_buf_t *rpc_buf;

		err = bcm_xdr_unpack_uint32(&b, &r);
		ASSERT(!err);

		err = bcm_xdr_unpack_uint32(&b, &size);
		ASSERT(!err);

		ret = wlc_reg_read(wlc, (void*)r, size);

		rpc_buf = wlc_rpc_buf_alloc(rpc, &retb, sizeof(uint32), WLRPC_WLC_REG_READ_ID);
		ASSERT(rpc_buf != NULL);

		err = bcm_xdr_pack_uint32(&retb, ret);
		ASSERT(!err);

		err = wlc_rpc_call_return(rpc, rpc_buf);
		ASSERT(!err);

		break;
	}

	case WLRPC_WLC_REG_WRITE_ID: {
		uint32 r;
		uint32 v;
		uint size;

		err = bcm_xdr_unpack_uint32(&b, &r);
		ASSERT(!err);

		err = bcm_xdr_unpack_uint32(&b, &v);
		ASSERT(!err);

		err = bcm_xdr_unpack_uint32(&b, &size);
		ASSERT(!err);

		wlc_reg_write(wlc, (void*)r, v, size);

		break;
	}

	case WLRPC_WLC_BMAC_WRITE_HW_BCNTEMPLATES_ID: {
		void *bcn;
		uint len;
		uint both;

		err = bcm_xdr_unpack_uint32(&b, &both);
		ASSERT(!err);

		err = bcm_xdr_unpack_opaque_varlen(&b, &len, &bcn);
		ASSERT(!err);

		wlc_bmac_write_hw_bcntemplates(wlc_hw, bcn, len, both);

		break;
	}

	case WLRPC_WLC_MHF_SET_ID: {
		uint32 ret;
		uint8 idx;
		uint16 mask;
		uint16 val;
		int bands;

		err = bcm_xdr_unpack_uint32(&b, &ret);
		ASSERT(!err);

		idx = (uint8)ret;

		err = bcm_xdr_unpack_uint32(&b, &ret);
		ASSERT(!err);

		mask = (uint16)ret;

		err = bcm_xdr_unpack_uint32(&b, &ret);
		ASSERT(!err);

		val = (uint16)ret;

		err = bcm_xdr_unpack_uint32(&b, &ret);
		ASSERT(!err);

		bands = (int)ret;

		wlc_bmac_mhf(wlc_hw, idx, mask, val, bands);
		break;
	}

	case WLRPC_WLC_MHF_GET_ID: {
		uint32 ret;
		uint8 idx;
		int bands;
		bcm_xdr_buf_t retb;
		rpc_buf_t *rpc_buf;

		err = bcm_xdr_unpack_uint32(&b, &ret);
		ASSERT(!err);
		idx = (uint8)ret;

		err = bcm_xdr_unpack_uint32(&b, &ret);
		ASSERT(!err);
		bands = (int)ret;

		ret = wlc_bmac_mhf_get(wlc_hw, idx, bands);

		rpc_buf = wlc_rpc_buf_alloc(rpc, &retb, sizeof(uint32), WLRPC_WLC_MHF_GET_ID);
		ASSERT(rpc_buf != NULL);

		err = bcm_xdr_pack_uint32(&retb, ret);
		ASSERT(!err);

		err = wlc_rpc_call_return(rpc, rpc_buf);
		ASSERT(!err);

		break;
	}

	case WLRPC_WLC_BMAC_RESET_ID:
		wlc_bmac_reset(wlc_hw);
		break;

	case WLRPC_WLC_DNGL_REBOOT_ID:
		wlc_rpc_dngl_reboot(wlc_hw);
		break;

	case WLRPC_WLC_BMAC_RPC_MSGLEVEL_SET_ID: {
		uint32 msglevel;
		err = bcm_xdr_unpack_uint32(&b, &msglevel);
		ASSERT(!err);

		bcm_rpc_msglevel_set(rpc, msglevel & 0xffff, FALSE);
		break;
	}

	case WLRPC_WLC_BMAC_RPC_TXQ_WM_SET_ID: {
		uint32 hiwm, lowm;
		err = bcm_xdr_unpack_uint32(&b, &hiwm);
		ASSERT(!err);

		err = bcm_xdr_unpack_uint32(&b, &lowm);
		ASSERT(!err);

		bcm_rpc_tp_txq_wm_set(rpc_th, (uint8)hiwm, (uint8)lowm);
		break;
	}

	case WLRPC_WLC_BMAC_RPC_TXQ_WM_GET_ID: {
		uint8 phi, plo;
		bcm_xdr_buf_t retb;
		rpc_buf_t *rpc_buf;

		rpc_buf = wlc_rpc_buf_alloc(rpc, &retb, sizeof(uint32) * 2,
		                            WLRPC_WLC_BMAC_RPC_TXQ_WM_GET_ID);
		ASSERT(rpc_buf != NULL);

		bcm_rpc_tp_txq_wm_get(rpc_th, &phi, &plo);

		err = bcm_xdr_pack_uint32(&retb, phi);
		ASSERT(!err);

		err = bcm_xdr_pack_uint32(&retb, plo);
		ASSERT(!err);

		err = wlc_rpc_call_return(rpc, rpc_buf);
		ASSERT(!err);
		break;
	}

	case WLRPC_WLC_BMAC_RPC_AGG_SET_ID: {
		uint32 aggbits, agg;
		err = bcm_xdr_unpack_uint32(&b, &aggbits);
		ASSERT(!err);

		agg = aggbits & BCM_RPC_TP_DNGL_AGG_MASK;

		if (wlc_hw->rpc_dngl_agg == agg)
			break;

		wlc_hw->rpc_dngl_agg = agg;

		if (wlc_hw->rpc_dngl_agg == 0) {
			/* if clear, close agg immediately */
			bcm_rpc_tp_agg_set(rpc_th, BCM_RPC_TP_DNGL_AGG_MASK, FALSE);
		} else {
			/* wait for each individual condition to trigger the agg */
		}
		break;
	}

	case WLRPC_WLC_BMAC_RPC_AGG_LIMIT_SET_ID: {
		uint32 sf, bytes;
		err = bcm_xdr_unpack_uint32(&b, &sf);
		ASSERT(!err);

		err = bcm_xdr_unpack_uint32(&b, &bytes);
		ASSERT(!err);

		bcm_rpc_tp_agg_limit_set(rpc_th, (uint8)sf, (uint16)bytes);
		break;
	}

	case WLRPC_WLC_BMAC_RPC_AGG_LIMIT_GET_ID: {
		uint8 psf;
		uint16 pbytes;
		bcm_xdr_buf_t retb;
		rpc_buf_t *rpc_buf;

		rpc_buf = wlc_rpc_buf_alloc(rpc, &retb, sizeof(uint32) * 2,
		                            WLRPC_WLC_BMAC_RPC_AGG_LIMIT_GET_ID);
		ASSERT(rpc_buf != NULL);

		bcm_rpc_tp_agg_limit_get(rpc_th, &psf, &pbytes);

		err = bcm_xdr_pack_uint32(&retb, psf);
		ASSERT(!err);

		err = bcm_xdr_pack_uint32(&retb, pbytes);
		ASSERT(!err);

		err = wlc_rpc_call_return(rpc, rpc_buf);
		ASSERT(!err);
		break;
	}

	case WLRPC_WLC_BMAC_HW_UP_ID: {
		wlc_bmac_hw_up(wlc_hw);
		break;
	}

	case WLRPC_WLC_BMAC_UP_PREP_ID: {
		bcm_xdr_buf_t retb;
		rpc_buf_t *rpc_buf;
		int ret;

		rpc_buf = wlc_rpc_buf_alloc(rpc, &retb, sizeof(uint32), WLRPC_WLC_BMAC_UP_PREP_ID);
		ASSERT(rpc_buf != NULL);

		ret = wlc_bmac_up_prep(wlc_hw);

		err = bcm_xdr_pack_int32(&retb, ret);
		ASSERT(!err);

		err = wlc_rpc_call_return(rpc, rpc_buf);
		ASSERT(!err);

		break;
	}

	case WLRPC_WLC_BMAC_UP_FINISH_ID: {
		bcm_xdr_buf_t retb;
		rpc_buf_t *rpc_buf;
		int ret;

		rpc_buf = wlc_rpc_buf_alloc(rpc, &retb, sizeof(uint32),
			WLRPC_WLC_BMAC_UP_FINISH_ID);
		ASSERT(rpc_buf != NULL);

		ret = wlc_bmac_up_finish(wlc_hw);

		err = bcm_xdr_pack_int32(&retb, ret);
		ASSERT(!err);

		err = wlc_rpc_call_return(rpc, rpc_buf);
		ASSERT(!err);

		break;
	}

	case WLRPC_WLC_BMAC_SET_CTRL_EPA_ID: {
		bcm_xdr_buf_t retb;
		rpc_buf_t *rpc_buf;
		int ret;

		rpc_buf = wlc_rpc_buf_alloc(rpc, &retb, sizeof(uint32),
		        WLRPC_WLC_BMAC_SET_CTRL_EPA_ID);
		ASSERT(rpc_buf != NULL);

		ret = wlc_bmac_set_ctrl_ePA(wlc_hw);

		err = bcm_xdr_pack_int32(&retb, ret);
		ASSERT(!err);

		err = wlc_rpc_call_return(rpc, rpc_buf);
		ASSERT(!err);

		break;
	}

	case WLRPC_WLC_BMAC_SET_CTRL_SROM_ID: {
		bcm_xdr_buf_t retb;
		rpc_buf_t *rpc_buf;
		int ret;

		rpc_buf = wlc_rpc_buf_alloc(rpc, &retb, sizeof(uint32),
			WLRPC_WLC_BMAC_SET_CTRL_SROM_ID);
		ASSERT(rpc_buf != NULL);

		ret = wlc_bmac_set_ctrl_SROM(wlc_hw);

		err = bcm_xdr_pack_int32(&retb, ret);
		ASSERT(!err);

		err = wlc_rpc_call_return(rpc, rpc_buf);
		ASSERT(!err);

		break;
	}

	case WLRPC_WLC_BMAC_SET_CTRL_BT_SHD0_ID: {
		bcm_xdr_buf_t retb;
		rpc_buf_t *rpc_buf;
		int ret;
		uint32 enable;

		err = bcm_xdr_unpack_uint32(&b, &enable);
		ASSERT(!err);

		rpc_buf = wlc_rpc_buf_alloc(rpc, &retb, sizeof(uint32),
			WLRPC_WLC_BMAC_SET_CTRL_BT_SHD0_ID);
		ASSERT(rpc_buf != NULL);

		ret = wlc_bmac_set_ctrl_bt_shd0(wlc_hw, (enable != 0));

		err = bcm_xdr_pack_int32(&retb, ret);
		ASSERT(!err);

		err = wlc_rpc_call_return(rpc, rpc_buf);
		ASSERT(!err);

		break;
	}

	case WLRPC_WLC_BMAC_DOWN_PREP_ID: {
		bcm_xdr_buf_t retb;
		rpc_buf_t *rpc_buf;
		int ret;

		rpc_buf = wlc_rpc_buf_alloc(rpc, &retb, sizeof(uint32),
		                            WLRPC_WLC_BMAC_DOWN_PREP_ID);
		ASSERT(rpc_buf != NULL);

		ret = wlc_bmac_down_prep(wlc_hw);

		err = bcm_xdr_pack_int32(&retb, ret);
		ASSERT(!err);

		err = wlc_rpc_call_return(rpc, rpc_buf);
		ASSERT(!err);

		break;
	}

	case WLRPC_WLC_BMAC_DOWN_FINISH_ID: {
		bcm_xdr_buf_t retb;
		rpc_buf_t *rpc_buf;
		int ret;

		rpc_buf = wlc_rpc_buf_alloc(rpc, &retb, sizeof(uint32),
			WLRPC_WLC_BMAC_DOWN_FINISH_ID);
		ASSERT(rpc_buf != NULL);

		ret = wlc_bmac_down_finish(wlc_hw);

		err = bcm_xdr_pack_int32(&retb, ret);
		ASSERT(!err);

		err = wlc_rpc_call_return(rpc, rpc_buf);
		ASSERT(!err);

		break;
	}

	case WLRPC_WLC_BMAC_INIT_ID: {
		uint32 chanspec, mute, defmacintmask;

		err = bcm_xdr_unpack_uint32(&b, &chanspec);
		ASSERT(!err);

		err = bcm_xdr_unpack_uint32(&b, &mute);
		ASSERT(!err);

		err = bcm_xdr_unpack_uint32(&b, &defmacintmask);
		ASSERT(!err);

		wlc_bmac_init(wlc_hw, (chanspec_t)chanspec, (bool)mute, defmacintmask);
		break;
	}

	case WLRPC_WLC_BMAC_SET_CWMIN_ID: {
		uint32 ret;
		uint16 newmin;

		err = bcm_xdr_unpack_uint32(&b, &ret);
		ASSERT(!err);

		newmin = (uint16)ret;

		wlc_bmac_set_cwmin(wlc_hw, newmin);
		break;
	}

	case WLRPC_WLC_BMAC_MUTE_ID: {
		uint32 ret;
		bool on;

		err = bcm_xdr_unpack_uint32(&b, &ret);
		ASSERT(!err);

		on = (bool)ret;

		wlc_bmac_mute(wlc_hw, on, 0);
		break;
	}

	case WLRPC_WLC_BMAC_SET_DEAF: {
		uint32 ret;
		bool user_flag;

		err = bcm_xdr_unpack_uint32(&b, &ret);
		ASSERT(!err);

		user_flag = (bool)ret;

		wlc_bmac_set_deaf(wlc_hw, user_flag);
		break;
	}

#if defined(WLTEST)
	case WLRPC_WLC_BMAC_CLEAR_DEAF: {
		uint32 ret;
		bool user_flag;

		err = bcm_xdr_unpack_uint32(&b, &ret);
		ASSERT(!err);

		user_flag = (bool)ret;

		wlc_bmac_clear_deaf(wlc_hw, user_flag);
		break;
	}
#endif // endif

	case WLRPC_WLC_BMAC_DISPATCH_IOV_ID: {
		uint32 actionid;
		void *p, *a;
		uint plen;
		int alen, vsize, ret;
		bcm_xdr_buf_t retb;
		rpc_buf_t *rpc_buf;
		uint32 tid;

		err = bcm_xdr_unpack_uint32(&b, &tid);
		ASSERT(!err);

		err = bcm_xdr_unpack_uint32(&b, &actionid);
		ASSERT(!err);

		/* unpack *p (no need to malloc) as iovar_dispatch input */
		err = bcm_xdr_unpack_uint32(&b, &plen);
		ASSERT(!err);
		err = bcm_xdr_unpack_opaque(&b, plen, &p);
		ASSERT(!err);

		/* malloc *a as iovar_dispatch return buffer */
		err = bcm_xdr_unpack_int32(&b, &alen);
		ASSERT(!err);

		a = MALLOC(wlc_hw->osh, ROUNDUP(alen, sizeof(uint32)));
		ASSERT(a);

		err = bcm_xdr_unpack_int32(&b, &vsize);
		ASSERT(!err);

		ret = wlc_bmac_dispatch_iov(wlc_hw, (uint16)tid, actionid, 0,
		                            p, plen, a, alen, vsize);

		rpc_buf = wlc_rpc_buf_alloc(rpc, &retb, sizeof(int32) +
			ROUNDUP(alen, sizeof(uint32)), WLRPC_WLC_BMAC_DISPATCH_IOV_ID);

		ASSERT(rpc_buf != NULL);

		err = bcm_xdr_pack_opaque(&retb, alen, a);
		ASSERT(!err);

		err = bcm_xdr_pack_int32(&retb, ret);
		ASSERT(!err);

		err = wlc_rpc_call_return(rpc, rpc_buf);
		ASSERT(!err);

		MFREE(wlc_hw->osh, a, ROUNDUP(alen, sizeof(uint32)));

		break;
	}

	case WLRPC_WLC_BMAC_DISPATCH_IOC_ID: {
		uint32 cid;
		void *a;
		uint alen;
		int ret;
		bcm_xdr_buf_t retb;
		rpc_buf_t *rpc_buf;
		uint32 tid;
		bool ta_ok;

		err = bcm_xdr_unpack_uint32(&b, &tid);
		ASSERT(!err);

		err = bcm_xdr_unpack_uint32(&b, &cid);
		ASSERT(!err);

		err = bcm_xdr_unpack_opaque_varlen(&b, &alen, &a);
		ASSERT(!err);

		ret = wlc_bmac_dispatch_ioc(wlc_hw, tid, cid, 0, a, alen, &ta_ok);

		rpc_buf = wlc_rpc_buf_alloc(rpc, &retb, 2 * sizeof(int32) +
		                ROUNDUP(alen, sizeof(uint32))+ sizeof(uint32),
		                WLRPC_WLC_BMAC_DISPATCH_IOC_ID);

		ASSERT(rpc_buf != NULL);

		err = bcm_xdr_pack_opaque_varlen(&retb, alen, a);
		ASSERT(!err);

		err = bcm_xdr_pack_uint32(&retb, ta_ok);
		ASSERT(!err);

		err = bcm_xdr_pack_int32(&retb, ret);
		ASSERT(!err);

		err = wlc_rpc_call_return(rpc, rpc_buf);
		ASSERT(!err);

		break;
	}

	case WLRPC_WLC_PHY_HOLD_UPD_ID: {
		uint32 id, val;
		bool set;

		err = bcm_xdr_unpack_uint32(&b, &id);
		ASSERT(!err);

		err = bcm_xdr_unpack_uint32(&b, &val);
		ASSERT(!err);

		set = (bool)val;

		wlc_phy_hold_upd(((wlc_hw_info_t *)wlc_hw)->band->pi, id, set);
		break;
	}

	case WLRPC_WLC_PHY_ISMUTED_ID: {
		bool muted;
		bcm_xdr_buf_t retb;
		rpc_buf_t *rpc_buf;

		rpc_buf = wlc_rpc_buf_alloc(rpc, &retb, sizeof(uint32),
		                            WLRPC_WLC_PHY_ISMUTED_ID);
		ASSERT(rpc_buf != NULL);

		muted = phy_utils_ismuted((phy_info_t *)((wlc_hw_info_t *)wlc_hw)->band->pi);

		err = bcm_xdr_pack_uint32(&retb, muted);
		ASSERT(!err);

		err = wlc_rpc_call_return(rpc, rpc_buf);
		ASSERT(!err);
		break;
	}

	case WLRPC_WLC_PHY_CAP_GET_ID: {
		uint32 cap;
		bcm_xdr_buf_t retb;
		rpc_buf_t *rpc_buf;

		rpc_buf = wlc_rpc_buf_alloc(rpc, &retb, sizeof(uint32),
		                            WLRPC_WLC_PHY_CAP_GET_ID);
		ASSERT(rpc_buf != NULL);

		cap = wlc_phy_cap_get(((wlc_hw_info_t *)wlc_hw)->band->pi);

		err = bcm_xdr_pack_uint32(&retb, cap);
		ASSERT(!err);

		err = wlc_rpc_call_return(rpc, rpc_buf);
		ASSERT(!err);
		break;
	}

	case WLRPC_WLC_PHY_MUTE_UPD_ID: {
		uint32 flags, val;
		bool set;

		err = bcm_xdr_unpack_uint32(&b, &val);
		ASSERT(!err);

		err = bcm_xdr_unpack_uint32(&b, &flags);
		ASSERT(!err);

		set = (bool)val;

		wlc_phy_mute_upd(((wlc_hw_info_t *)wlc_hw)->band->pi, set, flags);
		break;
	}

	case WLRPC_WLC_PHY_CLEAR_TSSI_ID:
		wlc_phy_clear_tssi(((wlc_hw_info_t *)wlc_hw)->band->pi);
		break;
	case WLRPC_WLC_PHY_TSSIVIS_THRESH_GET_ID: {
		int pval = 0;
		bcm_xdr_buf_t retb;
		rpc_buf_t *rpc_buf;
		rpc_buf = wlc_rpc_buf_alloc(rpc, &retb, sizeof(uint32),
		WLRPC_WLC_PHY_TSSIVIS_THRESH_GET_ID);
		ASSERT(rpc_buf != NULL);
		pval = wlc_phy_tssivisible_thresh(((wlc_hw_info_t *)wlc_hw)->band->pi);
		err = bcm_xdr_pack_uint32(&retb, pval);
		ASSERT(!err);
		err = wlc_rpc_call_return(rpc, rpc_buf);
		ASSERT(!err);
		break;
	}
	case WLRPC_WLC_PHY_ANTDIV_GET_RX_ID: {
		uint8 pval = 0;
		bcm_xdr_buf_t retb;
		rpc_buf_t *rpc_buf;

		rpc_buf = wlc_rpc_buf_alloc(rpc, &retb, sizeof(uint32),
		                            WLRPC_WLC_PHY_ANTDIV_GET_RX_ID);
		ASSERT(rpc_buf != NULL);

		phy_antdiv_get_rx((phy_info_t *)wlc_hw->band->pi, &pval);

		err = bcm_xdr_pack_uint32(&retb, pval);
		ASSERT(!err);

		err = wlc_rpc_call_return(rpc, rpc_buf);
		ASSERT(!err);

		break;
	}

	case WLRPC_WLC_PHY_ANTDIV_SET_RX_ID: {
		int32 ret;
		uint32 val;
		bcm_xdr_buf_t retb;
		rpc_buf_t *rpc_buf;

		rpc_buf = wlc_rpc_buf_alloc(rpc, &retb, sizeof(uint32),
		                            WLRPC_WLC_PHY_ANTDIV_SET_RX_ID);
		ASSERT(rpc_buf != NULL);

		err = bcm_xdr_unpack_uint32(&b, &val);
		ASSERT(!err);

		ret = phy_antdiv_set_rx((phy_info_t *)wlc_hw->band->pi, (uint8)val);

		err = bcm_xdr_pack_int32(&retb, ret);
		ASSERT(!err);

		err = wlc_rpc_call_return(rpc, rpc_buf);
		ASSERT(!err);

		break;
	}

	case WLRPC_WLC_PHY_SAR_LIM_ID: {
		uint32 ret;

		err = bcm_xdr_unpack_uint32(&b, &ret);
		ASSERT(!err);
#ifdef WL_SARLIMIT
		wlc_phy_sar_limit_set(((wlc_hw_info_t *)wlc_hw)->band->pi, ret);
#endif /* WL_SARLIMIT */
		break;
	}

	case WLRPC_WLC_PHY_PREAMBLE_SET_ID: {
		uint32 ret;
		int8 val;

		err = bcm_xdr_unpack_uint32(&b, &ret);
		ASSERT(!err);

		val = (int8)ret;

		wlc_phy_preamble_override_set(((wlc_hw_info_t *)wlc_hw)->band->pi, val);
		break;
	}

	case WLRPC_WLC_PHY_FREQTRACK_END_ID:
		wlc_phy_freqtrack_end(((wlc_hw_info_t *)wlc_hw)->band->pi);
		break;

	case WLRPC_WLC_PHY_FREQTRACK_START_ID:
		wlc_phy_freqtrack_start(((wlc_hw_info_t *)wlc_hw)->band->pi);
		break;

	case WLRPC_WLC_PHY_NOISE_SAMPLE_REQUEST_ID:
		wlc_phy_noise_sample_request_external(((wlc_hw_info_t *)rpc_ctx->wlc_hw)->band->pi);
		break;

	case WLRPC_WLC_PHY_CAL_PERICAL_ID: {
		uint32 ret;
		uint8 reason;

		err = bcm_xdr_unpack_uint32(&b, &ret);
		ASSERT(!err);

		reason = (uint8)ret;

		wlc_phy_cal_perical(((wlc_hw_info_t *)wlc_hw)->band->pi, reason);
		break;
	};

	case WLRPC_WLC_PHY_TXPOWER_GET_ID: {
#if defined(WLTXPWR1_SIGNED)
		int8 qdbm;
#else
		uint qdbm;
#endif // endif
		bool override;

		bcm_xdr_buf_t retb;
		rpc_buf_t *rpc_buf;

		rpc_buf = wlc_rpc_buf_alloc(rpc, &retb, sizeof(uint32) * 2,
		                            WLRPC_WLC_PHY_TXPOWER_GET_ID);
		ASSERT(rpc_buf != NULL);

		wlc_phy_txpower_get(((wlc_hw_info_t*)wlc_hw)->band->pi, &qdbm, &override);

		err = bcm_xdr_pack_uint32(&retb, qdbm);
		ASSERT(!err);

		err = bcm_xdr_pack_uint32(&retb, override);
		ASSERT(!err);

		err = wlc_rpc_call_return(rpc, rpc_buf);
		ASSERT(!err);

		break;
	}

	case WLRPC_WLC_PHY_ACIM_NOISEM_RESET_NPHY_ID: {
		uint32 ret;
		chanspec_t chanspec;
		bool clear_aci_state, clear_noise_state, disassoc;

		err = bcm_xdr_unpack_uint32(&b, &ret);
		ASSERT(!err);
		chanspec = (chanspec_t) ret;

		err = bcm_xdr_unpack_uint32(&b, &ret);
		ASSERT(!err);
		clear_aci_state = (bool) ret;

		err = bcm_xdr_unpack_uint32(&b, &ret);
		ASSERT(!err);
		clear_noise_state = (bool) ret;

		err = bcm_xdr_unpack_uint32(&b, &ret);
		ASSERT(!err);
		disassoc = (bool) ret;

		wlc_phy_acimode_noisemode_reset(((wlc_hw_info_t *)wlc_hw)->band->pi,
			chanspec, clear_aci_state, clear_noise_state, disassoc);
		break;
	}

	case WLRPC_WLC_PHY_INTERFER_SET_NPHY_ID: {
		uint32 ret;
		int init;

		err = bcm_xdr_unpack_uint32(&b, &ret);
		init = (int) ret;

		wlc_phy_interference_set(((wlc_hw_info_t *)wlc_hw)->band->pi, init);
		break;
	}

	case WLRPC_WLC_PHY_TXPOWER_SET_ID: {
		uint32 ret;
		uint qdbm;
		bool override;
		bcm_xdr_buf_t retb;
		rpc_buf_t *rpc_buf;
		void* bptr;
		uint len;
		ppr_t* reg_power;

		rpc_buf = wlc_rpc_buf_alloc(rpc, &retb, sizeof(uint32),
		                            WLRPC_WLC_PHY_TXPOWER_SET_ID);
		ASSERT(rpc_buf != NULL);

		err = bcm_xdr_unpack_uint32(&b, &ret);
		ASSERT(!err);

		qdbm = (uint)ret;

		err = bcm_xdr_unpack_uint32(&b, &ret);
		ASSERT(!err);

		override = (bool)ret;

		err = bcm_xdr_unpack_opaque_varlen(&b, &len, &bptr);
		ASSERT(!err);

		if ((ppr_deserialize_create(wlc_hw->osh, bptr, len, &reg_power)) == BCME_OK) {
			ret = wlc_phy_txpower_set(((wlc_hw_info_t*)wlc_hw)->band->pi, qdbm,
				override, reg_power);
			ppr_delete(wlc_hw->osh, reg_power);
		}
		err = bcm_xdr_pack_int32(&retb, ret);
		ASSERT(!err);

		err = wlc_rpc_call_return(rpc, rpc_buf);
		ASSERT(!err);

		break;
	}

	case WLRPC_WLC_PHY_TXPOWER_SROMLIMIT_ID: {
		int32 ret;
		chanspec_t chan;
		uint32 temp;
		uint8 min_pwr;
		ppr_t* max_pwr;
		uint8 core;

		bcm_xdr_buf_t retb;
		rpc_buf_t *rpc_buf;
		uint len = 0;
		uint slen = 0;
		uint8* tbuf = NULL;
		wl_tx_bw_t ch_bw;

		err = bcm_xdr_unpack_uint32(&b, (uint32*)&temp);
		ASSERT(!err);
		chan = (chanspec_t) temp;

		err = bcm_xdr_unpack_int32(&b, &ret);
		ASSERT(!err);

		core = (uint8)ret;

		err = bcm_xdr_unpack_int32(&b, &ret);
		if (!err) {
			ch_bw = (wl_tx_bw_t)ret;

			if ((max_pwr = ppr_create(wlc_hw->osh, ch_bw)) != NULL) {
				slen = ppr_ser_size(max_pwr);
				if ((tbuf = MALLOC(wlc_hw->osh, slen)) != NULL) {
					wlc_phy_txpower_sromlimit(
						((wlc_hw_info_t *)wlc_hw)->band->pi, chan, &min_pwr,
						max_pwr, core);

					if (ppr_serialize(max_pwr, tbuf, slen, &len) == BCME_OK) {
						len = XDR_PACK_OPAQUE_VAR_SZ(len);
					}
				}
			}
		} else {
			max_pwr = NULL;
			wlc_phy_txpower_sromlimit(((wlc_hw_info_t *)wlc_hw)->band->pi, chan,
				&min_pwr, max_pwr, core);
			len = sizeof(uint32);
		}

		len = len + sizeof(uint32); /* tbuf size + min power */
		rpc_buf = wlc_rpc_buf_alloc(rpc, &retb, len + 2*sizeof(uint32),
			WLRPC_WLC_PHY_TXPOWER_SROMLIMIT_ID);

		ASSERT(rpc_buf != NULL);

		err = bcm_xdr_pack_uint32(&retb, min_pwr);
		ASSERT(!err);

		if (max_pwr) {
			err = bcm_xdr_pack_opaque_varlen(&retb, len, tbuf);
			ASSERT(!err);
		}

		err = wlc_rpc_call_return(rpc, rpc_buf);
		ASSERT(!err);

		if (tbuf != NULL)
			MFREE(wlc_hw->osh, tbuf, slen);
		if (max_pwr != NULL)
			ppr_delete(wlc_hw->osh, max_pwr);

		break;
	}

	case WLRPC_WLC_PHY_RADAR_DETECT_ENABLE_ID: {
		uint32 ret;
		bool on;

		err = bcm_xdr_unpack_uint32(&b, &ret);
		ASSERT(!err);

		on = (bool)ret;

#if defined(AP) && defined(RADAR)
		phy_radar_detect_enable((phy_info_t *)wlc_hw->band->pi, on);
#endif // endif
		break;
	}

	case WLRPC_WLC_PHY_RADAR_DETECT_RUN_ID: {
		bcm_xdr_buf_t retb;
		rpc_buf_t *rpc_buf;
		int32 ret;

		rpc_buf = wlc_rpc_buf_alloc(rpc, &retb, sizeof(uint32),
		                            WLRPC_WLC_PHY_RADAR_DETECT_RUN_ID);
		ASSERT(rpc_buf != NULL);

#if defined(AP) && defined(RADAR)
		ret = phy_radar_detect_run((phy_info_t *)wlc_hw->band->pi, 0);
#else
		ret = 0;
#endif // endif
		err = bcm_xdr_pack_int32(&retb, ret);
		ASSERT(!err);

		err = wlc_rpc_call_return(rpc, rpc_buf);
		ASSERT(!err);

		break;
	}

	case WLRPC_WLC_PHY_RADAR_DETECT_MODE_SET_ID: {
#if defined(AP) && defined(RADAR)
		phy_radar_detect_mode_t mode;
		uint32 tmp;

		err = bcm_xdr_unpack_uint32(&b, &tmp);
		ASSERT(!err);

		mode = (phy_radar_detect_mode_t)tmp;
		phy_radar_detect_mode_set((phy_info_t *)wlc_hw->band->pi, mode);
#endif // endif
		break;
	}

	case WLRPC_WLC_PHY_TEST_ISON_ID :{
		bool phytest_on;
		bcm_xdr_buf_t retb;
		rpc_buf_t *rpc_buf;

		rpc_buf = wlc_rpc_buf_alloc(rpc, &retb, sizeof(uint32),
		                            WLRPC_WLC_PHY_TEST_ISON_ID);
		ASSERT(rpc_buf != NULL);

		phytest_on = wlc_phy_test_ison(wlc_hw->band->pi);

		err = bcm_xdr_pack_uint32(&retb, phytest_on);
		ASSERT(!err);

		err = wlc_rpc_call_return(rpc, rpc_buf);
		ASSERT(!err);
		break;
	}

	case WLRPC_WLC_BMAC_COPYFROM_VARS_ID: {
		static char *vars;
		static uint vars_len;
		char *buffer;
		uint buflen;
		uint32 offset;
		uint32 residue_len;
		bcm_xdr_buf_t retb;
		rpc_buf_t *rpc_buf;

		err = bcm_xdr_unpack_uint32(&b, &offset);
		ASSERT(!err);

		if (offset == 0) {
			wlc_bmac_copyfrom_vars(wlc_hw, &vars, &vars_len);
		}

		buflen = MIN((vars_len - offset), MAX_VARS_BUFLEN);
		buffer = vars + offset;
		residue_len = vars_len - buflen - offset;

		rpc_buf = wlc_rpc_buf_alloc(rpc, &retb,
			2*sizeof(uint32) + ROUNDUP(buflen, sizeof(uint32)),
		                            WLRPC_WLC_BMAC_COPYFROM_VARS_ID);
		ASSERT(rpc_buf != NULL);

		err = bcm_xdr_pack_uint32(&retb, residue_len);
		ASSERT(!err);

		err = bcm_xdr_pack_opaque_varlen(&retb, buflen, buffer);
		ASSERT(!err);

		err = wlc_rpc_call_return(rpc, rpc_buf);
		ASSERT(!err);

		break;
	}

	case WLRPC_WLC_BMAC_COPYFROM_OBJMEM_ID: {
		uint offset;
		uint8* shmbuf;
		int len;
		uint32 sel;
		bcm_xdr_buf_t retb;
		rpc_buf_t *rpc_buf;

		err = bcm_xdr_unpack_uint32(&b, &offset);
		ASSERT(!err);

		err = bcm_xdr_unpack_int32(&b, &len);
		ASSERT(!err && (len > 0));

		err = bcm_xdr_unpack_uint32(&b, &sel);
		ASSERT(!err);

		if ((shmbuf = (uint8*)MALLOC(NULL, len)) == NULL) {
			ASSERT(0);
			break;
		}

		wlc_bmac_copyfrom_objmem(wlc_hw, offset, shmbuf, len, sel);

		rpc_buf = wlc_rpc_buf_alloc(rpc, &retb,
		                            sizeof(uint32) + ROUNDUP(len, sizeof(uint32)),
		                            WLRPC_WLC_BMAC_COPYFROM_OBJMEM_ID);
		ASSERT(rpc_buf != NULL);

		err = bcm_xdr_pack_opaque_varlen(&retb, len, shmbuf);
		ASSERT(!err);

		err = wlc_rpc_call_return(rpc, rpc_buf);
		ASSERT(!err);

		MFREE(NULL, shmbuf, FALSE);
		break;
	}
	case WLRPC_WLC_BMAC_COPYTO_OBJMEM_ID: {
		uint offset;
		void* shmbuf;
		uint len;
		uint32 sel;
		err = bcm_xdr_unpack_uint32(&b, &offset);
		ASSERT(!err);

		err = bcm_xdr_unpack_opaque_varlen(&b, &len, &shmbuf);
		ASSERT(!err);

		err = bcm_xdr_unpack_uint32(&b, &sel);
		ASSERT(!err);

		wlc_bmac_copyto_objmem(wlc_hw, offset, shmbuf, len, sel);
		break;
	}
	case WLRPC_WLC_ENABLE_MAC_ID:
		wlc_bmac_enable_mac(wlc_hw);
		break;

	case WLRPC_WLC_MCTRL_ID: {
		uint32 mask;
		uint32 val;

		err = bcm_xdr_unpack_uint32(&b, &mask);
		ASSERT(!err);

		err = bcm_xdr_unpack_uint32(&b, &val);
		ASSERT(!err);

		wlc_bmac_mctrl(wlc_hw, mask, val);
		break;
	}

	case WLRPC_WLC_CORERESET_ID: {
		uint32 flags;

		err = bcm_xdr_unpack_uint32(&b, &flags);
		ASSERT(!err);

		wlc_bmac_corereset(wlc_hw, flags);
		break;
	}

	case WLRPC_WLC_BMAC_READ_SHM_ID: {
		uint32 ret;
		uint offset;
		bcm_xdr_buf_t retb;
		rpc_buf_t *rpc_buf;

		rpc_buf = wlc_rpc_buf_alloc(rpc, &retb, sizeof(uint32), WLRPC_WLC_BMAC_READ_SHM_ID);
		ASSERT(rpc_buf != NULL);

		err = bcm_xdr_unpack_uint32(&b, &offset);
		ASSERT(!err);

		ret = (uint16)wlc_bmac_read_shm(wlc_hw, offset);

		err = bcm_xdr_pack_uint32(&retb, ret);
		ASSERT(!err);

		err = wlc_rpc_call_return(rpc, rpc_buf);
		ASSERT(!err);

		break;
	}

	case WLRPC_WLC_BMAC_READ_TSF_ID: {
		uint32 tsf_l_ptr;
		uint32 tsf_h_ptr;

		bcm_xdr_buf_t retb;
		rpc_buf_t *rpc_buf;

		rpc_buf = wlc_rpc_buf_alloc(rpc, &retb, sizeof(uint32) * 2,
			WLRPC_WLC_BMAC_READ_TSF_ID);
		ASSERT(rpc_buf != NULL);

		wlc_bmac_read_tsf(wlc_hw, &tsf_l_ptr, &tsf_h_ptr);

		err = bcm_xdr_pack_uint32(&retb, tsf_l_ptr);
		ASSERT(!err);

		err = bcm_xdr_pack_uint32(&retb, tsf_h_ptr);
		ASSERT(!err);

		err = wlc_rpc_call_return(rpc, rpc_buf);
		ASSERT(!err);

		break;
	}

	case WLRPC_WLC_BMAC_SET_RXE_ADDRMATCH_ID: {
		int match_reg_offset;
		struct ether_addr addr;

		err = bcm_xdr_unpack_int32(&b, &match_reg_offset);
		ASSERT(!err);

		err = bcm_xdr_unpack_opaque_cpy(&b, sizeof(struct ether_addr), &addr);
		ASSERT(!err);

		wlc_bmac_set_rxe_addrmatch(wlc_hw, match_reg_offset, &addr);
		break;
	}

	case WLRPC_WLC_BMAC_SET_CWMAX_ID: {
		uint32 ret;
		uint16 newmax;

		err = bcm_xdr_unpack_uint32(&b, &ret);
		ASSERT(!err);

		newmax = (uint16)ret;

		wlc_bmac_set_cwmax(wlc_hw, newmax);
		break;
	}

	case WLRPC_WLC_BMAC_GET_RCMTA_ID: {
		int idx;
		struct ether_addr ea;
		bcm_xdr_buf_t retb;
		rpc_buf_t *rpc_buf;

		err = bcm_xdr_unpack_int32(&b, &idx);
		ASSERT(!err);

		wlc_bmac_get_rcmta(wlc_hw, idx, &ea);

		rpc_buf = wlc_rpc_buf_alloc(rpc, &retb,
			ROUNDUP(sizeof(ea), sizeof(uint32)), WLRPC_WLC_BMAC_GET_RCMTA_ID);
		ASSERT(rpc_buf != NULL);

		err = bcm_xdr_pack_opaque(&retb, sizeof(ea), &ea);
		ASSERT(!err);

		err = wlc_rpc_call_return(rpc, rpc_buf);
		ASSERT(!err);

		break;
	}

	case WLRPC_WLC_BMAC_READ_AMT_ID: {
		int idx;
		struct ether_addr ea;
		uint16 attr;
		uint32 temp;
		bcm_xdr_buf_t retb;
		rpc_buf_t *rpc_buf;

		err = bcm_xdr_unpack_int32(&b, &idx);
		ASSERT(!err);

		wlc_bmac_read_amt(wlc_hw, idx, &ea, &attr);

		rpc_buf = wlc_rpc_buf_alloc(rpc, &retb,
			ROUNDUP(sizeof(ea) + sizeof(temp), sizeof(uint32)),
			WLRPC_WLC_BMAC_READ_AMT_ID);
		ASSERT(rpc_buf != NULL);

		err = bcm_xdr_pack_opaque(&retb, sizeof(ea), &ea);
		ASSERT(!err);

		temp = attr;
		err = bcm_xdr_pack_uint32(&retb, temp);
		ASSERT(!err);

		err = wlc_rpc_call_return(rpc, rpc_buf);
		ASSERT(!err);

		break;
	}

	case WLRPC_WLC_BMAC_SET_RCMTA_ID: {
		int idx;
		struct ether_addr addr;

		err = bcm_xdr_unpack_int32(&b, &idx);
		ASSERT(!err);

		err = bcm_xdr_unpack_opaque_cpy(&b, sizeof(struct ether_addr), &addr);
		ASSERT(!err);

		wlc_bmac_set_rcmta(wlc_hw, idx, &addr);
		break;
	}

	case WLRPC_WLC_BMAC_WRITE_AMT_ID: {
		int idx;
		uint32 val;
		uint16 attr;
		struct ether_addr addr;
		bcm_xdr_buf_t retb;
		rpc_buf_t *rpc_buf;

		err = bcm_xdr_unpack_int32(&b, &idx);
		ASSERT(!err);

		err = bcm_xdr_unpack_opaque_cpy(&b, sizeof(struct ether_addr), &addr);
		ASSERT(!err);

		err = bcm_xdr_unpack_uint32(&b, &val);
		ASSERT(!err);
		attr = (uint16)val;

		val = wlc_bmac_write_amt(wlc_hw, idx, &addr, attr);

		rpc_buf = wlc_rpc_buf_alloc(rpc, &retb, sizeof(uint32),
			WLRPC_WLC_BMAC_WRITE_AMT_ID);
		ASSERT(rpc_buf != NULL);

		err = bcm_xdr_pack_uint32(&retb, val);
		ASSERT(!err);

		err = wlc_rpc_call_return(rpc, rpc_buf);
		ASSERT(!err);
		break;
	}

	case WLRPC_WLC_BMAC_SET_SHM_ID: {
		uint32 ret;
		uint offset;
		uint16 v;
		int len;

		err = bcm_xdr_unpack_uint32(&b, &offset);
		ASSERT(!err);

		err = bcm_xdr_unpack_uint32(&b, &ret);
		ASSERT(!err);

		v = (uint16)ret;

		err = bcm_xdr_unpack_int32(&b, &len);
		ASSERT(!err);

		wlc_bmac_set_shm(wlc_hw, offset, v, len);
		break;
	}

	case WLRPC_WLC_SUSPEND_MAC_AND_WAIT_ID:
		wlc_bmac_suspend_mac_and_wait(wlc_hw);
		break;

	case WLRPC_WLC_BMAC_WRITE_IHR_ID:
	case WLRPC_WLC_BMAC_WRITE_SHM_ID: {
		uint32 ret;
		uint offset;
		uint16 v;

		err = bcm_xdr_unpack_uint32(&b, &ret);
		ASSERT(!err);

		offset = (uint)ret;

		err = bcm_xdr_unpack_uint32(&b, &ret);
		ASSERT(!err);

		v = (uint16)ret;

		if (rpc_id == WLRPC_WLC_BMAC_WRITE_SHM_ID)
			wlc_bmac_write_shm(wlc_hw, offset, v);
		else
			wlc_bmac_write_ihr(wlc_hw, offset, v);
		break;
	}

	case WLRPC_WLC_BMAC_UPDATE_SHM_ID: {
		uint32 ret;
		uint offset;
		uint16 v;
		uint16 mask;

		err = bcm_xdr_unpack_uint32(&b, &ret);
		ASSERT(!err);

		offset = (uint)ret;

		err = bcm_xdr_unpack_uint32(&b, &ret);
		ASSERT(!err);

		v = (uint16)ret;

		err = bcm_xdr_unpack_uint32(&b, &ret);
		ASSERT(!err);

		mask = (uint16)ret;

		wlc_bmac_update_shm(wlc_hw, offset, v, mask);
		break;
	}

	case WLRPC_WLC_BMAC_WRITE_TEMPLATE_RAM_ID: {
		int offset;
		uint len;
		void *bufl;

		err = bcm_xdr_unpack_int32(&b, &offset);
		ASSERT(!err);

		err = bcm_xdr_unpack_opaque_varlen(&b, &len, &bufl);
		ASSERT(!err);

		wlc_bmac_write_template_ram(wlc_hw, offset, (int)len, bufl);
		break;
	}

	case WLRPC_WLC_TX_FIFO_SUSPEND_ID: {
		uint32 ret;
		uint tx_fifo;

		err = bcm_xdr_unpack_uint32(&b, &ret);
		ASSERT(!err);

		tx_fifo = (uint)ret;

		wlc_bmac_tx_fifo_suspend(wlc_hw, tx_fifo);
		break;
	}

	case WLRPC_WLC_TX_FIFO_RESUME_ID: {
		uint32 ret;
		uint tx_fifo;

		err = bcm_xdr_unpack_uint32(&b, &ret);
		ASSERT(!err);

		tx_fifo = (uint)ret;

		wlc_bmac_tx_fifo_resume(wlc_hw, tx_fifo);
		break;
	}

	case WLRPC_WLC_TX_FIFO_SUSPENDED_ID: {
		uint32 ret;
		uint tx_fifo;
		bool is_suspended;
		bcm_xdr_buf_t retb;
		rpc_buf_t *rpc_buf;

		rpc_buf = wlc_rpc_buf_alloc(rpc, &retb, sizeof(uint32),
		                            WLRPC_WLC_TX_FIFO_SUSPENDED_ID);
		ASSERT(rpc_buf != NULL);
		err = bcm_xdr_unpack_uint32(&b, &ret);
		ASSERT(!err);

		tx_fifo = (uint)ret;

		is_suspended = wlc_bmac_tx_fifo_suspended(wlc_hw, tx_fifo);

		err = bcm_xdr_pack_uint32(&retb, is_suspended);
		ASSERT(!err);

		err = wlc_rpc_call_return(rpc, rpc_buf);
		ASSERT(!err);
		break;
	}

	case WLRPC_WLC_HW_ETHERADDR_ID: {
		struct ether_addr ea;

		bcm_xdr_buf_t retb;
		rpc_buf_t *rpc_buf;

		rpc_buf = wlc_rpc_buf_alloc(rpc, &retb,
		                            ROUNDUP(sizeof(struct ether_addr), sizeof(uint32)),
		                            WLRPC_WLC_HW_ETHERADDR_ID);
		ASSERT(rpc_buf != NULL);

		wlc_bmac_hw_etheraddr(wlc_hw, &ea);

		err = bcm_xdr_pack_opaque(&retb, sizeof(struct ether_addr), &ea);
		ASSERT(!err);

		err = wlc_rpc_call_return(rpc, rpc_buf);
		ASSERT(!err);

		break;
	}

	case WLRPC_WLC_SET_HW_ETHERADDR_ID: {
		struct ether_addr ea;

		err = bcm_xdr_unpack_opaque_cpy(&b, sizeof(struct ether_addr), &ea);
		ASSERT(!err);

		wlc_bmac_set_hw_etheraddr(wlc_hw, &ea);

		break;
	}

	case WLRPC_WLC_BMAC_CHANSPEC_SET_ID: {
		uint32 ret;
		chanspec_t chanspec;
		bool mute;
		ppr_t* txppr;
		void* bptr;
		uint len;

		err = bcm_xdr_unpack_uint32(&b, &ret);
		ASSERT(!err);
		chanspec = (chanspec_t)ret;

		err = bcm_xdr_unpack_uint32(&b, &ret);
		ASSERT(!err);
		mute = (bool)ret;

		err = bcm_xdr_unpack_opaque_varlen(&b, &len, &bptr);
		ASSERT(!err);

		if ((ppr_deserialize_create(wlc_hw->osh, bptr, len, &txppr)) == BCME_OK) {
			wlc_bmac_set_chanspec(wlc_hw, chanspec, mute, txppr);
			ppr_delete(wlc_hw->osh, txppr);
		}
		break;
	}

	case WLRPC_WLC_BMAC_TXANT_SET_ID: {
		uint32 ret;
		uint16 phytxant;

		err = bcm_xdr_unpack_uint32(&b, &ret);
		ASSERT(!err);

		phytxant = (uint16)ret;

		wlc_bmac_txant_set(wlc_hw, phytxant);
		break;
	}

	case WLRPC_WLC_BMAC_ANTSEL_TYPE_SET_ID: {
		uint8 antsel_type;
		uint32 ret;

		err = bcm_xdr_unpack_uint32(&b, &ret);
		ASSERT(!err);

		antsel_type = (uint8)ret;

		/* Update the antsel type in the bmac */
		wlc_bmac_antsel_type_set(wlc_hw, antsel_type);
		break;
	}

	case WLRPC_WLC_BMAC_DEBUG_ID: {
		/* DEBUG, host can send command down synchronously using call_with_return */
		bcm_xdr_buf_t retb;
		rpc_buf_t *rpc_buf;

		/* any thing for debug, like histogram */

		rpc_buf = wlc_rpc_buf_alloc(rpc, &retb, sizeof(uint32), WLRPC_WLC_BMAC_DEBUG_ID);
		ASSERT(rpc_buf != NULL);
		err = wlc_rpc_call_return(rpc, rpc_buf);
		ASSERT(!err);
		break;
	}

	case WLRPC_WLC_BMAC_TXFIFO_ID: {
		void *prev_p = NULL;
		void *p;
		uint32 ret;
		uint fifo;
		bool commit;
		uint16 frameid;
		uint8 txpktpend;
		uint32 flags;
		uint32 exptime;
		uint32 totlen;
		osl_t *osh;

		osh = wlc_hw->osh;

/* chained pkts are only used by host for txfifo. Need to handle the boundary case for each move */
#define HANDLE_BOUNDARY(b, p, prev_p)						\
	do {									\
		if (b.size == 0) {						\
			prev_p = p;						\
			p = PKTNEXT(osh, prev_p);				\
			bcm_xdr_buf_init(&b, PKTDATA(osh, p), PKTLEN(osh, p));	\
			WL_NONE(("TXFIFO: next p %p len %d data %p\n",		\
			p, PKTLEN(osh, p), PKTDATA(osh, p)));			\
		}								\
	} while (0)

		p = (struct lbuf *)buf;

		WL_NONE(("TXFIFO: totlen %d p %p len %d data %p\n",
		         pkttotlen(osh, p), p, PKTLEN(osh, p), PKTDATA(osh, p)));

		err = bcm_xdr_unpack_uint32(&b, &fifo);
		ASSERT(!err);
		HANDLE_BOUNDARY(b, p, prev_p);

		err = bcm_xdr_unpack_uint32(&b, &ret);
		ASSERT(!err);
		HANDLE_BOUNDARY(b, p, prev_p);

		commit = (bool)ret;

		err = bcm_xdr_unpack_uint32(&b, &ret);
		ASSERT(!err);
		HANDLE_BOUNDARY(b, p, prev_p);

		frameid = (uint16)ret;

		err = bcm_xdr_unpack_uint32(&b, &ret);
		ASSERT(!err);
		HANDLE_BOUNDARY(b, p, prev_p);

		txpktpend = (uint8)ret;

		err = bcm_xdr_unpack_uint32(&b, &flags);
		ASSERT(!err);
		HANDLE_BOUNDARY(b, p, prev_p);

		err = bcm_xdr_unpack_uint32(&b, &exptime);
		ASSERT(!err);
		HANDLE_BOUNDARY(b, p, prev_p);

		err = bcm_xdr_unpack_uint32(&b, &totlen);
		ASSERT(!err);
		HANDLE_BOUNDARY(b, p, prev_p);

		if (flags & WLC_BMAC_F_PAD2) {
			PKTPULL(osh, p, WLC_RPC_TXFIFO_UNALIGN_PAD_2BYTE);
			totlen -= WLC_RPC_TXFIFO_UNALIGN_PAD_2BYTE;
		}

		if (flags & WLC_BMAC_F_AMPDU_MPDU)
			WLPKTTAG(p)->flags = WLF_AMPDU_MPDU;

		/* Packet expiration time implemented in low driver */
		WLPKTTAG(p)->u.exptime = exptime;

		/* if we are not at the head buffer in the rpc buffer
		 * chain, unlink the chain so the head will not free the remaining
		 * buffers.
		 * Otherwise we will be using the entire chain including the head
		 * buffer, so set the flag to skip the free of the 'buf' parameter.
		 */
		if (prev_p) {
			WL_NONE(("TXFIFO: unlinking prev_p %p from p %p\n", prev_p, p));
			PKTSETNEXT(osh, prev_p, NULL);
		} else {
			WL_NONE(("TXFIFO: buf %p is p %p, so don't free\n", buf, p));
			free_buf = FALSE;
		}

		p = wlc_rpc_bmac_txfifo_buf_to_txpkt(rpc, wlc_hw->osh, &b, p, totlen);

		if (p == NULL) {
			printf("%s: Pkt alloc failed :%d\n", __FUNCTION__, totlen);
			break;
		}

		WL_TRACE(("%s fifo:%d txpktpend:%d totlen:%d flags:0x%x p:%p\n",
		          __FUNCTION__, fifo, txpktpend, totlen, flags, p));

		if (fifo == TX_DATA_FIFO) {
			int current_pend = TXPKTPENDGET(wlc, fifo);

			if ((current_pend < HIST_SIZE) && (current_pend >= 0)) {
				histogram_txfifo[current_pend]++;
			} else {
				WL_TRACE(("wl%d:TXPKTPENDGET: %d\n", wlc_hw->unit, current_pend));
				if (current_pend < 0) {
					histogram_txfifo[0]++;
				} else {
					histogram_txfifo[HIST_SIZE - 1]++;
				}
			}
		}

		wlc_bmac_txfifo(wlc_hw, fifo, p, commit, frameid, txpktpend);
		break;
	}

	case WLRPC_WLC_RADIO_READ_HWDISABLED_ID: {
		bcm_xdr_buf_t retb;
		rpc_buf_t *rpc_buf;
		uint32 ret;

		rpc_buf = wlc_rpc_buf_alloc(rpc, &retb, sizeof(uint32),
		                            WLRPC_WLC_RADIO_READ_HWDISABLED_ID);
		ASSERT(rpc_buf != NULL);

		ret = (uint32)wlc_bmac_radio_read_hwdisabled(wlc_hw);

		err = bcm_xdr_pack_uint32(&retb, ret);
		ASSERT(!err);

		err = wlc_rpc_call_return(rpc, rpc_buf);
		ASSERT(!err);

		break;
	}

#ifdef STA
#ifdef WLRM
	case WLRPC_WLC_RM_CCA_MEASURE_ID: {
		uint32 us;

		err = bcm_xdr_unpack_uint32(&b, &us);
		ASSERT(!err);

		wlc_bmac_rm_cca_measure(wlc_hw, us);
		break;
	}
#endif	/* WLRM */
#endif	/* STA */

	case WLRPC_WLC_SET_SHORTSLOT_ID: {
		uint32 ret;
		bool shortslot;

		err = bcm_xdr_unpack_uint32(&b, &ret);
		ASSERT(!err);

		shortslot = (bool)ret;

		wlc_bmac_set_shortslot(wlc_hw, shortslot);
		break;
	}

	case WLRPC_WLC_WAIT_FOR_WAKE_ID:
		wlc_bmac_wait_for_wake(wlc_hw);
		break;

#ifdef	WL11N
	case WLRPC_WLC_BAND_STF_SS_SET_ID: {
		uint8 stf_mode;
		uint32 ret;

		err = bcm_xdr_unpack_uint32(&b, &ret);
		ASSERT(!err);

		stf_mode = (uint8)ret;

		wlc_bmac_band_stf_ss_set(wlc_hw, stf_mode);
		break;
	}
#endif	/* WL11N */

	case WLRPC_WLC_PHY_TXPOWER_GET_CURRENT_ID: {
		bcm_xdr_buf_t retb;
		rpc_buf_t *rpc_buf;
		phy_tx_power_t *power;
		uint32 temp_var;
		wl_tx_bw_t ch_bw;
		uint ser_size_bl, ser_size_tp;
		uint8 *buf_bl, *buf_tp;
		void* bptr;
		uint len;
		ppr_t* reg_power;
		uint32 offset, size;

		power = (phy_tx_power_t *)MALLOC(wlc_hw->osh, sizeof(*power));
		if (power == NULL) {
			ASSERT(0);
			break;
		}

		err = bcm_xdr_unpack_uint32(&b, &temp_var);
		ASSERT(!err);
		offset = (temp_var >> 16) & 0xffff;
		size = (temp_var & 0xffff);
		BCM_REFERENCE(size);

		err = bcm_xdr_unpack_uint32(&b, &temp_var);
		ASSERT(!err);
		power->flags = temp_var;

		err = bcm_xdr_unpack_uint32(&b, &temp_var);
		ASSERT(!err);
		power->chanspec = (temp_var >> 16) & 0xffff;
		power->local_chanspec = (temp_var & 0xffff);

		err = bcm_xdr_unpack_uint32(&b, &ch_bw);
		ASSERT(!err);

		power->ppr_board_limits = ppr_create(wlc_hw->osh, ch_bw);
		if (power->ppr_board_limits == NULL) {
			ASSERT(0);
			MFREE(wlc_hw->osh, power, sizeof(*power));
			break;
		}

		power->ppr_target_powers = ppr_create(wlc->osh, ch_bw);
		if (power->ppr_target_powers == NULL) {
			ASSERT(0);
			ppr_delete(wlc_hw->osh, power->ppr_board_limits);
			MFREE(wlc_hw->osh, power, sizeof(*power));
			break;
		}

		ser_size_bl = ppr_ser_size(power->ppr_board_limits);
		ser_size_tp = ppr_ser_size(power->ppr_target_powers);

		rpc_buf = wlc_rpc_buf_alloc(rpc, &retb,
			ROUNDUP(OFFSETOF(phy_tx_power_t, ppr_board_limits), sizeof(uint32)) +
			ROUNDUP(ser_size_bl, sizeof(uint32)) + sizeof(uint32) +
			ROUNDUP(ser_size_tp, sizeof(uint32)) + sizeof(uint32),
		WLRPC_WLC_PHY_TXPOWER_GET_CURRENT_ID);
		ASSERT(rpc_buf != NULL);

		err = bcm_xdr_unpack_opaque_varlen(&b, &len, &bptr);
		ASSERT(!err);

		if ((ppr_deserialize_create(wlc_hw->osh, bptr, len, &reg_power)) == BCME_OK) {
			wlc_phy_txpower_get_current(((wlc_hw_info_t *)wlc_hw)->band->pi, reg_power,
				power);
			ppr_delete(wlc_hw->osh, reg_power);
		}

		if (offset == 0) {
			ser_size_bl = ppr_ser_size(power->ppr_board_limits);
			err = bcm_xdr_pack_opaque(&retb, OFFSETOF(phy_tx_power_t, ppr_board_limits),
			((uint8 *)power));
			ASSERT(!err);

			err = bcm_xdr_opaque_resrv_varlen(&retb, ser_size_bl, (void **)&buf_bl);
			ASSERT(!err);
			err = ppr_serialize
				(power->ppr_board_limits, buf_bl, ser_size_bl, &ser_size_bl);
			ASSERT(!err);
		} else {
			ser_size_tp = ppr_ser_size(power->ppr_target_powers);
			err = bcm_xdr_opaque_resrv_varlen(&retb, ser_size_tp, (void **)&buf_tp);
			ASSERT(!err);
			err = ppr_serialize
				(power->ppr_target_powers, buf_tp, ser_size_tp, &ser_size_tp);
			ASSERT(!err);
		}

		err = wlc_rpc_call_return(rpc, rpc_buf);
		ASSERT(!err);

		ppr_delete(wlc_hw->osh, power->ppr_board_limits);
		ppr_delete(wlc_hw->osh, power->ppr_target_powers);
		MFREE(wlc_hw->osh, power, sizeof(*power));

		break;
	}

	case WLRPC_WLC_PHY_BAND_FIRST_CHANSPEC_ID: {
		uint band, band_index;
		phy_info_t *pi;
		bcm_xdr_buf_t retb;
		rpc_buf_t *rpc_buf;
		uint32 ret;

		rpc_buf = wlc_rpc_buf_alloc(rpc, &retb, sizeof(uint32),
		                            WLRPC_WLC_PHY_BAND_FIRST_CHANSPEC_ID);
		ASSERT(rpc_buf != NULL);

		err = bcm_xdr_unpack_uint32(&b, &band);
		ASSERT(!err);

		band_index = (band == WLC_BAND_2G) ? BAND_2G_INDEX : BAND_5G_INDEX;

		pi = (phy_info_t *)((wlc_hw_info_t *)wlc_hw)->bandstate[band_index]->pi;
		ret = (uint32)phy_utils_chanspec_band_firstch(pi, band);

		err = bcm_xdr_pack_uint32(&retb, ret);
		ASSERT(!err);

		err = wlc_rpc_call_return(rpc, rpc_buf);
		ASSERT(!err);
		break;
	}

	case WLRPC_WLC_PHY_TXPOWER_HW_CTRL_GET_ID :{
		bool hwpwrctrl;
		bcm_xdr_buf_t retb;
		rpc_buf_t *rpc_buf;

		rpc_buf = wlc_rpc_buf_alloc(rpc, &retb, sizeof(uint32),
		                            WLRPC_WLC_PHY_TXPOWER_HW_CTRL_GET_ID);
		ASSERT(rpc_buf != NULL);

		hwpwrctrl = wlc_phy_txpower_hw_ctrl_get(((wlc_hw_info_t *)wlc_hw)->band->pi);

		err = bcm_xdr_pack_uint32(&retb, hwpwrctrl);
		ASSERT(!err);

		err = wlc_rpc_call_return(rpc, rpc_buf);
		ASSERT(!err);
		break;
	}

	case WLRPC_WLC_PHY_TXPOWER_HW_CTRL_SET_ID: {
		bool hwpwrctrl;
		uint32 data;

		err = bcm_xdr_unpack_uint32(&b, &data);
		ASSERT(!err);

		hwpwrctrl = (bool)data;

		wlc_phy_txpower_hw_ctrl_set(((wlc_hw_info_t *)wlc_hw)->band->pi, hwpwrctrl);

		break;
	}

	case WLRPC_WLC_PHY_TXPOWER_LIMIT_SET_ID: {
		chanspec_t chanspec;
		uint32 ret;

		ppr_t* txppr;
		void* bptr;
		uint len;
		err = bcm_xdr_unpack_uint32(&b, &ret);
		ASSERT(!err);
		chanspec = (chanspec_t)ret;

		err = bcm_xdr_unpack_opaque_varlen(&b, &len, &bptr);
		ASSERT(!err);

		if ((ppr_deserialize_create(wlc_hw->osh, bptr, len, &txppr)) == BCME_OK) {
			wlc_phy_txpower_limit_set(((wlc_hw_info_t *)wlc_hw)->band->pi,
				txppr, chanspec);
			ppr_delete(wlc_hw->osh, txppr);
		}
		break;
	}

	case WLRPC_WLC_PHY_BSSINIT_ID: {
		bool bonlyap;
		int noise;
		uint32 uret;

		err = bcm_xdr_unpack_uint32(&b, &uret);
		ASSERT(!err);

		bonlyap = (bool)uret;

		err = bcm_xdr_unpack_int32(&b, &noise);
		ASSERT(!err);

		wlc_phy_BSSinit(((wlc_hw_info_t *)wlc_hw)->band->pi, bonlyap, noise);

		break;
	}

	case WLRPC_WLC_PHY_BAND_CHANNELS_ID: {
		bcm_xdr_buf_t retb;
		rpc_buf_t *rpc_buf;
		uint band;
		chanvec_t channels;

		err = bcm_xdr_unpack_uint32(&b, &band);
		ASSERT(!err);

		phy_utils_chanspec_band_validch((phy_info_t *)((wlc_hw_info_t *)wlc_hw)->band->pi,
			band, &channels);

		rpc_buf = wlc_rpc_buf_alloc(rpc, &retb,
		                            ROUNDUP(sizeof(chanvec_t), sizeof(uint32)),
		                            WLRPC_WLC_PHY_BAND_CHANNELS_ID);
		ASSERT(rpc_buf != NULL);

		err = bcm_xdr_pack_opaque(&retb, sizeof(chanvec_t), &channels);
		ASSERT(!err);

		err = wlc_rpc_call_return(rpc, rpc_buf);
		ASSERT(!err);

		break;
	}

	case WLRPC_WLC_BMAC_REVINFO_GET_ID: {
		bcm_xdr_buf_t retb;
		rpc_buf_t *rpc_buf;
		wlc_bmac_revinfo_t revinfo;
		uint idx;

		printf("revinfo\n");
		/* round up for ccode? */
		rpc_buf = wlc_rpc_buf_alloc(rpc, &retb,
		                            ROUNDUP(sizeof(wlc_bmac_revinfo_t), sizeof(uint32)),
		                            WLRPC_WLC_BMAC_REVINFO_GET_ID);
		ASSERT(rpc_buf != NULL);

		wlc_bmac_revinfo_get(wlc->hw, &revinfo);

		/* pack the whole revinfo structure */
		err = bcm_xdr_pack_uint32(&retb, revinfo.vendorid);
		ASSERT(!err);

		err = bcm_xdr_pack_uint32(&retb, revinfo.deviceid);
		ASSERT(!err);

		err = bcm_xdr_pack_uint32(&retb, revinfo.corerev);
		ASSERT(!err);

		err = bcm_xdr_pack_uint32(&retb, revinfo.boardrev);
		ASSERT(!err);

		err = bcm_xdr_pack_uint32(&retb, revinfo.sromrev);
		ASSERT(!err);

		err = bcm_xdr_pack_uint32(&retb, revinfo.chiprev);
		ASSERT(!err);

		err = bcm_xdr_pack_uint32(&retb, revinfo.chip);
		ASSERT(!err);

		err = bcm_xdr_pack_uint32(&retb, revinfo.chippkg);
		ASSERT(!err);

		err = bcm_xdr_pack_uint32(&retb, revinfo.boardtype);
		ASSERT(!err);

		err = bcm_xdr_pack_uint32(&retb, revinfo.boardvendor);
		ASSERT(!err);

		err = bcm_xdr_pack_uint32(&retb, revinfo.bustype);
		ASSERT(!err);

		err = bcm_xdr_pack_uint32(&retb, revinfo.buscoretype);
		ASSERT(!err);

		err = bcm_xdr_pack_uint32(&retb, revinfo.buscorerev);
		ASSERT(!err);

		err = bcm_xdr_pack_uint32(&retb, revinfo.issim);
		ASSERT(!err);

		err = bcm_xdr_pack_uint32(&retb, revinfo.nbands);
		ASSERT(!err);

		err = bcm_xdr_pack_uint32(&retb, revinfo.boardflags);
		ASSERT(!err);

		err = bcm_xdr_pack_uint32(&retb, revinfo.boardflags2);
		ASSERT(!err);

		err = bcm_xdr_pack_uint32(&retb, revinfo.boardflags4);
		ASSERT(!err);

		for (idx = 0; idx < revinfo.nbands; idx++) {
			/* So if this is a single band 11a card, use band 1 */
			if (IS_SINGLEBAND_5G(revinfo.deviceid))
				idx = BAND_5G_INDEX;

			err = bcm_xdr_pack_uint32(&retb, revinfo.band[idx].bandunit);
			ASSERT(!err);

			err = bcm_xdr_pack_uint32(&retb, revinfo.band[idx].bandtype);
			ASSERT(!err);

			err = bcm_xdr_pack_uint32(&retb, revinfo.band[idx].radiorev);
			ASSERT(!err);

			err = bcm_xdr_pack_uint32(&retb, revinfo.band[idx].phytype);
			ASSERT(!err);

			err = bcm_xdr_pack_uint32(&retb, revinfo.band[idx].phyrev);
			ASSERT(!err);

			err = bcm_xdr_pack_uint32(&retb, revinfo.band[idx].phy_minor_rev);
			ASSERT(!err);

			err = bcm_xdr_pack_uint32(&retb, revinfo.band[idx].anarev);
			ASSERT(!err);

			err = bcm_xdr_pack_uint32(&retb, revinfo.band[idx].radioid);
			ASSERT(!err);
		}

		err = bcm_xdr_pack_uint32(&retb, revinfo._wlsrvsdb);
		ASSERT(!err);

		err = bcm_xdr_pack_uint32(&retb, revinfo.ampdu_ba_rx_wsize);
		ASSERT(!err);

		err = bcm_xdr_pack_uint32(&retb, revinfo.host_rpc_agg_size);
		ASSERT(!err);

		err = bcm_xdr_pack_uint32(&retb, revinfo.ampdu_mpdu);
		ASSERT(!err);

		err = bcm_xdr_pack_uint32(&retb, revinfo.is_ss);
		ASSERT(!err);

		err = bcm_xdr_pack_uint32(&retb, revinfo.wowl_gpio);
		ASSERT(!err);

		err = bcm_xdr_pack_uint32(&retb, revinfo.wowl_gpiopol);
		ASSERT(!err);

		err = wlc_rpc_call_return(rpc, rpc_buf);
		ASSERT(!err);
		break;
	}

	case WLRPC_WLC_BMAC_STATE_GET_ID: {
		bcm_xdr_buf_t retb;
		rpc_buf_t *rpc_buf;
		wlc_bmac_state_t state;

		rpc_buf = wlc_rpc_buf_alloc(rpc, &retb,
			ROUNDUP(sizeof(wlc_bmac_state_t), sizeof(uint32)),
			WLRPC_WLC_BMAC_STATE_GET_ID);
		ASSERT(rpc_buf != NULL);

		wlc_bmac_state_get(wlc_hw, &state);

		err = bcm_xdr_pack_uint32(&retb, state.machwcap);
		ASSERT(!err);

		err = bcm_xdr_pack_uint32(&retb, state.preamble_ovr);
		ASSERT(!err);

		err = wlc_rpc_call_return(rpc, rpc_buf);
		ASSERT(!err);
		break;
	}
	case WLRPC_WLC_BMAC_XMTFIFO_SZ_GET_ID: {
		bcm_xdr_buf_t retb;
		rpc_buf_t *rpc_buf;
		uint fifo, blocks;
		int fn_err;

		err = bcm_xdr_unpack_uint32(&b, &fifo);
		ASSERT(!err);

		fn_err = wlc_bmac_xmtfifo_sz_get(wlc_hw, fifo, &blocks);

		rpc_buf = wlc_rpc_buf_alloc(rpc, &retb,
		                            sizeof(uint32) * 2,
		                            WLRPC_WLC_BMAC_XMTFIFO_SZ_GET_ID);
		ASSERT(rpc_buf != NULL);

		err = bcm_xdr_pack_uint32(&retb, blocks);
		ASSERT(!err);

		err = bcm_xdr_pack_int32(&retb, fn_err);
		ASSERT(!err);

		err = wlc_rpc_call_return(rpc, rpc_buf);
		ASSERT(!err);
		break;
	}

	case WLRPC_WLC_BMAC_XMTFIFO_SZ_SET_ID: {
		bcm_xdr_buf_t retb;
		rpc_buf_t *rpc_buf;
		uint fifo, blocks;
		int fn_err;

		err = bcm_xdr_unpack_uint32(&b, &fifo);
		ASSERT(!err);

		err = bcm_xdr_unpack_uint32(&b, &blocks);
		ASSERT(!err);

		fn_err = wlc_bmac_xmtfifo_sz_set(wlc_hw, fifo, blocks);

		rpc_buf = wlc_rpc_buf_alloc(rpc, &retb,
		                            sizeof(uint32),
		                            WLRPC_WLC_BMAC_XMTFIFO_SZ_SET_ID);
		ASSERT(rpc_buf != NULL);

		err = bcm_xdr_pack_int32(&retb, fn_err);
		ASSERT(!err);

		err = wlc_rpc_call_return(rpc, rpc_buf);
		ASSERT(!err);
		break;
	}

	case WLRPC_WLC_BMAC_VALIDATE_CHIP_ACCESS_ID: {
		bcm_xdr_buf_t retb;
		rpc_buf_t *rpc_buf;
		bool ret;

		ret = wlc_bmac_validate_chip_access(wlc_hw);

		rpc_buf = wlc_rpc_buf_alloc(rpc, &retb, sizeof(uint32),
		                            WLRPC_WLC_BMAC_VALIDATE_CHIP_ACCESS_ID);
		ASSERT(rpc_buf != NULL);

		err = bcm_xdr_pack_uint32(&retb, ret);
		ASSERT(!err);

		err = wlc_rpc_call_return(rpc, rpc_buf);
		ASSERT(!err);
		break;
	}

	case WLRPC_WLC_PHY_SET_CHANNEL_14_WIDE_FILTER_ID: {
		bool wide_filter;
		uint32 uret;

		err = bcm_xdr_unpack_uint32(&b, &uret);
		ASSERT(!err);

		wide_filter = (bool)uret;

		wlc_phy_chanspec_ch14_widefilter_set(((wlc_hw_info_t *)wlc_hw)->band->pi,
		                                   wide_filter);

		break;
	}

	case WLRPC_WLC_PHY_NOISE_AVG_ID: {
		bcm_xdr_buf_t retb;
		rpc_buf_t *rpc_buf;
		int8 noise;

		noise = wlc_phy_noise_avg(((wlc_hw_info_t *)wlc_hw)->band->pi);

		rpc_buf = wlc_rpc_buf_alloc(rpc, &retb,
		                            sizeof(uint32),
		                            WLRPC_WLC_PHY_NOISE_AVG_ID);
		ASSERT(rpc_buf != NULL);

		err = bcm_xdr_pack_int8(&retb, noise);
		ASSERT(!err);

		err = wlc_rpc_call_return(rpc, rpc_buf);
		ASSERT(!err);

		break;
	}

	case WLRPC_WLC_PHYCHAIN_INIT_ID: {
		uint8 txchain, rxchain;
		uint32 tmp;

		err = bcm_xdr_unpack_uint32(&b, &tmp);
		ASSERT(!err);
		txchain = (uint8)tmp;

		err = bcm_xdr_unpack_uint32(&b, &tmp);
		ASSERT(!err);
		rxchain = (uint8)tmp;

		wlc_phy_stf_chain_init(wlc_hw->band->pi, txchain, rxchain);

		break;
	}

	case WLRPC_WLC_PHYCHAIN_SET_ID: {
		uint8 txchain, rxchain;
		uint32 tmp;

		err = bcm_xdr_unpack_uint32(&b, &tmp);
		ASSERT(!err);
		txchain = (uint8)tmp;

		err = bcm_xdr_unpack_uint32(&b, &tmp);
		ASSERT(!err);
		rxchain = (uint8)tmp;

		wlc_phy_stf_chain_set(wlc_hw->band->pi, txchain, rxchain);

		break;
	}

	case WLRPC_WLC_PHYCHAIN_GET_ID: {
		bcm_xdr_buf_t retb;
		rpc_buf_t *rpc_buf;
		uint8 txchain, rxchain;

		wlc_phy_stf_chain_get(wlc_hw->band->pi, &txchain, &rxchain);

		rpc_buf = wlc_rpc_buf_alloc(rpc, &retb,
		                            2*sizeof(uint32),
		                            WLRPC_WLC_PHYCHAIN_GET_ID);
		ASSERT(rpc_buf != NULL);

		err = bcm_xdr_pack_uint32(&retb, (uint32)txchain);
		ASSERT(!err);

		err = bcm_xdr_pack_uint32(&retb, (uint32)rxchain);
		ASSERT(!err);

		err = wlc_rpc_call_return(rpc, rpc_buf);
		ASSERT(!err);

		break;
	}

	case WLRPC_WLC_PHY_TKIP_RIFS_WAR_ID: {
		uint8 rifs;
		uint32 tmp;

		err = bcm_xdr_unpack_uint32(&b, &tmp);
		ASSERT(!err);
		rifs = (uint16)tmp;

		wlc_phy_tkip_rifs_war(wlc_hw->band->pi, rifs);

		break;
	}

	case WLRPC_WLC_BMAC_RETRYLIMIT_UPD_ID: {
		uint16 SRL, LRL;
		uint32 ret;

		err = bcm_xdr_unpack_uint32(&b, &ret);
		ASSERT(!err);

		SRL = (uint16)ret;

		err = bcm_xdr_unpack_uint32(&b, &ret);
		ASSERT(!err);

		LRL = (uint16)ret;

		wlc_bmac_retrylimit_upd(wlc_hw, SRL, LRL);
		break;
	}

	case WLRPC_WLC_BMAC_SET_NORESET_ID : {
		bool noreset_flag;
		uint32 uret;

		err = bcm_xdr_unpack_uint32(&b, &uret);
		ASSERT(!err);

		noreset_flag = (bool)uret;
		wlc_bmac_set_noreset(wlc_hw, noreset_flag);
		break;
	}

	case WLRPC_WLC_BMAC_P2P_CAP_ID: {
		bcm_xdr_buf_t retb;
		rpc_buf_t *rpc_buf;
		bool ret;

		ret = wlc_bmac_p2p_cap(wlc_hw);

		rpc_buf = wlc_rpc_buf_alloc(rpc, &retb, sizeof(uint32), WLRPC_WLC_BMAC_P2P_CAP_ID);
		ASSERT(rpc_buf != NULL);

		err = bcm_xdr_pack_int32(&retb, ret);
		ASSERT(!err);

		err = wlc_rpc_call_return(rpc, rpc_buf);
		ASSERT(!err);

		break;
	}

	case WLRPC_WLC_BMAC_P2P_SET_ID : {
		bool enable;
		uint32 uret;
		bcm_xdr_buf_t retb;
		rpc_buf_t *rpc_buf;
		bool ret;

		err = bcm_xdr_unpack_uint32(&b, &uret);
		ASSERT(!err);

		enable = (bool)uret;
		ret = wlc_bmac_p2p_set(wlc_hw, enable);

		rpc_buf = wlc_rpc_buf_alloc(rpc, &retb, sizeof(uint32), WLRPC_WLC_BMAC_P2P_SET_ID);
		ASSERT(rpc_buf != NULL);

		err = bcm_xdr_pack_int32(&retb, ret);
		ASSERT(!err);

		err = wlc_rpc_call_return(rpc, rpc_buf);
		ASSERT(!err);

		break;
	}

	case WLRPC_WLC_BMAC_BTC_MODE_SET_ID: {
		bcm_xdr_buf_t retb;
		rpc_buf_t *rpc_buf;
		int btc_mode;
		int ret;

		err = bcm_xdr_unpack_int32(&b, &btc_mode);
		ASSERT(!err);

		ret = wlc_bmac_btc_mode_set(wlc_hw, btc_mode);

		rpc_buf = wlc_rpc_buf_alloc(rpc, &retb,
		                            sizeof(uint32),
		                            WLRPC_WLC_BMAC_BTC_MODE_SET_ID);
		ASSERT(rpc_buf != NULL);

		err = bcm_xdr_pack_int32(&retb, ret);
		ASSERT(!err);

		err = wlc_rpc_call_return(rpc, rpc_buf);
		ASSERT(!err);

		break;
	}

	case WLRPC_WLC_BMAC_BTC_MODE_GET_ID: {
		bcm_xdr_buf_t retb;
		rpc_buf_t *rpc_buf;
		int ret;

		ret = wlc_bmac_btc_mode_get(wlc_hw);

		rpc_buf = wlc_rpc_buf_alloc(rpc, &retb,
		                            sizeof(uint32),
		                            WLRPC_WLC_BMAC_BTC_MODE_GET_ID);
		ASSERT(rpc_buf != NULL);

		err = bcm_xdr_pack_int32(&retb, ret);
		ASSERT(!err);

		err = wlc_rpc_call_return(rpc, rpc_buf);
		ASSERT(!err);

		break;
	}

	case WLRPC_WLC_BMAC_BTC_WIRE_SET_ID: {
		bcm_xdr_buf_t retb;
		rpc_buf_t *rpc_buf;
		int btc_wire;
		int ret;

		err = bcm_xdr_unpack_int32(&b, &btc_wire);
		ASSERT(!err);

		ret = wlc_bmac_btc_wire_set(wlc_hw, btc_wire);

		rpc_buf = wlc_rpc_buf_alloc(rpc, &retb,
		                            sizeof(uint32),
		                            WLRPC_WLC_BMAC_BTC_WIRE_SET_ID);
		ASSERT(rpc_buf != NULL);

		err = bcm_xdr_pack_int32(&retb, ret);
		ASSERT(!err);

		err = wlc_rpc_call_return(rpc, rpc_buf);
		ASSERT(!err);

		break;
	}

	case WLRPC_WLC_BMAC_BTC_WIRE_GET_ID: {
		bcm_xdr_buf_t retb;
		rpc_buf_t *rpc_buf;
		int ret;

		ret = wlc_bmac_btc_wire_get(wlc_hw);

		rpc_buf = wlc_rpc_buf_alloc(rpc, &retb,
		                            sizeof(uint32),
		                            WLRPC_WLC_BMAC_BTC_WIRE_GET_ID);
		ASSERT(rpc_buf != NULL);

		err = bcm_xdr_pack_int32(&retb, ret);
		ASSERT(!err);

		err = wlc_rpc_call_return(rpc, rpc_buf);
		ASSERT(!err);

		break;
	}

	case WLRPC_WLC_BMAC_BTC_FLAGS_IDX_SET_ID: {
		bcm_xdr_buf_t retb;
		rpc_buf_t *rpc_buf;
		int val1, val2;
		int ret;

		err = bcm_xdr_unpack_int32(&b, &val1);
		ASSERT(!err);

		err = bcm_xdr_unpack_int32(&b, &val2);
		ASSERT(!err);

		ret = wlc_bmac_btc_flags_idx_set(wlc_hw, val1, val2);

		rpc_buf = wlc_rpc_buf_alloc(rpc, &retb,
		                            sizeof(uint32),
		                            WLRPC_WLC_BMAC_BTC_FLAGS_IDX_SET_ID);
		ASSERT(rpc_buf != NULL);

		err = bcm_xdr_pack_int32(&retb, ret);
		ASSERT(!err);

		err = wlc_rpc_call_return(rpc, rpc_buf);
		ASSERT(!err);

		break;
	}

	case WLRPC_WLC_BMAC_BTC_FLAGS_IDX_GET_ID: {
		bcm_xdr_buf_t retb;
		rpc_buf_t *rpc_buf;
		int val1;
		int ret;

		err = bcm_xdr_unpack_int32(&b, &val1);
		ASSERT(!err);

		ret = wlc_bmac_btc_flags_idx_get(wlc_hw, val1);

		rpc_buf = wlc_rpc_buf_alloc(rpc, &retb,
			sizeof(uint32),
			WLRPC_WLC_BMAC_BTC_FLAGS_IDX_GET_ID);
		ASSERT(rpc_buf != NULL);

		err = bcm_xdr_pack_int32(&retb, ret);
		ASSERT(!err);

		err = wlc_rpc_call_return(rpc, rpc_buf);
		ASSERT(!err);

		break;
	}

	case WLRPC_WLC_BMAC_BTC_FLAGS_GET_ID: {
		bcm_xdr_buf_t retb;
		rpc_buf_t *rpc_buf;
		int ret;

		ret = wlc_bmac_btc_flags_get(wlc_hw);

		rpc_buf = wlc_rpc_buf_alloc(rpc, &retb,
		                            sizeof(uint32),
		                            WLRPC_WLC_BMAC_BTC_FLAGS_GET_ID);
		ASSERT(rpc_buf != NULL);

		err = bcm_xdr_pack_int32(&retb, ret);
		ASSERT(!err);

		err = wlc_rpc_call_return(rpc, rpc_buf);
		ASSERT(!err);

		break;
	}

	case WLRPC_WLC_BMAC_BTC_PARAMS_SET_ID: {
		bcm_xdr_buf_t retb;
		rpc_buf_t *rpc_buf;
		int val1, val2;
		int ret;

		err = bcm_xdr_unpack_int32(&b, &val1);
		ASSERT(!err);

		err = bcm_xdr_unpack_int32(&b, &val2);
		ASSERT(!err);

		ret = wlc_bmac_btc_params_set(wlc_hw, val1, val2);

		rpc_buf = wlc_rpc_buf_alloc(rpc, &retb,
			sizeof(uint32),	WLRPC_WLC_BMAC_BTC_PARAMS_SET_ID);
		ASSERT(rpc_buf != NULL);

		err = bcm_xdr_pack_int32(&retb, ret);
		ASSERT(!err);

		err = wlc_rpc_call_return(rpc, rpc_buf);
		ASSERT(!err);

		break;
	}

	case WLRPC_WLC_BMAC_BTC_PARAMS_GET_ID: {
		bcm_xdr_buf_t retb;
		rpc_buf_t *rpc_buf;
		int val1;
		int ret;

		err = bcm_xdr_unpack_int32(&b, &val1);
		ASSERT(!err);

		ret = wlc_bmac_btc_params_get(wlc_hw, val1);

		rpc_buf = wlc_rpc_buf_alloc(rpc, &retb,
			sizeof(uint32), WLRPC_WLC_BMAC_BTC_PARAMS_GET_ID);
		ASSERT(rpc_buf != NULL);

		err = bcm_xdr_pack_int32(&retb, ret);
		ASSERT(!err);

		err = wlc_rpc_call_return(rpc, rpc_buf);
		ASSERT(!err);

		break;
	}

	case WLRPC_WLC_BMAC_BTC_PERIOD_GET_ID: {
		bcm_xdr_buf_t retb;
		rpc_buf_t *rpc_buf;
		uint16 val;
		bool btactive;
		uint16 agg_off_bm;

		rpc_buf = wlc_rpc_buf_alloc(rpc, &retb,
			2 * sizeof(uint32), WLRPC_WLC_BMAC_BTC_PERIOD_GET_ID);
		ASSERT(rpc_buf != NULL);

		wlc_bmac_btc_period_get(wlc_hw, &val, &btactive, &agg_off_bm);

		err = bcm_xdr_pack_int32(&retb, val);
		ASSERT(!err);

		err = bcm_xdr_pack_int32(&retb, (uint32)btactive);
		ASSERT(!err);

		err = wlc_rpc_call_return(rpc, rpc_buf);
		ASSERT(!err);

		break;
	}

	case WLRPC_WLC_BMAC_BTC_STUCKWAR_ID: {
		uint32 uret;

		err = bcm_xdr_unpack_uint32(&b, &uret);
		ASSERT(!err);

		wlc_bmac_btc_stuck_war50943(wlc_hw, (bool)uret);
		break;
	}

	case WLRPC_WLC_BMAC_BTC_RSSI_THRESHOLD_GET_ID: {
		bcm_xdr_buf_t retb;
		rpc_buf_t *rpc_buf;
		uint8 val1, val2, val3;

		rpc_buf = wlc_rpc_buf_alloc(rpc, &retb,
			3 * sizeof(uint32), WLRPC_WLC_BMAC_BTC_RSSI_THRESHOLD_GET_ID);
		ASSERT(rpc_buf != NULL);

		wlc_bmac_btc_rssi_threshold_get(wlc_hw, &val1, &val2, &val3);

		err = bcm_xdr_pack_int32(&retb, val1);
		ASSERT(!err);

		err = bcm_xdr_pack_int32(&retb, val2);
		ASSERT(!err);

		err = bcm_xdr_pack_int32(&retb, val3);
		ASSERT(!err);

		err = wlc_rpc_call_return(rpc, rpc_buf);
		ASSERT(!err);

		break;
	}

	case WLRPC_WLC_BMAC_FIFOERRORS_ID:
		wlc_bmac_fifoerrors(wlc_hw);
		break;

	case WLRPC_WLC_BMAC_AMPDU_SET_ID: {
		uint ret;
		uint8 mode;

		err = bcm_xdr_unpack_uint32(&b, &ret);
		ASSERT(!err);
		mode = (uint8)ret;

		wlc_bmac_ampdu_set(wlc_hw, mode);
		break;
	}

	case WLRPC_WLC_PHY_TXPOWER_GET_TARGET_MIN_ID: {
		bcm_xdr_buf_t retb;
		rpc_buf_t *rpc_buf;
		uint32 ret;

		ret = wlc_phy_txpower_get_target_min(wlc_hw->band->pi);

		rpc_buf = wlc_rpc_buf_alloc(rpc, &retb,
			sizeof(uint32), WLRPC_WLC_PHY_TXPOWER_GET_TARGET_MIN_ID);
		ASSERT(rpc_buf != NULL);

		err = bcm_xdr_pack_uint32(&retb, ret);
		ASSERT(!err);

		err = wlc_rpc_call_return(rpc, rpc_buf);
		ASSERT(!err);

		break;
	}

	case WLRPC_WLC_PHY_TXPOWER_GET_TARGET_MAX_ID: {
		bcm_xdr_buf_t retb;
		rpc_buf_t *rpc_buf;
		uint32 ret;

		ret = wlc_phy_txpower_get_target_max(wlc_hw->band->pi);

		rpc_buf = wlc_rpc_buf_alloc(rpc, &retb,
		                            sizeof(uint32),
		                            WLRPC_WLC_PHY_TXPOWER_GET_TARGET_MAX_ID);
		ASSERT(rpc_buf != NULL);

		err = bcm_xdr_pack_uint32(&retb, ret);
		ASSERT(!err);

		err = wlc_rpc_call_return(rpc, rpc_buf);
		ASSERT(!err);

		break;
	}

#ifdef WLLED
	case WLRPC_WLC_BMAC_LED_HW_DEINIT_ID: {
		uint32 mask;

		err = bcm_xdr_unpack_uint32(&b, &mask);
		ASSERT(!err);

		wlc_bmac_led_hw_deinit(wlc_hw, mask);

		break;
	}

	case WLRPC_WLC_BMAC_LED_HW_MASK_INIT_ID: {
		uint32 mask;

		err = bcm_xdr_unpack_uint32(&b, &mask);
		ASSERT(!err);

		wlc_bmac_led_hw_mask_init(wlc_hw, mask);

		break;
	}
#endif /* WLLED */
	case WLRPC_WLC_PLLREQ_ID: {
		int set;
		uint req_bit;

		err = bcm_xdr_unpack_int32(&b, &set);
		ASSERT(!err);

		err = bcm_xdr_unpack_uint32(&b, &req_bit);
		ASSERT(!err);

		wlc_bmac_pllreq(wlc_hw, (bool)set, req_bit);

		break;
	}
	case WLRPC_WLC_BMAC_TACLEAR_ID: {
		int ta_ok;
		bcm_xdr_buf_t retb;
		rpc_buf_t *rpc_buf;
		bool ret;

		err = bcm_xdr_unpack_int32(&b, &ta_ok);
		ASSERT(!err);

#ifdef BCMASSERT_SUPPORT
		ret = wlc_bmac_taclear(wlc_hw, (bool)ta_ok);
#else
		ret = TRUE;
#endif // endif

		rpc_buf = wlc_rpc_buf_alloc(rpc, &retb,
		                            sizeof(int32),
		                            WLRPC_WLC_BMAC_TACLEAR_ID);
		ASSERT(rpc_buf != NULL);

		err = bcm_xdr_pack_int32(&retb, (int)ret);
		ASSERT(!err);

		err = wlc_rpc_call_return(rpc, rpc_buf);
		ASSERT(!err);

		break;
	}

	case WLRPC_WLC_BMAC_SET_CLK_ID: {
		int on;

		err = bcm_xdr_unpack_int32(&b, &on);
		ASSERT(!err);

		wlc_bmac_set_clk(wlc_hw, (bool)on);

		break;
	}

	case WLRPC_WLC_PHY_OFDM_RATESET_WAR_ID: {
		int on;

		err = bcm_xdr_unpack_int32(&b, &on);
		ASSERT(!err);

		wlc_phy_ofdm_rateset_war(wlc_hw->band->pi, (bool)on);

		break;
	}

	case WLRPC_WLC_PHY_BF_PREEMPT_ENABLE_ID: {
		int bf_preempt;

		err = bcm_xdr_unpack_int32(&b, &bf_preempt);
		ASSERT(!err);

		wlc_phy_bf_preempt_enable(wlc_hw->band->pi, (bool)bf_preempt);

		break;
	}

	case WLRPC_WLC_BMAC_DUMP_ID: {
		uint32 len, namelen;
		uint init_len, final_len, adj_len, actual_len = 0;
		char *name;
		bcm_xdr_buf_t retb;
		rpc_buf_t *rpc_buf;
		rpc_tp_info_t *rpc_tp;
		struct bcmstrbuf strb;

		err = bcm_xdr_unpack_uint32(&b, &namelen);
		ASSERT(!err);
		err = bcm_xdr_unpack_opaque(&b, namelen, (void **)&name);
		ASSERT(!err);

		err = bcm_xdr_unpack_uint32(&b, &len);
		ASSERT(!err);

		if (len > 1024)
			len = 1024;

		init_len = sizeof(int32) + ROUNDUP(len, sizeof(uint32)),
		rpc_buf = wlc_rpc_buf_alloc(rpc, &retb,	init_len,
			WLRPC_WLC_BMAC_DUMP_ID);
		ASSERT(rpc_buf != NULL);

		/* init the strb to be sizeof(uint32) from the beginning */
		/* of retb->buf */
		bcm_binit(&strb, (char*)retb.buf + sizeof(uint32), len);

		wlc_bmac_dump(wlc_hw, name, &strb);

		/* only need to pack the real len used */
		actual_len = strb.origsize - strb.size;
		err = bcm_xdr_pack_uint32(&retb, actual_len);
		ASSERT(!err);

		final_len = sizeof(int32) + ROUNDUP(actual_len, sizeof(uint32));
		adj_len = init_len - final_len;
		rpc_tp = bcm_rpc_tp_get(rpc);
		bcm_rpc_buf_len_set(rpc_tp, rpc_buf,
			bcm_rpc_buf_len_get(rpc_tp, rpc_buf) - adj_len);

		err = wlc_rpc_call_return(rpc, rpc_buf);
		ASSERT(!err);

		break;
	}

	case WLRPC_WLC_BMAC_PKTENG_ID: {
		uint32 ret = BCME_UNSUPPORTED;
		rpc_buf_t *rpc_buf;
		bcm_xdr_buf_t retb;
#if (defined(WLTEST) || defined(WLPKTENG))
		void *p;
		uint32 totlen;
		wl_pkteng_t *pkteng;

		/* Unpack cis */
		err = bcm_xdr_unpack_opaque(&b, sizeof(wl_pkteng_t), (void*)&pkteng);
		ASSERT(!err);

		err = bcm_xdr_unpack_uint32(&b, &totlen);
		ASSERT(!err);

		ret = 0;
		if (totlen > 0) {
			free_buf = FALSE;

			if (buf != NULL) {
				p = wlc_rpc_bmac_txfifo_buf_to_txpkt(rpc, wlc_hw->osh, &b, buf,
					totlen);
				if (p == NULL) {
					WL_ERROR(("%s: Pkt alloc failed : totlen %d\n",
						__FUNCTION__, totlen));
					ret = BCME_NOMEM;
				}
			} else {
				WL_ERROR(("%s: buf is null, skip\n", __FUNCTION__));
				ret = BCME_BUFTOOSHORT;
			}
		} else {
			p = NULL;
		}

		if (ret == 0)
			ret = wlc_bmac_pkteng(wlc_hw, pkteng, p);

#endif // endif
		rpc_buf = wlc_rpc_buf_alloc(rpc, &retb, sizeof(int32), WLRPC_WLC_BMAC_PKTENG_ID);
		ASSERT(rpc_buf != NULL);

		err = bcm_xdr_pack_int32(&retb, ret);
		ASSERT(!err);

		err = wlc_rpc_call_return(rpc, rpc_buf);
		ASSERT(!err);

		break;
	}
#if (defined(BCMNVRAMR) || defined(BCMNVRAMW)) && defined(WLTEST)
#ifdef BCMNVRAMW
	case WLRPC_WLC_CISWRITE_ID: {
		int len = 0, ret = 0;
		bcm_xdr_buf_t retb;
		rpc_buf_t *rpc_buf;
		cis_rw_t *cis = NULL;
		uint16 *tbuf = NULL;

		/* Unpack len and tbuf */
		err = bcm_xdr_unpack_int32(&b, &len);
		ASSERT(!err);
		err = bcm_xdr_unpack_opaque(&b, len, (void*)&tbuf);
		ASSERT(!err);

		/* Unpack cis */
		err = bcm_xdr_unpack_opaque(&b, sizeof(cis_rw_t), (void*)&cis);
		ASSERT(!err);

		ret = wlc_bmac_ciswrite(wlc_hw, cis, tbuf, len);

		rpc_buf = wlc_rpc_buf_alloc(rpc, &retb, sizeof(int32), WLRPC_WLC_CISWRITE_ID);
		ASSERT(rpc_buf != NULL);

		err = bcm_xdr_pack_int32(&retb, ret);
		ASSERT(!err);

		err = wlc_rpc_call_return(rpc, rpc_buf);
		ASSERT(!err);

		break;
	}
#endif /* def BCMNVRAMW */

	case WLRPC_WLC_CISDUMP_ID: {
		int len = 0, ret = 0;
		bcm_xdr_buf_t retb;
		rpc_buf_t *rpc_buf;
		cis_rw_t *cis = NULL;
		uint16 *tbuf = NULL;

		/* Unpack len */
		err = bcm_xdr_unpack_int32(&b, &len);
		ASSERT(!err);

		/* Unpack cis */
		err = bcm_xdr_unpack_opaque(&b, sizeof(cis_rw_t), (void*)&cis);
		ASSERT(!err);

		rpc_buf = wlc_rpc_buf_alloc(rpc, &retb, sizeof(int32) +
			+ sizeof(cis_rw_t) + SROM_MAX, WLRPC_WLC_CISDUMP_ID);
		ASSERT(rpc_buf != NULL);

		if ((tbuf = (uint16*)MALLOC(wlc_hw->osh, SROM_MAX)) == NULL) {
			/* If no memory, return error to the high driver */
			ret = BCME_NOMEM;
			err = bcm_xdr_pack_int32(&retb, ret);
			ASSERT(!err);

			err = wlc_rpc_call_return(rpc, rpc_buf);
			ASSERT(!err);
			break;
		}
		memset(tbuf, 0, SROM_MAX);

		ret = wlc_bmac_cisdump(wlc_hw, cis, tbuf, len);

		err = bcm_xdr_pack_int32(&retb, ret);
		ASSERT(!err);

		err = bcm_xdr_pack_opaque(&retb, sizeof(cis_rw_t), (void*)cis);
		ASSERT(!err);

		err = bcm_xdr_pack_opaque(&retb, SROM_MAX, tbuf);
		ASSERT(!err);

		err = wlc_rpc_call_return(rpc, rpc_buf);
		ASSERT(!err);

		MFREE(wlc_hw->osh, tbuf, SROM_MAX);
		break;
	}
#endif // endif
#ifdef WLLED
	case WLRPC_WLC_BMAC_LED_BLINK_EVENT_ID: {
		int blink;

		err = bcm_xdr_unpack_int32(&b, &blink);
		ASSERT(!err);

		wlc_bmac_led_blink_event(wlc_hw, (bool)blink);

		break;
	}

	case WLRPC_WLC_BMAC_LED_SET_ID: {
		int index;
		int activehi;

		err = bcm_xdr_unpack_int32(&b, &index);
		ASSERT(!err);

		err = bcm_xdr_unpack_int32(&b, &activehi);
		ASSERT(!err);

		wlc_bmac_led_set(wlc_hw, index, (bool)activehi);

		break;
	}

	case WLRPC_WLC_BMAC_LED_BLINK_ID: {
		int index;
		int msec_on;
		int msec_off;

		err = bcm_xdr_unpack_int32(&b, &index);
		ASSERT(!err);

		err = bcm_xdr_unpack_int32(&b, &msec_on);
		ASSERT(!err);

		err = bcm_xdr_unpack_int32(&b, &msec_off);
		ASSERT(!err);

		wlc_bmac_led_blink(wlc_hw, index, msec_on, msec_off);

		break;
	}

	case WLRPC_WLC_BMAC_LED_ID: {
		uint32 mask;
		uint32 val;
		int activehi;

		err = bcm_xdr_unpack_uint32(&b, &mask);
		ASSERT(!err);

		err = bcm_xdr_unpack_uint32(&b, &val);
		ASSERT(!err);

		err = bcm_xdr_unpack_int32(&b, &activehi);
		ASSERT(!err);

		wlc_bmac_led(wlc_hw, mask, val, (bool)activehi);

		break;
	}

#endif /* WLLED */

	case WLRPC_SI_ISCORE_UP_ID: {
		bool ret;
		bcm_xdr_buf_t retb;
		rpc_buf_t *rpc_buf;

		ret = si_iscoreup(wlc_hw->sih);

		rpc_buf = wlc_rpc_buf_alloc(rpc, &retb, sizeof(int32), rpc_id);
		ASSERT(rpc_buf != NULL);

		err = bcm_xdr_pack_int32(&retb, (int)ret);
		ASSERT(!err);

		err = wlc_rpc_call_return(rpc, rpc_buf);
		ASSERT(!err);

		break;
	}
#ifdef AP
	case WLRPC_WLC_BMAC_PS_SWITCH_ID: {
		struct ether_addr ea;
		struct ether_addr *ea_p = NULL;
		uint32 ea_null;
		int8 ps_flags;

		err = bcm_xdr_unpack_int8(&b, &ps_flags);
		err = bcm_xdr_unpack_uint32(&b, &ea_null);
		ASSERT(!err);

		if (ea_null)
			ea_p = NULL;
		else {
			ea_p = &ea;
			err = bcm_xdr_unpack_opaque_cpy(&b, sizeof(struct ether_addr), ea_p);
			ASSERT(!err);
		}

		wlc_bmac_process_ps_switch(wlc_hw, ea_p, ps_flags, NULL);

		break;
	}

	#endif /* AP */

	case WLRPC_WLC_BMAC_RATE_SHM_OFFSET_ID : {
		uint16 ret;
		bcm_xdr_buf_t retb;
		rpc_buf_t *rpc_buf;
		uint32 rate;

		err = bcm_xdr_unpack_uint32(&b, &rate);
		ASSERT(!err);
		ret = wlc_bmac_rate_shm_offset(wlc_hw, (uint8)rate);

		rpc_buf = wlc_rpc_buf_alloc(rpc, &retb, sizeof(int32),
		                            WLRPC_WLC_BMAC_RATE_SHM_OFFSET_ID);

		ASSERT(rpc_buf != NULL);

		err = bcm_xdr_pack_uint32(&retb, (uint32)ret);
		ASSERT(!err);

		err = wlc_rpc_call_return(rpc, rpc_buf);
		ASSERT(!err);
		break;
	}

	case WLRPC_WLC_BMAC_STF_SET_RATE_SHM_OFFSET_ID : {
		uint32 rate;
		uint32 pos;
		uint32 count;
		uint32 val;
		uint32 mask;
		wlc_stf_rs_shm_offset_t stf_rs;
		uint32 idx;
		wlc_rateset_t rs;

		err = bcm_xdr_unpack_uint32(&b, &pos);
		ASSERT(!err);
		err = bcm_xdr_unpack_uint32(&b, &mask);
		ASSERT(!err);
		err = bcm_xdr_unpack_uint32(&b, &count);
		ASSERT(!err);
		rs.count = (uint) count;

		for (idx = 0; idx < count; idx ++) {
			err = bcm_xdr_unpack_uint32(&b, &rate);
			ASSERT(!err);
			stf_rs.rate[idx] = (uint8) rate;

			err = bcm_xdr_unpack_uint32(&b, &val);
			ASSERT(!err);
			stf_rs.val[idx] = (uint16) val;
		}
		wlc_bmac_stf_set_rateset_shm_offset(wlc_hw, count, pos, mask, &stf_rs);

		break;
	}

	case WLRPC_WLC_PHY_STF_SSMODE_GET_ID : {
		int8 ret;
		bcm_xdr_buf_t retb;
		rpc_buf_t *rpc_buf;
		uint32 chanspec;

		err = bcm_xdr_unpack_uint32(&b, &chanspec);
		ASSERT(!err);
		ret = wlc_phy_stf_ssmode_get(wlc_hw->band->pi, (chanspec_t)chanspec);

		rpc_buf = wlc_rpc_buf_alloc(rpc, &retb, sizeof(int32),
		                            WLRPC_WLC_PHY_STF_SSMODE_GET_ID);

		ASSERT(rpc_buf != NULL);

		err = bcm_xdr_pack_uint32(&retb, (uint32)ret);
		ASSERT(!err);

		err = wlc_rpc_call_return(rpc, rpc_buf);
		ASSERT(!err);
		break;
	}

	case WLRPC_WLC_PHY_TXPOWER_IPA_ISON: {
		int8 ret;
		bcm_xdr_buf_t retb;
		rpc_buf_t *rpc_buf;

		ret = wlc_phy_txpower_ipa_ison(wlc_hw->band->pi);

		rpc_buf = wlc_rpc_buf_alloc(rpc, &retb, sizeof(int32),
			WLRPC_WLC_PHY_TXPOWER_IPA_ISON);

		ASSERT(rpc_buf != NULL);

		err = bcm_xdr_pack_uint32(&retb, (uint32)ret);
		ASSERT(!err);

		err = wlc_rpc_call_return(rpc, rpc_buf);
		ASSERT(!err);
		break;
	}
#ifdef WLEXTLOG
#ifdef WLC_LOW_ONLY
	case WLRPC_WLC_EXTLOG_CFG_ID: {
		uint32 val;
		wlc_extlog_cfg_t cfg;

		err = bcm_xdr_unpack_uint32(&b, &val);
		ASSERT(!err);
		cfg.module = (uint16)val;

		err = bcm_xdr_unpack_uint32(&b, &val);
		ASSERT(!err);
		cfg.level = (uint8)val;

		err = bcm_xdr_unpack_uint32(&b, &val);
		ASSERT(!err);
		cfg.flag = (uint8)val;

		wlc_bmac_extlog_cfg_set(wlc_hw, &cfg);

		break;
	}
#endif /* WLC_LOW_ONLY */
#endif /* WLEXTLOG */

	case WLRPC_BCM_ASSERT_TYPE_ID: {
		uint32 val;

		err = bcm_xdr_unpack_uint32(&b, &val);
		ASSERT(!err);

		g_assert_type = val;

		break;
	}
#ifdef PHYCAL_CACHING
	case WLRPC_WLC_BMAC_SET_PHYCAL_CACHE_FLAG_ID: {
		bool state;
		uint32 uret;

		err = bcm_xdr_unpack_uint32(&b, &uret);
		ASSERT(!err);

		state = (bool)uret;

		wlc_bmac_set_phycal_cache_flag(wlc_hw, state);
		break;
	}

	case WLRPC_WLC_BMAC_GET_PHYCAL_CACHE_FLAG_ID: {
		int8 ret;
		bcm_xdr_buf_t retb;
		rpc_buf_t *rpc_buf;

		ret = wlc_bmac_get_phycal_cache_flag(wlc_hw);

		rpc_buf = wlc_rpc_buf_alloc(rpc, &retb, sizeof(int32),
		                            WLRPC_WLC_BMAC_GET_PHYCAL_CACHE_FLAG_ID);
		ASSERT(rpc_buf != NULL);

		err = bcm_xdr_pack_uint32(&retb, (uint32)ret);
		ASSERT(!err);

		err = wlc_rpc_call_return(rpc, rpc_buf);
		ASSERT(!err);
		break;
	}
	case WLRPC_WLC_PHY_CAL_CACHE_INIT_ID: {
		bcm_xdr_buf_t retb;
		rpc_buf_t *rpc_buf;
		int ret;

		ret = wlc_phy_cal_cache_init(((wlc_hw_info_t *)wlc_hw)->band->pi);
		rpc_buf = wlc_rpc_buf_alloc(rpc, &retb, sizeof(int32),
		                            WLRPC_WLC_PHY_CAL_CACHE_INIT_ID);
		ASSERT(rpc_buf != NULL);

		err = bcm_xdr_pack_uint32(&retb, (uint32)ret);
		ASSERT(!err);

		err = wlc_rpc_call_return(rpc, rpc_buf);
		ASSERT(!err);
		break;
	}

	case WLRPC_WLC_PHY_CAL_CACHE_DEINIT_ID: {
		wlc_phy_cal_cache_deinit(((wlc_hw_info_t *)wlc_hw)->band->pi);
		break;
	}

#endif /* PHYCAL_CACHING */

	case WLRPC_WLC_BMAC_SET_TXPWR_PERCENT_ID: {
		uint32 val;

		err = bcm_xdr_unpack_uint32(&b, &val);
		ASSERT(!err);

		wlc_bmac_set_txpwr_percent(wlc_hw, (uint8)val);

		break;
	}

	case WLRPC_WLC_PHYCHAIN_ACTIVE_GET_ID : {
		int8 ret;
		bcm_xdr_buf_t retb;
		rpc_buf_t *rpc_buf;

		ret = wlc_phy_stf_chain_active_get(wlc_hw->band->pi);

		rpc_buf = wlc_rpc_buf_alloc(rpc, &retb, sizeof(int32),
		                            WLRPC_WLC_PHYCHAIN_ACTIVE_GET_ID);

		ASSERT(rpc_buf != NULL);

		err = bcm_xdr_pack_uint32(&retb, (uint32)ret);
		ASSERT(!err);

		err = wlc_rpc_call_return(rpc, rpc_buf);
		ASSERT(!err);
		break;
	}

#ifdef WLLED
	case WLRPC_WLC_BMAC_BLINK_SYNC_ID: {
		uint32 blink_pins;

		err = bcm_xdr_unpack_uint32(&b, &blink_pins);
		ASSERT(!err);

		wlc_bmac_blink_sync(wlc_hw, blink_pins);

		break;
	}
#endif /* WLLED */

	case WLRPC_WLC_BMAC_IFSCTL_EDCRS_SET_ID: {
		uint32 isht;

		err = bcm_xdr_unpack_uint32(&b, &isht);
		ASSERT(!err);

		wlc_bmac_ifsctl_edcrs_set(wlc_hw, (bool)isht);

		break;
	}

	case WLRPC_WLC_BMAC_UCODE_DBGSEL_GET_ID: {
		break;
	}

	case WLRPC_WLC_BMAC_UCODE_DBGSEL_SET_ID: {
		break;
	}

	case WLRPC_WLC_BMAC_CCA_STATS_READ_ID: {
		bcm_xdr_buf_t retb;
		rpc_buf_t *rpc_buf;
		cca_ucode_counts_t tmp;

		wlc_bmac_cca_stats_read(wlc_hw, &tmp);

		rpc_buf = wlc_rpc_buf_alloc(rpc, &retb, ROUNDUP(sizeof(cca_ucode_counts_t),
			sizeof(uint32)), WLRPC_WLC_BMAC_CCA_STATS_READ_ID);
		ASSERT(rpc_buf != NULL);

		err =  bcm_xdr_pack_opaque(&retb, sizeof(cca_ucode_counts_t), (void*)&tmp);
		ASSERT(!err);

		err = wlc_rpc_call_return(rpc, rpc_buf);
		ASSERT(!err);

		break;
	}
	case WLRPC_WLC_BMAC_ANTSEL_SET_ID: {
		uint32 antsel_avail;
		err = bcm_xdr_unpack_uint32(&b, &antsel_avail);
		ASSERT(!err);
		wlc_bmac_antsel_set(wlc_hw, antsel_avail);
		break;
	}

	case WLRPC_WLC_PHY_LDPC_SET_ID: {
		int8 ldpc;

		err = bcm_xdr_unpack_int8(&b, &ldpc);
		ASSERT(!err);

		wlc_phy_ldpc_override_set(((wlc_hw_info_t *)wlc_hw)->band->pi, (bool)ldpc);
		break;
	}

	case WLRPC_WLC_BMAC_DNGL_WD_KEEP_ALIVE_ID: {
		bcm_xdr_buf_t retb;
		rpc_buf_t *rpc_buf;
		uint32 delay;

		err = bcm_xdr_unpack_uint32(&b, &delay);
		ASSERT(!err);

		si_watchdog_ms(wlc_hw->sih, delay);

		rpc_buf = wlc_rpc_buf_alloc(rpc, &retb, 0, WLRPC_WLC_BMAC_DNGL_WD_KEEP_ALIVE_ID);
		ASSERT(rpc_buf != NULL);

		err = wlc_rpc_call_return(rpc, rpc_buf);
		ASSERT(!err);

		break;
	}

	case WLRPC_WLC_BMAC_WOWL_UCODE_ID: {

#ifdef WOWL
		rpc_buf_t *rpc_buf;
		bcm_xdr_buf_t retb;
		bool ret;
		int32 val;

		err = bcm_xdr_unpack_int32(&b, &val);
		ASSERT(!err);
		wlc_hw->wowl_gpio = (uint8) val;

		err = bcm_xdr_unpack_int32(&b, &val);
		ASSERT(!err);
		wlc_hw->wowl_gpiopol = (bool) val;

		ret = wlc_bmac_wowlucode_init(wlc_hw);

		rpc_buf = wlc_rpc_buf_alloc(rpc, &retb, sizeof(uint32),
			WLRPC_WLC_BMAC_WOWL_UCODE_ID);

		ASSERT(rpc_buf != NULL);

		err = bcm_xdr_pack_int8(&retb, ret);
		ASSERT(!err);

		err = wlc_rpc_call_return(rpc, rpc_buf);
		ASSERT(!err);
#endif /* WOWL */
		break;

	}

	case WLRPC_WLC_BMAC_WOWL_UCODESTART_ID: {

#ifdef WOWL
		rpc_buf_t *rpc_buf;
		bcm_xdr_buf_t retb;
		bool ret;

		ret = wlc_bmac_wowlucode_start(wlc_hw);

		rpc_buf = wlc_rpc_buf_alloc(rpc, &retb, sizeof(uint32),
			WLRPC_WLC_BMAC_WOWL_UCODESTART_ID);

		ASSERT(rpc_buf != NULL);

		err = bcm_xdr_pack_int8(&retb, ret);
		ASSERT(!err);

		err = wlc_rpc_call_return(rpc, rpc_buf);
		ASSERT(!err);
#endif /* WOWL */
		break;

	}

	case WLRPC_WLC_BMAC_WOWL_INITS_ID: {

#ifdef WOWL
		rpc_buf_t *rpc_buf;
		bcm_xdr_buf_t retb;
		int32 len;
		bool ret;
		void *inits;

		err = bcm_xdr_unpack_int32(&b, &len);
		ASSERT(!err);

		err = bcm_xdr_unpack_opaque(&b, len, &inits);
		ASSERT(!err);

		ret = wlc_bmac_write_inits(wlc_hw, inits, len);

		rpc_buf = wlc_rpc_buf_alloc(rpc, &retb, sizeof(uint32),
			WLRPC_WLC_BMAC_WOWL_INITS_ID);

		ASSERT(rpc_buf != NULL);

		err = bcm_xdr_pack_int8(&retb, ret);
		ASSERT(!err);

		err = wlc_rpc_call_return(rpc, rpc_buf);
		ASSERT(!err);

#endif /* WOWL */
		break;

	}

	case WLRPC_WLC_BMAC_WOWL_DNLDDONE_ID: {

#ifdef WOWL
		rpc_buf_t *rpc_buf;
		bcm_xdr_buf_t retb;
		bool ret;

		ret = wlc_bmac_wakeucode_dnlddone(wlc_hw);

		rpc_buf = wlc_rpc_buf_alloc(rpc, &retb, sizeof(uint32),
			WLRPC_WLC_BMAC_WOWL_DNLDDONE_ID);

		ASSERT(rpc_buf != NULL);

		err = bcm_xdr_pack_int8(&retb, ret);
		ASSERT(!err);

		err = wlc_rpc_call_return(rpc, rpc_buf);
		ASSERT(!err);
#endif /* WOWL */
		break;
	}

	case WLRPC_WL_WOWL_DNGLDOWN: {

#ifdef WL_WOWL_MEDIA
		wl_wowl_dngldown(wlc->wl);
#endif /* WL_WOWL_MEDIA */
		break;
	}

	case WLRPC_WLC_BMAC_TX_FIFO_SYNC_ID: {

#ifdef WL_MULTIQUEUE
		uint32 val32;
		uint fifo_bitmap;
		uint8 flag;

		err = bcm_xdr_unpack_uint32(&b, &val32);
		ASSERT(!err);
		fifo_bitmap = val32;

		err = bcm_xdr_unpack_uint32(&b, &val32);
		ASSERT(!err);
		flag = val32;

		wlc_bmac_tx_fifo_sync(wlc_hw, fifo_bitmap, flag);
#endif /* WL_MULTIQUEUE */
		break;
	}

	case WLRPC_WLC_BMAC_4331_EPA_INIT_ID: {
		bcm_xdr_buf_t retb;
		rpc_buf_t *rpc_buf;
		int ret;

		rpc_buf = wlc_rpc_buf_alloc(rpc, &retb, sizeof(uint32),
		    WLRPC_WLC_BMAC_4331_EPA_INIT_ID);
		ASSERT(rpc_buf != NULL);

		ret = wlc_bmac_4331_epa_init(wlc_hw);

		err = bcm_xdr_pack_int32(&retb, ret);
		ASSERT(!err);

		err = wlc_rpc_call_return(rpc, rpc_buf);
		ASSERT(!err);

		break;
	}

	case WLRPC_WLC_BMAC_SET_EXTLNA_PWRSAVE_SHMEM_ID: {
		wlc_bmac_set_extlna_pwrsave_shmem(wlc_hw);
		break;
	}

	case WLRPC_WLC_BMAC_CONFIG_4331_EPA_ID : {
#ifdef WOWL
		bool is5G, is_4331_12x9;
		uint32 uret1, uret2;

		err = bcm_xdr_unpack_uint32(&b, &uret1);
		ASSERT(!err);
		err = bcm_xdr_unpack_uint32(&b, &uret2);
		ASSERT(!err);

		is5G = (bool)uret1;
		is_4331_12x9 = (bool)uret2;
		wlc_bmac_wowl_config_4331_5GePA(wlc_hw, is5G, is_4331_12x9);
#endif /* WOWL */
		break;
	}

#ifdef MBSS
	case WLRPC_WLC_BMAC_UCODEMBSS_HWCAP: {
		bcm_xdr_buf_t retb;
		rpc_buf_t *rpc_buf;
		bool ret;

		ret = wlc_bmac_ucodembss_hwcap(wlc_hw);

		rpc_buf = wlc_rpc_buf_alloc(rpc, &retb, sizeof(uint32),
			WLRPC_WLC_BMAC_UCODEMBSS_HWCAP);
		ASSERT(rpc_buf != NULL);

		err = bcm_xdr_pack_uint32(&retb, (uint32)ret);
		ASSERT(!err);

		err = wlc_rpc_call_return(rpc, rpc_buf);
		ASSERT(!err);

		break;
	}
#endif /* MBSS */

	case WLRPC_WLC_PHY_DESTROY_CHANCTX_ID: {
#ifdef WLMCHAN
		uint32 val32;
		chanspec_t chanspec;

		err = bcm_xdr_unpack_uint32(&b, &val32);
		ASSERT(!err);
		chanspec = (chanspec_t)val32;

		wlc_phy_destroy_chanctx(wlc_hw->band->pi, chanspec);
#endif /* WLMCHAN */
		break;
	}

	case WLRPC_WLC_PHY_CREATE_CHANCTX_ID: {
#if defined(WLMCHAN) || defined(PHYCAL_CACHING)
		bcm_xdr_buf_t retb;
		rpc_buf_t *rpc_buf;
		uint32 val32;
		chanspec_t chanspec;
		int ret;

		err = bcm_xdr_unpack_uint32(&b, &val32);
		ASSERT(!err);
		chanspec = (chanspec_t)val32;

		ret = wlc_phy_create_chanctx(wlc_hw->band->pi, chanspec);

		rpc_buf = wlc_rpc_buf_alloc(rpc, &retb, sizeof(int32),
			WLRPC_WLC_PHY_CREATE_CHANCTX_ID);
		ASSERT(rpc_buf != NULL);

		err = bcm_xdr_pack_int32(&retb, ret);
		ASSERT(!err);

		err = wlc_rpc_call_return(rpc, rpc_buf);
		ASSERT(!err);

#endif /* WLMCHAN || PHYCAL_CACHING */
		break;
	}

	case WLRPC_WLC_BMAC_SET_FILT_WAR_ID: {
		int on;

		err = bcm_xdr_unpack_int32(&b, &on);
		ASSERT(!err);

		wlc_phy_set_filt_war(wlc_hw->band->pi, (bool)on);

		break;
	}

	case WLRPC_WLC_BMAC_SET_DEFMACINTMASK_ID: {
		uint32 mask32, val32;

		err = bcm_xdr_unpack_uint32(&b, &mask32);
		ASSERT(!err);
		err = bcm_xdr_unpack_uint32(&b, &val32);
		ASSERT(!err);

		wlc_bmac_set_defmacintmask(wlc_hw, mask32, val32);
		break;
	}

	case WLRPC_WLC_BMAC_ENABLE_TBTT_ID: {
		uint32 mask32, val32;

		err = bcm_xdr_unpack_uint32(&b, &mask32);
		ASSERT(!err);
		err = bcm_xdr_unpack_uint32(&b, &val32);
		ASSERT(!err);

		wlc_bmac_enable_tbtt(wlc_hw, mask32, val32);
		break;
	}

	case WLRPC_WLC_BMAC_RADIO_HW_ID: {
		uint32 enable, skip_anacore;
		bcm_xdr_buf_t retb;
		rpc_buf_t *rpc_buf;
		int ret;

		err = bcm_xdr_unpack_uint32(&b, &enable);
		ASSERT(!err);
		err = bcm_xdr_unpack_uint32(&b, &skip_anacore);
		ASSERT(!err);

		ret = wlc_bmac_radio_hw(wlc_hw, enable, skip_anacore);

		rpc_buf = wlc_rpc_buf_alloc(rpc, &retb, sizeof(int32),
			WLRPC_WLC_BMAC_RADIO_HW_ID);
		ASSERT(rpc_buf != NULL);

		err = bcm_xdr_pack_int32(&retb, ret);
		ASSERT(!err);

		err = wlc_rpc_call_return(rpc, rpc_buf);
		ASSERT(!err);

		break;
	}

	case WLRPC_WLC_BMAC_CCA_READ_COUNTER_ID: {
		bcm_xdr_buf_t retb;
		rpc_buf_t *rpc_buf;
		int lo_off, hi_off;
		uint32 val32;

		err = bcm_xdr_unpack_int32(&b, &lo_off);
		ASSERT(!err);
		err = bcm_xdr_unpack_int32(&b, &hi_off);
		ASSERT(!err);

		val32 = wlc_bmac_cca_read_counter(wlc_hw, lo_off, hi_off);

		rpc_buf = wlc_rpc_buf_alloc(rpc, &retb, sizeof(int32),
			WLRPC_WLC_BMAC_CCA_READ_COUNTER_ID);
		ASSERT(rpc_buf != NULL);

		err = bcm_xdr_pack_int32(&retb, val32);
		ASSERT(!err);

		err = wlc_rpc_call_return(rpc, rpc_buf);
		ASSERT(!err);

		break;
	}

	case WLRPC_WLC_PHY_INTERF_RSSI_UPDATE: {
		uint32 chanspec;
		int32 leastRSSI;

		err = bcm_xdr_unpack_uint32(&b, &chanspec);
		ASSERT(!err);

		err = bcm_xdr_unpack_int32(&b, &leastRSSI);
		ASSERT(!err);

		wlc_phy_interf_rssi_update(((wlc_hw_info_t *)wlc_hw)->band->pi,
			chanspec, (int8)leastRSSI);
		break;
	}

	case WLRPC_WLC_PHY_INTERF_CHAN_STATS_UPDATE: {
		uint32 chanspec, txop;
		uint32 crsglitch, bphy_crsglitch, badplcp, bphy_badplcp, mbsstime;

		err = bcm_xdr_unpack_uint32(&b, &chanspec);
		ASSERT(!err);

		err = bcm_xdr_unpack_uint32(&b, &crsglitch);
		ASSERT(!err);

		err = bcm_xdr_unpack_uint32(&b, &bphy_crsglitch);
		ASSERT(!err);

		err = bcm_xdr_unpack_uint32(&b, &badplcp);
		ASSERT(!err);

		err = bcm_xdr_unpack_uint32(&b, &bphy_badplcp);
		ASSERT(!err);

		err = bcm_xdr_unpack_uint32(&b, &txop);
		ASSERT(!err);

		err = bcm_xdr_unpack_uint32(&b, &mbsstime);
		ASSERT(!err);

		wlc_phy_interf_chan_stats_update(((wlc_hw_info_t *)wlc_hw)->band->pi,
			(chanspec_t)chanspec, crsglitch, bphy_crsglitch,
			badplcp, bphy_badplcp, txop, mbsstime);
		break;
	}

	case WLRPC_WLC_PHY_GET_EST_POUT_ID: {
		uint8 est_Pout[PHY_MAX_CORES];
		uint8 est_Pout_adj[PHY_MAX_CORES];
		uint8 est_Pout_cck;
		uint i;
		uint8* valptr;
		uint32 val;

		bcm_xdr_buf_t retb;
		rpc_buf_t *rpc_buf;

		rpc_buf = wlc_rpc_buf_alloc(rpc, &retb, sizeof(uint32) * 2,
		                            WLRPC_WLC_PHY_TXPOWER_GET_ID);
		ASSERT(rpc_buf != NULL);

		wlc_phy_get_est_pout(((wlc_hw_info_t*)wlc_hw)->band->pi, est_Pout, est_Pout_adj,
			&est_Pout_cck);

		ASSERT(PHY_MAX_CORES <= sizeof(val));
		valptr = (uint8*)&val;
		for (i = 0; i < PHY_MAX_CORES; i++)
			*valptr++ = est_Pout[i];

		err = bcm_xdr_pack_uint32(&retb, val);
		ASSERT(!err);

		valptr = (uint8*)&val;
		for (i = 0; i < PHY_MAX_CORES; i++)
			*valptr++ = est_Pout_adj[i];

		err = bcm_xdr_pack_uint32(&retb, val);
		ASSERT(!err);

		val = est_Pout_cck;
		err = bcm_xdr_pack_uint32(&retb, val);
		ASSERT(!err);

		err = wlc_rpc_call_return(rpc, rpc_buf);
		ASSERT(!err);

		break;
	}

	case WLRPC_WLC_PHY_TSSISEN_MIN_ID: {
		int8 tssiSensMin[PHY_MAX_CORES];
		uint i;

		bcm_xdr_buf_t retb;
		rpc_buf_t *rpc_buf;
		uint32 val;
		int8* valptr;

		rpc_buf = wlc_rpc_buf_alloc(rpc, &retb, sizeof(uint32),
		                            WLRPC_WLC_PHY_TSSISEN_MIN_ID);
		ASSERT(rpc_buf != NULL);

		bzero(tssiSensMin, sizeof(tssiSensMin));
		wlc_phy_get_tssi_sens_min(((wlc_hw_info_t*)wlc_hw)->band->pi, tssiSensMin);

		valptr = (int8*)&val;
		for (i = 0; i < PHY_MAX_CORES; i++) {
			*valptr++ = tssiSensMin[i];
		}

		err = bcm_xdr_pack_uint32(&retb, val);
		ASSERT(!err);

		err = wlc_rpc_call_return(rpc, rpc_buf);
		ASSERT(!err);

		break;
	}

	case WLRPC_WLC_BMAC_SRVSDB_FORCE_SET:
		if (SRHWVSDB_ENAB(wlc_hw)) {
			int8 val;

			err = bcm_xdr_unpack_int8(&b, &val);
			ASSERT(!err);

			wlc_bmac_srvsdb_force_set(wlc_hw, (uint8)val);
		}
		break;

	case WLRPC_WLC_PHY_FORCE_VSDB_CHANS:
		if (SRHWVSDB_ENAB(wlc_hw)) {
			uint32 val;
			uint16 channels[2];
			bool set;

			err = bcm_xdr_unpack_uint16_vec(&b, sizeof(uint16) * 2, channels);
			ASSERT(!err);

			err = bcm_xdr_unpack_uint32(&b, &val);
			ASSERT(!err);

			set = (bool)val;

			wlc_phy_force_vsdb_chans(((wlc_hw_info_t *)wlc_hw)->band->pi, channels,
				set);
		}
		break;

	case WLRPC_WLC_BMAC_ACTIVATE_SRVSDB:
		if (SRHWVSDB_ENAB(wlc_hw)) {
			uint32 ret;
			uint32 chan0, chan1;
			rpc_buf_t *rpc_buf;
			bcm_xdr_buf_t retb;

			err = bcm_xdr_unpack_uint32(&b, &chan0);
			ASSERT(!err);

			err = bcm_xdr_unpack_uint32(&b, &chan1);
			ASSERT(!err);

			ret = wlc_bmac_activate_srvsdb(wlc_hw, chan0, chan1);

			rpc_buf = wlc_rpc_buf_alloc(rpc, &retb, sizeof(uint32),
				WLRPC_WLC_BMAC_ACTIVATE_SRVSDB);
			ASSERT(rpc_buf != NULL);

			err = bcm_xdr_pack_uint32(&retb, ret);
			ASSERT(!err);

			err = wlc_rpc_call_return(rpc, rpc_buf);
			ASSERT(!err);
		}
		break;

	case WLRPC_WLC_BMAC_DEACTIVATE_SRVSDB: {
		if (SRHWVSDB_ENAB(wlc_hw)) {
			wlc_bmac_deactivate_srvsdb(wlc_hw);
		}
		break;
	}
	case WLRPC_WLC_BMAC_PHY_TXPWR_CACHE_INVALIDATE_ID: {
#ifdef WLTXPWR_CACHE
		wlc_phy_txpwr_cache_invalidate(
			wlc_phy_get_txpwr_cache(((wlc_hw_info_t *)wlc_hw)->band->pi));
#endif // endif
		break;
	}
#if defined(PHYCAL_CACHING) || defined(WLMCHAN)
	case WLRPC_WLC_PHY_CAL_CACHE_ACPHY_ID: {
		wlc_phy_cal_cache(((wlc_hw_info_t *)wlc_hw)->band->pi);
		break;
	}
	case WLRPC_WLC_PHY_CAL_CACHE_RESTORE_ID: {
		bcm_xdr_buf_t retb;
		rpc_buf_t *rpc_buf;
		int pval;
		rpc_buf = wlc_rpc_buf_alloc(rpc, &retb, sizeof(uint32),
		WLRPC_WLC_PHY_CAL_CACHE_RESTORE_ID);
		ASSERT(rpc_buf != NULL);
		pval = wlc_phy_cal_cache_return(((wlc_hw_info_t *)wlc_hw)->band->pi);
		err = bcm_xdr_pack_uint32(&retb, pval);
		ASSERT(!err);
		err = wlc_rpc_call_return(rpc, rpc_buf);
		ASSERT(!err);
		break;
	}
#ifdef WLOLPC
	case WLRPC_WLC_PHY_PI_UPDATE_OLPC_CAL_ID: {
		uint32 ret;
		err = bcm_xdr_unpack_uint32(&b, &ret);
		ASSERT(!err);
		wlc_phy_update_olpc_cal(((wlc_hw_info_t *)wlc_hw)->band->pi,
		(ret & 0x0f) != 0, (ret & 0xf0) != 0);
		break;
	}
#endif /* WLOLPC */
#endif /* PHYCAL_CACHING || MCHAN */
	case WLRPC_WLC_PHY_IS_TXBFCAL: {
		bcm_xdr_buf_t retb;
		rpc_buf_t *rpc_buf;
		uint32 ret;

		ret = wlc_phy_is_txbfcal(wlc_hw->band->pi);

		rpc_buf = wlc_rpc_buf_alloc(rpc, &retb,
		                            sizeof(uint32),
		                            WLRPC_WLC_PHY_IS_TXBFCAL);
		ASSERT(rpc_buf != NULL);

		err = bcm_xdr_pack_uint32(&retb, ret);
		ASSERT(!err);

		err = wlc_rpc_call_return(rpc, rpc_buf);
		ASSERT(!err);

		break;
	}

	case WLRPC_WLC_BMAC_GET_NORESET_ID: {
		bcm_xdr_buf_t retb;
		rpc_buf_t *rpc_buf;
		uint32 ret;

		ret = wlc_bmac_get_noreset(wlc_hw);

		rpc_buf = wlc_rpc_buf_alloc(rpc, &retb,
		                            sizeof(uint32),
		                            WLRPC_WLC_BMAC_GET_NORESET_ID);
		ASSERT(rpc_buf != NULL);

		err = bcm_xdr_pack_uint32(&retb, ret);
		ASSERT(!err);

		err = wlc_rpc_call_return(rpc, rpc_buf);
		ASSERT(!err);

		break;
	}

	case WLRPC_WLC_PHY_NOISE_AVG_PER_ANTENNA_ID: {
		bcm_xdr_buf_t retb;
		rpc_buf_t *rpc_buf;
		int32 coreidx;
		int8 noise;

		err = bcm_xdr_unpack_int32(&b, &coreidx);
		ASSERT(!err);

		noise = wlc_phy_noise_avg_per_antenna(
			((wlc_hw_info_t *)wlc_hw)->band->pi, coreidx);

		rpc_buf = wlc_rpc_buf_alloc(rpc, &retb,
			sizeof(uint32),
			WLRPC_WLC_PHY_NOISE_AVG_PER_ANTENNA_ID);
		ASSERT(rpc_buf != NULL);

		err = bcm_xdr_pack_int8(&retb, noise);
		ASSERT(!err);

		err = wlc_rpc_call_return(rpc, rpc_buf);
		ASSERT(!err);

		break;
	}
	case WLRPC_WLC_PHY_TXPWR_MIN_GET_ID: {
		uint8 min_pwr;
		bcm_xdr_buf_t retb;
		rpc_buf_t *rpc_buf;
		uint32 ret;

		err = bcm_xdr_unpack_uint32(&b, &ret);
		ASSERT(!err);
		min_pwr = (uint8)ret;

		rpc_buf = wlc_rpc_buf_alloc(rpc, &retb, sizeof(uint32),
		WLRPC_WLC_PHY_TXPWR_MIN_GET_ID);
		ASSERT(rpc_buf != NULL);
		wlc_phy_get_txpwr_min(((wlc_hw_info_t *)wlc_hw)->band->pi, &min_pwr);

		err = bcm_xdr_pack_uint32(&retb, min_pwr);
		ASSERT(!err);
		err = wlc_rpc_call_return(rpc, rpc_buf);
		ASSERT(!err);

		break;
	}
	case WLRPC_WLC_PHY_TXPWR_BACKOFF_GET_ID: {
		int pval = 0;
		bcm_xdr_buf_t retb;
		rpc_buf_t *rpc_buf;

		rpc_buf = wlc_rpc_buf_alloc(rpc, &retb, sizeof(uint32),
		WLRPC_WLC_PHY_TXPWR_BACKOFF_GET_ID);
		ASSERT(rpc_buf != NULL);
		pval = wlc_phy_get_txpwr_backoff(((wlc_hw_info_t *)wlc_hw)->band->pi);

		err = bcm_xdr_pack_uint32(&retb, pval);
		ASSERT(!err);
		err = wlc_rpc_call_return(rpc, rpc_buf);
		ASSERT(!err);
		break;
	}
	case WLRPC_WLC_PHY_MPHASE_CAL_SCHEDULE_ID: {
		uint32 ret;
		uint delay_val;

		err = bcm_xdr_unpack_uint32(&b, &ret);
		ASSERT(!err);

		delay_val = (uint)ret;

		wlc_phy_mphase_cal_schedule(((wlc_hw_info_t *)wlc_hw)->band->pi, delay_val);
		break;
	}
	case WLRPC_WLC_PHY_RXGCRS_SET_LOCALE: {
		uint32 ret;
		uint8 region_group;

		err = bcm_xdr_unpack_uint32(&b, &ret);
		ASSERT(!err);

		region_group = (uint8)ret;

		wlc_phy_set_locale((phy_info_t *)wlc_hw->band->pi, region_group);
		break;
	}
	case WLRPC_WLC_PHY_GET_BFE_NDP_RECVSTS: {
		int pval = 0;
		bcm_xdr_buf_t retb;
		rpc_buf_t *rpc_buf;

		rpc_buf = wlc_rpc_buf_alloc(rpc, &retb, sizeof(uint32),
		WLRPC_WLC_PHY_GET_BFE_NDP_RECVSTS);
		ASSERT(rpc_buf != NULL);
		pval = wlc_phy_get_bfe_ndp_recvstreams(((wlc_hw_info_t *)wlc_hw)->band->pi);

		err = bcm_xdr_pack_uint32(&retb, pval);
		ASSERT(!err);
		err = wlc_rpc_call_return(rpc, rpc_buf);
		ASSERT(!err);
		break;
	}

	case WLRPC_WLC_BMAC_SET_OBJMEM32_ID: {
		uint32 ret;
		uint offset;
		uint16 v;
		int len;
		uint32 sel;

		err = bcm_xdr_unpack_uint32(&b, &offset);
		ASSERT(!err);

		err = bcm_xdr_unpack_uint32(&b, &ret);
		ASSERT(!err);

		v = (uint16)ret;

		err = bcm_xdr_unpack_int32(&b, &len);
		ASSERT(!err);

		err = bcm_xdr_unpack_uint32(&b, &sel);
		ASSERT(!err);

		wlc_bmac_set_objmem32(wlc_hw, offset, v, len, sel);
		break;
	}
	case WLRPC_WLC_BMAC_COPYFROM_OBJMEM32_ID: {
		uint offset;
		uint8* hwbuf;
		int len;
		uint32 sel;
		bcm_xdr_buf_t retb;
		rpc_buf_t *rpc_buf;

		err = bcm_xdr_unpack_uint32(&b, &offset);
		ASSERT(!err);

		err = bcm_xdr_unpack_int32(&b, &len);
		ASSERT(!err && (len > 0));

		err = bcm_xdr_unpack_uint32(&b, &sel);
		ASSERT(!err);

		if ((hwbuf = (uint8*)MALLOC(NULL, len)) == NULL) {
			ASSERT(0);
			break;
		}

		wlc_bmac_copyfrom_objmem32(wlc_hw, offset, hwbuf, len, sel);

		rpc_buf = wlc_rpc_buf_alloc(rpc, &retb,
		                            sizeof(uint32) + ROUNDUP(len, sizeof(uint32)),
		                            WLRPC_WLC_BMAC_COPYFROM_OBJMEM32_ID);
		ASSERT(rpc_buf != NULL);

		err = bcm_xdr_pack_opaque_varlen(&retb, len, hwbuf);
		ASSERT(!err);

		err = wlc_rpc_call_return(rpc, rpc_buf);
		ASSERT(!err);

		MFREE(NULL, hwbuf, FALSE);
		break;
	}
	case WLRPC_WLC_BMAC_COPYTO_OBJMEM32_ID: {
		uint offset;
		void* hwbuf;
		uint len;
		uint32 sel;
		err = bcm_xdr_unpack_uint32(&b, &offset);
		ASSERT(!err);

		err = bcm_xdr_unpack_opaque_varlen(&b, &len, &hwbuf);
		ASSERT(!err);

		err = bcm_xdr_unpack_uint32(&b, &sel);
		ASSERT(!err);

		wlc_bmac_copyto_objmem32(wlc_hw, offset, (uint8 *)hwbuf, len, sel);
		break;
	}
#if defined(WL_PSMX)
	case WLRPC_WLC_ENABLE_MACX_ID:
		wlc_bmac_enable_macx(wlc_hw);
		break;
	case WLRPC_WLC_SUSPEND_MACX_AND_WAIT_ID:
		wlc_bmac_suspend_macx_and_wait(wlc_hw);
		break;
#endif /* WL_PSMX */
	default:
		printf("wl_rpc_bmac_dispatch: rpc_id %d\n", rpc_id);
		ASSERT(0);
	}

	/* entire buffer should be consumed */
	if (b.size != 0) {
		WL_ERROR(("rpc buffer not consumed, rpc_id %d b.size = %d \n", rpc_id, b.size));
#if defined(BCMDBG) || defined(BCMDBG_ERR)
		WL_TRACE(("id %s\n",
		          WLC_RPC_ID_LOOKUP(rpc_name_tbl, rpc_id)));
#endif /* BCMDBG || BCMDBG_ERR */
	}
	/*
	  XXX : removing the ASSERT for now in favor of printing information about which
	  RPC call is flawed.
	  ASSERT(b.size == 0);
	*/
	if (free_buf && buf != NULL)
		bcm_rpc_buf_free(rpc, buf);
}

/* ============= High stub functions =================== */
#ifdef WLRM
void
wlc_rm_cca_complete(wlc_info_t *wlc, uint32 cca_idle_us)
{
	bcm_xdr_buf_t b;
	rpc_buf_t *rpc_buf;
	int err;
	rpc_info_t *rpc = wlc->rpc;

	rpc_buf = wlc_rpc_buf_alloc(rpc, &b, sizeof(uint32), WLRPC_WLC_RM_CCA_COMPLETE_ID);
	ASSERT(rpc_buf != NULL);

	err = bcm_xdr_pack_uint32(&b, cca_idle_us);
	ASSERT(!err);

	err = wlc_rpc_call(rpc, rpc_buf);
	ASSERT(!err);
}
#endif /* WLRM */

void
wlc_lq_noise_cb(wlc_info_t *wlc, uint8 channel, int8 noise_dbm)
{
	bcm_xdr_buf_t b;
	rpc_buf_t *rpc_buf;
	int err;
	rpc_info_t *rpc = wlc->rpc;

	rpc_buf = wlc_rpc_buf_alloc(rpc, &b, 2*sizeof(uint32), WLRPC_WLC_NOISE_CB_ID);
	ASSERT(rpc_buf != NULL);

	err = bcm_xdr_pack_int8(&b, channel);
	ASSERT(!err);

	err = bcm_xdr_pack_int8(&b, noise_dbm);
	ASSERT(!err);

	err = wlc_rpc_call(rpc, rpc_buf);
	ASSERT(!err);
}

#define USB_TOTAL_LEN_BAD	516
#define USB_TOTAL_LEN_BAD_PAD	8
/* above is pre-aggregate, post-aggregate needs to be padded in bcm_rpc_tp_rte.c module
#define BCM_RPC_TP_DNGL_TOTLEN_BAD	516
#define BCM_RPC_TP_DNGL_TOTLEN_BAD_PAD	8
*/

#ifndef BCM_OL_DEV

static rpc_buf_t *
wlc_rxpkt_to_rpc_buf(wlc_hw_info_t *wlc_hw, rpc_info_t *rpc, void *p)
{
	bcm_xdr_buf_t b;
	rpc_buf_t *rpc_buf = NULL;
	int err;
	uint len = pkttotlen(wlc_hw->osh, p);
	uint pktlen = ROUNDUP(len, sizeof(uint32));
	uint32 tsf_l, tsf_h;

	/* XXX PR60308 WAR 4322USB: USB core on dongle sometime transfers data at incorrect boundary
	 * For problem value USB_TOTAL_LEN_BAD (= 500bytes +  bcm_rpc_buf_header_len(8) + 8 ),
	 *    | TP_header(4) | RPC_header(4) | RPC_ID(4) | vLEN(4) | vData(rounded up) |
	 * pad the length. The actual length is carried by 'len' field
	 */
	if (pktlen == USB_TOTAL_LEN_BAD - 16) {
		pktlen += USB_TOTAL_LEN_BAD_PAD;
	}

#ifndef RPCPKTCOPY
	/* Make sure there is enough headroom
	 * 4 byte for RPC ID, 4 bytes for length of the packet, and finally the RPC + TP header
	 */
	ASSERT(PKTHEADROOM(wlc_hw->osh, p) >=
		(sizeof(uint32) * 4 + bcm_rpc_buf_header_len(rpc)));

	PKTSETLEN(wlc_hw->osh, p, pktlen);

	/* Make room for RPC ID + length field */
	PKTPUSH(wlc_hw->osh, p, sizeof(uint32) * 4);

	/* This is direct mapping */
	rpc_buf = (rpc_buf_t *)p;

	/* adjust RPC TP buffer accounting since we adding the buffer to RPC */
	bcm_rpc_tp_buf_cnt_adjust(bcm_rpc_tp_get(rpc), 1);

	bcm_xdr_buf_init(&b, bcm_rpc_buf_data(bcm_rpc_tp_get(rpc), rpc_buf), sizeof(uint32) * 4);

	err = bcm_xdr_pack_uint32(&b, WLRPC_WLC_RECV_ID);
	ASSERT(!err);

	wlc_bmac_read_tsf(wlc_hw, &tsf_l, &tsf_h);

	err = bcm_xdr_pack_uint32(&b, tsf_l);
	ASSERT(!err);

	err = bcm_xdr_pack_uint32(&b, tsf_h);
	ASSERT(!err);

	/* Add the length separately */
	err = bcm_xdr_pack_uint32(&b, len);
	ASSERT(!err);

#else	/* old PKT copy version */

	rpc_buf = wlc_rpc_buf_alloc(rpc, &b, sizeof(uint32) + pktlen, WLRPC_WLC_RECV_ID);

	if (rpc_buf == NULL) {
		printf("%s %d : Insufficient memory %d(%dK) to receive Requested:%d\n",
		       __FUNCTION__,
		       __LINE__, hnd_memavail(), hnd_memavail()/1000,
		       sizeof(uint32) + pktlen);
		goto err;
	}

	err = bcm_xdr_pack_opaque_varlen(&b, len, PKTDATA(wlc->hw->osh, p));
	ASSERT(!err);

err:
	PKTFREE(wlc_hw->osh, p, FALSE);
#endif /* RPCPKTCOPY */

	return rpc_buf;
}

void
wlc_recv(wlc_info_t *wlc, void *p)
{
	rpc_buf_t *rpc_buf = NULL;
	int err;
	rpc_info_t *rpc = wlc->rpc;

	rpc_buf = wlc_rxpkt_to_rpc_buf(wlc->hw, rpc, p);

	err = wlc_rpc_call(rpc, rpc_buf);
	ASSERT(!err);
}

/* Special handling for AMPDU is needed because of the way TX status is posted for an AMPDU.
 * It's a two-stage process:
 *    * First TX status just tells whether AMPDU was ack'ed successfully or not
 *    * If ACK was recvd, then second txstatus fills in rest of the information
 * For High-driver AMPDU to handle this correctly, the BMAC needs to consume both TX statuses
 * and send them to HIGH driver. Also, for AMPDU, all of the individual MPDUs, posted to the
 * DMA ring, that belong to that one AMPDU need to be freed regardless of ACK
 * Note: This function needs to emulate wlc_ampdu.c:wlc_ampdu_free_chain
 */
static bool
wlc_high_stubs_ampdu_txstatus_complete(wlc_info_t *wlc, void *p, tx_status_t *txs, uint8 queue,
                                       bcm_xdr_buf_t *b)
{
	int err;

#ifndef DMA_TX_FREE
	d11txh_t *txh;
	uint16 mcl;

	while (p) {
		ASSERT(WLPKTTAG(p)->flags & WLF_AMPDU_MPDU);

		/* we don't queue zero length tx frame, it must have valid txh->mcl bytes
		 * even the whole pkt may wrap over to next bufer
		 */
		ASSERT(PKTLEN(wlc->osh, p) >= 2);
		txh = (d11txh_t *)PKTDATA(wlc->osh, p);
		mcl = ltoh16(txh->MacTxControlLow);

		WL_TRACE(("%s %d p:%p mcl:0x%x\n", __FUNCTION__, __LINE__, p, mcl));

		/* free the pkt chain as there is not more use in BMAC driver */
		PKTFREE(wlc->hw->osh, p, TRUE);

		/* if it's the last ampdu, stop pulling pkts from DMA ring */
		if (((mcl & TXC_AMPDU_MASK) >> TXC_AMPDU_SHIFT) == TXC_AMPDU_LAST)
			break;

		/* For regular MPDU, just return */
		if ((mcl & TXC_AMPDU_MASK) == TXC_AMPDU_NONE) {
			TXPKTPENDDEC(wlc, queue, 1);
			WL_TRACE(("wlc_high_stubs_ampdu_txstatus_complete, pktpend dec 1 to %d\n",
				TXPKTPENDGET(wlc, queue)));
			return FALSE;
		}

		p = wlc_bmac_dma_getnexttxp(wlc, queue, HNDDMA_RANGE_TRANSMITTED);
	}
#endif /* DMA_TX_FREE */

	TXPKTPENDDEC(wlc, queue, 2);	/* AMPDU TX WEIGHT is 2 */
	WL_TRACE(("wlc_high_stubs_ampdu_txstatus_complete, pktpend dec 2 to %d\n",
		TXPKTPENDGET(wlc, queue)));

	/* For successful ACK, BMAC needs to retrieve and send the 2nd TX status of an AMPDU */
	if (txs->status.was_acked) {
		uint8 status_delay = 0;
		uint32 s1 = 0, s2 = 0;

		/* wait till the next 8 bytes of txstatus is available */
		while (((s1 = R_REG(wlc->osh, &wlc->regs->frmtxstatus)) & TXS_V) == 0) {
			OSL_DELAY(1);
			status_delay++;
			if (status_delay > 10) {
				ASSERT(0);
				break;
			}
		}

		WL_TRACE(("wl%d: wlc_ampdu_dotxstatus: 2nd status in delay %d 0x%x\n",
		          wlc->hw->unit, status_delay, s1));

		ASSERT(!(s1 & TX_STATUS_INTERMEDIATE));
		ASSERT(s1 & TX_STATUS_AMPDU);

		s2 = R_REG(wlc->osh, &wlc->regs->frmtxstatus2);

		err = bcm_xdr_pack_uint32(b, WLRPC_WLC_AMPDU_TXSTATUS_COMPLETE_ID);
		ASSERT(!err);

		err = bcm_xdr_pack_uint32(b, s1);
		ASSERT(!err);

		err = bcm_xdr_pack_uint32(b, s2);
		ASSERT(!err);
		return TRUE;
	}

	err = bcm_xdr_pack_uint32(b, WLRPC_NULL_ID);
	ASSERT(!err);

	return FALSE;
}

bool
wlc_dotxstatus(wlc_info_t *wlc, tx_status_t *txs, uint32 frm_tx2)
{
	bcm_xdr_buf_t b;
	rpc_buf_t *rpc_buf = NULL;
	int err;
	rpc_info_t *rpc = wlc->rpc;
	wlc_rpc_txstatus_t rpc_txstatus;
	uint queue;
	uint rpc_len = ROUNDUP(sizeof(wlc_rpc_txstatus_t), sizeof(uint32)) +
	        sizeof(uint32);
	uint rpc_ampdu_len = 3 * sizeof(uint32); /* Allocate with extra length */
	void *p = NULL;

	bzero(&rpc_txstatus, sizeof(wlc_rpc_txstatus_t));
	txstatus2rpc_txstatus(txs, &rpc_txstatus);

	/* Allocate a buffer big enough to include AMPDU txstatus */
	rpc_buf = wlc_rpc_buf_alloc(rpc, &b, rpc_len + rpc_ampdu_len,
	                            WLRPC_WLC_DOTXSTATUS_ID);
	ASSERT(rpc_buf != NULL);

	err = bcm_xdr_pack_uint32(&b, rpc_txstatus.frameid_framelen);
	ASSERT(!err);

	err = bcm_xdr_pack_uint32(&b, rpc_txstatus.sequence_status);
	ASSERT(!err);

	err = bcm_xdr_pack_uint32(&b, rpc_txstatus.lasttxtime);
	ASSERT(!err);

	err = bcm_xdr_pack_uint32(&b, rpc_txstatus.ackphyrxsh_phyerr);
	ASSERT(!err);

	err = bcm_xdr_pack_uint32(&b, frm_tx2);
	ASSERT(!err);

	err = bcm_xdr_pack_uint32(&b, rpc_txstatus.s3);
	ASSERT(!err);

	err = bcm_xdr_pack_uint32(&b, rpc_txstatus.s4);
	ASSERT(!err);

	err = bcm_xdr_pack_uint32(&b, rpc_txstatus.s5);
	ASSERT(!err);

	err = bcm_xdr_pack_uint32(&b, rpc_txstatus.s6);
	ASSERT(!err);

	err = bcm_xdr_pack_uint32(&b, rpc_txstatus.s7);
	ASSERT(!err);

	queue = WLC_TXFID_GET_QUEUE(txs->frameid);

	if (D11REV_LT(wlc->hw->corerev, 40)) {
#ifndef DMA_TX_FREE
			/* XXX Free the packet from the ring to reclaim memory.
			 * To be optimized later using TX interrupt.
			 * Note for AMPDU, the whole ring will need to be freed up
	 */
	p = wlc_bmac_dma_getnexttxp(wlc, queue, HNDDMA_RANGE_TRANSMITTED);
	ASSERT(p);
#endif /* !DMA_TX_FREE */
		if (txs->status.raw_bits & TX_STATUS_AMPDU) {
			wlc_high_stubs_ampdu_txstatus_complete(wlc, p, txs, queue, &b);
		} else {
#ifndef DMA_TX_FREE
			PKTFREE(wlc->hw->osh, p, TRUE);
#endif /* !DMA_TX_FREE */
			TXPKTPENDDEC(wlc, queue, 1);
			WL_TRACE(("wlc_dotxstatus, pktpend dec 1 to %d\n",
				TXPKTPENDGET(wlc, queue)));
			err = bcm_xdr_pack_uint32(&b, WLRPC_NULL_ID);
			ASSERT(!err);
		}
	} else {
		int ncons = 0;
#ifndef DMA_TX_FREE
		uint16 supr_status = 0;
#endif // endif
		ncons = ((txs->status.raw_bits & TX_STATUS40_NCONS)
			>> TX_STATUS40_NCONS_SHIFT);
#ifndef DMA_TX_FREE
		if (!(txs->status.raw_bits & TXS_V)) {
			printf("not valid %x\n", txs->status.raw_bits);
			ncons = 0;
			WL_TRACE(("%s: invalid txstatus 0x%x\n", __FUNCTION__,
				txs->status.raw_bits));
		}

		supr_status = (txs->status.raw_bits & TX_STATUS40_SUPR)
			>> TX_STATUS40_SUPR_SHIFT;
		if (supr_status & (TX_STATUS_SUPR_FLUSH | TX_STATUS_SUPR_BADCH)) {
			WL_TRACE(("%s: suppressed due to flush\n", __FUNCTION__));
		}

		while (ncons > 0) {
			p = wlc_bmac_dma_getnexttxp(wlc, queue, HNDDMA_RANGE_TRANSMITTED);
			ASSERT(p);
		PKTFREE(wlc->hw->osh, p, TRUE);
			ncons--;
		TXPKTPENDDEC(wlc, queue, 1);
			WL_TRACE(("wlc_dotxstatus, pktpend dec 1 to %d\n",
				TXPKTPENDGET(wlc, queue)));
		}
#endif /* !DMA_TX_FREE */
		err = bcm_xdr_pack_uint32(&b, WLRPC_NULL_ID);
		ASSERT(!err);
	}

	err = wlc_rpc_call(rpc, rpc_buf);
	ASSERT(!err);

	return FALSE;
}
#endif /* BCM_OL_DEV */

void
wlc_high_dpc(wlc_info_t *wlc, uint32 macintstatus)
{
	bcm_xdr_buf_t b;
	rpc_buf_t *rpc_buf;
	int err;
	rpc_info_t *rpc = wlc->rpc;
	uint32 magic = 0xA5A5A5A5;
	uint32 tsf_l, tsf_h;

	rpc_buf = wlc_rpc_buf_alloc(rpc, &b, 4 * sizeof(uint32), WLRPC_WLC_HIGH_DPC_ID);
	ASSERT(rpc_buf != NULL);

	wlc_bmac_read_tsf(wlc->hw, &tsf_l, &tsf_h);

	err = bcm_xdr_pack_uint32(&b, tsf_l);
	ASSERT(!err);

	err = bcm_xdr_pack_uint32(&b, tsf_h);
	ASSERT(!err);

	err = bcm_xdr_pack_uint32(&b, macintstatus);
	ASSERT(!err);

	/* XXX PR61642, some host machines have problem with small 12 bytes USB pkt.
	 * may not be necessary with RPC/TP header
	 */
	err = bcm_xdr_pack_uint32(&b, magic);
	ASSERT(!err);

	err = wlc_rpc_call(rpc, rpc_buf);
	ASSERT(!err);
}

void
wlc_bmac_rpc_watchdog(wlc_info_t *wlc)
{
	bcm_rpc_watchdog(wlc->rpc);
}

void
wlc_fatal_error(wlc_info_t *wlc)
{
	bcm_xdr_buf_t b;
	rpc_buf_t *rpc_buf;
	int err;
	rpc_info_t *rpc = wlc->rpc;

	rpc_buf = wlc_rpc_buf_alloc(rpc, &b, 0, WLRPC_WLC_FATAL_ERROR_ID);
	ASSERT(rpc_buf != NULL);

	err = wlc_rpc_call(rpc, rpc_buf);
	ASSERT(!err);
}

void
wlc_update_phy_mode(wlc_info_t *wlc, uint32 phy_mode)
{
	bcm_xdr_buf_t b;
	rpc_buf_t *rpc_buf;
	int err;
	rpc_info_t *rpc = wlc->rpc;

	rpc_buf = wlc_rpc_buf_alloc(rpc, &b, sizeof(uint32), WLRPC_WLC_UPDATE_PHY_MODE_ID);
	ASSERT(rpc_buf != NULL);

	err = bcm_xdr_pack_uint32(&b, phy_mode);
	ASSERT(!err);

	err = wlc_rpc_call(rpc, rpc_buf);
	ASSERT(!err);
}

void
wlc_reset_bmac_done(wlc_info_t *wlc)
{
	bcm_xdr_buf_t b;
	rpc_buf_t *rpc_buf;
	int err;
	rpc_info_t *rpc = wlc->rpc;

	rpc_buf = wlc_rpc_buf_alloc(rpc, &b, 0, WLRPC_WLC_RESET_BMAC_DONE_ID);
	ASSERT(rpc_buf != NULL);

	err = wlc_rpc_call(rpc, rpc_buf);
	ASSERT(!err);
}
#ifdef AP
int
wlc_apps_process_ps_switch(wlc_info_t *wlc, struct ether_addr *ea, int8 ps_on)
{
	bcm_xdr_buf_t b;
	rpc_buf_t *rpc_buf = NULL;
	int err;
	rpc_info_t *rpc = wlc->rpc;

	rpc_buf = wlc_rpc_buf_alloc(rpc, &b, ROUNDUP(sizeof(struct ether_addr) +
	                                             sizeof(uint32), sizeof(uint32)),
	                            WLRPC_WLC_BMAC_PS_SWITCH_ID);

	ASSERT(rpc_buf != NULL);

	err = bcm_xdr_pack_int8(&b, ps_on);
	ASSERT(!err);
	if (ea) {
		err = bcm_xdr_pack_opaque(&b, sizeof(struct ether_addr), (void*)ea);
		ASSERT(!err);
		ASSERT(!(ps_on & PS_SWITCH_MAC_INVALID));
	}
	else /* make sure we indicate no valid mac addr   */
		ASSERT(ps_on & PS_SWITCH_MAC_INVALID);

	err = wlc_rpc_call(rpc, rpc_buf);
	ASSERT(!err);
	return 0;
}
#endif /* AP */

#ifdef WLEXTLOG
void
wlc_extlog_msg(wlc_info_t *wlc, uint16 module, uint8 id,
	uint8 level, uint8 sub_unit, int arg, char *str)
{
	bcm_xdr_buf_t b;
	rpc_buf_t *rpc_buf = NULL;
	int err;
	rpc_info_t *rpc = wlc->rpc;
	wlc_extlog_info_t *extlog = (wlc_extlog_info_t *)wlc->hw->extlog;

	if ((module > (1 << (LOG_MODULE_MAX - 1))) || (level > WL_LOG_LEVEL_MAX)) {
		ASSERT(0); /* XXXXX DON'T CONVERT !!!!XXXXX */
		return;
	}

	if (!(extlog->cfg.module & module) || (level > extlog->cfg.level))
		return;

	rpc_buf = wlc_rpc_buf_alloc(rpc, &b, (sizeof(uint32) * 6 + MAX_ARGSTR_LEN),
	                            WLRPC_WLC_EXTLOG_MSG_ID);

	ASSERT(rpc_buf != NULL);

	err = bcm_xdr_pack_uint32(&b, module);
	ASSERT(!err);

	err = bcm_xdr_pack_uint32(&b, id);
	ASSERT(!err);

	err = bcm_xdr_pack_uint32(&b, level);
	ASSERT(!err);

	err = bcm_xdr_pack_uint32(&b, sub_unit);
	ASSERT(!err);

	err = bcm_xdr_pack_uint32(&b, arg);
	ASSERT(!err);

	if (str)
		err = bcm_xdr_pack_opaque_varlen(&b, MIN(MAX_ARGSTR_LEN, strlen(str)), (void*)str);
	else
		err = bcm_xdr_pack_opaque_varlen(&b, 0, NULL);
	ASSERT(!err);

	WL_TRACE(("%s(): module=%d, id=%d, level=%d, sub_unit=%d, arg=%d, str=%s\n",
		__FUNCTION__, module, id, level, sub_unit, arg, str ? str : NULL));

	err = wlc_rpc_call(rpc, rpc_buf);
	ASSERT(!err);

}
#endif /* WLEXTLOG */

void
wlc_update_txppr_offset(wlc_info_t *wlc, ppr_t *txppr)
{
	bcm_xdr_buf_t b;
	rpc_buf_t *rpc_buf;
	int err;
	uint len;
	uint slen;
	rpc_info_t *rpc = wlc->rpc;
	uint8* tbuf;

	slen = ppr_ser_size(txppr);
	if ((tbuf = MALLOC(wlc->osh, slen)) != NULL) {
		if (ppr_serialize(txppr, tbuf, slen, &len) == BCME_OK) {
			len = XDR_PACK_OPAQUE_VAR_SZ(len);

			rpc_buf = wlc_rpc_buf_alloc(rpc, &b, len + sizeof(uint32),
				WLRPC_WLC_BMAC_TXPPR_ID);
			ASSERT(rpc_buf != NULL);

			err = bcm_xdr_pack_opaque_varlen(&b, len, tbuf);
			ASSERT(!err);

			err = wlc_rpc_call(rpc, rpc_buf);
			ASSERT(!err);
		}
		MFREE(wlc->osh, tbuf, slen);
	}
}

#ifdef AP
void
wlc_tx_fifo_sync_bcmc_reset(wlc_info_t *wlc)
{
	bcm_xdr_buf_t b;
	rpc_buf_t *rpc_buf;
	int err;
	rpc_info_t *rpc = wlc->rpc;

	rpc_buf = wlc_rpc_buf_alloc(rpc, &b, 0, WLRPC_WLC_BMAC_TX_FIFO_SYNC_BCMC_RESET_ID);
	ASSERT(rpc_buf != NULL);

	err = wlc_rpc_call(rpc, rpc_buf);
	ASSERT(!err);
}
#endif /* AP */

#ifdef WL_MULTIQUEUE
void
wlc_tx_fifo_sync_complete(wlc_info_t *wlc, uint fifo_bitmap, uint8 flag)
{
	bcm_xdr_buf_t b;
	rpc_buf_t *rpc_buf;
	int err;
	rpc_info_t *rpc = wlc->rpc;

	/* signal upper layer to sync rpc tx fifo */
	rpc_buf = wlc_rpc_buf_alloc(rpc, &b, sizeof(uint32) * 2,
		WLRPC_WLC_BMAC_TX_FIFO_SYNC_COMPLETE_ID);
	ASSERT(rpc_buf != NULL);

	err = bcm_xdr_pack_uint32(&b, fifo_bitmap);
	ASSERT(!err);

	err = bcm_xdr_pack_uint32(&b, flag);
	ASSERT(!err);

	err = wlc_rpc_call(rpc, rpc_buf);
	ASSERT(!err);
}

#endif /* WL_MULTIQUEUE */

#ifdef WLMCNX
void
wlc_p2p_int_proc(wlc_info_t *wlc, uint8 *p2p_interrupts, uint32 tsf_l, uint32 tsf_h)
{
	bcm_xdr_buf_t b;
	rpc_buf_t *rpc_buf;
	int err;
	rpc_info_t *rpc = wlc->rpc;

	/* signal upper layer to process p2p interrupts */
	rpc_buf = wlc_rpc_buf_alloc(rpc, &b, ROUNDUP((sizeof(uint8) * M_P2P_BSS_MAX)
		+ sizeof(uint32) * 2, sizeof(uint32)), WLRPC_WLC_P2P_INT_PROC_ID);
	ASSERT(rpc_buf != NULL);

	err = bcm_xdr_pack_uint32(&b, tsf_l);
	ASSERT(!err);

	err = bcm_xdr_pack_uint32(&b, tsf_h);
	ASSERT(!err);

	err = bcm_xdr_pack_uint8_vec(&b, p2p_interrupts, M_P2P_BSS_MAX);
	ASSERT(!err);

	err = wlc_rpc_call(rpc, rpc_buf);
	ASSERT(!err);
}
#endif /* WLMCNX */

#if defined(BCMDBG) || defined(WLMCHAN) || defined(SRHWVSDB)
/** signals upper layer */
void wlc_tsf_adjust(wlc_info_t *wlc, int delta)
{
	bcm_xdr_buf_t b;
	rpc_buf_t *rpc_buf;
	int err;
	rpc_info_t *rpc = wlc->rpc;

	rpc_buf = wlc_rpc_buf_alloc(rpc, &b, sizeof(uint32), WLRPC_WLC_TSF_ADJUST);
	ASSERT(rpc_buf != NULL);

	err = bcm_xdr_pack_uint32(&b, delta);
	ASSERT(!err);

	err = wlc_rpc_call(rpc, rpc_buf);
	ASSERT(!err);
}
#endif /* BCMDBG || WLMCHAN || SRHWVSDB */

/** signals upper layer */
void wlc_srvsdb_switch_ppr(wlc_info_t *wlc, chanspec_t new_chanspec, bool last_chan_saved,
	bool switched)
{
	bcm_xdr_buf_t b;
	rpc_buf_t *rpc_buf;
	int err;
	rpc_info_t *rpc = wlc->rpc;

	rpc_buf = wlc_rpc_buf_alloc(rpc, &b, 3 * sizeof(uint32), WLRPC_WLC_SRVSDB_SWITCH_PPR);
	ASSERT(rpc_buf != NULL);

	err = bcm_xdr_pack_uint32(&b, (uint32)new_chanspec);
	ASSERT(!err);

	err = bcm_xdr_pack_uint32(&b, (uint32)last_chan_saved);
	ASSERT(!err);

	err = bcm_xdr_pack_uint32(&b, (uint32)switched);
	ASSERT(!err);

	err = wlc_rpc_call(rpc, rpc_buf);
	ASSERT(!err);
}
