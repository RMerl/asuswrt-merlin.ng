/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2022 StarFive Technology Co., Ltd.
 * Author: Yanhong Wang<yanhong.wang@starfivetech.com>
 */

#ifndef __STARFIVE_DDR_H__
#define __STARFIVE_DDR_H__

#define SEC_CTRL_ADDR		0x1000
#define PHY_BASE_ADDR		0x800
#define PHY_AC_BASE_ADDR	0x1000

#define DDR_BUS_MASK		GENMASK(29, 24)
#define DDR_AXI_MASK		BIT(31)
#define DDR_BUS_OFFSET		0xAC
#define DDR_AXI_OFFSET		0xB0

#define DDR_BUS_OSC_DIV2	0
#define DDR_BUS_PLL1_DIV2	1
#define DDR_BUS_PLL1_DIV4	2
#define DDR_BUS_PLL1_DIV8	3
#define DDR_AXI_DISABLE		0
#define DDR_AXI_ENABLE		1

#define OFFSET_SEL		BIT(31)
#define REG2G			BIT(30)
#define REG4G			BIT(29)
#define REG8G			BIT(28)
#define F_ADDSET		BIT(2)
#define F_SET			BIT(1)
#define F_CLRSET		BIT(0)
#define REGALL			(REG2G | REG4G | REG8G)
#define REGSETALL		(F_SET | REGALL)
#define REGCLRSETALL		(F_CLRSET | REGALL)
#define REGADDSETALL		(F_ADDSET | REGALL)

struct ddr_reg_cfg {
	u32 offset;
	u32 mask;
	u32 val;
	u32 flag;
};

enum ddr_size_t {
	DDR_SIZE_2G,
	DDR_SIZE_4G,
	DDR_SIZE_8G,
	DDR_SIZE_16G,
};

void ddr_phy_train(u32 *phyreg);
void ddr_phy_util(u32 *phyreg);
void ddr_phy_start(u32 *phyreg, enum ddr_size_t size);
void ddrcsr_boot(u32 *csrreg, u32 *secreg, u32 *phyreg, enum ddr_size_t size);

#define DDR_REG_TRIGGER(addr, mask, value) \
	out_le32((addr), (in_le32(addr) & (mask)) | (value))

#define DDR_REG_SET(type, val) \
	clrsetbits_le32(JH7110_SYS_CRG + DDR_##type##_OFFSET, \
		DDR_##type##_MASK, \
		((val) << __ffs(DDR_##type##_MASK)) & DDR_##type##_MASK)

#endif /*__STARFIVE_DDR_H__*/
