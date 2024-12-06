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
 * Field: XPORT_INTR_CPU_STATUS_RESERVED0
 ******************************************************************************/
const ru_field_rec XPORT_INTR_CPU_STATUS_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    XPORT_INTR_CPU_STATUS_RESERVED0_FIELD_MASK,
    0,
    XPORT_INTR_CPU_STATUS_RESERVED0_FIELD_WIDTH,
    XPORT_INTR_CPU_STATUS_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_INTR_CPU_STATUS_MAB_STATUS_INTR
 ******************************************************************************/
const ru_field_rec XPORT_INTR_CPU_STATUS_MAB_STATUS_INTR_FIELD =
{
    "MAB_STATUS_INTR",
#if RU_INCLUDE_DESC
    "",
    "Asserted when any of statuses get set in MSBUS Adaptation Block Status Register(s).",
#endif
    XPORT_INTR_CPU_STATUS_MAB_STATUS_INTR_FIELD_MASK,
    0,
    XPORT_INTR_CPU_STATUS_MAB_STATUS_INTR_FIELD_WIDTH,
    XPORT_INTR_CPU_STATUS_MAB_STATUS_INTR_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_INTR_CPU_STATUS_RX_REMOTE_FAULT_INTR
 ******************************************************************************/
const ru_field_rec XPORT_INTR_CPU_STATUS_RX_REMOTE_FAULT_INTR_FIELD =
{
    "RX_REMOTE_FAULT_INTR",
#if RU_INCLUDE_DESC
    "",
    "RX Remote Fault Interrupt for P0. Valid only when port is in 10G mode",
#endif
    XPORT_INTR_CPU_STATUS_RX_REMOTE_FAULT_INTR_FIELD_MASK,
    0,
    XPORT_INTR_CPU_STATUS_RX_REMOTE_FAULT_INTR_FIELD_WIDTH,
    XPORT_INTR_CPU_STATUS_RX_REMOTE_FAULT_INTR_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_INTR_CPU_STATUS_SERDES_MOD_DEF0_EVENT_INTR
 ******************************************************************************/
const ru_field_rec XPORT_INTR_CPU_STATUS_SERDES_MOD_DEF0_EVENT_INTR_FIELD =
{
    "SERDES_MOD_DEF0_EVENT_INTR",
#if RU_INCLUDE_DESC
    "",
    "Asserted when a change in optical module presence is detected.\n"
    "serdes_mod_def0_event_intr[1] corresponds to SGMII SERDES.\n"
    "serdes_mod_def0_event_intr[0] corresponds to PON AE SERDES.\n"
    "The corresponding status register should be read to determine whether the interrupt is caused by the optical module insertion or removal.",
#endif
    XPORT_INTR_CPU_STATUS_SERDES_MOD_DEF0_EVENT_INTR_FIELD_MASK,
    0,
    XPORT_INTR_CPU_STATUS_SERDES_MOD_DEF0_EVENT_INTR_FIELD_WIDTH,
    XPORT_INTR_CPU_STATUS_SERDES_MOD_DEF0_EVENT_INTR_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_INTR_CPU_STATUS_DSERDES_SD_OFF_INTR
 ******************************************************************************/
const ru_field_rec XPORT_INTR_CPU_STATUS_DSERDES_SD_OFF_INTR_FIELD =
{
    "DSERDES_SD_OFF_INTR",
#if RU_INCLUDE_DESC
    "",
    "Inverted sd_on interrupt. Interrupt to serdes lane mapping is same as for dserdes_sd_on_intr[1:0]. "
    "Based on filtered signal detect.",
#endif
    XPORT_INTR_CPU_STATUS_DSERDES_SD_OFF_INTR_FIELD_MASK,
    0,
    XPORT_INTR_CPU_STATUS_DSERDES_SD_OFF_INTR_FIELD_WIDTH,
    XPORT_INTR_CPU_STATUS_DSERDES_SD_OFF_INTR_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_INTR_CPU_STATUS_DSERDES_SD_ON_INTR
 ******************************************************************************/
const ru_field_rec XPORT_INTR_CPU_STATUS_DSERDES_SD_ON_INTR_FIELD =
{
    "DSERDES_SD_ON_INTR",
#if RU_INCLUDE_DESC
    "",
    "Active high signal detect interrupt (signals presence of the link partner).\n"
    "dserdes_sd_on_intr[1] corresponds to 2.5G SGMII SERDES.\n"
    "dserdes_sd_on_intr[0] corresponds to PON AE SERDES.\n"
    "Based on filtered signal detect.",
#endif
    XPORT_INTR_CPU_STATUS_DSERDES_SD_ON_INTR_FIELD_MASK,
    0,
    XPORT_INTR_CPU_STATUS_DSERDES_SD_ON_INTR_FIELD_WIDTH,
    XPORT_INTR_CPU_STATUS_DSERDES_SD_ON_INTR_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_INTR_CPU_STATUS_LINK_DOWN_INTR
 ******************************************************************************/
const ru_field_rec XPORT_INTR_CPU_STATUS_LINK_DOWN_INTR_FIELD =
{
    "LINK_DOWN_INTR",
#if RU_INCLUDE_DESC
    "",
    "Link down (i.e. link interruption) interrupt for P1:P0. Interrupt corresponds to the attached PHY/interface link down event.",
#endif
    XPORT_INTR_CPU_STATUS_LINK_DOWN_INTR_FIELD_MASK,
    0,
    XPORT_INTR_CPU_STATUS_LINK_DOWN_INTR_FIELD_WIDTH,
    XPORT_INTR_CPU_STATUS_LINK_DOWN_INTR_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_INTR_CPU_STATUS_LINK_UP_INTR
 ******************************************************************************/
const ru_field_rec XPORT_INTR_CPU_STATUS_LINK_UP_INTR_FIELD =
{
    "LINK_UP_INTR",
#if RU_INCLUDE_DESC
    "",
    "Link up interrupt for P1:P0. Interrupt corresponds to the attached PHY/interface link up event.",
#endif
    XPORT_INTR_CPU_STATUS_LINK_UP_INTR_FIELD_MASK,
    0,
    XPORT_INTR_CPU_STATUS_LINK_UP_INTR_FIELD_WIDTH,
    XPORT_INTR_CPU_STATUS_LINK_UP_INTR_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_INTR_CPU_STATUS_XLMAC_INTR
 ******************************************************************************/
const ru_field_rec XPORT_INTR_CPU_STATUS_XLMAC_INTR_FIELD =
{
    "XLMAC_INTR",
#if RU_INCLUDE_DESC
    "",
    "Interrupt generated by XLMAC signaling various events as described in XLMAC_INTR_STATUS."
    "Bits [1:0] correspond to XLMAC0 ports 1 and 0.",
#endif
    XPORT_INTR_CPU_STATUS_XLMAC_INTR_FIELD_MASK,
    0,
    XPORT_INTR_CPU_STATUS_XLMAC_INTR_FIELD_WIDTH,
    XPORT_INTR_CPU_STATUS_XLMAC_INTR_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_INTR_CPU_STATUS_TX_TIMESYNC_FIFO_ENTRY_VALID_INTR
 ******************************************************************************/
const ru_field_rec XPORT_INTR_CPU_STATUS_TX_TIMESYNC_FIFO_ENTRY_VALID_INTR_FIELD =
{
    "TX_TIMESYNC_FIFO_ENTRY_VALID_INTR",
#if RU_INCLUDE_DESC
    "",
    "tx_timesync_fifo_entry_valid_intr[1:0].\n"
    "PTP timestamp available for read. Bits [1:0] correspond to XLMAC0 ports 1 and 0.",
#endif
    XPORT_INTR_CPU_STATUS_TX_TIMESYNC_FIFO_ENTRY_VALID_INTR_FIELD_MASK,
    0,
    XPORT_INTR_CPU_STATUS_TX_TIMESYNC_FIFO_ENTRY_VALID_INTR_FIELD_WIDTH,
    XPORT_INTR_CPU_STATUS_TX_TIMESYNC_FIFO_ENTRY_VALID_INTR_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_INTR_CPU_STATUS_SGPHY_ENERGY_OFF_INTR
 ******************************************************************************/
const ru_field_rec XPORT_INTR_CPU_STATUS_SGPHY_ENERGY_OFF_INTR_FIELD =
{
    "SGPHY_ENERGY_OFF_INTR",
#if RU_INCLUDE_DESC
    "",
    "Active high energy detect interrupt signal (signals presence of the link partner) asserted "
    "when SGPHYs detects loss of energy. Based on SGPHY's energy_det_apd.",
#endif
    XPORT_INTR_CPU_STATUS_SGPHY_ENERGY_OFF_INTR_FIELD_MASK,
    0,
    XPORT_INTR_CPU_STATUS_SGPHY_ENERGY_OFF_INTR_FIELD_WIDTH,
    XPORT_INTR_CPU_STATUS_SGPHY_ENERGY_OFF_INTR_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_INTR_CPU_STATUS_SGPHY_ENERGY_ON_INTR
 ******************************************************************************/
const ru_field_rec XPORT_INTR_CPU_STATUS_SGPHY_ENERGY_ON_INTR_FIELD =
{
    "SGPHY_ENERGY_ON_INTR",
#if RU_INCLUDE_DESC
    "",
    "Active high energy detect interrupt signal (signals presence of the link partner) asserted "
    "when SGPHYs detects the energy. Based on SGPHY's energy_det_apd.",
#endif
    XPORT_INTR_CPU_STATUS_SGPHY_ENERGY_ON_INTR_FIELD_MASK,
    0,
    XPORT_INTR_CPU_STATUS_SGPHY_ENERGY_ON_INTR_FIELD_WIDTH,
    XPORT_INTR_CPU_STATUS_SGPHY_ENERGY_ON_INTR_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_INTR_CPU_STATUS_MIB_REG_ERR_INTR
 ******************************************************************************/
const ru_field_rec XPORT_INTR_CPU_STATUS_MIB_REG_ERR_INTR_FIELD =
{
    "MIB_REG_ERR_INTR",
#if RU_INCLUDE_DESC
    "",
    "mib_reg_err_intr.\n"
    "MIB register transaction error.",
#endif
    XPORT_INTR_CPU_STATUS_MIB_REG_ERR_INTR_FIELD_MASK,
    0,
    XPORT_INTR_CPU_STATUS_MIB_REG_ERR_INTR_FIELD_WIDTH,
    XPORT_INTR_CPU_STATUS_MIB_REG_ERR_INTR_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_INTR_CPU_STATUS_MAC_REG_ERR_INTR
 ******************************************************************************/
const ru_field_rec XPORT_INTR_CPU_STATUS_MAC_REG_ERR_INTR_FIELD =
{
    "MAC_REG_ERR_INTR",
#if RU_INCLUDE_DESC
    "",
    "mac_reg_err_intr.\n"
    "XLMAC register transaction error",
#endif
    XPORT_INTR_CPU_STATUS_MAC_REG_ERR_INTR_FIELD_MASK,
    0,
    XPORT_INTR_CPU_STATUS_MAC_REG_ERR_INTR_FIELD_WIDTH,
    XPORT_INTR_CPU_STATUS_MAC_REG_ERR_INTR_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_INTR_CPU_STATUS_UBUS_ERR_INTR
 ******************************************************************************/
const ru_field_rec XPORT_INTR_CPU_STATUS_UBUS_ERR_INTR_FIELD =
{
    "UBUS_ERR_INTR",
#if RU_INCLUDE_DESC
    "",
    "UBUS transaction error.",
#endif
    XPORT_INTR_CPU_STATUS_UBUS_ERR_INTR_FIELD_MASK,
    0,
    XPORT_INTR_CPU_STATUS_UBUS_ERR_INTR_FIELD_WIDTH,
    XPORT_INTR_CPU_STATUS_UBUS_ERR_INTR_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_INTR_CPU_SET_RESERVED0
 ******************************************************************************/
const ru_field_rec XPORT_INTR_CPU_SET_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    XPORT_INTR_CPU_SET_RESERVED0_FIELD_MASK,
    0,
    XPORT_INTR_CPU_SET_RESERVED0_FIELD_WIDTH,
    XPORT_INTR_CPU_SET_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_INTR_CPU_SET_MAB_STATUS_INTR
 ******************************************************************************/
const ru_field_rec XPORT_INTR_CPU_SET_MAB_STATUS_INTR_FIELD =
{
    "MAB_STATUS_INTR",
#if RU_INCLUDE_DESC
    "",
    "Asserted when any of statuses get set in MSBUS Adaptation Block Status Register(s).",
#endif
    XPORT_INTR_CPU_SET_MAB_STATUS_INTR_FIELD_MASK,
    0,
    XPORT_INTR_CPU_SET_MAB_STATUS_INTR_FIELD_WIDTH,
    XPORT_INTR_CPU_SET_MAB_STATUS_INTR_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_INTR_CPU_SET_RX_REMOTE_FAULT_INTR
 ******************************************************************************/
const ru_field_rec XPORT_INTR_CPU_SET_RX_REMOTE_FAULT_INTR_FIELD =
{
    "RX_REMOTE_FAULT_INTR",
#if RU_INCLUDE_DESC
    "",
    "RX Remote Fault Interrupt for P0. Valid only when port is in 10G mode",
#endif
    XPORT_INTR_CPU_SET_RX_REMOTE_FAULT_INTR_FIELD_MASK,
    0,
    XPORT_INTR_CPU_SET_RX_REMOTE_FAULT_INTR_FIELD_WIDTH,
    XPORT_INTR_CPU_SET_RX_REMOTE_FAULT_INTR_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_INTR_CPU_SET_SERDES_MOD_DEF0_EVENT_INTR
 ******************************************************************************/
const ru_field_rec XPORT_INTR_CPU_SET_SERDES_MOD_DEF0_EVENT_INTR_FIELD =
{
    "SERDES_MOD_DEF0_EVENT_INTR",
#if RU_INCLUDE_DESC
    "",
    "Asserted when a change in optical module presence is detected.\n"
    "serdes_mod_def0_event_intr[1] corresponds to SGMII SERDES.\n"
    "serdes_mod_def0_event_intr[0] corresponds to PON AE SERDES.\n"
    "The corresponding status register should be read to determine whether the interrupt is caused by the optical module insertion or removal.",
#endif
    XPORT_INTR_CPU_SET_SERDES_MOD_DEF0_EVENT_INTR_FIELD_MASK,
    0,
    XPORT_INTR_CPU_SET_SERDES_MOD_DEF0_EVENT_INTR_FIELD_WIDTH,
    XPORT_INTR_CPU_SET_SERDES_MOD_DEF0_EVENT_INTR_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_INTR_CPU_SET_DSERDES_SD_OFF_INTR
 ******************************************************************************/
const ru_field_rec XPORT_INTR_CPU_SET_DSERDES_SD_OFF_INTR_FIELD =
{
    "DSERDES_SD_OFF_INTR",
#if RU_INCLUDE_DESC
    "",
    "Inverted sd_on interrupt. Interrupt to serdes lane mapping is same as for dserdes_sd_on_intr[1:0]. "
    "Based on filtered signal detect.",
#endif
    XPORT_INTR_CPU_SET_DSERDES_SD_OFF_INTR_FIELD_MASK,
    0,
    XPORT_INTR_CPU_SET_DSERDES_SD_OFF_INTR_FIELD_WIDTH,
    XPORT_INTR_CPU_SET_DSERDES_SD_OFF_INTR_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_INTR_CPU_SET_DSERDES_SD_ON_INTR
 ******************************************************************************/
const ru_field_rec XPORT_INTR_CPU_SET_DSERDES_SD_ON_INTR_FIELD =
{
    "DSERDES_SD_ON_INTR",
#if RU_INCLUDE_DESC
    "",
    "Active high signal detect interrupt (signals presence of the link partner).\n"
    "dserdes_sd_on_intr[1] corresponds to 2.5G SGMII SERDES.\n"
    "dserdes_sd_on_intr[0] corresponds to PON AE SERDES.\n"
    "Based on filtered signal detect.",
#endif
    XPORT_INTR_CPU_SET_DSERDES_SD_ON_INTR_FIELD_MASK,
    0,
    XPORT_INTR_CPU_SET_DSERDES_SD_ON_INTR_FIELD_WIDTH,
    XPORT_INTR_CPU_SET_DSERDES_SD_ON_INTR_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_INTR_CPU_SET_LINK_DOWN_INTR
 ******************************************************************************/
const ru_field_rec XPORT_INTR_CPU_SET_LINK_DOWN_INTR_FIELD =
{
    "LINK_DOWN_INTR",
#if RU_INCLUDE_DESC
    "",
    "Link down (i.e. link interruption) interrupt for P1:P0. Interrupt corresponds to the attached PHY/interface link down event.",
#endif
    XPORT_INTR_CPU_SET_LINK_DOWN_INTR_FIELD_MASK,
    0,
    XPORT_INTR_CPU_SET_LINK_DOWN_INTR_FIELD_WIDTH,
    XPORT_INTR_CPU_SET_LINK_DOWN_INTR_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_INTR_CPU_SET_LINK_UP_INTR
 ******************************************************************************/
const ru_field_rec XPORT_INTR_CPU_SET_LINK_UP_INTR_FIELD =
{
    "LINK_UP_INTR",
#if RU_INCLUDE_DESC
    "",
    "Link up interrupt for P1:P0. Interrupt corresponds to the attached PHY/interface link up event.",
#endif
    XPORT_INTR_CPU_SET_LINK_UP_INTR_FIELD_MASK,
    0,
    XPORT_INTR_CPU_SET_LINK_UP_INTR_FIELD_WIDTH,
    XPORT_INTR_CPU_SET_LINK_UP_INTR_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_INTR_CPU_SET_XLMAC_INTR
 ******************************************************************************/
const ru_field_rec XPORT_INTR_CPU_SET_XLMAC_INTR_FIELD =
{
    "XLMAC_INTR",
#if RU_INCLUDE_DESC
    "",
    "Interrupt generated by XLMAC signaling various events as described in XLMAC_INTR_STATUS."
    "Bits [1:0] correspond to XLMAC0 ports 1 and 0.",
#endif
    XPORT_INTR_CPU_SET_XLMAC_INTR_FIELD_MASK,
    0,
    XPORT_INTR_CPU_SET_XLMAC_INTR_FIELD_WIDTH,
    XPORT_INTR_CPU_SET_XLMAC_INTR_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_INTR_CPU_SET_TX_TIMESYNC_FIFO_ENTRY_VALID_INTR
 ******************************************************************************/
const ru_field_rec XPORT_INTR_CPU_SET_TX_TIMESYNC_FIFO_ENTRY_VALID_INTR_FIELD =
{
    "TX_TIMESYNC_FIFO_ENTRY_VALID_INTR",
#if RU_INCLUDE_DESC
    "",
    "tx_timesync_fifo_entry_valid_intr[1:0].\n"
    "PTP timestamp available for read. Bits [1:0] correspond to XLMAC0 ports 1 and 0.",
#endif
    XPORT_INTR_CPU_SET_TX_TIMESYNC_FIFO_ENTRY_VALID_INTR_FIELD_MASK,
    0,
    XPORT_INTR_CPU_SET_TX_TIMESYNC_FIFO_ENTRY_VALID_INTR_FIELD_WIDTH,
    XPORT_INTR_CPU_SET_TX_TIMESYNC_FIFO_ENTRY_VALID_INTR_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_INTR_CPU_SET_SGPHY_ENERGY_OFF_INTR
 ******************************************************************************/
const ru_field_rec XPORT_INTR_CPU_SET_SGPHY_ENERGY_OFF_INTR_FIELD =
{
    "SGPHY_ENERGY_OFF_INTR",
#if RU_INCLUDE_DESC
    "",
    "Active high energy detect interrupt signal (signals presence of the link partner) asserted "
    "when SGPHYs detects loss of energy. Based on SGPHY's energy_det_apd.",
#endif
    XPORT_INTR_CPU_SET_SGPHY_ENERGY_OFF_INTR_FIELD_MASK,
    0,
    XPORT_INTR_CPU_SET_SGPHY_ENERGY_OFF_INTR_FIELD_WIDTH,
    XPORT_INTR_CPU_SET_SGPHY_ENERGY_OFF_INTR_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_INTR_CPU_SET_SGPHY_ENERGY_ON_INTR
 ******************************************************************************/
const ru_field_rec XPORT_INTR_CPU_SET_SGPHY_ENERGY_ON_INTR_FIELD =
{
    "SGPHY_ENERGY_ON_INTR",
#if RU_INCLUDE_DESC
    "",
    "Active high energy detect interrupt signal (signals presence of the link partner) asserted "
    "when SGPHYs detects the energy. Based on SGPHY's energy_det_apd.",
#endif
    XPORT_INTR_CPU_SET_SGPHY_ENERGY_ON_INTR_FIELD_MASK,
    0,
    XPORT_INTR_CPU_SET_SGPHY_ENERGY_ON_INTR_FIELD_WIDTH,
    XPORT_INTR_CPU_SET_SGPHY_ENERGY_ON_INTR_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_INTR_CPU_SET_MIB_REG_ERR_INTR
 ******************************************************************************/
const ru_field_rec XPORT_INTR_CPU_SET_MIB_REG_ERR_INTR_FIELD =
{
    "MIB_REG_ERR_INTR",
#if RU_INCLUDE_DESC
    "",
    "mib_reg_err_intr.\n"
    "MIB register transaction error.",
#endif
    XPORT_INTR_CPU_SET_MIB_REG_ERR_INTR_FIELD_MASK,
    0,
    XPORT_INTR_CPU_SET_MIB_REG_ERR_INTR_FIELD_WIDTH,
    XPORT_INTR_CPU_SET_MIB_REG_ERR_INTR_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_INTR_CPU_SET_MAC_REG_ERR_INTR
 ******************************************************************************/
const ru_field_rec XPORT_INTR_CPU_SET_MAC_REG_ERR_INTR_FIELD =
{
    "MAC_REG_ERR_INTR",
#if RU_INCLUDE_DESC
    "",
    "mac_reg_err_intr.\n"
    "XLMAC register transaction error",
#endif
    XPORT_INTR_CPU_SET_MAC_REG_ERR_INTR_FIELD_MASK,
    0,
    XPORT_INTR_CPU_SET_MAC_REG_ERR_INTR_FIELD_WIDTH,
    XPORT_INTR_CPU_SET_MAC_REG_ERR_INTR_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_INTR_CPU_SET_UBUS_ERR_INTR
 ******************************************************************************/
const ru_field_rec XPORT_INTR_CPU_SET_UBUS_ERR_INTR_FIELD =
{
    "UBUS_ERR_INTR",
#if RU_INCLUDE_DESC
    "",
    "UBUS transaction error.",
#endif
    XPORT_INTR_CPU_SET_UBUS_ERR_INTR_FIELD_MASK,
    0,
    XPORT_INTR_CPU_SET_UBUS_ERR_INTR_FIELD_WIDTH,
    XPORT_INTR_CPU_SET_UBUS_ERR_INTR_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_INTR_CPU_CLEAR_RESERVED0
 ******************************************************************************/
const ru_field_rec XPORT_INTR_CPU_CLEAR_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    XPORT_INTR_CPU_CLEAR_RESERVED0_FIELD_MASK,
    0,
    XPORT_INTR_CPU_CLEAR_RESERVED0_FIELD_WIDTH,
    XPORT_INTR_CPU_CLEAR_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_INTR_CPU_CLEAR_MAB_STATUS_INTR
 ******************************************************************************/
const ru_field_rec XPORT_INTR_CPU_CLEAR_MAB_STATUS_INTR_FIELD =
{
    "MAB_STATUS_INTR",
#if RU_INCLUDE_DESC
    "",
    "Asserted when any of statuses get set in MSBUS Adaptation Block Status Register(s).",
#endif
    XPORT_INTR_CPU_CLEAR_MAB_STATUS_INTR_FIELD_MASK,
    0,
    XPORT_INTR_CPU_CLEAR_MAB_STATUS_INTR_FIELD_WIDTH,
    XPORT_INTR_CPU_CLEAR_MAB_STATUS_INTR_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_INTR_CPU_CLEAR_RX_REMOTE_FAULT_INTR
 ******************************************************************************/
const ru_field_rec XPORT_INTR_CPU_CLEAR_RX_REMOTE_FAULT_INTR_FIELD =
{
    "RX_REMOTE_FAULT_INTR",
#if RU_INCLUDE_DESC
    "",
    "RX Remote Fault Interrupt for P0. Valid only when port is in 10G mode",
#endif
    XPORT_INTR_CPU_CLEAR_RX_REMOTE_FAULT_INTR_FIELD_MASK,
    0,
    XPORT_INTR_CPU_CLEAR_RX_REMOTE_FAULT_INTR_FIELD_WIDTH,
    XPORT_INTR_CPU_CLEAR_RX_REMOTE_FAULT_INTR_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_INTR_CPU_CLEAR_SERDES_MOD_DEF0_EVENT_INTR
 ******************************************************************************/
const ru_field_rec XPORT_INTR_CPU_CLEAR_SERDES_MOD_DEF0_EVENT_INTR_FIELD =
{
    "SERDES_MOD_DEF0_EVENT_INTR",
#if RU_INCLUDE_DESC
    "",
    "Asserted when a change in optical module presence is detected.\n"
    "serdes_mod_def0_event_intr[1] corresponds to SGMII SERDES.\n"
    "serdes_mod_def0_event_intr[0] corresponds to PON AE SERDES.\n"
    "The corresponding status register should be read to determine whether the interrupt is caused by the optical module insertion or removal.",
#endif
    XPORT_INTR_CPU_CLEAR_SERDES_MOD_DEF0_EVENT_INTR_FIELD_MASK,
    0,
    XPORT_INTR_CPU_CLEAR_SERDES_MOD_DEF0_EVENT_INTR_FIELD_WIDTH,
    XPORT_INTR_CPU_CLEAR_SERDES_MOD_DEF0_EVENT_INTR_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_INTR_CPU_CLEAR_DSERDES_SD_OFF_INTR
 ******************************************************************************/
const ru_field_rec XPORT_INTR_CPU_CLEAR_DSERDES_SD_OFF_INTR_FIELD =
{
    "DSERDES_SD_OFF_INTR",
#if RU_INCLUDE_DESC
    "",
    "Inverted sd_on interrupt. Interrupt to serdes lane mapping is same as for dserdes_sd_on_intr[1:0]. "
    "Based on filtered signal detect.",
#endif
    XPORT_INTR_CPU_CLEAR_DSERDES_SD_OFF_INTR_FIELD_MASK,
    0,
    XPORT_INTR_CPU_CLEAR_DSERDES_SD_OFF_INTR_FIELD_WIDTH,
    XPORT_INTR_CPU_CLEAR_DSERDES_SD_OFF_INTR_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_INTR_CPU_CLEAR_DSERDES_SD_ON_INTR
 ******************************************************************************/
const ru_field_rec XPORT_INTR_CPU_CLEAR_DSERDES_SD_ON_INTR_FIELD =
{
    "DSERDES_SD_ON_INTR",
#if RU_INCLUDE_DESC
    "",
    "Active high signal detect interrupt (signals presence of the link partner).\n"
    "dserdes_sd_on_intr[1] corresponds to 2.5G SGMII SERDES.\n"
    "dserdes_sd_on_intr[0] corresponds to PON AE SERDES.\n"
    "Based on filtered signal detect.",
#endif
    XPORT_INTR_CPU_CLEAR_DSERDES_SD_ON_INTR_FIELD_MASK,
    0,
    XPORT_INTR_CPU_CLEAR_DSERDES_SD_ON_INTR_FIELD_WIDTH,
    XPORT_INTR_CPU_CLEAR_DSERDES_SD_ON_INTR_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_INTR_CPU_CLEAR_LINK_DOWN_INTR
 ******************************************************************************/
const ru_field_rec XPORT_INTR_CPU_CLEAR_LINK_DOWN_INTR_FIELD =
{
    "LINK_DOWN_INTR",
#if RU_INCLUDE_DESC
    "",
    "Link down (i.e. link interruption) interrupt for P1:P0. Interrupt corresponds to the attached PHY/interface link down event.",
#endif
    XPORT_INTR_CPU_CLEAR_LINK_DOWN_INTR_FIELD_MASK,
    0,
    XPORT_INTR_CPU_CLEAR_LINK_DOWN_INTR_FIELD_WIDTH,
    XPORT_INTR_CPU_CLEAR_LINK_DOWN_INTR_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_INTR_CPU_CLEAR_LINK_UP_INTR
 ******************************************************************************/
const ru_field_rec XPORT_INTR_CPU_CLEAR_LINK_UP_INTR_FIELD =
{
    "LINK_UP_INTR",
#if RU_INCLUDE_DESC
    "",
    "Link up interrupt for P1:P0. Interrupt corresponds to the attached PHY/interface link up event.",
#endif
    XPORT_INTR_CPU_CLEAR_LINK_UP_INTR_FIELD_MASK,
    0,
    XPORT_INTR_CPU_CLEAR_LINK_UP_INTR_FIELD_WIDTH,
    XPORT_INTR_CPU_CLEAR_LINK_UP_INTR_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_INTR_CPU_CLEAR_XLMAC_INTR
 ******************************************************************************/
const ru_field_rec XPORT_INTR_CPU_CLEAR_XLMAC_INTR_FIELD =
{
    "XLMAC_INTR",
#if RU_INCLUDE_DESC
    "",
    "Interrupt generated by XLMAC signaling various events as described in XLMAC_INTR_STATUS."
    "Bits [1:0] correspond to XLMAC0 ports 1 and 0.",
#endif
    XPORT_INTR_CPU_CLEAR_XLMAC_INTR_FIELD_MASK,
    0,
    XPORT_INTR_CPU_CLEAR_XLMAC_INTR_FIELD_WIDTH,
    XPORT_INTR_CPU_CLEAR_XLMAC_INTR_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_INTR_CPU_CLEAR_TX_TIMESYNC_FIFO_ENTRY_VALID_INTR
 ******************************************************************************/
const ru_field_rec XPORT_INTR_CPU_CLEAR_TX_TIMESYNC_FIFO_ENTRY_VALID_INTR_FIELD =
{
    "TX_TIMESYNC_FIFO_ENTRY_VALID_INTR",
#if RU_INCLUDE_DESC
    "",
    "tx_timesync_fifo_entry_valid_intr[1:0].\n"
    "PTP timestamp available for read. Bits [1:0] correspond to XLMAC0 ports 1 and 0.",
#endif
    XPORT_INTR_CPU_CLEAR_TX_TIMESYNC_FIFO_ENTRY_VALID_INTR_FIELD_MASK,
    0,
    XPORT_INTR_CPU_CLEAR_TX_TIMESYNC_FIFO_ENTRY_VALID_INTR_FIELD_WIDTH,
    XPORT_INTR_CPU_CLEAR_TX_TIMESYNC_FIFO_ENTRY_VALID_INTR_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_INTR_CPU_CLEAR_SGPHY_ENERGY_OFF_INTR
 ******************************************************************************/
const ru_field_rec XPORT_INTR_CPU_CLEAR_SGPHY_ENERGY_OFF_INTR_FIELD =
{
    "SGPHY_ENERGY_OFF_INTR",
#if RU_INCLUDE_DESC
    "",
    "Active high energy detect interrupt signal (signals presence of the link partner) asserted "
    "when SGPHYs detects loss of energy. Based on SGPHY's energy_det_apd.",
#endif
    XPORT_INTR_CPU_CLEAR_SGPHY_ENERGY_OFF_INTR_FIELD_MASK,
    0,
    XPORT_INTR_CPU_CLEAR_SGPHY_ENERGY_OFF_INTR_FIELD_WIDTH,
    XPORT_INTR_CPU_CLEAR_SGPHY_ENERGY_OFF_INTR_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_INTR_CPU_CLEAR_SGPHY_ENERGY_ON_INTR
 ******************************************************************************/
const ru_field_rec XPORT_INTR_CPU_CLEAR_SGPHY_ENERGY_ON_INTR_FIELD =
{
    "SGPHY_ENERGY_ON_INTR",
#if RU_INCLUDE_DESC
    "",
    "Active high energy detect interrupt signal (signals presence of the link partner) asserted "
    "when SGPHYs detects the energy. Based on SGPHY's energy_det_apd.",
#endif
    XPORT_INTR_CPU_CLEAR_SGPHY_ENERGY_ON_INTR_FIELD_MASK,
    0,
    XPORT_INTR_CPU_CLEAR_SGPHY_ENERGY_ON_INTR_FIELD_WIDTH,
    XPORT_INTR_CPU_CLEAR_SGPHY_ENERGY_ON_INTR_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_INTR_CPU_CLEAR_MIB_REG_ERR_INTR
 ******************************************************************************/
const ru_field_rec XPORT_INTR_CPU_CLEAR_MIB_REG_ERR_INTR_FIELD =
{
    "MIB_REG_ERR_INTR",
#if RU_INCLUDE_DESC
    "",
    "mib_reg_err_intr.\n"
    "MIB register transaction error.",
#endif
    XPORT_INTR_CPU_CLEAR_MIB_REG_ERR_INTR_FIELD_MASK,
    0,
    XPORT_INTR_CPU_CLEAR_MIB_REG_ERR_INTR_FIELD_WIDTH,
    XPORT_INTR_CPU_CLEAR_MIB_REG_ERR_INTR_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_INTR_CPU_CLEAR_MAC_REG_ERR_INTR
 ******************************************************************************/
const ru_field_rec XPORT_INTR_CPU_CLEAR_MAC_REG_ERR_INTR_FIELD =
{
    "MAC_REG_ERR_INTR",
#if RU_INCLUDE_DESC
    "",
    "mac_reg_err_intr.\n"
    "XLMAC register transaction error",
#endif
    XPORT_INTR_CPU_CLEAR_MAC_REG_ERR_INTR_FIELD_MASK,
    0,
    XPORT_INTR_CPU_CLEAR_MAC_REG_ERR_INTR_FIELD_WIDTH,
    XPORT_INTR_CPU_CLEAR_MAC_REG_ERR_INTR_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_INTR_CPU_CLEAR_UBUS_ERR_INTR
 ******************************************************************************/
const ru_field_rec XPORT_INTR_CPU_CLEAR_UBUS_ERR_INTR_FIELD =
{
    "UBUS_ERR_INTR",
#if RU_INCLUDE_DESC
    "",
    "UBUS transaction error.",
#endif
    XPORT_INTR_CPU_CLEAR_UBUS_ERR_INTR_FIELD_MASK,
    0,
    XPORT_INTR_CPU_CLEAR_UBUS_ERR_INTR_FIELD_WIDTH,
    XPORT_INTR_CPU_CLEAR_UBUS_ERR_INTR_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_INTR_CPU_MASK_STATUS_RESERVED0
 ******************************************************************************/
const ru_field_rec XPORT_INTR_CPU_MASK_STATUS_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    XPORT_INTR_CPU_MASK_STATUS_RESERVED0_FIELD_MASK,
    0,
    XPORT_INTR_CPU_MASK_STATUS_RESERVED0_FIELD_WIDTH,
    XPORT_INTR_CPU_MASK_STATUS_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_INTR_CPU_MASK_STATUS_MAB_STATUS_INTR_MASK
 ******************************************************************************/
const ru_field_rec XPORT_INTR_CPU_MASK_STATUS_MAB_STATUS_INTR_MASK_FIELD =
{
    "MAB_STATUS_INTR_MASK",
#if RU_INCLUDE_DESC
    "",
    "This bit corresponds to the mask for msbus adaptation block status interrupt. See register description above for details of how to use this bit.",
#endif
    XPORT_INTR_CPU_MASK_STATUS_MAB_STATUS_INTR_MASK_FIELD_MASK,
    0,
    XPORT_INTR_CPU_MASK_STATUS_MAB_STATUS_INTR_MASK_FIELD_WIDTH,
    XPORT_INTR_CPU_MASK_STATUS_MAB_STATUS_INTR_MASK_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_INTR_CPU_MASK_STATUS_RX_REMOTE_FAULT_INTR_MASK
 ******************************************************************************/
const ru_field_rec XPORT_INTR_CPU_MASK_STATUS_RX_REMOTE_FAULT_INTR_MASK_FIELD =
{
    "RX_REMOTE_FAULT_INTR_MASK",
#if RU_INCLUDE_DESC
    "",
    "This bit corresponds to the mask for rx remote fault interrupt. See register description above for details of how to use this bit.",
#endif
    XPORT_INTR_CPU_MASK_STATUS_RX_REMOTE_FAULT_INTR_MASK_FIELD_MASK,
    0,
    XPORT_INTR_CPU_MASK_STATUS_RX_REMOTE_FAULT_INTR_MASK_FIELD_WIDTH,
    XPORT_INTR_CPU_MASK_STATUS_RX_REMOTE_FAULT_INTR_MASK_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_INTR_CPU_MASK_STATUS_SERDES_MOD_DEF0_EVENT_INTR_MASK
 ******************************************************************************/
const ru_field_rec XPORT_INTR_CPU_MASK_STATUS_SERDES_MOD_DEF0_EVENT_INTR_MASK_FIELD =
{
    "SERDES_MOD_DEF0_EVENT_INTR_MASK",
#if RU_INCLUDE_DESC
    "",
    "This bit corresponds to the mask for serdes mod_def0 event interrupt. See register description above for details of how to use this bit.",
#endif
    XPORT_INTR_CPU_MASK_STATUS_SERDES_MOD_DEF0_EVENT_INTR_MASK_FIELD_MASK,
    0,
    XPORT_INTR_CPU_MASK_STATUS_SERDES_MOD_DEF0_EVENT_INTR_MASK_FIELD_WIDTH,
    XPORT_INTR_CPU_MASK_STATUS_SERDES_MOD_DEF0_EVENT_INTR_MASK_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_INTR_CPU_MASK_STATUS_DSERDES_SD_OFF_INTR_MASK
 ******************************************************************************/
const ru_field_rec XPORT_INTR_CPU_MASK_STATUS_DSERDES_SD_OFF_INTR_MASK_FIELD =
{
    "DSERDES_SD_OFF_INTR_MASK",
#if RU_INCLUDE_DESC
    "",
    "This bit corresponds to the mask for dual serdes signal detect off interrupt. See register description above for details of how to use this bit.",
#endif
    XPORT_INTR_CPU_MASK_STATUS_DSERDES_SD_OFF_INTR_MASK_FIELD_MASK,
    0,
    XPORT_INTR_CPU_MASK_STATUS_DSERDES_SD_OFF_INTR_MASK_FIELD_WIDTH,
    XPORT_INTR_CPU_MASK_STATUS_DSERDES_SD_OFF_INTR_MASK_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_INTR_CPU_MASK_STATUS_DSERDES_SD_ON_INTR_MASK
 ******************************************************************************/
const ru_field_rec XPORT_INTR_CPU_MASK_STATUS_DSERDES_SD_ON_INTR_MASK_FIELD =
{
    "DSERDES_SD_ON_INTR_MASK",
#if RU_INCLUDE_DESC
    "",
    "This bit corresponds to the mask for dual serdes signal detect on interrupt. See register description above for details of how to use this bit.",
#endif
    XPORT_INTR_CPU_MASK_STATUS_DSERDES_SD_ON_INTR_MASK_FIELD_MASK,
    0,
    XPORT_INTR_CPU_MASK_STATUS_DSERDES_SD_ON_INTR_MASK_FIELD_WIDTH,
    XPORT_INTR_CPU_MASK_STATUS_DSERDES_SD_ON_INTR_MASK_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_INTR_CPU_MASK_STATUS_LINK_DOWN_INTR_MASK
 ******************************************************************************/
const ru_field_rec XPORT_INTR_CPU_MASK_STATUS_LINK_DOWN_INTR_MASK_FIELD =
{
    "LINK_DOWN_INTR_MASK",
#if RU_INCLUDE_DESC
    "",
    "This bit corresponds to the mask for link down interrupt. See register description above for details of how to use this bit.",
#endif
    XPORT_INTR_CPU_MASK_STATUS_LINK_DOWN_INTR_MASK_FIELD_MASK,
    0,
    XPORT_INTR_CPU_MASK_STATUS_LINK_DOWN_INTR_MASK_FIELD_WIDTH,
    XPORT_INTR_CPU_MASK_STATUS_LINK_DOWN_INTR_MASK_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_INTR_CPU_MASK_STATUS_LINK_UP_INTR_MASK
 ******************************************************************************/
const ru_field_rec XPORT_INTR_CPU_MASK_STATUS_LINK_UP_INTR_MASK_FIELD =
{
    "LINK_UP_INTR_MASK",
#if RU_INCLUDE_DESC
    "",
    "This bit corresponds to the mask for link up interrupt. See register description above for details of how to use this bit.",
#endif
    XPORT_INTR_CPU_MASK_STATUS_LINK_UP_INTR_MASK_FIELD_MASK,
    0,
    XPORT_INTR_CPU_MASK_STATUS_LINK_UP_INTR_MASK_FIELD_WIDTH,
    XPORT_INTR_CPU_MASK_STATUS_LINK_UP_INTR_MASK_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_INTR_CPU_MASK_STATUS_XLMAC_INTR_MASK
 ******************************************************************************/
const ru_field_rec XPORT_INTR_CPU_MASK_STATUS_XLMAC_INTR_MASK_FIELD =
{
    "XLMAC_INTR_MASK",
#if RU_INCLUDE_DESC
    "",
    "This bit corresponds to the mask for  mac tx cdc double bit error interrupt. See register description above for details of how to use this bit.",
#endif
    XPORT_INTR_CPU_MASK_STATUS_XLMAC_INTR_MASK_FIELD_MASK,
    0,
    XPORT_INTR_CPU_MASK_STATUS_XLMAC_INTR_MASK_FIELD_WIDTH,
    XPORT_INTR_CPU_MASK_STATUS_XLMAC_INTR_MASK_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_INTR_CPU_MASK_STATUS_TX_TIMESYNC_FIFO_ENTRY_VALID_INTR_MASK
 ******************************************************************************/
const ru_field_rec XPORT_INTR_CPU_MASK_STATUS_TX_TIMESYNC_FIFO_ENTRY_VALID_INTR_MASK_FIELD =
{
    "TX_TIMESYNC_FIFO_ENTRY_VALID_INTR_MASK",
#if RU_INCLUDE_DESC
    "",
    "This bit corresponds to the mask for  tx timesync fifo entry valid interrupt. See register description above for details of how to use this bit.",
#endif
    XPORT_INTR_CPU_MASK_STATUS_TX_TIMESYNC_FIFO_ENTRY_VALID_INTR_MASK_FIELD_MASK,
    0,
    XPORT_INTR_CPU_MASK_STATUS_TX_TIMESYNC_FIFO_ENTRY_VALID_INTR_MASK_FIELD_WIDTH,
    XPORT_INTR_CPU_MASK_STATUS_TX_TIMESYNC_FIFO_ENTRY_VALID_INTR_MASK_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_INTR_CPU_MASK_STATUS_SGPHY_ENERGY_OFF_INTR_MASK
 ******************************************************************************/
const ru_field_rec XPORT_INTR_CPU_MASK_STATUS_SGPHY_ENERGY_OFF_INTR_MASK_FIELD =
{
    "SGPHY_ENERGY_OFF_INTR_MASK",
#if RU_INCLUDE_DESC
    "",
    "This bit corresponds to the mask for qgphy energy off interrupt. See register description above for details of how to use this bit.",
#endif
    XPORT_INTR_CPU_MASK_STATUS_SGPHY_ENERGY_OFF_INTR_MASK_FIELD_MASK,
    0,
    XPORT_INTR_CPU_MASK_STATUS_SGPHY_ENERGY_OFF_INTR_MASK_FIELD_WIDTH,
    XPORT_INTR_CPU_MASK_STATUS_SGPHY_ENERGY_OFF_INTR_MASK_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_INTR_CPU_MASK_STATUS_SGPHY_ENERGY_ON_INTR_MASK
 ******************************************************************************/
const ru_field_rec XPORT_INTR_CPU_MASK_STATUS_SGPHY_ENERGY_ON_INTR_MASK_FIELD =
{
    "SGPHY_ENERGY_ON_INTR_MASK",
#if RU_INCLUDE_DESC
    "",
    "This bit corresponds to the mask for qgphy energy on interrupt. See register description above for details of how to use this bit.",
#endif
    XPORT_INTR_CPU_MASK_STATUS_SGPHY_ENERGY_ON_INTR_MASK_FIELD_MASK,
    0,
    XPORT_INTR_CPU_MASK_STATUS_SGPHY_ENERGY_ON_INTR_MASK_FIELD_WIDTH,
    XPORT_INTR_CPU_MASK_STATUS_SGPHY_ENERGY_ON_INTR_MASK_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_INTR_CPU_MASK_STATUS_MIB_REG_ERR_INTR_MASK
 ******************************************************************************/
const ru_field_rec XPORT_INTR_CPU_MASK_STATUS_MIB_REG_ERR_INTR_MASK_FIELD =
{
    "MIB_REG_ERR_INTR_MASK",
#if RU_INCLUDE_DESC
    "",
    "This bit corresponds to the mask for mib register error interrupt. See register description above for details of how to use this bit.",
#endif
    XPORT_INTR_CPU_MASK_STATUS_MIB_REG_ERR_INTR_MASK_FIELD_MASK,
    0,
    XPORT_INTR_CPU_MASK_STATUS_MIB_REG_ERR_INTR_MASK_FIELD_WIDTH,
    XPORT_INTR_CPU_MASK_STATUS_MIB_REG_ERR_INTR_MASK_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_INTR_CPU_MASK_STATUS_MAC_REG_ERR_INTR_MASK
 ******************************************************************************/
const ru_field_rec XPORT_INTR_CPU_MASK_STATUS_MAC_REG_ERR_INTR_MASK_FIELD =
{
    "MAC_REG_ERR_INTR_MASK",
#if RU_INCLUDE_DESC
    "",
    "This bit corresponds to the mask for mac register error interrupt. See register description above for details of how to use this bit.",
#endif
    XPORT_INTR_CPU_MASK_STATUS_MAC_REG_ERR_INTR_MASK_FIELD_MASK,
    0,
    XPORT_INTR_CPU_MASK_STATUS_MAC_REG_ERR_INTR_MASK_FIELD_WIDTH,
    XPORT_INTR_CPU_MASK_STATUS_MAC_REG_ERR_INTR_MASK_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_INTR_CPU_MASK_STATUS_UBUS_ERR_INTR_MASK
 ******************************************************************************/
const ru_field_rec XPORT_INTR_CPU_MASK_STATUS_UBUS_ERR_INTR_MASK_FIELD =
{
    "UBUS_ERR_INTR_MASK",
#if RU_INCLUDE_DESC
    "",
    "This bit corresponds to the mask for ubus error interrupt. See register description above for details of how to use this bit.",
#endif
    XPORT_INTR_CPU_MASK_STATUS_UBUS_ERR_INTR_MASK_FIELD_MASK,
    0,
    XPORT_INTR_CPU_MASK_STATUS_UBUS_ERR_INTR_MASK_FIELD_WIDTH,
    XPORT_INTR_CPU_MASK_STATUS_UBUS_ERR_INTR_MASK_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_INTR_CPU_MASK_SET_RESERVED0
 ******************************************************************************/
const ru_field_rec XPORT_INTR_CPU_MASK_SET_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    XPORT_INTR_CPU_MASK_SET_RESERVED0_FIELD_MASK,
    0,
    XPORT_INTR_CPU_MASK_SET_RESERVED0_FIELD_WIDTH,
    XPORT_INTR_CPU_MASK_SET_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_INTR_CPU_MASK_SET_MAB_STATUS_INTR_MASK
 ******************************************************************************/
const ru_field_rec XPORT_INTR_CPU_MASK_SET_MAB_STATUS_INTR_MASK_FIELD =
{
    "MAB_STATUS_INTR_MASK",
#if RU_INCLUDE_DESC
    "",
    "This bit corresponds to the mask for msbus adaptation block status interrupt. See register description above for details of how to use this bit.",
#endif
    XPORT_INTR_CPU_MASK_SET_MAB_STATUS_INTR_MASK_FIELD_MASK,
    0,
    XPORT_INTR_CPU_MASK_SET_MAB_STATUS_INTR_MASK_FIELD_WIDTH,
    XPORT_INTR_CPU_MASK_SET_MAB_STATUS_INTR_MASK_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_INTR_CPU_MASK_SET_RX_REMOTE_FAULT_INTR_MASK
 ******************************************************************************/
const ru_field_rec XPORT_INTR_CPU_MASK_SET_RX_REMOTE_FAULT_INTR_MASK_FIELD =
{
    "RX_REMOTE_FAULT_INTR_MASK",
#if RU_INCLUDE_DESC
    "",
    "This bit corresponds to the mask for rx remote fault interrupt. See register description above for details of how to use this bit.",
#endif
    XPORT_INTR_CPU_MASK_SET_RX_REMOTE_FAULT_INTR_MASK_FIELD_MASK,
    0,
    XPORT_INTR_CPU_MASK_SET_RX_REMOTE_FAULT_INTR_MASK_FIELD_WIDTH,
    XPORT_INTR_CPU_MASK_SET_RX_REMOTE_FAULT_INTR_MASK_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_INTR_CPU_MASK_SET_SERDES_MOD_DEF0_EVENT_INTR_MASK
 ******************************************************************************/
const ru_field_rec XPORT_INTR_CPU_MASK_SET_SERDES_MOD_DEF0_EVENT_INTR_MASK_FIELD =
{
    "SERDES_MOD_DEF0_EVENT_INTR_MASK",
#if RU_INCLUDE_DESC
    "",
    "This bit corresponds to the mask for serdes mod_def0 event interrupt. See register description above for details of how to use this bit.",
#endif
    XPORT_INTR_CPU_MASK_SET_SERDES_MOD_DEF0_EVENT_INTR_MASK_FIELD_MASK,
    0,
    XPORT_INTR_CPU_MASK_SET_SERDES_MOD_DEF0_EVENT_INTR_MASK_FIELD_WIDTH,
    XPORT_INTR_CPU_MASK_SET_SERDES_MOD_DEF0_EVENT_INTR_MASK_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_INTR_CPU_MASK_SET_DSERDES_SD_OFF_INTR_MASK
 ******************************************************************************/
const ru_field_rec XPORT_INTR_CPU_MASK_SET_DSERDES_SD_OFF_INTR_MASK_FIELD =
{
    "DSERDES_SD_OFF_INTR_MASK",
#if RU_INCLUDE_DESC
    "",
    "This bit corresponds to the mask for dual serdes signal detect off interrupt. See register description above for details of how to use this bit.",
#endif
    XPORT_INTR_CPU_MASK_SET_DSERDES_SD_OFF_INTR_MASK_FIELD_MASK,
    0,
    XPORT_INTR_CPU_MASK_SET_DSERDES_SD_OFF_INTR_MASK_FIELD_WIDTH,
    XPORT_INTR_CPU_MASK_SET_DSERDES_SD_OFF_INTR_MASK_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_INTR_CPU_MASK_SET_DSERDES_SD_ON_INTR_MASK
 ******************************************************************************/
const ru_field_rec XPORT_INTR_CPU_MASK_SET_DSERDES_SD_ON_INTR_MASK_FIELD =
{
    "DSERDES_SD_ON_INTR_MASK",
#if RU_INCLUDE_DESC
    "",
    "This bit corresponds to the mask for dual serdes signal detect on interrupt. See register description above for details of how to use this bit.",
#endif
    XPORT_INTR_CPU_MASK_SET_DSERDES_SD_ON_INTR_MASK_FIELD_MASK,
    0,
    XPORT_INTR_CPU_MASK_SET_DSERDES_SD_ON_INTR_MASK_FIELD_WIDTH,
    XPORT_INTR_CPU_MASK_SET_DSERDES_SD_ON_INTR_MASK_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_INTR_CPU_MASK_SET_LINK_DOWN_INTR_MASK
 ******************************************************************************/
const ru_field_rec XPORT_INTR_CPU_MASK_SET_LINK_DOWN_INTR_MASK_FIELD =
{
    "LINK_DOWN_INTR_MASK",
#if RU_INCLUDE_DESC
    "",
    "This bit corresponds to the mask for link down interrupt. See register description above for details of how to use this bit.",
#endif
    XPORT_INTR_CPU_MASK_SET_LINK_DOWN_INTR_MASK_FIELD_MASK,
    0,
    XPORT_INTR_CPU_MASK_SET_LINK_DOWN_INTR_MASK_FIELD_WIDTH,
    XPORT_INTR_CPU_MASK_SET_LINK_DOWN_INTR_MASK_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_INTR_CPU_MASK_SET_LINK_UP_INTR_MASK
 ******************************************************************************/
const ru_field_rec XPORT_INTR_CPU_MASK_SET_LINK_UP_INTR_MASK_FIELD =
{
    "LINK_UP_INTR_MASK",
#if RU_INCLUDE_DESC
    "",
    "This bit corresponds to the mask for link up interrupt. See register description above for details of how to use this bit.",
#endif
    XPORT_INTR_CPU_MASK_SET_LINK_UP_INTR_MASK_FIELD_MASK,
    0,
    XPORT_INTR_CPU_MASK_SET_LINK_UP_INTR_MASK_FIELD_WIDTH,
    XPORT_INTR_CPU_MASK_SET_LINK_UP_INTR_MASK_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_INTR_CPU_MASK_SET_XLMAC_INTR_MASK
 ******************************************************************************/
const ru_field_rec XPORT_INTR_CPU_MASK_SET_XLMAC_INTR_MASK_FIELD =
{
    "XLMAC_INTR_MASK",
#if RU_INCLUDE_DESC
    "",
    "This bit corresponds to the mask for  mac tx cdc double bit error interrupt. See register description above for details of how to use this bit.",
#endif
    XPORT_INTR_CPU_MASK_SET_XLMAC_INTR_MASK_FIELD_MASK,
    0,
    XPORT_INTR_CPU_MASK_SET_XLMAC_INTR_MASK_FIELD_WIDTH,
    XPORT_INTR_CPU_MASK_SET_XLMAC_INTR_MASK_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_INTR_CPU_MASK_SET_TX_TIMESYNC_FIFO_ENTRY_VALID_INTR_MASK
 ******************************************************************************/
const ru_field_rec XPORT_INTR_CPU_MASK_SET_TX_TIMESYNC_FIFO_ENTRY_VALID_INTR_MASK_FIELD =
{
    "TX_TIMESYNC_FIFO_ENTRY_VALID_INTR_MASK",
#if RU_INCLUDE_DESC
    "",
    "This bit corresponds to the mask for  tx timesync fifo entry valid interrupt. See register description above for details of how to use this bit.",
#endif
    XPORT_INTR_CPU_MASK_SET_TX_TIMESYNC_FIFO_ENTRY_VALID_INTR_MASK_FIELD_MASK,
    0,
    XPORT_INTR_CPU_MASK_SET_TX_TIMESYNC_FIFO_ENTRY_VALID_INTR_MASK_FIELD_WIDTH,
    XPORT_INTR_CPU_MASK_SET_TX_TIMESYNC_FIFO_ENTRY_VALID_INTR_MASK_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_INTR_CPU_MASK_SET_SGPHY_ENERGY_OFF_INTR_MASK
 ******************************************************************************/
const ru_field_rec XPORT_INTR_CPU_MASK_SET_SGPHY_ENERGY_OFF_INTR_MASK_FIELD =
{
    "SGPHY_ENERGY_OFF_INTR_MASK",
#if RU_INCLUDE_DESC
    "",
    "This bit corresponds to the mask for qgphy energy off interrupt. See register description above for details of how to use this bit.",
#endif
    XPORT_INTR_CPU_MASK_SET_SGPHY_ENERGY_OFF_INTR_MASK_FIELD_MASK,
    0,
    XPORT_INTR_CPU_MASK_SET_SGPHY_ENERGY_OFF_INTR_MASK_FIELD_WIDTH,
    XPORT_INTR_CPU_MASK_SET_SGPHY_ENERGY_OFF_INTR_MASK_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_INTR_CPU_MASK_SET_SGPHY_ENERGY_ON_INTR_MASK
 ******************************************************************************/
const ru_field_rec XPORT_INTR_CPU_MASK_SET_SGPHY_ENERGY_ON_INTR_MASK_FIELD =
{
    "SGPHY_ENERGY_ON_INTR_MASK",
#if RU_INCLUDE_DESC
    "",
    "This bit corresponds to the mask for qgphy energy on interrupt. See register description above for details of how to use this bit.",
#endif
    XPORT_INTR_CPU_MASK_SET_SGPHY_ENERGY_ON_INTR_MASK_FIELD_MASK,
    0,
    XPORT_INTR_CPU_MASK_SET_SGPHY_ENERGY_ON_INTR_MASK_FIELD_WIDTH,
    XPORT_INTR_CPU_MASK_SET_SGPHY_ENERGY_ON_INTR_MASK_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_INTR_CPU_MASK_SET_MIB_REG_ERR_INTR_MASK
 ******************************************************************************/
const ru_field_rec XPORT_INTR_CPU_MASK_SET_MIB_REG_ERR_INTR_MASK_FIELD =
{
    "MIB_REG_ERR_INTR_MASK",
#if RU_INCLUDE_DESC
    "",
    "This bit corresponds to the mask for mib register error interrupt. See register description above for details of how to use this bit.",
#endif
    XPORT_INTR_CPU_MASK_SET_MIB_REG_ERR_INTR_MASK_FIELD_MASK,
    0,
    XPORT_INTR_CPU_MASK_SET_MIB_REG_ERR_INTR_MASK_FIELD_WIDTH,
    XPORT_INTR_CPU_MASK_SET_MIB_REG_ERR_INTR_MASK_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_INTR_CPU_MASK_SET_MAC_REG_ERR_INTR_MASK
 ******************************************************************************/
const ru_field_rec XPORT_INTR_CPU_MASK_SET_MAC_REG_ERR_INTR_MASK_FIELD =
{
    "MAC_REG_ERR_INTR_MASK",
#if RU_INCLUDE_DESC
    "",
    "This bit corresponds to the mask for mac register error interrupt. See register description above for details of how to use this bit.",
#endif
    XPORT_INTR_CPU_MASK_SET_MAC_REG_ERR_INTR_MASK_FIELD_MASK,
    0,
    XPORT_INTR_CPU_MASK_SET_MAC_REG_ERR_INTR_MASK_FIELD_WIDTH,
    XPORT_INTR_CPU_MASK_SET_MAC_REG_ERR_INTR_MASK_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_INTR_CPU_MASK_SET_UBUS_ERR_INTR_MASK
 ******************************************************************************/
const ru_field_rec XPORT_INTR_CPU_MASK_SET_UBUS_ERR_INTR_MASK_FIELD =
{
    "UBUS_ERR_INTR_MASK",
#if RU_INCLUDE_DESC
    "",
    "This bit corresponds to the mask for ubus error interrupt. See register description above for details of how to use this bit.",
#endif
    XPORT_INTR_CPU_MASK_SET_UBUS_ERR_INTR_MASK_FIELD_MASK,
    0,
    XPORT_INTR_CPU_MASK_SET_UBUS_ERR_INTR_MASK_FIELD_WIDTH,
    XPORT_INTR_CPU_MASK_SET_UBUS_ERR_INTR_MASK_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_INTR_CPU_MASK_CLEAR_RESERVED0
 ******************************************************************************/
const ru_field_rec XPORT_INTR_CPU_MASK_CLEAR_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    XPORT_INTR_CPU_MASK_CLEAR_RESERVED0_FIELD_MASK,
    0,
    XPORT_INTR_CPU_MASK_CLEAR_RESERVED0_FIELD_WIDTH,
    XPORT_INTR_CPU_MASK_CLEAR_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_INTR_CPU_MASK_CLEAR_MAB_STATUS_INTR_MASK
 ******************************************************************************/
const ru_field_rec XPORT_INTR_CPU_MASK_CLEAR_MAB_STATUS_INTR_MASK_FIELD =
{
    "MAB_STATUS_INTR_MASK",
#if RU_INCLUDE_DESC
    "",
    "This bit corresponds to the mask for msbus adaptation block status interrupt. See register description above for details of how to use this bit.",
#endif
    XPORT_INTR_CPU_MASK_CLEAR_MAB_STATUS_INTR_MASK_FIELD_MASK,
    0,
    XPORT_INTR_CPU_MASK_CLEAR_MAB_STATUS_INTR_MASK_FIELD_WIDTH,
    XPORT_INTR_CPU_MASK_CLEAR_MAB_STATUS_INTR_MASK_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_INTR_CPU_MASK_CLEAR_RX_REMOTE_FAULT_INTR_MASK
 ******************************************************************************/
const ru_field_rec XPORT_INTR_CPU_MASK_CLEAR_RX_REMOTE_FAULT_INTR_MASK_FIELD =
{
    "RX_REMOTE_FAULT_INTR_MASK",
#if RU_INCLUDE_DESC
    "",
    "This bit corresponds to the mask for rx remote fault interrupt. See register description above for details of how to use this bit.",
#endif
    XPORT_INTR_CPU_MASK_CLEAR_RX_REMOTE_FAULT_INTR_MASK_FIELD_MASK,
    0,
    XPORT_INTR_CPU_MASK_CLEAR_RX_REMOTE_FAULT_INTR_MASK_FIELD_WIDTH,
    XPORT_INTR_CPU_MASK_CLEAR_RX_REMOTE_FAULT_INTR_MASK_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_INTR_CPU_MASK_CLEAR_SERDES_MOD_DEF0_EVENT_INTR_MASK
 ******************************************************************************/
const ru_field_rec XPORT_INTR_CPU_MASK_CLEAR_SERDES_MOD_DEF0_EVENT_INTR_MASK_FIELD =
{
    "SERDES_MOD_DEF0_EVENT_INTR_MASK",
#if RU_INCLUDE_DESC
    "",
    "This bit corresponds to the mask for serdes mod_def0 event interrupt. See register description above for details of how to use this bit.",
#endif
    XPORT_INTR_CPU_MASK_CLEAR_SERDES_MOD_DEF0_EVENT_INTR_MASK_FIELD_MASK,
    0,
    XPORT_INTR_CPU_MASK_CLEAR_SERDES_MOD_DEF0_EVENT_INTR_MASK_FIELD_WIDTH,
    XPORT_INTR_CPU_MASK_CLEAR_SERDES_MOD_DEF0_EVENT_INTR_MASK_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_INTR_CPU_MASK_CLEAR_DSERDES_SD_OFF_INTR_MASK
 ******************************************************************************/
const ru_field_rec XPORT_INTR_CPU_MASK_CLEAR_DSERDES_SD_OFF_INTR_MASK_FIELD =
{
    "DSERDES_SD_OFF_INTR_MASK",
#if RU_INCLUDE_DESC
    "",
    "This bit corresponds to the mask for dual serdes signal detect off interrupt. See register description above for details of how to use this bit.",
#endif
    XPORT_INTR_CPU_MASK_CLEAR_DSERDES_SD_OFF_INTR_MASK_FIELD_MASK,
    0,
    XPORT_INTR_CPU_MASK_CLEAR_DSERDES_SD_OFF_INTR_MASK_FIELD_WIDTH,
    XPORT_INTR_CPU_MASK_CLEAR_DSERDES_SD_OFF_INTR_MASK_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_INTR_CPU_MASK_CLEAR_DSERDES_SD_ON_INTR_MASK
 ******************************************************************************/
const ru_field_rec XPORT_INTR_CPU_MASK_CLEAR_DSERDES_SD_ON_INTR_MASK_FIELD =
{
    "DSERDES_SD_ON_INTR_MASK",
#if RU_INCLUDE_DESC
    "",
    "This bit corresponds to the mask for dual serdes signal detect on interrupt. See register description above for details of how to use this bit.",
#endif
    XPORT_INTR_CPU_MASK_CLEAR_DSERDES_SD_ON_INTR_MASK_FIELD_MASK,
    0,
    XPORT_INTR_CPU_MASK_CLEAR_DSERDES_SD_ON_INTR_MASK_FIELD_WIDTH,
    XPORT_INTR_CPU_MASK_CLEAR_DSERDES_SD_ON_INTR_MASK_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_INTR_CPU_MASK_CLEAR_LINK_DOWN_INTR_MASK
 ******************************************************************************/
const ru_field_rec XPORT_INTR_CPU_MASK_CLEAR_LINK_DOWN_INTR_MASK_FIELD =
{
    "LINK_DOWN_INTR_MASK",
#if RU_INCLUDE_DESC
    "",
    "This bit corresponds to the mask for link down interrupt. See register description above for details of how to use this bit.",
#endif
    XPORT_INTR_CPU_MASK_CLEAR_LINK_DOWN_INTR_MASK_FIELD_MASK,
    0,
    XPORT_INTR_CPU_MASK_CLEAR_LINK_DOWN_INTR_MASK_FIELD_WIDTH,
    XPORT_INTR_CPU_MASK_CLEAR_LINK_DOWN_INTR_MASK_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_INTR_CPU_MASK_CLEAR_LINK_UP_INTR_MASK
 ******************************************************************************/
const ru_field_rec XPORT_INTR_CPU_MASK_CLEAR_LINK_UP_INTR_MASK_FIELD =
{
    "LINK_UP_INTR_MASK",
#if RU_INCLUDE_DESC
    "",
    "This bit corresponds to the mask for link up interrupt. See register description above for details of how to use this bit.",
#endif
    XPORT_INTR_CPU_MASK_CLEAR_LINK_UP_INTR_MASK_FIELD_MASK,
    0,
    XPORT_INTR_CPU_MASK_CLEAR_LINK_UP_INTR_MASK_FIELD_WIDTH,
    XPORT_INTR_CPU_MASK_CLEAR_LINK_UP_INTR_MASK_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_INTR_CPU_MASK_CLEAR_XLMAC_INTR_MASK
 ******************************************************************************/
const ru_field_rec XPORT_INTR_CPU_MASK_CLEAR_XLMAC_INTR_MASK_FIELD =
{
    "XLMAC_INTR_MASK",
#if RU_INCLUDE_DESC
    "",
    "This bit corresponds to the mask for  mac tx cdc double bit error interrupt. See register description above for details of how to use this bit.",
#endif
    XPORT_INTR_CPU_MASK_CLEAR_XLMAC_INTR_MASK_FIELD_MASK,
    0,
    XPORT_INTR_CPU_MASK_CLEAR_XLMAC_INTR_MASK_FIELD_WIDTH,
    XPORT_INTR_CPU_MASK_CLEAR_XLMAC_INTR_MASK_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_INTR_CPU_MASK_CLEAR_TX_TIMESYNC_FIFO_ENTRY_VALID_INTR_MASK
 ******************************************************************************/
const ru_field_rec XPORT_INTR_CPU_MASK_CLEAR_TX_TIMESYNC_FIFO_ENTRY_VALID_INTR_MASK_FIELD =
{
    "TX_TIMESYNC_FIFO_ENTRY_VALID_INTR_MASK",
#if RU_INCLUDE_DESC
    "",
    "This bit corresponds to the mask for  tx timesync fifo entry valid interrupt. See register description above for details of how to use this bit.",
#endif
    XPORT_INTR_CPU_MASK_CLEAR_TX_TIMESYNC_FIFO_ENTRY_VALID_INTR_MASK_FIELD_MASK,
    0,
    XPORT_INTR_CPU_MASK_CLEAR_TX_TIMESYNC_FIFO_ENTRY_VALID_INTR_MASK_FIELD_WIDTH,
    XPORT_INTR_CPU_MASK_CLEAR_TX_TIMESYNC_FIFO_ENTRY_VALID_INTR_MASK_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_INTR_CPU_MASK_CLEAR_SGPHY_ENERGY_OFF_INTR_MASK
 ******************************************************************************/
const ru_field_rec XPORT_INTR_CPU_MASK_CLEAR_SGPHY_ENERGY_OFF_INTR_MASK_FIELD =
{
    "SGPHY_ENERGY_OFF_INTR_MASK",
#if RU_INCLUDE_DESC
    "",
    "This bit corresponds to the mask for qgphy energy off interrupt. See register description above for details of how to use this bit.",
#endif
    XPORT_INTR_CPU_MASK_CLEAR_SGPHY_ENERGY_OFF_INTR_MASK_FIELD_MASK,
    0,
    XPORT_INTR_CPU_MASK_CLEAR_SGPHY_ENERGY_OFF_INTR_MASK_FIELD_WIDTH,
    XPORT_INTR_CPU_MASK_CLEAR_SGPHY_ENERGY_OFF_INTR_MASK_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_INTR_CPU_MASK_CLEAR_SGPHY_ENERGY_ON_INTR_MASK
 ******************************************************************************/
const ru_field_rec XPORT_INTR_CPU_MASK_CLEAR_SGPHY_ENERGY_ON_INTR_MASK_FIELD =
{
    "SGPHY_ENERGY_ON_INTR_MASK",
#if RU_INCLUDE_DESC
    "",
    "This bit corresponds to the mask for qgphy energy on interrupt. See register description above for details of how to use this bit.",
#endif
    XPORT_INTR_CPU_MASK_CLEAR_SGPHY_ENERGY_ON_INTR_MASK_FIELD_MASK,
    0,
    XPORT_INTR_CPU_MASK_CLEAR_SGPHY_ENERGY_ON_INTR_MASK_FIELD_WIDTH,
    XPORT_INTR_CPU_MASK_CLEAR_SGPHY_ENERGY_ON_INTR_MASK_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_INTR_CPU_MASK_CLEAR_MIB_REG_ERR_INTR_MASK
 ******************************************************************************/
const ru_field_rec XPORT_INTR_CPU_MASK_CLEAR_MIB_REG_ERR_INTR_MASK_FIELD =
{
    "MIB_REG_ERR_INTR_MASK",
#if RU_INCLUDE_DESC
    "",
    "This bit corresponds to the mask for mib register error interrupt. See register description above for details of how to use this bit.",
#endif
    XPORT_INTR_CPU_MASK_CLEAR_MIB_REG_ERR_INTR_MASK_FIELD_MASK,
    0,
    XPORT_INTR_CPU_MASK_CLEAR_MIB_REG_ERR_INTR_MASK_FIELD_WIDTH,
    XPORT_INTR_CPU_MASK_CLEAR_MIB_REG_ERR_INTR_MASK_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_INTR_CPU_MASK_CLEAR_MAC_REG_ERR_INTR_MASK
 ******************************************************************************/
const ru_field_rec XPORT_INTR_CPU_MASK_CLEAR_MAC_REG_ERR_INTR_MASK_FIELD =
{
    "MAC_REG_ERR_INTR_MASK",
#if RU_INCLUDE_DESC
    "",
    "This bit corresponds to the mask for mac register error interrupt. See register description above for details of how to use this bit.",
#endif
    XPORT_INTR_CPU_MASK_CLEAR_MAC_REG_ERR_INTR_MASK_FIELD_MASK,
    0,
    XPORT_INTR_CPU_MASK_CLEAR_MAC_REG_ERR_INTR_MASK_FIELD_WIDTH,
    XPORT_INTR_CPU_MASK_CLEAR_MAC_REG_ERR_INTR_MASK_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_INTR_CPU_MASK_CLEAR_UBUS_ERR_INTR_MASK
 ******************************************************************************/
const ru_field_rec XPORT_INTR_CPU_MASK_CLEAR_UBUS_ERR_INTR_MASK_FIELD =
{
    "UBUS_ERR_INTR_MASK",
#if RU_INCLUDE_DESC
    "",
    "This bit corresponds to the mask for ubus error interrupt. See register description above for details of how to use this bit.",
#endif
    XPORT_INTR_CPU_MASK_CLEAR_UBUS_ERR_INTR_MASK_FIELD_MASK,
    0,
    XPORT_INTR_CPU_MASK_CLEAR_UBUS_ERR_INTR_MASK_FIELD_WIDTH,
    XPORT_INTR_CPU_MASK_CLEAR_UBUS_ERR_INTR_MASK_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_INTR_PCI_STATUS_RESERVED0
 ******************************************************************************/
const ru_field_rec XPORT_INTR_PCI_STATUS_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    XPORT_INTR_PCI_STATUS_RESERVED0_FIELD_MASK,
    0,
    XPORT_INTR_PCI_STATUS_RESERVED0_FIELD_WIDTH,
    XPORT_INTR_PCI_STATUS_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_INTR_PCI_STATUS_MAB_STATUS_INTR
 ******************************************************************************/
const ru_field_rec XPORT_INTR_PCI_STATUS_MAB_STATUS_INTR_FIELD =
{
    "MAB_STATUS_INTR",
#if RU_INCLUDE_DESC
    "",
    "Asserted when any of statuses get set in MSBUS Adaptation Block Status Register(s).",
#endif
    XPORT_INTR_PCI_STATUS_MAB_STATUS_INTR_FIELD_MASK,
    0,
    XPORT_INTR_PCI_STATUS_MAB_STATUS_INTR_FIELD_WIDTH,
    XPORT_INTR_PCI_STATUS_MAB_STATUS_INTR_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_INTR_PCI_STATUS_RX_REMOTE_FAULT_INTR
 ******************************************************************************/
const ru_field_rec XPORT_INTR_PCI_STATUS_RX_REMOTE_FAULT_INTR_FIELD =
{
    "RX_REMOTE_FAULT_INTR",
#if RU_INCLUDE_DESC
    "",
    "RX Remote Fault Interrupt for P0. Valid only when port is in 10G mode",
#endif
    XPORT_INTR_PCI_STATUS_RX_REMOTE_FAULT_INTR_FIELD_MASK,
    0,
    XPORT_INTR_PCI_STATUS_RX_REMOTE_FAULT_INTR_FIELD_WIDTH,
    XPORT_INTR_PCI_STATUS_RX_REMOTE_FAULT_INTR_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_INTR_PCI_STATUS_SERDES_MOD_DEF0_EVENT_INTR
 ******************************************************************************/
const ru_field_rec XPORT_INTR_PCI_STATUS_SERDES_MOD_DEF0_EVENT_INTR_FIELD =
{
    "SERDES_MOD_DEF0_EVENT_INTR",
#if RU_INCLUDE_DESC
    "",
    "Asserted when a change in optical module presence is detected.\n"
    "serdes_mod_def0_event_intr[1] corresponds to SGMII SERDES.\n"
    "serdes_mod_def0_event_intr[0] corresponds to PON AE SERDES.\n"
    "The corresponding status register should be read to determine whether the interrupt is caused by the optical module insertion or removal.",
#endif
    XPORT_INTR_PCI_STATUS_SERDES_MOD_DEF0_EVENT_INTR_FIELD_MASK,
    0,
    XPORT_INTR_PCI_STATUS_SERDES_MOD_DEF0_EVENT_INTR_FIELD_WIDTH,
    XPORT_INTR_PCI_STATUS_SERDES_MOD_DEF0_EVENT_INTR_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_INTR_PCI_STATUS_DSERDES_SD_OFF_INTR
 ******************************************************************************/
const ru_field_rec XPORT_INTR_PCI_STATUS_DSERDES_SD_OFF_INTR_FIELD =
{
    "DSERDES_SD_OFF_INTR",
#if RU_INCLUDE_DESC
    "",
    "Inverted sd_on interrupt. Interrupt to serdes lane mapping is same as for dserdes_sd_on_intr[1:0]. "
    "Based on filtered signal detect.",
#endif
    XPORT_INTR_PCI_STATUS_DSERDES_SD_OFF_INTR_FIELD_MASK,
    0,
    XPORT_INTR_PCI_STATUS_DSERDES_SD_OFF_INTR_FIELD_WIDTH,
    XPORT_INTR_PCI_STATUS_DSERDES_SD_OFF_INTR_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_INTR_PCI_STATUS_DSERDES_SD_ON_INTR
 ******************************************************************************/
const ru_field_rec XPORT_INTR_PCI_STATUS_DSERDES_SD_ON_INTR_FIELD =
{
    "DSERDES_SD_ON_INTR",
#if RU_INCLUDE_DESC
    "",
    "Active high signal detect interrupt (signals presence of the link partner).\n"
    "dserdes_sd_on_intr[1] corresponds to 2.5G SGMII SERDES.\n"
    "dserdes_sd_on_intr[0] corresponds to PON AE SERDES.\n"
    "Based on filtered signal detect.",
#endif
    XPORT_INTR_PCI_STATUS_DSERDES_SD_ON_INTR_FIELD_MASK,
    0,
    XPORT_INTR_PCI_STATUS_DSERDES_SD_ON_INTR_FIELD_WIDTH,
    XPORT_INTR_PCI_STATUS_DSERDES_SD_ON_INTR_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_INTR_PCI_STATUS_LINK_DOWN_INTR
 ******************************************************************************/
const ru_field_rec XPORT_INTR_PCI_STATUS_LINK_DOWN_INTR_FIELD =
{
    "LINK_DOWN_INTR",
#if RU_INCLUDE_DESC
    "",
    "Link down (i.e. link interruption) interrupt for P1:P0. Interrupt corresponds to the attached PHY/interface link down event.",
#endif
    XPORT_INTR_PCI_STATUS_LINK_DOWN_INTR_FIELD_MASK,
    0,
    XPORT_INTR_PCI_STATUS_LINK_DOWN_INTR_FIELD_WIDTH,
    XPORT_INTR_PCI_STATUS_LINK_DOWN_INTR_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_INTR_PCI_STATUS_LINK_UP_INTR
 ******************************************************************************/
const ru_field_rec XPORT_INTR_PCI_STATUS_LINK_UP_INTR_FIELD =
{
    "LINK_UP_INTR",
#if RU_INCLUDE_DESC
    "",
    "Link up interrupt for P1:P0. Interrupt corresponds to the attached PHY/interface link up event.",
#endif
    XPORT_INTR_PCI_STATUS_LINK_UP_INTR_FIELD_MASK,
    0,
    XPORT_INTR_PCI_STATUS_LINK_UP_INTR_FIELD_WIDTH,
    XPORT_INTR_PCI_STATUS_LINK_UP_INTR_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_INTR_PCI_STATUS_XLMAC_INTR
 ******************************************************************************/
const ru_field_rec XPORT_INTR_PCI_STATUS_XLMAC_INTR_FIELD =
{
    "XLMAC_INTR",
#if RU_INCLUDE_DESC
    "",
    "Interrupt generated by XLMAC signaling various events as described in XLMAC_INTR_STATUS."
    "Bits [1:0] correspond to XLMAC0 ports 1 and 0.",
#endif
    XPORT_INTR_PCI_STATUS_XLMAC_INTR_FIELD_MASK,
    0,
    XPORT_INTR_PCI_STATUS_XLMAC_INTR_FIELD_WIDTH,
    XPORT_INTR_PCI_STATUS_XLMAC_INTR_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_INTR_PCI_STATUS_TX_TIMESYNC_FIFO_ENTRY_VALID_INTR
 ******************************************************************************/
const ru_field_rec XPORT_INTR_PCI_STATUS_TX_TIMESYNC_FIFO_ENTRY_VALID_INTR_FIELD =
{
    "TX_TIMESYNC_FIFO_ENTRY_VALID_INTR",
#if RU_INCLUDE_DESC
    "",
    "tx_timesync_fifo_entry_valid_intr[1:0].\n"
    "PTP timestamp available for read. Bits [1:0] correspond to XLMAC0 ports 1 and 0.",
#endif
    XPORT_INTR_PCI_STATUS_TX_TIMESYNC_FIFO_ENTRY_VALID_INTR_FIELD_MASK,
    0,
    XPORT_INTR_PCI_STATUS_TX_TIMESYNC_FIFO_ENTRY_VALID_INTR_FIELD_WIDTH,
    XPORT_INTR_PCI_STATUS_TX_TIMESYNC_FIFO_ENTRY_VALID_INTR_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_INTR_PCI_STATUS_SGPHY_ENERGY_OFF_INTR
 ******************************************************************************/
const ru_field_rec XPORT_INTR_PCI_STATUS_SGPHY_ENERGY_OFF_INTR_FIELD =
{
    "SGPHY_ENERGY_OFF_INTR",
#if RU_INCLUDE_DESC
    "",
    "Active high energy detect interrupt signal (signals presence of the link partner) asserted "
    "when SGPHYs detects loss of energy. Based on SGPHY's energy_det_apd.",
#endif
    XPORT_INTR_PCI_STATUS_SGPHY_ENERGY_OFF_INTR_FIELD_MASK,
    0,
    XPORT_INTR_PCI_STATUS_SGPHY_ENERGY_OFF_INTR_FIELD_WIDTH,
    XPORT_INTR_PCI_STATUS_SGPHY_ENERGY_OFF_INTR_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_INTR_PCI_STATUS_SGPHY_ENERGY_ON_INTR
 ******************************************************************************/
const ru_field_rec XPORT_INTR_PCI_STATUS_SGPHY_ENERGY_ON_INTR_FIELD =
{
    "SGPHY_ENERGY_ON_INTR",
#if RU_INCLUDE_DESC
    "",
    "Active high energy detect interrupt signal (signals presence of the link partner) asserted "
    "when SGPHYs detects the energy. Based on SGPHY's energy_det_apd.",
#endif
    XPORT_INTR_PCI_STATUS_SGPHY_ENERGY_ON_INTR_FIELD_MASK,
    0,
    XPORT_INTR_PCI_STATUS_SGPHY_ENERGY_ON_INTR_FIELD_WIDTH,
    XPORT_INTR_PCI_STATUS_SGPHY_ENERGY_ON_INTR_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_INTR_PCI_STATUS_MIB_REG_ERR_INTR
 ******************************************************************************/
const ru_field_rec XPORT_INTR_PCI_STATUS_MIB_REG_ERR_INTR_FIELD =
{
    "MIB_REG_ERR_INTR",
#if RU_INCLUDE_DESC
    "",
    "mib_reg_err_intr.\n"
    "MIB register transaction error.",
#endif
    XPORT_INTR_PCI_STATUS_MIB_REG_ERR_INTR_FIELD_MASK,
    0,
    XPORT_INTR_PCI_STATUS_MIB_REG_ERR_INTR_FIELD_WIDTH,
    XPORT_INTR_PCI_STATUS_MIB_REG_ERR_INTR_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_INTR_PCI_STATUS_MAC_REG_ERR_INTR
 ******************************************************************************/
const ru_field_rec XPORT_INTR_PCI_STATUS_MAC_REG_ERR_INTR_FIELD =
{
    "MAC_REG_ERR_INTR",
#if RU_INCLUDE_DESC
    "",
    "mac_reg_err_intr.\n"
    "XLMAC register transaction error",
#endif
    XPORT_INTR_PCI_STATUS_MAC_REG_ERR_INTR_FIELD_MASK,
    0,
    XPORT_INTR_PCI_STATUS_MAC_REG_ERR_INTR_FIELD_WIDTH,
    XPORT_INTR_PCI_STATUS_MAC_REG_ERR_INTR_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_INTR_PCI_STATUS_UBUS_ERR_INTR
 ******************************************************************************/
const ru_field_rec XPORT_INTR_PCI_STATUS_UBUS_ERR_INTR_FIELD =
{
    "UBUS_ERR_INTR",
#if RU_INCLUDE_DESC
    "",
    "UBUS transaction error.",
#endif
    XPORT_INTR_PCI_STATUS_UBUS_ERR_INTR_FIELD_MASK,
    0,
    XPORT_INTR_PCI_STATUS_UBUS_ERR_INTR_FIELD_WIDTH,
    XPORT_INTR_PCI_STATUS_UBUS_ERR_INTR_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_INTR_PCI_SET_RESERVED0
 ******************************************************************************/
const ru_field_rec XPORT_INTR_PCI_SET_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    XPORT_INTR_PCI_SET_RESERVED0_FIELD_MASK,
    0,
    XPORT_INTR_PCI_SET_RESERVED0_FIELD_WIDTH,
    XPORT_INTR_PCI_SET_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_INTR_PCI_SET_MAB_STATUS_INTR
 ******************************************************************************/
const ru_field_rec XPORT_INTR_PCI_SET_MAB_STATUS_INTR_FIELD =
{
    "MAB_STATUS_INTR",
#if RU_INCLUDE_DESC
    "",
    "Asserted when any of statuses get set in MSBUS Adaptation Block Status Register(s).",
#endif
    XPORT_INTR_PCI_SET_MAB_STATUS_INTR_FIELD_MASK,
    0,
    XPORT_INTR_PCI_SET_MAB_STATUS_INTR_FIELD_WIDTH,
    XPORT_INTR_PCI_SET_MAB_STATUS_INTR_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_INTR_PCI_SET_RX_REMOTE_FAULT_INTR
 ******************************************************************************/
const ru_field_rec XPORT_INTR_PCI_SET_RX_REMOTE_FAULT_INTR_FIELD =
{
    "RX_REMOTE_FAULT_INTR",
#if RU_INCLUDE_DESC
    "",
    "RX Remote Fault Interrupt for P0. Valid only when port is in 10G mode",
#endif
    XPORT_INTR_PCI_SET_RX_REMOTE_FAULT_INTR_FIELD_MASK,
    0,
    XPORT_INTR_PCI_SET_RX_REMOTE_FAULT_INTR_FIELD_WIDTH,
    XPORT_INTR_PCI_SET_RX_REMOTE_FAULT_INTR_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_INTR_PCI_SET_SERDES_MOD_DEF0_EVENT_INTR
 ******************************************************************************/
const ru_field_rec XPORT_INTR_PCI_SET_SERDES_MOD_DEF0_EVENT_INTR_FIELD =
{
    "SERDES_MOD_DEF0_EVENT_INTR",
#if RU_INCLUDE_DESC
    "",
    "Asserted when a change in optical module presence is detected.\n"
    "serdes_mod_def0_event_intr[1] corresponds to SGMII SERDES.\n"
    "serdes_mod_def0_event_intr[0] corresponds to PON AE SERDES.\n"
    "The corresponding status register should be read to determine whether the interrupt is caused by the optical module insertion or removal.",
#endif
    XPORT_INTR_PCI_SET_SERDES_MOD_DEF0_EVENT_INTR_FIELD_MASK,
    0,
    XPORT_INTR_PCI_SET_SERDES_MOD_DEF0_EVENT_INTR_FIELD_WIDTH,
    XPORT_INTR_PCI_SET_SERDES_MOD_DEF0_EVENT_INTR_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_INTR_PCI_SET_DSERDES_SD_OFF_INTR
 ******************************************************************************/
const ru_field_rec XPORT_INTR_PCI_SET_DSERDES_SD_OFF_INTR_FIELD =
{
    "DSERDES_SD_OFF_INTR",
#if RU_INCLUDE_DESC
    "",
    "Inverted sd_on interrupt. Interrupt to serdes lane mapping is same as for dserdes_sd_on_intr[1:0]. "
    "Based on filtered signal detect.",
#endif
    XPORT_INTR_PCI_SET_DSERDES_SD_OFF_INTR_FIELD_MASK,
    0,
    XPORT_INTR_PCI_SET_DSERDES_SD_OFF_INTR_FIELD_WIDTH,
    XPORT_INTR_PCI_SET_DSERDES_SD_OFF_INTR_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_INTR_PCI_SET_DSERDES_SD_ON_INTR
 ******************************************************************************/
const ru_field_rec XPORT_INTR_PCI_SET_DSERDES_SD_ON_INTR_FIELD =
{
    "DSERDES_SD_ON_INTR",
#if RU_INCLUDE_DESC
    "",
    "Active high signal detect interrupt (signals presence of the link partner).\n"
    "dserdes_sd_on_intr[1] corresponds to 2.5G SGMII SERDES.\n"
    "dserdes_sd_on_intr[0] corresponds to PON AE SERDES.\n"
    "Based on filtered signal detect.",
#endif
    XPORT_INTR_PCI_SET_DSERDES_SD_ON_INTR_FIELD_MASK,
    0,
    XPORT_INTR_PCI_SET_DSERDES_SD_ON_INTR_FIELD_WIDTH,
    XPORT_INTR_PCI_SET_DSERDES_SD_ON_INTR_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_INTR_PCI_SET_LINK_DOWN_INTR
 ******************************************************************************/
const ru_field_rec XPORT_INTR_PCI_SET_LINK_DOWN_INTR_FIELD =
{
    "LINK_DOWN_INTR",
#if RU_INCLUDE_DESC
    "",
    "Link down (i.e. link interruption) interrupt for P1:P0. Interrupt corresponds to the attached PHY/interface link down event.",
#endif
    XPORT_INTR_PCI_SET_LINK_DOWN_INTR_FIELD_MASK,
    0,
    XPORT_INTR_PCI_SET_LINK_DOWN_INTR_FIELD_WIDTH,
    XPORT_INTR_PCI_SET_LINK_DOWN_INTR_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_INTR_PCI_SET_LINK_UP_INTR
 ******************************************************************************/
const ru_field_rec XPORT_INTR_PCI_SET_LINK_UP_INTR_FIELD =
{
    "LINK_UP_INTR",
#if RU_INCLUDE_DESC
    "",
    "Link up interrupt for P1:P0. Interrupt corresponds to the attached PHY/interface link up event.",
#endif
    XPORT_INTR_PCI_SET_LINK_UP_INTR_FIELD_MASK,
    0,
    XPORT_INTR_PCI_SET_LINK_UP_INTR_FIELD_WIDTH,
    XPORT_INTR_PCI_SET_LINK_UP_INTR_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_INTR_PCI_SET_XLMAC_INTR
 ******************************************************************************/
const ru_field_rec XPORT_INTR_PCI_SET_XLMAC_INTR_FIELD =
{
    "XLMAC_INTR",
#if RU_INCLUDE_DESC
    "",
    "Interrupt generated by XLMAC signaling various events as described in XLMAC_INTR_STATUS."
    "Bits [1:0] correspond to XLMAC0 ports 1 and 0.",
#endif
    XPORT_INTR_PCI_SET_XLMAC_INTR_FIELD_MASK,
    0,
    XPORT_INTR_PCI_SET_XLMAC_INTR_FIELD_WIDTH,
    XPORT_INTR_PCI_SET_XLMAC_INTR_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_INTR_PCI_SET_TX_TIMESYNC_FIFO_ENTRY_VALID_INTR
 ******************************************************************************/
const ru_field_rec XPORT_INTR_PCI_SET_TX_TIMESYNC_FIFO_ENTRY_VALID_INTR_FIELD =
{
    "TX_TIMESYNC_FIFO_ENTRY_VALID_INTR",
#if RU_INCLUDE_DESC
    "",
    "tx_timesync_fifo_entry_valid_intr[1:0].\n"
    "PTP timestamp available for read. Bits [1:0] correspond to XLMAC0 ports 1 and 0.",
#endif
    XPORT_INTR_PCI_SET_TX_TIMESYNC_FIFO_ENTRY_VALID_INTR_FIELD_MASK,
    0,
    XPORT_INTR_PCI_SET_TX_TIMESYNC_FIFO_ENTRY_VALID_INTR_FIELD_WIDTH,
    XPORT_INTR_PCI_SET_TX_TIMESYNC_FIFO_ENTRY_VALID_INTR_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_INTR_PCI_SET_SGPHY_ENERGY_OFF_INTR
 ******************************************************************************/
const ru_field_rec XPORT_INTR_PCI_SET_SGPHY_ENERGY_OFF_INTR_FIELD =
{
    "SGPHY_ENERGY_OFF_INTR",
#if RU_INCLUDE_DESC
    "",
    "Active high energy detect interrupt signal (signals presence of the link partner) asserted "
    "when SGPHYs detects loss of energy. Based on SGPHY's energy_det_apd.",
#endif
    XPORT_INTR_PCI_SET_SGPHY_ENERGY_OFF_INTR_FIELD_MASK,
    0,
    XPORT_INTR_PCI_SET_SGPHY_ENERGY_OFF_INTR_FIELD_WIDTH,
    XPORT_INTR_PCI_SET_SGPHY_ENERGY_OFF_INTR_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_INTR_PCI_SET_SGPHY_ENERGY_ON_INTR
 ******************************************************************************/
const ru_field_rec XPORT_INTR_PCI_SET_SGPHY_ENERGY_ON_INTR_FIELD =
{
    "SGPHY_ENERGY_ON_INTR",
#if RU_INCLUDE_DESC
    "",
    "Active high energy detect interrupt signal (signals presence of the link partner) asserted "
    "when SGPHYs detects the energy. Based on SGPHY's energy_det_apd.",
#endif
    XPORT_INTR_PCI_SET_SGPHY_ENERGY_ON_INTR_FIELD_MASK,
    0,
    XPORT_INTR_PCI_SET_SGPHY_ENERGY_ON_INTR_FIELD_WIDTH,
    XPORT_INTR_PCI_SET_SGPHY_ENERGY_ON_INTR_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_INTR_PCI_SET_MIB_REG_ERR_INTR
 ******************************************************************************/
const ru_field_rec XPORT_INTR_PCI_SET_MIB_REG_ERR_INTR_FIELD =
{
    "MIB_REG_ERR_INTR",
#if RU_INCLUDE_DESC
    "",
    "mib_reg_err_intr.\n"
    "MIB register transaction error.",
#endif
    XPORT_INTR_PCI_SET_MIB_REG_ERR_INTR_FIELD_MASK,
    0,
    XPORT_INTR_PCI_SET_MIB_REG_ERR_INTR_FIELD_WIDTH,
    XPORT_INTR_PCI_SET_MIB_REG_ERR_INTR_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_INTR_PCI_SET_MAC_REG_ERR_INTR
 ******************************************************************************/
const ru_field_rec XPORT_INTR_PCI_SET_MAC_REG_ERR_INTR_FIELD =
{
    "MAC_REG_ERR_INTR",
#if RU_INCLUDE_DESC
    "",
    "mac_reg_err_intr.\n"
    "XLMAC register transaction error",
#endif
    XPORT_INTR_PCI_SET_MAC_REG_ERR_INTR_FIELD_MASK,
    0,
    XPORT_INTR_PCI_SET_MAC_REG_ERR_INTR_FIELD_WIDTH,
    XPORT_INTR_PCI_SET_MAC_REG_ERR_INTR_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_INTR_PCI_SET_UBUS_ERR_INTR
 ******************************************************************************/
const ru_field_rec XPORT_INTR_PCI_SET_UBUS_ERR_INTR_FIELD =
{
    "UBUS_ERR_INTR",
#if RU_INCLUDE_DESC
    "",
    "UBUS transaction error.",
#endif
    XPORT_INTR_PCI_SET_UBUS_ERR_INTR_FIELD_MASK,
    0,
    XPORT_INTR_PCI_SET_UBUS_ERR_INTR_FIELD_WIDTH,
    XPORT_INTR_PCI_SET_UBUS_ERR_INTR_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_INTR_PCI_CLEAR_RESERVED0
 ******************************************************************************/
const ru_field_rec XPORT_INTR_PCI_CLEAR_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    XPORT_INTR_PCI_CLEAR_RESERVED0_FIELD_MASK,
    0,
    XPORT_INTR_PCI_CLEAR_RESERVED0_FIELD_WIDTH,
    XPORT_INTR_PCI_CLEAR_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_INTR_PCI_CLEAR_MAB_STATUS_INTR
 ******************************************************************************/
const ru_field_rec XPORT_INTR_PCI_CLEAR_MAB_STATUS_INTR_FIELD =
{
    "MAB_STATUS_INTR",
#if RU_INCLUDE_DESC
    "",
    "Asserted when any of statuses get set in MSBUS Adaptation Block Status Register(s).",
#endif
    XPORT_INTR_PCI_CLEAR_MAB_STATUS_INTR_FIELD_MASK,
    0,
    XPORT_INTR_PCI_CLEAR_MAB_STATUS_INTR_FIELD_WIDTH,
    XPORT_INTR_PCI_CLEAR_MAB_STATUS_INTR_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_INTR_PCI_CLEAR_RX_REMOTE_FAULT_INTR
 ******************************************************************************/
const ru_field_rec XPORT_INTR_PCI_CLEAR_RX_REMOTE_FAULT_INTR_FIELD =
{
    "RX_REMOTE_FAULT_INTR",
#if RU_INCLUDE_DESC
    "",
    "RX Remote Fault Interrupt for P0. Valid only when port is in 10G mode",
#endif
    XPORT_INTR_PCI_CLEAR_RX_REMOTE_FAULT_INTR_FIELD_MASK,
    0,
    XPORT_INTR_PCI_CLEAR_RX_REMOTE_FAULT_INTR_FIELD_WIDTH,
    XPORT_INTR_PCI_CLEAR_RX_REMOTE_FAULT_INTR_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_INTR_PCI_CLEAR_SERDES_MOD_DEF0_EVENT_INTR
 ******************************************************************************/
const ru_field_rec XPORT_INTR_PCI_CLEAR_SERDES_MOD_DEF0_EVENT_INTR_FIELD =
{
    "SERDES_MOD_DEF0_EVENT_INTR",
#if RU_INCLUDE_DESC
    "",
    "Asserted when a change in optical module presence is detected.\n"
    "serdes_mod_def0_event_intr[1] corresponds to SGMII SERDES.\n"
    "serdes_mod_def0_event_intr[0] corresponds to PON AE SERDES.\n"
    "The corresponding status register should be read to determine whether the interrupt is caused by the optical module insertion or removal.",
#endif
    XPORT_INTR_PCI_CLEAR_SERDES_MOD_DEF0_EVENT_INTR_FIELD_MASK,
    0,
    XPORT_INTR_PCI_CLEAR_SERDES_MOD_DEF0_EVENT_INTR_FIELD_WIDTH,
    XPORT_INTR_PCI_CLEAR_SERDES_MOD_DEF0_EVENT_INTR_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_INTR_PCI_CLEAR_DSERDES_SD_OFF_INTR
 ******************************************************************************/
const ru_field_rec XPORT_INTR_PCI_CLEAR_DSERDES_SD_OFF_INTR_FIELD =
{
    "DSERDES_SD_OFF_INTR",
#if RU_INCLUDE_DESC
    "",
    "Inverted sd_on interrupt. Interrupt to serdes lane mapping is same as for dserdes_sd_on_intr[1:0]. "
    "Based on filtered signal detect.",
#endif
    XPORT_INTR_PCI_CLEAR_DSERDES_SD_OFF_INTR_FIELD_MASK,
    0,
    XPORT_INTR_PCI_CLEAR_DSERDES_SD_OFF_INTR_FIELD_WIDTH,
    XPORT_INTR_PCI_CLEAR_DSERDES_SD_OFF_INTR_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_INTR_PCI_CLEAR_DSERDES_SD_ON_INTR
 ******************************************************************************/
const ru_field_rec XPORT_INTR_PCI_CLEAR_DSERDES_SD_ON_INTR_FIELD =
{
    "DSERDES_SD_ON_INTR",
#if RU_INCLUDE_DESC
    "",
    "Active high signal detect interrupt (signals presence of the link partner).\n"
    "dserdes_sd_on_intr[1] corresponds to 2.5G SGMII SERDES.\n"
    "dserdes_sd_on_intr[0] corresponds to PON AE SERDES.\n"
    "Based on filtered signal detect.",
#endif
    XPORT_INTR_PCI_CLEAR_DSERDES_SD_ON_INTR_FIELD_MASK,
    0,
    XPORT_INTR_PCI_CLEAR_DSERDES_SD_ON_INTR_FIELD_WIDTH,
    XPORT_INTR_PCI_CLEAR_DSERDES_SD_ON_INTR_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_INTR_PCI_CLEAR_LINK_DOWN_INTR
 ******************************************************************************/
const ru_field_rec XPORT_INTR_PCI_CLEAR_LINK_DOWN_INTR_FIELD =
{
    "LINK_DOWN_INTR",
#if RU_INCLUDE_DESC
    "",
    "Link down (i.e. link interruption) interrupt for P1:P0. Interrupt corresponds to the attached PHY/interface link down event.",
#endif
    XPORT_INTR_PCI_CLEAR_LINK_DOWN_INTR_FIELD_MASK,
    0,
    XPORT_INTR_PCI_CLEAR_LINK_DOWN_INTR_FIELD_WIDTH,
    XPORT_INTR_PCI_CLEAR_LINK_DOWN_INTR_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_INTR_PCI_CLEAR_LINK_UP_INTR
 ******************************************************************************/
const ru_field_rec XPORT_INTR_PCI_CLEAR_LINK_UP_INTR_FIELD =
{
    "LINK_UP_INTR",
#if RU_INCLUDE_DESC
    "",
    "Link up interrupt for P1:P0. Interrupt corresponds to the attached PHY/interface link up event.",
#endif
    XPORT_INTR_PCI_CLEAR_LINK_UP_INTR_FIELD_MASK,
    0,
    XPORT_INTR_PCI_CLEAR_LINK_UP_INTR_FIELD_WIDTH,
    XPORT_INTR_PCI_CLEAR_LINK_UP_INTR_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_INTR_PCI_CLEAR_XLMAC_INTR
 ******************************************************************************/
const ru_field_rec XPORT_INTR_PCI_CLEAR_XLMAC_INTR_FIELD =
{
    "XLMAC_INTR",
#if RU_INCLUDE_DESC
    "",
    "Interrupt generated by XLMAC signaling various events as described in XLMAC_INTR_STATUS."
    "Bits [1:0] correspond to XLMAC0 ports 1 and 0.",
#endif
    XPORT_INTR_PCI_CLEAR_XLMAC_INTR_FIELD_MASK,
    0,
    XPORT_INTR_PCI_CLEAR_XLMAC_INTR_FIELD_WIDTH,
    XPORT_INTR_PCI_CLEAR_XLMAC_INTR_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_INTR_PCI_CLEAR_TX_TIMESYNC_FIFO_ENTRY_VALID_INTR
 ******************************************************************************/
const ru_field_rec XPORT_INTR_PCI_CLEAR_TX_TIMESYNC_FIFO_ENTRY_VALID_INTR_FIELD =
{
    "TX_TIMESYNC_FIFO_ENTRY_VALID_INTR",
#if RU_INCLUDE_DESC
    "",
    "tx_timesync_fifo_entry_valid_intr[1:0].\n"
    "PTP timestamp available for read. Bits [1:0] correspond to XLMAC0 ports 1 and 0.",
#endif
    XPORT_INTR_PCI_CLEAR_TX_TIMESYNC_FIFO_ENTRY_VALID_INTR_FIELD_MASK,
    0,
    XPORT_INTR_PCI_CLEAR_TX_TIMESYNC_FIFO_ENTRY_VALID_INTR_FIELD_WIDTH,
    XPORT_INTR_PCI_CLEAR_TX_TIMESYNC_FIFO_ENTRY_VALID_INTR_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_INTR_PCI_CLEAR_SGPHY_ENERGY_OFF_INTR
 ******************************************************************************/
const ru_field_rec XPORT_INTR_PCI_CLEAR_SGPHY_ENERGY_OFF_INTR_FIELD =
{
    "SGPHY_ENERGY_OFF_INTR",
#if RU_INCLUDE_DESC
    "",
    "Active high energy detect interrupt signal (signals presence of the link partner) asserted "
    "when SGPHYs detects loss of energy. Based on SGPHY's energy_det_apd.",
#endif
    XPORT_INTR_PCI_CLEAR_SGPHY_ENERGY_OFF_INTR_FIELD_MASK,
    0,
    XPORT_INTR_PCI_CLEAR_SGPHY_ENERGY_OFF_INTR_FIELD_WIDTH,
    XPORT_INTR_PCI_CLEAR_SGPHY_ENERGY_OFF_INTR_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_INTR_PCI_CLEAR_SGPHY_ENERGY_ON_INTR
 ******************************************************************************/
const ru_field_rec XPORT_INTR_PCI_CLEAR_SGPHY_ENERGY_ON_INTR_FIELD =
{
    "SGPHY_ENERGY_ON_INTR",
#if RU_INCLUDE_DESC
    "",
    "Active high energy detect interrupt signal (signals presence of the link partner) asserted "
    "when SGPHYs detects the energy. Based on SGPHY's energy_det_apd.",
#endif
    XPORT_INTR_PCI_CLEAR_SGPHY_ENERGY_ON_INTR_FIELD_MASK,
    0,
    XPORT_INTR_PCI_CLEAR_SGPHY_ENERGY_ON_INTR_FIELD_WIDTH,
    XPORT_INTR_PCI_CLEAR_SGPHY_ENERGY_ON_INTR_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_INTR_PCI_CLEAR_MIB_REG_ERR_INTR
 ******************************************************************************/
const ru_field_rec XPORT_INTR_PCI_CLEAR_MIB_REG_ERR_INTR_FIELD =
{
    "MIB_REG_ERR_INTR",
#if RU_INCLUDE_DESC
    "",
    "mib_reg_err_intr.\n"
    "MIB register transaction error.",
#endif
    XPORT_INTR_PCI_CLEAR_MIB_REG_ERR_INTR_FIELD_MASK,
    0,
    XPORT_INTR_PCI_CLEAR_MIB_REG_ERR_INTR_FIELD_WIDTH,
    XPORT_INTR_PCI_CLEAR_MIB_REG_ERR_INTR_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_INTR_PCI_CLEAR_MAC_REG_ERR_INTR
 ******************************************************************************/
const ru_field_rec XPORT_INTR_PCI_CLEAR_MAC_REG_ERR_INTR_FIELD =
{
    "MAC_REG_ERR_INTR",
#if RU_INCLUDE_DESC
    "",
    "mac_reg_err_intr.\n"
    "XLMAC register transaction error",
#endif
    XPORT_INTR_PCI_CLEAR_MAC_REG_ERR_INTR_FIELD_MASK,
    0,
    XPORT_INTR_PCI_CLEAR_MAC_REG_ERR_INTR_FIELD_WIDTH,
    XPORT_INTR_PCI_CLEAR_MAC_REG_ERR_INTR_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_INTR_PCI_CLEAR_UBUS_ERR_INTR
 ******************************************************************************/
const ru_field_rec XPORT_INTR_PCI_CLEAR_UBUS_ERR_INTR_FIELD =
{
    "UBUS_ERR_INTR",
#if RU_INCLUDE_DESC
    "",
    "UBUS transaction error.",
#endif
    XPORT_INTR_PCI_CLEAR_UBUS_ERR_INTR_FIELD_MASK,
    0,
    XPORT_INTR_PCI_CLEAR_UBUS_ERR_INTR_FIELD_WIDTH,
    XPORT_INTR_PCI_CLEAR_UBUS_ERR_INTR_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_INTR_PCI_MASK_STATUS_RESERVED0
 ******************************************************************************/
const ru_field_rec XPORT_INTR_PCI_MASK_STATUS_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    XPORT_INTR_PCI_MASK_STATUS_RESERVED0_FIELD_MASK,
    0,
    XPORT_INTR_PCI_MASK_STATUS_RESERVED0_FIELD_WIDTH,
    XPORT_INTR_PCI_MASK_STATUS_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_INTR_PCI_MASK_STATUS_MAB_STATUS_INTR_MASK
 ******************************************************************************/
const ru_field_rec XPORT_INTR_PCI_MASK_STATUS_MAB_STATUS_INTR_MASK_FIELD =
{
    "MAB_STATUS_INTR_MASK",
#if RU_INCLUDE_DESC
    "",
    "This bit corresponds to the mask for msbus adaptation block status interrupt. See register description above for details of how to use this bit.",
#endif
    XPORT_INTR_PCI_MASK_STATUS_MAB_STATUS_INTR_MASK_FIELD_MASK,
    0,
    XPORT_INTR_PCI_MASK_STATUS_MAB_STATUS_INTR_MASK_FIELD_WIDTH,
    XPORT_INTR_PCI_MASK_STATUS_MAB_STATUS_INTR_MASK_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_INTR_PCI_MASK_STATUS_RX_REMOTE_FAULT_INTR_MASK
 ******************************************************************************/
const ru_field_rec XPORT_INTR_PCI_MASK_STATUS_RX_REMOTE_FAULT_INTR_MASK_FIELD =
{
    "RX_REMOTE_FAULT_INTR_MASK",
#if RU_INCLUDE_DESC
    "",
    "This bit corresponds to the mask for rx remote fault interrupt. See register description above for details of how to use this bit.",
#endif
    XPORT_INTR_PCI_MASK_STATUS_RX_REMOTE_FAULT_INTR_MASK_FIELD_MASK,
    0,
    XPORT_INTR_PCI_MASK_STATUS_RX_REMOTE_FAULT_INTR_MASK_FIELD_WIDTH,
    XPORT_INTR_PCI_MASK_STATUS_RX_REMOTE_FAULT_INTR_MASK_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_INTR_PCI_MASK_STATUS_SERDES_MOD_DEF0_EVENT_INTR_MASK
 ******************************************************************************/
const ru_field_rec XPORT_INTR_PCI_MASK_STATUS_SERDES_MOD_DEF0_EVENT_INTR_MASK_FIELD =
{
    "SERDES_MOD_DEF0_EVENT_INTR_MASK",
#if RU_INCLUDE_DESC
    "",
    "This bit corresponds to the mask for serdes mod_def0 event interrupt. See register description above for details of how to use this bit.",
#endif
    XPORT_INTR_PCI_MASK_STATUS_SERDES_MOD_DEF0_EVENT_INTR_MASK_FIELD_MASK,
    0,
    XPORT_INTR_PCI_MASK_STATUS_SERDES_MOD_DEF0_EVENT_INTR_MASK_FIELD_WIDTH,
    XPORT_INTR_PCI_MASK_STATUS_SERDES_MOD_DEF0_EVENT_INTR_MASK_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_INTR_PCI_MASK_STATUS_DSERDES_SD_OFF_INTR_MASK
 ******************************************************************************/
const ru_field_rec XPORT_INTR_PCI_MASK_STATUS_DSERDES_SD_OFF_INTR_MASK_FIELD =
{
    "DSERDES_SD_OFF_INTR_MASK",
#if RU_INCLUDE_DESC
    "",
    "This bit corresponds to the mask for dual serdes signal detect off interrupt. See register description above for details of how to use this bit.",
#endif
    XPORT_INTR_PCI_MASK_STATUS_DSERDES_SD_OFF_INTR_MASK_FIELD_MASK,
    0,
    XPORT_INTR_PCI_MASK_STATUS_DSERDES_SD_OFF_INTR_MASK_FIELD_WIDTH,
    XPORT_INTR_PCI_MASK_STATUS_DSERDES_SD_OFF_INTR_MASK_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_INTR_PCI_MASK_STATUS_DSERDES_SD_ON_INTR_MASK
 ******************************************************************************/
const ru_field_rec XPORT_INTR_PCI_MASK_STATUS_DSERDES_SD_ON_INTR_MASK_FIELD =
{
    "DSERDES_SD_ON_INTR_MASK",
#if RU_INCLUDE_DESC
    "",
    "This bit corresponds to the mask for dual serdes signal detect on interrupt. See register description above for details of how to use this bit.",
#endif
    XPORT_INTR_PCI_MASK_STATUS_DSERDES_SD_ON_INTR_MASK_FIELD_MASK,
    0,
    XPORT_INTR_PCI_MASK_STATUS_DSERDES_SD_ON_INTR_MASK_FIELD_WIDTH,
    XPORT_INTR_PCI_MASK_STATUS_DSERDES_SD_ON_INTR_MASK_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_INTR_PCI_MASK_STATUS_LINK_DOWN_INTR_MASK
 ******************************************************************************/
const ru_field_rec XPORT_INTR_PCI_MASK_STATUS_LINK_DOWN_INTR_MASK_FIELD =
{
    "LINK_DOWN_INTR_MASK",
#if RU_INCLUDE_DESC
    "",
    "This bit corresponds to the mask for link down interrupt. See register description above for details of how to use this bit.",
#endif
    XPORT_INTR_PCI_MASK_STATUS_LINK_DOWN_INTR_MASK_FIELD_MASK,
    0,
    XPORT_INTR_PCI_MASK_STATUS_LINK_DOWN_INTR_MASK_FIELD_WIDTH,
    XPORT_INTR_PCI_MASK_STATUS_LINK_DOWN_INTR_MASK_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_INTR_PCI_MASK_STATUS_LINK_UP_INTR_MASK
 ******************************************************************************/
const ru_field_rec XPORT_INTR_PCI_MASK_STATUS_LINK_UP_INTR_MASK_FIELD =
{
    "LINK_UP_INTR_MASK",
#if RU_INCLUDE_DESC
    "",
    "This bit corresponds to the mask for link up interrupt. See register description above for details of how to use this bit.",
#endif
    XPORT_INTR_PCI_MASK_STATUS_LINK_UP_INTR_MASK_FIELD_MASK,
    0,
    XPORT_INTR_PCI_MASK_STATUS_LINK_UP_INTR_MASK_FIELD_WIDTH,
    XPORT_INTR_PCI_MASK_STATUS_LINK_UP_INTR_MASK_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_INTR_PCI_MASK_STATUS_XLMAC_INTR_MASK
 ******************************************************************************/
const ru_field_rec XPORT_INTR_PCI_MASK_STATUS_XLMAC_INTR_MASK_FIELD =
{
    "XLMAC_INTR_MASK",
#if RU_INCLUDE_DESC
    "",
    "This bit corresponds to the mask for  mac tx cdc double bit error interrupt. See register description above for details of how to use this bit.",
#endif
    XPORT_INTR_PCI_MASK_STATUS_XLMAC_INTR_MASK_FIELD_MASK,
    0,
    XPORT_INTR_PCI_MASK_STATUS_XLMAC_INTR_MASK_FIELD_WIDTH,
    XPORT_INTR_PCI_MASK_STATUS_XLMAC_INTR_MASK_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_INTR_PCI_MASK_STATUS_TX_TIMESYNC_FIFO_ENTRY_VALID_INTR_MASK
 ******************************************************************************/
const ru_field_rec XPORT_INTR_PCI_MASK_STATUS_TX_TIMESYNC_FIFO_ENTRY_VALID_INTR_MASK_FIELD =
{
    "TX_TIMESYNC_FIFO_ENTRY_VALID_INTR_MASK",
#if RU_INCLUDE_DESC
    "",
    "This bit corresponds to the mask for  tx timesync fifo entry valid interrupt. See register description above for details of how to use this bit.",
#endif
    XPORT_INTR_PCI_MASK_STATUS_TX_TIMESYNC_FIFO_ENTRY_VALID_INTR_MASK_FIELD_MASK,
    0,
    XPORT_INTR_PCI_MASK_STATUS_TX_TIMESYNC_FIFO_ENTRY_VALID_INTR_MASK_FIELD_WIDTH,
    XPORT_INTR_PCI_MASK_STATUS_TX_TIMESYNC_FIFO_ENTRY_VALID_INTR_MASK_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_INTR_PCI_MASK_STATUS_SGPHY_ENERGY_OFF_INTR_MASK
 ******************************************************************************/
const ru_field_rec XPORT_INTR_PCI_MASK_STATUS_SGPHY_ENERGY_OFF_INTR_MASK_FIELD =
{
    "SGPHY_ENERGY_OFF_INTR_MASK",
#if RU_INCLUDE_DESC
    "",
    "This bit corresponds to the mask for qgphy energy off interrupt. See register description above for details of how to use this bit.",
#endif
    XPORT_INTR_PCI_MASK_STATUS_SGPHY_ENERGY_OFF_INTR_MASK_FIELD_MASK,
    0,
    XPORT_INTR_PCI_MASK_STATUS_SGPHY_ENERGY_OFF_INTR_MASK_FIELD_WIDTH,
    XPORT_INTR_PCI_MASK_STATUS_SGPHY_ENERGY_OFF_INTR_MASK_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_INTR_PCI_MASK_STATUS_SGPHY_ENERGY_ON_INTR_MASK
 ******************************************************************************/
const ru_field_rec XPORT_INTR_PCI_MASK_STATUS_SGPHY_ENERGY_ON_INTR_MASK_FIELD =
{
    "SGPHY_ENERGY_ON_INTR_MASK",
#if RU_INCLUDE_DESC
    "",
    "This bit corresponds to the mask for qgphy energy on interrupt. See register description above for details of how to use this bit.",
#endif
    XPORT_INTR_PCI_MASK_STATUS_SGPHY_ENERGY_ON_INTR_MASK_FIELD_MASK,
    0,
    XPORT_INTR_PCI_MASK_STATUS_SGPHY_ENERGY_ON_INTR_MASK_FIELD_WIDTH,
    XPORT_INTR_PCI_MASK_STATUS_SGPHY_ENERGY_ON_INTR_MASK_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_INTR_PCI_MASK_STATUS_MIB_REG_ERR_INTR_MASK
 ******************************************************************************/
const ru_field_rec XPORT_INTR_PCI_MASK_STATUS_MIB_REG_ERR_INTR_MASK_FIELD =
{
    "MIB_REG_ERR_INTR_MASK",
#if RU_INCLUDE_DESC
    "",
    "This bit corresponds to the mask for mib register error interrupt. See register description above for details of how to use this bit.",
#endif
    XPORT_INTR_PCI_MASK_STATUS_MIB_REG_ERR_INTR_MASK_FIELD_MASK,
    0,
    XPORT_INTR_PCI_MASK_STATUS_MIB_REG_ERR_INTR_MASK_FIELD_WIDTH,
    XPORT_INTR_PCI_MASK_STATUS_MIB_REG_ERR_INTR_MASK_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_INTR_PCI_MASK_STATUS_MAC_REG_ERR_INTR_MASK
 ******************************************************************************/
const ru_field_rec XPORT_INTR_PCI_MASK_STATUS_MAC_REG_ERR_INTR_MASK_FIELD =
{
    "MAC_REG_ERR_INTR_MASK",
#if RU_INCLUDE_DESC
    "",
    "This bit corresponds to the mask for mac register error interrupt. See register description above for details of how to use this bit.",
#endif
    XPORT_INTR_PCI_MASK_STATUS_MAC_REG_ERR_INTR_MASK_FIELD_MASK,
    0,
    XPORT_INTR_PCI_MASK_STATUS_MAC_REG_ERR_INTR_MASK_FIELD_WIDTH,
    XPORT_INTR_PCI_MASK_STATUS_MAC_REG_ERR_INTR_MASK_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_INTR_PCI_MASK_STATUS_UBUS_ERR_INTR_MASK
 ******************************************************************************/
const ru_field_rec XPORT_INTR_PCI_MASK_STATUS_UBUS_ERR_INTR_MASK_FIELD =
{
    "UBUS_ERR_INTR_MASK",
#if RU_INCLUDE_DESC
    "",
    "This bit corresponds to the mask for ubus error interrupt. See register description above for details of how to use this bit.",
#endif
    XPORT_INTR_PCI_MASK_STATUS_UBUS_ERR_INTR_MASK_FIELD_MASK,
    0,
    XPORT_INTR_PCI_MASK_STATUS_UBUS_ERR_INTR_MASK_FIELD_WIDTH,
    XPORT_INTR_PCI_MASK_STATUS_UBUS_ERR_INTR_MASK_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_INTR_PCI_MASK_SET_RESERVED0
 ******************************************************************************/
const ru_field_rec XPORT_INTR_PCI_MASK_SET_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    XPORT_INTR_PCI_MASK_SET_RESERVED0_FIELD_MASK,
    0,
    XPORT_INTR_PCI_MASK_SET_RESERVED0_FIELD_WIDTH,
    XPORT_INTR_PCI_MASK_SET_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_INTR_PCI_MASK_SET_MAB_STATUS_INTR_MASK
 ******************************************************************************/
const ru_field_rec XPORT_INTR_PCI_MASK_SET_MAB_STATUS_INTR_MASK_FIELD =
{
    "MAB_STATUS_INTR_MASK",
#if RU_INCLUDE_DESC
    "",
    "This bit corresponds to the mask for msbus adaptation block status interrupt. See register description above for details of how to use this bit.",
#endif
    XPORT_INTR_PCI_MASK_SET_MAB_STATUS_INTR_MASK_FIELD_MASK,
    0,
    XPORT_INTR_PCI_MASK_SET_MAB_STATUS_INTR_MASK_FIELD_WIDTH,
    XPORT_INTR_PCI_MASK_SET_MAB_STATUS_INTR_MASK_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_INTR_PCI_MASK_SET_RX_REMOTE_FAULT_INTR_MASK
 ******************************************************************************/
const ru_field_rec XPORT_INTR_PCI_MASK_SET_RX_REMOTE_FAULT_INTR_MASK_FIELD =
{
    "RX_REMOTE_FAULT_INTR_MASK",
#if RU_INCLUDE_DESC
    "",
    "This bit corresponds to the mask for rx remote fault interrupt. See register description above for details of how to use this bit.",
#endif
    XPORT_INTR_PCI_MASK_SET_RX_REMOTE_FAULT_INTR_MASK_FIELD_MASK,
    0,
    XPORT_INTR_PCI_MASK_SET_RX_REMOTE_FAULT_INTR_MASK_FIELD_WIDTH,
    XPORT_INTR_PCI_MASK_SET_RX_REMOTE_FAULT_INTR_MASK_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_INTR_PCI_MASK_SET_SERDES_MOD_DEF0_EVENT_INTR_MASK
 ******************************************************************************/
const ru_field_rec XPORT_INTR_PCI_MASK_SET_SERDES_MOD_DEF0_EVENT_INTR_MASK_FIELD =
{
    "SERDES_MOD_DEF0_EVENT_INTR_MASK",
#if RU_INCLUDE_DESC
    "",
    "This bit corresponds to the mask for serdes mod_def0 event interrupt. See register description above for details of how to use this bit.",
#endif
    XPORT_INTR_PCI_MASK_SET_SERDES_MOD_DEF0_EVENT_INTR_MASK_FIELD_MASK,
    0,
    XPORT_INTR_PCI_MASK_SET_SERDES_MOD_DEF0_EVENT_INTR_MASK_FIELD_WIDTH,
    XPORT_INTR_PCI_MASK_SET_SERDES_MOD_DEF0_EVENT_INTR_MASK_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_INTR_PCI_MASK_SET_DSERDES_SD_OFF_INTR_MASK
 ******************************************************************************/
const ru_field_rec XPORT_INTR_PCI_MASK_SET_DSERDES_SD_OFF_INTR_MASK_FIELD =
{
    "DSERDES_SD_OFF_INTR_MASK",
#if RU_INCLUDE_DESC
    "",
    "This bit corresponds to the mask for dual serdes signal detect off interrupt. See register description above for details of how to use this bit.",
#endif
    XPORT_INTR_PCI_MASK_SET_DSERDES_SD_OFF_INTR_MASK_FIELD_MASK,
    0,
    XPORT_INTR_PCI_MASK_SET_DSERDES_SD_OFF_INTR_MASK_FIELD_WIDTH,
    XPORT_INTR_PCI_MASK_SET_DSERDES_SD_OFF_INTR_MASK_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_INTR_PCI_MASK_SET_DSERDES_SD_ON_INTR_MASK
 ******************************************************************************/
const ru_field_rec XPORT_INTR_PCI_MASK_SET_DSERDES_SD_ON_INTR_MASK_FIELD =
{
    "DSERDES_SD_ON_INTR_MASK",
#if RU_INCLUDE_DESC
    "",
    "This bit corresponds to the mask for dual serdes signal detect on interrupt. See register description above for details of how to use this bit.",
#endif
    XPORT_INTR_PCI_MASK_SET_DSERDES_SD_ON_INTR_MASK_FIELD_MASK,
    0,
    XPORT_INTR_PCI_MASK_SET_DSERDES_SD_ON_INTR_MASK_FIELD_WIDTH,
    XPORT_INTR_PCI_MASK_SET_DSERDES_SD_ON_INTR_MASK_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_INTR_PCI_MASK_SET_LINK_DOWN_INTR_MASK
 ******************************************************************************/
const ru_field_rec XPORT_INTR_PCI_MASK_SET_LINK_DOWN_INTR_MASK_FIELD =
{
    "LINK_DOWN_INTR_MASK",
#if RU_INCLUDE_DESC
    "",
    "This bit corresponds to the mask for link down interrupt. See register description above for details of how to use this bit.",
#endif
    XPORT_INTR_PCI_MASK_SET_LINK_DOWN_INTR_MASK_FIELD_MASK,
    0,
    XPORT_INTR_PCI_MASK_SET_LINK_DOWN_INTR_MASK_FIELD_WIDTH,
    XPORT_INTR_PCI_MASK_SET_LINK_DOWN_INTR_MASK_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_INTR_PCI_MASK_SET_LINK_UP_INTR_MASK
 ******************************************************************************/
const ru_field_rec XPORT_INTR_PCI_MASK_SET_LINK_UP_INTR_MASK_FIELD =
{
    "LINK_UP_INTR_MASK",
#if RU_INCLUDE_DESC
    "",
    "This bit corresponds to the mask for link up interrupt. See register description above for details of how to use this bit.",
#endif
    XPORT_INTR_PCI_MASK_SET_LINK_UP_INTR_MASK_FIELD_MASK,
    0,
    XPORT_INTR_PCI_MASK_SET_LINK_UP_INTR_MASK_FIELD_WIDTH,
    XPORT_INTR_PCI_MASK_SET_LINK_UP_INTR_MASK_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_INTR_PCI_MASK_SET_XLMAC_INTR_MASK
 ******************************************************************************/
const ru_field_rec XPORT_INTR_PCI_MASK_SET_XLMAC_INTR_MASK_FIELD =
{
    "XLMAC_INTR_MASK",
#if RU_INCLUDE_DESC
    "",
    "This bit corresponds to the mask for  mac tx cdc double bit error interrupt. See register description above for details of how to use this bit.",
#endif
    XPORT_INTR_PCI_MASK_SET_XLMAC_INTR_MASK_FIELD_MASK,
    0,
    XPORT_INTR_PCI_MASK_SET_XLMAC_INTR_MASK_FIELD_WIDTH,
    XPORT_INTR_PCI_MASK_SET_XLMAC_INTR_MASK_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_INTR_PCI_MASK_SET_TX_TIMESYNC_FIFO_ENTRY_VALID_INTR_MASK
 ******************************************************************************/
const ru_field_rec XPORT_INTR_PCI_MASK_SET_TX_TIMESYNC_FIFO_ENTRY_VALID_INTR_MASK_FIELD =
{
    "TX_TIMESYNC_FIFO_ENTRY_VALID_INTR_MASK",
#if RU_INCLUDE_DESC
    "",
    "This bit corresponds to the mask for  tx timesync fifo entry valid interrupt. See register description above for details of how to use this bit.",
#endif
    XPORT_INTR_PCI_MASK_SET_TX_TIMESYNC_FIFO_ENTRY_VALID_INTR_MASK_FIELD_MASK,
    0,
    XPORT_INTR_PCI_MASK_SET_TX_TIMESYNC_FIFO_ENTRY_VALID_INTR_MASK_FIELD_WIDTH,
    XPORT_INTR_PCI_MASK_SET_TX_TIMESYNC_FIFO_ENTRY_VALID_INTR_MASK_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_INTR_PCI_MASK_SET_SGPHY_ENERGY_OFF_INTR_MASK
 ******************************************************************************/
const ru_field_rec XPORT_INTR_PCI_MASK_SET_SGPHY_ENERGY_OFF_INTR_MASK_FIELD =
{
    "SGPHY_ENERGY_OFF_INTR_MASK",
#if RU_INCLUDE_DESC
    "",
    "This bit corresponds to the mask for qgphy energy off interrupt. See register description above for details of how to use this bit.",
#endif
    XPORT_INTR_PCI_MASK_SET_SGPHY_ENERGY_OFF_INTR_MASK_FIELD_MASK,
    0,
    XPORT_INTR_PCI_MASK_SET_SGPHY_ENERGY_OFF_INTR_MASK_FIELD_WIDTH,
    XPORT_INTR_PCI_MASK_SET_SGPHY_ENERGY_OFF_INTR_MASK_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_INTR_PCI_MASK_SET_SGPHY_ENERGY_ON_INTR_MASK
 ******************************************************************************/
const ru_field_rec XPORT_INTR_PCI_MASK_SET_SGPHY_ENERGY_ON_INTR_MASK_FIELD =
{
    "SGPHY_ENERGY_ON_INTR_MASK",
#if RU_INCLUDE_DESC
    "",
    "This bit corresponds to the mask for qgphy energy on interrupt. See register description above for details of how to use this bit.",
#endif
    XPORT_INTR_PCI_MASK_SET_SGPHY_ENERGY_ON_INTR_MASK_FIELD_MASK,
    0,
    XPORT_INTR_PCI_MASK_SET_SGPHY_ENERGY_ON_INTR_MASK_FIELD_WIDTH,
    XPORT_INTR_PCI_MASK_SET_SGPHY_ENERGY_ON_INTR_MASK_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_INTR_PCI_MASK_SET_MIB_REG_ERR_INTR_MASK
 ******************************************************************************/
const ru_field_rec XPORT_INTR_PCI_MASK_SET_MIB_REG_ERR_INTR_MASK_FIELD =
{
    "MIB_REG_ERR_INTR_MASK",
#if RU_INCLUDE_DESC
    "",
    "This bit corresponds to the mask for mib register error interrupt. See register description above for details of how to use this bit.",
#endif
    XPORT_INTR_PCI_MASK_SET_MIB_REG_ERR_INTR_MASK_FIELD_MASK,
    0,
    XPORT_INTR_PCI_MASK_SET_MIB_REG_ERR_INTR_MASK_FIELD_WIDTH,
    XPORT_INTR_PCI_MASK_SET_MIB_REG_ERR_INTR_MASK_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_INTR_PCI_MASK_SET_MAC_REG_ERR_INTR_MASK
 ******************************************************************************/
const ru_field_rec XPORT_INTR_PCI_MASK_SET_MAC_REG_ERR_INTR_MASK_FIELD =
{
    "MAC_REG_ERR_INTR_MASK",
#if RU_INCLUDE_DESC
    "",
    "This bit corresponds to the mask for mac register error interrupt. See register description above for details of how to use this bit.",
#endif
    XPORT_INTR_PCI_MASK_SET_MAC_REG_ERR_INTR_MASK_FIELD_MASK,
    0,
    XPORT_INTR_PCI_MASK_SET_MAC_REG_ERR_INTR_MASK_FIELD_WIDTH,
    XPORT_INTR_PCI_MASK_SET_MAC_REG_ERR_INTR_MASK_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_INTR_PCI_MASK_SET_UBUS_ERR_INTR_MASK
 ******************************************************************************/
const ru_field_rec XPORT_INTR_PCI_MASK_SET_UBUS_ERR_INTR_MASK_FIELD =
{
    "UBUS_ERR_INTR_MASK",
#if RU_INCLUDE_DESC
    "",
    "This bit corresponds to the mask for ubus error interrupt. See register description above for details of how to use this bit.",
#endif
    XPORT_INTR_PCI_MASK_SET_UBUS_ERR_INTR_MASK_FIELD_MASK,
    0,
    XPORT_INTR_PCI_MASK_SET_UBUS_ERR_INTR_MASK_FIELD_WIDTH,
    XPORT_INTR_PCI_MASK_SET_UBUS_ERR_INTR_MASK_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_INTR_PCI_MASK_CLEAR_RESERVED0
 ******************************************************************************/
const ru_field_rec XPORT_INTR_PCI_MASK_CLEAR_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    XPORT_INTR_PCI_MASK_CLEAR_RESERVED0_FIELD_MASK,
    0,
    XPORT_INTR_PCI_MASK_CLEAR_RESERVED0_FIELD_WIDTH,
    XPORT_INTR_PCI_MASK_CLEAR_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_INTR_PCI_MASK_CLEAR_MAB_STATUS_INTR_MASK
 ******************************************************************************/
const ru_field_rec XPORT_INTR_PCI_MASK_CLEAR_MAB_STATUS_INTR_MASK_FIELD =
{
    "MAB_STATUS_INTR_MASK",
#if RU_INCLUDE_DESC
    "",
    "This bit corresponds to the mask for msbus adaptation block status interrupt. See register description above for details of how to use this bit.",
#endif
    XPORT_INTR_PCI_MASK_CLEAR_MAB_STATUS_INTR_MASK_FIELD_MASK,
    0,
    XPORT_INTR_PCI_MASK_CLEAR_MAB_STATUS_INTR_MASK_FIELD_WIDTH,
    XPORT_INTR_PCI_MASK_CLEAR_MAB_STATUS_INTR_MASK_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_INTR_PCI_MASK_CLEAR_RX_REMOTE_FAULT_INTR_MASK
 ******************************************************************************/
const ru_field_rec XPORT_INTR_PCI_MASK_CLEAR_RX_REMOTE_FAULT_INTR_MASK_FIELD =
{
    "RX_REMOTE_FAULT_INTR_MASK",
#if RU_INCLUDE_DESC
    "",
    "This bit corresponds to the mask for rx remote fault interrupt. See register description above for details of how to use this bit.",
#endif
    XPORT_INTR_PCI_MASK_CLEAR_RX_REMOTE_FAULT_INTR_MASK_FIELD_MASK,
    0,
    XPORT_INTR_PCI_MASK_CLEAR_RX_REMOTE_FAULT_INTR_MASK_FIELD_WIDTH,
    XPORT_INTR_PCI_MASK_CLEAR_RX_REMOTE_FAULT_INTR_MASK_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_INTR_PCI_MASK_CLEAR_SERDES_MOD_DEF0_EVENT_INTR_MASK
 ******************************************************************************/
const ru_field_rec XPORT_INTR_PCI_MASK_CLEAR_SERDES_MOD_DEF0_EVENT_INTR_MASK_FIELD =
{
    "SERDES_MOD_DEF0_EVENT_INTR_MASK",
#if RU_INCLUDE_DESC
    "",
    "This bit corresponds to the mask for serdes mod_def0 event interrupt. See register description above for details of how to use this bit.",
#endif
    XPORT_INTR_PCI_MASK_CLEAR_SERDES_MOD_DEF0_EVENT_INTR_MASK_FIELD_MASK,
    0,
    XPORT_INTR_PCI_MASK_CLEAR_SERDES_MOD_DEF0_EVENT_INTR_MASK_FIELD_WIDTH,
    XPORT_INTR_PCI_MASK_CLEAR_SERDES_MOD_DEF0_EVENT_INTR_MASK_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_INTR_PCI_MASK_CLEAR_DSERDES_SD_OFF_INTR_MASK
 ******************************************************************************/
const ru_field_rec XPORT_INTR_PCI_MASK_CLEAR_DSERDES_SD_OFF_INTR_MASK_FIELD =
{
    "DSERDES_SD_OFF_INTR_MASK",
#if RU_INCLUDE_DESC
    "",
    "This bit corresponds to the mask for dual serdes signal detect off interrupt. See register description above for details of how to use this bit.",
#endif
    XPORT_INTR_PCI_MASK_CLEAR_DSERDES_SD_OFF_INTR_MASK_FIELD_MASK,
    0,
    XPORT_INTR_PCI_MASK_CLEAR_DSERDES_SD_OFF_INTR_MASK_FIELD_WIDTH,
    XPORT_INTR_PCI_MASK_CLEAR_DSERDES_SD_OFF_INTR_MASK_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_INTR_PCI_MASK_CLEAR_DSERDES_SD_ON_INTR_MASK
 ******************************************************************************/
const ru_field_rec XPORT_INTR_PCI_MASK_CLEAR_DSERDES_SD_ON_INTR_MASK_FIELD =
{
    "DSERDES_SD_ON_INTR_MASK",
#if RU_INCLUDE_DESC
    "",
    "This bit corresponds to the mask for dual serdes signal detect on interrupt. See register description above for details of how to use this bit.",
#endif
    XPORT_INTR_PCI_MASK_CLEAR_DSERDES_SD_ON_INTR_MASK_FIELD_MASK,
    0,
    XPORT_INTR_PCI_MASK_CLEAR_DSERDES_SD_ON_INTR_MASK_FIELD_WIDTH,
    XPORT_INTR_PCI_MASK_CLEAR_DSERDES_SD_ON_INTR_MASK_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_INTR_PCI_MASK_CLEAR_LINK_DOWN_INTR_MASK
 ******************************************************************************/
const ru_field_rec XPORT_INTR_PCI_MASK_CLEAR_LINK_DOWN_INTR_MASK_FIELD =
{
    "LINK_DOWN_INTR_MASK",
#if RU_INCLUDE_DESC
    "",
    "This bit corresponds to the mask for link down interrupt. See register description above for details of how to use this bit.",
#endif
    XPORT_INTR_PCI_MASK_CLEAR_LINK_DOWN_INTR_MASK_FIELD_MASK,
    0,
    XPORT_INTR_PCI_MASK_CLEAR_LINK_DOWN_INTR_MASK_FIELD_WIDTH,
    XPORT_INTR_PCI_MASK_CLEAR_LINK_DOWN_INTR_MASK_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_INTR_PCI_MASK_CLEAR_LINK_UP_INTR_MASK
 ******************************************************************************/
const ru_field_rec XPORT_INTR_PCI_MASK_CLEAR_LINK_UP_INTR_MASK_FIELD =
{
    "LINK_UP_INTR_MASK",
#if RU_INCLUDE_DESC
    "",
    "This bit corresponds to the mask for link up interrupt. See register description above for details of how to use this bit.",
#endif
    XPORT_INTR_PCI_MASK_CLEAR_LINK_UP_INTR_MASK_FIELD_MASK,
    0,
    XPORT_INTR_PCI_MASK_CLEAR_LINK_UP_INTR_MASK_FIELD_WIDTH,
    XPORT_INTR_PCI_MASK_CLEAR_LINK_UP_INTR_MASK_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_INTR_PCI_MASK_CLEAR_XLMAC_INTR_MASK
 ******************************************************************************/
const ru_field_rec XPORT_INTR_PCI_MASK_CLEAR_XLMAC_INTR_MASK_FIELD =
{
    "XLMAC_INTR_MASK",
#if RU_INCLUDE_DESC
    "",
    "This bit corresponds to the mask for  mac tx cdc double bit error interrupt. See register description above for details of how to use this bit.",
#endif
    XPORT_INTR_PCI_MASK_CLEAR_XLMAC_INTR_MASK_FIELD_MASK,
    0,
    XPORT_INTR_PCI_MASK_CLEAR_XLMAC_INTR_MASK_FIELD_WIDTH,
    XPORT_INTR_PCI_MASK_CLEAR_XLMAC_INTR_MASK_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_INTR_PCI_MASK_CLEAR_TX_TIMESYNC_FIFO_ENTRY_VALID_INTR_MASK
 ******************************************************************************/
const ru_field_rec XPORT_INTR_PCI_MASK_CLEAR_TX_TIMESYNC_FIFO_ENTRY_VALID_INTR_MASK_FIELD =
{
    "TX_TIMESYNC_FIFO_ENTRY_VALID_INTR_MASK",
#if RU_INCLUDE_DESC
    "",
    "This bit corresponds to the mask for  tx timesync fifo entry valid interrupt. See register description above for details of how to use this bit.",
#endif
    XPORT_INTR_PCI_MASK_CLEAR_TX_TIMESYNC_FIFO_ENTRY_VALID_INTR_MASK_FIELD_MASK,
    0,
    XPORT_INTR_PCI_MASK_CLEAR_TX_TIMESYNC_FIFO_ENTRY_VALID_INTR_MASK_FIELD_WIDTH,
    XPORT_INTR_PCI_MASK_CLEAR_TX_TIMESYNC_FIFO_ENTRY_VALID_INTR_MASK_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_INTR_PCI_MASK_CLEAR_SGPHY_ENERGY_OFF_INTR_MASK
 ******************************************************************************/
const ru_field_rec XPORT_INTR_PCI_MASK_CLEAR_SGPHY_ENERGY_OFF_INTR_MASK_FIELD =
{
    "SGPHY_ENERGY_OFF_INTR_MASK",
#if RU_INCLUDE_DESC
    "",
    "This bit corresponds to the mask for qgphy energy off interrupt. See register description above for details of how to use this bit.",
#endif
    XPORT_INTR_PCI_MASK_CLEAR_SGPHY_ENERGY_OFF_INTR_MASK_FIELD_MASK,
    0,
    XPORT_INTR_PCI_MASK_CLEAR_SGPHY_ENERGY_OFF_INTR_MASK_FIELD_WIDTH,
    XPORT_INTR_PCI_MASK_CLEAR_SGPHY_ENERGY_OFF_INTR_MASK_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_INTR_PCI_MASK_CLEAR_SGPHY_ENERGY_ON_INTR_MASK
 ******************************************************************************/
const ru_field_rec XPORT_INTR_PCI_MASK_CLEAR_SGPHY_ENERGY_ON_INTR_MASK_FIELD =
{
    "SGPHY_ENERGY_ON_INTR_MASK",
#if RU_INCLUDE_DESC
    "",
    "This bit corresponds to the mask for qgphy energy on interrupt. See register description above for details of how to use this bit.",
#endif
    XPORT_INTR_PCI_MASK_CLEAR_SGPHY_ENERGY_ON_INTR_MASK_FIELD_MASK,
    0,
    XPORT_INTR_PCI_MASK_CLEAR_SGPHY_ENERGY_ON_INTR_MASK_FIELD_WIDTH,
    XPORT_INTR_PCI_MASK_CLEAR_SGPHY_ENERGY_ON_INTR_MASK_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_INTR_PCI_MASK_CLEAR_MIB_REG_ERR_INTR_MASK
 ******************************************************************************/
const ru_field_rec XPORT_INTR_PCI_MASK_CLEAR_MIB_REG_ERR_INTR_MASK_FIELD =
{
    "MIB_REG_ERR_INTR_MASK",
#if RU_INCLUDE_DESC
    "",
    "This bit corresponds to the mask for mib register error interrupt. See register description above for details of how to use this bit.",
#endif
    XPORT_INTR_PCI_MASK_CLEAR_MIB_REG_ERR_INTR_MASK_FIELD_MASK,
    0,
    XPORT_INTR_PCI_MASK_CLEAR_MIB_REG_ERR_INTR_MASK_FIELD_WIDTH,
    XPORT_INTR_PCI_MASK_CLEAR_MIB_REG_ERR_INTR_MASK_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_INTR_PCI_MASK_CLEAR_MAC_REG_ERR_INTR_MASK
 ******************************************************************************/
const ru_field_rec XPORT_INTR_PCI_MASK_CLEAR_MAC_REG_ERR_INTR_MASK_FIELD =
{
    "MAC_REG_ERR_INTR_MASK",
#if RU_INCLUDE_DESC
    "",
    "This bit corresponds to the mask for mac register error interrupt. See register description above for details of how to use this bit.",
#endif
    XPORT_INTR_PCI_MASK_CLEAR_MAC_REG_ERR_INTR_MASK_FIELD_MASK,
    0,
    XPORT_INTR_PCI_MASK_CLEAR_MAC_REG_ERR_INTR_MASK_FIELD_WIDTH,
    XPORT_INTR_PCI_MASK_CLEAR_MAC_REG_ERR_INTR_MASK_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPORT_INTR_PCI_MASK_CLEAR_UBUS_ERR_INTR_MASK
 ******************************************************************************/
const ru_field_rec XPORT_INTR_PCI_MASK_CLEAR_UBUS_ERR_INTR_MASK_FIELD =
{
    "UBUS_ERR_INTR_MASK",
#if RU_INCLUDE_DESC
    "",
    "This bit corresponds to the mask for ubus error interrupt. See register description above for details of how to use this bit.",
#endif
    XPORT_INTR_PCI_MASK_CLEAR_UBUS_ERR_INTR_MASK_FIELD_MASK,
    0,
    XPORT_INTR_PCI_MASK_CLEAR_UBUS_ERR_INTR_MASK_FIELD_WIDTH,
    XPORT_INTR_PCI_MASK_CLEAR_UBUS_ERR_INTR_MASK_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

#endif /* RU_INCLUDE_FIELD_DB */

/******************************************************************************
 * Register: XPORT_INTR_CPU_STATUS
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XPORT_INTR_CPU_STATUS_FIELDS[] =
{
    &XPORT_INTR_CPU_STATUS_RESERVED0_FIELD,
    &XPORT_INTR_CPU_STATUS_MAB_STATUS_INTR_FIELD,
    &XPORT_INTR_CPU_STATUS_RX_REMOTE_FAULT_INTR_FIELD,
    &XPORT_INTR_CPU_STATUS_SERDES_MOD_DEF0_EVENT_INTR_FIELD,
    &XPORT_INTR_CPU_STATUS_DSERDES_SD_OFF_INTR_FIELD,
    &XPORT_INTR_CPU_STATUS_DSERDES_SD_ON_INTR_FIELD,
    &XPORT_INTR_CPU_STATUS_LINK_DOWN_INTR_FIELD,
    &XPORT_INTR_CPU_STATUS_LINK_UP_INTR_FIELD,
    &XPORT_INTR_CPU_STATUS_XLMAC_INTR_FIELD,
    &XPORT_INTR_CPU_STATUS_TX_TIMESYNC_FIFO_ENTRY_VALID_INTR_FIELD,
    &XPORT_INTR_CPU_STATUS_SGPHY_ENERGY_OFF_INTR_FIELD,
    &XPORT_INTR_CPU_STATUS_SGPHY_ENERGY_ON_INTR_FIELD,
    &XPORT_INTR_CPU_STATUS_MIB_REG_ERR_INTR_FIELD,
    &XPORT_INTR_CPU_STATUS_MAC_REG_ERR_INTR_FIELD,
    &XPORT_INTR_CPU_STATUS_UBUS_ERR_INTR_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec XPORT_INTR_CPU_STATUS_REG = 
{
    "CPU_STATUS",
#if RU_INCLUDE_DESC
    "PCI interrupt Clear Register",
    "This read-only register shows the current status of the interrupts for the CPU processor.  There is one "
    "bit per interrupt source.  The assertion of any of these interrupts will cause an interrupt to the CPU "
    "processor if the interrupt's mask bit is zero."
    "\nFor each bit: 1 = asserted. 0 = not asserted."
    "This write-only register is used to set interrupts for the CPU processor.  There is one bit per interrupt "
    "source.  Writing a 1 to a bit in this register will set the interrupt corresponding to that bit.  Writing "
    "a 0 has no effect.  Note that it is not necessary to read-modify-write this register because bits that have "
    "zeros written to them are not affected."
    "\nFor each bit: 1 = set the corresponding interrupt bit. 0 = no change."
    "This write-only register is used to clear interrupts for the CPU processor.  There is one bit per interrupt "
    "source.  Writing a 1 to a bit in this register will clear the interrupt corresponding to that bit.  Writing "
    "a 0 has no effect.  Note that it is not necessary to read-modify-write this register because bits that have "
    "zeros written to them are not affected."
    "\nFor each bit: 1 = clear the corresponding interrupt bit. 0 = no change."
    "This read-only register shows the current status of the interrupts for the PCI bus.  There is one bit per "
    "interrupt source.  The assertion of any of these interrupts will cause an interrupt to the PCI bus if the"
    "interrupt's mask bit is zero."
    "\nFor each bit: 1 = asserted. 0 = not asserted."
    "This write-only register is used to set interrupts for the PCI bus.  There is one bit per interrupt source."
    "Writing a 1 to a bit in this register will set the interrupt corresponding to that bit.  Writing a 0 has "
    "no effect.  Note that it is not necessary to read-modify-write this register because bits that have zeros "
    "written to them are not affected."
    "\nFor each bit: 1 = set the corresponding interrupt bit. 0 = no change."
    "This write-only register is used to clear interrupts for the PCI bus.  There is one bit per interrupt "
    "source.  Writing a 1 to a bit in this register will clear the interrupt corresponding to that bit.  Writing "
    "a 0 has no effect.  Note that it is not necessary to read-modify-write this register because bits that have "
    "zeros written to them are not affected."
    "\nFor each bit: 1 = clear the corresponding interrupt bit. 0 = no change.",
#endif
    XPORT_INTR_CPU_STATUS_REG_OFFSET,
    0,
    0,
    224,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    15,
    XPORT_INTR_CPU_STATUS_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: XPORT_INTR_CPU_SET
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XPORT_INTR_CPU_SET_FIELDS[] =
{
    &XPORT_INTR_CPU_SET_RESERVED0_FIELD,
    &XPORT_INTR_CPU_SET_MAB_STATUS_INTR_FIELD,
    &XPORT_INTR_CPU_SET_RX_REMOTE_FAULT_INTR_FIELD,
    &XPORT_INTR_CPU_SET_SERDES_MOD_DEF0_EVENT_INTR_FIELD,
    &XPORT_INTR_CPU_SET_DSERDES_SD_OFF_INTR_FIELD,
    &XPORT_INTR_CPU_SET_DSERDES_SD_ON_INTR_FIELD,
    &XPORT_INTR_CPU_SET_LINK_DOWN_INTR_FIELD,
    &XPORT_INTR_CPU_SET_LINK_UP_INTR_FIELD,
    &XPORT_INTR_CPU_SET_XLMAC_INTR_FIELD,
    &XPORT_INTR_CPU_SET_TX_TIMESYNC_FIFO_ENTRY_VALID_INTR_FIELD,
    &XPORT_INTR_CPU_SET_SGPHY_ENERGY_OFF_INTR_FIELD,
    &XPORT_INTR_CPU_SET_SGPHY_ENERGY_ON_INTR_FIELD,
    &XPORT_INTR_CPU_SET_MIB_REG_ERR_INTR_FIELD,
    &XPORT_INTR_CPU_SET_MAC_REG_ERR_INTR_FIELD,
    &XPORT_INTR_CPU_SET_UBUS_ERR_INTR_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec XPORT_INTR_CPU_SET_REG = 
{
    "CPU_SET",
#if RU_INCLUDE_DESC
    "PCI interrupt Clear Register",
    "This read-only register shows the current status of the interrupts for the CPU processor.  There is one "
    "bit per interrupt source.  The assertion of any of these interrupts will cause an interrupt to the CPU "
    "processor if the interrupt's mask bit is zero."
    "\nFor each bit: 1 = asserted. 0 = not asserted."
    "This write-only register is used to set interrupts for the CPU processor.  There is one bit per interrupt "
    "source.  Writing a 1 to a bit in this register will set the interrupt corresponding to that bit.  Writing "
    "a 0 has no effect.  Note that it is not necessary to read-modify-write this register because bits that have "
    "zeros written to them are not affected."
    "\nFor each bit: 1 = set the corresponding interrupt bit. 0 = no change."
    "This write-only register is used to clear interrupts for the CPU processor.  There is one bit per interrupt "
    "source.  Writing a 1 to a bit in this register will clear the interrupt corresponding to that bit.  Writing "
    "a 0 has no effect.  Note that it is not necessary to read-modify-write this register because bits that have "
    "zeros written to them are not affected."
    "\nFor each bit: 1 = clear the corresponding interrupt bit. 0 = no change."
    "This read-only register shows the current status of the interrupts for the PCI bus.  There is one bit per "
    "interrupt source.  The assertion of any of these interrupts will cause an interrupt to the PCI bus if the"
    "interrupt's mask bit is zero."
    "\nFor each bit: 1 = asserted. 0 = not asserted."
    "This write-only register is used to set interrupts for the PCI bus.  There is one bit per interrupt source."
    "Writing a 1 to a bit in this register will set the interrupt corresponding to that bit.  Writing a 0 has "
    "no effect.  Note that it is not necessary to read-modify-write this register because bits that have zeros "
    "written to them are not affected."
    "\nFor each bit: 1 = set the corresponding interrupt bit. 0 = no change."
    "This write-only register is used to clear interrupts for the PCI bus.  There is one bit per interrupt "
    "source.  Writing a 1 to a bit in this register will clear the interrupt corresponding to that bit.  Writing "
    "a 0 has no effect.  Note that it is not necessary to read-modify-write this register because bits that have "
    "zeros written to them are not affected."
    "\nFor each bit: 1 = clear the corresponding interrupt bit. 0 = no change.",
#endif
    XPORT_INTR_CPU_SET_REG_OFFSET,
    0,
    0,
    225,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    15,
    XPORT_INTR_CPU_SET_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: XPORT_INTR_CPU_CLEAR
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XPORT_INTR_CPU_CLEAR_FIELDS[] =
{
    &XPORT_INTR_CPU_CLEAR_RESERVED0_FIELD,
    &XPORT_INTR_CPU_CLEAR_MAB_STATUS_INTR_FIELD,
    &XPORT_INTR_CPU_CLEAR_RX_REMOTE_FAULT_INTR_FIELD,
    &XPORT_INTR_CPU_CLEAR_SERDES_MOD_DEF0_EVENT_INTR_FIELD,
    &XPORT_INTR_CPU_CLEAR_DSERDES_SD_OFF_INTR_FIELD,
    &XPORT_INTR_CPU_CLEAR_DSERDES_SD_ON_INTR_FIELD,
    &XPORT_INTR_CPU_CLEAR_LINK_DOWN_INTR_FIELD,
    &XPORT_INTR_CPU_CLEAR_LINK_UP_INTR_FIELD,
    &XPORT_INTR_CPU_CLEAR_XLMAC_INTR_FIELD,
    &XPORT_INTR_CPU_CLEAR_TX_TIMESYNC_FIFO_ENTRY_VALID_INTR_FIELD,
    &XPORT_INTR_CPU_CLEAR_SGPHY_ENERGY_OFF_INTR_FIELD,
    &XPORT_INTR_CPU_CLEAR_SGPHY_ENERGY_ON_INTR_FIELD,
    &XPORT_INTR_CPU_CLEAR_MIB_REG_ERR_INTR_FIELD,
    &XPORT_INTR_CPU_CLEAR_MAC_REG_ERR_INTR_FIELD,
    &XPORT_INTR_CPU_CLEAR_UBUS_ERR_INTR_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec XPORT_INTR_CPU_CLEAR_REG = 
{
    "CPU_CLEAR",
#if RU_INCLUDE_DESC
    "PCI interrupt Clear Register",
    "This read-only register shows the current status of the interrupts for the CPU processor.  There is one "
    "bit per interrupt source.  The assertion of any of these interrupts will cause an interrupt to the CPU "
    "processor if the interrupt's mask bit is zero."
    "\nFor each bit: 1 = asserted. 0 = not asserted."
    "This write-only register is used to set interrupts for the CPU processor.  There is one bit per interrupt "
    "source.  Writing a 1 to a bit in this register will set the interrupt corresponding to that bit.  Writing "
    "a 0 has no effect.  Note that it is not necessary to read-modify-write this register because bits that have "
    "zeros written to them are not affected."
    "\nFor each bit: 1 = set the corresponding interrupt bit. 0 = no change."
    "This write-only register is used to clear interrupts for the CPU processor.  There is one bit per interrupt "
    "source.  Writing a 1 to a bit in this register will clear the interrupt corresponding to that bit.  Writing "
    "a 0 has no effect.  Note that it is not necessary to read-modify-write this register because bits that have "
    "zeros written to them are not affected."
    "\nFor each bit: 1 = clear the corresponding interrupt bit. 0 = no change."
    "This read-only register shows the current status of the interrupts for the PCI bus.  There is one bit per "
    "interrupt source.  The assertion of any of these interrupts will cause an interrupt to the PCI bus if the"
    "interrupt's mask bit is zero."
    "\nFor each bit: 1 = asserted. 0 = not asserted."
    "This write-only register is used to set interrupts for the PCI bus.  There is one bit per interrupt source."
    "Writing a 1 to a bit in this register will set the interrupt corresponding to that bit.  Writing a 0 has "
    "no effect.  Note that it is not necessary to read-modify-write this register because bits that have zeros "
    "written to them are not affected."
    "\nFor each bit: 1 = set the corresponding interrupt bit. 0 = no change."
    "This write-only register is used to clear interrupts for the PCI bus.  There is one bit per interrupt "
    "source.  Writing a 1 to a bit in this register will clear the interrupt corresponding to that bit.  Writing "
    "a 0 has no effect.  Note that it is not necessary to read-modify-write this register because bits that have "
    "zeros written to them are not affected."
    "\nFor each bit: 1 = clear the corresponding interrupt bit. 0 = no change.",
#endif
    XPORT_INTR_CPU_CLEAR_REG_OFFSET,
    0,
    0,
    226,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    15,
    XPORT_INTR_CPU_CLEAR_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: XPORT_INTR_CPU_MASK_STATUS
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XPORT_INTR_CPU_MASK_STATUS_FIELDS[] =
{
    &XPORT_INTR_CPU_MASK_STATUS_RESERVED0_FIELD,
    &XPORT_INTR_CPU_MASK_STATUS_MAB_STATUS_INTR_MASK_FIELD,
    &XPORT_INTR_CPU_MASK_STATUS_RX_REMOTE_FAULT_INTR_MASK_FIELD,
    &XPORT_INTR_CPU_MASK_STATUS_SERDES_MOD_DEF0_EVENT_INTR_MASK_FIELD,
    &XPORT_INTR_CPU_MASK_STATUS_DSERDES_SD_OFF_INTR_MASK_FIELD,
    &XPORT_INTR_CPU_MASK_STATUS_DSERDES_SD_ON_INTR_MASK_FIELD,
    &XPORT_INTR_CPU_MASK_STATUS_LINK_DOWN_INTR_MASK_FIELD,
    &XPORT_INTR_CPU_MASK_STATUS_LINK_UP_INTR_MASK_FIELD,
    &XPORT_INTR_CPU_MASK_STATUS_XLMAC_INTR_MASK_FIELD,
    &XPORT_INTR_CPU_MASK_STATUS_TX_TIMESYNC_FIFO_ENTRY_VALID_INTR_MASK_FIELD,
    &XPORT_INTR_CPU_MASK_STATUS_SGPHY_ENERGY_OFF_INTR_MASK_FIELD,
    &XPORT_INTR_CPU_MASK_STATUS_SGPHY_ENERGY_ON_INTR_MASK_FIELD,
    &XPORT_INTR_CPU_MASK_STATUS_MIB_REG_ERR_INTR_MASK_FIELD,
    &XPORT_INTR_CPU_MASK_STATUS_MAC_REG_ERR_INTR_MASK_FIELD,
    &XPORT_INTR_CPU_MASK_STATUS_UBUS_ERR_INTR_MASK_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec XPORT_INTR_CPU_MASK_STATUS_REG = 
{
    "CPU_MASK_STATUS",
#if RU_INCLUDE_DESC
    "PCI interrupt Mask Clear Register",
    "This read-only register shows the current masking of the interrupts for the CPU processor.  There is one "
    "bit per interrupt source.  A value of 1 in a bit position indicates that the interrupt corresponding to "
    "that bit is currently masked.  Masking does not prevent the interrupt from appearing in the status register."
    "\nFor each bit: 1 = masked. 0 = not masked."
    "This write-only register is used to disable interrupts for the CPU processor.  There is one bit per interrupt "
    "source.  Writing a 1 to a bit in this register will set the mask bit for the corresponding interrupt.  Writing "
    "a 0 has no effect.  Note that it is not necessary to read-modify-write this register because bits that have "
    "zeros written to them are not affected."
    "\nFor each bit: 1 = disable the corresponding interrupt. 0 = no change."
    "This write-only register is used to enable interrupts for the CPU processor.  There is one bit per interrupt "
    "source.  Writing a 1 to a bit in this register will clear the mask bit for the corresponding interrupt.  Writing "
    "a 0 has no effect.  Note that it is not necessary to read-modify-write this register because bits that have "
    "zeros written to them are not affected."
    "\nFor each bit: 1 = enable the corresponding interrupt. 0 = no change."
    "This read-only register shows the current masking of the interrupts for the PCI bus.  There is one bit per"
    "interrupt source.  A value of 1 in a bit position indicates that the interrupt corresponding to that bit is"
    "currently masked.  Masking does not prevent the interrupt from appearing in the status register."
    "\nFor each bit: 1 = masked. 0 = not masked."
    "This write-only register is used to disable interrupts for the PCI bus.  There is one bit per interrupt source."
    "Writing a 1 to a bit in this register will set the mask bit for the corresponding interrupt.  Writing a 0 has no"
    "effect.  Note that it is not necessary to read-modify-write this register because bits that have zeros written "
    "to them are not affected."
    "\nFor each bit: 1 = disable the corresponding interrupt. 0 = no change."
    "This write-only register is used to enable interrupts for the PCI bus.  There is one bit per interrupt "
    "source.  Writing a 1 to a bit in this register will clear the mask bit for the corresponding interrupt.  Writing "
    "a 0 has no effect.  Note that it is not necessary to read-modify-write this register because bits that have "
    "zeros written to them are not affected."
    "\nFor each bit: 1 = enable the corresponding interrupt. 0 = no change.",
#endif
    XPORT_INTR_CPU_MASK_STATUS_REG_OFFSET,
    0,
    0,
    227,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    15,
    XPORT_INTR_CPU_MASK_STATUS_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: XPORT_INTR_CPU_MASK_SET
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XPORT_INTR_CPU_MASK_SET_FIELDS[] =
{
    &XPORT_INTR_CPU_MASK_SET_RESERVED0_FIELD,
    &XPORT_INTR_CPU_MASK_SET_MAB_STATUS_INTR_MASK_FIELD,
    &XPORT_INTR_CPU_MASK_SET_RX_REMOTE_FAULT_INTR_MASK_FIELD,
    &XPORT_INTR_CPU_MASK_SET_SERDES_MOD_DEF0_EVENT_INTR_MASK_FIELD,
    &XPORT_INTR_CPU_MASK_SET_DSERDES_SD_OFF_INTR_MASK_FIELD,
    &XPORT_INTR_CPU_MASK_SET_DSERDES_SD_ON_INTR_MASK_FIELD,
    &XPORT_INTR_CPU_MASK_SET_LINK_DOWN_INTR_MASK_FIELD,
    &XPORT_INTR_CPU_MASK_SET_LINK_UP_INTR_MASK_FIELD,
    &XPORT_INTR_CPU_MASK_SET_XLMAC_INTR_MASK_FIELD,
    &XPORT_INTR_CPU_MASK_SET_TX_TIMESYNC_FIFO_ENTRY_VALID_INTR_MASK_FIELD,
    &XPORT_INTR_CPU_MASK_SET_SGPHY_ENERGY_OFF_INTR_MASK_FIELD,
    &XPORT_INTR_CPU_MASK_SET_SGPHY_ENERGY_ON_INTR_MASK_FIELD,
    &XPORT_INTR_CPU_MASK_SET_MIB_REG_ERR_INTR_MASK_FIELD,
    &XPORT_INTR_CPU_MASK_SET_MAC_REG_ERR_INTR_MASK_FIELD,
    &XPORT_INTR_CPU_MASK_SET_UBUS_ERR_INTR_MASK_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec XPORT_INTR_CPU_MASK_SET_REG = 
{
    "CPU_MASK_SET",
#if RU_INCLUDE_DESC
    "PCI interrupt Mask Clear Register",
    "This read-only register shows the current masking of the interrupts for the CPU processor.  There is one "
    "bit per interrupt source.  A value of 1 in a bit position indicates that the interrupt corresponding to "
    "that bit is currently masked.  Masking does not prevent the interrupt from appearing in the status register."
    "\nFor each bit: 1 = masked. 0 = not masked."
    "This write-only register is used to disable interrupts for the CPU processor.  There is one bit per interrupt "
    "source.  Writing a 1 to a bit in this register will set the mask bit for the corresponding interrupt.  Writing "
    "a 0 has no effect.  Note that it is not necessary to read-modify-write this register because bits that have "
    "zeros written to them are not affected."
    "\nFor each bit: 1 = disable the corresponding interrupt. 0 = no change."
    "This write-only register is used to enable interrupts for the CPU processor.  There is one bit per interrupt "
    "source.  Writing a 1 to a bit in this register will clear the mask bit for the corresponding interrupt.  Writing "
    "a 0 has no effect.  Note that it is not necessary to read-modify-write this register because bits that have "
    "zeros written to them are not affected."
    "\nFor each bit: 1 = enable the corresponding interrupt. 0 = no change."
    "This read-only register shows the current masking of the interrupts for the PCI bus.  There is one bit per"
    "interrupt source.  A value of 1 in a bit position indicates that the interrupt corresponding to that bit is"
    "currently masked.  Masking does not prevent the interrupt from appearing in the status register."
    "\nFor each bit: 1 = masked. 0 = not masked."
    "This write-only register is used to disable interrupts for the PCI bus.  There is one bit per interrupt source."
    "Writing a 1 to a bit in this register will set the mask bit for the corresponding interrupt.  Writing a 0 has no"
    "effect.  Note that it is not necessary to read-modify-write this register because bits that have zeros written "
    "to them are not affected."
    "\nFor each bit: 1 = disable the corresponding interrupt. 0 = no change."
    "This write-only register is used to enable interrupts for the PCI bus.  There is one bit per interrupt "
    "source.  Writing a 1 to a bit in this register will clear the mask bit for the corresponding interrupt.  Writing "
    "a 0 has no effect.  Note that it is not necessary to read-modify-write this register because bits that have "
    "zeros written to them are not affected."
    "\nFor each bit: 1 = enable the corresponding interrupt. 0 = no change.",
#endif
    XPORT_INTR_CPU_MASK_SET_REG_OFFSET,
    0,
    0,
    228,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    15,
    XPORT_INTR_CPU_MASK_SET_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: XPORT_INTR_CPU_MASK_CLEAR
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XPORT_INTR_CPU_MASK_CLEAR_FIELDS[] =
{
    &XPORT_INTR_CPU_MASK_CLEAR_RESERVED0_FIELD,
    &XPORT_INTR_CPU_MASK_CLEAR_MAB_STATUS_INTR_MASK_FIELD,
    &XPORT_INTR_CPU_MASK_CLEAR_RX_REMOTE_FAULT_INTR_MASK_FIELD,
    &XPORT_INTR_CPU_MASK_CLEAR_SERDES_MOD_DEF0_EVENT_INTR_MASK_FIELD,
    &XPORT_INTR_CPU_MASK_CLEAR_DSERDES_SD_OFF_INTR_MASK_FIELD,
    &XPORT_INTR_CPU_MASK_CLEAR_DSERDES_SD_ON_INTR_MASK_FIELD,
    &XPORT_INTR_CPU_MASK_CLEAR_LINK_DOWN_INTR_MASK_FIELD,
    &XPORT_INTR_CPU_MASK_CLEAR_LINK_UP_INTR_MASK_FIELD,
    &XPORT_INTR_CPU_MASK_CLEAR_XLMAC_INTR_MASK_FIELD,
    &XPORT_INTR_CPU_MASK_CLEAR_TX_TIMESYNC_FIFO_ENTRY_VALID_INTR_MASK_FIELD,
    &XPORT_INTR_CPU_MASK_CLEAR_SGPHY_ENERGY_OFF_INTR_MASK_FIELD,
    &XPORT_INTR_CPU_MASK_CLEAR_SGPHY_ENERGY_ON_INTR_MASK_FIELD,
    &XPORT_INTR_CPU_MASK_CLEAR_MIB_REG_ERR_INTR_MASK_FIELD,
    &XPORT_INTR_CPU_MASK_CLEAR_MAC_REG_ERR_INTR_MASK_FIELD,
    &XPORT_INTR_CPU_MASK_CLEAR_UBUS_ERR_INTR_MASK_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec XPORT_INTR_CPU_MASK_CLEAR_REG = 
{
    "CPU_MASK_CLEAR",
#if RU_INCLUDE_DESC
    "PCI interrupt Mask Clear Register",
    "This read-only register shows the current masking of the interrupts for the CPU processor.  There is one "
    "bit per interrupt source.  A value of 1 in a bit position indicates that the interrupt corresponding to "
    "that bit is currently masked.  Masking does not prevent the interrupt from appearing in the status register."
    "\nFor each bit: 1 = masked. 0 = not masked."
    "This write-only register is used to disable interrupts for the CPU processor.  There is one bit per interrupt "
    "source.  Writing a 1 to a bit in this register will set the mask bit for the corresponding interrupt.  Writing "
    "a 0 has no effect.  Note that it is not necessary to read-modify-write this register because bits that have "
    "zeros written to them are not affected."
    "\nFor each bit: 1 = disable the corresponding interrupt. 0 = no change."
    "This write-only register is used to enable interrupts for the CPU processor.  There is one bit per interrupt "
    "source.  Writing a 1 to a bit in this register will clear the mask bit for the corresponding interrupt.  Writing "
    "a 0 has no effect.  Note that it is not necessary to read-modify-write this register because bits that have "
    "zeros written to them are not affected."
    "\nFor each bit: 1 = enable the corresponding interrupt. 0 = no change."
    "This read-only register shows the current masking of the interrupts for the PCI bus.  There is one bit per"
    "interrupt source.  A value of 1 in a bit position indicates that the interrupt corresponding to that bit is"
    "currently masked.  Masking does not prevent the interrupt from appearing in the status register."
    "\nFor each bit: 1 = masked. 0 = not masked."
    "This write-only register is used to disable interrupts for the PCI bus.  There is one bit per interrupt source."
    "Writing a 1 to a bit in this register will set the mask bit for the corresponding interrupt.  Writing a 0 has no"
    "effect.  Note that it is not necessary to read-modify-write this register because bits that have zeros written "
    "to them are not affected."
    "\nFor each bit: 1 = disable the corresponding interrupt. 0 = no change."
    "This write-only register is used to enable interrupts for the PCI bus.  There is one bit per interrupt "
    "source.  Writing a 1 to a bit in this register will clear the mask bit for the corresponding interrupt.  Writing "
    "a 0 has no effect.  Note that it is not necessary to read-modify-write this register because bits that have "
    "zeros written to them are not affected."
    "\nFor each bit: 1 = enable the corresponding interrupt. 0 = no change.",
#endif
    XPORT_INTR_CPU_MASK_CLEAR_REG_OFFSET,
    0,
    0,
    229,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    15,
    XPORT_INTR_CPU_MASK_CLEAR_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: XPORT_INTR_PCI_STATUS
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XPORT_INTR_PCI_STATUS_FIELDS[] =
{
    &XPORT_INTR_PCI_STATUS_RESERVED0_FIELD,
    &XPORT_INTR_PCI_STATUS_MAB_STATUS_INTR_FIELD,
    &XPORT_INTR_PCI_STATUS_RX_REMOTE_FAULT_INTR_FIELD,
    &XPORT_INTR_PCI_STATUS_SERDES_MOD_DEF0_EVENT_INTR_FIELD,
    &XPORT_INTR_PCI_STATUS_DSERDES_SD_OFF_INTR_FIELD,
    &XPORT_INTR_PCI_STATUS_DSERDES_SD_ON_INTR_FIELD,
    &XPORT_INTR_PCI_STATUS_LINK_DOWN_INTR_FIELD,
    &XPORT_INTR_PCI_STATUS_LINK_UP_INTR_FIELD,
    &XPORT_INTR_PCI_STATUS_XLMAC_INTR_FIELD,
    &XPORT_INTR_PCI_STATUS_TX_TIMESYNC_FIFO_ENTRY_VALID_INTR_FIELD,
    &XPORT_INTR_PCI_STATUS_SGPHY_ENERGY_OFF_INTR_FIELD,
    &XPORT_INTR_PCI_STATUS_SGPHY_ENERGY_ON_INTR_FIELD,
    &XPORT_INTR_PCI_STATUS_MIB_REG_ERR_INTR_FIELD,
    &XPORT_INTR_PCI_STATUS_MAC_REG_ERR_INTR_FIELD,
    &XPORT_INTR_PCI_STATUS_UBUS_ERR_INTR_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec XPORT_INTR_PCI_STATUS_REG = 
{
    "PCI_STATUS",
#if RU_INCLUDE_DESC
    "PCI interrupt Clear Register",
    "This read-only register shows the current status of the interrupts for the CPU processor.  There is one "
    "bit per interrupt source.  The assertion of any of these interrupts will cause an interrupt to the CPU "
    "processor if the interrupt's mask bit is zero."
    "\nFor each bit: 1 = asserted. 0 = not asserted."
    "This write-only register is used to set interrupts for the CPU processor.  There is one bit per interrupt "
    "source.  Writing a 1 to a bit in this register will set the interrupt corresponding to that bit.  Writing "
    "a 0 has no effect.  Note that it is not necessary to read-modify-write this register because bits that have "
    "zeros written to them are not affected."
    "\nFor each bit: 1 = set the corresponding interrupt bit. 0 = no change."
    "This write-only register is used to clear interrupts for the CPU processor.  There is one bit per interrupt "
    "source.  Writing a 1 to a bit in this register will clear the interrupt corresponding to that bit.  Writing "
    "a 0 has no effect.  Note that it is not necessary to read-modify-write this register because bits that have "
    "zeros written to them are not affected."
    "\nFor each bit: 1 = clear the corresponding interrupt bit. 0 = no change."
    "This read-only register shows the current status of the interrupts for the PCI bus.  There is one bit per "
    "interrupt source.  The assertion of any of these interrupts will cause an interrupt to the PCI bus if the"
    "interrupt's mask bit is zero."
    "\nFor each bit: 1 = asserted. 0 = not asserted."
    "This write-only register is used to set interrupts for the PCI bus.  There is one bit per interrupt source."
    "Writing a 1 to a bit in this register will set the interrupt corresponding to that bit.  Writing a 0 has "
    "no effect.  Note that it is not necessary to read-modify-write this register because bits that have zeros "
    "written to them are not affected."
    "\nFor each bit: 1 = set the corresponding interrupt bit. 0 = no change."
    "This write-only register is used to clear interrupts for the PCI bus.  There is one bit per interrupt "
    "source.  Writing a 1 to a bit in this register will clear the interrupt corresponding to that bit.  Writing "
    "a 0 has no effect.  Note that it is not necessary to read-modify-write this register because bits that have "
    "zeros written to them are not affected."
    "\nFor each bit: 1 = clear the corresponding interrupt bit. 0 = no change.",
#endif
    XPORT_INTR_PCI_STATUS_REG_OFFSET,
    0,
    0,
    230,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    15,
    XPORT_INTR_PCI_STATUS_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: XPORT_INTR_PCI_SET
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XPORT_INTR_PCI_SET_FIELDS[] =
{
    &XPORT_INTR_PCI_SET_RESERVED0_FIELD,
    &XPORT_INTR_PCI_SET_MAB_STATUS_INTR_FIELD,
    &XPORT_INTR_PCI_SET_RX_REMOTE_FAULT_INTR_FIELD,
    &XPORT_INTR_PCI_SET_SERDES_MOD_DEF0_EVENT_INTR_FIELD,
    &XPORT_INTR_PCI_SET_DSERDES_SD_OFF_INTR_FIELD,
    &XPORT_INTR_PCI_SET_DSERDES_SD_ON_INTR_FIELD,
    &XPORT_INTR_PCI_SET_LINK_DOWN_INTR_FIELD,
    &XPORT_INTR_PCI_SET_LINK_UP_INTR_FIELD,
    &XPORT_INTR_PCI_SET_XLMAC_INTR_FIELD,
    &XPORT_INTR_PCI_SET_TX_TIMESYNC_FIFO_ENTRY_VALID_INTR_FIELD,
    &XPORT_INTR_PCI_SET_SGPHY_ENERGY_OFF_INTR_FIELD,
    &XPORT_INTR_PCI_SET_SGPHY_ENERGY_ON_INTR_FIELD,
    &XPORT_INTR_PCI_SET_MIB_REG_ERR_INTR_FIELD,
    &XPORT_INTR_PCI_SET_MAC_REG_ERR_INTR_FIELD,
    &XPORT_INTR_PCI_SET_UBUS_ERR_INTR_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec XPORT_INTR_PCI_SET_REG = 
{
    "PCI_SET",
#if RU_INCLUDE_DESC
    "PCI interrupt Clear Register",
    "This read-only register shows the current status of the interrupts for the CPU processor.  There is one "
    "bit per interrupt source.  The assertion of any of these interrupts will cause an interrupt to the CPU "
    "processor if the interrupt's mask bit is zero."
    "\nFor each bit: 1 = asserted. 0 = not asserted."
    "This write-only register is used to set interrupts for the CPU processor.  There is one bit per interrupt "
    "source.  Writing a 1 to a bit in this register will set the interrupt corresponding to that bit.  Writing "
    "a 0 has no effect.  Note that it is not necessary to read-modify-write this register because bits that have "
    "zeros written to them are not affected."
    "\nFor each bit: 1 = set the corresponding interrupt bit. 0 = no change."
    "This write-only register is used to clear interrupts for the CPU processor.  There is one bit per interrupt "
    "source.  Writing a 1 to a bit in this register will clear the interrupt corresponding to that bit.  Writing "
    "a 0 has no effect.  Note that it is not necessary to read-modify-write this register because bits that have "
    "zeros written to them are not affected."
    "\nFor each bit: 1 = clear the corresponding interrupt bit. 0 = no change."
    "This read-only register shows the current status of the interrupts for the PCI bus.  There is one bit per "
    "interrupt source.  The assertion of any of these interrupts will cause an interrupt to the PCI bus if the"
    "interrupt's mask bit is zero."
    "\nFor each bit: 1 = asserted. 0 = not asserted."
    "This write-only register is used to set interrupts for the PCI bus.  There is one bit per interrupt source."
    "Writing a 1 to a bit in this register will set the interrupt corresponding to that bit.  Writing a 0 has "
    "no effect.  Note that it is not necessary to read-modify-write this register because bits that have zeros "
    "written to them are not affected."
    "\nFor each bit: 1 = set the corresponding interrupt bit. 0 = no change."
    "This write-only register is used to clear interrupts for the PCI bus.  There is one bit per interrupt "
    "source.  Writing a 1 to a bit in this register will clear the interrupt corresponding to that bit.  Writing "
    "a 0 has no effect.  Note that it is not necessary to read-modify-write this register because bits that have "
    "zeros written to them are not affected."
    "\nFor each bit: 1 = clear the corresponding interrupt bit. 0 = no change.",
#endif
    XPORT_INTR_PCI_SET_REG_OFFSET,
    0,
    0,
    231,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    15,
    XPORT_INTR_PCI_SET_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: XPORT_INTR_PCI_CLEAR
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XPORT_INTR_PCI_CLEAR_FIELDS[] =
{
    &XPORT_INTR_PCI_CLEAR_RESERVED0_FIELD,
    &XPORT_INTR_PCI_CLEAR_MAB_STATUS_INTR_FIELD,
    &XPORT_INTR_PCI_CLEAR_RX_REMOTE_FAULT_INTR_FIELD,
    &XPORT_INTR_PCI_CLEAR_SERDES_MOD_DEF0_EVENT_INTR_FIELD,
    &XPORT_INTR_PCI_CLEAR_DSERDES_SD_OFF_INTR_FIELD,
    &XPORT_INTR_PCI_CLEAR_DSERDES_SD_ON_INTR_FIELD,
    &XPORT_INTR_PCI_CLEAR_LINK_DOWN_INTR_FIELD,
    &XPORT_INTR_PCI_CLEAR_LINK_UP_INTR_FIELD,
    &XPORT_INTR_PCI_CLEAR_XLMAC_INTR_FIELD,
    &XPORT_INTR_PCI_CLEAR_TX_TIMESYNC_FIFO_ENTRY_VALID_INTR_FIELD,
    &XPORT_INTR_PCI_CLEAR_SGPHY_ENERGY_OFF_INTR_FIELD,
    &XPORT_INTR_PCI_CLEAR_SGPHY_ENERGY_ON_INTR_FIELD,
    &XPORT_INTR_PCI_CLEAR_MIB_REG_ERR_INTR_FIELD,
    &XPORT_INTR_PCI_CLEAR_MAC_REG_ERR_INTR_FIELD,
    &XPORT_INTR_PCI_CLEAR_UBUS_ERR_INTR_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec XPORT_INTR_PCI_CLEAR_REG = 
{
    "PCI_CLEAR",
#if RU_INCLUDE_DESC
    "PCI interrupt Clear Register",
    "This read-only register shows the current status of the interrupts for the CPU processor.  There is one "
    "bit per interrupt source.  The assertion of any of these interrupts will cause an interrupt to the CPU "
    "processor if the interrupt's mask bit is zero."
    "\nFor each bit: 1 = asserted. 0 = not asserted."
    "This write-only register is used to set interrupts for the CPU processor.  There is one bit per interrupt "
    "source.  Writing a 1 to a bit in this register will set the interrupt corresponding to that bit.  Writing "
    "a 0 has no effect.  Note that it is not necessary to read-modify-write this register because bits that have "
    "zeros written to them are not affected."
    "\nFor each bit: 1 = set the corresponding interrupt bit. 0 = no change."
    "This write-only register is used to clear interrupts for the CPU processor.  There is one bit per interrupt "
    "source.  Writing a 1 to a bit in this register will clear the interrupt corresponding to that bit.  Writing "
    "a 0 has no effect.  Note that it is not necessary to read-modify-write this register because bits that have "
    "zeros written to them are not affected."
    "\nFor each bit: 1 = clear the corresponding interrupt bit. 0 = no change."
    "This read-only register shows the current status of the interrupts for the PCI bus.  There is one bit per "
    "interrupt source.  The assertion of any of these interrupts will cause an interrupt to the PCI bus if the"
    "interrupt's mask bit is zero."
    "\nFor each bit: 1 = asserted. 0 = not asserted."
    "This write-only register is used to set interrupts for the PCI bus.  There is one bit per interrupt source."
    "Writing a 1 to a bit in this register will set the interrupt corresponding to that bit.  Writing a 0 has "
    "no effect.  Note that it is not necessary to read-modify-write this register because bits that have zeros "
    "written to them are not affected."
    "\nFor each bit: 1 = set the corresponding interrupt bit. 0 = no change."
    "This write-only register is used to clear interrupts for the PCI bus.  There is one bit per interrupt "
    "source.  Writing a 1 to a bit in this register will clear the interrupt corresponding to that bit.  Writing "
    "a 0 has no effect.  Note that it is not necessary to read-modify-write this register because bits that have "
    "zeros written to them are not affected."
    "\nFor each bit: 1 = clear the corresponding interrupt bit. 0 = no change.",
#endif
    XPORT_INTR_PCI_CLEAR_REG_OFFSET,
    0,
    0,
    232,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    15,
    XPORT_INTR_PCI_CLEAR_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: XPORT_INTR_PCI_MASK_STATUS
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XPORT_INTR_PCI_MASK_STATUS_FIELDS[] =
{
    &XPORT_INTR_PCI_MASK_STATUS_RESERVED0_FIELD,
    &XPORT_INTR_PCI_MASK_STATUS_MAB_STATUS_INTR_MASK_FIELD,
    &XPORT_INTR_PCI_MASK_STATUS_RX_REMOTE_FAULT_INTR_MASK_FIELD,
    &XPORT_INTR_PCI_MASK_STATUS_SERDES_MOD_DEF0_EVENT_INTR_MASK_FIELD,
    &XPORT_INTR_PCI_MASK_STATUS_DSERDES_SD_OFF_INTR_MASK_FIELD,
    &XPORT_INTR_PCI_MASK_STATUS_DSERDES_SD_ON_INTR_MASK_FIELD,
    &XPORT_INTR_PCI_MASK_STATUS_LINK_DOWN_INTR_MASK_FIELD,
    &XPORT_INTR_PCI_MASK_STATUS_LINK_UP_INTR_MASK_FIELD,
    &XPORT_INTR_PCI_MASK_STATUS_XLMAC_INTR_MASK_FIELD,
    &XPORT_INTR_PCI_MASK_STATUS_TX_TIMESYNC_FIFO_ENTRY_VALID_INTR_MASK_FIELD,
    &XPORT_INTR_PCI_MASK_STATUS_SGPHY_ENERGY_OFF_INTR_MASK_FIELD,
    &XPORT_INTR_PCI_MASK_STATUS_SGPHY_ENERGY_ON_INTR_MASK_FIELD,
    &XPORT_INTR_PCI_MASK_STATUS_MIB_REG_ERR_INTR_MASK_FIELD,
    &XPORT_INTR_PCI_MASK_STATUS_MAC_REG_ERR_INTR_MASK_FIELD,
    &XPORT_INTR_PCI_MASK_STATUS_UBUS_ERR_INTR_MASK_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec XPORT_INTR_PCI_MASK_STATUS_REG = 
{
    "PCI_MASK_STATUS",
#if RU_INCLUDE_DESC
    "PCI interrupt Mask Clear Register",
    "This read-only register shows the current masking of the interrupts for the CPU processor.  There is one "
    "bit per interrupt source.  A value of 1 in a bit position indicates that the interrupt corresponding to "
    "that bit is currently masked.  Masking does not prevent the interrupt from appearing in the status register."
    "\nFor each bit: 1 = masked. 0 = not masked."
    "This write-only register is used to disable interrupts for the CPU processor.  There is one bit per interrupt "
    "source.  Writing a 1 to a bit in this register will set the mask bit for the corresponding interrupt.  Writing "
    "a 0 has no effect.  Note that it is not necessary to read-modify-write this register because bits that have "
    "zeros written to them are not affected."
    "\nFor each bit: 1 = disable the corresponding interrupt. 0 = no change."
    "This write-only register is used to enable interrupts for the CPU processor.  There is one bit per interrupt "
    "source.  Writing a 1 to a bit in this register will clear the mask bit for the corresponding interrupt.  Writing "
    "a 0 has no effect.  Note that it is not necessary to read-modify-write this register because bits that have "
    "zeros written to them are not affected."
    "\nFor each bit: 1 = enable the corresponding interrupt. 0 = no change."
    "This read-only register shows the current masking of the interrupts for the PCI bus.  There is one bit per"
    "interrupt source.  A value of 1 in a bit position indicates that the interrupt corresponding to that bit is"
    "currently masked.  Masking does not prevent the interrupt from appearing in the status register."
    "\nFor each bit: 1 = masked. 0 = not masked."
    "This write-only register is used to disable interrupts for the PCI bus.  There is one bit per interrupt source."
    "Writing a 1 to a bit in this register will set the mask bit for the corresponding interrupt.  Writing a 0 has no"
    "effect.  Note that it is not necessary to read-modify-write this register because bits that have zeros written "
    "to them are not affected."
    "\nFor each bit: 1 = disable the corresponding interrupt. 0 = no change."
    "This write-only register is used to enable interrupts for the PCI bus.  There is one bit per interrupt "
    "source.  Writing a 1 to a bit in this register will clear the mask bit for the corresponding interrupt.  Writing "
    "a 0 has no effect.  Note that it is not necessary to read-modify-write this register because bits that have "
    "zeros written to them are not affected."
    "\nFor each bit: 1 = enable the corresponding interrupt. 0 = no change.",
#endif
    XPORT_INTR_PCI_MASK_STATUS_REG_OFFSET,
    0,
    0,
    233,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    15,
    XPORT_INTR_PCI_MASK_STATUS_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: XPORT_INTR_PCI_MASK_SET
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XPORT_INTR_PCI_MASK_SET_FIELDS[] =
{
    &XPORT_INTR_PCI_MASK_SET_RESERVED0_FIELD,
    &XPORT_INTR_PCI_MASK_SET_MAB_STATUS_INTR_MASK_FIELD,
    &XPORT_INTR_PCI_MASK_SET_RX_REMOTE_FAULT_INTR_MASK_FIELD,
    &XPORT_INTR_PCI_MASK_SET_SERDES_MOD_DEF0_EVENT_INTR_MASK_FIELD,
    &XPORT_INTR_PCI_MASK_SET_DSERDES_SD_OFF_INTR_MASK_FIELD,
    &XPORT_INTR_PCI_MASK_SET_DSERDES_SD_ON_INTR_MASK_FIELD,
    &XPORT_INTR_PCI_MASK_SET_LINK_DOWN_INTR_MASK_FIELD,
    &XPORT_INTR_PCI_MASK_SET_LINK_UP_INTR_MASK_FIELD,
    &XPORT_INTR_PCI_MASK_SET_XLMAC_INTR_MASK_FIELD,
    &XPORT_INTR_PCI_MASK_SET_TX_TIMESYNC_FIFO_ENTRY_VALID_INTR_MASK_FIELD,
    &XPORT_INTR_PCI_MASK_SET_SGPHY_ENERGY_OFF_INTR_MASK_FIELD,
    &XPORT_INTR_PCI_MASK_SET_SGPHY_ENERGY_ON_INTR_MASK_FIELD,
    &XPORT_INTR_PCI_MASK_SET_MIB_REG_ERR_INTR_MASK_FIELD,
    &XPORT_INTR_PCI_MASK_SET_MAC_REG_ERR_INTR_MASK_FIELD,
    &XPORT_INTR_PCI_MASK_SET_UBUS_ERR_INTR_MASK_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec XPORT_INTR_PCI_MASK_SET_REG = 
{
    "PCI_MASK_SET",
#if RU_INCLUDE_DESC
    "PCI interrupt Mask Clear Register",
    "This read-only register shows the current masking of the interrupts for the CPU processor.  There is one "
    "bit per interrupt source.  A value of 1 in a bit position indicates that the interrupt corresponding to "
    "that bit is currently masked.  Masking does not prevent the interrupt from appearing in the status register."
    "\nFor each bit: 1 = masked. 0 = not masked."
    "This write-only register is used to disable interrupts for the CPU processor.  There is one bit per interrupt "
    "source.  Writing a 1 to a bit in this register will set the mask bit for the corresponding interrupt.  Writing "
    "a 0 has no effect.  Note that it is not necessary to read-modify-write this register because bits that have "
    "zeros written to them are not affected."
    "\nFor each bit: 1 = disable the corresponding interrupt. 0 = no change."
    "This write-only register is used to enable interrupts for the CPU processor.  There is one bit per interrupt "
    "source.  Writing a 1 to a bit in this register will clear the mask bit for the corresponding interrupt.  Writing "
    "a 0 has no effect.  Note that it is not necessary to read-modify-write this register because bits that have "
    "zeros written to them are not affected."
    "\nFor each bit: 1 = enable the corresponding interrupt. 0 = no change."
    "This read-only register shows the current masking of the interrupts for the PCI bus.  There is one bit per"
    "interrupt source.  A value of 1 in a bit position indicates that the interrupt corresponding to that bit is"
    "currently masked.  Masking does not prevent the interrupt from appearing in the status register."
    "\nFor each bit: 1 = masked. 0 = not masked."
    "This write-only register is used to disable interrupts for the PCI bus.  There is one bit per interrupt source."
    "Writing a 1 to a bit in this register will set the mask bit for the corresponding interrupt.  Writing a 0 has no"
    "effect.  Note that it is not necessary to read-modify-write this register because bits that have zeros written "
    "to them are not affected."
    "\nFor each bit: 1 = disable the corresponding interrupt. 0 = no change."
    "This write-only register is used to enable interrupts for the PCI bus.  There is one bit per interrupt "
    "source.  Writing a 1 to a bit in this register will clear the mask bit for the corresponding interrupt.  Writing "
    "a 0 has no effect.  Note that it is not necessary to read-modify-write this register because bits that have "
    "zeros written to them are not affected."
    "\nFor each bit: 1 = enable the corresponding interrupt. 0 = no change.",
#endif
    XPORT_INTR_PCI_MASK_SET_REG_OFFSET,
    0,
    0,
    234,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    15,
    XPORT_INTR_PCI_MASK_SET_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: XPORT_INTR_PCI_MASK_CLEAR
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XPORT_INTR_PCI_MASK_CLEAR_FIELDS[] =
{
    &XPORT_INTR_PCI_MASK_CLEAR_RESERVED0_FIELD,
    &XPORT_INTR_PCI_MASK_CLEAR_MAB_STATUS_INTR_MASK_FIELD,
    &XPORT_INTR_PCI_MASK_CLEAR_RX_REMOTE_FAULT_INTR_MASK_FIELD,
    &XPORT_INTR_PCI_MASK_CLEAR_SERDES_MOD_DEF0_EVENT_INTR_MASK_FIELD,
    &XPORT_INTR_PCI_MASK_CLEAR_DSERDES_SD_OFF_INTR_MASK_FIELD,
    &XPORT_INTR_PCI_MASK_CLEAR_DSERDES_SD_ON_INTR_MASK_FIELD,
    &XPORT_INTR_PCI_MASK_CLEAR_LINK_DOWN_INTR_MASK_FIELD,
    &XPORT_INTR_PCI_MASK_CLEAR_LINK_UP_INTR_MASK_FIELD,
    &XPORT_INTR_PCI_MASK_CLEAR_XLMAC_INTR_MASK_FIELD,
    &XPORT_INTR_PCI_MASK_CLEAR_TX_TIMESYNC_FIFO_ENTRY_VALID_INTR_MASK_FIELD,
    &XPORT_INTR_PCI_MASK_CLEAR_SGPHY_ENERGY_OFF_INTR_MASK_FIELD,
    &XPORT_INTR_PCI_MASK_CLEAR_SGPHY_ENERGY_ON_INTR_MASK_FIELD,
    &XPORT_INTR_PCI_MASK_CLEAR_MIB_REG_ERR_INTR_MASK_FIELD,
    &XPORT_INTR_PCI_MASK_CLEAR_MAC_REG_ERR_INTR_MASK_FIELD,
    &XPORT_INTR_PCI_MASK_CLEAR_UBUS_ERR_INTR_MASK_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec XPORT_INTR_PCI_MASK_CLEAR_REG = 
{
    "PCI_MASK_CLEAR",
#if RU_INCLUDE_DESC
    "PCI interrupt Mask Clear Register",
    "This read-only register shows the current masking of the interrupts for the CPU processor.  There is one "
    "bit per interrupt source.  A value of 1 in a bit position indicates that the interrupt corresponding to "
    "that bit is currently masked.  Masking does not prevent the interrupt from appearing in the status register."
    "\nFor each bit: 1 = masked. 0 = not masked."
    "This write-only register is used to disable interrupts for the CPU processor.  There is one bit per interrupt "
    "source.  Writing a 1 to a bit in this register will set the mask bit for the corresponding interrupt.  Writing "
    "a 0 has no effect.  Note that it is not necessary to read-modify-write this register because bits that have "
    "zeros written to them are not affected."
    "\nFor each bit: 1 = disable the corresponding interrupt. 0 = no change."
    "This write-only register is used to enable interrupts for the CPU processor.  There is one bit per interrupt "
    "source.  Writing a 1 to a bit in this register will clear the mask bit for the corresponding interrupt.  Writing "
    "a 0 has no effect.  Note that it is not necessary to read-modify-write this register because bits that have "
    "zeros written to them are not affected."
    "\nFor each bit: 1 = enable the corresponding interrupt. 0 = no change."
    "This read-only register shows the current masking of the interrupts for the PCI bus.  There is one bit per"
    "interrupt source.  A value of 1 in a bit position indicates that the interrupt corresponding to that bit is"
    "currently masked.  Masking does not prevent the interrupt from appearing in the status register."
    "\nFor each bit: 1 = masked. 0 = not masked."
    "This write-only register is used to disable interrupts for the PCI bus.  There is one bit per interrupt source."
    "Writing a 1 to a bit in this register will set the mask bit for the corresponding interrupt.  Writing a 0 has no"
    "effect.  Note that it is not necessary to read-modify-write this register because bits that have zeros written "
    "to them are not affected."
    "\nFor each bit: 1 = disable the corresponding interrupt. 0 = no change."
    "This write-only register is used to enable interrupts for the PCI bus.  There is one bit per interrupt "
    "source.  Writing a 1 to a bit in this register will clear the mask bit for the corresponding interrupt.  Writing "
    "a 0 has no effect.  Note that it is not necessary to read-modify-write this register because bits that have "
    "zeros written to them are not affected."
    "\nFor each bit: 1 = enable the corresponding interrupt. 0 = no change.",
#endif
    XPORT_INTR_PCI_MASK_CLEAR_REG_OFFSET,
    0,
    0,
    235,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    15,
    XPORT_INTR_PCI_MASK_CLEAR_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Block: XPORT_INTR
 ******************************************************************************/
static const ru_reg_rec *XPORT_INTR_REGS[] =
{
    &XPORT_INTR_CPU_STATUS_REG,
    &XPORT_INTR_CPU_SET_REG,
    &XPORT_INTR_CPU_CLEAR_REG,
    &XPORT_INTR_CPU_MASK_STATUS_REG,
    &XPORT_INTR_CPU_MASK_SET_REG,
    &XPORT_INTR_CPU_MASK_CLEAR_REG,
    &XPORT_INTR_PCI_STATUS_REG,
    &XPORT_INTR_PCI_SET_REG,
    &XPORT_INTR_PCI_CLEAR_REG,
    &XPORT_INTR_PCI_MASK_STATUS_REG,
    &XPORT_INTR_PCI_MASK_SET_REG,
    &XPORT_INTR_PCI_MASK_CLEAR_REG,
};

unsigned long XPORT_INTR_ADDRS[] =
{
    0x8013b200,
};

const ru_block_rec XPORT_INTR_BLOCK = 
{
    "XPORT_INTR",
    XPORT_INTR_ADDRS,
    1,
    12,
    XPORT_INTR_REGS
};

/* End of file XPORT_INTR.c */
