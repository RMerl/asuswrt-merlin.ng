// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2016 Rockchip Electronics Co., Ltd
 * Copyright (c) 2016 Andreas Färber
 */

#include <common.h>
#include <init.h>
#include <syscon.h>
#include <asm/armv8/mmu.h>
#include <asm/io.h>
#include <asm/arch-rockchip/bootrom.h>
#include <asm/arch-rockchip/clock.h>
#include <asm/arch-rockchip/cru_rk3368.h>
#include <asm/arch-rockchip/grf_rk3368.h>
#include <asm/arch-rockchip/hardware.h>
#include <linux/bitops.h>
#include <linux/delay.h>

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

const char * const boot_devices[BROM_LAST_BOOTSOURCE + 1] = {
	[BROM_BOOTSOURCE_EMMC] = "/mmc@ff0f0000",
	[BROM_BOOTSOURCE_SD] = "/mmc@ff0c0000",
};

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

#ifdef CONFIG_SPL_BUILD
/*
 * The SPL (and also the full U-Boot stage on the RK3368) will run in
 * secure mode (i.e. EL3) and an ATF will eventually be booted before
 * starting up the operating system... so we can initialize the SGRF
 * here and rely on the ATF installing the final (secure) policy
 * later.
 */
static inline uintptr_t sgrf_soc_con_addr(unsigned int no)
{
	const uintptr_t SGRF_BASE =
		(uintptr_t)syscon_get_first_range(ROCKCHIP_SYSCON_SGRF);

	return SGRF_BASE + sizeof(u32) * no;
}

static inline uintptr_t sgrf_busdmac_addr(unsigned int no)
{
	const uintptr_t SGRF_BASE =
		(uintptr_t)syscon_get_first_range(ROCKCHIP_SYSCON_SGRF);
	const uintptr_t SGRF_BUSDMAC_OFFSET = 0x100;
	const uintptr_t SGRF_BUSDMAC_BASE = SGRF_BASE + SGRF_BUSDMAC_OFFSET;

	return SGRF_BUSDMAC_BASE + sizeof(u32) * no;
}

static void sgrf_init(void)
{
	struct rk3368_cru * const cru =
		(struct rk3368_cru * const)rockchip_get_cru();
	const u16 SGRF_SOC_CON_SEC = GENMASK(15, 0);
	const u16 SGRF_BUSDMAC_CON0_SEC = BIT(2);
	const u16 SGRF_BUSDMAC_CON1_SEC = GENMASK(15, 12);

	/* Set all configurable IP to 'non secure'-mode */
	rk_setreg(sgrf_soc_con_addr(5), SGRF_SOC_CON_SEC);
	rk_setreg(sgrf_soc_con_addr(6), SGRF_SOC_CON_SEC);
	rk_setreg(sgrf_soc_con_addr(7), SGRF_SOC_CON_SEC);

	/*
	 * From rockchip-uboot/arch/arm/cpu/armv8/rk33xx/cpu.c
	 * Original comment: "ddr space set no secure mode"
	 */
	rk_clrreg(sgrf_soc_con_addr(8), SGRF_SOC_CON_SEC);
	rk_clrreg(sgrf_soc_con_addr(9), SGRF_SOC_CON_SEC);
	rk_clrreg(sgrf_soc_con_addr(10), SGRF_SOC_CON_SEC);

	/* Set 'secure dma' to 'non secure'-mode */
	rk_setreg(sgrf_busdmac_addr(0), SGRF_BUSDMAC_CON0_SEC);
	rk_setreg(sgrf_busdmac_addr(1), SGRF_BUSDMAC_CON1_SEC);

	dsb();  /* barrier */

	rk_setreg(&cru->softrst_con[1], DMA1_SRST_REQ);
	rk_setreg(&cru->softrst_con[4], DMA2_SRST_REQ);

	dsb();  /* barrier */
	udelay(10);

	rk_clrreg(&cru->softrst_con[1], DMA1_SRST_REQ);
	rk_clrreg(&cru->softrst_con[4], DMA2_SRST_REQ);
}

int arch_cpu_init(void)
{
	/* Reset security, so we can use DMA in the MMC drivers */
	sgrf_init();

	return 0;
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
