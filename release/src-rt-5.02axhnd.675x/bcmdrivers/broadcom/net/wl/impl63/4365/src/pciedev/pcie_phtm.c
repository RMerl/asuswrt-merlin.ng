/*
 * Broadcom PCIE device-side driver
 * Older gen of chips 4350b1 and lower, 4335c0 and lower depends on sdio/usb core dmas
 * to transfer data between host and TCM. Since its not standard way and will give way to
 * pcie dma, older set of devices are called phantom devices and kept in a different file.
 * pcie_phtm.c handles sdio/usb dma related register interface.
 * interrupt and dma trigger routines are different for phantom devices.
 * this file manages the routines required to handle interrupt and dma for pahntom devices.
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
 * $Id: pcie_phtm.c  $
 */
#include <typedefs.h>
#include <bcmdefs.h>
#include <bcmdevs.h>
#include <bcmendian.h>
#include <bcmutils.h>
#include <siutils.h>
#include <bcmnvram.h>
#include <sbhnddma.h>
#include <hnddma.h>
#include <osl.h>
#include <hndsoc.h>
#include <proto/ethernet.h>
#include <proto/802.1d.h>
#include <sbphantom.h>
#include <sbsdio.h>
#include <pcie_core.h>
#include <dngl_bus.h>
#include <dngl_api.h>
#include <bcmcdc.h>
#include <msgtrace.h>
#include <circularbuf.h>
#include <bcmpcie.h>
#include <bcmmsgbuf.h>
#include <hndpmu.h>
#include <pciedev.h>
#include <pciedev_dbg.h>

/* indexes into tunables array */
#define NTXD			1
#define NRXD			2
#define RXBUFS			3
#define RXBUFSZ			4
#define MAXTUNABLE		5

#ifndef PD_NTXD
#define PD_NTXD			256
#endif // endif
#ifndef PD_NRXD
#define PD_NRXD			256
#endif // endif
#define PD_RXBUF_SIZE	PKTBUFSZ

/* DMA RX OFFSET */
#define USB_RXOFFSET		8
#define SDIO_RXOFFSET		8

#define DEF_DMA_INTMASK		0x1dc00		/* dma errors and rx cmplt int mask */
#define SDIO_WAR_DUMMY_DMA_SIZE	256 /* size of dummy packet to fix sd dma hang issue */
#define DMA_ALIGN_LEN		4
#define SDIO_DMA_WAR_TX_SIZE SDIO_WAR_DUMMY_DMA_SIZE + DMA_ALIGN_LEN  /* TX descriptor size */
#define SDIO_DMA_WAR_RX_SIZE SDIO_WAR_DUMMY_DMA_SIZE + DMA_ALIGN_LEN  + SDIO_RXOFFSET /* RX size */

#define SDDMAREG(h, dir, chnl) \
	((dir) == DMA_TX ? \
	 (void *)(uintptr)&((h)->dma.sdiod64.dma64regs[chnl].xmt) : \
	 (void *)(uintptr)&((h)->dma.sdiod64.dma64regs[chnl].rcv))
#define USBDMAREG(h, dir, chnl) \
	((dir) == DMA_TX ? \
	 (void *)(uintptr)&((h)->dma64regs[chnl].dmaxmt) : \
	 (void *)(uintptr)&((h)->dma64regs[chnl].dmarcv))

typedef struct sd_dma_info {
	sdpcmd_regs_t *regs;		/* SDPCMD registers */
	hnddma_t *di;		/* DMA engine handle */
	uint *txavail;		/* Pointer to DMA txavail variable */
	uint *rxavail;		/* Pointer to DMA rxavail variable */
} sd_dma_info_t;
typedef struct usb_dma_info {
	usbdev_sb_regs_t *regs;         /* USB registers */
	hnddma_t *di;		/* DMA engine handle */
	uint *txavail;		/* Pointer to DMA txavail variable */
	uint *rxavail;		/* Pointer to DMA rxavail variable */
} usb_dma_info_t;

