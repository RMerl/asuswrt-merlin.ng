/*
 * Generic Broadcom Home Networking Division (HND) DMA receive routines.
 * This supports the following chips: BCM42xx, 44xx, 47xx .
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
 *
 * <<Broadcom-WL-IPTag/Open:>>
 *
 * $Id: hnddma_rx.c 779929 2019-10-10 07:50:13Z $
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
#include <osl.h>
#include <bcmendian.h>
#include <hndsoc.h>
#include <bcmutils.h>
#include <siutils.h>

#ifdef HNDCTF
#include <ctf/hndctf.h>
#endif // endif

#include <sbhnddma.h>
#include <hnddma.h>
#include "hnddma_priv.h"
#ifdef SRMEM
#include <hndsrmem.h>
#endif /* SRMEM */

#ifdef STS_FIFO_RXEN
#include <hndd11.h>
#endif // endif

#if !defined(OSL_CACHE_COHERENT)
/* XXX JIRA: SWWLAN-23796: Double invalidation ACP WAR Cortex-A9
 * ARM HW prefetcher is enabled and can prefetch, so need to invalidate on Rx
 */
#if defined(BCM47XX_CA9) || defined(STB) || defined(STB_SOC_WIFI) || \
	defined(BCA_HNDROUTER)
#define OSL_CACHE_INV_RX
#endif // endif
#endif  /* ! OSL_CACHE_COHERENT */

/* Packets allocated from BPM have their data buffers pre-cache-invalidated */
#if defined(BCM_NBUFF_PKT) && (defined(CONFIG_BCM_BPM) || \
	defined(CONFIG_BCM_BPM_MODULE))
#define BCM_NBUFF_PKT_BPM /* Packet data buffers, are from BPM pool */
#ifndef BCMDBG
#define BCM_NBUFF_PKT_BPM_COHERENCY /* BPM buffers are cache invalidated */
#endif // endif
#endif /* BCM_NBUFF_PKT && CONFIG_BCM_BPM */

#if defined(D11_SPLIT_RX_FD)
static bool dma_splitrxfill(dma_info_t *di);
#endif // endif

#ifdef BULKRX_PKTLIST
#ifdef STS_FIFO_RXEN
static void *_dma_getnext_sts_rxlist(dma_info_t *di, bool forceall);
static void _dma_sts_rx(dma_info_t *di, rx_list_t *rx_sts_list);
#endif // endif
static void *_dma_getnext_rxlist(dma_info_t *di, uint nbound, bool forceall);
#endif /* BULKRX_PKTLIST */

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

/**
 * !! rx entry routine
 * returns a pointer to the list of next frame/frames received, or NULL if there are no more
 * DMA_CTRL_RXMULTI and DMA scattering(multiple buffers) is not supported yet
 */
#ifdef BULKRX_PKTLIST
#ifdef STS_FIFO_RXEN

