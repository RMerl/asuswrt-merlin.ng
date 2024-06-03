/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) 2016, NVIDIA CORPORATION.
 */

#include <config.h>
#include <linux/linkage.h>

#define SMC_SIP_INVOKE_MCE		0x82FFFF00
#define MCE_SMC_ROC_FLUSH_CACHE		(SMC_SIP_INVOKE_MCE | 11)
#define MCE_SMC_ROC_FLUSH_CACHE_ONLY	(SMC_SIP_INVOKE_MCE | 14)
#define MCE_SMC_ROC_CLEAN_CACHE_ONLY	(SMC_SIP_INVOKE_MCE | 15)

ENTRY(__asm_tegra_cache_smc)
	mov	x1, #0
	mov	x2, #0
	mov	x3, #0
	mov	x4, #0
	mov	x5, #0
	mov	x6, #0
	smc	#0
	mov	x0, #0
	ret
ENDPROC(__asm_invalidate_l3_dcache)

ENTRY(__asm_invalidate_l3_dcache)
	mov	x0, #(MCE_SMC_ROC_FLUSH_CACHE_ONLY & 0xffff)
	movk	x0, #(MCE_SMC_ROC_FLUSH_CACHE_ONLY >> 16), lsl #16
	b	__asm_tegra_cache_smc
ENDPROC(__asm_invalidate_l3_dcache)

ENTRY(__asm_flush_l3_dcache)
	mov	x0, #(MCE_SMC_ROC_CLEAN_CACHE_ONLY & 0xffff)
	movk	x0, #(MCE_SMC_ROC_CLEAN_CACHE_ONLY >> 16), lsl #16
	b	__asm_tegra_cache_smc
ENDPROC(__asm_flush_l3_dcache)

ENTRY(__asm_invalidate_l3_icache)
	mov	x0, #(MCE_SMC_ROC_FLUSH_CACHE & 0xffff)
	movk	x0, #(MCE_SMC_ROC_FLUSH_CACHE >> 16), lsl #16
	b	__asm_tegra_cache_smc
ENDPROC(__asm_invalidate_l3_icache)
