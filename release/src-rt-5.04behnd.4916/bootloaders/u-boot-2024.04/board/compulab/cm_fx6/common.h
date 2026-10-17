/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2014, Compulab Ltd - http://compulab.co.il/
 *
 * Author: Nikita Kiryanov <nikita@compulab.co.il>
 */

#include <asm/arch/mx6-pins.h>
#include <asm/arch/clock.h>

#define UART_PAD_CTRL  (PAD_CTL_PUS_100K_UP |	\
			PAD_CTL_SPEED_MED | PAD_CTL_DSE_40ohm |	\
			PAD_CTL_SRE_FAST  | PAD_CTL_HYS)

#define CM_FX6_ECSPI_BUS0_CS0	IMX_GPIO_NR(2, 30)
#define CM_FX6_GREEN_LED	IMX_GPIO_NR(2, 31)
#define CM_FX6_ENET_NRST	IMX_GPIO_NR(2, 8)
#define CM_FX6_ENET_NRST	IMX_GPIO_NR(2, 8)
#define CM_FX6_USB_HUB_RST	IMX_GPIO_NR(7, 8)
#define SB_FX6_USB_OTG_PWR	IMX_GPIO_NR(3, 22)
#define CM_FX6_ENET_NRST	IMX_GPIO_NR(2, 8)
#define CM_FX6_USB_HUB_RST	IMX_GPIO_NR(7, 8)
#define SB_FX6_USB_OTG_PWR	IMX_GPIO_NR(3, 22)
#define CM_FX6_SATA_PWREN	IMX_GPIO_NR(1, 28)
#define CM_FX6_SATA_VDDC_CTRL	IMX_GPIO_NR(1, 30)
#define CM_FX6_SATA_LDO_EN	IMX_GPIO_NR(2, 16)
#define CM_FX6_SATA_NSTANDBY1	IMX_GPIO_NR(3, 20)
#define CM_FX6_SATA_PHY_SLP	IMX_GPIO_NR(3, 23)
#define CM_FX6_SATA_STBY_REQ	IMX_GPIO_NR(3, 29)
#define CM_FX6_SATA_NSTANDBY2	IMX_GPIO_NR(5, 2)
#define CM_FX6_SATA_NRSTDLY	IMX_GPIO_NR(6, 6)
#define CM_FX6_SATA_PWLOSS_INT	IMX_GPIO_NR(6, 31)


void cm_fx6_set_usdhc_iomux(void);
void cm_fx6_set_ecspi_iomux(void);
