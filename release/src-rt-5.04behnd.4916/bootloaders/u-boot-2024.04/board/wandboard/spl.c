// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2014 Wandboard
 * Author: Tungyi Lin <tungyilin1127@gmail.com>
 *         Richard Hu <hakahu@gmail.com>
 */

#include <common.h>
#include <init.h>
#include <asm/arch/clock.h>
#include <asm/arch/imx-regs.h>
#include <asm/arch/iomux.h>
#include <asm/arch/mx6-pins.h>
#include <linux/delay.h>
#include <linux/errno.h>
#include <asm/gpio.h>
#include <asm/mach-imx/iomux-v3.h>
#include <asm/mach-imx/video.h>
#include <mmc.h>
#include <fsl_esdhc_imx.h>
#include <asm/arch/crm_regs.h>
#include <asm/io.h>
#include <asm/arch/sys_proto.h>
#include <spl.h>

#include <asm/arch/mx6-ddr.h>
/*
 * Driving strength:
 *   0x30 == 40 Ohm
 *   0x28 == 48 Ohm
 */

#define IMX6DQ_DRIVE_STRENGTH		0x30
#define IMX6SDL_DRIVE_STRENGTH		0x28
#define IMX6QP_DRIVE_STRENGTH		0x28

/* configure MX6Q/DUAL mmdc DDR io registers */
static struct mx6dq_iomux_ddr_regs mx6dq_ddr_ioregs = {
	.dram_sdclk_0 = IMX6DQ_DRIVE_STRENGTH,
	.dram_sdclk_1 = IMX6DQ_DRIVE_STRENGTH,
	.dram_cas = IMX6DQ_DRIVE_STRENGTH,
	.dram_ras = IMX6DQ_DRIVE_STRENGTH,
	.dram_reset = IMX6DQ_DRIVE_STRENGTH,
	.dram_sdcke0 = IMX6DQ_DRIVE_STRENGTH,
	.dram_sdcke1 = IMX6DQ_DRIVE_STRENGTH,
	.dram_sdba2 = 0x00000000,
	.dram_sdodt0 = IMX6DQ_DRIVE_STRENGTH,
	.dram_sdodt1 = IMX6DQ_DRIVE_STRENGTH,
	.dram_sdqs0 = IMX6DQ_DRIVE_STRENGTH,
	.dram_sdqs1 = IMX6DQ_DRIVE_STRENGTH,
	.dram_sdqs2 = IMX6DQ_DRIVE_STRENGTH,
	.dram_sdqs3 = IMX6DQ_DRIVE_STRENGTH,
	.dram_sdqs4 = IMX6DQ_DRIVE_STRENGTH,
	.dram_sdqs5 = IMX6DQ_DRIVE_STRENGTH,
	.dram_sdqs6 = IMX6DQ_DRIVE_STRENGTH,
	.dram_sdqs7 = IMX6DQ_DRIVE_STRENGTH,
	.dram_dqm0 = IMX6DQ_DRIVE_STRENGTH,
	.dram_dqm1 = IMX6DQ_DRIVE_STRENGTH,
	.dram_dqm2 = IMX6DQ_DRIVE_STRENGTH,
	.dram_dqm3 = IMX6DQ_DRIVE_STRENGTH,
	.dram_dqm4 = IMX6DQ_DRIVE_STRENGTH,
	.dram_dqm5 = IMX6DQ_DRIVE_STRENGTH,
	.dram_dqm6 = IMX6DQ_DRIVE_STRENGTH,
	.dram_dqm7 = IMX6DQ_DRIVE_STRENGTH,
};

