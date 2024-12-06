 /**************************************************************
 * bcm63xx-pcm-xxx.c  --  ALSA SoC Audio Layer - Broadcom PCM-Controller driver
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
#include <linux/io.h>
#include <linux/module.h>
#include <sound/pcm_params.h>
#include <linux/regmap.h>
#include <linux/of_device.h>
#include <sound/soc.h>
#include "bcm63xx-squamish.h"
#include <board.h>

#define SILICON_VERIFICATION 0
#define SQUAMISH_MAX_DEVICE 2
#define DMA_DESC_NUM 6
#define DMA_DEBUG 0

struct pcm_dma_desc
{
	uint32_t dma_flags;
	uint32_t dma_addr;
};

static struct pcm_dma_desc_info
{
	uint8_t *dma_area;
	dma_addr_t dma_addr;
	uint32_t dma_len;
}pcm_dma_desc_info[SNDRV_PCM_STREAM_LAST+1];

struct bcm63xx_runtime_data {
	int dma_len;
	dma_addr_t dma_addr;
	dma_addr_t dma_addr_next;
};

extern struct regmap *regmap_pcm;
static struct resource *r_irq_rx,*r_irq_tx;
static int snd_pcm_idx=0;
static int capturedeviceidx=-1,playbackdeviceidx=-1;
static int end_avail_desc[SNDRV_PCM_STREAM_LAST+1] = 
			{ DMA_DESC_NUM-1, DMA_DESC_NUM-1 };
static struct snd_pcm *whistler_snd_pcm[SQUAMISH_MAX_DEVICE];
static int tx_irq_cnt=0;


static const struct snd_pcm_hardware bcm63xx_pcm_hardware = {
	.info = SNDRV_PCM_INFO_MMAP |
		SNDRV_PCM_INFO_MMAP_VALID |
		SNDRV_PCM_INFO_INTERLEAVED |
		SNDRV_PCM_INFO_PAUSE |
		SNDRV_PCM_INFO_RESUME,
	.formats = SNDRV_PCM_FMTBIT_S32_LE, /* support S32 only */
	.period_bytes_max = 2048,
	.periods_min = 1,
	.periods_max = PAGE_SIZE/sizeof(struct pcm_dma_desc),
	.buffer_bytes_max = 128 * 1024,
	.fifo_size = 32,
};

#if DMA_DEBUG
static void debug_desc(struct pcm_dma_desc *pdesc,int irq_id)
{
	int i;
	for(i=0;i<DMA_DESC_NUM;i++)
	{
		pr_err("%d %d:0x%0x,0x%x\n",irq_id,i,
			pdesc[i].dma_flags,pdesc[i].dma_addr);
	}
}
#endif

static int bcm63xx_pcm_hw_params(struct snd_pcm_substream *substream,
				struct snd_pcm_hw_params *params)
{
	struct snd_pcm_runtime *runtime = substream->runtime;
	struct device *dev = substream->pcm->card->dev;
	int desc_size,stream;

	desc_size = sizeof( struct pcm_dma_desc );
	stream = substream->stream;
	if (stream > SNDRV_PCM_STREAM_LAST) {
			dev_err(dev,"Error:stream out of range.\n");
						return -EINVAL;
	}

	snd_pcm_set_runtime_buffer(substream, &substream->dma_buffer);
	runtime->dma_bytes = params_buffer_bytes(params);
	pcm_dma_desc_info[stream].dma_len = desc_size*DMA_DESC_NUM;
	pcm_dma_desc_info[stream].dma_area=
			dma_alloc_writecombine(dev, 
				pcm_dma_desc_info[stream].dma_len,
				&pcm_dma_desc_info[stream].dma_addr,
				GFP_KERNEL);
	if (!pcm_dma_desc_info[stream].dma_area) {
		dev_err(dev,"Allocate new descriptor memory failed\n");
		return -ENOMEM;
	}

	return 0;
}

static int bcm63xx_pcm_hw_free(struct snd_pcm_substream *substream)
{
	struct device *dev = substream->pcm->card->dev;
	int stream = substream->stream;
	dma_free_writecombine(dev, pcm_dma_desc_info[stream].dma_len,
			pcm_dma_desc_info[stream].dma_area,
			pcm_dma_desc_info[stream].dma_addr);
	pcm_dma_desc_info[stream].dma_area = NULL;
	snd_pcm_set_runtime_buffer(substream, NULL);
	return 0;
}

