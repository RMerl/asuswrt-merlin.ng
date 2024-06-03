/*
 * Generic Broadcom Home Networking Division (HND) DMA module.
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
 * $Id: hnddma.c 831031 2023-10-06 14:32:20Z $
 */

/**
 * @file
 * @brief
 * Source file for HNDDMA module. This file contains the functionality to initialize and run the
 * DMA engines in e.g. D11 and PCIe cores.
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

#include <sbhnddma.h>
#include <hnddma.h>
#include "hnddma_priv.h"
#ifdef BCMHME
#include <sbtopcie.h> // sbtopcie_cpy32()
#endif
#ifdef BCM_HWALITE
#include "mlc_export.h"
#include <d11.h>
#endif /* BCM_HWALITE */

/* default dma message level (if input msg_level pointer is null in dma_attach()) */
static uint dma_msg_level =
#ifdef BCMDBG_ERR
	1;
#else
	0;
#endif /* BCMDBG_ERR */

/* Common prototypes */
static bool _dma_isaddrext(dma_info_t *di);
static bool _dma_descriptor_align(dma_info_t *di);
static void _dma_ddtable_init(dma_info_t *di, uint direction, dmaaddr_t pa);

static uint8 dma_align_sizetobits(uint size, uint flags);
static void *dma_ringalloc(dma_info_t *di, uint32 boundary, uint size, uint16 *alignbits,
	uint* alloced, dmaaddr_t *descpa, osldma_t **dmah);
static bool _dma32_addrext(osl_t *osh, dma32regs_t *dma32regs);

/* Prototypes for 64-bit routines */
static bool dma64_alloc_tx(dma_info_t *di);
static bool dma64_alloc_rx(dma_info_t *di);

static bool _dma64_addrext(osl_t *osh, dma64regs_t *dma64regs);

static void dma_param_set_nrxpost(dma_info_t *di, uint16 paramval);
static void dma_descptr_update(hnddma_t *dmah);

/*
 * This function needs to be called during initialization before calling dma_attach_ext.
 */
dma_common_t *
BCMATTACHFN_DMA_ATTACH(dma_common_attach)(osl_t *osh, volatile uint32 *indqsel,
	volatile uint32 **suspreq, volatile uint32 **flushreq, uint reg_count,
	volatile uint32 *puqaqmringdepth, volatile uint32 *suspflush_grpsel)
{
	dma_common_t *dmac;
	int i;

	/* allocate private info structure */
	if ((dmac = MALLOCZ(osh, sizeof(*dmac))) == NULL) {
		DMA_NONE(("%s: out of memory, malloced %d bytes\n",
			__FUNCTION__, MALLOCED(osh)));
		return (NULL);
	}

	dmac->osh = osh;

	dmac->indqsel = indqsel;

	ASSERT(reg_count <= BCM_DMA_REQ_REG_COUNT);
	dmac->reg_count = reg_count;

	for (i = 0; i < reg_count; i++) {
		dmac->suspreq[i]  = *suspreq++;
		dmac->flushreq[i] = *flushreq++;
	}

	dmac->puqaqmringdepth = puqaqmringdepth;
	dmac->suspflush_grpsel = suspflush_grpsel;

	return dmac;
}

void
dma_common_detach(dma_common_t *dmacommon)
{
	if (dmacommon->sgmt) {
		MFREE(dmacommon->osh, (void *)dmacommon->sgmt,
			(dmacommon->sgmt_avail * sizeof(dma_hme_sgmt_t)));
	}

	/* free our private info structure */
	MFREE(dmacommon->osh, (void *)dmacommon, sizeof(*dmacommon));
}

#ifdef BCM_DMA_INDIRECT
/* Function to configure the q_index into indqsel register
 * to indicate to the DMA engine which queue is being selected
 * for updates or to read back status.
 * Also updates global dma common state
 * Caller should verify if the di is configured for indirect access.
 * If force is TRUE, the configuration will be done even though
 * global DMA state indicates it is the same as last_qsel
 */
void
dma_set_indqsel(hnddma_t *dmah, bool override)
{
	dma_info_t *di = DI_INFO(dmah);
	dma_common_t *dmacommon = di->dmacommon;
	ASSERT(di->indirect_dma == TRUE);

#ifdef BCMHWA
	if (DMA_HWA_TX(di)) {
		DMA_TRACE(("HWATX: DMA: SW try updating indqsel to <%d:%d>%s\n",
			dmacommon->last_qsel, di->q_index,
			(override) ? " override" : ""));
	}
#endif

	if ((dmacommon->last_qsel != di->q_index) || override) {
		/* configure the indqsel register */
		if (DMA_PUQ_QREMAP(di)) {
			/* PUQ use bit[15:8] MACTXQSel */
			W_REG(di->osh, dmacommon->indqsel, (di->q_index & 0xFF) << 8);
		} else {
			W_REG(di->osh, dmacommon->indqsel, di->q_index);
		}

		/* also update the global state dmac */
		dmacommon->last_qsel = di->q_index;
	}
}
#endif /* BCM_DMA_INDIRECT */

static void dma_attach_cond_config(dma_info_t *di);

/**
 * This is new dma_attach API to provide the option of using an Indirect DMA interface.
 * Initializes DMA data structures and hardware. Allocates rx and tx descriptor rings. Does not
 * allocate the buffers that the descriptors are going to point at. Does not initialize the
 * descriptors in the allocated ring.
 *
 * @param nrxd    Number of receive descriptors, must be a power of 2.
 * @param ntxd    Number of transmit descriptors, must be a power of 2.
 * @param nrxpost Number of empty receive buffers to keep posted to rx ring, must be <= nrxd - 1
 */
hnddma_t *
BCMATTACHFN_DMA_ATTACH(dma_attach_ext)(dma_common_t *dmac, osl_t *osh, const char *name,
	si_t *sih, volatile void *dmaregstx, volatile void *dmaregsrx, uint32 flags,
	uint8 qnum, uint ntxd, uint nrxd, uint rxbufsize, int rxextheadroom, uint nrxpost,
	uint rxoffset, uint *msg_level)
{
	dma_info_t *di;
	uint size;
	uint32 mask, reg_val;

	ASSERT(nrxpost <= nrxd - 1); /* because XXD macro counts up to nrxd - 1, not nrxd */

	/* allocate private info structure */
	if ((di = MALLOC(osh, sizeof (dma_info_t))) == NULL) {
#if defined(BCMDBG) || defined(WLC_BCMDMA_ERRORS)
		DMA_ERROR(("%s: out of memory, malloced %d bytes\n", __FUNCTION__, MALLOCED(osh)));
		OSL_SYS_HALT();
#endif
		return (NULL);
	}

	bzero(di, sizeof(dma_info_t));
	di->msg_level = msg_level ? msg_level : &dma_msg_level;

#ifdef BCM_DMA_INDIRECT
	if ((flags & BCM_DMA_IND_INTF_FLAG) && (dmac == NULL)) {
		DMA_ERROR(("%s: Inconsistent flags and dmac params \n", __FUNCTION__));
		ASSERT(0);
		goto fail;
	}
#endif /* ifdef BCM_DMA_INDIRECT */

	/* save dma_common */
	di->dmacommon = dmac;

#ifdef BCM_DMA_INDIRECT
	/* record indirect DMA interface  params */
	di->indirect_dma = (flags & BCM_DMA_IND_INTF_FLAG) ? TRUE : FALSE;
	di->q_index = qnum;

	if (DMA_INDIRECT(di)) {
		ASSERT((di->q_index & DMA_INDQSEL_IA) == 0);

		/* Cache queue mask and suspend/flush register addresses */
		if (flags & BCM_DMA_PUQ_QREMAP) {
			/* There are 3 groups to extend the fifo to 256.
			 * Group1: 0 ~ 95
			 * Group2: 96 ~ 191
			 * Group3: 192 ~ 255
			 */
			ASSERT(((qnum / 32) % 3) < dmac->reg_count);
		} else {
			ASSERT((qnum / 32) < dmac->reg_count);
		}
		di->q_index_mask = (1 << (qnum % 32));
		di->suspreq	 = dmac->suspreq[(qnum / 32) % 3];
		di->flushreq	 = dmac->flushreq[(qnum / 32) % 3];
	}
#endif /* BCM_DMA_INDIRECT */

	/* old chips w/o sb is no longer supported */
	ASSERT(sih != NULL);
	/* Number of DMA descriptors can be non power of 2, check flags and ASSERT */
	if (!(flags & BCM_DMA_NUM_DESC_NON_POWER_2))
		ASSERT(ISPOWEROF2(ntxd));
	ASSERT(ISPOWEROF2(nrxd));

	if (nrxd == 0)
		ASSERT(dmaregsrx == NULL);
	if (ntxd == 0)
		ASSERT(dmaregstx == NULL);

	/* init dma reg pointer */
	di->d64txregs = (dma64regs_t *)dmaregstx;
	di->d64rxregs = (dma64regs_t *)dmaregsrx;

	/* Default flags (which can be changed by the driver calling dma_ctrlflags
	 * before enable): For backwards compatibility both Rx Overflow Continue
	 * and Parity are DISABLED.
	 * supports it.
	 */
	dma_ctrlflags(&di->hnddma, DMA_CTRL_ROC | DMA_CTRL_PEN, 0);

	/* Default 16 bytes dd64 size */
	di->dd64_size = sizeof(dma64dd_t);

	/* check flags for descriptor only DMA */
	if (flags & BCM_DMA_DESC_ONLY_FLAG) {
		di->hnddma.dmactrlflags |= DMA_CTRL_DESC_ONLY_FLAG;

		/* Use 8 bytes AQM short descriptor */
		if (flags & BCM_DMA_DESC_ONLY_8B_FLAG) {
			di->hnddma.dmactrlflags |= DMA_CTRL_DESC_ONLY_8B_FLAG;
			di->dd64_size = sizeof(dma64dd_short_t);
		}
	}

	/* Modern-MACs (d11 corerev 64 and higher) supporting higher MOR need CS set. */
	if (flags & BCM_DMA_CHAN_SWITCH_EN) {
		di->hnddma.dmactrlflags |= DMA_CTRL_CS;
	}

	/* Supporting 0-255B receive frame offset. */
	if (flags & BCM_DMA_ROEXT_SUPPORT) {
		di->hnddma.dmactrlflags |= DMA_CTRL_ROEXT;
	}

	if (flags & BCM_DMA_RX_ALIGN_8BYTE) {
		di->hnddma.dmactrlflags |= DMA_CTRL_RX_ALIGN_8BYTE;
	}

#ifdef BULK_PKTLIST	/* Datapath bulk processing */
	if (flags & BCM_DMA_BULK_PROCESSING) {
		di->hnddma.dmactrlflags |= DMA_CTRL_BULK_PROCESSING;
	}
#endif

#ifdef BULKRX_PKTLIST	/* Datapath bulk processing */
	if (flags & BCM_DMA_BULKRX_PROCESSING) {
		di->hnddma.dmactrlflags |= DMA_CTRL_BULKRX_PROCESSING;
	}
#endif

	if (flags & BCM_DMA_DESC_BASE_IDX_UPD) {
		di->hnddma.dmactrlflags |= DMA_CTRL_DESC_BASE_IDX_UPD;
#ifdef WL_PUQ
		if (flags & BCM_DMA_PUQ_QREMAP) {
			di->hnddma.dmactrlflags |= DMA_CTRL_PUQ_QREMAP;
			/* When PUQ is enabled, read value is incorrect for TX.
			 * Set default value to 0.
			 */
			if (flags & BCM_DMA_PUQ_WAR) {
				di->hnddma.dmactrlflags |= DMA_CTRL_PUQ_WAR;
				di->xmtcontrol = 0x0;
			}
		}
#endif /* WL_PUQ */
	}

#ifdef BCMHWA
	if (nrxd && (flags & BCM_DMA_HWA_MACRXFIFO)) {
		di->hnddma.dmactrlflags |= DMA_CTRL_HWA_RX;
	}
	if (nrxd && (flags & BCM_DMA_HWA_HME_RX)) {
		di->hnddma.dmactrlflags |= DMA_CTRL_HWA_HME_RX;
	}
	if (ntxd && (flags & BCM_DMA_HWA_MACTXFIFO)) {
		di->hnddma.dmactrlflags |= DMA_CTRL_HWA_TX;
	}
	if (ntxd && (flags & BCM_DMA_HWA_HME_TX)) {
		di->hnddma.dmactrlflags |= DMA_CTRL_HWA_HME_TX;
	}
	if (flags & BCM_DMA_NUM_DESC_NON_POWER_2) {
		di->hnddma.dmactrlflags |= DMA_CTRL_NUM_DESC_NON_POWER_2;
	}

	if (dmaregstx && (flags & (BCM_DMA_HWA_HME_RX | BCM_DMA_HWA_HME_TX))) {
		dma_ctrlflags(&di->hnddma, DMA_CTRL_PEN, DMA_CTRL_PEN);
	}
#endif /* BCMHWA */

	if (flags & BCM_DMA_MLO_SKIP_ALLOC) {
		di->hnddma.dmactrlflags |= DMA_CTRL_SKIP_ALLOC;
	}

	if (flags & BCM_DMA_MLO_BI_BUS) {
		di->hnddma.dmactrlflags |= DMA_CTRL_MLO_BI_BUS;
	}

	if (flags & BCM_DMA_MLO_RXFIFO) {
		di->hnddma.dmactrlflags |= DMA_CTRL_MLO_RXFIFO;
	}

	if (flags & BCM_DMA_BOUNDRY64KB_WAR) {
		di->hnddma.dmactrlflags |= DMA_CTRL_BOUNDRY64KB_WAR;
	}

	DMA_TRACE(("%s: %s: osh %p flags 0x%x ntxd %d nrxd %d rxbufsize %d "
		   "rxextheadroom %d nrxpost %d rxoffset %d dmaregstx %p dmaregsrx %p\n",
		   name, __FUNCTION__,
		   OSL_OBFUSCATE_BUF(osh), di->hnddma.dmactrlflags, ntxd, nrxd,
		   rxbufsize, rxextheadroom, nrxpost, rxoffset, OSL_OBFUSCATE_BUF(dmaregstx),
		   OSL_OBFUSCATE_BUF(dmaregsrx)));
#ifdef BCM_DMA_INDIRECT
	DMA_TRACE(("%s: %s: indirect DMA %s, q_index %d \n",
		name, __FUNCTION__, (di->indirect_dma ? "TRUE" : "FALSE"), di->q_index));
#endif
	/* make a private copy of our callers name */
	strncpy(di->name, name, MAXNAMEL);
	di->name[MAXNAMEL-1] = '\0';

	di->osh = osh;
	di->sih = sih;

	/* save tunables */
	di->ntxd = (uint16)ntxd;
	di->nrxd = (uint16)nrxd;

	/* the actual dma size doesn't include the extra headroom */
	di->rxextrahdrroom = (rxextheadroom == -1) ? BCMEXTRAHDROOM : rxextheadroom;
	if (rxbufsize > BCMEXTRAHDROOM)
		di->rxbufsize = (uint16)(rxbufsize - di->rxextrahdrroom);
	else
		di->rxbufsize = (uint16)rxbufsize;

	/* If Buffer length is adjusted for alignment purpose
	 * Reflect the same in di->rxbufsize too
	 */
	if (di->hnddma.dmactrlflags & DMA_CTRL_RX_ALIGN_8BYTE) {
		if (di->rxbufsize >= BCM_DMA_RX_ALIGN_8BYTE_COUNT) {
			di->rxbufsize -= BCM_DMA_RX_ALIGN_8BYTE_COUNT;
		}
	}

	di->nrxpost = (uint16)nrxpost;
	di->rxoffset = (uint8)rxoffset;

	/* if this DMA channel is using indirect access, then configure the IndQSel register
	 * for this DMA channel
	 */
	if (DMA_INDIRECT(di)) {
		dma_set_indqsel((hnddma_t *)di, TRUE);
	}

	/* Get the default values (POR) of the burstlen. This can be overridden by the modules
	 * if this has to be different. Otherwise this value will be used to program the control
	 * register after the reset or during the init.
	 */
	/* For 4345 PCIE rev 5, writing all 1's to control is broken,
	 * you will read all 1's back
	 */

	if (dmaregsrx) {
		/* first disable the dma if not already done */
		reg_val = R_REG(di->osh, &di->d64rxregs->control);
		if (reg_val & 1) {
			reg_val &= ~1;
			W_REG(di->osh, &di->d64rxregs->control, reg_val);
			W_REG(di->osh, &di->d64rxregs->control, reg_val);
		}
		/* detect the dma descriptor address mask,
		 * should be 0x1fff before 4360B0, 0xffff start from 4360B0
		 */
		W_REG(di->osh, &di->d64rxregs->addrlow, 0xffffffff);
		/* For 4345 PCIE rev 5/8, need one more write to make it work */
		if ((si_coreid(di->sih) == PCIE2_CORE_ID) &&
			((si_corerev(di->sih) == 5) || (si_corerev(di->sih) == 8))) {
			W_REG(di->osh, &di->d64rxregs->addrlow, 0xffffffff);
		}
		mask = R_REG(di->osh, &di->d64rxregs->addrlow);

		if (mask & 0xfff)
			mask = R_REG(di->osh, &di->d64rxregs->ptr) | 0xf;
		else
			mask = 0x1fff;

		DMA_TRACE(("%s: dma_rx_mask: %08x\n", di->name, mask));
		di->d64_rs0_cd_mask = mask;
		di->d64_rs1_ad_mask = mask;

		if (mask == 0x1fff) {
			ASSERT(nrxd <= D64MAXDD);
		} else {
			if (di->hnddma.dmactrlflags & DMA_CTRL_DESC_BASE_IDX_UPD) {
				ASSERT(nrxd <= D64MAXDD_LARGE_IDX_MODE);
			} else {
				ASSERT(nrxd <= D64MAXDD_LARGE_BYTE_MODE);
			}
		}

		di->rxburstlen = (R_REG(di->osh,
			&di->d64rxregs->control) & D64_RC_BL_MASK) >> D64_RC_BL_SHIFT;
		di->rxwaitforcomplt = (R_REG(di->osh,
			&di->d64rxregs->control) & D64_RC_WAITCMP_MASK) >>
			D64_RC_WAITCMP_SHIFT;
		di->rxprefetchctl = (R_REG(di->osh,
			&di->d64rxregs->control) & D64_RC_PC_MASK) >> D64_RC_PC_SHIFT;
		di->rxprefetchthresh = (R_REG(di->osh,
			&di->d64rxregs->control) & D64_RC_PT_MASK) >> D64_RC_PT_SHIFT;
	}
	if (dmaregstx) {
		if (DMA_PUQ_WAR(di)) {
			/* When PUQ is enabled, read value may not be correct,
			 * So set the default value manually.
			 */
			mask = 0xffff;
		} else {
			/* first disable the dma if not already done */
			reg_val = R_REG(di->osh, &di->d64txregs->control);
			if (reg_val & 1) {
				reg_val &= ~1;
				W_REG(di->osh, &di->d64txregs->control, reg_val);
				W_REG(di->osh, &di->d64txregs->control, reg_val);
			}

			/* detect the dma descriptor address mask,
			 * should be 0x1fff before 4360B0, 0xffff start from 4360B0
			 */
			W_REG(di->osh, &di->d64txregs->addrlow, 0xffffffff);
			/* For 4345 PCIE rev 5/8, need one more write to make it work */
			if ((si_coreid(di->sih) == PCIE2_CORE_ID) &&
				((si_corerev(di->sih) == 5) || (si_corerev(di->sih) == 8))) {
				W_REG(di->osh, &di->d64txregs->addrlow, 0xffffffff);
			}
			mask = R_REG(di->osh, &di->d64txregs->addrlow);

			if (mask & 0xfff) {
				mask = R_REG(di->osh, &di->d64txregs->ptr) | 0xf;
			} else {
				mask = 0x1fff;
			}
		}

		/* When PUQ is enabled, 4 bits used for wrapper around and
		 * 12 bits used for AQM LD/CD/AD
		 */
		if (DMA_PUQ_QREMAP(di) && DMA_AQM_DESC(di)) {
			mask &= 0xfff;
		}

		DMA_TRACE(("%s: dma_tx_mask: %08x\n", di->name, mask));
		di->d64_xs0_cd_mask = mask;
		di->d64_xs1_ad_mask = mask;

		if (mask == 0x1fff) {
			ASSERT(ntxd <= D64MAXDD);
		} else {
			if (di->hnddma.dmactrlflags & DMA_CTRL_DESC_BASE_IDX_UPD) {
				if (DMA_PUQ_QREMAP(di) && DMA_AQM_DESC(di)) {
					ASSERT(ntxd <= (D64MAXDD_LARGE_IDX_MODE >> 4));
				} else {
					ASSERT(ntxd <= D64MAXDD_LARGE_IDX_MODE);
				}
			} else {
				ASSERT(ntxd <= D64MAXDD_LARGE_BYTE_MODE);
			}
		}

		if (DMA_PUQ_WAR(di)) {
			/* When PUQ is enabled, read value may not be correct,
			 * So set the default value manually.
			 */
			di->txburstlen = 0;
			di->txmultioutstdrd = 0;
			di->txprefetchctl = 0;
			di->txprefetchthresh = 0;
			di->txchanswitch = 1;
		} else {
			di->txburstlen = (R_REG(di->osh,
				&di->d64txregs->control) & D64_XC_BL_MASK) >> D64_XC_BL_SHIFT;
			di->txmultioutstdrd = (R_REG(di->osh,
				&di->d64txregs->control) & D64_XC_MR_MASK) >> D64_XC_MR_SHIFT;
			di->txprefetchctl = (R_REG(di->osh,
				&di->d64txregs->control) & D64_XC_PC_MASK) >> D64_XC_PC_SHIFT;
			di->txprefetchthresh = (R_REG(di->osh,
				&di->d64txregs->control) & D64_XC_PT_MASK) >> D64_XC_PT_SHIFT;
			di->txchanswitch = (R_REG(di->osh,
				&di->d64txregs->control) & D64_XC_CS_MASK) >> D64_XC_CS_SHIFT;
		}
	}

	/*
	 * figure out the DMA physical address offset for dd and data
	 *     PCI/PCIE: they map silicon backplace address to zero based memory, need offset
	 *     Other bus: use zero
	 *     SI_BUS BIGENDIAN kludge: use sdram swapped region for data buffer, not descriptor
	 */
	di->ddoffsetlow = 0;
	di->dataoffsetlow = 0;
	/* for pci bus, add offset */
	if (sih->bustype == PCI_BUS) {
		if ((BUSCORETYPE(sih->buscoretype) == PCIE_CORE_ID ||
		     BUSCORETYPE(sih->buscoretype) == PCIE2_CORE_ID)) {
			/* pcie with DMA64 */
			di->ddoffsetlow = 0;
			di->rxddoffsethigh = SI_PCIE_DMA_H32;
			di->txddoffsethigh = SI_PCIE_DMA_H32;
		} else {
			/* pci(DMA32/DMA64) or pcie with DMA32 */
			di->ddoffsetlow = SI_PCI_DMA;
			di->rxddoffsethigh = 0;
			di->txddoffsethigh = 0;
		}
		di->dataoffsetlow =  di->ddoffsetlow;
		di->dataoffsethigh =  di->rxddoffsethigh;
	} else {
		if (DMA_HWA_HME_RX(di) || DMA_HWA_HME_TX(di)) {
			/* Must be PCIE */
			if ((BUSCORETYPE(sih->buscoretype) != PCIE_CORE_ID &&
				BUSCORETYPE(sih->buscoretype) != PCIE2_CORE_ID)) {
				ASSERT(0);
			} else {
				/* Don't care dataoffsethigh since HWA managed FIFOs don't use it */
				if (DMA_HWA_HME_RX(di))
					di->rxddoffsethigh = SI_PCIE_DMA_H32;
				if (DMA_HWA_HME_TX(di))
					di->txddoffsethigh = SI_PCIE_DMA_H32;
			}
		} else {
			/* Add SI_PCIE_DMA_H32 to MLO DMA channel if MAP is not a PCI_BUS
			 * For example: Embedded:MAP:6765 + PCIe:AAP:6711.
			 * The 6765 MAP need to set bit 63 to the hi address of a
			 * descriptor entry so that the 6711 MAC can access it.
			 * 6765 MAC can access the hi address with bit 63 set in A1
			 * but A0 is not supported.
			 */
			if (di->hnddma.dmactrlflags & DMA_CTRL_MLO_BI_BUS) {
				di->dataoffsethigh = SI_PCIE_DMA_H32;
			}
		}
	}

#ifndef DSLCPE
#if defined(__mips__) && defined(IL_BIGENDIAN)
	/* use sdram swapped region for data buffers but not dma descriptors.
	 * this assumes that we are running on a 47xx mips with a swap window.
	 * But __mips__ is too general, there should be one si_ishndmips() checking
	 * for OUR mips
	 */
	di->dataoffsetlow = di->dataoffsetlow + SI_SDRAM_SWAPPED;
#endif /* defined(__mips__) && defined(IL_BIGENDIAN) */
#endif /* DSLCPE */
	/* WAR64450 : DMACtl.Addr ext fields are not supported in SDIOD core. */
	if ((si_coreid(sih) == SDIOD_CORE_ID) && ((si_corerev(sih) > 0) && (si_corerev(sih) <= 2)))
		di->addrext = 0;
	else if ((si_coreid(sih) == I2S_CORE_ID) &&
	         ((si_corerev(sih) == 0) || (si_corerev(sih) == 1)))
		di->addrext = 0;
	else
		di->addrext = _dma_isaddrext(di);

	/* does the descriptors need to be aligned and if yes, on 4K/8K or not */
	di->aligndesc_4k = _dma_descriptor_align(di);
	if (di->aligndesc_4k) {
		di->dmadesc_align = D64RINGALIGN_BITS;
		if ((ntxd < D64MAXDD / 2) && (nrxd < D64MAXDD / 2)) {
			/* for smaller dd table, HW relax the alignment requirement */
			di->dmadesc_align = D64RINGALIGN_BITS  - 1;
		}
	} else {
		/* The start address of descriptor table should be algined to cache line size,
		 * or other structure may share a cache line with it, which can lead to memory
		 * overlapping due to cache write-back operation. In the case of MIPS 74k, the
		 * cache line size is 32 bytes.
		 */
#ifdef __mips__
		di->dmadesc_align = 5;	/* 32 byte alignment */
#else
		di->dmadesc_align = 4;	/* 16 byte alignment */

		/* Aqm txd table alignment 4KB, limitation comes from the bug of CD,
		 * see DMA_CTRL_DESC_CD_WAR and it will be fixed in C0
		 */
		if ((di->hnddma.dmactrlflags & (DMA_CTRL_DESC_ONLY_FLAG | DMA_CTRL_DESC_CD_WAR)) ==
			(DMA_CTRL_DESC_ONLY_FLAG | DMA_CTRL_DESC_CD_WAR)) {
			di->dmadesc_align = 12;	/* 4K byte alignment */
		}
#endif
	}

	DMA_NONE(("DMA descriptor align_needed %d, align %d\n",
		di->aligndesc_4k, di->dmadesc_align));

#ifdef BULK_PKTLIST_DEBUG
	/* allocate tx packet pointer vector
	 * Descriptor only DMAs do not need packet pointer array
	 *
	 * When BULK_PKTLIST_DEBUG is enabled both the TXP[] array and
	 * the linked list impmenentations are turned on.
	 *
	 */
	if (!DMA_AQM_DESC(di) && (ntxd)) {
		size = ntxd * sizeof(void *);
		if ((di->txp = MALLOC(osh, size)) == NULL) {
			DMA_ERROR(("%s: %s: out of tx memory, malloced %d bytes\n",
				di->name, __FUNCTION__, MALLOCED(osh)));
			goto fail;
		}
		bzero(di->txp, size);
	}
#else
	/* allocate tx packet pointer vector
	 * Descriptor only DMAs do not need packet pointer array
	 *
	 * Skip txp[] allocation if bulk processing is enabled
	 */
	if (DMA_BULK_PATH(di)) {
		di->txp = NULL;
	} else if (!DMA_AQM_DESC(di) && (ntxd) && !DMA_HWA_TX(di)) {
		size = ntxd * sizeof(void *);
		if ((di->txp = MALLOC(osh, size)) == NULL) {
			DMA_ERROR(("%s: %s: out of tx memory, malloced %d bytes\n",
				di->name, __FUNCTION__, MALLOCED(osh)));
			goto fail;
		}
		bzero(di->txp, size);
	}
#endif /* BULK_PKTLIST_DEBUG */

	/* allocate rx packet pointer vector */
	/* HWA doesn't need di->rxp */
	if (nrxd && !DMA_HWA_RX(di)) {
		size = nrxd * sizeof(void *);
		if ((di->rxp = MALLOC(osh, size)) == NULL) {
			DMA_ERROR(("%s: %s: out of rx memory, malloced %d bytes\n",
				di->name, __FUNCTION__, MALLOCED(osh)));
			goto fail;
		}
		bzero(di->rxp, size);
	}

	/* allocate transmit descriptor ring, only need ntxd descriptors but it must be aligned */
	if (!(flags & BCM_DMA_MLO_SKIP_ALLOC) &&
		(ntxd && !DMA_HWA_HME_TX(di))) {
		if (!dma64_alloc_tx(di)) /* does not allocate buffers nor init descriptors */
			goto fail;
	}

	/* allocate receive descriptor ring, only need nrxd descriptors but it must be aligned */
	if (nrxd && !DMA_HWA_HME_RX(di)) {
		if (!dma64_alloc_rx(di)) /* does not allocate buffers nor init descriptors */
			goto fail;
	}

	if ((di->ddoffsetlow != 0) && !di->addrext) {
		if (PHYSADDRLO(di->txdpa) > SI_PCI_DMA_SZ) {
			DMA_ERROR(("%s: %s: txdpa 0x%x: addrext not supported\n",
			           di->name, __FUNCTION__, (uint32)PHYSADDRLO(di->txdpa)));
			goto fail;
		}
		if (PHYSADDRLO(di->rxdpa) > SI_PCI_DMA_SZ) {
			DMA_ERROR(("%s: %s: rxdpa 0x%x: addrext not supported\n",
			           di->name, __FUNCTION__, (uint32)PHYSADDRLO(di->rxdpa)));
			goto fail;
		}
	}

	DMA_TRACE(("ddoffsetlow 0x%x rxddoffsethigh 0x%x txddoffsethigh 0x%x "
	           "dataoffsetlow 0x%x dataoffsethigh 0x%x addrext %d\n",
	           di->ddoffsetlow, di->rxddoffsethigh, di->txddoffsethigh,
	           di->dataoffsetlow, di->dataoffsethigh, di->addrext));

	if (!di->aligndesc_4k) {
		di->xmtptrbase = PHYSADDRLO(di->txdpa);
		di->rcvptrbase = PHYSADDRLO(di->rxdpa);
	}

	/* allocate DMA mapping vectors */
	if (DMASGLIST_ENAB) {
		if (ntxd && !DMA_HWA_TX(di)) {
			size = ntxd * sizeof(hnddma_seg_map_t);
			if ((di->txp_dmah = (hnddma_seg_map_t *)MALLOC(osh, size)) == NULL)
				goto fail;
			bzero(di->txp_dmah, size);
		}

		/* We don't support sglist when HWA is enabled */
		if (nrxd && !DMA_HWA_RX(di)) {
			size = nrxd * sizeof(hnddma_seg_map_t);
			if ((di->rxp_dmah = (hnddma_seg_map_t *)MALLOC(osh, size)) == NULL)
				goto fail;
			bzero(di->rxp_dmah, size);
		}

		ASSERT((di->hnddma.dmactrlflags & DMA_CTRL_MLO_RXFIFO) == 0);
	}

	dma_attach_cond_config(di);

	/* Debug for DMA descriptor protocol error in Router NIC driver */
	if (flags & BCM_DMA_COHERENT_PER_TRANS) {
		di->trans_coherent = 1;
	}

#if defined(BULK_DESCR_FLUSH)
	/* init the bulk_descr_tx_start_txout ... no DMA USER BULK TX ACTIVE */
	DMA_BULK_DESCR_TX_SET_INVALID(di);
#endif

	return ((hnddma_t *)di);

fail:
#ifdef WLC_BCMDMA_ERRORS
	DMA_ERROR(("%s: %s: DMA attach failed \n", di->name, __FUNCTION__));
	OSL_SYS_HALT();
#endif
	dma_detach((hnddma_t *)di);
	return (NULL);
}

