/*
<:copyright-BRCM:2021:DUAL/GPL:standard

   Copyright (c) 2021 Broadcom 
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


#ifndef __BCMDSL_H__
#define __BCMDSL_H__

#define BCMDSL_MAX_LINES			2
#define BCMDSL_INVALID_VALUE		0xFFFFFFFF
#define BCMDSL_DEFAULT_VALUE		0x0

#define BCMDSL_SUCCESS				0
#define BCMDSL_INVALID				1
#define BCMDSL_NOT_DEFINED			2
#define BCMDSL_SET_GPIO_FAIL		3

#define BCMDSL_VDSLCTL_0			0
#define BCMDSL_VDSLCTL_1			1
#define BCMDSL_VDSLCTL_2			2
#define BCMDSL_VDSLCTL_3			3
#define BCMDSL_VDSLCTL_4			4
#define BCMDSL_VDSLCTL_5			5

#ifndef BP_SUCCESS
#define BP_SUCCESS		BCMDSL_SUCCESS
#endif

int bcmdsl_get_afe_ids(uint32_t *afe_ids); /* get primiary afe id for both lines */
int bcmdsl_get_primary_afe_id(int line, uint32_t *afe_ids);
int bcmdsl_get_secondary_afe_id(int line, uint32_t *afe_ids);

int bcmdsl_get_afe_pwr_ctl(int line, uint16_t *pwr_ctl);
int bcmdsl_get_afe_data_ctl(int line, uint16_t *data_ctl);
int bcmdsl_get_afe_clk_ctl(int line, uint16_t *clk_ctl);
int bcmdsl_get_afe_mode_ctl(int line, uint16_t *mode_ctl);

int bcmdsl_set_relay_gpio(int line, int active);
int bcmdsl_set_reset_gpio(int line, int active);
int bcmdsl_set_vr5p3pwr_gpio(int line, int active);
int bcmdsl_set_pwrboost_gpio(int line, int active);
int bcmdsl_set_tddenable_gpio(int line, int active);

int bcmdsl_get_relay_gpio_num(int line, uint16_t* gpio);
int bcmdsl_get_reset_gpio_num(int line, uint16_t* gpio);
int bcmdsl_get_vr5p3pwr_gpio_num(int line, uint16_t* gpio);
int bcmdsl_get_pwrboost_gpio_num(int line, uint16_t* gpio);
int bcmdsl_get_tddenable_gpio_num(int line, uint16_t* gpio);

uintptr_t bcmdsl_get_phy_base(void);
uintptr_t bcmdsl_get_lmem_base(void);
uintptr_t bcmdsl_get_xmem_base(void);
int bcmdsl_get_irq(void);

