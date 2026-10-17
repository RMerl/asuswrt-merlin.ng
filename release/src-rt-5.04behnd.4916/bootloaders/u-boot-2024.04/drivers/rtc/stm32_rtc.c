// SPDX-License-Identifier: GPL-2.0+ OR BSD-3-Clause
/*
 * Copyright (C) 2019, STMicroelectronics - All Rights Reserved
 */

#define LOG_CATEGORY UCLASS_RTC

#include <common.h>
#include <clk.h>
#include <dm.h>
#include <malloc.h>
#include <rtc.h>
#include <asm/io.h>
#include <dm/device_compat.h>
#include <linux/bitops.h>
#include <linux/iopoll.h>

#define STM32_RTC_TR		0x00
#define STM32_RTC_DR		0x04
#define STM32_RTC_ISR		0x0C
#define STM32_RTC_PRER		0x10
#define STM32_RTC_CR		0x18
#define STM32_RTC_WPR		0x24

/* STM32_RTC_TR bit fields  */
#define STM32_RTC_SEC_SHIFT	0
#define STM32_RTC_SEC		GENMASK(6, 0)
#define STM32_RTC_MIN_SHIFT	8
#define STM32_RTC_MIN		GENMASK(14, 8)
#define STM32_RTC_HOUR_SHIFT	16
#define STM32_RTC_HOUR		GENMASK(21, 16)

/* STM32_RTC_DR bit fields */
#define STM32_RTC_DATE_SHIFT	0
#define STM32_RTC_DATE		GENMASK(5, 0)
#define STM32_RTC_MONTH_SHIFT	8
#define STM32_RTC_MONTH		GENMASK(12, 8)
#define STM32_RTC_WDAY_SHIFT	13
#define STM32_RTC_WDAY		GENMASK(15, 13)
#define STM32_RTC_YEAR_SHIFT	16
#define STM32_RTC_YEAR		GENMASK(23, 16)

/* STM32_RTC_CR bit fields */
#define STM32_RTC_CR_FMT	BIT(6)

/* STM32_RTC_ISR/STM32_RTC_ICSR bit fields */
#define STM32_RTC_ISR_INITS	BIT(4)
#define STM32_RTC_ISR_RSF	BIT(5)
#define STM32_RTC_ISR_INITF	BIT(6)
#define STM32_RTC_ISR_INIT	BIT(7)

/* STM32_RTC_PRER bit fields */
#define STM32_RTC_PRER_PRED_S_SHIFT	0
#define STM32_RTC_PRER_PRED_S		GENMASK(14, 0)
#define STM32_RTC_PRER_PRED_A_SHIFT	16
#define STM32_RTC_PRER_PRED_A		GENMASK(22, 16)

/* STM32_RTC_WPR key constants */
#define RTC_WPR_1ST_KEY		0xCA
#define RTC_WPR_2ND_KEY		0x53
#define RTC_WPR_WRONG_KEY	0xFF

struct stm32_rtc_priv {
	fdt_addr_t base;
};

static int stm32_rtc_get(struct udevice *dev, struct rtc_time *tm)
{
	struct stm32_rtc_priv *priv = dev_get_priv(dev);
	u32 tr, dr;

	tr = readl(priv->base + STM32_RTC_TR);
	dr = readl(priv->base + STM32_RTC_DR);

	tm->tm_sec = bcd2bin((tr & STM32_RTC_SEC) >> STM32_RTC_SEC_SHIFT);
	tm->tm_min = bcd2bin((tr & STM32_RTC_MIN) >> STM32_RTC_MIN_SHIFT);
	tm->tm_hour = bcd2bin((tr & STM32_RTC_HOUR) >> STM32_RTC_HOUR_SHIFT);

	tm->tm_mday = bcd2bin((dr & STM32_RTC_DATE) >> STM32_RTC_DATE_SHIFT);
	tm->tm_mon = bcd2bin((dr & STM32_RTC_MONTH) >> STM32_RTC_MONTH_SHIFT);
	tm->tm_year = 2000 +
		      bcd2bin((dr & STM32_RTC_YEAR) >> STM32_RTC_YEAR_SHIFT);
	tm->tm_wday = bcd2bin((dr & STM32_RTC_WDAY) >> STM32_RTC_WDAY_SHIFT);
	tm->tm_yday = 0;
	tm->tm_isdst = 0;

	dev_dbg(dev, "Get DATE: %4d-%02d-%02d (wday=%d)  TIME: %2d:%02d:%02d\n",
		tm->tm_year, tm->tm_mon, tm->tm_mday, tm->tm_wday,
		tm->tm_hour, tm->tm_min, tm->tm_sec);

	return 0;
}

