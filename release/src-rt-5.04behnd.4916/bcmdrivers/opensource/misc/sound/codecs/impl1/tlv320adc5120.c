/* 
 * tlv320adc5120.c  --   ASoC Driver for TI ADC tlv320adc5120 codecs
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

*/

#include <linux/clk.h>
#include <linux/i2c.h>
#include <linux/module.h>
#include <sound/pcm_params.h>
#include <sound/soc.h>
#include <sound/soc-dapm.h>
#include <sound/tlv.h>
#include <linux/delay.h>
#include <linux/kthread.h>

#include "tlv320adc5120.h"

#define MUTE_DURATION 150
DECLARE_WAIT_QUEUE_HEAD(wqMute);

/*channel volume in dB, -100 to +27 only*/
static int init_volume[ADC5120_MAX_CH] = { 21, 21, 21, 21 }; 
static int adc_volume[ADC5120_MAX_CH];
static struct snd_soc_dai *g_dai = NULL;
static struct task_struct *muteTask;
static int adc5120_mute(struct snd_soc_dai*, int);

static const struct regmap_range_cfg adc5120_regmap_pages[] = {
   {
     .selector_reg  = 0,
     .selector_mask = 0xff,
     .window_start  = 0,
     .window_len    = 128,
     .range_min     = 0,
     .range_max     = ADC5120_PAGE_SIZE * ADC5120_PAGE_MAX,
   },
};

static const struct regmap_config adc5120_regmap = {
   .reg_bits        = 8,
   .val_bits        = 8,
   .cache_type      = REGCACHE_NONE,
   .max_register    = ADC5120_PAGE_SIZE * ADC5120_PAGE_MAX,
   .ranges          = adc5120_regmap_pages,
   .num_ranges      = ARRAY_SIZE(adc5120_regmap_pages),
};

int kthreadfunc(void *data)
{
	DECLARE_WAITQUEUE(wq, current);
	add_wait_queue(&wqMute, &wq);
	while (!kthread_should_stop()) {
		set_current_state(TASK_INTERRUPTIBLE);
		schedule();
		msleep(MUTE_DURATION);
		adc5120_mute(g_dai, 0);
	}
	remove_wait_queue(&wqMute, &wq);
	do_exit(0);
}

static int adc5120_set_tdm_slot(struct snd_soc_dai *codec_dai,
	unsigned int tx_mask, unsigned int rx_mask, int channels, int slot_width)
{
	unsigned int i,codec_ch,bitvalue;
	struct snd_soc_component *component = codec_dai->component;

#if ADC5120_TX_LOOPBACK_EN
	if (channels < ADC5120_MIN_CH + ADC5120_NUM_TX_LOOPBACK_CH) {
		dev_err(component->dev,
			"Error,invalid channel numbers for TX loop back mode(ch=%d).\n",channels);
		return -EINVAL;
	}
	codec_ch = channels - ADC5120_NUM_TX_LOOPBACK_CH;
	dev_info(component->dev,
		"RX is set to loopback mode.Codec channel=%d.Loopback channel=%d\n",
		codec_ch, ADC5120_NUM_TX_LOOPBACK_CH);
#else
	codec_ch = channels;
	dev_info(component->dev,
		     "RX is set to non-loopback mode. Number of channel=%d.\n",codec_ch);
#endif
	
	bitvalue = 0;
	for (i=1; i<=codec_ch; i++)
		bitvalue = (1 << (8-i)) + bitvalue;

	/* set number of output channel en*/
	snd_soc_component_write(component, ADC5120_ASIOUT_CHEN, 
		bitvalue);

	switch(slot_width) {
		case 16:
			bitvalue = 0;
			break;
		case 20:
		case 24:
		case 32:
			bitvalue = slot_width/8-1;
			break;
		default:
			dev_err(component->dev,
				"Error,invalid slot length(%d).\n",slot_width);
			return -EINVAL;
	}
	snd_soc_component_update_bits(component,ADC5120_ASI_CFG0,
		ADC5120_ASI_SLOT_LEN_MASK,bitvalue << ADC5120_ASI_SLOT_LEN_SHIFT);

	return 0;
}

