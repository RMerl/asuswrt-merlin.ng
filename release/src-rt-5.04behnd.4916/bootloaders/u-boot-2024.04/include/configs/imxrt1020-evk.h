/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2020
 * Author(s): Giulio Benetti <giulio.benetti@benettiengineering.com>
 */

#ifndef __IMXRT1020_EVK_H
#define __IMXRT1020_EVK_H

#include <asm/arch/imx-regs.h>

#define ESDHCI_QUIRK_BROKEN_TIMEOUT_VALUE	1

#define PHYS_SDRAM			0x80000000
#define PHYS_SDRAM_SIZE			(32 * 1024 * 1024)

#define DMAMEM_SZ_ALL			(1 * 1024 * 1024)
#define DMAMEM_BASE			(PHYS_SDRAM + PHYS_SDRAM_SIZE - \
					 DMAMEM_SZ_ALL)

#endif /* __IMXRT1020_EVK_H */
