// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2015-2016 Marvell International Ltd.
 */

#include <common.h>
#include <fdt_support.h>
#include <log.h>
#include <asm/global_data.h>
#include <asm/io.h>
#include <asm/arch/cpu.h>
#include <asm/arch/soc.h>
#include <linux/delay.h>
#include <phy.h>

#include "comphy_a3700.h"

DECLARE_GLOBAL_DATA_PTR;

struct comphy_mux_data a3700_comphy_mux_data[] = {
	/* Lane 0 */
	{
		4,
		{
			{ COMPHY_TYPE_UNCONNECTED,	0x0 },
			{ COMPHY_TYPE_SGMII1,	0x0 },
			{ COMPHY_TYPE_USB3_HOST0,	0x1 },
			{ COMPHY_TYPE_USB3_DEVICE,	0x1 }
		}
	},
	/* Lane 1 */
	{
		3,
		{
			{ COMPHY_TYPE_UNCONNECTED,	0x0},
			{ COMPHY_TYPE_SGMII0,	0x0},
			{ COMPHY_TYPE_PEX0,	0x1}
		}
	},
	/* Lane 2 */
	{
		4,
		{
			{ COMPHY_TYPE_UNCONNECTED,	0x0},
			{ COMPHY_TYPE_SATA0,	0x0},
			{ COMPHY_TYPE_USB3_HOST0,	0x1},
			{ COMPHY_TYPE_USB3_DEVICE,	0x1}
		}
	},
};

struct sgmii_phy_init_data_fix {
	u16 addr;
	u16 value;
};

/* Changes to 40M1G25 mode data required for running 40M3G125 init mode */
static struct sgmii_phy_init_data_fix sgmii_phy_init_fix[] = {
	{0x005, 0x07CC}, {0x015, 0x0000}, {0x01B, 0x0000}, {0x01D, 0x0000},
	{0x01E, 0x0000}, {0x01F, 0x0000}, {0x020, 0x0000}, {0x021, 0x0030},
	{0x026, 0x0888}, {0x04D, 0x0152}, {0x04F, 0xA020}, {0x050, 0x07CC},
	{0x053, 0xE9CA}, {0x055, 0xBD97}, {0x071, 0x3015}, {0x076, 0x03AA},
	{0x07C, 0x0FDF}, {0x0C2, 0x3030}, {0x0C3, 0x8000}, {0x0E2, 0x5550},
	{0x0E3, 0x12A4}, {0x0E4, 0x7D00}, {0x0E6, 0x0C83}, {0x101, 0xFCC0},
	{0x104, 0x0C10}
};

/* 40M1G25 mode init data */
static u16 sgmii_phy_init[512] = {
	/* 0       1       2       3       4       5       6       7 */
	/*-----------------------------------------------------------*/
	/* 8       9       A       B       C       D       E       F */
	0x3110, 0xFD83, 0x6430, 0x412F, 0x82C0, 0x06FA, 0x4500, 0x6D26,	/* 00 */
	0xAFC0, 0x8000, 0xC000, 0x0000, 0x2000, 0x49CC, 0x0BC9, 0x2A52,	/* 08 */
	0x0BD2, 0x0CDE, 0x13D2, 0x0CE8, 0x1149, 0x10E0, 0x0000, 0x0000,	/* 10 */
	0x0000, 0x0000, 0x0000, 0x0001, 0x0000, 0x4134, 0x0D2D, 0xFFFF,	/* 18 */
	0xFFE0, 0x4030, 0x1016, 0x0030, 0x0000, 0x0800, 0x0866, 0x0000,	/* 20 */
	0x0000, 0x0000, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF,	/* 28 */
	0xFFFF, 0xFFFF, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,	/* 30 */
	0x0000, 0x0000, 0x000F, 0x6A62, 0x1988, 0x3100, 0x3100, 0x3100,	/* 38 */
	0x3100, 0xA708, 0x2430, 0x0830, 0x1030, 0x4610, 0xFF00, 0xFF00,	/* 40 */
	0x0060, 0x1000, 0x0400, 0x0040, 0x00F0, 0x0155, 0x1100, 0xA02A,	/* 48 */
	0x06FA, 0x0080, 0xB008, 0xE3ED, 0x5002, 0xB592, 0x7A80, 0x0001,	/* 50 */
	0x020A, 0x8820, 0x6014, 0x8054, 0xACAA, 0xFC88, 0x2A02, 0x45CF,	/* 58 */
	0x000F, 0x1817, 0x2860, 0x064F, 0x0000, 0x0204, 0x1800, 0x6000,	/* 60 */
	0x810F, 0x4F23, 0x4000, 0x4498, 0x0850, 0x0000, 0x000E, 0x1002,	/* 68 */
	0x9D3A, 0x3009, 0xD066, 0x0491, 0x0001, 0x6AB0, 0x0399, 0x3780,	/* 70 */
	0x0040, 0x5AC0, 0x4A80, 0x0000, 0x01DF, 0x0000, 0x0007, 0x0000,	/* 78 */
	0x2D54, 0x00A1, 0x4000, 0x0100, 0xA20A, 0x0000, 0x0000, 0x0000,	/* 80 */
	0x0000, 0x0000, 0x0000, 0x7400, 0x0E81, 0x1000, 0x1242, 0x0210,	/* 88 */
	0x80DF, 0x0F1F, 0x2F3F, 0x4F5F, 0x6F7F, 0x0F1F, 0x2F3F, 0x4F5F,	/* 90 */
	0x6F7F, 0x4BAD, 0x0000, 0x0000, 0x0800, 0x0000, 0x2400, 0xB651,	/* 98 */
	0xC9E0, 0x4247, 0x0A24, 0x0000, 0xAF19, 0x1004, 0x0000, 0x0000,	/* A0 */
	0x0000, 0x0013, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,	/* A8 */
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,	/* B0 */
	0x0000, 0x0000, 0x0000, 0x0060, 0x0000, 0x0000, 0x0000, 0x0000,	/* B8 */
	0x0000, 0x0000, 0x3010, 0xFA00, 0x0000, 0x0000, 0x0000, 0x0003,	/* C0 */
	0x1618, 0x8200, 0x8000, 0x0400, 0x050F, 0x0000, 0x0000, 0x0000,	/* C8 */
	0x4C93, 0x0000, 0x1000, 0x1120, 0x0010, 0x1242, 0x1242, 0x1E00,	/* D0 */
	0x0000, 0x0000, 0x0000, 0x00F8, 0x0000, 0x0041, 0x0800, 0x0000,	/* D8 */
	0x82A0, 0x572E, 0x2490, 0x14A9, 0x4E00, 0x0000, 0x0803, 0x0541,	/* E0 */
	0x0C15, 0x0000, 0x0000, 0x0400, 0x2626, 0x0000, 0x0000, 0x4200,	/* E8 */
	0x0000, 0xAA55, 0x1020, 0x0000, 0x0000, 0x5010, 0x0000, 0x0000,	/* F0 */
	0x0000, 0x0000, 0x5000, 0x0000, 0x0000, 0x0000, 0x02F2, 0x0000,	/* F8 */
	0x101F, 0xFDC0, 0x4000, 0x8010, 0x0110, 0x0006, 0x0000, 0x0000,	/*100 */
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,	/*108 */
	0x04CF, 0x0000, 0x04CF, 0x0000, 0x04CF, 0x0000, 0x04C6, 0x0000,	/*110 */
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,	/*118 */
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,	/*120 */
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,	/*128 */
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,	/*130 */
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,	/*138 */
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,	/*140 */
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,	/*148 */
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,	/*150 */
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,	/*158 */
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,	/*160 */
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,	/*168 */
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,	/*170 */
	0x0000, 0x0000, 0x0000, 0x00F0, 0x08A2, 0x3112, 0x0A14, 0x0000,	/*178 */
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,	/*180 */
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,	/*188 */
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,	/*190 */
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,	/*198 */
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,	/*1A0 */
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,	/*1A8 */
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,	/*1B0 */
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,	/*1B8 */
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,	/*1C0 */
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,	/*1C8 */
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,	/*1D0 */
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,	/*1D8 */
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,	/*1E0 */
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,	/*1E8 */
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,	/*1F0 */
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000	/*1F8 */
};

