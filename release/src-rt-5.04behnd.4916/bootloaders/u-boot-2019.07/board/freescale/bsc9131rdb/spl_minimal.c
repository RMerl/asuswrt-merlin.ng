// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2013 Freescale Semiconductor, Inc.
 */

#include <common.h>
#include <ns16550.h>
#include <asm/io.h>
#include <nand.h>
#include <linux/compiler.h>
#include <asm/fsl_law.h>
#include <fsl_ddr_sdram.h>
#include <asm/global_data.h>

DECLARE_GLOBAL_DATA_PTR;

/*
 * Fixed sdram init -- doesn't use serial presence detect.
 */
static void sdram_init(void)
{
	struct ccsr_ddr __iomem *ddr =
		(struct ccsr_ddr __iomem *)CONFIG_SYS_FSL_DDR_ADDR;

	__raw_writel(CONFIG_SYS_DDR_CS0_BNDS, &ddr->cs0_bnds);
	__raw_writel(CONFIG_SYS_DDR_CS0_CONFIG, &ddr->cs0_config);
#if CONFIG_CHIP_SELECTS_PER_CTRL > 1
	__raw_writel(CONFIG_SYS_DDR_CS1_BNDS, &ddr->cs1_bnds);
	__raw_writel(CONFIG_SYS_DDR_CS1_CONFIG, &ddr->cs1_config);
#endif
	__raw_writel(CONFIG_SYS_DDR_TIMING_3_800, &ddr->timing_cfg_3);
	__raw_writel(CONFIG_SYS_DDR_TIMING_0_800, &ddr->timing_cfg_0);
	__raw_writel(CONFIG_SYS_DDR_TIMING_1_800, &ddr->timing_cfg_1);
	__raw_writel(CONFIG_SYS_DDR_TIMING_2_800, &ddr->timing_cfg_2);

	__raw_writel(CONFIG_SYS_DDR_CONTROL_2, &ddr->sdram_cfg_2);
	__raw_writel(CONFIG_SYS_DDR_MODE_1_800, &ddr->sdram_mode);
	__raw_writel(CONFIG_SYS_DDR_MODE_2_800, &ddr->sdram_mode_2);

	__raw_writel(CONFIG_SYS_DDR_INTERVAL_800, &ddr->sdram_interval);
	__raw_writel(CONFIG_SYS_DDR_DATA_INIT, &ddr->sdram_data_init);
	__raw_writel(CONFIG_SYS_DDR_CLK_CTRL_800, &ddr->sdram_clk_cntl);

	__raw_writel(CONFIG_SYS_DDR_WRLVL_CONTROL_800, &ddr->ddr_wrlvl_cntl);
	__raw_writel(CONFIG_SYS_DDR_TIMING_4, &ddr->timing_cfg_4);
	__raw_writel(CONFIG_SYS_DDR_TIMING_5, &ddr->timing_cfg_5);
	__raw_writel(CONFIG_SYS_DDR_ZQ_CONTROL, &ddr->ddr_zq_cntl);

	/* Set, but do not enable the memory */
	__raw_writel(CONFIG_SYS_DDR_CONTROL & ~SDRAM_CFG_MEM_EN, &ddr->sdram_cfg);

	asm volatile("sync;isync");
	udelay(500);

	/* Let the controller go */
	out_be32(&ddr->sdram_cfg, in_be32(&ddr->sdram_cfg) | SDRAM_CFG_MEM_EN);

	set_next_law(CONFIG_SYS_NAND_DDR_LAW, LAW_SIZE_1G, LAW_TRGT_IF_DDR_1);
}

void board_init_f(ulong bootflag)
{
	u32 plat_ratio;
	ccsr_gur_t *gur = (void *)CONFIG_SYS_MPC85xx_GUTS_ADDR;

	/* initialize selected port with appropriate baud rate */
	plat_ratio = in_be32(&gur->porpllsr) & MPC85xx_PORPLLSR_PLAT_RATIO;
	plat_ratio >>= 1;
	gd->bus_clk = CONFIG_SYS_CLK_FREQ * plat_ratio;

	NS16550_init((NS16550_t)CONFIG_SYS_NS16550_COM1,
		     gd->bus_clk / 16 / CONFIG_BAUDRATE);

	puts("\nNAND boot... ");

	/* Initialize the DDR3 */
	sdram_init();

	/* copy code to RAM and jump to it - this should not return */
	/* NOTE - code has to be copied out of NAND buffer before
	 * other blocks can be read.
	 */
	relocate_code(CONFIG_SPL_RELOC_STACK, 0, CONFIG_SPL_RELOC_TEXT_BASE);
}

void board_init_r(gd_t *gd, ulong dest_addr)
{
	nand_boot();
}

void putc(char c)
{
	if (c == '\n')
		NS16550_putc((NS16550_t)CONFIG_SYS_NS16550_COM1, '\r');

	NS16550_putc((NS16550_t)CONFIG_SYS_NS16550_COM1, c);
}

void puts(const char *str)
{
	while (*str)
		putc(*str++);
}
