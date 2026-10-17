// SPDX-License-Identifier: GPL-2.0+ OR MIT
/*
 * Copyright 2021 NXP
 */
#include <common.h>
#include <asm/io.h>
#include <asm/arch/clock.h>
#include <asm/arch/ddr.h>
#include <asm/arch/imx-regs.h>

#define DENALI_CTL_00		(DDR_CTL_BASE_ADDR + 4 * 0)
#define CTL_START		0x1

#define DENALI_CTL_03		(DDR_CTL_BASE_ADDR + 4 * 3)
#define DENALI_CTL_197		(DDR_CTL_BASE_ADDR + 4 * 197)
#define DENALI_CTL_250		(DDR_CTL_BASE_ADDR + 4 * 250)
#define DENALI_CTL_251		(DDR_CTL_BASE_ADDR + 4 * 251)
#define DENALI_CTL_266		(DDR_CTL_BASE_ADDR + 4 * 266)
#define DFI_INIT_COMPLETE	0x2

#define DENALI_CTL_614		(DDR_CTL_BASE_ADDR + 4 * 614)
#define DENALI_CTL_615		(DDR_CTL_BASE_ADDR + 4 * 615)

#define DENALI_PI_00		(DDR_PI_BASE_ADDR + 4 * 0)
#define PI_START		0x1

#define DENALI_PI_04		(DDR_PI_BASE_ADDR + 4 * 4)
#define DENALI_PI_11		(DDR_PI_BASE_ADDR + 4 * 11)
#define DENALI_PI_12		(DDR_PI_BASE_ADDR + 4 * 12)
#define DENALI_CTL_23		(DDR_CTL_BASE_ADDR + 4 * 23)
#define DENALI_CTL_25		(DDR_CTL_BASE_ADDR + 4 * 25)

#define DENALI_PHY_1624		(DDR_PHY_BASE_ADDR + 4 * 1624)
#define DENALI_PHY_1625  	(DDR_PHY_BASE_ADDR + 4 * 1625)
#define DENALI_PHY_1537		(DDR_PHY_BASE_ADDR + 4 * 1537)
#define PHY_FREQ_SEL_MULTICAST_EN(X)	((X) << 8)
#define PHY_FREQ_SEL_INDEX(X)		((X) << 16)

#define DENALI_PHY_1547		(DDR_PHY_BASE_ADDR + 4 * 1547)
#define DENALI_PHY_1555		(DDR_PHY_BASE_ADDR + 4 * 1555)
#define DENALI_PHY_1564		(DDR_PHY_BASE_ADDR + 4 * 1564)
#define DENALI_PHY_1565		(DDR_PHY_BASE_ADDR + 4 * 1565)

static void ddr_enable_pll_bypass(void)
{
	u32 reg_val;

	/* PI_INIT_LVL_EN=0x0  (DENALI_PI_04) */
	reg_val = readl(DENALI_PI_04) & ~0x1;
	writel(reg_val, DENALI_PI_04);

	/* PI_FREQ_MAP=0x1  (DENALI_PI_12) */
	writel(0x1, DENALI_PI_12);

	/* PI_INIT_WORK_FREQ=0x0  (DENALI_PI_11) */
	reg_val = readl(DENALI_PI_11) & ~(0x1f << 8);
	writel(reg_val, DENALI_PI_11);

	/* DFIBUS_FREQ_INIT=0x0  (DENALI_CTL_23) */
	reg_val = readl(DENALI_CTL_23) & ~(0x3 << 24);
	writel(reg_val, DENALI_CTL_23);

	/* PHY_LP4_BOOT_DISABLE=0x0 (DENALI_PHY_1547) */
	reg_val = readl(DENALI_PHY_1547) & ~(0x1 << 8);
	writel(reg_val, DENALI_PHY_1547);

	/* PHY_PLL_BYPASS=0x1 (DENALI_PHY_1624) */
	reg_val = readl(DENALI_PHY_1624) | 0x1;
	writel(reg_val, DENALI_PHY_1624);

	/* PHY_LP4_BOOT_PLL_BYPASS to 0x1 (DENALI_PHY_1555) */
	reg_val = readl(DENALI_PHY_1555) | 0x1;
	writel(reg_val, DENALI_PHY_1555);

	/* FREQ_CHANGE_TYPE_F0 = 0x0/FREQ_CHANGE_TYPE_F1 = 0x1/FREQ_CHANGE_TYPE_F2 = 0x2 */
	reg_val = 0x020100;
	writel(reg_val, DENALI_CTL_25);
}

