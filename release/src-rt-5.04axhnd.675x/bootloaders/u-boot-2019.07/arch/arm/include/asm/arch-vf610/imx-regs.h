/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2013-2014 Freescale Semiconductor, Inc.
 */

#ifndef __ASM_ARCH_IMX_REGS_H__
#define __ASM_ARCH_IMX_REGS_H__

#define ARCH_MXC

#define IRAM_BASE_ADDR		0x3F000000	/* internal ram */
#define IRAM_SIZE		0x00080000	/* 512 KB */

#define AIPS0_BASE_ADDR		0x40000000
#define AIPS1_BASE_ADDR		0x40080000

/* AIPS 0 */
#define MSCM_BASE_ADDR		(AIPS0_BASE_ADDR + 0x00001000)
#define MSCM_IR_BASE_ADDR	(AIPS0_BASE_ADDR + 0x00001800)
#define CA5SCU_BASE_ADDR	(AIPS0_BASE_ADDR + 0x00002000)
#define CA5_INTD_BASE_ADDR	(AIPS0_BASE_ADDR + 0x00003000)
#define CA5_L2C_BASE_ADDR	(AIPS0_BASE_ADDR + 0x00006000)
#define NIC0_BASE_ADDR		(AIPS0_BASE_ADDR + 0x00008000)
#define NIC1_BASE_ADDR		(AIPS0_BASE_ADDR + 0x00009000)
#define NIC2_BASE_ADDR		(AIPS0_BASE_ADDR + 0x0000A000)
#define NIC3_BASE_ADDR		(AIPS0_BASE_ADDR + 0x0000B000)
#define NIC4_BASE_ADDR		(AIPS0_BASE_ADDR + 0x0000C000)
#define NIC5_BASE_ADDR		(AIPS0_BASE_ADDR + 0x0000D000)
#define NIC6_BASE_ADDR		(AIPS0_BASE_ADDR + 0x0000E000)
#define NIC7_BASE_ADDR		(AIPS0_BASE_ADDR + 0x0000F000)
#define AHBTZASC_BASE_ADDR	(AIPS0_BASE_ADDR + 0x00010000)
#define TZASC_SYS0_BASE_ADDR	(AIPS0_BASE_ADDR + 0x00011000)
#define TZASC_SYS1_BASE_ADDR	(AIPS0_BASE_ADDR + 0x00012000)
#define TZASC_GFX_BASE_ADDR	(AIPS0_BASE_ADDR + 0x00013000)
#define TZASC_DDR0_BASE_ADDR	(AIPS0_BASE_ADDR + 0x00014000)
#define TZASC_DDR1_BASE_ADDR	(AIPS0_BASE_ADDR + 0x00015000)
#define CSU_BASE_ADDR		(AIPS0_BASE_ADDR + 0x00017000)
#define DMA0_BASE_ADDR		(AIPS0_BASE_ADDR + 0x00018000)
#define DMA0_TCD_BASE_ADDR	(AIPS0_BASE_ADDR + 0x00019000)
#define SEMA4_BASE_ADDR		(AIPS0_BASE_ADDR + 0x0001D000)
#define FB_BASE_ADDR		(AIPS0_BASE_ADDR + 0x0001E000)
#define DMA_MUX0_BASE_ADDR	(AIPS0_BASE_ADDR + 0x00024000)
#define UART0_BASE		(AIPS0_BASE_ADDR + 0x00027000)
#define UART1_BASE		(AIPS0_BASE_ADDR + 0x00028000)
#define UART2_BASE		(AIPS0_BASE_ADDR + 0x00029000)
#define UART3_BASE		(AIPS0_BASE_ADDR + 0x0002A000)
#define SPI0_BASE_ADDR		(AIPS0_BASE_ADDR + 0x0002C000)
#define SPI1_BASE_ADDR		(AIPS0_BASE_ADDR + 0x0002D000)
#define SAI0_BASE_ADDR		(AIPS0_BASE_ADDR + 0x0002F000)
#define SAI1_BASE_ADDR		(AIPS0_BASE_ADDR + 0x00030000)
#define SAI2_BASE_ADDR		(AIPS0_BASE_ADDR + 0x00031000)
#define SAI3_BASE_ADDR		(AIPS0_BASE_ADDR + 0x00032000)
#define CRC_BASE_ADDR		(AIPS0_BASE_ADDR + 0x00033000)
#define USBC0_BASE_ADDR     (AIPS0_BASE_ADDR + 0x00034000)
#define PDB_BASE_ADDR		(AIPS0_BASE_ADDR + 0x00036000)
#define PIT_BASE_ADDR		(AIPS0_BASE_ADDR + 0x00037000)
#define FTM0_BASE_ADDR		(AIPS0_BASE_ADDR + 0x00038000)
#define FTM1_BASE_ADDR		(AIPS0_BASE_ADDR + 0x00039000)
#define ADC_BASE_ADDR		(AIPS0_BASE_ADDR + 0x0003B000)
#define TCON0_BASE_ADDR		(AIPS0_BASE_ADDR + 0x0003D000)
#define WDOG1_BASE_ADDR		(AIPS0_BASE_ADDR + 0x0003E000)
#define LPTMR_BASE_ADDR		(AIPS0_BASE_ADDR + 0x00040000)
#define RLE_BASE_ADDR		(AIPS0_BASE_ADDR + 0x00042000)
#define MLB_BASE_ADDR		(AIPS0_BASE_ADDR + 0x00043000)
#define QSPI0_BASE_ADDR		(AIPS0_BASE_ADDR + 0x00044000)
#define IOMUXC_BASE_ADDR	(AIPS0_BASE_ADDR + 0x00048000)
#define ANADIG_BASE_ADDR	(AIPS0_BASE_ADDR + 0x00050000)
#define USB_PHY0_BASE_ADDR  (AIPS0_BASE_ADDR + 0x00050800)
#define USB_PHY1_BASE_ADDR  (AIPS0_BASE_ADDR + 0x00050C00)
#define SCSC_BASE_ADDR		(AIPS0_BASE_ADDR + 0x00052000)
#define DCU0_BASE_ADDR		(AIPS0_BASE_ADDR + 0x00058000)
#define ASRC_BASE_ADDR		(AIPS0_BASE_ADDR + 0x00060000)
#define SPDIF_BASE_ADDR		(AIPS0_BASE_ADDR + 0x00061000)
#define ESAI_BASE_ADDR		(AIPS0_BASE_ADDR + 0x00062000)
#define ESAI_FIFO_BASE_ADDR	(AIPS0_BASE_ADDR + 0x00063000)
#define WDOG_BASE_ADDR		(AIPS0_BASE_ADDR + 0x00065000)
#define I2C1_BASE_ADDR		(AIPS0_BASE_ADDR + 0x00066000)
#define I2C2_BASE_ADDR		(AIPS0_BASE_ADDR + 0x00067000)
#define I2C3_BASE_ADDR		(AIPS0_BASE_ADDR + 0x000E6000)
#define I2C4_BASE_ADDR		(AIPS0_BASE_ADDR + 0x000E7000)
#define WKUP_BASE_ADDR		(AIPS0_BASE_ADDR + 0x0006A000)
#define CCM_BASE_ADDR		(AIPS0_BASE_ADDR + 0x0006B000)
#define GPC_BASE_ADDR		(AIPS0_BASE_ADDR + 0x0006C000)
#define VREG_DIG_BASE_ADDR	(AIPS0_BASE_ADDR + 0x0006D000)
#define SRC_BASE_ADDR		(AIPS0_BASE_ADDR + 0x0006E000)
#define CMU_BASE_ADDR		(AIPS0_BASE_ADDR + 0x0006F000)
#define GPIO0_BASE_ADDR		(AIPS0_BASE_ADDR + 0x000FF000)
#define GPIO1_BASE_ADDR		(AIPS0_BASE_ADDR + 0x000FF040)
#define GPIO2_BASE_ADDR		(AIPS0_BASE_ADDR + 0x000FF080)
#define GPIO3_BASE_ADDR		(AIPS0_BASE_ADDR + 0x000FF0C0)
#define GPIO4_BASE_ADDR		(AIPS0_BASE_ADDR + 0x000FF100)

