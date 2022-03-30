// SPDX-License-Identifier: GPL-2.0+
/*
 * max98090.c -- MAX98090 ALSA SoC Audio driver
 *
 * Copyright 2011 Maxim Integrated Products
 */

#include <common.h>
#include <audio_codec.h>
#include <div64.h>
#include <dm.h>
#include <i2c.h>
#include <i2s.h>
#include <sound.h>
#include <asm/gpio.h>
#include "maxim_codec.h"
#include "max98090.h"

/*
 * Sets hw params for max98090
 *
 * @priv: max98090 information pointer
 * @rate: Sampling rate
 * @bits_per_sample: Bits per sample
 *
 * @return -EIO for error, 0 for success.
 */
int max98090_hw_params(struct maxim_priv *priv, unsigned int rate,
		       unsigned int bits_per_sample)
{
	int error;
	unsigned char value;

	switch (bits_per_sample) {
	case 16:
		maxim_i2c_read(priv, M98090_REG_INTERFACE_FORMAT, &value);
		error = maxim_bic_or(priv, M98090_REG_INTERFACE_FORMAT,
				     M98090_WS_MASK, 0);
		maxim_i2c_read(priv, M98090_REG_INTERFACE_FORMAT, &value);
		break;
	default:
		debug("%s: Illegal bits per sample %d.\n",
		      __func__, bits_per_sample);
		return -1;
	}

	/* Update filter mode */
	if (rate < 240000)
		error |= maxim_bic_or(priv, M98090_REG_FILTER_CONFIG,
				      M98090_MODE_MASK, 0);
	else
		error |= maxim_bic_or(priv, M98090_REG_FILTER_CONFIG,
				      M98090_MODE_MASK, M98090_MODE_MASK);

	/* Update sample rate mode */
	if (rate < 50000)
		error |= maxim_bic_or(priv, M98090_REG_FILTER_CONFIG,
				      M98090_DHF_MASK, 0);
	else
		error |= maxim_bic_or(priv, M98090_REG_FILTER_CONFIG,
				      M98090_DHF_MASK, M98090_DHF_MASK);

	if (error < 0) {
		debug("%s: Error setting hardware params.\n", __func__);
		return -EIO;
	}
	priv->rate = rate;

	return 0;
}

/*
 * Configures Audio interface system clock for the given frequency
 *
 * @priv: max98090 information
 * @freq: Sampling frequency in Hz
 *
 * @return -EIO for error, 0 for success.
 */
int max98090_set_sysclk(struct maxim_priv *priv, unsigned int freq)
{
	int error = 0;

	/* Requested clock frequency is already setup */
	if (freq == priv->sysclk)
		return 0;

	/* Setup clocks for slave mode, and using the PLL
	 * PSCLK = 0x01 (when master clk is 10MHz to 20MHz)
	 *	0x02 (when master clk is 20MHz to 40MHz)..
	 *	0x03 (when master clk is 40MHz to 60MHz)..
	 */
	if (freq >= 10000000 && freq < 20000000) {
		error = maxim_i2c_write(priv, M98090_REG_SYSTEM_CLOCK,
					M98090_PSCLK_DIV1);
	} else if (freq >= 20000000 && freq < 40000000) {
		error = maxim_i2c_write(priv, M98090_REG_SYSTEM_CLOCK,
					M98090_PSCLK_DIV2);
	} else if (freq >= 40000000 && freq < 60000000) {
		error = maxim_i2c_write(priv, M98090_REG_SYSTEM_CLOCK,
					M98090_PSCLK_DIV4);
	} else {
		debug("%s: Invalid master clock frequency\n", __func__);
		return -1;
	}

	debug("%s: Clock at %uHz\n", __func__, freq);

	if (error < 0)
		return -1;

	priv->sysclk = freq;

	return 0;
}

/*
 * Sets Max98090 I2S format
 *
 * @priv: max98090 information
 * @fmt: i2S format - supports a subset of the options defined in i2s.h.
 *
 * @return -EIO for error, 0 for success.
 */
