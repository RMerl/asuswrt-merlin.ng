 /**************************************************************
 * bcm63xx-pcm.c  --  ALSA SoC Audio Layer - Broadcom PCM-Controller driver
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
 */

#include <linux/dma-mapping.h>
#include <linux/module.h>
#include <sound/pcm.h>
#include <sound/pcm_params.h>
#include <linux/regmap.h>
#include <sound/soc.h>

#include "bcm63xx-i2s.h"

#define DRV_NAME "brcm-pcm"
extern struct regmap   *regmap_i2s;
static struct resource *r_irq;

#if defined(CONFIG_BCM963148) || defined(CONFIG_BCM963138)
static struct resource bcm63138_i2s_resources[] = {
	[0] =	{
			.start = INTERRUPT_ID_I2S,
			.end   = INTERRUPT_ID_I2S,
			.flags = IORESOURCE_IRQ,
		},
};
#endif

struct i2s_dma_desc
{
   char *         buffer_addr;      /* Buffer address */
   dma_addr_t     dma_addr;         /* DMA address to be passed to h/w */  
   unsigned int   dma_len;          /* Length of dma transfer */ 
};

struct bcm63xx_runtime_data {
   int periodsize_inbyte;
   int dma_len;
   dma_addr_t dma_addr;
   dma_addr_t dma_addr_next;
};

static const struct snd_pcm_hardware bcm63xx_pcm_hardware = {
   .info	  = SNDRV_PCM_INFO_MMAP |
              SNDRV_PCM_INFO_MMAP_VALID |
              SNDRV_PCM_INFO_INTERLEAVED |
              SNDRV_PCM_INFO_PAUSE |
              SNDRV_PCM_INFO_RESUME,
   .formats = SNDRV_PCM_FMTBIT_S32_LE,/* support S32 only */					
   .period_bytes_max  = 8192 - 32,
   .periods_min       = 1,
   .periods_max       = PAGE_SIZE/sizeof(struct i2s_dma_desc),
   .buffer_bytes_max	= 128 * 1024,
   .fifo_size         = 32,
};

struct i2s_dma_desc   *pdma_desc = NULL;

static int bcm63xx_pcm_hw_params(struct snd_pcm_substream *substream,struct snd_pcm_hw_params *params)
{
   struct snd_pcm_runtime      *runtime = substream->runtime;
   struct bcm63xx_runtime_data *prtd    = substream->runtime->private_data;
   struct device *dev                   = substream->pcm->card->dev;

   snd_pcm_set_runtime_buffer(substream, &substream->dma_buffer);
   runtime->dma_bytes = params_buffer_bytes(params);

   prtd->periodsize_inbyte = params_period_bytes(params); // Keep periodsize_inbyte to private 

   if(!pdma_desc)
   {
      pdma_desc = kzalloc( sizeof(struct i2s_dma_desc), GFP_NOWAIT );
      if( !pdma_desc )
      {
       	  dev_err(dev, "Allocate new descriptor memory failed\n");
          return -ENOMEM;
      }
    }

   return 0;
}

static int bcm63xx_pcm_hw_free(struct snd_pcm_substream *substream)
{
   if( pdma_desc )
   {
      kfree(pdma_desc);
   }
   pdma_desc = NULL;

   snd_pcm_set_runtime_buffer(substream, NULL);
    
   return 0;
}

