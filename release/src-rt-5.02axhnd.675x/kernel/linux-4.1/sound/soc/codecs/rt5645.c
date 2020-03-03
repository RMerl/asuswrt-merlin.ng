/*
 * rt5645.c  --  RT5645 ALSA SoC audio codec driver
 *
 * Copyright 2013 Realtek Semiconductor Corp.
 * Author: Bard Liao <bardliao@realtek.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <linux/pm.h>
#include <linux/i2c.h>
#include <linux/platform_device.h>
#include <linux/spi/spi.h>
#include <linux/gpio.h>
#include <linux/acpi.h>
#include <sound/core.h>
#include <sound/pcm.h>
#include <sound/pcm_params.h>
#include <sound/jack.h>
#include <sound/soc.h>
#include <sound/soc-dapm.h>
#include <sound/initval.h>
#include <sound/tlv.h>

#include "rl6231.h"
#include "rt5645.h"

#define RT5645_DEVICE_ID 0x6308
#define RT5650_DEVICE_ID 0x6419

#define RT5645_PR_RANGE_BASE (0xff + 1)
#define RT5645_PR_SPACING 0x100

#define RT5645_PR_BASE (RT5645_PR_RANGE_BASE + (0 * RT5645_PR_SPACING))

static const struct regmap_range_cfg rt5645_ranges[] = {
	{
		.name = "PR",
		.range_min = RT5645_PR_BASE,
		.range_max = RT5645_PR_BASE + 0xf8,
		.selector_reg = RT5645_PRIV_INDEX,
		.selector_mask = 0xff,
		.selector_shift = 0x0,
		.window_start = RT5645_PRIV_DATA,
		.window_len = 0x1,
	},
};

static const struct reg_default init_list[] = {
	{RT5645_PR_BASE + 0x3d,	0x3600},
	{RT5645_PR_BASE + 0x1c,	0xfd20},
	{RT5645_PR_BASE + 0x20,	0x611f},
	{RT5645_PR_BASE + 0x21,	0x4040},
	{RT5645_PR_BASE + 0x23,	0x0004},
};
#define RT5645_INIT_REG_LEN ARRAY_SIZE(init_list)

static const struct reg_default rt5650_init_list[] = {
	{0xf6,	0x0100},
};

static const struct reg_default rt5645_reg[] = {
	{ 0x00, 0x0000 },
	{ 0x01, 0xc8c8 },
	{ 0x02, 0xc8c8 },
	{ 0x03, 0xc8c8 },
	{ 0x0a, 0x0002 },
	{ 0x0b, 0x2827 },
	{ 0x0c, 0xe000 },
	{ 0x0d, 0x0000 },
	{ 0x0e, 0x0000 },
	{ 0x0f, 0x0808 },
	{ 0x14, 0x3333 },
	{ 0x16, 0x4b00 },
	{ 0x18, 0x018b },
	{ 0x19, 0xafaf },
	{ 0x1a, 0xafaf },
	{ 0x1b, 0x0001 },
	{ 0x1c, 0x2f2f },
	{ 0x1d, 0x2f2f },
	{ 0x1e, 0x0000 },
	{ 0x20, 0x0000 },
	{ 0x27, 0x7060 },
	{ 0x28, 0x7070 },
	{ 0x29, 0x8080 },
	{ 0x2a, 0x5656 },
	{ 0x2b, 0x5454 },
	{ 0x2c, 0xaaa0 },
	{ 0x2d, 0x0000 },
	{ 0x2f, 0x1002 },
	{ 0x31, 0x5000 },
	{ 0x32, 0x0000 },
	{ 0x33, 0x0000 },
	{ 0x34, 0x0000 },
	{ 0x35, 0x0000 },
	{ 0x3b, 0x0000 },
	{ 0x3c, 0x007f },
	{ 0x3d, 0x0000 },
	{ 0x3e, 0x007f },
	{ 0x3f, 0x0000 },
	{ 0x40, 0x001f },
	{ 0x41, 0x0000 },
	{ 0x42, 0x001f },
	{ 0x45, 0x6000 },
	{ 0x46, 0x003e },
	{ 0x47, 0x003e },
	{ 0x48, 0xf807 },
	{ 0x4a, 0x0004 },
	{ 0x4d, 0x0000 },
	{ 0x4e, 0x0000 },
	{ 0x4f, 0x01ff },
	{ 0x50, 0x0000 },
	{ 0x51, 0x0000 },
	{ 0x52, 0x01ff },
	{ 0x53, 0xf000 },
	{ 0x56, 0x0111 },
	{ 0x57, 0x0064 },
	{ 0x58, 0xef0e },
	{ 0x59, 0xf0f0 },
	{ 0x5a, 0xef0e },
	{ 0x5b, 0xf0f0 },
	{ 0x5c, 0xef0e },
	{ 0x5d, 0xf0f0 },
	{ 0x5e, 0xf000 },
	{ 0x5f, 0x0000 },
	{ 0x61, 0x0300 },
	{ 0x62, 0x0000 },
	{ 0x63, 0x00c2 },
	{ 0x64, 0x0000 },
	{ 0x65, 0x0000 },
	{ 0x66, 0x0000 },
	{ 0x6a, 0x0000 },
	{ 0x6c, 0x0aaa },
	{ 0x70, 0x8000 },
	{ 0x71, 0x8000 },
	{ 0x72, 0x8000 },
	{ 0x73, 0x7770 },
	{ 0x74, 0x3e00 },
	{ 0x75, 0x2409 },
	{ 0x76, 0x000a },
	{ 0x77, 0x0c00 },
	{ 0x78, 0x0000 },
	{ 0x79, 0x0123 },
	{ 0x80, 0x0000 },
	{ 0x81, 0x0000 },
	{ 0x82, 0x0000 },
	{ 0x83, 0x0000 },
	{ 0x84, 0x0000 },
	{ 0x85, 0x0000 },
	{ 0x8a, 0x0000 },
	{ 0x8e, 0x0004 },
	{ 0x8f, 0x1100 },
	{ 0x90, 0x0646 },
	{ 0x91, 0x0c06 },
	{ 0x93, 0x0000 },
	{ 0x94, 0x0200 },
	{ 0x95, 0x0000 },
	{ 0x9a, 0x2184 },
	{ 0x9b, 0x010a },
	{ 0x9c, 0x0aea },
	{ 0x9d, 0x000c },
	{ 0x9e, 0x0400 },
	{ 0xa0, 0xa0a8 },
	{ 0xa1, 0x0059 },
	{ 0xa2, 0x0001 },
	{ 0xae, 0x6000 },
	{ 0xaf, 0x0000 },
	{ 0xb0, 0x6000 },
	{ 0xb1, 0x0000 },
	{ 0xb2, 0x0000 },
	{ 0xb3, 0x001f },
	{ 0xb4, 0x020c },
	{ 0xb5, 0x1f00 },
	{ 0xb6, 0x0000 },
	{ 0xbb, 0x0000 },
	{ 0xbc, 0x0000 },
	{ 0xbd, 0x0000 },
	{ 0xbe, 0x0000 },
	{ 0xbf, 0x3100 },
	{ 0xc0, 0x0000 },
	{ 0xc1, 0x0000 },
	{ 0xc2, 0x0000 },
	{ 0xc3, 0x2000 },
	{ 0xcd, 0x0000 },
	{ 0xce, 0x0000 },
	{ 0xcf, 0x1813 },
	{ 0xd0, 0x0690 },
	{ 0xd1, 0x1c17 },
	{ 0xd3, 0xb320 },
	{ 0xd4, 0x0000 },
	{ 0xd6, 0x0400 },
	{ 0xd9, 0x0809 },
	{ 0xda, 0x0000 },
	{ 0xdb, 0x0003 },
	{ 0xdc, 0x0049 },
	{ 0xdd, 0x001b },
	{ 0xdf, 0x0008 },
	{ 0xe0, 0x4000 },
	{ 0xe6, 0x8000 },
	{ 0xe7, 0x0200 },
	{ 0xec, 0xb300 },
	{ 0xed, 0x0000 },
	{ 0xf0, 0x001f },
	{ 0xf1, 0x020c },
	{ 0xf2, 0x1f00 },
	{ 0xf3, 0x0000 },
	{ 0xf4, 0x4000 },
	{ 0xf8, 0x0000 },
	{ 0xf9, 0x0000 },
	{ 0xfa, 0x2060 },
	{ 0xfb, 0x4040 },
	{ 0xfc, 0x0000 },
	{ 0xfd, 0x0002 },
	{ 0xfe, 0x10ec },
	{ 0xff, 0x6308 },
};

static int rt5645_reset(struct snd_soc_codec *codec)
{
	return snd_soc_write(codec, RT5645_RESET, 0);
}

static bool rt5645_volatile_register(struct device *dev, unsigned int reg)
{
	int i;

	for (i = 0; i < ARRAY_SIZE(rt5645_ranges); i++) {
		if (reg >= rt5645_ranges[i].range_min &&
			reg <= rt5645_ranges[i].range_max) {
			return true;
		}
	}

	switch (reg) {
	case RT5645_RESET:
	case RT5645_PRIV_DATA:
	case RT5645_IN1_CTRL1:
	case RT5645_IN1_CTRL2:
	case RT5645_IN1_CTRL3:
	case RT5645_A_JD_CTRL1:
	case RT5645_ADC_EQ_CTRL1:
	case RT5645_EQ_CTRL1:
	case RT5645_ALC_CTRL_1:
	case RT5645_IRQ_CTRL2:
	case RT5645_IRQ_CTRL3:
	case RT5645_INT_IRQ_ST:
	case RT5645_IL_CMD:
	case RT5650_4BTN_IL_CMD1:
	case RT5645_VENDOR_ID:
	case RT5645_VENDOR_ID1:
	case RT5645_VENDOR_ID2:
		return true;
	default:
		return false;
	}
}

static bool rt5645_readable_register(struct device *dev, unsigned int reg)
{
	int i;

	for (i = 0; i < ARRAY_SIZE(rt5645_ranges); i++) {
		if (reg >= rt5645_ranges[i].range_min &&
			reg <= rt5645_ranges[i].range_max) {
			return true;
		}
	}

	switch (reg) {
	case RT5645_RESET:
	case RT5645_SPK_VOL:
	case RT5645_HP_VOL:
	case RT5645_LOUT1:
	case RT5645_IN1_CTRL1:
	case RT5645_IN1_CTRL2:
	case RT5645_IN1_CTRL3:
	case RT5645_IN2_CTRL:
	case RT5645_INL1_INR1_VOL:
	case RT5645_SPK_FUNC_LIM:
	case RT5645_ADJ_HPF_CTRL:
	case RT5645_DAC1_DIG_VOL:
	case RT5645_DAC2_DIG_VOL:
	case RT5645_DAC_CTRL:
	case RT5645_STO1_ADC_DIG_VOL:
	case RT5645_MONO_ADC_DIG_VOL:
	case RT5645_ADC_BST_VOL1:
	case RT5645_ADC_BST_VOL2:
	case RT5645_STO1_ADC_MIXER:
	case RT5645_MONO_ADC_MIXER:
	case RT5645_AD_DA_MIXER:
	case RT5645_STO_DAC_MIXER:
	case RT5645_MONO_DAC_MIXER:
	case RT5645_DIG_MIXER:
	case RT5650_A_DAC_SOUR:
	case RT5645_DIG_INF1_DATA:
	case RT5645_PDM_OUT_CTRL:
	case RT5645_REC_L1_MIXER:
	case RT5645_REC_L2_MIXER:
	case RT5645_REC_R1_MIXER:
	case RT5645_REC_R2_MIXER:
	case RT5645_HPMIXL_CTRL:
	case RT5645_HPOMIXL_CTRL:
	case RT5645_HPMIXR_CTRL:
	case RT5645_HPOMIXR_CTRL:
	case RT5645_HPO_MIXER:
	case RT5645_SPK_L_MIXER:
	case RT5645_SPK_R_MIXER:
	case RT5645_SPO_MIXER:
	case RT5645_SPO_CLSD_RATIO:
	case RT5645_OUT_L1_MIXER:
	case RT5645_OUT_R1_MIXER:
	case RT5645_OUT_L_GAIN1:
	case RT5645_OUT_L_GAIN2:
	case RT5645_OUT_R_GAIN1:
	case RT5645_OUT_R_GAIN2:
	case RT5645_LOUT_MIXER:
	case RT5645_HAPTIC_CTRL1:
	case RT5645_HAPTIC_CTRL2:
	case RT5645_HAPTIC_CTRL3:
	case RT5645_HAPTIC_CTRL4:
	case RT5645_HAPTIC_CTRL5:
	case RT5645_HAPTIC_CTRL6:
	case RT5645_HAPTIC_CTRL7:
	case RT5645_HAPTIC_CTRL8:
	case RT5645_HAPTIC_CTRL9:
	case RT5645_HAPTIC_CTRL10:
	case RT5645_PWR_DIG1:
	case RT5645_PWR_DIG2:
	case RT5645_PWR_ANLG1:
	case RT5645_PWR_ANLG2:
	case RT5645_PWR_MIXER:
	case RT5645_PWR_VOL:
	case RT5645_PRIV_INDEX:
	case RT5645_PRIV_DATA:
	case RT5645_I2S1_SDP:
	case RT5645_I2S2_SDP:
	case RT5645_ADDA_CLK1:
	case RT5645_ADDA_CLK2:
	case RT5645_DMIC_CTRL1:
	case RT5645_DMIC_CTRL2:
	case RT5645_TDM_CTRL_1:
	case RT5645_TDM_CTRL_2:
	case RT5645_TDM_CTRL_3:
	case RT5645_GLB_CLK:
	case RT5645_PLL_CTRL1:
	case RT5645_PLL_CTRL2:
	case RT5645_ASRC_1:
	case RT5645_ASRC_2:
	case RT5645_ASRC_3:
	case RT5645_ASRC_4:
	case RT5645_DEPOP_M1:
	case RT5645_DEPOP_M2:
	case RT5645_DEPOP_M3:
	case RT5645_MICBIAS:
	case RT5645_A_JD_CTRL1:
	case RT5645_VAD_CTRL4:
	case RT5645_CLSD_OUT_CTRL:
	case RT5645_ADC_EQ_CTRL1:
	case RT5645_ADC_EQ_CTRL2:
	case RT5645_EQ_CTRL1:
	case RT5645_EQ_CTRL2:
	case RT5645_ALC_CTRL_1:
	case RT5645_ALC_CTRL_2:
	case RT5645_ALC_CTRL_3:
	case RT5645_ALC_CTRL_4:
	case RT5645_ALC_CTRL_5:
	case RT5645_JD_CTRL:
	case RT5645_IRQ_CTRL1:
	case RT5645_IRQ_CTRL2:
	case RT5645_IRQ_CTRL3:
	case RT5645_INT_IRQ_ST:
	case RT5645_GPIO_CTRL1:
	case RT5645_GPIO_CTRL2:
	case RT5645_GPIO_CTRL3:
	case RT5645_BASS_BACK:
	case RT5645_MP3_PLUS1:
	case RT5645_MP3_PLUS2:
	case RT5645_ADJ_HPF1:
	case RT5645_ADJ_HPF2:
	case RT5645_HP_CALIB_AMP_DET:
	case RT5645_SV_ZCD1:
	case RT5645_SV_ZCD2:
	case RT5645_IL_CMD:
	case RT5645_IL_CMD2:
	case RT5645_IL_CMD3:
	case RT5650_4BTN_IL_CMD1:
	case RT5650_4BTN_IL_CMD2:
	case RT5645_DRC1_HL_CTRL1:
	case RT5645_DRC2_HL_CTRL1:
	case RT5645_ADC_MONO_HP_CTRL1:
	case RT5645_ADC_MONO_HP_CTRL2:
	case RT5645_DRC2_CTRL1:
	case RT5645_DRC2_CTRL2:
	case RT5645_DRC2_CTRL3:
	case RT5645_DRC2_CTRL4:
	case RT5645_DRC2_CTRL5:
	case RT5645_JD_CTRL3:
	case RT5645_JD_CTRL4:
	case RT5645_GEN_CTRL1:
	case RT5645_GEN_CTRL2:
	case RT5645_GEN_CTRL3:
	case RT5645_VENDOR_ID:
	case RT5645_VENDOR_ID1:
	case RT5645_VENDOR_ID2:
		return true;
	default:
		return false;
	}
}

static const DECLARE_TLV_DB_SCALE(out_vol_tlv, -4650, 150, 0);
static const DECLARE_TLV_DB_SCALE(dac_vol_tlv, -65625, 375, 0);
static const DECLARE_TLV_DB_SCALE(in_vol_tlv, -3450, 150, 0);
static const DECLARE_TLV_DB_SCALE(adc_vol_tlv, -17625, 375, 0);
static const DECLARE_TLV_DB_SCALE(adc_bst_tlv, 0, 1200, 0);

/* {0, +20, +24, +30, +35, +40, +44, +50, +52} dB */
static unsigned int bst_tlv[] = {
	TLV_DB_RANGE_HEAD(7),
	0, 0, TLV_DB_SCALE_ITEM(0, 0, 0),
	1, 1, TLV_DB_SCALE_ITEM(2000, 0, 0),
	2, 2, TLV_DB_SCALE_ITEM(2400, 0, 0),
	3, 5, TLV_DB_SCALE_ITEM(3000, 500, 0),
	6, 6, TLV_DB_SCALE_ITEM(4400, 0, 0),
	7, 7, TLV_DB_SCALE_ITEM(5000, 0, 0),
	8, 8, TLV_DB_SCALE_ITEM(5200, 0, 0),
};

static const char * const rt5645_tdm_data_swap_select[] = {
	"L/R", "R/L", "L/L", "R/R"
};

static SOC_ENUM_SINGLE_DECL(rt5645_tdm_adc_slot0_1_enum,
	RT5645_TDM_CTRL_1, 6, rt5645_tdm_data_swap_select);

static SOC_ENUM_SINGLE_DECL(rt5645_tdm_adc_slot2_3_enum,
	RT5645_TDM_CTRL_1, 4, rt5645_tdm_data_swap_select);

static SOC_ENUM_SINGLE_DECL(rt5645_tdm_adc_slot4_5_enum,
	RT5645_TDM_CTRL_1, 2, rt5645_tdm_data_swap_select);

static SOC_ENUM_SINGLE_DECL(rt5645_tdm_adc_slot6_7_enum,
	RT5645_TDM_CTRL_1, 0, rt5645_tdm_data_swap_select);

static const char * const rt5645_tdm_adc_data_select[] = {
	"1/2/R", "2/1/R", "R/1/2", "R/2/1"
};

