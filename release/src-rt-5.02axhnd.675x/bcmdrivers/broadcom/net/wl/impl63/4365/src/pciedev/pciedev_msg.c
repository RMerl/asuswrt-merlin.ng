/*
 * Broadcom PCIE2 device-side driver
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
 * $Id: pciedev.c	Harishv$
 */

#include <typedefs.h>
#include <bcmdefs.h>
#include <osl.h>
#include <hndsoc.h>
#include <bcmdevs.h>
#include <hndcpu.h>
#include <siutils.h>
#include <epivers.h>
#include <sbhndarm.h>
#include <pcie_core.h>
#include <dngl_bus.h>
#include <dngl_api.h>
#include <sbhnddma.h>
#include <hnddma.h>
#include <dngl_bus.h>
#include <d11.h>
#include <bcmendian.h>
#include <bcm_ol_msg.h>
#include <rte_dev.h>
#include <rte_isr.h>

/* Pool length to receive messages from Host */
#define SDHDR_POOL_LEN 10

/* Size of Buffer to receive message from Host */
#define MSGBUFSZ 	1024

static void *
pciedngl_probe(hnd_dev_t *rtedev, void *regs, uint bus, uint16 device, uint coreid, uint unit);
static int pciedngl_open(hnd_dev_t *drv);
static int pciedngl_close(hnd_dev_t *drv);
static int
pciedngl_send(hnd_dev_t *src, hnd_dev_t *drv, struct lbuf *lb);
static int
pciedngl_ioctl(hnd_dev_t *drv, uint32 cmd, void *buf, int len, int *used, int *needed, int set);
static void pciedngl_isr(void *cbdata);
int pciedngl_bus_binddev(void *bus, void *dev, uint numslaves);
int pciedngl_bus_unbinddev(void *bus, void *dev);
int
pciedngl_bus_tx(struct dngl_bus *bus, void *p);
static void pciedngl_ol_msg_attach(struct dngl_bus *pciedngl);
static int pciedngl_msgpool_attach(struct dngl_bus *pciedngl);

static void
pciedngl_poll(hnd_dev_t *dev)
{
	pciedngl_isr(dev);
}

static hnd_dev_ops_t pciedngl_funcs = {
	probe:		pciedngl_probe,
	open:		pciedngl_open,
	close:		pciedngl_close,
	xmit:		pciedngl_send,
	ioctl:		pciedngl_ioctl,
	txflowcontrol:	NULL,
	poll:		pciedngl_poll,
	xmit_ctl:	NULL
};

struct dngl_bus_ops pciedngl_bus_ops = {
	softreset:	NULL,
	binddev:	pciedngl_bus_binddev,
	unbinddev:	pciedngl_bus_unbinddev,
	tx:		pciedngl_bus_tx,
	sendctl:	NULL,
	rxflowcontrol:	NULL,
	iovar:		NULL,
	resume:		NULL,
};

hnd_dev_t pciedngl_dev = {
	name:		"pciedngldev",
	ops:		&pciedngl_funcs
};

struct dngl_bus {
	uint unit;		/* device instance number */
	uint coreid;
	uint corerev;
	si_t *sih;			/* SiliconBackplane handle */
	osl_t *osh;
	struct dngl *dngl;
	sbpcieregs_t *regs;
	hnd_dev_t *rtedev;
	pktpool_t *pktpool_pmsg;
	olmsg_info ol_info;
	uchar *msgbuf;
};

/* Shared data between Host and device written at known location in TCM */
extern uint32 tramsz;

extern pktpool_t * pktpool_shared_msgs;

static int
pciedngl_open(hnd_dev_t *drv)
{
	struct dngl_bus *pciedngl = (struct dngl_bus *)drv->softc;

	/* Enable PCIE to SB interrupts */
	OR_REG(pciedngl->osh, &pciedngl->regs->intmask, PCIE_MB_TOPCIE_FN0_0);
	return 0;
}

static int
pciedngl_ioctl(hnd_dev_t *drv, uint32 cmd, void *buf, int len, int *used, int *needed, int set)
{
	return 0;
}

