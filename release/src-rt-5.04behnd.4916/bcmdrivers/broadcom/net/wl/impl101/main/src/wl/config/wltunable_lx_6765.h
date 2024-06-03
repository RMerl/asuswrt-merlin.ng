/*
 * Broadcom 802.11 Networking Device Driver Configuration file
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
 * $Id: wltunable_lx_6765.h 833916 2023-12-07 03:30:05Z $
 *
 * wl driver tunables
 */

#define D11CONF		0x0
#define D11CONF2	0x0
#define D11CONF3	0x0
#define D11CONF4	0x0
#define ACCONF		0x0
#define ACCONF2		0x0

#define D11CONF5	0x000004C8	/* D11 Core Rev > 127, 131(6710)
					 * 134(6726B0), 135(6765), 138(6711)
					 */
#define ACCONF5		0x00000262	/* AC-Phy Rev > 127, Rev 129(6710),
					 * 133(6726B0), 134(6765), 137(6711)
					 */

#define NRXBUFPOST	56	/* # rx buffers posted */
#define RXBND		24	/* max # rx frames to process */
#define PKTCBND		36	/* max # rx frames to chain */

#define NTXD_AC3X3		512	/* TX descriptor ring */
#define NRXD_AC3X3		512	/* RX descriptor ring */
#define NTXD_LARGE_AC3X3	2048	/* TX descriptor ring */
#define NRXD_LARGE_AC3X3	2048	/* RX descriptor ring */
#define NRXBUFPOST_AC3X3	500	/* # rx buffers posted */
#define NTXD_VIP_FIFO		4096	/* VIP TX descriptor ring */
#define RXBND_AC3X3		36	/* max # rx frames to process */
#define NRXBUFPOST_CLASSIFIED_FIFO	16 /* rxpost for classified fifo */
#define NRXD_CLASSIFIED_FIFO		64 /* rxd for classified fifo */
#define BCMRX_PCN_PKTPOOL_LEN		64 /* max pcn pktpool length */

#define PKTCBND_AC3X3		48	/* max # rx frames to chain */

#define TXMR			2	/* number of outstanding reads */
#define TXPREFTHRESH		8	/* prefetch threshold */
#define TXPREFCTL		16	/* max descr allowed in prefetch request */
#define TXBURSTLEN		256	/* burst length for dma reads */

#define RXPREFTHRESH		1	/* prefetch threshold */
#define RXPREFCTL		8	/* max descr allowed in prefetch request */
#define RXBURSTLEN		256	/* burst length for dma writes */

#define MRRS			512	/* Max read request size */

/* AC2 settings */
#define TXMR_AC2		12	/* number of outstanding reads */
#define TXPREFTHRESH_AC2	8	/* prefetch threshold */
#define TXPREFCTL_AC2		16	/* max descr allowed in prefetch request */
#define TXBURSTLEN_AC2		1024	/* burst length for dma reads */
#define RXPREFTHRESH_AC2	8	/* prefetch threshold */
#define RXPREFCTL_AC2		16	/* max descr allowed in prefetch request */
#define RXBURSTLEN_AC2		128	/* burst length for dma writes */
#define MRRS_AC2		1024	/* Max read request size */
/* AC2 settings */

#define AMPDU_PKTQ_LEN		8192

#define WLRXEXTHDROOM -1        /* to reserve extra headroom in DMA Rx buffer */

#define TX_STATUS_MACTXS_RING_DEPTH	1024	/* TxStatus Ring depth */
#define PHYRX_STATUS_RING_DEPTH		2048	/* PhyRx Status Ring depth */

/* D11 Core Rev > 133 11BE Tunables */
#define NTXD_LARGE_BE		2048	/* TX descriptor ring */
#define NRXD_LARGE_BE		2048	/* RX descriptor ring */
#define NRXBUFPOST_BE		1024	/* # rx buffers posted for 11BE chips */
#define PKTCBND_BE		64	/* max # rx frames to chain */
