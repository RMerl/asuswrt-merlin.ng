/* 
 * tlv320adc3101.c  --   ASoC Driver for TI ADC tlv320adc3101 codecs
 *
 * Author: Kevin Li <kevin-ke.li@broadcom.com>
 *
<:copyright-BRCM:2018:DUAL/GPL:standard

   Copyright (c) 2018 Broadcom 
   All Rights Reserved

Unless you and Broadcom execute a separate written software license
agreement governing use of this software, this software is licensed
to you under the terms of the GNU General Public License version 2
(the "GPL"), available at http://www.broadcom.com/licenses/GPLv2.php,
with the following added to such license:

   As a special exception, the copyright holders of this software give
   you permission to link this software with independent modules, and
   to copy and distribute the resulting executable under terms of your
   choice, provided that you also meet, for each linked independent
   module, the terms and conditions of the license of that module.
   An independent module is a module which is not derived from this
   software.  The special exception does not apply to any modifications
   of the software.

Not withstanding the above, under no circumstances may you combine
this software in any way with any other Broadcom software provided
under a license other than the GPL, without Broadcom's express prior
written consent.

:>
*/

#include <linux/clk.h>
#include <linux/i2c.h>
#include <linux/module.h>
#include <sound/pcm_params.h>
#include <sound/soc.h>
#include <sound/soc-dapm.h>
#include <sound/tlv.h>

#include "tlv320adc3101.h"

/* 0dB min, 0.5dB steps        name     min step mute*/
static DECLARE_TLV_DB_SCALE(tlv_step_0_5, 0, 50, 0);
/* -12dB min, 0.5dB steps */
static DECLARE_TLV_DB_SCALE(tlv_adc_vol, -1200, 50, 0);

static const struct snd_kcontrol_new adc3101_snd_controls[] = {
   SOC_SINGLE("ADCFGA Left Mute Switch", ADC3101_ADCFGA, 7, 1, 0),
   SOC_SINGLE("ADCFGA Right Mute Switch",ADC3101_ADCFGA, 3, 1, 0),
   SOC_SINGLE("AGC Left Switch",         ADC3101_LAGC1,  7, 1, 0),
   SOC_SINGLE("AGC Right Switch",        ADC3101_RAGC1,  7, 1, 0),
   SOC_SINGLE("Left Mic Bias Value",     ADC3101_MICBIAS,5, 3, 0),
   SOC_SINGLE("Right Mic Bias Value",    ADC3101_MICBIAS,3, 3, 0),
   SOC_DOUBLE_R("Mic PGA Switch",        ADC3101_LMICPGAVOL, 
                                         ADC3101_RMICPGAVOL, 7, 0x01, 1),
   SOC_DOUBLE_R("AGC Target Level",      ADC3101_LAGC1,
                                         ADC3101_RAGC1,      4, 0x07, 0),
   SOC_DOUBLE_R("AGC Hysteresis",        ADC3101_LAGC2,
                                         ADC3101_RAGC2,      6, 0x03, 0),
   SOC_DOUBLE_R("AGC Noise Threshold",   ADC3101_LAGC2,
                                         ADC3101_RAGC2,      1, 0x1F, 0),
   SOC_DOUBLE_R("AGC Max PGA",           ADC3101_LAGC3,
                                         ADC3101_RAGC3,      0, 0x7F, 0),
   SOC_DOUBLE_R("AGC Attack Time",       ADC3101_LAGC4,
                                         ADC3101_RAGC4,      3, 0x1F, 0),
   SOC_DOUBLE_R("AGC Decay Time",        ADC3101_LAGC5,
                                         ADC3101_RAGC5,      3, 0x1F, 0),
   SOC_DOUBLE_R("AGC Noise Debounce",    ADC3101_LAGC6,
                                         ADC3101_RAGC6,      0, 0x1F, 0),
   SOC_DOUBLE_R("AGC Signal Debounce",   ADC3101_LAGC7,
                                         ADC3101_RAGC7,      0, 0x0F, 0),
   SOC_DOUBLE_R_S_TLV("ADC Level Volume",ADC3101_LADCVOL,
                        ADC3101_RADCVOL,    0, -0x18, 0x28, 6, 0, tlv_adc_vol),
   SOC_DOUBLE_R_TLV("Mic PGA Level Volume",  ADC3101_LMICPGAVOL, 
                        ADC3101_RMICPGAVOL, 0, 0x5f, 0, tlv_step_0_5),
};

