// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2013 Gateworks Corporation
 *
 * Author: Tim Harvey <tharvey@gateworks.com>
 */

#include <linux/errno.h>
#include <common.h>
#include <i2c.h>
#include <linux/ctype.h>

#include "ventana_eeprom.h"
#include "gsc.h"

/*
 * The Gateworks System Controller will fail to ACK a master transaction if
 * it is busy, which can occur during its 1HZ timer tick while reading ADC's.
 * When this does occur, it will never be busy long enough to fail more than
 * 2 back-to-back transfers.  Thus we wrap i2c_read and i2c_write with
 * 3 retries.
 */
int gsc_i2c_read(uchar chip, uint addr, int alen, uchar *buf, int len)
{
	int retry = 3;
	int n = 0;
	int ret;

	while (n++ < retry) {
		ret = i2c_read(chip, addr, alen, buf, len);
		if (!ret)
			break;
		debug("%s: 0x%02x 0x%02x retry%d: %d\n", __func__, chip, addr,
		      n, ret);
		if (ret != -ENODEV)
			break;
		mdelay(10);
	}
	return ret;
}

int gsc_i2c_write(uchar chip, uint addr, int alen, uchar *buf, int len)
{
	int retry = 3;
	int n = 0;
	int ret;

	while (n++ < retry) {
		ret = i2c_write(chip, addr, alen, buf, len);
		if (!ret)
			break;
		debug("%s: 0x%02x 0x%02x retry%d: %d\n", __func__, chip, addr,
		      n, ret);
		if (ret != -ENODEV)
			break;
		mdelay(10);
	}
	mdelay(100);
	return ret;
}

static void read_hwmon(const char *name, uint reg, uint size)
{
	unsigned char buf[3];
	uint ui;

	printf("%-8s:", name);
	memset(buf, 0, sizeof(buf));
	if (gsc_i2c_read(GSC_HWMON_ADDR, reg, 1, buf, size)) {
		puts("fRD\n");
	} else {
		ui = buf[0] | (buf[1]<<8) | (buf[2]<<16);
		if (size == 2 && ui > 0x8000)
			ui -= 0xffff;
		if (ui == 0xffffff)
			puts("invalid\n");
		else
			printf("%d\n", ui);
	}
}

int gsc_info(int verbose)
{
	unsigned char buf[16];

	i2c_set_bus_num(0);
	if (gsc_i2c_read(GSC_SC_ADDR, 0, 1, buf, 16))
		return CMD_RET_FAILURE;

	printf("GSC:   v%d", buf[GSC_SC_FWVER]);
	printf(" 0x%04x", buf[GSC_SC_FWCRC] | buf[GSC_SC_FWCRC+1]<<8);
	printf(" WDT:%sabled", (buf[GSC_SC_CTRL1] & (1<<GSC_SC_CTRL1_WDEN))
		? "en" : "dis");
	if (buf[GSC_SC_STATUS] & (1 << GSC_SC_IRQ_WATCHDOG)) {
		buf[GSC_SC_STATUS] &= ~(1 << GSC_SC_IRQ_WATCHDOG);
		puts(" WDT_RESET");
		gsc_i2c_write(GSC_SC_ADDR, GSC_SC_STATUS, 1,
			      &buf[GSC_SC_STATUS], 1);
	}
	if (!gsc_i2c_read(GSC_HWMON_ADDR, GSC_HWMON_TEMP, 1, buf, 2)) {
		int ui = buf[0] | buf[1]<<8;
		if (ui > 0x8000)
			ui -= 0xffff;
		printf(" board temp at %dC", ui / 10);
	}
	puts("\n");
	if (!verbose)
		return CMD_RET_SUCCESS;

	read_hwmon("Temp",     GSC_HWMON_TEMP, 2);
	read_hwmon("VIN",      GSC_HWMON_VIN, 3);
	read_hwmon("VBATT",    GSC_HWMON_VBATT, 3);
	read_hwmon("VDD_3P3",  GSC_HWMON_VDD_3P3, 3);
	read_hwmon("VDD_ARM",  GSC_HWMON_VDD_CORE, 3);
	read_hwmon("VDD_SOC",  GSC_HWMON_VDD_SOC, 3);
	read_hwmon("VDD_HIGH", GSC_HWMON_VDD_HIGH, 3);
	read_hwmon("VDD_DDR",  GSC_HWMON_VDD_DDR, 3);
	read_hwmon("VDD_5P0",  GSC_HWMON_VDD_5P0, 3);
	if (strncasecmp((const char*) ventana_info.model, "GW553", 5))
		read_hwmon("VDD_2P5",  GSC_HWMON_VDD_2P5, 3);
	read_hwmon("VDD_1P8",  GSC_HWMON_VDD_1P8, 3);
	read_hwmon("VDD_IO2",  GSC_HWMON_VDD_IO2, 3);
	switch (ventana_info.model[3]) {
	case '1': /* GW51xx */
		read_hwmon("VDD_IO3",  GSC_HWMON_VDD_IO4, 3); /* -C rev */
		break;
	case '2': /* GW52xx */
		break;
	case '3': /* GW53xx */
		read_hwmon("VDD_IO4",  GSC_HWMON_VDD_IO4, 3); /* -C rev */
		read_hwmon("VDD_GPS",  GSC_HWMON_VDD_IO3, 3);
		break;
	case '4': /* GW54xx */
		read_hwmon("VDD_IO3",  GSC_HWMON_VDD_IO4, 3); /* -C rev */
		read_hwmon("VDD_GPS",  GSC_HWMON_VDD_IO3, 3);
		break;
	case '5': /* GW55xx */
		break;
	case '6': /* GW560x */
		read_hwmon("VDD_IO4",  GSC_HWMON_VDD_IO4, 3);
		read_hwmon("VDD_GPS",  GSC_HWMON_VDD_IO3, 3);
		break;
	case '9': /* GW590x */
		read_hwmon("AMONBMON",  GSC_HWMON_VDD_IO3, 3);
		read_hwmon("BAT_VOLT",  GSC_HWMON_VDD_EXT, 3);
		read_hwmon("BAT_TEMP",  GSC_HWMON_VDD_IO4, 2);
	}
	return 0;
}