/* BULKRX_PKTLIST && STS_FIFO_RXEN specific function */
bool BCMFASTPATH
dma_sts_rxfill(hnddma_t *dmah)
{
	dma_info_t *di = DI_INFO(dmah);
	sts_buff_t *p;
	uint16 rxin, rxout;
	uint16 rxactive;
	uint32 flags = 0;
	uint n;
	uint i;
	dmaaddr_t pa;
#ifdef BCM_SECURE_DMA
#ifdef BCMDMA64OSL
	dma_addr_t paddr;
#endif /* BCMDMA64OSL */
#endif /* BCM_SECURE_DMA */

	bool ring_empty;
	sts_buff_t *tail = NULL;
	uint dpp = 1; /* dpp - descriptors per packet */

	if (di->rxfill_suspended)
		return FALSE;
	/*
	 * Determine how many receive buffers we're lacking
	 * from the full complement, allocate, initialize,
	 * and post them, then update the chip rx lastdscr.
	 */

	rxin = di->rxin;
	rxout = di->rxout;
	rxactive = NRXDACTIVE(rxin, rxout);

	/* if currently posted is more than requested, no need to do anything */
	if (MIN((di->nrxd/dpp), di->nrxpost) < CEIL(rxactive, dpp))
		return TRUE;

	/* No. of packets to post is : n = MIN(nrxd, nrxpost) - NRXDACTIVE;
	 * But, if (dpp/descriptors per packet > 1), we need this adjustment.
	 */
	n = MIN((di->nrxd/dpp), di->nrxpost) - CEIL(rxactive, dpp);
	DMA_TRACE(("%s: dma_sts_rxfill: post %d\n", di->name, n));

	tail = di->dma_rx_pkt_list.tail_pkt;

	for (i = 0; i < n; i++) {
		int dlen = 0;
		p =  STSBUF_ALLOC(di->sts_mempool);
		if (p == NULL) {
			DMA_TRACE(("%s: dma_sts_rxfill: out of rxbufs\n", di->name));
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

		*(uint32 *)(STSBUF_DATA(p)) = 0;
#ifdef BCM_SECURE_DMA
#ifdef BCMDMA64OSL
		paddr = SECURE_DMA_MAP(di->osh, STSBUF_DATA(p), di->rxbufsize, DMA_RX,
			NULL, NULL, &di->sec_cma_info_rx, 0, SECDMA_TXBUF_POST);
		ULONGTOPHYSADDR(paddr, pa);
#else
		pa = SECURE_DMA_MAP(di->osh, STSBUF_DATA(p), di->rxbufsize, DMA_RX,
			NULL, NULL, &di->sec_cma_info_rx, 0, SECDMA_TXBUF_POST);
#endif /* BCMDMA64OSL */
#else
#if !defined(OSL_CACHE_COHERENT)
		pa = DMA_MAP(di->osh, STSBUF_DATA(p), di->rxbufsize, DMA_RX, NULL, NULL);
#else
		pa = (dmaaddr_t)VIRT_TO_PHYS(STSBUF_DATA(p));
#endif  /* ! OSL_CACHE_COHERENT */
#endif  /* ! BCM_SECURE_DMA */

		/* Status buffers are always allocated with 8 byte alignemnt in place */
		ASSERT(ISALIGNED(PHYSADDRLO(pa), 8));

		STSBUF_SETDMAIDX(p, rxout);
		STSBUF_SETLINK(p, NULL);
		if (tail) {
			STSBUF_SETLINK(tail, p);
			di->dma_rx_pkt_list.tail_pkt = tail = p;
		} else {
			ASSERT((di->dma_rx_pkt_list.head_pkt == NULL));
			di->dma_rx_pkt_list.head_pkt = tail = p;
			di->dma_rx_pkt_list.tail_pkt = tail;
		}

		/* reset flags for each descriptor */
		flags = D64_CTRL1_SOF;
		dlen = di->rxbufsize;

		if (rxout == (di->nrxd - 1))
			flags |= D64_CTRL1_EOT;

		dma64_dd_upd(di, di->rxd64, pa, rxout, &flags,
				dlen);
		rxout = NEXTRXD(rxout);
	}

#if !defined(BULK_DESCR_FLUSH)
	di->rxout = rxout;
#endif // endif

	/* update the chip lastdscr pointer */
#if !defined(OSL_CACHE_COHERENT)
#if defined(BULK_DESCR_FLUSH)
	{
		uint32 flush_cnt = NRXDACTIVE(di->rxout, rxout);
		if (rxout < di->rxout) {
			DMA_MAP(di->osh, dma64_rxd64(di, 0), DMA64_FLUSH_LEN(rxout),
			        DMA_TX, NULL, NULL);
			flush_cnt -= rxout;
		}
		DMA_MAP(di->osh, dma64_rxd64(di, di->rxout), DMA64_FLUSH_LEN(flush_cnt),
		        DMA_TX, NULL, NULL);
	}
#endif /* BULK_DESCR_FLUSH */
#endif /* ! OSL_CACHE_COHERENT */

	W_REG(di->osh, &di->d64rxregs->ptr, (uint32)(di->rcvptrbase + I2B(rxout, dma64dd_t)));

#if defined(BULK_DESCR_FLUSH)
	di->rxout = rxout;
#endif // endif
	return !ring_empty;

} /* dma_sts_rxfill */

/**
 * !! sts rx entry routine
 * returns a pointer to the list of next frame/frames received, or NULL if there are no more
 * BULKRX_PKTLIST && STS_FIFO_RXEN specific function.
 */
static void BCMFASTPATH
_dma_sts_rx(dma_info_t *di, rx_list_t *rx_sts_list)
{
	sts_buff_t *head, *head0;
	uint len;
	int resid = 0;
	void *data;
	sts_buff_t *prev = NULL, *free_p;
	BCM_REFERENCE(prev);
	BCM_REFERENCE(free_p);

	head0 = head = _dma_getnext_sts_rxlist(di, FALSE);

	while (head != NULL) {
		data = STSBUF_DATA(head);

#if (!defined(__mips__) && !(defined(BCM47XX_CA9) || defined(STB) || \
	defined(BCA_HNDROUTER)))
		len = ltoh16(*(uint16 *)(data));
#else  /* !__mips__ && !BCM47XX_CA9 */
		{
			int read_count = 0;
			/* PR77137 DMA(Bug) on some chips seems to declare that the
			 * packet is ready, but the packet length is not updated
			 * (by DMA) by the time we are here
			 */
			for (read_count = 200; read_count; read_count--) {
				len = ltoh16(*(uint16 *)(data));
				if (len != 0)
					break;
#if !defined(OSL_CACHE_COHERENT)
				DMA_MAP(di->osh, data, sizeof(uint16), DMA_RX, NULL, NULL);
#endif // endif
				OSL_DELAY(1);
			}

			if (!len) {
				DMA_ERROR(("%s: dma_rx: frame length (%d)\n", di->name, len));
				free_p = head;
				if (prev != NULL) {
					head = STSBUF_LINK(free_p);
					STSBUF_SETLINK(prev, head);
				} else {
					head0 = head = STSBUF_LINK(free_p);
				}
				STSBUF_SETLINK(free_p, NULL);
				STSBUF_FREE(free_p, di->sts_mempool);
				di->hnddma.rxgiants++;
				continue;
			}

		}
#endif /* !__mips__ && !BCM47XX_CA9 */
		DMA_TRACE(("%s: dma_rx len %d\n", di->name, len));
		/* set actual length */
		resid = len - (di->rxbufsize);

		if (resid <= 0) {
			/* Single frame, all good */
		} else {
			DMA_ERROR(("%s: dma_rx bad sts len %d\n", di->name, len));
			ASSERT(0);
		}

#if defined(OSL_CACHE_INV_RX)
		DMA_MAP(di->osh, STSBUF_DATA(head), STSBUF_LEN(head), DMA_RX, head, NULL);
#endif // endif
		rx_sts_list->rxfifocnt++;
		prev = head;
		head = STSBUF_LINK(head);
	} /* End While() */

	rx_sts_list->rx_head = head0;
	rx_sts_list->rx_tail = prev;
}

