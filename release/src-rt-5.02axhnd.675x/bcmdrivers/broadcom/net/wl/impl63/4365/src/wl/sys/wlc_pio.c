/*
 * HND PIO module
 * Broadcom 802.11abg Networking Device Driver
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
 * $Id: wlc_pio.c 708017 2017-06-29 14:11:45Z $
 */

/* XXX: Define wlc_cfg.h to be the first header file included as some builds
 * get their feature flags thru this file.
 */
#include <wlc_cfg.h>

#ifndef WLPIO
#error "WLPIO is not defined"
#endif	/* WLPIO */

#include <typedefs.h>
#include <bcmdefs.h>
#include <osl.h>

#include <bcmutils.h>
#include <siutils.h>
#include <bcmendian.h>
#include <wlioctl.h>

#include <sbhndpio.h>
#include <d11.h>
#include <wlc_rate.h>
#include <wlc_pub.h>
#include <wlc_bsscfg.h>
#include <wlc.h>

/* private pio structure */
typedef struct pio_info_s {
	pio_t		piopub;			/* public structure of pio module */
	wlc_info_t	*wlc;			/* common code handler */
	wlc_pub_t	*pub;			/* public common code handler */
	uint		_fifo;			/* fifo number */
	bool		pio4;			/* PIO 4 bytes or 2 bytes */
	pio2regs_t	*p2txregs;		/* pio2 trx regs */
	pio2regs_t	*p2rxregs;		/* pio2 rx regs */
	pio4regs_t	*p4txregs;		/* pio4 tx regs */
	pio4regs_t	*p4rxregs;		/* pio4 rx regs */
	/* pio */
	struct pktq	txpioq;			/* tx sw queue, wait txstatus to reclaim */
	uint		txfrmcnt;		/* tx frame count */
	uint		txdatacnt;		/* tx data count */
	uint		txfifolimit;		/* tx fifo depth */

	bool		war38778;
	pio_rx_t	rx_fn_def;		/* default rx function pointer */
	pio_tx_t	tx_fn_def;		/* default tx function pointer */
} pio_info_t;

#define PIO_INFO(pio)	(pio_info_t *)pio

#define	PIO_TX	1 /* Xmit engine for PIO Fifo */
#define	PIO_RX	2 /* Recv. engine for PIO Fifo */

/* local prototype */

/* pio common routines for both pio2 and pio4 */
static int _wlc_pio_com_detach(pio_info_t *pt);
static int _wlc_pio_com_init(pio_info_t *pt);
static int _wlc_pio_com_reset(pio_info_t *pt);
static int _wlc_pio_com_txsuspend(pio_info_t *pt);
static bool _wlc_pio_com_txsuspended(pio_info_t *pt);
static int _wlc_pio_com_txresume(pio_info_t *pt);
static bool _wlc_pio_com_txavailable(pio_info_t *pt, uint len, uint nfrags);
static bool _wlc_pio_com_rxfrmrdy(pio_info_t *pt);
static int _wlc_pio_com_cntupd(pio_info_t *pt, uint len);
static void _wlc_pio_com_dump(pio_info_t *pt, struct bcmstrbuf *b);
static void* _wlc_pio_getnexttxp(pio_info_t *pt);
static int   _wlc_pio_txreclaim(pio_info_t *pt);
static int   _wlc_pio_txfifodepthset(pio_info_t *pt, uint len);
static uint  _wlc_pio_txfifodepthget(pio_info_t *pt);

/* pio routines for 2 bytes pio */
static int _wlc_pio2_tx(pio_info_t *pt, void *p0);
static void _wlc_pio2_xmtfifo(pio_info_t *pioh, pio2regs_t *regs, uchar *va, uint len);
static void *_wlc_pio2_rx(pio_info_t *pt);

/* pio routines for 4 bytes pio */
static int _wlc_pio4_tx(pio_info_t *pt, void *p0);
static void _wlc_pio4_xmtfifo(pio_info_t *pt, pio4regs_t *regs, uchar *va, uint len);
static void *_wlc_pio4_rx(pio_info_t *pt);

static bool _wlc_pio_rxcheck(pio_info_t *pt, int len, void **p);
static volatile void *wlc_pioregs_offset(wlc_info_t *wlc, int txrx, int fifonum);

static void *_wlc_pio_rx_war38778(pio_info_t *pt);
static int _wlc_pio_tx_war38778(pio_info_t *pt, void *p);

/* len is rounded up since free fifo length in ucode counts in multiples of 4 */
#define PIO_ADD_TX_CNT(pt, len) {	\
	pt->txfrmcnt++; \
	ASSERT(pt->txfrmcnt <= D11_MAX_TX_FRMS); \
	pt->txdatacnt += ROUNDUP(len, 4); \
	ASSERT(pt->txdatacnt <= pt->txfifolimit);	\
	}
