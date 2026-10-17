/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (C) 2020-2021, Intel Corporation
 */

#ifndef __N5X_CLOCK_H
#define __N5X_CLOCK_H

/* fixed rate clocks */
#define N5X_OSC1					0
#define N5X_CB_INTOSC_HS_DIV2_CLK		1
#define N5X_CB_INTOSC_LS_CLK			2
#define N5X_L4_SYS_FREE_CLK			3
#define N5X_F2S_FREE_CLK				4

/* PLL clocks */
#define N5X_MAIN_PLL_CLK				5
#define N5X_MAIN_PLL_C0_CLK			6
#define N5X_MAIN_PLL_C1_CLK			7
#define N5X_MAIN_PLL_C2_CLK			8
#define N5X_MAIN_PLL_C3_CLK			9
#define N5X_PERIPH_PLL_CLK			10
#define N5X_PERIPH_PLL_C0_CLK			11
#define N5X_PERIPH_PLL_C1_CLK			12
#define N5X_PERIPH_PLL_C2_CLK			13
#define N5X_PERIPH_PLL_C3_CLK			14
#define N5X_MPU_FREE_CLK				15
#define N5X_MPU_CCU_CLK				16
#define N5X_BOOT_CLK				17

/* fixed factor clocks */
#define N5X_L3_MAIN_FREE_CLK			18
#define N5X_NOC_FREE_CLK				19
#define N5X_S2F_USR0_CLK				20
#define N5X_NOC_CLK				21
#define N5X_EMAC_A_FREE_CLK			22
#define N5X_EMAC_B_FREE_CLK			23
#define N5X_EMAC_PTP_FREE_CLK			24
#define N5X_GPIO_DB_FREE_CLK			25
#define N5X_SDMMC_FREE_CLK			26
#define N5X_S2F_USER0_FREE_CLK			27
#define N5X_S2F_USER1_FREE_CLK			28
#define N5X_PSI_REF_FREE_CLK			29

/* Gate clocks */
#define N5X_MPU_CLK				30
#define N5X_MPU_PERIPH_CLK			31
#define N5X_L4_MAIN_CLK				32
#define N5X_L4_MP_CLK				33
#define N5X_L4_SP_CLK				34
#define N5X_CS_AT_CLK				35
#define N5X_CS_TRACE_CLK				36
#define N5X_CS_PDBG_CLK				37
#define N5X_CS_TIMER_CLK				38
#define N5X_S2F_USER0_CLK			39
#define N5X_EMAC0_CLK				40
#define N5X_EMAC1_CLK				41
#define N5X_EMAC2_CLK				42
#define N5X_EMAC_PTP_CLK				43
#define N5X_GPIO_DB_CLK				44
#define N5X_NAND_CLK				45
#define N5X_PSI_REF_CLK				46
#define N5X_S2F_USER1_CLK			47
#define N5X_SDMMC_CLK				48
#define N5X_SPI_M_CLK				49
#define N5X_USB_CLK				50
#define N5X_NAND_X_CLK				51
#define N5X_NAND_ECC_CLK				52
#define N5X_NUM_CLKS				53

#endif	/* __N5X_CLOCK_H */
