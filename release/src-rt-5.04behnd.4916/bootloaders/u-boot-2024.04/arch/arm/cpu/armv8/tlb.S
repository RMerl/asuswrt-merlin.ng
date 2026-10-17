/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2013
 * David Feng <fenghua@phytium.com.cn>
 */

#include <asm-offsets.h>
#include <config.h>
#include <linux/linkage.h>
#include <asm/macro.h>

/*
 * void __asm_invalidate_tlb_all(void)
 *
 * invalidate all tlb entries.
*/
.pushsection .text.__asm_invalidate_tlb_all, "ax"
ENTRY(__asm_invalidate_tlb_all)
	switch_el x9, 3f, 2f, 1f
3:	tlbi	alle3
	dsb	sy
	isb
	b	0f
2:	tlbi	alle2
	dsb	sy
	isb
	b	0f
1:	tlbi	vmalle1
	dsb	sy
	isb
0:
	ret
ENDPROC(__asm_invalidate_tlb_all)
.popsection
