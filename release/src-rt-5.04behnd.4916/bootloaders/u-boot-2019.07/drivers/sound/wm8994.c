// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2012 Samsung Electronics
 * R. Chandrasekar <rcsekar@samsung.com>
 */
#include <common.h>
#include <audio_codec.h>
#include <dm.h>
#include <div64.h>
#include <fdtdec.h>
#include <i2c.h>
#include <i2s.h>
#include <sound.h>
#include <asm/gpio.h>
#include <asm/io.h>
#include <asm/arch/clk.h>
#include <asm/arch/cpu.h>
#include <asm/arch/sound.h>
#include "wm8994.h"
#include "wm8994_registers.h"

/* defines for wm8994 system clock selection */
#define SEL_MCLK1	0x00
#define SEL_MCLK2	0x08
#define SEL_FLL1	0x10
#define SEL_FLL2	0x18

/* fll config to configure fll */
struct wm8994_fll_config {
	int src;	/* Source */
	int in;		/* Input frequency in Hz */
	int out;	/* output frequency in Hz */
};

/* codec private data */
struct wm8994_priv {
	enum wm8994_type type;		/* codec type of wolfson */
	int revision;			/* Revision */
	int sysclk[WM8994_MAX_AIF];	/* System clock frequency in Hz  */
	int mclk[WM8994_MAX_AIF];	/* master clock frequency in Hz */
	int aifclk[WM8994_MAX_AIF];	/* audio interface clock in Hz   */
	struct wm8994_fll_config fll[2]; /* fll config to configure fll */
	struct udevice *dev;
};

/* wm 8994 supported sampling rate values */
static unsigned int src_rate[] = {
			 8000, 11025, 12000, 16000, 22050, 24000,
			 32000, 44100, 48000, 88200, 96000
};

/* op clock divisions */
static int opclk_divs[] = { 10, 20, 30, 40, 55, 60, 80, 120, 160 };

/* lr clock frame size ratio */
static int fs_ratios[] = {
	64, 128, 192, 256, 348, 512, 768, 1024, 1408, 1536
};

/* bit clock divisors */
static int bclk_divs[] = {
	10, 15, 20, 30, 40, 50, 60, 80, 110, 120, 160, 220, 240, 320, 440, 480,
	640, 880, 960, 1280, 1760, 1920
};

/*
 * Writes value to a device register through i2c
 *
 * @param priv	Private data for driver
 * @param reg	reg number to be write
 * @param data	data to be writen to the above registor
 *
 * @return	int value 1 for change, 0 for no change or negative error code.
 */
static int wm8994_i2c_write(struct wm8994_priv *priv, unsigned int reg,
			    unsigned short data)
{
	unsigned char val[2];

	val[0] = (unsigned char)((data >> 8) & 0xff);
	val[1] = (unsigned char)(data & 0xff);
	debug("Write Addr : 0x%04X, Data :  0x%04X\n", reg, data);

	return dm_i2c_write(priv->dev, reg, val, 2);
}

/*
 * Read a value from a device register through i2c
 *
 * @param priv	Private data for driver
 * @param reg	reg number to be read
 * @param data	address of read data to be stored
 *
 * @return	int value 0 for success, -1 in case of error.
 */
static unsigned int wm8994_i2c_read(struct wm8994_priv *priv, unsigned int reg,
				    unsigned short *data)
{
	unsigned char val[2];
	int ret;

	ret = dm_i2c_read(priv->dev, reg, val, 1);
	if (ret != 0) {
		debug("%s: Error while reading register %#04x\n",
		      __func__, reg);
		return -1;
	}

	*data = val[0];
	*data <<= 8;
	*data |= val[1];

	return 0;
}

/*
 * update device register bits through i2c
 *
 * @param priv	Private data for driver
 * @param reg	codec register
 * @param mask	register mask
 * @param value	new value
 *
 * @return int value 1 if change in the register value,
 * 0 for no change or negative error code.
 */
