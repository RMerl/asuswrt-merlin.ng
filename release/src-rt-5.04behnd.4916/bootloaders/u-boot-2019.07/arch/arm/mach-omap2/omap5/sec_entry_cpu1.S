/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Secure entry function for CPU Core #1
 *
 * (C) Copyright 2016
 * Texas Instruments, <www.ti.com>
 *
 * Author :
 *	Harinarayan Bhatta <harinarayan@ti.com>
 */

#include <config.h>
#include <asm/arch/omap.h>
#include <asm/omap_common.h>
#include <linux/linkage.h>

.arch_extension sec

#if !CONFIG_IS_ENABLED(SYS_DCACHE_OFF)
.global flush_dcache_range
#endif

#define AUX_CORE_BOOT_0		0x48281800
#define AUX_CORE_BOOT_1		0x48281804

#ifdef CONFIG_DRA7XX
/* DRA7xx ROM code function "startup_BootSlave". This function is where CPU1
 * waits on WFE, polling on AUX_CORE_BOOT_x registers.
 * This address is same for J6 and J6 Eco.
 */
#define ROM_FXN_STARTUP_BOOTSLAVE     0x00038a64
#endif

/* Assembly core where CPU1 is woken up into
 * No need to save-restore registers, does not use stack.
 */
LENTRY(cpu1_entry)
	ldr	r4, =omap_smc_sec_cpu1_args
	ldm	r4, {r0,r1,r2,r3}	@ Retrieve args

	mov	r6, #0xFF	@ Indicate new Task call
	mov	r12, #0x00	@ Secure Service ID in R12

	dsb
	dmb
	smc	0		@ SMC #0 to enter monitor mode

	b	.Lend		@ exit at end of the service execution
	nop

	@ In case of IRQ happening in Secure, then ARM will branch here.
	@ At that moment, IRQ will be pending and ARM will jump to Non Secure
	@ IRQ handler
	mov	r12, #0xFE

	dsb
	dmb
	smc	0		@ SMC #0 to enter monitor mode

.Lend:
	ldr	r4, =omap_smc_sec_cpu1_args
	str	r0, [r4, #0x10]		@ save return value
	ldr	r4, =AUX_CORE_BOOT_0
	mov	r5, #0x0
	str	r5, [r4]
	ldr	r4, =ROM_FXN_STARTUP_BOOTSLAVE
	sev				@ Tell CPU0 we are done
	bx	r4			@ Jump back to ROM
END(cpu1_entry)

/*
 * u32 omap_smc_sec_cpu1(u32 service, u32 proc_id, u32 flag, u32 *params);
 *
 * Makes a secure ROM/PPA call on CPU Core #1 on supported platforms.
 * Assumes that CPU #1 is waiting in ROM code and not yet woken up or used by
 * u-boot.
 */
ENTRY(omap_smc_sec_cpu1)
	push	{r4, r5, lr}
	ldr	r4, =omap_smc_sec_cpu1_args
	stm	r4, {r0,r1,r2,r3}	@ Save args to memory
#if !CONFIG_IS_ENABLED(SYS_DCACHE_OFF)
	mov	r0, r4
	mov	r1, #CONFIG_SYS_CACHELINE_SIZE
	add	r1, r0, r1		@ dcache is not enabled on CPU1, so
	blx	flush_dcache_range	@ flush the cache on args buffer
#endif
	ldr	r4, =AUX_CORE_BOOT_1
	ldr	r5, =cpu1_entry
	str	r5, [r4]		@ Setup CPU1 entry function
	ldr	r4, =AUX_CORE_BOOT_0
	mov	r5, #0x10
	str	r5, [r4]		@ Tell ROM to exit while loop
	sev				@ Wake up CPU1
.Lwait:
	wfe				@ Wait for CPU1 to finish
	nop
	ldr	r5, [r4]		@ Check if CPU1 is done
	cmp	r5, #0
	bne	.Lwait

	ldr	r4, =omap_smc_sec_cpu1_args
	ldr	r0, [r4, #0x10]         @ Retrieve return value
	pop	{r4, r5, pc}
ENDPROC(omap_smc_sec_cpu1)

/*
 * Buffer to save function arguments and return value for omap_smc_sec_cpu1
 */
.section .data
omap_smc_sec_cpu1_args:
#if !CONFIG_IS_ENABLED(SYS_DCACHE_OFF)
	.balign CONFIG_SYS_CACHELINE_SIZE
	.rept  CONFIG_SYS_CACHELINE_SIZE/4
	.word 0
	.endr
#else
	.rept 5
	.word 0
	.endr
#endif
END(omap_smc_sec_cpu1_args)
