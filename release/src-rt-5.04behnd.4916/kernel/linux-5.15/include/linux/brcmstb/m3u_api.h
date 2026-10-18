/* SPDX-License-Identifier: GPL-2.0 */
/* Copyright (C) 2018 Broadcom */
#ifndef _BRCMSTB_M3U_API_H_
#define _BRCMSTB_M3U_API_H_

#include <linux/errno.h>
#include <linux/types.h>
/*
 * brcm_nexus_m3u_init - Initialize the m3u unit that is targeted for Nexus
 *   HVS use.  This "targeted" m3u must have its DT node contain the
 *   "brcm,type" property with the value "nexus".  Subsequently, any other
 *   DT node -- such as a unicam or pisp -- that is connected to this m3u
 *   must have the "iommus" property specified in its DT node (and it must
 *   point to the targeted m3u node).
 *
 * @l1_mmu_va: kernel virtual pointer to the base of the m3u l1 table.
 *   The caller is responsible for allocating and deallocating the memory
 *   for the L1 as well as the L2 pages that are required for the specified
 *   range.
 *
 * @dma_base: The base of the IOVA range/region that is to be used in
 *   supporting the devices attached to this m3u.  Specifically, the
 *   rangeis as follows: [dma_base...(dma_basebo + size - 1)]
 *
 * @size: The size of the allowed DMA region.
 *
 * @flush: Callback function that has the responsibility of clearing both
 *   m3u TLB and MMUC entries that pertain to the specified range.  The
 *   Linux m3u driver holds the responsibility of flushing the specified
 *   range from the CPU caches to memory.
 */

/* This functionality has been obsoleted */
static inline int brcm_nexus_m3u_init(void *l1_mmu_va, u64 dma_base,
			size_t size, int (*flush)(u64 dma_addr, size_t size))
{
	return -EOPNOTSUPP;
}

/*
 * TBD, currently unimplemented.
 */
static inline int brcm_nexus_m3u_uninit(void)
{
	return -EOPNOTSUPP;
}

#endif /* _BRCMSTB_M3U_API_H_ */
