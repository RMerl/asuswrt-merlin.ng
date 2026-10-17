/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2014-2015 Freescale Semiconductor
 * Copyright 2019-2022 NXP
 *
 * Extracted from armv8/start.S
 */

#include <config.h>
#include <linux/linkage.h>
#include <asm/gic.h>
#include <asm/macro.h>
#include <asm/arch-fsl-layerscape/soc.h>
#ifdef CONFIG_FSL_LSCH3
#include <asm/arch-fsl-layerscape/immap_lsch3.h>
#endif
#include <asm/u-boot.h>

	.align 3
	.weak secondary_boot_addr
secondary_boot_addr:
	.quad 0

/* Get GIC offset
* For LS1043a rev1.0, GIC base address align with 4k.
* For LS1043a rev1.1, if DCFG_GIC400_ALIGN[GIC_ADDR_BIT]
* is set, GIC base address align with 4K, or else align
* with 64k.
* output:
*	x0: the base address of GICD
*	x1: the base address of GICC
*/
ENTRY(get_gic_offset)
	ldr     x0, =GICD_BASE
#ifdef CONFIG_GICV2
	ldr     x1, =GICC_BASE
#endif
#ifdef CONFIG_HAS_FEATURE_GIC64K_ALIGN
	ldr     x2, =DCFG_CCSR_SVR
	ldr	w2, [x2]
	rev	w2, w2
	lsr	w3, w2, #16
	ldr	w4, =SVR_DEV(SVR_LS1043A)
	cmp	w3, w4
	b.ne	1f
	ands	w2, w2, #0xff
	cmp	w2, #REV1_0
	b.eq	1f
	ldr	x2, =SCFG_GIC400_ALIGN
	ldr	w2, [x2]
	rev	w2, w2
	tbnz	w2, #GIC_ADDR_BIT, 1f
	ldr     x0, =GICD_BASE_64K
#ifdef CONFIG_GICV2
	ldr     x1, =GICC_BASE_64K
#endif
1:
#endif
	ret
ENDPROC(get_gic_offset)

ENTRY(smp_kick_all_cpus)
	/* Kick secondary cpus up by SGI 0 interrupt */
#if defined(CONFIG_GICV2) || defined(CONFIG_GICV3)
	mov	x29, lr			/* Save LR */
	bl	get_gic_offset
	bl	gic_kick_secondary_cpus
	mov	lr, x29			/* Restore LR */
#endif
	ret
ENDPROC(smp_kick_all_cpus)


ENTRY(lowlevel_init)
	mov	x29, lr			/* Save LR */

	switch_el x1, 1f, 100f, 100f	/* skip if not in EL3 */
1:

#if defined (CONFIG_SYS_FSL_HAS_CCN504)

	/* Set Wuo bit for RN-I 20 */
#ifdef CONFIG_ARCH_LS2080A
	ldr	x0, =CCI_AUX_CONTROL_BASE(20)
	ldr	x1, =0x00000010
	bl	ccn504_set_aux

	/*
	 * Set forced-order mode in RNI-6, RNI-20
	 * This is required for performance optimization on LS2088A
	 * LS2080A family does not support setting forced-order mode,
	 * so skip this operation for LS2080A family
	 */
	bl	get_svr
	lsr	w0, w0, #16
	ldr	w1, =SVR_DEV(SVR_LS2080A)
	cmp	w0, w1
	b.eq	1f

	ldr	x0, =CCI_AUX_CONTROL_BASE(6)
	ldr	x1, =0x00000020
	bl	ccn504_set_aux
	ldr	x0, =CCI_AUX_CONTROL_BASE(20)
	ldr	x1, =0x00000020
	bl	ccn504_set_aux
