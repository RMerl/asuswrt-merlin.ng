// SPDX-License-Identifier: GPL-2.0-only
/*
 * Analog Devices MAX313XX series I2C RTC driver
 *
 * Copyright 2022 Analog Devices Inc.
 */
#include <bcd.h>
#include <dm.h>
#include <i2c.h>
#include <rtc.h>
#include <dm/device_compat.h>
#include <linux/bitfield.h>
#include <linux/delay.h>
#include <linux/kernel.h>

/* common registers */
#define MAX313XX_INT_ALARM1		BIT(0)
#define MAX313XX_INT_ALARM2		BIT(1)
#define MAX313XX_HRS_F_12_24		BIT(6)
#define MAX313XX_HRS_F_AM_PM		BIT(5)
#define MAX313XX_MONTH_CENTURY		BIT(7)

#define MAX313XX_TMR_CFG_ENABLE		BIT(4)
#define MAX313XX_TMR_CFG_FREQ_MASK	GENMASK(1, 0)
#define MAX313XX_TMR_CFG_FREQ_16HZ	0x03

#define MAX313XX_REG_MINUTE		0x01
#define MAX313XX_REG_HOUR		0x02

#define MAX313XX_TIME_SIZE		0x07

/* device specific registers */
#define MAX3134X_CFG2_REG		0x01
#define MAX3134X_CFG2_SET_RTC		BIT(1)

#define MAX31341_TRICKLE_RES_MASK	GENMASK(1, 0)
#define MAX31341_TRICKLE_DIODE_EN	BIT(2)
#define MAX31341_TRICKLE_ENABLE_BIT	BIT(3)
#define MAX31341_POWER_MGMT_REG		0x56
#define MAX31341_POWER_MGMT_TRICKLE_BIT	BIT(0)

#define MAX3133X_TRICKLE_RES_MASK	GENMASK(2, 1)
#define MAX3133X_TRICKLE_DIODE_EN	BIT(3)
#define MAX3133X_TRICKLE_ENABLE_BIT	BIT(0)

#define MAX31329_TRICKLE_ENABLE_BIT	BIT(7)
#define MAX31343_TRICKLE_ENABLE_MASK	GENMASK(7, 4)
#define MAX31343_TRICKLE_ENABLE_CODE	5
#define MAX31329_43_TRICKLE_RES_MASK	GENMASK(1, 0)
#define MAX31329_43_TRICKLE_DIODE_EN	BIT(2)

#define MAX31329_CONFIG2_REG		0x04
#define MAX31329_CONFIG2_CLKIN_EN	BIT(2)
#define MAX31329_CONFIG2_CLKIN_FREQ	GENMASK(1, 0)

#define MAX31341_42_CONFIG1_REG		0x00
#define MAX31341_42_CONFIG1_CLKIN_EN	BIT(7)
#define MAX31341_42_CONFIG1_CLKIN_FREQ	GENMASK(5, 4)
#define MAX31341_42_CONFIG1_OSC_DISABLE	BIT(3)
#define MAX31341_42_CONFIG1_SWRST	BIT(0)

enum max313xx_ids {
	ID_MAX31328,
	ID_MAX31329,
	ID_MAX31331,
	ID_MAX31334,
	ID_MAX31341,
	ID_MAX31342,
	ID_MAX31343,
	MAX313XX_ID_NR
};

/**
 * struct chip_desc - descriptor for MAX313xx variants
 * @sec_reg: Offset to seconds register. Used to denote the start of the
 *           current time registers.
 * @alarm1_sec_reg: Offset to Alarm1 seconds register. Used to denote the
 *                  start of the alarm registers.
 * @int_en_reg: Offset to the interrupt enable register.
 * @int_status_reg: Offset to the interrupt status register.
 * @ram_reg: Offset to the timestamp RAM (which can be used as SRAM).
 * @ram_size: Size of the timestamp RAM.
 * @temp_reg: Offset to the temperature register (or 0 if temperature
 *            sensor is not supported).
 * @trickle_reg: Offset to the trickle charger configuration register (or
 *                0 if trickle charger is not supported).
 * @rst_reg: Offset to the reset register.
 * @rst_bit: Bit within the reset register for the software reset.
 */
struct chip_desc {
	u8 sec_reg;
	u8 alarm1_sec_reg;

