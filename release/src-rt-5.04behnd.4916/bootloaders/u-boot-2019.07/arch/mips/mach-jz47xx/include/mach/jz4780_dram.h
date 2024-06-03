/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * JZ4780 DDR initialization - parameters definitions
 *
 * Copyright (c) 2015 Imagination Technologies
 * Author: Matt Redfearn <matt.redfearn.com>
 */

#ifndef __JZ4780_DRAM_H__
#define __JZ4780_DRAM_H__

/*
 * DDR
 */
#define DDRC_ST				0x0
#define DDRC_CFG			0x4
#define DDRC_CTRL			0x8
#define DDRC_LMR			0xc
#define DDRC_REFCNT			0x18
#define DDRC_DQS			0x1c
#define DDRC_DQS_ADJ			0x20
#define DDRC_MMAP0			0x24
#define DDRC_MMAP1			0x28
#define DDRC_MDELAY			0x2c
#define DDRC_CKEL			0x30
#define DDRC_PMEMCTRL0			0x54
#define DDRC_PMEMCTRL1			0x50
#define DDRC_PMEMCTRL2			0x58
#define DDRC_PMEMCTRL3			0x5c

#define DDRC_TIMING(n)			(0x60 + 4 * (n))
#define DDRC_REMMAP(n)			(0x9c + 4 * (n))

/*
 * DDR PHY
 */
#define DDR_MEM_PHY_BASE		0x20000000
#define DDR_PHY_OFFSET			0x1000

#define DDRP_PIR			0x4
#define DDRP_PGCR			0x8
#define DDRP_PGSR			0xc

#define DDRP_PTR0			0x18
#define DDRP_PTR1			0x1c
#define DDRP_PTR2			0x20

#define DDRP_ACIOCR			0x24
#define DDRP_DXCCR			0x28
#define DDRP_DSGCR			0x2c
#define DDRP_DCR			0x30

#define DDRP_DTPR0			0x34
#define DDRP_DTPR1			0x38
#define DDRP_DTPR2			0x3c
#define DDRP_MR0			0x40
#define DDRP_MR1			0x44
#define DDRP_MR2			0x48
#define DDRP_MR3			0x4c

#define DDRP_ODTCR			0x50
#define DDRP_DTAR			0x54
#define DDRP_DTDR0			0x58
#define DDRP_DTDR1			0x5c

#define DDRP_DCUAR			0xc0
#define DDRP_DCUDR			0xc4
#define DDRP_DCURR			0xc8
#define DDRP_DCULR			0xcc
#define DDRP_DCUGCR			0xd0
#define DDRP_DCUTPR			0xd4
#define DDRP_DCUSR0			0xd8
#define DDRP_DCUSR1			0xdc

#define DDRP_ZQXCR0(n)			(0x180 + ((n) * 0x10))
#define DDRP_ZQXCR1(n)			(0x184 + ((n) * 0x10))
#define DDRP_ZQXSR0(n)			(0x188 + ((n) * 0x10))
#define DDRP_ZQXSR1(n)			(0x18c + ((n) * 0x10))

#define DDRP_DXGCR(n)			(0x1c0 + ((n) * 0x40))
#define DDRP_DXGSR0(n)			(0x1c4 + ((n) * 0x40))
#define DDRP_DXGSR1(n)			(0x1c8 + ((n) * 0x40))
#define DDRP_DXDQSTR(n)			(0x1d4 + ((n) * 0x40))

/* DDRC Status Register */
#define DDRC_ST_ENDIAN			BIT(7)
#define DDRC_ST_DPDN			BIT(5)
#define DDRC_ST_PDN			BIT(4)
#define DDRC_ST_AREF			BIT(3)
#define DDRC_ST_SREF			BIT(2)
#define DDRC_ST_CKE1			BIT(1)
#define DDRC_ST_CKE0			BIT(0)

/* DDRC Configure Register */
#define DDRC_CFG_ROW1_BIT		27
#define DDRC_CFG_ROW1_MASK		(0x7 << DDRC_CFG_ROW1_BIT)
#define DDRC_CFG_COL1_BIT		24
#define DDRC_CFG_COL1_MASK		(0x7 << DDRC_CFG_COL1_BIT)
#define DDRC_CFG_BA1			BIT(23)
#define DDRC_CFG_IMBA			BIT(22)
#define DDRC_CFG_BL_8			BIT(21)