1:
#endif

	/* Add fully-coherent masters to DVM domain */
	ldr	x0, =CCI_MN_BASE
	ldr	x1, =CCI_MN_RNF_NODEID_LIST
	ldr	x2, =CCI_MN_DVM_DOMAIN_CTL_SET
	bl	ccn504_add_masters_to_dvm

	/* Set all RN-I ports to QoS of 15 */
	ldr	x0, =CCI_S0_QOS_CONTROL_BASE(0)
	ldr	x1, =0x00FF000C
	bl	ccn504_set_qos
	ldr	x0, =CCI_S1_QOS_CONTROL_BASE(0)
	ldr	x1, =0x00FF000C
	bl	ccn504_set_qos
	ldr	x0, =CCI_S2_QOS_CONTROL_BASE(0)
	ldr	x1, =0x00FF000C
	bl	ccn504_set_qos

	ldr	x0, =CCI_S0_QOS_CONTROL_BASE(2)
	ldr	x1, =0x00FF000C
	bl	ccn504_set_qos
	ldr	x0, =CCI_S1_QOS_CONTROL_BASE(2)
	ldr	x1, =0x00FF000C
	bl	ccn504_set_qos
	ldr	x0, =CCI_S2_QOS_CONTROL_BASE(2)
	ldr	x1, =0x00FF000C
	bl	ccn504_set_qos

	ldr	x0, =CCI_S0_QOS_CONTROL_BASE(6)
	ldr	x1, =0x00FF000C
	bl	ccn504_set_qos
	ldr	x0, =CCI_S1_QOS_CONTROL_BASE(6)
	ldr	x1, =0x00FF000C
	bl	ccn504_set_qos
	ldr	x0, =CCI_S2_QOS_CONTROL_BASE(6)
	ldr	x1, =0x00FF000C
	bl	ccn504_set_qos

	ldr	x0, =CCI_S0_QOS_CONTROL_BASE(12)
	ldr	x1, =0x00FF000C
	bl	ccn504_set_qos
	ldr	x0, =CCI_S1_QOS_CONTROL_BASE(12)
	ldr	x1, =0x00FF000C
	bl	ccn504_set_qos
	ldr	x0, =CCI_S2_QOS_CONTROL_BASE(12)
	ldr	x1, =0x00FF000C
	bl	ccn504_set_qos

	ldr	x0, =CCI_S0_QOS_CONTROL_BASE(16)
	ldr	x1, =0x00FF000C
	bl	ccn504_set_qos
	ldr	x0, =CCI_S1_QOS_CONTROL_BASE(16)
	ldr	x1, =0x00FF000C
	bl	ccn504_set_qos
	ldr	x0, =CCI_S2_QOS_CONTROL_BASE(16)
	ldr	x1, =0x00FF000C
	bl	ccn504_set_qos

	ldr	x0, =CCI_S0_QOS_CONTROL_BASE(20)
	ldr	x1, =0x00FF000C
	bl	ccn504_set_qos
	ldr	x0, =CCI_S1_QOS_CONTROL_BASE(20)
	ldr	x1, =0x00FF000C
	bl	ccn504_set_qos
	ldr	x0, =CCI_S2_QOS_CONTROL_BASE(20)
	ldr	x1, =0x00FF000C
	bl	ccn504_set_qos
#endif /* CONFIG_SYS_FSL_HAS_CCN504 */