#define PIO_SUB_TX_CNT(pt, len) {	\
	ASSERT(pt->txfrmcnt); \
	pt->txfrmcnt--;	\
	ASSERT(pt->txdatacnt >= ROUNDUP(len, 4));	\
	pt->txdatacnt -= ROUNDUP(len, 4);	\
	}

#define WLC_WAR38778(pt) ((pt)->war38778)

#ifdef BCMSDIO
/* Convenience "flags" values for bcmsdh calls */
#define SD_FIFO_2	SDIO_REQ_FIXED
#define SD_FIFO_4	(SDIO_REQ_FIXED | SDIO_REQ_4BYTE)
#endif // endif

pio_t*
wlc_pio_attach(wlc_pub_t *pub, wlc_info_t *cwlc, uint fifo)
{
	pio_info_t *pt;
	piof_t	pio_fns;
	volatile void *pioregstx;
	volatile void *pioregsrx;

	if ((pt = (pio_info_t *)MALLOC(pub->osh, sizeof(pio_info_t))) == NULL) {
		WL_ERROR(("wlc_pio_attach: out of memory, malloced %d bytes", MALLOCED(pub->osh)));
		return NULL;
	}
	bzero((char *)pt, sizeof(pio_info_t));
	pt->wlc = cwlc;
	pt->pub = pub;
	pt->_fifo = fifo;

	/* pio tx and rx register addresses */
	pioregstx = wlc_pioregs_offset(cwlc, PIO_TX, fifo);
	pioregsrx = wlc_pioregs_offset(cwlc, PIO_RX, fifo);

	/*
	 * register pio functions
	 * To add a new one, add the type and array element in wlc_pio.h,
	 *    add handler here and in wlc_pio_register_fn
	 */
	pio_fns.detach = (pio_detach_t)_wlc_pio_com_detach;
	pio_fns.reset = (pio_reset_t)_wlc_pio_com_reset;
	pio_fns.init = (pio_init_t)_wlc_pio_com_init;
	pio_fns.txsuspend = (pio_txsuspend_t)_wlc_pio_com_txsuspend;
	pio_fns.txsuspended = (pio_txsuspended_t)_wlc_pio_com_txsuspended;
	pio_fns.txresume = (pio_txresume_t)_wlc_pio_com_txresume;
	pio_fns.txavailable = (pio_txavailable_t)_wlc_pio_com_txavailable;
	pio_fns.rxfrmrdy = (pio_rxfrmrdy_t)_wlc_pio_com_rxfrmrdy;
	if (WLC_UPDATE_STATS(cwlc)) {
		pio_fns.dump = (pio_dump_t)_wlc_pio_com_dump;
	}
	else {
		pio_fns.dump = NULL;
	}
	pio_fns.cntupd = (pio_cntupd_t)_wlc_pio_com_cntupd;
	pio_fns.nexttxp = (pio_getnexttxp_t)_wlc_pio_getnexttxp;
	pio_fns.txreclaim = (pio_txreclaim_t)_wlc_pio_txreclaim;
	pio_fns.txdepthget = (pio_txfifodepthget_t)_wlc_pio_txfifodepthget;
	pio_fns.txdepthset = (pio_txfifodepthset_t)_wlc_pio_txfifodepthset;

	/* init pio reg pointer */
	if (D11REV_LT(pt->pub->corerev, 8)) {
		pt->p2txregs = (pio2regs_t *)pioregstx;
		pt->p2rxregs = (pio2regs_t *)pioregsrx;
		pio_fns.rx = (pio_rx_t)_wlc_pio2_rx;
		pio_fns.tx = (pio_tx_t)_wlc_pio2_tx;
		pt->pio4 = 0;
	} else {
		pt->p4txregs = (pio4regs_t *)pioregstx;
		pt->p4rxregs = (pio4regs_t *)pioregsrx;
		pio_fns.rx = (pio_rx_t)_wlc_pio4_rx;
		pio_fns.tx = (pio_tx_t)_wlc_pio4_tx;
		pt->pio4 = 1;
	}

	wlc_pio_register_fn((pio_t *)pt, &pio_fns);

	pktq_init(&pt->txpioq, 1, PKTQ_LEN_DEFAULT);
	ASSERT(pktq_avail(&pt->txpioq) >= D11_MAX_TX_FRMS);

	return (pio_t*)pt;
}