#define DDRC_CFG_TYPE_BIT		17
#define DDRC_CFG_TYPE_MASK		(0x7 << DDRC_CFG_TYPE_BIT)
#define DDRC_CFG_TYPE_DDR1		(2 << DDRC_CFG_TYPE_BIT)
#define DDRC_CFG_TYPE_MDDR		(3 << DDRC_CFG_TYPE_BIT)
#define DDRC_CFG_TYPE_DDR2		(4 << DDRC_CFG_TYPE_BIT)
#define DDRC_CFG_TYPE_LPDDR2		(5 << DDRC_CFG_TYPE_BIT)
#define DDRC_CFG_TYPE_DDR3		(6 << DDRC_CFG_TYPE_BIT)

#define DDRC_CFG_ODT_EN			BIT(16)

#define DDRC_CFG_MPRT			BIT(15)

#define DDRC_CFG_ROW_BIT		11
#define DDRC_CFG_ROW_MASK		(0x7 << DDRC_CFG_ROW_BIT)
#define DDRC_CFG_ROW_12			(0 << DDRC_CFG_ROW_BIT)
#define DDRC_CFG_ROW_13			(1 << DDRC_CFG_ROW_BIT)
#define DDRC_CFG_ROW_14			(2 << DDRC_CFG_ROW_BIT)

#define DDRC_CFG_COL_BIT		8
#define DDRC_CFG_COL_MASK		(0x7 << DDRC_CFG_COL_BIT)
#define DDRC_CFG_COL_8			(0 << DDRC_CFG_COL_BIT)
#define DDRC_CFG_COL_9			(1 << DDRC_CFG_COL_BIT)
#define DDRC_CFG_COL_10			(2 << DDRC_CFG_COL_BIT)
#define DDRC_CFG_COL_11			(3 << DDRC_CFG_COL_BIT)

#define DDRC_CFG_CS1EN			BIT(7)
#define DDRC_CFG_CS0EN			BIT(6)
#define DDRC_CFG_CL_BIT			2
#define DDRC_CFG_CL_MASK		(0xf << DDRC_CFG_CL_BIT)
#define DDRC_CFG_CL_3			(0 << DDRC_CFG_CL_BIT)
#define DDRC_CFG_CL_4			(1 << DDRC_CFG_CL_BIT)
#define DDRC_CFG_CL_5			(2 << DDRC_CFG_CL_BIT)
#define DDRC_CFG_CL_6			(3 << DDRC_CFG_CL_BIT)

#define DDRC_CFG_BA			BIT(1)
#define DDRC_CFG_DW			BIT(0)

/* DDRC Control Register */
#define DDRC_CTRL_DFI_RST		BIT(23)
#define DDRC_CTRL_DLL_RST		BIT(22)
#define DDRC_CTRL_CTL_RST		BIT(21)
#define DDRC_CTRL_CFG_RST		BIT(20)
#define DDRC_CTRL_ACTPD			BIT(15)
#define DDRC_CTRL_PDT_BIT		12
#define DDRC_CTRL_PDT_MASK		(0x7 << DDRC_CTRL_PDT_BIT)
#define DDRC_CTRL_PDT_DIS		(0 << DDRC_CTRL_PDT_BIT)
#define DDRC_CTRL_PDT_8			(1 << DDRC_CTRL_PDT_BIT)
#define DDRC_CTRL_PDT_16		(2 << DDRC_CTRL_PDT_BIT)
#define DDRC_CTRL_PDT_32		(3 << DDRC_CTRL_PDT_BIT)
#define DDRC_CTRL_PDT_64		(4 << DDRC_CTRL_PDT_BIT)
#define DDRC_CTRL_PDT_128		(5 << DDRC_CTRL_PDT_BIT)