/** BULKRX_PKTLIST && STS_FIFO_RXEN specific function */
void BCMFASTPATH
dma_sts_rx(hnddma_t *dmah, rx_list_t *rx_sts_list)
{
	dma_info_t *di = DI_INFO(dmah);

	_dma_sts_rx(di, rx_sts_list);
}

#endif /* STS_FIFO_RXEN */

/**
 * BULKRX_PKTLIST specific function
 * @param[inout] rx_list  Linked list of mpdus that were received
 */
void BCMFASTPATH
dma_rx(hnddma_t *dmah, rx_list_t *rx_list, rx_list_t *rx_sts_list, uint nbound)
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
#endif // endif
		rxoffset = di->rxoffset;

	while (head != NULL) {
		data = PKTDATA(di->osh, head);

#if (!defined(__mips__) && !(defined(BCM47XX_CA9) || defined(STB) || \
	defined(BCA_HNDROUTER)))
		len = ltoh16(*(uint16 *)(data));
#else  /* !__mips__ && !BCM47XX_CA9 */
		{
			int read_count = 0;
			/* PR77137 DMA(Bug) on some chips seems to declare that the
			 * packet is ready, but the packet length is not updated
			 * (by DMA) by the time we are here
			 */
			for (read_count = 200; read_count; read_count--) {
				len = ltoh16(*(uint16 *)(data));
				if (len != 0)
					break;
#if defined(BCM_GMAC3)
				if (PKTISFWDERBUF(di->osh, head)) {
					OSL_CACHE_INV_LINE(data);
				} else
#endif /* BCM_GMAC3 */
				{
#if !defined(OSL_CACHE_COHERENT)
					DMA_MAP(di->osh, data, sizeof(uint16), DMA_RX, NULL, NULL);
#endif // endif
				}
				OSL_DELAY(1);
			}

			if (!len) {
				DMA_ERROR(("%s: dma_rx: frame length (%d)\n", di->name, len));
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
			DMA_ERROR(("%s: dma_rx: corrupted length (%d)\n", di->name, len));
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

		} else {
			/* multi-buffer rx */
			int num_dd = 0; /* Number of DMA descriptors used for this packet. */
			void *tail, *p;
			num_dd = ((ltoh32(*(uint32 *)(PKTDATA(di->osh, head))) &
				D64_RX_FRM_STS_DSCRCNT) >> D64_RX_FRM_STS_DSCRCNT_SHIFT);

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
				/* For split rx case LCL packet length should
				 * not be more than tcm_maxsize.
				 * XXX: multi buffer case is not handled
				 */
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
				uint16 cur;
				ASSERT(p == NULL);

				cur = B2I(((R_REG(di->osh, &di->d64rxregs->status0) &
					D64_RS0_CD_MASK) - di->rcvptrbase) & D64_RS0_CD_MASK,
					dma64dd_t);

				DMA_ERROR(("_dma_rx, rxin %d rxout %d, hw_curr %d\n",
					di->rxin, di->rxout, cur));
			}
#endif /* BCMDBG */
			if ((di->hnddma.dmactrlflags & DMA_CTRL_RXMULTI) == 0) {
				DMA_ERROR(("%s: dma_rx: bad frame length (%d)\n", di->name, len));

#if defined(BCMPCIEDEV) && (defined(STS_FIFO_RXEN) || defined(WLC_OFFLOADS_RXSTS))
				if (BCMPCIEDEV_ENAB()) {
					/* XXX: RB:157255 - Drop rx frames exceeding the len of
					 * rxbufsize after processing corresponding phyrxstatus
					 * in FD mode
					 */
					di->hnddma.rxgiants++;
				} else
#endif /* BCMPCIEDEV && (STS_FIFO_RXEN || WLC_OFFLOADS_RXSTS) */
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
					di->hnddma.rxgiants++;
					continue;
				}
			}
		}
		rx_list->rxfifocnt++;
		prev = head;
		head = PKTLINK(head);
	} /* End While() */

#ifdef STS_FIFO_RXEN
	if (di->sts_di && rx_sts_list) {
		_dma_sts_rx(di->sts_di, rx_sts_list);
	}
#endif // endif
	rx_list->rx_head = head0;
	rx_list->rx_tail = prev;
} /* dma_rx */

#else /* BULKRX_PKTLIST */

/**
 * !! rx entry routine
 * returns a pointer to the next frame received, or NULL if there are no more
 *   if DMA_CTRL_RXMULTI is defined, DMA scattering(multiple buffers) is supported
 *      with pkts chain
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

	if (di->split_fifo == SPLIT_FIFO_0) {
		/* Fifo-0 handles all host address */
		/* Below PKT ops are not valid for host pkts */
		goto ret;
	}

	data = PKTDATA(di->osh, head);

#if (!defined(__mips__) && !(defined(BCM47XX_CA9) || defined(STB) || \
	defined(BCA_HNDROUTER)))
	if (di->hnddma.dmactrlflags & DMA_CTRL_SDIO_RXGLOM) {
		/* In case of glommed pkt get length from hwheader */
		len = ltoh16(*((uint16 *)(data) + di->rxoffset/2 + 2)) + 4;

		*(uint16 *)(data) = (uint16)len;
	} else {
		len = ltoh16(*(uint16 *)(data));
	}
