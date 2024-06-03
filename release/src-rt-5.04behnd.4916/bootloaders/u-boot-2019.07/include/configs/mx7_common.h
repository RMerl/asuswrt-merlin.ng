/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2015 Freescale Semiconductor, Inc.
 *
 * Configuration settings for the Freescale i.MX7.
 */

#ifndef __MX7_COMMON_H
#define __MX7_COMMON_H

#include <linux/sizes.h>
#include <asm/arch/imx-regs.h>
#include <asm/mach-imx/gpio.h>

#ifndef CONFIG_MX7
#define CONFIG_MX7
#endif

/* Timer settings */
#define CONFIG_MXC_GPT_HCLK
#define CONFIG_SC_TIMER_CLK 8000000 /* 8Mhz */
#define COUNTER_FREQUENCY CONFIG_SC_TIMER_CLK
#define CONFIG_SYS_FSL_CLK

#define CONFIG_SYS_BOOTM_LEN	0x1000000

/* Enable iomux-lpsr support */
#define CONFIG_IOMUX_LPSR

#define CONFIG_LOADADDR                 0x80800000

/* allow to overwrite serial and ethaddr */
#define CONFIG_ENV_OVERWRITE

/* Miscellaneous configurable options */
#define CONFIG_SYS_CBSIZE		512
#define CONFIG_SYS_MAXARGS		32

/* UART */
#define CONFIG_MXC_UART

/* MMC */
#define CONFIG_FSL_USDHC

#define CONFIG_ARMV7_SECURE_BASE	0x00900000

#define CONFIG_ARMV7_PSCI_1_0

/* Secure boot (HAB) support */
#ifdef CONFIG_SECURE_BOOT
#define CONFIG_CSF_SIZE			0x4000
#ifdef CONFIG_SPL_BUILD
#define CONFIG_SPL_DRIVERS_MISC_SUPPORT
#endif
#endif

/*
 * If we have defined the OPTEE ram size and not OPTEE it means that we were
 * launched by OPTEE, because of that we shall skip all the low level
 * initialization since it was already done by ATF or OPTEE
 */
#if (CONFIG_OPTEE_TZDRAM_SIZE != 0)
#ifndef CONFIG_OPTEE
#define CONFIG_SKIP_LOWLEVEL_INIT
#endif
#endif

#endif
