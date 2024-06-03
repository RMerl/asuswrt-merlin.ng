/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (c) 2016 Google, Inc
 */
#ifndef _ASM_ARCH_SCU_AST2500_H
#define _ASM_ARCH_SCU_AST2500_H

#define SCU_UNLOCK_VALUE		0x1688a8a8

#define SCU_HWSTRAP_VGAMEM_SHIFT	2
#define SCU_HWSTRAP_VGAMEM_MASK		(3 << SCU_HWSTRAP_VGAMEM_SHIFT)
#define SCU_HWSTRAP_MAC1_RGMII		(1 << 6)
#define SCU_HWSTRAP_MAC2_RGMII		(1 << 7)
#define SCU_HWSTRAP_DDR4		(1 << 24)
#define SCU_HWSTRAP_CLKIN_25MHZ		(1 << 23)

#define SCU_MPLL_DENUM_SHIFT		0
#define SCU_MPLL_DENUM_MASK		0x1f
#define SCU_MPLL_NUM_SHIFT		5
#define SCU_MPLL_NUM_MASK		(0xff << SCU_MPLL_NUM_SHIFT)
#define SCU_MPLL_POST_SHIFT		13
#define SCU_MPLL_POST_MASK		(0x3f << SCU_MPLL_POST_SHIFT)
#define SCU_PCLK_DIV_SHIFT		23
#define SCU_PCLK_DIV_MASK		(7 << SCU_PCLK_DIV_SHIFT)
#define SCU_HPLL_DENUM_SHIFT		0
#define SCU_HPLL_DENUM_MASK		0x1f
#define SCU_HPLL_NUM_SHIFT		5
#define SCU_HPLL_NUM_MASK		(0xff << SCU_HPLL_NUM_SHIFT)
#define SCU_HPLL_POST_SHIFT		13
#define SCU_HPLL_POST_MASK		(0x3f << SCU_HPLL_POST_SHIFT)

#define SCU_MACCLK_SHIFT		16
#define SCU_MACCLK_MASK			(7 << SCU_MACCLK_SHIFT)

#define SCU_MISC2_RGMII_HPLL		(1 << 23)
#define SCU_MISC2_RGMII_CLKDIV_SHIFT	20
#define SCU_MISC2_RGMII_CLKDIV_MASK	(3 << SCU_MISC2_RGMII_CLKDIV_SHIFT)
#define SCU_MISC2_RMII_MPLL		(1 << 19)
#define SCU_MISC2_RMII_CLKDIV_SHIFT	16
#define SCU_MISC2_RMII_CLKDIV_MASK	(3 << SCU_MISC2_RMII_CLKDIV_SHIFT)
#define SCU_MISC2_UARTCLK_SHIFT		24

#define SCU_MISC_D2PLL_OFF		(1 << 4)
#define SCU_MISC_UARTCLK_DIV13		(1 << 12)
#define SCU_MISC_GCRT_USB20CLK		(1 << 21)

#define SCU_MICDS_MAC1RGMII_TXDLY_SHIFT	0
#define SCU_MICDS_MAC1RGMII_TXDLY_MASK	(0x3f\
					 << SCU_MICDS_MAC1RGMII_TXDLY_SHIFT)
#define SCU_MICDS_MAC2RGMII_TXDLY_SHIFT	6
#define SCU_MICDS_MAC2RGMII_TXDLY_MASK	(0x3f\
					 << SCU_MICDS_MAC2RGMII_TXDLY_SHIFT)
#define SCU_MICDS_MAC1RMII_RDLY_SHIFT	12
#define SCU_MICDS_MAC1RMII_RDLY_MASK	(0x3f << SCU_MICDS_MAC1RMII_RDLY_SHIFT)
#define SCU_MICDS_MAC2RMII_RDLY_SHIFT	18
#define SCU_MICDS_MAC2RMII_RDLY_MASK	(0x3f << SCU_MICDS_MAC2RMII_RDLY_SHIFT)
#define SCU_MICDS_MAC1RMII_TXFALL	(1 << 24)
#define SCU_MICDS_MAC2RMII_TXFALL	(1 << 25)
#define SCU_MICDS_RMII1_RCLKEN		(1 << 29)
#define SCU_MICDS_RMII2_RCLKEN		(1 << 30)
#define SCU_MICDS_RGMIIPLL		(1 << 31)