#else  /* !__mips__ && !BCM47XX_CA9 */
	{
	int read_count = 0;
	/* PR77137 DMA(Bug) on some chips seems to declare that the
	 * packet is ready, but the packet length is not updated
	 * (by DMA) by the time we are here
	 */
	for (read_count = 200; read_count; read_count--) {
		len = ltoh16(*(uint16 *)(data));
		if (len != 0)
			break;
#if defined(BCM_GMAC3)
		if (PKTISFWDERBUF(di->osh, head)) {
			OSL_CACHE_INV_LINE(data);
		} else
#endif /* BCM_GMAC3 */
		{
#if !defined(OSL_CACHE_COHERENT)
			DMA_MAP(di->osh, data, sizeof(uint16), DMA_RX, NULL, NULL);
#endif // endif
		}
		OSL_DELAY(1);
	}

	if (!len) {
		DMA_ERROR(("%s: dma_rx: frame length (%d)\n", di->name, len));
		PKTFREE(di->osh, head, FALSE);
		goto next_frame;
	}

	}
#endif /* !__mips__ && !BCM47XX_CA9 */
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
		DMA_TRACE(("%s: dma_rx: corrupted length (%d)\n", di->name, len));
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
				/* For split rx case LCL packet length should
				 * not be more than tcm_maxsize.
				 * XXX: multi buffer case is not handled
				 */
				if (di->sep_rxhdr)
					PKTSETLEN(di->osh, p, MIN(pkt_len, tcm_maxsize));
				else
#endif /* D11_SPLIT_RX_FD */
					PKTSETLEN(di->osh, p, pkt_len);

				tail = p;
				resid -= di->rxbufsize;
			}
		} else {
			int num_dd = 0; /* Number of DMA descriptors used for this packet. */
			num_dd = ((ltoh32(*(uint32 *)(PKTDATA(di->osh, head))) &
					D64_RX_FRM_STS_DSCRCNT) >> D64_RX_FRM_STS_DSCRCNT_SHIFT);

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
				/* For split rx case LCL packet length should
				 * not be more than tcm_maxsize.
				 * XXX: multi buffer case is not handled
				 */
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
			uint16 cur;
			ASSERT(p == NULL);

			cur = B2I(((R_REG(di->osh, &di->d64rxregs->status0) & D64_RS0_CD_MASK) -
				di->rcvptrbase) & D64_RS0_CD_MASK, dma64dd_t);

			DMA_ERROR(("_dma_rx, rxin %d rxout %d, hw_curr %d\n",
				di->rxin, di->rxout, cur));
		}
#endif /* BCMDBG */

		if ((di->hnddma.dmactrlflags & DMA_CTRL_RXMULTI) == 0) {
			DMA_ERROR(("%s: dma_rx: bad frame length (%d)\n", di->name, len));
			PKTFREE(di->osh, head, FALSE);
			di->hnddma.rxgiants++;
			goto next_frame;
		}
	}

ret:
	return (head);
}
#endif /* BULKRX_PKTLIST */

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
#ifdef BCM_SECURE_DMA
#ifdef BCMDMA64OSL
	dma_addr_t paddr;
#endif /* BCMDMA64OSL */
#endif /* BCM_SECURE_DMA */
	uint extra_offset = 0, extra_pad = 0;
	bool ring_empty;
#ifdef BULKRX_PKTLIST
	void *tail = NULL;
#endif // endif
	uint alignment_req = (di->hnddma.dmactrlflags & DMA_CTRL_USB_BOUNDRY4KB_WAR) ?
				16 : 1;	/* MUST BE POWER of 2 */

	/* if sep_rxhdr is enabled, for every pkt, two descriptors are programmed */
	/* NRXDACTIVE(rxin, rxout) would show 2 times no of actual full pkts */
	uint dpp = (di->sep_rxhdr) ? 2 : 1; /* dpp - descriptors per packet */

	if (DMA_CTRL_IS_HWA_RX(di))
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
	if (MIN((di->nrxd/dpp), di->nrxpost) < CEIL(rxactive, dpp))
		return TRUE;

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
#endif // endif
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
#if defined(BCM_NBUFF_PKT_BPM) && defined(CC_BPM_SKB_POOL_BUILD)
			p = PKTPOOLGET(di->osh, len); /* Use BPM packets and data buffers */
#else
			p = PKTGET(di->osh, len, FALSE);
#endif // endif
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

#ifdef CTFMAP
		/* mark as ctf buffer for fast mapping */
		if (CTF_ENAB(kcih)) {
			/* XXX At this point we don't know how much header room is
			 * reserved for the user (wl in particular) so we can't
			 * assert here.
			ASSERT((((uint32)PKTDATA(di->osh, p)) & 31) == 0);
			*/
			PKTSETCTF(di->osh, p);
		}
#endif /* CTFMAP */

		*(uint32 *)(PKTDATA(di->osh, p)) = 0;

#if defined(linux) && (defined(BCM47XX_CA9) || defined(__mips__)) && defined(BCM_GMAC3)
		/* Packets tagged as FWDER_BUF are ensured to only have FWDER_PKTMAPSZ
		 * (at most) in the cache, with the first 2Bytes dirty in cache.
		 * FWDER_BUF tagged packets are placed in NON ACP address space. Non
		 * FWDER_BUF packets are placed into ACP region and avail of HW cache
		 * coherency via ACP.
		 */
		if (PKTISFWDERBUF(di->osh, p)) {
			void *va = PKTDATA(di->osh, p);
			pa = virt_to_phys_noacp(va); /* map to non acp address space */
			OSL_CACHE_FLUSH_LINE(va); /* 1 cache line */
		} else {
#if !defined(OSL_CACHE_COHERENT)
			/* cache flush first 4Byte length */
			DMA_MAP(di->osh, PKTDATA(di->osh, p), sizeof(uint32), DMA_TX, NULL, NULL);
			/* cache invalidate entire packet buffer */
			pa = DMA_MAP(di->osh, PKTDATA(di->osh, p), di->rxbufsize, DMA_RX, p, NULL);
#else
			pa = (dmaaddr_t)VIRT_TO_PHYS(PKTDATA(di->osh, p));
#endif // endif
		}

