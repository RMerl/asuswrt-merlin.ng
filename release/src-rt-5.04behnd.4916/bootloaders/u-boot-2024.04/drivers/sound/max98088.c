// SPDX-License-Identifier: GPL-2.0+
/*
 * max98088.c -- MAX98088 ALSA SoC Audio driver
 *
 * Copyright 2010 Maxim Integrated Products
 *
 * Modified for U-Boot by Chih-Chung Chang (chihchung@chromium.org),
 * following the changes made in max98095.c
 */

#include <common.h>
#include <audio_codec.h>
#include <div64.h>
#include <dm.h>
#include <i2c.h>
#include <i2s.h>
#include <log.h>
#include <sound.h>
#include <asm/gpio.h>
#include "maxim_codec.h"
#include "max98088.h"

/* codec mclk clock divider coefficients. Index 0 is reserved. */
static const int rate_table[] = {0, 8000, 11025, 16000, 22050, 24000, 32000,
				 44100, 48000, 88200, 96000};

/*
 * codec mclk clock divider coefficients based on sampling rate
 *
 * @param rate sampling rate
 * @param value address of indexvalue to be stored
 *
 * Return:	0 for success or negative error code.
 */
static int rate_value(int rate, u8 *value)
{
	int i;

	for (i = 1; i < ARRAY_SIZE(rate_table); i++) {
		if (rate_table[i] >= rate) {
			*value = i;
			return 0;
		}
	}
	*value = 1;

	return -EINVAL;
}

/*
 * Sets hw params for max98088
 *
 * @priv: max98088 information pointer
 * @rate: Sampling rate
 * @bits_per_sample: Bits per sample
 *
 * Return: -EIO for error, 0 for success.
 */