static int wm8994_bic_or(struct wm8994_priv *priv, unsigned int reg,
			 unsigned short mask, unsigned short value)
{
	int change , ret = 0;
	unsigned short old, new;

	if (wm8994_i2c_read(priv, reg, &old) != 0)
		return -1;
	new = (old & ~mask) | (value & mask);
	change  = (old != new) ? 1 : 0;
	if (change)
		ret = wm8994_i2c_write(priv, reg, new);
	if (ret < 0)
		return ret;

	return change;
}

/*
 * Sets i2s set format
 *
 * @param priv		wm8994 information
 * @param aif_id	Interface ID
 * @param fmt		i2S format
 *
 * @return -1 for error and 0  Success.
 */
static int wm8994_set_fmt(struct wm8994_priv *priv, int aif_id, uint fmt)
{
	int ms_reg;
	int aif_reg;
	int ms = 0;
	int aif = 0;
	int aif_clk = 0;
	int error = 0;

	switch (aif_id) {
	case 1:
		ms_reg = WM8994_AIF1_MASTER_SLAVE;
		aif_reg = WM8994_AIF1_CONTROL_1;
		aif_clk = WM8994_AIF1_CLOCKING_1;
		break;
	case 2:
		ms_reg = WM8994_AIF2_MASTER_SLAVE;
		aif_reg = WM8994_AIF2_CONTROL_1;
		aif_clk = WM8994_AIF2_CLOCKING_1;
		break;
	default:
		debug("%s: Invalid audio interface selection\n", __func__);
		return -1;
	}

	switch (fmt & SND_SOC_DAIFMT_MASTER_MASK) {
	case SND_SOC_DAIFMT_CBS_CFS:
		break;
	case SND_SOC_DAIFMT_CBM_CFM:
		ms = WM8994_AIF1_MSTR;
		break;
	default:
		debug("%s: Invalid i2s master selection\n", __func__);
		return -1;
	}

	switch (fmt & SND_SOC_DAIFMT_FORMAT_MASK) {
	case SND_SOC_DAIFMT_DSP_B:
		aif |= WM8994_AIF1_LRCLK_INV;
	case SND_SOC_DAIFMT_DSP_A:
		aif |= 0x18;
		break;
	case SND_SOC_DAIFMT_I2S:
		aif |= 0x10;
		break;
	case SND_SOC_DAIFMT_RIGHT_J:
		break;
	case SND_SOC_DAIFMT_LEFT_J:
		aif |= 0x8;
		break;
	default:
		debug("%s: Invalid i2s format selection\n", __func__);
		return -1;
	}

	switch (fmt & SND_SOC_DAIFMT_FORMAT_MASK) {
	case SND_SOC_DAIFMT_DSP_A:
	case SND_SOC_DAIFMT_DSP_B:
		/* frame inversion not valid for DSP modes */
		switch (fmt & SND_SOC_DAIFMT_INV_MASK) {
		case SND_SOC_DAIFMT_NB_NF:
			break;
		case SND_SOC_DAIFMT_IB_NF:
			aif |= WM8994_AIF1_BCLK_INV;
			break;
		default:
			debug("%s: Invalid i2s frame inverse selection\n",
			      __func__);
			return -1;
		}
		break;

	case SND_SOC_DAIFMT_I2S:
	case SND_SOC_DAIFMT_RIGHT_J:
	case SND_SOC_DAIFMT_LEFT_J:
		switch (fmt & SND_SOC_DAIFMT_INV_MASK) {
		case SND_SOC_DAIFMT_NB_NF:
			break;
		case SND_SOC_DAIFMT_IB_IF:
			aif |= WM8994_AIF1_BCLK_INV | WM8994_AIF1_LRCLK_INV;
			break;
		case SND_SOC_DAIFMT_IB_NF:
			aif |= WM8994_AIF1_BCLK_INV;
			break;
		case SND_SOC_DAIFMT_NB_IF:
			aif |= WM8994_AIF1_LRCLK_INV;
			break;
		default:
			debug("%s: Invalid i2s clock polarity selection\n",
			      __func__);
			return -1;
		}
		break;
	default:
		debug("%s: Invalid i2s format selection\n", __func__);
		return -1;
	}

	error = wm8994_bic_or(priv, aif_reg, WM8994_AIF1_BCLK_INV |
			      WM8994_AIF1_LRCLK_INV_MASK |
			       WM8994_AIF1_FMT_MASK, aif);

	error |= wm8994_bic_or(priv, ms_reg, WM8994_AIF1_MSTR_MASK, ms);
	error |= wm8994_bic_or(priv, aif_clk, WM8994_AIF1CLK_ENA_MASK,
			       WM8994_AIF1CLK_ENA);
	if (error < 0) {
		debug("%s: codec register access error\n", __func__);
		return -1;
	}

	return 0;
}