#else /* !BCM_GMAC3 based FWDER_BUF optimization */

#if !defined(OSL_CACHE_COHERENT)

#if defined(linux) && (defined(BCM47XX_CA9) || defined(__mips__) || \
	defined(BCA_HNDROUTER))
		DMA_MAP(di->osh, PKTDATA(di->osh, p), sizeof(uint32), DMA_TX, NULL, NULL);
#endif // endif

#if defined(SGLIST_RX_SUPPORT)
		if (DMASGLIST_ENAB)
			bzero(&di->rxp_dmah[rxout], sizeof(hnddma_seg_map_t));

#ifdef BCM_SECURE_DMA
#ifdef BCMDMA64OSL
		paddr = SECURE_DMA_MAP(di->osh, PKTDATA(di->osh, p), di->rxbufsize, DMA_RX,
			NULL, NULL, &di->sec_cma_info_rx, 0, SECDMA_RXBUF_POST);
		ULONGTOPHYSADDR(paddr, pa);
#else
		pa = SECURE_DMA_MAP(di->osh, PKTDATA(di->osh, p), di->rxbufsize, DMA_RX,
			NULL, NULL, &di->sec_cma_info_rx, 0, SECDMA_RXBUF_POST);
#endif /* BCMDMA64OSL */
#else
		pa = DMA_MAP(di->osh, PKTDATA(di->osh, p), di->rxbufsize, DMA_RX, p,
			&di->rxp_dmah[rxout]);
#endif /* ! BCM_SECURE_DMA */

#else  /* ! SGLIST_RX_SUPPORT */

#ifdef BCM_SECURE_DMA
#ifdef BCMDMA64OSL
		paddr = SECURE_DMA_MAP(di->osh, PKTDATA(di->osh, p), di->rxbufsize, DMA_RX,
			NULL, NULL, &di->sec_cma_info_rx, 0, SECDMA_TXBUF_POST);
		ULONGTOPHYSADDR(paddr, pa);
#else
		pa = SECURE_DMA_MAP(di->osh, PKTDATA(di->osh, p), di->rxbufsize, DMA_RX,
			NULL, NULL, &di->sec_cma_info_rx, 0, SECDMA_TXBUF_POST);
#endif /* BCMDMA64OSL */
#else

#if defined(BCM_NBUFF_PKT_BPM)
		pa = (dmaaddr_t)VIRT_TO_PHYS(PKTDATA(di->osh, p));
#else
		pa = DMA_MAP(di->osh, PKTDATA(di->osh, p),
		             di->rxbufsize, DMA_RX, p, NULL);
#endif /* ! BCM_NBUFF_PKT_BPM */

#endif /* ! BCM_SECURE_DMA */
#endif /* ! SGLIST_RX_SUPPORT */

#else  /* OSL_CACHE_COHERENT */
		/* Do not cache flush or invalidate on HW cache coherent systems */
		pa = (dmaaddr_t)VIRT_TO_PHYS(PKTDATA(di->osh, p));
#endif /* ! OSL_CACHE_COHERENT */

#endif /* !(linux && (BCM47XX_CA9 || __mips__) && BCM_GMAC3) */

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
#endif // endif
		{
			/* save the free packet pointer */
			ASSERT(di->rxp[rxout] == NULL);
			di->rxp[rxout] = p;
		}

#if defined(DONGLEBUILD) && defined(SRMEM)
		if (SRMEM_ENAB()) {
			PKTSRMEM_DEC_INUSE(p);
		}
#endif /* DONGLEBUILD && SRMEM */

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
			pa64.loaddr = PKTFRAGDATA_LO(di->osh, p, 1);
			pa64.hiaddr = PKTFRAGDATA_HI(di->osh, p, 1);

			/* Extract out host length */
			dlen = pktlen = PKTFRAGLEN(di->osh, p, 1);
			ds_len = (di->d11rx_war) ? D11RX_WAR_MAX_BUF_SIZE : pktlen;

			do {
				uint64 addr = 0;
				/* Mark up this descriptor that its a host descriptor
				* store 0x80000000 so that dma_rx need'nt
				* process this descriptor
				*/
#ifndef BULKRX_PKTLIST
				di->rxp[rxout] = (void*)PCI64ADDR_HIGH;
#endif // endif
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
#endif // endif

	/* update the chip lastdscr pointer */

#if !defined(OSL_CACHE_COHERENT)
#if defined(BULK_DESCR_FLUSH)
	{
		uint32 flush_cnt = NRXDACTIVE(di->rxout, rxout);
		if (rxout < di->rxout) {
			DMA_MAP(di->osh, dma64_rxd64(di, 0), DMA64_FLUSH_LEN(rxout),
			        DMA_TX, NULL, NULL);
			flush_cnt -= rxout;
		}
		DMA_MAP(di->osh, dma64_rxd64(di, di->rxout), DMA64_FLUSH_LEN(flush_cnt),
		        DMA_TX, NULL, NULL);
	}
#endif /* BULK_DESCR_FLUSH */
#endif /* ! OSL_CACHE_COHERENT */

	W_REG(di->osh, &di->d64rxregs->ptr, (uint32)(di->rcvptrbase + I2B(rxout, dma64dd_t)));

#if defined(BULK_DESCR_FLUSH)
	di->rxout = rxout;
#endif // endif
	return !ring_empty;
} /* _dma_rxfill */

