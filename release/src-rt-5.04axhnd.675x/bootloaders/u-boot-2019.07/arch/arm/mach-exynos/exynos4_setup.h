/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Machine Specific Values for EXYNOS4012 based board
 *
 * Copyright (C) 2011 Samsung Electronics
 */

#ifndef _ORIGEN_SETUP_H
#define _ORIGEN_SETUP_H

#include <config.h>
#include <asm/arch/cpu.h>

#ifdef CONFIG_CLK_800_330_165
#define DRAM_CLK_330
#endif
#ifdef CONFIG_CLK_1000_200_200
#define DRAM_CLK_200
#endif
#ifdef CONFIG_CLK_1000_330_165
#define DRAM_CLK_330
#endif
#ifdef CONFIG_CLK_1000_400_200
#define DRAM_CLK_400
#endif

/* Bus Configuration Register Address */
#define ASYNC_CONFIG		0x10010350

/* CLK_SRC_CPU */
#define MUX_HPM_SEL_MOUTAPLL		0x0
#define MUX_HPM_SEL_SCLKMPLL		0x1
#define MUX_CORE_SEL_MOUTAPLL		0x0
#define MUX_CORE_SEL_SCLKMPLL		0x1
#define MUX_MPLL_SEL_FILPLL		0x0
#define MUX_MPLL_SEL_MOUTMPLLFOUT	0x1
#define MUX_APLL_SEL_FILPLL		0x0
#define MUX_APLL_SEL_MOUTMPLLFOUT	0x1
#define CLK_SRC_CPU_VAL			((MUX_HPM_SEL_MOUTAPLL << 20) \
					| (MUX_CORE_SEL_MOUTAPLL << 16) \
					| (MUX_MPLL_SEL_MOUTMPLLFOUT << 8)\
					| (MUX_APLL_SEL_MOUTMPLLFOUT << 0))

/* CLK_DIV_CPU0 */
#define APLL_RATIO		0x0
#define PCLK_DBG_RATIO		0x1
#define ATB_RATIO		0x3
#define PERIPH_RATIO		0x3
#define COREM1_RATIO		0x7
#define COREM0_RATIO		0x3
#define CORE_RATIO		0x0
#define CLK_DIV_CPU0_VAL	((APLL_RATIO << 24) \
				| (PCLK_DBG_RATIO << 20) \
				| (ATB_RATIO << 16) \
				| (PERIPH_RATIO << 12) \
				| (COREM1_RATIO << 8) \
				| (COREM0_RATIO << 4) \
				| (CORE_RATIO << 0))

/* CLK_DIV_CPU1 */
#define HPM_RATIO		0x0
#define COPY_RATIO		0x3
#define CLK_DIV_CPU1_VAL	((HPM_RATIO << 4) | (COPY_RATIO))

/* CLK_SRC_DMC */
#define MUX_PWI_SEL_XXTI		0x0
#define MUX_PWI_SEL_XUSBXTI		0x1
#define MUX_PWI_SEL_SCLK_HDMI24M	0x2
#define MUX_PWI_SEL_SCLK_USBPHY0	0x3
#define MUX_PWI_SEL_SCLK_USBPHY1	0x4
#define MUX_PWI_SEL_SCLK_HDMIPHY	0x5
#define MUX_PWI_SEL_SCLKMPLL		0x6
#define MUX_PWI_SEL_SCLKEPLL		0x7
#define MUX_PWI_SEL_SCLKVPLL		0x8
#define MUX_DPHY_SEL_SCLKMPLL		0x0
#define MUX_DPHY_SEL_SCLKAPLL		0x1
#define MUX_DMC_BUS_SEL_SCLKMPLL	0x0
#define MUX_DMC_BUS_SEL_SCLKAPLL	0x1
#define CLK_SRC_DMC_VAL			((MUX_PWI_SEL_XUSBXTI << 16) \
					| (MUX_DPHY_SEL_SCLKMPLL << 8) \
					| (MUX_DMC_BUS_SEL_SCLKMPLL << 4))

