/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2022 NXP
 * Copyright (C) 2023 PHYTEC Messtechnik GmbH
 * Christoph Stoidner <c.stoidner@phytec.de>
 * Copyright (C) 2024 Mathieu Othacehe <m.othacehe@gmail.com>
 */

#ifndef __PHYCORE_IMX93_H
#define __PHYCORE_IMX93_H

#include <linux/sizes.h>
#include <asm/arch/imx-regs.h>

#define CFG_SYS_UBOOT_BASE	\
	(QSPI0_AMBA_BASE + CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_SECTOR * 512)

#define CFG_SYS_INIT_RAM_ADDR        0x80000000
#define CFG_SYS_INIT_RAM_SIZE        0x200000

#define CFG_SYS_SDRAM_BASE           0x80000000
#define PHYS_SDRAM                   0x80000000
#define PHYS_SDRAM_SIZE              0x80000000

/* Using ULP WDOG for reset */
#define WDOG_BASE_ADDR          WDG3_BASE_ADDR

#endif /* __PHYCORE_IMX93_H */