/* register pio mode function pointers */
void
wlc_pio_register_fn(pio_t *pioh, piof_t *fn)
{
	pio_info_t *pt = (pio_info_t *)pioh;

	pioh->pio_fn.detach = fn->detach;
	pioh->pio_fn.reset = fn->reset;
	pioh->pio_fn.init = fn->init;
	pioh->pio_fn.txsuspend = fn->txsuspend;
	pioh->pio_fn.txsuspended = fn->txsuspended;
	pioh->pio_fn.txresume = fn->txresume;
	pioh->pio_fn.rxfrmrdy = fn->rxfrmrdy;
	pioh->pio_fn.txavailable = fn->txavailable;
	pioh->pio_fn.dump = fn->dump;
	pioh->pio_fn.cntupd = fn->cntupd;
	pioh->pio_fn.nexttxp = fn->nexttxp;
	pioh->pio_fn.txreclaim = fn->txreclaim;
	pioh->pio_fn.txdepthget = fn->txdepthget;
	pioh->pio_fn.txdepthset = fn->txdepthset;

	if (WLC_WAR38778(pt)) {
		/* save default tx and rx function pointers */
		pt->tx_fn_def = fn->tx;
		pt->rx_fn_def = fn->rx;
		pioh->pio_fn.tx = (pio_tx_t)_wlc_pio_tx_war38778;
		pioh->pio_fn.rx = (pio_rx_t)_wlc_pio_rx_war38778;
	} else {
		pioh->pio_fn.tx = fn->tx;
		pioh->pio_fn.rx = fn->rx;
	}
}

static int
_wlc_pio_com_detach(pio_info_t *pt)
{
	if (pt) {
		/* assert pkts are freed */
		ASSERT(pktq_empty(&pt->txpioq));
		MFREE(pt->pub->osh, pt, sizeof(pio_info_t));
	}
	return 0;
}

/* pio initialization */
static int
_wlc_pio_com_init(pio_info_t *pt)
{
	if (WLC_WAR38778(pt)) {
		wlc_mhf(pt->wlc, MHF2, MHF2_4317PIORXWAR, 0, WLC_BAND_ALL);
	}

	return 0;
}

static int
_wlc_pio_com_reset(pio_info_t *pt)
{
	return 0;
}

static int
_wlc_pio_com_txsuspend(pio_info_t *pt)
{
	if (pt->pio4) {
		OR_REG(pt->wlc->osh, &pt->p4txregs->fifocontrol, XFC4_SE);
	} else {
		OR_REG(pt->wlc->osh, &pt->p2txregs->fifocontrol, XFC_SE);
	}
	return 0;
}

static bool
_wlc_pio_com_txsuspended(pio_info_t *pt)
{
	if (pt->pio4) {
		if ((R_REG(pt->wlc->osh, &pt->p4txregs->fifocontrol) & (XFC4_SE | XFC4_SP))
	            == XFC4_SE)
			return TRUE;
	} else {
		if ((R_REG(pt->wlc->osh, &pt->p2txregs->fifocontrol) & (XFC_SE | XFC_SP))
		    == XFC_SE)
			return TRUE;
	}
	return FALSE;
}

static int
_wlc_pio_com_txresume(pio_info_t *pt)
{
	if (pt->pio4) {
		AND_REG(pt->wlc->osh, &pt->p4txregs->fifocontrol, ~XFC4_SE);
	} else {
		AND_REG(pt->wlc->osh, &pt->p2txregs->fifocontrol, ~XFC_SE);
	}
	return 0;
}

static bool
_wlc_pio_com_rxfrmrdy(pio_info_t *pt)
{
	if (pt->pio4)
		return (R_REG(pt->wlc->osh, &pt->p4rxregs->fifocontrol) & RFC_FR);
	else
		return (R_REG(pt->wlc->osh, &pt->p2rxregs->fifocontrol) & RFC_FR);
}

static bool
_wlc_pio_com_txavailable(pio_info_t *pt, uint len, uint nfrags)
{
	if (((pt->txfrmcnt + nfrags) <= D11_MAX_TX_FRMS) &&
	    ((pt->txdatacnt + ((nfrags == 1) ?
		(ROUNDUP(len, 4)) : (len + 3*nfrags))) <= pt->txfifolimit)) {
		if (pt->pio4)
			return (!(R_REG(pt->wlc->osh, &pt->p4txregs->fifocontrol) & XFC4_SE));
		else
			return (!(R_REG(pt->wlc->osh, &pt->p2txregs->fifocontrol) & XFC_SE));
	}

	return FALSE;
}

static int
_wlc_pio_com_cntupd(pio_info_t *pt, uint len)
{
	PIO_SUB_TX_CNT(pt, len);
	return 0;
}

