/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (C) 2019 MediaTek Inc.
 *
 * Author:  Weijie Gao <weijie.gao@mediatek.com>
 */

#ifndef _DT_BINDINGS_MT7628_CLK_H_
#define _DT_BINDINGS_MT7628_CLK_H_

/* Base clocks */
#define CLK_SYS				34
#define CLK_CPU				33
#define CLK_XTAL			32

/* Peripheral clocks */
#define CLK_PWM				31
#define CLK_SDXC			30
#define CLK_CRYPTO			29
#define CLK_MIPS_CNT			28
#define CLK_PCIE			26
#define CLK_UPHY			25
#define CLK_ETH				23
#define CLK_UART2			20
#define CLK_UART1			19
#define CLK_SPI				18
#define CLK_I2S				17
#define CLK_I2C				16
#define CLK_GDMA			14
#define CLK_PIO				13
#define CLK_UART0			12
#define CLK_PCM				11
#define CLK_MC				10
#define CLK_INTC			9
#define CLK_TIMER			8

#endif /* _DT_BINDINGS_MT7628_CLK_H_ */
