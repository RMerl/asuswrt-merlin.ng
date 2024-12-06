/* 
 * tlv320adc5140.c  --   ASoC Driver for TI ADC tlv320adc5140 codecs
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

#include "tlv320adc5140.h"

#define MUTE_DURATION 290

static u8 adc_volume[8];
DECLARE_WAIT_QUEUE_HEAD(wqMute);
static struct snd_soc_dai *g_dai = NULL;
static int adc5140_mute(struct snd_soc_dai*, int);

static const struct regmap_range_cfg adc5140_regmap_pages[] = {
   {
     .selector_reg  = 0,
     .selector_mask = 0xff,
     .window_start  = 0,
     .window_len    = 128,
     .range_min     = 0,
     .range_max     = ADC5140_PAGE_SIZE * ADC5140_PAGE_MAX,
   },
};

static const struct regmap_config adc5140_regmap = {
   .reg_bits        = 8,
   .val_bits        = 8,
   .cache_type      = REGCACHE_NONE,
   .max_register    = ADC5140_PAGE_SIZE * ADC5140_PAGE_MAX,
   .ranges          = adc5140_regmap_pages,
   .num_ranges      = ARRAY_SIZE(adc5140_regmap_pages),
};

int kthreadfunc(void *data)
{
	DECLARE_WAITQUEUE(wq, current);
	add_wait_queue(&wqMute, &wq);
	while (1) {
		set_current_state(TASK_INTERRUPTIBLE);
		schedule();
		msleep(MUTE_DURATION);
		adc5140_mute(g_dai, 0);
	}
	remove_wait_queue(&wqMute, &wq);
	do_exit(0);
}
static int adc5140_set_tdm_slot(struct snd_soc_dai *codec_dai,
	unsigned int tx_mask, unsigned int rx_mask, int channels, int slot_width)
{
	unsigned int i,bitvalue = 0;
	struct snd_soc_component *component = codec_dai->component;

	if (channels > 8 || channels < 1) {
		dev_err(component->dev,
		"Error: ADC codec channels out of range(%d).\n", channels);
		return -EINVAL;
	}

	for (i=1; i<=channels; i++)
		bitvalue = (1 << (8-i)) + bitvalue;

	/* set ASI output channel en*/
	snd_soc_component_write(component, ADC5140_ASIOUT_CHEN, 
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
				"Invalid slot length(%d).\n",slot_width);
			return -EINVAL;
	}
	snd_soc_component_update_bits(component,ADC5140_ASI_CFG0,
		ADC5140_ASI_SLOT_LEN_MASK,bitvalue << ADC5140_ASI_SLOT_LEN_SHIFT);

	return 0;
}

static int adc5140_set_dai_fmt(struct snd_soc_dai *codec_dai, unsigned int fmt)
{
   struct snd_soc_component *component = codec_dai->component;
   u8 iface_reg_1;
   iface_reg_1 = snd_soc_component_read32(component,ADC5140_MST_CFG0);

	/* set master/slave audio interface */
	switch (fmt & SND_SOC_DAIFMT_MASTER_MASK) {
		case SND_SOC_DAIFMT_CBM_CFM:  /* codec clk&frm master */
			iface_reg_1 |=  ADC5140_BCLK_FS_MASTER;
			break;
		case SND_SOC_DAIFMT_CBS_CFS:
			iface_reg_1 &= ~ADC5140_BCLK_FS_MASTER;
			break;
		default:
			dev_err(component->dev, "Invalid DAI master/slave interface.\n");
			return -EINVAL;
   }
	snd_soc_component_update_bits(component, ADC5140_MST_CFG0,
				      ADC5140_MST_SLV_MASK,
				      iface_reg_1);

	iface_reg_1 = snd_soc_component_read32(component,ADC5140_ASI_CFG0);
	iface_reg_1 &= ~ADC5140_ASI_FORMAT_MASK;
	/* set I2S/DSP/RJF/LJF mode */
	switch (fmt & SND_SOC_DAIFMT_FORMAT_MASK) {
		case SND_SOC_DAIFMT_I2S:
			iface_reg_1 |= ADC5140_ASI_FORMAT_I2S ;
			break;
		case SND_SOC_DAIFMT_DSP_A:
		case SND_SOC_DAIFMT_DSP_B:
			iface_reg_1 |= ADC5140_ASI_FORMAT_TDM;
			break;
		case SND_SOC_DAIFMT_LEFT_J:
			iface_reg_1 |= ADC5140_ASI_FORMAT_LJ;
			break;
		default:
			dev_err(component->dev, "Invalid DAI interface format\n");
			return -EINVAL;
	}

	iface_reg_1 &= ~(ADC5140_FSYNC_POL_MASK | ADC5140_BCLK_POL_MASK);
	/* signal polarity */
	switch (fmt & SND_SOC_DAIFMT_INV_MASK) {
	case SND_SOC_DAIFMT_NB_IF:
		iface_reg_1 |= ADC5140_BCLK_POL_STD | ADC5140_FSYNC_POL_INV;
		break;
	case SND_SOC_DAIFMT_IB_IF:
		iface_reg_1 |= ADC5140_BCLK_POL_INV | ADC5140_FSYNC_POL_INV;
		break;
	case SND_SOC_DAIFMT_IB_NF:
		iface_reg_1 |= ADC5140_BCLK_POL_INV | ADC5140_FSYNC_POL_STD;
		break;
	case SND_SOC_DAIFMT_NB_NF:
		break;
	default:
		dev_err(component->dev, "Invalid DAI clock signal polarity\n");
		return -EINVAL;
	}

	snd_soc_component_update_bits(component, ADC5140_ASI_CFG0,
				      ADC5140_ASI_FORMAT_MASK,
				      iface_reg_1);

	return 0;
}

