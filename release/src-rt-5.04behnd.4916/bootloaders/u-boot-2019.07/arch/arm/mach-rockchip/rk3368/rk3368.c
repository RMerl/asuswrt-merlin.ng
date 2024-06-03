// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2016 Rockchip Electronics Co., Ltd
 * Copyright (c) 2016 Andreas Färber
 */

#include <common.h>
#include <asm/armv8/mmu.h>
#include <asm/io.h>
#include <asm/arch-rockchip/clock.h>
#include <asm/arch-rockchip/cru_rk3368.h>
#include <asm/arch-rockchip/grf_rk3368.h>
#include <syscon.h>

DECLARE_GLOBAL_DATA_PTR;

#define IMEM_BASE                  0xFF8C0000

/* Max MCU's SRAM value is 8K, begin at (IMEM_BASE + 4K) */
#define MCU_SRAM_BASE			(IMEM_BASE + 1024 * 4)
#define MCU_SRAM_BASE_BIT31_BIT28	((MCU_SRAM_BASE & GENMASK(31, 28)) >> 28)
#define MCU_SRAM_BASE_BIT27_BIT12	((MCU_SRAM_BASE & GENMASK(27, 12)) >> 12)
/* exsram may using by mcu to accessing dram(0x0-0x20000000) */
#define MCU_EXSRAM_BASE    (0)
#define MCU_EXSRAM_BASE_BIT31_BIT28       ((MCU_EXSRAM_BASE & GENMASK(31, 28)) >> 28)
#define MCU_EXSRAM_BASE_BIT27_BIT12       ((MCU_EXSRAM_BASE & GENMASK(27, 12)) >> 12)
/* experi no used, reserved value = 0 */
#define MCU_EXPERI_BASE    (0)
#define MCU_EXPERI_BASE_BIT31_BIT28       ((MCU_EXPERI_BASE & GENMASK(31, 28)) >> 28)
#define MCU_EXPERI_BASE_BIT27_BIT12       ((MCU_EXPERI_BASE & GENMASK(27, 12)) >> 12)

static struct mm_region rk3368_mem_map[] = {
	{
		.virt = 0x0UL,
		.phys = 0x0UL,
		.size = 0x80000000UL,
		.attrs = PTE_BLOCK_MEMTYPE(MT_NORMAL) |
			 PTE_BLOCK_INNER_SHARE
	}, {
		.virt = 0xf0000000UL,
		.phys = 0xf0000000UL,
		.size = 0x10000000UL,
		.attrs = PTE_BLOCK_MEMTYPE(MT_DEVICE_NGNRNE) |
			 PTE_BLOCK_NON_SHARE |
			 PTE_BLOCK_PXN | PTE_BLOCK_UXN
	}, {
		/* List terminator */
		0,
	}
};

struct mm_region *mem_map = rk3368_mem_map;

int dram_init_banksize(void)
{
	size_t max_size = min((unsigned long)gd->ram_size, gd->ram_top);

	/* Reserve 0x200000 for ATF bl31 */
	gd->bd->bi_dram[0].start = 0x200000;
	gd->bd->bi_dram[0].size = max_size - gd->bd->bi_dram[0].start;

	return 0;
}

#ifdef CONFIG_ARCH_EARLY_INIT_R
static int mcu_init(void)
{
	struct rk3368_grf *grf = syscon_get_first_range(ROCKCHIP_SYSCON_GRF);
	struct rk3368_cru *cru = rockchip_get_cru();

	rk_clrsetreg(&grf->soc_con14, MCU_SRAM_BASE_BIT31_BIT28_MASK,
		     MCU_SRAM_BASE_BIT31_BIT28 << MCU_SRAM_BASE_BIT31_BIT28_SHIFT);
	rk_clrsetreg(&grf->soc_con11, MCU_SRAM_BASE_BIT27_BIT12_MASK,
		     MCU_SRAM_BASE_BIT27_BIT12 << MCU_SRAM_BASE_BIT27_BIT12_SHIFT);
	rk_clrsetreg(&grf->soc_con14, MCU_EXSRAM_BASE_BIT31_BIT28_MASK,
		     MCU_EXSRAM_BASE_BIT31_BIT28 << MCU_EXSRAM_BASE_BIT31_BIT28_SHIFT);
	rk_clrsetreg(&grf->soc_con12, MCU_EXSRAM_BASE_BIT27_BIT12_MASK,
		     MCU_EXSRAM_BASE_BIT27_BIT12 << MCU_EXSRAM_BASE_BIT27_BIT12_SHIFT);
	rk_clrsetreg(&grf->soc_con14, MCU_EXPERI_BASE_BIT31_BIT28_MASK,
		     MCU_EXPERI_BASE_BIT31_BIT28 << MCU_EXPERI_BASE_BIT31_BIT28_SHIFT);
	rk_clrsetreg(&grf->soc_con13, MCU_EXPERI_BASE_BIT27_BIT12_MASK,
		     MCU_EXPERI_BASE_BIT27_BIT12 << MCU_EXPERI_BASE_BIT27_BIT12_SHIFT);

	rk_clrsetreg(&cru->clksel_con[12], MCU_PLL_SEL_MASK | MCU_CLK_DIV_MASK,
		     (MCU_PLL_SEL_GPLL << MCU_PLL_SEL_SHIFT) |
		     (5 << MCU_CLK_DIV_SHIFT));

	 /* mcu dereset, for start running */
	rk_clrreg(&cru->softrst_con[1], MCU_PO_SRST_MASK | MCU_SYS_SRST_MASK);

	return 0;
}

