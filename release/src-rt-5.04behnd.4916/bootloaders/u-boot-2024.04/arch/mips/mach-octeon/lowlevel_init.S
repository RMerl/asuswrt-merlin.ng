/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (C) 2020 Stefan Roese <sr@denx.de>
 */

#include <config.h>
#include <asm-offsets.h>
#include <asm/cacheops.h>
#include <asm/regdef.h>
#include <asm/mipsregs.h>
#include <asm/addrspace.h>
#include <asm/asm.h>
#include <mach/octeon-model.h>

#define COP0_CVMCTL_REG		$9,7	/* Cavium control */
#define COP0_CVMMEMCTL_REG	$11,7	/* Cavium memory control */
#define COP0_PROC_ID_REG	$15,0

	.set noreorder

LEAF(lowlevel_init)

	/* Set LMEMSZ in CVMMEMCTL register */
	dmfc0	a0, COP0_CVMMEMCTL_REG
	dins	a0, zero, 0, 9
	mfc0	a4, COP0_PROC_ID_REG
	li	a5, OCTEON_CN63XX_PASS1_0 /* Octeon cn63xx pass1 chip id */
	bgt	a5, a4, 2f
	 ori	 a0, 0x104	/* setup 4 lines of scratch */
	ori	a6, a5, 8	/* Octeon cn63xx pass2 chip id */
	bge	a4, a6, 2f
	 nop
	li	a6, 4
	ins	a0, a6, 11, 4	/* Set WBTHRESH=4 as per Core-14752 errata */
2:
	dmtc0	a0, COP0_CVMMEMCTL_REG

	/* Set REPUN bit in CVMCTL register */
	dmfc0	a0, COP0_CVMCTL_REG
	ori	a0, 1<<14	/* enable fixup of unaligned mem access */
	dmtc0	a0, COP0_CVMCTL_REG

	jr	ra
	 nop
	END(lowlevel_init)

LEAF(mips_mach_early_init)

	move    s0, ra

	bal	__dummy
	 nop

__dummy:
	/* Get the actual address that we are running at */
	PTR_LA	a7, __dummy
	dsubu	t3, ra, a7	/* t3 now has reloc offset */

	PTR_LA	t1, _start
	daddu	t0, t1, t3	/* t0 now has actual address of _start */

	/* Calculate end address of copy loop */
	PTR_LA	t2, _end
	daddiu	t2, t2, 0x4000	/* Increase size to include appended DTB */
	daddiu	t2, t2, 127
	ins	t2, zero, 0, 7	/* Round up to cache line for memcpy */

	/* Copy ourself to the L2 cache from flash, 32 bytes at a time */
1:
	ld	a0, 0(t0)
	ld	a1, 8(t0)
	ld	a2, 16(t0)
	ld	a3, 24(t0)
	sd	a0, 0(t1)
	sd	a1, 8(t1)
	sd	a2, 16(t1)
	sd	a3, 24(t1)
	addiu	t0, 32
	addiu	t1, 32
	bne	t1, t2, 1b
	 nop

	sync

	/*
	 * Return to start.S now running from TEXT_BASE, which points
	 * to DRAM address space, which effectively is L2 cache now.
	 * This speeds up the init process extremely, especially the
	 * DDR init code.
	 */
	dsubu	s0, s0, t3	/* Fixup return address with reloc offset */
	jr.hb	s0		/* Jump back with hazard barrier */
	 nop

	END(mips_mach_early_init)

LEAF(nmi_bootvector)

	/*
	 * From Marvell original bootvector setup
	 */
	mfc0	k0, CP0_STATUS
	/* Enable 64-bit addressing, set ERL (should already be set) */
	ori	k0, 0x84
	mtc0	k0, CP0_STATUS
	/* Core-14345, clear L1 Dcache virtual tags if the core hit an NMI */
	cache	17, 0($0)

	/*
	 * Needed for Linux kernel booting, otherwise it hangs while
	 * zero'ing all of CVMSEG
	 */
	dmfc0	a0, COP0_CVMMEMCTL_REG
	dins	a0, zero, 0, 9
	ori	a0, 0x104	/* setup 4 lines of scratch */
	dmtc0	a0, COP0_CVMMEMCTL_REG

	/*
	 * Load parameters and entry point
	 */
	PTR_LA	t9, nmi_handler_para
	sync

	ld	s0, 0x00(t9)
	ld	a0, 0x08(t9)
	ld	a1, 0x10(t9)
	ld	a2, 0x18(t9)
	ld	a3, 0x20(t9)

	/* Finally jump to entry point (start kernel etc) */
	j	s0
	 nop

	END(nmi_bootvector)

	/*
	 * Add here some space for the NMI parameters (entry point and args)
	 */
	.globl nmi_handler_para
nmi_handler_para:
	.dword	0	// entry-point
	.dword	0	// arg0
	.dword	0	// arg1
	.dword	0	// arg2
	.dword	0	// arg3