/* conditional configuration */
static void
dma_attach_cond_config(dma_info_t *di)
{
#ifdef BCM_DMAPAD
	#if defined(BCMSDIODEV_ENABLED)
		di->dmapad_required = TRUE;
	#else
		di->dmapad_required = FALSE;
	#endif /* BCMSDIODEV_ENABLED */
#else
	di->dmapad_required = FALSE;
#endif /* BCM_DMAPAD */

#ifdef BCM_DMA_TRANSCOHERENT
	di->trans_coherent = 1;
#else
	di->trans_coherent = 0;
#endif /* BCM_DMA_TRANSCOHERENT */
}

/* This is the legacy dma_attach() API. The interface is kept as-is for
 * legacy code ("et" driver) that does not need the dma_common_t support
 * interface
 */
hnddma_t *
BCMATTACHFN_DMA_ATTACH(dma_attach)(osl_t *osh, const char *name, si_t *sih,
	volatile void *dmaregstx, volatile void *dmaregsrx,
	uint ntxd, uint nrxd, uint rxbufsize, int rxextheadroom, uint nrxpost,
	uint rxoffset, uint *msg_level)
{
	return (hnddma_t *)(dma_attach_ext(NULL, osh, name, sih, dmaregstx, dmaregsrx,
		0, 0, ntxd, nrxd, rxbufsize, rxextheadroom, nrxpost, rxoffset, msg_level));
} /* dma_attach */

static bool
_dma32_addrext(osl_t *osh, dma32regs_t *dma32regs)
{
	uint32 w;

	OR_REG(osh, &dma32regs->control, XC_AE);
	w = R_REG(osh, &dma32regs->control);
	AND_REG(osh, &dma32regs->control, ~XC_AE);
	return ((w & XC_AE) == XC_AE);
}

/** !! may be called with core in reset */
void
dma_detach(hnddma_t *dmah)
{
	dma_info_t *di = DI_INFO(dmah);

	DMA_TRACE(("%s: dma_detach\n", di->name));

	/* shouldn't be here if descriptors are unreclaimed */
	ASSERT(di->txin == di->txout);
	ASSERT(di->rxin == di->rxout);

	/* free dma descriptor rings */
	if (di->txd64 && !DMA_HWA_HME_TX(di) &&
			!(di->hnddma.dmactrlflags & DMA_CTRL_SKIP_ALLOC)) {
		DMA_FREE_CONSISTENT(di->osh, ((int8 *)(uintptr)di->txd64 - di->txdalign),
		                    di->txdalloc, (di->txdpaorig), di->tx_dmah);
	}

	if (di->rxd64 && !DMA_HWA_HME_RX(di)) {
		DMA_FREE_CONSISTENT(di->osh, ((int8 *)(uintptr)di->rxd64 - di->rxdalign),
		                    di->rxdalloc, (di->rxdpaorig), di->rx_dmah);
	}

	/* free packet pointer vectors */
	if (di->txp)
		MFREE(di->osh, (void *)di->txp, (di->ntxd * sizeof(void *)));
	if (di->rxp)
		MFREE(di->osh, (void *)di->rxp, (di->nrxd * sizeof(void *)));

	/* free tx packet DMA handles */
	if (di->txp_dmah)
		MFREE(di->osh, (void *)di->txp_dmah, di->ntxd * sizeof(hnddma_seg_map_t));

	/* free rx packet DMA handles */
	if (di->rxp_dmah)
		MFREE(di->osh, (void *)di->rxp_dmah, di->nrxd * sizeof(hnddma_seg_map_t));

	/* free our private info structure */
	MFREE(di->osh, (void *)di, sizeof(dma_info_t));

}

static bool
BCMATTACHFN_DMA_ATTACH(_dma_descriptor_align)(dma_info_t *di)
{
	uint32 addrl;

	if (DMA_PUQ_WAR(di)) {
		return FALSE;
	}

	/* if this DMA channel is using indirect access, then configure the IndQSel register
	 * for this DMA channel
	 */
	if (DMA_INDIRECT(di)) {
		dma_set_indqsel((hnddma_t *)di, FALSE);
	}

	/* Check to see if the descriptors need to be aligned on 4K/8K or not */
	if (di->d64txregs != NULL) {
		W_REG(di->osh, &di->d64txregs->addrlow, 0xff0);
		addrl = R_REG(di->osh, &di->d64txregs->addrlow);
		if (addrl != 0)
			return FALSE;
	} else if (di->d64rxregs != NULL) {
		W_REG(di->osh, &di->d64rxregs->addrlow, 0xff0);
		addrl = R_REG(di->osh, &di->d64rxregs->addrlow);
		if (addrl != 0)
			return FALSE;
	}
	return TRUE;
}

/** return TRUE if this dma engine supports DmaExtendedAddrChanges, otherwise FALSE */
static bool
BCMATTACHFN_DMA_ATTACH(_dma_isaddrext)(dma_info_t *di)
{
	if (DMA_PUQ_WAR(di)) {
		return TRUE;
	}

	/* DMA64 supports full 32- or 64-bit operation. AE is always valid */

	/* not all tx or rx channel are available */
	if (di->d64txregs != NULL) {
		/* if using indirect DMA access, then configure IndQSel */
		if (DMA_INDIRECT(di)) {
			dma_set_indqsel((hnddma_t *)di, FALSE);
		}
		if (!_dma64_addrext(di->osh, di->d64txregs)) {
			DMA_ERROR(("%s: _dma_isaddrext: DMA64 tx doesn't have AE set\n",
				di->name));
			ASSERT(0);
		}
		return TRUE;
	} else if (di->d64rxregs != NULL) {
		if (!_dma64_addrext(di->osh, di->d64rxregs)) {
			DMA_ERROR(("%s: _dma_isaddrext: DMA64 rx doesn't have AE set\n",
				di->name));
			ASSERT(0);
		}
		return TRUE;
	}
	return FALSE;
}

