// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2016 Stefan Roese <sr@denx.de>
 * Copyright (C) 2020 Marek Behún <kabel@kernel.org>
 */

#include <common.h>
#include <cpu_func.h>
#include <dm.h>
#include <fdtdec.h>
#include <fdt_support.h>
#include <init.h>
#include <asm/global_data.h>
#include <linux/bitops.h>
#include <linux/libfdt.h>
#include <linux/sizes.h>
#include <asm/io.h>
#include <asm/system.h>
#include <asm/arch/cpu.h>
#include <asm/arch/soc.h>
#include <asm/armv8/mmu.h>
#include <sort.h>

/* Armada 3700 */
#define MVEBU_GPIO_NB_REG_BASE		(MVEBU_REGISTER(0x13800))

#define MVEBU_TEST_PIN_LATCH_N		(MVEBU_GPIO_NB_REG_BASE + 0x8)
#define MVEBU_XTAL_MODE_MASK		BIT(9)
#define MVEBU_XTAL_MODE_OFFS		9
#define MVEBU_XTAL_CLOCK_25MHZ		0x0
#define MVEBU_XTAL_CLOCK_40MHZ		0x1

#define MVEBU_NB_WARM_RST_REG		(MVEBU_GPIO_NB_REG_BASE + 0x40)
#define MVEBU_NB_WARM_RST_MAGIC_NUM	0x1d1e

/* Armada 3700 CPU Address Decoder registers */
#define MVEBU_CPU_DEC_WIN_REG_BASE	(size_t)(MVEBU_REGISTER(0xcf00))
#define MVEBU_CPU_DEC_WIN_CTRL(w) \
	(MVEBU_CPU_DEC_WIN_REG_BASE + ((w) << 4))
#define MVEBU_CPU_DEC_WIN_CTRL_EN	BIT(0)
#define MVEBU_CPU_DEC_WIN_CTRL_TGT_MASK	0xf
#define MVEBU_CPU_DEC_WIN_CTRL_TGT_OFFS	4
#define MVEBU_CPU_DEC_WIN_CTRL_TGT_DRAM	0
#define MVEBU_CPU_DEC_WIN_CTRL_TGT_PCIE	2
#define MVEBU_CPU_DEC_WIN_SIZE(w)	(MVEBU_CPU_DEC_WIN_CTRL(w) + 0x4)
#define MVEBU_CPU_DEC_WIN_BASE(w)	(MVEBU_CPU_DEC_WIN_CTRL(w) + 0x8)
#define MVEBU_CPU_DEC_WIN_REMAP(w)	(MVEBU_CPU_DEC_WIN_CTRL(w) + 0xc)
#define MVEBU_CPU_DEC_WIN_GRANULARITY	16
#define MVEBU_CPU_DEC_WINS		5
#define MVEBU_CPU_DEC_CCI_BASE		(MVEBU_CPU_DEC_WIN_REG_BASE + 0xe0)
#define MVEBU_CPU_DEC_ROM_BASE		(MVEBU_CPU_DEC_WIN_REG_BASE + 0xf4)

#define MAX_MEM_MAP_REGIONS		(MVEBU_CPU_DEC_WINS + 4)

#define A3700_PTE_BLOCK_NORMAL \
	(PTE_BLOCK_MEMTYPE(MT_NORMAL) | PTE_BLOCK_INNER_SHARE)
#define A3700_PTE_BLOCK_DEVICE \
	(PTE_BLOCK_MEMTYPE(MT_DEVICE_NGNRNE) | PTE_BLOCK_NON_SHARE)

DECLARE_GLOBAL_DATA_PTR;

static struct mm_region mvebu_mem_map[MAX_MEM_MAP_REGIONS] = {
	{
		/*
		 * SRAM, MMIO regions
		 * Don't remove this, build_mem_map needs it.
		 */
		.phys = SOC_REGS_PHY_BASE,
		.virt = SOC_REGS_PHY_BASE,
		.size = 0x02000000UL,	/* 32MiB internal registers */
		.attrs = A3700_PTE_BLOCK_DEVICE
	},
};