#ifdef SMMU_BASE
	/* Set the SMMU page size in the sACR register */
	ldr	x1, =SMMU_BASE
	ldr	w0, [x1, #0x10]
	orr	w0, w0, #1 << 16  /* set sACR.pagesize to indicate 64K page */
	str	w0, [x1, #0x10]
#endif

	/* Initialize GIC Secure Bank Status */
#if !defined(CONFIG_SPL_BUILD)
#if defined(CONFIG_GICV2) || defined(CONFIG_GICV3)
	branch_if_slave x0, 1f
	bl	get_gic_offset
	bl	gic_init_secure
1:
#ifdef CONFIG_GICV3
	ldr	x0, =GICR_BASE
	bl	gic_init_secure_percpu
#elif defined(CONFIG_GICV2)
	bl	get_gic_offset
	bl	gic_init_secure_percpu
#endif
#endif
#endif

100:
	branch_if_master x0, 2f

#if defined(CONFIG_MP) && defined(CONFIG_ARMV8_MULTIENTRY)
	/*
	 * Formerly, here was a jump to secondary_boot_func, but we just
	 * return early here and let the generic code in start.S handle
	 * the jump to secondary_boot_func.
	 */
	mov	lr, x29			/* Restore LR */
	ret
#endif

2:
	switch_el x1, 1f, 100f, 100f	/* skip if not in EL3 */
1:
#ifdef CONFIG_FSL_TZPC_BP147
	/* Set Non Secure access for all devices protected via TZPC */
	ldr	x1, =TZPCDECPROT_0_SET_BASE /* Decode Protection-0 Set Reg */
	orr	w0, w0, #1 << 3 /* DCFG_RESET is accessible from NS world */
	str	w0, [x1]

	isb
	dsb	sy
#endif

#ifdef CONFIG_FSL_TZASC_400
	/*
	 * LS2080 and its personalities does not support TZASC
	 * So skip TZASC related operations
	 */
	bl	get_svr
	lsr	w0, w0, #16
	ldr	w1, =SVR_DEV(SVR_LS2080A)
	cmp	w0, w1
	b.eq	1f

	/* Set TZASC so that:
	 * a. We use only Region0 whose global secure write/read is EN
	 * b. We use only Region0 whose NSAID write/read is EN
	 *
	 * NOTE: As per the CCSR map doc, TZASC 3 and TZASC 4 are just
	 *	 placeholders.
	 */

.macro tzasc_prog, xreg

	mov     x12, TZASC1_BASE
	mov     x16, #0x10000
	mul     x14, \xreg, x16
	add     x14, x14,x12
	mov	x1, #0x8
	add     x1, x1, x14

	ldr     w0, [x1]		/* Filter 0 Gate Keeper Register */
	orr     w0, w0, #1 << 0		/* Set open_request for Filter 0 */
	str     w0, [x1]

	mov	x1, #0x110
	add     x1, x1, x14

	ldr     w0, [x1]		/* Region-0 Attributes Register */
	orr     w0, w0, #1 << 31	/* Set Sec global write en, Bit[31] */
	orr     w0, w0, #1 << 30	/* Set Sec global read en, Bit[30] */
	str     w0, [x1]

	mov	x1, #0x114
	add     x1, x1, x14

	ldr     w0, [x1]		/* Region-0 Access Register */
	mov     w0, #0xFFFFFFFF		/* Set nsaid_wr_en and nsaid_rd_en */
	str     w0, [x1]
.endm

#ifdef CONFIG_FSL_TZASC_1
	mov     x13, #0
	tzasc_prog	x13

#endif
#ifdef CONFIG_FSL_TZASC_2
	mov     x13, #1
	tzasc_prog	x13

#endif
	isb
	dsb	sy
#endif
100:
1:
#ifdef CONFIG_ARCH_LS1046A
	switch_el x1, 1f, 100f, 100f	/* skip if not in EL3 */
1:
	/* Initialize the L2 RAM latency */
	mrs   x1, S3_1_c11_c0_2
	mov   x0, #0x1C7
	/* Clear L2 Tag RAM latency and L2 Data RAM latency */
	bic   x1, x1, x0
	/* Set L2 data ram latency bits [2:0] */
	orr   x1, x1, #0x2
	/* set L2 tag ram latency bits [8:6] */
	orr   x1,  x1, #0x80
	msr   S3_1_c11_c0_2, x1
	isb
100:
#endif

#if !defined(CONFIG_TFABOOT) && \
	(defined(CONFIG_FSL_LSCH2) && !defined(CONFIG_SPL_BUILD))
	bl	fsl_ocram_init
#endif

	mov	lr, x29			/* Restore LR */
	ret
ENDPROC(lowlevel_init)

#if defined(CONFIG_FSL_LSCH2) && !defined(CONFIG_SPL_BUILD)
ENTRY(fsl_ocram_init)
	mov	x28, lr			/* Save LR */
	bl	fsl_clear_ocram
	bl	fsl_ocram_clear_ecc_err
	mov	lr, x28			/* Restore LR */
	ret
ENDPROC(fsl_ocram_init)

ENTRY(fsl_clear_ocram)
/* Clear OCRAM */
	ldr	x0, =CFG_SYS_FSL_OCRAM_BASE
	ldr	x1, =(CFG_SYS_FSL_OCRAM_BASE + CFG_SYS_FSL_OCRAM_SIZE)
	mov	x2, #0
clear_loop:
	str	x2, [x0]
	add	x0, x0, #8
	cmp	x0, x1
	b.lo	clear_loop
	ret
ENDPROC(fsl_clear_ocram)

ENTRY(fsl_ocram_clear_ecc_err)
	/* OCRAM1/2 ECC status bit */
	mov	w1, #0x60
	ldr	x0, =DCSR_DCFG_SBEESR2
	str	w1, [x0]
	ldr	x0, =DCSR_DCFG_MBEESR2
	str	w1, [x0]
	ret
ENDPROC(fsl_ocram_clear_ecc_err)
#endif

#ifdef CONFIG_FSL_LSCH3
	.globl get_svr
get_svr:
	ldr	x1, =FSL_LSCH3_SVR
	ldr	w0, [x1]
	ret
#endif

#if defined(CONFIG_SYS_FSL_HAS_CCN504) || defined(CONFIG_SYS_FSL_HAS_CCN508)
hnf_pstate_poll:
	/* x0 has the desired status, return only if operation succeed
	 * clobber x1, x2, x6
	 */
	mov	x1, x0
	mov	w6, #8			/* HN-F node count */
	mov	x0, #0x18
	movk	x0, #0x420, lsl #16	/* HNF0_PSTATE_STATUS */
1:
	ldr	x2, [x0]
	cmp	x2, x1			/* check status */
	b.eq	2f
	b	1b
2:
	add	x0, x0, #0x10000	/* move to next node */
	subs	w6, w6, #1
	cbnz	w6, 1b
	ret

hnf_set_pstate:
	/* x0 has the desired state, clobber x1, x2, x6 */
	mov	x1, x0
	/* power state to SFONLY */
	mov	w6, #8			/* HN-F node count */
	mov	x0, #0x10
	movk	x0, #0x420, lsl #16	/* HNF0_PSTATE_REQ */
1:	/* set pstate to sfonly */
	ldr	x2, [x0]
	and	x2, x2, #0xfffffffffffffffc	/* & HNFPSTAT_MASK */
	orr	x2, x2, x1
	str	x2, [x0]
	add	x0, x0, #0x10000	/* move to next node */
	subs	w6, w6, #1
	cbnz	w6, 1b

	ret

ENTRY(__asm_flush_l3_dcache)
	/*
	 * Return status in x0
	 *    success 0
	 */
	mov	x29, lr

	dsb	sy
	mov	x0, #0x1		/* HNFPSTAT_SFONLY */
	bl	hnf_set_pstate

	mov	x0, #0x4		/* SFONLY status */
	bl	hnf_pstate_poll

	dsb	sy
	mov	x0, #0x3		/* HNFPSTAT_FAM */
	bl	hnf_set_pstate

	mov	x0, #0xc		/* FAM status */
	bl	hnf_pstate_poll

	mov	x0, #0
	mov	lr, x29
	ret
ENDPROC(__asm_flush_l3_dcache)
#endif /* CONFIG_SYS_FSL_HAS_CCN504 */
