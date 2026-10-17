// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2022 Gateworks Corporation
 */

#include <command.h>
#include <gsc.h>
#include <i2c.h>
#include <rtc.h>
#include <asm/unaligned.h>
#include <linux/delay.h>
#include <dm/device.h>
#include <dm/device-internal.h>
#include <dm/ofnode.h>
#include <dm/read.h>

#define GSC_BUSNO	0
#define GSC_SC_ADDR	0x20
#define GSC_HWMON_ADDR	0x29
#define GSC_RTC_ADDR	0x68

/* System Controller registers */
enum {
	GSC_SC_CTRL0		= 0,
	GSC_SC_CTRL1		= 1,
	GSC_SC_TIME		= 2,
	GSC_SC_TIME_ADD		= 6,
	GSC_SC_STATUS		= 10,
	GSC_SC_FWCRC		= 12,
	GSC_SC_FWVER		= 14,
	GSC_SC_WP		= 15,
	GSC_SC_RST_CAUSE	= 16,
	GSC_SC_THERM_PROTECT	= 19,
};

/* System Controller Control1 bits */
enum {
	GSC_SC_CTRL1_SLEEP_EN		= 0, /* 1 = enable sleep */
	GSC_SC_CTRL1_SLEEP_ACTIVATE	= 1, /* 1 = activate sleep */
	GSC_SC_CTRL1_SLEEP_ADD		= 2, /* 1 = latch and add sleep time */
	GSC_SC_CTRL1_SLEEP_NOWAKEPB	= 3, /* 1 = do not wake on sleep on button press */
	GSC_SC_CTRL1_WDTIME		= 4, /* 1 = 60s timeout, 0 = 30s timeout */
	GSC_SC_CTRL1_WDEN		= 5, /* 1 = enable, 0 = disable */
	GSC_SC_CTRL1_BOOT_CHK		= 6, /* 1 = enable alt boot check */
	GSC_SC_CTRL1_WDDIS		= 7, /* 1 = disable boot watchdog */
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

/* System Controller WP bits */
enum {
	GSC_SC_WP_ALL		= 0, /* Write Protect All EEPROM regions */
	GSC_SC_WP_BOARDINFO	= 1, /* Write Protect Board Info region */
};

/* System Controller Reset Cause */
enum {
	GSC_SC_RST_CAUSE_VIN		= 0,
	GSC_SC_RST_CAUSE_PB		= 1,
	GSC_SC_RST_CAUSE_WDT		= 2,
	GSC_SC_RST_CAUSE_CPU		= 3,
	GSC_SC_RST_CAUSE_TEMP_LOCAL	= 4,
	GSC_SC_RST_CAUSE_TEMP_REMOTE	= 5,
	GSC_SC_RST_CAUSE_SLEEP		= 6,
	GSC_SC_RST_CAUSE_BOOT_WDT	= 7,
	GSC_SC_RST_CAUSE_BOOT_WDT_MAN	= 8,
	GSC_SC_RST_CAUSE_SOFT_PWR	= 9,
	GSC_SC_RST_CAUSE_MAX		= 10,
};

#if CONFIG_IS_ENABLED(DM_I2C)

struct gsc_priv {
	int gscver;
	int fwver;
	int fwcrc;
	struct udevice *hwmon;
	struct udevice *rtc;
};

/*
 * GSCv2 will fail to ACK an I2C transaction if it is busy, which can occur
 * during its 1HZ timer tick while reading ADC's. When this does occur,
 * it will never be busy longer than 2 back-to-back transfers so retry 3 times.
 */
static int gsc_i2c_read(struct udevice *dev, uint addr, u8 *buf, int len)
{
	struct gsc_priv *priv = dev_get_priv(dev);
	int retry = (priv->gscver == 3) ? 1 : 3;
	int n = 0;
	int ret;

	while (n++ < retry) {
		ret = dm_i2c_read(dev, addr, buf, len);
		if (!ret)
			break;
		if (ret != -EREMOTEIO)
			break;
		mdelay(10);
	}
	return ret;
}

static int gsc_i2c_write(struct udevice *dev, uint addr, const u8 *buf, int len)
{
	struct gsc_priv *priv = dev_get_priv(dev);
	int retry = (priv->gscver == 3) ? 1 : 3;
	int n = 0;
	int ret;

	while (n++ < retry) {
		ret = dm_i2c_write(dev, addr, buf, len);
		if (!ret)
			break;
		if (ret != -EREMOTEIO)
			break;
		mdelay(10);
	}
	return ret;
}

static struct udevice *gsc_get_dev(int busno, int slave)
{
	struct udevice *dev, *bus;
	int ret;