int ddr_calibration(unsigned int fsp_table[3])
{
	u32 reg_val;
	u32 int_status_init, phy_freq_req, phy_freq_type;
	u32 lock_0, lock_1, lock_2;
	u32 freq_chg_pt, freq_chg_cnt;
	u32 is_lpddr4 = 0;

	if (IS_ENABLED(CONFIG_IMX8ULP_DRAM_PHY_PLL_BYPASS)) {
		ddr_enable_pll_bypass();
		freq_chg_cnt = 0;
		freq_chg_pt = 0;
	} else {
		reg_val = (readl(DENALI_CTL_00)>>8)&0xf;
		if(reg_val == 0x7) {
			/* LPDDR3 type */
			set_ddr_clk(fsp_table[1] >> 1);
			freq_chg_cnt = 0;
			freq_chg_pt = 0;
		} else if(reg_val == 0xb) {
			/* LPDDR4/4x type */
			is_lpddr4 = 1;
			reg_val = readl(DENALI_CTL_250);
			if (((reg_val >> 16) & 0x3) == 1)
				freq_chg_cnt = 2;
			else
				freq_chg_cnt = 3;

			reg_val = readl(DENALI_PI_12);
			if(reg_val == 0x3)
				freq_chg_pt = 1;
			else if(reg_val == 0x7)
				freq_chg_pt = 2;
			else {
				printf("frequency map(0x%x) is wrong, please check!\r\n", reg_val);
				return -1;
			}
		} else {
			printf("Incorrect DDR type configured!\r\n");
			return -1;
		}
	}

	/* Assert PI_START parameter and then assert START parameter in Controller. */
	reg_val = readl(DENALI_PI_00) | PI_START;
	writel(reg_val, DENALI_PI_00);

	reg_val = readl(DENALI_CTL_00) | CTL_START;
	writel(reg_val, DENALI_CTL_00);

	/* Poll for init_done_bit in Controller interrupt status register (INT_STATUS_INIT) */
	do {
		if (!freq_chg_cnt) {
			int_status_init = (readl(DENALI_CTL_266) >> 8) & 0xff;
			/* DDR subsystem is ready for traffic. */
			if (int_status_init & DFI_INIT_COMPLETE) {
				debug("complete\n");
				break;
			}
		}

		/*
		 * During leveling, PHY will request for freq change and SoC clock logic
		 * should provide requested frequency
		 * Polling SIM LPDDR_CTRL2 Bit phy_freq_chg_req until be 1'b1
		 */
		reg_val = readl(AVD_SIM_LPDDR_CTRL2);
		/* DFS interrupt is set */
		phy_freq_req = ((reg_val >> 7) & 0x1) && ((reg_val >> 15) & 0x1);
		if (phy_freq_req) {
			phy_freq_type = reg_val & 0x1F;
			if (phy_freq_type == 0x00) {
				debug("Poll for freq_chg_req on SIM register and change to F0 frequency.\n");
				set_ddr_clk(fsp_table[phy_freq_type] >> 1);

				/* Write 1'b1 at LPDDR_CTRL2 bit phy_freq_cfg_ack */
				reg_val = readl(AVD_SIM_LPDDR_CTRL2);
				writel(reg_val | (0x1 << 6), AVD_SIM_LPDDR_CTRL2);
			} else if (phy_freq_type == 0x01) {
				debug("Poll for freq_chg_req on SIM register and change to F1 frequency.\n");
				set_ddr_clk(fsp_table[phy_freq_type] >> 1);

				/* Write 1'b1 at LPDDR_CTRL2 bit phy_freq_cfg_ack */
				reg_val = readl(AVD_SIM_LPDDR_CTRL2);
				writel(reg_val | (0x1 << 6), AVD_SIM_LPDDR_CTRL2);
				if (freq_chg_pt == 1)
					freq_chg_cnt--;
			} else if (phy_freq_type == 0x02) {
				debug("Poll for freq_chg_req on SIM register and change to F2 frequency.\n");
				set_ddr_clk(fsp_table[phy_freq_type] >> 1);

				/* Write 1'b1 at LPDDR_CTRL2 bit phy_freq_cfg_ack */
				reg_val = readl(AVD_SIM_LPDDR_CTRL2);
				writel(reg_val | (0x1 << 6), AVD_SIM_LPDDR_CTRL2);
				if (freq_chg_pt == 2)
					freq_chg_cnt--;
			}

			/* Hardware clear the ack on falling edge of LPDDR_CTRL2:phy_freq_chg_reg */
			/* Ensure the ack is clear before starting to poll request again */
			while ((readl(AVD_SIM_LPDDR_CTRL2) & BIT(6)))
				;
		}
	} while (1);

	/* Check PLL lock status */
	lock_0 = readl(DENALI_PHY_1564) & 0xffff;
	lock_1 = (readl(DENALI_PHY_1564) >> 16) & 0xffff;
	lock_2 = readl(DENALI_PHY_1565) & 0xffff;

	if ((lock_0 & 0x3) != 0x3 || (lock_1 & 0x3) != 0x3 || (lock_2 & 0x3) != 0x3) {
		debug("De-Skew PLL failed to lock\n");
		debug("lock_0=0x%x, lock_1=0x%x, lock_2=0x%x\n", lock_0, lock_1, lock_2);
		return -1;
	}

	debug("De-Skew PLL is locked and ready\n");

	/* Change LPDDR4 FREQ1 to bypass mode if it is lower than 200MHz */
	if(is_lpddr4 && fsp_table[1] < 400) {
		/* Set FREQ1 to bypass mode */
		reg_val = PHY_FREQ_SEL_MULTICAST_EN(0) | PHY_FREQ_SEL_INDEX(0);
		writel(reg_val, DENALI_PHY_1537);

		/* PHY_PLL_BYPASS=0x1 (DENALI_PHY_1624) */
		reg_val =readl(DENALI_PHY_1624) | 0x1;
		writel(reg_val, DENALI_PHY_1624);

		/* DENALI_PHY_1625: bypass mode in PHY PLL */
		reg_val =readl(DENALI_PHY_1625) & ~0xf;
		writel(reg_val, DENALI_PHY_1625);
	}

	return 0;
}

