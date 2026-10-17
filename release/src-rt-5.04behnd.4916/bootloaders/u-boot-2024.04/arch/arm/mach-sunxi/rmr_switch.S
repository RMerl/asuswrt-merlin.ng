@
@ ARMv8 RMR reset sequence on Allwinner SoCs.
@
@ All 64-bit capable Allwinner SoCs reset in AArch32 (and continue to
@ exectute the Boot ROM in this state), so we need to switch to AArch64
@ at some point.
@ Section G6.2.133 of the ARMv8 ARM describes the Reset Management Register
@ (RMR), which triggers a warm-reset of a core and can request to switch
@ into a different execution state (AArch32 or AArch64).
@ The address at which execution starts after the reset is held in the
@ RVBAR system register, which is architecturally read-only.
@ Allwinner provides a writable alias of this register in MMIO space, so
@ we can easily set the start address of AArch64 code.
@ This code below switches to AArch64 and starts execution at the specified
@ start address. It needs to be assembled by an ARM(32) assembler and
@ the machine code must be inserted as verbatim .word statements into the
@ beginning of the AArch64 U-Boot code.
@ To get the encoded bytes, use:
@ ${CROSS_COMPILE}gcc -c -o rmr_switch.o rmr_switch.S
@ ${CROSS_COMPILE}objdump -d rmr_switch.o
@
@ The resulting words should be inserted into the U-Boot file at
@ arch/arm/include/asm/arch-sunxi/boot0.h.
@
@ This file is not build by the U-Boot build system, but provided only as a
@ reference and to be able to regenerate a (probably fixed) version of this
@ code found in encoded form in boot0.h.

#include <config.h>

.text

#ifndef CONFIG_SUN50I_GEN_H6
	ldr	r1, =0x017000a0		@ MMIO mapped RVBAR[0] register
#else
	ldr	r1, =0x09010040		@ MMIO mapped RVBAR[0] register
#endif
	ldr	r0, =0x57aA7add		@ start address, to be replaced
	str	r0, [r1]
	dsb	sy
	isb	sy
	mrc	15, 0, r0, cr12, cr0, 2	@ read RMR register
	orr	r0, r0, #3		@ request reset in AArch64
	mcr	15, 0, r0, cr12, cr0, 2 @ write RMR register
	isb	sy
1:	wfi
	b	1b
