// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2018, STMicroelectronics - All Rights Reserved
 * Author: Fabrice Gasnier <fabrice.gasnier@st.com>
 *
 * Originally based on the Linux kernel v4.18 drivers/iio/adc/stm32-adc.c.
 */

#include <common.h>
#include <adc.h>
#include <asm/io.h>
#include <linux/iopoll.h>
#include "stm32-adc-core.h"

/* STM32H7 - Registers for each ADC instance */
#define STM32H7_ADC_ISR			0x00
#define STM32H7_ADC_CR			0x08
#define STM32H7_ADC_CFGR		0x0C
#define STM32H7_ADC_SMPR1		0x14
#define STM32H7_ADC_SMPR2		0x18
#define STM32H7_ADC_PCSEL		0x1C
#define STM32H7_ADC_SQR1		0x30
#define STM32H7_ADC_DR			0x40
#define STM32H7_ADC_DIFSEL		0xC0

/* STM32H7_ADC_ISR - bit fields */
#define STM32MP1_VREGREADY		BIT(12)
#define STM32H7_EOC			BIT(2)
#define STM32H7_ADRDY			BIT(0)

/* STM32H7_ADC_CR - bit fields */
#define STM32H7_DEEPPWD			BIT(29)
#define STM32H7_ADVREGEN		BIT(28)
#define STM32H7_BOOST			BIT(8)
#define STM32H7_ADSTART			BIT(2)
#define STM32H7_ADDIS			BIT(1)
#define STM32H7_ADEN			BIT(0)

/* STM32H7_ADC_CFGR bit fields */
#define STM32H7_EXTEN			GENMASK(11, 10)
#define STM32H7_DMNGT			GENMASK(1, 0)

/* STM32H7_ADC_SQR1 - bit fields */
#define STM32H7_SQ1_SHIFT		6

/* BOOST bit must be set on STM32H7 when ADC clock is above 20MHz */
#define STM32H7_BOOST_CLKRATE		20000000UL

#define STM32_ADC_CH_MAX		20	/* max number of channels */
#define STM32_ADC_TIMEOUT_US		100000

struct stm32_adc_cfg {
	unsigned int max_channels;
	unsigned int num_bits;
	bool has_vregready;
};

struct stm32_adc {
	void __iomem *regs;
	int active_channel;
	const struct stm32_adc_cfg *cfg;
};

static int stm32_adc_stop(struct udevice *dev)
{
	struct stm32_adc *adc = dev_get_priv(dev);

	setbits_le32(adc->regs + STM32H7_ADC_CR, STM32H7_ADDIS);
	clrbits_le32(adc->regs + STM32H7_ADC_CR, STM32H7_BOOST);
	/* Setting DEEPPWD disables ADC vreg and clears ADVREGEN */
	setbits_le32(adc->regs + STM32H7_ADC_CR, STM32H7_DEEPPWD);
	adc->active_channel = -1;

	return 0;
}

static int stm32_adc_start_channel(struct udevice *dev, int channel)
{
	struct adc_uclass_platdata *uc_pdata = dev_get_uclass_platdata(dev);
	struct stm32_adc_common *common = dev_get_priv(dev_get_parent(dev));
	struct stm32_adc *adc = dev_get_priv(dev);
	int ret;
	u32 val;

	/* Exit deep power down, then enable ADC voltage regulator */
	clrbits_le32(adc->regs + STM32H7_ADC_CR, STM32H7_DEEPPWD);
	setbits_le32(adc->regs + STM32H7_ADC_CR, STM32H7_ADVREGEN);
	if (common->rate > STM32H7_BOOST_CLKRATE)
		setbits_le32(adc->regs + STM32H7_ADC_CR, STM32H7_BOOST);

	/* Wait for startup time */
	if (!adc->cfg->has_vregready) {
		udelay(20);
	} else {
		ret = readl_poll_timeout(adc->regs + STM32H7_ADC_ISR, val,
					 val & STM32MP1_VREGREADY,
					 STM32_ADC_TIMEOUT_US);
		if (ret < 0) {
			stm32_adc_stop(dev);
			dev_err(dev, "Failed to enable vreg: %d\n", ret);
			return ret;
		}
	}

	/* Only use single ended channels */
	writel(0, adc->regs + STM32H7_ADC_DIFSEL);

	/* Enable ADC, Poll for ADRDY to be set (after adc startup time) */
	setbits_le32(adc->regs + STM32H7_ADC_CR, STM32H7_ADEN);
	ret = readl_poll_timeout(adc->regs + STM32H7_ADC_ISR, val,
				 val & STM32H7_ADRDY, STM32_ADC_TIMEOUT_US);
	if (ret < 0) {
		stm32_adc_stop(dev);
		dev_err(dev, "Failed to enable ADC: %d\n", ret);
		return ret;
	}

	/* Preselect channels */
	writel(uc_pdata->channel_mask, adc->regs + STM32H7_ADC_PCSEL);

	/* Set sampling time to max value by default */
	writel(0xffffffff, adc->regs + STM32H7_ADC_SMPR1);
	writel(0xffffffff, adc->regs + STM32H7_ADC_SMPR2);

	/* Program regular sequence: chan in SQ1 & len = 0 for one channel */
	writel(channel << STM32H7_SQ1_SHIFT, adc->regs + STM32H7_ADC_SQR1);

	/* Trigger detection disabled (conversion can be launched in SW) */
	clrbits_le32(adc->regs + STM32H7_ADC_CFGR, STM32H7_EXTEN |
		     STM32H7_DMNGT);
	adc->active_channel = channel;

	return 0;
}

