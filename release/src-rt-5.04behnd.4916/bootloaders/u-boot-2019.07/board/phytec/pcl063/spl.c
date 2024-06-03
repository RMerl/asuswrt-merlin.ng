// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2018 Collabora Ltd.
 *
 * Based on board/ccv/xpress/spl.c:
 * Copyright (C) 2015-2016 Stefan Roese <sr@denx.de>
 */

#include <common.h>
#include <spl.h>
#include <asm/arch/clock.h>
#include <asm/io.h>
#include <asm/arch/mx6-ddr.h>
#include <asm/arch/mx6-pins.h>
#include <asm/arch/crm_regs.h>
#include <asm/arch/sys_proto.h>
#include <fsl_esdhc.h>

/* Configuration for Micron MT41K256M16TW-107 IT:P, 32M x 16 x 8 -> 256MiB */

static struct mx6ul_iomux_grp_regs mx6_grp_ioregs = {
	.grp_addds = 0x00000030,
	.grp_ddrmode_ctl = 0x00020000,
	.grp_b0ds = 0x00000030,
	.grp_ctlds = 0x00000030,
	.grp_b1ds = 0x00000030,
	.grp_ddrpke = 0x00000000,
	.grp_ddrmode = 0x00020000,
	.grp_ddr_type = 0x000c0000,
};

static struct mx6ul_iomux_ddr_regs mx6_ddr_ioregs = {
	.dram_dqm0 = 0x00000030,
	.dram_dqm1 = 0x00000030,
	.dram_ras = 0x00000030,
	.dram_cas = 0x00000030,
	.dram_odt0 = 0x00000030,
	.dram_odt1 = 0x00000030,
	.dram_sdba2 = 0x00000000,
	.dram_sdclk_0 = 0x00000030,
	.dram_sdqs0 = 0x00000030,
	.dram_sdqs1 = 0x00000030,
	.dram_reset = 0x00000030,
};

static struct mx6_mmdc_calibration mx6_mmcd_calib = {
	.p0_mpwldectrl0 = 0x00000000,
	.p0_mpdgctrl0 = 0x41480148,
	.p0_mprddlctl = 0x40403E42,
	.p0_mpwrdlctl = 0x40405852,
};

struct mx6_ddr_sysinfo ddr_sysinfo = {
	.dsize = 0,		/* Bus size = 16bit */
	.cs_density = 18,
	.ncs = 1,
	.cs1_mirror = 0,
	.rtt_wr = 1,
	.rtt_nom = 1,
	.walat = 1,		/* Write additional latency */
	.ralat = 5,		/* Read additional latency */
	.mif3_mode = 3,		/* Command prediction working mode */
	.bi_on = 1,		/* Bank interleaving enabled */
	.pd_fast_exit = 1,
	.sde_to_rst = 0x10,	/* 14 cycles, 200us (JEDEC default) */
	.rst_to_cke = 0x23,	/* 33 cycles, 500us (JEDEC default) */
	.ddr_type = DDR_TYPE_DDR3,
	.refsel = 1,		/* Refresh cycles at 32KHz */
	.refr = 7,		/* 8 refresh commands per refresh cycle */
};

static struct mx6_ddr3_cfg mem_ddr = {
	.mem_speed = 933,
	.density = 4,
	.width = 16,
	.banks = 8,
	.rowaddr = 14,
	.coladdr = 10,
	.pagesz = 1,
	.trcd = 1391,
	.trcmin = 4791,
	.trasmin = 3400,
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

static void spl_dram_init(void)
{
	mx6ul_dram_iocfg(mem_ddr.width, &mx6_ddr_ioregs, &mx6_grp_ioregs);
	mx6_dram_cfg(&ddr_sysinfo, &mx6_mmcd_calib, &mem_ddr);
}

#ifdef CONFIG_FSL_ESDHC

#define USDHC_PAD_CTRL (PAD_CTL_PKE         | PAD_CTL_PUE       | \
			PAD_CTL_PUS_22K_UP  | PAD_CTL_SPEED_LOW | \
			PAD_CTL_DSE_80ohm   | PAD_CTL_SRE_FAST  | \
			PAD_CTL_HYS)

static iomux_v3_cfg_t const usdhc1_pads[] = {
	MX6_PAD_SD1_CLK__USDHC1_CLK | MUX_PAD_CTRL(USDHC_PAD_CTRL),
	MX6_PAD_SD1_CMD__USDHC1_CMD | MUX_PAD_CTRL(USDHC_PAD_CTRL),
	MX6_PAD_SD1_DATA0__USDHC1_DATA0 | MUX_PAD_CTRL(USDHC_PAD_CTRL),
	MX6_PAD_SD1_DATA1__USDHC1_DATA1 | MUX_PAD_CTRL(USDHC_PAD_CTRL),
	MX6_PAD_SD1_DATA2__USDHC1_DATA2 | MUX_PAD_CTRL(USDHC_PAD_CTRL),
	MX6_PAD_SD1_DATA3__USDHC1_DATA3 | MUX_PAD_CTRL(USDHC_PAD_CTRL),
	MX6_PAD_UART1_RTS_B__USDHC1_CD_B | MUX_PAD_CTRL(USDHC_PAD_CTRL),
};

