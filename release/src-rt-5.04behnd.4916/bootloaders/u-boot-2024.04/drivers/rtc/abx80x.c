// SPDX-License-Identifier: GPL-2.0
/*
 * A driver for the I2C members of the Abracon AB x8xx RTC family,
 * and compatible: AB 1805 and AB 0805
 *
 * Copyright 2014-2015 Macq S.A.
 * Copyright 2020 Linaro
 *
 * Author: Philippe De Muyter <phdm@macqel.be>
 * Author: Alexandre Belloni <alexandre.belloni@bootlin.com>
 * Author: Ying-Chun Liu (PaulLiu) <paul.liu@linaro.org>
 *
 */

#include <common.h>
#include <dm.h>
#include <i2c.h>
#include <rtc.h>
#include <log.h>
#include <linux/bitfield.h>

#define ABX8XX_REG_HTH		0x00
#define ABX8XX_REG_SC		0x01
#define ABX8XX_REG_MN		0x02
#define ABX8XX_REG_HR		0x03
#define ABX8XX_REG_DA		0x04
#define ABX8XX_REG_MO		0x05
#define ABX8XX_REG_YR		0x06
#define ABX8XX_REG_WD		0x07

#define ABX8XX_REG_AHTH		0x08
#define ABX8XX_REG_ASC		0x09
#define ABX8XX_REG_AMN		0x0a
#define ABX8XX_REG_AHR		0x0b
#define ABX8XX_REG_ADA		0x0c
#define ABX8XX_REG_AMO		0x0d
#define ABX8XX_REG_AWD		0x0e

#define ABX8XX_REG_STATUS	0x0f
#define ABX8XX_STATUS_AF	BIT(2)
#define ABX8XX_STATUS_BLF	BIT(4)
#define ABX8XX_STATUS_WDT	BIT(6)

#define ABX8XX_REG_CTRL1	0x10
#define ABX8XX_CTRL_WRITE	BIT(0)
#define ABX8XX_CTRL_ARST	BIT(2)
#define ABX8XX_CTRL_12_24	BIT(6)

#define ABX8XX_REG_CTRL2	0x11
#define ABX8XX_CTRL2_RSVD	BIT(5)

#define ABX8XX_REG_IRQ		0x12
#define ABX8XX_IRQ_AIE		BIT(2)
#define ABX8XX_IRQ_IM_1_4	(0x3 << 5)

#define ABX8XX_REG_CD_TIMER_CTL	0x18

#define ABX8XX_REG_OSC		0x1c
#define ABX8XX_OSC_FOS		BIT(3)
#define ABX8XX_OSC_BOS		BIT(4)
#define ABX8XX_OSC_ACAL_512	BIT(5)
#define ABX8XX_OSC_ACAL_1024	BIT(6)

#define ABX8XX_OSC_OSEL		BIT(7)

#define ABX8XX_REG_OSS		0x1d
#define ABX8XX_OSS_OF		BIT(1)
#define ABX8XX_OSS_OMODE	BIT(4)

#define ABX8XX_REG_WDT		0x1b
#define ABX8XX_WDT_WDS		BIT(7)
#define ABX8XX_WDT_BMB_MASK	0x7c
#define ABX8XX_WDT_BMB_SHIFT	2
#define ABX8XX_WDT_MAX_TIME	(ABX8XX_WDT_BMB_MASK >> ABX8XX_WDT_BMB_SHIFT)
#define ABX8XX_WDT_WRB_MASK	0x03
#define ABX8XX_WDT_WRB_1HZ	0x02

#define ABX8XX_REG_CFG_KEY	0x1f
#define ABX8XX_CFG_KEY_OSC	0xa1
#define ABX8XX_CFG_KEY_MISC	0x9d

#define ABX8XX_REG_ID0		0x28

#define ABX8XX_REG_OUT_CTRL	0x30
#define ABX8XX_OUT_CTRL_EXDS	BIT(4)

#define ABX8XX_REG_TRICKLE	0x20
#define ABX8XX_TRICKLE_CHARGE_ENABLE	0xa0
#define ABX8XX_TRICKLE_STANDARD_DIODE	0x8
#define ABX8XX_TRICKLE_SCHOTTKY_DIODE	0x4