/* AIPS 1 */
#define OCOTP_BASE_ADDR		(AIPS1_BASE_ADDR + 0x00025000)
#define DDR_BASE_ADDR		(AIPS1_BASE_ADDR + 0x0002E000)
#define ESDHC0_BASE_ADDR	(AIPS1_BASE_ADDR + 0x00031000)
#define ESDHC1_BASE_ADDR	(AIPS1_BASE_ADDR + 0x00032000)
#define USBC1_BASE_ADDR     (AIPS1_BASE_ADDR + 0x00034000)
#define ENET_BASE_ADDR		(AIPS1_BASE_ADDR + 0x00050000)
#define ENET1_BASE_ADDR		(AIPS1_BASE_ADDR + 0x00051000)
#define DCU1_BASE_ADDR		(AIPS1_BASE_ADDR + 0x00058000)
#define NFC_BASE_ADDR		(AIPS1_BASE_ADDR + 0x00060000)

#define QSPI0_AMBA_BASE		0x20000000

/* MUX mode and PAD ctrl are in one register */
#define CONFIG_IOMUX_SHARE_CONF_REG

#define FEC_QUIRK_ENET_MAC
#define I2C_QUIRK_REG

/* MSCM interrupt rounter */
#define MSCM_IRSPRC_CP0_EN				1
#define MSCM_IRSPRC_NUM					112

