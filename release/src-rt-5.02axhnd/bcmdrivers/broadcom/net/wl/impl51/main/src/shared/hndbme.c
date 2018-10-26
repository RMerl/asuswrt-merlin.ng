/*
 * Generic Broadcom Home Networking Division (HND) BME module.
 * This supports chips with revs >= 128.
 *
 * Copyright (C) 2018, Broadcom. All Rights Reserved.
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
 * $Id: hndbme.c 999999 2017-01-04 16:02:31Z $
 */

/**
 * @file
 * @brief
 * Source file for HNDBME module. This file contains the functionality to initialize and run the
 * BME engine. BME stands for Byte Move Engine. The BME is a non-descriptor based DMA engine
 * capable of handling a single asynchronous transfer at a time, which gets programmed solely
 * through registers.
 */

#include <osl.h>
#include <siutils.h>
#include <hndbme.h>
#include <sbhnddma.h>
#include <bcmutils.h>
#include <hndsoc.h>
#include <m2mdma_core.h> 	/* for m2md_regs_t */
#include <bcmdevs.h> 		/* for e.g. EMBEDDED_2x2AX_CORE */
#define SIZE_64BIT_PTRS				8

#define	PCI64ADDR_HIGH				0x80000000	/* address[63] */

#define DMA_OFFSET_BME_BASE_CHANNEL2_XMT	0x280
#define DMA_OFFSET_BME_BASE_CHANNEL2_RCV	0x2A0
#define DMA_OFFSET_BME_BASE_CHANNEL3_XMT	0x2C0
#define DMA_OFFSET_BME_BASE_CHANNEL3_RCV	0x2E0

/** 63178/47622 introduced an enhancement: acceleration for phyrx/tx status processing */
#define M2M_OFFSET_BME_TXSTS_REGS		0x800
#define M2M_OFFSET_BME_PHYRXSTS_REGS		0x900

#define BME_CONFIG_FLAG_SRC_PCIE_ADDREXT	(1 << 31)
#define BME_CONFIG_FLAG_DEST_PCIE_ADDREXT	(1 << 30)

/**
 * The BME has the same registers as a DMA engine (using the same address space). So the
 * struct dma64regs_t from sbhnddma.h is reused. The bits of some of the fields are
 * defined here.
 */
#define BMEREG_CONTROL_EN			(1 << 0)
#define BMEREG_CONTROL_BURSTLEN_SHIFT		18
#define BMEREG_CONTROL_BURSTLEN_MASK		0x7

#define BMEREG_XMTPTR_BUFFCOUNT_MASK		0x1FFF
#define BMEREG_XMTPTR_ADDREXT			(1 << 13)
#define BMEREG_XMTPTR_COHERENT			(1 << 14)
#define BMEREG_XMTPTR_START_BUSY		(1 << 31)

#define BMEREG_RCVPTR_ADDREXT			(1 << 13)
#define BMEREG_RCVPTR_COHERENT			(1 << 14)

#define M2M_INTCONTROL_TXS_WRIND_UPD_MASK	(1<<30)
#define M2M_INTCONTROL_PHYRXS_WRIND_UPD_MASK	(1<<31)

#define M2M_INTCONTROL_MASK \
	(M2M_INTCONTROL_TXS_WRIND_UPD_MASK)

#define M2M_INTCTL_TXS (1<<30)

/** txstatus memory inside the d11 core. Physical address from the BME's perspective. */
#define D11_TXSTS_MEM_BASE 0x848d8000
/** phyrxtatus memory inside the d11 core. Physical address from the BME's perspective. */
#define D11_PHYRXSTS_MEM_BASE 0x848d9000

#if defined(__ARM_ARCH_7A__)
#define datasynchronizationbarrier() __asm__ __volatile__ ("dsb")
#endif /* __ARM_ARCH_7A__ */

#define BME_SWITCHCORE(_sih_, _origidx_, _intr_val_)				\
({										\
	*_origidx_ = 0;								\
	*_intr_val_ = 0;							\
	if (BUSTYPE(_sih_->bustype) == PCI_BUS) {				\
		si_switch_core(_sih_, M2MDMA_CORE_ID, _origidx_, _intr_val_);	\
	}									\
})

