/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Miscellaneous assembly functions.
 *
 * Copyright (C) 2001 - 2007 Tensilica Inc.
 * Copyright (C) 2014 - 2016 Cadence Design Systems Inc.
 *
 * Chris Zankel	<chris@zankel.net>
 */


#include <linux/linkage.h>
#include <asm/asmmacro.h>
#include <asm/cacheasm.h>

/*
 * void __invalidate_icache_page(ulong start)
 */

ENTRY(__invalidate_icache_page)

	abi_entry

	___invalidate_icache_page a2 a3
	isync

	abi_ret

ENDPROC(__invalidate_icache_page)

/*
 * void __invalidate_dcache_page(ulong start)
 */

ENTRY(__invalidate_dcache_page)

	abi_entry

	___invalidate_dcache_page a2 a3
	dsync

	abi_ret

ENDPROC(__invalidate_dcache_page)

/*
 * void __flush_invalidate_dcache_page(ulong start)
 */

ENTRY(__flush_invalidate_dcache_page)

	abi_entry

	___flush_invalidate_dcache_page a2 a3

	dsync
	abi_ret

ENDPROC(__flush_invalidate_dcache_page)

/*
 * void __flush_dcache_page(ulong start)
 */

ENTRY(__flush_dcache_page)

	abi_entry

	___flush_dcache_page a2 a3

	dsync
	abi_ret

ENDPROC(__flush_dcache_page)

/*
 * void __invalidate_icache_range(ulong start, ulong size)
 */

ENTRY(__invalidate_icache_range)

	abi_entry

	___invalidate_icache_range a2 a3 a4
	isync

	abi_ret

ENDPROC(__invalidate_icache_range)

/*
 * void __flush_invalidate_dcache_range(ulong start, ulong size)
 */

ENTRY(__flush_invalidate_dcache_range)

	abi_entry

	___flush_invalidate_dcache_range a2 a3 a4
	dsync

	abi_ret

ENDPROC(__flush_invalidate_dcache_range)

/*
 * void _flush_dcache_range(ulong start, ulong size)
 */

ENTRY(__flush_dcache_range)

	abi_entry

	___flush_dcache_range a2 a3 a4
	dsync

	abi_ret

ENDPROC(__flush_dcache_range)

/*
 * void _invalidate_dcache_range(ulong start, ulong size)
 */

ENTRY(__invalidate_dcache_range)

	abi_entry

	___invalidate_dcache_range a2 a3 a4

	abi_ret

ENDPROC(__invalidate_dcache_range)

/*
 * void _invalidate_icache_all(void)
 */

ENTRY(__invalidate_icache_all)

	abi_entry

	___invalidate_icache_all a2 a3
	isync

	abi_ret

ENDPROC(__invalidate_icache_all)

/*
 * void _flush_invalidate_dcache_all(void)
 */

ENTRY(__flush_invalidate_dcache_all)

	abi_entry

	___flush_invalidate_dcache_all a2 a3
	dsync

	abi_ret

ENDPROC(__flush_invalidate_dcache_all)

/*
 * void _invalidate_dcache_all(void)
 */

ENTRY(__invalidate_dcache_all)

	abi_entry

	___invalidate_dcache_all a2 a3
	dsync

	abi_ret

ENDPROC(__invalidate_dcache_all)
