/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2018 Synopsys, Inc. All rights reserved.
 */

#ifndef _CONFIG_IOT_DEVKIT_H_
#define _CONFIG_IOT_DEVKIT_H_

#include <linux/sizes.h>

/*
 *                         MEMORY MAP
 *
 * eFlash: 0x0000_0000 - 0x0008_0000 (512K)
 *   ICCM: 0x2000_0000 - 0x2004_0000 (256K)
 *   SRAM: 0x3000_0000 - 0x3002_0000 (128K)
 *   DCCM: 0x8000_0000 - 0x8002_0000 (128K)
 *     Note: only data goes here, as IFQ cannot fetch instructions from DCCM
 *
 *
 *                         RAM PARTITIONING
 *
 *   +-----------+----------+---------------------+-------------+
 *   | <-- Stack |  .data   | Malloc              | Environment |
 *   +-----------+----------+---------------------+-------------+
 *   :           :          :                     :\___________/
 *   :           :          :                     :      |
 *   :           :          :                     :     CONFIG_ENV_SIZE
 *   :           :           \____________________/
 *   :           :                     |
 *   :           :                    CONFIG_SYS_MALLOC_LEN
 *   :           :
 *   :          Specified explicitly by CONFIG_SYS_INIT_SP_ADDR
 *   :
 *  Specified explicitly by CONFIG_SYS_SDRAM_BASE
 *
 *  NOTES:
 *    - Stack starts from CONFIG_SYS_INIT_SP_ADDR and grows down,
 *      i.e. towards CONFIG_SYS_SDRAM_BASE but nothing stops it from crossing
 *      that CONFIG_SYS_SDRAM_BASE in which case data won't be really saved on
 *      stack any longer and values popped from stack will contain garbage
 *      leading to unexpected behavior, typically but not limited to:
 *        - "Returning" back to bogus caller function
 *        - Reading data from weird addresses
 */

#define CONFIG_SYS_MONITOR_BASE		CONFIG_SYS_TEXT_BASE

#define SRAM_BASE			0x30000000
#define SRAM_SIZE			SZ_128K

#define DCCM_BASE			0x80000000
#define DCCM_SIZE			SZ_128K

#define CONFIG_SYS_SDRAM_BASE		DCCM_BASE
#define CONFIG_SYS_SDRAM_SIZE		DCCM_SIZE

#define CONFIG_SYS_INIT_SP_ADDR		(CONFIG_SYS_SDRAM_BASE + SZ_32K)

#define CONFIG_SYS_MALLOC_LEN		SZ_64K
#define CONFIG_SYS_BOOTM_LEN		SZ_128K
#define CONFIG_SYS_LOAD_ADDR		SRAM_BASE

#define ROM_BASE			CONFIG_SYS_MONITOR_BASE
#define ROM_SIZE			SZ_256K

#define RAM_DATA_BASE			CONFIG_SYS_INIT_SP_ADDR
#define RAM_DATA_SIZE			CONFIG_SYS_SDRAM_SIZE - \
					(CONFIG_SYS_INIT_SP_ADDR - \
					CONFIG_SYS_SDRAM_BASE) - \
					CONFIG_SYS_MALLOC_LEN - \
					CONFIG_ENV_SIZE

/*
 * Environment
 */
#define CONFIG_BOOTFILE			"app.bin"
#define CONFIG_LOADADDR			CONFIG_SYS_LOAD_ADDR

#endif /* _CONFIG_IOT_DEVKIT_H_ */
