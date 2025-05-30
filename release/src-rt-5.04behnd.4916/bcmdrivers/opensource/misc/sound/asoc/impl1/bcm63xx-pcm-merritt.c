/*********************************************************************
 * bcm63xx-pcm-merritt.c -- ALSA SoC Audio Layer - Broadcom I2S/TDM Controller driver
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
 **********************************************************************/
#include <linux/dma-mapping.h>
#include <linux/io.h>
#include <linux/module.h>
#include <sound/pcm_params.h>
#include <linux/regmap.h>
#include <linux/of_device.h>
#include <sound/soc.h>
#include "bcm63xx-i2stdm.h"

#define SILICON_VERIFICATION 0

static int rx_update_iff = 1;

/* SW loop back is based on 32K sample rate 
  10ms delay, 2 tx channels and 2 or 4  
  rx channels only*/
#ifdef I2S_SW_LOOPBACK
#define RX_CHAN					2
#define TX_PERIODS_SIZE_BYTES 	2560
#define RX_PERIODS_SIZE_BYTES 	(2560 + 2560*RX_CHAN/2 )
#define TX_CH1_ON_RX_TS      	RX_CHAN
#define TX_CH2_ON_RX_TS      	(RX_CHAN + 1)
static char lpbk_buf[TX_PERIODS_SIZE_BYTES];
static unsigned char *plpbk_period_area;
#endif

struct i2s_dma_desc {
	unsigned char *dma_area;
	dma_addr_t dma_addr;
	unsigned int dma_len;
};

struct bcm63xx_runtime_data {
	int dma_len;
	dma_addr_t dma_addr;
	dma_addr_t dma_addr_next;
};

static struct snd_pcm_hardware bcm63xx_pcm_hardware = {
	.info = SNDRV_PCM_INFO_MMAP |
		SNDRV_PCM_INFO_MMAP_VALID |
		SNDRV_PCM_INFO_INTERLEAVED |
		SNDRV_PCM_INFO_PAUSE |
		SNDRV_PCM_INFO_RESUME,
	.formats = SNDRV_PCM_FMTBIT_S32_LE, /* support S32 only */
	.periods_min = 1,
	.periods_max = PAGE_SIZE/sizeof(struct i2s_dma_desc),
	.buffer_bytes_max = 128 * 1024,
	.fifo_size = 32,
	.channels_min = 2,	/* min channels */
	.channels_max = 8,	/* max channels */
};

static int bcm63xx_pcm_hw_params(struct snd_pcm_substream *substream,
				 struct snd_pcm_hw_params *params)
{
	struct i2s_dma_desc *dma_desc;
	struct snd_soc_pcm_runtime *rtd = substream->private_data;
	struct snd_pcm_runtime *runtime = substream->runtime;

	snd_pcm_set_runtime_buffer(substream, &substream->dma_buffer);
	runtime->dma_bytes = params_buffer_bytes(params);

	dma_desc = kzalloc(sizeof(*dma_desc), GFP_NOWAIT);
	if (!dma_desc)
		return -ENOMEM;

	snd_soc_dai_set_dma_data(rtd->cpu_dai, substream, dma_desc);

	return 0;
}

static int bcm63xx_pcm_hw_free(struct snd_pcm_substream *substream)
{
	struct i2s_dma_desc	*dma_desc;
	struct snd_soc_pcm_runtime *rtd = substream->private_data;

	dma_desc = snd_soc_dai_get_dma_data(rtd->cpu_dai, substream);
	kfree(dma_desc);
	snd_pcm_set_runtime_buffer(substream, NULL);

	return 0;
}

