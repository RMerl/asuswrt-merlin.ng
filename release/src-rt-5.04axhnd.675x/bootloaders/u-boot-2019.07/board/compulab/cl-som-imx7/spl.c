// SPDX-License-Identifier: GPL-2.0+
/*
 * SPL board functions for CompuLab CL-SOM-iMX7 module
 *
 * (C) Copyright 2017 CompuLab, Ltd. http://www.compulab.com
 *
 * Author: Uri Mashiach <uri.mashiach@compulab.co.il>
 */

#include <common.h>
#include <spl.h>
#include <fsl_esdhc.h>
#include <asm/mach-imx/iomux-v3.h>
#include <asm/arch-mx7/mx7-pins.h>
#include <asm/arch-mx7/clock.h>
#include <asm/arch-mx7/mx7-ddr.h>
#include "common.h"

#ifdef CONFIG_FSL_ESDHC

static struct fsl_esdhc_cfg cl_som_imx7_spl_usdhc_cfg = {
	USDHC1_BASE_ADDR, 0, 4};

int board_mmc_init(bd_t *bis)
{
	cl_som_imx7_usdhc1_pads_set();
	cl_som_imx7_spl_usdhc_cfg.sdhc_clk = mxc_get_clock(MXC_ESDHC_CLK);
	return fsl_esdhc_initialize(bis, &cl_som_imx7_spl_usdhc_cfg);
}
#endif /* CONFIG_FSL_ESDHC */

static iomux_v3_cfg_t const led_pads[] = {
	MX7D_PAD_SAI1_TX_SYNC__GPIO6_IO14 | MUX_PAD_CTRL(PAD_CTL_PUS_PU5KOHM |
		PAD_CTL_PUE | PAD_CTL_SRE_SLOW)
};

static struct ddrc cl_som_imx7_spl_ddrc_regs_val = {
	.init1		= 0x00690000,
	.init0		= 0x00020083,
	.init3		= 0x09300004,
	.init4		= 0x04080000,
	.init5		= 0x00100004,
	.rankctl	= 0x0000033F,
	.dramtmg1	= 0x0007020E,
	.dramtmg2	= 0x03040407,
	.dramtmg3	= 0x00002006,
	.dramtmg4	= 0x04020305,
	.dramtmg5	= 0x03030202,
	.dramtmg8	= 0x00000803,
	.zqctl0		= 0x00810021,
	.dfitmg0	= 0x02098204,
	.dfitmg1	= 0x00030303,
	.dfiupd0	= 0x80400003,
	.dfiupd1	= 0x00100020,
	.dfiupd2	= 0x80100004,
	.addrmap4	= 0x00000F0F,
	.odtcfg		= 0x06000604,
	.odtmap		= 0x00000001,
};

static struct ddrc_mp cl_som_imx7_spl_ddrc_mp_val = {
	.pctrl_0	= 0x00000001,
};

static struct ddr_phy cl_som_imx7_spl_ddr_phy_regs_val = {
	.phy_con0	= 0x17420F40,
	.phy_con1	= 0x10210100,
	.phy_con4	= 0x00060807,
	.mdll_con0	= 0x1010007E,
	.drvds_con0	= 0x00000D6E,
	.cmd_sdll_con0	= 0x00000010,
	.offset_lp_con0	= 0x0000000F,
};

struct mx7_calibration cl_som_imx7_spl_calib_param = {
	.num_val	= 5,
	.values		= {
		0x0E407304,
		0x0E447304,
		0x0E447306,
		0x0E447304,
		0x0E407304,
	},
};

