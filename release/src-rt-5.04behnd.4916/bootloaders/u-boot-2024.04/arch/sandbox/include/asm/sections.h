/*
 * decls for symbols defined in the linker script
 *
 * Copyright (c) 2012 The Chromium OS Authors.
 *
 * Licensed under the GPL-2 or later.
 */

#ifndef __SANDBOX_SECTIONS_H
#define __SANDBOX_SECTIONS_H

#include <asm-generic/sections.h>

struct sandbox_cmdline_option;

static inline struct sandbox_cmdline_option **
__u_boot_sandbox_option_start(void)
{
	static char start[0] __aligned(4) __attribute__((unused))
		__section("_u_boot_sandbox_getopt_start");

	return (struct sandbox_cmdline_option **)&start;
}

static inline struct sandbox_cmdline_option **
__u_boot_sandbox_option_end(void)
{
	static char end[0] __aligned(4) __attribute__((unused))
		__section("_u_boot_sandbox_getopt_end");

	return (struct sandbox_cmdline_option **)&end;
}

static inline size_t __u_boot_sandbox_option_count(void)
{
	return __u_boot_sandbox_option_end() - __u_boot_sandbox_option_start();
}

#endif
