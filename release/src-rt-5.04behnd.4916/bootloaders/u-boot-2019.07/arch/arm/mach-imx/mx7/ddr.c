// SPDX-License-Identifier: GPL-2.0+
/*
 * DDR controller configuration for the i.MX7 architecture
 *
 * (C) Copyright 2017 CompuLab, Ltd. http://www.compulab.com
 *
 * Author: Uri Mashiach <uri.mashiach@compulab.co.il>
 */

#include <linux/types.h>
#include <asm/io.h>
#include <asm/arch/imx-regs.h>
#include <asm/arch/crm_regs.h>
#include <asm/arch/mx7-ddr.h>
#include <common.h>

/*
 * Routine: mx7_dram_cfg
 * Description: DDR controller configuration
 *
 * @ddrc_regs_val: DDRC registers value
 * @ddrc_mp_val: DDRC_MP registers value
 * @ddr_phy_regs_val: DDR_PHY registers value
 * @calib_param: calibration parameters
 *
 */
void mx7_dram_cfg(struct ddrc *ddrc_regs_val, struct ddrc_mp *ddrc_mp_val,
		  struct ddr_phy *ddr_phy_regs_val,
		  struct mx7_calibration *calib_param)
{
	struct src *const src_regs = (struct src *)SRC_BASE_ADDR;
	struct ddrc *const ddrc_regs = (struct ddrc *)DDRC_IPS_BASE_ADDR;
	struct ddrc_mp *const ddrc_mp_reg = (struct ddrc_mp *)DDRC_MP_BASE_ADDR;
	struct ddr_phy *const ddr_phy_regs =
		(struct ddr_phy *)DDRPHY_IPS_BASE_ADDR;
	struct iomuxc_gpr_base_regs *const iomuxc_gpr_regs =
		(struct iomuxc_gpr_base_regs *)IOMUXC_GPR_BASE_ADDR;
	int i;

	/* Assert DDR Controller preset and DDR PHY reset */
	writel(SRC_DDRC_RCR_DDRC_CORE_RST_MASK, &src_regs->ddrc_rcr);

	/* DDR controller configuration */
	writel(ddrc_regs_val->mstr, &ddrc_regs->mstr);
	writel(ddrc_regs_val->rfshtmg, &ddrc_regs->rfshtmg);
	writel(ddrc_mp_val->pctrl_0, &ddrc_mp_reg->pctrl_0);
	writel(ddrc_regs_val->init1, &ddrc_regs->init1);
	writel(ddrc_regs_val->init0, &ddrc_regs->init0);
	writel(ddrc_regs_val->init3, &ddrc_regs->init3);
	writel(ddrc_regs_val->init4, &ddrc_regs->init4);
	writel(ddrc_regs_val->init5, &ddrc_regs->init5);
	writel(ddrc_regs_val->rankctl, &ddrc_regs->rankctl);
	writel(ddrc_regs_val->dramtmg0, &ddrc_regs->dramtmg0);
	writel(ddrc_regs_val->dramtmg1, &ddrc_regs->dramtmg1);
	writel(ddrc_regs_val->dramtmg2, &ddrc_regs->dramtmg2);
	writel(ddrc_regs_val->dramtmg3, &ddrc_regs->dramtmg3);
	writel(ddrc_regs_val->dramtmg4, &ddrc_regs->dramtmg4);
	writel(ddrc_regs_val->dramtmg5, &ddrc_regs->dramtmg5);
	writel(ddrc_regs_val->dramtmg8, &ddrc_regs->dramtmg8);
	writel(ddrc_regs_val->zqctl0, &ddrc_regs->zqctl0);
	writel(ddrc_regs_val->dfitmg0, &ddrc_regs->dfitmg0);
	writel(ddrc_regs_val->dfitmg1, &ddrc_regs->dfitmg1);
	writel(ddrc_regs_val->dfiupd0, &ddrc_regs->dfiupd0);
	writel(ddrc_regs_val->dfiupd1, &ddrc_regs->dfiupd1);
	writel(ddrc_regs_val->dfiupd2, &ddrc_regs->dfiupd2);
	writel(ddrc_regs_val->addrmap0, &ddrc_regs->addrmap0);
	writel(ddrc_regs_val->addrmap1, &ddrc_regs->addrmap1);
	writel(ddrc_regs_val->addrmap4, &ddrc_regs->addrmap4);
	writel(ddrc_regs_val->addrmap5, &ddrc_regs->addrmap5);
	writel(ddrc_regs_val->addrmap6, &ddrc_regs->addrmap6);
	writel(ddrc_regs_val->odtcfg, &ddrc_regs->odtcfg);
	writel(ddrc_regs_val->odtmap, &ddrc_regs->odtmap);

