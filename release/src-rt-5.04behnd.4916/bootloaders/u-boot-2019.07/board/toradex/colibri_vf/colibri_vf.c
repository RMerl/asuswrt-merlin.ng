// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2015-2019 Toradex, Inc.
 *
 * Based on vf610twr.c:
 * Copyright 2013 Freescale Semiconductor, Inc.
 */

#include <common.h>

#include <asm/arch/clock.h>
#include <asm/arch/crm_regs.h>
#include <asm/arch/ddrmc-vf610.h>
#include <asm/arch/imx-regs.h>
#include <asm/arch/iomux-vf610.h>
#include <asm/gpio.h>
#include <asm/io.h>
#include <fdt_support.h>
#include <fsl_dcu_fb.h>
#include <g_dnl.h>
#include <jffs2/load_kernel.h>
#include <mtd_node.h>
#include <usb.h>

#include "../common/tdx-common.h"

DECLARE_GLOBAL_DATA_PTR;

#define PTC0_GPIO_45		45

static struct ddrmc_cr_setting colibri_vf_cr_settings[] = {
	{ DDRMC_CR79_CTLUPD_AREF(1), 79 },
	/* sets manual values for read lvl. (gate) delay of data slice 0/1 */
	{ DDRMC_CR105_RDLVL_DL_0(28), 105 },
	{ DDRMC_CR106_RDLVL_GTDL_0(24), 106 },
	{ DDRMC_CR110_RDLVL_DL_1(28) | DDRMC_CR110_RDLVL_GTDL_1(24), 110 },
	{ DDRMC_CR102_RDLVL_GT_REGEN | DDRMC_CR102_RDLVL_REG_EN, 102 },

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
	{ DDRMC_CR155_PAD_ODT_BYTE1(2) | DDRMC_CR155_PAD_ODT_BYTE0(2), 155 },
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
		.tdll              = 512, /* not applicable since freq. scaling
					   * is not used
					   */
		.trp_ab            = 6,
		.tref              = 3120,
		.trfc              = 64,
		.tref_int          = 0,
		.tpdex             = 3,
		.txpdll            = 10,
		.txsnr             = 68,  /* changed to conform to JEDEC
					   * specifications
					   */
		.txsr              = 506, /* changed to conform to JEDEC
					   * specifications
					   */
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

	ddrmc_ctrl_init_ddr3(&timings, colibri_vf_cr_settings, NULL, 1, 2);
	gd->ram_size = get_ram_size((void *)PHYS_SDRAM, PHYS_SDRAM_SIZE);

	return 0;
}

#ifdef CONFIG_VYBRID_GPIO
static void setup_iomux_gpio(void)
{
	static const iomux_v3_cfg_t gpio_pads[] = {
		VF610_PAD_PTA17__GPIO_7,
		VF610_PAD_PTA20__GPIO_10,
		VF610_PAD_PTA21__GPIO_11,
		VF610_PAD_PTA30__GPIO_20,
		VF610_PAD_PTA31__GPIO_21,
		VF610_PAD_PTB0__GPIO_22,
		VF610_PAD_PTB1__GPIO_23,
		VF610_PAD_PTB6__GPIO_28,
		VF610_PAD_PTB7__GPIO_29,
		VF610_PAD_PTB8__GPIO_30,
		VF610_PAD_PTB9__GPIO_31,
		VF610_PAD_PTB12__GPIO_34,
		VF610_PAD_PTB13__GPIO_35,
		VF610_PAD_PTB16__GPIO_38,
		VF610_PAD_PTB17__GPIO_39,
		VF610_PAD_PTB18__GPIO_40,
		VF610_PAD_PTB21__GPIO_43,
		VF610_PAD_PTB22__GPIO_44,
		VF610_PAD_PTC0__GPIO_45,
		VF610_PAD_PTC1__GPIO_46,
		VF610_PAD_PTC2__GPIO_47,
		VF610_PAD_PTC3__GPIO_48,
		VF610_PAD_PTC4__GPIO_49,
		VF610_PAD_PTC5__GPIO_50,
		VF610_PAD_PTC6__GPIO_51,
		VF610_PAD_PTC7__GPIO_52,
		VF610_PAD_PTC8__GPIO_53,
		VF610_PAD_PTD31__GPIO_63,
		VF610_PAD_PTD30__GPIO_64,
		VF610_PAD_PTD29__GPIO_65,
		VF610_PAD_PTD28__GPIO_66,
		VF610_PAD_PTD27__GPIO_67,
		VF610_PAD_PTD26__GPIO_68,
		VF610_PAD_PTD25__GPIO_69,
		VF610_PAD_PTD24__GPIO_70,
		VF610_PAD_PTD9__GPIO_88,
		VF610_PAD_PTD10__GPIO_89,
		VF610_PAD_PTD11__GPIO_90,
		VF610_PAD_PTD12__GPIO_91,
		VF610_PAD_PTD13__GPIO_92,
		VF610_PAD_PTB23__GPIO_93,
		VF610_PAD_PTB26__GPIO_96,
		VF610_PAD_PTB28__GPIO_98,
		VF610_PAD_PTC30__GPIO_103,
		VF610_PAD_PTA7__GPIO_134,
	};

	imx_iomux_v3_setup_multiple_pads(gpio_pads, ARRAY_SIZE(gpio_pads));
}
#endif

