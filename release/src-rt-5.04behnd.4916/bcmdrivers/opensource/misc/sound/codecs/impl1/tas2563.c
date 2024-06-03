/*
 * tas2563.c  --   ASoC Driver for TI DAC tas2563 codecs
 *
 * Author: Kevin Li <kevin-ke.li@broadcom.com>
 *
<:copyright-BRCM:2022:DUAL/GPL:standard

   Copyright (c) 2022 Broadcom 
   All Rights Reserved

Unless you and Broadcom execute a separate written software license
agreement governing use of this software, this software is licensed
to you under the terms of the GNU General Public License version 2
(the "GPL"), available at http://www.broadcom.com/licenses/GPLv2.php,
with the following added to such license:

   As a special exception, the copyright holders of this software give
   you permission to link this software with independent modules, and
   to copy and distribute the resulting executable under terms of your
   choice, provided that you also meet, for each linked independent
   module, the terms and conditions of the license of that module.
   An independent module is a module which is not derived from this
   software.  The special exception does not apply to any modifications
   of the software.

Not withstanding the above, under no circumstances may you combine
this software in any way with any other Broadcom software provided
under a license other than the GPL, without Broadcom's express prior
written consent.

:>
*/

#include <linux/clk.h>
#include <linux/i2c.h>
#include <linux/module.h>
#include <sound/pcm_params.h>
#include <sound/soc.h>
#include <sound/soc-dapm.h>
#include <sound/tlv.h>

#include "tas2563.h"

#define DIGITAL_GAIN_INIT -21
static int digital_gain[][5] = 	
			{ {0,  0x04, 0x00, 0x00, 0x00},
				{-3, 0x2d, 0x4e, 0xfb, 0xd5},
				{-6, 0x20, 0x13, 0x73, 0x9e},
				{-9, 0x16, 0xb5, 0x43, 0x37},
				{-12,0x10, 0x13, 0x79, 0x87},
				{-15,0x0b, 0x61, 0x88, 0x71},
				{-18,0x08, 0x0e, 0x9f, 0x96},
				{-21,0x05, 0xb4, 0x39, 0xbc}
			};

static const struct regmap_range_cfg dac2563_regmap_pages[] = {
	{
		.selector_reg   = 0,
		.selector_mask  = 0xff,
		.window_start   = 0,
		.window_len     = TAS2563_PAGE_LEN,
		.range_min      = 0,
		.range_max      = TAS2563_PAGE2_START + TAS2563_PAGE_LEN,
	},
};

static const struct regmap_config dac2563_regmap = {
	.reg_bits       = 8,
	.val_bits       = 8,
	.cache_type     = REGCACHE_NONE,
	.max_register   = TAS2563_PAGE2_START + TAS2563_PAGE_LEN,
	.ranges         = dac2563_regmap_pages,
	.num_ranges     = ARRAY_SIZE(dac2563_regmap_pages),
};

static int dac2563_set_dai_fmt(struct snd_soc_dai *codec_dai,
			       unsigned int fmt)
{
	u8 iface_reg_1;
	struct snd_soc_component *component = codec_dai->component;
	iface_reg_1 = snd_soc_component_read32(component, TAS2563_TDM_CFG1);

	/* set master/slave audio interface */
	switch (fmt & SND_SOC_DAIFMT_MASTER_MASK) {
		case SND_SOC_DAIFMT_CBS_CFS:
		break;
		case SND_SOC_DAIFMT_CBM_CFM:
		break;
		default:
		dev_err(component->dev,
			"tas563: invalid DAI master/slave interface\n");
		return -EINVAL;
	}
	/* set I2S/DSP/RJF/LJF mode */
	switch (fmt & SND_SOC_DAIFMT_FORMAT_MASK) {
		case SND_SOC_DAIFMT_I2S:
		break;
		case SND_SOC_DAIFMT_DSP_A:
			iface_reg_1 &= ~TAS2563_RX_OFFSET_MASK;
			iface_reg_1 |= TAS2563_RX_OFFSET_1;
		break;
		/* todo */
		case SND_SOC_DAIFMT_DSP_B:
		break;
		case SND_SOC_DAIFMT_RIGHT_J:
		break;
		case SND_SOC_DAIFMT_LEFT_J:
		break;
		default:
		dev_err(component->dev,
			"tas2563: invalid DAI interface format\n");
		return -EINVAL;
	}
	snd_soc_component_write(component, TAS2563_TDM_CFG1, iface_reg_1);

	return 0;
}

static int dac2563_hw_params(struct snd_pcm_substream *substream,
	                           struct snd_pcm_hw_params *params,
	                           struct snd_soc_dai *dai)
{
	struct snd_soc_component *component = dai->component;

