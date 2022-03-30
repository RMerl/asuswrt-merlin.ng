/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2013
 * Texas Instruments Incorporated, <www.ti.com>
 *
 * Sricharan R	<r.sricharan@ti.com>
 * Nishant Kamat <nskamat@ti.com>
 */
#ifndef _MUX_DATA_DRA7XX_H_
#define _MUX_DATA_DRA7XX_H_

#include <asm/arch/mux_dra7xx.h>

const struct pad_conf_entry dra72x_core_padconf_array_common[] = {
	{GPMC_AD0, (M3 | PIN_INPUT)},	/* gpmc_ad0.vout3_d0 */
	{GPMC_AD1, (M3 | PIN_INPUT)},	/* gpmc_ad1.vout3_d1 */
	{GPMC_AD2, (M3 | PIN_INPUT)},	/* gpmc_ad2.vout3_d2 */
	{GPMC_AD3, (M3 | PIN_INPUT)},	/* gpmc_ad3.vout3_d3 */
	{GPMC_AD4, (M3 | PIN_INPUT)},	/* gpmc_ad4.vout3_d4 */
	{GPMC_AD5, (M3 | PIN_INPUT)},	/* gpmc_ad5.vout3_d5 */
	{GPMC_AD6, (M3 | PIN_INPUT)},	/* gpmc_ad6.vout3_d6 */
	{GPMC_AD7, (M3 | PIN_INPUT)},	/* gpmc_ad7.vout3_d7 */
	{GPMC_AD8, (M3 | PIN_INPUT)},	/* gpmc_ad8.vout3_d8 */
	{GPMC_AD9, (M3 | PIN_INPUT)},	/* gpmc_ad9.vout3_d9 */
	{GPMC_AD10, (M3 | PIN_INPUT)},	/* gpmc_ad10.vout3_d10 */
	{GPMC_AD11, (M3 | PIN_INPUT)},	/* gpmc_ad11.vout3_d11 */
	{GPMC_AD12, (M3 | PIN_INPUT)},	/* gpmc_ad12.vout3_d12 */
	{GPMC_AD13, (M3 | PIN_INPUT)},	/* gpmc_ad13.vout3_d13 */
	{GPMC_AD14, (M3 | PIN_INPUT)},	/* gpmc_ad14.vout3_d14 */
	{GPMC_AD15, (M3 | PIN_INPUT)},	/* gpmc_ad15.vout3_d15 */
	{GPMC_A0, (M3 | PIN_INPUT_PULLDOWN)},	/* gpmc_a0.vout3_d16 */
	{GPMC_A1, (M3 | PIN_INPUT_PULLDOWN)},	/* gpmc_a1.vout3_d17 */
	{GPMC_A2, (M3 | PIN_INPUT_PULLDOWN)},	/* gpmc_a2.vout3_d18 */
	{GPMC_A3, (M3 | PIN_INPUT_PULLDOWN)},	/* gpmc_a3.vout3_d19 */
	{GPMC_A4, (M3 | PIN_INPUT_PULLDOWN)},	/* gpmc_a4.vout3_d20 */
	{GPMC_A5, (M3 | PIN_INPUT_PULLDOWN)},	/* gpmc_a5.vout3_d21 */
	{GPMC_A6, (M3 | PIN_INPUT_PULLDOWN)},	/* gpmc_a6.vout3_d22 */
	{GPMC_A7, (M3 | PIN_INPUT_PULLDOWN)},	/* gpmc_a7.vout3_d23 */
	{GPMC_A8, (M3 | PIN_INPUT_PULLDOWN)},	/* gpmc_a8.vout3_hsync */
	{GPMC_A9, (M3 | PIN_INPUT_PULLDOWN)},	/* gpmc_a9.vout3_vsync */
	{GPMC_A10, (M3 | PIN_INPUT_PULLDOWN)},	/* gpmc_a10.vout3_de */
	{GPMC_A11, (M14 | PIN_INPUT_PULLDOWN)},	/* gpmc_a11.gpio2_1 */
	{GPMC_A13, (M1 | PIN_INPUT_PULLDOWN | MANUAL_MODE)}, /* gpmc_a13.qspi1_rtclk */
	{GPMC_A14, (M1 | PIN_INPUT_PULLDOWN | MANUAL_MODE)}, /* gpmc_a14.qspi1_d3 */
	{GPMC_A15, (M1 | PIN_INPUT_PULLDOWN | MANUAL_MODE)}, /* gpmc_a15.qspi1_d2 */
	{GPMC_A16, (M1 | PIN_INPUT_PULLDOWN | MANUAL_MODE)}, /* gpmc_a16.qspi1_d0 */
	{GPMC_A17, (M1 | PIN_INPUT_PULLDOWN | MANUAL_MODE)}, /* gpmc_a17.qspi1_d1 */
	{GPMC_A18, (M1 | PIN_INPUT_PULLDOWN | MANUAL_MODE)}, /* gpmc_a18.qspi1_sclk */
	{GPMC_A19, (M1 | PIN_INPUT_PULLUP)},	/* gpmc_a19.mmc2_dat4 */
	{GPMC_A20, (M1 | PIN_INPUT_PULLUP)},	/* gpmc_a20.mmc2_dat5 */
	{GPMC_A21, (M1 | PIN_INPUT_PULLUP)},	/* gpmc_a21.mmc2_dat6 */
	{GPMC_A22, (M1 | PIN_INPUT_PULLUP)},	/* gpmc_a22.mmc2_dat7 */
	{GPMC_A23, (M1 | PIN_INPUT_PULLUP)},	/* gpmc_a23.mmc2_clk */
	{GPMC_A24, (M1 | PIN_INPUT_PULLUP)},	/* gpmc_a24.mmc2_dat0 */
	{GPMC_A25, (M1 | PIN_INPUT_PULLUP)},	/* gpmc_a25.mmc2_dat1 */
	{GPMC_A26, (M1 | PIN_INPUT_PULLUP)},	/* gpmc_a26.mmc2_dat2 */
	{GPMC_A27, (M1 | PIN_INPUT_PULLUP)},	/* gpmc_a27.mmc2_dat3 */
	{GPMC_CS1, (M1 | PIN_INPUT_PULLUP)},	/* gpmc_cs1.mmc2_cmd */
	{GPMC_CS2, (M1 | PIN_INPUT_PULLUP | MANUAL_MODE)}, /* gpmc_cs2.qspi1_cs0 */
	{GPMC_CS3, (M3 | PIN_INPUT_PULLUP)},	/* gpmc_cs3.vout3_clk */
	{VIN2A_CLK0, (M0 | PIN_INPUT_PULLDOWN | VIRTUAL_MODE9)},	/* vin2a_clk0.vin2a_clk0 */
	{VIN2A_HSYNC0, (M0 | PIN_INPUT_PULLDOWN | VIRTUAL_MODE6)},	/* vin2a_hsync0.vin2a_hsync0 */
	{VIN2A_VSYNC0, (M0 | PIN_INPUT_PULLDOWN | VIRTUAL_MODE9)},	/* vin2a_vsync0.vin2a_vsync0 */
	{VIN2A_D0, (M0 | PIN_INPUT_PULLDOWN | VIRTUAL_MODE14)},	/* vin2a_d0.vin2a_d0 */
	{VIN2A_D1, (M0 | PIN_INPUT_PULLDOWN | VIRTUAL_MODE14)},	/* vin2a_d1.vin2a_d1 */
	{VIN2A_D2, (M0 | PIN_INPUT_PULLDOWN | VIRTUAL_MODE14)},	/* vin2a_d2.vin2a_d2 */
	{VIN2A_D3, (M0 | PIN_INPUT_PULLDOWN | VIRTUAL_MODE8)},	/* vin2a_d3.vin2a_d3 */
	{VIN2A_D4, (M0 | PIN_INPUT_PULLDOWN | VIRTUAL_MODE8)},	/* vin2a_d4.vin2a_d4 */
	{VIN2A_D5, (M0 | PIN_INPUT_PULLDOWN | VIRTUAL_MODE8)},	/* vin2a_d5.vin2a_d5 */
	{VIN2A_D6, (M0 | PIN_INPUT_PULLDOWN | VIRTUAL_MODE6)},	/* vin2a_d6.vin2a_d6 */
	{VIN2A_D7, (M0 | PIN_INPUT_PULLDOWN | VIRTUAL_MODE6)},	/* vin2a_d7.vin2a_d7 */
	{VIN2A_D8, (M0 | PIN_INPUT_PULLDOWN | VIRTUAL_MODE2)},	/* vin2a_d8.vin2a_d8 */
	{VIN2A_D9, (M0 | PIN_INPUT_PULLDOWN | VIRTUAL_MODE2)},	/* vin2a_d9.vin2a_d9 */
	{VIN2A_D10, (M0 | PIN_INPUT_PULLDOWN | VIRTUAL_MODE5)},	/* vin2a_d10.vin2a_d10 */
	{VIN2A_D11, (M0 | PIN_INPUT_PULLDOWN | VIRTUAL_MODE5)},	/* vin2a_d11.vin2a_d11 */
	{VOUT1_CLK, (M0 | PIN_INPUT_PULLDOWN)},	/* vout1_clk.vout1_clk */
	{VOUT1_DE, (M0 | PIN_INPUT_PULLDOWN)},	/* vout1_de.vout1_de */
	{VOUT1_HSYNC, (M0 | PIN_INPUT_PULLDOWN | MANUAL_MODE)},	/* vout1_hsync.vout1_hsync */
	{VOUT1_VSYNC, (M0 | PIN_INPUT_PULLDOWN)},	/* vout1_vsync.vout1_vsync */
	{VOUT1_D0, (M0 | PIN_INPUT_PULLDOWN)},	/* vout1_d0.vout1_d0 */
	{VOUT1_D1, (M0 | PIN_INPUT_PULLDOWN)},	/* vout1_d1.vout1_d1 */
	{VOUT1_D2, (M0 | PIN_INPUT_PULLDOWN)},	/* vout1_d2.vout1_d2 */
	{VOUT1_D3, (M0 | PIN_INPUT_PULLDOWN)},	/* vout1_d3.vout1_d3 */
	{VOUT1_D4, (M0 | PIN_INPUT_PULLDOWN)},	/* vout1_d4.vout1_d4 */
	{VOUT1_D5, (M0 | PIN_INPUT_PULLDOWN)},	/* vout1_d5.vout1_d5 */
	{VOUT1_D6, (M0 | PIN_INPUT_PULLDOWN)},	/* vout1_d6.vout1_d6 */
	{VOUT1_D7, (M0 | PIN_INPUT_PULLDOWN)},	/* vout1_d7.vout1_d7 */
	{VOUT1_D8, (M0 | PIN_INPUT_PULLDOWN)},	/* vout1_d8.vout1_d8 */
	{VOUT1_D9, (M0 | PIN_INPUT_PULLDOWN)},	/* vout1_d9.vout1_d9 */
	{VOUT1_D10, (M0 | PIN_INPUT_PULLDOWN)},	/* vout1_d10.vout1_d10 */
	{VOUT1_D11, (M0 | PIN_INPUT_PULLDOWN)},	/* vout1_d11.vout1_d11 */
	{VOUT1_D12, (M0 | PIN_INPUT_PULLDOWN)},	/* vout1_d12.vout1_d12 */
	{VOUT1_D13, (M0 | PIN_INPUT_PULLDOWN)},	/* vout1_d13.vout1_d13 */
	{VOUT1_D14, (M0 | PIN_INPUT_PULLDOWN)},	/* vout1_d14.vout1_d14 */
	{VOUT1_D15, (M0 | PIN_INPUT_PULLDOWN)},	/* vout1_d15.vout1_d15 */
	{VOUT1_D16, (M0 | PIN_INPUT_PULLDOWN)},	/* vout1_d16.vout1_d16 */
	{VOUT1_D17, (M0 | PIN_INPUT_PULLDOWN)},	/* vout1_d17.vout1_d17 */
	{VOUT1_D18, (M0 | PIN_INPUT_PULLDOWN)},	/* vout1_d18.vout1_d18 */
	{VOUT1_D19, (M0 | PIN_INPUT_PULLDOWN)},	/* vout1_d19.vout1_d19 */
	{VOUT1_D20, (M0 | PIN_INPUT_PULLDOWN)},	/* vout1_d20.vout1_d20 */
	{VOUT1_D21, (M0 | PIN_INPUT_PULLDOWN)},	/* vout1_d21.vout1_d21 */
	{VOUT1_D22, (M0 | PIN_INPUT_PULLDOWN)},	/* vout1_d22.vout1_d22 */
	{VOUT1_D23, (M0 | PIN_INPUT_PULLDOWN)},	/* vout1_d23.vout1_d23 */
	{MDIO_MCLK, (M0 | PIN_INPUT_PULLUP | SLEWCONTROL)},	/* mdio_mclk.mdio_mclk */
	{MDIO_D, (M0 | PIN_INPUT_PULLUP | SLEWCONTROL)},	/* mdio_d.mdio_d */
	{USB1_DRVVBUS, (M0 | PIN_INPUT_SLEW)},	/* usb1_drvvbus.usb1_drvvbus */
	{USB2_DRVVBUS, (M0 | PIN_INPUT_SLEW)},	/* usb2_drvvbus.usb2_drvvbus */
	{GPIO6_14, (M9 | PIN_INPUT_PULLUP)},	/* gpio6_14.i2c3_sda */
	{GPIO6_15, (M9 | PIN_INPUT_PULLUP)},	/* gpio6_15.i2c3_scl */
	{GPIO6_16, (M14 | PIN_INPUT_PULLUP)},	/* gpio6_16.gpio6_16 */
	{MCASP1_AXR0, (M10 | PIN_INPUT_SLEW)},	/* mcasp1_axr0.i2c5_sda */
	{MCASP1_AXR1, (M10 | PIN_INPUT_SLEW)},	/* mcasp1_axr1.i2c5_scl */
	{MCASP1_AXR2, (M14 | PIN_INPUT_PULLDOWN)},	/* mcasp1_axr2.gpio5_4 */
	{MCASP1_AXR3, (M14 | PIN_INPUT_PULLDOWN)},	/* mcasp1_axr3.gpio5_5 */
	{MCASP1_AXR4, (M14 | PIN_INPUT_PULLDOWN)},	/* mcasp1_axr4.gpio5_6 */
	{MCASP1_AXR5, (M14 | PIN_INPUT_PULLDOWN)},	/* mcasp1_axr5.gpio5_7 */
	{MCASP1_AXR6, (M14 | PIN_INPUT_PULLDOWN)},	/* mcasp1_axr6.gpio5_8 */
	{MCASP1_AXR7, (M14 | PIN_INPUT_PULLDOWN)},	/* mcasp1_axr7.gpio5_9 */
	{MCASP1_AXR12, (M1 | PIN_INPUT_SLEW | VIRTUAL_MODE10)},	/* mcasp1_axr12.mcasp7_axr0 */
	{MCASP1_AXR13, (M1 | PIN_INPUT_SLEW)},	/* mcasp1_axr13.mcasp7_axr1 */
	{MCASP1_AXR14, (M1 | PIN_INPUT_SLEW | VIRTUAL_MODE10)},	/* mcasp1_axr14.mcasp7_aclkx */
	{MCASP1_AXR15, (M1 | PIN_INPUT_SLEW | VIRTUAL_MODE10)},	/* mcasp1_axr15.mcasp7_fsx */
	{MCASP2_ACLKR, (M0 | PIN_INPUT_PULLDOWN)},	/* mcasp2_aclkr.mcasp2_aclkr */
	{MCASP3_ACLKX, (M0 | PIN_INPUT_PULLDOWN)},	/* mcasp3_aclkx.mcasp3_aclkx */
	{MCASP3_FSX, (M0 | PIN_INPUT_SLEW)},	/* mcasp3_fsx.mcasp3_fsx */
	{MCASP3_AXR0, (M0 | PIN_INPUT_SLEW)},	/* mcasp3_axr0.mcasp3_axr0 */
	{MCASP3_AXR1, (M0 | PIN_INPUT_SLEW | VIRTUAL_MODE6)},	/* mcasp3_axr1.mcasp3_axr1 */
	{MMC1_CLK, (M0 | PIN_INPUT_PULLUP)},	/* mmc1_clk.mmc1_clk */
	{MMC1_CMD, (M0 | PIN_INPUT_PULLUP)},	/* mmc1_cmd.mmc1_cmd */
	{MMC1_DAT0, (M0 | PIN_INPUT_PULLUP)},	/* mmc1_dat0.mmc1_dat0 */
	{MMC1_DAT1, (M0 | PIN_INPUT_PULLUP)},	/* mmc1_dat1.mmc1_dat1 */
	{MMC1_DAT2, (M0 | PIN_INPUT_PULLUP)},	/* mmc1_dat2.mmc1_dat2 */
	{MMC1_DAT3, (M0 | PIN_INPUT_PULLUP)},	/* mmc1_dat3.mmc1_dat3 */
	{MMC1_SDCD, (M14 | PIN_INPUT_PULLUP)},	/* mmc1_sdcd.gpio6_27 */
	{MMC1_SDWP, (M14 | PIN_INPUT_SLEW)},	/* mmc1_sdwp.gpio6_28 */
	{SPI1_SCLK, (M0 | PIN_INPUT_PULLDOWN)},	/* spi1_sclk.spi1_sclk */
	{SPI1_D1, (M0 | PIN_INPUT_PULLDOWN)},	/* spi1_d1.spi1_d1 */
	{SPI1_D0, (M0 | PIN_INPUT_PULLDOWN)},	/* spi1_d0.spi1_d0 */
	{SPI1_CS0, (M0 | PIN_INPUT_PULLUP)},	/* spi1_cs0.spi1_cs0 */
	{SPI1_CS1, (M14 | PIN_OUTPUT)},	/* spi1_cs1.gpio7_11 */
	{SPI1_CS2, (M14 | PIN_INPUT_PULLDOWN)},	/* spi1_cs2.gpio7_12 */
	{SPI1_CS3, (M6 | PIN_INPUT | SLEWCONTROL)},	/* spi1_cs3.hdmi1_cec */
	{SPI2_SCLK, (M1 | PIN_INPUT_PULLDOWN)},	/* spi2_sclk.uart3_rxd */
	{SPI2_D1, (M1 | PIN_INPUT_SLEW)},	/* spi2_d1.uart3_txd */
	{SPI2_D0, (M1 | PIN_INPUT_SLEW)},	/* spi2_d0.uart3_ctsn */
	{SPI2_CS0, (M1 | PIN_INPUT_PULLUP | SLEWCONTROL)},	/* spi2_cs0.uart3_rtsn */
	{DCAN1_TX, (M15 | PULL_UP)},	/* dcan1_tx.safe for dcan1_tx */
	{DCAN1_RX, (M14 | PIN_INPUT_PULLUP | SLEWCONTROL)},	/* dcan1_rx.gpio1_15 */
	{UART1_RXD, (M0 | PIN_INPUT_PULLUP | SLEWCONTROL)},	/* uart1_rxd.uart1_rxd */
	{UART1_TXD, (M0 | PIN_INPUT_PULLUP | SLEWCONTROL)},	/* uart1_txd.uart1_txd */
	{UART1_CTSN, (M3 | PIN_INPUT_PULLUP)},	/* uart1_ctsn.mmc4_clk */
	{UART1_RTSN, (M3 | PIN_INPUT_PULLUP)},	/* uart1_rtsn.mmc4_cmd */
	{UART2_RXD, (M3 | PIN_INPUT_PULLUP)},	/* uart2_rxd.mmc4_dat0 */
	{UART2_TXD, (M3 | PIN_INPUT_PULLUP)},	/* uart2_txd.mmc4_dat1 */
	{UART2_CTSN, (M3 | PIN_INPUT_PULLUP)},	/* uart2_ctsn.mmc4_dat2 */
	{UART2_RTSN, (M3 | PIN_INPUT_PULLUP)},	/* uart2_rtsn.mmc4_dat3 */
	{I2C2_SDA, (M1 | PIN_INPUT_PULLUP)},	/* i2c2_sda.hdmi1_ddc_scl */
	{I2C2_SCL, (M1 | PIN_INPUT_PULLUP)},	/* i2c2_scl.hdmi1_ddc_sda */
	{WAKEUP0, (M15 | PULL_UP)},	/* Wakeup0.safe for dcan1_rx */
	{WAKEUP3, (M1 | PULL_ENA | PULL_UP)},	/* Wakeup3.sys_nirq1 */
};