/*
 * Sets hw params FOR WM8994
 *
 * @param priv			wm8994 information pointer
 * @param aif_id		Audio interface ID
 * @param sampling_rate		Sampling rate
 * @param bits_per_sample	Bits per sample
 * @param Channels		Channels in the given audio input
 *
 * @return -1 for error  and 0  Success.
 */
static int wm8994_hw_params(struct wm8994_priv *priv, int aif_id,
			    uint sampling_rate, uint bits_per_sample,
			    uint channels)
{
	int aif1_reg;
	int aif2_reg;
	int bclk_reg;
	int bclk = 0;
	int rate_reg;
	int aif1 = 0;
	int aif2 = 0;
	int rate_val = 0;
	int id = aif_id - 1;
	int i, cur_val, best_val, bclk_rate, best;
	unsigned short reg_data;
	int ret = 0;

	switch (aif_id) {
	case 1:
		aif1_reg = WM8994_AIF1_CONTROL_1;
		aif2_reg = WM8994_AIF1_CONTROL_2;
		bclk_reg = WM8994_AIF1_BCLK;
		rate_reg = WM8994_AIF1_RATE;
		break;
	case 2:
		aif1_reg = WM8994_AIF2_CONTROL_1;
		aif2_reg = WM8994_AIF2_CONTROL_2;
		bclk_reg = WM8994_AIF2_BCLK;
		rate_reg = WM8994_AIF2_RATE;
		break;
	default:
		return -1;
	}

	bclk_rate = sampling_rate * 32;
	switch (bits_per_sample) {
	case 16:
		bclk_rate *= 16;
		break;
	case 20:
		bclk_rate *= 20;
		aif1 |= 0x20;
		break;
	case 24:
		bclk_rate *= 24;
		aif1 |= 0x40;
		break;
	case 32:
		bclk_rate *= 32;
		aif1 |= 0x60;
		break;
	default:
		return -1;
	}

	/* Try to find an appropriate sample rate; look for an exact match. */
	for (i = 0; i < ARRAY_SIZE(src_rate); i++)
		if (src_rate[i] == sampling_rate)
			break;

	if (i == ARRAY_SIZE(src_rate)) {
		debug("%s: Could not get the best matching samplingrate\n",
		      __func__);
		return -1;
	}

	rate_val |= i << WM8994_AIF1_SR_SHIFT;

	/* AIFCLK/fs ratio; look for a close match in either direction */
	best = 0;
	best_val = abs((fs_ratios[0] * sampling_rate) - priv->aifclk[id]);

	for (i = 1; i < ARRAY_SIZE(fs_ratios); i++) {
		cur_val = abs(fs_ratios[i] * sampling_rate - priv->aifclk[id]);
		if (cur_val >= best_val)
			continue;
		best = i;
		best_val = cur_val;
	}

	rate_val |= best;

	/*
	 * We may not get quite the right frequency if using
	 * approximate clocks so look for the closest match that is
	 * higher than the target (we need to ensure that there enough
	 * BCLKs to clock out the samples).
	 */
	best = 0;
	for (i = 0; i < ARRAY_SIZE(bclk_divs); i++) {
		cur_val = (priv->aifclk[id] * 10 / bclk_divs[i]) - bclk_rate;
		if (cur_val < 0) /* BCLK table is sorted */
			break;
		best = i;
	}

	if (i ==  ARRAY_SIZE(bclk_divs)) {
		debug("%s: Could not get the best matching bclk division\n",
		      __func__);
		return -1;
	}

	bclk_rate = priv->aifclk[id] * 10 / bclk_divs[best];
	bclk |= best << WM8994_AIF1_BCLK_DIV_SHIFT;

	if (wm8994_i2c_read(priv, aif1_reg, &reg_data) != 0) {
		debug("%s: AIF1 register read Failed\n", __func__);
		return -1;
	}

	if ((channels == 1) && ((reg_data & 0x18) == 0x18))
		aif2 |= WM8994_AIF1_MONO;

	if (priv->aifclk[id] == 0) {
		debug("%s:Audio interface clock not set\n", __func__);
		return -1;
	}

	ret = wm8994_bic_or(priv, aif1_reg, WM8994_AIF1_WL_MASK, aif1);
	ret |= wm8994_bic_or(priv, aif2_reg, WM8994_AIF1_MONO, aif2);
	ret |= wm8994_bic_or(priv, bclk_reg, WM8994_AIF1_BCLK_DIV_MASK,
				  bclk);
	ret |= wm8994_bic_or(priv, rate_reg, WM8994_AIF1_SR_MASK |
				  WM8994_AIF1CLK_RATE_MASK, rate_val);

	debug("rate vale = %x , bclk val= %x\n", rate_val, bclk);

	if (ret < 0) {
		debug("%s: codec register access error\n", __func__);
		return -1;
	}

	return 0;
}