static int adc5140_hw_params(struct snd_pcm_substream *substream,
                             struct snd_pcm_hw_params *params,
                             struct snd_soc_dai *dai)
{
	unsigned int i, ret;
	struct snd_soc_component *component = dai->component;

	snd_soc_component_write(component, ADC5140_PSEL, 0);
	snd_soc_component_write(component, ADC5140_SLEEP_CFG,
		ADC5140_AREG_SELECT_INTL << ADC5140_AREG_SELECT_SHIFT |
		ADC5140_WAKEUP_MODE << ADC5140_SLEEP_ENZ_SHIFT);
	msleep(10);

	/* set TX bus keeper always enabled*/
	ret = snd_soc_component_read32(component,ADC5140_ASI_CFG1);
	ret = ret & !(3 << ADC5140_BUS_KEEPER_SHIFT);
	ret = ret | ADC5140_BUS_KEEPER_ENABLE<<ADC5140_BUS_KEEPER_SHIFT;
	snd_soc_component_write(component, ADC5140_ASI_CFG1, ret);

	snd_soc_component_write(component, ADC5140_CH1_CFG0,
		ADC5140_CHX_INPDM << ADC5140_CHX_INSRC_SHIFT);
	snd_soc_component_write(component, ADC5140_CH2_CFG0,
		ADC5140_CHX_INPDM << ADC5140_CHX_INSRC_SHIFT);
	snd_soc_component_write(component, ADC5140_CH3_CFG0,
		ADC5140_CHX_INPDM << ADC5140_CHX_INSRC_SHIFT);
	snd_soc_component_write(component, ADC5140_CH4_CFG0,
		ADC5140_CHX_INPDM << ADC5140_CHX_INSRC_SHIFT);
	/* set GPO1-4 to be PDM clock output and active low and high*/
	snd_soc_component_write(component, ADC5140_GPO1_CFG, 
		ADC5140_GPOX_PDMCLK | ADC5140_GPOX_ACTHL);
	snd_soc_component_write(component, ADC5140_GPO2_CFG, 
		ADC5140_GPOX_PDMCLK | ADC5140_GPOX_ACTHL);
	snd_soc_component_write(component, ADC5140_GPO3_CFG, 
		ADC5140_GPOX_PDMCLK | ADC5140_GPOX_ACTHL);
	snd_soc_component_write(component, ADC5140_GPO4_CFG, 
		ADC5140_GPOX_PDMCLK | ADC5140_GPOX_ACTHL);
	/* set GPIx pin as PDM data input*/
	snd_soc_component_write(component, ADC5140_GPI_CFG0, 
		ADC5140_GPI1_PDMD_CH12 | ADC5140_GPI2_PDMD_CH34);
	snd_soc_component_write(component, ADC5140_GPI_CFG1, 
		ADC5140_GPI3_PDMD_CH56 | ADC5140_GPI4_PDMD_CH78);
	/* set input channel enable*/
	snd_soc_component_write(component, ADC5140_INPUT_CHEN, 
		ADC5140_CHEN_ALL);