/* DDRMC */
#define DDRMC_PHY_DQ_TIMING				0x00002613
#define DDRMC_PHY_DQS_TIMING				0x00002615
#define DDRMC_PHY_CTRL					0x00210000
#define DDRMC_PHY_MASTER_CTRL				0x0001012a
#define DDRMC_PHY_SLAVE_CTRL				0x00002000
#define DDRMC_PHY_OFF					0x00000000
#define DDRMC_PHY_PROC_PAD_ODT				0x00010101

#define DDRMC_PHY50_DDR3_MODE				(1 << 12)
#define DDRMC_PHY50_EN_SW_HALF_CYCLE			(1 << 8)

#define DDRMC_CR00_DRAM_CLASS_DDR3			(0x6 << 8)
#define DDRMC_CR00_DRAM_CLASS_LPDDR2			(0x5 << 8)
#define DDRMC_CR00_START				1
#define DDRMC_CR02_DRAM_TINIT(v)			((v) & 0xffffff)
#define DDRMC_CR10_TRST_PWRON(v)			(v)
#define DDRMC_CR11_CKE_INACTIVE(v)			(v)
#define DDRMC_CR12_WRLAT(v)				(((v) & 0x1f) << 8)
#define DDRMC_CR12_CASLAT_LIN(v)			((v) & 0x3f)
#define DDRMC_CR13_TRC(v)				(((v) & 0xff) << 24)
#define DDRMC_CR13_TRRD(v)				(((v) & 0xff) << 16)
#define DDRMC_CR13_TCCD(v)				(((v) & 0x1f) << 8)
#define DDRMC_CR13_TBST_INT_INTERVAL(v)			((v) & 0x7)
#define DDRMC_CR14_TFAW(v)				(((v) & 0x3f) << 24)
#define DDRMC_CR14_TRP(v)				(((v) & 0x1f) << 16)
#define DDRMC_CR14_TWTR(v)				(((v) & 0xf) << 8)
#define DDRMC_CR14_TRAS_MIN(v)				((v) & 0xff)
#define DDRMC_CR16_TMRD(v)				(((v) & 0x1f) << 24)
#define DDRMC_CR16_TRTP(v)				(((v) & 0xf) << 16)
#define DDRMC_CR17_TRAS_MAX(v)				(((v) & 0x1ffff) << 8)
#define DDRMC_CR17_TMOD(v)				((v) & 0xff)
#define DDRMC_CR18_TCKESR(v)				(((v) & 0x1f) << 8)
#define DDRMC_CR18_TCKE(v)				((v) & 0x7)
#define DDRMC_CR20_AP_EN				(1 << 24)
#define DDRMC_CR21_TRCD_INT(v)				(((v) & 0xff) << 16)
#define DDRMC_CR21_TRAS_LOCKOUT(v)			((v) << 8)
#define DDRMC_CR21_CCMAP_EN				1
#define DDRMC_CR22_TDAL(v)				(((v) & 0x3f) << 16)
#define DDRMC_CR23_BSTLEN(v)				(((v) & 0x7) << 24)
#define DDRMC_CR23_TDLL(v)				((v) & 0xffff)
#define DDRMC_CR24_TRP_AB(v)				((v) & 0x1f)
#define DDRMC_CR25_TREF_EN				(1 << 16)
#define DDRMC_CR26_TREF(v)				(((v) & 0xffff) << 16)
#define DDRMC_CR26_TRFC(v)				((v) & 0x3ff)
#define DDRMC_CR28_TREF_INT(v)				((v) & 0xffff)
#define DDRMC_CR29_TPDEX(v)				((v) & 0xffff)
#define DDRMC_CR30_TXPDLL(v)				((v) & 0xffff)
#define DDRMC_CR31_TXSNR(v)				(((v) & 0xffff) << 16)
#define DDRMC_CR31_TXSR(v)				((v) & 0xffff)
#define DDRMC_CR33_EN_QK_SREF				(1 << 16)
#define DDRMC_CR34_CKSRX(v)				(((v) & 0xf) << 16)
#define DDRMC_CR34_CKSRE(v)				(((v) & 0xf) << 8)
#define DDRMC_CR38_FREQ_CHG_EN(v)			(((v) & 0x1) << 8)
#define DDRMC_CR39_PHY_INI_COM(v)			(((v) & 0xffff) << 16)
#define DDRMC_CR39_PHY_INI_STA(v)			(((v) & 0xff) << 8)
#define DDRMC_CR39_FRQ_CH_DLLOFF(v)			((v) & 0x3)
#define DDRMC_CR41_PHY_INI_STRT_INI_DIS			1
#define DDRMC_CR48_MR1_DA_0(v)				(((v) & 0xffff) << 16)
#define DDRMC_CR48_MR0_DA_0(v)				((v) & 0xffff)
#define DDRMC_CR66_ZQCL(v)				(((v) & 0xfff) << 16)
#define DDRMC_CR66_ZQINIT(v)				((v) & 0xfff)
#define DDRMC_CR67_ZQCS(v)				((v) & 0xfff)
#define DDRMC_CR69_ZQ_ON_SREF_EX(v)			(((v) & 0xf) << 8)
#define DDRMC_CR70_REF_PER_ZQ(v)			(v)
#define DDRMC_CR72_ZQCS_ROTATE(v)			(((v) & 0x1) << 24)
#define DDRMC_CR73_APREBIT(v)				(((v) & 0xf) << 24)
#define DDRMC_CR73_COL_DIFF(v)				(((v) & 0x7) << 16)
#define DDRMC_CR73_ROW_DIFF(v)				(((v) & 0x3) << 8)
#define DDRMC_CR74_BANKSPLT_EN				(1 << 24)
#define DDRMC_CR74_ADDR_CMP_EN				(1 << 16)
#define DDRMC_CR74_CMD_AGE_CNT(v)			(((v) & 0xff) << 8)
#define DDRMC_CR74_AGE_CNT(v)				((v) & 0xff)
#define DDRMC_CR75_RW_PG_EN				(1 << 24)
#define DDRMC_CR75_RW_EN				(1 << 16)
#define DDRMC_CR75_PRI_EN				(1 << 8)
#define DDRMC_CR75_PLEN					1
#define DDRMC_CR76_NQENT_ACTDIS(v)			(((v) & 0x7) << 24)
#define DDRMC_CR76_D_RW_G_BKCN(v)			(((v) & 0x3) << 16)
#define DDRMC_CR76_W2R_SPLT_EN				(1 << 8)
#define DDRMC_CR76_CS_EN				1
#define DDRMC_CR77_CS_MAP				(1 << 24)
#define DDRMC_CR77_DI_RD_INTLEAVE			(1 << 8)
#define DDRMC_CR77_SWAP_EN				1
#define DDRMC_CR78_Q_FULLNESS(v)			(((v) & 0x7) << 24)
#define DDRMC_CR78_BUR_ON_FLY_BIT(v)			((v) & 0xf)
#define DDRMC_CR79_CTLUPD_AREF(v)			(((v) & 0x1) << 24)
#define DDRMC_CR80_MC_INIT_COMPLETE			(1 << 8)
#define DDRMC_CR82_INT_MASK				(1 << 28)
#define DDRMC_CR87_ODT_WR_MAPCS0(v)			((v) << 24)
#define DDRMC_CR87_ODT_RD_MAPCS0(v)			((v) << 16)
#define DDRMC_CR88_TODTL_CMD(v)				(((v) & 0x1f) << 16)
#define DDRMC_CR89_AODT_RWSMCS(v)			((v) & 0xf)
#define DDRMC_CR91_R2W_SMCSDL(v)			(((v) & 0x7) << 16)
#define DDRMC_CR93_SW_LVL_MODE_OFF			(8)
#define DDRMC_CR93_SW_LVL_MODE(v) (((v) & 0x3) << DDRMC_CR93_SW_LVL_MODE_OFF)
#define DDRMC_CR93_SWLVL_LOAD				BIT(16)
#define DDRMC_CR93_SWLVL_START				BIT(24)
#define DDRMC_CR94_SWLVL_EXIT				BIT(0)
#define DDRMC_CR94_SWLVL_OP_DONE			BIT(8)
#define DDRMC_CR94_SWLVL_RESP_0_OFF			(24)
#define DDRMC_CR95_SWLVL_RESP_1_OFF			(0)
#define DDRMC_CR96_WLMRD(v)				(((v) & 0x3f) << 8)
#define DDRMC_CR96_WLDQSEN(v)				((v) & 0x3f)
#define DDRMC_CR97_WRLVL_EN				(1 << 24)
#define DDRMC_CR98_WRLVL_DL_0(v)			((v) & 0xffff)
#define DDRMC_CR99_WRLVL_DL_1(v)			((v) & 0xffff)
#define DDRMC_CR101_PHY_RDLVL_EDGE_OFF			(24)
#define DDRMC_CR101_PHY_RDLVL_EDGE BIT(DDRMC_CR101_PHY_RDLVL_EDGE_OFF)
#define DDRMC_CR102_RDLVL_GT_REGEN			(1 << 16)
#define DDRMC_CR102_RDLVL_REG_EN			(1 << 8)
#define DDRMC_CR105_RDLVL_DL_0_OFF			(8)
#define DDRMC_CR105_RDLVL_DL_0(v) (((v) & 0xff) << DDRMC_CR105_RDLVL_DL_0_OFF)
#define DDRMC_CR106_RDLVL_GTDL_0(v)			((v) & 0xff)
#define DDRMC_CR110_RDLVL_DL_1_OFF			(0)
#define DDRMC_CR110_RDLVL_DL_1(v)			((v) & 0xff)
#define DDRMC_CR110_RDLVL_GTDL_1(v)			(((v) & 0xff) << 16)
#define DDRMC_CR114_RDLVL_GTDL_2(v)			(((v) & 0xffff) << 8)
#define DDRMC_CR115_RDLVL_GTDL_2(v)			((v) & 0xff)
#define DDRMC_CR117_AXI0_W_PRI(v)			(((v) & 0x3) << 8)
#define DDRMC_CR117_AXI0_R_PRI(v)			((v) & 0x3)
#define DDRMC_CR118_AXI1_W_PRI(v)			(((v) & 0x3) << 24)
#define DDRMC_CR118_AXI1_R_PRI(v)			(((v) & 0x3) << 16)
#define DDRMC_CR120_AXI0_PRI1_RPRI(v)			(((v) & 0xf) << 24)
#define DDRMC_CR120_AXI0_PRI0_RPRI(v)			(((v) & 0xf) << 16)
#define DDRMC_CR121_AXI0_PRI3_RPRI(v)			(((v) & 0xf) << 8)
#define DDRMC_CR121_AXI0_PRI2_RPRI(v)			((v) & 0xf)
#define DDRMC_CR122_AXI1_PRI1_RPRI(v)			(((v) & 0xf) << 24)
#define DDRMC_CR122_AXI1_PRI0_RPRI(v)			(((v) & 0xf) << 16)
#define DDRMC_CR122_AXI0_PRIRLX(v)			((v) & 0x3ff)
#define DDRMC_CR123_AXI1_PRI3_RPRI(v)			(((v) & 0xf) << 8)
#define DDRMC_CR123_AXI1_PRI2_RPRI(v)			((v) & 0xf)
#define DDRMC_CR123_AXI1_P_ODR_EN			(1 << 16)
#define DDRMC_CR124_AXI1_PRIRLX(v)			((v) & 0x3ff)
#define DDRMC_CR126_PHY_RDLAT(v)			(((v) & 0x3f) << 8)
#define DDRMC_CR132_WRLAT_ADJ(v)			(((v) & 0x1f) << 8)
#define DDRMC_CR132_RDLAT_ADJ(v)			((v) & 0x3f)
#define DDRMC_CR137_PHYCTL_DL(v)			(((v) & 0xf) << 16)
#define DDRMC_CR138_PHY_WRLV_MXDL(v)			(((v) & 0xffff) << 16)
#define DDRMC_CR138_PHYDRAM_CK_EN(v)			(((v) & 0x7) << 8)
#define DDRMC_CR139_PHY_WRLV_RESPLAT(v)			(((v) & 0xff) << 24)
#define DDRMC_CR139_PHY_WRLV_LOAD(v)			(((v) & 0xff) << 16)
#define DDRMC_CR139_PHY_WRLV_DLL(v)			(((v) & 0xff) << 8)
#define DDRMC_CR139_PHY_WRLV_EN(v)			((v) & 0xff)
#define DDRMC_CR140_PHY_WRLV_WW(v)			((v) & 0x3ff)
#define DDRMC_CR143_RDLV_GAT_MXDL(v)			(((v) & 0xffff) << 16)
#define DDRMC_CR143_RDLV_MXDL(v)			((v) & 0xffff)
#define DDRMC_CR144_PHY_RDLVL_RES(v)			(((v) & 0xff) << 24)
#define DDRMC_CR144_PHY_RDLV_LOAD(v)			(((v) & 0xff) << 16)
#define DDRMC_CR144_PHY_RDLV_DLL(v)			(((v) & 0xff) << 8)
#define DDRMC_CR144_PHY_RDLV_EN(v)			((v) & 0xff)
#define DDRMC_CR145_PHY_RDLV_RR(v)			((v) & 0x3ff)
#define DDRMC_CR146_PHY_RDLVL_RESP(v)			(v)
#define DDRMC_CR147_RDLV_RESP_MASK(v)			((v) & 0xfffff)
#define DDRMC_CR148_RDLV_GATE_RESP_MASK(v)		((v) & 0xfffff)
#define DDRMC_CR151_RDLV_GAT_DQ_ZERO_CNT(v)		(((v) & 0xf) << 8)
#define DDRMC_CR151_RDLVL_DQ_ZERO_CNT(v)		((v) & 0xf)
#define DDRMC_CR154_PAD_ZQ_EARLY_CMP_EN_TIMER(v)	(((v) & 0x1f) << 27)
#define DDRMC_CR154_PAD_ZQ_MODE(v)			(((v) & 0x3) << 21)
#define DDRMC_CR154_DDR_SEL_PAD_CONTR(v)		(((v) & 0x3) << 18)
#define DDRMC_CR154_PAD_ZQ_HW_FOR(v)			(((v) & 0x1) << 14)
#define DDRMC_CR155_AXI0_AWCACHE			(1 << 10)
#define DDRMC_CR155_PAD_ODT_BYTE1(v)			(((v) & 0x7) << 3)
#define DDRMC_CR155_PAD_ODT_BYTE0(v)			((v) & 0x7)
#define DDRMC_CR158_TWR(v)				((v) & 0x3f)
#define DDRMC_CR161_ODT_EN(v)				(((v) & 0x1) << 16)
#define DDRMC_CR161_TODTH_RD(v)				(((v) & 0xf) << 8)
#define DDRMC_CR161_TODTH_WR(v)				((v) & 0xf)