	u8 int_en_reg;
	u8 int_status_reg;

	u8 ram_reg;
	u8 ram_size;

	u8 temp_reg;

	u8 trickle_reg;

	u8 rst_reg;
	u8 rst_bit;
};

struct max313xx_priv {
	enum max313xx_ids id;
	const struct chip_desc *chip;
};

static const struct chip_desc chip[MAX313XX_ID_NR] = {
	[ID_MAX31328] = {
		.int_en_reg = 0x0E,
		.int_status_reg = 0x0F,
		.sec_reg = 0x00,
		.alarm1_sec_reg = 0x07,
		.temp_reg = 0x11,
	},
	[ID_MAX31329] = {
		.int_en_reg = 0x01,
		.int_status_reg = 0x00,
		.sec_reg = 0x06,
		.alarm1_sec_reg = 0x0D,
		.ram_reg = 0x22,
		.ram_size = 64,
		.trickle_reg = 0x19,
		.rst_reg = 0x02,
		.rst_bit = BIT(0),
	},
	[ID_MAX31331] = {
		.int_en_reg = 0x01,
		.int_status_reg = 0x00,
		.sec_reg = 0x08,
		.alarm1_sec_reg = 0x0F,
		.ram_reg = 0x20,
		.ram_size = 32,
		.trickle_reg = 0x1B,
		.rst_reg = 0x02,
		.rst_bit = BIT(0),
	},
	[ID_MAX31334] = {
		.int_en_reg = 0x01,
		.int_status_reg = 0x00,
		.sec_reg = 0x09,
		.alarm1_sec_reg = 0x10,
		.ram_reg = 0x30,
		.ram_size = 32,
		.trickle_reg = 0x1E,
		.rst_reg = 0x02,
		.rst_bit = BIT(0),
	},
	[ID_MAX31341] = {
		.int_en_reg = 0x04,
		.int_status_reg = 0x05,
		.sec_reg = 0x06,
		.alarm1_sec_reg = 0x0D,
		.ram_reg = 0x16,
		.ram_size = 64,
		.trickle_reg = 0x57,
		.rst_reg = 0x00,
		.rst_bit = BIT(0),
	},
	[ID_MAX31342] = {
		.int_en_reg = 0x04,
		.int_status_reg = 0x05,
		.sec_reg = 0x06,
		.alarm1_sec_reg = 0x0D,
		.rst_reg = 0x00,
		.rst_bit = BIT(0),
	},
	[ID_MAX31343] = {
		.int_en_reg = 0x01,
		.int_status_reg = 0x00,
		.sec_reg = 0x06,
		.alarm1_sec_reg = 0x0D,
		.ram_reg = 0x22,
		.ram_size = 64,
		.temp_reg = 0x1A,
		.trickle_reg = 0x19,
		.rst_reg = 0x02,
		.rst_bit = BIT(0),
	},
};

static const u32 max313xx_trickle_ohms[] = { 3000, 6000, 11000 };

static int max313xx_set_bits(struct udevice *dev, unsigned int reg, unsigned int bits)
{
	int ret;

	ret = dm_i2c_reg_read(dev, reg);
	if (ret < 0)
		return ret;

	return dm_i2c_reg_write(dev, reg, ret | bits);
}

static int max313xx_clear_bits(struct udevice *dev, unsigned int reg, unsigned int bits)
{
	int ret;

	ret = dm_i2c_reg_read(dev, reg);
	if (ret < 0)
		return ret;

	return dm_i2c_reg_write(dev, reg, ret & ~bits);
}

static int max313xx_get_hour(u8 hour_reg)
{
	int hour;

	/* 24Hr mode */
	if (!FIELD_GET(MAX313XX_HRS_F_12_24, hour_reg))
		return bcd2bin(hour_reg & 0x3f);

	/* 12Hr mode */
	hour = bcd2bin(hour_reg & 0x1f);
	if (hour == 12)
		hour = 0;

	if (FIELD_GET(MAX313XX_HRS_F_AM_PM, hour_reg))
		hour += 12;

	return hour;
}