/*
 * Configures Audio interface Clock
 *
 * @param priv		wm8994 information pointer
 * @param aif		Audio Interface ID
 *
 * @return -1 for error  and 0  Success.
 */
static int configure_aif_clock(struct wm8994_priv *priv, int aif)
{
	int rate;
	int reg1 = 0;
	int offset;
	int ret;

	/* AIF(1/0) register adress offset calculated */
	if (aif-1)
		offset = 4;
	else
		offset = 0;

	switch (priv->sysclk[aif - 1]) {
	case WM8994_SYSCLK_MCLK1:
		reg1 |= SEL_MCLK1;
		rate = priv->mclk[0];
		break;

	case WM8994_SYSCLK_MCLK2:
		reg1 |= SEL_MCLK2;
		rate = priv->mclk[1];
		break;

	case WM8994_SYSCLK_FLL1:
		reg1 |= SEL_FLL1;
		rate = priv->fll[0].out;
		break;

	case WM8994_SYSCLK_FLL2:
		reg1 |= SEL_FLL2;
		rate = priv->fll[1].out;
		break;

	default:
		debug("%s: Invalid input clock selection [%d]\n",
		      __func__, priv->sysclk[aif - 1]);
		return -1;
	}

	/* if input clock frequenct is more than 135Mhz then divide */
	if (rate >= WM8994_MAX_INPUT_CLK_FREQ) {
		rate /= 2;
		reg1 |= WM8994_AIF1CLK_DIV;
	}

	priv->aifclk[aif - 1] = rate;

	ret = wm8994_bic_or(priv, WM8994_AIF1_CLOCKING_1 + offset,
			    WM8994_AIF1CLK_SRC_MASK | WM8994_AIF1CLK_DIV,
			    reg1);

	if (aif == WM8994_AIF1)
		ret |= wm8994_bic_or(priv, WM8994_CLOCKING_1,
			WM8994_AIF1DSPCLK_ENA_MASK | WM8994_SYSDSPCLK_ENA_MASK,
			WM8994_AIF1DSPCLK_ENA | WM8994_SYSDSPCLK_ENA);
	else if (aif == WM8994_AIF2)
		ret |= wm8994_bic_or(priv, WM8994_CLOCKING_1,
			WM8994_SYSCLK_SRC | WM8994_AIF2DSPCLK_ENA_MASK |
			WM8994_SYSDSPCLK_ENA_MASK, WM8994_SYSCLK_SRC |
			WM8994_AIF2DSPCLK_ENA | WM8994_SYSDSPCLK_ENA);

	if (ret < 0) {
		debug("%s: codec register access error\n", __func__);
		return -1;
	}

	return 0;
}

