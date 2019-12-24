/*
 * Generic Broadcom Home Networking Division (HND) BME module.
 * This supports chips with revs >= 128.
 *
 * Copyright (C) 2019, Broadcom. All Rights Reserved.
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

/**
 * XXX For more information, see:
 * Confluence:[BME%2C+the+non-descriptor+based+DMA+engine]
 */

#include <osl.h>
#include <siutils.h>
#include <hndbme.h>
#include <sbhnddma.h>
#include <bcmutils.h>
#include <hndsoc.h>
#include <m2mdma_core.h>	/* for m2md_regs_t */

#define SIZE_64BIT_PTRS				8

#define	PCI64ADDR_HIGH				0x80000000	/* address[63] */

#define DMA_OFFSET_BME_BASE_CHANNEL2_XMT	0x280
#define DMA_OFFSET_BME_BASE_CHANNEL2_RCV	0x2A0
#define DMA_OFFSET_BME_BASE_CHANNEL3_XMT	0x2C0
#define DMA_OFFSET_BME_BASE_CHANNEL3_RCV	0x2E0

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

#define BMEREG_RCVCONTROL_WCPD_SHIFT		(1 << 28)

#define BMEREG_XMTPTR_BUFFCOUNT_MASK		0x1FFF
#define BMEREG_XMTPTR_ADDREXT			(1 << 13)
#define BMEREG_XMTPTR_COHERENT			(1 << 14)
#define BMEREG_XMTPTR_START_BUSY		(1 << 31)

#define BMEREG_RCVPTR_ADDREXT			(1 << 13)
#define BMEREG_RCVPTR_COHERENT			(1 << 14)

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

struct bme_info_s {
	si_t			*sih;
	void			*osh;		    /**< os handle */
	volatile dma64regs_t	*xmt;
	volatile dma64regs_t	*rcv;
	volatile m2md_regs_t	*m2m_regs;          /**< virtual address of m2m core registers */
	uint32			hi_src;
	uint32			hi_dest;
	uint32			flags;
	bme_channel_t		channel;
	bool			config_written;
};

bme_info_t *
BCMATTACHFN(bme_attach)(si_t *sih, bme_channel_t channel)
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

	return bme_info;
} /* bme_attach */

void
BCMATTACHFN(bme_detach)(bme_info_t *bme_info)
{
	uint saved_core_idx;
	uint intr_val;
	struct bme_circ_buf_s *p;

	BCM_REFERENCE(p);
	BME_SWITCHCORE(bme_info->sih, &saved_core_idx, &intr_val);

	AND_REG(bme_info->osh, &bme_info->xmt->control, ~BMEREG_CONTROL_EN);
	AND_REG(bme_info->osh, &bme_info->rcv->control, ~BMEREG_CONTROL_EN);

	BME_RESTORECORE(bme_info->sih, saved_core_idx, intr_val);

	MFREE(bme_info->osh, (void *)bme_info, sizeof(*bme_info));
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
#if (defined(BCA_CPEROUTER) || defined(BCA_HNDROUTER)) && (defined(CONFIG_BCM963178) || \
	defined(CONFIG_BCM947622))
	uint i;
	while (R_REG(bme_info->osh, &bme_info->xmt->ptr) & BMEREG_XMTPTR_START_BUSY) {
		if (++i == 10000) {
			printf("bme_sync failed? DMA register dump:");
			for (i = 0; i < 12; i++) {
				printf("%08x\n", ((uint32*)(&bme_info->xmt))[i]);
			}
			printf("M2M register dump:");
			for (i = 0; i < 12; i++) {
				printf("%08x\n", ((uint32*)(&bme_info->m2m_regs))[i]);
			}
			i = 10000;
		}
	}
#else
	while (R_REG(bme_info->osh, &bme_info->xmt->ptr) & BMEREG_XMTPTR_START_BUSY) {
		;
	}
#endif /* (BCA_CPEROUTER || BCA_HNDROUTER) && (CONFIG_BCM963178 || CONFIG_BCM947622) */

	BME_RESTORECORE(bme_info->sih, saved_core_idx, intr_val);
}
