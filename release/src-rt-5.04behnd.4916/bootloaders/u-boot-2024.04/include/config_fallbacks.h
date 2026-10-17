/*
 * Copyright 2012 Texas Instruments
 *
 * This file is licensed under the terms of the GNU General Public
 * License Version 2. This file is licensed "as is" without any
 * warranty of any kind, whether express or implied.
 */

#ifndef __CONFIG_FALLBACKS_H
#define __CONFIG_FALLBACKS_H

#ifdef CONFIG_SPL_PAD_TO
#ifdef CONFIG_SPL_MAX_SIZE
#if CONFIG_SPL_PAD_TO && CONFIG_SPL_PAD_TO < CONFIG_SPL_MAX_SIZE
#error CONFIG_SPL_PAD_TO < CONFIG_SPL_MAX_SIZE
#endif
#endif
#endif

#ifndef CFG_SYS_BAUDRATE_TABLE
#define CFG_SYS_BAUDRATE_TABLE	{ 9600, 19200, 38400, 57600, 115200 }
#endif

#endif	/* __CONFIG_FALLBACKS_H */
