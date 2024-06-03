/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2015 Broadcom Corporation.
 */

#ifndef __BCM_UDC_OTG_H
#define __BCM_UDC_OTG_H

#include <common.h>

static inline void wfld_set(uintptr_t addr, uint32_t fld_val, uint32_t fld_mask)
{
	writel(((readl(addr) & ~(fld_mask)) | (fld_val)), (addr));
}

static inline void wfld_clear(uintptr_t addr, uint32_t fld_mask)
{
	writel((readl(addr) & ~(fld_mask)), (addr));
}

#endif