/* configure MX6QP mmdc DDR io registers */
static struct mx6dq_iomux_ddr_regs mx6qp_ddr_ioregs = {
	.dram_sdclk_0 = IMX6QP_DRIVE_STRENGTH,
	.dram_sdclk_1 = IMX6QP_DRIVE_STRENGTH,
	.dram_cas = IMX6QP_DRIVE_STRENGTH,
	.dram_ras = IMX6QP_DRIVE_STRENGTH,
	.dram_reset = IMX6QP_DRIVE_STRENGTH,
	.dram_sdcke0 = IMX6QP_DRIVE_STRENGTH,
	.dram_sdcke1 = IMX6QP_DRIVE_STRENGTH,
	.dram_sdba2 = 0x00000000,
	.dram_sdodt0 = IMX6QP_DRIVE_STRENGTH,
	.dram_sdodt1 = IMX6QP_DRIVE_STRENGTH,
	.dram_sdqs0 = IMX6QP_DRIVE_STRENGTH,
	.dram_sdqs1 = IMX6QP_DRIVE_STRENGTH,
	.dram_sdqs2 = IMX6QP_DRIVE_STRENGTH,
	.dram_sdqs3 = IMX6QP_DRIVE_STRENGTH,
	.dram_sdqs4 = IMX6QP_DRIVE_STRENGTH,
	.dram_sdqs5 = IMX6QP_DRIVE_STRENGTH,
	.dram_sdqs6 = IMX6QP_DRIVE_STRENGTH,
	.dram_sdqs7 = IMX6QP_DRIVE_STRENGTH,
	.dram_dqm0 = IMX6QP_DRIVE_STRENGTH,
	.dram_dqm1 = IMX6QP_DRIVE_STRENGTH,
	.dram_dqm2 = IMX6QP_DRIVE_STRENGTH,
	.dram_dqm3 = IMX6QP_DRIVE_STRENGTH,
	.dram_dqm4 = IMX6QP_DRIVE_STRENGTH,
	.dram_dqm5 = IMX6QP_DRIVE_STRENGTH,
	.dram_dqm6 = IMX6QP_DRIVE_STRENGTH,
	.dram_dqm7 = IMX6QP_DRIVE_STRENGTH,
};

/* configure MX6Q/DUAL mmdc GRP io registers */
static struct mx6dq_iomux_grp_regs mx6dq_grp_ioregs = {
	.grp_ddr_type = 0x000c0000,
	.grp_ddrmode_ctl = 0x00020000,
	.grp_ddrpke = 0x00000000,
	.grp_addds = IMX6DQ_DRIVE_STRENGTH,
	.grp_ctlds = IMX6DQ_DRIVE_STRENGTH,
	.grp_ddrmode = 0x00020000,
	.grp_b0ds = IMX6DQ_DRIVE_STRENGTH,
	.grp_b1ds = IMX6DQ_DRIVE_STRENGTH,
	.grp_b2ds = IMX6DQ_DRIVE_STRENGTH,
	.grp_b3ds = IMX6DQ_DRIVE_STRENGTH,
	.grp_b4ds = IMX6DQ_DRIVE_STRENGTH,
	.grp_b5ds = IMX6DQ_DRIVE_STRENGTH,
	.grp_b6ds = IMX6DQ_DRIVE_STRENGTH,
	.grp_b7ds = IMX6DQ_DRIVE_STRENGTH,
};

/* configure MX6QP mmdc GRP io registers */
static struct mx6dq_iomux_grp_regs mx6qp_grp_ioregs = {
	.grp_ddr_type = 0x000c0000,
	.grp_ddrmode_ctl = 0x00020000,
	.grp_ddrpke = 0x00000000,
	.grp_addds = IMX6QP_DRIVE_STRENGTH,
	.grp_ctlds = IMX6QP_DRIVE_STRENGTH,
	.grp_ddrmode = 0x00020000,
	.grp_b0ds = IMX6QP_DRIVE_STRENGTH,
	.grp_b1ds = IMX6QP_DRIVE_STRENGTH,
	.grp_b2ds = IMX6QP_DRIVE_STRENGTH,
	.grp_b3ds = IMX6QP_DRIVE_STRENGTH,
	.grp_b4ds = IMX6QP_DRIVE_STRENGTH,
	.grp_b5ds = IMX6QP_DRIVE_STRENGTH,
	.grp_b6ds = IMX6QP_DRIVE_STRENGTH,
	.grp_b7ds = IMX6QP_DRIVE_STRENGTH,
};

