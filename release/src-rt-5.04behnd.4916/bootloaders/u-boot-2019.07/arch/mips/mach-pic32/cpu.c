// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2015
 * Purna Chandra Mandal <purna.mandal@microchip.com>
 *
 */
#include <common.h>
#include <clk.h>
#include <dm.h>
#include <mach/pic32.h>
#include <mach/ddr.h>
#include <dt-bindings/clock/microchip,clock.h>

/* Flash prefetch */
#define PRECON          0x00

/* Flash ECCCON */
#define ECC_MASK	0x03
#define ECC_SHIFT	4

#define CLK_MHZ(x)	((x) / 1000000)

DECLARE_GLOBAL_DATA_PTR;

static ulong rate(int id)
{
	int ret;
	struct udevice *dev;
	struct clk clk;
	ulong rate;

	ret = uclass_get_device(UCLASS_CLK, 0, &dev);
	if (ret) {
		printf("clk-uclass not found\n");
		return 0;
	}

	clk.id = id;
	ret = clk_request(dev, &clk);
	if (ret < 0)
		return ret;

	rate = clk_get_rate(&clk);

	clk_free(&clk);

	return rate;
}

static ulong clk_get_cpu_rate(void)
{
	return rate(PB7CLK);
}

/* initialize prefetch module related to cpu_clk */
static void prefetch_init(void)
{
	struct pic32_reg_atomic *regs;
	const void __iomem *base;
	int v, nr_waits;
	ulong rate;

	/* cpu frequency in MHZ */
	rate = clk_get_cpu_rate() / 1000000;

	/* get flash ECC type */
	base = pic32_get_syscfg_base();
	v = (readl(base + CFGCON) >> ECC_SHIFT) & ECC_MASK;

	if (v < 2) {
		if (rate < 66)
			nr_waits = 0;
		else if (rate < 133)
			nr_waits = 1;
		else
			nr_waits = 2;
	} else {
		if (rate <= 83)
			nr_waits = 0;
		else if (rate <= 166)
			nr_waits = 1;
		else
			nr_waits = 2;
	}

	regs = ioremap(PREFETCH_BASE + PRECON, sizeof(*regs));
	writel(nr_waits, &regs->raw);

	/* Enable prefetch for all */
	writel(0x30, &regs->set);
	iounmap(regs);
}

/* arch specific CPU init after DM */
int arch_cpu_init_dm(void)
{
	/* flash prefetch */
	prefetch_init();
	return 0;
}

/* Un-gate DDR2 modules (gated by default) */
static void ddr2_pmd_ungate(void)
{
	void __iomem *regs;

	regs = pic32_get_syscfg_base();
	writel(0, regs + PMD7);
}

/* initialize the DDR2 Controller and DDR2 PHY */
int dram_init(void)
{
	ddr2_pmd_ungate();
	ddr2_phy_init();
	ddr2_ctrl_init();
	gd->ram_size = ddr2_calculate_size();

	return 0;
}

int misc_init_r(void)
{
	set_io_port_base(0);
	return 0;
}

#ifdef CONFIG_DISPLAY_BOARDINFO
const char *get_core_name(void)
{
	u32 proc_id;
	const char *str;

	proc_id = read_c0_prid();
	switch (proc_id) {
	case 0x19e28:
		str = "PIC32MZ[DA]";
		break;
	default:
		str = "UNKNOWN";
	}

	return str;
}
#endif
#ifdef CONFIG_CMD_CLK

int soc_clk_dump(void)
{
	int i;

	printf("PLL Speed: %lu MHz\n",
	       CLK_MHZ(rate(PLLCLK)));

	printf("CPU Speed: %lu MHz\n", CLK_MHZ(rate(PB7CLK)));

	printf("MPLL Speed: %lu MHz\n", CLK_MHZ(rate(MPLL)));

	for (i = PB1CLK; i <= PB7CLK; i++)
		printf("PB%d Clock Speed: %lu MHz\n", i - PB1CLK + 1,
		       CLK_MHZ(rate(i)));

	for (i = REF1CLK; i <= REF5CLK; i++)
		printf("REFO%d Clock Speed: %lu MHz\n", i - REF1CLK + 1,
		       CLK_MHZ(rate(i)));
	return 0;
}
#endif
