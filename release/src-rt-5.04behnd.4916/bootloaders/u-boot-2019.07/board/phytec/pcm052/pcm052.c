// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2018
 * Lukasz Majewski, DENX Software Engineering, lukma@denx.de.
 *
 * Copyright 2013 Freescale Semiconductor, Inc.
 */

#include <common.h>
#include <asm/io.h>
#include <asm/arch/imx-regs.h>
#include <asm/arch/iomux-vf610.h>
#include <asm/arch/ddrmc-vf610.h>
#include <asm/arch/crm_regs.h>
#include <asm/arch/clock.h>
#include <led.h>
#include <environment.h>
#include <miiphy.h>

DECLARE_GLOBAL_DATA_PTR;

static struct ddrmc_cr_setting pcm052_cr_settings[] = {
	/* not in the datasheets, but in the original code */
	{ 0x00002000, 105 },
	{ 0x00000020, 110 },
	/* AXI */
	{ DDRMC_CR117_AXI0_W_PRI(1) | DDRMC_CR117_AXI0_R_PRI(1), 117 },
	{ DDRMC_CR118_AXI1_W_PRI(1) | DDRMC_CR118_AXI1_R_PRI(1), 118 },
	{ DDRMC_CR120_AXI0_PRI1_RPRI(2) |
		   DDRMC_CR120_AXI0_PRI0_RPRI(2), 120 },
	{ DDRMC_CR121_AXI0_PRI3_RPRI(2) |
		   DDRMC_CR121_AXI0_PRI2_RPRI(2), 121 },
	{ DDRMC_CR122_AXI1_PRI1_RPRI(1) | DDRMC_CR122_AXI1_PRI0_RPRI(1) |
		   DDRMC_CR122_AXI0_PRIRLX(100), 122 },
	{ DDRMC_CR123_AXI1_P_ODR_EN | DDRMC_CR123_AXI1_PRI3_RPRI(1) |
		   DDRMC_CR123_AXI1_PRI2_RPRI(1), 123 },
	{ DDRMC_CR124_AXI1_PRIRLX(100), 124 },
	{ DDRMC_CR126_PHY_RDLAT(11), 126 },
	{ DDRMC_CR132_WRLAT_ADJ(5) | DDRMC_CR132_RDLAT_ADJ(6), 132 },
	{ DDRMC_CR137_PHYCTL_DL(2), 137 },
	{ DDRMC_CR139_PHY_WRLV_RESPLAT(4) | DDRMC_CR139_PHY_WRLV_LOAD(7) |
		   DDRMC_CR139_PHY_WRLV_DLL(3) |
		   DDRMC_CR139_PHY_WRLV_EN(3), 139 },
	{ DDRMC_CR154_PAD_ZQ_EARLY_CMP_EN_TIMER(13) |
		   DDRMC_CR154_PAD_ZQ_MODE(1) |
		   DDRMC_CR154_DDR_SEL_PAD_CONTR(3) |
		   DDRMC_CR154_PAD_ZQ_HW_FOR(0), 154 },
	{ DDRMC_CR155_PAD_ODT_BYTE1(5) | DDRMC_CR155_PAD_ODT_BYTE0(5), 155 },
	{ DDRMC_CR158_TWR(6), 158 },
	{ DDRMC_CR161_ODT_EN(0) | DDRMC_CR161_TODTH_RD(0) |
		   DDRMC_CR161_TODTH_WR(6), 161 },
	/* end marker */
	{ 0, -1 }
};

/* PHY settings -- most of them differ from default in imx-regs.h */

#define PCM052_DDRMC_PHY_DQ_TIMING			0x00002213
#define PCM052_DDRMC_PHY_CTRL				0x00290000
#define PCM052_DDRMC_PHY_SLAVE_CTRL			0x00002c00
#define PCM052_DDRMC_PHY_PROC_PAD_ODT			0x00010020

static struct ddrmc_phy_setting pcm052_phy_settings[] = {
	{ PCM052_DDRMC_PHY_DQ_TIMING,  0 },
	{ PCM052_DDRMC_PHY_DQ_TIMING, 16 },
	{ PCM052_DDRMC_PHY_DQ_TIMING, 32 },
	{ PCM052_DDRMC_PHY_DQ_TIMING, 48 },
	{ DDRMC_PHY_DQS_TIMING,  1 },
	{ DDRMC_PHY_DQS_TIMING, 17 },
	{ DDRMC_PHY_DQS_TIMING, 33 },
	{ DDRMC_PHY_DQS_TIMING, 49 },
	{ PCM052_DDRMC_PHY_CTRL,  2 },
	{ PCM052_DDRMC_PHY_CTRL, 18 },
	{ PCM052_DDRMC_PHY_CTRL, 34 },
	{ DDRMC_PHY_MASTER_CTRL,  3 },
	{ DDRMC_PHY_MASTER_CTRL, 19 },
	{ DDRMC_PHY_MASTER_CTRL, 35 },
	{ PCM052_DDRMC_PHY_SLAVE_CTRL,  4 },
	{ PCM052_DDRMC_PHY_SLAVE_CTRL, 20 },
	{ PCM052_DDRMC_PHY_SLAVE_CTRL, 36 },
	{ DDRMC_PHY50_DDR3_MODE | DDRMC_PHY50_EN_SW_HALF_CYCLE, 50 },
	{ PCM052_DDRMC_PHY_PROC_PAD_ODT, 52 },