/*
 * Configures Audio interface  for the given frequency
 *
 * @param priv		wm8994 information
 * @param aif_id	Audio Interface
 * @param clk_id	Input Clock ID
 * @param freq		Sampling frequency in Hz
 *
 * @return -1 for error and 0 success.
 */
static int wm8994_set_sysclk(struct wm8994_priv *priv, int aif_id, int clk_id,
			     unsigned int freq)
{
	int i;
	int ret = 0;

	priv->sysclk[aif_id - 1] = clk_id;

	switch (clk_id) {
	case WM8994_SYSCLK_MCLK1:
		priv->mclk[0] = freq;
		if (aif_id == 2) {
			ret = wm8994_bic_or(priv, WM8994_AIF1_CLOCKING_2,
					    WM8994_AIF2DAC_DIV_MASK, 0);
		}
		break;

	case WM8994_SYSCLK_MCLK2:
		/* TODO: Set GPIO AF */
		priv->mclk[1] = freq;
		break;

	case WM8994_SYSCLK_FLL1:
	case WM8994_SYSCLK_FLL2:
		break;

	case WM8994_SYSCLK_OPCLK:
		/*
		 * Special case - a division (times 10) is given and
		 * no effect on main clocking.
		 */
		if (freq) {
			for (i = 0; i < ARRAY_SIZE(opclk_divs); i++)
				if (opclk_divs[i] == freq)
					break;
			if (i == ARRAY_SIZE(opclk_divs)) {
				debug("%s frequency divisor not found\n",
				      __func__);
				return -1;
			}
			ret = wm8994_bic_or(priv, WM8994_CLOCKING_2,
					    WM8994_OPCLK_DIV_MASK, i);
			ret |= wm8994_bic_or(priv, WM8994_POWER_MANAGEMENT_2,
					     WM8994_OPCLK_ENA,
					     WM8994_OPCLK_ENA);
		} else {
			ret |= wm8994_bic_or(priv, WM8994_POWER_MANAGEMENT_2,
					     WM8994_OPCLK_ENA, 0);
		}

	default:
		debug("%s Invalid input clock selection [%d]\n",
		      __func__, clk_id);
		return -1;
	}

	ret |= configure_aif_clock(priv, aif_id);

	if (ret < 0) {
		debug("%s: codec register access error\n", __func__);
		return -1;
	}

	return 0;
}

/*
 * Initializes Volume for AIF2 to HP path
 *
 * @param priv		wm8994 information
 * @returns -1 for error  and 0 Success.
 *
 */
static int wm8994_init_volume_aif2_dac1(struct wm8994_priv *priv)
{
	int ret;

	/* Unmute AIF2DAC */
	ret = wm8994_bic_or(priv, WM8994_AIF2_DAC_FILTERS_1,
			    WM8994_AIF2DAC_MUTE_MASK, 0);


	ret |= wm8994_bic_or(priv, WM8994_AIF2_DAC_LEFT_VOLUME,
			     WM8994_AIF2DAC_VU_MASK | WM8994_AIF2DACL_VOL_MASK,
			     WM8994_AIF2DAC_VU | 0xff);

	ret |= wm8994_bic_or(priv, WM8994_AIF2_DAC_RIGHT_VOLUME,
			     WM8994_AIF2DAC_VU_MASK | WM8994_AIF2DACR_VOL_MASK,
			     WM8994_AIF2DAC_VU | 0xff);


	ret |= wm8994_bic_or(priv, WM8994_DAC1_LEFT_VOLUME,
			     WM8994_DAC1_VU_MASK | WM8994_DAC1L_VOL_MASK |
			     WM8994_DAC1L_MUTE_MASK, WM8994_DAC1_VU | 0xc0);

	ret |= wm8994_bic_or(priv, WM8994_DAC1_RIGHT_VOLUME,
			     WM8994_DAC1_VU_MASK | WM8994_DAC1R_VOL_MASK |
			     WM8994_DAC1R_MUTE_MASK, WM8994_DAC1_VU | 0xc0);
	/* Head Phone Volume */
	ret |= wm8994_i2c_write(priv, WM8994_LEFT_OUTPUT_VOLUME, 0x12D);
	ret |= wm8994_i2c_write(priv, WM8994_RIGHT_OUTPUT_VOLUME, 0x12D);

	if (ret < 0) {
		debug("%s: codec register access error\n", __func__);
		return -1;
	}

	return 0;
}