/** initialize descriptor table base address */
static void
_dma_ddtable_init(dma_info_t *di, uint direction, dmaaddr_t pa)
{
	if (!di->aligndesc_4k) {
		if (direction == DMA_TX)
			di->xmtptrbase = PHYSADDRLO(pa);
		else
			di->rcvptrbase = PHYSADDRLO(pa);
	}
	/* if this DMA channel is using indirect access, then configure the IndQSel register
	 * for this DMA channel
	 */
	if (DMA_INDIRECT(di)) {
		dma_set_indqsel((hnddma_t *)di, FALSE);
	}
	/* BCMDMA64OSL needed for PA's above beyond low 4GB */
	ASSERT(sizeof(PHYSADDRLO(pa)) > sizeof(uint32) ?
			(PHYSADDRLO(pa) >> 32) == 0 : TRUE);

	if (DMA_PUQ_QREMAP(di) && (direction == DMA_TX)) {
		/* When PUQ is enabled,
		 * initialize the xmtptr and puqaqmringdepth for AQM descriptor ring.
		 * and initialize xmtptr for TXDMA descriptor ring.
		 */
		if (DMA_AQM_DESC(di)) {
			dma_common_t *dmacommon = di->dmacommon;
			di->desc_wrap_ind = 0;
			W_REG(di->osh, dmacommon->puqaqmringdepth, (uint32)di->ntxd);
			W_REG(di->osh, &di->d64txregs->ptr,
				(uint32)(B2I(di->xmtptrbase, di->dd64_size) & 0xfff));
		} else {
			W_REG(di->osh, &di->d64txregs->ptr,
				(uint32)(B2I(di->xmtptrbase, di->dd64_size) & 0xffff));
		}
	}

	/* Hybrid DMA descriptor table handling.
	 * In dongle 32bits system, the PHYSADDRHI macro returns zero which
	 * cannot handle hybrid DMA descriptor table allocated from Host DDR (HME).
	 * Add HME high address through PHYSADDR64
	 */
	if (((direction == DMA_TX) && DMA_HWA_HME_TX(di)) ||
		((direction == DMA_RX) && DMA_HWA_HME_RX(di))) {

		ASSERT(PHYSADDRHI(pa) == 0);

		if (direction == DMA_TX) {
			ASSERT(!(di->txdpahi & PCI32ADDR_HIGH));

			W_REG(di->osh, &di->d64txregs->addrlow,
				(uint32)(PHYSADDRLO(pa) + di->ddoffsetlow));
			W_REG(di->osh, &di->d64txregs->addrhigh,
				(di->txdpahi + di->txddoffsethigh));
		} else {
			ASSERT(!(di->rxdpahi & PCI32ADDR_HIGH));

			W_REG(di->osh, &di->d64rxregs->addrlow,
				(uint32)(PHYSADDRLO(pa) + di->ddoffsetlow));
			W_REG(di->osh, &di->d64rxregs->addrhigh,
				(di->rxdpahi + di->rxddoffsethigh));
		}
	} else if ((di->ddoffsetlow == 0) || !(PHYSADDRLO(pa) & PCI32ADDR_HIGH)) {
		if (direction == DMA_TX) {
			W_REG(di->osh, &di->d64txregs->addrlow, (uint32)(PHYSADDRLO(pa) +
			                                         di->ddoffsetlow));
			W_REG(di->osh, &di->d64txregs->addrhigh, (PHYSADDRHI(pa) +
			                                          di->txddoffsethigh));
		} else {
			W_REG(di->osh, &di->d64rxregs->addrlow, (uint32)(PHYSADDRLO(pa) +
			                                         di->ddoffsetlow));
			W_REG(di->osh, &di->d64rxregs->addrhigh, (PHYSADDRHI(pa) +
			                                          di->rxddoffsethigh));
		}
	} else {
		/* DMA64 32bits address extension */
		uint32 ae;
		ASSERT(di->addrext);
		ASSERT(PHYSADDRHI(pa) == 0);

		/* shift the high bit(s) from pa to ae */
		ae = (PHYSADDRLO(pa) & PCI32ADDR_HIGH) >> PCI32ADDR_HIGH_SHIFT;
		PHYSADDRLO(pa) &= ~PCI32ADDR_HIGH;

		if (direction == DMA_TX) {
			W_REG(di->osh, &di->d64txregs->addrlow, (uint32)(PHYSADDRLO(pa) +
			                                         di->ddoffsetlow));
			W_REG(di->osh, &di->d64txregs->addrhigh, di->txddoffsethigh);
			SET_REG(di->osh, &di->d64txregs->control, D64_XC_AE,
				(ae << D64_XC_AE_SHIFT));
		} else {
			W_REG(di->osh, &di->d64rxregs->addrlow, (uint32)(PHYSADDRLO(pa) +
			                                         di->ddoffsetlow));
			W_REG(di->osh, &di->d64rxregs->addrhigh, di->rxddoffsethigh);
			SET_REG(di->osh, &di->d64rxregs->control, D64_RC_AE,
				(ae << D64_RC_AE_SHIFT));
		}
	}
}

void
dma_fifoloopbackenable(hnddma_t *dmah)
{
	dma_info_t *di = DI_INFO(dmah);

	DMA_TRACE(("%s: dma_fifoloopbackenable\n", di->name));

	/* if this DMA channel is using indirect access, then configure the IndQSel register
	 * for this DMA channel
	 */
	if (DMA_INDIRECT(di)) {
		dma_set_indqsel(dmah, FALSE);
	}
	if (DMA_PUQ_WAR(di)) {
		di->xmtcontrol |= D64_XC_LE;
		W_REG(di->osh, &di->d64txregs->control, di->xmtcontrol);
	} else {
		OR_REG(di->osh, &di->d64txregs->control, D64_XC_LE);
	}
}

void
dma_rxinit(hnddma_t *dmah)
{
	dma_info_t *di = DI_INFO(dmah);

	DMA_TRACE(("%s: dma_rxinit\n", di->name));

	if (di->nrxd == 0)
		return;

#ifdef BCMPKTPOOL
	/* During the reset procedure, the active rxd may not be zero if pktpool is
	 * enabled, we need to reclaim active rxd to avoid rxd being leaked.
	 */
	if ((POOL_ENAB(di->pktpool)) && (NRXDACTIVE(di->rxin, di->rxout))) {
		dma_rxreclaim(dmah);
	}
#endif

	/* For split fifo, for fifo-0 reclaim is handled by fifo-1 */
	ASSERT((di->rxin == di->rxout) || (di->split_fifo == SPLIT_FIFO_0));
	di->rxin = di->rxout = di->rs0cd = 0;
	di->rxavail = di->nrxd - 1;

#ifdef BCM_HWALITE
	if ((mlo_role(di->mlc_dev) == MLC_ROLE_MAP) && (di->fifo == RX_MLO_FIFO3)) {
		uint32 mlo_unit;

		ASSERT(mlc_hwalite_is_enable(di->mlc_dev));

		FOREACH_MLO_AAP(di->mlc_dev, mlo_unit) {
			struct dma_mlo_aux_info *aux_info;
			aux_info = &di->aux_info[mlo_unit - 1];

			/* rxin/out needs to be reset during down.
			 * depth and other parameters will be adjusted during
			 * rxfill
			 */
			aux_info->rxin = aux_info->rxout = aux_info->rs0cd = 0;
		}
	}
#endif /* BCM_HWALITE */

	/* limit nrxpost buffer count to the max that will fit on the ring */
	if (di->sep_rxhdr) {
		/* 2 descriptors per buffer for 'sep_rxhdr' */
		di->nrxpost = MIN(di->nrxpost, di->rxavail/2);
	} else {
		/* 1 descriptor for each buffer in the normal case */
		di->nrxpost = MIN(di->nrxpost, di->rxavail);
	}

	/* clear rx descriptor ring */
	if (!DMA_HWA_HME_RX(di))
		BZERO_SM((void *)(uintptr)di->rxd64, (di->nrxd * sizeof(dma64dd_t)));

#if defined(BCM_ROUTER)
	DMA_MAP(di->osh, (void *)(uintptr)di->rxd64, (di->nrxd * sizeof(dma64dd_t)),
	        DMA_TX, NULL, NULL);
#endif

	/* DMA engine with out alignment requirement requires table to be inited
	 * before enabling the engine
	 */
	if (!di->aligndesc_4k)
		_dma_ddtable_init(di, DMA_RX, di->rxdpa);

	dma_rxenable(dmah);

	if (di->aligndesc_4k)
		_dma_ddtable_init(di, DMA_RX, di->rxdpa);
}

void
dma_rxenable(hnddma_t *dmah)
{
	dma_info_t *di = DI_INFO(dmah);

	uint dmactrlflags = di->hnddma.dmactrlflags;
	uint32 control = (R_REG(di->osh, &di->d64rxregs->control) & D64_RC_AE) | D64_RC_RE;

	DMA_TRACE(("%s: dma_rxenable\n", di->name));

	if ((dmactrlflags & DMA_CTRL_PEN) == 0)
		control |= D64_RC_PD;

	if (dmactrlflags & DMA_CTRL_ROC)
		control |= D64_RC_OC;

	/* These bits 20:18 (burstLen) of control register can be written but will take
	 * effect only if these bits are valid. So this will not affect previous versions
	 * of the DMA. They will continue to have those bits set to 0.
	 */
	control &= ~D64_RC_BL_MASK;
	control |= (di->rxburstlen << D64_RC_BL_SHIFT);
#ifndef DMA_WAIT_COMPLT_ROM_COMPAT
	control &= ~D64_RC_WAITCMP_MASK;
	control |= (di->rxwaitforcomplt << D64_RC_WAITCMP_SHIFT);
#endif
	control &= ~D64_RC_PC_MASK;
	control |= (di->rxprefetchctl << D64_RC_PC_SHIFT);

	control &= ~D64_RC_PT_MASK;
	control |= (di->rxprefetchthresh << D64_RC_PT_SHIFT);

	if (DMA_TRANSCOHERENT(di) && !DMA_HWA_HME_RX(di)) {
		control &= ~D64_RC_CO_MASK;
		control |= (1 << D64_RC_CO_SHIFT);
	}

#if defined(D11_SPLIT_RX_FD)
	/* Separate rx hdr descriptor */
	if (di->sep_rxhdr)
		control |= (di->sep_rxhdr << D64_RC_SHIFT);
#endif /* D11_SPLIT_RX_FD */
	if (di->d11rx_war)
		control |= D64_RC_SH;

	control &= ~D64_RC_RO_MASK;
	control |= (di->rxoffset & D64_RC_RO_MASK) << D64_RC_RO_SHIFT;

	/* Support rx frame offset extension and requesting offset >= 128 */
	if ((dmactrlflags & DMA_CTRL_ROEXT) && (di->rxoffset >= 128)) {
		control &= ~D64_RC_ROEXT_MASK;
		control |= (1 << D64_RC_ROEXT_SHIFT);
	}
	W_REG(di->osh, &di->d64rxregs->control, control);
}

/**
 * Enable Waitcomplete (D64_RC_WC) for a dma to protect against INT racing ahead of DMA transfers.
 * Impact of WC is essentially a "readback" of the last 2Bytes DMAed
 * before posting an INT and updating the write index.
 */
void
dma_rx_waitcomplete_enable(hnddma_t *dmah, bool enable)
{
	dma_info_t * di = DI_INFO(dmah);
	uint32 control;
	/* Enable WaitForComplete (readback) */
	di->rx_waitcomplete_enabled = enable;

	control = R_REG(di->osh, &di->d64rxregs->control);

	if (enable) {
		control |= D64_RC_WC;
	} else {
		control &= ~D64_RC_WC;
	}
	W_REG(di->osh, &di->d64rxregs->control, control);

}

void
dma_rxparam_get(hnddma_t *dmah, uint16 *rxoffset, uint16 *rxbufsize)
{
	dma_info_t * di = DI_INFO(dmah);

	/* the normal values fit into 16 bits */
	*rxoffset = (uint16)di->rxoffset;
	*rxbufsize = (uint16)di->rxbufsize;
}

/*
 * there are cases where the host mem can't be used when the dongle is in certain
 * state, so this covers that case
 * used only for RX FIFO0 with the split rx builds
*/

int
dma_rxfill_suspend(hnddma_t *dmah, bool suspended)
{
	dma_info_t * di = DI_INFO(dmah);

	di->rxfill_suspended = suspended;

	/* Suspend linked DMA also to avoid any rxfill. */
	if (di->linked_di) {
		di->linked_di->rxfill_suspended = suspended;
	}
	return 0;
}

int
dma_peekntxp(hnddma_t *dmah, int *len, void *txps[], txd_range_t range)
{
	dma_info_t *di = DI_INFO(dmah);

	uint16 act, start, end, i;
	void *txp = NULL;
	int k, len_max;

	DMA_TRACE(("%s: dma_peekntxp\n", di->name));

	ASSERT(len);
	ASSERT(txps);
	ASSERT(di);
	if (di->ntxd == 0) {
		*len = 0;
		return BCME_ERROR;
	}

	len_max = *len;
	*len = 0;

	start = di->xs0cd;
	if (range == HNDDMA_RANGE_ALL)
		end = di->txout;
	else {
		/* if this DMA channel is using indirect access, then configure the
		 * IndQSel register for this DMA channel
		 */
		if (DMA_INDIRECT(di)) {
			dma_set_indqsel(dmah, FALSE);
		}
		end = DMA64_TX_CD(di);
		act = DMA64_TX_AD(di);
		di->xs0cd = end;
		if (end != act)
			end = PREVTXD(act);
	}

	if ((start == 0) && (end > di->txout))
		return BCME_ERROR;

	k = 0;

#ifdef BULK_PKTLIST
	if (DMA_BULK_PATH(di)) {
		txp = di->dma_pkt_list.head_pkt;

		if (txp == NULL) {
			*len = k;
			return BCME_OK;
		}

		for (i = start; i != end; i = NEXTTXD(i)) {
			if (i == PKTDMAIDX(di->osh, txp)) {
				txps[k++] = txp;
				txp = PKTLINK(txp);
				if ((txp == NULL) || (k >= len_max)) {
					break;
				}
			}
		}
	} else
#endif /* BULK_PKTLIST */
	{
		for (i = start; i != end; i = NEXTTXD(i)) {
			txp = di->txp[i];
			if (txp != NULL) {
				if (k < len_max)
					txps[k++] = txp;
				else
					break;
			}
		}
	}

	*len = k;

	return BCME_OK;
}

/** like getnextrxp but not take off the ring */
void *
dma_peeknextrxp(hnddma_t *dmah)
{
	dma_info_t *di = DI_INFO(dmah);

	uint16 end, i;

	if ((di->nrxd == 0) || DMA_HWA_RX(di))
		return (NULL);

	end = DMA64_RX_CD(di);
	di->rs0cd = end;

	for (i = di->rxin; i != end; i = NEXTRXD(i)) {
#ifdef BULKRX_PKTLIST
		if (DMA_BULK_PATH(di)) {
#if defined(D11_SPLIT_RX_FD)
			if ((di->split_fifo) && (di->split_fifo == SPLIT_FIFO_0)) {
				return (di->linked_di->dma_rx_pkt_list.head_pkt);
			} else
#endif /* D11_SPLIT_RX_FD */
			{
				return (di->dma_rx_pkt_list.head_pkt);
			}
		} else
#endif /* BULKRX_PKTLIST */
		{
			if (di->rxp[i])
				return (di->rxp[i]);
		}
	}

	return (NULL);
}

void
dma_txblock(hnddma_t *dmah)
{
	dma_info_t *di = DI_INFO(dmah);

	di->hnddma.txavail = 0;
}

void
dma_txunblock(hnddma_t *dmah)
{
	dma_info_t *di = DI_INFO(dmah);

	di->hnddma.txavail = di->ntxd - NTXDACTIVE(di->txin, di->txout) - 1;
}

uint
dma_txactive(hnddma_t *dmah)
{
	dma_info_t *di = DI_INFO(dmah);

	return NTXDACTIVE(di->txin, di->txout);
}

/* count the number of tx packets that are queued to the dma ring */
uint
dma_txp(hnddma_t *dmah)
{
	uint count = 0;
	uint16 i;
	dma_info_t * di = DI_INFO(dmah);

	/* count the number of txp pointers that are non-null */
#ifdef BULK_PKTLIST
	if (DMA_BULK_PATH(di)) {
		void *txp = di->dma_pkt_list.head_pkt;
		while (txp) {
			count++;
			txp = PKTLINK(txp);
		}
	} else
#endif
	if (di->txp != NULL) {
		for (i = 0; i < di->ntxd; i++) {
			if (di->txp[i] != NULL) {
				count++;
			}
		}
	}

	return count;
}

uint
dma_txpending(hnddma_t *dmah)
{
	dma_info_t *di = DI_INFO(dmah);

	uint16 curr;

	/* if this DMA channel is using indirect access, then configure the IndQSel register
	 * for this DMA channel
	 */
	if (DMA_INDIRECT(di)) {
		dma_set_indqsel(dmah, FALSE);
	}

	curr = DMA64_TX_CD(di);

	di->xs0cd = curr;

	return NTXDACTIVE(curr, di->txout);
}

uint
dma_txcommitted(hnddma_t *dmah)
{
	dma_info_t *di = DI_INFO(dmah);

	uint16 ptr;
	uint txin = di->txin;

	if (txin == di->txout)
		return 0;

	/* if this DMA channel is using indirect access, then configure the IndQSel
	 * for this DMA channel
	 */
	if (DMA_INDIRECT(di)) {
		dma_set_indqsel(dmah, FALSE);
	}
	ptr = DMA64_TX_LD(di);

	return NTXDACTIVE(di->txin, ptr);
}

uint
dma_rxactive(hnddma_t *dmah)
{
	dma_info_t *di = DI_INFO(dmah);

	return NRXDACTIVE(di->rxin, di->rxout);
}

uint
dma_activerxbuf(hnddma_t *dmah)
{
	dma_info_t *di = DI_INFO(dmah);

	uint16 curr, ptr;

	curr = DMA64_RX_CD(di);
	ptr =  DMA64_RX_LD(di);

	return NRXDACTIVE(curr, ptr);
}

void
dma_counterreset(hnddma_t *dmah)
{
	dma_info_t *di = DI_INFO(dmah);

	/* reset all software counter */
	di->hnddma.rxgiants = 0;
	di->hnddma.rxnobuf = 0;
	di->hnddma.txnobuf = 0;
}

uint
dma_ctrlflags(hnddma_t *dmah, uint mask, uint flags)
{
	dma_info_t *di = DI_INFO(dmah);

	uint dmactrlflags;

	if (!di) {
		DMA_ERROR(("_dma_ctrlflags: NULL dma handle\n"));
		return (0);
	}

	dmactrlflags = di->hnddma.dmactrlflags;
	ASSERT((flags & ~mask) == 0);

	dmactrlflags &= ~mask;
	dmactrlflags |= flags;

	/* If trying to enable parity, check if parity is actually supported */
	if ((dmactrlflags & DMA_CTRL_PEN) && !DMA_PUQ_WAR(di)) {
		uint32 control;

		/* if using indirect DMA access, then configure IndQSel */
		if (DMA_INDIRECT(di)) {
			dma_set_indqsel(dmah, FALSE);
		}

		control = R_REG(di->osh, &di->d64txregs->control);
		W_REG(di->osh, &di->d64txregs->control, control | D64_XC_PD);
		if (R_REG(di->osh, &di->d64txregs->control) & D64_XC_PD) {
			/* We *can* disable it so it is supported,
			 * restore control register
			 */
			W_REG(di->osh, &di->d64txregs->control, control);
		} else {
			/* Not supported, don't allow it to be enabled */
			dmactrlflags &= ~DMA_CTRL_PEN;
		}
	}

	di->hnddma.dmactrlflags = dmactrlflags;

	return (dmactrlflags);
}

