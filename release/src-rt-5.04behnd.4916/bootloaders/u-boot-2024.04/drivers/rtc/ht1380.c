// SPDX-License-Identifier: GPL-2.0+
/*
 * Holtek HT1380/HT1381 Serial Timekeeper Chip
 *
 * Communication with the chip is vendor-specific.
 * It is done via 3 GPIO pins: reset, clock, and data.
 * Describe in .dts this way:
 *
 * rtc {
 *         compatible = "holtek,ht1380";
 *         rst-gpios = <&gpio 19 GPIO_ACTIVE_LOW>;
 *         clk-gpios = <&gpio 20 GPIO_ACTIVE_HIGH>;
 *         dat-gpios = <&gpio 21 GPIO_ACTIVE_HIGH>;
 * };
 *
 */

#include <common.h>
#include <dm.h>
#include <rtc.h>
#include <bcd.h>
#include <asm/gpio.h>
#include <linux/delay.h>

struct ht1380_priv {
	struct gpio_desc rst_desc;
	struct gpio_desc clk_desc;
	struct gpio_desc dat_desc;
};

enum registers {
	SEC,
	MIN,
	HOUR,
	MDAY,
	MONTH,
	WDAY,
	YEAR,
	WP,
	N_REGS
};

enum hour_mode {
	AMPM_MODE = 0x80, /* RTC is in AM/PM mode */
	PM_NOW = 0x20,    /* set if PM, clear if AM */
};

static const int BURST = 0xbe;
static const int READ = 1;

static void ht1380_half_period_delay(void)
{
	/*
	 * Delay for half a period. 1 us complies with the 500 KHz maximum
	 * input serial clock limit given by the datasheet.
	 */
	udelay(1);
}

static int ht1380_send_byte(struct ht1380_priv *priv, int byte)
{
	int ret;

	for (int bit = 0; bit < 8; bit++) {
		ret = dm_gpio_set_value(&priv->dat_desc, byte >> bit & 1);
		if (ret)
			break;
		ht1380_half_period_delay();

		ret = dm_gpio_set_value(&priv->clk_desc, 1);
		if (ret)
			break;
		ht1380_half_period_delay();

		ret = dm_gpio_set_value(&priv->clk_desc, 0);
		if (ret)
			break;
	}

	return ret;
}

/*
 * Leave reset state. The transfer operation can then be started.
 */
static int ht1380_reset_off(struct ht1380_priv *priv)
{
	const unsigned int T_CC = 4; /* us, Reset to Clock Setup */
	int ret;

	/*
	 * Leave RESET state.
	 * Make sure we make the minimal delay required by the datasheet.
	 */
	ret = dm_gpio_set_value(&priv->rst_desc, 0);
	udelay(T_CC);

	return ret;
}

/*
 * Enter reset state. Completes the transfer operation.
 */
static int ht1380_reset_on(struct ht1380_priv *priv)
{
	const unsigned int T_CWH = 4; /* us, Reset Inactive Time */
	int ret;

	/*
	 * Enter RESET state.
	 * Make sure we make the minimal delay required by the datasheet.
	 */
	ret = dm_gpio_set_value(&priv->rst_desc, 1);
	udelay(T_CWH);

	return ret;
}

static int ht1380_rtc_get(struct udevice *dev, struct rtc_time *tm)
{
	struct ht1380_priv *priv = dev_get_priv(dev);
	int ret, i, bit, reg[N_REGS];

	ret = dm_gpio_set_value(&priv->clk_desc, 0);
	if (ret)
		return ret;

	ret = dm_gpio_set_dir_flags(&priv->dat_desc, GPIOD_IS_OUT);
	if (ret)
		return ret;

	ret = ht1380_reset_off(priv);
	if (ret)
		goto exit;

	ret = ht1380_send_byte(priv, BURST + READ);
	if (ret)
		goto exit;

	ret = dm_gpio_set_dir_flags(&priv->dat_desc, GPIOD_IS_IN);
	if (ret)
		goto exit;

	for (i = 0; i < N_REGS; i++) {
		reg[i] = 0;

		for (bit = 0; bit < 8; bit++) {
			ht1380_half_period_delay();

			ret = dm_gpio_set_value(&priv->clk_desc, 1);
			if (ret)
				goto exit;
			ht1380_half_period_delay();

			reg[i] |= dm_gpio_get_value(&priv->dat_desc) << bit;
			ret = dm_gpio_set_value(&priv->clk_desc, 0);
			if (ret)
				goto exit;
		}
	}

	ret = -EINVAL;

	/* Correctness check: some bits are always zero */
	if (reg[MIN] & 0x80 || reg[HOUR] & 0x40 || reg[MDAY] & 0xc0 ||
	    reg[MONTH] & 0xe0 || reg[WDAY] & 0xf8 || reg[WP] & 0x7f)
		goto exit;

	/* Correctness check: some registers are always non-zero */
	if (!reg[MDAY] || !reg[MONTH] || !reg[WDAY])
		goto exit;

	tm->tm_sec = bcd2bin(reg[SEC]);
	tm->tm_min = bcd2bin(reg[MIN]);
	if (reg[HOUR] & AMPM_MODE) {
		/* AM-PM Mode, range is 01-12 */
		tm->tm_hour = bcd2bin(reg[HOUR] & 0x1f) % 12;
		if (reg[HOUR] & PM_NOW) {
			/* it is PM (otherwise AM) */
			tm->tm_hour += 12;
		}
	} else {
		/* 24-hour Mode, range is 0-23 */
		tm->tm_hour = bcd2bin(reg[HOUR]);
	}
	tm->tm_mday = bcd2bin(reg[MDAY]);
	tm->tm_mon = bcd2bin(reg[MONTH]);
	tm->tm_year = 2000 + bcd2bin(reg[YEAR]);
	tm->tm_wday = bcd2bin(reg[WDAY]) - 1;
	tm->tm_yday = 0;
	tm->tm_isdst = 0;

	ret = 0;

exit:
	ht1380_reset_on(priv);

	return ret;
}

