/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2018  Cisco Systems, Inc.
 *
 * Author: Thomas Fitzsimmons <fitzsim@fitzsim.org>
 *
 * Configuration settings for the Broadcom BCM7260 SoC family.
 */

#ifndef __CONFIG_H
#define __CONFIG_H

#define CONFIG_SYS_NS16550_COM1	0xf040c000

#define CONFIG_SYS_TEXT_BASE		0x10100000
#define CONFIG_SYS_INIT_RAM_ADDR	0x10200000

#define CONFIG_SYS_MALLOC_LEN		((40 * 1024) << 10) /* 40 MiB */

#include "bcmstb.h"

#define BCMSTB_TIMER_LOW	0xf0412008
#define BCMSTB_TIMER_HIGH	0xf041200c
#define BCMSTB_TIMER_FREQUENCY	0xf0412020
#define BCMSTB_HIF_MSPI_BASE	0xf0203c00
#define BCMSTB_BSPI_BASE	0xf0203a00
#define BCMSTB_HIF_SPI_INTR2	0xf0201a00
#define BCMSTB_CS_REG		0xf0200920

/*
 * Environment configuration for eMMC.
 */
#define CONFIG_ENV_OFFSET	(0x000040a4 * 512)
#define CONFIG_SYS_MMC_ENV_DEV	0
#define CONFIG_SYS_MMC_ENV_PART	0

#define CONFIG_CMD_GPT

#endif	/* __CONFIG_H */
