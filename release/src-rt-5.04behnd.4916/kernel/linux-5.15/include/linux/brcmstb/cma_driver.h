/*
 *  cma_driver.h - Broadcom STB platform CMA driver
 *
 *  Copyright (C) 2009 - 2013 Broadcom Corporation
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2, or (at your option)
 *  any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; see the file COPYING.  If not, write to
 *  the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.
 */
#ifndef __CMA_DRIVER_H__
#define __CMA_DRIVER_H__

#include <linux/device.h>
#include <linux/platform_device.h>
#include <linux/ioctl.h>
#include <linux/uaccess.h>
#include <linux/list.h>
#include <linux/dma-map-ops.h>
#include <linux/brcmstb/memory_api.h>
#include <uapi/linux/brcmstb/cma_driver.h>

/* Incremented when public API changes */
#define CMA_DRIVER_VERSION  0

struct mem_range {
	u64 base;
	u32 size;
};

struct region_list {
	struct list_head list;
	struct mem_range region;
};

struct cma_dev {
	struct device *dev;
	u32 cma_dev_index;
	struct mem_range range; /* addresses that this cma_dev controls */
	struct list_head list;
	int memc;
};

/*
 * Note to kernel module / driver developers:
 * This interface is subject to change!
 *
 * For more information, please refer to:
 * http://jira.broadcom.com/browse/SWLINUX-2473
 */
struct cma_dev *cma_dev_get_cma_dev(int memc);
int cma_dev_get_mem(struct cma_dev *cma_dev, u64 *addr, u32 len,
	u32 align);
int cma_dev_put_mem(struct cma_dev *cma_dev, u64 addr, u32 len);
int cma_dev_get_num_regions(struct cma_dev *cma_dev);
int cma_dev_get_region_info(struct cma_dev *cma_dev, int region_num,
	s32 *memc, u64 *addr, u32 *num_bytes);
int cma_drvr_is_ready(void);

static inline void *cma_dev_kva_map(struct page *page, int num_pages,
		pgprot_t pgprot)
{
	return brcmstb_memory_kva_map(page, num_pages, pgprot);
}

static inline int cma_dev_kva_unmap(const void *kva)
{
	return brcmstb_memory_kva_unmap(kva);
}

/* Below functions are for calling during initialization and may need stubs */

#ifdef CONFIG_BRCMSTB_CMA
void __init cma_reserve(void);
void __init cma_register(void);
#else
static inline void cma_reserve(void) {}
static inline void cma_register(void) {}
#endif

#endif /* __CMA_DRIVER_H__ */
