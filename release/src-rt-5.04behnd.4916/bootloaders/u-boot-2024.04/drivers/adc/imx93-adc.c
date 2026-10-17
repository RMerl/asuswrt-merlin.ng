// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2023 ASEM Srl
 * Author: Luca Ellero <l.ellero@asem.it>
 *
 * Originally based on NXP linux-imx kernel v5.15 drivers/iio/adc/imx93_adc.c
 */

#include <common.h>
#include <errno.h>
#include <dm.h>
#include <linux/bitfield.h>
#include <linux/iopoll.h>
#include <clk.h>
#include <adc.h>

#define IMX93_ADC_MCR			0x00
#define IMX93_ADC_MSR			0x04
#define IMX93_ADC_ISR			0x10
#define IMX93_ADC_IMR			0x20
#define IMX93_ADC_CIMR0			0x24
#define IMX93_ADC_CTR0			0x94
#define IMX93_ADC_NCMR0			0xA4
#define IMX93_ADC_PCDR0			0x100
#define IMX93_ADC_PCDR1			0x104
#define IMX93_ADC_PCDR2			0x108
#define IMX93_ADC_PCDR3			0x10c
#define IMX93_ADC_PCDR4			0x110
#define IMX93_ADC_PCDR5			0x114
#define IMX93_ADC_PCDR6			0x118
#define IMX93_ADC_PCDR7			0x11c
#define IMX93_ADC_CALSTAT		0x39C

#define IMX93_ADC_MCR_MODE_MASK		BIT(29)
#define IMX93_ADC_MCR_NSTART_MASK	BIT(24)
#define IMX93_ADC_MCR_CALSTART_MASK	BIT(14)
#define IMX93_ADC_MCR_ADCLKSE_MASK	BIT(8)
#define IMX93_ADC_MCR_PWDN_MASK		BIT(0)

#define IMX93_ADC_MSR_CALFAIL_MASK	BIT(30)
#define IMX93_ADC_MSR_CALBUSY_MASK	BIT(29)
#define IMX93_ADC_MSR_ADCSTATUS_MASK	GENMASK(2, 0)

#define IMX93_ADC_ISR_EOC_MASK		BIT(1)

#define IMX93_ADC_IMR_EOC_MASK		BIT(1)
#define IMX93_ADC_IMR_ECH_MASK		BIT(0)

#define IMX93_ADC_PCDR_CDATA_MASK	GENMASK(11, 0)

#define IDLE				0
#define POWER_DOWN			1
#define WAIT_STATE			2
#define BUSY_IN_CALIBRATION		3
#define SAMPLE				4
#define CONVERSION			6

#define IMX93_ADC_MAX_CHANNEL		3
#define IMX93_ADC_DAT_MASK		0xfff
#define IMX93_ADC_TIMEOUT		100000

struct imx93_adc_priv {
	int active_channel;
	void __iomem *regs;
	struct clk ipg_clk;
};

static void imx93_adc_power_down(struct imx93_adc_priv *adc)
{
	u32 mcr, msr;
	int ret;

	mcr = readl(adc->regs + IMX93_ADC_MCR);
	mcr |= FIELD_PREP(IMX93_ADC_MCR_PWDN_MASK, 1);
	writel(mcr, adc->regs + IMX93_ADC_MCR);

	ret = readl_poll_timeout(adc->regs + IMX93_ADC_MSR, msr,
		((msr & IMX93_ADC_MSR_ADCSTATUS_MASK) == POWER_DOWN), 50);
	if (ret == -ETIMEDOUT)
		pr_warn("ADC not in power down mode, current MSR: %x\n", msr);
}

static void imx93_adc_power_up(struct imx93_adc_priv *adc)
{
	u32 mcr;

	/* bring ADC out of power down state, in idle state */
	mcr = readl(adc->regs + IMX93_ADC_MCR);
	mcr &= ~FIELD_PREP(IMX93_ADC_MCR_PWDN_MASK, 1);
	writel(mcr, adc->regs + IMX93_ADC_MCR);
}

static void imx93_adc_config_ad_clk(struct imx93_adc_priv *adc)
{
	u32 mcr;

	/* put adc in power down mode */
	imx93_adc_power_down(adc);

	/* config the AD_CLK equal to bus clock */
	mcr = readl(adc->regs + IMX93_ADC_MCR);
	mcr |= FIELD_PREP(IMX93_ADC_MCR_ADCLKSE_MASK, 1);
	writel(mcr, adc->regs + IMX93_ADC_MCR);

	/* bring ADC out of power down state, in idle state */
	imx93_adc_power_up(adc);
}

static int imx93_adc_calibration(struct imx93_adc_priv *adc)
{
	u32 mcr, msr;
	int ret;

	/* make sure ADC is in power down mode */
	imx93_adc_power_down(adc);

	/* config SAR controller operating clock */
	mcr = readl(adc->regs + IMX93_ADC_MCR);
	mcr &= ~FIELD_PREP(IMX93_ADC_MCR_ADCLKSE_MASK, 1);
	writel(mcr, adc->regs + IMX93_ADC_MCR);

	/* bring ADC out of power down state */
	imx93_adc_power_up(adc);

	/*
	 * we use the default TSAMP/NRSMPL/AVGEN in MCR,
	 * can add the setting of these bit if need
	 */

	/* run calibration */
	mcr = readl(adc->regs + IMX93_ADC_MCR);
	mcr |= FIELD_PREP(IMX93_ADC_MCR_CALSTART_MASK, 1);
	writel(mcr, adc->regs + IMX93_ADC_MCR);

	/* wait calibration to be finished */
	ret = readl_poll_timeout(adc->regs + IMX93_ADC_MSR, msr,
		!(msr & IMX93_ADC_MSR_CALBUSY_MASK), 2000000);
	if (ret == -ETIMEDOUT) {
		pr_warn("ADC calibration timeout\n");
		return ret;
	}

	/* check whether calbration is successful or not */
	msr = readl(adc->regs + IMX93_ADC_MSR);
	if (msr & IMX93_ADC_MSR_CALFAIL_MASK) {
		pr_warn("ADC calibration failed!\n");
		return -EAGAIN;
	}

	return 0;
}

