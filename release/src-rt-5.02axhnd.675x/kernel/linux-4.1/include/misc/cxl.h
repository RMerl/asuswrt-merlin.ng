/*
 * Copyright 2014 IBM Corp.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version
 * 2 of the License, or (at your option) any later version.
 */

#ifndef _MISC_CXL_H
#define _MISC_CXL_H

#ifdef CONFIG_CXL_BASE

#define CXL_IRQ_RANGES 4

struct cxl_irq_ranges {
	irq_hw_number_t offset[CXL_IRQ_RANGES];
	irq_hw_number_t range[CXL_IRQ_RANGES];
};

extern atomic_t cxl_use_count;

static inline bool cxl_ctx_in_use(void)
{
       return (atomic_read(&cxl_use_count) != 0);
}

static inline void cxl_ctx_get(void)
{
       atomic_inc(&cxl_use_count);
}

static inline void cxl_ctx_put(void)
{
       atomic_dec(&cxl_use_count);
}

void cxl_slbia(struct mm_struct *mm);

#else /* CONFIG_CXL_BASE */

static inline bool cxl_ctx_in_use(void) { return false; }
static inline void cxl_slbia(struct mm_struct *mm) {}

#endif /* CONFIG_CXL_BASE */

#endif
