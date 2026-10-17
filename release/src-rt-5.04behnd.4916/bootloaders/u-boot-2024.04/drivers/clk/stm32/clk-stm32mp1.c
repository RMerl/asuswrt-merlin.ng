// SPDX-License-Identifier: GPL-2.0+ OR BSD-3-Clause
/*
 * Copyright (C) 2018, STMicroelectronics - All Rights Reserved
 */

#define LOG_CATEGORY UCLASS_CLK

#include <common.h>
#include <clk-uclass.h>
#include <div64.h>
#include <dm.h>
#include <init.h>
#include <log.h>
#include <regmap.h>
#include <spl.h>
#include <syscon.h>
#include <time.h>
#include <vsprintf.h>
#include <asm/arch/sys_proto.h>
#include <asm/global_data.h>
#include <dm/device_compat.h>
#include <dt-bindings/clock/stm32mp1-clks.h>
#include <dt-bindings/clock/stm32mp1-clksrc.h>
#include <linux/bitops.h>
#include <linux/io.h>
#include <linux/iopoll.h>

DECLARE_GLOBAL_DATA_PTR;

#if defined(CONFIG_SPL_BUILD)
/* activate clock tree initialization in the driver */
#define STM32MP1_CLOCK_TREE_INIT
#endif

#define MAX_HSI_HZ		64000000

/* TIMEOUT */
#define TIMEOUT_200MS		200000
#define TIMEOUT_1S		1000000

/* STGEN registers */
#define STGENC_CNTCR		0x00
#define STGENC_CNTSR		0x04
#define STGENC_CNTCVL		0x08
#define STGENC_CNTCVU		0x0C
#define STGENC_CNTFID0		0x20

#define STGENC_CNTCR_EN		BIT(0)

/* RCC registers */
#define RCC_OCENSETR		0x0C
#define RCC_OCENCLRR		0x10
#define RCC_HSICFGR		0x18
#define RCC_MPCKSELR		0x20
#define RCC_ASSCKSELR		0x24
#define RCC_RCK12SELR		0x28
#define RCC_MPCKDIVR		0x2C
#define RCC_AXIDIVR		0x30
#define RCC_APB4DIVR		0x3C
#define RCC_APB5DIVR		0x40
#define RCC_RTCDIVR		0x44
#define RCC_MSSCKSELR		0x48
#define RCC_PLL1CR		0x80
#define RCC_PLL1CFGR1		0x84
#define RCC_PLL1CFGR2		0x88
#define RCC_PLL1FRACR		0x8C
#define RCC_PLL1CSGR		0x90
#define RCC_PLL2CR		0x94
#define RCC_PLL2CFGR1		0x98
#define RCC_PLL2CFGR2		0x9C
#define RCC_PLL2FRACR		0xA0
#define RCC_PLL2CSGR		0xA4
#define RCC_I2C46CKSELR		0xC0
#define RCC_SPI6CKSELR		0xC4
#define RCC_UART1CKSELR		0xC8
#define RCC_CPERCKSELR		0xD0
#define RCC_STGENCKSELR		0xD4
#define RCC_DDRITFCR		0xD8
#define RCC_BDCR		0x140
#define RCC_RDLSICR		0x144
#define RCC_MP_APB4ENSETR	0x200
#define RCC_MP_APB5ENSETR	0x208
#define RCC_MP_AHB5ENSETR	0x210
#define RCC_MP_AHB6ENSETR	0x218
#define RCC_OCRDYR		0x808
#define RCC_DBGCFGR		0x80C
#define RCC_RCK3SELR		0x820
#define RCC_RCK4SELR		0x824
#define RCC_MCUDIVR		0x830
#define RCC_APB1DIVR		0x834
#define RCC_APB2DIVR		0x838
#define RCC_APB3DIVR		0x83C
#define RCC_PLL3CR		0x880
#define RCC_PLL3CFGR1		0x884
#define RCC_PLL3CFGR2		0x888
#define RCC_PLL3FRACR		0x88C
#define RCC_PLL3CSGR		0x890
#define RCC_PLL4CR		0x894
#define RCC_PLL4CFGR1		0x898
#define RCC_PLL4CFGR2		0x89C
#define RCC_PLL4FRACR		0x8A0
#define RCC_PLL4CSGR		0x8A4
#define RCC_I2C12CKSELR		0x8C0
#define RCC_I2C35CKSELR		0x8C4
#define RCC_SPI2S1CKSELR	0x8D8
#define RCC_SPI2S23CKSELR	0x8DC
#define RCC_SPI45CKSELR		0x8E0
#define RCC_UART6CKSELR		0x8E4
#define RCC_UART24CKSELR	0x8E8
#define RCC_UART35CKSELR	0x8EC
#define RCC_UART78CKSELR	0x8F0
#define RCC_SDMMC12CKSELR	0x8F4
#define RCC_SDMMC3CKSELR	0x8F8
#define RCC_ETHCKSELR		0x8FC
#define RCC_QSPICKSELR		0x900
#define RCC_FMCCKSELR		0x904
#define RCC_USBCKSELR		0x91C
#define RCC_DSICKSELR		0x924
#define RCC_ADCCKSELR		0x928
#define RCC_MP_APB1ENSETR	0xA00
#define RCC_MP_APB2ENSETR	0XA08
#define RCC_MP_APB3ENSETR	0xA10
#define RCC_MP_AHB2ENSETR	0xA18
#define RCC_MP_AHB3ENSETR	0xA20
#define RCC_MP_AHB4ENSETR	0xA28

/* used for most of SELR register */
#define RCC_SELR_SRC_MASK	GENMASK(2, 0)
#define RCC_SELR_SRCRDY		BIT(31)

/* Values of RCC_MPCKSELR register */
#define RCC_MPCKSELR_HSI	0
#define RCC_MPCKSELR_HSE	1
#define RCC_MPCKSELR_PLL	2
#define RCC_MPCKSELR_PLL_MPUDIV	3

/* Values of RCC_ASSCKSELR register */
#define RCC_ASSCKSELR_HSI	0
#define RCC_ASSCKSELR_HSE	1
#define RCC_ASSCKSELR_PLL	2

/* Values of RCC_MSSCKSELR register */
#define RCC_MSSCKSELR_HSI	0
#define RCC_MSSCKSELR_HSE	1
#define RCC_MSSCKSELR_CSI	2
#define RCC_MSSCKSELR_PLL	3

/* Values of RCC_CPERCKSELR register */
#define RCC_CPERCKSELR_HSI	0
#define RCC_CPERCKSELR_CSI	1
#define RCC_CPERCKSELR_HSE	2

/* used for most of DIVR register : max div for RTC */
#define RCC_DIVR_DIV_MASK	GENMASK(5, 0)
#define RCC_DIVR_DIVRDY		BIT(31)

/* Masks for specific DIVR registers */
#define RCC_APBXDIV_MASK	GENMASK(2, 0)
#define RCC_MPUDIV_MASK		GENMASK(2, 0)
#define RCC_AXIDIV_MASK		GENMASK(2, 0)
#define RCC_MCUDIV_MASK		GENMASK(3, 0)

/*  offset between RCC_MP_xxxENSETR and RCC_MP_xxxENCLRR registers */
#define RCC_MP_ENCLRR_OFFSET	4

/* Fields of RCC_BDCR register */
#define RCC_BDCR_LSEON		BIT(0)
#define RCC_BDCR_LSEBYP		BIT(1)
#define RCC_BDCR_LSERDY		BIT(2)
#define RCC_BDCR_DIGBYP		BIT(3)
#define RCC_BDCR_LSEDRV_MASK	GENMASK(5, 4)
#define RCC_BDCR_LSEDRV_SHIFT	4
#define RCC_BDCR_LSECSSON	BIT(8)
#define RCC_BDCR_RTCCKEN	BIT(20)
#define RCC_BDCR_RTCSRC_MASK	GENMASK(17, 16)
#define RCC_BDCR_RTCSRC_SHIFT	16

/* Fields of RCC_RDLSICR register */
#define RCC_RDLSICR_LSION	BIT(0)
#define RCC_RDLSICR_LSIRDY	BIT(1)

/* used for ALL PLLNCR registers */
#define RCC_PLLNCR_PLLON	BIT(0)
#define RCC_PLLNCR_PLLRDY	BIT(1)
#define RCC_PLLNCR_SSCG_CTRL	BIT(2)
#define RCC_PLLNCR_DIVPEN	BIT(4)
#define RCC_PLLNCR_DIVQEN	BIT(5)
#define RCC_PLLNCR_DIVREN	BIT(6)
#define RCC_PLLNCR_DIVEN_SHIFT	4

/* used for ALL PLLNCFGR1 registers */
#define RCC_PLLNCFGR1_DIVM_SHIFT	16
#define RCC_PLLNCFGR1_DIVM_MASK		GENMASK(21, 16)
#define RCC_PLLNCFGR1_DIVN_SHIFT	0
#define RCC_PLLNCFGR1_DIVN_MASK		GENMASK(8, 0)
/* only for PLL3 and PLL4 */
#define RCC_PLLNCFGR1_IFRGE_SHIFT	24
#define RCC_PLLNCFGR1_IFRGE_MASK	GENMASK(25, 24)

/* used for ALL PLLNCFGR2 registers , using stm32mp1_div_id */
#define RCC_PLLNCFGR2_SHIFT(div_id)	((div_id) * 8)
#define RCC_PLLNCFGR2_DIVX_MASK		GENMASK(6, 0)
#define RCC_PLLNCFGR2_DIVP_SHIFT	RCC_PLLNCFGR2_SHIFT(_DIV_P)
#define RCC_PLLNCFGR2_DIVP_MASK		GENMASK(6, 0)
#define RCC_PLLNCFGR2_DIVQ_SHIFT	RCC_PLLNCFGR2_SHIFT(_DIV_Q)
#define RCC_PLLNCFGR2_DIVQ_MASK		GENMASK(14, 8)
#define RCC_PLLNCFGR2_DIVR_SHIFT	RCC_PLLNCFGR2_SHIFT(_DIV_R)
#define RCC_PLLNCFGR2_DIVR_MASK		GENMASK(22, 16)

/* used for ALL PLLNFRACR registers */
#define RCC_PLLNFRACR_FRACV_SHIFT	3
#define RCC_PLLNFRACR_FRACV_MASK	GENMASK(15, 3)
#define RCC_PLLNFRACR_FRACLE		BIT(16)

/* used for ALL PLLNCSGR registers */
#define RCC_PLLNCSGR_INC_STEP_SHIFT	16
#define RCC_PLLNCSGR_INC_STEP_MASK	GENMASK(30, 16)
#define RCC_PLLNCSGR_MOD_PER_SHIFT	0
#define RCC_PLLNCSGR_MOD_PER_MASK	GENMASK(12, 0)
#define RCC_PLLNCSGR_SSCG_MODE_SHIFT	15
#define RCC_PLLNCSGR_SSCG_MODE_MASK	BIT(15)

/* used for RCC_OCENSETR and RCC_OCENCLRR registers */
#define RCC_OCENR_HSION			BIT(0)
#define RCC_OCENR_CSION			BIT(4)
#define RCC_OCENR_DIGBYP		BIT(7)
#define RCC_OCENR_HSEON			BIT(8)
#define RCC_OCENR_HSEBYP		BIT(10)
#define RCC_OCENR_HSECSSON		BIT(11)

/* Fields of RCC_OCRDYR register */
#define RCC_OCRDYR_HSIRDY		BIT(0)
#define RCC_OCRDYR_HSIDIVRDY		BIT(2)
#define RCC_OCRDYR_CSIRDY		BIT(4)
#define RCC_OCRDYR_HSERDY		BIT(8)

/* Fields of DDRITFCR register */
#define RCC_DDRITFCR_DDRCKMOD_MASK	GENMASK(22, 20)
#define RCC_DDRITFCR_DDRCKMOD_SHIFT	20
#define RCC_DDRITFCR_DDRCKMOD_SSR	0

/* Fields of RCC_HSICFGR register */
#define RCC_HSICFGR_HSIDIV_MASK		GENMASK(1, 0)

/* used for MCO related operations */
#define RCC_MCOCFG_MCOON		BIT(12)
#define RCC_MCOCFG_MCODIV_MASK		GENMASK(7, 4)
#define RCC_MCOCFG_MCODIV_SHIFT		4
#define RCC_MCOCFG_MCOSRC_MASK		GENMASK(2, 0)

