/*
   Copyright (c) 2015 Broadcom Corporation
   All Rights Reserved

    <:label-BRCM:2015:DUAL/GPL:standard

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

#include "ru.h"

#if RU_INCLUDE_FIELD_DB
/******************************************************************************
 * Field: XPORT_REG_XPORT_REVISION_RESERVED0
 ******************************************************************************/
const ru_field_rec XPORT_REG_XPORT_REVISION_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    XPORT_REG_XPORT_REVISION_RESERVED0_FIELD_MASK,
    0,
    XPORT_REG_XPORT_REVISION_RESERVED0_FIELD_WIDTH,
    XPORT_REG_XPORT_REVISION_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: XPORT_REG_XPORT_REVISION_XPORT_REV
 ******************************************************************************/
const ru_field_rec XPORT_REG_XPORT_REVISION_XPORT_REV_FIELD =
{
    "XPORT_REV",
#if RU_INCLUDE_DESC
    "",
    "XPORT revision.",
#endif
    XPORT_REG_XPORT_REVISION_XPORT_REV_FIELD_MASK,
    0,
    XPORT_REG_XPORT_REVISION_XPORT_REV_FIELD_WIDTH,
    XPORT_REG_XPORT_REVISION_XPORT_REV_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: XPORT_REG_LED_PWM_CNTRL_RESERVED0
 ******************************************************************************/
const ru_field_rec XPORT_REG_LED_PWM_CNTRL_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    XPORT_REG_LED_PWM_CNTRL_RESERVED0_FIELD_MASK,
    0,
    XPORT_REG_LED_PWM_CNTRL_RESERVED0_FIELD_WIDTH,
    XPORT_REG_LED_PWM_CNTRL_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_REG_LED_PWM_CNTRL_PWM_ENABLE
 ******************************************************************************/
const ru_field_rec XPORT_REG_LED_PWM_CNTRL_PWM_ENABLE_FIELD =
{
    "PWM_ENABLE",
#if RU_INCLUDE_DESC
    "",
    "When set LED intensity can be controlled using PWM.",
#endif
    XPORT_REG_LED_PWM_CNTRL_PWM_ENABLE_FIELD_MASK,
    0,
    XPORT_REG_LED_PWM_CNTRL_PWM_ENABLE_FIELD_WIDTH,
    XPORT_REG_LED_PWM_CNTRL_PWM_ENABLE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_REG_LED_INTENSITY_CNTRL_LED_ON_LOW
 ******************************************************************************/
const ru_field_rec XPORT_REG_LED_INTENSITY_CNTRL_LED_ON_LOW_FIELD =
{
    "LED_ON_LOW",
#if RU_INCLUDE_DESC
    "",
    "LED_ON_TIME PWM modulated ON (low) time. LED_ON_LOW and LED_ON_HIGH determine "
    "PWM duty cycle for the LED intensity. Expressed in 50us units.",
#endif
    XPORT_REG_LED_INTENSITY_CNTRL_LED_ON_LOW_FIELD_MASK,
    0,
    XPORT_REG_LED_INTENSITY_CNTRL_LED_ON_LOW_FIELD_WIDTH,
    XPORT_REG_LED_INTENSITY_CNTRL_LED_ON_LOW_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_REG_LED_INTENSITY_CNTRL_LED_ON_HIGH
 ******************************************************************************/
const ru_field_rec XPORT_REG_LED_INTENSITY_CNTRL_LED_ON_HIGH_FIELD =
{
    "LED_ON_HIGH",
#if RU_INCLUDE_DESC
    "",
    "LED_ON_TIME PWM modulated OFF (high) time. LED_ON_LOW and LED_ON_HIGH determine "
    "PWM duty cycle for the LED intensity. Expressed in 50us units.",
#endif
    XPORT_REG_LED_INTENSITY_CNTRL_LED_ON_HIGH_FIELD_MASK,
    0,
    XPORT_REG_LED_INTENSITY_CNTRL_LED_ON_HIGH_FIELD_WIDTH,
    XPORT_REG_LED_INTENSITY_CNTRL_LED_ON_HIGH_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_REG_LED_0_CNTRL_RESERVED0
 ******************************************************************************/
const ru_field_rec XPORT_REG_LED_0_CNTRL_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    XPORT_REG_LED_0_CNTRL_RESERVED0_FIELD_MASK,
    0,
    XPORT_REG_LED_0_CNTRL_RESERVED0_FIELD_WIDTH,
    XPORT_REG_LED_0_CNTRL_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_REG_LED_0_CNTRL_LNK_OVRD_EN
 ******************************************************************************/
const ru_field_rec XPORT_REG_LED_0_CNTRL_LNK_OVRD_EN_FIELD =
{
    "LNK_OVRD_EN",
#if RU_INCLUDE_DESC
    "",
    "When this bit is set MAC/PHY provided link indication is overridden using lnk_status_ovrd.",
#endif
    XPORT_REG_LED_0_CNTRL_LNK_OVRD_EN_FIELD_MASK,
    0,
    XPORT_REG_LED_0_CNTRL_LNK_OVRD_EN_FIELD_WIDTH,
    XPORT_REG_LED_0_CNTRL_LNK_OVRD_EN_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_REG_LED_0_CNTRL_SPD_OVRD_EN
 ******************************************************************************/
const ru_field_rec XPORT_REG_LED_0_CNTRL_SPD_OVRD_EN_FIELD =
{
    "SPD_OVRD_EN",
#if RU_INCLUDE_DESC
    "",
    "When this bit is set MAC/PHY provided speed indications are overridden using led_spd_ovrd[2:0].",
#endif
    XPORT_REG_LED_0_CNTRL_SPD_OVRD_EN_FIELD_MASK,
    0,
    XPORT_REG_LED_0_CNTRL_SPD_OVRD_EN_FIELD_WIDTH,
    XPORT_REG_LED_0_CNTRL_SPD_OVRD_EN_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_REG_LED_0_CNTRL_LNK_STATUS_OVRD
 ******************************************************************************/
const ru_field_rec XPORT_REG_LED_0_CNTRL_LNK_STATUS_OVRD_FIELD =
{
    "LNK_STATUS_OVRD",
#if RU_INCLUDE_DESC
    "",
    "Link status override. Used only for LED.",
#endif
    XPORT_REG_LED_0_CNTRL_LNK_STATUS_OVRD_FIELD_MASK,
    0,
    XPORT_REG_LED_0_CNTRL_LNK_STATUS_OVRD_FIELD_WIDTH,
    XPORT_REG_LED_0_CNTRL_LNK_STATUS_OVRD_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_REG_LED_0_CNTRL_LED_SPD_OVRD
 ******************************************************************************/
const ru_field_rec XPORT_REG_LED_0_CNTRL_LED_SPD_OVRD_FIELD =
{
    "LED_SPD_OVRD",
#if RU_INCLUDE_DESC
    "",
    "LED speed override. Default encoding is:\n"
    "000 : 10Mb/s.\n"
    "001 : 100Mb/s.\n"
    "010 : 1000Mb/s.\n"
    "011 : 2.5Gb/s.\n"
    "100 : 10Gb/s or higher.\n"
    "101 : Custom speed 1.\n"
    "110 : Custom speed 2.\n"
    "111 : no-link.\n"
    "Using this register LED speeds can be encoded in any way that suits customer application.",
#endif
    XPORT_REG_LED_0_CNTRL_LED_SPD_OVRD_FIELD_MASK,
    0,
    XPORT_REG_LED_0_CNTRL_LED_SPD_OVRD_FIELD_WIDTH,
    XPORT_REG_LED_0_CNTRL_LED_SPD_OVRD_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_REG_LED_0_CNTRL_ACT_LED_POL_SEL
 ******************************************************************************/
const ru_field_rec XPORT_REG_LED_0_CNTRL_ACT_LED_POL_SEL_FIELD =
{
    "ACT_LED_POL_SEL",
#if RU_INCLUDE_DESC
    "",
    "When set to 1'b1 inverts polarity of the activity signal that is used for ACT_LED.",
#endif
    XPORT_REG_LED_0_CNTRL_ACT_LED_POL_SEL_FIELD_MASK,
    0,
    XPORT_REG_LED_0_CNTRL_ACT_LED_POL_SEL_FIELD_WIDTH,
    XPORT_REG_LED_0_CNTRL_ACT_LED_POL_SEL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_REG_LED_0_CNTRL_SPDLNK_LED2_ACT_POL_SEL
 ******************************************************************************/
const ru_field_rec XPORT_REG_LED_0_CNTRL_SPDLNK_LED2_ACT_POL_SEL_FIELD =
{
    "SPDLNK_LED2_ACT_POL_SEL",
#if RU_INCLUDE_DESC
    "",
    "When set to 1'b1 inverts polarity of the activity signal that is used for SPDLNK_LED[2]. "
    "Applicable only when the activity drives this LED.",
#endif
    XPORT_REG_LED_0_CNTRL_SPDLNK_LED2_ACT_POL_SEL_FIELD_MASK,
    0,
    XPORT_REG_LED_0_CNTRL_SPDLNK_LED2_ACT_POL_SEL_FIELD_WIDTH,
    XPORT_REG_LED_0_CNTRL_SPDLNK_LED2_ACT_POL_SEL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_REG_LED_0_CNTRL_SPDLNK_LED1_ACT_POL_SEL
 ******************************************************************************/
const ru_field_rec XPORT_REG_LED_0_CNTRL_SPDLNK_LED1_ACT_POL_SEL_FIELD =
{
    "SPDLNK_LED1_ACT_POL_SEL",
#if RU_INCLUDE_DESC
    "",
    "When set to 1'b1 inverts polarity of the activity signal that is used for SPDLNK_LED[1]. "
    "Applicable only when the activity drives this LED.",
#endif
    XPORT_REG_LED_0_CNTRL_SPDLNK_LED1_ACT_POL_SEL_FIELD_MASK,
    0,
    XPORT_REG_LED_0_CNTRL_SPDLNK_LED1_ACT_POL_SEL_FIELD_WIDTH,
    XPORT_REG_LED_0_CNTRL_SPDLNK_LED1_ACT_POL_SEL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_REG_LED_0_CNTRL_SPDLNK_LED0_ACT_POL_SEL
 ******************************************************************************/
const ru_field_rec XPORT_REG_LED_0_CNTRL_SPDLNK_LED0_ACT_POL_SEL_FIELD =
{
    "SPDLNK_LED0_ACT_POL_SEL",
#if RU_INCLUDE_DESC
    "",
    "When set to 1'b1 inverts polarity of the activity signal that is used for SPDLNK_LED[0]. "
    "Applicable only when the activity drives this LED.",
#endif
    XPORT_REG_LED_0_CNTRL_SPDLNK_LED0_ACT_POL_SEL_FIELD_MASK,
    0,
    XPORT_REG_LED_0_CNTRL_SPDLNK_LED0_ACT_POL_SEL_FIELD_WIDTH,
    XPORT_REG_LED_0_CNTRL_SPDLNK_LED0_ACT_POL_SEL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_REG_LED_0_CNTRL_ACT_LED_ACT_SEL
 ******************************************************************************/
const ru_field_rec XPORT_REG_LED_0_CNTRL_ACT_LED_ACT_SEL_FIELD =
{
    "ACT_LED_ACT_SEL",
#if RU_INCLUDE_DESC
    "",
    "Selects source of activity for ACT_LED. For encoding see description for spdlnk_led0_act_sel.",
#endif
    XPORT_REG_LED_0_CNTRL_ACT_LED_ACT_SEL_FIELD_MASK,
    0,
    XPORT_REG_LED_0_CNTRL_ACT_LED_ACT_SEL_FIELD_WIDTH,
    XPORT_REG_LED_0_CNTRL_ACT_LED_ACT_SEL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_REG_LED_0_CNTRL_SPDLNK_LED2_ACT_SEL
 ******************************************************************************/
const ru_field_rec XPORT_REG_LED_0_CNTRL_SPDLNK_LED2_ACT_SEL_FIELD =
{
    "SPDLNK_LED2_ACT_SEL",
#if RU_INCLUDE_DESC
    "",
    "Selects source of activity for SPDLNK_LED[2]. For encoding see description for spdlnk_led0_act_sel.",
#endif
    XPORT_REG_LED_0_CNTRL_SPDLNK_LED2_ACT_SEL_FIELD_MASK,
    0,
    XPORT_REG_LED_0_CNTRL_SPDLNK_LED2_ACT_SEL_FIELD_WIDTH,
    XPORT_REG_LED_0_CNTRL_SPDLNK_LED2_ACT_SEL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_REG_LED_0_CNTRL_SPDLNK_LED1_ACT_SEL
 ******************************************************************************/
const ru_field_rec XPORT_REG_LED_0_CNTRL_SPDLNK_LED1_ACT_SEL_FIELD =
{
    "SPDLNK_LED1_ACT_SEL",
#if RU_INCLUDE_DESC
    "",
    "Selects source of activity for SPDLNK_LED[1]. For encoding see description for spdlnk_led0_act_sel.",
#endif
    XPORT_REG_LED_0_CNTRL_SPDLNK_LED1_ACT_SEL_FIELD_MASK,
    0,
    XPORT_REG_LED_0_CNTRL_SPDLNK_LED1_ACT_SEL_FIELD_WIDTH,
    XPORT_REG_LED_0_CNTRL_SPDLNK_LED1_ACT_SEL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_REG_LED_0_CNTRL_SPDLNK_LED0_ACT_SEL
 ******************************************************************************/
const ru_field_rec XPORT_REG_LED_0_CNTRL_SPDLNK_LED0_ACT_SEL_FIELD =
{
    "SPDLNK_LED0_ACT_SEL",
#if RU_INCLUDE_DESC
    "",
    "Selects source of the activity for SPDLNK_LED[0]:\n"
    "0 : LED is 0 when link is up and blinks when there is activity.\n"
    "1 : LED is 1 and blinks when there is activity.",
#endif
    XPORT_REG_LED_0_CNTRL_SPDLNK_LED0_ACT_SEL_FIELD_MASK,
    0,
    XPORT_REG_LED_0_CNTRL_SPDLNK_LED0_ACT_SEL_FIELD_WIDTH,
    XPORT_REG_LED_0_CNTRL_SPDLNK_LED0_ACT_SEL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_REG_LED_0_CNTRL_TX_ACT_EN
 ******************************************************************************/
const ru_field_rec XPORT_REG_LED_0_CNTRL_TX_ACT_EN_FIELD =
{
    "TX_ACT_EN",
#if RU_INCLUDE_DESC
    "",
    "Enables TX_SOP event to contribute to the activity.",
#endif
    XPORT_REG_LED_0_CNTRL_TX_ACT_EN_FIELD_MASK,
    0,
    XPORT_REG_LED_0_CNTRL_TX_ACT_EN_FIELD_WIDTH,
    XPORT_REG_LED_0_CNTRL_TX_ACT_EN_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_REG_LED_0_CNTRL_RX_ACT_EN
 ******************************************************************************/
const ru_field_rec XPORT_REG_LED_0_CNTRL_RX_ACT_EN_FIELD =
{
    "RX_ACT_EN",
#if RU_INCLUDE_DESC
    "",
    "Enables RX_SOP event to contribute to the activity.",
#endif
    XPORT_REG_LED_0_CNTRL_RX_ACT_EN_FIELD_MASK,
    0,
    XPORT_REG_LED_0_CNTRL_RX_ACT_EN_FIELD_WIDTH,
    XPORT_REG_LED_0_CNTRL_RX_ACT_EN_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_REG_LED_0_LINK_AND_SPEED_ENCODING_SEL_RESERVED0
 ******************************************************************************/
const ru_field_rec XPORT_REG_LED_0_LINK_AND_SPEED_ENCODING_SEL_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    XPORT_REG_LED_0_LINK_AND_SPEED_ENCODING_SEL_RESERVED0_FIELD_MASK,
    0,
    XPORT_REG_LED_0_LINK_AND_SPEED_ENCODING_SEL_RESERVED0_FIELD_WIDTH,
    XPORT_REG_LED_0_LINK_AND_SPEED_ENCODING_SEL_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_REG_LED_0_LINK_AND_SPEED_ENCODING_SEL_RSVD_SEL_SPD_ENCODE_2
 ******************************************************************************/
const ru_field_rec XPORT_REG_LED_0_LINK_AND_SPEED_ENCODING_SEL_RSVD_SEL_SPD_ENCODE_2_FIELD =
{
    "RSVD_SEL_SPD_ENCODE_2",
#if RU_INCLUDE_DESC
    "",
    "Reserved SPDLNK_LED_SEL[2:0] encoding select.",
#endif
    XPORT_REG_LED_0_LINK_AND_SPEED_ENCODING_SEL_RSVD_SEL_SPD_ENCODE_2_FIELD_MASK,
    0,
    XPORT_REG_LED_0_LINK_AND_SPEED_ENCODING_SEL_RSVD_SEL_SPD_ENCODE_2_FIELD_WIDTH,
    XPORT_REG_LED_0_LINK_AND_SPEED_ENCODING_SEL_RSVD_SEL_SPD_ENCODE_2_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_REG_LED_0_LINK_AND_SPEED_ENCODING_SEL_RSVD_SEL_SPD_ENCODE_1
 ******************************************************************************/
const ru_field_rec XPORT_REG_LED_0_LINK_AND_SPEED_ENCODING_SEL_RSVD_SEL_SPD_ENCODE_1_FIELD =
{
    "RSVD_SEL_SPD_ENCODE_1",
#if RU_INCLUDE_DESC
    "",
    "Reserved SPDLNK_LED_SEL[2:0] encoding select.",
#endif
    XPORT_REG_LED_0_LINK_AND_SPEED_ENCODING_SEL_RSVD_SEL_SPD_ENCODE_1_FIELD_MASK,
    0,
    XPORT_REG_LED_0_LINK_AND_SPEED_ENCODING_SEL_RSVD_SEL_SPD_ENCODE_1_FIELD_WIDTH,
    XPORT_REG_LED_0_LINK_AND_SPEED_ENCODING_SEL_RSVD_SEL_SPD_ENCODE_1_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_REG_LED_0_LINK_AND_SPEED_ENCODING_SEL_SEL_10G_ENCODE
 ******************************************************************************/
const ru_field_rec XPORT_REG_LED_0_LINK_AND_SPEED_ENCODING_SEL_SEL_10G_ENCODE_FIELD =
{
    "SEL_10G_ENCODE",
#if RU_INCLUDE_DESC
    "",
    "SPDLNK_LED_SEL[2:0] encoding for 10Gb/s and higher link speed.\n"
    "When SPDLNK_LED_SEL[x] = 1'''b0, SPDLNK_LED[x] is driven by bits [17:0] of Link and Speed Encoding Register.\n"
    "When SPDLNK_LED_SEL[x] = 1'''b1, SPDLNK_LED[x] is driven by the activity.\n",
#endif
    XPORT_REG_LED_0_LINK_AND_SPEED_ENCODING_SEL_SEL_10G_ENCODE_FIELD_MASK,
    0,
    XPORT_REG_LED_0_LINK_AND_SPEED_ENCODING_SEL_SEL_10G_ENCODE_FIELD_WIDTH,
    XPORT_REG_LED_0_LINK_AND_SPEED_ENCODING_SEL_SEL_10G_ENCODE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_REG_LED_0_LINK_AND_SPEED_ENCODING_SEL_SEL_2500M_ENCODE
 ******************************************************************************/
const ru_field_rec XPORT_REG_LED_0_LINK_AND_SPEED_ENCODING_SEL_SEL_2500M_ENCODE_FIELD =
{
    "SEL_2500M_ENCODE",
#if RU_INCLUDE_DESC
    "",
    "SPDLNK_LED_SEL[2:0] encoding for 2500Mb/s link speed.",
#endif
    XPORT_REG_LED_0_LINK_AND_SPEED_ENCODING_SEL_SEL_2500M_ENCODE_FIELD_MASK,
    0,
    XPORT_REG_LED_0_LINK_AND_SPEED_ENCODING_SEL_SEL_2500M_ENCODE_FIELD_WIDTH,
    XPORT_REG_LED_0_LINK_AND_SPEED_ENCODING_SEL_SEL_2500M_ENCODE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_REG_LED_0_LINK_AND_SPEED_ENCODING_SEL_SEL_1000M_ENCODE
 ******************************************************************************/
const ru_field_rec XPORT_REG_LED_0_LINK_AND_SPEED_ENCODING_SEL_SEL_1000M_ENCODE_FIELD =
{
    "SEL_1000M_ENCODE",
#if RU_INCLUDE_DESC
    "",
    "SPDLNK_LED_SEL[2:0] encoding for 1000Mb/s link speed.",
#endif
    XPORT_REG_LED_0_LINK_AND_SPEED_ENCODING_SEL_SEL_1000M_ENCODE_FIELD_MASK,
    0,
    XPORT_REG_LED_0_LINK_AND_SPEED_ENCODING_SEL_SEL_1000M_ENCODE_FIELD_WIDTH,
    XPORT_REG_LED_0_LINK_AND_SPEED_ENCODING_SEL_SEL_1000M_ENCODE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_REG_LED_0_LINK_AND_SPEED_ENCODING_SEL_SEL_100M_ENCODE
 ******************************************************************************/
const ru_field_rec XPORT_REG_LED_0_LINK_AND_SPEED_ENCODING_SEL_SEL_100M_ENCODE_FIELD =
{
    "SEL_100M_ENCODE",
#if RU_INCLUDE_DESC
    "",
    "SPDLNK_LED_SEL[2:0] encoding for 100Mb/s link speed.",
#endif
    XPORT_REG_LED_0_LINK_AND_SPEED_ENCODING_SEL_SEL_100M_ENCODE_FIELD_MASK,
    0,
    XPORT_REG_LED_0_LINK_AND_SPEED_ENCODING_SEL_SEL_100M_ENCODE_FIELD_WIDTH,
    XPORT_REG_LED_0_LINK_AND_SPEED_ENCODING_SEL_SEL_100M_ENCODE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_REG_LED_0_LINK_AND_SPEED_ENCODING_SEL_SEL_10M_ENCODE
 ******************************************************************************/
const ru_field_rec XPORT_REG_LED_0_LINK_AND_SPEED_ENCODING_SEL_SEL_10M_ENCODE_FIELD =
{
    "SEL_10M_ENCODE",
#if RU_INCLUDE_DESC
    "",
    "SPDLNK_LED_SEL[2:0] encoding for 10Mb/s link speed.",
#endif
    XPORT_REG_LED_0_LINK_AND_SPEED_ENCODING_SEL_SEL_10M_ENCODE_FIELD_MASK,
    0,
    XPORT_REG_LED_0_LINK_AND_SPEED_ENCODING_SEL_SEL_10M_ENCODE_FIELD_WIDTH,
    XPORT_REG_LED_0_LINK_AND_SPEED_ENCODING_SEL_SEL_10M_ENCODE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_REG_LED_0_LINK_AND_SPEED_ENCODING_SEL_SEL_NO_LINK_ENCODE
 ******************************************************************************/
const ru_field_rec XPORT_REG_LED_0_LINK_AND_SPEED_ENCODING_SEL_SEL_NO_LINK_ENCODE_FIELD =
{
    "SEL_NO_LINK_ENCODE",
#if RU_INCLUDE_DESC
    "",
    "SPDLNK_LED_SEL[2:0] encoding for the no-link state.",
#endif
    XPORT_REG_LED_0_LINK_AND_SPEED_ENCODING_SEL_SEL_NO_LINK_ENCODE_FIELD_MASK,
    0,
    XPORT_REG_LED_0_LINK_AND_SPEED_ENCODING_SEL_SEL_NO_LINK_ENCODE_FIELD_WIDTH,
    XPORT_REG_LED_0_LINK_AND_SPEED_ENCODING_SEL_SEL_NO_LINK_ENCODE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_REG_LED_0_LINK_AND_SPEED_ENCODING_RESERVED0
 ******************************************************************************/
const ru_field_rec XPORT_REG_LED_0_LINK_AND_SPEED_ENCODING_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    XPORT_REG_LED_0_LINK_AND_SPEED_ENCODING_RESERVED0_FIELD_MASK,
    0,
    XPORT_REG_LED_0_LINK_AND_SPEED_ENCODING_RESERVED0_FIELD_WIDTH,
    XPORT_REG_LED_0_LINK_AND_SPEED_ENCODING_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_REG_LED_0_LINK_AND_SPEED_ENCODING_RSVD_SPD_ENCODE_2
 ******************************************************************************/
const ru_field_rec XPORT_REG_LED_0_LINK_AND_SPEED_ENCODING_RSVD_SPD_ENCODE_2_FIELD =
{
    "RSVD_SPD_ENCODE_2",
#if RU_INCLUDE_DESC
    "",
    "Reserved SPDLNK_LED_SEL[2:0] encoding.",
#endif
    XPORT_REG_LED_0_LINK_AND_SPEED_ENCODING_RSVD_SPD_ENCODE_2_FIELD_MASK,
    0,
    XPORT_REG_LED_0_LINK_AND_SPEED_ENCODING_RSVD_SPD_ENCODE_2_FIELD_WIDTH,
    XPORT_REG_LED_0_LINK_AND_SPEED_ENCODING_RSVD_SPD_ENCODE_2_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_REG_LED_0_LINK_AND_SPEED_ENCODING_RSVD_SPD_ENCODE_1
 ******************************************************************************/
const ru_field_rec XPORT_REG_LED_0_LINK_AND_SPEED_ENCODING_RSVD_SPD_ENCODE_1_FIELD =
{
    "RSVD_SPD_ENCODE_1",
#if RU_INCLUDE_DESC
    "",
    "Reserved SPDLNK_LED_SEL[2:0] encoding.",
#endif
    XPORT_REG_LED_0_LINK_AND_SPEED_ENCODING_RSVD_SPD_ENCODE_1_FIELD_MASK,
    0,
    XPORT_REG_LED_0_LINK_AND_SPEED_ENCODING_RSVD_SPD_ENCODE_1_FIELD_WIDTH,
    XPORT_REG_LED_0_LINK_AND_SPEED_ENCODING_RSVD_SPD_ENCODE_1_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_REG_LED_0_LINK_AND_SPEED_ENCODING_M10G_ENCODE
 ******************************************************************************/
const ru_field_rec XPORT_REG_LED_0_LINK_AND_SPEED_ENCODING_M10G_ENCODE_FIELD =
{
    "M10G_ENCODE",
#if RU_INCLUDE_DESC
    "",
    "SPDLNK_LED[2:0] encoding for 10Gb/s and higherlink speed.",
#endif
    XPORT_REG_LED_0_LINK_AND_SPEED_ENCODING_M10G_ENCODE_FIELD_MASK,
    0,
    XPORT_REG_LED_0_LINK_AND_SPEED_ENCODING_M10G_ENCODE_FIELD_WIDTH,
    XPORT_REG_LED_0_LINK_AND_SPEED_ENCODING_M10G_ENCODE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_REG_LED_0_LINK_AND_SPEED_ENCODING_M2500_ENCODE
 ******************************************************************************/
const ru_field_rec XPORT_REG_LED_0_LINK_AND_SPEED_ENCODING_M2500_ENCODE_FIELD =
{
    "M2500_ENCODE",
#if RU_INCLUDE_DESC
    "",
    "SPDLNK_LED[2:0] encoding for 2.5Gb/s link speed.",
#endif
    XPORT_REG_LED_0_LINK_AND_SPEED_ENCODING_M2500_ENCODE_FIELD_MASK,
    0,
    XPORT_REG_LED_0_LINK_AND_SPEED_ENCODING_M2500_ENCODE_FIELD_WIDTH,
    XPORT_REG_LED_0_LINK_AND_SPEED_ENCODING_M2500_ENCODE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_REG_LED_0_LINK_AND_SPEED_ENCODING_M1000_ENCODE
 ******************************************************************************/
const ru_field_rec XPORT_REG_LED_0_LINK_AND_SPEED_ENCODING_M1000_ENCODE_FIELD =
{
    "M1000_ENCODE",
#if RU_INCLUDE_DESC
    "",
    "SPDLNK_LED[2:0] encoding for 1Gb/s link speed.",
#endif
    XPORT_REG_LED_0_LINK_AND_SPEED_ENCODING_M1000_ENCODE_FIELD_MASK,
    0,
    XPORT_REG_LED_0_LINK_AND_SPEED_ENCODING_M1000_ENCODE_FIELD_WIDTH,
    XPORT_REG_LED_0_LINK_AND_SPEED_ENCODING_M1000_ENCODE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_REG_LED_0_LINK_AND_SPEED_ENCODING_M100_ENCODE
 ******************************************************************************/
const ru_field_rec XPORT_REG_LED_0_LINK_AND_SPEED_ENCODING_M100_ENCODE_FIELD =
{
    "M100_ENCODE",
#if RU_INCLUDE_DESC
    "",
    "SPDLNK_LED[2:0] encoding for 100Mb/s link speed.",
#endif
    XPORT_REG_LED_0_LINK_AND_SPEED_ENCODING_M100_ENCODE_FIELD_MASK,
    0,
    XPORT_REG_LED_0_LINK_AND_SPEED_ENCODING_M100_ENCODE_FIELD_WIDTH,
    XPORT_REG_LED_0_LINK_AND_SPEED_ENCODING_M100_ENCODE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_REG_LED_0_LINK_AND_SPEED_ENCODING_M10_ENCODE
 ******************************************************************************/
const ru_field_rec XPORT_REG_LED_0_LINK_AND_SPEED_ENCODING_M10_ENCODE_FIELD =
{
    "M10_ENCODE",
#if RU_INCLUDE_DESC
    "",
    "SPDLNK_LED[2:0] encoding for 10Mb/s link speed.",
#endif
    XPORT_REG_LED_0_LINK_AND_SPEED_ENCODING_M10_ENCODE_FIELD_MASK,
    0,
    XPORT_REG_LED_0_LINK_AND_SPEED_ENCODING_M10_ENCODE_FIELD_WIDTH,
    XPORT_REG_LED_0_LINK_AND_SPEED_ENCODING_M10_ENCODE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_REG_LED_0_LINK_AND_SPEED_ENCODING_NO_LINK_ENCODE
 ******************************************************************************/
const ru_field_rec XPORT_REG_LED_0_LINK_AND_SPEED_ENCODING_NO_LINK_ENCODE_FIELD =
{
    "NO_LINK_ENCODE",
#if RU_INCLUDE_DESC
    "",
    "SPDLNK_LED[2:0] encoding for the no-link state.",
#endif
    XPORT_REG_LED_0_LINK_AND_SPEED_ENCODING_NO_LINK_ENCODE_FIELD_MASK,
    0,
    XPORT_REG_LED_0_LINK_AND_SPEED_ENCODING_NO_LINK_ENCODE_FIELD_WIDTH,
    XPORT_REG_LED_0_LINK_AND_SPEED_ENCODING_NO_LINK_ENCODE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_REG_LED_1_CNTRL_RESERVED0
 ******************************************************************************/
const ru_field_rec XPORT_REG_LED_1_CNTRL_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    XPORT_REG_LED_1_CNTRL_RESERVED0_FIELD_MASK,
    0,
    XPORT_REG_LED_1_CNTRL_RESERVED0_FIELD_WIDTH,
    XPORT_REG_LED_1_CNTRL_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_REG_LED_1_CNTRL_LNK_OVRD_EN
 ******************************************************************************/
const ru_field_rec XPORT_REG_LED_1_CNTRL_LNK_OVRD_EN_FIELD =
{
    "LNK_OVRD_EN",
#if RU_INCLUDE_DESC
    "",
    "When this bit is set MAC/PHY provided link indication is overridden using lnk_status_ovrd.",
#endif
    XPORT_REG_LED_1_CNTRL_LNK_OVRD_EN_FIELD_MASK,
    0,
    XPORT_REG_LED_1_CNTRL_LNK_OVRD_EN_FIELD_WIDTH,
    XPORT_REG_LED_1_CNTRL_LNK_OVRD_EN_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_REG_LED_1_CNTRL_SPD_OVRD_EN
 ******************************************************************************/
const ru_field_rec XPORT_REG_LED_1_CNTRL_SPD_OVRD_EN_FIELD =
{
    "SPD_OVRD_EN",
#if RU_INCLUDE_DESC
    "",
    "When this bit is set MAC/PHY provided speed indications are overridden using led_spd_ovrd[2:0].",
#endif
    XPORT_REG_LED_1_CNTRL_SPD_OVRD_EN_FIELD_MASK,
    0,
    XPORT_REG_LED_1_CNTRL_SPD_OVRD_EN_FIELD_WIDTH,
    XPORT_REG_LED_1_CNTRL_SPD_OVRD_EN_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_REG_LED_1_CNTRL_LNK_STATUS_OVRD
 ******************************************************************************/
const ru_field_rec XPORT_REG_LED_1_CNTRL_LNK_STATUS_OVRD_FIELD =
{
    "LNK_STATUS_OVRD",
#if RU_INCLUDE_DESC
    "",
    "Link status override. Used only for LED.",
#endif
    XPORT_REG_LED_1_CNTRL_LNK_STATUS_OVRD_FIELD_MASK,
    0,
    XPORT_REG_LED_1_CNTRL_LNK_STATUS_OVRD_FIELD_WIDTH,
    XPORT_REG_LED_1_CNTRL_LNK_STATUS_OVRD_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_REG_LED_1_CNTRL_LED_SPD_OVRD
 ******************************************************************************/
const ru_field_rec XPORT_REG_LED_1_CNTRL_LED_SPD_OVRD_FIELD =
{
    "LED_SPD_OVRD",
#if RU_INCLUDE_DESC
    "",
    "LED speed override. Default encoding is:\n"
    "000 : 10Mb/s.\n"
    "001 : 100Mb/s.\n"
    "010 : 1000Mb/s.\n"
    "011 : 2.5Gb/s.\n"
    "100 : 10Gb/s or higher.\n"
    "101 : Custom speed 1.\n"
    "110 : Custom speed 2.\n"
    "111 : no-link.\n"
    "Using this register LED speeds can be encoded in any way that suits customer application.",
#endif
    XPORT_REG_LED_1_CNTRL_LED_SPD_OVRD_FIELD_MASK,
    0,
    XPORT_REG_LED_1_CNTRL_LED_SPD_OVRD_FIELD_WIDTH,
    XPORT_REG_LED_1_CNTRL_LED_SPD_OVRD_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_REG_LED_1_CNTRL_ACT_LED_POL_SEL
 ******************************************************************************/
const ru_field_rec XPORT_REG_LED_1_CNTRL_ACT_LED_POL_SEL_FIELD =
{
    "ACT_LED_POL_SEL",
#if RU_INCLUDE_DESC
    "",
    "When set to 1'b1 inverts polarity of the activity signal that is used for ACT_LED.",
#endif
    XPORT_REG_LED_1_CNTRL_ACT_LED_POL_SEL_FIELD_MASK,
    0,
    XPORT_REG_LED_1_CNTRL_ACT_LED_POL_SEL_FIELD_WIDTH,
    XPORT_REG_LED_1_CNTRL_ACT_LED_POL_SEL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_REG_LED_1_CNTRL_SPDLNK_LED2_ACT_POL_SEL
 ******************************************************************************/
const ru_field_rec XPORT_REG_LED_1_CNTRL_SPDLNK_LED2_ACT_POL_SEL_FIELD =
{
    "SPDLNK_LED2_ACT_POL_SEL",
#if RU_INCLUDE_DESC
    "",
    "When set to 1'b1 inverts polarity of the activity signal that is used for SPDLNK_LED[2]. "
    "Applicable only when the activity drives this LED.",
#endif
    XPORT_REG_LED_1_CNTRL_SPDLNK_LED2_ACT_POL_SEL_FIELD_MASK,
    0,
    XPORT_REG_LED_1_CNTRL_SPDLNK_LED2_ACT_POL_SEL_FIELD_WIDTH,
    XPORT_REG_LED_1_CNTRL_SPDLNK_LED2_ACT_POL_SEL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_REG_LED_1_CNTRL_SPDLNK_LED1_ACT_POL_SEL
 ******************************************************************************/
const ru_field_rec XPORT_REG_LED_1_CNTRL_SPDLNK_LED1_ACT_POL_SEL_FIELD =
{
    "SPDLNK_LED1_ACT_POL_SEL",
#if RU_INCLUDE_DESC
    "",
    "When set to 1'b1 inverts polarity of the activity signal that is used for SPDLNK_LED[1]. "
    "Applicable only when the activity drives this LED.",
#endif
    XPORT_REG_LED_1_CNTRL_SPDLNK_LED1_ACT_POL_SEL_FIELD_MASK,
    0,
    XPORT_REG_LED_1_CNTRL_SPDLNK_LED1_ACT_POL_SEL_FIELD_WIDTH,
    XPORT_REG_LED_1_CNTRL_SPDLNK_LED1_ACT_POL_SEL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_REG_LED_1_CNTRL_SPDLNK_LED0_ACT_POL_SEL
 ******************************************************************************/
const ru_field_rec XPORT_REG_LED_1_CNTRL_SPDLNK_LED0_ACT_POL_SEL_FIELD =
{
    "SPDLNK_LED0_ACT_POL_SEL",
#if RU_INCLUDE_DESC
    "",
    "When set to 1'b1 inverts polarity of the activity signal that is used for SPDLNK_LED[0]. "
    "Applicable only when the activity drives this LED.",
#endif
    XPORT_REG_LED_1_CNTRL_SPDLNK_LED0_ACT_POL_SEL_FIELD_MASK,
    0,
    XPORT_REG_LED_1_CNTRL_SPDLNK_LED0_ACT_POL_SEL_FIELD_WIDTH,
    XPORT_REG_LED_1_CNTRL_SPDLNK_LED0_ACT_POL_SEL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_REG_LED_1_CNTRL_ACT_LED_ACT_SEL
 ******************************************************************************/
const ru_field_rec XPORT_REG_LED_1_CNTRL_ACT_LED_ACT_SEL_FIELD =
{
    "ACT_LED_ACT_SEL",
#if RU_INCLUDE_DESC
    "",
    "Selects source of activity for ACT_LED. For encoding see description for spdlnk_led0_act_sel.",
#endif
    XPORT_REG_LED_1_CNTRL_ACT_LED_ACT_SEL_FIELD_MASK,
    0,
    XPORT_REG_LED_1_CNTRL_ACT_LED_ACT_SEL_FIELD_WIDTH,
    XPORT_REG_LED_1_CNTRL_ACT_LED_ACT_SEL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_REG_LED_1_CNTRL_SPDLNK_LED2_ACT_SEL
 ******************************************************************************/
const ru_field_rec XPORT_REG_LED_1_CNTRL_SPDLNK_LED2_ACT_SEL_FIELD =
{
    "SPDLNK_LED2_ACT_SEL",
#if RU_INCLUDE_DESC
    "",
    "Selects source of activity for SPDLNK_LED[2]. For encoding see description for spdlnk_led0_act_sel.",
#endif
    XPORT_REG_LED_1_CNTRL_SPDLNK_LED2_ACT_SEL_FIELD_MASK,
    0,
    XPORT_REG_LED_1_CNTRL_SPDLNK_LED2_ACT_SEL_FIELD_WIDTH,
    XPORT_REG_LED_1_CNTRL_SPDLNK_LED2_ACT_SEL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_REG_LED_1_CNTRL_SPDLNK_LED1_ACT_SEL
 ******************************************************************************/
const ru_field_rec XPORT_REG_LED_1_CNTRL_SPDLNK_LED1_ACT_SEL_FIELD =
{
    "SPDLNK_LED1_ACT_SEL",
#if RU_INCLUDE_DESC
    "",
    "Selects source of activity for SPDLNK_LED[1]. For encoding see description for spdlnk_led0_act_sel.",
#endif
    XPORT_REG_LED_1_CNTRL_SPDLNK_LED1_ACT_SEL_FIELD_MASK,
    0,
    XPORT_REG_LED_1_CNTRL_SPDLNK_LED1_ACT_SEL_FIELD_WIDTH,
    XPORT_REG_LED_1_CNTRL_SPDLNK_LED1_ACT_SEL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_REG_LED_1_CNTRL_SPDLNK_LED0_ACT_SEL
 ******************************************************************************/
const ru_field_rec XPORT_REG_LED_1_CNTRL_SPDLNK_LED0_ACT_SEL_FIELD =
{
    "SPDLNK_LED0_ACT_SEL",
#if RU_INCLUDE_DESC
    "",
    "Selects source of the activity for SPDLNK_LED[0]:\n"
    "0 : LED is 0 when link is up and blinks when there is activity.\n"
    "1 : LED is 1 and blinks when there is activity.",
#endif
    XPORT_REG_LED_1_CNTRL_SPDLNK_LED0_ACT_SEL_FIELD_MASK,
    0,
    XPORT_REG_LED_1_CNTRL_SPDLNK_LED0_ACT_SEL_FIELD_WIDTH,
    XPORT_REG_LED_1_CNTRL_SPDLNK_LED0_ACT_SEL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_REG_LED_1_CNTRL_TX_ACT_EN
 ******************************************************************************/
const ru_field_rec XPORT_REG_LED_1_CNTRL_TX_ACT_EN_FIELD =
{
    "TX_ACT_EN",
#if RU_INCLUDE_DESC
    "",
    "Enables TX_SOP event to contribute to the activity.",
#endif
    XPORT_REG_LED_1_CNTRL_TX_ACT_EN_FIELD_MASK,
    0,
    XPORT_REG_LED_1_CNTRL_TX_ACT_EN_FIELD_WIDTH,
    XPORT_REG_LED_1_CNTRL_TX_ACT_EN_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_REG_LED_1_CNTRL_RX_ACT_EN
 ******************************************************************************/
const ru_field_rec XPORT_REG_LED_1_CNTRL_RX_ACT_EN_FIELD =
{
    "RX_ACT_EN",
#if RU_INCLUDE_DESC
    "",
    "Enables RX_SOP event to contribute to the activity.",
#endif
    XPORT_REG_LED_1_CNTRL_RX_ACT_EN_FIELD_MASK,
    0,
    XPORT_REG_LED_1_CNTRL_RX_ACT_EN_FIELD_WIDTH,
    XPORT_REG_LED_1_CNTRL_RX_ACT_EN_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_REG_LED_1_LINK_AND_SPEED_ENCODING_SEL_RESERVED0
 ******************************************************************************/
const ru_field_rec XPORT_REG_LED_1_LINK_AND_SPEED_ENCODING_SEL_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    XPORT_REG_LED_1_LINK_AND_SPEED_ENCODING_SEL_RESERVED0_FIELD_MASK,
    0,
    XPORT_REG_LED_1_LINK_AND_SPEED_ENCODING_SEL_RESERVED0_FIELD_WIDTH,
    XPORT_REG_LED_1_LINK_AND_SPEED_ENCODING_SEL_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_REG_LED_1_LINK_AND_SPEED_ENCODING_SEL_RSVD_SEL_SPD_ENCODE_2
 ******************************************************************************/
const ru_field_rec XPORT_REG_LED_1_LINK_AND_SPEED_ENCODING_SEL_RSVD_SEL_SPD_ENCODE_2_FIELD =
{
    "RSVD_SEL_SPD_ENCODE_2",
#if RU_INCLUDE_DESC
    "",
    "Reserved SPDLNK_LED_SEL[2:0] encoding select.",
#endif
    XPORT_REG_LED_1_LINK_AND_SPEED_ENCODING_SEL_RSVD_SEL_SPD_ENCODE_2_FIELD_MASK,
    0,
    XPORT_REG_LED_1_LINK_AND_SPEED_ENCODING_SEL_RSVD_SEL_SPD_ENCODE_2_FIELD_WIDTH,
    XPORT_REG_LED_1_LINK_AND_SPEED_ENCODING_SEL_RSVD_SEL_SPD_ENCODE_2_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_REG_LED_1_LINK_AND_SPEED_ENCODING_SEL_RSVD_SEL_SPD_ENCODE_1
 ******************************************************************************/
const ru_field_rec XPORT_REG_LED_1_LINK_AND_SPEED_ENCODING_SEL_RSVD_SEL_SPD_ENCODE_1_FIELD =
{
    "RSVD_SEL_SPD_ENCODE_1",
#if RU_INCLUDE_DESC
    "",
    "Reserved SPDLNK_LED_SEL[2:0] encoding select.",
#endif
    XPORT_REG_LED_1_LINK_AND_SPEED_ENCODING_SEL_RSVD_SEL_SPD_ENCODE_1_FIELD_MASK,
    0,
    XPORT_REG_LED_1_LINK_AND_SPEED_ENCODING_SEL_RSVD_SEL_SPD_ENCODE_1_FIELD_WIDTH,
    XPORT_REG_LED_1_LINK_AND_SPEED_ENCODING_SEL_RSVD_SEL_SPD_ENCODE_1_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_REG_LED_1_LINK_AND_SPEED_ENCODING_SEL_SEL_10G_ENCODE
 ******************************************************************************/
const ru_field_rec XPORT_REG_LED_1_LINK_AND_SPEED_ENCODING_SEL_SEL_10G_ENCODE_FIELD =
{
    "SEL_10G_ENCODE",
#if RU_INCLUDE_DESC
    "",
    "SPDLNK_LED_SEL[2:0] encoding for 10Gb/s and higher link speed.\n"
    "When SPDLNK_LED_SEL[x] = 1'''b0, SPDLNK_LED[x] is driven by bits [17:0] of Link and Speed Encoding Register.\n"
    "When SPDLNK_LED_SEL[x] = 1'''b1, SPDLNK_LED[x] is driven by the activity.\n",
#endif
    XPORT_REG_LED_1_LINK_AND_SPEED_ENCODING_SEL_SEL_10G_ENCODE_FIELD_MASK,
    0,
    XPORT_REG_LED_1_LINK_AND_SPEED_ENCODING_SEL_SEL_10G_ENCODE_FIELD_WIDTH,
    XPORT_REG_LED_1_LINK_AND_SPEED_ENCODING_SEL_SEL_10G_ENCODE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_REG_LED_1_LINK_AND_SPEED_ENCODING_SEL_SEL_2500M_ENCODE
 ******************************************************************************/
const ru_field_rec XPORT_REG_LED_1_LINK_AND_SPEED_ENCODING_SEL_SEL_2500M_ENCODE_FIELD =
{
    "SEL_2500M_ENCODE",
#if RU_INCLUDE_DESC
    "",
    "SPDLNK_LED_SEL[2:0] encoding for 2500Mb/s link speed.",
#endif
    XPORT_REG_LED_1_LINK_AND_SPEED_ENCODING_SEL_SEL_2500M_ENCODE_FIELD_MASK,
    0,
    XPORT_REG_LED_1_LINK_AND_SPEED_ENCODING_SEL_SEL_2500M_ENCODE_FIELD_WIDTH,
    XPORT_REG_LED_1_LINK_AND_SPEED_ENCODING_SEL_SEL_2500M_ENCODE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_REG_LED_1_LINK_AND_SPEED_ENCODING_SEL_SEL_1000M_ENCODE
 ******************************************************************************/
const ru_field_rec XPORT_REG_LED_1_LINK_AND_SPEED_ENCODING_SEL_SEL_1000M_ENCODE_FIELD =
{
    "SEL_1000M_ENCODE",
#if RU_INCLUDE_DESC
    "",
    "SPDLNK_LED_SEL[2:0] encoding for 1000Mb/s link speed.",
#endif
    XPORT_REG_LED_1_LINK_AND_SPEED_ENCODING_SEL_SEL_1000M_ENCODE_FIELD_MASK,
    0,
    XPORT_REG_LED_1_LINK_AND_SPEED_ENCODING_SEL_SEL_1000M_ENCODE_FIELD_WIDTH,
    XPORT_REG_LED_1_LINK_AND_SPEED_ENCODING_SEL_SEL_1000M_ENCODE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_REG_LED_1_LINK_AND_SPEED_ENCODING_SEL_SEL_100M_ENCODE
 ******************************************************************************/
const ru_field_rec XPORT_REG_LED_1_LINK_AND_SPEED_ENCODING_SEL_SEL_100M_ENCODE_FIELD =
{
    "SEL_100M_ENCODE",
#if RU_INCLUDE_DESC
    "",
    "SPDLNK_LED_SEL[2:0] encoding for 100Mb/s link speed.",
#endif
    XPORT_REG_LED_1_LINK_AND_SPEED_ENCODING_SEL_SEL_100M_ENCODE_FIELD_MASK,
    0,
    XPORT_REG_LED_1_LINK_AND_SPEED_ENCODING_SEL_SEL_100M_ENCODE_FIELD_WIDTH,
    XPORT_REG_LED_1_LINK_AND_SPEED_ENCODING_SEL_SEL_100M_ENCODE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_REG_LED_1_LINK_AND_SPEED_ENCODING_SEL_SEL_10M_ENCODE
 ******************************************************************************/
const ru_field_rec XPORT_REG_LED_1_LINK_AND_SPEED_ENCODING_SEL_SEL_10M_ENCODE_FIELD =
{
    "SEL_10M_ENCODE",
#if RU_INCLUDE_DESC
    "",
    "SPDLNK_LED_SEL[2:0] encoding for 10Mb/s link speed.",
#endif
    XPORT_REG_LED_1_LINK_AND_SPEED_ENCODING_SEL_SEL_10M_ENCODE_FIELD_MASK,
    0,
    XPORT_REG_LED_1_LINK_AND_SPEED_ENCODING_SEL_SEL_10M_ENCODE_FIELD_WIDTH,
    XPORT_REG_LED_1_LINK_AND_SPEED_ENCODING_SEL_SEL_10M_ENCODE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_REG_LED_1_LINK_AND_SPEED_ENCODING_SEL_SEL_NO_LINK_ENCODE
 ******************************************************************************/
const ru_field_rec XPORT_REG_LED_1_LINK_AND_SPEED_ENCODING_SEL_SEL_NO_LINK_ENCODE_FIELD =
{
    "SEL_NO_LINK_ENCODE",
#if RU_INCLUDE_DESC
    "",
    "SPDLNK_LED_SEL[2:0] encoding for the no-link state.",
#endif
    XPORT_REG_LED_1_LINK_AND_SPEED_ENCODING_SEL_SEL_NO_LINK_ENCODE_FIELD_MASK,
    0,
    XPORT_REG_LED_1_LINK_AND_SPEED_ENCODING_SEL_SEL_NO_LINK_ENCODE_FIELD_WIDTH,
    XPORT_REG_LED_1_LINK_AND_SPEED_ENCODING_SEL_SEL_NO_LINK_ENCODE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_REG_LED_1_LINK_AND_SPEED_ENCODING_RESERVED0
 ******************************************************************************/
const ru_field_rec XPORT_REG_LED_1_LINK_AND_SPEED_ENCODING_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    XPORT_REG_LED_1_LINK_AND_SPEED_ENCODING_RESERVED0_FIELD_MASK,
    0,
    XPORT_REG_LED_1_LINK_AND_SPEED_ENCODING_RESERVED0_FIELD_WIDTH,
    XPORT_REG_LED_1_LINK_AND_SPEED_ENCODING_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_REG_LED_1_LINK_AND_SPEED_ENCODING_RSVD_SPD_ENCODE_2
 ******************************************************************************/
const ru_field_rec XPORT_REG_LED_1_LINK_AND_SPEED_ENCODING_RSVD_SPD_ENCODE_2_FIELD =
{
    "RSVD_SPD_ENCODE_2",
#if RU_INCLUDE_DESC
    "",
    "Reserved SPDLNK_LED_SEL[2:0] encoding.",
#endif
    XPORT_REG_LED_1_LINK_AND_SPEED_ENCODING_RSVD_SPD_ENCODE_2_FIELD_MASK,
    0,
    XPORT_REG_LED_1_LINK_AND_SPEED_ENCODING_RSVD_SPD_ENCODE_2_FIELD_WIDTH,
    XPORT_REG_LED_1_LINK_AND_SPEED_ENCODING_RSVD_SPD_ENCODE_2_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_REG_LED_1_LINK_AND_SPEED_ENCODING_RSVD_SPD_ENCODE_1
 ******************************************************************************/
const ru_field_rec XPORT_REG_LED_1_LINK_AND_SPEED_ENCODING_RSVD_SPD_ENCODE_1_FIELD =
{
    "RSVD_SPD_ENCODE_1",
#if RU_INCLUDE_DESC
    "",
    "Reserved SPDLNK_LED_SEL[2:0] encoding.",
#endif
    XPORT_REG_LED_1_LINK_AND_SPEED_ENCODING_RSVD_SPD_ENCODE_1_FIELD_MASK,
    0,
    XPORT_REG_LED_1_LINK_AND_SPEED_ENCODING_RSVD_SPD_ENCODE_1_FIELD_WIDTH,
    XPORT_REG_LED_1_LINK_AND_SPEED_ENCODING_RSVD_SPD_ENCODE_1_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_REG_LED_1_LINK_AND_SPEED_ENCODING_M10G_ENCODE
 ******************************************************************************/
const ru_field_rec XPORT_REG_LED_1_LINK_AND_SPEED_ENCODING_M10G_ENCODE_FIELD =
{
    "M10G_ENCODE",
#if RU_INCLUDE_DESC
    "",
    "SPDLNK_LED[2:0] encoding for 10Gb/s and higherlink speed.",
#endif
    XPORT_REG_LED_1_LINK_AND_SPEED_ENCODING_M10G_ENCODE_FIELD_MASK,
    0,
    XPORT_REG_LED_1_LINK_AND_SPEED_ENCODING_M10G_ENCODE_FIELD_WIDTH,
    XPORT_REG_LED_1_LINK_AND_SPEED_ENCODING_M10G_ENCODE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_REG_LED_1_LINK_AND_SPEED_ENCODING_M2500_ENCODE
 ******************************************************************************/
const ru_field_rec XPORT_REG_LED_1_LINK_AND_SPEED_ENCODING_M2500_ENCODE_FIELD =
{
    "M2500_ENCODE",
#if RU_INCLUDE_DESC
    "",
    "SPDLNK_LED[2:0] encoding for 2.5Gb/s link speed.",
#endif
    XPORT_REG_LED_1_LINK_AND_SPEED_ENCODING_M2500_ENCODE_FIELD_MASK,
    0,
    XPORT_REG_LED_1_LINK_AND_SPEED_ENCODING_M2500_ENCODE_FIELD_WIDTH,
    XPORT_REG_LED_1_LINK_AND_SPEED_ENCODING_M2500_ENCODE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_REG_LED_1_LINK_AND_SPEED_ENCODING_M1000_ENCODE
 ******************************************************************************/
const ru_field_rec XPORT_REG_LED_1_LINK_AND_SPEED_ENCODING_M1000_ENCODE_FIELD =
{
    "M1000_ENCODE",
#if RU_INCLUDE_DESC
    "",
    "SPDLNK_LED[2:0] encoding for 1Gb/s link speed.",
#endif
    XPORT_REG_LED_1_LINK_AND_SPEED_ENCODING_M1000_ENCODE_FIELD_MASK,
    0,
    XPORT_REG_LED_1_LINK_AND_SPEED_ENCODING_M1000_ENCODE_FIELD_WIDTH,
    XPORT_REG_LED_1_LINK_AND_SPEED_ENCODING_M1000_ENCODE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_REG_LED_1_LINK_AND_SPEED_ENCODING_M100_ENCODE
 ******************************************************************************/
const ru_field_rec XPORT_REG_LED_1_LINK_AND_SPEED_ENCODING_M100_ENCODE_FIELD =
{
    "M100_ENCODE",
#if RU_INCLUDE_DESC
    "",
    "SPDLNK_LED[2:0] encoding for 100Mb/s link speed.",
#endif
    XPORT_REG_LED_1_LINK_AND_SPEED_ENCODING_M100_ENCODE_FIELD_MASK,
    0,
    XPORT_REG_LED_1_LINK_AND_SPEED_ENCODING_M100_ENCODE_FIELD_WIDTH,
    XPORT_REG_LED_1_LINK_AND_SPEED_ENCODING_M100_ENCODE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_REG_LED_1_LINK_AND_SPEED_ENCODING_M10_ENCODE
 ******************************************************************************/
const ru_field_rec XPORT_REG_LED_1_LINK_AND_SPEED_ENCODING_M10_ENCODE_FIELD =
{
    "M10_ENCODE",
#if RU_INCLUDE_DESC
    "",
    "SPDLNK_LED[2:0] encoding for 10Mb/s link speed.",
#endif
    XPORT_REG_LED_1_LINK_AND_SPEED_ENCODING_M10_ENCODE_FIELD_MASK,
    0,
    XPORT_REG_LED_1_LINK_AND_SPEED_ENCODING_M10_ENCODE_FIELD_WIDTH,
    XPORT_REG_LED_1_LINK_AND_SPEED_ENCODING_M10_ENCODE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_REG_LED_1_LINK_AND_SPEED_ENCODING_NO_LINK_ENCODE
 ******************************************************************************/
const ru_field_rec XPORT_REG_LED_1_LINK_AND_SPEED_ENCODING_NO_LINK_ENCODE_FIELD =
{
    "NO_LINK_ENCODE",
#if RU_INCLUDE_DESC
    "",
    "SPDLNK_LED[2:0] encoding for the no-link state.",
#endif
    XPORT_REG_LED_1_LINK_AND_SPEED_ENCODING_NO_LINK_ENCODE_FIELD_MASK,
    0,
    XPORT_REG_LED_1_LINK_AND_SPEED_ENCODING_NO_LINK_ENCODE_FIELD_WIDTH,
    XPORT_REG_LED_1_LINK_AND_SPEED_ENCODING_NO_LINK_ENCODE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_REG_LED_BLINK_RATE_CNTRL_LED_ON_TIME
 ******************************************************************************/
const ru_field_rec XPORT_REG_LED_BLINK_RATE_CNTRL_LED_ON_TIME_FIELD =
{
    "LED_ON_TIME",
#if RU_INCLUDE_DESC
    "",
    "Led ON time. Expressed in 50us units.",
#endif
    XPORT_REG_LED_BLINK_RATE_CNTRL_LED_ON_TIME_FIELD_MASK,
    0,
    XPORT_REG_LED_BLINK_RATE_CNTRL_LED_ON_TIME_FIELD_WIDTH,
    XPORT_REG_LED_BLINK_RATE_CNTRL_LED_ON_TIME_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_REG_LED_BLINK_RATE_CNTRL_LED_OFF_TIME
 ******************************************************************************/
const ru_field_rec XPORT_REG_LED_BLINK_RATE_CNTRL_LED_OFF_TIME_FIELD =
{
    "LED_OFF_TIME",
#if RU_INCLUDE_DESC
    "",
    "Led OFF time. Expressed in 50us  units.",
#endif
    XPORT_REG_LED_BLINK_RATE_CNTRL_LED_OFF_TIME_FIELD_MASK,
    0,
    XPORT_REG_LED_BLINK_RATE_CNTRL_LED_OFF_TIME_FIELD_WIDTH,
    XPORT_REG_LED_BLINK_RATE_CNTRL_LED_OFF_TIME_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_REG_LED_SERIAL_CNTRL_RESERVED0
 ******************************************************************************/
const ru_field_rec XPORT_REG_LED_SERIAL_CNTRL_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    XPORT_REG_LED_SERIAL_CNTRL_RESERVED0_FIELD_MASK,
    0,
    XPORT_REG_LED_SERIAL_CNTRL_RESERVED0_FIELD_WIDTH,
    XPORT_REG_LED_SERIAL_CNTRL_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_REG_LED_SERIAL_CNTRL_SMODE
 ******************************************************************************/
const ru_field_rec XPORT_REG_LED_SERIAL_CNTRL_SMODE_FIELD =
{
    "SMODE",
#if RU_INCLUDE_DESC
    "",
    "Indicates number of LED signals per port that are shifted out:\n"
    "11 : 4 LEDs per port mode (SPDLNK_LED[2:0] and ACT_LED).\n"
    "10 : 3 LEDs per port mode (SPDLNK_LED[2:0]).\n"
    "01 : 3 LEDs per port mode (SPDLNK_LED[1:0] and ACT_LED).\n"
    "00 : 2 LEDs per port mode (SPDLNK_LED[1:0])",
#endif
    XPORT_REG_LED_SERIAL_CNTRL_SMODE_FIELD_MASK,
    0,
    XPORT_REG_LED_SERIAL_CNTRL_SMODE_FIELD_WIDTH,
    XPORT_REG_LED_SERIAL_CNTRL_SMODE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_REG_LED_SERIAL_CNTRL_SLED_CLK_FREQUENCY
 ******************************************************************************/
const ru_field_rec XPORT_REG_LED_SERIAL_CNTRL_SLED_CLK_FREQUENCY_FIELD =
{
    "SLED_CLK_FREQUENCY",
#if RU_INCLUDE_DESC
    "",
    "Indicates SLED_CLK frequency.\n"
    "0 : SLED_CLK is 6.25Mhz.\n"
    "1 : SLED_CLK is 3.125Mhz.",
#endif
    XPORT_REG_LED_SERIAL_CNTRL_SLED_CLK_FREQUENCY_FIELD_MASK,
    0,
    XPORT_REG_LED_SERIAL_CNTRL_SLED_CLK_FREQUENCY_FIELD_WIDTH,
    XPORT_REG_LED_SERIAL_CNTRL_SLED_CLK_FREQUENCY_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_REG_LED_SERIAL_CNTRL_SLED_CLK_POL
 ******************************************************************************/
const ru_field_rec XPORT_REG_LED_SERIAL_CNTRL_SLED_CLK_POL_FIELD =
{
    "SLED_CLK_POL",
#if RU_INCLUDE_DESC
    "",
    "When this bit is 1'b1 serial LED clock(SCLK) polarity is inveretd. "
    "Used with shift registers that trigger on the falling edge.",
#endif
    XPORT_REG_LED_SERIAL_CNTRL_SLED_CLK_POL_FIELD_MASK,
    0,
    XPORT_REG_LED_SERIAL_CNTRL_SLED_CLK_POL_FIELD_WIDTH,
    XPORT_REG_LED_SERIAL_CNTRL_SLED_CLK_POL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_REG_LED_SERIAL_CNTRL_REFRESH_PERIOD
 ******************************************************************************/
const ru_field_rec XPORT_REG_LED_SERIAL_CNTRL_REFRESH_PERIOD_FIELD =
{
    "REFRESH_PERIOD",
#if RU_INCLUDE_DESC
    "",
    "Serial LED refresh period. "
    "Expressed in 5ms units. Value of 0 means 32x5ms period.",
#endif
    XPORT_REG_LED_SERIAL_CNTRL_REFRESH_PERIOD_FIELD_MASK,
    0,
    XPORT_REG_LED_SERIAL_CNTRL_REFRESH_PERIOD_FIELD_WIDTH,
    XPORT_REG_LED_SERIAL_CNTRL_REFRESH_PERIOD_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_REG_LED_SERIAL_CNTRL_PORT_EN
 ******************************************************************************/
const ru_field_rec XPORT_REG_LED_SERIAL_CNTRL_PORT_EN_FIELD =
{
    "PORT_EN",
#if RU_INCLUDE_DESC
    "",
    "When the corresponding bit is set, port LEDs are shifted out. "
    "When all bits are cleared, serial LED interface is disabled.",
#endif
    XPORT_REG_LED_SERIAL_CNTRL_PORT_EN_FIELD_MASK,
    0,
    XPORT_REG_LED_SERIAL_CNTRL_PORT_EN_FIELD_WIDTH,
    XPORT_REG_LED_SERIAL_CNTRL_PORT_EN_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_REG_REFRESH_PERIOD_CNTRL_RESERVED0
 ******************************************************************************/
const ru_field_rec XPORT_REG_REFRESH_PERIOD_CNTRL_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    XPORT_REG_REFRESH_PERIOD_CNTRL_RESERVED0_FIELD_MASK,
    0,
    XPORT_REG_REFRESH_PERIOD_CNTRL_RESERVED0_FIELD_WIDTH,
    XPORT_REG_REFRESH_PERIOD_CNTRL_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_REG_REFRESH_PERIOD_CNTRL_REFRESH_PERIOD_CNT
 ******************************************************************************/
const ru_field_rec XPORT_REG_REFRESH_PERIOD_CNTRL_REFRESH_PERIOD_CNT_FIELD =
{
    "REFRESH_PERIOD_CNT",
#if RU_INCLUDE_DESC
    "",
    "This register is used only in debug purposes. It controls REFRESH_PERIOD time unit that is based on 25MHz clock. "
    "default is 5 ms.",
#endif
    XPORT_REG_REFRESH_PERIOD_CNTRL_REFRESH_PERIOD_CNT_FIELD_MASK,
    0,
    XPORT_REG_REFRESH_PERIOD_CNTRL_REFRESH_PERIOD_CNT_FIELD_WIDTH,
    XPORT_REG_REFRESH_PERIOD_CNTRL_REFRESH_PERIOD_CNT_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_REG_AGGREGATE_LED_CNTRL_RESERVED0
 ******************************************************************************/
const ru_field_rec XPORT_REG_AGGREGATE_LED_CNTRL_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    XPORT_REG_AGGREGATE_LED_CNTRL_RESERVED0_FIELD_MASK,
    0,
    XPORT_REG_AGGREGATE_LED_CNTRL_RESERVED0_FIELD_WIDTH,
    XPORT_REG_AGGREGATE_LED_CNTRL_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_REG_AGGREGATE_LED_CNTRL_LNK_POL_SEL
 ******************************************************************************/
const ru_field_rec XPORT_REG_AGGREGATE_LED_CNTRL_LNK_POL_SEL_FIELD =
{
    "LNK_POL_SEL",
#if RU_INCLUDE_DESC
    "",
    "When set to 1'b1 inverts polarity of the link signal that is used for aggregate LNK_LED.",
#endif
    XPORT_REG_AGGREGATE_LED_CNTRL_LNK_POL_SEL_FIELD_MASK,
    0,
    XPORT_REG_AGGREGATE_LED_CNTRL_LNK_POL_SEL_FIELD_WIDTH,
    XPORT_REG_AGGREGATE_LED_CNTRL_LNK_POL_SEL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_REG_AGGREGATE_LED_CNTRL_ACT_POL_SEL
 ******************************************************************************/
const ru_field_rec XPORT_REG_AGGREGATE_LED_CNTRL_ACT_POL_SEL_FIELD =
{
    "ACT_POL_SEL",
#if RU_INCLUDE_DESC
    "",
    "When set to 1'b1 inverts polarity of the activity signal that is used for aggregate ACT_LED.",
#endif
    XPORT_REG_AGGREGATE_LED_CNTRL_ACT_POL_SEL_FIELD_MASK,
    0,
    XPORT_REG_AGGREGATE_LED_CNTRL_ACT_POL_SEL_FIELD_WIDTH,
    XPORT_REG_AGGREGATE_LED_CNTRL_ACT_POL_SEL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_REG_AGGREGATE_LED_CNTRL_ACT_SEL
 ******************************************************************************/
const ru_field_rec XPORT_REG_AGGREGATE_LED_CNTRL_ACT_SEL_FIELD =
{
    "ACT_SEL",
#if RU_INCLUDE_DESC
    "",
    "Selects behavior for aggregate ACT_LED. Encoded as:"
    "0 : LED is 0 when aggregate link is up and blinks when there is activity. "
    "    LED is 1 when aggregate link is down.\n"
    "1 : LED is 1 and blinks when there is activity, regardless of the aggregate link status.\n",
#endif
    XPORT_REG_AGGREGATE_LED_CNTRL_ACT_SEL_FIELD_MASK,
    0,
    XPORT_REG_AGGREGATE_LED_CNTRL_ACT_SEL_FIELD_WIDTH,
    XPORT_REG_AGGREGATE_LED_CNTRL_ACT_SEL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_REG_AGGREGATE_LED_CNTRL_PORT_EN
 ******************************************************************************/
const ru_field_rec XPORT_REG_AGGREGATE_LED_CNTRL_PORT_EN_FIELD =
{
    "PORT_EN",
#if RU_INCLUDE_DESC
    "",
    "When the corresponding bit is set, port LEDs are included in aggregate LED signals. "
    "When all bits are cleared, aggregate LED interface is disabled.",
#endif
    XPORT_REG_AGGREGATE_LED_CNTRL_PORT_EN_FIELD_MASK,
    0,
    XPORT_REG_AGGREGATE_LED_CNTRL_PORT_EN_FIELD_WIDTH,
    XPORT_REG_AGGREGATE_LED_CNTRL_PORT_EN_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_REG_AGGREGATE_LED_BLINK_RATE_CNTRL_LED_ON_TIME
 ******************************************************************************/
const ru_field_rec XPORT_REG_AGGREGATE_LED_BLINK_RATE_CNTRL_LED_ON_TIME_FIELD =
{
    "LED_ON_TIME",
#if RU_INCLUDE_DESC
    "",
    "Led ON time. Expressed in 50us units.",
#endif
    XPORT_REG_AGGREGATE_LED_BLINK_RATE_CNTRL_LED_ON_TIME_FIELD_MASK,
    0,
    XPORT_REG_AGGREGATE_LED_BLINK_RATE_CNTRL_LED_ON_TIME_FIELD_WIDTH,
    XPORT_REG_AGGREGATE_LED_BLINK_RATE_CNTRL_LED_ON_TIME_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_REG_AGGREGATE_LED_BLINK_RATE_CNTRL_LED_OFF_TIME
 ******************************************************************************/
const ru_field_rec XPORT_REG_AGGREGATE_LED_BLINK_RATE_CNTRL_LED_OFF_TIME_FIELD =
{
    "LED_OFF_TIME",
#if RU_INCLUDE_DESC
    "",
    "Led OFF time. Expressed in 50us  units.",
#endif
    XPORT_REG_AGGREGATE_LED_BLINK_RATE_CNTRL_LED_OFF_TIME_FIELD_MASK,
    0,
    XPORT_REG_AGGREGATE_LED_BLINK_RATE_CNTRL_LED_OFF_TIME_FIELD_WIDTH,
    XPORT_REG_AGGREGATE_LED_BLINK_RATE_CNTRL_LED_OFF_TIME_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_REG_SPARE_CNTRL_SPARE_REG
 ******************************************************************************/
const ru_field_rec XPORT_REG_SPARE_CNTRL_SPARE_REG_FIELD =
{
    "SPARE_REG",
#if RU_INCLUDE_DESC
    "",
    "Spare register. Reserved for future use. ",
#endif
    XPORT_REG_SPARE_CNTRL_SPARE_REG_FIELD_MASK,
    0,
    XPORT_REG_SPARE_CNTRL_SPARE_REG_FIELD_WIDTH,
    XPORT_REG_SPARE_CNTRL_SPARE_REG_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_REG_XPORT_CNTRL_1_RESERVED0
 ******************************************************************************/
const ru_field_rec XPORT_REG_XPORT_CNTRL_1_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    XPORT_REG_XPORT_CNTRL_1_RESERVED0_FIELD_MASK,
    0,
    XPORT_REG_XPORT_CNTRL_1_RESERVED0_FIELD_WIDTH,
    XPORT_REG_XPORT_CNTRL_1_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_REG_XPORT_CNTRL_1_MSBUS_CLK_SEL
 ******************************************************************************/
const ru_field_rec XPORT_REG_XPORT_CNTRL_1_MSBUS_CLK_SEL_FIELD =
{
    "MSBUS_CLK_SEL",
#if RU_INCLUDE_DESC
    "",
    "MSBUS clock select.\n"
    "0 : 500MHz.\n"
    "1 : 644.53125MHz.\n"
    "644.53125MHz clock should be used ONLY when 10G AE and SGMII serdes are simultaneously connected to XRDP.",
#endif
    XPORT_REG_XPORT_CNTRL_1_MSBUS_CLK_SEL_FIELD_MASK,
    0,
    XPORT_REG_XPORT_CNTRL_1_MSBUS_CLK_SEL_FIELD_WIDTH,
    XPORT_REG_XPORT_CNTRL_1_MSBUS_CLK_SEL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_REG_XPORT_CNTRL_1_WAN_LED0_SEL
 ******************************************************************************/
const ru_field_rec XPORT_REG_XPORT_CNTRL_1_WAN_LED0_SEL_FIELD =
{
    "WAN_LED0_SEL",
#if RU_INCLUDE_DESC
    "",
    "selects port driving WAN LED0 set.\n"
    "0 : P0 drives LEDs.\n"
    "1 : P1 drives LEDs.\n",
#endif
    XPORT_REG_XPORT_CNTRL_1_WAN_LED0_SEL_FIELD_MASK,
    0,
    XPORT_REG_XPORT_CNTRL_1_WAN_LED0_SEL_FIELD_WIDTH,
    XPORT_REG_XPORT_CNTRL_1_WAN_LED0_SEL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_REG_XPORT_CNTRL_1_TIMEOUT_RST_DISABLE
 ******************************************************************************/
const ru_field_rec XPORT_REG_XPORT_CNTRL_1_TIMEOUT_RST_DISABLE_FIELD =
{
    "TIMEOUT_RST_DISABLE",
#if RU_INCLUDE_DESC
    "",
    "When this bit is set, XPORT internal register bus bridges are not automatically reseted/reinitalized when the UBUS slave port times out.\n",
#endif
    XPORT_REG_XPORT_CNTRL_1_TIMEOUT_RST_DISABLE_FIELD_MASK,
    0,
    XPORT_REG_XPORT_CNTRL_1_TIMEOUT_RST_DISABLE_FIELD_WIDTH,
    XPORT_REG_XPORT_CNTRL_1_TIMEOUT_RST_DISABLE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_REG_XPORT_CNTRL_1_RESERVED1
 ******************************************************************************/
const ru_field_rec XPORT_REG_XPORT_CNTRL_1_RESERVED1_FIELD =
{
    "RESERVED1",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    XPORT_REG_XPORT_CNTRL_1_RESERVED1_FIELD_MASK,
    0,
    XPORT_REG_XPORT_CNTRL_1_RESERVED1_FIELD_WIDTH,
    XPORT_REG_XPORT_CNTRL_1_RESERVED1_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_REG_XPORT_CNTRL_1_P0_MODE
 ******************************************************************************/
const ru_field_rec XPORT_REG_XPORT_CNTRL_1_P0_MODE_FIELD =
{
    "P0_MODE",
#if RU_INCLUDE_DESC
    "",
    "P0 Mode:\n"
    "0 : P0 operates in GMII mode.\n"
    "1 : P0 operates in XGMII mode.\n",
#endif
    XPORT_REG_XPORT_CNTRL_1_P0_MODE_FIELD_MASK,
    0,
    XPORT_REG_XPORT_CNTRL_1_P0_MODE_FIELD_WIDTH,
    XPORT_REG_XPORT_CNTRL_1_P0_MODE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_REG_XPORT_CNTRL_1_RESERVED2
 ******************************************************************************/
const ru_field_rec XPORT_REG_XPORT_CNTRL_1_RESERVED2_FIELD =
{
    "RESERVED2",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    XPORT_REG_XPORT_CNTRL_1_RESERVED2_FIELD_MASK,
    0,
    XPORT_REG_XPORT_CNTRL_1_RESERVED2_FIELD_WIDTH,
    XPORT_REG_XPORT_CNTRL_1_RESERVED2_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_REG_CROSSBAR_STATUS_RESERVED0
 ******************************************************************************/
const ru_field_rec XPORT_REG_CROSSBAR_STATUS_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    XPORT_REG_CROSSBAR_STATUS_RESERVED0_FIELD_MASK,
    0,
    XPORT_REG_CROSSBAR_STATUS_RESERVED0_FIELD_WIDTH,
    XPORT_REG_CROSSBAR_STATUS_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: XPORT_REG_CROSSBAR_STATUS_FULL_DUPLEX
 ******************************************************************************/
const ru_field_rec XPORT_REG_CROSSBAR_STATUS_FULL_DUPLEX_FIELD =
{
    "FULL_DUPLEX",
#if RU_INCLUDE_DESC
    "",
    "When set indicates that full-duplex link is established. "
    "Half-duplex is not supported and indicates erroneous link.",
#endif
    XPORT_REG_CROSSBAR_STATUS_FULL_DUPLEX_FIELD_MASK,
    0,
    XPORT_REG_CROSSBAR_STATUS_FULL_DUPLEX_FIELD_WIDTH,
    XPORT_REG_CROSSBAR_STATUS_FULL_DUPLEX_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: XPORT_REG_CROSSBAR_STATUS_PAUSE_TX
 ******************************************************************************/
const ru_field_rec XPORT_REG_CROSSBAR_STATUS_PAUSE_TX_FIELD =
{
    "PAUSE_TX",
#if RU_INCLUDE_DESC
    "",
    "When set indicates that TX PAUSE is negotiated.",
#endif
    XPORT_REG_CROSSBAR_STATUS_PAUSE_TX_FIELD_MASK,
    0,
    XPORT_REG_CROSSBAR_STATUS_PAUSE_TX_FIELD_WIDTH,
    XPORT_REG_CROSSBAR_STATUS_PAUSE_TX_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: XPORT_REG_CROSSBAR_STATUS_PAUSE_RX
 ******************************************************************************/
const ru_field_rec XPORT_REG_CROSSBAR_STATUS_PAUSE_RX_FIELD =
{
    "PAUSE_RX",
#if RU_INCLUDE_DESC
    "",
    "When set indicates that RX PAUSE is negotiated.",
#endif
    XPORT_REG_CROSSBAR_STATUS_PAUSE_RX_FIELD_MASK,
    0,
    XPORT_REG_CROSSBAR_STATUS_PAUSE_RX_FIELD_WIDTH,
    XPORT_REG_CROSSBAR_STATUS_PAUSE_RX_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: XPORT_REG_CROSSBAR_STATUS_SPEED_2500
 ******************************************************************************/
const ru_field_rec XPORT_REG_CROSSBAR_STATUS_SPEED_2500_FIELD =
{
    "SPEED_2500",
#if RU_INCLUDE_DESC
    "",
    "When set indicate that link is 2.5Gb.",
#endif
    XPORT_REG_CROSSBAR_STATUS_SPEED_2500_FIELD_MASK,
    0,
    XPORT_REG_CROSSBAR_STATUS_SPEED_2500_FIELD_WIDTH,
    XPORT_REG_CROSSBAR_STATUS_SPEED_2500_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: XPORT_REG_CROSSBAR_STATUS_SPEED_1000
 ******************************************************************************/
const ru_field_rec XPORT_REG_CROSSBAR_STATUS_SPEED_1000_FIELD =
{
    "SPEED_1000",
#if RU_INCLUDE_DESC
    "",
    "When set indicate that link is 1Gb.",
#endif
    XPORT_REG_CROSSBAR_STATUS_SPEED_1000_FIELD_MASK,
    0,
    XPORT_REG_CROSSBAR_STATUS_SPEED_1000_FIELD_WIDTH,
    XPORT_REG_CROSSBAR_STATUS_SPEED_1000_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: XPORT_REG_CROSSBAR_STATUS_SPEED_100
 ******************************************************************************/
const ru_field_rec XPORT_REG_CROSSBAR_STATUS_SPEED_100_FIELD =
{
    "SPEED_100",
#if RU_INCLUDE_DESC
    "",
    "When set indicate that link is 100Mb.",
#endif
    XPORT_REG_CROSSBAR_STATUS_SPEED_100_FIELD_MASK,
    0,
    XPORT_REG_CROSSBAR_STATUS_SPEED_100_FIELD_WIDTH,
    XPORT_REG_CROSSBAR_STATUS_SPEED_100_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: XPORT_REG_CROSSBAR_STATUS_SPEED_10
 ******************************************************************************/
const ru_field_rec XPORT_REG_CROSSBAR_STATUS_SPEED_10_FIELD =
{
    "SPEED_10",
#if RU_INCLUDE_DESC
    "",
    "When set indicate that link is 10Mb.",
#endif
    XPORT_REG_CROSSBAR_STATUS_SPEED_10_FIELD_MASK,
    0,
    XPORT_REG_CROSSBAR_STATUS_SPEED_10_FIELD_WIDTH,
    XPORT_REG_CROSSBAR_STATUS_SPEED_10_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: XPORT_REG_CROSSBAR_STATUS_LINK_STATUS
 ******************************************************************************/
const ru_field_rec XPORT_REG_CROSSBAR_STATUS_LINK_STATUS_FIELD =
{
    "LINK_STATUS",
#if RU_INCLUDE_DESC
    "",
    "Link Status. When 1 indicates that link is up for the selected PHY/RGMII.",
#endif
    XPORT_REG_CROSSBAR_STATUS_LINK_STATUS_FIELD_MASK,
    0,
    XPORT_REG_CROSSBAR_STATUS_LINK_STATUS_FIELD_WIDTH,
    XPORT_REG_CROSSBAR_STATUS_LINK_STATUS_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: XPORT_REG_PON_AE_SERDES_STATUS_RESERVED0
 ******************************************************************************/
const ru_field_rec XPORT_REG_PON_AE_SERDES_STATUS_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    XPORT_REG_PON_AE_SERDES_STATUS_RESERVED0_FIELD_MASK,
    0,
    XPORT_REG_PON_AE_SERDES_STATUS_RESERVED0_FIELD_WIDTH,
    XPORT_REG_PON_AE_SERDES_STATUS_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: XPORT_REG_PON_AE_SERDES_STATUS_MOD_DEF0
 ******************************************************************************/
const ru_field_rec XPORT_REG_PON_AE_SERDES_STATUS_MOD_DEF0_FIELD =
{
    "MOD_DEF0",
#if RU_INCLUDE_DESC
    "",
    "When 0 indicates presence of the optical module.",
#endif
    XPORT_REG_PON_AE_SERDES_STATUS_MOD_DEF0_FIELD_MASK,
    0,
    XPORT_REG_PON_AE_SERDES_STATUS_MOD_DEF0_FIELD_WIDTH,
    XPORT_REG_PON_AE_SERDES_STATUS_MOD_DEF0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: XPORT_REG_PON_AE_SERDES_STATUS_EXT_SIG_DET
 ******************************************************************************/
const ru_field_rec XPORT_REG_PON_AE_SERDES_STATUS_EXT_SIG_DET_FIELD =
{
    "EXT_SIG_DET",
#if RU_INCLUDE_DESC
    "",
    "Non-filtered signal detect (or loss of signal) from the pin as provided by the external optical module. "
    "Please consult used optical module datasheet for polarity. "
    "NVRAM bit that indicates expected polarity is recommended.",
#endif
    XPORT_REG_PON_AE_SERDES_STATUS_EXT_SIG_DET_FIELD_MASK,
    0,
    XPORT_REG_PON_AE_SERDES_STATUS_EXT_SIG_DET_FIELD_WIDTH,
    XPORT_REG_PON_AE_SERDES_STATUS_EXT_SIG_DET_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: XPORT_REG_PON_AE_SERDES_STATUS_PLL1_LOCK
 ******************************************************************************/
const ru_field_rec XPORT_REG_PON_AE_SERDES_STATUS_PLL1_LOCK_FIELD =
{
    "PLL1_LOCK",
#if RU_INCLUDE_DESC
    "",
    "PLL1 Lock. When 1'b1, indicates that single SERDES PLL1 is locked."
    "Only one of PLLs (PLL0 or PLL1) is active at any time, depending on the operational mode.",
#endif
    XPORT_REG_PON_AE_SERDES_STATUS_PLL1_LOCK_FIELD_MASK,
    0,
    XPORT_REG_PON_AE_SERDES_STATUS_PLL1_LOCK_FIELD_WIDTH,
    XPORT_REG_PON_AE_SERDES_STATUS_PLL1_LOCK_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: XPORT_REG_PON_AE_SERDES_STATUS_PLL0_LOCK
 ******************************************************************************/
const ru_field_rec XPORT_REG_PON_AE_SERDES_STATUS_PLL0_LOCK_FIELD =
{
    "PLL0_LOCK",
#if RU_INCLUDE_DESC
    "",
    "PLL0 Lock. When 1'b1, indicates that single SERDES PLL0 is locked."
    "Only one of PLLs (PLL0 or PLL1) is active at any time, depending on the operational mode.",
#endif
    XPORT_REG_PON_AE_SERDES_STATUS_PLL0_LOCK_FIELD_MASK,
    0,
    XPORT_REG_PON_AE_SERDES_STATUS_PLL0_LOCK_FIELD_WIDTH,
    XPORT_REG_PON_AE_SERDES_STATUS_PLL0_LOCK_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: XPORT_REG_PON_AE_SERDES_STATUS_LINK_STATUS
 ******************************************************************************/
const ru_field_rec XPORT_REG_PON_AE_SERDES_STATUS_LINK_STATUS_FIELD =
{
    "LINK_STATUS",
#if RU_INCLUDE_DESC
    "",
    "Link Status. When 1'b1, indicates that link is up.",
#endif
    XPORT_REG_PON_AE_SERDES_STATUS_LINK_STATUS_FIELD_MASK,
    0,
    XPORT_REG_PON_AE_SERDES_STATUS_LINK_STATUS_FIELD_WIDTH,
    XPORT_REG_PON_AE_SERDES_STATUS_LINK_STATUS_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: XPORT_REG_PON_AE_SERDES_STATUS_CDR_LOCK
 ******************************************************************************/
const ru_field_rec XPORT_REG_PON_AE_SERDES_STATUS_CDR_LOCK_FIELD =
{
    "CDR_LOCK",
#if RU_INCLUDE_DESC
    "",
    "CDR Lock. When 1'b1, indicates that CDR is locked.",
#endif
    XPORT_REG_PON_AE_SERDES_STATUS_CDR_LOCK_FIELD_MASK,
    0,
    XPORT_REG_PON_AE_SERDES_STATUS_CDR_LOCK_FIELD_WIDTH,
    XPORT_REG_PON_AE_SERDES_STATUS_CDR_LOCK_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: XPORT_REG_PON_AE_SERDES_STATUS_RX_SIGDET
 ******************************************************************************/
const ru_field_rec XPORT_REG_PON_AE_SERDES_STATUS_RX_SIGDET_FIELD =
{
    "RX_SIGDET",
#if RU_INCLUDE_DESC
    "",
    "Filtered Rx Signal Detect. When 1'b1 indicates presence of the signal on Rx pins.",
#endif
    XPORT_REG_PON_AE_SERDES_STATUS_RX_SIGDET_FIELD_MASK,
    0,
    XPORT_REG_PON_AE_SERDES_STATUS_RX_SIGDET_FIELD_WIDTH,
    XPORT_REG_PON_AE_SERDES_STATUS_RX_SIGDET_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

#endif /* RU_INCLUDE_FIELD_DB */

/******************************************************************************
 * Register: XPORT_REG_XPORT_REVISION
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XPORT_REG_XPORT_REVISION_FIELDS[] =
{
    &XPORT_REG_XPORT_REVISION_RESERVED0_FIELD,
    &XPORT_REG_XPORT_REVISION_XPORT_REV_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec XPORT_REG_XPORT_REVISION_REG = 
{
    "XPORT_REVISION",
#if RU_INCLUDE_DESC
    "XPORT Revision Control Register",
    "",
#endif
    XPORT_REG_XPORT_REVISION_REG_OFFSET,
    0,
    0,
    165,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    XPORT_REG_XPORT_REVISION_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: XPORT_REG_LED_PWM_CNTRL
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XPORT_REG_LED_PWM_CNTRL_FIELDS[] =
{
    &XPORT_REG_LED_PWM_CNTRL_RESERVED0_FIELD,
    &XPORT_REG_LED_PWM_CNTRL_PWM_ENABLE_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec XPORT_REG_LED_PWM_CNTRL_REG = 
{
    "LED_PWM_CNTRL",
#if RU_INCLUDE_DESC
    "LED PWM Control Register",
    "",
#endif
    XPORT_REG_LED_PWM_CNTRL_REG_OFFSET,
    0,
    0,
    166,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    XPORT_REG_LED_PWM_CNTRL_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: XPORT_REG_LED_INTENSITY_CNTRL
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XPORT_REG_LED_INTENSITY_CNTRL_FIELDS[] =
{
    &XPORT_REG_LED_INTENSITY_CNTRL_LED_ON_LOW_FIELD,
    &XPORT_REG_LED_INTENSITY_CNTRL_LED_ON_HIGH_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec XPORT_REG_LED_INTENSITY_CNTRL_REG = 
{
    "LED_INTENSITY_CNTRL",
#if RU_INCLUDE_DESC
    "LED Intensity Control Register",
    "",
#endif
    XPORT_REG_LED_INTENSITY_CNTRL_REG_OFFSET,
    0,
    0,
    167,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    XPORT_REG_LED_INTENSITY_CNTRL_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: XPORT_REG_LED_0_CNTRL
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XPORT_REG_LED_0_CNTRL_FIELDS[] =
{
    &XPORT_REG_LED_0_CNTRL_RESERVED0_FIELD,
    &XPORT_REG_LED_0_CNTRL_LNK_OVRD_EN_FIELD,
    &XPORT_REG_LED_0_CNTRL_SPD_OVRD_EN_FIELD,
    &XPORT_REG_LED_0_CNTRL_LNK_STATUS_OVRD_FIELD,
    &XPORT_REG_LED_0_CNTRL_LED_SPD_OVRD_FIELD,
    &XPORT_REG_LED_0_CNTRL_ACT_LED_POL_SEL_FIELD,
    &XPORT_REG_LED_0_CNTRL_SPDLNK_LED2_ACT_POL_SEL_FIELD,
    &XPORT_REG_LED_0_CNTRL_SPDLNK_LED1_ACT_POL_SEL_FIELD,
    &XPORT_REG_LED_0_CNTRL_SPDLNK_LED0_ACT_POL_SEL_FIELD,
    &XPORT_REG_LED_0_CNTRL_ACT_LED_ACT_SEL_FIELD,
    &XPORT_REG_LED_0_CNTRL_SPDLNK_LED2_ACT_SEL_FIELD,
    &XPORT_REG_LED_0_CNTRL_SPDLNK_LED1_ACT_SEL_FIELD,
    &XPORT_REG_LED_0_CNTRL_SPDLNK_LED0_ACT_SEL_FIELD,
    &XPORT_REG_LED_0_CNTRL_TX_ACT_EN_FIELD,
    &XPORT_REG_LED_0_CNTRL_RX_ACT_EN_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec XPORT_REG_LED_0_CNTRL_REG = 
{
    "LED_0_CNTRL",
#if RU_INCLUDE_DESC
    "LED 1 Control Register",
    "",
#endif
    XPORT_REG_LED_0_CNTRL_REG_OFFSET,
    0,
    0,
    168,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    15,
    XPORT_REG_LED_0_CNTRL_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: XPORT_REG_LED_0_LINK_AND_SPEED_ENCODING_SEL
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XPORT_REG_LED_0_LINK_AND_SPEED_ENCODING_SEL_FIELDS[] =
{
    &XPORT_REG_LED_0_LINK_AND_SPEED_ENCODING_SEL_RESERVED0_FIELD,
    &XPORT_REG_LED_0_LINK_AND_SPEED_ENCODING_SEL_RSVD_SEL_SPD_ENCODE_2_FIELD,
    &XPORT_REG_LED_0_LINK_AND_SPEED_ENCODING_SEL_RSVD_SEL_SPD_ENCODE_1_FIELD,
    &XPORT_REG_LED_0_LINK_AND_SPEED_ENCODING_SEL_SEL_10G_ENCODE_FIELD,
    &XPORT_REG_LED_0_LINK_AND_SPEED_ENCODING_SEL_SEL_2500M_ENCODE_FIELD,
    &XPORT_REG_LED_0_LINK_AND_SPEED_ENCODING_SEL_SEL_1000M_ENCODE_FIELD,
    &XPORT_REG_LED_0_LINK_AND_SPEED_ENCODING_SEL_SEL_100M_ENCODE_FIELD,
    &XPORT_REG_LED_0_LINK_AND_SPEED_ENCODING_SEL_SEL_10M_ENCODE_FIELD,
    &XPORT_REG_LED_0_LINK_AND_SPEED_ENCODING_SEL_SEL_NO_LINK_ENCODE_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec XPORT_REG_LED_0_LINK_AND_SPEED_ENCODING_SEL_REG = 
{
    "LED_0_LINK_AND_SPEED_ENCODING_SEL",
#if RU_INCLUDE_DESC
    "LED 1 Link And Speed Encoding Selection Register",
    "",
#endif
    XPORT_REG_LED_0_LINK_AND_SPEED_ENCODING_SEL_REG_OFFSET,
    0,
    0,
    169,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    9,
    XPORT_REG_LED_0_LINK_AND_SPEED_ENCODING_SEL_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: XPORT_REG_LED_0_LINK_AND_SPEED_ENCODING
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XPORT_REG_LED_0_LINK_AND_SPEED_ENCODING_FIELDS[] =
{
    &XPORT_REG_LED_0_LINK_AND_SPEED_ENCODING_RESERVED0_FIELD,
    &XPORT_REG_LED_0_LINK_AND_SPEED_ENCODING_RSVD_SPD_ENCODE_2_FIELD,
    &XPORT_REG_LED_0_LINK_AND_SPEED_ENCODING_RSVD_SPD_ENCODE_1_FIELD,
    &XPORT_REG_LED_0_LINK_AND_SPEED_ENCODING_M10G_ENCODE_FIELD,
    &XPORT_REG_LED_0_LINK_AND_SPEED_ENCODING_M2500_ENCODE_FIELD,
    &XPORT_REG_LED_0_LINK_AND_SPEED_ENCODING_M1000_ENCODE_FIELD,
    &XPORT_REG_LED_0_LINK_AND_SPEED_ENCODING_M100_ENCODE_FIELD,
    &XPORT_REG_LED_0_LINK_AND_SPEED_ENCODING_M10_ENCODE_FIELD,
    &XPORT_REG_LED_0_LINK_AND_SPEED_ENCODING_NO_LINK_ENCODE_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec XPORT_REG_LED_0_LINK_AND_SPEED_ENCODING_REG = 
{
    "LED_0_LINK_AND_SPEED_ENCODING",
#if RU_INCLUDE_DESC
    "LED 1 Link And Speed Encoding Register",
    "",
#endif
    XPORT_REG_LED_0_LINK_AND_SPEED_ENCODING_REG_OFFSET,
    0,
    0,
    170,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    9,
    XPORT_REG_LED_0_LINK_AND_SPEED_ENCODING_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: XPORT_REG_LED_1_CNTRL
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XPORT_REG_LED_1_CNTRL_FIELDS[] =
{
    &XPORT_REG_LED_1_CNTRL_RESERVED0_FIELD,
    &XPORT_REG_LED_1_CNTRL_LNK_OVRD_EN_FIELD,
    &XPORT_REG_LED_1_CNTRL_SPD_OVRD_EN_FIELD,
    &XPORT_REG_LED_1_CNTRL_LNK_STATUS_OVRD_FIELD,
    &XPORT_REG_LED_1_CNTRL_LED_SPD_OVRD_FIELD,
    &XPORT_REG_LED_1_CNTRL_ACT_LED_POL_SEL_FIELD,
    &XPORT_REG_LED_1_CNTRL_SPDLNK_LED2_ACT_POL_SEL_FIELD,
    &XPORT_REG_LED_1_CNTRL_SPDLNK_LED1_ACT_POL_SEL_FIELD,
    &XPORT_REG_LED_1_CNTRL_SPDLNK_LED0_ACT_POL_SEL_FIELD,
    &XPORT_REG_LED_1_CNTRL_ACT_LED_ACT_SEL_FIELD,
    &XPORT_REG_LED_1_CNTRL_SPDLNK_LED2_ACT_SEL_FIELD,
    &XPORT_REG_LED_1_CNTRL_SPDLNK_LED1_ACT_SEL_FIELD,
    &XPORT_REG_LED_1_CNTRL_SPDLNK_LED0_ACT_SEL_FIELD,
    &XPORT_REG_LED_1_CNTRL_TX_ACT_EN_FIELD,
    &XPORT_REG_LED_1_CNTRL_RX_ACT_EN_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec XPORT_REG_LED_1_CNTRL_REG = 
{
    "LED_1_CNTRL",
#if RU_INCLUDE_DESC
    "LED 1 Control Register",
    "",
#endif
    XPORT_REG_LED_1_CNTRL_REG_OFFSET,
    0,
    0,
    171,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    15,
    XPORT_REG_LED_1_CNTRL_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: XPORT_REG_LED_1_LINK_AND_SPEED_ENCODING_SEL
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XPORT_REG_LED_1_LINK_AND_SPEED_ENCODING_SEL_FIELDS[] =
{
    &XPORT_REG_LED_1_LINK_AND_SPEED_ENCODING_SEL_RESERVED0_FIELD,
    &XPORT_REG_LED_1_LINK_AND_SPEED_ENCODING_SEL_RSVD_SEL_SPD_ENCODE_2_FIELD,
    &XPORT_REG_LED_1_LINK_AND_SPEED_ENCODING_SEL_RSVD_SEL_SPD_ENCODE_1_FIELD,
    &XPORT_REG_LED_1_LINK_AND_SPEED_ENCODING_SEL_SEL_10G_ENCODE_FIELD,
    &XPORT_REG_LED_1_LINK_AND_SPEED_ENCODING_SEL_SEL_2500M_ENCODE_FIELD,
    &XPORT_REG_LED_1_LINK_AND_SPEED_ENCODING_SEL_SEL_1000M_ENCODE_FIELD,
    &XPORT_REG_LED_1_LINK_AND_SPEED_ENCODING_SEL_SEL_100M_ENCODE_FIELD,
    &XPORT_REG_LED_1_LINK_AND_SPEED_ENCODING_SEL_SEL_10M_ENCODE_FIELD,
    &XPORT_REG_LED_1_LINK_AND_SPEED_ENCODING_SEL_SEL_NO_LINK_ENCODE_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec XPORT_REG_LED_1_LINK_AND_SPEED_ENCODING_SEL_REG = 
{
    "LED_1_LINK_AND_SPEED_ENCODING_SEL",
#if RU_INCLUDE_DESC
    "LED 1 Link And Speed Encoding Selection Register",
    "",
#endif
    XPORT_REG_LED_1_LINK_AND_SPEED_ENCODING_SEL_REG_OFFSET,
    0,
    0,
    172,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    9,
    XPORT_REG_LED_1_LINK_AND_SPEED_ENCODING_SEL_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: XPORT_REG_LED_1_LINK_AND_SPEED_ENCODING
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XPORT_REG_LED_1_LINK_AND_SPEED_ENCODING_FIELDS[] =
{
    &XPORT_REG_LED_1_LINK_AND_SPEED_ENCODING_RESERVED0_FIELD,
    &XPORT_REG_LED_1_LINK_AND_SPEED_ENCODING_RSVD_SPD_ENCODE_2_FIELD,
    &XPORT_REG_LED_1_LINK_AND_SPEED_ENCODING_RSVD_SPD_ENCODE_1_FIELD,
    &XPORT_REG_LED_1_LINK_AND_SPEED_ENCODING_M10G_ENCODE_FIELD,
    &XPORT_REG_LED_1_LINK_AND_SPEED_ENCODING_M2500_ENCODE_FIELD,
    &XPORT_REG_LED_1_LINK_AND_SPEED_ENCODING_M1000_ENCODE_FIELD,
    &XPORT_REG_LED_1_LINK_AND_SPEED_ENCODING_M100_ENCODE_FIELD,
    &XPORT_REG_LED_1_LINK_AND_SPEED_ENCODING_M10_ENCODE_FIELD,
    &XPORT_REG_LED_1_LINK_AND_SPEED_ENCODING_NO_LINK_ENCODE_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec XPORT_REG_LED_1_LINK_AND_SPEED_ENCODING_REG = 
{
    "LED_1_LINK_AND_SPEED_ENCODING",
#if RU_INCLUDE_DESC
    "LED 1 Link And Speed Encoding Register",
    "",
#endif
    XPORT_REG_LED_1_LINK_AND_SPEED_ENCODING_REG_OFFSET,
    0,
    0,
    173,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    9,
    XPORT_REG_LED_1_LINK_AND_SPEED_ENCODING_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: XPORT_REG_LED_BLINK_RATE_CNTRL
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XPORT_REG_LED_BLINK_RATE_CNTRL_FIELDS[] =
{
    &XPORT_REG_LED_BLINK_RATE_CNTRL_LED_ON_TIME_FIELD,
    &XPORT_REG_LED_BLINK_RATE_CNTRL_LED_OFF_TIME_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec XPORT_REG_LED_BLINK_RATE_CNTRL_REG = 
{
    "LED_BLINK_RATE_CNTRL",
#if RU_INCLUDE_DESC
    "Aggregate LED Blink Rate Control Register",
    "",
#endif
    XPORT_REG_LED_BLINK_RATE_CNTRL_REG_OFFSET,
    0,
    0,
    174,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    XPORT_REG_LED_BLINK_RATE_CNTRL_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: XPORT_REG_LED_SERIAL_CNTRL
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XPORT_REG_LED_SERIAL_CNTRL_FIELDS[] =
{
    &XPORT_REG_LED_SERIAL_CNTRL_RESERVED0_FIELD,
    &XPORT_REG_LED_SERIAL_CNTRL_SMODE_FIELD,
    &XPORT_REG_LED_SERIAL_CNTRL_SLED_CLK_FREQUENCY_FIELD,
    &XPORT_REG_LED_SERIAL_CNTRL_SLED_CLK_POL_FIELD,
    &XPORT_REG_LED_SERIAL_CNTRL_REFRESH_PERIOD_FIELD,
    &XPORT_REG_LED_SERIAL_CNTRL_PORT_EN_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec XPORT_REG_LED_SERIAL_CNTRL_REG = 
{
    "LED_SERIAL_CNTRL",
#if RU_INCLUDE_DESC
    "LED Serial Control Register",
    "",
#endif
    XPORT_REG_LED_SERIAL_CNTRL_REG_OFFSET,
    0,
    0,
    175,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    6,
    XPORT_REG_LED_SERIAL_CNTRL_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: XPORT_REG_REFRESH_PERIOD_CNTRL
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XPORT_REG_REFRESH_PERIOD_CNTRL_FIELDS[] =
{
    &XPORT_REG_REFRESH_PERIOD_CNTRL_RESERVED0_FIELD,
    &XPORT_REG_REFRESH_PERIOD_CNTRL_REFRESH_PERIOD_CNT_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec XPORT_REG_REFRESH_PERIOD_CNTRL_REG = 
{
    "REFRESH_PERIOD_CNTRL",
#if RU_INCLUDE_DESC
    "Refresh Period Control Register",
    "",
#endif
    XPORT_REG_REFRESH_PERIOD_CNTRL_REG_OFFSET,
    0,
    0,
    176,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    XPORT_REG_REFRESH_PERIOD_CNTRL_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: XPORT_REG_AGGREGATE_LED_CNTRL
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XPORT_REG_AGGREGATE_LED_CNTRL_FIELDS[] =
{
    &XPORT_REG_AGGREGATE_LED_CNTRL_RESERVED0_FIELD,
    &XPORT_REG_AGGREGATE_LED_CNTRL_LNK_POL_SEL_FIELD,
    &XPORT_REG_AGGREGATE_LED_CNTRL_ACT_POL_SEL_FIELD,
    &XPORT_REG_AGGREGATE_LED_CNTRL_ACT_SEL_FIELD,
    &XPORT_REG_AGGREGATE_LED_CNTRL_PORT_EN_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec XPORT_REG_AGGREGATE_LED_CNTRL_REG = 
{
    "AGGREGATE_LED_CNTRL",
#if RU_INCLUDE_DESC
    "Aggregate LED Control Register",
    "",
#endif
    XPORT_REG_AGGREGATE_LED_CNTRL_REG_OFFSET,
    0,
    0,
    177,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    5,
    XPORT_REG_AGGREGATE_LED_CNTRL_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: XPORT_REG_AGGREGATE_LED_BLINK_RATE_CNTRL
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XPORT_REG_AGGREGATE_LED_BLINK_RATE_CNTRL_FIELDS[] =
{
    &XPORT_REG_AGGREGATE_LED_BLINK_RATE_CNTRL_LED_ON_TIME_FIELD,
    &XPORT_REG_AGGREGATE_LED_BLINK_RATE_CNTRL_LED_OFF_TIME_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec XPORT_REG_AGGREGATE_LED_BLINK_RATE_CNTRL_REG = 
{
    "AGGREGATE_LED_BLINK_RATE_CNTRL",
#if RU_INCLUDE_DESC
    "Aggregate LED Blink Rate Control Register",
    "",
#endif
    XPORT_REG_AGGREGATE_LED_BLINK_RATE_CNTRL_REG_OFFSET,
    0,
    0,
    178,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    XPORT_REG_AGGREGATE_LED_BLINK_RATE_CNTRL_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: XPORT_REG_SPARE_CNTRL
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XPORT_REG_SPARE_CNTRL_FIELDS[] =
{
    &XPORT_REG_SPARE_CNTRL_SPARE_REG_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec XPORT_REG_SPARE_CNTRL_REG = 
{
    "SPARE_CNTRL",
#if RU_INCLUDE_DESC
    "Spare Control Register",
    "",
#endif
    XPORT_REG_SPARE_CNTRL_REG_OFFSET,
    0,
    0,
    179,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    XPORT_REG_SPARE_CNTRL_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: XPORT_REG_XPORT_CNTRL_1
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XPORT_REG_XPORT_CNTRL_1_FIELDS[] =
{
    &XPORT_REG_XPORT_CNTRL_1_RESERVED0_FIELD,
    &XPORT_REG_XPORT_CNTRL_1_MSBUS_CLK_SEL_FIELD,
    &XPORT_REG_XPORT_CNTRL_1_WAN_LED0_SEL_FIELD,
    &XPORT_REG_XPORT_CNTRL_1_TIMEOUT_RST_DISABLE_FIELD,
    &XPORT_REG_XPORT_CNTRL_1_RESERVED1_FIELD,
    &XPORT_REG_XPORT_CNTRL_1_P0_MODE_FIELD,
    &XPORT_REG_XPORT_CNTRL_1_RESERVED2_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec XPORT_REG_XPORT_CNTRL_1_REG = 
{
    "XPORT_CNTRL_1",
#if RU_INCLUDE_DESC
    "XPORT Control 1 Register",
    "",
#endif
    XPORT_REG_XPORT_CNTRL_1_REG_OFFSET,
    0,
    0,
    180,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    7,
    XPORT_REG_XPORT_CNTRL_1_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: XPORT_REG_CROSSBAR_STATUS
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XPORT_REG_CROSSBAR_STATUS_FIELDS[] =
{
    &XPORT_REG_CROSSBAR_STATUS_RESERVED0_FIELD,
    &XPORT_REG_CROSSBAR_STATUS_FULL_DUPLEX_FIELD,
    &XPORT_REG_CROSSBAR_STATUS_PAUSE_TX_FIELD,
    &XPORT_REG_CROSSBAR_STATUS_PAUSE_RX_FIELD,
    &XPORT_REG_CROSSBAR_STATUS_SPEED_2500_FIELD,
    &XPORT_REG_CROSSBAR_STATUS_SPEED_1000_FIELD,
    &XPORT_REG_CROSSBAR_STATUS_SPEED_100_FIELD,
    &XPORT_REG_CROSSBAR_STATUS_SPEED_10_FIELD,
    &XPORT_REG_CROSSBAR_STATUS_LINK_STATUS_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec XPORT_REG_CROSSBAR_STATUS_REG = 
{
    "CROSSBAR_STATUS",
#if RU_INCLUDE_DESC
    "Crossbar Status Register",
    "",
#endif
    XPORT_REG_CROSSBAR_STATUS_REG_OFFSET,
    0,
    0,
    181,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    9,
    XPORT_REG_CROSSBAR_STATUS_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: XPORT_REG_PON_AE_SERDES_STATUS
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XPORT_REG_PON_AE_SERDES_STATUS_FIELDS[] =
{
    &XPORT_REG_PON_AE_SERDES_STATUS_RESERVED0_FIELD,
    &XPORT_REG_PON_AE_SERDES_STATUS_MOD_DEF0_FIELD,
    &XPORT_REG_PON_AE_SERDES_STATUS_EXT_SIG_DET_FIELD,
    &XPORT_REG_PON_AE_SERDES_STATUS_PLL1_LOCK_FIELD,
    &XPORT_REG_PON_AE_SERDES_STATUS_PLL0_LOCK_FIELD,
    &XPORT_REG_PON_AE_SERDES_STATUS_LINK_STATUS_FIELD,
    &XPORT_REG_PON_AE_SERDES_STATUS_CDR_LOCK_FIELD,
    &XPORT_REG_PON_AE_SERDES_STATUS_RX_SIGDET_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec XPORT_REG_PON_AE_SERDES_STATUS_REG = 
{
    "PON_AE_SERDES_STATUS",
#if RU_INCLUDE_DESC
    "PON AE SERDES Status Register",
    "",
#endif
    XPORT_REG_PON_AE_SERDES_STATUS_REG_OFFSET,
    0,
    0,
    182,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    8,
    XPORT_REG_PON_AE_SERDES_STATUS_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Block: XPORT_REG
 ******************************************************************************/
static const ru_reg_rec *XPORT_REG_REGS[] =
{
    &XPORT_REG_XPORT_REVISION_REG,
    &XPORT_REG_LED_PWM_CNTRL_REG,
    &XPORT_REG_LED_INTENSITY_CNTRL_REG,
    &XPORT_REG_LED_0_CNTRL_REG,
    &XPORT_REG_LED_0_LINK_AND_SPEED_ENCODING_SEL_REG,
    &XPORT_REG_LED_0_LINK_AND_SPEED_ENCODING_REG,
    &XPORT_REG_LED_1_CNTRL_REG,
    &XPORT_REG_LED_1_LINK_AND_SPEED_ENCODING_SEL_REG,
    &XPORT_REG_LED_1_LINK_AND_SPEED_ENCODING_REG,
    &XPORT_REG_LED_BLINK_RATE_CNTRL_REG,
    &XPORT_REG_LED_SERIAL_CNTRL_REG,
    &XPORT_REG_REFRESH_PERIOD_CNTRL_REG,
    &XPORT_REG_AGGREGATE_LED_CNTRL_REG,
    &XPORT_REG_AGGREGATE_LED_BLINK_RATE_CNTRL_REG,
    &XPORT_REG_SPARE_CNTRL_REG,
    &XPORT_REG_XPORT_CNTRL_1_REG,
    &XPORT_REG_CROSSBAR_STATUS_REG,
    &XPORT_REG_PON_AE_SERDES_STATUS_REG,
};

unsigned long XPORT_REG_ADDRS[] =
{
    0x8013a004,
};

const ru_block_rec XPORT_REG_BLOCK = 
{
    "XPORT_REG",
    XPORT_REG_ADDRS,
    1,
    18,
    XPORT_REG_REGS
};

/* End of file XPORT_REG.c */