/* need fifo to avoid touch some rx fifo, which doesn't exist */
static void
_wlc_pio_com_dump(pio_info_t *pt, struct bcmstrbuf *b)
{
	/* common software part */
	bcm_bprintf(b, "pio%d: qlen %d txfrmcnt %d txdatacnt %d txfifodepth %d\n",
		pt->_fifo, pktq_len(&pt->txpioq), pt->txfrmcnt, pt->txdatacnt, pt->txfifolimit);

	if (pt->pio4) {
		bcm_bprintf(b, "xmtcontrol 0x%x\n",
		               R_REG(pt->wlc->osh, &pt->p4txregs->fifocontrol));
		if (pt->_fifo == RX_FIFO) {
			bcm_bprintf(b, "rcvcontrol 0x%x\n",
			               R_REG(pt->wlc->osh, &pt->p4rxregs->fifocontrol));
		}
	} else {
		bcm_bprintf(b, "xmtcontrol 0x%x\n",
		               R_REG(pt->wlc->osh, &pt->p2txregs->fifocontrol));
		if (pt->_fifo == RX_FIFO) {
			bcm_bprintf(b, "rcvcontrol 0x%x\n",
			               R_REG(pt->wlc->osh, &pt->p2rxregs->fifocontrol));
		}
	}
}

/* 2-byte mode for corerev < 8. poke a chain of fragment buffers into tx channel fifo */
static int
_wlc_pio2_tx(pio_info_t *pt, void *p0)
{
	pio2regs_t *pioregs = pt->p2txregs;
	void *p;
	osl_t *osh;
	uint totlen;
	uchar *va;
	uint len;

	WL_TRACE(("wl%d: _wlc_pio2_tx\n", pt->pub->unit));

	ASSERT(pioregs != NULL);
	osh = pt->pub->osh;
	totlen = pkttotlen(osh, p0);

	/* ASSERT(_wlc_pio_tx_available((pio_t *)pt, totlen, 1)); */

	PIO_ADD_TX_CNT(pt, totlen);

	/* clear frameready */
	W_REG(pt->wlc->osh, &pioregs->fifocontrol, XFC_FR);

	/*
	 * Walk the chain of fragments poking each into the tx fifo.
	 * Dealing with PR3775 and arbitrary chains of zero-length buffers
	 * complicates this considerably.
	 */
	for (p = p0; p; p = PKTNEXT(osh, p)) {
		va = PKTDATA(osh, p);
		len = PKTLEN(osh, p);

		/* skip any zero-byte buffers */
		if (len == 0)
			continue;

		_wlc_pio2_xmtfifo(pt, pioregs, va, len);
		totlen -= len;
	}

	W_REG(pt->wlc->osh, &pioregs->fifocontrol, XFC_EF);

	/* save frag in pio sw queue */
	pktenq(&pt->txpioq, p0);
	return 0;
}

/* 2-byte mode for corerev < 8.  move tx data to fifo */
static void
_wlc_pio2_xmtfifo(pio_info_t *pt, pio2regs_t *regs, uchar *va, uint len)
{
	uint16 *va16;
	volatile uint16 *fifo;
#ifdef BCMSDIO
	int sderr;
#endif // endif

	if (len == 0)
		return;

	fifo = &regs->fifodata;

	/* write any odd leading byte */
	if ((uintptr)va & 1) {
		W_REG(pt->wlc->osh, &regs->fifocontrol, XFC_LO);
		W_REG(pt->wlc->osh, fifo, (uint16)*va);
		va++;
		len--;
	}

	va16 = (uint16*)va;

	if (len >= 2) {
		W_REG(pt->wlc->osh, &regs->fifocontrol, XFC_BOTH);

#ifdef BCMSDIO
		/* write the middle bytes, using bursting for SDIO */
		if (BUSTYPE(pt->pub->sih->bustype) == SDIO_BUS) {
			sderr = bcmsdh_send_buf(pt->wlc->btparam, (uintptr)fifo, 1, SD_FIFO_2,
			                        (uint8 *)va16, (len/2)*2, NULL, NULL, NULL);
			ASSERT(sderr != BCME_PENDING);
			if (sderr) {
				WL_ERROR(("SDIO send error"));
				return;
			}
			va16 += len / 2;
			len = len % 2;
		} else
#endif /* BCMSDIO */
		{
			while (len >= 2) {
				W_REG(pt->wlc->osh, fifo, *va16);
				va16++;
				len -= 2;
			}
		}
	}

	/* write any odd trailing byte */
	if (len) {
		W_REG(pt->wlc->osh, &regs->fifocontrol, XFC_LO);
		W_REG(pt->wlc->osh, fifo, (uint16)*(uint8*)va16);
	}
}