/*
 * comphy_poll_reg
 *
 * return: 1 on success, 0 on timeout
 */
static u32 comphy_poll_reg(void *addr, u32 val, u32 mask, u8 op_type)
{
	u32 rval = 0xDEAD, timeout;

	for (timeout = PLL_LOCK_TIMEOUT; timeout > 0; timeout--) {
		if (op_type == POLL_16B_REG)
			rval = readw(addr);	/* 16 bit */
		else
			rval = readl(addr) ;	/* 32 bit */

		if ((rval & mask) == val)
			return 1;

		udelay(10000);
	}

	debug("Time out waiting (%p = %#010x)\n", addr, rval);
	return 0;
}

/*
 * comphy_pcie_power_up
 *
 * return: 1 if PLL locked (OK), 0 otherwise (FAIL)
 */
static int comphy_pcie_power_up(u32 speed, u32 invert)
{
	int ret;

	debug_enter();

	/*
	 * 1. Enable max PLL.
	 */
	reg_set16(phy_addr(PCIE, LANE_CFG1), bf_use_max_pll_rate, 0);

	/*
	 * 2. Select 20 bit SERDES interface.
	 */
	reg_set16(phy_addr(PCIE, GLOB_CLK_SRC_LO), bf_cfg_sel_20b, 0);

	/*
	 * 3. Force to use reg setting for PCIe mode
	 */
	reg_set16(phy_addr(PCIE, MISC_REG1), bf_sel_bits_pcie_force, 0);

	/*
	 * 4. Change RX wait
	 */
	reg_set16(phy_addr(PCIE, PWR_MGM_TIM1), 0x10C, 0xFFFF);

	/*
	 * 5. Enable idle sync
	 */
	reg_set16(phy_addr(PCIE, UNIT_CTRL), 0x60 | rb_idle_sync_en, 0xFFFF);

	/*
	 * 6. Enable the output of 100M/125M/500M clock
	 */
	reg_set16(phy_addr(PCIE, MISC_REG0),
		  0xA00D | rb_clk500m_en | rb_txdclk_2x_sel | rb_clk100m_125m_en, 0xFFFF);

	/*
	 * 7. Enable TX
	 */
	reg_set(PCIE_REF_CLK_ADDR, 0x1342, 0xFFFFFFFF);

	/*
	 * 8. Check crystal jumper setting and program the Power and PLL
	 *    Control accordingly
	 */
	if (get_ref_clk() == 40) {
		/* 40 MHz */
		reg_set16(phy_addr(PCIE, PWR_PLL_CTRL), 0xFC63, 0xFFFF);
	} else {
		/* 25 MHz */
		reg_set16(phy_addr(PCIE, PWR_PLL_CTRL), 0xFC62, 0xFFFF);
	}

	/*
	 * 9. Override Speed_PLL value and use MAC PLL
	 */
	reg_set16(phy_addr(PCIE, KVCO_CAL_CTRL), 0x0040 | rb_use_max_pll_rate,
		  0xFFFF);

	/*
	 * 10. Check the Polarity invert bit
	 */
	if (invert & COMPHY_POLARITY_TXD_INVERT)
		reg_set16(phy_addr(PCIE, SYNC_PATTERN), phy_txd_inv, 0);
	else
		reg_set16(phy_addr(PCIE, SYNC_PATTERN), 0, phy_txd_inv);

	if (invert & COMPHY_POLARITY_RXD_INVERT)
		reg_set16(phy_addr(PCIE, SYNC_PATTERN), phy_rxd_inv, 0);
	else
		reg_set16(phy_addr(PCIE, SYNC_PATTERN), 0, phy_rxd_inv);

	/*
	 * 11. Release SW reset
	 */
	reg_set16(phy_addr(PCIE, GLOB_PHY_CTRL0),
		  rb_mode_core_clk_freq_sel | rb_mode_pipe_width_32,
		  bf_soft_rst | bf_mode_refdiv);

	/* Wait for > 55 us to allow PCLK be enabled */
	udelay(PLL_SET_DELAY_US);

	/* Assert PCLK enabled */
	ret = comphy_poll_reg(phy_addr(PCIE, LANE_STAT1),	/* address */
			      rb_txdclk_pclk_en,		/* value */
			      rb_txdclk_pclk_en,		/* mask */
			      POLL_16B_REG);			/* 16bit */
	if (!ret)
		printf("Failed to lock PCIe PLL\n");

	debug_exit();

	/* Return the status of the PLL */
	return ret;
}

