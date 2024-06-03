/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2014 Samsung Electronics
 * Przemyslaw Marczak <p.marczak@samsung.com>
 */

#ifndef __ODROIDU3_SETUP__
#define __ODROIDU3_SETUP__

/* A/M PLL_CON0 */
#define SDIV(x)                 ((x) & 0x7)
#define PDIV(x)                 (((x) & 0x3f) << 8)
#define MDIV(x)                 (((x) & 0x3ff) << 16)
#define FSEL(x)                 (((x) & 0x1) << 27)
#define PLL_LOCKED_BIT          (0x1 << 29)
#define PLL_ENABLE(x)           (((x) & 0x1) << 31)

/* CLK_SRC_CPU */
#define MUX_APLL_SEL(x)         ((x) & 0x1)
#define MUX_CORE_SEL(x)         (((x) & 0x1) << 16)
#define MUX_HPM_SEL(x)          (((x) & 0x1) << 20)
#define MUX_MPLL_USER_SEL_C(x)  (((x) & 0x1) << 24)

#define MUX_STAT_CHANGING       0x100

/* CLK_MUX_STAT_CPU */
#define APLL_SEL(x)             ((x) & 0x7)
#define CORE_SEL(x)             (((x) & 0x7) << 16)
#define HPM_SEL(x)              (((x) & 0x7) << 20)
#define MPLL_USER_SEL_C(x)      (((x) & 0x7) << 24)
#define MUX_STAT_CPU_CHANGING   (APLL_SEL(MUX_STAT_CHANGING) | \
				CORE_SEL(MUX_STAT_CHANGING) | \
				HPM_SEL(MUX_STAT_CHANGING) | \
				MPLL_USER_SEL_C(MUX_STAT_CHANGING))

/* CLK_DIV_CPU0 */
#define CORE_RATIO(x)           ((x) & 0x7)
#define COREM0_RATIO(x)         (((x) & 0x7) << 4)
#define COREM1_RATIO(x)         (((x) & 0x7) << 8)
#define PERIPH_RATIO(x)         (((x) & 0x7) << 12)
#define ATB_RATIO(x)            (((x) & 0x7) << 16)
#define PCLK_DBG_RATIO(x)       (((x) & 0x7) << 20)
#define APLL_RATIO(x)           (((x) & 0x7) << 24)
#define CORE2_RATIO(x)          (((x) & 0x7) << 28)

/* CLK_DIV_STAT_CPU0 */
#define DIV_CORE(x)             ((x) & 0x1)
#define DIV_COREM0(x)           (((x) & 0x1) << 4)
#define DIV_COREM1(x)           (((x) & 0x1) << 8)
#define DIV_PERIPH(x)           (((x) & 0x1) << 12)
#define DIV_ATB(x)              (((x) & 0x1) << 16)
#define DIV_PCLK_DBG(x)         (((x) & 0x1) << 20)
#define DIV_APLL(x)             (((x) & 0x1) << 24)
#define DIV_CORE2(x)            (((x) & 0x1) << 28)

#define DIV_STAT_CHANGING       0x1
#define DIV_STAT_CPU0_CHANGING  (DIV_CORE(DIV_STAT_CHANGING) | \
				DIV_COREM0(DIV_STAT_CHANGING) | \
				DIV_COREM1(DIV_STAT_CHANGING) | \
				DIV_PERIPH(DIV_STAT_CHANGING) | \
				DIV_ATB(DIV_STAT_CHANGING) | \
				DIV_PCLK_DBG(DIV_STAT_CHANGING) | \
				DIV_APLL(DIV_STAT_CHANGING) | \
				DIV_CORE2(DIV_STAT_CHANGING))

/* CLK_DIV_CPU1 */
#define COPY_RATIO(x)           ((x) & 0x7)
#define HPM_RATIO(x)            (((x) & 0x7) << 4)
#define CORES_RATIO(x)          (((x) & 0x7) << 8)

/* CLK_DIV_STAT_CPU1 */
#define DIV_COPY(x)             ((x) & 0x7)
#define DIV_HPM(x)              (((x) & 0x1) << 4)
#define DIV_CORES(x)            (((x) & 0x1) << 8)

#define DIV_STAT_CPU1_CHANGING	(DIV_COPY(DIV_STAT_CHANGING) | \
				DIV_HPM(DIV_STAT_CHANGING) | \
				DIV_CORES(DIV_STAT_CHANGING))

/* CLK_SRC_DMC */
#define MUX_C2C_SEL(x)		((x) & 0x1)
#define MUX_DMC_BUS_SEL(x)	(((x) & 0x1) << 4)
#define MUX_DPHY_SEL(x)		(((x) & 0x1) << 8)
#define MUX_MPLL_SEL(x)		(((x) & 0x1) << 12)
#define MUX_PWI_SEL(x)		(((x) & 0xf) << 16)
#define MUX_G2D_ACP0_SEL(x)	(((x) & 0x1) << 20)
#define MUX_G2D_ACP1_SEL(x)	(((x) & 0x1) << 24)
#define MUX_G2D_ACP_SEL(x)	(((x) & 0x1) << 28)