static SOC_ENUM_SINGLE_DECL(rt5645_tdm_adc_sel_enum,
				RT5645_TDM_CTRL_1, 8,
				rt5645_tdm_adc_data_select);

static const struct snd_kcontrol_new rt5645_snd_controls[] = {
	/* Speaker Output Volume */
	SOC_DOUBLE("Speaker Channel Switch", RT5645_SPK_VOL,
		RT5645_VOL_L_SFT, RT5645_VOL_R_SFT, 1, 1),
	SOC_DOUBLE_TLV("Speaker Playback Volume", RT5645_SPK_VOL,
		RT5645_L_VOL_SFT, RT5645_R_VOL_SFT, 39, 1, out_vol_tlv),

	/* Headphone Output Volume */
	SOC_DOUBLE("HP Channel Switch", RT5645_HP_VOL,
		RT5645_VOL_L_SFT, RT5645_VOL_R_SFT, 1, 1),
	SOC_DOUBLE_TLV("HP Playback Volume", RT5645_HP_VOL,
		RT5645_L_VOL_SFT, RT5645_R_VOL_SFT, 39, 1, out_vol_tlv),

	/* OUTPUT Control */
	SOC_DOUBLE("OUT Playback Switch", RT5645_LOUT1,
		RT5645_L_MUTE_SFT, RT5645_R_MUTE_SFT, 1, 1),
	SOC_DOUBLE("OUT Channel Switch", RT5645_LOUT1,
		RT5645_VOL_L_SFT, RT5645_VOL_R_SFT, 1, 1),
	SOC_DOUBLE_TLV("OUT Playback Volume", RT5645_LOUT1,
		RT5645_L_VOL_SFT, RT5645_R_VOL_SFT, 39, 1, out_vol_tlv),

	/* DAC Digital Volume */
	SOC_DOUBLE("DAC2 Playback Switch", RT5645_DAC_CTRL,
		RT5645_M_DAC_L2_VOL_SFT, RT5645_M_DAC_R2_VOL_SFT, 1, 1),
	SOC_DOUBLE_TLV("DAC1 Playback Volume", RT5645_DAC1_DIG_VOL,
		RT5645_L_VOL_SFT, RT5645_R_VOL_SFT, 175, 0, dac_vol_tlv),
	SOC_DOUBLE_TLV("Mono DAC Playback Volume", RT5645_DAC2_DIG_VOL,
		RT5645_L_VOL_SFT, RT5645_R_VOL_SFT, 175, 0, dac_vol_tlv),

	/* IN1/IN2 Control */
	SOC_SINGLE_TLV("IN1 Boost", RT5645_IN1_CTRL1,
		RT5645_BST_SFT1, 12, 0, bst_tlv),
	SOC_SINGLE_TLV("IN2 Boost", RT5645_IN2_CTRL,
		RT5645_BST_SFT2, 8, 0, bst_tlv),

	/* INL/INR Volume Control */
	SOC_DOUBLE_TLV("IN Capture Volume", RT5645_INL1_INR1_VOL,
		RT5645_INL_VOL_SFT, RT5645_INR_VOL_SFT, 31, 1, in_vol_tlv),

	/* ADC Digital Volume Control */
	SOC_DOUBLE("ADC Capture Switch", RT5645_STO1_ADC_DIG_VOL,
		RT5645_L_MUTE_SFT, RT5645_R_MUTE_SFT, 1, 1),
	SOC_DOUBLE_TLV("ADC Capture Volume", RT5645_STO1_ADC_DIG_VOL,
		RT5645_L_VOL_SFT, RT5645_R_VOL_SFT, 127, 0, adc_vol_tlv),
	SOC_DOUBLE("Mono ADC Capture Switch", RT5645_MONO_ADC_DIG_VOL,
		RT5645_L_MUTE_SFT, RT5645_R_MUTE_SFT, 1, 1),
	SOC_DOUBLE_TLV("Mono ADC Capture Volume", RT5645_MONO_ADC_DIG_VOL,
		RT5645_L_VOL_SFT, RT5645_R_VOL_SFT, 127, 0, adc_vol_tlv),

	/* ADC Boost Volume Control */
	SOC_DOUBLE_TLV("STO1 ADC Boost Gain", RT5645_ADC_BST_VOL1,
		RT5645_STO1_ADC_L_BST_SFT, RT5645_STO1_ADC_R_BST_SFT, 3, 0,
		adc_bst_tlv),
	SOC_DOUBLE_TLV("STO2 ADC Boost Gain", RT5645_ADC_BST_VOL1,
		RT5645_STO2_ADC_L_BST_SFT, RT5645_STO2_ADC_R_BST_SFT, 3, 0,
		adc_bst_tlv),

	/* I2S2 function select */
	SOC_SINGLE("I2S2 Func Switch", RT5645_GPIO_CTRL1, RT5645_I2S2_SEL_SFT,
		1, 1),

	/* TDM */
	SOC_ENUM("TDM Adc Slot0 1 Data", rt5645_tdm_adc_slot0_1_enum),
	SOC_ENUM("TDM Adc Slot2 3 Data", rt5645_tdm_adc_slot2_3_enum),
	SOC_ENUM("TDM Adc Slot4 5 Data", rt5645_tdm_adc_slot4_5_enum),
	SOC_ENUM("TDM Adc Slot6 7 Data", rt5645_tdm_adc_slot6_7_enum),
	SOC_ENUM("TDM IF1 ADC DATA Sel", rt5645_tdm_adc_sel_enum),
	SOC_SINGLE("TDM IF1_DAC1_L Sel", RT5645_TDM_CTRL_3, 12, 7, 0),
	SOC_SINGLE("TDM IF1_DAC1_R Sel", RT5645_TDM_CTRL_3, 8, 7, 0),
	SOC_SINGLE("TDM IF1_DAC2_L Sel", RT5645_TDM_CTRL_3, 4, 7, 0),
	SOC_SINGLE("TDM IF1_DAC2_R Sel", RT5645_TDM_CTRL_3, 0, 7, 0),
};

/**
 * set_dmic_clk - Set parameter of dmic.
 *
 * @w: DAPM widget.
 * @kcontrol: The kcontrol of this widget.
 * @event: Event id.
 *
 */
static int set_dmic_clk(struct snd_soc_dapm_widget *w,
	struct snd_kcontrol *kcontrol, int event)
{
	struct snd_soc_codec *codec = snd_soc_dapm_to_codec(w->dapm);
	struct rt5645_priv *rt5645 = snd_soc_codec_get_drvdata(codec);
	int idx = -EINVAL;

	idx = rl6231_calc_dmic_clk(rt5645->sysclk);

	if (idx < 0)
		dev_err(codec->dev, "Failed to set DMIC clock\n");
	else
		snd_soc_update_bits(codec, RT5645_DMIC_CTRL1,
			RT5645_DMIC_CLK_MASK, idx << RT5645_DMIC_CLK_SFT);
	return idx;
}

static int is_sys_clk_from_pll(struct snd_soc_dapm_widget *source,
			 struct snd_soc_dapm_widget *sink)
{
	struct snd_soc_codec *codec = snd_soc_dapm_to_codec(source->dapm);
	unsigned int val;

	val = snd_soc_read(codec, RT5645_GLB_CLK);
	val &= RT5645_SCLK_SRC_MASK;
	if (val == RT5645_SCLK_SRC_PLL1)
		return 1;
	else
		return 0;
}

static int is_using_asrc(struct snd_soc_dapm_widget *source,
			 struct snd_soc_dapm_widget *sink)
{
	struct snd_soc_codec *codec = snd_soc_dapm_to_codec(source->dapm);
	unsigned int reg, shift, val;

	switch (source->shift) {
	case 0:
		reg = RT5645_ASRC_3;
		shift = 0;
		break;
	case 1:
		reg = RT5645_ASRC_3;
		shift = 4;
		break;
	case 3:
		reg = RT5645_ASRC_2;
		shift = 0;
		break;
	case 8:
		reg = RT5645_ASRC_2;
		shift = 4;
		break;
	case 9:
		reg = RT5645_ASRC_2;
		shift = 8;
		break;
	case 10:
		reg = RT5645_ASRC_2;
		shift = 12;
		break;
	default:
		return 0;
	}

	val = (snd_soc_read(codec, reg) >> shift) & 0xf;
	switch (val) {
	case 1:
	case 2:
	case 3:
	case 4:
		return 1;
	default:
		return 0;
	}

}

/**
 * rt5645_sel_asrc_clk_src - select ASRC clock source for a set of filters
 * @codec: SoC audio codec device.
 * @filter_mask: mask of filters.
 * @clk_src: clock source
 *
 * The ASRC function is for asynchronous MCLK and LRCK. Also, since RT5645 can
 * only support standard 32fs or 64fs i2s format, ASRC should be enabled to
 * support special i2s clock format such as Intel's 100fs(100 * sampling rate).
 * ASRC function will track i2s clock and generate a corresponding system clock
 * for codec. This function provides an API to select the clock source for a
 * set of filters specified by the mask. And the codec driver will turn on ASRC
 * for these filters if ASRC is selected as their clock source.
 */
int rt5645_sel_asrc_clk_src(struct snd_soc_codec *codec,
		unsigned int filter_mask, unsigned int clk_src)
{
	unsigned int asrc2_mask = 0;
	unsigned int asrc2_value = 0;
	unsigned int asrc3_mask = 0;
	unsigned int asrc3_value = 0;

	switch (clk_src) {
	case RT5645_CLK_SEL_SYS:
	case RT5645_CLK_SEL_I2S1_ASRC:
	case RT5645_CLK_SEL_I2S2_ASRC:
	case RT5645_CLK_SEL_SYS2:
		break;

	default:
		return -EINVAL;
	}

	if (filter_mask & RT5645_DA_STEREO_FILTER) {
		asrc2_mask |= RT5645_DA_STO_CLK_SEL_MASK;
		asrc2_value = (asrc2_value & ~RT5645_DA_STO_CLK_SEL_MASK)
			| (clk_src << RT5645_DA_STO_CLK_SEL_SFT);
	}

	if (filter_mask & RT5645_DA_MONO_L_FILTER) {
		asrc2_mask |= RT5645_DA_MONOL_CLK_SEL_MASK;
		asrc2_value = (asrc2_value & ~RT5645_DA_MONOL_CLK_SEL_MASK)
			| (clk_src << RT5645_DA_MONOL_CLK_SEL_SFT);
	}

	if (filter_mask & RT5645_DA_MONO_R_FILTER) {
		asrc2_mask |= RT5645_DA_MONOR_CLK_SEL_MASK;
		asrc2_value = (asrc2_value & ~RT5645_DA_MONOR_CLK_SEL_MASK)
			| (clk_src << RT5645_DA_MONOR_CLK_SEL_SFT);
	}

	if (filter_mask & RT5645_AD_STEREO_FILTER) {
		asrc2_mask |= RT5645_AD_STO1_CLK_SEL_MASK;
		asrc2_value = (asrc2_value & ~RT5645_AD_STO1_CLK_SEL_MASK)
			| (clk_src << RT5645_AD_STO1_CLK_SEL_SFT);
	}

	if (filter_mask & RT5645_AD_MONO_L_FILTER) {
		asrc3_mask |= RT5645_AD_MONOL_CLK_SEL_MASK;
		asrc3_value = (asrc3_value & ~RT5645_AD_MONOL_CLK_SEL_MASK)
			| (clk_src << RT5645_AD_MONOL_CLK_SEL_SFT);
	}

	if (filter_mask & RT5645_AD_MONO_R_FILTER)  {
		asrc3_mask |= RT5645_AD_MONOR_CLK_SEL_MASK;
		asrc3_value = (asrc3_value & ~RT5645_AD_MONOR_CLK_SEL_MASK)
			| (clk_src << RT5645_AD_MONOR_CLK_SEL_SFT);
	}

	if (asrc2_mask)
		snd_soc_update_bits(codec, RT5645_ASRC_2,
			asrc2_mask, asrc2_value);

	if (asrc3_mask)
		snd_soc_update_bits(codec, RT5645_ASRC_3,
			asrc3_mask, asrc3_value);

	return 0;
}
EXPORT_SYMBOL_GPL(rt5645_sel_asrc_clk_src);

/* Digital Mixer */
static const struct snd_kcontrol_new rt5645_sto1_adc_l_mix[] = {
	SOC_DAPM_SINGLE("ADC1 Switch", RT5645_STO1_ADC_MIXER,
			RT5645_M_ADC_L1_SFT, 1, 1),
	SOC_DAPM_SINGLE("ADC2 Switch", RT5645_STO1_ADC_MIXER,
			RT5645_M_ADC_L2_SFT, 1, 1),
};

static const struct snd_kcontrol_new rt5645_sto1_adc_r_mix[] = {
	SOC_DAPM_SINGLE("ADC1 Switch", RT5645_STO1_ADC_MIXER,
			RT5645_M_ADC_R1_SFT, 1, 1),
	SOC_DAPM_SINGLE("ADC2 Switch", RT5645_STO1_ADC_MIXER,
			RT5645_M_ADC_R2_SFT, 1, 1),
};

static const struct snd_kcontrol_new rt5645_mono_adc_l_mix[] = {
	SOC_DAPM_SINGLE("ADC1 Switch", RT5645_MONO_ADC_MIXER,
			RT5645_M_MONO_ADC_L1_SFT, 1, 1),
	SOC_DAPM_SINGLE("ADC2 Switch", RT5645_MONO_ADC_MIXER,
			RT5645_M_MONO_ADC_L2_SFT, 1, 1),
};

static const struct snd_kcontrol_new rt5645_mono_adc_r_mix[] = {
	SOC_DAPM_SINGLE("ADC1 Switch", RT5645_MONO_ADC_MIXER,
			RT5645_M_MONO_ADC_R1_SFT, 1, 1),
	SOC_DAPM_SINGLE("ADC2 Switch", RT5645_MONO_ADC_MIXER,
			RT5645_M_MONO_ADC_R2_SFT, 1, 1),
};

static const struct snd_kcontrol_new rt5645_dac_l_mix[] = {
	SOC_DAPM_SINGLE("Stereo ADC Switch", RT5645_AD_DA_MIXER,
			RT5645_M_ADCMIX_L_SFT, 1, 1),
	SOC_DAPM_SINGLE("DAC1 Switch", RT5645_AD_DA_MIXER,
			RT5645_M_DAC1_L_SFT, 1, 1),
};

static const struct snd_kcontrol_new rt5645_dac_r_mix[] = {
	SOC_DAPM_SINGLE("Stereo ADC Switch", RT5645_AD_DA_MIXER,
			RT5645_M_ADCMIX_R_SFT, 1, 1),
	SOC_DAPM_SINGLE("DAC1 Switch", RT5645_AD_DA_MIXER,
			RT5645_M_DAC1_R_SFT, 1, 1),
};

static const struct snd_kcontrol_new rt5645_sto_dac_l_mix[] = {
	SOC_DAPM_SINGLE("DAC L1 Switch", RT5645_STO_DAC_MIXER,
			RT5645_M_DAC_L1_SFT, 1, 1),
	SOC_DAPM_SINGLE("DAC L2 Switch", RT5645_STO_DAC_MIXER,
			RT5645_M_DAC_L2_SFT, 1, 1),
	SOC_DAPM_SINGLE("DAC R1 Switch", RT5645_STO_DAC_MIXER,
			RT5645_M_DAC_R1_STO_L_SFT, 1, 1),
};

static const struct snd_kcontrol_new rt5645_sto_dac_r_mix[] = {
	SOC_DAPM_SINGLE("DAC R1 Switch", RT5645_STO_DAC_MIXER,
			RT5645_M_DAC_R1_SFT, 1, 1),
	SOC_DAPM_SINGLE("DAC R2 Switch", RT5645_STO_DAC_MIXER,
			RT5645_M_DAC_R2_SFT, 1, 1),
	SOC_DAPM_SINGLE("DAC L1 Switch", RT5645_STO_DAC_MIXER,
			RT5645_M_DAC_L1_STO_R_SFT, 1, 1),
};

static const struct snd_kcontrol_new rt5645_mono_dac_l_mix[] = {
	SOC_DAPM_SINGLE("DAC L1 Switch", RT5645_MONO_DAC_MIXER,
			RT5645_M_DAC_L1_MONO_L_SFT, 1, 1),
	SOC_DAPM_SINGLE("DAC L2 Switch", RT5645_MONO_DAC_MIXER,
			RT5645_M_DAC_L2_MONO_L_SFT, 1, 1),
	SOC_DAPM_SINGLE("DAC R2 Switch", RT5645_MONO_DAC_MIXER,
			RT5645_M_DAC_R2_MONO_L_SFT, 1, 1),
};

static const struct snd_kcontrol_new rt5645_mono_dac_r_mix[] = {
	SOC_DAPM_SINGLE("DAC R1 Switch", RT5645_MONO_DAC_MIXER,
			RT5645_M_DAC_R1_MONO_R_SFT, 1, 1),
	SOC_DAPM_SINGLE("DAC R2 Switch", RT5645_MONO_DAC_MIXER,
			RT5645_M_DAC_R2_MONO_R_SFT, 1, 1),
	SOC_DAPM_SINGLE("DAC L2 Switch", RT5645_MONO_DAC_MIXER,
			RT5645_M_DAC_L2_MONO_R_SFT, 1, 1),
};

static const struct snd_kcontrol_new rt5645_dig_l_mix[] = {
	SOC_DAPM_SINGLE("Sto DAC Mix L Switch", RT5645_DIG_MIXER,
			RT5645_M_STO_L_DAC_L_SFT, 1, 1),
	SOC_DAPM_SINGLE("DAC L2 Switch", RT5645_DIG_MIXER,
			RT5645_M_DAC_L2_DAC_L_SFT, 1, 1),
	SOC_DAPM_SINGLE("DAC R2 Switch", RT5645_DIG_MIXER,
			RT5645_M_DAC_R2_DAC_L_SFT, 1, 1),
};

static const struct snd_kcontrol_new rt5645_dig_r_mix[] = {
	SOC_DAPM_SINGLE("Sto DAC Mix R Switch", RT5645_DIG_MIXER,
			RT5645_M_STO_R_DAC_R_SFT, 1, 1),
	SOC_DAPM_SINGLE("DAC R2 Switch", RT5645_DIG_MIXER,
			RT5645_M_DAC_R2_DAC_R_SFT, 1, 1),
	SOC_DAPM_SINGLE("DAC L2 Switch", RT5645_DIG_MIXER,
			RT5645_M_DAC_L2_DAC_R_SFT, 1, 1),
};