/* configure MX6SOLO/DUALLITE mmdc DDR io registers */
struct mx6sdl_iomux_ddr_regs mx6sdl_ddr_ioregs = {
	.dram_sdclk_0 = IMX6SDL_DRIVE_STRENGTH,
	.dram_sdclk_1 = IMX6SDL_DRIVE_STRENGTH,
	.dram_cas = IMX6SDL_DRIVE_STRENGTH,
	.dram_ras = IMX6SDL_DRIVE_STRENGTH,
	.dram_reset = IMX6SDL_DRIVE_STRENGTH,
	.dram_sdcke0 = IMX6SDL_DRIVE_STRENGTH,
	.dram_sdcke1 = IMX6SDL_DRIVE_STRENGTH,
	.dram_sdba2 = 0x00000000,
	.dram_sdodt0 = IMX6SDL_DRIVE_STRENGTH,
	.dram_sdodt1 = IMX6SDL_DRIVE_STRENGTH,
	.dram_sdqs0 = IMX6SDL_DRIVE_STRENGTH,
	.dram_sdqs1 = IMX6SDL_DRIVE_STRENGTH,
	.dram_sdqs2 = IMX6SDL_DRIVE_STRENGTH,
	.dram_sdqs3 = IMX6SDL_DRIVE_STRENGTH,
	.dram_sdqs4 = IMX6SDL_DRIVE_STRENGTH,
	.dram_sdqs5 = IMX6SDL_DRIVE_STRENGTH,
	.dram_sdqs6 = IMX6SDL_DRIVE_STRENGTH,
	.dram_sdqs7 = IMX6SDL_DRIVE_STRENGTH,
	.dram_dqm0 = IMX6SDL_DRIVE_STRENGTH,
	.dram_dqm1 = IMX6SDL_DRIVE_STRENGTH,
	.dram_dqm2 = IMX6SDL_DRIVE_STRENGTH,
	.dram_dqm3 = IMX6SDL_DRIVE_STRENGTH,
	.dram_dqm4 = IMX6SDL_DRIVE_STRENGTH,
	.dram_dqm5 = IMX6SDL_DRIVE_STRENGTH,
	.dram_dqm6 = IMX6SDL_DRIVE_STRENGTH,
	.dram_dqm7 = IMX6SDL_DRIVE_STRENGTH,
};

/* configure MX6SOLO/DUALLITE mmdc GRP io registers */
struct mx6sdl_iomux_grp_regs mx6sdl_grp_ioregs = {
	.grp_ddr_type = 0x000c0000,
	.grp_ddrmode_ctl = 0x00020000,
	.grp_ddrpke = 0x00000000,
	.grp_addds = IMX6SDL_DRIVE_STRENGTH,
	.grp_ctlds = IMX6SDL_DRIVE_STRENGTH,
	.grp_ddrmode = 0x00020000,
	.grp_b0ds = IMX6SDL_DRIVE_STRENGTH,
	.grp_b1ds = IMX6SDL_DRIVE_STRENGTH,
	.grp_b2ds = IMX6SDL_DRIVE_STRENGTH,
	.grp_b3ds = IMX6SDL_DRIVE_STRENGTH,
	.grp_b4ds = IMX6SDL_DRIVE_STRENGTH,
	.grp_b5ds = IMX6SDL_DRIVE_STRENGTH,
	.grp_b6ds = IMX6SDL_DRIVE_STRENGTH,
	.grp_b7ds = IMX6SDL_DRIVE_STRENGTH,
};

/* H5T04G63AFR-PB */
static struct mx6_ddr3_cfg h5t04g63afr = {
	.mem_speed = 1600,
	.density = 4,
	.width = 16,
	.banks = 8,
	.rowaddr = 15,
	.coladdr = 10,
	.pagesz = 2,
	.trcd = 1375,
	.trcmin = 4875,
	.trasmin = 3500,
};

/* H5TQ2G63DFR-H9 */
static struct mx6_ddr3_cfg h5tq2g63dfr = {
	.mem_speed = 1333,
	.density = 2,
	.width = 16,
	.banks = 8,
	.rowaddr = 14,
	.coladdr = 10,
	.pagesz = 2,
	.trcd = 1350,
	.trcmin = 4950,
	.trasmin = 3600,
};