/*
 * SYSRESET is actually more like a Power register,
 * except that corresponding bit set to 1 means that
 * the peripheral is off.
 */
#define SCU_SYSRESET_XDMA		(1 << 25)
#define SCU_SYSRESET_MCTP		(1 << 24)
#define SCU_SYSRESET_ADC		(1 << 23)
#define SCU_SYSRESET_JTAG		(1 << 22)
#define SCU_SYSRESET_MIC		(1 << 18)
#define SCU_SYSRESET_SDIO		(1 << 16)
#define SCU_SYSRESET_USB11HOST		(1 << 15)
#define SCU_SYSRESET_USBHUB		(1 << 14)
#define SCU_SYSRESET_CRT		(1 << 13)
#define SCU_SYSRESET_MAC2		(1 << 12)
#define SCU_SYSRESET_MAC1		(1 << 11)
#define SCU_SYSRESET_PECI		(1 << 10)
#define SCU_SYSRESET_PWM		(1 << 9)
#define SCU_SYSRESET_PCI_VGA		(1 << 8)
#define SCU_SYSRESET_2D			(1 << 7)
#define SCU_SYSRESET_VIDEO		(1 << 6)
#define SCU_SYSRESET_LPC		(1 << 5)
#define SCU_SYSRESET_HAC		(1 << 4)
#define SCU_SYSRESET_USBHID		(1 << 3)
#define SCU_SYSRESET_I2C		(1 << 2)
#define SCU_SYSRESET_AHB		(1 << 1)
#define SCU_SYSRESET_SDRAM_WDT		(1 << 0)

/* Bits 16-27 in the register control pin functions for I2C devices 3-14 */
#define SCU_PINMUX_CTRL5_I2C		(1 << 16)

/*
 * The values are grouped by function, not by register.
 * They are actually scattered across multiple loosely related registers.
 */
#define SCU_PIN_FUN_MAC1_MDC		(1 << 30)
#define SCU_PIN_FUN_MAC1_MDIO		(1 << 31)
#define SCU_PIN_FUN_MAC1_PHY_LINK	(1 << 0)
#define SCU_PIN_FUN_MAC2_MDIO		(1 << 2)
#define SCU_PIN_FUN_MAC2_PHY_LINK	(1 << 1)
#define SCU_PIN_FUN_SCL1		(1 << 12)
#define SCU_PIN_FUN_SCL2		(1 << 14)
#define SCU_PIN_FUN_SDA1		(1 << 13)
#define SCU_PIN_FUN_SDA2		(1 << 15)

#define SCU_CLKSTOP_MAC1		(1 << 20)
#define SCU_CLKSTOP_MAC2		(1 << 21)

#define SCU_D2PLL_EXT1_OFF		(1 << 0)
#define SCU_D2PLL_EXT1_BYPASS		(1 << 1)
#define SCU_D2PLL_EXT1_RESET		(1 << 2)
#define SCU_D2PLL_EXT1_MODE_SHIFT	3
#define SCU_D2PLL_EXT1_MODE_MASK	(3 << SCU_D2PLL_EXT1_MODE_SHIFT)
#define SCU_D2PLL_EXT1_PARAM_SHIFT	5
#define SCU_D2PLL_EXT1_PARAM_MASK	(0x1ff << SCU_D2PLL_EXT1_PARAM_SHIFT)

#define SCU_D2PLL_NUM_SHIFT		0
#define SCU_D2PLL_NUM_MASK		(0xff << SCU_D2PLL_NUM_SHIFT)
#define SCU_D2PLL_DENUM_SHIFT		8
#define SCU_D2PLL_DENUM_MASK		(0x1f << SCU_D2PLL_DENUM_SHIFT)
#define SCU_D2PLL_POST_SHIFT		13
#define SCU_D2PLL_POST_MASK		(0x3f << SCU_D2PLL_POST_SHIFT)
#define SCU_D2PLL_ODIV_SHIFT		19
#define SCU_D2PLL_ODIV_MASK		(7 << SCU_D2PLL_ODIV_SHIFT)
#define SCU_D2PLL_SIC_SHIFT		22
#define SCU_D2PLL_SIC_MASK		(0x1f << SCU_D2PLL_SIC_SHIFT)
#define SCU_D2PLL_SIP_SHIFT		27
#define SCU_D2PLL_SIP_MASK		(0x1f << SCU_D2PLL_SIP_SHIFT)