#define DDRC_CTRL_PRET_BIT		8
#define DDRC_CTRL_PRET_MASK		(0x7 << DDRC_CTRL_PRET_BIT)
#define DDRC_CTRL_PRET_DIS		(0 << DDRC_CTRL_PRET_BIT)
#define DDRC_CTRL_PRET_8		(1 << DDRC_CTRL_PRET_BIT)
#define DDRC_CTRL_PRET_16		(2 << DDRC_CTRL_PRET_BIT)
#define DDRC_CTRL_PRET_32		(3 << DDRC_CTRL_PRET_BIT)
#define DDRC_CTRL_PRET_64		(4 << DDRC_CTRL_PRET_BIT)
#define DDRC_CTRL_PRET_128		(5 << DDRC_CTRL_PRET_BIT)

#define DDRC_CTRL_DPD			BIT(6)
#define DDRC_CTRL_SR			BIT(5)
#define DDRC_CTRL_UNALIGN		BIT(4)
#define DDRC_CTRL_ALH			BIT(3)
#define DDRC_CTRL_RDC			BIT(2)
#define DDRC_CTRL_CKE			BIT(1)
#define DDRC_CTRL_RESET			BIT(0)

/* DDRC Load-Mode-Register */
#define DDRC_LMR_DDR_ADDR_BIT		16
#define DDRC_LMR_DDR_ADDR_MASK		(0x3fff << DDRC_LMR_DDR_ADDR_BIT)

#define DDRC_LMR_BA_BIT			8
#define DDRC_LMR_BA_MASK		(0x7 << DDRC_LMR_BA_BIT)
/* For DDR2 */
#define DDRC_LMR_BA_MRS			(0 << DDRC_LMR_BA_BIT)
#define DDRC_LMR_BA_EMRS1		(1 << DDRC_LMR_BA_BIT)
#define DDRC_LMR_BA_EMRS2		(2 << DDRC_LMR_BA_BIT)
#define DDRC_LMR_BA_EMRS3		(3 << DDRC_LMR_BA_BIT)
/* For mobile DDR */
#define DDRC_LMR_BA_M_MRS		(0 << DDRC_LMR_BA_BIT)
#define DDRC_LMR_BA_M_EMRS		(2 << DDRC_LMR_BA_BIT)
#define DDRC_LMR_BA_M_SR		(1 << DDRC_LMR_BA_BIT)
/* For Normal DDR1 */
#define DDRC_LMR_BA_N_MRS		(0 << DDRC_LMR_BA_BIT)
#define DDRC_LMR_BA_N_EMRS		(1 << DDRC_LMR_BA_BIT)

#define DDRC_LMR_CMD_BIT		4
#define DDRC_LMR_CMD_MASK		(0x3 << DDRC_LMR_CMD_BIT)
#define DDRC_LMR_CMD_PREC		(0 << DDRC_LMR_CMD_BIT)
#define DDRC_LMR_CMD_AUREF		(1 << DDRC_LMR_CMD_BIT)
#define DDRC_LMR_CMD_LMR		(2 << DDRC_LMR_CMD_BIT)

#define DDRC_LMR_START			BIT(0)

/* DDRC Timing Config Register 1 */
#define DDRC_TIMING1_TRTP_BIT		24
#define DDRC_TIMING1_TRTP_MASK		(0x3f << DDRC_TIMING1_TRTP_BIT)
#define DDRC_TIMING1_TWTR_BIT		16
#define DDRC_TIMING1_TWTR_MASK		(0x3f << DDRC_TIMING1_TWTR_BIT)
#define DDRC_TIMING1_TWTR_1		(0 << DDRC_TIMING1_TWTR_BIT)
#define DDRC_TIMING1_TWTR_2		(1 << DDRC_TIMING1_TWTR_BIT)
#define DDRC_TIMING1_TWTR_3		(2 << DDRC_TIMING1_TWTR_BIT)
#define DDRC_TIMING1_TWTR_4		(3 << DDRC_TIMING1_TWTR_BIT)
#define DDRC_TIMING1_TWR_BIT		8
#define DDRC_TIMING1_TWR_MASK		(0x3f << DDRC_TIMING1_TWR_BIT)
#define DDRC_TIMING1_TWR_1		(0 << DDRC_TIMING1_TWR_BIT)
#define DDRC_TIMING1_TWR_2		(1 << DDRC_TIMING1_TWR_BIT)
#define DDRC_TIMING1_TWR_3		(2 << DDRC_TIMING1_TWR_BIT)
#define DDRC_TIMING1_TWR_4		(3 << DDRC_TIMING1_TWR_BIT)
#define DDRC_TIMING1_TWR_5		(4 << DDRC_TIMING1_TWR_BIT)
#define DDRC_TIMING1_TWR_6		(5 << DDRC_TIMING1_TWR_BIT)
#define DDRC_TIMING1_TWL_BIT		0
#define DDRC_TIMING1_TWL_MASK		(0x3f << DDRC_TIMING1_TWL_BIT)