/*
 * reg_set_indirect
 *
 * return: void
 */
static void reg_set_indirect(u32 reg, u16 data, u16 mask)
{
	reg_set(rh_vsreg_addr, reg, 0xFFFFFFFF);
	reg_set(rh_vsreg_data, data, mask);
}

/*
 * comphy_sata_power_up
 *
 * return: 1 if PLL locked (OK), 0 otherwise (FAIL)
 */
static int comphy_sata_power_up(u32 invert)
{
	int ret;
	u32 data = 0;

	debug_enter();

	/*
	 * 0. Check the Polarity invert bits
	 */
	if (invert & COMPHY_POLARITY_TXD_INVERT)
		data |= bs_txd_inv;

	if (invert & COMPHY_POLARITY_RXD_INVERT)
		data |= bs_rxd_inv;

	reg_set_indirect(vphy_sync_pattern_reg, data, bs_txd_inv | bs_rxd_inv);

	/*
	 * 1. Select 40-bit data width width
	 */
	reg_set_indirect(vphy_loopback_reg0, 0x800, bs_phyintf_40bit);

	/*
	 * 2. Select reference clock and PHY mode (SATA)
	 */
	if (get_ref_clk() == 40) {
		/* 40 MHz */
		reg_set_indirect(vphy_power_reg0, 0x3, 0x00FF);
	} else {
		/* 20 MHz */
		reg_set_indirect(vphy_power_reg0, 0x1, 0x00FF);
	}

	/*
	 * 3. Use maximum PLL rate (no power save)
	 */
	reg_set_indirect(vphy_calctl_reg, bs_max_pll_rate, bs_max_pll_rate);

	/*
	 * 4. Reset reserved bit (??)
	 */
	reg_set_indirect(vphy_reserve_reg, 0, bs_phyctrl_frm_pin);

	/*
	 * 5. Set vendor-specific configuration (??)
	 */
	reg_set(rh_vs0_a, vsata_ctrl_reg, 0xFFFFFFFF);
	reg_set(rh_vs0_d, bs_phy_pu_pll, bs_phy_pu_pll);

	/* Wait for > 55 us to allow PLL be enabled */
	udelay(PLL_SET_DELAY_US);

	/* Assert SATA PLL enabled */
	reg_set(rh_vsreg_addr, vphy_loopback_reg0, 0xFFFFFFFF);
	ret = comphy_poll_reg(rh_vsreg_data,	/* address */
			      bs_pll_ready_tx,	/* value */
			      bs_pll_ready_tx,	/* mask */
			      POLL_32B_REG);	/* 32bit */
	if (!ret)
		printf("Failed to lock SATA PLL\n");

	debug_exit();

	return ret;
}

/*
 * usb3_reg_set16
 *
 * return: void
 */
static void usb3_reg_set16(u32 reg, u16 data, u16 mask, u32 lane)
{
	/*
	 * When Lane 2 PHY is for USB3, access the PHY registers
	 * through indirect Address and Data registers INDIR_ACC_PHY_ADDR
	 * (RD00E0178h [31:0]) and INDIR_ACC_PHY_DATA (RD00E017Ch [31:0])
	 * within the SATA Host Controller registers, Lane 2 base register
	 * offset is 0x200
	 */

	if (lane == 2)
		reg_set_indirect(USB3PHY_LANE2_REG_BASE_OFFSET + reg, data,
				 mask);
	else
		reg_set16(phy_addr(USB3, reg), data, mask);
}

/*
 * comphy_usb3_power_up
 *
 * return: 1 if PLL locked (OK), 0 otherwise (FAIL)
 */