#ifdef STS_FIFO_RXEN
void  BCMFASTPATH
dma_sts_rxreclaim(hnddma_t *dmah)
{
	dma_info_t *di = DI_INFO(dmah);
	sts_buff_t *p;
	sts_buff_t *next = NULL;

	DMA_TRACE(("%s: dma_sts_rxreclaim\n", di->name));
	p = _dma_getnext_sts_rxlist(di, TRUE);
	while (p != NULL) {
		next = STSBUF_LINK(p);
		STSBUF_SETLINK(p, NULL);
		STSBUF_FREE(p, di->sts_mempool);
		p = next;
	}
}
#endif /* STS_FIFO_RXEN */

/** Called when e.g. flushing d11 packets */
void  BCMFASTPATH
dma_rxreclaim(hnddma_t *dmah)
{
	dma_info_t *di = DI_INFO(dmah);

	void *p;
#ifdef BCMPKTPOOL
	bool origcb = TRUE;
#endif /* BCMPKTPOOL */
#ifdef BULKRX_PKTLIST
	void *next = NULL;
#endif // endif
	DMA_TRACE(("%s: dma_rxreclaim\n", di->name));

#ifdef BCMPKTPOOL
	if (POOL_ENAB(di->pktpool) &&
	    ((origcb = pktpool_emptycb_disabled(di->pktpool)) == FALSE))
		pktpool_emptycb_disable(di->pktpool, TRUE);
#endif /* BCMPKTPOOL */

	/* For split fifo, this function expects fifo-1 di to proceed further. */
	/* fifo-0 requests can be discarded since fifo-1 will reclaim both fifos */
	if (di->split_fifo == SPLIT_FIFO_0) {
		return;
	}

#ifdef BULKRX_PKTLIST
	if (DMA_BULKRX_PATH(di)) {
		p = _dma_getnext_rxlist(di, 0, TRUE);
	} else
#endif // endif
	p = dma_getnextrxp(dmah, TRUE);
	while (p != NULL) {
		/* For unframed data, we don't have any packets to free */
#if (defined(__mips__) || defined(BCM47XX_CA9) || defined(STB) || \
	defined(BCA_HNDROUTER)) && defined(linux)
		if (!(di->hnddma.dmactrlflags & DMA_CTRL_UNFRAMED))
#endif /* (__mips__ || BCM47XX_CA9) && linux */
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
#endif // endif
			PKTFREE(di->osh, p, FALSE);
		}

#ifdef BULKRX_PKTLIST
		if (DMA_BULKRX_PATH(di)) {
			p = next;
		} else
#endif // endif
		{
			p = dma_getnextrxp(dmah, TRUE);
		}
	}

#ifdef BCMPKTPOOL
	if (POOL_ENAB(di->pktpool) && origcb == FALSE)
		pktpool_emptycb_disable(di->pktpool, FALSE);
#endif /* BCMPKTPOOL */
} /* dma_rxreclaim */

/**
 * Initializes one tx or rx descriptor with the caller provided arguments, notably a buffer.
 * Knows how to handle native 64-bit addressing AND bit64 extension.
 *    @param[in] di       Handle
 *    @param[in] ddring   Pointer to properties of the descriptor ring
 *    @param[in] pa       Physical address of rx or tx buffer that the descriptor should point at
 *    @param[in] outidx   Index of the targetted DMA descriptor within the descriptor ring
 *    @param[in] flags    Value that is written into the descriptors 'ctrl1' field
 *    @param[in] bufcount Buffer count value, written into the descriptors 'ctrl2' field
 */
void BCMFASTPATH
dma64_dd_upd(dma_info_t *di, dma64dd_t *ddring, dmaaddr_t pa, uint outidx, uint32 *flags,
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
		W_SM(&ddring[outidx].ctrl1, BUS_SWAP32(*flags));
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
		W_SM(&ddring[outidx].ctrl1, BUS_SWAP32(*flags));
		W_SM(&ddring[outidx].ctrl2, BUS_SWAP32(ctrl2));
	}

	if (di->hnddma.dmactrlflags & DMA_CTRL_PEN) {
		if (DMA64_DD_PARITY(&ddring[outidx])) {
			W_SM(&ddring[outidx].ctrl2, BUS_SWAP32(ctrl2 | D64_CTRL2_PARITY));
		}
	}

#if !defined(OSL_CACHE_COHERENT)
#if (defined(BCM47XX_CA9) || ((defined(STB) || defined(STBAP)) && \
	!defined(DHD_USE_COHERENT_MEM_FOR_RING)) || defined(BCA_HNDROUTER)) && \
	!defined(BULK_DESCR_FLUSH)
	DMA_MAP(di->osh, (void *)(((unsigned long long)(&ddring[outidx])) & ~0x1f), 32,
		DMA_TX, NULL, NULL);
#endif /* BCM47XX_CA9 && !BULK_DESCR_FLUSH */
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
#if (!defined(__mips__) && !(defined(BCM47XX_CA9) || defined(STB) || \
	defined(BCA_HNDROUTER))) || defined(BCM_SECURE_DMA)
	dmaaddr_t pa;
#endif // endif
#ifdef BCM_SECURE_DMA
#ifdef BCMDMA64OSL
	dma_addr_t paddr;