#ifdef CONFIG_VIDEO_FSL_DCU_FB
static void setup_iomux_fsl_dcu(void)
{
	static const iomux_v3_cfg_t dcu0_pads[] = {
		VF610_PAD_PTE0__DCU0_HSYNC,
		VF610_PAD_PTE1__DCU0_VSYNC,
		VF610_PAD_PTE2__DCU0_PCLK,
		VF610_PAD_PTE4__DCU0_DE,
		VF610_PAD_PTE5__DCU0_R0,
		VF610_PAD_PTE6__DCU0_R1,
		VF610_PAD_PTE7__DCU0_R2,
		VF610_PAD_PTE8__DCU0_R3,
		VF610_PAD_PTE9__DCU0_R4,
		VF610_PAD_PTE10__DCU0_R5,
		VF610_PAD_PTE11__DCU0_R6,
		VF610_PAD_PTE12__DCU0_R7,
		VF610_PAD_PTE13__DCU0_G0,
		VF610_PAD_PTE14__DCU0_G1,
		VF610_PAD_PTE15__DCU0_G2,
		VF610_PAD_PTE16__DCU0_G3,
		VF610_PAD_PTE17__DCU0_G4,
		VF610_PAD_PTE18__DCU0_G5,
		VF610_PAD_PTE19__DCU0_G6,
		VF610_PAD_PTE20__DCU0_G7,
		VF610_PAD_PTE21__DCU0_B0,
		VF610_PAD_PTE22__DCU0_B1,
		VF610_PAD_PTE23__DCU0_B2,
		VF610_PAD_PTE24__DCU0_B3,
		VF610_PAD_PTE25__DCU0_B4,
		VF610_PAD_PTE26__DCU0_B5,
		VF610_PAD_PTE27__DCU0_B6,
		VF610_PAD_PTE28__DCU0_B7,
	};

	imx_iomux_v3_setup_multiple_pads(dcu0_pads, ARRAY_SIZE(dcu0_pads));
}

static void setup_tcon(void)
{
	setbits_le32(TCON0_BASE_ADDR, (1 << 29));
}
#endif

static inline int is_colibri_vf61(void)
{
	struct mscm *mscm = (struct mscm *)MSCM_BASE_ADDR;

	/*
	 * Detect board type by Level 2 Cache: VF50 don't have any
	 * Level 2 Cache.
	 */
	return !!mscm->cpxcfg1;
}