/* System Reset Controller (SRC) */
#define SRC_SRSR_SW_RST					(0x1 << 18)
#define SRC_SRSR_RESETB					(0x1 << 7)
#define SRC_SRSR_JTAG_RST				(0x1 << 5)
#define SRC_SRSR_WDOG_M4				(0x1 << 4)
#define SRC_SRSR_WDOG_A5				(0x1 << 3)
#define SRC_SRSR_POR_RST				(0x1 << 0)
#define SRC_SBMR1_BOOTCFG1_SDMMC        BIT(6)
#define SRC_SBMR1_BOOTCFG1_MMC          BIT(4)
#define SRC_SBMR2_BMOD_MASK             (0x3 << 24)
#define SRC_SBMR2_BMOD_SHIFT            24
#define SRC_SBMR2_BMOD_FUSES            0x0
#define SRC_SBMR2_BMOD_SERIAL           0x1
#define SRC_SBMR2_BMOD_RCON             0x2

/* Slow Clock Source Controller Module (SCSC) */
#define SCSC_SOSC_CTR_SOSC_EN            0x1

#if !(defined(__KERNEL_STRICT_NAMES) || defined(__ASSEMBLY__))
#include <asm/types.h>

/* System Reset Controller (SRC) */
struct src {
	u32 scr;
	u32 sbmr1;
	u32 srsr;
	u32 secr;
	u32 gpsr;
	u32 sicr;
	u32 simr;
	u32 sbmr2;
	u32 gpr0;
	u32 gpr1;
	u32 gpr2;
	u32 gpr3;
	u32 gpr4;
	u32 hab0;
	u32 hab1;
	u32 hab2;
	u32 hab3;
	u32 hab4;
	u32 hab5;
	u32 misc0;
	u32 misc1;
	u32 misc2;
	u32 misc3;
};

