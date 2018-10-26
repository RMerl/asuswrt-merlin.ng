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

/******************************************************************************
 * Register: UNIMAC_RDP_UMAC_DUMMY
 ******************************************************************************/
const ru_reg_rec UNIMAC_RDP_UMAC_DUMMY_REG = 
{
    "UMAC_DUMMY",
#if RU_INCLUDE_DESC
    "UniMAC Dummy Register",
    "",
#endif
    UNIMAC_RDP_UMAC_DUMMY_REG_OFFSET,
    0,
    0,
    844,
};

/******************************************************************************
 * Register: UNIMAC_RDP_HD_BKP_CNTL
 ******************************************************************************/
const ru_reg_rec UNIMAC_RDP_HD_BKP_CNTL_REG = 
{
    "HD_BKP_CNTL",
#if RU_INCLUDE_DESC
    "UniMAC Half Duplex Backpressure Control Register",
    "",
#endif
    UNIMAC_RDP_HD_BKP_CNTL_REG_OFFSET,
    0,
    0,
    845,
};

/******************************************************************************
 * Register: UNIMAC_RDP_CMD
 ******************************************************************************/
const ru_reg_rec UNIMAC_RDP_CMD_REG = 
{
    "CMD",
#if RU_INCLUDE_DESC
    "UniMAC Command Register",
    "",
#endif
    UNIMAC_RDP_CMD_REG_OFFSET,
    0,
    0,
    846,
};

/******************************************************************************
 * Register: UNIMAC_RDP_MAC0
 ******************************************************************************/
const ru_reg_rec UNIMAC_RDP_MAC0_REG = 
{
    "MAC0",
#if RU_INCLUDE_DESC
    "UniMAC MAC 0",
    "",
#endif
    UNIMAC_RDP_MAC0_REG_OFFSET,
    0,
    0,
    847,
};

/******************************************************************************
 * Register: UNIMAC_RDP_MAC1
 ******************************************************************************/
const ru_reg_rec UNIMAC_RDP_MAC1_REG = 
{
    "MAC1",
#if RU_INCLUDE_DESC
    "UniMAC MAC 1",
    "",
#endif
    UNIMAC_RDP_MAC1_REG_OFFSET,
    0,
    0,
    848,
};

/******************************************************************************
 * Register: UNIMAC_RDP_FRM_LEN
 ******************************************************************************/
const ru_reg_rec UNIMAC_RDP_FRM_LEN_REG = 
{
    "FRM_LEN",
#if RU_INCLUDE_DESC
    "UniMAC Frame Length",
    "",
#endif
    UNIMAC_RDP_FRM_LEN_REG_OFFSET,
    0,
    0,
    849,
};

/******************************************************************************
 * Register: UNIMAC_RDP_PAUSE_QUNAT
 ******************************************************************************/
const ru_reg_rec UNIMAC_RDP_PAUSE_QUNAT_REG = 
{
    "PAUSE_QUNAT",
#if RU_INCLUDE_DESC
    "UniMAC Pause Quanta",
    "",
#endif
    UNIMAC_RDP_PAUSE_QUNAT_REG_OFFSET,
    0,
    0,
    850,
};

/******************************************************************************
 * Register: UNIMAC_RDP_SFD_OFFSET
 ******************************************************************************/
const ru_reg_rec UNIMAC_RDP_SFD_OFFSET_REG = 
{
    "SFD_OFFSET",
#if RU_INCLUDE_DESC
    "UniMAC EFM Preamble Length",
    "",
#endif
    UNIMAC_RDP_SFD_OFFSET_REG_OFFSET,
    0,
    0,
    851,
};

/******************************************************************************
 * Register: UNIMAC_RDP_MODE
 ******************************************************************************/
const ru_reg_rec UNIMAC_RDP_MODE_REG = 
{
    "MODE",
#if RU_INCLUDE_DESC
    "UniMAC Mode",
    "",
#endif
    UNIMAC_RDP_MODE_REG_OFFSET,
    0,
    0,
    852,
};