	/* power up enabled ADC-PDM and PLL */
	snd_soc_component_write(component, ADC5140_PWR_CFG, 
		ADC5140_ADC_PDZ | ADC5140_PLL_PDZ);

	g_dai = dai;
	adc5140_mute(g_dai, 1); // mute around 300ms
	wake_up(&wqMute);

	return 0;
}

static int adc5140_mute(struct snd_soc_dai *dai, int mute)
{
	int i;
	struct snd_soc_component *component;

	if (!dai)
		return 0;

	component = dai->component;
	for (i=0;i<8;i++) {
		if (mute) {
			adc_volume[i] = snd_soc_component_read32(component,ADC5140_CH1_CFG2+i*5);
			snd_soc_component_write(component, ADC5140_CH1_CFG2+i*5, 0);
		}
		else
			snd_soc_component_write(component, ADC5140_CH1_CFG2+i*5, adc_volume[i]);
	}
	return 0;
}

static const struct snd_soc_dai_ops adc5140_ops = {
   .hw_params    = adc5140_hw_params,
   .digital_mute = adc5140_mute,
   .set_fmt      = adc5140_set_dai_fmt,
   .set_tdm_slot = adc5140_set_tdm_slot,
};


static struct snd_soc_dai_driver adc5140_dai = {
   .name            = "tlv320adc5140-hifi",
   .capture = {
      .stream_name  = "Capture",
      .channels_min = 1,
      .channels_max = 8,
      .rates        = ADC5140_RATES,
      .formats      = ADC5140_FORMATS,},
   .ops             = &adc5140_ops,
   //.symmetric_rates = 1,
};

static int adc5140_probe(struct snd_soc_component *component)
{
	snd_soc_component_write(component, ADC5140_PSEL, 0);
	snd_soc_component_update_bits(component, ADC5140_RESET, 0x01, 1);
	snd_soc_component_write(component, ADC5140_DSP_CFG1, ADC5140_1_BIQUADS);
	return 0;
}

static struct snd_soc_component_driver soc_component_dev_adc5140 = {
   .probe            = adc5140_probe,
};

static int adc5140_i2c_probe(struct i2c_client *i2c,
                             const struct i2c_device_id *id)
{
   int ret;
   unsigned int val;
   struct adc5140_priv *adc5140;
   struct task_struct *muteTask;

   adc5140 = devm_kzalloc(&i2c->dev, sizeof(*adc5140), GFP_KERNEL);
   if (!adc5140)
      return -ENOMEM;

   adc5140->regmap = devm_regmap_init_i2c(i2c, &adc5140_regmap);
   if (IS_ERR(adc5140->regmap))
      return PTR_ERR(adc5140->regmap);

   i2c_set_clientdata(i2c, adc5140);

   ret = regmap_read(adc5140->regmap, ADC5140_PSEL, &val);
   if (ret != 0) {
      dev_err(&i2c->dev, "Failed to read device ID: %d\n", ret);
      return ret;
   }

   ret = devm_snd_soc_register_component(&i2c->dev, &soc_component_dev_adc5140, &adc5140_dai, 1);
   if (ret) {
      dev_err(&i2c->dev, "Failed to register codec\n");
      return ret;
   }

   muteTask = kthread_run(kthreadfunc, NULL, "Codec_muteThread");
   if (!muteTask)
      dev_err(&i2c->dev, "Create codec mute thread failed.\n");

   return 0;
}

static const struct i2c_device_id adc5140_i2c_id[] = {
   { "tlv320adc5140", 0 },
   { }
};
MODULE_DEVICE_TABLE(i2c, adc5140_i2c_id);

static struct i2c_driver adc5140_i2c_driver = {
   .driver = {
      .name  = "tlv320adc5140",
      .owner = THIS_MODULE,
   },
   .probe    =  adc5140_i2c_probe,
   .id_table =  adc5140_i2c_id,
};

module_i2c_driver(adc5140_i2c_driver);

MODULE_DESCRIPTION("ASoC tlv320adc5140 codec driver");
MODULE_AUTHOR("Kevin Li <kevin.ke-li@broadcom.com>");
MODULE_LICENSE("GPL");
