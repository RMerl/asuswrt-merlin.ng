// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2017 Rockchip Electronics Co., Ltd
 */
#include <common.h>
#include <clk.h>
#include <dm.h>
#include <fdt_support.h>
#include <init.h>
#include <spl.h>
#include <asm/armv8/mmu.h>
#include <asm/io.h>
#include <asm/arch-rockchip/bootrom.h>
#include <asm/arch-rockchip/grf_px30.h>
#include <asm/arch-rockchip/hardware.h>
#include <asm/arch-rockchip/uart.h>
#include <asm/arch-rockchip/clock.h>
#include <asm/arch-rockchip/cru_px30.h>
#include <dt-bindings/clock/px30-cru.h>

const char * const boot_devices[BROM_LAST_BOOTSOURCE + 1] = {
	[BROM_BOOTSOURCE_EMMC] = "/mmc@ff390000",
	[BROM_BOOTSOURCE_SD] = "/mmc@ff370000",
};

static struct mm_region px30_mem_map[] = {
	{
		.virt = 0x0UL,
		.phys = 0x0UL,
		.size = 0xff000000UL,
		.attrs = PTE_BLOCK_MEMTYPE(MT_NORMAL) |
			 PTE_BLOCK_INNER_SHARE
	}, {
		.virt = 0xff000000UL,
		.phys = 0xff000000UL,
		.size = 0x01000000UL,
		.attrs = PTE_BLOCK_MEMTYPE(MT_DEVICE_NGNRNE) |
			 PTE_BLOCK_NON_SHARE |
			 PTE_BLOCK_PXN | PTE_BLOCK_UXN
	}, {
		/* List terminator */
		0,
	}
};

struct mm_region *mem_map = px30_mem_map;

#define PMU_PWRDN_CON			0xff000018
#define PMUGRF_BASE			0xff010000
#define GRF_BASE			0xff140000
#define CRU_BASE			0xff2b0000
#define PMUCRU_BASE			0xff2bc000
#define VIDEO_PHY_BASE			0xff2e0000
#define SERVICE_CORE_ADDR		0xff508000
#define DDR_FW_BASE			0xff534000

#define FW_DDR_CON			0x40

#define QOS_PRIORITY			0x08

#define QOS_PRIORITY_LEVEL(h, l)	((((h) & 3) << 8) | ((l) & 3))

/* GRF_GPIO1AL_IOMUX */
enum {
	GPIO1A3_SHIFT		= 12,
	GPIO1A3_MASK		= 0xf << GPIO1A3_SHIFT,
	GPIO1A3_GPIO		= 0,
	GPIO1A3_FLASH_D3,
	GPIO1A3_EMMC_D3,
	GPIO1A3_SFC_SIO3,

	GPIO1A2_SHIFT		= 8,
	GPIO1A2_MASK		= 0xf << GPIO1A2_SHIFT,
	GPIO1A2_GPIO		= 0,
	GPIO1A2_FLASH_D2,
	GPIO1A2_EMMC_D2,
	GPIO1A2_SFC_SIO2,

	GPIO1A1_SHIFT		= 4,
	GPIO1A1_MASK		= 0xf << GPIO1A1_SHIFT,
	GPIO1A1_GPIO		= 0,
	GPIO1A1_FLASH_D1,
	GPIO1A1_EMMC_D1,
	GPIO1A1_SFC_SIO1,

	GPIO1A0_SHIFT		= 0,
	GPIO1A0_MASK		= 0xf << GPIO1A0_SHIFT,
	GPIO1A0_GPIO		= 0,
	GPIO1A0_FLASH_D0,
	GPIO1A0_EMMC_D0,
	GPIO1A0_SFC_SIO0,
};

/* GRF_GPIO1AH_IOMUX */
enum {
	GPIO1A4_SHIFT		= 0,
	GPIO1A4_MASK		= 0xf << GPIO1A4_SHIFT,
	GPIO1A4_GPIO		= 0,
	GPIO1A4_FLASH_D4,
	GPIO1A4_EMMC_D4,
	GPIO1A4_SFC_CSN0,
};

/* GRF_GPIO1BL_IOMUX */
enum {
	GPIO1B1_SHIFT		= 4,
	GPIO1B1_MASK		= 0xf << GPIO1B1_SHIFT,
	GPIO1B1_GPIO		= 0,
	GPIO1B1_FLASH_RDY,
	GPIO1B1_EMMC_CLKOUT,
	GPIO1B1_SFC_CLK,
};