int max98088_hw_params(struct maxim_priv *priv, unsigned int rate,
		       unsigned int bits_per_sample)
{
	int error;
	u8 regval;

	switch (bits_per_sample) {
	case 16:
		error = maxim_bic_or(priv, M98088_REG_DAI1_FORMAT,
				     M98088_DAI_WS, 0);
		break;
	case 24:
		error = maxim_bic_or(priv, M98088_REG_DAI1_FORMAT,
				     M98088_DAI_WS, M98088_DAI_WS);
		break;
	default:
		debug("%s: Illegal bits per sample %d.\n",
		      __func__, bits_per_sample);
		return -EINVAL;
	}

	error |= maxim_bic_or(priv, M98088_REG_PWR_SYS, M98088_SHDNRUN, 0);

	if (rate_value(rate, &regval)) {
		debug("%s: Failed to set sample rate to %d.\n",
		      __func__, rate);
		return -EIO;
	}

	error |= maxim_bic_or(priv, M98088_REG_DAI1_CLKMODE,
			      M98088_CLKMODE_MASK, regval << 4);
	priv->rate = rate;

	/* Update sample rate mode */
	if (rate < 50000)
		error |= maxim_bic_or(priv, M98088_REG_DAI1_FILTERS,
				      M98088_DAI_DHF, 0);
	else
		error |= maxim_bic_or(priv, M98088_REG_DAI1_FILTERS,
				      M98088_DAI_DHF, M98088_DAI_DHF);

	error |= maxim_bic_or(priv, M98088_REG_PWR_SYS, M98088_SHDNRUN,
			      M98088_SHDNRUN);

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
 * @priv: max98088 information
 * @freq: Sampling frequency in Hz
 *
 * Return: -EIO for error, 0 for success.
 */
int max98088_set_sysclk(struct maxim_priv *priv, unsigned int freq)
{
	int error = 0;
	u8 pwr;

	/* Requested clock frequency is already setup */
	if (freq == priv->sysclk)
		return 0;

	/*
	 * Setup clocks for slave mode, and using the PLL
	 * PSCLK = 0x01 (when master clk is 10MHz to 20MHz)
	 *         0x02 (when master clk is 20MHz to 30MHz)..
	 */
	if (freq >= 10000000 && freq < 20000000) {
		error = maxim_i2c_write(priv, M98088_REG_SYS_CLK, 0x10);
	} else if ((freq >= 20000000) && (freq < 30000000)) {
		error = maxim_i2c_write(priv, M98088_REG_SYS_CLK, 0x20);
	} else {
		debug("%s: Invalid master clock frequency\n", __func__);
		return -EIO;
	}

	error |= maxim_i2c_read(priv, M98088_REG_PWR_SYS, &pwr);
	if (pwr & M98088_SHDNRUN) {
		error |= maxim_bic_or(priv, M98088_REG_PWR_SYS,
				      M98088_SHDNRUN, 0);
		error |= maxim_bic_or(priv, M98088_REG_PWR_SYS,
				      M98088_SHDNRUN, M98088_SHDNRUN);
	}

	debug("%s: Clock at %uHz\n", __func__, freq);
	if (error < 0)
		return -EIO;

	priv->sysclk = freq;

	return 0;
}

/*
 * Sets Max98090 I2S format
 *
 * @priv: max98088 information
 * @fmt: i2S format - supports a subset of the options defined in i2s.h.
 *
 * Return: -EIO for error, 0 for success.
 */
int max98088_set_fmt(struct maxim_priv *priv, int fmt)
{
	u8 reg15val;
	u8 reg14val = 0;
	int error = 0;

	if (fmt == priv->fmt)
		return 0;

	priv->fmt = fmt;

	switch (fmt & SND_SOC_DAIFMT_MASTER_MASK) {
	case SND_SOC_DAIFMT_CBS_CFS:
		/* Slave mode PLL */
		error |= maxim_i2c_write(priv, M98088_REG_DAI1_CLKCFG_HI,
					    0x80);
		error |= maxim_i2c_write(priv, M98088_REG_DAI1_CLKCFG_LO,
					    0x00);
		break;
	case SND_SOC_DAIFMT_CBM_CFM:
		/* Set to master mode */
		reg14val |= M98088_DAI_MAS;
		break;
	case SND_SOC_DAIFMT_CBS_CFM:
	case SND_SOC_DAIFMT_CBM_CFS:
	default:
		debug("%s: Clock mode unsupported\n", __func__);
		return -EINVAL;
	}

	switch (fmt & SND_SOC_DAIFMT_FORMAT_MASK) {
	case SND_SOC_DAIFMT_I2S:
		reg14val |= M98088_DAI_DLY;
		break;
	case SND_SOC_DAIFMT_LEFT_J:
		break;
	default:
		debug("%s: Unrecognized format.\n", __func__);
		return -EINVAL;
	}

	switch (fmt & SND_SOC_DAIFMT_INV_MASK) {
	case SND_SOC_DAIFMT_NB_NF:
		break;
	case SND_SOC_DAIFMT_NB_IF:
		reg14val |= M98088_DAI_WCI;
		break;
	case SND_SOC_DAIFMT_IB_NF:
		reg14val |= M98088_DAI_BCI;
		break;
	case SND_SOC_DAIFMT_IB_IF:
		reg14val |= M98088_DAI_BCI | M98088_DAI_WCI;
		break;
	default:
		debug("%s: Unrecognized inversion settings.\n",  __func__);
		return -EINVAL;
	}

	error |= maxim_bic_or(priv, M98088_REG_DAI1_FORMAT,
			      M98088_DAI_MAS | M98088_DAI_DLY | M98088_DAI_BCI |
			      M98088_DAI_WCI, reg14val);
	reg15val = M98088_DAI_BSEL64;
	error |= maxim_i2c_write(priv, M98088_REG_DAI1_CLOCK, reg15val);

	if (error < 0) {
		debug("%s: Error setting i2s format.\n", __func__);
		return -EIO;
	}

	return 0;
}

/*
 * max98088_reset() - reset the audio codec
 *
 * @priv: max98088 information
 * Return: -EIO for error, 0 for success.
 */
static int max98088_reset(struct maxim_priv *priv)
{
	int ret, i;
	u8 val;

	/*
	 * Reset to hardware default for registers, as there is not a soft
	 * reset hardware control register.
	 */
	for (i = M98088_REG_IRQ_ENABLE; i <= M98088_REG_PWR_SYS; i++) {
		switch (i) {
		case M98088_REG_BIAS_CNTL:
			val = 0xf0;
			break;
		case M98088_REG_DAC_BIAS2:
			val = 0x0f;
			break;
		default:
			val = 0;
		}
		ret = maxim_i2c_write(priv, i, val);
		if (ret < 0) {
			debug("%s: Failed to reset: %d\n", __func__, ret);
			return ret;
		}
	}

	return 0;
}

/**
 * max98088_device_init() - Initialise max98088 codec device
 *
 * @priv: max98088 information
 *
 * Return: -EIO for error, 0 for success.
 */
static int max98088_device_init(struct maxim_priv *priv)
{
	unsigned char id;
	int error = 0;

	/* reset the codec, the DSP core, and disable all interrupts */
	error = max98088_reset(priv);
	if (error != 0) {
		debug("Reset\n");
		return error;
	}

	/* initialize private data */
	priv->sysclk = -1U;
	priv->rate = -1U;
	priv->fmt = -1U;

	error = maxim_i2c_read(priv, M98088_REG_REV_ID, &id);
	if (error < 0) {
		debug("%s: Failure reading hardware revision: %d\n",
		      __func__, id);
		return -EIO;
	}
	debug("%s: Hardware revision: %d\n", __func__, id);

	return 0;
}

static int max98088_setup_interface(struct maxim_priv *priv)
{
	int error;

	/* Reading interrupt status to clear them */
	error = maxim_i2c_write(priv, M98088_REG_PWR_SYS, M98088_PWRSV);
	error |= maxim_i2c_write(priv, M98088_REG_IRQ_ENABLE, 0x00);

	/*
	 * initialize registers to hardware default configuring audio
	 * interface2 to DAI1
	 */
	error |= maxim_i2c_write(priv, M98088_REG_MIX_DAC,
				 M98088_DAI1L_TO_DACL | M98088_DAI1R_TO_DACR);
	error |= maxim_i2c_write(priv, M98088_REG_BIAS_CNTL, 0xF0);
	error |= maxim_i2c_write(priv, M98088_REG_DAC_BIAS2, 0x0F);
	error |= maxim_i2c_write(priv, M98088_REG_DAI1_IOCFG,
				 M98088_S2NORMAL | M98088_SDATA);

	/*
	 * route DACL and DACR output to headphone and speakers
	 * Ordering: DACL, DACR, DACL, DACR
	 */
	error |= maxim_i2c_write(priv, M98088_REG_MIX_SPK_LEFT, 1);
	error |= maxim_i2c_write(priv, M98088_REG_MIX_SPK_RIGHT, 1);
	error |= maxim_i2c_write(priv, M98088_REG_MIX_HP_LEFT, 1);
	error |= maxim_i2c_write(priv, M98088_REG_MIX_HP_RIGHT, 1);

	/* set volume: -12db */
	error |= maxim_i2c_write(priv, M98088_REG_LVL_SPK_L, 0x0f);
	error |= maxim_i2c_write(priv, M98088_REG_LVL_SPK_R, 0x0f);

	/* set volume: -22db */
	error |= maxim_i2c_write(priv, M98088_REG_LVL_HP_L, 0x0d);
	error |= maxim_i2c_write(priv, M98088_REG_LVL_HP_R, 0x0d);

	/* power enable */
	error |= maxim_i2c_write(priv, M98088_REG_PWR_EN_OUT,
				 M98088_HPLEN | M98088_HPREN | M98088_SPLEN |
				 M98088_SPREN | M98088_DALEN | M98088_DAREN);
	if (error < 0)
		return -EIO;

	return 0;
}

static int max98088_do_init(struct maxim_priv *priv, int sampling_rate,
			    int mclk_freq, int bits_per_sample)
{
	int ret = 0;

	ret = max98088_setup_interface(priv);
	if (ret < 0) {
		debug("%s: max98088 setup interface failed\n", __func__);
		return ret;
	}

	ret = max98088_set_sysclk(priv, mclk_freq);
	if (ret < 0) {
		debug("%s: max98088 codec set sys clock failed\n", __func__);
		return ret;
	}

	ret = max98088_hw_params(priv, sampling_rate, bits_per_sample);

	if (ret == 0) {
		ret = max98088_set_fmt(priv, SND_SOC_DAIFMT_I2S |
				       SND_SOC_DAIFMT_NB_NF |
				       SND_SOC_DAIFMT_CBS_CFS);
	}

	return ret;
}

static int max98088_set_params(struct udevice *dev, int interface, int rate,
			       int mclk_freq, int bits_per_sample,
			       uint channels)
{
	struct maxim_priv *priv = dev_get_priv(dev);

	return max98088_do_init(priv, rate, mclk_freq, bits_per_sample);
}

static int max98088_probe(struct udevice *dev)
{
	struct maxim_priv *priv = dev_get_priv(dev);
	int ret;

	priv->dev = dev;
	ret = max98088_device_init(priv);
	if (ret < 0) {
		debug("%s: max98088 codec chip init failed\n", __func__);
		return ret;
	}

	return 0;
}

static const struct audio_codec_ops max98088_ops = {
	.set_params	= max98088_set_params,
};

static const struct udevice_id max98088_ids[] = {
	{ .compatible = "maxim,max98088" },
	{ }
};

U_BOOT_DRIVER(max98088) = {
	.name		= "max98088",
	.id		= UCLASS_AUDIO_CODEC,
	.of_match	= max98088_ids,
	.probe		= max98088_probe,
	.ops		= &max98088_ops,
	.priv_auto	= sizeof(struct maxim_priv),
};