uint
dma_avoidance_cnt(hnddma_t *dmah)
{
	dma_info_t *di = DI_INFO(dmah);

	return (di->dma_avoidance_cnt);
}

static uint8
dma_align_sizetobits(uint size, uint flags)
{
	uint8 bitpos = 0;

	ASSERT(size);

	/* Add one more bit if the size is not power of two */
	if (!ISPOWEROF2(size)) {
		bitpos = 1;
	}

	if (!(flags & DMA_CTRL_NUM_DESC_NON_POWER_2)) {
		ASSERT(ISPOWEROF2(size));
	}

	while (size >>= 1) {
		bitpos ++;
	}

	return (bitpos);
}

/**
 * Allocates one rx or tx descriptor ring. Does not allocate buffers.
 * This function ensures that the DMA descriptor ring will not get allocated
 * across Page boundary. If the allocation is done across the page boundary
 * at the first time, then it is freed and the allocation is done at
 * descriptor ring size aligned location. This will ensure that the ring will
 * not cross page boundary
 * The allocated ring is *not* guaranteed to be filled with zeroes.
 */
static void *
dma_ringalloc(dma_info_t *di, uint32 boundary, uint size, uint16 *alignbits,
	uint* alloced, dmaaddr_t *descpa, osldma_t **dmah)
{
	void * va;
	uint32 desc_strtaddr;
	uint32 alignbytes = 1 << *alignbits;
	uint32 boundarybits = bcm_find_fsb(boundary);
	osl_t *osh = di->osh;

	ASSERT(ISPOWEROF2(boundary));
	if ((va = DMA_ALLOC_CONSISTENT_BOUNDARY(osh, size, *alignbits, boundarybits,
		alloced, descpa, (void **)dmah)) == NULL) {
		return NULL;
	}

	BCM_REFERENCE(boundarybits);
	/* Check boundary */
	desc_strtaddr = (uint32)ROUNDUP((uint)PHYSADDRLO(*descpa), alignbytes);
	if (((desc_strtaddr + size - 1) & boundary) !=
	    (desc_strtaddr & boundary)) {
		*alignbits = dma_align_sizetobits(size, di->hnddma.dmactrlflags);
		DMA_FREE_CONSISTENT(osh, va,
		                    size, *descpa, *dmah);
		va = DMA_ALLOC_CONSISTENT_BOUNDARY(osh, size, *alignbits, boundarybits,
			alloced, descpa, (void **)dmah);
	}

	/* Check if hit the 1MB boundary */
	if  (di->hnddma.dmactrlflags & DMA_CTRL_DESC_BASE_IDX_UPD) {
		alignbytes = 1 << *alignbits;
		desc_strtaddr = (uint32)ROUNDUP((uint)PHYSADDRLO(*descpa), alignbytes);
		if (((desc_strtaddr & (D64RINGBOUNDARY_LARGE_IDX_MODE - 1)) + size) ==
			D64RINGBOUNDARY_LARGE_IDX_MODE) {
			DMA_FREE_CONSISTENT(osh, va, size, *descpa, *dmah);
			*alignbits = D64RINGBOUNDARY_LARGE_IDX_MODE_BITS;
			va = DMA_ALLOC_CONSISTENT_BOUNDARY(osh, size, *alignbits,
				D64RINGBOUNDARY_LARGE_IDX_MODE_BITS, alloced,
				descpa, (void **)dmah);
		}
	}

	return va;
}

#if defined(BCMDBG) || defined(BCMDBG_TXSTALL) || defined(WL_MACDBG)

static void
dma64_dumpring(dma_info_t *di, struct bcmstrbuf *b, dma64dd_t *ring, uint start, uint end,
	uint max_num)
{
	uint i;

	/* if using indirect DMA access, then configure IndQSel */
	if (DMA_INDIRECT(di)) {
		dma_set_indqsel((hnddma_t *)di, FALSE);
	}

	BCM_REFERENCE(di);

	for (i = start; i != end; i = XXD((i + 1), max_num)) {
		if (DMA_AQM_DESC_SHORT(di)) {
			dma64dd_short_t *ring_short = (dma64dd_short_t *)ring;
			/* in the format of high->low 8 bytes */
			if (b) {
				bcm_bprintf(b, "ring index %d: SHORT_DESC %x %x\n", i,
					R_SM(&ring_short[i].ctrl2), R_SM(&ring_short[i].ctrl1));
			} else {
				DMA_ERROR(("ring index %d: SHORT_DESC %x %x\n", i,
					R_SM(&ring_short[i].ctrl2), R_SM(&ring_short[i].ctrl1)));
			}
		} else {
			/* in the format of high->low 16 bytes */
			if (b) {
				bcm_bprintf(b, "ring index %d: LONG_DESC 0x%x %x %x %x\n",
				i, R_SM(&ring[i].addrhigh), R_SM(&ring[i].addrlow),
				R_SM(&ring[i].ctrl2), R_SM(&ring[i].ctrl1));
			} else {
				DMA_ERROR(("ring index %d: LONG_DESC 0x%x %x %x %x\n",
				i, R_SM(&ring[i].addrhigh), R_SM(&ring[i].addrlow),
				R_SM(&ring[i].ctrl2), R_SM(&ring[i].ctrl1)));
			}
		}
	}
}

void
dma_dumptx(hnddma_t *dmah, struct bcmstrbuf *b, bool dumpring)
{
	dma_info_t *di = DI_INFO(dmah);
	dma_common_t *dmacommon = di->dmacommon;
	unsigned long txdpahi;
	uint32 xmptr, xmts0;

	if (di->ntxd == 0)
		return;

	/* if using indirect DMA access, then configure IndQSel */
	if (DMA_INDIRECT(di)) {
		/* Set override to TRUE since HWA may update it */
		dma_set_indqsel(dmah, TRUE);
	}

	txdpahi = PHYSADDRHI(di->txdpaorig);
	if (DMA_HWA_HME_TX(di)) {
		txdpahi = di->txdpahi;
	}

	xmptr = R_REG(di->osh, &di->d64txregs->ptr);
	xmts0 = R_REG(di->osh, &di->d64txregs->status0);

	if (b) {
#ifdef BULK_PKTLIST
		bcm_bprintf(b, "DMA64: txp %p txd64 %p txdpa 0x%x txdpahi 0x%lx "
			"head %p tail %p txin %d txout %d txavail %d txnodesc %d flags:0x%x\n",
			di->txp, di->txd64, (uint32)PHYSADDRLO(di->txdpa),
			txdpahi, di->dma_pkt_list.head_pkt,
			di->dma_pkt_list.tail_pkt, di->txin, di->txout,
			di->hnddma.txavail, di->hnddma.txnodesc, di->hnddma.dmactrlflags);
#else
		bcm_bprintf(b, "DMA64: txd64 %p txdpa 0x%x txdpahi 0x%lx txp %p txin %d txout %d "
			"txavail %d txnodesc %d flags:0x%x\n", di->txd64,
			(uint32)PHYSADDRLO(di->txdpa), txdpahi, di->txp, di->txin, di->txout,
			di->hnddma.txavail, di->hnddma.txnodesc, di->hnddma.dmactrlflags);
#endif
		bcm_bprintf(b, "xmtcontrol 0x%x desc_ring_pa: xmtaddrlow 0x%08x xmtaddrhigh 0x%08x "
			    "xmtptr 0x%x xmtstatus0 0x%x xmtstatus1 0x%x\n",
			    R_REG(di->osh, &di->d64txregs->control),
			    R_REG(di->osh, &di->d64txregs->addrlow),
			    R_REG(di->osh, &di->d64txregs->addrhigh),
			    xmptr, xmts0, R_REG(di->osh, &di->d64txregs->status1));

		if (DMA_PUQ_QREMAP(di) && DMA_AQM_DESC(di)) {
			bcm_bprintf(b, "puqaqmringdepth %d",
				R_REG(di->osh, dmacommon->puqaqmringdepth));
		}

#ifdef BCMHWA
		/* Only AQM registers value are reliable. */
		if (DMA_HWA_TX(di) && DMA_AQM_DESC(di)) {
			uint16 txin, txout;
			if (di->hnddma.dmactrlflags & DMA_CTRL_DESC_BASE_IDX_UPD) {
				txout = (uint16)((xmptr & D64_XS0_CD_MASK) -
					(B2I(di->xmtptrbase, di->dd64_size) & D64_XS0_CD_MASK));
				txin = (uint16)((xmts0 & D64_XS0_CD_MASK) -
					(B2I(di->xmtptrbase, di->dd64_size) & D64_XS0_CD_MASK));
			} else {
				txout = (uint16)B2I(((xmptr & D64_XS0_CD_MASK) -
					di->xmtptrbase) & D64_XS0_CD_MASK, di->dd64_size);
				txin = (uint16)B2I(((xmts0 & D64_XS0_CD_MASK) -
					di->xmtptrbase) & D64_XS0_CD_MASK, di->dd64_size);
			}
			bcm_bprintf(b, " hwatxpending %d hwatxavail %d\n",
				NTXDACTIVE(txin, txout), (di->ntxd - NTXDACTIVE(txin, txout) - 1));
		} else
#endif /* BCMHWA */
		{
			bcm_bprintf(b, "\n");
		}

		if ((xmts0 & D64_XS0_XS_MASK) == D64_XS0_XS_STOPPED) {
			uint32 xmts1 = R_REG(di->osh, &di->d64txregs->status1);
			xmts1 &= D64_XS1_XE_MASK;
			switch (xmts1) {
				case D64_XS1_XE_DPE:
					bcm_bprintf(b, "****** Descriptor protocol error ******\n");
					break;
				case D64_XS1_XE_DFU:
					bcm_bprintf(b, "****** Data fifo underrun ******\n");
					break;
				case D64_XS1_XE_DTE:
					bcm_bprintf(b, "****** Data transfer error ******\n");
					break;
				case D64_XS1_XE_DESRE:
					bcm_bprintf(b, "****** Descriptor read error ******\n");
					break;
				case D64_XS1_XE_COREE:
					bcm_bprintf(b, "****** Core error ******\n");
					break;
				case D64_XS1_XE_PARITY:
					bcm_bprintf(b, "****** Descriptor parity error ******\n");
					break;
				default:
					break;
			}
		}

		bcm_bprintf(b, "DMA64: DMA avoidance applied %d\n", di->dma_avoidance_cnt);
	} else {
		DMA_ERROR(("DMA64: txd64 %p txdpa 0x%x txdpahi 0x%lx txp %p txin %d txout %d "
		       "txavail %d txnodesc %d xmtptrbase %x\n",
		       di->txd64, (uint32)PHYSADDRLO(di->txdpa),
		       txdpahi, di->txp, di->txin, di->txout,
		       di->hnddma.txavail, di->hnddma.txnodesc, di->xmtptrbase));

		DMA_ERROR(("xmtcontrol 0x%x desc_ring_pa: xmtaddrlow 0x%08x xmtaddrhigh 0x%08x "
		       "xmtptr 0x%x xmtstatus0 0x%x xmtstatus1 0x%x\n",
		       R_REG(di->osh, &di->d64txregs->control),
		       R_REG(di->osh, &di->d64txregs->addrlow),
		       R_REG(di->osh, &di->d64txregs->addrhigh),
		       xmptr, xmts0,
		       R_REG(di->osh, &di->d64txregs->status1)));

		if (DMA_PUQ_QREMAP(di) && DMA_AQM_DESC(di)) {
			DMA_ERROR(("puqaqmringdepth %d",
				R_REG(di->osh, dmacommon->puqaqmringdepth)));
		}

#ifdef BCMHWA
		/* Only AQM registers value are reliable. */
		if (DMA_HWA_TX(di) && DMA_AQM_DESC(di)) {
			uint16 txin, txout;
			if (di->hnddma.dmactrlflags & DMA_CTRL_DESC_BASE_IDX_UPD) {
				txout = (uint16)((xmptr & D64_XS0_CD_MASK) -
					(B2I(di->xmtptrbase, di->dd64_size) & D64_XS0_CD_MASK));
				txin = (uint16)((xmts0 & D64_XS0_CD_MASK) -
					(B2I(di->xmtptrbase, di->dd64_size) & D64_XS0_CD_MASK));
			} else {
				txout = (uint16)B2I(((xmptr & D64_XS0_CD_MASK) -
					di->xmtptrbase) & D64_XS0_CD_MASK, di->dd64_size);
				txin = (uint16)B2I(((xmts0 & D64_XS0_CD_MASK) -
					di->xmtptrbase) & D64_XS0_CD_MASK, di->dd64_size);
			}
			DMA_ERROR((" hwatxpending %d hwatxavail %d CD %d txout %d\n",
				NTXDACTIVE(txin, txout), (di->ntxd - NTXDACTIVE(txin, txout) - 1),
				txin, txout));
		} else
#endif /* BCMHWA */
		{
			DMA_ERROR(("\n"));
		}

		if ((xmts0 & D64_XS0_XS_MASK) == D64_XS0_XS_STOPPED) {

			uint32 xmts1 = R_REG(di->osh, &di->d64txregs->status1);
			xmts1 &= D64_XS1_XE_MASK;

			switch (xmts1) {
				case D64_XS1_XE_DPE:
					DMA_ERROR(("****** Descriptor protocol error ******\n"));
					break;
				case D64_XS1_XE_DFU:
					DMA_ERROR(("****** Data fifo underrun ******\n"));
					break;
				case D64_XS1_XE_DTE:
					DMA_ERROR(("****** Data transfer error ******\n"));
					break;
				case D64_XS1_XE_DESRE:
					DMA_ERROR(("****** Descriptor read error ******\n"));
					break;
				case D64_XS1_XE_COREE:
					DMA_ERROR(("****** Core error ******\n"));
					break;
				case D64_XS1_XE_PARITY:
					DMA_ERROR(("****** Descriptor parity error ******\n"));
					break;
				default:
					break;
			}
		}
	}

	if (dumpring && di->txd64 && !DMA_HWA_HME_TX(di)) {
		dma64_dumpring(di, b, di->txd64, di->txin, di->txout, di->ntxd);
	}
}

void
dma_dumprx(hnddma_t *dmah, struct bcmstrbuf *b, bool dumpring)
{
	dma_info_t *di = DI_INFO(dmah);

	uint32 rcvptr, rcvs0, rcvs1;
	uint16 curr_descr, active_descr, last_descr;
	unsigned long rxdpahi;

	if (di->nrxd == 0)
		return;

	rxdpahi = PHYSADDRHI(di->rxdpaorig);
	if (DMA_HWA_HME_RX(di)) {
		rxdpahi = di->rxdpahi;
	}

#ifdef BULKRX_PKTLIST
	bcm_bprintf(b, "DMA64: rxd64 %p rxdpa 0x%x rxdpahi 0x%lx head %p tail "
			"%p rxin %d rxout %d flags:0x%x\n", OSL_OBFUSCATE_BUF(di->rxd64),
			(uint32)PHYSADDRLO(di->rxdpa), rxdpahi,
			di->dma_rx_pkt_list.head_pkt, di->dma_rx_pkt_list.tail_pkt,
			di->rxin, di->rxout, di->hnddma.dmactrlflags);
#else
	bcm_bprintf(b, "DMA64: rxd64 %p rxdpa 0x%x rxdpahi 0x%lx rxp %p rxin %d "
			"rxout %d flags:0x%x\n",
			OSL_OBFUSCATE_BUF(di->rxd64), (uint32)PHYSADDRLO(di->rxdpa),
			rxdpahi, OSL_OBFUSCATE_BUF(di->rxp),
			di->rxin, di->rxout, di->hnddma.dmactrlflags);
#endif

	rcvptr = R_REG(di->osh, &di->d64rxregs->ptr);
	rcvs0 = R_REG(di->osh, &di->d64rxregs->status0);
	rcvs1 = R_REG(di->osh, &di->d64rxregs->status1);
	if (di->hnddma.dmactrlflags & DMA_CTRL_DESC_BASE_IDX_UPD) {
		last_descr = (uint16)((rcvptr & D64_RS0_CD_MASK) -
			(B2I(di->rcvptrbase, di->dd64_size) & D64_RS0_CD_MASK));
		curr_descr = (uint16)((rcvs0 & D64_RS0_CD_MASK) -
			(B2I(di->rcvptrbase, di->dd64_size) & D64_RS0_CD_MASK));
		active_descr = (uint16)((rcvs1 & D64_RS0_CD_MASK) -
			(B2I(di->rcvptrbase, di->dd64_size) & D64_RS0_CD_MASK));
	} else {
		last_descr = (uint16)B2I(((rcvptr & D64_RS0_CD_MASK) - di->rcvptrbase) &
			D64_RS0_CD_MASK, di->dd64_size);
		curr_descr = (uint16)B2I(((rcvs0 & D64_RS0_CD_MASK) - di->rcvptrbase) &
			D64_RS0_CD_MASK, di->dd64_size);
		active_descr = (uint16)B2I(((rcvs1 & D64_RS0_CD_MASK) - di->rcvptrbase) &
			D64_RS0_CD_MASK, di->dd64_size);
	}

	bcm_bprintf(b, "rcvcontrol 0x%x rcvaddrlow 0x%08x rcvaddrhigh 0x%08x rcvptr "
		       "0x%x rcvstatus0 0x%x rcvstatus1 0x%x rxfilled %d\n"
		       "currdescr %d activedescr %d lastdescr %d\n",
		       R_REG(di->osh, &di->d64rxregs->control),
		       R_REG(di->osh, &di->d64rxregs->addrlow),
		       R_REG(di->osh, &di->d64rxregs->addrhigh),
		       rcvptr, rcvs0, R_REG(di->osh, &di->d64rxregs->status1),
		       ((last_descr == curr_descr) ? 0 :
		       (di->nrxd - NRXDACTIVE(last_descr, curr_descr))),
		       curr_descr, active_descr, last_descr);

	if ((rcvs0 & D64_RS0_RS_MASK) == D64_RS0_RS_STOPPED) {
		rcvs1 &= D64_RS1_RE_MASK;
		switch (rcvs1) {
			case D64_RS1_RE_DPE:
				bcm_bprintf(b, "****** Descriptor protocol error ******\n");
				break;
			case D64_RS1_RE_DFU:
				bcm_bprintf(b, "****** Data fifo underrun ******\n");
				break;
			case D64_RS1_RE_DTE:
				bcm_bprintf(b, "****** Data transfer error ******\n");
				break;
			case D64_RS1_RE_DESRE:
				bcm_bprintf(b, "****** Descriptor read error ******\n");
				break;
			case D64_RS1_RE_COREE:
				bcm_bprintf(b, "****** Core error ******\n");
				break;
			case D64_RS1_RE_PARITY:
				bcm_bprintf(b, "****** Descriptor parity error ******\n");
				break;
			default:
				break;
		}
	}

	if (di->rxd64 && dumpring && !DMA_HWA_HME_RX(di)) {
		if (DMA_HWA_RX(di))
			// for debug only
			dma64_dumpring(di, b, di->rxd64, 0, di->nrxd-1, di->nrxd);
		else
			dma64_dumpring(di, b, di->rxd64, di->rxin, di->rxout, di->nrxd);
	}
}

void
dma_dump(hnddma_t *dmah, struct bcmstrbuf *b, bool dumpring)
{
	dma_dumptx(dmah, b, dumpring);
	dma_dumprx(dmah, b, dumpring);
}

#endif

