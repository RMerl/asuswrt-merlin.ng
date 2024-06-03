// SPDX-License-Identifier: GPL-2.0+
/*
 * pinmux setup for siemens rut board
 *
 * (C) Copyright 2013 Siemens Schweiz AG
 * (C) Heiko Schocher, DENX Software Engineering, hs@denx.de.
 *
 * Based on:
 * u-boot:/board/ti/am335x/mux.c
 *
 * Copyright (C) 2011 Texas Instruments Incorporated - http://www.ti.com/
 */

#include <common.h>
#include <asm/arch/sys_proto.h>
#include <asm/arch/hardware.h>
#include <asm/arch/mux.h>
#include <asm/io.h>
#include <i2c.h>

static struct module_pin_mux uart0_pin_mux[] = {
	{OFFSET(uart0_rxd), (MODE(0) | PULLUDDIS | RXACTIVE)},	/* UART0_RXD */
	{OFFSET(uart0_txd), (MODE(0) | PULLUDDIS)},		/* UART0_TXD */
	{-1},
};

static struct module_pin_mux ddr_pin_mux[] = {
	{OFFSET(ddr_resetn), (MODE(0))},
	{OFFSET(ddr_csn0), (MODE(0) | PULLUP_EN)},
	{OFFSET(ddr_ck), (MODE(0))},
	{OFFSET(ddr_nck), (MODE(0))},
	{OFFSET(ddr_casn), (MODE(0) | PULLUP_EN)},
	{OFFSET(ddr_rasn), (MODE(0) | PULLUP_EN)},
	{OFFSET(ddr_wen), (MODE(0) | PULLUP_EN)},
	{OFFSET(ddr_ba0), (MODE(0) | PULLUP_EN)},
	{OFFSET(ddr_ba1), (MODE(0) | PULLUP_EN)},
	{OFFSET(ddr_ba2), (MODE(0) | PULLUP_EN)},
	{OFFSET(ddr_a0), (MODE(0) | PULLUP_EN)},
	{OFFSET(ddr_a1), (MODE(0) | PULLUP_EN)},
	{OFFSET(ddr_a2), (MODE(0) | PULLUP_EN)},
	{OFFSET(ddr_a3), (MODE(0) | PULLUP_EN)},
	{OFFSET(ddr_a4), (MODE(0) | PULLUP_EN)},
	{OFFSET(ddr_a5), (MODE(0) | PULLUP_EN)},
	{OFFSET(ddr_a6), (MODE(0) | PULLUP_EN)},
	{OFFSET(ddr_a7), (MODE(0) | PULLUP_EN)},
	{OFFSET(ddr_a8), (MODE(0) | PULLUP_EN)},
	{OFFSET(ddr_a9), (MODE(0) | PULLUP_EN)},
	{OFFSET(ddr_a10), (MODE(0) | PULLUP_EN)},
	{OFFSET(ddr_a11), (MODE(0) | PULLUP_EN)},
	{OFFSET(ddr_a12), (MODE(0) | PULLUP_EN)},
	{OFFSET(ddr_a13), (MODE(0) | PULLUP_EN)},
	{OFFSET(ddr_a14), (MODE(0) | PULLUP_EN)},
	{OFFSET(ddr_a15), (MODE(0) | PULLUP_EN)},
	{OFFSET(ddr_odt), (MODE(0))},
	{OFFSET(ddr_d0), (MODE(0) | RXACTIVE)},
	{OFFSET(ddr_d1), (MODE(0) | RXACTIVE)},
	{OFFSET(ddr_d2), (MODE(0) | RXACTIVE)},
	{OFFSET(ddr_d3), (MODE(0) | RXACTIVE)},
	{OFFSET(ddr_d4), (MODE(0) | RXACTIVE)},
	{OFFSET(ddr_d5), (MODE(0) | RXACTIVE)},
	{OFFSET(ddr_d6), (MODE(0) | RXACTIVE)},
	{OFFSET(ddr_d7), (MODE(0) | RXACTIVE)},
	{OFFSET(ddr_d8), (MODE(0) | RXACTIVE)},
	{OFFSET(ddr_d9), (MODE(0) | RXACTIVE)},
	{OFFSET(ddr_d10), (MODE(0) | RXACTIVE)},
	{OFFSET(ddr_d11), (MODE(0) | RXACTIVE)},
	{OFFSET(ddr_d12), (MODE(0) | RXACTIVE)},
	{OFFSET(ddr_d13), (MODE(0) | RXACTIVE)},
	{OFFSET(ddr_d14), (MODE(0) | RXACTIVE)},
	{OFFSET(ddr_d15), (MODE(0) | RXACTIVE)},
	{OFFSET(ddr_dqm0), (MODE(0) | PULLUP_EN)},
	{OFFSET(ddr_dqm1), (MODE(0) | PULLUP_EN)},
	{OFFSET(ddr_dqs0), (MODE(0) | RXACTIVE)},
	{OFFSET(ddr_dqsn0), (MODE(0) | RXACTIVE | PULLUP_EN)},
	{OFFSET(ddr_dqs1), (MODE(0) | RXACTIVE)},
	{OFFSET(ddr_dqsn1), (MODE(0) | RXACTIVE | PULLUP_EN)},
	{OFFSET(ddr_vref), (MODE(0) | RXACTIVE | PULLUDDIS)},
	{OFFSET(ddr_vtp), (MODE(0) | RXACTIVE | PULLUDDIS)},
	{-1},
};