/* Analog Input Mixer */
static const struct snd_kcontrol_new rt5645_rec_l_mix[] = {
	SOC_DAPM_SINGLE("HPOL Switch", RT5645_REC_L2_MIXER,
			RT5645_M_HP_L_RM_L_SFT, 1, 1),
	SOC_DAPM_SINGLE("INL Switch", RT5645_REC_L2_MIXER,
			RT5645_M_IN_L_RM_L_SFT, 1, 1),
	SOC_DAPM_SINGLE("BST2 Switch", RT5645_REC_L2_MIXER,
			RT5645_M_BST2_RM_L_SFT, 1, 1),
	SOC_DAPM_SINGLE("BST1 Switch", RT5645_REC_L2_MIXER,
			RT5645_M_BST1_RM_L_SFT, 1, 1),
	SOC_DAPM_SINGLE("OUT MIXL Switch", RT5645_REC_L2_MIXER,
			RT5645_M_OM_L_RM_L_SFT, 1, 1),
};

static const struct snd_kcontrol_new rt5645_rec_r_mix[] = {
	SOC_DAPM_SINGLE("HPOR Switch", RT5645_REC_R2_MIXER,
			RT5645_M_HP_R_RM_R_SFT, 1, 1),
	SOC_DAPM_SINGLE("INR Switch", RT5645_REC_R2_MIXER,
			RT5645_M_IN_R_RM_R_SFT, 1, 1),
	SOC_DAPM_SINGLE("BST2 Switch", RT5645_REC_R2_MIXER,
			RT5645_M_BST2_RM_R_SFT, 1, 1),
	SOC_DAPM_SINGLE("BST1 Switch", RT5645_REC_R2_MIXER,
			RT5645_M_BST1_RM_R_SFT, 1, 1),
	SOC_DAPM_SINGLE("OUT MIXR Switch", RT5645_REC_R2_MIXER,
			RT5645_M_OM_R_RM_R_SFT, 1, 1),
};

static const struct snd_kcontrol_new rt5645_spk_l_mix[] = {
	SOC_DAPM_SINGLE("DAC L1 Switch", RT5645_SPK_L_MIXER,
			RT5645_M_DAC_L1_SM_L_SFT, 1, 1),
	SOC_DAPM_SINGLE("DAC L2 Switch", RT5645_SPK_L_MIXER,
			RT5645_M_DAC_L2_SM_L_SFT, 1, 1),
	SOC_DAPM_SINGLE("INL Switch", RT5645_SPK_L_MIXER,
			RT5645_M_IN_L_SM_L_SFT, 1, 1),
	SOC_DAPM_SINGLE("BST1 Switch", RT5645_SPK_L_MIXER,
			RT5645_M_BST1_L_SM_L_SFT, 1, 1),
};

static const struct snd_kcontrol_new rt5645_spk_r_mix[] = {
	SOC_DAPM_SINGLE("DAC R1 Switch", RT5645_SPK_R_MIXER,
			RT5645_M_DAC_R1_SM_R_SFT, 1, 1),
	SOC_DAPM_SINGLE("DAC R2 Switch", RT5645_SPK_R_MIXER,
			RT5645_M_DAC_R2_SM_R_SFT, 1, 1),
	SOC_DAPM_SINGLE("INR Switch", RT5645_SPK_R_MIXER,
			RT5645_M_IN_R_SM_R_SFT, 1, 1),
	SOC_DAPM_SINGLE("BST2 Switch", RT5645_SPK_R_MIXER,
			RT5645_M_BST2_R_SM_R_SFT, 1, 1),
};

static const struct snd_kcontrol_new rt5645_out_l_mix[] = {
	SOC_DAPM_SINGLE("BST1 Switch", RT5645_OUT_L1_MIXER,
			RT5645_M_BST1_OM_L_SFT, 1, 1),
	SOC_DAPM_SINGLE("INL Switch", RT5645_OUT_L1_MIXER,
			RT5645_M_IN_L_OM_L_SFT, 1, 1),
	SOC_DAPM_SINGLE("DAC L2 Switch", RT5645_OUT_L1_MIXER,
			RT5645_M_DAC_L2_OM_L_SFT, 1, 1),
	SOC_DAPM_SINGLE("DAC L1 Switch", RT5645_OUT_L1_MIXER,
			RT5645_M_DAC_L1_OM_L_SFT, 1, 1),
};

static const struct snd_kcontrol_new rt5645_out_r_mix[] = {
	SOC_DAPM_SINGLE("BST2 Switch", RT5645_OUT_R1_MIXER,
			RT5645_M_BST2_OM_R_SFT, 1, 1),
	SOC_DAPM_SINGLE("INR Switch", RT5645_OUT_R1_MIXER,
			RT5645_M_IN_R_OM_R_SFT, 1, 1),
	SOC_DAPM_SINGLE("DAC R2 Switch", RT5645_OUT_R1_MIXER,
			RT5645_M_DAC_R2_OM_R_SFT, 1, 1),
	SOC_DAPM_SINGLE("DAC R1 Switch", RT5645_OUT_R1_MIXER,
			RT5645_M_DAC_R1_OM_R_SFT, 1, 1),
};

static const struct snd_kcontrol_new rt5645_spo_l_mix[] = {
	SOC_DAPM_SINGLE("DAC R1 Switch", RT5645_SPO_MIXER,
			RT5645_M_DAC_R1_SPM_L_SFT, 1, 1),
	SOC_DAPM_SINGLE("DAC L1 Switch", RT5645_SPO_MIXER,
			RT5645_M_DAC_L1_SPM_L_SFT, 1, 1),
	SOC_DAPM_SINGLE("SPKVOL R Switch", RT5645_SPO_MIXER,
			RT5645_M_SV_R_SPM_L_SFT, 1, 1),
	SOC_DAPM_SINGLE("SPKVOL L Switch", RT5645_SPO_MIXER,
			RT5645_M_SV_L_SPM_L_SFT, 1, 1),
};

static const struct snd_kcontrol_new rt5645_spo_r_mix[] = {
	SOC_DAPM_SINGLE("DAC R1 Switch", RT5645_SPO_MIXER,
			RT5645_M_DAC_R1_SPM_R_SFT, 1, 1),
	SOC_DAPM_SINGLE("SPKVOL R Switch", RT5645_SPO_MIXER,
			RT5645_M_SV_R_SPM_R_SFT, 1, 1),
};

static const struct snd_kcontrol_new rt5645_hpo_mix[] = {
	SOC_DAPM_SINGLE("DAC1 Switch", RT5645_HPO_MIXER,
			RT5645_M_DAC1_HM_SFT, 1, 1),
	SOC_DAPM_SINGLE("HPVOL Switch", RT5645_HPO_MIXER,
			RT5645_M_HPVOL_HM_SFT, 1, 1),
};

static const struct snd_kcontrol_new rt5645_hpvoll_mix[] = {
	SOC_DAPM_SINGLE("DAC1 Switch", RT5645_HPOMIXL_CTRL,
			RT5645_M_DAC1_HV_SFT, 1, 1),
	SOC_DAPM_SINGLE("DAC2 Switch", RT5645_HPOMIXL_CTRL,
			RT5645_M_DAC2_HV_SFT, 1, 1),
	SOC_DAPM_SINGLE("INL Switch", RT5645_HPOMIXL_CTRL,
			RT5645_M_IN_HV_SFT, 1, 1),
	SOC_DAPM_SINGLE("BST1 Switch", RT5645_HPOMIXL_CTRL,
			RT5645_M_BST1_HV_SFT, 1, 1),
};

static const struct snd_kcontrol_new rt5645_hpvolr_mix[] = {
	SOC_DAPM_SINGLE("DAC1 Switch", RT5645_HPOMIXR_CTRL,
			RT5645_M_DAC1_HV_SFT, 1, 1),
	SOC_DAPM_SINGLE("DAC2 Switch", RT5645_HPOMIXR_CTRL,
			RT5645_M_DAC2_HV_SFT, 1, 1),
	SOC_DAPM_SINGLE("INR Switch", RT5645_HPOMIXR_CTRL,
			RT5645_M_IN_HV_SFT, 1, 1),
	SOC_DAPM_SINGLE("BST2 Switch", RT5645_HPOMIXR_CTRL,
			RT5645_M_BST2_HV_SFT, 1, 1),
};

static const struct snd_kcontrol_new rt5645_lout_mix[] = {
	SOC_DAPM_SINGLE("DAC L1 Switch", RT5645_LOUT_MIXER,
			RT5645_M_DAC_L1_LM_SFT, 1, 1),
	SOC_DAPM_SINGLE("DAC R1 Switch", RT5645_LOUT_MIXER,
			RT5645_M_DAC_R1_LM_SFT, 1, 1),
	SOC_DAPM_SINGLE("OUTMIX L Switch", RT5645_LOUT_MIXER,
			RT5645_M_OV_L_LM_SFT, 1, 1),
	SOC_DAPM_SINGLE("OUTMIX R Switch", RT5645_LOUT_MIXER,
			RT5645_M_OV_R_LM_SFT, 1, 1),
};

/*DAC1 L/R source*/ /* MX-29 [9:8] [11:10] */
static const char * const rt5645_dac1_src[] = {
	"IF1 DAC", "IF2 DAC", "IF3 DAC"
};

static SOC_ENUM_SINGLE_DECL(
	rt5645_dac1l_enum, RT5645_AD_DA_MIXER,
	RT5645_DAC1_L_SEL_SFT, rt5645_dac1_src);

static const struct snd_kcontrol_new rt5645_dac1l_mux =
	SOC_DAPM_ENUM("DAC1 L source", rt5645_dac1l_enum);

static SOC_ENUM_SINGLE_DECL(
	rt5645_dac1r_enum, RT5645_AD_DA_MIXER,
	RT5645_DAC1_R_SEL_SFT, rt5645_dac1_src);

static const struct snd_kcontrol_new rt5645_dac1r_mux =
	SOC_DAPM_ENUM("DAC1 R source", rt5645_dac1r_enum);

/*DAC2 L/R source*/ /* MX-1B [6:4] [2:0] */
static const char * const rt5645_dac12_src[] = {
	"IF1 DAC", "IF2 DAC", "IF3 DAC", "Mono ADC", "VAD_ADC"
};

static SOC_ENUM_SINGLE_DECL(
	rt5645_dac2l_enum, RT5645_DAC_CTRL,
	RT5645_DAC2_L_SEL_SFT, rt5645_dac12_src);

static const struct snd_kcontrol_new rt5645_dac_l2_mux =
	SOC_DAPM_ENUM("DAC2 L source", rt5645_dac2l_enum);

static const char * const rt5645_dacr2_src[] = {
	"IF1 DAC", "IF2 DAC", "IF3 DAC", "Mono ADC", "Haptic"
};

static SOC_ENUM_SINGLE_DECL(
	rt5645_dac2r_enum, RT5645_DAC_CTRL,
	RT5645_DAC2_R_SEL_SFT, rt5645_dacr2_src);

static const struct snd_kcontrol_new rt5645_dac_r2_mux =
	SOC_DAPM_ENUM("DAC2 R source", rt5645_dac2r_enum);


/* INL/R source */
static const char * const rt5645_inl_src[] = {
	"IN2P", "MonoP"
};

static SOC_ENUM_SINGLE_DECL(
	rt5645_inl_enum, RT5645_INL1_INR1_VOL,
	RT5645_INL_SEL_SFT, rt5645_inl_src);

static const struct snd_kcontrol_new rt5645_inl_mux =
	SOC_DAPM_ENUM("INL source", rt5645_inl_enum);

static const char * const rt5645_inr_src[] = {
	"IN2N", "MonoN"
};

static SOC_ENUM_SINGLE_DECL(
	rt5645_inr_enum, RT5645_INL1_INR1_VOL,
	RT5645_INR_SEL_SFT, rt5645_inr_src);

static const struct snd_kcontrol_new rt5645_inr_mux =
	SOC_DAPM_ENUM("INR source", rt5645_inr_enum);

/* Stereo1 ADC source */
/* MX-27 [12] */
static const char * const rt5645_stereo_adc1_src[] = {
	"DAC MIX", "ADC"
};

static SOC_ENUM_SINGLE_DECL(
	rt5645_stereo1_adc1_enum, RT5645_STO1_ADC_MIXER,
	RT5645_ADC_1_SRC_SFT, rt5645_stereo_adc1_src);

static const struct snd_kcontrol_new rt5645_sto_adc1_mux =
	SOC_DAPM_ENUM("Stereo1 ADC1 Mux", rt5645_stereo1_adc1_enum);

/* MX-27 [11] */
static const char * const rt5645_stereo_adc2_src[] = {
	"DAC MIX", "DMIC"
};

static SOC_ENUM_SINGLE_DECL(
	rt5645_stereo1_adc2_enum, RT5645_STO1_ADC_MIXER,
	RT5645_ADC_2_SRC_SFT, rt5645_stereo_adc2_src);

static const struct snd_kcontrol_new rt5645_sto_adc2_mux =
	SOC_DAPM_ENUM("Stereo1 ADC2 Mux", rt5645_stereo1_adc2_enum);

/* MX-27 [8] */
static const char * const rt5645_stereo_dmic_src[] = {
	"DMIC1", "DMIC2"
};

static SOC_ENUM_SINGLE_DECL(
	rt5645_stereo1_dmic_enum, RT5645_STO1_ADC_MIXER,
	RT5645_DMIC_SRC_SFT, rt5645_stereo_dmic_src);

static const struct snd_kcontrol_new rt5645_sto1_dmic_mux =
	SOC_DAPM_ENUM("Stereo1 DMIC source", rt5645_stereo1_dmic_enum);

/* Mono ADC source */
/* MX-28 [12] */
static const char * const rt5645_mono_adc_l1_src[] = {
	"Mono DAC MIXL", "ADC"
};

static SOC_ENUM_SINGLE_DECL(
	rt5645_mono_adc_l1_enum, RT5645_MONO_ADC_MIXER,
	RT5645_MONO_ADC_L1_SRC_SFT, rt5645_mono_adc_l1_src);

static const struct snd_kcontrol_new rt5645_mono_adc_l1_mux =
	SOC_DAPM_ENUM("Mono ADC1 left source", rt5645_mono_adc_l1_enum);
/* MX-28 [11] */
static const char * const rt5645_mono_adc_l2_src[] = {
	"Mono DAC MIXL", "DMIC"
};

static SOC_ENUM_SINGLE_DECL(
	rt5645_mono_adc_l2_enum, RT5645_MONO_ADC_MIXER,
	RT5645_MONO_ADC_L2_SRC_SFT, rt5645_mono_adc_l2_src);

static const struct snd_kcontrol_new rt5645_mono_adc_l2_mux =
	SOC_DAPM_ENUM("Mono ADC2 left source", rt5645_mono_adc_l2_enum);

/* MX-28 [8] */
static const char * const rt5645_mono_dmic_src[] = {
	"DMIC1", "DMIC2"
};

static SOC_ENUM_SINGLE_DECL(
	rt5645_mono_dmic_l_enum, RT5645_MONO_ADC_MIXER,
	RT5645_MONO_DMIC_L_SRC_SFT, rt5645_mono_dmic_src);

static const struct snd_kcontrol_new rt5645_mono_dmic_l_mux =
	SOC_DAPM_ENUM("Mono DMIC left source", rt5645_mono_dmic_l_enum);
/* MX-28 [1:0] */
static SOC_ENUM_SINGLE_DECL(
	rt5645_mono_dmic_r_enum, RT5645_MONO_ADC_MIXER,
	RT5645_MONO_DMIC_R_SRC_SFT, rt5645_mono_dmic_src);

static const struct snd_kcontrol_new rt5645_mono_dmic_r_mux =
	SOC_DAPM_ENUM("Mono DMIC Right source", rt5645_mono_dmic_r_enum);
/* MX-28 [4] */
static const char * const rt5645_mono_adc_r1_src[] = {
	"Mono DAC MIXR", "ADC"
};

static SOC_ENUM_SINGLE_DECL(
	rt5645_mono_adc_r1_enum, RT5645_MONO_ADC_MIXER,
	RT5645_MONO_ADC_R1_SRC_SFT, rt5645_mono_adc_r1_src);

static const struct snd_kcontrol_new rt5645_mono_adc_r1_mux =
	SOC_DAPM_ENUM("Mono ADC1 right source", rt5645_mono_adc_r1_enum);
/* MX-28 [3] */
static const char * const rt5645_mono_adc_r2_src[] = {
	"Mono DAC MIXR", "DMIC"
};

static SOC_ENUM_SINGLE_DECL(
	rt5645_mono_adc_r2_enum, RT5645_MONO_ADC_MIXER,
	RT5645_MONO_ADC_R2_SRC_SFT, rt5645_mono_adc_r2_src);

static const struct snd_kcontrol_new rt5645_mono_adc_r2_mux =
	SOC_DAPM_ENUM("Mono ADC2 right source", rt5645_mono_adc_r2_enum);

/* MX-77 [9:8] */
static const char * const rt5645_if1_adc_in_src[] = {
	"IF_ADC1", "IF_ADC2", "VAD_ADC"
};

static SOC_ENUM_SINGLE_DECL(
	rt5645_if1_adc_in_enum, RT5645_TDM_CTRL_1,
	RT5645_IF1_ADC_IN_SFT, rt5645_if1_adc_in_src);

static const struct snd_kcontrol_new rt5645_if1_adc_in_mux =
	SOC_DAPM_ENUM("IF1 ADC IN source", rt5645_if1_adc_in_enum);

/* MX-2d [3] [2] */
static const char * const rt5650_a_dac1_src[] = {
	"DAC1", "Stereo DAC Mixer"
};

static SOC_ENUM_SINGLE_DECL(
	rt5650_a_dac1_l_enum, RT5650_A_DAC_SOUR,
	RT5650_A_DAC1_L_IN_SFT, rt5650_a_dac1_src);

static const struct snd_kcontrol_new rt5650_a_dac1_l_mux =
	SOC_DAPM_ENUM("A DAC1 L source", rt5650_a_dac1_l_enum);

static SOC_ENUM_SINGLE_DECL(
	rt5650_a_dac1_r_enum, RT5650_A_DAC_SOUR,
	RT5650_A_DAC1_R_IN_SFT, rt5650_a_dac1_src);

static const struct snd_kcontrol_new rt5650_a_dac1_r_mux =
	SOC_DAPM_ENUM("A DAC1 R source", rt5650_a_dac1_r_enum);

/* MX-2d [1] [0] */
static const char * const rt5650_a_dac2_src[] = {
	"Stereo DAC Mixer", "Mono DAC Mixer"
};

static SOC_ENUM_SINGLE_DECL(
	rt5650_a_dac2_l_enum, RT5650_A_DAC_SOUR,
	RT5650_A_DAC2_L_IN_SFT, rt5650_a_dac2_src);