enum stm32mp1_parent_id {
/*
 * _HSI, _HSE, _CSI, _LSI, _LSE should not be moved
 * they are used as index in osc_clk[] as clock reference
 */
	_HSI,
	_HSE,
	_CSI,
	_LSI,
	_LSE,
	_I2S_CKIN,
	NB_OSC,

/* other parent source */
	_HSI_KER = NB_OSC,
	_HSE_KER,
	_HSE_KER_DIV2,
	_CSI_KER,
	_PLL1_P,
	_PLL1_Q,
	_PLL1_R,
	_PLL2_P,
	_PLL2_Q,
	_PLL2_R,
	_PLL3_P,
	_PLL3_Q,
	_PLL3_R,
	_PLL4_P,
	_PLL4_Q,
	_PLL4_R,
	_ACLK,
	_PCLK1,
	_PCLK2,
	_PCLK3,
	_PCLK4,
	_PCLK5,
	_HCLK6,
	_HCLK2,
	_CK_PER,
	_CK_MPU,
	_CK_MCU,
	_DSI_PHY,
	_USB_PHY_48,
	_PARENT_NB,
	_UNKNOWN_ID = 0xff,
};

enum stm32mp1_parent_sel {
	_I2C12_SEL,
	_I2C35_SEL,
	_I2C46_SEL,
	_UART6_SEL,
	_UART24_SEL,
	_UART35_SEL,
	_UART78_SEL,
	_SDMMC12_SEL,
	_SDMMC3_SEL,
	_ETH_SEL,
	_QSPI_SEL,
	_FMC_SEL,
	_USBPHY_SEL,
	_USBO_SEL,
	_STGEN_SEL,
	_DSI_SEL,
	_ADC12_SEL,
	_SPI1_SEL,
	_SPI23_SEL,
	_SPI45_SEL,
	_SPI6_SEL,
	_RTC_SEL,
	_UART1_SEL,
	_PARENT_SEL_NB,
	_UNKNOWN_SEL = 0xff,
};

enum stm32mp1_pll_id {
	_PLL1,
	_PLL2,
	_PLL3,
	_PLL4,
	_PLL_NB
};

enum stm32mp1_div_id {
	_DIV_P,
	_DIV_Q,
	_DIV_R,
	_DIV_NB,
};

enum stm32mp1_clksrc_id {
	CLKSRC_MPU,
	CLKSRC_AXI,
	CLKSRC_MCU,
	CLKSRC_PLL12,
	CLKSRC_PLL3,
	CLKSRC_PLL4,
	CLKSRC_RTC,
	CLKSRC_MCO1,
	CLKSRC_MCO2,
	CLKSRC_NB
};

enum stm32mp1_clkdiv_id {
	CLKDIV_MPU,
	CLKDIV_AXI,
	CLKDIV_MCU,
	CLKDIV_APB1,
	CLKDIV_APB2,
	CLKDIV_APB3,
	CLKDIV_APB4,
	CLKDIV_APB5,
	CLKDIV_RTC,
	CLKDIV_MCO1,
	CLKDIV_MCO2,
	CLKDIV_NB
};

enum stm32mp1_pllcfg {
	PLLCFG_M,
	PLLCFG_N,
	PLLCFG_P,
	PLLCFG_Q,
	PLLCFG_R,
	PLLCFG_O,
	PLLCFG_NB
};

enum stm32mp1_pllcsg {
	PLLCSG_MOD_PER,
	PLLCSG_INC_STEP,
	PLLCSG_SSCG_MODE,
	PLLCSG_NB
};

enum stm32mp1_plltype {
	PLL_800,
	PLL_1600,
	PLL_TYPE_NB
};

struct stm32mp1_pll {
	u8 refclk_min;
	u8 refclk_max;
	u8 divn_max;
};

struct stm32mp1_clk_gate {
	u16 offset;
	u8 bit;
	u8 index;
	u8 set_clr;
	u8 sel;
	u8 fixed;
};

struct stm32mp1_clk_sel {
	u16 offset;
	u8 src;
	u8 msk;
	u8 nb_parent;
	const u8 *parent;
};

#define REFCLK_SIZE 4
struct stm32mp1_clk_pll {
	enum stm32mp1_plltype plltype;
	u16 rckxselr;
	u16 pllxcfgr1;
	u16 pllxcfgr2;
	u16 pllxfracr;
	u16 pllxcr;
	u16 pllxcsgr;
	u8 refclk[REFCLK_SIZE];
};

struct stm32mp1_clk_data {
	const struct stm32mp1_clk_gate *gate;
	const struct stm32mp1_clk_sel *sel;
	const struct stm32mp1_clk_pll *pll;
	const int nb_gate;
};

struct stm32mp1_clk_priv {
	fdt_addr_t base;
	const struct stm32mp1_clk_data *data;
	struct clk osc_clk[NB_OSC];
};

#define STM32MP1_CLK(off, b, idx, s)		\
	{					\
		.offset = (off),		\
		.bit = (b),			\
		.index = (idx),			\
		.set_clr = 0,			\
		.sel = (s),			\
		.fixed = _UNKNOWN_ID,		\
	}

#define STM32MP1_CLK_F(off, b, idx, f)		\
	{					\
		.offset = (off),		\
		.bit = (b),			\
		.index = (idx),			\
		.set_clr = 0,			\
		.sel = _UNKNOWN_SEL,		\
		.fixed = (f),			\
	}

#define STM32MP1_CLK_SET_CLR(off, b, idx, s)	\
	{					\
		.offset = (off),		\
		.bit = (b),			\
		.index = (idx),			\
		.set_clr = 1,			\
		.sel = (s),			\
		.fixed = _UNKNOWN_ID,		\
	}

#define STM32MP1_CLK_SET_CLR_F(off, b, idx, f)	\
	{					\
		.offset = (off),		\
		.bit = (b),			\
		.index = (idx),			\
		.set_clr = 1,			\
		.sel = _UNKNOWN_SEL,		\
		.fixed = (f),			\
	}

#define STM32MP1_CLK_PARENT(idx, off, s, m, p)   \
	[(idx)] = {				\
		.offset = (off),		\
		.src = (s),			\
		.msk = (m),			\
		.parent = (p),			\
		.nb_parent = ARRAY_SIZE((p))	\
	}

#define STM32MP1_CLK_PLL(idx, type, off1, off2, off3, off4, off5, off6,\
			p1, p2, p3, p4) \
	[(idx)] = {				\
		.plltype = (type),			\
		.rckxselr = (off1),		\
		.pllxcfgr1 = (off2),		\
		.pllxcfgr2 = (off3),		\
		.pllxfracr = (off4),		\
		.pllxcr = (off5),		\
		.pllxcsgr = (off6),		\
		.refclk[0] = (p1),		\
		.refclk[1] = (p2),		\
		.refclk[2] = (p3),		\
		.refclk[3] = (p4),		\
	}

static const u8 stm32mp1_clks[][2] = {
	{CK_PER, _CK_PER},
	{CK_MPU, _CK_MPU},
	{CK_AXI, _ACLK},
	{CK_MCU, _CK_MCU},
	{CK_HSE, _HSE},
	{CK_CSI, _CSI},
	{CK_LSI, _LSI},
	{CK_LSE, _LSE},
	{CK_HSI, _HSI},
	{CK_HSE_DIV2, _HSE_KER_DIV2},
};

static const struct stm32mp1_clk_gate stm32mp1_clk_gate[] = {
	STM32MP1_CLK(RCC_DDRITFCR, 0, DDRC1, _UNKNOWN_SEL),
	STM32MP1_CLK(RCC_DDRITFCR, 1, DDRC1LP, _UNKNOWN_SEL),
	STM32MP1_CLK(RCC_DDRITFCR, 2, DDRC2, _UNKNOWN_SEL),
	STM32MP1_CLK(RCC_DDRITFCR, 3, DDRC2LP, _UNKNOWN_SEL),
	STM32MP1_CLK_F(RCC_DDRITFCR, 4, DDRPHYC, _PLL2_R),
	STM32MP1_CLK(RCC_DDRITFCR, 5, DDRPHYCLP, _UNKNOWN_SEL),
	STM32MP1_CLK(RCC_DDRITFCR, 6, DDRCAPB, _UNKNOWN_SEL),
	STM32MP1_CLK(RCC_DDRITFCR, 7, DDRCAPBLP, _UNKNOWN_SEL),
	STM32MP1_CLK(RCC_DDRITFCR, 8, AXIDCG, _UNKNOWN_SEL),
	STM32MP1_CLK(RCC_DDRITFCR, 9, DDRPHYCAPB, _UNKNOWN_SEL),
	STM32MP1_CLK(RCC_DDRITFCR, 10, DDRPHYCAPBLP, _UNKNOWN_SEL),

	STM32MP1_CLK_SET_CLR(RCC_MP_APB1ENSETR, 11, SPI2_K, _SPI23_SEL),
	STM32MP1_CLK_SET_CLR(RCC_MP_APB1ENSETR, 12, SPI3_K, _SPI23_SEL),
	STM32MP1_CLK_SET_CLR(RCC_MP_APB1ENSETR, 14, USART2_K, _UART24_SEL),
	STM32MP1_CLK_SET_CLR(RCC_MP_APB1ENSETR, 15, USART3_K, _UART35_SEL),
	STM32MP1_CLK_SET_CLR(RCC_MP_APB1ENSETR, 16, UART4_K, _UART24_SEL),
	STM32MP1_CLK_SET_CLR(RCC_MP_APB1ENSETR, 17, UART5_K, _UART35_SEL),
	STM32MP1_CLK_SET_CLR(RCC_MP_APB1ENSETR, 18, UART7_K, _UART78_SEL),
	STM32MP1_CLK_SET_CLR(RCC_MP_APB1ENSETR, 19, UART8_K, _UART78_SEL),
	STM32MP1_CLK_SET_CLR(RCC_MP_APB1ENSETR, 21, I2C1_K, _I2C12_SEL),
	STM32MP1_CLK_SET_CLR(RCC_MP_APB1ENSETR, 22, I2C2_K, _I2C12_SEL),
	STM32MP1_CLK_SET_CLR(RCC_MP_APB1ENSETR, 23, I2C3_K, _I2C35_SEL),
	STM32MP1_CLK_SET_CLR(RCC_MP_APB1ENSETR, 24, I2C5_K, _I2C35_SEL),

	STM32MP1_CLK_SET_CLR(RCC_MP_APB2ENSETR, 8, SPI1_K, _SPI1_SEL),
	STM32MP1_CLK_SET_CLR(RCC_MP_APB2ENSETR, 9, SPI4_K, _SPI45_SEL),
	STM32MP1_CLK_SET_CLR(RCC_MP_APB2ENSETR, 10, SPI5_K, _SPI45_SEL),
	STM32MP1_CLK_SET_CLR(RCC_MP_APB2ENSETR, 13, USART6_K, _UART6_SEL),

	STM32MP1_CLK_SET_CLR_F(RCC_MP_APB3ENSETR, 13, VREF, _PCLK3),
	STM32MP1_CLK_SET_CLR_F(RCC_MP_APB3ENSETR, 11, SYSCFG, _UNKNOWN_SEL),

	STM32MP1_CLK_SET_CLR_F(RCC_MP_APB4ENSETR, 0, LTDC_PX, _PLL4_Q),
	STM32MP1_CLK_SET_CLR_F(RCC_MP_APB4ENSETR, 4, DSI_PX, _PLL4_Q),
	STM32MP1_CLK_SET_CLR(RCC_MP_APB4ENSETR, 4, DSI_K, _DSI_SEL),
	STM32MP1_CLK_SET_CLR(RCC_MP_APB4ENSETR, 8, DDRPERFM, _UNKNOWN_SEL),
	STM32MP1_CLK_SET_CLR(RCC_MP_APB4ENSETR, 15, IWDG2, _UNKNOWN_SEL),
	STM32MP1_CLK_SET_CLR(RCC_MP_APB4ENSETR, 16, USBPHY_K, _USBPHY_SEL),

	STM32MP1_CLK_SET_CLR(RCC_MP_APB5ENSETR, 0, SPI6_K, _SPI6_SEL),
	STM32MP1_CLK_SET_CLR(RCC_MP_APB5ENSETR, 2, I2C4_K, _I2C46_SEL),
	STM32MP1_CLK_SET_CLR(RCC_MP_APB5ENSETR, 3, I2C6_K, _I2C46_SEL),
	STM32MP1_CLK_SET_CLR(RCC_MP_APB5ENSETR, 4, USART1_K, _UART1_SEL),
	STM32MP1_CLK_SET_CLR(RCC_MP_APB5ENSETR, 8, RTCAPB, _PCLK5),
	STM32MP1_CLK_SET_CLR(RCC_MP_APB5ENSETR, 16, BSEC, _UNKNOWN_SEL),
	STM32MP1_CLK_SET_CLR(RCC_MP_APB5ENSETR, 20, STGEN_K, _STGEN_SEL),

	STM32MP1_CLK_SET_CLR_F(RCC_MP_AHB2ENSETR, 5, ADC12, _HCLK2),
	STM32MP1_CLK_SET_CLR(RCC_MP_AHB2ENSETR, 5, ADC12_K, _ADC12_SEL),
	STM32MP1_CLK_SET_CLR(RCC_MP_AHB2ENSETR, 8, USBO_K, _USBO_SEL),
	STM32MP1_CLK_SET_CLR(RCC_MP_AHB2ENSETR, 16, SDMMC3_K, _SDMMC3_SEL),

	STM32MP1_CLK_SET_CLR(RCC_MP_AHB3ENSETR, 11, HSEM, _UNKNOWN_SEL),
	STM32MP1_CLK_SET_CLR(RCC_MP_AHB3ENSETR, 12, IPCC, _UNKNOWN_SEL),

	STM32MP1_CLK_SET_CLR(RCC_MP_AHB4ENSETR, 0, GPIOA, _UNKNOWN_SEL),
	STM32MP1_CLK_SET_CLR(RCC_MP_AHB4ENSETR, 1, GPIOB, _UNKNOWN_SEL),
	STM32MP1_CLK_SET_CLR(RCC_MP_AHB4ENSETR, 2, GPIOC, _UNKNOWN_SEL),
	STM32MP1_CLK_SET_CLR(RCC_MP_AHB4ENSETR, 3, GPIOD, _UNKNOWN_SEL),
	STM32MP1_CLK_SET_CLR(RCC_MP_AHB4ENSETR, 4, GPIOE, _UNKNOWN_SEL),
	STM32MP1_CLK_SET_CLR(RCC_MP_AHB4ENSETR, 5, GPIOF, _UNKNOWN_SEL),
	STM32MP1_CLK_SET_CLR(RCC_MP_AHB4ENSETR, 6, GPIOG, _UNKNOWN_SEL),
	STM32MP1_CLK_SET_CLR(RCC_MP_AHB4ENSETR, 7, GPIOH, _UNKNOWN_SEL),
	STM32MP1_CLK_SET_CLR(RCC_MP_AHB4ENSETR, 8, GPIOI, _UNKNOWN_SEL),
	STM32MP1_CLK_SET_CLR(RCC_MP_AHB4ENSETR, 9, GPIOJ, _UNKNOWN_SEL),
	STM32MP1_CLK_SET_CLR(RCC_MP_AHB4ENSETR, 10, GPIOK, _UNKNOWN_SEL),

	STM32MP1_CLK_SET_CLR(RCC_MP_AHB5ENSETR, 0, GPIOZ, _UNKNOWN_SEL),
	STM32MP1_CLK_SET_CLR(RCC_MP_AHB5ENSETR, 6, RNG1_K, _UNKNOWN_SEL),

	STM32MP1_CLK_SET_CLR(RCC_MP_AHB6ENSETR, 7, ETHCK_K, _ETH_SEL),
	STM32MP1_CLK_SET_CLR(RCC_MP_AHB6ENSETR, 8, ETHTX, _UNKNOWN_SEL),
	STM32MP1_CLK_SET_CLR(RCC_MP_AHB6ENSETR, 9, ETHRX, _UNKNOWN_SEL),
	STM32MP1_CLK_SET_CLR_F(RCC_MP_AHB6ENSETR, 10, ETHMAC, _ACLK),
	STM32MP1_CLK_SET_CLR(RCC_MP_AHB6ENSETR, 12, FMC_K, _FMC_SEL),
	STM32MP1_CLK_SET_CLR(RCC_MP_AHB6ENSETR, 14, QSPI_K, _QSPI_SEL),
	STM32MP1_CLK_SET_CLR(RCC_MP_AHB6ENSETR, 16, SDMMC1_K, _SDMMC12_SEL),
	STM32MP1_CLK_SET_CLR(RCC_MP_AHB6ENSETR, 17, SDMMC2_K, _SDMMC12_SEL),
	STM32MP1_CLK_SET_CLR(RCC_MP_AHB6ENSETR, 24, USBH, _UNKNOWN_SEL),

	STM32MP1_CLK(RCC_DBGCFGR, 8, CK_DBG, _UNKNOWN_SEL),

	STM32MP1_CLK(RCC_BDCR, 20, RTC, _RTC_SEL),
};

