/*
 * Generic Broadcom Home Networking Division (HND) DMA receive routines.
 * This supports the following chips: BCM42xx, 44xx, 47xx .
 *
 * Copyright (C) 2025, Broadcom. All Rights Reserved.
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
 *
 * <<Broadcom-WL-IPTag/Open:>>
 *
 * $Id: hnddma_rx.c 848971 2024-12-05 08:31:34Z $
 */

/**
 * @file
 * @brief
 * Source file for HNDDMA module. This file contains the functionality for the RX data path.
 */

#include <bcm_cfg.h>
#include <typedefs.h>
#include <bcmdefs.h>
#include <bcmdevs.h>
#include <bcmpcie.h>
#include <osl.h>
#include <bcmendian.h>
#include <hndsoc.h>
#include <bcmutils.h>
#include <siutils.h>

#include <sbhnddma.h>
#include <hnddma.h>
#include "hnddma_priv.h"
#ifdef BCM_HWALITE
#include "mlc_export.h"
#endif /* BCM_HWALITE */

#if defined(STS_XFER_PHYRXS)
#include <hndd11.h>
#endif /* STS_XFER_PHYRXS */

#ifdef BCM_PCAP
#include <wlc_pcap.h>
#include <bcm_pcap.h>
#endif /* BCM_PCAP */

#if !defined(OSL_CACHE_COHERENT)
#if defined(STB) || defined(BCM_ROUTER)
#define OSL_CACHE_INV_RX
#endif
#endif  /* ! OSL_CACHE_COHERENT */

/* Packets allocated from BPM have their data buffers pre-cache-invalidated */
#if defined(BCM_NBUFF_PKT) && (defined(CONFIG_BCM_BPM) || \
	defined(CONFIG_BCM_BPM_MODULE))
#define NBUFF_PKT_BPM /* Packet data buffers, are from BPM pool */
#endif /* BCM_NBUFF_PKT && CONFIG_BCM_BPM */

#if defined(D11_SPLIT_RX_FD)
static bool dma_splitrxfill(dma_info_t *di);
#endif

#ifdef BULKRX_PKTLIST
static void *_dma_getnext_rxlist(dma_info_t *di, uint nbound, uint *rxfifocnt, bool forceall,
	bool is_aux, uint32 mlo_unit);
static void _dma_rx_prepare_rxlist(dma_info_t *di, void *head, rx_list_t *rx_list,
	bool is_aux, uint32 mlo_unit);
static void _dma_get_rx_proc_params(dma_info_t *di, uint nbound, bool forceall,
	bool is_aux, uint32 mlo_unit, uint32 *nrxd_p, uint16 *rxin_p, uint16 *rxout_p,
	uint16 *curr_p, uint16 *nproc_p, dma64dd_t **_rxd64_p, void ***rxpdat_p);
#endif /* BULKRX_PKTLIST */

#ifdef BCM_PCAP
static bool BCMFASTPATH dma_pcap_rxfill(hnddma_t *dmah);
static void BCMFASTPATH dma_pcap_rxreclaim(hnddma_t *dmah);
static uint16 BCMFASTPATH dma_pcap_rx_rs0cd_fixup(dma_info_t *di, uint16 rs0cd_hw);
#endif /* BCM_PCAP */

static void
_dma_update_num_dd(dma_info_t *di, int *num_dd)
{
#if defined(D11_SPLIT_RX_FD)
	/* Descriptors will be jumped by 2 in
	 * case of sep_rxhdr and 1 otherwise.
	 */
	if (di->sep_rxhdr) {
		*num_dd -= 2;
	} else
#endif /* D11_SPLIT_RX_FD */
	{
		*num_dd -= 1;
	}
}

#ifdef BULKRX_PKTLIST

#if defined(STS_XFER_PHYRXS_PKT)
/* For 6726/6765, PhyRx Status is transferred in headroom for Management frames but
 * dma_flags in HW RxStatus (d11rxhdr_t::dma_flags) is not set accordingly.
 * SW WAR to reset PhyRx Status length and read for non-zero
 */
void
dma_set_phyrxs_dma_flags_war(hnddma_t *dmah, bool phyrxs_dma_flags_war)
{
	dma_info_t *di = DI_INFO(dmah);
	ASSERT(di);

	di->phyrxs_dma_flags_war = phyrxs_dma_flags_war;
}
#endif /* STS_XFER_PHYRXS_PKT */

#if defined(STS_XFER_PHYRXS_SEQ)
/**
 * PhyRx Status of Rx frames for this FIFO are transferred separately
 * using RxFIFO-3 or M2MDMA Ch#3.
 */
void
dma_set_sts_xfer_phyrxs(hnddma_t *dmah, bool sts_xfer_phyrxs)
{
	dma_info_t *di = DI_INFO(dmah);
	ASSERT(di);

	di->sts_xfer_phyrxs = sts_xfer_phyrxs;

}
#endif /* STS_XFER_PHYRXS_SEQ */

#if defined(STS_XFER_PHYRXS_MODE_FIFO)
/*
 * ------------------------------------------------------------------------------------------------
 * Section: PhyRx Status FIFO (STS_FIFO) dma handlers
 * ------------------------------------------------------------------------------------------------
 */

/** Intialize PhyRx Status ring memory used to post buffers to STS_FIFO DMA descriptors */
void dma_sts_set_memory(hnddma_t *dmah, sts_mem_type_t sts_mem_type,
	uint32 memaddr_hi, uint32 memaddr_lo)
{
	dma_info_t *di = DI_INFO(dmah);

	ASSERT(di);

	switch (sts_mem_type) {

#if !defined(STS_XFER_PHYRXS_MBX)
	case PHYRXS:

		PHYSADDRLOSET(di->phyrxsts_dmaaddr, memaddr_lo);
		PHYSADDRHISET(di->phyrxsts_dmaaddr, memaddr_hi);

		/* PhyRx Status buffers should be 8 byte aligned */
		ASSERT(PHYSADDRISZERO(di->phyrxsts_dmaaddr) ||
			ISALIGNED(memaddr_lo, D11PHYRXSTS_GE129_ALIGN_BYTES));

		break;

#else /* STS_XFER_PHYRXS_MBX */
	case PHYRXS_MBX:
		/* With HNDMBX, PhyRx Status ring is allocated in Host */

		HADDR64_LO_SET(di->phyrxsts_haddr64, memaddr_lo);
		HADDR64_HI_SET(di->phyrxsts_haddr64, memaddr_hi);

		if (!HADDR64_IS_ZERO(di->phyrxsts_haddr64)) {
			/* PhyRx Status buffers should be 8 byte aligned */
			ASSERT(ISALIGNED(memaddr_lo, D11PHYRXSTS_GE129_ALIGN_BYTES));
			/* Set 64bit high addr for Host memory */
			HADDR64_HI_SET(di->phyrxsts_haddr64,
				HADDR64_HI(di->phyrxsts_haddr64) | PCI64ADDR_HIGH);
		}
		break;
#endif /* STS_XFER_PHYRXS_MBX */

	default:
		DMA_ERROR(("%s: %s: Unsupported STS memory type[%d]\n",
			__FUNCTION__, di->name, sts_mem_type));
		ASSERT(0);
	}
} /* dma_sts_set_memory() */

/**
 * Post PhyRx Status buffers to STS_FIFO DMA descriptors
 * - chip lastdscr pointer will be updated in dma_rxfill
 * - Buffers will be cache invalidated during memory alloc (wlc_sts_xfer_attach).
 *
 * Note: Descriptors per packet should be always 1
 */
void
dma_sts_rxinit(hnddma_t *dmah)
{
	dma_info_t	*di = DI_INFO(dmah);
	uint32		rxout, flags = 0;
	uint32		iter;
#if defined(STS_XFER_PHYRXS_MBX)
	haddr64_t	haddr64;
#else /* ! STS_XFER_PHYRXS_MBX */
	dmaaddr_t	dmaaddr;
#endif /* ! STS_XFER_PHYRXS_MBX */

	DMA_TRACE(("%s: dma_sts_rxinit\n", di->name));

	ASSERT(di);

	ASSERT(di->sep_rxhdr == 0);
	ASSERT(di->rxfill_suspended == 0);

	rxout = 0;
#if defined(STS_XFER_PHYRXS_MBX)
	HADDR64_SET(haddr64, di->phyrxsts_haddr64);
#else /* ! STS_XFER_PHYRXS_MBX */
	PHYSADDRHISET(dmaaddr, PHYSADDRHI(di->phyrxsts_dmaaddr));
	PHYSADDRLOSET(dmaaddr, PHYSADDRLO(di->phyrxsts_dmaaddr));
#endif /* ! STS_XFER_PHYRXS_MBX */

	for (iter = 0; iter < di->nrxd; iter++) {
		/* reset flags for each descriptor */
		flags = D64_CTRL1_SOF;

		/* Set "end of descriptor table" for last descriptor */
		if (rxout == (di->nrxd - 1))
			flags |= D64_CTRL1_EOT;

#if defined(STS_XFER_PHYRXS_MBX)
		/* Load Rx_FIFO3 descriptors with PhyRx Status host buffers */
		dma64_dd_upd_64_from_params(di, di->rxd64, haddr64, rxout, &flags, di->rxbufsize);
		HADDR64_LO_SET(haddr64, PHYSADDR64LO(haddr64) + di->rxbufsize);
#else /* ! STS_XFER_PHYRXS_MBX */
		dma64_dd_upd(di, di->rxd64, dmaaddr, rxout, &flags, di->rxbufsize);
		PHYSADDRLOSET(dmaaddr, PHYSADDRLO(dmaaddr) + di->rxbufsize);
#endif /* ! STS_XFER_PHYRXS_MBX */

		rxout = NEXTRXD(rxout);
	}

#if !defined(OSL_CACHE_COHERENT) && defined(BULK_DESCR_FLUSH)
	DMA_MAP(di->osh, dma64_rxd64(di, 0), DMA64_FLUSH_LEN(di, di->nrxd), DMA_TX, NULL, NULL);
#endif /* !OSL_CACHE_COHERENT && BULK_DESCR_FLUSH */

	/* Enable WaitForComplete (readback) for Rx FIFO-3 (PhyRx Status) */
	dma_rx_waitcomplete_enable(dmah, TRUE);

} /* dma_sts_rxinit() */

/**
 * Update chip lastdscr pointer
 * NOTE:
 * - PhyRx Buffers are cache invalidated in dma_sts_rx().
 * - D64_RC_WC is set for Rx-FIFO-3 to protect against INT racing ahead of DMA transfers.
 *   so need to reset and readback PhyRx Status len.
 */
bool BCMFASTPATH
dma_sts_rxfill(hnddma_t *dmah)
{
	dma_info_t	*di = DI_INFO(dmah);
	int		rxpost_rem;
	uint16		rxout, rxactive;

	if (di->rxfill_suspended)
		return FALSE;

	/*
	 * Determine how many receive buffers we're lacking from the full complement and
	 * then update the chip rx lastdscr.
	 */
	rxactive = NRXDACTIVE(di->rxin, di->rxout);

	/* If currently posted is more than requested, no need to do anything */
	if (rxactive >= di->nrxpost)
		return TRUE;

	/* No. of packets to post */
	rxpost_rem = di->nrxpost - rxactive;
	rxout = di->rxout;

	DMA_TRACE(("%s: dma_sts_rxfill: post %d\n", di->name, rxpost_rem));

	/* NEXTRXD */
	rxout = RXD(rxout + rxpost_rem);

#if !defined(OSL_CACHE_COHERENT) && defined(BULK_DESCR_FLUSH)
	{
		uint32 flush_cnt = NRXDACTIVE(di->rxout, rxout);
		if (rxout < di->rxout) {
			DMA_MAP(di->osh, dma64_rxd64(di, 0), DMA64_FLUSH_LEN(di, rxout),
					DMA_TX, NULL, NULL);
			flush_cnt -= rxout;
		}
		DMA_MAP(di->osh, dma64_rxd64(di, di->rxout), DMA64_FLUSH_LEN(di, flush_cnt),
				DMA_TX, NULL, NULL);
	}
#endif /* !OSL_CACHE_COHERENT && BULK_DESCR_FLUSH */

	/* update the chip lastdscr pointer */
	DMA64_RX_LD_UPD(di, rxout);
	di->rxout = rxout;

	return TRUE;
} /* dma_sts_rxfill() */

/**
 * PhyRx Status Rx entry routine
 * - Returns index(rs0cd) of last received PhyRx Status buffer.
 * Note:
 * - PhyRx Status buffers are invalidated in STS_XFER module (__phyrxs_ring_write_update).
 * - di->rxin will be updated in dma_sts_rx_done
 */
uint16 BCMFASTPATH
dma_sts_rx(hnddma_t *dmah)
{
	dma_info_t	*di = DI_INFO(dmah);

	ASSERT(di->nrxd != 0);

	/* Get current descriptor pointer */
	di->rs0cd = DMA64_RX_CD(di);

	return di->rs0cd;	/* Write index of PhyRx Status ring */
} /* dma_sts_rx() */

/**
 * Updates di->rxin.
 * Invoked by consumer (WLAN) afer processing PhyRx Statuses
 */
void BCMFASTPATH
dma_sts_rx_done(hnddma_t *dmah, uint16 rxin)
{
	dma_info_t *di = DI_INFO(dmah);

	di->rxin = rxin;
	di->rxavail = di->nrxd - NRXDACTIVE(di->rxin, di->rxout) - 1;
} /* dma_sts_rx_done() */

/** Reclaim PhyRx Status buffers */
void  BCMFASTPATH
dma_sts_rxreclaim(hnddma_t *dmah)
{
	dma_info_t *di = DI_INFO(dmah);
	uint16	iter;

	DMA_TRACE(("%s: dma_sts_rxreclaim\n", di->name));

	if (di->nrxd == 0)
		return;

#ifdef BCM_BACKPLANE_TIMEOUT
	if (!si_deviceremoved(di->sih))	{
		/* if forcing, dma engine must be disabled */
		ASSERT(!dma64_rxenabled(di));
	}
#endif /* BCM_BACKPLANE_TIMEOUT */

#ifdef PP_XPM
	ASSERT(!di->pp_xpm_enab);
#endif /* PP_XPM */
	iter = di->rxin;

	while (iter != di->rxout) {

#if !defined(OSL_CACHE_COHERENT)
		dmaaddr_t pa;

		PHYSADDRLOSET(pa,
			(BUS_SWAP32(R_SM(&di->rxd64[iter].addrlow)) - di->dataoffsetlow));
		PHYSADDRHISET(pa,
			(BUS_SWAP32(R_SM(&di->rxd64[iter].addrhigh)) - di->dataoffsethigh));

		/* Cache invalidate entire PhyRx Status buffer */
		DMA_UNMAP(di->osh, pa, di->rxbufsize, DMA_RX, NULL, NULL);
#endif /* ! OSL_CACHE_COHERENT */

		iter = NEXTRXD(iter);
	}

	di->rxin = iter;
	di->rxavail = di->nrxd - NRXDACTIVE(di->rxin, di->rxout) - 1;
} /* dma_sts_rxreclaim() */

uint16
dma_sts_rxin(hnddma_t *dmah)
{
	dma_info_t *di = DI_INFO(dmah);
	return di->rxin;
}

#endif /* STS_XFER_PHYRXS_MODE_FIFO */

#ifdef PP_XPM
/* Calculate pull offset based on the extraheadroom, alignment requirement for
 * a rxpost buffer
 *
 * This offset will be used for PKTPULL
 */
static void
dma_datapull_offset_len(dma_info_t *di, uintptr addr, uint *pull_offset, uint *pktlen)
{
	uint alignment_req = 8;
	uint extra_offset = 0;
	uint extra_pad = 0;
	uint len;

	if (di->rxbufsize > BCMEXTRAHDROOM)
		extra_offset = di->rxextrahdrroom;

	len = di->rxbufsize + extra_offset + alignment_req - 1;

	/* Calculate padding if addr or extra_offset is not aligned */
	if (!ISALIGNED((addr + extra_offset), alignment_req)) {
		extra_pad = (alignment_req - ((addr + extra_offset) % alignment_req));

		DMA_TRACE(("[%s:%d] %s Computing headroom/len rxbufsize %d extrahdroom %d "
			"rxbufsize %d BCMEXTRAHDROOM %d extra_pad %d extra_offset %d "
			"addr %p\n",
			__FUNCTION__, __LINE__, di->name, di->rxbufsize, di->rxextrahdrroom,
			di->rxbufsize, BCMEXTRAHDROOM, extra_pad, extra_offset, (void *)addr));
	}

	/* Adjust extra_offset based on padding requirement */
	if (extra_pad) {
		extra_offset += extra_pad;
		len = len & ~(alignment_req - 1);

		DMA_TRACE(("[%s:%d] %s Computing headroom/len rxbufsize %d extrahdroom %d "
			"rxbufsize %d BCMEXTRAHDROOM %d extra_pad %d extra_offset %d "
			"addr %p\n",
			__FUNCTION__, __LINE__, di->name, di->rxbufsize, di->rxextrahdrroom,
			di->rxbufsize, BCMEXTRAHDROOM, extra_pad, extra_offset, (void *)addr));
	}

	*pktlen = len;
	*pull_offset = extra_offset;

	DMA_TRACE(("[%s:%d] %s Computing headroom/len rxbufsize %d extrahdroom %d "
		"rxbufsize %d BCMEXTRAHDROOM %d extra_pad %d extra_offset %d addr %p\n",
		__FUNCTION__, __LINE__, di->name, di->rxbufsize, di->rxextrahdrroom,
		di->rxbufsize, BCMEXTRAHDROOM, extra_pad, extra_offset, (void *)addr));

	return;
} /* dma_datapull_offset_len */