#define BME_RESTORECORE(_sih_, _coreid_, _intr_val_)	\
if (BUSTYPE(_sih_->bustype) == PCI_BUS)			\
	si_restore_core(_sih_, _coreid_, _intr_val_);

/** 63178 / 47622 support tx/phyrx status offloading in BME channel #1 */
struct bme_circ_buf_s {
	uint8	*va;        /**< virtual start address of ring buffer */
	volatile m2md_bme_status_regs_t *regs; /**< virtual address of txs or phyrxsts registers */
	dmaaddr_t pa;       /**< physical start address of ring buffer */
	uint	element_sz; /**< size of one circular buffer element in [bytes] */
	uint	n_elements;
	uint	buf_sz;     /**< size of the circular buffer in [bytes] */
	uint    rd_idx;     /**< software updates read index */
};

struct bme_info_s {
	si_t			*sih;
	void			*osh;		    /**< os handle */
	volatile dma64regs_t	*xmt;
	volatile dma64regs_t	*rcv;
	volatile m2md_regs_t	*m2m_regs;          /**< virtual address of m2m core registers */
	uint32			hi_src;
	uint32			hi_dest;
	uint32			flags;
	uint32		        m2m_intmask;        /**< cause reducing reg rds is more efficient */
	bme_channel_t		channel;
	bool			config_written;
	struct bme_circ_buf_s   txsts;              /**< txstatus offload support */
	struct bme_circ_buf_s   phyrxsts;           /**< phy rx status offload support */
};

#if defined(BME_OFFLOADS_PHYRXSTS) || defined(BME_OFFLOADS_TXSTS)

/** 63178 / 47622 support tx/phyrx status offloading in BME channel #1 */
/*
 * Initializes one tx or one phyrx status offload channel.
 *
 * @param[in]      sih            Silicon backplane handle
 * @param[in]      regs           Volatile pointer to circular buffer registers
 * @param[inout]   p              Variables associated to one circular BME buffer
 * @param[in]      n_buf_els      Number of elements in one circular buffer
 * @param[in]      element_sz     Size in [bytes] of one circular buffer element
 *
 * @return         TRUE if call succeeded
 */
static bool
bme_attach_status_channel(si_t *sih, volatile char *regs, struct bme_circ_buf_s *p,
                          int n_buf_els, int element_sz)
{
	p->regs = (volatile m2md_bme_status_regs_t *)(regs);
	p->element_sz = element_sz;
	p->n_elements = n_buf_els;
	p->buf_sz = element_sz * n_buf_els;
	p->va = MALLOCZ(si_osh(sih), p->buf_sz);
	if (p->va == NULL) {
		return FALSE;
	}
	p->pa = DMA_MAP(si_osh(sih), p->va, p->buf_sz, DMA_RX, NULL, NULL);

	return TRUE;
}

/*
 * @param[inout]   p              Variables associated to one circular BME buffer
 */
static void
bme_detach_status_channel(osl_t *osh, struct bme_circ_buf_s *p)
{
	if (p->va != NULL) {
		DMA_UNMAP(osh, p->pa, p->buf_sz, DMA_RX, NULL, NULL);
		MFREE(osh, p->va, p->buf_sz);
	}
}

#endif /* BME_OFFLOADS_PHYRXSTS || BME_OFFLOADS_TXSTS */

