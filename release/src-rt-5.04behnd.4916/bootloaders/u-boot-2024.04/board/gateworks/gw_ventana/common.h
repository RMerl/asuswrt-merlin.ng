/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2013 Gateworks Corporation
 *
 * Author: Tim Harvey <tharvey@gateworks.com>
 */

#ifndef _GWVENTANA_COMMON_H_
#define _GWVENTANA_COMMON_H_

#include "eeprom.h"

/* GPIO's common to all baseboards */
#define GP_RS232_EN	IMX_GPIO_NR(2, 11)
#define GP_MSATA_SEL	IMX_GPIO_NR(2, 8)

#define UART_PAD_CTRL  (PAD_CTL_PKE | PAD_CTL_PUE |		\
	PAD_CTL_PUS_100K_UP | PAD_CTL_SPEED_MED |		\
	PAD_CTL_DSE_40ohm   | PAD_CTL_SRE_FAST  | PAD_CTL_HYS)

#define USDHC_PAD_CTRL (PAD_CTL_PKE | PAD_CTL_PUE |		\
	PAD_CTL_PUS_47K_UP  | PAD_CTL_SPEED_LOW |		\
	PAD_CTL_DSE_80ohm   | PAD_CTL_SRE_FAST  | PAD_CTL_HYS)

#define ENET_PAD_CTRL  (PAD_CTL_PKE | PAD_CTL_PUE |		\
	PAD_CTL_PUS_100K_UP | PAD_CTL_SPEED_MED	  |		\
	PAD_CTL_DSE_40ohm   | PAD_CTL_HYS)

#define SPI_PAD_CTRL (PAD_CTL_HYS |				\
	PAD_CTL_PUS_100K_DOWN | PAD_CTL_SPEED_MED |		\
	PAD_CTL_DSE_40ohm     | PAD_CTL_SRE_FAST)

#define I2C_PAD_CTRL  (PAD_CTL_PUS_100K_UP |			\
	PAD_CTL_SPEED_MED | PAD_CTL_DSE_40ohm | PAD_CTL_HYS |	\
	PAD_CTL_ODE | PAD_CTL_SRE_FAST)

#define IRQ_PAD_CTRL  (PAD_CTL_PKE | PAD_CTL_PUE |		\
	PAD_CTL_PUS_100K_UP | PAD_CTL_SPEED_MED |		\
	PAD_CTL_DSE_34ohm | PAD_CTL_HYS | PAD_CTL_SRE_FAST)

#define DIO_PAD_CFG   (MUX_PAD_CTRL(IRQ_PAD_CTRL) | MUX_MODE_SION)

#define PC MUX_PAD_CTRL(I2C_PAD_CTRL)

/*
 * each baseboard has an optional set user configurable Digital IO lines which
 * can be pinmuxed as a GPIO or in some cases a PWM
 */
struct dio_cfg {
	iomux_v3_cfg_t gpio_padmux[2];
	unsigned gpio_param;
	iomux_v3_cfg_t pwm_padmux[2];
	unsigned pwm_param;
};

struct ventana {
	/* pinmux */
	iomux_v3_cfg_t const *gpio_pads;
	int num_pads;
	/* DIO pinmux/val */
	struct dio_cfg *dio_cfg;
	int dio_num;
	/* various gpios (0 if non-existent) */
	int mezz_pwren;
	int mezz_irq;
	int rs485en;
	int gps_shdn;
	int dioi2c_en;
	int pcie_sson;
	int usb_sel;
	int wdis;
	int msata_en;
	int rs232_en;
	int vsel_pin;
	int mmc_cd;
	/* various features */
	bool usd_vsel;
};

extern struct ventana gpio_cfg[GW_UNKNOWN];

/* configure gpio iomux/defaults */
void setup_iomux_gpio(int board);

#endif /* #ifndef _GWVENTANA_COMMON_H_ */
