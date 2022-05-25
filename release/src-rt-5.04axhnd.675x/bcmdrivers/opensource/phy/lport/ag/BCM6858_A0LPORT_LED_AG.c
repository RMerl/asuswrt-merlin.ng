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
 * Field: LPORT_LED_CNTRL_RESERVED0
 ******************************************************************************/
const ru_field_rec LPORT_LED_CNTRL_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    LPORT_LED_CNTRL_RESERVED0_FIELD_MASK,
    0,
    LPORT_LED_CNTRL_RESERVED0_FIELD_WIDTH,
    LPORT_LED_CNTRL_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_LED_CNTRL_LNK_OVRD_EN
 ******************************************************************************/
const ru_field_rec LPORT_LED_CNTRL_LNK_OVRD_EN_FIELD =
{
    "LNK_OVRD_EN",
#if RU_INCLUDE_DESC
    "",
    "When this bit is set MAC/PHY provided link indication is overridden using lnk_status_ovrd.",
#endif
    LPORT_LED_CNTRL_LNK_OVRD_EN_FIELD_MASK,
    0,
    LPORT_LED_CNTRL_LNK_OVRD_EN_FIELD_WIDTH,
    LPORT_LED_CNTRL_LNK_OVRD_EN_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_LED_CNTRL_SPD_OVRD_EN
 ******************************************************************************/
const ru_field_rec LPORT_LED_CNTRL_SPD_OVRD_EN_FIELD =
{
    "SPD_OVRD_EN",
#if RU_INCLUDE_DESC
    "",
    "When this bit is set MAC/PHY provided speed indications are overridden using led_spd_ovrd[2:0].",
#endif
    LPORT_LED_CNTRL_SPD_OVRD_EN_FIELD_MASK,
    0,
    LPORT_LED_CNTRL_SPD_OVRD_EN_FIELD_WIDTH,
    LPORT_LED_CNTRL_SPD_OVRD_EN_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_LED_CNTRL_LNK_STATUS_OVRD
 ******************************************************************************/
const ru_field_rec LPORT_LED_CNTRL_LNK_STATUS_OVRD_FIELD =
{
    "LNK_STATUS_OVRD",
#if RU_INCLUDE_DESC
    "",
    "Link status override. Used only for LED.",
#endif
    LPORT_LED_CNTRL_LNK_STATUS_OVRD_FIELD_MASK,
    0,
    LPORT_LED_CNTRL_LNK_STATUS_OVRD_FIELD_WIDTH,
    LPORT_LED_CNTRL_LNK_STATUS_OVRD_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_LED_CNTRL_LED_SPD_OVRD
 ******************************************************************************/
const ru_field_rec LPORT_LED_CNTRL_LED_SPD_OVRD_FIELD =
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
    LPORT_LED_CNTRL_LED_SPD_OVRD_FIELD_MASK,
    0,
    LPORT_LED_CNTRL_LED_SPD_OVRD_FIELD_WIDTH,
    LPORT_LED_CNTRL_LED_SPD_OVRD_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_LED_CNTRL_ACT_LED_POL_SEL
 ******************************************************************************/
const ru_field_rec LPORT_LED_CNTRL_ACT_LED_POL_SEL_FIELD =
{
    "ACT_LED_POL_SEL",
#if RU_INCLUDE_DESC
    "",
    "When set to 1'b1 inverts polarity of the activity signal that is used for ACT_LED.",
#endif
    LPORT_LED_CNTRL_ACT_LED_POL_SEL_FIELD_MASK,
    0,
    LPORT_LED_CNTRL_ACT_LED_POL_SEL_FIELD_WIDTH,
    LPORT_LED_CNTRL_ACT_LED_POL_SEL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_LED_CNTRL_SPDLNK_LED2_ACT_POL_SEL
 ******************************************************************************/
const ru_field_rec LPORT_LED_CNTRL_SPDLNK_LED2_ACT_POL_SEL_FIELD =
{
    "SPDLNK_LED2_ACT_POL_SEL",
#if RU_INCLUDE_DESC
    "",
    "When set to 1'b1 inverts polarity of the activity signal that is used for SPDLNK_LED[2]. "
    "Applicable only when the activity drives this LED.",
#endif
    LPORT_LED_CNTRL_SPDLNK_LED2_ACT_POL_SEL_FIELD_MASK,
    0,
    LPORT_LED_CNTRL_SPDLNK_LED2_ACT_POL_SEL_FIELD_WIDTH,
    LPORT_LED_CNTRL_SPDLNK_LED2_ACT_POL_SEL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_LED_CNTRL_SPDLNK_LED1_ACT_POL_SEL
 ******************************************************************************/
const ru_field_rec LPORT_LED_CNTRL_SPDLNK_LED1_ACT_POL_SEL_FIELD =
{
    "SPDLNK_LED1_ACT_POL_SEL",
#if RU_INCLUDE_DESC
    "",
    "When set to 1'b1 inverts polarity of the activity signal that is used for SPDLNK_LED[1]. "
    "Applicable only when the activity drives this LED.",
#endif
    LPORT_LED_CNTRL_SPDLNK_LED1_ACT_POL_SEL_FIELD_MASK,
    0,
    LPORT_LED_CNTRL_SPDLNK_LED1_ACT_POL_SEL_FIELD_WIDTH,
    LPORT_LED_CNTRL_SPDLNK_LED1_ACT_POL_SEL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_LED_CNTRL_SPDLNK_LED0_ACT_POL_SEL
 ******************************************************************************/
const ru_field_rec LPORT_LED_CNTRL_SPDLNK_LED0_ACT_POL_SEL_FIELD =
{
    "SPDLNK_LED0_ACT_POL_SEL",
#if RU_INCLUDE_DESC
    "",
    "When set to 1'b1 inverts polarity of the activity signal that is used for SPDLNK_LED[0]. "
    "Applicable only when the activity drives this LED.",
#endif
    LPORT_LED_CNTRL_SPDLNK_LED0_ACT_POL_SEL_FIELD_MASK,
    0,
    LPORT_LED_CNTRL_SPDLNK_LED0_ACT_POL_SEL_FIELD_WIDTH,
    LPORT_LED_CNTRL_SPDLNK_LED0_ACT_POL_SEL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_LED_CNTRL_ACT_LED_ACT_SEL
 ******************************************************************************/
const ru_field_rec LPORT_LED_CNTRL_ACT_LED_ACT_SEL_FIELD =
{
    "ACT_LED_ACT_SEL",
#if RU_INCLUDE_DESC
    "",
    "Selects source of activity for ACT_LED. For encoding see description for spdlnk_led0_act_sel.",
#endif
    LPORT_LED_CNTRL_ACT_LED_ACT_SEL_FIELD_MASK,
    0,
    LPORT_LED_CNTRL_ACT_LED_ACT_SEL_FIELD_WIDTH,
    LPORT_LED_CNTRL_ACT_LED_ACT_SEL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_LED_CNTRL_SPDLNK_LED2_ACT_SEL
 ******************************************************************************/
const ru_field_rec LPORT_LED_CNTRL_SPDLNK_LED2_ACT_SEL_FIELD =
{
    "SPDLNK_LED2_ACT_SEL",
#if RU_INCLUDE_DESC
    "",
    "Selects source of activity for SPDLNK_LED[2]. For encoding see description for spdlnk_led0_act_sel.",
#endif
    LPORT_LED_CNTRL_SPDLNK_LED2_ACT_SEL_FIELD_MASK,
    0,
    LPORT_LED_CNTRL_SPDLNK_LED2_ACT_SEL_FIELD_WIDTH,
    LPORT_LED_CNTRL_SPDLNK_LED2_ACT_SEL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_LED_CNTRL_SPDLNK_LED1_ACT_SEL
 ******************************************************************************/
const ru_field_rec LPORT_LED_CNTRL_SPDLNK_LED1_ACT_SEL_FIELD =
{
    "SPDLNK_LED1_ACT_SEL",
#if RU_INCLUDE_DESC
    "",
    "Selects source of activity for SPDLNK_LED[1]. For encoding see description for spdlnk_led0_act_sel.",
#endif
    LPORT_LED_CNTRL_SPDLNK_LED1_ACT_SEL_FIELD_MASK,
    0,
    LPORT_LED_CNTRL_SPDLNK_LED1_ACT_SEL_FIELD_WIDTH,
    LPORT_LED_CNTRL_SPDLNK_LED1_ACT_SEL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_LED_CNTRL_SPDLNK_LED0_ACT_SEL
 ******************************************************************************/
const ru_field_rec LPORT_LED_CNTRL_SPDLNK_LED0_ACT_SEL_FIELD =
{
    "SPDLNK_LED0_ACT_SEL",
#if RU_INCLUDE_DESC
    "",
    "Selects source of the activity for SPDLNK_LED[0]:\n"
    "0 : LED is 0 when link is up and blinks when there is activity.\n"
    "1 : LED is 1 and blinks when there is activity.",
#endif
    LPORT_LED_CNTRL_SPDLNK_LED0_ACT_SEL_FIELD_MASK,
    0,
    LPORT_LED_CNTRL_SPDLNK_LED0_ACT_SEL_FIELD_WIDTH,
    LPORT_LED_CNTRL_SPDLNK_LED0_ACT_SEL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_LED_CNTRL_TX_ACT_EN
 ******************************************************************************/
const ru_field_rec LPORT_LED_CNTRL_TX_ACT_EN_FIELD =
{
    "TX_ACT_EN",
#if RU_INCLUDE_DESC
    "",
    "Enables TX_SOP event to contribute to the activity.",
#endif
    LPORT_LED_CNTRL_TX_ACT_EN_FIELD_MASK,
    0,
    LPORT_LED_CNTRL_TX_ACT_EN_FIELD_WIDTH,
    LPORT_LED_CNTRL_TX_ACT_EN_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_LED_CNTRL_RX_ACT_EN
 ******************************************************************************/
const ru_field_rec LPORT_LED_CNTRL_RX_ACT_EN_FIELD =
{
    "RX_ACT_EN",
#if RU_INCLUDE_DESC
    "",
    "Enables RX_SOP event to contribute to the activity.",
#endif
    LPORT_LED_CNTRL_RX_ACT_EN_FIELD_MASK,
    0,
    LPORT_LED_CNTRL_RX_ACT_EN_FIELD_WIDTH,
    LPORT_LED_CNTRL_RX_ACT_EN_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_LED_LINK_AND_SPEED_ENCODING_SEL_RESERVED0
 ******************************************************************************/
const ru_field_rec LPORT_LED_LINK_AND_SPEED_ENCODING_SEL_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    LPORT_LED_LINK_AND_SPEED_ENCODING_SEL_RESERVED0_FIELD_MASK,
    0,
    LPORT_LED_LINK_AND_SPEED_ENCODING_SEL_RESERVED0_FIELD_WIDTH,
    LPORT_LED_LINK_AND_SPEED_ENCODING_SEL_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_LED_LINK_AND_SPEED_ENCODING_SEL_RSVD_SEL_SPD_ENCODE_2
 ******************************************************************************/
const ru_field_rec LPORT_LED_LINK_AND_SPEED_ENCODING_SEL_RSVD_SEL_SPD_ENCODE_2_FIELD =
{
    "RSVD_SEL_SPD_ENCODE_2",
#if RU_INCLUDE_DESC
    "",
    "Reserved SPDLNK_LED_SEL[2:0] encoding select.",
#endif
    LPORT_LED_LINK_AND_SPEED_ENCODING_SEL_RSVD_SEL_SPD_ENCODE_2_FIELD_MASK,
    0,
    LPORT_LED_LINK_AND_SPEED_ENCODING_SEL_RSVD_SEL_SPD_ENCODE_2_FIELD_WIDTH,
    LPORT_LED_LINK_AND_SPEED_ENCODING_SEL_RSVD_SEL_SPD_ENCODE_2_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_LED_LINK_AND_SPEED_ENCODING_SEL_RSVD_SEL_SPD_ENCODE_1
 ******************************************************************************/
const ru_field_rec LPORT_LED_LINK_AND_SPEED_ENCODING_SEL_RSVD_SEL_SPD_ENCODE_1_FIELD =
{
    "RSVD_SEL_SPD_ENCODE_1",
#if RU_INCLUDE_DESC
    "",
    "Reserved SPDLNK_LED_SEL[2:0] encoding select.",
#endif
    LPORT_LED_LINK_AND_SPEED_ENCODING_SEL_RSVD_SEL_SPD_ENCODE_1_FIELD_MASK,
    0,
    LPORT_LED_LINK_AND_SPEED_ENCODING_SEL_RSVD_SEL_SPD_ENCODE_1_FIELD_WIDTH,
    LPORT_LED_LINK_AND_SPEED_ENCODING_SEL_RSVD_SEL_SPD_ENCODE_1_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_LED_LINK_AND_SPEED_ENCODING_SEL_SEL_10G_ENCODE
 ******************************************************************************/
const ru_field_rec LPORT_LED_LINK_AND_SPEED_ENCODING_SEL_SEL_10G_ENCODE_FIELD =
{
    "SEL_10G_ENCODE",
#if RU_INCLUDE_DESC
    "",
    "SPDLNK_LED_SEL[2:0] encoding for 10Gb/s and higher link speed.\n"
    "When SPDLNK_LED_SEL[x] = 1'''b0, SPDLNK_LED[x] is driven by bits [17:0] of Link and Speed Encoding Register.\n"
    "When SPDLNK_LED_SEL[x] = 1'''b1, SPDLNK_LED[x] is driven by the activity.\n",
#endif
    LPORT_LED_LINK_AND_SPEED_ENCODING_SEL_SEL_10G_ENCODE_FIELD_MASK,
    0,
    LPORT_LED_LINK_AND_SPEED_ENCODING_SEL_SEL_10G_ENCODE_FIELD_WIDTH,
    LPORT_LED_LINK_AND_SPEED_ENCODING_SEL_SEL_10G_ENCODE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_LED_LINK_AND_SPEED_ENCODING_SEL_SEL_2500M_ENCODE
 ******************************************************************************/
const ru_field_rec LPORT_LED_LINK_AND_SPEED_ENCODING_SEL_SEL_2500M_ENCODE_FIELD =
{
    "SEL_2500M_ENCODE",
#if RU_INCLUDE_DESC
    "",
    "SPDLNK_LED_SEL[2:0] encoding for 2500Mb/s link speed.",
#endif
    LPORT_LED_LINK_AND_SPEED_ENCODING_SEL_SEL_2500M_ENCODE_FIELD_MASK,
    0,
    LPORT_LED_LINK_AND_SPEED_ENCODING_SEL_SEL_2500M_ENCODE_FIELD_WIDTH,
    LPORT_LED_LINK_AND_SPEED_ENCODING_SEL_SEL_2500M_ENCODE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_LED_LINK_AND_SPEED_ENCODING_SEL_SEL_1000M_ENCODE
 ******************************************************************************/
const ru_field_rec LPORT_LED_LINK_AND_SPEED_ENCODING_SEL_SEL_1000M_ENCODE_FIELD =
{
    "SEL_1000M_ENCODE",
#if RU_INCLUDE_DESC
    "",
    "SPDLNK_LED_SEL[2:0] encoding for 1000Mb/s link speed.",
#endif
    LPORT_LED_LINK_AND_SPEED_ENCODING_SEL_SEL_1000M_ENCODE_FIELD_MASK,
    0,
    LPORT_LED_LINK_AND_SPEED_ENCODING_SEL_SEL_1000M_ENCODE_FIELD_WIDTH,
    LPORT_LED_LINK_AND_SPEED_ENCODING_SEL_SEL_1000M_ENCODE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_LED_LINK_AND_SPEED_ENCODING_SEL_SEL_100M_ENCODE
 ******************************************************************************/
const ru_field_rec LPORT_LED_LINK_AND_SPEED_ENCODING_SEL_SEL_100M_ENCODE_FIELD =
{
    "SEL_100M_ENCODE",
#if RU_INCLUDE_DESC
    "",
    "SPDLNK_LED_SEL[2:0] encoding for 100Mb/s link speed.",
#endif
    LPORT_LED_LINK_AND_SPEED_ENCODING_SEL_SEL_100M_ENCODE_FIELD_MASK,
    0,
    LPORT_LED_LINK_AND_SPEED_ENCODING_SEL_SEL_100M_ENCODE_FIELD_WIDTH,
    LPORT_LED_LINK_AND_SPEED_ENCODING_SEL_SEL_100M_ENCODE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_LED_LINK_AND_SPEED_ENCODING_SEL_SEL_10M_ENCODE
 ******************************************************************************/
const ru_field_rec LPORT_LED_LINK_AND_SPEED_ENCODING_SEL_SEL_10M_ENCODE_FIELD =
{
    "SEL_10M_ENCODE",
#if RU_INCLUDE_DESC
    "",
    "SPDLNK_LED_SEL[2:0] encoding for 10Mb/s link speed.",
#endif
    LPORT_LED_LINK_AND_SPEED_ENCODING_SEL_SEL_10M_ENCODE_FIELD_MASK,
    0,
    LPORT_LED_LINK_AND_SPEED_ENCODING_SEL_SEL_10M_ENCODE_FIELD_WIDTH,
    LPORT_LED_LINK_AND_SPEED_ENCODING_SEL_SEL_10M_ENCODE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_LED_LINK_AND_SPEED_ENCODING_SEL_SEL_NO_LINK_ENCODE
 ******************************************************************************/
const ru_field_rec LPORT_LED_LINK_AND_SPEED_ENCODING_SEL_SEL_NO_LINK_ENCODE_FIELD =
{
    "SEL_NO_LINK_ENCODE",
#if RU_INCLUDE_DESC
    "",
    "SPDLNK_LED_SEL[2:0] encoding for the no-link state.",
#endif
    LPORT_LED_LINK_AND_SPEED_ENCODING_SEL_SEL_NO_LINK_ENCODE_FIELD_MASK,
    0,
    LPORT_LED_LINK_AND_SPEED_ENCODING_SEL_SEL_NO_LINK_ENCODE_FIELD_WIDTH,
    LPORT_LED_LINK_AND_SPEED_ENCODING_SEL_SEL_NO_LINK_ENCODE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_LED_LINK_AND_SPEED_ENCODING_RESERVED0
 ******************************************************************************/
const ru_field_rec LPORT_LED_LINK_AND_SPEED_ENCODING_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    LPORT_LED_LINK_AND_SPEED_ENCODING_RESERVED0_FIELD_MASK,
    0,
    LPORT_LED_LINK_AND_SPEED_ENCODING_RESERVED0_FIELD_WIDTH,
    LPORT_LED_LINK_AND_SPEED_ENCODING_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_LED_LINK_AND_SPEED_ENCODING_RSVD_SPD_ENCODE_2
 ******************************************************************************/
const ru_field_rec LPORT_LED_LINK_AND_SPEED_ENCODING_RSVD_SPD_ENCODE_2_FIELD =
{
    "RSVD_SPD_ENCODE_2",
#if RU_INCLUDE_DESC
    "",
    "Reserved SPDLNK_LED_SEL[2:0] encoding.",
#endif
    LPORT_LED_LINK_AND_SPEED_ENCODING_RSVD_SPD_ENCODE_2_FIELD_MASK,
    0,
    LPORT_LED_LINK_AND_SPEED_ENCODING_RSVD_SPD_ENCODE_2_FIELD_WIDTH,
    LPORT_LED_LINK_AND_SPEED_ENCODING_RSVD_SPD_ENCODE_2_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_LED_LINK_AND_SPEED_ENCODING_RSVD_SPD_ENCODE_1
 ******************************************************************************/
const ru_field_rec LPORT_LED_LINK_AND_SPEED_ENCODING_RSVD_SPD_ENCODE_1_FIELD =
{
    "RSVD_SPD_ENCODE_1",
#if RU_INCLUDE_DESC
    "",
    "Reserved SPDLNK_LED_SEL[2:0] encoding.",
#endif
    LPORT_LED_LINK_AND_SPEED_ENCODING_RSVD_SPD_ENCODE_1_FIELD_MASK,
    0,
    LPORT_LED_LINK_AND_SPEED_ENCODING_RSVD_SPD_ENCODE_1_FIELD_WIDTH,
    LPORT_LED_LINK_AND_SPEED_ENCODING_RSVD_SPD_ENCODE_1_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_LED_LINK_AND_SPEED_ENCODING_M10G_ENCODE
 ******************************************************************************/
const ru_field_rec LPORT_LED_LINK_AND_SPEED_ENCODING_M10G_ENCODE_FIELD =
{
    "M10G_ENCODE",
#if RU_INCLUDE_DESC
    "",
    "SPDLNK_LED[2:0] encoding for 10Gb/s and higherlink speed.",
#endif
    LPORT_LED_LINK_AND_SPEED_ENCODING_M10G_ENCODE_FIELD_MASK,
    0,
    LPORT_LED_LINK_AND_SPEED_ENCODING_M10G_ENCODE_FIELD_WIDTH,
    LPORT_LED_LINK_AND_SPEED_ENCODING_M10G_ENCODE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_LED_LINK_AND_SPEED_ENCODING_M2500_ENCODE
 ******************************************************************************/
const ru_field_rec LPORT_LED_LINK_AND_SPEED_ENCODING_M2500_ENCODE_FIELD =
{
    "M2500_ENCODE",
#if RU_INCLUDE_DESC
    "",
    "SPDLNK_LED[2:0] encoding for 2.5Gb/s link speed.",
#endif
    LPORT_LED_LINK_AND_SPEED_ENCODING_M2500_ENCODE_FIELD_MASK,
    0,
    LPORT_LED_LINK_AND_SPEED_ENCODING_M2500_ENCODE_FIELD_WIDTH,
    LPORT_LED_LINK_AND_SPEED_ENCODING_M2500_ENCODE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_LED_LINK_AND_SPEED_ENCODING_M1000_ENCODE
 ******************************************************************************/
const ru_field_rec LPORT_LED_LINK_AND_SPEED_ENCODING_M1000_ENCODE_FIELD =
{
    "M1000_ENCODE",
#if RU_INCLUDE_DESC
    "",
    "SPDLNK_LED[2:0] encoding for 1Gb/s link speed.",
#endif
    LPORT_LED_LINK_AND_SPEED_ENCODING_M1000_ENCODE_FIELD_MASK,
    0,
    LPORT_LED_LINK_AND_SPEED_ENCODING_M1000_ENCODE_FIELD_WIDTH,
    LPORT_LED_LINK_AND_SPEED_ENCODING_M1000_ENCODE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_LED_LINK_AND_SPEED_ENCODING_M100_ENCODE
 ******************************************************************************/
const ru_field_rec LPORT_LED_LINK_AND_SPEED_ENCODING_M100_ENCODE_FIELD =
{
    "M100_ENCODE",
#if RU_INCLUDE_DESC
    "",
    "SPDLNK_LED[2:0] encoding for 100Mb/s link speed.",
#endif
    LPORT_LED_LINK_AND_SPEED_ENCODING_M100_ENCODE_FIELD_MASK,
    0,
    LPORT_LED_LINK_AND_SPEED_ENCODING_M100_ENCODE_FIELD_WIDTH,
    LPORT_LED_LINK_AND_SPEED_ENCODING_M100_ENCODE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_LED_LINK_AND_SPEED_ENCODING_M10_ENCODE
 ******************************************************************************/
const ru_field_rec LPORT_LED_LINK_AND_SPEED_ENCODING_M10_ENCODE_FIELD =
{
    "M10_ENCODE",
#if RU_INCLUDE_DESC
    "",
    "SPDLNK_LED[2:0] encoding for 10Mb/s link speed.",
#endif
    LPORT_LED_LINK_AND_SPEED_ENCODING_M10_ENCODE_FIELD_MASK,
    0,
    LPORT_LED_LINK_AND_SPEED_ENCODING_M10_ENCODE_FIELD_WIDTH,
    LPORT_LED_LINK_AND_SPEED_ENCODING_M10_ENCODE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_LED_LINK_AND_SPEED_ENCODING_NO_LINK_ENCODE
 ******************************************************************************/
const ru_field_rec LPORT_LED_LINK_AND_SPEED_ENCODING_NO_LINK_ENCODE_FIELD =
{
    "NO_LINK_ENCODE",
#if RU_INCLUDE_DESC
    "",
    "SPDLNK_LED[2:0] encoding for the no-link state.",
#endif
    LPORT_LED_LINK_AND_SPEED_ENCODING_NO_LINK_ENCODE_FIELD_MASK,
    0,
    LPORT_LED_LINK_AND_SPEED_ENCODING_NO_LINK_ENCODE_FIELD_WIDTH,
    LPORT_LED_LINK_AND_SPEED_ENCODING_NO_LINK_ENCODE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_LED_AGGREGATE_LED_CNTRL_RESERVED0
 ******************************************************************************/
const ru_field_rec LPORT_LED_AGGREGATE_LED_CNTRL_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    LPORT_LED_AGGREGATE_LED_CNTRL_RESERVED0_FIELD_MASK,
    0,
    LPORT_LED_AGGREGATE_LED_CNTRL_RESERVED0_FIELD_WIDTH,
    LPORT_LED_AGGREGATE_LED_CNTRL_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_LED_AGGREGATE_LED_CNTRL_LNK_POL_SEL
 ******************************************************************************/
const ru_field_rec LPORT_LED_AGGREGATE_LED_CNTRL_LNK_POL_SEL_FIELD =
{
    "LNK_POL_SEL",
#if RU_INCLUDE_DESC
    "",
    "When set to 1'b1 inverts polarity of the link signal that is used for aggregate LNK_LED.",
#endif
    LPORT_LED_AGGREGATE_LED_CNTRL_LNK_POL_SEL_FIELD_MASK,
    0,
    LPORT_LED_AGGREGATE_LED_CNTRL_LNK_POL_SEL_FIELD_WIDTH,
    LPORT_LED_AGGREGATE_LED_CNTRL_LNK_POL_SEL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_LED_AGGREGATE_LED_CNTRL_ACT_POL_SEL
 ******************************************************************************/
const ru_field_rec LPORT_LED_AGGREGATE_LED_CNTRL_ACT_POL_SEL_FIELD =
{
    "ACT_POL_SEL",
#if RU_INCLUDE_DESC
    "",
    "When set to 1'b1 inverts polarity of the activity signal that is used for aggregate ACT_LED.",
#endif
    LPORT_LED_AGGREGATE_LED_CNTRL_ACT_POL_SEL_FIELD_MASK,
    0,
    LPORT_LED_AGGREGATE_LED_CNTRL_ACT_POL_SEL_FIELD_WIDTH,
    LPORT_LED_AGGREGATE_LED_CNTRL_ACT_POL_SEL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_LED_AGGREGATE_LED_CNTRL_ACT_SEL
 ******************************************************************************/
const ru_field_rec LPORT_LED_AGGREGATE_LED_CNTRL_ACT_SEL_FIELD =
{
    "ACT_SEL",
#if RU_INCLUDE_DESC
    "",
    "Selects behavior for aggregate ACT_LED. Encoded as:"
    "0 : LED is 0 when aggregate link is up and blinks when there is activity. "
    "    LED is 1 when aggregate link is down.\n"
    "1 : LED is 1 and blinks when there is activity, regardless of the aggregate link status.\n",
#endif
    LPORT_LED_AGGREGATE_LED_CNTRL_ACT_SEL_FIELD_MASK,
    0,
    LPORT_LED_AGGREGATE_LED_CNTRL_ACT_SEL_FIELD_WIDTH,
    LPORT_LED_AGGREGATE_LED_CNTRL_ACT_SEL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_LED_AGGREGATE_LED_CNTRL_PORT_EN
 ******************************************************************************/
const ru_field_rec LPORT_LED_AGGREGATE_LED_CNTRL_PORT_EN_FIELD =
{
    "PORT_EN",
#if RU_INCLUDE_DESC
    "",
    "When the corresponding bit is set, port LEDs are included in aggregate LED signals. "
    "When all bits are cleared, aggregate LED interface is disabled.",
#endif
    LPORT_LED_AGGREGATE_LED_CNTRL_PORT_EN_FIELD_MASK,
    0,
    LPORT_LED_AGGREGATE_LED_CNTRL_PORT_EN_FIELD_WIDTH,
    LPORT_LED_AGGREGATE_LED_CNTRL_PORT_EN_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_LED_AGGREGATE_LED_BLINK_RATE_CNTRL_LED_ON_TIME
 ******************************************************************************/
const ru_field_rec LPORT_LED_AGGREGATE_LED_BLINK_RATE_CNTRL_LED_ON_TIME_FIELD =
{
    "LED_ON_TIME",
#if RU_INCLUDE_DESC
    "",
    "Led ON time. Expressed in 50us units.",
#endif
    LPORT_LED_AGGREGATE_LED_BLINK_RATE_CNTRL_LED_ON_TIME_FIELD_MASK,
    0,
    LPORT_LED_AGGREGATE_LED_BLINK_RATE_CNTRL_LED_ON_TIME_FIELD_WIDTH,
    LPORT_LED_AGGREGATE_LED_BLINK_RATE_CNTRL_LED_ON_TIME_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: LPORT_LED_AGGREGATE_LED_BLINK_RATE_CNTRL_LED_OFF_TIME
 ******************************************************************************/
const ru_field_rec LPORT_LED_AGGREGATE_LED_BLINK_RATE_CNTRL_LED_OFF_TIME_FIELD =
{
    "LED_OFF_TIME",
#if RU_INCLUDE_DESC
    "",
    "Led OFF time. Expressed in 50us  units.",
#endif
    LPORT_LED_AGGREGATE_LED_BLINK_RATE_CNTRL_LED_OFF_TIME_FIELD_MASK,
    0,
    LPORT_LED_AGGREGATE_LED_BLINK_RATE_CNTRL_LED_OFF_TIME_FIELD_WIDTH,
    LPORT_LED_AGGREGATE_LED_BLINK_RATE_CNTRL_LED_OFF_TIME_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

#endif /* RU_INCLUDE_FIELD_DB */

/******************************************************************************
 * Register: LPORT_LED_CNTRL
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *LPORT_LED_CNTRL_FIELDS[] =
{
    &LPORT_LED_CNTRL_RESERVED0_FIELD,
    &LPORT_LED_CNTRL_LNK_OVRD_EN_FIELD,
    &LPORT_LED_CNTRL_SPD_OVRD_EN_FIELD,
    &LPORT_LED_CNTRL_LNK_STATUS_OVRD_FIELD,
    &LPORT_LED_CNTRL_LED_SPD_OVRD_FIELD,
    &LPORT_LED_CNTRL_ACT_LED_POL_SEL_FIELD,
    &LPORT_LED_CNTRL_SPDLNK_LED2_ACT_POL_SEL_FIELD,
    &LPORT_LED_CNTRL_SPDLNK_LED1_ACT_POL_SEL_FIELD,
    &LPORT_LED_CNTRL_SPDLNK_LED0_ACT_POL_SEL_FIELD,
    &LPORT_LED_CNTRL_ACT_LED_ACT_SEL_FIELD,
    &LPORT_LED_CNTRL_SPDLNK_LED2_ACT_SEL_FIELD,
    &LPORT_LED_CNTRL_SPDLNK_LED1_ACT_SEL_FIELD,
    &LPORT_LED_CNTRL_SPDLNK_LED0_ACT_SEL_FIELD,
    &LPORT_LED_CNTRL_TX_ACT_EN_FIELD,
    &LPORT_LED_CNTRL_RX_ACT_EN_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec LPORT_LED_CNTRL_REG = 
{
    "CNTRL",
#if RU_INCLUDE_DESC
    "LED 7 Control Register",
    "",
#endif
    LPORT_LED_CNTRL_REG_OFFSET,
    0,
    0,
    200,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    15,
    LPORT_LED_CNTRL_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: LPORT_LED_LINK_AND_SPEED_ENCODING_SEL
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *LPORT_LED_LINK_AND_SPEED_ENCODING_SEL_FIELDS[] =
{
    &LPORT_LED_LINK_AND_SPEED_ENCODING_SEL_RESERVED0_FIELD,
    &LPORT_LED_LINK_AND_SPEED_ENCODING_SEL_RSVD_SEL_SPD_ENCODE_2_FIELD,
    &LPORT_LED_LINK_AND_SPEED_ENCODING_SEL_RSVD_SEL_SPD_ENCODE_1_FIELD,
    &LPORT_LED_LINK_AND_SPEED_ENCODING_SEL_SEL_10G_ENCODE_FIELD,
    &LPORT_LED_LINK_AND_SPEED_ENCODING_SEL_SEL_2500M_ENCODE_FIELD,
    &LPORT_LED_LINK_AND_SPEED_ENCODING_SEL_SEL_1000M_ENCODE_FIELD,
    &LPORT_LED_LINK_AND_SPEED_ENCODING_SEL_SEL_100M_ENCODE_FIELD,
    &LPORT_LED_LINK_AND_SPEED_ENCODING_SEL_SEL_10M_ENCODE_FIELD,
    &LPORT_LED_LINK_AND_SPEED_ENCODING_SEL_SEL_NO_LINK_ENCODE_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec LPORT_LED_LINK_AND_SPEED_ENCODING_SEL_REG = 
{
    "LINK_AND_SPEED_ENCODING_SEL",
#if RU_INCLUDE_DESC
    "LED 7 Link And Speed Encoding Selection Register",
    "",
#endif
    LPORT_LED_LINK_AND_SPEED_ENCODING_SEL_REG_OFFSET,
    0,
    0,
    201,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    9,
    LPORT_LED_LINK_AND_SPEED_ENCODING_SEL_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: LPORT_LED_LINK_AND_SPEED_ENCODING
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *LPORT_LED_LINK_AND_SPEED_ENCODING_FIELDS[] =
{
    &LPORT_LED_LINK_AND_SPEED_ENCODING_RESERVED0_FIELD,
    &LPORT_LED_LINK_AND_SPEED_ENCODING_RSVD_SPD_ENCODE_2_FIELD,
    &LPORT_LED_LINK_AND_SPEED_ENCODING_RSVD_SPD_ENCODE_1_FIELD,
    &LPORT_LED_LINK_AND_SPEED_ENCODING_M10G_ENCODE_FIELD,
    &LPORT_LED_LINK_AND_SPEED_ENCODING_M2500_ENCODE_FIELD,
    &LPORT_LED_LINK_AND_SPEED_ENCODING_M1000_ENCODE_FIELD,
    &LPORT_LED_LINK_AND_SPEED_ENCODING_M100_ENCODE_FIELD,
    &LPORT_LED_LINK_AND_SPEED_ENCODING_M10_ENCODE_FIELD,
    &LPORT_LED_LINK_AND_SPEED_ENCODING_NO_LINK_ENCODE_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec LPORT_LED_LINK_AND_SPEED_ENCODING_REG = 
{
    "LINK_AND_SPEED_ENCODING",
#if RU_INCLUDE_DESC
    "LED 7 Link And Speed Encoding Register",
    "",
#endif
    LPORT_LED_LINK_AND_SPEED_ENCODING_REG_OFFSET,
    0,
    0,
    202,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    9,
    LPORT_LED_LINK_AND_SPEED_ENCODING_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: LPORT_LED_AGGREGATE_LED_CNTRL
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *LPORT_LED_AGGREGATE_LED_CNTRL_FIELDS[] =
{
    &LPORT_LED_AGGREGATE_LED_CNTRL_RESERVED0_FIELD,
    &LPORT_LED_AGGREGATE_LED_CNTRL_LNK_POL_SEL_FIELD,
    &LPORT_LED_AGGREGATE_LED_CNTRL_ACT_POL_SEL_FIELD,
    &LPORT_LED_AGGREGATE_LED_CNTRL_ACT_SEL_FIELD,
    &LPORT_LED_AGGREGATE_LED_CNTRL_PORT_EN_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec LPORT_LED_AGGREGATE_LED_CNTRL_REG = 
{
    "AGGREGATE_LED_CNTRL",
#if RU_INCLUDE_DESC
    "Aggregate LED Control Register",
    "",
#endif
    LPORT_LED_AGGREGATE_LED_CNTRL_REG_OFFSET,
    0,
    0,
    203,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    5,
    LPORT_LED_AGGREGATE_LED_CNTRL_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: LPORT_LED_AGGREGATE_LED_BLINK_RATE_CNTRL
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *LPORT_LED_AGGREGATE_LED_BLINK_RATE_CNTRL_FIELDS[] =
{
    &LPORT_LED_AGGREGATE_LED_BLINK_RATE_CNTRL_LED_ON_TIME_FIELD,
    &LPORT_LED_AGGREGATE_LED_BLINK_RATE_CNTRL_LED_OFF_TIME_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec LPORT_LED_AGGREGATE_LED_BLINK_RATE_CNTRL_REG = 
{
    "AGGREGATE_LED_BLINK_RATE_CNTRL",
#if RU_INCLUDE_DESC
    "Aggregate LED Blink Rate Control Register",
    "",
#endif
    LPORT_LED_AGGREGATE_LED_BLINK_RATE_CNTRL_REG_OFFSET,
    0,
    0,
    204,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    LPORT_LED_AGGREGATE_LED_BLINK_RATE_CNTRL_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Block: LPORT_LED
 ******************************************************************************/
static const ru_reg_rec *LPORT_LED_REGS[] =
{
    &LPORT_LED_CNTRL_REG,
    &LPORT_LED_LINK_AND_SPEED_ENCODING_SEL_REG,
    &LPORT_LED_LINK_AND_SPEED_ENCODING_REG,
    &LPORT_LED_AGGREGATE_LED_CNTRL_REG,
    &LPORT_LED_AGGREGATE_LED_BLINK_RATE_CNTRL_REG,
};

unsigned long LPORT_LED_ADDRS[] =
{
    0x8013c074,
    0x8013c080,
    0x8013c08c,
    0x8013c098,
    0x8013c0a4,
    0x8013c0b0,
    0x8013c0bc,
    0x8013c0c8,
};

const ru_block_rec LPORT_LED_BLOCK = 
{
    "LPORT_LED",
    LPORT_LED_ADDRS,
    8,
    5,
    LPORT_LED_REGS
};

/* End of file BCM6858_A0LPORT_LED.c */