/* CLK_DIV_DMC0 */
#define CORE_TIMERS_RATIO	0x1
#define COPY2_RATIO		0x3
#define DMCP_RATIO		0x1
#define DMCD_RATIO		0x1
#define DMC_RATIO		0x1
#define DPHY_RATIO		0x1
#define ACP_PCLK_RATIO		0x1
#define ACP_RATIO		0x3
#define CLK_DIV_DMC0_VAL	((CORE_TIMERS_RATIO << 28) \
				| (COPY2_RATIO << 24) \
				| (DMCP_RATIO << 20) \
				| (DMCD_RATIO << 16) \
				| (DMC_RATIO << 12) \
				| (DPHY_RATIO << 8) \
				| (ACP_PCLK_RATIO << 4)	\
				| (ACP_RATIO << 0))

/* CLK_DIV_DMC1 */
#define DPM_RATIO		0x1
#define DVSEM_RATIO		0x1
#define PWI_RATIO		0x1
#define CLK_DIV_DMC1_VAL	((DPM_RATIO << 24) \
				| (DVSEM_RATIO << 16) \
				| (PWI_RATIO << 8))

/* CLK_SRC_TOP0 */
#define MUX_ONENAND_SEL_ACLK_133	0x0
#define MUX_ONENAND_SEL_ACLK_160	0x1
#define MUX_ACLK_133_SEL_SCLKMPLL	0x0
#define MUX_ACLK_133_SEL_SCLKAPLL	0x1
#define MUX_ACLK_160_SEL_SCLKMPLL	0x0
#define MUX_ACLK_160_SEL_SCLKAPLL	0x1
#define MUX_ACLK_100_SEL_SCLKMPLL	0x0
#define MUX_ACLK_100_SEL_SCLKAPLL	0x1
#define MUX_ACLK_200_SEL_SCLKMPLL	0x0
#define MUX_ACLK_200_SEL_SCLKAPLL	0x1
#define MUX_VPLL_SEL_FINPLL		0x0
#define MUX_VPLL_SEL_FOUTVPLL		0x1
#define MUX_EPLL_SEL_FINPLL		0x0
#define MUX_EPLL_SEL_FOUTEPLL		0x1
#define MUX_ONENAND_1_SEL_MOUTONENAND	0x0
#define MUX_ONENAND_1_SEL_SCLKVPLL	0x1
#define CLK_SRC_TOP0_VAL		((MUX_ONENAND_SEL_ACLK_133 << 28) \
					| (MUX_ACLK_133_SEL_SCLKMPLL << 24) \
					| (MUX_ACLK_160_SEL_SCLKMPLL << 20) \
					| (MUX_ACLK_100_SEL_SCLKMPLL << 16) \
					| (MUX_ACLK_200_SEL_SCLKMPLL << 12) \
					| (MUX_VPLL_SEL_FINPLL << 8) \
					| (MUX_EPLL_SEL_FINPLL << 4)\
					| (MUX_ONENAND_1_SEL_MOUTONENAND << 0))

/* CLK_SRC_TOP1 */
#define VPLLSRC_SEL_FINPLL	0x0
#define VPLLSRC_SEL_SCLKHDMI24M	0x1
#define CLK_SRC_TOP1_VAL	(VPLLSRC_SEL_FINPLL)

/* CLK_DIV_TOP */
#define ONENAND_RATIO		0x0
#define ACLK_133_RATIO		0x5
#define ACLK_160_RATIO		0x4
#define ACLK_100_RATIO		0x7
#define ACLK_200_RATIO		0x3
#define CLK_DIV_TOP_VAL		((ONENAND_RATIO << 16)	\
				| (ACLK_133_RATIO << 12)\
				| (ACLK_160_RATIO << 8)	\
				| (ACLK_100_RATIO << 4)	\
				| (ACLK_200_RATIO << 0))

/* CLK_SRC_LEFTBUS */
#define MUX_GDL_SEL_SCLKMPLL	0x0
#define MUX_GDL_SEL_SCLKAPLL	0x1
#define CLK_SRC_LEFTBUS_VAL	(MUX_GDL_SEL_SCLKMPLL)

/* CLK_DIV_LEFTBUS */
#define GPL_RATIO		0x1
#define GDL_RATIO		0x3
#define CLK_DIV_LEFTBUS_VAL	((GPL_RATIO << 4) | (GDL_RATIO))