static void stm32_rtc_unlock(struct udevice *dev)
{
	struct stm32_rtc_priv *priv = dev_get_priv(dev);

	writel(RTC_WPR_1ST_KEY, priv->base + STM32_RTC_WPR);
	writel(RTC_WPR_2ND_KEY, priv->base + STM32_RTC_WPR);
}

static void stm32_rtc_lock(struct udevice *dev)
{
	struct stm32_rtc_priv *priv = dev_get_priv(dev);

	writel(RTC_WPR_WRONG_KEY, priv->base + STM32_RTC_WPR);
}

static int stm32_rtc_enter_init_mode(struct udevice *dev)
{
	struct stm32_rtc_priv *priv = dev_get_priv(dev);
	u32 isr = readl(priv->base + STM32_RTC_ISR);

	if (!(isr & STM32_RTC_ISR_INITF)) {
		isr |= STM32_RTC_ISR_INIT;
		writel(isr, priv->base + STM32_RTC_ISR);

		return readl_poll_timeout(priv->base + STM32_RTC_ISR,
					  isr,
					  (isr & STM32_RTC_ISR_INITF),
					  100000);
	}

	return 0;
}

static int stm32_rtc_wait_sync(struct udevice *dev)
{
	struct stm32_rtc_priv *priv = dev_get_priv(dev);
	u32 isr = readl(priv->base + STM32_RTC_ISR);

	isr &= ~STM32_RTC_ISR_RSF;
	writel(isr, priv->base + STM32_RTC_ISR);

	/*
	 * Wait for RSF to be set to ensure the calendar registers are
	 * synchronised, it takes around 2 rtc_ck clock cycles
	 */
	return readl_poll_timeout(priv->base + STM32_RTC_ISR,
				  isr, (isr & STM32_RTC_ISR_RSF),
				  100000);
}

static void stm32_rtc_exit_init_mode(struct udevice *dev)
{
	struct stm32_rtc_priv *priv = dev_get_priv(dev);
	u32 isr = readl(priv->base + STM32_RTC_ISR);

	isr &= ~STM32_RTC_ISR_INIT;
	writel(isr, priv->base + STM32_RTC_ISR);
}

static int stm32_rtc_set_time(struct udevice *dev, u32 time, u32 date)
{
	struct stm32_rtc_priv *priv = dev_get_priv(dev);
	int ret;

	stm32_rtc_unlock(dev);

	ret = stm32_rtc_enter_init_mode(dev);
	if (ret)
		goto lock;

	writel(time, priv->base + STM32_RTC_TR);
	writel(date, priv->base + STM32_RTC_DR);

	stm32_rtc_exit_init_mode(dev);

	ret = stm32_rtc_wait_sync(dev);

lock:
	stm32_rtc_lock(dev);
	return ret;
}

static int stm32_rtc_set(struct udevice *dev, const struct rtc_time *tm)
{
	u32 t, d;

	dev_dbg(dev, "Set DATE: %4d-%02d-%02d (wday=%d)  TIME: %2d:%02d:%02d\n",
		tm->tm_year, tm->tm_mon, tm->tm_mday, tm->tm_wday,
		tm->tm_hour, tm->tm_min, tm->tm_sec);

	if (tm->tm_year < 2000 || tm->tm_year > 2099)
		return -EINVAL;

	/* Time in BCD format */
	t = (bin2bcd(tm->tm_sec) << STM32_RTC_SEC_SHIFT) & STM32_RTC_SEC;
	t |= (bin2bcd(tm->tm_min) << STM32_RTC_MIN_SHIFT) & STM32_RTC_MIN;
	t |= (bin2bcd(tm->tm_hour) << STM32_RTC_HOUR_SHIFT) & STM32_RTC_HOUR;

	/* Date in BCD format */
	d = (bin2bcd(tm->tm_mday) << STM32_RTC_DATE_SHIFT) & STM32_RTC_DATE;
	d |= (bin2bcd(tm->tm_mon) << STM32_RTC_MONTH_SHIFT) & STM32_RTC_MONTH;
	d |= (bin2bcd(tm->tm_year - 2000) << STM32_RTC_YEAR_SHIFT) &
	      STM32_RTC_YEAR;
	d |= (bin2bcd(tm->tm_wday) << STM32_RTC_WDAY_SHIFT) & STM32_RTC_WDAY;

	return stm32_rtc_set_time(dev, t, d);
}