static const u8 i2c12_parents[] = {_PCLK1, _PLL4_R, _HSI_KER, _CSI_KER};
static const u8 i2c35_parents[] = {_PCLK1, _PLL4_R, _HSI_KER, _CSI_KER};
static const u8 i2c46_parents[] = {_PCLK5, _PLL3_Q, _HSI_KER, _CSI_KER};
static const u8 uart1_parents[] = {_PCLK5, _PLL3_Q, _HSI_KER, _CSI_KER,
					_PLL4_Q, _HSE_KER};
static const u8 uart6_parents[] = {_PCLK2, _PLL4_Q, _HSI_KER, _CSI_KER,
					_HSE_KER};
static const u8 uart24_parents[] = {_PCLK1, _PLL4_Q, _HSI_KER, _CSI_KER,
					 _HSE_KER};
static const u8 uart35_parents[] = {_PCLK1, _PLL4_Q, _HSI_KER, _CSI_KER,
					 _HSE_KER};
static const u8 uart78_parents[] = {_PCLK1, _PLL4_Q, _HSI_KER, _CSI_KER,
					 _HSE_KER};
static const u8 sdmmc12_parents[] = {_HCLK6, _PLL3_R, _PLL4_P, _HSI_KER};
static const u8 sdmmc3_parents[] = {_HCLK2, _PLL3_R, _PLL4_P, _HSI_KER};
static const u8 eth_parents[] = {_PLL4_P, _PLL3_Q};
static const u8 qspi_parents[] = {_ACLK, _PLL3_R, _PLL4_P, _CK_PER};
static const u8 fmc_parents[] = {_ACLK, _PLL3_R, _PLL4_P, _CK_PER};
static const u8 usbphy_parents[] = {_HSE_KER, _PLL4_R, _HSE_KER_DIV2};
static const u8 usbo_parents[] = {_PLL4_R, _USB_PHY_48};
static const u8 stgen_parents[] = {_HSI_KER, _HSE_KER};
static const u8 dsi_parents[] = {_DSI_PHY, _PLL4_P};
static const u8 adc_parents[] = {_PLL4_R, _CK_PER, _PLL3_Q};
/* same parents for SPI1=RCC_SPI2S1CKSELR and SPI2&3 = RCC_SPI2S23CKSELR */
static const u8 spi_parents[] = {_PLL4_P, _PLL3_Q, _I2S_CKIN, _CK_PER,
				 _PLL3_R};
static const u8 spi45_parents[] = {_PCLK2, _PLL4_Q, _HSI_KER, _CSI_KER,
				   _HSE_KER};
static const u8 spi6_parents[] = {_PCLK5, _PLL4_Q, _HSI_KER, _CSI_KER,
				  _HSE_KER, _PLL3_Q};
static const u8 rtc_parents[] = {_UNKNOWN_ID, _LSE, _LSI, _HSE};

static const struct stm32mp1_clk_sel stm32mp1_clk_sel[_PARENT_SEL_NB] = {
	STM32MP1_CLK_PARENT(_I2C12_SEL, RCC_I2C12CKSELR, 0, 0x7, i2c12_parents),
	STM32MP1_CLK_PARENT(_I2C35_SEL, RCC_I2C35CKSELR, 0, 0x7, i2c35_parents),
	STM32MP1_CLK_PARENT(_I2C46_SEL, RCC_I2C46CKSELR, 0, 0x7, i2c46_parents),
	STM32MP1_CLK_PARENT(_UART6_SEL, RCC_UART6CKSELR, 0, 0x7, uart6_parents),
	STM32MP1_CLK_PARENT(_UART24_SEL, RCC_UART24CKSELR, 0, 0x7,
			    uart24_parents),
	STM32MP1_CLK_PARENT(_UART35_SEL, RCC_UART35CKSELR, 0, 0x7,
			    uart35_parents),
	STM32MP1_CLK_PARENT(_UART78_SEL, RCC_UART78CKSELR, 0, 0x7,
			    uart78_parents),
	STM32MP1_CLK_PARENT(_SDMMC12_SEL, RCC_SDMMC12CKSELR, 0, 0x7,
			    sdmmc12_parents),
	STM32MP1_CLK_PARENT(_SDMMC3_SEL, RCC_SDMMC3CKSELR, 0, 0x7,
			    sdmmc3_parents),
	STM32MP1_CLK_PARENT(_ETH_SEL, RCC_ETHCKSELR, 0, 0x3, eth_parents),
	STM32MP1_CLK_PARENT(_QSPI_SEL, RCC_QSPICKSELR, 0, 0x3, qspi_parents),
	STM32MP1_CLK_PARENT(_FMC_SEL, RCC_FMCCKSELR, 0, 0x3, fmc_parents),
	STM32MP1_CLK_PARENT(_USBPHY_SEL, RCC_USBCKSELR, 0, 0x3, usbphy_parents),
	STM32MP1_CLK_PARENT(_USBO_SEL, RCC_USBCKSELR, 4, 0x1, usbo_parents),
	STM32MP1_CLK_PARENT(_STGEN_SEL, RCC_STGENCKSELR, 0, 0x3, stgen_parents),
	STM32MP1_CLK_PARENT(_DSI_SEL, RCC_DSICKSELR, 0, 0x1, dsi_parents),
	STM32MP1_CLK_PARENT(_ADC12_SEL, RCC_ADCCKSELR, 0, 0x3, adc_parents),
	STM32MP1_CLK_PARENT(_SPI1_SEL, RCC_SPI2S1CKSELR, 0, 0x7, spi_parents),
	STM32MP1_CLK_PARENT(_SPI23_SEL, RCC_SPI2S23CKSELR, 0, 0x7, spi_parents),
	STM32MP1_CLK_PARENT(_SPI45_SEL, RCC_SPI45CKSELR, 0, 0x7, spi45_parents),
	STM32MP1_CLK_PARENT(_SPI6_SEL, RCC_SPI6CKSELR, 0, 0x7, spi6_parents),
	STM32MP1_CLK_PARENT(_RTC_SEL, RCC_BDCR, RCC_BDCR_RTCSRC_SHIFT,
			    (RCC_BDCR_RTCSRC_MASK >> RCC_BDCR_RTCSRC_SHIFT),
			    rtc_parents),
	STM32MP1_CLK_PARENT(_UART1_SEL, RCC_UART1CKSELR, 0, 0x7, uart1_parents),
};

#ifdef STM32MP1_CLOCK_TREE_INIT

/* define characteristic of PLL according type */
#define DIVM_MIN	0
#define DIVM_MAX	63
#define DIVN_MIN	24
#define DIVP_MIN	0
#define DIVP_MAX	127
#define FRAC_MAX	8192

#define PLL1600_VCO_MIN	800000000
#define PLL1600_VCO_MAX	1600000000

static const struct stm32mp1_pll stm32mp1_pll[PLL_TYPE_NB] = {
	[PLL_800] = {
		.refclk_min = 4,
		.refclk_max = 16,
		.divn_max = 99,
		},
	[PLL_1600] = {
		.refclk_min = 8,
		.refclk_max = 16,
		.divn_max = 199,
		},
};
#endif /* STM32MP1_CLOCK_TREE_INIT */

