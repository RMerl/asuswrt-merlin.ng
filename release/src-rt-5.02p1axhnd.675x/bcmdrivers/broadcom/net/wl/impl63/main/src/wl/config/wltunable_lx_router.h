/*
 * Broadcom 802.11 Networking Device Driver Configuration file
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
 * $Id: wltunable_lx_router.h 780032 2019-10-14 14:43:14Z $
 *
 * wl driver tunables
 */

#define D11CONF		0x40000000	/* D11 Core Rev
					 * 30 (43217).
					 */
#ifdef __CONFIG_DHDAP__
/* no support for legacy dongles in NIC mode */
#define D11CONF2	0x00010400	/* D11 Core Rev > 31, Rev 42(4360b0),
					 * 48(43570a2)
					 */
#define D11CONF3	0x0		/* D11 Core Rev > 63 */
#define ACCONF		0x00020002	/* AC-Phy Rev 1(4360b0), 17(43570a2) */
#define ACCONF2		0x00088000	/* AC-Phy Rev > 31, 47(43684b0) */
#else /* __CONFIG_DHDAP__ */
#define D11CONF2	0x00030400	/* D11 Core Rev > 31, Rev 42(4360b0),
					 * 48(43570a2), 49 (43602a3)
					 */
#define D11CONF3	0x00000002	/* D11 Core Rev > 63, 65(4365c0) */
#define ACCONF		0x00060002	/* AC-Phy Rev 1(4360b0), 17(43570a2),
					 * 18 (43602a3)
					 */
#define ACCONF2		0x00008002	/* AC-Phy Rev > 31, Rev 33(4365c0),
					 * 47(43684b0)
					 */
#endif /* __CONFIG_DHDAP__ */

#define D11CONF4	0x0
#define D11CONF5	0x0000000a	/* D11 Core Rev > 127, 129(43684b0),
					 * 131(6710a0)
					 */
#define ACCONF5		0x00000002	/* AC-Phy Rev > 127, Rev 129(6710a0) */
#define NCONF		0x00020000	/* Rev 17(43217) */
#define LCN20CONF	0x0

#define NRXBUFPOST	56	/* # rx buffers posted */
#define RXBND		24	/* max # rx frames to process */
#define PKTCBND		36	/* max # rx frames to chain */
#ifdef __ARM_ARCH_7A__
#define CTFPOOLSZ       512	/* max buffers in ctfpool */
#else
#define CTFPOOLSZ       192	/* max buffers in ctfpool */
#endif // endif

#define WME_PER_AC_TX_PARAMS 1
#define WME_PER_AC_TUNING 1

#define NTXD_AC3X3		512	/* TX descriptor ring */
#define NRXD_AC3X3		512	/* RX descriptor ring */
#define NTXD_LARGE_AC3X3	2048	/* TX descriptor ring */
#define NRXD_LARGE_AC3X3	2048	/* RX descriptor ring */
#define NRXBUFPOST_AC3X3	500	/* # rx buffers posted */
#define RXBND_AC3X3		36	/* max # rx frames to process */

#define STSBUF_MP_N_OBJ 2048
#define NRXD_STS 2048
#define NRXBUFPOST_STS 1024

#ifdef __ARM_ARCH_7A__
#if defined(BCM_GMAC3)
#define CTFPOOLSZ_AC3X3		1536	/* max buffers in ctfpool */
#else
#define CTFPOOLSZ_AC3X3		1024	/* max buffers in ctfpool */
#endif /* ! BCM_GMAC3 */
#else
#define CTFPOOLSZ_AC3X3		512	/* max buffers in ctfpool */
#endif /* ! __ARM_ARCH_7A__ */
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