int max98090_set_fmt(struct maxim_priv *priv, int fmt)
{
	u8 regval = 0;
	int error = 0;

	if (fmt == priv->fmt)
		return 0;

	priv->fmt = fmt;

	switch (fmt & SND_SOC_DAIFMT_MASTER_MASK) {
	case SND_SOC_DAIFMT_CBS_CFS:
		/* Set to slave mode PLL - MAS mode off */
		error |= maxim_i2c_write(priv, M98090_REG_CLOCK_RATIO_NI_MSB,
					 0x00);
		error |= maxim_i2c_write(priv, M98090_REG_CLOCK_RATIO_NI_LSB,
					 0x00);
		error |= maxim_bic_or(priv, M98090_REG_CLOCK_MODE,
				      M98090_USE_M1_MASK, 0);
		break;
	case SND_SOC_DAIFMT_CBM_CFM:
		/* Set to master mode */
		debug("Master mode not supported\n");
		break;
	case SND_SOC_DAIFMT_CBS_CFM:
	case SND_SOC_DAIFMT_CBM_CFS:
	default:
		debug("%s: Clock mode unsupported\n", __func__);
		return -EINVAL;
	}

	error |= maxim_i2c_write(priv, M98090_REG_MASTER_MODE, regval);

	regval = 0;
	switch (fmt & SND_SOC_DAIFMT_FORMAT_MASK) {
	case SND_SOC_DAIFMT_I2S:
		regval |= M98090_DLY_MASK;
		break;
	case SND_SOC_DAIFMT_LEFT_J:
		break;
	case SND_SOC_DAIFMT_RIGHT_J:
		regval |= M98090_RJ_MASK;
		break;
	case SND_SOC_DAIFMT_DSP_A:
		/* Not supported mode */
	default:
		debug("%s: Unrecognized format.\n", __func__);
		return -EINVAL;
	}

	switch (fmt & SND_SOC_DAIFMT_INV_MASK) {
	case SND_SOC_DAIFMT_NB_NF:
		break;
	case SND_SOC_DAIFMT_NB_IF:
		regval |= M98090_WCI_MASK;
		break;
	case SND_SOC_DAIFMT_IB_NF:
		regval |= M98090_BCI_MASK;
		break;
	case SND_SOC_DAIFMT_IB_IF:
		regval |= M98090_BCI_MASK | M98090_WCI_MASK;
		break;
	default:
		debug("%s: Unrecognized inversion settings.\n", __func__);
		return -EINVAL;
	}

	error |= maxim_i2c_write(priv, M98090_REG_INTERFACE_FORMAT, regval);

	if (error < 0) {
		debug("%s: Error setting i2s format.\n", __func__);
		return -EIO;
	}

	return 0;
}

/*
 * resets the audio codec
 *
 * @priv: max98090 information
 * @return -EIO for error, 0 for success.
 */
static int max98090_reset(struct maxim_priv *priv)
{
	int ret;

	/*
	 * Gracefully reset the DSP core and the codec hardware in a proper
	 * sequence.
	 */
	ret = maxim_i2c_write(priv, M98090_REG_SOFTWARE_RESET,
			      M98090_SWRESET_MASK);
	if (ret != 0) {
		debug("%s: Failed to reset DSP: %d\n", __func__, ret);
		return ret;
	}
	mdelay(20);

	return 0;
}

/*
 * Initialise max98090 codec device
 *
 * @priv: max98090 information
 *
 * @return -EIO for error, 0 for success.
 */
int max98090_device_init(struct maxim_priv *priv)
{
	unsigned char id;
	int error = 0;

	/* reset the codec, the DSP core, and disable all interrupts */
	error = max98090_reset(priv);
	if (error != 0) {
		debug("Reset\n");
		return error;
	}

	/* initialize private data */
	priv->sysclk = -1U;
	priv->rate = -1U;
	priv->fmt = -1U;

	error = maxim_i2c_read(priv, M98090_REG_REVISION_ID, &id);
	if (error < 0) {
		debug("%s: Failure reading hardware revision: %d\n",
		      __func__, id);
		return -EIO;
	}
	debug("%s: Hardware revision: %d\n", __func__, id);

	return 0;
}