static void clock_init(void)
{
	struct ccm_reg *ccm = (struct ccm_reg *)CCM_BASE_ADDR;
	struct anadig_reg *anadig = (struct anadig_reg *)ANADIG_BASE_ADDR;
	u32 pfd_clk_sel, ddr_clk_sel;

	clrsetbits_le32(&ccm->ccgr0, CCM_REG_CTRL_MASK,
			CCM_CCGR0_UART0_CTRL_MASK);
#ifdef CONFIG_FSL_DSPI
	setbits_le32(&ccm->ccgr0, CCM_CCGR0_DSPI1_CTRL_MASK);
#endif
	clrsetbits_le32(&ccm->ccgr1, CCM_REG_CTRL_MASK,
			CCM_CCGR1_PIT_CTRL_MASK | CCM_CCGR1_WDOGA5_CTRL_MASK);
	clrsetbits_le32(&ccm->ccgr2, CCM_REG_CTRL_MASK,
			CCM_CCGR2_IOMUXC_CTRL_MASK | CCM_CCGR2_PORTA_CTRL_MASK |
			CCM_CCGR2_PORTB_CTRL_MASK | CCM_CCGR2_PORTC_CTRL_MASK |
			CCM_CCGR2_PORTD_CTRL_MASK | CCM_CCGR2_PORTE_CTRL_MASK);
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

#ifdef CONFIG_USB_EHCI_VF
	setbits_le32(&ccm->ccgr1, CCM_CCGR1_USBC0_CTRL_MASK);
	setbits_le32(&ccm->ccgr7, CCM_CCGR7_USBC1_CTRL_MASK);

	clrsetbits_le32(&anadig->pll3_ctrl, ANADIG_PLL3_CTRL_BYPASS |
			ANADIG_PLL3_CTRL_POWERDOWN |
			ANADIG_PLL3_CTRL_DIV_SELECT,
			ANADIG_PLL3_CTRL_ENABLE);
	clrsetbits_le32(&anadig->pll7_ctrl, ANADIG_PLL7_CTRL_BYPASS |
			ANADIG_PLL7_CTRL_POWERDOWN |
			ANADIG_PLL7_CTRL_DIV_SELECT,
			ANADIG_PLL7_CTRL_ENABLE);
#endif

	clrsetbits_le32(&anadig->pll5_ctrl, ANADIG_PLL5_CTRL_BYPASS |
			ANADIG_PLL5_CTRL_POWERDOWN, ANADIG_PLL5_CTRL_ENABLE |
			ANADIG_PLL5_CTRL_DIV_SELECT);

	if (is_colibri_vf61()) {
		clrsetbits_le32(&anadig->pll2_ctrl, ANADIG_PLL5_CTRL_BYPASS |
				ANADIG_PLL2_CTRL_POWERDOWN,
				ANADIG_PLL2_CTRL_ENABLE |
				ANADIG_PLL2_CTRL_DIV_SELECT);
	}

	clrsetbits_le32(&anadig->pll1_ctrl, ANADIG_PLL1_CTRL_POWERDOWN,
			ANADIG_PLL1_CTRL_ENABLE | ANADIG_PLL1_CTRL_DIV_SELECT);

	clrsetbits_le32(&ccm->ccr, CCM_CCR_OSCNT_MASK,
			CCM_CCR_FIRC_EN | CCM_CCR_OSCNT(5));

	/* See "Typical PLL Configuration" */
	if (is_colibri_vf61()) {
		pfd_clk_sel = CCM_CCSR_PLL1_PFD_CLK_SEL(1);
		ddr_clk_sel = CCM_CCSR_DDRC_CLK_SEL(0);
	} else {
		pfd_clk_sel = CCM_CCSR_PLL1_PFD_CLK_SEL(3);
		ddr_clk_sel = CCM_CCSR_DDRC_CLK_SEL(1);
	}

	clrsetbits_le32(&ccm->ccsr, CCM_REG_CTRL_MASK, pfd_clk_sel |
			CCM_CCSR_PLL2_PFD4_EN | CCM_CCSR_PLL2_PFD3_EN |
			CCM_CCSR_PLL2_PFD2_EN | CCM_CCSR_PLL2_PFD1_EN |
			CCM_CCSR_PLL1_PFD4_EN | CCM_CCSR_PLL1_PFD3_EN |
			CCM_CCSR_PLL1_PFD2_EN | CCM_CCSR_PLL1_PFD1_EN |
			ddr_clk_sel | CCM_CCSR_FAST_CLK_SEL(1) |
			CCM_CCSR_SYS_CLK_SEL(4));

	clrsetbits_le32(&ccm->cacrr, CCM_REG_CTRL_MASK,
			CCM_CACRR_IPG_CLK_DIV(1) | CCM_CACRR_BUS_CLK_DIV(2) |
			CCM_CACRR_ARM_CLK_DIV(0));
	clrsetbits_le32(&ccm->cscmr1, CCM_REG_CTRL_MASK,
			CCM_CSCMR1_ESDHC1_CLK_SEL(3) |
			CCM_CSCMR1_NFC_CLK_SEL(0));
	clrsetbits_le32(&ccm->cscdr1, CCM_REG_CTRL_MASK,
			CCM_CSCDR1_RMII_CLK_EN);
	clrsetbits_le32(&ccm->cscdr2, CCM_REG_CTRL_MASK,
			CCM_CSCDR2_ESDHC1_EN | CCM_CSCDR2_ESDHC1_CLK_DIV(0) |
			CCM_CSCDR2_NFC_EN);
	clrsetbits_le32(&ccm->cscdr3, CCM_REG_CTRL_MASK,
			CCM_CSCDR3_NFC_PRE_DIV(3));
	clrsetbits_le32(&ccm->cscmr2, CCM_REG_CTRL_MASK,
			CCM_CSCMR2_RMII_CLK_SEL(2));

#ifdef CONFIG_VIDEO_FSL_DCU_FB
		setbits_le32(&ccm->ccgr1, CCM_CCGR1_TCON0_CTRL_MASK);
		setbits_le32(&ccm->ccgr3, CCM_CCGR3_DCU0_CTRL_MASK);
#endif
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

#ifdef CONFIG_VYBRID_GPIO
	setup_iomux_gpio();
#endif

#ifdef CONFIG_VIDEO_FSL_DCU_FB
	setup_tcon();
	setup_iomux_fsl_dcu();
#endif

	return 0;
}