/* Prepare a list of packets for nproc buffers
 * - A new packet context structure (XKB) will be fetched and linked with the buffer
 *   already posted into the rings.
 * - Packet data will be adjusted based on the headroom and padding requirement
 * - Multiple packets will be linked using PKTLINK
 */
static uint16
dma_pp_xpm_rx_prepare(dma_info_t *di, uint16 *nproc_orig, uint16 di_rxin, uint16 di_rxout,
	uint32 nrxd, void **rxpdata, void **pktlist_head, void **pktlist_tail)
{
	uint16 rxp_idx;
	void *tail = NULL;
	uint32 len, pull_offset;
	void *pnbuff;
	uint i, nproc = 0;

	DMA_TRACE(("[%s:%d] %s Start preparing rxin %d rxout %d pktlist [head %p tail %p]\n",
		__FUNCTION__, __LINE__, di->name, di_rxin, di_rxout,
		*pktlist_head, *pktlist_tail));

	tail = *pktlist_tail;
	ASSERT(tail == NULL);

	rxp_idx = di_rxin;

	for (i = 0; i < *nproc_orig; i++) {
		/* Get the pull_offset for the buffer posted at this index,
		 * need to pull pkt for the same offset
		 */
		dma_datapull_offset_len(di, (uintptr)rxpdata[rxp_idx], &pull_offset, &len);

#ifdef BCM_SPC_RX
		if (di->spc_rx_enab) {
			pnbuff = PP_XPM_ATTACH_XKB(di->osh, rxpdata[rxp_idx], pull_offset, len);

			ASSERT(pnbuff);

			/* Prefetch RxStatus and 802.11 header */
			OSL_PREFETCH(PKTSPCRXDATA(di->osh, pnbuff));
			OSL_PREFETCH(PKTSPCRXDATA(di->osh, pnbuff) + 64);

			/* Cnvert to PKTLINK */
			PKTSPCRXSETLINK(pnbuff, NULL);
			if (tail) {
				PKTSPCRXSETLINK(tail, pnbuff);
				*pktlist_tail = tail = pnbuff;
			} else {
				ASSERT((*pktlist_head == NULL));
				*pktlist_head = tail = pnbuff;
				*pktlist_tail = tail;
			}
		} else
#endif /* BCM_SPC_RX */
		{
			pnbuff = PP_XPM_SKB_GET(di->osh);

			if (pnbuff == NULL) {
				/* Free up the Data buffer */
				di->hnddma.rxnobuf++;
				PP_XPM_DATABUF_FREE(di->osh, rxpdata[rxp_idx]);
				rxp_idx = NEXTRXD_MLO(rxp_idx, nrxd);
				continue;
			}

			/* Attach SKB to rxpdata[rxp_idx], skb_pull is done inside
			 * PP_XPM_ATTACH_SKB based on the pull_offset specified
			 */
			PP_XPM_ATTACH_SKB(di->osh, pnbuff, rxpdata[rxp_idx],
				pull_offset, len);

			ASSERT(len);

			/* Convert to PKTLINK */
			PKTSETLINK(pnbuff, NULL);
			if (tail) {
				PKTSETLINK(tail, pnbuff);
				*pktlist_tail = tail = pnbuff;
			} else {
				ASSERT((*pktlist_head == NULL));
				*pktlist_head = tail = pnbuff;
				*pktlist_tail = tail;
			}
		}

		DMA_TRACE(("[%s:%d] %s linked pnbuff %p pData %p idx %d "
			"headroom %d len %d\n",
			__FUNCTION__, __LINE__, di->name, pnbuff,
			rxpdata[rxp_idx], rxp_idx,
			pull_offset, len));

		rxp_idx = NEXTRXD_MLO(rxp_idx, nrxd);
		nproc++;
	}

	DMA_TRACE(("[%s:%d] %s prepared %d pkts rxin %d rxout %d pktlist [head %p tail %p]\n",
		__FUNCTION__, __LINE__, di->name, nproc, di_rxin, di_rxout,
		*pktlist_head, *pktlist_tail));

	*nproc_orig = nproc;

	return rxp_idx;
} /* dma_pp_xpm_rx_prepare */

/* Fetch a databuf from BPM/MPM pool and post to the descriptor ring.
 * Packet Context Structure is not attached to the data buffer at this stage. This is to
 * reduce the cache misses in the system. If we access both SKB and
 * databuf, both of them are pulled into cache evicting others which
 * is not desirable
 */
static bool BCMFASTPATH
_dma_pp_xpm_rxfill(hnddma_t *dmah, uint16 *di_rxin, uint16 *di_rxout,
	dma64dd_t *_rxd64, uint32 rcvptrbase, uint32 nrxd, uint32 nrxpost,
	void **rxpdata, uint fifo)
{
	dma_info_t *di = DI_INFO(dmah);
	void *pdatabuf;
	uint16 rxin, rxout;
	uint16 rxactive;
	uint32 flags = 0;
	uint n = 0, i = 0;
	dmaaddr_t pa;
	bool ring_empty;

	/* PP_XPM APIs are not matured enough to handle
	 * all these combinations and legacy chips
	 */
	ASSERT(!DMA_HWA_RX(di));
	ASSERT(!di->sep_rxhdr);
	ASSERT(di->hnddma.dmactrlflags & DMA_CTRL_RX_ALIGN_8BYTE);
	ASSERT(!di->d11rx_war);
	ASSERT(!di->phyrxs_dma_flags_war);
	ASSERT(DMA_BULKRX_PATH(di));
	ASSERT(!DMA_PCAP_EN(di));
	ASSERT(di->pp_xpm_enab);

	ring_empty = FALSE;

	/*
	 * Determine how many receive buffers we're lacking
	 * from the full complement, allocate, initialize,
	 * and post them, then update the chip rx lastdscr.
	 */

	rxin = *di_rxin;
	rxout = *di_rxout;
	rxactive = NRXDACTIVE_MLO(rxin, rxout, nrxd);

	/* if currently posted is more than requested, no need to do anything */
	if (MIN((nrxd), nrxpost) <= rxactive) {
		goto done;
	}

	/* No. of packets to post is : n = MIN(nrxd, nrxpost) - NRXDACTIVE;
	 */
	n = MIN(nrxd, nrxpost) - rxactive;

	DMA_TRACE(("%s: %s: post %d\n", __FUNCTION__, di->name, n));

	DMA_TRACE(("[%s:%d] %s fifo %d Start filling %d bufs rxin %d rxout %d\n",
		__FUNCTION__, __LINE__, di->name, fifo, n, *di_rxin, *di_rxout));

	for (i = 0; i < n; i++) {
		int len = 0;
		uint pull_offset;

		pdatabuf = PP_XPM_DATABUF_GET(di->osh, len);

		if (pdatabuf == NULL) {
			DMA_TRACE(("%s: %s: dma_rxfill: out of rxbufs\n",
				__FUNCTION__, di->name));

			if (i == 0) {
				if (dma_rxidle(dmah)) {
					DMA_TRACE(("%s: rxfill64: ring is empty !\n",
						di->name));
					ring_empty = TRUE;
				}
			}
			di->hnddma.rxnobuf += (n - i);
#ifdef BPM_BULK_FREE
			linux_commit_skb_freelist(di->osh);
#endif /* BPM_BULK_FREE */
			break;
		}

		/* save pdata pointer
		 * Pointer saved in pdata array is before the offset adjustment
		 * This should be the pData received from MPM/BPM
		 */
		rxpdata[rxout] = pdatabuf;

		dma_datapull_offset_len(di, (uintptr)pdatabuf, &pull_offset, &len);

		/* Move data pointer as per offset and alignment requirement
		 * This needs to be done before resetting the fist 4 bytes
		 */
		pdatabuf = (void*)((uintptr)(pdatabuf) + pull_offset);

		ASSERT(ISALIGNED((uintptr)pdatabuf, 8));

		DMA_TRACE(("[%s:%d] %s fifo %d filling idx %d with databuf %p rxin %d "
			"rxout %d pull_offset %d len %d aligned %d\n",
			__FUNCTION__, __LINE__, di->name, fifo, i, rxpdata[rxout],
			rxin, rxout, pull_offset, len, ISALIGNED((uintptr)pdatabuf, 8)));

		if (!di->rx_waitcomplete_enabled) {
			*(uint32 *)(pdatabuf) = 0;

#if !defined(OSL_CACHE_COHERENT) && defined(linux) && (defined(__mips__) || \
	defined(BCM_ROUTER))
			DMA_MAP(di->osh, pdatabuf, sizeof(uint32), DMA_TX, NULL, NULL);
#endif
		}

		OSL_VIRT_TO_PHYSADDR(di->osh, pdatabuf, pa);

		ASSERT(ISALIGNED(PHYSADDRLO(pa), 8));

		/* reset flags for each descriptor */
		flags = D64_CTRL1_SOF;

		if (rxout == (nrxd - 1))
			flags |= D64_CTRL1_EOT;

		dma64_dd_upd(di, _rxd64, pa, rxout, &flags, di->rxbufsize);

		rxout = NEXTRXD_MLO(rxout, nrxd);
	} /* for n */

#if !defined(BULK_DESCR_FLUSH)
	*di_rxout = rxout;
#endif

#if !defined(OSL_CACHE_COHERENT)
#if defined(BULK_DESCR_FLUSH)
	{
		uint32 flush_cnt = NRXDACTIVE_MLO(*di_rxout, rxout, nrxd);

		if (rxout < *di_rxout) {
			DMA_MAP(di->osh, &_rxd64[0], (sizeof(dma64dd_t) * rxout),
				DMA_TX, NULL, NULL);

			flush_cnt -= rxout;
		}

		DMA_MAP(di->osh, &_rxd64[*di_rxout], (sizeof(dma64dd_t) * flush_cnt),
			DMA_TX, NULL, NULL);
	}
#endif /* BULK_DESCR_FLUSH */
#endif /* ! OSL_CACHE_COHERENT */

#if defined(BULK_DESCR_FLUSH)
	*di_rxout = rxout;
#endif

	ASSERT(rxin == *di_rxin);

#ifdef BCM_HWALITE
	/* update the chip lastdscr pointer */
	if (mlc_hwalite_is_enable(di->mlc_dev)) {
		/* MLO LD UPDATE is index based only */
		mlc_hwalite_rxfifo_ld_upd(di->mlc_dev, fifo,
			(uint16)(B2I(rcvptrbase, sizeof(dma64dd_t)) + rxout));
	} else
#endif
	{
		DMA64_RX_LD_UPD(di, rxout);
	}

	DMA_TRACE(("[%s:%d] %s fifo %d Filled %d databufs rxin %d rxout %d\n",
		__FUNCTION__, __LINE__, di->name, fifo, n, rxin, rxout));

done:
	return !ring_empty;
} /* _dma_pp_xpm_rxfill */

#ifdef BCM_HWALITE
static bool BCMFASTPATH
_dma_pp_xpm_mlo_aux_rxfill(hnddma_t *dmah, uint32 mlo_unit)
{
	dma_info_t *di = DI_INFO(dmah);
	dma_mlo_aux_info_t *aux_info = NULL;
	uint64	fifo_addr_va;
	uint32	nrxd, nrxpost, rcvptrbase;
	dma64dd_t *_rxd64;
	bool ret = FALSE;

	/* get parames for aux */
	ASSERT(mlo_unit <= DMA_MAX_MLO_AUX_LINKS);
	aux_info = &di->aux_info[mlo_unit - 1];

	ASSERT(aux_info);
	ASSERT(aux_info->rxpdata);

	mlc_hwalite_get_aux_rxdma_info(di->mlc_dev,
		mlo_unit, &nrxd, &nrxpost, &fifo_addr_va, &rcvptrbase);
	_rxd64 = (dma64dd_t*)((uintptr)fifo_addr_va);

	ret = _dma_pp_xpm_rxfill(dmah, &aux_info->rxin, &aux_info->rxout,
		_rxd64, rcvptrbase, nrxd, nrxpost, aux_info->rxpdata,
		MLO_MLOUNIT_TO_RXFIFO(mlo_unit));

	return ret;
} /* _dma_pp_xpm_mlo_aux_rxfill */
#endif /* BCM_HWALITE */

static bool BCMFASTPATH
dma_pp_xpm_rxfill(hnddma_t *dmah)
{
	dma_info_t *di = DI_INFO(dmah);
	bool ret = FALSE;
	uint fifo = 0xff;

#ifdef BCM_HWALITE
	fifo = di->fifo;
#endif /* BCM_HWALITE */

	ret = _dma_pp_xpm_rxfill(dmah, &di->rxin, &di->rxout, di->rxd64,
		di->rcvptrbase, di->nrxd, di->nrxpost, di->rxpdata, fifo);

#ifdef BCM_HWALITE
	if ((mlo_role(di->mlc_dev) == MLC_ROLE_MAP) && (di->fifo == RX_MLO_FIFO3)) {
		uint32 mlo_unit;

		ASSERT(mlc_hwalite_is_enable(di->mlc_dev));
		ASSERT(di->hnddma.dmactrlflags & DMA_CTRL_MLO_RXFIFO);

		/* Post buffers to Aux Fifo */
		FOREACH_MLO_AAP(di->mlc_dev, mlo_unit) {
			ret = _dma_pp_xpm_mlo_aux_rxfill(dmah, mlo_unit);
		}
	}
#endif /* BCM_HWALITE */

	return ret;
} /* dma_pp_xpm_rxfill */

/* Reclaim databuffers posted to RxDescriptors
 *
 * No XKBs are attached to databuffers and buffers are directly recycled to XPM pool.
 */
void BCMFASTPATH
_dma_pp_xpm_rxreclaim(dma_info_t *di, bool is_aux, uint32 mlo_unit)
{
	void	**rxpdata;
#ifdef BCM_HWALITE
	dma_mlo_aux_info_t *aux_info = NULL;
#endif /* BCM_HWALITE */
	uint32	nrxd;
	uint16	rxin, rxout;
	uint16	nproc, curr;

	_dma_get_rx_proc_params(di, 0, TRUE, is_aux, mlo_unit,
		&nrxd, &rxin, &rxout, &curr, &nproc, NULL, &rxpdata);

	/* Free up data buffers */
	while (rxin != curr) {
		PP_XPM_DATABUF_FREE(di->osh, rxpdata[rxin]);
		rxin = NEXTRXD_MLO(rxin, nrxd);
		nproc--;
	}

	ASSERT(nproc == 0);
	ASSERT(rxin == rxout);
	ASSERT(curr == rxout);

#ifdef BCM_HWALITE
	if (is_aux) {
		ASSERT(mlo_unit <= DMA_MAX_MLO_AUX_LINKS);
		aux_info = &di->aux_info[mlo_unit - 1];
		aux_info->rs0cd	= curr;
		aux_info->rxin	= rxin;
	} else
#endif /* BCM_HWALITE */
	{
		di->rs0cd	= curr;
		di->rxin	= rxin;
		di->rxavail	= nrxd - NRXDACTIVE_MLO(rxin, rxout, nrxd) - 1;
	}
} /* _dma_pp_xpm_rxreclaim */

/** returns list of entries on the ring, in the order in which they were placed on the ring
 * PP_XPM equivalent of _dma_getnext_rxlist()
 */
