/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2013 Gateworks Corporation
 *
 * Author: Tim Harvey <tharvey@gateworks.com>
 */

#ifndef __ASSEMBLY__

/* i2c slave addresses */
#define GSC_SC_ADDR		0x20
#define GSC_RTC_ADDR		0x68
#define GSC_HWMON_ADDR		0x29
#define GSC_EEPROM_ADDR		0x51

/* System Controller registers */
enum {
	GSC_SC_CTRL0		= 0x00,
	GSC_SC_CTRL1		= 0x01,
	GSC_SC_STATUS		= 0x0a,
	GSC_SC_FWCRC		= 0x0c,
	GSC_SC_FWVER		= 0x0e,
};

/* System Controller Control1 bits */
enum {
	GSC_SC_CTRL1_WDTIME	= 4, /* 1 = 60s timeout, 0 = 30s timeout */
	GSC_SC_CTRL1_WDEN	= 5, /* 1 = enable, 0 = disable */
	GSC_SC_CTRL1_WDDIS	= 7, /* 1 = disable boot watchdog */
};

/* System Controller Interrupt bits */
enum {
	GSC_SC_IRQ_PB		= 0, /* Pushbutton switch */
	GSC_SC_IRQ_SECURE	= 1, /* Secure Key erase operation complete */
	GSC_SC_IRQ_EEPROM_WP	= 2, /* EEPROM write violation */
	GSC_SC_IRQ_GPIO		= 4, /* GPIO change */
	GSC_SC_IRQ_TAMPER	= 5, /* Tamper detect */
	GSC_SC_IRQ_WATCHDOG	= 6, /* Watchdog trip */
	GSC_SC_IRQ_PBLONG	= 7, /* Pushbutton long hold */
};

/* Hardware Monitor registers */
enum {
	GSC_HWMON_TEMP		= 0x00,
	GSC_HWMON_VIN		= 0x02,
	GSC_HWMON_VDD_3P3	= 0x05,
	GSC_HWMON_VBATT		= 0x08,
	GSC_HWMON_VDD_5P0	= 0x0b,
	GSC_HWMON_VDD_CORE	= 0x0e,
	GSC_HWMON_VDD_SOC	= 0x11,
	GSC_HWMON_VDD_HIGH	= 0x14,
	GSC_HWMON_VDD_DDR	= 0x17,
	GSC_HWMON_VDD_EXT	= 0x1a,
	GSC_HWMON_VDD_1P8	= 0x1d,
	GSC_HWMON_VDD_IO2	= 0x20,
	GSC_HWMON_VDD_2P5	= 0x23,
	GSC_HWMON_VDD_IO3	= 0x26,
	GSC_HWMON_VDD_IO4	= 0x29,
};

/*
 * I2C transactions to the GSC are done via these functions which
 * perform retries in the case of a busy GSC NAK'ing the transaction
 */
int gsc_i2c_read(uchar chip, uint addr, int alen, uchar *buf, int len);
int gsc_i2c_write(uchar chip, uint addr, int alen, uchar *buf, int len);
int gsc_info(int verbose);
int gsc_boot_wd_disable(void);
#endif

