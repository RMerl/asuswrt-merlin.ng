/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * K3: AM6 SoC definitions, structures etc.
 *
 * (C) Copyright (C) 2018 Texas Instruments Incorporated - http://www.ti.com/
 */
#ifndef __ASM_ARCH_AM6_HARDWARE_H
#define __ASM_ARCH_AM6_HARDWARE_H

#include <config.h>

#define CTRL_MMR0_BASE					0x00100000
#define CTRLMMR_MAIN_DEVSTAT				(CTRL_MMR0_BASE + 0x30)

#define CTRLMMR_MAIN_DEVSTAT_BOOTMODE_MASK		GENMASK(3, 0)
#define CTRLMMR_MAIN_DEVSTAT_BOOTMODE_SHIFT		0
#define CTRLMMR_MAIN_DEVSTAT_BKUP_BOOTMODE_MASK		GENMASK(6, 4)
#define CTRLMMR_MAIN_DEVSTAT_BKUP_BOOTMODE_SHIFT	4
#define CTRLMMR_MAIN_DEVSTAT_MMC_PORT_MASK		GENMASK(12, 12)
#define CTRLMMR_MAIN_DEVSTAT_MMC_PORT_SHIFT		12
#define CTRLMMR_MAIN_DEVSTAT_EMMC_PORT_MASK		GENMASK(14, 14)
#define CTRLMMR_MAIN_DEVSTAT_EMMC_PORT_SHIFT		14
#define CTRLMMR_MAIN_DEVSTAT_BKUP_MMC_PORT_MASK		GENMASK(17, 17)
#define CTRLMMR_MAIN_DEVSTAT_BKUP_MMC_PORT_SHIFT	12

#define WKUP_CTRL_MMR0_BASE				0x43000000
#define MCU_CTRL_MMR0_BASE				0x40f00000

/*
 * The CTRL_MMR0 memory space is divided into several equally-spaced
 * partitions, so defining the partition size allows us to determine
 * register addresses common to those partitions.
 */
#define CTRL_MMR0_PARTITION_SIZE			0x4000

/*
 * CTRL_MMR0, WKUP_CTRL_MMR0, and MCU_CTR_MMR0 lock/kick-mechanism
 * shared register definitions.
 */
#define CTRLMMR_LOCK_KICK0				0x01008
#define CTRLMMR_LOCK_KICK0_UNLOCK_VAL			0x68ef3490
#define CTRLMMR_LOCK_KICK0_UNLOCKED_MASK		BIT(0)
#define CTRLMMR_LOCK_KICK0_UNLOCKED_SHIFT		0
#define CTRLMMR_LOCK_KICK1				0x0100c
#define CTRLMMR_LOCK_KICK1_UNLOCK_VAL			0xd172bc5a

#endif /* __ASM_ARCH_AM6_HARDWARE_H */