static struct module_pin_mux lcd_pin_mux[] = {
	{OFFSET(gpmc_ad8), (MODE(1))},
	{OFFSET(gpmc_ad9), (MODE(1))},
	{OFFSET(gpmc_ad10), (MODE(1))},
	{OFFSET(gpmc_ad11), (MODE(1))},
	{OFFSET(gpmc_ad12), (MODE(1))},
	{OFFSET(gpmc_ad13), (MODE(1))},
	{OFFSET(gpmc_ad14), (MODE(1))},
	{OFFSET(gpmc_ad15), (MODE(1))},
	{OFFSET(lcd_data0), (MODE(0) | PULLUDDIS)},
	{OFFSET(lcd_data1), (MODE(0) | PULLUDDIS)},
	{OFFSET(lcd_data2), (MODE(0) | PULLUDDIS)},
	{OFFSET(lcd_data3), (MODE(0) | PULLUDDIS)},
	{OFFSET(lcd_data4), (MODE(0) | PULLUDDIS)},
	{OFFSET(lcd_data5), (MODE(0) | PULLUDDIS)},
	{OFFSET(lcd_data6), (MODE(0) | PULLUDDIS)},
	{OFFSET(lcd_data7), (MODE(0) | PULLUDDIS)},
	{OFFSET(lcd_data8), (MODE(0) | PULLUDDIS)},
	{OFFSET(lcd_data9), (MODE(0) | PULLUDDIS)},
	{OFFSET(lcd_data10), (MODE(0) | PULLUDDIS)},
	{OFFSET(lcd_data11), (MODE(0) | PULLUDDIS)},
	{OFFSET(lcd_data12), (MODE(0) | PULLUDDIS)},
	{OFFSET(lcd_data13), (MODE(0) | PULLUDDIS)},
	{OFFSET(lcd_data14), (MODE(0) | PULLUDDIS)},
	{OFFSET(lcd_data15), (MODE(0) | PULLUDDIS)},
	{OFFSET(lcd_vsync), (MODE(0))},
	{OFFSET(lcd_hsync), (MODE(0))},
	{OFFSET(lcd_pclk), (MODE(0))},
	{OFFSET(lcd_ac_bias_en), (MODE(0))},
	{-1},
};

static struct module_pin_mux mmc0_pin_mux[] = {
	{OFFSET(mmc0_dat3), (MODE(0) | RXACTIVE | PULLUP_EN)},
	{OFFSET(mmc0_dat2), (MODE(0) | RXACTIVE | PULLUP_EN)},
	{OFFSET(mmc0_dat1), (MODE(0) | RXACTIVE | PULLUP_EN)},
	{OFFSET(mmc0_dat0), (MODE(0) | RXACTIVE | PULLUP_EN)},
	{OFFSET(mmc0_clk), (MODE(0) | RXACTIVE | PULLUP_EN)},
	{OFFSET(mmc0_cmd), (MODE(0) | RXACTIVE | PULLUP_EN)},
	{-1},
};