/* pcie phantom info data structure */
typedef struct pcie_phtm {
	struct dngl_bus *pciedev;
	usb_dma_info_t *usb_dma;		/* USB dma info */
	sd_dma_info_t *sd_dma;		/* SDIO dma info */
	bool sd_dma_pending;
	uint32 sd_dma_intstatus;
	uint32 usb_dma_intstatus;
	bool sddpc_sched;
	uint32	tunables[MAXTUNABLE];
	uint8 *dummy_rxoff;
	uint32 d2h_dma_rxoffset;
	osl_t *osh;
	si_t *sih;
	sbpcieregs_t *regs;		/* PCIE registers */
	pciedev_shared_t *pcie_sh;
	void *sdio_war_dummy_tx;
	void *sdio_war_dummy_rx;
} pcie_phtm_t;

static int pciedev_sd_dma_attach(struct pcie_phtm *phtm, uint32 rxoffset);
static int pciedev_usb_dma_attach(struct pcie_phtm *phtm);
static void pciedev_sddma_intrsoff(struct pcie_phtm *phtm);
static void pciedev_sddma_intrson(struct pcie_phtm *phtm);
static void pciedev_usbdma_intrson(struct pcie_phtm *phtm);
static void pciedev_usbdma_intrsoff(struct pcie_phtm *phtm);
static bool pciedev_sd_dma_dpc(struct pcie_phtm *phtm);
static bool pciedev_usb_dma_dpc(struct pcie_phtm *phtm);
static bool pciedev_sd_intr_dispatch(struct pcie_phtm *phtm);
static bool pciedev_usb_intr_dispatch(struct pcie_phtm *phtm);
static void pciedev_clear_sd_interrupt(struct pcie_phtm *phtm);
static void pciedev_clear_usb_interrupt(struct pcie_phtm *phtm);
static void phtm_dma_tunables_init(struct pcie_phtm *phtm);

/* Core attach function for pcie phantom device
 * Initialise sdio and usb dma
 * Initialise reg info for usb and sdio core
 * Initialise tunables
*/
struct pcie_phtm *
BCMATTACHFN(pcie_phtm_attach)(struct dngl_bus *pcie, osl_t *osh, si_t *sih,
	void *regs, pciedev_shared_t *pcie_sh, uint32 d2h_dma_rxoffset)
{
	pcie_phtm_t *phtm;

	/* allocate the phantom dev state */
	if (!(phtm = MALLOC(osh, sizeof(struct pcie_phtm)))) {
		PCI_ERROR(("pcie_phtm_attach: out of memory, malloced %d bytes\n",
			MALLOCED(osh)));
		return NULL;
	}

	/* Dummy rxoffset to handle rx header inserted by dma engine */
	/* for circular buffer, we dont need it inline with actual message */
	/* so just catch it in a dummy descriptor always which is ignored */
	phtm->dummy_rxoff = MALLOC(osh, SDIO_RXOFFSET);
	if (!phtm->dummy_rxoff) {
		PCI_ERROR(("pcie_phtm_attach: out of memory, malloced %d bytes\n",
			MALLOCED(osh)));
		return NULL;
	}

	phtm->osh = osh;
	phtm->sih = sih;
	phtm->regs = regs;
	phtm->pcie_sh = pcie_sh;
	phtm->d2h_dma_rxoffset = d2h_dma_rxoffset;

	/* pcie phantom dev tunables */
	phtm_dma_tunables_init(phtm);

	/* SDIO DMA ATTACH */
	if (pciedev_sd_dma_attach(phtm, d2h_dma_rxoffset))
		goto fail;

	/* USB DMA ATTACH */
	if (pciedev_usb_dma_attach(phtm))
		goto fail;

	phtm->pciedev = pcie;

	/* Temporary war for SDIO DMA hang with short sized packets */
	/* War to prevent sdio fifo running out of pkts */
	/* required only for sdio dma cases */
	phtm->sdio_war_dummy_tx = MALLOC(osh, SDIO_DMA_WAR_TX_SIZE);
	if (phtm->sdio_war_dummy_tx == NULL)
		goto fail;
	phtm->sdio_war_dummy_tx = ALIGN_ADDR(phtm->sdio_war_dummy_tx, DMA_ALIGN_LEN);

	phtm->sdio_war_dummy_rx = MALLOC(osh, SDIO_DMA_WAR_RX_SIZE);
	if (phtm->sdio_war_dummy_rx == NULL)
		goto fail;
	phtm->sdio_war_dummy_rx = ALIGN_ADDR(phtm->sdio_war_dummy_rx, DMA_ALIGN_LEN);

	return phtm;

fail:
	/* Detach  sequence */
	if (phtm->sdio_war_dummy_rx != NULL)
		MFREE(osh, phtm->sdio_war_dummy_rx, SDIO_DMA_WAR_RX_SIZE);

	if (phtm->sdio_war_dummy_tx != NULL)
		MFREE(osh, phtm->sdio_war_dummy_tx, SDIO_DMA_WAR_TX_SIZE);

	if (phtm->dummy_rxoff != NULL)
		MFREE(osh, phtm->dummy_rxoff, sizeof(SDIO_RXOFFSET));

	if (phtm != NULL)
		MFREE(osh, phtm, sizeof(struct pcie_phtm));

	return NULL;
}

