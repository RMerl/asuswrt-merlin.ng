// SPDX-License-Identifier: GPL-2.0+
/*
 * Functions related to OMAP3 SDRC.
 *
 * This file has been created after exctracting and consolidating
 * the SDRC related content from mem.c and board.c, also created
 * generic init function (mem_init).
 *
 * Copyright (C) 2004-2010
 * Texas Instruments Incorporated - http://www.ti.com/
 *
 * Copyright (C) 2011
 * Corscience GmbH & Co. KG - Simon Schwarz <schwarz@corscience.de>
 *
 * Author :
 *     Vaibhav Hiremath <hvaibhav@ti.com>
 *
 * Original implementation by (mem.c, board.c) :
 *      Sunil Kumar <sunilsaini05@gmail.com>
 *      Shashi Ranjan <shashiranjanmca05@gmail.com>
 *      Manikandan Pillai <mani.pillai@ti.com>
 */

#include <common.h>
#include <asm/io.h>
#include <asm/arch/mem.h>
#include <asm/arch/sys_proto.h>

DECLARE_GLOBAL_DATA_PTR;
extern omap3_sysinfo sysinfo;

static struct sdrc *sdrc_base = (struct sdrc *)OMAP34XX_SDRC_BASE;

/*
 * is_mem_sdr -
 *  - Return 1 if mem type in use is SDR
 */
u32 is_mem_sdr(void)
{
	if (readl(&sdrc_base->cs[CS0].mr) == SDRC_MR_0_SDR)
		return 1;
	return 0;
}

/*
 * make_cs1_contiguous -
 * - When we have CS1 populated we want to have it mapped after cs0 to allow
 *   command line mem=xyz use all memory with out discontinuous support
 *   compiled in.  We could do it in the ATAG, but there really is two banks...
 */
void make_cs1_contiguous(void)
{
	u32 size, a_add_low, a_add_high;

	size = get_sdr_cs_size(CS0);
	size >>= 25;	/* divide by 32 MiB to find size to offset CS1 */
	a_add_high = (size & 3) << 8;	/* set up low field */
	a_add_low = (size & 0x3C) >> 2;	/* set up high field */
	writel((a_add_high | a_add_low), &sdrc_base->cs_cfg);

}


/*
 * get_sdr_cs_size -
 *  - Get size of chip select 0/1
 */
u32 get_sdr_cs_size(u32 cs)
{
	u32 size;

	/* get ram size field */
	size = readl(&sdrc_base->cs[cs].mcfg) >> 8;
	size &= 0x3FF;		/* remove unwanted bits */
	size <<= 21;		/* multiply by 2 MiB to find size in MB */
	return size;
}

/*
 * get_sdr_cs_offset -
 *  - Get offset of cs from cs0 start
 */
u32 get_sdr_cs_offset(u32 cs)
{
	u32 offset;

	if (!cs)
		return 0;

	offset = readl(&sdrc_base->cs_cfg);
	offset = (offset & 15) << 27 | (offset & 0x300) << 17;

	return offset;
}

/*
 * write_sdrc_timings -
 *  - Takes CS and associated timings and initalize SDRAM
 *  - Test CS to make sure it's OK for use
 */
static void write_sdrc_timings(u32 cs, struct sdrc_actim *sdrc_actim_base,
			struct board_sdrc_timings *timings)
{
	/* Setup timings we got from the board. */
	writel(timings->mcfg, &sdrc_base->cs[cs].mcfg);
	writel(timings->ctrla, &sdrc_actim_base->ctrla);
	writel(timings->ctrlb, &sdrc_actim_base->ctrlb);
	writel(timings->rfr_ctrl, &sdrc_base->cs[cs].rfr_ctrl);
	writel(CMD_NOP, &sdrc_base->cs[cs].manual);
	writel(CMD_PRECHARGE, &sdrc_base->cs[cs].manual);
	writel(CMD_AUTOREFRESH, &sdrc_base->cs[cs].manual);
	writel(CMD_AUTOREFRESH, &sdrc_base->cs[cs].manual);
	writel(timings->mr, &sdrc_base->cs[cs].mr);

	/*
	 * Test ram in this bank
	 * Disable if bad or not present
	 */
	if (!mem_ok(cs))
		writel(0, &sdrc_base->cs[cs].mcfg);
}