	snd_soc_component_update_bits(component, TAS2563_PWR_CTL,
					TAS2563_PWR_MODE_MASK,TAS2563_PWR_MODE_ACTIVE);
	snd_soc_component_update_bits(component, TAS2563_TDM_CFG2,
					TAS2563_RX_WLEN_MASK,TAS2563_RX_WLEN_32BITS);
	snd_soc_component_update_bits(component, TAS2563_PB_CFG1,
					TAS2563_AMP_LEVEL_MASK,TAS2563_AMP_LEVEL_9DB);

	return 0;
}

static int dac2563_mute(struct snd_soc_dai *dai, int mute)
{
	struct snd_soc_component *component = dai->component;
	u8 dac_reg;
	dac_reg = snd_soc_component_read32(component, TAS2563_PWR_CTL);
	if (mute)
		snd_soc_component_write(component, TAS2563_PWR_CTL, dac_reg | 
						TAS2563_PWR_MODE_MUTE);
	else {
		dac_reg &= ~TAS2563_PWR_MODE_MASK;
		snd_soc_component_write(component, TAS2563_PWR_CTL, dac_reg);
	}
	return 0;
}

static void dac2563_shutdown(struct snd_pcm_substream *substream,
	                           struct snd_soc_dai *dai)
{
	struct snd_soc_component *component = dai->component;
	snd_soc_component_update_bits(component,
				      TAS2563_PWR_CTL,
				      TAS2563_PWR_MODE_MASK,
				      TAS2563_PWR_MODE_SOFTDOWN);
	return;
}

static const struct snd_soc_dai_ops dac2563_ops = {
	.hw_params    = dac2563_hw_params,
	.digital_mute = dac2563_mute,
	.set_fmt      = dac2563_set_dai_fmt,
	.shutdown     = dac2563_shutdown,
};

static struct snd_soc_dai_driver dac2563_dai = {
	.name     = "tas2563-hifi",
	.playback = {
		.stream_name  = "Playback",
		.channels_min = 1,
		.channels_max = 2,
		.rates        = TAS2563_RATES,
		.formats      = TAS2563_FORMATS,
	},
	.ops                  = &dac2563_ops,
	.symmetric_rates      = 1,
};

static int dac2563_probe(struct snd_soc_component *component)
{
	int i,j;
	snd_soc_component_write(component, TAS2563_RESET, 0x01);
	snd_soc_component_update_bits(component, TAS2563_TDM_CFG2,
					TAS2563_RX_SCFG_MASK,TAS2563_RX_SCFG_STEREO_MIX);
	snd_soc_component_update_bits(component, TAS2563_TDM_CFG0,
					TAS2563_FS_START_MASK,TAS2563_FS_START_L2H);
	for (i=0;i<sizeof(digital_gain) / sizeof(digital_gain[0]);i++) {
		if (digital_gain[i][0] == DIGITAL_GAIN_INIT ) {
			for(j=0;j<TAS2563_DVC_PCM_LEN;j++)
				snd_soc_component_write(component, TAS2563_DVC_PCM_0+j,
				digital_gain[i][j+1]);
		}
	}
	return 0;
}

static struct snd_soc_component_driver soc_component_dev_dac2563 = {
	.probe = dac2563_probe,
};

static int dac2563_i2c_probe(struct i2c_client *i2c,
	                     const struct i2c_device_id *id)
{
	struct tas2563_priv *dac2563;
	int ret;
	dac2563 = devm_kzalloc(&i2c->dev, sizeof(*dac2563),GFP_KERNEL);
	if (dac2563 == NULL)
		return -ENOMEM;

	dac2563->regmap = devm_regmap_init_i2c(i2c, &dac2563_regmap);
	if (IS_ERR(dac2563->regmap))
		return PTR_ERR(dac2563->regmap);

	i2c_set_clientdata(i2c, dac2563);

	ret = devm_snd_soc_register_component(&i2c->dev,
					&soc_component_dev_dac2563,
					&dac2563_dai,
					1);

	if (ret) {
		dev_err(&i2c->dev, "Failed to register codec\n");
		return ret;
	}

	dev_notice(&i2c->dev,"%s successfully probed!\n",  __FUNCTION__);

	return 0;
}

static const struct i2c_device_id dac2563_i2c_id[] = {
	{ "tas2563amp", 0 },
	{ }
};
MODULE_DEVICE_TABLE(i2c, dac2563_i2c_id);

static struct i2c_driver dac2563_i2c_driver = {
	.driver   = {
		.name  = "tas2563",
		.owner = THIS_MODULE,
	},
	.probe    = dac2563_i2c_probe,
	.id_table = dac2563_i2c_id,
};

module_i2c_driver(dac2563_i2c_driver);

MODULE_DESCRIPTION("ASoC tas2563 codec driver");
MODULE_AUTHOR("Kevin Li <kevin-ke.li@broadcom.com>");
MODULE_LICENSE("GPL");