bme_info_t *
BCMATTACHFN(bme_attach)(si_t *sih, bme_channel_t channel,
                        int n_buf_txs, int sizeof_txs, int n_buf_phyrxsts, int sizeof_phyrxsts)
{
	bme_info_t *bme_info;
	uint saved_core_idx;
	uint intr_val;
	volatile char *base;

	/* Allocate private info structure */
	if ((bme_info = MALLOCZ(si_osh(sih), sizeof(*bme_info))) == NULL) {
		printf("%s: out of memory, malloced %d bytes\n",
			__FUNCTION__, MALLOCED(si_osh(sih)));
		return (NULL);
	}
	bme_info->sih = sih;
	bme_info->osh = si_osh(sih);

	base = (volatile char *)si_switch_core(sih, M2MDMA_CORE_ID, &saved_core_idx, &intr_val);
	ASSERT(base);
	bme_info->m2m_regs = (volatile m2md_regs_t*)base;

	/* Take M2M core out of reset if it's not */
	if (!si_iscoreup(sih))
		si_core_reset(sih, 0, 0);

	bme_info->channel = channel;
	if (channel == BME_CHANNEL_0) {
		bme_info->xmt = (volatile dma64regs_t *)(base + DMA_OFFSET_BME_BASE_CHANNEL2_XMT);
		bme_info->rcv = (volatile dma64regs_t *)(base + DMA_OFFSET_BME_BASE_CHANNEL2_RCV);
	} else {
		bme_info->xmt = (volatile dma64regs_t *)(base + DMA_OFFSET_BME_BASE_CHANNEL3_XMT);
		bme_info->rcv = (volatile dma64regs_t *)(base + DMA_OFFSET_BME_BASE_CHANNEL3_RCV);
	}

	OR_REG(bme_info->osh, &bme_info->xmt->control, BMEREG_CONTROL_EN);
	OR_REG(bme_info->osh, &bme_info->rcv->control, BMEREG_CONTROL_EN);

	si_restore_core(sih, saved_core_idx, intr_val);

	if (EMBEDDED_2x2AX_CORE(sih->chip) && channel == BME_CHANNEL_1) {
#ifdef BME_OFFLOADS_TXSTS
		if (sizeof_txs != 0) {
			if (bme_attach_status_channel(sih, base + M2M_OFFSET_BME_TXSTS_REGS,
			                              &bme_info->txsts,
			                              n_buf_txs, sizeof_txs) == FALSE) {
				goto fail;
			}
		}
#endif /* BME_OFFLOADS_TXSTS */
#ifdef BME_OFFLOADS_PHYRXSTS
		if (sizeof_phyrxsts != 0) {
			if (bme_attach_status_channel(sih, base + M2M_OFFSET_BME_PHYRXSTS_REGS,
			                              &bme_info->phyrxsts,
			                              n_buf_phyrxsts, sizeof_phyrxsts) == FALSE) {
				goto fail;
			}
		}
#endif /* BME_OFFLOADS_PHYRXSTS */
	}

	return bme_info;

#if defined(BME_OFFLOADS_PHYRXSTS) || defined(BME_OFFLOADS_TXSTS)
fail:
	bme_detach(bme_info);

	return NULL;
#endif /* BME_OFFLOADS_TXSTS || BME_OFFLOADS_TXSTS */
} /* bme_attach */

void
BCMATTACHFN(bme_detach)(bme_info_t *bme_info)
{
	uint saved_core_idx;
	uint intr_val;

	BME_SWITCHCORE(bme_info->sih, &saved_core_idx, &intr_val);

	AND_REG(bme_info->osh, &bme_info->xmt->control, ~BMEREG_CONTROL_EN);
	AND_REG(bme_info->osh, &bme_info->rcv->control, ~BMEREG_CONTROL_EN);

	BME_RESTORECORE(bme_info->sih, saved_core_idx, intr_val);

#if defined(BME_OFFLOADS_PHYRXSTS) || defined(BME_OFFLOADS_TXSTS)
	bme_detach_status_channel(bme_info->osh, &bme_info->txsts);
	bme_detach_status_channel(bme_info->osh, &bme_info->phyrxsts);
#endif /* BME_OFFLOADS_PHYRXSTS || BME_OFFLOADS_TXSTS */

	MFREE(bme_info->osh, (void *)bme_info, sizeof(*bme_info));
}

#if defined(BME_OFFLOADS_PHYRXSTS) || defined(BME_OFFLOADS_TXSTS)
/**
 * (Re)initializes a specific BME tx/phyrx status offload channel. Usually called on a 'wl up'.
 * @param[in] d11_src_addr   Backplane address pointing to a d11 core internal memory
 */