static const struct stm32mp1_clk_pll stm32mp1_clk_pll[_PLL_NB] = {
	STM32MP1_CLK_PLL(_PLL1, PLL_1600,
			 RCC_RCK12SELR, RCC_PLL1CFGR1, RCC_PLL1CFGR2,
			 RCC_PLL1FRACR, RCC_PLL1CR, RCC_PLL1CSGR,
			 _HSI, _HSE, _UNKNOWN_ID, _UNKNOWN_ID),
	STM32MP1_CLK_PLL(_PLL2, PLL_1600,
			 RCC_RCK12SELR, RCC_PLL2CFGR1, RCC_PLL2CFGR2,
			 RCC_PLL2FRACR, RCC_PLL2CR, RCC_PLL2CSGR,
			 _HSI, _HSE, _UNKNOWN_ID, _UNKNOWN_ID),
	STM32MP1_CLK_PLL(_PLL3, PLL_800,
			 RCC_RCK3SELR, RCC_PLL3CFGR1, RCC_PLL3CFGR2,
			 RCC_PLL3FRACR, RCC_PLL3CR, RCC_PLL3CSGR,
			 _HSI, _HSE, _CSI, _UNKNOWN_ID),
	STM32MP1_CLK_PLL(_PLL4, PLL_800,
			 RCC_RCK4SELR, RCC_PLL4CFGR1, RCC_PLL4CFGR2,
			 RCC_PLL4FRACR, RCC_PLL4CR, RCC_PLL4CSGR,
			 _HSI, _HSE, _CSI, _I2S_CKIN),
};

/* Prescaler table lookups for clock computation */
/* div = /1 /2 /4 /8 / 16 /64 /128 /512 */
static const u8 stm32mp1_mcu_div[16] = {
	0, 1, 2, 3, 4, 6, 7, 8, 9, 9, 9, 9, 9, 9, 9, 9
};

/* div = /1 /2 /4 /8 /16 : same divider for pmu and apbx*/
#define stm32mp1_mpu_div stm32mp1_mpu_apbx_div
#define stm32mp1_apbx_div stm32mp1_mpu_apbx_div
static const u8 stm32mp1_mpu_apbx_div[8] = {
	0, 1, 2, 3, 4, 4, 4, 4
};

/* div = /1 /2 /3 /4 */
static const u8 stm32mp1_axi_div[8] = {
	1, 2, 3, 4, 4, 4, 4, 4
};

static const __maybe_unused
char * const stm32mp1_clk_parent_name[_PARENT_NB] = {
	[_HSI] = "HSI",
	[_HSE] = "HSE",
	[_CSI] = "CSI",
	[_LSI] = "LSI",
	[_LSE] = "LSE",
	[_I2S_CKIN] = "I2S_CKIN",
	[_HSI_KER] = "HSI_KER",
	[_HSE_KER] = "HSE_KER",
	[_HSE_KER_DIV2] = "HSE_KER_DIV2",
	[_CSI_KER] = "CSI_KER",
	[_PLL1_P] = "PLL1_P",
	[_PLL1_Q] = "PLL1_Q",
	[_PLL1_R] = "PLL1_R",
	[_PLL2_P] = "PLL2_P",
	[_PLL2_Q] = "PLL2_Q",
	[_PLL2_R] = "PLL2_R",
	[_PLL3_P] = "PLL3_P",
	[_PLL3_Q] = "PLL3_Q",
	[_PLL3_R] = "PLL3_R",
	[_PLL4_P] = "PLL4_P",
	[_PLL4_Q] = "PLL4_Q",
	[_PLL4_R] = "PLL4_R",
	[_ACLK] = "ACLK",
	[_PCLK1] = "PCLK1",
	[_PCLK2] = "PCLK2",
	[_PCLK3] = "PCLK3",
	[_PCLK4] = "PCLK4",
	[_PCLK5] = "PCLK5",
	[_HCLK6] = "KCLK6",
	[_HCLK2] = "HCLK2",
	[_CK_PER] = "CK_PER",
	[_CK_MPU] = "CK_MPU",
	[_CK_MCU] = "CK_MCU",
	[_USB_PHY_48] = "USB_PHY_48",
	[_DSI_PHY] = "DSI_PHY_PLL",
};

static const __maybe_unused
char * const stm32mp1_clk_parent_sel_name[_PARENT_SEL_NB] = {
	[_I2C12_SEL] = "I2C12",
	[_I2C35_SEL] = "I2C35",
	[_I2C46_SEL] = "I2C46",
	[_UART6_SEL] = "UART6",
	[_UART24_SEL] = "UART24",
	[_UART35_SEL] = "UART35",
	[_UART78_SEL] = "UART78",
	[_SDMMC12_SEL] = "SDMMC12",
	[_SDMMC3_SEL] = "SDMMC3",
	[_ETH_SEL] = "ETH",
	[_QSPI_SEL] = "QSPI",
	[_FMC_SEL] = "FMC",
	[_USBPHY_SEL] = "USBPHY",
	[_USBO_SEL] = "USBO",
	[_STGEN_SEL] = "STGEN",
	[_DSI_SEL] = "DSI",
	[_ADC12_SEL] = "ADC12",
	[_SPI1_SEL] = "SPI1",
	[_SPI45_SEL] = "SPI45",
	[_RTC_SEL] = "RTC",
	[_UART1_SEL] = "UART1",
};

static const struct stm32mp1_clk_data stm32mp1_data = {
	.gate = stm32mp1_clk_gate,
	.sel = stm32mp1_clk_sel,
	.pll = stm32mp1_clk_pll,
	.nb_gate = ARRAY_SIZE(stm32mp1_clk_gate),
};

static ulong stm32mp1_clk_get_fixed(struct stm32mp1_clk_priv *priv, int idx)
{
	if (idx >= NB_OSC) {
		log_debug("clk id %d not found\n", idx);
		return 0;
	}

	return clk_get_rate(&priv->osc_clk[idx]);
}

static int stm32mp1_clk_get_id(struct stm32mp1_clk_priv *priv, unsigned long id)
{
	const struct stm32mp1_clk_gate *gate = priv->data->gate;
	int i, nb_clks = priv->data->nb_gate;

	for (i = 0; i < nb_clks; i++) {
		if (gate[i].index == id)
			break;
	}

	if (i == nb_clks) {
		log_err("clk id %d not found\n", (u32)id);
		return -EINVAL;
	}

	return i;
}

static int stm32mp1_clk_get_sel(struct stm32mp1_clk_priv *priv,
				int i)
{
	const struct stm32mp1_clk_gate *gate = priv->data->gate;

	if (gate[i].sel > _PARENT_SEL_NB) {
		log_err("parents for clk id %d not found\n", i);
		return -EINVAL;
	}

	return gate[i].sel;
}

static int stm32mp1_clk_get_fixed_parent(struct stm32mp1_clk_priv *priv,
					 int i)
{
	const struct stm32mp1_clk_gate *gate = priv->data->gate;

	if (gate[i].fixed == _UNKNOWN_ID)
		return -ENOENT;

	return gate[i].fixed;
}

static int stm32mp1_clk_get_parent(struct stm32mp1_clk_priv *priv,
				   unsigned long id)
{
	const struct stm32mp1_clk_sel *sel = priv->data->sel;
	int i;
	int s, p;
	unsigned int idx;

	for (idx = 0; idx < ARRAY_SIZE(stm32mp1_clks); idx++)
		if (stm32mp1_clks[idx][0] == id)
			return stm32mp1_clks[idx][1];

	i = stm32mp1_clk_get_id(priv, id);
	if (i < 0)
		return i;

	p = stm32mp1_clk_get_fixed_parent(priv, i);
	if (p >= 0 && p < _PARENT_NB)
		return p;

	s = stm32mp1_clk_get_sel(priv, i);
	if (s < 0)
		return s;

	p = (readl(priv->base + sel[s].offset) >> sel[s].src) & sel[s].msk;

	if (p < sel[s].nb_parent) {
		log_content("%s clock is the parent %s of clk id %d\n",
			    stm32mp1_clk_parent_name[sel[s].parent[p]],
			    stm32mp1_clk_parent_sel_name[s],
			    (u32)id);
		return sel[s].parent[p];
	}

	/* clock is DISABLED when the clock src is not in clk_parent[] range */
	log_debug("no parents defined for clk id %d\n", (u32)id);

	return -EINVAL;
}

static ulong  pll_get_fref_ck(struct stm32mp1_clk_priv *priv,
			      int pll_id)
{
	const struct stm32mp1_clk_pll *pll = priv->data->pll;
	u32 selr;
	int src;
	ulong refclk;

	/* Get current refclk */
	selr = readl(priv->base + pll[pll_id].rckxselr);
	src = selr & RCC_SELR_SRC_MASK;

	refclk = stm32mp1_clk_get_fixed(priv, pll[pll_id].refclk[src]);

	return refclk;
}

/*
 * pll_get_fvco() : return the VCO or (VCO / 2) frequency for the requested PLL
 * - PLL1 & PLL2 => return VCO / 2 with Fpll_y_ck = FVCO / 2 * (DIVy + 1)
 * - PLL3 & PLL4 => return VCO     with Fpll_y_ck = FVCO / (DIVy + 1)
 * => in all the case Fpll_y_ck = pll_get_fvco() / (DIVy + 1)
 */
static ulong pll_get_fvco(struct stm32mp1_clk_priv *priv,
			  int pll_id)
{
	const struct stm32mp1_clk_pll *pll = priv->data->pll;
	int divm, divn;
	ulong refclk, fvco;
	u32 cfgr1, fracr;

	cfgr1 = readl(priv->base + pll[pll_id].pllxcfgr1);
	fracr = readl(priv->base + pll[pll_id].pllxfracr);

	divm = (cfgr1 & (RCC_PLLNCFGR1_DIVM_MASK)) >> RCC_PLLNCFGR1_DIVM_SHIFT;
	divn = cfgr1 & RCC_PLLNCFGR1_DIVN_MASK;

	refclk = pll_get_fref_ck(priv, pll_id);

	/* with FRACV :
	 *   Fvco = Fck_ref * ((DIVN + 1) + FRACV / 2^13) / (DIVM + 1)
	 * without FRACV
	 *   Fvco = Fck_ref * ((DIVN + 1) / (DIVM + 1)
	 */
	if (fracr & RCC_PLLNFRACR_FRACLE) {
		u32 fracv = (fracr & RCC_PLLNFRACR_FRACV_MASK)
			    >> RCC_PLLNFRACR_FRACV_SHIFT;
		fvco = (ulong)lldiv((unsigned long long)refclk *
				     (((divn + 1) << 13) + fracv),
				     ((unsigned long long)(divm + 1)) << 13);
	} else {
		fvco = (ulong)(refclk * (divn + 1) / (divm + 1));
	}

	return fvco;
}

static ulong stm32mp1_read_pll_freq(struct stm32mp1_clk_priv *priv,
				    int pll_id, int div_id)
{
	const struct stm32mp1_clk_pll *pll = priv->data->pll;
	int divy;
	ulong dfout;
	u32 cfgr2;

	if (div_id >= _DIV_NB)
		return 0;

	cfgr2 = readl(priv->base + pll[pll_id].pllxcfgr2);
	divy = (cfgr2 >> RCC_PLLNCFGR2_SHIFT(div_id)) & RCC_PLLNCFGR2_DIVX_MASK;

	dfout = pll_get_fvco(priv, pll_id) / (divy + 1);

	return dfout;
}

static ulong stm32mp1_clk_get_by_name(const char *name)
{
	struct clk clk;
	struct udevice *dev = NULL;
	ulong clock = 0;

	if (!uclass_get_device_by_name(UCLASS_CLK, name, &dev)) {
		if (clk_request(dev, &clk)) {
			log_err("%s request", name);
		} else {
			clk.id = 0;
			clock = clk_get_rate(&clk);
		}
	}

	return clock;
}