#define ABX8XX_REG_EXTRAM	0x3f
#define ABX8XX_EXTRAM_XADS	GENMASK(1, 0)

#define ABX8XX_SRAM_BASE	0x40
#define ABX8XX_SRAM_WIN_SIZE	0x40U
#define ABX8XX_RAM_SIZE		256

#define RAM_ADDR_LOWER		GENMASK(5, 0)
#define RAM_ADDR_UPPER		GENMASK(7, 6)

static u8 trickle_resistors[] = {0, 3, 6, 11};

enum abx80x_chip {AB0801, AB0803, AB0804, AB0805,
	AB1801, AB1803, AB1804, AB1805, RV1805, ABX80X};

struct abx80x_cap {
	u16 pn;
	bool has_tc;
	bool has_wdog;
};

static struct abx80x_cap abx80x_caps[] = {
	[AB0801] = {.pn = 0x0801},
	[AB0803] = {.pn = 0x0803},
	[AB0804] = {.pn = 0x0804, .has_tc = true, .has_wdog = true},
	[AB0805] = {.pn = 0x0805, .has_tc = true, .has_wdog = true},
	[AB1801] = {.pn = 0x1801},
	[AB1803] = {.pn = 0x1803},
	[AB1804] = {.pn = 0x1804, .has_tc = true, .has_wdog = true},
	[AB1805] = {.pn = 0x1805, .has_tc = true, .has_wdog = true},
	[RV1805] = {.pn = 0x1805, .has_tc = true, .has_wdog = true},
	[ABX80X] = {.pn = 0}
};

static int abx80x_rtc_xfer(struct udevice *dev, unsigned int offset,
			   u8 *val, unsigned int bytes, bool write)
{
	int ret;

	if (offset + bytes > ABX8XX_RAM_SIZE)
		return -EINVAL;

	while (bytes) {
		u8 extram, reg, len, lower, upper;

		lower = FIELD_GET(RAM_ADDR_LOWER, offset);
		upper = FIELD_GET(RAM_ADDR_UPPER, offset);
		extram = FIELD_PREP(ABX8XX_EXTRAM_XADS, upper);
		reg = ABX8XX_SRAM_BASE + lower;
		len = min(lower + bytes, ABX8XX_SRAM_WIN_SIZE) - lower;

		ret = dm_i2c_reg_write(dev, ABX8XX_REG_EXTRAM, extram);
		if (ret)
			return ret;

		if (write)
			ret = dm_i2c_write(dev, reg, val, len);
		else
			ret = dm_i2c_read(dev, reg, val, len);
		if (ret)
			return ret;

		offset += len;
		val += len;
		bytes -= len;
	}

	return 0;
}

static int abx80x_rtc_read(struct udevice *dev, unsigned int offset, u8 *val,
			   unsigned int bytes)
{
	return abx80x_rtc_xfer(dev, offset, val, bytes, false);
}

static int abx80x_rtc_write(struct udevice *dev, unsigned int offset,
			    const u8 *val, unsigned int bytes)
{
	return abx80x_rtc_xfer(dev, offset, (u8 *)val, bytes, true);
}

static int abx80x_is_rc_mode(struct udevice *dev)
{
	int flags = 0;

	flags =  dm_i2c_reg_read(dev, ABX8XX_REG_OSS);
	if (flags < 0) {
		log_err("Failed to read autocalibration attribute\n");
		return flags;
	}

	return (flags & ABX8XX_OSS_OMODE) ? 1 : 0;
}

static int abx80x_enable_trickle_charger(struct udevice *dev, u8 trickle_cfg)
{
	int err;

	/*
	 * Write the configuration key register to enable access to the Trickle
	 * register
	 */
	err = dm_i2c_reg_write(dev, ABX8XX_REG_CFG_KEY, ABX8XX_CFG_KEY_MISC);
	if (err < 0) {
		log_err("Unable to write configuration key\n");
		return -EIO;
	}

	err = dm_i2c_reg_write(dev, ABX8XX_REG_TRICKLE,
			       ABX8XX_TRICKLE_CHARGE_ENABLE | trickle_cfg);
	if (err < 0) {
		log_err("Unable to write trickle register\n");
		return -EIO;
	}

	return 0;
}

