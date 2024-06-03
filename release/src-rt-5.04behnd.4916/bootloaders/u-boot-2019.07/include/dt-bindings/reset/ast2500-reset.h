/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2017 Google, Inc
 */

#ifndef _ABI_MACH_ASPEED_AST2500_RESET_H_
#define _ABI_MACH_ASPEED_AST2500_RESET_H_

/*
 * The values are intentionally layed out as flags in
 * WDT reset parameter.
 */

#define AST_RESET_SOC			0
#define AST_RESET_CHIP			1
#define AST_RESET_CPU			(1 << 1)
#define AST_RESET_ARM			(1 << 2)
#define AST_RESET_COPROC		(1 << 3)
#define AST_RESET_SDRAM			(1 << 4)
#define AST_RESET_AHB			(1 << 5)
#define AST_RESET_I2C			(1 << 6)
#define AST_RESET_MAC1			(1 << 7)
#define AST_RESET_MAC2			(1 << 8)
#define AST_RESET_GCRT			(1 << 9)
#define AST_RESET_USB20			(1 << 10)
#define AST_RESET_USB11_HOST		(1 << 11)
#define AST_RESET_USB11_HID		(1 << 12)
#define AST_RESET_VIDEO			(1 << 13)
#define AST_RESET_HAC			(1 << 14)
#define AST_RESET_LPC			(1 << 15)
#define AST_RESET_SDIO			(1 << 16)
#define AST_RESET_MIC			(1 << 17)
#define AST_RESET_CRT2D			(1 << 18)
#define AST_RESET_PWM			(1 << 19)
#define AST_RESET_PECI			(1 << 20)
#define AST_RESET_JTAG			(1 << 21)
#define AST_RESET_ADC			(1 << 22)
#define AST_RESET_GPIO			(1 << 23)
#define AST_RESET_MCTP			(1 << 24)
#define AST_RESET_XDMA			(1 << 25)
#define AST_RESET_SPI			(1 << 26)
#define AST_RESET_MISC			(1 << 27)

#endif  /* _ABI_MACH_ASPEED_AST2500_RESET_H_ */
