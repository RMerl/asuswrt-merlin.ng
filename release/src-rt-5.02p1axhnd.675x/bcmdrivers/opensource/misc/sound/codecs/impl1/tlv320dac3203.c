/*
 * tlv320dac3203.c  --   ASoC Driver for TI DAC tlv320dac3203 codecs
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

#include "tlv320dac3203.h"

#define CODEC_DEBUG 0
#if CODEC_DEBUG
int dump_codec_reg(void *data)
{
   struct snd_soc_codec *codec = (void *)data;
   int n,m;

   snd_soc_write(codec,1,snd_soc_read(codec,1));
   printk("\np%d:  ",snd_soc_read(codec,0));
   for(m=0;m<16;m++)
      printk("%2x ",m);
   printk("\n");
   for(n=0;n<8;n++)
   {
      printk("%02x: ",n*16);
      for(m=0;m<16;m++)
         printk("%02x ",snd_soc_read(codec,n*16+m));
      printk("\n");
   }
   snd_soc_write(codec,129,snd_soc_read(codec,129));
   printk("\npage%d:\n",snd_soc_read(codec,0));
   for(n=8;n<16;n++)
   {
      printk("%02x: ",(n-8)*16);
      for(m=0;m<16;m++)
        printk("%02x ",snd_soc_read(codec,n*16+m));
      printk("\n");
   }
   printk("\n");
   return 0;
}
#endif
/* -63.5dB min, 0.5dB steps */
static DECLARE_TLV_DB_SCALE(tlv_pcm,         -6350, 50, 0);
/* -6dB min, 1dB steps */
static DECLARE_TLV_DB_SCALE(tlv_driver_gain, -600, 100, 0);

static const struct snd_kcontrol_new dac3203_snd_controls[] = {
   SOC_SINGLE("Auto-mute Switch",              DAC3203_DACMUTE,
                                               4, 7, 0),
   SOC_SINGLE("REF Power-up Cfg",              DAC3203_REF_PWRUP_CFG,
                                               0, 7, 0),
   SOC_DOUBLE_R("HP DAC Playback Switch",      DAC3203_HPLGAIN,DAC3203_HPRGAIN,
                                               6, 1, 1),
   SOC_DOUBLE_R_S_TLV("PCM Playback Volume",   DAC3203_LDACVOL,DAC3203_RDACVOL,
                                               0, -0x7f,0x30, 7, 0, tlv_pcm),
   SOC_DOUBLE_R_S_TLV("HP Driver Gain Volume", DAC3203_HPLGAIN,DAC3203_HPRGAIN,
                                               0, -0x6, 0x1d, 5, 0,
                                               tlv_driver_gain),
};

static const struct snd_kcontrol_new hpl_output_mixer_controls[] = {
   SOC_DAPM_SINGLE("L_DAC Switch", DAC3203_HPLROUTE, 3, 1, 0),
   SOC_DAPM_SINGLE("IN1_L Switch", DAC3203_HPLROUTE, 2, 1, 0),
};

static const struct snd_kcontrol_new hpr_output_mixer_controls[] = {
   SOC_DAPM_SINGLE("R_DAC Switch", DAC3203_HPRROUTE, 3, 1, 0),
   SOC_DAPM_SINGLE("IN1_R Switch", DAC3203_HPRROUTE, 2, 1, 0),
};
static const struct snd_soc_dapm_widget dac3203_dapm_widgets[] = {
   SND_SOC_DAPM_OUTPUT("HPL"),
   SND_SOC_DAPM_OUTPUT("HPR"),
   SND_SOC_DAPM_INPUT("IN1_L"),
   SND_SOC_DAPM_INPUT("IN1_R"),
   SND_SOC_DAPM_PGA("HPL Power", DAC3203_OUTPWRCTL, 5, 0, NULL, 0),
   SND_SOC_DAPM_PGA("HPR Power", DAC3203_OUTPWRCTL, 4, 0, NULL, 0),
   SND_SOC_DAPM_DAC("Left DAC",  "Left Playback", DAC3203_DACSETUP, 7, 0),
   SND_SOC_DAPM_DAC("Right DAC", "Right Playback",DAC3203_DACSETUP, 6, 0),
   SND_SOC_DAPM_MIXER("HPL Output Mixer", SND_SOC_NOPM, 0, 0,
          &hpl_output_mixer_controls[0],ARRAY_SIZE(hpl_output_mixer_controls)),
   SND_SOC_DAPM_MIXER("HPR Output Mixer", SND_SOC_NOPM, 0, 0,
          &hpr_output_mixer_controls[0],ARRAY_SIZE(hpr_output_mixer_controls)),
};