const struct pad_conf_entry dra72x_rgmii_padconf_array_revb[] = {
	{GPIO6_11, (M14 | PIN_INPUT_PULLUP)},	/* gpio6_11.gpio6_11 */
	{RGMII0_TXC, (M0 | PIN_OUTPUT | MANUAL_MODE)},	/* rgmii0_txc.rgmii0_txc */
	{RGMII0_TXCTL, (M0 | PIN_OUTPUT | MANUAL_MODE)},	/* rgmii0_txctl.rgmii0_txctl */
	{RGMII0_TXD3, (M0 | PIN_OUTPUT | MANUAL_MODE)},	/* rgmii0_txd3.rgmii0_txd3 */
	{RGMII0_TXD2, (M0 | PIN_OUTPUT | MANUAL_MODE)},	/* rgmii0_txd2.rgmii0_txd2 */
	{RGMII0_TXD1, (M0 | PIN_OUTPUT | MANUAL_MODE)},	/* rgmii0_txd1.rgmii0_txd1 */
	{RGMII0_TXD0, (M0 | PIN_OUTPUT | MANUAL_MODE)},	/* rgmii0_txd0.rgmii0_txd0 */
	{RGMII0_RXC, (M0 | PIN_INPUT | MANUAL_MODE)},	/* rgmii0_rxc.rgmii0_rxc */
	{RGMII0_RXCTL, (M0 | PIN_INPUT | MANUAL_MODE)},	/* rgmii0_rxctl.rgmii0_rxctl */
	{RGMII0_RXD3, (M0 | PIN_INPUT | MANUAL_MODE)},	/* rgmii0_rxd3.rgmii0_rxd3 */
	{RGMII0_RXD2, (M0 | PIN_INPUT | MANUAL_MODE)},	/* rgmii0_rxd2.rgmii0_rxd2 */
	{RGMII0_RXD1, (M0 | PIN_INPUT | MANUAL_MODE)},	/* rgmii0_rxd1.rgmii0_rxd1 */
	{RGMII0_RXD0, (M0 | PIN_INPUT | MANUAL_MODE)},	/* rgmii0_rxd0.rgmii0_rxd0 */
	{VIN2A_D12, (M3 | PIN_OUTPUT | MANUAL_MODE)},	/* vin2a_d0.rgmii1_txc */
	{VIN2A_D13, (M3 | PIN_OUTPUT | MANUAL_MODE)},	/* vin2a_d1.rgmii1_txctl */
	{VIN2A_D14, (M3 | PIN_OUTPUT | MANUAL_MODE)},	/* vin2a_d2.rgmii1_txd3 */
	{VIN2A_D15, (M3 | PIN_OUTPUT | MANUAL_MODE)},	/* vin2a_d3.rgmii1_txd2 */
	{VIN2A_D16, (M3 | PIN_OUTPUT | MANUAL_MODE)},	/* vin2a_d4.rgmii1_txd1 */
	{VIN2A_D17, (M3 | PIN_OUTPUT | MANUAL_MODE)},	/* vin2a_d5.rgmii1_txd0 */
	{VIN2A_D18, (M3 | PIN_INPUT | MANUAL_MODE)},	/* vin2a_d6.rgmii1_rxc */
	{VIN2A_D19, (M3 | PIN_INPUT | MANUAL_MODE)},	/* vin2a_d7.rgmii1_rxctl */
	{VIN2A_D20, (M3 | PIN_INPUT | MANUAL_MODE)},	/* vin2a_d8.rgmii1_rxd3 */
	{VIN2A_D21, (M3 | PIN_INPUT | MANUAL_MODE)},	/* vin2a_d9.rgmii1_rxd2 */
	{VIN2A_D22, (M3 | PIN_INPUT | MANUAL_MODE)},	/* vin2a_d10.rgmii1_rxd1 */
	{VIN2A_D23, (M3 | PIN_INPUT | MANUAL_MODE)},	/* vin2a_d11.rgmii1_rxd0 */
	{XREF_CLK1, (M5 | PIN_OUTPUT)},	/* xref_clk1.atl_clk1 */
	{XREF_CLK2, (M5 | PIN_OUTPUT)},	/* xref_clk2.atl_clk2 */
};

const struct pad_conf_entry dra72x_rgmii_padconf_array_revc[] = {
	{VIN2A_FLD0, (M14 | PIN_INPUT)},	/* vin2a_fld0.gpio3_30 */
	{RGMII0_TXC, (M0 | PIN_OUTPUT | MANUAL_MODE)},	/* rgmii0_txc.rgmii0_txc */
	{RGMII0_TXCTL, (M0 | PIN_OUTPUT | MANUAL_MODE)},	/* rgmii0_txctl.rgmii0_txctl */
	{RGMII0_TXD3, (M0 | PIN_OUTPUT | MANUAL_MODE)},	/* rgmii0_txd3.rgmii0_txd3 */
	{RGMII0_TXD2, (M0 | PIN_OUTPUT | MANUAL_MODE)},	/* rgmii0_txd2.rgmii0_txd2 */
	{RGMII0_TXD1, (M0 | PIN_OUTPUT | MANUAL_MODE)},	/* rgmii0_txd1.rgmii0_txd1 */
	{RGMII0_TXD0, (M0 | PIN_OUTPUT | MANUAL_MODE)},	/* rgmii0_txd0.rgmii0_txd0 */
	{RGMII0_RXC, (M0 | PIN_INPUT_PULLDOWN | MANUAL_MODE)},	/* rgmii0_rxc.rgmii0_rxc */
	{RGMII0_RXCTL, (M0 | PIN_INPUT_PULLDOWN | MANUAL_MODE)},	/* rgmii0_rxctl.rgmii0_rxctl */
	{RGMII0_RXD3, (M0 | PIN_INPUT_PULLDOWN | MANUAL_MODE)},	/* rgmii0_rxd3.rgmii0_rxd3 */
	{RGMII0_RXD2, (M0 | PIN_INPUT_PULLDOWN | MANUAL_MODE)},	/* rgmii0_rxd2.rgmii0_rxd2 */
	{RGMII0_RXD1, (M0 | PIN_INPUT_PULLDOWN | MANUAL_MODE)},	/* rgmii0_rxd1.rgmii0_rxd1 */
	{RGMII0_RXD0, (M0 | PIN_INPUT_PULLDOWN | MANUAL_MODE)},	/* rgmii0_rxd0.rgmii0_rxd0 */
	{VIN2A_D12, (M3 | PIN_OUTPUT | MANUAL_MODE)},	/* vin2a_d12.rgmii1_txc */
	{VIN2A_D13, (M3 | PIN_OUTPUT | MANUAL_MODE)},	/* vin2a_d13.rgmii1_txctl */
	{VIN2A_D14, (M3 | PIN_OUTPUT | MANUAL_MODE)},	/* vin2a_d14.rgmii1_txd3 */
	{VIN2A_D15, (M3 | PIN_OUTPUT | MANUAL_MODE)},	/* vin2a_d15.rgmii1_txd2 */
	{VIN2A_D16, (M3 | PIN_OUTPUT | MANUAL_MODE)},	/* vin2a_d16.rgmii1_txd1 */
	{VIN2A_D17, (M3 | PIN_OUTPUT | MANUAL_MODE)},	/* vin2a_d17.rgmii1_txd0 */
	{VIN2A_D18, (M3 | PIN_INPUT_PULLDOWN | MANUAL_MODE)},	/* vin2a_d18.rgmii1_rxc */
	{VIN2A_D19, (M3 | PIN_INPUT_PULLDOWN | MANUAL_MODE)},	/* vin2a_d19.rgmii1_rxctl */
	{VIN2A_D20, (M3 | PIN_INPUT_PULLDOWN | MANUAL_MODE)},	/* vin2a_d20.rgmii1_rxd3 */
	{VIN2A_D21, (M3 | PIN_INPUT_PULLDOWN | MANUAL_MODE)},	/* vin2a_d21.rgmii1_rxd2 */
	{VIN2A_D22, (M3 | PIN_INPUT_PULLDOWN | MANUAL_MODE)},	/* vin2a_d22.rgmii1_rxd1 */
	{VIN2A_D23, (M3 | PIN_INPUT_PULLDOWN | MANUAL_MODE)},	/* vin2a_d23.rgmii1_rxd0 */
	{XREF_CLK2, (M5 | PIN_INPUT_PULLDOWN)},	/* xref_clk2.atl_clk2 */
};

