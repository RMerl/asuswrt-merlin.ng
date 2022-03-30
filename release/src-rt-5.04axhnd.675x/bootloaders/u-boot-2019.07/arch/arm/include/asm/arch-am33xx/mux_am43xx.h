/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * mux_am43xx.h
 *
 * Copyright (C) 2013 Texas Instruments Incorporated - http://www.ti.com/
 */

#ifndef _MUX_AM43XX_H_
#define _MUX_AM43XX_H_

#include <common.h>
#include <asm/io.h>

#define MUX_CFG(value, offset)	\
	__raw_writel(value, (CTRL_BASE + offset));

/* PAD Control Fields */
#define SLEWCTRL	(0x1 << 19)
#define RXACTIVE	(0x1 << 18)
#define PULLDOWN_EN	(0x0 << 17) /* Pull Down Selection */
#define PULLUP_EN	(0x1 << 17) /* Pull Up Selection */
#define PULLUDEN	(0x0 << 16) /* Pull up/down enable */
#define PULLUDDIS	(0x1 << 16) /* Pull up/down disable */
#define MODE(val)	val	/* used for Readability */

/*
 * PAD CONTROL OFFSETS
 * Field names corresponds to the pad signal name
 */
struct pad_signals {
	int gpmc_ad0;
	int gpmc_ad1;
	int gpmc_ad2;
	int gpmc_ad3;
	int gpmc_ad4;
	int gpmc_ad5;
	int gpmc_ad6;
	int gpmc_ad7;
	int gpmc_ad8;
	int gpmc_ad9;
	int gpmc_ad10;
	int gpmc_ad11;
	int gpmc_ad12;
	int gpmc_ad13;
	int gpmc_ad14;
	int gpmc_ad15;
	int gpmc_a0;
	int gpmc_a1;
	int gpmc_a2;
	int gpmc_a3;
	int gpmc_a4;
	int gpmc_a5;
	int gpmc_a6;
	int gpmc_a7;
	int gpmc_a8;
	int gpmc_a9;
	int gpmc_a10;
	int gpmc_a11;
	int gpmc_wait0;
	int gpmc_wpn;
	int gpmc_be1n;
	int gpmc_csn0;
	int gpmc_csn1;
	int gpmc_csn2;
	int gpmc_csn3;
	int gpmc_clk;
	int gpmc_advn_ale;
	int gpmc_oen_ren;
	int gpmc_wen;
	int gpmc_be0n_cle;
	int lcd_data0;
	int lcd_data1;
	int lcd_data2;
	int lcd_data3;
	int lcd_data4;
	int lcd_data5;
	int lcd_data6;
	int lcd_data7;
	int lcd_data8;
	int lcd_data9;
	int lcd_data10;
	int lcd_data11;
	int lcd_data12;
	int lcd_data13;
	int lcd_data14;
	int lcd_data15;
	int lcd_vsync;
	int lcd_hsync;
	int lcd_pclk;
	int lcd_ac_bias_en;
	int mmc0_dat3;
	int mmc0_dat2;
	int mmc0_dat1;
	int mmc0_dat0;
	int mmc0_clk;
	int mmc0_cmd;
	int mii1_col;
	int mii1_crs;
	int mii1_rxerr;
	int mii1_txen;
	int mii1_rxdv;
	int mii1_txd3;
	int mii1_txd2;
	int mii1_txd1;
	int mii1_txd0;
	int mii1_txclk;
	int mii1_rxclk;
	int mii1_rxd3;
	int mii1_rxd2;
	int mii1_rxd1;
	int mii1_rxd0;
	int rmii1_refclk;
	int mdio_data;
	int mdio_clk;
	int spi0_sclk;
	int spi0_d0;
	int spi0_d1;
	int spi0_cs0;
	int spi0_cs1;
	int ecap0_in_pwm0_out;
	int uart0_ctsn;
	int uart0_rtsn;
	int uart0_rxd;
	int uart0_txd;
	int uart1_ctsn;
	int uart1_rtsn;
	int uart1_rxd;
	int uart1_txd;
	int i2c0_sda;
	int i2c0_scl;
	int mcasp0_aclkx;
	int mcasp0_fsx;
	int mcasp0_axr0;
	int mcasp0_ahclkr;
	int mcasp0_aclkr;
	int mcasp0_fsr;
	int mcasp0_axr1;
	int mcasp0_ahclkx;
	int cam0_hd;
	int cam0_vd;
	int cam0_field;
	int cam0_wen;
	int cam0_pclk;
	int cam0_data8;
	int cam0_data9;
	int cam1_data9;
	int cam1_data8;
	int cam1_hd;
	int cam1_vd;
	int cam1_pclk;
	int cam1_field;
	int cam1_wen;
	int cam1_data0;
	int cam1_data1;
	int cam1_data2;
	int cam1_data3;
	int cam1_data4;
	int cam1_data5;
	int cam1_data6;
	int cam1_data7;
	int cam0_data0;
	int cam0_data1;
	int cam0_data2;
	int cam0_data3;
	int cam0_data4;
	int cam0_data5;
	int cam0_data6;
	int cam0_data7;
	int uart3_rxd;
	int uart3_txd;
	int uart3_ctsn;
	int uart3_rtsn;
	int gpio5_8;
	int gpio5_9;
	int gpio5_10;
	int gpio5_11;
	int gpio5_12;
	int gpio5_13;
	int spi4_sclk;
	int spi4_d0;
	int spi4_d1;
	int spi4_cs0;
	int spi2_sclk;
	int spi2_d0;
	int spi2_d1;
	int spi2_cs0;
	int xdma_evt_intr0;
	int xdma_evt_intr1;
	int clkreq;
	int nresetin_out;
	int rsvd1;
	int nnmi;
	int rsvd2;
	int rsvd3;
	int tms;
	int tdi;
	int tdo;
	int tck;
	int ntrst;
	int emu0;
	int emu1;
	int osc1_in;
	int osc1_out;
	int rtc_porz;
	int ext_wakeup0;
	int pmic_power_en0;
	int usb0_drvvbus;
	int usb1_drvvbus;
};

#endif /* _MUX_AM43XX_H_ */