static int stm32_rtc_reset(struct udevice *dev)
{
	dev_dbg(dev, "Reset DATE\n");

	return stm32_rtc_set_time(dev, 0, 0);
}

static int stm32_rtc_init(struct udevice *dev)
{
	struct stm32_rtc_priv *priv = dev_get_priv(dev);
	unsigned int prer, pred_a, pred_s, pred_a_max, pred_s_max, cr;
	unsigned int rate;
	struct clk clk;
	int ret;
	u32 isr = readl(priv->base + STM32_RTC_ISR);

	if (isr & STM32_RTC_ISR_INITS)
		return  0;

	ret = clk_get_by_index(dev, 1, &clk);
	if (ret)
		return ret;

	ret = clk_enable(&clk);
	if (ret)
		return ret;

	rate = clk_get_rate(&clk);

	/* Find prediv_a and prediv_s to obtain the 1Hz calendar clock */
	pred_a_max = STM32_RTC_PRER_PRED_A >> STM32_RTC_PRER_PRED_A_SHIFT;
	pred_s_max = STM32_RTC_PRER_PRED_S >> STM32_RTC_PRER_PRED_S_SHIFT;

	for (pred_a = pred_a_max; pred_a + 1 > 0; pred_a--) {
		pred_s = (rate / (pred_a + 1)) - 1;

		if (((pred_s + 1) * (pred_a + 1)) == rate)
			break;
	}

	/*
	 * Can't find a 1Hz, so give priority to RTC power consumption
	 * by choosing the higher possible value for prediv_a
	 */
	if (pred_s > pred_s_max || pred_a > pred_a_max) {
		pred_a = pred_a_max;
		pred_s = (rate / (pred_a + 1)) - 1;
	}

	stm32_rtc_unlock(dev);

	ret = stm32_rtc_enter_init_mode(dev);
	if (ret) {
		dev_err(dev,
			"Can't enter in init mode. Prescaler config failed.\n");
		goto unlock;
	}

	prer = (pred_s << STM32_RTC_PRER_PRED_S_SHIFT) & STM32_RTC_PRER_PRED_S;
	prer |= (pred_a << STM32_RTC_PRER_PRED_A_SHIFT) & STM32_RTC_PRER_PRED_A;
	writel(prer, priv->base + STM32_RTC_PRER);

	/* Force 24h time format */
	cr = readl(priv->base + STM32_RTC_CR);
	cr &= ~STM32_RTC_CR_FMT;
	writel(cr, priv->base + STM32_RTC_CR);

	stm32_rtc_exit_init_mode(dev);

	ret = stm32_rtc_wait_sync(dev);

unlock:
	stm32_rtc_lock(dev);

	if (ret)
		clk_disable(&clk);

	return ret;
}

static int stm32_rtc_probe(struct udevice *dev)
{
	struct stm32_rtc_priv *priv = dev_get_priv(dev);
	struct clk clk;
	int ret;

	priv->base = dev_read_addr(dev);
	if (priv->base == FDT_ADDR_T_NONE)
		return -EINVAL;

	ret = clk_get_by_index(dev, 0, &clk);
	if (ret)
		return ret;

	ret = clk_enable(&clk);
	if (ret)
		return ret;

	ret = stm32_rtc_init(dev);

	if (ret)
		clk_disable(&clk);

	return ret;
}

static const struct rtc_ops stm32_rtc_ops = {
	.get = stm32_rtc_get,
	.set = stm32_rtc_set,
	.reset = stm32_rtc_reset,
};

static const struct udevice_id stm32_rtc_ids[] = {
	{ .compatible = "st,stm32mp1-rtc" },
	{ }
};

U_BOOT_DRIVER(rtc_stm32) = {
	.name	= "rtc-stm32",
	.id	= UCLASS_RTC,
	.probe	= stm32_rtc_probe,
	.of_match = stm32_rtc_ids,
	.ops	= &stm32_rtc_ops,
	.priv_auto	= sizeof(struct stm32_rtc_priv),
};
