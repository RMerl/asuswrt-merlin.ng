// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2013 Freescale Semiconductor, Inc.
 */

#include <common.h>
#include <asm/io.h>
#include <asm/arch/imx-regs.h>
#include <asm/arch/iomux-vf610.h>
#include <asm/arch/ddrmc-vf610.h>
#include <asm/arch/crm_regs.h>
#include <asm/arch/clock.h>
#include <mmc.h>
#include <fsl_esdhc.h>
#include <miiphy.h>
#include <netdev.h>
#include <i2c.h>

DECLARE_GLOBAL_DATA_PTR;

#define UART_PAD_CTRL	(PAD_CTL_PUS_100K_UP | PAD_CTL_SPEED_MED | \
			PAD_CTL_DSE_25ohm | PAD_CTL_OBE_IBE_ENABLE)

#define ESDHC_PAD_CTRL	(PAD_CTL_PUS_100K_UP | PAD_CTL_SPEED_HIGH | \
			PAD_CTL_DSE_20ohm | PAD_CTL_OBE_IBE_ENABLE)

#define ENET_PAD_CTRL	(PAD_CTL_PUS_47K_UP | PAD_CTL_SPEED_HIGH | \
			PAD_CTL_DSE_50ohm | PAD_CTL_OBE_IBE_ENABLE)

static struct ddrmc_cr_setting vf610twr_cr_settings[] = {
	/* levelling */
	{ DDRMC_CR97_WRLVL_EN, 97 },
	{ DDRMC_CR98_WRLVL_DL_0(0), 98 },
	{ DDRMC_CR99_WRLVL_DL_1(0), 99 },
	{ DDRMC_CR102_RDLVL_REG_EN | DDRMC_CR102_RDLVL_GT_REGEN, 102 },
	{ DDRMC_CR105_RDLVL_DL_0(0), 105 },
	{ DDRMC_CR106_RDLVL_GTDL_0(4), 106 },
	{ DDRMC_CR110_RDLVL_DL_1(0) | DDRMC_CR110_RDLVL_GTDL_1(4), 110 },
	/* AXI */
	{ DDRMC_CR117_AXI0_W_PRI(0) | DDRMC_CR117_AXI0_R_PRI(0), 117 },
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
	{ DDRMC_CR126_PHY_RDLAT(8), 126 },
	{ DDRMC_CR132_WRLAT_ADJ(5) |
		   DDRMC_CR132_RDLAT_ADJ(6), 132 },
	{ DDRMC_CR137_PHYCTL_DL(2), 137 },
	{ DDRMC_CR138_PHY_WRLV_MXDL(256) |
		   DDRMC_CR138_PHYDRAM_CK_EN(1), 138 },
	{ DDRMC_CR139_PHY_WRLV_RESPLAT(4) | DDRMC_CR139_PHY_WRLV_LOAD(7) |
		   DDRMC_CR139_PHY_WRLV_DLL(3) |
		   DDRMC_CR139_PHY_WRLV_EN(3), 139 },
	{ DDRMC_CR140_PHY_WRLV_WW(64), 140 },
	{ DDRMC_CR143_RDLV_GAT_MXDL(1536) |
		   DDRMC_CR143_RDLV_MXDL(128), 143 },
	{ DDRMC_CR144_PHY_RDLVL_RES(4) | DDRMC_CR144_PHY_RDLV_LOAD(7) |
		   DDRMC_CR144_PHY_RDLV_DLL(3) |
		   DDRMC_CR144_PHY_RDLV_EN(3), 144 },
	{ DDRMC_CR145_PHY_RDLV_RR(64), 145 },
	{ DDRMC_CR146_PHY_RDLVL_RESP(64), 146 },
	{ DDRMC_CR147_RDLV_RESP_MASK(983040), 147 },
	{ DDRMC_CR148_RDLV_GATE_RESP_MASK(983040), 148 },
	{ DDRMC_CR151_RDLV_GAT_DQ_ZERO_CNT(1) |
		   DDRMC_CR151_RDLVL_DQ_ZERO_CNT(1), 151 },

	{ DDRMC_CR154_PAD_ZQ_EARLY_CMP_EN_TIMER(13) |
		   DDRMC_CR154_PAD_ZQ_MODE(1) |
		   DDRMC_CR154_DDR_SEL_PAD_CONTR(3) |
		   DDRMC_CR154_PAD_ZQ_HW_FOR(1), 154 },
	{ DDRMC_CR155_PAD_ODT_BYTE1(1) | DDRMC_CR155_PAD_ODT_BYTE0(1), 155 },
	{ DDRMC_CR158_TWR(6), 158 },
	{ DDRMC_CR161_ODT_EN(1) | DDRMC_CR161_TODTH_RD(2) |
		   DDRMC_CR161_TODTH_WR(2), 161 },
	/* end marker */
	{ 0, -1 }
};

