// SPDX-License-Identifier: GPL-2.0+
/*
 * max98095.c -- MAX98095 ALSA SoC Audio driver
 *
 * Copyright 2011 Maxim Integrated Products
 *
 * Modified for U-Boot by R. Chandrasekar (rcsekar@samsung.com)
 */

#include <common.h>
#include <audio_codec.h>
#include <dm.h>
#include <div64.h>
#include <fdtdec.h>
#include <i2c.h>
#include <log.h>
#include <sound.h>
#include <asm/gpio.h>
#include "i2s.h"
#include "max98095.h"

/* Index 0 is reserved. */
int rate_table[] = {0, 8000, 11025, 16000, 22050, 24000, 32000, 44100, 48000,
		88200, 96000};

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
 * Sets hw params for max98095
 *
 * @param priv		max98095 information pointer
 * @param rate		Sampling rate
 * @param bits_per_sample	Bits per sample
 *
 * Return:	0 for success or negative error code.
 */
static int max98095_hw_params(struct maxim_priv *priv,
			      enum en_max_audio_interface aif_id,
			      unsigned int rate, unsigned int bits_per_sample)
{
	u8 regval;
	int error;
	unsigned short M98095_DAI_CLKMODE;
	unsigned short M98095_DAI_FORMAT;
	unsigned short M98095_DAI_FILTERS;

	if (aif_id == AIF1) {
		M98095_DAI_CLKMODE = M98095_027_DAI1_CLKMODE;
		M98095_DAI_FORMAT = M98095_02A_DAI1_FORMAT;
		M98095_DAI_FILTERS = M98095_02E_DAI1_FILTERS;
	} else {
		M98095_DAI_CLKMODE = M98095_031_DAI2_CLKMODE;
		M98095_DAI_FORMAT = M98095_034_DAI2_FORMAT;
		M98095_DAI_FILTERS = M98095_038_DAI2_FILTERS;
	}

	switch (bits_per_sample) {
	case 16:
		error = maxim_bic_or(priv, M98095_DAI_FORMAT, M98095_DAI_WS, 0);
		break;
	case 24:
		error = maxim_bic_or(priv, M98095_DAI_FORMAT, M98095_DAI_WS,
				     M98095_DAI_WS);
		break;
	default:
		debug("%s: Illegal bits per sample %d.\n",
		      __func__, bits_per_sample);
		return -EINVAL;
	}

	if (rate_value(rate, &regval)) {
		debug("%s: Failed to set sample rate to %d.\n",
		      __func__, rate);
		return -EINVAL;
	}
	priv->rate = rate;

	error |= maxim_bic_or(priv, M98095_DAI_CLKMODE, M98095_CLKMODE_MASK,
				 regval);

	/* Update sample rate mode */
	if (rate < 50000)
		error |= maxim_bic_or(priv, M98095_DAI_FILTERS,
					 M98095_DAI_DHF, 0);
	else
		error |= maxim_bic_or(priv, M98095_DAI_FILTERS,
					 M98095_DAI_DHF, M98095_DAI_DHF);

	if (error < 0) {
		debug("%s: Error setting hardware params.\n", __func__);
		return -EIO;
	}

	return 0;
}

/*
 * Configures Audio interface system clock for the given frequency
 *
 * @param priv		max98095 information
 * @param freq		Sampling frequency in Hz
 *
 * Return:	0 for success or negative error code.
 */
static int max98095_set_sysclk(struct maxim_priv *priv, unsigned int freq)
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
	if ((freq >= 10000000) && (freq < 20000000)) {
		error = maxim_i2c_write(priv, M98095_026_SYS_CLK, 0x10);
	} else if ((freq >= 20000000) && (freq < 40000000)) {
		error = maxim_i2c_write(priv, M98095_026_SYS_CLK, 0x20);
	} else if ((freq >= 40000000) && (freq < 60000000)) {
		error = maxim_i2c_write(priv, M98095_026_SYS_CLK, 0x30);
	} else {
		debug("%s: Invalid master clock frequency\n", __func__);
		return -EINVAL;
	}

	debug("%s: Clock at %uHz\n", __func__, freq);

	if (error < 0)
		return -EIO;

	priv->sysclk = freq;
	return 0;
}

/*
 * Sets Max98095 I2S format
 *
 * @param priv		max98095 information
 * @param fmt		i2S format - supports a subset of the options defined
 *			in i2s.h.
 *
 * Return:	0 for success or negative error code.
 */
static int max98095_set_fmt(struct maxim_priv *priv, int fmt,
			    enum en_max_audio_interface aif_id)
{
	u8 regval = 0;
	int error = 0;
	unsigned short M98095_DAI_CLKCFG_HI;
	unsigned short M98095_DAI_CLKCFG_LO;
	unsigned short M98095_DAI_FORMAT;
	unsigned short M98095_DAI_CLOCK;

	if (fmt == priv->fmt)
		return 0;

	priv->fmt = fmt;

