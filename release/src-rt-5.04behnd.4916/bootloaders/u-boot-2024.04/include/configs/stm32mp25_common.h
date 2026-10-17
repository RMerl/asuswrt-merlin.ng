/* SPDX-License-Identifier: GPL-2.0-or-later OR BSD-3-Clause */
/*
 * Copyright (C) 2023, STMicroelectronics - All Rights Reserved
 *
 * Configuration settings for the STM32MP25x CPU
 */

#ifndef __CONFIG_STM32MP25_COMMMON_H
#define __CONFIG_STM32MP25_COMMMON_H
#include <linux/sizes.h>
#include <asm/arch/stm32.h>

/*
 * Configuration of the external SRAM memory used by U-Boot
 */
#define CFG_SYS_SDRAM_BASE	STM32_DDR_BASE

/*
 * For booting Linux, use the first 256 MB of memory, since this is
 * the maximum mapped by the Linux kernel during initialization.
 */
#define CFG_SYS_BOOTMAPSZ	SZ_256M

#endif /* __CONFIG_STM32MP25_COMMMON_H */