static void * BCMFASTPATH
_dma_pp_xpm_getnext_rxlist(dma_info_t *di, uint nbound, uint *rxfifocnt, bool forceall,
	bool is_aux, uint32 mlo_unit)
{
	void	**rxpdata;
	void	*head = NULL, *tail = NULL;
#ifdef BCM_HWALITE
	dma_mlo_aux_info_t *aux_info = NULL;
#endif /* BCM_HWALITE */
	uint32	nrxd;
	uint16	rxin, rxout;
	uint16	nproc, curr;

	/* PP_XPM APIs are not matured enough to handle
	 * all these combinations and legacy chips
	 */
	ASSERT(!DMA_HWA_RX(di));
	ASSERT(!di->sep_rxhdr);
	ASSERT(di->hnddma.dmactrlflags & DMA_CTRL_RX_ALIGN_8BYTE);
	ASSERT(!di->d11rx_war);
	ASSERT(!di->phyrxs_dma_flags_war);
	ASSERT(DMA_BULKRX_PATH(di));
	ASSERT(!DMA_PCAP_EN(di));

	ASSERT(forceall == FALSE);

	_dma_get_rx_proc_params(di, nbound, forceall, is_aux, mlo_unit,
		&nrxd, &rxin, &rxout, &curr, &nproc, NULL, &rxpdata);
	if ((nrxd == 0) || (nproc == 0)) {
		return NULL;
	}

	/* Update actual number of buffers retrieved from RxFIFO decriptors */
	if (rxfifocnt != NULL) {
		*rxfifocnt += nproc;
	}

	rxin = dma_pp_xpm_rx_prepare(di, &nproc, rxin, rxout, nrxd, rxpdata, &head, &tail);

#ifdef BCM_HWALITE
	if (is_aux) {
		ASSERT(mlo_unit <= DMA_MAX_MLO_AUX_LINKS);
		aux_info = &di->aux_info[mlo_unit - 1];
		aux_info->rs0cd	= curr;
		aux_info->rxin	= rxin;
	} else
#endif /* BCM_HWALITE */
	{
		di->rs0cd	= curr;
		di->rxin	= rxin;
		di->rxavail	= nrxd - NRXDACTIVE_MLO(rxin, rxout, nrxd) - 1;
	}

	if (tail) {
#ifdef BCM_SPC_RX
		if (di->spc_rx_enab) {
			PKTSPCRXSETLINK(tail, NULL);
		} else
#endif /* BCM_SPC_RX */
		{
			PKTSETLINK(tail, NULL);
		}
	}

	return head;

} /* _dma_pp_xpm_getnext_rxlist */
#endif /* PP_XPM */

/**
 * BULKRX_PKTLIST specific function
 * @param[inout] rx_list  Linked list of mpdus that were received
 */
/**
 * !! rx entry routine
 * returns a pointer to the list of next frame/frames received, or NULL if there are no more
 * DMA_CTRL_RXMULTI and DMA scattering(multiple buffers) is not supported yet
 */
void BCMFASTPATH
dma_rx(hnddma_t *dmah, rx_list_t *rx_list, uint nbound)
{
	dma_info_t *di = DI_INFO(dmah);
	void	*head;

	rx_list->rxfifocnt = 0;
	head = _dma_getnext_rxlist(di, nbound, &rx_list->rxfifocnt,
		FALSE, FALSE, DMA_MLO_UNIT_INVALID);

	_dma_rx_prepare_rxlist(di, head, rx_list, FALSE, DMA_MLO_UNIT_INVALID);
}

/* Preparing packets received in multiple descriptors - PKTLINK has to be broken
 * and the fragments has to be linked using PKTNEXT.
 *
 * However, driver does not support such format, we free up these packets down the line
 */
static void
dma_rx_multi_buf_prepare_rxlist(dma_info_t *di, uint *rxfifocnt, void *head, int *_resid,
	uint8 tcm_maxsize, bool is_aux, uint32 mlo_unit)
{
	void *tail = NULL, *p = NULL;
	uint pkt_len;
	int resid = *_resid;
	uint32 rx_frm_sts;
	int num_dd; /* Number of DMA descriptors used for this packet. */

	rx_frm_sts = ltoh32(*(uint32 *)PKTDATA(di->osh, head));
	num_dd = ((rx_frm_sts & D64_RX_FRM_STS_DSCRCNT) >> D64_RX_FRM_STS_DSCRCNT_SHIFT);

	/* Receive Frame Status Header DscrCntr (27:24) Contains one less
	 * than the number of DMA descriptors used to transfer this frame
	 * into memory. So incrementing it by 1 will give the actual number
	 * of DMA descriptors used for this transfer.
	 */
	num_dd++;
	_dma_update_num_dd(di, &num_dd);

	tail = head;

	while (num_dd > 0) {
		if (PKTLINK(tail) != NULL) {
			p = PKTLINK(tail);
		} else {
#ifdef WLSPP_NIC
			/* DMA Rx need more packets than the budget
			 * Trigger SPP to advance cd_peek and feed new packets
			 * to SPP scheduler
			 */
			if (di->spp_nic_enab) {
				di->spp_cb(di->spp_ctx, di->fifo);
			}
#endif /* WLSPP_NIC */

			p = _dma_getnext_rxlist(di, 1, rxfifocnt, FALSE, is_aux, mlo_unit);
		}

		PKTSETNEXT(di->osh, tail, p);
		if (p == NULL)
			break;

		pkt_len = MIN(resid, (int)di->rxbufsize);
#if defined(D11_SPLIT_RX_FD)
		if (di->sep_rxhdr) {
			PKTSETLEN(di->osh, p, MIN(pkt_len, tcm_maxsize));
		} else
#endif /* D11_SPLIT_RX_FD */
		{
			PKTSETLEN(di->osh, p, pkt_len);
		}

		PKTSETLINK(tail, NULL);
		tail = p;
		resid -= di->rxbufsize;
		/* Descriptors will be jumped by 2 in
		 * case of sep_rxhdr and 1 otherwise.
		 */
		_dma_update_num_dd(di, &num_dd);
	}

	if (head != tail) {
		PKTSETLINK(head, PKTLINK(tail));
		/* Terminate the link of the last fragments which is linked using PKTNEXT */
		PKTSETLINK(tail, NULL);
	}

#ifdef BCMDBG
	if (resid > 0) {
		DMA_ERROR(("%s: %s: mlo_unit %d rxin %d rxout %d, hw_curr %d\n",
			di->name, __FUNCTION__, mlo_unit,
			di->rxin, di->rxout, DMA64_RX_CD(di)));
		if (p != NULL) {
			DMA_ERROR(("%s: %s: all descriptors processed but only"
				" partial frame received (missing %d bytes)\n",
				di->name, __FUNCTION__, resid));
		}
	}
#endif /* BCMDBG */

	*_resid = resid;
} /* dma_rx_multi_buf_prepare_rxlist */

/* Free up giant packets received in multiple buffers - PKTLINK is adjusted
 * to point to next normal Rx buffers
 */
static void
dma_rx_multi_buf_free_rxlist(dma_info_t *di, void **head, void **head0, void *prev)
{
	void *free_p;

	free_p = *head;

	if (prev != NULL) {
		*head = PKTLINK(free_p);
		PKTSETLINK(prev, *head);
	} else {
		*head0 = *head = PKTLINK(free_p);
	}

	PKTSETLINK(free_p, NULL);
	PKTFREE(di->osh, free_p, FALSE);
} /* dma_rx_multi_buf_free_rxlist */

#ifdef BCM_SPC_RX
/* SPC varient to Prepare packets received in multiple descriptors -
 * PKTLINK has to be broken and the fragments has to be linked using PKTNEXT.
 *
 * However, driver does not support such format, we free up these packets down the line
 *
 * When SPC is enabled, all buffers are in xkb format and there is no PKTNEXT concept.
 * All non head received fragments are saved in an array and freed up at the end.
 * Head fragment is freed at the caller
 */
static void
dma_rx_spc_multi_buf_prepare_rxlist(dma_info_t *di, uint *rxfifocnt, void *head, int *_resid,
	uint8 tcm_maxsize, bool is_aux, uint32 mlo_unit)
{
	void *p = NULL, *link = NULL;
	uint pkt_len;
	int resid = *_resid;
	uint32 rx_frm_sts;
	int num_dd; /* number of dma descriptors used for this packet. */

	rx_frm_sts = ltoh32(*(uint32 *)PKTSPCRXDATA(di->osh, head));
	num_dd = ((rx_frm_sts & D64_RX_FRM_STS_DSCRCNT) >> D64_RX_FRM_STS_DSCRCNT_SHIFT);

	/* receive frame status header dscrcntr (27:24) contains one less
	 * than the number of dma descriptors used to transfer this frame
	 * into memory. so incrementing it by 1 will give the actual number
	 * of dma descriptors used for this transfer.
	 */
	num_dd++;
	_dma_update_num_dd(di, &num_dd);

	printf("[%s] Multi Buffer Rx on %s num_dd %d rx_frm_sts 0x%08x "
		"resid %d head offset/len %d/%d\n",
		__FUNCTION__, di->name, num_dd, rx_frm_sts, resid,
		PKTSPCRXOFFSET(di->osh, head), PKTSPCRXLEN(di->osh, head));
	prhex("head:", (uint8 *)PKTSPCRXDATA(di->osh, head), PKTSPCRXLEN(di->osh, head));

	link = PKTSPCRXLINK(head);

	while (num_dd > 0) {
		if (link != NULL) {
			p = link;
		} else {
#ifdef WLSPP_NIC
			/* DMA Rx need more packets than the SPP budget
			 * So trigger SPP to advance cd_peek and feed new packets
			 * to SPP scheduler to adjust the SPP accounting.
			 */
			if (di->spp_nic_enab) {
				di->spp_cb(di->spp_ctx, di->fifo);
			}
#endif /* WLSPP_NIC */

			p = _dma_getnext_rxlist(di, 1, rxfifocnt, FALSE, is_aux, mlo_unit);
			if (p == NULL) {
				break;
			}
		}

		ASSERT(PKTSPCRXDMAFMT(p));

		pkt_len = MIN(resid, (int)di->rxbufsize);

		PKTSPCRXSETLEN(di->osh, p, pkt_len);

		printf("Frag resid %d num_dd %d offset/len %d/%d\n",
			resid, num_dd, PKTSPCRXOFFSET(di->osh, p),
			PKTSPCRXLEN(di->osh, p));
		prhex("Frag", (uint8 *)PKTSPCRXDATA(di->osh, p), (int)di->rxbufsize);

		link = PKTSPCRXLINK(p);

		PKTFREE(di->osh, p, FALSE);

		resid -= di->rxbufsize;
		/* Descriptors will be jumped by 2 in
		 * case of sep_rxhdr and 1 otherwise.
		 */
		_dma_update_num_dd(di, &num_dd);
	}

	/* move PKTLINK to the packet after num_dd descriptors */
	PKTSPCRXSETLINK(head, link);

#ifdef BCMDBG
	if (resid > 0) {
		DMA_ERROR(("%s: %s: mlo_unit %d rxin %d rxout %d, hw_curr %d\n",
			di->name, __FUNCTION__, mlo_unit,
			di->rxin, di->rxout, DMA64_RX_CD(di)));
		if (p != NULL) {
			DMA_ERROR(("%s: %s: all descriptors processed but only"
				" partial frame received (missing %d bytes)\n",
				di->name, __FUNCTION__, resid));
		}
	}
#endif /* BCMDBG */
	*_resid = resid;
} /* dma_rx_spc_multi_buf_prepare_rxlist */

static void
dma_rx_spc_multi_buf_free_rxlist(dma_info_t *di, void **head, void **head0, void *prev)
{
	void *free_p;

	free_p = *head;

	if (prev != NULL) {
		*head = PKTSPCRXLINK(free_p);
		PKTSPCRXSETLINK(prev, *head);
	} else {
		*head0 = *head = PKTSPCRXLINK(free_p);
	}

	PKTSPCRXSETLINK(free_p, NULL);
	PKTFREE(di->osh, free_p, FALSE);
} /* dma_rx_spc_multi_buf_free_rxlist */
#endif /* BCM_SPC_RX */

static void
_dma_rx_prepare_rxlist(dma_info_t *di, void *head, rx_list_t *rx_list,
	bool is_aux, uint32 mlo_unit)
{
	void *head0, *prev = NULL;
	uint len;
	uint pkt_len;
	int resid = 0;
	uint tcm_maxsize = 0;		/* max size of tcm descriptor */
	void *data, *free_p;
	uint rxoffset;

	head0 = head;

#ifdef HW_HDR_CONVERSION
	/* Even with splitmode-4, dongle receives DMA done interrupt for both
	 * fifo-0 & fifo-1. So depending upon which DMA finishes first dongle
	 * firmware may pick up either di[0] or di[1] to start procssing the
	 * local packet. Always force fifo-1 rxoffset since fifo-0 rxoffset
	 * is invalid for local data section.
	 */
	if (di->split_fifo == SPLIT_FIFO_0) {
		rxoffset = di->linked_di->rxoffset;
	} else
#endif /* HW_HDR_CONVERSION */
	{
		rxoffset = di->rxoffset;
	}

	while (head != NULL) {
#ifdef BCM_SPC_RX
		if (di->spc_rx_enab) {
			data = PKTSPCRXDATA(di->osh, head);
		} else
#endif /* BCM_SPC_RX */
		{
			data = PKTDATA(di->osh, head);
		}

#if (!defined(__mips__) && !(defined(STB) || defined(BCM_ROUTER)))
		len = ltoh16(*(uint16 *)(data));
#else  /* !__mips__ && !(STB || BCM_ROUTER) */

		if (di->rx_waitcomplete_enabled) {
			len = ltoh16(*(uint16 *)(data));
		} else {
			int read_count = 0;
			for (read_count = 200; read_count; read_count--) {
				len = ltoh16(*(uint16 *)(data));
				if (len != 0)
					break;
#if !defined(OSL_CACHE_COHERENT)
				DMA_MAP(di->osh, data, sizeof(uint16), DMA_RX, NULL, NULL);
#endif
				OSL_DELAY(1);
			}

			if (!len) {
				DMA_ERROR(("%s: %s: mlo_unit %d frame length (%d)."
					"Packet is not ready, Big Hammer\n",
					di->name, __FUNCTION__, mlo_unit, len));
				free_p = head;

#ifdef BCM_SPC_RX
				if (di->spc_rx_enab) {
					if (prev != NULL) {
						head = PKTSPCRXLINK(free_p);
						PKTSPCRXSETLINK(prev, head);
					} else {
						head0 = head = PKTSPCRXLINK(free_p);
					}
					PKTSPCRXSETLINK(free_p, NULL);
				} else
#endif /* BCM_SPC_RX */
				{
					if (prev != NULL) {
						head = PKTLINK(free_p);
						PKTSETLINK(prev, head);
					} else {
						head0 = head = PKTLINK(free_p);
					}
					PKTSETLINK(free_p, NULL);

				}
				PKTFREE(di->osh, free_p, FALSE);
				di->hnddma.rxgiants++;
				continue;
			}
		}
#endif /* !__mips__ && !(STB || BCM_ROUTER) */
		DMA_TRACE(("%s: %s: mlo_unit %d len %d\n", di->name, __FUNCTION__, mlo_unit, len));

		/* set actual length */
		pkt_len = MIN((rxoffset + len), di->rxbufsize);

#if defined(D11_SPLIT_RX_FD)
		if (di->sep_rxhdr) {
			/* If separate desc feature is enabled, set length for lcl & host pkt */
			tcm_maxsize = PKTLEN(di->osh, head);
			/* Dont support multi buffer pkt for now */
			if (pkt_len <= tcm_maxsize) {
				/* Full pkt sitting in TCM */
				PKTSETLEN(di->osh, head, pkt_len);	/* LCL pkt length */
				PKTSETFRAGUSEDLEN(di->osh, head, 0);	/* Host segment length */
			} else {
				/* Pkt got split between TCM and host */
				PKTSETLEN(di->osh, head, tcm_maxsize);	/* LCL pkt length */
				/* use PKTFRAGUSEDLEN to indicate actual length dma ed  by d11 */
				/* Cant use PKTFRAGLEN since if need to reclaim this */
				/* we need fraglen intact */
				PKTSETFRAGUSEDLEN(di->osh, head,
					(pkt_len - tcm_maxsize)); /* Host segment length */
			}
		} else
#endif /* D11_SPLIT_RX_FD */
		{
#ifdef BCM_SPC_RX
			if (di->spc_rx_enab) {
				PKTSPCRXSETLEN(di->osh, head, pkt_len);
			} else
#endif /* BCM_SPC_RX */
			{
				PKTSETLEN(di->osh, head, pkt_len);
			}
		}

		resid = len - (di->rxbufsize - rxoffset);

		if (resid <= 0) {
			/* Single frame, all good */
		} else if (di->hnddma.dmactrlflags & DMA_CTRL_RXSINGLE) {
			DMA_ERROR(("%s: %s: mlo_unit %d corrupted length (%d). "
				"Frame received in multiple buffers\n",
				di->name, __FUNCTION__, mlo_unit, len));
			di->hnddma.rxgiants++;

#if defined(STS_XFER_PHYRXS_SEQ)
			if (di->sts_xfer_phyrxs) {
				/* Drop corrupted Rx frames after processing corresponding
				 * PhyRx Status.
				 */
				ASSERT(!PKTISRXCORRUPTED(di->osh, head));
				PKTSETRXCORRUPTED(di->osh, head);
				goto nextpkt_proc;
			} else
#endif /* STS_XFER_PHYRXS_SEQ */
			{
				free_p = head;
#ifdef BCM_SPC_RX
				if (di->spc_rx_enab) {
					if (prev != NULL) {
						head = PKTSPCRXLINK(free_p);
						PKTSPCRXSETLINK(prev, head);
					} else {
						head0 = head = PKTSPCRXLINK(free_p);
					}

					PKTSPCRXSETLINK(free_p, NULL);
				} else
#endif /* BCM_SPC_RX */
				{
					if (prev != NULL) {
						head = PKTLINK(free_p);
						PKTSETLINK(prev, head);
					} else {
						head0 = head = PKTLINK(free_p);
					}

					PKTSETLINK(free_p, NULL);
				}

				PKTFREE(di->osh, free_p, FALSE);
				continue;
			}
		} else {
			/* multi-buffer rx */

#ifdef BCM_SPC_RX
			if (di->spc_rx_enab) {
				dma_rx_spc_multi_buf_prepare_rxlist(di, &rx_list->rxfifocnt,
					head, &resid, tcm_maxsize, is_aux, mlo_unit);
			} else
#endif /* BCM_SPC_RX */
			{
				dma_rx_multi_buf_prepare_rxlist(di, &rx_list->rxfifocnt,
					head, &resid, tcm_maxsize, is_aux, mlo_unit);
			}

			if ((di->hnddma.dmactrlflags & DMA_CTRL_RXMULTI) == 0) {
#ifdef WL_EAP_REKEY_WAR
				if (!di->dis_msgs)
#endif
				{
					DMA_ERROR(("%s: %s: mlo_unit %d bad frame length (%d), "
						"Frame received in "
						"multiple buffers\n", di->name, __FUNCTION__,
						mlo_unit, len));
				}
				di->hnddma.rxgiants++;
#if defined(STS_XFER_PHYRXS_SEQ)
				if (di->sts_xfer_phyrxs) {
					/* Drop corrupted Rx frames after processing corresponding
					 * PhyRx Status.
					 */
					ASSERT(!PKTISRXCORRUPTED(di->osh, head));
					PKTSETRXCORRUPTED(di->osh, head);
					goto nextpkt_proc;
				} else
#endif /* STS_XFER_PHYRXS_SEQ */
				{
#ifdef BCM_SPC_RX
					if (di->spc_rx_enab) {
						dma_rx_spc_multi_buf_free_rxlist(di, &head,
							&head0, prev);
					//	ASSERT(!di->spc_rx_enab);
					} else
#endif /* BCM_SPC_RX */
					{
						dma_rx_multi_buf_free_rxlist(di, &head,
							&head0, prev);
					}
					continue;
				}
			}

#ifdef BCM_SPC_RX
			/* Assert until we figure out which frame is trggering
			 * multi buffer rx
			 */
			ASSERT(!di->spc_rx_enab);
#endif /* BCM_SPC_RX */

		}

#if defined(STS_XFER_PHYRXS_SEQ)
nextpkt_proc:
#endif /* STS_XFER_PHYRXS_SEQ */

		prev = head;

#ifdef BCM_SPC_RX
		if (di->spc_rx_enab) {
			head = PKTSPCRXLINK(head);
		} else
#endif /* BCM_SPC_RX */
		{
			head = PKTLINK(head);
		}
	} /* End While() */

	rx_list->rx_head = head0;
	rx_list->rx_tail = prev;
} /* _dma_rx_prepare_rxlist */

