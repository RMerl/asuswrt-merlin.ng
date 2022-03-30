/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2010-2013 Freescale Semiconductor, Inc.
 * Copyright (C) 2014 Bachmann electronic GmbH
 */

#ifndef __CONFIG_H
#define __CONFIG_H

#include "mx6_common.h"

/* Size of malloc() pool */
#define CONFIG_SYS_MALLOC_LEN           (10 * 1024 * 1024)

/* UART Configs */
#define CONFIG_MXC_UART
#define CONFIG_MXC_UART_BASE           UART1_BASE

/* SF Configs */

/* IO expander */
#define CONFIG_PCA953X
#define CONFIG_SYS_I2C_PCA953X_ADDR	0x20
#define CONFIG_SYS_I2C_PCA953X_WIDTH	{ {0x20, 16} }

/* I2C Configs */
#define CONFIG_SYS_I2C
#define CONFIG_SYS_I2C_MXC
#define CONFIG_SYS_I2C_MXC_I2C1		/* enable I2C bus 1 */
#define CONFIG_SYS_I2C_MXC_I2C2		/* enable I2C bus 2 */
#define CONFIG_SYS_I2C_MXC_I2C3		/* enable I2C bus 3 */
#define CONFIG_SYS_I2C_SPEED            100000

/* OCOTP Configs */
#define CONFIG_IMX_OTP
#define IMX_OTP_BASE                    OCOTP_BASE_ADDR
#define IMX_OTP_ADDR_MAX                0x7F
#define IMX_OTP_DATA_ERROR_VAL          0xBADABADA
#define IMX_OTPWRITE_ENABLED

/* MMC Configs */
#define CONFIG_SYS_FSL_ESDHC_ADDR      0
#define CONFIG_SYS_FSL_USDHC_NUM       2

/* USB Configs */
#define CONFIG_MXC_USB_PORTSC   (PORT_PTS_UTMI | PORT_PTS_PTW)
#define CONFIG_USB_MAX_CONTROLLER_COUNT 2

/*
 * SATA Configs
 */
#ifdef CONFIG_CMD_SATA
#define CONFIG_SYS_SATA_MAX_DEVICE	1
#define CONFIG_DWC_AHSATA_PORT_ID	0
#define CONFIG_DWC_AHSATA_BASE_ADDR	SATA_ARB_BASE_ADDR
#define CONFIG_LBA48
#endif

/* SPL */
#ifdef CONFIG_SPL
#include "imx6_spl.h"
#define CONFIG_SYS_SPI_U_BOOT_OFFS     (64 * 1024)
#endif

#define CONFIG_FEC_MXC
#define IMX_FEC_BASE                    ENET_BASE_ADDR
#define CONFIG_FEC_XCV_TYPE             MII100
#define CONFIG_ETHPRIME                 "FEC"
#define CONFIG_FEC_MXC_PHYADDR          0x5
#define CONFIG_PHY_SMSC

#ifndef CONFIG_SPL
#define CONFIG_ENV_EEPROM_IS_ON_I2C
#define CONFIG_SYS_I2C_EEPROM_BUS             1
#define CONFIG_SYS_I2C_EEPROM_ADDR_LEN        1
#define CONFIG_SYS_EEPROM_PAGE_WRITE_BITS     3
#define CONFIG_SYS_EEPROM_PAGE_WRITE_DELAY_MS 5
#endif

#define CONFIG_PREBOOT                 ""

/* Thermal support */
#define CONFIG_IMX_THERMAL

/* Physical Memory Map */
#define PHYS_SDRAM                     MMDC0_ARB_BASE_ADDR

#define CONFIG_SYS_SDRAM_BASE          PHYS_SDRAM
#define CONFIG_SYS_INIT_RAM_ADDR       IRAM_BASE_ADDR
#define CONFIG_SYS_INIT_RAM_SIZE       IRAM_SIZE

#define CONFIG_SYS_INIT_SP_OFFSET \
	(CONFIG_SYS_INIT_RAM_SIZE - GENERATED_GBL_DATA_SIZE)
#define CONFIG_SYS_INIT_SP_ADDR \
	(CONFIG_SYS_INIT_RAM_ADDR + CONFIG_SYS_INIT_SP_OFFSET)

/* Environment organization */
#define CONFIG_ENV_SIZE                 (64 * 1024)	/* 64 kb */
#define CONFIG_ENV_OFFSET               (1024 * 1024)
/* M25P16 has an erase size of 64 KiB */
#define CONFIG_ENV_SECT_SIZE            (64 * 1024)

#define CONFIG_BOOTP_SERVERIP
#define CONFIG_BOOTP_BOOTFILE

#endif         /* __CONFIG_H */