/* Disable sdio dma interrupts */
static void
pciedev_sddma_intrsoff(struct pcie_phtm *phtm)
{
	W_REG(phtm->osh, &phtm->sd_dma->regs->intrcvlazy, 0);
	W_REG(phtm->osh, &phtm->sd_dma->regs->intmask, 0);
}

/* Switch on sdio dma interrupts */
static void
pciedev_sddma_intrson(struct pcie_phtm *phtm)
{
	W_REG(phtm->osh, &phtm->sd_dma->regs->intrcvlazy, 0x1 << IRL_FC_SHIFT);
	W_REG(phtm->osh, &phtm->sd_dma->regs->intmask, DEF_DMA_INTMASK);
}

/* Switch off usb dma interrupts */
static void
pciedev_usbdma_intrsoff(struct pcie_phtm *phtm)
{
	W_REG(phtm->osh, &phtm->usb_dma->regs->intrcvlazy[0], 0);
	W_REG(phtm->osh, &phtm->usb_dma->regs->dmaint[0].mask, 0);
}

/* Switch on usb dma interrupts */
static void
pciedev_usbdma_intrson(struct pcie_phtm *phtm)
{
	W_REG(phtm->osh, &phtm->usb_dma->regs->intrcvlazy[0], 0x1 << IRL_FC_SHIFT);
	W_REG(phtm->osh, &phtm->usb_dma->regs->dmaint[0].mask, DEF_DMA_INTMASK);
}

/* Phantom device init
 * Initiliaze sdio & usb dma engines
 * Turn on interrupts
*/
void
BCMATTACHFN(pcie_phtm_init)(struct pcie_phtm *phtm)
{
	/* SDIO DMA INIT */
	PCI_TRACE(("SDIO dma pciedev_init\n"));
	dma_fifoloopbackenable(phtm->sd_dma->di);
	dma_txinit(phtm->sd_dma->di);
	dma_rxinit(phtm->sd_dma->di);
	phtm->sd_dma->txavail = (uint *) dma_getvar(phtm->sd_dma->di, "&txavail");
	phtm->sd_dma->rxavail = (uint *) dma_getvar(phtm->sd_dma->di, "&rxavail");

	/*  USB DMA INIT */
	PCI_TRACE(("USB dma pciedev_init\n"));
	dma_fifoloopbackenable(phtm->usb_dma->di);
	dma_txinit(phtm->usb_dma->di);
	dma_rxinit(phtm->usb_dma->di);

	/* Record avail descriptors */
	phtm->usb_dma->txavail = (uint *) dma_getvar(phtm->usb_dma->di, "&txavail");
	phtm->usb_dma->rxavail = (uint *) dma_getvar(phtm->usb_dma->di, "&rxavail");

	/* Turn on dma interrupts */
	pciedev_sddma_intrson(phtm);
	pciedev_usbdma_intrson(phtm);

	return;
}