static ulong stm32mp1_clk_get(struct stm32mp1_clk_priv *priv, int p)
{
	u32 reg;
	ulong clock = 0;

	switch (p) {
	case _CK_MPU:
	/* MPU sub system */
		reg = readl(priv->base + RCC_MPCKSELR);
		switch (reg & RCC_SELR_SRC_MASK) {
		case RCC_MPCKSELR_HSI:
			clock = stm32mp1_clk_get_fixed(priv, _HSI);
			break;
		case RCC_MPCKSELR_HSE:
			clock = stm32mp1_clk_get_fixed(priv, _HSE);
			break;
		case RCC_MPCKSELR_PLL:
		case RCC_MPCKSELR_PLL_MPUDIV:
			clock = stm32mp1_read_pll_freq(priv, _PLL1, _DIV_P);
			if ((reg & RCC_SELR_SRC_MASK) ==
			    RCC_MPCKSELR_PLL_MPUDIV) {
				reg = readl(priv->base + RCC_MPCKDIVR);
				clock >>= stm32mp1_mpu_div[reg &
					RCC_MPUDIV_MASK];
			}
			break;
		}
		break;
	/* AXI sub system */
	case _ACLK:
	case _HCLK2:
	case _HCLK6:
	case _PCLK4:
	case _PCLK5:
		reg = readl(priv->base + RCC_ASSCKSELR);
		switch (reg & RCC_SELR_SRC_MASK) {
		case RCC_ASSCKSELR_HSI:
			clock = stm32mp1_clk_get_fixed(priv, _HSI);
			break;
		case RCC_ASSCKSELR_HSE:
			clock = stm32mp1_clk_get_fixed(priv, _HSE);
			break;
		case RCC_ASSCKSELR_PLL:
			clock = stm32mp1_read_pll_freq(priv, _PLL2, _DIV_P);
			break;
		}

		/* System clock divider */
		reg = readl(priv->base + RCC_AXIDIVR);
		clock /= stm32mp1_axi_div[reg & RCC_AXIDIV_MASK];

		switch (p) {
		case _PCLK4:
			reg = readl(priv->base + RCC_APB4DIVR);
			clock >>= stm32mp1_apbx_div[reg & RCC_APBXDIV_MASK];
			break;
		case _PCLK5:
			reg = readl(priv->base + RCC_APB5DIVR);
			clock >>= stm32mp1_apbx_div[reg & RCC_APBXDIV_MASK];
			break;
		default:
			break;
		}
		break;
	/* MCU sub system */
	case _CK_MCU:
	case _PCLK1:
	case _PCLK2:
	case _PCLK3:
		reg = readl(priv->base + RCC_MSSCKSELR);
		switch (reg & RCC_SELR_SRC_MASK) {
		case RCC_MSSCKSELR_HSI:
			clock = stm32mp1_clk_get_fixed(priv, _HSI);
			break;
		case RCC_MSSCKSELR_HSE:
			clock = stm32mp1_clk_get_fixed(priv, _HSE);
			break;
		case RCC_MSSCKSELR_CSI:
			clock = stm32mp1_clk_get_fixed(priv, _CSI);
			break;
		case RCC_MSSCKSELR_PLL:
			clock = stm32mp1_read_pll_freq(priv, _PLL3, _DIV_P);
			break;
		}

		/* MCU clock divider */
		reg = readl(priv->base + RCC_MCUDIVR);
		clock >>= stm32mp1_mcu_div[reg & RCC_MCUDIV_MASK];

		switch (p) {
		case _PCLK1:
			reg = readl(priv->base + RCC_APB1DIVR);
			clock >>= stm32mp1_apbx_div[reg & RCC_APBXDIV_MASK];
			break;
		case _PCLK2:
			reg = readl(priv->base + RCC_APB2DIVR);
			clock >>= stm32mp1_apbx_div[reg & RCC_APBXDIV_MASK];
			break;
		case _PCLK3:
			reg = readl(priv->base + RCC_APB3DIVR);
			clock >>= stm32mp1_apbx_div[reg & RCC_APBXDIV_MASK];
			break;
		case _CK_MCU:
		default:
			break;
		}
		break;
	case _CK_PER:
		reg = readl(priv->base + RCC_CPERCKSELR);
		switch (reg & RCC_SELR_SRC_MASK) {
		case RCC_CPERCKSELR_HSI:
			clock = stm32mp1_clk_get_fixed(priv, _HSI);
			break;
		case RCC_CPERCKSELR_HSE:
			clock = stm32mp1_clk_get_fixed(priv, _HSE);
			break;
		case RCC_CPERCKSELR_CSI:
			clock = stm32mp1_clk_get_fixed(priv, _CSI);
			break;
		}
		break;
	case _HSI:
	case _HSI_KER:
		clock = stm32mp1_clk_get_fixed(priv, _HSI);
		break;
	case _CSI:
	case _CSI_KER:
		clock = stm32mp1_clk_get_fixed(priv, _CSI);
		break;
	case _HSE:
	case _HSE_KER:
	case _HSE_KER_DIV2:
		clock = stm32mp1_clk_get_fixed(priv, _HSE);
		if (p == _HSE_KER_DIV2)
			clock >>= 1;
		break;
	case _LSI:
		clock = stm32mp1_clk_get_fixed(priv, _LSI);
		break;
	case _LSE:
		clock = stm32mp1_clk_get_fixed(priv, _LSE);
		break;
	/* PLL */
	case _PLL1_P:
	case _PLL1_Q:
	case _PLL1_R:
		clock = stm32mp1_read_pll_freq(priv, _PLL1, p - _PLL1_P);
		break;
	case _PLL2_P:
	case _PLL2_Q:
	case _PLL2_R:
		clock = stm32mp1_read_pll_freq(priv, _PLL2, p - _PLL2_P);
		break;
	case _PLL3_P:
	case _PLL3_Q:
	case _PLL3_R:
		clock = stm32mp1_read_pll_freq(priv, _PLL3, p - _PLL3_P);
		break;
	case _PLL4_P:
	case _PLL4_Q:
	case _PLL4_R:
		clock = stm32mp1_read_pll_freq(priv, _PLL4, p - _PLL4_P);
		break;
	/* other */
	case _USB_PHY_48:
		clock = stm32mp1_clk_get_by_name("ck_usbo_48m");
		break;
	case _DSI_PHY:
		clock = stm32mp1_clk_get_by_name("ck_dsi_phy");
		break;
	default:
		break;
	}

	log_debug("id=%d clock = %lx : %ld kHz\n", p, clock, clock / 1000);

	return clock;
}

static int stm32mp1_clk_enable(struct clk *clk)
{
	struct stm32mp1_clk_priv *priv = dev_get_priv(clk->dev);
	const struct stm32mp1_clk_gate *gate = priv->data->gate;
	int i = stm32mp1_clk_get_id(priv, clk->id);

	if (i < 0)
		return i;

	if (gate[i].set_clr)
		writel(BIT(gate[i].bit), priv->base + gate[i].offset);
	else
		setbits_le32(priv->base + gate[i].offset, BIT(gate[i].bit));

	dev_dbg(clk->dev, "%s: id clock %d has been enabled\n", __func__, (u32)clk->id);

	return 0;
}

static int stm32mp1_clk_disable(struct clk *clk)
{
	struct stm32mp1_clk_priv *priv = dev_get_priv(clk->dev);
	const struct stm32mp1_clk_gate *gate = priv->data->gate;
	int i = stm32mp1_clk_get_id(priv, clk->id);

	if (i < 0)
		return i;

	if (gate[i].set_clr)
		writel(BIT(gate[i].bit),
		       priv->base + gate[i].offset
		       + RCC_MP_ENCLRR_OFFSET);
	else
		clrbits_le32(priv->base + gate[i].offset, BIT(gate[i].bit));

	dev_dbg(clk->dev, "%s: id clock %d has been disabled\n", __func__, (u32)clk->id);

	return 0;
}

static ulong stm32mp1_clk_get_rate(struct clk *clk)
{
	struct stm32mp1_clk_priv *priv = dev_get_priv(clk->dev);
	int p = stm32mp1_clk_get_parent(priv, clk->id);
	ulong rate;

	if (p < 0)
		return 0;

	rate = stm32mp1_clk_get(priv, p);

	dev_vdbg(clk->dev, "computed rate for id clock %d is %d (parent is %s)\n",
		 (u32)clk->id, (u32)rate, stm32mp1_clk_parent_name[p]);

	return rate;
}

#ifdef STM32MP1_CLOCK_TREE_INIT

bool stm32mp1_supports_opp(u32 opp_id, u32 cpu_type)
{
	unsigned int id;

	switch (opp_id) {
	case 1:
	case 2:
		id = opp_id;
		break;
	default:
		id = 1; /* default value */
		break;
	}

	switch (cpu_type) {
	case CPU_STM32MP157Fxx:
	case CPU_STM32MP157Dxx:
	case CPU_STM32MP153Fxx:
	case CPU_STM32MP153Dxx:
	case CPU_STM32MP151Fxx:
	case CPU_STM32MP151Dxx:
		return true;
	default:
		return id == 1;
	}
}

__weak void board_vddcore_init(u32 voltage_mv)
{
}

/*
 * gets OPP parameters (frequency in KHz and voltage in mV) from
 * an OPP table subnode. Platform HW support capabilities are also checked.
 * Returns 0 on success and a negative FDT error code on failure.
 */
static int stm32mp1_get_opp(u32 cpu_type, ofnode subnode,
			    u32 *freq_khz, u32 *voltage_mv)
{
	u32 opp_hw;
	u64 read_freq_64;
	u32 read_voltage_32;

	*freq_khz = 0;
	*voltage_mv = 0;

	opp_hw = ofnode_read_u32_default(subnode, "opp-supported-hw", 0);
	if (opp_hw)
		if (!stm32mp1_supports_opp(opp_hw, cpu_type))
			return -FDT_ERR_BADVALUE;

	read_freq_64 = ofnode_read_u64_default(subnode, "opp-hz", 0) /
		       1000ULL;
	read_voltage_32 = ofnode_read_u32_default(subnode, "opp-microvolt", 0) /
			  1000U;

	if (!read_voltage_32 || !read_freq_64)
		return -FDT_ERR_NOTFOUND;

	/* Frequency value expressed in KHz must fit on 32 bits */
	if (read_freq_64 > U32_MAX)
		return -FDT_ERR_BADVALUE;

	/* Millivolt value must fit on 16 bits */
	if (read_voltage_32 > U16_MAX)
		return -FDT_ERR_BADVALUE;

	*freq_khz = (u32)read_freq_64;
	*voltage_mv = read_voltage_32;

	return 0;
}

/*
 * parses OPP table in DT and finds the parameters for the
 * highest frequency supported by the HW platform.
 * Returns 0 on success and a negative FDT error code on failure.
 */
int stm32mp1_get_max_opp_freq(struct stm32mp1_clk_priv *priv, u64 *freq_hz)
{
	ofnode node, subnode;
	int ret;
	u32 freq = 0U, voltage = 0U;
	u32 cpu_type = get_cpu_type();

	node = ofnode_by_compatible(ofnode_null(), "operating-points-v2");
	if (!ofnode_valid(node))
		return -FDT_ERR_NOTFOUND;

	ofnode_for_each_subnode(subnode, node) {
		unsigned int read_freq;
		unsigned int read_voltage;

		ret = stm32mp1_get_opp(cpu_type, subnode,
				       &read_freq, &read_voltage);
		if (ret)
			continue;

		if (read_freq > freq) {
			freq = read_freq;
			voltage = read_voltage;
		}
	}

	if (!freq || !voltage)
		return -FDT_ERR_NOTFOUND;

	*freq_hz = (u64)1000U * freq;
	board_vddcore_init(voltage);

	return 0;
}

static int stm32mp1_pll1_opp(struct stm32mp1_clk_priv *priv, int clksrc,
			     u32 *pllcfg, u32 *fracv)
{
	u32 post_divm;
	u32 input_freq;
	u64 output_freq;
	u64 freq;
	u64 vco;
	u32 divm, divn, divp, frac;
	int i, ret;
	u32 diff;
	u32 best_diff = U32_MAX;

	/* PLL1 is 1600 */
	const u32 DIVN_MAX = stm32mp1_pll[PLL_1600].divn_max;
	const u32 POST_DIVM_MIN = stm32mp1_pll[PLL_1600].refclk_min * 1000000U;
	const u32 POST_DIVM_MAX = stm32mp1_pll[PLL_1600].refclk_max * 1000000U;

	ret = stm32mp1_get_max_opp_freq(priv, &output_freq);
	if (ret) {
		log_debug("PLL1 OPP configuration not found (%d).\n", ret);
		return ret;
	}

	switch (clksrc) {
	case CLK_PLL12_HSI:
		input_freq = stm32mp1_clk_get_fixed(priv, _HSI);
		break;
	case CLK_PLL12_HSE:
		input_freq = stm32mp1_clk_get_fixed(priv, _HSE);
		break;
	default:
		return -EINTR;
	}

	/* Following parameters have always the same value */
	pllcfg[PLLCFG_Q] = 0;
	pllcfg[PLLCFG_R] = 0;
	pllcfg[PLLCFG_O] = PQR(1, 0, 0);

	for (divm = DIVM_MAX; divm >= DIVM_MIN; divm--)	{
		post_divm = (u32)(input_freq / (divm + 1));
		if (post_divm < POST_DIVM_MIN || post_divm > POST_DIVM_MAX)
			continue;

		for (divp = DIVP_MIN; divp <= DIVP_MAX; divp++) {
			freq = output_freq * (divm + 1) * (divp + 1);
			divn = (u32)((freq / input_freq) - 1);
			if (divn < DIVN_MIN || divn > DIVN_MAX)
				continue;

			frac = (u32)(((freq * FRAC_MAX) / input_freq) -
				     ((divn + 1) * FRAC_MAX));
			/* 2 loops to refine the fractional part */
			for (i = 2; i != 0; i--) {
				if (frac > FRAC_MAX)
					break;

				vco = (post_divm * (divn + 1)) +
				      ((post_divm * (u64)frac) /
				       FRAC_MAX);
				if (vco < (PLL1600_VCO_MIN / 2) ||
				    vco > (PLL1600_VCO_MAX / 2)) {
					frac++;
					continue;
				}
				freq = vco / (divp + 1);
				if (output_freq < freq)
					diff = (u32)(freq - output_freq);
				else
					diff = (u32)(output_freq - freq);
				if (diff < best_diff)  {
					pllcfg[PLLCFG_M] = divm;
					pllcfg[PLLCFG_N] = divn;
					pllcfg[PLLCFG_P] = divp;
					*fracv = frac;

					if (diff == 0)
						return 0;

					best_diff = diff;
				}
				frac++;
			}
		}
	}

	if (best_diff == U32_MAX)
		return -1;

	return 0;
}