#endif /* BCMDMA64OSL */
#endif /* BCM_SECURE_DMA */
	if ((di->nrxd == 0) || DMA_CTRL_IS_HWA_RX(di))
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
		curr = (uint16)B2I(((R_REG(di->osh, &di->d64rxregs->status0) & D64_RS0_CD_MASK) -
		di->rcvptrbase) & D64_RS0_CD_MASK, dma64dd_t);
		di->rs0cd = curr;
	} else
		curr = di->rs0cd;

	/* ignore curr if forceall */
	if (!forceall && (i == curr))
		return (NULL);

	/* get the packet pointer that corresponds to the rx descriptor */
	rxp = di->rxp[i];
	ASSERT(rxp);

#if (defined(__mips__) || defined(BCM47XX_CA9) || defined(STB)) && !defined(_CFE_)

	if (!(di->hnddma.dmactrlflags & DMA_CTRL_UNFRAMED)) {
		/* Processor prefetch of 1 x 32B cacheline carrying HWRXOFF */
		uint8 * addr = PKTDATA(di->osh, rxp);
#if defined(BCM_GMAC3)
		if (PKTISFWDERBUF(di->osh, rxp)) {
			/* Double invalidation WAR for ACP based platforms - Cortex-A9 */
			OSL_CACHE_INV_LINE(addr);
		}
#endif // endif
		bcm_prefetch_32B(addr, 1);
	}
#endif /* (__mips__ || BCM47XX_CA9) && linux */

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

#if defined(DONGLEBUILD) && defined(SRMEM)
	if (SRMEM_ENAB()) {
		PKTSRMEM_INC_INUSE(rxp);
	}
#endif /* DONGLEBUILD && SRMEM */

#if defined(SGLIST_RX_SUPPORT)
	PHYSADDRLOSET(pa, (BUS_SWAP32(R_SM(&di->rxd64[i].addrlow)) - di->dataoffsetlow));
	PHYSADDRHISET(pa, (BUS_SWAP32(R_SM(&di->rxd64[i].addrhigh)) - di->dataoffsethigh));

	/* clear this packet from the descriptor ring */
#if !defined(OSL_CACHE_COHERENT)
#ifdef BCM_SECURE_DMA
#ifdef BCMDMA64OSL
	PHYSADDRTOULONG(pa, paddr);
	SECURE_DMA_UNMAP(di->osh, paddr, di->rxbufsize, DMA_RX, NULL, NULL,
		&di->sec_cma_info_rx, 0);
#else
	SECURE_DMA_UNMAP(di->osh, pa, di->rxbufsize, DMA_RX, NULL, NULL,
		&di->sec_cma_info_rx, 0);
#endif /* BCMDMA64OSL */
#else
	DMA_UNMAP(di->osh, pa, di->rxbufsize, DMA_RX, rxp, &di->rxp_dmah[i]);
#endif /* BCM_SECURE_DMA */
#endif /* OSL_CACHE_COHERENT */
#endif /* SGLIST_RX_SUPPORT */

#if (defined(DESCR_DEADBEEF) || defined(STB))
	W_SM(&di->rxd64[i].addrlow, 0xdeadbeef);
	W_SM(&di->rxd64[i].addrhigh, 0xdeadbeef);
#endif /* DESCR_DEADBEEF */

	di->rxin = NEXTRXD(i);

	di->rxavail = di->nrxd - NRXDACTIVE(di->rxin, di->rxout) - 1;

	return (rxp);
}

#ifdef BULKRX_PKTLIST

#ifdef STS_FIFO_RXEN
/** returns list of entries on the ring, in the order in which they were placed on the ring */
static void * BCMFASTPATH
_dma_getnext_sts_rxlist(dma_info_t *di, bool forceall)
{
	uint16 i, curr, dpp;
	sts_buff_t **head, **tail, *head0, *tail0;
	dmaaddr_t pa;
#ifdef BCM_SECURE_DMA
#ifdef BCMDMA64OSL
	dma_addr_t paddr;
#endif /* BCMDMA64OSL */
#endif /* BCM_SECURE_DMA */
	if (di->nrxd == 0)
		return (NULL);

#ifdef BCM_BACKPLANE_TIMEOUT
	if (!si_deviceremoved(di->sih))
	{
		/* if forcing, dma engine must be disabled */
		ASSERT(!forceall || !dma64_rxenabled(di));
	}
#endif /* BCM_BACKPLANE_TIMEOUT */

	head = (sts_buff_t **)&di->dma_rx_pkt_list.head_pkt;
	tail = (sts_buff_t **)&di->dma_rx_pkt_list.tail_pkt;

	dpp = 1; /* dpp - descriptors per packet */
	i = di->rxin;
	if (forceall) {
		curr = di->rxout;
	} else {
		/* Number of DMA descr to process */
		if (i == di->rs0cd) {
			curr = (uint16)B2I(((R_REG(di->osh,
				&di->d64rxregs->status0) & D64_RS0_CD_MASK) -
				di->rcvptrbase) & D64_RS0_CD_MASK, dma64dd_t);
			di->rs0cd = curr;
		} else {
			curr = di->rs0cd;
		}
	}
	head0 = *head;
	tail0 = NULL;
	while (i != curr) {

		tail0 = *head;
		ASSERT((STSBUF_DMAIDX(tail0) == i));

		*head = STSBUF_LINK(*head);

		PHYSADDRLOSET(pa, (BUS_SWAP32(R_SM(&di->rxd64[i].addrlow)) - di->dataoffsetlow));
		PHYSADDRHISET(pa, (BUS_SWAP32(R_SM(&di->rxd64[i].addrhigh)) - di->dataoffsethigh));

		/* clear this packet from the descriptor ring */
#if !defined(OSL_CACHE_COHERENT)
#ifdef BCM_SECURE_DMA
#ifdef BCMDMA64OSL
		PHYSADDRTOULONG(pa, paddr);
		SECURE_DMA_UNMAP(di->osh, paddr, di->rxbufsize, DMA_RX, NULL, NULL,
		    &di->sec_cma_info_rx, 0);
#else
		SECURE_DMA_UNMAP(di->osh, pa, di->rxbufsize, DMA_RX, NULL, NULL,
			&di->sec_cma_info_rx, 0);

#endif /* BCMDMA64OSL */
#else
		DMA_UNMAP(di->osh, pa, di->rxbufsize, DMA_RX, NULL, NULL);
#endif /* BCM_SECURE_DMA */
#endif /* OSL_CACHE_COHERENT */

		/* NEXTRXD */
		i = RXD(i + dpp);
	}

	if (tail0) {
		STSBUF_SETLINK(tail0, NULL);
		di->rxin = i;
		di->rxavail = di->nrxd - NRXDACTIVE(i, di->rxout) - 1;

		if (*head == NULL) {
			*tail = NULL;
		}
		return head0;
	}

	return (NULL);
}
#endif /* STS_FIFO_RXEN */

