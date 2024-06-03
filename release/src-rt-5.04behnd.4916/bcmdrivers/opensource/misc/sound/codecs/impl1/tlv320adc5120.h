/*
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

#ifndef _TLV320ADC5120_H
#define _TLV320ADC5120_H

#include "../../asoc/impl1/bcm63xx-i2stdm.h"

#define ADC5120_RATES SNDRV_PCM_RATE_8000_192000
#define ADC5120_FORMATS (SNDRV_PCM_FMTBIT_S16_LE | \
                         SNDRV_PCM_FMTBIT_S20_LE | \
                         SNDRV_PCM_FMTBIT_S24_LE | \
                         SNDRV_PCM_FMTBIT_S32_LE)

#define STUB_RATES SNDRV_PCM_RATE_8000_192000

#define STUB_FORMATS (SNDRV_PCM_FMTBIT_S16_LE | \
                         SNDRV_PCM_FMTBIT_S24_LE | \
                         SNDRV_PCM_FMTBIT_S32_LE)

#define ADC5120_TX_LOOPBACK_EN 		I2S_SW_LOOPBACK
#define ADC5120_NUM_TX_LOOPBACK_CH	2
#define ADC5120_MIN_CH			2
#define ADC5120_MAX_CH			4

#define ADC5120_PAGE_SIZE		128
#define ADC5120_PAGE_MAX		4
/* page 0 */
#define ADC5120_PSEL			0  /*reg 0*/
#define ADC5120_RESET			1

#define ADC5120_SLEEP_CFG		2
#define ADC5120_AREG_SELECT_SHIFT	7
#define ADC5120_AREG_SELECT_MSK		(1 << ADC5120_AREG_SELECT_SHIFT)
#define ADC5120_AREG_SELECT_EXT		0
#define ADC5120_AREG_SELECT_INTL	1
#define ADC5120_SLEEP_ENZ_SHIFT		0
#define ADC5120_SLEEP_ENZ_MSK		(1 << ADC5120_SLEEP_ENZ_SHIFT)
#define ADC5120_SLEEP_MODE		0
#define ADC5120_WAKEUP_MODE		1

#define ADC5120_ASI_CFG0		7 
#define ADC5120_ASI_FORMAT_SHIFT	6
#define ADC5120_ASI_FORMAT_MASK	(3 << ADC5120_ASI_FORMAT_SHIFT)
#define ADC5120_ASI_FORMAT_TDM	0
#define ADC5120_ASI_FORMAT_I2S	(1 << ADC5120_ASI_FORMAT_SHIFT)
#define ADC5120_ASI_FORMAT_LJ	(2 << ADC5120_ASI_FORMAT_SHIFT)
#define ADC5120_ASI_SLOT_LEN_SHIFT	4
#define ADC5120_ASI_SLOT_LEN_MASK	(3 << ADC5120_ASI_SLOT_LEN_SHIFT)
#define ADC5120_FSYNC_POL_SHIFT	3
#define ADC5120_FSYNC_POL_MASK	(1 << ADC5120_FSYNC_POL_SHIFT)
#define ADC5120_FSYNC_POL_STD	0
#define ADC5120_FSYNC_POL_INV	(1 << ADC5120_FSYNC_POL_SHIFT)
#define ADC5120_BCLK_POL_SHIFT	2
#define ADC5120_BCLK_POL_MASK	(1 << ADC5120_FSYNC_POL_SHIFT)
#define ADC5120_BCLK_POL_STD	0
#define ADC5120_BCLK_POL_INV	(1 << ADC5120_FSYNC_POL_SHIFT)

#define ADC5120_ASI_CFG1		8
#define ADC5120_BUS_KEEPER_SHIFT 5
#define ADC5120_BUS_KEEPER_MASK (3 << ADC5120_BUS_KEEPER_SHIFT)
#define ADC5120_BUS_KEEPER_DISABLE 0
#define ADC5120_BUS_KEEPER_ENABLE 1


#define ADC5120_MST_CFG0		19 /* 0x13 */
#define ADC5120_MST_SLV_SHIFT		7
#define ADC5120_MST_SLV_MASK		(1 << ADC5120_MST_SLV_SHIFT)
#define ADC5120_BCLK_FS_MASTER		(1 << ADC5120_MST_SLV_SHIFT)
#define ADC5120_BCLK_FS_SLAVE		0