#ifndef CONFIG_NAND_MXS
static iomux_v3_cfg_t const usdhc2_pads[] = {
	MX6_PAD_NAND_RE_B__USDHC2_CLK    | MUX_PAD_CTRL(USDHC_PAD_CTRL),
	MX6_PAD_NAND_WE_B__USDHC2_CMD    | MUX_PAD_CTRL(USDHC_PAD_CTRL),
	MX6_PAD_NAND_DATA00__USDHC2_DATA0 | MUX_PAD_CTRL(USDHC_PAD_CTRL),
	MX6_PAD_NAND_DATA01__USDHC2_DATA1 | MUX_PAD_CTRL(USDHC_PAD_CTRL),
	MX6_PAD_NAND_DATA02__USDHC2_DATA2 | MUX_PAD_CTRL(USDHC_PAD_CTRL),
	MX6_PAD_NAND_DATA03__USDHC2_DATA3 | MUX_PAD_CTRL(USDHC_PAD_CTRL),
	MX6_PAD_NAND_DATA04__USDHC2_DATA4 | MUX_PAD_CTRL(USDHC_PAD_CTRL),
	MX6_PAD_NAND_DATA05__USDHC2_DATA5 | MUX_PAD_CTRL(USDHC_PAD_CTRL),
	MX6_PAD_NAND_DATA06__USDHC2_DATA6 | MUX_PAD_CTRL(USDHC_PAD_CTRL),
	MX6_PAD_NAND_DATA07__USDHC2_DATA7 | MUX_PAD_CTRL(USDHC_PAD_CTRL),
};
#endif

static struct fsl_esdhc_cfg usdhc_cfg[] = {
	{
		.esdhc_base = USDHC1_BASE_ADDR,
		.max_bus_width = 4,
	},
#ifndef CONFIG_NAND_MXS
	{
		.esdhc_base = USDHC2_BASE_ADDR,
		.max_bus_width = 8,
	},
#endif
};

int board_mmc_getcd(struct mmc *mmc)
{
	return 1;
}

int board_mmc_init(bd_t *bis)
{
	int i, ret;

	for (i = 0; i < CONFIG_SYS_FSL_USDHC_NUM; i++) {
		switch (i) {
		case 0:
			SETUP_IOMUX_PADS(usdhc1_pads);
			usdhc_cfg[i].sdhc_clk = mxc_get_clock(MXC_ESDHC_CLK);
			break;
#ifndef CONFIG_NAND_MXS
		case 1:
			SETUP_IOMUX_PADS(usdhc2_pads);
			usdhc_cfg[i].sdhc_clk = mxc_get_clock(MXC_ESDHC_CLK);
			break;
#endif
		default:
			printf("Warning - USDHC%d controller not supporting\n",
			       i + 1);
			return 0;
		}

		ret = fsl_esdhc_initialize(bis, &usdhc_cfg[i]);
		if (ret) {
			printf("Warning: failed to initialize mmc dev %d\n", i);
			return ret;
		}
	}

	return 0;
}

void board_boot_order(u32 *spl_boot_list)
{
	u32 bmode = imx6_src_get_boot_mode();
	u8 boot_dev = BOOT_DEVICE_MMC1;

	switch ((bmode & IMX6_BMODE_MASK) >> IMX6_BMODE_SHIFT) {
	case IMX6_BMODE_SD:
	case IMX6_BMODE_ESD:
		boot_dev = BOOT_DEVICE_MMC1;
		break;
	case IMX6_BMODE_MMC:
	case IMX6_BMODE_EMMC:
		boot_dev = BOOT_DEVICE_MMC2;
		break;
	default:
		/* Default - BOOT_DEVICE_MMC1 */
		printf("Wrong board boot order\n");
		break;
	}

	spl_boot_list[0] = boot_dev;
}
#endif /* CONFIG_FSL_ESDHC */

void board_init_f(ulong dummy)
{
	ccgr_init();

	/* Setup AIPS and disable watchdog */
	arch_cpu_init();

	/* Setup iomux and fec */
	board_early_init_f();

	/* Setup GP timer */
	timer_init();

	/* UART clocks enabled and gd valid - init serial console */
	preloader_console_init();

	/* DDR initialization */
	spl_dram_init();
}