struct mm_region *mem_map = mvebu_mem_map;

static int get_cpu_dec_win(int win, u32 *tgt, u32 *base, u32 *size)
{
	u32 reg;

	reg = readl(MVEBU_CPU_DEC_WIN_CTRL(win));
	if (!(reg & MVEBU_CPU_DEC_WIN_CTRL_EN))
		return -1;

	if (tgt) {
		reg >>= MVEBU_CPU_DEC_WIN_CTRL_TGT_OFFS;
		reg &= MVEBU_CPU_DEC_WIN_CTRL_TGT_MASK;
		*tgt = reg;
	}

	if (base) {
		reg = readl(MVEBU_CPU_DEC_WIN_BASE(win));
		*base = reg << MVEBU_CPU_DEC_WIN_GRANULARITY;
	}

	if (size) {
		/*
		 * Window size is encoded as the number of 1s from LSB to MSB,
		 * followed by 0s. The number of 1s specifies the size in 64 KiB
		 * granularity.
		 */
		reg = readl(MVEBU_CPU_DEC_WIN_SIZE(win));
		*size = ((reg + 1) << MVEBU_CPU_DEC_WIN_GRANULARITY);
	}

	return 0;
}

/*
 * Builds mem_map according to CPU Address Decoder settings, which were set by
 * the TIMH image on the Cortex-M3 secure processor, or by ARM Trusted Firmware
 */
static void build_mem_map(void)
{
	int win, region;
	u32 reg;

	region = 1;

	/* CCI-400 */
	reg = readl(MVEBU_CPU_DEC_CCI_BASE);
	mvebu_mem_map[region].phys = reg << 20;
	mvebu_mem_map[region].virt = reg << 20;
	mvebu_mem_map[region].size = SZ_64K;
	mvebu_mem_map[region].attrs = A3700_PTE_BLOCK_DEVICE;
	++region;

	/* AP BootROM */
	reg = readl(MVEBU_CPU_DEC_ROM_BASE);
	mvebu_mem_map[region].phys = reg << 20;
	mvebu_mem_map[region].virt = reg << 20;
	mvebu_mem_map[region].size = SZ_1M;
	mvebu_mem_map[region].attrs = A3700_PTE_BLOCK_NORMAL;
	++region;

	for (win = 0; win < MVEBU_CPU_DEC_WINS; ++win) {
		u32 base, tgt, size;
		u64 attrs;

		/* skip disabled windows */
		if (get_cpu_dec_win(win, &tgt, &base, &size))
			continue;

		if (tgt == MVEBU_CPU_DEC_WIN_CTRL_TGT_DRAM)
			attrs = A3700_PTE_BLOCK_NORMAL;
		else if (tgt == MVEBU_CPU_DEC_WIN_CTRL_TGT_PCIE)
			attrs = A3700_PTE_BLOCK_DEVICE;
		else
			/* skip windows with other targets */
			continue;

		mvebu_mem_map[region].phys = base;
		mvebu_mem_map[region].virt = base;
		mvebu_mem_map[region].size = size;
		mvebu_mem_map[region].attrs = attrs;
		++region;
	}

	/* add list terminator */
	mvebu_mem_map[region].size = 0;
	mvebu_mem_map[region].attrs = 0;
}

void enable_caches(void)
{
	icache_enable();
	dcache_enable();
}

int a3700_dram_init(void)
{
	int win;

	build_mem_map();

	gd->ram_size = 0;
	for (win = 0; win < MVEBU_CPU_DEC_WINS; ++win) {
		u32 base, tgt, size;

		/* skip disabled windows */
		if (get_cpu_dec_win(win, &tgt, &base, &size))
			continue;

		/* skip non-DRAM windows */
		if (tgt != MVEBU_CPU_DEC_WIN_CTRL_TGT_DRAM)
			continue;

		/*
		 * It is possible that one image was built for boards with
		 * different RAM sizes, for example 512 MiB and 1 GiB.
		 * We therefore try to determine the actual RAM size in the
		 * window with get_ram_size.
		 */
		gd->ram_size += get_ram_size((void *)(size_t)base, size);
	}

	return 0;
}