/*
 * Initializes Volume for AIF1 to HP path
 *
 * @param priv		wm8994 information
 * @returns -1 for error  and 0 Success.
 *
 */
static int wm8994_init_volume_aif1_dac1(struct wm8994_priv *priv)
{
	int ret = 0;

	/* Unmute AIF1DAC */
	ret |= wm8994_i2c_write(priv, WM8994_AIF1_DAC_FILTERS_1, 0x0000);

	ret |= wm8994_bic_or(priv, WM8994_DAC1_LEFT_VOLUME,
			     WM8994_DAC1_VU_MASK | WM8994_DAC1L_VOL_MASK |
			     WM8994_DAC1L_MUTE_MASK, WM8994_DAC1_VU | 0xc0);

	ret |= wm8994_bic_or(priv, WM8994_DAC1_RIGHT_VOLUME,
			     WM8994_DAC1_VU_MASK | WM8994_DAC1R_VOL_MASK |
			     WM8994_DAC1R_MUTE_MASK, WM8994_DAC1_VU | 0xc0);
	/* Head Phone Volume */
	ret |= wm8994_i2c_write(priv, WM8994_LEFT_OUTPUT_VOLUME, 0x12D);
	ret |= wm8994_i2c_write(priv, WM8994_RIGHT_OUTPUT_VOLUME, 0x12D);

	if (ret < 0) {
		debug("%s: codec register access error\n", __func__);
		return -1;
	}

	return 0;
}

/*
 * Intialise wm8994 codec device
 *
 * @param priv		wm8994 information
 *
 * @returns -1 for error  and 0 Success.
 */
static int wm8994_device_init(struct wm8994_priv *priv)
{
	const char *devname;
	unsigned short reg_data;
	int ret;

	wm8994_i2c_write(priv, WM8994_SOFTWARE_RESET, WM8994_SW_RESET);

	ret = wm8994_i2c_read(priv, WM8994_SOFTWARE_RESET, &reg_data);
	if (ret < 0) {
		debug("Failed to read ID register\n");
		return ret;
	}

	if (reg_data == WM8994_ID) {
		devname = "WM8994";
		debug("Device registered as type %d\n", priv->type);
		priv->type = WM8994;
	} else {
		debug("Device is not a WM8994, ID is %x\n", ret);
		return -ENXIO;
	}

	ret = wm8994_i2c_read(priv, WM8994_CHIP_REVISION, &reg_data);
	if (ret < 0) {
		debug("Failed to read revision register: %d\n", ret);
		return ret;
	}
	priv->revision = reg_data;
	debug("%s revision %c\n", devname, 'A' + priv->revision);

	return 0;
}

static int wm8994_setup_interface(struct wm8994_priv *priv,
				  enum en_audio_interface aif_id)
{
	int ret;

	/* VMID Selection */
	ret = wm8994_bic_or(priv, WM8994_POWER_MANAGEMENT_1,
			    WM8994_VMID_SEL_MASK | WM8994_BIAS_ENA_MASK, 0x3);

	/* Charge Pump Enable */
	ret |= wm8994_bic_or(priv, WM8994_CHARGE_PUMP_1, WM8994_CP_ENA_MASK,
			     WM8994_CP_ENA);

	/* Head Phone Power Enable */
	ret |= wm8994_bic_or(priv, WM8994_POWER_MANAGEMENT_1,
			     WM8994_HPOUT1L_ENA_MASK, WM8994_HPOUT1L_ENA);

	ret |= wm8994_bic_or(priv, WM8994_POWER_MANAGEMENT_1,
			     WM8994_HPOUT1R_ENA_MASK, WM8994_HPOUT1R_ENA);

