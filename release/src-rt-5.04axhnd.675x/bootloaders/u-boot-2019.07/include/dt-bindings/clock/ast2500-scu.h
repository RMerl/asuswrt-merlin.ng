/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2016 Google Inc.
 */

/* Core Clocks */
#define PLL_HPLL	1
#define PLL_DPLL	2
#define PLL_D2PLL	3
#define PLL_MPLL	4
#define ARMCLK		5


/* Bus Clocks, derived from core clocks */
#define BCLK_PCLK	101
#define BCLK_LHCLK	102
#define BCLK_MACCLK	103
#define BCLK_SDCLK	104
#define BCLK_ARMCLK	105

#define MCLK_DDR	201

/* Special clocks */
#define PCLK_UART1	501
#define PCLK_UART2	502
#define PCLK_UART3	503
#define PCLK_UART4	504
#define PCLK_UART5	505
#define PCLK_MAC1	506
#define PCLK_MAC2	507