static const struct snd_kcontrol_new left_input_mixer_controls[] = {
   SOC_DAPM_SINGLE("IN1_L P Switch",     ADC3101_LMICPGAPIN, 0, 3, 0),
   SOC_DAPM_SINGLE("IN2_L P Switch",     ADC3101_LMICPGAPIN, 2, 3, 0),
   SOC_DAPM_SINGLE("IN3_L M Switch",     ADC3101_LMICPGAPIN, 4, 3, 0),
   SOC_DAPM_SINGLE("IN2LP_IN3LM Switch", ADC3101_LMICPGAPIN, 6, 3, 0),
};
static const struct snd_kcontrol_new right_input_mixer_controls[] = {
   SOC_DAPM_SINGLE("IN1_R M Switch",     ADC3101_RMICPGAPIN, 0, 3, 0),
   SOC_DAPM_SINGLE("IN2_R P Switch",     ADC3101_RMICPGAPIN, 2, 3, 0),
   SOC_DAPM_SINGLE("IN3_R M Switch",     ADC3101_RMICPGAPIN, 4, 3, 0),
   SOC_DAPM_SINGLE("IN2RP_IN3RM Switch", ADC3101_RMICPGAPIN, 6, 3, 0),
};
   
static const struct snd_soc_dapm_widget adc3101_dapm_widgets[] = {
   SND_SOC_DAPM_INPUT("IN1_L"),
   SND_SOC_DAPM_INPUT("IN1_R"),
   SND_SOC_DAPM_INPUT("IN2_L"),
   SND_SOC_DAPM_INPUT("IN2_R"),
   SND_SOC_DAPM_INPUT("IN3_L"),
   SND_SOC_DAPM_INPUT("IN3_R"),
   SND_SOC_DAPM_MICBIAS("Mic Bias1", ADC3101_MICBIAS, 5, 0),
   SND_SOC_DAPM_MICBIAS("Mic Bias2", ADC3101_MICBIAS, 3, 0),
   SND_SOC_DAPM_ADC("Left ADC", "Left Capture",   ADC3101_ADCSETUP, 7, 0),
   SND_SOC_DAPM_ADC("Right ADC", "Right Capture", ADC3101_ADCSETUP, 6, 0),
   SND_SOC_DAPM_MIXER("Left Input Mixer", SND_SOC_NOPM, 0, 0,
        &left_input_mixer_controls[0], ARRAY_SIZE( left_input_mixer_controls)),
   SND_SOC_DAPM_MIXER("Right Input Mixer",SND_SOC_NOPM, 0, 0,
        &right_input_mixer_controls[0],ARRAY_SIZE(right_input_mixer_controls)),
};

static const struct snd_soc_dapm_route adc3101_dapm_routes[] = {
   /* Left input */
   {"Left Input Mixer", "IN1_L P Switch", "IN1_L"},
   {"Left Input Mixer", "IN2_L P Switch", "IN2_L"},
   {"Left Input Mixer", "IN3_L M Switch", "IN3_L"},

   {"Left ADC", NULL, "Left Input Mixer"},

   /* Right Input */
   {"Right Input Mixer", "IN1_R M Switch", "IN1_R"},
   {"Right Input Mixer", "IN2_R P Switch", "IN2_R"},
   {"Right Input Mixer", "IN3_R M Switch", "IN3_R"},

   {"Right ADC", NULL, "Right Input Mixer"},
};