static void
bme_init_offload_ch(osl_t *osh, struct bme_circ_buf_s *p, uint32 d11_src_addr)
{
	W_REG(osh, &p->regs->sa_base_l, d11_src_addr);      // source address
	W_REG(osh, &p->regs->sa_base_h, 0);
	W_REG(osh, &p->regs->da_base_l, PHYSADDRLO(p->pa)); // destination address
	W_REG(osh, &p->regs->da_base_h, PHYSADDRHI(p->ba));
	W_REG(osh, &p->regs->size, p->n_elements);
	p->rd_idx = 0;
	W_REG(osh, &p->regs->rd_idx, p->rd_idx);
	/* source is is AXI backplane (so 'not PCIe'), destination is coherent 63178 ARM mem */
	W_REG(osh, &p->regs->dma_template,
	      BME_DMATMPL_COHERENT_BITMASK | BME_DMATMPL_NOTPCIESP_BITMASK);
	OR_REG(osh, &p->regs->cfg, BME_STS_CFG_MOD_ENBL_MASK);
}
#endif /* BME_OFFLOADS_PHYRXSTS | BME_OFFLOADS_TXSTS */

/**
 * (Re)initializes the BME hardware and software. Enables interrupts. Usually called on a 'wl up'.
 *
 * Prerequisite: firmware has progressed passed the BCMATTACH phase
 *
 * @param bme_info     Handle related to one of the BME dma channels in the M2M core.
 */
void
bme_init(bme_info_t *bme_info)
{
	uint saved_core_idx;
	uint intr_val;

	BME_SWITCHCORE(bme_info->sih, &saved_core_idx, &intr_val);
#ifdef BME_OFFLOADS_TXSTS
	if (bme_info->txsts.regs != NULL) { /* channel supports phyrx/tx status offloading */
		bme_init_offload_ch(bme_info->osh, &bme_info->txsts, D11_TXSTS_MEM_BASE);
		OR_REG(bme_info->osh, &bme_info->m2m_regs->intcontrol, M2M_INTCONTROL_MASK);
	}
#endif /* BME_OFFLOADS_TXSTS */
#ifdef BME_OFFLOADS_PHYRXSTS
	if (bme_info->phyrxsts.regs != NULL) {
		bme_init_offload_ch(bme_info->osh, &bme_info->phyrxsts,
		                    D11_PHYRXSTS_MEM_BASE);
	}
#endif /* BME_OFFLOADS_PHYRXSTS */
	BME_RESTORECORE(bme_info->sih, saved_core_idx, intr_val);
}

#if defined(BME_OFFLOADS_PHYRXSTS) || defined(BME_OFFLOADS_TXSTS)
/**
 * Disables a specific BME channel. Usually called on a 'wl down'.
 */
static void
bme_deinit_offload_ch(osl_t *osh, struct bme_circ_buf_s *p)
{
	AND_REG(osh, &p->regs->cfg, ~BME_STS_CFG_MOD_ENBL_MASK);
}
#endif /* BME_OFFLOADS_PHYRXSTS || BME_OFFLOADS_TXSTS */

/**
 * Disables interrupts. Usually called on a 'wl down'.
 *
 * Prerequisite: firmware has progressed passed the BCMATTACH phase
 */
void
bme_deinit(bme_info_t *bme_info)
{

	uint saved_core_idx;
	uint intr_val;

	BME_SWITCHCORE(bme_info->sih, &saved_core_idx, &intr_val);
#ifdef BME_OFFLOADS_TXSTS
	if (bme_info->txsts.regs != NULL) { /* channel supports phyrx/tx status offloading */
		AND_REG(bme_info->osh, &bme_info->m2m_regs->intcontrol, ~M2M_INTCONTROL_MASK);
		bme_deinit_offload_ch(bme_info->osh, &bme_info->txsts);
	}
#endif /* BME_OFFLOADS_TXSTS */
#ifdef BME_OFFLOADS_PHYRXSTS
	if (bme_info->phyrxsts.regs != NULL) {
		bme_deinit_offload_ch(bme_info->osh, &bme_info->phyrxsts);
	}
#endif /* BME_OFFLOADS_PHYRXSTS */
	BME_RESTORECORE(bme_info->sih, saved_core_idx, intr_val);
}