#else /* BULKRX_PKTLIST */

/**
 * !! rx entry routine
 * returns a pointer to the next frame received, or NULL if there are no more
 *   if DMA_CTRL_RXMULTI is defined, DMA scattering(multiple buffers) is supported
 *	  with pkts chain
 *   otherwise, it's treated as giant pkt and will be tossed.
 *   The DMA scattering starts with normal DMA header, followed by first buffer data.
 *   After it reaches the max size of buffer, the data continues in next DMA descriptor
 *   buffer WITHOUT DMA header
 *
 * !BULKRX_PKTLIST specific function.
 */
void * BCMFASTPATH
dma_rx(hnddma_t *dmah)
{
	dma_info_t *di = DI_INFO(dmah);

	void *p, *head, *tail;
	uint len = 0;
	uint pkt_len = 0;
	int resid = 0;
#if defined(D11_SPLIT_RX_FD)
	uint tcm_maxsize = 0;		/* max size of tcm descriptor */
#endif /* D11_SPLIT_RX_FD */
	void *data;

next_frame:
	head = dma_getnextrxp(dmah, FALSE);
	if (head == NULL)
		return (NULL);

#if defined(D11_SPLIT_RX_FD)
	if (di->split_fifo == SPLIT_FIFO_0) {
		/* Fifo-0 handles all host address */
		/* Below PKT ops are not valid for host pkts */
		goto ret;
	}
#endif /* D11_SPLIT_RX_FD */

	data = PKTDATA(di->osh, head);

#if (!defined(__mips__) && !(defined(STB) || defined(BCM_ROUTER)))
	if (di->hnddma.dmactrlflags & DMA_CTRL_SDIO_RXGLOM) {
		/* In case of glommed pkt get length from hwheader */
		len = ltoh16(*((uint16 *)(data) + di->rxoffset/2 + 2)) + 4;

		*(uint16 *)(data) = (uint16)len;
	} else {
		len = ltoh16(*(uint16 *)(data));
	}
#else  /* !__mips__ && !(STB || BCM_ROUTER) */

	if (di->rx_waitcomplete_enabled) {
		len = ltoh16(*(uint16 *)(data));
	} else {

	int read_count = 0;
	for (read_count = 200; read_count; read_count--) {
		len = ltoh16(*(uint16 *)(data));
		if (len != 0)
			break;
#if !defined(OSL_CACHE_COHERENT)
		DMA_MAP(di->osh, data, sizeof(uint16), DMA_RX, NULL, NULL);
#endif
		OSL_DELAY(1);
	}

	if (!len) {
		DMA_ERROR(("%s: dma_rx: frame length (%d) packet is not ready, Big Hammer\n",
			di->name, len));
		PKTFREE(di->osh, head, FALSE);
		goto next_frame;
	}

	}
#endif /* !__mips__ && !(STB || BCM_ROUTER) */
	DMA_TRACE(("%s: dma_rx len %d\n", di->name, len));

	/* set actual length */
	pkt_len = MIN((di->rxoffset + len), di->rxbufsize);

#if defined(D11_SPLIT_RX_FD)
	if (di->sep_rxhdr) {
		/* If separate desc feature is enabled, set length for lcl & host pkt */
		tcm_maxsize = PKTLEN(di->osh, head);
		/* Dont support multi buffer pkt for now */
		if (pkt_len <= tcm_maxsize) {
			/* Full pkt sitting in TCM */
			PKTSETLEN(di->osh, head, pkt_len);	/* LCL pkt length */
			PKTSETFRAGUSEDLEN(di->osh, head, 0);	/* Host segment length */
		} else {
			/* Pkt got split between TCM and host */
			PKTSETLEN(di->osh, head, tcm_maxsize);	/* LCL pkt length */
			/* use PKTFRAGUSEDLEN to indicate actual length dma ed  by d11 */
			/* Cant use PKTFRAGLEN since if need to reclaim this */
			/* we need fraglen intact */
			PKTSETFRAGUSEDLEN(di->osh, head,
				(pkt_len - tcm_maxsize)); /* Host segment length */
		}
	} else
#endif /* D11_SPLIT_RX_FD */
	{
		PKTSETLEN(di->osh, head, pkt_len);
	}

	resid = len - (di->rxbufsize - di->rxoffset);

	if (resid <= 0) {
		/* Single frame, all good */
	} else if (di->hnddma.dmactrlflags & DMA_CTRL_RXSINGLE) {
		DMA_ERROR(("%s: dma_rx: corrupted length (%d). "
			"Frame received in multiple buffers\n", di->name, len));
		PKTFREE(di->osh, head, FALSE);
		di->hnddma.rxgiants++;
		goto next_frame;
	} else {
		/* multi-buffer rx */
#ifdef BCMDBG
		/* get rid of compiler warning */
		p = NULL;
#endif /* BCMDBG */
		tail = head;
		if (di->hnddma.dmactrlflags & DMA_CTRL_SDIO_RXGLOM) {
			while ((resid > 0) && (p = dma_getnextrxp(dmah, FALSE))) {
				PKTSETNEXT(di->osh, tail, p);
				pkt_len = MIN(resid, (int)di->rxbufsize);
#if defined(D11_SPLIT_RX_FD)
				if (di->sep_rxhdr)
					PKTSETLEN(di->osh, p, MIN(pkt_len, tcm_maxsize));
				else
#endif /* D11_SPLIT_RX_FD */
					PKTSETLEN(di->osh, p, pkt_len);

				tail = p;
				resid -= di->rxbufsize;
			}
		} else {
			uint32 rx_frm_sts;
			int num_dd; /* Number of DMA descriptors used for this packet. */

#ifdef BCM_SPC_RX
			ASSERT(!di->spc_rx_enab);
#endif /* BCM_SPC_RX */
			rx_frm_sts = ltoh32(*(uint32 *)(PKTDATA(di->osh, head)));
			num_dd = ((rx_frm_sts & D64_RX_FRM_STS_DSCRCNT) >>
				D64_RX_FRM_STS_DSCRCNT_SHIFT);

			/* Receive Frame Status Header DscrCntr (27:24) Contains one less
			 * than the number of DMA descriptors used to transfer this frame
			 * into memory. So incrementing it by 1 will give the actual number
			 * of DMA descriptors used for this transfer.
			 */
			num_dd++;
			_dma_update_num_dd(di, &num_dd);
			while (num_dd > 0 && (p = dma_getnextrxp(dmah, FALSE))) {
				PKTSETNEXT(di->osh, tail, p);
				pkt_len = MIN(resid, (int)di->rxbufsize);
#if defined(D11_SPLIT_RX_FD)
				if (di->sep_rxhdr)
					PKTSETLEN(di->osh, p, MIN(pkt_len, tcm_maxsize));
				else
#endif /* D11_SPLIT_RX_FD */
					PKTSETLEN(di->osh, p, pkt_len);

				tail = p;
				resid -= di->rxbufsize;
				/* Descriptors will be jumped by 2 in
				 * case of sep_rxhdr and 1 otherwise.
				 */
				_dma_update_num_dd(di, &num_dd);
			}

		}

#ifdef BCMDBG
		if (resid > 0) {
			DMA_ERROR(("%s: dma_rx: rxin %d rxout %d, hw_curr %d\n",
				di->name, di->rxin, di->rxout, DMA64_RX_CD(di)));
			ASSERT(p == NULL);
		}
#endif /* BCMDBG */

		if ((di->hnddma.dmactrlflags & DMA_CTRL_RXMULTI) == 0) {
#ifdef WL_EAP_REKEY_WAR
			if (!di->dis_msgs)
#endif /* WL_EAP_REKEY_WAR */
			{
				DMA_ERROR(("%s: dma_rx: bad frame length (%d), frmsts (0x%08x). "
					"Frame received in multiple buffers\n",
					di->name, len, rx_frm_sts));
			}
			PKTFREE(di->osh, head, FALSE);
			di->hnddma.rxgiants++;
			goto next_frame;
		}
	}

ret:
	return (head);
}
#endif /* BULKRX_PKTLIST */

#ifdef BCM_HWALITE
/* Refill buffers in descriptor rings of MLO AUX links */
static void BCMFASTPATH
_dma_mlo_aux_rxfill(hnddma_t *dmah, uint32 mlo_unit)
{
	dma_info_t	*di = DI_INFO(dmah);
	dma_mlo_aux_info_t *aux_info;
	dma64dd_t	*_rxd64;	/* pointer to dma64 rx descriptor ring */
	void		*p, *tail = NULL;
	dmaaddr_t	pa;
	uint64	fifo_addr_va;
	uint	alignment_req = 8;
	uint	n, i, fifo = 0;
	uint	extra_offset = 0, extra_pad = 0;
	uint32	nrxd, nrxpost, rcvptrbase;
	uint32	flags = 0;
	uint16	rxin, rxout;
	uint16	rxactive;

	/* This flag will only be enabled if MLO is enabled and configured */
	ASSERT(di->hnddma.dmactrlflags & DMA_CTRL_MLO_RXFIFO);
	ASSERT((mlo_role(di->mlc_dev) == MLC_ROLE_MAP));
	ASSERT(di->hnddma.dmactrlflags & DMA_CTRL_DESC_BASE_IDX_UPD);

	/* Aux fifo fill on MAP MLO Fifos */
	/* We already have skipped rxfill for aux. So this
	 * block should be executed only for main with the di
	 * of main. And in the same context we need to post
	 * buffers for all the participating aux
	 */
	ASSERT(!DMA_HWA_RX(di));
	ASSERT(!di->sep_rxhdr);
	ASSERT(di->hnddma.dmactrlflags & DMA_CTRL_RX_ALIGN_8BYTE);
	ASSERT(!di->d11rx_war);
	ASSERT(DMA_BULKRX_PATH(di));
	/* for D11rev >=128, BCM_DMA_RX_ALIGN_8BYTE will be set */
	/* d11rx_war will not be set */

	/*
	 * Determine how many receive buffers we're lacking
	 * from the full complement, allocate, initialize,
	 * and post them, then update the chip rx lastdscr.
	 */

	/* get parames for aux */
	ASSERT(mlo_unit <= DMA_MAX_MLO_AUX_LINKS);
	aux_info = &di->aux_info[mlo_unit - 1];
	rxin  = aux_info->rxin;
	rxout = aux_info->rxout;
	mlc_hwalite_get_aux_rxdma_info(di->mlc_dev,
		mlo_unit, &nrxd, &nrxpost, &fifo_addr_va, &rcvptrbase);
	_rxd64 = (dma64dd_t*)((uintptr)fifo_addr_va);

	rxactive = NRXDACTIVE_MLO(rxin, rxout, nrxd);

	/* if currently posted is more than requested, no need to do anything */
	if (nrxpost <= rxactive) {
		return;
	}

	/* No. of packets to post is : n = MIN(nrxd, nrxpost) - NRXDACTIVE;
	 * But, if (dpp/descriptors per packet > 1), we need this adjustment.
	 */
	n = MIN((nrxd), nrxpost) - rxactive;

	/* Assuming rxbufsize will be same for both the links so we can derive
	 * it from main's di. Same with extra_offset computation
	 */

	DMA_TRACE(("%s: %s: post %d mlo_unit %d\n", di->name, __FUNCTION__, n, mlo_unit));

	if (di->rxbufsize > BCMEXTRAHDROOM)
		extra_offset = di->rxextrahdrroom;

	/* Using Main's pool */
	tail = aux_info->dma_rx_pkt_list.tail_pkt;

	for (i = 0; i < n; i++) {
		/* the di->rxbufsize doesn't include the extra headroom, we need to add it to the
		   size to be allocated
		*/
		/* BCMPKTPOOL Not enabled for NIC */
		uint32 len = di->rxbufsize + extra_offset + alignment_req - 1;

#if defined(NBUFF_PKT_BPM)
		p = PKTPOOLGET(di->osh, len); /* Use BPM packets and data buffers */
#else
		p = PKTGET(di->osh, len, FALSE);
#endif /* NBUFF_PKT_BPM */

		if (p == NULL) {
			DMA_TRACE(("%s: %s: mlo_unit %d out of rxbufs\n",
				di->name, __FUNCTION__, mlo_unit));
			di->hnddma.rxnobuf += (n - i);
			break;
		}

		if (extra_offset)
			PKTPULL(di->osh, p, extra_offset);

		/* Adjust the address of p for address alignment */
		if (alignment_req > 1) {
			extra_pad = (alignment_req - (uintptr)PKTDATA(di->osh, p)) &
				(alignment_req - 1);

			if (extra_pad) {
				PKTPULL(di->osh, p, extra_pad);
			}

			PKTSETLEN(di->osh, p, PKTLEN(di->osh, p) & ~(alignment_req - 1));
		}

		if (!di->rx_waitcomplete_enabled) {
			*(uint32 *)(PKTDATA(di->osh, p)) = 0;
		}

		ASSERT(!di->phyrxs_dma_flags_war);

#if defined(linux) && (defined(BCM47XX_CA9) || defined(__mips__)) && defined(BCM_GMAC3)
#error "Did not expect this block to be enabled"
#else /* !BCM_GMAC3 based FWDER_BUF optimization */

#if !defined(OSL_CACHE_COHERENT)

#if defined(linux) && (defined(BCM47XX_CA9) || defined(__mips__) || \
	defined(BCA_HNDROUTER))
		DMA_MAP(di->osh, PKTDATA(di->osh, p), sizeof(uint32), DMA_TX, NULL, NULL);
#endif
		ASSERT(!DMASGLIST_ENAB);

#if defined(NBUFF_PKT_BPM)
		OSL_VIRT_TO_PHYSADDR(di->osh, PKTDATA(di->osh, p), pa);
#else
		pa = DMA_MAP(di->osh, PKTDATA(di->osh, p),
			di->rxbufsize, DMA_RX, p, NULL);
#endif /* ! NBUFF_PKT_BPM */

#else  /* OSL_CACHE_COHERENT */
		/* Do not cache flush or invalidate on HW cache coherent systems */
		OSL_VIRT_TO_PHYSADDR(di->osh, PKTDATA(di->osh, p), pa);
#endif /* ! OSL_CACHE_COHERENT */

#endif /* !(linux && (BCM47XX_CA9 || __mips__) && BCM_GMAC3) */

		ASSERT(ISALIGNED(PHYSADDRLO(pa), 8));
		ASSERT(DMA_BULKRX_PATH(di));

		PKTSETLINK(p, NULL);
		if (tail) {
			PKTSETLINK(tail, p);
			aux_info->dma_rx_pkt_list.tail_pkt = tail = p;
		} else {
			ASSERT((aux_info->dma_rx_pkt_list.head_pkt == NULL));
			aux_info->dma_rx_pkt_list.head_pkt = tail = p;
			aux_info->dma_rx_pkt_list.tail_pkt = tail;
		}

		/* reset flags for each descriptor */
		flags = D64_CTRL1_SOF;

		if (rxout == (nrxd - 1))
			flags |= D64_CTRL1_EOT;

		dma64_dd_upd(di, _rxd64, pa, rxout, &flags, di->rxbufsize);

		rxout = NEXTRXD_MLO(rxout, nrxd);
	} /* for n */

