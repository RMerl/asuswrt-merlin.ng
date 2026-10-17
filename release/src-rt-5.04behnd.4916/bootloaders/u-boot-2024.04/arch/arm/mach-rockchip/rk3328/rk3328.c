// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2016 Rockchip Electronics Co., Ltd
 */

#include <common.h>
#include <init.h>
#include <asm/arch-rockchip/bootrom.h>
#include <asm/arch-rockchip/hardware.h>
#include <asm/arch-rockchip/grf_rk3328.h>
#include <asm/arch-rockchip/uart.h>
#include <asm/armv8/mmu.h>
#include <asm/io.h>

#define CRU_BASE		0xFF440000
#define GRF_BASE		0xFF100000
#define UART2_BASE		0xFF130000
#define FW_DDR_CON_REG		0xFF7C0040
#define EFUSE_NS_BASE		0xFF260000

#define EFUSE_MOD		0x0000
#define EFUSE_INT_CON		0x0014
#define EFUSE_T_CSB_P		0x0028
#define EFUSE_T_PGENB_P		0x002C
#define EFUSE_T_LOAD_P		0x0030
#define EFUSE_T_ADDR_P		0x0034
#define EFUSE_T_STROBE_P	0x0038
#define EFUSE_T_CSB_R		0x003C
#define EFUSE_T_PGENB_R		0x0040
#define EFUSE_T_LOAD_R		0x0044
#define EFUSE_T_ADDR_R		0x0048
#define EFUSE_T_STROBE_R	0x004C

#define EFUSE_USER_MODE		0x1
#define EFUSE_TIMING(s, l)	(((s) << 16) | (l))

const char * const boot_devices[BROM_LAST_BOOTSOURCE + 1] = {
	[BROM_BOOTSOURCE_EMMC] = "/mmc@ff520000",
	[BROM_BOOTSOURCE_SD] = "/mmc@ff500000",
};

static struct mm_region rk3328_mem_map[] = {
	{
		.virt = 0x0UL,
		.phys = 0x0UL,
		.size = 0xff000000UL,
		.attrs = PTE_BLOCK_MEMTYPE(MT_NORMAL) |
			 PTE_BLOCK_INNER_SHARE
	}, {
		.virt = 0xff000000UL,
		.phys = 0xff000000UL,
		.size = 0x1000000UL,
		.attrs = PTE_BLOCK_MEMTYPE(MT_DEVICE_NGNRNE) |
			 PTE_BLOCK_NON_SHARE |
			 PTE_BLOCK_PXN | PTE_BLOCK_UXN
	}, {
		/* List terminator */
		0,
	}
};

struct mm_region *mem_map = rk3328_mem_map;

int arch_cpu_init(void)
{
#ifdef CONFIG_SPL_BUILD
	u32 reg;

	/* We do some SoC one time setting here. */

	/* Disable the ddr secure region setting to make it non-secure */
	rk_setreg(FW_DDR_CON_REG, 0x200);

	/* Use efuse auto mode */
	reg = readl(EFUSE_NS_BASE + EFUSE_MOD);
	writel(reg & ~EFUSE_USER_MODE, EFUSE_NS_BASE + EFUSE_MOD);

	/* Enable efuse finish and auto access err interrupt */
	writel(0x07, EFUSE_NS_BASE + EFUSE_INT_CON);

	/* Set efuse timing control */
	writel(EFUSE_TIMING(1, 241), EFUSE_NS_BASE + EFUSE_T_CSB_P);
	writel(EFUSE_TIMING(1, 241), EFUSE_NS_BASE + EFUSE_T_PGENB_P);
	writel(EFUSE_TIMING(1, 241), EFUSE_NS_BASE + EFUSE_T_LOAD_P);
	writel(EFUSE_TIMING(1, 241), EFUSE_NS_BASE + EFUSE_T_ADDR_P);
	writel(EFUSE_TIMING(2, 240), EFUSE_NS_BASE + EFUSE_T_STROBE_P);
	writel(EFUSE_TIMING(1, 4), EFUSE_NS_BASE + EFUSE_T_CSB_R);
	writel(EFUSE_TIMING(1, 4), EFUSE_NS_BASE + EFUSE_T_PGENB_R);
	writel(EFUSE_TIMING(1, 4), EFUSE_NS_BASE + EFUSE_T_LOAD_R);
	writel(EFUSE_TIMING(1, 4), EFUSE_NS_BASE + EFUSE_T_ADDR_R);
	writel(EFUSE_TIMING(2, 3), EFUSE_NS_BASE + EFUSE_T_STROBE_R);
#endif
	return 0;
}

void board_debug_uart_init(void)
{
	struct rk3328_grf_regs * const grf = (void *)GRF_BASE;
	struct rk_uart * const uart = (void *)UART2_BASE;
	enum{
		GPIO2A0_SEL_SHIFT       = 0,
		GPIO2A0_SEL_MASK        = 3 << GPIO2A0_SEL_SHIFT,
		GPIO2A0_UART2_TX_M1     = 1,

		GPIO2A1_SEL_SHIFT       = 2,
		GPIO2A1_SEL_MASK        = 3 << GPIO2A1_SEL_SHIFT,
		GPIO2A1_UART2_RX_M1     = 1,
	};
	enum {
		IOMUX_SEL_UART2_SHIFT   = 0,
		IOMUX_SEL_UART2_MASK    = 3 << IOMUX_SEL_UART2_SHIFT,
		IOMUX_SEL_UART2_M0      = 0,
		IOMUX_SEL_UART2_M1,
	};

	/* uart_sel_clk default select 24MHz */
	writel((3 << (8 + 16)) | (2 << 8), CRU_BASE + 0x148);

	/* init uart baud rate 1500000 */
	writel(0x83, &uart->lcr);
	writel(0x1, &uart->rbr);
	writel(0x3, &uart->lcr);

	/* Enable early UART2 */
	rk_clrsetreg(&grf->com_iomux,
		     IOMUX_SEL_UART2_MASK,
		     IOMUX_SEL_UART2_M1 << IOMUX_SEL_UART2_SHIFT);
	rk_clrsetreg(&grf->gpio2a_iomux,
		     GPIO2A0_SEL_MASK,
		     GPIO2A0_UART2_TX_M1 << GPIO2A0_SEL_SHIFT);
	rk_clrsetreg(&grf->gpio2a_iomux,
		     GPIO2A1_SEL_MASK,
		     GPIO2A1_UART2_RX_M1 << GPIO2A1_SEL_SHIFT);

	/* enable FIFO */
	writel(0x1, &uart->sfe);
}
