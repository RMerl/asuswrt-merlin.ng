/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright(C) 2019
 * Author(s): Giulio Benetti <giulio.benetti@benettiengineering.com>
 */

#ifndef __ASM_ARCH_IMX_REGS_H__
#define __ASM_ARCH_IMX_REGS_H__

#define ARCH_MXC

#define GPIO1_BASE_ADDR		0x401B8000
#define GPIO2_BASE_ADDR		0x401BC000
#define GPIO3_BASE_ADDR		0x401C0000
#define GPIO4_BASE_ADDR		0x401C4000
#define GPIO5_BASE_ADDR		0x400C0000

#define MXS_LCDIF_BASE		0x402b8000

#if !(defined(__KERNEL_STRICT_NAMES) || defined(__ASSEMBLY__))
#include <asm/mach-imx/regs-lcdif.h>
#endif

#define USB_BASE_ADDR		0x402E0000
#define USB_PHY0_BASE_ADDR	0x400D9000
#define USB_PHY1_BASE_ADDR	0x400DA000

#endif /* __ASM_ARCH_IMX_REGS_H__ */