static int ht1380_write_protection_off(struct ht1380_priv *priv)
{
	int ret;
	const int PROTECT = 0x8e;

	ret = ht1380_reset_off(priv);
	if (ret)
		return ret;

	ret = ht1380_send_byte(priv, PROTECT);
	if (ret)
		return ret;
	ret = ht1380_send_byte(priv, 0); /* WP bit is 0 */
	if (ret)
		return ret;

	return ht1380_reset_on(priv);
}

static int ht1380_rtc_set(struct udevice *dev, const struct rtc_time *tm)
{
	struct ht1380_priv *priv = dev_get_priv(dev);
	int ret, i, reg[N_REGS];

	ret = dm_gpio_set_value(&priv->clk_desc, 0);
	if (ret)
		return ret;

	ret = dm_gpio_set_dir_flags(&priv->dat_desc, GPIOD_IS_OUT);
	if (ret)
		goto exit;

	ret = ht1380_write_protection_off(priv);
	if (ret)
		goto exit;

	reg[SEC] = bin2bcd(tm->tm_sec);
	reg[MIN] = bin2bcd(tm->tm_min);
	reg[HOUR] = bin2bcd(tm->tm_hour);
	reg[MDAY] = bin2bcd(tm->tm_mday);
	reg[MONTH] = bin2bcd(tm->tm_mon);
	reg[WDAY] = bin2bcd(tm->tm_wday) + 1;
	reg[YEAR] = bin2bcd(tm->tm_year - 2000);
	reg[WP] = 0x80; /* WP bit is 1 */

	ret = ht1380_reset_off(priv);
	if (ret)
		goto exit;

	ret = ht1380_send_byte(priv, BURST);
	for (i = 0; i < N_REGS && ret; i++)
		ret = ht1380_send_byte(priv, reg[i]);

exit:
	ht1380_reset_on(priv);

	return ret;
}

static int ht1380_probe(struct udevice *dev)
{
	int ret;
	struct ht1380_priv *priv;

	priv = dev_get_priv(dev);
	if (!priv)
		return -EINVAL;

	ret = gpio_request_by_name(dev, "rst-gpios", 0,
				   &priv->rst_desc, GPIOD_IS_OUT);
	if (ret)
		goto fail_rst;

	ret = gpio_request_by_name(dev, "clk-gpios", 0,
				   &priv->clk_desc, GPIOD_IS_OUT);
	if (ret)
		goto fail_clk;

	ret = gpio_request_by_name(dev, "dat-gpios", 0,
				   &priv->dat_desc, 0);
	if (ret)
		goto fail_dat;

	ret = ht1380_reset_on(priv);
	if (ret)
		goto fail;

	return 0;

fail:
	dm_gpio_free(dev, &priv->dat_desc);
fail_dat:
	dm_gpio_free(dev, &priv->clk_desc);
fail_clk:
	dm_gpio_free(dev, &priv->rst_desc);
fail_rst:
	return ret;
}

static int ht1380_remove(struct udevice *dev)
{
	struct ht1380_priv *priv = dev_get_priv(dev);

	dm_gpio_free(dev, &priv->rst_desc);
	dm_gpio_free(dev, &priv->clk_desc);
	dm_gpio_free(dev, &priv->dat_desc);

	return 0;
}

static const struct rtc_ops ht1380_rtc_ops = {
	.get = ht1380_rtc_get,
	.set = ht1380_rtc_set,
};

static const struct udevice_id ht1380_rtc_ids[] = {
	{ .compatible = "holtek,ht1380" },
	{ }
};

U_BOOT_DRIVER(rtc_ht1380) = {
	.name = "rtc-ht1380",
	.id = UCLASS_RTC,
	.probe = ht1380_probe,
	.remove = ht1380_remove,
	.of_match = ht1380_rtc_ids,
	.ops = &ht1380_rtc_ops,
	.priv_auto = sizeof(struct ht1380_priv),
};
