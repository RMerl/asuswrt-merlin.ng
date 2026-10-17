// SPDX-License-Identifier: GPL-2.0+
/*
 * K+P iMX6Q KP_IMX6Q_TPC board configuration
 *
 * Copyright (C) 2018 Lukasz Majewski <lukma@denx.de>
 */

#include <common.h>
#include <init.h>
#include <log.h>
#include <asm/arch/clock.h>
#include <asm/arch/crm_regs.h>
#include <asm/arch/imx-regs.h>
#include <asm/arch/mx6-ddr.h>
#include <asm/arch/sys_proto.h>
#include <asm/global_data.h>
#include <asm/sections.h>
#include <asm/io.h>
#include <errno.h>
#include <spl.h>
#include <linux/delay.h>

DECLARE_GLOBAL_DATA_PTR;

static void ccgr_init(void)
{
	struct mxc_ccm_reg *ccm = (struct mxc_ccm_reg *)CCM_BASE_ADDR;

	writel(0x00C03F3F, &ccm->CCGR0);
	writel(0x0030FC03, &ccm->CCGR1);
	writel(0x0FFFC000, &ccm->CCGR2);
	writel(0x3FF00000, &ccm->CCGR3);
	writel(0x00FFF300, &ccm->CCGR4);
	writel(0x0F0000C3, &ccm->CCGR5);
	writel(0x000003FF, &ccm->CCGR6);
}

/* DDR3 */
static const struct mx6dq_iomux_ddr_regs mx6_ddr_ioregs = {
	.dram_sdclk_0 = 0x00000030,
	.dram_sdclk_1 = 0x00000030,
	.dram_cas = 0x00000030,
	.dram_ras = 0x00000030,
	.dram_reset = 0x00000030,
	.dram_sdcke0 = 0x00003000,
	.dram_sdcke1 = 0x00003000,
	.dram_sdba2 = 0x00000000,
	.dram_sdodt0 = 0x00000030,
	.dram_sdodt1 = 0x00000030,

	.dram_sdqs0 = 0x00000018,
	.dram_sdqs1 = 0x00000018,
	.dram_sdqs2 = 0x00000018,
	.dram_sdqs3 = 0x00000018,
	.dram_sdqs4 = 0x00000018,
	.dram_sdqs5 = 0x00000018,
	.dram_sdqs6 = 0x00000018,
	.dram_sdqs7 = 0x00000018,

	.dram_dqm0 = 0x00000018,
	.dram_dqm1 = 0x00000018,
	.dram_dqm2 = 0x00000018,
	.dram_dqm3 = 0x00000018,
	.dram_dqm4 = 0x00000018,
	.dram_dqm5 = 0x00000018,
	.dram_dqm6 = 0x00000018,
	.dram_dqm7 = 0x00000018,
};

static const struct mx6dq_iomux_grp_regs mx6_grp_ioregs = {
	.grp_ddr_type = 0x000c0000,
	.grp_ddrmode_ctl = 0x00020000,
	.grp_ddrpke = 0x00000000,
	.grp_addds = 0x00000030,
	.grp_ctlds = 0x00000030,
	.grp_ddrmode = 0x00020000,
	.grp_b0ds = 0x00000018,
	.grp_b1ds = 0x00000018,
	.grp_b2ds = 0x00000018,
	.grp_b3ds = 0x00000018,
	.grp_b4ds = 0x00000018,
	.grp_b5ds = 0x00000018,
	.grp_b6ds = 0x00000018,
	.grp_b7ds = 0x00000018,
};

static const struct mx6_mmdc_calibration mx6_4x256mx16_mmdc_calib = {
	.p0_mpwldectrl0 = 0x001F001F,
	.p0_mpwldectrl1 = 0x001F001F,
	.p1_mpwldectrl0 = 0x001F001F,
	.p1_mpwldectrl1 = 0x001F001F,
	.p0_mpdgctrl0 = 0x43270338,
	.p0_mpdgctrl1 = 0x03200314,
	.p1_mpdgctrl0 = 0x431A032F,
	.p1_mpdgctrl1 = 0x03200263,
	.p0_mprddlctl = 0x4B434748,
	.p1_mprddlctl = 0x4445404C,
	.p0_mpwrdlctl = 0x38444542,
	.p1_mpwrdlctl = 0x4935493A,
};

