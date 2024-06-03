/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2014 Broadcom Corporation.
 */

#ifndef __BCM_EP_BOARD_H
#define __BCM_EP_BOARD_H

#include <asm/arch/configs.h>

#define CONFIG_SKIP_LOWLEVEL_INIT

/*
 * Memory configuration
 * (these must be defined elsewhere)
 */
#ifndef CONFIG_SYS_TEXT_BASE
#error	CONFIG_SYS_TEXT_BASE must be defined!
#endif
#ifndef CONFIG_SYS_SDRAM_BASE
#error	CONFIG_SYS_SDRAM_BASE must be defined!
#endif
#ifndef CONFIG_SYS_SDRAM_SIZE
#error	CONFIG_SYS_SDRAM_SIZE must be defined!
#endif

#define CONFIG_SYS_MALLOC_LEN		(4 * 1024 * 1024)

/* Some commands use this as the default load address */
#define CONFIG_SYS_LOAD_ADDR		CONFIG_SYS_SDRAM_BASE

/*
 * This is the initial SP which is used only briefly for relocating the u-boot
 * image to the top of SDRAM. After relocation u-boot moves the stack to the
 * proper place.
 */
#define CONFIG_SYS_INIT_SP_ADDR		CONFIG_SYS_TEXT_BASE

/* allow to overwrite serial and ethaddr */
#define CONFIG_ENV_OVERWRITE

/* Serial Info */
#define CONFIG_SYS_NS16550_SERIAL

#define CONFIG_ENV_SIZE			0x2000

/* console configuration */
#define CONFIG_SYS_CBSIZE		1024	/* Console buffer size */
#define CONFIG_SYS_MAXARGS		64
#define CONFIG_SYS_BARGSIZE		CONFIG_SYS_CBSIZE

/* version string, parser, etc */

#define CONFIG_MX_CYCLIC

/* Enable Time Command */

#endif /* __BCM_EP_BOARD_H */