/*
 * do_sdrc_init -
 *  - Code called once in C-Stack only context for CS0 and with early being
 *    true and a possible 2nd time depending on memory configuration from
 *    stack+global context.
 */
void do_sdrc_init(u32 cs, u32 early)
{
	struct sdrc_actim *sdrc_actim_base0, *sdrc_actim_base1;
	struct board_sdrc_timings timings;

	sdrc_actim_base0 = (struct sdrc_actim *)SDRC_ACTIM_CTRL0_BASE;
	sdrc_actim_base1 = (struct sdrc_actim *)SDRC_ACTIM_CTRL1_BASE;

	/* set some default timings */
	timings.sharing = SDRC_SHARING;

	/*
	 * When called in the early context this may be SPL and we will
	 * need to set all of the timings.  This ends up being board
	 * specific so we call a helper function to take care of this
	 * for us.  Otherwise, to be safe, we need to copy the settings
	 * from the first bank to the second.  We will setup CS0,
	 * then set cs_cfg to the appropriate value then try and
	 * setup CS1.
	 */
#ifdef CONFIG_SPL_BUILD
	/* set/modify board-specific timings */
	get_board_mem_timings(&timings);
#endif
	if (early) {
		/* reset sdrc controller */
		writel(SOFTRESET, &sdrc_base->sysconfig);
		wait_on_value(RESETDONE, RESETDONE, &sdrc_base->status,
				12000000);
		writel(0, &sdrc_base->sysconfig);

		/* setup sdrc to ball mux */
		writel(timings.sharing, &sdrc_base->sharing);

		/* Disable Power Down of CKE because of 1 CKE on combo part */
		writel(WAKEUPPROC | SRFRONRESET | PAGEPOLICY_HIGH,
				&sdrc_base->power);

		writel(ENADLL | DLLPHASE_90, &sdrc_base->dlla_ctrl);
		sdelay(0x20000);
#ifdef CONFIG_SPL_BUILD
		write_sdrc_timings(CS0, sdrc_actim_base0, &timings);
		make_cs1_contiguous();
		write_sdrc_timings(CS1, sdrc_actim_base1, &timings);
#endif

	}

	/*
	 * If we aren't using SPL we have been loaded by some
	 * other means which may not have correctly initialized
	 * both CS0 and CS1 (such as some older versions of x-loader)
	 * so we may be asked now to setup CS1.
	 */
	if (cs == CS1) {
		timings.mcfg = readl(&sdrc_base->cs[CS0].mcfg),
		timings.rfr_ctrl = readl(&sdrc_base->cs[CS0].rfr_ctrl);
		timings.ctrla = readl(&sdrc_actim_base0->ctrla);
		timings.ctrlb = readl(&sdrc_actim_base0->ctrlb);
		timings.mr = readl(&sdrc_base->cs[CS0].mr);
		write_sdrc_timings(cs, sdrc_actim_base1, &timings);
	}
}

/*
 * dram_init -
 *  - Sets uboots idea of sdram size
 */
int dram_init(void)
{
	unsigned int size0 = 0, size1 = 0;

	size0 = get_sdr_cs_size(CS0);
	/*
	 * We always need to have cs_cfg point at where the second
	 * bank would be, if present.  Failure to do so can lead to
	 * strange situations where memory isn't detected and
	 * configured correctly.  CS0 will already have been setup
	 * at this point.
	 */
	make_cs1_contiguous();
	do_sdrc_init(CS1, NOT_EARLY);
	size1 = get_sdr_cs_size(CS1);

	gd->ram_size = size0 + size1;

	return 0;
}

int dram_init_banksize(void)
{
	unsigned int size0 = 0, size1 = 0;

	size0 = get_sdr_cs_size(CS0);
	size1 = get_sdr_cs_size(CS1);

	gd->bd->bi_dram[0].start = PHYS_SDRAM_1;
	gd->bd->bi_dram[0].size = size0;
	gd->bd->bi_dram[1].start = PHYS_SDRAM_1 + get_sdr_cs_offset(CS1);
	gd->bd->bi_dram[1].size = size1;

	return 0;
}

/*
 * mem_init -
 *  - Init the sdrc chip,
 *  - Selects CS0 and CS1,
 */
void mem_init(void)
{
	/* only init up first bank here */
	do_sdrc_init(CS0, EARLY_INIT);
}