static const struct regmap_range_cfg adc3101_regmap_pages[] = {
   {
     .selector_reg  = 0,
     .selector_mask = 0xff,
     .window_start  = 0,
     .window_len    = 128,
     .range_min     = 0,
     .range_max     = ADC3101_PAGE1 + OFFSET_CALLIBRATION,
   },
};

static const struct regmap_config adc3101_regmap = {
   .reg_bits        = 8,
   .val_bits        = 8,
   .cache_type      = REGCACHE_NONE,
   .max_register    = ADC3101_PAGE1 + OFFSET_CALLIBRATION,
   .ranges          = adc3101_regmap_pages,
   .num_ranges      = ARRAY_SIZE(adc3101_regmap_pages),
};

static inline int adc3101_get_divs(int mclk, int rate)
{
   int i;
   for (i = 0; i < ARRAY_SIZE(adc3101_divs); i++) {
      if ((adc3101_divs[i].rate == rate) && (adc3101_divs[i].mclk == mclk)) {
         return i;
      }
   }
   return -EINVAL;
}

static int adc3101_set_dai_sysclk(struct snd_soc_dai *codec_dai, int clk_id, 
                                                  unsigned int freq, int dir)
{
   struct snd_soc_codec *codec = codec_dai->codec;
   struct adc3101_priv *adc3101 = snd_soc_codec_get_drvdata(codec);

   switch (freq) {
      case MCLK_8192000:
      case MCLK_11289600:
      case MCLK_12288000:
      case MCLK_16384000:
      case MCLK_22579200:
      case MCLK_24576000:
      case MCLK_32768000:
      case MCLK_45158400:
      case MCLK_49152000:
         adc3101->sysclk = freq;
      return 0;
   }
   dev_err(codec->dev, "adc3101: invalid frequency to set DAI system clock\n");
   return -EINVAL;
}

static int adc3101_set_dai_fmt(struct snd_soc_dai *codec_dai, unsigned int fmt)
{
   struct snd_soc_codec *codec = codec_dai->codec;
   u8 iface_reg_1,dataslot_offset,iface_reg_2;

   iface_reg_1 = snd_soc_read(codec, ADC3101_IFACE1);
   dataslot_offset = 0;
   iface_reg_2 = snd_soc_read(codec, ADC3101_IFACE2);
   iface_reg_2 &= ~(1 << I2S_DEFAULT_BIT_POL_SHIFT);

   /* set master/slave audio interface */
   switch (fmt & SND_SOC_DAIFMT_MASTER_MASK) {
      case SND_SOC_DAIFMT_CBM_CFM:  /* codec clk&frm master */
         iface_reg_1 |=  1 << ADC3101_BCLK_DIR_SHIFT | 
                         1 << ADC3101_WCLK_DIR_SHIFT ;
      break;
      case SND_SOC_DAIFMT_CBS_CFS:
         iface_reg_1 &= ~( 1 << ADC3101_BCLK_DIR_SHIFT | 
                         1 << ADC3101_WCLK_DIR_SHIFT );
      break;
      default:
      return -EINVAL;
   }
   /* set I2S/DSP/RJF/LJF mode */
   switch (fmt & SND_SOC_DAIFMT_FORMAT_MASK) {
      case SND_SOC_DAIFMT_I2S:
         iface_reg_1 &=  ~( 3 << ADC3101_AUDIO_INTERFACE_SHIFT ) ;
      break;
      case SND_SOC_DAIFMT_DSP_A:
         iface_reg_1 |= (ADC3101_DSP_MODE << ADC3101_PLLJ_SHIFT);
         iface_reg_2 |= (1 << 3); /* invert bit clock */
         dataslot_offset = 0x01; /* add offset 1 */
      break;
      case SND_SOC_DAIFMT_DSP_B:
         iface_reg_1 |= (ADC3101_DSP_MODE << ADC3101_PLLJ_SHIFT);
         iface_reg_2 |= (1 << 3); /* invert bit clock */
      break;
      case SND_SOC_DAIFMT_RIGHT_J:
         iface_reg_1 |= (ADC3101_RIGHT_JUSTIFIED_MODE << ADC3101_PLLJ_SHIFT);
      break;
      case SND_SOC_DAIFMT_LEFT_J:
         iface_reg_1 |= (ADC3101_LEFT_JUSTIFIED_MODE << ADC3101_PLLJ_SHIFT);
      break;
      default:
         printk(KERN_ERR "adc3101: invalid DAI interface format\n");
      return -EINVAL;
   }

   snd_soc_write(codec, ADC3101_IFACE1, iface_reg_1);
   snd_soc_write(codec, ADC3101_DATASLOT_OFFSET, dataslot_offset);
   snd_soc_write(codec, ADC3101_IFACE2, iface_reg_2);
   return 0;
}