#ifdef CONFIG_BOARD_LATE_INIT
int board_late_init(void)
{
	struct src *src = (struct src *)SRC_BASE_ADDR;

	if (((src->sbmr2 & SRC_SBMR2_BMOD_MASK) >> SRC_SBMR2_BMOD_SHIFT)
			== SRC_SBMR2_BMOD_SERIAL) {
		printf("Serial Downloader recovery mode, disable autoboot\n");
		env_set("bootdelay", "-1");
	}

	return 0;
}
#endif /* CONFIG_BOARD_LATE_INIT */

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
	if (is_colibri_vf61())
		puts("Model: Toradex Colibri VF61\n");
	else
		puts("Model: Toradex Colibri VF50\n");

	return 0;
}

#if defined(CONFIG_OF_LIBFDT) && defined(CONFIG_OF_BOARD_SETUP)
int ft_board_setup(void *blob, bd_t *bd)
{
	int ret = 0;
#ifdef CONFIG_FDT_FIXUP_PARTITIONS
	static const struct node_info nodes[] = {
		{ "fsl,vf610-nfc", MTD_DEV_TYPE_NAND, }, /* NAND flash */
	};

	/* Update partition nodes using info from mtdparts env var */
	puts("   Updating MTD partitions...\n");
	fdt_fixup_mtdparts(blob, nodes, ARRAY_SIZE(nodes));
#endif
#ifdef CONFIG_VIDEO_FSL_DCU_FB
	ret = fsl_dcu_fixedfb_setup(blob);
	if (ret)
		return ret;
#endif

	return ft_common_board_setup(blob, bd);
}
#endif

/*
 * Backlight off before OS handover
 */
void board_preboot_os(void)
{
	gpio_request(PTC0_GPIO_45, "BL_ON");
	gpio_direction_output(PTC0_GPIO_45, 0);
}