/* DMA attach and reg offset initialization */
static int
BCMATTACHFN(pciedev_usb_dma_attach)(pcie_phtm_t *phtm)
{
	/* Allocate usb dma info */
	phtm->usb_dma = MALLOC(phtm->osh, sizeof(usb_dma_info_t));

	if (phtm->usb_dma == NULL)
		goto fail;

	/* save usb regs info */
	phtm->usb_dma->regs = (void*)si_setcore(phtm->sih, USB20D_CORE_ID, 0);
	si_core_reset(phtm->sih, 0, 0);

	/* enable USB */
	W_REG(phtm->osh, &phtm->usb_dma->regs->devcontrol, 0x3fc2);

	/* Reset interrupts */
	pciedev_clear_usb_interrupt(phtm);

	/* dma attach */
	phtm->usb_dma->di = dma_attach(phtm->osh,
		"H2D",
		phtm->sih,
		USBDMAREG(phtm->usb_dma->regs, DMA_TX, 0),
		USBDMAREG(phtm->usb_dma->regs, DMA_RX, 0),
		phtm->tunables[NTXD], phtm->tunables[NRXD],
		phtm->tunables[RXBUFSZ], -1,
		phtm->tunables[RXBUFS], USB_RXOFFSET, NULL);

	if (!(phtm->usb_dma->di)) {
		PCI_ERROR(("pciedev_usb_dma_attach: dma_attach failed\n"));
		goto fail;
	}
	dma_burstlen_set(phtm->usb_dma->di, DMA_BL_64, DMA_BL_128);

	return 0;
fail:
	if (phtm->usb_dma)
		MFREE(phtm->osh, phtm->usb_dma, sizeof(usb_dma_info_t));
	return -1;
}

/* DMA attach and reg offset initialization */
static int
BCMATTACHFN(pciedev_sd_dma_attach)(pcie_phtm_t *phtm, uint32 rxoffset)
{
	/* Enable SDIO clock */
	/* Since there is no separate host controller, sdio clocl has to be driver by pll */
	si_pmu_chipcontrol(phtm->sih, 5, 0x20, 0x20);

	/* Alloc sdio dma info */
	phtm->sd_dma = MALLOC(phtm->osh, sizeof(sd_dma_info_t));

	if (phtm->sd_dma == NULL) {
		goto fail;
	}
	phtm->sd_dma->regs = (void*)si_setcore(phtm->sih, SDIOD_CORE_ID, 0);

	/* core reset */
	si_core_reset(phtm->sih, 0, 0);

	/* Reset interrupts */
	pciedev_clear_sd_interrupt(phtm);

	phtm->sd_dma->di = dma_attach(phtm->osh,
		"D2H",
		phtm->sih,
		SDDMAREG(phtm->sd_dma->regs, DMA_TX, 0),
		SDDMAREG(phtm->sd_dma->regs, DMA_RX, 0),
		phtm->tunables[NTXD], phtm->tunables[NRXD],
		phtm->tunables[RXBUFSZ], -1,
		phtm->tunables[RXBUFS], rxoffset, NULL);

	if (!(phtm->sd_dma->di)) {
		PCI_ERROR(("pciedev_sd_dma_attach: dma_attach failed\n"));
		goto fail;
	}
	dma_burstlen_set(phtm->sd_dma->di, DMA_BL_64, DMA_BL_64);
	return 0;
fail:
	if (phtm->sd_dma)
		MFREE(phtm->osh, phtm->sd_dma, sizeof(sd_dma_info_t));

	return -1;
}

/* Read usb intstatus and clear interrupt bits */
static uint32
pciedev_usb_intstatus(struct pcie_phtm *phtm)
{
	uint32 intstatus = R_REG(phtm->osh, &phtm->usb_dma->regs->dmaint[0].status);

	/* clear asserted device-side intstatus bits */
	W_REG(phtm->osh, &phtm->usb_dma->regs->dmaint[0].status, intstatus);

	intstatus = intstatus & DEF_DMA_INTMASK;

	return intstatus;
}

/* clear asserted device-side intstatus bits */
static void
pciedev_clear_usb_interrupt(struct pcie_phtm *phtm)
{
	W_REG(phtm->osh, &phtm->usb_dma->regs->dmaint[0].status, 0xFFFFFFFF);

}

/* clear asserted device-side intstatus bits */
static void
pciedev_clear_sd_interrupt(struct pcie_phtm *phtm)
{
	W_REG(phtm->osh, &phtm->sd_dma->regs->intstatus, 0xFFFFFFFF);

}