/* Periodic Interrupt Timer (PIT) */
struct pit_reg {
	u32 mcr;
	u32 recv0[55];
	u32 ltmr64h;
	u32 ltmr64l;
	u32 recv1[6];
	u32 ldval0;
	u32 cval0;
	u32 tctrl0;
	u32 tflg0;
	u32 ldval1;
	u32 cval1;
	u32 tctrl1;
	u32 tflg1;
	u32 ldval2;
	u32 cval2;
	u32 tctrl2;
	u32 tflg2;
	u32 ldval3;
	u32 cval3;
	u32 tctrl3;
	u32 tflg3;
	u32 ldval4;
	u32 cval4;
	u32 tctrl4;
	u32 tflg4;
	u32 ldval5;
	u32 cval5;
	u32 tctrl5;
	u32 tflg5;
	u32 ldval6;
	u32 cval6;
	u32 tctrl6;
	u32 tflg6;
	u32 ldval7;
	u32 cval7;
	u32 tctrl7;
	u32 tflg7;
};

/* Watchdog Timer (WDOG) */
struct wdog_regs {
	u16 wcr;
	u16 wsr;
	u16 wrsr;
	u16 wicr;
	u16 wmcr;
};

/* LPDDR2/DDR3 SDRAM Memory Controller (DDRMC) */
struct ddrmr_regs {
	u32 cr[162];
	u32 rsvd[94];
	u32 phy[53];
};