#define SCU_CLKDUTY_DCLK_SHIFT		0
#define SCU_CLKDUTY_DCLK_MASK		(0x3f << SCU_CLKDUTY_DCLK_SHIFT)
#define SCU_CLKDUTY_RGMII1TXCK_SHIFT	8
#define SCU_CLKDUTY_RGMII1TXCK_MASK	(0x7f << SCU_CLKDUTY_RGMII1TXCK_SHIFT)
#define SCU_CLKDUTY_RGMII2TXCK_SHIFT	16
#define SCU_CLKDUTY_RGMII2TXCK_MASK	(0x7f << SCU_CLKDUTY_RGMII2TXCK_SHIFT)

#ifndef __ASSEMBLY__

struct ast2500_clk_priv {
	struct ast2500_scu *scu;
};

struct ast2500_scu {
	u32 protection_key;
	u32 sysreset_ctrl1;
	u32 clk_sel1;
	u32 clk_stop_ctrl1;
	u32 freq_counter_ctrl;
	u32 freq_counter_cmp;
	u32 intr_ctrl;
	u32 d2_pll_param;
	u32 m_pll_param;
	u32 h_pll_param;
	u32 d_pll_param;
	u32 misc_ctrl1;
	u32 pci_config[3];
	u32 sysreset_status;
	u32 vga_handshake[2];
	u32 mac_clk_delay;
	u32 misc_ctrl2;
	u32 vga_scratch[8];
	u32 hwstrap;
	u32 rng_ctrl;
	u32 rng_data;
	u32 rev_id;
	u32 pinmux_ctrl[6];
	u32 reserved0;
	u32 extrst_sel;
	u32 pinmux_ctrl1[4];
	u32 reserved1[2];
	u32 mac_clk_delay_100M;
	u32 mac_clk_delay_10M;
	u32 wakeup_enable;
	u32 wakeup_control;
	u32 reserved2[3];
	u32 sysreset_ctrl2;
	u32 clk_sel2;
	u32 clk_stop_ctrl2;
	u32 freerun_counter;
	u32 freerun_counter_ext;
	u32 clk_duty_meas_ctrl;
	u32 clk_duty_meas_res;
	u32 reserved3[4];
	/* The next registers are not key-protected */
	struct ast2500_cpu2 {
		u32 ctrl;
		u32 base_addr[9];
		u32 cache_ctrl;
	} cpu2;
	u32 reserved4;
	u32 d_pll_ext_param[3];
	u32 d2_pll_ext_param[3];
	u32 mh_pll_ext_param;
	u32 reserved5;
	u32 chip_id[2];
	u32 reserved6[2];
	u32 uart_clk_ctrl;
	u32 reserved7[7];
	u32 pcie_config;
	u32 mmio_decode;
	u32 reloc_ctrl_decode[2];
	u32 mailbox_addr;
	u32 shared_sram_decode[2];
	u32 bmc_rev_id;
	u32 reserved8;
	u32 bmc_device_id;
	u32 reserved9[13];
	u32 clk_duty_sel;
};

/**
 * ast_get_clk() - get a pointer to Clock Driver
 *
 * @devp, OUT - pointer to Clock Driver
 * @return zero on success, error code (< 0) otherwise.
 */
int ast_get_clk(struct udevice **devp);

/**
 * ast_get_scu() - get a pointer to SCU registers
 *
 * @return pointer to struct ast2500_scu on success, ERR_PTR otherwise
 */
void *ast_get_scu(void);

/**
 * ast_scu_unlock() - unlock protected registers
 *
 * @scu, pointer to ast2500_scu
 */
void ast_scu_unlock(struct ast2500_scu *scu);

/**
 * ast_scu_lock() - lock protected registers
 *
 * @scu, pointer to ast2500_scu
 */
void ast_scu_lock(struct ast2500_scu *scu);

#endif  /* __ASSEMBLY__ */

#endif  /* _ASM_ARCH_SCU_AST2500_H */