static int
pciedngl_close(hnd_dev_t *drv)
{
	struct dngl_bus *pciedngl = (struct dngl_bus *)drv->softc;

	/* Disable PCIE to SB interrupts */
	AND_REG(pciedngl->osh, &pciedngl->regs->intmask, ~PCIE_MB_TOPCIE_FN0_0);
	return 0;
}

static void *
pciedngl_probe(hnd_dev_t *rtedev, void *regs, uint bus, uint16 device, uint coreid, uint unit)
{
	osl_t *osh;
	struct dngl_bus *pciedngl;

	osh = osl_attach(rtedev);

	/* allocate private info */
	if (!(pciedngl = (struct dngl_bus *)MALLOC(NULL, sizeof(struct dngl_bus)))) {
		printf("%d: MALLOC failed\n", unit);
		goto fail;
	}
	bzero(pciedngl, sizeof(struct dngl_bus));

	pciedngl->unit = unit;
	pciedngl->rtedev = rtedev;
	pciedngl->osh = osh;

	pciedngl->sih = si_attach((uint)device, osh, regs, bus, NULL, NULL, 0);

	pciedngl->regs = regs;
	pciedngl->coreid = si_coreid(pciedngl->sih);
	pciedngl->corerev = si_corerev(pciedngl->sih);

	if (!(pciedngl->dngl = dngl_attach(pciedngl, NULL, pciedngl->sih, osh))) {
		printf("%s: dngl_attach failed\n", __FUNCTION__);
		goto fail;
	}
#ifndef RTE_POLL
	if (hnd_isr_register(0, coreid, unit, pciedngl_isr, rtedev, bus) ||
#ifdef THREAD_SUPPORT
//#error "DPC undefined"
#endif // endif
	    FALSE) {
		printf("pcidongle_probe:hnd_isr_register failed\n");
		goto fail;
	}
#endif /* RTE_POLL */

	pciedngl_ol_msg_attach(pciedngl);

	return pciedngl;

fail:
	if (pciedngl)
		MFREE(pciedngl->osh, pciedngl, sizeof(struct dngl_bus));
	ASSERT(0);
	return NULL;
}

static void pciedngl_ol_msg_attach(struct dngl_bus *pciedngl)
{
	volatile sbpcieregs_t *pcie2_regs = pciedngl->regs;

	/* Initialize shared message infra */
	pciedngl->msgbuf = (uchar *)(SI_PCI_MEM | (ppcie_shared->msgbufaddr_low&~SBTOPCIE0_MASK));
	W_REG(pciedngl->osh,
		&pcie2_regs->sbtopcie0, 0xc |(ppcie_shared->msgbufaddr_low&SBTOPCIE0_MASK));
	bcm_olmsg_init(&pciedngl->ol_info, (uchar *)pciedngl->msgbuf,
		ppcie_shared->msgbuf_sz, OLMSG_READ_DONGLE_INDEX, OLMSG_WRITE_DONGLE_INDEX);
	bcm_olmsg_dump_record(&pciedngl->ol_info);

	/* Initialize dongle packet pool to receive messages from Host */
	pciedngl_msgpool_attach(pciedngl);

}

static int pciedngl_msgpool_attach(struct dngl_bus *pciedngl)
{
	int err = BCME_OK;
	int n = SDHDR_POOL_LEN;

	if ((pciedngl->pktpool_pmsg = MALLOC(pciedngl->osh, sizeof(pktpool_t))) == NULL) {
		ASSERT(0);
		return BCME_ERROR;
	}

	bzero(pciedngl->pktpool_pmsg, sizeof(pktpool_t));

	err = pktpool_init(pciedngl->osh, pciedngl->pktpool_pmsg, &n,
		MSGBUFSZ, FALSE, lbuf_basic);
	if (err == BCME_ERROR) {
		ASSERT(0);
		MFREE(pciedngl->osh, pciedngl->pktpool_pmsg, sizeof(pktpool_t));
		pciedngl->pktpool_pmsg = NULL;
		return err;
	}

	pktpool_shared_msgs = pciedngl->pktpool_pmsg;

	if ((err == 0) && (n < SDHDR_POOL_LEN))
		printf("partial pkt pool allocated: %d %d\n", n, SDHDR_POOL_LEN);

	if (err)
	ASSERT(FALSE);

	return err;

}