const struct pad_conf_entry dra71x_core_padconf_array[] = {
	{GPMC_A0, (M3 | PIN_INPUT_PULLDOWN)},	/* gpmc_a0.vout3_d16 */
	{GPMC_A1, (M3 | PIN_INPUT_PULLDOWN)},	/* gpmc_a1.vout3_d17 */
	{GPMC_A2, (M3 | PIN_INPUT_PULLDOWN)},	/* gpmc_a2.vout3_d18 */
	{GPMC_A3, (M3 | PIN_INPUT_PULLDOWN)},	/* gpmc_a3.vout3_d19 */
	{GPMC_A4, (M3 | PIN_INPUT_PULLDOWN)},	/* gpmc_a4.vout3_d20 */
	{GPMC_A5, (M3 | PIN_INPUT_PULLDOWN)},	/* gpmc_a5.vout3_d21 */
	{GPMC_A6, (M3 | PIN_INPUT_PULLDOWN)},	/* gpmc_a6.vout3_d22 */
	{GPMC_A7, (M3 | PIN_INPUT_PULLDOWN)},	/* gpmc_a7.vout3_d23 */
	{GPMC_A8, (M3 | PIN_INPUT_PULLDOWN)},	/* gpmc_a8.vout3_hsync */
	{GPMC_A9, (M3 | PIN_INPUT_PULLDOWN)},	/* gpmc_a9.vout3_vsync */
	{GPMC_A10, (M3 | PIN_INPUT_PULLDOWN)},	/* gpmc_a10.vout3_de */
	{GPMC_A11, (M14 | PIN_INPUT)},	/* gpmc_a11.gpio2_1 */
	{GPMC_A13, (M1 | PIN_INPUT_PULLDOWN | MANUAL_MODE)},	/* gpmc_a13.qspi1_rtclk */
	{GPMC_A14, (M1 | PIN_INPUT_PULLUP | MANUAL_MODE)},	/* gpmc_a14.qspi1_d3 */
	{GPMC_A15, (M1 | PIN_INPUT_PULLUP | MANUAL_MODE)},	/* gpmc_a15.qspi1_d2 */
	{GPMC_A16, (M1 | PIN_INPUT_PULLDOWN | MANUAL_MODE)},	/* gpmc_a16.qspi1_d0 */
	{GPMC_A17, (M1 | PIN_INPUT_PULLDOWN | MANUAL_MODE)},	/* gpmc_a17.qspi1_d1 */
	{GPMC_A18, (M1 | PIN_INPUT_PULLDOWN | MANUAL_MODE)},	/* gpmc_a18.qspi1_sclk */
	{GPMC_A19, (M1 | PIN_INPUT_PULLUP)},	/* gpmc_a19.mmc2_dat4 */
	{GPMC_A20, (M1 | PIN_INPUT_PULLUP)},	/* gpmc_a20.mmc2_dat5 */
	{GPMC_A21, (M1 | PIN_INPUT_PULLUP)},	/* gpmc_a21.mmc2_dat6 */
	{GPMC_A22, (M1 | PIN_INPUT_PULLUP)},	/* gpmc_a22.mmc2_dat7 */
	{GPMC_A23, (M1 | PIN_INPUT_PULLUP)},	/* gpmc_a23.mmc2_clk */
	{GPMC_A24, (M1 | PIN_INPUT_PULLUP)},	/* gpmc_a24.mmc2_dat0 */
	{GPMC_A25, (M1 | PIN_INPUT_PULLUP)},	/* gpmc_a25.mmc2_dat1 */
	{GPMC_A26, (M1 | PIN_INPUT_PULLUP)},	/* gpmc_a26.mmc2_dat2 */
	{GPMC_A27, (M1 | PIN_INPUT_PULLUP)},	/* gpmc_a27.mmc2_dat3 */
	{GPMC_CS1, (M1 | PIN_INPUT_PULLUP)},	/* gpmc_cs1.mmc2_cmd */
	{GPMC_CS2, (M1 | PIN_INPUT_PULLUP | MANUAL_MODE)},	/* gpmc_cs2.qspi1_cs0 */
	{GPMC_CS3, (M3 | PIN_INPUT_PULLUP)},	/* gpmc_cs3.vout3_clk */
	{VIN2A_CLK0, (M0 | PIN_INPUT_PULLDOWN | VIRTUAL_MODE9)},	/* vin2a_clk0.vin2a_clk0 */
	{VIN2A_FLD0, (M14 | PIN_INPUT)},	/* vin2a_fld0.gpio3_30 */
	{VIN2A_HSYNC0, (M0 | PIN_INPUT_PULLDOWN | VIRTUAL_MODE6)},	/* vin2a_hsync0.vin2a_hsync0 */
	{VIN2A_VSYNC0, (M0 | PIN_INPUT_PULLDOWN | VIRTUAL_MODE9)},	/* vin2a_vsync0.vin2a_vsync0 */
	{VIN2A_D0, (M0 | PIN_INPUT_PULLDOWN | VIRTUAL_MODE14)},	/* vin2a_d0.vin2a_d0 */
	{VIN2A_D1, (M0 | PIN_INPUT_PULLDOWN | VIRTUAL_MODE14)},	/* vin2a_d1.vin2a_d1 */
	{VIN2A_D2, (M0 | PIN_INPUT_PULLDOWN | VIRTUAL_MODE14)},	/* vin2a_d2.vin2a_d2 */
	{VIN2A_D3, (M0 | PIN_INPUT_PULLDOWN | VIRTUAL_MODE8)},	/* vin2a_d3.vin2a_d3 */
	{VIN2A_D4, (M0 | PIN_INPUT_PULLDOWN | VIRTUAL_MODE8)},	/* vin2a_d4.vin2a_d4 */
	{VIN2A_D5, (M0 | PIN_INPUT_PULLDOWN | VIRTUAL_MODE8)},	/* vin2a_d5.vin2a_d5 */
	{VIN2A_D6, (M0 | PIN_INPUT_PULLDOWN | VIRTUAL_MODE6)},	/* vin2a_d6.vin2a_d6 */
	{VIN2A_D7, (M0 | PIN_INPUT_PULLDOWN | VIRTUAL_MODE6)},	/* vin2a_d7.vin2a_d7 */
	{VIN2A_D8, (M0 | PIN_INPUT_PULLDOWN | VIRTUAL_MODE2)},	/* vin2a_d8.vin2a_d8 */
	{VIN2A_D9, (M0 | PIN_INPUT_PULLDOWN | VIRTUAL_MODE2)},	/* vin2a_d9.vin2a_d9 */
	{VIN2A_D10, (M0 | PIN_INPUT_PULLDOWN | VIRTUAL_MODE5)},	/* vin2a_d10.vin2a_d10 */
	{VIN2A_D11, (M0 | PIN_INPUT_PULLDOWN | VIRTUAL_MODE5)},	/* vin2a_d11.vin2a_d11 */
	{VIN2A_D12, (M3 | PIN_OUTPUT | MANUAL_MODE)},	/* vin2a_d12.rgmii1_txc */
	{VIN2A_D13, (M3 | PIN_OUTPUT | MANUAL_MODE)},	/* vin2a_d13.rgmii1_txctl */
	{VIN2A_D14, (M3 | PIN_OUTPUT | MANUAL_MODE)},	/* vin2a_d14.rgmii1_txd3 */
	{VIN2A_D15, (M3 | PIN_OUTPUT | MANUAL_MODE)},	/* vin2a_d15.rgmii1_txd2 */
	{VIN2A_D16, (M3 | PIN_OUTPUT | MANUAL_MODE)},	/* vin2a_d16.rgmii1_txd1 */
	{VIN2A_D17, (M3 | PIN_OUTPUT | MANUAL_MODE)},	/* vin2a_d17.rgmii1_txd0 */
	{VIN2A_D18, (M3 | PIN_INPUT_PULLDOWN | MANUAL_MODE)},	/* vin2a_d18.rgmii1_rxc */
	{VIN2A_D19, (M3 | PIN_INPUT_PULLDOWN | MANUAL_MODE)},	/* vin2a_d19.rgmii1_rxctl */
	{VIN2A_D20, (M3 | PIN_INPUT_PULLDOWN | MANUAL_MODE)},	/* vin2a_d20.rgmii1_rxd3 */
	{VIN2A_D21, (M3 | PIN_INPUT_PULLDOWN | MANUAL_MODE)},	/* vin2a_d21.rgmii1_rxd2 */
	{VIN2A_D22, (M3 | PIN_INPUT_PULLDOWN | MANUAL_MODE)},	/* vin2a_d22.rgmii1_rxd1 */
	{VIN2A_D23, (M3 | PIN_INPUT_PULLDOWN | MANUAL_MODE)},	/* vin2a_d23.rgmii1_rxd0 */
	{VOUT1_D2, (M0 | PIN_INPUT_PULLDOWN)},	/* N/A.N/A */
	{VOUT1_D10, (M0 | PIN_INPUT_PULLDOWN)},	/* N/A.N/A */
	{VOUT1_D18, (M0 | PIN_INPUT_PULLDOWN)},	/* N/A.N/A */
	{MDIO_MCLK, (M0 | PIN_INPUT_PULLUP | SLEWCONTROL)},	/* mdio_mclk.mdio_mclk */
	{MDIO_D, (M0 | PIN_INPUT_PULLUP | SLEWCONTROL)},	/* mdio_d.mdio_d */
	{RGMII0_TXC, (M0 | PIN_OUTPUT | MANUAL_MODE)},	/* rgmii0_txc.rgmii0_txc */
	{RGMII0_TXCTL, (M0 | PIN_OUTPUT | MANUAL_MODE)},	/* rgmii0_txctl.rgmii0_txctl */
	{RGMII0_TXD3, (M0 | PIN_OUTPUT | MANUAL_MODE)},	/* rgmii0_txd3.rgmii0_txd3 */
	{RGMII0_TXD2, (M0 | PIN_OUTPUT | MANUAL_MODE)},	/* rgmii0_txd2.rgmii0_txd2 */
	{RGMII0_TXD1, (M0 | PIN_OUTPUT | MANUAL_MODE)},	/* rgmii0_txd1.rgmii0_txd1 */
	{RGMII0_TXD0, (M0 | PIN_OUTPUT | MANUAL_MODE)},	/* rgmii0_txd0.rgmii0_txd0 */
	{RGMII0_RXC, (M0 | PIN_INPUT_PULLDOWN | MANUAL_MODE)},	/* rgmii0_rxc.rgmii0_rxc */
	{RGMII0_RXCTL, (M0 | PIN_INPUT_PULLDOWN | MANUAL_MODE)},	/* rgmii0_rxctl.rgmii0_rxctl */
	{RGMII0_RXD3, (M0 | PIN_INPUT_PULLDOWN | MANUAL_MODE)},	/* rgmii0_rxd3.rgmii0_rxd3 */
	{RGMII0_RXD2, (M0 | PIN_INPUT_PULLDOWN | MANUAL_MODE)},	/* rgmii0_rxd2.rgmii0_rxd2 */
	{RGMII0_RXD1, (M0 | PIN_INPUT_PULLDOWN | MANUAL_MODE)},	/* rgmii0_rxd1.rgmii0_rxd1 */
	{RGMII0_RXD0, (M0 | PIN_INPUT_PULLDOWN | MANUAL_MODE)},	/* rgmii0_rxd0.rgmii0_rxd0 */
	{USB1_DRVVBUS, (M0 | PIN_INPUT_SLEW)},	/* usb1_drvvbus.usb1_drvvbus */
	{USB2_DRVVBUS, (M0 | PIN_INPUT_SLEW)},	/* usb2_drvvbus.usb2_drvvbus */
	{GPIO6_14, (M9 | PIN_INPUT_PULLUP)},	/* gpio6_14.i2c3_sda */
	{GPIO6_15, (M9 | PIN_INPUT_PULLUP)},	/* gpio6_15.i2c3_scl */
	{GPIO6_16, (M14 | PIN_INPUT_PULLUP)},	/* gpio6_16.gpio6_16 */
	{XREF_CLK2, (M5 | PIN_INPUT_PULLDOWN)},	/* xref_clk2.atl_clk2 */
	{MCASP1_ACLKX, (M14 | PIN_INPUT)},	/* mcasp1_aclkx.gpio7_31 */
	{MCASP1_FSX, (M14 | 0x000d0000)},	/* mcasp1_fsx.gpio7_30 */
	{MCASP1_AXR0, (M10 | PIN_INPUT_PULLUP | SLEWCONTROL)},	/* mcasp1_axr0.i2c5_sda */
	{MCASP1_AXR1, (M10 | PIN_INPUT_PULLUP | SLEWCONTROL)},	/* mcasp1_axr1.i2c5_scl */
	{MCASP1_AXR2, (M14 | PIN_INPUT_PULLDOWN)},	/* mcasp1_axr2.gpio5_4 */
	{MCASP1_AXR3, (M14 | PIN_INPUT_PULLDOWN)},	/* mcasp1_axr3.gpio5_5 */
	{MCASP1_AXR4, (M14 | PIN_INPUT_PULLDOWN)},	/* mcasp1_axr4.gpio5_6 */
	{MCASP1_AXR5, (M14 | PIN_INPUT_PULLDOWN)},	/* mcasp1_axr5.gpio5_7 */
	{MCASP1_AXR6, (M14 | PIN_INPUT_PULLDOWN)},	/* mcasp1_axr6.gpio5_8 */
	{MCASP1_AXR7, (M14 | PIN_INPUT_PULLDOWN)},	/* mcasp1_axr7.gpio5_9 */
	{MCASP1_AXR12, (M1 | PIN_INPUT_SLEW | VIRTUAL_MODE10)},	/* mcasp1_axr12.mcasp7_axr0 */
	{MCASP1_AXR13, (M1 | PIN_INPUT_SLEW)},	/* mcasp1_axr13.mcasp7_axr1 */
	{MCASP1_AXR14, (M1 | PIN_INPUT_SLEW | VIRTUAL_MODE10)},	/* mcasp1_axr14.mcasp7_aclkx */
	{MCASP1_AXR15, (M1 | PIN_INPUT_SLEW | VIRTUAL_MODE10)},	/* mcasp1_axr15.mcasp7_fsx */
	{MCASP3_ACLKX, (M0 | PIN_INPUT_PULLDOWN)},	/* mcasp3_aclkx.mcasp3_aclkx */
	{MCASP3_FSX, (M0 | PIN_INPUT_SLEW)},	/* mcasp3_fsx.mcasp3_fsx */
	{MCASP3_AXR0, (M0 | PIN_INPUT_SLEW)},	/* mcasp3_axr0.mcasp3_axr0 */
	{MCASP3_AXR1, (M0 | PIN_INPUT_SLEW | VIRTUAL_MODE6)},	/* mcasp3_axr1.mcasp3_axr1 */
	{MMC1_CLK, (M0 | PIN_INPUT_PULLUP)},	/* mmc1_clk.mmc1_clk */
	{MMC1_CMD, (M0 | PIN_INPUT_PULLUP)},	/* mmc1_cmd.mmc1_cmd */
	{MMC1_DAT0, (M0 | PIN_INPUT_PULLUP)},	/* mmc1_dat0.mmc1_dat0 */
	{MMC1_DAT1, (M0 | PIN_INPUT_PULLUP)},	/* mmc1_dat1.mmc1_dat1 */
	{MMC1_DAT2, (M0 | PIN_INPUT_PULLUP)},	/* mmc1_dat2.mmc1_dat2 */
	{MMC1_DAT3, (M0 | PIN_INPUT_PULLUP)},	/* mmc1_dat3.mmc1_dat3 */
	{MMC1_SDCD, (M14 | PIN_INPUT_PULLUP | SLEWCONTROL)},	/* mmc1_sdcd.gpio6_27 */
	{MMC1_SDWP, (M14 | PIN_INPUT_SLEW)},	/* mmc1_sdwp.gpio6_28 */
	{SPI1_SCLK, (M0 | PIN_INPUT_PULLDOWN)},	/* spi1_sclk.spi1_sclk */
	{SPI1_D1, (M0 | PIN_INPUT_PULLDOWN)},	/* spi1_d1.spi1_d1 */
	{SPI1_D0, (M0 | PIN_INPUT_PULLDOWN)},	/* spi1_d0.spi1_d0 */
	{SPI1_CS0, (M0 | PIN_INPUT_PULLUP)},	/* spi1_cs0.spi1_cs0 */
	{SPI1_CS1, (M14 | PIN_INPUT_PULLUP)},	/* spi1_cs1.gpio7_11 */
	{SPI1_CS2, (M14 | PIN_INPUT_PULLDOWN)},	/* spi1_cs2.gpio7_12 */
	{SPI1_CS3, (M6 | PIN_INPUT | SLEWCONTROL)},	/* spi1_cs3.hdmi1_cec */
	{SPI2_SCLK, (M1 | PIN_INPUT_PULLDOWN)},	/* spi2_sclk.uart3_rxd */
	{SPI2_D1, (M1 | PIN_INPUT_SLEW)},	/* spi2_d1.uart3_txd */
	{SPI2_D0, (M1 | PIN_INPUT_SLEW)},	/* spi2_d0.uart3_ctsn */
	{SPI2_CS0, (M1 | PIN_INPUT_PULLUP | SLEWCONTROL)},	/* spi2_cs0.uart3_rtsn */
	{DCAN1_TX, (M15 | PULL_UP)},	/* dcan1_tx.safe for dcan1_tx */
	{DCAN1_RX, (M14 | PIN_INPUT_PULLUP | SLEWCONTROL)},	/* dcan1_rx.gpio1_15 */
	{UART1_RXD, (M0 | PIN_INPUT_PULLUP | SLEWCONTROL)},	/* uart1_rxd.uart1_rxd */
	{UART1_TXD, (M0 | PIN_INPUT_PULLUP | SLEWCONTROL)},	/* uart1_txd.uart1_txd */
	{UART1_CTSN, (M3 | PIN_INPUT_PULLUP)},	/* uart1_ctsn.mmc4_clk */
	{UART1_RTSN, (M3 | PIN_INPUT_PULLUP)},	/* uart1_rtsn.mmc4_cmd */
	{UART2_RXD, (M3 | PIN_INPUT_PULLUP)},	/* uart2_rxd.mmc4_dat0 */
	{UART2_TXD, (M3 | PIN_INPUT_PULLUP)},	/* uart2_txd.mmc4_dat1 */
	{UART2_CTSN, (M3 | PIN_INPUT_PULLUP)},	/* uart2_ctsn.mmc4_dat2 */
	{UART2_RTSN, (M3 | PIN_INPUT_PULLUP)},	/* uart2_rtsn.mmc4_dat3 */
	{I2C2_SDA, (M1 | PIN_INPUT_PULLUP)},	/* i2c2_sda.hdmi1_ddc_scl */
	{I2C2_SCL, (M1 | PIN_INPUT_PULLUP)},	/* i2c2_scl.hdmi1_ddc_sda */
	{WAKEUP0, (M15 | PULL_UP)},	/* Wakeup0.safe for dcan1_rx */
	{WAKEUP3, (M1 | PULL_ENA | PULL_UP)},	/* Wakeup3.sys_nirq1 */
};

const struct pad_conf_entry dra71x_vout3_padconf_array[] = {
	{GPMC_AD0, (M3 | PIN_INPUT)},	/* gpmc_ad0.vout3_d0 */
	{GPMC_AD1, (M3 | PIN_INPUT)},	/* gpmc_ad1.vout3_d1 */
	{GPMC_AD2, (M3 | PIN_INPUT)},	/* gpmc_ad2.vout3_d2 */
	{GPMC_AD3, (M3 | PIN_INPUT)},	/* gpmc_ad3.vout3_d3 */
	{GPMC_AD4, (M3 | PIN_INPUT)},	/* gpmc_ad4.vout3_d4 */
	{GPMC_AD5, (M3 | PIN_INPUT)},	/* gpmc_ad5.vout3_d5 */
	{GPMC_AD6, (M3 | PIN_INPUT)},	/* gpmc_ad6.vout3_d6 */
	{GPMC_AD7, (M3 | PIN_INPUT)},	/* gpmc_ad7.vout3_d7 */
	{GPMC_AD8, (M3 | PIN_INPUT)},	/* gpmc_ad8.vout3_d8 */
	{GPMC_AD9, (M3 | PIN_INPUT)},	/* gpmc_ad9.vout3_d9 */
	{GPMC_AD10, (M3 | PIN_INPUT)},	/* gpmc_ad10.vout3_d10 */
	{GPMC_AD11, (M3 | PIN_INPUT)},	/* gpmc_ad11.vout3_d11 */
	{GPMC_AD12, (M3 | PIN_INPUT)},	/* gpmc_ad12.vout3_d12 */
	{GPMC_AD13, (M3 | PIN_INPUT)},	/* gpmc_ad13.vout3_d13 */
	{GPMC_AD14, (M3 | PIN_INPUT)},	/* gpmc_ad14.vout3_d14 */
	{GPMC_AD15, (M3 | PIN_INPUT)},	/* gpmc_ad15.vout3_d15 */
};

const struct pad_conf_entry dra71x_nand_padconf_array[] = {
	{GPMC_AD0, (M0 | PIN_INPUT)},	/* gpmc_ad0.gpmc_ad0 */
	{GPMC_AD1, (M0 | PIN_INPUT)},	/* gpmc_ad1.gpmc_ad1 */
	{GPMC_AD2, (M0 | PIN_INPUT)},	/* gpmc_ad2.gpmc_ad2 */
	{GPMC_AD3, (M0 | PIN_INPUT)},	/* gpmc_ad3.gpmc_ad3 */
	{GPMC_AD4, (M0 | PIN_INPUT)},	/* gpmc_ad4.gpmc_ad4 */
	{GPMC_AD5, (M0 | PIN_INPUT)},	/* gpmc_ad5.gpmc_ad5 */
	{GPMC_AD6, (M0 | PIN_INPUT)},	/* gpmc_ad6.gpmc_ad6 */
	{GPMC_AD7, (M0 | PIN_INPUT)},	/* gpmc_ad7.gpmc_ad7 */
	{GPMC_AD8, (M0 | PIN_INPUT)},	/* gpmc_ad8.gpmc_ad8 */
	{GPMC_AD9, (M0 | PIN_INPUT)},	/* gpmc_ad9.gpmc_ad9 */
	{GPMC_AD10, (M0 | PIN_INPUT)},	/* gpmc_ad10.gpmc_ad10 */
	{GPMC_AD11, (M0 | PIN_INPUT)},	/* gpmc_ad11.gpmc_ad11 */
	{GPMC_AD12, (M0 | PIN_INPUT)},	/* gpmc_ad12.gpmc_ad12 */
	{GPMC_AD13, (M0 | PIN_INPUT)},	/* gpmc_ad13.gpmc_ad13 */
	{GPMC_AD14, (M0 | PIN_INPUT)},	/* gpmc_ad14.gpmc_ad14 */
	{GPMC_AD15, (M0 | PIN_INPUT)},	/* gpmc_ad15.gpmc_ad15 */
	{GPMC_CS0, (M0 | PIN_INPUT_PULLUP)},	/* gpmc_cs0.gpmc_cs0 */
	{GPMC_ADVN_ALE, (M0 | PIN_INPUT_PULLDOWN)},	/* gpmc_advn_ale.gpmc_advn_ale */
	{GPMC_OEN_REN, (M0 | PIN_INPUT_PULLUP)},	/* gpmc_oen_ren.gpmc_oen_ren */
	{GPMC_WEN, (M0 | PIN_INPUT_PULLUP)},	/* gpmc_wen.gpmc_wen */
	{GPMC_BEN0, (M0 | PIN_INPUT_PULLDOWN)},	/* gpmc_ben0.gpmc_ben0 */
	{GPMC_WAIT0, (M0 | PIN_INPUT_PULLUP | SLEWCONTROL)},	/* gpmc_wait0.gpmc_wait0 */
};