	if (aif_id == AIF1) {
		M98095_DAI_CLKCFG_HI = M98095_028_DAI1_CLKCFG_HI;
		M98095_DAI_CLKCFG_LO = M98095_029_DAI1_CLKCFG_LO;
		M98095_DAI_FORMAT = M98095_02A_DAI1_FORMAT;
		M98095_DAI_CLOCK = M98095_02B_DAI1_CLOCK;
	} else {
		M98095_DAI_CLKCFG_HI = M98095_032_DAI2_CLKCFG_HI;
		M98095_DAI_CLKCFG_LO = M98095_033_DAI2_CLKCFG_LO;
		M98095_DAI_FORMAT = M98095_034_DAI2_FORMAT;
		M98095_DAI_CLOCK = M98095_035_DAI2_CLOCK;
	}

	switch (fmt & SND_SOC_DAIFMT_MASTER_MASK) {
	case SND_SOC_DAIFMT_CBS_CFS:
		/* Slave mode PLL */
		error |= maxim_i2c_write(priv, M98095_DAI_CLKCFG_HI, 0x80);
		error |= maxim_i2c_write(priv, M98095_DAI_CLKCFG_LO, 0x00);
		break;
	case SND_SOC_DAIFMT_CBM_CFM:
		/* Set to master mode */
		regval |= M98095_DAI_MAS;
		break;
	case SND_SOC_DAIFMT_CBS_CFM:
	case SND_SOC_DAIFMT_CBM_CFS:
	default:
		debug("%s: Clock mode unsupported\n", __func__);
		return -EINVAL;
	}

	switch (fmt & SND_SOC_DAIFMT_FORMAT_MASK) {
	case SND_SOC_DAIFMT_I2S:
		regval |= M98095_DAI_DLY;
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
		regval |= M98095_DAI_WCI;
		break;
	case SND_SOC_DAIFMT_IB_NF:
		regval |= M98095_DAI_BCI;
		break;
	case SND_SOC_DAIFMT_IB_IF:
		regval |= M98095_DAI_BCI | M98095_DAI_WCI;
		break;
	default:
		debug("%s: Unrecognized inversion settings.\n", __func__);
		return -EINVAL;
	}

	error |= maxim_bic_or(priv, M98095_DAI_FORMAT,
				 M98095_DAI_MAS | M98095_DAI_DLY |
				 M98095_DAI_BCI | M98095_DAI_WCI, regval);

	error |= maxim_i2c_write(priv, M98095_DAI_CLOCK, M98095_DAI_BSEL64);

	if (error < 0) {
		debug("%s: Error setting i2s format.\n", __func__);
		return -EIO;
	}

	return 0;
}

/*
 * resets the audio codec
 *
 * @param priv	Private data for driver
 * Return:	0 for success or negative error code.
 */
static int max98095_reset(struct maxim_priv *priv)
{
	int i, ret;

	/*
	 * Gracefully reset the DSP core and the codec hardware in a proper
	 * sequence.
	 */
	ret = maxim_i2c_write(priv, M98095_00F_HOST_CFG, 0);
	if (ret != 0) {
		debug("%s: Failed to reset DSP: %d\n", __func__, ret);
		return ret;
	}

	ret = maxim_i2c_write(priv, M98095_097_PWR_SYS, 0);
	if (ret != 0) {
		debug("%s: Failed to reset codec: %d\n", __func__, ret);
		return ret;
	}

	/*
	 * Reset to hardware default for registers, as there is not a soft
	 * reset hardware control register.
	 */
	for (i = M98095_010_HOST_INT_CFG; i < M98095_REG_MAX_CACHED; i++) {
		ret = maxim_i2c_write(priv, i, 0);
		if (ret < 0) {
			debug("%s: Failed to reset: %d\n", __func__, ret);
			return ret;
		}
	}

	return 0;
}

/*
 * Intialise max98095 codec device
 *
 * @param priv		max98095 information
 * Return:	0 for success or negative error code.
 */
static int max98095_device_init(struct maxim_priv *priv)
{
	unsigned char id;
	int ret;

	/* reset the codec, the DSP core, and disable all interrupts */
	ret = max98095_reset(priv);
	if (ret != 0) {
		debug("Reset\n");
		return ret;
	}

	/* initialize private data */
	priv->sysclk = -1U;
	priv->rate = -1U;
	priv->fmt = -1U;

	ret = maxim_i2c_read(priv, M98095_0FF_REV_ID, &id);
	if (ret < 0) {
		debug("%s: Failure reading hardware revision: %d\n",
		      __func__, id);
		return ret;
	}
	debug("%s: Hardware revision: %c\n", __func__, (id - 0x40) + 'A');

	return 0;
}

static int max98095_setup_interface(struct maxim_priv *priv,
				    enum en_max_audio_interface aif_id)
{
	int error;

	error = maxim_i2c_write(priv, M98095_097_PWR_SYS, M98095_PWRSV);