static void stm32mp1_ls_osc_set(int enable, fdt_addr_t rcc, u32 offset,
				u32 mask_on)
{
	u32 address = rcc + offset;

	if (enable)
		setbits_le32(address, mask_on);
	else
		clrbits_le32(address, mask_on);
}

static void stm32mp1_hs_ocs_set(int enable, fdt_addr_t rcc, u32 mask_on)
{
	writel(mask_on, rcc + (enable ? RCC_OCENSETR : RCC_OCENCLRR));
}

static int stm32mp1_osc_wait(int enable, fdt_addr_t rcc, u32 offset,
			     u32 mask_rdy)
{
	u32 mask_test = 0;
	u32 address = rcc + offset;
	u32 val;
	int ret;

	if (enable)
		mask_test = mask_rdy;

	ret = readl_poll_timeout(address, val,
				 (val & mask_rdy) == mask_test,
				 TIMEOUT_1S);

	if (ret)
		log_err("OSC %x @ %x timeout for enable=%d : 0x%x\n",
			mask_rdy, address, enable, readl(address));

	return ret;
}

static void stm32mp1_lse_enable(fdt_addr_t rcc, int bypass, int digbyp,
				u32 lsedrv)
{
	u32 value;

	if (digbyp)
		setbits_le32(rcc + RCC_BDCR, RCC_BDCR_DIGBYP);

	if (bypass || digbyp)
		setbits_le32(rcc + RCC_BDCR, RCC_BDCR_LSEBYP);

	/*
	 * warning: not recommended to switch directly from "high drive"
	 * to "medium low drive", and vice-versa.
	 */
	value = (readl(rcc + RCC_BDCR) & RCC_BDCR_LSEDRV_MASK)
		>> RCC_BDCR_LSEDRV_SHIFT;

	while (value != lsedrv) {
		if (value > lsedrv)
			value--;
		else
			value++;

		clrsetbits_le32(rcc + RCC_BDCR,
				RCC_BDCR_LSEDRV_MASK,
				value << RCC_BDCR_LSEDRV_SHIFT);
	}

	stm32mp1_ls_osc_set(1, rcc, RCC_BDCR, RCC_BDCR_LSEON);
}

static void stm32mp1_lse_wait(fdt_addr_t rcc)
{
	stm32mp1_osc_wait(1, rcc, RCC_BDCR, RCC_BDCR_LSERDY);
}

static void stm32mp1_lsi_set(fdt_addr_t rcc, int enable)
{
	stm32mp1_ls_osc_set(enable, rcc, RCC_RDLSICR, RCC_RDLSICR_LSION);
	stm32mp1_osc_wait(enable, rcc, RCC_RDLSICR, RCC_RDLSICR_LSIRDY);
}

static void stm32mp1_hse_enable(fdt_addr_t rcc, int bypass, int digbyp, int css)
{
	if (digbyp)
		writel(RCC_OCENR_DIGBYP, rcc + RCC_OCENSETR);
	if (bypass || digbyp)
		writel(RCC_OCENR_HSEBYP, rcc + RCC_OCENSETR);

	stm32mp1_hs_ocs_set(1, rcc, RCC_OCENR_HSEON);
	stm32mp1_osc_wait(1, rcc, RCC_OCRDYR, RCC_OCRDYR_HSERDY);

	if (css)
		writel(RCC_OCENR_HSECSSON, rcc + RCC_OCENSETR);
}

static void stm32mp1_csi_set(fdt_addr_t rcc, int enable)
{
	stm32mp1_hs_ocs_set(enable, rcc, RCC_OCENR_CSION);
	stm32mp1_osc_wait(enable, rcc, RCC_OCRDYR, RCC_OCRDYR_CSIRDY);
}

static void stm32mp1_hsi_set(fdt_addr_t rcc, int enable)
{
	stm32mp1_hs_ocs_set(enable, rcc, RCC_OCENR_HSION);
	stm32mp1_osc_wait(enable, rcc, RCC_OCRDYR, RCC_OCRDYR_HSIRDY);
}

static int stm32mp1_set_hsidiv(fdt_addr_t rcc, u8 hsidiv)
{
	u32 address = rcc + RCC_OCRDYR;
	u32 val;
	int ret;

	clrsetbits_le32(rcc + RCC_HSICFGR,
			RCC_HSICFGR_HSIDIV_MASK,
			RCC_HSICFGR_HSIDIV_MASK & hsidiv);

	ret = readl_poll_timeout(address, val,
				 val & RCC_OCRDYR_HSIDIVRDY,
				 TIMEOUT_200MS);
	if (ret)
		log_err("HSIDIV failed @ 0x%x: 0x%x\n",
			address, readl(address));

	return ret;
}

static int stm32mp1_hsidiv(fdt_addr_t rcc, ulong hsifreq)
{
	u8 hsidiv;
	u32 hsidivfreq = MAX_HSI_HZ;

	for (hsidiv = 0; hsidiv < 4; hsidiv++,
	     hsidivfreq = hsidivfreq / 2)
		if (hsidivfreq == hsifreq)
			break;

	if (hsidiv == 4) {
		log_err("hsi frequency invalid");
		return -1;
	}

	if (hsidiv > 0)
		return stm32mp1_set_hsidiv(rcc, hsidiv);

	return 0;
}

static void pll_start(struct stm32mp1_clk_priv *priv, int pll_id)
{
	const struct stm32mp1_clk_pll *pll = priv->data->pll;

	clrsetbits_le32(priv->base + pll[pll_id].pllxcr,
			RCC_PLLNCR_DIVPEN | RCC_PLLNCR_DIVQEN |
			RCC_PLLNCR_DIVREN,
			RCC_PLLNCR_PLLON);
}

static int pll_output(struct stm32mp1_clk_priv *priv, int pll_id, int output)
{
	const struct stm32mp1_clk_pll *pll = priv->data->pll;
	u32 pllxcr = priv->base + pll[pll_id].pllxcr;
	u32 val;
	int ret;

	ret = readl_poll_timeout(pllxcr, val, val & RCC_PLLNCR_PLLRDY,
				 TIMEOUT_200MS);

	if (ret) {
		log_err("PLL%d start failed @ 0x%x: 0x%x\n",
			pll_id, pllxcr, readl(pllxcr));
		return ret;
	}

	/* start the requested output */
	setbits_le32(pllxcr, output << RCC_PLLNCR_DIVEN_SHIFT);

	return 0;
}

static int pll_stop(struct stm32mp1_clk_priv *priv, int pll_id)
{
	const struct stm32mp1_clk_pll *pll = priv->data->pll;
	u32 pllxcr = priv->base + pll[pll_id].pllxcr;
	u32 val;

	/* stop all output */
	clrbits_le32(pllxcr,
		     RCC_PLLNCR_DIVPEN | RCC_PLLNCR_DIVQEN | RCC_PLLNCR_DIVREN);

	/* stop PLL */
	clrbits_le32(pllxcr, RCC_PLLNCR_PLLON);

	/* wait PLL stopped */
	return readl_poll_timeout(pllxcr, val, (val & RCC_PLLNCR_PLLRDY) == 0,
				  TIMEOUT_200MS);
}

static void pll_config_output(struct stm32mp1_clk_priv *priv,
			      int pll_id, u32 *pllcfg)
{
	const struct stm32mp1_clk_pll *pll = priv->data->pll;
	fdt_addr_t rcc = priv->base;
	u32 value;

	value = (pllcfg[PLLCFG_P] << RCC_PLLNCFGR2_DIVP_SHIFT)
		& RCC_PLLNCFGR2_DIVP_MASK;
	value |= (pllcfg[PLLCFG_Q] << RCC_PLLNCFGR2_DIVQ_SHIFT)
		 & RCC_PLLNCFGR2_DIVQ_MASK;
	value |= (pllcfg[PLLCFG_R] << RCC_PLLNCFGR2_DIVR_SHIFT)
		 & RCC_PLLNCFGR2_DIVR_MASK;
	writel(value, rcc + pll[pll_id].pllxcfgr2);
}

static int pll_config(struct stm32mp1_clk_priv *priv, int pll_id,
		      u32 *pllcfg, u32 fracv)
{
	const struct stm32mp1_clk_pll *pll = priv->data->pll;
	fdt_addr_t rcc = priv->base;
	enum stm32mp1_plltype type = pll[pll_id].plltype;
	int src;
	ulong refclk;
	u8 ifrge = 0;
	u32 value;

	src = readl(priv->base + pll[pll_id].rckxselr) & RCC_SELR_SRC_MASK;

	refclk = stm32mp1_clk_get_fixed(priv, pll[pll_id].refclk[src]) /
		 (pllcfg[PLLCFG_M] + 1);

	if (refclk < (stm32mp1_pll[type].refclk_min * 1000000) ||
	    refclk > (stm32mp1_pll[type].refclk_max * 1000000)) {
		log_err("invalid refclk = %x\n", (u32)refclk);
		return -EINVAL;
	}
	if (type == PLL_800 && refclk >= 8000000)
		ifrge = 1;

	value = (pllcfg[PLLCFG_N] << RCC_PLLNCFGR1_DIVN_SHIFT)
		 & RCC_PLLNCFGR1_DIVN_MASK;
	value |= (pllcfg[PLLCFG_M] << RCC_PLLNCFGR1_DIVM_SHIFT)
		 & RCC_PLLNCFGR1_DIVM_MASK;
	value |= (ifrge << RCC_PLLNCFGR1_IFRGE_SHIFT)
		 & RCC_PLLNCFGR1_IFRGE_MASK;
	writel(value, rcc + pll[pll_id].pllxcfgr1);

	/* fractional configuration: load sigma-delta modulator (SDM) */

	/* Write into FRACV the new fractional value , and FRACLE to 0 */
	writel(fracv << RCC_PLLNFRACR_FRACV_SHIFT,
	       rcc + pll[pll_id].pllxfracr);

	/* Write FRACLE to 1 : FRACV value is loaded into the SDM */
	setbits_le32(rcc + pll[pll_id].pllxfracr,
		     RCC_PLLNFRACR_FRACLE);

	pll_config_output(priv, pll_id, pllcfg);

	return 0;
}

static void pll_csg(struct stm32mp1_clk_priv *priv, int pll_id, u32 *csg)
{
	const struct stm32mp1_clk_pll *pll = priv->data->pll;
	u32 pllxcsg;

	pllxcsg = ((csg[PLLCSG_MOD_PER] << RCC_PLLNCSGR_MOD_PER_SHIFT) &
		    RCC_PLLNCSGR_MOD_PER_MASK) |
		  ((csg[PLLCSG_INC_STEP] << RCC_PLLNCSGR_INC_STEP_SHIFT) &
		    RCC_PLLNCSGR_INC_STEP_MASK) |
		  ((csg[PLLCSG_SSCG_MODE] << RCC_PLLNCSGR_SSCG_MODE_SHIFT) &
		    RCC_PLLNCSGR_SSCG_MODE_MASK);

	writel(pllxcsg, priv->base + pll[pll_id].pllxcsgr);

	setbits_le32(priv->base + pll[pll_id].pllxcr, RCC_PLLNCR_SSCG_CTRL);
}

