/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (C) 2020 Texas Instruments Incorporated - https://www.ti.com/
 *	Andrew F. Davis <afd@ti.com>
 */

#include <config.h>
#include <linux/linkage.h>

#if defined(CONFIG_SPL_BUILD)
ENTRY(__asm_invalidate_l3_dcache)
	/* Invalidate SPL address range */
	mov x0, #CONFIG_SPL_TEXT_BASE
	add x1, x0, #CONFIG_SPL_MAX_SIZE
	b __asm_flush_dcache_range
ENDPROC(__asm_invalidate_l3_dcache)

ENTRY(__asm_flush_l3_dcache)
	/* Flush SPL address range */
	mov x0, #CONFIG_SPL_TEXT_BASE
	add x1, x0, #CONFIG_SPL_MAX_SIZE
	b __asm_flush_dcache_range
ENDPROC(__asm_flush_l3_dcache)
#endif
