// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2001
 * Erik Theisen,  Wave 7 Optics, etheisen@mindspring.com.
 */

#include <linux/kconfig.h>

#ifndef __ASSEMBLY__
#define	__ASSEMBLY__			/* Dirty trick to get only #defines */
#endif
#define	__ASM_STUB_PROCESSOR_H__	/* don't include asm/processor. */
#include <config.h>
#undef	__ASSEMBLY__
#include <environment.h>
#include <linux/stringify.h>

/* Handle HOSTS that have prepended crap on symbol names, not TARGETS. */
#if defined(__APPLE__)
/* Leading underscore on symbols */
#  define SYM_CHAR "_"
#else /* No leading character on symbols */
#  define SYM_CHAR
#endif

/*
 * Generate embedded environment table
 * inside U-Boot image, if needed.
 */
#if defined(ENV_IS_EMBEDDED) || defined(CONFIG_BUILD_ENVCRC)
/*
 * Put the environment in the .text section when we are building
 * U-Boot proper.  The host based program "tools/envcrc" does not need
 * a seperate section.
 */
#if defined(USE_HOSTCC) /* Native for 'tools/envcrc' */
#  define __UBOOT_ENV_SECTION__(name)	/*XXX DO_NOT_DEL_THIS_COMMENT*/

#else /* Environment is embedded in U-Boot's .text section */
/* XXX - This only works with GNU C */
#  define __UBOOT_ENV_SECTION__(name)	__attribute__ ((section(".text."#name)))
#endif

/*
 * Macros to generate global absolutes.
 */
#if defined(__bfin__)
# define GEN_SET_VALUE(name, value)	\
	asm(".set " GEN_SYMNAME(name) ", " GEN_VALUE(value))
#else
# define GEN_SET_VALUE(name, value)	\
	asm(GEN_SYMNAME(name) " = " GEN_VALUE(value))
#endif
#define GEN_SYMNAME(str)	SYM_CHAR #str
#define GEN_VALUE(str)		#str
#define GEN_ABS(name, value)			\
	asm(".globl " GEN_SYMNAME(name));	\
	GEN_SET_VALUE(name, value)

/*
 * Check to see if we are building with a
 * computed CRC.  Otherwise define it as ~0.
 */
#if !defined(ENV_CRC)
#  define ENV_CRC	(~0)
#endif

#define DEFAULT_ENV_INSTANCE_EMBEDDED
#include <env_default.h>

#ifdef CONFIG_ENV_ADDR_REDUND
env_t redundand_environment __UBOOT_ENV_SECTION__(redundand_environment) = {
	0,		/* CRC Sum: invalid */
	0,		/* Flags:   invalid */
	{
	"\0"
	}
};
#endif	/* CONFIG_ENV_ADDR_REDUND */

/*
 * These will end up in the .text section
 * if the environment strings are embedded
 * in the image.  When this is used for
 * tools/envcrc, they are placed in the
 * .data/.sdata section.
 *
 */
unsigned long env_size __UBOOT_ENV_SECTION__(env_size) = sizeof(env_t);

/*
 * Add in absolutes.
 */
GEN_ABS(env_offset, CONFIG_ENV_OFFSET);

#endif /* ENV_IS_EMBEDDED */
