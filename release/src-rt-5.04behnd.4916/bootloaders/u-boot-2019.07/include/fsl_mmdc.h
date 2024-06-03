/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2016 Freescale Semiconductor, Inc.
 */

#ifndef FSL_MMDC_H
#define FSL_MMDC_H

/* PHY Write Leveling Configuration and Error Status Register (MPWLGCR) */
#define MPWLGCR_HW_WL_EN		(1 << 0)

/* PHY Pre-defined Compare and CA delay-line Configuration (MPPDCMPR2) */
#define MPPDCMPR2_MPR_COMPARE_EN	(1 << 0)


/* MMDC PHY Read DQS gating control register 0 (MPDGCTRL0) */
#define AUTO_RD_DQS_GATING_CALIBRATION_EN	(1 << 28)

/* MMDC PHY Read Delay HW Calibration Control Register (MPRDDLHWCTL) */
#define MPRDDLHWCTL_AUTO_RD_CALIBRATION_EN	(1 << 4)

/* MMDC Core Power Saving Control and Status Register (MMDC_MAPSR) */
#define MMDC_MAPSR_PWR_SAV_CTRL_STAT	0x00001067

/* MMDC Core Refresh Control Register (MMDC_MDREF) */
#define MDREF_START_REFRESH	(1 << 0)

/* MMDC Core Special Command Register (MDSCR) */
#define CMD_ADDR_MSB_MR_OP(x)	(x << 24)
#define CMD_ADDR_LSB_MR_ADDR(x) (x << 16)
#define MDSCR_DISABLE_CFG_REQ   (0 << 15)
#define MDSCR_ENABLE_CON_REQ	(1 << 15)
#define MDSCR_CON_ACK		(1 << 14)
#define MDSCR_WL_EN		(1 << 9)
#define	CMD_NORMAL		(0 << 4)
#define	CMD_PRECHARGE		(1 << 4)
#define	CMD_AUTO_REFRESH	(2 << 4)
#define	CMD_LOAD_MODE_REG	(3 << 4)
#define	CMD_ZQ_CALIBRATION	(4 << 4)
#define	CMD_PRECHARGE_BANK_OPEN	(5 << 4)
#define	CMD_MRR			(6 << 4)
#define CMD_BANK_ADDR_0		0x0
#define CMD_BANK_ADDR_1		0x1
#define CMD_BANK_ADDR_2		0x2
#define CMD_BANK_ADDR_3		0x3
#define CMD_BANK_ADDR_4		0x4
#define CMD_BANK_ADDR_5		0x5
#define CMD_BANK_ADDR_6		0x6
#define CMD_BANK_ADDR_7		0x7

/* MMDC Core Control Register (MDCTL) */
#define MDCTL_SDE0		(1 << 31)
#define MDCTL_SDE1		(1 << 30)

/* MMDC PHY ZQ HW control register (MMDC_MPZQHWCTRL) */
#define MPZQHWCTRL_ZQ_HW_FORCE	(1 << 16)

/* MMDC PHY Measure Unit Register (MMDC_MPMUR0) */
#define MMDC_MPMUR0_FRC_MSR	(1 << 11)

/* MMDC PHY Read delay-lines Configuration Register (MMDC_MPRDDLCTL) */
/* default 64 for a quarter cycle delay */
#define MMDC_MPRDDLCTL_DEFAULT_DELAY	0x40404040

/* MMDC Registers */
struct mmdc_regs {
	u32 mdctl;
	u32 mdpdc;
	u32 mdotc;
	u32 mdcfg0;
	u32 mdcfg1;
	u32 mdcfg2;
	u32 mdmisc;
	u32 mdscr;
	u32 mdref;
	u32 res1[2];
	u32 mdrwd;
	u32 mdor;
	u32 mdmrr;
	u32 mdcfg3lp;
	u32 mdmr4;
	u32 mdasp;
	u32 res2[239];
	u32 maarcr;
	u32 mapsr;
	u32 maexidr0;
	u32 maexidr1;
	u32 madpcr0;
	u32 madpcr1;
	u32 madpsr0;
	u32 madpsr1;
	u32 madpsr2;
	u32 madpsr3;
	u32 madpsr4;
	u32 madpsr5;
	u32 masbs0;
	u32 masbs1;
	u32 res3[2];
	u32 magenp;
	u32 res4[239];
	u32 mpzqhwctrl;
	u32 mpzqswctrl;
	u32 mpwlgcr;
	u32 mpwldectrl0;
	u32 mpwldectrl1;
	u32 mpwldlst;
	u32 mpodtctrl;
	u32 mprddqby0dl;
	u32 mprddqby1dl;
	u32 mprddqby2dl;
	u32 mprddqby3dl;
	u32 mpwrdqby0dl;
	u32 mpwrdqby1dl;
	u32 mpwrdqby2dl;
	u32 mpwrdqby3dl;
	u32 mpdgctrl0;
	u32 mpdgctrl1;
	u32 mpdgdlst0;
	u32 mprddlctl;
	u32 mprddlst;
	u32 mpwrdlctl;
	u32 mpwrdlst;
	u32 mpsdctrl;
	u32 mpzqlp2ctl;
	u32 mprddlhwctl;
	u32 mpwrdlhwctl;
	u32 mprddlhwst0;
	u32 mprddlhwst1;
	u32 mpwrdlhwst0;
	u32 mpwrdlhwst1;
	u32 mpwlhwerr;
	u32 mpdghwst0;
	u32 mpdghwst1;
	u32 mpdghwst2;
	u32 mpdghwst3;
	u32 mppdcmpr1;
	u32 mppdcmpr2;
	u32 mpswdar0;
	u32 mpswdrdr0;
	u32 mpswdrdr1;
	u32 mpswdrdr2;
	u32 mpswdrdr3;
	u32 mpswdrdr4;
	u32 mpswdrdr5;
	u32 mpswdrdr6;
	u32 mpswdrdr7;
	u32 mpmur0;
	u32 mpwrcadl;
	u32 mpdccr;
};

struct fsl_mmdc_info {
	u32 mdctl;
	u32 mdpdc;
	u32 mdotc;
	u32 mdcfg0;
	u32 mdcfg1;
	u32 mdcfg2;
	u32 mdmisc;
	u32 mdref;
	u32 mdrwd;
	u32 mdor;
	u32 mdasp;
	u32 mpodtctrl;
	u32 mpzqhwctrl;
	u32 mprddlctl;
};

void mmdc_init(const struct fsl_mmdc_info *);

#endif /* FSL_MMDC_H */