/* CLK_SRC_RIGHTBUS */
#define MUX_GDR_SEL_SCLKMPLL	0x0
#define MUX_GDR_SEL_SCLKAPLL	0x1
#define CLK_SRC_RIGHTBUS_VAL	(MUX_GDR_SEL_SCLKMPLL)

/* CLK_DIV_RIGHTBUS */
#define GPR_RATIO		0x1
#define GDR_RATIO		0x3
#define CLK_DIV_RIGHTBUS_VAL	((GPR_RATIO << 4) | (GDR_RATIO))

/* CLK_SRS_FSYS: 6 = SCLKMPLL */
#define SATA_SEL_SCLKMPLL	0
#define SATA_SEL_SCLKAPLL	1

#define MMC_SEL_XXTI		0
#define MMC_SEL_XUSBXTI		1
#define MMC_SEL_SCLK_HDMI24M	2
#define MMC_SEL_SCLK_USBPHY0	3
#define MMC_SEL_SCLK_USBPHY1	4
#define MMC_SEL_SCLK_HDMIPHY	5
#define MMC_SEL_SCLKMPLL	6
#define MMC_SEL_SCLKEPLL	7
#define MMC_SEL_SCLKVPLL	8

#define MMCC0_SEL		MMC_SEL_SCLKMPLL
#define MMCC1_SEL		MMC_SEL_SCLKMPLL
#define MMCC2_SEL		MMC_SEL_SCLKMPLL
#define MMCC3_SEL		MMC_SEL_SCLKMPLL
#define MMCC4_SEL		MMC_SEL_SCLKMPLL
#define CLK_SRC_FSYS_VAL	((SATA_SEL_SCLKMPLL << 24) \
				| (MMCC4_SEL << 16) \
				| (MMCC3_SEL << 12) \
				| (MMCC2_SEL << 8) \
				| (MMCC1_SEL << 4) \
				| (MMCC0_SEL << 0))

/* SCLK_MMC[0-4] = MOUTMMC[0-4]/(MMC[0-4]_RATIO + 1)/(MMC[0-4]_PRE_RATIO +1) */
/* CLK_DIV_FSYS1 */
#define MMC0_RATIO		0xF
#define MMC0_PRE_RATIO		0x0
#define MMC1_RATIO		0xF
#define MMC1_PRE_RATIO		0x0
#define CLK_DIV_FSYS1_VAL	((MMC1_PRE_RATIO << 24) \
				| (MMC1_RATIO << 16) \
				| (MMC0_PRE_RATIO << 8) \
				| (MMC0_RATIO << 0))

/* CLK_DIV_FSYS2 */
#define MMC2_RATIO		0xF
#define MMC2_PRE_RATIO		0x0
#define MMC3_RATIO		0xF
#define MMC3_PRE_RATIO		0x0
#define CLK_DIV_FSYS2_VAL	((MMC3_PRE_RATIO << 24) \
				| (MMC3_RATIO << 16) \
				| (MMC2_PRE_RATIO << 8) \
				| (MMC2_RATIO << 0))

/* CLK_DIV_FSYS3 */
#define MMC4_RATIO		0xF
#define MMC4_PRE_RATIO		0x0
#define CLK_DIV_FSYS3_VAL	((MMC4_PRE_RATIO << 8) \
				| (MMC4_RATIO << 0))

/* CLK_SRC_PERIL0 */
#define UART_SEL_XXTI		0
#define UART_SEL_XUSBXTI	1
#define UART_SEL_SCLK_HDMI24M	2
#define UART_SEL_SCLK_USBPHY0	3
#define UART_SEL_SCLK_USBPHY1	4
#define UART_SEL_SCLK_HDMIPHY	5
#define UART_SEL_SCLKMPLL	6
#define UART_SEL_SCLKEPLL	7
#define UART_SEL_SCLKVPLL	8

#define UART0_SEL		UART_SEL_SCLKMPLL
#define UART1_SEL		UART_SEL_SCLKMPLL
#define UART2_SEL		UART_SEL_SCLKMPLL
#define UART3_SEL		UART_SEL_SCLKMPLL
#define UART4_SEL		UART_SEL_SCLKMPLL
#define CLK_SRC_PERIL0_VAL	((UART4_SEL << 16) \
				| (UART3_SEL << 12) \
				| (UART2_SEL << 8) \
				| (UART1_SEL << 4) \
				| (UART0_SEL << 0))