static struct mx6_mmdc_calibration mx6q_2g_mmdc_calib = {
	.p0_mpwldectrl0 = 0x001f001f,
	.p0_mpwldectrl1 = 0x001f001f,
	.p1_mpwldectrl0 = 0x001f001f,
	.p1_mpwldectrl1 = 0x001f001f,
	.p0_mpdgctrl0 = 0x4301030d,
	.p0_mpdgctrl1 = 0x03020277,
	.p1_mpdgctrl0 = 0x4300030a,
	.p1_mpdgctrl1 = 0x02780248,
	.p0_mprddlctl = 0x4536393b,
	.p1_mprddlctl = 0x36353441,
	.p0_mpwrdlctl = 0x41414743,
	.p1_mpwrdlctl = 0x462f453f,
};

/* DDR 64bit 2GB */
static struct mx6_ddr_sysinfo mem_q = {
	.dsize		= 2,
	.cs1_mirror	= 0,
	/* config for full 4GB range so that get_mem_size() works */
	.cs_density	= 32,
	.ncs		= 1,
	.bi_on		= 1,
	.rtt_nom	= 1,
	.rtt_wr		= 0,
	.ralat		= 5,
	.walat		= 0,
	.mif3_mode	= 3,
	.rst_to_cke	= 0x23,
	.sde_to_rst	= 0x10,
	.refsel = 1,	/* Refresh cycles at 32KHz */
	.refr = 3,	/* 4 refresh commands per refresh cycle */
};

static struct mx6_mmdc_calibration mx6dl_1g_mmdc_calib = {
	.p0_mpwldectrl0 = 0x001f001f,
	.p0_mpwldectrl1 = 0x001f001f,
	.p1_mpwldectrl0 = 0x001f001f,
	.p1_mpwldectrl1 = 0x001f001f,
	.p0_mpdgctrl0 = 0x420e020e,
	.p0_mpdgctrl1 = 0x02000200,
	.p1_mpdgctrl0 = 0x42020202,
	.p1_mpdgctrl1 = 0x01720172,
	.p0_mprddlctl = 0x494c4f4c,
	.p1_mprddlctl = 0x4a4c4c49,
	.p0_mpwrdlctl = 0x3f3f3133,
	.p1_mpwrdlctl = 0x39373f2e,
};

static struct mx6_mmdc_calibration mx6s_512m_mmdc_calib = {
	.p0_mpwldectrl0 = 0x0040003c,
	.p0_mpwldectrl1 = 0x0032003e,
	.p0_mpdgctrl0 = 0x42350231,
	.p0_mpdgctrl1 = 0x021a0218,
	.p0_mprddlctl = 0x4b4b4e49,
	.p0_mpwrdlctl = 0x3f3f3035,
};

/* DDR 64bit 1GB */
static struct mx6_ddr_sysinfo mem_dl = {
	.dsize		= 2,
	.cs1_mirror	= 0,
	/* config for full 4GB range so that get_mem_size() works */
	.cs_density	= 32,
	.ncs		= 1,
	.bi_on		= 1,
	.rtt_nom	= 1,
	.rtt_wr		= 0,
	.ralat		= 5,
	.walat		= 0,
	.mif3_mode	= 3,
	.rst_to_cke	= 0x23,
	.sde_to_rst	= 0x10,
	.refsel = 1,	/* Refresh cycles at 32KHz */
	.refr = 3,	/* 4 refresh commands per refresh cycle */
};

/* DDR 32bit 512MB */
static struct mx6_ddr_sysinfo mem_s = {
	.dsize		= 1,
	.cs1_mirror	= 0,
	/* config for full 4GB range so that get_mem_size() works */
	.cs_density	= 32,
	.ncs		= 1,
	.bi_on		= 1,
	.rtt_nom	= 1,
	.rtt_wr		= 0,
	.ralat		= 5,
	.walat		= 0,
	.mif3_mode	= 3,
	.rst_to_cke	= 0x23,
	.sde_to_rst	= 0x10,
	.refsel = 1,	/* Refresh cycles at 32KHz */
	.refr = 3,	/* 4 refresh commands per refresh cycle */
};