static int max313xx_read_time(struct udevice *dev, struct rtc_time *t)
{
	struct max313xx_priv *rtc = dev_get_priv(dev);
	u8 regs[7];
	int ret;

	ret = dm_i2c_read(dev, rtc->chip->sec_reg, regs, 7);
	if (ret)
		return ret;

	t->tm_sec = bcd2bin(regs[0] & 0x7f);
	t->tm_min = bcd2bin(regs[1] & 0x7f);
	t->tm_hour = max313xx_get_hour(regs[2]);
	t->tm_wday = bcd2bin(regs[3] & 0x07) - 1;
	t->tm_mday = bcd2bin(regs[4] & 0x3f);
	t->tm_mon = bcd2bin(regs[5] & 0x1f);
	t->tm_year = bcd2bin(regs[6]) + 2000;

	if (FIELD_GET(MAX313XX_MONTH_CENTURY, regs[5]))
		t->tm_year += 100;

	dev_dbg(dev, "read %4d-%02d-%02d (wday=%d) %2d:%02d:%02d\n",
		t->tm_year, t->tm_mon, t->tm_mday,
		t->tm_wday, t->tm_hour, t->tm_min, t->tm_sec);

	return 0;
}

static int max313xx_set_time(struct udevice *dev, const struct rtc_time *t)
{
	struct max313xx_priv *rtc = dev_get_priv(dev);
	u8 regs[7];
	int ret;

	dev_dbg(dev, "set %4d-%02d-%02d (wday=%d) %2d:%02d:%02d\n",
		t->tm_year, t->tm_mon, t->tm_mday,
		t->tm_wday, t->tm_hour, t->tm_min, t->tm_sec);

	if (t->tm_year < 2000) {
		dev_err(dev, "year %d (before 2000) not supported\n",
			t->tm_year);
		return -EINVAL;
	}

	if (rtc->chip->rst_bit) {
		ret = max313xx_clear_bits(dev, rtc->chip->rst_reg, rtc->chip->rst_bit);
		if (ret)
			return ret;
	}

	regs[0] = bin2bcd(t->tm_sec);
	regs[1] = bin2bcd(t->tm_min);
	regs[2] = bin2bcd(t->tm_hour);
	regs[3] = bin2bcd(t->tm_wday + 1);
	regs[4] = bin2bcd(t->tm_mday);
	regs[5] = bin2bcd(t->tm_mon);
	regs[6] = bin2bcd((t->tm_year - 2000) % 100);

	if ((t->tm_year - 2000) >= 200)
		regs[5] |= FIELD_PREP(MAX313XX_MONTH_CENTURY, 1);

	ret = dm_i2c_write(dev, rtc->chip->sec_reg, regs, 7);
	if (ret)
		return ret;

	switch (rtc->id) {
	case ID_MAX31341:
	case ID_MAX31342:
		ret = max313xx_set_bits(dev, MAX3134X_CFG2_REG,
					MAX3134X_CFG2_SET_RTC);
		if (ret)
			return ret;

		udelay(10000);

		ret = max313xx_clear_bits(dev, MAX3134X_CFG2_REG,
					  MAX3134X_CFG2_SET_RTC);
		if (ret)
			return ret;

		break;
	default:
		break;
	}

	return ret;
}

static int max313xx_reset(struct udevice *dev)
{
	struct max313xx_priv *rtc = dev_get_priv(dev);
	int ret = -EINVAL;

	if (rtc->chip->rst_bit)
		ret = max313xx_set_bits(dev, rtc->chip->rst_reg, rtc->chip->rst_bit);

	return ret;
}

static int max313xx_read8(struct udevice *dev, unsigned int reg)
{
	return  dm_i2c_reg_read(dev, reg);
}

static int max313xx_write8(struct udevice *dev, unsigned int reg, int val)
{
	return dm_i2c_reg_write(dev, reg, val);
}

static const struct rtc_ops max3133x_rtc_ops = {
	.get	= max313xx_read_time,
	.set	= max313xx_set_time,
	.reset  = max313xx_reset,
	.read8	= max313xx_read8,
	.write8	= max313xx_write8,
};

static int max313xx_init(struct udevice *dev)
{
	struct max313xx_priv *rtc = dev_get_priv(dev);
	int ret;

	switch (rtc->id) {
	case ID_MAX31341:
	case ID_MAX31342:
		ret = max313xx_clear_bits(dev, MAX31341_42_CONFIG1_REG,
					  MAX31341_42_CONFIG1_OSC_DISABLE);
		if (ret)
			return ret;

		return max313xx_set_bits(dev, MAX31341_42_CONFIG1_REG,
				       MAX31341_42_CONFIG1_SWRST);
	default:
		return 0;
	}
}

