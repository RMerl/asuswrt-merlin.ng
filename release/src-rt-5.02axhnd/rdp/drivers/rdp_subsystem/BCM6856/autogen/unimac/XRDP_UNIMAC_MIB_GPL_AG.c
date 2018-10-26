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
 * Register: UNIMAC_MIB_MIB_CNTRL
 ******************************************************************************/
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
    886,
};

/******************************************************************************
 * Register: UNIMAC_MIB_GR64
 ******************************************************************************/
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
    887,
};

/******************************************************************************
 * Register: UNIMAC_MIB_GR127
 ******************************************************************************/
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
    888,
};

/******************************************************************************
 * Register: UNIMAC_MIB_GR255
 ******************************************************************************/
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
    889,
};

/******************************************************************************
 * Register: UNIMAC_MIB_GR511
 ******************************************************************************/
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
    890,
};

/******************************************************************************
 * Register: UNIMAC_MIB_GR1023
 ******************************************************************************/
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
    891,
};

/******************************************************************************
 * Register: UNIMAC_MIB_GR1518
 ******************************************************************************/
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
    892,
};

/******************************************************************************
 * Register: UNIMAC_MIB_GRMGV
 ******************************************************************************/
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
    893,
};

/******************************************************************************
 * Register: UNIMAC_MIB_GR2047
 ******************************************************************************/
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
    894,
};

/******************************************************************************
 * Register: UNIMAC_MIB_GR4095
 ******************************************************************************/
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
    895,
};

/******************************************************************************
 * Register: UNIMAC_MIB_GR9216
 ******************************************************************************/
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
    896,
};

/******************************************************************************
 * Register: UNIMAC_MIB_GRPKT
 ******************************************************************************/
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
    897,
};

/******************************************************************************
 * Register: UNIMAC_MIB_GRBYT
 ******************************************************************************/
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
    898,
};

/******************************************************************************
 * Register: UNIMAC_MIB_GRMCA
 ******************************************************************************/
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
    899,
};

/******************************************************************************
 * Register: UNIMAC_MIB_GRBCA
 ******************************************************************************/
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
    900,
};

/******************************************************************************
 * Register: UNIMAC_MIB_GRFCS
 ******************************************************************************/
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
    901,
};

/******************************************************************************
 * Register: UNIMAC_MIB_GRXCF
 ******************************************************************************/
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
    902,
};

/******************************************************************************
 * Register: UNIMAC_MIB_GRXPF
 ******************************************************************************/
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
    903,
};

/******************************************************************************
 * Register: UNIMAC_MIB_GRXUO
 ******************************************************************************/
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
    904,
};

/******************************************************************************
 * Register: UNIMAC_MIB_GRALN
 ******************************************************************************/
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
    905,
};

/******************************************************************************
 * Register: UNIMAC_MIB_GRFLR
 ******************************************************************************/
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
    906,
};

/******************************************************************************
 * Register: UNIMAC_MIB_GRCDE
 ******************************************************************************/
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
    907,
};

/******************************************************************************
 * Register: UNIMAC_MIB_GRFCR
 ******************************************************************************/
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
    908,
};

/******************************************************************************
 * Register: UNIMAC_MIB_GROVR
 ******************************************************************************/
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
    909,
};

/******************************************************************************
 * Register: UNIMAC_MIB_GRJBR
 ******************************************************************************/
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
    910,
};

/******************************************************************************
 * Register: UNIMAC_MIB_GRMTUE
 ******************************************************************************/
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
    911,
};

/******************************************************************************
 * Register: UNIMAC_MIB_GRPOK
 ******************************************************************************/
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
    912,
};

/******************************************************************************
 * Register: UNIMAC_MIB_GRUC
 ******************************************************************************/
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
    913,
};

/******************************************************************************
 * Register: UNIMAC_MIB_GRPPP
 ******************************************************************************/
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
    914,
};

/******************************************************************************
 * Register: UNIMAC_MIB_GRCRC
 ******************************************************************************/
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
    915,
};

/******************************************************************************
 * Register: UNIMAC_MIB_RRPKT
 ******************************************************************************/
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
    916,
};

/******************************************************************************
 * Register: UNIMAC_MIB_RRUND
 ******************************************************************************/
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
    917,
};