static void ccgr_init(void)
{
	struct mxc_ccm_reg *ccm = (struct mxc_ccm_reg *)CCM_BASE_ADDR;

	writel(0x00C03F3F, &ccm->CCGR0);
	writel(0x0030FC03, &ccm->CCGR1);
	writel(0x0FFFC000, &ccm->CCGR2);
	writel(0x3FF03000, &ccm->CCGR3);
	writel(0x00FFF300, &ccm->CCGR4);
	writel(0x0F0000C3, &ccm->CCGR5);
	writel(0x000003FF, &ccm->CCGR6);
}

static void spl_dram_init_imx6qp_lpddr3(void)
{
	/* MMDC0_MDSCR set the Configuration request bit during MMDC set up */
	writel(0x00008000, MMDC_P0_BASE_ADDR + 0x01c);
	/* Calibrations - ZQ */
	writel(0xa1390003, MMDC_P0_BASE_ADDR + 0x800);
	/* write leveling */
	writel(0x00060004, MMDC_P0_BASE_ADDR + 0x80c);
	writel(0x000B0004, MMDC_P0_BASE_ADDR + 0x810);
	writel(0x00000004, MMDC_P1_BASE_ADDR + 0x80c);
	writel(0x00000000, MMDC_P1_BASE_ADDR + 0x810);
	/*
	 * DQS gating, read delay, write delay calibration values
	 * based on calibration compare of 0x00ffff00
	 */
	writel(0x03040314, MMDC_P0_BASE_ADDR + 0x83c);
	writel(0x03080300, MMDC_P0_BASE_ADDR + 0x840);
	writel(0x03000310, MMDC_P1_BASE_ADDR + 0x83c);
	writel(0x0268023C, MMDC_P1_BASE_ADDR + 0x840);
	writel(0x4034363A, MMDC_P0_BASE_ADDR + 0x848);
	writel(0x36302C3C, MMDC_P1_BASE_ADDR + 0x848);
	writel(0x3E3E4046, MMDC_P0_BASE_ADDR + 0x850);
	writel(0x483A4844, MMDC_P1_BASE_ADDR + 0x850);
	writel(0x33333333, MMDC_P0_BASE_ADDR + 0x81c);
	writel(0x33333333, MMDC_P0_BASE_ADDR + 0x820);
	writel(0x33333333, MMDC_P0_BASE_ADDR + 0x824);
	writel(0x33333333, MMDC_P0_BASE_ADDR + 0x828);
	writel(0x33333333, MMDC_P1_BASE_ADDR + 0x81c);
	writel(0x33333333, MMDC_P1_BASE_ADDR + 0x820);
	writel(0x33333333, MMDC_P1_BASE_ADDR + 0x824);
	writel(0x33333333, MMDC_P1_BASE_ADDR + 0x828);
	writel(0x24912489, MMDC_P0_BASE_ADDR + 0x8c0);
	writel(0x24914452, MMDC_P1_BASE_ADDR + 0x8c0);
	writel(0x00000800, MMDC_P0_BASE_ADDR + 0x8b8);
	writel(0x00000800, MMDC_P1_BASE_ADDR + 0x8b8);
	/* MMDC init: in DDR3, 64-bit mode, only MMDC0 is initiated */
	writel(0x00020036, MMDC_P0_BASE_ADDR + 0x004);
	writel(0x09444040, MMDC_P0_BASE_ADDR + 0x008);
	writel(0x898E79A4, MMDC_P0_BASE_ADDR + 0x00c);
	writel(0xDB538F64, MMDC_P0_BASE_ADDR + 0x010);
	writel(0x01FF00DD, MMDC_P0_BASE_ADDR + 0x014);
	writel(0x00011740, MMDC_P0_BASE_ADDR + 0x018);
	writel(0x00008000, MMDC_P0_BASE_ADDR + 0x01c);
	writel(0x000026D2, MMDC_P0_BASE_ADDR + 0x02c);
	writel(0x008E1023, MMDC_P0_BASE_ADDR + 0x030);
	writel(0x00000047, MMDC_P0_BASE_ADDR + 0x040);
	writel(0x14420000, MMDC_P0_BASE_ADDR + 0x400);
	writel(0x841A0000, MMDC_P0_BASE_ADDR + 0x000);
	writel(0x00400c58, MMDC_P0_BASE_ADDR + 0x890);
	/* add NOC DDR configuration */
	writel(0x00000000, NOC_DDR_BASE_ADDR + 0x008);
	writel(0x2871C39B, NOC_DDR_BASE_ADDR + 0x00c);
	writel(0x000005B4, NOC_DDR_BASE_ADDR + 0x038);
	writel(0x00000040, NOC_DDR_BASE_ADDR + 0x014);
	writel(0x00000020, NOC_DDR_BASE_ADDR + 0x028);
	writel(0x00000020, NOC_DDR_BASE_ADDR + 0x02c);
	writel(0x02088032, MMDC_P0_BASE_ADDR + 0x01c);
	writel(0x00008033, MMDC_P0_BASE_ADDR + 0x01c);
	writel(0x00048031, MMDC_P0_BASE_ADDR + 0x01c);
	writel(0x19308030, MMDC_P0_BASE_ADDR + 0x01c);
	writel(0x04008040, MMDC_P0_BASE_ADDR + 0x01c);
	writel(0x00007800, MMDC_P0_BASE_ADDR + 0x020);
	writel(0x00022227, MMDC_P0_BASE_ADDR + 0x818);
	writel(0x00022227, MMDC_P1_BASE_ADDR + 0x818);
	writel(0x00025576, MMDC_P0_BASE_ADDR + 0x004);
	writel(0x00011006, MMDC_P0_BASE_ADDR + 0x404);
	writel(0x00000000, MMDC_P0_BASE_ADDR + 0x01c);
}

