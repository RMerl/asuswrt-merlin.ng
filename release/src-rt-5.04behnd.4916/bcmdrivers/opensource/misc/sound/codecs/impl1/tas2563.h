/*
<:copyright-BRCM:2018:DUAL/GPL:standard

   Copyright (c) 2018 Broadcom 
   All Rights Reserved

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License, version 2, as published by
the Free Software Foundation (the "GPL").

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.


A copy of the GPL is available at http://www.broadcom.com/licenses/GPLv2.php, or by
writing to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
Boston, MA 02111-1307, USA.

:>
*/

#ifndef _TAS2563_H
#define _TAS2563_H

#define TAS2563_RATES    SNDRV_PCM_RATE_8000_96000
#define TAS2563_FORMATS  (SNDRV_PCM_FMTBIT_S16_LE | \
                          SNDRV_PCM_FMTBIT_S24_LE | \
                          SNDRV_PCM_FMTBIT_S32_LE)

#define TAS2563_PAGE_LEN					 128 
#define TAS2563_PAGE1_START					 TAS2563_PAGE_LEN
#define TAS2563_PAGE2_START					 TAS2563_PAGE_LEN *2 
/* page 0 */
#define TAS2563_PSEL                     0  /*reg 0*/
#define TAS2563_RESET                    1
#define TAS2563_PWR_CTL                  2  /* reg 2 */
#define TAS2563_PWR_MODE_MASK            0x03
#define TAS2563_PWR_MODE_ACTIVE          0
#define TAS2563_PWR_MODE_MUTE            1
#define TAS2563_PWR_MODE_SOFTDOWN        2
#define TAS2563_PB_CFG1                  3  /* reg 3 */
#define TAS2563_AMP_LEVEL_SHIFT          1
#define TAS2563_AMP_LEVEL_MASK           (0x1F << TAS2563_AMP_LEVEL_SHIFT)
#define TAS2563_AMP_LEVEL_9DB            (0x02 << TAS2563_AMP_LEVEL_SHIFT)
#define TAS2563_TDM_CFG0                 6 /* reg 6 */
#define TAS2563_FS_START_SHIFT		 0
#define TAS2563_FS_START_MASK		 (1 << TAS2563_FS_START_SHIFT)
#define TAS2563_FS_START_L2H		 0
#define TAS2563_TDM_CFG1                 7 /* reg 7 */
#define TAS2563_RX_OFFSET_SHIFT          1
#define TAS2563_RX_OFFSET_MASK           (0x1F << TAS2563_RX_OFFSET_SHIFT)
#define TAS2563_RX_OFFSET_1              (1 << TAS2563_RX_OFFSET_SHIFT)
#define TAS2563_TDM_CFG2                 8 /*reg 8 */
#define TAS2563_RX_WLEN_SHIFT            2
#define TAS2563_RX_WLEN_MASK             (0x03 << TAS2563_RX_WLEN_SHIFT)
#define TAS2563_RX_WLEN_32BITS           (0x03 << TAS2563_RX_WLEN_SHIFT)
#define TAS2563_RX_SCFG_SHIFT            4
#define TAS2563_RX_SCFG_MASK             (0x03 << TAS2563_RX_SCFG_SHIFT)
#define TAS2563_RX_SCFG_STEREO_MIX       (0x03 << TAS2563_RX_SCFG_SHIFT)
#define TAS2563_TG_CFG0                  63 /*reg 63 */
#define TAS2563_TONE_TRIGGER_SHIFT       4
#define TAS2563_TONE_TRIGGER_MASK        (2 << TAS2563_TONE_TRIGGER_SHIFT)
#define TAS2563_TONE_TRIGGER_SDIN        (1 << TAS2563_TONE_TRIGGER_SHIFT)

/*page 2*/
#define TAS2563_DVC_PCM_LEN			 	 4 /* 4 bytes */
#define TAS2563_DVC_PCM_0				 (TAS2563_PAGE2_START+12) /* reg 12*/

struct tas2563_priv {
   struct regmap *regmap;
};

#endif
