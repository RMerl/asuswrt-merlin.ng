/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Board specific setup info
 *
 * (C) Copyright 2010
 * Texas Instruments, <www.ti.com>
 *
 * Author :
 *	Aneesh V	<aneesh@ti.com>
 */

#include <config.h>
#include <asm/arch/omap.h>
#include <asm/omap_common.h>
#include <asm/arch/spl.h>
#include <linux/linkage.h>

.arch_extension sec

#ifdef CONFIG_SPL
ENTRY(save_boot_params)
	ldr	r1, =OMAP_SRAM_SCRATCH_BOOT_PARAMS
	str	r0, [r1]
	b	save_boot_params_ret
ENDPROC(save_boot_params)

#if !defined(CONFIG_TI_SECURE_DEVICE) && defined(CONFIG_ARMV7_LPAE)
ENTRY(switch_to_hypervisor)

/*
 * Switch to hypervisor mode
 */
	adr	r0, save_sp
	str	sp, [r0]
	adr	r1, restore_from_hyp
	ldr	r0, =0x102
	b	omap_smc1
restore_from_hyp:
	adr	r0, save_sp
	ldr	sp, [r0]
	MRC p15, 4, R0, c1, c0, 0
	ldr     r1, =0X1004	@Set cache enable bits for hypervisor mode
	orr     r0, r0, r1
	MCR p15, 4, R0, c1, c0, 0
	b	switch_to_hypervisor_ret
save_sp:
	.word	0x0
ENDPROC(switch_to_hypervisor)
#endif
#endif

ENTRY(omap_smc1)
	push	{r4-r12, lr}	@ save registers - ROM code may pollute
				@ our registers
	mov	r12, r0		@ Service
	mov	r0, r1		@ Argument

	dsb
	dmb
	smc	0		@ SMC #0 to enter monitor mode
				@ call ROM Code API for the service requested
	pop	{r4-r12, pc}
ENDPROC(omap_smc1)

ENTRY(omap_smc_sec)
	push	{r4-r12, lr}	@ save registers - ROM code may pollute
				@ our registers
	mov	r6, #0xFF	@ Indicate new Task call
	mov	r12, #0x00	@ Secure Service ID in R12

	dsb
	dmb
	smc	0		@ SMC #0 to enter monitor mode

	b	omap_smc_sec_end @ exit at end of the service execution
	nop

	@ In case of IRQ happening in Secure, then ARM will branch here.
	@ At that moment, IRQ will be pending and ARM will jump to Non Secure
	@ IRQ handler
	mov	r12, #0xFE

	dsb
	dmb
	smc	0		@ SMC #0 to enter monitor mode

omap_smc_sec_end:
	pop	{r4-r12, pc}
ENDPROC(omap_smc_sec)
