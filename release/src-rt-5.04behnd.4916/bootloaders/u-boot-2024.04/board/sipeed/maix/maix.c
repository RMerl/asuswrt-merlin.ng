// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2019-20 Sean Anderson <seanga2@gmail.com>
 */

#include <common.h>
#include <clk.h>
#include <dm.h>
#include <fdt_support.h>
#include <asm/io.h>

phys_size_t get_effective_memsize(void)
{
	return CFG_SYS_SDRAM_SIZE;
}

static int sram_init(void)
{
	int ret, i;
	const char * const banks[] = { "sram0", "sram1", "aisram" };
	ofnode memory;
	struct clk clk;

	/* Enable RAM clocks */
	memory = ofnode_by_compatible(ofnode_null(), "canaan,k210-sram");
	if (ofnode_equal(memory, ofnode_null()))
		return -ENOENT;

	for (i = 0; i < ARRAY_SIZE(banks); i++) {
		ret = clk_get_by_name_nodev(memory, banks[i], &clk);
		if (ret)
			continue;

		ret = clk_enable(&clk);
		if (ret)
			return ret;
	}

	return 0;
}

int board_early_init_f(void)
{
	return sram_init();
}

int board_init(void)
{
	return 0;
}