/* On-Chip One Time Programmable Controller (OCOTP) */
struct ocotp_regs {
	u32 ctrl;
	u32 ctrl_set;
	u32 ctrl_clr;
	u32 ctrl_tog;
	u32 timing;
	u32 rsvd0[3];
	u32 data;
	u32 rsvd1[3];
	u32 read_ctrl;
	u32 rsvd2[3];
	u32 read_fuse_data;
	u32 rsvd3[7];
	u32 scs;
	u32 scs_set;
	u32 scs_clr;
	u32 scs_tog;
	u32 crc_addr;
	u32 rsvd4[3];
	u32 crc_value;
	u32 rsvd5[3];
	u32 version;
	u32 rsvd6[0xdb];

	struct fuse_bank {
		u32 fuse_regs[0x20];
	} bank[16];
};

struct fuse_bank0_regs {
	u32 lock;
	u32 rsvd0[3];
	u32 uid_low;
	u32 rsvd1[3];
	u32 uid_high;
	u32 rsvd2[0x17];
};

struct fuse_bank4_regs {
	u32 sjc_resp0;
	u32 rsvd0[3];
	u32 sjc_resp1;
	u32 rsvd1[3];
	u32 mac_addr0;
	u32 rsvd2[3];
	u32 mac_addr1;
	u32 rsvd3[3];
	u32 mac_addr2;
	u32 rsvd4[3];
	u32 mac_addr3;
	u32 rsvd5[3];
	u32 gp1;
	u32 rsvd6[3];
	u32 gp2;
	u32 rsvd7[3];
};

/* MSCM Interrupt Router */
struct mscm_ir {
	u32 ircp0ir;
	u32 ircp1ir;
	u32 rsvd1[6];
	u32 ircpgir;
	u32 rsvd2[23];
	u16 irsprc[112];
	u16 rsvd3[848];
};

/* SCSC */
struct scsc_reg {
	u32 sirc_ctr;
	u32 sosc_ctr;
};

/* MSCM */
struct mscm {
	u32 cpxtype;
	u32 cpxnum;
	u32 cpxmaster;
	u32 cpxcount;
	u32 cpxcfg0;
	u32 cpxcfg1;
	u32 cpxcfg2;
	u32 cpxcfg3;
};

#endif	/* __ASSEMBLER__*/

#endif	/* __ASM_ARCH_IMX_REGS_H__ */
