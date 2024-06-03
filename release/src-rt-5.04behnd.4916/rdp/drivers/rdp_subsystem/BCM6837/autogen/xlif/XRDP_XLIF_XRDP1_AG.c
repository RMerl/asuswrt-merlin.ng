/*
   Copyright (c) 2015 Broadcom
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


#include "XRDP_XLIF_XRDP1_AG.h"

/******************************************************************************
 * Register: NAME: XLIF_XRDP1_CHANNEL_XLIF_RX_IF_IF_DIS, TYPE: Type_CHANNEL_XLIF_RX_IF_IF_DIS
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: DISABLE *****/
const ru_field_rec XLIF_XRDP1_CHANNEL_XLIF_RX_IF_IF_DIS_DISABLE_FIELD =
{
    "DISABLE",
#if RU_INCLUDE_DESC
    "",
    "Disable\n",
#endif
    { XLIF_XRDP1_CHANNEL_XLIF_RX_IF_IF_DIS_DISABLE_FIELD_MASK },
    0,
    { XLIF_XRDP1_CHANNEL_XLIF_RX_IF_IF_DIS_DISABLE_FIELD_WIDTH },
    { XLIF_XRDP1_CHANNEL_XLIF_RX_IF_IF_DIS_DISABLE_FIELD_SHIFT },
    1,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XLIF_XRDP1_CHANNEL_XLIF_RX_IF_IF_DIS_FIELDS[] =
{
    &XLIF_XRDP1_CHANNEL_XLIF_RX_IF_IF_DIS_DISABLE_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: XLIF_XRDP1_CHANNEL_XLIF_RX_IF_IF_DIS *****/
const ru_reg_rec XLIF_XRDP1_CHANNEL_XLIF_RX_IF_IF_DIS_REG =
{
    "CHANNEL_XLIF_RX_IF_IF_DIS",
#if RU_INCLUDE_DESC
    "INTERFACE_DISABLE Register",
    "Interface_Disable\n",
#endif
    { XLIF_XRDP1_CHANNEL_XLIF_RX_IF_IF_DIS_REG_OFFSET },
    0,
    0,
    1123,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    XLIF_XRDP1_CHANNEL_XLIF_RX_IF_IF_DIS_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: XLIF_XRDP1_CHANNEL_XLIF_RX_IF_OFLW_FLAG, TYPE: Type_CHANNEL_XLIF_RX_IF_OFLW_FLAG
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: OFLW *****/
const ru_field_rec XLIF_XRDP1_CHANNEL_XLIF_RX_IF_OFLW_FLAG_OFLW_FIELD =
{
    "OFLW",
#if RU_INCLUDE_DESC
    "",
    "Overflow\n",
#endif
    { XLIF_XRDP1_CHANNEL_XLIF_RX_IF_OFLW_FLAG_OFLW_FIELD_MASK },
    0,
    { XLIF_XRDP1_CHANNEL_XLIF_RX_IF_OFLW_FLAG_OFLW_FIELD_WIDTH },
    { XLIF_XRDP1_CHANNEL_XLIF_RX_IF_OFLW_FLAG_OFLW_FIELD_SHIFT },
    1,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XLIF_XRDP1_CHANNEL_XLIF_RX_IF_OFLW_FLAG_FIELDS[] =
{
    &XLIF_XRDP1_CHANNEL_XLIF_RX_IF_OFLW_FLAG_OFLW_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: XLIF_XRDP1_CHANNEL_XLIF_RX_IF_OFLW_FLAG *****/
const ru_reg_rec XLIF_XRDP1_CHANNEL_XLIF_RX_IF_OFLW_FLAG_REG =
{
    "CHANNEL_XLIF_RX_IF_OFLW_FLAG",
#if RU_INCLUDE_DESC
    "OVRFLOW_FLAG Register",
    "Indicate an overflow event (data valid while FIFO is full).\nread clear\n",
#endif
    { XLIF_XRDP1_CHANNEL_XLIF_RX_IF_OFLW_FLAG_REG_OFFSET },
    0,
    0,
    1124,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    XLIF_XRDP1_CHANNEL_XLIF_RX_IF_OFLW_FLAG_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: XLIF_XRDP1_CHANNEL_XLIF_RX_IF_ERR_FLAG, TYPE: Type_CHANNEL_XLIF_RX_IF_ERR_FLAG
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: ERR *****/
const ru_field_rec XLIF_XRDP1_CHANNEL_XLIF_RX_IF_ERR_FLAG_ERR_FIELD =
{
    "ERR",
#if RU_INCLUDE_DESC
    "",
    "Error\n",
#endif
    { XLIF_XRDP1_CHANNEL_XLIF_RX_IF_ERR_FLAG_ERR_FIELD_MASK },
    0,
    { XLIF_XRDP1_CHANNEL_XLIF_RX_IF_ERR_FLAG_ERR_FIELD_WIDTH },
    { XLIF_XRDP1_CHANNEL_XLIF_RX_IF_ERR_FLAG_ERR_FIELD_SHIFT },
    1,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XLIF_XRDP1_CHANNEL_XLIF_RX_IF_ERR_FLAG_FIELDS[] =
{
    &XLIF_XRDP1_CHANNEL_XLIF_RX_IF_ERR_FLAG_ERR_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: XLIF_XRDP1_CHANNEL_XLIF_RX_IF_ERR_FLAG *****/
const ru_reg_rec XLIF_XRDP1_CHANNEL_XLIF_RX_IF_ERR_FLAG_REG =
{
    "CHANNEL_XLIF_RX_IF_ERR_FLAG",
#if RU_INCLUDE_DESC
    "PROTOCOL_ERR Register",
    "Indicate RX protocol Error.\nread clear\n",
#endif
    { XLIF_XRDP1_CHANNEL_XLIF_RX_IF_ERR_FLAG_REG_OFFSET },
    0,
    0,
    1125,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    XLIF_XRDP1_CHANNEL_XLIF_RX_IF_ERR_FLAG_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: XLIF_XRDP1_CHANNEL_XLIF_RX_FLOW_CONTROL_COSMAP_EN, TYPE: Type_CHANNEL_XLIF_RX_FLOW_CONTROL_COSMAP_EN
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: PFC_EN *****/
const ru_field_rec XLIF_XRDP1_CHANNEL_XLIF_RX_FLOW_CONTROL_COSMAP_EN_PFC_EN_FIELD =
{
    "PFC_EN",
#if RU_INCLUDE_DESC
    "",
    "PFC_EN\n",
#endif
    { XLIF_XRDP1_CHANNEL_XLIF_RX_FLOW_CONTROL_COSMAP_EN_PFC_EN_FIELD_MASK },
    0,
    { XLIF_XRDP1_CHANNEL_XLIF_RX_FLOW_CONTROL_COSMAP_EN_PFC_EN_FIELD_WIDTH },
    { XLIF_XRDP1_CHANNEL_XLIF_RX_FLOW_CONTROL_COSMAP_EN_PFC_EN_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: LLFC_EN *****/
const ru_field_rec XLIF_XRDP1_CHANNEL_XLIF_RX_FLOW_CONTROL_COSMAP_EN_LLFC_EN_FIELD =
{
    "LLFC_EN",
#if RU_INCLUDE_DESC
    "",
    "LLFC_en\n",
#endif
    { XLIF_XRDP1_CHANNEL_XLIF_RX_FLOW_CONTROL_COSMAP_EN_LLFC_EN_FIELD_MASK },
    0,
    { XLIF_XRDP1_CHANNEL_XLIF_RX_FLOW_CONTROL_COSMAP_EN_LLFC_EN_FIELD_WIDTH },
    { XLIF_XRDP1_CHANNEL_XLIF_RX_FLOW_CONTROL_COSMAP_EN_LLFC_EN_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XLIF_XRDP1_CHANNEL_XLIF_RX_FLOW_CONTROL_COSMAP_EN_FIELDS[] =
{
    &XLIF_XRDP1_CHANNEL_XLIF_RX_FLOW_CONTROL_COSMAP_EN_PFC_EN_FIELD,
    &XLIF_XRDP1_CHANNEL_XLIF_RX_FLOW_CONTROL_COSMAP_EN_LLFC_EN_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: XLIF_XRDP1_CHANNEL_XLIF_RX_FLOW_CONTROL_COSMAP_EN *****/
const ru_reg_rec XLIF_XRDP1_CHANNEL_XLIF_RX_FLOW_CONTROL_COSMAP_EN_REG =
{
    "CHANNEL_XLIF_RX_FLOW_CONTROL_COSMAP_EN",
#if RU_INCLUDE_DESC
    "COSMAP_EN Register",
    ".\n",
#endif
    { XLIF_XRDP1_CHANNEL_XLIF_RX_FLOW_CONTROL_COSMAP_EN_REG_OFFSET },
    0,
    0,
    1126,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    XLIF_XRDP1_CHANNEL_XLIF_RX_FLOW_CONTROL_COSMAP_EN_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: XLIF_XRDP1_CHANNEL_XLIF_RX_FLOW_CONTROL_COSMAP, TYPE: Type_CHANNEL_XLIF_RX_FLOW_CONTROL_COSMAP
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: VALUE *****/
const ru_field_rec XLIF_XRDP1_CHANNEL_XLIF_RX_FLOW_CONTROL_COSMAP_VALUE_FIELD =
{
    "VALUE",
#if RU_INCLUDE_DESC
    "",
    "value\n",
#endif
    { XLIF_XRDP1_CHANNEL_XLIF_RX_FLOW_CONTROL_COSMAP_VALUE_FIELD_MASK },
    0,
    { XLIF_XRDP1_CHANNEL_XLIF_RX_FLOW_CONTROL_COSMAP_VALUE_FIELD_WIDTH },
    { XLIF_XRDP1_CHANNEL_XLIF_RX_FLOW_CONTROL_COSMAP_VALUE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XLIF_XRDP1_CHANNEL_XLIF_RX_FLOW_CONTROL_COSMAP_FIELDS[] =
{
    &XLIF_XRDP1_CHANNEL_XLIF_RX_FLOW_CONTROL_COSMAP_VALUE_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: XLIF_XRDP1_CHANNEL_XLIF_RX_FLOW_CONTROL_COSMAP *****/
const ru_reg_rec XLIF_XRDP1_CHANNEL_XLIF_RX_FLOW_CONTROL_COSMAP_REG =
{
    "CHANNEL_XLIF_RX_FLOW_CONTROL_COSMAP",
#if RU_INCLUDE_DESC
    "COSMAP Register",
    ".\n",
#endif
    { XLIF_XRDP1_CHANNEL_XLIF_RX_FLOW_CONTROL_COSMAP_REG_OFFSET },
    0,
    0,
    1127,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    XLIF_XRDP1_CHANNEL_XLIF_RX_FLOW_CONTROL_COSMAP_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: XLIF_XRDP1_CHANNEL_XLIF_TX_IF_IF_ENABLE, TYPE: Type_CHANNEL_XLIF_TX_IF_IF_ENABLE
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: DISABLE_WITH_CREDITS *****/
const ru_field_rec XLIF_XRDP1_CHANNEL_XLIF_TX_IF_IF_ENABLE_DISABLE_WITH_CREDITS_FIELD =
{
    "DISABLE_WITH_CREDITS",
#if RU_INCLUDE_DESC
    "",
    "Disable_With_Credits\n",
#endif
    { XLIF_XRDP1_CHANNEL_XLIF_TX_IF_IF_ENABLE_DISABLE_WITH_CREDITS_FIELD_MASK },
    0,
    { XLIF_XRDP1_CHANNEL_XLIF_TX_IF_IF_ENABLE_DISABLE_WITH_CREDITS_FIELD_WIDTH },
    { XLIF_XRDP1_CHANNEL_XLIF_TX_IF_IF_ENABLE_DISABLE_WITH_CREDITS_FIELD_SHIFT },
    1,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: DISABLE_WO_CREDITS *****/
const ru_field_rec XLIF_XRDP1_CHANNEL_XLIF_TX_IF_IF_ENABLE_DISABLE_WO_CREDITS_FIELD =
{
    "DISABLE_WO_CREDITS",
#if RU_INCLUDE_DESC
    "",
    "Disable_WO_Credits\n",
#endif
    { XLIF_XRDP1_CHANNEL_XLIF_TX_IF_IF_ENABLE_DISABLE_WO_CREDITS_FIELD_MASK },
    0,
    { XLIF_XRDP1_CHANNEL_XLIF_TX_IF_IF_ENABLE_DISABLE_WO_CREDITS_FIELD_WIDTH },
    { XLIF_XRDP1_CHANNEL_XLIF_TX_IF_IF_ENABLE_DISABLE_WO_CREDITS_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XLIF_XRDP1_CHANNEL_XLIF_TX_IF_IF_ENABLE_FIELDS[] =
{
    &XLIF_XRDP1_CHANNEL_XLIF_TX_IF_IF_ENABLE_DISABLE_WITH_CREDITS_FIELD,
    &XLIF_XRDP1_CHANNEL_XLIF_TX_IF_IF_ENABLE_DISABLE_WO_CREDITS_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: XLIF_XRDP1_CHANNEL_XLIF_TX_IF_IF_ENABLE *****/
const ru_reg_rec XLIF_XRDP1_CHANNEL_XLIF_TX_IF_IF_ENABLE_REG =
{
    "CHANNEL_XLIF_TX_IF_IF_ENABLE",
#if RU_INCLUDE_DESC
    "INTERFACE_ENABLE Register",
    "Interface_Enable\n",
#endif
    { XLIF_XRDP1_CHANNEL_XLIF_TX_IF_IF_ENABLE_REG_OFFSET },
    0,
    0,
    1128,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    XLIF_XRDP1_CHANNEL_XLIF_TX_IF_IF_ENABLE_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: XLIF_XRDP1_CHANNEL_XLIF_TX_IF_READ_CREDITS, TYPE: Type_CHANNEL_XLIF_TX_IF_READ_CREDITS
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: VALUE *****/
const ru_field_rec XLIF_XRDP1_CHANNEL_XLIF_TX_IF_READ_CREDITS_VALUE_FIELD =
{
    "VALUE",
#if RU_INCLUDE_DESC
    "",
    "Value\n",
#endif
    { XLIF_XRDP1_CHANNEL_XLIF_TX_IF_READ_CREDITS_VALUE_FIELD_MASK },
    0,
    { XLIF_XRDP1_CHANNEL_XLIF_TX_IF_READ_CREDITS_VALUE_FIELD_WIDTH },
    { XLIF_XRDP1_CHANNEL_XLIF_TX_IF_READ_CREDITS_VALUE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XLIF_XRDP1_CHANNEL_XLIF_TX_IF_READ_CREDITS_FIELDS[] =
{
    &XLIF_XRDP1_CHANNEL_XLIF_TX_IF_READ_CREDITS_VALUE_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: XLIF_XRDP1_CHANNEL_XLIF_TX_IF_READ_CREDITS *****/
const ru_reg_rec XLIF_XRDP1_CHANNEL_XLIF_TX_IF_READ_CREDITS_REG =
{
    "CHANNEL_XLIF_TX_IF_READ_CREDITS",
#if RU_INCLUDE_DESC
    "READ_CREDITS Register",
    "Read_Credits\n",
#endif
    { XLIF_XRDP1_CHANNEL_XLIF_TX_IF_READ_CREDITS_REG_OFFSET },
    0,
    0,
    1129,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    XLIF_XRDP1_CHANNEL_XLIF_TX_IF_READ_CREDITS_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: XLIF_XRDP1_CHANNEL_XLIF_TX_IF_SET_CREDITS, TYPE: Type_CHANNEL_XLIF_TX_IF_SET_CREDITS
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: VALUE *****/
const ru_field_rec XLIF_XRDP1_CHANNEL_XLIF_TX_IF_SET_CREDITS_VALUE_FIELD =
{
    "VALUE",
#if RU_INCLUDE_DESC
    "",
    "Value\n",
#endif
    { XLIF_XRDP1_CHANNEL_XLIF_TX_IF_SET_CREDITS_VALUE_FIELD_MASK },
    0,
    { XLIF_XRDP1_CHANNEL_XLIF_TX_IF_SET_CREDITS_VALUE_FIELD_WIDTH },
    { XLIF_XRDP1_CHANNEL_XLIF_TX_IF_SET_CREDITS_VALUE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: EN *****/
const ru_field_rec XLIF_XRDP1_CHANNEL_XLIF_TX_IF_SET_CREDITS_EN_FIELD =
{
    "EN",
#if RU_INCLUDE_DESC
    "",
    "enable\n",
#endif
    { XLIF_XRDP1_CHANNEL_XLIF_TX_IF_SET_CREDITS_EN_FIELD_MASK },
    0,
    { XLIF_XRDP1_CHANNEL_XLIF_TX_IF_SET_CREDITS_EN_FIELD_WIDTH },
    { XLIF_XRDP1_CHANNEL_XLIF_TX_IF_SET_CREDITS_EN_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XLIF_XRDP1_CHANNEL_XLIF_TX_IF_SET_CREDITS_FIELDS[] =
{
    &XLIF_XRDP1_CHANNEL_XLIF_TX_IF_SET_CREDITS_VALUE_FIELD,
    &XLIF_XRDP1_CHANNEL_XLIF_TX_IF_SET_CREDITS_EN_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: XLIF_XRDP1_CHANNEL_XLIF_TX_IF_SET_CREDITS *****/
const ru_reg_rec XLIF_XRDP1_CHANNEL_XLIF_TX_IF_SET_CREDITS_REG =
{
    "CHANNEL_XLIF_TX_IF_SET_CREDITS",
#if RU_INCLUDE_DESC
    "SET_CREDITS Register",
    "Set_Credits\nSet credit value first.\nThen, set enable bit.\nThen, reset enable bit.\n\n",
#endif
    { XLIF_XRDP1_CHANNEL_XLIF_TX_IF_SET_CREDITS_REG_OFFSET },
    0,
    0,
    1130,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    XLIF_XRDP1_CHANNEL_XLIF_TX_IF_SET_CREDITS_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: XLIF_XRDP1_CHANNEL_XLIF_TX_IF_OUT_CTRL, TYPE: Type_CHANNEL_XLIF_TX_IF_OUT_CTRL
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: MAC_TXERR *****/
const ru_field_rec XLIF_XRDP1_CHANNEL_XLIF_TX_IF_OUT_CTRL_MAC_TXERR_FIELD =
{
    "MAC_TXERR",
#if RU_INCLUDE_DESC
    "",
    "mac_txerr\n",
#endif
    { XLIF_XRDP1_CHANNEL_XLIF_TX_IF_OUT_CTRL_MAC_TXERR_FIELD_MASK },
    0,
    { XLIF_XRDP1_CHANNEL_XLIF_TX_IF_OUT_CTRL_MAC_TXERR_FIELD_WIDTH },
    { XLIF_XRDP1_CHANNEL_XLIF_TX_IF_OUT_CTRL_MAC_TXERR_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: MAC_TXCRCERR *****/
const ru_field_rec XLIF_XRDP1_CHANNEL_XLIF_TX_IF_OUT_CTRL_MAC_TXCRCERR_FIELD =
{
    "MAC_TXCRCERR",
#if RU_INCLUDE_DESC
    "",
    "mac_txcrcerr\n",
#endif
    { XLIF_XRDP1_CHANNEL_XLIF_TX_IF_OUT_CTRL_MAC_TXCRCERR_FIELD_MASK },
    0,
    { XLIF_XRDP1_CHANNEL_XLIF_TX_IF_OUT_CTRL_MAC_TXCRCERR_FIELD_WIDTH },
    { XLIF_XRDP1_CHANNEL_XLIF_TX_IF_OUT_CTRL_MAC_TXCRCERR_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: MAC_TXOSTS_SINEXT *****/
const ru_field_rec XLIF_XRDP1_CHANNEL_XLIF_TX_IF_OUT_CTRL_MAC_TXOSTS_SINEXT_FIELD =
{
    "MAC_TXOSTS_SINEXT",
#if RU_INCLUDE_DESC
    "",
    "mac_txosts_sinext\n",
#endif
    { XLIF_XRDP1_CHANNEL_XLIF_TX_IF_OUT_CTRL_MAC_TXOSTS_SINEXT_FIELD_MASK },
    0,
    { XLIF_XRDP1_CHANNEL_XLIF_TX_IF_OUT_CTRL_MAC_TXOSTS_SINEXT_FIELD_WIDTH },
    { XLIF_XRDP1_CHANNEL_XLIF_TX_IF_OUT_CTRL_MAC_TXOSTS_SINEXT_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: MAC_TXCRCMODE *****/
const ru_field_rec XLIF_XRDP1_CHANNEL_XLIF_TX_IF_OUT_CTRL_MAC_TXCRCMODE_FIELD =
{
    "MAC_TXCRCMODE",
#if RU_INCLUDE_DESC
    "",
    "mac_txcrcmode\n",
#endif
    { XLIF_XRDP1_CHANNEL_XLIF_TX_IF_OUT_CTRL_MAC_TXCRCMODE_FIELD_MASK },
    0,
    { XLIF_XRDP1_CHANNEL_XLIF_TX_IF_OUT_CTRL_MAC_TXCRCMODE_FIELD_WIDTH },
    { XLIF_XRDP1_CHANNEL_XLIF_TX_IF_OUT_CTRL_MAC_TXCRCMODE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XLIF_XRDP1_CHANNEL_XLIF_TX_IF_OUT_CTRL_FIELDS[] =
{
    &XLIF_XRDP1_CHANNEL_XLIF_TX_IF_OUT_CTRL_MAC_TXERR_FIELD,
    &XLIF_XRDP1_CHANNEL_XLIF_TX_IF_OUT_CTRL_MAC_TXCRCERR_FIELD,
    &XLIF_XRDP1_CHANNEL_XLIF_TX_IF_OUT_CTRL_MAC_TXOSTS_SINEXT_FIELD,
    &XLIF_XRDP1_CHANNEL_XLIF_TX_IF_OUT_CTRL_MAC_TXCRCMODE_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: XLIF_XRDP1_CHANNEL_XLIF_TX_IF_OUT_CTRL *****/
const ru_reg_rec XLIF_XRDP1_CHANNEL_XLIF_TX_IF_OUT_CTRL_REG =
{
    "CHANNEL_XLIF_TX_IF_OUT_CTRL",
#if RU_INCLUDE_DESC
    "OUTPUTS_CONTROL Register",
    "Control the values of several output signals on the XRDP -> XLMAC interface.\n",
#endif
    { XLIF_XRDP1_CHANNEL_XLIF_TX_IF_OUT_CTRL_REG_OFFSET },
    0,
    0,
    1131,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    4,
    XLIF_XRDP1_CHANNEL_XLIF_TX_IF_OUT_CTRL_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: XLIF_XRDP1_CHANNEL_XLIF_TX_IF_URUN_PORT_ENABLE, TYPE: Type_CHANNEL_XLIF_TX_IF_URUN_PORT_ENABLE
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: ENABLE *****/
const ru_field_rec XLIF_XRDP1_CHANNEL_XLIF_TX_IF_URUN_PORT_ENABLE_ENABLE_FIELD =
{
    "ENABLE",
#if RU_INCLUDE_DESC
    "",
    "Enable\n",
#endif
    { XLIF_XRDP1_CHANNEL_XLIF_TX_IF_URUN_PORT_ENABLE_ENABLE_FIELD_MASK },
    0,
    { XLIF_XRDP1_CHANNEL_XLIF_TX_IF_URUN_PORT_ENABLE_ENABLE_FIELD_WIDTH },
    { XLIF_XRDP1_CHANNEL_XLIF_TX_IF_URUN_PORT_ENABLE_ENABLE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XLIF_XRDP1_CHANNEL_XLIF_TX_IF_URUN_PORT_ENABLE_FIELDS[] =
{
    &XLIF_XRDP1_CHANNEL_XLIF_TX_IF_URUN_PORT_ENABLE_ENABLE_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: XLIF_XRDP1_CHANNEL_XLIF_TX_IF_URUN_PORT_ENABLE *****/
const ru_reg_rec XLIF_XRDP1_CHANNEL_XLIF_TX_IF_URUN_PORT_ENABLE_REG =
{
    "CHANNEL_XLIF_TX_IF_URUN_PORT_ENABLE",
#if RU_INCLUDE_DESC
    "UNDERRUN_PROTECTION_ENABLE Register",
    "Underrun_Protection_Enable\n",
#endif
    { XLIF_XRDP1_CHANNEL_XLIF_TX_IF_URUN_PORT_ENABLE_REG_OFFSET },
    0,
    0,
    1132,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    XLIF_XRDP1_CHANNEL_XLIF_TX_IF_URUN_PORT_ENABLE_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: XLIF_XRDP1_CHANNEL_XLIF_TX_IF_TX_THRESHOLD, TYPE: Type_CHANNEL_XLIF_TX_IF_TX_THRESHOLD
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: VALUE *****/
const ru_field_rec XLIF_XRDP1_CHANNEL_XLIF_TX_IF_TX_THRESHOLD_VALUE_FIELD =
{
    "VALUE",
#if RU_INCLUDE_DESC
    "",
    "Value\n",
#endif
    { XLIF_XRDP1_CHANNEL_XLIF_TX_IF_TX_THRESHOLD_VALUE_FIELD_MASK },
    0,
    { XLIF_XRDP1_CHANNEL_XLIF_TX_IF_TX_THRESHOLD_VALUE_FIELD_WIDTH },
    { XLIF_XRDP1_CHANNEL_XLIF_TX_IF_TX_THRESHOLD_VALUE_FIELD_SHIFT },
    1,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XLIF_XRDP1_CHANNEL_XLIF_TX_IF_TX_THRESHOLD_FIELDS[] =
{
    &XLIF_XRDP1_CHANNEL_XLIF_TX_IF_TX_THRESHOLD_VALUE_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: XLIF_XRDP1_CHANNEL_XLIF_TX_IF_TX_THRESHOLD *****/
const ru_reg_rec XLIF_XRDP1_CHANNEL_XLIF_TX_IF_TX_THRESHOLD_REG =
{
    "CHANNEL_XLIF_TX_IF_TX_THRESHOLD",
#if RU_INCLUDE_DESC
    "TX_THRESHOLD Register",
    "TX threshold for the TX CDC FIFO in units of 128 bit.\nThe TX CDC FIFO is depth is 16 entries.\n\n",
#endif
    { XLIF_XRDP1_CHANNEL_XLIF_TX_IF_TX_THRESHOLD_REG_OFFSET },
    0,
    0,
    1133,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    XLIF_XRDP1_CHANNEL_XLIF_TX_IF_TX_THRESHOLD_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: XLIF_XRDP1_CHANNEL_XLIF_TX_IF_TDM_MODE, TYPE: Type_CHANNEL_XLIF_TX_IF_TDM_MODE
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: VALUE *****/
const ru_field_rec XLIF_XRDP1_CHANNEL_XLIF_TX_IF_TDM_MODE_VALUE_FIELD =
{
    "VALUE",
#if RU_INCLUDE_DESC
    "",
    "Value\n",
#endif
    { XLIF_XRDP1_CHANNEL_XLIF_TX_IF_TDM_MODE_VALUE_FIELD_MASK },
    0,
    { XLIF_XRDP1_CHANNEL_XLIF_TX_IF_TDM_MODE_VALUE_FIELD_WIDTH },
    { XLIF_XRDP1_CHANNEL_XLIF_TX_IF_TDM_MODE_VALUE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XLIF_XRDP1_CHANNEL_XLIF_TX_IF_TDM_MODE_FIELDS[] =
{
    &XLIF_XRDP1_CHANNEL_XLIF_TX_IF_TDM_MODE_VALUE_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: XLIF_XRDP1_CHANNEL_XLIF_TX_IF_TDM_MODE *****/
const ru_reg_rec XLIF_XRDP1_CHANNEL_XLIF_TX_IF_TDM_MODE_REG =
{
    "CHANNEL_XLIF_TX_IF_TDM_MODE",
#if RU_INCLUDE_DESC
    "TDM_MODE Register",
    "TDM mode.\n0 - flexible TDM\n1 - Fixed TDM\n\n",
#endif
    { XLIF_XRDP1_CHANNEL_XLIF_TX_IF_TDM_MODE_REG_OFFSET },
    0,
    0,
    1134,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    XLIF_XRDP1_CHANNEL_XLIF_TX_IF_TDM_MODE_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: XLIF_XRDP1_CHANNEL_XLIF_TX_IF_LOOPBACK, TYPE: Type_CHANNEL_XLIF_TX_IF_LOOPBACK
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: XLMAC_RX2TX *****/
const ru_field_rec XLIF_XRDP1_CHANNEL_XLIF_TX_IF_LOOPBACK_XLMAC_RX2TX_FIELD =
{
    "XLMAC_RX2TX",
#if RU_INCLUDE_DESC
    "",
    "Traffic coming from xlmac will be looped back towards the xlmac\n",
#endif
    { XLIF_XRDP1_CHANNEL_XLIF_TX_IF_LOOPBACK_XLMAC_RX2TX_FIELD_MASK },
    0,
    { XLIF_XRDP1_CHANNEL_XLIF_TX_IF_LOOPBACK_XLMAC_RX2TX_FIELD_WIDTH },
    { XLIF_XRDP1_CHANNEL_XLIF_TX_IF_LOOPBACK_XLMAC_RX2TX_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: BBH_RX2TX *****/
const ru_field_rec XLIF_XRDP1_CHANNEL_XLIF_TX_IF_LOOPBACK_BBH_RX2TX_FIELD =
{
    "BBH_RX2TX",
#if RU_INCLUDE_DESC
    "",
    "Traffic coming from the bbh will be looped back towards the bbh\n",
#endif
    { XLIF_XRDP1_CHANNEL_XLIF_TX_IF_LOOPBACK_BBH_RX2TX_FIELD_MASK },
    0,
    { XLIF_XRDP1_CHANNEL_XLIF_TX_IF_LOOPBACK_BBH_RX2TX_FIELD_WIDTH },
    { XLIF_XRDP1_CHANNEL_XLIF_TX_IF_LOOPBACK_BBH_RX2TX_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XLIF_XRDP1_CHANNEL_XLIF_TX_IF_LOOPBACK_FIELDS[] =
{
    &XLIF_XRDP1_CHANNEL_XLIF_TX_IF_LOOPBACK_XLMAC_RX2TX_FIELD,
    &XLIF_XRDP1_CHANNEL_XLIF_TX_IF_LOOPBACK_BBH_RX2TX_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: XLIF_XRDP1_CHANNEL_XLIF_TX_IF_LOOPBACK *****/
const ru_reg_rec XLIF_XRDP1_CHANNEL_XLIF_TX_IF_LOOPBACK_REG =
{
    "CHANNEL_XLIF_TX_IF_LOOPBACK",
#if RU_INCLUDE_DESC
    "LOOPBACK Register",
    "loopback at xlmac side\n\n",
#endif
    { XLIF_XRDP1_CHANNEL_XLIF_TX_IF_LOOPBACK_REG_OFFSET },
    0,
    0,
    1135,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    XLIF_XRDP1_CHANNEL_XLIF_TX_IF_LOOPBACK_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: XLIF_XRDP1_CHANNEL_XLIF_TX_FLOW_CONTROL_COSMAP_EN_STAT, TYPE: Type_CHANNEL_XLIF_TX_FLOW_CONTROL_COSMAP_EN_STAT
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: PFC_EN *****/
const ru_field_rec XLIF_XRDP1_CHANNEL_XLIF_TX_FLOW_CONTROL_COSMAP_EN_STAT_PFC_EN_FIELD =
{
    "PFC_EN",
#if RU_INCLUDE_DESC
    "",
    "PFC_EN\n",
#endif
    { XLIF_XRDP1_CHANNEL_XLIF_TX_FLOW_CONTROL_COSMAP_EN_STAT_PFC_EN_FIELD_MASK },
    0,
    { XLIF_XRDP1_CHANNEL_XLIF_TX_FLOW_CONTROL_COSMAP_EN_STAT_PFC_EN_FIELD_WIDTH },
    { XLIF_XRDP1_CHANNEL_XLIF_TX_FLOW_CONTROL_COSMAP_EN_STAT_PFC_EN_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: LLFC_EN *****/
const ru_field_rec XLIF_XRDP1_CHANNEL_XLIF_TX_FLOW_CONTROL_COSMAP_EN_STAT_LLFC_EN_FIELD =
{
    "LLFC_EN",
#if RU_INCLUDE_DESC
    "",
    "LLFC_en\n",
#endif
    { XLIF_XRDP1_CHANNEL_XLIF_TX_FLOW_CONTROL_COSMAP_EN_STAT_LLFC_EN_FIELD_MASK },
    0,
    { XLIF_XRDP1_CHANNEL_XLIF_TX_FLOW_CONTROL_COSMAP_EN_STAT_LLFC_EN_FIELD_WIDTH },
    { XLIF_XRDP1_CHANNEL_XLIF_TX_FLOW_CONTROL_COSMAP_EN_STAT_LLFC_EN_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XLIF_XRDP1_CHANNEL_XLIF_TX_FLOW_CONTROL_COSMAP_EN_STAT_FIELDS[] =
{
    &XLIF_XRDP1_CHANNEL_XLIF_TX_FLOW_CONTROL_COSMAP_EN_STAT_PFC_EN_FIELD,
    &XLIF_XRDP1_CHANNEL_XLIF_TX_FLOW_CONTROL_COSMAP_EN_STAT_LLFC_EN_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: XLIF_XRDP1_CHANNEL_XLIF_TX_FLOW_CONTROL_COSMAP_EN_STAT *****/
const ru_reg_rec XLIF_XRDP1_CHANNEL_XLIF_TX_FLOW_CONTROL_COSMAP_EN_STAT_REG =
{
    "CHANNEL_XLIF_TX_FLOW_CONTROL_COSMAP_EN_STAT",
#if RU_INCLUDE_DESC
    "COSMAP_EN_STATUS Register",
    "cosmap_en indications from the XLMAC Interface\n",
#endif
    { XLIF_XRDP1_CHANNEL_XLIF_TX_FLOW_CONTROL_COSMAP_EN_STAT_REG_OFFSET },
    0,
    0,
    1136,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    XLIF_XRDP1_CHANNEL_XLIF_TX_FLOW_CONTROL_COSMAP_EN_STAT_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: XLIF_XRDP1_CHANNEL_XLIF_TX_FLOW_CONTROL_COSMAP_STAT, TYPE: Type_CHANNEL_XLIF_TX_FLOW_CONTROL_COSMAP_STAT
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: VALUE *****/
const ru_field_rec XLIF_XRDP1_CHANNEL_XLIF_TX_FLOW_CONTROL_COSMAP_STAT_VALUE_FIELD =
{
    "VALUE",
#if RU_INCLUDE_DESC
    "",
    "value\n",
#endif
    { XLIF_XRDP1_CHANNEL_XLIF_TX_FLOW_CONTROL_COSMAP_STAT_VALUE_FIELD_MASK },
    0,
    { XLIF_XRDP1_CHANNEL_XLIF_TX_FLOW_CONTROL_COSMAP_STAT_VALUE_FIELD_WIDTH },
    { XLIF_XRDP1_CHANNEL_XLIF_TX_FLOW_CONTROL_COSMAP_STAT_VALUE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XLIF_XRDP1_CHANNEL_XLIF_TX_FLOW_CONTROL_COSMAP_STAT_FIELDS[] =
{
    &XLIF_XRDP1_CHANNEL_XLIF_TX_FLOW_CONTROL_COSMAP_STAT_VALUE_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: XLIF_XRDP1_CHANNEL_XLIF_TX_FLOW_CONTROL_COSMAP_STAT *****/
const ru_reg_rec XLIF_XRDP1_CHANNEL_XLIF_TX_FLOW_CONTROL_COSMAP_STAT_REG =
{
    "CHANNEL_XLIF_TX_FLOW_CONTROL_COSMAP_STAT",
#if RU_INCLUDE_DESC
    "COSMAP_STATUS Register",
    "cosmap_status from the XLMAC Interface\n",
#endif
    { XLIF_XRDP1_CHANNEL_XLIF_TX_FLOW_CONTROL_COSMAP_STAT_REG_OFFSET },
    0,
    0,
    1137,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    XLIF_XRDP1_CHANNEL_XLIF_TX_FLOW_CONTROL_COSMAP_STAT_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: XLIF_XRDP1_CHANNEL_DEBUG_BUS_SEL, TYPE: Type_CHANNEL_DEBUG_BUS_SEL
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: SELECT_MODULE *****/
const ru_field_rec XLIF_XRDP1_CHANNEL_DEBUG_BUS_SEL_SELECT_MODULE_FIELD =
{
    "SELECT_MODULE",
#if RU_INCLUDE_DESC
    "",
    "RX_TX selection\n00 - RX\n10 - TX\n",
#endif
    { XLIF_XRDP1_CHANNEL_DEBUG_BUS_SEL_SELECT_MODULE_FIELD_MASK },
    0,
    { XLIF_XRDP1_CHANNEL_DEBUG_BUS_SEL_SELECT_MODULE_FIELD_WIDTH },
    { XLIF_XRDP1_CHANNEL_DEBUG_BUS_SEL_SELECT_MODULE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: SELECT_LANE *****/
const ru_field_rec XLIF_XRDP1_CHANNEL_DEBUG_BUS_SEL_SELECT_LANE_FIELD =
{
    "SELECT_LANE",
#if RU_INCLUDE_DESC
    "",
    "Select_lane\n",
#endif
    { XLIF_XRDP1_CHANNEL_DEBUG_BUS_SEL_SELECT_LANE_FIELD_MASK },
    0,
    { XLIF_XRDP1_CHANNEL_DEBUG_BUS_SEL_SELECT_LANE_FIELD_WIDTH },
    { XLIF_XRDP1_CHANNEL_DEBUG_BUS_SEL_SELECT_LANE_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XLIF_XRDP1_CHANNEL_DEBUG_BUS_SEL_FIELDS[] =
{
    &XLIF_XRDP1_CHANNEL_DEBUG_BUS_SEL_SELECT_MODULE_FIELD,
    &XLIF_XRDP1_CHANNEL_DEBUG_BUS_SEL_SELECT_LANE_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: XLIF_XRDP1_CHANNEL_DEBUG_BUS_SEL *****/
const ru_reg_rec XLIF_XRDP1_CHANNEL_DEBUG_BUS_SEL_REG =
{
    "CHANNEL_DEBUG_BUS_SEL",
#if RU_INCLUDE_DESC
    "SEL Register",
    "Select\n",
#endif
    { XLIF_XRDP1_CHANNEL_DEBUG_BUS_SEL_REG_OFFSET },
    0,
    0,
    1138,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    XLIF_XRDP1_CHANNEL_DEBUG_BUS_SEL_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: XLIF_XRDP1_CHANNEL_XLIF_EEE_IND, TYPE: Type_CHANNEL_XLIF_EEE_IND
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: LPI_RX_DETECT *****/
const ru_field_rec XLIF_XRDP1_CHANNEL_XLIF_EEE_IND_LPI_RX_DETECT_FIELD =
{
    "LPI_RX_DETECT",
#if RU_INCLUDE_DESC
    "",
    "lpi_rx_detect\n",
#endif
    { XLIF_XRDP1_CHANNEL_XLIF_EEE_IND_LPI_RX_DETECT_FIELD_MASK },
    0,
    { XLIF_XRDP1_CHANNEL_XLIF_EEE_IND_LPI_RX_DETECT_FIELD_WIDTH },
    { XLIF_XRDP1_CHANNEL_XLIF_EEE_IND_LPI_RX_DETECT_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: LPI_TX_DETECT *****/
const ru_field_rec XLIF_XRDP1_CHANNEL_XLIF_EEE_IND_LPI_TX_DETECT_FIELD =
{
    "LPI_TX_DETECT",
#if RU_INCLUDE_DESC
    "",
    "lpi_tx_detect\n",
#endif
    { XLIF_XRDP1_CHANNEL_XLIF_EEE_IND_LPI_TX_DETECT_FIELD_MASK },
    0,
    { XLIF_XRDP1_CHANNEL_XLIF_EEE_IND_LPI_TX_DETECT_FIELD_WIDTH },
    { XLIF_XRDP1_CHANNEL_XLIF_EEE_IND_LPI_TX_DETECT_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XLIF_XRDP1_CHANNEL_XLIF_EEE_IND_FIELDS[] =
{
    &XLIF_XRDP1_CHANNEL_XLIF_EEE_IND_LPI_RX_DETECT_FIELD,
    &XLIF_XRDP1_CHANNEL_XLIF_EEE_IND_LPI_TX_DETECT_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: XLIF_XRDP1_CHANNEL_XLIF_EEE_IND *****/
const ru_reg_rec XLIF_XRDP1_CHANNEL_XLIF_EEE_IND_REG =
{
    "CHANNEL_XLIF_EEE_IND",
#if RU_INCLUDE_DESC
    "INDICATIONS Register",
    "eee indications from the XLMAC interface\n",
#endif
    { XLIF_XRDP1_CHANNEL_XLIF_EEE_IND_REG_OFFSET },
    0,
    0,
    1139,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    XLIF_XRDP1_CHANNEL_XLIF_EEE_IND_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: XLIF_XRDP1_CHANNEL_Q_OFF_IND, TYPE: Type_CHANNEL_Q_OFF_IND
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: Q_OFF *****/
const ru_field_rec XLIF_XRDP1_CHANNEL_Q_OFF_IND_Q_OFF_FIELD =
{
    "Q_OFF",
#if RU_INCLUDE_DESC
    "",
    "Q_OFF\n",
#endif
    { XLIF_XRDP1_CHANNEL_Q_OFF_IND_Q_OFF_FIELD_MASK },
    0,
    { XLIF_XRDP1_CHANNEL_Q_OFF_IND_Q_OFF_FIELD_WIDTH },
    { XLIF_XRDP1_CHANNEL_Q_OFF_IND_Q_OFF_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: FAILOVER_ON *****/
const ru_field_rec XLIF_XRDP1_CHANNEL_Q_OFF_IND_FAILOVER_ON_FIELD =
{
    "FAILOVER_ON",
#if RU_INCLUDE_DESC
    "",
    "Failover_on\n",
#endif
    { XLIF_XRDP1_CHANNEL_Q_OFF_IND_FAILOVER_ON_FIELD_MASK },
    0,
    { XLIF_XRDP1_CHANNEL_Q_OFF_IND_FAILOVER_ON_FIELD_WIDTH },
    { XLIF_XRDP1_CHANNEL_Q_OFF_IND_FAILOVER_ON_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XLIF_XRDP1_CHANNEL_Q_OFF_IND_FIELDS[] =
{
    &XLIF_XRDP1_CHANNEL_Q_OFF_IND_Q_OFF_FIELD,
    &XLIF_XRDP1_CHANNEL_Q_OFF_IND_FAILOVER_ON_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: XLIF_XRDP1_CHANNEL_Q_OFF_IND *****/
const ru_reg_rec XLIF_XRDP1_CHANNEL_Q_OFF_IND_REG =
{
    "CHANNEL_Q_OFF_IND",
#if RU_INCLUDE_DESC
    "INDICATIONS Register",
    "indications from the XLMAC IF\n",
#endif
    { XLIF_XRDP1_CHANNEL_Q_OFF_IND_REG_OFFSET },
    0,
    0,
    1140,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    XLIF_XRDP1_CHANNEL_Q_OFF_IND_FIELDS,
#endif
};

/******************************************************************************
 * Register: NAME: XLIF_XRDP1_CHANNEL_LANE_TO_PORT_MAP_PORT_MAP, TYPE: Type_CHANNEL_LANE_TO_PORT_MAP_PORT_MAP
 ******************************************************************************/

#if RU_INCLUDE_FIELD_DB
/***** Field: LANE0 *****/
const ru_field_rec XLIF_XRDP1_CHANNEL_LANE_TO_PORT_MAP_PORT_MAP_LANE0_FIELD =
{
    "LANE0",
#if RU_INCLUDE_DESC
    "",
    "XLMAC port to be mapped to BBH0\n",
#endif
    { XLIF_XRDP1_CHANNEL_LANE_TO_PORT_MAP_PORT_MAP_LANE0_FIELD_MASK },
    0,
    { XLIF_XRDP1_CHANNEL_LANE_TO_PORT_MAP_PORT_MAP_LANE0_FIELD_WIDTH },
    { XLIF_XRDP1_CHANNEL_LANE_TO_PORT_MAP_PORT_MAP_LANE0_FIELD_SHIFT },
    0,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: LANE1 *****/
const ru_field_rec XLIF_XRDP1_CHANNEL_LANE_TO_PORT_MAP_PORT_MAP_LANE1_FIELD =
{
    "LANE1",
#if RU_INCLUDE_DESC
    "",
    "XLMAC port which is mapped to BBH1\n",
#endif
    { XLIF_XRDP1_CHANNEL_LANE_TO_PORT_MAP_PORT_MAP_LANE1_FIELD_MASK },
    0,
    { XLIF_XRDP1_CHANNEL_LANE_TO_PORT_MAP_PORT_MAP_LANE1_FIELD_WIDTH },
    { XLIF_XRDP1_CHANNEL_LANE_TO_PORT_MAP_PORT_MAP_LANE1_FIELD_SHIFT },
    1,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: LANE2 *****/
const ru_field_rec XLIF_XRDP1_CHANNEL_LANE_TO_PORT_MAP_PORT_MAP_LANE2_FIELD =
{
    "LANE2",
#if RU_INCLUDE_DESC
    "",
    "XLMAC port which is mapped to BBH 2\n",
#endif
    { XLIF_XRDP1_CHANNEL_LANE_TO_PORT_MAP_PORT_MAP_LANE2_FIELD_MASK },
    0,
    { XLIF_XRDP1_CHANNEL_LANE_TO_PORT_MAP_PORT_MAP_LANE2_FIELD_WIDTH },
    { XLIF_XRDP1_CHANNEL_LANE_TO_PORT_MAP_PORT_MAP_LANE2_FIELD_SHIFT },
    2,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
/***** Field: LANE3 *****/
const ru_field_rec XLIF_XRDP1_CHANNEL_LANE_TO_PORT_MAP_PORT_MAP_LANE3_FIELD =
{
    "LANE3",
#if RU_INCLUDE_DESC
    "",
    "XLMAC port which is mapped to BBH 3\n",
#endif
    { XLIF_XRDP1_CHANNEL_LANE_TO_PORT_MAP_PORT_MAP_LANE3_FIELD_MASK },
    0,
    { XLIF_XRDP1_CHANNEL_LANE_TO_PORT_MAP_PORT_MAP_LANE3_FIELD_WIDTH },
    { XLIF_XRDP1_CHANNEL_LANE_TO_PORT_MAP_PORT_MAP_LANE3_FIELD_SHIFT },
    3,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
};
#endif

#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XLIF_XRDP1_CHANNEL_LANE_TO_PORT_MAP_PORT_MAP_FIELDS[] =
{
    &XLIF_XRDP1_CHANNEL_LANE_TO_PORT_MAP_PORT_MAP_LANE0_FIELD,
    &XLIF_XRDP1_CHANNEL_LANE_TO_PORT_MAP_PORT_MAP_LANE1_FIELD,
    &XLIF_XRDP1_CHANNEL_LANE_TO_PORT_MAP_PORT_MAP_LANE2_FIELD,
    &XLIF_XRDP1_CHANNEL_LANE_TO_PORT_MAP_PORT_MAP_LANE3_FIELD,
};
#endif /* RU_INCLUDE_FIELD_DB */

/***** Register struct: XLIF_XRDP1_CHANNEL_LANE_TO_PORT_MAP_PORT_MAP *****/
const ru_reg_rec XLIF_XRDP1_CHANNEL_LANE_TO_PORT_MAP_PORT_MAP_REG =
{
    "CHANNEL_LANE_TO_PORT_MAP_PORT_MAP",
#if RU_INCLUDE_DESC
    "PORT_MAP Register",
    "PORT_MAP\n",
#endif
    { XLIF_XRDP1_CHANNEL_LANE_TO_PORT_MAP_PORT_MAP_REG_OFFSET },
    0,
    0,
    1141,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    4,
    XLIF_XRDP1_CHANNEL_LANE_TO_PORT_MAP_PORT_MAP_FIELDS,
#endif
};

unsigned long XLIF_XRDP1_ADDRS[] =
{
    0x828B2800,
    0x828B2A00,
    0x828B2C00,
    0x828B2E00,
};

static const ru_reg_rec *XLIF_XRDP1_REGS[] =
{
    &XLIF_XRDP1_CHANNEL_XLIF_RX_IF_IF_DIS_REG,
    &XLIF_XRDP1_CHANNEL_XLIF_RX_IF_OFLW_FLAG_REG,
    &XLIF_XRDP1_CHANNEL_XLIF_RX_IF_ERR_FLAG_REG,
    &XLIF_XRDP1_CHANNEL_XLIF_RX_FLOW_CONTROL_COSMAP_EN_REG,
    &XLIF_XRDP1_CHANNEL_XLIF_RX_FLOW_CONTROL_COSMAP_REG,
    &XLIF_XRDP1_CHANNEL_XLIF_TX_IF_IF_ENABLE_REG,
    &XLIF_XRDP1_CHANNEL_XLIF_TX_IF_READ_CREDITS_REG,
    &XLIF_XRDP1_CHANNEL_XLIF_TX_IF_SET_CREDITS_REG,
    &XLIF_XRDP1_CHANNEL_XLIF_TX_IF_OUT_CTRL_REG,
    &XLIF_XRDP1_CHANNEL_XLIF_TX_IF_URUN_PORT_ENABLE_REG,
    &XLIF_XRDP1_CHANNEL_XLIF_TX_IF_TX_THRESHOLD_REG,
    &XLIF_XRDP1_CHANNEL_XLIF_TX_IF_TDM_MODE_REG,
    &XLIF_XRDP1_CHANNEL_XLIF_TX_IF_LOOPBACK_REG,
    &XLIF_XRDP1_CHANNEL_XLIF_TX_FLOW_CONTROL_COSMAP_EN_STAT_REG,
    &XLIF_XRDP1_CHANNEL_XLIF_TX_FLOW_CONTROL_COSMAP_STAT_REG,
    &XLIF_XRDP1_CHANNEL_DEBUG_BUS_SEL_REG,
    &XLIF_XRDP1_CHANNEL_XLIF_EEE_IND_REG,
    &XLIF_XRDP1_CHANNEL_Q_OFF_IND_REG,
    &XLIF_XRDP1_CHANNEL_LANE_TO_PORT_MAP_PORT_MAP_REG,
};

const ru_block_rec XLIF_XRDP1_BLOCK =
{
    "XLIF_XRDP1",
    XLIF_XRDP1_ADDRS,
    4,
    19,
    XLIF_XRDP1_REGS,
};