static struct module_pin_mux mii_pin_mux[] = {
	{OFFSET(mii1_crs), (MODE(1) | RXACTIVE)},
	{OFFSET(mii1_rxerr), (MODE(1) | RXACTIVE)},
	{OFFSET(mii1_txen), (MODE(1))},
	{OFFSET(mii1_txd1), (MODE(1))},
	{OFFSET(mii1_txd0), (MODE(1))},
	{OFFSET(mii1_rxd1), (MODE(1) | RXACTIVE)},
	{OFFSET(mii1_rxd0), (MODE(1) | RXACTIVE)},
	{OFFSET(rmii1_refclk), (MODE(0) | RXACTIVE)},
	{OFFSET(mdio_data), (MODE(0) | RXACTIVE | PULLUP_EN)},
	{OFFSET(mdio_clk), (MODE(0) | PULLUP_EN)},
	{-1},
};

static struct module_pin_mux gpio_pin_mux[] = {
	{OFFSET(mii1_col), (MODE(7) | RXACTIVE)},
	{OFFSET(uart1_ctsn), (MODE(7) | RXACTIVE | PULLUDDIS)},
	{OFFSET(uart1_rtsn), (MODE(7) | RXACTIVE | PULLUDDIS)},
	{OFFSET(uart1_rxd), (MODE(7) | RXACTIVE | PULLUDDIS)},
	{OFFSET(uart1_txd), (MODE(7) | RXACTIVE | PULLUDDIS)},
	{OFFSET(mii1_rxdv), (MODE(7) | RXACTIVE)},
	{OFFSET(mii1_txd3), (MODE(7) | RXACTIVE)},
	{OFFSET(mii1_txd2), (MODE(7) | RXACTIVE)},
	{OFFSET(mii1_txclk), (MODE(7) | RXACTIVE)},
	{OFFSET(mii1_rxclk), (MODE(7) | RXACTIVE)},
	{OFFSET(mii1_rxd3), (MODE(7) | RXACTIVE)},
	{OFFSET(mii1_rxd2), (MODE(7) | RXACTIVE)},
	{OFFSET(gpmc_a0), (MODE(7) | RXACTIVE)},
	{OFFSET(gpmc_a1), (MODE(7) | RXACTIVE)},
	{OFFSET(gpmc_a4), (MODE(7) | RXACTIVE)},
	{OFFSET(gpmc_a5), (MODE(7) | RXACTIVE)},
	{OFFSET(gpmc_a6), (MODE(7) | RXACTIVE)},
	{OFFSET(gpmc_a7), (MODE(7) | RXACTIVE)},
	{OFFSET(gpmc_a8), (MODE(7) | RXACTIVE)},
	{OFFSET(gpmc_a9), (MODE(7) | RXACTIVE)},
	{OFFSET(gpmc_a10), (MODE(7) | RXACTIVE)},
	{OFFSET(gpmc_a11), (MODE(7) | RXACTIVE)},
	{OFFSET(gpmc_wpn), (MODE(7) | RXACTIVE | PULLUP_EN)},
	{OFFSET(gpmc_be1n), (MODE(7) | RXACTIVE | PULLUP_EN)},
	{OFFSET(gpmc_csn1), (MODE(7) | RXACTIVE | PULLUP_EN)},
	{OFFSET(gpmc_csn2), (MODE(7) | RXACTIVE | PULLUP_EN)},
	{OFFSET(gpmc_csn3), (MODE(7) | RXACTIVE | PULLUP_EN)},
	{OFFSET(mcasp0_aclkr), (MODE(7) | RXACTIVE)},
	{OFFSET(mcasp0_fsr), (MODE(7))},
	{OFFSET(mcasp0_axr1), (MODE(7) | RXACTIVE)},
	{OFFSET(mcasp0_ahclkx), (MODE(7) | RXACTIVE)},
	{OFFSET(xdma_event_intr0), (MODE(7) | RXACTIVE | PULLUDDIS)},
	{OFFSET(xdma_event_intr1), (MODE(7) | RXACTIVE | PULLUDDIS)},
	{-1},
};

static struct module_pin_mux i2c0_pin_mux[] = {
	{OFFSET(i2c0_sda), (MODE(0) | RXACTIVE | PULLUDDIS)},
	{OFFSET(i2c0_scl), (MODE(0) | RXACTIVE | PULLUDDIS)},
	{-1},
};

static struct module_pin_mux i2c1_pin_mux[] = {
	{OFFSET(uart0_ctsn), (MODE(3) | RXACTIVE | PULLUDDIS)},
	{OFFSET(uart0_rtsn), (MODE(3) | RXACTIVE | PULLUDDIS)},
	{-1},
};

