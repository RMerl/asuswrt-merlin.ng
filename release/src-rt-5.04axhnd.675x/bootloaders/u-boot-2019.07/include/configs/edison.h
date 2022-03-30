/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (c) 2017 Intel Corp.
 */

#ifndef __CONFIG_H
#define __CONFIG_H

#include <asm/ibmpc.h>

/* Miscellaneous configurable options */

#define CONFIG_SYS_CBSIZE	2048
#define CONFIG_SYS_MAXARGS	128
#define CONFIG_SYS_BARGSIZE	CONFIG_SYS_CBSIZE

/* Memory */
#define CONFIG_SYS_LOAD_ADDR			0x100000
#define CONFIG_PHYSMEM

#define CONFIG_SYS_STACK_SIZE			(32 * 1024)

#define CONFIG_SYS_MONITOR_BASE			CONFIG_SYS_TEXT_BASE
#define CONFIG_SYS_MONITOR_LEN			(256 * 1024)

#define CONFIG_SYS_MALLOC_LEN			(128 * 1024 * 1024)

#define CONFIG_SYS_MEMTEST_START		0x00100000
#define CONFIG_SYS_MEMTEST_END			0x01000000

/* Environment */
#define CONFIG_SYS_MMC_ENV_DEV			0
#define CONFIG_SYS_MMC_ENV_PART			0
#define CONFIG_ENV_SIZE				(64 * 1024)
#define CONFIG_ENV_OFFSET			(3 * 1024 * 1024)
#define CONFIG_ENV_OFFSET_REDUND		(6 * 1024 * 1024)

/* RTC */
#define CONFIG_SYS_ISA_IO_BASE_ADDRESS	0

#endif