static  __maybe_unused int pll_set_rate(struct udevice *dev,
					int pll_id,
					int div_id,
					unsigned long clk_rate)
{
	struct stm32mp1_clk_priv *priv = dev_get_priv(dev);
	unsigned int pllcfg[PLLCFG_NB];
	ofnode plloff;
	char name[12];
	const struct stm32mp1_clk_pll *pll = priv->data->pll;
	enum stm32mp1_plltype type = pll[pll_id].plltype;
	int divm, divn, divy;
	int ret;
	ulong fck_ref;
	u32 fracv;
	u64 value;

	if (div_id > _DIV_NB)
		return -EINVAL;

	sprintf(name, "st,pll@%d", pll_id);
	plloff = dev_read_subnode(dev, name);
	if (!ofnode_valid(plloff))
		return -FDT_ERR_NOTFOUND;

	ret = ofnode_read_u32_array(plloff, "cfg",
				    pllcfg, PLLCFG_NB);
	if (ret < 0)
		return -FDT_ERR_NOTFOUND;

	fck_ref = pll_get_fref_ck(priv, pll_id);

	divm = pllcfg[PLLCFG_M];
	/* select output divider = 0: for _DIV_P, 1:_DIV_Q 2:_DIV_R */
	divy = pllcfg[PLLCFG_P + div_id];

	/* For: PLL1 & PLL2 => VCO is * 2 but ck_pll_y is also / 2
	 * So same final result than PLL2 et 4
	 * with FRACV
	 * Fck_pll_y = Fck_ref * ((DIVN + 1) + FRACV / 2^13)
	 *             / (DIVy + 1) * (DIVM + 1)
	 * value = (DIVN + 1) * 2^13 + FRACV / 2^13
	 *       = Fck_pll_y (DIVy + 1) * (DIVM + 1) * 2^13 / Fck_ref
	 */
	value = ((u64)clk_rate * (divy + 1) * (divm + 1)) << 13;
	value = lldiv(value, fck_ref);

	divn = (value >> 13) - 1;
	if (divn < DIVN_MIN ||
	    divn > stm32mp1_pll[type].divn_max) {
		dev_err(dev, "divn invalid = %d", divn);
		return -EINVAL;
	}
	fracv = value - ((divn + 1) << 13);
	pllcfg[PLLCFG_N] = divn;

	/* reconfigure PLL */
	pll_stop(priv, pll_id);
	pll_config(priv, pll_id, pllcfg, fracv);
	pll_start(priv, pll_id);
	pll_output(priv, pll_id, pllcfg[PLLCFG_O]);

	return 0;
}

static int set_clksrc(struct stm32mp1_clk_priv *priv, unsigned int clksrc)
{
	u32 address = priv->base + (clksrc >> 4);
	u32 val;
	int ret;

	clrsetbits_le32(address, RCC_SELR_SRC_MASK, clksrc & RCC_SELR_SRC_MASK);
	ret = readl_poll_timeout(address, val, val & RCC_SELR_SRCRDY,
				 TIMEOUT_200MS);
	if (ret)
		log_err("CLKSRC %x start failed @ 0x%x: 0x%x\n",
			clksrc, address, readl(address));

	return ret;
}

static void stgen_config(struct stm32mp1_clk_priv *priv)
{
	int p;
	u32 stgenc, cntfid0;
	ulong rate;

	stgenc = STM32_STGEN_BASE;
	cntfid0 = readl(stgenc + STGENC_CNTFID0);
	p = stm32mp1_clk_get_parent(priv, STGEN_K);
	rate = stm32mp1_clk_get(priv, p);

	if (cntfid0 != rate) {
		u64 counter;

		log_debug("System Generic Counter (STGEN) update\n");
		clrbits_le32(stgenc + STGENC_CNTCR, STGENC_CNTCR_EN);
		counter = (u64)readl(stgenc + STGENC_CNTCVL);
		counter |= ((u64)(readl(stgenc + STGENC_CNTCVU))) << 32;
		counter = lldiv(counter * (u64)rate, cntfid0);
		writel((u32)counter, stgenc + STGENC_CNTCVL);
		writel((u32)(counter >> 32), stgenc + STGENC_CNTCVU);
		writel(rate, stgenc + STGENC_CNTFID0);
		setbits_le32(stgenc + STGENC_CNTCR, STGENC_CNTCR_EN);

		__asm__ volatile("mcr p15, 0, %0, c14, c0, 0" : : "r" (rate));

		/* need to update gd->arch.timer_rate_hz with new frequency */
		timer_init();
	}
}

static int set_clkdiv(unsigned int clkdiv, u32 address)
{
	u32 val;
	int ret;

	clrsetbits_le32(address, RCC_DIVR_DIV_MASK, clkdiv & RCC_DIVR_DIV_MASK);
	ret = readl_poll_timeout(address, val, val & RCC_DIVR_DIVRDY,
				 TIMEOUT_200MS);
	if (ret)
		log_err("CLKDIV %x start failed @ 0x%x: 0x%x\n",
			clkdiv, address, readl(address));

	return ret;
}

static void stm32mp1_mco_csg(struct stm32mp1_clk_priv *priv,
			     u32 clksrc, u32 clkdiv)
{
	u32 address = priv->base + (clksrc >> 4);

	/*
	 * binding clksrc : bit15-4 offset
	 *                  bit3:   disable
	 *                  bit2-0: MCOSEL[2:0]
	 */
	if (clksrc & 0x8) {
		clrbits_le32(address, RCC_MCOCFG_MCOON);
	} else {
		clrsetbits_le32(address,
				RCC_MCOCFG_MCOSRC_MASK,
				clksrc & RCC_MCOCFG_MCOSRC_MASK);
		clrsetbits_le32(address,
				RCC_MCOCFG_MCODIV_MASK,
				clkdiv << RCC_MCOCFG_MCODIV_SHIFT);
		setbits_le32(address, RCC_MCOCFG_MCOON);
	}
}

static void set_rtcsrc(struct stm32mp1_clk_priv *priv,
		       unsigned int clksrc,
		       int lse_css)
{
	u32 address = priv->base + RCC_BDCR;

	if (readl(address) & RCC_BDCR_RTCCKEN)
		goto skip_rtc;

	if (clksrc == CLK_RTC_DISABLED)
		goto skip_rtc;

	clrsetbits_le32(address,
			RCC_BDCR_RTCSRC_MASK,
			clksrc << RCC_BDCR_RTCSRC_SHIFT);

	setbits_le32(address, RCC_BDCR_RTCCKEN);

skip_rtc:
	if (lse_css)
		setbits_le32(address, RCC_BDCR_LSECSSON);
}

static void pkcs_config(struct stm32mp1_clk_priv *priv, u32 pkcs)
{
	u32 address = priv->base + ((pkcs >> 4) & 0xFFF);
	u32 value = pkcs & 0xF;
	u32 mask = 0xF;

	if (pkcs & BIT(31)) {
		mask <<= 4;
		value <<= 4;
	}
	clrsetbits_le32(address, mask, value);
}

static int stm32mp1_clktree(struct udevice *dev)
{
	struct stm32mp1_clk_priv *priv = dev_get_priv(dev);
	fdt_addr_t rcc = priv->base;
	unsigned int clksrc[CLKSRC_NB];
	unsigned int clkdiv[CLKDIV_NB];
	unsigned int pllcfg[_PLL_NB][PLLCFG_NB];
	unsigned int pllfracv[_PLL_NB];
	unsigned int pllcsg[_PLL_NB][PLLCSG_NB];
	bool pllcfg_valid[_PLL_NB];
	bool pllcsg_set[_PLL_NB];
	int ret;
	int i, len;
	int lse_css = 0;
	const u32 *pkcs_cell;

	/* check mandatory field */
	ret = dev_read_u32_array(dev, "st,clksrc", clksrc, CLKSRC_NB);
	if (ret < 0) {
		dev_dbg(dev, "field st,clksrc invalid: error %d\n", ret);
		return -FDT_ERR_NOTFOUND;
	}

	ret = dev_read_u32_array(dev, "st,clkdiv", clkdiv, CLKDIV_NB);
	if (ret < 0) {
		dev_dbg(dev, "field st,clkdiv invalid: error %d\n", ret);
		return -FDT_ERR_NOTFOUND;
	}

	/* check mandatory field in each pll */
	for (i = 0; i < _PLL_NB; i++) {
		char name[12];
		ofnode node;

		sprintf(name, "st,pll@%d", i);
		node = dev_read_subnode(dev, name);
		pllcfg_valid[i] = ofnode_valid(node);
		pllcsg_set[i] = false;
		if (pllcfg_valid[i]) {
			dev_dbg(dev, "DT for PLL %d @ %s\n", i, name);
			ret = ofnode_read_u32_array(node, "cfg",
						    pllcfg[i], PLLCFG_NB);
			if (ret < 0) {
				dev_dbg(dev, "field cfg invalid: error %d\n", ret);
				return -FDT_ERR_NOTFOUND;
			}
			pllfracv[i] = ofnode_read_u32_default(node, "frac", 0);

			ret = ofnode_read_u32_array(node, "csg", pllcsg[i],
						    PLLCSG_NB);
			if (!ret) {
				pllcsg_set[i] = true;
			} else if (ret != -FDT_ERR_NOTFOUND) {
				dev_dbg(dev, "invalid csg node for pll@%d res=%d\n",
					i, ret);
				return ret;
			}
		} else if (i == _PLL1)	{
			/* use OPP for PLL1 for A7 CPU */
			dev_dbg(dev, "DT for PLL %d with OPP\n", i);
			ret = stm32mp1_pll1_opp(priv,
						clksrc[CLKSRC_PLL12],
						pllcfg[i],
						&pllfracv[i]);
			if (ret) {
				dev_dbg(dev, "PLL %d with OPP error = %d\n", i, ret);
				return ret;
			}
			pllcfg_valid[i] = true;
		}
	}

	dev_dbg(dev, "configuration MCO\n");
	stm32mp1_mco_csg(priv, clksrc[CLKSRC_MCO1], clkdiv[CLKDIV_MCO1]);
	stm32mp1_mco_csg(priv, clksrc[CLKSRC_MCO2], clkdiv[CLKDIV_MCO2]);

	dev_dbg(dev, "switch ON osillator\n");
	/*
	 * switch ON oscillator found in device-tree,
	 * HSI already ON after bootrom
	 */
	if (clk_valid(&priv->osc_clk[_LSI]))
		stm32mp1_lsi_set(rcc, 1);

	if (clk_valid(&priv->osc_clk[_LSE])) {
		int bypass, digbyp;
		u32 lsedrv;
		struct udevice *dev = priv->osc_clk[_LSE].dev;

		bypass = dev_read_bool(dev, "st,bypass");
		digbyp = dev_read_bool(dev, "st,digbypass");
		lse_css = dev_read_bool(dev, "st,css");
		lsedrv = dev_read_u32_default(dev, "st,drive",
					      LSEDRV_MEDIUM_HIGH);

		stm32mp1_lse_enable(rcc, bypass, digbyp, lsedrv);
	}

	if (clk_valid(&priv->osc_clk[_HSE])) {
		int bypass, digbyp, css;
		struct udevice *dev = priv->osc_clk[_HSE].dev;

		bypass = dev_read_bool(dev, "st,bypass");
		digbyp = dev_read_bool(dev, "st,digbypass");
		css = dev_read_bool(dev, "st,css");

		stm32mp1_hse_enable(rcc, bypass, digbyp, css);
	}
	/* CSI is mandatory for automatic I/O compensation (SYSCFG_CMPCR)
	 * => switch on CSI even if node is not present in device tree
	 */
	stm32mp1_csi_set(rcc, 1);

	/* come back to HSI */
	dev_dbg(dev, "come back to HSI\n");
	set_clksrc(priv, CLK_MPU_HSI);
	set_clksrc(priv, CLK_AXI_HSI);
	set_clksrc(priv, CLK_MCU_HSI);

	dev_dbg(dev, "pll stop\n");
	for (i = 0; i < _PLL_NB; i++)
		pll_stop(priv, i);

	/* configure HSIDIV */
	dev_dbg(dev, "configure HSIDIV\n");
	if (clk_valid(&priv->osc_clk[_HSI])) {
		stm32mp1_hsidiv(rcc, clk_get_rate(&priv->osc_clk[_HSI]));
		stgen_config(priv);
	}

	/* select DIV */
	dev_dbg(dev, "select DIV\n");
	/* no ready bit when MPUSRC != CLK_MPU_PLL1P_DIV, MPUDIV is disabled */
	writel(clkdiv[CLKDIV_MPU] & RCC_DIVR_DIV_MASK, rcc + RCC_MPCKDIVR);
	set_clkdiv(clkdiv[CLKDIV_AXI], rcc + RCC_AXIDIVR);
	set_clkdiv(clkdiv[CLKDIV_APB4], rcc + RCC_APB4DIVR);
	set_clkdiv(clkdiv[CLKDIV_APB5], rcc + RCC_APB5DIVR);
	set_clkdiv(clkdiv[CLKDIV_MCU], rcc + RCC_MCUDIVR);
	set_clkdiv(clkdiv[CLKDIV_APB1], rcc + RCC_APB1DIVR);
	set_clkdiv(clkdiv[CLKDIV_APB2], rcc + RCC_APB2DIVR);
	set_clkdiv(clkdiv[CLKDIV_APB3], rcc + RCC_APB3DIVR);

	/* no ready bit for RTC */
	writel(clkdiv[CLKDIV_RTC] & RCC_DIVR_DIV_MASK, rcc + RCC_RTCDIVR);

	/* configure PLLs source */
	dev_dbg(dev, "configure PLLs source\n");
	set_clksrc(priv, clksrc[CLKSRC_PLL12]);
	set_clksrc(priv, clksrc[CLKSRC_PLL3]);
	set_clksrc(priv, clksrc[CLKSRC_PLL4]);

	/* configure and start PLLs */
	dev_dbg(dev, "configure PLLs\n");
	for (i = 0; i < _PLL_NB; i++) {
		if (!pllcfg_valid[i])
			continue;
		dev_dbg(dev, "configure PLL %d\n", i);
		pll_config(priv, i, pllcfg[i], pllfracv[i]);
		if (pllcsg_set[i])
			pll_csg(priv, i, pllcsg[i]);
		pll_start(priv, i);
	}

	/* wait and start PLLs ouptut when ready */
	for (i = 0; i < _PLL_NB; i++) {
		if (!pllcfg_valid[i])
			continue;
		dev_dbg(dev, "output PLL %d\n", i);
		pll_output(priv, i, pllcfg[i][PLLCFG_O]);
	}

	/* wait LSE ready before to use it */
	if (clk_valid(&priv->osc_clk[_LSE]))
		stm32mp1_lse_wait(rcc);

	/* configure with expected clock source */
	dev_dbg(dev, "CLKSRC\n");
	set_clksrc(priv, clksrc[CLKSRC_MPU]);
	set_clksrc(priv, clksrc[CLKSRC_AXI]);
	set_clksrc(priv, clksrc[CLKSRC_MCU]);
	set_rtcsrc(priv, clksrc[CLKSRC_RTC], lse_css);

	/* configure PKCK */
	dev_dbg(dev, "PKCK\n");
	pkcs_cell = dev_read_prop(dev, "st,pkcs", &len);
	if (pkcs_cell) {
		bool ckper_disabled = false;

		for (i = 0; i < len / sizeof(u32); i++) {
			u32 pkcs = (u32)fdt32_to_cpu(pkcs_cell[i]);

			if (pkcs == CLK_CKPER_DISABLED) {
				ckper_disabled = true;
				continue;
			}
			pkcs_config(priv, pkcs);
		}
		/* CKPER is source for some peripheral clock
		 * (FMC-NAND / QPSI-NOR) and switching source is allowed
		 * only if previous clock is still ON
		 * => deactivated CKPER only after switching clock
		 */
		if (ckper_disabled)
			pkcs_config(priv, CLK_CKPER_DISABLED);
	}

	/* STGEN clock source can change with CLK_STGEN_XXX */
	stgen_config(priv);

	dev_dbg(dev, "oscillator off\n");
	/* switch OFF HSI if not found in device-tree */
	if (!clk_valid(&priv->osc_clk[_HSI]))
		stm32mp1_hsi_set(rcc, 0);

	/* Software Self-Refresh mode (SSR) during DDR initilialization */
	clrsetbits_le32(priv->base + RCC_DDRITFCR,
			RCC_DDRITFCR_DDRCKMOD_MASK,
			RCC_DDRITFCR_DDRCKMOD_SSR <<
			RCC_DDRITFCR_DDRCKMOD_SHIFT);

	return 0;
}
#endif /* STM32MP1_CLOCK_TREE_INIT */

