// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2019 Linaro
 * Author: Manivannan Sadhasivam <manivannan.sadhasivam@linaro.org>
 */

#include <common.h>
#include <cpu_func.h>
#include <dm.h>
#include <errno.h>
#include <asm/cache.h>
#include <init.h>
#include <asm/global_data.h>
#include <asm/io.h>
#include <asm/arch/hi3660.h>
#include <asm/armv8/mmu.h>
#include <asm/psci.h>
#include <linux/arm-smccc.h>
#include <linux/delay.h>
#include <linux/psci.h>

#define PMIC_REG_TO_BUS_ADDR(x) (x << 2)
#define PMIC_VSEL_MASK		0x7

DECLARE_GLOBAL_DATA_PTR;

#if !CONFIG_IS_ENABLED(OF_CONTROL)
#include <dm/platform_data/serial_pl01x.h>

static const struct pl01x_serial_plat serial_plat = {
	.base = HI3660_UART6_BASE,
	.type = TYPE_PL011,
	.clock = 19200000
};

U_BOOT_DRVINFO(hikey960_serial0) = {
	.name = "serial_pl01x",
	.plat = &serial_plat,
};
#endif

static struct mm_region hikey_mem_map[] = {
	{
		.virt = 0x0UL, /* DDR */
		.phys = 0x0UL,
		.size = 0xC0000000UL,
		.attrs = PTE_BLOCK_MEMTYPE(MT_NORMAL) |
			 PTE_BLOCK_INNER_SHARE
	}, {
		.virt = 0xE0000000UL, /* Peripheral block */
		.phys = 0xE0000000UL,
		.size = 0x20000000UL,
		.attrs = PTE_BLOCK_MEMTYPE(MT_DEVICE_NGNRNE) |
			 PTE_BLOCK_NON_SHARE |
			 PTE_BLOCK_PXN | PTE_BLOCK_UXN
	}, {
		/* List terminator */
		0,
	}
};

struct mm_region *mem_map = hikey_mem_map;

int board_early_init_f(void)
{
	return 0;
}

int misc_init_r(void)
{
	return 0;
}

int dram_init(void)
{
	gd->ram_size = PHYS_SDRAM_1_SIZE;

	return 0;
}

int dram_init_banksize(void)
{
	gd->bd->bi_dram[0].start = PHYS_SDRAM_1;
	gd->bd->bi_dram[0].size = gd->ram_size;

	return 0;
}

void hikey960_sd_init(void)
{
	u32 data;

	/* Enable FPLL0 */
	data = readl(SCTRL_SCFPLLCTRL0);
	data |= SCTRL_SCFPLLCTRL0_FPLL0_EN;
	writel(data, SCTRL_SCFPLLCTRL0);

	/* Configure LDO16 */
	data = readl(PMU_REG_BASE + PMIC_REG_TO_BUS_ADDR(0x79)) &
		     PMIC_VSEL_MASK;
	data |= 6;
	writel(data, PMU_REG_BASE + PMIC_REG_TO_BUS_ADDR(0x79));

	data = readl(PMU_REG_BASE + PMIC_REG_TO_BUS_ADDR(0x78));
	data |= 2;
	writel(data, PMU_REG_BASE + PMIC_REG_TO_BUS_ADDR(0x78));

	udelay(100);

	/* Configure LDO9 */
	data = readl(PMU_REG_BASE + PMIC_REG_TO_BUS_ADDR(0x6b)) &
		     PMIC_VSEL_MASK;
	data |= 5;
	writel(data, PMU_REG_BASE + PMIC_REG_TO_BUS_ADDR(0x6b));

	data = readl(PMU_REG_BASE + PMIC_REG_TO_BUS_ADDR(0x6a));
	data |= 2;
	writel(data, PMU_REG_BASE + PMIC_REG_TO_BUS_ADDR(0x6a));

	udelay(100);

	/* GPIO CD */
	writel(0, PINMUX4_SDDET);

	/* SD Pinconf */
	writel(15 << 4, PINCONF3_SDCLK);
	writel((1 << 0) | (8 << 4), PINCONF3_SDCMD);
	writel((1 << 0) | (8 << 4), PINCONF3_SDDATA0);
	writel((1 << 0) | (8 << 4), PINCONF3_SDDATA1);
	writel((1 << 0) | (8 << 4), PINCONF3_SDDATA2);
	writel((1 << 0) | (8 << 4), PINCONF3_SDDATA3);

	/* Set SD clock mux */
	do {
		data = readl(CRG_REG_BASE + 0xb8);
		data |= ((1 << 6) | (1 << 6 << 16) | (0 << 4) | (3 << 4 << 16));
		writel(data, CRG_REG_BASE + 0xb8);

		data = readl(CRG_REG_BASE + 0xb8);
	} while ((data & ((1 << 6) | (3 << 4))) != ((1 << 6) | (0 << 4)));

	/* Take SD out of reset */
	writel(1 << 18, CRG_PERRSTDIS4);
	do {
		data = readl(CRG_PERRSTSTAT4);
	} while ((data & (1 << 18)) == (1 << 18));

	/* Enable hclk_gate_sd */
	data = readl(CRG_REG_BASE + 0);
	data |= (1 << 30);
	writel(data, CRG_REG_BASE + 0);

	/* Enable clk_andgt_mmc */
	data = readl(CRG_REG_BASE + 0xf4);
	data |= ((1 << 3) | (1 << 3 << 16));
	writel(data, CRG_REG_BASE + 0xf4);

	/* Enable clk_gate_sd */
	data = readl(CRG_PEREN4);
	data |= (1 << 17);
	writel(data, CRG_PEREN4);
	do {
		data = readl(CRG_PERCLKEN4);
	} while ((data & (1 << 17)) != (1 << 17));
}

static void show_psci_version(void)
{
	struct arm_smccc_res res;

	arm_smccc_smc(ARM_PSCI_0_2_FN_PSCI_VERSION, 0, 0, 0, 0, 0, 0, 0, &res);

	printf("PSCI:  v%ld.%ld\n",
	       PSCI_VERSION_MAJOR(res.a0),
		PSCI_VERSION_MINOR(res.a0));
}

int board_init(void)
{
	/* Init SD */
	hikey960_sd_init();

	show_psci_version();

	return 0;
}

void reset_cpu(void)
{
	psci_system_reset();
}