static int bcm63xx_pcm_trigger(struct snd_pcm_substream *substream, int cmd)
{
	int ret = 0;
	uint32_t temp;

	if (substream->stream == SNDRV_PCM_STREAM_PLAYBACK) {
		tx_irq_cnt=0;
		switch (cmd) {
			case SNDRV_PCM_TRIGGER_START:
				regmap_update_bits(regmap_pcm,
					DMA_TX_IRQ_MASK,
					DMA_INTMASK_NOTVLD | 
					DMA_INTMASK_BDONE,
					DMA_INTMASK_NOTVLD | 
					DMA_INTMASK_BDONE);
				regmap_update_bits(regmap_pcm, 
					DMA_CTRL_GLB_IRQMASK,
					DMA_GLB_TX_IRQMASK,
					DMA_GLB_TX_IRQEN);
				regmap_update_bits(regmap_pcm,
					DMA_TX_CH_CFG,
					DMA_CH_EN,DMA_CH_EN );
				regmap_update_bits(regmap_pcm,
					DMA_CTRL,
					DMA_EN_MASK,
					DMA_EN);
				regmap_update_bits(regmap_pcm,
					PCM_CTRL, 
					PCM_ENABLE ,
					PCM_ENABLE );
				break;
			case SNDRV_PCM_TRIGGER_STOP:
			case SNDRV_PCM_TRIGGER_SUSPEND:
			case SNDRV_PCM_TRIGGER_PAUSE_PUSH:
				regmap_update_bits(regmap_pcm,
					DMA_TX_IRQ_MASK,
					DMA_INTMASK_NOTVLD |
					DMA_INTMASK_BDONE,
					0);
				regmap_update_bits(regmap_pcm,
					DMA_TX_CH_CFG,
					DMA_CH_EN,0);
				regmap_read(regmap_pcm,DMA_RX_IRQ_MASK,&temp);
				if (!temp) {
					regmap_update_bits(regmap_pcm,
						PCM_CTRL, 
						PCM_ENABLE ,
						0 ); 
					regmap_update_bits(regmap_pcm,
						DMA_CTRL,
						DMA_EN_MASK,
						0);
				}
				break;
			default:
				ret = -EINVAL;
		}
	} else {
		switch (cmd) {
			case SNDRV_PCM_TRIGGER_START:
				regmap_update_bits(regmap_pcm,
					DMA_RX_IRQ_MASK,
					DMA_INTMASK_NOTVLD | 
					DMA_INTMASK_BDONE,
					DMA_INTMASK_NOTVLD | 
					DMA_INTMASK_BDONE);
				regmap_update_bits(regmap_pcm,
					DMA_CTRL_GLB_IRQMASK,
					DMA_GLB_RX_IRQMASK,
					DMA_GLB_RX_IRQEN);
				regmap_update_bits(regmap_pcm,
					DMA_RX_CH_CFG,
					DMA_CH_EN,DMA_CH_EN );
				regmap_update_bits(regmap_pcm,
					DMA_CTRL,
					DMA_EN_MASK,
					DMA_EN);
				regmap_update_bits(regmap_pcm,
					PCM_CTRL, 
					PCM_ENABLE,
					PCM_ENABLE);
				break;
			case SNDRV_PCM_TRIGGER_STOP:
			case SNDRV_PCM_TRIGGER_SUSPEND:
			case SNDRV_PCM_TRIGGER_PAUSE_PUSH:
				regmap_update_bits(regmap_pcm,
					DMA_RX_IRQ_MASK,
					DMA_INTMASK_NOTVLD |
					DMA_INTMASK_BDONE,
					0);
				regmap_update_bits(regmap_pcm,
					DMA_RX_CH_CFG,
					DMA_CH_EN,0);

				regmap_read(regmap_pcm,DMA_TX_IRQ_MASK,&temp);
				if (!temp) {
					regmap_update_bits(regmap_pcm,
						PCM_CTRL, 
						PCM_ENABLE,
						0 ); 
					regmap_update_bits(regmap_pcm,
						DMA_CTRL,
						DMA_EN_MASK,
						0);
				}
				break;
			default:
				ret = -EINVAL;
		}
	}
	return ret;
}