/* SCLK_UART[0-4] = MOUTUART[0-4]/(UART[0-4]_RATIO + 1) */
/* CLK_DIV_PERIL0 */
#define UART0_RATIO		7
#define UART1_RATIO		7
#define UART2_RATIO		7
#define UART3_RATIO		7
#define UART4_RATIO		7
#define CLK_DIV_PERIL0_VAL	((UART4_RATIO << 16) \
				| (UART3_RATIO << 12) \
				| (UART2_RATIO << 8) \
				| (UART1_RATIO << 4) \
				| (UART0_RATIO << 0))

/* Clock Source CAM/FIMC */
/* CLK_SRC_CAM */
#define CAM0_SEL_XUSBXTI	1
#define CAM1_SEL_XUSBXTI	1
#define CSIS0_SEL_XUSBXTI	1
#define CSIS1_SEL_XUSBXTI	1

#define FIMC_SEL_SCLKMPLL	6
#define FIMC0_LCLK_SEL		FIMC_SEL_SCLKMPLL
#define FIMC1_LCLK_SEL		FIMC_SEL_SCLKMPLL
#define FIMC2_LCLK_SEL		FIMC_SEL_SCLKMPLL
#define FIMC3_LCLK_SEL		FIMC_SEL_SCLKMPLL

#define CLK_SRC_CAM_VAL		((CSIS1_SEL_XUSBXTI << 28) \
				| (CSIS0_SEL_XUSBXTI << 24) \
				| (CAM1_SEL_XUSBXTI << 20) \
				| (CAM0_SEL_XUSBXTI << 16) \
				| (FIMC3_LCLK_SEL << 12) \
				| (FIMC2_LCLK_SEL << 8) \
				| (FIMC1_LCLK_SEL << 4) \
				| (FIMC0_LCLK_SEL << 0))

/* SCLK CAM */
/* CLK_DIV_CAM */
#define FIMC0_LCLK_RATIO	4
#define FIMC1_LCLK_RATIO	4
#define FIMC2_LCLK_RATIO	4
#define FIMC3_LCLK_RATIO	4
#define CLK_DIV_CAM_VAL		((FIMC3_LCLK_RATIO << 12) \
				| (FIMC2_LCLK_RATIO << 8) \
				| (FIMC1_LCLK_RATIO << 4) \
				| (FIMC0_LCLK_RATIO << 0))

/* SCLK MFC */
/* CLK_SRC_MFC */
#define MFC_SEL_MPLL		0
#define MOUTMFC_0		0
#define MFC_SEL			MOUTMFC_0
#define MFC_0_SEL		MFC_SEL_MPLL
#define CLK_SRC_MFC_VAL		((MFC_SEL << 8) | (MFC_0_SEL))


/* CLK_DIV_MFC */
#define MFC_RATIO		3
#define CLK_DIV_MFC_VAL		(MFC_RATIO)

/* SCLK G3D */
/* CLK_SRC_G3D */
#define G3D_SEL_MPLL		0
#define MOUTG3D_0		0
#define G3D_SEL			MOUTG3D_0
#define G3D_0_SEL		G3D_SEL_MPLL
#define CLK_SRC_G3D_VAL		((G3D_SEL << 8) | (G3D_0_SEL))

/* CLK_DIV_G3D */
#define G3D_RATIO		1
#define CLK_DIV_G3D_VAL		(G3D_RATIO)

/* SCLK LCD0 */
/* CLK_SRC_LCD0 */
#define FIMD_SEL_SCLKMPLL	6
#define MDNIE0_SEL_XUSBXTI	1
#define MDNIE_PWM0_SEL_XUSBXTI	1
#define MIPI0_SEL_XUSBXTI	1
#define CLK_SRC_LCD0_VAL	((MIPI0_SEL_XUSBXTI << 12) \
				| (MDNIE_PWM0_SEL_XUSBXTI << 8) \
				| (MDNIE0_SEL_XUSBXTI << 4) \
				| (FIMD_SEL_SCLKMPLL << 0))