/* 4-byte mode for corerev >= 8. poke a chain of fragment buffers into tx channel fifo */
static int
_wlc_pio4_tx(pio_info_t *pt, void *p0)
{
	pio4regs_t *pioregs = pt->p4txregs;
	void *p;
	osl_t *osh;
	uint totlen;
	uchar *va;
	uint len;

	WL_TRACE(("wl%d: _wlc_pio4_tx\n", pt->pub->unit));

	ASSERT(pioregs != NULL);
	osh = pt->pub->osh;
	totlen = pkttotlen(osh, p0);

	/* ASSERT(_wlc_pio_tx_available((pio_t *)pt, totlen, 1)); */

	PIO_ADD_TX_CNT(pt, totlen);

	/* clear frameready */
	W_REG(pt->wlc->osh, &pioregs->fifocontrol, XFC4_FR);

	/*
	 * Walk the chain of fragments poking each into the tx fifo.
	 * Dealing with PR3775 and arbitrary chains of zero-length buffers
	 * complicates this considerably.
	 */
	for (p = p0; p; p = PKTNEXT(osh, p)) {
		va = PKTDATA(osh, p);
		len = PKTLEN(osh, p);

		/* skip any zero-byte buffers */
		if (len == 0)
			continue;

		_wlc_pio4_xmtfifo(pt, pioregs, va, len);
		totlen -= len;
	}

	W_REG(pt->wlc->osh, &pioregs->fifocontrol, XFC4_EF);

	/* save frag in pio sw queue */
	pktenq(&pt->txpioq, p0);
	return 0;
}

/* 4-byte mode for corerev >= 8.  move tx data to fifo */
static void
_wlc_pio4_xmtfifo(pio_info_t *pt, pio4regs_t *regs, uchar *va,
	uint len)
{
	uint    offset;
	uint		nbytes;
	uint		fifo_mask;
	volatile uint32 *fifo;
#ifdef BCMSDIO
	int sderr;
#endif // endif

	fifo = &regs->fifodata;

	while (len)
	{
		offset = (uint) ((uintptr)va & 0x03);
		nbytes = MIN(4 - offset, len);

		/*
		 *  Form bit mask that controls xmit fifo.
		 *  Note: 2**n - 1 forms a bit mask n bits wide.
		 *  Shift the mask to the desired offset.
		 */
#if !defined(IL_BIGENDIAN)
		fifo_mask = ((1 << nbytes) - 1) << offset;
#else
		fifo_mask = ((1 << nbytes) - 1) << (4 - nbytes - offset);
#endif /* IL_BIGENDIAN */
		W_REG(pt->wlc->osh, &regs->fifocontrol, fifo_mask);

		/*
		 *  Emit bytes as either a partial word or one or more
		 *  full 4-byte words
		 */
		if (nbytes < 4)
		{
			uint32 val = *(uint32 *)((uintptr)va & ~0x03);
			W_REG(pt->wlc->osh, fifo, val);
			va += nbytes;
			len -= nbytes;
		}
		else
		{
#ifdef BCMSDIO
			if (BUSTYPE(pt->pub->sih->bustype) == SDIO_BUS) {
				nbytes = len & ~0x03;
				sderr = bcmsdh_send_buf(pt->wlc->btparam, (uintptr)fifo,
					1, SD_FIFO_4, (uint8 *)va, nbytes, NULL, NULL, NULL);
				ASSERT(sderr != BCME_PENDING);
				if (sderr) {
					WL_ERROR(("SDIO send error"));
					return;
				}
				va += nbytes;
				len -= nbytes;
			}
			else
#endif /* BCMSDIO */
			{
				while (len >= 4) {
					W_REG(pt->wlc->osh, fifo, *(uint32 *) va);
					va += 4;
					len -= 4;
				}
			}
		}
	}
}

/* shared rx process between pio2 and pio4 */
static bool
_wlc_pio_rxcheck(pio_info_t *pt, int len, void **p)
{
	/* flush giant frames */
	if (len > pt->pub->tunables->rxbufsz) {
		WL_ERROR(("wl%d: %s: giant frame. len %d\n", pt->pub->unit, __FUNCTION__, len));
		WLCNTINCR(pt->pub->_cnt->rxgiant);
		return FALSE;
	}
	/* flush runt frames */
	if (len == 0) {
		WL_ERROR(("wl%d: %s: runt frame\n", pt->pub->unit, __FUNCTION__));
		WLCNTINCR(pt->pub->_cnt->rxrunt);
		return FALSE;
	}
	/* alloc a new buf-wlc_recvdata() fragment reassembly requires RXBUFSZ receive buffers */
	if ((*p = PKTGET(pt->pub->osh, pt->pub->tunables->rxbufsz, FALSE)) == NULL) {
		WL_ERROR(("wl%d: %s: out of rxbufs\n", pt->pub->unit, __FUNCTION__));
		WLCNTINCR(pt->pub->_cnt->rxnobuf);
		return FALSE;
	}

	return TRUE;
}