/* DDRC Timing Config Register 2 */
#define DDRC_TIMING2_TCCD_BIT		24
#define DDRC_TIMING2_TCCD_MASK		(0x3f << DDRC_TIMING2_TCCD_BIT)
#define DDRC_TIMING2_TRAS_BIT		16
#define DDRC_TIMING2_TRAS_MASK		(0x3f << DDRC_TIMING2_TRAS_BIT)
#define DDRC_TIMING2_TRCD_BIT		8
#define DDRC_TIMING2_TRCD_MASK		(0x3f << DDRC_TIMING2_TRCD_BIT)
#define DDRC_TIMING2_TRL_BIT		0
#define DDRC_TIMING2_TRL_MASK		(0x3f << DDRC_TIMING2_TRL_BIT)

/* DDRC Timing Config Register 3 */
#define DDRC_TIMING3_ONUM		27
#define DDRC_TIMING3_TCKSRE_BIT		24
#define DDRC_TIMING3_TCKSRE_MASK	(0x3f << DDRC_TIMING3_TCKSRE_BIT)
#define DDRC_TIMING3_TRP_BIT		16
#define DDRC_TIMING3_TRP_MASK		(0x3f << DDRC_TIMING3_TRP_BIT)
#define DDRC_TIMING3_TRRD_BIT		8
#define DDRC_TIMING3_TRRD_MASK		(0x3f << DDRC_TIMING3_TRRD_BIT)
#define DDRC_TIMING3_TRRD_DISABLE	(0 << DDRC_TIMING3_TRRD_BIT)
#define DDRC_TIMING3_TRRD_2		(1 << DDRC_TIMING3_TRRD_BIT)
#define DDRC_TIMING3_TRRD_3		(2 << DDRC_TIMING3_TRRD_BIT)
#define DDRC_TIMING3_TRRD_4		(3 << DDRC_TIMING3_TRRD_BIT)
#define DDRC_TIMING3_TRC_BIT		0
#define DDRC_TIMING3_TRC_MASK		(0x3f << DDRC_TIMING3_TRC_BIT)

/* DDRC Timing Config Register 4 */
#define DDRC_TIMING4_TRFC_BIT		24
#define DDRC_TIMING4_TRFC_MASK		(0x3f << DDRC_TIMING4_TRFC_BIT)
#define DDRC_TIMING4_TEXTRW_BIT		21
#define DDRC_TIMING4_TEXTRW_MASK	(0x7 << DDRC_TIMING4_TEXTRW_BIT)
#define DDRC_TIMING4_TRWCOV_BIT		19
#define DDRC_TIMING4_TRWCOV_MASK	(0x3 << DDRC_TIMING4_TRWCOV_BIT)
#define DDRC_TIMING4_TCKE_BIT		16
#define DDRC_TIMING4_TCKE_MASK		(0x7 << DDRC_TIMING4_TCKE_BIT)
#define DDRC_TIMING4_TMINSR_BIT		8
#define DDRC_TIMING4_TMINSR_MASK	(0xf << DDRC_TIMING4_TMINSR_BIT)
#define DDRC_TIMING4_TXP_BIT		4
#define DDRC_TIMING4_TXP_MASK		(0x7 << DDRC_TIMING4_TXP_BIT)
#define DDRC_TIMING4_TMRD_BIT		0
#define DDRC_TIMING4_TMRD_MASK		(0x3 << DDRC_TIMING4_TMRD_BIT)

