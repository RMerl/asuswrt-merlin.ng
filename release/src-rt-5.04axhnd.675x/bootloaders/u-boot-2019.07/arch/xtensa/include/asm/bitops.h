/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2001 - 2012 Tensilica Inc.
 * Copyright (C) 2014 - 2016 Cadence Design Systems Inc.
 */

#ifndef _XTENSA_BITOPS_H
#define _XTENSA_BITOPS_H

#include <asm/system.h>
#include <asm-generic/bitops/fls.h>
#include <asm-generic/bitops/__fls.h>
#include <asm-generic/bitops/fls64.h>
#include <asm-generic/bitops/__ffs.h>

static inline int test_bit(int nr, const void *addr)
{
	return ((unsigned char *)addr)[nr >> 3] & (1u << (nr & 7));
}

static inline int test_and_set_bit(int nr, volatile void *addr)
{
	unsigned long flags;
	unsigned char tmp;
	unsigned char mask = 1u << (nr & 7);

	local_irq_save(flags);
	tmp = ((unsigned char *)addr)[nr >> 3];
	((unsigned char *)addr)[nr >> 3] |= mask;
	local_irq_restore(flags);

	return tmp & mask;
}

#endif	/* _XTENSA_BITOPS_H */
