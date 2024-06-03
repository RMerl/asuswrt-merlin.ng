/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * include/configs/draak.h
 *     This file is Draak board configuration.
 *
 * Copyright (C) 2015 Renesas Electronics Corporation
 */

#ifndef __DRAAK_H
#define __DRAAK_H

#include "rcar-gen3-common.h"

/* Ethernet RAVB */
#define CONFIG_BITBANGMII
#define CONFIG_BITBANGMII_MULTI

/* Generic Timer Definitions (use in assembler source) */
#define COUNTER_FREQUENCY	0xFE502A	/* 16.66MHz from CPclk */

/* Environment in eMMC, at the end of 2nd "boot sector" */
#define CONFIG_ENV_OFFSET		(-CONFIG_ENV_SIZE)
#define CONFIG_SYS_MMC_ENV_DEV		0
#define CONFIG_SYS_MMC_ENV_PART		2

#define CONFIG_CFI_FLASH_USE_WEAK_ACCESSORS
#define CONFIG_FLASH_SHOW_PROGRESS	45
#define CONFIG_SYS_FLASH_BANKS_LIST	{ 0x08000000 }
#define CONFIG_SYS_FLASH_CFI_WIDTH	FLASH_CFI_16BIT
#define CONFIG_SYS_MAX_FLASH_BANKS_DETECT	1
#define CONFIG_SYS_MAX_FLASH_SECT	256
#define CONFIG_SYS_WRITE_SWAPPED_DATA

#endif /* __DRAAK_H */