/* 2-byte mode for corerev < 8.  move rx fifo data to buffer */
static void*
_wlc_pio2_rx(pio_info_t *pt)
{
	void *p = NULL;
	osl_t *osh;
	uint8 *va;
	uint16 rxstatus;
	uint32 fifocontrol;
	d11rxhdr_t *rxh;
	int i, len;
	pio2regs_t *regs;
#ifdef BCMSDIO
	int sderr;
#endif // endif

	uint16 *va16;
	volatile uint16 *dptr, w;

	WL_TRACE(("wl%d: _wlc_pio2_rx\n", pt->pub->unit));

	osh = pt->pub->osh;
	regs = pt->p2rxregs;
	ASSERT(regs != NULL);

	/* if frame is not ready, return */
	if (!(R_REG(pt->wlc->osh, &regs->fifocontrol) & RFC_FR))
		return NULL;
	/* clear frame ready */
	W_REG(pt->wlc->osh, &regs->fifocontrol, RFC_FR);
	/* wait for data valid */
	SPINWAIT((((fifocontrol = R_REG(pt->wlc->osh,
	                                &regs->fifocontrol)) & RFC_DR) == 0), 100);
	if ((fifocontrol & RFC_DR) == 0) {
		WL_ERROR(("wl%d: %s: data ready never set\n", pt->pub->unit, __FUNCTION__));
		return NULL;
	}

	dptr = &regs->fifodata;	/* rcv data */
	/* read the first 16bits of the rxhdr which gives the length */
	w = R_REG(pt->wlc->osh, dptr);
	/* hanlde both Big and Little endian */
	len = ltoh16(w);

	if (!_wlc_pio_rxcheck(pt, len, &p))
		goto flush;

	va = (uint8 *)PKTDATA(osh, p);
	ASSERT(ISALIGNED(va, sizeof(uint32)));
	PKTSETLEN(osh, p, (pt->wlc->hwrxoff + len));

	/* read the rxheader from the fifo first */
	va16 = (uint16 *)va;
	*va16++ = w;
	i = (RXHDR_LEN / 2) - 1;
#ifdef BCMSDIO
	if (pt->pub->sih->bustype == SDIO_BUS) {
		sderr = bcmsdh_recv_buf(pt->wlc->btparam, (uintptr)dptr, 1, SD_FIFO_2,
		                        (uint8 *)va16, i*2, NULL, NULL, NULL);
		ASSERT(sderr != BCME_PENDING);
		if (sderr) {
			WL_ERROR(("wl%d: %s: SDIO receive error", pt->pub->unit, __FUNCTION__));
			goto flush;
		}
	} else
#endif /* BCMSDIO */
	{
		while (i--) {
			*va16 = R_REG(pt->wlc->osh, dptr);
			va16++;
		}
	}

	/* toss bad packet before read/pass up whole frame */
	rxh = (d11rxhdr_t*)va;
	rxstatus = ltoh16(rxh->RxStatus1);
	if (rxstatus & RXS_FCSERR) {
		/* ucode should toss it */
		ASSERT(0);
		goto flush;
	}

	/* read the frame */
	va16 = (uint16*) &va[pt->wlc->hwrxoff];

	if (rxstatus & RXS_PBPRES) {
		/* advance data pointer by two-byte for address alignment */
		va16++;
		PKTSETLEN(osh, p, (pt->wlc->hwrxoff + len + 2));
	}

#ifdef BCMSDIO
	if (pt->pub->sih->bustype == SDIO_BUS) {
		/* receive all bytes */
		sderr = bcmsdh_recv_buf(pt->wlc->btparam, (uintptr)dptr, 1, SD_FIFO_2,
		                        (uint8 *)va16, len, NULL, NULL, NULL);
		ASSERT(sderr != BCME_PENDING);
		if (sderr) {
			WL_ERROR(("wl%d: %s: SDIO receive error", pt->pub->unit, __FUNCTION__));
			goto flush;
		}
	} else
#endif /* BCMSDIO */
	{
		i = len / 2;
		while (i--) {
			*va16 = R_REG(pt->wlc->osh, dptr);
			va16++;
		}
		/* read last (odd) byte */
		if (len & 1) {
			*va16 = R_REG(pt->wlc->osh, dptr) & 0xff;
		}
	}

	return (p);

flush:
	/* flush rx frame */
	W_REG(pt->wlc->osh, &regs->fifocontrol, RFC_DR);
	if (p)
		PKTFREE(osh, p, FALSE);

	return (NULL);
}