/* GRF_GPIO1BH_IOMUX */
enum {
	GPIO1B7_SHIFT		= 12,
	GPIO1B7_MASK		= 0xf << GPIO1B7_SHIFT,
	GPIO1B7_GPIO		= 0,
	GPIO1B7_FLASH_RDN,
	GPIO1B7_UART3_RXM1,
	GPIO1B7_SPI0_CLK,

	GPIO1B6_SHIFT		= 8,
	GPIO1B6_MASK		= 0xf << GPIO1B6_SHIFT,
	GPIO1B6_GPIO		= 0,
	GPIO1B6_FLASH_CS1,
	GPIO1B6_UART3_TXM1,
	GPIO1B6_SPI0_CSN,
};

/* GRF_GPIO1CL_IOMUX */
enum {
	GPIO1C1_SHIFT		= 4,
	GPIO1C1_MASK		= 0xf << GPIO1C1_SHIFT,
	GPIO1C1_GPIO		= 0,
	GPIO1C1_UART1_TX,

	GPIO1C0_SHIFT		= 0,
	GPIO1C0_MASK		= 0xf << GPIO1C0_SHIFT,
	GPIO1C0_GPIO		= 0,
	GPIO1C0_UART1_RX,
};

/* GRF_GPIO1DL_IOMUX */
enum {
	GPIO1D3_SHIFT		= 12,
	GPIO1D3_MASK		= 0xf << GPIO1D3_SHIFT,
	GPIO1D3_GPIO		= 0,
	GPIO1D3_SDMMC_D1,
	GPIO1D3_UART2_RXM0,

	GPIO1D2_SHIFT		= 8,
	GPIO1D2_MASK		= 0xf << GPIO1D2_SHIFT,
	GPIO1D2_GPIO		= 0,
	GPIO1D2_SDMMC_D0,
	GPIO1D2_UART2_TXM0,
};

/* GRF_GPIO1DH_IOMUX */
enum {
	GPIO1D7_SHIFT		= 12,
	GPIO1D7_MASK		= 0xf << GPIO1D7_SHIFT,
	GPIO1D7_GPIO		= 0,
	GPIO1D7_SDMMC_CMD,

	GPIO1D6_SHIFT		= 8,
	GPIO1D6_MASK		= 0xf << GPIO1D6_SHIFT,
	GPIO1D6_GPIO		= 0,
	GPIO1D6_SDMMC_CLK,

	GPIO1D5_SHIFT		= 4,
	GPIO1D5_MASK		= 0xf << GPIO1D5_SHIFT,
	GPIO1D5_GPIO		= 0,
	GPIO1D5_SDMMC_D3,

	GPIO1D4_SHIFT		= 0,
	GPIO1D4_MASK		= 0xf << GPIO1D4_SHIFT,
	GPIO1D4_GPIO		= 0,
	GPIO1D4_SDMMC_D2,
};

/* GRF_GPIO2BH_IOMUX */
enum {
	GPIO2B6_SHIFT		= 8,
	GPIO2B6_MASK		= 0xf << GPIO2B6_SHIFT,
	GPIO2B6_GPIO		= 0,
	GPIO2B6_CIF_D1M0,
	GPIO2B6_UART2_RXM1,

	GPIO2B4_SHIFT		= 0,
	GPIO2B4_MASK		= 0xf << GPIO2B4_SHIFT,
	GPIO2B4_GPIO		= 0,
	GPIO2B4_CIF_D0M0,
	GPIO2B4_UART2_TXM1,
};

/* GRF_GPIO3AL_IOMUX */
enum {
	GPIO3A2_SHIFT		= 8,
	GPIO3A2_MASK		= 0xf << GPIO3A2_SHIFT,
	GPIO3A2_GPIO		= 0,
	GPIO3A2_UART5_TX	= 4,

	GPIO3A1_SHIFT		= 4,
	GPIO3A1_MASK		= 0xf << GPIO3A1_SHIFT,
	GPIO3A1_GPIO		= 0,
	GPIO3A1_UART5_RX	= 4,
};

/* PMUGRF_GPIO0BL_IOMUX */
enum {
	GPIO0B3_SHIFT		= 6,
	GPIO0B3_MASK		= 0x3 << GPIO0B3_SHIFT,
	GPIO0B3_GPIO		= 0,
	GPIO0B3_UART0_RX,
	GPIO0B3_PMU_DEBUG1,