/* DDRC Timing Config Register 5 */
#define DDRC_TIMING5_TCTLUPD_BIT	24
#define DDRC_TIMING4_TCTLUPD_MASK	(0x3f << DDRC_TIMING5_TCTLUDP_BIT)
#define DDRC_TIMING5_TRTW_BIT		16
#define DDRC_TIMING5_TRTW_MASK		(0x3f << DDRC_TIMING5_TRTW_BIT)
#define DDRC_TIMING5_TRDLAT_BIT		8
#define DDRC_TIMING5_TRDLAT_MASK	(0x3f << DDRC_TIMING5_TRDLAT_BIT)
#define DDRC_TIMING5_TWDLAT_BIT		0
#define DDRC_TIMING5_TWDLAT_MASK	(0x3f << DDRC_TIMING5_TWDLAT_BIT)

/* DDRC Timing Config Register 6 */
#define DDRC_TIMING6_TXSRD_BIT		24
#define DDRC_TIMING6_TXSRD_MASK		(0x3f << DDRC_TIMING6_TXSRD_BIT)
#define DDRC_TIMING6_TFAW_BIT		16
#define DDRC_TIMING6_TFAW_MASK		(0x3f << DDRC_TIMING6_TFAW_BIT)
#define DDRC_TIMING6_TCFGW_BIT		8
#define DDRC_TIMING6_TCFGW_MASK		(0x3f << DDRC_TIMING6_TCFGW_BIT)
#define DDRC_TIMING6_TCFGR_BIT		0
#define DDRC_TIMING6_TCFGR_MASK		(0x3f << DDRC_TIMING6_TCFGR_BIT)

/* DDRC  Auto-Refresh Counter */
#define DDRC_REFCNT_CON_BIT		16
#define DDRC_REFCNT_CON_MASK		(0xff << DDRC_REFCNT_CON_BIT)
#define DDRC_REFCNT_CNT_BIT		8
#define DDRC_REFCNT_CNT_MASK		(0xff << DDRC_REFCNT_CNT_BIT)
#define DDRC_REFCNT_CLKDIV_BIT		1
#define DDRC_REFCNT_CLKDIV_MASK		(0x7 << DDRC_REFCNT_CLKDIV_BIT)
#define DDRC_REFCNT_REF_EN		BIT(0)

/* DDRC DQS Delay Control Register */
#define DDRC_DQS_ERROR			BIT(29)
#define DDRC_DQS_READY			BIT(28)
#define DDRC_DQS_AUTO			BIT(23)
#define DDRC_DQS_DET			BIT(24)
#define DDRC_DQS_SRDET			BIT(25)
#define DDRC_DQS_CLKD_BIT		16
#define DDRC_DQS_CLKD_MASK		(0x3f << DDRC_DQS_CLKD_BIT)
#define DDRC_DQS_WDQS_BIT		8
#define DDRC_DQS_WDQS_MASK		(0x3f << DDRC_DQS_WDQS_BIT)
#define DDRC_DQS_RDQS_BIT		0
#define DDRC_DQS_RDQS_MASK		(0x3f << DDRC_DQS_RDQS_BIT)

/* DDRC DQS Delay Adjust Register */
#define DDRC_DQS_ADJWDQS_BIT		8
#define DDRC_DQS_ADJWDQS_MASK		(0x1f << DDRC_DQS_ADJWDQS_BIT)
#define DDRC_DQS_ADJRDQS_BIT		0
#define DDRC_DQS_ADJRDQS_MASK		(0x1f << DDRC_DQS_ADJRDQS_BIT)

/* DDRC Memory Map Config Register */
#define DDRC_MMAP_BASE_BIT		8
#define DDRC_MMAP_BASE_MASK		(0xff << DDRC_MMAP_BASE_BIT)
#define DDRC_MMAP_MASK_BIT		0
#define DDRC_MMAP_MASK_MASK		(0xff << DDRC_MMAP_MASK_BIT)