	/* De-assert DDR Controller preset and DDR PHY reset */
	clrbits_le32(&src_regs->ddrc_rcr, SRC_DDRC_RCR_DDRC_CORE_RST_MASK);

	/* PHY configuration */
	writel(ddr_phy_regs_val->phy_con0, &ddr_phy_regs->phy_con0);
	writel(ddr_phy_regs_val->phy_con1, &ddr_phy_regs->phy_con1);
	writel(ddr_phy_regs_val->phy_con4, &ddr_phy_regs->phy_con4);
	writel(ddr_phy_regs_val->mdll_con0, &ddr_phy_regs->mdll_con0);
	writel(ddr_phy_regs_val->drvds_con0, &ddr_phy_regs->drvds_con0);
	writel(ddr_phy_regs_val->offset_wr_con0, &ddr_phy_regs->offset_wr_con0);
	writel(ddr_phy_regs_val->offset_rd_con0, &ddr_phy_regs->offset_rd_con0);
	writel(ddr_phy_regs_val->cmd_sdll_con0 |
	       DDR_PHY_CMD_SDLL_CON0_CTRL_RESYNC_MASK,
	       &ddr_phy_regs->cmd_sdll_con0);
	writel(ddr_phy_regs_val->cmd_sdll_con0 &
	       ~DDR_PHY_CMD_SDLL_CON0_CTRL_RESYNC_MASK,
	       &ddr_phy_regs->cmd_sdll_con0);
	writel(ddr_phy_regs_val->offset_lp_con0, &ddr_phy_regs->offset_lp_con0);

	/* calibration */
	for (i = 0; i < calib_param->num_val; i++)
		writel(calib_param->values[i], &ddr_phy_regs->zq_con0);

	/* Wake_up DDR PHY */
	HW_CCM_CCGR_WR(CCGR_IDX_DDR, CCM_CLK_ON_N_N);
	writel(IOMUXC_GPR_GPR8_ddr_phy_ctrl_wake_up(0xf) |
	       IOMUXC_GPR_GPR8_ddr_phy_dfi_init_start_MASK,
	       &iomuxc_gpr_regs->gpr[8]);
	HW_CCM_CCGR_WR(CCGR_IDX_DDR, CCM_CLK_ON_R_W);
}

/*
 * Routine: imx_ddr_size
 * Description: extract the current DRAM size from the DDRC registers
 *
 * @return: DRAM size
 */
