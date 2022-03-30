// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2015 Samsung Electronics
 * Przemyslaw Marczak <p.marczak@samsung.com>
 */
#include <common.h>
#include <errno.h>
#include <dm.h>
#include <adc.h>
#include <asm/arch/adc.h>

struct exynos_adc_priv {
	int active_channel;
	struct exynos_adc_v2 *regs;
};

int exynos_adc_channel_data(struct udevice *dev, int channel,
			    unsigned int *data)
{
	struct exynos_adc_priv *priv = dev_get_priv(dev);
	struct exynos_adc_v2 *regs = priv->regs;

	if (channel != priv->active_channel) {
		pr_err("Requested channel is not active!");
		return -EINVAL;
	}

	if (ADC_V2_GET_STATUS_FLAG(readl(&regs->status)) != FLAG_CONV_END)
		return -EBUSY;

	*data = readl(&regs->dat) & ADC_V2_DAT_MASK;

	return 0;
}

int exynos_adc_start_channel(struct udevice *dev, int channel)
{
	struct exynos_adc_priv *priv = dev_get_priv(dev);
	struct exynos_adc_v2 *regs = priv->regs;
	unsigned int cfg;

	/* Choose channel */
	cfg = readl(&regs->con2);
	cfg &= ~ADC_V2_CON2_CHAN_SEL_MASK;
	cfg |= ADC_V2_CON2_CHAN_SEL(channel);
	writel(cfg, &regs->con2);

	/* Start conversion */
	cfg = readl(&regs->con1);
	writel(cfg | ADC_V2_CON1_STC_EN, &regs->con1);

	priv->active_channel = channel;

	return 0;
}

int exynos_adc_stop(struct udevice *dev)
{
	struct exynos_adc_priv *priv = dev_get_priv(dev);
	struct exynos_adc_v2 *regs = priv->regs;
	unsigned int cfg;

	/* Stop conversion */
	cfg = readl(&regs->con1);
	cfg &= ~ADC_V2_CON1_STC_EN;

	writel(cfg, &regs->con1);

	priv->active_channel = -1;

	return 0;
}

int exynos_adc_probe(struct udevice *dev)
{
	struct exynos_adc_priv *priv = dev_get_priv(dev);
	struct exynos_adc_v2 *regs = priv->regs;
	unsigned int cfg;

	/* Check HW version */
	if (readl(&regs->version) != ADC_V2_VERSION) {
		pr_err("This driver supports only ADC v2!");
		return -ENXIO;
	}

	/* ADC Reset */
	writel(ADC_V2_CON1_SOFT_RESET, &regs->con1);

	/* Disable INT - will read status only */
	writel(0x0, &regs->int_en);

	/* CON2 - set conversion parameters */
	cfg = ADC_V2_CON2_C_TIME(3); /* Conversion times: (1 << 3) = 8 */
	cfg |= ADC_V2_CON2_OSEL(OSEL_BINARY);
	cfg |= ADC_V2_CON2_ESEL(ESEL_ADC_EVAL_TIME_20CLK);
	cfg |= ADC_V2_CON2_HIGHF(HIGHF_CONV_RATE_600KSPS);
	writel(cfg, &regs->con2);

	priv->active_channel = -1;

	return 0;
}

int exynos_adc_ofdata_to_platdata(struct udevice *dev)
{
	struct adc_uclass_platdata *uc_pdata = dev_get_uclass_platdata(dev);
	struct exynos_adc_priv *priv = dev_get_priv(dev);

	priv->regs = (struct exynos_adc_v2 *)devfdt_get_addr(dev);
	if (priv->regs == (struct exynos_adc_v2 *)FDT_ADDR_T_NONE) {
		pr_err("Dev: %s - can't get address!", dev->name);
		return -ENODATA;
	}

	uc_pdata->data_mask = ADC_V2_DAT_MASK;
	uc_pdata->data_format = ADC_DATA_FORMAT_BIN;
	uc_pdata->data_timeout_us = ADC_V2_CONV_TIMEOUT_US;

	/* Mask available channel bits: [0:9] */
	uc_pdata->channel_mask = (2 << ADC_V2_MAX_CHANNEL) - 1;

	return 0;
}

static const struct adc_ops exynos_adc_ops = {
	.start_channel = exynos_adc_start_channel,
	.channel_data = exynos_adc_channel_data,
	.stop = exynos_adc_stop,
};

static const struct udevice_id exynos_adc_ids[] = {
	{ .compatible = "samsung,exynos-adc-v2" },
	{ }
};

U_BOOT_DRIVER(exynos_adc) = {
	.name		= "exynos-adc",
	.id		= UCLASS_ADC,
	.of_match	= exynos_adc_ids,
	.ops		= &exynos_adc_ops,
	.probe		= exynos_adc_probe,
	.ofdata_to_platdata = exynos_adc_ofdata_to_platdata,
	.priv_auto_alloc_size = sizeof(struct exynos_adc_priv),
};
