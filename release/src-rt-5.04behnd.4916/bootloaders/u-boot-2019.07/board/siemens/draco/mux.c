// SPDX-License-Identifier: GPL-2.0+
/*
 * pinmux setup for siemens draco board
 *
 * (C) Copyright 2013 Siemens Schweiz AG
 * (C) Heiko Schocher, DENX Software Engineering, hs@denx.de.
 *
 * Based on:
 * u-boot:/board/ti/am335x/mux.c
 * Copyright (C) 2011 Texas Instruments Incorporated - http://www.ti.com/
 */

#include <common.h>
#include <asm/arch/sys_proto.h>
#include <asm/arch/hardware.h>
#include <asm/arch/mux.h>
#include <asm/io.h>
#include <i2c.h>
#include "board.h"

static struct module_pin_mux uart0_pin_mux[] = {
	{OFFSET(uart0_rxd), (MODE(0) | PULLUP_EN | RXACTIVE)},	/* UART0_RXD */
	{OFFSET(uart0_txd), (MODE(0) | PULLUDEN)},		/* UART0_TXD */
	{-1},
};

static struct module_pin_mux uart3_pin_mux[] = {
	{OFFSET(spi0_cs1), (MODE(1) | PULLUP_EN | RXACTIVE)},	/* UART3_RXD */
	{OFFSET(ecap0_in_pwm0_out), (MODE(1) | PULLUDEN)},	/* UART3_TXD */
	{-1},
};

static struct module_pin_mux i2c0_pin_mux[] = {
	{OFFSET(i2c0_sda), (MODE(0) | RXACTIVE |
			PULLUDEN | SLEWCTRL)}, /* I2C_DATA */
	{OFFSET(i2c0_scl), (MODE(0) | RXACTIVE |
			PULLUDEN | SLEWCTRL)}, /* I2C_SCLK */
	{-1},
};

static struct module_pin_mux nand_pin_mux[] = {
	{OFFSET(gpmc_ad0), (MODE(0) | PULLUP_EN | RXACTIVE)},	/* NAND AD0 */
	{OFFSET(gpmc_ad1), (MODE(0) | PULLUP_EN | RXACTIVE)},	/* NAND AD1 */
	{OFFSET(gpmc_ad2), (MODE(0) | PULLUP_EN | RXACTIVE)},	/* NAND AD2 */
	{OFFSET(gpmc_ad3), (MODE(0) | PULLUP_EN | RXACTIVE)},	/* NAND AD3 */
	{OFFSET(gpmc_ad4), (MODE(0) | PULLUP_EN | RXACTIVE)},	/* NAND AD4 */
	{OFFSET(gpmc_ad5), (MODE(0) | PULLUP_EN | RXACTIVE)},	/* NAND AD5 */
	{OFFSET(gpmc_ad6), (MODE(0) | PULLUP_EN | RXACTIVE)},	/* NAND AD6 */
	{OFFSET(gpmc_ad7), (MODE(0) | PULLUP_EN | RXACTIVE)},	/* NAND AD7 */
	{OFFSET(gpmc_wait0), (MODE(0) | RXACTIVE | PULLUP_EN)}, /* NAND WAIT */
	{OFFSET(gpmc_wpn), (MODE(7) | PULLUP_EN | RXACTIVE)},	/* NAND_WPN */
	{OFFSET(gpmc_csn0), (MODE(0) | PULLUDEN)},	/* NAND_CS0 */
	{OFFSET(gpmc_csn1), MODE(0) | PULLUDEN | PULLUP_EN},    /* NAND_CS1 */
	{OFFSET(gpmc_advn_ale), (MODE(0) | PULLUDEN)}, /* NAND_ADV_ALE */
	{OFFSET(gpmc_oen_ren), (MODE(0) | PULLUDEN)},	/* NAND_OE */
	{OFFSET(gpmc_wen), (MODE(0) | PULLUDEN)},	/* NAND_WEN */
	{OFFSET(gpmc_be0n_cle), (MODE(0) | PULLUDEN)},	/* NAND_BE_CLE */
	{-1},
};