#if !defined(BULK_DESCR_FLUSH)
	aux_info->rxout = rxout;
#endif
	/* update the chip lastdscr pointer */

#if !defined(OSL_CACHE_COHERENT)
#if defined(BULK_DESCR_FLUSH)
	{
		uint32 flush_cnt = NRXDACTIVE_MLO(aux_info->rxout, rxout, nrxd);
		if (rxout < aux_info->rxout) {
			/* For RxFiFo dd64_size should be same as sizeof(dma64dd_t)
			 * as BCM_DMA_DESC_ONLY_FLAG will not be set
			 */
			ASSERT(di->dd64_size == sizeof(dma64dd_t));
			DMA_MAP(di->osh, (void *)&_rxd64[0], (sizeof(dma64dd_t) * rxout),
				DMA_TX, NULL, NULL);
			flush_cnt -= rxout;
		}
		DMA_MAP(di->osh, (void *)&_rxd64[aux_info->rxout],
			(sizeof(dma64dd_t) * flush_cnt),
			DMA_TX, NULL, NULL);
	}
#endif /* BULK_DESCR_FLUSH */
#endif /* ! OSL_CACHE_COHERENT */

	fifo = MLO_MLOUNIT_TO_RXFIFO(mlo_unit);

	/* MLO LD UPDATE is index based only */
	mlc_hwalite_rxfifo_ld_upd(di->mlc_dev, fifo,
		(uint16)(B2I(rcvptrbase, sizeof(dma64dd_t)) + rxout));
#if defined(BULK_DESCR_FLUSH)
	aux_info->rxout = rxout;
#endif

}

void BCMFASTPATH
dma_mlo_rx(hnddma_t *dmah, rx_list_t *rx_list, uint nbound, uint32 mlo_unit)
{
	dma_info_t *di = DI_INFO(dmah);
	void *head;
	bool is_aux = FALSE;

	if (mlo_unit) {
		is_aux = TRUE;
	}

	rx_list->rxfifocnt = 0;
	head = _dma_getnext_rxlist(di, nbound, &rx_list->rxfifocnt, FALSE, is_aux, mlo_unit);
	_dma_rx_prepare_rxlist(di, head, rx_list, is_aux, mlo_unit);

	return;
} /* dma_mlo_rx */

/* Reclaim Rx buffers posted to MLO AUX link decriptor ring */
static void BCMFASTPATH
_dma_mlo_aux_rxreclaim(hnddma_t *dmah, uint32 mlo_unit)
{
	dma_info_t *di = DI_INFO(dmah);
	void *p, *next = NULL;

	DMA_TRACE(("%s: %s: of AUX link[%d]\n", di->name, __FUNCTION__, mlo_unit));

	ASSERT(!(di->bcmrx_pcn_fifo));

	ASSERT(mlo_unit);

#ifdef PP_XPM
	if (di->pp_xpm_enab) {
		return _dma_pp_xpm_rxreclaim(di, TRUE, mlo_unit);
	}
#endif /* PP_XPM */

	p = _dma_getnext_rxlist(di, 0, NULL, TRUE, TRUE, mlo_unit);

	while (p != NULL) {
		next = PKTLINK(p);
		PKTSETLINK(p, NULL);
		PKTFREE(di->osh, p, FALSE);

		p = next;
	}
} /* _dma_mlo_aux_rxreclaim() */
#endif /* BCM_HWALITE */

/**
 * A 'receive' DMA engine must be fed with buffers to write received data into. This function
 * 'posts' receive buffers. If the 'packet pool' feature is enabled, the buffers are drawn from the
 * packet pool. Otherwise, the buffers are retrieved using the OSL 'PKTGET' macro.
 *
 *  return FALSE is refill failed completely and ring is empty
 *  this will stall the rx dma and user might want to call rxfill again asap
 *  This unlikely happens on memory-rich NIC, but often on memory-constrained dongle
 */
bool BCMFASTPATH
dma_rxfill(hnddma_t *dmah)
{
	dma_info_t *di = DI_INFO(dmah);

	void *p;
	uint16 rxin, rxout;
	uint16 rxactive;
	uint32 flags = 0;
	uint n;
	uint i;
	dmaaddr_t pa;
	uint extra_offset = 0, extra_pad = 0;
	bool ring_empty;
#ifdef BULKRX_PKTLIST
	void *tail = NULL;
#endif
	uint alignment_req = (di->hnddma.dmactrlflags & DMA_CTRL_USB_BOUNDRY4KB_WAR) ?
				16 : 1;	/* MUST BE POWER of 2 */

	/* if sep_rxhdr is enabled, for every pkt, two descriptors are programmed */
	/* NRXDACTIVE(rxin, rxout) would show 2 times no of actual full pkts */
	uint dpp = (di->sep_rxhdr) ? 2 : 1; /* dpp - descriptors per packet */

	if (DMA_HWA_RX(di))
		return FALSE;

	if (di->hnddma.dmactrlflags & DMA_CTRL_RX_ALIGN_8BYTE) {
		alignment_req = 8;
	}

#if defined(D11_SPLIT_RX_FD)
	uint pktlen;
	dma64addr_t pa64 = { .low_addr = 0, .high_addr = 0 };

	if (di->rxfill_suspended)
		return FALSE;

	if (di->split_fifo) {
		/* SPLIFIFO rxfill is handled separately */
		return dma_splitrxfill(di);
	}
#endif /* D11_SPLIT_RX_FD */
#if defined(BCM_PCAP)
	if (DMA_PCAP_EN(di)) {
		return dma_pcap_rxfill(dmah);
	}
#endif /* BCM_PCAP */

	if (di->rxfill_suspended)
		return FALSE;

#ifdef PP_XPM
	if (di->pp_xpm_enab) {
		return dma_pp_xpm_rxfill(dmah);
	}
#endif /* PP_XPM */

	if (di->d11rx_war) {
		int plen = 0;
		dpp = 0; /* Recompute descriptors per pkt, for D11 RX WAR */
#ifdef BCMPKTPOOL
		if (POOL_ENAB(di->pktpool)) {
			plen = pktpool_max_pkt_bytes(di->pktpool);
		}
		else
#endif /* BCMPKTPOOL */
		{
			plen = di->rxbufsize;
		}

		if (di->sep_rxhdr) {
			dpp = 1; /* TCM + (HOST breakup) */
			plen = MAXRXBUFSZ - plen;
		}

		dpp += CEIL(plen, D11RX_WAR_MAX_BUF_SIZE);
	}

	ring_empty = FALSE;

	/*
	 * Determine how many receive buffers we're lacking
	 * from the full complement, allocate, initialize,
	 * and post them, then update the chip rx lastdscr.
	 */

	rxin = di->rxin;
	rxout = di->rxout;
	rxactive = NRXDACTIVE(rxin, rxout);

	/* if currently posted is more than requested, no need to do anything */
	if (MIN((di->nrxd/dpp), di->nrxpost) <= CEIL(rxactive, dpp)) {
		goto done;
	}

	/* No. of packets to post is : n = MIN(nrxd, nrxpost) - NRXDACTIVE;
	 * But, if (dpp/descriptors per packet > 1), we need this adjustment.
	 */
	n = MIN((di->nrxd/dpp), di->nrxpost) - CEIL(rxactive, dpp);

	if (di->rxbufsize > BCMEXTRAHDROOM)
		extra_offset = di->rxextrahdrroom;

	DMA_TRACE(("%s: dma_rxfill: post %d\n", di->name, n));

#ifdef BCM_PCAP
	if (di->rs0cd_reset) {
		/* After RX DMA reset, RS0CD is stuck until next rxd is
		 * consumed. Ensure that rxout +n != the RS0CD at reset,
		 * to avoid ambiguity and correctly detect when RS0CD
		 * has truly advanced.
		 */
		if (n > 0 && RXD(rxout + n) == di->rs0cd_last) {
			n--;
		}
	}
#endif /* BCM_PCAP */
#ifdef BULKRX_PKTLIST
	if (DMA_BULKRX_PATH(di))
		tail = di->dma_rx_pkt_list.tail_pkt;
#endif
	for (i = 0; i < n; i++) {
		int dlen = 0;
		int ds_len = 0;
		/* the di->rxbufsize doesn't include the extra headroom, we need to add it to the
		   size to be allocated
		*/
#ifdef BCMPKTPOOL
		if (POOL_ENAB(di->pktpool)) {
			ASSERT(di->pktpool);
			p = pktpool_get(di->pktpool);
#ifdef BCMDBG_POOL
			if (p)
				PKTPOOLSETSTATE(p, POOL_RXFILL);
#endif /* BCMDBG_POOL */
		} else
#endif /* BCMPKTPOOL */
		{
			uint32 len = di->rxbufsize + extra_offset + alignment_req - 1;
#if defined(BCMRX_PCN)
			if (di->bcmrx_pcn_fifo) {
#if defined(BCMRX_PCN_PKTPOOL)
				p = pktpool_get((pktpool_t *)OSH_GET_PCNPKTPOOL(di->osh));
#else
				p = PKTGET(di->osh, len, FALSE);
#endif /* BCMRX_PCN_PKTPOOL */
			}
			else
#endif /* BCMRX_PCN */
			{
#if defined(NBUFF_PKT_BPM)
				p = PKTPOOLGET(di->osh, len); /* Use BPM packets and data buffers */
#else
				p = PKTGET(di->osh, len, FALSE);
#endif /* NBUFF_PKT_BPM */
			}
		}

		if (p == NULL) {
			DMA_TRACE(("%s: dma_rxfill: out of rxbufs\n", di->name));
			if (i == 0) {
				if (dma_rxidle(dmah)) {
					DMA_TRACE(("%s: rxfill64: ring is empty !\n",
						di->name));
					ring_empty = TRUE;
				}
			}
			di->hnddma.rxnobuf += (n - i);
#ifdef BPM_BULK_FREE
			linux_commit_skb_freelist(di->osh);
#endif /* BPM_BULK_FREE */
			break;
		}

		if (extra_offset)
			PKTPULL(di->osh, p, extra_offset);

		/* Adjust the address of p for address alignment */
		if (alignment_req > 1) {
			extra_pad = (alignment_req - (uintptr)PKTDATA(di->osh, p)) &
				(alignment_req - 1);

		if (extra_pad) {
				PKTPULL(di->osh, p, extra_pad);
			}

			PKTSETLEN(di->osh, p, PKTLEN(di->osh, p) & ~(alignment_req - 1));
		}

		if (!di->rx_waitcomplete_enabled) {
			*(uint32 *)(PKTDATA(di->osh, p)) = 0;
		}

#if defined(STS_XFER_PHYRXS_PKT)
		if (di->phyrxs_dma_flags_war) {
			/* Reset PhyRx Sts length. 6726/6765 SW WAR to fix d11rxhdr_t::dma_flags */
			*(uint32 *)(PKTDATA(di->osh, p) + D11_RXHDR_PHYRXPKT_WAR_PHYRXS_OFFSET) = 0;
		}
#endif /* STS_XFER_PHYRXS_PKT */

#if !defined(OSL_CACHE_COHERENT)

#if defined(linux) && (defined(__mips__) || defined(BCM_ROUTER))
		DMA_MAP(di->osh, PKTDATA(di->osh, p), sizeof(uint32), DMA_TX, NULL, NULL);
#endif

#if defined(SGLIST_RX_SUPPORT)
		if (DMASGLIST_ENAB)
			bzero(&di->rxp_dmah[rxout], sizeof(hnddma_seg_map_t));

		pa = DMA_MAP(di->osh, PKTDATA(di->osh, p), di->rxbufsize, DMA_RX, p,
			&di->rxp_dmah[rxout]);

#else  /* ! SGLIST_RX_SUPPORT */

#if defined(BCMRX_PCN)
		if (di->bcmrx_pcn_fifo) {
			pa = DMA_MAP(di->osh, PKTDATA(di->osh, p),
				di->rxbufsize, DMA_RX, p, NULL);
		}
		else
#endif /* BCMRX_PCN */
		{
#if defined(NBUFF_PKT_BPM)
			OSL_VIRT_TO_PHYSADDR(di->osh, PKTDATA(di->osh, p), pa);
#else
			pa = DMA_MAP(di->osh, PKTDATA(di->osh, p),
				di->rxbufsize, DMA_RX, p, NULL);
#endif /* ! NBUFF_PKT_BPM */
		}

#endif /* ! SGLIST_RX_SUPPORT */

#else  /* OSL_CACHE_COHERENT */
		/* Do not cache flush or invalidate on HW cache coherent systems */
		OSL_VIRT_TO_PHYSADDR(di->osh, PKTDATA(di->osh, p), pa);
#endif /* ! OSL_CACHE_COHERENT */

		if (di->hnddma.dmactrlflags & DMA_CTRL_RX_ALIGN_8BYTE) {
			ASSERT(ISALIGNED(PHYSADDRLO(pa), 8));
		}
		else {
			ASSERT(ISALIGNED(PHYSADDRLO(pa), 4));
		}
#ifdef BULKRX_PKTLIST
		if (DMA_BULKRX_PATH(di)) {
			PKTSETLINK(p, NULL);
			if (tail) {
				PKTSETLINK(tail, p);
				di->dma_rx_pkt_list.tail_pkt = tail = p;
			} else {
				ASSERT((di->dma_rx_pkt_list.head_pkt == NULL));
				di->dma_rx_pkt_list.head_pkt = tail = p;
				di->dma_rx_pkt_list.tail_pkt = tail;
			}
		} else
#endif
		{
			/* save the free packet pointer */
			ASSERT(di->rxp[rxout] == NULL);
			di->rxp[rxout] = p;
		}

#if defined(D11_SPLIT_RX_FD)
		if (!di->sep_rxhdr)
#endif /* D11_SPLIT_RX_FD */
		{
			/* reset flags for each descriptor */
			flags = D64_CTRL1_SOF;
			dlen = di->rxbufsize;
			ds_len = (di->d11rx_war) ? D11RX_WAR_MAX_BUF_SIZE : dlen;
			do {
				uint64 addr;

				if (di->hnddma.dmactrlflags & DMA_CTRL_RX_ALIGN_8BYTE) {
					ASSERT(ISALIGNED(PHYSADDRLO(pa), 8));
				}

				if (rxout == (di->nrxd - 1))
					flags |= D64_CTRL1_EOT;
				dma64_dd_upd(di, di->rxd64, pa, rxout, &flags,
					MIN(dlen, ds_len));

				rxout = NEXTRXD(rxout);
				dlen -= ds_len;
				if (dlen <= 0) {
					/* bail out if we are done */
					break;
				}
				di->rxp[rxout] = (void*)PCI64ADDR_HIGH;

				/* prep for next descriptor */
				addr = (((uint64)PHYSADDRHI(pa) << 32) | PHYSADDRLO(pa)) + ds_len;
				PHYSADDRHISET(pa, (uint32)(addr >> 32));
				PHYSADDRLOSET(pa, (uint32)(addr & 0xFFFFFFFF));
				flags = 0;
			} while (dlen > 0);
		}
#if defined(D11_SPLIT_RX_FD)
		else {
			/* TCM Descriptor */
			flags = 0;
			pktlen = PKTLEN(di->osh, p);

			if (rxout == (di->nrxd - 1))
				flags = D64_CTRL1_EOT;

			/* MAR SOF, so start frame will go to this descriptor */
			flags |= D64_CTRL1_SOF;

			if (di->hnddma.dmactrlflags & DMA_CTRL_RX_ALIGN_8BYTE) {
				ASSERT(ISALIGNED(PHYSADDRLO(pa), 8));
			}

			dma64_dd_upd(di, di->rxd64, pa, rxout, &flags, pktlen);
			rxout = NEXTRXD(rxout);		/* update rxout */

			/* Now program host descriptor(s) */

			/* Extract out host addresses */
			pa64.loaddr = PKTFRAGDATA_LO(di->osh, p, LB_FRAG1);
			pa64.hiaddr = PKTFRAGDATA_HI(di->osh, p, LB_FRAG1);

			/* Extract out host length */
			dlen = pktlen = PKTFRAGLEN(di->osh, p, LB_FRAG1);
			ds_len = (di->d11rx_war) ? D11RX_WAR_MAX_BUF_SIZE : pktlen;

			do {
				uint64 addr = 0;
				/* Mark up this descriptor that its a host descriptor
				* store 0x80000000 so that dma_rx need'nt
				* process this descriptor
				*/
#ifndef BULKRX_PKTLIST
				di->rxp[rxout] = (void*)PCI64ADDR_HIGH;
#endif
				flags = 0;	/* Reset the flags */

				if (rxout == (di->nrxd - 1))
					flags = D64_CTRL1_EOT;

				if (di->hnddma.dmactrlflags & DMA_CTRL_RX_ALIGN_8BYTE) {
					ASSERT(ISALIGNED(PHYSADDRLO(pa64.loaddr), 8));
				}

				/* load the descriptor */
				dma64_dd_upd_64_from_params(di, di->rxd64, pa64, rxout,
					&flags,	MIN(ds_len, dlen));
				rxout = NEXTRXD(rxout);	/* update rxout */

				/* prep for next descriptor */
				dlen -= ds_len;
				if (dlen <= 0) break; /* bail out if we are done */
				addr = (((uint64)pa64.hiaddr << 32) | pa64.loaddr) + ds_len;
				pa64.hiaddr = (uint32)(addr >> 32);
				pa64.loaddr = (uint32)(addr & 0xFFFFFFFF);
			} while (dlen > 0);
		}
#endif /* D11_SPLIT_RX_FD */
	} /* for n */

#if !defined(BULK_DESCR_FLUSH)
	di->rxout = rxout;
#endif

	/* update the chip lastdscr pointer */

#if !defined(OSL_CACHE_COHERENT)
#if defined(BULK_DESCR_FLUSH)
	{
		uint32 flush_cnt = NRXDACTIVE(di->rxout, rxout);
		if (rxout < di->rxout) {
			DMA_MAP(di->osh, dma64_rxd64(di, 0), DMA64_FLUSH_LEN(di, rxout),
					DMA_TX, NULL, NULL);
			flush_cnt -= rxout;
		}
		DMA_MAP(di->osh, dma64_rxd64(di, di->rxout), DMA64_FLUSH_LEN(di, flush_cnt),
				DMA_TX, NULL, NULL);
	}
#endif /* BULK_DESCR_FLUSH */
#endif /* ! OSL_CACHE_COHERENT */

#ifdef BCM_HWALITE
	if (mlc_hwalite_is_enable(di->mlc_dev)) {
		/* MLO LD UPDATE is index based only */
		mlc_hwalite_rxfifo_ld_upd(di->mlc_dev, di->fifo,
			(uint16)(B2I(di->rcvptrbase, sizeof(dma64dd_t)) + rxout));
	} else
#endif
	{
		DMA64_RX_LD_UPD(di, rxout);
	}

#if defined(BULK_DESCR_FLUSH)
	di->rxout = rxout;
#endif

done:
#ifdef BCM_HWALITE
	if ((mlo_role(di->mlc_dev) == MLC_ROLE_MAP) && (di->fifo == RX_MLO_FIFO3)) {
		uint32 mlo_unit;

		ASSERT(mlc_hwalite_is_enable(di->mlc_dev));
		ASSERT(di->hnddma.dmactrlflags & DMA_CTRL_MLO_RXFIFO);

		/* Post buffers to Aux Fifo */
		FOREACH_MLO_AAP(di->mlc_dev, mlo_unit) {
			_dma_mlo_aux_rxfill(dmah, mlo_unit);
		}
	}
#endif /* BCM_HWALITE */

	return !ring_empty;
} /* _dma_rxfill */

