/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2010-2015
 * NVIDIA Corporation <www.nvidia.com>
 */

#ifndef _TEGRA_H_
#define _TEGRA_H_

#define NV_PA_ARM_PERIPHBASE	0x50040000
#define NV_PA_PG_UP_BASE	0x60000000
#define NV_PA_TMRUS_BASE	0x60005010
#define NV_PA_CLK_RST_BASE	0x60006000
#define NV_PA_FLOW_BASE		0x60007000
#define NV_PA_GPIO_BASE		0x6000D000
#define NV_PA_EVP_BASE		0x6000F000
#define NV_PA_APB_MISC_BASE	0x70000000
#define NV_PA_APB_MISC_GP_BASE	(NV_PA_APB_MISC_BASE + 0x0800)
#define NV_PA_APB_UARTA_BASE	(NV_PA_APB_MISC_BASE + 0x6000)
#define NV_PA_APB_UARTB_BASE	(NV_PA_APB_MISC_BASE + 0x6040)
#define NV_PA_APB_UARTC_BASE	(NV_PA_APB_MISC_BASE + 0x6200)
#define NV_PA_APB_UARTD_BASE	(NV_PA_APB_MISC_BASE + 0x6300)
#define NV_PA_APB_UARTE_BASE	(NV_PA_APB_MISC_BASE + 0x6400)
#define NV_PA_NAND_BASE		(NV_PA_APB_MISC_BASE + 0x8000)
#define NV_PA_SPI_BASE		(NV_PA_APB_MISC_BASE + 0xC380)
#define NV_PA_SLINK1_BASE	(NV_PA_APB_MISC_BASE + 0xD400)
#define NV_PA_SLINK2_BASE	(NV_PA_APB_MISC_BASE + 0xD600)
#define NV_PA_SLINK3_BASE	(NV_PA_APB_MISC_BASE + 0xD800)
#define NV_PA_SLINK4_BASE	(NV_PA_APB_MISC_BASE + 0xDA00)
#define NV_PA_SLINK5_BASE	(NV_PA_APB_MISC_BASE + 0xDC00)
#define NV_PA_SLINK6_BASE	(NV_PA_APB_MISC_BASE + 0xDE00)
#define TEGRA_DVC_BASE		(NV_PA_APB_MISC_BASE + 0xD000)
#if defined(CONFIG_TEGRA20) || defined(CONFIG_TEGRA30) || \
	defined(CONFIG_TEGRA114) || defined(CONFIG_TEGRA124) || \
	defined(CONFIG_TEGRA132) || defined(CONFIG_TEGRA210)
#define NV_PA_PMC_BASE		(NV_PA_APB_MISC_BASE + 0xE400)
#else
#define NV_PA_PMC_BASE		0xc360000
#endif
#define NV_PA_EMC_BASE		(NV_PA_APB_MISC_BASE + 0xF400)
#define NV_PA_FUSE_BASE		(NV_PA_APB_MISC_BASE + 0xF800)
#if defined(CONFIG_TEGRA20) || defined(CONFIG_TEGRA30) || \
	defined(CONFIG_TEGRA114)
#define NV_PA_CSITE_BASE	0x70040000
#else
#define NV_PA_CSITE_BASE	0x70800000
#endif
#define TEGRA_USB_ADDR_MASK	0xFFFFC000

#define NV_PA_SDRC_CS0		NV_PA_SDRAM_BASE
#define LOW_LEVEL_SRAM_STACK	0x4000FFFC
#define EARLY_AVP_STACK		(NV_PA_SDRAM_BASE + 0x20000)
#define EARLY_CPU_STACK		(EARLY_AVP_STACK - 4096)
#define PG_UP_TAG_AVP		0xAAAAAAAA

#ifndef __ASSEMBLY__
struct timerus {
	unsigned int cntr_1us;
};

/* Address at which WB code runs, it must not overlap Bootrom's IRAM usage */
#define NV_WB_RUN_ADDRESS	0x40020000

#define NVBOOTTYPE_RECOVERY	2	/* BR entered RCM */
#define NVBOOTINFOTABLE_BOOTTYPE 0xC	/* Boot type in BIT in IRAM */
#define NVBOOTINFOTABLE_BCTSIZE	0x38	/* BCT size in BIT in IRAM */
#define NVBOOTINFOTABLE_BCTPTR	0x3C	/* BCT pointer in BIT in IRAM */

/* These are the available SKUs (product types) for Tegra */
enum {
	SKU_ID_T20_7		= 0x7,
	SKU_ID_T20		= 0x8,
	SKU_ID_T25SE		= 0x14,
	SKU_ID_AP25		= 0x17,
	SKU_ID_T25		= 0x18,
	SKU_ID_AP25E		= 0x1b,
	SKU_ID_T25E		= 0x1c,
	SKU_ID_T33		= 0x80,
	SKU_ID_T30		= 0x81, /* Cardhu value */
	SKU_ID_TM30MQS_P_A3	= 0xb1,
	SKU_ID_T114_ENG		= 0x00, /* Dalmore value, unfused */
	SKU_ID_T114_1		= 0x01,
	SKU_ID_T124_ENG		= 0x00, /* Venice2 value, unfused */
	SKU_ID_T210_ENG		= 0x00, /* unfused value TBD */
};

/*
 * These are used to distinguish SOC types for setting up clocks. Mostly
 * we can tell the clocking required by looking at the SOC sku_id, but
 * for T30 it is a user option as to whether to run PLLP in fast or slow
 * mode, so we have two options there.
 */
enum {
	TEGRA_SOC_T20,
	TEGRA_SOC_T25,
	TEGRA_SOC_T30,
	TEGRA_SOC_T114,
	TEGRA_SOC_T124,
	TEGRA_SOC_T210,

	TEGRA_SOC_CNT,
	TEGRA_SOC_UNKNOWN	= -1,
};

/* Tegra system controller (SYSCON) devices */
enum {
	TEGRA_SYSCON_PMC,
};

#else  /* __ASSEMBLY__ */
#define PRM_RSTCTRL		NV_PA_PMC_BASE
#endif

#endif	/* TEGRA_H */