int arch_early_init_r(void)
{
	return mcu_init();
}
#endif

#ifdef CONFIG_DEBUG_UART_BOARD_INIT
void board_debug_uart_init(void)
{
	/*
	 * N.B.: This is called before the device-model has been
	 *       initialised. For this reason, we can not access
	 *       the GRF address range using the syscon API.
	 */
#if defined(CONFIG_DEBUG_UART_BASE) && (CONFIG_DEBUG_UART_BASE == 0xff180000)
	struct rk3368_grf * const grf =
		(struct rk3368_grf * const)0xff770000;

	enum {
		GPIO2D1_MASK            = GENMASK(3, 2),
		GPIO2D1_GPIO            = 0,
		GPIO2D1_UART0_SOUT      = (1 << 2),

		GPIO2D0_MASK            = GENMASK(1, 0),
		GPIO2D0_GPIO            = 0,
		GPIO2D0_UART0_SIN       = (1 << 0),
	};

	/* Enable early UART0 on the RK3368 */
	rk_clrsetreg(&grf->gpio2d_iomux,
		     GPIO2D0_MASK, GPIO2D0_UART0_SIN);
	rk_clrsetreg(&grf->gpio2d_iomux,
		     GPIO2D1_MASK, GPIO2D1_UART0_SOUT);
#elif defined(CONFIG_DEBUG_UART_BASE) && (CONFIG_DEBUG_UART_BASE == 0xff1c0000)
	struct rk3368_pmu_grf * const pmugrf __maybe_unused =
		(struct rk3368_pmu_grf * const)0xff738000;

	enum {
		/* UART4 */
		GPIO0D2_MASK		= GENMASK(5, 4),
		GPIO0D2_GPIO		= 0,
		GPIO0D2_UART4_SOUT	= (3 << 4),

		GPIO0D3_MASK		= GENMASK(7, 6),
		GPIO0D3_GPIO		= 0,
		GPIO0D3_UART4_SIN	= (3 << 6),
	};

	/* Enable early UART4 on the PX5 */
	rk_clrsetreg(&pmugrf->gpio0d_iomux,
		     GPIO0D2_MASK | GPIO0D3_MASK,
		     GPIO0D2_UART4_SOUT | GPIO0D3_UART4_SIN);
#elif defined(CONFIG_DEBUG_UART_BASE) && (CONFIG_DEBUG_UART_BASE == 0xff690000)
	struct rk3368_grf * const grf =
		(struct rk3368_grf * const)0xff770000;

	enum {
		GPIO2A6_SHIFT           = 12,
		GPIO2A6_MASK            = GENMASK(13, 12),
		GPIO2A6_GPIO            = 0,
		GPIO2A6_UART2_SIN       = (2 << GPIO2A6_SHIFT),

		GPIO2A5_SHIFT           = 10,
		GPIO2A5_MASK            = GENMASK(11, 10),
		GPIO2A5_GPIO            = 0,
		GPIO2A5_UART2_SOUT      = (2 << GPIO2A5_SHIFT),
	};

	/* Enable early UART2 on the RK3368 */
	rk_clrsetreg(&grf->gpio2a_iomux,
		     GPIO2A6_MASK, GPIO2A6_UART2_SIN);
	rk_clrsetreg(&grf->gpio2a_iomux,
		     GPIO2A5_MASK, GPIO2A5_UART2_SOUT);
#endif
}
#endif