	/* end marker */
	{ 0, -1 }
};

int dram_init(void)
{
#if defined(CONFIG_TARGET_PCM052)

	static const struct ddr3_jedec_timings pcm052_ddr_timings = {
		.tinit             = 5,
		.trst_pwron        = 80000,
		.cke_inactive      = 200000,
		.wrlat             = 5,
		.caslat_lin        = 12,
		.trc               = 6,
		.trrd              = 4,
		.tccd              = 4,
		.tbst_int_interval = 4,
		.tfaw              = 18,
		.trp               = 6,
		.twtr              = 4,
		.tras_min          = 15,
		.tmrd              = 4,
		.trtp              = 4,
		.tras_max          = 14040,
		.tmod              = 12,
		.tckesr            = 4,
		.tcke              = 3,
		.trcd_int          = 6,
		.tras_lockout      = 1,
		.tdal              = 10,
		.bstlen            = 3,
		.tdll              = 512,
		.trp_ab            = 6,
		.tref              = 1542,
		.trfc              = 64,
		.tref_int          = 5,
		.tpdex             = 3,
		.txpdll            = 10,
		.txsnr             = 68,
		.txsr              = 506,
		.cksrx             = 5,
		.cksre             = 5,
		.freq_chg_en       = 1,
		.zqcl              = 256,
		.zqinit            = 512,
		.zqcs              = 64,
		.ref_per_zq        = 64,
		.zqcs_rotate       = 1,
		.aprebit           = 10,
		.cmd_age_cnt       = 255,
		.age_cnt           = 255,
		.q_fullness        = 0,
		.odt_rd_mapcs0     = 1,
		.odt_wr_mapcs0     = 1,
		.wlmrd             = 40,
		.wldqsen           = 25,
	};

    const int row_diff = 2;

#elif defined(CONFIG_TARGET_BK4R1)

	static const struct ddr3_jedec_timings pcm052_ddr_timings = {
		.tinit             = 5,
		.trst_pwron        = 80000,
		.cke_inactive      = 200000,
		.wrlat             = 5,
		.caslat_lin        = 12,
		.trc               = 6,
		.trrd              = 4,
		.tccd              = 4,
		.tbst_int_interval = 0,
		.tfaw              = 16,
		.trp               = 6,
		.twtr              = 4,
		.tras_min          = 15,
		.tmrd              = 4,
		.trtp              = 4,
		.tras_max          = 28080,
		.tmod              = 12,
		.tckesr            = 4,
		.tcke              = 3,
		.trcd_int          = 6,
		.tras_lockout      = 1,
		.tdal              = 12,
		.bstlen            = 3,
		.tdll              = 512,
		.trp_ab            = 6,
		.tref              = 3120,
		.trfc              = 104,
		.tref_int          = 0,
		.tpdex             = 3,
		.txpdll            = 10,
		.txsnr             = 108,
		.txsr              = 512,
		.cksrx             = 5,
		.cksre             = 5,
		.freq_chg_en       = 1,
		.zqcl              = 256,
		.zqinit            = 512,
		.zqcs              = 64,
		.ref_per_zq        = 64,
		.zqcs_rotate       = 1,
		.aprebit           = 10,
		.cmd_age_cnt       = 255,
		.age_cnt           = 255,
		.q_fullness        = 0,
		.odt_rd_mapcs0     = 1,
		.odt_wr_mapcs0     = 1,
		.wlmrd             = 40,
		.wldqsen           = 25,
	};

    const int row_diff = 1;

#else /* Unknown PCM052 variant */

#error DDR characteristics undefined for this target. Please define them.

#endif

	ddrmc_ctrl_init_ddr3(&pcm052_ddr_timings, pcm052_cr_settings,
			     pcm052_phy_settings, 1, row_diff);

	gd->ram_size = get_ram_size((void *)PHYS_SDRAM, PHYS_SDRAM_SIZE);

	return 0;
}

