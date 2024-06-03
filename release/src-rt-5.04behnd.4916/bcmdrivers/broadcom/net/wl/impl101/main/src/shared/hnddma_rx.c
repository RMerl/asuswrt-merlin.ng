/*
 * Generic Broadcom Home Networking Division (HND) DMA receive routines.
 * This supports the following chips: BCM42xx, 44xx, 47xx .
 *
 * Copyright (C) 2023, Broadcom. All Rights Reserved.
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
 * $Id: hnddma_rx.c 831037 2023-10-06 20:38:54Z $
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
#define BCM_NBUFF_PKT_BPM /* Packet data buffers, are from BPM pool */
#ifndef BCMDBG
#define BCM_NBUFF_PKT_BPM_COHERENCY /* BPM buffers are cache invalidated */
#endif
#endif /* BCM_NBUFF_PKT && CONFIG_BCM_BPM */

#if defined(D11_SPLIT_RX_FD)
static bool dma_splitrxfill(dma_info_t *di);
#endif

#ifdef BULKRX_PKTLIST
static void *_dma_getnext_rxlist(dma_info_t *di, uint nbound, bool forceall);
#endif /* BULKRX_PKTLIST */

#ifdef BCM_PCAP
static bool BCMFASTPATH dma_pcap_rxfill(hnddma_t *dmah);
static void BCMFASTPATH dma_pcap_rxreclaim(hnddma_t *dmah);
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
/* For 6717/6726/6765, PhyRx Status is transferred in headroom for Management frames but
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

	iter = di->rxin;

	while (iter != di->rxout) {

#if !defined(OSL_CACHE_COHERENT)
		dmaaddr_t pa;

		PHYSADDRLOSET(pa,
			(BUS_SWAP32(R_SM(&di->rxd64[iter].addrlow)) - di->dataoffsetlow));
		PHYSADDRHISET(pa,
			(BUS_SWAP32(R_SM(&di->rxd64[iter].addrhigh)) - di->dataoffsethigh));

#if defined(BCM_SECURE_DMA)
		DMA_SYNC(di->osh, pa, di->rxbufsize, DMA_RX);
#else /* ! BCM_SECURE_DMA */
		/* Cache invalidate entire PhyRx Status buffer */
		DMA_UNMAP(di->osh, pa, di->rxbufsize, DMA_RX, NULL, NULL);
#endif /* ! BCM_SECURE_DMA */
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
dma_rx(hnddma_t *dmah, rx_list_t *rx_list,  uint nbound)
{
	dma_info_t *di = DI_INFO(dmah);

	void *head, *head0, *prev = NULL;
	uint len;
	uint pkt_len;
	int resid = 0;
#if defined(D11_SPLIT_RX_FD)
	uint tcm_maxsize = 0;		/* max size of tcm descriptor */
#endif /* D11_SPLIT_RX_FD */
	void *data, *free_p;
	uint rxoffset;

	head0 = head = _dma_getnext_rxlist(di, nbound, FALSE);

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
		data = PKTDATA(di->osh, head);

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
				DMA_ERROR(("%s: dma_rx: frame length (%d)."
					"Packet is not ready, Big Hammer\n",
					di->name, len));
				free_p = head;
				if (prev != NULL) {
					head = PKTLINK(free_p);
					PKTSETLINK(prev, head);
				} else {
					head0 = head = PKTLINK(free_p);
				}
				PKTSETLINK(free_p, NULL);
				PKTFREE(di->osh, free_p, FALSE);
				di->hnddma.rxgiants++;
				continue;
			}
		}