int dram_init(void)
{
	static const struct ddr3_jedec_timings timings = {
		.tinit             = 5,
		.trst_pwron        = 80000,
		.cke_inactive      = 200000,
		.wrlat             = 5,
		.caslat_lin        = 12,
		.trc               = 21,
		.trrd              = 4,
		.tccd              = 4,
		.tbst_int_interval = 0,
		.tfaw              = 20,
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
		.tras_lockout      = 0,
		.tdal              = 12,
		.bstlen            = 3,
		.tdll              = 512,
		.trp_ab            = 6,
		.tref              = 3120,
		.trfc              = 44,
		.tref_int          = 0,
		.tpdex             = 3,
		.txpdll            = 10,
		.txsnr             = 48,
		.txsr              = 468,
		.cksrx             = 5,
		.cksre             = 5,
		.freq_chg_en       = 0,
		.zqcl              = 256,
		.zqinit            = 512,
		.zqcs              = 64,
		.ref_per_zq        = 64,
		.zqcs_rotate       = 0,
		.aprebit           = 10,
		.cmd_age_cnt       = 64,
		.age_cnt           = 64,
		.q_fullness        = 7,
		.odt_rd_mapcs0     = 0,
		.odt_wr_mapcs0     = 1,
		.wlmrd             = 40,
		.wldqsen           = 25,
	};

	ddrmc_setup_iomux(NULL, 0);

	ddrmc_ctrl_init_ddr3(&timings, vf610twr_cr_settings, NULL, 1, 3);
	gd->ram_size = get_ram_size((void *)PHYS_SDRAM, PHYS_SDRAM_SIZE);

	return 0;
}

static void setup_iomux_uart(void)
{
	static const iomux_v3_cfg_t uart1_pads[] = {
		NEW_PAD_CTRL(VF610_PAD_PTB4__UART1_TX, UART_PAD_CTRL),
		NEW_PAD_CTRL(VF610_PAD_PTB5__UART1_RX, UART_PAD_CTRL),
	};

	imx_iomux_v3_setup_multiple_pads(uart1_pads, ARRAY_SIZE(uart1_pads));
}

static void setup_iomux_enet(void)
{
	static const iomux_v3_cfg_t enet0_pads[] = {
		NEW_PAD_CTRL(VF610_PAD_PTA6__RMII0_CLKIN, ENET_PAD_CTRL),
		NEW_PAD_CTRL(VF610_PAD_PTC1__RMII0_MDIO, ENET_PAD_CTRL),
		NEW_PAD_CTRL(VF610_PAD_PTC0__RMII0_MDC, ENET_PAD_CTRL),
		NEW_PAD_CTRL(VF610_PAD_PTC2__RMII0_CRS_DV, ENET_PAD_CTRL),
		NEW_PAD_CTRL(VF610_PAD_PTC3__RMII0_RD1, ENET_PAD_CTRL),
		NEW_PAD_CTRL(VF610_PAD_PTC4__RMII0_RD0, ENET_PAD_CTRL),
		NEW_PAD_CTRL(VF610_PAD_PTC5__RMII0_RXER, ENET_PAD_CTRL),
		NEW_PAD_CTRL(VF610_PAD_PTC6__RMII0_TD1, ENET_PAD_CTRL),
		NEW_PAD_CTRL(VF610_PAD_PTC7__RMII0_TD0, ENET_PAD_CTRL),
		NEW_PAD_CTRL(VF610_PAD_PTC8__RMII0_TXEN, ENET_PAD_CTRL),
	};

	imx_iomux_v3_setup_multiple_pads(enet0_pads, ARRAY_SIZE(enet0_pads));
}

static void setup_iomux_i2c(void)
{
	static const iomux_v3_cfg_t i2c0_pads[] = {
		VF610_PAD_PTB14__I2C0_SCL,
		VF610_PAD_PTB15__I2C0_SDA,
	};

	imx_iomux_v3_setup_multiple_pads(i2c0_pads, ARRAY_SIZE(i2c0_pads));
}

#ifdef CONFIG_NAND_VF610_NFC
static void setup_iomux_nfc(void)
{
	static const iomux_v3_cfg_t nfc_pads[] = {
		VF610_PAD_PTD31__NF_IO15,
		VF610_PAD_PTD30__NF_IO14,
		VF610_PAD_PTD29__NF_IO13,
		VF610_PAD_PTD28__NF_IO12,
		VF610_PAD_PTD27__NF_IO11,
		VF610_PAD_PTD26__NF_IO10,
		VF610_PAD_PTD25__NF_IO9,
		VF610_PAD_PTD24__NF_IO8,
		VF610_PAD_PTD23__NF_IO7,
		VF610_PAD_PTD22__NF_IO6,
		VF610_PAD_PTD21__NF_IO5,
		VF610_PAD_PTD20__NF_IO4,
		VF610_PAD_PTD19__NF_IO3,
		VF610_PAD_PTD18__NF_IO2,
		VF610_PAD_PTD17__NF_IO1,
		VF610_PAD_PTD16__NF_IO0,
		VF610_PAD_PTB24__NF_WE_B,
		VF610_PAD_PTB25__NF_CE0_B,
		VF610_PAD_PTB27__NF_RE_B,
		VF610_PAD_PTC26__NF_RB_B,
		VF610_PAD_PTC27__NF_ALE,
		VF610_PAD_PTC28__NF_CLE
	};

	imx_iomux_v3_setup_multiple_pads(nfc_pads, ARRAY_SIZE(nfc_pads));
}
#endif