static void cl_som_imx7_spl_dram_cfg_size(u32 ram_size)
{
	switch (ram_size) {
	case SZ_256M:
		cl_som_imx7_spl_ddrc_regs_val.mstr		= 0x01041001;
		cl_som_imx7_spl_ddrc_regs_val.rfshtmg		= 0x00400046;
		cl_som_imx7_spl_ddrc_regs_val.dramtmg0		= 0x090E1109;
		cl_som_imx7_spl_ddrc_regs_val.addrmap0		= 0x00000014;
		cl_som_imx7_spl_ddrc_regs_val.addrmap1		= 0x00151515;
		cl_som_imx7_spl_ddrc_regs_val.addrmap5		= 0x03030303;
		cl_som_imx7_spl_ddrc_regs_val.addrmap6		= 0x0F0F0303;
		cl_som_imx7_spl_ddr_phy_regs_val.offset_rd_con0	= 0x0C0C0C0C;
		cl_som_imx7_spl_ddr_phy_regs_val.offset_wr_con0	= 0x04040404;
		break;
	case SZ_512M:
		cl_som_imx7_spl_ddrc_regs_val.mstr		= 0x01040001;
		cl_som_imx7_spl_ddrc_regs_val.rfshtmg		= 0x00400046;
		cl_som_imx7_spl_ddrc_regs_val.dramtmg0		= 0x090E1109;
		cl_som_imx7_spl_ddrc_regs_val.addrmap0		= 0x00000015;
		cl_som_imx7_spl_ddrc_regs_val.addrmap1		= 0x00161616;
		cl_som_imx7_spl_ddrc_regs_val.addrmap5		= 0x04040404;
		cl_som_imx7_spl_ddrc_regs_val.addrmap6		= 0x0F0F0404;
		cl_som_imx7_spl_ddr_phy_regs_val.offset_rd_con0	= 0x0C0C0C0C;
		cl_som_imx7_spl_ddr_phy_regs_val.offset_wr_con0	= 0x04040404;
		break;
	case SZ_1G:
		cl_som_imx7_spl_ddrc_regs_val.mstr		= 0x01040001;
		cl_som_imx7_spl_ddrc_regs_val.rfshtmg		= 0x00400046;
		cl_som_imx7_spl_ddrc_regs_val.dramtmg0		= 0x090E1109;
		cl_som_imx7_spl_ddrc_regs_val.addrmap0		= 0x00000016;
		cl_som_imx7_spl_ddrc_regs_val.addrmap1		= 0x00171717;
		cl_som_imx7_spl_ddrc_regs_val.addrmap5		= 0x04040404;
		cl_som_imx7_spl_ddrc_regs_val.addrmap6		= 0x0F040404;
		cl_som_imx7_spl_ddr_phy_regs_val.offset_rd_con0	= 0x0A0A0A0A;
		cl_som_imx7_spl_ddr_phy_regs_val.offset_wr_con0	= 0x02020202;
		break;
	case SZ_2G:
		cl_som_imx7_spl_ddrc_regs_val.mstr		= 0x01040001;
		cl_som_imx7_spl_ddrc_regs_val.rfshtmg		= 0x0040005E;
		cl_som_imx7_spl_ddrc_regs_val.dramtmg0		= 0x090E110A;
		cl_som_imx7_spl_ddrc_regs_val.addrmap0		= 0x00000018;
		cl_som_imx7_spl_ddrc_regs_val.addrmap1		= 0x00181818;
		cl_som_imx7_spl_ddrc_regs_val.addrmap5		= 0x04040404;
		cl_som_imx7_spl_ddrc_regs_val.addrmap6		= 0x04040404;
		cl_som_imx7_spl_ddr_phy_regs_val.offset_rd_con0	= 0x0A0A0A0A;
		cl_som_imx7_spl_ddr_phy_regs_val.offset_wr_con0	= 0x04040404;
		break;
	}

	mx7_dram_cfg(&cl_som_imx7_spl_ddrc_regs_val,
		     &cl_som_imx7_spl_ddrc_mp_val,
		     &cl_som_imx7_spl_ddr_phy_regs_val,
		     &cl_som_imx7_spl_calib_param);
}

static void cl_som_imx7_spl_dram_cfg(void)
{
	ulong ram_size_test, ram_size = 0;

	for (ram_size = SZ_2G; ram_size >= SZ_256M; ram_size >>= 1) {
		cl_som_imx7_spl_dram_cfg_size(ram_size);
		ram_size_test = get_ram_size((long int *)PHYS_SDRAM, ram_size);
		if (ram_size_test == ram_size)
			break;
	}

	if (ram_size < SZ_256M) {
		puts("!!!ERROR!!! DRAM detection failed!!!\n");
		hang();
	}
}

#ifdef CONFIG_SPL_SPI_SUPPORT

static void cl_som_imx7_spl_spi_init(void)
{
	cl_som_imx7_espi1_pads_set();
}
#else /* !CONFIG_SPL_SPI_SUPPORT */
static void cl_som_imx7_spl_spi_init(void) {}
#endif /* CONFIG_SPL_SPI_SUPPORT */

void board_init_f(ulong dummy)
{
	imx_iomux_v3_setup_multiple_pads(led_pads, 1);
	/* setup AIPS and disable watchdog */
	arch_cpu_init();
	/* setup GP timer */
	timer_init();
	cl_som_imx7_spl_spi_init();
	cl_som_imx7_uart1_pads_set();
	/* UART clocks enabled and gd valid - init serial console */
	preloader_console_init();
	/* DRAM detection  */
	cl_som_imx7_spl_dram_cfg();
	/* Clear the BSS. */
	memset(__bss_start, 0, __bss_end - __bss_start);
	/* load/boot image from boot device */
	board_init_r(NULL, 0);
}

void spl_board_init(void)
{
	u32 boot_device = spl_boot_device();

	if (boot_device == BOOT_DEVICE_SPI)
		puts("Booting from SPI flash\n");
	else if (boot_device == BOOT_DEVICE_MMC1)
		puts("Booting from SD card\n");
	else
		puts("Unknown boot device\n");
}

void board_boot_order(u32 *spl_boot_list)
{
	spl_boot_list[0] = spl_boot_device();
	switch (spl_boot_list[0]) {
	case BOOT_DEVICE_SPI:
		spl_boot_list[1] = BOOT_DEVICE_MMC1;
		break;
	case BOOT_DEVICE_MMC1:
		spl_boot_list[1] = BOOT_DEVICE_SPI;
		break;
	}
}
