// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2009
 * Marvell Semiconductor <www.marvell.com>
 * Written-by: Prafulla Wadaskar <prafulla@marvell.com>
 */

#include <config.h>
#include <common.h>
#include <asm/io.h>
#include <asm/arch/cpu.h>
#include <asm/arch/soc.h>

#if defined(CONFIG_ARCH_MVEBU)
/* Use common XOR definitions for A3x and AXP */
#include "../../../drivers/ddr/marvell/axp/xor.h"
#include "../../../drivers/ddr/marvell/axp/xor_regs.h"
#endif

DECLARE_GLOBAL_DATA_PTR;

struct sdram_bank {
	u32	win_bar;
	u32	win_sz;
};

struct sdram_addr_dec {
	struct sdram_bank sdram_bank[4];
};

#define REG_CPUCS_WIN_ENABLE		(1 << 0)
#define REG_CPUCS_WIN_WR_PROTECT	(1 << 1)
#define REG_CPUCS_WIN_WIN0_CS(x)	(((x) & 0x3) << 2)
#define REG_CPUCS_WIN_SIZE(x)		(((x) & 0xff) << 24)

#ifndef MVEBU_SDRAM_SIZE_MAX
#define MVEBU_SDRAM_SIZE_MAX		0xc0000000
#endif

#define SCRUB_MAGIC		0xbeefdead

#define SCRB_XOR_UNIT		0
#define SCRB_XOR_CHAN		1
#define SCRB_XOR_WIN		0

#define XEBARX_BASE_OFFS	16

/*
 * mvebu_sdram_bar - reads SDRAM Base Address Register
 */
u32 mvebu_sdram_bar(enum memory_bank bank)
{
	struct sdram_addr_dec *base =
		(struct sdram_addr_dec *)MVEBU_SDRAM_BASE;
	u32 result = 0;
	u32 enable = 0x01 & readl(&base->sdram_bank[bank].win_sz);

	if ((!enable) || (bank > BANK3))
		return 0;

	result = readl(&base->sdram_bank[bank].win_bar);
	return result;
}

/*
 * mvebu_sdram_bs_set - writes SDRAM Bank size
 */
static void mvebu_sdram_bs_set(enum memory_bank bank, u32 size)
{
	struct sdram_addr_dec *base =
		(struct sdram_addr_dec *)MVEBU_SDRAM_BASE;
	/* Read current register value */
	u32 reg = readl(&base->sdram_bank[bank].win_sz);

	/* Clear window size */
	reg &= ~REG_CPUCS_WIN_SIZE(0xFF);

	/* Set new window size */
	reg |= REG_CPUCS_WIN_SIZE((size - 1) >> 24);

	writel(reg, &base->sdram_bank[bank].win_sz);
}

/*
 * mvebu_sdram_bs - reads SDRAM Bank size
 */
u32 mvebu_sdram_bs(enum memory_bank bank)
{
	struct sdram_addr_dec *base =
		(struct sdram_addr_dec *)MVEBU_SDRAM_BASE;
	u32 result = 0;
	u32 enable = 0x01 & readl(&base->sdram_bank[bank].win_sz);

	if ((!enable) || (bank > BANK3))
		return 0;
	result = 0xff000000 & readl(&base->sdram_bank[bank].win_sz);
	result += 0x01000000;
	return result;
}

void mvebu_sdram_size_adjust(enum memory_bank bank)
{
	u32 size;

	/* probe currently equipped RAM size */
	size = get_ram_size((void *)mvebu_sdram_bar(bank),
			    mvebu_sdram_bs(bank));

	/* adjust SDRAM window size accordingly */
	mvebu_sdram_bs_set(bank, size);
}

#if defined(CONFIG_ARCH_MVEBU)
static u32 xor_ctrl_save;
static u32 xor_base_save;
static u32 xor_mask_save;

static void mv_xor_init2(u32 cs)
{
	u32 reg, base, size, base2;
	u32 bank_attr[4] = { 0xe00, 0xd00, 0xb00, 0x700 };

	xor_ctrl_save = reg_read(XOR_WINDOW_CTRL_REG(SCRB_XOR_UNIT,
						     SCRB_XOR_CHAN));
	xor_base_save = reg_read(XOR_BASE_ADDR_REG(SCRB_XOR_UNIT,
						   SCRB_XOR_WIN));
	xor_mask_save = reg_read(XOR_SIZE_MASK_REG(SCRB_XOR_UNIT,
						   SCRB_XOR_WIN));

	/* Enable Window x for each CS */
	reg = 0x1;
	reg |= (0x3 << 16);
	reg_write(XOR_WINDOW_CTRL_REG(SCRB_XOR_UNIT, SCRB_XOR_CHAN), reg);

	base = 0;
	size = mvebu_sdram_bs(cs) - 1;
	if (size) {
		base2 = ((base / (64 << 10)) << XEBARX_BASE_OFFS) |
			bank_attr[cs];
		reg_write(XOR_BASE_ADDR_REG(SCRB_XOR_UNIT, SCRB_XOR_WIN),
			  base2);

		base += size + 1;
		size = (size / (64 << 10)) << 16;
		/* Window x - size - 256 MB */
		reg_write(XOR_SIZE_MASK_REG(SCRB_XOR_UNIT, SCRB_XOR_WIN), size);
	}

	mv_xor_hal_init(0);

	return;
}

static void mv_xor_finish2(void)
{
	reg_write(XOR_WINDOW_CTRL_REG(SCRB_XOR_UNIT, SCRB_XOR_CHAN),
		  xor_ctrl_save);
	reg_write(XOR_BASE_ADDR_REG(SCRB_XOR_UNIT, SCRB_XOR_WIN),
		  xor_base_save);
	reg_write(XOR_SIZE_MASK_REG(SCRB_XOR_UNIT, SCRB_XOR_WIN),
		  xor_mask_save);
}