static const struct snd_soc_dapm_route dac3203_dapm_routes[] = {
   /* Left Output */
   {"HPL Output Mixer", "L_DAC Switch", "Left DAC"},
   {"HPL Output Mixer", "IN1_L Switch", "IN1_L"},

   {"HPL Power", NULL,  "HPL Output Mixer"},
   {"HPL",       NULL,  "HPL Power"},

   /* Right Output */
   {"HPR Output Mixer", "R_DAC Switch", "Right DAC"},
   {"HPR Output Mixer", "IN1_R Switch", "IN1_R"},

   {"HPR Power", NULL,  "HPR Output Mixer"},
   {"HPR",       NULL,  "HPR Power"},
};

static const struct regmap_range_cfg dac3203_regmap_pages[] = {
   {
   .selector_reg   = 0,
   .selector_mask  = 0xff,
   .window_start   = 0,
   .window_len     = 128,
   .range_min      = 0,
   .range_max      =  DAC3203_PAGE1 + OFFSET_CALLIBRATION,
   },
};

static const struct regmap_config dac3203_regmap = {
   .reg_bits       = 8,
   .val_bits       = 8,
   .cache_type     = REGCACHE_NONE,
   .max_register   = DAC3203_PAGE1 + OFFSET_CALLIBRATION, //kevin changed
   .ranges         = dac3203_regmap_pages,
   .num_ranges     = ARRAY_SIZE(dac3203_regmap_pages),
};

static inline int dac3203_get_divs(int mclk, int rate)
{
   int i;
   for (i = 0; i < ARRAY_SIZE(dac3203_divs); i++) {
      if ((dac3203_divs[i].rate == rate) && (dac3203_divs[i].mclk == mclk)) {
         return i;
      }
   }
   return -EINVAL;
}

static int dac3203_set_dai_sysclk(struct snd_soc_dai *codec_dai, int clk_id,
                                  unsigned int freq, int dir)
{
   struct snd_soc_codec *codec = codec_dai->codec;
   struct dac3203_priv *dac3203 = snd_soc_codec_get_drvdata(codec);

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
         dac3203->sysclk = freq;
      return 0;
   }
   dev_err(codec->dev,"dac3203: invalid frequency to set DAI system clock\n");
   return -EINVAL;
}


static int dac3203_set_dai_fmt(struct snd_soc_dai *codec_dai, unsigned int fmt)
{
   struct snd_soc_codec *codec = codec_dai->codec;
   u8 iface_reg_1,iface_reg_2,iface_reg_3;

   iface_reg_1 = snd_soc_read(codec, DAC3203_IFACE1);
   iface_reg_2 = 0;
   iface_reg_3 = snd_soc_read(codec, DAC3203_IFACE3);

   /* set master/slave audio interface */
   switch (fmt & SND_SOC_DAIFMT_MASTER_MASK) {
      case SND_SOC_DAIFMT_CBM_CFM:
         iface_reg_1 |=  1 << DAC3203_BCLK_DIR_SHIFT |
                         1 << DAC3203_WCLK_DIR_SHIFT ;
      break;
      case SND_SOC_DAIFMT_CBS_CFS:
         iface_reg_1 &= ~( 1 << DAC3203_BCLK_DIR_SHIFT |
                           1 << DAC3203_WCLK_DIR_SHIFT );
      break;
      default:
         dev_err(codec->dev,"dac3203: invalid DAI master/slave interface\n");
      return -EINVAL;
   }
   /* set I2S/DSP/RJF/LJF mode */
   switch (fmt & SND_SOC_DAIFMT_FORMAT_MASK) {
      case SND_SOC_DAIFMT_I2S:
         iface_reg_1 &=  ~( 3 << DAC3203_AUDIO_INTERFACE_SHIFT ) ;
         iface_reg_3 = iface_reg_3 & ~(1 << I2S_DEFAULT_BIT_POL_SHIFT);
      break;
      case SND_SOC_DAIFMT_DSP_A:
         iface_reg_1 |= (DAC3203_DSP_MODE << DAC3203_PLLJ_SHIFT);
         iface_reg_3 |= (1 << 3); /* invert bit clock */
         iface_reg_2 = 0x01; /* add offset 1 */
      break;
      case SND_SOC_DAIFMT_DSP_B:
         iface_reg_1 |= (DAC3203_DSP_MODE << DAC3203_PLLJ_SHIFT);
         iface_reg_3 |= (1 << 3); /* invert bit clock */
      break;
      case SND_SOC_DAIFMT_RIGHT_J:
         iface_reg_1 |= (DAC3203_RIGHT_JUSTIFIED_MODE << DAC3203_PLLJ_SHIFT);
      break;
      case SND_SOC_DAIFMT_LEFT_J:
         iface_reg_1 |= (DAC3203_LEFT_JUSTIFIED_MODE << DAC3203_PLLJ_SHIFT);
      break;
      default:
         dev_err(codec->dev,"dac3203: invalid DAI interface format\n");
      return -EINVAL;
   }

   snd_soc_write(codec, DAC3203_IFACE1, iface_reg_1);
   snd_soc_write(codec, DAC3203_IFACE2, iface_reg_2);
   snd_soc_write(codec, DAC3203_IFACE3, iface_reg_3);
   return 0;
}

