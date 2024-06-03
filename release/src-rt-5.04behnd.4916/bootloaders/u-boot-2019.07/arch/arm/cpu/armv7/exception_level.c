// SPDX-License-Identifier: GPL-2.0+
/*
 * Switch to non-secure mode
 *
 * Copyright (c) 2018 Heinrich Schuchardt
 *
 * This module contains the ARMv7 specific code required for leaving the
 * secure mode before booting an operating system.
 */

#include <common.h>
#include <bootm.h>
#include <asm/armv7.h>
#include <asm/secure.h>
#include <asm/setjmp.h>

/**
 * entry_non_secure() - entry point when switching to non-secure mode
 *
 * When switching to non-secure mode switch_to_non_secure_mode() calls this
 * function passing a jump buffer. We use this jump buffer to restore the
 * original stack and register state.
 *
 * @non_secure_jmp:	jump buffer for restoring stack and registers
 */
static void entry_non_secure(struct jmp_buf_data *non_secure_jmp)
{
	dcache_enable();
	debug("Reached non-secure mode\n");

	/* Restore stack and registers saved in switch_to_non_secure_mode() */
	longjmp(non_secure_jmp, 1);
}

/**
 * switch_to_non_secure_mode() - switch to non-secure mode
 *
 * Operating systems may expect to run in non-secure mode. Here we check if
 * we are running in secure mode and switch to non-secure mode if necessary.
 */
void switch_to_non_secure_mode(void)
{
	static bool is_nonsec;
	struct jmp_buf_data non_secure_jmp;

	if (armv7_boot_nonsec() && !is_nonsec) {
		if (setjmp(&non_secure_jmp))
			return;
		dcache_disable();	/* flush cache before switch to HYP */
		armv7_init_nonsec();
		is_nonsec = true;
		secure_ram_addr(_do_nonsec_entry)(entry_non_secure,
						  (uintptr_t)&non_secure_jmp,
						  0, 0);
	}
}