static void spl_dram_init(void)
{
	if (is_mx6dqp()) {
		mx6dq_dram_iocfg(64, &mx6qp_ddr_ioregs, &mx6qp_grp_ioregs);
		spl_dram_init_imx6qp_lpddr3();
	} else if (is_cpu_type(MXC_CPU_MX6SOLO)) {
		mx6sdl_dram_iocfg(32, &mx6sdl_ddr_ioregs, &mx6sdl_grp_ioregs);
		mx6_dram_cfg(&mem_s, &mx6s_512m_mmdc_calib, &h5tq2g63dfr);
	} else if (is_cpu_type(MXC_CPU_MX6DL)) {
		mx6sdl_dram_iocfg(64, &mx6sdl_ddr_ioregs, &mx6sdl_grp_ioregs);
		mx6_dram_cfg(&mem_dl, &mx6dl_1g_mmdc_calib, &h5tq2g63dfr);
	} else if (is_cpu_type(MXC_CPU_MX6Q)) {
		mx6dq_dram_iocfg(64, &mx6dq_ddr_ioregs, &mx6dq_grp_ioregs);
		mx6_dram_cfg(&mem_q, &mx6q_2g_mmdc_calib, &h5t04g63afr);
	}

	udelay(100);
}

void board_init_f(ulong dummy)
{
	ccgr_init();

	/* setup AIPS and disable watchdog */
	arch_cpu_init();

	gpr_init();

	/* iomux */
	board_early_init_f();

	/* setup GP timer */
	timer_init();

	/* UART clocks enabled and gd valid - init serial console */
	preloader_console_init();

	/* DDR initialization */
	spl_dram_init();
}

#define USDHC1_CD_GPIO		IMX_GPIO_NR(1, 2)
#define USDHC3_CD_GPIO		IMX_GPIO_NR(3, 9)

#define USDHC_PAD_CTRL (PAD_CTL_PUS_47K_UP |			\
	PAD_CTL_SPEED_LOW | PAD_CTL_DSE_80ohm |			\
	PAD_CTL_SRE_FAST  | PAD_CTL_HYS)

static struct fsl_esdhc_cfg usdhc_cfg[2] = {
	{USDHC3_BASE_ADDR},
	{USDHC1_BASE_ADDR},
};