	/*
	 * initialize registers to hardware default configuring audio
	 * interface2 to DAC
	 */
	if (aif_id == AIF1)
		error |= maxim_i2c_write(priv, M98095_048_MIX_DAC_LR,
					    M98095_DAI1L_TO_DACL |
					    M98095_DAI1R_TO_DACR);
	else
		error |= maxim_i2c_write(priv, M98095_048_MIX_DAC_LR,
					    M98095_DAI2M_TO_DACL |
					    M98095_DAI2M_TO_DACR);

	error |= maxim_i2c_write(priv, M98095_092_PWR_EN_OUT,
				    M98095_SPK_SPREADSPECTRUM);
	error |= maxim_i2c_write(priv, M98095_04E_CFG_HP, M98095_HPNORMAL);
	if (aif_id == AIF1)
		error |= maxim_i2c_write(priv, M98095_02C_DAI1_IOCFG,
					    M98095_S1NORMAL | M98095_SDATA);
	else
		error |= maxim_i2c_write(priv, M98095_036_DAI2_IOCFG,
					    M98095_S2NORMAL | M98095_SDATA);

	/* take the codec out of the shut down */
	error |= maxim_bic_or(priv, M98095_097_PWR_SYS, M98095_SHDNRUN,
				 M98095_SHDNRUN);
	/*
	 * route DACL and DACR output to HO and Speakers
	 * Ordering: DACL, DACR, DACL, DACR
	 */
	error |= maxim_i2c_write(priv, M98095_050_MIX_SPK_LEFT, 0x01);
	error |= maxim_i2c_write(priv, M98095_051_MIX_SPK_RIGHT, 0x01);
	error |= maxim_i2c_write(priv, M98095_04C_MIX_HP_LEFT, 0x01);
	error |= maxim_i2c_write(priv, M98095_04D_MIX_HP_RIGHT, 0x01);

	/* power Enable */
	error |= maxim_i2c_write(priv, M98095_091_PWR_EN_OUT, 0xF3);

	/* set Volume */
	error |= maxim_i2c_write(priv, M98095_064_LVL_HP_L, 15);
	error |= maxim_i2c_write(priv, M98095_065_LVL_HP_R, 15);
	error |= maxim_i2c_write(priv, M98095_067_LVL_SPK_L, 16);
	error |= maxim_i2c_write(priv, M98095_068_LVL_SPK_R, 16);

	/* Enable DAIs */
	error |= maxim_i2c_write(priv, M98095_093_BIAS_CTRL, 0x30);
	if (aif_id == AIF1)
		error |= maxim_i2c_write(priv, M98095_096_PWR_DAC_CK, 0x01);
	else
		error |= maxim_i2c_write(priv, M98095_096_PWR_DAC_CK, 0x07);

	if (error < 0)
		return -EIO;

	return 0;
}

static int max98095_do_init(struct maxim_priv *priv,
			    enum en_max_audio_interface aif_id,
			    int sampling_rate, int mclk_freq,
			    int bits_per_sample)
{
	int ret = 0;

	ret = max98095_setup_interface(priv, aif_id);
	if (ret < 0) {
		debug("%s: max98095 setup interface failed\n", __func__);
		return ret;
	}

	ret = max98095_set_sysclk(priv, mclk_freq);
	if (ret < 0) {
		debug("%s: max98095 codec set sys clock failed\n", __func__);
		return ret;
	}

	ret = max98095_hw_params(priv, aif_id, sampling_rate,
				 bits_per_sample);

	if (ret == 0) {
		ret = max98095_set_fmt(priv, SND_SOC_DAIFMT_I2S |
				       SND_SOC_DAIFMT_NB_NF |
				       SND_SOC_DAIFMT_CBS_CFS,
				       aif_id);
	}

	return ret;
}

static int max98095_set_params(struct udevice *dev, int interface, int rate,
			       int mclk_freq, int bits_per_sample,
			       uint channels)
{
	struct maxim_priv *priv = dev_get_priv(dev);

	return max98095_do_init(priv, interface, rate, mclk_freq,
				bits_per_sample);
}

static int max98095_probe(struct udevice *dev)
{
	struct maxim_priv *priv = dev_get_priv(dev);
	int ret;

	priv->dev = dev;
	ret = max98095_device_init(priv);
	if (ret < 0) {
		debug("%s: max98095 codec chip init failed\n", __func__);
		return ret;
	}

	return 0;
}

static const struct audio_codec_ops max98095_ops = {
	.set_params	= max98095_set_params,
};

static const struct udevice_id max98095_ids[] = {
	{ .compatible = "maxim,max98095" },
	{ }
};

U_BOOT_DRIVER(max98095) = {
	.name		= "max98095",
	.id		= UCLASS_AUDIO_CODEC,
	.of_match	= max98095_ids,
	.probe		= max98095_probe,
	.ops		= &max98095_ops,
	.priv_auto	= sizeof(struct maxim_priv),
};
