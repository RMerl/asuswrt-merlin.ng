/*
 * Generic Broadcom Home Networking Division (HND) DMA module.
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
 * $Id: hnddma.c 784528 2020-02-28 22:23:04Z $
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

#if defined(DONGLEBUILD) && defined(SRMEM)
#include <hndsrmem.h>
#endif /* DONGLEBUILD && SRMEM */

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
static bool _dma_alloc(dma_info_t *di, uint direction);
static void _dma_ddtable_init(dma_info_t *di, uint direction, dmaaddr_t pa);

static uint8 dma_align_sizetobits(uint size);
static void *dma_ringalloc(osl_t *osh, uint32 boundary, uint size, uint16 *alignbits, uint* alloced,
	dmaaddr_t *descpa, osldma_t **dmah);
static bool _dma32_addrext(osl_t *osh, dma32regs_t *dma32regs);

/* Prototypes for 64-bit routines */
static bool dma64_alloc(dma_info_t *di, uint direction);

static bool _dma64_addrext(osl_t *osh, dma64regs_t *dma64regs);

static void dma_param_set_nrxpost(dma_info_t *di, uint16 paramval);
static void dma_descptr_update(hnddma_t *dmah);

/*
 * This function needs to be called during initialization before calling dma_attach_ext.
 */
dma_common_t *
BCMATTACHFN_DMA_ATTACH(dma_common_attach)(osl_t *osh, volatile uint32 *indqsel,
	volatile uint32 **suspreq, volatile uint32 **flushreq, uint reg_count)
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

	return dmac;
}

void
dma_common_detach(dma_common_t *dmacommon)
{
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
	if (DMA_CTRL_IS_HWA_TX(di)) {
		DMA_TRACE(("HWATX: DMA: SW try updating indqsel to <%d:%d>%s\n",
			dmacommon->last_qsel, di->q_index,
			(override) ? " override" : ""));
	}
#endif // endif

	if ((dmacommon->last_qsel != di->q_index) || override) {
		/* configure the indqsel register */
		W_REG(di->osh, dmacommon->indqsel, di->q_index);

		/* also update the global state dmac */
		dmacommon->last_qsel = di->q_index;
	}
	return;
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

#ifdef EVENT_LOG_COMPILE
	event_log_tag_start(EVENT_LOG_TAG_DMA_ERROR, EVENT_LOG_SET_ERROR, EVENT_LOG_TAG_FLAG_LOG);
#endif // endif
	/* allocate private info structure */
	if ((di = MALLOC(osh, sizeof (dma_info_t))) == NULL) {
#if defined(BCMDBG) || defined(WLC_BCMDMA_ERRORS)
		DMA_ERROR(("%s: out of memory, malloced %d bytes\n", __FUNCTION__, MALLOCED(osh)));
		OSL_SYS_HALT();
#endif // endif
		return (NULL);
	}

	bzero(di, sizeof(dma_info_t));

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
		ASSERT((qnum / 32) < dmac->reg_count);
		di->q_index_mask = (1 << (qnum % 32));
		di->suspreq	 = dmac->suspreq[qnum / 32];
		di->flushreq	 = dmac->flushreq[qnum / 32];
	}
