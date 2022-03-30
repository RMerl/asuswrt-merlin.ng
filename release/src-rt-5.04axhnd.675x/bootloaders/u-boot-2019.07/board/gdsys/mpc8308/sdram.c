// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2007 Freescale Semiconductor, Inc.
 * Copyright (C) 2010 Ilya Yanok, Emcraft Systems, yanok@emcraft.com
 *
 * Authors: Nick.Spence@freescale.com
 *          Wilson.Lo@freescale.com
 *          scottwood@freescale.com
 *
 * This files is  mostly identical to the original from
 * board\freescale\mpc8315erdb\sdram.c
 */

#ifndef CONFIG_MPC83XX_SDRAM

#include <common.h>
#include <mpc83xx.h>
#include <spd_sdram.h>

#include <asm/bitops.h>
#include <asm/io.h>

#include <asm/processor.h>

DECLARE_GLOBAL_DATA_PTR;

/* Fixed sdram init -- doesn't use serial presence detect.
 *
 * This is useful for faster booting in configs where the RAM is unlikely
 * to be changed, or for things like NAND booting where space is tight.
 */
static long fixed_sdram(void)
{
	immap_t *im = (immap_t *)CONFIG_SYS_IMMR;
	u32 msize = CONFIG_SYS_DDR_SIZE * 1024 * 1024;
	u32 msize_log2 = __ilog2(msize);

	out_be32(&im->sysconf.ddrlaw[0].bar,
		 CONFIG_SYS_SDRAM_BASE  & 0xfffff000);
	out_be32(&im->sysconf.ddrlaw[0].ar, LBLAWAR_EN | (msize_log2 - 1));
	out_be32(&im->sysconf.ddrcdr, CONFIG_SYS_DDRCDR_VALUE);

	out_be32(&im->ddr.csbnds[0].csbnds, (msize - 1) >> 24);
	out_be32(&im->ddr.cs_config[0], CONFIG_SYS_DDR_CS0_CONFIG);

	/* Currently we use only one CS, so disable the other bank. */
	out_be32(&im->ddr.cs_config[1], 0);

	out_be32(&im->ddr.sdram_clk_cntl, CONFIG_SYS_DDR_SDRAM_CLK_CNTL);
	out_be32(&im->ddr.timing_cfg_3, CONFIG_SYS_DDR_TIMING_3);
	out_be32(&im->ddr.timing_cfg_1, CONFIG_SYS_DDR_TIMING_1);
	out_be32(&im->ddr.timing_cfg_2, CONFIG_SYS_DDR_TIMING_2);
	out_be32(&im->ddr.timing_cfg_0, CONFIG_SYS_DDR_TIMING_0);

	out_be32(&im->ddr.sdram_cfg, CONFIG_SYS_DDR_SDRAM_CFG);
	out_be32(&im->ddr.sdram_cfg2, CONFIG_SYS_DDR_SDRAM_CFG2);
	out_be32(&im->ddr.sdram_mode, CONFIG_SYS_DDR_MODE);
	out_be32(&im->ddr.sdram_mode2, CONFIG_SYS_DDR_MODE2);

	out_be32(&im->ddr.sdram_interval, CONFIG_SYS_DDR_INTERVAL);
	sync();

	/* enable DDR controller */
	setbits_be32(&im->ddr.sdram_cfg, SDRAM_CFG_MEM_EN);
	sync();

	return get_ram_size(CONFIG_SYS_SDRAM_BASE, msize);
}

int dram_init(void)
{
	immap_t *im = (immap_t *)CONFIG_SYS_IMMR;
	u32 msize;

	if ((in_be32(&im->sysconf.immrbar) & IMMRBAR_BASE_ADDR) != (u32)im)
		return -ENXIO;

	/* DDR SDRAM */
	msize = fixed_sdram();

	/* return total bus SDRAM size(bytes)  -- DDR */
	gd->ram_size = msize;

	return 0;
}

#endif /* !CONFIG_MPC83XX_SDRAM */