	if (aif_id == WM8994_AIF1) {
		ret |= wm8994_i2c_write(priv, WM8994_POWER_MANAGEMENT_2,
					WM8994_TSHUT_ENA | WM8994_MIXINL_ENA |
					WM8994_MIXINR_ENA | WM8994_IN2L_ENA |
					WM8994_IN2R_ENA);

		ret |= wm8994_i2c_write(priv, WM8994_POWER_MANAGEMENT_4,
					WM8994_ADCL_ENA | WM8994_ADCR_ENA |
					WM8994_AIF1ADC1R_ENA |
					WM8994_AIF1ADC1L_ENA);

		/* Power enable for AIF1 and DAC1 */
		ret |= wm8994_i2c_write(priv, WM8994_POWER_MANAGEMENT_5,
					WM8994_AIF1DACL_ENA |
					WM8994_AIF1DACR_ENA |
					WM8994_DAC1L_ENA | WM8994_DAC1R_ENA);
	} else if (aif_id == WM8994_AIF2) {
		/* Power enable for AIF2 and DAC1 */
		ret |= wm8994_bic_or(priv, WM8994_POWER_MANAGEMENT_5,
			WM8994_AIF2DACL_ENA_MASK | WM8994_AIF2DACR_ENA_MASK |
			WM8994_DAC1L_ENA_MASK | WM8994_DAC1R_ENA_MASK,
			WM8994_AIF2DACL_ENA | WM8994_AIF2DACR_ENA |
			WM8994_DAC1L_ENA | WM8994_DAC1R_ENA);
	}
	/* Head Phone Initialisation */
	ret |= wm8994_bic_or(priv, WM8994_ANALOGUE_HP_1,
		WM8994_HPOUT1L_DLY_MASK | WM8994_HPOUT1R_DLY_MASK,
		WM8994_HPOUT1L_DLY | WM8994_HPOUT1R_DLY);

	ret |= wm8994_bic_or(priv, WM8994_DC_SERVO_1,
			WM8994_DCS_ENA_CHAN_0_MASK |
			WM8994_DCS_ENA_CHAN_1_MASK , WM8994_DCS_ENA_CHAN_0 |
			WM8994_DCS_ENA_CHAN_1);

	ret |= wm8994_bic_or(priv, WM8994_ANALOGUE_HP_1,
			WM8994_HPOUT1L_DLY_MASK |
			WM8994_HPOUT1R_DLY_MASK | WM8994_HPOUT1L_OUTP_MASK |
			WM8994_HPOUT1R_OUTP_MASK |
			WM8994_HPOUT1L_RMV_SHORT_MASK |
			WM8994_HPOUT1R_RMV_SHORT_MASK, WM8994_HPOUT1L_DLY |
			WM8994_HPOUT1R_DLY | WM8994_HPOUT1L_OUTP |
			WM8994_HPOUT1R_OUTP | WM8994_HPOUT1L_RMV_SHORT |
			WM8994_HPOUT1R_RMV_SHORT);

	/* MIXER Config DAC1 to HP */
	ret |= wm8994_bic_or(priv, WM8994_OUTPUT_MIXER_1,
			     WM8994_DAC1L_TO_HPOUT1L_MASK,
			     WM8994_DAC1L_TO_HPOUT1L);

	ret |= wm8994_bic_or(priv, WM8994_OUTPUT_MIXER_2,
			     WM8994_DAC1R_TO_HPOUT1R_MASK,
			     WM8994_DAC1R_TO_HPOUT1R);