static int dac3203_hw_params(struct snd_pcm_substream *substream,
                             struct snd_pcm_hw_params *params,
                             struct snd_soc_dai *dai)
{
   u8 data;
   struct snd_soc_codec *codec = dai->codec;
   struct dac3203_priv *dac3203 = snd_soc_codec_get_drvdata(codec);
   int params_idx;
   
   params_idx = dac3203_get_divs(dac3203->sysclk, params_rate(params));
   if (params_idx < 0) {
      dev_err(codec->dev, "dac3203: sampling rate not supported\n");
      return params_idx;
   }
   snd_soc_write(codec, DAC3203_PSEL, 0);/* turn to page 0*/
   /* P0 reg=11 powerup NDAC+set value to 1, so =0x81 */
   snd_soc_write(codec, DAC3203_NDAC,DAC3203_NDACEN |
                        dac3203_divs[params_idx].ndac );
   /* powerup MDAC+set value */
   snd_soc_write(codec, DAC3203_MDAC,DAC3203_MDACEN |
                        dac3203_divs[params_idx].mdac );
   /* set DOSR value */
   snd_soc_write(codec, DAC3203_DOSRMSB, dac3203_divs[params_idx].dosr>>8);
   snd_soc_write(codec, DAC3203_DOSRLSB, (u8)(dac3203_divs[params_idx].dosr));
   data = snd_soc_read(codec, DAC3203_IFACE1);
   data = data & ~(3 << DAC3203_AUDIO_DATA_LENGTH_SHIFT);
   switch (params_width(params)) {
   case 16:
   break;
   case 20:
      data |= (DAC3203_WORD_LEN_20BITS << DAC3203_DOSRMSB_SHIFT);
   break;
   case 24:
      data |= (DAC3203_WORD_LEN_24BITS << DAC3203_DOSRMSB_SHIFT);
   break;
   case 32:
      data |= (DAC3203_WORD_LEN_32BITS << DAC3203_DOSRMSB_SHIFT);
   break;
   }
   snd_soc_write(codec, DAC3203_IFACE1, data);

   if (params_channels(params) == 1) {
      data = DAC3203_RDAC2LCHN | DAC3203_LDAC2LCHN;
   } else {
      if (dac3203->swapdacs)
         data = DAC3203_RDAC2LCHN | DAC3203_LDAC2RCHN;
      else
         data = DAC3203_LDAC2LCHN | DAC3203_RDAC2RCHN;
   }
   snd_soc_update_bits(codec, DAC3203_DACSETUP, DAC3203_DAC_CHAN_MASK,data);

#if CODEC_DEBUG
   dump_codec_reg((void *)codec );
#endif

   return 0;
}

static int dac3203_mute(struct snd_soc_dai *dai, int mute)
{
   struct snd_soc_codec *codec = dai->codec;
   u8 dac_reg;
   dac_reg = snd_soc_read(codec, DAC3203_DACMUTE) & ~DAC3203_MUTEON;
   if (mute)
      snd_soc_write(codec, DAC3203_DACMUTE, dac_reg | DAC3203_MUTEON);
   else
      snd_soc_write(codec, DAC3203_DACMUTE, dac_reg);

#if CODEC_DEBUG
   dump_codec_reg((void *)codec );
#endif

   return 0;
}

static const struct snd_soc_dai_ops dac3203_ops = {
   .hw_params    = dac3203_hw_params,
   .digital_mute = dac3203_mute,
   .set_fmt      = dac3203_set_dai_fmt,
   .set_sysclk   = dac3203_set_dai_sysclk,
};