static int imx93_adc_channel_data(struct udevice *dev, int channel,
			    unsigned int *data)
{
	struct imx93_adc_priv *adc = dev_get_priv(dev);
	u32 isr, pcda;
	int ret;

	if (channel != adc->active_channel) {
		pr_err("Requested channel is not active!\n");
		return -EINVAL;
	}

	ret = readl_poll_timeout(adc->regs + IMX93_ADC_ISR, isr,
		(isr & IMX93_ADC_ISR_EOC_MASK), IMX93_ADC_TIMEOUT);

	/* clear interrupts */
	writel(isr, adc->regs + IMX93_ADC_ISR);

	if (ret == -ETIMEDOUT) {
		pr_warn("ADC conversion timeout!\n");
		return ret;
	}

	pcda = readl(adc->regs + IMX93_ADC_PCDR0 + channel * 4);

	*data = FIELD_GET(IMX93_ADC_PCDR_CDATA_MASK, pcda);

	return 0;
}

static int imx93_adc_start_channel(struct udevice *dev, int channel)
{
	struct imx93_adc_priv *adc = dev_get_priv(dev);
	u32 imr, mcr;

	/* config channel mask register */
	writel(1 << channel, adc->regs + IMX93_ADC_NCMR0);

	/* config interrupt mask */
	imr = FIELD_PREP(IMX93_ADC_IMR_EOC_MASK, 1);
	writel(imr, adc->regs + IMX93_ADC_IMR);
	writel(1 << channel, adc->regs + IMX93_ADC_CIMR0);

	/* config one-shot mode */
	mcr = readl(adc->regs + IMX93_ADC_MCR);
	mcr &= ~FIELD_PREP(IMX93_ADC_MCR_MODE_MASK, 1);
	writel(mcr, adc->regs + IMX93_ADC_MCR);

	/* start normal conversion */
	mcr = readl(adc->regs + IMX93_ADC_MCR);
	mcr |= FIELD_PREP(IMX93_ADC_MCR_NSTART_MASK, 1);
	writel(mcr, adc->regs + IMX93_ADC_MCR);

	adc->active_channel = channel;

	return 0;
}

static int imx93_adc_stop(struct udevice *dev)
{
	struct imx93_adc_priv *adc = dev_get_priv(dev);

	imx93_adc_power_down(adc);

	adc->active_channel = -1;

	return 0;
}

static int imx93_adc_probe(struct udevice *dev)
{
	struct imx93_adc_priv *adc = dev_get_priv(dev);
	unsigned int ret;

	ret = imx93_adc_calibration(adc);
	if (ret < 0)
		return ret;

	imx93_adc_config_ad_clk(adc);

	adc->active_channel = -1;

	return 0;
}

static int imx93_adc_of_to_plat(struct udevice *dev)
{
	struct adc_uclass_plat *uc_pdata = dev_get_uclass_plat(dev);
	struct imx93_adc_priv *adc = dev_get_priv(dev);
	unsigned int ret;

	adc->regs = dev_read_addr_ptr(dev);
	if (adc->regs == (struct imx93_adc *)FDT_ADDR_T_NONE) {
		pr_err("Dev: %s - can't get address!", dev->name);
		return -ENODATA;
	}

	ret = clk_get_by_name(dev, "ipg", &adc->ipg_clk);
	if (ret < 0) {
		pr_err("Can't get ADC ipg clk: %d\n", ret);
		return ret;
	}
	ret = clk_enable(&adc->ipg_clk);
	if(ret) {
		pr_err("Can't enable ADC ipg clk: %d\n", ret);
		return ret;
	}

	uc_pdata->data_mask = IMX93_ADC_DAT_MASK;
	uc_pdata->data_format = ADC_DATA_FORMAT_BIN;
	uc_pdata->data_timeout_us = IMX93_ADC_TIMEOUT;

	/* Mask available channel bits: [0:3] */
	uc_pdata->channel_mask = (2 << IMX93_ADC_MAX_CHANNEL) - 1;

	return 0;
}

static const struct adc_ops imx93_adc_ops = {
	.start_channel = imx93_adc_start_channel,
	.channel_data = imx93_adc_channel_data,
	.stop = imx93_adc_stop,
};

static const struct udevice_id imx93_adc_ids[] = {
	{ .compatible = "nxp,imx93-adc" },
	{ }
};

U_BOOT_DRIVER(imx93_adc) = {
	.name		= "imx93-adc",
	.id		= UCLASS_ADC,
	.of_match	= imx93_adc_ids,
	.ops		= &imx93_adc_ops,
	.probe		= imx93_adc_probe,
	.of_to_plat	= imx93_adc_of_to_plat,
	.priv_auto	= sizeof(struct imx93_adc_priv),
};