static void dram_ecc_scrubbing(void)
{
	int cs;
	u32 size, temp;
	u32 total_mem = 0;
	u64 total;
	u32 start_addr;

	/*
	 * The DDR training code from the bin_hdr / SPL already
	 * scrubbed the DDR till 0x1000000. And the main U-Boot
	 * is loaded to an address < 0x1000000. So we need to
	 * skip this range to not re-scrub this area again.
	 */
	temp = reg_read(REG_SDRAM_CONFIG_ADDR);
	temp |= (1 << REG_SDRAM_CONFIG_IERR_OFFS);
	reg_write(REG_SDRAM_CONFIG_ADDR, temp);

	for (cs = 0; cs < CONFIG_NR_DRAM_BANKS; cs++) {
		size = mvebu_sdram_bs(cs);
		if (size == 0)
			continue;

		total = (u64)size;
		total_mem += (u32)(total / (1 << 30));
		start_addr = 0;
		mv_xor_init2(cs);

		/* Skip first 16 MiB */
		if (0 == cs) {
			start_addr = 0x1000000;
			size -= start_addr;
		}

		mv_xor_mem_init(SCRB_XOR_CHAN, start_addr, size - 1,
				SCRUB_MAGIC, SCRUB_MAGIC);

		/* Wait for previous transfer completion */
		while (mv_xor_state_get(SCRB_XOR_CHAN) != MV_IDLE)
			;

		mv_xor_finish2();
	}

	temp = reg_read(REG_SDRAM_CONFIG_ADDR);
	temp &= ~(1 << REG_SDRAM_CONFIG_IERR_OFFS);
	reg_write(REG_SDRAM_CONFIG_ADDR, temp);
}

static int ecc_enabled(void)
{
	if (reg_read(REG_SDRAM_CONFIG_ADDR) & (1 << REG_SDRAM_CONFIG_ECC_OFFS))
		return 1;

	return 0;
}

/* Return the width of the DRAM bus, or 0 for unknown. */
static int bus_width(void)
{
	int full_width = 0;

	if (reg_read(REG_SDRAM_CONFIG_ADDR) & (1 << REG_SDRAM_CONFIG_WIDTH_OFFS))
		full_width = 1;

	switch (mvebu_soc_family()) {
	case MVEBU_SOC_AXP:
	    return full_width ? 64 : 32;
	    break;
	case MVEBU_SOC_A375:
	case MVEBU_SOC_A38X:
	case MVEBU_SOC_MSYS:
	    return full_width ? 32 : 16;
	default:
	    return 0;
	}
}

static int cycle_mode(void)
{
	int val = reg_read(REG_DUNIT_CTRL_LOW_ADDR);

	return (val >> REG_DUNIT_CTRL_LOW_2T_OFFS) & REG_DUNIT_CTRL_LOW_2T_MASK;
}

#else
static void dram_ecc_scrubbing(void)
{
}

static int ecc_enabled(void)
{
	return 0;
}
#endif

int dram_init(void)
{
	u64 size = 0;
	int i;

	for (i = 0; i < CONFIG_NR_DRAM_BANKS; i++) {
		/*
		 * It is assumed that all memory banks are consecutive
		 * and without gaps.
		 * If the gap is found, ram_size will be reported for
		 * consecutive memory only
		 */
		if (mvebu_sdram_bar(i) != size)
			break;

		/*
		 * Don't report more than 3GiB of SDRAM, otherwise there is no
		 * address space left for the internal registers etc.
		 */
		size += mvebu_sdram_bs(i);
		if (size > MVEBU_SDRAM_SIZE_MAX)
			size = MVEBU_SDRAM_SIZE_MAX;
	}

	for (; i < CONFIG_NR_DRAM_BANKS; i++) {
		/* If above loop terminated prematurely, we need to set
		 * remaining banks' start address & size as 0. Otherwise other
		 * u-boot functions and Linux kernel gets wrong values which
		 * could result in crash */
		gd->bd->bi_dram[i].start = 0;
		gd->bd->bi_dram[i].size = 0;
	}


	if (ecc_enabled())
		dram_ecc_scrubbing();

	gd->ram_size = size;

	return 0;
}

/*
 * If this function is not defined here,
 * board.c alters dram bank zero configuration defined above.
 */
int dram_init_banksize(void)
{
	u64 size = 0;
	int i;

	for (i = 0; i < CONFIG_NR_DRAM_BANKS; i++) {
		gd->bd->bi_dram[i].start = mvebu_sdram_bar(i);
		gd->bd->bi_dram[i].size = mvebu_sdram_bs(i);

		/* Clip the banksize to 1GiB if it exceeds the max size */
		size += gd->bd->bi_dram[i].size;
		if (size > MVEBU_SDRAM_SIZE_MAX)
			mvebu_sdram_bs_set(i, 0x40000000);
	}

	return 0;
}

#if defined(CONFIG_ARCH_MVEBU)
void board_add_ram_info(int use_default)
{
	struct sar_freq_modes sar_freq;
	int mode;
	int width;

	get_sar_freq(&sar_freq);
	printf(" (%d MHz, ", sar_freq.d_clk);

	width = bus_width();
	if (width)
		printf("%d-bit, ", width);

	mode = cycle_mode();
	/* Mode 0 = Single cycle
	 * Mode 1 = Two cycles   (2T)
	 * Mode 2 = Three cycles (3T)
	 */
	if (mode == 1)
		printf("2T, ");
	if (mode == 2)
		printf("3T, ");

	if (ecc_enabled())
		printf("ECC");
	else
		printf("ECC not");
	printf(" enabled)");
}
#endif