/* MT41K256M16 (4Gb density) */
static const struct mx6_ddr3_cfg mt41k256m16 = {
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

#ifdef CONFIG_MX6_DDRCAL
static void spl_dram_print_cal(struct mx6_ddr_sysinfo const *sysinfo)
{
	struct mx6_mmdc_calibration calibration = {0};

	mmdc_read_calibration(sysinfo, &calibration);

	debug(".p0_mpdgctrl0\t= 0x%08X\n", calibration.p0_mpdgctrl0);
	debug(".p0_mpdgctrl1\t= 0x%08X\n", calibration.p0_mpdgctrl1);
	debug(".p0_mprddlctl\t= 0x%08X\n", calibration.p0_mprddlctl);
	debug(".p0_mpwrdlctl\t= 0x%08X\n", calibration.p0_mpwrdlctl);
	debug(".p0_mpwldectrl0\t= 0x%08X\n", calibration.p0_mpwldectrl0);
	debug(".p0_mpwldectrl1\t= 0x%08X\n", calibration.p0_mpwldectrl1);
	debug(".p1_mpdgctrl0\t= 0x%08X\n", calibration.p1_mpdgctrl0);
	debug(".p1_mpdgctrl1\t= 0x%08X\n", calibration.p1_mpdgctrl1);
	debug(".p1_mprddlctl\t= 0x%08X\n", calibration.p1_mprddlctl);
	debug(".p1_mpwrdlctl\t= 0x%08X\n", calibration.p1_mpwrdlctl);
	debug(".p1_mpwldectrl0\t= 0x%08X\n", calibration.p1_mpwldectrl0);
	debug(".p1_mpwldectrl1\t= 0x%08X\n", calibration.p1_mpwldectrl1);
}

static void spl_dram_perform_cal(struct mx6_ddr_sysinfo const *sysinfo)
{
	int ret;

	/* Perform DDR DRAM calibration */
	udelay(100);
	ret = mmdc_do_write_level_calibration(sysinfo);
	if (ret) {
		printf("DDR: Write level calibration error [%d]\n", ret);
		return;
	}

	ret = mmdc_do_dqs_calibration(sysinfo);
	if (ret) {
		printf("DDR: DQS calibration error [%d]\n", ret);
		return;
	}

	spl_dram_print_cal(sysinfo);
}
#endif /* CONFIG_MX6_DDRCAL */

static void spl_dram_init(void)
{
	struct mx6_ddr_sysinfo sysinfo = {
		/* width of data bus:0=16,1=32,2=64 */
		.dsize = 2,
		/* config for full 4GB range so that get_mem_size() works */
		.cs_density = 32, /* 32Gb per CS */
		/* single chip select */
		.ncs = 1,
		.cs1_mirror = 0,
		.rtt_wr = 1 /*DDR3_RTT_60_OHM*/,	/* RTT_Wr = RZQ/4 */
		.rtt_nom = 2 /*DDR3_RTT_120_OHM*/,	/* RTT_Nom = RZQ/2 */
		.walat = 1,	/* Write additional latency */
		.ralat = 5,	/* Read additional latency */
		.mif3_mode = 3,	/* Command prediction working mode */
		.bi_on = 1,	/* Bank interleaving enabled */
		.sde_to_rst = 0x10,	/* 14 cycles, 200us (JEDEC default) */
		.rst_to_cke = 0x23,	/* 33 cycles, 500us (JEDEC default) */
		.pd_fast_exit = 1, /* enable precharge power-down fast exit */
		.ddr_type = DDR_TYPE_DDR3,
		.refsel = 1,	/* Refresh cycles at 32KHz */
		.refr = 7,	/* 8 refresh commands per refresh cycle */
	};

	mx6dq_dram_iocfg(64, &mx6_ddr_ioregs, &mx6_grp_ioregs);
	mx6_dram_cfg(&sysinfo, &mx6_4x256mx16_mmdc_calib, &mt41k256m16);

#ifdef CONFIG_MX6_DDRCAL
	spl_dram_perform_cal(&sysinfo);
#endif
}

void board_boot_order(u32 *spl_boot_list)
{
	u32 boot_device = spl_boot_device();
	u32 reg = imx6_src_get_boot_mode();

	reg = (reg & IMX6_BMODE_MASK) >> IMX6_BMODE_SHIFT;

	debug("%s: boot device: 0x%x (0x4 SD, 0x6 eMMC)\n", __func__, reg);
	if (boot_device == BOOT_DEVICE_MMC1)
		if (reg == IMX6_BMODE_MMC || reg == IMX6_BMODE_EMMC)
			boot_device = BOOT_DEVICE_MMC2;

	spl_boot_list[0] = boot_device;
	/*
	 * Below boot device is a 'fallback' - it shall always be possible to
	 * boot from SD card
	 */
	spl_boot_list[1] = BOOT_DEVICE_MMC1;
}

void board_init_f(ulong dummy)
{
	/* setup AIPS and disable watchdog */
	arch_cpu_init();

	ccgr_init();
	gpr_init();

	/* setup GP timer */
	timer_init();

	/* Early - pre reloc - driver model setup */
	spl_early_init();

	/* UART clocks enabled and gd valid - init serial console */
	preloader_console_init();

	/* DDR initialization */
	spl_dram_init();

	/* Clear the BSS. */
	memset(__bss_start, 0, __bss_end - __bss_start);
}
