/*
 * ac97.c  --  ALSA Soc AC97 codec support
 *
 * Copyright 2005 Wolfson Microelectronics PLC.
 * Author: Liam Girdwood <lrg@slimlogic.co.uk>
 *
 *  This program is free software; you can redistribute  it and/or modify it
 *  under  the terms of  the GNU General  Public License as published by the
 *  Free Software Foundation;  either version 2 of the  License, or (at your
 *  option) any later version.
 *
 * Generic AC97 support.
 */

#include <linux/init.h>
#include <linux/slab.h>
#include <linux/kernel.h>
#include <linux/device.h>
#include <linux/module.h>
#include <sound/core.h>
#include <sound/pcm.h>
#include <sound/ac97_codec.h>
#include <sound/initval.h>
#include <sound/soc.h>

static const struct snd_soc_dapm_widget ac97_widgets[] = {
	SND_SOC_DAPM_INPUT("RX"),
	SND_SOC_DAPM_OUTPUT("TX"),
};

static const struct snd_soc_dapm_route ac97_routes[] = {
	{ "AC97 Capture", NULL, "RX" },
	{ "TX", NULL, "AC97 Playback" },
};

static int ac97_prepare(struct snd_pcm_substream *substream,
			struct snd_soc_dai *dai)
{
	struct snd_soc_codec *codec = dai->codec;
	struct snd_ac97 *ac97 = snd_soc_codec_get_drvdata(codec);

	int reg = (substream->stream == SNDRV_PCM_STREAM_PLAYBACK) ?
		  AC97_PCM_FRONT_DAC_RATE : AC97_PCM_LR_ADC_RATE;
	return snd_ac97_set_rate(ac97, reg, substream->runtime->rate);
}

#define STD_AC97_RATES (SNDRV_PCM_RATE_8000 | SNDRV_PCM_RATE_11025 |\
		SNDRV_PCM_RATE_22050 | SNDRV_PCM_RATE_44100 |\
		SNDRV_PCM_RATE_48000)

static const struct snd_soc_dai_ops ac97_dai_ops = {
	.prepare	= ac97_prepare,
};

static struct snd_soc_dai_driver ac97_dai = {
	.name = "ac97-hifi",
	.playback = {
		.stream_name = "AC97 Playback",
		.channels_min = 1,
		.channels_max = 2,
		.rates = STD_AC97_RATES,
		.formats = SND_SOC_STD_AC97_FMTS,},
	.capture = {
		.stream_name = "AC97 Capture",
		.channels_min = 1,
		.channels_max = 2,
		.rates = STD_AC97_RATES,
		.formats = SND_SOC_STD_AC97_FMTS,},
	.ops = &ac97_dai_ops,
};

static int ac97_soc_probe(struct snd_soc_codec *codec)
{
	struct snd_ac97 *ac97;
	struct snd_ac97_bus *ac97_bus;
	struct snd_ac97_template ac97_template;
	int ret;

	/* add codec as bus device for standard ac97 */
	ret = snd_ac97_bus(codec->component.card->snd_card, 0, soc_ac97_ops,
			   NULL, &ac97_bus);
	if (ret < 0)
		return ret;

	memset(&ac97_template, 0, sizeof(struct snd_ac97_template));
	ret = snd_ac97_mixer(ac97_bus, &ac97_template, &ac97);
	if (ret < 0)
		return ret;

	snd_soc_codec_set_drvdata(codec, ac97);

	return 0;
}

#ifdef CONFIG_PM
static int ac97_soc_suspend(struct snd_soc_codec *codec)
{
	struct snd_ac97 *ac97 = snd_soc_codec_get_drvdata(codec);

	snd_ac97_suspend(ac97);

	return 0;
}

static int ac97_soc_resume(struct snd_soc_codec *codec)
{

	struct snd_ac97 *ac97 = snd_soc_codec_get_drvdata(codec);

	snd_ac97_resume(ac97);

	return 0;
}
#else
#define ac97_soc_suspend NULL
#define ac97_soc_resume NULL
#endif

static struct snd_soc_codec_driver soc_codec_dev_ac97 = {
	.probe = 	ac97_soc_probe,
	.suspend =	ac97_soc_suspend,
	.resume =	ac97_soc_resume,

	.dapm_widgets = ac97_widgets,
	.num_dapm_widgets = ARRAY_SIZE(ac97_widgets),
	.dapm_routes = ac97_routes,
	.num_dapm_routes = ARRAY_SIZE(ac97_routes),
};

static int ac97_probe(struct platform_device *pdev)
{
	return snd_soc_register_codec(&pdev->dev,
			&soc_codec_dev_ac97, &ac97_dai, 1);
}

static int ac97_remove(struct platform_device *pdev)
{
	snd_soc_unregister_codec(&pdev->dev);
	return 0;
}

static struct platform_driver ac97_codec_driver = {
	.driver = {
		.name = "ac97-codec",
	},

	.probe = ac97_probe,
	.remove = ac97_remove,
};

module_platform_driver(ac97_codec_driver);

MODULE_DESCRIPTION("Soc Generic AC97 driver");
MODULE_AUTHOR("Liam Girdwood");
MODULE_LICENSE("GPL");
MODULE_ALIAS("platform:ac97-codec");