#endif /* !__mips__ && !(STB || BCM_ROUTER) */
		DMA_TRACE(("%s: dma_rx len %d\n", di->name, len));

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
			PKTSETLEN(di->osh, head, pkt_len);
		}

		resid = len - (di->rxbufsize - rxoffset);

		if (resid <= 0) {
			/* Single frame, all good */
		} else if (di->hnddma.dmactrlflags & DMA_CTRL_RXSINGLE) {
			DMA_ERROR(("%s: dma_rx: corrupted length (%d). "
				"Frame received in multiple buffers\n", di->name, len));
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
				if (prev != NULL) {
					head = PKTLINK(free_p);
					PKTSETLINK(prev, head);
				} else {
					head0 = head = PKTLINK(free_p);
				}

				PKTSETLINK(free_p, NULL);
				PKTFREE(di->osh, free_p, FALSE);
				continue;
			}
		} else {
			/* multi-buffer rx */
			uint32 rx_frm_sts;
			int num_dd; /* Number of DMA descriptors used for this packet. */
			void *tail, *p;

			rx_frm_sts = ltoh32(*(uint32 *)PKTDATA(di->osh, head));
			num_dd = ((rx_frm_sts & D64_RX_FRM_STS_DSCRCNT) >>
				D64_RX_FRM_STS_DSCRCNT_SHIFT);

			/* Receive Frame Status Header DscrCntr (27:24) Contains one less
			 * than the number of DMA descriptors used to transfer this frame
			 * into memory. So incrementing it by 1 will give the actual number
			 * of DMA descriptors used for this transfer.
			 */
			num_dd++;
			_dma_update_num_dd(di, &num_dd);
#ifdef BCMDBG
			/* get rid of compiler warning */
			p = NULL;
#endif /* BCMDBG */
			tail = head;
			while (num_dd > 0) {

				p = (PKTLINK(tail) != NULL) ? PKTLINK(tail) :
					_dma_getnext_rxlist(di, 1, FALSE);

				PKTSETNEXT(di->osh, tail, p);
				if (p == NULL)
					break;

				pkt_len = MIN(resid, (int)di->rxbufsize);
#if defined(D11_SPLIT_RX_FD)
				if (di->sep_rxhdr)
					PKTSETLEN(di->osh, p, MIN(pkt_len, tcm_maxsize));
				else
#endif /* D11_SPLIT_RX_FD */
					PKTSETLEN(di->osh, p, pkt_len);

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
				PKTSETLINK(tail, NULL);
			}
#ifdef BCMDBG
			if (resid > 0) {
				DMA_ERROR(("%s: dma_rx: rxin %d rxout %d, hw_curr %d\n",
					di->name, di->rxin, di->rxout, DMA64_RX_CD(di)));
				if (p != NULL) {
					DMA_ERROR(("%s: dma_rx: all descriptors processed but only"
						" partial frame received (missing %d bytes)\n",
						di->name, resid));
				}
			}
#endif /* BCMDBG */
			if ((di->hnddma.dmactrlflags & DMA_CTRL_RXMULTI) == 0) {
#ifdef WL_EAP_REKEY_WAR
				if (!di->dis_msgs)
#endif
				{
					DMA_ERROR(("%s: dma_rx: bad frame length (%d), frmsts "
						"(0x%08x). Frame received in multiple buffers\n",
						di->name, len, rx_frm_sts));
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
					free_p = head;
					if (prev != NULL) {
						head = PKTLINK(free_p);
						PKTSETLINK(prev, head);
					} else {
						head0 = head = PKTLINK(free_p);
					}
					PKTSETLINK(free_p, NULL);
					PKTFREE(di->osh, free_p, FALSE);
					continue;
				}
			}
		}