static const struct snd_kcontrol_new rt5650_a_dac2_l_mux =
	SOC_DAPM_ENUM("A DAC2 L source", rt5650_a_dac2_l_enum);

static SOC_ENUM_SINGLE_DECL(
	rt5650_a_dac2_r_enum, RT5650_A_DAC_SOUR,
	RT5650_A_DAC2_R_IN_SFT, rt5650_a_dac2_src);

static const struct snd_kcontrol_new rt5650_a_dac2_r_mux =
	SOC_DAPM_ENUM("A DAC2 R source", rt5650_a_dac2_r_enum);

/* MX-2F [13:12] */
static const char * const rt5645_if2_adc_in_src[] = {
	"IF_ADC1", "IF_ADC2", "VAD_ADC"
};

static SOC_ENUM_SINGLE_DECL(
	rt5645_if2_adc_in_enum, RT5645_DIG_INF1_DATA,
	RT5645_IF2_ADC_IN_SFT, rt5645_if2_adc_in_src);

static const struct snd_kcontrol_new rt5645_if2_adc_in_mux =
	SOC_DAPM_ENUM("IF2 ADC IN source", rt5645_if2_adc_in_enum);

/* MX-2F [1:0] */
static const char * const rt5645_if3_adc_in_src[] = {
	"IF_ADC1", "IF_ADC2", "VAD_ADC"
};

static SOC_ENUM_SINGLE_DECL(
	rt5645_if3_adc_in_enum, RT5645_DIG_INF1_DATA,
	RT5645_IF3_ADC_IN_SFT, rt5645_if3_adc_in_src);

static const struct snd_kcontrol_new rt5645_if3_adc_in_mux =
	SOC_DAPM_ENUM("IF3 ADC IN source", rt5645_if3_adc_in_enum);

/* MX-31 [15] [13] [11] [9] */
static const char * const rt5645_pdm_src[] = {
	"Mono DAC", "Stereo DAC"
};

static SOC_ENUM_SINGLE_DECL(
	rt5645_pdm1_l_enum, RT5645_PDM_OUT_CTRL,
	RT5645_PDM1_L_SFT, rt5645_pdm_src);

static const struct snd_kcontrol_new rt5645_pdm1_l_mux =
	SOC_DAPM_ENUM("PDM1 L source", rt5645_pdm1_l_enum);

static SOC_ENUM_SINGLE_DECL(
	rt5645_pdm1_r_enum, RT5645_PDM_OUT_CTRL,
	RT5645_PDM1_R_SFT, rt5645_pdm_src);

static const struct snd_kcontrol_new rt5645_pdm1_r_mux =
	SOC_DAPM_ENUM("PDM1 R source", rt5645_pdm1_r_enum);

/* MX-9D [9:8] */
static const char * const rt5645_vad_adc_src[] = {
	"Sto1 ADC L", "Mono ADC L", "Mono ADC R"
};

static SOC_ENUM_SINGLE_DECL(
	rt5645_vad_adc_enum, RT5645_VAD_CTRL4,
	RT5645_VAD_SEL_SFT, rt5645_vad_adc_src);

static const struct snd_kcontrol_new rt5645_vad_adc_mux =
	SOC_DAPM_ENUM("VAD ADC source", rt5645_vad_adc_enum);

static const struct snd_kcontrol_new spk_l_vol_control =
	SOC_DAPM_SINGLE_AUTODISABLE("Switch", RT5645_SPK_VOL,
		RT5645_L_MUTE_SFT, 1, 1);

static const struct snd_kcontrol_new spk_r_vol_control =
	SOC_DAPM_SINGLE_AUTODISABLE("Switch", RT5645_SPK_VOL,
		RT5645_R_MUTE_SFT, 1, 1);

static const struct snd_kcontrol_new hp_l_vol_control =
	SOC_DAPM_SINGLE_AUTODISABLE("Switch", RT5645_HP_VOL,
		RT5645_L_MUTE_SFT, 1, 1);

static const struct snd_kcontrol_new hp_r_vol_control =
	SOC_DAPM_SINGLE_AUTODISABLE("Switch", RT5645_HP_VOL,
		RT5645_R_MUTE_SFT, 1, 1);

static const struct snd_kcontrol_new pdm1_l_vol_control =
	SOC_DAPM_SINGLE_AUTODISABLE("Switch", RT5645_PDM_OUT_CTRL,
		RT5645_M_PDM1_L, 1, 1);

static const struct snd_kcontrol_new pdm1_r_vol_control =
	SOC_DAPM_SINGLE_AUTODISABLE("Switch", RT5645_PDM_OUT_CTRL,
		RT5645_M_PDM1_R, 1, 1);

static void hp_amp_power(struct snd_soc_codec *codec, int on)
{
	static int hp_amp_power_count;
	struct rt5645_priv *rt5645 = snd_soc_codec_get_drvdata(codec);

	if (on) {
		if (hp_amp_power_count <= 0) {
			/* depop parameters */
			snd_soc_update_bits(codec, RT5645_DEPOP_M2,
				RT5645_DEPOP_MASK, RT5645_DEPOP_MAN);
			snd_soc_write(codec, RT5645_DEPOP_M1, 0x000d);
			regmap_write(rt5645->regmap, RT5645_PR_BASE +
				RT5645_HP_DCC_INT1, 0x9f01);
			mdelay(150);
			/* headphone amp power on */
			snd_soc_update_bits(codec, RT5645_PWR_ANLG1,
				RT5645_PWR_FV1 | RT5645_PWR_FV2 , 0);
			snd_soc_update_bits(codec, RT5645_PWR_VOL,
				RT5645_PWR_HV_L | RT5645_PWR_HV_R,
				RT5645_PWR_HV_L | RT5645_PWR_HV_R);
			snd_soc_update_bits(codec, RT5645_PWR_ANLG1,
				RT5645_PWR_HP_L | RT5645_PWR_HP_R |
				RT5645_PWR_HA,
				RT5645_PWR_HP_L | RT5645_PWR_HP_R |
				RT5645_PWR_HA);
			mdelay(5);
			snd_soc_update_bits(codec, RT5645_PWR_ANLG1,
				RT5645_PWR_FV1 | RT5645_PWR_FV2,
				RT5645_PWR_FV1 | RT5645_PWR_FV2);

			snd_soc_update_bits(codec, RT5645_DEPOP_M1,
				RT5645_HP_CO_MASK | RT5645_HP_SG_MASK,
				RT5645_HP_CO_EN | RT5645_HP_SG_EN);
			regmap_write(rt5645->regmap, RT5645_PR_BASE +
				0x14, 0x1aaa);
			regmap_write(rt5645->regmap, RT5645_PR_BASE +
				0x24, 0x0430);
		}
		hp_amp_power_count++;
	} else {
		hp_amp_power_count--;
		if (hp_amp_power_count <= 0) {
			snd_soc_update_bits(codec, RT5645_DEPOP_M1,
				RT5645_HP_SG_MASK | RT5645_HP_L_SMT_MASK |
				RT5645_HP_R_SMT_MASK, RT5645_HP_SG_DIS |
				RT5645_HP_L_SMT_DIS | RT5645_HP_R_SMT_DIS);
			/* headphone amp power down */
			snd_soc_write(codec, RT5645_DEPOP_M1, 0x0000);
			snd_soc_update_bits(codec, RT5645_PWR_ANLG1,
				RT5645_PWR_HP_L | RT5645_PWR_HP_R |
				RT5645_PWR_HA, 0);
			snd_soc_update_bits(codec, RT5645_DEPOP_M2,
				RT5645_DEPOP_MASK, 0);
		}
	}
}

static int rt5645_hp_event(struct snd_soc_dapm_widget *w,
	struct snd_kcontrol *kcontrol, int event)
{
	struct snd_soc_codec *codec = snd_soc_dapm_to_codec(w->dapm);
	struct rt5645_priv *rt5645 = snd_soc_codec_get_drvdata(codec);

	switch (event) {
	case SND_SOC_DAPM_POST_PMU:
		hp_amp_power(codec, 1);
		/* headphone unmute sequence */
		if (rt5645->codec_type == CODEC_TYPE_RT5650) {
			snd_soc_write(codec, RT5645_DEPOP_M3, 0x0737);
		} else {
			snd_soc_update_bits(codec, RT5645_DEPOP_M3,
				RT5645_CP_FQ1_MASK | RT5645_CP_FQ2_MASK |
				RT5645_CP_FQ3_MASK,
				(RT5645_CP_FQ_192_KHZ << RT5645_CP_FQ1_SFT) |
				(RT5645_CP_FQ_12_KHZ << RT5645_CP_FQ2_SFT) |
				(RT5645_CP_FQ_192_KHZ << RT5645_CP_FQ3_SFT));
		}
		regmap_write(rt5645->regmap,
			RT5645_PR_BASE + RT5645_MAMP_INT_REG2, 0xfc00);
		snd_soc_update_bits(codec, RT5645_DEPOP_M1,
			RT5645_SMT_TRIG_MASK, RT5645_SMT_TRIG_EN);
		snd_soc_update_bits(codec, RT5645_DEPOP_M1,
			RT5645_RSTN_MASK, RT5645_RSTN_EN);
		snd_soc_update_bits(codec, RT5645_DEPOP_M1,
			RT5645_RSTN_MASK | RT5645_HP_L_SMT_MASK |
			RT5645_HP_R_SMT_MASK, RT5645_RSTN_DIS |
			RT5645_HP_L_SMT_EN | RT5645_HP_R_SMT_EN);
		msleep(40);
		snd_soc_update_bits(codec, RT5645_DEPOP_M1,
			RT5645_HP_SG_MASK | RT5645_HP_L_SMT_MASK |
			RT5645_HP_R_SMT_MASK, RT5645_HP_SG_DIS |
			RT5645_HP_L_SMT_DIS | RT5645_HP_R_SMT_DIS);
		break;

	case SND_SOC_DAPM_PRE_PMD:
		/* headphone mute sequence */
		if (rt5645->codec_type == CODEC_TYPE_RT5650) {
			snd_soc_write(codec, RT5645_DEPOP_M3, 0x0737);
		} else {
			snd_soc_update_bits(codec, RT5645_DEPOP_M3,
				RT5645_CP_FQ1_MASK | RT5645_CP_FQ2_MASK |
				RT5645_CP_FQ3_MASK,
				(RT5645_CP_FQ_96_KHZ << RT5645_CP_FQ1_SFT) |
				(RT5645_CP_FQ_12_KHZ << RT5645_CP_FQ2_SFT) |
				(RT5645_CP_FQ_96_KHZ << RT5645_CP_FQ3_SFT));
		}
		regmap_write(rt5645->regmap,
			RT5645_PR_BASE + RT5645_MAMP_INT_REG2, 0xfc00);
		snd_soc_update_bits(codec, RT5645_DEPOP_M1,
			RT5645_HP_SG_MASK, RT5645_HP_SG_EN);
		snd_soc_update_bits(codec, RT5645_DEPOP_M1,
			RT5645_RSTP_MASK, RT5645_RSTP_EN);
		snd_soc_update_bits(codec, RT5645_DEPOP_M1,
			RT5645_RSTP_MASK | RT5645_HP_L_SMT_MASK |
			RT5645_HP_R_SMT_MASK, RT5645_RSTP_DIS |
			RT5645_HP_L_SMT_EN | RT5645_HP_R_SMT_EN);
		msleep(30);
		hp_amp_power(codec, 0);
		break;

	default:
		return 0;
	}

	return 0;
}

static int rt5645_spk_event(struct snd_soc_dapm_widget *w,
	struct snd_kcontrol *kcontrol, int event)
{
	struct snd_soc_codec *codec = snd_soc_dapm_to_codec(w->dapm);

	switch (event) {
	case SND_SOC_DAPM_POST_PMU:
		snd_soc_update_bits(codec, RT5645_PWR_DIG1,
			RT5645_PWR_CLS_D | RT5645_PWR_CLS_D_R |
			RT5645_PWR_CLS_D_L,
			RT5645_PWR_CLS_D | RT5645_PWR_CLS_D_R |
			RT5645_PWR_CLS_D_L);
		break;

	case SND_SOC_DAPM_PRE_PMD:
		snd_soc_update_bits(codec, RT5645_PWR_DIG1,
			RT5645_PWR_CLS_D | RT5645_PWR_CLS_D_R |
			RT5645_PWR_CLS_D_L, 0);
		break;

	default:
		return 0;
	}

	return 0;
}

static int rt5645_lout_event(struct snd_soc_dapm_widget *w,
	struct snd_kcontrol *kcontrol, int event)
{
	struct snd_soc_codec *codec = snd_soc_dapm_to_codec(w->dapm);

	switch (event) {
	case SND_SOC_DAPM_POST_PMU:
		hp_amp_power(codec, 1);
		snd_soc_update_bits(codec, RT5645_PWR_ANLG1,
			RT5645_PWR_LM, RT5645_PWR_LM);
		snd_soc_update_bits(codec, RT5645_LOUT1,
			RT5645_L_MUTE | RT5645_R_MUTE, 0);
		break;

	case SND_SOC_DAPM_PRE_PMD:
		snd_soc_update_bits(codec, RT5645_LOUT1,
			RT5645_L_MUTE | RT5645_R_MUTE,
			RT5645_L_MUTE | RT5645_R_MUTE);
		snd_soc_update_bits(codec, RT5645_PWR_ANLG1,
			RT5645_PWR_LM, 0);
		hp_amp_power(codec, 0);
		break;

	default:
		return 0;
	}

	return 0;
}

static int rt5645_bst2_event(struct snd_soc_dapm_widget *w,
	struct snd_kcontrol *kcontrol, int event)
{
	struct snd_soc_codec *codec = snd_soc_dapm_to_codec(w->dapm);

	switch (event) {
	case SND_SOC_DAPM_POST_PMU:
		snd_soc_update_bits(codec, RT5645_PWR_ANLG2,
			RT5645_PWR_BST2_P, RT5645_PWR_BST2_P);
		break;

	case SND_SOC_DAPM_PRE_PMD:
		snd_soc_update_bits(codec, RT5645_PWR_ANLG2,
			RT5645_PWR_BST2_P, 0);
		break;

	default:
		return 0;
	}

	return 0;
}

