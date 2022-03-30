/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * DDR controller registers of the i.MX7 architecture
 *
 * (C) Copyright 2017 CompuLab, Ltd. http://www.compulab.com
 *
 * Author: Uri Mashiach <uri.mashiach@compulab.co.il>
 */

#ifndef __ASM_ARCH_MX7_DDR_H__
#define __ASM_ARCH_MX7_DDR_H__

/* DDRC Registers (DDRC_IPS_BASE_ADDR) */
struct ddrc {
	u32 mstr;		/* 0x0000 */
	u32 reserved1[0x18];
	u32 rfshtmg;		/* 0x0064 */
	u32 reserved2[0x1a];
	u32 init0;		/* 0x00d0 */
	u32 init1;		/* 0x00d4 */
	u32 reserved3;
	u32 init3;		/* 0x00dc */
	u32 init4;		/* 0x00e0 */
	u32 init5;		/* 0x00e4 */
	u32 reserved4[0x03];
	u32 rankctl;		/* 0x00f4 */
	u32 reserved5[0x02];
	u32 dramtmg0;		/* 0x0100 */
	u32 dramtmg1;		/* 0x0104 */
	u32 dramtmg2;		/* 0x0108 */
	u32 dramtmg3;		/* 0x010c */
	u32 dramtmg4;		/* 0x0110 */
	u32 dramtmg5;		/* 0x0114 */
	u32 reserved6[0x02];
	u32 dramtmg8;		/* 0x0120 */
	u32 reserved7[0x17];
	u32 zqctl0;		/* 0x0180 */
	u32 reserved8[0x03];
	u32 dfitmg0;		/* 0x0190 */
	u32 dfitmg1;		/* 0x0194 */
	u32 reserved9[0x02];
	u32 dfiupd0;		/* 0x01a0 */
	u32 dfiupd1;		/* 0x01a4 */
	u32 dfiupd2;		/* 0x01a8 */
	u32 reserved10[0x15];
	u32 addrmap0;		/* 0x0200 */
	u32 addrmap1;		/* 0x0204 */
	u32 addrmap2;		/* 0x0208 */
	u32 addrmap3;		/* 0x020c */
	u32 addrmap4;		/* 0x0210 */
	u32 addrmap5;		/* 0x0214 */
	u32 addrmap6;		/* 0x0218 */
	u32 reserved12[0x09];
	u32 odtcfg;		/* 0x0240 */
	u32 odtmap;		/* 0x0244 */
};

