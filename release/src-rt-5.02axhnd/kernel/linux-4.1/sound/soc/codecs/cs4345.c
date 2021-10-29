/*********************************************************************
 * cs4345.c  --   ASoC Driver for Cirrus Logic CS4345 codecs
 *
 * Author: Kevin Li <kevin-ke.li@broadcom.com>
 * 
 * Copyright (c) 2018 Broadcom Corporation
 * All Rights Reserved
 *
 * <:label-BRCM:2018:DUAL/GPL:standard
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License, version 2, as published by
 * the Free Software Foundation (the "GPL").
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * 
 * A copy of the GPL is available at http://www.broadcom.com/licenses/GPLv2.php, or by
 * writing to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 * 
 * :>
 **********************************************************************/

#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/slab.h>
#include <sound/soc.h>
#include <sound/pcm.h>
#include <sound/initval.h>
#include <linux/of.h>

#define DRV_NAME "brcm-cs4345"

#define STUB_RATES	SNDRV_PCM_RATE_8000_192000

#define STUB_FORMATS	(SNDRV_PCM_FMTBIT_S16_LE | \
                         SNDRV_PCM_FMTBIT_S24_LE | \
                         SNDRV_PCM_FMTBIT_S32_LE)

static struct snd_soc_codec_driver soc_codec_cs4345 = {
};

static int cs4345_dai_startup(struct snd_pcm_substream *substream,
                              struct snd_soc_dai *dai)
{
   return 0;
}
static int cs4345_hw_params(struct snd_pcm_substream *substream,
                            struct snd_pcm_hw_params *params,
                            struct snd_soc_dai *dai)
{
   return 0;
}

static int cs4345_set_fmt(struct snd_soc_dai *dai, unsigned int fmt)
{
   return 0;
}

static const struct snd_soc_dai_ops cs4345_dai_ops = {
   .startup   = cs4345_dai_startup,
   .hw_params = cs4345_hw_params,
   .set_fmt   = cs4345_set_fmt,
};

static struct snd_soc_dai_driver cs4345_dai = {
   .name            = "cs4345-hifi",
   .playback 	    = 
   {
     .stream_name   = "Playback",
     .channels_min  = 2,
     .channels_max  = 2,
     .rates         = STUB_RATES,
     .formats       = STUB_FORMATS,
   },
     .ops           = &cs4345_dai_ops,
};

static int cs4345_probe(struct platform_device *pdev)
{
   return snd_soc_register_codec(&pdev->dev, &soc_codec_cs4345, &cs4345_dai, 1);
}

static int cs4345_remove(struct platform_device *pdev)
{
   snd_soc_unregister_codec(&pdev->dev);
   return 0;
}

#ifdef CONFIG_OF
static const struct of_device_id snd_soc_bcm_audio_match[] = 
{
	{ .compatible   = "crus,cs4345-dac" },
	{ }
};
#endif
static struct platform_driver cs4345_driver = {
   .probe             = cs4345_probe,
   .remove            = cs4345_remove,
   .driver            = 
   {
      .name           = "cs4345",
      .of_match_table = of_match_ptr(snd_soc_bcm_audio_match),
   },
};

module_platform_driver(cs4345_driver);

MODULE_AUTHOR("Kevin Li <kevin-ke.li@broadcom.com>");
MODULE_DESCRIPTION("BCM CS4345 codec driver");
MODULE_LICENSE("GPL");
MODULE_ALIAS("platform:" DRV_NAME);
