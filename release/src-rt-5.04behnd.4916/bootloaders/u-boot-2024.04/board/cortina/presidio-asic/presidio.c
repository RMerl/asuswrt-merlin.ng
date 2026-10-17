// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2020 - Cortina Access Inc.
 *
 */
#include <common.h>
#include <event.h>
#include <init.h>
#include <malloc.h>
#include <errno.h>
#include <netdev.h>
#include <asm/global_data.h>
#include <asm/io.h>
#include <linux/bitops.h>
#include <linux/compiler.h>
#include <configs/presidio_asic.h>
#include <linux/psci.h>
#include <asm/psci.h>
#include <cpu_func.h>
#include <asm/armv8/mmu.h>

DECLARE_GLOBAL_DATA_PTR;

#define CA_PERIPH_BASE                  0xE0000000UL
#define CA_PERIPH_SIZE                  0x20000000UL
#define CA_GLOBAL_BASE                  0xf4320000
#define CA_GLOBAL_JTAG_ID               0xf4320000
#define CA_GLOBAL_BLOCK_RESET           0xf4320004
#define CA_GLOBAL_BLOCK_RESET_RESET_DMA BIT(16)
#define CA_DMA_SEC_SSP_BAUDRATE_CTRL    0xf7001b94
#define CA_DMA_SEC_SSP_ID               0xf7001b80

int print_cpuinfo(void)
{
	printf("CPU:   Cortina Presidio G3\n");
	return 0;
}

static struct mm_region presidio_mem_map[] = {
	{
	.virt = DDR_BASE,
	.phys = DDR_BASE,
	.size = PHYS_SDRAM_1_SIZE,
	.attrs = PTE_BLOCK_MEMTYPE(MT_NORMAL) |
		 PTE_BLOCK_OUTER_SHARE
	},
	{
	.virt = CA_PERIPH_BASE,
	.phys = CA_PERIPH_BASE,
	.size = CA_PERIPH_SIZE,
	.attrs = PTE_BLOCK_MEMTYPE(MT_DEVICE_NGNRNE) |
		 PTE_BLOCK_NON_SHARE
	},
	{
	/* List terminator */
	0,
	}
};

struct mm_region *mem_map = presidio_mem_map;

static noinline int invoke_psci_fn_smc(u64 function_id, u64 arg0, u64 arg1,
				       u64 arg2)
{
	asm volatile("mov x0, %0\n"
		    "mov x1, %1\n"
		    "mov x2, %2\n"
		    "mov x3, %3\n"
		    "smc	#0\n"
		    : "+r" (function_id)
		    : "r" (arg0), "r" (arg1), "r" (arg2)
		    );

	return function_id;
}

int board_early_init_r(void)
{
	dcache_disable();
	return 0;
}

int board_init(void)
{
	unsigned int reg_data, jtag_id;

	/* Enable timer */
	writel(1, CFG_SYS_TIMER_BASE);

	/* Enable snoop in CCI400 slave port#4 */
	writel(3, 0xF5595000);

	jtag_id = readl(CA_GLOBAL_JTAG_ID);

	/* If this is HGU variant then do not use
	 * the Saturn daughter card ref. clk
	 */
	if (jtag_id == 0x1010D8F3) {
		reg_data = readl(0xF3100064);
		/* change multifunc. REF CLK pin to
		 * a simple GPIO pin
		 */
		reg_data |= (1 << 1);
		writel(reg_data, 0xf3100064);
	}

	return 0;
}

int dram_init(void)
{
	unsigned int ddr_size;

	ddr_size = readl(0x111100c);
	gd->ram_size = ddr_size * 0x100000;
	return 0;
}

void reset_cpu(void)
{
	invoke_psci_fn_smc(PSCI_0_2_FN_SYSTEM_RESET, 0, 0, 0);
}

#ifdef CONFIG_LAST_STAGE_INIT
static int last_stage_init(void)
{
	u32 val;

	val = readl(CA_GLOBAL_BLOCK_RESET);
	val &= ~CA_GLOBAL_BLOCK_RESET_RESET_DMA;
	writel(val, CA_GLOBAL_BLOCK_RESET);

	/* reduce output pclk ~3.7Hz to save power consumption */
	writel(0x000000FF, CA_DMA_SEC_SSP_BAUDRATE_CTRL);

	return 0;
}
EVENT_SPY_SIMPLE(EVT_LAST_STAGE_INIT, last_stage_init);
#endif