/******************************************************************************
 * Register: UNIMAC_RDP_FRM_TAG0
 ******************************************************************************/
const ru_reg_rec UNIMAC_RDP_FRM_TAG0_REG = 
{
    "FRM_TAG0",
#if RU_INCLUDE_DESC
    "UniMAC Preamble Outer TAG 0",
    "",
#endif
    UNIMAC_RDP_FRM_TAG0_REG_OFFSET,
    0,
    0,
    853,
};

/******************************************************************************
 * Register: UNIMAC_RDP_FRM_TAG1
 ******************************************************************************/
const ru_reg_rec UNIMAC_RDP_FRM_TAG1_REG = 
{
    "FRM_TAG1",
#if RU_INCLUDE_DESC
    "UniMAC Preamble Outer TAG 1",
    "",
#endif
    UNIMAC_RDP_FRM_TAG1_REG_OFFSET,
    0,
    0,
    854,
};

/******************************************************************************
 * Register: UNIMAC_RDP_TX_IPG_LEN
 ******************************************************************************/
const ru_reg_rec UNIMAC_RDP_TX_IPG_LEN_REG = 
{
    "TX_IPG_LEN",
#if RU_INCLUDE_DESC
    "UniMAC Inter Packet Gap",
    "",
#endif
    UNIMAC_RDP_TX_IPG_LEN_REG_OFFSET,
    0,
    0,
    855,
};

/******************************************************************************
 * Register: UNIMAC_RDP_EEE_CTRL
 ******************************************************************************/
const ru_reg_rec UNIMAC_RDP_EEE_CTRL_REG = 
{
    "EEE_CTRL",
#if RU_INCLUDE_DESC
    "UniMAC EEE Control",
    "",
#endif
    UNIMAC_RDP_EEE_CTRL_REG_OFFSET,
    0,
    0,
    856,
};

/******************************************************************************
 * Register: UNIMAC_RDP_EEE_LPI_TIMER
 ******************************************************************************/
const ru_reg_rec UNIMAC_RDP_EEE_LPI_TIMER_REG = 
{
    "EEE_LPI_TIMER",
#if RU_INCLUDE_DESC
    "UniMAC EEE  LPI Timer",
    "",
#endif
    UNIMAC_RDP_EEE_LPI_TIMER_REG_OFFSET,
    0,
    0,
    857,
};

/******************************************************************************
 * Register: UNIMAC_RDP_EEE_WAKE_TIMER
 ******************************************************************************/
const ru_reg_rec UNIMAC_RDP_EEE_WAKE_TIMER_REG = 
{
    "EEE_WAKE_TIMER",
#if RU_INCLUDE_DESC
    "UniMAC EEE  Wake Timer",
    "",
#endif
    UNIMAC_RDP_EEE_WAKE_TIMER_REG_OFFSET,
    0,
    0,
    858,
};

/******************************************************************************
 * Register: UNIMAC_RDP_EEE_REF_COUNT
 ******************************************************************************/
const ru_reg_rec UNIMAC_RDP_EEE_REF_COUNT_REG = 
{
    "EEE_REF_COUNT",
#if RU_INCLUDE_DESC
    "UniMAC EEE Reference Count",
    "",
#endif
    UNIMAC_RDP_EEE_REF_COUNT_REG_OFFSET,
    0,
    0,
    859,
};

/******************************************************************************
 * Register: UNIMAC_RDP_RX_PKT_DROP_STATUS
 ******************************************************************************/
const ru_reg_rec UNIMAC_RDP_RX_PKT_DROP_STATUS_REG = 
{
    "RX_PKT_DROP_STATUS",
#if RU_INCLUDE_DESC
    "UniMAC Rx Pkt Drop Status",
    "",
#endif
    UNIMAC_RDP_RX_PKT_DROP_STATUS_REG_OFFSET,
    0,
    0,
    860,
};

/******************************************************************************
 * Register: UNIMAC_RDP_SYMMETRIC_IDLE_THRESHOLD
 ******************************************************************************/