static int comphy_usb3_power_up(u32 lane, u32 type, u32 speed, u32 invert)
{
	int ret;

	debug_enter();

	/*
	 * 1. Power up OTG module
	 */
	reg_set(USB2_PHY_OTG_CTRL_ADDR, rb_pu_otg, 0);

	/*
	 * 2. Set counter for 100us pulse in USB3 Host and Device
	 * restore default burst size limit (Reference Clock 31:24)
	 */
	reg_set(USB3_CTRPUL_VAL_REG, 0x8 << 24, rb_usb3_ctr_100ns);


	/* 0xd005c300 = 0x1001 */
	/* set PRD_TXDEEMPH (3.5db de-emph) */
	usb3_reg_set16(LANE_CFG0, 0x1, 0xFF, lane);

	/*
	 * Set BIT0: enable transmitter in high impedance mode
	 * Set BIT[3:4]: delay 2 clock cycles for HiZ off latency
	 * Set BIT6: Tx detect Rx at HiZ mode
	 * Unset BIT15: set to 0 to set USB3 De-emphasize level to -3.5db
	 *              together with bit 0 of COMPHY_REG_LANE_CFG0_ADDR
	 *              register
	 */
	usb3_reg_set16(LANE_CFG1,
		       tx_det_rx_mode | gen2_tx_data_dly_deft
		       | tx_elec_idle_mode_en,
		       prd_txdeemph1_mask | tx_det_rx_mode
		       | gen2_tx_data_dly_mask | tx_elec_idle_mode_en, lane);

	/* 0xd005c310 = 0x93: set Spread Spectrum Clock Enabled */
	usb3_reg_set16(LANE_CFG4, bf_spread_spectrum_clock_en, 0x80, lane);

	/*
	 * set Override Margining Controls From the MAC: Use margining signals
	 * from lane configuration
	 */
	usb3_reg_set16(TEST_MODE_CTRL, rb_mode_margin_override, 0xFFFF, lane);

	/* set Lane-to-Lane Bundle Clock Sampling Period = per PCLK cycles */
	/* set Mode Clock Source = PCLK is generated from REFCLK */
	usb3_reg_set16(GLOB_CLK_SRC_LO, 0x0, 0xFF, lane);

	/* set G2 Spread Spectrum Clock Amplitude at 4K */
	usb3_reg_set16(GEN2_SETTINGS_2, g2_tx_ssc_amp, 0xF000, lane);

	/*
	 * unset G3 Spread Spectrum Clock Amplitude & set G3 TX and RX Register
	 * Master Current Select
	 */
	usb3_reg_set16(GEN2_SETTINGS_3, 0x0, 0xFFFF, lane);

	/*
	 * 3. Check crystal jumper setting and program the Power and PLL
	 * Control accordingly
	 * 4. Change RX wait
	 */
	if (get_ref_clk() == 40) {
		/* 40 MHz */
		usb3_reg_set16(PWR_PLL_CTRL, 0xFCA3, 0xFFFF, lane);
		usb3_reg_set16(PWR_MGM_TIM1, 0x10C, 0xFFFF, lane);
	} else {
		/* 25 MHz */
		usb3_reg_set16(PWR_PLL_CTRL, 0xFCA2, 0xFFFF, lane);
		usb3_reg_set16(PWR_MGM_TIM1, 0x107, 0xFFFF, lane);
	}

	/*
	 * 5. Enable idle sync
	 */
	usb3_reg_set16(UNIT_CTRL, 0x60 | rb_idle_sync_en, 0xFFFF, lane);

	/*
	 * 6. Enable the output of 500M clock
	 */
	usb3_reg_set16(MISC_REG0, 0xA00D | rb_clk500m_en, 0xFFFF, lane);

	/*
	 * 7. Set 20-bit data width
	 */
	usb3_reg_set16(DIG_LB_EN, 0x0400, 0xFFFF, lane);

	/*
	 * 8. Override Speed_PLL value and use MAC PLL
	 */
	usb3_reg_set16(KVCO_CAL_CTRL, 0x0040 | rb_use_max_pll_rate, 0xFFFF,
		       lane);

	/*
	 * 9. Check the Polarity invert bit
	 */
	if (invert & COMPHY_POLARITY_TXD_INVERT)
		usb3_reg_set16(SYNC_PATTERN, phy_txd_inv, 0, lane);
	else
		usb3_reg_set16(SYNC_PATTERN, 0, phy_txd_inv, lane);

	if (invert & COMPHY_POLARITY_RXD_INVERT)
		usb3_reg_set16(SYNC_PATTERN, phy_rxd_inv, 0, lane);
	else
		usb3_reg_set16(SYNC_PATTERN, 0, phy_rxd_inv, lane);

	/*
	 * 10. Set max speed generation to USB3.0 5Gbps
	 */
	usb3_reg_set16(SYNC_MASK_GEN, 0x0400, 0x0C00, lane);

	/*
	 * 11. Set capacitor value for FFE gain peaking to 0xF
	 */
	usb3_reg_set16(GEN3_SETTINGS_3, 0xF, 0xF, lane);

	/*
	 * 12. Release SW reset
	 */
	usb3_reg_set16(GLOB_PHY_CTRL0,
		       rb_mode_core_clk_freq_sel | rb_mode_pipe_width_32
		       | 0x20, 0xFFFF, lane);

	/* Wait for > 55 us to allow PCLK be enabled */
	udelay(PLL_SET_DELAY_US);

	/* Assert PCLK enabled */
	if (lane == 2) {
		reg_set(rh_vsreg_addr,
			LANE_STAT1 + USB3PHY_LANE2_REG_BASE_OFFSET,
			0xFFFFFFFF);
		ret = comphy_poll_reg(rh_vsreg_data,		/* address */
				      rb_txdclk_pclk_en,	/* value */
				      rb_txdclk_pclk_en,	/* mask */
				      POLL_32B_REG);		/* 32bit */
	} else {
		ret = comphy_poll_reg(phy_addr(USB3, LANE_STAT1), /* address */
				      rb_txdclk_pclk_en,	  /* value */
				      rb_txdclk_pclk_en,	  /* mask */
				      POLL_16B_REG);		  /* 16bit */
	}
	if (!ret)
		printf("Failed to lock USB3 PLL\n");

	/*
	 * Set Soft ID for Host mode (Device mode works with Hard ID
	 * detection)
	 */
	if (type == COMPHY_TYPE_USB3_HOST0) {
		/*
		 * set   BIT0: set ID_MODE of Host/Device = "Soft ID" (BIT1)
		 * clear BIT1: set SOFT_ID = Host
		 * set   BIT4: set INT_MODE = ID. Interrupt Mode: enable
		 *             interrupt by ID instead of using both interrupts
		 *             of HOST and Device ORed simultaneously
		 *             INT_MODE=ID in order to avoid unexpected
		 *             behaviour or both interrupts together
		 */
		reg_set(USB32_CTRL_BASE,
			usb32_ctrl_id_mode | usb32_ctrl_int_mode,
			usb32_ctrl_id_mode | usb32_ctrl_soft_id |
			usb32_ctrl_int_mode);
	}

	debug_exit();

	return ret;
}

