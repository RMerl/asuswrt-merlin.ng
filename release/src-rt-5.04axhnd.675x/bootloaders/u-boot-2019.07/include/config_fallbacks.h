/*
 * Copyright 2012 Texas Instruments
 *
 * This file is licensed under the terms of the GNU General Public
 * License Version 2. This file is licensed "as is" without any
 * warranty of any kind, whether express or implied.
 */

#ifndef __CONFIG_FALLBACKS_H
#define __CONFIG_FALLBACKS_H

#ifdef CONFIG_SPL
#ifdef CONFIG_SPL_PAD_TO
#ifdef CONFIG_SPL_MAX_SIZE
#if CONFIG_SPL_PAD_TO && CONFIG_SPL_PAD_TO < CONFIG_SPL_MAX_SIZE
#error CONFIG_SPL_PAD_TO < CONFIG_SPL_MAX_SIZE
#endif
#endif
#else
#ifdef CONFIG_SPL_MAX_SIZE
#define CONFIG_SPL_PAD_TO	CONFIG_SPL_MAX_SIZE
#else
#define CONFIG_SPL_PAD_TO	0
#endif
#endif
#endif

#ifndef CONFIG_SYS_BAUDRATE_TABLE
#define CONFIG_SYS_BAUDRATE_TABLE	{ 9600, 19200, 38400, 57600, 115200 }
#endif

/* Console I/O Buffer Size */
#ifndef CONFIG_SYS_CBSIZE
#if defined(CONFIG_CMD_KGDB)
#define CONFIG_SYS_CBSIZE	1024
#else
#define CONFIG_SYS_CBSIZE	256
#endif
#endif

#ifndef CONFIG_SYS_PBSIZE
#define CONFIG_SYS_PBSIZE	(CONFIG_SYS_CBSIZE + sizeof(CONFIG_SYS_PROMPT) + 16)
#endif

#ifndef CONFIG_SYS_MAXARGS
#define CONFIG_SYS_MAXARGS	16
#endif

#ifdef CONFIG_DM_I2C
# ifdef CONFIG_SYS_I2C
#  error "Cannot define CONFIG_SYS_I2C when CONFIG_DM_I2C is used"
# endif
#endif

#endif	/* __CONFIG_FALLBACKS_H */