/******************************************************************************
 * Register: UNIMAC_MIB_RRFRG
 ******************************************************************************/
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
    918,
};

/******************************************************************************
 * Register: UNIMAC_MIB_RRBYT
 ******************************************************************************/
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
    919,
};

/******************************************************************************
 * Register: UNIMAC_MIB_TR64
 ******************************************************************************/
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
    920,
};

/******************************************************************************
 * Register: UNIMAC_MIB_TR127
 ******************************************************************************/
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
    921,
};

/******************************************************************************
 * Register: UNIMAC_MIB_TR255
 ******************************************************************************/
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
    922,
};

/******************************************************************************
 * Register: UNIMAC_MIB_TR511
 ******************************************************************************/
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
    923,
};

/******************************************************************************
 * Register: UNIMAC_MIB_TR1023
 ******************************************************************************/
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
    924,
};

/******************************************************************************
 * Register: UNIMAC_MIB_TR1518
 ******************************************************************************/
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
    925,
};

/******************************************************************************
 * Register: UNIMAC_MIB_TRMGV
 ******************************************************************************/
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
    926,
};

/******************************************************************************
 * Register: UNIMAC_MIB_TR2047
 ******************************************************************************/
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
    927,
};

/******************************************************************************
 * Register: UNIMAC_MIB_TR4095
 ******************************************************************************/
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
    928,
};

/******************************************************************************
 * Register: UNIMAC_MIB_TR9216
 ******************************************************************************/
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
    929,
};

/******************************************************************************
 * Register: UNIMAC_MIB_GTPKT
 ******************************************************************************/
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
    930,
};

/******************************************************************************
 * Register: UNIMAC_MIB_GTMCA
 ******************************************************************************/
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
    931,
};

/******************************************************************************
 * Register: UNIMAC_MIB_GTBCA
 ******************************************************************************/
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
    932,
};

/******************************************************************************
 * Register: UNIMAC_MIB_GTXPF
 ******************************************************************************/
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
    933,
};

/******************************************************************************
 * Register: UNIMAC_MIB_GTXCF
 ******************************************************************************/
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
    934,
};

/******************************************************************************
 * Register: UNIMAC_MIB_GTFCS
 ******************************************************************************/
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
    935,
};

/******************************************************************************
 * Register: UNIMAC_MIB_GTOVR
 ******************************************************************************/
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
    936,
};

/******************************************************************************
 * Register: UNIMAC_MIB_GTDRF
 ******************************************************************************/
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
    937,
};

/******************************************************************************
 * Register: UNIMAC_MIB_GTEDF
 ******************************************************************************/
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
    938,
};

/******************************************************************************
 * Register: UNIMAC_MIB_GTSCL
 ******************************************************************************/
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
    939,
};

/******************************************************************************
 * Register: UNIMAC_MIB_GTMCL
 ******************************************************************************/
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
    940,
};

/******************************************************************************
 * Register: UNIMAC_MIB_GTLCL
 ******************************************************************************/
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
    941,
};

/******************************************************************************
 * Register: UNIMAC_MIB_GTXCL
 ******************************************************************************/
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
    942,
};

/******************************************************************************
 * Register: UNIMAC_MIB_GTFRG
 ******************************************************************************/
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
    943,
};

/******************************************************************************
 * Register: UNIMAC_MIB_GTNCL
 ******************************************************************************/
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
    944,
};

/******************************************************************************
 * Register: UNIMAC_MIB_GTJBR
 ******************************************************************************/
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
    945,
};

/******************************************************************************
 * Register: UNIMAC_MIB_GTBYT
 ******************************************************************************/
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
    946,
};

/******************************************************************************
 * Register: UNIMAC_MIB_GTPOK
 ******************************************************************************/
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
    947,
};

/******************************************************************************
 * Register: UNIMAC_MIB_GTUC
 ******************************************************************************/
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
    948,
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
    0x82da9580,
};

const ru_block_rec UNIMAC_MIB_BLOCK = 
{
    "UNIMAC_MIB",
    UNIMAC_MIB_ADDRS,
    6,
    63,
    UNIMAC_MIB_REGS
};

/* End of file XRDP_UNIMAC_MIB.c */
