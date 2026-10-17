// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2020, Heinrich Schuchardt <xypron.glpk@gmx.de>
 */

#include <cpu_func.h>
#include <asm/state.h>

void flush_cache(unsigned long addr, unsigned long size)
{
	/* Clang uses (char *) parameters, GCC (void *) */
	__builtin___clear_cache((void *)addr, (void *)(addr + size));
}

void invalidate_icache_all(void)
{
	struct sandbox_state *state = state_get_current();

	/* Clang uses (char *) parameters, GCC (void *) */
	__builtin___clear_cache((void *)state->ram_buf,
				(void *)(state->ram_buf + state->ram_size));
}