static struct module_pin_mux usb0_pin_mux[] = {
	{OFFSET(usb0_dm), (MODE(0) | RXACTIVE | PULLUDDIS)},
	{OFFSET(usb0_dp), (MODE(0) | RXACTIVE | PULLUDDIS)},
	{OFFSET(usb0_ce), (MODE(0) | RXACTIVE | PULLUDDIS)},
	{OFFSET(usb0_id), (MODE(0) | RXACTIVE | PULLUDDIS)},
	{OFFSET(usb0_vbus), (MODE(0) | RXACTIVE | PULLUDDIS)},
	{OFFSET(usb0_drvvbus), (MODE(0))},
	{-1},
};

static struct module_pin_mux usb1_pin_mux[] = {
	{OFFSET(usb1_dm), (MODE(0) | RXACTIVE | PULLUDDIS)},
	{OFFSET(usb1_dp), (MODE(0) | RXACTIVE | PULLUDDIS)},
	{OFFSET(usb1_ce), (MODE(0) | RXACTIVE | PULLUDDIS)},
	{OFFSET(usb1_id), (MODE(0) | RXACTIVE | PULLUDDIS)},
	{OFFSET(usb1_vbus), (MODE(0) | RXACTIVE | PULLUDDIS)},
	{OFFSET(usb1_drvvbus), (MODE(0))},
	{-1},
};

static struct module_pin_mux spi0_pin_mux[] = {
	{OFFSET(spi0_sclk), (MODE(0) | RXACTIVE | PULLUDDIS)},
	{OFFSET(spi0_d0), (MODE(0) | RXACTIVE | PULLUDDIS)},
	{OFFSET(spi0_d1), (MODE(0) | RXACTIVE | PULLUDDIS)},
	{OFFSET(spi0_cs0), (MODE(0) | RXACTIVE | PULLUDDIS)},
	{OFFSET(spi0_cs1), (MODE(0) | RXACTIVE | PULLUDDIS)},
	{-1},
};

static struct module_pin_mux spi1_pin_mux[] = {
	{OFFSET(mcasp0_aclkx), (MODE(3) | RXACTIVE | PULLUP_EN)},
	{OFFSET(mcasp0_fsx), (MODE(3) | RXACTIVE | PULLUP_EN)},
	{OFFSET(mcasp0_axr0), (MODE(3) | RXACTIVE | PULLUP_EN)},
	{OFFSET(mcasp0_ahclkr), (MODE(3) | RXACTIVE | PULLUP_EN)},
	{-1},
};

static struct module_pin_mux jtag_pin_mux[] = {
	{OFFSET(tms), (MODE(0) | RXACTIVE | PULLUP_EN)},
	{OFFSET(tdi), (MODE(0) | RXACTIVE | PULLUP_EN)},
	{OFFSET(tdo), (MODE(0) | PULLUP_EN)},
	{OFFSET(tck), (MODE(0) | RXACTIVE | PULLUP_EN)},
	{OFFSET(ntrst), (MODE(0) | RXACTIVE)},
	{-1},
};

static struct module_pin_mux nand_pin_mux[] = {
	{OFFSET(gpmc_ad0), (MODE(0) | RXACTIVE)},
	{OFFSET(gpmc_ad1), (MODE(0) | RXACTIVE)},
	{OFFSET(gpmc_ad2), (MODE(0) | RXACTIVE)},
	{OFFSET(gpmc_ad3), (MODE(0) | RXACTIVE)},
	{OFFSET(gpmc_ad4), (MODE(0) | RXACTIVE)},
	{OFFSET(gpmc_ad5), (MODE(0) | RXACTIVE)},
	{OFFSET(gpmc_ad6), (MODE(0) | RXACTIVE)},
	{OFFSET(gpmc_ad7), (MODE(0) | RXACTIVE)},
	{OFFSET(gpmc_advn_ale), (MODE(0) | PULLUP_EN)},
	{OFFSET(gpmc_be0n_cle), (MODE(0) | PULLUP_EN)},
	{OFFSET(gpmc_csn0), (MODE(0) | PULLUP_EN)},
	{OFFSET(gpmc_oen_ren), (MODE(0) | PULLUP_EN)},
	{OFFSET(gpmc_wen), (MODE(0) | PULLUP_EN)},
	{-1},
};