const ru_reg_rec UNIMAC_RDP_SYMMETRIC_IDLE_THRESHOLD_REG = 
{
    "SYMMETRIC_IDLE_THRESHOLD",
#if RU_INCLUDE_DESC
    "UniMAC Symmetric Idle Threshold",
    "",
#endif
    UNIMAC_RDP_SYMMETRIC_IDLE_THRESHOLD_REG_OFFSET,
    0,
    0,
    861,
};

/******************************************************************************
 * Register: UNIMAC_RDP_MACSEC_PROG_TX_CRC
 ******************************************************************************/
const ru_reg_rec UNIMAC_RDP_MACSEC_PROG_TX_CRC_REG = 
{
    "MACSEC_PROG_TX_CRC",
#if RU_INCLUDE_DESC
    "UniMAC Programmable CRC value",
    "",
#endif
    UNIMAC_RDP_MACSEC_PROG_TX_CRC_REG_OFFSET,
    0,
    0,
    862,
};

/******************************************************************************
 * Register: UNIMAC_RDP_MACSEC_CNTRL
 ******************************************************************************/
const ru_reg_rec UNIMAC_RDP_MACSEC_CNTRL_REG = 
{
    "MACSEC_CNTRL",
#if RU_INCLUDE_DESC
    "UniMAC Misc. MACSEC Control Register",
    "",
#endif
    UNIMAC_RDP_MACSEC_CNTRL_REG_OFFSET,
    0,
    0,
    863,
};

/******************************************************************************
 * Register: UNIMAC_RDP_TS_STATUS_CNTRL
 ******************************************************************************/
const ru_reg_rec UNIMAC_RDP_TS_STATUS_CNTRL_REG = 
{
    "TS_STATUS_CNTRL",
#if RU_INCLUDE_DESC
    "UniMAC Timestamp Status",
    "",
#endif
    UNIMAC_RDP_TS_STATUS_CNTRL_REG_OFFSET,
    0,
    0,
    864,
};

/******************************************************************************
 * Register: UNIMAC_RDP_TX_TS_DATA
 ******************************************************************************/
const ru_reg_rec UNIMAC_RDP_TX_TS_DATA_REG = 
{
    "TX_TS_DATA",
#if RU_INCLUDE_DESC
    "UniMAC Timestamp Data",
    "",
#endif
    UNIMAC_RDP_TX_TS_DATA_REG_OFFSET,
    0,
    0,
    865,
};

/******************************************************************************
 * Register: UNIMAC_RDP_PAUSE_CNTRL
 ******************************************************************************/
const ru_reg_rec UNIMAC_RDP_PAUSE_CNTRL_REG = 
{
    "PAUSE_CNTRL",
#if RU_INCLUDE_DESC
    "UniMAC Repetitive Pause Control in TX direction",
    "",
#endif
    UNIMAC_RDP_PAUSE_CNTRL_REG_OFFSET,
    0,
    0,
    866,
};

/******************************************************************************
 * Register: UNIMAC_RDP_TXFIFO_FLUSH
 ******************************************************************************/
const ru_reg_rec UNIMAC_RDP_TXFIFO_FLUSH_REG = 
{
    "TXFIFO_FLUSH",
#if RU_INCLUDE_DESC
    "UniMAC TX Flush",
    "",
#endif
    UNIMAC_RDP_TXFIFO_FLUSH_REG_OFFSET,
    0,
    0,
    867,
};

/******************************************************************************
 * Register: UNIMAC_RDP_RXFIFO_STAT
 ******************************************************************************/
const ru_reg_rec UNIMAC_RDP_RXFIFO_STAT_REG = 
{
    "RXFIFO_STAT",
#if RU_INCLUDE_DESC
    "UniMAC RX FIFO Status",
    "",
#endif
    UNIMAC_RDP_RXFIFO_STAT_REG_OFFSET,
    0,
    0,
    868,
};