/* CLK_DIV_LCD0 */
#define FIMD0_RATIO		4
#define CLK_DIV_LCD0_VAL	(FIMD0_RATIO)

/* Required period to generate a stable clock output */
/* PLL_LOCK_TIME */
#define PLL_LOCKTIME		0x1C20

/* PLL Values */
#define DISABLE			0
#define ENABLE			1
#define SET_PLL(mdiv, pdiv, sdiv)	((ENABLE << 31)\
					| (mdiv << 16) \
					| (pdiv << 8) \
					| (sdiv << 0))

/* APLL_CON0 */
#define APLL_MDIV		0xFA
#define APLL_PDIV		0x6
#define APLL_SDIV		0x1
#define APLL_CON0_VAL		SET_PLL(APLL_MDIV, APLL_PDIV, APLL_SDIV)

/* APLL_CON1 */
#define APLL_AFC_ENB		0x1
#define APLL_AFC		0xC
#define APLL_CON1_VAL		((APLL_AFC_ENB << 31) | (APLL_AFC << 0))

/* MPLL_CON0 */
#define MPLL_MDIV		0xC8
#define MPLL_PDIV		0x6
#define MPLL_SDIV		0x1
#define MPLL_CON0_VAL		SET_PLL(MPLL_MDIV, MPLL_PDIV, MPLL_SDIV)

/* MPLL_CON1 */
#define MPLL_AFC_ENB		0x0
#define MPLL_AFC		0x1C
#define MPLL_CON1_VAL		((MPLL_AFC_ENB << 31) | (MPLL_AFC << 0))

/* EPLL_CON0 */
#define EPLL_MDIV		0x30
#define EPLL_PDIV		0x3
#define EPLL_SDIV		0x2
#define EPLL_CON0_VAL		SET_PLL(EPLL_MDIV, EPLL_PDIV, EPLL_SDIV)

/* EPLL_CON1 */
#define EPLL_K			0x0
#define EPLL_CON1_VAL		(EPLL_K >> 0)

/* VPLL_CON0 */
#define VPLL_MDIV		0x35
#define VPLL_PDIV		0x3
#define VPLL_SDIV		0x2
#define VPLL_CON0_VAL		SET_PLL(VPLL_MDIV, VPLL_PDIV, VPLL_SDIV)

/* VPLL_CON1 */
#define VPLL_SSCG_EN		DISABLE
#define VPLL_SEL_PF_DN_SPREAD	0x0
#define VPLL_MRR		0x11
#define VPLL_MFR		0x0
#define VPLL_K			0x400
#define VPLL_CON1_VAL		((VPLL_SSCG_EN << 31)\
				| (VPLL_SEL_PF_DN_SPREAD << 29) \
				| (VPLL_MRR << 24) \
				| (VPLL_MFR << 16) \
				| (VPLL_K << 0))

/* DMC */
#define DIRECT_CMD_NOP	0x07000000
#define DIRECT_CMD_ZQ	0x0a000000
#define DIRECT_CMD_CHIP1_SHIFT	(1 << 20)
#define MEM_TIMINGS_MSR_COUNT	4
#define CTRL_START	(1 << 0)
#define CTRL_DLL_ON	(1 << 1)
#define AREF_EN		(1 << 5)
#define DRV_TYPE	(1 << 6)

struct mem_timings {
	unsigned direct_cmd_msr[MEM_TIMINGS_MSR_COUNT];
	unsigned timingref;
	unsigned timingrow;
	unsigned timingdata;
	unsigned timingpower;
	unsigned zqcontrol;
	unsigned control0;
	unsigned control1;
	unsigned control2;
	unsigned concontrol;
	unsigned prechconfig;
	unsigned memcontrol;
	unsigned memconfig0;
	unsigned memconfig1;
	unsigned dll_resync;
	unsigned dll_on;
};