static int adc5120_set_dai_fmt(struct snd_soc_dai *codec_dai, unsigned int fmt)
{
	u8 iface_reg;
	struct snd_soc_component *component = codec_dai->component;

	iface_reg = snd_soc_component_read32(component,ADC5120_MST_CFG0);
	/* set master/slave audio interface */
	switch (fmt & SND_SOC_DAIFMT_MASTER_MASK) {
		case SND_SOC_DAIFMT_CBM_CFM:  /* codec clk&frm master */
			iface_reg |=  ADC5120_BCLK_FS_MASTER;
			break;
		case SND_SOC_DAIFMT_CBS_CFS:
			iface_reg &= ~ADC5120_BCLK_FS_MASTER;
			break;
		default:
			dev_err(component->dev, "Error,invalid DAI master/slave interface.\n");
			return -EINVAL;
	}
	snd_soc_component_update_bits(component, ADC5120_MST_CFG0,
				      ADC5120_MST_SLV_MASK,
				      iface_reg);

	iface_reg = snd_soc_component_read32(component,ADC5120_ASI_CFG0);
	iface_reg &= ~ADC5120_ASI_FORMAT_MASK;
	/* set I2S/DSP/RJF/LJF mode */
	switch (fmt & SND_SOC_DAIFMT_FORMAT_MASK) {
		case SND_SOC_DAIFMT_I2S:
			iface_reg |= ADC5120_ASI_FORMAT_I2S ;
			break;
		case SND_SOC_DAIFMT_DSP_A:
		case SND_SOC_DAIFMT_DSP_B:
			iface_reg |= ADC5120_ASI_FORMAT_TDM;
			break;
		case SND_SOC_DAIFMT_LEFT_J:
			iface_reg |= ADC5120_ASI_FORMAT_LJ;
			break;
		default:
			dev_err(component->dev, "Error,invalid DAI interface format\n");
			return -EINVAL;
	}

	iface_reg &= ~(ADC5120_FSYNC_POL_MASK | ADC5120_BCLK_POL_MASK);
	/* signal polarity */
	switch (fmt & SND_SOC_DAIFMT_INV_MASK) {
	case SND_SOC_DAIFMT_NB_IF:
		iface_reg |= ADC5120_BCLK_POL_STD | ADC5120_FSYNC_POL_INV;
		break;
	case SND_SOC_DAIFMT_IB_IF:
		iface_reg |= ADC5120_BCLK_POL_INV | ADC5120_FSYNC_POL_INV;
		break;
	case SND_SOC_DAIFMT_IB_NF:
		iface_reg |= ADC5120_BCLK_POL_INV | ADC5120_FSYNC_POL_STD;
		break;
	case SND_SOC_DAIFMT_NB_NF:
		iface_reg |= ADC5120_BCLK_POL_STD | ADC5120_FSYNC_POL_STD;
		break;
	default:
		dev_err(component->dev, "Invalid DAI clock signal polarity\n");
		return -EINVAL;
	}
	snd_soc_component_update_bits(component, ADC5120_ASI_CFG0,
				      ADC5120_ASI_FORMAT_MASK,
				      iface_reg);

	return 0;
}

static int adc5120_hw_params(struct snd_pcm_substream *substream,
                             struct snd_pcm_hw_params *params,
                             struct snd_soc_dai *dai)
{
	unsigned int i, ret;
	struct snd_soc_component *component = dai->component;

	snd_soc_component_write(component, ADC5120_PSEL, 0);

	snd_soc_component_write(component, ADC5120_SLEEP_CFG,
		ADC5120_AREG_SELECT_INTL << ADC5120_AREG_SELECT_SHIFT |
		ADC5120_WAKEUP_MODE << ADC5120_SLEEP_ENZ_SHIFT);
	msleep(10);

	snd_soc_component_update_bits(component, ADC5120_BIAS_CFG,
				           ADC5120_BIAS_VAL_MASK, ADC5120_BIAS_AS_GPI2);
	snd_soc_component_write(component, ADC5120_GPO1_CFG, 
		ADC5120_GPOX_PDMCLK | ADC5120_GPOX_ACTHL);
	snd_soc_component_write(component, ADC5120_GPI_CFG0, 
		ADC5120_GPI1_PDMD_CH12 | ADC5120_GPI2_PDMD_CH34);
	snd_soc_component_write(component, ADC5120_INPUT_CHEN, 
		ADC5120_CHEN_ALL);

	g_dai = dai;
	adc5120_mute(g_dai, 1); // mute around 150ms
	wake_up(&wqMute);

	return 0;
}

static int adc5120_mute(struct snd_soc_dai *dai, int mute)
{
	int i;
	struct snd_soc_component *component;

	if (!dai)
		return 0;

	component = dai->component;
	for (i=0; i<ADC5120_MAX_CH; i++) {
		if (mute) { /* The address diff of vol reg for each ch is 5 */
			adc_volume[i] = snd_soc_component_read32(component,ADC5120_CH1_CFG2+i*5);
			snd_soc_component_write(component, ADC5120_CH1_CFG2+i*5, 0);
		}
		else
			snd_soc_component_write(component, ADC5120_CH1_CFG2+i*5, adc_volume[i]);
	}
	return 0;
}

static int adc5120_trigger(struct snd_pcm_substream *substream, int cmd,
			       struct snd_soc_dai *dai)
{
	struct snd_soc_component *component = dai->component;
	switch (cmd) {
	case SNDRV_PCM_TRIGGER_START:
	case SNDRV_PCM_TRIGGER_PAUSE_RELEASE:
	case SNDRV_PCM_TRIGGER_RESUME:
		/* power up ADC-PDM and PLL */
		snd_soc_component_write(component, ADC5120_PWR_CFG, 
			ADC5120_ADC_PDZ | ADC5120_PLL_PDZ);
		break;
	case SNDRV_PCM_TRIGGER_STOP:
	case SNDRV_PCM_TRIGGER_PAUSE_PUSH:
	case SNDRV_PCM_TRIGGER_SUSPEND:
		/* power down ADC-PDM and PLL */
		snd_soc_component_write(component, ADC5120_PWR_CFG,0);
		break;
	default:
		return -EINVAL;
	}