static int bcm63xx_pcm_trigger(struct snd_pcm_substream *substream, int cmd)
{
	int ret = 0;
	struct snd_soc_pcm_runtime *rtd;
	struct bcm_i2s_priv *i2s_priv;
	struct regmap   *regmap_i2s;

	rtd = substream->private_data;
	i2s_priv = dev_get_drvdata(rtd->cpu_dai->dev);
	regmap_i2s = i2s_priv->regmap_i2s;

	if (substream->stream == SNDRV_PCM_STREAM_PLAYBACK) {
		switch (cmd) {
		case SNDRV_PCM_TRIGGER_START:
			regmap_update_bits(regmap_i2s,
					   I2S_TX_IRQ_EN,
					   I2S_TX_DESC_OFF_INTR_EN,
					   I2S_TX_DESC_OFF_INTR_EN);
			regmap_update_bits(regmap_i2s,
					   I2S_TX_CFG,
					   I2S_TX_ENABLE_MASK,
					   I2S_TX_ENABLE);
			break;
		case SNDRV_PCM_TRIGGER_STOP:
		case SNDRV_PCM_TRIGGER_SUSPEND:
		case SNDRV_PCM_TRIGGER_PAUSE_PUSH:
			regmap_write(regmap_i2s,
				     I2S_TX_IRQ_EN,
				     0);
			regmap_update_bits(regmap_i2s,
					   I2S_TX_CFG,
					   I2S_TX_ENABLE_MASK,
					   0);
			break;
		default:
			ret = -EINVAL;
		}
	} else {
		switch (cmd) {
		case SNDRV_PCM_TRIGGER_START:
			rx_update_iff = 1;
			regmap_update_bits(regmap_i2s,
					   I2S_RX_IRQ_EN,
					   I2S_RX_DESC_OFF_INTR_EN_MSK,
					   I2S_RX_DESC_OFF_INTR_EN);
			regmap_update_bits(regmap_i2s,
					   I2S_RX_CFG,
					   I2S_RX_ENABLE_MASK,
					   I2S_RX_ENABLE);
			break;
		case SNDRV_PCM_TRIGGER_STOP:
		case SNDRV_PCM_TRIGGER_SUSPEND:
		case SNDRV_PCM_TRIGGER_PAUSE_PUSH:
			rx_update_iff = 0;
			break;
		default:
			ret = -EINVAL;
		}
	}
	return ret;
}

static int bcm63xx_pcm_prepare(struct snd_pcm_substream *substream)
{
	int period_bytes;
	struct i2s_dma_desc	*dma_desc;
	struct regmap		*regmap_i2s;
	struct bcm_i2s_priv	*i2s_priv;
	struct snd_soc_pcm_runtime *rtd = substream->private_data;
	struct snd_pcm_runtime *runtime = substream->runtime;
	uint32_t regaddr_desclen, regaddr_descaddr;
	struct bcm63xx_runtime_data *prtd = runtime->private_data;

	dma_desc = snd_soc_dai_get_dma_data(rtd->cpu_dai, substream);
	dma_desc->dma_len  = snd_pcm_lib_period_bytes(substream);
	dma_desc->dma_addr = runtime->dma_addr;
	prtd->dma_addr_next = runtime->dma_addr +
				snd_pcm_lib_period_bytes(substream);
	dma_desc->dma_area = runtime->dma_area;


	if (substream->stream == SNDRV_PCM_STREAM_PLAYBACK) {
		regaddr_desclen = I2S_TX_DESC_IFF_LEN;
		regaddr_descaddr = I2S_TX_DESC_IFF_ADDR;
	} else {
		regaddr_desclen = I2S_RX_DESC_IFF_LEN;
		regaddr_descaddr = I2S_RX_DESC_IFF_ADDR;
#if I2S_SW_LOOPBACK
		/*clear loop back buf each start*/
		memset(lpbk_buf, 0, sizeof(lpbk_buf));
		plpbk_period_area = runtime->dma_area;
#endif
	}

	i2s_priv = dev_get_drvdata(rtd->cpu_dai->dev);
	regmap_i2s = i2s_priv->regmap_i2s;

	regmap_write(regmap_i2s, regaddr_desclen, dma_desc->dma_len);
	regmap_write(regmap_i2s, regaddr_descaddr, dma_desc->dma_addr);

	return 0;
}

static snd_pcm_uframes_t
bcm63xx_pcm_pointer(struct snd_pcm_substream *substream)
{
	snd_pcm_uframes_t x;
	struct bcm63xx_runtime_data *prtd = substream->runtime->private_data;

	x = bytes_to_frames(substream->runtime,
		prtd->dma_addr_next - substream->runtime->dma_addr);

	return x == substream->runtime->buffer_size ? 0 : x;
}

static int bcm63xx_pcm_mmap(struct snd_pcm_substream *substream,
			    struct vm_area_struct *vma)
{
	struct snd_pcm_runtime *runtime = substream->runtime;

	return dma_mmap_writecombine(substream->pcm->card->dev, vma,
				     runtime->dma_area,
				     runtime->dma_addr,
				     runtime->dma_bytes);
}