static int bcm63xx_pcm_trigger(struct snd_pcm_substream *substream, int cmd)
{
   int ret = 0;
   switch (cmd) 
   {
     case SNDRV_PCM_TRIGGER_START:
       regmap_update_bits(regmap_i2s, I2S_TX_IRQ_EN,  I2S_DESC_OFF_INTR_EN,I2S_DESC_OFF_INTR_EN ); 
       regmap_update_bits(regmap_i2s, I2S_TX_CFG,     I2S_ENABLE ,         I2S_ENABLE);
     break;
     case SNDRV_PCM_TRIGGER_STOP:
     case SNDRV_PCM_TRIGGER_SUSPEND:
     case SNDRV_PCM_TRIGGER_PAUSE_PUSH:
       regmap_update_bits(regmap_i2s, I2S_TX_IRQ_CTL,  I2S_INTR_MASK, 0 ); 
       regmap_write(regmap_i2s, I2S_TX_IRQ_EN,  0);// clear irq en
       regmap_update_bits(regmap_i2s, I2S_TX_CFG, I2S_ENABLE,0 ); 
     break;
     default:
       ret = -EINVAL;
   }
   return ret;
}

static int bcm63xx_pcm_prepare(struct snd_pcm_substream *substream)
{
   struct snd_pcm_runtime *runtime = substream->runtime;
   struct bcm63xx_runtime_data *prtd = runtime->private_data;
  
   pdma_desc->dma_len  = prtd->periodsize_inbyte;
   pdma_desc->dma_addr = runtime->dma_addr;     // point to ring buffer start point
    
   regmap_write(regmap_i2s, I2S_TX_DESC_IFF_LEN,  pdma_desc->dma_len); // push to descriptor fifo
   regmap_write(regmap_i2s, I2S_TX_DESC_IFF_ADDR,  pdma_desc->dma_addr); // push to descriptor fifo

   return 0;
}

static snd_pcm_uframes_t bcm63xx_pcm_pointer(struct snd_pcm_substream *substream)
{
   struct bcm63xx_runtime_data *prtd = substream->runtime->private_data;
   
   if((void *) prtd->dma_addr_next == NULL)
   	 prtd->dma_addr_next = substream->runtime->dma_addr;

   snd_pcm_uframes_t x = bytes_to_frames(substream->runtime, prtd->dma_addr_next - substream->runtime->dma_addr);

   return x == substream->runtime->buffer_size ? 0 : x;
}

static int bcm63xx_pcm_mmap(struct snd_pcm_substream *substream,	struct vm_area_struct *vma)
{
   struct snd_pcm_runtime *runtime = substream->runtime;
   return dma_mmap_writecombine(substream->pcm->card->dev, vma,
                                runtime->dma_area,
                                runtime->dma_addr,
                                runtime->dma_bytes);
}

static int bcm63xx_pcm_open(struct snd_pcm_substream *substream)
{
   struct snd_pcm_runtime *runtime = substream->runtime;
   struct bcm63xx_runtime_data *prtd;
   int ret;
   runtime->hw = bcm63xx_pcm_hardware;

   ret = snd_pcm_hw_constraint_step(runtime, 0, SNDRV_PCM_HW_PARAM_PERIOD_BYTES, 32);
   if (ret)
   {
      goto out;
   }
   
   ret = snd_pcm_hw_constraint_step(runtime, 0,	SNDRV_PCM_HW_PARAM_BUFFER_BYTES, 32);
   if (ret)
   {
      goto out;
   }
   
   ret = snd_pcm_hw_constraint_integer(runtime, SNDRV_PCM_HW_PARAM_PERIODS);
   if (ret < 0)
   {
      goto out;
   }

   ret = -ENOMEM;
   prtd = kzalloc(sizeof(*prtd), GFP_KERNEL);
   if (!prtd)
   {
      goto out;
   }
   runtime->private_data = prtd;
   return 0;
out:
   return ret;
}

static int bcm63xx_pcm_close(struct snd_pcm_substream *substream)
{
   struct snd_pcm_runtime *runtime = substream->runtime;
   struct bcm63xx_runtime_data *prtd = runtime->private_data;
   if( prtd )
   {
      kfree( prtd );
   }
   return 0;
}