unsigned int imx_ddr_size(void)
{
	struct ddrc *const ddrc_regs = (struct ddrc *)DDRC_IPS_BASE_ADDR;
	u32 reg_val, field_val;
	int bits = 0;/* Number of address bits */

	/* Count data bus width bits */
	reg_val = readl(&ddrc_regs->mstr);
	field_val = (reg_val & MSTR_DATA_BUS_WIDTH_MASK) >> MSTR_DATA_BUS_WIDTH_SHIFT;
	bits += 2 - field_val;
	/* Count rank address bits */
	field_val = (reg_val & MSTR_DATA_ACTIVE_RANKS_MASK) >> MSTR_DATA_ACTIVE_RANKS_SHIFT;
	if (field_val > 1)
		bits += field_val - 1;
	/* Count column address bits */
	bits += 2;/* Column address 0 and 1 are fixed mapped */
	reg_val = readl(&ddrc_regs->addrmap2);
	field_val = (reg_val & ADDRMAP2_COL_B2_MASK) >> ADDRMAP2_COL_B2_SHIFT;
	if (field_val <= 7)
		bits++;
	field_val = (reg_val & ADDRMAP2_COL_B3_MASK) >> ADDRMAP2_COL_B3_SHIFT;
	if (field_val <= 7)
		bits++;
	field_val = (reg_val & ADDRMAP2_COL_B4_MASK) >> ADDRMAP2_COL_B4_SHIFT;
	if (field_val <= 7)
		bits++;
	field_val = (reg_val & ADDRMAP2_COL_B5_MASK) >> ADDRMAP2_COL_B5_SHIFT;
	if (field_val <= 7)
		bits++;
	reg_val = readl(&ddrc_regs->addrmap3);
	field_val = (reg_val & ADDRMAP3_COL_B6_MASK) >> ADDRMAP3_COL_B6_SHIFT;
	if (field_val <= 7)
		bits++;
	field_val = (reg_val & ADDRMAP3_COL_B7_MASK) >> ADDRMAP3_COL_B7_SHIFT;
	if (field_val <= 7)
		bits++;
	field_val = (reg_val & ADDRMAP3_COL_B8_MASK) >> ADDRMAP3_COL_B8_SHIFT;
	if (field_val <= 7)
		bits++;
	field_val = (reg_val & ADDRMAP3_COL_B9_MASK) >> ADDRMAP3_COL_B9_SHIFT;
	if (field_val <= 7)
		bits++;
	reg_val = readl(&ddrc_regs->addrmap4);
	field_val = (reg_val & ADDRMAP4_COL_B10_MASK) >> ADDRMAP4_COL_B10_SHIFT;
	if (field_val <= 7)
		bits++;
	field_val = (reg_val & ADDRMAP4_COL_B11_MASK) >> ADDRMAP4_COL_B11_SHIFT;
	if (field_val <= 7)
		bits++;
	/* Count row address bits */
	reg_val = readl(&ddrc_regs->addrmap5);
	field_val = (reg_val & ADDRMAP5_ROW_B0_MASK) >> ADDRMAP5_ROW_B0_SHIFT;
	if (field_val <= 11)
		bits++;
	field_val = (reg_val & ADDRMAP5_ROW_B1_MASK) >> ADDRMAP5_ROW_B1_SHIFT;
	if (field_val <= 11)
		bits++;
	field_val = (reg_val & ADDRMAP5_ROW_B2_10_MASK) >> ADDRMAP5_ROW_B2_10_SHIFT;
	if (field_val <= 11)
		bits += 9;
	field_val = (reg_val & ADDRMAP5_ROW_B11_MASK) >> ADDRMAP5_ROW_B11_SHIFT;
	if (field_val <= 11)
		bits++;
	reg_val = readl(&ddrc_regs->addrmap6);
	field_val = (reg_val & ADDRMAP6_ROW_B12_MASK) >> ADDRMAP6_ROW_B12_SHIFT;
	if (field_val <= 11)
		bits++;
	field_val = (reg_val & ADDRMAP6_ROW_B13_MASK) >> ADDRMAP6_ROW_B13_SHIFT;
	if (field_val <= 11)
		bits++;
	field_val = (reg_val & ADDRMAP6_ROW_B14_MASK) >> ADDRMAP6_ROW_B14_SHIFT;
	if (field_val <= 11)
		bits++;
	field_val = (reg_val & ADDRMAP6_ROW_B15_MASK) >> ADDRMAP6_ROW_B15_SHIFT;
	if (field_val <= 11)
		bits++;
	/* Count bank bits */
	reg_val = readl(&ddrc_regs->addrmap1);
	field_val = (reg_val & ADDRMAP1_BANK_B0_MASK) >> ADDRMAP1_BANK_B0_SHIFT;
	if (field_val <= 30)
		bits++;
	field_val = (reg_val & ADDRMAP1_BANK_B1_MASK) >> ADDRMAP1_BANK_B1_SHIFT;
	if (field_val <= 30)
		bits++;
	field_val = (reg_val & ADDRMAP1_BANK_B2_MASK) >> ADDRMAP1_BANK_B2_SHIFT;
	if (field_val <= 29)
		bits++;

	/* cap to max 2 GB */
	if (bits > 31)
		bits = 31;

	return 1 << bits;
}
