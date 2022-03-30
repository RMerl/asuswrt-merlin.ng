// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2019 Google LLC
 */

#define LOG_CATEGORY UCLASS_SOUND

#include <common.h>
#include <audio_codec.h>
#include <dm.h>
#include <i2c.h>
#include "rt5677.h"

struct rt5677_priv {
	struct udevice *dev;
};

/* RT5677 has 256 8-bit register addresses, and 16-bit register data */
struct rt5677_init_reg {
	u8 reg;
	u16 val;
};

static struct rt5677_init_reg init_list[] = {
	{RT5677_LOUT1,		  0x0800},
	{RT5677_SIDETONE_CTRL,	  0x0000},
	{RT5677_STO1_ADC_DIG_VOL, 0x3F3F},
	{RT5677_DAC1_DIG_VOL,	  0x9090},
	{RT5677_STO2_ADC_MIXER,	  0xA441},
	{RT5677_STO1_ADC_MIXER,	  0x5480},
	{RT5677_STO1_DAC_MIXER,	  0x8A8A},
	{RT5677_PWR_DIG1,	  0x9800}, /* Power up I2S1 */
	{RT5677_PWR_ANLG1,	  0xE9D5},
	{RT5677_PWR_ANLG2,	  0x2CC0},
	{RT5677_PWR_DSP2,	  0x0C00},
	{RT5677_I2S2_SDP,	  0x0000},
	{RT5677_CLK_TREE_CTRL1,	  0x1111},
	{RT5677_PLL1_CTRL1,	  0x0000},
	{RT5677_PLL1_CTRL2,	  0x0000},
	{RT5677_DIG_MISC,	  0x0029},
	{RT5677_GEN_CTRL1,	  0x00FF},
	{RT5677_GPIO_CTRL2,	  0x0020},
	{RT5677_PWR_DIG2,	  0x9024}, /* Power on ADC Stereo Filters */
	{RT5677_PDM_OUT_CTRL,	  0x0088}, /* Unmute PDM, set stereo1 DAC */
	{RT5677_PDM_DATA_CTRL1,   0x0001}, /* Sysclk to PDM filter divider 2 */
};

/**
 * rt5677_i2c_read() - Read a 16-bit register
 *
 * @priv: Private driver data
 * @reg: Register number to read
 * @returns data read or -ve on error
 */
static int rt5677_i2c_read(struct rt5677_priv *priv, uint reg)
{
	u8 buf[2];
	int ret;

	ret = dm_i2c_read(priv->dev, reg, buf, sizeof(u16));
	if (ret)
		return ret;
	return buf[0] << 8 | buf[1];
}

/**
 * rt5677_i2c_write() - Write a 16-bit register
 *
 * @priv: Private driver data
 * @reg: Register number to read
 * @data: Data to write
 * @returns 0 if OK, -ve on error
 */
static int rt5677_i2c_write(struct rt5677_priv *priv, uint reg, uint data)
{
	u8 buf[2];

	buf[0] = (data >> 8) & 0xff;
	buf[1] = data & 0xff;

	return dm_i2c_write(priv->dev, reg, buf, sizeof(u16));
}

/**
 * rt5677_bic_or() - Set and clear bits of a codec register
 *
 * @priv: Private driver data
 * @reg: Register number to update
 * @bic: Mask of bits to clear
 * @set: Mask of bits to set
 * @returns 0 if OK, -ve on error
 *
 */
static int rt5677_bic_or(struct rt5677_priv *priv, uint reg, uint bic,
			 uint set)
{
	uint old, new_value;
	int ret;

	old = rt5677_i2c_read(priv, reg);
	if (old < 0)
		return old;

	new_value = (old & ~bic) | (set & bic);

	if (old != new_value) {
		ret = rt5677_i2c_write(priv, reg, new_value);
		if (ret)
			return ret;
	}

	return 0;
}

/**
 * rt5677_reg_init() - Initialise codec regs w/static/base values
 *
 * @priv: Private driver data
 * @returns 0 if OK, -ve on error
 */
static int rt5677_reg_init(struct rt5677_priv *priv)
{
	int ret;
	int i;

	for (i = 0; i < ARRAY_SIZE(init_list); i++) {
		ret = rt5677_i2c_write(priv, init_list[i].reg, init_list[i].val);
		if (ret)
			return ret;
	}

	return 0;
}

#ifdef DEBUG
static void debug_dump_5677_regs(struct rt5677_priv *priv, int swap)
{
	uint i, reg_word;

	/* Show all 16-bit codec regs */
	for (i = 0; i < RT5677_REG_CNT; i++) {
		if (i % 8 == 0)
			log_debug("\nMX%02x: ", i);

		rt5677_i2c_read(priv, (u8)i, &reg_word);
		if (swap)
			log_debug("%04x ", swap_bytes16(reg_word));
		else
			log_debug("%04x ", reg_word);
	}
	log_debug("\n");

	/* Show all 16-bit 'private' codec regs */
	for (i = 0; i < RT5677_PR_REG_CNT; i++) {
		if (i % 8 == 0)
			log_debug("\nPR%02x: ", i);

		rt5677_i2c_write(priv, RT5677_PRIV_INDEX, i);
		rt5677_i2c_read(priv, RT5677_PRIV_DATA, &reg_word);
		if (swap)
			log_debug("%04x ", swap_bytes16(reg_word));
		else
			log_debug("%04x ", reg_word);
	}
	log_debug("\n");
}
#endif	/* DEBUG */

