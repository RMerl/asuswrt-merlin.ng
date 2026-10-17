/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Architecture-specific SPL handoff information for ARM
 *
 * Copyright 2019 Amarula Solutions, BV
 * Written by Michael Trimarchi <michael@amarulasolutions.com>
 */

#ifndef __asm_handoff_h
#define __asm_handoff_h

/**
 * struct arch_spl_handoff - architecture-specific handoff info
 *
 * @usable_ram_top: Value returned by board_get_usable_ram_top() in SPL
 */
struct arch_spl_handoff {
	ulong usable_ram_top;
};

#endif