static int abx80x_rtc_read_time(struct udevice *dev, struct rtc_time *tm)
{
	unsigned char buf[8];
	int err, flags, rc_mode = 0;

	/* Read the Oscillator Failure only in XT mode */
	rc_mode = abx80x_is_rc_mode(dev);
	if (rc_mode < 0)
		return rc_mode;

	if (!rc_mode) {
		flags = dm_i2c_reg_read(dev, ABX8XX_REG_OSS);
		if (flags < 0) {
			log_err("Unable to read oscillator status.\n");
			return flags;
		}

		if (flags & ABX8XX_OSS_OF)
			log_debug("Oscillator fail, data is not accurate.\n");
	}

	err = dm_i2c_read(dev, ABX8XX_REG_HTH,
			  buf, sizeof(buf));
	if (err < 0) {
		log_err("Unable to read date\n");
		return -EIO;
	}

	tm->tm_sec = bcd2bin(buf[ABX8XX_REG_SC] & 0x7F);
	tm->tm_min = bcd2bin(buf[ABX8XX_REG_MN] & 0x7F);
	tm->tm_hour = bcd2bin(buf[ABX8XX_REG_HR] & 0x3F);
	tm->tm_wday = buf[ABX8XX_REG_WD] & 0x7;
	tm->tm_mday = bcd2bin(buf[ABX8XX_REG_DA] & 0x3F);
	tm->tm_mon = bcd2bin(buf[ABX8XX_REG_MO] & 0x1F);
	tm->tm_year = bcd2bin(buf[ABX8XX_REG_YR]) + 2000;

	return 0;
}

static int abx80x_rtc_set_time(struct udevice *dev, const struct rtc_time *tm)
{
	unsigned char buf[8];
	int err, flags;

	if (tm->tm_year < 2000)
		return -EINVAL;

	buf[ABX8XX_REG_HTH] = 0;
	buf[ABX8XX_REG_SC] = bin2bcd(tm->tm_sec);
	buf[ABX8XX_REG_MN] = bin2bcd(tm->tm_min);
	buf[ABX8XX_REG_HR] = bin2bcd(tm->tm_hour);
	buf[ABX8XX_REG_DA] = bin2bcd(tm->tm_mday);
	buf[ABX8XX_REG_MO] = bin2bcd(tm->tm_mon);
	buf[ABX8XX_REG_YR] = bin2bcd(tm->tm_year - 2000);
	buf[ABX8XX_REG_WD] = tm->tm_wday;

	err = dm_i2c_write(dev, ABX8XX_REG_HTH,
			   buf, sizeof(buf));
	if (err < 0) {
		log_err("Unable to write to date registers\n");
		return -EIO;
	}

	/* Clear the OF bit of Oscillator Status Register */
	flags = dm_i2c_reg_read(dev, ABX8XX_REG_OSS);
	if (flags < 0) {
		log_err("Unable to read oscillator status.\n");
		return flags;
	}

	err = dm_i2c_reg_write(dev, ABX8XX_REG_OSS,
			       flags & ~ABX8XX_OSS_OF);
	if (err < 0) {
		log_err("Unable to write oscillator status register\n");
		return err;
	}

	return 0;
}

static int abx80x_rtc_set_autocalibration(struct udevice *dev,
					  int autocalibration)
{
	int retval, flags = 0;

	if (autocalibration != 0 && autocalibration != 1024 &&
	    autocalibration != 512) {
		log_err("autocalibration value outside permitted range\n");
		return -EINVAL;
	}

	flags = dm_i2c_reg_read(dev, ABX8XX_REG_OSC);
	if (flags < 0)
		return flags;

	if (autocalibration == 0) {
		flags &= ~(ABX8XX_OSC_ACAL_512 | ABX8XX_OSC_ACAL_1024);
	} else if (autocalibration == 1024) {
		/* 1024 autocalibration is 0x10 */
		flags |= ABX8XX_OSC_ACAL_1024;
		flags &= ~(ABX8XX_OSC_ACAL_512);
	} else {
		/* 512 autocalibration is 0x11 */
		flags |= (ABX8XX_OSC_ACAL_1024 | ABX8XX_OSC_ACAL_512);
	}

	/* Unlock write access to Oscillator Control Register */
	retval = dm_i2c_reg_write(dev, ABX8XX_REG_CFG_KEY,
				  ABX8XX_CFG_KEY_OSC);
	if (retval < 0) {
		log_err("Failed to write CONFIG_KEY register\n");
		return retval;
	}

	retval = dm_i2c_reg_write(dev, ABX8XX_REG_OSC, flags);

	return retval;
}

