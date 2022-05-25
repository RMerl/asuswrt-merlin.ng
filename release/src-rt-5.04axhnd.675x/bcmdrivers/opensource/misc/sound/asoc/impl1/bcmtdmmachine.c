/*********************************************************************
 * bcm63xxx machine driver.c -- ALSA SoC machine driver for BCM963146/4915 board 
 * 
 * Copyright (c) 2020 Broadcom Corporation
 * All Rights Reserved
 *
 * <:label-BRCM:2020:DUAL/GPL:standard
 * 
 * Unless you and Broadcom execute a separate written software license
 * agreement governing use of this software, this software is licensed
 * to you under the terms of the GNU General Public License version 2
 * (the "GPL"), available at http://www.broadcom.com/licenses/GPLv2.php,
 * with the following added to such license:
 * 
 *    As a special exception, the copyright holders of this software give
 *    you permission to link this software with independent modules, and
 *    to copy and distribute the resulting executable under terms of your
 *    choice, provided that you also meet, for each linked independent
 *    module, the terms and conditions of the license of that module.
 *    An independent module is a module which is not derived from this
 *    software.  The special exception does not apply to any modifications
 *    of the software.
 * 
 * Not withstanding the above, under no circumstances may you combine
 * this software in any way with any other Broadcom software provided
 * under a license other than the GPL, without Broadcom's express prior
 * written consent.
 * 
 * :>
 ********************************************************************/
#include <linux/module.h>
#include <sound/pcm_params.h>
#include <sound/soc.h>
#include <bcm_pinmux.h>
#include <boardparms.h>
#include <board.h>
#include "bcm63xx-i2stdm.h"

#define SCLK_BIT_PER_CHANNEL	32
#define MCLK_RATIO		(params_rate(params)<=48000 ? 8 : 1)
#define MCLK_RATIO_FACTOR	2

static int tlv320dac3203_hw_params(struct snd_pcm_substream *substream,
			struct snd_pcm_hw_params *params)
{
	int ret = 0;
	unsigned int mclk; 
	struct snd_soc_pcm_runtime *rtd = substream->private_data;
	struct snd_soc_dai *codec_dai = rtd->codec_dai;
	struct device *dev = rtd->card->dev;

	mclk = params_rate(params)*SCLK_BIT_PER_CHANNEL*
			2*MCLK_RATIO*MCLK_RATIO_FACTOR;

	/* set the codec system clock */
	ret = snd_soc_dai_set_sysclk(codec_dai,0,
			mclk,SND_SOC_CLOCK_OUT);
	if (ret < 0) {
		dev_err(dev, "%s Failed to set codec dai format.\n",
			__func__);
		return ret;
	}
	return 0;
}

static int tlv320adc5140_hw_params(struct snd_pcm_substream *substream,
			struct snd_pcm_hw_params *params)
{
	int ret = 0,slots = 8;
	unsigned int width = 32;
	unsigned int mask = I2S_TDM_VALID_SLOT_MASK | I2S_BIT_PER_SLOT_MASK;
	unsigned int channels;
	struct snd_soc_pcm_runtime *rtd = substream->private_data;
	struct snd_soc_dai *cpu_dai = rtd->cpu_dai;
	struct snd_soc_dai *codec_dai = rtd->codec_dai;
	struct device *dev = rtd->card->dev;

	channels = params_channels(params);
	ret = snd_soc_dai_set_tdm_slot(cpu_dai, mask, mask, channels, width);
	if (ret < 0) {
		dev_err(dev, "%s Failed to set cpu dai tdm time slot.\n",
		__func__);
		return ret;
	}

	ret = snd_soc_dai_set_tdm_slot(codec_dai, mask, mask, slots, width);
	if (ret < 0) {
		dev_err(dev, "%s Failed to set codec dai tdm time slot.\n",
			__func__);
		return ret;
	}

	return 0;
}

static struct snd_soc_ops tlv320dac3203_ops = {
	.hw_params = tlv320dac3203_hw_params,
};

static struct snd_soc_ops tlv320adc5140_ops = {
	.hw_params = tlv320adc5140_hw_params,
};

static struct snd_soc_dai_link bcm63xx_soc_card_dai[] = 
{
	{
		.name		= "Maple Speaker",
		.stream_name	= "Playback",
		.platform_name	= "ff802080.bcm63xx-i2s",
		.cpu_dai_name	= "merritt-cpu-dai",
		.codec_dai_name	= "tlv320dac3203-hifi",
#if defined(CONFIG_BCM96756)
		.codec_name	= "tlv320dac3203.0-0018",
#else
		.codec_name	= "tlv320dac3203.1-0018",
#endif
		.ops		= &tlv320dac3203_ops,
		.dai_fmt	= (SND_SOC_DAIFMT_I2S |
				   SND_SOC_DAIFMT_CBS_CFS |
				   SND_SOC_DAIFMT_NB_NF),
	},
	{
		.name		= "Maple Mic Array",
		.stream_name	= "Capture",
		.platform_name	= "ff802080.bcm63xx-i2s",
		.cpu_dai_name	= "merritt-cpu-dai",
		.codec_dai_name	= "tlv320adc5140-hifi",
#if defined(CONFIG_BCM96756)
		.codec_name	= "tlv320adc5140.0-004e",
#else
		.codec_name	= "tlv320adc5140.1-004e",
#endif
		.ops		= &tlv320adc5140_ops,
		.dai_fmt	= (SND_SOC_DAIFMT_DSP_A |
				   SND_SOC_DAIFMT_CBS_CFS | 
				   SND_SOC_DAIFMT_NB_NF),
   },
};

static struct snd_soc_card snd_soc_bcm63xx_SoundCard = 
{
	.name		=	"MapleTree",	
	.owner		=	THIS_MODULE,
	.dai_link	=	bcm63xx_soc_card_dai,
	.num_links	=	ARRAY_SIZE(bcm63xx_soc_card_dai),
};

static struct platform_device *bcm63xx_snd_device;

static int __init bcm63xx_audio_init(void)
{
	int ret;
	bcm63xx_snd_device = platform_device_alloc("soc-audio", -1);
	if (!bcm63xx_snd_device)
		return -ENOMEM;

	platform_set_drvdata(bcm63xx_snd_device, &snd_soc_bcm63xx_SoundCard);

	ret = platform_device_add(bcm63xx_snd_device);
	if (ret)
		platform_device_put(bcm63xx_snd_device);

	return ret;
}

late_initcall(bcm63xx_audio_init);

static void __exit bcm63xx_audio_exit(void)
{
	platform_device_unregister(bcm63xx_snd_device);
}
module_exit(bcm63xx_audio_exit);

MODULE_AUTHOR("Kevin Li kevin-ke.li@broadcom.com");
MODULE_DESCRIPTION("ALSA SoC BCM Maple sound card machine driver");
MODULE_LICENSE("GPL");