/** Called when e.g. flushing d11 packets */
void BCMFASTPATH
dma_rxreclaim(hnddma_t *dmah)
{
	dma_info_t *di = DI_INFO(dmah);

	void *p;
#ifdef BCMPKTPOOL
	bool origcb = TRUE;
#endif /* BCMPKTPOOL */
#ifdef BULKRX_PKTLIST
	void *next = NULL;
#endif
	DMA_TRACE(("%s: dma_rxreclaim\n", di->name));

#ifdef BCM_PCAP
	if (DMA_PCAP_EN(di)) {
		return dma_pcap_rxreclaim(dmah);
	}
#endif /* BCM_PCAP */
#ifdef BCMPKTPOOL
	if (POOL_ENAB(di->pktpool) &&
		((origcb = pktpool_emptycb_disabled(di->pktpool)) == FALSE))
		pktpool_emptycb_disable(di->pktpool, TRUE);
#endif /* BCMPKTPOOL */

#if defined(D11_SPLIT_RX_FD)
	/* For split fifo, this function expects fifo-1 di to proceed further. */
	/* fifo-0 requests can be discarded since fifo-1 will reclaim both fifos */
	if (di->split_fifo == SPLIT_FIFO_0) {
		return;
	}
#endif /* D11_SPLIT_RX_FD */

#ifdef PP_XPM
	if (di->pp_xpm_enab) {
		_dma_pp_xpm_rxreclaim(di, FALSE, DMA_MLO_UNIT_INVALID);
		goto reclaim_aux;
	}
#endif /* PP_XPM */

#ifdef BULKRX_PKTLIST
	if (DMA_BULKRX_PATH(di)) {
		p = _dma_getnext_rxlist(di, 0, NULL, TRUE, FALSE, DMA_MLO_UNIT_INVALID);
	} else
#endif /* BULKRX_PKTLIST */
	{
		p = dma_getnextrxp(dmah, TRUE);
	}

	while (p != NULL) {
#if defined(BCMSPLITRX) && defined(D11_SPLIT_RX_FD)
		if (PKTISRXFRAG(di->osh, p)) {
			PKTRESETFIFO0INT(di->osh, p);
			PKTRESETFIFO1INT(di->osh, p);
		}
#endif /* BCMSPLITRX && D11_SPLIT_RX_FD */
#ifdef BULKRX_PKTLIST
		if (DMA_BULKRX_PATH(di)) {
			next = PKTLINK(p);
			PKTSETLINK(p, NULL);
		}
#endif

#if defined(BCMRX_PCN) && defined(BCMRX_PCN_PKTPOOL)
		if (di->bcmrx_pcn_fifo) {
			pktpool_free(di->osh, OSH_GET_PCNPKTPOOL(di->osh), p);
		}
		else
#endif /* BCMRX_PCN && BCMRX_PCN_PKTPOOL */
		{
			PKTFREE(di->osh, p, FALSE);
		}

#ifdef BULKRX_PKTLIST
		if (DMA_BULKRX_PATH(di)) {
			p = next;
		} else
#endif
		{
			p = dma_getnextrxp(dmah, TRUE);
		}
	}

#ifdef PP_XPM
reclaim_aux:
#endif /* PP_XPM */

#ifdef BCM_HWALITE
	if ((mlo_role(di->mlc_dev) == MLC_ROLE_MAP) && (di->fifo == RX_MLO_FIFO3)) {
		uint32 mlo_unit;

		ASSERT(mlc_hwalite_is_enable(di->mlc_dev));
		ASSERT(di->hnddma.dmactrlflags & DMA_CTRL_MLO_RXFIFO);

		/* Post buffers to Aux Fifo */
		FOREACH_MLO_AAP(di->mlc_dev, mlo_unit) {
			_dma_mlo_aux_rxreclaim(dmah, mlo_unit);
		}
	}
#endif /* BCM_HWALITE */

#ifdef BCMPKTPOOL
	if (POOL_ENAB(di->pktpool) && origcb == FALSE)
		pktpool_emptycb_disable(di->pktpool, FALSE);
#endif /* BCMPKTPOOL */

	return;
} /* dma_rxreclaim */

/**
 * Initializes one tx or rx descriptor with the caller provided arguments, notably a buffer.
 * Knows how to handle native 64-bit addressing AND bit64 extension.
 *	@param[in] di	    Handle
 *	@param[in] ddring   Pointer to properties of the descriptor ring
 *	@param[in] pa	    Physical address of rx or tx buffer that the descriptor should point at
 *	@param[in] outidx   Index of the targetted DMA descriptor within the descriptor ring
 *	@param[in] ctrl1    Value that is written into the descriptors 'ctrl1' field
 *	@param[in] bufcount Buffer count value, written into the descriptors 'ctrl2' field
 */
void BCMFASTPATH
dma64_dd_upd(dma_info_t *di, dma64dd_t *ddring, dmaaddr_t pa, uint outidx, uint32 *ctrl1,
	uint32 bufcount)
{
	uint32 ctrl2 = bufcount & D64_CTRL2_BC_MASK;

	/* PCI bus with big(>1G) physical address, use address extension */
#if defined(__mips__) && defined(IL_BIGENDIAN)
	if ((di->dataoffsetlow == SI_SDRAM_SWAPPED) || !(PHYSADDRLO(pa) & PCI32ADDR_HIGH)) {
#else
	if ((di->dataoffsetlow == 0) || !(PHYSADDRLO(pa) & PCI32ADDR_HIGH)) {
#endif /* defined(__mips__) && defined(IL_BIGENDIAN) */
		/* This is where 64-bit addr ext will come into picture but most likely
		 * nobody will be around by the time we have full 64-bit memory addressing
		 * requirement
		 */
		ASSERT((PHYSADDRHI(pa) & PCI64ADDR_HIGH) == 0);

		W_SM(&ddring[outidx].addrlow, BUS_SWAP32(PHYSADDRLO(pa) + di->dataoffsetlow));
		W_SM(&ddring[outidx].addrhigh, BUS_SWAP32(PHYSADDRHI(pa) + di->dataoffsethigh));
		W_SM(&ddring[outidx].ctrl1, BUS_SWAP32(*ctrl1));
		W_SM(&ddring[outidx].ctrl2, BUS_SWAP32(ctrl2));
	} else {
		/* address extension for 32-bit PCI */
		uint32 ae;
		ASSERT(di->addrext);

		ae = (PHYSADDRLO(pa) & PCI32ADDR_HIGH) >> PCI32ADDR_HIGH_SHIFT;
		PHYSADDRLO(pa) &= ~PCI32ADDR_HIGH;
		ASSERT(PHYSADDRHI(pa) == 0);

		ctrl2 |= (ae << D64_CTRL2_AE_SHIFT) & D64_CTRL2_AE;
		W_SM(&ddring[outidx].addrlow, BUS_SWAP32(PHYSADDRLO(pa) + di->dataoffsetlow));
		W_SM(&ddring[outidx].addrhigh, BUS_SWAP32(0 + di->dataoffsethigh));
		W_SM(&ddring[outidx].ctrl1, BUS_SWAP32(*ctrl1));
		W_SM(&ddring[outidx].ctrl2, BUS_SWAP32(ctrl2));
	}

	if (di->hnddma.dmactrlflags & DMA_CTRL_PEN) {
		if (DMA64_DD_PARITY(&ddring[outidx])) {
			W_SM(&ddring[outidx].ctrl2, BUS_SWAP32(ctrl2 | D64_CTRL2_PARITY));
		}
	}

#if !defined(OSL_CACHE_COHERENT)
#if (((defined(STB) || defined(STBAP)) && !defined(DHD_USE_COHERENT_MEM_FOR_RING)) || \
	defined(BCM_ROUTER)) && !defined(BULK_DESCR_FLUSH)
	DMA_MAP(di->osh, (void *)(((unsigned long long)(&ddring[outidx])) & ~0x1f), 32,
		DMA_TX, NULL, NULL);
#endif
#endif /* ! OSL_CACHE_COHERENT */

	/* memory barrier before posting the descriptor */
	DMB();
} /* dma64_dd_upd */

bool
dma_rxidle(hnddma_t *dmah)
{
	dma_info_t *di = DI_INFO(dmah);

	DMA_TRACE(("%s: dma_rxidle\n", di->name));

	if (di->nrxd == 0)
		return TRUE;

	return ((R_REG(di->osh, &di->d64rxregs->status0) & D64_RS0_CD_MASK) ==
		(R_REG(di->osh, &di->d64rxregs->ptr) & D64_RS0_CD_MASK));
}

bool
dma_rxenabled(hnddma_t *dmah)
{
	dma_info_t *di = DI_INFO(dmah);

	uint32 rc;

	rc = R_REG(di->osh, &di->d64rxregs->control);
	return ((rc != 0xffffffff) && (rc & D64_RC_RE));
}

/** returns entries on the ring, in the order in which they were placed on the ring */
void * BCMFASTPATH
dma_getnextrxp(hnddma_t *dmah, bool forceall)
{
	dma_info_t *di = DI_INFO(dmah);

	uint16 i, curr;
	void *rxp;
#if (!defined(__mips__) && !(defined(STB) || defined(BCM_ROUTER)))
	dmaaddr_t pa;
#endif
	if ((di->nrxd == 0) || DMA_HWA_RX(di))
		return (NULL);

#ifdef BCM_BACKPLANE_TIMEOUT
	if (!si_deviceremoved(di->sih))
	{
		/* if forcing, dma engine must be disabled */
		ASSERT(!forceall || !dma_rxenabled(dmah));
	}
#endif /* BCM_BACKPLANE_TIMEOUT */

nextframe:

	i = di->rxin;

	/* return if no packets posted */
	if (i == di->rxout)
		return (NULL);

	if (di->rxin == di->rs0cd) {
		curr = DMA64_RX_CD(di);
#ifdef BCM_PCAP
		if (di->rs0cd_reset) {
			curr = dma_pcap_rx_rs0cd_fixup(di, curr);
		}
#endif /* BCM_PCAP */
		di->rs0cd = curr;
	} else
		curr = di->rs0cd;

	/* ignore curr if forceall */
	if (!forceall && (i == curr))
		return (NULL);

	/* get the packet pointer that corresponds to the rx descriptor */
	rxp = di->rxp[i];
	ASSERT(rxp);

#if (defined(__mips__) || defined(STB)) && !defined(_CFE_)
	if (!DMA_PCAP_EN(di)) {
		/* Processor prefetch of 1 x 32B cacheline carrying HWRXOFF */
		uint8 * addr = PKTDATA(di->osh, rxp);
		bcm_prefetch_32B(addr, 1);
	}
#endif /* (__mips__ || STB) && linux */

	di->rxp[i] = NULL;

	/* We had marked up host/split descriptors as 0x80000000 */
	/* if we find that address, just skip that and go to next frame */
	if (di->sep_rxhdr || di->d11rx_war) {
		if ((uint32)(uintptr)rxp == PCI64ADDR_HIGH) {
			di->rxin = NEXTRXD(i);
			di->rxavail = di->nrxd - NRXDACTIVE(di->rxin, di->rxout) - 1;
			goto nextframe;
		}
	}

	/* PCAP DMA channel configuration uses host DMA buffers */
	if (! DMA_PCAP_EN(di)) {
#if defined(SGLIST_RX_SUPPORT)
		PHYSADDRLOSET(pa, (BUS_SWAP32(R_SM(&di->rxd64[i].addrlow)) - di->dataoffsetlow));
		PHYSADDRHISET(pa, (BUS_SWAP32(R_SM(&di->rxd64[i].addrhigh)) - di->dataoffsethigh));

		/* clear this packet from the descriptor ring */
#if !defined(OSL_CACHE_COHERENT)
		DMA_UNMAP(di->osh, pa, di->rxbufsize, DMA_RX, rxp, &di->rxp_dmah[i]);
#endif /* OSL_CACHE_COHERENT */
#endif /* SGLIST_RX_SUPPORT */
	}

#if (defined(DESCR_DEADBEEF) || defined(STB))
	W_SM(&di->rxd64[i].addrlow, 0xdeadbeef);
	W_SM(&di->rxd64[i].addrhigh, 0xdeadbeef);
#endif /* DESCR_DEADBEEF */

	di->rxin = NEXTRXD(i);

	di->rxavail = di->nrxd - NRXDACTIVE(di->rxin, di->rxout) - 1;

	return (rxp);
}

