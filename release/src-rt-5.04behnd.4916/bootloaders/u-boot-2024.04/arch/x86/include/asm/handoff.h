/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Architecture-specific SPL handoff information for x86
 *
 * Copyright 2018 Google, Inc
 * Written by Simon Glass <sjg@chromium.org>
 */

#ifndef __x86_asm_handoff_h
#define __x86_asm_handoff_h

/**
 * struct arch_spl_handoff - architecture-specific handoff info
 *
 * @usable_ram_top: Value returned by board_get_usable_ram_top() in SPL
 * @hob_list: Start of FSP hand-off blocks (HOBs)
 */
struct arch_spl_handoff {
	ulong usable_ram_top;
	void *hob_list;
};

#endif
