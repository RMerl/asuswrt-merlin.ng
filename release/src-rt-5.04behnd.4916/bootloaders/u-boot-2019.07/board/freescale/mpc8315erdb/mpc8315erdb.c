// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2007 Freescale Semiconductor, Inc.
 *
 * Author: Scott Wood <scottwood@freescale.com>
 *         Dave Liu <daveliu@freescale.com>
 */

#include <common.h>
#include <hwconfig.h>
#include <i2c.h>
#include <linux/libfdt.h>
#include <fdt_support.h>
#include <pci.h>
#include <mpc83xx.h>
#include <netdev.h>
#include <asm/io.h>
#include <ns16550.h>
#include <nand.h>

DECLARE_GLOBAL_DATA_PTR;

int board_early_init_f(void)
{
	volatile immap_t *im = (immap_t *)CONFIG_SYS_IMMR;

	if (im->pmc.pmccr1 & PMCCR1_POWER_OFF)
		gd->flags |= GD_FLG_SILENT;

	return 0;
}

#ifndef CONFIG_NAND_SPL

static u8 read_board_info(void)
{
	u8 val8;
	i2c_set_bus_num(0);

	if (i2c_read(CONFIG_SYS_I2C_PCF8574A_ADDR, 0, 0, &val8, 1) == 0)
		return val8;
	else
		return 0;
}

int checkboard(void)
{
	static const char * const rev_str[] = {
		"0.0",
		"0.1",
		"1.0",
		"1.1",
		"<unknown>",
	};
	u8 info;
	int i;

	info = read_board_info();
	i = (!info) ? 4: info & 0x03;

	printf("Board: Freescale MPC8315ERDB Rev %s\n", rev_str[i]);

	return 0;
}

static struct pci_region pci_regions[] = {
	{
		bus_start: CONFIG_SYS_PCI_MEM_BASE,
		phys_start: CONFIG_SYS_PCI_MEM_PHYS,
		size: CONFIG_SYS_PCI_MEM_SIZE,
		flags: PCI_REGION_MEM | PCI_REGION_PREFETCH
	},
	{
		bus_start: CONFIG_SYS_PCI_MMIO_BASE,
		phys_start: CONFIG_SYS_PCI_MMIO_PHYS,
		size: CONFIG_SYS_PCI_MMIO_SIZE,
		flags: PCI_REGION_MEM
	},
	{
		bus_start: CONFIG_SYS_PCI_IO_BASE,
		phys_start: CONFIG_SYS_PCI_IO_PHYS,
		size: CONFIG_SYS_PCI_IO_SIZE,
		flags: PCI_REGION_IO
	}
};

static struct pci_region pcie_regions_0[] = {
	{
		.bus_start = CONFIG_SYS_PCIE1_MEM_BASE,
		.phys_start = CONFIG_SYS_PCIE1_MEM_PHYS,
		.size = CONFIG_SYS_PCIE1_MEM_SIZE,
		.flags = PCI_REGION_MEM,
	},
	{
		.bus_start = CONFIG_SYS_PCIE1_IO_BASE,
		.phys_start = CONFIG_SYS_PCIE1_IO_PHYS,
		.size = CONFIG_SYS_PCIE1_IO_SIZE,
		.flags = PCI_REGION_IO,
	},
};

static struct pci_region pcie_regions_1[] = {
	{
		.bus_start = CONFIG_SYS_PCIE2_MEM_BASE,
		.phys_start = CONFIG_SYS_PCIE2_MEM_PHYS,
		.size = CONFIG_SYS_PCIE2_MEM_SIZE,
		.flags = PCI_REGION_MEM,
	},
	{
		.bus_start = CONFIG_SYS_PCIE2_IO_BASE,
		.phys_start = CONFIG_SYS_PCIE2_IO_PHYS,
		.size = CONFIG_SYS_PCIE2_IO_SIZE,
		.flags = PCI_REGION_IO,
	},
};

