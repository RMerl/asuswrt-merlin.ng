/* SPDX-License-Identifier: GPL-2.0
 *
 * Copyright (C) 2017-2018 Intel Corporation <www.intel.com>
 *
 */

#ifndef	_SDRAM_S10_H_
#define	_SDRAM_S10_H_

#define DDR_TWR				15
#define DDR_READ_LATENCY_DELAY		40
#define DDR_ACTIVATE_FAWBANK		0x1

/* ECC HMC registers */
#define DDRIOCTRL			0x8
#define DDRCALSTAT			0xc
#define DRAMADDRWIDTH			0xe0
#define ECCCTRL1			0x100
#define ECCCTRL2			0x104
#define ERRINTEN			0x110
#define ERRINTENS			0x114
#define INTMODE				0x11c
#define INTSTAT				0x120
#define AUTOWB_CORRADDR			0x138
#define ECC_REG2WRECCDATABUS		0x144
#define ECC_DIAGON			0x150
#define ECC_DECSTAT			0x154
#define HPSINTFCSEL			0x210
#define RSTHANDSHAKECTRL		0x214
#define RSTHANDSHAKESTAT		0x218

#define DDR_HMC_DDRIOCTRL_IOSIZE_MSK		0x00000003
#define DDR_HMC_DDRCALSTAT_CAL_MSK		BIT(0)
#define DDR_HMC_ECCCTL_AWB_CNT_RST_SET_MSK	BIT(16)
#define DDR_HMC_ECCCTL_CNT_RST_SET_MSK		BIT(8)
#define DDR_HMC_ECCCTL_ECC_EN_SET_MSK		BIT(0)
#define DDR_HMC_ECCCTL2_RMW_EN_SET_MSK		BIT(8)
#define DDR_HMC_ECCCTL2_AWB_EN_SET_MSK		BIT(0)
#define DDR_HMC_ECC_DIAGON_ECCDIAGON_EN_SET_MSK	BIT(16)
#define DDR_HMC_ECC_DIAGON_WRDIAGON_EN_SET_MSK	BIT(0)
#define DDR_HMC_ERRINTEN_SERRINTEN_EN_SET_MSK	BIT(0)
#define DDR_HMC_ERRINTEN_DERRINTEN_EN_SET_MSK	BIT(1)
#define DDR_HMC_INTSTAT_SERRPENA_SET_MSK	BIT(0)
#define DDR_HMC_INTSTAT_DERRPENA_SET_MSK	BIT(1)
#define DDR_HMC_INTSTAT_ADDRMTCFLG_SET_MSK	BIT(16)
#define DDR_HMC_INTMODE_INTMODE_SET_MSK		BIT(0)
#define DDR_HMC_RSTHANDSHAKE_MASK		0x000000ff
#define DDR_HMC_CORE2SEQ_INT_REQ		0xF
#define DDR_HMC_SEQ2CORE_INT_RESP_MASK		BIT(3)
#define DDR_HMC_HPSINTFCSEL_ENABLE_MASK		0x001f1f1f

#define	DDR_HMC_ERRINTEN_INTMASK				\
		(DDR_HMC_ERRINTEN_SERRINTEN_EN_SET_MSK |	\
		 DDR_HMC_ERRINTEN_DERRINTEN_EN_SET_MSK)

/* NOC DDR scheduler */
#define DDR_SCH_ID_COREID		0
#define DDR_SCH_ID_REVID		0x4
#define DDR_SCH_DDRCONF			0x8
#define DDR_SCH_DDRTIMING		0xc
#define DDR_SCH_DDRMODE			0x10
#define DDR_SCH_READ_LATENCY		0x14
#define DDR_SCH_ACTIVATE		0x38
#define DDR_SCH_DEVTODEV		0x3c
#define DDR_SCH_DDR4TIMING		0x40

#define DDR_SCH_DDRTIMING_ACTTOACT_OFF		0
#define DDR_SCH_DDRTIMING_RDTOMISS_OFF		6
#define DDR_SCH_DDRTIMING_WRTOMISS_OFF		12
#define DDR_SCH_DDRTIMING_BURSTLEN_OFF		18
#define DDR_SCH_DDRTIMING_RDTOWR_OFF		21
#define DDR_SCH_DDRTIMING_WRTORD_OFF		26
#define DDR_SCH_DDRTIMING_BWRATIO_OFF		31
#define DDR_SCH_DDRMOD_BWRATIOEXTENDED_OFF	1
#define DDR_SCH_ACTIVATE_RRD_OFF		0
#define DDR_SCH_ACTIVATE_FAW_OFF		4
#define DDR_SCH_ACTIVATE_FAWBANK_OFF		10
#define DDR_SCH_DEVTODEV_BUSRDTORD_OFF		0
#define DDR_SCH_DEVTODEV_BUSRDTOWR_OFF		2
#define DDR_SCH_DEVTODEV_BUSWRTORD_OFF		4

/* HMC MMR IO48 registers */
#define CTRLCFG0			0x28
#define CTRLCFG1			0x2c
#define DRAMTIMING0			0x50
#define CALTIMING0			0x7c
#define CALTIMING1			0x80
#define CALTIMING2			0x84
#define CALTIMING3			0x88
#define CALTIMING4			0x8c
#define CALTIMING9			0xa0
#define DRAMADDRW			0xa8
#define DRAMSTS				0xec
#define NIOSRESERVED0			0x110
#define NIOSRESERVED1			0x114
#define NIOSRESERVED2			0x118