/*
 * comphy_usb2_power_up
 *
 * return: 1 if PLL locked (OK), 0 otherwise (FAIL)
 */
static int comphy_usb2_power_up(u8 usb32)
{
	int ret;

	debug_enter();

	if (usb32 != 0 && usb32 != 1) {
		printf("invalid usb32 value: (%d), should be either 0 or 1\n",
		       usb32);
		debug_exit();
		return 0;
	}

	/*
	 * 0. Setup PLL. 40MHz clock uses defaults.
	 *    See "PLL Settings for Typical REFCLK" table
	 */
	if (get_ref_clk() == 25) {
		reg_set(USB2_PHY_BASE(usb32), 5 | (96 << 16),
			0x3F | (0xFF << 16) | (0x3 << 28));
	}

	/*
	 * 1. PHY pull up and disable USB2 suspend
	 */
	reg_set(USB2_PHY_CTRL_ADDR(usb32),
		RB_USB2PHY_SUSPM(usb32) | RB_USB2PHY_PU(usb32), 0);

	if (usb32 != 0) {
		/*
		 * 2. Power up OTG module
		 */
		reg_set(USB2_PHY_OTG_CTRL_ADDR, rb_pu_otg, 0);

		/*
		 * 3. Configure PHY charger detection
		 */
		reg_set(USB2_PHY_CHRGR_DET_ADDR, 0,
			rb_cdp_en | rb_dcp_en | rb_pd_en | rb_cdp_dm_auto |
			rb_enswitch_dp | rb_enswitch_dm | rb_pu_chrg_dtc);
	}

	/* Assert PLL calibration done */
	ret = comphy_poll_reg(USB2_PHY_CAL_CTRL_ADDR(usb32),
			      rb_usb2phy_pllcal_done,	/* value */
			      rb_usb2phy_pllcal_done,	/* mask */
			      POLL_32B_REG);		/* 32bit */
	if (!ret) {
		printf("Failed to end USB2 PLL calibration\n");
		goto out;
	}

	/* Assert impedance calibration done */
	ret = comphy_poll_reg(USB2_PHY_CAL_CTRL_ADDR(usb32),
			      rb_usb2phy_impcal_done,	/* value */
			      rb_usb2phy_impcal_done,	/* mask */
			      POLL_32B_REG);		/* 32bit */
	if (!ret) {
		printf("Failed to end USB2 impedance calibration\n");
		goto out;
	}

	/* Assert squetch calibration done */
	ret = comphy_poll_reg(USB2_PHY_RX_CHAN_CTRL1_ADDR(usb32),
			      rb_usb2phy_sqcal_done,	/* value */
			      rb_usb2phy_sqcal_done,	/* mask */
			      POLL_32B_REG);		/* 32bit */
	if (!ret) {
		printf("Failed to end USB2 unknown calibration\n");
		goto out;
	}

	/* Assert PLL is ready */
	ret = comphy_poll_reg(USB2_PHY_PLL_CTRL0_ADDR(usb32),
			      rb_usb2phy_pll_ready,		/* value */
			      rb_usb2phy_pll_ready,		/* mask */
			      POLL_32B_REG);		/* 32bit */

	if (!ret) {
		printf("Failed to lock USB2 PLL\n");
		goto out;
	}

out:
	debug_exit();

	return ret;
}

/*
 * comphy_emmc_power_up
 *
 * return: 1 if PLL locked (OK), 0 otherwise (FAIL)
 */
static int comphy_emmc_power_up(void)
{
	debug_enter();

	/*
	 * 1. Bus power ON, Bus voltage 1.8V
	 */
	reg_set(SDIO_HOST_CTRL1_ADDR, 0xB00, 0xF00);

	/*
	 * 2. Set FIFO parameters
	 */
	reg_set(SDIO_SDHC_FIFO_ADDR, 0x315, 0xFFFFFFFF);

	/*
	 * 3. Set Capabilities 1_2
	 */
	reg_set(SDIO_CAP_12_ADDR, 0x25FAC8B2, 0xFFFFFFFF);

	/*
	 * 4. Set Endian
	 */
	reg_set(SDIO_ENDIAN_ADDR, 0x00c00000, 0);

	/*
	 * 4. Init PHY
	 */
	reg_set(SDIO_PHY_TIMING_ADDR, 0x80000000, 0x80000000);
	reg_set(SDIO_PHY_PAD_CTRL0_ADDR, 0x50000000, 0xF0000000);

	/*
	 * 5. DLL reset
	 */
	reg_set(SDIO_DLL_RST_ADDR, 0xFFFEFFFF, 0);
	reg_set(SDIO_DLL_RST_ADDR, 0x00010000, 0);

	debug_exit();

	return 1;
}

/*
 * comphy_sgmii_power_up
 *
 * return:
 */
static void comphy_sgmii_phy_init(u32 lane, u32 speed)
{
	const int fix_arr_sz = ARRAY_SIZE(sgmii_phy_init_fix);
	int addr, fix_idx;
	u16 val;

	fix_idx = 0;
	for (addr = 0; addr < 512; addr++) {
		/*
		 * All PHY register values are defined in full for 3.125Gbps
		 * SERDES speed. The values required for 1.25 Gbps are almost
		 * the same and only few registers should be "fixed" in
		 * comparison to 3.125 Gbps values. These register values are
		 * stored in "sgmii_phy_init_fix" array.
		 */
		if (speed != COMPHY_SPEED_1_25G &&
		    sgmii_phy_init_fix[fix_idx].addr == addr) {
			/* Use new value */
			val = sgmii_phy_init_fix[fix_idx].value;
			if (fix_idx < fix_arr_sz)
				fix_idx++;
		} else {
			val = sgmii_phy_init[addr];
		}

		reg_set16(sgmiiphy_addr(lane, addr), val, 0xFFFF);
	}
}

/*
 * comphy_sgmii_power_up
 *
 * return: 1 if PLL locked (OK), 0 otherwise (FAIL)
 */