static struct module_pin_mux gpios_pin_mux[] = {
	/* DFU button GPIO0_27*/
	{OFFSET(gpmc_ad11), (MODE(7) | PULLUDEN | PULLUP_EN | RXACTIVE)},
	{OFFSET(gpmc_csn3), MODE(7) },			/* LED0 GPIO2_0 */
	{OFFSET(emu0), MODE(7)},			/* LED1 GPIO3_7 */
	/* Triacs in HW Rev 2 */
	{OFFSET(uart1_ctsn), MODE(7) | PULLUDDIS | RXACTIVE},	/* Y5 GPIO0_12*/
	{OFFSET(mmc0_dat1), MODE(7) | PULLUDDIS | RXACTIVE},	/* Y3 GPIO2_28*/
	{OFFSET(mmc0_dat2), MODE(7) | PULLUDDIS | RXACTIVE},	/* Y7 GPIO2_27*/
	/* Triacs initial HW Rev */
	{OFFSET(gpmc_be1n), MODE(7) | RXACTIVE | PULLUDDIS},	/* 1_28 Y1 */
	{OFFSET(gpmc_csn2), MODE(7) | RXACTIVE | PULLUDDIS},	/* 1_31 Y2 */
	{OFFSET(lcd_data15), MODE(7) | RXACTIVE | PULLUDDIS},	/* 0_11 Y3 */
	{OFFSET(lcd_data14), MODE(7) | RXACTIVE | PULLUDDIS},	/* 0_10 Y4 */
	{OFFSET(gpmc_clk), MODE(7) | RXACTIVE | PULLUDDIS},	/* 2_1  Y5 */
	{OFFSET(emu1), MODE(7) | RXACTIVE | PULLUDDIS},		/* 3_8  Y6 */
	{OFFSET(gpmc_ad15), MODE(7) | RXACTIVE | PULLUDDIS},	/* 1_15 Y7 */
	/* Remaining pins that were not used in this file */
	{OFFSET(gpmc_ad8), MODE(7) | RXACTIVE | PULLUDDIS},
	{OFFSET(gpmc_ad9), MODE(7) | RXACTIVE | PULLUDDIS},
	{OFFSET(gpmc_a0), MODE(7) | RXACTIVE | PULLUDDIS},
	{OFFSET(gpmc_a1), MODE(7) | RXACTIVE | PULLUDDIS},
	{OFFSET(gpmc_a2), MODE(7) | RXACTIVE | PULLUDDIS},
	{OFFSET(gpmc_a3), MODE(7) | RXACTIVE | PULLUDDIS},
	{OFFSET(gpmc_a4), MODE(7) | RXACTIVE | PULLUDDIS},
	{OFFSET(gpmc_a5), MODE(7) | RXACTIVE | PULLUDDIS},
	{OFFSET(gpmc_a6), MODE(7) | RXACTIVE | PULLUDDIS},
	{OFFSET(gpmc_a7), MODE(7) | RXACTIVE | PULLUDDIS},
	{OFFSET(gpmc_a8), MODE(7) | RXACTIVE | PULLUDDIS},
	{OFFSET(gpmc_a9), MODE(7) | RXACTIVE | PULLUDDIS},
	{OFFSET(gpmc_a10), MODE(7) | RXACTIVE | PULLUDDIS},
	{OFFSET(gpmc_a11), MODE(7) | RXACTIVE | PULLUDDIS},
	{OFFSET(lcd_data0), MODE(7) | RXACTIVE | PULLUDDIS},
	{OFFSET(lcd_data2), MODE(7) | RXACTIVE | PULLUDDIS},
	{OFFSET(lcd_data3), MODE(7) | RXACTIVE | PULLUDDIS},
	{OFFSET(lcd_data4), MODE(7) | RXACTIVE | PULLUDDIS},
	{OFFSET(lcd_data5), MODE(7) | RXACTIVE | PULLUDDIS},
	{OFFSET(lcd_data6), MODE(7) | RXACTIVE | PULLUDDIS},
	{OFFSET(lcd_data7), MODE(7) | RXACTIVE | PULLUDDIS},
	{OFFSET(lcd_data8), MODE(7) | RXACTIVE | PULLUDDIS},
	{OFFSET(lcd_data9), MODE(7) | RXACTIVE | PULLUDDIS},
	{OFFSET(lcd_vsync), MODE(7) | RXACTIVE | PULLUDDIS},
	{OFFSET(lcd_hsync), MODE(7) | RXACTIVE | PULLUDDIS},
	{OFFSET(lcd_pclk), MODE(7) | RXACTIVE | PULLUDDIS},
	{OFFSET(lcd_ac_bias_en), MODE(7) | RXACTIVE | PULLUDDIS},
	{OFFSET(mmc0_dat3), MODE(7) | RXACTIVE | PULLUDDIS},
	{OFFSET(mmc0_dat0), MODE(7) | RXACTIVE | PULLUDDIS},
	{OFFSET(mmc0_clk), MODE(7) | RXACTIVE | PULLUDDIS},
	{OFFSET(mmc0_cmd), MODE(7) | RXACTIVE | PULLUDDIS},
	{OFFSET(spi0_sclk), MODE(7) | RXACTIVE | PULLUDDIS},
	{OFFSET(spi0_d0), MODE(7) | RXACTIVE | PULLUDDIS},
	{OFFSET(spi0_d1), MODE(7) | RXACTIVE | PULLUDDIS},
	{OFFSET(spi0_cs0), MODE(7) | RXACTIVE | PULLUDDIS},
	{OFFSET(uart0_ctsn), MODE(7) | RXACTIVE | PULLUDDIS},
	{OFFSET(uart0_rtsn), MODE(7) | RXACTIVE | PULLUDDIS},
	{OFFSET(uart1_rtsn), MODE(7) | RXACTIVE | PULLUDDIS},
	{OFFSET(uart1_rxd), MODE(7) | RXACTIVE | PULLUDDIS},
	{OFFSET(uart1_txd), MODE(7) | RXACTIVE | PULLUDDIS},
	{OFFSET(mcasp0_aclkx), MODE(7) | RXACTIVE | PULLUDDIS},
	{OFFSET(mcasp0_fsx), MODE(7) | RXACTIVE | PULLUDDIS},
	{OFFSET(mcasp0_axr0), MODE(7) | RXACTIVE | PULLUDDIS},
	{OFFSET(mcasp0_ahclkr), MODE(7) | RXACTIVE | PULLUDDIS},
	{OFFSET(mcasp0_aclkr), MODE(7) | RXACTIVE | PULLUDDIS},
	{OFFSET(mcasp0_fsr), MODE(7) | RXACTIVE | PULLUDDIS},
	{OFFSET(mcasp0_axr1), MODE(7) | RXACTIVE | PULLUDDIS},
	{OFFSET(mcasp0_ahclkx), MODE(7) | RXACTIVE | PULLUDDIS},
	{OFFSET(xdma_event_intr0), MODE(7) | RXACTIVE | PULLUDDIS},
	{OFFSET(xdma_event_intr1), MODE(7) | RXACTIVE | PULLUDDIS},
	{OFFSET(nresetin_out), MODE(7) | RXACTIVE | PULLUDDIS},
	{OFFSET(porz), MODE(7) | RXACTIVE | PULLUDDIS},
	{OFFSET(nnmi), MODE(7) | RXACTIVE | PULLUDDIS},
	{OFFSET(osc0_in), MODE(7) | RXACTIVE | PULLUDDIS},
	{OFFSET(osc0_out), MODE(7) | RXACTIVE | PULLUDDIS},
	{OFFSET(rsvd1), MODE(7) | RXACTIVE | PULLUDDIS},
	{OFFSET(tms), MODE(7) | RXACTIVE | PULLUDDIS},
	{OFFSET(tdi), MODE(7) | RXACTIVE | PULLUDDIS},
	{OFFSET(tdo), MODE(7) | RXACTIVE | PULLUDDIS},
	{OFFSET(tck), MODE(7) | RXACTIVE | PULLUDDIS},
	{OFFSET(ntrst), MODE(7) | RXACTIVE | PULLUDDIS},
	{OFFSET(osc1_in), MODE(7) | RXACTIVE | PULLUDDIS},
	{OFFSET(osc1_out), MODE(7) | RXACTIVE | PULLUDDIS},
	{OFFSET(pmic_power_en), MODE(7) | RXACTIVE | PULLUDDIS},
	{OFFSET(rtc_porz), MODE(7) | RXACTIVE | PULLUDDIS},
	{OFFSET(rsvd2), MODE(7) | RXACTIVE | PULLUDDIS},
	{OFFSET(ext_wakeup), MODE(7) | RXACTIVE | PULLUDDIS},
	{OFFSET(enz_kaldo_1p8v), MODE(7) | RXACTIVE | PULLUDDIS},
	{OFFSET(usb0_dm), MODE(7) | RXACTIVE | PULLUDDIS},
	{OFFSET(usb0_dp), MODE(7) | RXACTIVE | PULLUDDIS},
	{OFFSET(usb0_ce), MODE(7) | RXACTIVE | PULLUDDIS},
	{OFFSET(usb0_id), MODE(7) | RXACTIVE | PULLUDDIS},
	{OFFSET(usb0_vbus), MODE(7) | RXACTIVE | PULLUDDIS},
	{OFFSET(usb0_drvvbus), MODE(7) | RXACTIVE | PULLUDDIS},
	{OFFSET(usb1_dm), MODE(7) | RXACTIVE | PULLUDDIS},
	{OFFSET(usb1_dp), MODE(7) | RXACTIVE | PULLUDDIS},
	{OFFSET(usb1_ce), MODE(7) | RXACTIVE | PULLUDDIS},
	{OFFSET(usb1_id), MODE(7) | RXACTIVE | PULLUDDIS},
	{OFFSET(usb1_vbus), MODE(7) | RXACTIVE | PULLUDDIS},
	{OFFSET(usb1_drvvbus), MODE(7) | RXACTIVE | PULLUDDIS},
	{OFFSET(ddr_resetn), MODE(7) | RXACTIVE | PULLUDDIS},
	{OFFSET(ddr_csn0), MODE(7) | RXACTIVE | PULLUDDIS},
	{OFFSET(ddr_cke), MODE(7) | RXACTIVE | PULLUDDIS},
	{OFFSET(ddr_ck), MODE(7) | RXACTIVE | PULLUDDIS},
	{OFFSET(ddr_nck), MODE(7) | RXACTIVE | PULLUDDIS},
	{OFFSET(ddr_casn), MODE(7) | RXACTIVE | PULLUDDIS},
	{OFFSET(ddr_rasn), MODE(7) | RXACTIVE | PULLUDDIS},
	{OFFSET(ddr_wen), MODE(7) | RXACTIVE | PULLUDDIS},
	{OFFSET(ddr_ba0), MODE(7) | RXACTIVE | PULLUDDIS},
	{OFFSET(ddr_ba1), MODE(7) | RXACTIVE | PULLUDDIS},
	{OFFSET(ddr_ba2), MODE(7) | RXACTIVE | PULLUDDIS},
	{OFFSET(ddr_a0), MODE(7) | RXACTIVE | PULLUDDIS},
	{OFFSET(ddr_a1), MODE(7) | RXACTIVE | PULLUDDIS},
	{OFFSET(ddr_a2), MODE(7) | RXACTIVE | PULLUDDIS},
	{OFFSET(ddr_a3), MODE(7) | RXACTIVE | PULLUDDIS},
	{OFFSET(ddr_a4), MODE(7) | RXACTIVE | PULLUDDIS},
	{OFFSET(ddr_a5), MODE(7) | RXACTIVE | PULLUDDIS},
	{OFFSET(ddr_a6), MODE(7) | RXACTIVE | PULLUDDIS},
	{OFFSET(ddr_a7), MODE(7) | RXACTIVE | PULLUDDIS},
	{OFFSET(ddr_a8), MODE(7) | RXACTIVE | PULLUDDIS},
	{OFFSET(ddr_a9), MODE(7) | RXACTIVE | PULLUDDIS},
	{OFFSET(ddr_a10), MODE(7) | RXACTIVE | PULLUDDIS},
	{OFFSET(ddr_a11), MODE(7) | RXACTIVE | PULLUDDIS},
	{OFFSET(ddr_a12), MODE(7) | RXACTIVE | PULLUDDIS},
	{OFFSET(ddr_a13), MODE(7) | RXACTIVE | PULLUDDIS},
	{OFFSET(ddr_a14), MODE(7) | RXACTIVE | PULLUDDIS},
	{OFFSET(ddr_a15), MODE(7) | RXACTIVE | PULLUDDIS},
	{OFFSET(ddr_odt), MODE(7) | RXACTIVE | PULLUDDIS},
	{OFFSET(ddr_d0), MODE(7) | RXACTIVE | PULLUDDIS},
	{OFFSET(ddr_d1), MODE(7) | RXACTIVE | PULLUDDIS},
	{OFFSET(ddr_d2), MODE(7) | RXACTIVE | PULLUDDIS},
	{OFFSET(ddr_d3), MODE(7) | RXACTIVE | PULLUDDIS},
	{OFFSET(ddr_d4), MODE(7) | RXACTIVE | PULLUDDIS},
	{OFFSET(ddr_d5), MODE(7) | RXACTIVE | PULLUDDIS},
	{OFFSET(ddr_d6), MODE(7) | RXACTIVE | PULLUDDIS},
	{OFFSET(ddr_d7), MODE(7) | RXACTIVE | PULLUDDIS},
	{OFFSET(ddr_d8), MODE(7) | RXACTIVE | PULLUDDIS},
	{OFFSET(ddr_d9), MODE(7) | RXACTIVE | PULLUDDIS},
	{OFFSET(ddr_d10), MODE(7) | RXACTIVE | PULLUDDIS},
	{OFFSET(ddr_d11), MODE(7) | RXACTIVE | PULLUDDIS},
	{OFFSET(ddr_d12), MODE(7) | RXACTIVE | PULLUDDIS},
	{OFFSET(ddr_d13), MODE(7) | RXACTIVE | PULLUDDIS},
	{OFFSET(ddr_d14), MODE(7) | RXACTIVE | PULLUDDIS},
	{OFFSET(ddr_d15), MODE(7) | RXACTIVE | PULLUDDIS},
	{OFFSET(ddr_dqm0), MODE(7) | RXACTIVE | PULLUDDIS},
	{OFFSET(ddr_dqm1), MODE(7) | RXACTIVE | PULLUDDIS},
	{OFFSET(ddr_dqs0), MODE(7) | RXACTIVE | PULLUDDIS},
	{OFFSET(ddr_dqsn0), MODE(7) | RXACTIVE | PULLUDDIS},
	{OFFSET(ddr_dqs1), MODE(7) | RXACTIVE | PULLUDDIS},
	{OFFSET(ddr_dqsn1), MODE(7) | RXACTIVE | PULLUDDIS},
	{OFFSET(ddr_vref), MODE(7) | RXACTIVE | PULLUDDIS},
	{OFFSET(ddr_vtp), MODE(7) | RXACTIVE | PULLUDDIS},
	{OFFSET(ddr_strben0), MODE(7) | RXACTIVE | PULLUDDIS},
	{OFFSET(ddr_strben1), MODE(7) | RXACTIVE | PULLUDDIS},
	{OFFSET(ain7), MODE(7) | RXACTIVE | PULLUDDIS},
	{OFFSET(ain6), MODE(7) | RXACTIVE | PULLUDDIS},
	{OFFSET(ain5), MODE(7) | RXACTIVE | PULLUDDIS},
	{OFFSET(ain4), MODE(7) | RXACTIVE | PULLUDDIS},
	{OFFSET(ain3), MODE(7) | RXACTIVE | PULLUDDIS},
	{OFFSET(ain2), MODE(7) | RXACTIVE | PULLUDDIS},
	{OFFSET(ain1), MODE(7) | RXACTIVE | PULLUDDIS},
	{OFFSET(ain0), MODE(7) | RXACTIVE | PULLUDDIS},
	{OFFSET(vrefp), MODE(7) | RXACTIVE | PULLUDDIS},
	{OFFSET(vrefn), MODE(7) | RXACTIVE | PULLUDDIS},
	/* nRST for SMSC LAN9303 switch - GPIO2_24 */
	{OFFSET(lcd_pclk), MODE(7) | PULLUDEN | PULLUP_EN }, /* LAN9303 nRST */
	{-1},
};