/**
 * Updating rxfill dynamically
 *
 * Dynamically updates rxfill'ed descriptors. If actively posted buffers are more
 * than nrxpost, this function will free up the remaining (active-nrxpost) posted buffers.
 */
void
dma_update_rxfill(hnddma_t *dmah)
{
	void *p;
	uint16 ad, rxout, n, dpp;
	dma_info_t * di = DI_INFO(dmah);
#if defined(SGLIST_RX_SUPPORT)
	dmaaddr_t pa;
#endif /* SGLIST_RX_SUPPORT */

	if (DMA_HWA_RX(di)) {
		return;
	}
	if (DMA_PCAP_EN(di)) {
		return;
	}

	/* if sep_rxhdr is enabled, for every pkt, two descriptors are programmed */
	/* NRXDACTIVE(rxin, rxout) would show 2 times no of actual full pkts */
	dpp = (di->sep_rxhdr) ? 2 : 1; /* dpp - descriptors per packet */

	/* read active descriptor */
	ad =  DMA64_RX_AD(di);
	rxout = di->rxout;

	/* if currently less buffers posted than requesting rxpost, do nothing */
	if (NRXDACTIVE(ad, rxout) <= (uint16)(di->nrxpost * dpp))
		return;

	/* calculating number of descriptors to remove */
	n = NRXDACTIVE(ad, rxout) - di->nrxpost * dpp;
	if (n % 2 != 0)
		n++;

	/* rewind rxout to move back "n" descriptors */
	di->rxout = PREVNRXD(rxout, n);

	/* update the chip lastdscr pointer */
	DMA64_RX_LD_UPD(di, di->rxout);

	 /* free up "n" descriptors i.e., (n/dpp) packets from bottom */
	while (n-- > 0) {
		rxout = PREVRXD(rxout);
		p = di->rxp[rxout];
		/* indicate 'rxout' index is free */
		di->rxp[rxout] = NULL;
#if defined(SGLIST_RX_SUPPORT)
		PHYSADDRLOSET(pa,
			(BUS_SWAP32(R_SM(&di->rxd64[rxout].addrlow)) - di->dataoffsetlow));
		PHYSADDRHISET(pa,
			(BUS_SWAP32(R_SM(&di->rxd64[rxout].addrhigh)) - di->dataoffsethigh));

		/* clear this packet from the descriptor ring */
		DMA_UNMAP(di->osh, pa, di->rxbufsize, DMA_RX, p, &di->rxp_dmah[rxout]);

		W_SM(&di->rxd64[rxout].addrlow, 0xdeadbeef);
		W_SM(&di->rxd64[rxout].addrhigh, 0xdeadbeef);
#endif /* SGLIST_RX_SUPPORT */
		if (p != (void*)PCI64ADDR_HIGH)
			PKTFREE(di->osh, p, FALSE);
	}

	ASSERT(NRXDACTIVE(ad, rxout) <= (uint16)(di->nrxpost * dpp));
}

/* 64-bit DMA functions */

void
dma_txinit(hnddma_t *dmah)
{
	dma_info_t *di = DI_INFO(dmah);

	uint32 control;
#ifdef BCM_DMA_INDIRECT
	uint32 addrlow;
	uint32 act;
	int i;
	si_t *sih = di->sih;
#endif /* BCM_DMA_INDIRECT */

	DMA_TRACE(("%s: dma_txinit\n", di->name));

	if (di->ntxd == 0)
		return;
	/* if using indirect DMA access, then configure IndQSel */
	if (DMA_INDIRECT(di)) {
		dma_set_indqsel(dmah, FALSE);
	}

	di->txin = di->txout = di->xs0cd = di->xs0cd_snapshot = 0;
	di->hnddma.txavail = di->ntxd - 1;

	if (!(di->hnddma.dmactrlflags & DMA_CTRL_SKIP_ALLOC)) {
		/* clear tx descriptor ring */
		if (!DMA_HWA_HME_TX(di)) {
			BZERO_SM((void *)(uintptr)di->txd64, (di->ntxd * di->dd64_size));
		}

#if defined(BCM_ROUTER)
		DMA_MAP(di->osh, (void *)(uintptr)di->txd64, (di->ntxd * di->dd64_size),
			DMA_TX, NULL, NULL);
#endif
	}

	/* These bits 20:18 (burstLen) of control register can be written but will take
	 * effect only if these bits are valid. So this will not affect previous versions
	 * of the DMA. They will continue to have those bits set to 0.
	 */
	if (DMA_PUQ_WAR(di)) {
		/* When enable queue remap read value is incorrect for TX,
		 * use register default value.
		 */
		control = di->xmtcontrol;
	} else {
		control = R_REG(di->osh, &di->d64txregs->control);
	}
	control = (control & ~D64_XC_BL_MASK) | (di->txburstlen << D64_XC_BL_SHIFT);
	control = (control & ~D64_XC_MR_MASK) | (di->txmultioutstdrd << D64_XC_MR_SHIFT);
	control = (control & ~D64_XC_PC_MASK) | (di->txprefetchctl << D64_XC_PC_SHIFT);
	control = (control & ~D64_XC_PT_MASK) | (di->txprefetchthresh << D64_XC_PT_SHIFT);
	control = (control & ~D64_XC_CS_MASK) | (di->txchanswitch << D64_XC_CS_SHIFT);
	if (DMA_TRANSCOHERENT(di) && !DMA_HWA_HME_TX(di))
		control = (control & ~D64_XC_CO_MASK) | (1 << D64_XC_CO_SHIFT);
	W_REG(di->osh, &di->d64txregs->control, control);

	control |= D64_XC_XE;
	/* DMA engine with out alignment requirement requires table to be inited
	 * before enabling the engine
	 */
	if (!di->aligndesc_4k)
		_dma_ddtable_init(di, DMA_TX, di->txdpa);

#ifdef BCM_DMA_INDIRECT
	if (DMA_INDIRECT(di) && DMA_AQM_DESC(di) &&
		((si_coreid(sih) == D11_CORE_ID) && (si_corerev(sih) == 65))) {
		addrlow = (uint32)(R_REG(di->osh, &di->d64txregs->addrlow) & 0xffff);
		if (addrlow != 0)
			W_REG(di->osh, &di->d64txregs->ptr, addrlow);
		for (i = 0; i < 20; i++) {
			act = (uint32)(R_REG(di->osh, &di->d64txregs->status1) & 0xffff);
			if (addrlow == act) {
				break;
			}
			OSL_DELAY(1);
		}
		if (addrlow != act) {
			DMA_ERROR(("%s %s: dma txdesc AD %#x != addrlow %#x\n", di->name,
			__FUNCTION__, act, addrlow));
		}
		ASSERT(addrlow == act);
	}
#endif /* BCM_DMA_INDIRECT */

	if (di->hnddma.dmactrlflags & DMA_CTRL_CS)
		control |= D64_XC_CS_MASK;

	if ((di->hnddma.dmactrlflags & DMA_CTRL_PEN) == 0)
		control |= D64_XC_PD;
	W_REG(di->osh, &di->d64txregs->control, control);
	if (DMA_PUQ_WAR(di)) {
		/* Store the register value in di->xmtcontrol */
		di->xmtcontrol = control;
	}

	/* DMA engine with alignment requirement requires table to be inited
	 * before enabling the engine
	 */
	if (di->aligndesc_4k)
		_dma_ddtable_init(di, DMA_TX, di->txdpa);

#if defined(BULK_DESCR_FLUSH)
	/* init the bulk_descr_tx_start_txout ... no DMA USER BULK TX ACTIVE */
	DMA_BULK_DESCR_TX_SET_INVALID(di);
#endif
}

bool
dma_txenabled(hnddma_t *dmah)
{
	dma_info_t *di = DI_INFO(dmah);

	uint32 xc;
	/* if using indirect DMA access, then configure IndQSel */
	if (DMA_INDIRECT(di)) {
		dma_set_indqsel(dmah, FALSE);
	}

	/* If the chip is dead, it is not enabled :-) */
	xc = R_REG(di->osh, &di->d64txregs->control);
	return ((xc != 0xffffffff) && (xc & D64_XC_XE));
}

void
dma_txsuspend(hnddma_t *dmah)
{
	dma_info_t *di = DI_INFO(dmah);

	DMA_TRACE(("%s: dma_txsuspend\n", di->name));

	if (di->ntxd == 0)
		return;

#ifdef BCM_DMA_INDIRECT
	/* if using indirect DMA access, then use common registers, SuspReq[x] */
	if (DMA_INDIRECT(di)) {
		if (DMA_PUQ_QREMAP(di)) {
			dma_common_t *dmac = di->dmacommon;
			W_REG(di->osh, dmac->suspflush_grpsel, (di->q_index / 96));
		}

		OR_REG(di->osh, di->suspreq, di->q_index_mask);
	} else
#endif /* BCM_DMA_INDIRECT */
	{
		OR_REG(di->osh, &di->d64txregs->control, D64_XC_SE);
	}
}

void
dma_txresume(hnddma_t *dmah)
{
	dma_info_t *di = DI_INFO(dmah);

	DMA_TRACE(("%s: dma_txresume\n", di->name));

	if (di->ntxd == 0)
		return;

#ifdef BCM_DMA_INDIRECT
	/* if using indirect DMA access, then use common registers, SuspReq[x] */
	if (DMA_INDIRECT(di)) {
		if (DMA_PUQ_QREMAP(di)) {
			dma_common_t *dmac = di->dmacommon;
			W_REG(di->osh, dmac->suspflush_grpsel, (di->q_index / 96));
		}

		AND_REG(di->osh, di->suspreq, ~di->q_index_mask);
	} else
#endif /* BCM_DMA_INDIRECT */
	{
		AND_REG(di->osh, &di->d64txregs->control, ~D64_XC_SE);
	}
}

bool
dma_txsuspended(hnddma_t *dmah)
{
	dma_info_t *di = DI_INFO(dmah);

	if (di->ntxd == 0)
		return TRUE;

#ifdef BCM_DMA_INDIRECT
	/* if using indirect DMA access, then use common registers, SuspReq[x] */
	if (DMA_INDIRECT(di)) {
		if (DMA_PUQ_QREMAP(di)) {
			dma_common_t *dmac = di->dmacommon;
			W_REG(di->osh, dmac->suspflush_grpsel, (di->q_index / 96));
		}

		return ((R_REG(di->osh, di->suspreq) & di->q_index_mask) != 0);
	} else
#endif /* BCM_DMA_INDIRECT */
	{
		return ((R_REG(di->osh, &di->d64txregs->control) & D64_XC_SE) == D64_XC_SE);
	}
}

void
dma_txflush(hnddma_t *dmah)
{
	dma_info_t *di = DI_INFO(dmah);

	DMA_TRACE(("%s: dma_txflush\n", di->name));

	if (di->ntxd == 0)
		return;

#ifdef BCM_DMA_INDIRECT
	/* if using indirect DMA access, then use common registers, FlushReq[x] */
	if (DMA_INDIRECT(di)) {
		if (DMA_PUQ_QREMAP(di)) {
			dma_common_t *dmac = di->dmacommon;
			W_REG(di->osh, dmac->suspflush_grpsel, (di->q_index / 96));
		}

		OR_REG(di->osh, di->flushreq, di->q_index_mask);
	} else
#endif /* BCM_DMA_INDIRECT */
	{
		OR_REG(di->osh, &di->d64txregs->control, D64_XC_SE | D64_XC_FL);
	}
}

void
dma_txflush_clear(hnddma_t *dmah)
{
	dma_info_t *di = DI_INFO(dmah);

	uint32 status;

	DMA_TRACE(("%s: dma_txflush_clear\n", di->name));

	if (di->ntxd == 0)
		return;

#ifdef BCM_DMA_INDIRECT
	/* if using indirect DMA access, then use common registers, FlushReq[x] */
	if (DMA_INDIRECT(di)) {
		if (DMA_PUQ_QREMAP(di)) {
			dma_common_t *dmac = di->dmacommon;
			W_REG(di->osh, dmac->suspflush_grpsel, (di->q_index / 96));
		}

		AND_REG(di->osh, di->flushreq, ~di->q_index_mask);
	} else
#endif /* BCM_DMA_INDIRECT */
	{
		SPINWAIT(((status = (R_REG(di->osh, &di->d64txregs->status0) & D64_XS0_XS_MASK)) !=
		          D64_XS0_XS_DISABLED) &&
		         (status != D64_XS0_XS_IDLE) &&
		         (status != D64_XS0_XS_STOPPED),
		         10000);
		AND_REG(di->osh, &di->d64txregs->control, ~D64_XC_FL);
	}
}

void
dma_txrewind(hnddma_t *dmah)
{
	uint16 start, end, i;
	uint32 flags;
	dma64dd_t *ring;

	dma_info_t * di = DI_INFO(dmah);

	DMA_TRACE(("%s: dma64_txrewind\n", di->name));

	/* if using indirect DMA access, then configure IndQSel */
	if (DMA_INDIRECT(di)) {
		dma_set_indqsel(dmah, FALSE);
	}

	ASSERT(dma_txsuspended(dmah));

	/* select to read index of already fetched desc */
	AND_REG(di->osh, &di->d64txregs->control, ~D64_RC_SA);

	start = DMA64_TX_AD(di);
	end = di->txout;

	/* Should not be AQM */
	ASSERT(!DMA_AQM_DESC_SHORT(di));

	ring = di->txd64;
	for (i = start; i != end; i = NEXTTXD(i)) {
		/* find the first one having eof set */
		flags = R_SM(&ring[i].ctrl1);
		if (flags & CTRL_EOF) {
			/* rewind end to (i+1) */
			DMA64_TX_LD_UPD(di, NEXTTXD(i));
			DMA_TRACE(("ActiveIdx %d EndIdx was %d now %d\n", start, end, NEXTTXD(i)));
			break;
		}
	}
}

bool
dma_txstopped(hnddma_t *dmah)
{
	dma_info_t *di = DI_INFO(dmah);

	/* if using indirect DMA access, then configure IndQSel */
	if (DMA_INDIRECT(di)) {
		dma_set_indqsel(dmah, FALSE);
	}

	return ((R_REG(di->osh, &di->d64txregs->status0) & D64_XS0_XS_MASK) == D64_XS0_XS_STOPPED);
}

bool
dma_rxstopped(hnddma_t *dmah)
{
	dma_info_t *di = DI_INFO(dmah);

	return ((R_REG(di->osh, &di->d64rxregs->status0) & D64_RS0_RS_MASK) == D64_RS0_RS_STOPPED);
}

/**
 * Allocates one descriptor ring. Does *not* initialize the allocated descriptors.
 */
static bool
BCMATTACHFN_DMA_ATTACH(dma64_alloc_tx)(dma_info_t *di)
{
	uint32 size;
	void *va;
	uint alloced = 0;
	uint32 align;
	uint16 align_bits;
	uint32 boundary;

	size = di->ntxd * di->dd64_size;
	align_bits = di->dmadesc_align;
	align = (1 << align_bits);

	/* There is a HW limitation on index based programing of descriptor ring.
	 * 1. Cannot cross 1MB boundary.
	 * 2. Cannot hit 1MB boundary.
	 */
	if  (di->hnddma.dmactrlflags & DMA_CTRL_DESC_BASE_IDX_UPD) {
		if (size > D64RINGBOUNDARY_LARGE) {
			boundary = D64RINGBOUNDARY_LARGE_IDX_MODE;
		} else {
			boundary = D64RINGBOUNDARY_LARGE;
		}
	} else if (di->d64_xs0_cd_mask == 0x1fff) {
		boundary = D64RINGBOUNDARY;
	} else {
		boundary = D64RINGBOUNDARY_LARGE;
	}

	if ((va = dma_ringalloc(di, boundary, size, &align_bits, &alloced,
		&di->txdpaorig, &di->tx_dmah)) == NULL) {
		DMA_ERROR(("%s: dma64_alloc_tx: DMA_ALLOC_CONSISTENT(ntxd) failed\n", di->name));
		return FALSE;
	}
	align = (1 << align_bits);

	/* adjust the pa by rounding up to the alignment */
	PHYSADDRLOSET(di->txdpa, ROUNDUP(PHYSADDRLO(di->txdpaorig), align));
	PHYSADDRHISET(di->txdpa, PHYSADDRHI(di->txdpaorig));

	/* Make sure that alignment didn't overflow */
	ASSERT(PHYSADDRLO(di->txdpa) >= PHYSADDRLO(di->txdpaorig));

	/* find the alignment offset that was used */
	di->txdalign = (uint)(PHYSADDRLO(di->txdpa) - PHYSADDRLO(di->txdpaorig));

	/* make sure we don't cross our boundaries */
	ASSERT(size + di->txdalign <= alloced);

	/* adjust the va by the same offset */
	di->txd64 = (dma64dd_t *)((uintptr)va + di->txdalign);

	di->txdalloc = alloced;
	ASSERT(ISALIGNED(PHYSADDRLO(di->txdpa), align));

	return TRUE;
}

/**
 * Allocates one descriptor ring. Does *not* initialize the allocated descriptors.
 */
static bool
dma64_alloc_rx(dma_info_t *di)
{
	uint32 size;
	void *va;
	uint alloced = 0;
	uint32 align;
	uint16 align_bits;
	uint32 boundary;

	size = di->nrxd * di->dd64_size;
	align_bits = di->dmadesc_align;
	align = (1 << align_bits);

	if  (di->hnddma.dmactrlflags & DMA_CTRL_DESC_BASE_IDX_UPD) {
		boundary = D64RINGBOUNDARY_LARGE_IDX_MODE;
	} else if (di->d64_rs0_cd_mask == 0x1fff) {
		boundary = D64RINGBOUNDARY;
	} else {
		boundary = D64RINGBOUNDARY_LARGE;
	}

	if ((va = dma_ringalloc(di, boundary, size, &align_bits, &alloced,
		&di->rxdpaorig, &di->rx_dmah)) == NULL) {
		DMA_ERROR(("%s: dma64_alloc_rx: DMA_ALLOC_CONSISTENT(nrxd) failed\n", di->name));
		return FALSE;
	}
	align = (1 << align_bits);

	/* adjust the pa by rounding up to the alignment */
	PHYSADDRLOSET(di->rxdpa, ROUNDUP(PHYSADDRLO(di->rxdpaorig), align));
	PHYSADDRHISET(di->rxdpa, PHYSADDRHI(di->rxdpaorig));

	/* Make sure that alignment didn't overflow */
	ASSERT(PHYSADDRLO(di->rxdpa) >= PHYSADDRLO(di->rxdpaorig));

	/* find the alignment offset that was used */
	di->rxdalign = (uint)(PHYSADDRLO(di->rxdpa) - PHYSADDRLO(di->rxdpaorig));

	/* make sure we don't cross our boundaries */
	ASSERT(size + di->rxdalign <= alloced);

	/* adjust the va by the same offset */
	di->rxd64 = (dma64dd_t *)((uintptr)va + di->rxdalign);

	di->rxdalloc = alloced;
	ASSERT(ISALIGNED(PHYSADDRLO(di->rxdpa), align));

	return TRUE;
}