#endif /* BCM_DMA_INDIRECT */

	di->msg_level = msg_level ? msg_level : &dma_msg_level;

	/* old chips w/o sb is no longer supported */
	ASSERT(sih != NULL);

	/* check arguments */
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

	/* check flags for descriptor only DMA */
	if (flags & BCM_DMA_DESC_ONLY_FLAG) {
		di->hnddma.dmactrlflags |= DMA_CTRL_DESC_ONLY_FLAG;
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
#endif // endif

#ifdef BULKRX_PKTLIST	/* Datapath bulk processing */
	if (flags & BCM_DMA_BULKRX_PROCESSING) {
		di->hnddma.dmactrlflags |= DMA_CTRL_BULKRX_PROCESSING;
	}
#endif // endif

#ifdef BCMHWA
	if (flags & BCM_DMA_HWA_MACRXFIFO) {
		di->hnddma.dmactrlflags |= DMA_CTRL_HWA_RX;
	}
	if (flags & BCM_DMA_HWA_MACTXFIFO) {
		di->hnddma.dmactrlflags |= DMA_CTRL_HWA_TX;
	}
#endif // endif

	DMA_TRACE(("%s: %s: osh %p flags 0x%x ntxd %d nrxd %d rxbufsize %d "
		   "rxextheadroom %d nrxpost %d rxoffset %d dmaregstx %p dmaregsrx %p\n",
		   name, __FUNCTION__,
		   OSL_OBFUSCATE_BUF(osh), di->hnddma.dmactrlflags, ntxd, nrxd,
		   rxbufsize, rxextheadroom, nrxpost, rxoffset, OSL_OBFUSCATE_BUF(dmaregstx),
		   OSL_OBFUSCATE_BUF(dmaregsrx)));
#ifdef BCM_DMA_INDIRECT
	DMA_TRACE(("%s: %s: indirect DMA %s, q_index %d \n",
		name, __FUNCTION__, (di->indirect_dma ? "TRUE" : "FALSE"), di->q_index));
#endif // endif
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

		if (mask == 0x1fff)
			ASSERT(nrxd <= D64MAXDD);
		else
			ASSERT(nrxd <= D64MAXDD_LARGE);

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

		if (mask & 0xfff)
			mask = R_REG(di->osh, &di->d64txregs->ptr) | 0xf;
		else
			mask = 0x1fff;

		DMA_TRACE(("%s: dma_tx_mask: %08x\n", di->name, mask));
		di->d64_xs0_cd_mask = mask;
		di->d64_xs1_ad_mask = mask;

		if (mask == 0x1fff)
			ASSERT(ntxd <= D64MAXDD);
		else
			ASSERT(ntxd <= D64MAXDD_LARGE);

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
			di->ddoffsethigh = SI_PCIE_DMA_H32;
		} else {
			/* pci(DMA32/DMA64) or pcie with DMA32 */
			di->ddoffsetlow = SI_PCI_DMA;
			di->ddoffsethigh = 0;
		}
		di->dataoffsetlow =  di->ddoffsetlow;
		di->dataoffsethigh =  di->ddoffsethigh;
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
#endif // endif
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
	if ((!(di->hnddma.dmactrlflags & DMA_CTRL_DESC_ONLY_FLAG)) && (ntxd)) {
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
	} else if ((!(di->hnddma.dmactrlflags & DMA_CTRL_DESC_ONLY_FLAG)) && (ntxd) &&
		!DMA_CTRL_IS_HWA_TX(di)) {
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
	if (nrxd && !DMA_CTRL_IS_HWA_RX(di)) {
		size = nrxd * sizeof(void *);
		if ((di->rxp = MALLOC(osh, size)) == NULL) {
			DMA_ERROR(("%s: %s: out of rx memory, malloced %d bytes\n",
				di->name, __FUNCTION__, MALLOCED(osh)));
			goto fail;
		}
		bzero(di->rxp, size);
	}

	/* allocate transmit descriptor ring, only need ntxd descriptors but it must be aligned */
	if (ntxd) {
		if (!_dma_alloc(di, DMA_TX)) /* does not allocate buffers nor init descriptors */
			goto fail;
	}

	/* allocate receive descriptor ring, only need nrxd descriptors but it must be aligned */
	if (nrxd) {
		if (!_dma_alloc(di, DMA_RX)) /* does not allocate buffers nor init descriptors */
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

	DMA_TRACE(("ddoffsetlow 0x%x ddoffsethigh 0x%x dataoffsetlow 0x%x dataoffsethigh "
	           "0x%x addrext %d\n", di->ddoffsetlow, di->ddoffsethigh, di->dataoffsetlow,
	           di->dataoffsethigh, di->addrext));

	if (!di->aligndesc_4k) {
		di->xmtptrbase = PHYSADDRLO(di->txdpa);
		di->rcvptrbase = PHYSADDRLO(di->rxdpa);
	}

	/* allocate DMA mapping vectors */
	if (DMASGLIST_ENAB) {
		if (ntxd && !DMA_CTRL_IS_HWA_TX(di)) {
			size = ntxd * sizeof(hnddma_seg_map_t);
			if ((di->txp_dmah = (hnddma_seg_map_t *)MALLOC(osh, size)) == NULL)
				goto fail;
			bzero(di->txp_dmah, size);
		}

		/* We don't support sglist when HWA is enabled */
		if (nrxd && !DMA_CTRL_IS_HWA_RX(di)) {
			size = nrxd * sizeof(hnddma_seg_map_t);
			if ((di->rxp_dmah = (hnddma_seg_map_t *)MALLOC(osh, size)) == NULL)
				goto fail;
			bzero(di->rxp_dmah, size);
		}
	}

	dma_attach_cond_config(di);

#if defined(BULK_DESCR_FLUSH)
	/* init the bulk_descr_tx_start_txout ... no DMA USER BULK TX ACTIVE */
	DMA_BULK_DESCR_TX_SET_INVALID(di);
#endif // endif

	return ((hnddma_t *)di);

fail:
#ifdef WLC_BCMDMA_ERRORS
	DMA_ERROR(("%s: %s: DMA attach failed \n", di->name, __FUNCTION__));
	OSL_SYS_HALT();
#endif // endif
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

/**
 * Allocates one rx or tx descriptor ring. Does not allocate buffers. Does not initialize the
 * allocated descriptors.
 */
static bool
BCMATTACHFN_DMA_ATTACH(_dma_alloc)(dma_info_t *di, uint direction)
{
	return dma64_alloc(di, direction);
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
	if (di->txd64)
		DMA_FREE_CONSISTENT(di->osh, ((int8 *)(uintptr)di->txd64 - di->txdalign),
		                    di->txdalloc, (di->txdpaorig), di->tx_dmah);
	if (di->rxd64)
		DMA_FREE_CONSISTENT(di->osh, ((int8 *)(uintptr)di->rxd64 - di->rxdalign),
		                    di->rxdalloc, (di->rxdpaorig), di->rx_dmah);

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

	if ((di->ddoffsetlow == 0) || !(PHYSADDRLO(pa) & PCI32ADDR_HIGH)) {
		if (direction == DMA_TX) {
			W_REG(di->osh, &di->d64txregs->addrlow, (uint32)(PHYSADDRLO(pa) +
			                                         di->ddoffsetlow));
			W_REG(di->osh, &di->d64txregs->addrhigh, (PHYSADDRHI(pa) +
			                                          di->ddoffsethigh));
		} else {
			W_REG(di->osh, &di->d64rxregs->addrlow, (uint32)(PHYSADDRLO(pa) +
			                                         di->ddoffsetlow));
			W_REG(di->osh, &di->d64rxregs->addrhigh, (PHYSADDRHI(pa) +
			                                          di->ddoffsethigh));
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
			W_REG(di->osh, &di->d64txregs->addrhigh, di->ddoffsethigh);
			SET_REG(di->osh, &di->d64txregs->control, D64_XC_AE,
				(ae << D64_XC_AE_SHIFT));
		} else {
			W_REG(di->osh, &di->d64rxregs->addrlow, (uint32)(PHYSADDRLO(pa) +
			                                         di->ddoffsetlow));
			W_REG(di->osh, &di->d64rxregs->addrhigh, di->ddoffsethigh);
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
	OR_REG(di->osh, &di->d64txregs->control, D64_XC_LE);
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
#endif // endif

	/* For split fifo, for fifo-0 reclaim is handled by fifo-1 */
	ASSERT((di->rxin == di->rxout) || (di->split_fifo == SPLIT_FIFO_0));
	di->rxin = di->rxout = di->rs0cd = 0;
	di->rxavail = di->nrxd - 1;

	/* limit nrxpost buffer count to the max that will fit on the ring */
	if (di->sep_rxhdr) {
		/* 2 descriptors per buffer for 'sep_rxhdr' */
		di->nrxpost = MIN(di->nrxpost, di->rxavail/2);
	} else {
		/* 1 descriptor for each buffer in the normal case */
		di->nrxpost = MIN(di->nrxpost, di->rxavail);
	}

	/* clear rx descriptor ring */
	BZERO_SM((void *)(uintptr)di->rxd64, (di->nrxd * sizeof(dma64dd_t)));
#if defined(BCM47XX_CA9) || defined(BCA_HNDROUTER)
	DMA_MAP(di->osh, (void *)(uintptr)di->rxd64, (di->nrxd * sizeof(dma64dd_t)),
	        DMA_TX, NULL, NULL);
#endif // endif

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
#endif // endif
	control &= ~D64_RC_PC_MASK;
	control |= (di->rxprefetchctl << D64_RC_PC_SHIFT);

	control &= ~D64_RC_PT_MASK;
	control |= (di->rxprefetchthresh << D64_RC_PT_SHIFT);

	if (DMA_TRANSCOHERENT(di)) {
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

/** like getnexttxp but no reclaim */
void *
dma_peeknexttxp(hnddma_t *dmah)
{
	dma_info_t *di = DI_INFO(dmah);

	uint16 end;

	if (di->ntxd == 0)
		return (NULL);

	/* if this DMA channel is using indirect access, then configure the IndQSel register
	 * for this DMA channel
	 */
	if (DMA_INDIRECT(di)) {
		dma_set_indqsel(dmah, FALSE);
	}

	end = (uint16)B2I(((R_REG(di->osh, &di->d64txregs->status0) & D64_XS0_CD_MASK) -
	           di->xmtptrbase) & D64_XS0_CD_MASK, dma64dd_t);
	di->xs0cd = end;

#ifdef BULK_PKTLIST
	/* Routine does not appear to be used anywhere.
	 * XXX If is is used the caller has to reclaim this packet
	 * and terminate the PKTLINK before freeing the packet,
	 * the packet free code ASSERTS on a non NULL link.
	 * The packet cannot be terminated here as it had not been reclaimed
	 * and will break the pending DMA packet chain.
	 */
	if (DMA_BULK_PATH(di)) {
		return (di->dma_pkt_list.head_pkt);
	} else
#endif /* BULK_PKTLIST */
	{
		uint16 i;
		for (i = di->txin; i != end; i = NEXTTXD(i)) {
			if (di->txp[i]) {
				return (di->txp[i]);
			}
		}
	}

	return (NULL);
}

int
dma_peekntxp(hnddma_t *dmah, int *len, void *txps[], txd_range_t range)
{
	dma_info_t *di = DI_INFO(dmah);

	uint16 start, end, i;
	uint act;
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

		end = B2I(((R_REG(di->osh, &di->d64txregs->status0) & D64_XS0_CD_MASK) -
			di->xmtptrbase) & D64_XS0_CD_MASK, dma64dd_t);

		act = (uint)(R_REG(di->osh, &di->d64txregs->status1) & D64_XS1_AD_MASK);
		act = (act - di->xmtptrbase) & D64_XS0_CD_MASK;
		act = (uint)B2I(act, dma64dd_t);

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

	if ((di->nrxd == 0) || DMA_CTRL_IS_HWA_RX(di))
		return (NULL);

	end = (uint16)B2I(((R_REG(di->osh, &di->d64rxregs->status0) & D64_RS0_CD_MASK) -
		di->rcvptrbase) & D64_RS0_CD_MASK, dma64dd_t);
	di->rs0cd = end;

	for (i = di->rxin; i != end; i = NEXTRXD(i)) {
#ifdef BULKRX_PKTLIST
		if (DMA_BULK_PATH(di)) {
			if ((di->split_fifo) && (di->split_fifo == SPLIT_FIFO_0)) {
				return (di->linked_di->dma_rx_pkt_list.head_pkt);
			} else {
				return (di->dma_rx_pkt_list.head_pkt);
			}
		} else
#endif // endif
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
#endif // endif
	for (i = 0; i < di->ntxd; i++) {
		if (di->txp[i] != NULL) {
			count++;
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

	curr = B2I(((R_REG(di->osh, &di->d64txregs->status0) & D64_XS0_CD_MASK) -
	           di->xmtptrbase) & D64_XS0_CD_MASK, dma64dd_t);
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
	ptr = B2I(R_REG(di->osh, &di->d64txregs->ptr), dma64dd_t);

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
	curr = B2I(((R_REG(di->osh, &di->d64rxregs->status0) & D64_RS0_CD_MASK) -
		di->rcvptrbase) & D64_RS0_CD_MASK, dma64dd_t);
	ptr =  B2I(((R_REG(di->osh, &di->d64rxregs->ptr) & D64_RS0_CD_MASK) -
		di->rcvptrbase) & D64_RS0_CD_MASK, dma64dd_t);
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
	if (dmactrlflags & DMA_CTRL_PEN) {
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

static
uint8 BCMATTACHFN_DMA_ATTACH(dma_align_sizetobits)(uint size)
{
	uint8 bitpos = 0;
	ASSERT(size);
	ASSERT(!(size & (size-1)));
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
BCMATTACHFN_DMA_ATTACH(dma_ringalloc)(osl_t *osh, uint32 boundary, uint size, uint16 *alignbits,
	uint* alloced, dmaaddr_t *descpa, osldma_t **dmah)
{
	void * va;
	uint32 desc_strtaddr;
	uint32 alignbytes = 1 << *alignbits;

	if ((va = DMA_ALLOC_CONSISTENT(osh, size, *alignbits, alloced,
			descpa, (void **)dmah)) == NULL)
		return NULL;

	desc_strtaddr = (uint32)ROUNDUP((uint)PHYSADDRLO(*descpa), alignbytes);
	if (((desc_strtaddr + size - 1) & boundary) !=
	    (desc_strtaddr & boundary)) {
		*alignbits = dma_align_sizetobits(size);
		DMA_FREE_CONSISTENT(osh, va,
		                    size, *descpa, *dmah);
		va = DMA_ALLOC_CONSISTENT(osh, size, *alignbits, alloced, descpa, (void **)dmah);
	}
	return va;
}

#if defined(BCMDBG) || defined(BCMDBG_DUMP) || defined(BCMDBG_TXSTALL)
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
		/* in the format of high->low 16 bytes */
		if (b) {
			bcm_bprintf(b, "ring index %d: 0x%x %x %x %x\n",
			i, R_SM(&ring[i].addrhigh), R_SM(&ring[i].addrlow),
			R_SM(&ring[i].ctrl2), R_SM(&ring[i].ctrl1));
		} else {
			DMA_ERROR(("ring index %d: 0x%x %x %x %x\n",
			i, R_SM(&ring[i].addrhigh), R_SM(&ring[i].addrlow),
			R_SM(&ring[i].ctrl2), R_SM(&ring[i].ctrl1)));
		}
	}
}

void
dma_dumptx(hnddma_t *dmah, struct bcmstrbuf *b, bool dumpring)
{
	dma_info_t *di = DI_INFO(dmah);

	if (di->ntxd == 0)
		return;

	/* if using indirect DMA access, then configure IndQSel */
	if (DMA_INDIRECT(di)) {
		/* Set override to TRUE since HWA may update it */
		dma_set_indqsel(dmah, TRUE);
	}

	if (b) {
		uint32 xmptr, xmts0;

		xmptr = R_REG(di->osh, &di->d64txregs->ptr);
		xmts0 = R_REG(di->osh, &di->d64txregs->status0);

#ifdef BULK_PKTLIST
		bcm_bprintf(b, "DMA64: txp %p txd64 %p txdpa 0x%lx txdpahi 0x%lx "
			"head %p tail %p txin %d txout %d txavail %d txnodesc %d flags:0x%x\n",
			di->txp, di->txd64, PHYSADDRLO(di->txdpa),
			PHYSADDRHI(di->txdpaorig), di->dma_pkt_list.head_pkt,
			di->dma_pkt_list.tail_pkt, di->txin, di->txout,
			di->hnddma.txavail, di->hnddma.txnodesc, di->hnddma.dmactrlflags);
#else
		bcm_bprintf(b, "DMA64: txd64 %p txdpa 0x%lx txdpahi 0x%lx txp %p txin %d txout %d "
			"txavail %d txnodesc %d\n", di->txd64, PHYSADDRLO(di->txdpa),
			PHYSADDRHI(di->txdpaorig), di->txp, di->txin, di->txout, di->hnddma.txavail,
			di->hnddma.txnodesc);
#endif // endif
		bcm_bprintf(b, "xmtcontrol 0x%x xmtaddrlow 0x%x xmtaddrhigh 0x%x "
			    "xmtptr 0x%x xmtstatus0 0x%x xmtstatus1 0x%x\n",
			    R_REG(di->osh, &di->d64txregs->control),
			    R_REG(di->osh, &di->d64txregs->addrlow),
			    R_REG(di->osh, &di->d64txregs->addrhigh),
			    xmptr, xmts0, R_REG(di->osh, &di->d64txregs->status1));

#ifdef BCMHWA
		/* Only AQM registers value are reliable. */
		if (DMA_CTRL_IS_HWA_TX(di) &&
			di->hnddma.dmactrlflags & DMA_CTRL_DESC_ONLY_FLAG) {
			uint16 txin, txout;
			txout = (uint16)B2I(((xmptr & D64_XS0_CD_MASK) - di->xmtptrbase) &
				D64_XS0_CD_MASK, dma64dd_t);
			txin = (uint16)B2I(((xmts0 & D64_XS0_CD_MASK) - di->xmtptrbase) &
				D64_XS0_CD_MASK, dma64dd_t);
			bcm_bprintf(b, "hwatxpending %d hwatxavail %d\n",
				NTXDACTIVE(txin, txout), (di->ntxd - NTXDACTIVE(txin, txout) - 1));
		}
#endif // endif

		bcm_bprintf(b, "DMA64: DMA avoidance applied %d\n", di->dma_avoidance_cnt);
	} else {
		DMA_ERROR(("DMA64: txd64 %p txdpa 0x%lx txdpahi 0x%lx txp %p txin %d txout %d "
		       "txavail %d txnodesc %d\n", di->txd64, (unsigned long)PHYSADDRLO(di->txdpa),
		       (unsigned long)PHYSADDRHI(di->txdpaorig), di->txp, di->txin, di->txout,
		       di->hnddma.txavail, di->hnddma.txnodesc));

		DMA_ERROR(("xmtcontrol 0x%x xmtaddrlow 0x%x xmtaddrhigh 0x%x "
		       "xmtptr 0x%x xmtstatus0 0x%x xmtstatus1 0x%x\n",
		       R_REG(di->osh, &di->d64txregs->control),
		       R_REG(di->osh, &di->d64txregs->addrlow),
		       R_REG(di->osh, &di->d64txregs->addrhigh),
		       R_REG(di->osh, &di->d64txregs->ptr),
		       R_REG(di->osh, &di->d64txregs->status0),
		       R_REG(di->osh, &di->d64txregs->status1)));
	}

	if (dumpring && di->txd64) {
		dma64_dumpring(di, b, di->txd64, di->txin, di->txout, di->ntxd);
	}
}

void
dma_dumprx(hnddma_t *dmah, struct bcmstrbuf *b, bool dumpring)
{
	dma_info_t *di = DI_INFO(dmah);

	uint32 rcvptr, rcvs0, rcvs1;
	uint16 curr_descr, active_descr, last_descr;

	if (di->nrxd == 0)
		return;

#ifdef BULKRX_PKTLIST
	bcm_bprintf(b, "DMA64: rxd64 %p rxdpa 0x%lx rxdpahi 0x%lx head %p tail "
			"%p rxin %d rxout %d\n", OSL_OBFUSCATE_BUF(di->rxd64),
			PHYSADDRLO(di->rxdpa), PHYSADDRHI(di->rxdpaorig),
			di->dma_rx_pkt_list.head_pkt, di->dma_rx_pkt_list.tail_pkt,
			di->rxin, di->rxout);
#else
	bcm_bprintf(b, "DMA64: rxd64 %p rxdpa 0x%lx rxdpahi 0x%lx rxp %p rxin %d rxout %d\n",
			OSL_OBFUSCATE_BUF(di->rxd64), PHYSADDRLO(di->rxdpa),
			PHYSADDRHI(di->rxdpaorig), OSL_OBFUSCATE_BUF(di->rxp),
			di->rxin, di->rxout);
#endif // endif

	rcvptr = R_REG(di->osh, &di->d64rxregs->ptr);
	rcvs0 = R_REG(di->osh, &di->d64rxregs->status0);
	rcvs1 = R_REG(di->osh, &di->d64rxregs->status1);
	last_descr = (uint16)B2I(((rcvptr & D64_RS0_CD_MASK) - di->rcvptrbase) &
		D64_RS0_CD_MASK, dma64dd_t);
	curr_descr = (uint16)B2I(((rcvs0 & D64_RS0_CD_MASK) - di->rcvptrbase) &
		D64_RS0_CD_MASK, dma64dd_t);
	active_descr = (uint16)B2I(((rcvs1 & D64_RS0_CD_MASK) - di->rcvptrbase) &
		D64_RS0_CD_MASK, dma64dd_t);

	bcm_bprintf(b, "rcvcontrol 0x%x rcvaddrlow 0x%x rcvaddrhigh 0x%x rcvptr "
		       "0x%x rcvstatus0 0x%x rcvstatus1 0x%x rxfilled %d\n"
		       "currdescr %d activedescr %d lastdescr %d\n",
		       R_REG(di->osh, &di->d64rxregs->control),
		       R_REG(di->osh, &di->d64rxregs->addrlow),
		       R_REG(di->osh, &di->d64rxregs->addrhigh),
		       rcvptr, rcvs0, R_REG(di->osh, &di->d64rxregs->status1),
		       ((last_descr == curr_descr) ? 0 :
		       (di->nrxd - NRXDACTIVE(last_descr, curr_descr))),
		       curr_descr, active_descr, last_descr);

	if (di->rxd64 && dumpring) {
		if (DMA_CTRL_IS_HWA_RX(di))
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

#endif	/* BCMDBG || BCMDBG_DUMP || BCMDBG_TXSTALL */

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

	if (DMA_CTRL_IS_HWA_RX(di))
		return;

	/* if sep_rxhdr is enabled, for every pkt, two descriptors are programmed */
	/* NRXDACTIVE(rxin, rxout) would show 2 times no of actual full pkts */
	dpp = (di->sep_rxhdr) ? 2 : 1; /* dpp - descriptors per packet */

	/* read active descriptor */
	ad = (uint16)B2I(((R_REG(di->osh, &di->d64rxregs->status1) & D64_RS1_AD_MASK) -
		di->rcvptrbase) & D64_RS1_AD_MASK, dma64dd_t);
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
	W_REG(di->osh, &di->d64rxregs->ptr, (uint32)(di->rcvptrbase + I2B(di->rxout, dma64dd_t)));

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

	/* clear tx descriptor ring */
	BZERO_SM((void *)(uintptr)di->txd64, (di->ntxd * sizeof(dma64dd_t)));
#if defined(BCM47XX_CA9) || defined(BCA_HNDROUTER)
	DMA_MAP(di->osh, (void *)(uintptr)di->txd64, (di->ntxd * sizeof(dma64dd_t)),
	        DMA_TX, NULL, NULL);
#endif // endif

	/* These bits 20:18 (burstLen) of control register can be written but will take
	 * effect only if these bits are valid. So this will not affect previous versions
	 * of the DMA. They will continue to have those bits set to 0.
	 */
	control = R_REG(di->osh, &di->d64txregs->control);
	control = (control & ~D64_XC_BL_MASK) | (di->txburstlen << D64_XC_BL_SHIFT);
	control = (control & ~D64_XC_MR_MASK) | (di->txmultioutstdrd << D64_XC_MR_SHIFT);
	control = (control & ~D64_XC_PC_MASK) | (di->txprefetchctl << D64_XC_PC_SHIFT);
	control = (control & ~D64_XC_PT_MASK) | (di->txprefetchthresh << D64_XC_PT_SHIFT);
	control = (control & ~D64_XC_CS_MASK) | (di->txchanswitch << D64_XC_CS_SHIFT);
	if (DMA_TRANSCOHERENT(di))
		control = (control & ~D64_XC_CO_MASK) | (1 << D64_XC_CO_SHIFT);
	W_REG(di->osh, &di->d64txregs->control, control);

	control = D64_XC_XE;
	/* DMA engine with out alignment requirement requires table to be inited
	 * before enabling the engine
	 */
	if (!di->aligndesc_4k)
		_dma_ddtable_init(di, DMA_TX, di->txdpa);

#ifdef BCM_DMA_INDIRECT
	if (DMA_INDIRECT(di) &&
		(di->hnddma.dmactrlflags & DMA_CTRL_DESC_ONLY_FLAG) &&
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
	OR_REG(di->osh, &di->d64txregs->control, control);

	/* DMA engine with alignment requirement requires table to be inited
	 * before enabling the engine
	 */
	if (di->aligndesc_4k)
		_dma_ddtable_init(di, DMA_TX, di->txdpa);

#if defined(BULK_DESCR_FLUSH)
	/* init the bulk_descr_tx_start_txout ... no DMA USER BULK TX ACTIVE */
	DMA_BULK_DESCR_TX_SET_INVALID(di);
#endif // endif
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
	uint act;
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

	act = (uint)(R_REG(di->osh, &di->d64txregs->status1) & D64_XS1_AD_MASK);
	act = (act - di->xmtptrbase) & D64_XS0_CD_MASK;
	start = (uint16)B2I(act, dma64dd_t);

	end = di->txout;

	ring = di->txd64;
	for (i = start; i != end; i = NEXTTXD(i)) {
		/* find the first one having eof set */
		flags = R_SM(&ring[i].ctrl1);
		if (flags & CTRL_EOF) {
			/* rewind end to (i+1) */
			W_REG(di->osh,
			      &di->d64txregs->ptr,
			      (uint32)(di->xmtptrbase + I2B(NEXTTXD(i), dma64dd_t)));
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
BCMATTACHFN_DMA_ATTACH(dma64_alloc)(dma_info_t *di, uint direction)
{
	uint32 size;
	uint ddlen;
	void *va;
	uint alloced = 0;
	uint32 align;
	uint16 align_bits;

	ddlen = sizeof(dma64dd_t);

	size = (direction == DMA_TX) ? (di->ntxd * ddlen) : (di->nrxd * ddlen);
	align_bits = di->dmadesc_align;
	align = (1 << align_bits);

	if (direction == DMA_TX) {
		if ((va = dma_ringalloc(di->osh,
			(di->d64_xs0_cd_mask == 0x1fff) ? D64RINGBOUNDARY : D64RINGBOUNDARY_LARGE,
			size, &align_bits, &alloced,
			&di->txdpaorig, &di->tx_dmah)) == NULL) {
			DMA_ERROR(("%s: dma64_alloc: DMA_ALLOC_CONSISTENT(ntxd) failed\n",
			           di->name));
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

		/* adjust the va by the same offset */
		di->txd64 = (dma64dd_t *)((uintptr)va + di->txdalign);

		di->txdalloc = alloced;
		ASSERT(ISALIGNED(PHYSADDRLO(di->txdpa), align));
	} else {
		if ((va = dma_ringalloc(di->osh,
			(di->d64_rs0_cd_mask == 0x1fff) ? D64RINGBOUNDARY : D64RINGBOUNDARY_LARGE,
			size, &align_bits, &alloced,
			&di->rxdpaorig, &di->rx_dmah)) == NULL) {
			DMA_ERROR(("%s: dma64_alloc: DMA_ALLOC_CONSISTENT(nrxd) failed\n",
			           di->name));
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

		/* adjust the va by the same offset */
		di->rxd64 = (dma64dd_t *)((uintptr)va + di->rxdalign);

		di->rxdalloc = alloced;
		ASSERT(ISALIGNED(PHYSADDRLO(di->rxdpa), align));
	}

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

	if (!DMA_INDIRECT(di) || (di->hnddma.dmactrlflags & DMA_CTRL_DESC_ONLY_FLAG)) {
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
	}

#if defined(BULK_DESCR_FLUSH)
	/* reset/init the bulk_descr_tx_start_txout ... no DMA USER BULK TX ACTIVE */
	DMA_BULK_DESCR_TX_SET_INVALID(di);
#endif // endif
	return (status == D64_XS0_XS_DISABLED);
}

bool
dma_rxidlestatus(hnddma_t *dmah)
{
	dma_info_t *di = DI_INFO(dmah);

	if ((di->nrxd == 0) || DMA_CTRL_IS_HWA_RX(di)) {
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

		cur_idx = B2I(((R_REG(di->osh, &di->d64txregs->status0) & D64_XS0_CD_MASK) -
		               di->xmtptrbase) & D64_XS0_CD_MASK, dma64dd_t);
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
#endif // endif
		va = di->txp[cur_idx];
	} else {
		cur_idx = B2I(((R_REG(di->osh, &di->d64rxregs->status0) & D64_RS0_CD_MASK) -
		               di->rcvptrbase) & D64_RS0_CD_MASK, dma64dd_t);
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
#ifdef BCM_SECURE_DMA
#ifdef BCMDMA64OSL
	dma_addr_t paddr;
#endif /* BCMDMA64OSL */
#endif /* BCM_SECURE_DMA */
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
#ifdef BCMDMA64OSL
	paddr = SECURE_DMA_MAP(di->osh, buf, len, DMA_TX, NULL, NULL,
		&di->sec_cma_info_tx, 0, SECDMA_TXBUF_POST);
	ULONGTOPHYSADDR(paddr, pa);
#else
	pa = SECURE_DMA_MAP(di->osh, buf, len, DMA_TX, NULL, NULL,
		&di->sec_cma_info_tx, 0, SECDMA_TXBUF_POST);
#endif /* BCMDMA64OSL */
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
#endif // endif

#if defined(BULK_DESCR_FLUSH)
	if (!DMA_BULK_DESCR_TX_IS_VALID(di)) {
		DMA_MAP(di->osh,  dma64_txd64(di, di->txout), DMA64_FLUSH_LEN(1),
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
#endif // endif

	txout = NEXTTXD(txout);
	/* bump the tx descriptor index */
	di->txout = txout;

	/* kick the chip */
	if (commit) {
		dma64_txcommit_local(di);
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
	DMA_MAP(di->osh, dma64_rxd64(di, di->rxout), DMA64_FLUSH_LEN(1),
		DMA_TX, NULL, NULL);
#endif /* BULK_DESCR_FLUSH */

	/* save the buffer pointer - used by dma_getpos */
	di->rxp[rxout] = buf;

	rxout = NEXTRXD(rxout);

	/* kick the chip */
	if (commit) {
		W_REG(di->osh, &di->d64rxregs->ptr,
		    (uint32)(di->rcvptrbase + I2B(rxout, dma64dd_t)));
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
		curr = (uint16)B2I(((R_REG(di->osh, &di->d64rxregs->status0) & D64_RS0_CD_MASK) -
		di->rcvptrbase) & D64_RS0_CD_MASK, dma64dd_t);
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
#endif // endif

	ASSERT(dma_txsuspendedidle(dmah));

	/* if using indirect DMA access, then configure IndQSel */
	if (DMA_INDIRECT(di)) {
		dma_set_indqsel(dmah, FALSE);
	}

	nactive = dma_txactive(dmah);
	ad = B2I((((R_REG(di->osh, &di->d64txregs->status1) & D64_XS1_AD_MASK)
		- di->xmtptrbase) & D64_XS1_AD_MASK), dma64dd_t);
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
#endif // endif
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
#endif // endif
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
#endif // endif

	/* update txin and txout */
	di->txin = ad;
	di->txout = TXD(di->txout + rot);
	di->hnddma.txavail = di->ntxd - NTXDACTIVE(di->txin, di->txout) - 1;

#if (defined(__ARM_ARCH_7A__) && defined(CA7)) || defined(STB)
	/* memory barrier before posting the descriptor */
	DMB();
#endif // endif
	/* kick the chip */
	dma64_txcommit_local(di);
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
#endif // endif

pktpool_t*
dma_pktpool_get(hnddma_t *dmah)
{
	dma_info_t * di = DI_INFO(dmah);

	ASSERT(di);
	ASSERT(di->pktpool != NULL);
	return (di->pktpool);
}

#ifdef STS_FIFO_RXEN
void
dma_sts_mp_set(hnddma_t *dmah, void *sts_mempool)
{
	dma_info_t * di = DI_INFO(dmah);
	ASSERT(di);
	ASSERT(di->sts_mempool == NULL);
	di->sts_mempool = sts_mempool;
}
#endif // endif

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
			curr = (uint16)(B2I(((R_REG(di->osh, &di->d64txregs->status0) &
				D64_XS0_CD_MASK) - di->xmtptrbase) &
				D64_XS0_CD_MASK, dma64dd_t));

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

void
dma_burstlen_set(hnddma_t *dmah, uint8 rxburstlen, uint8 txburstlen)
{
	dma_info_t *di = DI_INFO(dmah);

	di->rxburstlen = rxburstlen;
	di->txburstlen = txburstlen;
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
#endif // endif

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

int BCMFASTPATH
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
	W_REG(di->osh, &di->d64rxregs->ptr, (uint32)(di->rcvptrbase + I2B(rxout, dma64dd_t)));
	return 0;
outofrxd:
	di->rxavail = 0;
	DMA_ERROR(("%s: dma_rxfast: out of rxds\n", di->name));
	return -1;
}

int BCMFASTPATH
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
		dma64_txcommit_local(di);
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

int BCMFASTPATH
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
		dma64_txcommit_local(di);
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

#ifdef STS_FIFO_RXEN
void
dma_link_sts_handle(hnddma_t *dmah1, hnddma_t *dmah2)
{
	dma_info_t *di1 = DI_INFO(dmah1);
	dma_info_t *di2 = DI_INFO(dmah2);
	di2->sts_di = di1;
}
#endif // endif

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
	dma64regs_t *dregs = di->d64txregs;
	uint16 start, end;

	if (di->ntxd == 0)
		return;

	start = di->txin;

	if (di->txin == di->xs0cd) {
		end = (uint16)(B2I(((R_REG(di->osh, &dregs->status0) & D64_XS0_CD_MASK) -
		      di->xmtptrbase) & D64_XS0_CD_MASK, dma64dd_t));
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
#endif // endif

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
	W_REG(di->osh, &di->d64rxregs->ptr, (uint32)(di->rcvptrbase + I2B(rxout, dma64dd_t)));

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
		dma64_txcommit_local(di);
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

	W_REG(di->osh, &di->d64rxregs->ptr, (uint32)(di->rcvptrbase + I2B(di->rxout, dma64dd_t)));
	dma64_txcommit_local(di);
}

/* Get physical address of descriptor ring */
int
dma_get_fifo_info(hnddma_t *dmah, dmaaddr_t *pa, uint *depth, bool tx)
{
	dma_info_t * di = DI_INFO(dmah);

	ASSERT(pa);
	ASSERT(depth);

	if (pa == NULL || depth == NULL)
		return BCME_BADARG;

	if (tx) {
		*pa = di->txdpa;
		*depth = di->ntxd;
	}
	else {
		*pa = di->rxdpa;
		*depth = di->nrxd;
	}

	return BCME_OK;
}

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
#endif // endif

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
#endif // endif