void pci_init_board(void)
{
	volatile immap_t *immr = (volatile immap_t *)CONFIG_SYS_IMMR;
	volatile sysconf83xx_t *sysconf = &immr->sysconf;
	volatile clk83xx_t *clk = (volatile clk83xx_t *)&immr->clk;
	volatile law83xx_t *pci_law = immr->sysconf.pcilaw;
	volatile law83xx_t *pcie_law = sysconf->pcielaw;
	struct pci_region *reg[] = { pci_regions };
	struct pci_region *pcie_reg[] = { pcie_regions_0, pcie_regions_1, };

	/* Enable all 3 PCI_CLK_OUTPUTs. */
	clk->occr |= 0xe0000000;

	/*
	 * Configure PCI Local Access Windows
	 */
	pci_law[0].bar = CONFIG_SYS_PCI_MEM_PHYS & LAWBAR_BAR;
	pci_law[0].ar = LBLAWAR_EN | LBLAWAR_512MB;

	pci_law[1].bar = CONFIG_SYS_PCI_IO_PHYS & LAWBAR_BAR;
	pci_law[1].ar = LBLAWAR_EN | LBLAWAR_1MB;

	mpc83xx_pci_init(1, reg);

	/* Configure the clock for PCIE controller */
	clrsetbits_be32(&clk->sccr, SCCR_PCIEXP1CM | SCCR_PCIEXP2CM,
				    SCCR_PCIEXP1CM_1 | SCCR_PCIEXP2CM_1);

	/* Deassert the resets in the control register */
	out_be32(&sysconf->pecr1, 0xE0008000);
	out_be32(&sysconf->pecr2, 0xE0008000);
	udelay(2000);

	/* Configure PCI Express Local Access Windows */
	out_be32(&pcie_law[0].bar, CONFIG_SYS_PCIE1_BASE & LAWBAR_BAR);
	out_be32(&pcie_law[0].ar, LBLAWAR_EN | LBLAWAR_512MB);

	out_be32(&pcie_law[1].bar, CONFIG_SYS_PCIE2_BASE & LAWBAR_BAR);
	out_be32(&pcie_law[1].ar, LBLAWAR_EN | LBLAWAR_512MB);

	mpc83xx_pcie_init(2, pcie_reg);
}

#if defined(CONFIG_OF_BOARD_SETUP)
void fdt_tsec1_fixup(void *fdt, bd_t *bd)
{
	const char disabled[] = "disabled";
	const char *path;
	int ret;

	if (hwconfig_arg_cmp("board_type", "tsec1")) {
		return;
	} else if (!hwconfig_arg_cmp("board_type", "ulpi")) {
		printf("NOTICE: No or unknown board_type hwconfig specified.\n"
		       "        Assuming board with TSEC1.\n");
		return;
	}

	ret = fdt_path_offset(fdt, "/aliases");
	if (ret < 0) {
		printf("WARNING: can't find /aliases node\n");
		return;
	}

	path = fdt_getprop(fdt, ret, "ethernet0", NULL);
	if (!path) {
		printf("WARNING: can't find ethernet0 alias\n");
		return;
	}

	do_fixup_by_path(fdt, path, "status", disabled, sizeof(disabled), 1);
}

int ft_board_setup(void *blob, bd_t *bd)
{
	ft_cpu_setup(blob, bd);
#ifdef CONFIG_PCI
	ft_pci_setup(blob, bd);
#endif
	fsl_fdt_fixup_dr_usb(blob, bd);
	fdt_tsec1_fixup(blob, bd);

	return 0;
}
#endif

int board_eth_init(bd_t *bis)
{
	cpu_eth_init(bis);	/* Initialize TSECs first */
	return pci_eth_init(bis);
}

#else /* CONFIG_NAND_SPL */

int checkboard(void)
{
	puts("Board: Freescale MPC8315ERDB\n");
	return 0;
}

void board_init_f(ulong bootflag)
{
	board_early_init_f();
	NS16550_init((NS16550_t)(CONFIG_SYS_IMMR + 0x4500),
		     CONFIG_SYS_NS16550_CLK / 16 / CONFIG_BAUDRATE);
	puts("NAND boot... ");
	timer_init();
	dram_init();
	relocate_code(CONFIG_SYS_NAND_U_BOOT_RELOC + 0x10000, (gd_t *)gd,
		      CONFIG_SYS_NAND_U_BOOT_RELOC);
}

void board_init_r(gd_t *gd, ulong dest_addr)
{
	nand_boot();
}

void putc(char c)
{
	if (gd->flags & GD_FLG_SILENT)
		return;

	if (c == '\n')
		NS16550_putc((NS16550_t)(CONFIG_SYS_IMMR + 0x4500), '\r');

	NS16550_putc((NS16550_t)(CONFIG_SYS_IMMR + 0x4500), c);
}

#endif /* CONFIG_NAND_SPL */
