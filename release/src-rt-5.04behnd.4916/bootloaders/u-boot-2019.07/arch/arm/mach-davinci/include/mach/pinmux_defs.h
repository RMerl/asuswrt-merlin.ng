/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Pinmux configurations for the DAxxx SoCs
 *
 * Copyright (C) 2011 OMICRON electronics GmbH
 */

#ifndef __ASM_ARCH_PINMUX_DEFS_H
#define __ASM_ARCH_PINMUX_DEFS_H

#include <asm/arch/davinci_misc.h>
#include <config.h>

/* SPI0 pin muxer settings */
extern const struct pinmux_config spi0_pins_base[3];
extern const struct pinmux_config spi0_pins_scs0[1];
extern const struct pinmux_config spi0_pins_ena[1];

/* SPI1 pin muxer settings */
extern const struct pinmux_config spi1_pins_base[3];
extern const struct pinmux_config spi1_pins_scs0[1];

/* UART pin muxer settings */
extern const struct pinmux_config uart0_pins_txrx[2];
extern const struct pinmux_config uart0_pins_rtscts[2];
extern const struct pinmux_config uart1_pins_txrx[2];
extern const struct pinmux_config uart2_pins_txrx[2];
extern const struct pinmux_config uart2_pins_rtscts[2];

/* EMAC pin muxer settings*/
extern const struct pinmux_config emac_pins_rmii[8];
extern const struct pinmux_config emac_pins_rmii_clk_source[1];
extern const struct pinmux_config emac_pins_mii[15];
extern const struct pinmux_config emac_pins_mdio[2];

/* I2C pin muxer settings */
extern const struct pinmux_config i2c0_pins[2];
extern const struct pinmux_config i2c1_pins[2];

/* EMIFA pin muxer settings */
extern const struct pinmux_config emifa_pins[40];
extern const struct pinmux_config emifa_pins_cs0[1];
extern const struct pinmux_config emifa_pins_cs2[1];
extern const struct pinmux_config emifa_pins_cs3[1];
extern const struct pinmux_config emifa_pins_cs4[1];
extern const struct pinmux_config emifa_pins_nand[12];
extern const struct pinmux_config emifa_pins_nor[43];

/* USB pin mux setting */
extern const struct pinmux_config usb_pins[1];

/* MMC pin muxer settings */
extern const struct pinmux_config mmc0_pins_8bit[10];
extern const struct pinmux_config mmc0_pins[6];

#endif