/* Read and clear sdio intstatus bits */
static uint32
pciedev_sd_intstatus(struct pcie_phtm *phtm)
{
	uint32 intstatus = R_REG(phtm->osh, &phtm->sd_dma->regs->intstatus);

	/* clear asserted device-side intstatus bits */
	W_REG(phtm->osh, &phtm->sd_dma->regs->intstatus, intstatus);

	intstatus = intstatus & DEF_DMA_INTMASK;

	return intstatus;
}

/* Dispatch interrupt events */
static bool
pciedev_sd_intr_dispatch(struct pcie_phtm *phtm)
{
	uint32 intstatus;

	intstatus = pciedev_sd_intstatus(phtm);

	if (!intstatus)
		return FALSE;

	phtm->sd_dma_intstatus = intstatus;
	return TRUE;
}

/* Dispatch usb interrupts */
static bool
pciedev_usb_intr_dispatch(struct pcie_phtm *phtm)
{
	uint32 intstatus;

	intstatus = pciedev_usb_intstatus(phtm);

	if (!intstatus)
		return FALSE;

	phtm->usb_dma_intstatus = intstatus;
	return TRUE;
}

/* core sdio dma dpc routine */
static bool
pciedev_sd_dma_dpc(struct pcie_phtm *phtm)
{
	uint32 intstatus;
	bool resched = FALSE;
	void * prev;

	intstatus = phtm->sd_dma_intstatus;
	phtm->sd_dma_intstatus = 0;
	if (intstatus & I_RI) {

		/* free up the rx descriptors */
		while ((prev = dma_getnextrxp(phtm->sd_dma->di, FALSE)));
		/* Move read pointers of dma engine */
		while ((prev = dma_getnexttxp(phtm->sd_dma->di, FALSE))) {
			if (prev == phtm->sdio_war_dummy_tx)
				continue;

			/* core handle routine for d2h dma completion */
			pciedev_handle_d2h_dmacomplete(phtm->pciedev, prev);
		}
	} else {
		PCI_ERROR(("pciedev_sd_dma_dpc : SDIO DMA Error : intstatus : %x \n", intstatus));
	}
	return resched;
}

/* core usb dma dpc routine
 * Sendup the message to be processed by message buffer protocol
 * Update circular buffer read/write pointers
*/
static bool
pciedev_usb_dma_dpc(struct pcie_phtm *phtm)
{
	uint32 intstatus;
	bool resched = FALSE;
	void * txpkt, *rxpkt, *rxpkt0;
	circularbuf_t *htod_lcl, *htod_msgbuf;
	cmn_msg_hdr_t * msg;
	uint16 dmalen;
	uint8 msgtype;

	intstatus = phtm->usb_dma_intstatus;
	phtm->usb_dma_intstatus = 0;
	htod_lcl = pciedev_htodlcl(phtm->pciedev);
	htod_msgbuf = pciedev_htodmsgbuf(phtm->pciedev);

	if (intstatus & I_RI) {
		while (1) {
			/* fetch tx packet */
			txpkt = dma_getnexttxp(phtm->usb_dma->di, FALSE);
			if (txpkt == NULL)
				break;

			/* Rx offset Pkt */
			rxpkt0 = dma_getnextrxp(phtm->usb_dma->di, FALSE);
			if (rxpkt0 == NULL)
				break;

			/* Real msgbuf  pkt */
			rxpkt = dma_getnextrxp(phtm->usb_dma->di, FALSE);
			if (rxpkt == NULL)
				break;

			ASSERT(rxpkt != phtm->dummy_rxoff);
			/* Retrieve queued up pkt length */
			dmalen = pciedev_htoddma_deque(phtm->pciedev, &msgtype);

			if (msgtype == MSG_TYPE_TX_PYLD) {
				pciedev_process_tx_payload(phtm->pciedev);
				continue;
			}

			msg = (cmn_msg_hdr_t *)rxpkt;
			if (BCM_SPLITBUF_ENAB() && MESSAGE_CTRLPATH(msg->msgtype)) {
				htod_lcl = pciedev_htodlcl_ctrl(phtm->pciedev);
				htod_msgbuf = pciedev_htod_ctrlbuf(phtm->pciedev);
			}
			else {
				htod_lcl = pciedev_htodlcl(phtm->pciedev);
				htod_msgbuf = pciedev_htodmsgbuf(phtm->pciedev);
			}

			if (MESSAGE_PAYLOAD(msg->msgtype)) {
				/* if its payload, skip circular buffer updates */
				pciedev_sendup(phtm->pciedev, rxpkt, dmalen);
			} else {
				/* We need to inform the htod_msgbuf that read is complete */
				circularbuf_read_complete(htod_msgbuf, (uint16)dmalen);
				/* Update the shared register with the new read ptr */
				if (BCM_SPLITBUF_ENAB() && MESSAGE_CTRLPATH(msg->msgtype)) {
					pciedev_set_h2dring_rxoffset(phtm->pciedev,
						CIRCULARBUF_READ_PTR(htod_msgbuf), TRUE);
				}
				else {
					pciedev_set_h2dring_rxoffset(phtm->pciedev,
						CIRCULARBUF_READ_PTR(htod_msgbuf), FALSE);
				}
				/* inform the the local_htod msgbuf that write is complete */
				circularbuf_write_complete(htod_lcl, dmalen);
			}
		}
	} else {
		PCI_ERROR(("pciedev_usb_dma_dpc : USB DMA Error : intstatus : %x \n", intstatus));
	}
	return resched;
}

