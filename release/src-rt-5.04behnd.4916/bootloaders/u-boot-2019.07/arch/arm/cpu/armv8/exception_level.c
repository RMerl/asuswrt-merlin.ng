// SPDX-License-Identifier: GPL-2.0+
/*
 * Switch to non-secure mode
 *
 * Copyright (c) 2018 Heinrich Schuchardt
 *
 * This module contains the ARMv8 specific code required to adjust the exception
 * level before booting an operating system.
 */

#include <common.h>
#include <bootm.h>
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
 * Exception level EL3 is meant to be used by the secure monitor only (ARM
 * trusted firmware being one embodiment). The operating system shall be
 * started at exception level EL2. So here we check the exception level
 * and switch it if necessary.
 */
void switch_to_non_secure_mode(void)
{
	struct jmp_buf_data non_secure_jmp;

	/* On AArch64 we need to make sure we call our payload in < EL3 */
	if (current_el() == 3) {
		if (setjmp(&non_secure_jmp))
			return;
		dcache_disable();	/* flush cache before switch to EL2 */

		/* Move into EL2 and keep running there */
		armv8_switch_to_el2((uintptr_t)&non_secure_jmp, 0, 0, 0,
				    (uintptr_t)entry_non_secure, ES_TO_AARCH64);
	}
}
