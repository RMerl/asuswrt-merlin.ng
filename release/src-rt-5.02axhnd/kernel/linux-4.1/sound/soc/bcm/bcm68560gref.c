/*********************************************************************
 * bcm68360gref-pcm5100.c -- ALSA SoC machine driver for BCM98360GREF board 
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
 ********************************************************************/

#include <linux/module.h>
#include <sound/soc.h>
#include <sound/pcm_params.h>

static int pcm5100_hw_params(struct snd_pcm_substream *substream,
                             struct snd_pcm_hw_params *params)
{
   int ret = 0;
   struct snd_soc_pcm_runtime *rtd = substream->private_data;
   struct snd_soc_dai *cpu_dai     = rtd->cpu_dai;

   /* Set the AP DAI configuration */
   ret = snd_soc_dai_set_fmt(cpu_dai, SND_SOC_DAIFMT_I2S   |
                                      SND_SOC_DAIFMT_NB_NF |
                                      SND_SOC_DAIFMT_CBS_CFS);
   if (ret < 0)
   {
      return ret;
   }

   ret = snd_soc_dai_set_sysclk(cpu_dai, 0, params_rate(params), SND_SOC_CLOCK_OUT);
   if (ret < 0)
   {
      return ret;
   }

   return 0;
}

static struct snd_soc_ops pcm5100_ops = 
{
   .hw_params = pcm5100_hw_params,
};

static struct snd_soc_dai_link bcm63xx_soc_card_dai[] = 
{
   {
      .name           = "bcm68560pcm5100", //This is chosen arbitrarily.  Can be anything.
      .stream_name    = "Playback",
      .codec_dai_name = "pcm5100-hifi",
      .cpu_dai_name   = "ff802080.bcm63xx-i2s",
      .platform_name  = "bcm63xx-pcm-audio",
      .codec_name     = "pcm5100",
      .ops            = &pcm5100_ops,
      .dai_fmt 	      = (SND_SOC_DAIFMT_CBS_CFS | SND_SOC_DAIFMT_I2S | SND_SOC_DAIFMT_IB_NF),	
	},
};

static struct snd_soc_card snd_soc_bcm63xx_pcm5100 = 
{
   .name      = "BCM68360GREF",	
   .owner     = THIS_MODULE,
   .dai_link  = bcm63xx_soc_card_dai,
   .num_links = 1,
};

static struct platform_device *bcm63xx_snd_device;

static int __init bcm63xx_audio_init(void)
{
   int ret;
   bcm63xx_snd_device = platform_device_alloc("soc-audio", -1);
   if (!bcm63xx_snd_device)
   {
      return -ENOMEM;
   }

   platform_set_drvdata(bcm63xx_snd_device, &snd_soc_bcm63xx_pcm5100);
  
   ret = platform_device_add(bcm63xx_snd_device);
   if (ret)
   {
      platform_device_put(bcm63xx_snd_device);
   }

   return ret;
}
module_init(bcm63xx_audio_init);

static void __exit bcm63xx_audio_exit(void)
{
   platform_device_unregister(bcm63xx_snd_device);
}
module_exit(bcm63xx_audio_exit);

MODULE_AUTHOR("Kevin Li kevin-ke.li@broadcom.com");
MODULE_DESCRIPTION("ALSA SoC BCM 68360GREF-PCM5100");
MODULE_LICENSE("GPL");
