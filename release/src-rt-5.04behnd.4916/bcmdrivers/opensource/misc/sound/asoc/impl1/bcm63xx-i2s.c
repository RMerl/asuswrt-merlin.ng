/*********************************************************************
 * bcm63xx-i2s.c  --  ALSA SoC Audio Layer - Broadcom I2S Controller driver
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
 

#include <linux/clk.h>
#include <linux/delay.h>
#include <linux/device.h>
#include <linux/io.h>
#include <linux/module.h>
#include <linux/regmap.h>
#include <sound/pcm_params.h>
#include <sound/soc.h>

#include "bcm63xx-i2s.h"
#include "bcm63148clk.h"

#define DRV_NAME "brcm-i2s"
#define BCM63XX_I2S_RATES_BASIC  \
  SNDRV_PCM_RATE_16000 | SNDRV_PCM_RATE_32000 | SNDRV_PCM_RATE_44100 | \
	SNDRV_PCM_RATE_48000 | SNDRV_PCM_RATE_96000 | SNDRV_PCM_RATE_192000 
#define BCM63XX_I2S_RATES_EXT  \
  SNDRV_PCM_RATE_8000  | SNDRV_PCM_RATE_11025 | SNDRV_PCM_RATE_22050

#define BCM63XX_I2S_RATES  BCM63XX_I2S_RATES_BASIC 

#if defined( CONFIG_BCM96856)
static struct clk    *i2s_clk;
#endif

struct regmap *regmap_i2s;
static bool brcm_i2s_wr_reg(struct device *dev, unsigned int reg)
{
	 switch (reg) 
	 {
	   case I2S_TX_CFG:
	   case I2S_TX_IRQ_CTL:
	   case I2S_TX_IRQ_EN:
	   case I2S_TX_IRQ_IFF_THLD:
	   case I2S_TX_IRQ_OFF_THLD:
	   case I2S_TX_DESC_IFF_ADDR:
	   case I2S_TX_DESC_IFF_LEN:
		   return true;
	   default:
		   return false;
	}
}
static bool brcm_i2s_rd_reg(struct device *dev, unsigned int reg)
{
   switch (reg) 
   {
     case I2S_TX_CFG:
     case I2S_TX_IRQ_CTL:
	   case I2S_TX_IRQ_EN:
	   case I2S_TX_IRQ_IFF_THLD:
	   case I2S_TX_IRQ_OFF_THLD:
	   case I2S_TX_DESC_IFF_ADDR:
	   case I2S_TX_DESC_IFF_LEN:
	   case I2S_TX_DESC_OFF_ADDR:
	   case I2S_TX_DESC_OFF_LEN:
		   return true;
	   default:
		   return false;
	}
}

static bool brcm_i2s_volatile_reg(struct device *dev, unsigned int reg)
{
	 switch (reg) 
	 {
     case I2S_TX_IRQ_CTL:
     case I2S_TX_DESC_OFF_ADDR:
     case I2S_TX_DESC_OFF_LEN:
     case I2S_TX_DESC_IFF_LEN:
     case I2S_TX_DESC_IFF_ADDR:
		   return true;
	   default:
		   return false;
	}
}

static const struct regmap_config brcm_i2s_regmap_config = {
	 .reg_bits     = 32,
	 .reg_stride   = 4,
	 .val_bits     = 32,
	 .max_register = I2S_REG_MAX,
	 .writeable_reg= brcm_i2s_wr_reg,
	 .readable_reg = brcm_i2s_rd_reg,
	 .volatile_reg = brcm_i2s_volatile_reg,
//	.precious_reg = brcm_i2s_precious_reg,
	 .cache_type   = REGCACHE_FLAT,
};

/* Set I2S DAI format */
static int bcm63xx_i2s_set_fmt(struct snd_soc_dai *cpu_dai, unsigned int fmt)
{
   switch (fmt & SND_SOC_DAIFMT_MASTER_MASK) 
   {
     case SND_SOC_DAIFMT_CBS_CFS:    // todo:
       break;
     default:
       return -EINVAL;
	 }
   switch (fmt & SND_SOC_DAIFMT_INV_MASK) 
   {
     case SND_SOC_DAIFMT_NB_NF:
        // Set both bit clock and lrclk failling edge  
        regmap_update_bits(regmap_i2s, I2S_TX_CFG, I2S_SCLK_POLARITY | I2S_LRCK_POLARITY, 0 );
		    break;
     default:
        return -EINVAL;
   }
	return 0;
}

static int bcm63xx_i2s_hw_params(struct snd_pcm_substream *substream,
				struct snd_pcm_hw_params *params,
				struct snd_soc_dai *dai)
{
   switch (params_format(params)) 
	 {
     case SNDRV_PCM_FORMAT_S32_LE:
       regmap_update_bits(regmap_i2s, I2S_TX_CFG, I2S_SCLKS_PER_1FS_DIV32_MASK, I2S_64BITS_PERFRAME );
#if defined( CONFIG_BCM96856)
       clk_set_rate(i2s_clk, params_rate(params));
#else
       set_i2s_clk( SNDRV_PCM_FORMAT_S32_LE, params_rate(params) );
#endif
       break;		
     default:
       dev_err(dai->dev, "Format unsupported\n");
     return -EINVAL;
   }
   return 0;
}

static int bcm63xx_i2s_startup(struct snd_pcm_substream *substream,
           struct snd_soc_dai *dai)
{
   struct snd_soc_pcm_runtime *rtd = substream->private_data;
   struct snd_soc_dai *cpu_dai = rtd->cpu_dai;