	return 0;
}
static const struct snd_soc_dai_ops adc5120_ops = {
   .hw_params    = adc5120_hw_params,
   .digital_mute = adc5120_mute,
   .set_fmt      = adc5120_set_dai_fmt,
   .set_tdm_slot = adc5120_set_tdm_slot,
   .trigger      = adc5120_trigger,
};


static struct snd_soc_dai_driver adc5120_dai = {
   .name            = "tlv320adc5120-hifi",
   .capture = {
      .stream_name  = "Capture",
      .channels_min = ADC5120_MIN_CH,
#if ADC5120_TX_LOOPBACK_EN
      .channels_max = ADC5120_MAX_CH + ADC5120_NUM_TX_LOOPBACK_CH,
#else
      .channels_max = ADC5120_MAX_CH,
#endif
       .rates        = ADC5120_RATES,
       .formats      = ADC5120_FORMATS,},
   .ops             = &adc5120_ops,
   //.symmetric_rates = 1,
};

static int adc5120_probe(struct snd_soc_component *component)
{
	int i;
	snd_soc_component_write(component, ADC5120_PSEL, 0);
	snd_soc_component_update_bits(component, ADC5120_RESET, 0x01, 1);
	snd_soc_component_write(component, ADC5120_DSP_CFG1, ADC5120_1_BIQUADS);
	snd_soc_component_update_bits(component, ADC5120_GPIO_CFG0,
			ADC5120_GPIO1_CFG_MASK, ADC5120_GPIO1_DISABLE);
	snd_soc_component_write(component, ADC5120_CH1_CFG0,
		ADC5120_CHX_INPDM << ADC5120_CHX_INSRC_SHIFT);
	snd_soc_component_write(component, ADC5120_CH2_CFG0,
		ADC5120_CHX_INPDM << ADC5120_CHX_INSRC_SHIFT);
	snd_soc_component_write(component, ADC5120_CH3_CFG0,
		ADC5120_CHX_INPDM << ADC5120_CHX_INSRC_SHIFT);
	snd_soc_component_write(component, ADC5120_CH4_CFG0,
		ADC5120_CHX_INPDM << ADC5120_CHX_INSRC_SHIFT);

	/* set init volume for each channel reg val=dB*2+201*/
	for (i=0;i<ADC5120_MAX_CH;i++) {
		/* The address diff of vol reg for each ch is 5 */
		snd_soc_component_write(component, ADC5120_CH1_CFG2+i*5,
			init_volume[i]*2+201);
	}

	return 0;
}

static struct snd_soc_component_driver soc_component_dev_adc5120 = {
   .probe            = adc5120_probe,
};

static int adc5120_i2c_probe(struct i2c_client *i2c,
                             const struct i2c_device_id *id)
{
   int ret;
   unsigned int val;
   struct adc5120_priv *adc5120;

   adc5120 = devm_kzalloc(&i2c->dev, sizeof(*adc5120), GFP_KERNEL);
   if (!adc5120)
      return -ENOMEM;

   adc5120->regmap = devm_regmap_init_i2c(i2c, &adc5120_regmap);
   if (IS_ERR(adc5120->regmap))
      return PTR_ERR(adc5120->regmap);

   i2c_set_clientdata(i2c, adc5120);

   ret = regmap_read(adc5120->regmap, ADC5120_PSEL, &val);
   if (ret != 0) {
      dev_err(&i2c->dev, "Failed to read device ID: %d\n", ret);
      return ret;
   }

   ret = devm_snd_soc_register_component(&i2c->dev, &soc_component_dev_adc5120, &adc5120_dai, 1);
   if (ret) {
      dev_err(&i2c->dev, "Failed to register codec\n");
      return ret;
   }

   muteTask = kthread_run(kthreadfunc, NULL, "Codec_muteThread");
   if (IS_ERR(muteTask))
      dev_err(&i2c->dev, "Create codec mute thread failed.\n");

   return 0;
}

static int adc5120_i2c_remove(struct i2c_client *i2c)
{
	if(muteTask){
		wake_up(&wqMute);
		kthread_stop(muteTask);
		muteTask = NULL;
	}
	return 0;
}

static const struct i2c_device_id adc5120_i2c_id[] = {
   { "tlv320adc5140", 0 },
   { "tlv320adc5120", 1 },
   { }
};
MODULE_DEVICE_TABLE(i2c, adc5120_i2c_id);

static struct i2c_driver adc5120_i2c_driver = {
   .driver = {
      .name  = "tlv320adc5120",
      .owner = THIS_MODULE,
   },
   .probe    =  adc5120_i2c_probe,
   .id_table =  adc5120_i2c_id,
   .remove   = adc5120_i2c_remove,
};

module_i2c_driver(adc5120_i2c_driver);

MODULE_DESCRIPTION("ASoC tlv320adc5120 codec driver");
MODULE_AUTHOR("Kevin Li <kevin.ke-li@broadcom.com>");
MODULE_LICENSE("GPL");