/* DDRC_MSTR fields */
#define MSTR_DATA_BUS_WIDTH_MASK	0x3 << 12
#define MSTR_DATA_BUS_WIDTH_SHIFT	12
#define MSTR_DATA_ACTIVE_RANKS_MASK	0xf << 24
#define MSTR_DATA_ACTIVE_RANKS_SHIFT	24
/* DDRC_ADDRMAP1 fields */
#define ADDRMAP1_BANK_B0_MASK		0x1f << 0
#define ADDRMAP1_BANK_B0_SHIFT		0
#define ADDRMAP1_BANK_B1_MASK		0x1f << 8
#define ADDRMAP1_BANK_B1_SHIFT		8
#define ADDRMAP1_BANK_B2_MASK		0x1f << 16
#define ADDRMAP1_BANK_B2_SHIFT		16
/* DDRC_ADDRMAP2 fields */
#define ADDRMAP2_COL_B2_MASK		0xF << 0
#define ADDRMAP2_COL_B2_SHIFT		0
#define ADDRMAP2_COL_B3_MASK		0xF << 8
#define ADDRMAP2_COL_B3_SHIFT		8
#define ADDRMAP2_COL_B4_MASK		0xF << 16
#define ADDRMAP2_COL_B4_SHIFT		16
#define ADDRMAP2_COL_B5_MASK		0xF << 24
#define ADDRMAP2_COL_B5_SHIFT		24
/* DDRC_ADDRMAP3 fields */
#define ADDRMAP3_COL_B6_MASK		0xF << 0
#define ADDRMAP3_COL_B6_SHIFT		0
#define ADDRMAP3_COL_B7_MASK		0xF << 8
#define ADDRMAP3_COL_B7_SHIFT		8
#define ADDRMAP3_COL_B8_MASK		0xF << 16
#define ADDRMAP3_COL_B8_SHIFT		16
#define ADDRMAP3_COL_B9_MASK		0xF << 24
#define ADDRMAP3_COL_B9_SHIFT		24
/* DDRC_ADDRMAP4 fields */
#define ADDRMAP4_COL_B10_MASK		0xF << 0
#define ADDRMAP4_COL_B10_SHIFT		0
#define ADDRMAP4_COL_B11_MASK		0xF << 8
#define ADDRMAP4_COL_B11_SHIFT		8
/* DDRC_ADDRMAP5 fields */
#define ADDRMAP5_ROW_B0_MASK		0xF << 0
#define ADDRMAP5_ROW_B0_SHIFT		0
#define ADDRMAP5_ROW_B1_MASK		0xF << 8
#define ADDRMAP5_ROW_B1_SHIFT		8
#define ADDRMAP5_ROW_B2_10_MASK		0xF << 16
#define ADDRMAP5_ROW_B2_10_SHIFT	16
#define ADDRMAP5_ROW_B11_MASK		0xF << 24
#define ADDRMAP5_ROW_B11_SHIFT		24
/* DDRC_ADDRMAP6 fields */
#define ADDRMAP6_ROW_B12_MASK		0xF << 0
#define ADDRMAP6_ROW_B12_SHIFT		0
#define ADDRMAP6_ROW_B13_MASK		0xF << 8
#define ADDRMAP6_ROW_B13_SHIFT		8
#define ADDRMAP6_ROW_B14_MASK		0xF << 16
#define ADDRMAP6_ROW_B14_SHIFT		16
#define ADDRMAP6_ROW_B15_MASK		0xF << 24
#define ADDRMAP6_ROW_B15_SHIFT		24

/* DDRC_MP Registers */
#define DDRC_MP_BASE_ADDR (DDRC_IPS_BASE_ADDR + 0x03fc)
struct ddrc_mp {
	u32 reserved1[0x25];
	u32 pctrl_0;		/* 0x0094 */
};

/* DDR_PHY registers */
struct ddr_phy {
	u32 phy_con0;		/* 0x0000 */
	u32 phy_con1;		/* 0x0004 */
	u32 reserved1[0x02];
	u32 phy_con4;		/* 0x0010 */
	u32 reserved2;
	u32 offset_lp_con0;	/* 0x0018 */
	u32 reserved3;
	u32 offset_rd_con0;	/* 0x0020 */
	u32 reserved4[0x03];
	u32 offset_wr_con0;	/* 0x0030 */
	u32 reserved5[0x07];
	u32 cmd_sdll_con0;	/* 0x0050 */
	u32 reserved6[0x12];
	u32 drvds_con0;		/* 0x009c */
	u32 reserved7[0x04];
	u32 mdll_con0;		/* 0x00b0 */
	u32 reserved8[0x03];
	u32 zq_con0;		/* 0x00c0 */
};

#define DDR_PHY_CMD_SDLL_CON0_CTRL_RESYNC_MASK BIT(24)

#define MX7_CAL_VAL_MAX 5
/* Calibration parameters */
struct mx7_calibration {
	int num_val;			/* Number of calibration values */
	u32 values[MX7_CAL_VAL_MAX];	/* calibration values */
};

void mx7_dram_cfg(struct ddrc *ddrc_regs_val, struct ddrc_mp *ddrc_mp_val,
		  struct ddr_phy *ddr_phy_regs_val,
		  struct mx7_calibration *calib_param);

#endif	/*__ASM_ARCH_MX7_DDR_H__ */
