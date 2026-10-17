// SPDX-License-Identifier: GPL-2.0+

#include <common.h>
#include <cpu_func.h>
#include <hang.h>
#include <init.h>
#include <asm/arch/clock.h>
#include <asm/arch/iomux.h>
#include <asm/arch/imx-regs.h>
#include <asm/arch/crm_regs.h>
#include <asm/arch/mx6ull_pins.h>
#include <asm/arch/mx6-pins.h>
#include <asm/arch/sys_proto.h>
#include <asm/gpio.h>
#include <asm/mach-imx/iomux-v3.h>
#include <asm/mach-imx/boot_mode.h>
#include <linux/libfdt.h>
#include <spl.h>
#include <asm/arch/mx6-ddr.h>

#define UART_PAD_CTRL (PAD_CTL_PKE | PAD_CTL_PUE | \
		       PAD_CTL_PUS_100K_UP | PAD_CTL_SPEED_MED | \
		       PAD_CTL_DSE_40ohm | PAD_CTL_SRE_FAST | PAD_CTL_HYS)

static const iomux_v3_cfg_t uart4_pads[] = {
	MX6_PAD_UART4_TX_DATA__UART4_DCE_TX | MUX_PAD_CTRL(UART_PAD_CTRL),
	MX6_PAD_UART4_RX_DATA__UART4_DCE_RX | MUX_PAD_CTRL(UART_PAD_CTRL),
};

static void setup_iomux_uart(void)
{
	imx_iomux_v3_setup_multiple_pads(uart4_pads, ARRAY_SIZE(uart4_pads));
}

static struct mx6ul_iomux_grp_regs mx6_grp_ioregs = {
	.grp_addds		= 0x00000028,
	.grp_ddrmode_ctl	= 0x00020000,
	.grp_b0ds		= 0x00000028,
	.grp_ctlds		= 0x00000028,
	.grp_b1ds		= 0x00000028,
	.grp_ddrpke		= 0x00000000,
	.grp_ddrmode		= 0x00020000,
	.grp_ddr_type		= 0x000c0000,
};

static struct mx6ul_iomux_ddr_regs mx6_ddr_ioregs = {
	.dram_dqm0		= 0x00000028,
	.dram_dqm1		= 0x00000028,
	.dram_ras		= 0x00000028,
	.dram_cas		= 0x00000028,
	.dram_odt0		= 0x00000028,
	.dram_odt1		= 0x00000028,
	.dram_sdba2		= 0x00000000,
	.dram_sdclk_0		= 0x00000028,
	.dram_sdqs0		= 0x00000028,
	.dram_sdqs1		= 0x00000028,
	.dram_reset		= 0x000c0028,
};

static struct mx6_mmdc_calibration mx6_mmcd_calib = {
	.p0_mpwldectrl0		= 0x00000000,
	.p0_mpwldectrl1		= 0x00100010,
	.p0_mpdgctrl0		= 0x414c014c,
	.p0_mpdgctrl1		= 0x00000000,
	.p0_mprddlctl		= 0x40403a42,
	.p0_mpwrdlctl		= 0x4040342e,
};

static struct mx6_ddr_sysinfo ddr_sysinfo = {
	.dsize			= 0,
	.cs1_mirror		= 0,
	.cs_density		= 32,
	.ncs			= 1,
	.bi_on			= 1,
	.rtt_nom		= 1,
	.rtt_wr			= 0,
	.ralat			= 5,
	.walat			= 1,
	.mif3_mode		= 3,
	.rst_to_cke		= 0x23,	/* 33 cycles (JEDEC value for DDR3) - total of 500 us */
	.sde_to_rst		= 0x10,	/* 14 cycles (JEDEC value for DDR3) - total of 200 us */
	.refsel			= 1,
	.refr			= 3,
};

static struct mx6_ddr3_cfg mem_ddr = {
	.mem_speed		= 1333,
	.density		= 2,
	.width			= 16,
	.banks			= 8,
	.rowaddr		= 13,
	.coladdr		= 10,
	.pagesz			= 2,
	.trcd			= 1350,
	.trcmin			= 4950,
	.trasmin		= 3600,
};

static void ccgr_init(void)
{
	struct mxc_ccm_reg *ccm = (struct mxc_ccm_reg *)CCM_BASE_ADDR;

	writel(0xFFFFFFFF, &ccm->CCGR0);
	writel(0xFFFFFFFF, &ccm->CCGR1);
	writel(0xFFFFFFFF, &ccm->CCGR2);
	writel(0xFFFFFFFF, &ccm->CCGR3);
	writel(0xFFFFFFFF, &ccm->CCGR4);
	writel(0xFFFFFFFF, &ccm->CCGR5);
	writel(0xFFFFFFFF, &ccm->CCGR6);
}

static void imx6ul_spl_dram_cfg(void)
{
	mx6ul_dram_iocfg(mem_ddr.width, &mx6_ddr_ioregs, &mx6_grp_ioregs);
	mx6_dram_cfg(&ddr_sysinfo, &mx6_mmcd_calib, &mem_ddr);
}

void board_init_f(ulong dummy)
{
	ccgr_init();
	arch_cpu_init();
	timer_init();
	setup_iomux_uart();
	preloader_console_init();
	imx6ul_spl_dram_cfg();
}

void reset_cpu(void)
{
}
