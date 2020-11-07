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
 * Field: NGPON_GEARBOX_RX_CTL_0_RESERVED0
 ******************************************************************************/
const ru_field_rec NGPON_GEARBOX_RX_CTL_0_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    NGPON_GEARBOX_RX_CTL_0_RESERVED0_FIELD_MASK,
    0,
    NGPON_GEARBOX_RX_CTL_0_RESERVED0_FIELD_WIDTH,
    NGPON_GEARBOX_RX_CTL_0_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: NGPON_GEARBOX_RX_CTL_0_CFNGPONGBOXRXFIFORDPTR
 ******************************************************************************/
const ru_field_rec NGPON_GEARBOX_RX_CTL_0_CFNGPONGBOXRXFIFORDPTR_FIELD =
{
    "CFNGPONGBOXRXFIFORDPTR",
#if RU_INCLUDE_DESC
    "",
    "Value for RX output RIFO read pointer.",
#endif
    NGPON_GEARBOX_RX_CTL_0_CFNGPONGBOXRXFIFORDPTR_FIELD_MASK,
    0,
    NGPON_GEARBOX_RX_CTL_0_CFNGPONGBOXRXFIFORDPTR_FIELD_WIDTH,
    NGPON_GEARBOX_RX_CTL_0_CFNGPONGBOXRXFIFORDPTR_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: NGPON_GEARBOX_RX_CTL_0_CFNGPONGBOXRXPTRAUTOLDDIS
 ******************************************************************************/
const ru_field_rec NGPON_GEARBOX_RX_CTL_0_CFNGPONGBOXRXPTRAUTOLDDIS_FIELD =
{
    "CFNGPONGBOXRXPTRAUTOLDDIS",
#if RU_INCLUDE_DESC
    "",
    "Disable pointer auto-load going into lock for output FIFO.",
#endif
    NGPON_GEARBOX_RX_CTL_0_CFNGPONGBOXRXPTRAUTOLDDIS_FIELD_MASK,
    0,
    NGPON_GEARBOX_RX_CTL_0_CFNGPONGBOXRXPTRAUTOLDDIS_FIELD_WIDTH,
    NGPON_GEARBOX_RX_CTL_0_CFNGPONGBOXRXPTRAUTOLDDIS_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: NGPON_GEARBOX_RX_CTL_0_CFNGPONGBOXRXMAXBADK
 ******************************************************************************/
const ru_field_rec NGPON_GEARBOX_RX_CTL_0_CFNGPONGBOXRXMAXBADK_FIELD =
{
    "CFNGPONGBOXRXMAXBADK",
#if RU_INCLUDE_DESC
    "",
    "Number of bad KChar to go out of lock.",
#endif
    NGPON_GEARBOX_RX_CTL_0_CFNGPONGBOXRXMAXBADK_FIELD_MASK,
    0,
    NGPON_GEARBOX_RX_CTL_0_CFNGPONGBOXRXMAXBADK_FIELD_WIDTH,
    NGPON_GEARBOX_RX_CTL_0_CFNGPONGBOXRXMAXBADK_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: NGPON_GEARBOX_RX_CTL_0_CFNGPONGBOXRXFRMK28ONLY
 ******************************************************************************/
const ru_field_rec NGPON_GEARBOX_RX_CTL_0_CFNGPONGBOXRXFRMK28ONLY_FIELD =
{
    "CFNGPONGBOXRXFRMK28ONLY",
#if RU_INCLUDE_DESC
    "",
    "Use only K28.5 for framing.",
#endif
    NGPON_GEARBOX_RX_CTL_0_CFNGPONGBOXRXFRMK28ONLY_FIELD_MASK,
    0,
    NGPON_GEARBOX_RX_CTL_0_CFNGPONGBOXRXFRMK28ONLY_FIELD_WIDTH,
    NGPON_GEARBOX_RX_CTL_0_CFNGPONGBOXRXFRMK28ONLY_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: NGPON_GEARBOX_RX_CTL_0_CFNGPONGBOXRXMAXGOODK
 ******************************************************************************/
const ru_field_rec NGPON_GEARBOX_RX_CTL_0_CFNGPONGBOXRXMAXGOODK_FIELD =
{
    "CFNGPONGBOXRXMAXGOODK",
#if RU_INCLUDE_DESC
    "",
    "Number of good KChar in a row for lock.",
#endif
    NGPON_GEARBOX_RX_CTL_0_CFNGPONGBOXRXMAXGOODK_FIELD_MASK,
    0,
    NGPON_GEARBOX_RX_CTL_0_CFNGPONGBOXRXMAXGOODK_FIELD_WIDTH,
    NGPON_GEARBOX_RX_CTL_0_CFNGPONGBOXRXMAXGOODK_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: NGPON_GEARBOX_RX_CTL_0_CFNGPONGBOXRXFRCHUNT
 ******************************************************************************/
const ru_field_rec NGPON_GEARBOX_RX_CTL_0_CFNGPONGBOXRXFRCHUNT_FIELD =
{
    "CFNGPONGBOXRXFRCHUNT",
#if RU_INCLUDE_DESC
    "",
    "Force 10b framer to go to HUNT state on rising edge.",
#endif
    NGPON_GEARBOX_RX_CTL_0_CFNGPONGBOXRXFRCHUNT_FIELD_MASK,
    0,
    NGPON_GEARBOX_RX_CTL_0_CFNGPONGBOXRXFRCHUNT_FIELD_WIDTH,
    NGPON_GEARBOX_RX_CTL_0_CFNGPONGBOXRXFRCHUNT_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: NGPON_GEARBOX_RX_CTL_0_CFNGPONGBOXRXOUTDATAFLIP
 ******************************************************************************/
const ru_field_rec NGPON_GEARBOX_RX_CTL_0_CFNGPONGBOXRXOUTDATAFLIP_FIELD =
{
    "CFNGPONGBOXRXOUTDATAFLIP",
#if RU_INCLUDE_DESC
    "",
    "Bitwise flip 32b output data.",
#endif
    NGPON_GEARBOX_RX_CTL_0_CFNGPONGBOXRXOUTDATAFLIP_FIELD_MASK,
    0,
    NGPON_GEARBOX_RX_CTL_0_CFNGPONGBOXRXOUTDATAFLIP_FIELD_WIDTH,
    NGPON_GEARBOX_RX_CTL_0_CFNGPONGBOXRXOUTDATAFLIP_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: NGPON_GEARBOX_RX_CTL_0_CFNGPONGBOXRXFRCMUXSEL
 ******************************************************************************/
const ru_field_rec NGPON_GEARBOX_RX_CTL_0_CFNGPONGBOXRXFRCMUXSEL_FIELD =
{
    "CFNGPONGBOXRXFRCMUXSEL",
#if RU_INCLUDE_DESC
    "",
    "Force mux select to value in cfNGponGboxRxFrcMuxVal.",
#endif
    NGPON_GEARBOX_RX_CTL_0_CFNGPONGBOXRXFRCMUXSEL_FIELD_MASK,
    0,
    NGPON_GEARBOX_RX_CTL_0_CFNGPONGBOXRXFRCMUXSEL_FIELD_WIDTH,
    NGPON_GEARBOX_RX_CTL_0_CFNGPONGBOXRXFRCMUXSEL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: NGPON_GEARBOX_RX_CTL_0_CFNGPONGBOXRXFRCMUXVAL
 ******************************************************************************/
const ru_field_rec NGPON_GEARBOX_RX_CTL_0_CFNGPONGBOXRXFRCMUXVAL_FIELD =
{
    "CFNGPONGBOXRXFRCMUXVAL",
#if RU_INCLUDE_DESC
    "",
    "Value that will be forced to mux select when cfNGponGboxRxFrcMuxSel"
    "is asserted.",
#endif
    NGPON_GEARBOX_RX_CTL_0_CFNGPONGBOXRXFRCMUXVAL_FIELD_MASK,
    0,
    NGPON_GEARBOX_RX_CTL_0_CFNGPONGBOXRXFRCMUXVAL_FIELD_WIDTH,
    NGPON_GEARBOX_RX_CTL_0_CFNGPONGBOXRXFRCMUXVAL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: NGPON_GEARBOX_RX_CTL_0_CFNGPONGBOXRX20BDATAFLIP
 ******************************************************************************/
const ru_field_rec NGPON_GEARBOX_RX_CTL_0_CFNGPONGBOXRX20BDATAFLIP_FIELD =
{
    "CFNGPONGBOXRX20BDATAFLIP",
#if RU_INCLUDE_DESC
    "",
    "Bitwise flip RX 20b gearbox data.",
#endif
    NGPON_GEARBOX_RX_CTL_0_CFNGPONGBOXRX20BDATAFLIP_FIELD_MASK,
    0,
    NGPON_GEARBOX_RX_CTL_0_CFNGPONGBOXRX20BDATAFLIP_FIELD_WIDTH,
    NGPON_GEARBOX_RX_CTL_0_CFNGPONGBOXRX20BDATAFLIP_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: NGPON_GEARBOX_RX_CTL_0_CFNGPONGBOXRXSERDATAFLIP
 ******************************************************************************/
const ru_field_rec NGPON_GEARBOX_RX_CTL_0_CFNGPONGBOXRXSERDATAFLIP_FIELD =
{
    "CFNGPONGBOXRXSERDATAFLIP",
#if RU_INCLUDE_DESC
    "",
    "Bitwise flip RX 16b data from SERDES.",
#endif
    NGPON_GEARBOX_RX_CTL_0_CFNGPONGBOXRXSERDATAFLIP_FIELD_MASK,
    0,
    NGPON_GEARBOX_RX_CTL_0_CFNGPONGBOXRXSERDATAFLIP_FIELD_WIDTH,
    NGPON_GEARBOX_RX_CTL_0_CFNGPONGBOXRXSERDATAFLIP_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: NGPON_GEARBOX_RX_CTL_0_CFNGPONGBOXRXSERDATAINV
 ******************************************************************************/
const ru_field_rec NGPON_GEARBOX_RX_CTL_0_CFNGPONGBOXRXSERDATAINV_FIELD =
{
    "CFNGPONGBOXRXSERDATAINV",
#if RU_INCLUDE_DESC
    "",
    "Bitwise invert RX 16b data from SERDES.",
#endif
    NGPON_GEARBOX_RX_CTL_0_CFNGPONGBOXRXSERDATAINV_FIELD_MASK,
    0,
    NGPON_GEARBOX_RX_CTL_0_CFNGPONGBOXRXSERDATAINV_FIELD_WIDTH,
    NGPON_GEARBOX_RX_CTL_0_CFNGPONGBOXRXSERDATAINV_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: NGPON_GEARBOX_RX_CTL_0_CFNGPONGBOXRXFIFOPTRLD
 ******************************************************************************/
const ru_field_rec NGPON_GEARBOX_RX_CTL_0_CFNGPONGBOXRXFIFOPTRLD_FIELD =
{
    "CFNGPONGBOXRXFIFOPTRLD",
#if RU_INCLUDE_DESC
    "",
    "Load value for FIFO read pointer.  Write pointer will be loaded to"
    "0.",
#endif
    NGPON_GEARBOX_RX_CTL_0_CFNGPONGBOXRXFIFOPTRLD_FIELD_MASK,
    0,
    NGPON_GEARBOX_RX_CTL_0_CFNGPONGBOXRXFIFOPTRLD_FIELD_WIDTH,
    NGPON_GEARBOX_RX_CTL_0_CFNGPONGBOXRXFIFOPTRLD_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: NGPON_GEARBOX_RX_CTL_0_CFNGPONGBOXRXSWSYNCHOLD
 ******************************************************************************/
const ru_field_rec NGPON_GEARBOX_RX_CTL_0_CFNGPONGBOXRXSWSYNCHOLD_FIELD =
{
    "CFNGPONGBOXRXSWSYNCHOLD",
#if RU_INCLUDE_DESC
    "",
    "When set, synchronization will be held indefinitely.",
#endif
    NGPON_GEARBOX_RX_CTL_0_CFNGPONGBOXRXSWSYNCHOLD_FIELD_MASK,
    0,
    NGPON_GEARBOX_RX_CTL_0_CFNGPONGBOXRXSWSYNCHOLD_FIELD_WIDTH,
    NGPON_GEARBOX_RX_CTL_0_CFNGPONGBOXRXSWSYNCHOLD_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: NGPON_GEARBOX_RX_CTL_0_CFNGPONGBOXRXMODE
 ******************************************************************************/
const ru_field_rec NGPON_GEARBOX_RX_CTL_0_CFNGPONGBOXRXMODE_FIELD =
{
    "CFNGPONGBOXRXMODE",
#if RU_INCLUDE_DESC
    "",
    "0. 8B/10B decoder mode operating at 777 MHz.  1. Pass through mode"
    "operating at 622 MHz.",
#endif
    NGPON_GEARBOX_RX_CTL_0_CFNGPONGBOXRXMODE_FIELD_MASK,
    0,
    NGPON_GEARBOX_RX_CTL_0_CFNGPONGBOXRXMODE_FIELD_WIDTH,
    NGPON_GEARBOX_RX_CTL_0_CFNGPONGBOXRXMODE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: NGPON_GEARBOX_RX_CTL_0_CFNGPONGBOXRXEN
 ******************************************************************************/
const ru_field_rec NGPON_GEARBOX_RX_CTL_0_CFNGPONGBOXRXEN_FIELD =
{
    "CFNGPONGBOXRXEN",
#if RU_INCLUDE_DESC
    "",
    "Synchronous enable for RX gearbox.",
#endif
    NGPON_GEARBOX_RX_CTL_0_CFNGPONGBOXRXEN_FIELD_MASK,
    0,
    NGPON_GEARBOX_RX_CTL_0_CFNGPONGBOXRXEN_FIELD_WIDTH,
    NGPON_GEARBOX_RX_CTL_0_CFNGPONGBOXRXEN_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: NGPON_GEARBOX_RX_CTL_0_CFNGPONGBOXRSTN
 ******************************************************************************/
const ru_field_rec NGPON_GEARBOX_RX_CTL_0_CFNGPONGBOXRSTN_FIELD =
{
    "CFNGPONGBOXRSTN",
#if RU_INCLUDE_DESC
    "",
    "Asynchronous, active-low, software reset for gearbox.",
#endif
    NGPON_GEARBOX_RX_CTL_0_CFNGPONGBOXRSTN_FIELD_MASK,
    0,
    NGPON_GEARBOX_RX_CTL_0_CFNGPONGBOXRSTN_FIELD_WIDTH,
    NGPON_GEARBOX_RX_CTL_0_CFNGPONGBOXRSTN_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: NGPON_GEARBOX_RX_CTL_1_RESERVED0
 ******************************************************************************/
const ru_field_rec NGPON_GEARBOX_RX_CTL_1_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    NGPON_GEARBOX_RX_CTL_1_RESERVED0_FIELD_MASK,
    0,
    NGPON_GEARBOX_RX_CTL_1_RESERVED0_FIELD_WIDTH,
    NGPON_GEARBOX_RX_CTL_1_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: NGPON_GEARBOX_RX_CTL_1_CFNGPONGBOXRXMAXTIMERCNT
 ******************************************************************************/
const ru_field_rec NGPON_GEARBOX_RX_CTL_1_CFNGPONGBOXRXMAXTIMERCNT_FIELD =
{
    "CFNGPONGBOXRXMAXTIMERCNT",
#if RU_INCLUDE_DESC
    "",
    "Max counter for 125 us timer.",
#endif
    NGPON_GEARBOX_RX_CTL_1_CFNGPONGBOXRXMAXTIMERCNT_FIELD_MASK,
    0,
    NGPON_GEARBOX_RX_CTL_1_CFNGPONGBOXRXMAXTIMERCNT_FIELD_WIDTH,
    NGPON_GEARBOX_RX_CTL_1_CFNGPONGBOXRXMAXTIMERCNT_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: NGPON_GEARBOX_RX_CTL_2_RESERVED0
 ******************************************************************************/
const ru_field_rec NGPON_GEARBOX_RX_CTL_2_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    NGPON_GEARBOX_RX_CTL_2_RESERVED0_FIELD_MASK,
    0,
    NGPON_GEARBOX_RX_CTL_2_RESERVED0_FIELD_WIDTH,
    NGPON_GEARBOX_RX_CTL_2_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: NGPON_GEARBOX_RX_CTL_2_CFNGPONGBOXRXK28D5RDP
 ******************************************************************************/
const ru_field_rec NGPON_GEARBOX_RX_CTL_2_CFNGPONGBOXRXK28D5RDP_FIELD =
{
    "CFNGPONGBOXRXK28D5RDP",
#if RU_INCLUDE_DESC
    "",
    "RD+ K28.5 pattern.",
#endif
    NGPON_GEARBOX_RX_CTL_2_CFNGPONGBOXRXK28D5RDP_FIELD_MASK,
    0,
    NGPON_GEARBOX_RX_CTL_2_CFNGPONGBOXRXK28D5RDP_FIELD_WIDTH,
    NGPON_GEARBOX_RX_CTL_2_CFNGPONGBOXRXK28D5RDP_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: NGPON_GEARBOX_RX_CTL_2_RESERVED1
 ******************************************************************************/
const ru_field_rec NGPON_GEARBOX_RX_CTL_2_RESERVED1_FIELD =
{
    "RESERVED1",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    NGPON_GEARBOX_RX_CTL_2_RESERVED1_FIELD_MASK,
    0,
    NGPON_GEARBOX_RX_CTL_2_RESERVED1_FIELD_WIDTH,
    NGPON_GEARBOX_RX_CTL_2_RESERVED1_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: NGPON_GEARBOX_RX_CTL_2_CFNGPONGBOXRXK28D5RDN
 ******************************************************************************/
const ru_field_rec NGPON_GEARBOX_RX_CTL_2_CFNGPONGBOXRXK28D5RDN_FIELD =
{
    "CFNGPONGBOXRXK28D5RDN",
#if RU_INCLUDE_DESC
    "",
    "RD- K28.5 pattern.",
#endif
    NGPON_GEARBOX_RX_CTL_2_CFNGPONGBOXRXK28D5RDN_FIELD_MASK,
    0,
    NGPON_GEARBOX_RX_CTL_2_CFNGPONGBOXRXK28D5RDN_FIELD_WIDTH,
    NGPON_GEARBOX_RX_CTL_2_CFNGPONGBOXRXK28D5RDN_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: NGPON_GEARBOX_RX_CTL_3_RESERVED0
 ******************************************************************************/
const ru_field_rec NGPON_GEARBOX_RX_CTL_3_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    NGPON_GEARBOX_RX_CTL_3_RESERVED0_FIELD_MASK,
    0,
    NGPON_GEARBOX_RX_CTL_3_RESERVED0_FIELD_WIDTH,
    NGPON_GEARBOX_RX_CTL_3_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: NGPON_GEARBOX_RX_CTL_3_CFNGPONGBOXRXD5D7RDP
 ******************************************************************************/
const ru_field_rec NGPON_GEARBOX_RX_CTL_3_CFNGPONGBOXRXD5D7RDP_FIELD =
{
    "CFNGPONGBOXRXD5D7RDP",
#if RU_INCLUDE_DESC
    "",
    "RD+ D5.7 pattern.",
#endif
    NGPON_GEARBOX_RX_CTL_3_CFNGPONGBOXRXD5D7RDP_FIELD_MASK,
    0,
    NGPON_GEARBOX_RX_CTL_3_CFNGPONGBOXRXD5D7RDP_FIELD_WIDTH,
    NGPON_GEARBOX_RX_CTL_3_CFNGPONGBOXRXD5D7RDP_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: NGPON_GEARBOX_RX_CTL_3_RESERVED1
 ******************************************************************************/
const ru_field_rec NGPON_GEARBOX_RX_CTL_3_RESERVED1_FIELD =
{
    "RESERVED1",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    NGPON_GEARBOX_RX_CTL_3_RESERVED1_FIELD_MASK,
    0,
    NGPON_GEARBOX_RX_CTL_3_RESERVED1_FIELD_WIDTH,
    NGPON_GEARBOX_RX_CTL_3_RESERVED1_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: NGPON_GEARBOX_RX_CTL_3_CFNGPONGBOXRXD5D7RDN
 ******************************************************************************/
const ru_field_rec NGPON_GEARBOX_RX_CTL_3_CFNGPONGBOXRXD5D7RDN_FIELD =
{
    "CFNGPONGBOXRXD5D7RDN",
#if RU_INCLUDE_DESC
    "",
    "RD- D5.7 pattern.",
#endif
    NGPON_GEARBOX_RX_CTL_3_CFNGPONGBOXRXD5D7RDN_FIELD_MASK,
    0,
    NGPON_GEARBOX_RX_CTL_3_CFNGPONGBOXRXD5D7RDN_FIELD_WIDTH,
    NGPON_GEARBOX_RX_CTL_3_CFNGPONGBOXRXD5D7RDN_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: NGPON_GEARBOX_TX_CTL_RESERVED0
 ******************************************************************************/
const ru_field_rec NGPON_GEARBOX_TX_CTL_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    NGPON_GEARBOX_TX_CTL_RESERVED0_FIELD_MASK,
    0,
    NGPON_GEARBOX_TX_CTL_RESERVED0_FIELD_WIDTH,
    NGPON_GEARBOX_TX_CTL_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: NGPON_GEARBOX_TX_CTL_CFNGPONGBOXTXFIFODATARDPTR
 ******************************************************************************/
const ru_field_rec NGPON_GEARBOX_TX_CTL_CFNGPONGBOXTXFIFODATARDPTR_FIELD =
{
    "CFNGPONGBOXTXFIFODATARDPTR",
#if RU_INCLUDE_DESC
    "",
    "Value for TX data FIFO read pointer.  Steps of 2 x txClk (622 MHz),"
    "jumps of 32 bits.",
#endif
    NGPON_GEARBOX_TX_CTL_CFNGPONGBOXTXFIFODATARDPTR_FIELD_MASK,
    0,
    NGPON_GEARBOX_TX_CTL_CFNGPONGBOXTXFIFODATARDPTR_FIELD_WIDTH,
    NGPON_GEARBOX_TX_CTL_CFNGPONGBOXTXFIFODATARDPTR_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: NGPON_GEARBOX_TX_CTL_RESERVED1
 ******************************************************************************/
const ru_field_rec NGPON_GEARBOX_TX_CTL_RESERVED1_FIELD =
{
    "RESERVED1",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    NGPON_GEARBOX_TX_CTL_RESERVED1_FIELD_MASK,
    0,
    NGPON_GEARBOX_TX_CTL_RESERVED1_FIELD_WIDTH,
    NGPON_GEARBOX_TX_CTL_RESERVED1_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: NGPON_GEARBOX_TX_CTL_CFNGPONGBOXTXFIFOVLDOFF
 ******************************************************************************/
const ru_field_rec NGPON_GEARBOX_TX_CTL_CFNGPONGBOXTXFIFOVLDOFF_FIELD =
{
    "CFNGPONGBOXTXFIFOVLDOFF",
#if RU_INCLUDE_DESC
    "",
    "Value for TX valid FIFO offset.  Steps of txClk (622 MHz), jumps of"
    "16 bits.  1 to 15 are advances valid vs data, valid comes out ahead."
    "1=1 clock, 2=2 clocks, 3=3 clocks...  31 to 16 are advances valid"
    "vs data, valid comes out behind.  31=1 clock, 30=2 clocks, 29=3"
    "clocks...",
#endif
    NGPON_GEARBOX_TX_CTL_CFNGPONGBOXTXFIFOVLDOFF_FIELD_MASK,
    0,
    NGPON_GEARBOX_TX_CTL_CFNGPONGBOXTXFIFOVLDOFF_FIELD_WIDTH,
    NGPON_GEARBOX_TX_CTL_CFNGPONGBOXTXFIFOVLDOFF_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: NGPON_GEARBOX_TX_CTL_RESERVED2
 ******************************************************************************/
const ru_field_rec NGPON_GEARBOX_TX_CTL_RESERVED2_FIELD =
{
    "RESERVED2",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    NGPON_GEARBOX_TX_CTL_RESERVED2_FIELD_MASK,
    0,
    NGPON_GEARBOX_TX_CTL_RESERVED2_FIELD_WIDTH,
    NGPON_GEARBOX_TX_CTL_RESERVED2_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: NGPON_GEARBOX_TX_CTL_CFNGPONGBOXTXSERVLDFLIP
 ******************************************************************************/
const ru_field_rec NGPON_GEARBOX_TX_CTL_CFNGPONGBOXTXSERVLDFLIP_FIELD =
{
    "CFNGPONGBOXTXSERVLDFLIP",
#if RU_INCLUDE_DESC
    "",
    "Flip TX data valid endian on 32b input.",
#endif
    NGPON_GEARBOX_TX_CTL_CFNGPONGBOXTXSERVLDFLIP_FIELD_MASK,
    0,
    NGPON_GEARBOX_TX_CTL_CFNGPONGBOXTXSERVLDFLIP_FIELD_WIDTH,
    NGPON_GEARBOX_TX_CTL_CFNGPONGBOXTXSERVLDFLIP_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: NGPON_GEARBOX_TX_CTL_CFNGPONGBOXTXSERDATAFLIP
 ******************************************************************************/
const ru_field_rec NGPON_GEARBOX_TX_CTL_CFNGPONGBOXTXSERDATAFLIP_FIELD =
{
    "CFNGPONGBOXTXSERDATAFLIP",
#if RU_INCLUDE_DESC
    "",
    "Flip TX data endian on 32b input.",
#endif
    NGPON_GEARBOX_TX_CTL_CFNGPONGBOXTXSERDATAFLIP_FIELD_MASK,
    0,
    NGPON_GEARBOX_TX_CTL_CFNGPONGBOXTXSERDATAFLIP_FIELD_WIDTH,
    NGPON_GEARBOX_TX_CTL_CFNGPONGBOXTXSERDATAFLIP_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: NGPON_GEARBOX_TX_CTL_CFNGPONGBOXTXSERVLDINV
 ******************************************************************************/
const ru_field_rec NGPON_GEARBOX_TX_CTL_CFNGPONGBOXTXSERVLDINV_FIELD =
{
    "CFNGPONGBOXTXSERVLDINV",
#if RU_INCLUDE_DESC
    "",
    "Bitwise invert TX 4b valid to SERDES.",
#endif
    NGPON_GEARBOX_TX_CTL_CFNGPONGBOXTXSERVLDINV_FIELD_MASK,
    0,
    NGPON_GEARBOX_TX_CTL_CFNGPONGBOXTXSERVLDINV_FIELD_WIDTH,
    NGPON_GEARBOX_TX_CTL_CFNGPONGBOXTXSERVLDINV_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: NGPON_GEARBOX_TX_CTL_CFNGPONGBOXTXSERDATAINV
 ******************************************************************************/
const ru_field_rec NGPON_GEARBOX_TX_CTL_CFNGPONGBOXTXSERDATAINV_FIELD =
{
    "CFNGPONGBOXTXSERDATAINV",
#if RU_INCLUDE_DESC
    "",
    "Bitwise invert TX 16b data to SERDES.",
#endif
    NGPON_GEARBOX_TX_CTL_CFNGPONGBOXTXSERDATAINV_FIELD_MASK,
    0,
    NGPON_GEARBOX_TX_CTL_CFNGPONGBOXTXSERDATAINV_FIELD_WIDTH,
    NGPON_GEARBOX_TX_CTL_CFNGPONGBOXTXSERDATAINV_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: NGPON_GEARBOX_TX_CTL_CFNGPONGBOXTXFIFOVLDPTRLD
 ******************************************************************************/
const ru_field_rec NGPON_GEARBOX_TX_CTL_CFNGPONGBOXTXFIFOVLDPTRLD_FIELD =
{
    "CFNGPONGBOXTXFIFOVLDPTRLD",
#if RU_INCLUDE_DESC
    "",
    "Load only the offset for TX valid FIFO pointer.  This is an offset"
    "from the data read pointer.",
#endif
    NGPON_GEARBOX_TX_CTL_CFNGPONGBOXTXFIFOVLDPTRLD_FIELD_MASK,
    0,
    NGPON_GEARBOX_TX_CTL_CFNGPONGBOXTXFIFOVLDPTRLD_FIELD_WIDTH,
    NGPON_GEARBOX_TX_CTL_CFNGPONGBOXTXFIFOVLDPTRLD_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: NGPON_GEARBOX_TX_CTL_CFNGPONGBOXTXFIFOPTRLD
 ******************************************************************************/
const ru_field_rec NGPON_GEARBOX_TX_CTL_CFNGPONGBOXTXFIFOPTRLD_FIELD =
{
    "CFNGPONGBOXTXFIFOPTRLD",
#if RU_INCLUDE_DESC
    "",
    "Load value for TX data FIFO read pointer and valid read pointer"
    "offset.  Data/valid write will be loaded to 0.",
#endif
    NGPON_GEARBOX_TX_CTL_CFNGPONGBOXTXFIFOPTRLD_FIELD_MASK,
    0,
    NGPON_GEARBOX_TX_CTL_CFNGPONGBOXTXFIFOPTRLD_FIELD_WIDTH,
    NGPON_GEARBOX_TX_CTL_CFNGPONGBOXTXFIFOPTRLD_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: NGPON_GEARBOX_TX_CTL_CFNGPONGBOXTXEN
 ******************************************************************************/
const ru_field_rec NGPON_GEARBOX_TX_CTL_CFNGPONGBOXTXEN_FIELD =
{
    "CFNGPONGBOXTXEN",
#if RU_INCLUDE_DESC
    "",
    "Synchronous enable for TX gearbox.",
#endif
    NGPON_GEARBOX_TX_CTL_CFNGPONGBOXTXEN_FIELD_MASK,
    0,
    NGPON_GEARBOX_TX_CTL_CFNGPONGBOXTXEN_FIELD_WIDTH,
    NGPON_GEARBOX_TX_CTL_CFNGPONGBOXTXEN_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: NGPON_GEARBOX_STATUS_RESERVED0
 ******************************************************************************/
const ru_field_rec NGPON_GEARBOX_STATUS_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    NGPON_GEARBOX_STATUS_RESERVED0_FIELD_MASK,
    0,
    NGPON_GEARBOX_STATUS_RESERVED0_FIELD_WIDTH,
    NGPON_GEARBOX_STATUS_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: NGPON_GEARBOX_STATUS_NGPONTXGBOXFIFOVLDPTRCOL
 ******************************************************************************/
const ru_field_rec NGPON_GEARBOX_STATUS_NGPONTXGBOXFIFOVLDPTRCOL_FIELD =
{
    "NGPONTXGBOXFIFOVLDPTRCOL",
#if RU_INCLUDE_DESC
    "",
    "Pointer collision.",
#endif
    NGPON_GEARBOX_STATUS_NGPONTXGBOXFIFOVLDPTRCOL_FIELD_MASK,
    0,
    NGPON_GEARBOX_STATUS_NGPONTXGBOXFIFOVLDPTRCOL_FIELD_WIDTH,
    NGPON_GEARBOX_STATUS_NGPONTXGBOXFIFOVLDPTRCOL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: NGPON_GEARBOX_STATUS_NGPONRXGBOXSTATE
 ******************************************************************************/
const ru_field_rec NGPON_GEARBOX_STATUS_NGPONRXGBOXSTATE_FIELD =
{
    "NGPONRXGBOXSTATE",
#if RU_INCLUDE_DESC
    "",
    "Framer state.",
#endif
    NGPON_GEARBOX_STATUS_NGPONRXGBOXSTATE_FIELD_MASK,
    0,
    NGPON_GEARBOX_STATUS_NGPONRXGBOXSTATE_FIELD_WIDTH,
    NGPON_GEARBOX_STATUS_NGPONRXGBOXSTATE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: NGPON_GEARBOX_STATUS_NGPONTXGBOXFIFODATAPTRCOL
 ******************************************************************************/
const ru_field_rec NGPON_GEARBOX_STATUS_NGPONTXGBOXFIFODATAPTRCOL_FIELD =
{
    "NGPONTXGBOXFIFODATAPTRCOL",
#if RU_INCLUDE_DESC
    "",
    "Pointer collision.",
#endif
    NGPON_GEARBOX_STATUS_NGPONTXGBOXFIFODATAPTRCOL_FIELD_MASK,
    0,
    NGPON_GEARBOX_STATUS_NGPONTXGBOXFIFODATAPTRCOL_FIELD_WIDTH,
    NGPON_GEARBOX_STATUS_NGPONTXGBOXFIFODATAPTRCOL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: NGPON_GEARBOX_STATUS_NGPONRXGBOXKCNT
 ******************************************************************************/
const ru_field_rec NGPON_GEARBOX_STATUS_NGPONRXGBOXKCNT_FIELD =
{
    "NGPONRXGBOXKCNT",
#if RU_INCLUDE_DESC
    "",
    "Number of KChar.",
#endif
    NGPON_GEARBOX_STATUS_NGPONRXGBOXKCNT_FIELD_MASK,
    0,
    NGPON_GEARBOX_STATUS_NGPONRXGBOXKCNT_FIELD_WIDTH,
    NGPON_GEARBOX_STATUS_NGPONRXGBOXKCNT_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: NGPON_GEARBOX_STATUS_NGPONRXGBOXFIFOPTRDELTA
 ******************************************************************************/
const ru_field_rec NGPON_GEARBOX_STATUS_NGPONRXGBOXFIFOPTRDELTA_FIELD =
{
    "NGPONRXGBOXFIFOPTRDELTA",
#if RU_INCLUDE_DESC
    "",
    "Pointer delta.",
#endif
    NGPON_GEARBOX_STATUS_NGPONRXGBOXFIFOPTRDELTA_FIELD_MASK,
    0,
    NGPON_GEARBOX_STATUS_NGPONRXGBOXFIFOPTRDELTA_FIELD_WIDTH,
    NGPON_GEARBOX_STATUS_NGPONRXGBOXFIFOPTRDELTA_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: NGPON_GEARBOX_STATUS_NGPONRXGBOXSYNCACQ
 ******************************************************************************/
const ru_field_rec NGPON_GEARBOX_STATUS_NGPONRXGBOXSYNCACQ_FIELD =
{
    "NGPONRXGBOXSYNCACQ",
#if RU_INCLUDE_DESC
    "",
    "10b sync acquired.",
#endif
    NGPON_GEARBOX_STATUS_NGPONRXGBOXSYNCACQ_FIELD_MASK,
    0,
    NGPON_GEARBOX_STATUS_NGPONRXGBOXSYNCACQ_FIELD_WIDTH,
    NGPON_GEARBOX_STATUS_NGPONRXGBOXSYNCACQ_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: NGPON_GEARBOX_STATUS_NGPONRXGBOXFIFOPTRCOL
 ******************************************************************************/
const ru_field_rec NGPON_GEARBOX_STATUS_NGPONRXGBOXFIFOPTRCOL_FIELD =
{
    "NGPONRXGBOXFIFOPTRCOL",
#if RU_INCLUDE_DESC
    "",
    "FIFO pointer collision.",
#endif
    NGPON_GEARBOX_STATUS_NGPONRXGBOXFIFOPTRCOL_FIELD_MASK,
    0,
    NGPON_GEARBOX_STATUS_NGPONRXGBOXFIFOPTRCOL_FIELD_WIDTH,
    NGPON_GEARBOX_STATUS_NGPONRXGBOXFIFOPTRCOL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: NGPON_GEARBOX_STATUS_NGPONRXGBOXCODEERRCNTSTAT
 ******************************************************************************/
const ru_field_rec NGPON_GEARBOX_STATUS_NGPONRXGBOXCODEERRCNTSTAT_FIELD =
{
    "NGPONRXGBOXCODEERRCNTSTAT",
#if RU_INCLUDE_DESC
    "",
    "Line errors.",
#endif
    NGPON_GEARBOX_STATUS_NGPONRXGBOXCODEERRCNTSTAT_FIELD_MASK,
    0,
    NGPON_GEARBOX_STATUS_NGPONRXGBOXCODEERRCNTSTAT_FIELD_WIDTH,
    NGPON_GEARBOX_STATUS_NGPONRXGBOXCODEERRCNTSTAT_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

#endif /* RU_INCLUDE_FIELD_DB */

/******************************************************************************
 * Register: NGPON_GEARBOX_RX_CTL_0
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *NGPON_GEARBOX_RX_CTL_0_FIELDS[] =
{
    &NGPON_GEARBOX_RX_CTL_0_RESERVED0_FIELD,
    &NGPON_GEARBOX_RX_CTL_0_CFNGPONGBOXRXFIFORDPTR_FIELD,
    &NGPON_GEARBOX_RX_CTL_0_CFNGPONGBOXRXPTRAUTOLDDIS_FIELD,
    &NGPON_GEARBOX_RX_CTL_0_CFNGPONGBOXRXMAXBADK_FIELD,
    &NGPON_GEARBOX_RX_CTL_0_CFNGPONGBOXRXFRMK28ONLY_FIELD,
    &NGPON_GEARBOX_RX_CTL_0_CFNGPONGBOXRXMAXGOODK_FIELD,
    &NGPON_GEARBOX_RX_CTL_0_CFNGPONGBOXRXFRCHUNT_FIELD,
    &NGPON_GEARBOX_RX_CTL_0_CFNGPONGBOXRXOUTDATAFLIP_FIELD,
    &NGPON_GEARBOX_RX_CTL_0_CFNGPONGBOXRXFRCMUXSEL_FIELD,
    &NGPON_GEARBOX_RX_CTL_0_CFNGPONGBOXRXFRCMUXVAL_FIELD,
    &NGPON_GEARBOX_RX_CTL_0_CFNGPONGBOXRX20BDATAFLIP_FIELD,
    &NGPON_GEARBOX_RX_CTL_0_CFNGPONGBOXRXSERDATAFLIP_FIELD,
    &NGPON_GEARBOX_RX_CTL_0_CFNGPONGBOXRXSERDATAINV_FIELD,
    &NGPON_GEARBOX_RX_CTL_0_CFNGPONGBOXRXFIFOPTRLD_FIELD,
    &NGPON_GEARBOX_RX_CTL_0_CFNGPONGBOXRXSWSYNCHOLD_FIELD,
    &NGPON_GEARBOX_RX_CTL_0_CFNGPONGBOXRXMODE_FIELD,
    &NGPON_GEARBOX_RX_CTL_0_CFNGPONGBOXRXEN_FIELD,
    &NGPON_GEARBOX_RX_CTL_0_CFNGPONGBOXRSTN_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec NGPON_GEARBOX_RX_CTL_0_REG = 
{
    "RX_CTL_0",
#if RU_INCLUDE_DESC
    "NGPON_GEARBOX_RX_CTL_0 Register",
    "Configuration for the NGPON gearbox.",
#endif
    NGPON_GEARBOX_RX_CTL_0_REG_OFFSET,
    0,
    0,
    5,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    18,
    NGPON_GEARBOX_RX_CTL_0_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: NGPON_GEARBOX_RX_CTL_1
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *NGPON_GEARBOX_RX_CTL_1_FIELDS[] =
{
    &NGPON_GEARBOX_RX_CTL_1_RESERVED0_FIELD,
    &NGPON_GEARBOX_RX_CTL_1_CFNGPONGBOXRXMAXTIMERCNT_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec NGPON_GEARBOX_RX_CTL_1_REG = 
{
    "RX_CTL_1",
#if RU_INCLUDE_DESC
    "NGPON_GEARBOX_RX_CTL_1 Register",
    "Configuration for the NGPON gearbox.",
#endif
    NGPON_GEARBOX_RX_CTL_1_REG_OFFSET,
    0,
    0,
    6,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    NGPON_GEARBOX_RX_CTL_1_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: NGPON_GEARBOX_RX_CTL_2
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *NGPON_GEARBOX_RX_CTL_2_FIELDS[] =
{
    &NGPON_GEARBOX_RX_CTL_2_RESERVED0_FIELD,
    &NGPON_GEARBOX_RX_CTL_2_CFNGPONGBOXRXK28D5RDP_FIELD,
    &NGPON_GEARBOX_RX_CTL_2_RESERVED1_FIELD,
    &NGPON_GEARBOX_RX_CTL_2_CFNGPONGBOXRXK28D5RDN_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec NGPON_GEARBOX_RX_CTL_2_REG = 
{
    "RX_CTL_2",
#if RU_INCLUDE_DESC
    "NGPON_GEARBOX_RX_CTL_2 Register",
    "Configuration for the NGPON gearbox.",
#endif
    NGPON_GEARBOX_RX_CTL_2_REG_OFFSET,
    0,
    0,
    7,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    4,
    NGPON_GEARBOX_RX_CTL_2_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: NGPON_GEARBOX_RX_CTL_3
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *NGPON_GEARBOX_RX_CTL_3_FIELDS[] =
{
    &NGPON_GEARBOX_RX_CTL_3_RESERVED0_FIELD,
    &NGPON_GEARBOX_RX_CTL_3_CFNGPONGBOXRXD5D7RDP_FIELD,
    &NGPON_GEARBOX_RX_CTL_3_RESERVED1_FIELD,
    &NGPON_GEARBOX_RX_CTL_3_CFNGPONGBOXRXD5D7RDN_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec NGPON_GEARBOX_RX_CTL_3_REG = 
{
    "RX_CTL_3",
#if RU_INCLUDE_DESC
    "NGPON_GEARBOX_RX_CTL_3 Register",
    "Configuration for the NGPON gearbox.",
#endif
    NGPON_GEARBOX_RX_CTL_3_REG_OFFSET,
    0,
    0,
    8,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    4,
    NGPON_GEARBOX_RX_CTL_3_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: NGPON_GEARBOX_TX_CTL
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *NGPON_GEARBOX_TX_CTL_FIELDS[] =
{
    &NGPON_GEARBOX_TX_CTL_RESERVED0_FIELD,
    &NGPON_GEARBOX_TX_CTL_CFNGPONGBOXTXFIFODATARDPTR_FIELD,
    &NGPON_GEARBOX_TX_CTL_RESERVED1_FIELD,
    &NGPON_GEARBOX_TX_CTL_CFNGPONGBOXTXFIFOVLDOFF_FIELD,
    &NGPON_GEARBOX_TX_CTL_RESERVED2_FIELD,
    &NGPON_GEARBOX_TX_CTL_CFNGPONGBOXTXSERVLDFLIP_FIELD,
    &NGPON_GEARBOX_TX_CTL_CFNGPONGBOXTXSERDATAFLIP_FIELD,
    &NGPON_GEARBOX_TX_CTL_CFNGPONGBOXTXSERVLDINV_FIELD,
    &NGPON_GEARBOX_TX_CTL_CFNGPONGBOXTXSERDATAINV_FIELD,
    &NGPON_GEARBOX_TX_CTL_CFNGPONGBOXTXFIFOVLDPTRLD_FIELD,
    &NGPON_GEARBOX_TX_CTL_CFNGPONGBOXTXFIFOPTRLD_FIELD,
    &NGPON_GEARBOX_TX_CTL_CFNGPONGBOXTXEN_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec NGPON_GEARBOX_TX_CTL_REG = 
{
    "TX_CTL",
#if RU_INCLUDE_DESC
    "NGPON_GEARBOX_TX_CTL Register",
    "Configuration for the NGPON gearbox.",
#endif
    NGPON_GEARBOX_TX_CTL_REG_OFFSET,
    0,
    0,
    9,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    12,
    NGPON_GEARBOX_TX_CTL_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: NGPON_GEARBOX_STATUS
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *NGPON_GEARBOX_STATUS_FIELDS[] =
{
    &NGPON_GEARBOX_STATUS_RESERVED0_FIELD,
    &NGPON_GEARBOX_STATUS_NGPONTXGBOXFIFOVLDPTRCOL_FIELD,
    &NGPON_GEARBOX_STATUS_NGPONRXGBOXSTATE_FIELD,
    &NGPON_GEARBOX_STATUS_NGPONTXGBOXFIFODATAPTRCOL_FIELD,
    &NGPON_GEARBOX_STATUS_NGPONRXGBOXKCNT_FIELD,
    &NGPON_GEARBOX_STATUS_NGPONRXGBOXFIFOPTRDELTA_FIELD,
    &NGPON_GEARBOX_STATUS_NGPONRXGBOXSYNCACQ_FIELD,
    &NGPON_GEARBOX_STATUS_NGPONRXGBOXFIFOPTRCOL_FIELD,
    &NGPON_GEARBOX_STATUS_NGPONRXGBOXCODEERRCNTSTAT_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec NGPON_GEARBOX_STATUS_REG = 
{
    "STATUS",
#if RU_INCLUDE_DESC
    "NGPON_GEARBOX_STATUS Register",
    "Status for the NGPON gearbox.",
#endif
    NGPON_GEARBOX_STATUS_REG_OFFSET,
    0,
    0,
    10,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    9,
    NGPON_GEARBOX_STATUS_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Block: NGPON_GEARBOX
 ******************************************************************************/
static const ru_reg_rec *NGPON_GEARBOX_REGS[] =
{
    &NGPON_GEARBOX_RX_CTL_0_REG,
    &NGPON_GEARBOX_RX_CTL_1_REG,
    &NGPON_GEARBOX_RX_CTL_2_REG,
    &NGPON_GEARBOX_RX_CTL_3_REG,
    &NGPON_GEARBOX_TX_CTL_REG,
    &NGPON_GEARBOX_STATUS_REG,
};

unsigned long NGPON_GEARBOX_ADDRS[] =
{
    0x801440d8
};

const ru_block_rec NGPON_GEARBOX_BLOCK = 
{
    "NGPON_GEARBOX",
    NGPON_GEARBOX_ADDRS,
    1,
    6,
    NGPON_GEARBOX_REGS
};

/* End of file NGPON_GEARBOX.c */