static const struct snd_soc_dapm_widget rt5645_dapm_widgets[] = {
	SND_SOC_DAPM_SUPPLY("LDO2", RT5645_PWR_MIXER,
		RT5645_PWR_LDO2_BIT, 0, NULL, 0),
	SND_SOC_DAPM_SUPPLY("PLL1", RT5645_PWR_ANLG2,
		RT5645_PWR_PLL_BIT, 0, NULL, 0),

	SND_SOC_DAPM_SUPPLY("JD Power", RT5645_PWR_ANLG2,
		RT5645_PWR_JD1_BIT, 0, NULL, 0),
	SND_SOC_DAPM_SUPPLY("Mic Det Power", RT5645_PWR_VOL,
		RT5645_PWR_MIC_DET_BIT, 0, NULL, 0),

	/* ASRC */
	SND_SOC_DAPM_SUPPLY_S("I2S1 ASRC", 1, RT5645_ASRC_1,
			      11, 0, NULL, 0),
	SND_SOC_DAPM_SUPPLY_S("I2S2 ASRC", 1, RT5645_ASRC_1,
			      12, 0, NULL, 0),
	SND_SOC_DAPM_SUPPLY_S("DAC STO ASRC", 1, RT5645_ASRC_1,
			      10, 0, NULL, 0),
	SND_SOC_DAPM_SUPPLY_S("DAC MONO L ASRC", 1, RT5645_ASRC_1,
			      9, 0, NULL, 0),
	SND_SOC_DAPM_SUPPLY_S("DAC MONO R ASRC", 1, RT5645_ASRC_1,
			      8, 0, NULL, 0),
	SND_SOC_DAPM_SUPPLY_S("DMIC STO1 ASRC", 1, RT5645_ASRC_1,
			      7, 0, NULL, 0),
	SND_SOC_DAPM_SUPPLY_S("DMIC MONO L ASRC", 1, RT5645_ASRC_1,
			      5, 0, NULL, 0),
	SND_SOC_DAPM_SUPPLY_S("DMIC MONO R ASRC", 1, RT5645_ASRC_1,
			      4, 0, NULL, 0),
	SND_SOC_DAPM_SUPPLY_S("ADC STO1 ASRC", 1, RT5645_ASRC_1,
			      3, 0, NULL, 0),
	SND_SOC_DAPM_SUPPLY_S("ADC MONO L ASRC", 1, RT5645_ASRC_1,
			      1, 0, NULL, 0),
	SND_SOC_DAPM_SUPPLY_S("ADC MONO R ASRC", 1, RT5645_ASRC_1,
			      0, 0, NULL, 0),

	/* Input Side */
	/* micbias */
	SND_SOC_DAPM_MICBIAS("micbias1", RT5645_PWR_ANLG2,
			RT5645_PWR_MB1_BIT, 0),
	SND_SOC_DAPM_MICBIAS("micbias2", RT5645_PWR_ANLG2,
			RT5645_PWR_MB2_BIT, 0),
	/* Input Lines */
	SND_SOC_DAPM_INPUT("DMIC L1"),
	SND_SOC_DAPM_INPUT("DMIC R1"),
	SND_SOC_DAPM_INPUT("DMIC L2"),
	SND_SOC_DAPM_INPUT("DMIC R2"),

	SND_SOC_DAPM_INPUT("IN1P"),
	SND_SOC_DAPM_INPUT("IN1N"),
	SND_SOC_DAPM_INPUT("IN2P"),
	SND_SOC_DAPM_INPUT("IN2N"),

	SND_SOC_DAPM_INPUT("Haptic Generator"),

	SND_SOC_DAPM_PGA("DMIC1", SND_SOC_NOPM, 0, 0, NULL, 0),
	SND_SOC_DAPM_PGA("DMIC2", SND_SOC_NOPM, 0, 0, NULL, 0),
	SND_SOC_DAPM_SUPPLY("DMIC CLK", SND_SOC_NOPM, 0, 0,
		set_dmic_clk, SND_SOC_DAPM_PRE_PMU),
	SND_SOC_DAPM_SUPPLY("DMIC1 Power", RT5645_DMIC_CTRL1,
		RT5645_DMIC_1_EN_SFT, 0, NULL, 0),
	SND_SOC_DAPM_SUPPLY("DMIC2 Power", RT5645_DMIC_CTRL1,
		RT5645_DMIC_2_EN_SFT, 0, NULL, 0),
	/* Boost */
	SND_SOC_DAPM_PGA("BST1", RT5645_PWR_ANLG2,
		RT5645_PWR_BST1_BIT, 0, NULL, 0),
	SND_SOC_DAPM_PGA_E("BST2", RT5645_PWR_ANLG2,
		RT5645_PWR_BST2_BIT, 0, NULL, 0, rt5645_bst2_event,
		SND_SOC_DAPM_PRE_PMD | SND_SOC_DAPM_POST_PMU),
	/* Input Volume */
	SND_SOC_DAPM_PGA("INL VOL", RT5645_PWR_VOL,
		RT5645_PWR_IN_L_BIT, 0, NULL, 0),
	SND_SOC_DAPM_PGA("INR VOL", RT5645_PWR_VOL,
		RT5645_PWR_IN_R_BIT, 0, NULL, 0),
	/* REC Mixer */
	SND_SOC_DAPM_MIXER("RECMIXL", RT5645_PWR_MIXER, RT5645_PWR_RM_L_BIT,
			0, rt5645_rec_l_mix, ARRAY_SIZE(rt5645_rec_l_mix)),
	SND_SOC_DAPM_MIXER("RECMIXR", RT5645_PWR_MIXER, RT5645_PWR_RM_R_BIT,
			0, rt5645_rec_r_mix, ARRAY_SIZE(rt5645_rec_r_mix)),
	/* ADCs */
	SND_SOC_DAPM_ADC("ADC L", NULL, SND_SOC_NOPM, 0, 0),
	SND_SOC_DAPM_ADC("ADC R", NULL, SND_SOC_NOPM, 0, 0),

	SND_SOC_DAPM_SUPPLY("ADC L power", RT5645_PWR_DIG1,
		RT5645_PWR_ADC_L_BIT, 0, NULL, 0),
	SND_SOC_DAPM_SUPPLY("ADC R power", RT5645_PWR_DIG1,
		RT5645_PWR_ADC_R_BIT, 0, NULL, 0),

	/* ADC Mux */
	SND_SOC_DAPM_MUX("Stereo1 DMIC Mux", SND_SOC_NOPM, 0, 0,
		&rt5645_sto1_dmic_mux),
	SND_SOC_DAPM_MUX("Stereo1 ADC L2 Mux", SND_SOC_NOPM, 0, 0,
		&rt5645_sto_adc2_mux),
	SND_SOC_DAPM_MUX("Stereo1 ADC R2 Mux", SND_SOC_NOPM, 0, 0,
		&rt5645_sto_adc2_mux),
	SND_SOC_DAPM_MUX("Stereo1 ADC L1 Mux", SND_SOC_NOPM, 0, 0,
		&rt5645_sto_adc1_mux),
	SND_SOC_DAPM_MUX("Stereo1 ADC R1 Mux", SND_SOC_NOPM, 0, 0,
		&rt5645_sto_adc1_mux),
	SND_SOC_DAPM_MUX("Mono DMIC L Mux", SND_SOC_NOPM, 0, 0,
		&rt5645_mono_dmic_l_mux),
	SND_SOC_DAPM_MUX("Mono DMIC R Mux", SND_SOC_NOPM, 0, 0,
		&rt5645_mono_dmic_r_mux),
	SND_SOC_DAPM_MUX("Mono ADC L2 Mux", SND_SOC_NOPM, 0, 0,
		&rt5645_mono_adc_l2_mux),
	SND_SOC_DAPM_MUX("Mono ADC L1 Mux", SND_SOC_NOPM, 0, 0,
		&rt5645_mono_adc_l1_mux),
	SND_SOC_DAPM_MUX("Mono ADC R1 Mux", SND_SOC_NOPM, 0, 0,
		&rt5645_mono_adc_r1_mux),
	SND_SOC_DAPM_MUX("Mono ADC R2 Mux", SND_SOC_NOPM, 0, 0,
		&rt5645_mono_adc_r2_mux),
	/* ADC Mixer */

	SND_SOC_DAPM_SUPPLY_S("adc stereo1 filter", 1, RT5645_PWR_DIG2,
		RT5645_PWR_ADC_S1F_BIT, 0, NULL, 0),
	SND_SOC_DAPM_MIXER_E("Sto1 ADC MIXL", SND_SOC_NOPM, 0, 0,
		rt5645_sto1_adc_l_mix, ARRAY_SIZE(rt5645_sto1_adc_l_mix),
		NULL, 0),
	SND_SOC_DAPM_MIXER_E("Sto1 ADC MIXR", SND_SOC_NOPM, 0, 0,
		rt5645_sto1_adc_r_mix, ARRAY_SIZE(rt5645_sto1_adc_r_mix),
		NULL, 0),
	SND_SOC_DAPM_SUPPLY_S("adc mono left filter", 1, RT5645_PWR_DIG2,
		RT5645_PWR_ADC_MF_L_BIT, 0, NULL, 0),
	SND_SOC_DAPM_MIXER_E("Mono ADC MIXL", SND_SOC_NOPM, 0, 0,
		rt5645_mono_adc_l_mix, ARRAY_SIZE(rt5645_mono_adc_l_mix),
		NULL, 0),
	SND_SOC_DAPM_SUPPLY_S("adc mono right filter", 1, RT5645_PWR_DIG2,
		RT5645_PWR_ADC_MF_R_BIT, 0, NULL, 0),
	SND_SOC_DAPM_MIXER_E("Mono ADC MIXR", SND_SOC_NOPM, 0, 0,
		rt5645_mono_adc_r_mix, ARRAY_SIZE(rt5645_mono_adc_r_mix),
		NULL, 0),

	/* ADC PGA */
	SND_SOC_DAPM_PGA("Stereo1 ADC MIXL", SND_SOC_NOPM, 0, 0, NULL, 0),
	SND_SOC_DAPM_PGA("Stereo1 ADC MIXR", SND_SOC_NOPM, 0, 0, NULL, 0),
	SND_SOC_DAPM_PGA("Sto2 ADC LR MIX", SND_SOC_NOPM, 0, 0, NULL, 0),
	SND_SOC_DAPM_PGA("VAD_ADC", SND_SOC_NOPM, 0, 0, NULL, 0),
	SND_SOC_DAPM_PGA("IF_ADC1", SND_SOC_NOPM, 0, 0, NULL, 0),
	SND_SOC_DAPM_PGA("IF_ADC2", SND_SOC_NOPM, 0, 0, NULL, 0),
	SND_SOC_DAPM_PGA("IF1_ADC1", SND_SOC_NOPM, 0, 0, NULL, 0),
	SND_SOC_DAPM_PGA("IF1_ADC2", SND_SOC_NOPM, 0, 0, NULL, 0),
	SND_SOC_DAPM_PGA("IF1_ADC3", SND_SOC_NOPM, 0, 0, NULL, 0),
	SND_SOC_DAPM_PGA("IF1_ADC4", SND_SOC_NOPM, 0, 0, NULL, 0),

	/* IF1 2 Mux */
	SND_SOC_DAPM_MUX("IF1 ADC Mux", SND_SOC_NOPM,
		0, 0, &rt5645_if1_adc_in_mux),
	SND_SOC_DAPM_MUX("IF2 ADC Mux", SND_SOC_NOPM,
		0, 0, &rt5645_if2_adc_in_mux),

	/* Digital Interface */
	SND_SOC_DAPM_SUPPLY("I2S1", RT5645_PWR_DIG1,
		RT5645_PWR_I2S1_BIT, 0, NULL, 0),
	SND_SOC_DAPM_PGA("IF1 DAC1", SND_SOC_NOPM, 0, 0, NULL, 0),
	SND_SOC_DAPM_PGA("IF1 DAC2", SND_SOC_NOPM, 0, 0, NULL, 0),
	SND_SOC_DAPM_PGA("IF1 DAC1 L", SND_SOC_NOPM, 0, 0, NULL, 0),
	SND_SOC_DAPM_PGA("IF1 DAC1 R", SND_SOC_NOPM, 0, 0, NULL, 0),
	SND_SOC_DAPM_PGA("IF1 DAC2 L", SND_SOC_NOPM, 0, 0, NULL, 0),
	SND_SOC_DAPM_PGA("IF1 DAC2 R", SND_SOC_NOPM, 0, 0, NULL, 0),
	SND_SOC_DAPM_PGA("IF1 ADC", SND_SOC_NOPM, 0, 0, NULL, 0),
	SND_SOC_DAPM_PGA("IF1 ADC L", SND_SOC_NOPM, 0, 0, NULL, 0),
	SND_SOC_DAPM_PGA("IF1 ADC R", SND_SOC_NOPM, 0, 0, NULL, 0),
	SND_SOC_DAPM_SUPPLY("I2S2", RT5645_PWR_DIG1,
		RT5645_PWR_I2S2_BIT, 0, NULL, 0),
	SND_SOC_DAPM_PGA("IF2 DAC", SND_SOC_NOPM, 0, 0, NULL, 0),
	SND_SOC_DAPM_PGA("IF2 DAC L", SND_SOC_NOPM, 0, 0, NULL, 0),
	SND_SOC_DAPM_PGA("IF2 DAC R", SND_SOC_NOPM, 0, 0, NULL, 0),
	SND_SOC_DAPM_PGA("IF2 ADC", SND_SOC_NOPM, 0, 0, NULL, 0),

	/* Digital Interface Select */
	SND_SOC_DAPM_MUX("VAD ADC Mux", SND_SOC_NOPM,
		0, 0, &rt5645_vad_adc_mux),

	/* Audio Interface */
	SND_SOC_DAPM_AIF_IN("AIF1RX", "AIF1 Playback", 0, SND_SOC_NOPM, 0, 0),
	SND_SOC_DAPM_AIF_OUT("AIF1TX", "AIF1 Capture", 0, SND_SOC_NOPM, 0, 0),
	SND_SOC_DAPM_AIF_IN("AIF2RX", "AIF2 Playback", 0, SND_SOC_NOPM, 0, 0),
	SND_SOC_DAPM_AIF_OUT("AIF2TX", "AIF2 Capture", 0, SND_SOC_NOPM, 0, 0),

	/* Output Side */
	/* DAC mixer before sound effect  */
	SND_SOC_DAPM_MIXER("DAC1 MIXL", SND_SOC_NOPM, 0, 0,
		rt5645_dac_l_mix, ARRAY_SIZE(rt5645_dac_l_mix)),
	SND_SOC_DAPM_MIXER("DAC1 MIXR", SND_SOC_NOPM, 0, 0,
		rt5645_dac_r_mix, ARRAY_SIZE(rt5645_dac_r_mix)),

	/* DAC2 channel Mux */
	SND_SOC_DAPM_MUX("DAC L2 Mux", SND_SOC_NOPM, 0, 0, &rt5645_dac_l2_mux),
	SND_SOC_DAPM_MUX("DAC R2 Mux", SND_SOC_NOPM, 0, 0, &rt5645_dac_r2_mux),
	SND_SOC_DAPM_PGA("DAC L2 Volume", RT5645_PWR_DIG1,
		RT5645_PWR_DAC_L2_BIT, 0, NULL, 0),
	SND_SOC_DAPM_PGA("DAC R2 Volume", RT5645_PWR_DIG1,
		RT5645_PWR_DAC_R2_BIT, 0, NULL, 0),

	SND_SOC_DAPM_MUX("DAC1 L Mux", SND_SOC_NOPM, 0, 0, &rt5645_dac1l_mux),
	SND_SOC_DAPM_MUX("DAC1 R Mux", SND_SOC_NOPM, 0, 0, &rt5645_dac1r_mux),

	/* DAC Mixer */
	SND_SOC_DAPM_SUPPLY_S("dac stereo1 filter", 1, RT5645_PWR_DIG2,
		RT5645_PWR_DAC_S1F_BIT, 0, NULL, 0),
	SND_SOC_DAPM_SUPPLY_S("dac mono left filter", 1, RT5645_PWR_DIG2,
		RT5645_PWR_DAC_MF_L_BIT, 0, NULL, 0),
	SND_SOC_DAPM_SUPPLY_S("dac mono right filter", 1, RT5645_PWR_DIG2,
		RT5645_PWR_DAC_MF_R_BIT, 0, NULL, 0),
	SND_SOC_DAPM_MIXER("Stereo DAC MIXL", SND_SOC_NOPM, 0, 0,
		rt5645_sto_dac_l_mix, ARRAY_SIZE(rt5645_sto_dac_l_mix)),
	SND_SOC_DAPM_MIXER("Stereo DAC MIXR", SND_SOC_NOPM, 0, 0,
		rt5645_sto_dac_r_mix, ARRAY_SIZE(rt5645_sto_dac_r_mix)),
	SND_SOC_DAPM_MIXER("Mono DAC MIXL", SND_SOC_NOPM, 0, 0,
		rt5645_mono_dac_l_mix, ARRAY_SIZE(rt5645_mono_dac_l_mix)),
	SND_SOC_DAPM_MIXER("Mono DAC MIXR", SND_SOC_NOPM, 0, 0,
		rt5645_mono_dac_r_mix, ARRAY_SIZE(rt5645_mono_dac_r_mix)),
	SND_SOC_DAPM_MIXER("DAC MIXL", SND_SOC_NOPM, 0, 0,
		rt5645_dig_l_mix, ARRAY_SIZE(rt5645_dig_l_mix)),
	SND_SOC_DAPM_MIXER("DAC MIXR", SND_SOC_NOPM, 0, 0,
		rt5645_dig_r_mix, ARRAY_SIZE(rt5645_dig_r_mix)),

	/* DACs */
	SND_SOC_DAPM_DAC("DAC L1", NULL, RT5645_PWR_DIG1, RT5645_PWR_DAC_L1_BIT,
		0),
	SND_SOC_DAPM_DAC("DAC L2", NULL, RT5645_PWR_DIG1, RT5645_PWR_DAC_L2_BIT,
		0),
	SND_SOC_DAPM_DAC("DAC R1", NULL, RT5645_PWR_DIG1, RT5645_PWR_DAC_R1_BIT,
		0),
	SND_SOC_DAPM_DAC("DAC R2", NULL, RT5645_PWR_DIG1, RT5645_PWR_DAC_R2_BIT,
		0),
	/* OUT Mixer */
	SND_SOC_DAPM_MIXER("SPK MIXL", RT5645_PWR_MIXER, RT5645_PWR_SM_L_BIT,
		0, rt5645_spk_l_mix, ARRAY_SIZE(rt5645_spk_l_mix)),
	SND_SOC_DAPM_MIXER("SPK MIXR", RT5645_PWR_MIXER, RT5645_PWR_SM_R_BIT,
		0, rt5645_spk_r_mix, ARRAY_SIZE(rt5645_spk_r_mix)),
	SND_SOC_DAPM_MIXER("OUT MIXL", RT5645_PWR_MIXER, RT5645_PWR_OM_L_BIT,
		0, rt5645_out_l_mix, ARRAY_SIZE(rt5645_out_l_mix)),
	SND_SOC_DAPM_MIXER("OUT MIXR", RT5645_PWR_MIXER, RT5645_PWR_OM_R_BIT,
		0, rt5645_out_r_mix, ARRAY_SIZE(rt5645_out_r_mix)),
	/* Ouput Volume */
	SND_SOC_DAPM_SWITCH("SPKVOL L", RT5645_PWR_VOL, RT5645_PWR_SV_L_BIT, 0,
		&spk_l_vol_control),
	SND_SOC_DAPM_SWITCH("SPKVOL R", RT5645_PWR_VOL, RT5645_PWR_SV_R_BIT, 0,
		&spk_r_vol_control),
	SND_SOC_DAPM_MIXER("HPOVOL MIXL", RT5645_PWR_VOL, RT5645_PWR_HV_L_BIT,
		0, rt5645_hpvoll_mix, ARRAY_SIZE(rt5645_hpvoll_mix)),
	SND_SOC_DAPM_MIXER("HPOVOL MIXR", RT5645_PWR_VOL, RT5645_PWR_HV_R_BIT,
		0, rt5645_hpvolr_mix, ARRAY_SIZE(rt5645_hpvolr_mix)),
	SND_SOC_DAPM_SUPPLY("HPOVOL MIXL Power", RT5645_PWR_MIXER,
		RT5645_PWR_HM_L_BIT, 0, NULL, 0),
	SND_SOC_DAPM_SUPPLY("HPOVOL MIXR Power", RT5645_PWR_MIXER,
		RT5645_PWR_HM_R_BIT, 0, NULL, 0),
	SND_SOC_DAPM_PGA("DAC 1", SND_SOC_NOPM, 0, 0, NULL, 0),
	SND_SOC_DAPM_PGA("DAC 2", SND_SOC_NOPM, 0, 0, NULL, 0),
	SND_SOC_DAPM_PGA("HPOVOL", SND_SOC_NOPM, 0, 0, NULL, 0),
	SND_SOC_DAPM_SWITCH("HPOVOL L", SND_SOC_NOPM, 0, 0, &hp_l_vol_control),
	SND_SOC_DAPM_SWITCH("HPOVOL R", SND_SOC_NOPM, 0, 0, &hp_r_vol_control),

	/* HPO/LOUT/Mono Mixer */
	SND_SOC_DAPM_MIXER("SPOL MIX", SND_SOC_NOPM, 0, 0, rt5645_spo_l_mix,
		ARRAY_SIZE(rt5645_spo_l_mix)),
	SND_SOC_DAPM_MIXER("SPOR MIX", SND_SOC_NOPM, 0, 0, rt5645_spo_r_mix,
		ARRAY_SIZE(rt5645_spo_r_mix)),
	SND_SOC_DAPM_MIXER("HPO MIX", SND_SOC_NOPM, 0, 0, rt5645_hpo_mix,
		ARRAY_SIZE(rt5645_hpo_mix)),
	SND_SOC_DAPM_MIXER("LOUT MIX", SND_SOC_NOPM, 0, 0, rt5645_lout_mix,
		ARRAY_SIZE(rt5645_lout_mix)),

	SND_SOC_DAPM_PGA_S("HP amp", 1, SND_SOC_NOPM, 0, 0, rt5645_hp_event,
		SND_SOC_DAPM_PRE_PMD | SND_SOC_DAPM_POST_PMU),
	SND_SOC_DAPM_PGA_S("LOUT amp", 1, SND_SOC_NOPM, 0, 0, rt5645_lout_event,
		SND_SOC_DAPM_PRE_PMD | SND_SOC_DAPM_POST_PMU),
	SND_SOC_DAPM_PGA_S("SPK amp", 2, SND_SOC_NOPM, 0, 0, rt5645_spk_event,
		SND_SOC_DAPM_PRE_PMD | SND_SOC_DAPM_POST_PMU),

	/* PDM */
	SND_SOC_DAPM_SUPPLY("PDM1 Power", RT5645_PWR_DIG2, RT5645_PWR_PDM1_BIT,
		0, NULL, 0),
	SND_SOC_DAPM_MUX("PDM1 L Mux", SND_SOC_NOPM, 0, 0, &rt5645_pdm1_l_mux),
	SND_SOC_DAPM_MUX("PDM1 R Mux", SND_SOC_NOPM, 0, 0, &rt5645_pdm1_r_mux),

	SND_SOC_DAPM_SWITCH("PDM1 L", SND_SOC_NOPM, 0, 0, &pdm1_l_vol_control),
	SND_SOC_DAPM_SWITCH("PDM1 R", SND_SOC_NOPM, 0, 0, &pdm1_r_vol_control),

	/* Output Lines */
	SND_SOC_DAPM_OUTPUT("HPOL"),
	SND_SOC_DAPM_OUTPUT("HPOR"),
	SND_SOC_DAPM_OUTPUT("LOUTL"),
	SND_SOC_DAPM_OUTPUT("LOUTR"),
	SND_SOC_DAPM_OUTPUT("PDM1L"),
	SND_SOC_DAPM_OUTPUT("PDM1R"),
	SND_SOC_DAPM_OUTPUT("SPOL"),
	SND_SOC_DAPM_OUTPUT("SPOR"),
};