/* Reset phantom dma engine */
void
BCMATTACHFN(pcie_phtm_reset)(struct pcie_phtm *phtm)
{
	/* SDIO DMA Reset */
	dma_rxreset(phtm->sd_dma->di);
	dma_txreset(phtm->sd_dma->di);

	dma_rxreclaim(phtm->sd_dma->di);
	dma_txreclaim(phtm->sd_dma->di, HNDDMA_RANGE_ALL);

	/* USB DMA Reset */
	dma_rxreset(phtm->usb_dma->di);
	dma_txreset(phtm->usb_dma->di);

	dma_rxreclaim(phtm->usb_dma->di);
	dma_txreclaim(phtm->usb_dma->di, HNDDMA_RANGE_ALL);
}

void
pcie_phtm_dma_sts_update(struct pcie_phtm *phtm, uint32 status)
{
	phtm->sd_dma_pending |= status;
}

/* dongle to host doorbell register */
void
phtm_ring_dtoh_doorbell(struct pcie_phtm *phtm)
{
	W_REG(phtm->osh, &phtm->regs->sbtopcimailbox, I_F0_B1);
}

void
pcie_phtm_bit0_intr_process(struct pcie_phtm *phtm)
{
	/* not implemented for now */
}

/* Core DMA routine to transfer messages/payload from TCM to HOST */
void
pcie_phtm_tx(struct pcie_phtm *phtm, void *src, dma64addr_t dst, uint16 msglen, uint8 l_msgtype)
{
	dma64addr_t haddr;

	/* RX descriptor programming with host address */
	/* high 32 OR ed with 0x80000000 to mark as pcie address */
	PHYSADDR64HISET(dst, PHYSADDR64HI(dst) | 0x80000000);
	if (dma_rxfast(phtm->sd_dma->di, dst, (uint32)msglen))
		PCI_ERROR(("pcie_phtm_tx : dma_rxfast failed \n"));

	/* TX descriptor programming with local TCM address */
	if (MESSAGE_PAYLOAD(l_msgtype)) {
		/* if lbuf, use legacy dma function, */
		/* we need to retrieve pkt address later to free up on dma complt */
		dma_txfast(phtm->sd_dma->di, src, TRUE);
	} else {
		PHYSADDR64HISET(haddr, (uint32) 0);
		PHYSADDR64LOSET(haddr, (uint32) src);
		if (dma_msgbuf_txfast(phtm->sd_dma->di, haddr, TRUE,
			msglen - phtm->d2h_dma_rxoffset))
			PCI_ERROR(("pcie_phtm_tx : dma fill failed  \n"));
	}

	/* Temporary war for SDIO DMA hang with short sized packets */
	/* War to prevent sdio fifo running out of pkts */
	/* required only for sdio dma cases */
	if (msglen < SDIO_WAR_DUMMY_DMA_SIZE) {
		PHYSADDR64HISET(haddr, 0);
		PHYSADDR64LOSET(haddr, (uint32) phtm->sdio_war_dummy_rx);
		if (dma_rxfast(phtm->sd_dma->di, haddr, SDIO_WAR_DUMMY_DMA_SIZE + SDIO_RXOFFSET))
			PCI_ERROR(("pcie_phtm_tx: %d: dma_rxfast failed \n", __LINE__));

		PHYSADDR64HISET(haddr, 0);
		PHYSADDR64LOSET(haddr, (uint32) phtm->sdio_war_dummy_tx);
		if (dma_msgbuf_txfast(phtm->sd_dma->di, haddr, TRUE, SDIO_WAR_DUMMY_DMA_SIZE))
			PCI_ERROR(("pcie_phtm_tx:%d: dma fill failed  \n", __LINE__));

	}
}