/* 4-byte mode for corerev >= 8.  move rx fifo data to buffer */
static void*
_wlc_pio4_rx(pio_info_t *pt)
{
	void *p = NULL;
	void *retval = NULL;
	osl_t *osh;
	uint8 *va;
	uint16 rxstatus;
	uint32 fifocontrol;
	d11rxhdr_t *rxh;
	int i, len;
	pio4regs_t *regs;
	uint32 *va32;
	volatile uint32 *dptr, w;
#ifdef BCMSDIO
	int sderr;
#endif // endif
	bool mfh2_pio_rx = FALSE;

	WL_TRACE(("wl%d: _wlc_pio4_rx\n", pt->pub->unit));

	osh = pt->pub->osh;
	regs = pt->p4rxregs;
	ASSERT(regs != NULL);

	/* if frame is not ready, return */
	if (!(R_REG(pt->wlc->osh, &regs->fifocontrol) & RFC_FR)) {
		return NULL;
	} else if (WLC_WAR38778(pt)) {
		wlc_mhf(pt->wlc, MHF2, MHF2_4317PIORXWAR, MHF2_4317PIORXWAR, WLC_BAND_ALL);
		mfh2_pio_rx = TRUE;
	}

	/* clear frame ready */
	W_REG(pt->wlc->osh, &regs->fifocontrol, RFC_FR);
	/* wait for data valid */
	SPINWAIT((((fifocontrol = R_REG(pt->wlc->osh,
	                                &regs->fifocontrol)) & RFC_DR) == 0), 100);
	if ((fifocontrol & RFC_DR) == 0) {
		WL_ERROR(("wl%d: %s: data ready never set\n", pt->pub->unit, __FUNCTION__));
		goto ret_normal;
	}

	dptr = &regs->fifodata;	/* rcv data */
	/* read the first 16bits of the rxhdr which gives the length */
	w = R_REG(pt->wlc->osh, dptr);
	/* hanlde both Big and Little endian */
	len = (uint16)ltoh32(w);

	if (!_wlc_pio_rxcheck(pt, len, &p))
		goto flush;

	va = (uint8 *)PKTDATA(osh, p);
	ASSERT(ISALIGNED(va, sizeof(uint32)));
	PKTSETLEN(osh, p, (pt->wlc->hwrxoff + len));

	/* read the rxheader from the fifo first */
	va32 = (uint32 *)va;
	*va32++ = w;
	i = (RXHDR_LEN / 4) - 1;
#ifdef BCMSDIO
	if (BUSTYPE(pt->pub->sih->bustype) == SDIO_BUS) {
		sderr = bcmsdh_recv_buf(pt->wlc->btparam, (uintptr)dptr, 1, SD_FIFO_4,
		                        (uint8 *)va32, i*4, NULL, NULL, NULL);
		ASSERT(sderr != BCME_PENDING);
		if (sderr) {
			WL_ERROR(("wl%d: %s: SDIO receive error", pt->pub->unit, __FUNCTION__));
			goto flush;
		}
		va32 += i;
	} else
#endif /* BCMSDIO */
	{
		while (i--) {
			*va32 = R_REG(pt->wlc->osh, dptr);
			va32++;
		}
	}

	if (RXHDR_LEN % 4) {
		ASSERT((RXHDR_LEN % 4) == 2);
		*((uint16 *)va32) = R_REG(pt->wlc->osh, (volatile uint16 *)dptr);
		va32 = (uint32 *)((int8*)va32 + 2);
	}

	/* toss bad packet before read/pass up whole frame */
	rxh = (d11rxhdr_t*)va;
	rxstatus = ltoh16(rxh->RxStatus1);
	if (rxstatus & RXS_FCSERR) {
		/* ucode should toss it */
		ASSERT(0);
		goto flush;
	}

	/* read the frame */
	va32 = (uint32*) &va[pt->wlc->hwrxoff];

	/* align buffer to 32-bit boundary */
	if ((uintptr)va32 % 4 && len >= 2) {
		uint32 val = R_REG(pt->wlc->osh, (volatile uint16 *)dptr);
		*((uint16 *)va32) = (uint16)val;
		va32 = (uint32 *)((int8*)va32 + 2);
		len -= 2;
	}

	i = len / 4;

#ifdef BCMSDIO
	if (BUSTYPE(pt->pub->sih->bustype) == SDIO_BUS) {
		/* receiving the whole buffer other than tailing odd bytes */
		sderr = bcmsdh_recv_buf(pt->wlc->btparam, (uintptr)dptr, 1, SD_FIFO_4,
		                        (uint8 *)va32, i*4, NULL, NULL, NULL);
		ASSERT(sderr != BCME_PENDING);
		if (sderr) {
			WL_ERROR(("wl%d: %s: SDIO receive error", pt->pub->unit, __FUNCTION__));
			goto flush;
		}
		va32 += i;
	} else
#endif /* BCMSDIO */
	{
		while (i--) {
			*va32 = R_REG(pt->wlc->osh, dptr);
			va32++;
		}
	}

	/* read the remain bytes if exist */
	i = len % 4;
	if (i) {
		uint32 mask[] = {0, 0x000000ff, 0x0000ffff, 0x00ffffff};
		*va32 = R_REG(pt->wlc->osh, dptr) & mask[i];
	}

	retval = p;

ret_normal:
	if (mfh2_pio_rx) {
		wlc_mhf(pt->wlc, MHF2, MHF2_4317PIORXWAR, 0, WLC_BAND_ALL);
	}

	return (retval);

flush:
	/* flush rx frame */
	W_REG(pt->wlc->osh, &regs->fifocontrol, RFC_DR);
	if (p)
		PKTFREE(osh, p, FALSE);

	if (mfh2_pio_rx) {
		wlc_mhf(pt->wlc, MHF2, MHF2_4317PIORXWAR, 0, WLC_BAND_ALL);
	}
	return (retval);
}