#define DDRC_MMAP0_BASE			(0x20 << DDRC_MMAP_BASE_BIT)
#define DDRC_MMAP1_BASE_64M		(0x24 << DDRC_MMAP_BASE_BIT)
#define DDRC_MMAP1_BASE_128M		(0x28 << DDRC_MMAP_BASE_BIT)
#define DDRC_MMAP1_BASE_256M		(0x30 << DDRC_MMAP_BASE_BIT)

#define DDRC_MMAP_MASK_64_64		(0xfc << DDRC_MMAP_MASK_BIT)
#define DDRC_MMAP_MASK_128_128		(0xf8 << DDRC_MMAP_MASK_BIT)
#define DDRC_MMAP_MASK_256_256		(0xf0 << DDRC_MMAP_MASK_BIT)

/* DDRP PHY Initialization Register */
#define DDRP_PIR_INIT			BIT(0)
#define DDRP_PIR_DLLSRST		BIT(1)
#define DDRP_PIR_DLLLOCK		BIT(2)
#define DDRP_PIR_ZCAL			BIT(3)
#define DDRP_PIR_ITMSRST		BIT(4)
#define DDRP_PIR_DRAMRST		BIT(5)
#define DDRP_PIR_DRAMINT		BIT(6)
#define DDRP_PIR_QSTRN			BIT(7)
#define DDRP_PIR_EYETRN			BIT(8)
#define DDRP_PIR_DLLBYP			BIT(17)
/* DDRP PHY General Configurate Register */
#define DDRP_PGCR_ITMDMD		BIT(0)
#define DDRP_PGCR_DQSCFG		BIT(1)
#define DDRP_PGCR_DFTCMP		BIT(2)
#define DDRP_PGCR_DFTLMT_BIT		3
#define DDRP_PGCR_DTOSEL_BIT		5
#define DDRP_PGCR_CKEN_BIT		9
#define DDRP_PGCR_CKDV_BIT		12
#define DDRP_PGCR_CKINV			BIT(14)
#define DDRP_PGCR_RANKEN_BIT		18
#define DDRP_PGCR_ZCKSEL_32		(2 << 22)
#define DDRP_PGCR_PDDISDX		BIT(24)
/* DDRP PHY General Status Register */
#define DDRP_PGSR_IDONE			BIT(0)
#define DDRP_PGSR_DLDONE		BIT(1)
#define DDRP_PGSR_ZCDONE		BIT(2)
#define DDRP_PGSR_DIDONE		BIT(3)
#define DDRP_PGSR_DTDONE		BIT(4)
#define DDRP_PGSR_DTERR			BIT(5)
#define DDRP_PGSR_DTIERR		BIT(6)
#define DDRP_PGSR_DFTEERR		BIT(7)
/* DDRP DRAM Configuration Register */
#define DDRP_DCR_TYPE_BIT		0
#define DDRP_DCR_TYPE_MASK		(0x7 << DDRP_DCR_TYPE_BIT)
#define DDRP_DCR_TYPE_MDDR		(0 << DDRP_DCR_TYPE_BIT)
#define DDRP_DCR_TYPE_DDR		(1 << DDRP_DCR_TYPE_BIT)
#define DDRP_DCR_TYPE_DDR2		(2 << DDRP_DCR_TYPE_BIT)
#define DDRP_DCR_TYPE_DDR3		(3 << DDRP_DCR_TYPE_BIT)
#define DDRP_DCR_TYPE_LPDDR2		(4 << DDRP_DCR_TYPE_BIT)
#define DDRP_DCR_DDR8BNK_BIT		3
#define DDRP_DCR_DDR8BNK_MASK		(1 << DDRP_DCR_DDR8BNK_BIT)
#define DDRP_DCR_DDR8BNK		(1 << DDRP_DCR_DDR8BNK_BIT)
#define DDRP_DCR_DDR8BNK_DIS		(0 << DDRP_DCR_DDR8BNK_BIT)

#define DRP_DTRP1_RTODT			BIT(11)

#define DDRP_DXGCR_DXEN			BIT(0)