/** returns list of entries on the ring, in the order in which they were placed on the ring */
static void * BCMFASTPATH
_dma_getnext_rxlist(dma_info_t *di, uint nbound, bool forceall)
{
	uint16 i, curr, nproc = 0;
	void **head = NULL, **tail = NULL, *head0 = NULL, *tail0 = NULL;
	dmaaddr_t pa;
#ifdef BCM_SECURE_DMA
#ifdef BCMDMA64OSL
	dma_addr_t paddr;
#endif /* BCMDMA64OSL */
#endif /* BCM_SECURE_DMA */

	BCM_REFERENCE(pa);
	if ((di->nrxd == 0) || DMA_CTRL_IS_HWA_RX(di))
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
		/* Number of DMA descr to process */
		curr = (uint16)B2I(((R_REG(di->osh,
			&di->d64rxregs->status0) & D64_RS0_CD_MASK) -
			di->rcvptrbase) & D64_RS0_CD_MASK, dma64dd_t);
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
	/* XXX : avoid this traversal of the pkt list for cache coherent platforms
	   as we are only doing dma_unmap in the loop;
	   'i' access after the loop would then need to be taken care of
	*/
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
#ifdef BCMDMA64OSL
		PHYSADDRTOULONG(pa, paddr);
		SECURE_DMA_UNMAP(di->osh, paddr, di->rxbufsize, DMA_RX, NULL, NULL,
			&di->sec_cma_info_rx, 0);
#else
		SECURE_DMA_UNMAP(di->osh, pa, di->rxbufsize, DMA_RX, NULL, NULL,
			&di->sec_cma_info_rx, 0);
#endif /* BCMDMA64OSL */
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

		if (di->split_fifo) {
			di->linked_di->rxin = i;
			di->linked_di->rxavail = di->rxavail;
		}

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
#if (defined(BCA_HNDROUTER) && !defined(BULK_DESCR_FLUSH))
	DMA_MAP(di->osh, (void *)(((unsigned long)(&ddring[outidx])) & ~0x1f), 32, DMA_TX,
		NULL, NULL);
#endif // endif
#endif  /* ! OSL_CACHE_COHERENT */
}

#if defined(D11_SPLIT_RX_FD)
static bool BCMFASTPATH
dma_splitrxfill(dma_info_t *di)
{
	void *p = NULL;
#ifdef BULKRX_PKTLIST
	void *tail = NULL;
#endif // endif
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
#endif // endif
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
#if defined(linux) && (defined(BCM47XX_CA9) || defined(__mips__) || defined(STB) || \
	defined(BCA_HNDROUTER))
		DMA_MAP(di->osh, PKTDATA(di->osh, p), sizeof(uint16), DMA_TX, NULL, NULL);
#endif // endif
#endif  /* ! OSL_CACHE_COHERENT */

		if (DMASGLIST_ENAB)
			bzero(&di->rxp_dmah[rxout1], sizeof(hnddma_seg_map_t));
#if !defined(BCM_SECURE_DMA)
#if !defined(OSL_CACHE_COHERENT)
		pa = DMA_MAP(di->osh, PKTDATA(di->osh, p),
			di->rxbufsize, DMA_RX, p,
			&di->rxp_dmah[rxout1]);
#else
		pa = (dmaaddr_t)VIRT_TO_PHYS(PKTDATA(di->osh, p));
#endif // endif
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
#endif // endif
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
#endif // endif
		{
			di->linked_di->rxp[rxout2] = p;
		}

		flags = 0;	/* Reset the flags */
		if (rxout2 == (di->linked_di->nrxd - 1))
			flags = D64_CTRL1_EOT;

		/* Extract out host length */
		pktlen = PKTFRAGLEN(di->linked_di->osh, p, 1);

		/* Extract out host addresses */
		pa64.loaddr = PKTFRAGDATA_LO(di->linked_di->osh, p, 1);
		pa64.hiaddr = PKTFRAGDATA_HI(di->linked_di->osh, p, 1);

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
	W_REG(di->osh, &di->d64rxregs->ptr, di->rcvptrbase + I2B(rxout1, dma64dd_t));

	W_REG(di->linked_di->osh, &di->linked_di->d64rxregs->ptr,
		di->linked_di->rcvptrbase + I2B(rxout2, dma64dd_t));

	return !ring_empty;
}
#endif /* D11_SPLIT_RX_FD */