	GPIO0B2_SHIFT		= 4,
	GPIO0B2_MASK		= 0x3 << GPIO0B2_SHIFT,
	GPIO0B2_GPIO		= 0,
	GPIO0B2_UART0_TX,
	GPIO0B2_PMU_DEBUG0,
};

/* PMUGRF_GPIO0CL_IOMUX */
enum {
	GPIO0C1_SHIFT		= 2,
	GPIO0C1_MASK		= 0x3 << GPIO0C1_SHIFT,
	GPIO0C1_GPIO		= 0,
	GPIO0C1_PWM_3,
	GPIO0C1_UART3_RXM0,
	GPIO0C1_PMU_DEBUG4,

	GPIO0C0_SHIFT		= 0,
	GPIO0C0_MASK		= 0x3 << GPIO0C0_SHIFT,
	GPIO0C0_GPIO		= 0,
	GPIO0C0_PWM_1,
	GPIO0C0_UART3_TXM0,
	GPIO0C0_PMU_DEBUG3,
};

int arch_cpu_init(void)
{
	static struct px30_grf * const grf = (void *)GRF_BASE;
	static struct px30_cru * const cru = (void *)CRU_BASE;
	u32 __maybe_unused val;

#ifdef CONFIG_SPL_BUILD
	/* We do some SoC one time setting here. */
	/* Disable the ddr secure region setting to make it non-secure */
	writel(0x0, DDR_FW_BASE + FW_DDR_CON);

	/* Set cpu qos priority */
	writel(QOS_PRIORITY_LEVEL(1, 1), SERVICE_CORE_ADDR + QOS_PRIORITY);

#if !defined(CONFIG_DEBUG_UART_BOARD_INIT) || \
	(CONFIG_DEBUG_UART_BASE != 0xff160000) || \
	(CONFIG_DEBUG_UART_CHANNEL != 0)
	/* fix sdmmc pinmux if not using uart2-channel0 as debug uart */
	rk_clrsetreg(&grf->gpio1dl_iomux,
		     GPIO1D3_MASK | GPIO1D2_MASK,
		     GPIO1D3_SDMMC_D1 << GPIO1D3_SHIFT |
		     GPIO1D2_SDMMC_D0 << GPIO1D2_SHIFT);
	rk_clrsetreg(&grf->gpio1dh_iomux,
		     GPIO1D7_MASK | GPIO1D6_MASK | GPIO1D5_MASK | GPIO1D4_MASK,
		     GPIO1D7_SDMMC_CMD << GPIO1D7_SHIFT |
		     GPIO1D6_SDMMC_CLK << GPIO1D6_SHIFT |
		     GPIO1D5_SDMMC_D3 << GPIO1D5_SHIFT |
		     GPIO1D4_SDMMC_D2 << GPIO1D4_SHIFT);
#endif

#ifdef CONFIG_ROCKCHIP_SFC
	rk_clrsetreg(&grf->gpio1al_iomux,
		     GPIO1A3_MASK | GPIO1A2_MASK | GPIO1A1_MASK | GPIO1A0_MASK,
		     GPIO1A3_SFC_SIO3 << GPIO1A3_SHIFT |
		     GPIO1A2_SFC_SIO2 << GPIO1A2_SHIFT |
		     GPIO1A1_SFC_SIO1 << GPIO1A1_SHIFT |
		     GPIO1A0_SFC_SIO0 << GPIO1A0_SHIFT);
	rk_clrsetreg(&grf->gpio1ah_iomux, GPIO1A4_MASK,
		     GPIO1A4_SFC_CSN0 << GPIO1A4_SHIFT);
	rk_clrsetreg(&grf->gpio1bl_iomux, GPIO1B1_MASK,
		     GPIO1B1_SFC_CLK << GPIO1B1_SHIFT);
#endif

#endif

	/* Enable PD_VO (default disable at reset) */
	rk_clrreg(PMU_PWRDN_CON, 1 << 13);

	/* Disable video phy bandgap by default */
	writel(0x82, VIDEO_PHY_BASE + 0x0000);
	writel(0x05, VIDEO_PHY_BASE + 0x03ac);

	/* Clear the force_jtag */
	rk_clrreg(&grf->cpu_con[1], 1 << 7);

	/* Make TSADC and WDT trigger a first global reset */
	clrsetbits_le32(&cru->glb_rst_con, 0x3, 0x3);

	return 0;
}

