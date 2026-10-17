/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2019
 * Author(s): Giulio Benetti <giulio.benetti@benettiengineering.com>
 */

#ifndef __IMXRT1050_EVK_H
#define __IMXRT1050_EVK_H

#include <asm/arch/imx-regs.h>

#define ESDHCI_QUIRK_BROKEN_TIMEOUT_VALUE	1

#define PHYS_SDRAM			0x80000000
#define PHYS_SDRAM_SIZE			(32 * 1024 * 1024)

#define DMAMEM_SZ_ALL			(1 * 1024 * 1024)
#define DMAMEM_BASE			(PHYS_SDRAM + PHYS_SDRAM_SIZE - \
					 DMAMEM_SZ_ALL)

#ifdef CONFIG_VIDEO
#define CFG_EXTRA_ENV_SETTINGS \
		"stdin=serial\0" \
		"stdout=serial,vidconsole\0" \
		"stderr=serial,vidconsole\0"
#endif

/*
 * Address of U-Boot for SPI NOR boot
 */

#define CFG_SYS_UBOOT_BASE			0x60010000

#endif /* __IMXRT1050_EVK_H */