static void*
_wlc_pio_getnexttxp(pio_info_t *pt)
{
	return pktdeq(&pt->txpioq);
}

static int
_wlc_pio_txreclaim(pio_info_t *pt)
{
	void *p;
	while ((p = pktdeq(&pt->txpioq)))
		PKTFREE(pt->pub->osh, p, TRUE);

	/* reset per-fifo software counters */
	pt->txfrmcnt = 0;
	pt->txdatacnt = 0;
	return 0;
}

static int
_wlc_pio_txfifodepthset(pio_info_t *pt, uint len)
{
	pt->txfifolimit = len;
	return 0;
}

static uint
_wlc_pio_txfifodepthget(pio_info_t *pt)
{
	return pt->txfifolimit;
}

static void*
_wlc_pio_rx_war38778(pio_info_t *pt)
{
	void *p;
	wlc_info_t *wlc = pt->wlc;
	wlc_bsscfg_t *cfg = wlc->cfg;

	if (cfg->pm->PMenabled) {
		bool wake = wlc->wake;
		wlc->wake = TRUE;
		wlc_set_ps_ctrl(cfg);
		p = pt->rx_fn_def((pio_t*)pt);
		wlc->wake = wake;
		wlc_set_ps_ctrl(cfg);
		return p;
	} else
		return pt->rx_fn_def((pio_t*)pt);
}

static int
_wlc_pio_tx_war38778(pio_info_t *pt, void *p)
{
	int status;
	wlc_info_t *wlc = pt->wlc;
	wlc_bsscfg_t *cfg = wlc->cfg;

	if (cfg->pm->PMenabled) {
		bool wake = wlc->wake;
		wlc->wake = TRUE;
		wlc_set_ps_ctrl(cfg);
		status = pt->tx_fn_def((pio_t*)pt, p);
		wlc->wake = wake;
		wlc_set_ps_ctrl(cfg);
	} else
		status = pt->tx_fn_def((pio_t*)pt, p);

	return status;
}

/* pio register offsets in the d11 mac register space */
static volatile void *
wlc_pioregs_offset(wlc_info_t *wlc, int txrx, int fifonum)
{
	volatile void *regs;

	if (D11REV_LT(wlc->pub->corerev, 11)) {
		u_pioreg_t *pioregs;
		pioregs = &(wlc->regs->fifo.f32regs.pioregs[fifonum]);
		if (D11REV_LT(wlc->pub->corerev, 8)) {
			if (txrx == PIO_TX)
				regs = (volatile void *)&pioregs->b2.tx;
			else
				regs = (volatile void *)&pioregs->b2.rx;
		}
		else {
			if (txrx == PIO_TX)
				regs = (volatile void *)&pioregs->b4.tx;
			else
				regs = (volatile void *)&pioregs->b4.rx;
		}
	}
	else {
		fifo64_t *fifo;

		fifo = &(wlc->regs->fifo.f64regs[fifonum]);

		if (txrx == PIO_TX)
			regs = (volatile void *)&fifo->piotx;
		else
			regs = (volatile void *)&fifo->piorx;

	}
	return regs;
}