/** called by HNDRTE */
static void
pciedngl_isr(void *cbdata)
{
	volatile uint32 intstatus;
	int ret;
	void * p;
	uint16 pktlen = 0, nextpktlen = 0;
	uchar *pktdata;
	hnd_dev_t *drv = (hnd_dev_t *)cbdata;
	struct dngl_bus *pciedngl = (struct dngl_bus *)drv->softc;
	volatile sbpcieregs_t *pcie2_regs = pciedngl->regs;

	intstatus = R_REG(pciedngl->osh, &pcie2_regs->intstatus);
	if (!(intstatus & PCIE_MB_TOPCIE_FN0_0)) {
		printf("%s: invalid ISR status: 0x%08x",
			__FUNCTION__, intstatus);

		return;
	}

	do {
		/*
		 * Clear the PCIE_MB_TOPCIE_FN0_0 bit to handle possible race
		 * between host and ARM
		 */
		W_REG(pciedngl->osh, &pcie2_regs->intstatus, PCIE_MB_TOPCIE_FN0_0);

		p = pktpool_get(pciedngl->pktpool_pmsg);
		if (p == NULL) {
			printf("%s: malloc failure\n", __FUNCTION__);
			ASSERT(0);
			break;
		}

		pktdata = (uint8 *) PKTDATA(pciedngl->osh, p);
		pktlen = bcm_olmsg_readmsg(&pciedngl->ol_info, (uchar *)pktdata, MSGBUFSZ);
		nextpktlen = bcm_olmsg_peekmsg_len(&pciedngl->ol_info);

		if (pktlen) {
			PKTSETLEN(pciedngl->osh, p, pktlen);
			if ((ret = dngl_dev_ioctl(pciedngl->dngl, 0, pktdata, pktlen)) < 0)
				printf("%s: error calling dngl_dev_ioctl: %d",
					__FUNCTION__, ret);
		}

		PKTFREE(pciedngl->osh, p, FALSE);
	} while (nextpktlen != 0);

	return;
}

static int
pciedngl_send(hnd_dev_t *src, hnd_dev_t *drv, struct lbuf *lb)
{
	struct dngl_bus *pciedngl = (struct dngl_bus *)drv->softc;

	if (pciedngl) {
		return dngl_sendpkt((void *)(pciedngl->dngl), src, (void *)lb);
	}
	else {
		printf("%s called: pciedngl is NULL\n", __FUNCTION__);
		ASSERT(0);
	}
	return 0;
}

int
pciedngl_bus_binddev(void *bus, void *dev, uint numslaves)
{
	struct dngl_bus *pciedngl = ((hnd_dev_t *)bus)->softc;
	return dngl_binddev(pciedngl->dngl, bus, dev, numslaves);
}

int
pciedngl_bus_unbinddev(void *bus, void *dev)
{
	struct dngl_bus *pciedngl = ((hnd_dev_t *)bus)->softc;
	return dngl_unbinddev(pciedngl->dngl, bus, dev);
}

int
pciedngl_bus_tx(struct dngl_bus *bus, void *p)
{
	uint16 pktlen = 0;
	uchar *pktdata;
	struct dngl_bus *pciedngl = (struct dngl_bus *)bus;

	if (pciedngl) {

		pktdata = (uchar *) PKTDATA(pciedngl->osh, p);
		pktlen = PKTLEN(pciedngl->osh, p);
		bcm_olmsg_writemsg(&pciedngl->ol_info, (uchar *)pktdata, pktlen);
		/* Interrupt Host */
		W_REG(pciedngl->osh, &pciedngl->regs->sbtopcimailbox, PCIE_MB_TOPCIE_FN0_0);
		PKTFREE(pciedngl->osh, p, FALSE);
	}
	else
		ASSERT(0);

	return pktlen;
}