#define ADC5120_BIAS_CFG		59 /*0x3b*/
#define ADC5120_BIAS_VAL_SHIFT	4
#define ADC5120_BIAS_VAL_MASK	(0x7 << ADC5120_BIAS_VAL_SHIFT)
#define ADC5120_BIAS_AS_GPI2	(0x7 << ADC5120_BIAS_VAL_SHIFT)
#define ADC5120_CH1_CFG0		60 /*0x3c*/
#define ADC5120_CH1_CFG2		62 /*0x3e*/
#define ADC5120_CH2_CFG0		65 /*0x41*/
#define ADC5120_CH2_CFG2		67 /*0x43*/
#define ADC5120_CH3_CFG0		70 /*0x46*/
#define ADC5120_CH3_CFG2		72 /*0x48*/
#define ADC5120_CH4_CFG0		75 /*0x4B*/
#define ADC5120_CH4_CFG2		77 /*0x4D*/
#define ADC5120_CHX_INSRC_SHIFT		5
#define ADC5120_CHX_INPDM		2

#define ADC5120_GPIO_CFG0		33 /*0x21*/
#define ADC5120_GPIO1_DISABLE_SHIFT 4
#define ADC5120_GPIO1_CFG_MASK	(0x0F << ADC5120_GPIO1_DISABLE_SHIFT)
#define ADC5120_GPIO1_DISABLE	(0 << ADC5120_GPIO1_DISABLE_SHIFT)
#define ADC5120_GPO1_CFG		34 /*0x22*/
#define ADC5120_GPO2_CFG		35 /*0x23*/
#define ADC5120_GPO3_CFG		36 /*0x24*/
#define ADC5120_GPO4_CFG		37 /*0x25*/
#define ADC5120_PDMCLK_SHIFT		4
#define ADC5120_GPOX_PDMCLK		(4 << ADC5120_PDMCLK_SHIFT)
#define ADC5120_GPOX_ACTHL		1

#define ADC5120_GPI_CFG0		43 /* 0x2b*/
#define ADC5120_GPI_CFG1		44 /* 0x2c*/
#define ADC5120_GPIX_CFG_SHIFT		4
#define ADC5120_GPI1_PDMD_CH12		(4 << ADC5120_GPIX_CFG_SHIFT)
#define ADC5120_GPI2_PDMD_CH34		5
#define ADC5120_GPI3_PDMD_CH56		(6 << ADC5120_GPIX_CFG_SHIFT)
#define ADC5120_GPI4_PDMD_CH78		7

#define ADC5120_CH1_CFG1		61 /*0x3d*/
#define ADC5120_CH1_CFG2		62 /*0x3e*/
#define ADC5120_CH_VOLUME_9DB 219
#define ADC5120_CH2_CFG2		67 /*0x43*/
#define ADC5120_CH3_CFG2		72 /*0x48*/
#define ADC5120_CH4_CFG2		77 /*0x4d*/
#define ADC5120_CH5_CFG2		82 /*0x52*/
#define ADC5120_CH6_CFG2		87 /*0x57*/
#define ADC5120_CH7_CFG2		92 /*0x5c*/
#define ADC5120_CH8_CFG2		97 /*0x57*/

#define ADC5120_DSP_CFG1		0x6c
#define ADC5120_BIQUADS_SHIFT	5
#define ADC5120_BIQUADS_MASK	(3 << ADC5120_BIQUADS_SHIFT)
#define ADC5120_NO_BIQUADS		(0 << ADC5120_BIQUADS_SHIFT)
#define ADC5120_1_BIQUADS		(1 << ADC5120_BIQUADS_SHIFT)

#define ADC5120_INPUT_CHEN		115 /*0x73*/
#define ADC5120_ASIOUT_CHEN		116 /*0x74*/
#define ADC5120_CHEN_SHIFT		4
#define ADC5120_CHEN_MASK		(0xF << ADC5120_CHEN_SHIFT)
#define ADC5120_CHEN_ALL		ADC5120_CHEN_MASK
#define ADC5120_PWR_CFG		117 /*0x75*/
#define ADC5120_ADC_PDZ_SHIFT		6
#define ADC5120_ADC_PDZ_MASK		(1 << ADC5120_ADC_PDZ_SHIFT)
#define ADC5120_ADC_PDZ		(1 << ADC5120_ADC_PDZ_SHIFT)
#define ADC5120_PLL_PDZ_SHIFT		5
#define ADC5120_PLL_PDZ_MASK		(1 << ADC5120_PLL_PDZ_SHIFT)
#define ADC5120_PLL_PDZ		(1 << ADC5120_PLL_PDZ_SHIFT)

struct adc5120_priv {
   struct regmap *regmap;
};

#endif