/* CLK_MUX_STAT_DMC */
#define C2C_SEL(x)		(((x)) & 0x7)
#define DMC_BUS_SEL(x)		(((x) & 0x7) << 4)
#define DPHY_SEL(x)		(((x) & 0x7) << 8)
#define MPLL_SEL(x)		(((x) & 0x7) << 12)
/* #define PWI_SEL(x)		(((x) & 0xf) << 16)  - Reserved */
#define G2D_ACP0_SEL(x)		(((x) & 0x7) << 20)
#define G2D_ACP1_SEL(x)		(((x) & 0x7) << 24)
#define G2D_ACP_SEL(x)		(((x) & 0x7) << 28)

#define MUX_STAT_DMC_CHANGING	(C2C_SEL(MUX_STAT_CHANGING) | \
				DMC_BUS_SEL(MUX_STAT_CHANGING) | \
				DPHY_SEL(MUX_STAT_CHANGING) | \
				MPLL_SEL(MUX_STAT_CHANGING) |\
				G2D_ACP0_SEL(MUX_STAT_CHANGING) | \
				G2D_ACP1_SEL(MUX_STAT_CHANGING) | \
				G2D_ACP_SEL(MUX_STAT_CHANGING))

/* CLK_DIV_DMC0 */
#define ACP_RATIO(x)		((x) & 0x7)
#define ACP_PCLK_RATIO(x)	(((x) & 0x7) << 4)
#define DPHY_RATIO(x)		(((x) & 0x7) << 8)
#define DMC_RATIO(x)		(((x) & 0x7) << 12)
#define DMCD_RATIO(x)		(((x) & 0x7) << 16)
#define DMCP_RATIO(x)		(((x) & 0x7) << 20)

/* CLK_DIV_STAT_DMC0 */
#define DIV_ACP(x)		((x) & 0x1)
#define DIV_ACP_PCLK(x)		(((x) & 0x1) << 4)
#define DIV_DPHY(x)		(((x) & 0x1) << 8)
#define DIV_DMC(x)		(((x) & 0x1) << 12)
#define DIV_DMCD(x)		(((x) & 0x1) << 16)
#define DIV_DMCP(x)		(((x) & 0x1) << 20)

#define DIV_STAT_DMC0_CHANGING	(DIV_ACP(DIV_STAT_CHANGING) | \
				DIV_ACP_PCLK(DIV_STAT_CHANGING) | \
				DIV_DPHY(DIV_STAT_CHANGING) | \
				DIV_DMC(DIV_STAT_CHANGING) | \
				DIV_DMCD(DIV_STAT_CHANGING) | \
				DIV_DMCP(DIV_STAT_CHANGING))

/* CLK_DIV_DMC1 */
#define G2D_ACP_RATIO(x)	((x) & 0xf)
#define C2C_RATIO(x)		(((x) & 0x7) << 4)
#define PWI_RATIO(x)		(((x) & 0xf) << 8)
#define C2C_ACLK_RATIO(x)	(((x) & 0x7) << 12)
#define DVSEM_RATIO(x)		(((x) & 0x7f) << 16)
#define DPM_RATIO(x)		(((x) & 0x7f) << 24)

/* CLK_DIV_STAT_DMC1 */
#define DIV_G2D_ACP(x)		((x) & 0x1)
#define DIV_C2C(x)		(((x) & 0x1) << 4)
#define DIV_PWI(x)		(((x) & 0x1) << 8)
#define DIV_C2C_ACLK(x)		(((x) & 0x1) << 12)
#define DIV_DVSEM(x)		(((x) & 0x1) << 16)
#define DIV_DPM(x)		(((x) & 0x1) << 24)

#define DIV_STAT_DMC1_CHANGING	(DIV_G2D_ACP(DIV_STAT_CHANGING) | \
				DIV_C2C(DIV_STAT_CHANGING) | \
				DIV_PWI(DIV_STAT_CHANGING) | \
				DIV_C2C_ACLK(DIV_STAT_CHANGING) | \
				DIV_DVSEM(DIV_STAT_CHANGING) | \
				DIV_DPM(DIV_STAT_CHANGING))

/* Set CLK_SRC_PERIL0 */
#define UART4_SEL(x)		(((x) & 0xf) << 16)
#define UART3_SEL(x)		(((x) & 0xf) << 12)
#define UART2_SEL(x)		(((x) & 0xf) << 8)
#define UART1_SEL(x)		(((x) & 0xf) << 4)
#define UART0_SEL(x)		((x) & 0xf)

/* Set CLK_DIV_PERIL0 */
#define UART4_RATIO(x)		(((x) & 0xf) << 16)
#define UART3_RATIO(x)		(((x) & 0xf) << 12)
#define UART2_RATIO(x)		(((x) & 0xf) << 8)
#define UART1_RATIO(x)		(((x) & 0xf) << 4)
#define UART0_RATIO(x)		((x) & 0xf)

/* Set CLK_DIV_STAT_PERIL0 */
#define DIV_UART4(x)		(((x) & 0x1) << 16)
#define DIV_UART3(x)		(((x) & 0x1) << 12)
#define DIV_UART2(x)		(((x) & 0x1) << 8)
#define DIV_UART1(x)		(((x) & 0x1) << 4)
#define DIV_UART0(x)		((x) & 0x1)

