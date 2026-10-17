// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2018 Armadeus Systems
 */

#include <init.h>
#include <asm/arch/clock.h>
#include <asm/arch/crm_regs.h>
#include <asm/arch/imx-regs.h>
#include <asm/arch/sys_proto.h>
#include <asm/global_data.h>
#include <asm/io.h>
#include <common.h>
#include <env.h>

DECLARE_GLOBAL_DATA_PTR;

#ifdef CONFIG_FEC_MXC
#include <miiphy.h>

int board_phy_config(struct phy_device *phydev)
{
	phy_write(phydev, MDIO_DEVAD_NONE, 0x1f, 0x8190);

	if (phydev->drv->config)
		phydev->drv->config(phydev);

	return 0;
}

static int setup_fec(void)
{
	struct iomuxc *const iomuxc_regs = (struct iomuxc *)IOMUXC_BASE_ADDR;

	/* Use 50M anatop loopback REF_CLK1 for ENET1,
	 * clear gpr1[13], set gpr1[17] */
	clrsetbits_le32(&iomuxc_regs->gpr[1], IOMUX_GPR1_FEC1_MASK,
			IOMUX_GPR1_FEC1_CLOCK_MUX1_SEL_MASK);

	return enable_fec_anatop_clock(0, ENET_50MHZ);
}
#endif /* CONFIG_FEC_MXC */

int board_init(void)
{
	/* Address of boot parameters */
	gd->bd->bi_boot_params = CFG_SYS_SDRAM_BASE + 0x100;

#ifdef CONFIG_FEC_MXC
	setup_fec();
#endif

	return 0;
}

int __weak opos6ul_board_late_init(void)
{
	return 0;
}

int board_late_init(void)
{
	struct src *psrc = (struct src *)SRC_BASE_ADDR;
	unsigned reg = readl(&psrc->sbmr2);

	/* In bootstrap don't use the env vars */
	if (((reg & 0x3000000) >> 24) == 0x1) {
		env_set_default(NULL, 0);
		env_set("preboot", "");
	}

	return opos6ul_board_late_init();
}

int dram_init(void)
{
	gd->ram_size = imx_ddr_size();

	return 0;
}

#ifdef CONFIG_SPL_BUILD
#include <asm/arch/mx6-ddr.h>
#include <linux/libfdt.h>
#include <spl.h>

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
	.dram_sdclk_0 = 0x00000008,
	.dram_sdqs0 = 0x00000038,
	.dram_sdqs1 = 0x00000030,
	.dram_reset = 0x00000030,
};

static struct mx6_mmdc_calibration mx6_mmcd_calib = {
	.p0_mpwldectrl0 = 0x00070007,
	.p0_mpdgctrl0 = 0x41490145,
	.p0_mprddlctl = 0x40404546,
	.p0_mpwrdlctl = 0x4040524D,
};

struct mx6_ddr_sysinfo ddr_sysinfo = {
	.dsize = 0,
	.cs_density = 20,
	.ncs = 1,
	.cs1_mirror = 0,
	.rtt_wr = 2,
	.rtt_nom = 1,		/* RTT_Nom = RZQ/2 */
	.walat = 1,		/* Write additional latency */
	.ralat = 5,		/* Read additional latency */
	.mif3_mode = 3,		/* Command prediction working mode */
	.bi_on = 1,		/* Bank interleaving enabled */
	.sde_to_rst = 0x10,	/* 14 cycles, 200us (JEDEC default) */
	.rst_to_cke = 0x23,	/* 33 cycles, 500us (JEDEC default) */
	.ddr_type = DDR_TYPE_DDR3,
	.refsel = 1,		/* Refresh cycles at 32KHz */
	.refr = 7,		/* 8 refreshes commands per refresh cycle */
};

static struct mx6_ddr3_cfg mem_ddr = {
	.mem_speed = 800,
	.density = 2,
	.width = 16,
	.banks = 8,
	.rowaddr = 14,
	.coladdr = 10,
	.pagesz = 2,
	.trcd = 1500,
	.trcmin = 5250,
	.trasmin = 3750,
};

void board_boot_order(u32 *spl_boot_list)
{
	unsigned int bmode = readl(&src_base->sbmr2);

	if (((bmode >> 24) & 0x03) == 0x01) /* Serial Downloader */
		spl_boot_list[0] = BOOT_DEVICE_UART;
	else
		spl_boot_list[0] = spl_boot_device();
}

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
	writel(0xFFFFFFFF, &ccm->CCGR7);
}

static void spl_dram_init(void)
{
	struct ocotp_regs *ocotp = (struct ocotp_regs *)OCOTP_BASE_ADDR;
	struct fuse_bank *bank = &ocotp->bank[4];
	struct fuse_bank4_regs *fuse =
		(struct fuse_bank4_regs *)bank->fuse_regs;
	int reg = readl(&fuse->gp1);

	/* 512MB of RAM */
	if (reg & 0x1) {
		mem_ddr.density = 4;
		mem_ddr.rowaddr = 15;
		mem_ddr.trcd = 1375;
		mem_ddr.trcmin = 4875;
		mem_ddr.trasmin = 3500;
	}

	mx6ul_dram_iocfg(mem_ddr.width, &mx6_ddr_ioregs, &mx6_grp_ioregs);
	mx6_dram_cfg(&ddr_sysinfo, &mx6_mmcd_calib, &mem_ddr);
}

void spl_board_init(void)
{
	preloader_console_init();
}

void board_init_f(ulong dummy)
{
	ccgr_init();

	/* setup AIPS and disable watchdog */
	arch_cpu_init();

	/* setup GP timer */
	timer_init();

	/* DDR initialization */
	spl_dram_init();
}
#endif /* CONFIG_SPL_BUILD */
