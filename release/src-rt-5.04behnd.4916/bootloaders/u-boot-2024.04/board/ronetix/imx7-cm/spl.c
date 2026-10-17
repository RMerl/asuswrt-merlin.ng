// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2021 Ronetix GmbH
 *
 * Author: Ilko Iliev <iliev@ronetix.at>
 */

#include <common.h>
#include <cpu_func.h>
#include <init.h>
#include <asm/arch/clock.h>
#include <asm/arch/imx-regs.h>
#include <asm/arch/crm_regs.h>
#include <asm/arch/mx7-pins.h>
#include <asm/arch/sys_proto.h>
#include <asm/arch-mx7/mx7-ddr.h>
#include <asm/mach-imx/iomux-v3.h>
#include <asm/gpio.h>
#include <asm/sections.h>
#include <fsl_esdhc_imx.h>
#include <spl.h>

static struct ddrc ddrc_regs_val = {
	.mstr		= 0x01040001,
	.rfshtmg	= 0x00400046,
	.init1		= 0x00690000,
	.init0		= 0x00020083,
	.init3		= 0x09300004,
	.init4		= 0x04080000,
	.init5		= 0x00100004,
	.rankctl	= 0x0000033F,
	.dramtmg0	= 0x09081109,
	.dramtmg1	= 0x0007020d,
	.dramtmg2	= 0x03040407,
	.dramtmg3	= 0x00002006,
	.dramtmg4	= 0x04020205,
	.dramtmg5	= 0x03030202,
	.dramtmg8	= 0x00000803,
	.zqctl0		= 0x00800020,
	.dfitmg0	= 0x02098204,
	.dfitmg1	= 0x00030303,
	.dfiupd0	= 0x80400003,
	.dfiupd1	= 0x00100020,
	.dfiupd2	= 0x80100004,
	.addrmap4	= 0x00000F0F,
	.odtcfg		= 0x06000604,
	.odtmap		= 0x00000001,
	.rfshtmg	= 0x00400046,
	.dramtmg0	= 0x09081109,
	.addrmap0	= 0x0000001f,
	.addrmap1	= 0x00080808,
	.addrmap4	= 0x00000f0f,
	.addrmap5	= 0x07070707,
	.addrmap6	= 0x0f0f0707,
};

static struct ddrc_mp ddrc_mp_val = {
	.pctrl_0	= 0x00000001,
};

static struct ddr_phy ddr_phy_regs_val = {
	.phy_con0	= 0x17420f40,
	.phy_con1	= 0x10210100,
	.phy_con4	= 0x00060807,
	.mdll_con0	= 0x1010007e,
	.drvds_con0	= 0x00000d6e,
	.cmd_sdll_con0	= 0x00000010,
	.offset_lp_con0	= 0x0000000f,
	.offset_rd_con0	= 0x0a0a0a0a,
	.offset_wr_con0	= 0x06060606,
};

static struct mx7_calibration calib_param = {
	.num_val	= 5,
	.values		= {
		0x0E407304,
		0x0E447304,
		0x0E447306,
		0x0E447304,
		0x0E447304,
	},
};

static void ddr_init(void)
{
	mx7_dram_cfg(&ddrc_regs_val, &ddrc_mp_val, &ddr_phy_regs_val, &calib_param);
}

#define UART_PAD_CTRL		(PAD_CTL_DSE_3P3V_49OHM | \
				PAD_CTL_PUS_PU100KOHM | PAD_CTL_HYS)

static iomux_v3_cfg_t const uart1_pads[] = {
	MX7D_PAD_UART1_TX_DATA__UART1_DCE_TX | MUX_PAD_CTRL(UART_PAD_CTRL),
	MX7D_PAD_UART1_RX_DATA__UART1_DCE_RX | MUX_PAD_CTRL(UART_PAD_CTRL),
};

void uart1_pads_set(void)
{
	imx_iomux_v3_setup_multiple_pads(uart1_pads, ARRAY_SIZE(uart1_pads));
}

void board_init_f(ulong dummy)
{
	arch_cpu_init();

	uart1_pads_set();

	timer_init();

	preloader_console_init();

	ddr_init();

	memset(__bss_start, 0, __bss_end - __bss_start);

	board_init_r(NULL, 0);
}

void reset_cpu(void)
{
}

#define USDHC_PAD_CTRL		(PAD_CTL_DSE_3P3V_32OHM | PAD_CTL_SRE_SLOW | \
				PAD_CTL_HYS | PAD_CTL_PUE | \
				PAD_CTL_PUS_PU47KOHM)

static iomux_v3_cfg_t const usdhc1_pads[] = {
	MX7D_PAD_SD1_CLK__SD1_CLK | MUX_PAD_CTRL(USDHC_PAD_CTRL),
	MX7D_PAD_SD1_CMD__SD1_CMD | MUX_PAD_CTRL(USDHC_PAD_CTRL),
	MX7D_PAD_SD1_DATA0__SD1_DATA0 | MUX_PAD_CTRL(USDHC_PAD_CTRL),
	MX7D_PAD_SD1_DATA1__SD1_DATA1 | MUX_PAD_CTRL(USDHC_PAD_CTRL),
	MX7D_PAD_SD1_DATA2__SD1_DATA2 | MUX_PAD_CTRL(USDHC_PAD_CTRL),
	MX7D_PAD_SD1_DATA3__SD1_DATA3 | MUX_PAD_CTRL(USDHC_PAD_CTRL),

	MX7D_PAD_SD1_CD_B__GPIO5_IO0 | MUX_PAD_CTRL(USDHC_PAD_CTRL),
};

void usdhc1_pads_set(void)
{
	imx_iomux_v3_setup_multiple_pads(usdhc1_pads, ARRAY_SIZE(usdhc1_pads));
}

static struct fsl_esdhc_cfg usdhc_cfg = {
	USDHC1_BASE_ADDR, 0, 4
};

int board_mmc_init(struct bd_info *bis)
{
	usdhc1_pads_set();
	usdhc_cfg.sdhc_clk = mxc_get_clock(MXC_ESDHC_CLK);
	return fsl_esdhc_initialize(bis, &usdhc_cfg);
}

int board_mmc_getcd(struct mmc *mmc)
{
	return 1;
}
