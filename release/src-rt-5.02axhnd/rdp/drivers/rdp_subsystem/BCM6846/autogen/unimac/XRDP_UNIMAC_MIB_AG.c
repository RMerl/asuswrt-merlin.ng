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
 * Field: UNIMAC_MIB_MIB_CNTRL_RESERVED0
 ******************************************************************************/
const ru_field_rec UNIMAC_MIB_MIB_CNTRL_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    UNIMAC_MIB_MIB_CNTRL_RESERVED0_FIELD_MASK,
    0,
    UNIMAC_MIB_MIB_CNTRL_RESERVED0_FIELD_WIDTH,
    UNIMAC_MIB_MIB_CNTRL_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UNIMAC_MIB_MIB_CNTRL_TX_CNT_RST
 ******************************************************************************/
const ru_field_rec UNIMAC_MIB_MIB_CNTRL_TX_CNT_RST_FIELD =
{
    "TX_CNT_RST",
#if RU_INCLUDE_DESC
    "",
    "High active; When this bit is set, it resets TX statistics counters. ",
#endif
    UNIMAC_MIB_MIB_CNTRL_TX_CNT_RST_FIELD_MASK,
    0,
    UNIMAC_MIB_MIB_CNTRL_TX_CNT_RST_FIELD_WIDTH,
    UNIMAC_MIB_MIB_CNTRL_TX_CNT_RST_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UNIMAC_MIB_MIB_CNTRL_RUNT_CNT_RST
 ******************************************************************************/
const ru_field_rec UNIMAC_MIB_MIB_CNTRL_RUNT_CNT_RST_FIELD =
{
    "RUNT_CNT_RST",
#if RU_INCLUDE_DESC
    "",
    "High active; When this bit is set it resets Runt statistics counters. ",
#endif
    UNIMAC_MIB_MIB_CNTRL_RUNT_CNT_RST_FIELD_MASK,
    0,
    UNIMAC_MIB_MIB_CNTRL_RUNT_CNT_RST_FIELD_WIDTH,
    UNIMAC_MIB_MIB_CNTRL_RUNT_CNT_RST_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UNIMAC_MIB_MIB_CNTRL_RX_CNT_ST
 ******************************************************************************/
const ru_field_rec UNIMAC_MIB_MIB_CNTRL_RX_CNT_ST_FIELD =
{
    "RX_CNT_ST",
#if RU_INCLUDE_DESC
    "",
    "High active; When this bit is set it resets RX statistics counters. ",
#endif
    UNIMAC_MIB_MIB_CNTRL_RX_CNT_ST_FIELD_MASK,
    0,
    UNIMAC_MIB_MIB_CNTRL_RX_CNT_ST_FIELD_WIDTH,
    UNIMAC_MIB_MIB_CNTRL_RX_CNT_ST_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UNIMAC_MIB_GR64_GR
 ******************************************************************************/
const ru_field_rec UNIMAC_MIB_GR64_GR_FIELD =
{
    "GR",
#if RU_INCLUDE_DESC
    "",
    "Receive 64 Bytes frame counter. ",
#endif
    UNIMAC_MIB_GR64_GR_FIELD_MASK,
    0,
    UNIMAC_MIB_GR64_GR_FIELD_WIDTH,
    UNIMAC_MIB_GR64_GR_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UNIMAC_MIB_GR127_GR
 ******************************************************************************/
const ru_field_rec UNIMAC_MIB_GR127_GR_FIELD =
{
    "GR",
#if RU_INCLUDE_DESC
    "",
    "Receive 65 bytes to 127 bytes frame counter. ",
#endif
    UNIMAC_MIB_GR127_GR_FIELD_MASK,
    0,
    UNIMAC_MIB_GR127_GR_FIELD_WIDTH,
    UNIMAC_MIB_GR127_GR_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UNIMAC_MIB_GR255_GR
 ******************************************************************************/
const ru_field_rec UNIMAC_MIB_GR255_GR_FIELD =
{
    "GR",
#if RU_INCLUDE_DESC
    "",
    "Receive 128 bytes to 255 bytes frame counter. ",
#endif
    UNIMAC_MIB_GR255_GR_FIELD_MASK,
    0,
    UNIMAC_MIB_GR255_GR_FIELD_WIDTH,
    UNIMAC_MIB_GR255_GR_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UNIMAC_MIB_GR511_GR
 ******************************************************************************/
const ru_field_rec UNIMAC_MIB_GR511_GR_FIELD =
{
    "GR",
#if RU_INCLUDE_DESC
    "",
    "Receive 256 bytes to 511 bytes frame counter. ",
#endif
    UNIMAC_MIB_GR511_GR_FIELD_MASK,
    0,
    UNIMAC_MIB_GR511_GR_FIELD_WIDTH,
    UNIMAC_MIB_GR511_GR_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UNIMAC_MIB_GR1023_GR
 ******************************************************************************/
const ru_field_rec UNIMAC_MIB_GR1023_GR_FIELD =
{
    "GR",
#if RU_INCLUDE_DESC
    "",
    "Receive 512 bytes to 1023 bytes frame counter. ",
#endif
    UNIMAC_MIB_GR1023_GR_FIELD_MASK,
    0,
    UNIMAC_MIB_GR1023_GR_FIELD_WIDTH,
    UNIMAC_MIB_GR1023_GR_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UNIMAC_MIB_GR1518_GR
 ******************************************************************************/
const ru_field_rec UNIMAC_MIB_GR1518_GR_FIELD =
{
    "GR",
#if RU_INCLUDE_DESC
    "",
    "Receive 1024 bytes to 1518 bytes frame counter. ",
#endif
    UNIMAC_MIB_GR1518_GR_FIELD_MASK,
    0,
    UNIMAC_MIB_GR1518_GR_FIELD_WIDTH,
    UNIMAC_MIB_GR1518_GR_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UNIMAC_MIB_GRMGV_GR
 ******************************************************************************/
const ru_field_rec UNIMAC_MIB_GRMGV_GR_FIELD =
{
    "GR",
#if RU_INCLUDE_DESC
    "",
    "Receive 1519 bytes to 1522 bytes good VLAN frame counter. ",
#endif
    UNIMAC_MIB_GRMGV_GR_FIELD_MASK,
    0,
    UNIMAC_MIB_GRMGV_GR_FIELD_WIDTH,
    UNIMAC_MIB_GRMGV_GR_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UNIMAC_MIB_GR2047_GR
 ******************************************************************************/
const ru_field_rec UNIMAC_MIB_GR2047_GR_FIELD =
{
    "GR",
#if RU_INCLUDE_DESC
    "",
    "Receive 1519 bytes to 2047 bytes frame counter. ",
#endif
    UNIMAC_MIB_GR2047_GR_FIELD_MASK,
    0,
    UNIMAC_MIB_GR2047_GR_FIELD_WIDTH,
    UNIMAC_MIB_GR2047_GR_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UNIMAC_MIB_GR4095_GR
 ******************************************************************************/
const ru_field_rec UNIMAC_MIB_GR4095_GR_FIELD =
{
    "GR",
#if RU_INCLUDE_DESC
    "",
    "Receive 2048 bytes to 4096 bytes frame counter. ",
#endif
    UNIMAC_MIB_GR4095_GR_FIELD_MASK,
    0,
    UNIMAC_MIB_GR4095_GR_FIELD_WIDTH,
    UNIMAC_MIB_GR4095_GR_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UNIMAC_MIB_GR9216_GR
 ******************************************************************************/
const ru_field_rec UNIMAC_MIB_GR9216_GR_FIELD =
{
    "GR",
#if RU_INCLUDE_DESC
    "",
    "Receive 4096 bytes to 9216 bytes frame counter. ",
#endif
    UNIMAC_MIB_GR9216_GR_FIELD_MASK,
    0,
    UNIMAC_MIB_GR9216_GR_FIELD_WIDTH,
    UNIMAC_MIB_GR9216_GR_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UNIMAC_MIB_GRPKT_GR
 ******************************************************************************/
const ru_field_rec UNIMAC_MIB_GRPKT_GR_FIELD =
{
    "GR",
#if RU_INCLUDE_DESC
    "",
    "Receive packet counter. ",
#endif
    UNIMAC_MIB_GRPKT_GR_FIELD_MASK,
    0,
    UNIMAC_MIB_GRPKT_GR_FIELD_WIDTH,
    UNIMAC_MIB_GRPKT_GR_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UNIMAC_MIB_GRBYT_GR
 ******************************************************************************/
const ru_field_rec UNIMAC_MIB_GRBYT_GR_FIELD =
{
    "GR",
#if RU_INCLUDE_DESC
    "",
    "Receive byte counter. ",
#endif
    UNIMAC_MIB_GRBYT_GR_FIELD_MASK,
    0,
    UNIMAC_MIB_GRBYT_GR_FIELD_WIDTH,
    UNIMAC_MIB_GRBYT_GR_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UNIMAC_MIB_GRMCA_GR
 ******************************************************************************/
const ru_field_rec UNIMAC_MIB_GRMCA_GR_FIELD =
{
    "GR",
#if RU_INCLUDE_DESC
    "",
    "Receive multicast packet counter. ",
#endif
    UNIMAC_MIB_GRMCA_GR_FIELD_MASK,
    0,
    UNIMAC_MIB_GRMCA_GR_FIELD_WIDTH,
    UNIMAC_MIB_GRMCA_GR_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UNIMAC_MIB_GRBCA_GR
 ******************************************************************************/
const ru_field_rec UNIMAC_MIB_GRBCA_GR_FIELD =
{
    "GR",
#if RU_INCLUDE_DESC
    "",
    "Receive broadcast packet counter. ",
#endif
    UNIMAC_MIB_GRBCA_GR_FIELD_MASK,
    0,
    UNIMAC_MIB_GRBCA_GR_FIELD_WIDTH,
    UNIMAC_MIB_GRBCA_GR_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UNIMAC_MIB_GRFCS_GR
 ******************************************************************************/
const ru_field_rec UNIMAC_MIB_GRFCS_GR_FIELD =
{
    "GR",
#if RU_INCLUDE_DESC
    "",
    "Receive FCS error counter. ",
#endif
    UNIMAC_MIB_GRFCS_GR_FIELD_MASK,
    0,
    UNIMAC_MIB_GRFCS_GR_FIELD_WIDTH,
    UNIMAC_MIB_GRFCS_GR_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UNIMAC_MIB_GRXCF_GR
 ******************************************************************************/
const ru_field_rec UNIMAC_MIB_GRXCF_GR_FIELD =
{
    "GR",
#if RU_INCLUDE_DESC
    "",
    "Receive control frame packet counter. ",
#endif
    UNIMAC_MIB_GRXCF_GR_FIELD_MASK,
    0,
    UNIMAC_MIB_GRXCF_GR_FIELD_WIDTH,
    UNIMAC_MIB_GRXCF_GR_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UNIMAC_MIB_GRXPF_GR
 ******************************************************************************/
const ru_field_rec UNIMAC_MIB_GRXPF_GR_FIELD =
{
    "GR",
#if RU_INCLUDE_DESC
    "",
    "Receive pause frame packet counter. ",
#endif
    UNIMAC_MIB_GRXPF_GR_FIELD_MASK,
    0,
    UNIMAC_MIB_GRXPF_GR_FIELD_WIDTH,
    UNIMAC_MIB_GRXPF_GR_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UNIMAC_MIB_GRXUO_GR
 ******************************************************************************/
const ru_field_rec UNIMAC_MIB_GRXUO_GR_FIELD =
{
    "GR",
#if RU_INCLUDE_DESC
    "",
    "Receive unknown op code packet counter. ",
#endif
    UNIMAC_MIB_GRXUO_GR_FIELD_MASK,
    0,
    UNIMAC_MIB_GRXUO_GR_FIELD_WIDTH,
    UNIMAC_MIB_GRXUO_GR_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UNIMAC_MIB_GRALN_GR
 ******************************************************************************/
const ru_field_rec UNIMAC_MIB_GRALN_GR_FIELD =
{
    "GR",
#if RU_INCLUDE_DESC
    "",
    "Receive alignmenet error counter. ",
#endif
    UNIMAC_MIB_GRALN_GR_FIELD_MASK,
    0,
    UNIMAC_MIB_GRALN_GR_FIELD_WIDTH,
    UNIMAC_MIB_GRALN_GR_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UNIMAC_MIB_GRFLR_GR
 ******************************************************************************/
const ru_field_rec UNIMAC_MIB_GRFLR_GR_FIELD =
{
    "GR",
#if RU_INCLUDE_DESC
    "",
    "Receive frame length out of range counter. ",
#endif
    UNIMAC_MIB_GRFLR_GR_FIELD_MASK,
    0,
    UNIMAC_MIB_GRFLR_GR_FIELD_WIDTH,
    UNIMAC_MIB_GRFLR_GR_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UNIMAC_MIB_GRCDE_GR
 ******************************************************************************/
const ru_field_rec UNIMAC_MIB_GRCDE_GR_FIELD =
{
    "GR",
#if RU_INCLUDE_DESC
    "",
    "Receive code error packet counter. ",
#endif
    UNIMAC_MIB_GRCDE_GR_FIELD_MASK,
    0,
    UNIMAC_MIB_GRCDE_GR_FIELD_WIDTH,
    UNIMAC_MIB_GRCDE_GR_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UNIMAC_MIB_GRFCR_GR
 ******************************************************************************/
const ru_field_rec UNIMAC_MIB_GRFCR_GR_FIELD =
{
    "GR",
#if RU_INCLUDE_DESC
    "",
    "Receive carrier sense error packet counter. ",
#endif
    UNIMAC_MIB_GRFCR_GR_FIELD_MASK,
    0,
    UNIMAC_MIB_GRFCR_GR_FIELD_WIDTH,
    UNIMAC_MIB_GRFCR_GR_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UNIMAC_MIB_GROVR_GR
 ******************************************************************************/
const ru_field_rec UNIMAC_MIB_GROVR_GR_FIELD =
{
    "GR",
#if RU_INCLUDE_DESC
    "",
    "Receive oversize packet counter. ",
#endif
    UNIMAC_MIB_GROVR_GR_FIELD_MASK,
    0,
    UNIMAC_MIB_GROVR_GR_FIELD_WIDTH,
    UNIMAC_MIB_GROVR_GR_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UNIMAC_MIB_GRJBR_GR
 ******************************************************************************/
const ru_field_rec UNIMAC_MIB_GRJBR_GR_FIELD =
{
    "GR",
#if RU_INCLUDE_DESC
    "",
    "Receive jabber counter. ",
#endif
    UNIMAC_MIB_GRJBR_GR_FIELD_MASK,
    0,
    UNIMAC_MIB_GRJBR_GR_FIELD_WIDTH,
    UNIMAC_MIB_GRJBR_GR_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UNIMAC_MIB_GRMTUE_GR
 ******************************************************************************/
const ru_field_rec UNIMAC_MIB_GRMTUE_GR_FIELD =
{
    "GR",
#if RU_INCLUDE_DESC
    "",
    "Receive MTU error packet counter. ",
#endif
    UNIMAC_MIB_GRMTUE_GR_FIELD_MASK,
    0,
    UNIMAC_MIB_GRMTUE_GR_FIELD_WIDTH,
    UNIMAC_MIB_GRMTUE_GR_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UNIMAC_MIB_GRPOK_GR
 ******************************************************************************/
const ru_field_rec UNIMAC_MIB_GRPOK_GR_FIELD =
{
    "GR",
#if RU_INCLUDE_DESC
    "",
    "Receive good packet counter. ",
#endif
    UNIMAC_MIB_GRPOK_GR_FIELD_MASK,
    0,
    UNIMAC_MIB_GRPOK_GR_FIELD_WIDTH,
    UNIMAC_MIB_GRPOK_GR_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UNIMAC_MIB_GRUC_GR
 ******************************************************************************/
const ru_field_rec UNIMAC_MIB_GRUC_GR_FIELD =
{
    "GR",
#if RU_INCLUDE_DESC
    "",
    "Received unicast packet counter. ",
#endif
    UNIMAC_MIB_GRUC_GR_FIELD_MASK,
    0,
    UNIMAC_MIB_GRUC_GR_FIELD_WIDTH,
    UNIMAC_MIB_GRUC_GR_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UNIMAC_MIB_GRPPP_GR
 ******************************************************************************/
const ru_field_rec UNIMAC_MIB_GRPPP_GR_FIELD =
{
    "GR",
#if RU_INCLUDE_DESC
    "",
    "Receive PPP packet counter. ",
#endif
    UNIMAC_MIB_GRPPP_GR_FIELD_MASK,
    0,
    UNIMAC_MIB_GRPPP_GR_FIELD_WIDTH,
    UNIMAC_MIB_GRPPP_GR_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UNIMAC_MIB_GRCRC_GR
 ******************************************************************************/
const ru_field_rec UNIMAC_MIB_GRCRC_GR_FIELD =
{
    "GR",
#if RU_INCLUDE_DESC
    "",
    "Receive CRC match packet counter. ",
#endif
    UNIMAC_MIB_GRCRC_GR_FIELD_MASK,
    0,
    UNIMAC_MIB_GRCRC_GR_FIELD_WIDTH,
    UNIMAC_MIB_GRCRC_GR_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UNIMAC_MIB_RRPKT_RR
 ******************************************************************************/
const ru_field_rec UNIMAC_MIB_RRPKT_RR_FIELD =
{
    "RR",
#if RU_INCLUDE_DESC
    "",
    "Receive RUNT packet counter. ",
#endif
    UNIMAC_MIB_RRPKT_RR_FIELD_MASK,
    0,
    UNIMAC_MIB_RRPKT_RR_FIELD_WIDTH,
    UNIMAC_MIB_RRPKT_RR_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UNIMAC_MIB_RRUND_RR
 ******************************************************************************/
const ru_field_rec UNIMAC_MIB_RRUND_RR_FIELD =
{
    "RR",
#if RU_INCLUDE_DESC
    "",
    "Receive RUNT packet and contain a valid FCS. ",
#endif
    UNIMAC_MIB_RRUND_RR_FIELD_MASK,
    0,
    UNIMAC_MIB_RRUND_RR_FIELD_WIDTH,
    UNIMAC_MIB_RRUND_RR_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UNIMAC_MIB_RRFRG_RR
 ******************************************************************************/
const ru_field_rec UNIMAC_MIB_RRFRG_RR_FIELD =
{
    "RR",
#if RU_INCLUDE_DESC
    "",
    "Receive RUNT packet and contain invalid FCS or alignment error. ",
#endif
    UNIMAC_MIB_RRFRG_RR_FIELD_MASK,
    0,
    UNIMAC_MIB_RRFRG_RR_FIELD_WIDTH,
    UNIMAC_MIB_RRFRG_RR_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UNIMAC_MIB_RRBYT_RR
 ******************************************************************************/
const ru_field_rec UNIMAC_MIB_RRBYT_RR_FIELD =
{
    "RR",
#if RU_INCLUDE_DESC
    "",
    "Receive RUNT packet byte counter. ",
#endif
    UNIMAC_MIB_RRBYT_RR_FIELD_MASK,
    0,
    UNIMAC_MIB_RRBYT_RR_FIELD_WIDTH,
    UNIMAC_MIB_RRBYT_RR_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UNIMAC_MIB_TR64_TR
 ******************************************************************************/
const ru_field_rec UNIMAC_MIB_TR64_TR_FIELD =
{
    "TR",
#if RU_INCLUDE_DESC
    "",
    "Transmit 64 bytes frame counter. ",
#endif
    UNIMAC_MIB_TR64_TR_FIELD_MASK,
    0,
    UNIMAC_MIB_TR64_TR_FIELD_WIDTH,
    UNIMAC_MIB_TR64_TR_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UNIMAC_MIB_TR127_TR
 ******************************************************************************/
const ru_field_rec UNIMAC_MIB_TR127_TR_FIELD =
{
    "TR",
#if RU_INCLUDE_DESC
    "",
    "Transmit 65 bytes to 127 bytes frame counter. ",
#endif
    UNIMAC_MIB_TR127_TR_FIELD_MASK,
    0,
    UNIMAC_MIB_TR127_TR_FIELD_WIDTH,
    UNIMAC_MIB_TR127_TR_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UNIMAC_MIB_TR255_TR
 ******************************************************************************/
const ru_field_rec UNIMAC_MIB_TR255_TR_FIELD =
{
    "TR",
#if RU_INCLUDE_DESC
    "",
    "Transmit 128 bytes to 255 bytes frame counter. ",
#endif
    UNIMAC_MIB_TR255_TR_FIELD_MASK,
    0,
    UNIMAC_MIB_TR255_TR_FIELD_WIDTH,
    UNIMAC_MIB_TR255_TR_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UNIMAC_MIB_TR511_TR
 ******************************************************************************/
const ru_field_rec UNIMAC_MIB_TR511_TR_FIELD =
{
    "TR",
#if RU_INCLUDE_DESC
    "",
    "Transmit 256 bytes to 511 bytes frame counter. ",
#endif
    UNIMAC_MIB_TR511_TR_FIELD_MASK,
    0,
    UNIMAC_MIB_TR511_TR_FIELD_WIDTH,
    UNIMAC_MIB_TR511_TR_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UNIMAC_MIB_TR1023_TR
 ******************************************************************************/
const ru_field_rec UNIMAC_MIB_TR1023_TR_FIELD =
{
    "TR",
#if RU_INCLUDE_DESC
    "",
    "Transmit 512 bytes to 1023 bytes frame counter. ",
#endif
    UNIMAC_MIB_TR1023_TR_FIELD_MASK,
    0,
    UNIMAC_MIB_TR1023_TR_FIELD_WIDTH,
    UNIMAC_MIB_TR1023_TR_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UNIMAC_MIB_TR1518_TR
 ******************************************************************************/
const ru_field_rec UNIMAC_MIB_TR1518_TR_FIELD =
{
    "TR",
#if RU_INCLUDE_DESC
    "",
    "Transmit 1024 bytes to 1518 bytes frame counter. ",
#endif
    UNIMAC_MIB_TR1518_TR_FIELD_MASK,
    0,
    UNIMAC_MIB_TR1518_TR_FIELD_WIDTH,
    UNIMAC_MIB_TR1518_TR_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UNIMAC_MIB_TRMGV_TR
 ******************************************************************************/
const ru_field_rec UNIMAC_MIB_TRMGV_TR_FIELD =
{
    "TR",
#if RU_INCLUDE_DESC
    "",
    "Transmit 1519 bytes to 1522 bytes good VLAN frame counter. ",
#endif
    UNIMAC_MIB_TRMGV_TR_FIELD_MASK,
    0,
    UNIMAC_MIB_TRMGV_TR_FIELD_WIDTH,
    UNIMAC_MIB_TRMGV_TR_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UNIMAC_MIB_TR2047_TR
 ******************************************************************************/
const ru_field_rec UNIMAC_MIB_TR2047_TR_FIELD =
{
    "TR",
#if RU_INCLUDE_DESC
    "",
    "Transmit 1519 bytes to 2047 bytes Frame Counter. ",
#endif
    UNIMAC_MIB_TR2047_TR_FIELD_MASK,
    0,
    UNIMAC_MIB_TR2047_TR_FIELD_WIDTH,
    UNIMAC_MIB_TR2047_TR_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UNIMAC_MIB_TR4095_TR
 ******************************************************************************/
const ru_field_rec UNIMAC_MIB_TR4095_TR_FIELD =
{
    "TR",
#if RU_INCLUDE_DESC
    "",
    "Transmit 2048 bytes to 4096 bytes frame counter. ",
#endif
    UNIMAC_MIB_TR4095_TR_FIELD_MASK,
    0,
    UNIMAC_MIB_TR4095_TR_FIELD_WIDTH,
    UNIMAC_MIB_TR4095_TR_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UNIMAC_MIB_TR9216_TR
 ******************************************************************************/
const ru_field_rec UNIMAC_MIB_TR9216_TR_FIELD =
{
    "TR",
#if RU_INCLUDE_DESC
    "",
    "Transmit 4096 bytes to 9216 bytes Frame Counter. ",
#endif
    UNIMAC_MIB_TR9216_TR_FIELD_MASK,
    0,
    UNIMAC_MIB_TR9216_TR_FIELD_WIDTH,
    UNIMAC_MIB_TR9216_TR_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UNIMAC_MIB_GTPKT_GT
 ******************************************************************************/
const ru_field_rec UNIMAC_MIB_GTPKT_GT_FIELD =
{
    "GT",
#if RU_INCLUDE_DESC
    "",
    "Transmit packet counter. ",
#endif
    UNIMAC_MIB_GTPKT_GT_FIELD_MASK,
    0,
    UNIMAC_MIB_GTPKT_GT_FIELD_WIDTH,
    UNIMAC_MIB_GTPKT_GT_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UNIMAC_MIB_GTMCA_GT
 ******************************************************************************/
const ru_field_rec UNIMAC_MIB_GTMCA_GT_FIELD =
{
    "GT",
#if RU_INCLUDE_DESC
    "",
    "Transmit multicast packet counter. ",
#endif
    UNIMAC_MIB_GTMCA_GT_FIELD_MASK,
    0,
    UNIMAC_MIB_GTMCA_GT_FIELD_WIDTH,
    UNIMAC_MIB_GTMCA_GT_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UNIMAC_MIB_GTBCA_GT
 ******************************************************************************/
const ru_field_rec UNIMAC_MIB_GTBCA_GT_FIELD =
{
    "GT",
#if RU_INCLUDE_DESC
    "",
    "Transmit broadcast packet counter. ",
#endif
    UNIMAC_MIB_GTBCA_GT_FIELD_MASK,
    0,
    UNIMAC_MIB_GTBCA_GT_FIELD_WIDTH,
    UNIMAC_MIB_GTBCA_GT_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UNIMAC_MIB_GTXPF_GT
 ******************************************************************************/
const ru_field_rec UNIMAC_MIB_GTXPF_GT_FIELD =
{
    "GT",
#if RU_INCLUDE_DESC
    "",
    "Transmit pause frame packet counter. ",
#endif
    UNIMAC_MIB_GTXPF_GT_FIELD_MASK,
    0,
    UNIMAC_MIB_GTXPF_GT_FIELD_WIDTH,
    UNIMAC_MIB_GTXPF_GT_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UNIMAC_MIB_GTXCF_GT
 ******************************************************************************/
const ru_field_rec UNIMAC_MIB_GTXCF_GT_FIELD =
{
    "GT",
#if RU_INCLUDE_DESC
    "",
    "Transmit control frame packet counter. ",
#endif
    UNIMAC_MIB_GTXCF_GT_FIELD_MASK,
    0,
    UNIMAC_MIB_GTXCF_GT_FIELD_WIDTH,
    UNIMAC_MIB_GTXCF_GT_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UNIMAC_MIB_GTFCS_GT
 ******************************************************************************/
const ru_field_rec UNIMAC_MIB_GTFCS_GT_FIELD =
{
    "GT",
#if RU_INCLUDE_DESC
    "",
    "Transmit FCS error counter. ",
#endif
    UNIMAC_MIB_GTFCS_GT_FIELD_MASK,
    0,
    UNIMAC_MIB_GTFCS_GT_FIELD_WIDTH,
    UNIMAC_MIB_GTFCS_GT_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UNIMAC_MIB_GTOVR_GT
 ******************************************************************************/
const ru_field_rec UNIMAC_MIB_GTOVR_GT_FIELD =
{
    "GT",
#if RU_INCLUDE_DESC
    "",
    "Transmit oversize packet counter. ",
#endif
    UNIMAC_MIB_GTOVR_GT_FIELD_MASK,
    0,
    UNIMAC_MIB_GTOVR_GT_FIELD_WIDTH,
    UNIMAC_MIB_GTOVR_GT_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UNIMAC_MIB_GTDRF_GT
 ******************************************************************************/
const ru_field_rec UNIMAC_MIB_GTDRF_GT_FIELD =
{
    "GT",
#if RU_INCLUDE_DESC
    "",
    "Transmit deferral packet counter. ",
#endif
    UNIMAC_MIB_GTDRF_GT_FIELD_MASK,
    0,
    UNIMAC_MIB_GTDRF_GT_FIELD_WIDTH,
    UNIMAC_MIB_GTDRF_GT_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UNIMAC_MIB_GTEDF_GT
 ******************************************************************************/
const ru_field_rec UNIMAC_MIB_GTEDF_GT_FIELD =
{
    "GT",
#if RU_INCLUDE_DESC
    "",
    "Transmit excessive deferral packet counter. ",
#endif
    UNIMAC_MIB_GTEDF_GT_FIELD_MASK,
    0,
    UNIMAC_MIB_GTEDF_GT_FIELD_WIDTH,
    UNIMAC_MIB_GTEDF_GT_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UNIMAC_MIB_GTSCL_GT
 ******************************************************************************/
const ru_field_rec UNIMAC_MIB_GTSCL_GT_FIELD =
{
    "GT",
#if RU_INCLUDE_DESC
    "",
    "Transmit single collision packet counter. ",
#endif
    UNIMAC_MIB_GTSCL_GT_FIELD_MASK,
    0,
    UNIMAC_MIB_GTSCL_GT_FIELD_WIDTH,
    UNIMAC_MIB_GTSCL_GT_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UNIMAC_MIB_GTMCL_GT
 ******************************************************************************/
const ru_field_rec UNIMAC_MIB_GTMCL_GT_FIELD =
{
    "GT",
#if RU_INCLUDE_DESC
    "",
    "Transmit multiple collision packet counter. ",
#endif
    UNIMAC_MIB_GTMCL_GT_FIELD_MASK,
    0,
    UNIMAC_MIB_GTMCL_GT_FIELD_WIDTH,
    UNIMAC_MIB_GTMCL_GT_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UNIMAC_MIB_GTLCL_GT
 ******************************************************************************/
const ru_field_rec UNIMAC_MIB_GTLCL_GT_FIELD =
{
    "GT",
#if RU_INCLUDE_DESC
    "",
    "Transmit late collision packet counter. ",
#endif
    UNIMAC_MIB_GTLCL_GT_FIELD_MASK,
    0,
    UNIMAC_MIB_GTLCL_GT_FIELD_WIDTH,
    UNIMAC_MIB_GTLCL_GT_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UNIMAC_MIB_GTXCL_GT
 ******************************************************************************/
const ru_field_rec UNIMAC_MIB_GTXCL_GT_FIELD =
{
    "GT",
#if RU_INCLUDE_DESC
    "",
    "Transmit excessive collision packet counter. ",
#endif
    UNIMAC_MIB_GTXCL_GT_FIELD_MASK,
    0,
    UNIMAC_MIB_GTXCL_GT_FIELD_WIDTH,
    UNIMAC_MIB_GTXCL_GT_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UNIMAC_MIB_GTFRG_GT
 ******************************************************************************/
const ru_field_rec UNIMAC_MIB_GTFRG_GT_FIELD =
{
    "GT",
#if RU_INCLUDE_DESC
    "",
    "Transmit fragments packet counter. ",
#endif
    UNIMAC_MIB_GTFRG_GT_FIELD_MASK,
    0,
    UNIMAC_MIB_GTFRG_GT_FIELD_WIDTH,
    UNIMAC_MIB_GTFRG_GT_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UNIMAC_MIB_GTNCL_GT
 ******************************************************************************/
const ru_field_rec UNIMAC_MIB_GTNCL_GT_FIELD =
{
    "GT",
#if RU_INCLUDE_DESC
    "",
    "Transmit total collision counter. ",
#endif
    UNIMAC_MIB_GTNCL_GT_FIELD_MASK,
    0,
    UNIMAC_MIB_GTNCL_GT_FIELD_WIDTH,
    UNIMAC_MIB_GTNCL_GT_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UNIMAC_MIB_GTJBR_GT
 ******************************************************************************/
const ru_field_rec UNIMAC_MIB_GTJBR_GT_FIELD =
{
    "GT",
#if RU_INCLUDE_DESC
    "",
    "Transmit jabber counter. ",
#endif
    UNIMAC_MIB_GTJBR_GT_FIELD_MASK,
    0,
    UNIMAC_MIB_GTJBR_GT_FIELD_WIDTH,
    UNIMAC_MIB_GTJBR_GT_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UNIMAC_MIB_GTBYT_GT
 ******************************************************************************/
const ru_field_rec UNIMAC_MIB_GTBYT_GT_FIELD =
{
    "GT",
#if RU_INCLUDE_DESC
    "",
    "Transmit byte counter. ",
#endif
    UNIMAC_MIB_GTBYT_GT_FIELD_MASK,
    0,
    UNIMAC_MIB_GTBYT_GT_FIELD_WIDTH,
    UNIMAC_MIB_GTBYT_GT_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UNIMAC_MIB_GTPOK_GT
 ******************************************************************************/
const ru_field_rec UNIMAC_MIB_GTPOK_GT_FIELD =
{
    "GT",
#if RU_INCLUDE_DESC
    "",
    "Transmitted good packets counter. ",
#endif
    UNIMAC_MIB_GTPOK_GT_FIELD_MASK,
    0,
    UNIMAC_MIB_GTPOK_GT_FIELD_WIDTH,
    UNIMAC_MIB_GTPOK_GT_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: UNIMAC_MIB_GTUC_GT
 ******************************************************************************/
const ru_field_rec UNIMAC_MIB_GTUC_GT_FIELD =
{
    "GT",
#if RU_INCLUDE_DESC
    "",
    "Transmitted Unicast packets counter.",
#endif
    UNIMAC_MIB_GTUC_GT_FIELD_MASK,
    0,
    UNIMAC_MIB_GTUC_GT_FIELD_WIDTH,
    UNIMAC_MIB_GTUC_GT_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

#endif /* RU_INCLUDE_FIELD_DB */

/******************************************************************************
 * Register: UNIMAC_MIB_MIB_CNTRL
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *UNIMAC_MIB_MIB_CNTRL_FIELDS[] =
{
    &UNIMAC_MIB_MIB_CNTRL_RESERVED0_FIELD,
    &UNIMAC_MIB_MIB_CNTRL_TX_CNT_RST_FIELD,
    &UNIMAC_MIB_MIB_CNTRL_RUNT_CNT_RST_FIELD,
    &UNIMAC_MIB_MIB_CNTRL_RX_CNT_ST_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec UNIMAC_MIB_MIB_CNTRL_REG = 
{
    "MIB_CNTRL",
#if RU_INCLUDE_DESC
    "MIB Control Register",
    "",
#endif
    UNIMAC_MIB_MIB_CNTRL_REG_OFFSET,
    0,
    0,
    884,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    4,
    UNIMAC_MIB_MIB_CNTRL_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: UNIMAC_MIB_GR64
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *UNIMAC_MIB_GR64_FIELDS[] =
{
    &UNIMAC_MIB_GR64_GR_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec UNIMAC_MIB_GR64_REG = 
{
    "GR64",
#if RU_INCLUDE_DESC
    "Receive 64B Frame Counter",
    "",
#endif
    UNIMAC_MIB_GR64_REG_OFFSET,
    0,
    0,
    885,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    UNIMAC_MIB_GR64_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: UNIMAC_MIB_GR127
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *UNIMAC_MIB_GR127_FIELDS[] =
{
    &UNIMAC_MIB_GR127_GR_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec UNIMAC_MIB_GR127_REG = 
{
    "GR127",
#if RU_INCLUDE_DESC
    "Receive 65B to 127B Frame Counter",
    "",
#endif
    UNIMAC_MIB_GR127_REG_OFFSET,
    0,
    0,
    886,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    UNIMAC_MIB_GR127_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: UNIMAC_MIB_GR255
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *UNIMAC_MIB_GR255_FIELDS[] =
{
    &UNIMAC_MIB_GR255_GR_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec UNIMAC_MIB_GR255_REG = 
{
    "GR255",
#if RU_INCLUDE_DESC
    "Receive 128B to 255B Frame Counter",
    "",
#endif
    UNIMAC_MIB_GR255_REG_OFFSET,
    0,
    0,
    887,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    UNIMAC_MIB_GR255_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: UNIMAC_MIB_GR511
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *UNIMAC_MIB_GR511_FIELDS[] =
{
    &UNIMAC_MIB_GR511_GR_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec UNIMAC_MIB_GR511_REG = 
{
    "GR511",
#if RU_INCLUDE_DESC
    "Receive 256B to 511B Frame Counter",
    "",
#endif
    UNIMAC_MIB_GR511_REG_OFFSET,
    0,
    0,
    888,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    UNIMAC_MIB_GR511_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: UNIMAC_MIB_GR1023
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *UNIMAC_MIB_GR1023_FIELDS[] =
{
    &UNIMAC_MIB_GR1023_GR_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec UNIMAC_MIB_GR1023_REG = 
{
    "GR1023",
#if RU_INCLUDE_DESC
    "Receive 512B to 1023B Frame Counter",
    "",
#endif
    UNIMAC_MIB_GR1023_REG_OFFSET,
    0,
    0,
    889,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    UNIMAC_MIB_GR1023_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: UNIMAC_MIB_GR1518
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *UNIMAC_MIB_GR1518_FIELDS[] =
{
    &UNIMAC_MIB_GR1518_GR_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec UNIMAC_MIB_GR1518_REG = 
{
    "GR1518",
#if RU_INCLUDE_DESC
    "Receive 1024B to 1518B Frame Counter",
    "",
#endif
    UNIMAC_MIB_GR1518_REG_OFFSET,
    0,
    0,
    890,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    UNIMAC_MIB_GR1518_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: UNIMAC_MIB_GRMGV
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *UNIMAC_MIB_GRMGV_FIELDS[] =
{
    &UNIMAC_MIB_GRMGV_GR_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec UNIMAC_MIB_GRMGV_REG = 
{
    "GRMGV",
#if RU_INCLUDE_DESC
    "Receive 1519B to 1522B Good VLAN Frame Counter",
    "",
#endif
    UNIMAC_MIB_GRMGV_REG_OFFSET,
    0,
    0,
    891,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    UNIMAC_MIB_GRMGV_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: UNIMAC_MIB_GR2047
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *UNIMAC_MIB_GR2047_FIELDS[] =
{
    &UNIMAC_MIB_GR2047_GR_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec UNIMAC_MIB_GR2047_REG = 
{
    "GR2047",
#if RU_INCLUDE_DESC
    "Receive 1519B to 2047B Frame Counter",
    "",
#endif
    UNIMAC_MIB_GR2047_REG_OFFSET,
    0,
    0,
    892,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    UNIMAC_MIB_GR2047_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: UNIMAC_MIB_GR4095
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *UNIMAC_MIB_GR4095_FIELDS[] =
{
    &UNIMAC_MIB_GR4095_GR_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec UNIMAC_MIB_GR4095_REG = 
{
    "GR4095",
#if RU_INCLUDE_DESC
    "Receive 2048B to 4095B Frame Counter",
    "",
#endif
    UNIMAC_MIB_GR4095_REG_OFFSET,
    0,
    0,
    893,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    UNIMAC_MIB_GR4095_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: UNIMAC_MIB_GR9216
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *UNIMAC_MIB_GR9216_FIELDS[] =
{
    &UNIMAC_MIB_GR9216_GR_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec UNIMAC_MIB_GR9216_REG = 
{
    "GR9216",
#if RU_INCLUDE_DESC
    "Receive 4096B to 9216B Frame Counter",
    "",
#endif
    UNIMAC_MIB_GR9216_REG_OFFSET,
    0,
    0,
    894,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    UNIMAC_MIB_GR9216_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: UNIMAC_MIB_GRPKT
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *UNIMAC_MIB_GRPKT_FIELDS[] =
{
    &UNIMAC_MIB_GRPKT_GR_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec UNIMAC_MIB_GRPKT_REG = 
{
    "GRPKT",
#if RU_INCLUDE_DESC
    "Receive Packet Counter",
    "",
#endif
    UNIMAC_MIB_GRPKT_REG_OFFSET,
    0,
    0,
    895,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    UNIMAC_MIB_GRPKT_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: UNIMAC_MIB_GRBYT
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *UNIMAC_MIB_GRBYT_FIELDS[] =
{
    &UNIMAC_MIB_GRBYT_GR_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec UNIMAC_MIB_GRBYT_REG = 
{
    "GRBYT",
#if RU_INCLUDE_DESC
    "Receive Byte Counter",
    "",
#endif
    UNIMAC_MIB_GRBYT_REG_OFFSET,
    0,
    0,
    896,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    UNIMAC_MIB_GRBYT_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: UNIMAC_MIB_GRMCA
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *UNIMAC_MIB_GRMCA_FIELDS[] =
{
    &UNIMAC_MIB_GRMCA_GR_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec UNIMAC_MIB_GRMCA_REG = 
{
    "GRMCA",
#if RU_INCLUDE_DESC
    "Receive Multicast Frame Counter",
    "",
#endif
    UNIMAC_MIB_GRMCA_REG_OFFSET,
    0,
    0,
    897,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    UNIMAC_MIB_GRMCA_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: UNIMAC_MIB_GRBCA
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *UNIMAC_MIB_GRBCA_FIELDS[] =
{
    &UNIMAC_MIB_GRBCA_GR_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec UNIMAC_MIB_GRBCA_REG = 
{
    "GRBCA",
#if RU_INCLUDE_DESC
    "Receive Broadcast Frame Counter",
    "",
#endif
    UNIMAC_MIB_GRBCA_REG_OFFSET,
    0,
    0,
    898,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    UNIMAC_MIB_GRBCA_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: UNIMAC_MIB_GRFCS
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *UNIMAC_MIB_GRFCS_FIELDS[] =
{
    &UNIMAC_MIB_GRFCS_GR_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec UNIMAC_MIB_GRFCS_REG = 
{
    "GRFCS",
#if RU_INCLUDE_DESC
    "Receive FCS Error Counter",
    "",
#endif
    UNIMAC_MIB_GRFCS_REG_OFFSET,
    0,
    0,
    899,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    UNIMAC_MIB_GRFCS_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: UNIMAC_MIB_GRXCF
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *UNIMAC_MIB_GRXCF_FIELDS[] =
{
    &UNIMAC_MIB_GRXCF_GR_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec UNIMAC_MIB_GRXCF_REG = 
{
    "GRXCF",
#if RU_INCLUDE_DESC
    "Receive Control Frame Packet Counter",
    "",
#endif
    UNIMAC_MIB_GRXCF_REG_OFFSET,
    0,
    0,
    900,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    UNIMAC_MIB_GRXCF_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: UNIMAC_MIB_GRXPF
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *UNIMAC_MIB_GRXPF_FIELDS[] =
{
    &UNIMAC_MIB_GRXPF_GR_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec UNIMAC_MIB_GRXPF_REG = 
{
    "GRXPF",
#if RU_INCLUDE_DESC
    "Receive Pause Frame Packet Counter",
    "",
#endif
    UNIMAC_MIB_GRXPF_REG_OFFSET,
    0,
    0,
    901,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    UNIMAC_MIB_GRXPF_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: UNIMAC_MIB_GRXUO
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *UNIMAC_MIB_GRXUO_FIELDS[] =
{
    &UNIMAC_MIB_GRXUO_GR_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec UNIMAC_MIB_GRXUO_REG = 
{
    "GRXUO",
#if RU_INCLUDE_DESC
    "Receive Unknown OP Code Packet Counter",
    "",
#endif
    UNIMAC_MIB_GRXUO_REG_OFFSET,
    0,
    0,
    902,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    UNIMAC_MIB_GRXUO_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: UNIMAC_MIB_GRALN
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *UNIMAC_MIB_GRALN_FIELDS[] =
{
    &UNIMAC_MIB_GRALN_GR_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec UNIMAC_MIB_GRALN_REG = 
{
    "GRALN",
#if RU_INCLUDE_DESC
    "Receive Alignmenet Error Counter",
    "",
#endif
    UNIMAC_MIB_GRALN_REG_OFFSET,
    0,
    0,
    903,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    UNIMAC_MIB_GRALN_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: UNIMAC_MIB_GRFLR
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *UNIMAC_MIB_GRFLR_FIELDS[] =
{
    &UNIMAC_MIB_GRFLR_GR_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec UNIMAC_MIB_GRFLR_REG = 
{
    "GRFLR",
#if RU_INCLUDE_DESC
    "Receive Frame Length Out Of Range Counter",
    "",
#endif
    UNIMAC_MIB_GRFLR_REG_OFFSET,
    0,
    0,
    904,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    UNIMAC_MIB_GRFLR_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: UNIMAC_MIB_GRCDE
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *UNIMAC_MIB_GRCDE_FIELDS[] =
{
    &UNIMAC_MIB_GRCDE_GR_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec UNIMAC_MIB_GRCDE_REG = 
{
    "GRCDE",
#if RU_INCLUDE_DESC
    "Receive Code Error Packet Counter",
    "",
#endif
    UNIMAC_MIB_GRCDE_REG_OFFSET,
    0,
    0,
    905,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    UNIMAC_MIB_GRCDE_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: UNIMAC_MIB_GRFCR
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *UNIMAC_MIB_GRFCR_FIELDS[] =
{
    &UNIMAC_MIB_GRFCR_GR_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec UNIMAC_MIB_GRFCR_REG = 
{
    "GRFCR",
#if RU_INCLUDE_DESC
    "Receive Carrier Sense Error Packet Counter",
    "",
#endif
    UNIMAC_MIB_GRFCR_REG_OFFSET,
    0,
    0,
    906,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    UNIMAC_MIB_GRFCR_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: UNIMAC_MIB_GROVR
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *UNIMAC_MIB_GROVR_FIELDS[] =
{
    &UNIMAC_MIB_GROVR_GR_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec UNIMAC_MIB_GROVR_REG = 
{
    "GROVR",
#if RU_INCLUDE_DESC
    "Receive Oversize Packet Counter",
    "",
#endif
    UNIMAC_MIB_GROVR_REG_OFFSET,
    0,
    0,
    907,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    UNIMAC_MIB_GROVR_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: UNIMAC_MIB_GRJBR
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *UNIMAC_MIB_GRJBR_FIELDS[] =
{
    &UNIMAC_MIB_GRJBR_GR_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec UNIMAC_MIB_GRJBR_REG = 
{
    "GRJBR",
#if RU_INCLUDE_DESC
    "Receive Jabber Counter",
    "",
#endif
    UNIMAC_MIB_GRJBR_REG_OFFSET,
    0,
    0,
    908,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    UNIMAC_MIB_GRJBR_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: UNIMAC_MIB_GRMTUE
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *UNIMAC_MIB_GRMTUE_FIELDS[] =
{
    &UNIMAC_MIB_GRMTUE_GR_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec UNIMAC_MIB_GRMTUE_REG = 
{
    "GRMTUE",
#if RU_INCLUDE_DESC
    "Receive MTU Error Packet Counter",
    "",
#endif
    UNIMAC_MIB_GRMTUE_REG_OFFSET,
    0,
    0,
    909,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    UNIMAC_MIB_GRMTUE_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: UNIMAC_MIB_GRPOK
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *UNIMAC_MIB_GRPOK_FIELDS[] =
{
    &UNIMAC_MIB_GRPOK_GR_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec UNIMAC_MIB_GRPOK_REG = 
{
    "GRPOK",
#if RU_INCLUDE_DESC
    "Receive Good Packet Counter",
    "",
#endif
    UNIMAC_MIB_GRPOK_REG_OFFSET,
    0,
    0,
    910,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    UNIMAC_MIB_GRPOK_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: UNIMAC_MIB_GRUC
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *UNIMAC_MIB_GRUC_FIELDS[] =
{
    &UNIMAC_MIB_GRUC_GR_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec UNIMAC_MIB_GRUC_REG = 
{
    "GRUC",
#if RU_INCLUDE_DESC
    "Receive Unicast Packet Counter",
    "",
#endif
    UNIMAC_MIB_GRUC_REG_OFFSET,
    0,
    0,
    911,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    UNIMAC_MIB_GRUC_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: UNIMAC_MIB_GRPPP
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *UNIMAC_MIB_GRPPP_FIELDS[] =
{
    &UNIMAC_MIB_GRPPP_GR_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec UNIMAC_MIB_GRPPP_REG = 
{
    "GRPPP",
#if RU_INCLUDE_DESC
    "Receive PPP Packet Counter",
    "",
#endif
    UNIMAC_MIB_GRPPP_REG_OFFSET,
    0,
    0,
    912,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    UNIMAC_MIB_GRPPP_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: UNIMAC_MIB_GRCRC
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *UNIMAC_MIB_GRCRC_FIELDS[] =
{
    &UNIMAC_MIB_GRCRC_GR_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec UNIMAC_MIB_GRCRC_REG = 
{
    "GRCRC",
#if RU_INCLUDE_DESC
    "Receive CRC Match Packet Counter",
    "",
#endif
    UNIMAC_MIB_GRCRC_REG_OFFSET,
    0,
    0,
    913,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    UNIMAC_MIB_GRCRC_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: UNIMAC_MIB_RRPKT
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *UNIMAC_MIB_RRPKT_FIELDS[] =
{
    &UNIMAC_MIB_RRPKT_RR_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec UNIMAC_MIB_RRPKT_REG = 
{
    "RRPKT",
#if RU_INCLUDE_DESC
    "Receive RUNT Packet Counter",
    "",
#endif
    UNIMAC_MIB_RRPKT_REG_OFFSET,
    0,
    0,
    914,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    UNIMAC_MIB_RRPKT_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: UNIMAC_MIB_RRUND
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *UNIMAC_MIB_RRUND_FIELDS[] =
{
    &UNIMAC_MIB_RRUND_RR_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec UNIMAC_MIB_RRUND_REG = 
{
    "RRUND",
#if RU_INCLUDE_DESC
    "Receive RUNT Packet And Contain A Valid FCS",
    "",
#endif
    UNIMAC_MIB_RRUND_REG_OFFSET,
    0,
    0,
    915,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    UNIMAC_MIB_RRUND_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: UNIMAC_MIB_RRFRG
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *UNIMAC_MIB_RRFRG_FIELDS[] =
{
    &UNIMAC_MIB_RRFRG_RR_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec UNIMAC_MIB_RRFRG_REG = 
{
    "RRFRG",
#if RU_INCLUDE_DESC
    "Receive RUNT Packet And Contain Invalid FCS or Alignment Error",
    "",
#endif
    UNIMAC_MIB_RRFRG_REG_OFFSET,
    0,
    0,
    916,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    UNIMAC_MIB_RRFRG_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: UNIMAC_MIB_RRBYT
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *UNIMAC_MIB_RRBYT_FIELDS[] =
{
    &UNIMAC_MIB_RRBYT_RR_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec UNIMAC_MIB_RRBYT_REG = 
{
    "RRBYT",
#if RU_INCLUDE_DESC
    "Receive RUNT Packet Byte Counter",
    "",
#endif
    UNIMAC_MIB_RRBYT_REG_OFFSET,
    0,
    0,
    917,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    UNIMAC_MIB_RRBYT_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: UNIMAC_MIB_TR64
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *UNIMAC_MIB_TR64_FIELDS[] =
{
    &UNIMAC_MIB_TR64_TR_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec UNIMAC_MIB_TR64_REG = 
{
    "TR64",
#if RU_INCLUDE_DESC
    "Transmit 64B Frame Counter",
    "",
#endif
    UNIMAC_MIB_TR64_REG_OFFSET,
    0,
    0,
    918,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    UNIMAC_MIB_TR64_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: UNIMAC_MIB_TR127
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *UNIMAC_MIB_TR127_FIELDS[] =
{
    &UNIMAC_MIB_TR127_TR_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec UNIMAC_MIB_TR127_REG = 
{
    "TR127",
#if RU_INCLUDE_DESC
    "Transmit 65B to 127B Frame Counter",
    "",
#endif
    UNIMAC_MIB_TR127_REG_OFFSET,
    0,
    0,
    919,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    UNIMAC_MIB_TR127_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: UNIMAC_MIB_TR255
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *UNIMAC_MIB_TR255_FIELDS[] =
{
    &UNIMAC_MIB_TR255_TR_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec UNIMAC_MIB_TR255_REG = 
{
    "TR255",
#if RU_INCLUDE_DESC
    "Transmit 128B to 255B Frame Counter",
    "",
#endif
    UNIMAC_MIB_TR255_REG_OFFSET,
    0,
    0,
    920,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    UNIMAC_MIB_TR255_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: UNIMAC_MIB_TR511
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *UNIMAC_MIB_TR511_FIELDS[] =
{
    &UNIMAC_MIB_TR511_TR_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec UNIMAC_MIB_TR511_REG = 
{
    "TR511",
#if RU_INCLUDE_DESC
    "Transmit 256B to 511B Frame Counter",
    "",
#endif
    UNIMAC_MIB_TR511_REG_OFFSET,
    0,
    0,
    921,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    UNIMAC_MIB_TR511_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: UNIMAC_MIB_TR1023
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *UNIMAC_MIB_TR1023_FIELDS[] =
{
    &UNIMAC_MIB_TR1023_TR_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec UNIMAC_MIB_TR1023_REG = 
{
    "TR1023",
#if RU_INCLUDE_DESC
    "Transmit 512B to 1023B Frame Counter",
    "",
#endif
    UNIMAC_MIB_TR1023_REG_OFFSET,
    0,
    0,
    922,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    UNIMAC_MIB_TR1023_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: UNIMAC_MIB_TR1518
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *UNIMAC_MIB_TR1518_FIELDS[] =
{
    &UNIMAC_MIB_TR1518_TR_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec UNIMAC_MIB_TR1518_REG = 
{
    "TR1518",
#if RU_INCLUDE_DESC
    "Transmit 1024B to 1518B Frame Counter",
    "",
#endif
    UNIMAC_MIB_TR1518_REG_OFFSET,
    0,
    0,
    923,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    UNIMAC_MIB_TR1518_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: UNIMAC_MIB_TRMGV
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *UNIMAC_MIB_TRMGV_FIELDS[] =
{
    &UNIMAC_MIB_TRMGV_TR_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec UNIMAC_MIB_TRMGV_REG = 
{
    "TRMGV",
#if RU_INCLUDE_DESC
    "Transmit 1519B to 1522B Good VLAN Frame Counter",
    "",
#endif
    UNIMAC_MIB_TRMGV_REG_OFFSET,
    0,
    0,
    924,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    UNIMAC_MIB_TRMGV_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: UNIMAC_MIB_TR2047
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *UNIMAC_MIB_TR2047_FIELDS[] =
{
    &UNIMAC_MIB_TR2047_TR_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec UNIMAC_MIB_TR2047_REG = 
{
    "TR2047",
#if RU_INCLUDE_DESC
    "Transmit 1519B to 2047B Frame Counter",
    "",
#endif
    UNIMAC_MIB_TR2047_REG_OFFSET,
    0,
    0,
    925,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    UNIMAC_MIB_TR2047_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: UNIMAC_MIB_TR4095
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *UNIMAC_MIB_TR4095_FIELDS[] =
{
    &UNIMAC_MIB_TR4095_TR_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec UNIMAC_MIB_TR4095_REG = 
{
    "TR4095",
#if RU_INCLUDE_DESC
    "Transmit 2048B to 4095B Frame Counter",
    "",
#endif
    UNIMAC_MIB_TR4095_REG_OFFSET,
    0,
    0,
    926,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    UNIMAC_MIB_TR4095_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: UNIMAC_MIB_TR9216
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *UNIMAC_MIB_TR9216_FIELDS[] =
{
    &UNIMAC_MIB_TR9216_TR_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec UNIMAC_MIB_TR9216_REG = 
{
    "TR9216",
#if RU_INCLUDE_DESC
    "Transmit 4096B to 9216B Frame Counter",
    "",
#endif
    UNIMAC_MIB_TR9216_REG_OFFSET,
    0,
    0,
    927,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    UNIMAC_MIB_TR9216_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: UNIMAC_MIB_GTPKT
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *UNIMAC_MIB_GTPKT_FIELDS[] =
{
    &UNIMAC_MIB_GTPKT_GT_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec UNIMAC_MIB_GTPKT_REG = 
{
    "GTPKT",
#if RU_INCLUDE_DESC
    "Transmit Packet Counter",
    "",
#endif
    UNIMAC_MIB_GTPKT_REG_OFFSET,
    0,
    0,
    928,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    UNIMAC_MIB_GTPKT_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: UNIMAC_MIB_GTMCA
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *UNIMAC_MIB_GTMCA_FIELDS[] =
{
    &UNIMAC_MIB_GTMCA_GT_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec UNIMAC_MIB_GTMCA_REG = 
{
    "GTMCA",
#if RU_INCLUDE_DESC
    "Transmit Multicast Packet Counter",
    "",
#endif
    UNIMAC_MIB_GTMCA_REG_OFFSET,
    0,
    0,
    929,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    UNIMAC_MIB_GTMCA_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: UNIMAC_MIB_GTBCA
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *UNIMAC_MIB_GTBCA_FIELDS[] =
{
    &UNIMAC_MIB_GTBCA_GT_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec UNIMAC_MIB_GTBCA_REG = 
{
    "GTBCA",
#if RU_INCLUDE_DESC
    "Transmit Broadcast Packet Counter",
    "",
#endif
    UNIMAC_MIB_GTBCA_REG_OFFSET,
    0,
    0,
    930,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    UNIMAC_MIB_GTBCA_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: UNIMAC_MIB_GTXPF
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *UNIMAC_MIB_GTXPF_FIELDS[] =
{
    &UNIMAC_MIB_GTXPF_GT_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec UNIMAC_MIB_GTXPF_REG = 
{
    "GTXPF",
#if RU_INCLUDE_DESC
    "Transmit Pause Frame Packet Counter",
    "",
#endif
    UNIMAC_MIB_GTXPF_REG_OFFSET,
    0,
    0,
    931,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    UNIMAC_MIB_GTXPF_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: UNIMAC_MIB_GTXCF
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *UNIMAC_MIB_GTXCF_FIELDS[] =
{
    &UNIMAC_MIB_GTXCF_GT_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec UNIMAC_MIB_GTXCF_REG = 
{
    "GTXCF",
#if RU_INCLUDE_DESC
    "Transmit Control Frame Packet Counter",
    "",
#endif
    UNIMAC_MIB_GTXCF_REG_OFFSET,
    0,
    0,
    932,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    UNIMAC_MIB_GTXCF_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: UNIMAC_MIB_GTFCS
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *UNIMAC_MIB_GTFCS_FIELDS[] =
{
    &UNIMAC_MIB_GTFCS_GT_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec UNIMAC_MIB_GTFCS_REG = 
{
    "GTFCS",
#if RU_INCLUDE_DESC
    "Transmit FCS Error Counter",
    "",
#endif
    UNIMAC_MIB_GTFCS_REG_OFFSET,
    0,
    0,
    933,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    UNIMAC_MIB_GTFCS_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: UNIMAC_MIB_GTOVR
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *UNIMAC_MIB_GTOVR_FIELDS[] =
{
    &UNIMAC_MIB_GTOVR_GT_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec UNIMAC_MIB_GTOVR_REG = 
{
    "GTOVR",
#if RU_INCLUDE_DESC
    "Transmit Oversize Packet Counter",
    "",
#endif
    UNIMAC_MIB_GTOVR_REG_OFFSET,
    0,
    0,
    934,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    UNIMAC_MIB_GTOVR_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: UNIMAC_MIB_GTDRF
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *UNIMAC_MIB_GTDRF_FIELDS[] =
{
    &UNIMAC_MIB_GTDRF_GT_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec UNIMAC_MIB_GTDRF_REG = 
{
    "GTDRF",
#if RU_INCLUDE_DESC
    "Transmit Deferral Packet Counter",
    "",
#endif
    UNIMAC_MIB_GTDRF_REG_OFFSET,
    0,
    0,
    935,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    UNIMAC_MIB_GTDRF_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: UNIMAC_MIB_GTEDF
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *UNIMAC_MIB_GTEDF_FIELDS[] =
{
    &UNIMAC_MIB_GTEDF_GT_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec UNIMAC_MIB_GTEDF_REG = 
{
    "GTEDF",
#if RU_INCLUDE_DESC
    "Transmit Excessive Deferral Packet Counter",
    "",
#endif
    UNIMAC_MIB_GTEDF_REG_OFFSET,
    0,
    0,
    936,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    UNIMAC_MIB_GTEDF_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: UNIMAC_MIB_GTSCL
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *UNIMAC_MIB_GTSCL_FIELDS[] =
{
    &UNIMAC_MIB_GTSCL_GT_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec UNIMAC_MIB_GTSCL_REG = 
{
    "GTSCL",
#if RU_INCLUDE_DESC
    "Transmit Single Collision Packet Counter",
    "",
#endif
    UNIMAC_MIB_GTSCL_REG_OFFSET,
    0,
    0,
    937,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    UNIMAC_MIB_GTSCL_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: UNIMAC_MIB_GTMCL
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *UNIMAC_MIB_GTMCL_FIELDS[] =
{
    &UNIMAC_MIB_GTMCL_GT_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec UNIMAC_MIB_GTMCL_REG = 
{
    "GTMCL",
#if RU_INCLUDE_DESC
    "Transmit Multiple Collision Packet Counter",
    "",
#endif
    UNIMAC_MIB_GTMCL_REG_OFFSET,
    0,
    0,
    938,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    UNIMAC_MIB_GTMCL_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: UNIMAC_MIB_GTLCL
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *UNIMAC_MIB_GTLCL_FIELDS[] =
{
    &UNIMAC_MIB_GTLCL_GT_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec UNIMAC_MIB_GTLCL_REG = 
{
    "GTLCL",
#if RU_INCLUDE_DESC
    "Transmit Late Collision Packet Counter",
    "",
#endif
    UNIMAC_MIB_GTLCL_REG_OFFSET,
    0,
    0,
    939,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    UNIMAC_MIB_GTLCL_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: UNIMAC_MIB_GTXCL
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *UNIMAC_MIB_GTXCL_FIELDS[] =
{
    &UNIMAC_MIB_GTXCL_GT_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec UNIMAC_MIB_GTXCL_REG = 
{
    "GTXCL",
#if RU_INCLUDE_DESC
    "Transmit Excessive Collision Packet Counter",
    "",
#endif
    UNIMAC_MIB_GTXCL_REG_OFFSET,
    0,
    0,
    940,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    UNIMAC_MIB_GTXCL_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: UNIMAC_MIB_GTFRG
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *UNIMAC_MIB_GTFRG_FIELDS[] =
{
    &UNIMAC_MIB_GTFRG_GT_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec UNIMAC_MIB_GTFRG_REG = 
{
    "GTFRG",
#if RU_INCLUDE_DESC
    "Transmit Fragments Packet Counter",
    "",
#endif
    UNIMAC_MIB_GTFRG_REG_OFFSET,
    0,
    0,
    941,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    UNIMAC_MIB_GTFRG_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: UNIMAC_MIB_GTNCL
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *UNIMAC_MIB_GTNCL_FIELDS[] =
{
    &UNIMAC_MIB_GTNCL_GT_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec UNIMAC_MIB_GTNCL_REG = 
{
    "GTNCL",
#if RU_INCLUDE_DESC
    "Transmit Total Collision Counter",
    "",
#endif
    UNIMAC_MIB_GTNCL_REG_OFFSET,
    0,
    0,
    942,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    UNIMAC_MIB_GTNCL_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: UNIMAC_MIB_GTJBR
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *UNIMAC_MIB_GTJBR_FIELDS[] =
{
    &UNIMAC_MIB_GTJBR_GT_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec UNIMAC_MIB_GTJBR_REG = 
{
    "GTJBR",
#if RU_INCLUDE_DESC
    "Transmit Jabber Counter",
    "",
#endif
    UNIMAC_MIB_GTJBR_REG_OFFSET,
    0,
    0,
    943,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    UNIMAC_MIB_GTJBR_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: UNIMAC_MIB_GTBYT
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *UNIMAC_MIB_GTBYT_FIELDS[] =
{
    &UNIMAC_MIB_GTBYT_GT_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec UNIMAC_MIB_GTBYT_REG = 
{
    "GTBYT",
#if RU_INCLUDE_DESC
    "Transmit Byte Counter",
    "",
#endif
    UNIMAC_MIB_GTBYT_REG_OFFSET,
    0,
    0,
    944,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    UNIMAC_MIB_GTBYT_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: UNIMAC_MIB_GTPOK
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *UNIMAC_MIB_GTPOK_FIELDS[] =
{
    &UNIMAC_MIB_GTPOK_GT_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec UNIMAC_MIB_GTPOK_REG = 
{
    "GTPOK",
#if RU_INCLUDE_DESC
    "Transmit Good Packet Counter",
    "",
#endif
    UNIMAC_MIB_GTPOK_REG_OFFSET,
    0,
    0,
    945,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    UNIMAC_MIB_GTPOK_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Register: UNIMAC_MIB_GTUC
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *UNIMAC_MIB_GTUC_FIELDS[] =
{
    &UNIMAC_MIB_GTUC_GT_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec UNIMAC_MIB_GTUC_REG = 
{
    "GTUC",
#if RU_INCLUDE_DESC
    "Transmit Unicast Packet Counter",
    "",
#endif
    UNIMAC_MIB_GTUC_REG_OFFSET,
    0,
    0,
    946,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    UNIMAC_MIB_GTUC_FIELDS
#endif /* RU_INCLUDE_FIELD_DB */
};

/******************************************************************************
 * Block: UNIMAC_MIB
 ******************************************************************************/
static const ru_reg_rec *UNIMAC_MIB_REGS[] =
{
    &UNIMAC_MIB_MIB_CNTRL_REG,
    &UNIMAC_MIB_GR64_REG,
    &UNIMAC_MIB_GR127_REG,
    &UNIMAC_MIB_GR255_REG,
    &UNIMAC_MIB_GR511_REG,
    &UNIMAC_MIB_GR1023_REG,
    &UNIMAC_MIB_GR1518_REG,
    &UNIMAC_MIB_GRMGV_REG,
    &UNIMAC_MIB_GR2047_REG,
    &UNIMAC_MIB_GR4095_REG,
    &UNIMAC_MIB_GR9216_REG,
    &UNIMAC_MIB_GRPKT_REG,
    &UNIMAC_MIB_GRBYT_REG,
    &UNIMAC_MIB_GRMCA_REG,
    &UNIMAC_MIB_GRBCA_REG,
    &UNIMAC_MIB_GRFCS_REG,
    &UNIMAC_MIB_GRXCF_REG,
    &UNIMAC_MIB_GRXPF_REG,
    &UNIMAC_MIB_GRXUO_REG,
    &UNIMAC_MIB_GRALN_REG,
    &UNIMAC_MIB_GRFLR_REG,
    &UNIMAC_MIB_GRCDE_REG,
    &UNIMAC_MIB_GRFCR_REG,
    &UNIMAC_MIB_GROVR_REG,
    &UNIMAC_MIB_GRJBR_REG,
    &UNIMAC_MIB_GRMTUE_REG,
    &UNIMAC_MIB_GRPOK_REG,
    &UNIMAC_MIB_GRUC_REG,
    &UNIMAC_MIB_GRPPP_REG,
    &UNIMAC_MIB_GRCRC_REG,
    &UNIMAC_MIB_RRPKT_REG,
    &UNIMAC_MIB_RRUND_REG,
    &UNIMAC_MIB_RRFRG_REG,
    &UNIMAC_MIB_RRBYT_REG,
    &UNIMAC_MIB_TR64_REG,
    &UNIMAC_MIB_TR127_REG,
    &UNIMAC_MIB_TR255_REG,
    &UNIMAC_MIB_TR511_REG,
    &UNIMAC_MIB_TR1023_REG,
    &UNIMAC_MIB_TR1518_REG,
    &UNIMAC_MIB_TRMGV_REG,
    &UNIMAC_MIB_TR2047_REG,
    &UNIMAC_MIB_TR4095_REG,
    &UNIMAC_MIB_TR9216_REG,
    &UNIMAC_MIB_GTPKT_REG,
    &UNIMAC_MIB_GTMCA_REG,
    &UNIMAC_MIB_GTBCA_REG,
    &UNIMAC_MIB_GTXPF_REG,
    &UNIMAC_MIB_GTXCF_REG,
    &UNIMAC_MIB_GTFCS_REG,
    &UNIMAC_MIB_GTOVR_REG,
    &UNIMAC_MIB_GTDRF_REG,
    &UNIMAC_MIB_GTEDF_REG,
    &UNIMAC_MIB_GTSCL_REG,
    &UNIMAC_MIB_GTMCL_REG,
    &UNIMAC_MIB_GTLCL_REG,
    &UNIMAC_MIB_GTXCL_REG,
    &UNIMAC_MIB_GTFRG_REG,
    &UNIMAC_MIB_GTNCL_REG,
    &UNIMAC_MIB_GTJBR_REG,
    &UNIMAC_MIB_GTBYT_REG,
    &UNIMAC_MIB_GTPOK_REG,
    &UNIMAC_MIB_GTUC_REG,
};

unsigned long UNIMAC_MIB_ADDRS[] =
{
    0x82da8180,
    0x82da8580,
    0x82da8980,
    0x82da8d80,
    0x82da9180,
};

const ru_block_rec UNIMAC_MIB_BLOCK = 
{
    "UNIMAC_MIB",
    UNIMAC_MIB_ADDRS,
    5,
    63,
    UNIMAC_MIB_REGS
};

/* End of file XRDP_UNIMAC_MIB.c */