static struct snd_soc_dai_driver dac3203_dai = {
   .name     = "tlv320dac3203-hifi",
   .playback = {
      .stream_name  = "Playback",
      .channels_min = 1,
      .channels_max = 2,
      .rates        = DAC3203_RATES,
      .formats      = DAC3203_FORMATS,
   },
  .ops      = &dac3203_ops,
  .symmetric_rates = 1,
};

static int dac3203_probe(struct snd_soc_codec *codec)
{
   snd_soc_write(codec, DAC3203_RESET, 0x01);
   snd_soc_write(codec, DAC3203_DACSPB,   
                        DAC3203_DACSPB_BLK(DAC3203_DACSPB_BLK_PRB_P8)); 
   snd_soc_write(codec, DAC3203_PWRCFG,   DAC3203_AVDDWEAKDISABLE );
   snd_soc_write(codec, DAC3203_LDOCTL,
                        ( DAC3203_AVDD_LDOOUT_172 | DAC3203_AVDD_LDO_PWRUP) &
                        ~DAC3203_ABLKPWR_DISABLE );
   snd_soc_write(codec, DAC3203_HEADSTART,
                        DAC3203_HEADPHONE_SLOW_PWRUP_X6 |
                        DAC3203_HEADPHONE_PWRUP_BYRES6K );
   snd_soc_write(codec, DAC3203_DACSETUP,
                        DAC3203_LDAC2LCHN |
                        DAC3203_RDAC2RCHN | 
                        DAC3203_DACCHVOL_SFTSTP_DISABLE );
   snd_soc_write(codec, DAC3203_DACMUTE,
                        DAC3203_DAC_AUTO_MUTE_DISABLED |
                        DAC3203_LDAC_CH_NOT_MUTED |
                        DAC3203_RDAC_CH_NOT_MUTED |
                        DAC3203_DAC_MASTER_VOLCTL_INDEP );
   return 0;
}

static struct snd_soc_codec_driver soc_codec_dev_dac3203 = {
   .probe            = dac3203_probe,
   .suspend_bias_off = true,
   .controls         = dac3203_snd_controls,
   .num_controls     = ARRAY_SIZE(dac3203_snd_controls),
   .dapm_widgets     = dac3203_dapm_widgets,
   .num_dapm_widgets = ARRAY_SIZE(dac3203_dapm_widgets),
   .dapm_routes      = dac3203_dapm_routes,
   .num_dapm_routes  = ARRAY_SIZE(dac3203_dapm_routes),
};

static int dac3203_i2c_probe(struct i2c_client *i2c,
                             const struct i2c_device_id *id)
{
   struct dac3203_priv *dac3203;
   int ret;
   dac3203 = devm_kzalloc(&i2c->dev, sizeof(struct dac3203_priv), GFP_KERNEL);
   if (dac3203 == NULL)
      return -ENOMEM;

   dac3203->regmap = devm_regmap_init_i2c(i2c, &dac3203_regmap);
   if (IS_ERR(dac3203->regmap))
      return PTR_ERR(dac3203->regmap);

   i2c_set_clientdata(i2c, dac3203);

   dac3203->swapdacs = false;

   ret = snd_soc_register_codec(&i2c->dev,
                                &soc_codec_dev_dac3203,
                                &dac3203_dai,
                                1);
   if (ret) {
      dev_err(&i2c->dev, "Failed to register codec\n");
      return ret;
   }

   return 0;
}

static int dac3203_i2c_remove(struct i2c_client *client)
{
   snd_soc_unregister_codec(&client->dev);
   return 0;
}

static const struct i2c_device_id dac3203_i2c_id[] = {
   { "tlv320dac3203", 0 },
   { }
};
MODULE_DEVICE_TABLE(i2c, dac3203_i2c_id);

static struct i2c_driver dac3203_i2c_driver = {
   .driver   = {
      .name  = "tlv320dac3203",
      .owner = THIS_MODULE,
   },
   .probe    = dac3203_i2c_probe,
   .remove   = dac3203_i2c_remove,
   .id_table = dac3203_i2c_id,
};

module_i2c_driver(dac3203_i2c_driver);

MODULE_DESCRIPTION("ASoC tlv320dac3203 codec driver");
MODULE_AUTHOR("Kevin Li <kevin-ke.li@broadcom.com>");
MODULE_LICENSE("GPL");