/* return available descriptors */
uint32
pciedev_get_avail_desc(struct dngl_bus *pciedev, uint8 direction, uint8 endpoint)
{
	struct pcie_phtm *phtm;
	phtm = pciedev_phtmdev(pciedev);

	if (direction == HTOD) {
		if (endpoint == RXDESC)
			return (*phtm->usb_dma->rxavail);
		else
			return (*phtm->usb_dma->txavail);

	} else {
		if (endpoint == RXDESC)
			return (*phtm->sd_dma->rxavail);
		else
			return (*phtm->sd_dma->txavail);
	}
}

/* RX- TX descriptor programming to fetch message from host circular buffer to local memory */
/* also used to transfer non inline ioctl request coming outside of message buffer */
void
pcie_phtm_msgbuf_dma(struct pcie_phtm *phtm, void *dst, dma64addr_t src, uint16 src_len)
{
	dma64addr_t haddr;

	/* RX offset descriptor */
	/* Dummy offset to handle 8 bytes of header info added by DMA */
	PHYSADDR64HISET(haddr, (uint32) 0);
	PHYSADDR64LOSET(haddr, (uint32) phtm->dummy_rxoff);
	if (dma_rxfast(phtm->usb_dma->di, haddr, 8))
		PCI_ERROR(("pcie_phtm_msgbuf_dma : dma_rxfast failed \n"));

	/* Rx descriptor */
	PHYSADDR64HISET(haddr, (uint32) 0);
	PHYSADDR64LOSET(haddr, (uint32) dst);
	if (dma_rxfast(phtm->usb_dma->di, haddr, src_len))
		PCI_ERROR(("pcie_phtm_msgbuf_dma : dma_rxfast failed \n"));

	/* Set the MSB of host addr to 1  - for phantom devices */
	/* this is to indicate its a pcie address */
	/* TX descriptor */
	PHYSADDR64HISET(src, PHYSADDR64HI(src) | 0x80000000);
	if (dma_msgbuf_txfast(phtm->usb_dma->di, src, TRUE, src_len))
		PCI_ERROR(("pcie_phtm_msgbuf_dma : dma fill failed  \n"));
}

void
pcie_phtm_sd_isr(struct dngl_bus *pciedev)
{
	pcie_phtm_t *phtm;

	phtm = pciedev_phtmdev(pciedev);

	/* call common first level interrupt handler */
	if (pciedev_sd_intr_dispatch(phtm)) {
		/* if more to do... */
		pciedev_sddma_intrsoff(phtm);
		pciedev_sd_dma_dpc(phtm);
		pciedev_sddma_intrson(phtm);
	}
}

void
pcie_phtm_usb_isr(struct dngl_bus *pciedev)
{
	pcie_phtm_t *phtm;

	phtm = pciedev_phtmdev(pciedev);

	/* call common first level interrupt handler */
	if (pciedev_usb_intr_dispatch(phtm)) {
		/* if more to do... */
		pciedev_usbdma_intrsoff(phtm);
		pciedev_usb_dma_dpc(phtm);
		pciedev_usbdma_intrson(phtm);
	}
}

/* tunables required for dma init */
static void
BCMATTACHFN(phtm_dma_tunables_init)(struct pcie_phtm *phtm)
{
	phtm->tunables[NTXD] = PD_NTXD;
	phtm->tunables[NRXD] = PD_NRXD;
	phtm->tunables[RXBUFS] = 32;
	phtm->tunables[RXBUFSZ] = PD_RXBUF_SIZE;
}
