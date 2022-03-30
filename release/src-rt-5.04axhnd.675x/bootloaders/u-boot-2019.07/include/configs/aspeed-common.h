/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2012-2020  ASPEED Technology Inc.
 * Ryan Chen <ryan_chen@aspeedtech.com>
 *
 * Copyright 2016 IBM Corporation
 * (C) Copyright 2016 Google, Inc
 */

#ifndef __AST_COMMON_CONFIG_H
#define __AST_COMMON_CONFIG_H

/* Misc CPU related */
#define CONFIG_CMDLINE_TAG
#define CONFIG_SETUP_MEMORY_TAGS
#define CONFIG_INITRD_TAG

#define CONFIG_SYS_SDRAM_BASE		0x80000000

#ifdef CONFIG_PRE_CON_BUF_SZ
#define CONFIG_SYS_INIT_RAM_ADDR	(0x1e720000 + CONFIG_PRE_CON_BUF_SZ)
#define CONFIG_SYS_INIT_RAM_SIZE	(36*1024 - CONFIG_PRE_CON_BUF_SZ)
#else
#define CONFIG_SYS_INIT_RAM_ADDR	(0x1e720000)
#define CONFIG_SYS_INIT_RAM_SIZE	(36*1024)
#endif

#define SYS_INIT_RAM_END		(CONFIG_SYS_INIT_RAM_ADDR \
					 + CONFIG_SYS_INIT_RAM_SIZE)
#define CONFIG_SYS_INIT_SP_ADDR		(SYS_INIT_RAM_END \
					 - GENERATED_GBL_DATA_SIZE)

#define CONFIG_SYS_MALLOC_LEN		(32 << 20)

/*
 * NS16550 Configuration
 */

/*
 * BOOTP options
 */
#define CONFIG_BOOTP_BOOTFILESIZE

/*
 * Miscellaneous configurable options
 */

#define CONFIG_BOOTCOMMAND		"bootm 20080000 20300000"
#define CONFIG_ENV_OVERWRITE

#define CONFIG_EXTRA_ENV_SETTINGS \
	"verify=yes\0"	\
	"spi_dma=yes\0" \
	""

#endif	/* __AST_COMMON_CONFIG_H */
