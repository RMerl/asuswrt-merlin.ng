/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2017-2019 NXP
 * Copyright 2019-2023 Kococonnector GmbH
 */

#ifndef __IMX8QM_DMSSE20_H
#define __IMX8QM_DMSSE20_H

#include <linux/sizes.h>
#include <asm/arch/imx-regs.h>

/* Flat Device Tree Definitions */

#define CFG_SYS_FSL_ESDHC_ADDR		0
#define USDHC1_BASE_ADDR		0x5B010000
#define USDHC2_BASE_ADDR		0x5B020000
#define USDHC3_BASE_ADDR		0x5B030000

#define FEC_QUIRK_ENET_MAC

#define IMX_FEC_BASE			0x5B040000
/* FEC1 */
#define IMX_FEC1_BASE			0x5B040000
/* FEC2 */
#define IMX_FEC2_BASE			0x5B050000

#ifdef CONFIG_NAND_BOOT
#define MFG_NAND_PARTITION "mtdparts=gpmi-nand:128m(boot),32m(kernel),16m(dtb),8m(misc),-(rootfs) "
#else
#define MFG_NAND_PARTITION ""
#endif

/* Incorporate settings into the U-Boot environment */
#define CFG_EXTRA_ENV_SETTINGS

#define CFG_SYS_FSL_USDHC_NUM		2

#define CFG_SYS_SDRAM_BASE		0x080000000
#define PHYS_SDRAM_1			0x080000000
#define PHYS_SDRAM_2			0x880000000
#define PHYS_SDRAM_1_SIZE		0x080000000	/* 2 GB */
#define PHYS_SDRAM_2_SIZE		0x180000000	/* 6 GB */

/* Generic Timer Definitions */
#define COUNTER_FREQUENCY		8000000	/* 8MHz */

#endif /* __IMX8QM_DMSSE20_H */