static int adc3101_hw_params(struct snd_pcm_substream *substream,
                             struct snd_pcm_hw_params *params,
                             struct snd_soc_dai *dai)
{
   struct snd_soc_codec *codec = dai->codec;
   struct adc3101_priv *adc3101 = snd_soc_codec_get_drvdata(codec);
   
   u8 data;
   int params_idx;
   unsigned int val;

   params_idx = adc3101_get_divs(adc3101->sysclk, params_rate(params));
   if (params_idx < 0) {
      printk(KERN_ERR "adc3101: sampling rate not supported\n");
      return params_idx;
   }
   snd_soc_write(codec, ADC3101_PSEL, 0);
   snd_soc_write(codec, ADC3101_NADC,ADC3101_NADCEN |
                        adc3101_divs[params_idx].nadc );
   snd_soc_write(codec, ADC3101_MADC,ADC3101_MADCEN |
                        adc3101_divs[params_idx].madc );
   snd_soc_write(codec, ADC3101_AOSR, adc3101_divs[params_idx].aosr);
   snd_soc_write(codec, ADC3101_ADCSPB, ADC3101_SPB_PRB_R1);
   data = snd_soc_read(codec, ADC3101_IFACE1);
   switch (params_width(params)) {
      case 16:                                            break;
      case 20: data |= (ADC3101_WORD_LEN_20BITS <<
                        ADC3101_AUDIO_DATA_LENGTH_SHIFT); break;
      case 24: data |= (ADC3101_WORD_LEN_24BITS <<
                        ADC3101_AUDIO_DATA_LENGTH_SHIFT); break;
      case 32: data |= (ADC3101_WORD_LEN_32BITS <<
                        ADC3101_AUDIO_DATA_LENGTH_SHIFT); break;
   }
   snd_soc_write(codec, ADC3101_IFACE1, data);
   snd_soc_write(codec, ADC3101_ADCSETUP, ADC3101_DISABLE_SFTSTEP );
   val=snd_soc_read(codec, ADC3101_ADCFGA);
   snd_soc_write(codec, ADC3101_ADCFGA, val & 
                      ~(ADC3101_LADC_MUTED | ADC3101_RADC_MUTED ));
   return 0;
}

static int adc3101_mute(struct snd_soc_dai *dai, int mute)
{
   struct snd_soc_codec *codec = dai->codec;
   u8 dac_reg;
   dac_reg = snd_soc_read(codec, ADC3101_DACMUTE) & ~ADC3101_MUTEON;
   if (mute)
      snd_soc_write(codec, ADC3101_DACMUTE, dac_reg | ADC3101_MUTEON);
   else
      snd_soc_write(codec, ADC3101_DACMUTE, dac_reg);
   return 0;
}