struct a3700_dram_window {
	size_t base, size;
};

static int dram_win_cmp(const void *a, const void *b)
{
	size_t ab, bb;

	ab = ((const struct a3700_dram_window *)a)->base;
	bb = ((const struct a3700_dram_window *)b)->base;

	if (ab < bb)
		return -1;
	else if (ab > bb)
		return 1;
	else
		return 0;
}

int a3700_dram_init_banksize(void)
{
	struct a3700_dram_window dram_wins[MVEBU_CPU_DEC_WINS];
	int bank, win, ndram_wins;
	u32 last_end;
	size_t size;

	ndram_wins = 0;
	for (win = 0; win < MVEBU_CPU_DEC_WINS; ++win) {
		u32 base, tgt, size;

		/* skip disabled windows */
		if (get_cpu_dec_win(win, &tgt, &base, &size))
			continue;

		/* skip non-DRAM windows */
		if (tgt != MVEBU_CPU_DEC_WIN_CTRL_TGT_DRAM)
			continue;

		dram_wins[win].base = base;
		dram_wins[win].size = size;
		++ndram_wins;
	}

	qsort(dram_wins, ndram_wins, sizeof(dram_wins[0]), dram_win_cmp);

	bank = 0;
	last_end = -1;

	for (win = 0; win < ndram_wins; ++win) {
		/* again determining actual RAM size as in a3700_dram_init */
		size = get_ram_size((void *)dram_wins[win].base,
				    dram_wins[win].size);

		/*
		 * Check if previous window ends as the current starts. If yes,
		 * merge these windows into one "bank". This is possible by this
		 * simple check thanks to mem_map regions being qsorted in
		 * build_mem_map.
		 */
		if (last_end == dram_wins[win].base) {
			gd->bd->bi_dram[bank - 1].size += size;
			last_end += size;
		} else {
			if (bank == CONFIG_NR_DRAM_BANKS) {
				printf("Need more CONFIG_NR_DRAM_BANKS\n");
				return -ENOBUFS;
			}

			gd->bd->bi_dram[bank].start = dram_wins[win].base;
			gd->bd->bi_dram[bank].size = size;
			last_end = dram_wins[win].base + size;
			++bank;
		}
	}

	/*
	 * If there is more place for DRAM BANKS definitions than needed, fill
	 * the rest with zeros.
	 */
	for (; bank < CONFIG_NR_DRAM_BANKS; ++bank) {
		gd->bd->bi_dram[bank].start = 0;
		gd->bd->bi_dram[bank].size = 0;
	}

	return 0;
}

static u32 find_pcie_window_base(void)
{
	int win;

	for (win = 0; win < MVEBU_CPU_DEC_WINS; ++win) {
		u32 base, tgt;

		/* skip disabled windows */
		if (get_cpu_dec_win(win, &tgt, &base, NULL))
			continue;

		if (tgt == MVEBU_CPU_DEC_WIN_CTRL_TGT_PCIE)
			return base;
	}

	return -1;
}

static int fdt_setprop_inplace_u32_partial(void *blob, int node,
					   const char *name,
					   u32 idx, u32 val)
{
	val = cpu_to_fdt32(val);

	return fdt_setprop_inplace_namelen_partial(blob, node, name,
						   strlen(name),
						   idx * sizeof(u32),
						   &val, sizeof(u32));
}

