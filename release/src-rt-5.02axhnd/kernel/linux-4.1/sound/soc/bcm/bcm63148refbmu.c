/*********************************************************************
 * bcm63148refbmu-pcm5100.c -- ALSA SoC machine driver for BCM963148REFBMU board 
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
#include <sound/soc.h>
#include <sound/pcm_params.h>

static int pcm5100_hw_params(struct snd_pcm_substream *substream,
                             struct snd_pcm_hw_params *params)
{
   int ret = 0;
   struct snd_soc_pcm_runtime *rtd = substream->private_data;
   struct snd_soc_dai *cpu_dai = rtd->cpu_dai;

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

static struct snd_soc_ops pcm5100_ops = {
   .hw_params = pcm5100_hw_params,
};

static struct snd_soc_dai_link bcm63xx_soc_card_dai[] = 
{
   {
      .name           = "bcm63148pcm5100", //This is chosen arbitrarily.  Can be anything.
      .stream_name    = "Playback",
      .codec_dai_name = "pcm5100-hifi",
      .cpu_dai_name   = "fffe8900.bcm63xx-i2s",
      .platform_name  = "bcm63xx-pcm-audio",
      .codec_name     = "pcm5100",
      .ops            = &pcm5100_ops,
      .dai_fmt 	      = (SND_SOC_DAIFMT_CBS_CFS | SND_SOC_DAIFMT_I2S |
			                   SND_SOC_DAIFMT_IB_NF),	
   },
};

static struct snd_soc_card snd_soc_bcm63xx_pcm5100 = {
   .name      = "BCM63148REF_BMU",	
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
MODULE_DESCRIPTION("ALSA SoC BCM 63148REF_BMU");
MODULE_LICENSE("GPL");
