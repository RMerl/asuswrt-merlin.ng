/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2010
 * Texas Instruments Incorporated, <www.ti.com>
 *
 *	Balaji Krishnamoorthy	<balajitk@ti.com>
 *	Aneesh V		<aneesh@ti.com>
 */
#ifndef _PANDA_MUX_DATA_H_
#define _PANDA_MUX_DATA_H_

#include <asm/arch/mux_omap4.h>


const struct pad_conf_entry core_padconf_array_essential[] = {

{GPMC_AD0, (PTU | IEN | OFF_EN | OFF_PD | OFF_IN | M1)}, /* sdmmc2_dat0 */
{GPMC_AD1, (PTU | IEN | OFF_EN | OFF_PD | OFF_IN | M1)}, /* sdmmc2_dat1 */
{GPMC_AD2, (PTU | IEN | OFF_EN | OFF_PD | OFF_IN | M1)}, /* sdmmc2_dat2 */
{GPMC_AD3, (PTU | IEN | OFF_EN | OFF_PD | OFF_IN | M1)}, /* sdmmc2_dat3 */
{GPMC_AD4, (PTU | IEN | OFF_EN | OFF_PD | OFF_IN | M1)}, /* sdmmc2_dat4 */
{GPMC_AD5, (PTU | IEN | OFF_EN | OFF_PD | OFF_IN | M1)}, /* sdmmc2_dat5 */
{GPMC_AD6, (PTU | IEN | OFF_EN | OFF_PD | OFF_IN | M1)}, /* sdmmc2_dat6 */
{GPMC_AD7, (PTU | IEN | OFF_EN | OFF_PD | OFF_IN | M1)}, /* sdmmc2_dat7 */
{GPMC_NOE, (PTU | IEN | OFF_EN | OFF_OUT_PTD | M1)},	 /* sdmmc2_clk */
{GPMC_NWE, (PTU | IEN | OFF_EN | OFF_PD | OFF_IN | M1)}, /* sdmmc2_cmd */
{SDMMC1_CLK, (PTU | OFF_EN | OFF_OUT_PTD | M0)},	 /* sdmmc1_clk */
{SDMMC1_CMD, (PTU | IEN | OFF_EN | OFF_PD | OFF_IN | M0)}, /* sdmmc1_cmd */
{SDMMC1_DAT0, (PTU | IEN | OFF_EN | OFF_PD | OFF_IN | M0)}, /* sdmmc1_dat0 */
{SDMMC1_DAT1, (PTU | IEN | OFF_EN | OFF_PD | OFF_IN | M0)}, /* sdmmc1_dat1 */
{SDMMC1_DAT2, (PTU | IEN | OFF_EN | OFF_PD | OFF_IN | M0)}, /* sdmmc1_dat2 */
{SDMMC1_DAT3, (PTU | IEN | OFF_EN | OFF_PD | OFF_IN | M0)}, /* sdmmc1_dat3 */
{SDMMC1_DAT4, (PTU | IEN | OFF_EN | OFF_PD | OFF_IN | M0)}, /* sdmmc1_dat4 */
{SDMMC1_DAT5, (PTU | IEN | OFF_EN | OFF_PD | OFF_IN | M0)}, /* sdmmc1_dat5 */
{SDMMC1_DAT6, (PTU | IEN | OFF_EN | OFF_PD | OFF_IN | M0)}, /* sdmmc1_dat6 */
{SDMMC1_DAT7, (PTU | IEN | OFF_EN | OFF_PD | OFF_IN | M0)}, /* sdmmc1_dat7 */
{I2C1_SCL, (PTU | IEN | M0)},				/* i2c1_scl */
{I2C1_SDA, (PTU | IEN | M0)},				/* i2c1_sda */
{I2C2_SCL, (PTU | IEN | M0)},				/* i2c2_scl */
{I2C2_SDA, (PTU | IEN | M0)},				/* i2c2_sda */
{I2C3_SCL, (PTU | IEN | M0)},				/* i2c3_scl */
{I2C3_SDA, (PTU | IEN | M0)},				/* i2c3_sda */
{I2C4_SCL, (PTU | IEN | M0)},				/* i2c4_scl */
{I2C4_SDA, (PTU | IEN | M0)},				/* i2c4_sda */
{UART3_CTS_RCTX, (PTU | IEN | M0)},			/* uart3_tx */
{UART3_RTS_SD, (M0)},					/* uart3_rts_sd */
{UART3_RX_IRRX, (IEN | M0)},				/* uart3_rx */
{UART3_TX_IRTX, (M0)},					/* uart3_tx */
{USBB1_ULPITLL_CLK, (PTD | IEN | OFF_EN | OFF_PD | OFF_IN | M4)},/* usbb1_ulpiphy_clk */
{USBB1_ULPITLL_STP, (OFF_EN | OFF_OUT_PTD | M4)},		/* usbb1_ulpiphy_stp */
{USBB1_ULPITLL_DIR, (IEN | OFF_EN | OFF_PD | OFF_IN | M4)},	/* usbb1_ulpiphy_dir */
{USBB1_ULPITLL_NXT, (IEN | OFF_EN | OFF_PD | OFF_IN | M4)},	/* usbb1_ulpiphy_nxt */
{USBB1_ULPITLL_DAT0, (IEN | OFF_EN | OFF_PD | OFF_IN | M4)},	/* usbb1_ulpiphy_dat0 */
{USBB1_ULPITLL_DAT1, (IEN | OFF_EN | OFF_PD | OFF_IN | M4)},	/* usbb1_ulpiphy_dat1 */
{USBB1_ULPITLL_DAT2, (IEN | OFF_EN | OFF_PD | OFF_IN | M4)},	/* usbb1_ulpiphy_dat2 */
{USBB1_ULPITLL_DAT3, (IEN | OFF_EN | OFF_PD | OFF_IN | M4)},	/* usbb1_ulpiphy_dat3 */
{USBB1_ULPITLL_DAT4, (IEN | OFF_EN | OFF_PD | OFF_IN | M4)},	/* usbb1_ulpiphy_dat4 */
{USBB1_ULPITLL_DAT5, (IEN | OFF_EN | OFF_PD | OFF_IN | M4)},	/* usbb1_ulpiphy_dat5 */
{USBB1_ULPITLL_DAT6, (IEN | OFF_EN | OFF_PD | OFF_IN | M4)},	/* usbb1_ulpiphy_dat6 */
{USBB1_ULPITLL_DAT7, (IEN | OFF_EN | OFF_PD | OFF_IN | M4)},	/* usbb1_ulpiphy_dat7 */
{USBB1_HSIC_DATA, (IEN | OFF_EN | OFF_PD | OFF_IN | M0)},	/* usbb1_hsic_data */
{USBB1_HSIC_STROBE, (IEN | OFF_EN | OFF_PD | OFF_IN | M0)},	/* usbb1_hsic_strobe */
{USBC1_ICUSB_DP, (IEN | M0)},					/* usbc1_icusb_dp */
{USBC1_ICUSB_DM, (IEN | M0)},					/* usbc1_icusb_dm */
{UNIPRO_TY2, (PTU | IEN | M3)},					/* gpio_1 */
{GPMC_WAIT1,  (PTD | OFF_EN | OFF_PD | OFF_OUT_PTD | M3)},	/* gpio_62 */
{FREF_CLK2_OUT, (PTU | IEN | M3)},				/* gpio_182 */

};

const struct pad_conf_entry wkup_padconf_array_essential[] = {

{PAD1_SR_SCL, (PTU | IEN | M0)}, /* sr_scl */
{PAD0_SR_SDA, (PTU | IEN | M0)}, /* sr_sda */
{PAD1_SYS_32K, (IEN | M0)},	 /* sys_32k */
{PAD0_FREF_CLK3_OUT, (M0)}, /* fref_clk3_out */

};

const struct pad_conf_entry wkup_padconf_array_essential_4460[] = {

{PAD1_FREF_CLK4_REQ, (M3)}, /* gpio_wk7 for TPS: Mode 3 */

};

#endif /* _PANDA_MUX_DATA_H_ */
