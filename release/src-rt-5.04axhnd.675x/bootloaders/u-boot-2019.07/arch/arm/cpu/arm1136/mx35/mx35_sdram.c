// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2012, Stefano Babic <sbabic@denx.de>
 */

#include <asm/io.h>
#include <linux/errno.h>
#include <asm/arch/imx-regs.h>
#include <linux/types.h>
#include <asm/arch/sys_proto.h>

#define ESDCTL_DDR2_EMR2	0x04000000
#define ESDCTL_DDR2_EMR3	0x06000000
#define ESDCTL_PRECHARGE	0x00000400
#define ESDCTL_DDR2_EN_DLL	0x02000400
#define ESDCTL_DDR2_RESET_DLL	0x00000333
#define ESDCTL_DDR2_MR		0x00000233
#define ESDCTL_DDR2_OCD_DEFAULT 0x02000780

enum {
	SMODE_NORMAL =	0,
	SMODE_PRECHARGE,
	SMODE_AUTO_REFRESH,
	SMODE_LOAD_REG,
	SMODE_MANUAL_REFRESH
};

#define set_mode(x, en, m)	(x | (en << 31) | (m << 28))

static inline void dram_wait(unsigned int count)
{
	volatile unsigned int wait = count;

	while (wait--)
		;

}

void mx3_setup_sdram_bank(u32 start_address, u32 ddr2_config,
	u32 row, u32 col, u32 dsize, u32 refresh)
{
	struct esdc_regs *esdc = (struct esdc_regs *)ESDCTL_BASE_ADDR;
	u32 *cfg_reg, *ctl_reg;
	u32 val;
	u32 ctlval;

	switch (start_address) {
	case CSD0_BASE_ADDR:
		cfg_reg = &esdc->esdcfg0;
		ctl_reg = &esdc->esdctl0;
		break;
	case CSD1_BASE_ADDR:
		cfg_reg = &esdc->esdcfg1;
		ctl_reg = &esdc->esdctl1;
		break;
	default:
		return;
	}

	/* The MX35 supports 11 up to 14 rows */
	if (row < 11 || row > 14 || col < 8 || col > 10)
		return;
	ctlval = (row - 11) << 24 | (col - 8) << 20 | (dsize << 16);

	/* Initialize MISC register for DDR2 */
	val = ESDC_MISC_RST | ESDC_MISC_MDDR_EN | ESDC_MISC_MDDR_DL_RST |
		ESDC_MISC_DDR_EN | ESDC_MISC_DDR2_EN;
	writel(val, &esdc->esdmisc);
	val &= ~(ESDC_MISC_RST | ESDC_MISC_MDDR_DL_RST);
	writel(val, &esdc->esdmisc);

	/*
	 * according to DDR2 specs, wait a while before
	 * the PRECHARGE_ALL command
	 */
	dram_wait(0x20000);

	/* Load DDR2 config and timing */
	writel(ddr2_config, cfg_reg);

	/* Precharge ALL */
	writel(set_mode(ctlval, 1, SMODE_PRECHARGE),
		ctl_reg);
	writel(0xda, start_address + ESDCTL_PRECHARGE);

	/* Load mode */
	writel(set_mode(ctlval, 1, SMODE_LOAD_REG),
		ctl_reg);
	writeb(0xda, start_address + ESDCTL_DDR2_EMR2); /* EMRS2 */
	writeb(0xda, start_address + ESDCTL_DDR2_EMR3); /* EMRS3 */
	writeb(0xda, start_address + ESDCTL_DDR2_EN_DLL); /* Enable DLL */
	writeb(0xda, start_address + ESDCTL_DDR2_RESET_DLL); /* Reset DLL */

	/* Precharge ALL */
	writel(set_mode(ctlval, 1, SMODE_PRECHARGE),
		ctl_reg);
	writel(0xda, start_address + ESDCTL_PRECHARGE);

	/* Set mode auto refresh : at least two refresh are required */
	writel(set_mode(ctlval, 1, SMODE_AUTO_REFRESH),
		ctl_reg);
	writel(0xda, start_address);
	writel(0xda, start_address);

	writel(set_mode(ctlval, 1, SMODE_LOAD_REG),
		ctl_reg);
	writeb(0xda, start_address + ESDCTL_DDR2_MR);
	writeb(0xda, start_address + ESDCTL_DDR2_OCD_DEFAULT);

	/* OCD mode exit */
	writeb(0xda, start_address + ESDCTL_DDR2_EN_DLL); /* Enable DLL */

	/* Set normal mode */
	writel(set_mode(ctlval, 1, SMODE_NORMAL) | refresh,
		ctl_reg);

	dram_wait(0x20000);

	/* Do not set delay lines, only for MDDR */
}