	if (aif_id == WM8994_AIF1) {
		/* Routing AIF1 to DAC1 */
		ret |= wm8994_i2c_write(priv, WM8994_DAC1_LEFT_MIXER_ROUTING,
					WM8994_AIF1DAC1L_TO_DAC1L);

		ret |= wm8994_i2c_write(priv, WM8994_DAC1_RIGHT_MIXER_ROUTING,
					WM8994_AIF1DAC1R_TO_DAC1R);

		/* GPIO Settings for AIF1 */
		ret |=  wm8994_i2c_write(priv, WM8994_GPIO_1,
					 WM8994_GPIO_DIR_OUTPUT |
					 WM8994_GPIO_FUNCTION_I2S_CLK |
					 WM8994_GPIO_INPUT_DEBOUNCE);

		ret |= wm8994_init_volume_aif1_dac1(priv);
	} else if (aif_id == WM8994_AIF2) {
		/* Routing AIF2 to DAC1 */
		ret |= wm8994_bic_or(priv, WM8994_DAC1_LEFT_MIXER_ROUTING,
				     WM8994_AIF2DACL_TO_DAC1L_MASK,
				     WM8994_AIF2DACL_TO_DAC1L);

		ret |= wm8994_bic_or(priv, WM8994_DAC1_RIGHT_MIXER_ROUTING,
				     WM8994_AIF2DACR_TO_DAC1R_MASK,
				     WM8994_AIF2DACR_TO_DAC1R);

		/* GPIO Settings for AIF2 */
		/* B CLK */
		ret |= wm8994_bic_or(priv, WM8994_GPIO_3, WM8994_GPIO_DIR_MASK |
				     WM8994_GPIO_FUNCTION_MASK,
				     WM8994_GPIO_DIR_OUTPUT);

		/* LR CLK */
		ret |= wm8994_bic_or(priv, WM8994_GPIO_4, WM8994_GPIO_DIR_MASK |
				     WM8994_GPIO_FUNCTION_MASK,
				     WM8994_GPIO_DIR_OUTPUT);

		/* DATA */
		ret |= wm8994_bic_or(priv, WM8994_GPIO_5, WM8994_GPIO_DIR_MASK |
				     WM8994_GPIO_FUNCTION_MASK,
				     WM8994_GPIO_DIR_OUTPUT);

		ret |= wm8994_init_volume_aif2_dac1(priv);
	}

	if (ret < 0)
		goto err;

	debug("%s: Codec chip setup ok\n", __func__);
	return 0;
err:
	debug("%s: Codec chip setup error\n", __func__);
	return -1;
}

static int _wm8994_init(struct wm8994_priv *priv,
			enum en_audio_interface aif_id, int sampling_rate,
			int mclk_freq, int bits_per_sample,
			unsigned int channels)
{
	int ret;

	ret = wm8994_setup_interface(priv, aif_id);
	if (ret < 0) {
		debug("%s: wm8994 codec chip init failed\n", __func__);
		return ret;
	}

	ret = wm8994_set_sysclk(priv, aif_id, WM8994_SYSCLK_MCLK1, mclk_freq);
	if (ret < 0) {
		debug("%s: wm8994 codec set sys clock failed\n", __func__);
		return ret;
	}

	ret = wm8994_hw_params(priv, aif_id, sampling_rate, bits_per_sample,
			       channels);

	if (ret == 0) {
		ret = wm8994_set_fmt(priv, aif_id, SND_SOC_DAIFMT_I2S |
				     SND_SOC_DAIFMT_NB_NF |
				     SND_SOC_DAIFMT_CBS_CFS);
	}

	return ret;
}

static int wm8994_set_params(struct udevice *dev, int interface, int rate,
			     int mclk_freq, int bits_per_sample, uint channels)
{
	struct wm8994_priv *priv = dev_get_priv(dev);

	return _wm8994_init(priv, interface, rate, mclk_freq, bits_per_sample,
			    channels);
}

static int wm8994_probe(struct udevice *dev)
{
	struct wm8994_priv *priv = dev_get_priv(dev);

	priv->dev = dev;
	return wm8994_device_init(priv);
}

static const struct audio_codec_ops wm8994_ops = {
	.set_params	= wm8994_set_params,
};

static const struct udevice_id wm8994_ids[] = {
	{ .compatible = "wolfson,wm8994" },
	{ }
};

U_BOOT_DRIVER(wm8994) = {
	.name		= "wm8994",
	.id		= UCLASS_AUDIO_CODEC,
	.of_match	= wm8994_ids,
	.probe		= wm8994_probe,
	.ops		= &wm8994_ops,
	.priv_auto_alloc_size	= sizeof(struct wm8994_priv),
};