#ifdef CONFIG_DEBUG_UART_BOARD_INIT
void board_debug_uart_init(void)
{
#if defined(CONFIG_DEBUG_UART_BASE) && \
	(((CONFIG_DEBUG_UART_BASE == 0xff168000) && \
	(CONFIG_DEBUG_UART_CHANNEL != 1)) || \
	CONFIG_DEBUG_UART_BASE == 0xff030000)
	static struct px30_pmugrf * const pmugrf = (void *)PMUGRF_BASE;
#endif
#if !defined(CONFIG_DEBUG_UART_BASE) || \
	(CONFIG_DEBUG_UART_BASE != 0xff158000 && \
	 CONFIG_DEBUG_UART_BASE != 0xff168000 && \
	 CONFIG_DEBUG_UART_BASE != 0xff178000 && \
	 CONFIG_DEBUG_UART_BASE != 0xff030000) || \
	(defined(CONFIG_DEBUG_UART_BASE) && \
	 (CONFIG_DEBUG_UART_BASE == 0xff158000 || \
	  CONFIG_DEBUG_UART_BASE == 0xff168000 || \
	  CONFIG_DEBUG_UART_BASE == 0xff178000))
	static struct px30_grf * const grf = (void *)GRF_BASE;
	static struct px30_cru * const cru = (void *)CRU_BASE;
#endif
#if defined(CONFIG_DEBUG_UART_BASE) && CONFIG_DEBUG_UART_BASE == 0xff030000
	static struct px30_pmucru * const pmucru = (void *)PMUCRU_BASE;
#endif

#if defined(CONFIG_DEBUG_UART_BASE) && (CONFIG_DEBUG_UART_BASE == 0xff158000)
	/* uart_sel_clk default select 24MHz */
	rk_clrsetreg(&cru->clksel_con[34],
		     UART1_PLL_SEL_MASK | UART1_DIV_CON_MASK,
		     UART1_PLL_SEL_24M << UART1_PLL_SEL_SHIFT | 0);
	rk_clrsetreg(&cru->clksel_con[35],
		     UART1_CLK_SEL_MASK,
		     UART1_CLK_SEL_UART1 << UART1_CLK_SEL_SHIFT);

	rk_clrsetreg(&grf->gpio1cl_iomux,
		     GPIO1C1_MASK | GPIO1C0_MASK,
		     GPIO1C1_UART1_TX << GPIO1C1_SHIFT |
		     GPIO1C0_UART1_RX << GPIO1C0_SHIFT);
#elif defined(CONFIG_DEBUG_UART_BASE) && (CONFIG_DEBUG_UART_BASE == 0xff168000)
	/* GRF_IOFUNC_CON0 */
	enum {
		CON_IOMUX_UART3SEL_SHIFT	= 9,
		CON_IOMUX_UART3SEL_MASK = 1 << CON_IOMUX_UART3SEL_SHIFT,
		CON_IOMUX_UART3SEL_M0	= 0,
		CON_IOMUX_UART3SEL_M1,
	};

	/* uart_sel_clk default select 24MHz */
	rk_clrsetreg(&cru->clksel_con[40],
		     UART3_PLL_SEL_MASK | UART3_DIV_CON_MASK,
		     UART3_PLL_SEL_24M << UART3_PLL_SEL_SHIFT | 0);
	rk_clrsetreg(&cru->clksel_con[41],
		     UART3_CLK_SEL_MASK,
		     UART3_CLK_SEL_UART3 << UART3_CLK_SEL_SHIFT);

#if (CONFIG_DEBUG_UART_CHANNEL == 1)
	rk_clrsetreg(&grf->iofunc_con0,
		     CON_IOMUX_UART3SEL_MASK,
		     CON_IOMUX_UART3SEL_M1 << CON_IOMUX_UART3SEL_SHIFT);

	rk_clrsetreg(&grf->gpio1bh_iomux,
		     GPIO1B7_MASK | GPIO1B6_MASK,
		     GPIO1B7_UART3_RXM1 << GPIO1B7_SHIFT |
		     GPIO1B6_UART3_TXM1 << GPIO1B6_SHIFT);
#else
	rk_clrsetreg(&grf->iofunc_con0,
		     CON_IOMUX_UART3SEL_MASK,
		     CON_IOMUX_UART3SEL_M0 << CON_IOMUX_UART3SEL_SHIFT);

	rk_clrsetreg(&pmugrf->gpio0cl_iomux,
		     GPIO0C1_MASK | GPIO0C0_MASK,
		     GPIO0C1_UART3_RXM0 << GPIO0C1_SHIFT |
		     GPIO0C0_UART3_TXM0 << GPIO0C0_SHIFT);
#endif /* CONFIG_DEBUG_UART_CHANNEL == 1 */

#elif defined(CONFIG_DEBUG_UART_BASE) && (CONFIG_DEBUG_UART_BASE == 0xff178000)
	/* uart_sel_clk default select 24MHz */
	rk_clrsetreg(&cru->clksel_con[46],
		     UART5_PLL_SEL_MASK | UART5_DIV_CON_MASK,
		     UART5_PLL_SEL_24M << UART5_PLL_SEL_SHIFT | 0);
	rk_clrsetreg(&cru->clksel_con[47],
		     UART5_CLK_SEL_MASK,
		     UART5_CLK_SEL_UART5 << UART5_CLK_SEL_SHIFT);

	rk_clrsetreg(&grf->gpio3al_iomux,
		     GPIO3A2_MASK | GPIO3A1_MASK,
		     GPIO3A2_UART5_TX << GPIO3A2_SHIFT |
		     GPIO3A1_UART5_RX << GPIO3A1_SHIFT);
#elif defined(CONFIG_DEBUG_UART_BASE) && (CONFIG_DEBUG_UART_BASE == 0xff030000)
	/* uart_sel_clk default select 24MHz */
	rk_clrsetreg(&pmucru->pmu_clksel_con[3],
		     UART0_PLL_SEL_MASK | UART0_DIV_CON_MASK,
		     UART0_PLL_SEL_24M << UART0_PLL_SEL_SHIFT | 0);
	rk_clrsetreg(&pmucru->pmu_clksel_con[4],
		     UART0_CLK_SEL_MASK,
		     UART0_CLK_SEL_UART0 << UART0_CLK_SEL_SHIFT);

	rk_clrsetreg(&pmugrf->gpio0bl_iomux,
		     GPIO0B3_MASK | GPIO0B2_MASK,
		     GPIO0B3_UART0_RX << GPIO0B3_SHIFT |
		     GPIO0B2_UART0_TX << GPIO0B2_SHIFT);
#else
	/* GRF_IOFUNC_CON0 */
	enum {
		CON_IOMUX_UART2SEL_SHIFT	= 10,
		CON_IOMUX_UART2SEL_MASK = 3 << CON_IOMUX_UART2SEL_SHIFT,
		CON_IOMUX_UART2SEL_M0	= 0,
		CON_IOMUX_UART2SEL_M1,
		CON_IOMUX_UART2SEL_USBPHY,
	};

	/* uart_sel_clk default select 24MHz */
	rk_clrsetreg(&cru->clksel_con[37],
		     UART2_PLL_SEL_MASK | UART2_DIV_CON_MASK,
		     UART2_PLL_SEL_24M << UART2_PLL_SEL_SHIFT | 0);
	rk_clrsetreg(&cru->clksel_con[38],
		     UART2_CLK_SEL_MASK,
		     UART2_CLK_SEL_UART2 << UART2_CLK_SEL_SHIFT);

#if (CONFIG_DEBUG_UART_CHANNEL == 1)
	/* Enable early UART2 */
	rk_clrsetreg(&grf->iofunc_con0,
		     CON_IOMUX_UART2SEL_MASK,
		     CON_IOMUX_UART2SEL_M1 << CON_IOMUX_UART2SEL_SHIFT);

	rk_clrsetreg(&grf->gpio2bh_iomux,
		     GPIO2B6_MASK | GPIO2B4_MASK,
		     GPIO2B6_UART2_RXM1 << GPIO2B6_SHIFT |
		     GPIO2B4_UART2_TXM1 << GPIO2B4_SHIFT);
#else
	rk_clrsetreg(&grf->iofunc_con0,
		     CON_IOMUX_UART2SEL_MASK,
		     CON_IOMUX_UART2SEL_M0 << CON_IOMUX_UART2SEL_SHIFT);

	rk_clrsetreg(&grf->gpio1dl_iomux,
		     GPIO1D3_MASK | GPIO1D2_MASK,
		     GPIO1D3_UART2_RXM0 << GPIO1D3_SHIFT |
		     GPIO1D2_UART2_TXM0 << GPIO1D2_SHIFT);
#endif /* CONFIG_DEBUG_UART_CHANNEL == 1 */

#endif /* CONFIG_DEBUG_UART_BASE && CONFIG_DEBUG_UART_BASE == ... */
}
#endif /* CONFIG_DEBUG_UART_BOARD_INIT */