void
bme_config(bme_info_t *bme_info, uint32 hi_src, uint32 hi_dest, uint32 flags, uint8 burst_len)
{
	uint32 control;
	uint saved_core_idx;
	uint intr_val;

	bme_info->flags = flags;

	bme_info->hi_src = hi_src;
	bme_info->hi_dest = hi_dest;

	bme_info->config_written = FALSE;

	ASSERT(burst_len < 3);
	if (burst_len > 2) {
		burst_len = 2;
	}

	BME_SWITCHCORE(bme_info->sih, &saved_core_idx, &intr_val);
	/* Configure xmit burstlen */
	control = R_REG(bme_info->osh, &bme_info->xmt->control);
	if (((control >> BMEREG_CONTROL_BURSTLEN_SHIFT) & BMEREG_CONTROL_BURSTLEN_MASK) !=
		burst_len) {
		control &= ~(BMEREG_CONTROL_BURSTLEN_MASK << BMEREG_CONTROL_BURSTLEN_SHIFT);
		control |= (burst_len << BMEREG_CONTROL_BURSTLEN_SHIFT);
		bme_sync(bme_info);
		W_REG(bme_info->osh, &bme_info->xmt->control, control);
		/* Configure receive burstlen */
		control = R_REG(bme_info->osh, &bme_info->rcv->control);
		control &= ~(BMEREG_CONTROL_BURSTLEN_MASK << BMEREG_CONTROL_BURSTLEN_SHIFT);
		control |= (burst_len << BMEREG_CONTROL_BURSTLEN_SHIFT);
		W_REG(bme_info->osh, &bme_info->rcv->control, control);
	}
	BME_RESTORECORE(bme_info->sih, saved_core_idx, intr_val);
}

/**
 * @param[in] src   Physical source address
 * @param[in] dest  Physical destination address
 */
int
bme_cpy(bme_info_t *bme_info, const uint8 *src, uint8 *dest, uint16 len)
{
	uint32 ptr;
	bool ptr_64bit;
	uint saved_core_idx;
	uint intr_val;

	BME_SWITCHCORE(bme_info->sih, &saved_core_idx, &intr_val);

	/* poll XmtPtr.StartBusy */
	while (R_REG(bme_info->osh, &bme_info->xmt->ptr) & BMEREG_XMTPTR_START_BUSY) {
		;
	}
	ptr_64bit = (sizeof(src) == SIZE_64BIT_PTRS);

	if (ptr_64bit) {
		/* Two shifts are needed to avoid the warning: */
		/*     "right shift count >= width of type" (on 32bit systems) */
		ptr = (((uintptr)src >> 16) >> 16);
		if (ptr & PCI64ADDR_HIGH) {
			bme_info->flags |= BME_CONFIG_FLAG_SRC_PCIE_ADDREXT;
			ptr &= ~PCI64ADDR_HIGH;
		} else {
			bme_info->flags &= ~BME_CONFIG_FLAG_SRC_PCIE_ADDREXT;
		}

		if (bme_info->flags & BME_CONFIG_FLAG_SRC_HOSTMEM)
			ptr |= PCI64ADDR_HIGH;
		W_REG(bme_info->osh, &bme_info->xmt->addrhigh, ptr);

		ptr = (((uintptr)dest >> 16) >> 16);
		if (ptr & PCI64ADDR_HIGH) {
			ptr &= ~PCI64ADDR_HIGH;
			if ((bme_info->flags & BME_CONFIG_FLAG_DEST_PCIE_ADDREXT) == 0) {
				bme_info->flags |= BME_CONFIG_FLAG_DEST_PCIE_ADDREXT;
				bme_info->config_written = FALSE;
			}
		} else {
			if (bme_info->flags & BME_CONFIG_FLAG_DEST_PCIE_ADDREXT) {
				bme_info->flags &= ~BME_CONFIG_FLAG_DEST_PCIE_ADDREXT;
				bme_info->config_written = FALSE;
			}
		}

		if (bme_info->flags & BME_CONFIG_FLAG_DEST_HOSTMEM)
			ptr |= PCI64ADDR_HIGH;
		W_REG(bme_info->osh, &bme_info->rcv->addrhigh, ptr);
	}

	if (!bme_info->config_written) {
		if (!ptr_64bit) {
			if (bme_info->hi_src & PCI64ADDR_HIGH) {
				bme_info->flags |= BME_CONFIG_FLAG_SRC_PCIE_ADDREXT;
			}
			if (bme_info->flags & BME_CONFIG_FLAG_SRC_HOSTMEM) {
				W_REG(bme_info->osh, &bme_info->xmt->addrhigh,
				      bme_info->hi_src | PCI64ADDR_HIGH);
			} else {
				W_REG(bme_info->osh, &bme_info->xmt->addrhigh,
				      bme_info->hi_src & ~PCI64ADDR_HIGH);
			}
			if (bme_info->hi_dest & PCI64ADDR_HIGH) {
				bme_info->flags |= BME_CONFIG_FLAG_DEST_PCIE_ADDREXT;
			}
			if (bme_info->flags & BME_CONFIG_FLAG_DEST_HOSTMEM) {
				W_REG(bme_info->osh, &bme_info->rcv->addrhigh,
				      bme_info->hi_dest | PCI64ADDR_HIGH);
			} else {
				W_REG(bme_info->osh, &bme_info->rcv->addrhigh,
				      bme_info->hi_dest & ~PCI64ADDR_HIGH);
			}
		}
		/* Set RcvPtr.Coherent as desired */
		ptr = 0;
		if (bme_info->flags & BME_CONFIG_FLAG_DEST_COHERENT) {
			ptr |= BMEREG_RCVPTR_COHERENT;
		}
		if (bme_info->flags & BME_CONFIG_FLAG_DEST_PCIE_ADDREXT) {
			ptr |= BMEREG_RCVPTR_ADDREXT;
		}
		W_REG(bme_info->osh, &bme_info->rcv->ptr, ptr);
		bme_info->config_written = TRUE;
	}

	/* write the destination address to RcvAddrHigh/Low */
	W_REG(bme_info->osh, &bme_info->rcv->addrlow, (uint32)(uintptr)dest);

	/* write the source address to XmtAddrHigh/Low */
	W_REG(bme_info->osh, &bme_info->xmt->addrlow, (uint32)(uintptr)src);

#if defined(__ARM_ARCH_7A__)
	/* Be sure to have the data which is to be copied has actually reached the Cache/mem */
	datasynchronizationbarrier();
#endif /* __ARM_ARCH_7A__ */

	/* write the len, flags and start the trasfer */
	ASSERT(len <= BMEREG_XMTPTR_BUFFCOUNT_MASK);
	ptr = ((len & BMEREG_XMTPTR_BUFFCOUNT_MASK) | BMEREG_XMTPTR_START_BUSY);
	if (bme_info->flags & BME_CONFIG_FLAG_SRC_COHERENT) {
		ptr |= BMEREG_XMTPTR_COHERENT;
	}
	if (bme_info->flags & BME_CONFIG_FLAG_SRC_PCIE_ADDREXT) {
		ptr |= BMEREG_XMTPTR_ADDREXT;
	}
	W_REG(bme_info->osh, &bme_info->xmt->ptr, ptr);

	BME_RESTORECORE(bme_info->sih, saved_core_idx, intr_val);

	return BCME_OK;
} /* bme_cpy */

