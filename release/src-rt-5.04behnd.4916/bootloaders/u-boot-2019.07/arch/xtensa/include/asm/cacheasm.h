/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2006 Tensilica Inc.
 * Copyright (C) 2014 - 2016 Cadence Design Systems Inc.
 */

#ifndef _XTENSA_CACHEASM_H
#define _XTENSA_CACHEASM_H

#include <asm/cache.h>
#include <asm/asmmacro.h>
#include <linux/stringify.h>

#define PAGE_SIZE 4096
#define DCACHE_WAY_SIZE (XCHAL_DCACHE_SIZE/XCHAL_DCACHE_WAYS)
#define ICACHE_WAY_SIZE (XCHAL_ICACHE_SIZE/XCHAL_ICACHE_WAYS)
#define DCACHE_WAY_SHIFT (XCHAL_DCACHE_SETWIDTH + XCHAL_DCACHE_LINEWIDTH)
#define ICACHE_WAY_SHIFT (XCHAL_ICACHE_SETWIDTH + XCHAL_ICACHE_LINEWIDTH)

/*
 * Define cache functions as macros here so that they can be used
 * by the kernel and boot loader. We should consider moving them to a
 * library that can be linked by both.
 *
 * Locking
 *
 *   ___unlock_dcache_all
 *   ___unlock_icache_all
 *
 * Flush and invaldating
 *
 *   ___flush_invalidate_dcache_{all|range|page}
 *   ___flush_dcache_{all|range|page}
 *   ___invalidate_dcache_{all|range|page}
 *   ___invalidate_icache_{all|range|page}
 *
 */

	.macro	__loop_cache_all ar at insn size line_width

	movi	\ar, 0

	__loopi	\ar, \at, \size, (4 << (\line_width))

	\insn	\ar, 0 << (\line_width)
	\insn	\ar, 1 << (\line_width)
	\insn	\ar, 2 << (\line_width)
	\insn	\ar, 3 << (\line_width)

	__endla	\ar, \at, 4 << (\line_width)

	.endm


	.macro	__loop_cache_range ar as at insn line_width

	extui	\at, \ar, 0, \line_width
	add	\as, \as, \at

	__loops	\ar, \as, \at, \line_width
	\insn	\ar, 0
	__endla	\ar, \at, (1 << (\line_width))

	.endm


	.macro	__loop_cache_page ar at insn line_width

	__loopi	\ar, \at, PAGE_SIZE, 4 << (\line_width)

	\insn	\ar, 0 << (\line_width)
	\insn	\ar, 1 << (\line_width)
	\insn	\ar, 2 << (\line_width)
	\insn	\ar, 3 << (\line_width)

	__endla	\ar, \at, 4 << (\line_width)

	.endm


	.macro	___unlock_dcache_all ar at

#if XCHAL_DCACHE_LINE_LOCKABLE && XCHAL_DCACHE_SIZE
	__loop_cache_all \ar \at diu XCHAL_DCACHE_SIZE XCHAL_DCACHE_LINEWIDTH
#endif

	.endm


	.macro	___unlock_icache_all ar at

#if XCHAL_ICACHE_LINE_LOCKABLE && XCHAL_ICACHE_SIZE
	__loop_cache_all \ar \at iiu XCHAL_ICACHE_SIZE XCHAL_ICACHE_LINEWIDTH
#endif

	.endm


	.macro	___flush_invalidate_dcache_all ar at

#if XCHAL_DCACHE_SIZE
	__loop_cache_all \ar \at diwbi XCHAL_DCACHE_SIZE XCHAL_DCACHE_LINEWIDTH
#endif

	.endm


	.macro	___flush_dcache_all ar at

#if XCHAL_DCACHE_SIZE
	__loop_cache_all \ar \at diwb XCHAL_DCACHE_SIZE XCHAL_DCACHE_LINEWIDTH
#endif

	.endm


	.macro	___invalidate_dcache_all ar at

#if XCHAL_DCACHE_SIZE
	__loop_cache_all \ar \at dii __stringify(DCACHE_WAY_SIZE) \
			 XCHAL_DCACHE_LINEWIDTH
#endif

	.endm


	.macro	___invalidate_icache_all ar at

#if XCHAL_ICACHE_SIZE
	__loop_cache_all \ar \at iii __stringify(ICACHE_WAY_SIZE) \
			 XCHAL_ICACHE_LINEWIDTH
#endif

	.endm



	.macro	___flush_invalidate_dcache_range ar as at

#if XCHAL_DCACHE_SIZE
	__loop_cache_range \ar \as \at dhwbi XCHAL_DCACHE_LINEWIDTH
#endif

	.endm


	.macro	___flush_dcache_range ar as at

#if XCHAL_DCACHE_SIZE
	__loop_cache_range \ar \as \at dhwb XCHAL_DCACHE_LINEWIDTH
#endif

	.endm


	.macro	___invalidate_dcache_range ar as at

#if XCHAL_DCACHE_SIZE
	__loop_cache_range \ar \as \at dhi XCHAL_DCACHE_LINEWIDTH
#endif

	.endm


	.macro	___invalidate_icache_range ar as at

#if XCHAL_ICACHE_SIZE
	__loop_cache_range \ar \as \at ihi XCHAL_ICACHE_LINEWIDTH
#endif

	.endm



	.macro	___flush_invalidate_dcache_page ar as

#if XCHAL_DCACHE_SIZE
	__loop_cache_page \ar \as dhwbi XCHAL_DCACHE_LINEWIDTH
#endif

	.endm


	.macro ___flush_dcache_page ar as

#if XCHAL_DCACHE_SIZE
	__loop_cache_page \ar \as dhwb XCHAL_DCACHE_LINEWIDTH
#endif

	.endm


	.macro	___invalidate_dcache_page ar as

#if XCHAL_DCACHE_SIZE
	__loop_cache_page \ar \as dhi XCHAL_DCACHE_LINEWIDTH
#endif

	.endm


	.macro	___invalidate_icache_page ar as

#if XCHAL_ICACHE_SIZE
	__loop_cache_page \ar \as ihi XCHAL_ICACHE_LINEWIDTH
#endif

	.endm

#endif	/* _XTENSA_CACHEASM_H */