#ifdef BULKRX_PKTLIST

static void
_dma_get_rx_proc_params(dma_info_t *di, uint nbound, bool forceall,
	bool is_aux, uint32 mlo_unit, uint32 *nrxd_p, uint16 *rxin_p, uint16 *rxout_p,
	uint16 *curr_p, uint16 *nproc_p, dma64dd_t **_rxd64_p, void ***rxpdata_p)
{

	dma64dd_t *_rxd64;	/* pointer to dma64 rx descriptor ring */
	void	**rxpdata = NULL;
#ifdef BCM_HWALITE
	dma_mlo_aux_info_t *aux_info = NULL;
	uint32	rcvptrbase;
	uint	fifo;
#endif /* BCM_HWALITE */
	uint32	nrxd;
	uint16	rxin, rxout;
	uint16	curr = 0, nproc = 0;
#ifdef WLSPP_NIC
	uint16	* cd_peek_p;
#endif /* WLSPP_NIC */

#ifdef BCM_HWALITE
	rcvptrbase = di->rcvptrbase;
	fifo = di->fifo;

	if (is_aux) {
		uint64	fifo_addr_va;

		ASSERT(mlo_unit <= DMA_MAX_MLO_AUX_LINKS);
		aux_info = &di->aux_info[mlo_unit - 1];
		mlc_hwalite_get_aux_rxdma_info(di->mlc_dev, mlo_unit, &nrxd,
			NULL, &fifo_addr_va, &rcvptrbase);

		fifo	= MLO_MLOUNIT_TO_RXFIFO(mlo_unit);
		rxin	= aux_info->rxin;
		rxout	= aux_info->rxout;
		_rxd64	= (dma64dd_t*)((uintptr)fifo_addr_va);
#ifdef PP_XPM
		rxpdata = aux_info->rxpdata;
#endif /* PP_XPM */
#ifdef WLSPP_NIC
		cd_peek_p = &aux_info->cd_peek;
#endif /* WLSPP_NIC */
	} else
#endif /* BCM_HWALITE */
	{
		nrxd	= di->nrxd;
		rxin	= di->rxin;
		rxout	= di->rxout;
		_rxd64	= di->rxd64;
#ifdef PP_XPM
		rxpdata = di->rxpdata;
#endif /* PP_XPM */
#ifdef WLSPP_NIC
		cd_peek_p = &di->cd_peek;
#endif /* WLSPP_NIC */
	}

	if (forceall) {
		curr	= rxout;
		nproc	= NRXDACTIVE_MLO(rxin, curr, nrxd);
#ifdef WLSPP_NIC
		/* Reset SPP cd_peek during reclaim */
		*cd_peek_p = curr;
#endif /* WLSPP_NIC */
	} else {
		/* Number of DMA descr to process */

#ifdef WLSPP_NIC
		/* If SPP-NIC is enabled, use the CD from peeked value.
		 * Actual CD will be read by SPP scheduler
		 */
		if (di->spp_nic_enab) {
			curr = *cd_peek_p;
		} else
#endif /* WLSPP_NIC */
#ifdef BCM_HWALITE
		if (mlc_hwalite_is_enable(di->mlc_dev) && IS_MLO_RXFIFO(di->fifo)) {
			uint16 cd;
			cd	= mlc_hwalite_rxfifo_cd(di->mlc_dev, fifo);
			curr	= (uint16)((cd - B2I(rcvptrbase, di->dd64_size)));
		} else
#endif /* BCM_HWALITE */
		{
			ASSERT(is_aux == FALSE);
#ifdef BCM_PCAP
			if (di->rs0cd_reset) {
				curr = dma_pcap_rx_rs0cd_fixup(di, curr);
			} else
#endif /* BCM_PCAP */
			{
				curr = DMA64_RX_CD(di);
#ifdef BCM_HWALITE
				if (curr > di->nrxd) {
					return;
				}
#endif
			}
		}

		nproc = NRXDACTIVE_MLO(rxin, curr, nrxd);

#ifdef WLSPP_NIC
#ifdef BCMDBG
		if (!((di->spp_nic_enab == FALSE) || (nbound <= nproc))) {
			uint16 hw_curr;

#ifdef BCM_HWALITE
			if (mlc_hwalite_is_enable(di->mlc_dev) && IS_MLO_RXFIFO(di->fifo)) {
				uint16 cd;
				cd	= mlc_hwalite_rxfifo_cd(di->mlc_dev, fifo);
				hw_curr	= (uint16)((cd - B2I(rcvptrbase, di->dd64_size)));
			} else
#endif /* BCM_HWALITE */
			{
				hw_curr = DMA64_RX_CD(di);
			}

			printf("%s: %s: spp_nic_enab %d nbound %d nproc %d rxin %d rxout %d "
				"curr %d cd_peek %d hw_curr %d nrxd %d is_aux %d mlo_unit %d\n",
				di->name, __FUNCTION__, di->spp_nic_enab, nbound, nproc, rxin,
				rxout, curr, *cd_peek_p, hw_curr, nrxd, is_aux, mlo_unit);
		}
#endif /* BCMDBG */

		ASSERT((di->spp_nic_enab == FALSE) || (nbound <= nproc));
#endif /* WLSPP_NIC */

#if defined(D11_SPLIT_RX_FD)
		if (di->split_fifo) {
			/* X: FIFO-0, Y:FIFO-1
			 * = X or Y for X == Y
			 * = X for X < Y
			 * = Y for X > Y
			 */
			uint16 nproc_di = NRXDACTIVE(rxin, di->linked_di->rs0cd);
			nproc = MIN(nproc, nproc_di);
			nproc = MIN(nproc, nbound);

			di->rs0cd = curr;
		} else if (di->sep_rxhdr) {
			/* Process Two descr per packet */
			nproc = MIN((nbound << 1), nproc);
		} else
#endif /* D11_SPLIT_RX_FD */
		{
			nproc = MIN(nproc, nbound);
		}
		curr =  RXD_MLO(rxin + nproc, nrxd);
	}

	*nrxd_p	  = nrxd;
	*rxin_p   = rxin;
	*rxout_p  = rxout;
	*curr_p   = curr;
	*nproc_p  = nproc;
	if (_rxd64_p) *_rxd64_p	  = _rxd64;
	if (rxpdata_p) *rxpdata_p = rxpdata;

} /* _dma_get_rx_proc_params */

/** returns list of entries on the ring, in the order in which they were placed on the ring */
static void * BCMFASTPATH
_dma_getnext_rxlist(dma_info_t *di, uint nbound, uint *rxfifocnt, bool forceall,
	bool is_aux, uint32 mlo_unit)
{
	void	**head = NULL, **tail = NULL, *head0 = NULL, *tail0 = NULL;
	dmaaddr_t pa;
	dma64dd_t *_rxd64;	/* pointer to dma64 rx descriptor ring */
#ifdef BCM_HWALITE
	dma_mlo_aux_info_t *aux_info = NULL;
#endif /* BCM_HWALITE */
	uint32	nrxd;
	uint16	rxin, rxout;
	uint16	curr = 0, nproc = 0;

	BCM_REFERENCE(pa);

	if ((di->nrxd == 0) || DMA_HWA_RX(di)) {
		return NULL;
	}

#ifdef BCM_BACKPLANE_TIMEOUT
	if (!si_deviceremoved(di->sih))
	{
		/* if forcing, dma engine must be disabled */
		ASSERT(!forceall || !dma_rxenabled((hnddma_t *)di));
	}
#endif /* BCM_BACKPLANE_TIMEOUT */

#ifdef PP_XPM
	if (di->pp_xpm_enab) {
		return _dma_pp_xpm_getnext_rxlist(di, nbound, rxfifocnt,
			forceall, is_aux, mlo_unit);
	}
#endif /* PP_XPM */

	_dma_get_rx_proc_params(di, nbound, forceall, is_aux, mlo_unit,
		&nrxd, &rxin, &rxout, &curr, &nproc, &_rxd64, NULL);
	if ((nrxd == 0) || (nproc == 0)) {
		return NULL;
	}

#ifdef BCM_HWALITE
	if (is_aux) {
		ASSERT(mlo_unit <= DMA_MAX_MLO_AUX_LINKS);
		aux_info = &di->aux_info[mlo_unit - 1];
	}
#endif /* BCM_HWALITE */

#if defined(D11_SPLIT_RX_FD)
	if ((di->split_fifo) && (di->split_fifo == SPLIT_FIFO_0)) {
		head = &di->linked_di->dma_rx_pkt_list.head_pkt;
		tail = &di->linked_di->dma_rx_pkt_list.tail_pkt;
	} else
#endif /* D11_SPLIT_RX_FD */
	{
#ifdef BCM_HWALITE
		if (is_aux) {
			head = &aux_info->dma_rx_pkt_list.head_pkt;
			tail = &aux_info->dma_rx_pkt_list.tail_pkt;
		} else
#endif /* BCM_HWALITE */
		{
			head = &di->dma_rx_pkt_list.head_pkt;
			tail = &di->dma_rx_pkt_list.tail_pkt;
		}
	}

	/* Update actual number of buffers retrieved from RxFIFO decriptors */
	if (rxfifocnt != NULL) {
		*rxfifocnt += nproc;
	}

	if ((*head == NULL) || (*tail == NULL)) {
		DMA_PRINT(("%s: *head %p *tail %p nproc %d\n", di->name,
			*head, *tail, nproc));
		OSL_SYS_HALT();
	}

	head0 = *head;
	tail0 = NULL;

	while (rxin != curr) {
#if defined(D11_SPLIT_RX_FD)
		if (di->sep_rxhdr) {
			dma64addr_t pa64 = { .low_addr = 0, .high_addr = 0 };
			PHYSADDR64HISET(pa64, (BUS_SWAP32(R_SM(&_rxd64[rxin].addrhigh)) -
				di->dataoffsethigh));
			if ((PHYSADDR64HI(pa64) & PCI64ADDR_HIGH) != 0) {
				rxin = NEXTRXD_MLO(rxin, nrxd);
				continue;
			}
		}
#endif /* D11_SPLIT_RX_FD */

		tail0 = *head;
		*head = PKTLINK(*head);

#ifndef DONGLEBUILD
#if !defined(OSL_CACHE_COHERENT)
		PHYSADDRLOSET(pa, (BUS_SWAP32(R_SM(&_rxd64[rxin].addrlow)) -
			di->dataoffsetlow));
		PHYSADDRHISET(pa, (BUS_SWAP32(R_SM(&_rxd64[rxin].addrhigh)) -
			di->dataoffsethigh));
		/* clear this packet from the descriptor ring */
		DMA_UNMAP(di->osh, pa, di->rxbufsize, DMA_RX, tail0, &di->rxp_dmah[rxin]);
#endif  /* ! OSL_CACHE_COHERENT */
#endif /* ! DONGLEBUILD */

		/* NEXTRXD */
		rxin = NEXTRXD_MLO(rxin, nrxd);
	}

#ifdef BCM_HWALITE
	if (is_aux) {
		aux_info->rs0cd	= curr;
		aux_info->rxin	= rxin;
	} else
#endif /* BCM_HWALITE */
	{
		di->rs0cd	= curr;
		di->rxin	= rxin;
		di->rxavail	= nrxd - NRXDACTIVE_MLO(rxin, rxout, nrxd) - 1;

#if defined(D11_SPLIT_RX_FD)
		if (di->split_fifo) {
			di->linked_di->rxin = di->rxin;
			di->linked_di->rxavail = di->rxavail;
		}
#endif /* D11_SPLIT_RX_FD */
	}

	if (tail0) {
		PKTSETLINK(tail0, NULL);
		if (*head == NULL) {
			*tail = NULL;
		}

		return head0;
	}

	return (NULL);
} /* _dma_getnext_rxlist */
#endif /* BULKRX_PKTLIST */

void BCMFASTPATH
dma64_dd_upd_64_from_params(dma_info_t *di, dma64dd_t *ddring, dma64addr_t pa, uint outidx,
	uint32 *flags, uint32 bufcount)
{
	uint32 ctrl2 = bufcount & D64_CTRL2_BC_MASK;

	/* bit 63 is arleady set for host addresses by the caller */
	W_SM(&ddring[outidx].addrlow, BUS_SWAP32(pa.loaddr + di->dataoffsetlow));
	W_SM(&ddring[outidx].addrhigh, BUS_SWAP32(pa.hiaddr + di->dataoffsethigh));
	W_SM(&ddring[outidx].ctrl1, BUS_SWAP32(*flags));
	W_SM(&ddring[outidx].ctrl2, BUS_SWAP32(ctrl2));

	if (di->hnddma.dmactrlflags & DMA_CTRL_PEN) {
		if (DMA64_DD_PARITY(&ddring[outidx])) {
			W_SM(&ddring[outidx].ctrl2, BUS_SWAP32(ctrl2 | D64_CTRL2_PARITY));
		}
	}

	/* memory barrier before posting the descriptor */
	DMB();

#if !defined(OSL_CACHE_COHERENT)
#if (defined(BCM_ROUTER) && !defined(BULK_DESCR_FLUSH))
	DMA_MAP(di->osh, (void *)(((unsigned long)(&ddring[outidx])) & ~0x1f), 32, DMA_TX,
		NULL, NULL);
#endif
#endif  /* ! OSL_CACHE_COHERENT */
}