int
bme_cpy64(bme_info_t *bme_info, uint64 src, uint64 dest, uint16 len)
{
	int ret_value;
	uint32 hi_src;
	uint32 hi_dest;

	if (sizeof(uint8 *) == SIZE_64BIT_PTRS) {
		return bme_cpy(bme_info, (const uint8 *)(uintptr)src, (uint8 *)(uintptr)dest, len);
	}

	hi_src = bme_info->hi_src;
	hi_dest = bme_info->hi_dest;

	bme_info->hi_src = (uint32)(src >> 32);
	bme_info->hi_dest = (uint32)(dest >> 32);
	bme_info->config_written = FALSE;

	ret_value = bme_cpy(bme_info, (const uint8 *)(uintptr)src, (uint8 *)(uintptr)dest, len);

	bme_info->hi_src = hi_src;
	bme_info->hi_dest = hi_dest;
	bme_info->config_written = FALSE;

	return ret_value;
}

bool
bme_completed(bme_info_t *bme_info)
{
	uint saved_core_idx;
	uint intr_val;

	BME_SWITCHCORE(bme_info->sih, &saved_core_idx, &intr_val);

	if (R_REG(bme_info->osh, &bme_info->xmt->ptr) & BMEREG_XMTPTR_START_BUSY) {
		BME_RESTORECORE(bme_info->sih, saved_core_idx, intr_val);
		return FALSE;
	}

	BME_RESTORECORE(bme_info->sih, saved_core_idx, intr_val);
	return TRUE;
}