const struct pad_conf_entry early_padconf[] = {
	{UART1_RXD, (PIN_INPUT_SLEW | M0)}, /* UART1_RXD */
	{UART1_TXD, (PIN_INPUT_SLEW | M0)}, /* UART1_TXD */
	{UART3_RXD, (PIN_INPUT_SLEW | M0)}, /* UART3_RXD */
	{UART3_TXD, (PIN_INPUT_SLEW | M0)}, /* UART3_TXD */
	{I2C1_SDA, (PIN_INPUT | M0)},	/* I2C1_SDA */
	{I2C1_SCL, (PIN_INPUT | M0)},	/* I2C1_SCL */
};

#ifdef CONFIG_IODELAY_RECALIBRATION
const struct iodelay_cfg_entry dra72_iodelay_cfg_array_revb[] = {
	{0x6F0, 359, 0}, /* RGMMI0_RXC_IN */
	{0x6FC, 129, 1896}, /* RGMMI0_RXCTL_IN */
	{0x708, 80, 1391}, /* RGMMI0_RXD0_IN */
	{0x714, 196, 1522}, /* RGMMI0_RXD1_IN */
	{0x720, 40, 1860}, /* RGMMI0_RXD2_IN */
	{0x72C, 0, 1956}, /* RGMMI0_RXD3_IN */
	{0x740, 0, 220}, /* RGMMI0_TXC_OUT */
	{0x74C, 1820, 180}, /* RGMMI0_TXCTL_OUT */
	{0x758, 1740, 440}, /* RGMMI0_TXD0_OUT */
	{0x764, 1740, 240}, /* RGMMI0_TXD1_OUT */
	{0x770, 1680, 380}, /* RGMMI0_TXD2_OUT */
	{0x77C, 1740, 440}, /* RGMMI0_TXD3_OUT */
	/* These values are for using RGMII1 configuration on VIN2a_x pins. */
	{0xAB0, 596, 0}, /* CFG_VIN2A_D18_IN */
	{0xABC, 314, 980}, /* CFG_VIN2A_D19_IN */
	{0xAD4, 241, 1536}, /* CFG_VIN2A_D20_IN */
	{0xAE0, 103, 1689}, /* CFG_VIN2A_D21_IN */
	{0xAEC, 161, 1563}, /* CFG_VIN2A_D22_IN */
	{0xAF8, 0, 1613}, /* CFG_VIN2A_D23_IN */
	{0xA70, 0, 200}, /* CFG_VIN2A_D12_OUT */
	{0xA7C, 1560, 140}, /* CFG_VIN2A_D13_OUT */
	{0xA88, 1700, 0}, /* CFG_VIN2A_D14_OUT */
	{0xA94, 1260, 0}, /* CFG_VIN2A_D15_OUT */
	{0xAA0, 1400, 0}, /* CFG_VIN2A_D16_OUT */
	{0xAAC, 1290, 0}, /* CFG_VIN2A_D17_OUT */
	{0x144, 0, 0}, /* CFG_GPMC_A13_IN */
	{0x150, 2062, 2277}, /* CFG_GPMC_A14_IN */
	{0x15C, 1960, 2289}, /* CFG_GPMC_A15_IN */
	{0x168, 2058, 2386}, /* CFG_GPMC_A16_IN */
	{0x170, 0, 0 },	/* CFG_GPMC_A16_OUT */
	{0x174, 2062, 2350}, /* CFG_GPMC_A17_IN */
	{0x188, 0, 0}, /* CFG_GPMC_A18_OUT */
	{0x374, 121, 0}, /* CFG_GPMC_CS2_OUT */
};

const struct iodelay_cfg_entry dra72_iodelay_cfg_array_revc[] = {
	{0x0144, 0, 0},	/* CFG_GPMC_A13_IN */
	{0x0150, 2247, 1186},	/* CFG_GPMC_A14_IN */
	{0x015C, 2176, 1197},	/* CFG_GPMC_A15_IN */
	{0x0168, 2229, 1268},	/* CFG_GPMC_A16_IN */
	{0x0170, 0, 0},	/* CFG_GPMC_A16_OUT */
	{0x0174, 2251, 1217},	/* CFG_GPMC_A17_IN */
	{0x0188, 0, 0},	/* CFG_GPMC_A18_OUT */
	{0x0374, 121, 0},	/* CFG_GPMC_CS2_OUT */
	{0x06F0, 413, 0},	/* CFG_RGMII0_RXC_IN */
	{0x06FC, 27, 2296},	/* CFG_RGMII0_RXCTL_IN */
	{0x0708, 3, 1721},	/* CFG_RGMII0_RXD0_IN */
	{0x0714, 134, 1786},	/* CFG_RGMII0_RXD1_IN */
	{0x0720, 40, 1966},	/* CFG_RGMII0_RXD2_IN */
	{0x072C, 0, 2057},	/* CFG_RGMII0_RXD3_IN */
	{0x0740, 0, 60},	/* CFG_RGMII0_TXC_OUT */
	{0x074C, 0, 60},	/* CFG_RGMII0_TXCTL_OUT */
	{0x0758, 0, 60},	/* CFG_RGMII0_TXD0_OUT */
	{0x0764, 0, 0},		/* CFG_RGMII0_TXD1_OUT */
	{0x0770, 0, 60},	/* CFG_RGMII0_TXD2_OUT */
	{0x077C, 0, 120},	/* CFG_RGMII0_TXD3_OUT */
	{0x0A70, 0, 0},		/* CFG_VIN2A_D12_OUT */
	{0x0A7C, 170, 0},	/* CFG_VIN2A_D13_OUT */
	{0x0A88, 150, 0},	/* CFG_VIN2A_D14_OUT */
	{0x0A94, 0, 0},		/* CFG_VIN2A_D15_OUT */
	{0x0AA0, 60, 0},	/* CFG_VIN2A_D16_OUT */
	{0x0AAC, 60, 0},	/* CFG_VIN2A_D17_OUT */
	{0x0AB0, 530, 0},	/* CFG_VIN2A_D18_IN */
	{0x0ABC, 71, 1099},	/* CFG_VIN2A_D19_IN */
	{0x0AC8, 2229, 10},	/* CFG_VIN2A_D1_IN */
	{0x0AD4, 142, 1337},	/* CFG_VIN2A_D20_IN */
	{0x0AE0, 114, 1517},	/* CFG_VIN2A_D21_IN */
	{0x0AEC, 171, 1331},	/* CFG_VIN2A_D22_IN */
	{0x0AF8, 0, 1328},	/* CFG_VIN2A_D23_IN */
};

const struct iodelay_cfg_entry dra71_iodelay_cfg_array[] = {
	{0x0144, 0, 0},	/* CFG_GPMC_A13_IN */
	{0x0150, 2247, 1186},	/* CFG_GPMC_A14_IN */
	{0x015C, 2176, 1197},	/* CFG_GPMC_A15_IN */
	{0x0168, 2229, 1268},	/* CFG_GPMC_A16_IN */
	{0x0170, 0, 0},	/* CFG_GPMC_A16_OUT */
	{0x0174, 2251, 1217},	/* CFG_GPMC_A17_IN */
	{0x0188, 0, 0},	/* CFG_GPMC_A18_OUT */
	{0x0374, 0, 0},	/* CFG_GPMC_CS2_OUT */
	{0x06F0, 413, 0},	/* CFG_RGMII0_RXC_IN */
	{0x06FC, 27, 2296},	/* CFG_RGMII0_RXCTL_IN */
	{0x0708, 3, 1721},	/* CFG_RGMII0_RXD0_IN */
	{0x0714, 134, 1786},	/* CFG_RGMII0_RXD1_IN */
	{0x0720, 40, 1966},	/* CFG_RGMII0_RXD2_IN */
	{0x072C, 0, 2057},	/* CFG_RGMII0_RXD3_IN */
	{0x0740, 0, 60},	/* CFG_RGMII0_TXC_OUT */
	{0x074C, 0, 60},	/* CFG_RGMII0_TXCTL_OUT */
	{0x0758, 0, 60},	/* CFG_RGMII0_TXD0_OUT */
	{0x0764, 0, 0},	/* CFG_RGMII0_TXD1_OUT */
	{0x0770, 0, 60},	/* CFG_RGMII0_TXD2_OUT */
	{0x077C, 0, 120},	/* CFG_RGMII0_TXD3_OUT */
	{0x0A38, 0, 0},	/* CFG_VIN2A_CLK0_IN */
	{0x0A44, 1936, 0},	/* CFG_VIN2A_D0_IN */
	{0x0A50, 2031, 0},	/* CFG_VIN2A_D10_IN */
	{0x0A5C, 1702, 0},	/* CFG_VIN2A_D11_IN */
	{0x0A70, 0, 0},	/* CFG_VIN2A_D12_OUT */
	{0x0A7C, 170, 0},	/* CFG_VIN2A_D13_OUT */
	{0x0A88, 150, 0},	/* CFG_VIN2A_D14_OUT */
	{0x0A94, 0, 0},	/* CFG_VIN2A_D15_OUT */
	{0x0AA0, 60, 0},	/* CFG_VIN2A_D16_OUT */
	{0x0AAC, 60, 0},	/* CFG_VIN2A_D17_OUT */
	{0x0AB0, 530, 0},	/* CFG_VIN2A_D18_IN */
	{0x0ABC, 71, 1099},	/* CFG_VIN2A_D19_IN */
	{0x0AC8, 2229, 10},	/* CFG_VIN2A_D1_IN */
	{0x0AD4, 142, 1337},	/* CFG_VIN2A_D20_IN */
	{0x0AE0, 114, 1517},	/* CFG_VIN2A_D21_IN */
	{0x0AEC, 171, 1331},	/* CFG_VIN2A_D22_IN */
	{0x0AF8, 0, 1328},	/* CFG_VIN2A_D23_IN */
	{0x0B04, 1736, 0},	/* CFG_VIN2A_D2_IN */
	{0x0B10, 1943, 0},	/* CFG_VIN2A_D3_IN */
	{0x0B1C, 1601, 0},	/* CFG_VIN2A_D4_IN */
	{0x0B28, 2052, 0},	/* CFG_VIN2A_D5_IN */
	{0x0B34, 1571, 0},	/* CFG_VIN2A_D6_IN */
	{0x0B40, 1855, 0},	/* CFG_VIN2A_D7_IN */
	{0x0B4C, 1224, 618},	/* CFG_VIN2A_D8_IN */
	{0x0B58, 1373, 509},	/* CFG_VIN2A_D9_IN */
	{0x0B7C, 1943, 0},	/* CFG_VIN2A_HSYNC0_IN */
	{0x0B88, 1612, 0},	/* CFG_VIN2A_VSYNC0_IN */
};
#endif