static int comphy_sgmii_power_up(u32 lane, u32 speed, u32 invert)
{
	int ret;
	u32 saved_selector;

	debug_enter();

	/*
	 * 1. Configure PHY to SATA/SAS mode by setting pin PIN_PIPE_SEL=0
	 */
	saved_selector = readl(COMPHY_SEL_ADDR);
	reg_set(COMPHY_SEL_ADDR, 0, 0xFFFFFFFF);

	/*
	 * 2. Reset PHY by setting PHY input port PIN_RESET=1.
	 * 3. Set PHY input port PIN_TX_IDLE=1, PIN_PU_IVREF=1 to keep
	 *    PHY TXP/TXN output to idle state during PHY initialization
	 * 4. Set PHY input port PIN_PU_PLL=0, PIN_PU_RX=0, PIN_PU_TX=0.
	 */
	reg_set(COMPHY_PHY_CFG1_ADDR(lane),
		rb_pin_reset_comphy | rb_pin_tx_idle | rb_pin_pu_iveref,
		rb_pin_reset_core | rb_pin_pu_pll |
		rb_pin_pu_rx | rb_pin_pu_tx);

	/*
	 * 5. Release reset to the PHY by setting PIN_RESET=0.
	 */
	reg_set(COMPHY_PHY_CFG1_ADDR(lane), 0, rb_pin_reset_comphy);

	/*
	 * 7. Set PIN_PHY_GEN_TX[3:0] and PIN_PHY_GEN_RX[3:0] to decide
	 *    COMPHY bit rate
	 */
	if (speed == COMPHY_SPEED_3_125G) { /* 3.125 GHz */
		reg_set(COMPHY_PHY_CFG1_ADDR(lane),
			(0x8 << rf_gen_rx_sel_shift) |
			(0x8 << rf_gen_tx_sel_shift),
			rf_gen_rx_select | rf_gen_tx_select);

	} else if (speed == COMPHY_SPEED_1_25G) { /* 1.25 GHz */
		reg_set(COMPHY_PHY_CFG1_ADDR(lane),
			(0x6 << rf_gen_rx_sel_shift) |
			(0x6 << rf_gen_tx_sel_shift),
			rf_gen_rx_select | rf_gen_tx_select);
	} else {
		printf("Unsupported COMPHY speed!\n");
		return 0;
	}

	/*
	 * 8. Wait 1mS for bandgap and reference clocks to stabilize;
	 *    then start SW programming.
	 */
	mdelay(10);

	/* 9. Program COMPHY register PHY_MODE */
	reg_set16(sgmiiphy_addr(lane, PWR_PLL_CTRL),
		  PHY_MODE_SGMII << rf_phy_mode_shift, rf_phy_mode_mask);

	/*
	 * 10. Set COMPHY register REFCLK_SEL to select the correct REFCLK
	 *     source
	 */
	reg_set16(sgmiiphy_addr(lane, MISC_REG0), 0, rb_ref_clk_sel);

	/*
	 * 11. Set correct reference clock frequency in COMPHY register
	 *     REF_FREF_SEL.
	 */
	if (get_ref_clk() == 40) {
		reg_set16(sgmiiphy_addr(lane, PWR_PLL_CTRL),
			  0x4 << rf_ref_freq_sel_shift, rf_ref_freq_sel_mask);
	} else {
		/* 25MHz */
		reg_set16(sgmiiphy_addr(lane, PWR_PLL_CTRL),
			  0x1 << rf_ref_freq_sel_shift, rf_ref_freq_sel_mask);
	}

	/* 12. Program COMPHY register PHY_GEN_MAX[1:0] */
	/*
	 * This step is mentioned in the flow received from verification team.
	 * However the PHY_GEN_MAX value is only meaningful for other
	 * interfaces (not SGMII). For instance, it selects SATA speed
	 * 1.5/3/6 Gbps or PCIe speed  2.5/5 Gbps
	 */

	/*
	 * 13. Program COMPHY register SEL_BITS to set correct parallel data
	 *     bus width
	 */
	/* 10bit */
	reg_set16(sgmiiphy_addr(lane, DIG_LB_EN), 0, rf_data_width_mask);

	/*
	 * 14. As long as DFE function needs to be enabled in any mode,
	 *     COMPHY register DFE_UPDATE_EN[5:0] shall be programmed to 0x3F
	 *     for real chip during COMPHY power on.
	 */
	/*
	 * The step 14 exists (and empty) in the original initialization flow
	 * obtained from the verification team. According to the functional
	 * specification DFE_UPDATE_EN already has the default value 0x3F
	 */

	/*
	 * 15. Program COMPHY GEN registers.
	 *     These registers should be programmed based on the lab testing
	 *     result to achieve optimal performance. Please contact the CEA
	 *     group to get the related GEN table during real chip bring-up.
	 *     We only requred to run though the entire registers programming
	 *     flow defined by "comphy_sgmii_phy_init" when the REF clock is
	 *     40 MHz. For REF clock 25 MHz the default values stored in PHY
	 *     registers are OK.
	 */
	debug("Running C-DPI phy init %s mode\n",
	      speed == COMPHY_SPEED_3_125G ? "2G5" : "1G");
	if (get_ref_clk() == 40)
		comphy_sgmii_phy_init(lane, speed);

	/*
	 * 16. [Simulation Only] should not be used for real chip.
	 *     By pass power up calibration by programming EXT_FORCE_CAL_DONE
	 *     (R02h[9]) to 1 to shorten COMPHY simulation time.
	 */
	/*
	 * 17. [Simulation Only: should not be used for real chip]
	 *     Program COMPHY register FAST_DFE_TIMER_EN=1 to shorten RX
	 *     training simulation time.
	 */

	/*
	 * 18. Check the PHY Polarity invert bit
	 */
	if (invert & COMPHY_POLARITY_TXD_INVERT)
		reg_set16(sgmiiphy_addr(lane, SYNC_PATTERN), phy_txd_inv, 0);
	else
		reg_set16(sgmiiphy_addr(lane, SYNC_PATTERN), 0, phy_txd_inv);

	if (invert & COMPHY_POLARITY_RXD_INVERT)
		reg_set16(sgmiiphy_addr(lane, SYNC_PATTERN), phy_rxd_inv, 0);
	else
		reg_set16(sgmiiphy_addr(lane, SYNC_PATTERN), 0, phy_rxd_inv);

	/*
	 * 19. Set PHY input ports PIN_PU_PLL, PIN_PU_TX and PIN_PU_RX to 1
	 *     to start PHY power up sequence. All the PHY register
	 *     programming should be done before PIN_PU_PLL=1. There should be
	 *     no register programming for normal PHY operation from this point.
	 */
	reg_set(COMPHY_PHY_CFG1_ADDR(lane),
		rb_pin_pu_pll | rb_pin_pu_rx | rb_pin_pu_tx,
		rb_pin_pu_pll | rb_pin_pu_rx | rb_pin_pu_tx);

	/*
	 * 20. Wait for PHY power up sequence to finish by checking output ports
	 *     PIN_PLL_READY_TX=1 and PIN_PLL_READY_RX=1.
	 */
	ret = comphy_poll_reg(COMPHY_PHY_STAT1_ADDR(lane),	/* address */
			      rb_pll_ready_tx | rb_pll_ready_rx, /* value */
			      rb_pll_ready_tx | rb_pll_ready_rx, /* mask */
			      POLL_32B_REG);			/* 32bit */
	if (!ret) {
		printf("Failed to lock PLL for SGMII PHY %d\n", lane);
		goto out;
	}

	/*
	 * 21. Set COMPHY input port PIN_TX_IDLE=0
	 */
	reg_set(COMPHY_PHY_CFG1_ADDR(lane), 0x0, rb_pin_tx_idle);

	/*
	 * 22. After valid data appear on PIN_RXDATA bus, set PIN_RX_INIT=1.
	 *     to start RX initialization. PIN_RX_INIT_DONE will be cleared to
	 *     0 by the PHY. After RX initialization is done, PIN_RX_INIT_DONE
	 *     will be set to 1 by COMPHY. Set PIN_RX_INIT=0 after
	 *     PIN_RX_INIT_DONE= 1.
	 *     Please refer to RX initialization part for details.
	 */
	reg_set(COMPHY_PHY_CFG1_ADDR(lane), rb_phy_rx_init, 0x0);

	ret = comphy_poll_reg(COMPHY_PHY_STAT1_ADDR(lane), /* address */
			      rb_rx_init_done,			/* value */
			      rb_rx_init_done,			/* mask */
			      POLL_32B_REG);			/* 32bit */
	if (!ret) {
		printf("Failed to init RX of SGMII PHY %d\n", lane);
		goto out;
	}

	/*
	 * Restore saved selector.
	 */
	reg_set(COMPHY_SEL_ADDR, saved_selector, 0xFFFFFFFF);

out:
	debug_exit();

	return ret;
}