static int bcm63xx_pcm_open(struct snd_pcm_substream *substream)
{
	int ret = 0;
	struct snd_pcm_runtime *runtime = substream->runtime;
	struct bcm63xx_runtime_data *prtd;

#if I2S_SW_LOOPBACK
	/* tx and rx have different period size*/
	if (substream->stream == SNDRV_PCM_STREAM_PLAYBACK) {
		bcm63xx_pcm_hardware.period_bytes_max = TX_PERIODS_SIZE_BYTES;
		bcm63xx_pcm_hardware.period_bytes_min = TX_PERIODS_SIZE_BYTES;
	} else {
		bcm63xx_pcm_hardware.period_bytes_max = RX_PERIODS_SIZE_BYTES;
		bcm63xx_pcm_hardware.period_bytes_min = RX_PERIODS_SIZE_BYTES;
	}
#else
	bcm63xx_pcm_hardware.period_bytes_max = 8192;
	bcm63xx_pcm_hardware.period_bytes_min = 256;
#endif

	runtime->hw = bcm63xx_pcm_hardware;
	ret = snd_pcm_hw_constraint_step(runtime, 0,
					 SNDRV_PCM_HW_PARAM_PERIOD_BYTES, 32);
	if (ret)
		goto out;

	ret = snd_pcm_hw_constraint_step(runtime, 0,
					 SNDRV_PCM_HW_PARAM_BUFFER_BYTES, 32);
	if (ret)
		goto out;

	ret = snd_pcm_hw_constraint_integer(runtime,
					    SNDRV_PCM_HW_PARAM_PERIODS);
	if (ret < 0)
		goto out;

	ret = -ENOMEM;
	prtd = kzalloc(sizeof(*prtd), GFP_KERNEL);
	if (!prtd)
		goto out;

	runtime->private_data = prtd;
	return 0;
out:
	return ret;
}

static int bcm63xx_pcm_close(struct snd_pcm_substream *substream)
{
	struct snd_pcm_runtime *runtime = substream->runtime;
	struct bcm63xx_runtime_data *prtd = runtime->private_data;

	kfree(prtd);
	return 0;
}

static struct snd_pcm_ops bcm63xx_pcm_ops = {
	.open = bcm63xx_pcm_open,
	.close = bcm63xx_pcm_close,
	.ioctl = snd_pcm_lib_ioctl,
	.hw_params = bcm63xx_pcm_hw_params,
	.hw_free = bcm63xx_pcm_hw_free,
	.prepare = bcm63xx_pcm_prepare,
	.trigger = bcm63xx_pcm_trigger,
	.pointer = bcm63xx_pcm_pointer,
	.mmap  = bcm63xx_pcm_mmap,
};



