/*********************************************************************
 * pcm5100.c  --   ASoC Driver for TI PCM5100 codecs
 *
 * Author: Kevin Li <kevin-ke.li@broadcom.com>
 * 
 * Copyright (c) 2018 Broadcom Corporation
 * All Rights Reserved
 *
 * <:label-BRCM:2018:DUAL/GPL:standard
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
 **********************************************************************/

#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/slab.h>
#include <sound/soc.h>
#include <sound/pcm.h>
#include <sound/initval.h>
#include <linux/of.h>

#define DRV_NAME "brcm-pcm5100"

#define STUB_RATES	SNDRV_PCM_RATE_8000_192000
/* Based on the PCM5102a datasheet, change the format and rate parameter limits. The rate define is actually OK as is, but the formats need to be limited to S16LE, S24LE and S32LE.*/
#define STUB_FORMATS	(SNDRV_PCM_FMTBIT_S16_LE | \
                         SNDRV_PCM_FMTBIT_S24_LE | \
                         SNDRV_PCM_FMTBIT_S32_LE)

static struct snd_soc_codec_driver soc_codec_pcm5100 = {
};

static int pcm5100_dai_startup(struct snd_pcm_substream *substream,
                               struct snd_soc_dai *dai)
{
   return 0;
}
static int pcm5100_hw_params(struct snd_pcm_substream *substream,
                             struct snd_pcm_hw_params *params,
                             struct snd_soc_dai *dai)
{
   return 0;
}

static int pcm5100_set_fmt(struct snd_soc_dai *dai, unsigned int fmt)
{
   return 0;
}

static const struct snd_soc_dai_ops pcm5100_dai_ops = 
{
   .startup   = pcm5100_dai_startup,
   .hw_params = pcm5100_hw_params,
   .set_fmt   = pcm5100_set_fmt,
};

static struct snd_soc_dai_driver pcm5100_dai = 
{
   .name             = "pcm5100-hifi",
   .playback         = 
   {
      .stream_name   = "Playback",
      .channels_min  = 2,
      .channels_max  = 2,
      .rates         = STUB_RATES,
      .formats       = STUB_FORMATS,
   },
   .ops = &pcm5100_dai_ops,
};

static int pcm5100_probe(struct platform_device *pdev)
{
   return snd_soc_register_codec(&pdev->dev, &soc_codec_pcm5100, &pcm5100_dai, 1);
}

static int pcm5100_remove(struct platform_device *pdev)
{
   snd_soc_unregister_codec(&pdev->dev);
   return 0;
}

#ifdef CONFIG_OF
static const struct of_device_id snd_soc_bcm_audio_match[] = 
{
   { .compatible   = "ti,pcm5100-dac" },
   { }
};
#endif

static struct platform_driver pcm5100_driver = 
{
   .probe             = pcm5100_probe,
   .remove            = pcm5100_remove,
   .driver            = 
   {
      .name           = "pcm5100",
      .of_match_table = of_match_ptr(snd_soc_bcm_audio_match),
   },
};

module_platform_driver(pcm5100_driver);

MODULE_AUTHOR("Kevin Li <kevin-ke.li@broadcom.com>");
MODULE_DESCRIPTION("BCM PCM5100 codec driver");
MODULE_LICENSE("GPL");
MODULE_ALIAS("platform:" DRV_NAME);
