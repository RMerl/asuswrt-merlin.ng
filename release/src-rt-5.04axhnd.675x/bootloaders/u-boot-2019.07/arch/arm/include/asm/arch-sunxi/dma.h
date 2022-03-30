/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2015 Roy Spliet <rspliet@ultimaker.com>
 */

#ifndef _SUNXI_DMA_H
#define _SUNXI_DMA_H

#if defined(CONFIG_MACH_SUN4I) || defined(CONFIG_MACH_SUN5I) || defined(CONFIG_MACH_SUN7I)
#include <asm/arch/dma_sun4i.h>
#else
#error "DMA definition not available for this architecture"
#endif

#endif /* _SUNXI_DMA_H */
