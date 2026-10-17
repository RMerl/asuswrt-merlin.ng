/* SPDX-License-Identifier: GPL-2.0+ */
/*
 *  relocate - common relocation function for ARM U-Boot
 *
 *  Copyright (c) 2013  Albert ARIBAUD <albert.u.boot@aribaud.net>
 */

#include <asm-offsets.h>
#include <asm/assembler.h>
#include <config.h>
#include <elf.h>
#include <linux/linkage.h>
#ifdef CONFIG_CPU_V7M
#include <asm/armv7m.h>
#endif

/*
 * Default/weak exception vectors relocation routine
 *
 * This routine covers the standard ARM cases: normal (0x00000000),
 * high (0xffff0000) and VBAR. SoCs which do not comply with any of
 * the standard cases must provide their own, strong, version.
 */

	.section	.text.relocate_vectors,"ax",%progbits

WEAK(relocate_vectors)

#ifdef CONFIG_CPU_V7M
	/*
	 * On ARMv7-M we only have to write the new vector address
	 * to VTOR register.
	 */
	ldr	r0, [r9, #GD_RELOCADDR]	/* r0 = gd->relocaddr */
	ldr	r1, =V7M_SCB_BASE
	str	r0, [r1, V7M_SCB_VTOR]
#else
#ifdef CONFIG_HAS_VBAR
	/*
	 * If the ARM processor has the security extensions,
	 * use VBAR to relocate the exception vectors.
	 */
	ldr	r0, [r9, #GD_RELOCADDR]	/* r0 = gd->relocaddr */
	mcr     p15, 0, r0, c12, c0, 0  /* Set VBAR */
#else
	/*
	 * Copy the relocated exception vectors to the
	 * correct address
	 * CP15 c1 V bit gives us the location of the vectors:
	 * 0x00000000 or 0xFFFF0000.
	 */
	ldr	r0, [r9, #GD_RELOCADDR]	/* r0 = gd->relocaddr */
	mrc	p15, 0, r2, c1, c0, 0	/* V bit (bit[13]) in CP15 c1 */
	ands	r2, r2, #(1 << 13)
	ldreq	r1, =0x00000000		/* If V=0 */
	ldrne	r1, =0xFFFF0000		/* If V=1 */
	ldmia	r0!, {r2-r8,r10}
	stmia	r1!, {r2-r8,r10}
	ldmia	r0!, {r2-r8,r10}
	stmia	r1!, {r2-r8,r10}
#endif
#endif
	ret	lr

ENDPROC(relocate_vectors)

/*
 * void relocate_code(addr_moni)
 *
 * This function relocates the monitor code.
 *
 * NOTE:
 * To prevent the code below from containing references with an R_ARM_ABS32
 * relocation record type, we never refer to linker-defined symbols directly.
 * Instead, we declare literals which contain their relative location with
 * respect to relocate_code, and at run time, add relocate_code back to them.
 */

ENTRY(relocate_code)
relocate_base:
	adr	r3, relocate_base
	ldr	r1, _image_copy_start_ofs
	add	r1, r3			/* r1 <- Run &__image_copy_start */
	subs	r4, r0, r1		/* r4 <- Run to copy offset      */
	beq	relocate_done		/* skip relocation               */
	ldr	r1, _image_copy_start_ofs
	add	r1, r3			/* r1 <- Run &__image_copy_start */
	ldr	r2, _image_copy_end_ofs
	add	r2, r3			/* r2 <- Run &__image_copy_end   */
copy_loop:
	ldmia	r1!, {r10-r11}		/* copy from source address [r1] */
	stmia	r0!, {r10-r11}		/* copy to   target address [r0] */
	cmp	r1, r2			/* until source end address [r2] */
	blo	copy_loop

	/*
	 * fix .rel.dyn relocations
	 */
	ldr	r1, _rel_dyn_start_ofs
	add	r2, r1, r3		/* r2 <- Run &__rel_dyn_start */
	ldr	r1, _rel_dyn_end_ofs
	add	r3, r1, r3		/* r3 <- Run &__rel_dyn_end */
fixloop:
	ldmia	r2!, {r0-r1}		/* (r0,r1) <- (SRC location,fixup) */
	and	r1, r1, #0xff
	cmp	r1, #R_ARM_RELATIVE
	bne	fixnext

	/* relative fix: increase location by offset */
	add	r0, r0, r4
	ldr	r1, [r0]
	add	r1, r1, r4
	str	r1, [r0]
fixnext:
	cmp	r2, r3
	blo	fixloop

relocate_done:

#ifdef __XSCALE__
	/*
	 * On xscale, icache must be invalidated and write buffers drained,
	 * even with cache disabled - 4.2.7 of xscale core developer's manual
	 */
	mcr	p15, 0, r0, c7, c7, 0	/* invalidate icache */
	mcr	p15, 0, r0, c7, c10, 4	/* drain write buffer */
#endif

	ret	lr

ENDPROC(relocate_code)

_image_copy_start_ofs:
	.word	__image_copy_start - relocate_code
_image_copy_end_ofs:
	.word	__image_copy_end - relocate_code
_rel_dyn_start_ofs:
	.word	__rel_dyn_start - relocate_code
_rel_dyn_end_ofs:
	.word	__rel_dyn_end - relocate_code