static struct snd_pcm_ops bcm63xx_pcm_ops = {
   .open      = bcm63xx_pcm_open,
   .close     = bcm63xx_pcm_close,
   .ioctl     = snd_pcm_lib_ioctl,
   .hw_params = bcm63xx_pcm_hw_params,
   .hw_free   = bcm63xx_pcm_hw_free,
   .prepare   = bcm63xx_pcm_prepare,
   .trigger   = bcm63xx_pcm_trigger,
   .pointer   = bcm63xx_pcm_pointer,
   .mmap      = bcm63xx_pcm_mmap,
};

static irqreturn_t i2s_dma_isr(int irq, void *dev_id)
{
   unsigned int availdepth,ifflevel,offlevel;
   struct snd_pcm_substream *substream  = dev_id;
   struct snd_pcm_runtime *runtime      = substream->runtime;
   struct bcm63xx_runtime_data *prtd = runtime->private_data;
   unsigned int int_status,val_1,val_2;

   int ret = regmap_read(regmap_i2s, I2S_TX_IRQ_CTL, &int_status);
   if( ret )
   {
      printk(KERN_ERR "Read IRQ status wrong: ret=%d\n", ret);
      return IRQ_NONE;
   }

   if( int_status & I2S_DESC_OFF_INTR )
   {	
   	  offlevel = int_status >> I2S_DESC_OFF_LEVEL_SHIFT & I2S_DESC_LEVEL_MASK;
   	  while( offlevel )
   	  {
         regmap_read(regmap_i2s, I2S_TX_DESC_OFF_ADDR, &val_1);
         regmap_read(regmap_i2s, I2S_TX_DESC_OFF_LEN,  &val_2);
         prtd->dma_addr_next = val_1 + val_2; //next dma addr will be played
         offlevel--;  
      }
      
      ifflevel   = int_status >> I2S_DESC_IFF_LEVEL_SHIFT & I2S_DESC_LEVEL_MASK;
      availdepth = I2S_DESC_FIFO_DEPTH - ifflevel;
      while( availdepth )
      {
         pdma_desc->dma_addr += prtd->periodsize_inbyte; //next dma addr in descr fifo
         if( pdma_desc->dma_addr - runtime->dma_addr >= runtime->dma_bytes )
         {
      	    pdma_desc->dma_addr = runtime->dma_addr;
         }

         prtd->dma_addr = pdma_desc->dma_addr; //push to private 
         regmap_write(regmap_i2s, I2S_TX_DESC_IFF_LEN,  prtd->periodsize_inbyte);//push to hw 
         regmap_write(regmap_i2s, I2S_TX_DESC_IFF_ADDR,  pdma_desc->dma_addr);
 
         availdepth--;
      }

      snd_pcm_period_elapsed(substream);
   }
   else if( int_status & I2S_DESC_OFF_OVERRUN_INTR ) 
   {
   	  regmap_update_bits(regmap_i2s, I2S_TX_IRQ_CTL,I2S_DESC_OFF_OVERRUN_INTR , 0 );
   }
   else if( int_status & I2S_DESC_IFF_INTR ) 
   {
   	  regmap_update_bits(regmap_i2s, I2S_TX_IRQ_CTL,I2S_DESC_IFF_INTR , 0 );
   }   
   else
   {
   	 // dev_err(dev,"%s: unknown irq detected int_status = 0x%08x\n", __FUNCTION__, int_status);
   }
   
   /* Clear interrupt by writing 0 */
   regmap_update_bits(regmap_i2s, I2S_TX_IRQ_CTL,I2S_INTR_MASK , 0 );

   return IRQ_HANDLED;    
}

static int bcm63xx_pcm_preallocate_dma_buffer(struct snd_pcm *pcm, int stream)
{
   struct snd_pcm_substream *substream = pcm->streams[stream].substream;
   struct snd_dma_buffer *buf          = &substream->dma_buffer;
   size_t size                         = bcm63xx_pcm_hardware.buffer_bytes_max;

   buf->dev.type     = SNDRV_DMA_TYPE_DEV;
   buf->dev.dev      = pcm->card->dev;
   buf->private_data = NULL;
   buf->area         = dma_alloc_writecombine(pcm->card->dev, size,&buf->addr, GFP_KERNEL);
   
   if (!buf->area)
   {
     return -ENOMEM;
   }
   buf->bytes = size;

   return 0;
}