/******************************************************************************
 * Register: UNIMAC_RDP_TXFIFO_STAT
 ******************************************************************************/
const ru_reg_rec UNIMAC_RDP_TXFIFO_STAT_REG = 
{
    "TXFIFO_STAT",
#if RU_INCLUDE_DESC
    "UniMAC TX FIFO Status",
    "",
#endif
    UNIMAC_RDP_TXFIFO_STAT_REG_OFFSET,
    0,
    0,
    869,
};

/******************************************************************************
 * Register: UNIMAC_RDP_PPP_CNTRL
 ******************************************************************************/
const ru_reg_rec UNIMAC_RDP_PPP_CNTRL_REG = 
{
    "PPP_CNTRL",
#if RU_INCLUDE_DESC
    "UniMAC PPP Control",
    "",
#endif
    UNIMAC_RDP_PPP_CNTRL_REG_OFFSET,
    0,
    0,
    870,
};

/******************************************************************************
 * Register: UNIMAC_RDP_PPP_REFRESH_CNTRL
 ******************************************************************************/
const ru_reg_rec UNIMAC_RDP_PPP_REFRESH_CNTRL_REG = 
{
    "PPP_REFRESH_CNTRL",
#if RU_INCLUDE_DESC
    "UniMAC Refresh Control",
    "",
#endif
    UNIMAC_RDP_PPP_REFRESH_CNTRL_REG_OFFSET,
    0,
    0,
    871,
};

/******************************************************************************
 * Register: UNIMAC_RDP_TX_PAUSE_PREL0
 ******************************************************************************/
const ru_reg_rec UNIMAC_RDP_TX_PAUSE_PREL0_REG = 
{
    "TX_PAUSE_PREL0",
#if RU_INCLUDE_DESC
    "UniMAC TX Pause PRBL[31:0]",
    "",
#endif
    UNIMAC_RDP_TX_PAUSE_PREL0_REG_OFFSET,
    0,
    0,
    872,
};

/******************************************************************************
 * Register: UNIMAC_RDP_TX_PAUSE_PREL1
 ******************************************************************************/
const ru_reg_rec UNIMAC_RDP_TX_PAUSE_PREL1_REG = 
{
    "TX_PAUSE_PREL1",
#if RU_INCLUDE_DESC
    "UniMAC TX Pause PRBL[63:32]",
    "",
#endif
    UNIMAC_RDP_TX_PAUSE_PREL1_REG_OFFSET,
    0,
    0,
    873,
};

/******************************************************************************
 * Register: UNIMAC_RDP_TX_PAUSE_PREL2
 ******************************************************************************/
const ru_reg_rec UNIMAC_RDP_TX_PAUSE_PREL2_REG = 
{
    "TX_PAUSE_PREL2",
#if RU_INCLUDE_DESC
    "UniMAC TX Pause PRBL[95:64]",
    "",
#endif
    UNIMAC_RDP_TX_PAUSE_PREL2_REG_OFFSET,
    0,
    0,
    874,
};

/******************************************************************************
 * Register: UNIMAC_RDP_TX_PAUSE_PREL3
 ******************************************************************************/
const ru_reg_rec UNIMAC_RDP_TX_PAUSE_PREL3_REG = 
{
    "TX_PAUSE_PREL3",
#if RU_INCLUDE_DESC
    "UniMAC TX Pause PRBL[127:96]",
    "",
#endif
    UNIMAC_RDP_TX_PAUSE_PREL3_REG_OFFSET,
    0,
    0,
    875,
};

/******************************************************************************
 * Register: UNIMAC_RDP_RX_PAUSE_PREL0
 ******************************************************************************/
const ru_reg_rec UNIMAC_RDP_RX_PAUSE_PREL0_REG = 
{
    "RX_PAUSE_PREL0",
#if RU_INCLUDE_DESC
    "UniMAC RX Pause PRBL[31:0]",
    "",
#endif
    UNIMAC_RDP_RX_PAUSE_PREL0_REG_OFFSET,
    0,
    0,
    876,
};