static int pll_set_output_rate(struct udevice *dev,
			       int pll_id,
			       int div_id,
			       unsigned long clk_rate)
{
	struct stm32mp1_clk_priv *priv = dev_get_priv(dev);
	const struct stm32mp1_clk_pll *pll = priv->data->pll;
	u32 pllxcr = priv->base + pll[pll_id].pllxcr;
	int div;
	ulong fvco;

	if (div_id > _DIV_NB)
		return -EINVAL;

	fvco = pll_get_fvco(priv, pll_id);

	if (fvco <= clk_rate)
		div = 1;
	else
		div = DIV_ROUND_UP(fvco, clk_rate);

	if (div > 128)
		div = 128;

	/* stop the requested output */
	clrbits_le32(pllxcr, 0x1 << div_id << RCC_PLLNCR_DIVEN_SHIFT);
	/* change divider */
	clrsetbits_le32(priv->base + pll[pll_id].pllxcfgr2,
			RCC_PLLNCFGR2_DIVX_MASK << RCC_PLLNCFGR2_SHIFT(div_id),
			(div - 1) << RCC_PLLNCFGR2_SHIFT(div_id));
	/* start the requested output */
	setbits_le32(pllxcr, 0x1 << div_id << RCC_PLLNCR_DIVEN_SHIFT);

	return 0;
}

static ulong stm32mp1_clk_set_rate(struct clk *clk, unsigned long clk_rate)
{
	struct stm32mp1_clk_priv *priv = dev_get_priv(clk->dev);
	int p;

	switch (clk->id) {
#if defined(STM32MP1_CLOCK_TREE_INIT) && \
	defined(CONFIG_STM32MP1_DDR_INTERACTIVE)
	case DDRPHYC:
		break;
#endif
	case LTDC_PX:
	case DSI_PX:
		break;
	default:
		dev_err(clk->dev, "Set of clk %ld not supported", clk->id);
		return -EINVAL;
	}

	p = stm32mp1_clk_get_parent(priv, clk->id);
	dev_vdbg(clk->dev, "parent = %d:%s\n", p, stm32mp1_clk_parent_name[p]);
	if (p < 0)
		return -EINVAL;

	switch (p) {
#if defined(STM32MP1_CLOCK_TREE_INIT) && \
	defined(CONFIG_STM32MP1_DDR_INTERACTIVE)
	case _PLL2_R: /* DDRPHYC */
	{
		/* only for change DDR clock in interactive mode */
		ulong result;

		set_clksrc(priv, CLK_AXI_HSI);
		result = pll_set_rate(clk->dev,  _PLL2, _DIV_R, clk_rate);
		set_clksrc(priv, CLK_AXI_PLL2P);
		return result;
	}
#endif

	case _PLL4_Q:
		/* for LTDC_PX and DSI_PX case */
		return pll_set_output_rate(clk->dev, _PLL4, _DIV_Q, clk_rate);
	}

	return -EINVAL;
}

static void stm32mp1_osc_init(struct udevice *dev)
{
	struct stm32mp1_clk_priv *priv = dev_get_priv(dev);
	int i;
	const char *name[NB_OSC] = {
		[_LSI] = "lsi",
		[_LSE] = "lse",
		[_HSI] = "hsi",
		[_HSE] = "hse",
		[_CSI] = "csi",
		[_I2S_CKIN] = "i2s_ckin",
	};

	for (i = 0; i < NB_OSC; i++) {
		if (clk_get_by_name(dev, name[i], &priv->osc_clk[i]))
			dev_dbg(dev, "No source clock \"%s\"\n", name[i]);
		else
			dev_dbg(dev, "%s clock rate: %luHz\n",
				name[i], clk_get_rate(&priv->osc_clk[i]));
	}
}

static void __maybe_unused stm32mp1_clk_dump(struct udevice *dev)
{
	char buf[32];
	int i, s, p;
	struct stm32mp1_clk_priv *priv;

	priv = dev_get_priv(dev);

	printf("Clocks:\n");
	for (i = 0; i < _PARENT_NB; i++) {
		printf("- %s : %s MHz\n",
		       stm32mp1_clk_parent_name[i],
		       strmhz(buf, stm32mp1_clk_get(priv, i)));
	}
	printf("Source Clocks:\n");
	for (i = 0; i < _PARENT_SEL_NB; i++) {
		p = (readl(priv->base + priv->data->sel[i].offset) >>
		     priv->data->sel[i].src) & priv->data->sel[i].msk;
		if (p < priv->data->sel[i].nb_parent) {
			s = priv->data->sel[i].parent[p];
			printf("- %s(%d) => parent %s(%d)\n",
			       stm32mp1_clk_parent_sel_name[i], i,
			       stm32mp1_clk_parent_name[s], s);
		} else {
			printf("- %s(%d) => parent index %d is invalid\n",
			       stm32mp1_clk_parent_sel_name[i], i, p);
		}
	}
}

static int stm32mp1_clk_probe(struct udevice *dev)
{
	int result = 0;
	struct stm32mp1_clk_priv *priv = dev_get_priv(dev);

	priv->base = dev_read_addr(dev->parent);
	if (priv->base == FDT_ADDR_T_NONE)
		return -EINVAL;

	priv->data = (void *)&stm32mp1_data;

	if (!priv->data->gate || !priv->data->sel ||
	    !priv->data->pll)
		return -EINVAL;

	stm32mp1_osc_init(dev);

#ifdef STM32MP1_CLOCK_TREE_INIT
	/* clock tree init is done only one time, before relocation */
	if (!(gd->flags & GD_FLG_RELOC))
		result = stm32mp1_clktree(dev);
	if (result)
		dev_err(dev, "clock tree initialization failed (%d)\n", result);
#endif

#ifndef CONFIG_SPL_BUILD
#if defined(VERBOSE_DEBUG)
	/* display debug information for probe after relocation */
	if (gd->flags & GD_FLG_RELOC)
		stm32mp1_clk_dump(dev);
#endif

	gd->cpu_clk = stm32mp1_clk_get(priv, _CK_MPU);
	gd->bus_clk = stm32mp1_clk_get(priv, _ACLK);
	/* DDRPHYC father */
	gd->mem_clk = stm32mp1_clk_get(priv, _PLL2_R);
#if defined(CONFIG_DISPLAY_CPUINFO)
	if (gd->flags & GD_FLG_RELOC) {
		char buf[32];

		log_info("Clocks:\n");
		log_info("- MPU : %s MHz\n", strmhz(buf, gd->cpu_clk));
		log_info("- MCU : %s MHz\n",
			 strmhz(buf, stm32mp1_clk_get(priv, _CK_MCU)));
		log_info("- AXI : %s MHz\n", strmhz(buf, gd->bus_clk));
		log_info("- PER : %s MHz\n",
			 strmhz(buf, stm32mp1_clk_get(priv, _CK_PER)));
		log_info("- DDR : %s MHz\n", strmhz(buf, gd->mem_clk));
	}
#endif /* CONFIG_DISPLAY_CPUINFO */
#endif

	return result;
}

static const struct clk_ops stm32mp1_clk_ops = {
	.enable = stm32mp1_clk_enable,
	.disable = stm32mp1_clk_disable,
	.get_rate = stm32mp1_clk_get_rate,
	.set_rate = stm32mp1_clk_set_rate,
#if IS_ENABLED(CONFIG_CMD_CLK) && !IS_ENABLED(CONFIG_SPL_BUILD)
	.dump = stm32mp1_clk_dump,
#endif
};

U_BOOT_DRIVER(stm32mp1_clock) = {
	.name = "stm32mp1_clk",
	.id = UCLASS_CLK,
	.ops = &stm32mp1_clk_ops,
	.priv_auto	= sizeof(struct stm32mp1_clk_priv),
	.probe = stm32mp1_clk_probe,
};
