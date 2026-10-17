/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2013-2014 Synopsys, Inc. All rights reserved.
 */

#ifndef __ASM_ARC_CACHE_H
#define __ASM_ARC_CACHE_H

/*
 * As of today we may handle any L1 cache line length right in software.
 * For that essentially cache line length is a variable not constant.
 * And to satisfy users of ARCH_DMA_MINALIGN we just use largest line length
 * that may exist in either L1 or L2 (AKA SLC) caches on ARC.
 */
#define ARCH_DMA_MINALIGN	128

#ifndef __ASSEMBLY__

void cache_init(void);
void flush_n_invalidate_dcache_all(void);
void sync_n_cleanup_cache_all(void);

static const inline int is_ioc_enabled(void)
{
	return IS_ENABLED(CONFIG_ARC_DBG_IOC_ENABLE);
}

/*
 * We export SLC control functions to use them in platform configuration code.
 * They maust not be used in any generic code!
 */
void slc_enable(void);
void slc_disable(void);

#endif /* __ASSEMBLY__ */

#endif /* __ASM_ARC_CACHE_H */