#if defined(STS_XFER_PHYRXS_SEQ)
nextpkt_proc:
#endif /* STS_XFER_PHYRXS_SEQ */

		rx_list->rxfifocnt++;
		prev = head;
		head = PKTLINK(head);
	} /* End While() */

	rx_list->rx_head = head0;
	rx_list->rx_tail = prev;
} /* dma_rx */

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
/* Refill buffers in descriptor rings of MLO RxFIFOs for an MLO AUX link */
static void BCMFASTPATH
dma_mlo_rxfill(hnddma_t *dmah, uint32 mlo_unit)
{
	dma_info_t	*di = DI_INFO(dmah);
	struct dma_mlo_aux_info *aux_info;
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
	ASSERT((mlo_unit > 0));
	aux_info = &di->aux_info[mlo_unit - 1];
	rxin  = aux_info->rxin;
	rxout = aux_info->rxout;
	mlc_hwalite_get_aux_rxdma_info(di->mlc_dev,
		mlo_unit, &nrxd, &nrxpost, &fifo_addr_va, &rcvptrbase);
	_rxd64 = (dma64dd_t*)((uintptr)fifo_addr_va);

	rxactive = NRXDACTIVE(rxin, rxout);

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

	DMA_TRACE(("%s: dma_mlo_rxfill: post %d mlo_unit %d\n", di->name, n, mlo_unit));

	/* Using Main's pool */
	tail = aux_info->dma_rx_pkt_list.tail_pkt;

	for (i = 0; i < n; i++) {
		/* the di->rxbufsize doesn't include the extra headroom, we need to add it to the
		   size to be allocated
		*/
		/* BCMPKTPOOL Not enabled for NIC */
		uint32 len = di->rxbufsize + extra_offset + alignment_req - 1;

#if defined(BCM_NBUFF_PKT_BPM) && defined(CC_BPM_SKB_POOL_BUILD)
		p = PKTPOOLGET(di->osh, len); /* Use BPM packets and data buffers */
#else
		p = PKTGET(di->osh, len, FALSE);
#endif /* BCM_NBUFF_PKT_BPM && CC_BPM_SKB_POOL_BUILD */

		if (p == NULL) {
			DMA_TRACE(("%s: dma_mlo_rxfill: out of rxbufs\n", di->name));
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
#ifdef BCM_SECURE_DMA
		ASSERT(0);
#endif

#if defined(BCM_NBUFF_PKT_BPM)
		OSL_VIRT_TO_PHYSADDR(di->osh, PKTDATA(di->osh, p), pa);
#else
		pa = DMA_MAP(di->osh, PKTDATA(di->osh, p),
			di->rxbufsize, DMA_RX, p, NULL);
#endif /* ! BCM_NBUFF_PKT_BPM */

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
		if (di->hnddma.dmactrlflags & DMA_CTRL_RX_ALIGN_8BYTE) {
			ASSERT(ISALIGNED(PHYSADDRLO(pa), 8));
		}

		if (rxout == (nrxd - 1))
			flags |= D64_CTRL1_EOT;

		dma64_dd_upd(di, _rxd64, pa, rxout, &flags, di->rxbufsize);

		rxout = NEXTRXD(rxout);
	} /* for n */

#if !defined(BULK_DESCR_FLUSH)
	aux_info->rxout = rxout;
#endif
	/* update the chip lastdscr pointer */

#if !defined(OSL_CACHE_COHERENT)
#if defined(BULK_DESCR_FLUSH)
	{
		uint32 flush_cnt = NRXDACTIVE(aux_info->rxout, rxout);
		if (rxout < aux_info->rxout) {
			/* For RxFiFo dd64_size should be same as sizeof(dma64dd_t)
			 * as BCM_DMA_DESC_ONLY_FLAG will not be set
			 */
			ASSERT(di->dd64_size == sizeof(dma64dd_t));
			DMA_MAP(di->osh, &_rxd64[0], (sizeof(dma64dd_t) * rxout),
				DMA_TX, NULL, NULL);
			flush_cnt -= rxout;
		}
		DMA_MAP(di->osh, &_rxd64[aux_info->rxout],
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

/** returns list of entries on the ring, in the order in which they were placed on the ring */
static void * BCMFASTPATH
_dma_mlo_getnext_rxlist(dma_info_t *di, uint nbound, bool forceall, uint32 mlo_unit)
{
	void	**head = NULL, **tail = NULL, *head0 = NULL, *tail0 = NULL;
	struct dma_mlo_aux_info *aux_info = NULL;
	dma64dd_t	*_rxd64;	/* pointer to dma64 rx descriptor ring */
	dmaaddr_t	pa;
	uint64	fifo_addr_va;
	uint32	nrxd, rcvptrbase;
	uint16	i, curr, nproc = 0;

	BCM_REFERENCE(pa);

	if (mlo_unit == 0) {
		if (di->nrxd == 0)
			return NULL;

		_rxd64 = di->rxd64;
		rcvptrbase = di->rcvptrbase;

		head = &di->dma_rx_pkt_list.head_pkt;
		tail = &di->dma_rx_pkt_list.tail_pkt;
		i = di->rxin;
	} else {

		aux_info = &di->aux_info[mlo_unit - 1];
		mlc_hwalite_get_aux_rxdma_info(di->mlc_dev,
			mlo_unit, &nrxd, NULL, &fifo_addr_va, &rcvptrbase);

		if (nrxd == 0)
			return NULL;

		_rxd64 = (dma64dd_t*)((uintptr)fifo_addr_va);

		head = &aux_info->dma_rx_pkt_list.head_pkt;
		tail = &aux_info->dma_rx_pkt_list.tail_pkt;
		i = aux_info->rxin;
	}

	ASSERT(head);
	ASSERT(tail);
	ASSERT(di->hnddma.dmactrlflags & DMA_CTRL_DESC_BASE_IDX_UPD);

	if (forceall) {
		curr = (mlo_unit == 0) ? di->rxout : aux_info->rxout;
	} else {
		uint16 cd;
		uint fifo = MLO_MLOUNIT_TO_RXFIFO(mlo_unit);

		cd = mlc_hwalite_rxfifo_cd(di->mlc_dev, fifo);
		/* Number of DMA descr to process */
		curr = (uint16)((cd - B2I(rcvptrbase, di->dd64_size)));

		if (mlo_unit == 0) {
			di->rs0cd = curr;
		} else {
			aux_info->rs0cd = curr;
		}

		nproc = NRXDACTIVE(i, curr);
		nproc = MIN(nproc, nbound);
		curr =  RXD(i + nproc);
	}

	head0 = *head;
	tail0 = NULL;
	while (i != curr) {

		tail0 = *head;
		*head = PKTLINK(*head);
		PHYSADDRLOSET(pa, (BUS_SWAP32(R_SM(&_rxd64[i].addrlow)) - di->dataoffsetlow));
		PHYSADDRHISET(pa, (BUS_SWAP32(R_SM(&_rxd64[i].addrhigh)) - di->dataoffsethigh));

		/* clear this packet from the descriptor ring */
#if !defined(OSL_CACHE_COHERENT)
		DMA_UNMAP(di->osh, pa, di->rxbufsize, DMA_RX, tail0, NULL);
#endif  /* ! OSL_CACHE_COHERENT */

		/* NEXTRXD */
		i = NEXTRXD(i);
	}

	if (tail0) {
		PKTSETLINK(tail0, NULL);
		if (mlo_unit == 0) {
			di->rxin = i;
			di->rxavail = di->nrxd - NRXDACTIVE(i, di->rxout) - 1;
		} else {
			aux_info->rxin = i;
		}

		if (*head == NULL) {
			*tail = NULL;
		}
		return head0;
	}

	return (NULL);
}

void BCMFASTPATH
dma_mlo_rx(hnddma_t *dmah, rx_list_t *rx_list,  uint nbound, uint32 mlo_unit)
{
	dma_info_t *di = DI_INFO(dmah);
	void	*head, *head0, *prev = NULL;
	void	*data, *free_p;
	uint	len, pkt_len;
	uint	rxoffset;
	int	resid = 0;

	head0 = head = _dma_mlo_getnext_rxlist(di, nbound, FALSE, mlo_unit);

	rxoffset = di->rxoffset;

	while (head != NULL) {
		data = PKTDATA(di->osh, head);

#if (!defined(__mips__) && !(defined(BCM47XX_CA9) || defined(STB) || \
	defined(BCA_HNDROUTER)))
		len = ltoh16(*(uint16 *)(data));
#else  /* !__mips__ && !BCM47XX_CA9 */

		if (di->rx_waitcomplete_enabled) {
			len = ltoh16(*(uint16 *)(data));
		} else {
			int read_count = 0;
			for (read_count = 200; read_count; read_count--) {
				len = ltoh16(*(uint16 *)(data));
				if (len != 0)
					break;
#if defined(BCM_GMAC3)
				ASSERT(PKTISFWDERBUF(di->osh, head) == FALSE);
#endif /* BCM_GMAC3 */

#if !defined(OSL_CACHE_COHERENT)
				DMA_MAP(di->osh, data, sizeof(uint16), DMA_RX, NULL, NULL);
#endif

				OSL_DELAY(1);
			}

			ASSERT(!di->sts_xfer_phyrxs);

			if (!len) {
				DMA_ERROR(("%s: dma_mlo_rx: mlo_unit %d frame length (%d)."
					"Packet is not ready, Big Hammer\n",
					di->name, mlo_unit, len));
				free_p = head;
				if (prev != NULL) {
					head = PKTLINK(free_p);
					PKTSETLINK(prev, head);
				} else {
					head0 = head = PKTLINK(free_p);
				}
				PKTSETLINK(free_p, NULL);
				PKTFREE(di->osh, free_p, FALSE);
				di->hnddma.rxgiants++;
				continue;
			}

		}
#endif /* !__mips__ && !BCM47XX_CA9 */

		DMA_TRACE(("%s: dma_mlo_rx mlo_unit %d len %d\n", di->name, mlo_unit, len));

		/* set actual length */
		pkt_len = MIN((rxoffset + len), di->rxbufsize);

		ASSERT(!di->sep_rxhdr);
		PKTSETLEN(di->osh, head, pkt_len);

		resid = len - (di->rxbufsize - rxoffset);

		if (resid <= 0) {
			/* Single frame, all good */
		} else if (di->hnddma.dmactrlflags & DMA_CTRL_RXSINGLE) {
			DMA_ERROR(("%s: dma_mlo_rx: mlo_unit %d corrupted length (%d). "
				"Frame received in multiple buffers\n",
				di->name, mlo_unit, len));
			di->hnddma.rxgiants++;

			ASSERT(!di->sts_xfer_phyrxs);
			free_p = head;
			if (prev != NULL) {
				head = PKTLINK(free_p);
				PKTSETLINK(prev, head);
			} else {
				head0 = head = PKTLINK(free_p);
			}

			PKTSETLINK(free_p, NULL);
			PKTFREE(di->osh, free_p, FALSE);
			continue;
		} else {
			/* multi-buffer rx */
			uint32 rx_frm_sts;
			int num_dd; /* Number of DMA descriptors used for this packet. */
			void *tail, *p;

			rx_frm_sts = ltoh32(*(uint32 *)(PKTDATA(di->osh, head)));
			num_dd = ((rx_frm_sts &	D64_RX_FRM_STS_DSCRCNT) >>
				D64_RX_FRM_STS_DSCRCNT_SHIFT);

			/* Receive Frame Status Header DscrCntr (27:24) Contains one less
			 * than the number of DMA descriptors used to transfer this frame
			 * into memory. So incrementing it by 1 will give the actual number
			 * of DMA descriptors used for this transfer.
			 */
			num_dd++;
			_dma_update_num_dd(di, &num_dd);
#ifdef BCMDBG
			/* get rid of compiler warning */
			p = NULL;
#endif /* BCMDBG */
			tail = head;
			while (num_dd > 0) {

				p = (PKTLINK(tail) != NULL) ? PKTLINK(tail) :
					_dma_mlo_getnext_rxlist(di, 1, FALSE, mlo_unit);

				PKTSETNEXT(di->osh, tail, p);
				if (p == NULL)
					break;

				pkt_len = MIN(resid, (int)di->rxbufsize);
				PKTSETLEN(di->osh, p, pkt_len);

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
				PKTSETLINK(tail, NULL);
			}
#ifdef BCMDBG
			if (resid > 0) {
				DMA_ERROR(("%s: dma_mlo_rx, rxin %d rxout %d, hw_curr %d\n",
					di->name, di->rxin, di->rxout, DMA64_RX_CD(di)));
				if (p != NULL) {
					DMA_ERROR(("%s: dma_mlo_rx: all descriptors processed but"
						" only partial frame received (missing %d bytes)\n",
						di->name, resid));
				}
			}
#endif /* BCMDBG */
			if ((di->hnddma.dmactrlflags & DMA_CTRL_RXMULTI) == 0) {
#ifdef WL_EAP_REKEY_WAR
				if (!di->dis_msgs)
#endif /* WL_EAP_REKEY_WAR */
				{
					DMA_ERROR(("%s: dma_mlo_rx: mlo_unit %d bad length (%d). "
						"frmsts (0x%08x). Frame received in multiple "
						"buffers\n", di->name, mlo_unit, len, rx_frm_sts));
				}

				di->hnddma.rxgiants++;

				free_p = head;
				if (prev != NULL) {
					head = PKTLINK(free_p);
					PKTSETLINK(prev, head);
				} else {
					head0 = head = PKTLINK(free_p);
				}
				PKTSETLINK(free_p, NULL);
				PKTFREE(di->osh, free_p, FALSE);
				continue;
			}
		}

		rx_list->rxfifocnt++;
		prev = head;
		head = PKTLINK(head);
	} /* End While() */

	rx_list->rx_head = head0;
	rx_list->rx_tail = prev;
} /* dma_mlo_rx */

static void BCMFASTPATH
dma_mlo_rxreclaim(hnddma_t *dmah, uint32 mlo_unit)
{
	dma_info_t *di = DI_INFO(dmah);
	void	*p, *next = NULL;

	DMA_TRACE(("%s: dma_mlo_rxreclaim of AUX link[%d]\n", di->name, mlo_unit));

	ASSERT(!(di->hnddma.dmactrlflags & DMA_CTRL_UNFRAMED));
	ASSERT(!(di->bcmrx_pcn_fifo));

	p = _dma_mlo_getnext_rxlist(di, 0, TRUE, mlo_unit);

	while (p != NULL) {
		next = PKTLINK(p);
		PKTSETLINK(p, NULL);
		PKTFREE(di->osh, p, FALSE);

		p = next;
	}
} /* dma_mlo_rxreclaim() */
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
#if defined(BCM_NBUFF_PKT_BPM) && defined(CC_BPM_SKB_POOL_BUILD)
				p = PKTPOOLGET(di->osh, len); /* Use BPM packets and data buffers */
#else
				p = PKTGET(di->osh, len, FALSE);
#endif /* BCM_NBUFF_PKT_BPM && CC_BPM_SKB_POOL_BUILD */
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
			/* Reset PhyRx Sts length. 6717/26/65 SW WAR to fix d11rxhdr_t::dma_flags */
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

#ifdef BCM_SECURE_DMA
		pa = SECURE_DMA_MAP(di->osh, PKTDATA(di->osh, p), di->rxbufsize, DMA_RX,
			NULL, NULL, &di->sec_cma_info_rx, 0, SECDMA_RXBUF_POST);
#else
		pa = DMA_MAP(di->osh, PKTDATA(di->osh, p), di->rxbufsize, DMA_RX, p,
			&di->rxp_dmah[rxout]);
#endif /* ! BCM_SECURE_DMA */

#else  /* ! SGLIST_RX_SUPPORT */

#ifdef BCM_SECURE_DMA
		pa = SECURE_DMA_MAP(di->osh, PKTDATA(di->osh, p), di->rxbufsize, DMA_RX,
			NULL, NULL, &di->sec_cma_info_rx, 0, SECDMA_TXBUF_POST);
#else

#if defined(BCM_NBUFF_PKT_BPM)
#if defined(BCMRX_PCN)
		if (di->bcmrx_pcn_fifo) {
			pa = DMA_MAP(di->osh, PKTDATA(di->osh, p),
				di->rxbufsize, DMA_RX, p, NULL);
		}
		else
#endif /* BCMRX_PCN */
		{
			OSL_VIRT_TO_PHYSADDR(di->osh, PKTDATA(di->osh, p), pa);
		}
#else
		pa = DMA_MAP(di->osh, PKTDATA(di->osh, p),
				di->rxbufsize, DMA_RX, p, NULL);
#endif /* ! BCM_NBUFF_PKT_BPM */

#endif /* ! BCM_SECURE_DMA */
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
			dma_mlo_rxfill(dmah, mlo_unit);
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

#ifdef BULKRX_PKTLIST
	if (DMA_BULKRX_PATH(di)) {
		p = _dma_getnext_rxlist(di, 0, TRUE);
	} else
#endif
	p = dma_getnextrxp(dmah, TRUE);
	while (p != NULL) {
		/* For unframed data, we don't have any packets to free */
#if (defined(__mips__) || defined(STB) || defined(BCM_ROUTER)) && defined(linux)
		if (!(di->hnddma.dmactrlflags & DMA_CTRL_UNFRAMED))
#endif /* (__mips__ || STB || BCM_ROUTER) && linux */
		{
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
				pktpool_free(OSH_GET_PCNPKTPOOL(di->osh), p);
			}
			else
#endif /* BCMRX_PCN && BCMRX_PCN_PKTPOOL */
			{
				PKTFREE(di->osh, p, FALSE);
			}
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

#ifdef BCM_HWALITE
	if ((mlo_role(di->mlc_dev) == MLC_ROLE_MAP) && (di->fifo == RX_MLO_FIFO3)) {
		uint32 mlo_unit;

		ASSERT(mlc_hwalite_is_enable(di->mlc_dev));
		ASSERT(di->hnddma.dmactrlflags & DMA_CTRL_MLO_RXFIFO);

		/* Post buffers to Aux Fifo */
		FOREACH_MLO_AAP(di->mlc_dev, mlo_unit) {
			dma_mlo_rxreclaim(dmah, mlo_unit);
		}
	}
#endif /* BCM_HWALITE */

#ifdef BCMPKTPOOL
	if (POOL_ENAB(di->pktpool) && origcb == FALSE)
		pktpool_emptycb_disable(di->pktpool, FALSE);
#endif /* BCMPKTPOOL */

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
#if (!defined(__mips__) && !(defined(STB) || defined(BCM_ROUTER))) || \
	defined(BCM_SECURE_DMA)
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
	if (!(di->hnddma.dmactrlflags & DMA_CTRL_UNFRAMED) && !DMA_PCAP_EN(di)) {
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
#ifdef BCM_SECURE_DMA
		SECURE_DMA_UNMAP(di->osh, pa, di->rxbufsize, DMA_RX, NULL, NULL,
				&di->sec_cma_info_rx, 0);
#else
		DMA_UNMAP(di->osh, pa, di->rxbufsize, DMA_RX, rxp, &di->rxp_dmah[i]);
#endif /* BCM_SECURE_DMA */
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

/** returns list of entries on the ring, in the order in which they were placed on the ring */
static void * BCMFASTPATH
_dma_getnext_rxlist(dma_info_t *di, uint nbound, bool forceall)
{
	uint16 i, curr = 0, nproc = 0;
	void **head = NULL, **tail = NULL, *head0 = NULL, *tail0 = NULL;
	dmaaddr_t pa;
	BCM_REFERENCE(pa);
	if ((di->nrxd == 0) || DMA_HWA_RX(di))
		return (NULL);

#ifdef BCM_BACKPLANE_TIMEOUT
	if (!si_deviceremoved(di->sih))
	{
		/* if forcing, dma engine must be disabled */
		ASSERT(!forceall || !dma_rxenabled((hnddma_t *)di));
	}
#endif /* BCM_BACKPLANE_TIMEOUT */

#if defined(D11_SPLIT_RX_FD)
	if ((di->split_fifo) && (di->split_fifo == SPLIT_FIFO_0)) {
		head = &di->linked_di->dma_rx_pkt_list.head_pkt;
		tail = &di->linked_di->dma_rx_pkt_list.tail_pkt;
	} else
#endif /* D11_SPLIT_RX_FD */
	{
		head = &di->dma_rx_pkt_list.head_pkt;
		tail = &di->dma_rx_pkt_list.tail_pkt;
	}

	i = di->rxin;
	if (forceall) {
		curr = di->rxout;
	} else {
#ifdef BCM_HWALITE
		if (mlc_hwalite_is_enable(di->mlc_dev)) {
			uint16 cd = mlc_hwalite_rxfifo_cd(di->mlc_dev, di->fifo);
			curr = (uint16)((cd - B2I(di->rcvptrbase, di->dd64_size)));
		} else
#endif /* BCM_HWALITE */
		/* Number of DMA descr to process */
		{
			curr = DMA64_RX_CD(di);
		}
		di->rs0cd = curr;

		nproc = NRXDACTIVE(i, curr);
#if defined(D11_SPLIT_RX_FD)
		if (di->split_fifo) {
			/* X: FIFO-0, Y:FIFO-1
			 * = X or Y for X == Y
			 * = X for X < Y
			 * = Y for X > Y
			 */
			uint16 nproc_di = NRXDACTIVE(i, di->linked_di->rs0cd);
			nproc = MIN(nproc, nproc_di);
			nproc = MIN(nproc, nbound);
		} else if (di->sep_rxhdr) {
			/* Process Two descr per packet */
			nproc = MIN((nbound << 1), nproc);
		} else
#endif /* D11_SPLIT_RX_FD */
		{
			nproc = MIN(nproc, nbound);
		}
		curr =  RXD(i + nproc);
	}

	head0 = *head;
	tail0 = NULL;
	while (i != curr) {
#if defined(D11_SPLIT_RX_FD)
		if (di->sep_rxhdr) {
			dma64addr_t pa64 = { .low_addr = 0, .high_addr = 0 };
			PHYSADDR64HISET(pa64, (BUS_SWAP32(R_SM(&di->rxd64[i].addrhigh)) -
				di->dataoffsethigh));
			if ((PHYSADDR64HI(pa64) & PCI64ADDR_HIGH) != 0) {
				i = NEXTRXD(i);
				continue;
			}
		}
#endif /* D11_SPLIT_RX_FD */
		tail0 = *head;
		*head = PKTLINK(*head);
#ifndef DONGLEBUILD
		PHYSADDRLOSET(pa, (BUS_SWAP32(R_SM(&di->rxd64[i].addrlow)) -
			di->dataoffsetlow));
		PHYSADDRHISET(pa, (BUS_SWAP32(R_SM(&di->rxd64[i].addrhigh)) -
			di->dataoffsethigh));
		/* clear this packet from the descriptor ring */
#if !defined(OSL_CACHE_COHERENT)
#ifdef BCM_SECURE_DMA
		SECURE_DMA_UNMAP(di->osh, pa, di->rxbufsize, DMA_RX, NULL, NULL,
			&di->sec_cma_info_rx, 0);
#else
		DMA_UNMAP(di->osh, pa, di->rxbufsize, DMA_RX, tail0, &di->rxp_dmah[i]);
#endif /* BCM_SECURE_DMA */
#endif  /* ! OSL_CACHE_COHERENT */

#endif /* ! DONGLEBUILD */
		/* NEXTRXD */
		i = NEXTRXD(i);
	}

	if (tail0) {
		PKTSETLINK(tail0, NULL);
		di->rxin = i;
		di->rxavail = di->nrxd - NRXDACTIVE(i, di->rxout) - 1;

#if defined(D11_SPLIT_RX_FD)
		if (di->split_fifo) {
			di->linked_di->rxin = i;
			di->linked_di->rxavail = di->rxavail;
		}
#endif /* D11_SPLIT_RX_FD */

		if (*head == NULL) {
			*tail = NULL;
		}
		return head0;
	}

	return (NULL);
}
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
#if !defined(BCM_SECURE_DMA)
#if !defined(OSL_CACHE_COHERENT)
		pa = DMA_MAP(di->osh, PKTDATA(di->osh, p),
			di->rxbufsize, DMA_RX, p,
			&di->rxp_dmah[rxout1]);
#else
		OSL_VIRT_TO_PHYSADDR(di->osh, PKTDATA(di->osh, p), pa);
#endif
#endif /* #if !defined(BCM_SECURE_DMA) */

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
	uint16 rxin, rxin_next;
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
	di->rs0cd = DMA64_RX_CD(di);

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

#endif /* BCM_PCAP */
