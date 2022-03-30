// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) Freescale Semiconductor, Inc. 2006-2007
 *
 * Authors: Nick.Spence@freescale.com
 *          Wilson.Lo@freescale.com
 *          scottwood@freescale.com
 */

#include <common.h>
#include <mpc83xx.h>
#include <spd_sdram.h>

#include <asm/bitops.h>
#include <asm/io.h>

#include <asm/processor.h>

DECLARE_GLOBAL_DATA_PTR;

#ifndef CONFIG_SYS_8313ERDB_BROKEN_PMC
static void resume_from_sleep(void)
{
	u32 magic = *(u32 *)0;

	typedef void (*func_t)(void);
	func_t resume = *(func_t *)4;

	if (magic == 0xf5153ae5)
		resume();

	gd->flags &= ~GD_FLG_SILENT;
	puts("\nResume from sleep failed: bad magic word\n");
}
#endif

/* Fixed sdram init -- doesn't use serial presence detect.
 *
 * This is useful for faster booting in configs where the RAM is unlikely
 * to be changed, or for things like NAND booting where space is tight.
 */
static long fixed_sdram(void)
{
	u32 msize = CONFIG_SYS_DDR_SIZE * 1024 * 1024;

#ifndef CONFIG_SYS_RAMBOOT
	volatile immap_t *im = (volatile immap_t *)CONFIG_SYS_IMMR;
	u32 msize_log2 = __ilog2(msize);

	im->sysconf.ddrlaw[0].bar = CONFIG_SYS_SDRAM_BASE & 0xfffff000;
	im->sysconf.ddrlaw[0].ar = LBLAWAR_EN | (msize_log2 - 1);
	im->sysconf.ddrcdr = CONFIG_SYS_DDRCDR_VALUE;

	/*
	 * Erratum DDR3 requires a 50ms delay after clearing DDRCDR[DDR_cfg],
	 * or the DDR2 controller may fail to initialize correctly.
	 */
	__udelay(50000);

#if ((CONFIG_SYS_SDRAM_BASE & 0x00FFFFFF) != 0)
#warning Chip select bounds is only configurable in 16MB increments
#endif
	im->ddr.csbnds[0].csbnds =
		((CONFIG_SYS_SDRAM_BASE >> CSBNDS_SA_SHIFT) & CSBNDS_SA) |
		(((CONFIG_SYS_SDRAM_BASE + msize - 1) >> CSBNDS_EA_SHIFT) &
			CSBNDS_EA);
	im->ddr.cs_config[0] = CONFIG_SYS_DDR_CS0_CONFIG;

	/* Currently we use only one CS, so disable the other bank. */
	im->ddr.cs_config[1] = 0;

	im->ddr.sdram_clk_cntl = CONFIG_SYS_DDR_CLK_CNTL;
	im->ddr.timing_cfg_3 = CONFIG_SYS_DDR_TIMING_3;
	im->ddr.timing_cfg_1 = CONFIG_SYS_DDR_TIMING_1;
	im->ddr.timing_cfg_2 = CONFIG_SYS_DDR_TIMING_2;
	im->ddr.timing_cfg_0 = CONFIG_SYS_DDR_TIMING_0;

#ifndef CONFIG_SYS_8313ERDB_BROKEN_PMC
	if (im->pmc.pmccr1 & PMCCR1_POWER_OFF)
		im->ddr.sdram_cfg = CONFIG_SYS_SDRAM_CFG | SDRAM_CFG_BI;
	else
#endif
		im->ddr.sdram_cfg = CONFIG_SYS_SDRAM_CFG;

	im->ddr.sdram_cfg2 = CONFIG_SYS_SDRAM_CFG2;
	im->ddr.sdram_mode = CONFIG_SYS_DDR_MODE;
	im->ddr.sdram_mode2 = CONFIG_SYS_DDR_MODE_2;

	im->ddr.sdram_interval = CONFIG_SYS_DDR_INTERVAL;
	sync();

	/* enable DDR controller */
	im->ddr.sdram_cfg |= SDRAM_CFG_MEM_EN;
#endif

	return msize;
}

int dram_init(void)
{
	volatile immap_t *im = (volatile immap_t *)CONFIG_SYS_IMMR;
	volatile fsl_lbc_t *lbc = &im->im_lbc;
	u32 msize;

	if ((im->sysconf.immrbar & IMMRBAR_BASE_ADDR) != (u32)im)
		return -ENXIO;

	/* DDR SDRAM - Main SODIMM */
	msize = fixed_sdram();

	/* Local Bus setup lbcr and mrtpr */
	lbc->lbcr = (0x00040000 | (0xFF << LBCR_BMT_SHIFT) | 0xF);
	/* LB refresh timer prescal, 266MHz/32 */
	lbc->mrtpr = 0x20000000;
	sync();

#ifndef CONFIG_SYS_8313ERDB_BROKEN_PMC
	if (im->pmc.pmccr1 & PMCCR1_POWER_OFF)
		resume_from_sleep();
#endif

	/* return total bus SDRAM size(bytes)  -- DDR */
	gd->ram_size = msize;

	return 0;
}