static struct module_pin_mux ethernet_pin_mux[] = {
	{OFFSET(mii1_col), (MODE(3) | RXACTIVE)},
	{OFFSET(mii1_crs), (MODE(1) | RXACTIVE)},
	{OFFSET(mii1_rxerr), (MODE(1) | RXACTIVE)},
	{OFFSET(mii1_txen), (MODE(1))},
	{OFFSET(mii1_rxdv), (MODE(3) | RXACTIVE)},
	{OFFSET(mii1_txd3), (MODE(7) | RXACTIVE)},
	{OFFSET(mii1_txd2), (MODE(7) | RXACTIVE)},
	{OFFSET(mii1_txd1), (MODE(1))},
	{OFFSET(mii1_txd0), (MODE(1))},
	{OFFSET(mii1_txclk), (MODE(1) | RXACTIVE)},
	{OFFSET(mii1_rxclk), (MODE(1) | RXACTIVE)},
	{OFFSET(mii1_rxd3), (MODE(1) | RXACTIVE)},
	{OFFSET(mii1_rxd2), (MODE(1))},
	{OFFSET(mii1_rxd1), (MODE(1) | RXACTIVE)},
	{OFFSET(mii1_rxd0), (MODE(1) | RXACTIVE)},
	{OFFSET(rmii1_refclk), (MODE(0) | RXACTIVE)},
	{OFFSET(mdio_data), (MODE(0) | RXACTIVE | PULLUP_EN)},
	{OFFSET(mdio_clk), (MODE(0) | PULLUP_EN)},
	{-1},
};

void enable_uart0_pin_mux(void)
{
	configure_module_pin_mux(uart0_pin_mux);
}

void enable_uart3_pin_mux(void)
{
	configure_module_pin_mux(uart3_pin_mux);
}

void enable_i2c0_pin_mux(void)
{
	configure_module_pin_mux(i2c0_pin_mux);
}

void enable_board_pin_mux(void)
{
	enable_uart3_pin_mux();
	configure_module_pin_mux(nand_pin_mux);
	configure_module_pin_mux(ethernet_pin_mux);
	configure_module_pin_mux(gpios_pin_mux);
}