static int stm32_adc_channel_data(struct udevice *dev, int channel,
				  unsigned int *data)
{
	struct stm32_adc *adc = dev_get_priv(dev);
	int ret;
	u32 val;

	if (channel != adc->active_channel) {
		dev_err(dev, "Requested channel is not active!\n");
		return -EINVAL;
	}

	setbits_le32(adc->regs + STM32H7_ADC_CR, STM32H7_ADSTART);
	ret = readl_poll_timeout(adc->regs + STM32H7_ADC_ISR, val,
				 val & STM32H7_EOC, STM32_ADC_TIMEOUT_US);
	if (ret < 0) {
		dev_err(dev, "conversion timed out: %d\n", ret);
		return ret;
	}

	*data = readl(adc->regs + STM32H7_ADC_DR);

	return 0;
}

static int stm32_adc_chan_of_init(struct udevice *dev)
{
	struct adc_uclass_platdata *uc_pdata = dev_get_uclass_platdata(dev);
	struct stm32_adc *adc = dev_get_priv(dev);
	u32 chans[STM32_ADC_CH_MAX];
	int i, num_channels, ret;

	/* Retrieve single ended channels listed in device tree */
	num_channels = dev_read_size(dev, "st,adc-channels");
	if (num_channels < 0) {
		dev_err(dev, "can't get st,adc-channels: %d\n", num_channels);
		return num_channels;
	}
	num_channels /= sizeof(u32);

	if (num_channels > adc->cfg->max_channels) {
		dev_err(dev, "too many st,adc-channels: %d\n", num_channels);
		return -EINVAL;
	}

	ret = dev_read_u32_array(dev, "st,adc-channels", chans, num_channels);
	if (ret < 0) {
		dev_err(dev, "can't read st,adc-channels: %d\n", ret);
		return ret;
	}

	for (i = 0; i < num_channels; i++) {
		if (chans[i] >= adc->cfg->max_channels) {
			dev_err(dev, "bad channel %u\n", chans[i]);
			return -EINVAL;
		}
		uc_pdata->channel_mask |= 1 << chans[i];
	}

	uc_pdata->data_mask = (1 << adc->cfg->num_bits) - 1;
	uc_pdata->data_format = ADC_DATA_FORMAT_BIN;
	uc_pdata->data_timeout_us = 100000;

	return 0;
}

static int stm32_adc_probe(struct udevice *dev)
{
	struct adc_uclass_platdata *uc_pdata = dev_get_uclass_platdata(dev);
	struct stm32_adc_common *common = dev_get_priv(dev_get_parent(dev));
	struct stm32_adc *adc = dev_get_priv(dev);
	int offset;

	offset = dev_read_u32_default(dev, "reg", -ENODATA);
	if (offset < 0) {
		dev_err(dev, "Can't read reg property\n");
		return offset;
	}
	adc->regs = common->base + offset;
	adc->cfg = (const struct stm32_adc_cfg *)dev_get_driver_data(dev);

	/* VDD supplied by common vref pin */
	uc_pdata->vdd_supply = common->vref;
	uc_pdata->vdd_microvolts = common->vref_uv;
	uc_pdata->vss_microvolts = 0;

	return stm32_adc_chan_of_init(dev);
}

static const struct adc_ops stm32_adc_ops = {
	.start_channel = stm32_adc_start_channel,
	.channel_data = stm32_adc_channel_data,
	.stop = stm32_adc_stop,
};

static const struct stm32_adc_cfg stm32h7_adc_cfg = {
	.num_bits = 16,
	.max_channels = STM32_ADC_CH_MAX,
};

static const struct stm32_adc_cfg stm32mp1_adc_cfg = {
	.num_bits = 16,
	.max_channels = STM32_ADC_CH_MAX,
	.has_vregready = true,
};

static const struct udevice_id stm32_adc_ids[] = {
	{ .compatible = "st,stm32h7-adc",
	  .data = (ulong)&stm32h7_adc_cfg },
	{ .compatible = "st,stm32mp1-adc",
	  .data = (ulong)&stm32mp1_adc_cfg },
	{}
};

U_BOOT_DRIVER(stm32_adc) = {
	.name  = "stm32-adc",
	.id = UCLASS_ADC,
	.of_match = stm32_adc_ids,
	.probe = stm32_adc_probe,
	.ops = &stm32_adc_ops,
	.priv_auto_alloc_size = sizeof(struct stm32_adc),
};