static int bcm63xx_pcm_prepare(struct snd_pcm_substream *substream)
{
	int i,stream,dma_len;
	struct snd_pcm_runtime *runtime;
	struct pcm_dma_desc *pdesc;

	stream = substream->stream;
	runtime = substream->runtime;
	dma_len = snd_pcm_lib_period_bytes(substream);

	pdesc = (struct pcm_dma_desc *)pcm_dma_desc_info[stream].dma_area;
	for (i=0;i<DMA_DESC_NUM;i++) {
		pdesc[i].dma_addr = runtime->dma_addr + i*dma_len;
		pdesc[i].dma_flags = dma_len<<DMA_DESC_LEN_SHIFT;
		pdesc[i].dma_flags |= DMA_STATUS_OWN;
		if (i == DMA_DESC_NUM-1)
			pdesc[i].dma_flags |= DMA_STATUS_WRAP;
	}

	regmap_write(regmap_pcm,
			DMA_CTRL_RST,
			stream == SNDRV_PCM_STREAM_PLAYBACK? 
			DMA_GLB_BIT_CH(DMA_TX_CH_ID) : 
			DMA_GLB_BIT_CH(DMA_RX_CH_ID));
	msleep(1);

	regmap_write(regmap_pcm,
			stream == SNDRV_PCM_STREAM_PLAYBACK? 
			DMA_TX_CH_STATE_RAM1:DMA_RX_CH_STATE_RAM1,
			pcm_dma_desc_info[stream].dma_addr);
	regmap_write(regmap_pcm,
			stream == SNDRV_PCM_STREAM_PLAYBACK?
			DMA_TX_CH_STATE_RAM2:DMA_RX_CH_STATE_RAM2,0);

	return 0;
}

static snd_pcm_uframes_t bcm63xx_pcm_pointer(
		struct snd_pcm_substream *substream )
{
	snd_pcm_uframes_t x;
	struct bcm63xx_runtime_data *prtd = substream->runtime->private_data;

	if ((void *) prtd->dma_addr_next == NULL)
		prtd->dma_addr_next = substream->runtime->dma_addr;

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
	if (prtd)
		kfree( prtd );
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
	.mmap = bcm63xx_pcm_mmap,
};

static int get_avail_desc( struct pcm_dma_desc *pdesc )
{
	int i,avail_desc;
	avail_desc=0;
	for (i=0;i<DMA_DESC_NUM;i++) {
		if (!(pdesc[i].dma_flags & DMA_STATUS_OWN ))
			avail_desc++;
	}
	return avail_desc;
}

static int find_last_desc( struct pcm_dma_desc *pdesc )
{
	int i,next_desc_id,curr_desc_id;
	for (i=0;i<DMA_DESC_NUM;i++) {
		curr_desc_id = i;
		next_desc_id = (i + 1)%DMA_DESC_NUM;
		if(pdesc[i].dma_flags & DMA_STATUS_OWN &&
			!(pdesc[next_desc_id].dma_flags & DMA_STATUS_OWN))
			break;
	}
	return curr_desc_id;
}

static int fill_avail_descs(struct pcm_dma_desc *pdesc,
			struct snd_pcm_substream *substream)
{
	struct snd_pcm_runtime *runtime;
	int i,dma_len,avail_desc,next_desc_id,curr_desc_id,stream;

	stream = substream->stream;
	dma_len = snd_pcm_lib_period_bytes(substream);
	runtime = substream->runtime;

	avail_desc= get_avail_desc(pdesc);
	if (avail_desc==DMA_DESC_NUM)
		curr_desc_id = end_avail_desc[stream];
	else
		curr_desc_id = find_last_desc(pdesc);
	next_desc_id = (curr_desc_id + 1)%DMA_DESC_NUM;

	for (i=0;i<avail_desc;i++) {
		pdesc[next_desc_id].dma_flags |= DMA_STATUS_OWN;
		pdesc[next_desc_id].dma_addr = 
				pdesc[curr_desc_id].dma_addr + dma_len;
		if (pdesc[next_desc_id].dma_addr - runtime->dma_addr >=
				 runtime->dma_bytes)
			pdesc[next_desc_id].dma_addr = runtime->dma_addr;
		curr_desc_id = next_desc_id;
		next_desc_id = (next_desc_id + 1 )%DMA_DESC_NUM;
	}

	end_avail_desc[stream] = curr_desc_id;
	
	return curr_desc_id;
}

static int get_ownership_sum( struct pcm_dma_desc *pdesc )
{
	int i,ownship_cnt=0;
	for(i=0;i<DMA_DESC_NUM;i++)
	{
		if(pdesc[i].dma_flags & DMA_STATUS_OWN)
			ownship_cnt++; 
	}
	return ownship_cnt;
}