bool
dma_txreset(hnddma_t *dmah)
{
	dma_info_t *di = DI_INFO(dmah);

	uint32 status = D64_XS0_XS_DISABLED;

	if (di->ntxd == 0)
		return TRUE;

	/* if using indirect DMA access, then configure IndQSel */
	/* If the DMA core has resetted, then the default IndQSel = 0 */
	/* So force the configuration */
	if (DMA_INDIRECT(di)) {
		dma_set_indqsel(dmah, TRUE);
	} else {
		/* If DMA is already in reset, do not reset. */
		if ((R_REG(di->osh, &di->d64txregs->status0) & D64_XS0_XS_MASK) ==
			D64_XS0_XS_DISABLED)
			return TRUE;

		/* suspend tx DMA first */
		W_REG(di->osh, &di->d64txregs->control, D64_XC_SE);
		SPINWAIT(((status = (R_REG(di->osh, &di->d64txregs->status0) & D64_XS0_XS_MASK)) !=
		          D64_XS0_XS_DISABLED) &&
		         (status != D64_XS0_XS_IDLE) &&
		         (status != D64_XS0_XS_STOPPED),
		         10000);
	}

	/* For IndDMA, the channel status is ignored. */
	W_REG(di->osh, &di->d64txregs->control, 0);

	/* Reset the default value to 0. */
	if (DMA_PUQ_WAR(di)) {
		di->xmtcontrol = 0x0;
	}

	if (!DMA_INDIRECT(di) || DMA_AQM_DESC(di)) {
		SPINWAIT(((status = (R_REG(di->osh, &di->d64txregs->status0) & D64_XS0_XS_MASK)) !=
		          D64_XS0_XS_DISABLED),
		         10000);

		/* We should be disabled at this point */
		if (status != D64_XS0_XS_DISABLED) {
			// The status at the xmtstatus may not be correct when PUQ is enabled
			// Ignore the status check
			if (DMA_PUQ_QREMAP(di)) {
				// Set status to disabled to avoid message flooding.
				status = D64_XS0_XS_DISABLED;
			} else {
				DMA_ERROR(("%s: status != D64_XS0_XS_DISABLED 0x%x\n",
					__FUNCTION__, status));
				ASSERT(status == D64_XS0_XS_DISABLED);
				OSL_DELAY(300);
			}
		}
	}

#if defined(BULK_DESCR_FLUSH)
	/* reset/init the bulk_descr_tx_start_txout ... no DMA USER BULK TX ACTIVE */
	DMA_BULK_DESCR_TX_SET_INVALID(di);
#endif
	return (status == D64_XS0_XS_DISABLED);
}

bool
dma_rxidlestatus(hnddma_t *dmah)
{
	dma_info_t *di = DI_INFO(dmah);

	if ((di->nrxd == 0) || DMA_HWA_RX(di)) {
		return TRUE;
	}

	/* Ensure that Rx rcvstatus is in Idle Wait */
	if (R_REG(di->osh, &di->d64rxregs->status0) & D64_RS0_RS_IDLE) {
		return TRUE;
	}
	return FALSE;
}

bool
dma_rxreset(hnddma_t *dmah)
{
	dma_info_t *di = DI_INFO(dmah);

	uint32 status;

	if (di->nrxd == 0)
		return TRUE;

	W_REG(di->osh, &di->d64rxregs->control, 0);
	SPINWAIT(((status = (R_REG(di->osh, &di->d64rxregs->status0) & D64_RS0_RS_MASK)) !=
	          D64_RS0_RS_DISABLED), 10000);

	return (status == D64_RS0_RS_DISABLED);
}

bool
dma_txsuspendedidle(hnddma_t *dmah)
{
	dma_info_t *di = DI_INFO(dmah);

	if (di->ntxd == 0)
		return TRUE;

	/* if using indirect DMA access, then configure IndQSel */
	if (DMA_INDIRECT(di)) {
		dma_set_indqsel(dmah, FALSE);
	}

	if (!(R_REG(di->osh, &di->d64txregs->control) & D64_XC_SE))
		return 0;

	if ((R_REG(di->osh, &di->d64txregs->status0) & D64_XS0_XS_MASK) == D64_XS0_XS_IDLE)
		return 1;

	return 0;
}

/**
 * Useful when sending unframed data.  This allows us to get a progress report from the DMA.
 * We return a pointer to the beginning of the data buffer of the current descriptor.
 * If DMA is idle, we return NULL.
 */
/* Might be nice if DMA HW could tell us current position rather than current descriptor */
void *
dma_getpos(hnddma_t *dmah, bool direction)
{
	dma_info_t *di = DI_INFO(dmah);

	void *va;
	bool idle;
	uint16 cur_idx;

	if (direction == DMA_TX) {
		/* if using indirect DMA access, then configure IndQSel */
		if (DMA_INDIRECT(di)) {
			dma_set_indqsel(dmah, FALSE);
		}
		cur_idx = DMA64_TX_CD(di);
		idle = !NTXDACTIVE(di->txin, di->txout);
#ifdef BULK_PKTLIST
		if (DMA_BULK_PATH(di)) {
			va = di->dma_pkt_list.head_pkt;
			while (va != NULL) {
				if (cur_idx == PKTDMAIDX(di->osh, va)) {
					break;
				}
				va = PKTLINK(va);
			}
		} else
#endif
		va = di->txp[cur_idx];
	} else {
		cur_idx = DMA64_RX_CD(di);
		idle = !NRXDACTIVE(di->rxin, di->rxout);
		va = di->rxp[cur_idx];
	}

	/* If DMA is IDLE, return NULL */
	if (idle) {
		DMA_TRACE(("%s: DMA idle, return NULL\n", __FUNCTION__));
		va = NULL;
	}

	return va;
}

/**
 * TX of unframed data
 *
 * Adds a DMA ring descriptor for the data pointed to by "buf".
 * This is for DMA of a buffer of data and is unlike other hnddma TX functions
 * that take a pointer to a "packet"
 * Each call to this is results in a single descriptor being added for "len" bytes of
 * data starting at "buf", it doesn't handle chained buffers.
 */
int
dma_txunframed(hnddma_t *dmah, void *buf, uint len, bool commit)
{
	dma_info_t *di = DI_INFO(dmah);

	uint16 txout;
	uint32 flags = 0;
	dmaaddr_t pa; /* phys addr */
	txout = di->txout;

	/* if using indirect DMA access, then configure IndQSel */
	if (DMA_INDIRECT(di)) {
		dma_set_indqsel(dmah, FALSE);
	}

	/* return nonzero if out of tx descriptors */
	if (NEXTTXD(txout) == di->txin)
		goto outoftxd;

	if (len == 0)
		return 0;

#ifdef BCM_SECURE_DMA
	pa = SECURE_DMA_MAP(di->osh, buf, len, DMA_TX, NULL, NULL,
		&di->sec_cma_info_tx, 0, SECDMA_TXBUF_POST);
#else
	pa = DMA_MAP(di->osh, buf, len, DMA_TX, NULL, &di->txp_dmah[txout]);
#endif /* BCM_SECURE_DMA */

	flags = (D64_CTRL1_SOF | D64_CTRL1_IOC | D64_CTRL1_EOF);

	if (txout == (di->ntxd - 1))
		flags |= D64_CTRL1_EOT;

	dma64_dd_upd(di, di->txd64, pa, txout, &flags, len);
#ifdef BULK_PKTLIST_DEBUG
	ASSERT(di->txp[txout] == NULL);
#else
	if (!DMA_BULK_PATH(di)) {
		ASSERT(di->txp[txout] == NULL);
	}
#endif

#if defined(BULK_DESCR_FLUSH)
	if (!DMA_BULK_DESCR_TX_IS_VALID(di)) {
		/* Should not be AQM */
		ASSERT(!DMA_AQM_DESC_SHORT(di));
		DMA_MAP(di->osh, dma64_txd64(di, di->txout), DMA64_FLUSH_LEN(di, 1),
			DMA_TX, NULL, NULL);
	}
#endif /* BULK_DESCR_FLUSH */

#ifdef BULK_PKTLIST_DEBUG
	/* save the buffer pointer - used by dma_getpos */
	di->txp[txout] = buf;
#else
	if (!DMA_BULK_PATH(di)) {
		/* save the buffer pointer - used by dma_getpos */
		di->txp[txout] = buf;
	}
#endif

	txout = NEXTTXD(txout);
	/* bump the tx descriptor index */
	di->txout = txout;

	/* kick the chip */
	if (commit) {
		dma64_txcommit_local(di, FALSE);
	}

	/* tx flow control */
	di->hnddma.txavail = di->ntxd - NTXDACTIVE(di->txin, di->txout) - 1;
	DMA_TRACE(("dma64_txunframed: di0x%p buf:0x%p\n", di, buf));
	return (0);

outoftxd:
	DMA_ERROR(("%s: %s: out of txds !!!\n", di->name, __FUNCTION__));
	di->hnddma.txavail = 0;
	di->hnddma.txnobuf++;
	return (-1);
}

/**
 * RX of unframed data
 *
 * Adds a DMA ring descriptor for the data pointed to by "buf".
 * This is for DMA of a buffer of data and is unlike other hnddma RX functions
 * that take a pointer to a "packet"
 * Each call to this is result in a single descriptor being added for "len" bytes of
 * data starting at "buf", it doesn't handle chained buffers.
 */
int
dma_rxfill_unframed(hnddma_t *dmah, void *buf, uint len, bool commit)
{
	uint16 rxout;
#if !defined(BCM_SECURE_DMA)
	uint32 flags = 0;
	dmaaddr_t pa; /* phys addr */
#endif /* !BCM_SECURE_DMA */
	dma_info_t *di = DI_INFO(dmah);

	rxout = di->rxout;

	/* return nonzero if out of rx descriptors */
	if (NEXTRXD(rxout) == di->rxin)
		goto outofrxd;

	ASSERT(len <= di->rxbufsize);
#if !defined(BCM_SECURE_DMA)
	/* cache invalidate maximum buffer length */
	pa = DMA_MAP(di->osh, buf, di->rxbufsize, DMA_RX, NULL, NULL);
	if (rxout == (di->nrxd - 1))
		flags |= D64_CTRL1_EOT;

	dma64_dd_upd(di, di->rxd64, pa, rxout, &flags, len);
	ASSERT(di->rxp[rxout] == NULL);
#endif /* #if !defined(BCM_SECURE_DMA) */

#if defined(BULK_DESCR_FLUSH)
	DMA_MAP(di->osh, dma64_rxd64(di, di->rxout), DMA64_FLUSH_LEN(di, 1),
		DMA_TX, NULL, NULL);
#endif /* BULK_DESCR_FLUSH */

	/* save the buffer pointer - used by dma_getpos */
	di->rxp[rxout] = buf;

	rxout = NEXTRXD(rxout);

	/* kick the chip */
	if (commit) {
		DMA64_RX_LD_UPD(di, rxout);
	}

	/* bump the rx descriptor index */
	di->rxout = rxout;

	/* rx flow control */
	di->rxavail = di->nrxd - NRXDACTIVE(di->rxin, di->rxout) - 1;

	return (0);

outofrxd:
	DMA_ERROR(("%s: %s: out of rxds !!!\n", di->name, __FUNCTION__));
	di->rxavail = 0;
	di->hnddma.rxnobuf++;
	return (-1);
}

void BCMFASTPATH
dma_clearrxp(hnddma_t *dmah)
{
	uint16 i, curr;
	dma_info_t *di = DI_INFO(dmah);

	i = di->rxin;

	/* return if no packets posted */
	if (i == di->rxout)
		return;

	if (di->rxin == di->rs0cd) {
		curr =  DMA64_RX_CD(di);
		di->rs0cd = curr;
	} else
		curr = di->rs0cd;

	while (i != curr) {

		ASSERT(di->rxp[i]);
		di->rxp[i] = NULL;

		W_SM(&di->rxd64[i].addrlow, 0xdeadbeef);
		W_SM(&di->rxd64[i].addrhigh, 0xdeadbeef);
		i = NEXTRXD(i);
	}

	di->rxin = i;

	di->rxavail = di->nrxd - NRXDACTIVE(di->rxin, di->rxout) - 1;

}

/* If using indirect DMA interface, then before calling this function the IndQSel must be
 * configured for the DMA channel for which this call is being made.
 */
static bool
BCMATTACHFN_DMA_ATTACH(_dma64_addrext)(osl_t *osh, dma64regs_t *dma64regs)
{
	uint32 w;
	OR_REG(osh, &dma64regs->control, D64_XC_AE);
	w = R_REG(osh, &dma64regs->control);
	AND_REG(osh, &dma64regs->control, ~D64_XC_AE);
	return ((w & D64_XC_AE) == D64_XC_AE);
}

/* Utility function to reverse the list ordering of the
 * pending dma pkt list.
 * dma64_txrotate() works backwards from the last node.
 */
#ifdef BULK_PKTLIST
static void
*reverse_pktlist(void *list)
{
	void *curr, *next, *prev;
	curr = list;
	next = NULL;
	prev = NULL;

	while (curr) {
		next = PKTLINK(curr);
		PKTSETLINK(curr, prev);
		prev = curr;
		curr = next;
	}
	return prev;
}
#endif /* BULK_PKTLIST */

/**
 * Rotate all active tx dma ring entries "forward" by (ActiveDescriptor - txin).
 */
void
dma_txrotate(hnddma_t *dmah)
{
	dma_info_t *di = DI_INFO(dmah);

	uint16 ad;
	uint nactive;
	uint rot;
	uint16 old, new;
	uint32 w;
	uint16 first, last;
#ifdef BULK_PKTLIST
	void *txp = NULL;
#endif

	ASSERT(dma_txsuspendedidle(dmah));

	/* if using indirect DMA access, then configure IndQSel */
	if (DMA_INDIRECT(di)) {
		dma_set_indqsel(dmah, FALSE);
	}

	nactive = dma_txactive(dmah);
	ad = DMA64_TX_AD(di);
	rot = TXD(ad - di->txin);

	ASSERT(rot < di->ntxd);

	/* full-ring case is a lot harder - don't worry about this */
	if (rot >= (di->ntxd - nactive)) {
		DMA_ERROR(("%s: dma_txrotate: ring full - punt\n", di->name));
		return;
	}

	first = di->txin;
	last = PREVTXD(di->txout);

#ifdef BULK_PKTLIST
	/* Reverse DMA pkt list */
	if (DMA_BULK_PATH(di)) {
		txp = reverse_pktlist(di->dma_pkt_list.head_pkt);
	}
#endif
	/* move entries starting at last and moving backwards to first */
	for (old = last; old != PREVTXD(first); old = PREVTXD(old)) {
		new = TXD(old + rot);

		/*
		 * Move the tx dma descriptor.
		 * EOT is set only in the last entry in the ring.
		 */
		w = BUS_SWAP32(R_SM(&di->txd64[old].ctrl1)) & ~D64_CTRL1_EOT;
		if (new == (di->ntxd - 1))
			w |= D64_CTRL1_EOT;
		W_SM(&di->txd64[new].ctrl1, BUS_SWAP32(w));

		w = BUS_SWAP32(R_SM(&di->txd64[old].ctrl2));
		W_SM(&di->txd64[new].ctrl2, BUS_SWAP32(w));

		W_SM(&di->txd64[new].addrlow, R_SM(&di->txd64[old].addrlow));
		W_SM(&di->txd64[new].addrhigh, R_SM(&di->txd64[old].addrhigh));
#if defined(DESCR_DEADBEEF)
		/* zap the old tx dma descriptor address field */
		W_SM(&di->txd64[old].addrlow, BUS_SWAP32(0xdeadbeef));
		W_SM(&di->txd64[old].addrhigh, BUS_SWAP32(0xdeadbeef));
#endif /* DESCR_DEADBEEF */

#ifdef BULK_PKTLIST
		ASSERT(txp);
		if (DMA_BULK_PATH(di)) {
			if (old == PKTDMAIDX(di->osh, txp)) {
				PKTSETDMAIDX(di->osh, txp, new);
				txp = PKTLINK(txp);
			}
#ifdef BULK_PKTLIST_DEBUG
			/* move the corresponding txp[] entry */
			ASSERT(di->txp[new] == NULL);
			di->txp[new] = di->txp[old];
			di->txp[old] = NULL;
#endif
		} else
#endif /* BULK_PKTLIST */
		{
			/* move the corresponding txp[] entry */
			ASSERT(di->txp[new] == NULL);
			di->txp[new] = di->txp[old];
			di->txp[old] = NULL;
		}
		/* Move the map */
		if (DMASGLIST_ENAB) {
			bcopy(&di->txp_dmah[old], &di->txp_dmah[new], sizeof(hnddma_seg_map_t));
			bzero(&di->txp_dmah[old], sizeof(hnddma_seg_map_t));
		}
	}

#ifdef BULK_PKTLIST
	/* Restore DMA pkt list */
	if (DMA_BULK_PATH(di)) {
		txp = reverse_pktlist(di->dma_pkt_list.head_pkt);
	}
#endif

	/* update txin and txout */
	di->txin = ad;
	di->txout = TXD(di->txout + rot);
	di->hnddma.txavail = di->ntxd - NTXDACTIVE(di->txin, di->txout) - 1;

#if (defined(__ARM_ARCH_7A__) && defined(CA7)) || (defined(__ARM_ARCH_8A__) && \
	defined(CA53)) || defined(STB)
	/* memory barrier before posting the descriptor */
	DMB();
#endif
	/* kick the chip */
	dma64_txcommit_local(di, FALSE);
}

uint
BCMATTACHFN_DMA_ATTACH(dma_addrwidth)(si_t *sih, void *dmaregs)
{
	dma32regs_t *dma32regs;
	osl_t *osh;

	osh = si_osh(sih);

	/* Perform 64-bit checks only if we want to advertise 64-bit (> 32bit) capability) */
	/* DMA engine is 64-bit capable */
	if ((si_core_sflags(sih, 0, 0) & SISF_DMA64) == SISF_DMA64) {
		/* backplane are 64-bit capable */
		if (si_backplane64(sih))
			/* If bus is System Backplane or PCIE then we can access 64-bits */
			if ((BUSTYPE(sih->bustype) == SI_BUS) ||
			    ((BUSTYPE(sih->bustype) == PCI_BUS) &&
			     ((BUSCORETYPE(sih->buscoretype) == PCIE_CORE_ID) ||
			      (BUSCORETYPE(sih->buscoretype) == PCIE2_CORE_ID))))
				return (DMADDRWIDTH_64);

		/* DMA64 is always 32-bit capable, AE is always TRUE */
		ASSERT(_dma64_addrext(osh, (dma64regs_t *)dmaregs));

		return (DMADDRWIDTH_32);
	}

	/* Start checking for 32-bit / 30-bit addressing */
	dma32regs = (dma32regs_t *)dmaregs;

	/* For System Backplane, PCIE bus or addrext feature, 32-bits ok */
	if ((BUSTYPE(sih->bustype) == SI_BUS) ||
	    ((BUSTYPE(sih->bustype) == PCI_BUS) &&
	     ((BUSCORETYPE(sih->buscoretype) == PCIE_CORE_ID) ||
	      (BUSCORETYPE(sih->buscoretype) == PCIE2_CORE_ID))) ||
	    (_dma32_addrext(osh, dma32regs)))
		return (DMADDRWIDTH_32);

	/* Fallthru */
	return (DMADDRWIDTH_30);
}

#ifdef BCMPKTPOOL
int
dma_pktpool_set(hnddma_t *dmah, pktpool_t *pool)
{
	dma_info_t *di = DI_INFO(dmah);

	ASSERT(di);
	ASSERT(di->pktpool == NULL);
	di->pktpool = pool;
	return 0;
}
#endif

pktpool_t*
dma_pktpool_get(hnddma_t *dmah)
{
	dma_info_t * di = DI_INFO(dmah);

	ASSERT(di);
	ASSERT(di->pktpool != NULL);
	return (di->pktpool);
}