const struct pad_conf_entry dra74x_core_padconf_array[] = {
	{GPMC_AD0, (M3 | PIN_INPUT)},	/* gpmc_ad0.vout3_d0 */
	{GPMC_AD1, (M3 | PIN_INPUT)},	/* gpmc_ad1.vout3_d1 */
	{GPMC_AD2, (M3 | PIN_INPUT)},	/* gpmc_ad2.vout3_d2 */
	{GPMC_AD3, (M3 | PIN_INPUT)},	/* gpmc_ad3.vout3_d3 */
	{GPMC_AD4, (M3 | PIN_INPUT)},	/* gpmc_ad4.vout3_d4 */
	{GPMC_AD5, (M3 | PIN_INPUT)},	/* gpmc_ad5.vout3_d5 */
	{GPMC_AD6, (M3 | PIN_INPUT)},	/* gpmc_ad6.vout3_d6 */
	{GPMC_AD7, (M3 | PIN_INPUT)},	/* gpmc_ad7.vout3_d7 */
	{GPMC_AD8, (M3 | PIN_INPUT)},	/* gpmc_ad8.vout3_d8 */
	{GPMC_AD9, (M3 | PIN_INPUT)},	/* gpmc_ad9.vout3_d9 */
	{GPMC_AD10, (M3 | PIN_INPUT)},	/* gpmc_ad10.vout3_d10 */
	{GPMC_AD11, (M3 | PIN_INPUT)},	/* gpmc_ad11.vout3_d11 */
	{GPMC_AD12, (M3 | PIN_INPUT)},	/* gpmc_ad12.vout3_d12 */
	{GPMC_AD13, (M3 | PIN_INPUT)},	/* gpmc_ad13.vout3_d13 */
	{GPMC_AD14, (M3 | PIN_INPUT)},	/* gpmc_ad14.vout3_d14 */
	{GPMC_AD15, (M3 | PIN_INPUT)},	/* gpmc_ad15.vout3_d15 */
	{GPMC_A0, (M3 | PIN_INPUT_PULLDOWN)},	/* gpmc_a0.vout3_d16 */
	{GPMC_A1, (M3 | PIN_INPUT_PULLDOWN)},	/* gpmc_a1.vout3_d17 */
	{GPMC_A2, (M3 | PIN_INPUT_PULLDOWN)},	/* gpmc_a2.vout3_d18 */
	{GPMC_A3, (M3 | PIN_INPUT_PULLDOWN)},	/* gpmc_a3.vout3_d19 */
	{GPMC_A4, (M3 | PIN_INPUT_PULLDOWN)},	/* gpmc_a4.vout3_d20 */
	{GPMC_A5, (M3 | PIN_INPUT_PULLDOWN)},	/* gpmc_a5.vout3_d21 */
	{GPMC_A6, (M3 | PIN_INPUT_PULLDOWN)},	/* gpmc_a6.vout3_d22 */
	{GPMC_A7, (M3 | PIN_INPUT_PULLDOWN)},	/* gpmc_a7.vout3_d23 */
	{GPMC_A8, (M3 | PIN_INPUT_PULLDOWN)},	/* gpmc_a8.vout3_hsync */
	{GPMC_A9, (M3 | PIN_INPUT_PULLDOWN)},	/* gpmc_a9.vout3_vsync */
	{GPMC_A10, (M3 | PIN_INPUT_PULLDOWN)},	/* gpmc_a10.vout3_de */
	{GPMC_A11, (M14 | PIN_INPUT_PULLDOWN)},	/* gpmc_a11.gpio2_1 */
	{GPMC_A13, (M1 | PIN_INPUT_PULLDOWN | MANUAL_MODE)},	/* gpmc_a13.qspi1_rtclk */
	{GPMC_A14, (M1 | PIN_INPUT_PULLDOWN | MANUAL_MODE)},	/* gpmc_a14.qspi1_d3 */
	{GPMC_A15, (M1 | PIN_INPUT_PULLDOWN | MANUAL_MODE)},	/* gpmc_a15.qspi1_d2 */
	{GPMC_A16, (M1 | PIN_INPUT_PULLDOWN | MANUAL_MODE)},	/* gpmc_a16.qspi1_d0 */
	{GPMC_A17, (M1 | PIN_INPUT_PULLDOWN | MANUAL_MODE)},	/* gpmc_a17.qspi1_d1 */
	{GPMC_A18, (M1 | PIN_INPUT_PULLDOWN | MANUAL_MODE)},	/* gpmc_a18.qspi1_sclk */
	{GPMC_A19, (M1 | PIN_INPUT_PULLUP)},	/* gpmc_a19.mmc2_dat4 */
	{GPMC_A20, (M1 | PIN_INPUT_PULLUP)},	/* gpmc_a20.mmc2_dat5 */
	{GPMC_A21, (M1 | PIN_INPUT_PULLUP)},	/* gpmc_a21.mmc2_dat6 */
	{GPMC_A22, (M1 | PIN_INPUT_PULLUP)},	/* gpmc_a22.mmc2_dat7 */
	{GPMC_A23, (M1 | PIN_INPUT_PULLUP)},	/* gpmc_a23.mmc2_clk */
	{GPMC_A24, (M1 | PIN_INPUT_PULLUP)},	/* gpmc_a24.mmc2_dat0 */
	{GPMC_A25, (M1 | PIN_INPUT_PULLUP)},	/* gpmc_a25.mmc2_dat1 */
	{GPMC_A26, (M1 | PIN_INPUT_PULLUP)},	/* gpmc_a26.mmc2_dat2 */
	{GPMC_A27, (M1 | PIN_INPUT_PULLUP)},	/* gpmc_a27.mmc2_dat3 */
	{GPMC_CS1, (M1 | PIN_INPUT_PULLUP)},	/* gpmc_cs1.mmc2_cmd */
	{GPMC_CS2, (M1 | PIN_INPUT_PULLUP | MANUAL_MODE)},	/* gpmc_cs2.qspi1_cs0 */
	{GPMC_CS3, (M3 | PIN_INPUT_PULLUP)},	/* gpmc_cs3.vout3_clk */
	{VIN1A_CLK0, (M0 | PIN_INPUT_PULLDOWN | MANUAL_MODE)},	/* vin1a_clk0.vin1a_clk0 */
	{VIN1A_DE0, (M0 | PIN_INPUT_PULLDOWN | MANUAL_MODE)},	/* vin1a_de0.vin1a_de0 */
	{VIN1A_FLD0, (M0 | PIN_INPUT_PULLDOWN | MANUAL_MODE)},	/* vin1a_fld0.vin1a_fld0 */
	{VIN1A_HSYNC0, (M0 | PIN_INPUT_PULLDOWN | MANUAL_MODE)},	/* vin1a_hsync0.vin1a_hsync0 */
	{VIN1A_VSYNC0, (M0 | PIN_INPUT_PULLDOWN | MANUAL_MODE)},	/* vin1a_vsync0.vin1a_vsync0 */
	{VIN1A_D0, (M0 | PIN_INPUT_PULLDOWN | MANUAL_MODE)},	/* vin1a_d0.vin1a_d0 */
	{VIN1A_D1, (M0 | PIN_INPUT_PULLDOWN | MANUAL_MODE)},	/* vin1a_d1.vin1a_d1 */
	{VIN1A_D2, (M0 | PIN_INPUT_PULLDOWN | MANUAL_MODE)},	/* vin1a_d2.vin1a_d2 */
	{VIN1A_D3, (M0 | PIN_INPUT_PULLDOWN | MANUAL_MODE)},	/* vin1a_d3.vin1a_d3 */
	{VIN1A_D4, (M0 | PIN_INPUT_PULLDOWN | MANUAL_MODE)},	/* vin1a_d4.vin1a_d4 */
	{VIN1A_D5, (M0 | PIN_INPUT_PULLDOWN | MANUAL_MODE)},	/* vin1a_d5.vin1a_d5 */
	{VIN1A_D6, (M0 | PIN_INPUT_PULLDOWN | MANUAL_MODE)},	/* vin1a_d6.vin1a_d6 */
	{VIN1A_D7, (M0 | PIN_INPUT_PULLDOWN | MANUAL_MODE)},	/* vin1a_d7.vin1a_d7 */
	{VIN1A_D8, (M0 | PIN_INPUT_PULLDOWN | MANUAL_MODE)},	/* vin1a_d8.vin1a_d8 */
	{VIN1A_D9, (M0 | PIN_INPUT_PULLDOWN | MANUAL_MODE)},	/* vin1a_d9.vin1a_d9 */
	{VIN1A_D10, (M0 | PIN_INPUT_PULLDOWN | MANUAL_MODE)},	/* vin1a_d10.vin1a_d10 */
	{VIN1A_D11, (M0 | PIN_INPUT_PULLDOWN | MANUAL_MODE)},	/* vin1a_d11.vin1a_d11 */
	{VIN1A_D12, (M0 | PIN_INPUT_PULLDOWN | MANUAL_MODE)},	/* vin1a_d12.vin1a_d12 */
	{VIN1A_D13, (M0 | PIN_INPUT_PULLDOWN | MANUAL_MODE)},	/* vin1a_d13.vin1a_d13 */
	{VIN1A_D14, (M0 | PIN_INPUT_PULLDOWN | MANUAL_MODE)},	/* vin1a_d14.vin1a_d14 */
	{VIN1A_D15, (M0 | PIN_INPUT_PULLDOWN | MANUAL_MODE)},	/* vin1a_d15.vin1a_d15 */
	{VIN1A_D16, (M0 | PIN_INPUT_PULLDOWN | MANUAL_MODE)},	/* vin1a_d16.vin1a_d16 */
	{VIN1A_D17, (M0 | PIN_INPUT_PULLDOWN | MANUAL_MODE)},	/* vin1a_d17.vin1a_d17 */
	{VIN1A_D18, (M0 | PIN_INPUT_PULLDOWN | MANUAL_MODE)},	/* vin1a_d18.vin1a_d18 */
	{VIN1A_D19, (M0 | PIN_INPUT_PULLDOWN | MANUAL_MODE)},	/* vin1a_d19.vin1a_d19 */
	{VIN1A_D20, (M0 | PIN_INPUT_PULLDOWN | MANUAL_MODE)},	/* vin1a_d20.vin1a_d20 */
	{VIN1A_D21, (M0 | PIN_INPUT_PULLDOWN | MANUAL_MODE)},	/* vin1a_d21.vin1a_d21 */
	{VIN1A_D22, (M0 | PIN_INPUT_PULLDOWN | MANUAL_MODE)},	/* vin1a_d22.vin1a_d22 */
	{VIN1A_D23, (M0 | PIN_INPUT_PULLDOWN | MANUAL_MODE)},	/* vin1a_d23.vin1a_d23 */
	{VIN2A_D12, (M3 | PIN_INPUT_PULLDOWN | MANUAL_MODE)},	/* vin2a_d12.rgmii1_txc */
	{VIN2A_D13, (M3 | PIN_INPUT_PULLDOWN | MANUAL_MODE)},	/* vin2a_d13.rgmii1_txctl */
	{VIN2A_D14, (M3 | PIN_INPUT_PULLDOWN | MANUAL_MODE)},	/* vin2a_d14.rgmii1_txd3 */
	{VIN2A_D15, (M3 | PIN_INPUT_PULLDOWN | MANUAL_MODE)},	/* vin2a_d15.rgmii1_txd2 */
	{VIN2A_D16, (M3 | PIN_INPUT_PULLDOWN | MANUAL_MODE)},	/* vin2a_d16.rgmii1_txd1 */
	{VIN2A_D17, (M3 | PIN_INPUT_PULLDOWN | MANUAL_MODE)},	/* vin2a_d17.rgmii1_txd0 */
	{VIN2A_D18, (M3 | PIN_INPUT_PULLDOWN | MANUAL_MODE)},	/* vin2a_d18.rgmii1_rxc */
	{VIN2A_D19, (M3 | PIN_INPUT_PULLDOWN | MANUAL_MODE)},	/* vin2a_d19.rgmii1_rxctl */
	{VIN2A_D20, (M3 | PIN_INPUT_PULLDOWN | MANUAL_MODE)},	/* vin2a_d20.rgmii1_rxd3 */
	{VIN2A_D21, (M3 | PIN_INPUT_PULLDOWN | MANUAL_MODE)},	/* vin2a_d21.rgmii1_rxd2 */
	{VIN2A_D22, (M3 | PIN_INPUT_PULLDOWN | MANUAL_MODE)},	/* vin2a_d22.rgmii1_rxd1 */
	{VIN2A_D23, (M3 | PIN_INPUT_PULLDOWN | MANUAL_MODE)},	/* vin2a_d23.rgmii1_rxd0 */
	{VOUT1_CLK, (M0 | PIN_INPUT_PULLDOWN)},	/* vout1_clk.vout1_clk */
	{VOUT1_DE, (M0 | PIN_INPUT_PULLDOWN)},	/* vout1_de.vout1_de */
	{VOUT1_HSYNC, (M0 | PIN_INPUT_PULLDOWN)},	/* vout1_hsync.vout1_hsync */
	{VOUT1_VSYNC, (M0 | PIN_INPUT_PULLDOWN)},	/* vout1_vsync.vout1_vsync */
	{VOUT1_D0, (M0 | PIN_INPUT_PULLDOWN)},	/* vout1_d0.vout1_d0 */
	{VOUT1_D1, (M0 | PIN_INPUT_PULLDOWN)},	/* vout1_d1.vout1_d1 */
	{VOUT1_D2, (M0 | PIN_INPUT_PULLDOWN)},	/* vout1_d2.vout1_d2 */
	{VOUT1_D3, (M0 | PIN_INPUT_PULLDOWN)},	/* vout1_d3.vout1_d3 */
	{VOUT1_D4, (M0 | PIN_INPUT_PULLDOWN)},	/* vout1_d4.vout1_d4 */
	{VOUT1_D5, (M0 | PIN_INPUT_PULLDOWN)},	/* vout1_d5.vout1_d5 */
	{VOUT1_D6, (M0 | PIN_INPUT_PULLDOWN)},	/* vout1_d6.vout1_d6 */
	{VOUT1_D7, (M0 | PIN_INPUT_PULLDOWN)},	/* vout1_d7.vout1_d7 */
	{VOUT1_D8, (M0 | PIN_INPUT_PULLDOWN)},	/* vout1_d8.vout1_d8 */
	{VOUT1_D9, (M0 | PIN_INPUT_PULLDOWN)},	/* vout1_d9.vout1_d9 */
	{VOUT1_D10, (M0 | PIN_INPUT_PULLDOWN)},	/* vout1_d10.vout1_d10 */
	{VOUT1_D11, (M0 | PIN_INPUT_PULLDOWN)},	/* vout1_d11.vout1_d11 */
	{VOUT1_D12, (M0 | PIN_INPUT_PULLDOWN)},	/* vout1_d12.vout1_d12 */
	{VOUT1_D13, (M0 | PIN_INPUT_PULLDOWN)},	/* vout1_d13.vout1_d13 */
	{VOUT1_D14, (M0 | PIN_INPUT_PULLDOWN)},	/* vout1_d14.vout1_d14 */
	{VOUT1_D15, (M0 | PIN_INPUT_PULLDOWN)},	/* vout1_d15.vout1_d15 */
	{VOUT1_D16, (M0 | PIN_INPUT_PULLDOWN)},	/* vout1_d16.vout1_d16 */
	{VOUT1_D17, (M0 | PIN_INPUT_PULLDOWN)},	/* vout1_d17.vout1_d17 */
	{VOUT1_D18, (M0 | PIN_INPUT_PULLDOWN)},	/* vout1_d18.vout1_d18 */
	{VOUT1_D19, (M0 | PIN_INPUT_PULLDOWN)},	/* vout1_d19.vout1_d19 */
	{VOUT1_D20, (M0 | PIN_INPUT_PULLDOWN)},	/* vout1_d20.vout1_d20 */
	{VOUT1_D21, (M0 | PIN_INPUT_PULLDOWN)},	/* vout1_d21.vout1_d21 */
	{VOUT1_D22, (M0 | PIN_INPUT_PULLDOWN)},	/* vout1_d22.vout1_d22 */
	{VOUT1_D23, (M0 | PIN_INPUT_PULLDOWN)},	/* vout1_d23.vout1_d23 */
	{MDIO_MCLK, (M0 | PIN_INPUT_PULLUP | SLEWCONTROL)},	/* mdio_mclk.mdio_mclk */
	{MDIO_D, (M0 | PIN_INPUT_PULLUP | SLEWCONTROL)},	/* mdio_d.mdio_d */
	{RGMII0_TXC, (M0 | PIN_INPUT_PULLDOWN | MANUAL_MODE)},	/* rgmii0_txc.rgmii0_txc */
	{RGMII0_TXCTL, (M0 | PIN_INPUT_PULLDOWN | MANUAL_MODE)},	/* rgmii0_txctl.rgmii0_txctl */
	{RGMII0_TXD3, (M0 | PIN_INPUT_PULLDOWN | MANUAL_MODE)},	/* rgmii0_txd3.rgmii0_txd3 */
	{RGMII0_TXD2, (M0 | PIN_INPUT_PULLDOWN | MANUAL_MODE)},	/* rgmii0_txd2.rgmii0_txd2 */
	{RGMII0_TXD1, (M0 | PIN_INPUT_PULLDOWN | MANUAL_MODE)},	/* rgmii0_txd1.rgmii0_txd1 */
	{RGMII0_TXD0, (M0 | PIN_INPUT_PULLDOWN | MANUAL_MODE)},	/* rgmii0_txd0.rgmii0_txd0 */
	{RGMII0_RXC, (M0 | PIN_INPUT_PULLDOWN | MANUAL_MODE)},	/* rgmii0_rxc.rgmii0_rxc */
	{RGMII0_RXCTL, (M0 | PIN_INPUT_PULLDOWN | MANUAL_MODE)},	/* rgmii0_rxctl.rgmii0_rxctl */
	{RGMII0_RXD3, (M0 | PIN_INPUT_PULLDOWN | MANUAL_MODE)},	/* rgmii0_rxd3.rgmii0_rxd3 */
	{RGMII0_RXD2, (M0 | PIN_INPUT_PULLDOWN | MANUAL_MODE)},	/* rgmii0_rxd2.rgmii0_rxd2 */
	{RGMII0_RXD1, (M0 | PIN_INPUT_PULLDOWN | MANUAL_MODE)},	/* rgmii0_rxd1.rgmii0_rxd1 */
	{RGMII0_RXD0, (M0 | PIN_INPUT_PULLDOWN | MANUAL_MODE)},	/* rgmii0_rxd0.rgmii0_rxd0 */
	{USB1_DRVVBUS, (M0 | PIN_INPUT_SLEW)},	/* usb1_drvvbus.usb1_drvvbus */
	{USB2_DRVVBUS, (M0 | PIN_INPUT_SLEW)},	/* usb2_drvvbus.usb2_drvvbus */
	{GPIO6_14, (M9 | PIN_INPUT_PULLUP)},	/* gpio6_14.i2c3_sda */
	{GPIO6_15, (M9 | PIN_INPUT_PULLUP)},	/* gpio6_15.i2c3_scl */
	{GPIO6_16, (M14 | PIN_INPUT_PULLUP)},	/* gpio6_16.gpio6_16 */
	{XREF_CLK2, (M5 | PIN_INPUT_PULLDOWN)},	/* xref_clk2.atl_clk2 */
	{MCASP1_ACLKX, (M0 | PIN_INPUT_PULLDOWN)},	/* mcasp1_aclkx.mcasp1_aclkx */
	{MCASP1_FSX, (M0 | PIN_INPUT_SLEW)},	/* mcasp1_fsx.mcasp1_fsx */
	{MCASP1_AXR0, (M0 | PIN_INPUT_SLEW | VIRTUAL_MODE15)},	/* mcasp1_axr0.mcasp1_axr0 */
	{MCASP1_AXR1, (M0 | PIN_INPUT_SLEW)},	/* mcasp1_axr1.mcasp1_axr1 */
	{MCASP1_AXR2, (M14 | PIN_INPUT_PULLDOWN)},	/* mcasp1_axr2.gpio5_4 */
	{MCASP1_AXR3, (M14 | PIN_INPUT_PULLDOWN)},	/* mcasp1_axr3.gpio5_5 */
	{MCASP1_AXR4, (M14 | PIN_INPUT_PULLDOWN)},	/* mcasp1_axr4.gpio5_6 */
	{MCASP1_AXR5, (M14 | PIN_INPUT_PULLDOWN)},	/* mcasp1_axr5.gpio5_7 */
	{MCASP1_AXR6, (M14 | PIN_INPUT_PULLDOWN)},	/* mcasp1_axr6.gpio5_8 */
	{MCASP1_AXR7, (M14 | PIN_INPUT_PULLDOWN)},	/* mcasp1_axr7.gpio5_9 */
	{MCASP1_AXR12, (M1 | PIN_INPUT_SLEW | VIRTUAL_MODE10)},	/* mcasp1_axr12.mcasp7_axr0 */
	{MCASP1_AXR13, (M1 | PIN_INPUT_SLEW)},	/* mcasp1_axr13.mcasp7_axr1 */
	{MCASP1_AXR14, (M1 | PIN_INPUT_SLEW | VIRTUAL_MODE10)},	/* mcasp1_axr14.mcasp7_aclkx */
	{MCASP1_AXR15, (M1 | PIN_INPUT_SLEW | VIRTUAL_MODE10)},	/* mcasp1_axr15.mcasp7_fsx */
	{MCASP2_ACLKR, (M0 | PIN_INPUT_PULLDOWN)},	/* mcasp2_aclkr.mcasp2_aclkr */
	{MCASP3_ACLKX, (M0 | PIN_INPUT_PULLDOWN)},	/* mcasp3_aclkx.mcasp3_aclkx */
	{MCASP3_FSX, (M0 | PIN_INPUT_SLEW)},	/* mcasp3_fsx.mcasp3_fsx */
	{MCASP3_AXR0, (M0 | PIN_INPUT_SLEW)},	/* mcasp3_axr0.mcasp3_axr0 */
	{MCASP3_AXR1, (M0 | PIN_INPUT_SLEW | VIRTUAL_MODE6)},	/* mcasp3_axr1.mcasp3_axr1 */
	{MMC1_CLK, (M0 | PIN_INPUT_PULLUP)},	/* mmc1_clk.mmc1_clk */
	{MMC1_CMD, (M0 | PIN_INPUT_PULLUP)},	/* mmc1_cmd.mmc1_cmd */
	{MMC1_DAT0, (M0 | PIN_INPUT_PULLUP)},	/* mmc1_dat0.mmc1_dat0 */
	{MMC1_DAT1, (M0 | PIN_INPUT_PULLUP)},	/* mmc1_dat1.mmc1_dat1 */
	{MMC1_DAT2, (M0 | PIN_INPUT_PULLUP)},	/* mmc1_dat2.mmc1_dat2 */
	{MMC1_DAT3, (M0 | PIN_INPUT_PULLUP)},	/* mmc1_dat3.mmc1_dat3 */
	{MMC1_SDCD, (M14 | PIN_INPUT_PULLUP)},	/* mmc1_sdcd.gpio6_27 */
	{MMC1_SDWP, (M14 | PIN_INPUT_SLEW)},	/* mmc1_sdwp.gpio6_28 */
	{GPIO6_11, (M14 | PIN_INPUT_PULLUP)},	/* gpio6_11.gpio6_11 */
	{SPI1_SCLK, (M0 | PIN_INPUT_PULLDOWN)},	/* spi1_sclk.spi1_sclk */
	{SPI1_D1, (M0 | PIN_INPUT_PULLDOWN)},	/* spi1_d1.spi1_d1 */
	{SPI1_D0, (M0 | PIN_INPUT_PULLDOWN)},	/* spi1_d0.spi1_d0 */
	{SPI1_CS0, (M0 | PIN_INPUT_PULLUP)},	/* spi1_cs0.spi1_cs0 */
	{SPI1_CS1, (M14 | PIN_OUTPUT)},		/* spi1_cs1.gpio7_11 */
	{SPI1_CS2, (M14 | PIN_INPUT_PULLDOWN)},	/* spi1_cs2.gpio7_12 */
	{SPI1_CS3, (M6 | PIN_INPUT | SLEWCONTROL)},	/* spi1_cs3.hdmi1_cec */
	{SPI2_SCLK, (M1 | PIN_INPUT_PULLDOWN)},	/* spi2_sclk.uart3_rxd */
	{SPI2_D1, (M1 | PIN_INPUT_SLEW)},	/* spi2_d1.uart3_txd */
	{SPI2_D0, (M1 | PIN_INPUT_SLEW)},	/* spi2_d0.uart3_ctsn */
	{SPI2_CS0, (M1 | PIN_INPUT_PULLUP | SLEWCONTROL)},	/* spi2_cs0.uart3_rtsn */
	{DCAN1_TX, (M15 | PULL_UP)},	/* dcan1_tx.safe for dcan1_tx */
	{DCAN1_RX, (M14 | PIN_INPUT_PULLUP | SLEWCONTROL)},	/* dcan1_rx.gpio1_15 */
	{UART1_RXD, (M0 | PIN_INPUT_PULLUP | SLEWCONTROL)},	/* uart1_rxd.uart1_rxd */
	{UART1_TXD, (M0 | PIN_INPUT_PULLUP | SLEWCONTROL)},	/* uart1_txd.uart1_txd */
	{UART1_CTSN, (M3 | PIN_INPUT_PULLUP)},	/* uart1_ctsn.mmc4_clk */
	{UART1_RTSN, (M3 | PIN_INPUT_PULLUP)},	/* uart1_rtsn.mmc4_cmd */
	{UART2_RXD, (M3 | PIN_INPUT_PULLUP)},	/* N/A.mmc4_dat0 */
	{UART2_TXD, (M3 | PIN_INPUT_PULLUP)},	/* uart2_txd.mmc4_dat1 */
	{UART2_CTSN, (M3 | PIN_INPUT_PULLUP)},	/* uart2_ctsn.mmc4_dat2 */
	{UART2_RTSN, (M3 | PIN_INPUT_PULLUP)},	/* uart2_rtsn.mmc4_dat3 */
	{I2C2_SDA, (M0 | PIN_INPUT_PULLUP)},	/* i2c2_sda.i2c2_sda */
	{I2C2_SCL, (M0 | PIN_INPUT_PULLUP)},	/* i2c2_scl.i2c2_scl */
	{WAKEUP0, (M15 | PULL_UP)},	/* Wakeup0.safe for dcan1_rx */
	{WAKEUP2, (M14)},		/* Wakeup2.gpio1_2 */
};