#define DIV_STAT_PERIL0_CHANGING	(DIV_UART4(DIV_STAT_CHANGING) | \
					DIV_UART3(DIV_STAT_CHANGING) | \
					DIV_UART2(DIV_STAT_CHANGING) | \
					DIV_UART1(DIV_STAT_CHANGING) | \
					DIV_UART0(DIV_STAT_CHANGING))

/* CLK_DIV_FSYS1 */
#define MMC0_RATIO(x)		((x) & 0xf)
#define MMC0_PRE_RATIO(x)	(((x) & 0xff) << 8)
#define MMC1_RATIO(x)		(((x) & 0xf) << 16)
#define MMC1_PRE_RATIO(x)	(((x) & 0xff) << 24)

/* CLK_DIV_STAT_FSYS1 */
#define DIV_MMC0(x)		((x) & 1)
#define DIV_MMC0_PRE(x)		(((x) & 1) << 8)
#define DIV_MMC1(x)		(((x) & 1) << 16)
#define DIV_MMC1_PRE(x)		(((x) & 1) << 24)

#define DIV_STAT_FSYS1_CHANGING		(DIV_MMC0(DIV_STAT_CHANGING) | \
					DIV_MMC0_PRE(DIV_STAT_CHANGING) | \
					DIV_MMC1(DIV_STAT_CHANGING) | \
					DIV_MMC1_PRE(DIV_STAT_CHANGING))

/* CLK_DIV_FSYS2 */
#define MMC2_RATIO(x)		((x) & 0xf)
#define MMC2_PRE_RATIO(x)	(((x) & 0xff) << 8)
#define MMC3_RATIO(x)		(((x) & 0xf) << 16)
#define MMC3_PRE_RATIO(x)	(((x) & 0xff) << 24)

/* CLK_DIV_STAT_FSYS2 */
#define DIV_MMC2(x)		((x) & 0x1)
#define DIV_MMC2_PRE(x)		(((x) & 0x1) << 8)
#define DIV_MMC3(x)		(((x) & 0x1) << 16)
#define DIV_MMC3_PRE(x)		(((x) & 0x1) << 24)

#define DIV_STAT_FSYS2_CHANGING		(DIV_MMC2(DIV_STAT_CHANGING) | \
					DIV_MMC2_PRE(DIV_STAT_CHANGING) | \
					DIV_MMC3(DIV_STAT_CHANGING) | \
					DIV_MMC3_PRE(DIV_STAT_CHANGING))

/* CLK_DIV_FSYS3 */
#define MMC4_RATIO(x)		((x) & 0x7)
#define MMC4_PRE_RATIO(x)	(((x) & 0xff) << 8)

/* CLK_DIV_STAT_FSYS3 */
#define DIV_MMC4(x)		((x) & 0x1)
#define DIV_MMC4_PRE(x)		(((x) & 0x1) << 8)

#define DIV_STAT_FSYS3_CHANGING		(DIV_MMC4(DIV_STAT_CHANGING) | \
					DIV_MMC4_PRE(DIV_STAT_CHANGING))

/* XCL205 GPIO config - Odroid U3 */
#define XCL205_GPIO_BASE		EXYNOS4X12_GPIO_PART1_BASE
#define XCL205_EN_GPIO_OFFSET		0x20 /* GPA1 */
#define XCL205_EN_GPIO_PIN		1
#define XCL205_EN_GPIO_CON		(XCL205_GPIO_BASE + \
					 XCL205_EN_GPIO_OFFSET)
#define XCL205_EN_GPIO_CON_CFG		(S5P_GPIO_OUTPUT << \
					 4 * XCL205_EN_GPIO_PIN)
#define XCL205_EN_GPIO_DAT_CFG		(0x1 << XCL205_EN_GPIO_PIN)
#define XCL205_EN_GPIO_PUD_CFG		(S5P_GPIO_PULL_UP << \
					 2 * XCL205_EN_GPIO_PIN)
#define XCL205_EN_GPIO_DRV_CFG		(S5P_GPIO_DRV_4X << \
					 2 * XCL205_EN_GPIO_PIN)

#define XCL205_STATE_GPIO_OFFSET	0x80 /* GPC1 */
#define XCL205_STATE_GPIO_PIN		2
#define XCL205_STATE_GPIO_CON		(XCL205_GPIO_BASE + \
					 XCL205_STATE_GPIO_OFFSET)
#define XCL205_STATE_GPIO_DAT		XCL205_STATE_GPIO_CON + 0x4
#define XCL205_STATE_GPIO_CON_CFG	(S5P_GPIO_INPUT << \
					4 * XCL205_STATE_GPIO_PIN)
#define XCL205_STATE_GPIO_PUD_CFG	(S5P_GPIO_PULL_NONE << \
					 2 * XCL205_STATE_GPIO_PIN)

#ifdef CONFIG_BOARD_TYPES
extern void sdelay(unsigned long);
#endif

#endif /*__ODROIDU3_SETUP__ */
