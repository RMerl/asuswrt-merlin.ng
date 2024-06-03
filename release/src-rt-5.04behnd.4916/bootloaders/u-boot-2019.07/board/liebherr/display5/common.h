/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2017 DENX Software Engineering
 * Lukasz Majewski, DENX Software Engineering, lukma@denx.de
 */

#ifndef __DISPL5_COMMON_H_
#define __DISPL5_COMMON_H_

#define UART_PAD_CTRL  (PAD_CTL_PKE | PAD_CTL_PUE |	       \
	PAD_CTL_PUS_100K_UP | PAD_CTL_SPEED_MED |	       \
	PAD_CTL_DSE_40ohm   | PAD_CTL_SRE_FAST  | PAD_CTL_HYS)

#define USDHC_PAD_CTRL (PAD_CTL_PKE | PAD_CTL_PUE |	       \
	PAD_CTL_PUS_47K_UP  | PAD_CTL_SPEED_LOW |	       \
	PAD_CTL_DSE_80ohm   | PAD_CTL_SRE_FAST  | PAD_CTL_HYS)

#define ENET_PAD_CTRL  (PAD_CTL_PKE | PAD_CTL_PUE |		\
	PAD_CTL_PUS_22K_UP | PAD_CTL_SPEED_MED	  |		\
	PAD_CTL_DSE_40ohm   | PAD_CTL_HYS)

#define SPI_PAD_CTRL (PAD_CTL_HYS |				\
	PAD_CTL_PUS_100K_DOWN | PAD_CTL_SPEED_MED |		\
	PAD_CTL_DSE_40ohm   | PAD_CTL_SRE_FAST)

#define I2C_PAD_CTRL	(PAD_CTL_PKE | PAD_CTL_PUE |		\
	PAD_CTL_PUS_100K_UP | PAD_CTL_SPEED_MED |		\
	PAD_CTL_DSE_40ohm | PAD_CTL_HYS |			\
	PAD_CTL_ODE | PAD_CTL_SRE_FAST)

#define ENET_PAD_CTRL_CLK  ((PAD_CTL_PUS_100K_UP & ~PAD_CTL_PKE) | \
	PAD_CTL_SPEED_HIGH | PAD_CTL_DSE_40ohm | PAD_CTL_SRE_FAST)

void displ5_set_iomux_uart_spl(void);
void displ5_set_iomux_uart(void);
void displ5_set_iomux_ecspi_spl(void);
void displ5_set_iomux_ecspi(void);
void displ5_set_iomux_usdhc_spl(void);
void displ5_set_iomux_usdhc(void);
void displ5_set_iomux_misc_spl(void);

#endif /* __DISPL5_COMMON_H_ */