static void setup_iomux_qspi(void)
{
	static const iomux_v3_cfg_t qspi0_pads[] = {
		VF610_PAD_PTD0__QSPI0_A_QSCK,
		VF610_PAD_PTD1__QSPI0_A_CS0,
		VF610_PAD_PTD2__QSPI0_A_DATA3,
		VF610_PAD_PTD3__QSPI0_A_DATA2,
		VF610_PAD_PTD4__QSPI0_A_DATA1,
		VF610_PAD_PTD5__QSPI0_A_DATA0,
		VF610_PAD_PTD7__QSPI0_B_QSCK,
		VF610_PAD_PTD8__QSPI0_B_CS0,
		VF610_PAD_PTD9__QSPI0_B_DATA3,
		VF610_PAD_PTD10__QSPI0_B_DATA2,
		VF610_PAD_PTD11__QSPI0_B_DATA1,
		VF610_PAD_PTD12__QSPI0_B_DATA0,
	};

	imx_iomux_v3_setup_multiple_pads(qspi0_pads, ARRAY_SIZE(qspi0_pads));
}

#ifdef CONFIG_FSL_ESDHC
struct fsl_esdhc_cfg esdhc_cfg[1] = {
	{ESDHC1_BASE_ADDR},
};

int board_mmc_getcd(struct mmc *mmc)
{
	/* eSDHC1 is always present */
	return 1;
}

int board_mmc_init(bd_t *bis)
{
	static const iomux_v3_cfg_t esdhc1_pads[] = {
		NEW_PAD_CTRL(VF610_PAD_PTA24__ESDHC1_CLK, ESDHC_PAD_CTRL),
		NEW_PAD_CTRL(VF610_PAD_PTA25__ESDHC1_CMD, ESDHC_PAD_CTRL),
		NEW_PAD_CTRL(VF610_PAD_PTA26__ESDHC1_DAT0, ESDHC_PAD_CTRL),
		NEW_PAD_CTRL(VF610_PAD_PTA27__ESDHC1_DAT1, ESDHC_PAD_CTRL),
		NEW_PAD_CTRL(VF610_PAD_PTA28__ESDHC1_DAT2, ESDHC_PAD_CTRL),
		NEW_PAD_CTRL(VF610_PAD_PTA29__ESDHC1_DAT3, ESDHC_PAD_CTRL),
	};

	esdhc_cfg[0].sdhc_clk = mxc_get_clock(MXC_ESDHC_CLK);

	imx_iomux_v3_setup_multiple_pads(
		esdhc1_pads, ARRAY_SIZE(esdhc1_pads));

	return fsl_esdhc_initialize(bis, &esdhc_cfg[0]);
}
#endif

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
		CCM_CCGR4_GPC_CTRL_MASK | CCM_CCGR4_I2C0_CTRL_MASK);
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
		CCM_CSCMR1_ESDHC1_CLK_SEL(3) | CCM_CSCMR1_QSPI0_CLK_SEL(3) |
		CCM_CSCMR1_NFC_CLK_SEL(0));
	clrsetbits_le32(&ccm->cscdr1, CCM_REG_CTRL_MASK,
		CCM_CSCDR1_RMII_CLK_EN);
	clrsetbits_le32(&ccm->cscdr2, CCM_REG_CTRL_MASK,
		CCM_CSCDR2_ESDHC1_EN | CCM_CSCDR2_ESDHC1_CLK_DIV(0) |
		CCM_CSCDR2_NFC_EN);
	clrsetbits_le32(&ccm->cscdr3, CCM_REG_CTRL_MASK,
		CCM_CSCDR3_QSPI0_EN | CCM_CSCDR3_QSPI0_DIV(1) |
		CCM_CSCDR3_QSPI0_X2_DIV(1) | CCM_CSCDR3_QSPI0_X4_DIV(3) |
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

int board_phy_config(struct phy_device *phydev)
{
	if (phydev->drv->config)
		phydev->drv->config(phydev);

	return 0;
}

int board_early_init_f(void)
{
	clock_init();
	mscm_init();

	setup_iomux_uart();
	setup_iomux_enet();
	setup_iomux_i2c();
	setup_iomux_qspi();
#ifdef CONFIG_NAND_VF610_NFC
	setup_iomux_nfc();
#endif

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

int checkboard(void)
{
	puts("Board: vf610twr\n");

	return 0;
}
