/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2019 NXP
 */

#if defined(CONFIG_SPL_BUILD)
	/*
	 * We use absolute address not PC relative address to jump.
	 * When running SPL on iMX8, the A core starts at address 0, a alias to OCRAM 0x100000,
	 * our linker address for SPL is from 0x100000. So using absolute address can jump to
	 * the OCRAM address from the alias.
	 * The alias only map first 96KB of OCRAM, so this require the SPL size can't beyond 96KB.
	 * But when using SPL DM, the size increase significantly and may exceed 96KB.
	 * That's why we have to jump to OCRAM.
	 */

	ldr	x0, =reset
	br	x0
#else
	b	reset
#endif