static int abx80x_rtc_get_autocalibration(struct udevice *dev)
{
	int flags = 0, autocalibration;

	flags =  dm_i2c_reg_read(dev, ABX8XX_REG_OSC);
	if (flags < 0)
		return flags;

	if (flags & ABX8XX_OSC_ACAL_512)
		autocalibration = 512;
	else if (flags & ABX8XX_OSC_ACAL_1024)
		autocalibration = 1024;
	else
		autocalibration = 0;

	return autocalibration;
}

static struct rtc_time default_tm = { 0, 0, 0, 1, 1, 2000, 6, 0, 0 };

static int abx80x_rtc_reset(struct udevice *dev)
{
	int ret = 0;

	int autocalib = abx80x_rtc_get_autocalibration(dev);

	if (autocalib != 0)
		abx80x_rtc_set_autocalibration(dev, 0);

	ret = abx80x_rtc_set_time(dev, &default_tm);
	if (ret != 0) {
		log_err("cannot set time to default_tm. error %d\n", ret);
		return ret;
	}

	return ret;
}

static const struct rtc_ops abx80x_rtc_ops = {
	.get	= abx80x_rtc_read_time,
	.set	= abx80x_rtc_set_time,
	.reset	= abx80x_rtc_reset,
	.read	= abx80x_rtc_read,
	.write	= abx80x_rtc_write,
};

static int abx80x_dt_trickle_cfg(struct udevice *dev)
{
	const char *diode;
	int trickle_cfg = 0;
	int i, ret = 0;
	u32 tmp;

	diode = ofnode_read_string(dev_ofnode(dev), "abracon,tc-diode");
	if (!diode)
		return ret;

	if (!strcmp(diode, "standard")) {
		trickle_cfg |= ABX8XX_TRICKLE_STANDARD_DIODE;
	} else if (!strcmp(diode, "schottky")) {
		trickle_cfg |= ABX8XX_TRICKLE_SCHOTTKY_DIODE;
	} else {
		log_err("Invalid tc-diode value: %s\n", diode);
		return -EINVAL;
	}

	ret = ofnode_read_u32(dev_ofnode(dev), "abracon,tc-resistor", &tmp);
	if (ret)
		return ret;

	for (i = 0; i < sizeof(trickle_resistors); i++)
		if (trickle_resistors[i] == tmp)
			break;

	if (i == sizeof(trickle_resistors)) {
		log_err("Invalid tc-resistor value: %u\n", tmp);
		return -EINVAL;
	}

	return (trickle_cfg | i);
}