/******************************************************************************
 * Register: UNIMAC_RDP_RX_PAUSE_PREL1
 ******************************************************************************/
const ru_reg_rec UNIMAC_RDP_RX_PAUSE_PREL1_REG = 
{
    "RX_PAUSE_PREL1",
#if RU_INCLUDE_DESC
    "UniMAC RX Pause PRBL[63:32]",
    "",
#endif
    UNIMAC_RDP_RX_PAUSE_PREL1_REG_OFFSET,
    0,
    0,
    877,
};

/******************************************************************************
 * Register: UNIMAC_RDP_RX_PAUSE_PREL2
 ******************************************************************************/
const ru_reg_rec UNIMAC_RDP_RX_PAUSE_PREL2_REG = 
{
    "RX_PAUSE_PREL2",
#if RU_INCLUDE_DESC
    "UniMAC RX Pause PRBL[95:64]",
    "",
#endif
    UNIMAC_RDP_RX_PAUSE_PREL2_REG_OFFSET,
    0,
    0,
    878,
};

/******************************************************************************
 * Register: UNIMAC_RDP_RX_PAUSE_PREL3
 ******************************************************************************/
const ru_reg_rec UNIMAC_RDP_RX_PAUSE_PREL3_REG = 
{
    "RX_PAUSE_PREL3",
#if RU_INCLUDE_DESC
    "UniMAC RX Pause PRBL[127:96]",
    "",
#endif
    UNIMAC_RDP_RX_PAUSE_PREL3_REG_OFFSET,
    0,
    0,
    879,
};

/******************************************************************************
 * Register: UNIMAC_RDP_RXERR_MASK
 ******************************************************************************/
const ru_reg_rec UNIMAC_RDP_RXERR_MASK_REG = 
{
    "RXERR_MASK",
#if RU_INCLUDE_DESC
    "UniMAC RXERR Mask Register",
    "",
#endif
    UNIMAC_RDP_RXERR_MASK_REG_OFFSET,
    0,
    0,
    880,
};

/******************************************************************************
 * Register: UNIMAC_RDP_RX_MAX_PKT_SIZE
 ******************************************************************************/
const ru_reg_rec UNIMAC_RDP_RX_MAX_PKT_SIZE_REG = 
{
    "RX_MAX_PKT_SIZE",
#if RU_INCLUDE_DESC
    "UniMAC RX MAX packet Size Register",
    "",
#endif
    UNIMAC_RDP_RX_MAX_PKT_SIZE_REG_OFFSET,
    0,
    0,
    881,
};

/******************************************************************************
 * Register: UNIMAC_RDP_MDIO_CMD
 ******************************************************************************/
const ru_reg_rec UNIMAC_RDP_MDIO_CMD_REG = 
{
    "MDIO_CMD",
#if RU_INCLUDE_DESC
    "MDIO Command Register",
    "",
#endif
    UNIMAC_RDP_MDIO_CMD_REG_OFFSET,
    0,
    0,
    882,
};

/******************************************************************************
 * Register: UNIMAC_RDP_MDIO_CFG
 ******************************************************************************/
const ru_reg_rec UNIMAC_RDP_MDIO_CFG_REG = 
{
    "MDIO_CFG",
#if RU_INCLUDE_DESC
    "MDIO Configuration Register",
    "",
#endif
    UNIMAC_RDP_MDIO_CFG_REG_OFFSET,
    0,
    0,
    883,
};

/******************************************************************************
 * Register: UNIMAC_RDP_MDF_CNT
 ******************************************************************************/
const ru_reg_rec UNIMAC_RDP_MDF_CNT_REG = 
{
    "MDF_CNT",
#if RU_INCLUDE_DESC
    "MDF Discard Packet Counter",
    "",
#endif
    UNIMAC_RDP_MDF_CNT_REG_OFFSET,
    0,
    0,
    884,
};

/******************************************************************************
 * Register: UNIMAC_RDP_DIAG_SEL
 ******************************************************************************/