const struct pad_conf_entry dra76x_core_padconf_array[] = {
	{GPMC_AD0, (M3 | PIN_INPUT | MANUAL_MODE)},	/* gpmc_ad0.vout3_d0 */
	{GPMC_AD1, (M3 | PIN_INPUT | MANUAL_MODE)},	/* gpmc_ad1.vout3_d1 */
	{GPMC_AD2, (M3 | PIN_INPUT | MANUAL_MODE)},	/* gpmc_ad2.vout3_d2 */
	{GPMC_AD3, (M3 | PIN_INPUT | MANUAL_MODE)},	/* gpmc_ad3.vout3_d3 */
	{GPMC_AD4, (M3 | PIN_INPUT | MANUAL_MODE)},	/* gpmc_ad4.vout3_d4 */
	{GPMC_AD5, (M3 | PIN_INPUT | MANUAL_MODE)},	/* gpmc_ad5.vout3_d5 */
	{GPMC_AD6, (M3 | PIN_INPUT | MANUAL_MODE)},	/* gpmc_ad6.vout3_d6 */
	{GPMC_AD7, (M3 | PIN_INPUT | MANUAL_MODE)},	/* gpmc_ad7.vout3_d7 */
	{GPMC_AD8, (M3 | PIN_INPUT | MANUAL_MODE)},	/* gpmc_ad8.vout3_d8 */
	{GPMC_AD9, (M3 | PIN_INPUT | MANUAL_MODE)},	/* gpmc_ad9.vout3_d9 */
	{GPMC_AD10, (M3 | PIN_INPUT | MANUAL_MODE)},	/* gpmc_ad10.vout3_d10 */
	{GPMC_AD11, (M3 | PIN_INPUT | MANUAL_MODE)},	/* gpmc_ad11.vout3_d11 */
	{GPMC_AD12, (M3 | PIN_INPUT | MANUAL_MODE)},	/* gpmc_ad12.vout3_d12 */
	{GPMC_AD13, (M3 | PIN_INPUT | MANUAL_MODE)},	/* gpmc_ad13.vout3_d13 */
	{GPMC_AD14, (M3 | PIN_INPUT | MANUAL_MODE)},	/* gpmc_ad14.vout3_d14 */
	{GPMC_AD15, (M3 | PIN_INPUT | MANUAL_MODE)},	/* gpmc_ad15.vout3_d15 */
	{GPMC_A0, (M3 | PIN_INPUT_PULLDOWN | MANUAL_MODE)},	/* gpmc_a0.vout3_d16 */
	{GPMC_A1, (M3 | PIN_INPUT_PULLDOWN | MANUAL_MODE)},	/* gpmc_a1.vout3_d17 */
	{GPMC_A2, (M3 | PIN_INPUT_PULLDOWN | MANUAL_MODE)},	/* gpmc_a2.vout3_d18 */
	{GPMC_A3, (M3 | PIN_INPUT_PULLDOWN | MANUAL_MODE)},	/* gpmc_a3.vout3_d19 */
	{GPMC_A4, (M3 | PIN_INPUT_PULLDOWN | MANUAL_MODE)},	/* gpmc_a4.vout3_d20 */
	{GPMC_A5, (M3 | PIN_INPUT_PULLDOWN | MANUAL_MODE)},	/* gpmc_a5.vout3_d21 */
	{GPMC_A6, (M3 | PIN_INPUT_PULLDOWN | MANUAL_MODE)},	/* gpmc_a6.vout3_d22 */
	{GPMC_A7, (M3 | PIN_INPUT_PULLDOWN | MANUAL_MODE)},	/* gpmc_a7.vout3_d23 */
	{GPMC_A8, (M3 | PIN_INPUT_PULLDOWN | MANUAL_MODE)},	/* gpmc_a8.vout3_hsync */
	{GPMC_A9, (M3 | PIN_INPUT_PULLDOWN | MANUAL_MODE)},	/* gpmc_a9.vout3_vsync */
	{GPMC_A10, (M3 | PIN_INPUT_PULLDOWN | MANUAL_MODE)},	/* gpmc_a10.vout3_de */
	{GPMC_A11, (M14 | PIN_INPUT_PULLUP)},	/* gpmc_a11.gpio2_1 */
	{GPMC_A12, (M14 | PIN_INPUT_PULLUP)},	/* gpmc_a12.gpio2_2 */
	{GPMC_A13, (M1 | PIN_INPUT_PULLDOWN | MANUAL_MODE)},	/* gpmc_a13.qspi1_rtclk */
	{GPMC_A14, (M1 | PIN_INPUT_PULLDOWN | MANUAL_MODE)},	/* gpmc_a14.qspi1_d3 */
	{GPMC_A15, (M1 | PIN_INPUT_PULLDOWN | MANUAL_MODE)},	/* gpmc_a15.qspi1_d2 */
	{GPMC_A16, (M1 | PIN_INPUT_PULLDOWN | MANUAL_MODE)},	/* gpmc_a16.qspi1_d0 */
	{GPMC_A17, (M1 | PIN_INPUT_PULLDOWN | MANUAL_MODE)},	/* gpmc_a17.qspi1_d1 */
	{GPMC_A18, (M1 | PIN_INPUT_PULLDOWN | MANUAL_MODE)},	/* gpmc_a18.qspi1_sclk */
	{GPMC_A19, (M1 | PIN_INPUT_PULLDOWN)},	/* gpmc_a19.mmc2_dat4 */
	{GPMC_A20, (M1 | PIN_INPUT_PULLDOWN)},	/* gpmc_a20.mmc2_dat5 */
	{GPMC_A21, (M1 | PIN_INPUT_PULLDOWN)},	/* gpmc_a21.mmc2_dat6 */
	{GPMC_A22, (M1 | PIN_INPUT_PULLDOWN)},	/* gpmc_a22.mmc2_dat7 */
	{GPMC_A23, (M1 | PIN_INPUT_PULLDOWN)},	/* gpmc_a23.mmc2_clk */
	{GPMC_A24, (M1 | PIN_INPUT_PULLDOWN)},	/* gpmc_a24.mmc2_dat0 */
	{GPMC_A25, (M1 | PIN_INPUT_PULLDOWN)},	/* gpmc_a25.mmc2_dat1 */
	{GPMC_A26, (M1 | PIN_INPUT_PULLDOWN)},	/* gpmc_a26.mmc2_dat2 */
	{GPMC_A27, (M1 | PIN_INPUT_PULLDOWN)},	/* gpmc_a27.mmc2_dat3 */
	{GPMC_CS1, (M1 | PIN_INPUT_PULLUP)},	/* gpmc_cs1.mmc2_cmd */
	{GPMC_CS0, (M0 | PIN_INPUT_PULLUP)},	/* gpmc_cs0.gpmc_cs0 */
	{GPMC_CS2, (M1 | PIN_INPUT_PULLUP | MANUAL_MODE)},	/* gpmc_cs2.qspi1_cs0 */
	{GPMC_CS3, (M3 | PIN_INPUT_PULLUP | MANUAL_MODE)},	/* gpmc_cs3.vout3_clk */
	{GPMC_ADVN_ALE, (M0 | PIN_INPUT_PULLUP)},	/* gpmc_advn_ale.gpmc_advn_ale */
	{GPMC_OEN_REN, (M0 | PIN_INPUT_PULLUP)},	/* gpmc_oen_ren.gpmc_oen_ren */
	{GPMC_WEN, (M0 | PIN_INPUT_PULLUP)},	/* gpmc_wen.gpmc_wen */
	{GPMC_BEN0, (M0 | PIN_INPUT_PULLUP)},	/* gpmc_ben0.gpmc_ben0 */
	{GPMC_WAIT0, (M0 | PIN_INPUT_PULLUP | SLEWCONTROL)},	/* gpmc_wait0.gpmc_wait0 */
	{VIN1A_FLD0, (M14 | PIN_INPUT_PULLUP)},	/* vin1a_fld0.gpio3_1 */
	{VIN2A_CLK0, (M0 | PIN_INPUT_PULLDOWN | MANUAL_MODE)},	/* vin2a_clk0.vin2a_clk0 */
	{VIN2A_DE0, (M15 | PIN_INPUT_PULLDOWN | MANUAL_MODE)},	/* vin2a_de0.Driveroff */
	{VIN2A_FLD0, (M14 | PIN_INPUT_PULLDOWN)},	/* vin2a_fld0.gpio3_30 */
	{VIN2A_HSYNC0, (M0 | PIN_INPUT_PULLDOWN | MANUAL_MODE)},	/* vin2a_hsync0.vin2a_hsync0 */
	{VIN2A_VSYNC0, (M0 | PIN_INPUT_PULLDOWN | MANUAL_MODE)},	/* vin2a_vsync0.vin2a_vsync0 */
	{VIN2A_D0, (M0 | PIN_INPUT_PULLDOWN | MANUAL_MODE)},	/* vin2a_d0.vin2a_d0 */
	{VIN2A_D1, (M0 | PIN_INPUT_PULLDOWN | MANUAL_MODE)},	/* vin2a_d1.vin2a_d1 */
	{VIN2A_D2, (M0 | PIN_INPUT_PULLDOWN | MANUAL_MODE)},	/* vin2a_d2.vin2a_d2 */
	{VIN2A_D3, (M0 | PIN_INPUT_PULLDOWN | MANUAL_MODE)},	/* vin2a_d3.vin2a_d3 */
	{VIN2A_D4, (M0 | PIN_INPUT_PULLDOWN | MANUAL_MODE)},	/* vin2a_d4.vin2a_d4 */
	{VIN2A_D5, (M0 | PIN_INPUT_PULLDOWN | MANUAL_MODE)},	/* vin2a_d5.vin2a_d5 */
	{VIN2A_D6, (M0 | PIN_INPUT_PULLDOWN | MANUAL_MODE)},	/* vin2a_d6.vin2a_d6 */
	{VIN2A_D7, (M0 | PIN_INPUT_PULLDOWN | MANUAL_MODE)},	/* vin2a_d7.vin2a_d7 */
	{VIN2A_D8, (M0 | PIN_INPUT_PULLDOWN | MANUAL_MODE)},	/* vin2a_d8.vin2a_d8 */
	{VIN2A_D9, (M0 | PIN_INPUT_PULLDOWN | MANUAL_MODE)},	/* vin2a_d9.vin2a_d9 */
	{VIN2A_D10, (M0 | PIN_INPUT_PULLDOWN | MANUAL_MODE)},	/* vin2a_d10.vin2a_d10 */
	{VIN2A_D11, (M0 | PIN_INPUT_PULLDOWN | MANUAL_MODE)},	/* vin2a_d11.vin2a_d11 */
	{VIN2A_D12, (M3 | PIN_INPUT | MANUAL_MODE)},	/* vin2a_d12.rgmii1_txc */
	{VIN2A_D13, (M3 | PIN_INPUT | MANUAL_MODE)},	/* vin2a_d13.rgmii1_txctl */
	{VIN2A_D14, (M3 | PIN_INPUT | MANUAL_MODE)},	/* vin2a_d14.rgmii1_txd3 */
	{VIN2A_D15, (M3 | PIN_INPUT | MANUAL_MODE)},	/* vin2a_d15.rgmii1_txd2 */
	{VIN2A_D16, (M3 | PIN_INPUT | MANUAL_MODE)},	/* vin2a_d16.rgmii1_txd1 */
	{VIN2A_D17, (M3 | PIN_INPUT | MANUAL_MODE)},	/* vin2a_d17.rgmii1_txd0 */
	{VIN2A_D18, (M3 | PIN_INPUT | MANUAL_MODE)},	/* vin2a_d18.rgmii1_rxc */
	{VIN2A_D19, (M3 | PIN_INPUT | MANUAL_MODE)},	/* vin2a_d19.rgmii1_rxctl */
	{VIN2A_D20, (M3 | PIN_INPUT | MANUAL_MODE)},	/* vin2a_d20.rgmii1_rxd3 */
	{VIN2A_D21, (M3 | PIN_INPUT | MANUAL_MODE)},	/* vin2a_d21.rgmii1_rxd2 */
	{VIN2A_D22, (M3 | PIN_INPUT | MANUAL_MODE)},	/* vin2a_d22.rgmii1_rxd1 */
	{VIN2A_D23, (M3 | PIN_INPUT | MANUAL_MODE)},	/* vin2a_d23.rgmii1_rxd0 */
	{VOUT1_CLK, (M0 | PIN_INPUT_PULLDOWN | MANUAL_MODE)},	/* vout1_clk.vout1_clk */
	{VOUT1_DE, (M0 | PIN_INPUT_PULLDOWN | MANUAL_MODE)},	/* vout1_de.vout1_de */
	{VOUT1_FLD, (M14 | PIN_INPUT_PULLUP)},	/* vout1_fld.gpio4_21 */
	{VOUT1_HSYNC, (M0 | PIN_INPUT_PULLDOWN | MANUAL_MODE)},	/* vout1_hsync.vout1_hsync */
	{VOUT1_VSYNC, (M0 | PIN_INPUT_PULLDOWN | MANUAL_MODE)},	/* vout1_vsync.vout1_vsync */
	{VOUT1_D0, (M0 | PIN_INPUT_PULLDOWN | MANUAL_MODE)},	/* vout1_d0.vout1_d0 */
	{VOUT1_D1, (M0 | PIN_INPUT_PULLDOWN | MANUAL_MODE)},	/* vout1_d1.vout1_d1 */
	{VOUT1_D2, (M0 | PIN_INPUT_PULLDOWN | MANUAL_MODE)},	/* vout1_d2.vout1_d2 */
	{VOUT1_D3, (M0 | PIN_INPUT_PULLDOWN | MANUAL_MODE)},	/* vout1_d3.vout1_d3 */
	{VOUT1_D4, (M0 | PIN_INPUT_PULLDOWN | MANUAL_MODE)},	/* vout1_d4.vout1_d4 */
	{VOUT1_D5, (M0 | PIN_INPUT_PULLDOWN | MANUAL_MODE)},	/* vout1_d5.vout1_d5 */
	{VOUT1_D6, (M0 | PIN_INPUT_PULLDOWN | MANUAL_MODE)},	/* vout1_d6.vout1_d6 */
	{VOUT1_D7, (M0 | PIN_INPUT_PULLDOWN | MANUAL_MODE)},	/* vout1_d7.vout1_d7 */
	{VOUT1_D8, (M0 | PIN_INPUT_PULLDOWN | MANUAL_MODE)},	/* vout1_d8.vout1_d8 */
	{VOUT1_D9, (M0 | PIN_INPUT_PULLDOWN | MANUAL_MODE)},	/* vout1_d9.vout1_d9 */
	{VOUT1_D10, (M0 | PIN_INPUT_PULLDOWN | MANUAL_MODE)},	/* vout1_d10.vout1_d10 */
	{VOUT1_D11, (M0 | PIN_INPUT_PULLDOWN | MANUAL_MODE)},	/* vout1_d11.vout1_d11 */
	{VOUT1_D12, (M0 | PIN_INPUT_PULLDOWN | MANUAL_MODE)},	/* vout1_d12.vout1_d12 */
	{VOUT1_D13, (M0 | PIN_INPUT_PULLDOWN | MANUAL_MODE)},	/* vout1_d13.vout1_d13 */
	{VOUT1_D14, (M0 | PIN_INPUT_PULLDOWN | MANUAL_MODE)},	/* vout1_d14.vout1_d14 */
	{VOUT1_D15, (M0 | PIN_INPUT_PULLDOWN | MANUAL_MODE)},	/* vout1_d15.vout1_d15 */
	{VOUT1_D16, (M0 | PIN_INPUT_PULLDOWN | MANUAL_MODE)},	/* vout1_d16.vout1_d16 */
	{VOUT1_D17, (M0 | PIN_INPUT_PULLDOWN | MANUAL_MODE)},	/* vout1_d17.vout1_d17 */
	{VOUT1_D18, (M0 | PIN_INPUT_PULLDOWN | MANUAL_MODE)},	/* vout1_d18.vout1_d18 */
	{VOUT1_D19, (M0 | PIN_INPUT_PULLDOWN | MANUAL_MODE)},	/* vout1_d19.vout1_d19 */
	{VOUT1_D20, (M0 | PIN_INPUT_PULLDOWN | MANUAL_MODE)},	/* vout1_d20.vout1_d20 */
	{VOUT1_D21, (M0 | PIN_INPUT_PULLDOWN | MANUAL_MODE)},	/* vout1_d21.vout1_d21 */
	{VOUT1_D22, (M0 | PIN_INPUT_PULLDOWN | MANUAL_MODE)},	/* vout1_d22.vout1_d22 */
	{VOUT1_D23, (M0 | PIN_INPUT_PULLDOWN | MANUAL_MODE)},	/* vout1_d23.vout1_d23 */
	{MDIO_MCLK, (M0 | PIN_INPUT_PULLUP | SLEWCONTROL)},	/* mdio_mclk.mdio_mclk */
	{MDIO_D, (M0 | PIN_INPUT_PULLUP | SLEWCONTROL)},	/* mdio_d.mdio_d */
	{RGMII0_TXC, (M0 | PIN_INPUT | MANUAL_MODE)},	/* rgmii0_txc.rgmii0_txc */
	{RGMII0_TXCTL, (M0 | PIN_INPUT | MANUAL_MODE)},	/* rgmii0_txctl.rgmii0_txctl */
	{RGMII0_TXD3, (M0 | PIN_INPUT | MANUAL_MODE)},	/* rgmii0_txd3.rgmii0_txd3 */
	{RGMII0_TXD2, (M0 | PIN_INPUT | MANUAL_MODE)},	/* rgmii0_txd2.rgmii0_txd2 */
	{RGMII0_TXD1, (M0 | PIN_INPUT | MANUAL_MODE)},	/* rgmii0_txd1.rgmii0_txd1 */
	{RGMII0_TXD0, (M0 | PIN_INPUT | MANUAL_MODE)},	/* rgmii0_txd0.rgmii0_txd0 */
	{RGMII0_RXC, (M0 | PIN_INPUT | MANUAL_MODE)},	/* rgmii0_rxc.rgmii0_rxc */
	{RGMII0_RXCTL, (M0 | PIN_INPUT | MANUAL_MODE)},	/* rgmii0_rxctl.rgmii0_rxctl */
	{RGMII0_RXD3, (M0 | PIN_INPUT | MANUAL_MODE)},	/* rgmii0_rxd3.rgmii0_rxd3 */
	{RGMII0_RXD2, (M0 | PIN_INPUT | MANUAL_MODE)},	/* rgmii0_rxd2.rgmii0_rxd2 */
	{RGMII0_RXD1, (M0 | PIN_INPUT | MANUAL_MODE)},	/* rgmii0_rxd1.rgmii0_rxd1 */
	{RGMII0_RXD0, (M0 | PIN_INPUT | MANUAL_MODE)},	/* rgmii0_rxd0.rgmii0_rxd0 */
	{USB1_DRVVBUS, (M0 | PIN_INPUT_SLEW)},	/* usb1_drvvbus.usb1_drvvbus */
	{USB2_DRVVBUS, (M0 | PIN_INPUT_SLEW)},	/* usb2_drvvbus.usb2_drvvbus */
	{GPIO6_14, (M9 | PIN_INPUT_PULLUP)},	/* gpio6_14.i2c3_sda */
	{GPIO6_15, (M9 | PIN_INPUT_PULLUP)},	/* gpio6_15.i2c3_scl */
	{GPIO6_16, (M0 | PIN_INPUT_PULLUP)},	/* gpio6_16.gpio6_16 */
	{XREF_CLK2, (M5 | PIN_INPUT_PULLDOWN)},	/* xref_clk2.atl_clk2 */
	{MCASP1_ACLKX, (M14 | 0x00070000)},	/* mcasp1_aclkx.gpio7_31 */
	{MCASP1_FSX, (M14 | PIN_INPUT | SLEWCONTROL)},	/* mcasp1_fsx.gpio7_30 */
	{MCASP1_AXR0, (M10 | PIN_INPUT_PULLUP | SLEWCONTROL)},	/* mcasp1_axr0.i2c5_sda */
	{MCASP1_AXR1, (M10 | 0x000f0000)},	/* mcasp1_axr1.i2c5_scl */
	{MCASP1_AXR2, (M14 | PIN_INPUT_PULLDOWN)},	/* mcasp1_axr2.gpio5_4 */
	{MCASP1_AXR3, (M14 | PIN_INPUT_PULLDOWN)},	/* mcasp1_axr3.gpio5_5 */
	{MCASP1_AXR4, (M14 | PIN_INPUT_PULLDOWN)},	/* mcasp1_axr4.gpio5_6 */
	{MCASP1_AXR5, (M14 | PIN_INPUT_PULLDOWN)},	/* mcasp1_axr5.gpio5_7 */
	{MCASP1_AXR6, (M14 | PIN_INPUT_PULLDOWN)},	/* mcasp1_axr6.gpio5_8 */
	{MCASP1_AXR7, (M14 | PIN_INPUT_PULLDOWN)},	/* mcasp1_axr7.gpio5_9 */
	{MCASP1_AXR12, (M1 | PIN_INPUT_SLEW | VIRTUAL_MODE10)},	/* mcasp1_axr12.mcasp7_axr0 */
	{MCASP1_AXR13, (M1 | PIN_INPUT_SLEW)},	/* mcasp1_axr13.mcasp7_axr1 */
	{MCASP1_AXR14, (M1 | PIN_INPUT_SLEW | VIRTUAL_MODE10)},	/* mcasp1_axr14.mcasp7_aclkx */
	{MCASP1_AXR15, (M1 | PIN_INPUT_SLEW | VIRTUAL_MODE10)},	/* mcasp1_axr15.mcasp7_fsx */
	{MCASP2_ACLKR, (M15 | PIN_INPUT_PULLUP)},	/* mcasp2_aclkr.Driveroff */
	{MCASP3_ACLKX, (M0 | PIN_INPUT_PULLDOWN)},	/* mcasp3_aclkx.mcasp3_aclkx */
	{MCASP3_FSX, (M0 | PIN_INPUT_SLEW)},	/* mcasp3_fsx.mcasp3_fsx */
	{MCASP3_AXR0, (M0 | PIN_INPUT_SLEW)},	/* mcasp3_axr0.mcasp3_axr0 */
	{MCASP3_AXR1, (M0 | PIN_INPUT_SLEW | VIRTUAL_MODE6)},	/* mcasp3_axr1.mcasp3_axr1 */
	{MMC1_CLK, (M0 | PIN_INPUT_PULLUP)},	/* mmc1_clk.mmc1_clk */
	{MMC1_CMD, (M0 | PIN_INPUT_PULLUP)},	/* mmc1_cmd.mmc1_cmd */
	{MMC1_DAT0, (M0 | PIN_INPUT_PULLUP)},	/* mmc1_dat0.mmc1_dat0 */
	{MMC1_DAT1, (M0 | PIN_INPUT_PULLUP)},	/* mmc1_dat1.mmc1_dat1 */
	{MMC1_DAT2, (M0 | PIN_INPUT_PULLUP)},	/* mmc1_dat2.mmc1_dat2 */
	{MMC1_DAT3, (M0 | PIN_INPUT_PULLUP)},	/* mmc1_dat3.mmc1_dat3 */
	{MMC1_SDCD, (M0 | PIN_INPUT_PULLUP | SLEWCONTROL)},	/* mmc1_sdcd.mmc1_sdcd */
	{MMC1_SDWP, (M14 | PIN_INPUT_SLEW)},	/* mmc1_sdwp.gpio6_28 */
	{SPI1_SCLK, (M0 | PIN_INPUT_PULLDOWN)},	/* spi1_sclk.spi1_sclk */
	{SPI1_D1, (M0 | PIN_INPUT_PULLDOWN)},	/* spi1_d1.spi1_d1 */
	{SPI1_D0, (M0 | PIN_INPUT_PULLDOWN)},	/* spi1_d0.spi1_d0 */
	{SPI1_CS0, (M0 | PIN_INPUT_PULLUP)},	/* spi1_cs0.spi1_cs0 */
	{SPI1_CS2, (M14 | PIN_INPUT_PULLDOWN)},	/* spi1_cs2.gpio7_12 */
	{SPI1_CS3, (M6 | PIN_INPUT | SLEWCONTROL)},	/* spi1_cs3.hdmi1_cec */
	{SPI2_SCLK, (M1 | PIN_INPUT_PULLDOWN)},	/* spi2_sclk.uart3_rxd */
	{SPI2_D1, (M1 | PIN_INPUT_SLEW)},	/* spi2_d1.uart3_txd */
	{SPI2_D0, (M1 | PIN_INPUT_SLEW)},	/* spi2_d0.uart3_ctsn */
	{SPI2_CS0, (M1 | PIN_INPUT_PULLUP | SLEWCONTROL)},	/* spi2_cs0.uart3_rtsn */
	{DCAN1_TX, (M0 | PIN_INPUT_PULLUP | SLEWCONTROL)},	/* dcan1_tx.dcan1_tx */
	{DCAN1_RX, (M0 | PIN_INPUT_PULLUP | SLEWCONTROL)},	/* dcan1_rx.dcan1_rx */
	{UART1_RXD, (M0 | PIN_INPUT_PULLUP | SLEWCONTROL)},	/* uart1_rxd.uart1_rxd */
	{UART1_TXD, (M0 | PIN_INPUT_PULLUP | SLEWCONTROL)},	/* uart1_txd.uart1_txd */
	{UART1_CTSN, (M3 | PIN_INPUT_PULLUP)},	/* uart1_ctsn.mmc4_clk */
	{UART1_RTSN, (M3 | PIN_INPUT_PULLUP)},	/* uart1_rtsn.mmc4_cmd */
	{UART2_RXD, (M3 | PIN_INPUT_PULLUP)},	/* N/A.mmc4_dat0 */
	{UART2_TXD, (M3 | PIN_INPUT_PULLUP)},	/* uart2_txd.mmc4_dat1 */
	{UART2_CTSN, (M3 | PIN_INPUT_PULLUP)},	/* uart2_ctsn.mmc4_dat2 */
	{UART2_RTSN, (M3 | PIN_INPUT_PULLUP)},	/* uart2_rtsn.mmc4_dat3 */
	{I2C2_SDA, (M1 | PIN_INPUT_PULLUP)},	/* i2c2_sda.hdmi1_ddc_scl */
	{I2C2_SCL, (M1 | PIN_INPUT_PULLUP)},	/* i2c2_scl.hdmi1_ddc_sda */
	{WAKEUP0, (M14 | PIN_OUTPUT)},	/* N/A.gpio1_0 */
	{WAKEUP1, (M14 | PIN_OUTPUT)},	/* N/A.gpio1_1 */
	{WAKEUP2, (M14 | PIN_INPUT)},	/* N/A.gpio1_2 */
	{WAKEUP3, (M1 | PIN_OUTPUT)},	/* N/A.sys_nirq1 */
};

