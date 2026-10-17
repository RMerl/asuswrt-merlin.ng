/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2017 Theobroma Systems Design und Consulting GmbH
 */

/*
 * Execution starts on the instruction following this 4-byte header
 * (containing the magic 'RK30', 'RK31', 'RK32' or 'RK33').  This
 * magic constant will be written into the final image by the rkimage
 * tool, but we need to reserve space for it here.
 *
 * To make life easier for everyone, we build the SPL binary with
 * space for this 4-byte header already included in the binary.
 */
#ifdef CONFIG_SPL_BUILD
	/*
	 * We need to add 4 bytes of space for the 'RK33' at the
	 * beginning of the executable.	 However, as we want to keep
	 * this generic and make it applicable to builds that are like
	 * the RK3368 (TPL needs this, SPL doesn't) or the RK3399 (no
	 * TPL, but extra space needed in the SPL), we simply insert
	 * a branch-to-next-instruction-word with the expectation that
	 * the first one may be overwritten, if this is the first stage
	 * contained in the final image created with mkimage)...
	 */
	b 1f	 /* if overwritten, entry-address is at the next word */
1:
#endif
#if CONFIG_IS_ENABLED(ROCKCHIP_EARLYRETURN_TO_BROM)
	adr     r3, entry_counter
	ldr	r0, [r3]
	cmp	r0, #1           /* check if entry_counter == 1 */
	beq	reset            /* regular bootup */
	add     r0, #1
	str	r0, [r3]         /* increment the entry_counter in memory */
	mov     r0, #0           /* return 0 to the BROM to signal 'OK' */
	bx	lr               /* return control to the BROM */
entry_counter:
	.word   0
#endif

#if (defined(CONFIG_SPL_BUILD) || defined(CONFIG_ARM64))
	/* U-Boot proper of armv7 do not need this */
	b reset
#endif

#if !defined(CONFIG_ARM64)
	/*
	 * For armv7, the addr '_start' will used as vector start address
	 * and write to VBAR register, which needs to aligned to 0x20.
	 */
	.align(5), 0x0
_start:
	ARM_VECTORS
#endif

#if !defined(CONFIG_TPL_BUILD) && defined(CONFIG_SPL_BUILD) && \
	(CONFIG_ROCKCHIP_SPL_RESERVE_IRAM > 0)
	.space CONFIG_ROCKCHIP_SPL_RESERVE_IRAM	/* space for the ATF data */
#endif
