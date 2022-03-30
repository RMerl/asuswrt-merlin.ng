// SPDX-License-Identifier: GPL-2.0+
/*
 * Utility functions needed for (some) EABI conformant tool chains.
 *
 * (C) Copyright 2009 Wolfgang Denk <wd@denx.de>
 */

#include <common.h>

int raise (int signum)
{
	/* Even if printf() is available, it's large. Punt it for SPL builds */
#if !defined(CONFIG_SPL_BUILD)
	printf("raise: Signal # %d caught\n", signum);
#endif
	return 0;
}

/* Dummy function to avoid linker complaints */
void __aeabi_unwind_cpp_pr0(void)
{
}

void __aeabi_unwind_cpp_pr1(void)
{
}

/* Copy memory like memcpy, but no return value required.  */
void __aeabi_memcpy(void *dest, const void *src, size_t n)
{
	(void) memcpy(dest, src, n);
}

void __aeabi_memset(void *dest, size_t n, int c)
{
	(void) memset(dest, c, n);
}