static const struct snd_soc_dapm_widget rt5650_specific_dapm_widgets[] = {
	SND_SOC_DAPM_MUX("A DAC1 L Mux", SND_SOC_NOPM,
		0, 0, &rt5650_a_dac1_l_mux),
	SND_SOC_DAPM_MUX("A DAC1 R Mux", SND_SOC_NOPM,
		0, 0, &rt5650_a_dac1_r_mux),
	SND_SOC_DAPM_MUX("A DAC2 L Mux", SND_SOC_NOPM,
		0, 0, &rt5650_a_dac2_l_mux),
	SND_SOC_DAPM_MUX("A DAC2 R Mux", SND_SOC_NOPM,
		0, 0, &rt5650_a_dac2_r_mux),
};

static const struct snd_soc_dapm_route rt5645_dapm_routes[] = {
	{ "adc stereo1 filter", NULL, "ADC STO1 ASRC", is_using_asrc },
	{ "adc mono left filter", NULL, "ADC MONO L ASRC", is_using_asrc },
	{ "adc mono right filter", NULL, "ADC MONO R ASRC", is_using_asrc },
	{ "dac mono left filter", NULL, "DAC MONO L ASRC", is_using_asrc },
	{ "dac mono right filter", NULL, "DAC MONO R ASRC", is_using_asrc },
	{ "dac stereo1 filter", NULL, "DAC STO ASRC", is_using_asrc },

	{ "I2S1", NULL, "I2S1 ASRC" },
	{ "I2S2", NULL, "I2S2 ASRC" },

	{ "IN1P", NULL, "LDO2" },
	{ "IN2P", NULL, "LDO2" },

	{ "DMIC1", NULL, "DMIC L1" },
	{ "DMIC1", NULL, "DMIC R1" },
	{ "DMIC2", NULL, "DMIC L2" },
	{ "DMIC2", NULL, "DMIC R2" },

	{ "BST1", NULL, "IN1P" },
	{ "BST1", NULL, "IN1N" },
	{ "BST1", NULL, "JD Power" },
	{ "BST1", NULL, "Mic Det Power" },
	{ "BST2", NULL, "IN2P" },
	{ "BST2", NULL, "IN2N" },

	{ "INL VOL", NULL, "IN2P" },
	{ "INR VOL", NULL, "IN2N" },

	{ "RECMIXL", "HPOL Switch", "HPOL" },
	{ "RECMIXL", "INL Switch", "INL VOL" },
	{ "RECMIXL", "BST2 Switch", "BST2" },
	{ "RECMIXL", "BST1 Switch", "BST1" },
	{ "RECMIXL", "OUT MIXL Switch", "OUT MIXL" },

	{ "RECMIXR", "HPOR Switch", "HPOR" },
	{ "RECMIXR", "INR Switch", "INR VOL" },
	{ "RECMIXR", "BST2 Switch", "BST2" },
	{ "RECMIXR", "BST1 Switch", "BST1" },
	{ "RECMIXR", "OUT MIXR Switch", "OUT MIXR" },

	{ "ADC L", NULL, "RECMIXL" },
	{ "ADC L", NULL, "ADC L power" },
	{ "ADC R", NULL, "RECMIXR" },
	{ "ADC R", NULL, "ADC R power" },

	{"DMIC L1", NULL, "DMIC CLK"},
	{"DMIC L1", NULL, "DMIC1 Power"},
	{"DMIC R1", NULL, "DMIC CLK"},
	{"DMIC R1", NULL, "DMIC1 Power"},
	{"DMIC L2", NULL, "DMIC CLK"},
	{"DMIC L2", NULL, "DMIC2 Power"},
	{"DMIC R2", NULL, "DMIC CLK"},
	{"DMIC R2", NULL, "DMIC2 Power"},

	{ "Stereo1 DMIC Mux", "DMIC1", "DMIC1" },
	{ "Stereo1 DMIC Mux", "DMIC2", "DMIC2" },
	{ "Stereo1 DMIC Mux", NULL, "DMIC STO1 ASRC" },

	{ "Mono DMIC L Mux", "DMIC1", "DMIC L1" },
	{ "Mono DMIC L Mux", "DMIC2", "DMIC L2" },
	{ "Mono DMIC L Mux", NULL, "DMIC MONO L ASRC" },

	{ "Mono DMIC R Mux", "DMIC1", "DMIC R1" },
	{ "Mono DMIC R Mux", "DMIC2", "DMIC R2" },
	{ "Mono DMIC R Mux", NULL, "DMIC MONO R ASRC" },

	{ "Stereo1 ADC L2 Mux", "DMIC", "Stereo1 DMIC Mux" },
	{ "Stereo1 ADC L2 Mux", "DAC MIX", "DAC MIXL" },
	{ "Stereo1 ADC L1 Mux", "ADC", "ADC L" },
	{ "Stereo1 ADC L1 Mux", "DAC MIX", "DAC MIXL" },

	{ "Stereo1 ADC R1 Mux", "ADC", "ADC R" },
	{ "Stereo1 ADC R1 Mux", "DAC MIX", "DAC MIXR" },
	{ "Stereo1 ADC R2 Mux", "DMIC", "Stereo1 DMIC Mux" },
	{ "Stereo1 ADC R2 Mux", "DAC MIX", "DAC MIXR" },

	{ "Mono ADC L2 Mux", "DMIC", "Mono DMIC L Mux" },
	{ "Mono ADC L2 Mux", "Mono DAC MIXL", "Mono DAC MIXL" },
	{ "Mono ADC L1 Mux", "Mono DAC MIXL", "Mono DAC MIXL" },
	{ "Mono ADC L1 Mux", "ADC", "ADC L" },

	{ "Mono ADC R1 Mux", "Mono DAC MIXR", "Mono DAC MIXR" },
	{ "Mono ADC R1 Mux", "ADC", "ADC R" },
	{ "Mono ADC R2 Mux", "DMIC", "Mono DMIC R Mux" },
	{ "Mono ADC R2 Mux", "Mono DAC MIXR", "Mono DAC MIXR" },

	{ "Sto1 ADC MIXL", "ADC1 Switch", "Stereo1 ADC L1 Mux" },
	{ "Sto1 ADC MIXL", "ADC2 Switch", "Stereo1 ADC L2 Mux" },
	{ "Sto1 ADC MIXR", "ADC1 Switch", "Stereo1 ADC R1 Mux" },
	{ "Sto1 ADC MIXR", "ADC2 Switch", "Stereo1 ADC R2 Mux" },

	{ "Stereo1 ADC MIXL", NULL, "Sto1 ADC MIXL" },
	{ "Stereo1 ADC MIXL", NULL, "adc stereo1 filter" },
	{ "adc stereo1 filter", NULL, "PLL1", is_sys_clk_from_pll },

	{ "Stereo1 ADC MIXR", NULL, "Sto1 ADC MIXR" },
	{ "Stereo1 ADC MIXR", NULL, "adc stereo1 filter" },
	{ "adc stereo1 filter", NULL, "PLL1", is_sys_clk_from_pll },

	{ "Mono ADC MIXL", "ADC1 Switch", "Mono ADC L1 Mux" },
	{ "Mono ADC MIXL", "ADC2 Switch", "Mono ADC L2 Mux" },
	{ "Mono ADC MIXL", NULL, "adc mono left filter" },
	{ "adc mono left filter", NULL, "PLL1", is_sys_clk_from_pll },

	{ "Mono ADC MIXR", "ADC1 Switch", "Mono ADC R1 Mux" },
	{ "Mono ADC MIXR", "ADC2 Switch", "Mono ADC R2 Mux" },
	{ "Mono ADC MIXR", NULL, "adc mono right filter" },
	{ "adc mono right filter", NULL, "PLL1", is_sys_clk_from_pll },

	{ "VAD ADC Mux", "Sto1 ADC L", "Stereo1 ADC MIXL" },
	{ "VAD ADC Mux", "Mono ADC L", "Mono ADC MIXL" },
	{ "VAD ADC Mux", "Mono ADC R", "Mono ADC MIXR" },

	{ "IF_ADC1", NULL, "Stereo1 ADC MIXL" },
	{ "IF_ADC1", NULL, "Stereo1 ADC MIXR" },
	{ "IF_ADC2", NULL, "Mono ADC MIXL" },
	{ "IF_ADC2", NULL, "Mono ADC MIXR" },
	{ "VAD_ADC", NULL, "VAD ADC Mux" },

	{ "IF1 ADC Mux", "IF_ADC1", "IF_ADC1" },
	{ "IF1 ADC Mux", "IF_ADC2", "IF_ADC2" },
	{ "IF1 ADC Mux", "VAD_ADC", "VAD_ADC" },

	{ "IF2 ADC Mux", "IF_ADC1", "IF_ADC1" },
	{ "IF2 ADC Mux", "IF_ADC2", "IF_ADC2" },
	{ "IF2 ADC Mux", "VAD_ADC", "VAD_ADC" },

	{ "IF1 ADC", NULL, "I2S1" },
	{ "IF1 ADC", NULL, "IF1 ADC Mux" },
	{ "IF2 ADC", NULL, "I2S2" },
	{ "IF2 ADC", NULL, "IF2 ADC Mux" },

	{ "AIF1TX", NULL, "IF1 ADC" },
	{ "AIF1TX", NULL, "IF2 ADC" },
	{ "AIF2TX", NULL, "IF2 ADC" },

	{ "IF1 DAC1", NULL, "AIF1RX" },
	{ "IF1 DAC2", NULL, "AIF1RX" },
	{ "IF2 DAC", NULL, "AIF2RX" },

	{ "IF1 DAC1", NULL, "I2S1" },
	{ "IF1 DAC2", NULL, "I2S1" },
	{ "IF2 DAC", NULL, "I2S2" },

	{ "IF1 DAC2 L", NULL, "IF1 DAC2" },
	{ "IF1 DAC2 R", NULL, "IF1 DAC2" },
	{ "IF1 DAC1 L", NULL, "IF1 DAC1" },
	{ "IF1 DAC1 R", NULL, "IF1 DAC1" },
	{ "IF2 DAC L", NULL, "IF2 DAC" },
	{ "IF2 DAC R", NULL, "IF2 DAC" },

	{ "DAC1 L Mux", "IF1 DAC", "IF1 DAC1 L" },
	{ "DAC1 L Mux", "IF2 DAC", "IF2 DAC L" },

	{ "DAC1 R Mux", "IF1 DAC", "IF1 DAC1 R" },
	{ "DAC1 R Mux", "IF2 DAC", "IF2 DAC R" },

	{ "DAC1 MIXL", "Stereo ADC Switch", "Stereo1 ADC MIXL" },
	{ "DAC1 MIXL", "DAC1 Switch", "DAC1 L Mux" },
	{ "DAC1 MIXL", NULL, "dac stereo1 filter" },
	{ "DAC1 MIXR", "Stereo ADC Switch", "Stereo1 ADC MIXR" },
	{ "DAC1 MIXR", "DAC1 Switch", "DAC1 R Mux" },
	{ "DAC1 MIXR", NULL, "dac stereo1 filter" },

	{ "DAC L2 Mux", "IF1 DAC", "IF1 DAC2 L" },
	{ "DAC L2 Mux", "IF2 DAC", "IF2 DAC L" },
	{ "DAC L2 Mux", "Mono ADC", "Mono ADC MIXL" },
	{ "DAC L2 Mux", "VAD_ADC", "VAD_ADC" },
	{ "DAC L2 Volume", NULL, "DAC L2 Mux" },
	{ "DAC L2 Volume", NULL, "dac mono left filter" },

	{ "DAC R2 Mux", "IF1 DAC", "IF1 DAC2 R" },
	{ "DAC R2 Mux", "IF2 DAC", "IF2 DAC R" },
	{ "DAC R2 Mux", "Mono ADC", "Mono ADC MIXR" },
	{ "DAC R2 Mux", "Haptic", "Haptic Generator" },
	{ "DAC R2 Volume", NULL, "DAC R2 Mux" },
	{ "DAC R2 Volume", NULL, "dac mono right filter" },

	{ "Stereo DAC MIXL", "DAC L1 Switch", "DAC1 MIXL" },
	{ "Stereo DAC MIXL", "DAC R1 Switch", "DAC1 MIXR" },
	{ "Stereo DAC MIXL", "DAC L2 Switch", "DAC L2 Volume" },
	{ "Stereo DAC MIXL", NULL, "dac stereo1 filter" },
	{ "Stereo DAC MIXR", "DAC R1 Switch", "DAC1 MIXR" },
	{ "Stereo DAC MIXR", "DAC L1 Switch", "DAC1 MIXL" },
	{ "Stereo DAC MIXR", "DAC R2 Switch", "DAC R2 Volume" },
	{ "Stereo DAC MIXR", NULL, "dac stereo1 filter" },

	{ "Mono DAC MIXL", "DAC L1 Switch", "DAC1 MIXL" },
	{ "Mono DAC MIXL", "DAC L2 Switch", "DAC L2 Volume" },
	{ "Mono DAC MIXL", "DAC R2 Switch", "DAC R2 Volume" },
	{ "Mono DAC MIXL", NULL, "dac mono left filter" },
	{ "Mono DAC MIXR", "DAC R1 Switch", "DAC1 MIXR" },
	{ "Mono DAC MIXR", "DAC R2 Switch", "DAC R2 Volume" },
	{ "Mono DAC MIXR", "DAC L2 Switch", "DAC L2 Volume" },
	{ "Mono DAC MIXR", NULL, "dac mono right filter" },

	{ "DAC MIXL", "Sto DAC Mix L Switch", "Stereo DAC MIXL" },
	{ "DAC MIXL", "DAC L2 Switch", "DAC L2 Volume" },
	{ "DAC MIXL", "DAC R2 Switch", "DAC R2 Volume" },
	{ "DAC MIXR", "Sto DAC Mix R Switch", "Stereo DAC MIXR" },
	{ "DAC MIXR", "DAC R2 Switch", "DAC R2 Volume" },
	{ "DAC MIXR", "DAC L2 Switch", "DAC L2 Volume" },

	{ "DAC L1", NULL, "PLL1", is_sys_clk_from_pll },
	{ "DAC R1", NULL, "PLL1", is_sys_clk_from_pll },
	{ "DAC L2", NULL, "PLL1", is_sys_clk_from_pll },
	{ "DAC R2", NULL, "PLL1", is_sys_clk_from_pll },

	{ "SPK MIXL", "BST1 Switch", "BST1" },
	{ "SPK MIXL", "INL Switch", "INL VOL" },
	{ "SPK MIXL", "DAC L1 Switch", "DAC L1" },
	{ "SPK MIXL", "DAC L2 Switch", "DAC L2" },
	{ "SPK MIXR", "BST2 Switch", "BST2" },
	{ "SPK MIXR", "INR Switch", "INR VOL" },
	{ "SPK MIXR", "DAC R1 Switch", "DAC R1" },
	{ "SPK MIXR", "DAC R2 Switch", "DAC R2" },

	{ "OUT MIXL", "BST1 Switch", "BST1" },
	{ "OUT MIXL", "INL Switch", "INL VOL" },
	{ "OUT MIXL", "DAC L2 Switch", "DAC L2" },
	{ "OUT MIXL", "DAC L1 Switch", "DAC L1" },

	{ "OUT MIXR", "BST2 Switch", "BST2" },
	{ "OUT MIXR", "INR Switch", "INR VOL" },
	{ "OUT MIXR", "DAC R2 Switch", "DAC R2" },
	{ "OUT MIXR", "DAC R1 Switch", "DAC R1" },

	{ "HPOVOL MIXL", "DAC1 Switch", "DAC L1" },
	{ "HPOVOL MIXL", "DAC2 Switch", "DAC L2" },
	{ "HPOVOL MIXL", "INL Switch", "INL VOL" },
	{ "HPOVOL MIXL", "BST1 Switch", "BST1" },
	{ "HPOVOL MIXL", NULL, "HPOVOL MIXL Power" },
	{ "HPOVOL MIXR", "DAC1 Switch", "DAC R1" },
	{ "HPOVOL MIXR", "DAC2 Switch", "DAC R2" },
	{ "HPOVOL MIXR", "INR Switch", "INR VOL" },
	{ "HPOVOL MIXR", "BST2 Switch", "BST2" },
	{ "HPOVOL MIXR", NULL, "HPOVOL MIXR Power" },

	{ "DAC 2", NULL, "DAC L2" },
	{ "DAC 2", NULL, "DAC R2" },
	{ "DAC 1", NULL, "DAC L1" },
	{ "DAC 1", NULL, "DAC R1" },
	{ "HPOVOL L", "Switch", "HPOVOL MIXL" },
	{ "HPOVOL R", "Switch", "HPOVOL MIXR" },
	{ "HPOVOL", NULL, "HPOVOL L" },
	{ "HPOVOL", NULL, "HPOVOL R" },
	{ "HPO MIX", "DAC1 Switch", "DAC 1" },
	{ "HPO MIX", "HPVOL Switch", "HPOVOL" },

	{ "SPKVOL L", "Switch", "SPK MIXL" },
	{ "SPKVOL R", "Switch", "SPK MIXR" },

	{ "SPOL MIX", "DAC R1 Switch", "DAC R1" },
	{ "SPOL MIX", "DAC L1 Switch", "DAC L1" },
	{ "SPOL MIX", "SPKVOL R Switch", "SPKVOL R" },
	{ "SPOL MIX", "SPKVOL L Switch", "SPKVOL L" },
	{ "SPOR MIX", "DAC R1 Switch", "DAC R1" },
	{ "SPOR MIX", "SPKVOL R Switch", "SPKVOL R" },

	{ "LOUT MIX", "DAC L1 Switch", "DAC L1" },
	{ "LOUT MIX", "DAC R1 Switch", "DAC R1" },
	{ "LOUT MIX", "OUTMIX L Switch", "OUT MIXL" },
	{ "LOUT MIX", "OUTMIX R Switch", "OUT MIXR" },

	{ "PDM1 L Mux", "Stereo DAC", "Stereo DAC MIXL" },
	{ "PDM1 L Mux", "Mono DAC", "Mono DAC MIXL" },
	{ "PDM1 L Mux", NULL, "PDM1 Power" },
	{ "PDM1 R Mux", "Stereo DAC", "Stereo DAC MIXR" },
	{ "PDM1 R Mux", "Mono DAC", "Mono DAC MIXR" },
	{ "PDM1 R Mux", NULL, "PDM1 Power" },

	{ "HP amp", NULL, "HPO MIX" },
	{ "HP amp", NULL, "JD Power" },
	{ "HP amp", NULL, "Mic Det Power" },
	{ "HP amp", NULL, "LDO2" },
	{ "HPOL", NULL, "HP amp" },
	{ "HPOR", NULL, "HP amp" },

	{ "LOUT amp", NULL, "LOUT MIX" },
	{ "LOUTL", NULL, "LOUT amp" },
	{ "LOUTR", NULL, "LOUT amp" },

	{ "PDM1 L", "Switch", "PDM1 L Mux" },
	{ "PDM1 R", "Switch", "PDM1 R Mux" },

	{ "PDM1L", NULL, "PDM1 L" },
	{ "PDM1R", NULL, "PDM1 R" },

	{ "SPK amp", NULL, "SPOL MIX" },
	{ "SPK amp", NULL, "SPOR MIX" },
	{ "SPOL", NULL, "SPK amp" },
	{ "SPOR", NULL, "SPK amp" },
};

