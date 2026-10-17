/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Actions Semi S900 Register Definitions
 *
 * Copyright (C) 2015 Actions Semi Co., Ltd.
 * Copyright (C) 2018 Manivannan Sadhasivam <manivannan.sadhasivam@linaro.org>
 *
 */

#ifndef _OWL_REGS_S900_H_
#define _OWL_REGS_S900_H_

/* CMU registers */
#define CMU_COREPLL				(0x0000)
#define CMU_DEVPLL				(0x0004)
#define CMU_DDRPLL				(0x0008)
#define CMU_NANDPLL				(0x000C)
#define CMU_DISPLAYPLL				(0x0010)
#define CMU_AUDIOPLL				(0x0014)
#define CMU_TVOUTPLL				(0x0018)
#define CMU_BUSCLK				(0x001C)
#define CMU_SENSORCLK				(0x0020)
#define CMU_LCDCLK				(0x0024)
#define CMU_DSICLK				(0x0028)
#define CMU_CSICLK				(0x002C)
#define CMU_DECLK				(0x0030)
#define CMU_BISPCLK				(0x0034)
#define CMU_IMXCLK				(0x0038)
#define CMU_HDECLK				(0x003C)
#define CMU_VDECLK				(0x0040)
#define CMU_VCECLK				(0x0044)
#define CMU_NANDCCLK				(0x004C)
#define CMU_SD0CLK				(0x0050)
#define CMU_SD1CLK				(0x0054)
#define CMU_SD2CLK				(0x0058)
#define CMU_UART0CLK				(0x005C)
#define CMU_UART1CLK				(0x0060)
#define CMU_UART2CLK				(0x0064)
#define CMU_PWM0CLK				(0x0070)
#define CMU_PWM1CLK				(0x0074)
#define CMU_PWM2CLK				(0x0078)
#define CMU_PWM3CLK				(0x007C)
#define CMU_USBPLL				(0x0080)
#define CMU_ASSISTPLL				(0x0084)
#define CMU_EDPCLK				(0x0088)
#define CMU_GPU3DCLK				(0x0090)
#define CMU_CORECTL				(0x009C)
#define CMU_DEVCLKEN0				(0x00A0)
#define CMU_DEVCLKEN1				(0x00A4)
#define CMU_DEVRST0				(0x00A8)
#define CMU_DEVRST1				(0x00AC)
#define CMU_UART3CLK				(0x00B0)
#define CMU_UART4CLK				(0x00B4)
#define CMU_UART5CLK				(0x00B8)
#define CMU_UART6CLK				(0x00BC)
#define CMU_TLSCLK				(0x00C0)
#define CMU_SD3CLK				(0x00C4)
#define CMU_PWM4CLK				(0x00C8)
#define CMU_PWM5CLK				(0x00CC)
#define CMU_ANALOGDEBUG				(0x00D4)
#define CMU_TVOUTPLLDEBUG0			(0x00EC)
#define CMU_TVOUTPLLDEBUG1			(0x00FC)

#define CMU_DEVCLKEN1_ETH			BIT(22)
#define CLK_ETHERNET				CLK_ETH_MAC
#define CMU_ETHERNETPLL				CMU_ASSISTPLL

#endif