#if defined(D11_SPLIT_RX_FD)
static bool BCMFASTPATH
dma_splitrxfill(dma_info_t *di)
{
	void *p = NULL;
#ifdef BULKRX_PKTLIST
	void *tail = NULL;
#endif
	uint16 rxin1, rxout1;
	uint16 rxin2, rxout2;
	uint32 flags = 0;
	uint n;
	uint i;
	dmaaddr_t pa;
	bool ring_empty;
	uint pktlen;
	uint extra_offset = 0, extra_pad = 0;
	dma64addr_t pa64 = { .low_addr = 0, .high_addr = 0 };
	uint alignment_req = (di->hnddma.dmactrlflags & DMA_CTRL_RX_ALIGN_8BYTE) ?
				8 : 1;	/* MUST BE POWER of 2 */

	ring_empty = FALSE;
	/*
	 * Determine how many receive buffers we're lacking
	 * from the full complement, allocate, initialize,
	 * and post them, then update the chip rx lastdscr.
	 * Both DMA engines may not have same no. of lacking descrs
	 * Take MIN of both
	 */
	if (di->split_fifo == SPLIT_FIFO_0) {
		/* For split fifo fill, this function expects fifo-1 di to proceed further. */
		/* fifo-0 requests can be discarded since fifo-1 request will fill up both fifos */
		return TRUE;
	}
	rxin1 = di->rxin;
	rxout1 = di->rxout;
	rxin2 = di->linked_di->rxin;
	rxout2 = di->linked_di->rxout;

	/* Always post same no of descriptors to fifo-0 & fifo-1 */
	/* While reposting , consider the fifo which has consumed least no of descriptors */
	n = MIN(di->nrxpost - NRXDACTIVE(rxin1, rxout1),
		di->linked_di->nrxpost - NRXDACTIVEPERHANDLE(rxin2, rxout2, di->linked_di));

	if (di->rxbufsize > BCMEXTRAHDROOM)
		extra_offset = di->rxextrahdrroom;
	/* Assert that both DMA engines have same pktpools */
	ASSERT(di->pktpool == di->linked_di->pktpool);

#ifdef BULKRX_PKTLIST
	if (DMA_BULKRX_PATH(di))
		tail = di->dma_rx_pkt_list.tail_pkt;
#endif
	for (i = 0; i < n; i++) {
		/* the di->rxbufsize doesn't include the extra headroom, we need to add it to the
		   size to be allocated
		*/
		if (POOL_ENAB(di->pktpool)) {
			ASSERT(di->pktpool);
			p = pktpool_get(di->pktpool);
#ifdef BCMDBG_POOL
			if (p)
				PKTPOOLSETSTATE(p, POOL_RXFILL);
#endif /* BCMDBG_POOL */
		}

		if (p == NULL) {
			DMA_TRACE(("%s: dma_rxfill: out of rxbufs\n", di->name));
			if (i == 0) {
				if (dma_rxidle((hnddma_t *)di)) {
					DMA_TRACE(("%s: rxfill64: ring is empty !\n",
						di->name));
					ring_empty = TRUE;
				}
			}
			di->hnddma.rxnobuf++;
			break;
		}

		if (extra_offset)
			PKTPULL(di->osh, p, extra_offset);

		/* Adjust the address of p for address alignment */
		if (alignment_req > 1) {
			extra_pad = (alignment_req - (uintptr)PKTDATA(di->osh, p)) &
				(alignment_req - 1);

		if (extra_pad) {
				PKTPULL(di->osh, p, extra_pad);
			}

			PKTSETLEN(di->osh, p, PKTLEN(di->osh, p) & ~(alignment_req - 1));
		}

		/* Do a cached write instead of uncached write since DMA_MAP
		 * will flush the cache.
		*/
		*(uint16 *)(PKTDATA(di->osh, p)) = 0;

#if !defined(OSL_CACHE_COHERENT)
#if defined(linux) && (defined(__mips__) || defined(STB) || defined(BCM_ROUTER))
		DMA_MAP(di->osh, PKTDATA(di->osh, p), sizeof(uint16), DMA_TX, NULL, NULL);
#endif
#endif  /* ! OSL_CACHE_COHERENT */

		if (DMASGLIST_ENAB)
			bzero(&di->rxp_dmah[rxout1], sizeof(hnddma_seg_map_t));
#if !defined(OSL_CACHE_COHERENT)
		pa = DMA_MAP(di->osh, PKTDATA(di->osh, p),
			di->rxbufsize, DMA_RX, p,
			&di->rxp_dmah[rxout1]);
#else
		OSL_VIRT_TO_PHYSADDR(di->osh, PKTDATA(di->osh, p), pa);
#endif
		if (di->hnddma.dmactrlflags & DMA_CTRL_RX_ALIGN_8BYTE) {
			ASSERT(ISALIGNED(PHYSADDRLO(pa), 8));
		}
		else {
			ASSERT(ISALIGNED(PHYSADDRLO(pa), 4));
		}

#ifdef BULKRX_PKTLIST
		if (DMA_BULKRX_PATH(di)) {
			PKTSETLINK(p, NULL);
			if (tail) {
				PKTSETLINK(tail, p);
				di->dma_rx_pkt_list.tail_pkt = tail = p;
			} else {
				ASSERT((di->dma_rx_pkt_list.head_pkt == NULL));
				di->dma_rx_pkt_list.head_pkt = tail = p;
				di->dma_rx_pkt_list.tail_pkt = tail;
			}
		} else
#endif
		{
			/* save the free packet pointer */
			ASSERT(di->rxp[rxout1] == NULL);
			di->rxp[rxout1] = p;
		}

		/* TCM Descriptor */
		flags = 0;
		pktlen = PKTLEN(di->osh, p);
		if (rxout1 == (di->nrxd - 1))
			flags = D64_CTRL1_EOT;

		dma64_dd_upd(di, di->rxd64, pa, rxout1, &flags, pktlen);

		rxout1 = NEXTRXD(rxout1);		/* update rxout */

		/* Now program host descriptor */
		/* Store pointer to packet */

#ifdef BULKRX_PKTLIST
		if (!DMA_BULKRX_PATH(di))
#endif
		{
			di->linked_di->rxp[rxout2] = p;
		}

		flags = 0;	/* Reset the flags */
		if (rxout2 == (di->linked_di->nrxd - 1))
			flags = D64_CTRL1_EOT;

		/* Extract out host length */
		pktlen = PKTFRAGLEN(di->linked_di->osh, p, LB_FRAG1);

		/* Extract out host addresses */
		pa64.loaddr = PKTFRAGDATA_LO(di->linked_di->osh, p, LB_FRAG1);
		pa64.hiaddr = PKTFRAGDATA_HI(di->linked_di->osh, p, LB_FRAG1);

		if (di->hnddma.dmactrlflags & DMA_CTRL_RX_ALIGN_8BYTE) {
			ASSERT(ISALIGNED(PHYSADDRLO(pa64.loaddr), 8));
		}

		/* load the descriptor */
		dma64_dd_upd_64_from_params(di->linked_di, di->linked_di->rxd64, pa64, rxout2,
			&flags, pktlen);

		rxout2 = NEXTRXD(rxout2);	/* update rxout */
	}

	di->rxout = rxout1;
	di->linked_di->rxout = rxout2;

	/* update the chip lastdscr pointer */
	DMA64_RX_LD_UPD(di, rxout1);
	DMA64_RX_LD_UPD(di->linked_di, rxout2);

	return !ring_empty;
}
#endif /* D11_SPLIT_RX_FD */

#ifdef BCM_PCAP
/**
 * A 'receive' DMA engine must be fed with buffers to write received data into. This function
 * 'posts' receive buffers. If the 'packet pool' feature is enabled, the buffers are drawn from the
 * packet pool. Otherwise, the buffers are retrieved using the OSL 'PKTGET' macro.
 *
 *  return FALSE is refill failed completely and ring is empty
 *  this will stall the rx dma and user might want to call rxfill again asap
 */
static bool BCMFASTPATH
dma_pcap_rxfill(hnddma_t *dmah)
{
	dma_info_t *di = DI_INFO(dmah);
	uint16 rxin, rxout;
	uint16 rxactive;
	uint32 flags = 0;
	uint n;
	uint i;
	bool ring_empty;
	int err;
	uint32 host_pktid;
	dma64addr_t pa64;
	void *pcap_hdl = di->dmacommon->pcap_hdl;

	if (DMA_HWA_RX(di)) {
		DMA_ERROR(("%s: HWA unsupported\n", __FUNCTION__));
		return FALSE;
	}

	if (di->rxfill_suspended) {
		DMA_TRACE(("%s: PCAP rx fill suspend\n", __FUNCTION__));
		return FALSE;
	}

	ring_empty = FALSE;

	/*
	 * Determine how many receive buffers we're lacking
	 * from the full complement, allocate, initialize,
	 * and post them, then update the chip rx lastdscr.
	 */

	rxin = di->rxin;
	rxout = di->rxout;
	rxactive = NRXDACTIVE(rxin, rxout);

	DMA_TRACE(("%s: PCAP in/out/activ %d/%d/%d\n", __FUNCTION__, rxin, rxout, rxactive));
	/* if currently posted is more than requested, no need to do anything */
	if (MIN(di->nrxd, di->nrxpost) <= rxactive) {
		goto done;
	}

	/* No. of packets to post is : n = MIN(nrxd, nrxpost) - NRXDACTIVE; */
	n = MIN(di->nrxd - 1, di->nrxpost) - rxactive;

#ifdef BCM_PCAP
	if (di->rs0cd_reset) {
		/* After RX DMA reset, RS0CD is stuck until next rxd is
		 * consumed. Ensure that rxout +n != the RS0CD at reset,
		 * to avoid ambiguity and correctly detect when RS0CD
		 * has truly advanced.
		 */
		if (n > 0 && RXD(rxout + n) == di->rs0cd_last) {
			n--;
		}
	}
#endif /* BCM_PCAP */

	DMA_TRACE(("%s: %s:  post %d\n", di->name, __FUNCTION__, n));

	for (i = 0; i < n; i++) {
		/* fill from the pcap host buffer pool */
		err = wlc_pcap_txrxfill(pcap_hdl, &pa64, &host_pktid);
		if (err != BCME_OK) {
			DMA_TRACE(("%s: dma_pcap_rxfill: out of rxbufs err=%d\n", di->name, err));
			if (i == 0) {
				if (dma_rxidle(dmah)) {
					DMA_ERROR(("%s: rxfill64: ring is empty !\n",
						di->name));
					ring_empty = TRUE;
				}
			}
			di->hnddma.rxnobuf += (n - i);
			break;
		}

		/* save the free packet pointer */
		ASSERT(di->rxp[rxout] == NULL);
		di->rxp[rxout] = (void *)host_pktid;

		/* reset flags for each descriptor */
		flags = D64_CTRL1_SOF;

		if (di->hnddma.dmactrlflags & DMA_CTRL_RX_ALIGN_8BYTE) {
			ASSERT(ISALIGNED(PHYSADDR64LO(pa64), 8));
		}

		/* Set bit 63 to indicate PCIE addr */
		PHYSADDR64HISET(pa64, PHYSADDR64HI(pa64) | PCI64ADDR_HIGH);

		if (rxout == (di->nrxd - 1))
			flags |= D64_CTRL1_EOT;
		dma64_dd_upd_64_from_params(di, di->rxd64, pa64, rxout,	&flags, PCAP_PKT_BUFSZ);

		rxout = NEXTRXD(rxout);
	} /* for n */

	/* update the chip lastdscr pointer */
	DMA64_RX_LD_UPD(di, rxout);

	di->rxout = rxout;

done:
	return !ring_empty;
} /* dma_pcap_rxfill */

void
dma_pcap_rx(hnddma_t *dmah)
{
	dma_info_t *di = DI_INFO(dmah);
	uint16 rxin, rxin_next, rs0cd;
	void *p;
	uint32 host_pktid;
	void *pcap_hdl = di->dmacommon->pcap_hdl;
	bool final, full;

	if ((di->nrxd == 0) || DMA_HWA_RX(di)) {
		return;
	}

	if (di->rxin == di->rxout) {
		/* no posted descriptors */
		DMA_ERROR(("%s: PCAP rxin/rxout/rs0cd/RS0CD %d/%d/%d/%d\n", __FUNCTION__,
			di->rxin, di->rxout, di->rs0cd, DMA64_RX_CD(di)));
		return;

	}
#ifdef BCM_BACKPLANE_TIMEOUT
	if (!si_deviceremoved(di->sih))
	{
		/* if forcing, dma engine must be disabled */
		ASSERT(!forceall || !dma_rxenabled(dmah));
	}
#endif /* BCM_BACKPLANE_TIMEOUT */

	rs0cd = DMA64_RX_CD(di);

	if (di->rs0cd_reset) {
		rs0cd = dma_pcap_rx_rs0cd_fixup(di, rs0cd);
	}

	di->rs0cd = rs0cd;
	rxin = di->rxin;

	DMA_TRACE(("%s: PCAP rxin/rs0cd %d/%d\n", __FUNCTION__, rxin, di->rs0cd));
	/* iterate from rxin to rs0cd, advancing rxin */
	while (rxin != di->rs0cd) {
		/* get host dma buffer pointer */
		p = di->rxp[rxin];
		host_pktid = (uint32)p;

		/* advance */
		rxin_next = NEXTRXD(rxin);

		if (rxin_next == di->rs0cd) {
			final = TRUE;
		} else {
			final = FALSE;
		}

		/* dispatch to bus layer, indicate end of batch */
		full = wlc_pcap_rx(pcap_hdl, host_pktid, final);
		if (full) {
			/* couldn't send up -- work item starvation */
			DMA_ERROR(("%s: overflow\n", __FUNCTION__));
			break;
		}

		di->rxp[rxin] = NULL;

		rxin = rxin_next;
	}

	di->rxin = rxin;
	di->rxavail = di->nrxd - NRXDACTIVE(di->rxin, di->rxout) - 1;
}

/** Called when e.g. flushing d11 packets */
static void BCMFASTPATH
dma_pcap_rxreclaim(hnddma_t *dmah)
{
	dma_info_t *di = DI_INFO(dmah);
	void *pcap_hdl = di->dmacommon->pcap_hdl;
	void *p;
	uint32 host_pktid;
	int cnt = 0;

	DMA_TRACE(("%s: dma_pcap_rxreclaim\n", di->name));

	p = dma_getnextrxp(dmah, TRUE);
	while (p != NULL) {
		host_pktid = (uint32)p;
		/* completion to bus layer for the packet */
		wlc_pcap_reclaim_pkt(pcap_hdl, host_pktid);

		/* force=TRUE to get all descriptors */
		p = dma_getnextrxp(dmah, TRUE);

		cnt++;
	}
	if (cnt > 0) {
		/* flush the last items to bus layer */
		wlc_pcap_reclaim_pkts_done(pcap_hdl);
	}
} /* dma_pcap_rxreclaim */

/* After DMA channel resets for enable/disable of promisc pcap mode, HW register
 * RS0CD continues reporting the last completed descriptor index. Must wait until
 * the next transaction for a valid descriptor.
 */
static uint16
dma_pcap_rx_rs0cd_fixup(dma_info_t *di, uint16 rs0cd_hw)
{
	uint16 rs0cd_fixed = rs0cd_hw;

	if (rs0cd_hw == di->rs0cd_last && di->rxin == 0) {
		/* HW reports stale RS0CD: ring is actually empty */
		DMA_NONE(("%s: corrected rxin/rxout/rs0cd/rs0cd_hw %d/%d/%d/%d\n",
			__FUNCTION__, di->rxin, di->rxout, di->rs0cd_last, rs0cd_hw));
		rs0cd_fixed = di->rxin;
	} else {
		/* HW value (rs0cd_hw) is correct */
		di->rs0cd_reset = FALSE;
		DMA_NONE(("%s: active rxin/rxout %d/%d rs0cd= %d->%d\n",
			__FUNCTION__, di->rxin, di->rxout, di->rs0cd_last, rs0cd_hw));
	}

	return rs0cd_fixed;
}
#endif /* BCM_PCAP */

void *
dma_rx_head_pkt_entry_ptr(hnddma_t *dmah)
{
	dma_info_t *di = DI_INFO(dmah);
	void *p = NULL;
	BCM_REFERENCE(p);
#ifdef BULKRX_PKTLIST
	p = &di->dma_rx_pkt_list.head_pkt;
	DMA_NONE(("%s: dma_rx_head_pkt_entry_ptr head = %p tail = %p\n", di->name,
		di->dma_rx_pkt_list.head_pkt, di->dma_rx_pkt_list.tail_pkt));
#else
	p = &di->rxp[di->rxin];
	DMA_NONE(("%s: dma_rx_head_pkt_entry_ptr head = %p\n", di->name, p));
#endif
	return p;
}

#ifdef WLSPP_NIC
/* Peeks into the latest CD and returns newly arrived packet count
 * i.e., cd_peek to cd_latest
 */
uint16 BCMFASTPATH
dma_rx_spp_progressive_pktcnt(hnddma_t *dmah, uint32 mlo_unit)
{
	dma_info_t *di = DI_INFO(dmah);
	uint16	* cd_peek_p;
#ifdef BCM_HWALITE
	dma_mlo_aux_info_t *aux_info = NULL;
	uint	fifo;
	uint32	rcvptrbase;
#endif /* BCM_HWALITE */
	uint32	nrxd, curr;
	uint32	rxpkt_ready = 0;

	ASSERT(di->spp_nic_enab);

	cd_peek_p = &di->cd_peek;
	nrxd = di->nrxd;

	/* Get the latest CD */
#ifdef BCM_HWALITE
	rcvptrbase = di->rcvptrbase;
	fifo = di->fifo;

	if ((fifo == RX_MLO_FIFO3) && mlo_unit) { /* is_aux */

		ASSERT(mlo_role(di->mlc_dev) == MLC_ROLE_MAP);
		ASSERT(mlo_unit <= DMA_MAX_MLO_AUX_LINKS);
		aux_info = &di->aux_info[mlo_unit - 1];

		mlc_hwalite_get_aux_rxdma_info(di->mlc_dev, mlo_unit, &nrxd,
			NULL, NULL, &rcvptrbase);
		cd_peek_p = &aux_info->cd_peek;
		fifo = MLO_MLOUNIT_TO_RXFIFO(mlo_unit);
	}

	if (mlc_hwalite_is_enable(di->mlc_dev) && IS_MLO_RXFIFO(di->fifo)) {
		uint16 cd;
		cd	= mlc_hwalite_rxfifo_cd(di->mlc_dev, fifo);
		curr	= (uint16)((cd - B2I(rcvptrbase, di->dd64_size)));
	} else
#endif /* BCM_HWALITE */
	{
#ifdef BCM_PCAP
		ASSERT(di->rs0cd_reset == FALSE);
#endif /* BCM_PCAP */
		curr = DMA64_RX_CD(di);
	}

	/* Get the count of newly arrived packets */
	rxpkt_ready = NRXDACTIVE_MLO(*cd_peek_p, curr, nrxd);

	/* Update CD peek to latest */
	*cd_peek_p = curr;

	return rxpkt_ready;
} /* dma_rx_spp_progressive_pktcnt */
#endif /* WLSPP_NIC */