   if (!cpu_dai->active)
   {
      regmap_write(regmap_i2s, I2S_TX_CFG, 0);// clear cfg reg 
      /* Setup I2S as follows ( Fs = sampling frequency ):                    *
       * 64Fs BCLK, leftChannel=0, rightchannel=1, falling BCLK,LRCLK low for *
       * left, Data delayed by 1 BCLK from LRCLK transition, MSB justified    */
      regmap_update_bits(regmap_i2s, I2S_TX_CFG, I2S_OUT_R | I2S_DATA_ALIGNMENT |I2S_DATA_ENABLE | I2S_CLOCK_ENABLE,
                                     I2S_OUT_R | I2S_DATA_ALIGNMENT |I2S_DATA_ENABLE | I2S_CLOCK_ENABLE ); 

      regmap_write(regmap_i2s, I2S_TX_IRQ_CTL, 0);// clear irq reg 
      regmap_write(regmap_i2s, I2S_TX_IRQ_EN,  0);// clear irq en

      regmap_write(regmap_i2s, I2S_TX_IRQ_IFF_THLD, 0);// clear 
      regmap_write(regmap_i2s, I2S_TX_IRQ_OFF_THLD, 0);// clear  
      
      /* Enable off interrupts - interrupt when output fifo level is over 0 */
      regmap_update_bits(regmap_i2s, I2S_TX_IRQ_EN,  I2S_DESC_INTR_TYPE_SEL | I2S_DESC_OFF_INTR_EN,
                                             ~I2S_DESC_INTR_TYPE_SEL | I2S_DESC_OFF_INTR_EN );       
   }
   return 0;
}

static void bcm63xx_i2s_shutdown(struct snd_pcm_substream *substream,
                                 struct snd_soc_dai *dai)
{
   /* Disable I2S interface */
   regmap_update_bits(regmap_i2s, I2S_TX_CFG, I2S_ENABLE, 0 );      
   /* Clear and disable I2S interrupts ( by writing 0 ) */
   regmap_update_bits(regmap_i2s, I2S_TX_IRQ_CTL, I2S_INTR_MASK, 0 ); 
   regmap_write(regmap_i2s, I2S_TX_IRQ_EN, 0);// clear cfg reg 
   return ;
}

static const struct snd_soc_dai_ops bcm63xx_i2s_dai_ops = {
   .startup    = bcm63xx_i2s_startup,
   .shutdown   = bcm63xx_i2s_shutdown,
   .hw_params  = bcm63xx_i2s_hw_params,
   .set_fmt    = bcm63xx_i2s_set_fmt,
};

static struct snd_soc_dai_driver bcm63xx_i2s_dai = {
   .name     = DRV_NAME,
   .playback = 
   {
      .channels_min = 2,
      .channels_max = 2,
      .rates        = BCM63XX_I2S_RATES,
      .formats      = SNDRV_PCM_FMTBIT_S32_LE,
   },
   .ops      = &bcm63xx_i2s_dai_ops,
};

static const struct snd_soc_component_driver bcm63xx_i2s_component = {
   .name		= "bcm63xx",
};

static int bcm63xx_i2s_dev_probe(struct platform_device *pdev)
{
   int ret = 0;
   void __iomem *regs;
   struct resource *r_mem, *region;

#if defined( CONFIG_BCM96856)
   i2s_clk = devm_clk_get(&pdev->dev, "i2sclk");
   if (IS_ERR(i2s_clk)) {
      dev_err(&pdev->dev, "%s: cannot get a brcm clock: %ld\n", __func__,PTR_ERR(i2s_clk));
      return PTR_ERR(i2s_clk);
   }
#endif
 
   r_mem = platform_get_resource(pdev, IORESOURCE_MEM, 0);
   if (!r_mem)
   {
      dev_err(&pdev->dev, "Unable to get register resource.\n");
      return -ENODEV;
   }

   region = devm_request_mem_region(&pdev->dev, r_mem->start,
                                    resource_size(r_mem), DRV_NAME);
   if (!region)
   {
      dev_err(&pdev->dev, "Memory region already claimed\n");
      return -EBUSY;
   }

   regs = devm_ioremap_nocache(&pdev->dev,r_mem->start, resource_size(r_mem));
   regmap_i2s = devm_regmap_init_mmio(&pdev->dev, regs, &brcm_i2s_regmap_config);
   if(!regmap_i2s)
   {
      dev_err(&pdev->dev,"Failed to initialise managed register map\n");
   }

   ret = devm_snd_soc_register_component(&pdev->dev,
                                         &bcm63xx_i2s_component,
                                         &bcm63xx_i2s_dai, 1);
   if (ret)
   {
      dev_err(&pdev->dev, "failed to register the dai\n");
      return ret;
   }

   return ret;
}

#ifdef CONFIG_OF
static const struct of_device_id snd_soc_bcm_audio_match[] = 
{
	{ .compatible   = "brcm,bcm63xx-i2s" },
	{ }
};
#endif

static struct platform_driver bcm63xx_i2s_driver = {
   .driver =
   {
      .name = DRV_NAME,
      .of_match_table = of_match_ptr(snd_soc_bcm_audio_match),
   },
   .probe  = bcm63xx_i2s_dev_probe,
};

module_platform_driver(bcm63xx_i2s_driver);

/* Module information */
MODULE_AUTHOR("Kevin,Li <kevin-ke.li@broadcom.com>");
MODULE_DESCRIPTION("Broadcom DSL XPON ASOC I2S Interface");
MODULE_LICENSE("GPL");
