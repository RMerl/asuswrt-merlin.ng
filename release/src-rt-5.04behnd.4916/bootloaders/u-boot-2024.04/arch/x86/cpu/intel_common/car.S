/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) 2014 Google, Inc
 *
 * From Coreboot file cpu/intel/model_206ax/cache_as_ram.inc
 *
 * Copyright (C) 2000,2007 Ronald G. Minnich <rminnich@gmail.com>
 * Copyright (C) 2005 Tyan (written by Yinghai Lu for Tyan)
 * Copyright (C) 2007-2008 coresystems GmbH
 * Copyright (C) 2012 Kyösti Mälkki <kyosti.malkki@gmail.com>
 */

#include <common.h>
#include <asm/microcode.h>
#include <asm/msr-index.h>
#include <asm/mtrr.h>
#include <asm/post.h>
#include <asm/processor.h>
#include <asm/processor-flags.h>

#define MTRR_PHYS_BASE_MSR(reg) (0x200 + 2 * (reg))
#define MTRR_PHYS_MASK_MSR(reg) (0x200 + 2 * (reg) + 1)

#define CACHE_AS_RAM_SIZE	CONFIG_DCACHE_RAM_SIZE
#define CACHE_AS_RAM_BASE	CONFIG_DCACHE_RAM_BASE

/* Cache 4GB - MRC_SIZE_KB for MRC */
#define CACHE_MRC_BYTES	((CONFIG_CACHE_MRC_SIZE_KB << 10) - 1)
#define CACHE_MRC_BASE		(0xFFFFFFFF - CACHE_MRC_BYTES)
#define CACHE_MRC_MASK		(~CACHE_MRC_BYTES)

#define CPU_PHYSMASK_HI	(1 << (CONFIG_CPU_ADDR_BITS - 32) - 1)

#define NOEVICTMOD_MSR	0x2e0

	/*
	 * Note: ebp must not be touched in this code as it holds the BIST
	 * value (built-in self test). We preserve this value until it can
	 * be written to global_data when CAR is ready for use.
	 */
.globl car_init
car_init:
	post_code(POST_CAR_START)

	/* Send INIT IPI to all excluding ourself */
	movl	$0x000C4500, %eax
	movl	$0xFEE00300, %esi
	movl	%eax, (%esi)

	/* TODO: Load microcode later - the 'no eviction' mode breaks this */
	movl	$MSR_IA32_UCODE_WRITE, %ecx
	xorl	%edx, %edx
	movl	$_dt_ucode_base_size, %eax
	movl	(%eax), %eax
	addl	$UCODE_HEADER_LEN, %eax
	wrmsr

	post_code(POST_CAR_SIPI)
	/* Zero out all fixed range and variable range MTRRs */
	movl	$mtrr_table, %esi
	movl	$((mtrr_table_end - mtrr_table) / 2), %edi
	xorl	%eax, %eax
	xorl	%edx, %edx
clear_mtrrs:
	movw	(%esi), %bx
	movzx	%bx, %ecx
	wrmsr
	add	$2, %esi
	dec	%edi
	jnz	clear_mtrrs

	post_code(POST_CAR_MTRR)
	/* Configure the default memory type to uncacheable */
	movl	$MTRR_DEF_TYPE_MSR, %ecx
	rdmsr
	andl	$(~0x00000cff), %eax
	wrmsr

	post_code(POST_CAR_UNCACHEABLE)
	/* Set Cache-as-RAM base address */
	movl	$(MTRR_PHYS_BASE_MSR(0)), %ecx
	movl	$(CACHE_AS_RAM_BASE | MTRR_TYPE_WRBACK), %eax
	xorl	%edx, %edx
	wrmsr

	post_code(POST_CAR_BASE_ADDRESS)
	/* Set Cache-as-RAM mask */
	movl	$(MTRR_PHYS_MASK_MSR(0)), %ecx
	movl	$(~(CACHE_AS_RAM_SIZE - 1) | MTRR_PHYS_MASK_VALID), %eax
	movl	$CPU_PHYSMASK_HI, %edx
	wrmsr

	post_code(POST_CAR_MASK)

	/* Enable MTRR */
	movl	$MTRR_DEF_TYPE_MSR, %ecx
	rdmsr
	orl	$MTRR_DEF_TYPE_EN, %eax
	wrmsr

	/* Enable cache (CR0.CD = 0, CR0.NW = 0) */
        movl	%cr0, %eax
	andl	$(~(X86_CR0_CD | X86_CR0_NW)), %eax
	invd
	movl	%eax, %cr0

	/* enable the 'no eviction' mode */
	movl    $NOEVICTMOD_MSR, %ecx
	rdmsr
	orl     $1, %eax
	andl    $~2, %eax
	wrmsr

       /* Clear the cache memory region. This will also fill up the cache */
	movl	$CACHE_AS_RAM_BASE, %esi
	movl	%esi, %edi
	movl	$(CACHE_AS_RAM_SIZE / 4), %ecx
	xorl	%eax, %eax
	rep	stosl

	/* enable the 'no eviction run' state */
	movl    $NOEVICTMOD_MSR, %ecx
	rdmsr
	orl     $3, %eax
	wrmsr

	post_code(POST_CAR_FILL)
	/* Enable Cache-as-RAM mode by disabling cache */
	movl	%cr0, %eax
	orl	$X86_CR0_CD, %eax
	movl	%eax, %cr0

	/* Enable cache for our code in Flash because we do XIP here */
	movl	$MTRR_PHYS_BASE_MSR(1), %ecx
	xorl	%edx, %edx
	movl    $car_init_ret, %eax
	andl    $(~(CONFIG_XIP_ROM_SIZE - 1)), %eax
	orl	$MTRR_TYPE_WRPROT, %eax
	wrmsr

	movl	$MTRR_PHYS_MASK_MSR(1), %ecx
	movl	$CPU_PHYSMASK_HI, %edx
	movl	$(~(CONFIG_XIP_ROM_SIZE - 1) | MTRR_PHYS_MASK_VALID), %eax
	wrmsr

	post_code(POST_CAR_ROM_CACHE)