void
bme_sync(bme_info_t *bme_info)
{
	uint saved_core_idx;
	uint intr_val;

	BME_SWITCHCORE(bme_info->sih, &saved_core_idx, &intr_val);

	while (R_REG(bme_info->osh, &bme_info->xmt->ptr) & BMEREG_XMTPTR_START_BUSY) {
		;
	}

	BME_RESTORECORE(bme_info->sih, saved_core_idx, intr_val);
}

#ifdef BME_OFFLOADS_TXSTS
/**
 * Returns a pointer to one txstatus in the circular buffer that was written by the d11 core but
 * not yet consumed by software.
 *
 * @param[in] caller_buf   Caller allocated buffer, large enough to contain phyrx or tx status
 * @return                 Pointer to caller_buf if succeeded
 */
void * BCMFASTPATH
bme_get_txstatus(bme_info_t *bme_info, void *caller_buf)
{
	uint saved_core_idx;
	uint intr_val;
	uint wr_idx;            /**< hardware updates write index */
	void *ret = NULL;       /**< return value */
	struct bme_circ_buf_s *pc = &bme_info->txsts; /**< tx status circular buffer info */

	ASSERT(pc->regs != NULL); /* BME channel does not support txs/phyrx status offloading */
	BME_SWITCHCORE(bme_info->sih, &saved_core_idx, &intr_val);

	wr_idx = R_REG(bme_info->osh, &pc->regs->wr_idx); /* register is updated by hardware */
	if (wr_idx != pc->rd_idx) {
		memcpy(caller_buf, pc->va + pc->rd_idx * pc->element_sz, pc->element_sz);
		ret = caller_buf;
		if (++pc->rd_idx == pc->n_elements) {
			pc->rd_idx = 0;
		}
		/* inform hw that it may advance */
		W_REG(bme_info->osh, &pc->regs->rd_idx, pc->rd_idx);
	}

	BME_RESTORECORE(bme_info->sih, saved_core_idx, intr_val);

	return ret;
} /* bme_get_txstatus */

/**
 * Read and optionally clear m2m intstatus register.
 * This routine must not be preempted by other WLAN code.
 *
 * @param[in] in_isr
 * @Return:  0xFFFFFFFF if DEVICEREMOVED, 0 if the interrupt is not for us, or we are in some
 *           special case, device interrupt status bits otherwise.
 */
uint32 BCMFASTPATH
bme_m2m_intstatus(bme_info_t *bme_info, bool in_isr)
{
	volatile m2md_regs_t *regs = bme_info->m2m_regs;
	uint32 active_irqs;

#ifdef LINUX_VERSION_CODE
	ASSERT((in_irq() && irqs_disabled()) || in_softirq());
#endif /* LINUX_VERSION_CODE */

	active_irqs = R_REG(bme_info->osh, &regs->intstatus);

	if (in_isr) {
		active_irqs &= bme_info->m2m_intmask;
		if (active_irqs == 0) /* it is not for us */
			return 0;

		if (active_irqs == 0xffffffff) {
			return 0xffffffff;
		}

		// de-assert interrupt so isr will not be re-invoked, dpc will be scheduled
		bme_set_txs_intmask(bme_info, FALSE);
	} else { /* called from DPC */
		/* clears irqs */
		W_REG(bme_info->osh, &regs->intstatus, active_irqs);
	}

	return active_irqs;
} /* bme_m2m_intstatus */

void BCMFASTPATH
bme_set_txs_intmask(bme_info_t *bme_info, bool enable)
{
	volatile m2md_regs_t *regs = bme_info->m2m_regs;

	bme_info->m2m_intmask = enable ? M2M_INTCTL_TXS : 0;
	W_REG(bme_info->osh, &regs->intcontrol, bme_info->m2m_intmask);
}

#ifdef BCMQT
bool BCMFASTPATH
bme_is_m2m_irq(bme_info_t *bme_info)
{
	volatile m2md_regs_t *regs = bme_info->m2m_regs;
	uint32 intstatus = R_REG(bme_info->osh, &regs->intstatus);

	return ((bme_info->m2m_intmask & intstatus) != 0);
}
#endif /* BCMQT */

#endif /* BME_OFFLOADS_TXSTS */