void comphy_dedicated_phys_init(void)
{
	int node, usb32, ret = 1;
	const void *blob = gd->fdt_blob;

	debug_enter();

	for (usb32 = 0; usb32 <= 1; usb32++) {
		/*
		 * There are 2 UTMI PHYs in this SOC.
		 * One is independendent and one is paired with USB3 port (OTG)
		 */
		if (usb32 == 0) {
			node = fdt_node_offset_by_compatible(
				blob, -1, "marvell,armada-3700-ehci");
		} else {
			node = fdt_node_offset_by_compatible(
				blob, -1, "marvell,armada3700-xhci");
		}

		if (node > 0) {
			if (fdtdec_get_is_enabled(blob, node)) {
				ret = comphy_usb2_power_up(usb32);
				if (!ret)
					printf("Failed to initialize UTMI PHY\n");
				else
					debug("UTMI PHY init succeed\n");
			} else {
				debug("USB%d node is disabled\n",
				      usb32 == 0 ? 2 : 3);
			}
		} else {
			debug("No USB%d node in DT\n", usb32 == 0 ? 2 : 3);
		}
	}

	node = fdt_node_offset_by_compatible(blob, -1,
					     "marvell,armada-8k-sdhci");
	if (node <= 0) {
		node = fdt_node_offset_by_compatible(
			blob, -1, "marvell,armada-3700-sdhci");
	}

	if (node > 0) {
		if (fdtdec_get_is_enabled(blob, node)) {
			ret = comphy_emmc_power_up();
			if (!ret)
				printf("Failed to initialize SDIO/eMMC PHY\n");
			else
				debug("SDIO/eMMC PHY init succeed\n");
		} else {
			debug("SDIO/eMMC node is disabled\n");
		}
	}  else {
		debug("No SDIO/eMMC node in DT\n");
	}

	debug_exit();
}

static int find_available_node_by_compatible(int offset, const char *compatible)
{
	fdt_for_each_node_by_compatible(offset, gd->fdt_blob, offset,
					compatible)
		if (fdtdec_get_is_enabled(gd->fdt_blob, offset))
			return offset;

	return -1;
}

static bool comphy_a3700_find_lane(const int nodes[3], int node,
				   int port, int *lane, int *invert)
{
	int res, i, j;

	for (i = 0; ; i++) {
		struct fdtdec_phandle_args args;

		res = fdtdec_parse_phandle_with_args(gd->fdt_blob, node, "phys",
						     "#phy-cells", 0, i, &args);
		if (res)
			return false;

		for (j = 0; j < 3; j++) {
			if (nodes[j] >= 0 && args.node == nodes[j] &&
			    (args.args_count >= 1 ? args.args[0] : 0) == port) {
				*lane = j;
				*invert = args.args_count >= 2 ? args.args[1]
							       : 0;
				return true;
			}
		}
	}

	return false;
}