static void save_dram_config(struct dram_timing_info2 *timing_info, unsigned long saved_timing_base)
{
	int i = 0;
	struct dram_timing_info2 *saved_timing = (struct dram_timing_info2 *)saved_timing_base;
	struct dram_cfg_param *cfg;

	saved_timing->ctl_cfg_num = timing_info->ctl_cfg_num;
	saved_timing->phy_f1_cfg_num = timing_info->phy_f1_cfg_num;
	saved_timing->phy_f2_cfg_num = timing_info->phy_f2_cfg_num;

	/* save the fsp table */
	for (i = 0; i < 3; i++)
		saved_timing->fsp_table[i] = timing_info->fsp_table[i];

	cfg = (struct dram_cfg_param *)(saved_timing_base +
					sizeof(*timing_info));

	/* save ctl config */
	saved_timing->ctl_cfg = cfg;
	for (i = 0; i < timing_info->ctl_cfg_num; i++) {
		cfg->reg = timing_info->ctl_cfg[i].reg;
		cfg->val = timing_info->ctl_cfg[i].val;
		cfg++;
	}

	/* save phy f1 config */
	saved_timing->phy_f1_cfg = cfg;
	for (i = 0; i < timing_info->phy_f1_cfg_num; i++) {
		cfg->reg = timing_info->phy_f1_cfg[i].reg;
		cfg->val = timing_info->phy_f1_cfg[i].val;
		cfg++;
	}

	/* save phy f2 config */
	saved_timing->phy_f2_cfg = cfg;
	for (i = 0; i < timing_info->phy_f2_cfg_num; i++) {
		cfg->reg = timing_info->phy_f2_cfg[i].reg;
		cfg->val = timing_info->phy_f2_cfg[i].val;
		cfg++;
	}
}

int ddr_init(struct dram_timing_info2 *dram_timing)
{
	int i;

	if (IS_ENABLED(CONFIG_IMX8ULP_DRAM_PHY_PLL_BYPASS)) {
		/* Use PLL bypass for boot freq */
		/* Since PLL can't generate the double freq, Need ddr clock to generate it. */
		set_ddr_clk(dram_timing->fsp_table[0]); /* Set to boot freq */
		setbits_le32(AVD_SIM_BASE_ADDR, 0x1); /* SIM_DDR_CTRL_DIV2_EN */
	} else {
		set_ddr_clk(dram_timing->fsp_table[0] >> 1); /* Set to boot freq */
		clrbits_le32(AVD_SIM_BASE_ADDR, 0x1); /* SIM_DDR_CTRL_DIV2_EN */
	}

	/* save the dram config into sram for low power mode */
	save_dram_config(dram_timing, CONFIG_SAVED_DRAM_TIMING_BASE);

	/* Initialize CTL registers */
	for (i = 0; i < dram_timing->ctl_cfg_num; i++)
		writel(dram_timing->ctl_cfg[i].val, (ulong)dram_timing->ctl_cfg[i].reg);

	/* Initialize PI registers */
	for (i = 0; i < dram_timing->pi_cfg_num; i++)
		writel(dram_timing->pi_cfg[i].val, (ulong)dram_timing->pi_cfg[i].reg);

	/* Write PHY regiters for all 3 frequency points (48Mhz/384Mhz/528Mhz): f1_index=0 */
	writel(PHY_FREQ_SEL_MULTICAST_EN(1) | PHY_FREQ_SEL_INDEX(0), DENALI_PHY_1537);
	for (i = 0; i < dram_timing->phy_f1_cfg_num; i++)
		writel(dram_timing->phy_f1_cfg[i].val, (ulong)dram_timing->phy_f1_cfg[i].reg);

	/* Write PHY regiters for freqency point 2 (528Mhz): f2_index=1 */
	writel(PHY_FREQ_SEL_MULTICAST_EN(0) | PHY_FREQ_SEL_INDEX(1), DENALI_PHY_1537);
	for (i = 0; i < dram_timing->phy_f2_cfg_num; i++)
		writel(dram_timing->phy_f2_cfg[i].val, (ulong)dram_timing->phy_f2_cfg[i].reg);

	/* Re-enable MULTICAST mode */
	writel(PHY_FREQ_SEL_MULTICAST_EN(1) | PHY_FREQ_SEL_INDEX(0), DENALI_PHY_1537);

	return ddr_calibration(dram_timing->fsp_table);
}
