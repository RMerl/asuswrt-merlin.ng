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

#include "ru.h"

#if RU_INCLUDE_FIELD_DB
/******************************************************************************
 * Field: XPORT_PORTRESET_P0_CTRL_RESERVED0
 ******************************************************************************/
const ru_field_rec XPORT_PORTRESET_P0_CTRL_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    XPORT_PORTRESET_P0_CTRL_RESERVED0_FIELD_MASK,
    0,
    XPORT_PORTRESET_P0_CTRL_RESERVED0_FIELD_WIDTH,
    XPORT_PORTRESET_P0_CTRL_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_PORTRESET_P0_CTRL_PORT_SW_RESET
 ******************************************************************************/
const ru_field_rec XPORT_PORTRESET_P0_CTRL_PORT_SW_RESET_FIELD =
{
    "PORT_SW_RESET",
#if RU_INCLUDE_DESC
    "",
    "XPORT software reset for the port.\n"
    "This control is relevant only when Port Reset State Machine is enabled for the port (see ENABLE_SM_RUN in Port Reset CONFIG register).\n"
    "Software can set this bit to initiate the port reset procedure.\n"
    "Alternatively, this bit can be set automatically by Port Reset hardware when the link is down and LINK_DOWN_RST_EN[port]==1.\n"
    "In either case, software must clear this bit after the link is up (typically software is notified by interrupt),"
    " after all settings for the new link (XPORT/XLMAC port interface mode, speed, etc.) are programmed.\n"
    "See other Port Reset registers for more details.",
#endif
    XPORT_PORTRESET_P0_CTRL_PORT_SW_RESET_FIELD_MASK,
    0,
    XPORT_PORTRESET_P0_CTRL_PORT_SW_RESET_FIELD_WIDTH,
    XPORT_PORTRESET_P0_CTRL_PORT_SW_RESET_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_PORTRESET_P1_CTRL_RESERVED0
 ******************************************************************************/
const ru_field_rec XPORT_PORTRESET_P1_CTRL_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    XPORT_PORTRESET_P1_CTRL_RESERVED0_FIELD_MASK,
    0,
    XPORT_PORTRESET_P1_CTRL_RESERVED0_FIELD_WIDTH,
    XPORT_PORTRESET_P1_CTRL_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_PORTRESET_P1_CTRL_PORT_SW_RESET
 ******************************************************************************/
const ru_field_rec XPORT_PORTRESET_P1_CTRL_PORT_SW_RESET_FIELD =
{
    "PORT_SW_RESET",
#if RU_INCLUDE_DESC
    "",
    "XPORT software reset for the port.\n"
    "This control is relevant only when Port Reset State Machine is enabled for the port (see ENABLE_SM_RUN in Port Reset CONFIG register).\n"
    "Software can set this bit to initiate the port reset procedure.\n"
    "Alternatively, this bit can be set automatically by Port Reset hardware when the link is down and LINK_DOWN_RST_EN[port]==1.\n"
    "In either case, software must clear this bit after the link is up (typically software is notified by interrupt),"
    " after all settings for the new link (XPORT/XLMAC port interface mode, speed, etc.) are programmed.\n"
    "See other Port Reset registers for more details.",
#endif
    XPORT_PORTRESET_P1_CTRL_PORT_SW_RESET_FIELD_MASK,
    0,
    XPORT_PORTRESET_P1_CTRL_PORT_SW_RESET_FIELD_WIDTH,
    XPORT_PORTRESET_P1_CTRL_PORT_SW_RESET_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_PORTRESET_P2_CTRL_RESERVED0
 ******************************************************************************/
const ru_field_rec XPORT_PORTRESET_P2_CTRL_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    XPORT_PORTRESET_P2_CTRL_RESERVED0_FIELD_MASK,
    0,
    XPORT_PORTRESET_P2_CTRL_RESERVED0_FIELD_WIDTH,
    XPORT_PORTRESET_P2_CTRL_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_PORTRESET_P2_CTRL_PORT_SW_RESET
 ******************************************************************************/
const ru_field_rec XPORT_PORTRESET_P2_CTRL_PORT_SW_RESET_FIELD =
{
    "PORT_SW_RESET",
#if RU_INCLUDE_DESC
    "",
    "XPORT software reset for the port.\n"
    "This control is relevant only when Port Reset State Machine is enabled for the port (see ENABLE_SM_RUN in Port Reset CONFIG register).\n"
    "Software can set this bit to initiate the port reset procedure.\n"
    "Alternatively, this bit can be set automatically by Port Reset hardware when the link is down and LINK_DOWN_RST_EN[port]==1.\n"
    "In either case, software must clear this bit after the link is up (typically software is notified by interrupt),"
    " after all settings for the new link (XPORT/XLMAC port interface mode, speed, etc.) are programmed.\n"
    "See other Port Reset registers for more details.",
#endif
    XPORT_PORTRESET_P2_CTRL_PORT_SW_RESET_FIELD_MASK,
    0,
    XPORT_PORTRESET_P2_CTRL_PORT_SW_RESET_FIELD_WIDTH,
    XPORT_PORTRESET_P2_CTRL_PORT_SW_RESET_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_PORTRESET_P3_CTRL_RESERVED0
 ******************************************************************************/
const ru_field_rec XPORT_PORTRESET_P3_CTRL_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    XPORT_PORTRESET_P3_CTRL_RESERVED0_FIELD_MASK,
    0,
    XPORT_PORTRESET_P3_CTRL_RESERVED0_FIELD_WIDTH,
    XPORT_PORTRESET_P3_CTRL_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_PORTRESET_P3_CTRL_PORT_SW_RESET
 ******************************************************************************/
const ru_field_rec XPORT_PORTRESET_P3_CTRL_PORT_SW_RESET_FIELD =
{
    "PORT_SW_RESET",
#if RU_INCLUDE_DESC
    "",
    "XPORT software reset for the port.\n"
    "This control is relevant only when Port Reset State Machine is enabled for the port (see ENABLE_SM_RUN in Port Reset CONFIG register).\n"
    "Software can set this bit to initiate the port reset procedure.\n"
    "Alternatively, this bit can be set automatically by Port Reset hardware when the link is down and LINK_DOWN_RST_EN[port]==1.\n"
    "In either case, software must clear this bit after the link is up (typically software is notified by interrupt),"
    " after all settings for the new link (XPORT/XLMAC port interface mode, speed, etc.) are programmed.\n"
    "See other Port Reset registers for more details.",
#endif
    XPORT_PORTRESET_P3_CTRL_PORT_SW_RESET_FIELD_MASK,
    0,
    XPORT_PORTRESET_P3_CTRL_PORT_SW_RESET_FIELD_WIDTH,
    XPORT_PORTRESET_P3_CTRL_PORT_SW_RESET_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_PORTRESET_CONFIG_RESERVED0
 ******************************************************************************/
const ru_field_rec XPORT_PORTRESET_CONFIG_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    XPORT_PORTRESET_CONFIG_RESERVED0_FIELD_MASK,
    0,
    XPORT_PORTRESET_CONFIG_RESERVED0_FIELD_WIDTH,
    XPORT_PORTRESET_CONFIG_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_PORTRESET_CONFIG_LINK_DOWN_RST_EN
 ******************************************************************************/
const ru_field_rec XPORT_PORTRESET_CONFIG_LINK_DOWN_RST_EN_FIELD =
{
    "LINK_DOWN_RST_EN",
#if RU_INCLUDE_DESC
    "",
    "One bit per port.\n"
    "When bit 'i' is set, and ENABLE_SM_RUN[i]==1, Port Reset State Machine will automatically run when the link is down."
    " SW however still needs to program all required settings for the new link"
    " (XPORT/XLMAC port interface mode, speed, etc.) once the link is up, and"
    " let hardware know once it is done, by clearing PORT_SW_RESET< port >.\n"
    "See PORT_SW_RESET in Px_CTRL register (one per port), and other Port Reset registers/fields for more details.",
#endif
    XPORT_PORTRESET_CONFIG_LINK_DOWN_RST_EN_FIELD_MASK,
    0,
    XPORT_PORTRESET_CONFIG_LINK_DOWN_RST_EN_FIELD_WIDTH,
    XPORT_PORTRESET_CONFIG_LINK_DOWN_RST_EN_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_PORTRESET_CONFIG_ENABLE_SM_RUN
 ******************************************************************************/
const ru_field_rec XPORT_PORTRESET_CONFIG_ENABLE_SM_RUN_FIELD =
{
    "ENABLE_SM_RUN",
#if RU_INCLUDE_DESC
    "",
    "Enable Port Reset State Machine for the port. One bit per port.",
#endif
    XPORT_PORTRESET_CONFIG_ENABLE_SM_RUN_FIELD_MASK,
    0,
    XPORT_PORTRESET_CONFIG_ENABLE_SM_RUN_FIELD_WIDTH,
    XPORT_PORTRESET_CONFIG_ENABLE_SM_RUN_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_PORTRESET_CONFIG_TICK_TIMER_NDIV
 ******************************************************************************/
const ru_field_rec XPORT_PORTRESET_CONFIG_TICK_TIMER_NDIV_FIELD =
{
    "TICK_TIMER_NDIV",
#if RU_INCLUDE_DESC
    "",
    "This setting specifies the period of timer tick pulses, specified as number of timer clock periods.",
#endif
    XPORT_PORTRESET_CONFIG_TICK_TIMER_NDIV_FIELD_MASK,
    0,
    XPORT_PORTRESET_CONFIG_TICK_TIMER_NDIV_FIELD_WIDTH,
    XPORT_PORTRESET_CONFIG_TICK_TIMER_NDIV_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_PORTRESET_P0_LINK_STAT_DEBOUNCE_CFG_RESERVED0
 ******************************************************************************/
const ru_field_rec XPORT_PORTRESET_P0_LINK_STAT_DEBOUNCE_CFG_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    XPORT_PORTRESET_P0_LINK_STAT_DEBOUNCE_CFG_RESERVED0_FIELD_MASK,
    0,
    XPORT_PORTRESET_P0_LINK_STAT_DEBOUNCE_CFG_RESERVED0_FIELD_WIDTH,
    XPORT_PORTRESET_P0_LINK_STAT_DEBOUNCE_CFG_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_PORTRESET_P0_LINK_STAT_DEBOUNCE_CFG_DISABLE
 ******************************************************************************/
const ru_field_rec XPORT_PORTRESET_P0_LINK_STAT_DEBOUNCE_CFG_DISABLE_FIELD =
{
    "DISABLE",
#if RU_INCLUDE_DESC
    "",
    "When this bit is set, XPORT link status debouncer for the port will be disabled, and raw PHY link status will be used by all XPORT blocks.",
#endif
    XPORT_PORTRESET_P0_LINK_STAT_DEBOUNCE_CFG_DISABLE_FIELD_MASK,
    0,
    XPORT_PORTRESET_P0_LINK_STAT_DEBOUNCE_CFG_DISABLE_FIELD_WIDTH,
    XPORT_PORTRESET_P0_LINK_STAT_DEBOUNCE_CFG_DISABLE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_PORTRESET_P0_LINK_STAT_DEBOUNCE_CFG_DEBOUNCE_TIME
 ******************************************************************************/
const ru_field_rec XPORT_PORTRESET_P0_LINK_STAT_DEBOUNCE_CFG_DEBOUNCE_TIME_FIELD =
{
    "DEBOUNCE_TIME",
#if RU_INCLUDE_DESC
    "",
    "Link status debounce time (link-up delay), in ms.",
#endif
    XPORT_PORTRESET_P0_LINK_STAT_DEBOUNCE_CFG_DEBOUNCE_TIME_FIELD_MASK,
    0,
    XPORT_PORTRESET_P0_LINK_STAT_DEBOUNCE_CFG_DEBOUNCE_TIME_FIELD_WIDTH,
    XPORT_PORTRESET_P0_LINK_STAT_DEBOUNCE_CFG_DEBOUNCE_TIME_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_PORTRESET_P1_LINK_STAT_DEBOUNCE_CFG_RESERVED0
 ******************************************************************************/
const ru_field_rec XPORT_PORTRESET_P1_LINK_STAT_DEBOUNCE_CFG_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    XPORT_PORTRESET_P1_LINK_STAT_DEBOUNCE_CFG_RESERVED0_FIELD_MASK,
    0,
    XPORT_PORTRESET_P1_LINK_STAT_DEBOUNCE_CFG_RESERVED0_FIELD_WIDTH,
    XPORT_PORTRESET_P1_LINK_STAT_DEBOUNCE_CFG_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_PORTRESET_P1_LINK_STAT_DEBOUNCE_CFG_DISABLE
 ******************************************************************************/
const ru_field_rec XPORT_PORTRESET_P1_LINK_STAT_DEBOUNCE_CFG_DISABLE_FIELD =
{
    "DISABLE",
#if RU_INCLUDE_DESC
    "",
    "When this bit is set, XPORT link status debouncer for the port will be disabled, and raw PHY link status will be used by all XPORT blocks.",
#endif
    XPORT_PORTRESET_P1_LINK_STAT_DEBOUNCE_CFG_DISABLE_FIELD_MASK,
    0,
    XPORT_PORTRESET_P1_LINK_STAT_DEBOUNCE_CFG_DISABLE_FIELD_WIDTH,
    XPORT_PORTRESET_P1_LINK_STAT_DEBOUNCE_CFG_DISABLE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_PORTRESET_P1_LINK_STAT_DEBOUNCE_CFG_DEBOUNCE_TIME
 ******************************************************************************/
const ru_field_rec XPORT_PORTRESET_P1_LINK_STAT_DEBOUNCE_CFG_DEBOUNCE_TIME_FIELD =
{
    "DEBOUNCE_TIME",
#if RU_INCLUDE_DESC
    "",
    "Link status debounce time (link-up delay), in ms.",
#endif
    XPORT_PORTRESET_P1_LINK_STAT_DEBOUNCE_CFG_DEBOUNCE_TIME_FIELD_MASK,
    0,
    XPORT_PORTRESET_P1_LINK_STAT_DEBOUNCE_CFG_DEBOUNCE_TIME_FIELD_WIDTH,
    XPORT_PORTRESET_P1_LINK_STAT_DEBOUNCE_CFG_DEBOUNCE_TIME_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_PORTRESET_P2_LINK_STAT_DEBOUNCE_CFG_RESERVED0
 ******************************************************************************/
const ru_field_rec XPORT_PORTRESET_P2_LINK_STAT_DEBOUNCE_CFG_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    XPORT_PORTRESET_P2_LINK_STAT_DEBOUNCE_CFG_RESERVED0_FIELD_MASK,
    0,
    XPORT_PORTRESET_P2_LINK_STAT_DEBOUNCE_CFG_RESERVED0_FIELD_WIDTH,
    XPORT_PORTRESET_P2_LINK_STAT_DEBOUNCE_CFG_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_PORTRESET_P2_LINK_STAT_DEBOUNCE_CFG_DISABLE
 ******************************************************************************/
const ru_field_rec XPORT_PORTRESET_P2_LINK_STAT_DEBOUNCE_CFG_DISABLE_FIELD =
{
    "DISABLE",
#if RU_INCLUDE_DESC
    "",
    "When this bit is set, XPORT link status debouncer for the port will be disabled, and raw PHY link status will be used by all XPORT blocks.",
#endif
    XPORT_PORTRESET_P2_LINK_STAT_DEBOUNCE_CFG_DISABLE_FIELD_MASK,
    0,
    XPORT_PORTRESET_P2_LINK_STAT_DEBOUNCE_CFG_DISABLE_FIELD_WIDTH,
    XPORT_PORTRESET_P2_LINK_STAT_DEBOUNCE_CFG_DISABLE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_PORTRESET_P2_LINK_STAT_DEBOUNCE_CFG_DEBOUNCE_TIME
 ******************************************************************************/
const ru_field_rec XPORT_PORTRESET_P2_LINK_STAT_DEBOUNCE_CFG_DEBOUNCE_TIME_FIELD =
{
    "DEBOUNCE_TIME",
#if RU_INCLUDE_DESC
    "",
    "Link status debounce time (link-up delay), in ms.",
#endif
    XPORT_PORTRESET_P2_LINK_STAT_DEBOUNCE_CFG_DEBOUNCE_TIME_FIELD_MASK,
    0,
    XPORT_PORTRESET_P2_LINK_STAT_DEBOUNCE_CFG_DEBOUNCE_TIME_FIELD_WIDTH,
    XPORT_PORTRESET_P2_LINK_STAT_DEBOUNCE_CFG_DEBOUNCE_TIME_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_PORTRESET_P3_LINK_STAT_DEBOUNCE_CFG_RESERVED0
 ******************************************************************************/
const ru_field_rec XPORT_PORTRESET_P3_LINK_STAT_DEBOUNCE_CFG_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    XPORT_PORTRESET_P3_LINK_STAT_DEBOUNCE_CFG_RESERVED0_FIELD_MASK,
    0,
    XPORT_PORTRESET_P3_LINK_STAT_DEBOUNCE_CFG_RESERVED0_FIELD_WIDTH,
    XPORT_PORTRESET_P3_LINK_STAT_DEBOUNCE_CFG_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_PORTRESET_P3_LINK_STAT_DEBOUNCE_CFG_DISABLE
 ******************************************************************************/
const ru_field_rec XPORT_PORTRESET_P3_LINK_STAT_DEBOUNCE_CFG_DISABLE_FIELD =
{
    "DISABLE",
#if RU_INCLUDE_DESC
    "",
    "When this bit is set, XPORT link status debouncer for the port will be disabled, and raw PHY link status will be used by all XPORT blocks.",
#endif
    XPORT_PORTRESET_P3_LINK_STAT_DEBOUNCE_CFG_DISABLE_FIELD_MASK,
    0,
    XPORT_PORTRESET_P3_LINK_STAT_DEBOUNCE_CFG_DISABLE_FIELD_WIDTH,
    XPORT_PORTRESET_P3_LINK_STAT_DEBOUNCE_CFG_DISABLE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_PORTRESET_P3_LINK_STAT_DEBOUNCE_CFG_DEBOUNCE_TIME
 ******************************************************************************/
const ru_field_rec XPORT_PORTRESET_P3_LINK_STAT_DEBOUNCE_CFG_DEBOUNCE_TIME_FIELD =
{
    "DEBOUNCE_TIME",
#if RU_INCLUDE_DESC
    "",
    "Link status debounce time (link-up delay), in ms.",
#endif
    XPORT_PORTRESET_P3_LINK_STAT_DEBOUNCE_CFG_DEBOUNCE_TIME_FIELD_MASK,
    0,
    XPORT_PORTRESET_P3_LINK_STAT_DEBOUNCE_CFG_DEBOUNCE_TIME_FIELD_WIDTH,
    XPORT_PORTRESET_P3_LINK_STAT_DEBOUNCE_CFG_DEBOUNCE_TIME_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_PORTRESET_P0_SIG_EN_RESERVED0
 ******************************************************************************/
const ru_field_rec XPORT_PORTRESET_P0_SIG_EN_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    XPORT_PORTRESET_P0_SIG_EN_RESERVED0_FIELD_MASK,
    0,
    XPORT_PORTRESET_P0_SIG_EN_RESERVED0_FIELD_WIDTH,
    XPORT_PORTRESET_P0_SIG_EN_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_PORTRESET_P0_SIG_EN_ENABLE_XLMAC_RX_DISAB
 ******************************************************************************/
const ru_field_rec XPORT_PORTRESET_P0_SIG_EN_ENABLE_XLMAC_RX_DISAB_FIELD =
{
    "ENABLE_XLMAC_RX_DISAB",
#if RU_INCLUDE_DESC
    "",
    "...",
#endif
    XPORT_PORTRESET_P0_SIG_EN_ENABLE_XLMAC_RX_DISAB_FIELD_MASK,
    0,
    XPORT_PORTRESET_P0_SIG_EN_ENABLE_XLMAC_RX_DISAB_FIELD_WIDTH,
    XPORT_PORTRESET_P0_SIG_EN_ENABLE_XLMAC_RX_DISAB_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_PORTRESET_P0_SIG_EN_ENABLE_XLMAC_TX_DISAB
 ******************************************************************************/
const ru_field_rec XPORT_PORTRESET_P0_SIG_EN_ENABLE_XLMAC_TX_DISAB_FIELD =
{
    "ENABLE_XLMAC_TX_DISAB",
#if RU_INCLUDE_DESC
    "",
    "...",
#endif
    XPORT_PORTRESET_P0_SIG_EN_ENABLE_XLMAC_TX_DISAB_FIELD_MASK,
    0,
    XPORT_PORTRESET_P0_SIG_EN_ENABLE_XLMAC_TX_DISAB_FIELD_WIDTH,
    XPORT_PORTRESET_P0_SIG_EN_ENABLE_XLMAC_TX_DISAB_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_PORTRESET_P0_SIG_EN_ENABLE_XLMAC_TX_DISCARD
 ******************************************************************************/
const ru_field_rec XPORT_PORTRESET_P0_SIG_EN_ENABLE_XLMAC_TX_DISCARD_FIELD =
{
    "ENABLE_XLMAC_TX_DISCARD",
#if RU_INCLUDE_DESC
    "",
    "...",
#endif
    XPORT_PORTRESET_P0_SIG_EN_ENABLE_XLMAC_TX_DISCARD_FIELD_MASK,
    0,
    XPORT_PORTRESET_P0_SIG_EN_ENABLE_XLMAC_TX_DISCARD_FIELD_WIDTH,
    XPORT_PORTRESET_P0_SIG_EN_ENABLE_XLMAC_TX_DISCARD_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_PORTRESET_P0_SIG_EN_ENABLE_XLMAC_SOFT_RESET
 ******************************************************************************/
const ru_field_rec XPORT_PORTRESET_P0_SIG_EN_ENABLE_XLMAC_SOFT_RESET_FIELD =
{
    "ENABLE_XLMAC_SOFT_RESET",
#if RU_INCLUDE_DESC
    "",
    "...",
#endif
    XPORT_PORTRESET_P0_SIG_EN_ENABLE_XLMAC_SOFT_RESET_FIELD_MASK,
    0,
    XPORT_PORTRESET_P0_SIG_EN_ENABLE_XLMAC_SOFT_RESET_FIELD_WIDTH,
    XPORT_PORTRESET_P0_SIG_EN_ENABLE_XLMAC_SOFT_RESET_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_PORTRESET_P0_SIG_EN_ENABLE_MAB_RX_PORT_INIT
 ******************************************************************************/
const ru_field_rec XPORT_PORTRESET_P0_SIG_EN_ENABLE_MAB_RX_PORT_INIT_FIELD =
{
    "ENABLE_MAB_RX_PORT_INIT",
#if RU_INCLUDE_DESC
    "",
    "...",
#endif
    XPORT_PORTRESET_P0_SIG_EN_ENABLE_MAB_RX_PORT_INIT_FIELD_MASK,
    0,
    XPORT_PORTRESET_P0_SIG_EN_ENABLE_MAB_RX_PORT_INIT_FIELD_WIDTH,
    XPORT_PORTRESET_P0_SIG_EN_ENABLE_MAB_RX_PORT_INIT_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_PORTRESET_P0_SIG_EN_ENABLE_MAB_TX_PORT_INIT
 ******************************************************************************/
const ru_field_rec XPORT_PORTRESET_P0_SIG_EN_ENABLE_MAB_TX_PORT_INIT_FIELD =
{
    "ENABLE_MAB_TX_PORT_INIT",
#if RU_INCLUDE_DESC
    "",
    "...",
#endif
    XPORT_PORTRESET_P0_SIG_EN_ENABLE_MAB_TX_PORT_INIT_FIELD_MASK,
    0,
    XPORT_PORTRESET_P0_SIG_EN_ENABLE_MAB_TX_PORT_INIT_FIELD_WIDTH,
    XPORT_PORTRESET_P0_SIG_EN_ENABLE_MAB_TX_PORT_INIT_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_PORTRESET_P0_SIG_EN_ENABLE_MAB_TX_CREDIT_DISAB
 ******************************************************************************/
const ru_field_rec XPORT_PORTRESET_P0_SIG_EN_ENABLE_MAB_TX_CREDIT_DISAB_FIELD =
{
    "ENABLE_MAB_TX_CREDIT_DISAB",
#if RU_INCLUDE_DESC
    "",
    "...",
#endif
    XPORT_PORTRESET_P0_SIG_EN_ENABLE_MAB_TX_CREDIT_DISAB_FIELD_MASK,
    0,
    XPORT_PORTRESET_P0_SIG_EN_ENABLE_MAB_TX_CREDIT_DISAB_FIELD_WIDTH,
    XPORT_PORTRESET_P0_SIG_EN_ENABLE_MAB_TX_CREDIT_DISAB_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_PORTRESET_P0_SIG_EN_ENABLE_MAB_TX_FIFO_INIT
 ******************************************************************************/
const ru_field_rec XPORT_PORTRESET_P0_SIG_EN_ENABLE_MAB_TX_FIFO_INIT_FIELD =
{
    "ENABLE_MAB_TX_FIFO_INIT",
#if RU_INCLUDE_DESC
    "",
    "...",
#endif
    XPORT_PORTRESET_P0_SIG_EN_ENABLE_MAB_TX_FIFO_INIT_FIELD_MASK,
    0,
    XPORT_PORTRESET_P0_SIG_EN_ENABLE_MAB_TX_FIFO_INIT_FIELD_WIDTH,
    XPORT_PORTRESET_P0_SIG_EN_ENABLE_MAB_TX_FIFO_INIT_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_PORTRESET_P0_SIG_EN_ENABLE_PORT_IS_UNDER_RESET
 ******************************************************************************/
const ru_field_rec XPORT_PORTRESET_P0_SIG_EN_ENABLE_PORT_IS_UNDER_RESET_FIELD =
{
    "ENABLE_PORT_IS_UNDER_RESET",
#if RU_INCLUDE_DESC
    "",
    "...",
#endif
    XPORT_PORTRESET_P0_SIG_EN_ENABLE_PORT_IS_UNDER_RESET_FIELD_MASK,
    0,
    XPORT_PORTRESET_P0_SIG_EN_ENABLE_PORT_IS_UNDER_RESET_FIELD_WIDTH,
    XPORT_PORTRESET_P0_SIG_EN_ENABLE_PORT_IS_UNDER_RESET_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_PORTRESET_P0_SIG_EN_ENABLE_XLMAC_EP_DISCARD
 ******************************************************************************/
const ru_field_rec XPORT_PORTRESET_P0_SIG_EN_ENABLE_XLMAC_EP_DISCARD_FIELD =
{
    "ENABLE_XLMAC_EP_DISCARD",
#if RU_INCLUDE_DESC
    "",
    "...",
#endif
    XPORT_PORTRESET_P0_SIG_EN_ENABLE_XLMAC_EP_DISCARD_FIELD_MASK,
    0,
    XPORT_PORTRESET_P0_SIG_EN_ENABLE_XLMAC_EP_DISCARD_FIELD_WIDTH,
    XPORT_PORTRESET_P0_SIG_EN_ENABLE_XLMAC_EP_DISCARD_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_PORTRESET_P0_SIG_ASSERT_TIMES_0_XLMAC_RX_DISAB_ASSERT_TIME
 ******************************************************************************/
const ru_field_rec XPORT_PORTRESET_P0_SIG_ASSERT_TIMES_0_XLMAC_RX_DISAB_ASSERT_TIME_FIELD =
{
    "XLMAC_RX_DISAB_ASSERT_TIME",
#if RU_INCLUDE_DESC
    "",
    "...",
#endif
    XPORT_PORTRESET_P0_SIG_ASSERT_TIMES_0_XLMAC_RX_DISAB_ASSERT_TIME_FIELD_MASK,
    0,
    XPORT_PORTRESET_P0_SIG_ASSERT_TIMES_0_XLMAC_RX_DISAB_ASSERT_TIME_FIELD_WIDTH,
    XPORT_PORTRESET_P0_SIG_ASSERT_TIMES_0_XLMAC_RX_DISAB_ASSERT_TIME_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_PORTRESET_P0_SIG_ASSERT_TIMES_0_XLMAC_TX_DISAB_ASSERT_TIME
 ******************************************************************************/
const ru_field_rec XPORT_PORTRESET_P0_SIG_ASSERT_TIMES_0_XLMAC_TX_DISAB_ASSERT_TIME_FIELD =
{
    "XLMAC_TX_DISAB_ASSERT_TIME",
#if RU_INCLUDE_DESC
    "",
    "...",
#endif
    XPORT_PORTRESET_P0_SIG_ASSERT_TIMES_0_XLMAC_TX_DISAB_ASSERT_TIME_FIELD_MASK,
    0,
    XPORT_PORTRESET_P0_SIG_ASSERT_TIMES_0_XLMAC_TX_DISAB_ASSERT_TIME_FIELD_WIDTH,
    XPORT_PORTRESET_P0_SIG_ASSERT_TIMES_0_XLMAC_TX_DISAB_ASSERT_TIME_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_PORTRESET_P0_SIG_ASSERT_TIMES_1_XLMAC_TXDISCARD_ASSERT_TIME
 ******************************************************************************/
const ru_field_rec XPORT_PORTRESET_P0_SIG_ASSERT_TIMES_1_XLMAC_TXDISCARD_ASSERT_TIME_FIELD =
{
    "XLMAC_TXDISCARD_ASSERT_TIME",
#if RU_INCLUDE_DESC
    "",
    "...",
#endif
    XPORT_PORTRESET_P0_SIG_ASSERT_TIMES_1_XLMAC_TXDISCARD_ASSERT_TIME_FIELD_MASK,
    0,
    XPORT_PORTRESET_P0_SIG_ASSERT_TIMES_1_XLMAC_TXDISCARD_ASSERT_TIME_FIELD_WIDTH,
    XPORT_PORTRESET_P0_SIG_ASSERT_TIMES_1_XLMAC_TXDISCARD_ASSERT_TIME_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_PORTRESET_P0_SIG_ASSERT_TIMES_1_XLMAC_SOFT_RESET_ASSERT_TIME
 ******************************************************************************/
const ru_field_rec XPORT_PORTRESET_P0_SIG_ASSERT_TIMES_1_XLMAC_SOFT_RESET_ASSERT_TIME_FIELD =
{
    "XLMAC_SOFT_RESET_ASSERT_TIME",
#if RU_INCLUDE_DESC
    "",
    "...",
#endif
    XPORT_PORTRESET_P0_SIG_ASSERT_TIMES_1_XLMAC_SOFT_RESET_ASSERT_TIME_FIELD_MASK,
    0,
    XPORT_PORTRESET_P0_SIG_ASSERT_TIMES_1_XLMAC_SOFT_RESET_ASSERT_TIME_FIELD_WIDTH,
    XPORT_PORTRESET_P0_SIG_ASSERT_TIMES_1_XLMAC_SOFT_RESET_ASSERT_TIME_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_PORTRESET_P0_SIG_ASSERT_TIMES_2_MAB_RX_PORT_INIT_ASSERT_TIME
 ******************************************************************************/
const ru_field_rec XPORT_PORTRESET_P0_SIG_ASSERT_TIMES_2_MAB_RX_PORT_INIT_ASSERT_TIME_FIELD =
{
    "MAB_RX_PORT_INIT_ASSERT_TIME",
#if RU_INCLUDE_DESC
    "",
    "...",
#endif
    XPORT_PORTRESET_P0_SIG_ASSERT_TIMES_2_MAB_RX_PORT_INIT_ASSERT_TIME_FIELD_MASK,
    0,
    XPORT_PORTRESET_P0_SIG_ASSERT_TIMES_2_MAB_RX_PORT_INIT_ASSERT_TIME_FIELD_WIDTH,
    XPORT_PORTRESET_P0_SIG_ASSERT_TIMES_2_MAB_RX_PORT_INIT_ASSERT_TIME_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_PORTRESET_P0_SIG_ASSERT_TIMES_2_MAB_TX_PORT_INIT_ASSERT_TIME
 ******************************************************************************/
const ru_field_rec XPORT_PORTRESET_P0_SIG_ASSERT_TIMES_2_MAB_TX_PORT_INIT_ASSERT_TIME_FIELD =
{
    "MAB_TX_PORT_INIT_ASSERT_TIME",
#if RU_INCLUDE_DESC
    "",
    "...",
#endif
    XPORT_PORTRESET_P0_SIG_ASSERT_TIMES_2_MAB_TX_PORT_INIT_ASSERT_TIME_FIELD_MASK,
    0,
    XPORT_PORTRESET_P0_SIG_ASSERT_TIMES_2_MAB_TX_PORT_INIT_ASSERT_TIME_FIELD_WIDTH,
    XPORT_PORTRESET_P0_SIG_ASSERT_TIMES_2_MAB_TX_PORT_INIT_ASSERT_TIME_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_PORTRESET_P0_SIG_ASSERT_TIMES_3_MAB_TX_CREDIT_DISAB_ASSERT_TIME
 ******************************************************************************/
const ru_field_rec XPORT_PORTRESET_P0_SIG_ASSERT_TIMES_3_MAB_TX_CREDIT_DISAB_ASSERT_TIME_FIELD =
{
    "MAB_TX_CREDIT_DISAB_ASSERT_TIME",
#if RU_INCLUDE_DESC
    "",
    "...",
#endif
    XPORT_PORTRESET_P0_SIG_ASSERT_TIMES_3_MAB_TX_CREDIT_DISAB_ASSERT_TIME_FIELD_MASK,
    0,
    XPORT_PORTRESET_P0_SIG_ASSERT_TIMES_3_MAB_TX_CREDIT_DISAB_ASSERT_TIME_FIELD_WIDTH,
    XPORT_PORTRESET_P0_SIG_ASSERT_TIMES_3_MAB_TX_CREDIT_DISAB_ASSERT_TIME_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_PORTRESET_P0_SIG_ASSERT_TIMES_3_MAB_TX_FIFO_INIT_ASSERT_TIME
 ******************************************************************************/
const ru_field_rec XPORT_PORTRESET_P0_SIG_ASSERT_TIMES_3_MAB_TX_FIFO_INIT_ASSERT_TIME_FIELD =
{
    "MAB_TX_FIFO_INIT_ASSERT_TIME",
#if RU_INCLUDE_DESC
    "",
    "...",
#endif
    XPORT_PORTRESET_P0_SIG_ASSERT_TIMES_3_MAB_TX_FIFO_INIT_ASSERT_TIME_FIELD_MASK,
    0,
    XPORT_PORTRESET_P0_SIG_ASSERT_TIMES_3_MAB_TX_FIFO_INIT_ASSERT_TIME_FIELD_WIDTH,
    XPORT_PORTRESET_P0_SIG_ASSERT_TIMES_3_MAB_TX_FIFO_INIT_ASSERT_TIME_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_PORTRESET_P0_SIG_ASSERT_TIMES_4_PORT_IS_UNDER_RESET_ASSERT_TIME
 ******************************************************************************/
const ru_field_rec XPORT_PORTRESET_P0_SIG_ASSERT_TIMES_4_PORT_IS_UNDER_RESET_ASSERT_TIME_FIELD =
{
    "PORT_IS_UNDER_RESET_ASSERT_TIME",
#if RU_INCLUDE_DESC
    "",
    "...",
#endif
    XPORT_PORTRESET_P0_SIG_ASSERT_TIMES_4_PORT_IS_UNDER_RESET_ASSERT_TIME_FIELD_MASK,
    0,
    XPORT_PORTRESET_P0_SIG_ASSERT_TIMES_4_PORT_IS_UNDER_RESET_ASSERT_TIME_FIELD_WIDTH,
    XPORT_PORTRESET_P0_SIG_ASSERT_TIMES_4_PORT_IS_UNDER_RESET_ASSERT_TIME_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_PORTRESET_P0_SIG_ASSERT_TIMES_4_RESERVED0
 ******************************************************************************/
const ru_field_rec XPORT_PORTRESET_P0_SIG_ASSERT_TIMES_4_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    XPORT_PORTRESET_P0_SIG_ASSERT_TIMES_4_RESERVED0_FIELD_MASK,
    0,
    XPORT_PORTRESET_P0_SIG_ASSERT_TIMES_4_RESERVED0_FIELD_WIDTH,
    XPORT_PORTRESET_P0_SIG_ASSERT_TIMES_4_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_PORTRESET_P0_SIG_DEASSERT_TIMES_0_XLMAC_RX_DISAB_DEASSERT_TIME
 ******************************************************************************/
const ru_field_rec XPORT_PORTRESET_P0_SIG_DEASSERT_TIMES_0_XLMAC_RX_DISAB_DEASSERT_TIME_FIELD =
{
    "XLMAC_RX_DISAB_DEASSERT_TIME",
#if RU_INCLUDE_DESC
    "",
    "...",
#endif
    XPORT_PORTRESET_P0_SIG_DEASSERT_TIMES_0_XLMAC_RX_DISAB_DEASSERT_TIME_FIELD_MASK,
    0,
    XPORT_PORTRESET_P0_SIG_DEASSERT_TIMES_0_XLMAC_RX_DISAB_DEASSERT_TIME_FIELD_WIDTH,
    XPORT_PORTRESET_P0_SIG_DEASSERT_TIMES_0_XLMAC_RX_DISAB_DEASSERT_TIME_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_PORTRESET_P0_SIG_DEASSERT_TIMES_0_XLMAC_TX_DISAB_DEASSERT_TIME
 ******************************************************************************/
const ru_field_rec XPORT_PORTRESET_P0_SIG_DEASSERT_TIMES_0_XLMAC_TX_DISAB_DEASSERT_TIME_FIELD =
{
    "XLMAC_TX_DISAB_DEASSERT_TIME",
#if RU_INCLUDE_DESC
    "",
    "...",
#endif
    XPORT_PORTRESET_P0_SIG_DEASSERT_TIMES_0_XLMAC_TX_DISAB_DEASSERT_TIME_FIELD_MASK,
    0,
    XPORT_PORTRESET_P0_SIG_DEASSERT_TIMES_0_XLMAC_TX_DISAB_DEASSERT_TIME_FIELD_WIDTH,
    XPORT_PORTRESET_P0_SIG_DEASSERT_TIMES_0_XLMAC_TX_DISAB_DEASSERT_TIME_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_PORTRESET_P0_SIG_DEASSERT_TIMES_1_XLMAC_TXDISCARD_DEASSERT_TIME
 ******************************************************************************/
const ru_field_rec XPORT_PORTRESET_P0_SIG_DEASSERT_TIMES_1_XLMAC_TXDISCARD_DEASSERT_TIME_FIELD =
{
    "XLMAC_TXDISCARD_DEASSERT_TIME",
#if RU_INCLUDE_DESC
    "",
    "...",
#endif
    XPORT_PORTRESET_P0_SIG_DEASSERT_TIMES_1_XLMAC_TXDISCARD_DEASSERT_TIME_FIELD_MASK,
    0,
    XPORT_PORTRESET_P0_SIG_DEASSERT_TIMES_1_XLMAC_TXDISCARD_DEASSERT_TIME_FIELD_WIDTH,
    XPORT_PORTRESET_P0_SIG_DEASSERT_TIMES_1_XLMAC_TXDISCARD_DEASSERT_TIME_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_PORTRESET_P0_SIG_DEASSERT_TIMES_1_XLMAC_SOFT_RESET_DEASSERT_TIME
 ******************************************************************************/
const ru_field_rec XPORT_PORTRESET_P0_SIG_DEASSERT_TIMES_1_XLMAC_SOFT_RESET_DEASSERT_TIME_FIELD =
{
    "XLMAC_SOFT_RESET_DEASSERT_TIME",
#if RU_INCLUDE_DESC
    "",
    "...",
#endif
    XPORT_PORTRESET_P0_SIG_DEASSERT_TIMES_1_XLMAC_SOFT_RESET_DEASSERT_TIME_FIELD_MASK,
    0,
    XPORT_PORTRESET_P0_SIG_DEASSERT_TIMES_1_XLMAC_SOFT_RESET_DEASSERT_TIME_FIELD_WIDTH,
    XPORT_PORTRESET_P0_SIG_DEASSERT_TIMES_1_XLMAC_SOFT_RESET_DEASSERT_TIME_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_PORTRESET_P0_SIG_DEASSERT_TIMES_2_MAB_RX_PORT_INIT_DEASSERT_TIME
 ******************************************************************************/
const ru_field_rec XPORT_PORTRESET_P0_SIG_DEASSERT_TIMES_2_MAB_RX_PORT_INIT_DEASSERT_TIME_FIELD =
{
    "MAB_RX_PORT_INIT_DEASSERT_TIME",
#if RU_INCLUDE_DESC
    "",
    "...",
#endif
    XPORT_PORTRESET_P0_SIG_DEASSERT_TIMES_2_MAB_RX_PORT_INIT_DEASSERT_TIME_FIELD_MASK,
    0,
    XPORT_PORTRESET_P0_SIG_DEASSERT_TIMES_2_MAB_RX_PORT_INIT_DEASSERT_TIME_FIELD_WIDTH,
    XPORT_PORTRESET_P0_SIG_DEASSERT_TIMES_2_MAB_RX_PORT_INIT_DEASSERT_TIME_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_PORTRESET_P0_SIG_DEASSERT_TIMES_2_MAB_TX_PORT_INIT_DEASSERT_TIME
 ******************************************************************************/
const ru_field_rec XPORT_PORTRESET_P0_SIG_DEASSERT_TIMES_2_MAB_TX_PORT_INIT_DEASSERT_TIME_FIELD =
{
    "MAB_TX_PORT_INIT_DEASSERT_TIME",
#if RU_INCLUDE_DESC
    "",
    "...",
#endif
    XPORT_PORTRESET_P0_SIG_DEASSERT_TIMES_2_MAB_TX_PORT_INIT_DEASSERT_TIME_FIELD_MASK,
    0,
    XPORT_PORTRESET_P0_SIG_DEASSERT_TIMES_2_MAB_TX_PORT_INIT_DEASSERT_TIME_FIELD_WIDTH,
    XPORT_PORTRESET_P0_SIG_DEASSERT_TIMES_2_MAB_TX_PORT_INIT_DEASSERT_TIME_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_PORTRESET_P0_SIG_DEASSERT_TIMES_3_MAB_TX_CREDIT_DISAB_DEASSERT_TIME
 ******************************************************************************/
const ru_field_rec XPORT_PORTRESET_P0_SIG_DEASSERT_TIMES_3_MAB_TX_CREDIT_DISAB_DEASSERT_TIME_FIELD =
{
    "MAB_TX_CREDIT_DISAB_DEASSERT_TIME",
#if RU_INCLUDE_DESC
    "",
    "...",
#endif
    XPORT_PORTRESET_P0_SIG_DEASSERT_TIMES_3_MAB_TX_CREDIT_DISAB_DEASSERT_TIME_FIELD_MASK,
    0,
    XPORT_PORTRESET_P0_SIG_DEASSERT_TIMES_3_MAB_TX_CREDIT_DISAB_DEASSERT_TIME_FIELD_WIDTH,
    XPORT_PORTRESET_P0_SIG_DEASSERT_TIMES_3_MAB_TX_CREDIT_DISAB_DEASSERT_TIME_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_PORTRESET_P0_SIG_DEASSERT_TIMES_3_MAB_TX_FIFO_INIT_DEASSERT_TIME
 ******************************************************************************/
const ru_field_rec XPORT_PORTRESET_P0_SIG_DEASSERT_TIMES_3_MAB_TX_FIFO_INIT_DEASSERT_TIME_FIELD =
{
    "MAB_TX_FIFO_INIT_DEASSERT_TIME",
#if RU_INCLUDE_DESC
    "",
    "...",
#endif
    XPORT_PORTRESET_P0_SIG_DEASSERT_TIMES_3_MAB_TX_FIFO_INIT_DEASSERT_TIME_FIELD_MASK,
    0,
    XPORT_PORTRESET_P0_SIG_DEASSERT_TIMES_3_MAB_TX_FIFO_INIT_DEASSERT_TIME_FIELD_WIDTH,
    XPORT_PORTRESET_P0_SIG_DEASSERT_TIMES_3_MAB_TX_FIFO_INIT_DEASSERT_TIME_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_PORTRESET_P0_SIG_DEASSERT_TIMES_4_PORT_IS_UNDER_RESET_DEASSERT_TIME
 ******************************************************************************/
const ru_field_rec XPORT_PORTRESET_P0_SIG_DEASSERT_TIMES_4_PORT_IS_UNDER_RESET_DEASSERT_TIME_FIELD =
{
    "PORT_IS_UNDER_RESET_DEASSERT_TIME",
#if RU_INCLUDE_DESC
    "",
    "...",
#endif
    XPORT_PORTRESET_P0_SIG_DEASSERT_TIMES_4_PORT_IS_UNDER_RESET_DEASSERT_TIME_FIELD_MASK,
    0,
    XPORT_PORTRESET_P0_SIG_DEASSERT_TIMES_4_PORT_IS_UNDER_RESET_DEASSERT_TIME_FIELD_WIDTH,
    XPORT_PORTRESET_P0_SIG_DEASSERT_TIMES_4_PORT_IS_UNDER_RESET_DEASSERT_TIME_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_PORTRESET_P0_SIG_DEASSERT_TIMES_4_RESERVED0
 ******************************************************************************/
const ru_field_rec XPORT_PORTRESET_P0_SIG_DEASSERT_TIMES_4_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    XPORT_PORTRESET_P0_SIG_DEASSERT_TIMES_4_RESERVED0_FIELD_MASK,
    0,
    XPORT_PORTRESET_P0_SIG_DEASSERT_TIMES_4_RESERVED0_FIELD_WIDTH,
    XPORT_PORTRESET_P0_SIG_DEASSERT_TIMES_4_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_PORTRESET_P1_SIG_EN_RESERVED0
 ******************************************************************************/
const ru_field_rec XPORT_PORTRESET_P1_SIG_EN_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    XPORT_PORTRESET_P1_SIG_EN_RESERVED0_FIELD_MASK,
    0,
    XPORT_PORTRESET_P1_SIG_EN_RESERVED0_FIELD_WIDTH,
    XPORT_PORTRESET_P1_SIG_EN_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_PORTRESET_P1_SIG_EN_ENABLE_XLMAC_RX_DISAB
 ******************************************************************************/
const ru_field_rec XPORT_PORTRESET_P1_SIG_EN_ENABLE_XLMAC_RX_DISAB_FIELD =
{
    "ENABLE_XLMAC_RX_DISAB",
#if RU_INCLUDE_DESC
    "",
    "...",
#endif
    XPORT_PORTRESET_P1_SIG_EN_ENABLE_XLMAC_RX_DISAB_FIELD_MASK,
    0,
    XPORT_PORTRESET_P1_SIG_EN_ENABLE_XLMAC_RX_DISAB_FIELD_WIDTH,
    XPORT_PORTRESET_P1_SIG_EN_ENABLE_XLMAC_RX_DISAB_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_PORTRESET_P1_SIG_EN_ENABLE_XLMAC_TX_DISAB
 ******************************************************************************/
const ru_field_rec XPORT_PORTRESET_P1_SIG_EN_ENABLE_XLMAC_TX_DISAB_FIELD =
{
    "ENABLE_XLMAC_TX_DISAB",
#if RU_INCLUDE_DESC
    "",
    "...",
#endif
    XPORT_PORTRESET_P1_SIG_EN_ENABLE_XLMAC_TX_DISAB_FIELD_MASK,
    0,
    XPORT_PORTRESET_P1_SIG_EN_ENABLE_XLMAC_TX_DISAB_FIELD_WIDTH,
    XPORT_PORTRESET_P1_SIG_EN_ENABLE_XLMAC_TX_DISAB_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_PORTRESET_P1_SIG_EN_ENABLE_XLMAC_TX_DISCARD
 ******************************************************************************/
const ru_field_rec XPORT_PORTRESET_P1_SIG_EN_ENABLE_XLMAC_TX_DISCARD_FIELD =
{
    "ENABLE_XLMAC_TX_DISCARD",
#if RU_INCLUDE_DESC
    "",
    "...",
#endif
    XPORT_PORTRESET_P1_SIG_EN_ENABLE_XLMAC_TX_DISCARD_FIELD_MASK,
    0,
    XPORT_PORTRESET_P1_SIG_EN_ENABLE_XLMAC_TX_DISCARD_FIELD_WIDTH,
    XPORT_PORTRESET_P1_SIG_EN_ENABLE_XLMAC_TX_DISCARD_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_PORTRESET_P1_SIG_EN_ENABLE_XLMAC_SOFT_RESET
 ******************************************************************************/
const ru_field_rec XPORT_PORTRESET_P1_SIG_EN_ENABLE_XLMAC_SOFT_RESET_FIELD =
{
    "ENABLE_XLMAC_SOFT_RESET",
#if RU_INCLUDE_DESC
    "",
    "...",
#endif
    XPORT_PORTRESET_P1_SIG_EN_ENABLE_XLMAC_SOFT_RESET_FIELD_MASK,
    0,
    XPORT_PORTRESET_P1_SIG_EN_ENABLE_XLMAC_SOFT_RESET_FIELD_WIDTH,
    XPORT_PORTRESET_P1_SIG_EN_ENABLE_XLMAC_SOFT_RESET_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_PORTRESET_P1_SIG_EN_ENABLE_MAB_RX_PORT_INIT
 ******************************************************************************/
const ru_field_rec XPORT_PORTRESET_P1_SIG_EN_ENABLE_MAB_RX_PORT_INIT_FIELD =
{
    "ENABLE_MAB_RX_PORT_INIT",
#if RU_INCLUDE_DESC
    "",
    "...",
#endif
    XPORT_PORTRESET_P1_SIG_EN_ENABLE_MAB_RX_PORT_INIT_FIELD_MASK,
    0,
    XPORT_PORTRESET_P1_SIG_EN_ENABLE_MAB_RX_PORT_INIT_FIELD_WIDTH,
    XPORT_PORTRESET_P1_SIG_EN_ENABLE_MAB_RX_PORT_INIT_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_PORTRESET_P1_SIG_EN_ENABLE_MAB_TX_PORT_INIT
 ******************************************************************************/
const ru_field_rec XPORT_PORTRESET_P1_SIG_EN_ENABLE_MAB_TX_PORT_INIT_FIELD =
{
    "ENABLE_MAB_TX_PORT_INIT",
#if RU_INCLUDE_DESC
    "",
    "...",
#endif
    XPORT_PORTRESET_P1_SIG_EN_ENABLE_MAB_TX_PORT_INIT_FIELD_MASK,
    0,
    XPORT_PORTRESET_P1_SIG_EN_ENABLE_MAB_TX_PORT_INIT_FIELD_WIDTH,
    XPORT_PORTRESET_P1_SIG_EN_ENABLE_MAB_TX_PORT_INIT_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_PORTRESET_P1_SIG_EN_ENABLE_MAB_TX_CREDIT_DISAB
 ******************************************************************************/
const ru_field_rec XPORT_PORTRESET_P1_SIG_EN_ENABLE_MAB_TX_CREDIT_DISAB_FIELD =
{
    "ENABLE_MAB_TX_CREDIT_DISAB",
#if RU_INCLUDE_DESC
    "",
    "...",
#endif
    XPORT_PORTRESET_P1_SIG_EN_ENABLE_MAB_TX_CREDIT_DISAB_FIELD_MASK,
    0,
    XPORT_PORTRESET_P1_SIG_EN_ENABLE_MAB_TX_CREDIT_DISAB_FIELD_WIDTH,
    XPORT_PORTRESET_P1_SIG_EN_ENABLE_MAB_TX_CREDIT_DISAB_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_PORTRESET_P1_SIG_EN_ENABLE_MAB_TX_FIFO_INIT
 ******************************************************************************/
const ru_field_rec XPORT_PORTRESET_P1_SIG_EN_ENABLE_MAB_TX_FIFO_INIT_FIELD =
{
    "ENABLE_MAB_TX_FIFO_INIT",
#if RU_INCLUDE_DESC
    "",
    "...",
#endif
    XPORT_PORTRESET_P1_SIG_EN_ENABLE_MAB_TX_FIFO_INIT_FIELD_MASK,
    0,
    XPORT_PORTRESET_P1_SIG_EN_ENABLE_MAB_TX_FIFO_INIT_FIELD_WIDTH,
    XPORT_PORTRESET_P1_SIG_EN_ENABLE_MAB_TX_FIFO_INIT_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_PORTRESET_P1_SIG_EN_ENABLE_PORT_IS_UNDER_RESET
 ******************************************************************************/
const ru_field_rec XPORT_PORTRESET_P1_SIG_EN_ENABLE_PORT_IS_UNDER_RESET_FIELD =
{
    "ENABLE_PORT_IS_UNDER_RESET",
#if RU_INCLUDE_DESC
    "",
    "...",
#endif
    XPORT_PORTRESET_P1_SIG_EN_ENABLE_PORT_IS_UNDER_RESET_FIELD_MASK,
    0,
    XPORT_PORTRESET_P1_SIG_EN_ENABLE_PORT_IS_UNDER_RESET_FIELD_WIDTH,
    XPORT_PORTRESET_P1_SIG_EN_ENABLE_PORT_IS_UNDER_RESET_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_PORTRESET_P1_SIG_EN_ENABLE_XLMAC_EP_DISCARD
 ******************************************************************************/
const ru_field_rec XPORT_PORTRESET_P1_SIG_EN_ENABLE_XLMAC_EP_DISCARD_FIELD =
{
    "ENABLE_XLMAC_EP_DISCARD",
#if RU_INCLUDE_DESC
    "",
    "...",
#endif
    XPORT_PORTRESET_P1_SIG_EN_ENABLE_XLMAC_EP_DISCARD_FIELD_MASK,
    0,
    XPORT_PORTRESET_P1_SIG_EN_ENABLE_XLMAC_EP_DISCARD_FIELD_WIDTH,
    XPORT_PORTRESET_P1_SIG_EN_ENABLE_XLMAC_EP_DISCARD_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_PORTRESET_P1_SIG_ASSERT_TIMES_0_XLMAC_RX_DISAB_ASSERT_TIME
 ******************************************************************************/
const ru_field_rec XPORT_PORTRESET_P1_SIG_ASSERT_TIMES_0_XLMAC_RX_DISAB_ASSERT_TIME_FIELD =
{
    "XLMAC_RX_DISAB_ASSERT_TIME",
#if RU_INCLUDE_DESC
    "",
    "...",
#endif
    XPORT_PORTRESET_P1_SIG_ASSERT_TIMES_0_XLMAC_RX_DISAB_ASSERT_TIME_FIELD_MASK,
    0,
    XPORT_PORTRESET_P1_SIG_ASSERT_TIMES_0_XLMAC_RX_DISAB_ASSERT_TIME_FIELD_WIDTH,
    XPORT_PORTRESET_P1_SIG_ASSERT_TIMES_0_XLMAC_RX_DISAB_ASSERT_TIME_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_PORTRESET_P1_SIG_ASSERT_TIMES_0_XLMAC_TX_DISAB_ASSERT_TIME
 ******************************************************************************/
const ru_field_rec XPORT_PORTRESET_P1_SIG_ASSERT_TIMES_0_XLMAC_TX_DISAB_ASSERT_TIME_FIELD =
{
    "XLMAC_TX_DISAB_ASSERT_TIME",
#if RU_INCLUDE_DESC
    "",
    "...",
#endif
    XPORT_PORTRESET_P1_SIG_ASSERT_TIMES_0_XLMAC_TX_DISAB_ASSERT_TIME_FIELD_MASK,
    0,
    XPORT_PORTRESET_P1_SIG_ASSERT_TIMES_0_XLMAC_TX_DISAB_ASSERT_TIME_FIELD_WIDTH,
    XPORT_PORTRESET_P1_SIG_ASSERT_TIMES_0_XLMAC_TX_DISAB_ASSERT_TIME_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_PORTRESET_P1_SIG_ASSERT_TIMES_1_XLMAC_TXDISCARD_ASSERT_TIME
 ******************************************************************************/
const ru_field_rec XPORT_PORTRESET_P1_SIG_ASSERT_TIMES_1_XLMAC_TXDISCARD_ASSERT_TIME_FIELD =
{
    "XLMAC_TXDISCARD_ASSERT_TIME",
#if RU_INCLUDE_DESC
    "",
    "...",
#endif
    XPORT_PORTRESET_P1_SIG_ASSERT_TIMES_1_XLMAC_TXDISCARD_ASSERT_TIME_FIELD_MASK,
    0,
    XPORT_PORTRESET_P1_SIG_ASSERT_TIMES_1_XLMAC_TXDISCARD_ASSERT_TIME_FIELD_WIDTH,
    XPORT_PORTRESET_P1_SIG_ASSERT_TIMES_1_XLMAC_TXDISCARD_ASSERT_TIME_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_PORTRESET_P1_SIG_ASSERT_TIMES_1_XLMAC_SOFT_RESET_ASSERT_TIME
 ******************************************************************************/
const ru_field_rec XPORT_PORTRESET_P1_SIG_ASSERT_TIMES_1_XLMAC_SOFT_RESET_ASSERT_TIME_FIELD =
{
    "XLMAC_SOFT_RESET_ASSERT_TIME",
#if RU_INCLUDE_DESC
    "",
    "...",
#endif
    XPORT_PORTRESET_P1_SIG_ASSERT_TIMES_1_XLMAC_SOFT_RESET_ASSERT_TIME_FIELD_MASK,
    0,
    XPORT_PORTRESET_P1_SIG_ASSERT_TIMES_1_XLMAC_SOFT_RESET_ASSERT_TIME_FIELD_WIDTH,
    XPORT_PORTRESET_P1_SIG_ASSERT_TIMES_1_XLMAC_SOFT_RESET_ASSERT_TIME_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_PORTRESET_P1_SIG_ASSERT_TIMES_2_MAB_RX_PORT_INIT_ASSERT_TIME
 ******************************************************************************/
const ru_field_rec XPORT_PORTRESET_P1_SIG_ASSERT_TIMES_2_MAB_RX_PORT_INIT_ASSERT_TIME_FIELD =
{
    "MAB_RX_PORT_INIT_ASSERT_TIME",
#if RU_INCLUDE_DESC
    "",
    "...",
#endif
    XPORT_PORTRESET_P1_SIG_ASSERT_TIMES_2_MAB_RX_PORT_INIT_ASSERT_TIME_FIELD_MASK,
    0,
    XPORT_PORTRESET_P1_SIG_ASSERT_TIMES_2_MAB_RX_PORT_INIT_ASSERT_TIME_FIELD_WIDTH,
    XPORT_PORTRESET_P1_SIG_ASSERT_TIMES_2_MAB_RX_PORT_INIT_ASSERT_TIME_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_PORTRESET_P1_SIG_ASSERT_TIMES_2_MAB_TX_PORT_INIT_ASSERT_TIME
 ******************************************************************************/
const ru_field_rec XPORT_PORTRESET_P1_SIG_ASSERT_TIMES_2_MAB_TX_PORT_INIT_ASSERT_TIME_FIELD =
{
    "MAB_TX_PORT_INIT_ASSERT_TIME",
#if RU_INCLUDE_DESC
    "",
    "...",
#endif
    XPORT_PORTRESET_P1_SIG_ASSERT_TIMES_2_MAB_TX_PORT_INIT_ASSERT_TIME_FIELD_MASK,
    0,
    XPORT_PORTRESET_P1_SIG_ASSERT_TIMES_2_MAB_TX_PORT_INIT_ASSERT_TIME_FIELD_WIDTH,
    XPORT_PORTRESET_P1_SIG_ASSERT_TIMES_2_MAB_TX_PORT_INIT_ASSERT_TIME_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_PORTRESET_P1_SIG_ASSERT_TIMES_3_MAB_TX_CREDIT_DISAB_ASSERT_TIME
 ******************************************************************************/
const ru_field_rec XPORT_PORTRESET_P1_SIG_ASSERT_TIMES_3_MAB_TX_CREDIT_DISAB_ASSERT_TIME_FIELD =
{
    "MAB_TX_CREDIT_DISAB_ASSERT_TIME",
#if RU_INCLUDE_DESC
    "",
    "...",
#endif
    XPORT_PORTRESET_P1_SIG_ASSERT_TIMES_3_MAB_TX_CREDIT_DISAB_ASSERT_TIME_FIELD_MASK,
    0,
    XPORT_PORTRESET_P1_SIG_ASSERT_TIMES_3_MAB_TX_CREDIT_DISAB_ASSERT_TIME_FIELD_WIDTH,
    XPORT_PORTRESET_P1_SIG_ASSERT_TIMES_3_MAB_TX_CREDIT_DISAB_ASSERT_TIME_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_PORTRESET_P1_SIG_ASSERT_TIMES_3_MAB_TX_FIFO_INIT_ASSERT_TIME
 ******************************************************************************/
const ru_field_rec XPORT_PORTRESET_P1_SIG_ASSERT_TIMES_3_MAB_TX_FIFO_INIT_ASSERT_TIME_FIELD =
{
    "MAB_TX_FIFO_INIT_ASSERT_TIME",
#if RU_INCLUDE_DESC
    "",
    "...",
#endif
    XPORT_PORTRESET_P1_SIG_ASSERT_TIMES_3_MAB_TX_FIFO_INIT_ASSERT_TIME_FIELD_MASK,
    0,
    XPORT_PORTRESET_P1_SIG_ASSERT_TIMES_3_MAB_TX_FIFO_INIT_ASSERT_TIME_FIELD_WIDTH,
    XPORT_PORTRESET_P1_SIG_ASSERT_TIMES_3_MAB_TX_FIFO_INIT_ASSERT_TIME_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_PORTRESET_P1_SIG_ASSERT_TIMES_4_PORT_IS_UNDER_RESET_ASSERT_TIME
 ******************************************************************************/
const ru_field_rec XPORT_PORTRESET_P1_SIG_ASSERT_TIMES_4_PORT_IS_UNDER_RESET_ASSERT_TIME_FIELD =
{
    "PORT_IS_UNDER_RESET_ASSERT_TIME",
#if RU_INCLUDE_DESC
    "",
    "...",
#endif
    XPORT_PORTRESET_P1_SIG_ASSERT_TIMES_4_PORT_IS_UNDER_RESET_ASSERT_TIME_FIELD_MASK,
    0,
    XPORT_PORTRESET_P1_SIG_ASSERT_TIMES_4_PORT_IS_UNDER_RESET_ASSERT_TIME_FIELD_WIDTH,
    XPORT_PORTRESET_P1_SIG_ASSERT_TIMES_4_PORT_IS_UNDER_RESET_ASSERT_TIME_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_PORTRESET_P1_SIG_ASSERT_TIMES_4_RESERVED0
 ******************************************************************************/
const ru_field_rec XPORT_PORTRESET_P1_SIG_ASSERT_TIMES_4_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    XPORT_PORTRESET_P1_SIG_ASSERT_TIMES_4_RESERVED0_FIELD_MASK,
    0,
    XPORT_PORTRESET_P1_SIG_ASSERT_TIMES_4_RESERVED0_FIELD_WIDTH,
    XPORT_PORTRESET_P1_SIG_ASSERT_TIMES_4_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_PORTRESET_P1_SIG_DEASSERT_TIMES_0_XLMAC_RX_DISAB_DEASSERT_TIME
 ******************************************************************************/
const ru_field_rec XPORT_PORTRESET_P1_SIG_DEASSERT_TIMES_0_XLMAC_RX_DISAB_DEASSERT_TIME_FIELD =
{
    "XLMAC_RX_DISAB_DEASSERT_TIME",
#if RU_INCLUDE_DESC
    "",
    "...",
#endif
    XPORT_PORTRESET_P1_SIG_DEASSERT_TIMES_0_XLMAC_RX_DISAB_DEASSERT_TIME_FIELD_MASK,
    0,
    XPORT_PORTRESET_P1_SIG_DEASSERT_TIMES_0_XLMAC_RX_DISAB_DEASSERT_TIME_FIELD_WIDTH,
    XPORT_PORTRESET_P1_SIG_DEASSERT_TIMES_0_XLMAC_RX_DISAB_DEASSERT_TIME_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_PORTRESET_P1_SIG_DEASSERT_TIMES_0_XLMAC_TX_DISAB_DEASSERT_TIME
 ******************************************************************************/
const ru_field_rec XPORT_PORTRESET_P1_SIG_DEASSERT_TIMES_0_XLMAC_TX_DISAB_DEASSERT_TIME_FIELD =
{
    "XLMAC_TX_DISAB_DEASSERT_TIME",
#if RU_INCLUDE_DESC
    "",
    "...",
#endif
    XPORT_PORTRESET_P1_SIG_DEASSERT_TIMES_0_XLMAC_TX_DISAB_DEASSERT_TIME_FIELD_MASK,
    0,
    XPORT_PORTRESET_P1_SIG_DEASSERT_TIMES_0_XLMAC_TX_DISAB_DEASSERT_TIME_FIELD_WIDTH,
    XPORT_PORTRESET_P1_SIG_DEASSERT_TIMES_0_XLMAC_TX_DISAB_DEASSERT_TIME_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_PORTRESET_P1_SIG_DEASSERT_TIMES_1_XLMAC_TXDISCARD_DEASSERT_TIME
 ******************************************************************************/
const ru_field_rec XPORT_PORTRESET_P1_SIG_DEASSERT_TIMES_1_XLMAC_TXDISCARD_DEASSERT_TIME_FIELD =
{
    "XLMAC_TXDISCARD_DEASSERT_TIME",
#if RU_INCLUDE_DESC
    "",
    "...",
#endif
    XPORT_PORTRESET_P1_SIG_DEASSERT_TIMES_1_XLMAC_TXDISCARD_DEASSERT_TIME_FIELD_MASK,
    0,
    XPORT_PORTRESET_P1_SIG_DEASSERT_TIMES_1_XLMAC_TXDISCARD_DEASSERT_TIME_FIELD_WIDTH,
    XPORT_PORTRESET_P1_SIG_DEASSERT_TIMES_1_XLMAC_TXDISCARD_DEASSERT_TIME_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_PORTRESET_P1_SIG_DEASSERT_TIMES_1_XLMAC_SOFT_RESET_DEASSERT_TIME
 ******************************************************************************/
const ru_field_rec XPORT_PORTRESET_P1_SIG_DEASSERT_TIMES_1_XLMAC_SOFT_RESET_DEASSERT_TIME_FIELD =
{
    "XLMAC_SOFT_RESET_DEASSERT_TIME",
#if RU_INCLUDE_DESC
    "",
    "...",
#endif
    XPORT_PORTRESET_P1_SIG_DEASSERT_TIMES_1_XLMAC_SOFT_RESET_DEASSERT_TIME_FIELD_MASK,
    0,
    XPORT_PORTRESET_P1_SIG_DEASSERT_TIMES_1_XLMAC_SOFT_RESET_DEASSERT_TIME_FIELD_WIDTH,
    XPORT_PORTRESET_P1_SIG_DEASSERT_TIMES_1_XLMAC_SOFT_RESET_DEASSERT_TIME_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_PORTRESET_P1_SIG_DEASSERT_TIMES_2_MAB_RX_PORT_INIT_DEASSERT_TIME
 ******************************************************************************/
const ru_field_rec XPORT_PORTRESET_P1_SIG_DEASSERT_TIMES_2_MAB_RX_PORT_INIT_DEASSERT_TIME_FIELD =
{
    "MAB_RX_PORT_INIT_DEASSERT_TIME",
#if RU_INCLUDE_DESC
    "",
    "...",
#endif
    XPORT_PORTRESET_P1_SIG_DEASSERT_TIMES_2_MAB_RX_PORT_INIT_DEASSERT_TIME_FIELD_MASK,
    0,
    XPORT_PORTRESET_P1_SIG_DEASSERT_TIMES_2_MAB_RX_PORT_INIT_DEASSERT_TIME_FIELD_WIDTH,
    XPORT_PORTRESET_P1_SIG_DEASSERT_TIMES_2_MAB_RX_PORT_INIT_DEASSERT_TIME_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_PORTRESET_P1_SIG_DEASSERT_TIMES_2_MAB_TX_PORT_INIT_DEASSERT_TIME
 ******************************************************************************/
const ru_field_rec XPORT_PORTRESET_P1_SIG_DEASSERT_TIMES_2_MAB_TX_PORT_INIT_DEASSERT_TIME_FIELD =
{
    "MAB_TX_PORT_INIT_DEASSERT_TIME",
#if RU_INCLUDE_DESC
    "",
    "...",
#endif
    XPORT_PORTRESET_P1_SIG_DEASSERT_TIMES_2_MAB_TX_PORT_INIT_DEASSERT_TIME_FIELD_MASK,
    0,
    XPORT_PORTRESET_P1_SIG_DEASSERT_TIMES_2_MAB_TX_PORT_INIT_DEASSERT_TIME_FIELD_WIDTH,
    XPORT_PORTRESET_P1_SIG_DEASSERT_TIMES_2_MAB_TX_PORT_INIT_DEASSERT_TIME_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_PORTRESET_P1_SIG_DEASSERT_TIMES_3_MAB_TX_CREDIT_DISAB_DEASSERT_TIME
 ******************************************************************************/
const ru_field_rec XPORT_PORTRESET_P1_SIG_DEASSERT_TIMES_3_MAB_TX_CREDIT_DISAB_DEASSERT_TIME_FIELD =
{
    "MAB_TX_CREDIT_DISAB_DEASSERT_TIME",
#if RU_INCLUDE_DESC
    "",
    "...",
#endif
    XPORT_PORTRESET_P1_SIG_DEASSERT_TIMES_3_MAB_TX_CREDIT_DISAB_DEASSERT_TIME_FIELD_MASK,
    0,
    XPORT_PORTRESET_P1_SIG_DEASSERT_TIMES_3_MAB_TX_CREDIT_DISAB_DEASSERT_TIME_FIELD_WIDTH,
    XPORT_PORTRESET_P1_SIG_DEASSERT_TIMES_3_MAB_TX_CREDIT_DISAB_DEASSERT_TIME_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_PORTRESET_P1_SIG_DEASSERT_TIMES_3_MAB_TX_FIFO_INIT_DEASSERT_TIME
 ******************************************************************************/
const ru_field_rec XPORT_PORTRESET_P1_SIG_DEASSERT_TIMES_3_MAB_TX_FIFO_INIT_DEASSERT_TIME_FIELD =
{
    "MAB_TX_FIFO_INIT_DEASSERT_TIME",
#if RU_INCLUDE_DESC
    "",
    "...",
#endif
    XPORT_PORTRESET_P1_SIG_DEASSERT_TIMES_3_MAB_TX_FIFO_INIT_DEASSERT_TIME_FIELD_MASK,
    0,
    XPORT_PORTRESET_P1_SIG_DEASSERT_TIMES_3_MAB_TX_FIFO_INIT_DEASSERT_TIME_FIELD_WIDTH,
    XPORT_PORTRESET_P1_SIG_DEASSERT_TIMES_3_MAB_TX_FIFO_INIT_DEASSERT_TIME_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_PORTRESET_P1_SIG_DEASSERT_TIMES_4_PORT_IS_UNDER_RESET_DEASSERT_TIME
 ******************************************************************************/
const ru_field_rec XPORT_PORTRESET_P1_SIG_DEASSERT_TIMES_4_PORT_IS_UNDER_RESET_DEASSERT_TIME_FIELD =
{
    "PORT_IS_UNDER_RESET_DEASSERT_TIME",
#if RU_INCLUDE_DESC
    "",
    "...",
#endif
    XPORT_PORTRESET_P1_SIG_DEASSERT_TIMES_4_PORT_IS_UNDER_RESET_DEASSERT_TIME_FIELD_MASK,
    0,
    XPORT_PORTRESET_P1_SIG_DEASSERT_TIMES_4_PORT_IS_UNDER_RESET_DEASSERT_TIME_FIELD_WIDTH,
    XPORT_PORTRESET_P1_SIG_DEASSERT_TIMES_4_PORT_IS_UNDER_RESET_DEASSERT_TIME_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_PORTRESET_P1_SIG_DEASSERT_TIMES_4_RESERVED0
 ******************************************************************************/
const ru_field_rec XPORT_PORTRESET_P1_SIG_DEASSERT_TIMES_4_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    XPORT_PORTRESET_P1_SIG_DEASSERT_TIMES_4_RESERVED0_FIELD_MASK,
    0,
    XPORT_PORTRESET_P1_SIG_DEASSERT_TIMES_4_RESERVED0_FIELD_WIDTH,
    XPORT_PORTRESET_P1_SIG_DEASSERT_TIMES_4_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_PORTRESET_P2_SIG_EN_RESERVED0
 ******************************************************************************/
const ru_field_rec XPORT_PORTRESET_P2_SIG_EN_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    XPORT_PORTRESET_P2_SIG_EN_RESERVED0_FIELD_MASK,
    0,
    XPORT_PORTRESET_P2_SIG_EN_RESERVED0_FIELD_WIDTH,
    XPORT_PORTRESET_P2_SIG_EN_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_PORTRESET_P2_SIG_EN_ENABLE_XLMAC_RX_DISAB
 ******************************************************************************/
const ru_field_rec XPORT_PORTRESET_P2_SIG_EN_ENABLE_XLMAC_RX_DISAB_FIELD =
{
    "ENABLE_XLMAC_RX_DISAB",
#if RU_INCLUDE_DESC
    "",
    "...",
#endif
    XPORT_PORTRESET_P2_SIG_EN_ENABLE_XLMAC_RX_DISAB_FIELD_MASK,
    0,
    XPORT_PORTRESET_P2_SIG_EN_ENABLE_XLMAC_RX_DISAB_FIELD_WIDTH,
    XPORT_PORTRESET_P2_SIG_EN_ENABLE_XLMAC_RX_DISAB_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_PORTRESET_P2_SIG_EN_ENABLE_XLMAC_TX_DISAB
 ******************************************************************************/
const ru_field_rec XPORT_PORTRESET_P2_SIG_EN_ENABLE_XLMAC_TX_DISAB_FIELD =
{
    "ENABLE_XLMAC_TX_DISAB",
#if RU_INCLUDE_DESC
    "",
    "...",
#endif
    XPORT_PORTRESET_P2_SIG_EN_ENABLE_XLMAC_TX_DISAB_FIELD_MASK,
    0,
    XPORT_PORTRESET_P2_SIG_EN_ENABLE_XLMAC_TX_DISAB_FIELD_WIDTH,
    XPORT_PORTRESET_P2_SIG_EN_ENABLE_XLMAC_TX_DISAB_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_PORTRESET_P2_SIG_EN_ENABLE_XLMAC_TX_DISCARD
 ******************************************************************************/
const ru_field_rec XPORT_PORTRESET_P2_SIG_EN_ENABLE_XLMAC_TX_DISCARD_FIELD =
{
    "ENABLE_XLMAC_TX_DISCARD",
#if RU_INCLUDE_DESC
    "",
    "...",
#endif
    XPORT_PORTRESET_P2_SIG_EN_ENABLE_XLMAC_TX_DISCARD_FIELD_MASK,
    0,
    XPORT_PORTRESET_P2_SIG_EN_ENABLE_XLMAC_TX_DISCARD_FIELD_WIDTH,
    XPORT_PORTRESET_P2_SIG_EN_ENABLE_XLMAC_TX_DISCARD_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_PORTRESET_P2_SIG_EN_ENABLE_XLMAC_SOFT_RESET
 ******************************************************************************/
const ru_field_rec XPORT_PORTRESET_P2_SIG_EN_ENABLE_XLMAC_SOFT_RESET_FIELD =
{
    "ENABLE_XLMAC_SOFT_RESET",
#if RU_INCLUDE_DESC
    "",
    "...",
#endif
    XPORT_PORTRESET_P2_SIG_EN_ENABLE_XLMAC_SOFT_RESET_FIELD_MASK,
    0,
    XPORT_PORTRESET_P2_SIG_EN_ENABLE_XLMAC_SOFT_RESET_FIELD_WIDTH,
    XPORT_PORTRESET_P2_SIG_EN_ENABLE_XLMAC_SOFT_RESET_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_PORTRESET_P2_SIG_EN_ENABLE_MAB_RX_PORT_INIT
 ******************************************************************************/
const ru_field_rec XPORT_PORTRESET_P2_SIG_EN_ENABLE_MAB_RX_PORT_INIT_FIELD =
{
    "ENABLE_MAB_RX_PORT_INIT",
#if RU_INCLUDE_DESC
    "",
    "...",
#endif
    XPORT_PORTRESET_P2_SIG_EN_ENABLE_MAB_RX_PORT_INIT_FIELD_MASK,
    0,
    XPORT_PORTRESET_P2_SIG_EN_ENABLE_MAB_RX_PORT_INIT_FIELD_WIDTH,
    XPORT_PORTRESET_P2_SIG_EN_ENABLE_MAB_RX_PORT_INIT_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_PORTRESET_P2_SIG_EN_ENABLE_MAB_TX_PORT_INIT
 ******************************************************************************/
const ru_field_rec XPORT_PORTRESET_P2_SIG_EN_ENABLE_MAB_TX_PORT_INIT_FIELD =
{
    "ENABLE_MAB_TX_PORT_INIT",
#if RU_INCLUDE_DESC
    "",
    "...",
#endif
    XPORT_PORTRESET_P2_SIG_EN_ENABLE_MAB_TX_PORT_INIT_FIELD_MASK,
    0,
    XPORT_PORTRESET_P2_SIG_EN_ENABLE_MAB_TX_PORT_INIT_FIELD_WIDTH,
    XPORT_PORTRESET_P2_SIG_EN_ENABLE_MAB_TX_PORT_INIT_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_PORTRESET_P2_SIG_EN_ENABLE_MAB_TX_CREDIT_DISAB
 ******************************************************************************/
const ru_field_rec XPORT_PORTRESET_P2_SIG_EN_ENABLE_MAB_TX_CREDIT_DISAB_FIELD =
{
    "ENABLE_MAB_TX_CREDIT_DISAB",
#if RU_INCLUDE_DESC
    "",
    "...",
#endif
    XPORT_PORTRESET_P2_SIG_EN_ENABLE_MAB_TX_CREDIT_DISAB_FIELD_MASK,
    0,
    XPORT_PORTRESET_P2_SIG_EN_ENABLE_MAB_TX_CREDIT_DISAB_FIELD_WIDTH,
    XPORT_PORTRESET_P2_SIG_EN_ENABLE_MAB_TX_CREDIT_DISAB_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_PORTRESET_P2_SIG_EN_ENABLE_MAB_TX_FIFO_INIT
 ******************************************************************************/
const ru_field_rec XPORT_PORTRESET_P2_SIG_EN_ENABLE_MAB_TX_FIFO_INIT_FIELD =
{
    "ENABLE_MAB_TX_FIFO_INIT",
#if RU_INCLUDE_DESC
    "",
    "...",
#endif
    XPORT_PORTRESET_P2_SIG_EN_ENABLE_MAB_TX_FIFO_INIT_FIELD_MASK,
    0,
    XPORT_PORTRESET_P2_SIG_EN_ENABLE_MAB_TX_FIFO_INIT_FIELD_WIDTH,
    XPORT_PORTRESET_P2_SIG_EN_ENABLE_MAB_TX_FIFO_INIT_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_PORTRESET_P2_SIG_EN_ENABLE_PORT_IS_UNDER_RESET
 ******************************************************************************/
const ru_field_rec XPORT_PORTRESET_P2_SIG_EN_ENABLE_PORT_IS_UNDER_RESET_FIELD =
{
    "ENABLE_PORT_IS_UNDER_RESET",
#if RU_INCLUDE_DESC
    "",
    "...",
#endif
    XPORT_PORTRESET_P2_SIG_EN_ENABLE_PORT_IS_UNDER_RESET_FIELD_MASK,
    0,
    XPORT_PORTRESET_P2_SIG_EN_ENABLE_PORT_IS_UNDER_RESET_FIELD_WIDTH,
    XPORT_PORTRESET_P2_SIG_EN_ENABLE_PORT_IS_UNDER_RESET_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_PORTRESET_P2_SIG_EN_ENABLE_XLMAC_EP_DISCARD
 ******************************************************************************/
const ru_field_rec XPORT_PORTRESET_P2_SIG_EN_ENABLE_XLMAC_EP_DISCARD_FIELD =
{
    "ENABLE_XLMAC_EP_DISCARD",
#if RU_INCLUDE_DESC
    "",
    "...",
#endif
    XPORT_PORTRESET_P2_SIG_EN_ENABLE_XLMAC_EP_DISCARD_FIELD_MASK,
    0,
    XPORT_PORTRESET_P2_SIG_EN_ENABLE_XLMAC_EP_DISCARD_FIELD_WIDTH,
    XPORT_PORTRESET_P2_SIG_EN_ENABLE_XLMAC_EP_DISCARD_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_PORTRESET_P2_SIG_ASSERT_TIMES_0_XLMAC_RX_DISAB_ASSERT_TIME
 ******************************************************************************/
const ru_field_rec XPORT_PORTRESET_P2_SIG_ASSERT_TIMES_0_XLMAC_RX_DISAB_ASSERT_TIME_FIELD =
{
    "XLMAC_RX_DISAB_ASSERT_TIME",
#if RU_INCLUDE_DESC
    "",
    "...",
#endif
    XPORT_PORTRESET_P2_SIG_ASSERT_TIMES_0_XLMAC_RX_DISAB_ASSERT_TIME_FIELD_MASK,
    0,
    XPORT_PORTRESET_P2_SIG_ASSERT_TIMES_0_XLMAC_RX_DISAB_ASSERT_TIME_FIELD_WIDTH,
    XPORT_PORTRESET_P2_SIG_ASSERT_TIMES_0_XLMAC_RX_DISAB_ASSERT_TIME_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_PORTRESET_P2_SIG_ASSERT_TIMES_0_XLMAC_TX_DISAB_ASSERT_TIME
 ******************************************************************************/
const ru_field_rec XPORT_PORTRESET_P2_SIG_ASSERT_TIMES_0_XLMAC_TX_DISAB_ASSERT_TIME_FIELD =
{
    "XLMAC_TX_DISAB_ASSERT_TIME",
#if RU_INCLUDE_DESC
    "",
    "...",
#endif
    XPORT_PORTRESET_P2_SIG_ASSERT_TIMES_0_XLMAC_TX_DISAB_ASSERT_TIME_FIELD_MASK,
    0,
    XPORT_PORTRESET_P2_SIG_ASSERT_TIMES_0_XLMAC_TX_DISAB_ASSERT_TIME_FIELD_WIDTH,
    XPORT_PORTRESET_P2_SIG_ASSERT_TIMES_0_XLMAC_TX_DISAB_ASSERT_TIME_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_PORTRESET_P2_SIG_ASSERT_TIMES_1_XLMAC_TXDISCARD_ASSERT_TIME
 ******************************************************************************/
const ru_field_rec XPORT_PORTRESET_P2_SIG_ASSERT_TIMES_1_XLMAC_TXDISCARD_ASSERT_TIME_FIELD =
{
    "XLMAC_TXDISCARD_ASSERT_TIME",
#if RU_INCLUDE_DESC
    "",
    "...",
#endif
    XPORT_PORTRESET_P2_SIG_ASSERT_TIMES_1_XLMAC_TXDISCARD_ASSERT_TIME_FIELD_MASK,
    0,
    XPORT_PORTRESET_P2_SIG_ASSERT_TIMES_1_XLMAC_TXDISCARD_ASSERT_TIME_FIELD_WIDTH,
    XPORT_PORTRESET_P2_SIG_ASSERT_TIMES_1_XLMAC_TXDISCARD_ASSERT_TIME_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_PORTRESET_P2_SIG_ASSERT_TIMES_1_XLMAC_SOFT_RESET_ASSERT_TIME
 ******************************************************************************/
const ru_field_rec XPORT_PORTRESET_P2_SIG_ASSERT_TIMES_1_XLMAC_SOFT_RESET_ASSERT_TIME_FIELD =
{
    "XLMAC_SOFT_RESET_ASSERT_TIME",
#if RU_INCLUDE_DESC
    "",
    "...",
#endif
    XPORT_PORTRESET_P2_SIG_ASSERT_TIMES_1_XLMAC_SOFT_RESET_ASSERT_TIME_FIELD_MASK,
    0,
    XPORT_PORTRESET_P2_SIG_ASSERT_TIMES_1_XLMAC_SOFT_RESET_ASSERT_TIME_FIELD_WIDTH,
    XPORT_PORTRESET_P2_SIG_ASSERT_TIMES_1_XLMAC_SOFT_RESET_ASSERT_TIME_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_PORTRESET_P2_SIG_ASSERT_TIMES_2_MAB_RX_PORT_INIT_ASSERT_TIME
 ******************************************************************************/
const ru_field_rec XPORT_PORTRESET_P2_SIG_ASSERT_TIMES_2_MAB_RX_PORT_INIT_ASSERT_TIME_FIELD =
{
    "MAB_RX_PORT_INIT_ASSERT_TIME",
#if RU_INCLUDE_DESC
    "",
    "...",
#endif
    XPORT_PORTRESET_P2_SIG_ASSERT_TIMES_2_MAB_RX_PORT_INIT_ASSERT_TIME_FIELD_MASK,
    0,
    XPORT_PORTRESET_P2_SIG_ASSERT_TIMES_2_MAB_RX_PORT_INIT_ASSERT_TIME_FIELD_WIDTH,
    XPORT_PORTRESET_P2_SIG_ASSERT_TIMES_2_MAB_RX_PORT_INIT_ASSERT_TIME_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_PORTRESET_P2_SIG_ASSERT_TIMES_2_MAB_TX_PORT_INIT_ASSERT_TIME
 ******************************************************************************/
const ru_field_rec XPORT_PORTRESET_P2_SIG_ASSERT_TIMES_2_MAB_TX_PORT_INIT_ASSERT_TIME_FIELD =
{
    "MAB_TX_PORT_INIT_ASSERT_TIME",
#if RU_INCLUDE_DESC
    "",
    "...",
#endif
    XPORT_PORTRESET_P2_SIG_ASSERT_TIMES_2_MAB_TX_PORT_INIT_ASSERT_TIME_FIELD_MASK,
    0,
    XPORT_PORTRESET_P2_SIG_ASSERT_TIMES_2_MAB_TX_PORT_INIT_ASSERT_TIME_FIELD_WIDTH,
    XPORT_PORTRESET_P2_SIG_ASSERT_TIMES_2_MAB_TX_PORT_INIT_ASSERT_TIME_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_PORTRESET_P2_SIG_ASSERT_TIMES_3_MAB_TX_CREDIT_DISAB_ASSERT_TIME
 ******************************************************************************/
const ru_field_rec XPORT_PORTRESET_P2_SIG_ASSERT_TIMES_3_MAB_TX_CREDIT_DISAB_ASSERT_TIME_FIELD =
{
    "MAB_TX_CREDIT_DISAB_ASSERT_TIME",
#if RU_INCLUDE_DESC
    "",
    "...",
#endif
    XPORT_PORTRESET_P2_SIG_ASSERT_TIMES_3_MAB_TX_CREDIT_DISAB_ASSERT_TIME_FIELD_MASK,
    0,
    XPORT_PORTRESET_P2_SIG_ASSERT_TIMES_3_MAB_TX_CREDIT_DISAB_ASSERT_TIME_FIELD_WIDTH,
    XPORT_PORTRESET_P2_SIG_ASSERT_TIMES_3_MAB_TX_CREDIT_DISAB_ASSERT_TIME_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_PORTRESET_P2_SIG_ASSERT_TIMES_3_MAB_TX_FIFO_INIT_ASSERT_TIME
 ******************************************************************************/
const ru_field_rec XPORT_PORTRESET_P2_SIG_ASSERT_TIMES_3_MAB_TX_FIFO_INIT_ASSERT_TIME_FIELD =
{
    "MAB_TX_FIFO_INIT_ASSERT_TIME",
#if RU_INCLUDE_DESC
    "",
    "...",
#endif
    XPORT_PORTRESET_P2_SIG_ASSERT_TIMES_3_MAB_TX_FIFO_INIT_ASSERT_TIME_FIELD_MASK,
    0,
    XPORT_PORTRESET_P2_SIG_ASSERT_TIMES_3_MAB_TX_FIFO_INIT_ASSERT_TIME_FIELD_WIDTH,
    XPORT_PORTRESET_P2_SIG_ASSERT_TIMES_3_MAB_TX_FIFO_INIT_ASSERT_TIME_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_PORTRESET_P2_SIG_ASSERT_TIMES_4_PORT_IS_UNDER_RESET_ASSERT_TIME
 ******************************************************************************/
const ru_field_rec XPORT_PORTRESET_P2_SIG_ASSERT_TIMES_4_PORT_IS_UNDER_RESET_ASSERT_TIME_FIELD =
{
    "PORT_IS_UNDER_RESET_ASSERT_TIME",
#if RU_INCLUDE_DESC
    "",
    "...",
#endif
    XPORT_PORTRESET_P2_SIG_ASSERT_TIMES_4_PORT_IS_UNDER_RESET_ASSERT_TIME_FIELD_MASK,
    0,
    XPORT_PORTRESET_P2_SIG_ASSERT_TIMES_4_PORT_IS_UNDER_RESET_ASSERT_TIME_FIELD_WIDTH,
    XPORT_PORTRESET_P2_SIG_ASSERT_TIMES_4_PORT_IS_UNDER_RESET_ASSERT_TIME_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_PORTRESET_P2_SIG_ASSERT_TIMES_4_RESERVED0
 ******************************************************************************/
const ru_field_rec XPORT_PORTRESET_P2_SIG_ASSERT_TIMES_4_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    XPORT_PORTRESET_P2_SIG_ASSERT_TIMES_4_RESERVED0_FIELD_MASK,
    0,
    XPORT_PORTRESET_P2_SIG_ASSERT_TIMES_4_RESERVED0_FIELD_WIDTH,
    XPORT_PORTRESET_P2_SIG_ASSERT_TIMES_4_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_PORTRESET_P2_SIG_DEASSERT_TIMES_0_XLMAC_RX_DISAB_DEASSERT_TIME
 ******************************************************************************/
const ru_field_rec XPORT_PORTRESET_P2_SIG_DEASSERT_TIMES_0_XLMAC_RX_DISAB_DEASSERT_TIME_FIELD =
{
    "XLMAC_RX_DISAB_DEASSERT_TIME",
#if RU_INCLUDE_DESC
    "",
    "...",
#endif
    XPORT_PORTRESET_P2_SIG_DEASSERT_TIMES_0_XLMAC_RX_DISAB_DEASSERT_TIME_FIELD_MASK,
    0,
    XPORT_PORTRESET_P2_SIG_DEASSERT_TIMES_0_XLMAC_RX_DISAB_DEASSERT_TIME_FIELD_WIDTH,
    XPORT_PORTRESET_P2_SIG_DEASSERT_TIMES_0_XLMAC_RX_DISAB_DEASSERT_TIME_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_PORTRESET_P2_SIG_DEASSERT_TIMES_0_XLMAC_TX_DISAB_DEASSERT_TIME
 ******************************************************************************/
const ru_field_rec XPORT_PORTRESET_P2_SIG_DEASSERT_TIMES_0_XLMAC_TX_DISAB_DEASSERT_TIME_FIELD =
{
    "XLMAC_TX_DISAB_DEASSERT_TIME",
#if RU_INCLUDE_DESC
    "",
    "...",
#endif
    XPORT_PORTRESET_P2_SIG_DEASSERT_TIMES_0_XLMAC_TX_DISAB_DEASSERT_TIME_FIELD_MASK,
    0,
    XPORT_PORTRESET_P2_SIG_DEASSERT_TIMES_0_XLMAC_TX_DISAB_DEASSERT_TIME_FIELD_WIDTH,
    XPORT_PORTRESET_P2_SIG_DEASSERT_TIMES_0_XLMAC_TX_DISAB_DEASSERT_TIME_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_PORTRESET_P2_SIG_DEASSERT_TIMES_1_XLMAC_TXDISCARD_DEASSERT_TIME
 ******************************************************************************/
const ru_field_rec XPORT_PORTRESET_P2_SIG_DEASSERT_TIMES_1_XLMAC_TXDISCARD_DEASSERT_TIME_FIELD =
{
    "XLMAC_TXDISCARD_DEASSERT_TIME",
#if RU_INCLUDE_DESC
    "",
    "...",
#endif
    XPORT_PORTRESET_P2_SIG_DEASSERT_TIMES_1_XLMAC_TXDISCARD_DEASSERT_TIME_FIELD_MASK,
    0,
    XPORT_PORTRESET_P2_SIG_DEASSERT_TIMES_1_XLMAC_TXDISCARD_DEASSERT_TIME_FIELD_WIDTH,
    XPORT_PORTRESET_P2_SIG_DEASSERT_TIMES_1_XLMAC_TXDISCARD_DEASSERT_TIME_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_PORTRESET_P2_SIG_DEASSERT_TIMES_1_XLMAC_SOFT_RESET_DEASSERT_TIME
 ******************************************************************************/
const ru_field_rec XPORT_PORTRESET_P2_SIG_DEASSERT_TIMES_1_XLMAC_SOFT_RESET_DEASSERT_TIME_FIELD =
{
    "XLMAC_SOFT_RESET_DEASSERT_TIME",
#if RU_INCLUDE_DESC
    "",
    "...",
#endif
    XPORT_PORTRESET_P2_SIG_DEASSERT_TIMES_1_XLMAC_SOFT_RESET_DEASSERT_TIME_FIELD_MASK,
    0,
    XPORT_PORTRESET_P2_SIG_DEASSERT_TIMES_1_XLMAC_SOFT_RESET_DEASSERT_TIME_FIELD_WIDTH,
    XPORT_PORTRESET_P2_SIG_DEASSERT_TIMES_1_XLMAC_SOFT_RESET_DEASSERT_TIME_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_PORTRESET_P2_SIG_DEASSERT_TIMES_2_MAB_RX_PORT_INIT_DEASSERT_TIME
 ******************************************************************************/
const ru_field_rec XPORT_PORTRESET_P2_SIG_DEASSERT_TIMES_2_MAB_RX_PORT_INIT_DEASSERT_TIME_FIELD =
{
    "MAB_RX_PORT_INIT_DEASSERT_TIME",
#if RU_INCLUDE_DESC
    "",
    "...",
#endif
    XPORT_PORTRESET_P2_SIG_DEASSERT_TIMES_2_MAB_RX_PORT_INIT_DEASSERT_TIME_FIELD_MASK,
    0,
    XPORT_PORTRESET_P2_SIG_DEASSERT_TIMES_2_MAB_RX_PORT_INIT_DEASSERT_TIME_FIELD_WIDTH,
    XPORT_PORTRESET_P2_SIG_DEASSERT_TIMES_2_MAB_RX_PORT_INIT_DEASSERT_TIME_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_PORTRESET_P2_SIG_DEASSERT_TIMES_2_MAB_TX_PORT_INIT_DEASSERT_TIME
 ******************************************************************************/
const ru_field_rec XPORT_PORTRESET_P2_SIG_DEASSERT_TIMES_2_MAB_TX_PORT_INIT_DEASSERT_TIME_FIELD =
{
    "MAB_TX_PORT_INIT_DEASSERT_TIME",
#if RU_INCLUDE_DESC
    "",
    "...",
#endif
    XPORT_PORTRESET_P2_SIG_DEASSERT_TIMES_2_MAB_TX_PORT_INIT_DEASSERT_TIME_FIELD_MASK,
    0,
    XPORT_PORTRESET_P2_SIG_DEASSERT_TIMES_2_MAB_TX_PORT_INIT_DEASSERT_TIME_FIELD_WIDTH,
    XPORT_PORTRESET_P2_SIG_DEASSERT_TIMES_2_MAB_TX_PORT_INIT_DEASSERT_TIME_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_PORTRESET_P2_SIG_DEASSERT_TIMES_3_MAB_TX_CREDIT_DISAB_DEASSERT_TIME
 ******************************************************************************/
const ru_field_rec XPORT_PORTRESET_P2_SIG_DEASSERT_TIMES_3_MAB_TX_CREDIT_DISAB_DEASSERT_TIME_FIELD =
{
    "MAB_TX_CREDIT_DISAB_DEASSERT_TIME",
#if RU_INCLUDE_DESC
    "",
    "...",
#endif
    XPORT_PORTRESET_P2_SIG_DEASSERT_TIMES_3_MAB_TX_CREDIT_DISAB_DEASSERT_TIME_FIELD_MASK,
    0,
    XPORT_PORTRESET_P2_SIG_DEASSERT_TIMES_3_MAB_TX_CREDIT_DISAB_DEASSERT_TIME_FIELD_WIDTH,
    XPORT_PORTRESET_P2_SIG_DEASSERT_TIMES_3_MAB_TX_CREDIT_DISAB_DEASSERT_TIME_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_PORTRESET_P2_SIG_DEASSERT_TIMES_3_MAB_TX_FIFO_INIT_DEASSERT_TIME
 ******************************************************************************/
const ru_field_rec XPORT_PORTRESET_P2_SIG_DEASSERT_TIMES_3_MAB_TX_FIFO_INIT_DEASSERT_TIME_FIELD =
{
    "MAB_TX_FIFO_INIT_DEASSERT_TIME",
#if RU_INCLUDE_DESC
    "",
    "...",
#endif
    XPORT_PORTRESET_P2_SIG_DEASSERT_TIMES_3_MAB_TX_FIFO_INIT_DEASSERT_TIME_FIELD_MASK,
    0,
    XPORT_PORTRESET_P2_SIG_DEASSERT_TIMES_3_MAB_TX_FIFO_INIT_DEASSERT_TIME_FIELD_WIDTH,
    XPORT_PORTRESET_P2_SIG_DEASSERT_TIMES_3_MAB_TX_FIFO_INIT_DEASSERT_TIME_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_PORTRESET_P2_SIG_DEASSERT_TIMES_4_PORT_IS_UNDER_RESET_DEASSERT_TIME
 ******************************************************************************/
const ru_field_rec XPORT_PORTRESET_P2_SIG_DEASSERT_TIMES_4_PORT_IS_UNDER_RESET_DEASSERT_TIME_FIELD =
{
    "PORT_IS_UNDER_RESET_DEASSERT_TIME",
#if RU_INCLUDE_DESC
    "",
    "...",
#endif
    XPORT_PORTRESET_P2_SIG_DEASSERT_TIMES_4_PORT_IS_UNDER_RESET_DEASSERT_TIME_FIELD_MASK,
    0,
    XPORT_PORTRESET_P2_SIG_DEASSERT_TIMES_4_PORT_IS_UNDER_RESET_DEASSERT_TIME_FIELD_WIDTH,
    XPORT_PORTRESET_P2_SIG_DEASSERT_TIMES_4_PORT_IS_UNDER_RESET_DEASSERT_TIME_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_PORTRESET_P2_SIG_DEASSERT_TIMES_4_RESERVED0
 ******************************************************************************/
const ru_field_rec XPORT_PORTRESET_P2_SIG_DEASSERT_TIMES_4_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    XPORT_PORTRESET_P2_SIG_DEASSERT_TIMES_4_RESERVED0_FIELD_MASK,
    0,
    XPORT_PORTRESET_P2_SIG_DEASSERT_TIMES_4_RESERVED0_FIELD_WIDTH,
    XPORT_PORTRESET_P2_SIG_DEASSERT_TIMES_4_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_PORTRESET_P3_SIG_EN_RESERVED0
 ******************************************************************************/
const ru_field_rec XPORT_PORTRESET_P3_SIG_EN_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    XPORT_PORTRESET_P3_SIG_EN_RESERVED0_FIELD_MASK,
    0,
    XPORT_PORTRESET_P3_SIG_EN_RESERVED0_FIELD_WIDTH,
    XPORT_PORTRESET_P3_SIG_EN_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_PORTRESET_P3_SIG_EN_ENABLE_XLMAC_RX_DISAB
 ******************************************************************************/
const ru_field_rec XPORT_PORTRESET_P3_SIG_EN_ENABLE_XLMAC_RX_DISAB_FIELD =
{
    "ENABLE_XLMAC_RX_DISAB",
#if RU_INCLUDE_DESC
    "",
    "...",
#endif
    XPORT_PORTRESET_P3_SIG_EN_ENABLE_XLMAC_RX_DISAB_FIELD_MASK,
    0,
    XPORT_PORTRESET_P3_SIG_EN_ENABLE_XLMAC_RX_DISAB_FIELD_WIDTH,
    XPORT_PORTRESET_P3_SIG_EN_ENABLE_XLMAC_RX_DISAB_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_PORTRESET_P3_SIG_EN_ENABLE_XLMAC_TX_DISAB
 ******************************************************************************/
const ru_field_rec XPORT_PORTRESET_P3_SIG_EN_ENABLE_XLMAC_TX_DISAB_FIELD =
{
    "ENABLE_XLMAC_TX_DISAB",
#if RU_INCLUDE_DESC
    "",
    "...",
#endif
    XPORT_PORTRESET_P3_SIG_EN_ENABLE_XLMAC_TX_DISAB_FIELD_MASK,
    0,
    XPORT_PORTRESET_P3_SIG_EN_ENABLE_XLMAC_TX_DISAB_FIELD_WIDTH,
    XPORT_PORTRESET_P3_SIG_EN_ENABLE_XLMAC_TX_DISAB_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_PORTRESET_P3_SIG_EN_ENABLE_XLMAC_TX_DISCARD
 ******************************************************************************/
const ru_field_rec XPORT_PORTRESET_P3_SIG_EN_ENABLE_XLMAC_TX_DISCARD_FIELD =
{
    "ENABLE_XLMAC_TX_DISCARD",
#if RU_INCLUDE_DESC
    "",
    "...",
#endif
    XPORT_PORTRESET_P3_SIG_EN_ENABLE_XLMAC_TX_DISCARD_FIELD_MASK,
    0,
    XPORT_PORTRESET_P3_SIG_EN_ENABLE_XLMAC_TX_DISCARD_FIELD_WIDTH,
    XPORT_PORTRESET_P3_SIG_EN_ENABLE_XLMAC_TX_DISCARD_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_PORTRESET_P3_SIG_EN_ENABLE_XLMAC_SOFT_RESET
 ******************************************************************************/
const ru_field_rec XPORT_PORTRESET_P3_SIG_EN_ENABLE_XLMAC_SOFT_RESET_FIELD =
{
    "ENABLE_XLMAC_SOFT_RESET",
#if RU_INCLUDE_DESC
    "",
    "...",
#endif
    XPORT_PORTRESET_P3_SIG_EN_ENABLE_XLMAC_SOFT_RESET_FIELD_MASK,
    0,
    XPORT_PORTRESET_P3_SIG_EN_ENABLE_XLMAC_SOFT_RESET_FIELD_WIDTH,
    XPORT_PORTRESET_P3_SIG_EN_ENABLE_XLMAC_SOFT_RESET_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_PORTRESET_P3_SIG_EN_ENABLE_MAB_RX_PORT_INIT
 ******************************************************************************/
const ru_field_rec XPORT_PORTRESET_P3_SIG_EN_ENABLE_MAB_RX_PORT_INIT_FIELD =
{
    "ENABLE_MAB_RX_PORT_INIT",
#if RU_INCLUDE_DESC
    "",
    "...",
#endif
    XPORT_PORTRESET_P3_SIG_EN_ENABLE_MAB_RX_PORT_INIT_FIELD_MASK,
    0,
    XPORT_PORTRESET_P3_SIG_EN_ENABLE_MAB_RX_PORT_INIT_FIELD_WIDTH,
    XPORT_PORTRESET_P3_SIG_EN_ENABLE_MAB_RX_PORT_INIT_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_PORTRESET_P3_SIG_EN_ENABLE_MAB_TX_PORT_INIT
 ******************************************************************************/
const ru_field_rec XPORT_PORTRESET_P3_SIG_EN_ENABLE_MAB_TX_PORT_INIT_FIELD =
{
    "ENABLE_MAB_TX_PORT_INIT",
#if RU_INCLUDE_DESC
    "",
    "...",
#endif
    XPORT_PORTRESET_P3_SIG_EN_ENABLE_MAB_TX_PORT_INIT_FIELD_MASK,
    0,
    XPORT_PORTRESET_P3_SIG_EN_ENABLE_MAB_TX_PORT_INIT_FIELD_WIDTH,
    XPORT_PORTRESET_P3_SIG_EN_ENABLE_MAB_TX_PORT_INIT_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_PORTRESET_P3_SIG_EN_ENABLE_MAB_TX_CREDIT_DISAB
 ******************************************************************************/
const ru_field_rec XPORT_PORTRESET_P3_SIG_EN_ENABLE_MAB_TX_CREDIT_DISAB_FIELD =
{
    "ENABLE_MAB_TX_CREDIT_DISAB",
#if RU_INCLUDE_DESC
    "",
    "...",
#endif
    XPORT_PORTRESET_P3_SIG_EN_ENABLE_MAB_TX_CREDIT_DISAB_FIELD_MASK,
    0,
    XPORT_PORTRESET_P3_SIG_EN_ENABLE_MAB_TX_CREDIT_DISAB_FIELD_WIDTH,
    XPORT_PORTRESET_P3_SIG_EN_ENABLE_MAB_TX_CREDIT_DISAB_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_PORTRESET_P3_SIG_EN_ENABLE_MAB_TX_FIFO_INIT
 ******************************************************************************/
const ru_field_rec XPORT_PORTRESET_P3_SIG_EN_ENABLE_MAB_TX_FIFO_INIT_FIELD =
{
    "ENABLE_MAB_TX_FIFO_INIT",
#if RU_INCLUDE_DESC
    "",
    "...",
#endif
    XPORT_PORTRESET_P3_SIG_EN_ENABLE_MAB_TX_FIFO_INIT_FIELD_MASK,
    0,
    XPORT_PORTRESET_P3_SIG_EN_ENABLE_MAB_TX_FIFO_INIT_FIELD_WIDTH,
    XPORT_PORTRESET_P3_SIG_EN_ENABLE_MAB_TX_FIFO_INIT_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_PORTRESET_P3_SIG_EN_ENABLE_PORT_IS_UNDER_RESET
 ******************************************************************************/
const ru_field_rec XPORT_PORTRESET_P3_SIG_EN_ENABLE_PORT_IS_UNDER_RESET_FIELD =
{
    "ENABLE_PORT_IS_UNDER_RESET",
#if RU_INCLUDE_DESC
    "",
    "...",
#endif
    XPORT_PORTRESET_P3_SIG_EN_ENABLE_PORT_IS_UNDER_RESET_FIELD_MASK,
    0,
    XPORT_PORTRESET_P3_SIG_EN_ENABLE_PORT_IS_UNDER_RESET_FIELD_WIDTH,
    XPORT_PORTRESET_P3_SIG_EN_ENABLE_PORT_IS_UNDER_RESET_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_PORTRESET_P3_SIG_EN_ENABLE_XLMAC_EP_DISCARD
 ******************************************************************************/
const ru_field_rec XPORT_PORTRESET_P3_SIG_EN_ENABLE_XLMAC_EP_DISCARD_FIELD =
{
    "ENABLE_XLMAC_EP_DISCARD",
#if RU_INCLUDE_DESC
    "",
    "...",
#endif
    XPORT_PORTRESET_P3_SIG_EN_ENABLE_XLMAC_EP_DISCARD_FIELD_MASK,
    0,
    XPORT_PORTRESET_P3_SIG_EN_ENABLE_XLMAC_EP_DISCARD_FIELD_WIDTH,
    XPORT_PORTRESET_P3_SIG_EN_ENABLE_XLMAC_EP_DISCARD_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_PORTRESET_P3_SIG_ASSERT_TIMES_0_XLMAC_RX_DISAB_ASSERT_TIME
 ******************************************************************************/
const ru_field_rec XPORT_PORTRESET_P3_SIG_ASSERT_TIMES_0_XLMAC_RX_DISAB_ASSERT_TIME_FIELD =
{
    "XLMAC_RX_DISAB_ASSERT_TIME",
#if RU_INCLUDE_DESC
    "",
    "...",
#endif
    XPORT_PORTRESET_P3_SIG_ASSERT_TIMES_0_XLMAC_RX_DISAB_ASSERT_TIME_FIELD_MASK,
    0,
    XPORT_PORTRESET_P3_SIG_ASSERT_TIMES_0_XLMAC_RX_DISAB_ASSERT_TIME_FIELD_WIDTH,
    XPORT_PORTRESET_P3_SIG_ASSERT_TIMES_0_XLMAC_RX_DISAB_ASSERT_TIME_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_PORTRESET_P3_SIG_ASSERT_TIMES_0_XLMAC_TX_DISAB_ASSERT_TIME
 ******************************************************************************/
const ru_field_rec XPORT_PORTRESET_P3_SIG_ASSERT_TIMES_0_XLMAC_TX_DISAB_ASSERT_TIME_FIELD =
{
    "XLMAC_TX_DISAB_ASSERT_TIME",
#if RU_INCLUDE_DESC
    "",
    "...",
#endif
    XPORT_PORTRESET_P3_SIG_ASSERT_TIMES_0_XLMAC_TX_DISAB_ASSERT_TIME_FIELD_MASK,
    0,
    XPORT_PORTRESET_P3_SIG_ASSERT_TIMES_0_XLMAC_TX_DISAB_ASSERT_TIME_FIELD_WIDTH,
    XPORT_PORTRESET_P3_SIG_ASSERT_TIMES_0_XLMAC_TX_DISAB_ASSERT_TIME_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_PORTRESET_P3_SIG_ASSERT_TIMES_1_XLMAC_TXDISCARD_ASSERT_TIME
 ******************************************************************************/
const ru_field_rec XPORT_PORTRESET_P3_SIG_ASSERT_TIMES_1_XLMAC_TXDISCARD_ASSERT_TIME_FIELD =
{
    "XLMAC_TXDISCARD_ASSERT_TIME",
#if RU_INCLUDE_DESC
    "",
    "...",
#endif
    XPORT_PORTRESET_P3_SIG_ASSERT_TIMES_1_XLMAC_TXDISCARD_ASSERT_TIME_FIELD_MASK,
    0,
    XPORT_PORTRESET_P3_SIG_ASSERT_TIMES_1_XLMAC_TXDISCARD_ASSERT_TIME_FIELD_WIDTH,
    XPORT_PORTRESET_P3_SIG_ASSERT_TIMES_1_XLMAC_TXDISCARD_ASSERT_TIME_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_PORTRESET_P3_SIG_ASSERT_TIMES_1_XLMAC_SOFT_RESET_ASSERT_TIME
 ******************************************************************************/
const ru_field_rec XPORT_PORTRESET_P3_SIG_ASSERT_TIMES_1_XLMAC_SOFT_RESET_ASSERT_TIME_FIELD =
{
    "XLMAC_SOFT_RESET_ASSERT_TIME",
#if RU_INCLUDE_DESC
    "",
    "...",
#endif
    XPORT_PORTRESET_P3_SIG_ASSERT_TIMES_1_XLMAC_SOFT_RESET_ASSERT_TIME_FIELD_MASK,
    0,
    XPORT_PORTRESET_P3_SIG_ASSERT_TIMES_1_XLMAC_SOFT_RESET_ASSERT_TIME_FIELD_WIDTH,
    XPORT_PORTRESET_P3_SIG_ASSERT_TIMES_1_XLMAC_SOFT_RESET_ASSERT_TIME_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_PORTRESET_P3_SIG_ASSERT_TIMES_2_MAB_RX_PORT_INIT_ASSERT_TIME
 ******************************************************************************/
const ru_field_rec XPORT_PORTRESET_P3_SIG_ASSERT_TIMES_2_MAB_RX_PORT_INIT_ASSERT_TIME_FIELD =
{
    "MAB_RX_PORT_INIT_ASSERT_TIME",
#if RU_INCLUDE_DESC
    "",
    "...",
#endif
    XPORT_PORTRESET_P3_SIG_ASSERT_TIMES_2_MAB_RX_PORT_INIT_ASSERT_TIME_FIELD_MASK,
    0,
    XPORT_PORTRESET_P3_SIG_ASSERT_TIMES_2_MAB_RX_PORT_INIT_ASSERT_TIME_FIELD_WIDTH,
    XPORT_PORTRESET_P3_SIG_ASSERT_TIMES_2_MAB_RX_PORT_INIT_ASSERT_TIME_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_PORTRESET_P3_SIG_ASSERT_TIMES_2_MAB_TX_PORT_INIT_ASSERT_TIME
 ******************************************************************************/
const ru_field_rec XPORT_PORTRESET_P3_SIG_ASSERT_TIMES_2_MAB_TX_PORT_INIT_ASSERT_TIME_FIELD =
{
    "MAB_TX_PORT_INIT_ASSERT_TIME",
#if RU_INCLUDE_DESC
    "",
    "...",
#endif
    XPORT_PORTRESET_P3_SIG_ASSERT_TIMES_2_MAB_TX_PORT_INIT_ASSERT_TIME_FIELD_MASK,
    0,
    XPORT_PORTRESET_P3_SIG_ASSERT_TIMES_2_MAB_TX_PORT_INIT_ASSERT_TIME_FIELD_WIDTH,
    XPORT_PORTRESET_P3_SIG_ASSERT_TIMES_2_MAB_TX_PORT_INIT_ASSERT_TIME_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_PORTRESET_P3_SIG_ASSERT_TIMES_3_MAB_TX_CREDIT_DISAB_ASSERT_TIME
 ******************************************************************************/
const ru_field_rec XPORT_PORTRESET_P3_SIG_ASSERT_TIMES_3_MAB_TX_CREDIT_DISAB_ASSERT_TIME_FIELD =
{
    "MAB_TX_CREDIT_DISAB_ASSERT_TIME",
#if RU_INCLUDE_DESC
    "",
    "...",
#endif
    XPORT_PORTRESET_P3_SIG_ASSERT_TIMES_3_MAB_TX_CREDIT_DISAB_ASSERT_TIME_FIELD_MASK,
    0,
    XPORT_PORTRESET_P3_SIG_ASSERT_TIMES_3_MAB_TX_CREDIT_DISAB_ASSERT_TIME_FIELD_WIDTH,
    XPORT_PORTRESET_P3_SIG_ASSERT_TIMES_3_MAB_TX_CREDIT_DISAB_ASSERT_TIME_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_PORTRESET_P3_SIG_ASSERT_TIMES_3_MAB_TX_FIFO_INIT_ASSERT_TIME
 ******************************************************************************/
const ru_field_rec XPORT_PORTRESET_P3_SIG_ASSERT_TIMES_3_MAB_TX_FIFO_INIT_ASSERT_TIME_FIELD =
{
    "MAB_TX_FIFO_INIT_ASSERT_TIME",
#if RU_INCLUDE_DESC
    "",
    "...",
#endif
    XPORT_PORTRESET_P3_SIG_ASSERT_TIMES_3_MAB_TX_FIFO_INIT_ASSERT_TIME_FIELD_MASK,
    0,
    XPORT_PORTRESET_P3_SIG_ASSERT_TIMES_3_MAB_TX_FIFO_INIT_ASSERT_TIME_FIELD_WIDTH,
    XPORT_PORTRESET_P3_SIG_ASSERT_TIMES_3_MAB_TX_FIFO_INIT_ASSERT_TIME_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_PORTRESET_P3_SIG_ASSERT_TIMES_4_PORT_IS_UNDER_RESET_ASSERT_TIME
 ******************************************************************************/
const ru_field_rec XPORT_PORTRESET_P3_SIG_ASSERT_TIMES_4_PORT_IS_UNDER_RESET_ASSERT_TIME_FIELD =
{
    "PORT_IS_UNDER_RESET_ASSERT_TIME",
#if RU_INCLUDE_DESC
    "",
    "...",
#endif
    XPORT_PORTRESET_P3_SIG_ASSERT_TIMES_4_PORT_IS_UNDER_RESET_ASSERT_TIME_FIELD_MASK,
    0,
    XPORT_PORTRESET_P3_SIG_ASSERT_TIMES_4_PORT_IS_UNDER_RESET_ASSERT_TIME_FIELD_WIDTH,
    XPORT_PORTRESET_P3_SIG_ASSERT_TIMES_4_PORT_IS_UNDER_RESET_ASSERT_TIME_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_PORTRESET_P3_SIG_ASSERT_TIMES_4_RESERVED0
 ******************************************************************************/
const ru_field_rec XPORT_PORTRESET_P3_SIG_ASSERT_TIMES_4_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    XPORT_PORTRESET_P3_SIG_ASSERT_TIMES_4_RESERVED0_FIELD_MASK,
    0,
    XPORT_PORTRESET_P3_SIG_ASSERT_TIMES_4_RESERVED0_FIELD_WIDTH,
    XPORT_PORTRESET_P3_SIG_ASSERT_TIMES_4_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_PORTRESET_P3_SIG_DEASSERT_TIMES_0_XLMAC_RX_DISAB_DEASSERT_TIME
 ******************************************************************************/
const ru_field_rec XPORT_PORTRESET_P3_SIG_DEASSERT_TIMES_0_XLMAC_RX_DISAB_DEASSERT_TIME_FIELD =
{
    "XLMAC_RX_DISAB_DEASSERT_TIME",
#if RU_INCLUDE_DESC
    "",
    "...",
#endif
    XPORT_PORTRESET_P3_SIG_DEASSERT_TIMES_0_XLMAC_RX_DISAB_DEASSERT_TIME_FIELD_MASK,
    0,
    XPORT_PORTRESET_P3_SIG_DEASSERT_TIMES_0_XLMAC_RX_DISAB_DEASSERT_TIME_FIELD_WIDTH,
    XPORT_PORTRESET_P3_SIG_DEASSERT_TIMES_0_XLMAC_RX_DISAB_DEASSERT_TIME_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_PORTRESET_P3_SIG_DEASSERT_TIMES_0_XLMAC_TX_DISAB_DEASSERT_TIME
 ******************************************************************************/
const ru_field_rec XPORT_PORTRESET_P3_SIG_DEASSERT_TIMES_0_XLMAC_TX_DISAB_DEASSERT_TIME_FIELD =
{
    "XLMAC_TX_DISAB_DEASSERT_TIME",
#if RU_INCLUDE_DESC
    "",
    "...",
#endif
    XPORT_PORTRESET_P3_SIG_DEASSERT_TIMES_0_XLMAC_TX_DISAB_DEASSERT_TIME_FIELD_MASK,
    0,
    XPORT_PORTRESET_P3_SIG_DEASSERT_TIMES_0_XLMAC_TX_DISAB_DEASSERT_TIME_FIELD_WIDTH,
    XPORT_PORTRESET_P3_SIG_DEASSERT_TIMES_0_XLMAC_TX_DISAB_DEASSERT_TIME_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_PORTRESET_P3_SIG_DEASSERT_TIMES_1_XLMAC_TXDISCARD_DEASSERT_TIME
 ******************************************************************************/
const ru_field_rec XPORT_PORTRESET_P3_SIG_DEASSERT_TIMES_1_XLMAC_TXDISCARD_DEASSERT_TIME_FIELD =
{
    "XLMAC_TXDISCARD_DEASSERT_TIME",
#if RU_INCLUDE_DESC
    "",
    "...",
#endif
    XPORT_PORTRESET_P3_SIG_DEASSERT_TIMES_1_XLMAC_TXDISCARD_DEASSERT_TIME_FIELD_MASK,
    0,
    XPORT_PORTRESET_P3_SIG_DEASSERT_TIMES_1_XLMAC_TXDISCARD_DEASSERT_TIME_FIELD_WIDTH,
    XPORT_PORTRESET_P3_SIG_DEASSERT_TIMES_1_XLMAC_TXDISCARD_DEASSERT_TIME_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_PORTRESET_P3_SIG_DEASSERT_TIMES_1_XLMAC_SOFT_RESET_DEASSERT_TIME
 ******************************************************************************/
const ru_field_rec XPORT_PORTRESET_P3_SIG_DEASSERT_TIMES_1_XLMAC_SOFT_RESET_DEASSERT_TIME_FIELD =
{
    "XLMAC_SOFT_RESET_DEASSERT_TIME",
#if RU_INCLUDE_DESC
    "",
    "...",
#endif
    XPORT_PORTRESET_P3_SIG_DEASSERT_TIMES_1_XLMAC_SOFT_RESET_DEASSERT_TIME_FIELD_MASK,
    0,
    XPORT_PORTRESET_P3_SIG_DEASSERT_TIMES_1_XLMAC_SOFT_RESET_DEASSERT_TIME_FIELD_WIDTH,
    XPORT_PORTRESET_P3_SIG_DEASSERT_TIMES_1_XLMAC_SOFT_RESET_DEASSERT_TIME_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_PORTRESET_P3_SIG_DEASSERT_TIMES_2_MAB_RX_PORT_INIT_DEASSERT_TIME
 ******************************************************************************/
const ru_field_rec XPORT_PORTRESET_P3_SIG_DEASSERT_TIMES_2_MAB_RX_PORT_INIT_DEASSERT_TIME_FIELD =
{
    "MAB_RX_PORT_INIT_DEASSERT_TIME",
#if RU_INCLUDE_DESC
    "",
    "...",
#endif
    XPORT_PORTRESET_P3_SIG_DEASSERT_TIMES_2_MAB_RX_PORT_INIT_DEASSERT_TIME_FIELD_MASK,
    0,
    XPORT_PORTRESET_P3_SIG_DEASSERT_TIMES_2_MAB_RX_PORT_INIT_DEASSERT_TIME_FIELD_WIDTH,
    XPORT_PORTRESET_P3_SIG_DEASSERT_TIMES_2_MAB_RX_PORT_INIT_DEASSERT_TIME_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_PORTRESET_P3_SIG_DEASSERT_TIMES_2_MAB_TX_PORT_INIT_DEASSERT_TIME
 ******************************************************************************/
const ru_field_rec XPORT_PORTRESET_P3_SIG_DEASSERT_TIMES_2_MAB_TX_PORT_INIT_DEASSERT_TIME_FIELD =
{
    "MAB_TX_PORT_INIT_DEASSERT_TIME",
#if RU_INCLUDE_DESC
    "",
    "...",
#endif
    XPORT_PORTRESET_P3_SIG_DEASSERT_TIMES_2_MAB_TX_PORT_INIT_DEASSERT_TIME_FIELD_MASK,
    0,
    XPORT_PORTRESET_P3_SIG_DEASSERT_TIMES_2_MAB_TX_PORT_INIT_DEASSERT_TIME_FIELD_WIDTH,
    XPORT_PORTRESET_P3_SIG_DEASSERT_TIMES_2_MAB_TX_PORT_INIT_DEASSERT_TIME_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_PORTRESET_P3_SIG_DEASSERT_TIMES_3_MAB_TX_CREDIT_DISAB_DEASSERT_TIME
 ******************************************************************************/
const ru_field_rec XPORT_PORTRESET_P3_SIG_DEASSERT_TIMES_3_MAB_TX_CREDIT_DISAB_DEASSERT_TIME_FIELD =
{
    "MAB_TX_CREDIT_DISAB_DEASSERT_TIME",
#if RU_INCLUDE_DESC
    "",
    "...",
#endif
    XPORT_PORTRESET_P3_SIG_DEASSERT_TIMES_3_MAB_TX_CREDIT_DISAB_DEASSERT_TIME_FIELD_MASK,
    0,
    XPORT_PORTRESET_P3_SIG_DEASSERT_TIMES_3_MAB_TX_CREDIT_DISAB_DEASSERT_TIME_FIELD_WIDTH,
    XPORT_PORTRESET_P3_SIG_DEASSERT_TIMES_3_MAB_TX_CREDIT_DISAB_DEASSERT_TIME_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_PORTRESET_P3_SIG_DEASSERT_TIMES_3_MAB_TX_FIFO_INIT_DEASSERT_TIME
 ******************************************************************************/
const ru_field_rec XPORT_PORTRESET_P3_SIG_DEASSERT_TIMES_3_MAB_TX_FIFO_INIT_DEASSERT_TIME_FIELD =
{
    "MAB_TX_FIFO_INIT_DEASSERT_TIME",
#if RU_INCLUDE_DESC
    "",
    "...",
#endif
    XPORT_PORTRESET_P3_SIG_DEASSERT_TIMES_3_MAB_TX_FIFO_INIT_DEASSERT_TIME_FIELD_MASK,
    0,
    XPORT_PORTRESET_P3_SIG_DEASSERT_TIMES_3_MAB_TX_FIFO_INIT_DEASSERT_TIME_FIELD_WIDTH,
    XPORT_PORTRESET_P3_SIG_DEASSERT_TIMES_3_MAB_TX_FIFO_INIT_DEASSERT_TIME_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_PORTRESET_P3_SIG_DEASSERT_TIMES_4_PORT_IS_UNDER_RESET_DEASSERT_TIME
 ******************************************************************************/
const ru_field_rec XPORT_PORTRESET_P3_SIG_DEASSERT_TIMES_4_PORT_IS_UNDER_RESET_DEASSERT_TIME_FIELD =
{
    "PORT_IS_UNDER_RESET_DEASSERT_TIME",
#if RU_INCLUDE_DESC
    "",
    "...",
#endif
    XPORT_PORTRESET_P3_SIG_DEASSERT_TIMES_4_PORT_IS_UNDER_RESET_DEASSERT_TIME_FIELD_MASK,
    0,
    XPORT_PORTRESET_P3_SIG_DEASSERT_TIMES_4_PORT_IS_UNDER_RESET_DEASSERT_TIME_FIELD_WIDTH,
    XPORT_PORTRESET_P3_SIG_DEASSERT_TIMES_4_PORT_IS_UNDER_RESET_DEASSERT_TIME_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_PORTRESET_P3_SIG_DEASSERT_TIMES_4_RESERVED0
 ******************************************************************************/
const ru_field_rec XPORT_PORTRESET_P3_SIG_DEASSERT_TIMES_4_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    XPORT_PORTRESET_P3_SIG_DEASSERT_TIMES_4_RESERVED0_FIELD_MASK,
    0,
    XPORT_PORTRESET_P3_SIG_DEASSERT_TIMES_4_RESERVED0_FIELD_WIDTH,
    XPORT_PORTRESET_P3_SIG_DEASSERT_TIMES_4_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_PORTRESET_DEBUG_RESERVED0
 ******************************************************************************/
const ru_field_rec XPORT_PORTRESET_DEBUG_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    XPORT_PORTRESET_DEBUG_RESERVED0_FIELD_MASK,
    0,
    XPORT_PORTRESET_DEBUG_RESERVED0_FIELD_WIDTH,
    XPORT_PORTRESET_DEBUG_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: XPORT_PORTRESET_DEBUG_P3_SM_STATE
 ******************************************************************************/
const ru_field_rec XPORT_PORTRESET_DEBUG_P3_SM_STATE_FIELD =
{
    "P3_SM_STATE",
#if RU_INCLUDE_DESC
    "",
    "...",
#endif
    XPORT_PORTRESET_DEBUG_P3_SM_STATE_FIELD_MASK,
    0,
    XPORT_PORTRESET_DEBUG_P3_SM_STATE_FIELD_WIDTH,
    XPORT_PORTRESET_DEBUG_P3_SM_STATE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: XPORT_PORTRESET_DEBUG_P2_SM_STATE
 ******************************************************************************/
const ru_field_rec XPORT_PORTRESET_DEBUG_P2_SM_STATE_FIELD =
{
    "P2_SM_STATE",
#if RU_INCLUDE_DESC
    "",
    "...",
#endif
    XPORT_PORTRESET_DEBUG_P2_SM_STATE_FIELD_MASK,
    0,
    XPORT_PORTRESET_DEBUG_P2_SM_STATE_FIELD_WIDTH,
    XPORT_PORTRESET_DEBUG_P2_SM_STATE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: XPORT_PORTRESET_DEBUG_P1_SM_STATE
 ******************************************************************************/
const ru_field_rec XPORT_PORTRESET_DEBUG_P1_SM_STATE_FIELD =
{
    "P1_SM_STATE",
#if RU_INCLUDE_DESC
    "",
    "...",
#endif
    XPORT_PORTRESET_DEBUG_P1_SM_STATE_FIELD_MASK,
    0,
    XPORT_PORTRESET_DEBUG_P1_SM_STATE_FIELD_WIDTH,
    XPORT_PORTRESET_DEBUG_P1_SM_STATE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: XPORT_PORTRESET_DEBUG_P0_SM_STATE
 ******************************************************************************/
const ru_field_rec XPORT_PORTRESET_DEBUG_P0_SM_STATE_FIELD =
{
    "P0_SM_STATE",
#if RU_INCLUDE_DESC
    "",
    "...",
#endif
    XPORT_PORTRESET_DEBUG_P0_SM_STATE_FIELD_MASK,
    0,
    XPORT_PORTRESET_DEBUG_P0_SM_STATE_FIELD_WIDTH,
    XPORT_PORTRESET_DEBUG_P0_SM_STATE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

#endif /* RU_INCLUDE_FIELD_DB */

/******************************************************************************
 * Register: XPORT_PORTRESET_P0_CTRL
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XPORT_PORTRESET_P0_CTRL_FIELDS[] =
{
    &XPORT_PORTRESET_P0_CTRL_RESERVED0_FIELD,
    &XPORT_PORTRESET_P0_CTRL_PORT_SW_RESET_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec XPORT_PORTRESET_P0_CTRL_REG = 
{
    "P0_CTRL",
#if RU_INCLUDE_DESC
    "Port Reset P3 Control Register",
    "One register per port.",
#endif
    XPORT_PORTRESET_P0_CTRL_REG_OFFSET,
    0,
    0,
    230,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    XPORT_PORTRESET_P0_CTRL_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: XPORT_PORTRESET_P1_CTRL
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XPORT_PORTRESET_P1_CTRL_FIELDS[] =
{
    &XPORT_PORTRESET_P1_CTRL_RESERVED0_FIELD,
    &XPORT_PORTRESET_P1_CTRL_PORT_SW_RESET_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec XPORT_PORTRESET_P1_CTRL_REG = 
{
    "P1_CTRL",
#if RU_INCLUDE_DESC
    "Port Reset P3 Control Register",
    "One register per port.",
#endif
    XPORT_PORTRESET_P1_CTRL_REG_OFFSET,
    0,
    0,
    231,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    XPORT_PORTRESET_P1_CTRL_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: XPORT_PORTRESET_P2_CTRL
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XPORT_PORTRESET_P2_CTRL_FIELDS[] =
{
    &XPORT_PORTRESET_P2_CTRL_RESERVED0_FIELD,
    &XPORT_PORTRESET_P2_CTRL_PORT_SW_RESET_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec XPORT_PORTRESET_P2_CTRL_REG = 
{
    "P2_CTRL",
#if RU_INCLUDE_DESC
    "Port Reset P3 Control Register",
    "One register per port.",
#endif
    XPORT_PORTRESET_P2_CTRL_REG_OFFSET,
    0,
    0,
    232,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    XPORT_PORTRESET_P2_CTRL_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: XPORT_PORTRESET_P3_CTRL
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XPORT_PORTRESET_P3_CTRL_FIELDS[] =
{
    &XPORT_PORTRESET_P3_CTRL_RESERVED0_FIELD,
    &XPORT_PORTRESET_P3_CTRL_PORT_SW_RESET_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec XPORT_PORTRESET_P3_CTRL_REG = 
{
    "P3_CTRL",
#if RU_INCLUDE_DESC
    "Port Reset P3 Control Register",
    "One register per port.",
#endif
    XPORT_PORTRESET_P3_CTRL_REG_OFFSET,
    0,
    0,
    233,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    XPORT_PORTRESET_P3_CTRL_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: XPORT_PORTRESET_CONFIG
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XPORT_PORTRESET_CONFIG_FIELDS[] =
{
    &XPORT_PORTRESET_CONFIG_RESERVED0_FIELD,
    &XPORT_PORTRESET_CONFIG_LINK_DOWN_RST_EN_FIELD,
    &XPORT_PORTRESET_CONFIG_ENABLE_SM_RUN_FIELD,
    &XPORT_PORTRESET_CONFIG_TICK_TIMER_NDIV_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec XPORT_PORTRESET_CONFIG_REG = 
{
    "CONFIG",
#if RU_INCLUDE_DESC
    "Port Reset Configuration Register",
    "",
#endif
    XPORT_PORTRESET_CONFIG_REG_OFFSET,
    0,
    0,
    234,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    4,
    XPORT_PORTRESET_CONFIG_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: XPORT_PORTRESET_P0_LINK_STAT_DEBOUNCE_CFG
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XPORT_PORTRESET_P0_LINK_STAT_DEBOUNCE_CFG_FIELDS[] =
{
    &XPORT_PORTRESET_P0_LINK_STAT_DEBOUNCE_CFG_RESERVED0_FIELD,
    &XPORT_PORTRESET_P0_LINK_STAT_DEBOUNCE_CFG_DISABLE_FIELD,
    &XPORT_PORTRESET_P0_LINK_STAT_DEBOUNCE_CFG_DEBOUNCE_TIME_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec XPORT_PORTRESET_P0_LINK_STAT_DEBOUNCE_CFG_REG = 
{
    "P0_LINK_STAT_DEBOUNCE_CFG",
#if RU_INCLUDE_DESC
    "P3 Link Status Debouncer Configuration Register",
    "",
#endif
    XPORT_PORTRESET_P0_LINK_STAT_DEBOUNCE_CFG_REG_OFFSET,
    0,
    0,
    235,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    3,
    XPORT_PORTRESET_P0_LINK_STAT_DEBOUNCE_CFG_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: XPORT_PORTRESET_P1_LINK_STAT_DEBOUNCE_CFG
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XPORT_PORTRESET_P1_LINK_STAT_DEBOUNCE_CFG_FIELDS[] =
{
    &XPORT_PORTRESET_P1_LINK_STAT_DEBOUNCE_CFG_RESERVED0_FIELD,
    &XPORT_PORTRESET_P1_LINK_STAT_DEBOUNCE_CFG_DISABLE_FIELD,
    &XPORT_PORTRESET_P1_LINK_STAT_DEBOUNCE_CFG_DEBOUNCE_TIME_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec XPORT_PORTRESET_P1_LINK_STAT_DEBOUNCE_CFG_REG = 
{
    "P1_LINK_STAT_DEBOUNCE_CFG",
#if RU_INCLUDE_DESC
    "P3 Link Status Debouncer Configuration Register",
    "",
#endif
    XPORT_PORTRESET_P1_LINK_STAT_DEBOUNCE_CFG_REG_OFFSET,
    0,
    0,
    236,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    3,
    XPORT_PORTRESET_P1_LINK_STAT_DEBOUNCE_CFG_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: XPORT_PORTRESET_P2_LINK_STAT_DEBOUNCE_CFG
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XPORT_PORTRESET_P2_LINK_STAT_DEBOUNCE_CFG_FIELDS[] =
{
    &XPORT_PORTRESET_P2_LINK_STAT_DEBOUNCE_CFG_RESERVED0_FIELD,
    &XPORT_PORTRESET_P2_LINK_STAT_DEBOUNCE_CFG_DISABLE_FIELD,
    &XPORT_PORTRESET_P2_LINK_STAT_DEBOUNCE_CFG_DEBOUNCE_TIME_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec XPORT_PORTRESET_P2_LINK_STAT_DEBOUNCE_CFG_REG = 
{
    "P2_LINK_STAT_DEBOUNCE_CFG",
#if RU_INCLUDE_DESC
    "P3 Link Status Debouncer Configuration Register",
    "",
#endif
    XPORT_PORTRESET_P2_LINK_STAT_DEBOUNCE_CFG_REG_OFFSET,
    0,
    0,
    237,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    3,
    XPORT_PORTRESET_P2_LINK_STAT_DEBOUNCE_CFG_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: XPORT_PORTRESET_P3_LINK_STAT_DEBOUNCE_CFG
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XPORT_PORTRESET_P3_LINK_STAT_DEBOUNCE_CFG_FIELDS[] =
{
    &XPORT_PORTRESET_P3_LINK_STAT_DEBOUNCE_CFG_RESERVED0_FIELD,
    &XPORT_PORTRESET_P3_LINK_STAT_DEBOUNCE_CFG_DISABLE_FIELD,
    &XPORT_PORTRESET_P3_LINK_STAT_DEBOUNCE_CFG_DEBOUNCE_TIME_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec XPORT_PORTRESET_P3_LINK_STAT_DEBOUNCE_CFG_REG = 
{
    "P3_LINK_STAT_DEBOUNCE_CFG",
#if RU_INCLUDE_DESC
    "P3 Link Status Debouncer Configuration Register",
    "",
#endif
    XPORT_PORTRESET_P3_LINK_STAT_DEBOUNCE_CFG_REG_OFFSET,
    0,
    0,
    238,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    3,
    XPORT_PORTRESET_P3_LINK_STAT_DEBOUNCE_CFG_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: XPORT_PORTRESET_P0_SIG_EN
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XPORT_PORTRESET_P0_SIG_EN_FIELDS[] =
{
    &XPORT_PORTRESET_P0_SIG_EN_RESERVED0_FIELD,
    &XPORT_PORTRESET_P0_SIG_EN_ENABLE_XLMAC_RX_DISAB_FIELD,
    &XPORT_PORTRESET_P0_SIG_EN_ENABLE_XLMAC_TX_DISAB_FIELD,
    &XPORT_PORTRESET_P0_SIG_EN_ENABLE_XLMAC_TX_DISCARD_FIELD,
    &XPORT_PORTRESET_P0_SIG_EN_ENABLE_XLMAC_SOFT_RESET_FIELD,
    &XPORT_PORTRESET_P0_SIG_EN_ENABLE_MAB_RX_PORT_INIT_FIELD,
    &XPORT_PORTRESET_P0_SIG_EN_ENABLE_MAB_TX_PORT_INIT_FIELD,
    &XPORT_PORTRESET_P0_SIG_EN_ENABLE_MAB_TX_CREDIT_DISAB_FIELD,
    &XPORT_PORTRESET_P0_SIG_EN_ENABLE_MAB_TX_FIFO_INIT_FIELD,
    &XPORT_PORTRESET_P0_SIG_EN_ENABLE_PORT_IS_UNDER_RESET_FIELD,
    &XPORT_PORTRESET_P0_SIG_EN_ENABLE_XLMAC_EP_DISCARD_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec XPORT_PORTRESET_P0_SIG_EN_REG = 
{
    "P0_SIG_EN",
#if RU_INCLUDE_DESC
    "P3 Port Reset Signal Enable Configuration Register",
    "",
#endif
    XPORT_PORTRESET_P0_SIG_EN_REG_OFFSET,
    0,
    0,
    239,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    11,
    XPORT_PORTRESET_P0_SIG_EN_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: XPORT_PORTRESET_P0_SIG_ASSERT_TIMES_0
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XPORT_PORTRESET_P0_SIG_ASSERT_TIMES_0_FIELDS[] =
{
    &XPORT_PORTRESET_P0_SIG_ASSERT_TIMES_0_XLMAC_RX_DISAB_ASSERT_TIME_FIELD,
    &XPORT_PORTRESET_P0_SIG_ASSERT_TIMES_0_XLMAC_TX_DISAB_ASSERT_TIME_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec XPORT_PORTRESET_P0_SIG_ASSERT_TIMES_0_REG = 
{
    "P0_SIG_ASSERT_TIMES_0",
#if RU_INCLUDE_DESC
    "P3 Port Reset Signal Assertion Times Configuration Register 0",
    "",
#endif
    XPORT_PORTRESET_P0_SIG_ASSERT_TIMES_0_REG_OFFSET,
    0,
    0,
    240,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    XPORT_PORTRESET_P0_SIG_ASSERT_TIMES_0_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: XPORT_PORTRESET_P0_SIG_ASSERT_TIMES_1
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XPORT_PORTRESET_P0_SIG_ASSERT_TIMES_1_FIELDS[] =
{
    &XPORT_PORTRESET_P0_SIG_ASSERT_TIMES_1_XLMAC_TXDISCARD_ASSERT_TIME_FIELD,
    &XPORT_PORTRESET_P0_SIG_ASSERT_TIMES_1_XLMAC_SOFT_RESET_ASSERT_TIME_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec XPORT_PORTRESET_P0_SIG_ASSERT_TIMES_1_REG = 
{
    "P0_SIG_ASSERT_TIMES_1",
#if RU_INCLUDE_DESC
    "P3 Port Reset Signal Assertion Times Configuration Register 1",
    "",
#endif
    XPORT_PORTRESET_P0_SIG_ASSERT_TIMES_1_REG_OFFSET,
    0,
    0,
    241,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    XPORT_PORTRESET_P0_SIG_ASSERT_TIMES_1_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: XPORT_PORTRESET_P0_SIG_ASSERT_TIMES_2
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XPORT_PORTRESET_P0_SIG_ASSERT_TIMES_2_FIELDS[] =
{
    &XPORT_PORTRESET_P0_SIG_ASSERT_TIMES_2_MAB_RX_PORT_INIT_ASSERT_TIME_FIELD,
    &XPORT_PORTRESET_P0_SIG_ASSERT_TIMES_2_MAB_TX_PORT_INIT_ASSERT_TIME_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec XPORT_PORTRESET_P0_SIG_ASSERT_TIMES_2_REG = 
{
    "P0_SIG_ASSERT_TIMES_2",
#if RU_INCLUDE_DESC
    "P3 Port Reset Signal Assertion Times Configuration Register 2",
    "",
#endif
    XPORT_PORTRESET_P0_SIG_ASSERT_TIMES_2_REG_OFFSET,
    0,
    0,
    242,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    XPORT_PORTRESET_P0_SIG_ASSERT_TIMES_2_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: XPORT_PORTRESET_P0_SIG_ASSERT_TIMES_3
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XPORT_PORTRESET_P0_SIG_ASSERT_TIMES_3_FIELDS[] =
{
    &XPORT_PORTRESET_P0_SIG_ASSERT_TIMES_3_MAB_TX_CREDIT_DISAB_ASSERT_TIME_FIELD,
    &XPORT_PORTRESET_P0_SIG_ASSERT_TIMES_3_MAB_TX_FIFO_INIT_ASSERT_TIME_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec XPORT_PORTRESET_P0_SIG_ASSERT_TIMES_3_REG = 
{
    "P0_SIG_ASSERT_TIMES_3",
#if RU_INCLUDE_DESC
    "P3 Port Reset Signal Assertion Times Configuration Register 3",
    "",
#endif
    XPORT_PORTRESET_P0_SIG_ASSERT_TIMES_3_REG_OFFSET,
    0,
    0,
    243,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    XPORT_PORTRESET_P0_SIG_ASSERT_TIMES_3_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: XPORT_PORTRESET_P0_SIG_ASSERT_TIMES_4
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XPORT_PORTRESET_P0_SIG_ASSERT_TIMES_4_FIELDS[] =
{
    &XPORT_PORTRESET_P0_SIG_ASSERT_TIMES_4_PORT_IS_UNDER_RESET_ASSERT_TIME_FIELD,
    &XPORT_PORTRESET_P0_SIG_ASSERT_TIMES_4_RESERVED0_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec XPORT_PORTRESET_P0_SIG_ASSERT_TIMES_4_REG = 
{
    "P0_SIG_ASSERT_TIMES_4",
#if RU_INCLUDE_DESC
    "P3 Port Reset Signal Assertion Times Configuration Register 4",
    "",
#endif
    XPORT_PORTRESET_P0_SIG_ASSERT_TIMES_4_REG_OFFSET,
    0,
    0,
    244,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    XPORT_PORTRESET_P0_SIG_ASSERT_TIMES_4_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: XPORT_PORTRESET_P0_SIG_DEASSERT_TIMES_0
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XPORT_PORTRESET_P0_SIG_DEASSERT_TIMES_0_FIELDS[] =
{
    &XPORT_PORTRESET_P0_SIG_DEASSERT_TIMES_0_XLMAC_RX_DISAB_DEASSERT_TIME_FIELD,
    &XPORT_PORTRESET_P0_SIG_DEASSERT_TIMES_0_XLMAC_TX_DISAB_DEASSERT_TIME_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec XPORT_PORTRESET_P0_SIG_DEASSERT_TIMES_0_REG = 
{
    "P0_SIG_DEASSERT_TIMES_0",
#if RU_INCLUDE_DESC
    "P3 Port Reset Signal Deassertion Times Configuration Register 0",
    "",
#endif
    XPORT_PORTRESET_P0_SIG_DEASSERT_TIMES_0_REG_OFFSET,
    0,
    0,
    245,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    XPORT_PORTRESET_P0_SIG_DEASSERT_TIMES_0_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: XPORT_PORTRESET_P0_SIG_DEASSERT_TIMES_1
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XPORT_PORTRESET_P0_SIG_DEASSERT_TIMES_1_FIELDS[] =
{
    &XPORT_PORTRESET_P0_SIG_DEASSERT_TIMES_1_XLMAC_TXDISCARD_DEASSERT_TIME_FIELD,
    &XPORT_PORTRESET_P0_SIG_DEASSERT_TIMES_1_XLMAC_SOFT_RESET_DEASSERT_TIME_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec XPORT_PORTRESET_P0_SIG_DEASSERT_TIMES_1_REG = 
{
    "P0_SIG_DEASSERT_TIMES_1",
#if RU_INCLUDE_DESC
    "P3 Port Reset Signal Deassertion Times Configuration Register 1",
    "",
#endif
    XPORT_PORTRESET_P0_SIG_DEASSERT_TIMES_1_REG_OFFSET,
    0,
    0,
    246,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    XPORT_PORTRESET_P0_SIG_DEASSERT_TIMES_1_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: XPORT_PORTRESET_P0_SIG_DEASSERT_TIMES_2
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XPORT_PORTRESET_P0_SIG_DEASSERT_TIMES_2_FIELDS[] =
{
    &XPORT_PORTRESET_P0_SIG_DEASSERT_TIMES_2_MAB_RX_PORT_INIT_DEASSERT_TIME_FIELD,
    &XPORT_PORTRESET_P0_SIG_DEASSERT_TIMES_2_MAB_TX_PORT_INIT_DEASSERT_TIME_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec XPORT_PORTRESET_P0_SIG_DEASSERT_TIMES_2_REG = 
{
    "P0_SIG_DEASSERT_TIMES_2",
#if RU_INCLUDE_DESC
    "P3 Port Reset Signal Deassertion Times Configuration Register 2",
    "",
#endif
    XPORT_PORTRESET_P0_SIG_DEASSERT_TIMES_2_REG_OFFSET,
    0,
    0,
    247,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    XPORT_PORTRESET_P0_SIG_DEASSERT_TIMES_2_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: XPORT_PORTRESET_P0_SIG_DEASSERT_TIMES_3
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XPORT_PORTRESET_P0_SIG_DEASSERT_TIMES_3_FIELDS[] =
{
    &XPORT_PORTRESET_P0_SIG_DEASSERT_TIMES_3_MAB_TX_CREDIT_DISAB_DEASSERT_TIME_FIELD,
    &XPORT_PORTRESET_P0_SIG_DEASSERT_TIMES_3_MAB_TX_FIFO_INIT_DEASSERT_TIME_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec XPORT_PORTRESET_P0_SIG_DEASSERT_TIMES_3_REG = 
{
    "P0_SIG_DEASSERT_TIMES_3",
#if RU_INCLUDE_DESC
    "P3 Port Reset Signal Deassertion Times Configuration Register 3",
    "",
#endif
    XPORT_PORTRESET_P0_SIG_DEASSERT_TIMES_3_REG_OFFSET,
    0,
    0,
    248,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    XPORT_PORTRESET_P0_SIG_DEASSERT_TIMES_3_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: XPORT_PORTRESET_P0_SIG_DEASSERT_TIMES_4
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XPORT_PORTRESET_P0_SIG_DEASSERT_TIMES_4_FIELDS[] =
{
    &XPORT_PORTRESET_P0_SIG_DEASSERT_TIMES_4_PORT_IS_UNDER_RESET_DEASSERT_TIME_FIELD,
    &XPORT_PORTRESET_P0_SIG_DEASSERT_TIMES_4_RESERVED0_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec XPORT_PORTRESET_P0_SIG_DEASSERT_TIMES_4_REG = 
{
    "P0_SIG_DEASSERT_TIMES_4",
#if RU_INCLUDE_DESC
    "P3 Port Reset Signal Deassertion Times Configuration Register 4",
    "",
#endif
    XPORT_PORTRESET_P0_SIG_DEASSERT_TIMES_4_REG_OFFSET,
    0,
    0,
    249,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    XPORT_PORTRESET_P0_SIG_DEASSERT_TIMES_4_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: XPORT_PORTRESET_P1_SIG_EN
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XPORT_PORTRESET_P1_SIG_EN_FIELDS[] =
{
    &XPORT_PORTRESET_P1_SIG_EN_RESERVED0_FIELD,
    &XPORT_PORTRESET_P1_SIG_EN_ENABLE_XLMAC_RX_DISAB_FIELD,
    &XPORT_PORTRESET_P1_SIG_EN_ENABLE_XLMAC_TX_DISAB_FIELD,
    &XPORT_PORTRESET_P1_SIG_EN_ENABLE_XLMAC_TX_DISCARD_FIELD,
    &XPORT_PORTRESET_P1_SIG_EN_ENABLE_XLMAC_SOFT_RESET_FIELD,
    &XPORT_PORTRESET_P1_SIG_EN_ENABLE_MAB_RX_PORT_INIT_FIELD,
    &XPORT_PORTRESET_P1_SIG_EN_ENABLE_MAB_TX_PORT_INIT_FIELD,
    &XPORT_PORTRESET_P1_SIG_EN_ENABLE_MAB_TX_CREDIT_DISAB_FIELD,
    &XPORT_PORTRESET_P1_SIG_EN_ENABLE_MAB_TX_FIFO_INIT_FIELD,
    &XPORT_PORTRESET_P1_SIG_EN_ENABLE_PORT_IS_UNDER_RESET_FIELD,
    &XPORT_PORTRESET_P1_SIG_EN_ENABLE_XLMAC_EP_DISCARD_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec XPORT_PORTRESET_P1_SIG_EN_REG = 
{
    "P1_SIG_EN",
#if RU_INCLUDE_DESC
    "P3 Port Reset Signal Enable Configuration Register",
    "",
#endif
    XPORT_PORTRESET_P1_SIG_EN_REG_OFFSET,
    0,
    0,
    250,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    11,
    XPORT_PORTRESET_P1_SIG_EN_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: XPORT_PORTRESET_P1_SIG_ASSERT_TIMES_0
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XPORT_PORTRESET_P1_SIG_ASSERT_TIMES_0_FIELDS[] =
{
    &XPORT_PORTRESET_P1_SIG_ASSERT_TIMES_0_XLMAC_RX_DISAB_ASSERT_TIME_FIELD,
    &XPORT_PORTRESET_P1_SIG_ASSERT_TIMES_0_XLMAC_TX_DISAB_ASSERT_TIME_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec XPORT_PORTRESET_P1_SIG_ASSERT_TIMES_0_REG = 
{
    "P1_SIG_ASSERT_TIMES_0",
#if RU_INCLUDE_DESC
    "P3 Port Reset Signal Assertion Times Configuration Register 0",
    "",
#endif
    XPORT_PORTRESET_P1_SIG_ASSERT_TIMES_0_REG_OFFSET,
    0,
    0,
    251,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    XPORT_PORTRESET_P1_SIG_ASSERT_TIMES_0_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: XPORT_PORTRESET_P1_SIG_ASSERT_TIMES_1
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XPORT_PORTRESET_P1_SIG_ASSERT_TIMES_1_FIELDS[] =
{
    &XPORT_PORTRESET_P1_SIG_ASSERT_TIMES_1_XLMAC_TXDISCARD_ASSERT_TIME_FIELD,
    &XPORT_PORTRESET_P1_SIG_ASSERT_TIMES_1_XLMAC_SOFT_RESET_ASSERT_TIME_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec XPORT_PORTRESET_P1_SIG_ASSERT_TIMES_1_REG = 
{
    "P1_SIG_ASSERT_TIMES_1",
#if RU_INCLUDE_DESC
    "P3 Port Reset Signal Assertion Times Configuration Register 1",
    "",
#endif
    XPORT_PORTRESET_P1_SIG_ASSERT_TIMES_1_REG_OFFSET,
    0,
    0,
    252,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    XPORT_PORTRESET_P1_SIG_ASSERT_TIMES_1_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: XPORT_PORTRESET_P1_SIG_ASSERT_TIMES_2
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XPORT_PORTRESET_P1_SIG_ASSERT_TIMES_2_FIELDS[] =
{
    &XPORT_PORTRESET_P1_SIG_ASSERT_TIMES_2_MAB_RX_PORT_INIT_ASSERT_TIME_FIELD,
    &XPORT_PORTRESET_P1_SIG_ASSERT_TIMES_2_MAB_TX_PORT_INIT_ASSERT_TIME_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec XPORT_PORTRESET_P1_SIG_ASSERT_TIMES_2_REG = 
{
    "P1_SIG_ASSERT_TIMES_2",
#if RU_INCLUDE_DESC
    "P3 Port Reset Signal Assertion Times Configuration Register 2",
    "",
#endif
    XPORT_PORTRESET_P1_SIG_ASSERT_TIMES_2_REG_OFFSET,
    0,
    0,
    253,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    XPORT_PORTRESET_P1_SIG_ASSERT_TIMES_2_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: XPORT_PORTRESET_P1_SIG_ASSERT_TIMES_3
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XPORT_PORTRESET_P1_SIG_ASSERT_TIMES_3_FIELDS[] =
{
    &XPORT_PORTRESET_P1_SIG_ASSERT_TIMES_3_MAB_TX_CREDIT_DISAB_ASSERT_TIME_FIELD,
    &XPORT_PORTRESET_P1_SIG_ASSERT_TIMES_3_MAB_TX_FIFO_INIT_ASSERT_TIME_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec XPORT_PORTRESET_P1_SIG_ASSERT_TIMES_3_REG = 
{
    "P1_SIG_ASSERT_TIMES_3",
#if RU_INCLUDE_DESC
    "P3 Port Reset Signal Assertion Times Configuration Register 3",
    "",
#endif
    XPORT_PORTRESET_P1_SIG_ASSERT_TIMES_3_REG_OFFSET,
    0,
    0,
    254,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    XPORT_PORTRESET_P1_SIG_ASSERT_TIMES_3_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: XPORT_PORTRESET_P1_SIG_ASSERT_TIMES_4
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XPORT_PORTRESET_P1_SIG_ASSERT_TIMES_4_FIELDS[] =
{
    &XPORT_PORTRESET_P1_SIG_ASSERT_TIMES_4_PORT_IS_UNDER_RESET_ASSERT_TIME_FIELD,
    &XPORT_PORTRESET_P1_SIG_ASSERT_TIMES_4_RESERVED0_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec XPORT_PORTRESET_P1_SIG_ASSERT_TIMES_4_REG = 
{
    "P1_SIG_ASSERT_TIMES_4",
#if RU_INCLUDE_DESC
    "P3 Port Reset Signal Assertion Times Configuration Register 4",
    "",
#endif
    XPORT_PORTRESET_P1_SIG_ASSERT_TIMES_4_REG_OFFSET,
    0,
    0,
    255,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    XPORT_PORTRESET_P1_SIG_ASSERT_TIMES_4_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: XPORT_PORTRESET_P1_SIG_DEASSERT_TIMES_0
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XPORT_PORTRESET_P1_SIG_DEASSERT_TIMES_0_FIELDS[] =
{
    &XPORT_PORTRESET_P1_SIG_DEASSERT_TIMES_0_XLMAC_RX_DISAB_DEASSERT_TIME_FIELD,
    &XPORT_PORTRESET_P1_SIG_DEASSERT_TIMES_0_XLMAC_TX_DISAB_DEASSERT_TIME_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec XPORT_PORTRESET_P1_SIG_DEASSERT_TIMES_0_REG = 
{
    "P1_SIG_DEASSERT_TIMES_0",
#if RU_INCLUDE_DESC
    "P3 Port Reset Signal Deassertion Times Configuration Register 0",
    "",
#endif
    XPORT_PORTRESET_P1_SIG_DEASSERT_TIMES_0_REG_OFFSET,
    0,
    0,
    256,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    XPORT_PORTRESET_P1_SIG_DEASSERT_TIMES_0_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: XPORT_PORTRESET_P1_SIG_DEASSERT_TIMES_1
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XPORT_PORTRESET_P1_SIG_DEASSERT_TIMES_1_FIELDS[] =
{
    &XPORT_PORTRESET_P1_SIG_DEASSERT_TIMES_1_XLMAC_TXDISCARD_DEASSERT_TIME_FIELD,
    &XPORT_PORTRESET_P1_SIG_DEASSERT_TIMES_1_XLMAC_SOFT_RESET_DEASSERT_TIME_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec XPORT_PORTRESET_P1_SIG_DEASSERT_TIMES_1_REG = 
{
    "P1_SIG_DEASSERT_TIMES_1",
#if RU_INCLUDE_DESC
    "P3 Port Reset Signal Deassertion Times Configuration Register 1",
    "",
#endif
    XPORT_PORTRESET_P1_SIG_DEASSERT_TIMES_1_REG_OFFSET,
    0,
    0,
    257,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    XPORT_PORTRESET_P1_SIG_DEASSERT_TIMES_1_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: XPORT_PORTRESET_P1_SIG_DEASSERT_TIMES_2
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XPORT_PORTRESET_P1_SIG_DEASSERT_TIMES_2_FIELDS[] =
{
    &XPORT_PORTRESET_P1_SIG_DEASSERT_TIMES_2_MAB_RX_PORT_INIT_DEASSERT_TIME_FIELD,
    &XPORT_PORTRESET_P1_SIG_DEASSERT_TIMES_2_MAB_TX_PORT_INIT_DEASSERT_TIME_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec XPORT_PORTRESET_P1_SIG_DEASSERT_TIMES_2_REG = 
{
    "P1_SIG_DEASSERT_TIMES_2",
#if RU_INCLUDE_DESC
    "P3 Port Reset Signal Deassertion Times Configuration Register 2",
    "",
#endif
    XPORT_PORTRESET_P1_SIG_DEASSERT_TIMES_2_REG_OFFSET,
    0,
    0,
    258,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    XPORT_PORTRESET_P1_SIG_DEASSERT_TIMES_2_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: XPORT_PORTRESET_P1_SIG_DEASSERT_TIMES_3
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XPORT_PORTRESET_P1_SIG_DEASSERT_TIMES_3_FIELDS[] =
{
    &XPORT_PORTRESET_P1_SIG_DEASSERT_TIMES_3_MAB_TX_CREDIT_DISAB_DEASSERT_TIME_FIELD,
    &XPORT_PORTRESET_P1_SIG_DEASSERT_TIMES_3_MAB_TX_FIFO_INIT_DEASSERT_TIME_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec XPORT_PORTRESET_P1_SIG_DEASSERT_TIMES_3_REG = 
{
    "P1_SIG_DEASSERT_TIMES_3",
#if RU_INCLUDE_DESC
    "P3 Port Reset Signal Deassertion Times Configuration Register 3",
    "",
#endif
    XPORT_PORTRESET_P1_SIG_DEASSERT_TIMES_3_REG_OFFSET,
    0,
    0,
    259,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    XPORT_PORTRESET_P1_SIG_DEASSERT_TIMES_3_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: XPORT_PORTRESET_P1_SIG_DEASSERT_TIMES_4
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XPORT_PORTRESET_P1_SIG_DEASSERT_TIMES_4_FIELDS[] =
{
    &XPORT_PORTRESET_P1_SIG_DEASSERT_TIMES_4_PORT_IS_UNDER_RESET_DEASSERT_TIME_FIELD,
    &XPORT_PORTRESET_P1_SIG_DEASSERT_TIMES_4_RESERVED0_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec XPORT_PORTRESET_P1_SIG_DEASSERT_TIMES_4_REG = 
{
    "P1_SIG_DEASSERT_TIMES_4",
#if RU_INCLUDE_DESC
    "P3 Port Reset Signal Deassertion Times Configuration Register 4",
    "",
#endif
    XPORT_PORTRESET_P1_SIG_DEASSERT_TIMES_4_REG_OFFSET,
    0,
    0,
    260,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    XPORT_PORTRESET_P1_SIG_DEASSERT_TIMES_4_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: XPORT_PORTRESET_P2_SIG_EN
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XPORT_PORTRESET_P2_SIG_EN_FIELDS[] =
{
    &XPORT_PORTRESET_P2_SIG_EN_RESERVED0_FIELD,
    &XPORT_PORTRESET_P2_SIG_EN_ENABLE_XLMAC_RX_DISAB_FIELD,
    &XPORT_PORTRESET_P2_SIG_EN_ENABLE_XLMAC_TX_DISAB_FIELD,
    &XPORT_PORTRESET_P2_SIG_EN_ENABLE_XLMAC_TX_DISCARD_FIELD,
    &XPORT_PORTRESET_P2_SIG_EN_ENABLE_XLMAC_SOFT_RESET_FIELD,
    &XPORT_PORTRESET_P2_SIG_EN_ENABLE_MAB_RX_PORT_INIT_FIELD,
    &XPORT_PORTRESET_P2_SIG_EN_ENABLE_MAB_TX_PORT_INIT_FIELD,
    &XPORT_PORTRESET_P2_SIG_EN_ENABLE_MAB_TX_CREDIT_DISAB_FIELD,
    &XPORT_PORTRESET_P2_SIG_EN_ENABLE_MAB_TX_FIFO_INIT_FIELD,
    &XPORT_PORTRESET_P2_SIG_EN_ENABLE_PORT_IS_UNDER_RESET_FIELD,
    &XPORT_PORTRESET_P2_SIG_EN_ENABLE_XLMAC_EP_DISCARD_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec XPORT_PORTRESET_P2_SIG_EN_REG = 
{
    "P2_SIG_EN",
#if RU_INCLUDE_DESC
    "P3 Port Reset Signal Enable Configuration Register",
    "",
#endif
    XPORT_PORTRESET_P2_SIG_EN_REG_OFFSET,
    0,
    0,
    261,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    11,
    XPORT_PORTRESET_P2_SIG_EN_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: XPORT_PORTRESET_P2_SIG_ASSERT_TIMES_0
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XPORT_PORTRESET_P2_SIG_ASSERT_TIMES_0_FIELDS[] =
{
    &XPORT_PORTRESET_P2_SIG_ASSERT_TIMES_0_XLMAC_RX_DISAB_ASSERT_TIME_FIELD,
    &XPORT_PORTRESET_P2_SIG_ASSERT_TIMES_0_XLMAC_TX_DISAB_ASSERT_TIME_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec XPORT_PORTRESET_P2_SIG_ASSERT_TIMES_0_REG = 
{
    "P2_SIG_ASSERT_TIMES_0",
#if RU_INCLUDE_DESC
    "P3 Port Reset Signal Assertion Times Configuration Register 0",
    "",
#endif
    XPORT_PORTRESET_P2_SIG_ASSERT_TIMES_0_REG_OFFSET,
    0,
    0,
    262,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    XPORT_PORTRESET_P2_SIG_ASSERT_TIMES_0_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: XPORT_PORTRESET_P2_SIG_ASSERT_TIMES_1
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XPORT_PORTRESET_P2_SIG_ASSERT_TIMES_1_FIELDS[] =
{
    &XPORT_PORTRESET_P2_SIG_ASSERT_TIMES_1_XLMAC_TXDISCARD_ASSERT_TIME_FIELD,
    &XPORT_PORTRESET_P2_SIG_ASSERT_TIMES_1_XLMAC_SOFT_RESET_ASSERT_TIME_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec XPORT_PORTRESET_P2_SIG_ASSERT_TIMES_1_REG = 
{
    "P2_SIG_ASSERT_TIMES_1",
#if RU_INCLUDE_DESC
    "P3 Port Reset Signal Assertion Times Configuration Register 1",
    "",
#endif
    XPORT_PORTRESET_P2_SIG_ASSERT_TIMES_1_REG_OFFSET,
    0,
    0,
    263,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    XPORT_PORTRESET_P2_SIG_ASSERT_TIMES_1_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: XPORT_PORTRESET_P2_SIG_ASSERT_TIMES_2
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XPORT_PORTRESET_P2_SIG_ASSERT_TIMES_2_FIELDS[] =
{
    &XPORT_PORTRESET_P2_SIG_ASSERT_TIMES_2_MAB_RX_PORT_INIT_ASSERT_TIME_FIELD,
    &XPORT_PORTRESET_P2_SIG_ASSERT_TIMES_2_MAB_TX_PORT_INIT_ASSERT_TIME_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec XPORT_PORTRESET_P2_SIG_ASSERT_TIMES_2_REG = 
{
    "P2_SIG_ASSERT_TIMES_2",
#if RU_INCLUDE_DESC
    "P3 Port Reset Signal Assertion Times Configuration Register 2",
    "",
#endif
    XPORT_PORTRESET_P2_SIG_ASSERT_TIMES_2_REG_OFFSET,
    0,
    0,
    264,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    XPORT_PORTRESET_P2_SIG_ASSERT_TIMES_2_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: XPORT_PORTRESET_P2_SIG_ASSERT_TIMES_3
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XPORT_PORTRESET_P2_SIG_ASSERT_TIMES_3_FIELDS[] =
{
    &XPORT_PORTRESET_P2_SIG_ASSERT_TIMES_3_MAB_TX_CREDIT_DISAB_ASSERT_TIME_FIELD,
    &XPORT_PORTRESET_P2_SIG_ASSERT_TIMES_3_MAB_TX_FIFO_INIT_ASSERT_TIME_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec XPORT_PORTRESET_P2_SIG_ASSERT_TIMES_3_REG = 
{
    "P2_SIG_ASSERT_TIMES_3",
#if RU_INCLUDE_DESC
    "P3 Port Reset Signal Assertion Times Configuration Register 3",
    "",
#endif
    XPORT_PORTRESET_P2_SIG_ASSERT_TIMES_3_REG_OFFSET,
    0,
    0,
    265,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    XPORT_PORTRESET_P2_SIG_ASSERT_TIMES_3_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: XPORT_PORTRESET_P2_SIG_ASSERT_TIMES_4
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XPORT_PORTRESET_P2_SIG_ASSERT_TIMES_4_FIELDS[] =
{
    &XPORT_PORTRESET_P2_SIG_ASSERT_TIMES_4_PORT_IS_UNDER_RESET_ASSERT_TIME_FIELD,
    &XPORT_PORTRESET_P2_SIG_ASSERT_TIMES_4_RESERVED0_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec XPORT_PORTRESET_P2_SIG_ASSERT_TIMES_4_REG = 
{
    "P2_SIG_ASSERT_TIMES_4",
#if RU_INCLUDE_DESC
    "P3 Port Reset Signal Assertion Times Configuration Register 4",
    "",
#endif
    XPORT_PORTRESET_P2_SIG_ASSERT_TIMES_4_REG_OFFSET,
    0,
    0,
    266,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    XPORT_PORTRESET_P2_SIG_ASSERT_TIMES_4_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: XPORT_PORTRESET_P2_SIG_DEASSERT_TIMES_0
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XPORT_PORTRESET_P2_SIG_DEASSERT_TIMES_0_FIELDS[] =
{
    &XPORT_PORTRESET_P2_SIG_DEASSERT_TIMES_0_XLMAC_RX_DISAB_DEASSERT_TIME_FIELD,
    &XPORT_PORTRESET_P2_SIG_DEASSERT_TIMES_0_XLMAC_TX_DISAB_DEASSERT_TIME_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec XPORT_PORTRESET_P2_SIG_DEASSERT_TIMES_0_REG = 
{
    "P2_SIG_DEASSERT_TIMES_0",
#if RU_INCLUDE_DESC
    "P3 Port Reset Signal Deassertion Times Configuration Register 0",
    "",
#endif
    XPORT_PORTRESET_P2_SIG_DEASSERT_TIMES_0_REG_OFFSET,
    0,
    0,
    267,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    XPORT_PORTRESET_P2_SIG_DEASSERT_TIMES_0_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: XPORT_PORTRESET_P2_SIG_DEASSERT_TIMES_1
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XPORT_PORTRESET_P2_SIG_DEASSERT_TIMES_1_FIELDS[] =
{
    &XPORT_PORTRESET_P2_SIG_DEASSERT_TIMES_1_XLMAC_TXDISCARD_DEASSERT_TIME_FIELD,
    &XPORT_PORTRESET_P2_SIG_DEASSERT_TIMES_1_XLMAC_SOFT_RESET_DEASSERT_TIME_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec XPORT_PORTRESET_P2_SIG_DEASSERT_TIMES_1_REG = 
{
    "P2_SIG_DEASSERT_TIMES_1",
#if RU_INCLUDE_DESC
    "P3 Port Reset Signal Deassertion Times Configuration Register 1",
    "",
#endif
    XPORT_PORTRESET_P2_SIG_DEASSERT_TIMES_1_REG_OFFSET,
    0,
    0,
    268,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    XPORT_PORTRESET_P2_SIG_DEASSERT_TIMES_1_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: XPORT_PORTRESET_P2_SIG_DEASSERT_TIMES_2
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XPORT_PORTRESET_P2_SIG_DEASSERT_TIMES_2_FIELDS[] =
{
    &XPORT_PORTRESET_P2_SIG_DEASSERT_TIMES_2_MAB_RX_PORT_INIT_DEASSERT_TIME_FIELD,
    &XPORT_PORTRESET_P2_SIG_DEASSERT_TIMES_2_MAB_TX_PORT_INIT_DEASSERT_TIME_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec XPORT_PORTRESET_P2_SIG_DEASSERT_TIMES_2_REG = 
{
    "P2_SIG_DEASSERT_TIMES_2",
#if RU_INCLUDE_DESC
    "P3 Port Reset Signal Deassertion Times Configuration Register 2",
    "",
#endif
    XPORT_PORTRESET_P2_SIG_DEASSERT_TIMES_2_REG_OFFSET,
    0,
    0,
    269,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    XPORT_PORTRESET_P2_SIG_DEASSERT_TIMES_2_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: XPORT_PORTRESET_P2_SIG_DEASSERT_TIMES_3
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XPORT_PORTRESET_P2_SIG_DEASSERT_TIMES_3_FIELDS[] =
{
    &XPORT_PORTRESET_P2_SIG_DEASSERT_TIMES_3_MAB_TX_CREDIT_DISAB_DEASSERT_TIME_FIELD,
    &XPORT_PORTRESET_P2_SIG_DEASSERT_TIMES_3_MAB_TX_FIFO_INIT_DEASSERT_TIME_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec XPORT_PORTRESET_P2_SIG_DEASSERT_TIMES_3_REG = 
{
    "P2_SIG_DEASSERT_TIMES_3",
#if RU_INCLUDE_DESC
    "P3 Port Reset Signal Deassertion Times Configuration Register 3",
    "",
#endif
    XPORT_PORTRESET_P2_SIG_DEASSERT_TIMES_3_REG_OFFSET,
    0,
    0,
    270,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    XPORT_PORTRESET_P2_SIG_DEASSERT_TIMES_3_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: XPORT_PORTRESET_P2_SIG_DEASSERT_TIMES_4
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XPORT_PORTRESET_P2_SIG_DEASSERT_TIMES_4_FIELDS[] =
{
    &XPORT_PORTRESET_P2_SIG_DEASSERT_TIMES_4_PORT_IS_UNDER_RESET_DEASSERT_TIME_FIELD,
    &XPORT_PORTRESET_P2_SIG_DEASSERT_TIMES_4_RESERVED0_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec XPORT_PORTRESET_P2_SIG_DEASSERT_TIMES_4_REG = 
{
    "P2_SIG_DEASSERT_TIMES_4",
#if RU_INCLUDE_DESC
    "P3 Port Reset Signal Deassertion Times Configuration Register 4",
    "",
#endif
    XPORT_PORTRESET_P2_SIG_DEASSERT_TIMES_4_REG_OFFSET,
    0,
    0,
    271,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    XPORT_PORTRESET_P2_SIG_DEASSERT_TIMES_4_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: XPORT_PORTRESET_P3_SIG_EN
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XPORT_PORTRESET_P3_SIG_EN_FIELDS[] =
{
    &XPORT_PORTRESET_P3_SIG_EN_RESERVED0_FIELD,
    &XPORT_PORTRESET_P3_SIG_EN_ENABLE_XLMAC_RX_DISAB_FIELD,
    &XPORT_PORTRESET_P3_SIG_EN_ENABLE_XLMAC_TX_DISAB_FIELD,
    &XPORT_PORTRESET_P3_SIG_EN_ENABLE_XLMAC_TX_DISCARD_FIELD,
    &XPORT_PORTRESET_P3_SIG_EN_ENABLE_XLMAC_SOFT_RESET_FIELD,
    &XPORT_PORTRESET_P3_SIG_EN_ENABLE_MAB_RX_PORT_INIT_FIELD,
    &XPORT_PORTRESET_P3_SIG_EN_ENABLE_MAB_TX_PORT_INIT_FIELD,
    &XPORT_PORTRESET_P3_SIG_EN_ENABLE_MAB_TX_CREDIT_DISAB_FIELD,
    &XPORT_PORTRESET_P3_SIG_EN_ENABLE_MAB_TX_FIFO_INIT_FIELD,
    &XPORT_PORTRESET_P3_SIG_EN_ENABLE_PORT_IS_UNDER_RESET_FIELD,
    &XPORT_PORTRESET_P3_SIG_EN_ENABLE_XLMAC_EP_DISCARD_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec XPORT_PORTRESET_P3_SIG_EN_REG = 
{
    "P3_SIG_EN",
#if RU_INCLUDE_DESC
    "P3 Port Reset Signal Enable Configuration Register",
    "",
#endif
    XPORT_PORTRESET_P3_SIG_EN_REG_OFFSET,
    0,
    0,
    272,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    11,
    XPORT_PORTRESET_P3_SIG_EN_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: XPORT_PORTRESET_P3_SIG_ASSERT_TIMES_0
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XPORT_PORTRESET_P3_SIG_ASSERT_TIMES_0_FIELDS[] =
{
    &XPORT_PORTRESET_P3_SIG_ASSERT_TIMES_0_XLMAC_RX_DISAB_ASSERT_TIME_FIELD,
    &XPORT_PORTRESET_P3_SIG_ASSERT_TIMES_0_XLMAC_TX_DISAB_ASSERT_TIME_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec XPORT_PORTRESET_P3_SIG_ASSERT_TIMES_0_REG = 
{
    "P3_SIG_ASSERT_TIMES_0",
#if RU_INCLUDE_DESC
    "P3 Port Reset Signal Assertion Times Configuration Register 0",
    "",
#endif
    XPORT_PORTRESET_P3_SIG_ASSERT_TIMES_0_REG_OFFSET,
    0,
    0,
    273,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    XPORT_PORTRESET_P3_SIG_ASSERT_TIMES_0_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: XPORT_PORTRESET_P3_SIG_ASSERT_TIMES_1
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XPORT_PORTRESET_P3_SIG_ASSERT_TIMES_1_FIELDS[] =
{
    &XPORT_PORTRESET_P3_SIG_ASSERT_TIMES_1_XLMAC_TXDISCARD_ASSERT_TIME_FIELD,
    &XPORT_PORTRESET_P3_SIG_ASSERT_TIMES_1_XLMAC_SOFT_RESET_ASSERT_TIME_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec XPORT_PORTRESET_P3_SIG_ASSERT_TIMES_1_REG = 
{
    "P3_SIG_ASSERT_TIMES_1",
#if RU_INCLUDE_DESC
    "P3 Port Reset Signal Assertion Times Configuration Register 1",
    "",
#endif
    XPORT_PORTRESET_P3_SIG_ASSERT_TIMES_1_REG_OFFSET,
    0,
    0,
    274,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    XPORT_PORTRESET_P3_SIG_ASSERT_TIMES_1_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: XPORT_PORTRESET_P3_SIG_ASSERT_TIMES_2
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XPORT_PORTRESET_P3_SIG_ASSERT_TIMES_2_FIELDS[] =
{
    &XPORT_PORTRESET_P3_SIG_ASSERT_TIMES_2_MAB_RX_PORT_INIT_ASSERT_TIME_FIELD,
    &XPORT_PORTRESET_P3_SIG_ASSERT_TIMES_2_MAB_TX_PORT_INIT_ASSERT_TIME_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec XPORT_PORTRESET_P3_SIG_ASSERT_TIMES_2_REG = 
{
    "P3_SIG_ASSERT_TIMES_2",
#if RU_INCLUDE_DESC
    "P3 Port Reset Signal Assertion Times Configuration Register 2",
    "",
#endif
    XPORT_PORTRESET_P3_SIG_ASSERT_TIMES_2_REG_OFFSET,
    0,
    0,
    275,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    XPORT_PORTRESET_P3_SIG_ASSERT_TIMES_2_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: XPORT_PORTRESET_P3_SIG_ASSERT_TIMES_3
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XPORT_PORTRESET_P3_SIG_ASSERT_TIMES_3_FIELDS[] =
{
    &XPORT_PORTRESET_P3_SIG_ASSERT_TIMES_3_MAB_TX_CREDIT_DISAB_ASSERT_TIME_FIELD,
    &XPORT_PORTRESET_P3_SIG_ASSERT_TIMES_3_MAB_TX_FIFO_INIT_ASSERT_TIME_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec XPORT_PORTRESET_P3_SIG_ASSERT_TIMES_3_REG = 
{
    "P3_SIG_ASSERT_TIMES_3",
#if RU_INCLUDE_DESC
    "P3 Port Reset Signal Assertion Times Configuration Register 3",
    "",
#endif
    XPORT_PORTRESET_P3_SIG_ASSERT_TIMES_3_REG_OFFSET,
    0,
    0,
    276,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    XPORT_PORTRESET_P3_SIG_ASSERT_TIMES_3_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: XPORT_PORTRESET_P3_SIG_ASSERT_TIMES_4
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XPORT_PORTRESET_P3_SIG_ASSERT_TIMES_4_FIELDS[] =
{
    &XPORT_PORTRESET_P3_SIG_ASSERT_TIMES_4_PORT_IS_UNDER_RESET_ASSERT_TIME_FIELD,
    &XPORT_PORTRESET_P3_SIG_ASSERT_TIMES_4_RESERVED0_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec XPORT_PORTRESET_P3_SIG_ASSERT_TIMES_4_REG = 
{
    "P3_SIG_ASSERT_TIMES_4",
#if RU_INCLUDE_DESC
    "P3 Port Reset Signal Assertion Times Configuration Register 4",
    "",
#endif
    XPORT_PORTRESET_P3_SIG_ASSERT_TIMES_4_REG_OFFSET,
    0,
    0,
    277,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    XPORT_PORTRESET_P3_SIG_ASSERT_TIMES_4_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: XPORT_PORTRESET_P3_SIG_DEASSERT_TIMES_0
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XPORT_PORTRESET_P3_SIG_DEASSERT_TIMES_0_FIELDS[] =
{
    &XPORT_PORTRESET_P3_SIG_DEASSERT_TIMES_0_XLMAC_RX_DISAB_DEASSERT_TIME_FIELD,
    &XPORT_PORTRESET_P3_SIG_DEASSERT_TIMES_0_XLMAC_TX_DISAB_DEASSERT_TIME_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec XPORT_PORTRESET_P3_SIG_DEASSERT_TIMES_0_REG = 
{
    "P3_SIG_DEASSERT_TIMES_0",
#if RU_INCLUDE_DESC
    "P3 Port Reset Signal Deassertion Times Configuration Register 0",
    "",
#endif
    XPORT_PORTRESET_P3_SIG_DEASSERT_TIMES_0_REG_OFFSET,
    0,
    0,
    278,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    XPORT_PORTRESET_P3_SIG_DEASSERT_TIMES_0_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: XPORT_PORTRESET_P3_SIG_DEASSERT_TIMES_1
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XPORT_PORTRESET_P3_SIG_DEASSERT_TIMES_1_FIELDS[] =
{
    &XPORT_PORTRESET_P3_SIG_DEASSERT_TIMES_1_XLMAC_TXDISCARD_DEASSERT_TIME_FIELD,
    &XPORT_PORTRESET_P3_SIG_DEASSERT_TIMES_1_XLMAC_SOFT_RESET_DEASSERT_TIME_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec XPORT_PORTRESET_P3_SIG_DEASSERT_TIMES_1_REG = 
{
    "P3_SIG_DEASSERT_TIMES_1",
#if RU_INCLUDE_DESC
    "P3 Port Reset Signal Deassertion Times Configuration Register 1",
    "",
#endif
    XPORT_PORTRESET_P3_SIG_DEASSERT_TIMES_1_REG_OFFSET,
    0,
    0,
    279,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    XPORT_PORTRESET_P3_SIG_DEASSERT_TIMES_1_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: XPORT_PORTRESET_P3_SIG_DEASSERT_TIMES_2
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XPORT_PORTRESET_P3_SIG_DEASSERT_TIMES_2_FIELDS[] =
{
    &XPORT_PORTRESET_P3_SIG_DEASSERT_TIMES_2_MAB_RX_PORT_INIT_DEASSERT_TIME_FIELD,
    &XPORT_PORTRESET_P3_SIG_DEASSERT_TIMES_2_MAB_TX_PORT_INIT_DEASSERT_TIME_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec XPORT_PORTRESET_P3_SIG_DEASSERT_TIMES_2_REG = 
{
    "P3_SIG_DEASSERT_TIMES_2",
#if RU_INCLUDE_DESC
    "P3 Port Reset Signal Deassertion Times Configuration Register 2",
    "",
#endif
    XPORT_PORTRESET_P3_SIG_DEASSERT_TIMES_2_REG_OFFSET,
    0,
    0,
    280,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    XPORT_PORTRESET_P3_SIG_DEASSERT_TIMES_2_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: XPORT_PORTRESET_P3_SIG_DEASSERT_TIMES_3
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XPORT_PORTRESET_P3_SIG_DEASSERT_TIMES_3_FIELDS[] =
{
    &XPORT_PORTRESET_P3_SIG_DEASSERT_TIMES_3_MAB_TX_CREDIT_DISAB_DEASSERT_TIME_FIELD,
    &XPORT_PORTRESET_P3_SIG_DEASSERT_TIMES_3_MAB_TX_FIFO_INIT_DEASSERT_TIME_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec XPORT_PORTRESET_P3_SIG_DEASSERT_TIMES_3_REG = 
{
    "P3_SIG_DEASSERT_TIMES_3",
#if RU_INCLUDE_DESC
    "P3 Port Reset Signal Deassertion Times Configuration Register 3",
    "",
#endif
    XPORT_PORTRESET_P3_SIG_DEASSERT_TIMES_3_REG_OFFSET,
    0,
    0,
    281,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    XPORT_PORTRESET_P3_SIG_DEASSERT_TIMES_3_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: XPORT_PORTRESET_P3_SIG_DEASSERT_TIMES_4
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XPORT_PORTRESET_P3_SIG_DEASSERT_TIMES_4_FIELDS[] =
{
    &XPORT_PORTRESET_P3_SIG_DEASSERT_TIMES_4_PORT_IS_UNDER_RESET_DEASSERT_TIME_FIELD,
    &XPORT_PORTRESET_P3_SIG_DEASSERT_TIMES_4_RESERVED0_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec XPORT_PORTRESET_P3_SIG_DEASSERT_TIMES_4_REG = 
{
    "P3_SIG_DEASSERT_TIMES_4",
#if RU_INCLUDE_DESC
    "P3 Port Reset Signal Deassertion Times Configuration Register 4",
    "",
#endif
    XPORT_PORTRESET_P3_SIG_DEASSERT_TIMES_4_REG_OFFSET,
    0,
    0,
    282,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    XPORT_PORTRESET_P3_SIG_DEASSERT_TIMES_4_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: XPORT_PORTRESET_DEBUG
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XPORT_PORTRESET_DEBUG_FIELDS[] =
{
    &XPORT_PORTRESET_DEBUG_RESERVED0_FIELD,
    &XPORT_PORTRESET_DEBUG_P3_SM_STATE_FIELD,
    &XPORT_PORTRESET_DEBUG_P2_SM_STATE_FIELD,
    &XPORT_PORTRESET_DEBUG_P1_SM_STATE_FIELD,
    &XPORT_PORTRESET_DEBUG_P0_SM_STATE_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec XPORT_PORTRESET_DEBUG_REG = 
{
    "DEBUG",
#if RU_INCLUDE_DESC
    "Port Reset Debug Register",
    "",
#endif
    XPORT_PORTRESET_DEBUG_REG_OFFSET,
    0,
    0,
    283,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    5,
    XPORT_PORTRESET_DEBUG_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Block: XPORT_PORTRESET
 ******************************************************************************/
static const ru_reg_rec *XPORT_PORTRESET_REGS[] =
{
    &XPORT_PORTRESET_P0_CTRL_REG,
    &XPORT_PORTRESET_P1_CTRL_REG,
    &XPORT_PORTRESET_P2_CTRL_REG,
    &XPORT_PORTRESET_P3_CTRL_REG,
    &XPORT_PORTRESET_CONFIG_REG,
    &XPORT_PORTRESET_P0_LINK_STAT_DEBOUNCE_CFG_REG,
    &XPORT_PORTRESET_P1_LINK_STAT_DEBOUNCE_CFG_REG,
    &XPORT_PORTRESET_P2_LINK_STAT_DEBOUNCE_CFG_REG,
    &XPORT_PORTRESET_P3_LINK_STAT_DEBOUNCE_CFG_REG,
    &XPORT_PORTRESET_P0_SIG_EN_REG,
    &XPORT_PORTRESET_P0_SIG_ASSERT_TIMES_0_REG,
    &XPORT_PORTRESET_P0_SIG_ASSERT_TIMES_1_REG,
    &XPORT_PORTRESET_P0_SIG_ASSERT_TIMES_2_REG,
    &XPORT_PORTRESET_P0_SIG_ASSERT_TIMES_3_REG,
    &XPORT_PORTRESET_P0_SIG_ASSERT_TIMES_4_REG,
    &XPORT_PORTRESET_P0_SIG_DEASSERT_TIMES_0_REG,
    &XPORT_PORTRESET_P0_SIG_DEASSERT_TIMES_1_REG,
    &XPORT_PORTRESET_P0_SIG_DEASSERT_TIMES_2_REG,
    &XPORT_PORTRESET_P0_SIG_DEASSERT_TIMES_3_REG,
    &XPORT_PORTRESET_P0_SIG_DEASSERT_TIMES_4_REG,
    &XPORT_PORTRESET_P1_SIG_EN_REG,
    &XPORT_PORTRESET_P1_SIG_ASSERT_TIMES_0_REG,
    &XPORT_PORTRESET_P1_SIG_ASSERT_TIMES_1_REG,
    &XPORT_PORTRESET_P1_SIG_ASSERT_TIMES_2_REG,
    &XPORT_PORTRESET_P1_SIG_ASSERT_TIMES_3_REG,
    &XPORT_PORTRESET_P1_SIG_ASSERT_TIMES_4_REG,
    &XPORT_PORTRESET_P1_SIG_DEASSERT_TIMES_0_REG,
    &XPORT_PORTRESET_P1_SIG_DEASSERT_TIMES_1_REG,
    &XPORT_PORTRESET_P1_SIG_DEASSERT_TIMES_2_REG,
    &XPORT_PORTRESET_P1_SIG_DEASSERT_TIMES_3_REG,
    &XPORT_PORTRESET_P1_SIG_DEASSERT_TIMES_4_REG,
    &XPORT_PORTRESET_P2_SIG_EN_REG,
    &XPORT_PORTRESET_P2_SIG_ASSERT_TIMES_0_REG,
    &XPORT_PORTRESET_P2_SIG_ASSERT_TIMES_1_REG,
    &XPORT_PORTRESET_P2_SIG_ASSERT_TIMES_2_REG,
    &XPORT_PORTRESET_P2_SIG_ASSERT_TIMES_3_REG,
    &XPORT_PORTRESET_P2_SIG_ASSERT_TIMES_4_REG,
    &XPORT_PORTRESET_P2_SIG_DEASSERT_TIMES_0_REG,
    &XPORT_PORTRESET_P2_SIG_DEASSERT_TIMES_1_REG,
    &XPORT_PORTRESET_P2_SIG_DEASSERT_TIMES_2_REG,
    &XPORT_PORTRESET_P2_SIG_DEASSERT_TIMES_3_REG,
    &XPORT_PORTRESET_P2_SIG_DEASSERT_TIMES_4_REG,
    &XPORT_PORTRESET_P3_SIG_EN_REG,
    &XPORT_PORTRESET_P3_SIG_ASSERT_TIMES_0_REG,
    &XPORT_PORTRESET_P3_SIG_ASSERT_TIMES_1_REG,
    &XPORT_PORTRESET_P3_SIG_ASSERT_TIMES_2_REG,
    &XPORT_PORTRESET_P3_SIG_ASSERT_TIMES_3_REG,
    &XPORT_PORTRESET_P3_SIG_ASSERT_TIMES_4_REG,
    &XPORT_PORTRESET_P3_SIG_DEASSERT_TIMES_0_REG,
    &XPORT_PORTRESET_P3_SIG_DEASSERT_TIMES_1_REG,
    &XPORT_PORTRESET_P3_SIG_DEASSERT_TIMES_2_REG,
    &XPORT_PORTRESET_P3_SIG_DEASSERT_TIMES_3_REG,
    &XPORT_PORTRESET_P3_SIG_DEASSERT_TIMES_4_REG,
    &XPORT_PORTRESET_DEBUG_REG,
};

unsigned long XPORT_PORTRESET_ADDRS[] =
{
    0x837f3400,
    0x837f7400,
};

const ru_block_rec XPORT_PORTRESET_BLOCK = 
{
    "XPORT_PORTRESET",
    XPORT_PORTRESET_ADDRS,
    2,
    54,
    XPORT_PORTRESET_REGS
};

/* End of file XPORT_PORTRESET.c */
