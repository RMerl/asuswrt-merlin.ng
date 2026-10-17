/* SPDX-License-Identifier: GPL-2.0+ */

#ifndef __ASM_ARCH_BOOT0_H__
#define __ASM_ARCH_BOOT0_H__

#include <asm/arch-rockchip/boot_mode.h>

/*
 * Execution starts on the instruction following this 4-byte header
 * (containing the magic 'RK30'). This magic constant will be written into
 * the final image by the rkimage tool, but we need to reserve space for it here.
 */
#ifdef CONFIG_SPL_BUILD
	b	1f  /* if overwritten, entry-address is at the next word */
1:
#endif

#if CONFIG_IS_ENABLED(ROCKCHIP_EARLYRETURN_TO_BROM)
/*
 * Keep track of the re-entries with help of the lr register.
 * This binary can be re-used and called from various BROM functions.
 * Only when it's called from the part that handles SPI, NAND or EMMC
 * hardware it needs to early return to BROM ones.
 * In download mode when it handles data on USB OTG and UART0
 * this section must be skipped.
 */
	ldr	r3, =CONFIG_ROCKCHIP_BOOT_LR_REG
	cmp	lr, r3          /* if (LR != CONFIG_ROCKCHIP_BOOT_LR_REG)        */
	bne	reset           /*     goto reset;                               */
/*
 * Unlike newer Rockchip SoC models the rk3066 BROM code does not have built-in
 * support to enter download mode on return to BROM. This binary must check
 * the boot mode register for the BOOT_BROM_DOWNLOAD flag and reset if it's set.
 * It then returns to BROM to the end of the function that reads boot blocks.
 * From there the BROM code goes into a download mode and waits for data
 * on USB OTG and UART0.
 */
	ldr	r2, =BOOT_BROM_DOWNLOAD
	ldr	r3, =CONFIG_ROCKCHIP_BOOT_MODE_REG
	ldr	r0, [r3]        /* if (readl(CONFIG_ROCKCHIP_BOOT_MODE_REG) !=   */
	cmp	r0, r2          /*     BOOT_BROM_DOWNLOAD) {                     */
	bne	early_return    /*     goto early_return;                        */
				/* } else {                                      */
	mov	r0, #0
	str	r0, [r3]        /*     writel(0, CONFIG_ROCKCHIP_BOOT_MODE_REG); */

	ldr	r3, =CONFIG_ROCKCHIP_BOOT_RETURN_REG
	bx	r3              /*     return to CONFIG_ROCKCHIP_BOOT_RETURN_REG;*/
				/* }                                             */
early_return:
	bx	lr              /* return to LR in BROM                          */

SAVE_SP_ADDR:
	.word 0

	.globl save_boot_params
save_boot_params:
	push	{r1-r12, lr}
	ldr	r0, =SAVE_SP_ADDR
	str	sp, [r0]
	b	save_boot_params_ret

	.globl back_to_bootrom
back_to_bootrom:
	ldr	r0, =SAVE_SP_ADDR
	ldr	sp, [r0]
	mov	r0, #0
	pop	{r1-r12, pc}
#endif

#if (defined(CONFIG_SPL_BUILD))
/* U-Boot proper of armv7 does not need this */
	b reset
#endif

/*
 * For armv7, the addr '_start' will be used as vector start address
 * and is written to the VBAR register, which needs to aligned to 0x20.
 */
	.align(5), 0x0
_start:
	ARM_VECTORS
#endif