#define DRAMADDRW_CFG_COL_ADDR_WIDTH(x)			\
	(((x) >> 0) & 0x1F)
#define DRAMADDRW_CFG_ROW_ADDR_WIDTH(x)			\
	(((x) >> 5) & 0x1F)
#define DRAMADDRW_CFG_BANK_ADDR_WIDTH(x)		\
	(((x) >> 10) & 0xF)
#define DRAMADDRW_CFG_BANK_GRP_ADDR_WIDTH(x)		\
	(((x) >> 14) & 0x3)
#define DRAMADDRW_CFG_CS_ADDR_WIDTH(x)			\
	(((x) >> 16) & 0x7)

#define CTRLCFG0_CFG_MEMTYPE(x)				\
	(((x) >> 0) & 0xF)
#define CTRLCFG0_CFG_DIMM_TYPE(x)			\
	(((x) >> 4) & 0x7)
#define CTRLCFG0_CFG_AC_POS(x)				\
	(((x) >> 7) & 0x3)
#define CTRLCFG0_CFG_CTRL_BURST_LEN(x)			\
	(((x) >> 9) & 0x1F)

#define CTRLCFG1_CFG_DBC3_BURST_LEN(x)			\
	(((x) >> 0) & 0x1F)
#define CTRLCFG1_CFG_ADDR_ORDER(x)			\
	(((x) >> 5) & 0x3)
#define CTRLCFG1_CFG_CTRL_EN_ECC(x)			\
	(((x) >> 7) & 0x1)

#define DRAMTIMING0_CFG_TCL(x)				\
	(((x) >> 0) & 0x7F)

#define CALTIMING0_CFG_ACT_TO_RDWR(x)			\
	(((x) >> 0) & 0x3F)
#define CALTIMING0_CFG_ACT_TO_PCH(x)			\
	(((x) >> 6) & 0x3F)
#define CALTIMING0_CFG_ACT_TO_ACT(x)			\
	(((x) >> 12) & 0x3F)
#define CALTIMING0_CFG_ACT_TO_ACT_DB(x)			\
	(((x) >> 18) & 0x3F)

#define CALTIMING1_CFG_RD_TO_RD(x)			\
	(((x) >> 0) & 0x3F)
#define CALTIMING1_CFG_RD_TO_RD_DC(x)			\
	(((x) >> 6) & 0x3F)
#define CALTIMING1_CFG_RD_TO_RD_DB(x)			\
	(((x) >> 12) & 0x3F)
#define CALTIMING1_CFG_RD_TO_WR(x)			\
	(((x) >> 18) & 0x3F)
#define CALTIMING1_CFG_RD_TO_WR_DC(x)			\
	(((x) >> 24) & 0x3F)

#define CALTIMING2_CFG_RD_TO_WR_DB(x)			\
	(((x) >> 0) & 0x3F)
#define CALTIMING2_CFG_RD_TO_WR_PCH(x)			\
	(((x) >> 6) & 0x3F)
#define CALTIMING2_CFG_RD_AP_TO_VALID(x)		\
	(((x) >> 12) & 0x3F)
#define CALTIMING2_CFG_WR_TO_WR(x)			\
	(((x) >> 18) & 0x3F)
#define CALTIMING2_CFG_WR_TO_WR_DC(x)			\
	(((x) >> 24) & 0x3F)

#define CALTIMING3_CFG_WR_TO_WR_DB(x)			\
	(((x) >> 0) & 0x3F)
#define CALTIMING3_CFG_WR_TO_RD(x)			\
	(((x) >> 6) & 0x3F)
#define CALTIMING3_CFG_WR_TO_RD_DC(x)			\
	(((x) >> 12) & 0x3F)
#define CALTIMING3_CFG_WR_TO_RD_DB(x)			\
	(((x) >> 18) & 0x3F)
#define CALTIMING3_CFG_WR_TO_PCH(x)			\
	(((x) >> 24) & 0x3F)

#define CALTIMING4_CFG_WR_AP_TO_VALID(x)		\
	(((x) >> 0) & 0x3F)
#define CALTIMING4_CFG_PCH_TO_VALID(x)			\
	(((x) >> 6) & 0x3F)
#define CALTIMING4_CFG_PCH_ALL_TO_VALID(x)		\
	(((x) >> 12) & 0x3F)
#define CALTIMING4_CFG_ARF_TO_VALID(x)			\
	(((x) >> 18) & 0xFF)
#define CALTIMING4_CFG_PDN_TO_VALID(x)			\
	(((x) >> 26) & 0x3F)

#define CALTIMING9_CFG_4_ACT_TO_ACT(x)			\
	(((x) >> 0) & 0xFF)

/* Firewall DDR scheduler MPFE */
#define FW_HMC_ADAPTOR_REG_ADDR			0xf8020004
#define FW_HMC_ADAPTOR_MPU_MASK			BIT(0)

#endif /* _SDRAM_S10_H_ */