static irqreturn_t i2s_dma_isr(int irq, void *bcm_i2s_priv)
{
	unsigned int availdepth, ifflevel, offlevel, int_status, off_addr, off_len;
	struct bcm63xx_runtime_data *prtd;
	struct snd_pcm_substream *substream;
	struct snd_pcm_runtime *runtime;
	struct regmap *regmap_i2s;
	struct i2s_dma_desc *dma_desc;
	struct snd_soc_pcm_runtime *rtd;
	struct bcm_i2s_priv *i2s_priv;

	i2s_priv = (struct bcm_i2s_priv *)bcm_i2s_priv;
	regmap_i2s = i2s_priv->regmap_i2s;

	/* rx */
	regmap_read(regmap_i2s, I2S_RX_IRQ_CTL, &int_status);

	if (int_status & I2S_RX_DESC_OFF_INTR_EN_MSK) {
		substream = i2s_priv->capture_substream;
		runtime = substream->runtime;
		rtd = substream->private_data;
		prtd = runtime->private_data;
		dma_desc = snd_soc_dai_get_dma_data(rtd->cpu_dai, substream);

		offlevel = (int_status & I2S_RX_DESC_OFF_LEVEL_MASK) >>
			   I2S_RX_DESC_OFF_LEVEL_SHIFT;
		while (offlevel) {
			regmap_read(regmap_i2s, I2S_RX_DESC_OFF_ADDR, &off_addr);
			regmap_read(regmap_i2s, I2S_RX_DESC_OFF_LEN, &off_len);
#if I2S_SW_LOOPBACK
			int i,j;
			unsigned int *p = (unsigned int *)plpbk_period_area;
			unsigned int *plpbk = (unsigned int *)lpbk_buf;
			if (rx_update_iff) {
				for (i = 0,j = 0; 
					i < snd_pcm_lib_period_bytes(substream)/sizeof(*p); 
					i+=i2s_priv->rx_chan,j+=i2s_priv->tx_chan) {
					*(p + i + TX_CH1_ON_RX_TS) = *(plpbk + j);
					*(p + i + TX_CH2_ON_RX_TS) = *(plpbk + j + 1);
				}
				plpbk_period_area += snd_pcm_lib_period_bytes(substream); 
				if (plpbk_period_area - runtime->dma_area >= runtime->dma_bytes)
					plpbk_period_area = runtime->dma_area;
			}
#endif
			offlevel--;
		}
		prtd->dma_addr_next = off_addr + off_len;
		ifflevel = (int_status & I2S_RX_DESC_IFF_LEVEL_MASK) >>
			   I2S_RX_DESC_IFF_LEVEL_SHIFT;

		availdepth = I2S_DESC_FIFO_DEPTH - ifflevel;
		while (availdepth && rx_update_iff) {
			dma_desc->dma_addr +=
					snd_pcm_lib_period_bytes(substream);
			dma_desc->dma_area +=
					snd_pcm_lib_period_bytes(substream);
			if (dma_desc->dma_addr - runtime->dma_addr >=
						runtime->dma_bytes) {
				dma_desc->dma_addr = runtime->dma_addr;
				dma_desc->dma_area = runtime->dma_area;
			}

			prtd->dma_addr = dma_desc->dma_addr;
			regmap_write(regmap_i2s, I2S_RX_DESC_IFF_LEN,
				     snd_pcm_lib_period_bytes(substream));
			regmap_write(regmap_i2s, I2S_RX_DESC_IFF_ADDR,
				     dma_desc->dma_addr);
			availdepth--;
		}

		snd_pcm_period_elapsed(substream);

		/* Clear interrupt by writing 0 */
		regmap_update_bits(regmap_i2s, I2S_RX_IRQ_CTL,
				   I2S_RX_INTR_MASK, 0);
	}

	/* tx */
	regmap_read(regmap_i2s, I2S_TX_IRQ_CTL, &int_status);

	if (int_status & I2S_TX_DESC_OFF_INTR_EN_MSK) {
		substream = i2s_priv->play_substream;
		runtime = substream->runtime;
		rtd = substream->private_data;
		prtd = runtime->private_data;
		dma_desc = snd_soc_dai_get_dma_data(rtd->cpu_dai, substream);

		offlevel = (int_status & I2S_TX_DESC_OFF_LEVEL_MASK) >>
			   I2S_TX_DESC_OFF_LEVEL_SHIFT;
		while (offlevel) {
			regmap_read(regmap_i2s, I2S_TX_DESC_OFF_ADDR, &off_addr);
			regmap_read(regmap_i2s, I2S_TX_DESC_OFF_LEN,  &off_len);
			prtd->dma_addr_next = off_addr + off_len;
			offlevel--;
		}

		ifflevel = (int_status & I2S_TX_DESC_IFF_LEVEL_MASK) >>
			I2S_TX_DESC_IFF_LEVEL_SHIFT;
		availdepth = I2S_DESC_FIFO_DEPTH - ifflevel;

		while (availdepth) {
			dma_desc->dma_addr +=
					snd_pcm_lib_period_bytes(substream);
			dma_desc->dma_area +=
					snd_pcm_lib_period_bytes(substream);

			if (dma_desc->dma_addr - runtime->dma_addr >=
							runtime->dma_bytes) {
				dma_desc->dma_addr = runtime->dma_addr;
				dma_desc->dma_area = runtime->dma_area;
			}

			prtd->dma_addr = dma_desc->dma_addr;

#if I2S_SW_LOOPBACK 
		unsigned int rx_cfg;
		regmap_read(regmap_i2s, I2S_RX_CFG, &rx_cfg);
		if( rx_cfg & I2S_RX_ENABLE_MASK ) {
			/*copy played period data to lookback buf for rx*/
			memcpy(lpbk_buf,dma_desc->dma_area,TX_PERIODS_SIZE_BYTES);
		}
#endif

#if SILICON_VERIFICATION
#define OUTPUT_DATA_LEFT  0xFF00550A
#define OUTPUT_DATA_RIGHT 0x5500FF0A
unsigned int *p = (unsigned int *)dma_desc->dma_area;
int i;
for (i = 0;
	i < snd_pcm_lib_period_bytes(substream)/sizeof(unsigned int); i += 2) {
	*(p + i) = OUTPUT_DATA_LEFT;
	*(p + i + 1) = OUTPUT_DATA_RIGHT;
}
#endif

			regmap_write(regmap_i2s, I2S_TX_DESC_IFF_LEN,
				snd_pcm_lib_period_bytes(substream));
			regmap_write(regmap_i2s, I2S_TX_DESC_IFF_ADDR,
					dma_desc->dma_addr);
			availdepth--;
		}

		snd_pcm_period_elapsed(substream);

		/* Clear interrupt by writing 0 */
		regmap_update_bits(regmap_i2s, I2S_TX_IRQ_CTL,
				   I2S_TX_INTR_MASK, 0);
	}

	return IRQ_HANDLED;
}

