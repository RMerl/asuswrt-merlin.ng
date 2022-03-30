/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * include/configs/ebisu.h
 *     This file is Ebisu board configuration.
 *
 * Copyright (C) 2018 Renesas Electronics Corporation
 */

#ifndef __EBISU_H
#define __EBISU_H

#undef DEBUG

#include "rcar-gen3-common.h"

/* Ethernet RAVB */
#define CONFIG_NET_MULTI
#define CONFIG_BITBANGMII
#define CONFIG_BITBANGMII_MULTI

/* Generic Timer Definitions (use in assembler source) */
#define COUNTER_FREQUENCY	0xFE502A	/* 16.66MHz from CPclk */

/* Environment in eMMC, at the end of 2nd "boot sector" */
#define CONFIG_ENV_OFFSET		(-CONFIG_ENV_SIZE)
#define CONFIG_SYS_MMC_ENV_DEV		2
#define CONFIG_SYS_MMC_ENV_PART		2

#endif /* __EBISU_H */