/* AFE IDs */
#define BP_AFE_DEFAULT                  0
#define BP_AFE_CHIP_INT                 (1 << 28)
#define BP_AFE_CHIP_6505                (2 << 28)
#define BP_AFE_CHIP_6306                (3 << 28)
#define BP_AFE_CHIP_CH0                 (4 << 28)
#define BP_AFE_CHIP_CH1                 (5 << 28)
#define BP_AFE_CHIP_GFAST               (6 << 28)
#define BP_AFE_CHIP_GFAST0              (6 << 28)
#define BP_AFE_CHIP_GFAST_CH0           (7 << 28)
#define BP_AFE_CHIP_GFAST1              (8 << 28)
#define BP_AFE_CHIP_GFAST_CH1           (9 << 28)
#define BP_AFE_LD_ISIL1556              (1 << 21)
#define BP_AFE_LD_6301                  (2 << 21)
#define BP_AFE_LD_6302                  (3 << 21)
#define BP_AFE_LD_6303                  (4 << 21)
#define BP_AFE_LD_6304                  (5 << 21)
#define BP_AFE_LD_6305                  (6 << 21)
#define BP_AFE_LD_REV_6303_VR5P3        (1 << 18)
#define BP_AFE_FE_ANNEXA                (1 << 15)
#define BP_AFE_FE_ANNEXB                (2 << 15)
#define BP_AFE_FE_ANNEXJ                (3 << 15)
#define BP_AFE_FE_ANNEXBJ               (4 << 15)
#define BP_AFE_FE_ANNEXM                (5 << 15)
#define BP_AFE_FE_ANNEXC                (6 << 15)
#define BP_AFE_FE_AVMODE_COMBO          (0 << 13)
#define BP_AFE_FE_AVMODE_ADSL           (1 << 13)
#define BP_AFE_FE_AVMODE_VDSL           (2 << 13)
/* VDSL only */
#define BP_AFE_FE_REV_ISIL_REV1         (1 << 8)
#define BP_AFE_FE_REV_12_20             BP_AFE_FE_REV_ISIL_REV1
#define BP_AFE_FE_REV_12_21             (2 << 8)
/* Combo */
#define BP_AFE_FE_REV_6302_REV1         (1 << 8)
#define BP_AFE_FE_REV_6302_REV_7_12     (1 << 8)
#define BP_AFE_FE_REV_6302_REV_7_2_21   (2 << 8)
#define BP_AFE_FE_REV_6302_REV_7_2_1    (3 << 8)
#define BP_AFE_FE_REV_6302_REV_7_2      (4 << 8)
#define BP_AFE_FE_REV_6302_REV_7_2_UR2  (5 << 8)
#define BP_AFE_FE_REV_6302_REV_7_2_2    (6 << 8)
#define BP_AFE_FE_REV_6302_REV_7_2_30    (7 << 8)
#define BP_AFE_6302_6306_REV_A_12_40    (8 << 8)
#define BP_AFE_FE_REV_6303_REV_12_3_30    (9 << 8)
#define BP_AFE_FE_REV_6303_REV_12_3_20    (1 << 8)
#define BP_AFE_FE_REV_6303_REV_12_3_40    (1 << 8)
#define BP_AFE_FE_REV_6303_REV_12_3_60    (1 << 8)
#define BP_AFE_FE_REV_6303_REV_12_3_50    (2 << 8)
#define BP_AFE_FE_REV_6303_REV_12_3_35    (3 << 8)
#define BP_AFE_FE_REV_6303_REV_12_3_70    (3 << 8)
#define BP_AFE_FE_REV_6303_REV_12_3_75    (4 << 8)
#define BP_AFE_FE_REV_6303_REV_12_50      (1 << 8)
#define BP_AFE_FE_REV_6303_REV_12_51      (2 << 8)
#define BP_AFE_FE_REV_6304_REV_12_4_40      (1 << 8)
#define BP_AFE_FE_REV_6304_REV_12_4_45      (2 << 8)
#define BP_AFE_FE_REV_6304_REV_12_4_60      (1 << 8)
#define BP_AFE_FE_REV_6305_REV_12_5_60_1    (1 << 8)
#define BP_AFE_FE_REV_6305_REV_12_5_60_2    (2 << 8)
#define BP_AFE_FE_REV_6304_REV_12_4_80      (4 << 8)
#define BP_AFE_FE_REV_6305_REV_12_5_80      (4 << 8)
#define BP_AFE_FE_REV_6303_146__REV_12_3_80 (3 << 8)
#define BP_AFE_FE_REV_6303_146__REV_12_3_85 (4 << 8)
#define BP_AFE_FE_REV_6303_REV_12_3_82      (5 << 8)
#define BP_AFE_FE_REV_6303_REV_12_3_72      (8 << 8)
/* ADSL only*/
#define BP_AFE_FE_REV_6302_REV_5_2_1    (1 << 8)
#define BP_AFE_FE_REV_6302_REV_5_2_2    (2 << 8)
#define BP_AFE_FE_REV_6302_REV_5_2_3    (3 << 8)
#define BP_AFE_FE_REV_6301_REV_5_1_1    (1 << 8)
#define BP_AFE_FE_REV_6301_REV_5_1_2    (2 << 8)
#define BP_AFE_FE_REV_6301_REV_5_1_3    (3 << 8)
#define BP_AFE_FE_REV_6301_REV_5_1_4    (4 << 8)
#define BP_AFE_FE_COAX                  (1 << 7)
#define BP_AFE_FE_RNC                   (1 << 6)
#define BP_AFE_FE_8dBm                  (1 << 5)

#endif