static const struct snd_soc_dapm_route rt5650_specific_dapm_routes[] = {
	{ "A DAC1 L Mux", "DAC1",  "DAC1 MIXL"},
	{ "A DAC1 L Mux", "Stereo DAC Mixer", "Stereo DAC MIXL"},
	{ "A DAC1 R Mux", "DAC1",  "DAC1 MIXR"},
	{ "A DAC1 R Mux", "Stereo DAC Mixer", "Stereo DAC MIXR"},

	{ "A DAC2 L Mux", "Stereo DAC Mixer", "Stereo DAC MIXL"},
	{ "A DAC2 L Mux", "Mono DAC Mixer", "Mono DAC MIXL"},
	{ "A DAC2 R Mux", "Stereo DAC Mixer", "Stereo DAC MIXR"},
	{ "A DAC2 R Mux", "Mono DAC Mixer", "Mono DAC MIXR"},

	{ "DAC L1", NULL, "A DAC1 L Mux" },
	{ "DAC R1", NULL, "A DAC1 R Mux" },
	{ "DAC L2", NULL, "A DAC2 L Mux" },
	{ "DAC R2", NULL, "A DAC2 R Mux" },
};

static const struct snd_soc_dapm_route rt5645_specific_dapm_routes[] = {
	{ "DAC L1", NULL, "Stereo DAC MIXL" },
	{ "DAC R1", NULL, "Stereo DAC MIXR" },
	{ "DAC L2", NULL, "Mono DAC MIXL" },
	{ "DAC R2", NULL, "Mono DAC MIXR" },
};

static int rt5645_hw_params(struct snd_pcm_substream *substream,
	struct snd_pcm_hw_params *params, struct snd_soc_dai *dai)
{
	struct snd_soc_codec *codec = dai->codec;
	struct rt5645_priv *rt5645 = snd_soc_codec_get_drvdata(codec);
	unsigned int val_len = 0, val_clk, mask_clk, dl_sft;
	int pre_div, bclk_ms, frame_size;

	rt5645->lrck[dai->id] = params_rate(params);
	pre_div = rl6231_get_clk_info(rt5645->sysclk, rt5645->lrck[dai->id]);
	if (pre_div < 0) {
		dev_err(codec->dev, "Unsupported clock setting\n");
		return -EINVAL;
	}
	frame_size = snd_soc_params_to_frame_size(params);
	if (frame_size < 0) {
		dev_err(codec->dev, "Unsupported frame size: %d\n", frame_size);
		return -EINVAL;
	}

	switch (rt5645->codec_type) {
	case CODEC_TYPE_RT5650:
		dl_sft = 4;
		break;
	default:
		dl_sft = 2;
		break;
	}

	bclk_ms = frame_size > 32;
	rt5645->bclk[dai->id] = rt5645->lrck[dai->id] * (32 << bclk_ms);

	dev_dbg(dai->dev, "bclk is %dHz and lrck is %dHz\n",
		rt5645->bclk[dai->id], rt5645->lrck[dai->id]);
	dev_dbg(dai->dev, "bclk_ms is %d and pre_div is %d for iis %d\n",
				bclk_ms, pre_div, dai->id);

	switch (params_width(params)) {
	case 16:
		break;
	case 20:
		val_len = 0x1;
		break;
	case 24:
		val_len = 0x2;
		break;
	case 8:
		val_len = 0x3;
		break;
	default:
		return -EINVAL;
	}

	switch (dai->id) {
	case RT5645_AIF1:
		mask_clk = RT5645_I2S_BCLK_MS1_MASK | RT5645_I2S_PD1_MASK;
		val_clk = bclk_ms << RT5645_I2S_BCLK_MS1_SFT |
			pre_div << RT5645_I2S_PD1_SFT;
		snd_soc_update_bits(codec, RT5645_I2S1_SDP,
			(0x3 << dl_sft), (val_len << dl_sft));
		snd_soc_update_bits(codec, RT5645_ADDA_CLK1, mask_clk, val_clk);
		break;
	case  RT5645_AIF2:
		mask_clk = RT5645_I2S_BCLK_MS2_MASK | RT5645_I2S_PD2_MASK;
		val_clk = bclk_ms << RT5645_I2S_BCLK_MS2_SFT |
			pre_div << RT5645_I2S_PD2_SFT;
		snd_soc_update_bits(codec, RT5645_I2S2_SDP,
			(0x3 << dl_sft), (val_len << dl_sft));
		snd_soc_update_bits(codec, RT5645_ADDA_CLK1, mask_clk, val_clk);
		break;
	default:
		dev_err(codec->dev, "Invalid dai->id: %d\n", dai->id);
		return -EINVAL;
	}

	return 0;
}

static int rt5645_set_dai_fmt(struct snd_soc_dai *dai, unsigned int fmt)
{
	struct snd_soc_codec *codec = dai->codec;
	struct rt5645_priv *rt5645 = snd_soc_codec_get_drvdata(codec);
	unsigned int reg_val = 0, pol_sft;

	switch (rt5645->codec_type) {
	case CODEC_TYPE_RT5650:
		pol_sft = 8;
		break;
	default:
		pol_sft = 7;
		break;
	}

	switch (fmt & SND_SOC_DAIFMT_MASTER_MASK) {
	case SND_SOC_DAIFMT_CBM_CFM:
		rt5645->master[dai->id] = 1;
		break;
	case SND_SOC_DAIFMT_CBS_CFS:
		reg_val |= RT5645_I2S_MS_S;
		rt5645->master[dai->id] = 0;
		break;
	default:
		return -EINVAL;
	}

	switch (fmt & SND_SOC_DAIFMT_INV_MASK) {
	case SND_SOC_DAIFMT_NB_NF:
		break;
	case SND_SOC_DAIFMT_IB_NF:
		reg_val |= (1 << pol_sft);
		break;
	default:
		return -EINVAL;
	}

	switch (fmt & SND_SOC_DAIFMT_FORMAT_MASK) {
	case SND_SOC_DAIFMT_I2S:
		break;
	case SND_SOC_DAIFMT_LEFT_J:
		reg_val |= RT5645_I2S_DF_LEFT;
		break;
	case SND_SOC_DAIFMT_DSP_A:
		reg_val |= RT5645_I2S_DF_PCM_A;
		break;
	case SND_SOC_DAIFMT_DSP_B:
		reg_val |= RT5645_I2S_DF_PCM_B;
		break;
	default:
		return -EINVAL;
	}
	switch (dai->id) {
	case RT5645_AIF1:
		snd_soc_update_bits(codec, RT5645_I2S1_SDP,
			RT5645_I2S_MS_MASK | (1 << pol_sft) |
			RT5645_I2S_DF_MASK, reg_val);
		break;
	case RT5645_AIF2:
		snd_soc_update_bits(codec, RT5645_I2S2_SDP,
			RT5645_I2S_MS_MASK | (1 << pol_sft) |
			RT5645_I2S_DF_MASK, reg_val);
		break;
	default:
		dev_err(codec->dev, "Invalid dai->id: %d\n", dai->id);
		return -EINVAL;
	}
	return 0;
}

static int rt5645_set_dai_sysclk(struct snd_soc_dai *dai,
		int clk_id, unsigned int freq, int dir)
{
	struct snd_soc_codec *codec = dai->codec;
	struct rt5645_priv *rt5645 = snd_soc_codec_get_drvdata(codec);
	unsigned int reg_val = 0;

	if (freq == rt5645->sysclk && clk_id == rt5645->sysclk_src)
		return 0;

	switch (clk_id) {
	case RT5645_SCLK_S_MCLK:
		reg_val |= RT5645_SCLK_SRC_MCLK;
		break;
	case RT5645_SCLK_S_PLL1:
		reg_val |= RT5645_SCLK_SRC_PLL1;
		break;
	case RT5645_SCLK_S_RCCLK:
		reg_val |= RT5645_SCLK_SRC_RCCLK;
		break;
	default:
		dev_err(codec->dev, "Invalid clock id (%d)\n", clk_id);
		return -EINVAL;
	}
	snd_soc_update_bits(codec, RT5645_GLB_CLK,
		RT5645_SCLK_SRC_MASK, reg_val);
	rt5645->sysclk = freq;
	rt5645->sysclk_src = clk_id;

	dev_dbg(dai->dev, "Sysclk is %dHz and clock id is %d\n", freq, clk_id);

	return 0;
}

static int rt5645_set_dai_pll(struct snd_soc_dai *dai, int pll_id, int source,
			unsigned int freq_in, unsigned int freq_out)
{
	struct snd_soc_codec *codec = dai->codec;
	struct rt5645_priv *rt5645 = snd_soc_codec_get_drvdata(codec);
	struct rl6231_pll_code pll_code;
	int ret;

	if (source == rt5645->pll_src && freq_in == rt5645->pll_in &&
	    freq_out == rt5645->pll_out)
		return 0;

	if (!freq_in || !freq_out) {
		dev_dbg(codec->dev, "PLL disabled\n");

		rt5645->pll_in = 0;
		rt5645->pll_out = 0;
		snd_soc_update_bits(codec, RT5645_GLB_CLK,
			RT5645_SCLK_SRC_MASK, RT5645_SCLK_SRC_MCLK);
		return 0;
	}

	switch (source) {
	case RT5645_PLL1_S_MCLK:
		snd_soc_update_bits(codec, RT5645_GLB_CLK,
			RT5645_PLL1_SRC_MASK, RT5645_PLL1_SRC_MCLK);
		break;
	case RT5645_PLL1_S_BCLK1:
	case RT5645_PLL1_S_BCLK2:
		switch (dai->id) {
		case RT5645_AIF1:
			snd_soc_update_bits(codec, RT5645_GLB_CLK,
				RT5645_PLL1_SRC_MASK, RT5645_PLL1_SRC_BCLK1);
			break;
		case  RT5645_AIF2:
			snd_soc_update_bits(codec, RT5645_GLB_CLK,
				RT5645_PLL1_SRC_MASK, RT5645_PLL1_SRC_BCLK2);
			break;
		default:
			dev_err(codec->dev, "Invalid dai->id: %d\n", dai->id);
			return -EINVAL;
		}
		break;
	default:
		dev_err(codec->dev, "Unknown PLL source %d\n", source);
		return -EINVAL;
	}

	ret = rl6231_pll_calc(freq_in, freq_out, &pll_code);
	if (ret < 0) {
		dev_err(codec->dev, "Unsupport input clock %d\n", freq_in);
		return ret;
	}

	dev_dbg(codec->dev, "bypass=%d m=%d n=%d k=%d\n",
		pll_code.m_bp, (pll_code.m_bp ? 0 : pll_code.m_code),
		pll_code.n_code, pll_code.k_code);

	snd_soc_write(codec, RT5645_PLL_CTRL1,
		pll_code.n_code << RT5645_PLL_N_SFT | pll_code.k_code);
	snd_soc_write(codec, RT5645_PLL_CTRL2,
		(pll_code.m_bp ? 0 : pll_code.m_code) << RT5645_PLL_M_SFT |
		pll_code.m_bp << RT5645_PLL_M_BP_SFT);

	rt5645->pll_in = freq_in;
	rt5645->pll_out = freq_out;
	rt5645->pll_src = source;

	return 0;
}

static int rt5645_set_tdm_slot(struct snd_soc_dai *dai, unsigned int tx_mask,
			unsigned int rx_mask, int slots, int slot_width)
{
	struct snd_soc_codec *codec = dai->codec;
	struct rt5645_priv *rt5645 = snd_soc_codec_get_drvdata(codec);
	unsigned int i_slot_sft, o_slot_sft, i_width_sht, o_width_sht, en_sft;
	unsigned int mask, val = 0;

	switch (rt5645->codec_type) {
	case CODEC_TYPE_RT5650:
		en_sft = 15;
		i_slot_sft = 10;
		o_slot_sft = 8;
		i_width_sht = 6;
		o_width_sht = 4;
		mask = 0x8ff0;
		break;
	default:
		en_sft = 14;
		i_slot_sft = o_slot_sft = 12;
		i_width_sht = o_width_sht = 10;
		mask = 0x7c00;
		break;
	}
	if (rx_mask || tx_mask) {
		val |= (1 << en_sft);
		if (rt5645->codec_type == CODEC_TYPE_RT5645)
			snd_soc_update_bits(codec, RT5645_BASS_BACK,
				RT5645_G_BB_BST_MASK, RT5645_G_BB_BST_25DB);
	}

	switch (slots) {
	case 4:
		val |= (1 << i_slot_sft) | (1 << o_slot_sft);
		break;
	case 6:
		val |= (2 << i_slot_sft) | (2 << o_slot_sft);
		break;
	case 8:
		val |= (3 << i_slot_sft) | (3 << o_slot_sft);
		break;
	case 2:
	default:
		break;
	}

	switch (slot_width) {
	case 20:
		val |= (1 << i_width_sht) | (1 << o_width_sht);
		break;
	case 24:
		val |= (2 << i_width_sht) | (2 << o_width_sht);
		break;
	case 32:
		val |= (3 << i_width_sht) | (3 << o_width_sht);
		break;
	case 16:
	default:
		break;
	}

	snd_soc_update_bits(codec, RT5645_TDM_CTRL_1, mask, val);

	return 0;
}

static int rt5645_set_bias_level(struct snd_soc_codec *codec,
			enum snd_soc_bias_level level)
{
	switch (level) {
	case SND_SOC_BIAS_PREPARE:
		if (SND_SOC_BIAS_STANDBY == codec->dapm.bias_level) {
			snd_soc_update_bits(codec, RT5645_PWR_ANLG1,
				RT5645_PWR_VREF1 | RT5645_PWR_MB |
				RT5645_PWR_BG | RT5645_PWR_VREF2,
				RT5645_PWR_VREF1 | RT5645_PWR_MB |
				RT5645_PWR_BG | RT5645_PWR_VREF2);
			mdelay(10);
			snd_soc_update_bits(codec, RT5645_PWR_ANLG1,
				RT5645_PWR_FV1 | RT5645_PWR_FV2,
				RT5645_PWR_FV1 | RT5645_PWR_FV2);
			snd_soc_update_bits(codec, RT5645_GEN_CTRL1,
				RT5645_DIG_GATE_CTRL, RT5645_DIG_GATE_CTRL);
		}
		break;

	case SND_SOC_BIAS_STANDBY:
		snd_soc_update_bits(codec, RT5645_PWR_ANLG1,
			RT5645_PWR_VREF1 | RT5645_PWR_MB |
			RT5645_PWR_BG | RT5645_PWR_VREF2,
			RT5645_PWR_VREF1 | RT5645_PWR_MB |
			RT5645_PWR_BG | RT5645_PWR_VREF2);
		snd_soc_update_bits(codec, RT5645_PWR_ANLG1,
			RT5645_PWR_FV1 | RT5645_PWR_FV2,
			RT5645_PWR_FV1 | RT5645_PWR_FV2);
		break;

	case SND_SOC_BIAS_OFF:
		snd_soc_write(codec, RT5645_DEPOP_M2, 0x1100);
		snd_soc_update_bits(codec, RT5645_GEN_CTRL1,
				RT5645_DIG_GATE_CTRL, 0);
		snd_soc_update_bits(codec, RT5645_PWR_ANLG1,
				RT5645_PWR_VREF1 | RT5645_PWR_MB |
				RT5645_PWR_BG | RT5645_PWR_VREF2 |
				RT5645_PWR_FV1 | RT5645_PWR_FV2, 0x0);
		break;

	default:
		break;
	}
	codec->dapm.bias_level = level;

	return 0;
}