#ifdef CONFIG_IODELAY_RECALIBRATION
const struct iodelay_cfg_entry dra742_es1_1_iodelay_cfg_array[] = {
	{0x06F0, 480, 0},	/* CFG_RGMII0_RXC_IN */
	{0x06FC, 111, 1641},	/* CFG_RGMII0_RXCTL_IN */
	{0x0708, 272, 1116},	/* CFG_RGMII0_RXD0_IN */
	{0x0714, 243, 1260},	/* CFG_RGMII0_RXD1_IN */
	{0x0720, 0, 1614},	/* CFG_RGMII0_RXD2_IN */
	{0x072C, 105, 1673},	/* CFG_RGMII0_RXD3_IN */
	{0x0740, 0, 0},		/* CFG_RGMII0_TXC_OUT */
	{0x074C, 1560, 120},	/* CFG_RGMII0_TXCTL_OUT */
	{0x0758, 1570, 120},	/* CFG_RGMII0_TXD0_OUT */
	{0x0764, 1500, 120},	/* CFG_RGMII0_TXD1_OUT */
	{0x0770, 1775, 120},	/* CFG_RGMII0_TXD2_OUT */
	{0x077C, 1875, 120},	/* CFG_RGMII0_TXD3_OUT */
	{0x08D0, 0, 0},		/* CFG_VIN1A_CLK0_IN */
	{0x08DC, 2600, 0},	/* CFG_VIN1A_D0_IN */
	{0x08E8, 2652, 46},	/* CFG_VIN1A_D10_IN */
	{0x08F4, 2541, 0},	/* CFG_VIN1A_D11_IN */
	{0x0900, 2603, 574},	/* CFG_VIN1A_D12_IN */
	{0x090C, 2548, 443},	/* CFG_VIN1A_D13_IN */
	{0x0918, 2624, 598},	/* CFG_VIN1A_D14_IN */
	{0x0924, 2535, 1027},	/* CFG_VIN1A_D15_IN */
	{0x0930, 2526, 818},	/* CFG_VIN1A_D16_IN */
	{0x093C, 2623, 797},	/* CFG_VIN1A_D17_IN */
	{0x0948, 2578, 888},	/* CFG_VIN1A_D18_IN */
	{0x0954, 2574, 1008},	/* CFG_VIN1A_D19_IN */
	{0x0960, 2527, 123},	/* CFG_VIN1A_D1_IN */
	{0x096C, 2577, 737},	/* CFG_VIN1A_D20_IN */
	{0x0978, 2627, 616},	/* CFG_VIN1A_D21_IN */
	{0x0984, 2573, 777},	/* CFG_VIN1A_D22_IN */
	{0x0990, 2730, 67},	/* CFG_VIN1A_D23_IN */
	{0x099C, 2509, 303},	/* CFG_VIN1A_D2_IN */
	{0x09A8, 2494, 267},	/* CFG_VIN1A_D3_IN */
	{0x09B4, 2474, 0},	/* CFG_VIN1A_D4_IN */
	{0x09C0, 2556, 181},	/* CFG_VIN1A_D5_IN */
	{0x09CC, 2516, 195},	/* CFG_VIN1A_D6_IN */
	{0x09D8, 2589, 210},	/* CFG_VIN1A_D7_IN */
	{0x09E4, 2624, 75},	/* CFG_VIN1A_D8_IN */
	{0x09F0, 2704, 14},	/* CFG_VIN1A_D9_IN */
	{0x09FC, 2469, 55},	/* CFG_VIN1A_DE0_IN */
	{0x0A08, 2557, 264},	/* CFG_VIN1A_FLD0_IN */
	{0x0A14, 2465, 269},	/* CFG_VIN1A_HSYNC0_IN */
	{0x0A20, 2411, 348},	/* CFG_VIN1A_VSYNC0_IN */
	{0x0A70, 150, 0},	/* CFG_VIN2A_D12_OUT */
	{0x0A7C, 1500, 0},	/* CFG_VIN2A_D13_OUT */
	{0x0A88, 1600, 0},	/* CFG_VIN2A_D14_OUT */
	{0x0A94, 900, 0},	/* CFG_VIN2A_D15_OUT */
	{0x0AA0, 680, 0},	/* CFG_VIN2A_D16_OUT */
	{0x0AAC, 500, 0},	/* CFG_VIN2A_D17_OUT */
	{0x0AB0, 702, 0},	/* CFG_VIN2A_D18_IN */
	{0x0ABC, 136, 976},	/* CFG_VIN2A_D19_IN */
	{0x0AD4, 210, 1357},	/* CFG_VIN2A_D20_IN */
	{0x0AE0, 189, 1462},	/* CFG_VIN2A_D21_IN */
	{0x0AEC, 232, 1278},	/* CFG_VIN2A_D22_IN */
	{0x0AF8, 0, 1397},	/* CFG_VIN2A_D23_IN */
	{0x0144, 0, 0},         /* CFG_GPMC_A13_IN */
	{0x0150, 1976, 1389},   /* CFG_GPMC_A14_IN */
	{0x015C, 1872, 1408},   /* CFG_GPMC_A15_IN */
	{0x0168, 1914, 1506},   /* CFG_GPMC_A16_IN */
	{0x0170, 57, 0},        /* CFG_GPMC_A16_OUT */
	{0x0174, 1904, 1471},   /* CFG_GPMC_A17_IN */
	{0x0188, 1690, 0},      /* CFG_GPMC_A18_OUT */
	{0x0374, 0, 0},         /* CFG_GPMC_CS2_OUT */
};