static struct module_pin_mux ainx_pin_mux[] = {
	{OFFSET(ain7), (MODE(0) | RXACTIVE | PULLUDDIS)},
	{OFFSET(ain6), (MODE(0) | RXACTIVE | PULLUDDIS)},
	{OFFSET(ain5), (MODE(0) | RXACTIVE | PULLUDDIS)},
	{OFFSET(ain4), (MODE(0) | RXACTIVE | PULLUDDIS)},
	{OFFSET(ain3), (MODE(0) | RXACTIVE | PULLUDDIS)},
	{OFFSET(ain2), (MODE(0) | RXACTIVE | PULLUDDIS)},
	{OFFSET(ain1), (MODE(0) | RXACTIVE | PULLUDDIS)},
	{OFFSET(ain0), (MODE(0) | RXACTIVE | PULLUDDIS)},
	{-1},
};

static struct module_pin_mux rtc_pin_mux[] = {
	{OFFSET(osc1_in), (MODE(0) | RXACTIVE | PULLUDDIS)},
	{OFFSET(osc1_out), (MODE(0) | RXACTIVE | PULLUDDIS)},
	{OFFSET(rtc_porz), (MODE(0) | RXACTIVE | PULLUDDIS)},
	{OFFSET(enz_kaldo_1p8v), (MODE(0) | RXACTIVE | PULLUDDIS)},
	{-1},
};

static struct module_pin_mux gpmc_pin_mux[] = {
	{OFFSET(gpmc_wait0), (MODE(0) | RXACTIVE | PULLUP_EN)},
	{OFFSET(gpmc_clk), (MODE(0) | RXACTIVE)},
	{-1},
};

static struct module_pin_mux pmic_pin_mux[] = {
	{OFFSET(pmic_power_en), (MODE(0) | PULLUP_EN)},
	{-1},
};

static struct module_pin_mux osc_pin_mux[] = {
	{OFFSET(osc0_in), (MODE(0) | RXACTIVE | PULLUP_EN)},
	{OFFSET(osc0_out), (MODE(0) | PULLUP_EN)},
	{-1},
};

static struct module_pin_mux pwm_pin_mux[] = {
	{OFFSET(ecap0_in_pwm0_out), (MODE(0) | RXACTIVE | PULLUDDIS)},
	{OFFSET(gpmc_a2), (MODE(6))},
	{OFFSET(gpmc_a3), (MODE(6))},
	{-1},
};

static struct module_pin_mux emu_pin_mux[] = {
	{OFFSET(emu0), (MODE(0) | RXACTIVE | PULLUP_EN)},
	{OFFSET(emu1), (MODE(0) | RXACTIVE | PULLUP_EN)},
	{-1},
};

static struct module_pin_mux vref_pin_mux[] = {
	{OFFSET(vrefp), (MODE(0) | RXACTIVE | PULLUDDIS)},
	{OFFSET(vrefn), (MODE(0) | RXACTIVE | PULLUDDIS)},
	{-1},
};

static struct module_pin_mux misc_pin_mux[] = {
	{OFFSET(porz), (MODE(0) | RXACTIVE | PULLUDDIS)},
	{OFFSET(nnmi), (MODE(0) | RXACTIVE | PULLUDDIS)},
	{OFFSET(ext_wakeup), (MODE(0) | RXACTIVE)},
	{-1},
};

void enable_uart0_pin_mux(void)
{
	configure_module_pin_mux(uart0_pin_mux);
}

void enable_i2c0_pin_mux(void)
{
	configure_module_pin_mux(i2c0_pin_mux);
}

void enable_board_pin_mux(void)
{
	configure_module_pin_mux(ddr_pin_mux);
	configure_module_pin_mux(lcd_pin_mux);
	configure_module_pin_mux(mmc0_pin_mux);
	configure_module_pin_mux(mii_pin_mux);
	configure_module_pin_mux(gpio_pin_mux);
	configure_module_pin_mux(i2c1_pin_mux);
	configure_module_pin_mux(usb0_pin_mux);
	configure_module_pin_mux(usb1_pin_mux);
	configure_module_pin_mux(spi0_pin_mux);
	configure_module_pin_mux(spi1_pin_mux);
	configure_module_pin_mux(jtag_pin_mux);
	configure_module_pin_mux(nand_pin_mux);
	configure_module_pin_mux(ainx_pin_mux);
	configure_module_pin_mux(rtc_pin_mux);
	configure_module_pin_mux(gpmc_pin_mux);
	configure_module_pin_mux(pmic_pin_mux);
	configure_module_pin_mux(osc_pin_mux);
	configure_module_pin_mux(pwm_pin_mux);
	configure_module_pin_mux(emu_pin_mux);
	configure_module_pin_mux(vref_pin_mux);
	configure_module_pin_mux(misc_pin_mux);
}