static int max98090_setup_interface(struct maxim_priv *priv)
{
	unsigned char id;
	int error;

	/* Reading interrupt status to clear them */
	error = maxim_i2c_read(priv, M98090_REG_DEVICE_STATUS, &id);

	error |= maxim_i2c_write(priv, M98090_REG_DAC_CONTROL,
				 M98090_DACHP_MASK);
	error |= maxim_i2c_write(priv, M98090_REG_BIAS_CONTROL,
				 M98090_VCM_MODE_MASK);

	error |= maxim_i2c_write(priv, M98090_REG_LEFT_SPK_MIXER, 0x1);
	error |= maxim_i2c_write(priv, M98090_REG_RIGHT_SPK_MIXER, 0x2);

	error |= maxim_i2c_write(priv, M98090_REG_LEFT_SPK_VOLUME, 0x25);
	error |= maxim_i2c_write(priv, M98090_REG_RIGHT_SPK_VOLUME, 0x25);

	error |= maxim_i2c_write(priv, M98090_REG_CLOCK_RATIO_NI_MSB, 0x0);
	error |= maxim_i2c_write(priv, M98090_REG_CLOCK_RATIO_NI_LSB, 0x0);
	error |= maxim_i2c_write(priv, M98090_REG_MASTER_MODE, 0x0);
	error |= maxim_i2c_write(priv, M98090_REG_INTERFACE_FORMAT, 0x0);
	error |= maxim_i2c_write(priv, M98090_REG_IO_CONFIGURATION,
				 M98090_SDIEN_MASK);
	error |= maxim_i2c_write(priv, M98090_REG_DEVICE_SHUTDOWN,
				 M98090_SHDNN_MASK);
	error |= maxim_i2c_write(priv, M98090_REG_OUTPUT_ENABLE,
				 M98090_HPREN_MASK | M98090_HPLEN_MASK |
				 M98090_SPREN_MASK | M98090_SPLEN_MASK |
				 M98090_DAREN_MASK | M98090_DALEN_MASK);
	error |= maxim_i2c_write(priv, M98090_REG_IO_CONFIGURATION,
				 M98090_SDOEN_MASK | M98090_SDIEN_MASK);

	if (error < 0)
		return -EIO;

	return 0;
}

static int max98090_do_init(struct maxim_priv *priv, int sampling_rate,
			    int mclk_freq, int bits_per_sample)
{
	int ret = 0;

	ret = max98090_setup_interface(priv);
	if (ret < 0) {
		debug("%s: max98090 setup interface failed\n", __func__);
		return ret;
	}

	ret = max98090_set_sysclk(priv, mclk_freq);
	if (ret < 0) {
		debug("%s: max98090 codec set sys clock failed\n", __func__);
		return ret;
	}

	ret = max98090_hw_params(priv, sampling_rate, bits_per_sample);

	if (ret == 0) {
		ret = max98090_set_fmt(priv, SND_SOC_DAIFMT_I2S |
				       SND_SOC_DAIFMT_NB_NF |
				       SND_SOC_DAIFMT_CBS_CFS);
	}

	return ret;
}

static int max98090_set_params(struct udevice *dev, int interface, int rate,
			       int mclk_freq, int bits_per_sample,
			       uint channels)
{
	struct maxim_priv *priv = dev_get_priv(dev);

	return max98090_do_init(priv, rate, mclk_freq, bits_per_sample);
}

static int max98090_probe(struct udevice *dev)
{
	struct maxim_priv *priv = dev_get_priv(dev);
	int ret;

	priv->dev = dev;
	ret = max98090_device_init(priv);
	if (ret < 0) {
		debug("%s: max98090 codec chip init failed\n", __func__);
		return ret;
	}

	return 0;
}

static const struct audio_codec_ops max98090_ops = {
	.set_params	= max98090_set_params,
};

static const struct udevice_id max98090_ids[] = {
	{ .compatible = "maxim,max98090" },
	{ }
};

U_BOOT_DRIVER(max98090) = {
	.name		= "max98090",
	.id		= UCLASS_AUDIO_CODEC,
	.of_match	= max98090_ids,
	.probe		= max98090_probe,
	.ops		= &max98090_ops,
	.priv_auto_alloc_size	= sizeof(struct maxim_priv),
};
