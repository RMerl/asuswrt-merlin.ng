/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2011-2012
 * Pali Rohár <pali.rohar@gmail.com>
 */

#include <config.h>

relocaddr:		/* address of this relocaddr section after coping */
	.word .		/* address of section (calculated at compile time) */

startaddr:		/* address of u-boot after copying */
	.word CONFIG_SYS_TEXT_BASE

kernaddr:		/* address of kernel after copying */
	.word KERNEL_ADDRESS

kernsize:		/* maximal size of kernel image */
	.word KERNEL_MAXSIZE

kernoffs:		/* offset of kernel image in loaded u-boot */
	.word KERNEL_OFFSET

imagesize:		/* maximal size of image */
	.word IMAGE_MAXSIZE

ih_magic:		/* IH_MAGIC in big endian from include/image.h */
	.word 0x56190527

/*
 * Routine: save_boot_params (called after reset from start.S)
 * Description: Copy attached kernel to address KERNEL_ADDRESS
 *              Copy u-boot to address CONFIG_SYS_TEXT_BASE
 *              Return to copied u-boot address
 */

.global save_boot_params
save_boot_params:
	/* Get return address */
	ldr	lr, =save_boot_params_ret

/* Copy valid attached kernel to address KERNEL_ADDRESS */

copy_kernel_start:
	adr	r0, relocaddr	/* r0 - address of section relocaddr */
	ldr	r1, relocaddr	/* r1 - address of relocaddr after relocation */
	cmp	r0, r1

	/* r4 - calculated offset */
	subhi	r4, r0, r1
	sublo	r4, r1, r0

	/* r0 - start of kernel before */
	ldr	r0, startaddr
	addhi	r0, r0, r4
	sublo	r0, r0, r4
	ldr	r1, kernoffs
	add	r0, r0, r1

	/* r3 - start of kernel after */
	ldr	r3, kernaddr

	/* r2 - end of kernel after */
	ldr	r1, kernsize
	add	r2, r3, r1

	/* r1 - end of kernel before */
	add	r1, r0, r1

	/* remove header in target kernel */
	mov	r5, #0
	str	r5, [r3]

	/* check for valid kernel uImage */
	ldr	r4, [r0]	/* r4 - 4 bytes header of kernel */
	ldr	r5, ih_magic	/* r5 - IH_MAGIC */
	cmp	r4, r5
	bne	copy_kernel_end	/* skip if invalid image */

copy_kernel_loop:
	ldmdb	r1!, {r3 - r10}
	stmdb	r2!, {r3 - r10}
	cmp	r1, r0
	bhi	copy_kernel_loop

copy_kernel_end:
	mov	r5, #0
	str	r5, [r0]	/* remove 4 bytes header of kernel */


/* Fix u-boot code */

fix_start:
	adr	r0, relocaddr	/* r0 - address of section relocaddr */
	ldr	r1, relocaddr	/* r1 - address of relocaddr after relocation */
	cmp	r0, r1

	beq	copy_uboot_end	/* skip if u-boot is on correct address */

	/* r5 - calculated offset */
	subhi	r5, r0, r1
	sublo	r5, r1, r0

	/* r6 - maximal u-boot size */
	ldr	r6, imagesize

	/* r1 - start of u-boot after */
	ldr	r1, startaddr

	/* r0 - start of u-boot before */
	addhi	r0, r1, r5
	sublo	r0, r1, r5

	/* check if we need to move uboot copy code before calling it */
	cmp	r5, r6
	bhi	copy_uboot_start /* now coping u-boot code directly is safe */


copy_code_start:
	/* r0 - start of u-boot before */
	/* r1 - start of u-boot after */
	/* r6 - maximal u-boot size */

	/* r7 - maximal kernel size */
	ldr	r7, kernsize

	/* r4 - end of kernel before */
	add	r4, r0, r6
	add	r4, r4, r7

	/* r5 - end of u-boot after */
	ldr	r5, startaddr
	add	r5, r5, r6

	/* r2 - start of loop code after */
	cmp	r4, r5		/* higher address (r4 or r5) */
	movhs	r2, r4
	movlo	r2, r5

	/* r3 - end of loop code before */
	adr	r3, end

	/* r4 - end of loop code after */
	adr	r4, copy_uboot_start
	sub	r4, r3, r4
	add	r4, r2, r4

copy_code_loop:
	ldmdb	r3!, {r7 - r10}
	stmdb	r4!, {r7 - r10}
	cmp	r4, r2
	bhi	copy_code_loop

copy_code_end:
	mov	pc, r2


/* Copy u-boot to address CONFIG_SYS_TEXT_BASE */

copy_uboot_start:
	/* r0 - start of u-boot before */
	/* r1 - start of u-boot after */
	/* r6 - maximal u-boot size */

	/* r2 - end of u-boot after */
	add	r2, r1, r6

	/* condition for copying from left to right */
	cmp	r0, r1
	addlo	r1, r0, r6	/* r1 - end of u-boot before */
	blo	copy_uboot_loop_right

copy_uboot_loop_left:
	ldmia	r0!, {r3 - r10}
	stmia	r1!, {r3 - r10}
	cmp	r1, r2
	blo	copy_uboot_loop_left
	b	copy_uboot_end

copy_uboot_loop_right:
	ldmdb	r1!, {r3 - r10}
	stmdb	r2!, {r3 - r10}
	cmp	r1, r0
	bhi	copy_uboot_loop_right

copy_uboot_end:
	bx	lr

end:
