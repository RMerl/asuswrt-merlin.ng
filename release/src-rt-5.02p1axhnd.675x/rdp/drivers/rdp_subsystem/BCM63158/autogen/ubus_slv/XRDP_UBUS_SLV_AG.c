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
 * Field: UBUS_SLV_VPB_BASE_BASE
 ******************************************************************************/
const ru_field_rec UBUS_SLV_VPB_BASE_BASE_FIELD =
{
    "BASE",
#if RU_INCLUDE_DESC
    "Base",
    "base",
#endif
    UBUS_SLV_VPB_BASE_BASE_FIELD_MASK,
    0,
    UBUS_SLV_VPB_BASE_BASE_FIELD_WIDTH,
    UBUS_SLV_VPB_BASE_BASE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UBUS_SLV_VPB_MASK_MASK
 ******************************************************************************/
const ru_field_rec UBUS_SLV_VPB_MASK_MASK_FIELD =
{
    "MASK",
#if RU_INCLUDE_DESC
    "Mask",
    "mask",
#endif
    UBUS_SLV_VPB_MASK_MASK_FIELD_MASK,
    0,
    UBUS_SLV_VPB_MASK_MASK_FIELD_WIDTH,
    UBUS_SLV_VPB_MASK_MASK_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UBUS_SLV_APB_BASE_BASE
 ******************************************************************************/
const ru_field_rec UBUS_SLV_APB_BASE_BASE_FIELD =
{
    "BASE",
#if RU_INCLUDE_DESC
    "Base",
    "base",
#endif
    UBUS_SLV_APB_BASE_BASE_FIELD_MASK,
    0,
    UBUS_SLV_APB_BASE_BASE_FIELD_WIDTH,
    UBUS_SLV_APB_BASE_BASE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UBUS_SLV_APB_MASK_MASK
 ******************************************************************************/
const ru_field_rec UBUS_SLV_APB_MASK_MASK_FIELD =
{
    "MASK",
#if RU_INCLUDE_DESC
    "Mask",
    "mask",
#endif
    UBUS_SLV_APB_MASK_MASK_FIELD_MASK,
    0,
    UBUS_SLV_APB_MASK_MASK_FIELD_WIDTH,
    UBUS_SLV_APB_MASK_MASK_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UBUS_SLV_DQM_BASE_BASE
 ******************************************************************************/
const ru_field_rec UBUS_SLV_DQM_BASE_BASE_FIELD =
{
    "BASE",
#if RU_INCLUDE_DESC
    "Base",
    "base",
#endif
    UBUS_SLV_DQM_BASE_BASE_FIELD_MASK,
    0,
    UBUS_SLV_DQM_BASE_BASE_FIELD_WIDTH,
    UBUS_SLV_DQM_BASE_BASE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UBUS_SLV_DQM_MASK_MASK
 ******************************************************************************/
const ru_field_rec UBUS_SLV_DQM_MASK_MASK_FIELD =
{
    "MASK",
#if RU_INCLUDE_DESC
    "Mask",
    "mask",
#endif
    UBUS_SLV_DQM_MASK_MASK_FIELD_MASK,
    0,
    UBUS_SLV_DQM_MASK_MASK_FIELD_WIDTH,
    UBUS_SLV_DQM_MASK_MASK_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UBUS_SLV_RNR_INTR_CTRL_ISR_IST
 ******************************************************************************/
const ru_field_rec UBUS_SLV_RNR_INTR_CTRL_ISR_IST_FIELD =
{
    "IST",
#if RU_INCLUDE_DESC
    "ISR",
    "ISR - 32bit RNR INT",
#endif
    UBUS_SLV_RNR_INTR_CTRL_ISR_IST_FIELD_MASK,
    0,
    UBUS_SLV_RNR_INTR_CTRL_ISR_IST_FIELD_WIDTH,
    UBUS_SLV_RNR_INTR_CTRL_ISR_IST_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UBUS_SLV_RNR_INTR_CTRL_ISM_ISM
 ******************************************************************************/
const ru_field_rec UBUS_SLV_RNR_INTR_CTRL_ISM_ISM_FIELD =
{
    "ISM",
#if RU_INCLUDE_DESC
    "Interrupt_status_masked",
    "Status Masked of corresponding interrupt source in the ISR",
#endif
    UBUS_SLV_RNR_INTR_CTRL_ISM_ISM_FIELD_MASK,
    0,
    UBUS_SLV_RNR_INTR_CTRL_ISM_ISM_FIELD_WIDTH,
    UBUS_SLV_RNR_INTR_CTRL_ISM_ISM_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: UBUS_SLV_RNR_INTR_CTRL_IER_IEM
 ******************************************************************************/
const ru_field_rec UBUS_SLV_RNR_INTR_CTRL_IER_IEM_FIELD =
{
    "IEM",
#if RU_INCLUDE_DESC
    "Interrupt_enable_mask",
    "Each bit in the mask controls the corresponding interrupt source in the IER",
#endif
    UBUS_SLV_RNR_INTR_CTRL_IER_IEM_FIELD_MASK,
    0,
    UBUS_SLV_RNR_INTR_CTRL_IER_IEM_FIELD_WIDTH,
    UBUS_SLV_RNR_INTR_CTRL_IER_IEM_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UBUS_SLV_RNR_INTR_CTRL_ITR_IST
 ******************************************************************************/
const ru_field_rec UBUS_SLV_RNR_INTR_CTRL_ITR_IST_FIELD =
{
    "IST",
#if RU_INCLUDE_DESC
    "Interrupt_simulation_test",
    "Each bit in the mask tests the corresponding interrupt source in the ISR",
#endif
    UBUS_SLV_RNR_INTR_CTRL_ITR_IST_FIELD_MASK,
    0,
    UBUS_SLV_RNR_INTR_CTRL_ITR_IST_FIELD_WIDTH,
    UBUS_SLV_RNR_INTR_CTRL_ITR_IST_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UBUS_SLV_PROFILING_CFG_COUNTER_ENABLE
 ******************************************************************************/
const ru_field_rec UBUS_SLV_PROFILING_CFG_COUNTER_ENABLE_FIELD =
{
    "COUNTER_ENABLE",
#if RU_INCLUDE_DESC
    "COUNTER_ENABLE",
    "Enable free-running counter",
#endif
    UBUS_SLV_PROFILING_CFG_COUNTER_ENABLE_FIELD_MASK,
    0,
    UBUS_SLV_PROFILING_CFG_COUNTER_ENABLE_FIELD_WIDTH,
    UBUS_SLV_PROFILING_CFG_COUNTER_ENABLE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UBUS_SLV_PROFILING_CFG_PROFILING_START
 ******************************************************************************/
const ru_field_rec UBUS_SLV_PROFILING_CFG_PROFILING_START_FIELD =
{
    "PROFILING_START",
#if RU_INCLUDE_DESC
    "PROFILING_START",
    "Start profiling window.",
#endif
    UBUS_SLV_PROFILING_CFG_PROFILING_START_FIELD_MASK,
    0,
    UBUS_SLV_PROFILING_CFG_PROFILING_START_FIELD_WIDTH,
    UBUS_SLV_PROFILING_CFG_PROFILING_START_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UBUS_SLV_PROFILING_CFG_MANUAL_STOP_MODE
 ******************************************************************************/
const ru_field_rec UBUS_SLV_PROFILING_CFG_MANUAL_STOP_MODE_FIELD =
{
    "MANUAL_STOP_MODE",
#if RU_INCLUDE_DESC
    "MANUAL_STOP_MODE",
    "Enable manual stop mode",
#endif
    UBUS_SLV_PROFILING_CFG_MANUAL_STOP_MODE_FIELD_MASK,
    0,
    UBUS_SLV_PROFILING_CFG_MANUAL_STOP_MODE_FIELD_WIDTH,
    UBUS_SLV_PROFILING_CFG_MANUAL_STOP_MODE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UBUS_SLV_PROFILING_CFG_DO_MANUAL_STOP
 ******************************************************************************/
const ru_field_rec UBUS_SLV_PROFILING_CFG_DO_MANUAL_STOP_FIELD =
{
    "DO_MANUAL_STOP",
#if RU_INCLUDE_DESC
    "DO_MANUAL_STOP",
    "Stop window now",
#endif
    UBUS_SLV_PROFILING_CFG_DO_MANUAL_STOP_FIELD_MASK,
    0,
    UBUS_SLV_PROFILING_CFG_DO_MANUAL_STOP_FIELD_WIDTH,
    UBUS_SLV_PROFILING_CFG_DO_MANUAL_STOP_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UBUS_SLV_PROFILING_CFG_RESERVED0
 ******************************************************************************/
const ru_field_rec UBUS_SLV_PROFILING_CFG_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    UBUS_SLV_PROFILING_CFG_RESERVED0_FIELD_MASK,
    0,
    UBUS_SLV_PROFILING_CFG_RESERVED0_FIELD_WIDTH,
    UBUS_SLV_PROFILING_CFG_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UBUS_SLV_PROFILING_STATUS_PROFILING_ON
 ******************************************************************************/
const ru_field_rec UBUS_SLV_PROFILING_STATUS_PROFILING_ON_FIELD =
{
    "PROFILING_ON",
#if RU_INCLUDE_DESC
    "PROFILING_ON",
    "Profiling is currently on",
#endif
    UBUS_SLV_PROFILING_STATUS_PROFILING_ON_FIELD_MASK,
    0,
    UBUS_SLV_PROFILING_STATUS_PROFILING_ON_FIELD_WIDTH,
    UBUS_SLV_PROFILING_STATUS_PROFILING_ON_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: UBUS_SLV_PROFILING_STATUS_CYCLES_COUNTER
 ******************************************************************************/
const ru_field_rec UBUS_SLV_PROFILING_STATUS_CYCLES_COUNTER_FIELD =
{
    "CYCLES_COUNTER",
#if RU_INCLUDE_DESC
    "CYCLES_COUNTER",
    "Current value of profiling window cycles counter (bits [30:0]",
#endif
    UBUS_SLV_PROFILING_STATUS_CYCLES_COUNTER_FIELD_MASK,
    0,
    UBUS_SLV_PROFILING_STATUS_CYCLES_COUNTER_FIELD_WIDTH,
    UBUS_SLV_PROFILING_STATUS_CYCLES_COUNTER_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: UBUS_SLV_PROFILING_COUNTER_VAL
 ******************************************************************************/
const ru_field_rec UBUS_SLV_PROFILING_COUNTER_VAL_FIELD =
{
    "VAL",
#if RU_INCLUDE_DESC
    "Val",
    "Value",
#endif
    UBUS_SLV_PROFILING_COUNTER_VAL_FIELD_MASK,
    0,
    UBUS_SLV_PROFILING_COUNTER_VAL_FIELD_WIDTH,
    UBUS_SLV_PROFILING_COUNTER_VAL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: UBUS_SLV_PROFILING_START_VALUE_VAL
 ******************************************************************************/
const ru_field_rec UBUS_SLV_PROFILING_START_VALUE_VAL_FIELD =
{
    "VAL",
#if RU_INCLUDE_DESC
    "Val",
    "Value",
#endif
    UBUS_SLV_PROFILING_START_VALUE_VAL_FIELD_MASK,
    0,
    UBUS_SLV_PROFILING_START_VALUE_VAL_FIELD_WIDTH,
    UBUS_SLV_PROFILING_START_VALUE_VAL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: UBUS_SLV_PROFILING_STOP_VALUE_VAL
 ******************************************************************************/
const ru_field_rec UBUS_SLV_PROFILING_STOP_VALUE_VAL_FIELD =
{
    "VAL",
#if RU_INCLUDE_DESC
    "Val",
    "Value",
#endif
    UBUS_SLV_PROFILING_STOP_VALUE_VAL_FIELD_MASK,
    0,
    UBUS_SLV_PROFILING_STOP_VALUE_VAL_FIELD_WIDTH,
    UBUS_SLV_PROFILING_STOP_VALUE_VAL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: UBUS_SLV_PROFILING_CYCLE_NUM_PROFILING_CYCLES_NUM
 ******************************************************************************/
const ru_field_rec UBUS_SLV_PROFILING_CYCLE_NUM_PROFILING_CYCLES_NUM_FIELD =
{
    "PROFILING_CYCLES_NUM",
#if RU_INCLUDE_DESC
    "PROFILING_CYCLES_NUM",
    "Length of profiling window in 500MHz clock cycles",
#endif
    UBUS_SLV_PROFILING_CYCLE_NUM_PROFILING_CYCLES_NUM_FIELD_MASK,
    0,
    UBUS_SLV_PROFILING_CYCLE_NUM_PROFILING_CYCLES_NUM_FIELD_WIDTH,
    UBUS_SLV_PROFILING_CYCLE_NUM_PROFILING_CYCLES_NUM_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UBUS_SLV__CNTRL_RGMII_MODE_EN
 ******************************************************************************/
const ru_field_rec UBUS_SLV__CNTRL_RGMII_MODE_EN_FIELD =
{
    "RGMII_MODE_EN",
#if RU_INCLUDE_DESC
    "RGMII_MODE_EN",
    "When set this bit enables RGMII interface."
    "This bit acts as a reset for RGMII block abd therefore it can be used to reset RGMII block when needed.",
#endif
    UBUS_SLV__CNTRL_RGMII_MODE_EN_FIELD_MASK,
    0,
    UBUS_SLV__CNTRL_RGMII_MODE_EN_FIELD_WIDTH,
    UBUS_SLV__CNTRL_RGMII_MODE_EN_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UBUS_SLV__CNTRL_ID_MODE_DIS
 ******************************************************************************/
const ru_field_rec UBUS_SLV__CNTRL_ID_MODE_DIS_FIELD =
{
    "ID_MODE_DIS",
#if RU_INCLUDE_DESC
    "ID_MODE_DIS",
    "RGMII Internal Delay (ID) mode disable."
    "When set RGMII transmit clock edges are aligned with the data."
    "When cleared RGMII transmit clock edges are centered in the middle of (transmit) data valid window.",
#endif
    UBUS_SLV__CNTRL_ID_MODE_DIS_FIELD_MASK,
    0,
    UBUS_SLV__CNTRL_ID_MODE_DIS_FIELD_WIDTH,
    UBUS_SLV__CNTRL_ID_MODE_DIS_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UBUS_SLV__CNTRL_PORT_MODE
 ******************************************************************************/
const ru_field_rec UBUS_SLV__CNTRL_PORT_MODE_FIELD =
{
    "PORT_MODE",
#if RU_INCLUDE_DESC
    "PORT_MODE",
    "Port Mode encoded as:"
    "000 : Internal EPHY (MII)."
    "001 : Internal GPHY (GMII/MII)."
    "010 : External EPHY (MII)."
    "011 : External GPHY (RGMII)."
    "100 : External RvMII."
    "Not all combinations are applicable to all chips."
    "",
#endif
    UBUS_SLV__CNTRL_PORT_MODE_FIELD_MASK,
    0,
    UBUS_SLV__CNTRL_PORT_MODE_FIELD_WIDTH,
    UBUS_SLV__CNTRL_PORT_MODE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UBUS_SLV__CNTRL_RVMII_REF_SEL
 ******************************************************************************/
const ru_field_rec UBUS_SLV__CNTRL_RVMII_REF_SEL_FIELD =
{
    "RVMII_REF_SEL",
#if RU_INCLUDE_DESC
    "RVMII_REF_SEL",
    "Selects clock in RvMII mode."
    "0 : RvMII reference clock is 50MHz."
    "1 : RvMII reference clock is 25MHz."
    "",
#endif
    UBUS_SLV__CNTRL_RVMII_REF_SEL_FIELD_MASK,
    0,
    UBUS_SLV__CNTRL_RVMII_REF_SEL_FIELD_WIDTH,
    UBUS_SLV__CNTRL_RVMII_REF_SEL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UBUS_SLV__CNTRL_RX_PAUSE_EN
 ******************************************************************************/
const ru_field_rec UBUS_SLV__CNTRL_RX_PAUSE_EN_FIELD =
{
    "RX_PAUSE_EN",
#if RU_INCLUDE_DESC
    "RX_PAUSE_EN",
    "Rx Pause as negotiated by the attached PHY. Obtained by SW via MDIO.",
#endif
    UBUS_SLV__CNTRL_RX_PAUSE_EN_FIELD_MASK,
    0,
    UBUS_SLV__CNTRL_RX_PAUSE_EN_FIELD_WIDTH,
    UBUS_SLV__CNTRL_RX_PAUSE_EN_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UBUS_SLV__CNTRL_TX_PAUSE_EN
 ******************************************************************************/
const ru_field_rec UBUS_SLV__CNTRL_TX_PAUSE_EN_FIELD =
{
    "TX_PAUSE_EN",
#if RU_INCLUDE_DESC
    "TX_PAUSE_EN",
    "Tx Pause as negotiated by the attached PHY. Obtained by SW via MDIO.",
#endif
    UBUS_SLV__CNTRL_TX_PAUSE_EN_FIELD_MASK,
    0,
    UBUS_SLV__CNTRL_TX_PAUSE_EN_FIELD_WIDTH,
    UBUS_SLV__CNTRL_TX_PAUSE_EN_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UBUS_SLV__CNTRL_TX_CLK_STOP_EN
 ******************************************************************************/
const ru_field_rec UBUS_SLV__CNTRL_TX_CLK_STOP_EN_FIELD =
{
    "TX_CLK_STOP_EN",
#if RU_INCLUDE_DESC
    "TX_CLK_STOP_EN",
    "hen set enables stopping TX_CLK after LPI is asserted. This bit should be set only when the connected EEE PHY supports it.",
#endif
    UBUS_SLV__CNTRL_TX_CLK_STOP_EN_FIELD_MASK,
    0,
    UBUS_SLV__CNTRL_TX_CLK_STOP_EN_FIELD_WIDTH,
    UBUS_SLV__CNTRL_TX_CLK_STOP_EN_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UBUS_SLV__CNTRL_LPI_COUNT
 ******************************************************************************/
const ru_field_rec UBUS_SLV__CNTRL_LPI_COUNT_FIELD =
{
    "LPI_COUNT",
#if RU_INCLUDE_DESC
    "LPI_COUNT",
    "Specifies number of cycles after which TX_CLK will be stopped (after LPI is asserted), if the clock stopping is enabled.",
#endif
    UBUS_SLV__CNTRL_LPI_COUNT_FIELD_MASK,
    0,
    UBUS_SLV__CNTRL_LPI_COUNT_FIELD_WIDTH,
    UBUS_SLV__CNTRL_LPI_COUNT_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UBUS_SLV__CNTRL_RX_ERR_MASK
 ******************************************************************************/
const ru_field_rec UBUS_SLV__CNTRL_RX_ERR_MASK_FIELD =
{
    "RX_ERR_MASK",
#if RU_INCLUDE_DESC
    "RX_ERR_MASK",
    "When this bit is set to 1b1, RX_ERR signal toward the MAC is 1b0 (i.e. no error). Applicable to MII/rvMII interfaces and used in case where link partner does not support RX_ERR.",
#endif
    UBUS_SLV__CNTRL_RX_ERR_MASK_FIELD_MASK,
    0,
    UBUS_SLV__CNTRL_RX_ERR_MASK_FIELD_WIDTH,
    UBUS_SLV__CNTRL_RX_ERR_MASK_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UBUS_SLV__CNTRL_COL_CRS_MASK
 ******************************************************************************/
const ru_field_rec UBUS_SLV__CNTRL_COL_CRS_MASK_FIELD =
{
    "COL_CRS_MASK",
#if RU_INCLUDE_DESC
    "COL_CRS_MASK",
    "When this bit is set to 1b1, COL signal toward the MAC is 1b0 and CRS signal toward the MAC is 1b1. Applicable to MII/rvMII interfaces and used in case where link partner does not support COL/CRS or the link is full-duplex. Note that as per IEEE 802.3 MACs ignore COL/CRS in full-duplex mode and therefore it is not necessary required to set this bit.",
#endif
    UBUS_SLV__CNTRL_COL_CRS_MASK_FIELD_MASK,
    0,
    UBUS_SLV__CNTRL_COL_CRS_MASK_FIELD_WIDTH,
    UBUS_SLV__CNTRL_COL_CRS_MASK_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UBUS_SLV__CNTRL_RESERVED0
 ******************************************************************************/
const ru_field_rec UBUS_SLV__CNTRL_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    UBUS_SLV__CNTRL_RESERVED0_FIELD_MASK,
    0,
    UBUS_SLV__CNTRL_RESERVED0_FIELD_WIDTH,
    UBUS_SLV__CNTRL_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UBUS_SLV__IB_STATUS_SPEED_DECODE
 ******************************************************************************/
const ru_field_rec UBUS_SLV__IB_STATUS_SPEED_DECODE_FIELD =
{
    "SPEED_DECODE",
#if RU_INCLUDE_DESC
    "SPEED_DECODE",
    "RGMII operating speed as extracted from in-band signaling."
    "00 : 10Mbp/s."
    "01 : 100Mbp/s."
    "10 : 1000Mbp/s."
    "11 : reserved.",
#endif
    UBUS_SLV__IB_STATUS_SPEED_DECODE_FIELD_MASK,
    0,
    UBUS_SLV__IB_STATUS_SPEED_DECODE_FIELD_WIDTH,
    UBUS_SLV__IB_STATUS_SPEED_DECODE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UBUS_SLV__IB_STATUS_DUPLEX_DECODE
 ******************************************************************************/
const ru_field_rec UBUS_SLV__IB_STATUS_DUPLEX_DECODE_FIELD =
{
    "DUPLEX_DECODE",
#if RU_INCLUDE_DESC
    "DUPLEX_DECODE",
    "RGMII duplex mode as extracted from in-band signaling."
    "1 : Full Duplex."
    "0 : Half Duplex.",
#endif
    UBUS_SLV__IB_STATUS_DUPLEX_DECODE_FIELD_MASK,
    0,
    UBUS_SLV__IB_STATUS_DUPLEX_DECODE_FIELD_WIDTH,
    UBUS_SLV__IB_STATUS_DUPLEX_DECODE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UBUS_SLV__IB_STATUS_LINK_DECODE
 ******************************************************************************/
const ru_field_rec UBUS_SLV__IB_STATUS_LINK_DECODE_FIELD =
{
    "LINK_DECODE",
#if RU_INCLUDE_DESC
    "LINK_DECODE",
    "RGMII link indication as extracted from in-band signaling."
    "0 : Link Down."
    "1 : Link Up.",
#endif
    UBUS_SLV__IB_STATUS_LINK_DECODE_FIELD_MASK,
    0,
    UBUS_SLV__IB_STATUS_LINK_DECODE_FIELD_WIDTH,
    UBUS_SLV__IB_STATUS_LINK_DECODE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UBUS_SLV__IB_STATUS_IB_STATUS_OVRD
 ******************************************************************************/
const ru_field_rec UBUS_SLV__IB_STATUS_IB_STATUS_OVRD_FIELD =
{
    "IB_STATUS_OVRD",
#if RU_INCLUDE_DESC
    "IB_STATUS_OVRD",
    "When this bit is set, RGMII in-band status can be overridden by bits [3:0] of this register by SW.",
#endif
    UBUS_SLV__IB_STATUS_IB_STATUS_OVRD_FIELD_MASK,
    0,
    UBUS_SLV__IB_STATUS_IB_STATUS_OVRD_FIELD_WIDTH,
    UBUS_SLV__IB_STATUS_IB_STATUS_OVRD_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UBUS_SLV__IB_STATUS_RESERVED0
 ******************************************************************************/
const ru_field_rec UBUS_SLV__IB_STATUS_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    UBUS_SLV__IB_STATUS_RESERVED0_FIELD_MASK,
    0,
    UBUS_SLV__IB_STATUS_RESERVED0_FIELD_WIDTH,
    UBUS_SLV__IB_STATUS_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UBUS_SLV__RX_CLOCK_DELAY_CNTRL_CTRI
 ******************************************************************************/
const ru_field_rec UBUS_SLV__RX_CLOCK_DELAY_CNTRL_CTRI_FIELD =
{
    "CTRI",
#if RU_INCLUDE_DESC
    "CTRI",
    "Charge pump current control. Contact BRCM for more information",
#endif
    UBUS_SLV__RX_CLOCK_DELAY_CNTRL_CTRI_FIELD_MASK,
    0,
    UBUS_SLV__RX_CLOCK_DELAY_CNTRL_CTRI_FIELD_WIDTH,
    UBUS_SLV__RX_CLOCK_DELAY_CNTRL_CTRI_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UBUS_SLV__RX_CLOCK_DELAY_CNTRL_DRNG
 ******************************************************************************/
const ru_field_rec UBUS_SLV__RX_CLOCK_DELAY_CNTRL_DRNG_FIELD =
{
    "DRNG",
#if RU_INCLUDE_DESC
    "DRNG",
    "VCDL control. Contact BRCM for more information",
#endif
    UBUS_SLV__RX_CLOCK_DELAY_CNTRL_DRNG_FIELD_MASK,
    0,
    UBUS_SLV__RX_CLOCK_DELAY_CNTRL_DRNG_FIELD_WIDTH,
    UBUS_SLV__RX_CLOCK_DELAY_CNTRL_DRNG_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UBUS_SLV__RX_CLOCK_DELAY_CNTRL_IDDQ
 ******************************************************************************/
const ru_field_rec UBUS_SLV__RX_CLOCK_DELAY_CNTRL_IDDQ_FIELD =
{
    "IDDQ",
#if RU_INCLUDE_DESC
    "IDDQ",
    "When set puts 2ns delay line in IDDQ mode. Requires HW reset (see bit 8 of this register) to bring 2ns delay line from power down."
    "",
#endif
    UBUS_SLV__RX_CLOCK_DELAY_CNTRL_IDDQ_FIELD_MASK,
    0,
    UBUS_SLV__RX_CLOCK_DELAY_CNTRL_IDDQ_FIELD_WIDTH,
    UBUS_SLV__RX_CLOCK_DELAY_CNTRL_IDDQ_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UBUS_SLV__RX_CLOCK_DELAY_CNTRL_BYPASS
 ******************************************************************************/
const ru_field_rec UBUS_SLV__RX_CLOCK_DELAY_CNTRL_BYPASS_FIELD =
{
    "BYPASS",
#if RU_INCLUDE_DESC
    "BYPASS",
    "When set it puts 2ns delay line in bypass mode (default). This bit should be cleared only in non-ID mode.",
#endif
    UBUS_SLV__RX_CLOCK_DELAY_CNTRL_BYPASS_FIELD_MASK,
    0,
    UBUS_SLV__RX_CLOCK_DELAY_CNTRL_BYPASS_FIELD_WIDTH,
    UBUS_SLV__RX_CLOCK_DELAY_CNTRL_BYPASS_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UBUS_SLV__RX_CLOCK_DELAY_CNTRL_DLY_SEL
 ******************************************************************************/
const ru_field_rec UBUS_SLV__RX_CLOCK_DELAY_CNTRL_DLY_SEL_FIELD =
{
    "DLY_SEL",
#if RU_INCLUDE_DESC
    "DLY_SEL",
    "When set delay line delay is ~2ns and when cleared delay line is > 2.2ns. Valid only when DLY_OVERRIDE bit is set.",
#endif
    UBUS_SLV__RX_CLOCK_DELAY_CNTRL_DLY_SEL_FIELD_MASK,
    0,
    UBUS_SLV__RX_CLOCK_DELAY_CNTRL_DLY_SEL_FIELD_WIDTH,
    UBUS_SLV__RX_CLOCK_DELAY_CNTRL_DLY_SEL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UBUS_SLV__RX_CLOCK_DELAY_CNTRL_DLY_OVERRIDE
 ******************************************************************************/
const ru_field_rec UBUS_SLV__RX_CLOCK_DELAY_CNTRL_DLY_OVERRIDE_FIELD =
{
    "DLY_OVERRIDE",
#if RU_INCLUDE_DESC
    "DLY_OVERRIDE",
    "Overrides HW selected delay.",
#endif
    UBUS_SLV__RX_CLOCK_DELAY_CNTRL_DLY_OVERRIDE_FIELD_MASK,
    0,
    UBUS_SLV__RX_CLOCK_DELAY_CNTRL_DLY_OVERRIDE_FIELD_WIDTH,
    UBUS_SLV__RX_CLOCK_DELAY_CNTRL_DLY_OVERRIDE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UBUS_SLV__RX_CLOCK_DELAY_CNTRL_RESET
 ******************************************************************************/
const ru_field_rec UBUS_SLV__RX_CLOCK_DELAY_CNTRL_RESET_FIELD =
{
    "RESET",
#if RU_INCLUDE_DESC
    "RESET",
    "When set it resets 2ns delay line.",
#endif
    UBUS_SLV__RX_CLOCK_DELAY_CNTRL_RESET_FIELD_MASK,
    0,
    UBUS_SLV__RX_CLOCK_DELAY_CNTRL_RESET_FIELD_WIDTH,
    UBUS_SLV__RX_CLOCK_DELAY_CNTRL_RESET_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UBUS_SLV__RX_CLOCK_DELAY_CNTRL_RESERVED0
 ******************************************************************************/
const ru_field_rec UBUS_SLV__RX_CLOCK_DELAY_CNTRL_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    UBUS_SLV__RX_CLOCK_DELAY_CNTRL_RESERVED0_FIELD_MASK,
    0,
    UBUS_SLV__RX_CLOCK_DELAY_CNTRL_RESERVED0_FIELD_WIDTH,
    UBUS_SLV__RX_CLOCK_DELAY_CNTRL_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UBUS_SLV__ATE_RX_CNTRL_EXP_DATA_EXPECTED_DATA_0
 ******************************************************************************/
const ru_field_rec UBUS_SLV__ATE_RX_CNTRL_EXP_DATA_EXPECTED_DATA_0_FIELD =
{
    "EXPECTED_DATA_0",
#if RU_INCLUDE_DESC
    "EXPECTED_DATA_0",
    "Data expected on the even rising edge of the RXC clock on the RGMII Rx interface. Bits[3:0] of this register are used only in MII modes and they represent RXD[3:0]. Bit 8 corresponds RX_ER."
    "Not used in Packet Generation mode.",
#endif
    UBUS_SLV__ATE_RX_CNTRL_EXP_DATA_EXPECTED_DATA_0_FIELD_MASK,
    0,
    UBUS_SLV__ATE_RX_CNTRL_EXP_DATA_EXPECTED_DATA_0_FIELD_WIDTH,
    UBUS_SLV__ATE_RX_CNTRL_EXP_DATA_EXPECTED_DATA_0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UBUS_SLV__ATE_RX_CNTRL_EXP_DATA_EXPECTED_DATA_1
 ******************************************************************************/
const ru_field_rec UBUS_SLV__ATE_RX_CNTRL_EXP_DATA_EXPECTED_DATA_1_FIELD =
{
    "EXPECTED_DATA_1",
#if RU_INCLUDE_DESC
    "EXPECTED_DATA_1",
    "Data expected on the odd rising edge of the RXC clock on the RGMII Rx interface. Bits[12:9] of this register are used only in MII modes and they represent RXD[3:0]. Bit 17 corresponds RX_ER."
    "Not used in Packet Generation mode.",
#endif
    UBUS_SLV__ATE_RX_CNTRL_EXP_DATA_EXPECTED_DATA_1_FIELD_MASK,
    0,
    UBUS_SLV__ATE_RX_CNTRL_EXP_DATA_EXPECTED_DATA_1_FIELD_WIDTH,
    UBUS_SLV__ATE_RX_CNTRL_EXP_DATA_EXPECTED_DATA_1_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UBUS_SLV__ATE_RX_CNTRL_EXP_DATA_GOOD_COUNT
 ******************************************************************************/
const ru_field_rec UBUS_SLV__ATE_RX_CNTRL_EXP_DATA_GOOD_COUNT_FIELD =
{
    "GOOD_COUNT",
#if RU_INCLUDE_DESC
    "GOOD_COUNT",
    "Count that specifies how many consecutive {EXPECTED_DATA_0, EXPECTED_DATA_1, EXPECTED_DATA_2, EXPECTED_DATA_3 } patterns should be received before RX_OK signal is asserted."
    "In packet generation mode it specifies number of expected packets.",
#endif
    UBUS_SLV__ATE_RX_CNTRL_EXP_DATA_GOOD_COUNT_FIELD_MASK,
    0,
    UBUS_SLV__ATE_RX_CNTRL_EXP_DATA_GOOD_COUNT_FIELD_WIDTH,
    UBUS_SLV__ATE_RX_CNTRL_EXP_DATA_GOOD_COUNT_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UBUS_SLV__ATE_RX_CNTRL_EXP_DATA_PKT_COUNT_RST
 ******************************************************************************/
const ru_field_rec UBUS_SLV__ATE_RX_CNTRL_EXP_DATA_PKT_COUNT_RST_FIELD =
{
    "PKT_COUNT_RST",
#if RU_INCLUDE_DESC
    "PKT_COUNT_RST",
    "When set resets received packets counter. Used only in packet generation mode (PKT_GEN_MODE bit is set).",
#endif
    UBUS_SLV__ATE_RX_CNTRL_EXP_DATA_PKT_COUNT_RST_FIELD_MASK,
    0,
    UBUS_SLV__ATE_RX_CNTRL_EXP_DATA_PKT_COUNT_RST_FIELD_WIDTH,
    UBUS_SLV__ATE_RX_CNTRL_EXP_DATA_PKT_COUNT_RST_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UBUS_SLV__ATE_RX_CNTRL_EXP_DATA_ATE_EN
 ******************************************************************************/
const ru_field_rec UBUS_SLV__ATE_RX_CNTRL_EXP_DATA_ATE_EN_FIELD =
{
    "ATE_EN",
#if RU_INCLUDE_DESC
    "ATE_EN",
    "When set enables ATE testing",
#endif
    UBUS_SLV__ATE_RX_CNTRL_EXP_DATA_ATE_EN_FIELD_MASK,
    0,
    UBUS_SLV__ATE_RX_CNTRL_EXP_DATA_ATE_EN_FIELD_WIDTH,
    UBUS_SLV__ATE_RX_CNTRL_EXP_DATA_ATE_EN_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UBUS_SLV__ATE_RX_CNTRL_EXP_DATA_RESERVED0
 ******************************************************************************/
const ru_field_rec UBUS_SLV__ATE_RX_CNTRL_EXP_DATA_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    UBUS_SLV__ATE_RX_CNTRL_EXP_DATA_RESERVED0_FIELD_MASK,
    0,
    UBUS_SLV__ATE_RX_CNTRL_EXP_DATA_RESERVED0_FIELD_WIDTH,
    UBUS_SLV__ATE_RX_CNTRL_EXP_DATA_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UBUS_SLV__ATE_RX_EXP_DATA_1_EXPECTED_DATA_2
 ******************************************************************************/
const ru_field_rec UBUS_SLV__ATE_RX_EXP_DATA_1_EXPECTED_DATA_2_FIELD =
{
    "EXPECTED_DATA_2",
#if RU_INCLUDE_DESC
    "EXPECTED_DATA_2",
    "Data expected on the even rising edge of the RXC clock on the RGMII Rx interface. Bits[3:0] of this register are used only in MII modes and they represent RXD[3:0]. Bit 8 corresponds RX_ER."
    "Not used in Packet Generation mode.",
#endif
    UBUS_SLV__ATE_RX_EXP_DATA_1_EXPECTED_DATA_2_FIELD_MASK,
    0,
    UBUS_SLV__ATE_RX_EXP_DATA_1_EXPECTED_DATA_2_FIELD_WIDTH,
    UBUS_SLV__ATE_RX_EXP_DATA_1_EXPECTED_DATA_2_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UBUS_SLV__ATE_RX_EXP_DATA_1_EXPECTED_DATA_3
 ******************************************************************************/
const ru_field_rec UBUS_SLV__ATE_RX_EXP_DATA_1_EXPECTED_DATA_3_FIELD =
{
    "EXPECTED_DATA_3",
#if RU_INCLUDE_DESC
    "EXPECTED_DATA_3",
    "Data expected on the odd rising edge of the RXC clock on the RGMII Rx interface. Bits[12:9] of this register are used only in MII modes and they represent RXD[3:0]. Bit 17 corresponds RX_ER."
    "Not used in Packet Generation mode.",
#endif
    UBUS_SLV__ATE_RX_EXP_DATA_1_EXPECTED_DATA_3_FIELD_MASK,
    0,
    UBUS_SLV__ATE_RX_EXP_DATA_1_EXPECTED_DATA_3_FIELD_WIDTH,
    UBUS_SLV__ATE_RX_EXP_DATA_1_EXPECTED_DATA_3_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UBUS_SLV__ATE_RX_EXP_DATA_1_RESERVED0
 ******************************************************************************/
const ru_field_rec UBUS_SLV__ATE_RX_EXP_DATA_1_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    UBUS_SLV__ATE_RX_EXP_DATA_1_RESERVED0_FIELD_MASK,
    0,
    UBUS_SLV__ATE_RX_EXP_DATA_1_RESERVED0_FIELD_WIDTH,
    UBUS_SLV__ATE_RX_EXP_DATA_1_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UBUS_SLV__ATE_RX_STATUS_0_RECEIVED_DATA_0
 ******************************************************************************/
const ru_field_rec UBUS_SLV__ATE_RX_STATUS_0_RECEIVED_DATA_0_FIELD =
{
    "RECEIVED_DATA_0",
#if RU_INCLUDE_DESC
    "RECEIVED_DATA_0",
    "Data received on the even rising edge of the RXC clock on the RGMII Rx interface. In MII modes, only Bits[3:0] of this register are used only for RXD[3:0]. Bit[8]: RX_ER"
    "In Packet Generation mode bits [7:0] are 1st received byte after SOF.",
#endif
    UBUS_SLV__ATE_RX_STATUS_0_RECEIVED_DATA_0_FIELD_MASK,
    0,
    UBUS_SLV__ATE_RX_STATUS_0_RECEIVED_DATA_0_FIELD_WIDTH,
    UBUS_SLV__ATE_RX_STATUS_0_RECEIVED_DATA_0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: UBUS_SLV__ATE_RX_STATUS_0_RECEIVED_DATA_1
 ******************************************************************************/
const ru_field_rec UBUS_SLV__ATE_RX_STATUS_0_RECEIVED_DATA_1_FIELD =
{
    "RECEIVED_DATA_1",
#if RU_INCLUDE_DESC
    "RECEIVED_DATA_1",
    "Data received on the odd rising edge of the RXC clock on the RGMII Rx interface. In MII modes, only Bits[12:9] of this register are used only for RXD[3:0]. Bit[17]: RX_ER"
    "In Packet Generation mode bits [7:0] are 2nd received byte after SOF.",
#endif
    UBUS_SLV__ATE_RX_STATUS_0_RECEIVED_DATA_1_FIELD_MASK,
    0,
    UBUS_SLV__ATE_RX_STATUS_0_RECEIVED_DATA_1_FIELD_WIDTH,
    UBUS_SLV__ATE_RX_STATUS_0_RECEIVED_DATA_1_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: UBUS_SLV__ATE_RX_STATUS_0_RX_OK
 ******************************************************************************/
const ru_field_rec UBUS_SLV__ATE_RX_STATUS_0_RX_OK_FIELD =
{
    "RX_OK",
#if RU_INCLUDE_DESC
    "RX_OK",
    "Test Status. This bit is cleared by HW on the rising edge of RX_CTL and asserted if GOOD_COUNT consective expected patterns are detected."
    "In packet generation mode this bit is cleared when PKT_COUNT_RST bit is set and set when received packet count = GOOD_COUNT.",
#endif
    UBUS_SLV__ATE_RX_STATUS_0_RX_OK_FIELD_MASK,
    0,
    UBUS_SLV__ATE_RX_STATUS_0_RX_OK_FIELD_WIDTH,
    UBUS_SLV__ATE_RX_STATUS_0_RX_OK_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: UBUS_SLV__ATE_RX_STATUS_0_RESERVED0
 ******************************************************************************/
const ru_field_rec UBUS_SLV__ATE_RX_STATUS_0_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    UBUS_SLV__ATE_RX_STATUS_0_RESERVED0_FIELD_MASK,
    0,
    UBUS_SLV__ATE_RX_STATUS_0_RESERVED0_FIELD_WIDTH,
    UBUS_SLV__ATE_RX_STATUS_0_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: UBUS_SLV__ATE_RX_STATUS_1_RECEIVED_DATA_2
 ******************************************************************************/
const ru_field_rec UBUS_SLV__ATE_RX_STATUS_1_RECEIVED_DATA_2_FIELD =
{
    "RECEIVED_DATA_2",
#if RU_INCLUDE_DESC
    "RECEIVED_DATA_2",
    "Data received on the even rising edge of the RXC clock on the RGMII Rx interface. In MII modes, only Bits[3:0] of this register are used only for RXD[3:0]. Bit[8]: RX_ER"
    "In Packet Generation mode bits [7:0] are 3rd received byte after SOF.",
#endif
    UBUS_SLV__ATE_RX_STATUS_1_RECEIVED_DATA_2_FIELD_MASK,
    0,
    UBUS_SLV__ATE_RX_STATUS_1_RECEIVED_DATA_2_FIELD_WIDTH,
    UBUS_SLV__ATE_RX_STATUS_1_RECEIVED_DATA_2_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: UBUS_SLV__ATE_RX_STATUS_1_RECEIVED_DATA_3
 ******************************************************************************/
const ru_field_rec UBUS_SLV__ATE_RX_STATUS_1_RECEIVED_DATA_3_FIELD =
{
    "RECEIVED_DATA_3",
#if RU_INCLUDE_DESC
    "RECEIVED_DATA_3",
    "Data received on the odd rising edge of the RXC clock on the RGMII Rx interface. In MII modes, only Bits[12:9] of this register are used only for RXD[3:0]. Bit[17]: RX_ER"
    "In Packet Generation mode bits [7:0] are 4th received byte after SOF.",
#endif
    UBUS_SLV__ATE_RX_STATUS_1_RECEIVED_DATA_3_FIELD_MASK,
    0,
    UBUS_SLV__ATE_RX_STATUS_1_RECEIVED_DATA_3_FIELD_WIDTH,
    UBUS_SLV__ATE_RX_STATUS_1_RECEIVED_DATA_3_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: UBUS_SLV__ATE_RX_STATUS_1_RESERVED0
 ******************************************************************************/
const ru_field_rec UBUS_SLV__ATE_RX_STATUS_1_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    UBUS_SLV__ATE_RX_STATUS_1_RESERVED0_FIELD_MASK,
    0,
    UBUS_SLV__ATE_RX_STATUS_1_RESERVED0_FIELD_WIDTH,
    UBUS_SLV__ATE_RX_STATUS_1_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: UBUS_SLV__ATE_TX_CNTRL_START_STOP_OVRD
 ******************************************************************************/
const ru_field_rec UBUS_SLV__ATE_TX_CNTRL_START_STOP_OVRD_FIELD =
{
    "START_STOP_OVRD",
#if RU_INCLUDE_DESC
    "START_STOP_OVRD",
    "START_STOP override. When this bit is set, transmit state machine will be controlled by START_STOP bit of this register instead of the chip pin.",
#endif
    UBUS_SLV__ATE_TX_CNTRL_START_STOP_OVRD_FIELD_MASK,
    0,
    UBUS_SLV__ATE_TX_CNTRL_START_STOP_OVRD_FIELD_WIDTH,
    UBUS_SLV__ATE_TX_CNTRL_START_STOP_OVRD_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UBUS_SLV__ATE_TX_CNTRL_START_STOP
 ******************************************************************************/
const ru_field_rec UBUS_SLV__ATE_TX_CNTRL_START_STOP_FIELD =
{
    "START_STOP",
#if RU_INCLUDE_DESC
    "START_STOP",
    "start_stop. When set transmit state matchin starts outputing programmed pattern over RGMII TX interface. When cleared transmit state machine stops outputting data.",
#endif
    UBUS_SLV__ATE_TX_CNTRL_START_STOP_FIELD_MASK,
    0,
    UBUS_SLV__ATE_TX_CNTRL_START_STOP_FIELD_WIDTH,
    UBUS_SLV__ATE_TX_CNTRL_START_STOP_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UBUS_SLV__ATE_TX_CNTRL_PKT_GEN_EN
 ******************************************************************************/
const ru_field_rec UBUS_SLV__ATE_TX_CNTRL_PKT_GEN_EN_FIELD =
{
    "PKT_GEN_EN",
#if RU_INCLUDE_DESC
    "PKT_GEN_EN",
    "When this bit is set ATE test logic operates in the packet generation mode.",
#endif
    UBUS_SLV__ATE_TX_CNTRL_PKT_GEN_EN_FIELD_MASK,
    0,
    UBUS_SLV__ATE_TX_CNTRL_PKT_GEN_EN_FIELD_WIDTH,
    UBUS_SLV__ATE_TX_CNTRL_PKT_GEN_EN_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UBUS_SLV__ATE_TX_CNTRL_PKT_CNT
 ******************************************************************************/
const ru_field_rec UBUS_SLV__ATE_TX_CNTRL_PKT_CNT_FIELD =
{
    "PKT_CNT",
#if RU_INCLUDE_DESC
    "PKT_CNT",
    "Number of packets generated when START_STOP bit is set.  When program to 0 it means infinite number of packets will be transmit (i.e. until START_STOP is cleared).",
#endif
    UBUS_SLV__ATE_TX_CNTRL_PKT_CNT_FIELD_MASK,
    0,
    UBUS_SLV__ATE_TX_CNTRL_PKT_CNT_FIELD_WIDTH,
    UBUS_SLV__ATE_TX_CNTRL_PKT_CNT_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UBUS_SLV__ATE_TX_CNTRL_PAYLOAD_LENGTH
 ******************************************************************************/
const ru_field_rec UBUS_SLV__ATE_TX_CNTRL_PAYLOAD_LENGTH_FIELD =
{
    "PAYLOAD_LENGTH",
#if RU_INCLUDE_DESC
    "PAYLOAD_LENGTH",
    "Generated packet payload in bytes. Must be between 46B and 1500B.",
#endif
    UBUS_SLV__ATE_TX_CNTRL_PAYLOAD_LENGTH_FIELD_MASK,
    0,
    UBUS_SLV__ATE_TX_CNTRL_PAYLOAD_LENGTH_FIELD_WIDTH,
    UBUS_SLV__ATE_TX_CNTRL_PAYLOAD_LENGTH_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UBUS_SLV__ATE_TX_CNTRL_PKT_IPG
 ******************************************************************************/
const ru_field_rec UBUS_SLV__ATE_TX_CNTRL_PKT_IPG_FIELD =
{
    "PKT_IPG",
#if RU_INCLUDE_DESC
    "PKT_IPG",
    "Inter-packet gap in packet generation mode.",
#endif
    UBUS_SLV__ATE_TX_CNTRL_PKT_IPG_FIELD_MASK,
    0,
    UBUS_SLV__ATE_TX_CNTRL_PKT_IPG_FIELD_WIDTH,
    UBUS_SLV__ATE_TX_CNTRL_PKT_IPG_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UBUS_SLV__ATE_TX_CNTRL_RESERVED0
 ******************************************************************************/
const ru_field_rec UBUS_SLV__ATE_TX_CNTRL_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    UBUS_SLV__ATE_TX_CNTRL_RESERVED0_FIELD_MASK,
    0,
    UBUS_SLV__ATE_TX_CNTRL_RESERVED0_FIELD_WIDTH,
    UBUS_SLV__ATE_TX_CNTRL_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UBUS_SLV__ATE_TX_DATA_0_TX_DATA_0
 ******************************************************************************/
const ru_field_rec UBUS_SLV__ATE_TX_DATA_0_TX_DATA_0_FIELD =
{
    "TX_DATA_0",
#if RU_INCLUDE_DESC
    "TX_DATA_0",
    "Data transmitted on the even rising edge of the TXC clock on the RGMII Tx interface. In case of MII, only bit[3:0] are used to transmit TXD[3:0]. Bit 8: TX_ER"
    "In Packet Generation mode bits [7:0] are 1st byte of MAC DA.",
#endif
    UBUS_SLV__ATE_TX_DATA_0_TX_DATA_0_FIELD_MASK,
    0,
    UBUS_SLV__ATE_TX_DATA_0_TX_DATA_0_FIELD_WIDTH,
    UBUS_SLV__ATE_TX_DATA_0_TX_DATA_0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UBUS_SLV__ATE_TX_DATA_0_TX_DATA_1
 ******************************************************************************/
const ru_field_rec UBUS_SLV__ATE_TX_DATA_0_TX_DATA_1_FIELD =
{
    "TX_DATA_1",
#if RU_INCLUDE_DESC
    "TX_DATA_1",
    "Data transmitted on the odd rising edge of the TXC clock on the RGMII Tx interface. In case of MII, only bit[12:9] are used to transmit TXD[3:0]. Bit 17: TX_ER"
    "In Packet Generation mode bits [7:0] are 2nd byte of MAC DA.",
#endif
    UBUS_SLV__ATE_TX_DATA_0_TX_DATA_1_FIELD_MASK,
    0,
    UBUS_SLV__ATE_TX_DATA_0_TX_DATA_1_FIELD_WIDTH,
    UBUS_SLV__ATE_TX_DATA_0_TX_DATA_1_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UBUS_SLV__ATE_TX_DATA_0_RESERVED0
 ******************************************************************************/
const ru_field_rec UBUS_SLV__ATE_TX_DATA_0_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    UBUS_SLV__ATE_TX_DATA_0_RESERVED0_FIELD_MASK,
    0,
    UBUS_SLV__ATE_TX_DATA_0_RESERVED0_FIELD_WIDTH,
    UBUS_SLV__ATE_TX_DATA_0_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UBUS_SLV__ATE_TX_DATA_1_TX_DATA_2
 ******************************************************************************/
const ru_field_rec UBUS_SLV__ATE_TX_DATA_1_TX_DATA_2_FIELD =
{
    "TX_DATA_2",
#if RU_INCLUDE_DESC
    "TX_DATA_2",
    "Data transmitted on the even rising edge of the TXC clock on the RGMII Tx interface. In case of MII, only bit[3:0] are used to transmit TXD[3:0]. Bit 8: TX_ER"
    "In Packet Generation mode bits [7:0] are 3rd byte of MAC DA.",
#endif
    UBUS_SLV__ATE_TX_DATA_1_TX_DATA_2_FIELD_MASK,
    0,
    UBUS_SLV__ATE_TX_DATA_1_TX_DATA_2_FIELD_WIDTH,
    UBUS_SLV__ATE_TX_DATA_1_TX_DATA_2_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UBUS_SLV__ATE_TX_DATA_1_TX_DATA_3
 ******************************************************************************/
const ru_field_rec UBUS_SLV__ATE_TX_DATA_1_TX_DATA_3_FIELD =
{
    "TX_DATA_3",
#if RU_INCLUDE_DESC
    "TX_DATA_3",
    "Data transmitted on the odd rising edge of the TXC clock on the RGMII Tx interface. In case of MII, only bit[12:9] are used to transmit TXD[3:0]. Bit 17: TX_ER"
    "In Packet Generation mode bits [7:0] are 4th byte of MAC DA.",
#endif
    UBUS_SLV__ATE_TX_DATA_1_TX_DATA_3_FIELD_MASK,
    0,
    UBUS_SLV__ATE_TX_DATA_1_TX_DATA_3_FIELD_WIDTH,
    UBUS_SLV__ATE_TX_DATA_1_TX_DATA_3_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UBUS_SLV__ATE_TX_DATA_1_RESERVED0
 ******************************************************************************/
const ru_field_rec UBUS_SLV__ATE_TX_DATA_1_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    UBUS_SLV__ATE_TX_DATA_1_RESERVED0_FIELD_MASK,
    0,
    UBUS_SLV__ATE_TX_DATA_1_RESERVED0_FIELD_WIDTH,
    UBUS_SLV__ATE_TX_DATA_1_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UBUS_SLV__ATE_TX_DATA_2_TX_DATA_4
 ******************************************************************************/
const ru_field_rec UBUS_SLV__ATE_TX_DATA_2_TX_DATA_4_FIELD =
{
    "TX_DATA_4",
#if RU_INCLUDE_DESC
    "TX_DATA_4",
    "In Packet Generation mode bits [7:0] are 5th byte of MAC DA",
#endif
    UBUS_SLV__ATE_TX_DATA_2_TX_DATA_4_FIELD_MASK,
    0,
    UBUS_SLV__ATE_TX_DATA_2_TX_DATA_4_FIELD_WIDTH,
    UBUS_SLV__ATE_TX_DATA_2_TX_DATA_4_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UBUS_SLV__ATE_TX_DATA_2_TX_DATA_5
 ******************************************************************************/
const ru_field_rec UBUS_SLV__ATE_TX_DATA_2_TX_DATA_5_FIELD =
{
    "TX_DATA_5",
#if RU_INCLUDE_DESC
    "TX_DATA_5",
    "In Packet Generation mode bits [7:0] are 6th byte of MAC DA",
#endif
    UBUS_SLV__ATE_TX_DATA_2_TX_DATA_5_FIELD_MASK,
    0,
    UBUS_SLV__ATE_TX_DATA_2_TX_DATA_5_FIELD_WIDTH,
    UBUS_SLV__ATE_TX_DATA_2_TX_DATA_5_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UBUS_SLV__ATE_TX_DATA_2_ETHER_TYPE
 ******************************************************************************/
const ru_field_rec UBUS_SLV__ATE_TX_DATA_2_ETHER_TYPE_FIELD =
{
    "ETHER_TYPE",
#if RU_INCLUDE_DESC
    "ETHER_TYPE",
    "Generated packet Ethertype",
#endif
    UBUS_SLV__ATE_TX_DATA_2_ETHER_TYPE_FIELD_MASK,
    0,
    UBUS_SLV__ATE_TX_DATA_2_ETHER_TYPE_FIELD_WIDTH,
    UBUS_SLV__ATE_TX_DATA_2_ETHER_TYPE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UBUS_SLV__TX_DELAY_CNTRL_0_TXD0_DEL_SEL
 ******************************************************************************/
const ru_field_rec UBUS_SLV__TX_DELAY_CNTRL_0_TXD0_DEL_SEL_FIELD =
{
    "TXD0_DEL_SEL",
#if RU_INCLUDE_DESC
    "TXD0_DEL_SEL",
    "txd0 CKTAP delay control. Refer to the CKTAP datasheet for programming",
#endif
    UBUS_SLV__TX_DELAY_CNTRL_0_TXD0_DEL_SEL_FIELD_MASK,
    0,
    UBUS_SLV__TX_DELAY_CNTRL_0_TXD0_DEL_SEL_FIELD_WIDTH,
    UBUS_SLV__TX_DELAY_CNTRL_0_TXD0_DEL_SEL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UBUS_SLV__TX_DELAY_CNTRL_0_TXD0_DEL_OVRD_EN
 ******************************************************************************/
const ru_field_rec UBUS_SLV__TX_DELAY_CNTRL_0_TXD0_DEL_OVRD_EN_FIELD =
{
    "TXD0_DEL_OVRD_EN",
#if RU_INCLUDE_DESC
    "TXD0_DEL_OVRD_EN",
    "txd0 CKTAP delay override enable. When set enables CKTAP delay to be controlled from this register.",
#endif
    UBUS_SLV__TX_DELAY_CNTRL_0_TXD0_DEL_OVRD_EN_FIELD_MASK,
    0,
    UBUS_SLV__TX_DELAY_CNTRL_0_TXD0_DEL_OVRD_EN_FIELD_WIDTH,
    UBUS_SLV__TX_DELAY_CNTRL_0_TXD0_DEL_OVRD_EN_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UBUS_SLV__TX_DELAY_CNTRL_0_TXD1_DEL_SEL
 ******************************************************************************/
const ru_field_rec UBUS_SLV__TX_DELAY_CNTRL_0_TXD1_DEL_SEL_FIELD =
{
    "TXD1_DEL_SEL",
#if RU_INCLUDE_DESC
    "TXD1_DEL_SEL",
    "txd1 CKTAP delay control. Refer to the CKTAP datasheet for programming",
#endif
    UBUS_SLV__TX_DELAY_CNTRL_0_TXD1_DEL_SEL_FIELD_MASK,
    0,
    UBUS_SLV__TX_DELAY_CNTRL_0_TXD1_DEL_SEL_FIELD_WIDTH,
    UBUS_SLV__TX_DELAY_CNTRL_0_TXD1_DEL_SEL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UBUS_SLV__TX_DELAY_CNTRL_0_TXD1_DEL_OVRD_EN
 ******************************************************************************/
const ru_field_rec UBUS_SLV__TX_DELAY_CNTRL_0_TXD1_DEL_OVRD_EN_FIELD =
{
    "TXD1_DEL_OVRD_EN",
#if RU_INCLUDE_DESC
    "TXD1_DEL_OVRD_EN",
    "txd1 CKTAP delay override enable. When set enables CKTAP delay to be controlled from this register.",
#endif
    UBUS_SLV__TX_DELAY_CNTRL_0_TXD1_DEL_OVRD_EN_FIELD_MASK,
    0,
    UBUS_SLV__TX_DELAY_CNTRL_0_TXD1_DEL_OVRD_EN_FIELD_WIDTH,
    UBUS_SLV__TX_DELAY_CNTRL_0_TXD1_DEL_OVRD_EN_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UBUS_SLV__TX_DELAY_CNTRL_0_TXD2_DEL_SEL
 ******************************************************************************/
const ru_field_rec UBUS_SLV__TX_DELAY_CNTRL_0_TXD2_DEL_SEL_FIELD =
{
    "TXD2_DEL_SEL",
#if RU_INCLUDE_DESC
    "TXD2_DEL_SEL",
    "txd2 CKTAP delay control. Refer to the CKTAP datasheet for programming",
#endif
    UBUS_SLV__TX_DELAY_CNTRL_0_TXD2_DEL_SEL_FIELD_MASK,
    0,
    UBUS_SLV__TX_DELAY_CNTRL_0_TXD2_DEL_SEL_FIELD_WIDTH,
    UBUS_SLV__TX_DELAY_CNTRL_0_TXD2_DEL_SEL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UBUS_SLV__TX_DELAY_CNTRL_0_TXD2_DEL_OVRD_EN
 ******************************************************************************/
const ru_field_rec UBUS_SLV__TX_DELAY_CNTRL_0_TXD2_DEL_OVRD_EN_FIELD =
{
    "TXD2_DEL_OVRD_EN",
#if RU_INCLUDE_DESC
    "TXD2_DEL_OVRD_EN",
    "txd2 CKTAP delay override enable. When set enables CKTAP delay to be controlled from this register.",
#endif
    UBUS_SLV__TX_DELAY_CNTRL_0_TXD2_DEL_OVRD_EN_FIELD_MASK,
    0,
    UBUS_SLV__TX_DELAY_CNTRL_0_TXD2_DEL_OVRD_EN_FIELD_WIDTH,
    UBUS_SLV__TX_DELAY_CNTRL_0_TXD2_DEL_OVRD_EN_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UBUS_SLV__TX_DELAY_CNTRL_0_TXD3_DEL_SEL
 ******************************************************************************/
const ru_field_rec UBUS_SLV__TX_DELAY_CNTRL_0_TXD3_DEL_SEL_FIELD =
{
    "TXD3_DEL_SEL",
#if RU_INCLUDE_DESC
    "TXD3_DEL_SEL",
    "txd3 CKTAP delay control. Refer to the CKTAP datasheet for programming",
#endif
    UBUS_SLV__TX_DELAY_CNTRL_0_TXD3_DEL_SEL_FIELD_MASK,
    0,
    UBUS_SLV__TX_DELAY_CNTRL_0_TXD3_DEL_SEL_FIELD_WIDTH,
    UBUS_SLV__TX_DELAY_CNTRL_0_TXD3_DEL_SEL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UBUS_SLV__TX_DELAY_CNTRL_0_TXD3_DEL_OVRD_EN
 ******************************************************************************/
const ru_field_rec UBUS_SLV__TX_DELAY_CNTRL_0_TXD3_DEL_OVRD_EN_FIELD =
{
    "TXD3_DEL_OVRD_EN",
#if RU_INCLUDE_DESC
    "TXD3_DEL_OVRD_EN",
    "txd3 CKTAP delay override enable. When set enables CKTAP delay to be controlled from this register.",
#endif
    UBUS_SLV__TX_DELAY_CNTRL_0_TXD3_DEL_OVRD_EN_FIELD_MASK,
    0,
    UBUS_SLV__TX_DELAY_CNTRL_0_TXD3_DEL_OVRD_EN_FIELD_WIDTH,
    UBUS_SLV__TX_DELAY_CNTRL_0_TXD3_DEL_OVRD_EN_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UBUS_SLV__TX_DELAY_CNTRL_0_RESERVED0
 ******************************************************************************/
const ru_field_rec UBUS_SLV__TX_DELAY_CNTRL_0_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    UBUS_SLV__TX_DELAY_CNTRL_0_RESERVED0_FIELD_MASK,
    0,
    UBUS_SLV__TX_DELAY_CNTRL_0_RESERVED0_FIELD_WIDTH,
    UBUS_SLV__TX_DELAY_CNTRL_0_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UBUS_SLV__TX_DELAY_CNTRL_1_TXCTL_DEL_SEL
 ******************************************************************************/
const ru_field_rec UBUS_SLV__TX_DELAY_CNTRL_1_TXCTL_DEL_SEL_FIELD =
{
    "TXCTL_DEL_SEL",
#if RU_INCLUDE_DESC
    "TXCTL_DEL_SEL",
    "txctl CKTAP delay control. Refer to the CKTAP datasheet for programming.",
#endif
    UBUS_SLV__TX_DELAY_CNTRL_1_TXCTL_DEL_SEL_FIELD_MASK,
    0,
    UBUS_SLV__TX_DELAY_CNTRL_1_TXCTL_DEL_SEL_FIELD_WIDTH,
    UBUS_SLV__TX_DELAY_CNTRL_1_TXCTL_DEL_SEL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UBUS_SLV__TX_DELAY_CNTRL_1_TXCTL_DEL_OVRD_EN
 ******************************************************************************/
const ru_field_rec UBUS_SLV__TX_DELAY_CNTRL_1_TXCTL_DEL_OVRD_EN_FIELD =
{
    "TXCTL_DEL_OVRD_EN",
#if RU_INCLUDE_DESC
    "TXCTL_DEL_OVRD_EN",
    "txctl CKTAP delay override enable. When set enables CKTAP delay to be controlled from this register.",
#endif
    UBUS_SLV__TX_DELAY_CNTRL_1_TXCTL_DEL_OVRD_EN_FIELD_MASK,
    0,
    UBUS_SLV__TX_DELAY_CNTRL_1_TXCTL_DEL_OVRD_EN_FIELD_WIDTH,
    UBUS_SLV__TX_DELAY_CNTRL_1_TXCTL_DEL_OVRD_EN_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UBUS_SLV__TX_DELAY_CNTRL_1_TXCLK_DEL_SEL
 ******************************************************************************/
const ru_field_rec UBUS_SLV__TX_DELAY_CNTRL_1_TXCLK_DEL_SEL_FIELD =
{
    "TXCLK_DEL_SEL",
#if RU_INCLUDE_DESC
    "TXCLK_DEL_SEL",
    "txclk NON-ID mode CKTAP delay control. Refer to the CKTAP datasheet for programming.",
#endif
    UBUS_SLV__TX_DELAY_CNTRL_1_TXCLK_DEL_SEL_FIELD_MASK,
    0,
    UBUS_SLV__TX_DELAY_CNTRL_1_TXCLK_DEL_SEL_FIELD_WIDTH,
    UBUS_SLV__TX_DELAY_CNTRL_1_TXCLK_DEL_SEL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UBUS_SLV__TX_DELAY_CNTRL_1_TXCLK_DEL_OVRD_EN
 ******************************************************************************/
const ru_field_rec UBUS_SLV__TX_DELAY_CNTRL_1_TXCLK_DEL_OVRD_EN_FIELD =
{
    "TXCLK_DEL_OVRD_EN",
#if RU_INCLUDE_DESC
    "TXCLK_DEL_OVRD_EN",
    "txclk NON_ID mode CKTAP delay override enable. When set enables CKTAP delay to be controlled from this register.",
#endif
    UBUS_SLV__TX_DELAY_CNTRL_1_TXCLK_DEL_OVRD_EN_FIELD_MASK,
    0,
    UBUS_SLV__TX_DELAY_CNTRL_1_TXCLK_DEL_OVRD_EN_FIELD_WIDTH,
    UBUS_SLV__TX_DELAY_CNTRL_1_TXCLK_DEL_OVRD_EN_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UBUS_SLV__TX_DELAY_CNTRL_1_TXCLK_ID_DEL_SEL
 ******************************************************************************/
const ru_field_rec UBUS_SLV__TX_DELAY_CNTRL_1_TXCLK_ID_DEL_SEL_FIELD =
{
    "TXCLK_ID_DEL_SEL",
#if RU_INCLUDE_DESC
    "TXCLK_ID_DEL_SEL",
    "txclk ID mode CKTAP delay control. Refer to the CKTAP datasheet for programming.",
#endif
    UBUS_SLV__TX_DELAY_CNTRL_1_TXCLK_ID_DEL_SEL_FIELD_MASK,
    0,
    UBUS_SLV__TX_DELAY_CNTRL_1_TXCLK_ID_DEL_SEL_FIELD_WIDTH,
    UBUS_SLV__TX_DELAY_CNTRL_1_TXCLK_ID_DEL_SEL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UBUS_SLV__TX_DELAY_CNTRL_1_TXCLK_ID_DEL_OVRD_EN
 ******************************************************************************/
const ru_field_rec UBUS_SLV__TX_DELAY_CNTRL_1_TXCLK_ID_DEL_OVRD_EN_FIELD =
{
    "TXCLK_ID_DEL_OVRD_EN",
#if RU_INCLUDE_DESC
    "TXCLK_ID_DEL_OVRD_EN",
    "txclk ID mode CKTAP delay override enable. When set enables CKTAP delay to be controlled from this register.",
#endif
    UBUS_SLV__TX_DELAY_CNTRL_1_TXCLK_ID_DEL_OVRD_EN_FIELD_MASK,
    0,
    UBUS_SLV__TX_DELAY_CNTRL_1_TXCLK_ID_DEL_OVRD_EN_FIELD_WIDTH,
    UBUS_SLV__TX_DELAY_CNTRL_1_TXCLK_ID_DEL_OVRD_EN_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UBUS_SLV__TX_DELAY_CNTRL_1_RESERVED0
 ******************************************************************************/
const ru_field_rec UBUS_SLV__TX_DELAY_CNTRL_1_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    UBUS_SLV__TX_DELAY_CNTRL_1_RESERVED0_FIELD_MASK,
    0,
    UBUS_SLV__TX_DELAY_CNTRL_1_RESERVED0_FIELD_WIDTH,
    UBUS_SLV__TX_DELAY_CNTRL_1_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UBUS_SLV__RX_DELAY_CNTRL_0_RXD0_DEL_SEL
 ******************************************************************************/
const ru_field_rec UBUS_SLV__RX_DELAY_CNTRL_0_RXD0_DEL_SEL_FIELD =
{
    "RXD0_DEL_SEL",
#if RU_INCLUDE_DESC
    "RXD0_DEL_SEL",
    "rxd0 CKTAP delay control. Refer to the CKTAP datasheet for programming",
#endif
    UBUS_SLV__RX_DELAY_CNTRL_0_RXD0_DEL_SEL_FIELD_MASK,
    0,
    UBUS_SLV__RX_DELAY_CNTRL_0_RXD0_DEL_SEL_FIELD_WIDTH,
    UBUS_SLV__RX_DELAY_CNTRL_0_RXD0_DEL_SEL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UBUS_SLV__RX_DELAY_CNTRL_0_RXD0_DEL_OVRD_EN
 ******************************************************************************/
const ru_field_rec UBUS_SLV__RX_DELAY_CNTRL_0_RXD0_DEL_OVRD_EN_FIELD =
{
    "RXD0_DEL_OVRD_EN",
#if RU_INCLUDE_DESC
    "RXD0_DEL_OVRD_EN",
    "rxd0 CKTAP delay override enable. When set enables CKTAP delay to be controlled from this register.",
#endif
    UBUS_SLV__RX_DELAY_CNTRL_0_RXD0_DEL_OVRD_EN_FIELD_MASK,
    0,
    UBUS_SLV__RX_DELAY_CNTRL_0_RXD0_DEL_OVRD_EN_FIELD_WIDTH,
    UBUS_SLV__RX_DELAY_CNTRL_0_RXD0_DEL_OVRD_EN_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UBUS_SLV__RX_DELAY_CNTRL_0_RXD1_DEL_SEL
 ******************************************************************************/
const ru_field_rec UBUS_SLV__RX_DELAY_CNTRL_0_RXD1_DEL_SEL_FIELD =
{
    "RXD1_DEL_SEL",
#if RU_INCLUDE_DESC
    "RXD1_DEL_SEL",
    "rxd1 CKTAP delay control. Refer to the CKTAP datasheet for programming",
#endif
    UBUS_SLV__RX_DELAY_CNTRL_0_RXD1_DEL_SEL_FIELD_MASK,
    0,
    UBUS_SLV__RX_DELAY_CNTRL_0_RXD1_DEL_SEL_FIELD_WIDTH,
    UBUS_SLV__RX_DELAY_CNTRL_0_RXD1_DEL_SEL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UBUS_SLV__RX_DELAY_CNTRL_0_RXD1_DEL_OVRD_EN
 ******************************************************************************/
const ru_field_rec UBUS_SLV__RX_DELAY_CNTRL_0_RXD1_DEL_OVRD_EN_FIELD =
{
    "RXD1_DEL_OVRD_EN",
#if RU_INCLUDE_DESC
    "RXD1_DEL_OVRD_EN",
    "rxd1 CKTAP delay override enable. When set enables CKTAP delay to be controlled from this register.",
#endif
    UBUS_SLV__RX_DELAY_CNTRL_0_RXD1_DEL_OVRD_EN_FIELD_MASK,
    0,
    UBUS_SLV__RX_DELAY_CNTRL_0_RXD1_DEL_OVRD_EN_FIELD_WIDTH,
    UBUS_SLV__RX_DELAY_CNTRL_0_RXD1_DEL_OVRD_EN_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UBUS_SLV__RX_DELAY_CNTRL_0_RXD2_DEL_SEL
 ******************************************************************************/
const ru_field_rec UBUS_SLV__RX_DELAY_CNTRL_0_RXD2_DEL_SEL_FIELD =
{
    "RXD2_DEL_SEL",
#if RU_INCLUDE_DESC
    "RXD2_DEL_SEL",
    "rxd2 CKTAP delay control. Refer to the CKTAP datasheet for programming",
#endif
    UBUS_SLV__RX_DELAY_CNTRL_0_RXD2_DEL_SEL_FIELD_MASK,
    0,
    UBUS_SLV__RX_DELAY_CNTRL_0_RXD2_DEL_SEL_FIELD_WIDTH,
    UBUS_SLV__RX_DELAY_CNTRL_0_RXD2_DEL_SEL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UBUS_SLV__RX_DELAY_CNTRL_0_RXD2_DEL_OVRD_EN
 ******************************************************************************/
const ru_field_rec UBUS_SLV__RX_DELAY_CNTRL_0_RXD2_DEL_OVRD_EN_FIELD =
{
    "RXD2_DEL_OVRD_EN",
#if RU_INCLUDE_DESC
    "RXD2_DEL_OVRD_EN",
    "rxd2 CKTAP delay override enable. When set enables CKTAP delay to be controlled from this register.",
#endif
    UBUS_SLV__RX_DELAY_CNTRL_0_RXD2_DEL_OVRD_EN_FIELD_MASK,
    0,
    UBUS_SLV__RX_DELAY_CNTRL_0_RXD2_DEL_OVRD_EN_FIELD_WIDTH,
    UBUS_SLV__RX_DELAY_CNTRL_0_RXD2_DEL_OVRD_EN_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UBUS_SLV__RX_DELAY_CNTRL_0_RXD3_DEL_SEL
 ******************************************************************************/
const ru_field_rec UBUS_SLV__RX_DELAY_CNTRL_0_RXD3_DEL_SEL_FIELD =
{
    "RXD3_DEL_SEL",
#if RU_INCLUDE_DESC
    "RXD3_DEL_SEL",
    "rxd3 CKTAP delay control. Refer to the CKTAP datasheet for programming",
#endif
    UBUS_SLV__RX_DELAY_CNTRL_0_RXD3_DEL_SEL_FIELD_MASK,
    0,
    UBUS_SLV__RX_DELAY_CNTRL_0_RXD3_DEL_SEL_FIELD_WIDTH,
    UBUS_SLV__RX_DELAY_CNTRL_0_RXD3_DEL_SEL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UBUS_SLV__RX_DELAY_CNTRL_0_RXD3_DEL_OVRD_EN
 ******************************************************************************/
const ru_field_rec UBUS_SLV__RX_DELAY_CNTRL_0_RXD3_DEL_OVRD_EN_FIELD =
{
    "RXD3_DEL_OVRD_EN",
#if RU_INCLUDE_DESC
    "RXD3_DEL_OVRD_EN",
    "rxd3 CKTAP delay override enable. When set enables CKTAP delay to be controlled from this register.",
#endif
    UBUS_SLV__RX_DELAY_CNTRL_0_RXD3_DEL_OVRD_EN_FIELD_MASK,
    0,
    UBUS_SLV__RX_DELAY_CNTRL_0_RXD3_DEL_OVRD_EN_FIELD_WIDTH,
    UBUS_SLV__RX_DELAY_CNTRL_0_RXD3_DEL_OVRD_EN_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UBUS_SLV__RX_DELAY_CNTRL_0_RESERVED0
 ******************************************************************************/
const ru_field_rec UBUS_SLV__RX_DELAY_CNTRL_0_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    UBUS_SLV__RX_DELAY_CNTRL_0_RESERVED0_FIELD_MASK,
    0,
    UBUS_SLV__RX_DELAY_CNTRL_0_RESERVED0_FIELD_WIDTH,
    UBUS_SLV__RX_DELAY_CNTRL_0_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UBUS_SLV__RX_DELAY_CNTRL_1_RXD4_DEL_SEL
 ******************************************************************************/
const ru_field_rec UBUS_SLV__RX_DELAY_CNTRL_1_RXD4_DEL_SEL_FIELD =
{
    "RXD4_DEL_SEL",
#if RU_INCLUDE_DESC
    "RXD4_DEL_SEL",
    "rxd4 CKTAP delay control. Refer to the CKTAP datasheet for programming",
#endif
    UBUS_SLV__RX_DELAY_CNTRL_1_RXD4_DEL_SEL_FIELD_MASK,
    0,
    UBUS_SLV__RX_DELAY_CNTRL_1_RXD4_DEL_SEL_FIELD_WIDTH,
    UBUS_SLV__RX_DELAY_CNTRL_1_RXD4_DEL_SEL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UBUS_SLV__RX_DELAY_CNTRL_1_RXD4_DEL_OVRD_EN
 ******************************************************************************/
const ru_field_rec UBUS_SLV__RX_DELAY_CNTRL_1_RXD4_DEL_OVRD_EN_FIELD =
{
    "RXD4_DEL_OVRD_EN",
#if RU_INCLUDE_DESC
    "RXD4_DEL_OVRD_EN",
    "rxd4 CKTAP delay override enable. When set enables CKTAP delay to be controlled from this register.",
#endif
    UBUS_SLV__RX_DELAY_CNTRL_1_RXD4_DEL_OVRD_EN_FIELD_MASK,
    0,
    UBUS_SLV__RX_DELAY_CNTRL_1_RXD4_DEL_OVRD_EN_FIELD_WIDTH,
    UBUS_SLV__RX_DELAY_CNTRL_1_RXD4_DEL_OVRD_EN_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UBUS_SLV__RX_DELAY_CNTRL_1_RXD5_DEL_SEL
 ******************************************************************************/
const ru_field_rec UBUS_SLV__RX_DELAY_CNTRL_1_RXD5_DEL_SEL_FIELD =
{
    "RXD5_DEL_SEL",
#if RU_INCLUDE_DESC
    "RXD5_DEL_SEL",
    "rxd5 CKTAP delay control. Refer to the CKTAP datasheet for programming",
#endif
    UBUS_SLV__RX_DELAY_CNTRL_1_RXD5_DEL_SEL_FIELD_MASK,
    0,
    UBUS_SLV__RX_DELAY_CNTRL_1_RXD5_DEL_SEL_FIELD_WIDTH,
    UBUS_SLV__RX_DELAY_CNTRL_1_RXD5_DEL_SEL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UBUS_SLV__RX_DELAY_CNTRL_1_RXD5_DEL_OVRD_EN
 ******************************************************************************/
const ru_field_rec UBUS_SLV__RX_DELAY_CNTRL_1_RXD5_DEL_OVRD_EN_FIELD =
{
    "RXD5_DEL_OVRD_EN",
#if RU_INCLUDE_DESC
    "RXD5_DEL_OVRD_EN",
    "rxd5 CKTAP delay override enable. When set enables CKTAP delay to be controlled from this register.",
#endif
    UBUS_SLV__RX_DELAY_CNTRL_1_RXD5_DEL_OVRD_EN_FIELD_MASK,
    0,
    UBUS_SLV__RX_DELAY_CNTRL_1_RXD5_DEL_OVRD_EN_FIELD_WIDTH,
    UBUS_SLV__RX_DELAY_CNTRL_1_RXD5_DEL_OVRD_EN_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UBUS_SLV__RX_DELAY_CNTRL_1_RXD6_DEL_SEL
 ******************************************************************************/
const ru_field_rec UBUS_SLV__RX_DELAY_CNTRL_1_RXD6_DEL_SEL_FIELD =
{
    "RXD6_DEL_SEL",
#if RU_INCLUDE_DESC
    "RXD6_DEL_SEL",
    "rxd6 CKTAP delay control. Refer to the CKTAP datasheet for programming",
#endif
    UBUS_SLV__RX_DELAY_CNTRL_1_RXD6_DEL_SEL_FIELD_MASK,
    0,
    UBUS_SLV__RX_DELAY_CNTRL_1_RXD6_DEL_SEL_FIELD_WIDTH,
    UBUS_SLV__RX_DELAY_CNTRL_1_RXD6_DEL_SEL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UBUS_SLV__RX_DELAY_CNTRL_1_RXD6_DEL_OVRD_EN
 ******************************************************************************/
const ru_field_rec UBUS_SLV__RX_DELAY_CNTRL_1_RXD6_DEL_OVRD_EN_FIELD =
{
    "RXD6_DEL_OVRD_EN",
#if RU_INCLUDE_DESC
    "RXD6_DEL_OVRD_EN",
    "rxd6 CKTAP delay override enable. When set enables CKTAP delay to be controlled from this register.",
#endif
    UBUS_SLV__RX_DELAY_CNTRL_1_RXD6_DEL_OVRD_EN_FIELD_MASK,
    0,
    UBUS_SLV__RX_DELAY_CNTRL_1_RXD6_DEL_OVRD_EN_FIELD_WIDTH,
    UBUS_SLV__RX_DELAY_CNTRL_1_RXD6_DEL_OVRD_EN_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UBUS_SLV__RX_DELAY_CNTRL_1_RXD7_DEL_SEL
 ******************************************************************************/
const ru_field_rec UBUS_SLV__RX_DELAY_CNTRL_1_RXD7_DEL_SEL_FIELD =
{
    "RXD7_DEL_SEL",
#if RU_INCLUDE_DESC
    "RXD7_DEL_SEL",
    "rxd7 CKTAP delay control. Refer to the CKTAP datasheet for programming",
#endif
    UBUS_SLV__RX_DELAY_CNTRL_1_RXD7_DEL_SEL_FIELD_MASK,
    0,
    UBUS_SLV__RX_DELAY_CNTRL_1_RXD7_DEL_SEL_FIELD_WIDTH,
    UBUS_SLV__RX_DELAY_CNTRL_1_RXD7_DEL_SEL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UBUS_SLV__RX_DELAY_CNTRL_1_RXD7_DEL_OVRD_EN
 ******************************************************************************/
const ru_field_rec UBUS_SLV__RX_DELAY_CNTRL_1_RXD7_DEL_OVRD_EN_FIELD =
{
    "RXD7_DEL_OVRD_EN",
#if RU_INCLUDE_DESC
    "RXD7_DEL_OVRD_EN",
    "rxd7 CKTAP delay override enable. When set enables CKTAP delay to be controlled from this register.",
#endif
    UBUS_SLV__RX_DELAY_CNTRL_1_RXD7_DEL_OVRD_EN_FIELD_MASK,
    0,
    UBUS_SLV__RX_DELAY_CNTRL_1_RXD7_DEL_OVRD_EN_FIELD_WIDTH,
    UBUS_SLV__RX_DELAY_CNTRL_1_RXD7_DEL_OVRD_EN_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UBUS_SLV__RX_DELAY_CNTRL_1_RESERVED0
 ******************************************************************************/
const ru_field_rec UBUS_SLV__RX_DELAY_CNTRL_1_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    UBUS_SLV__RX_DELAY_CNTRL_1_RESERVED0_FIELD_MASK,
    0,
    UBUS_SLV__RX_DELAY_CNTRL_1_RESERVED0_FIELD_WIDTH,
    UBUS_SLV__RX_DELAY_CNTRL_1_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UBUS_SLV__RX_DELAY_CNTRL_2_RXCTL_POS_DEL_SEL
 ******************************************************************************/
const ru_field_rec UBUS_SLV__RX_DELAY_CNTRL_2_RXCTL_POS_DEL_SEL_FIELD =
{
    "RXCTL_POS_DEL_SEL",
#if RU_INCLUDE_DESC
    "RXCTL_POS_DEL_SEL",
    "rxctl_pos CKTAP delay control. Refer to the CKTAP datasheet for programming",
#endif
    UBUS_SLV__RX_DELAY_CNTRL_2_RXCTL_POS_DEL_SEL_FIELD_MASK,
    0,
    UBUS_SLV__RX_DELAY_CNTRL_2_RXCTL_POS_DEL_SEL_FIELD_WIDTH,
    UBUS_SLV__RX_DELAY_CNTRL_2_RXCTL_POS_DEL_SEL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UBUS_SLV__RX_DELAY_CNTRL_2_RXCTL_POS_DEL_OVRD_EN
 ******************************************************************************/
const ru_field_rec UBUS_SLV__RX_DELAY_CNTRL_2_RXCTL_POS_DEL_OVRD_EN_FIELD =
{
    "RXCTL_POS_DEL_OVRD_EN",
#if RU_INCLUDE_DESC
    "RXCTL_POS_DEL_OVRD_EN",
    "rxctl_pos CKTAP delay override enable. When set enables CKTAP delay to be controlled from this register.",
#endif
    UBUS_SLV__RX_DELAY_CNTRL_2_RXCTL_POS_DEL_OVRD_EN_FIELD_MASK,
    0,
    UBUS_SLV__RX_DELAY_CNTRL_2_RXCTL_POS_DEL_OVRD_EN_FIELD_WIDTH,
    UBUS_SLV__RX_DELAY_CNTRL_2_RXCTL_POS_DEL_OVRD_EN_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UBUS_SLV__RX_DELAY_CNTRL_2_RXCTL_NEG_DEL_SEL
 ******************************************************************************/
const ru_field_rec UBUS_SLV__RX_DELAY_CNTRL_2_RXCTL_NEG_DEL_SEL_FIELD =
{
    "RXCTL_NEG_DEL_SEL",
#if RU_INCLUDE_DESC
    "RXCTL_NEG_DEL_SEL",
    "rxctl_neg CKTAP delay control. Refer to the CKTAP datasheet for programming.",
#endif
    UBUS_SLV__RX_DELAY_CNTRL_2_RXCTL_NEG_DEL_SEL_FIELD_MASK,
    0,
    UBUS_SLV__RX_DELAY_CNTRL_2_RXCTL_NEG_DEL_SEL_FIELD_WIDTH,
    UBUS_SLV__RX_DELAY_CNTRL_2_RXCTL_NEG_DEL_SEL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UBUS_SLV__RX_DELAY_CNTRL_2_RXCTL_NEG_DEL_OVRD_EN
 ******************************************************************************/
const ru_field_rec UBUS_SLV__RX_DELAY_CNTRL_2_RXCTL_NEG_DEL_OVRD_EN_FIELD =
{
    "RXCTL_NEG_DEL_OVRD_EN",
#if RU_INCLUDE_DESC
    "RXCTL_NEG_DEL_OVRD_EN",
    "rxctl_neg CKTAP delay override enable. When set enables CKTAP delay to be controlled from this register.",
#endif
    UBUS_SLV__RX_DELAY_CNTRL_2_RXCTL_NEG_DEL_OVRD_EN_FIELD_MASK,
    0,
    UBUS_SLV__RX_DELAY_CNTRL_2_RXCTL_NEG_DEL_OVRD_EN_FIELD_WIDTH,
    UBUS_SLV__RX_DELAY_CNTRL_2_RXCTL_NEG_DEL_OVRD_EN_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UBUS_SLV__RX_DELAY_CNTRL_2_RXCLK_DEL_SEL
 ******************************************************************************/
const ru_field_rec UBUS_SLV__RX_DELAY_CNTRL_2_RXCLK_DEL_SEL_FIELD =
{
    "RXCLK_DEL_SEL",
#if RU_INCLUDE_DESC
    "RXCLK_DEL_SEL",
    "rxclk CKTAP delay control. Refer to the CKTAP datasheet for programming.",
#endif
    UBUS_SLV__RX_DELAY_CNTRL_2_RXCLK_DEL_SEL_FIELD_MASK,
    0,
    UBUS_SLV__RX_DELAY_CNTRL_2_RXCLK_DEL_SEL_FIELD_WIDTH,
    UBUS_SLV__RX_DELAY_CNTRL_2_RXCLK_DEL_SEL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UBUS_SLV__RX_DELAY_CNTRL_2_RXCLK_DEL_OVRD_EN
 ******************************************************************************/
const ru_field_rec UBUS_SLV__RX_DELAY_CNTRL_2_RXCLK_DEL_OVRD_EN_FIELD =
{
    "RXCLK_DEL_OVRD_EN",
#if RU_INCLUDE_DESC
    "RXCLK_DEL_OVRD_EN",
    "rxclk CKTAP delay override enable. When set enables CKTAP delay to be controlled from this register.",
#endif
    UBUS_SLV__RX_DELAY_CNTRL_2_RXCLK_DEL_OVRD_EN_FIELD_MASK,
    0,
    UBUS_SLV__RX_DELAY_CNTRL_2_RXCLK_DEL_OVRD_EN_FIELD_WIDTH,
    UBUS_SLV__RX_DELAY_CNTRL_2_RXCLK_DEL_OVRD_EN_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UBUS_SLV__RX_DELAY_CNTRL_2_RESERVED0
 ******************************************************************************/
const ru_field_rec UBUS_SLV__RX_DELAY_CNTRL_2_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    UBUS_SLV__RX_DELAY_CNTRL_2_RESERVED0_FIELD_MASK,
    0,
    UBUS_SLV__RX_DELAY_CNTRL_2_RESERVED0_FIELD_WIDTH,
    UBUS_SLV__RX_DELAY_CNTRL_2_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UBUS_SLV__CLK_RST_CTRL_SWINIT
 ******************************************************************************/
const ru_field_rec UBUS_SLV__CLK_RST_CTRL_SWINIT_FIELD =
{
    "SWINIT",
#if RU_INCLUDE_DESC
    "SW_INIT",
    "SW init",
#endif
    UBUS_SLV__CLK_RST_CTRL_SWINIT_FIELD_MASK,
    0,
    UBUS_SLV__CLK_RST_CTRL_SWINIT_FIELD_WIDTH,
    UBUS_SLV__CLK_RST_CTRL_SWINIT_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UBUS_SLV__CLK_RST_CTRL_CLK250EN
 ******************************************************************************/
const ru_field_rec UBUS_SLV__CLK_RST_CTRL_CLK250EN_FIELD =
{
    "CLK250EN",
#if RU_INCLUDE_DESC
    "CLK250EN",
    "Enables clock 250",
#endif
    UBUS_SLV__CLK_RST_CTRL_CLK250EN_FIELD_MASK,
    0,
    UBUS_SLV__CLK_RST_CTRL_CLK250EN_FIELD_WIDTH,
    UBUS_SLV__CLK_RST_CTRL_CLK250EN_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UBUS_SLV__CLK_RST_CTRL_RESERVED0
 ******************************************************************************/
const ru_field_rec UBUS_SLV__CLK_RST_CTRL_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    UBUS_SLV__CLK_RST_CTRL_RESERVED0_FIELD_MASK,
    0,
    UBUS_SLV__CLK_RST_CTRL_RESERVED0_FIELD_WIDTH,
    UBUS_SLV__CLK_RST_CTRL_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

#endif /* RU_INCLUDE_FIELD_DB */

/******************************************************************************
 * Register: UBUS_SLV_VPB_BASE
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *UBUS_SLV_VPB_BASE_FIELDS[] =
{
    &UBUS_SLV_VPB_BASE_BASE_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec UBUS_SLV_VPB_BASE_REG = 
{
    "VPB_BASE",
#if RU_INCLUDE_DESC
    "VPB_BASE Register",
    "VPB Base address",
#endif
    UBUS_SLV_VPB_BASE_REG_OFFSET,
    0,
    0,
    541,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    UBUS_SLV_VPB_BASE_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: UBUS_SLV_VPB_MASK
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *UBUS_SLV_VPB_MASK_FIELDS[] =
{
    &UBUS_SLV_VPB_MASK_MASK_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec UBUS_SLV_VPB_MASK_REG = 
{
    "VPB_MASK",
#if RU_INCLUDE_DESC
    "VPB_MASK Register",
    "VPB mask address",
#endif
    UBUS_SLV_VPB_MASK_REG_OFFSET,
    0,
    0,
    542,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    UBUS_SLV_VPB_MASK_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: UBUS_SLV_APB_BASE
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *UBUS_SLV_APB_BASE_FIELDS[] =
{
    &UBUS_SLV_APB_BASE_BASE_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec UBUS_SLV_APB_BASE_REG = 
{
    "APB_BASE",
#if RU_INCLUDE_DESC
    "APB_BASE Register",
    "APB Base address",
#endif
    UBUS_SLV_APB_BASE_REG_OFFSET,
    0,
    0,
    543,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    UBUS_SLV_APB_BASE_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: UBUS_SLV_APB_MASK
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *UBUS_SLV_APB_MASK_FIELDS[] =
{
    &UBUS_SLV_APB_MASK_MASK_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec UBUS_SLV_APB_MASK_REG = 
{
    "APB_MASK",
#if RU_INCLUDE_DESC
    "APB_MASK Register",
    "APB mask address",
#endif
    UBUS_SLV_APB_MASK_REG_OFFSET,
    0,
    0,
    544,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    UBUS_SLV_APB_MASK_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: UBUS_SLV_DQM_BASE
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *UBUS_SLV_DQM_BASE_FIELDS[] =
{
    &UBUS_SLV_DQM_BASE_BASE_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec UBUS_SLV_DQM_BASE_REG = 
{
    "DQM_BASE",
#if RU_INCLUDE_DESC
    "DQM_BASE Register",
    "DQM Base address",
#endif
    UBUS_SLV_DQM_BASE_REG_OFFSET,
    0,
    0,
    545,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    UBUS_SLV_DQM_BASE_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: UBUS_SLV_DQM_MASK
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *UBUS_SLV_DQM_MASK_FIELDS[] =
{
    &UBUS_SLV_DQM_MASK_MASK_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec UBUS_SLV_DQM_MASK_REG = 
{
    "DQM_MASK",
#if RU_INCLUDE_DESC
    "DQM_MASK Register",
    "DQM mask address",
#endif
    UBUS_SLV_DQM_MASK_REG_OFFSET,
    0,
    0,
    546,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    UBUS_SLV_DQM_MASK_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: UBUS_SLV_RNR_INTR_CTRL_ISR
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *UBUS_SLV_RNR_INTR_CTRL_ISR_FIELDS[] =
{
    &UBUS_SLV_RNR_INTR_CTRL_ISR_IST_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec UBUS_SLV_RNR_INTR_CTRL_ISR_REG = 
{
    "RNR_INTR_CTRL_ISR",
#if RU_INCLUDE_DESC
    "INTERRUPT_STATUS_REGISTER Register",
    "This register contains the current active QM interrupts. Each asserted bit represents an active interrupt source. The interrupt remains active until the software clears it by writing 1 to the corresponding bit.",
#endif
    UBUS_SLV_RNR_INTR_CTRL_ISR_REG_OFFSET,
    0,
    0,
    547,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    UBUS_SLV_RNR_INTR_CTRL_ISR_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: UBUS_SLV_RNR_INTR_CTRL_ISM
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *UBUS_SLV_RNR_INTR_CTRL_ISM_FIELDS[] =
{
    &UBUS_SLV_RNR_INTR_CTRL_ISM_ISM_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec UBUS_SLV_RNR_INTR_CTRL_ISM_REG = 
{
    "RNR_INTR_CTRL_ISM",
#if RU_INCLUDE_DESC
    "INTERRUPT_STATUS_MASKED_REGISTER Register",
    "This register provides only the  enabled interrupts for each of the interrupt sources depicted in the ISR register.",
#endif
    UBUS_SLV_RNR_INTR_CTRL_ISM_REG_OFFSET,
    0,
    0,
    548,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    UBUS_SLV_RNR_INTR_CTRL_ISM_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: UBUS_SLV_RNR_INTR_CTRL_IER
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *UBUS_SLV_RNR_INTR_CTRL_IER_FIELDS[] =
{
    &UBUS_SLV_RNR_INTR_CTRL_IER_IEM_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec UBUS_SLV_RNR_INTR_CTRL_IER_REG = 
{
    "RNR_INTR_CTRL_IER",
#if RU_INCLUDE_DESC
    "INTERRUPT_ENABLE_REGISTER Register",
    "This register provides an enable mask for each of the interrupt sources depicted in the ISR register.",
#endif
    UBUS_SLV_RNR_INTR_CTRL_IER_REG_OFFSET,
    0,
    0,
    549,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    UBUS_SLV_RNR_INTR_CTRL_IER_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: UBUS_SLV_RNR_INTR_CTRL_ITR
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *UBUS_SLV_RNR_INTR_CTRL_ITR_FIELDS[] =
{
    &UBUS_SLV_RNR_INTR_CTRL_ITR_IST_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec UBUS_SLV_RNR_INTR_CTRL_ITR_REG = 
{
    "RNR_INTR_CTRL_ITR",
#if RU_INCLUDE_DESC
    "INTERRUPT_TEST_REGISTER Register",
    "This register enables testing by simulating interrupt sources. When the software sets a bit in the ITR, the corresponding bit in the ISR shows an active interrupt. The interrupt remains active until software clears the bit in the ITR",
#endif
    UBUS_SLV_RNR_INTR_CTRL_ITR_REG_OFFSET,
    0,
    0,
    550,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    UBUS_SLV_RNR_INTR_CTRL_ITR_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: UBUS_SLV_PROFILING_CFG
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *UBUS_SLV_PROFILING_CFG_FIELDS[] =
{
    &UBUS_SLV_PROFILING_CFG_COUNTER_ENABLE_FIELD,
    &UBUS_SLV_PROFILING_CFG_PROFILING_START_FIELD,
    &UBUS_SLV_PROFILING_CFG_MANUAL_STOP_MODE_FIELD,
    &UBUS_SLV_PROFILING_CFG_DO_MANUAL_STOP_FIELD,
    &UBUS_SLV_PROFILING_CFG_RESERVED0_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec UBUS_SLV_PROFILING_CFG_REG = 
{
    "PROFILING_CFG",
#if RU_INCLUDE_DESC
    "PROFILING_CFG Register",
    "Profiling configuration settings",
#endif
    UBUS_SLV_PROFILING_CFG_REG_OFFSET,
    0,
    0,
    551,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    5,
    UBUS_SLV_PROFILING_CFG_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: UBUS_SLV_PROFILING_STATUS
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *UBUS_SLV_PROFILING_STATUS_FIELDS[] =
{
    &UBUS_SLV_PROFILING_STATUS_PROFILING_ON_FIELD,
    &UBUS_SLV_PROFILING_STATUS_CYCLES_COUNTER_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec UBUS_SLV_PROFILING_STATUS_REG = 
{
    "PROFILING_STATUS",
#if RU_INCLUDE_DESC
    "PROFILING_STATUS Register",
    "Profiling status",
#endif
    UBUS_SLV_PROFILING_STATUS_REG_OFFSET,
    0,
    0,
    552,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    UBUS_SLV_PROFILING_STATUS_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: UBUS_SLV_PROFILING_COUNTER
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *UBUS_SLV_PROFILING_COUNTER_FIELDS[] =
{
    &UBUS_SLV_PROFILING_COUNTER_VAL_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec UBUS_SLV_PROFILING_COUNTER_REG = 
{
    "PROFILING_COUNTER",
#if RU_INCLUDE_DESC
    "PROFILING_COUNTER Register",
    "Read PROFILING_COUNTER current value",
#endif
    UBUS_SLV_PROFILING_COUNTER_REG_OFFSET,
    0,
    0,
    553,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    UBUS_SLV_PROFILING_COUNTER_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: UBUS_SLV_PROFILING_START_VALUE
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *UBUS_SLV_PROFILING_START_VALUE_FIELDS[] =
{
    &UBUS_SLV_PROFILING_START_VALUE_VAL_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec UBUS_SLV_PROFILING_START_VALUE_REG = 
{
    "PROFILING_START_VALUE",
#if RU_INCLUDE_DESC
    "PROFILING_START_VALUE Register",
    "Read PROFILING_START_VALUE value",
#endif
    UBUS_SLV_PROFILING_START_VALUE_REG_OFFSET,
    0,
    0,
    554,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    UBUS_SLV_PROFILING_START_VALUE_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: UBUS_SLV_PROFILING_STOP_VALUE
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *UBUS_SLV_PROFILING_STOP_VALUE_FIELDS[] =
{
    &UBUS_SLV_PROFILING_STOP_VALUE_VAL_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec UBUS_SLV_PROFILING_STOP_VALUE_REG = 
{
    "PROFILING_STOP_VALUE",
#if RU_INCLUDE_DESC
    "PROFILING_STOP_VALUE Register",
    "Read PROFILING_STOP_VALUE value",
#endif
    UBUS_SLV_PROFILING_STOP_VALUE_REG_OFFSET,
    0,
    0,
    555,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    UBUS_SLV_PROFILING_STOP_VALUE_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: UBUS_SLV_PROFILING_CYCLE_NUM
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *UBUS_SLV_PROFILING_CYCLE_NUM_FIELDS[] =
{
    &UBUS_SLV_PROFILING_CYCLE_NUM_PROFILING_CYCLES_NUM_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec UBUS_SLV_PROFILING_CYCLE_NUM_REG = 
{
    "PROFILING_CYCLE_NUM",
#if RU_INCLUDE_DESC
    "PROFILING_CYCLE_NUM Register",
    "Set length of profiling window",
#endif
    UBUS_SLV_PROFILING_CYCLE_NUM_REG_OFFSET,
    0,
    0,
    556,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    UBUS_SLV_PROFILING_CYCLE_NUM_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: UBUS_SLV__CNTRL
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *UBUS_SLV__CNTRL_FIELDS[] =
{
    &UBUS_SLV__CNTRL_RGMII_MODE_EN_FIELD,
    &UBUS_SLV__CNTRL_ID_MODE_DIS_FIELD,
    &UBUS_SLV__CNTRL_PORT_MODE_FIELD,
    &UBUS_SLV__CNTRL_RVMII_REF_SEL_FIELD,
    &UBUS_SLV__CNTRL_RX_PAUSE_EN_FIELD,
    &UBUS_SLV__CNTRL_TX_PAUSE_EN_FIELD,
    &UBUS_SLV__CNTRL_TX_CLK_STOP_EN_FIELD,
    &UBUS_SLV__CNTRL_LPI_COUNT_FIELD,
    &UBUS_SLV__CNTRL_RX_ERR_MASK_FIELD,
    &UBUS_SLV__CNTRL_COL_CRS_MASK_FIELD,
    &UBUS_SLV__CNTRL_RESERVED0_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec UBUS_SLV__CNTRL_REG = 
{
    "_CNTRL",
#if RU_INCLUDE_DESC
    "RGMII_CNTRL Register",
    "RGMII Control Register",
#endif
    UBUS_SLV__CNTRL_REG_OFFSET,
    0,
    0,
    557,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    11,
    UBUS_SLV__CNTRL_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: UBUS_SLV__IB_STATUS
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *UBUS_SLV__IB_STATUS_FIELDS[] =
{
    &UBUS_SLV__IB_STATUS_SPEED_DECODE_FIELD,
    &UBUS_SLV__IB_STATUS_DUPLEX_DECODE_FIELD,
    &UBUS_SLV__IB_STATUS_LINK_DECODE_FIELD,
    &UBUS_SLV__IB_STATUS_IB_STATUS_OVRD_FIELD,
    &UBUS_SLV__IB_STATUS_RESERVED0_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec UBUS_SLV__IB_STATUS_REG = 
{
    "_IB_STATUS",
#if RU_INCLUDE_DESC
    "RGMII_IB_STATUS Register",
    "RGMII IB Status Register",
#endif
    UBUS_SLV__IB_STATUS_REG_OFFSET,
    0,
    0,
    558,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    5,
    UBUS_SLV__IB_STATUS_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: UBUS_SLV__RX_CLOCK_DELAY_CNTRL
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *UBUS_SLV__RX_CLOCK_DELAY_CNTRL_FIELDS[] =
{
    &UBUS_SLV__RX_CLOCK_DELAY_CNTRL_CTRI_FIELD,
    &UBUS_SLV__RX_CLOCK_DELAY_CNTRL_DRNG_FIELD,
    &UBUS_SLV__RX_CLOCK_DELAY_CNTRL_IDDQ_FIELD,
    &UBUS_SLV__RX_CLOCK_DELAY_CNTRL_BYPASS_FIELD,
    &UBUS_SLV__RX_CLOCK_DELAY_CNTRL_DLY_SEL_FIELD,
    &UBUS_SLV__RX_CLOCK_DELAY_CNTRL_DLY_OVERRIDE_FIELD,
    &UBUS_SLV__RX_CLOCK_DELAY_CNTRL_RESET_FIELD,
    &UBUS_SLV__RX_CLOCK_DELAY_CNTRL_RESERVED0_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec UBUS_SLV__RX_CLOCK_DELAY_CNTRL_REG = 
{
    "_RX_CLOCK_DELAY_CNTRL",
#if RU_INCLUDE_DESC
    "RGMII_RX_CLOCK_DELAY_CNTRL Register",
    "RGMII RX Clock Delay Control Register",
#endif
    UBUS_SLV__RX_CLOCK_DELAY_CNTRL_REG_OFFSET,
    0,
    0,
    559,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    8,
    UBUS_SLV__RX_CLOCK_DELAY_CNTRL_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: UBUS_SLV__ATE_RX_CNTRL_EXP_DATA
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *UBUS_SLV__ATE_RX_CNTRL_EXP_DATA_FIELDS[] =
{
    &UBUS_SLV__ATE_RX_CNTRL_EXP_DATA_EXPECTED_DATA_0_FIELD,
    &UBUS_SLV__ATE_RX_CNTRL_EXP_DATA_EXPECTED_DATA_1_FIELD,
    &UBUS_SLV__ATE_RX_CNTRL_EXP_DATA_GOOD_COUNT_FIELD,
    &UBUS_SLV__ATE_RX_CNTRL_EXP_DATA_PKT_COUNT_RST_FIELD,
    &UBUS_SLV__ATE_RX_CNTRL_EXP_DATA_ATE_EN_FIELD,
    &UBUS_SLV__ATE_RX_CNTRL_EXP_DATA_RESERVED0_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec UBUS_SLV__ATE_RX_CNTRL_EXP_DATA_REG = 
{
    "_ATE_RX_CNTRL_EXP_DATA",
#if RU_INCLUDE_DESC
    "RGMII_ATE_RX_CNTRL_EXP_DATA Register",
    "RGMII port ATE RX Control and Expected Data Register",
#endif
    UBUS_SLV__ATE_RX_CNTRL_EXP_DATA_REG_OFFSET,
    0,
    0,
    560,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    6,
    UBUS_SLV__ATE_RX_CNTRL_EXP_DATA_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: UBUS_SLV__ATE_RX_EXP_DATA_1
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *UBUS_SLV__ATE_RX_EXP_DATA_1_FIELDS[] =
{
    &UBUS_SLV__ATE_RX_EXP_DATA_1_EXPECTED_DATA_2_FIELD,
    &UBUS_SLV__ATE_RX_EXP_DATA_1_EXPECTED_DATA_3_FIELD,
    &UBUS_SLV__ATE_RX_EXP_DATA_1_RESERVED0_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec UBUS_SLV__ATE_RX_EXP_DATA_1_REG = 
{
    "_ATE_RX_EXP_DATA_1",
#if RU_INCLUDE_DESC
    "RGMII_ATE_RX_EXP_DATA_1 Register",
    "RGMII port ATE RX Expected Data 1 Register",
#endif
    UBUS_SLV__ATE_RX_EXP_DATA_1_REG_OFFSET,
    0,
    0,
    561,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    3,
    UBUS_SLV__ATE_RX_EXP_DATA_1_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: UBUS_SLV__ATE_RX_STATUS_0
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *UBUS_SLV__ATE_RX_STATUS_0_FIELDS[] =
{
    &UBUS_SLV__ATE_RX_STATUS_0_RECEIVED_DATA_0_FIELD,
    &UBUS_SLV__ATE_RX_STATUS_0_RECEIVED_DATA_1_FIELD,
    &UBUS_SLV__ATE_RX_STATUS_0_RX_OK_FIELD,
    &UBUS_SLV__ATE_RX_STATUS_0_RESERVED0_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec UBUS_SLV__ATE_RX_STATUS_0_REG = 
{
    "_ATE_RX_STATUS_0",
#if RU_INCLUDE_DESC
    "RGMII_ATE_RX_STATUS_0 Register",
    "RGMII port ATE RX Status 0 Register",
#endif
    UBUS_SLV__ATE_RX_STATUS_0_REG_OFFSET,
    0,
    0,
    562,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    4,
    UBUS_SLV__ATE_RX_STATUS_0_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: UBUS_SLV__ATE_RX_STATUS_1
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *UBUS_SLV__ATE_RX_STATUS_1_FIELDS[] =
{
    &UBUS_SLV__ATE_RX_STATUS_1_RECEIVED_DATA_2_FIELD,
    &UBUS_SLV__ATE_RX_STATUS_1_RECEIVED_DATA_3_FIELD,
    &UBUS_SLV__ATE_RX_STATUS_1_RESERVED0_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec UBUS_SLV__ATE_RX_STATUS_1_REG = 
{
    "_ATE_RX_STATUS_1",
#if RU_INCLUDE_DESC
    "RGMII_ATE_RX_STATUS_1 Register",
    "RGMII port ATE RX Status 1 Register",
#endif
    UBUS_SLV__ATE_RX_STATUS_1_REG_OFFSET,
    0,
    0,
    563,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    3,
    UBUS_SLV__ATE_RX_STATUS_1_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: UBUS_SLV__ATE_TX_CNTRL
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *UBUS_SLV__ATE_TX_CNTRL_FIELDS[] =
{
    &UBUS_SLV__ATE_TX_CNTRL_START_STOP_OVRD_FIELD,
    &UBUS_SLV__ATE_TX_CNTRL_START_STOP_FIELD,
    &UBUS_SLV__ATE_TX_CNTRL_PKT_GEN_EN_FIELD,
    &UBUS_SLV__ATE_TX_CNTRL_PKT_CNT_FIELD,
    &UBUS_SLV__ATE_TX_CNTRL_PAYLOAD_LENGTH_FIELD,
    &UBUS_SLV__ATE_TX_CNTRL_PKT_IPG_FIELD,
    &UBUS_SLV__ATE_TX_CNTRL_RESERVED0_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec UBUS_SLV__ATE_TX_CNTRL_REG = 
{
    "_ATE_TX_CNTRL",
#if RU_INCLUDE_DESC
    "RGMII_ATE_TX_CNTRL Register",
    "RGMII port ATE TX Control Register",
#endif
    UBUS_SLV__ATE_TX_CNTRL_REG_OFFSET,
    0,
    0,
    564,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    7,
    UBUS_SLV__ATE_TX_CNTRL_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: UBUS_SLV__ATE_TX_DATA_0
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *UBUS_SLV__ATE_TX_DATA_0_FIELDS[] =
{
    &UBUS_SLV__ATE_TX_DATA_0_TX_DATA_0_FIELD,
    &UBUS_SLV__ATE_TX_DATA_0_TX_DATA_1_FIELD,
    &UBUS_SLV__ATE_TX_DATA_0_RESERVED0_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec UBUS_SLV__ATE_TX_DATA_0_REG = 
{
    "_ATE_TX_DATA_0",
#if RU_INCLUDE_DESC
    "RGMII_ATE_TX_DATA_0 Register",
    "RGMII port ATE TX Data 0 Register",
#endif
    UBUS_SLV__ATE_TX_DATA_0_REG_OFFSET,
    0,
    0,
    565,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    3,
    UBUS_SLV__ATE_TX_DATA_0_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: UBUS_SLV__ATE_TX_DATA_1
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *UBUS_SLV__ATE_TX_DATA_1_FIELDS[] =
{
    &UBUS_SLV__ATE_TX_DATA_1_TX_DATA_2_FIELD,
    &UBUS_SLV__ATE_TX_DATA_1_TX_DATA_3_FIELD,
    &UBUS_SLV__ATE_TX_DATA_1_RESERVED0_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec UBUS_SLV__ATE_TX_DATA_1_REG = 
{
    "_ATE_TX_DATA_1",
#if RU_INCLUDE_DESC
    "RGMII_ATE_TX_DATA_1 Register",
    "RGMII port ATE TX Data 1 Register",
#endif
    UBUS_SLV__ATE_TX_DATA_1_REG_OFFSET,
    0,
    0,
    566,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    3,
    UBUS_SLV__ATE_TX_DATA_1_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: UBUS_SLV__ATE_TX_DATA_2
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *UBUS_SLV__ATE_TX_DATA_2_FIELDS[] =
{
    &UBUS_SLV__ATE_TX_DATA_2_TX_DATA_4_FIELD,
    &UBUS_SLV__ATE_TX_DATA_2_TX_DATA_5_FIELD,
    &UBUS_SLV__ATE_TX_DATA_2_ETHER_TYPE_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec UBUS_SLV__ATE_TX_DATA_2_REG = 
{
    "_ATE_TX_DATA_2",
#if RU_INCLUDE_DESC
    "RGMII_ATE_TX_DATA_2 Register",
    "RGMII port ATE TX Data 2 Register",
#endif
    UBUS_SLV__ATE_TX_DATA_2_REG_OFFSET,
    0,
    0,
    567,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    3,
    UBUS_SLV__ATE_TX_DATA_2_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: UBUS_SLV__TX_DELAY_CNTRL_0
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *UBUS_SLV__TX_DELAY_CNTRL_0_FIELDS[] =
{
    &UBUS_SLV__TX_DELAY_CNTRL_0_TXD0_DEL_SEL_FIELD,
    &UBUS_SLV__TX_DELAY_CNTRL_0_TXD0_DEL_OVRD_EN_FIELD,
    &UBUS_SLV__TX_DELAY_CNTRL_0_TXD1_DEL_SEL_FIELD,
    &UBUS_SLV__TX_DELAY_CNTRL_0_TXD1_DEL_OVRD_EN_FIELD,
    &UBUS_SLV__TX_DELAY_CNTRL_0_TXD2_DEL_SEL_FIELD,
    &UBUS_SLV__TX_DELAY_CNTRL_0_TXD2_DEL_OVRD_EN_FIELD,
    &UBUS_SLV__TX_DELAY_CNTRL_0_TXD3_DEL_SEL_FIELD,
    &UBUS_SLV__TX_DELAY_CNTRL_0_TXD3_DEL_OVRD_EN_FIELD,
    &UBUS_SLV__TX_DELAY_CNTRL_0_RESERVED0_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec UBUS_SLV__TX_DELAY_CNTRL_0_REG = 
{
    "_TX_DELAY_CNTRL_0",
#if RU_INCLUDE_DESC
    "RGMII_TX_DELAY_CNTRL_0 Register",
    "RGMII TX Delay Control 0 Register",
#endif
    UBUS_SLV__TX_DELAY_CNTRL_0_REG_OFFSET,
    0,
    0,
    568,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    9,
    UBUS_SLV__TX_DELAY_CNTRL_0_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: UBUS_SLV__TX_DELAY_CNTRL_1
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *UBUS_SLV__TX_DELAY_CNTRL_1_FIELDS[] =
{
    &UBUS_SLV__TX_DELAY_CNTRL_1_TXCTL_DEL_SEL_FIELD,
    &UBUS_SLV__TX_DELAY_CNTRL_1_TXCTL_DEL_OVRD_EN_FIELD,
    &UBUS_SLV__TX_DELAY_CNTRL_1_TXCLK_DEL_SEL_FIELD,
    &UBUS_SLV__TX_DELAY_CNTRL_1_TXCLK_DEL_OVRD_EN_FIELD,
    &UBUS_SLV__TX_DELAY_CNTRL_1_TXCLK_ID_DEL_SEL_FIELD,
    &UBUS_SLV__TX_DELAY_CNTRL_1_TXCLK_ID_DEL_OVRD_EN_FIELD,
    &UBUS_SLV__TX_DELAY_CNTRL_1_RESERVED0_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec UBUS_SLV__TX_DELAY_CNTRL_1_REG = 
{
    "_TX_DELAY_CNTRL_1",
#if RU_INCLUDE_DESC
    "RGMII_TX_DELAY_CNTRL_1 Register",
    "RGMII TX Delay Control 1 Register",
#endif
    UBUS_SLV__TX_DELAY_CNTRL_1_REG_OFFSET,
    0,
    0,
    569,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    7,
    UBUS_SLV__TX_DELAY_CNTRL_1_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: UBUS_SLV__RX_DELAY_CNTRL_0
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *UBUS_SLV__RX_DELAY_CNTRL_0_FIELDS[] =
{
    &UBUS_SLV__RX_DELAY_CNTRL_0_RXD0_DEL_SEL_FIELD,
    &UBUS_SLV__RX_DELAY_CNTRL_0_RXD0_DEL_OVRD_EN_FIELD,
    &UBUS_SLV__RX_DELAY_CNTRL_0_RXD1_DEL_SEL_FIELD,
    &UBUS_SLV__RX_DELAY_CNTRL_0_RXD1_DEL_OVRD_EN_FIELD,
    &UBUS_SLV__RX_DELAY_CNTRL_0_RXD2_DEL_SEL_FIELD,
    &UBUS_SLV__RX_DELAY_CNTRL_0_RXD2_DEL_OVRD_EN_FIELD,
    &UBUS_SLV__RX_DELAY_CNTRL_0_RXD3_DEL_SEL_FIELD,
    &UBUS_SLV__RX_DELAY_CNTRL_0_RXD3_DEL_OVRD_EN_FIELD,
    &UBUS_SLV__RX_DELAY_CNTRL_0_RESERVED0_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec UBUS_SLV__RX_DELAY_CNTRL_0_REG = 
{
    "_RX_DELAY_CNTRL_0",
#if RU_INCLUDE_DESC
    "RGMII_RX_DELAY_CNTRL_0 Register",
    "RGMII RX Delay Control 0 Register",
#endif
    UBUS_SLV__RX_DELAY_CNTRL_0_REG_OFFSET,
    0,
    0,
    570,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    9,
    UBUS_SLV__RX_DELAY_CNTRL_0_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: UBUS_SLV__RX_DELAY_CNTRL_1
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *UBUS_SLV__RX_DELAY_CNTRL_1_FIELDS[] =
{
    &UBUS_SLV__RX_DELAY_CNTRL_1_RXD4_DEL_SEL_FIELD,
    &UBUS_SLV__RX_DELAY_CNTRL_1_RXD4_DEL_OVRD_EN_FIELD,
    &UBUS_SLV__RX_DELAY_CNTRL_1_RXD5_DEL_SEL_FIELD,
    &UBUS_SLV__RX_DELAY_CNTRL_1_RXD5_DEL_OVRD_EN_FIELD,
    &UBUS_SLV__RX_DELAY_CNTRL_1_RXD6_DEL_SEL_FIELD,
    &UBUS_SLV__RX_DELAY_CNTRL_1_RXD6_DEL_OVRD_EN_FIELD,
    &UBUS_SLV__RX_DELAY_CNTRL_1_RXD7_DEL_SEL_FIELD,
    &UBUS_SLV__RX_DELAY_CNTRL_1_RXD7_DEL_OVRD_EN_FIELD,
    &UBUS_SLV__RX_DELAY_CNTRL_1_RESERVED0_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec UBUS_SLV__RX_DELAY_CNTRL_1_REG = 
{
    "_RX_DELAY_CNTRL_1",
#if RU_INCLUDE_DESC
    "RGMII_RX_DELAY_CNTRL_1 Register",
    "RGMII RX Delay Control 1 Register",
#endif
    UBUS_SLV__RX_DELAY_CNTRL_1_REG_OFFSET,
    0,
    0,
    571,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    9,
    UBUS_SLV__RX_DELAY_CNTRL_1_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: UBUS_SLV__RX_DELAY_CNTRL_2
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *UBUS_SLV__RX_DELAY_CNTRL_2_FIELDS[] =
{
    &UBUS_SLV__RX_DELAY_CNTRL_2_RXCTL_POS_DEL_SEL_FIELD,
    &UBUS_SLV__RX_DELAY_CNTRL_2_RXCTL_POS_DEL_OVRD_EN_FIELD,
    &UBUS_SLV__RX_DELAY_CNTRL_2_RXCTL_NEG_DEL_SEL_FIELD,
    &UBUS_SLV__RX_DELAY_CNTRL_2_RXCTL_NEG_DEL_OVRD_EN_FIELD,
    &UBUS_SLV__RX_DELAY_CNTRL_2_RXCLK_DEL_SEL_FIELD,
    &UBUS_SLV__RX_DELAY_CNTRL_2_RXCLK_DEL_OVRD_EN_FIELD,
    &UBUS_SLV__RX_DELAY_CNTRL_2_RESERVED0_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec UBUS_SLV__RX_DELAY_CNTRL_2_REG = 
{
    "_RX_DELAY_CNTRL_2",
#if RU_INCLUDE_DESC
    "RGMII_RX_DELAY_CNTRL_2 Register",
    "RGMII RX Delay Control 2 Register",
#endif
    UBUS_SLV__RX_DELAY_CNTRL_2_REG_OFFSET,
    0,
    0,
    572,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    7,
    UBUS_SLV__RX_DELAY_CNTRL_2_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: UBUS_SLV__CLK_RST_CTRL
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *UBUS_SLV__CLK_RST_CTRL_FIELDS[] =
{
    &UBUS_SLV__CLK_RST_CTRL_SWINIT_FIELD,
    &UBUS_SLV__CLK_RST_CTRL_CLK250EN_FIELD,
    &UBUS_SLV__CLK_RST_CTRL_RESERVED0_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec UBUS_SLV__CLK_RST_CTRL_REG = 
{
    "_CLK_RST_CTRL",
#if RU_INCLUDE_DESC
    "RGMII_CLK_RST_CTRL Register",
    "Controls the following:"
    "i_sw_init"
    "i_clk_250_en",
#endif
    UBUS_SLV__CLK_RST_CTRL_REG_OFFSET,
    0,
    0,
    573,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    3,
    UBUS_SLV__CLK_RST_CTRL_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Block: UBUS_SLV
 ******************************************************************************/
static const ru_reg_rec *UBUS_SLV_REGS[] =
{
    &UBUS_SLV_VPB_BASE_REG,
    &UBUS_SLV_VPB_MASK_REG,
    &UBUS_SLV_APB_BASE_REG,
    &UBUS_SLV_APB_MASK_REG,
    &UBUS_SLV_DQM_BASE_REG,
    &UBUS_SLV_DQM_MASK_REG,
    &UBUS_SLV_RNR_INTR_CTRL_ISR_REG,
    &UBUS_SLV_RNR_INTR_CTRL_ISM_REG,
    &UBUS_SLV_RNR_INTR_CTRL_IER_REG,
    &UBUS_SLV_RNR_INTR_CTRL_ITR_REG,
    &UBUS_SLV_PROFILING_CFG_REG,
    &UBUS_SLV_PROFILING_STATUS_REG,
    &UBUS_SLV_PROFILING_COUNTER_REG,
    &UBUS_SLV_PROFILING_START_VALUE_REG,
    &UBUS_SLV_PROFILING_STOP_VALUE_REG,
    &UBUS_SLV_PROFILING_CYCLE_NUM_REG,
    &UBUS_SLV__CNTRL_REG,
    &UBUS_SLV__IB_STATUS_REG,
    &UBUS_SLV__RX_CLOCK_DELAY_CNTRL_REG,
    &UBUS_SLV__ATE_RX_CNTRL_EXP_DATA_REG,
    &UBUS_SLV__ATE_RX_EXP_DATA_1_REG,
    &UBUS_SLV__ATE_RX_STATUS_0_REG,
    &UBUS_SLV__ATE_RX_STATUS_1_REG,
    &UBUS_SLV__ATE_TX_CNTRL_REG,
    &UBUS_SLV__ATE_TX_DATA_0_REG,
    &UBUS_SLV__ATE_TX_DATA_1_REG,
    &UBUS_SLV__ATE_TX_DATA_2_REG,
    &UBUS_SLV__TX_DELAY_CNTRL_0_REG,
    &UBUS_SLV__TX_DELAY_CNTRL_1_REG,
    &UBUS_SLV__RX_DELAY_CNTRL_0_REG,
    &UBUS_SLV__RX_DELAY_CNTRL_1_REG,
    &UBUS_SLV__RX_DELAY_CNTRL_2_REG,
    &UBUS_SLV__CLK_RST_CTRL_REG,
};

unsigned long UBUS_SLV_ADDRS[] =
{
    0x82d97000,
};

const ru_block_rec UBUS_SLV_BLOCK = 
{
    "UBUS_SLV",
    UBUS_SLV_ADDRS,
    1,
    33,
    UBUS_SLV_REGS
};

/* End of file XRDP_UBUS_SLV.c */