bool
dma_rxtxerror(hnddma_t *dmah, bool istx)
{
	dma_info_t * di = DI_INFO(dmah);

	uint32 status1 = 0;
	uint16 curr;

	if (istx) {

		/* if using indirect DMA access, then configure IndQSel */
		if (DMA_INDIRECT(di)) {
			dma_set_indqsel(dmah, FALSE);
		}

		status1 = R_REG(di->osh, &di->d64txregs->status1);

		if ((status1 & D64_XS1_XE_MASK) != D64_XS1_XE_NOERR)
			return TRUE;
		else if (si_coreid(di->sih) == GMAC_CORE_ID && si_corerev(di->sih) >= 4) {
			curr = DMA64_TX_CD(di);
			if (NTXDACTIVE(di->txin, di->txout) != 0 &&
				curr == di->xs0cd_snapshot) {

				/* suspicious */
				return TRUE;
			}
			di->xs0cd_snapshot = di->xs0cd = curr;

			return FALSE;
		}
		else
			return FALSE;
	}
	else {

		status1 = R_REG(di->osh, &di->d64rxregs->status1);

		if ((status1 & D64_RS1_RE_MASK) != D64_RS1_RE_NOERR)
			return TRUE;
		else
			return FALSE;
	}

	return FALSE;
}

static void
dma_param_set_nrxpost(dma_info_t *di, uint16 paramval)
{
	uint nrxd_max_usable;
	uint nrxpost_max;

	/* Maximum no.of desc that can be active is "nrxd - 1" */
	nrxd_max_usable = (di->nrxd - 1);

	if (di->sep_rxhdr) {
		nrxpost_max = nrxd_max_usable/2;
	} else {
		nrxpost_max = nrxd_max_usable;
	}

	di->nrxpost = MIN(paramval, nrxpost_max);
}

uint16
dma_param_get(hnddma_t *dmah, uint16 paramid)
{
	dma_info_t *di = DI_INFO(dmah);
	uint16 paramval = 0;

	switch (paramid) {
	case HNDDMA_PID_RX_BURSTLEN:
		paramval = di->rxburstlen;
		break;
	default:
		DMA_ERROR(("%s: dma_param_get: invalid paramid %d\n", di->name, paramid));
		ASSERT(0);
		break;
	}

	return paramval;
}

void
dma_param_set(hnddma_t *dmah, uint16 paramid, uint16 paramval)
{
	dma_info_t *di = DI_INFO(dmah);

	switch (paramid) {
	case HNDDMA_PID_TX_MULTI_OUTSTD_RD:
		di->txmultioutstdrd = (uint8)paramval;
		break;

	case HNDDMA_PID_TX_PREFETCH_CTL:
		di->txprefetchctl = (uint8)paramval;
		break;

	case HNDDMA_PID_TX_PREFETCH_THRESH:
		di->txprefetchthresh = (uint8)paramval;
		break;

	case HNDDMA_PID_TX_BURSTLEN:
		di->txburstlen = (uint8)paramval;
		break;

	case HNDDMA_PID_RX_PREFETCH_CTL:
		di->rxprefetchctl = (uint8)paramval;
		break;

	case HNDDMA_PID_RX_PREFETCH_THRESH:
		di->rxprefetchthresh = (uint8)paramval;
		break;

	case HNDDMA_PID_RX_BURSTLEN:
		di->rxburstlen = (uint8)paramval;
		break;

#ifndef DMA_WAIT_COMPLT_ROM_COMPAT
	case HNDDMA_PID_RX_WAIT_CMPL:
		di->rxwaitforcomplt = (uint8)paramval;
		break;
#endif

	case HNDDMA_PID_BURSTLEN_CAP:
		di->burstsize_ctrl = (uint8)paramval;
		break;

#if defined(D11_SPLIT_RX_FD)
	case HNDDMA_SEP_RX_HDR:
		di->sep_rxhdr = (uint8)paramval;	/* indicate sep hdr descriptor is used */
		break;
#endif /* D11_SPLIT_RX_FD */

	case HNDDMA_SPLIT_FIFO :
		di->split_fifo = (uint8)paramval;
		break;

	case HNDDMA_BCMRX_PCN_FIFO:
		di->bcmrx_pcn_fifo = (uint8)paramval;
		break;

	case HNDDMA_PID_D11RX_WAR:
		di->d11rx_war = (uint8)paramval;
		break;
	case HNDDMA_NRXPOST:
		dma_param_set_nrxpost(di, paramval);
		break;
	case HNDDMA_PID_TX_CHAN_SWITCH:
		di->txchanswitch = (uint8)paramval;
		break;
	case HNDDMA_NRXBUFSZ:
		di->rxbufsize = paramval;
		break;
	default:
		DMA_ERROR(("%s: _dma_param_set: invalid paramid %d\n", di->name, paramid));
		ASSERT(0);
		break;
	}
}

bool
dma_glom_enable(hnddma_t *dmah, uint32 val)
{
	dma_info_t *di = DI_INFO(dmah);

	dma64regs_t *dregs = di->d64rxregs;
	bool ret = TRUE;

	di->hnddma.dmactrlflags &= ~DMA_CTRL_SDIO_RXGLOM;
	if (val) {
		OR_REG(di->osh, &dregs->control, D64_RC_GE);
		if (!(R_REG(di->osh, &dregs->control) & D64_RC_GE))
			ret = FALSE;
		else
			di->hnddma.dmactrlflags |= DMA_CTRL_SDIO_RXGLOM;
	} else {
		AND_REG(di->osh, &dregs->control, ~D64_RC_GE);
	}
	return ret;
}

void
dma_context(hnddma_t *dmah, setup_context_t fn, void *ctx)
{
	dma_info_t * di = DI_INFO(dmah);

	di->fn = fn;
	di->ctx = ctx;
}

int
dma_rxfast(hnddma_t *dmah, dma64addr_t p, uint32 len)
{
	uint16 rxout;
	uint32 flags = 0;

	dma_info_t * di = DI_INFO(dmah);
	/*
	 * Determine how many receive buffers we're lacking
	 * from the full complement, allocate, initialize,
	 * and post them, then update the chip rx lastdscr.
	 */

	rxout = di->rxout;

	if ((di->nrxd - NRXDACTIVE(di->rxin, di->rxout) - 1) < 1)
		goto outofrxd;

	/* reset flags for each descriptor */
	if (rxout == (di->nrxd - 1))
		flags = D64_CTRL1_EOT;

	/* Update descriptor */
	dma64_dd_upd_64_from_params(di, di->rxd64, p, rxout, &flags, len);

	di->rxp[rxout] = (void *)(uintptr)(p.loaddr);

	rxout = NEXTRXD(rxout);

	di->rxout = rxout;

	di->rxavail = di->nrxd - NRXDACTIVE(di->rxin, di->rxout) - 1;
	/* update the chip lastdscr pointer */
	DMA64_RX_LD_UPD(di, rxout);
	return 0;
outofrxd:
	di->rxavail = 0;
	DMA_ERROR(("%s: dma_rxfast: out of rxds\n", di->name));
	return -1;
}

int
dma_ptrbuf_txfast(hnddma_t *dmah, dma64addr_t p0, void *p, bool commit,
	uint32 len, bool first, bool last)
{
	uint16 txout;
	uint32 flags = 0;
	dma_info_t * di = DI_INFO(dmah);

	DMA_TRACE(("%s: dma_txfast\n", di->name));

	ASSERT(!DMA_BULK_PATH(di));
	txout = di->txout;

	/* return nonzero if out of tx descriptors */
	if (NEXTTXD(txout) == di->txin)
		goto outoftxd;

	if (len == 0)
		return 0;

	if (first)
		flags |= D64_CTRL1_SOF;
	if (last)
		flags |= D64_CTRL1_EOF | D64_CTRL1_IOC;

	if (txout == (di->ntxd - 1))
		flags |= D64_CTRL1_EOT;

	dma64_dd_upd_64_from_params(di, di->txd64, p0, txout, &flags, len);

	ASSERT(di->txp[txout] == NULL);

	txout = NEXTTXD(txout);

	/* return nonzero if out of tx descriptors */
	if (txout == di->txin) {
		DMA_ERROR(("%s: dma_txfast: Out-of-DMA descriptors"
			   " (txin %d txout %d)\n", __FUNCTION__,
			   di->txin, di->txout));
		goto outoftxd;
	}

	/* save the packet */
	di->txp[PREVTXD(txout)] = p;

	/* bump the tx descriptor index */
	di->txout = txout;

	/* kick the chip */
	if (commit) {
		dma64_txcommit_local(di, FALSE);
	}

	/* tx flow control */
	di->hnddma.txavail = di->ntxd - NTXDACTIVE(di->txin, di->txout) - 1;

	DMA_TRACE(("dma_ptrbuf_txfast: di0x%p p0:0x%p\n", di, p));

	return (0);

outoftxd:
	DMA_ERROR(("%s: dma_txfast: out of txds !!!\n", di->name));
	di->hnddma.txavail = 0;
	di->hnddma.txnobuf++;
	return (-1);
}

int
dma_msgbuf_txfast(hnddma_t *dmah, dma64addr_t p0, bool commit, uint32 len, bool first, bool last)
{
	uint16 txout;
	uint32 flags = 0;
	dma_info_t * di = DI_INFO(dmah);

	DMA_TRACE(("%s: dma_txfast\n", di->name));

	/* if using indirect DMA access, then configure IndQSel */
	if (DMA_INDIRECT(di)) {
		dma_set_indqsel(dmah, FALSE);
	}

	ASSERT(!DMA_BULK_PATH(di));

	txout = di->txout;

	/* return nonzero if out of tx descriptors */
	if (NEXTTXD(txout) == di->txin)
		goto outoftxd;

	if (len == 0)
		return 0;

	if (first)
		flags |= D64_CTRL1_SOF;
	if (last)
		flags |= D64_CTRL1_EOF | D64_CTRL1_IOC;

	if (txout == (di->ntxd - 1))
		flags |= D64_CTRL1_EOT;

	dma64_dd_upd_64_from_params(di, di->txd64, p0, txout, &flags, len);

	ASSERT(di->txp[txout] == NULL);

	txout = NEXTTXD(txout);

	/* return nonzero if out of tx descriptors */
	if (txout == di->txin) {
		DMA_ERROR(("%s: dma_txfast: Out-of-DMA descriptors"
			   " (txin %d txout %d)\n", __FUNCTION__,
			   di->txin, di->txout));
		goto outoftxd;
	}

	/* save the packet */
	di->txp[PREVTXD(txout)] = (void *)(uintptr)(p0.loaddr);

	/* bump the tx descriptor index */
	di->txout = txout;

	/* kick the chip */
	if (commit) {
		dma64_txcommit_local(di, FALSE);
	}

	/* tx flow control */
	di->hnddma.txavail = di->ntxd - NTXDACTIVE(di->txin, di->txout) - 1;
	DMA_TRACE(("dma_msgbuf_txfast: di0x%p p0:0x%p\n", di, (void *)(uintptr)(p0.loaddr)));

	return (0);

outoftxd:
	DMA_ERROR(("%s: dma_txfast: out of txds !!!\n", di->name));
	di->hnddma.txavail = 0;
	di->hnddma.txnobuf++;
	return (-1);
}

void
dma_link_handle(hnddma_t *dmah1, hnddma_t *dmah2)
{
	dma_info_t *di1 = DI_INFO(dmah1);
	dma_info_t *di2 = DI_INFO(dmah2);
	di1->linked_di = di2;
	di2->linked_di = di1;
}

void
dma_rxchan_reset(hnddma_t *dmah)
{
	dma_info_t *di = DI_INFO(dmah);
	uint size;

	AND_REG(di->osh, &di->d64rxregs->control, ~D64_RC_RE);
	OR_REG(di->osh, &di->d64rxregs->control, D64_RC_RE);

	size = di->nrxd * sizeof(void *);
	bzero(di->rxp, size);
}

void
dma_txchan_reset(hnddma_t *dmah)
{
	dma_info_t *di = DI_INFO(dmah);
	uint size;

	AND_REG(di->osh, &di->d64txregs->control, ~D64_XC_XE);
	OR_REG(di->osh, &di->d64txregs->control, D64_XC_XE);

	size = di->ntxd * sizeof(void *);
	bzero(di->txp, size);
}

void BCMFASTPATH
dma_cleartxp(hnddma_t *dmah)
{
	dma_info_t *di = DI_INFO(dmah);
	uint16 start, end;

	if (di->ntxd == 0)
		return;

	start = di->txin;

	if (di->txin == di->xs0cd) {
		end = DMA64_TX_CD(di);
		di->xs0cd = end;
	} else {
		end = di->xs0cd;
	}

	if ((start == 0) && (end > di->txout))
		goto bogus;

	while (start != end) {
#ifdef BULK_PKTLIST_DEBUG
		ASSERT(di->txp[start]);
		di->txp[start] = NULL;
#else
		if (!DMA_BULK_PATH(di)) {
			ASSERT(di->txp[start]);
			di->txp[start] = NULL;
		}
#endif

		W_SM(&di->txd64[start].addrlow, 0xdeadbeef);
		W_SM(&di->txd64[start].addrhigh, 0xdeadbeef);
		start = NEXTTXD(start);
	}

	di->txin = start;

	di->hnddma.txavail = di->ntxd - NTXDACTIVE(di->txin, di->txout) - 1;

	return;

bogus:
	DMA_NONE(("dma_cleartxp: bogus curr: start %d end %d txout %d\n",
	          start, end, di->txout));

	OSL_SYS_HALT();
	return;
}

/* Given a M2M request, configure the Rx and Tx descriptors of the M2M channels
 * in the PCIE core. Does not apply to M2MCORE
 */
int BCMFASTPATH
pcie_m2m_req_submit(hnddma_t *dmah, pcie_m2m_req_t *m2m_req, bool implicit)
{
	uint16 rxout, txout, num_avail;
	uint32 save_rxd, save_txd;
	pcie_m2m_vec_t *vec;
	uint8 num_vec, offset;
	uint32 flags;

	dma_info_t *di = DI_INFO(dmah);

	/* Sanity check */
	ASSERT((m2m_req->num_rx_vec + m2m_req->num_tx_vec) <= m2m_req->vec_max);

	num_avail = di->ntxd - NTXDACTIVE(di->txin, di->txout) - 1;
	if (num_avail < m2m_req->num_tx_vec) {
		goto outoftxd;
	}

	num_avail = di->nrxd - NRXDACTIVE(di->rxin, di->rxout) - 1;
	if (num_avail < m2m_req->num_rx_vec) {
		goto outofrxd;
	}

	rxout = di->rxout;

	for (num_vec = 0; num_vec < m2m_req->num_rx_vec; num_vec++) {
		vec = &m2m_req->vec[num_vec];
		save_rxd = vec->addr64.loaddr;
		flags = 0;

		if (rxout == (di->nrxd - 1))
			flags = D64_CTRL1_EOT;

		if (m2m_req->flags & XFER_TO_LBUF) {
			vec->addr64.loaddr = (uint32)(uintptr)PKTDATA(di->osh,
					(void *)(uintptr)(vec->addr64.loaddr));
		}

		ASSERT(vec->len);
		dma64_dd_upd_64_from_params(di, di->rxd64, vec->addr64,
				rxout, &flags, vec->len);

		ASSERT(di->rxp[rxout] == NULL);
		di->rxp[rxout] = (void *)(uintptr)(save_rxd);
		rxout = NEXTRXD(rxout);
	}

	di->rxout = rxout;
	di->rxavail = di->nrxd - NRXDACTIVE(di->rxin, di->rxout) - 1;
	DMA64_RX_LD_UPD(di, rxout);

	txout = di->txout;
	offset = m2m_req->num_rx_vec;

	for (num_vec = 0; num_vec < m2m_req->num_tx_vec; num_vec++) {
		vec = &m2m_req->vec[num_vec + offset];
		save_txd = vec->addr64.loaddr;
		flags = 0;

		if ((num_vec == 0) || implicit) {
			flags |= D64_CTRL1_SOF;
		}

		if ((num_vec == (m2m_req->num_tx_vec - 1)) || implicit) {
			flags |= D64_CTRL1_EOF | D64_CTRL1_IOC;
		}

		if (txout == (di->ntxd - 1)) {
			flags |= D64_CTRL1_EOT;
		}

		if (m2m_req->flags & XFER_FROM_LBUF) {
			vec->addr64.loaddr = (uint32)(uintptr)PKTDATA(di->osh,
					(void *)(uintptr)(vec->addr64.loaddr));
		}

		if (m2m_req->flags & XFER_INJ_ERR) {
			flags &= ~(D64_CTRL1_EOF | D64_CTRL1_SOF);
		}

		ASSERT(vec->len);
		dma64_dd_upd_64_from_params(di, di->txd64, vec->addr64,
			txout, &flags, vec->len);

		ASSERT(di->txp[txout] == NULL);
		di->txp[txout] = (void *)(uintptr)(save_txd);
		txout = NEXTTXD(txout);
	}

	di->txout = txout;
	di->hnddma.txavail = di->ntxd - NTXDACTIVE(di->txin, di->txout) - 1;

	if (m2m_req->commit) {
		dma64_txcommit_local(di, FALSE);
	}

	DMA_TRACE(("pcie_m2m_req_submit: di:0x%p m2m_req:0x%p\n", di, m2m_req));

	return 0;

outofrxd:
	DMA_ERROR(("%s: pcie_m2m_req_submit: out of rxds\n", di->name));
	di->rxavail = 0;
	return -1;

outoftxd:
	DMA_ERROR(("%s: pcie_m2m_req_submit: out of txds\n", di->name));
	di->hnddma.txavail = 0;
	di->hnddma.txnobuf++;
	return -1;
}

void
dma_chan_enable(hnddma_t *dmah, bool enable)
{
	dma_info_t *di = DI_INFO(dmah);
	uint32 status;

	if (enable) {
		OR_REG(di->osh, &di->d64txregs->control, D64_XC_XE);
		OR_REG(di->osh, &di->d64rxregs->control, D64_RC_RE);

		dma_descptr_update(dmah);
	} else {
		/* If DMA is already in reset, do not reset. */
		if ((R_REG(di->osh, &di->d64txregs->status0) & D64_XS0_XS_MASK) ==
			D64_XS0_XS_DISABLED) {
			return;
		}

		/* suspend tx DMA first */
		OR_REG(di->osh, &di->d64txregs->control, D64_XC_SE);
		SPINWAIT(((status = (R_REG(di->osh, &di->d64txregs->status0) & D64_XS0_XS_MASK)) !=
				D64_XS0_XS_DISABLED) &&
				(status != D64_XS0_XS_IDLE) &&
				(status != D64_XS0_XS_STOPPED),
				10000);

		AND_REG(di->osh, &di->d64txregs->control, ~D64_XC_XE);
		SPINWAIT(((status = (R_REG(di->osh, &di->d64txregs->status0) & D64_XS0_XS_MASK)) !=
				D64_XS0_XS_DISABLED),
				10000);

		/* We should be disabled at this point */
		if (status != D64_XS0_XS_DISABLED) {
			DMA_ERROR(("%s: status != D64_XS0_XS_DISABLED 0x%x\n",
				__FUNCTION__, status));
			ASSERT(status == D64_XS0_XS_DISABLED);
			OSL_DELAY(300);
		}

		AND_REG(di->osh, &di->d64txregs->control, ~D64_XC_SE);
		AND_REG(di->osh, &di->d64rxregs->control, ~D64_RC_RE);
	}
}