static int max313xx_trickle_charger_setup(struct udevice *dev)
{
	struct max313xx_priv *rtc = dev_get_priv(dev);
	bool diode;
	int index, reg;
	u32 ohms;
	u32 chargeable;
	int ret;

	if (dev_read_u32(dev, "trickle-resistor-ohms", &ohms) ||
	    dev_read_u32(dev, "aux-voltage-chargeable", &chargeable))
		return 0;

	switch (chargeable) {
	case 0:
		diode = false;
		break;
	case 1:
		diode = true;
		break;
	default:
		dev_dbg(dev, "unsupported aux-voltage-chargeable value\n");
		return -EINVAL;
	}

	if (!rtc->chip->trickle_reg) {
		dev_warn(dev, "device does not have trickle charger\n");
		return -ENOTSUPP;
	}

	index = find_closest(ohms, max313xx_trickle_ohms,
			     ARRAY_SIZE(max313xx_trickle_ohms)) + 1;

	switch (rtc->id) {
	case ID_MAX31329:
		reg = FIELD_PREP(MAX31329_TRICKLE_ENABLE_BIT, 1) |
		      FIELD_PREP(MAX31329_43_TRICKLE_RES_MASK, index) |
		      FIELD_PREP(MAX31329_43_TRICKLE_DIODE_EN, diode);
		break;
	case ID_MAX31331:
	case ID_MAX31334:
		reg = FIELD_PREP(MAX3133X_TRICKLE_ENABLE_BIT, 1) |
		      FIELD_PREP(MAX3133X_TRICKLE_DIODE_EN, diode) |
		      FIELD_PREP(MAX3133X_TRICKLE_RES_MASK, index);
		break;
	case ID_MAX31341:
		if (index == 1)
			index = 0;
		reg = FIELD_PREP(MAX31341_TRICKLE_ENABLE_BIT, 1) |
		      FIELD_PREP(MAX31341_TRICKLE_DIODE_EN, diode) |
		      FIELD_PREP(MAX31341_TRICKLE_RES_MASK, index);

		ret = max313xx_set_bits(dev, MAX31341_POWER_MGMT_REG,
					MAX31341_POWER_MGMT_TRICKLE_BIT);
		if (ret)
			return ret;

		break;
	case ID_MAX31343:
		reg = FIELD_PREP(MAX31329_43_TRICKLE_RES_MASK, index) |
		      FIELD_PREP(MAX31329_43_TRICKLE_DIODE_EN, diode) |
		      FIELD_PREP(MAX31343_TRICKLE_ENABLE_MASK,
				 MAX31343_TRICKLE_ENABLE_CODE);
		break;
	default:
		return -EOPNOTSUPP;
	}

	return dm_i2c_reg_write(dev, rtc->chip->trickle_reg, reg);
}

static int max313xx_probe(struct udevice *dev)
{
	struct max313xx_priv *max313xx = dev_get_priv(dev);
	int ret;

	max313xx->id = dev_get_driver_data(dev);
	max313xx->chip = &chip[max313xx->id];

	ret = max313xx_init(dev);
	if (ret)
		return ret;

	return max313xx_trickle_charger_setup(dev);
}

static const struct udevice_id max313xx_of_id[] = {
	{ .compatible = "adi,max31328", .data = ID_MAX31328 },
	{ .compatible = "adi,max31329", .data = ID_MAX31329 },
	{ .compatible = "adi,max31331", .data = ID_MAX31331 },
	{ .compatible = "adi,max31334", .data = ID_MAX31334 },
	{ .compatible = "adi,max31341", .data = ID_MAX31341 },
	{ .compatible = "adi,max31342", .data = ID_MAX31342 },
	{ .compatible = "adi,max31343", .data = ID_MAX31343 },
	{ }
};

U_BOOT_DRIVER(rtc_max313xx) = {
	.name	= "rtc-max313xx",
	.id     = UCLASS_RTC,
	.probe  = max313xx_probe,
	.of_match = max313xx_of_id,
	.priv_auto = sizeof(struct max313xx_priv),
	.ops    = &max3133x_rtc_ops,
};
