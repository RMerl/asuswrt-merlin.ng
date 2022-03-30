/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2014 Stefan Roese <sr@denx.de>
 */

#ifndef _PLATINUM_H_
#define _PLATINUM_H_

#include <miiphy.h>
#include <asm/arch/crm_regs.h>
#include <asm/io.h>

/* Defines */

#define ECSPI1_PAD_CLK		(PAD_CTL_SRE_SLOW | PAD_CTL_PUS_100K_DOWN | \
				 PAD_CTL_SPEED_LOW | PAD_CTL_DSE_40ohm | \
				 PAD_CTL_HYS)
#define ECSPI2_PAD_CLK		(PAD_CTL_SRE_FAST | PAD_CTL_PUS_100K_DOWN | \
				 PAD_CTL_SPEED_MED | PAD_CTL_DSE_40ohm | \
				 PAD_CTL_HYS)
#define ECSPI_PAD_MOSI		(PAD_CTL_SRE_SLOW | PAD_CTL_PUS_100K_DOWN | \
				 PAD_CTL_SPEED_LOW | PAD_CTL_DSE_120ohm | \
				 PAD_CTL_HYS)
#define ECSPI_PAD_MISO		(PAD_CTL_SRE_FAST | PAD_CTL_PUS_100K_DOWN | \
				 PAD_CTL_SPEED_LOW | PAD_CTL_DSE_40ohm | \
				 PAD_CTL_HYS)
#define ECSPI_PAD_SS		(PAD_CTL_SRE_SLOW | PAD_CTL_PUS_100K_UP | \
				 PAD_CTL_SPEED_LOW | PAD_CTL_DSE_120ohm | \
				 PAD_CTL_HYS)

#define ENET_PAD_CTRL		(PAD_CTL_PUS_100K_UP | PAD_CTL_SPEED_MED | \
				 PAD_CTL_DSE_40ohm | PAD_CTL_HYS)

#define I2C_PAD_CTRL		(PAD_CTL_PUS_100K_UP | PAD_CTL_SPEED_MED | \
				 PAD_CTL_DSE_40ohm | PAD_CTL_HYS | \
				 PAD_CTL_ODE | PAD_CTL_SRE_FAST)
#define I2C_PAD_CTRL_SCL	(PAD_CTL_PUS_100K_UP | PAD_CTL_SPEED_LOW | \
				 PAD_CTL_DSE_80ohm | PAD_CTL_HYS | \
				 PAD_CTL_ODE | PAD_CTL_SRE_SLOW)

#define UART_PAD_CTRL		(PAD_CTL_PUS_100K_UP | PAD_CTL_SPEED_MED | \
				 PAD_CTL_DSE_40ohm | PAD_CTL_SRE_FAST | \
				 PAD_CTL_HYS)

#define USDHC_PAD_CTRL		(PAD_CTL_PUS_47K_UP  | PAD_CTL_SPEED_LOW | \
				PAD_CTL_DSE_80ohm | PAD_CTL_SRE_FAST | \
				PAD_CTL_HYS)


#define PC			MUX_PAD_CTRL(I2C_PAD_CTRL)
#define PC_SCL			MUX_PAD_CTRL(I2C_PAD_CTRL_SCL)

/* Prototypes */

int platinum_setup_enet(void);
int platinum_setup_i2c(void);
int platinum_setup_spi(void);
int platinum_setup_uart(void);
int platinum_phy_config(struct phy_device *phydev);
int platinum_init_gpio(void);
int platinum_init_usb(void);
int platinum_init_finished(void);

static inline void ccgr_init(void)
{
	struct mxc_ccm_reg *ccm = (struct mxc_ccm_reg *)CCM_BASE_ADDR;

	writel(0x00C03F3F, &ccm->CCGR0);
	writel(0x0030FC03, &ccm->CCGR1);
	writel(0x0FFFC000, &ccm->CCGR2);
	writel(0x3FF00000, &ccm->CCGR3);
	writel(0xFFFFF300, &ccm->CCGR4);	/* enable NAND/GPMI/BCH clks */
	writel(0x0F0000C3, &ccm->CCGR5);
	writel(0x000003FF, &ccm->CCGR6);
}

#endif /* _PLATINUM_H_ */