#define DDRP_ZQXCR_ZDEN_BIT		28
#define DDRP_ZQXCR_ZDEN			(1 << DDRP_ZQXCR_ZDEN_BIT)
#define DDRP_ZQXCR_PULLUP_IMPE_BIT	5
#define DDRP_ZQXCR_PULLDOWN_IMPE_BIT	0

/* DDR3 Mode Register Set */
#define DDR3_MR0_BL_BIT			0
#define DDR3_MR0_BL_MASK		(3 << DDR3_MR0_BL_BIT)
#define DDR3_MR0_BL_8			(0 << DDR3_MR0_BL_BIT)
#define DDR3_MR0_BL_fly			(1 << DDR3_MR0_BL_BIT)
#define DDR3_MR0_BL_4			(2 << DDR3_MR0_BL_BIT)
#define DDR3_MR0_BT_BIT			3
#define DDR3_MR0_BT_MASK		(1 << DDR3_MR0_BT_BIT)
#define DDR3_MR0_BT_SEQ			(0 << DDR3_MR0_BT_BIT)
#define DDR3_MR0_BT_INTER		(1 << DDR3_MR0_BT_BIT)
#define DDR3_MR0_WR_BIT			9

#define DDR3_MR1_DLL_DISABLE		1
#define DDR3_MR1_DIC_6			(0 << 5 | 0 << 1)
#define DDR3_MR1_DIC_7			(0 << 5 | BIT(1))
#define DDR3_MR1_RTT_DIS		(0 << 9 | 0 << 6 | 0 << 2)
#define DDR3_MR1_RTT_4			(0 << 9 | 0 << 6 | BIT(2))
#define DDR3_MR1_RTT_2			(0 << 9 | BIT(6) | 0 << 2)
#define DDR3_MR1_RTT_6			(0 << 9 | BIT(6) | BIT(2))
#define DDR3_MR1_RTT_12			(BIT(9) | 0 << 6 | 0 << 2)
#define DDR3_MR1_RTT_8			(BIT(9) | 0 << 6 | BIT(2))

#define DDR3_MR2_CWL_BIT		3

/* Parameters common to all RAM devices used */

/* Chip Select */
/* CSEN : whether a ddr chip exists 0 - un-used, 1 - used */
#define DDR_CS0EN	1
/* CSEN : whether a ddr chip exists 0 - un-used, 1 - used */
#define DDR_CS1EN	0

/* ROW : 12 to 18 row address, 1G only 512MB */
#define DDR_ROW		15
/* COL :  8 to 14 column address */
#define DDR_COL		10
/* Banks each chip: 0-4bank, 1-8bank */
#define DDR_BANK8	1
/* 0 - 16-bit data width, 1 - 32-bit data width */
#define DDR_DW32	1

/* Refresh period: 64ms / 32768 = 1.95 us , 2 ^ 15 = 32768 */
#define DDR_tREFI	7800
/* Clock Divider */
#define DDR_CLK_DIV	1

/* DDR3 Burst length: 0 - 8 burst, 2 - 4 burst , 1 - 4 or 8 (on the fly) */
#define DDR_BL		8

/* CAS latency: 5 to 14, tCK */
#define DDR_CL		6
/* DDR3 only: CAS Write Latency, 5 to 8 */
#define DDR_tCWL	(DDR_CL - 1)

/* Structure representing per-RAM type configuration */

struct jz4780_ddr_config {
	u32	timing[6];	/* Timing1..6 register value */

	/* DDR PHY control */
	u16	mr0;	/* Mode Register 0 */
	u16	mr1;	/* Mode Register 1 */

	u32	ptr0;	/* PHY Timing Register 0 */
	u32	ptr1;	/* PHY Timing Register 1 */
	u32	ptr2;	/* PHY Timing Register 1 */

	u32	dtpr0;	/* DRAM Timing Parameters Register 0 */
	u32	dtpr1;	/* DRAM Timing Parameters Register 1 */
	u32	dtpr2;	/* DRAM Timing Parameters Register 2 */

	u8	pullup;	/* PHY pullup impedance */
	u8	pulldn;	/* PHY pulldown impedance */
};

void pll_init(void);
void sdram_init(void);

#endif	/* __JZ4780_DRAM_H__ */

