/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2017 Theobroma Systems Design und Consulting GmbH
 */

#ifndef __PUMA_RK3399_H
#define __PUMA_RK3399_H

#include <configs/rk3399_common.h>

#if defined(CONFIG_ENV_IS_IN_MMC)
#define CONFIG_SYS_MMC_ENV_DEV 1
#elif defined(CONFIG_ENV_IS_IN_SPI_FLASH)
#define CONFIG_ENV_SECT_SIZE		(8 * 1024)
#endif

#define SDRAM_BANK_SIZE			(2UL << 30)

#define CONFIG_SERIAL_TAG
#define CONFIG_ENV_OVERWRITE

#define CONFIG_BMP_16BPP
#define CONFIG_BMP_24BPP
#define CONFIG_BMP_32BPP

#endif