int a3700_fdt_fix_pcie_regions(void *blob)
{
	u32 base, lowest_cpu_addr, fix_offset;
	int pci_cells, cpu_cells, size_cells;
	const u32 *ranges;
	int node, pnode;
	int ret, i, len;

	base = find_pcie_window_base();
	if (base == -1)
		return -ENOENT;

	node = fdt_node_offset_by_compatible(blob, -1, "marvell,armada-3700-pcie");
	if (node < 0)
		return node;

	ranges = fdt_getprop(blob, node, "ranges", &len);
	if (!ranges || !len || len % sizeof(u32))
		return -EINVAL;

	/*
	 * The "ranges" property is an array of
	 *   { <PCI address> <CPU address> <size in PCI address space> }
	 * where number of PCI address cells and size cells is stored in the
	 * "#address-cells" and "#size-cells" properties of the same node
	 * containing the "ranges" property and number of CPU address cells
	 * is stored in the parent's "#address-cells" property.
	 *
	 * All 3 elements can span a diffent number of cells. Fetch them.
	 */
	pnode = fdt_parent_offset(blob, node);
	pci_cells = fdt_address_cells(blob, node);
	cpu_cells = fdt_address_cells(blob, pnode);
	size_cells = fdt_size_cells(blob, node);

	/* PCI addresses always use 3 cells */
	if (pci_cells != 3)
		return -EINVAL;

	/* CPU addresses on Armada 37xx always use 2 cells */
	if (cpu_cells != 2)
		return -EINVAL;

	for (i = 0; i < len / sizeof(u32);
	     i += pci_cells + cpu_cells + size_cells) {
		/*
		 * Parent CPU addresses on Armada 37xx are always 32-bit, so
		 * check that the high word is zero.
		 */
		if (fdt32_to_cpu(ranges[i + pci_cells]))
			return -EINVAL;

		if (i == 0 ||
		    fdt32_to_cpu(ranges[i + pci_cells + 1]) < lowest_cpu_addr)
			lowest_cpu_addr = fdt32_to_cpu(ranges[i + pci_cells + 1]);
	}

	/* Calculate fixup offset from the lowest (first) CPU address */
	fix_offset = base - lowest_cpu_addr;

	/* If fixup offset is zero there is nothing to fix */
	if (!fix_offset)
		return 0;

	/*
	 * Fix each CPU address and corresponding PCI address if PCI address
	 * is not already remapped (has the same value)
	 */
	for (i = 0; i < len / sizeof(u32);
	     i += pci_cells + cpu_cells + size_cells) {
		u32 cpu_addr;
		u64 pci_addr;
		int idx;

		/* Fix CPU address */
		idx = i + pci_cells + cpu_cells - 1;
		cpu_addr = fdt32_to_cpu(ranges[idx]);
		ret = fdt_setprop_inplace_u32_partial(blob, node, "ranges", idx,
						      cpu_addr + fix_offset);
		if (ret)
			return ret;

		/* Fix PCI address only if it isn't remapped (is same as CPU) */
		idx = i + pci_cells - 1;
		pci_addr = ((u64)fdt32_to_cpu(ranges[idx - 1]) << 32) |
			   fdt32_to_cpu(ranges[idx]);
		if (cpu_addr != pci_addr)
			continue;

		ret = fdt_setprop_inplace_u32_partial(blob, node, "ranges", idx,
						      cpu_addr + fix_offset);
		if (ret)
			return ret;
	}

	return 0;
}

void reset_cpu(void)
{
	/*
	 * Write magic number of 0x1d1e to North Bridge Warm Reset register
	 * to trigger warm reset
	 */
	writel(MVEBU_NB_WARM_RST_MAGIC_NUM, MVEBU_NB_WARM_RST_REG);
}

/*
 * get_ref_clk
 *
 * return: reference clock in MHz (25 or 40)
 */
u32 get_ref_clk(void)
{
	u32 regval;

	regval = (readl(MVEBU_TEST_PIN_LATCH_N) & MVEBU_XTAL_MODE_MASK) >>
		MVEBU_XTAL_MODE_OFFS;

	if (regval == MVEBU_XTAL_CLOCK_25MHZ)
		return 25;
	else
		return 40;
}