static int rt5677_hw_params(struct rt5677_priv *priv, uint bits_per_sample)
{
	int ret;

	switch (bits_per_sample) {
	case 16:
		ret = rt5677_bic_or(priv, RT5677_I2S1_SDP, RT5677_I2S_DL_MASK,
				    0);
		if (ret) {
			log_debug("Error updating I2S1 Interface Ctrl reg\n");
			return 1;
		}
		break;
	default:
		log_err("Illegal bits per sample %d\n", bits_per_sample);
		return -EINVAL;
	}

	return 0;
}

/**
 * rt5677_set_fmt() - set rt5677 I2S format
 *
 * @priv: Private driver data
 * @returns 0 if OK, -ve on error
 */
static int rt5677_set_fmt(struct rt5677_priv *priv)
{
	int ret = 0;

	/*
	 * Set format here: Assumes I2S, NB_NF, CBS_CFS
	 *
	 * CBS_CFS (Codec Bit Slave/Codec Frame Slave)
	 */
	ret = rt5677_bic_or(priv, RT5677_I2S1_SDP, RT5677_I2S_MS_MASK,
			    RT5677_I2S_MS_S);

	/* NB_NF (Normal Bit/Normal Frame) */
	ret |= rt5677_bic_or(priv, RT5677_I2S1_SDP, RT5677_I2S_BP_MASK,
			     RT5677_I2S_BP_NOR);

	/* I2S mode */
	ret |= rt5677_bic_or(priv, RT5677_I2S1_SDP, RT5677_I2S_DF_MASK,
			     RT5677_I2S_DF_I2S);

	/* A44: I2S2 (going to speaker amp) is master */
	ret |= rt5677_bic_or(priv, RT5677_I2S2_SDP, RT5677_I2S_MS_MASK,
			     RT5677_I2S_MS_M);

	if (ret) {
		log_err("Error updating I2S1 Interface Ctrl reg\n");
		return ret;
	}

	return 0;
}

/**
 * rt5677_reset() - reset the audio codec
 *
 * @priv: Private driver data
 * @returns 0 if OK, -ve on error
 */
static int rt5677_reset(struct rt5677_priv *priv)
{
	int ret;

	/* Reset the codec registers to their defaults */
	ret = rt5677_i2c_write(priv, RT5677_RESET, RT5677_SW_RESET);
	if (ret) {
		log_err("Error resetting codec\n");
		return ret;
	}

	return 0;
}

/**
 * Initialise rt5677 codec device
 *
 * @priv: Private driver data
 * @returns 0 if OK, -ve on error
 */
int rt5677_device_init(struct rt5677_priv *priv)
{
	int ret;

	/* Read status reg */
	ret = rt5677_i2c_read(priv, RT5677_RESET);
	if (ret < 0)
		return ret;
	log_debug("reg 00h, Software Reset & Status = 0x%04x\n", ret);

	/* Reset the codec/regs */
	ret = rt5677_reset(priv);
	if (ret)
		return ret;

	ret = rt5677_i2c_read(priv, RT5677_VENDOR_ID1);
	if (ret < 0) {
		log_err("Error reading vendor ID\n");
		return 1;
	}
	log_debug("Hardware ID: %0xX\n", ret);

	ret = rt5677_i2c_read(priv, RT5677_VENDOR_ID2);
	if (ret < 0) {
		log_err("Error reading vendor rev\n");
		return 1;
	}
	log_debug("Hardware revision: %04x\n", ret);

	return 0;
}

static int rt5677_set_params(struct udevice *dev, int interface, int rate,
			     int mclk_freq, int bits_per_sample,
			     uint channels)
{
	struct rt5677_priv *priv = dev_get_priv(dev);
	int ret;

	/* Initialise codec regs w/static/base values, same as Linux driver */
	ret = rt5677_reg_init(priv);
	if (ret)
		return ret;

	ret = rt5677_hw_params(priv, bits_per_sample);
	if (ret)
		return ret;

	ret = rt5677_set_fmt(priv);
	if (ret)
		return ret;

	return 0;
}

static int rt5677_probe(struct udevice *dev)
{
	struct rt5677_priv *priv = dev_get_priv(dev);

	priv->dev = dev;

	return rt5677_device_init(priv);
}

static const struct audio_codec_ops rt5677_ops = {
	.set_params	= rt5677_set_params,
};

static const struct udevice_id rt5677_ids[] = {
	{ .compatible = "realtek,rt5677" },
	{ }
};

U_BOOT_DRIVER(rt5677_drv) = {
	.name		= "rt5677",
	.id		= UCLASS_AUDIO_CODEC,
	.of_match	= rt5677_ids,
	.ops		= &rt5677_ops,
	.probe		= rt5677_probe,
	.priv_auto_alloc_size	= sizeof(struct rt5677_priv),
};