/* MIU */
/* MIU Config Register Offsets*/
#define APB_SFR_INTERLEAVE_CONF_OFFSET	0x400
#define APB_SFR_ARBRITATION_CONF_OFFSET	0xC00
#define ABP_SFR_SLV_ADDRMAP_CONF_OFFSET	0x800
#define ABP_SFR_INTERLEAVE_ADDRMAP_START_OFFSET	0x808
#define ABP_SFR_INTERLEAVE_ADDRMAP_END_OFFSET	0x810
#define ABP_SFR_SLV0_SINGLE_ADDRMAP_START_OFFSET	0x818
#define ABP_SFR_SLV0_SINGLE_ADDRMAP_END_OFFSET	0x820
#define ABP_SFR_SLV1_SINGLE_ADDRMAP_START_OFFSET	0x828
#define ABP_SFR_SLV1_SINGLE_ADDRMAP_END_OFFSET	0x830

#ifdef CONFIG_ORIGEN
/* Interleave: 2Bit, Interleave_bit1: 0x15, Interleave_bit0: 0x7 */
#define APB_SFR_INTERLEAVE_CONF_VAL	0x20001507
#define APB_SFR_ARBRITATION_CONF_VAL	0x00000001
#endif

#define INTERLEAVE_ADDR_MAP_START_ADDR	0x40000000
#define INTERLEAVE_ADDR_MAP_END_ADDR	0xbfffffff
#define INTERLEAVE_ADDR_MAP_EN		0x00000001

#ifdef CONFIG_MIU_1BIT_INTERLEAVED
/* Interleave_bit0: 0xC*/
#define APB_SFR_INTERLEAVE_CONF_VAL	0x0000000c
#endif
#ifdef CONFIG_MIU_2BIT_INTERLEAVED
/* Interleave: 2Bit, Interleave_bit1: 0x15, Interleave_bit0: 0xc */
#define APB_SFR_INTERLEAVE_CONF_VAL	0x2000150c
#endif
#define SLAVE0_SINGLE_ADDR_MAP_START_ADDR	0x40000000
#define SLAVE0_SINGLE_ADDR_MAP_END_ADDR		0x7fffffff
#define SLAVE1_SINGLE_ADDR_MAP_START_ADDR	0x80000000
#define SLAVE1_SINGLE_ADDR_MAP_END_ADDR		0xbfffffff
/* Enable SME0 and SME1*/
#define APB_SFR_SLV_ADDR_MAP_CONF_VAL		0x00000006

#define FORCE_DLL_RESYNC	3
#define DLL_CONTROL_ON		1

#define DIRECT_CMD1	0x00020000
#define DIRECT_CMD2	0x00030000
#define DIRECT_CMD3	0x00010002
#define DIRECT_CMD4	0x00000328

#define CTRL_ZQ_MODE_NOTERM	(0x1 << 0)
#define CTRL_ZQ_START		(0x1 << 1)
#define CTRL_ZQ_DIV		(0 << 4)
#define CTRL_ZQ_MODE_DDS	(0x7 << 8)
#define CTRL_ZQ_MODE_TERM	(0x2 << 11)
#define CTRL_ZQ_FORCE_IMPN	(0x5 << 14)
#define CTRL_ZQ_FORCE_IMPP	(0x6 << 17)
#define CTRL_DCC		(0xE38 << 20)
#define ZQ_CONTROL_VAL		(CTRL_ZQ_MODE_NOTERM | CTRL_ZQ_START\
				| CTRL_ZQ_DIV | CTRL_ZQ_MODE_DDS\
				| CTRL_ZQ_MODE_TERM | CTRL_ZQ_FORCE_IMPN\
				| CTRL_ZQ_FORCE_IMPP | CTRL_DCC)

#define ASYNC			(0 << 0)
#define CLK_RATIO		(1 << 1)
#define DIV_PIPE		(1 << 3)
#define AWR_ON			(1 << 4)
#define AREF_DISABLE		(0 << 5)
#define DRV_TYPE_DISABLE	(0 << 6)
#define CHIP0_NOT_EMPTY		(0 << 8)
#define CHIP1_NOT_EMPTY		(0 << 9)
#define DQ_SWAP_DISABLE		(0 << 10)
#define QOS_FAST_DISABLE	(0 << 11)
#define RD_FETCH		(0x3 << 12)
#define TIMEOUT_LEVEL0		(0xFFF << 16)
#define CONCONTROL_VAL		(ASYNC | CLK_RATIO | DIV_PIPE | AWR_ON\
				| AREF_DISABLE | DRV_TYPE_DISABLE\
				| CHIP0_NOT_EMPTY | CHIP1_NOT_EMPTY\
				| DQ_SWAP_DISABLE | QOS_FAST_DISABLE\
				| RD_FETCH | TIMEOUT_LEVEL0)

