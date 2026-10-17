/* SPDX-License-Identifier: GPL-2.0+
 *
 * (C) Copyright 2016 Nexell
 * Youngbok, Park <ybpark@nexell.co.kr>
 */

#ifndef __NEXELL_RESET__
#define __NEXELL_RESET__

#define NUMBER_OF_RESET_MODULE_PIN      69

enum rstcon {
	RSTCON_ASSERT	= 0UL,
	RSTCON_NEGATE	= 1UL
};

void nx_rstcon_setrst(u32 rstindex, enum rstcon status);

#endif /* __NEXELL_RESET__ */