static void
dma_descptr_update(hnddma_t *dmah)
{
	dma_info_t *di = DI_INFO(dmah);

	DMA64_RX_LD_UPD(di, di->rxout);
	dma64_txcommit_local(di, FALSE);
}

/* Get physical address of descriptor ring */
void
dma_get_fifo_info(hnddma_t *dmah, dmaaddr_t *pa, uint *depth, bool tx)
{
	dma_info_t * di = DI_INFO(dmah);

	ASSERT(pa);
	ASSERT(depth);

	if (tx) {
		*pa = di->txdpa;
		*depth = di->ntxd;
	}
	else {
		*pa = di->rxdpa;
		*depth = di->nrxd;
	}

	return;
}

#ifdef BCM_HWALITE
/* TODO: Merge with dma_get_fifo_info */
void
dma_get_di_rxinfo(hnddma_t *dmah, uint32 *nrxpost, uint64 *fifo_addr_va,
	uint32 *rcvptrbase)
{
	dma_info_t * di = DI_INFO(dmah);

	*nrxpost = di->nrxpost;
	*fifo_addr_va = (uint64)((uintptr)di->rxd64);
	*rcvptrbase = (uint32)di->rcvptrbase;

	return;
}
#endif /* BCM_HWALITE */

int
dmatx_map_pkts(hnddma_t *dmah, map_pkts_cb_fn cb, void *ctx)
{
	uint16 start, end, i;
	dma_info_t *di = DI_INFO(dmah);

	DMA_TRACE(("%s: dmatx_map_pkts\n", di->name));

	if (di->ntxd == 0) {
		return BCME_ERROR;
	}

#ifdef BULK_PKTLIST
	/* Walk through the DMA pkt list */
	if (DMA_BULK_PATH(di)) {
		void *txp = di->dma_pkt_list.head_pkt;

		while (txp) {
			/* Do map call back */
			(void)cb(ctx, txp);

			txp = PKTLINK(txp);
		}

		return BCME_OK;
	}
#endif

	start = di->txin;
	end = di->txout;

	for (i = start; i != end; i = NEXTTXD(i)) {
		if (di->txp[i] != NULL) {
			/* ignoring the return 'delete' bool since hnddma
			 * does not allow deleting pkts on the ring.
			 */
			(void)cb(ctx, di->txp[i]);
		}
	}
	return BCME_OK;
}

#if defined(PKTQ_STATUS) && defined(BULK_PKTLIST)
/* Utility function to walk the DMA ring */
void *
dma_get_nextpkt(hnddma_t *dmah, void *pkt)
{
	dma_info_t *di = DI_INFO(dmah);
	if (pkt == NULL) {
		pkt = DMA_GET_LISTHEAD(di);
	} else if (pkt == DMA_GET_LISTTAIL(di)) {
		pkt = NULL;
	} else {
		pkt = DMA_GET_NEXTPKT(pkt);
	}
	return pkt;
}
#endif

#ifdef BCMHME
/* Allocate memory from HME pool */
static void *
_dma_hme_ringalloc(dma_info_t *di, dma_hme_sgmt_t *sgmt, uint32 boundary, uint size,
	uint16 *alignbits, uint *alloced, dma64addr_t *descpa)
{
	void *va;
	uint32 desc_strtaddr;
	uint32 alignbytes = 1 << *alignbits;

	/* Find alignment address */
	desc_strtaddr = ROUNDUP(sgmt->hmecurr, alignbytes);
	if (desc_strtaddr + size > PHYSADDR64LO(sgmt->hmeaddr) + sgmt->hmelen) {
		return NULL;
	}

	/* Check boundary */
	if (((desc_strtaddr + size - 1) & boundary) != (desc_strtaddr & boundary)) {
		*alignbits = dma_align_sizetobits(size, di->hnddma.dmactrlflags);
		alignbytes = 1 << *alignbits;
		desc_strtaddr = ROUNDUP(sgmt->hmecurr, alignbytes);
		if (desc_strtaddr + size > PHYSADDR64LO(sgmt->hmeaddr) + sgmt->hmelen)
			return NULL;
	}

	/* Check if hit the 1MB boundary */
	if  (di->hnddma.dmactrlflags & DMA_CTRL_DESC_BASE_IDX_UPD) {
		if (((desc_strtaddr & (D64RINGBOUNDARY_LARGE_IDX_MODE - 1)) + size) ==
			D64RINGBOUNDARY_LARGE_IDX_MODE) {
			*alignbits = D64RINGBOUNDARY_LARGE_IDX_MODE_BITS;
			alignbytes = 1 << *alignbits;
			desc_strtaddr = ROUNDUP(sgmt->hmecurr, alignbytes);
			if (desc_strtaddr + size > PHYSADDR64LO(sgmt->hmeaddr) + sgmt->hmelen)
				return NULL;
		}
	}

	/* Decide the address */
	sgmt->hmecurr = desc_strtaddr + size;
	PHYSADDR64LOSET(*descpa, desc_strtaddr);
	PHYSADDR64HISET(*descpa, PHYSADDR64HI(sgmt->hmeaddr));
	*alloced = size;
	va = (void *)desc_strtaddr;

	return va;
}

static void *
dma_hme_ringalloc(dma_info_t *di, uint32 boundary, uint size, uint16 *alignbits,
	uint *alloced, dma64addr_t *descpa)
{
	int i;
	void *va = NULL;
	dma_hme_sgmt_t *sgmt;
	dma_common_t *dmac = di->dmacommon;

	for (i = 0; i < dmac->sgmt_avail; i++) {
		sgmt = dmac->sgmt + i;
		va = _dma_hme_ringalloc(di, sgmt, boundary, size, alignbits, alloced, descpa);
		if (va != NULL)
			break;
	}

	return va;
}

int
dma_common_hme_upd(dma_common_t *dmac, pcie_ipc_hme_sgmt_t *hme_sgmt, uint16 sgmt_avail)
{
	int i;
	uint32 size, hiaddr;
	pcie_ipc_hme_sgmt_t *sgmt;
	dma_hme_sgmt_t *dmac_sgmt;

	ASSERT(dmac->sgmt == NULL);
	ASSERT(sgmt_avail);

	// Save segments to dmac
	size = sgmt_avail * sizeof(dma_hme_sgmt_t);
	if ((dmac_sgmt = MALLOCZ(dmac->osh, size)) == NULL) {
		DMA_PRINT(("%s: out of memory, malloced %d bytes\n",
			__FUNCTION__, size));
		return BCME_NOMEM;
	}

	dmac->sgmt_avail = sgmt_avail;
	dmac->sgmt = dmac_sgmt;

	sgmt = hme_sgmt;
	hiaddr = HADDR64_HI(sgmt->haddr64);
	for (i = 0; i < sgmt_avail; i++) {
		if (hiaddr != HADDR64_HI(sgmt->haddr64)) {
			ASSERT(0);
		}
		PHYSADDR64HISET(dmac_sgmt->hmeaddr, HADDR64_HI(sgmt->haddr64));
		PHYSADDR64LOSET(dmac_sgmt->hmeaddr, HADDR64_LO(sgmt->haddr64));
		dmac_sgmt->hmelen = (uint)sgmt->bytes;
		dmac_sgmt->hmecurr = HADDR64_LO(sgmt->haddr64);
		dmac_sgmt++;
		sgmt++;
	}

	return BCME_OK;
}

void
dma_common_hme_dump(dma_common_t *dmac, struct bcmstrbuf *b)
{
	int i;
	uint32 tot_bytes;
	dma_hme_sgmt_t *sgmt;

	if (!dmac->sgmt)
		return;

	ASSERT(dmac->sgmt_avail);

	tot_bytes = 0;
	for (i = 0; i < dmac->sgmt_avail; i++) {
		sgmt = dmac->sgmt + i;
		tot_bytes += sgmt->hmelen;
	}

	if (b) {
		bcm_bprintf(b, "BCM HME MACIFS: segment avail %u total bytes %u\n",
			dmac->sgmt_avail, tot_bytes);
		bcm_bprintf(b, "\tSGMT   BYTES HADDR_HI HADDR_LO HADDR_CR   AVAIL\n");
	} else {
		DMA_PRINT(("BCM HME MACIFS: segment avail %u total bytes %u\n",
			dmac->sgmt_avail, tot_bytes));
		DMA_PRINT(("\tSGMT   BYTES HADDR_HI HADDR_LO HADDR_CR   AVAIL\n"));
	}

	for (i = 0; i < dmac->sgmt_avail; i++) {
		sgmt = dmac->sgmt + i;
		if (b) {
			bcm_bprintf(b, "\t%4u %7u %08x %08x %08x %7u\n", i,
				sgmt->hmelen, PHYSADDR64HI(sgmt->hmeaddr),
				PHYSADDR64LO(sgmt->hmeaddr), sgmt->hmecurr,
				(sgmt->hmelen - (sgmt->hmecurr - PHYSADDR64LO(sgmt->hmeaddr))));
		} else {
			DMA_PRINT(("\t%4u %7u %08x %08x %08x %7u\n", i,
				sgmt->hmelen, PHYSADDR64HI(sgmt->hmeaddr),
				PHYSADDR64LO(sgmt->hmeaddr), sgmt->hmecurr,
				(sgmt->hmelen - (sgmt->hmecurr - PHYSADDR64LO(sgmt->hmeaddr)))));
		}
	}
}

int
dma64_hme_alloc(hnddma_t *dmah, uint *flags)
{
	void *va;
	uint32 align, size, boundary;
	uint16 align_bits;
	uint alloced;
	dma64addr_t haddr64;
	dma_info_t *di = DI_INFO(dmah);

	ASSERT(flags);

	alloced = 1;
	align_bits = di->dmadesc_align;
	align = (1 << align_bits);
	*flags = 0;

	ASSERT(flags);

	if (DMA_HWA_HME_TX(di)) {

		ASSERT(di->txd64 == NULL);

		size = (di->ntxd * di->dd64_size);

		/* The 64k boundary of the HW limitation is only for 6726A0 MLO,
		 * it will cost more HME memory when allocating.
		 * So only apply the flag DMA_CTRL_BOUNDRY64KB_WAR for 6726A0.
		 * There is no such issue in 6726B0. It could use 1MB boundary to
		 * save the HME memory usage.
		 */
		if  (di->hnddma.dmactrlflags & DMA_CTRL_DESC_BASE_IDX_UPD) {
			if (!(di->hnddma.dmactrlflags & DMA_CTRL_BOUNDRY64KB_WAR) ||
				(size > D64RINGBOUNDARY_LARGE)) {
				boundary = D64RINGBOUNDARY_LARGE_IDX_MODE;
			} else {
				boundary = D64RINGBOUNDARY_LARGE;
			}
		} else if (di->d64_xs0_cd_mask == 0x1fff) {
			boundary = D64RINGBOUNDARY;
		} else {
			boundary = D64RINGBOUNDARY_LARGE;
		}

		if ((va = dma_hme_ringalloc(di, boundary,
			size, &align_bits, &alloced, &haddr64)) == NULL) {
			DMA_ERROR(("%s(): %s: dma_hme_ringalloc(ntxd) failed\n",
				__FUNCTION__, di->name));
			return BCME_NOMEM;
		}

		align = (1 << align_bits);

		PHYSADDRLOSET(di->txdpaorig, PHYSADDR64LO(haddr64));
		PHYSADDRHISET(di->txdpaorig, PHYSADDR64HI(haddr64));

		/* adjust the pa by rounding up to the alignment */
		PHYSADDRLOSET(di->txdpa, ROUNDUP(PHYSADDRLO(di->txdpaorig), align));
		PHYSADDRHISET(di->txdpa, PHYSADDRHI(di->txdpaorig));

		/* Save high address of txdpa */
		di->txdpahi = PHYSADDR64HI(haddr64);

		/* Make sure that alignment didn't overflow */
		ASSERT(PHYSADDRLO(di->txdpa) >= PHYSADDRLO(di->txdpaorig));

		/* find the alignment offset that was used */
		di->txdalign = (uint)(PHYSADDRLO(di->txdpa) - PHYSADDRLO(di->txdpaorig));

		/* adjust the va by the same offset */
		di->txd64 = (dma64dd_t *)((uintptr)va + di->txdalign);

		di->txdalloc = alloced;

		ASSERT(ISALIGNED(PHYSADDRLO(di->txdpa), align));

		if (!di->aligndesc_4k)
			di->xmtptrbase = PHYSADDRLO(di->txdpa);

		*flags |= BCM_DMA_HWA_HME_TX;
	}

	if (DMA_HWA_HME_RX(di)) {

		ASSERT(di->rxd64 == NULL);

		size = (di->nrxd * sizeof(dma64dd_t));

		if  (di->hnddma.dmactrlflags & DMA_CTRL_DESC_BASE_IDX_UPD) {
			boundary = D64RINGBOUNDARY_LARGE_IDX_MODE;
		} else if (di->d64_rs0_cd_mask == 0x1fff) {
			boundary = D64RINGBOUNDARY;
		} else {
			boundary = D64RINGBOUNDARY_LARGE;
		}

		if ((va = dma_hme_ringalloc(di, boundary,
			size, &align_bits, &alloced, &haddr64)) == NULL) {
			DMA_ERROR(("%s(): %s: dma_hme_ringalloc(nrxd) failed\n",
				__FUNCTION__, di->name));
			return BCME_NOMEM;
		}

		align = (1 << align_bits);

		PHYSADDRLOSET(di->rxdpaorig, PHYSADDR64LO(haddr64));
		PHYSADDRHISET(di->rxdpaorig, PHYSADDR64HI(haddr64));

		/* adjust the pa by rounding up to the alignment */
		PHYSADDRLOSET(di->rxdpa, ROUNDUP(PHYSADDRLO(di->rxdpaorig), align));
		PHYSADDRHISET(di->rxdpa, PHYSADDRHI(di->rxdpaorig));

		/* Save high address of txdpa */
		di->rxdpahi = PHYSADDR64HI(haddr64);

		/* Make sure that alignment didn't overflow */
		ASSERT(PHYSADDRLO(di->rxdpa) >= PHYSADDRLO(di->rxdpaorig));

		/* find the alignment offset that was used */
		di->rxdalign = (uint)(PHYSADDRLO(di->rxdpa) - PHYSADDRLO(di->rxdpaorig));

		/* adjust the va by the same offset */
		di->rxd64 = (dma64dd_t *)((uintptr)va + di->rxdalign);

		di->rxdalloc = alloced;

		ASSERT(ISALIGNED(PHYSADDRLO(di->rxdpa), align));

		if (!di->aligndesc_4k)
			di->rcvptrbase = PHYSADDRLO(di->rxdpa);

		*flags |= BCM_DMA_HWA_HME_RX;
	}

	return BCME_OK;
} /* dma64_hme_alloc */
#endif /* BCMHME */

#if defined(WL_MLO)
void
dma64_mlo_hme_tx_update(hnddma_t *dmah, dma64addr_t addr, uint32 fifo_depth)
{
	dma_info_t *di = DI_INFO(dmah);

	di->txd64 = (dma64dd_t *)((uintptr)(addr.loaddr));
	PHYSADDRLOSET(di->txdpa, PHYSADDR64LO(addr));
	PHYSADDRHISET(di->txdpa, PHYSADDR64HI(addr));
	di->txdpahi = PHYSADDR64HI(addr);
	di->xmtptrbase = PHYSADDRLO(di->txdpa);
	di->ntxd = fifo_depth;
}

/* Update rxoffset */
void
dma_set_mlo_rxoffset(hnddma_t *dmah, uint rxoffset)
{
	dma_info_t *di = DI_INFO(dmah);

	if (di && di->rxoffset != rxoffset) {
		di->rxoffset = (uint8)rxoffset;
	}
}

#endif /* WL_MLO */

#ifdef WL_EAP_REKEY_WAR
void
dma_set_dbgprnt(hnddma_t *dmah, bool disable)
{
	dma_info_t *di = DI_INFO(dmah);
	di->dis_msgs = disable;
}
#endif /* WL_EAP_REKEY_WAR */

#ifdef BCM_HWALITE
void
dma_set_mlo_param(hnddma_t *dmah, void *mlc_dev, uint32 fifo, bool is_aqm)
{
	dma_info_t *di  = DI_INFO(dmah);

	di->mlc_dev     = mlc_dev;
	di->fifo        = fifo;
	di->is_aqm      = is_aqm;
}
#endif /* BCM_HWALITE */

#ifdef BCM_PCAP
/* Allocate the larger RX FIFO 2 descriptor ring for promiscuous RX. Assumption: down state. */
static int
dma_pcap_alloc(dma_info_t *di, uint16 nrxd_alloc)
{
	uint32 size;
	uint32 nrxd_orig = di->nrxd;

	if (di->nrxd >= nrxd_alloc) {
		/* already initialized */
		return BCME_OK;
	}

	ASSERT(di->rxin == di->rxout);
	/* de-allocate the existing ring: following similar steps as dma_detach() */

	DMA_PRINT(("%s: upsize %d->%d RXFIFO2 dma ring\n", __FUNCTION__, di->nrxd, nrxd_alloc));

	/* free rx dma descriptor ring */
	if (di->rxd64) {
		DMA_FREE_CONSISTENT(di->osh, ((int8 *)(uintptr)di->rxd64 - di->rxdalign),
		                    di->rxdalloc, (di->rxdpaorig), di->rx_dmah);
		/* re-allocate */
		di->nrxd = nrxd_alloc;
		if (! dma64_alloc_rx(di)) {
			goto fail;
		}
	}

	/* free packet pointer vectors */
	if (di->rxp) {
		MFREE(di->osh, (void *)di->rxp, (nrxd_orig * sizeof(void *)));
		/* re-allocate */
		size = di->nrxd * sizeof(void *);
		if ((di->rxp = MALLOCZ(di->osh, size)) == NULL) {
			goto fail;
		}
	}

	/* free rx packet DMA handles */
	if (di->rxp_dmah) {
		MFREE(di->osh, (void *)di->rxp_dmah, nrxd_orig * sizeof(hnddma_seg_map_t));
		/* re-allocate */
		size = di->nrxd * sizeof(hnddma_seg_map_t);
		if ((di->rxp_dmah = (hnddma_seg_map_t *)MALLOCZ(di->osh, size)) == NULL) {
			goto fail;
		}
	}
	/* success */
	return BCME_OK;
fail:
	DMA_ERROR(("%s: %s: out of rx memory, malloced %d bytes\n",
		di->name, __FUNCTION__, MALLOCED(di->osh)));
	ASSERT(0);
	return BCME_NOMEM;
}

/* PCIE FD, set the RX FIFO 2 for PCAP. Posted RX buffers are in host memory */
void
dma_pcap_set(hnddma_t *dmah, bool en, uint16 nrxpost, uint16 nrxd)
{
	dma_info_t *di = DI_INFO(dmah);
	int err = BCME_OK;

	ASSERT(di);

	if (en) {
		/* first time initialization */
		err = dma_pcap_alloc(di, nrxd);
	}

	if (err == BCME_OK) {
		dma_param_set_nrxpost(di, nrxpost);

		DMA_PCAP_EN(di) = en;
	}
}

void
dma_pcap_attach(dma_common_t *dmacommon, void *pcap_hdl)
{
	ASSERT(pcap_hdl != NULL);
	dmacommon->pcap_hdl = pcap_hdl;
}
#endif /* BCM_PCAP */