#ifdef CONFIG_CACHE_MRC_BIN
	/* Enable caching for ram init code to run faster */
	movl	$MTRR_PHYS_BASE_MSR(2), %ecx
	movl	$(CACHE_MRC_BASE | MTRR_TYPE_WRPROT), %eax
	xorl	%edx, %edx
	wrmsr
	movl	$MTRR_PHYS_MASK_MSR(2), %ecx
	movl	$(CACHE_MRC_MASK | MTRR_PHYS_MASK_VALID), %eax
	movl	$CPU_PHYSMASK_HI, %edx
	wrmsr
#endif

	post_code(POST_CAR_MRC_CACHE)
	/* Enable cache */
	movl	%cr0, %eax
	andl	$(~(X86_CR0_CD | X86_CR0_NW)), %eax
	movl	%eax, %cr0

	post_code(POST_CAR_CPU_CACHE)

	/* All CPUs need to be in Wait for SIPI state */
wait_for_sipi:
	movl	(%esi), %eax
	bt	$12, %eax
	jc	wait_for_sipi

	/* return */
	jmp	car_init_ret

.globl car_uninit
car_uninit:
	/* Disable cache */
	movl	%cr0, %eax
	orl	$X86_CR0_CD, %eax
	movl	%eax, %cr0

	/* Disable MTRRs */
	movl	$MTRR_DEF_TYPE_MSR, %ecx
	rdmsr
	andl	$(~MTRR_DEF_TYPE_EN), %eax
	wrmsr

	/* Disable the no-eviction run state */
	movl    $NOEVICTMOD_MSR, %ecx
	rdmsr
	andl    $~2, %eax
	wrmsr

	invd

	/* Disable the no-eviction mode */
	rdmsr
	andl    $~1, %eax
	wrmsr

#ifdef CONFIG_CACHE_MRC_BIN
	/* Clear the MTRR that was used to cache MRC */
	xorl	%eax, %eax
	xorl	%edx, %edx
	movl	$MTRR_PHYS_BASE_MSR(2), %ecx
	wrmsr
	movl	$MTRR_PHYS_MASK_MSR(2), %ecx
	wrmsr
#endif

	/* Enable MTRRs */
	movl	$MTRR_DEF_TYPE_MSR, %ecx
	rdmsr
	orl	$MTRR_DEF_TYPE_EN, %eax
	wrmsr

	invd

	ret

mtrr_table:
	/* Fixed MTRRs */
	.word 0x250, 0x258, 0x259
	.word 0x268, 0x269, 0x26A
	.word 0x26B, 0x26C, 0x26D
	.word 0x26E, 0x26F
	/* Variable MTRRs */
	.word 0x200, 0x201, 0x202, 0x203
	.word 0x204, 0x205, 0x206, 0x207
	.word 0x208, 0x209, 0x20A, 0x20B
	.word 0x20C, 0x20D, 0x20E, 0x20F
	.word 0x210, 0x211, 0x212, 0x213
mtrr_table_end:

	.align 4
_dt_ucode_base_size:
	/* These next two fields are filled in by binman */
.globl ucode_base
ucode_base:	/* Declared in microcode.h */
	.long	0			/* microcode base */
.globl ucode_size
ucode_size:	/* Declared in microcode.h */
	.long	0			/* microcode size */