/*
 *  The Gateworks System Controller implements a boot
 *  watchdog (always enabled) as a workaround for IMX6 boot related
 *  errata such as:
 *    ERR005768 - no fix scheduled
 *    ERR006282 - fixed in silicon r1.2
 *    ERR007117 - fixed in silicon r1.3
 *    ERR007220 - fixed in silicon r1.3
 *    ERR007926 - no fix scheduled
 *  see http://cache.freescale.com/files/32bit/doc/errata/IMX6DQCE.pdf
 *
 * Disable the boot watchdog
 */
int gsc_boot_wd_disable(void)
{
	u8 reg;

	i2c_set_bus_num(CONFIG_I2C_GSC);
	if (!gsc_i2c_read(GSC_SC_ADDR, GSC_SC_CTRL1, 1, &reg, 1)) {
		reg |= (1 << GSC_SC_CTRL1_WDDIS);
		if (!gsc_i2c_write(GSC_SC_ADDR, GSC_SC_CTRL1, 1, &reg, 1))
			return 0;
	}
	puts("Error: could not disable GSC Watchdog\n");
	return 1;
}

#if defined(CONFIG_CMD_GSC) && !defined(CONFIG_SPL_BUILD)
static int do_gsc_sleep(cmd_tbl_t *cmdtp, int flag, int argc,
			char * const argv[])
{
	unsigned char reg;
	unsigned long secs = 0;

	if (argc < 2)
		return CMD_RET_USAGE;

	secs = simple_strtoul(argv[1], NULL, 10);
	printf("GSC Sleeping for %ld seconds\n", secs);

	i2c_set_bus_num(0);
	reg = (secs >> 24) & 0xff;
	if (gsc_i2c_write(GSC_SC_ADDR, 9, 1, &reg, 1))
		goto error;
	reg = (secs >> 16) & 0xff;
	if (gsc_i2c_write(GSC_SC_ADDR, 8, 1, &reg, 1))
		goto error;
	reg = (secs >> 8) & 0xff;
	if (gsc_i2c_write(GSC_SC_ADDR, 7, 1, &reg, 1))
		goto error;
	reg = secs & 0xff;
	if (gsc_i2c_write(GSC_SC_ADDR, 6, 1, &reg, 1))
		goto error;
	if (gsc_i2c_read(GSC_SC_ADDR, GSC_SC_CTRL1, 1, &reg, 1))
		goto error;
	reg |= (1 << 2);
	if (gsc_i2c_write(GSC_SC_ADDR, GSC_SC_CTRL1, 1, &reg, 1))
		goto error;
	reg &= ~(1 << 2);
	reg |= 0x3;
	if (gsc_i2c_write(GSC_SC_ADDR, GSC_SC_CTRL1, 1, &reg, 1))
		goto error;

	return CMD_RET_SUCCESS;

error:
	printf("i2c error\n");
	return CMD_RET_FAILURE;
}

static int do_gsc_wd(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	unsigned char reg;

	if (argc < 2)
		return CMD_RET_USAGE;

	if (strcasecmp(argv[1], "enable") == 0) {
		int timeout = 0;

		if (argc > 2)
			timeout = simple_strtoul(argv[2], NULL, 10);
		i2c_set_bus_num(0);
		if (gsc_i2c_read(GSC_SC_ADDR, GSC_SC_CTRL1, 1, &reg, 1))
			return CMD_RET_FAILURE;
		reg &= ~((1 << GSC_SC_CTRL1_WDEN) | (1 << GSC_SC_CTRL1_WDTIME));
		if (timeout == 60)
			reg |= (1 << GSC_SC_CTRL1_WDTIME);
		else
			timeout = 30;
		reg |= (1 << GSC_SC_CTRL1_WDEN);
		if (gsc_i2c_write(GSC_SC_ADDR, GSC_SC_CTRL1, 1, &reg, 1))
			return CMD_RET_FAILURE;
		printf("GSC Watchdog enabled with timeout=%d seconds\n",
		       timeout);
	} else if (strcasecmp(argv[1], "disable") == 0) {
		i2c_set_bus_num(0);
		if (gsc_i2c_read(GSC_SC_ADDR, GSC_SC_CTRL1, 1, &reg, 1))
			return CMD_RET_FAILURE;
		reg &= ~((1 << GSC_SC_CTRL1_WDEN) | (1 << GSC_SC_CTRL1_WDTIME));
		if (gsc_i2c_write(GSC_SC_ADDR, GSC_SC_CTRL1, 1, &reg, 1))
			return CMD_RET_FAILURE;
		printf("GSC Watchdog disabled\n");
	} else {
		return CMD_RET_USAGE;
	}
	return CMD_RET_SUCCESS;
}

static int do_gsc(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	if (argc < 2)
		return gsc_info(1);

	if (strcasecmp(argv[1], "wd") == 0)
		return do_gsc_wd(cmdtp, flag, --argc, ++argv);
	else if (strcasecmp(argv[1], "sleep") == 0)
		return do_gsc_sleep(cmdtp, flag, --argc, ++argv);

	return CMD_RET_USAGE;
}

U_BOOT_CMD(
	gsc, 4, 1, do_gsc, "GSC configuration",
	"[wd enable [30|60]]|[wd disable]|[sleep <secs>]\n"
	);

#endif /* CONFIG_CMD_GSC */