static int bcm63xx_pcm_preallocate_dma_buffer(struct snd_pcm *pcm, int stream)
{
	struct snd_pcm_substream *substream = pcm->streams[stream].substream;
	struct snd_dma_buffer *buf = &substream->dma_buffer;
	size_t size = bcm63xx_pcm_hardware.buffer_bytes_max;

	buf->dev.type = SNDRV_DMA_TYPE_DEV;
	buf->dev.dev = pcm->card->dev;
	buf->private_data = NULL;
	buf->area = dma_alloc_writecombine(pcm->card->dev, size, &buf->addr,
					   GFP_KERNEL);

	if (!buf->area)
		return -ENOMEM;

	buf->bytes = size;

	return 0;
}

static int bcm63xx_soc_pcm_new(struct snd_soc_pcm_runtime *rtd)
{
	struct snd_pcm *pcm = rtd->pcm;
	struct bcm_i2s_priv *i2s_priv;
	int ret;

	i2s_priv = dev_get_drvdata(rtd->cpu_dai->dev);

	of_dma_configure(pcm->card->dev, pcm->card->dev->of_node, 1);

	ret = dma_coerce_mask_and_coherent(pcm->card->dev, DMA_BIT_MASK(32));
	if (ret)
		goto out;

	if (pcm->streams[SNDRV_PCM_STREAM_PLAYBACK].substream) {
		ret = bcm63xx_pcm_preallocate_dma_buffer(pcm,
						 SNDRV_PCM_STREAM_PLAYBACK);
		if (ret)
			goto out;

		i2s_priv->play_substream =
			pcm->streams[SNDRV_PCM_STREAM_PLAYBACK].substream;
	}

	if (pcm->streams[SNDRV_PCM_STREAM_CAPTURE].substream) {
		ret = bcm63xx_pcm_preallocate_dma_buffer(pcm,
					SNDRV_PCM_STREAM_CAPTURE);
		if (ret)
			goto out;
		i2s_priv->capture_substream =
			pcm->streams[SNDRV_PCM_STREAM_CAPTURE].substream;
	}

out:
	return ret;
}

void bcm63xx_pcm_free_dma_buffers(struct snd_pcm *pcm)
{
	int stream;
	struct snd_dma_buffer *buf;
	struct snd_pcm_substream *substream;

	for (stream = 0; stream < 2; stream++) {
		substream = pcm->streams[stream].substream;
		if (!substream)
			continue;
		buf = &substream->dma_buffer;
		if (!buf->area)
			continue;
		dma_free_writecombine(pcm->card->dev, buf->bytes,
				      buf->area, buf->addr);
		buf->area = NULL;
	}
}

static const struct snd_soc_component_driver bcm63xx_soc_platform = {
	.ops = &bcm63xx_pcm_ops,
	.pcm_new = bcm63xx_soc_pcm_new,
	.pcm_free = bcm63xx_pcm_free_dma_buffers,
};

int bcm63xx_soc_platform_probe(struct platform_device *pdev,
			       struct bcm_i2s_priv *i2s_priv)
{
	int ret;

	i2s_priv->r_irq = platform_get_resource(pdev, IORESOURCE_IRQ, 0);
	if (!i2s_priv->r_irq) {
		dev_err(&pdev->dev, "Unable to get register irq resource.\n");
		return -ENODEV;
	}

	ret = devm_request_irq(&pdev->dev, i2s_priv->r_irq->start, i2s_dma_isr,
			i2s_priv->r_irq->flags, "i2s_dma", (void *)i2s_priv);
	if (ret) {
		dev_err(&pdev->dev,
			"i2s_init: failed to request interrupt.ret=%d\n", ret);
		return ret;
	}

	return devm_snd_soc_register_component(&pdev->dev,
					&bcm63xx_soc_platform, NULL, 0);
}

int bcm63xx_soc_platform_remove(struct platform_device *pdev)
{
	return 0;
}

MODULE_AUTHOR("Kevin,Li <kevin-ke.li@broadcom.com>");
MODULE_DESCRIPTION("Broadcom DSL XPON ASOC I2S/TDM Interface");
MODULE_LICENSE("GPL v2");