const struct iodelay_cfg_entry dra742_es2_0_iodelay_cfg_array[] = {
	{0x06F0, 471, 0},	/* CFG_RGMII0_RXC_IN */
	{0x06FC, 30, 1919},	/* CFG_RGMII0_RXCTL_IN */
	{0x0708, 74, 1688},	/* CFG_RGMII0_RXD0_IN */
	{0x0714, 94, 1697},	/* CFG_RGMII0_RXD1_IN */
	{0x0720, 0, 1703},	/* CFG_RGMII0_RXD2_IN */
	{0x072C, 70, 1804},	/* CFG_RGMII0_RXD3_IN */
	{0x0740, 70, 70},	/* CFG_RGMII0_TXC_OUT */
	{0x074C, 35, 70},	/* CFG_RGMII0_TXCTL_OUT */
	{0x0758, 100, 130},	/* CFG_RGMII0_TXD0_OUT */
	{0x0764, 0, 70},	/* CFG_RGMII0_TXD1_OUT */
	{0x0770, 0, 0},	/* CFG_RGMII0_TXD2_OUT */
	{0x077C, 100, 130},	/* CFG_RGMII0_TXD3_OUT */
	{0x08D0, 0, 0},	/* CFG_VIN1A_CLK0_IN */
	{0x08DC, 2105, 619},	/* CFG_VIN1A_D0_IN */
	{0x08E8, 2107, 739},	/* CFG_VIN1A_D10_IN */
	{0x08F4, 2005, 788},	/* CFG_VIN1A_D11_IN */
	{0x0900, 2059, 1297},	/* CFG_VIN1A_D12_IN */
	{0x090C, 2027, 1141},	/* CFG_VIN1A_D13_IN */
	{0x0918, 2071, 1332},	/* CFG_VIN1A_D14_IN */
	{0x0924, 1995, 1764},	/* CFG_VIN1A_D15_IN */
	{0x0930, 1999, 1542},	/* CFG_VIN1A_D16_IN */
	{0x093C, 2072, 1540},	/* CFG_VIN1A_D17_IN */
	{0x0948, 2034, 1629},	/* CFG_VIN1A_D18_IN */
	{0x0954, 2026, 1761},	/* CFG_VIN1A_D19_IN */
	{0x0960, 2017, 757},	/* CFG_VIN1A_D1_IN */
	{0x096C, 2037, 1469},	/* CFG_VIN1A_D20_IN */
	{0x0978, 2077, 1349},	/* CFG_VIN1A_D21_IN */
	{0x0984, 2022, 1545},	/* CFG_VIN1A_D22_IN */
	{0x0990, 2168, 784},	/* CFG_VIN1A_D23_IN */
	{0x099C, 1996, 962},	/* CFG_VIN1A_D2_IN */
	{0x09A8, 1993, 901},	/* CFG_VIN1A_D3_IN */
	{0x09B4, 2098, 499},	/* CFG_VIN1A_D4_IN */
	{0x09C0, 2038, 844},	/* CFG_VIN1A_D5_IN */
	{0x09CC, 2002, 863},	/* CFG_VIN1A_D6_IN */
	{0x09D8, 2063, 873},	/* CFG_VIN1A_D7_IN */
	{0x09E4, 2088, 759},	/* CFG_VIN1A_D8_IN */
	{0x09F0, 2152, 701},	/* CFG_VIN1A_D9_IN */
	{0x09FC, 1926, 728},	/* CFG_VIN1A_DE0_IN */
	{0x0A08, 2043, 937},	/* CFG_VIN1A_FLD0_IN */
	{0x0A14, 1978, 909},	/* CFG_VIN1A_HSYNC0_IN */
	{0x0A20, 1926, 987},	/* CFG_VIN1A_VSYNC0_IN */
	{0x0A70, 140, 0},	/* CFG_VIN2A_D12_OUT */
	{0x0A7C, 90, 70},	/* CFG_VIN2A_D13_OUT */
	{0x0A88, 0, 0},	/* CFG_VIN2A_D14_OUT */
	{0x0A94, 0, 0},	/* CFG_VIN2A_D15_OUT */
	{0x0AA0, 0, 70},	/* CFG_VIN2A_D16_OUT */
	{0x0AAC, 0, 0},	/* CFG_VIN2A_D17_OUT */
	{0x0AB0, 612, 0},	/* CFG_VIN2A_D18_IN */
	{0x0ABC, 4, 927},	/* CFG_VIN2A_D19_IN */
	{0x0AD4, 136, 1340},	/* CFG_VIN2A_D20_IN */
	{0x0AE0, 130, 1450},	/* CFG_VIN2A_D21_IN */
	{0x0AEC, 144, 1269},	/* CFG_VIN2A_D22_IN */
	{0x0AF8, 0, 1330},	/* CFG_VIN2A_D23_IN */
	{0x0144, 0, 0},         /* CFG_GPMC_A13_IN */
	{0x0150, 2575, 966},    /* CFG_GPMC_A14_IN */
	{0x015C, 2503, 889},    /* CFG_GPMC_A15_IN */
	{0x0168, 2528, 1007},   /* CFG_GPMC_A16_IN */
	{0x0170, 0, 0},         /* CFG_GPMC_A16_OUT */
	{0x0174, 2533, 980},    /* CFG_GPMC_A17_IN */
	{0x0188, 590, 0},       /* CFG_GPMC_A18_OUT */
	{0x0374, 0, 0},         /* CFG_GPMC_CS2_OUT */
};

const struct iodelay_cfg_entry dra76x_es1_0_iodelay_cfg_array[] = {
	{0x011C, 787, 0},	/* CFG_GPMC_A0_OUT */
	{0x0128, 1181, 0},	/* CFG_GPMC_A10_OUT */
	{0x0144, 0, 0},	/* CFG_GPMC_A13_IN */
	{0x0150, 2149, 1052},	/* CFG_GPMC_A14_IN */
	{0x015C, 2121, 997},	/* CFG_GPMC_A15_IN */
	{0x0168, 2159, 1134},	/* CFG_GPMC_A16_IN */
	{0x0170, 0, 0},	/* CFG_GPMC_A16_OUT */
	{0x0174, 2135, 1085},	/* CFG_GPMC_A17_IN */
	{0x0188, 0, 0},	/* CFG_GPMC_A18_OUT */
	{0x01A0, 592, 0},	/* CFG_GPMC_A1_OUT */
	{0x020C, 641, 0},	/* CFG_GPMC_A2_OUT */
	{0x0218, 1481, 0},	/* CFG_GPMC_A3_OUT */
	{0x0224, 1775, 0},	/* CFG_GPMC_A4_OUT */
	{0x0230, 785, 0},	/* CFG_GPMC_A5_OUT */
	{0x023C, 848, 0},	/* CFG_GPMC_A6_OUT */
	{0x0248, 851, 0},	/* CFG_GPMC_A7_OUT */
	{0x0254, 1783, 0},	/* CFG_GPMC_A8_OUT */
	{0x0260, 951, 0},	/* CFG_GPMC_A9_OUT */
	{0x026C, 1091, 0},	/* CFG_GPMC_AD0_OUT */
	{0x0278, 1027, 0},	/* CFG_GPMC_AD10_OUT */
	{0x0284, 824, 0},	/* CFG_GPMC_AD11_OUT */
	{0x0290, 1196, 0},	/* CFG_GPMC_AD12_OUT */
	{0x029C, 754, 0},	/* CFG_GPMC_AD13_OUT */
	{0x02A8, 665, 0},	/* CFG_GPMC_AD14_OUT */
	{0x02B4, 1027, 0},	/* CFG_GPMC_AD15_OUT */
	{0x02C0, 937, 0},	/* CFG_GPMC_AD1_OUT */
	{0x02CC, 1168, 0},	/* CFG_GPMC_AD2_OUT */
	{0x02D8, 872, 0},	/* CFG_GPMC_AD3_OUT */
	{0x02E4, 1092, 0},	/* CFG_GPMC_AD4_OUT */
	{0x02F0, 576, 0},	/* CFG_GPMC_AD5_OUT */
	{0x02FC, 1113, 0},	/* CFG_GPMC_AD6_OUT */
	{0x0308, 943, 0},	/* CFG_GPMC_AD7_OUT */
	{0x0314, 0, 0},	/* CFG_GPMC_AD8_OUT */
	{0x0320, 0, 0},	/* CFG_GPMC_AD9_OUT */
	{0x0374, 0, 0},	/* CFG_GPMC_CS2_OUT */
	{0x0380, 1801, 948},	/* CFG_GPMC_CS3_OUT */
	{0x06F0, 451, 0},	/* CFG_RGMII0_RXC_IN */
	{0x06FC, 127, 1571},	/* CFG_RGMII0_RXCTL_IN */
	{0x0708, 165, 1178},	/* CFG_RGMII0_RXD0_IN */
	{0x0714, 136, 1302},	/* CFG_RGMII0_RXD1_IN */
	{0x0720, 0, 1520},	/* CFG_RGMII0_RXD2_IN */
	{0x072C, 28, 1690},	/* CFG_RGMII0_RXD3_IN */
	{0x0740, 121, 0},	/* CFG_RGMII0_TXC_OUT */
	{0x074C, 60, 0},	/* CFG_RGMII0_TXCTL_OUT */
	{0x0758, 153, 0},	/* CFG_RGMII0_TXD0_OUT */
	{0x0764, 35, 0},	/* CFG_RGMII0_TXD1_OUT */
	{0x0770, 0, 0},	/* CFG_RGMII0_TXD2_OUT */
	{0x077C, 172, 0},	/* CFG_RGMII0_TXD3_OUT */
	{0x0A38, 0, 0},	/* CFG_VIN2A_CLK0_IN */
	{0x0A44, 2180, 0},	/* CFG_VIN2A_D0_IN */
	{0x0A50, 2297, 110},	/* CFG_VIN2A_D10_IN */
	{0x0A5C, 1938, 0},	/* CFG_VIN2A_D11_IN */
	{0x0A70, 147, 0},	/* CFG_VIN2A_D12_OUT */
	{0x0A7C, 110, 0},	/* CFG_VIN2A_D13_OUT */
	{0x0A88, 18, 0},	/* CFG_VIN2A_D14_OUT */
	{0x0A94, 82, 0},	/* CFG_VIN2A_D15_OUT */
	{0x0AA0, 33, 0},	/* CFG_VIN2A_D16_OUT */
	{0x0AAC, 0, 0},	/* CFG_VIN2A_D17_OUT */
	{0x0AB0, 417, 0},	/* CFG_VIN2A_D18_IN */
	{0x0ABC, 156, 843},	/* CFG_VIN2A_D19_IN */
	{0x0AC8, 2326, 309},	/* CFG_VIN2A_D1_IN */
	{0x0AD4, 223, 1413},	/* CFG_VIN2A_D20_IN */
	{0x0AE0, 169, 1415},	/* CFG_VIN2A_D21_IN */
	{0x0AEC, 43, 1150},	/* CFG_VIN2A_D22_IN */
	{0x0AF8, 0, 1210},	/* CFG_VIN2A_D23_IN */
	{0x0B04, 2057, 0},	/* CFG_VIN2A_D2_IN */
	{0x0B10, 2440, 257},	/* CFG_VIN2A_D3_IN */
	{0x0B1C, 2142, 0},	/* CFG_VIN2A_D4_IN */
	{0x0B28, 2455, 252},	/* CFG_VIN2A_D5_IN */
	{0x0B34, 1883, 0},	/* CFG_VIN2A_D6_IN */
	{0x0B40, 2229, 0},	/* CFG_VIN2A_D7_IN */
	{0x0B4C, 2250, 151},	/* CFG_VIN2A_D8_IN */
	{0x0B58, 2279, 27},	/* CFG_VIN2A_D9_IN */
	{0x0B7C, 2233, 0},	/* CFG_VIN2A_HSYNC0_IN */
	{0x0B88, 1936, 0},	/* CFG_VIN2A_VSYNC0_IN */
	{0x0B9C, 1281, 497},	/* CFG_VOUT1_CLK_OUT */
	{0x0BA8, 379, 0},	/* CFG_VOUT1_D0_OUT */
	{0x0BB4, 441, 0},	/* CFG_VOUT1_D10_OUT */
	{0x0BC0, 461, 0},	/* CFG_VOUT1_D11_OUT */
	{0x0BCC, 1189, 0},	/* CFG_VOUT1_D12_OUT */
	{0x0BD8, 312, 0},	/* CFG_VOUT1_D13_OUT */
	{0x0BE4, 298, 0},	/* CFG_VOUT1_D14_OUT */
	{0x0BF0, 284, 0},	/* CFG_VOUT1_D15_OUT */
	{0x0BFC, 152, 0},	/* CFG_VOUT1_D16_OUT */
	{0x0C08, 216, 0},	/* CFG_VOUT1_D17_OUT */
	{0x0C14, 408, 0},	/* CFG_VOUT1_D18_OUT */
	{0x0C20, 519, 0},	/* CFG_VOUT1_D19_OUT */
	{0x0C2C, 475, 0},	/* CFG_VOUT1_D1_OUT */
	{0x0C38, 316, 0},	/* CFG_VOUT1_D20_OUT */
	{0x0C44, 59, 0},	/* CFG_VOUT1_D21_OUT */
	{0x0C50, 221, 0},	/* CFG_VOUT1_D22_OUT */
	{0x0C5C, 96, 0},	/* CFG_VOUT1_D23_OUT */
	{0x0C68, 264, 0},	/* CFG_VOUT1_D2_OUT */
	{0x0C74, 421, 0},	/* CFG_VOUT1_D3_OUT */
	{0x0C80, 1257, 0},	/* CFG_VOUT1_D4_OUT */
	{0x0C8C, 432, 0},	/* CFG_VOUT1_D5_OUT */
	{0x0C98, 436, 0},	/* CFG_VOUT1_D6_OUT */
	{0x0CA4, 440, 0},	/* CFG_VOUT1_D7_OUT */
	{0x0CB0, 81, 100},	/* CFG_VOUT1_D8_OUT */
	{0x0CBC, 471, 0},	/* CFG_VOUT1_D9_OUT */
	{0x0CC8, 0, 0},	/* CFG_VOUT1_DE_OUT */
	{0x0CE0, 0, 0},	/* CFG_VOUT1_HSYNC_OUT */
	{0x0CEC, 815, 0},	/* CFG_VOUT1_VSYNC_OUT */
};
#endif

#endif /* _MUX_DATA_DRA7XX_H_ */
