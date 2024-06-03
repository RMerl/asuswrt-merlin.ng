// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2015 Samsung Electronics
 * Przemyslaw Marczak <p.marczak@samsung.com>
 */
#include <common.h>
#include <errno.h>
#include <dm.h>
#include <adc.h>
#include <sandbox-adc.h>

/**
 * struct sandbox_adc_priv - sandbox ADC device's operation status and data
 *
 * @conversion_status - conversion status: ACTIVE (started) / INACTIVE (stopped)
 * @conversion_mode   - conversion mode: single or multi-channel
 * @active_channel    - active channel number, valid for single channel mode
 * data[]             - channels data
 */
struct sandbox_adc_priv {
	int conversion_status;
	int conversion_mode;
	int active_channel_mask;
	unsigned int data[4];
};

int sandbox_adc_start_channel(struct udevice *dev, int channel)
{
	struct sandbox_adc_priv *priv = dev_get_priv(dev);

	/* Set single-channel mode */
	priv->conversion_mode = SANDBOX_ADC_MODE_SINGLE_CHANNEL;
	/* Select channel */
	priv->active_channel_mask = 1 << channel;
	/* Start conversion */
	priv->conversion_status = SANDBOX_ADC_ACTIVE;

	return 0;
}

int sandbox_adc_start_channels(struct udevice *dev, unsigned int channel_mask)
{
	struct sandbox_adc_priv *priv = dev_get_priv(dev);

	/* Set single-channel mode */
	priv->conversion_mode = SANDBOX_ADC_MODE_MULTI_CHANNEL;
	/* Select channel */
	priv->active_channel_mask = channel_mask;
	/* Start conversion */
	priv->conversion_status = SANDBOX_ADC_ACTIVE;

	return 0;
}

int sandbox_adc_channel_data(struct udevice *dev, int channel,
			     unsigned int *data)
{
	struct sandbox_adc_priv *priv = dev_get_priv(dev);

	/* For single-channel conversion mode, check if channel was selected */
	if ((priv->conversion_mode == SANDBOX_ADC_MODE_SINGLE_CHANNEL) &&
	    !(priv->active_channel_mask & (1 << channel))) {
		pr_err("Request for an inactive channel!");
		return -EINVAL;
	}

	/* The conversion must be started before reading the data */
	if (priv->conversion_status == SANDBOX_ADC_INACTIVE)
		return -EIO;

	*data = priv->data[channel];

	return 0;
}

int sandbox_adc_channels_data(struct udevice *dev, unsigned int channel_mask,
			      struct adc_channel *channels)
{
	struct sandbox_adc_priv *priv = dev_get_priv(dev);
	int i;

	/* Return error for single-channel conversion mode */
	if (priv->conversion_mode == SANDBOX_ADC_MODE_SINGLE_CHANNEL) {
		pr_err("ADC in single-channel mode!");
		return -EPERM;
	}
	/* Check channel selection */
	if (!(priv->active_channel_mask & channel_mask)) {
		pr_err("Request for an inactive channel!");
		return -EINVAL;
	}
	/* The conversion must be started before reading the data */
	if (priv->conversion_status == SANDBOX_ADC_INACTIVE)
		return -EIO;

	for (i = 0; i < SANDBOX_ADC_CHANNELS; i++) {
		if (!((channel_mask >> i) & 0x1))
			continue;

		channels->data = priv->data[i];
		channels->id = i;
		channels++;
	}

	return 0;
}

int sandbox_adc_stop(struct udevice *dev)
{
	struct sandbox_adc_priv *priv = dev_get_priv(dev);

	/* Start conversion */
	priv->conversion_status = SANDBOX_ADC_INACTIVE;

	return 0;
}

int sandbox_adc_probe(struct udevice *dev)
{
	struct sandbox_adc_priv *priv = dev_get_priv(dev);

	/* Stop conversion */
	priv->conversion_status = SANDBOX_ADC_INACTIVE;
	/* Set single-channel mode */
	priv->conversion_mode = SANDBOX_ADC_MODE_SINGLE_CHANNEL;
	/* Deselect all channels */
	priv->active_channel_mask = 0;

	/* Set sandbox test data */
	priv->data[0] = SANDBOX_ADC_CHANNEL0_DATA;
	priv->data[1] = SANDBOX_ADC_CHANNEL1_DATA;
	priv->data[2] = SANDBOX_ADC_CHANNEL2_DATA;
	priv->data[3] = SANDBOX_ADC_CHANNEL3_DATA;

	return 0;
}

int sandbox_adc_ofdata_to_platdata(struct udevice *dev)
{
	struct adc_uclass_platdata *uc_pdata = dev_get_uclass_platdata(dev);

	uc_pdata->data_mask = SANDBOX_ADC_DATA_MASK;
	uc_pdata->data_format = ADC_DATA_FORMAT_BIN;
	uc_pdata->data_timeout_us = 0;

	/* Mask available channel bits: [0:3] */
	uc_pdata->channel_mask = (1 << SANDBOX_ADC_CHANNELS) - 1;

	return 0;
}

static const struct adc_ops sandbox_adc_ops = {
	.start_channel = sandbox_adc_start_channel,
	.start_channels = sandbox_adc_start_channels,
	.channel_data = sandbox_adc_channel_data,
	.channels_data = sandbox_adc_channels_data,
	.stop = sandbox_adc_stop,
};

static const struct udevice_id sandbox_adc_ids[] = {
	{ .compatible = "sandbox,adc" },
	{ }
};

U_BOOT_DRIVER(sandbox_adc) = {
	.name		= "sandbox-adc",
	.id		= UCLASS_ADC,
	.of_match	= sandbox_adc_ids,
	.ops		= &sandbox_adc_ops,
	.probe		= sandbox_adc_probe,
	.ofdata_to_platdata = sandbox_adc_ofdata_to_platdata,
	.priv_auto_alloc_size = sizeof(struct sandbox_adc_priv),
};
