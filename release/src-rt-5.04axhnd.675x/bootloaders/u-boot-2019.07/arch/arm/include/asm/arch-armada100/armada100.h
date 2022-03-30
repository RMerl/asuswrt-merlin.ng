/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2010
 * Marvell Semiconductor <www.marvell.com>
 * Written-by: Prafulla Wadaskar <prafulla@marvell.com>
 * Contributor: Mahavir Jain <mjain@marvell.com>
 */

#ifndef _ASM_ARCH_ARMADA100_H
#define _ASM_ARCH_ARMADA100_H

#if defined (CONFIG_ARMADA100)

/* Common APB clock register bit definitions */
#define APBC_APBCLK     (1<<0)  /* APB Bus Clock Enable */
#define APBC_FNCLK      (1<<1)  /* Functional Clock Enable */
#define APBC_RST        (1<<2)  /* Reset Generation */
/* Functional Clock Selection Mask */
#define APBC_FNCLKSEL(x)        (((x) & 0xf) << 4)

/* Fast Ethernet Controller Clock register definition */
#define FE_CLK_RST		0x1
#define FE_CLK_ENA		0x8

/* SSP2 Clock Control */
#define SSP2_APBCLK		0x01
#define SSP2_FNCLK		0x02

/* USB Clock/reset control bits */
#define USB_SPH_AXICLK_EN	0x10
#define USB_SPH_AXI_RST		0x02

/* MPMU Clocks */
#define APB2_26M_EN		(1 << 20)
#define AP_26M			(1 << 4)

/* Register Base Addresses */
#define ARMD1_DRAM_BASE		0xB0000000
#define ARMD1_FEC_BASE		0xC0800000
#define ARMD1_TIMER_BASE	0xD4014000
#define ARMD1_APBC1_BASE	0xD4015000
#define ARMD1_APBC2_BASE	0xD4015800
#define ARMD1_UART1_BASE	0xD4017000
#define ARMD1_UART2_BASE	0xD4018000
#define ARMD1_GPIO_BASE		0xD4019000
#define ARMD1_SSP1_BASE		0xD401B000
#define ARMD1_SSP2_BASE		0xD401C000
#define ARMD1_MFPR_BASE		0xD401E000
#define ARMD1_SSP3_BASE		0xD401F000
#define ARMD1_SSP4_BASE		0xD4020000
#define ARMD1_SSP5_BASE		0xD4021000
#define ARMD1_UART3_BASE	0xD4026000
#define ARMD1_MPMU_BASE		0xD4050000
#define ARMD1_USB_HOST_BASE	0xD4209000
#define ARMD1_APMU_BASE		0xD4282800
#define ARMD1_CPU_BASE		0xD4282C00

#endif /* CONFIG_ARMADA100 */
#endif /* _ASM_ARCH_ARMADA100_H */
