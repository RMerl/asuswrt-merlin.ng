/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2022
 * Author(s): Jesse Taube <Mr.Bossman075@gmail.com>
 * Giulio Benetti <giulio.benetti@benettiengineering.com>
 */

#ifndef __IMXRT1170_EVK_H
#define __IMXRT1170_EVK_H

#include <asm/arch/imx-regs.h>

#define ESDHCI_QUIRK_BROKEN_TIMEOUT_VALUE	1

/*
 * Configuration of the external SDRAM memory
 */

#define PHYS_SDRAM			0x80000000
#define PHYS_SDRAM_SIZE			(64 * 1024 * 1024)

#define DMAMEM_SZ_ALL			(1 * 1024 * 1024)
#define DMAMEM_BASE			(PHYS_SDRAM + PHYS_SDRAM_SIZE - \
					 DMAMEM_SZ_ALL)

#endif /* __IMXRT1170_EVK_H */