static void clock_init(void)
{
	struct ccm_reg *ccm = (struct ccm_reg *)CCM_BASE_ADDR;
	struct anadig_reg *anadig = (struct anadig_reg *)ANADIG_BASE_ADDR;

	clrsetbits_le32(&ccm->ccgr0, CCM_REG_CTRL_MASK,
			CCM_CCGR0_UART1_CTRL_MASK);
	clrsetbits_le32(&ccm->ccgr1, CCM_REG_CTRL_MASK,
			CCM_CCGR1_PIT_CTRL_MASK | CCM_CCGR1_WDOGA5_CTRL_MASK);
	clrsetbits_le32(&ccm->ccgr2, CCM_REG_CTRL_MASK,
			CCM_CCGR2_IOMUXC_CTRL_MASK | CCM_CCGR2_PORTA_CTRL_MASK |
			CCM_CCGR2_PORTB_CTRL_MASK | CCM_CCGR2_PORTC_CTRL_MASK |
			CCM_CCGR2_PORTD_CTRL_MASK | CCM_CCGR2_PORTE_CTRL_MASK |
			CCM_CCGR2_QSPI0_CTRL_MASK);
	clrsetbits_le32(&ccm->ccgr3, CCM_REG_CTRL_MASK,
			CCM_CCGR3_ANADIG_CTRL_MASK | CCM_CCGR3_SCSC_CTRL_MASK);
	clrsetbits_le32(&ccm->ccgr4, CCM_REG_CTRL_MASK,
			CCM_CCGR4_WKUP_CTRL_MASK | CCM_CCGR4_CCM_CTRL_MASK |
			CCM_CCGR4_GPC_CTRL_MASK);
	clrsetbits_le32(&ccm->ccgr6, CCM_REG_CTRL_MASK,
			CCM_CCGR6_OCOTP_CTRL_MASK | CCM_CCGR6_DDRMC_CTRL_MASK);
	clrsetbits_le32(&ccm->ccgr7, CCM_REG_CTRL_MASK,
			CCM_CCGR7_SDHC1_CTRL_MASK);
	clrsetbits_le32(&ccm->ccgr9, CCM_REG_CTRL_MASK,
			CCM_CCGR9_FEC0_CTRL_MASK | CCM_CCGR9_FEC1_CTRL_MASK);
	clrsetbits_le32(&ccm->ccgr10, CCM_REG_CTRL_MASK,
			CCM_CCGR10_NFC_CTRL_MASK);

	clrsetbits_le32(&anadig->pll2_ctrl, ANADIG_PLL2_CTRL_POWERDOWN,
			ANADIG_PLL2_CTRL_ENABLE | ANADIG_PLL2_CTRL_DIV_SELECT);
	clrsetbits_le32(&anadig->pll1_ctrl, ANADIG_PLL1_CTRL_POWERDOWN,
			ANADIG_PLL1_CTRL_ENABLE | ANADIG_PLL1_CTRL_DIV_SELECT);

	clrsetbits_le32(&ccm->ccr, CCM_CCR_OSCNT_MASK,
			CCM_CCR_FIRC_EN | CCM_CCR_OSCNT(5));
	clrsetbits_le32(&ccm->ccsr, CCM_REG_CTRL_MASK,
			CCM_CCSR_PLL1_PFD_CLK_SEL(3) | CCM_CCSR_PLL2_PFD4_EN |
			CCM_CCSR_PLL2_PFD3_EN | CCM_CCSR_PLL2_PFD2_EN |
			CCM_CCSR_PLL2_PFD1_EN | CCM_CCSR_PLL1_PFD4_EN |
			CCM_CCSR_PLL1_PFD3_EN | CCM_CCSR_PLL1_PFD2_EN |
			CCM_CCSR_PLL1_PFD1_EN | CCM_CCSR_DDRC_CLK_SEL(1) |
			CCM_CCSR_FAST_CLK_SEL(1) | CCM_CCSR_SYS_CLK_SEL(4));
	clrsetbits_le32(&ccm->cacrr, CCM_REG_CTRL_MASK,
			CCM_CACRR_IPG_CLK_DIV(1) | CCM_CACRR_BUS_CLK_DIV(2) |
			CCM_CACRR_ARM_CLK_DIV(0));
	clrsetbits_le32(&ccm->cscmr1, CCM_REG_CTRL_MASK,
			CCM_CSCMR1_ESDHC1_CLK_SEL(3) |
			CCM_CSCMR1_QSPI0_CLK_SEL(3) |
			CCM_CSCMR1_NFC_CLK_SEL(0));
	clrsetbits_le32(&ccm->cscdr1, CCM_REG_CTRL_MASK,
			CCM_CSCDR1_RMII_CLK_EN);
	clrsetbits_le32(&ccm->cscdr2, CCM_REG_CTRL_MASK,
			CCM_CSCDR2_ESDHC1_EN | CCM_CSCDR2_ESDHC1_CLK_DIV(0) |
			CCM_CSCDR2_NFC_EN);
	clrsetbits_le32(&ccm->cscdr3, CCM_REG_CTRL_MASK,
			CCM_CSCDR3_QSPI0_EN | CCM_CSCDR3_QSPI0_DIV(1) |
			CCM_CSCDR3_QSPI0_X2_DIV(1) |
			CCM_CSCDR3_QSPI0_X4_DIV(3) |
			CCM_CSCDR3_NFC_PRE_DIV(5));
	clrsetbits_le32(&ccm->cscmr2, CCM_REG_CTRL_MASK,
			CCM_CSCMR2_RMII_CLK_SEL(0));
}