	ret = uclass_get_device_by_seq(UCLASS_I2C, busno, &bus);
	if (ret)
		return NULL;
	ret = dm_i2c_probe(bus, slave, 0, &dev);
	if (ret)
		return NULL;

	return dev;
}

static int gsc_thermal_get_info(struct udevice *dev, u8 *outreg, int *tmax, bool *enable)
{
	struct gsc_priv *priv = dev_get_priv(dev);
	int ret;
	u8 reg;

	if (priv->gscver > 2 && priv->fwver > 52) {
		ret = gsc_i2c_read(dev, GSC_SC_THERM_PROTECT, &reg, 1);
		if (!ret) {
			if (outreg)
				*outreg = reg;
			if (tmax) {
				*tmax = ((reg & 0xf8) >> 3) * 2;
				if (*tmax)
					*tmax += 70;
				else
					*tmax = 120;
			}
			if (enable)
				*enable = reg & 1;
		}
	} else {
		ret = -ENODEV;
	}

	return ret;
}

static int gsc_thermal_get_temp(struct udevice *dev)
{
	struct gsc_priv *priv = dev_get_priv(dev);
	u32 reg, mode, val;
	const char *label;
	ofnode node;
	u8 buf[2];

	ofnode_for_each_subnode(node, dev_read_subnode(dev, "adc")) {
		if (ofnode_read_u32(node, "reg", &reg))
			reg = -1;
		if (ofnode_read_u32(node, "gw,mode", &mode))
			mode = -1;
		label = ofnode_read_string(node, "label");

		if ((reg == -1) || (mode == -1) || !label)
			continue;

		if (mode != 0 || strcmp(label, "temp"))
			continue;

		memset(buf, 0, sizeof(buf));
		if (!gsc_i2c_read(priv->hwmon, reg, buf, sizeof(buf))) {
			val = buf[0] | buf[1] << 8;
			if (val > 0x8000)
				val -= 0xffff;
			return val;
		}
	}

	return 0;
}

static void gsc_thermal_info(struct udevice *dev)
{
	struct gsc_priv *priv = dev_get_priv(dev);

	switch (priv->gscver) {
	case 2:
		printf("board_temp:%dC ", gsc_thermal_get_temp(dev) / 10);
		break;
	case 3:
		if (priv->fwver > 52) {
			bool enabled;
			int tmax;

			if (!gsc_thermal_get_info(dev, NULL, &tmax, &enabled)) {
				puts("Thermal protection:");
				if (enabled)
					printf("enabled at %dC ", tmax);
				else
					puts("disabled ");
			}
		}
		break;
	}
}

static void gsc_reset_info(struct udevice *dev)
{
	struct gsc_priv *priv = dev_get_priv(dev);
	static const char * const names[] = {
		"VIN",
		"PB",
		"WDT",
		"CPU",
		"TEMP_L",
		"TEMP_R",
		"SLEEP",
		"BOOT_WDT1",
		"BOOT_WDT2",
		"SOFT_PWR",
	};
	u8 reg;

	/* reset cause */
	switch (priv->gscver) {
	case 2:
		if (!gsc_i2c_read(dev, GSC_SC_STATUS, &reg, 1)) {
			if (reg & BIT(GSC_SC_IRQ_WATCHDOG)) {
				puts("RST:WDT");
				reg &= ~BIT(GSC_SC_IRQ_WATCHDOG);
				gsc_i2c_write(dev, GSC_SC_STATUS, &reg, 1);
			} else {
				puts("RST:VIN");
			}
			printf(" WDT:%sabled ",
			       (reg & BIT(GSC_SC_CTRL1_WDEN)) ? "en" : "dis");
		}
		break;
	case 3:
		if (priv->fwver > 52 &&
		    !gsc_i2c_read(dev, GSC_SC_RST_CAUSE, &reg, 1)) {
			puts("RST:");
			if (reg < ARRAY_SIZE(names))
				printf("%s ", names[reg]);
			else
				printf("0x%02x ", reg);
		}
		break;
	}
}

/* display hardware monitor ADC channels */
static int gsc_hwmon(struct udevice *dev)
{
	struct gsc_priv *priv = dev_get_priv(dev);
	u32 reg, mode, val, offset;
	const char *label;
	ofnode node;
	u8 buf[2];
	u32 r[2];
	int ret;

	/* iterate over hwmon nodes */
	ofnode_for_each_subnode(node, dev_read_subnode(dev, "adc")) {
		if (ofnode_read_u32(node, "reg", &reg))
			reg = -1;
		if (ofnode_read_u32(node, "gw,mode", &mode))
			mode = -1;
		label = ofnode_read_string(node, "label");
		if ((reg == -1) || (mode == -1) || !label)
			continue;

		memset(buf, 0, sizeof(buf));
		ret = gsc_i2c_read(priv->hwmon, reg, buf, sizeof(buf));
		if (ret) {
			printf("i2c error: %d\n", ret);
			continue;
		}
		val = buf[0] | buf[1] << 8;

		switch (mode) {
		case 0: /* temperature (C*10) */
			if (val > 0x8000)
				val -= 0xffff;
			printf("%-8s: %d.%ldC\n", label, val / 10, abs(val % 10));
			break;
		case 1: /* prescaled voltage */
			if (val != 0xffff)
				printf("%-8s: %d.%03dV\n", label, val / 1000, val % 1000);
			break;
		case 2: /* scaled based on ref volt and resolution */
			val *= 2500;
			val /= 1 << 12;

			/* apply pre-scaler voltage divider */
			if (!ofnode_read_u32_index(node, "gw,voltage-divider-ohms", 0, &r[0]) &&
			    !ofnode_read_u32_index(node, "gw,voltage-divider-ohms", 1, &r[1]) &&
			    r[0] && r[1]) {
				val *= (r[0] + r[1]);
				val /= r[1];
			}

			/* adjust by offset */
			val += (offset / 1000);

			printf("%-8s: %d.%03dV\n", label, val / 1000, val % 1000);
			break;
		}
	}

	return 0;
}

static int gsc_banner(struct udevice *dev)
{
	struct gsc_priv *priv = dev_get_priv(dev);

	/* banner */
	printf("GSCv%d   : v%d 0x%04x ", priv->gscver, priv->fwver, priv->fwcrc);
	gsc_reset_info(dev);
	gsc_thermal_info(dev);
	puts("\n");

	/* Display RTC */
	if (priv->rtc) {
		u8 buf[4];
		time_t timestamp;
		struct rtc_time tm;

		if (!gsc_i2c_read(priv->rtc, 0, buf, 4)) {
			timestamp = get_unaligned_le32(buf);
			rtc_to_tm(timestamp, &tm);
			printf("RTC     : %4d-%02d-%02d  %2d:%02d:%02d UTC\n",
			       tm.tm_year, tm.tm_mon, tm.tm_mday,
			       tm.tm_hour, tm.tm_min, tm.tm_sec);
		}
	}

	return 0;
}

static int gsc_probe(struct udevice *dev)
{
	struct gsc_priv *priv = dev_get_priv(dev);
	u8 buf[32];
	int ret;

	ret = gsc_i2c_read(dev, 0, buf, sizeof(buf));
	if (ret)
		return ret;

	/*
	 * GSC chip version:
	 *   GSCv2 has 16 registers (which overlap)
	 *   GSCv3 has 32 registers
	 */
	priv->gscver = memcmp(buf, buf + 16, 16) ? 3 : 2;
	priv->fwver = buf[GSC_SC_FWVER];
	priv->fwcrc = buf[GSC_SC_FWCRC] | buf[GSC_SC_FWCRC + 1] << 8;
	priv->hwmon = gsc_get_dev(GSC_BUSNO, GSC_HWMON_ADDR);
	if (priv->hwmon)
		dev_set_priv(priv->hwmon, priv);
	priv->rtc = gsc_get_dev(GSC_BUSNO, GSC_RTC_ADDR);
	if (priv->rtc)
		dev_set_priv(priv->rtc, priv);

#ifdef CONFIG_SPL_BUILD
	gsc_banner(dev);
#endif

	return 0;
};

static const struct udevice_id gsc_ids[] = {
	{ .compatible = "gw,gsc", },
	{ }
};

U_BOOT_DRIVER(gsc) = {
	.name = "gsc",
	.id = UCLASS_MISC,
	.of_match = gsc_ids,
	.probe = gsc_probe,
	.priv_auto      = sizeof(struct gsc_priv),
	.flags = DM_FLAG_PRE_RELOC,
};

static int gsc_sleep(struct udevice *dev, unsigned long secs)
{
	u8 regs[4];
	int ret;

	printf("GSC Sleeping for %ld seconds\n", secs);
	put_unaligned_le32(secs, regs);
	ret = gsc_i2c_write(dev, GSC_SC_TIME_ADD, regs, sizeof(regs));
	if (ret)
		goto err;
	ret = gsc_i2c_read(dev, GSC_SC_CTRL1, regs, 1);
	if (ret)
		goto err;
	regs[0] |= BIT(GSC_SC_CTRL1_SLEEP_ADD);
	ret = gsc_i2c_write(dev, GSC_SC_CTRL1, regs, 1);
	if (ret)
		goto err;
	regs[0] &= ~BIT(GSC_SC_CTRL1_SLEEP_ADD);
	regs[0] |= BIT(GSC_SC_CTRL1_SLEEP_EN) | BIT(GSC_SC_CTRL1_SLEEP_ACTIVATE);
	ret = gsc_i2c_write(dev, GSC_SC_CTRL1, regs, 1);
	if (ret)
		goto err;

	return 0;

err:
	printf("i2c error: %d\n", ret);
	return ret;
}

static int gsc_wd_disable(struct udevice *dev)
{
	int ret;
	u8 reg;

	ret = gsc_i2c_read(dev, GSC_SC_CTRL1, &reg, 1);
	if (ret)
		goto err;
	reg |= BIT(GSC_SC_CTRL1_WDDIS);
	reg &= ~BIT(GSC_SC_CTRL1_BOOT_CHK);
	ret = gsc_i2c_write(dev, GSC_SC_CTRL1, &reg, 1);
	if (ret)
		goto err;
	puts("GSC     : boot watchdog disabled\n");

	return 0;

err:
	puts("i2c error");
	return ret;
}

static int gsc_thermal(struct udevice *dev, const char *cmd, const char *val)
{
	struct gsc_priv *priv = dev_get_priv(dev);
	int ret, tmax;
	bool enabled;
	u8 reg;

	if (priv->gscver < 3 || priv->fwver < 53)
		return -EINVAL;
	ret = gsc_thermal_get_info(dev, &reg, &tmax, &enabled);
	if (ret)
		return ret;
	if (cmd && !strcmp(cmd, "enable")) {
		if (val && *val) {
			tmax = clamp((int)simple_strtoul(val, NULL, 0), 72, 122);
			reg &= ~0xf8;
			reg |= ((tmax - 70) / 2) << 3;
		}
		reg |= BIT(0);
		gsc_i2c_write(dev, GSC_SC_THERM_PROTECT, &reg, 1);
	} else if (cmd && !strcmp(cmd, "disable")) {
		reg &= ~BIT(0);
		gsc_i2c_write(dev, GSC_SC_THERM_PROTECT, &reg, 1);
	} else if (cmd) {
		return -EINVAL;
	}

	/* show status */
	gsc_thermal_info(dev);
	puts("\n");

	return 0;
}

/* override in board files to display additional board EEPROM info */
__weak void board_gsc_info(void)
{
}

static void gsc_info(struct udevice *dev)
{
	gsc_banner(dev);
	board_gsc_info();
}

static int do_gsc(struct cmd_tbl *cmdtp, int flag, int argc, char * const argv[])
{
	struct udevice *dev;
	int ret;

	/* get/probe driver */
	ret = uclass_get_device_by_driver(UCLASS_MISC, DM_DRIVER_GET(gsc), &dev);
	if (ret)
		return CMD_RET_USAGE;
	if (argc < 2) {
		gsc_info(dev);
		return CMD_RET_SUCCESS;
	} else if (strcasecmp(argv[1], "sleep") == 0) {
		if (argc < 3)
			return CMD_RET_USAGE;
		if (!gsc_sleep(dev, dectoul(argv[2], NULL)))
			return CMD_RET_SUCCESS;
	} else if (strcasecmp(argv[1], "hwmon") == 0) {
		if (!gsc_hwmon(dev))
			return CMD_RET_SUCCESS;
	} else if (strcasecmp(argv[1], "wd-disable") == 0) {
		if (!gsc_wd_disable(dev))
			return CMD_RET_SUCCESS;
	} else if (strcasecmp(argv[1], "thermal") == 0) {
		const char *cmd, *val;

		cmd = cmd_arg2(argc, argv);
		val = cmd_arg3(argc, argv);
		if (!gsc_thermal(dev, cmd, val))
			return CMD_RET_SUCCESS;
	}

	return CMD_RET_USAGE;
}

U_BOOT_CMD(gsc, 4, 1, do_gsc, "Gateworks System Controller",
	   "[sleep <secs>]|[hwmon]|[wd-disable][thermal [disable|enable [temp]]]\n");

/* disable boot watchdog - useful for an SPL that wants to use falcon mode */
int gsc_boot_wd_disable(void)
{
	struct udevice *dev;
	int ret;

	/* get/probe driver */
	ret = uclass_get_device_by_driver(UCLASS_MISC, DM_DRIVER_GET(gsc), &dev);
	if (!ret)
		ret = gsc_wd_disable(dev);

	return ret;
}

# else

/*
 * GSCv2 will fail to ACK an I2C transaction if it is busy, which can occur
 * during its 1HZ timer tick while reading ADC's. When this does occur,
 * it will never be busy longer than 2 back-to-back transfers so retry 3 times.
 */
static int gsc_i2c_read(uint chip, uint addr, u8 *buf, int len)
{
	int retry = 3;
	int n = 0;
	int ret;

	while (n++ < retry) {
		ret = i2c_read(chip, addr, 1, buf, len);
		if (!ret)
			break;
		if (ret != -EREMOTEIO)
			break;
printf("%s 0x%02x retry %d\n", __func__, addr, n);
		mdelay(10);
	}
	return ret;
}

static int gsc_i2c_write(uint chip, uint addr, u8 *buf, int len)
{
	int retry = 3;
	int n = 0;
	int ret;

	while (n++ < retry) {
		ret = i2c_write(chip, addr, 1, buf, len);
		if (!ret)
			break;
		if (ret != -EREMOTEIO)
			break;
printf("%s 0x%02x retry %d\n", __func__, addr, n);
		mdelay(10);
	}
	return ret;
}

/* disable boot watchdog - useful for an SPL that wants to use falcon mode */
int gsc_boot_wd_disable(void)
{
	u8 buf[32];
	int ret;

	i2c_set_bus_num(GSC_BUSNO);
	ret = gsc_i2c_read(GSC_SC_ADDR, 0, buf, sizeof(buf));
	if (!ret) {
		buf[GSC_SC_CTRL1] |= BIT(GSC_SC_CTRL1_WDDIS);
		ret = gsc_i2c_write(GSC_SC_ADDR, GSC_SC_CTRL1, &buf[GSC_SC_CTRL1], 1);
		printf("GSCv%d: v%d 0x%04x ",
		       memcmp(buf, buf + 16, 16) ? 3 : 2,
		       buf[GSC_SC_FWVER],
		       buf[GSC_SC_FWCRC] | buf[GSC_SC_FWCRC + 1] << 8);
		if (buf[GSC_SC_STATUS] & BIT(GSC_SC_IRQ_WATCHDOG)) {
			puts("RST:WDT ");
			buf[GSC_SC_STATUS] &= ~BIT(GSC_SC_IRQ_WATCHDOG);
			gsc_i2c_write(GSC_SC_ADDR, GSC_SC_STATUS, &buf[GSC_SC_STATUS], 1);
		} else {
			puts("RST:VIN ");
		}
		puts("WDT:disabled\n");
	}

	return ret;
}

#endif