static int bcm63xx_soc_pcm_new(struct snd_soc_pcm_runtime *rtd)
{
   struct snd_card *card = rtd->card->snd_card;
   struct snd_pcm *pcm = rtd->pcm;
   int ret;
   
   if( !r_irq )
   {
   	  ret = -EINVAL;
   	  goto out;
   }

   ret = dma_coerce_mask_and_coherent(card->dev, DMA_BIT_MASK(32));
   if (ret)
   {
      goto out;
   }

   if (pcm->streams[SNDRV_PCM_STREAM_PLAYBACK].substream)
   {
      ret = bcm63xx_pcm_preallocate_dma_buffer(pcm,SNDRV_PCM_STREAM_PLAYBACK);
      if (ret)
      {
         goto out;
      }
   }

   ret = devm_request_irq( card->dev, r_irq->start , i2s_dma_isr , r_irq->flags , "i2s_dma", (void*)(pcm->streams[SNDRV_PCM_STREAM_PLAYBACK].substream));   
   if ( ret )
   {
   	  dev_err(card->dev, "i2s_init: failed to request interrupt.\n");
   }
out:
   return ret;
}

void bcm63xx_pcm_free_dma_buffers(struct snd_pcm *pcm)
{
   struct snd_pcm_substream *substream;
   struct snd_dma_buffer *buf;
   int stream;

   for (stream = 0; stream < 2; stream++) 
   {
      substream = pcm->streams[stream].substream;
      if (!substream)
         continue;
      buf = &substream->dma_buffer;
      if (!buf->area)
         continue;
      dma_free_writecombine(pcm->card->dev, buf->bytes,	buf->area, buf->addr);
      buf->area = NULL;
   }
}

static struct snd_soc_platform_driver bcm63xx_soc_platform = 
{
   .ops 	   = &bcm63xx_pcm_ops,
   .pcm_new	 = bcm63xx_soc_pcm_new,
   .pcm_free = bcm63xx_pcm_free_dma_buffers,
};

static int bcm63xx_soc_platform_probe(struct platform_device *pdev)
{
#if defined(CONFIG_BCM963148) || defined(CONFIG_BCM963138)
   r_irq = &bcm63138_i2s_resources[0]; /* pick irq resource from array instead of DT */
#else
   r_irq = platform_get_resource(pdev, IORESOURCE_IRQ, 0);
#endif
	 if (!r_irq) {
      dev_err(&pdev->dev, "Unable to get register irq resource.\n");
      return -ENODEV;
   }

   return devm_snd_soc_register_platform(&pdev->dev, &bcm63xx_soc_platform);
}

static int bcm63xx_soc_platform_remove(struct platform_device *pdev)
{
   snd_soc_unregister_platform(&pdev->dev);
   return 0;
}

#ifdef CONFIG_OF
static const struct of_device_id snd_soc_bcm_audio_match[] = 
{
	{ .compatible   = "brcm,bcm63xx-pcm-audio" },
	{ }
};
#endif

static struct platform_driver bcm63xx_pcm_driver = 
{
   .driver =
   {
      .name = DRV_NAME,
      .of_match_table = of_match_ptr(snd_soc_bcm_audio_match),
   },
   .probe  = bcm63xx_soc_platform_probe,
   .remove = bcm63xx_soc_platform_remove,
};

module_platform_driver(bcm63xx_pcm_driver);

MODULE_AUTHOR("Kevin Li <kevin.ke-li@broadcom.com>");
MODULE_DESCRIPTION("Broadcom DSL XPON CHIP ASOC PCM DMA module");
MODULE_LICENSE("GPL");