static irqreturn_t pcm_dma_isr_rx(int irq, void *snd_pcm_array)
{
	int val,end_desc,start_desc;
	struct pcm_dma_desc *pdesc;
	struct snd_pcm_str *streams;
	struct snd_pcm_substream *substream;
	struct snd_pcm_runtime *runtime;
	struct bcm63xx_runtime_data *prtd;
	struct snd_pcm **psnd_pcm=(struct snd_pcm  **)snd_pcm_array;

	pdesc = (struct pcm_dma_desc *)
		pcm_dma_desc_info[SNDRV_PCM_STREAM_CAPTURE].dma_area;
	if (!pdesc) {
		pr_warn("rx irq: no dma descriptor pointer.\n");
		return IRQ_NONE;
	}

	regmap_read(regmap_pcm, PCM_CTRL, &val);
	if (!(val & PCM_ENABLE)) {
		pr_warn("rx irq: invalid interrupt found.\n");
		return IRQ_NONE;
	}
	if (capturedeviceidx < 0 ||
		capturedeviceidx >= SQUAMISH_MAX_DEVICE ) {
		pr_warn("rx irq: invalide device id(%d).\n",capturedeviceidx);
		return IRQ_NONE;
	}

	val = get_ownership_sum( pdesc );
	if(val == DMA_DESC_NUM )
			return IRQ_NONE;

	streams = psnd_pcm[capturedeviceidx]->streams;
	substream = streams[SNDRV_PCM_STREAM_CAPTURE].substream;
	runtime = substream->runtime;
	prtd = runtime->private_data;

	end_desc = fill_avail_descs(pdesc,substream);
	start_desc = (end_desc + 1)%DMA_DESC_NUM;
	prtd->dma_addr_next = pdesc[start_desc].dma_addr;
	prtd->dma_addr = pdesc[end_desc].dma_addr;

	regmap_read(regmap_pcm, DMA_RX_IRQ_STAT, &val);
	regmap_write(regmap_pcm, DMA_RX_IRQ_STAT, val);
	regmap_update_bits(regmap_pcm,
			DMA_RX_CH_CFG,
			DMA_CH_EN,
			DMA_CH_EN );

	snd_pcm_period_elapsed(substream);

	return IRQ_HANDLED;
}

static irqreturn_t pcm_dma_isr_tx(int irq, void *snd_pcm_array)
{
	uint32_t val,end_desc,start_desc;
	struct pcm_dma_desc *pdesc;
	struct snd_pcm_str *streams;
	struct snd_pcm_runtime *runtime;
	struct snd_pcm_substream *substream;
	struct bcm63xx_runtime_data *prtd;
	struct snd_pcm **psnd_pcm=(struct snd_pcm  **)snd_pcm_array;

	pdesc = (struct pcm_dma_desc *)
		pcm_dma_desc_info[SNDRV_PCM_STREAM_PLAYBACK].dma_area;
	if (!pdesc) {
		pr_warn("tx irq: no dma descriptor pointer.\n");
		return IRQ_NONE;
	}

	regmap_read(regmap_pcm, PCM_CTRL, &val);
	if (!(val & PCM_ENABLE)) {
		pr_warn("tx irq: invalid interrupt found.\n");
		return IRQ_NONE;
	}

	if (playbackdeviceidx < 0 ||
		playbackdeviceidx >= SQUAMISH_MAX_DEVICE ) {
		pr_warn("tx irq: invalide device id(%d).\n",playbackdeviceidx);
		return IRQ_NONE;
	}

	val = get_ownership_sum( pdesc );
	if(val == DMA_DESC_NUM )
			return IRQ_NONE;

	streams = psnd_pcm[playbackdeviceidx]->streams;
	substream = streams[SNDRV_PCM_STREAM_PLAYBACK].substream;
	runtime = substream->runtime;
	prtd = runtime->private_data;

	end_desc = fill_avail_descs(pdesc,substream);
	start_desc = (end_desc + 1)%DMA_DESC_NUM;

	prtd->dma_addr_next = pdesc[start_desc].dma_addr;
	prtd->dma_addr = pdesc[end_desc].dma_addr;

#if SILICON_VERIFICATION
/* data on scope 78563421 */
#define OUTPUT_DATA_LEFT  0x12345678 
/* data on scope 00FF5AFF */ 
#define OUTPUT_DATA_RIGHT 0xFF5AFF00
	struct snd_pcm_runtime *runtime;
	runtime = substream->runtime;
	unsigned int *p=(unsigned int *)runtime->dma_area;
	for (i=0;i<runtime->dma_bytes/sizeof(unsigned int);i+=2) {
		*(p + i) = OUTPUT_DATA_LEFT;
		*(p + i + 1) = OUTPUT_DATA_RIGHT;
	}
#endif

	regmap_read(regmap_pcm, DMA_TX_IRQ_STAT, &val);
	regmap_write(regmap_pcm, DMA_TX_IRQ_STAT, val);
	regmap_update_bits(regmap_pcm,
			DMA_TX_CH_CFG,
			DMA_CH_EN,DMA_CH_EN );

	snd_pcm_period_elapsed(substream);

	return IRQ_HANDLED;
}