static int abx80x_probe(struct udevice *dev)
{
	int i, data, err, trickle_cfg = -EINVAL;
	unsigned char buf[7];
	unsigned int part = dev->driver_data;
	unsigned int partnumber;
	unsigned int majrev, minrev;
	unsigned int lot;
	unsigned int wafer;
	unsigned int uid;

	err = dm_i2c_read(dev, ABX8XX_REG_ID0, buf, sizeof(buf));
	if (err < 0) {
		log_err("Unable to read partnumber\n");
		return -EIO;
	}

	partnumber = (buf[0] << 8) | buf[1];
	majrev = buf[2] >> 3;
	minrev = buf[2] & 0x7;
	lot = ((buf[4] & 0x80) << 2) | ((buf[6] & 0x80) << 1) | buf[3];
	uid = ((buf[4] & 0x7f) << 8) | buf[5];
	wafer = (buf[6] & 0x7c) >> 2;
	log_debug("model %04x, revision %u.%u, lot %x, wafer %x, uid %x\n",
		  partnumber, majrev, minrev, lot, wafer, uid);

	data = dm_i2c_reg_read(dev, ABX8XX_REG_CTRL1);
	if (data < 0) {
		log_err("Unable to read control register\n");
		return -EIO;
	}

	err = dm_i2c_reg_write(dev, ABX8XX_REG_CTRL1,
			       ((data & ~(ABX8XX_CTRL_12_24 |
					  ABX8XX_CTRL_ARST)) |
				ABX8XX_CTRL_WRITE));
	if (err < 0) {
		log_err("Unable to write control register\n");
		return -EIO;
	}

	/* Configure RV1805 specifics */
	if (part == RV1805) {
		/*
		 * Avoid accidentally entering test mode. This can happen
		 * on the RV1805 in case the reserved bit 5 in control2
		 * register is set. RV-1805-C3 datasheet indicates that
		 * the bit should be cleared in section 11h - Control2.
		 */
		data = dm_i2c_reg_read(dev, ABX8XX_REG_CTRL2);
		if (data < 0) {
			log_err("Unable to read control2 register\n");
			return -EIO;
		}

		err = dm_i2c_reg_write(dev, ABX8XX_REG_CTRL2,
				       data & ~ABX8XX_CTRL2_RSVD);
		if (err < 0) {
			log_err("Unable to write control2 register\n");
			return -EIO;
		}

		/*
		 * Avoid extra power leakage. The RV1805 uses smaller
		 * 10pin package and the EXTI input is not present.
		 * Disable it to avoid leakage.
		 */
		data = dm_i2c_reg_read(dev, ABX8XX_REG_OUT_CTRL);
		if (data < 0) {
			log_err("Unable to read output control register\n");
			return -EIO;
		}

		/*
		 * Write the configuration key register to enable access to
		 * the config2 register
		 */
		err = dm_i2c_reg_write(dev, ABX8XX_REG_CFG_KEY,
				       ABX8XX_CFG_KEY_MISC);
		if (err < 0) {
			log_err("Unable to write configuration key\n");
			return -EIO;
		}

		err = dm_i2c_reg_write(dev, ABX8XX_REG_OUT_CTRL,
				       data | ABX8XX_OUT_CTRL_EXDS);
		if (err < 0) {
			log_err("Unable to write output control register\n");
			return -EIO;
		}
	}

	/* part autodetection */
	if (part == ABX80X) {
		for (i = 0; abx80x_caps[i].pn; i++)
			if (partnumber == abx80x_caps[i].pn)
				break;
		if (abx80x_caps[i].pn == 0) {
			log_err("Unknown part: %04x\n", partnumber);
			return -EINVAL;
		}
		part = i;
	}

	if (partnumber != abx80x_caps[part].pn) {
		log_err("partnumber mismatch %04x != %04x\n",
			partnumber, abx80x_caps[part].pn);
		return -EINVAL;
	}

	if (abx80x_caps[part].has_tc)
		trickle_cfg = abx80x_dt_trickle_cfg(dev);

	if (trickle_cfg > 0) {
		log_debug("Enabling trickle charger: %02x\n", trickle_cfg);
		abx80x_enable_trickle_charger(dev, trickle_cfg);
	}

	err = dm_i2c_reg_write(dev, ABX8XX_REG_CD_TIMER_CTL, BIT(2));
	if (err)
		return err;

	return 0;
}

static const struct udevice_id abx80x_of_match[] = {
	{
		.compatible = "abracon,abx80x",
		.data = ABX80X
	},
	{
		.compatible = "abracon,ab0801",
		.data = AB0801
	},
	{
		.compatible = "abracon,ab0803",
		.data = AB0803
	},
	{
		.compatible = "abracon,ab0804",
		.data = AB0804
	},
	{
		.compatible = "abracon,ab0805",
		.data = AB0805
	},
	{
		.compatible = "abracon,ab1801",
		.data = AB1801
	},
	{
		.compatible = "abracon,ab1803",
		.data = AB1803
	},
	{
		.compatible = "abracon,ab1804",
		.data = AB1804
	},
	{
		.compatible = "abracon,ab1805",
		.data = AB1805
	},
	{
		.compatible = "microcrystal,rv1805",
		.data = RV1805
	},
	{ }
};

U_BOOT_DRIVER(abx80x_rtc) = {
	.name	= "rtc-abx80x",
	.id     = UCLASS_RTC,
	.probe		= abx80x_probe,
	.of_match = abx80x_of_match,
	.ops = &abx80x_rtc_ops,
};