#define CLK_STOP_DISABLE	(0 << 1)
#define DPWRDN_DISABLE		(0 << 2)
#define DPWRDN_TYPE		(0 << 3)
#define TP_DISABLE		(0 << 4)
#define DSREF_DIABLE		(0 << 5)
#define ADD_LAT_PALL		(1 << 6)
#define MEM_TYPE_DDR3		(0x6 << 8)
#define MEM_WIDTH_32		(0x2 << 12)
#define NUM_CHIP_2		(1 << 16)
#define BL_8			(0x3 << 20)
#define MEMCONTROL_VAL		(CLK_STOP_DISABLE | DPWRDN_DISABLE\
				| DPWRDN_TYPE | TP_DISABLE | DSREF_DIABLE\
				| ADD_LAT_PALL | MEM_TYPE_DDR3 | MEM_WIDTH_32\
				| NUM_CHIP_2 | BL_8)


#define CHIP_BANK_8		(0x3 << 0)
#define CHIP_ROW_14		(0x2 << 4)
#define CHIP_COL_10		(0x3 << 8)
#define CHIP_MAP_INTERLEAVED	(1 << 12)
#define CHIP_MASK		(0xe0 << 16)
#ifdef CONFIG_MIU_LINEAR
#define CHIP0_BASE		(0x40 << 24)
#define CHIP1_BASE		(0x60 << 24)
#else
#define CHIP0_BASE		(0x20 << 24)
#define CHIP1_BASE		(0x40 << 24)
#endif
#define MEMCONFIG0_VAL		(CHIP_BANK_8 | CHIP_ROW_14 | CHIP_COL_10\
				| CHIP_MAP_INTERLEAVED | CHIP_MASK | CHIP0_BASE)
#define MEMCONFIG1_VAL		(CHIP_BANK_8 | CHIP_ROW_14 | CHIP_COL_10\
				| CHIP_MAP_INTERLEAVED | CHIP_MASK | CHIP1_BASE)

#define TP_CNT			(0xff << 24)
#define PRECHCONFIG		TP_CNT

#define CTRL_OFF		(0 << 0)
#define CTRL_DLL_OFF		(0 << 1)
#define CTRL_HALF		(0 << 2)
#define CTRL_DFDQS		(1 << 3)
#define DQS_DELAY		(0 << 4)
#define CTRL_START_POINT	(0x10 << 8)
#define CTRL_INC		(0x10 << 16)
#define CTRL_FORCE		(0x71 << 24)
#define CONTROL0_VAL		(CTRL_OFF | CTRL_DLL_OFF | CTRL_HALF\
				| CTRL_DFDQS | DQS_DELAY | CTRL_START_POINT\
				| CTRL_INC | CTRL_FORCE)

#define CTRL_SHIFTC		(0x6 << 0)
#define CTRL_REF		(8 << 4)
#define CTRL_SHGATE		(1 << 29)
#define TERM_READ_EN		(1 << 30)
#define TERM_WRITE_EN		(1 << 31)
#define CONTROL1_VAL		(CTRL_SHIFTC | CTRL_REF | CTRL_SHGATE\
				| TERM_READ_EN | TERM_WRITE_EN)

#define CONTROL2_VAL		0x00000000

#ifdef CONFIG_ORIGEN
#define TIMINGREF_VAL		0x000000BB
#define TIMINGROW_VAL		0x4046654f
#define	TIMINGDATA_VAL		0x46400506
#define	TIMINGPOWER_VAL		0x52000A3C
#else
#define TIMINGREF_VAL		0x000000BC
#ifdef DRAM_CLK_330
#define TIMINGROW_VAL		0x3545548d
#define	TIMINGDATA_VAL		0x45430506
#define	TIMINGPOWER_VAL		0x4439033c
#endif
#ifdef DRAM_CLK_400
#define TIMINGROW_VAL		0x45430506
#define	TIMINGDATA_VAL		0x56500506
#define	TIMINGPOWER_VAL		0x5444033d
#endif
#endif
#endif