static int rt5645_jack_detect(struct snd_soc_codec *codec)
{
	struct rt5645_priv *rt5645 = snd_soc_codec_get_drvdata(codec);
	int gpio_state, jack_type = 0;
	unsigned int val;

	if (!gpio_is_valid(rt5645->pdata.hp_det_gpio)) {
		dev_err(codec->dev, "invalid gpio\n");
		return -EINVAL;
	}
	gpio_state = gpio_get_value(rt5645->pdata.hp_det_gpio);

	dev_dbg(codec->dev, "gpio = %d(%d)\n", rt5645->pdata.hp_det_gpio,
		gpio_state);

	if ((rt5645->pdata.gpio_hp_det_active_high && gpio_state) ||
		(!rt5645->pdata.gpio_hp_det_active_high && !gpio_state)) {
		snd_soc_dapm_force_enable_pin(&codec->dapm, "micbias1");
		snd_soc_dapm_force_enable_pin(&codec->dapm, "micbias2");
		snd_soc_dapm_force_enable_pin(&codec->dapm, "LDO2");
		snd_soc_dapm_force_enable_pin(&codec->dapm, "Mic Det Power");
		snd_soc_dapm_sync(&codec->dapm);

		snd_soc_write(codec, RT5645_IN1_CTRL1, 0x0006);
		snd_soc_write(codec, RT5645_JD_CTRL3, 0x00b0);

		snd_soc_update_bits(codec, RT5645_IN1_CTRL2,
			RT5645_CBJ_MN_JD, 0);
		snd_soc_update_bits(codec, RT5645_IN1_CTRL2,
			RT5645_CBJ_MN_JD, RT5645_CBJ_MN_JD);

		msleep(400);
		val = snd_soc_read(codec, RT5645_IN1_CTRL3) & 0x7;
		dev_dbg(codec->dev, "val = %d\n", val);

		if (val == 1 || val == 2)
			jack_type = SND_JACK_HEADSET;
		else
			jack_type = SND_JACK_HEADPHONE;

		snd_soc_dapm_disable_pin(&codec->dapm, "micbias1");
		snd_soc_dapm_disable_pin(&codec->dapm, "micbias2");
		if (rt5645->pdata.jd_mode == 0)
			snd_soc_dapm_disable_pin(&codec->dapm, "LDO2");
		snd_soc_dapm_disable_pin(&codec->dapm, "Mic Det Power");
		snd_soc_dapm_sync(&codec->dapm);
	}

	snd_soc_jack_report(rt5645->hp_jack, jack_type, SND_JACK_HEADPHONE);
	snd_soc_jack_report(rt5645->mic_jack, jack_type, SND_JACK_MICROPHONE);
	return 0;
}

int rt5645_set_jack_detect(struct snd_soc_codec *codec,
	struct snd_soc_jack *hp_jack, struct snd_soc_jack *mic_jack)
{
	struct rt5645_priv *rt5645 = snd_soc_codec_get_drvdata(codec);

	rt5645->hp_jack = hp_jack;
	rt5645->mic_jack = mic_jack;
	rt5645_jack_detect(codec);

	return 0;
}
EXPORT_SYMBOL_GPL(rt5645_set_jack_detect);

static void rt5645_jack_detect_work(struct work_struct *work)
{
	struct rt5645_priv *rt5645 =
		container_of(work, struct rt5645_priv, jack_detect_work.work);

	rt5645_jack_detect(rt5645->codec);
}

static irqreturn_t rt5645_irq(int irq, void *data)
{
	struct rt5645_priv *rt5645 = data;

	queue_delayed_work(system_power_efficient_wq,
			   &rt5645->jack_detect_work, msecs_to_jiffies(250));

	return IRQ_HANDLED;
}

static int rt5645_probe(struct snd_soc_codec *codec)
{
	struct rt5645_priv *rt5645 = snd_soc_codec_get_drvdata(codec);

	rt5645->codec = codec;

	switch (rt5645->codec_type) {
	case CODEC_TYPE_RT5645:
		snd_soc_dapm_add_routes(&codec->dapm,
			rt5645_specific_dapm_routes,
			ARRAY_SIZE(rt5645_specific_dapm_routes));
		break;
	case CODEC_TYPE_RT5650:
		snd_soc_dapm_new_controls(&codec->dapm,
			rt5650_specific_dapm_widgets,
			ARRAY_SIZE(rt5650_specific_dapm_widgets));
		snd_soc_dapm_add_routes(&codec->dapm,
			rt5650_specific_dapm_routes,
			ARRAY_SIZE(rt5650_specific_dapm_routes));
		break;
	}

	rt5645_set_bias_level(codec, SND_SOC_BIAS_OFF);

	snd_soc_update_bits(codec, RT5645_CHARGE_PUMP, 0x0300, 0x0200);

	/* for JD function */
	if (rt5645->pdata.en_jd_func) {
		snd_soc_dapm_force_enable_pin(&codec->dapm, "JD Power");
		snd_soc_dapm_force_enable_pin(&codec->dapm, "LDO2");
		snd_soc_dapm_sync(&codec->dapm);
	}

	return 0;
}

static int rt5645_remove(struct snd_soc_codec *codec)
{
	rt5645_reset(codec);
	return 0;
}

#ifdef CONFIG_PM
static int rt5645_suspend(struct snd_soc_codec *codec)
{
	struct rt5645_priv *rt5645 = snd_soc_codec_get_drvdata(codec);

	regcache_cache_only(rt5645->regmap, true);
	regcache_mark_dirty(rt5645->regmap);

	return 0;
}

static int rt5645_resume(struct snd_soc_codec *codec)
{
	struct rt5645_priv *rt5645 = snd_soc_codec_get_drvdata(codec);

	regcache_cache_only(rt5645->regmap, false);
	regcache_sync(rt5645->regmap);

	return 0;
}
#else
#define rt5645_suspend NULL
#define rt5645_resume NULL
#endif

#define RT5645_STEREO_RATES SNDRV_PCM_RATE_8000_96000
#define RT5645_FORMATS (SNDRV_PCM_FMTBIT_S16_LE | SNDRV_PCM_FMTBIT_S20_3LE | \
			SNDRV_PCM_FMTBIT_S24_LE | SNDRV_PCM_FMTBIT_S8)

static struct snd_soc_dai_ops rt5645_aif_dai_ops = {
	.hw_params = rt5645_hw_params,
	.set_fmt = rt5645_set_dai_fmt,
	.set_sysclk = rt5645_set_dai_sysclk,
	.set_tdm_slot = rt5645_set_tdm_slot,
	.set_pll = rt5645_set_dai_pll,
};

static struct snd_soc_dai_driver rt5645_dai[] = {
	{
		.name = "rt5645-aif1",
		.id = RT5645_AIF1,
		.playback = {
			.stream_name = "AIF1 Playback",
			.channels_min = 1,
			.channels_max = 2,
			.rates = RT5645_STEREO_RATES,
			.formats = RT5645_FORMATS,
		},
		.capture = {
			.stream_name = "AIF1 Capture",
			.channels_min = 1,
			.channels_max = 2,
			.rates = RT5645_STEREO_RATES,
			.formats = RT5645_FORMATS,
		},
		.ops = &rt5645_aif_dai_ops,
	},
	{
		.name = "rt5645-aif2",
		.id = RT5645_AIF2,
		.playback = {
			.stream_name = "AIF2 Playback",
			.channels_min = 1,
			.channels_max = 2,
			.rates = RT5645_STEREO_RATES,
			.formats = RT5645_FORMATS,
		},
		.capture = {
			.stream_name = "AIF2 Capture",
			.channels_min = 1,
			.channels_max = 2,
			.rates = RT5645_STEREO_RATES,
			.formats = RT5645_FORMATS,
		},
		.ops = &rt5645_aif_dai_ops,
	},
};

static struct snd_soc_codec_driver soc_codec_dev_rt5645 = {
	.probe = rt5645_probe,
	.remove = rt5645_remove,
	.suspend = rt5645_suspend,
	.resume = rt5645_resume,
	.set_bias_level = rt5645_set_bias_level,
	.idle_bias_off = true,
	.controls = rt5645_snd_controls,
	.num_controls = ARRAY_SIZE(rt5645_snd_controls),
	.dapm_widgets = rt5645_dapm_widgets,
	.num_dapm_widgets = ARRAY_SIZE(rt5645_dapm_widgets),
	.dapm_routes = rt5645_dapm_routes,
	.num_dapm_routes = ARRAY_SIZE(rt5645_dapm_routes),
};

static const struct regmap_config rt5645_regmap = {
	.reg_bits = 8,
	.val_bits = 16,
	.use_single_rw = true,
	.max_register = RT5645_VENDOR_ID2 + 1 + (ARRAY_SIZE(rt5645_ranges) *
					       RT5645_PR_SPACING),
	.volatile_reg = rt5645_volatile_register,
	.readable_reg = rt5645_readable_register,

	.cache_type = REGCACHE_RBTREE,
	.reg_defaults = rt5645_reg,
	.num_reg_defaults = ARRAY_SIZE(rt5645_reg),
	.ranges = rt5645_ranges,
	.num_ranges = ARRAY_SIZE(rt5645_ranges),
};

static const struct i2c_device_id rt5645_i2c_id[] = {
	{ "rt5645", 0 },
	{ "rt5650", 0 },
	{ }
};
MODULE_DEVICE_TABLE(i2c, rt5645_i2c_id);

#ifdef CONFIG_ACPI
static struct acpi_device_id rt5645_acpi_match[] = {
	{ "10EC5645", 0 },
	{ "10EC5650", 0 },
	{},
};
MODULE_DEVICE_TABLE(acpi, rt5645_acpi_match);
#endif

static int rt5645_i2c_probe(struct i2c_client *i2c,
		    const struct i2c_device_id *id)
{
	struct rt5645_platform_data *pdata = dev_get_platdata(&i2c->dev);
	struct rt5645_priv *rt5645;
	int ret;
	unsigned int val;

	rt5645 = devm_kzalloc(&i2c->dev, sizeof(struct rt5645_priv),
				GFP_KERNEL);
	if (rt5645 == NULL)
		return -ENOMEM;

	rt5645->i2c = i2c;
	i2c_set_clientdata(i2c, rt5645);

	if (pdata)
		rt5645->pdata = *pdata;

	rt5645->regmap = devm_regmap_init_i2c(i2c, &rt5645_regmap);
	if (IS_ERR(rt5645->regmap)) {
		ret = PTR_ERR(rt5645->regmap);
		dev_err(&i2c->dev, "Failed to allocate register map: %d\n",
			ret);
		return ret;
	}

	regmap_read(rt5645->regmap, RT5645_VENDOR_ID2, &val);

	switch (val) {
	case RT5645_DEVICE_ID:
		rt5645->codec_type = CODEC_TYPE_RT5645;
		break;
	case RT5650_DEVICE_ID:
		rt5645->codec_type = CODEC_TYPE_RT5650;
		break;
	default:
		dev_err(&i2c->dev,
			"Device with ID register %x is not rt5645 or rt5650\n",
			val);
		return -ENODEV;
	}

	regmap_write(rt5645->regmap, RT5645_RESET, 0);

	ret = regmap_register_patch(rt5645->regmap, init_list,
				    ARRAY_SIZE(init_list));
	if (ret != 0)
		dev_warn(&i2c->dev, "Failed to apply regmap patch: %d\n", ret);

	if (rt5645->codec_type == CODEC_TYPE_RT5650) {
		ret = regmap_register_patch(rt5645->regmap, rt5650_init_list,
				    ARRAY_SIZE(rt5650_init_list));
		if (ret != 0)
			dev_warn(&i2c->dev, "Apply rt5650 patch failed: %d\n",
					   ret);
	}

	if (rt5645->pdata.in2_diff)
		regmap_update_bits(rt5645->regmap, RT5645_IN2_CTRL,
					RT5645_IN_DF2, RT5645_IN_DF2);

	if (rt5645->pdata.dmic_en) {
		regmap_update_bits(rt5645->regmap, RT5645_GPIO_CTRL1,
			RT5645_GP2_PIN_MASK, RT5645_GP2_PIN_DMIC1_SCL);

		switch (rt5645->pdata.dmic1_data_pin) {
		case RT5645_DMIC_DATA_IN2N:
			regmap_update_bits(rt5645->regmap, RT5645_DMIC_CTRL1,
				RT5645_DMIC_1_DP_MASK, RT5645_DMIC_1_DP_IN2N);
			break;

		case RT5645_DMIC_DATA_GPIO5:
			regmap_update_bits(rt5645->regmap, RT5645_DMIC_CTRL1,
				RT5645_DMIC_1_DP_MASK, RT5645_DMIC_1_DP_GPIO5);
			regmap_update_bits(rt5645->regmap, RT5645_GPIO_CTRL1,
				RT5645_GP5_PIN_MASK, RT5645_GP5_PIN_DMIC1_SDA);
			break;

		case RT5645_DMIC_DATA_GPIO11:
			regmap_update_bits(rt5645->regmap, RT5645_DMIC_CTRL1,
				RT5645_DMIC_1_DP_MASK, RT5645_DMIC_1_DP_GPIO11);
			regmap_update_bits(rt5645->regmap, RT5645_GPIO_CTRL1,
				RT5645_GP11_PIN_MASK,
				RT5645_GP11_PIN_DMIC1_SDA);
			break;

		default:
			break;
		}

		switch (rt5645->pdata.dmic2_data_pin) {
		case RT5645_DMIC_DATA_IN2P:
			regmap_update_bits(rt5645->regmap, RT5645_DMIC_CTRL1,
				RT5645_DMIC_2_DP_MASK, RT5645_DMIC_2_DP_IN2P);
			break;

		case RT5645_DMIC_DATA_GPIO6:
			regmap_update_bits(rt5645->regmap, RT5645_DMIC_CTRL1,
				RT5645_DMIC_2_DP_MASK, RT5645_DMIC_2_DP_GPIO6);
			regmap_update_bits(rt5645->regmap, RT5645_GPIO_CTRL1,
				RT5645_GP6_PIN_MASK, RT5645_GP6_PIN_DMIC2_SDA);
			break;

		case RT5645_DMIC_DATA_GPIO10:
			regmap_update_bits(rt5645->regmap, RT5645_DMIC_CTRL1,
				RT5645_DMIC_2_DP_MASK, RT5645_DMIC_2_DP_GPIO10);
			regmap_update_bits(rt5645->regmap, RT5645_GPIO_CTRL1,
				RT5645_GP10_PIN_MASK,
				RT5645_GP10_PIN_DMIC2_SDA);
			break;

		case RT5645_DMIC_DATA_GPIO12:
			regmap_update_bits(rt5645->regmap, RT5645_DMIC_CTRL1,
				RT5645_DMIC_2_DP_MASK, RT5645_DMIC_2_DP_GPIO12);
			regmap_update_bits(rt5645->regmap, RT5645_GPIO_CTRL1,
				RT5645_GP12_PIN_MASK,
				RT5645_GP12_PIN_DMIC2_SDA);
			break;

		default:
			break;
		}

	}

	if (rt5645->pdata.en_jd_func) {
		regmap_update_bits(rt5645->regmap, RT5645_GEN_CTRL3,
			RT5645_IRQ_CLK_GATE_CTRL | RT5645_MICINDET_MANU,
			RT5645_IRQ_CLK_GATE_CTRL | RT5645_MICINDET_MANU);
		regmap_update_bits(rt5645->regmap, RT5645_IN1_CTRL1,
			RT5645_CBJ_BST1_EN, RT5645_CBJ_BST1_EN);
		regmap_update_bits(rt5645->regmap, RT5645_JD_CTRL3,
			RT5645_JD_CBJ_EN | RT5645_JD_CBJ_POL,
			RT5645_JD_CBJ_EN | RT5645_JD_CBJ_POL);
		regmap_update_bits(rt5645->regmap, RT5645_MICBIAS,
			RT5645_IRQ_CLK_INT, RT5645_IRQ_CLK_INT);
	}

	if (rt5645->pdata.jd_mode) {
		regmap_update_bits(rt5645->regmap, RT5645_IRQ_CTRL2,
				   RT5645_IRQ_JD_1_1_EN, RT5645_IRQ_JD_1_1_EN);
		regmap_update_bits(rt5645->regmap, RT5645_GEN_CTRL3,
				   RT5645_JD_PSV_MODE, RT5645_JD_PSV_MODE);
		regmap_update_bits(rt5645->regmap, RT5645_HPO_MIXER,
				   RT5645_IRQ_PSV_MODE, RT5645_IRQ_PSV_MODE);
		regmap_update_bits(rt5645->regmap, RT5645_MICBIAS,
				   RT5645_MIC2_OVCD_EN, RT5645_MIC2_OVCD_EN);
		regmap_update_bits(rt5645->regmap, RT5645_GPIO_CTRL1,
				   RT5645_GP1_PIN_IRQ, RT5645_GP1_PIN_IRQ);
		switch (rt5645->pdata.jd_mode) {
		case 1:
			regmap_update_bits(rt5645->regmap, RT5645_A_JD_CTRL1,
					   RT5645_JD1_MODE_MASK,
					   RT5645_JD1_MODE_0);
			break;
		case 2:
			regmap_update_bits(rt5645->regmap, RT5645_A_JD_CTRL1,
					   RT5645_JD1_MODE_MASK,
					   RT5645_JD1_MODE_1);
			break;
		case 3:
			regmap_update_bits(rt5645->regmap, RT5645_A_JD_CTRL1,
					   RT5645_JD1_MODE_MASK,
					   RT5645_JD1_MODE_2);
			break;
		default:
			break;
		}
	}

	INIT_DELAYED_WORK(&rt5645->jack_detect_work, rt5645_jack_detect_work);

	if (rt5645->i2c->irq) {
		ret = request_threaded_irq(rt5645->i2c->irq, NULL, rt5645_irq,
			IRQF_TRIGGER_RISING | IRQF_TRIGGER_FALLING
			| IRQF_ONESHOT, "rt5645", rt5645);
		if (ret)
			dev_err(&i2c->dev, "Failed to reguest IRQ: %d\n", ret);
	}

	if (gpio_is_valid(rt5645->pdata.hp_det_gpio)) {
		ret = gpio_request(rt5645->pdata.hp_det_gpio, "rt5645");
		if (ret)
			dev_err(&i2c->dev, "Fail gpio_request hp_det_gpio\n");

		ret = gpio_direction_input(rt5645->pdata.hp_det_gpio);
		if (ret)
			dev_err(&i2c->dev, "Fail gpio_direction hp_det_gpio\n");
	}

	return snd_soc_register_codec(&i2c->dev, &soc_codec_dev_rt5645,
				      rt5645_dai, ARRAY_SIZE(rt5645_dai));
}

static int rt5645_i2c_remove(struct i2c_client *i2c)
{
	struct rt5645_priv *rt5645 = i2c_get_clientdata(i2c);

	if (i2c->irq)
		free_irq(i2c->irq, rt5645);

	cancel_delayed_work_sync(&rt5645->jack_detect_work);

	if (gpio_is_valid(rt5645->pdata.hp_det_gpio))
		gpio_free(rt5645->pdata.hp_det_gpio);

	snd_soc_unregister_codec(&i2c->dev);

	return 0;
}

static struct i2c_driver rt5645_i2c_driver = {
	.driver = {
		.name = "rt5645",
		.owner = THIS_MODULE,
		.acpi_match_table = ACPI_PTR(rt5645_acpi_match),
	},
	.probe = rt5645_i2c_probe,
	.remove   = rt5645_i2c_remove,
	.id_table = rt5645_i2c_id,
};
module_i2c_driver(rt5645_i2c_driver);

MODULE_DESCRIPTION("ASoC RT5645 driver");
MODULE_AUTHOR("Bard Liao <bardliao@realtek.com>");
MODULE_LICENSE("GPL v2");