static int adc3101_set_bias_level(struct snd_soc_codec *codec,
                                  enum snd_soc_bias_level level)
{
   switch (level) {
      case SND_SOC_BIAS_ON:
         snd_soc_update_bits(codec, ADC3101_NADC,
                                    ADC3101_NADCEN,
                                    ADC3101_NADCEN);
         snd_soc_update_bits(codec, ADC3101_MADC,
                                    ADC3101_MADCEN,
                                    ADC3101_MADCEN);
         snd_soc_update_bits(codec, ADC3101_BCLKN,
                                    ADC3101_BCLKEN,
                                    ADC3101_BCLKEN);
      break;
      case SND_SOC_BIAS_PREPARE:
      break;
      case SND_SOC_BIAS_STANDBY:
         snd_soc_update_bits(codec, ADC3101_BCLKN,
                                    ADC3101_BCLKEN,
                                    0);
         snd_soc_update_bits(codec, ADC3101_MADC,
                                    ADC3101_MADCEN,
                                    0);
         snd_soc_update_bits(codec, ADC3101_NADC,
                                    ADC3101_NADCEN,
                                    0);
      break;
      case SND_SOC_BIAS_OFF:
      break;
   }
   codec->dapm.bias_level = level;
   return 0;
}
static const struct snd_soc_dai_ops adc3101_ops = {
   .hw_params    = adc3101_hw_params,
   .digital_mute = adc3101_mute,
   .set_fmt      = adc3101_set_dai_fmt,
   .set_sysclk   = adc3101_set_dai_sysclk,
};


static struct snd_soc_dai_driver adc3101_dai = {
   .name            = "tlv320adc3101-hifi",
   .capture = {
      .stream_name  = "Capture",
      .channels_min = 1,
      .channels_max = 2,
      .rates        = ADC3101_RATES,
      .formats      = ADC3101_FORMATS,},
   .ops             = &adc3101_ops,
   .symmetric_rates = 1,
};

static int adc3101_probe(struct snd_soc_codec *codec)
{
   snd_soc_write(codec, ADC3101_RESET, 0x01);
   return 0;
}

static struct snd_soc_codec_driver soc_codec_dev_adc3101 = {
   .probe            = adc3101_probe,
   .set_bias_level   = adc3101_set_bias_level,
   .suspend_bias_off = true,

   .controls         = adc3101_snd_controls,
   .num_controls     = ARRAY_SIZE(adc3101_snd_controls),
   .dapm_widgets     = adc3101_dapm_widgets,
   .num_dapm_widgets = ARRAY_SIZE(adc3101_dapm_widgets),
   .dapm_routes      = adc3101_dapm_routes,
   .num_dapm_routes  = ARRAY_SIZE(adc3101_dapm_routes),
};

static int adc3101_i2c_probe(struct i2c_client *i2c,
                             const struct i2c_device_id *id)
{
   int ret;
   struct adc3101_priv *adc3101;

   adc3101 = devm_kzalloc(&i2c->dev, sizeof(struct adc3101_priv), GFP_KERNEL);
   if (adc3101 == NULL)
      return -ENOMEM;

   adc3101->regmap = devm_regmap_init_i2c(i2c, &adc3101_regmap);
   if (IS_ERR(adc3101->regmap))
      return PTR_ERR(adc3101->regmap);

   i2c_set_clientdata(i2c, adc3101);

   ret = snd_soc_register_codec(&i2c->dev, &soc_codec_dev_adc3101, 
                                           &adc3101_dai, 1);
   if (ret) {
      dev_err(&i2c->dev, "Failed to register codec\n");
      return ret;
   }

   return 0;
}

static int adc3101_i2c_remove(struct i2c_client *client)
{
   snd_soc_unregister_codec(&client->dev);
   return 0;
}

static const struct i2c_device_id adc3101_i2c_id[] = {
   { "tlv320adc3101", 0 },
   { }
};
MODULE_DEVICE_TABLE(i2c, adc3101_i2c_id);

static struct i2c_driver adc3101_i2c_driver = {
   .driver = {
      .name  = "tlv320adc3101",
      .owner = THIS_MODULE,
   },
   .probe    =  adc3101_i2c_probe,
   .remove   =  adc3101_i2c_remove,
   .id_table =  adc3101_i2c_id,
};

module_i2c_driver(adc3101_i2c_driver);

MODULE_DESCRIPTION("ASoC tlv320adc3101 codec driver");
MODULE_AUTHOR("Kevin Li <kevin.ke-li@broadcom.com>");
MODULE_LICENSE("GPL");