static int bcm63xx_pcm_preallocate_dma_buffer(struct snd_pcm *pcm, int stream)
{
	struct snd_pcm_substream *substream = pcm->streams[stream].substream;
	struct snd_dma_buffer *buf = &substream->dma_buffer;
	size_t size = bcm63xx_pcm_hardware.buffer_bytes_max;
	struct device *dev = pcm->card->dev;

	if (stream > SNDRV_PCM_STREAM_LAST) {
		dev_err(dev,"stream outof range.\n");
		return -EINVAL;
	}
	buf->dev.type = SNDRV_DMA_TYPE_DEV;
	buf->dev.dev = dev;
	buf->private_data = NULL;
	buf->area = dma_alloc_writecombine(dev, 
				size,&buf->addr,
				GFP_KERNEL);
	if (!buf->area)
		return -ENOMEM;
	buf->bytes = size;

	return 0;
}

static int bcm63xx_soc_pcm_new(struct snd_soc_pcm_runtime *rtd)
{
	struct snd_card *card = rtd->card->snd_card;
	struct snd_pcm *pcm = rtd->pcm;
	int ret;

	if (snd_pcm_idx >= SQUAMISH_MAX_DEVICE) {
		dev_err(card->dev, 
			"pcm_new error:sound card has too many devices(%d)\n",
			snd_pcm_idx);
		ret = -EINVAL;
		goto out;
	}
	whistler_snd_pcm[snd_pcm_idx] = pcm;

	if ( !(r_irq_rx && r_irq_tx) ) {
		ret = -EINVAL;
		goto out;
	}

	of_dma_configure(pcm->card->dev,pcm->card->dev->of_node,1);

	ret = dma_coerce_mask_and_coherent(pcm->card->dev, DMA_BIT_MASK(32));
	if (ret) {
		dev_err(pcm->card->dev,"Set DMA mask and coherence failed.\n");
		goto out;
	}

	if (pcm->streams[SNDRV_PCM_STREAM_PLAYBACK].substream) {
		ret = bcm63xx_pcm_preallocate_dma_buffer(pcm,
						SNDRV_PCM_STREAM_PLAYBACK);
		if (ret)
			goto out;
		ret = devm_request_irq( pcm->card->dev, 
			r_irq_tx->start ,
			pcm_dma_isr_tx,
			r_irq_tx->flags , 
			"pcm_dma_tx", (void*)(whistler_snd_pcm));
		if (ret) { 
			dev_err(pcm->card->dev,
			"install pcm tx irq(%d) failed.ret=%d\n", 
			r_irq_tx->start,ret);	
			goto out;
		}
		playbackdeviceidx = snd_pcm_idx;
	}

	if (pcm->streams[SNDRV_PCM_STREAM_CAPTURE].substream) {
		ret = bcm63xx_pcm_preallocate_dma_buffer(pcm,
				SNDRV_PCM_STREAM_CAPTURE);
		if (ret)
			goto out;
		ret = devm_request_irq( pcm->card->dev, 
			r_irq_rx->start,
			pcm_dma_isr_rx,
			r_irq_rx->flags , 
			"pcm_dma_rx", (void*)(whistler_snd_pcm));
		if (ret) {
			dev_err(pcm->card->dev,
			"install pcm rx irq(%d) failed.ret=%d\n", 
			r_irq_rx->start,ret);	
			goto out;
		}
		capturedeviceidx = snd_pcm_idx;
	}

	snd_pcm_idx++;

out:
	return ret;
}

void bcm63xx_pcm_free_dma_buffers(struct snd_pcm *pcm)
{
	struct snd_pcm_substream *substream;
	struct snd_dma_buffer *buf;
	int stream;

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
	.ops      = &bcm63xx_pcm_ops,
	.pcm_new  = bcm63xx_soc_pcm_new,
	.pcm_free = bcm63xx_pcm_free_dma_buffers,
};

int bcm63xx_soc_platform_probe(struct platform_device *pdev)
{
	r_irq_rx = platform_get_resource(pdev, IORESOURCE_IRQ, 0);
	if (!r_irq_rx) {
		dev_err(&pdev->dev, "Unable to get register irq resource.\n");
		return -ENODEV;
	}

	r_irq_tx = platform_get_resource(pdev, IORESOURCE_IRQ, 1);
	if (!r_irq_tx) {
		dev_err(&pdev->dev, "Unable to get register irq resource.\n");
		return -ENODEV;
	}

	return devm_snd_soc_register_component(&pdev->dev,
					&bcm63xx_soc_platform,NULL,0);
}

int bcm63xx_soc_platform_remove(struct platform_device *pdev)
{
	return 0;
}