static void mscm_init(void)
{
	struct mscm_ir *mscmir = (struct mscm_ir *)MSCM_IR_BASE_ADDR;
	int i;

	for (i = 0; i < MSCM_IRSPRC_NUM; i++)
		writew(MSCM_IRSPRC_CP0_EN, &mscmir->irsprc[i]);
}

int board_early_init_f(void)
{
	clock_init();
	mscm_init();

	return 0;
}

int board_init(void)
{
	struct scsc_reg *scsc = (struct scsc_reg *)SCSC_BASE_ADDR;

	/* address of boot parameters */
	gd->bd->bi_boot_params = PHYS_SDRAM + 0x100;

	/*
	 * Enable external 32K Oscillator
	 *
	 * The internal clock experiences significant drift
	 * so we must use the external oscillator in order
	 * to maintain correct time in the hwclock
	 */
	setbits_le32(&scsc->sosc_ctr, SCSC_SOSC_CTR_SOSC_EN);

	return 0;
}

#ifdef CONFIG_TARGET_BK4R1
void imx_get_mac_from_fuse(int dev_id, unsigned char *mac)
{
	struct ocotp_regs *ocotp = (struct ocotp_regs *)OCOTP_BASE_ADDR;
	struct fuse_bank *bank = &ocotp->bank[4];
	struct fuse_bank4_regs *fuse =
		(struct fuse_bank4_regs *)bank->fuse_regs;
	u32 value;

	/*
	 * BK4 has different layout of stored MAC address
	 * than one used in imx_get_mac_from_fuse() @ generic.c
	 */

	switch (dev_id) {
	case 0:
		value = readl(&fuse->mac_addr1);

		mac[0] = value >> 8;
		mac[1] = value;

		value = readl(&fuse->mac_addr0);
		mac[2] = value >> 24;
		mac[3] = value >> 16;
		mac[4] = value >> 8;
		mac[5] = value;
		break;
	case 1:
		value = readl(&fuse->mac_addr2);

		mac[0] = value >> 24;
		mac[1] = value >> 16;
		mac[2] = value >> 8;
		mac[3] = value;

		value = readl(&fuse->mac_addr1);
		mac[4] = value >> 24;
		mac[5] = value >> 16;
		break;
	}
}

int board_late_init(void)
{
	struct src *psrc = (struct src *)SRC_BASE_ADDR;
	u32 reg;

	if (IS_ENABLED(CONFIG_LED))
		led_default_state();

	/*
	 * BK4r1 handle emergency/service SD card boot
	 * Checking the SBMR1 register BOOTCFG1 byte:
	 * NAND:
	 *      bit [2] - NAND data width - 16
	 *	bit [5] - NAND fast boot
	 *	bit [7] = 1 - NAND as a source of booting
	 * SD card (0x64):
	 *      bit [4] = 0 - SD card source
	 *	bit [6] = 1 - SD/MMC source
	 */

	reg = readl(&psrc->sbmr1);
	if ((reg & SRC_SBMR1_BOOTCFG1_SDMMC) &&
	    !(reg & SRC_SBMR1_BOOTCFG1_MMC)) {
		printf("------ SD card boot -------\n");
		set_default_env("!LVFBootloader", 0);
		env_set("bootcmd",
			"run prepare_install_bk4r1_envs; run install_bk4r1rs");
	}

	return 0;
}

/**
 * KSZ8081
 */
#define MII_KSZ8081_REFERENCE_CLOCK_SELECT	0x1f
#define RMII_50MHz_CLOCK	0x8180

int board_phy_config(struct phy_device *phydev)
{
	/* Set 50 MHz reference clock */
	phy_write(phydev, MDIO_DEVAD_NONE, MII_KSZ8081_REFERENCE_CLOCK_SELECT,
		  RMII_50MHz_CLOCK);

	return genphy_config(phydev);
}
#endif /* CONFIG_TARGET_BK4R1 */

int checkboard(void)
{
#ifdef CONFIG_TARGET_BK4R1
	puts("Board: BK4r1 (L333)\n");
#else
	puts("Board: PCM-052\n");
#endif
	return 0;
}