static iomux_v3_cfg_t const usdhc1_pads[] = {
	IOMUX_PADS(PAD_SD1_CLK__SD1_CLK    | MUX_PAD_CTRL(USDHC_PAD_CTRL)),
	IOMUX_PADS(PAD_SD1_CMD__SD1_CMD    | MUX_PAD_CTRL(USDHC_PAD_CTRL)),
	IOMUX_PADS(PAD_SD1_DAT0__SD1_DATA0 | MUX_PAD_CTRL(USDHC_PAD_CTRL)),
	IOMUX_PADS(PAD_SD1_DAT1__SD1_DATA1 | MUX_PAD_CTRL(USDHC_PAD_CTRL)),
	IOMUX_PADS(PAD_SD1_DAT2__SD1_DATA2 | MUX_PAD_CTRL(USDHC_PAD_CTRL)),
	IOMUX_PADS(PAD_SD1_DAT3__SD1_DATA3 | MUX_PAD_CTRL(USDHC_PAD_CTRL)),
	/* Carrier MicroSD Card Detect */
	IOMUX_PADS(PAD_GPIO_2__GPIO1_IO02  | MUX_PAD_CTRL(NO_PAD_CTRL)),
};

static iomux_v3_cfg_t const usdhc3_pads[] = {
	IOMUX_PADS(PAD_SD3_CLK__SD3_CLK    | MUX_PAD_CTRL(USDHC_PAD_CTRL)),
	IOMUX_PADS(PAD_SD3_CMD__SD3_CMD    | MUX_PAD_CTRL(USDHC_PAD_CTRL)),
	IOMUX_PADS(PAD_SD3_DAT0__SD3_DATA0 | MUX_PAD_CTRL(USDHC_PAD_CTRL)),
	IOMUX_PADS(PAD_SD3_DAT1__SD3_DATA1 | MUX_PAD_CTRL(USDHC_PAD_CTRL)),
	IOMUX_PADS(PAD_SD3_DAT2__SD3_DATA2 | MUX_PAD_CTRL(USDHC_PAD_CTRL)),
	IOMUX_PADS(PAD_SD3_DAT3__SD3_DATA3 | MUX_PAD_CTRL(USDHC_PAD_CTRL)),
	/* SOM MicroSD Card Detect */
	IOMUX_PADS(PAD_EIM_DA9__GPIO3_IO09  | MUX_PAD_CTRL(NO_PAD_CTRL)),
};

int board_mmc_getcd(struct mmc *mmc)
{
	struct fsl_esdhc_cfg *cfg = (struct fsl_esdhc_cfg *)mmc->priv;
	int ret = 0;

	switch (cfg->esdhc_base) {
	case USDHC1_BASE_ADDR:
		ret = !gpio_get_value(USDHC1_CD_GPIO);
		break;
	case USDHC3_BASE_ADDR:
		ret = !gpio_get_value(USDHC3_CD_GPIO);
		break;
	}

	return ret;
}

int board_mmc_init(struct bd_info *bis)
{
	int ret;
	u32 index = 0;

	/*
	 * Following map is done:
	 * (U-Boot device node)    (Physical Port)
	 * mmc0                    SOM MicroSD
	 * mmc1                    Carrier board MicroSD
	 */
	for (index = 0; index < CFG_SYS_FSL_USDHC_NUM; ++index) {
		switch (index) {
		case 0:
			SETUP_IOMUX_PADS(usdhc3_pads);
			usdhc_cfg[0].sdhc_clk = mxc_get_clock(MXC_ESDHC3_CLK);
			usdhc_cfg[0].max_bus_width = 4;
			gpio_direction_input(USDHC3_CD_GPIO);
			break;
		case 1:
			SETUP_IOMUX_PADS(usdhc1_pads);
			usdhc_cfg[1].sdhc_clk = mxc_get_clock(MXC_ESDHC_CLK);
			usdhc_cfg[1].max_bus_width = 4;
			gpio_direction_input(USDHC1_CD_GPIO);
			break;
		default:
			printf("Warning: you configured more USDHC controllers"
			       "(%d) then supported by the board (%d)\n",
			       index + 1, CFG_SYS_FSL_USDHC_NUM);
			return -EINVAL;
		}

		ret = fsl_esdhc_initialize(bis, &usdhc_cfg[index]);
		if (ret)
			return ret;
	}

	return 0;
}
