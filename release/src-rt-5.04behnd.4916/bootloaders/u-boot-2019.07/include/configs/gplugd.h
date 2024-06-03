/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2011
 * eInfochips Ltd. <www.einfochips.com>
 * Written-by: Ajay Bhargav <contact@8051projects.net>
 *
 * Based on Aspenite:
 * (C) Copyright 2010
 * Marvell Semiconductor <www.marvell.com>
 * Written-by: Prafulla Wadaskar <prafulla@marvell.com>
 * Contributor: Mahavir Jain <mjain@marvell.com>
 */

#ifndef __CONFIG_GPLUGD_H
#define __CONFIG_GPLUGD_H

/*
 * High Level Configuration Options
 */
#define CONFIG_SHEEVA_88SV331xV5	1	/* CPU Core subversion */
#define CONFIG_ARMADA100		1	/* SOC Family Name */
#define CONFIG_ARMADA168		1	/* SOC Used on this Board */
#define CONFIG_MACH_TYPE		MACH_TYPE_GPLUGD /* Machine type */
#define CONFIG_SKIP_LOWLEVEL_INIT	/* disable board lowlevel_init */

/*
 * There is no internal RAM in ARMADA100, using DRAM
 * TBD: dcache to be used for this
 */
#define CONFIG_SYS_INIT_SP_ADDR		(CONFIG_SYS_TEXT_BASE - 0x00200000)

/*
 * Commands configuration
 */

/* Network configuration */
#ifdef CONFIG_CMD_NET
#define CONFIG_ARMADA100_FEC

/* DHCP Support */
#define CONFIG_BOOTP_DHCP_REQUEST_DELAY		50000
#endif /* CONFIG_CMD_NET */

/* GPIO Support */
#define CONFIG_MARVELL_GPIO

/* PHY configuration */
#define CONFIG_RESET_PHY_R
/* 88E3015 register definition */
#define PHY_LED_PAR_SEL_REG		22
#define PHY_LED_MAN_REG			25
#define PHY_LED_VAL			0x5b	/* LINK LED1, ACT LED2 */
/* GPIO Configuration for PHY */
#define CONFIG_SYS_GPIO_PHY_RST		104	/* GPIO104 */

/* Flash Support */

/*
 * mv-common.h should be defined after CMD configs since it used them
 * to enable certain macros
 */
#include "mv-common.h"

#ifdef CONFIG_SYS_NS16550_COM1
#undef CONFIG_SYS_NS16550_COM1
#endif /* CONFIG_SYS_NS16550_COM1 */

#define CONFIG_SYS_NS16550_COM1 ARMD1_UART3_BASE

/*
 * Environment variables configurations
 */
#define CONFIG_ENV_SIZE			0x4000

#ifdef CONFIG_CMD_USB
#define CONFIG_USB_EHCI_ARMADA100
#define CONFIG_EHCI_IS_TDI
#endif /* CONFIG_CMD_USB */

#endif	/* __CONFIG_GPLUGD_H */