static void comphy_a3700_fill_cfg(struct chip_serdes_phy_config *cfg,
				  const int nodes[3], const char *compatible,
				  int type)
{
	int node, lane, port, speed, invert;

	port = (type == COMPHY_TYPE_SGMII1) ? 1 : 0;

	node = -1;
	while (1) {
		node = find_available_node_by_compatible(node, compatible);
		if (node < 0)
			return;

		if (comphy_a3700_find_lane(nodes, node, port, &lane, &invert))
			break;
	}

	if (cfg->comphy_map_data[lane].type != COMPHY_TYPE_UNCONNECTED) {
		printf("Error: More PHYs defined for lane %d, skipping\n",
		       lane);
		return;
	}

	if (type == COMPHY_TYPE_SGMII0 || type == COMPHY_TYPE_SGMII1) {
		const char *phy_mode;

		phy_mode = fdt_getprop(gd->fdt_blob, node, "phy-mode", NULL);
		if (phy_mode &&
		    !strcmp(phy_mode,
			    phy_string_for_interface(PHY_INTERFACE_MODE_2500BASEX)))
			speed = COMPHY_SPEED_3_125G;
		else
			speed = COMPHY_SPEED_1_25G;
	} else if (type == COMPHY_TYPE_SATA0) {
		speed = COMPHY_SPEED_6G;
	} else {
		speed = COMPHY_SPEED_5G;
	}

	cfg->comphy_map_data[lane].type = type;
	cfg->comphy_map_data[lane].speed = speed;
	cfg->comphy_map_data[lane].invert = invert;
}

static const fdt32_t comphy_a3700_mux_lane_order[3] = {
	__constant_cpu_to_be32(1),
	__constant_cpu_to_be32(0),
	__constant_cpu_to_be32(2),
};

int comphy_a3700_init_serdes_map(int node, struct chip_serdes_phy_config *cfg)
{
	int comphy_nodes[3];
	int child, i;

	for (i = 0; i < ARRAY_SIZE(comphy_nodes); i++)
		comphy_nodes[i] = -FDT_ERR_NOTFOUND;

	fdt_for_each_subnode(child, gd->fdt_blob, node) {
		if (!fdtdec_get_is_enabled(gd->fdt_blob, child))
			continue;

		i = fdtdec_get_int(gd->fdt_blob, child, "reg", -1);
		if (i < 0 || i >= ARRAY_SIZE(comphy_nodes))
			continue;

		comphy_nodes[i] = child;
	}

	for (i = 0; i < ARRAY_SIZE(comphy_nodes); i++) {
		cfg->comphy_map_data[i].type = COMPHY_TYPE_UNCONNECTED;
		cfg->comphy_map_data[i].speed = COMPHY_SPEED_INVALID;
	}

	comphy_a3700_fill_cfg(cfg, comphy_nodes, "marvell,armada3700-u3d",
			      COMPHY_TYPE_USB3_DEVICE);
	comphy_a3700_fill_cfg(cfg, comphy_nodes, "marvell,armada3700-xhci",
			      COMPHY_TYPE_USB3_HOST0);
	comphy_a3700_fill_cfg(cfg, comphy_nodes, "marvell,armada-3700-pcie",
			      COMPHY_TYPE_PEX0);
	comphy_a3700_fill_cfg(cfg, comphy_nodes, "marvell,armada-3700-ahci",
			      COMPHY_TYPE_SATA0);
	comphy_a3700_fill_cfg(cfg, comphy_nodes, "marvell,armada-3700-neta",
			      COMPHY_TYPE_SGMII0);
	comphy_a3700_fill_cfg(cfg, comphy_nodes, "marvell,armada-3700-neta",
			      COMPHY_TYPE_SGMII1);

	cfg->comphy_lanes_count = 3;
	cfg->comphy_mux_bitcount = 4;
	cfg->comphy_mux_lane_order = comphy_a3700_mux_lane_order;

	return 0;
}

int comphy_a3700_init(struct chip_serdes_phy_config *chip_cfg,
		      struct comphy_map *serdes_map)
{
	struct comphy_map *comphy_map;
	u32 comphy_max_count = chip_cfg->comphy_lanes_count;
	u32 lane, ret = 0;

	debug_enter();

	/* Initialize PHY mux */
	chip_cfg->mux_data = a3700_comphy_mux_data;
	comphy_mux_init(chip_cfg, serdes_map, COMPHY_SEL_ADDR);

	for (lane = 0, comphy_map = serdes_map; lane < comphy_max_count;
	     lane++, comphy_map++) {
		debug("Initialize serdes number %d\n", lane);
		debug("Serdes type = 0x%x invert=%d\n",
		      comphy_map->type, comphy_map->invert);

		switch (comphy_map->type) {
		case COMPHY_TYPE_UNCONNECTED:
			continue;
			break;

		case COMPHY_TYPE_PEX0:
			ret = comphy_pcie_power_up(comphy_map->speed,
						   comphy_map->invert);
			break;

		case COMPHY_TYPE_USB3_HOST0:
		case COMPHY_TYPE_USB3_DEVICE:
			ret = comphy_usb3_power_up(lane,
						   comphy_map->type,
						   comphy_map->speed,
						   comphy_map->invert);
			break;

		case COMPHY_TYPE_SGMII0:
		case COMPHY_TYPE_SGMII1:
			ret = comphy_sgmii_power_up(lane, comphy_map->speed,
						    comphy_map->invert);
			break;

		case COMPHY_TYPE_SATA0:
			ret = comphy_sata_power_up(comphy_map->invert);
			break;

		default:
			debug("Unknown SerDes type, skip initialize SerDes %d\n",
			      lane);
			ret = 1;
			break;
		}
		if (!ret)
			printf("PLL is not locked - Failed to initialize lane %d\n",
			       lane);
	}

	debug_exit();
	return ret;
}