const ru_reg_rec UNIMAC_RDP_DIAG_SEL_REG = 
{
    "DIAG_SEL",
#if RU_INCLUDE_DESC
    "Diag Select Register",
    "",
#endif
    UNIMAC_RDP_DIAG_SEL_REG_OFFSET,
    0,
    0,
    885,
};

/******************************************************************************
 * Block: UNIMAC_RDP
 ******************************************************************************/
static const ru_reg_rec *UNIMAC_RDP_REGS[] =
{
    &UNIMAC_RDP_UMAC_DUMMY_REG,
    &UNIMAC_RDP_HD_BKP_CNTL_REG,
    &UNIMAC_RDP_CMD_REG,
    &UNIMAC_RDP_MAC0_REG,
    &UNIMAC_RDP_MAC1_REG,
    &UNIMAC_RDP_FRM_LEN_REG,
    &UNIMAC_RDP_PAUSE_QUNAT_REG,
    &UNIMAC_RDP_SFD_OFFSET_REG,
    &UNIMAC_RDP_MODE_REG,
    &UNIMAC_RDP_FRM_TAG0_REG,
    &UNIMAC_RDP_FRM_TAG1_REG,
    &UNIMAC_RDP_TX_IPG_LEN_REG,
    &UNIMAC_RDP_EEE_CTRL_REG,
    &UNIMAC_RDP_EEE_LPI_TIMER_REG,
    &UNIMAC_RDP_EEE_WAKE_TIMER_REG,
    &UNIMAC_RDP_EEE_REF_COUNT_REG,
    &UNIMAC_RDP_RX_PKT_DROP_STATUS_REG,
    &UNIMAC_RDP_SYMMETRIC_IDLE_THRESHOLD_REG,
    &UNIMAC_RDP_MACSEC_PROG_TX_CRC_REG,
    &UNIMAC_RDP_MACSEC_CNTRL_REG,
    &UNIMAC_RDP_TS_STATUS_CNTRL_REG,
    &UNIMAC_RDP_TX_TS_DATA_REG,
    &UNIMAC_RDP_PAUSE_CNTRL_REG,
    &UNIMAC_RDP_TXFIFO_FLUSH_REG,
    &UNIMAC_RDP_RXFIFO_STAT_REG,
    &UNIMAC_RDP_TXFIFO_STAT_REG,
    &UNIMAC_RDP_PPP_CNTRL_REG,
    &UNIMAC_RDP_PPP_REFRESH_CNTRL_REG,
    &UNIMAC_RDP_TX_PAUSE_PREL0_REG,
    &UNIMAC_RDP_TX_PAUSE_PREL1_REG,
    &UNIMAC_RDP_TX_PAUSE_PREL2_REG,
    &UNIMAC_RDP_TX_PAUSE_PREL3_REG,
    &UNIMAC_RDP_RX_PAUSE_PREL0_REG,
    &UNIMAC_RDP_RX_PAUSE_PREL1_REG,
    &UNIMAC_RDP_RX_PAUSE_PREL2_REG,
    &UNIMAC_RDP_RX_PAUSE_PREL3_REG,
    &UNIMAC_RDP_RXERR_MASK_REG,
    &UNIMAC_RDP_RX_MAX_PKT_SIZE_REG,
    &UNIMAC_RDP_MDIO_CMD_REG,
    &UNIMAC_RDP_MDIO_CFG_REG,
    &UNIMAC_RDP_MDF_CNT_REG,
    &UNIMAC_RDP_DIAG_SEL_REG,
};

unsigned long UNIMAC_RDP_ADDRS[] =
{
    0x82da0000,
    0x82da1000,
    0x82da2000,
    0x82da3000,
    0x82da4000,
    0x82da5000,
};

const ru_block_rec UNIMAC_RDP_BLOCK = 
{
    "UNIMAC_RDP",
    UNIMAC_RDP_ADDRS,
    6,
    42,
    UNIMAC_RDP_REGS
};

/* End of file XRDP_UNIMAC_RDP.c */
