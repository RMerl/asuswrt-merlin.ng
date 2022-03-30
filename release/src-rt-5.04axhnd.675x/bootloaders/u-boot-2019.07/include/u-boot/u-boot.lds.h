/*
 * Linker script helper macros
 *
 * Copyright (c) 2009 Analog Devices Inc.
 *
 * Licensed under the GPL-2 or later.
 */

#ifndef __U_BOOT_LDS__
#define __U_BOOT_LDS__

/* See if the linker version is at least the specified version */
#define LD_AT_LEAST(major, minor) \
	((major > LD_MAJOR) || (major == LD_MAJOR && minor <= LD_MINOR))

/*
 * Linker versions prior to 2.16 don't understand the builtin
 * functions SORT_BY_ALIGNMENT() and SORT_BY_NAME(), so disable these
 */
#if !LD_AT_LEAST(2, 16)
# define SORT_BY_ALIGNMENT(x) x
# define SORT_BY_NAME(x) x
#endif

#endif
