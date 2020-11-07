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
 * Field: XPCSRX_RX_RST_RESERVED0
 ******************************************************************************/
const ru_field_rec XPCSRX_RX_RST_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    XPCSRX_RX_RST_RESERVED0_FIELD_MASK,
    0,
    XPCSRX_RX_RST_RESERVED0_FIELD_WIDTH,
    XPCSRX_RX_RST_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPCSRX_RX_RST_CFGXPCSRXCLK161RSTN
 ******************************************************************************/
const ru_field_rec XPCSRX_RX_RST_CFGXPCSRXCLK161RSTN_FIELD =
{
    "CFGXPCSRXCLK161RSTN",
#if RU_INCLUDE_DESC
    "",
    "Active low reset for 161 MHz domain in XPCS RX.",
#endif
    XPCSRX_RX_RST_CFGXPCSRXCLK161RSTN_FIELD_MASK,
    0,
    XPCSRX_RX_RST_CFGXPCSRXCLK161RSTN_FIELD_WIDTH,
    XPCSRX_RX_RST_CFGXPCSRXCLK161RSTN_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPCSRX_RX_INT_STAT_RESERVED0
 ******************************************************************************/
const ru_field_rec XPCSRX_RX_INT_STAT_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    XPCSRX_RX_INT_STAT_RESERVED0_FIELD_MASK,
    0,
    XPCSRX_RX_INT_STAT_RESERVED0_FIELD_WIDTH,
    XPCSRX_RX_INT_STAT_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPCSRX_RX_INT_STAT_INTRXIDLEDAJIT
 ******************************************************************************/
const ru_field_rec XPCSRX_RX_INT_STAT_INTRXIDLEDAJIT_FIELD =
{
    "INTRXIDLEDAJIT",
#if RU_INCLUDE_DESC
    "",
    "DA jitter detected.",
#endif
    XPCSRX_RX_INT_STAT_INTRXIDLEDAJIT_FIELD_MASK,
    0,
    XPCSRX_RX_INT_STAT_INTRXIDLEDAJIT_FIELD_WIDTH,
    XPCSRX_RX_INT_STAT_INTRXIDLEDAJIT_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPCSRX_RX_INT_STAT_INTRXFRMRMISBRST
 ******************************************************************************/
const ru_field_rec XPCSRX_RX_INT_STAT_INTRXFRMRMISBRST_FIELD =
{
    "INTRXFRMRMISBRST",
#if RU_INCLUDE_DESC
    "",
    "Missing burst detected.",
#endif
    XPCSRX_RX_INT_STAT_INTRXFRMRMISBRST_FIELD_MASK,
    0,
    XPCSRX_RX_INT_STAT_INTRXFRMRMISBRST_FIELD_WIDTH,
    XPCSRX_RX_INT_STAT_INTRXFRMRMISBRST_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPCSRX_RX_INT_STAT_INTRXIDLESOPEOPGAPBIG
 ******************************************************************************/
const ru_field_rec XPCSRX_RX_INT_STAT_INTRXIDLESOPEOPGAPBIG_FIELD =
{
    "INTRXIDLESOPEOPGAPBIG",
#if RU_INCLUDE_DESC
    "",
    "Over size packet detected.",
#endif
    XPCSRX_RX_INT_STAT_INTRXIDLESOPEOPGAPBIG_FIELD_MASK,
    0,
    XPCSRX_RX_INT_STAT_INTRXIDLESOPEOPGAPBIG_FIELD_WIDTH,
    XPCSRX_RX_INT_STAT_INTRXIDLESOPEOPGAPBIG_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPCSRX_RX_INT_STAT_INTRXIDLEFRCINS
 ******************************************************************************/
const ru_field_rec XPCSRX_RX_INT_STAT_INTRXIDLEFRCINS_FIELD =
{
    "INTRXIDLEFRCINS",
#if RU_INCLUDE_DESC
    "",
    "No idle insert opportunity in 2000 bytes. Idle insert was forced.",
#endif
    XPCSRX_RX_INT_STAT_INTRXIDLEFRCINS_FIELD_MASK,
    0,
    XPCSRX_RX_INT_STAT_INTRXIDLEFRCINS_FIELD_WIDTH,
    XPCSRX_RX_INT_STAT_INTRXIDLEFRCINS_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPCSRX_RX_INT_STAT_INTRX64B66BMINIPGERR
 ******************************************************************************/
const ru_field_rec XPCSRX_RX_INT_STAT_INTRX64B66BMINIPGERR_FIELD =
{
    "INTRX64B66BMINIPGERR",
#if RU_INCLUDE_DESC
    "",
    "Min IPG error detected.",
#endif
    XPCSRX_RX_INT_STAT_INTRX64B66BMINIPGERR_FIELD_MASK,
    0,
    XPCSRX_RX_INT_STAT_INTRX64B66BMINIPGERR_FIELD_WIDTH,
    XPCSRX_RX_INT_STAT_INTRX64B66BMINIPGERR_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPCSRX_RX_INT_STAT_INTRXFECNQUECNTNEQ
 ******************************************************************************/
const ru_field_rec XPCSRX_RX_INT_STAT_INTRXFECNQUECNTNEQ_FIELD =
{
    "INTRXFECNQUECNTNEQ",
#if RU_INCLUDE_DESC
    "",
    "FEC CW store/foward enqueue input and output count not equal.",
#endif
    XPCSRX_RX_INT_STAT_INTRXFECNQUECNTNEQ_FIELD_MASK,
    0,
    XPCSRX_RX_INT_STAT_INTRXFECNQUECNTNEQ_FIELD_WIDTH,
    XPCSRX_RX_INT_STAT_INTRXFECNQUECNTNEQ_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPCSRX_RX_INT_STAT_INTRXIDLEFIFOUNDRUN
 ******************************************************************************/
const ru_field_rec XPCSRX_RX_INT_STAT_INTRXIDLEFIFOUNDRUN_FIELD =
{
    "INTRXIDLEFIFOUNDRUN",
#if RU_INCLUDE_DESC
    "",
    "Idle insert FIFO under run. Fatal",
#endif
    XPCSRX_RX_INT_STAT_INTRXIDLEFIFOUNDRUN_FIELD_MASK,
    0,
    XPCSRX_RX_INT_STAT_INTRXIDLEFIFOUNDRUN_FIELD_WIDTH,
    XPCSRX_RX_INT_STAT_INTRXIDLEFIFOUNDRUN_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPCSRX_RX_INT_STAT_INTRXIDLEFIFOOVRRUN
 ******************************************************************************/
const ru_field_rec XPCSRX_RX_INT_STAT_INTRXIDLEFIFOOVRRUN_FIELD =
{
    "INTRXIDLEFIFOOVRRUN",
#if RU_INCLUDE_DESC
    "",
    "Idle insert FIFO over run. Fatal",
#endif
    XPCSRX_RX_INT_STAT_INTRXIDLEFIFOOVRRUN_FIELD_MASK,
    0,
    XPCSRX_RX_INT_STAT_INTRXIDLEFIFOOVRRUN_FIELD_WIDTH,
    XPCSRX_RX_INT_STAT_INTRXIDLEFIFOOVRRUN_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPCSRX_RX_INT_STAT_INTRXFECHIGHCOR
 ******************************************************************************/
const ru_field_rec XPCSRX_RX_INT_STAT_INTRXFECHIGHCOR_FIELD =
{
    "INTRXFECHIGHCOR",
#if RU_INCLUDE_DESC
    "",
    "FEC high correction alarm.  High FEC correctoin occured over"
    "cfgXPcsRxFecCorIntval",
#endif
    XPCSRX_RX_INT_STAT_INTRXFECHIGHCOR_FIELD_MASK,
    0,
    XPCSRX_RX_INT_STAT_INTRXFECHIGHCOR_FIELD_WIDTH,
    XPCSRX_RX_INT_STAT_INTRXFECHIGHCOR_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPCSRX_RX_INT_STAT_RESERVED1
 ******************************************************************************/
const ru_field_rec XPCSRX_RX_INT_STAT_RESERVED1_FIELD =
{
    "RESERVED1",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    XPCSRX_RX_INT_STAT_RESERVED1_FIELD_MASK,
    0,
    XPCSRX_RX_INT_STAT_RESERVED1_FIELD_WIDTH,
    XPCSRX_RX_INT_STAT_RESERVED1_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPCSRX_RX_INT_STAT_INTRXFECDECSTOPONERR
 ******************************************************************************/
const ru_field_rec XPCSRX_RX_INT_STAT_INTRXFECDECSTOPONERR_FIELD =
{
    "INTRXFECDECSTOPONERR",
#if RU_INCLUDE_DESC
    "",
    "FEC CW decode fail and FEC decoder is frozen per"
    "cfgXPcsRxFecStopOnErr",
#endif
    XPCSRX_RX_INT_STAT_INTRXFECDECSTOPONERR_FIELD_MASK,
    0,
    XPCSRX_RX_INT_STAT_INTRXFECDECSTOPONERR_FIELD_WIDTH,
    XPCSRX_RX_INT_STAT_INTRXFECDECSTOPONERR_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPCSRX_RX_INT_STAT_INTRXFECDECPASS
 ******************************************************************************/
const ru_field_rec XPCSRX_RX_INT_STAT_INTRXFECDECPASS_FIELD =
{
    "INTRXFECDECPASS",
#if RU_INCLUDE_DESC
    "",
    "FEC CW decode passed.",
#endif
    XPCSRX_RX_INT_STAT_INTRXFECDECPASS_FIELD_MASK,
    0,
    XPCSRX_RX_INT_STAT_INTRXFECDECPASS_FIELD_WIDTH,
    XPCSRX_RX_INT_STAT_INTRXFECDECPASS_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPCSRX_RX_INT_STAT_INTRXSTATFRMRHIGHBER
 ******************************************************************************/
const ru_field_rec XPCSRX_RX_INT_STAT_INTRXSTATFRMRHIGHBER_FIELD =
{
    "INTRXSTATFRMRHIGHBER",
#if RU_INCLUDE_DESC
    "",
    "Framer has high BER.",
#endif
    XPCSRX_RX_INT_STAT_INTRXSTATFRMRHIGHBER_FIELD_MASK,
    0,
    XPCSRX_RX_INT_STAT_INTRXSTATFRMRHIGHBER_FIELD_WIDTH,
    XPCSRX_RX_INT_STAT_INTRXSTATFRMRHIGHBER_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPCSRX_RX_INT_STAT_INTRXFRMREXITBYSP
 ******************************************************************************/
const ru_field_rec XPCSRX_RX_INT_STAT_INTRXFRMREXITBYSP_FIELD =
{
    "INTRXFRMREXITBYSP",
#if RU_INCLUDE_DESC
    "",
    "Framer exited by hitting max count on SP.",
#endif
    XPCSRX_RX_INT_STAT_INTRXFRMREXITBYSP_FIELD_MASK,
    0,
    XPCSRX_RX_INT_STAT_INTRXFRMREXITBYSP_FIELD_WIDTH,
    XPCSRX_RX_INT_STAT_INTRXFRMREXITBYSP_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPCSRX_RX_INT_STAT_INTRXFRMRBADSHMAX
 ******************************************************************************/
const ru_field_rec XPCSRX_RX_INT_STAT_INTRXFRMRBADSHMAX_FIELD =
{
    "INTRXFRMRBADSHMAX",
#if RU_INCLUDE_DESC
    "",
    "Framer hit bad SH max count.",
#endif
    XPCSRX_RX_INT_STAT_INTRXFRMRBADSHMAX_FIELD_MASK,
    0,
    XPCSRX_RX_INT_STAT_INTRXFRMRBADSHMAX_FIELD_WIDTH,
    XPCSRX_RX_INT_STAT_INTRXFRMRBADSHMAX_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPCSRX_RX_INT_STAT_INTRXDSCRAMBURSTSEQOUT
 ******************************************************************************/
const ru_field_rec XPCSRX_RX_INT_STAT_INTRXDSCRAMBURSTSEQOUT_FIELD =
{
    "INTRXDSCRAMBURSTSEQOUT",
#if RU_INCLUDE_DESC
    "",
    "Burst sequence out of order.",
#endif
    XPCSRX_RX_INT_STAT_INTRXDSCRAMBURSTSEQOUT_FIELD_MASK,
    0,
    XPCSRX_RX_INT_STAT_INTRXDSCRAMBURSTSEQOUT_FIELD_WIDTH,
    XPCSRX_RX_INT_STAT_INTRXDSCRAMBURSTSEQOUT_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPCSRX_RX_INT_STAT_INTRXTESTPSUDOLOCK
 ******************************************************************************/
const ru_field_rec XPCSRX_RX_INT_STAT_INTRXTESTPSUDOLOCK_FIELD =
{
    "INTRXTESTPSUDOLOCK",
#if RU_INCLUDE_DESC
    "",
    "Test pattern psudo lock.",
#endif
    XPCSRX_RX_INT_STAT_INTRXTESTPSUDOLOCK_FIELD_MASK,
    0,
    XPCSRX_RX_INT_STAT_INTRXTESTPSUDOLOCK_FIELD_WIDTH,
    XPCSRX_RX_INT_STAT_INTRXTESTPSUDOLOCK_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPCSRX_RX_INT_STAT_INTRXTESTPSUDOTYPE
 ******************************************************************************/
const ru_field_rec XPCSRX_RX_INT_STAT_INTRXTESTPSUDOTYPE_FIELD =
{
    "INTRXTESTPSUDOTYPE",
#if RU_INCLUDE_DESC
    "",
    "Test pattern psudo type.",
#endif
    XPCSRX_RX_INT_STAT_INTRXTESTPSUDOTYPE_FIELD_MASK,
    0,
    XPCSRX_RX_INT_STAT_INTRXTESTPSUDOTYPE_FIELD_WIDTH,
    XPCSRX_RX_INT_STAT_INTRXTESTPSUDOTYPE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPCSRX_RX_INT_STAT_INTRXTESTPSUDOERR
 ******************************************************************************/
const ru_field_rec XPCSRX_RX_INT_STAT_INTRXTESTPSUDOERR_FIELD =
{
    "INTRXTESTPSUDOERR",
#if RU_INCLUDE_DESC
    "",
    "Test pattern psudo error.",
#endif
    XPCSRX_RX_INT_STAT_INTRXTESTPSUDOERR_FIELD_MASK,
    0,
    XPCSRX_RX_INT_STAT_INTRXTESTPSUDOERR_FIELD_WIDTH,
    XPCSRX_RX_INT_STAT_INTRXTESTPSUDOERR_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPCSRX_RX_INT_STAT_INTRXTESTPRBSLOCK
 ******************************************************************************/
const ru_field_rec XPCSRX_RX_INT_STAT_INTRXTESTPRBSLOCK_FIELD =
{
    "INTRXTESTPRBSLOCK",
#if RU_INCLUDE_DESC
    "",
    "Test pattern PRBS lock.",
#endif
    XPCSRX_RX_INT_STAT_INTRXTESTPRBSLOCK_FIELD_MASK,
    0,
    XPCSRX_RX_INT_STAT_INTRXTESTPRBSLOCK_FIELD_WIDTH,
    XPCSRX_RX_INT_STAT_INTRXTESTPRBSLOCK_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPCSRX_RX_INT_STAT_INTRXTESTPRBSERR
 ******************************************************************************/
const ru_field_rec XPCSRX_RX_INT_STAT_INTRXTESTPRBSERR_FIELD =
{
    "INTRXTESTPRBSERR",
#if RU_INCLUDE_DESC
    "",
    "Test pattern PRBS error.",
#endif
    XPCSRX_RX_INT_STAT_INTRXTESTPRBSERR_FIELD_MASK,
    0,
    XPCSRX_RX_INT_STAT_INTRXTESTPRBSERR_FIELD_WIDTH,
    XPCSRX_RX_INT_STAT_INTRXTESTPRBSERR_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPCSRX_RX_INT_STAT_INTRXFECPSISTDECFAIL
 ******************************************************************************/
const ru_field_rec XPCSRX_RX_INT_STAT_INTRXFECPSISTDECFAIL_FIELD =
{
    "INTRXFECPSISTDECFAIL",
#if RU_INCLUDE_DESC
    "",
    "Three consecative failed FEC CW decode.",
#endif
    XPCSRX_RX_INT_STAT_INTRXFECPSISTDECFAIL_FIELD_MASK,
    0,
    XPCSRX_RX_INT_STAT_INTRXFECPSISTDECFAIL_FIELD_WIDTH,
    XPCSRX_RX_INT_STAT_INTRXFECPSISTDECFAIL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPCSRX_RX_INT_STAT_INTRXFRAMERBADSH
 ******************************************************************************/
const ru_field_rec XPCSRX_RX_INT_STAT_INTRXFRAMERBADSH_FIELD =
{
    "INTRXFRAMERBADSH",
#if RU_INCLUDE_DESC
    "",
    "Framer detected bad SH while in lock.",
#endif
    XPCSRX_RX_INT_STAT_INTRXFRAMERBADSH_FIELD_MASK,
    0,
    XPCSRX_RX_INT_STAT_INTRXFRAMERBADSH_FIELD_WIDTH,
    XPCSRX_RX_INT_STAT_INTRXFRAMERBADSH_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPCSRX_RX_INT_STAT_INTRXFRAMERCWLOSS
 ******************************************************************************/
const ru_field_rec XPCSRX_RX_INT_STAT_INTRXFRAMERCWLOSS_FIELD =
{
    "INTRXFRAMERCWLOSS",
#if RU_INCLUDE_DESC
    "",
    "Framer went into loss.",
#endif
    XPCSRX_RX_INT_STAT_INTRXFRAMERCWLOSS_FIELD_MASK,
    0,
    XPCSRX_RX_INT_STAT_INTRXFRAMERCWLOSS_FIELD_WIDTH,
    XPCSRX_RX_INT_STAT_INTRXFRAMERCWLOSS_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPCSRX_RX_INT_STAT_INTRXFRAMERCWLOCK
 ******************************************************************************/
const ru_field_rec XPCSRX_RX_INT_STAT_INTRXFRAMERCWLOCK_FIELD =
{
    "INTRXFRAMERCWLOCK",
#if RU_INCLUDE_DESC
    "",
    "Framer went into lock.",
#endif
    XPCSRX_RX_INT_STAT_INTRXFRAMERCWLOCK_FIELD_MASK,
    0,
    XPCSRX_RX_INT_STAT_INTRXFRAMERCWLOCK_FIELD_WIDTH,
    XPCSRX_RX_INT_STAT_INTRXFRAMERCWLOCK_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPCSRX_RX_INT_STAT_INTRXFECDECFAIL
 ******************************************************************************/
const ru_field_rec XPCSRX_RX_INT_STAT_INTRXFECDECFAIL_FIELD =
{
    "INTRXFECDECFAIL",
#if RU_INCLUDE_DESC
    "",
    "FEC CW decode failed.",
#endif
    XPCSRX_RX_INT_STAT_INTRXFECDECFAIL_FIELD_MASK,
    0,
    XPCSRX_RX_INT_STAT_INTRXFECDECFAIL_FIELD_WIDTH,
    XPCSRX_RX_INT_STAT_INTRXFECDECFAIL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPCSRX_RX_INT_STAT_INTRX64B66BDECERR
 ******************************************************************************/
const ru_field_rec XPCSRX_RX_INT_STAT_INTRX64B66BDECERR_FIELD =
{
    "INTRX64B66BDECERR",
#if RU_INCLUDE_DESC
    "",
    "66b to 64b decode error has occured.",
#endif
    XPCSRX_RX_INT_STAT_INTRX64B66BDECERR_FIELD_MASK,
    0,
    XPCSRX_RX_INT_STAT_INTRX64B66BDECERR_FIELD_WIDTH,
    XPCSRX_RX_INT_STAT_INTRX64B66BDECERR_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPCSRX_RX_INT_STAT_INTRXFRMRNOLOCKLOS
 ******************************************************************************/
const ru_field_rec XPCSRX_RX_INT_STAT_INTRXFRMRNOLOCKLOS_FIELD =
{
    "INTRXFRMRNOLOCKLOS",
#if RU_INCLUDE_DESC
    "",
    "No frmrCwLk before the no lock timer expired.",
#endif
    XPCSRX_RX_INT_STAT_INTRXFRMRNOLOCKLOS_FIELD_MASK,
    0,
    XPCSRX_RX_INT_STAT_INTRXFRMRNOLOCKLOS_FIELD_WIDTH,
    XPCSRX_RX_INT_STAT_INTRXFRMRNOLOCKLOS_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPCSRX_RX_INT_STAT_INTRXFRMRROGUE
 ******************************************************************************/
const ru_field_rec XPCSRX_RX_INT_STAT_INTRXFRMRROGUE_FIELD =
{
    "INTRXFRMRROGUE",
#if RU_INCLUDE_DESC
    "",
    "SP count hit max in ranging but no lock",
#endif
    XPCSRX_RX_INT_STAT_INTRXFRMRROGUE_FIELD_MASK,
    0,
    XPCSRX_RX_INT_STAT_INTRXFRMRROGUE_FIELD_WIDTH,
    XPCSRX_RX_INT_STAT_INTRXFRMRROGUE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPCSRX_RX_INT_STAT_INT_REGS_ERR
 ******************************************************************************/
const ru_field_rec XPCSRX_RX_INT_STAT_INT_REGS_ERR_FIELD =
{
    "INT_REGS_ERR",
#if RU_INCLUDE_DESC
    "",
    "register access error has occured.",
#endif
    XPCSRX_RX_INT_STAT_INT_REGS_ERR_FIELD_MASK,
    0,
    XPCSRX_RX_INT_STAT_INT_REGS_ERR_FIELD_WIDTH,
    XPCSRX_RX_INT_STAT_INT_REGS_ERR_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPCSRX_RX_INT_MSK_RESERVED0
 ******************************************************************************/
const ru_field_rec XPCSRX_RX_INT_MSK_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    XPCSRX_RX_INT_MSK_RESERVED0_FIELD_MASK,
    0,
    XPCSRX_RX_INT_MSK_RESERVED0_FIELD_WIDTH,
    XPCSRX_RX_INT_MSK_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPCSRX_RX_INT_MSK_MSKRXIDLEDAJIT
 ******************************************************************************/
const ru_field_rec XPCSRX_RX_INT_MSK_MSKRXIDLEDAJIT_FIELD =
{
    "MSKRXIDLEDAJIT",
#if RU_INCLUDE_DESC
    "",
    "0 - Interrupt is masked. 1 - Interrupt is unmasked.",
#endif
    XPCSRX_RX_INT_MSK_MSKRXIDLEDAJIT_FIELD_MASK,
    0,
    XPCSRX_RX_INT_MSK_MSKRXIDLEDAJIT_FIELD_WIDTH,
    XPCSRX_RX_INT_MSK_MSKRXIDLEDAJIT_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPCSRX_RX_INT_MSK_MSKRXFRMRMISBRST
 ******************************************************************************/
const ru_field_rec XPCSRX_RX_INT_MSK_MSKRXFRMRMISBRST_FIELD =
{
    "MSKRXFRMRMISBRST",
#if RU_INCLUDE_DESC
    "",
    "0 - Interrupt is masked. 1 - Interrupt is unmasked.",
#endif
    XPCSRX_RX_INT_MSK_MSKRXFRMRMISBRST_FIELD_MASK,
    0,
    XPCSRX_RX_INT_MSK_MSKRXFRMRMISBRST_FIELD_WIDTH,
    XPCSRX_RX_INT_MSK_MSKRXFRMRMISBRST_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPCSRX_RX_INT_MSK_MSKRXIDLESOPEOPGAPBIG
 ******************************************************************************/
const ru_field_rec XPCSRX_RX_INT_MSK_MSKRXIDLESOPEOPGAPBIG_FIELD =
{
    "MSKRXIDLESOPEOPGAPBIG",
#if RU_INCLUDE_DESC
    "",
    "0 - Interrupt is masked. 1 - Interrupt is unmasked.",
#endif
    XPCSRX_RX_INT_MSK_MSKRXIDLESOPEOPGAPBIG_FIELD_MASK,
    0,
    XPCSRX_RX_INT_MSK_MSKRXIDLESOPEOPGAPBIG_FIELD_WIDTH,
    XPCSRX_RX_INT_MSK_MSKRXIDLESOPEOPGAPBIG_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPCSRX_RX_INT_MSK_MSKRXIDLEFRCINS
 ******************************************************************************/
const ru_field_rec XPCSRX_RX_INT_MSK_MSKRXIDLEFRCINS_FIELD =
{
    "MSKRXIDLEFRCINS",
#if RU_INCLUDE_DESC
    "",
    "0 - Interrupt is masked. 1 - Interrupt is unmasked.",
#endif
    XPCSRX_RX_INT_MSK_MSKRXIDLEFRCINS_FIELD_MASK,
    0,
    XPCSRX_RX_INT_MSK_MSKRXIDLEFRCINS_FIELD_WIDTH,
    XPCSRX_RX_INT_MSK_MSKRXIDLEFRCINS_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPCSRX_RX_INT_MSK_MSKRX64B66BMINIPGERR
 ******************************************************************************/
const ru_field_rec XPCSRX_RX_INT_MSK_MSKRX64B66BMINIPGERR_FIELD =
{
    "MSKRX64B66BMINIPGERR",
#if RU_INCLUDE_DESC
    "",
    "0 - Interrupt is masked. 1 - Interrupt is unmasked.",
#endif
    XPCSRX_RX_INT_MSK_MSKRX64B66BMINIPGERR_FIELD_MASK,
    0,
    XPCSRX_RX_INT_MSK_MSKRX64B66BMINIPGERR_FIELD_WIDTH,
    XPCSRX_RX_INT_MSK_MSKRX64B66BMINIPGERR_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPCSRX_RX_INT_MSK_MSKRXFECNQUECNTNEQ
 ******************************************************************************/
const ru_field_rec XPCSRX_RX_INT_MSK_MSKRXFECNQUECNTNEQ_FIELD =
{
    "MSKRXFECNQUECNTNEQ",
#if RU_INCLUDE_DESC
    "",
    "0 - Interrupt is masked. 1 - Interrupt is unmasked.",
#endif
    XPCSRX_RX_INT_MSK_MSKRXFECNQUECNTNEQ_FIELD_MASK,
    0,
    XPCSRX_RX_INT_MSK_MSKRXFECNQUECNTNEQ_FIELD_WIDTH,
    XPCSRX_RX_INT_MSK_MSKRXFECNQUECNTNEQ_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPCSRX_RX_INT_MSK_MSKRXIDLEFIFOUNDRUN
 ******************************************************************************/
const ru_field_rec XPCSRX_RX_INT_MSK_MSKRXIDLEFIFOUNDRUN_FIELD =
{
    "MSKRXIDLEFIFOUNDRUN",
#if RU_INCLUDE_DESC
    "",
    "0 - Interrupt is masked. 1 - Interrupt is unmasked.",
#endif
    XPCSRX_RX_INT_MSK_MSKRXIDLEFIFOUNDRUN_FIELD_MASK,
    0,
    XPCSRX_RX_INT_MSK_MSKRXIDLEFIFOUNDRUN_FIELD_WIDTH,
    XPCSRX_RX_INT_MSK_MSKRXIDLEFIFOUNDRUN_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPCSRX_RX_INT_MSK_MSKRXIDLEFIFOOVRRUN
 ******************************************************************************/
const ru_field_rec XPCSRX_RX_INT_MSK_MSKRXIDLEFIFOOVRRUN_FIELD =
{
    "MSKRXIDLEFIFOOVRRUN",
#if RU_INCLUDE_DESC
    "",
    "0 - Interrupt is masked. 1 - Interrupt is unmasked.",
#endif
    XPCSRX_RX_INT_MSK_MSKRXIDLEFIFOOVRRUN_FIELD_MASK,
    0,
    XPCSRX_RX_INT_MSK_MSKRXIDLEFIFOOVRRUN_FIELD_WIDTH,
    XPCSRX_RX_INT_MSK_MSKRXIDLEFIFOOVRRUN_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPCSRX_RX_INT_MSK_MSKRXFECHIGHCOR
 ******************************************************************************/
const ru_field_rec XPCSRX_RX_INT_MSK_MSKRXFECHIGHCOR_FIELD =
{
    "MSKRXFECHIGHCOR",
#if RU_INCLUDE_DESC
    "",
    "0 - Interrupt is masked. 1 - Interrupt is unmasked.",
#endif
    XPCSRX_RX_INT_MSK_MSKRXFECHIGHCOR_FIELD_MASK,
    0,
    XPCSRX_RX_INT_MSK_MSKRXFECHIGHCOR_FIELD_WIDTH,
    XPCSRX_RX_INT_MSK_MSKRXFECHIGHCOR_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPCSRX_RX_INT_MSK_RESERVED1
 ******************************************************************************/
const ru_field_rec XPCSRX_RX_INT_MSK_RESERVED1_FIELD =
{
    "RESERVED1",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    XPCSRX_RX_INT_MSK_RESERVED1_FIELD_MASK,
    0,
    XPCSRX_RX_INT_MSK_RESERVED1_FIELD_WIDTH,
    XPCSRX_RX_INT_MSK_RESERVED1_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPCSRX_RX_INT_MSK_MSKRXFECDECSTOPONERR
 ******************************************************************************/
const ru_field_rec XPCSRX_RX_INT_MSK_MSKRXFECDECSTOPONERR_FIELD =
{
    "MSKRXFECDECSTOPONERR",
#if RU_INCLUDE_DESC
    "",
    "0 - Interrupt is masked. 1 - Interrupt is unmasked.",
#endif
    XPCSRX_RX_INT_MSK_MSKRXFECDECSTOPONERR_FIELD_MASK,
    0,
    XPCSRX_RX_INT_MSK_MSKRXFECDECSTOPONERR_FIELD_WIDTH,
    XPCSRX_RX_INT_MSK_MSKRXFECDECSTOPONERR_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPCSRX_RX_INT_MSK_MSKRXFECDECPASS
 ******************************************************************************/
const ru_field_rec XPCSRX_RX_INT_MSK_MSKRXFECDECPASS_FIELD =
{
    "MSKRXFECDECPASS",
#if RU_INCLUDE_DESC
    "",
    "0 - Interrupt is masked. 1 - Interrupt is unmasked.",
#endif
    XPCSRX_RX_INT_MSK_MSKRXFECDECPASS_FIELD_MASK,
    0,
    XPCSRX_RX_INT_MSK_MSKRXFECDECPASS_FIELD_WIDTH,
    XPCSRX_RX_INT_MSK_MSKRXFECDECPASS_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPCSRX_RX_INT_MSK_MSKRXSTATFRMRHIGHBER
 ******************************************************************************/
const ru_field_rec XPCSRX_RX_INT_MSK_MSKRXSTATFRMRHIGHBER_FIELD =
{
    "MSKRXSTATFRMRHIGHBER",
#if RU_INCLUDE_DESC
    "",
    "0 - Interrupt is masked. 1 - Interrupt is unmasked.",
#endif
    XPCSRX_RX_INT_MSK_MSKRXSTATFRMRHIGHBER_FIELD_MASK,
    0,
    XPCSRX_RX_INT_MSK_MSKRXSTATFRMRHIGHBER_FIELD_WIDTH,
    XPCSRX_RX_INT_MSK_MSKRXSTATFRMRHIGHBER_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPCSRX_RX_INT_MSK_MSKRXFRMREXITBYSP
 ******************************************************************************/
const ru_field_rec XPCSRX_RX_INT_MSK_MSKRXFRMREXITBYSP_FIELD =
{
    "MSKRXFRMREXITBYSP",
#if RU_INCLUDE_DESC
    "",
    "0 - Interrupt is masked. 1 - Interrupt is unmasked.",
#endif
    XPCSRX_RX_INT_MSK_MSKRXFRMREXITBYSP_FIELD_MASK,
    0,
    XPCSRX_RX_INT_MSK_MSKRXFRMREXITBYSP_FIELD_WIDTH,
    XPCSRX_RX_INT_MSK_MSKRXFRMREXITBYSP_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPCSRX_RX_INT_MSK_MSKRXFRMRBADSHMAX
 ******************************************************************************/
const ru_field_rec XPCSRX_RX_INT_MSK_MSKRXFRMRBADSHMAX_FIELD =
{
    "MSKRXFRMRBADSHMAX",
#if RU_INCLUDE_DESC
    "",
    "0 - Interrupt is masked. 1 - Interrupt is unmasked.",
#endif
    XPCSRX_RX_INT_MSK_MSKRXFRMRBADSHMAX_FIELD_MASK,
    0,
    XPCSRX_RX_INT_MSK_MSKRXFRMRBADSHMAX_FIELD_WIDTH,
    XPCSRX_RX_INT_MSK_MSKRXFRMRBADSHMAX_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPCSRX_RX_INT_MSK_MSKRXDSCRAMBURSTSEQOUT
 ******************************************************************************/
const ru_field_rec XPCSRX_RX_INT_MSK_MSKRXDSCRAMBURSTSEQOUT_FIELD =
{
    "MSKRXDSCRAMBURSTSEQOUT",
#if RU_INCLUDE_DESC
    "",
    "0 - Interrupt is masked. 1 - Interrupt is unmasked.",
#endif
    XPCSRX_RX_INT_MSK_MSKRXDSCRAMBURSTSEQOUT_FIELD_MASK,
    0,
    XPCSRX_RX_INT_MSK_MSKRXDSCRAMBURSTSEQOUT_FIELD_WIDTH,
    XPCSRX_RX_INT_MSK_MSKRXDSCRAMBURSTSEQOUT_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPCSRX_RX_INT_MSK_MSKRXTESTPSUDOLOCK
 ******************************************************************************/
const ru_field_rec XPCSRX_RX_INT_MSK_MSKRXTESTPSUDOLOCK_FIELD =
{
    "MSKRXTESTPSUDOLOCK",
#if RU_INCLUDE_DESC
    "",
    "0 - Interrupt is masked. 1 - Interrupt is unmasked.",
#endif
    XPCSRX_RX_INT_MSK_MSKRXTESTPSUDOLOCK_FIELD_MASK,
    0,
    XPCSRX_RX_INT_MSK_MSKRXTESTPSUDOLOCK_FIELD_WIDTH,
    XPCSRX_RX_INT_MSK_MSKRXTESTPSUDOLOCK_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPCSRX_RX_INT_MSK_MSKRXTESTPSUDOTYPE
 ******************************************************************************/
const ru_field_rec XPCSRX_RX_INT_MSK_MSKRXTESTPSUDOTYPE_FIELD =
{
    "MSKRXTESTPSUDOTYPE",
#if RU_INCLUDE_DESC
    "",
    "0 - Interrupt is masked. 1 - Interrupt is unmasked.",
#endif
    XPCSRX_RX_INT_MSK_MSKRXTESTPSUDOTYPE_FIELD_MASK,
    0,
    XPCSRX_RX_INT_MSK_MSKRXTESTPSUDOTYPE_FIELD_WIDTH,
    XPCSRX_RX_INT_MSK_MSKRXTESTPSUDOTYPE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPCSRX_RX_INT_MSK_MSKRXTESTPSUDOERR
 ******************************************************************************/
const ru_field_rec XPCSRX_RX_INT_MSK_MSKRXTESTPSUDOERR_FIELD =
{
    "MSKRXTESTPSUDOERR",
#if RU_INCLUDE_DESC
    "",
    "0 - Interrupt is masked. 1 - Interrupt is unmasked.",
#endif
    XPCSRX_RX_INT_MSK_MSKRXTESTPSUDOERR_FIELD_MASK,
    0,
    XPCSRX_RX_INT_MSK_MSKRXTESTPSUDOERR_FIELD_WIDTH,
    XPCSRX_RX_INT_MSK_MSKRXTESTPSUDOERR_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPCSRX_RX_INT_MSK_MSKRXTESTPRBSLOCK
 ******************************************************************************/
const ru_field_rec XPCSRX_RX_INT_MSK_MSKRXTESTPRBSLOCK_FIELD =
{
    "MSKRXTESTPRBSLOCK",
#if RU_INCLUDE_DESC
    "",
    "0 - Interrupt is masked. 1 - Interrupt is unmasked.",
#endif
    XPCSRX_RX_INT_MSK_MSKRXTESTPRBSLOCK_FIELD_MASK,
    0,
    XPCSRX_RX_INT_MSK_MSKRXTESTPRBSLOCK_FIELD_WIDTH,
    XPCSRX_RX_INT_MSK_MSKRXTESTPRBSLOCK_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPCSRX_RX_INT_MSK_MSKRXTESTPRBSERR
 ******************************************************************************/
const ru_field_rec XPCSRX_RX_INT_MSK_MSKRXTESTPRBSERR_FIELD =
{
    "MSKRXTESTPRBSERR",
#if RU_INCLUDE_DESC
    "",
    "0 - Interrupt is masked. 1 - Interrupt is unmasked.",
#endif
    XPCSRX_RX_INT_MSK_MSKRXTESTPRBSERR_FIELD_MASK,
    0,
    XPCSRX_RX_INT_MSK_MSKRXTESTPRBSERR_FIELD_WIDTH,
    XPCSRX_RX_INT_MSK_MSKRXTESTPRBSERR_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPCSRX_RX_INT_MSK_MSKRXFECPSISTDECFAIL
 ******************************************************************************/
const ru_field_rec XPCSRX_RX_INT_MSK_MSKRXFECPSISTDECFAIL_FIELD =
{
    "MSKRXFECPSISTDECFAIL",
#if RU_INCLUDE_DESC
    "",
    "0 - Interrupt is masked. 1 - Interrupt is unmasked.",
#endif
    XPCSRX_RX_INT_MSK_MSKRXFECPSISTDECFAIL_FIELD_MASK,
    0,
    XPCSRX_RX_INT_MSK_MSKRXFECPSISTDECFAIL_FIELD_WIDTH,
    XPCSRX_RX_INT_MSK_MSKRXFECPSISTDECFAIL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPCSRX_RX_INT_MSK_MSKRXFRAMERBADSH
 ******************************************************************************/
const ru_field_rec XPCSRX_RX_INT_MSK_MSKRXFRAMERBADSH_FIELD =
{
    "MSKRXFRAMERBADSH",
#if RU_INCLUDE_DESC
    "",
    "0 - Interrupt is masked. 1 - Interrupt is unmasked.",
#endif
    XPCSRX_RX_INT_MSK_MSKRXFRAMERBADSH_FIELD_MASK,
    0,
    XPCSRX_RX_INT_MSK_MSKRXFRAMERBADSH_FIELD_WIDTH,
    XPCSRX_RX_INT_MSK_MSKRXFRAMERBADSH_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPCSRX_RX_INT_MSK_MSKRXFRAMERCWLOSS
 ******************************************************************************/
const ru_field_rec XPCSRX_RX_INT_MSK_MSKRXFRAMERCWLOSS_FIELD =
{
    "MSKRXFRAMERCWLOSS",
#if RU_INCLUDE_DESC
    "",
    "0 - Interrupt is masked. 1 - Interrupt is unmasked.",
#endif
    XPCSRX_RX_INT_MSK_MSKRXFRAMERCWLOSS_FIELD_MASK,
    0,
    XPCSRX_RX_INT_MSK_MSKRXFRAMERCWLOSS_FIELD_WIDTH,
    XPCSRX_RX_INT_MSK_MSKRXFRAMERCWLOSS_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPCSRX_RX_INT_MSK_MSKRXFRAMERCWLOCK
 ******************************************************************************/
const ru_field_rec XPCSRX_RX_INT_MSK_MSKRXFRAMERCWLOCK_FIELD =
{
    "MSKRXFRAMERCWLOCK",
#if RU_INCLUDE_DESC
    "",
    "0 - Interrupt is masked. 1 - Interrupt is unmasked.",
#endif
    XPCSRX_RX_INT_MSK_MSKRXFRAMERCWLOCK_FIELD_MASK,
    0,
    XPCSRX_RX_INT_MSK_MSKRXFRAMERCWLOCK_FIELD_WIDTH,
    XPCSRX_RX_INT_MSK_MSKRXFRAMERCWLOCK_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPCSRX_RX_INT_MSK_MSKRXFECDECFAIL
 ******************************************************************************/
const ru_field_rec XPCSRX_RX_INT_MSK_MSKRXFECDECFAIL_FIELD =
{
    "MSKRXFECDECFAIL",
#if RU_INCLUDE_DESC
    "",
    "0 - Interrupt is masked. 1 - Interrupt is unmasked.",
#endif
    XPCSRX_RX_INT_MSK_MSKRXFECDECFAIL_FIELD_MASK,
    0,
    XPCSRX_RX_INT_MSK_MSKRXFECDECFAIL_FIELD_WIDTH,
    XPCSRX_RX_INT_MSK_MSKRXFECDECFAIL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPCSRX_RX_INT_MSK_MSKRX64B66BDECERR
 ******************************************************************************/
const ru_field_rec XPCSRX_RX_INT_MSK_MSKRX64B66BDECERR_FIELD =
{
    "MSKRX64B66BDECERR",
#if RU_INCLUDE_DESC
    "",
    "0 - Interrupt is masked. 1 - Interrupt is unmasked.",
#endif
    XPCSRX_RX_INT_MSK_MSKRX64B66BDECERR_FIELD_MASK,
    0,
    XPCSRX_RX_INT_MSK_MSKRX64B66BDECERR_FIELD_WIDTH,
    XPCSRX_RX_INT_MSK_MSKRX64B66BDECERR_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPCSRX_RX_INT_MSK_MSKRXFRMRNOLOCKLOS
 ******************************************************************************/
const ru_field_rec XPCSRX_RX_INT_MSK_MSKRXFRMRNOLOCKLOS_FIELD =
{
    "MSKRXFRMRNOLOCKLOS",
#if RU_INCLUDE_DESC
    "",
    "0 - Interrupt is masked. 1 - Interrupt is unmasked.",
#endif
    XPCSRX_RX_INT_MSK_MSKRXFRMRNOLOCKLOS_FIELD_MASK,
    0,
    XPCSRX_RX_INT_MSK_MSKRXFRMRNOLOCKLOS_FIELD_WIDTH,
    XPCSRX_RX_INT_MSK_MSKRXFRMRNOLOCKLOS_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPCSRX_RX_INT_MSK_MSKRXFRMRROGUE
 ******************************************************************************/
const ru_field_rec XPCSRX_RX_INT_MSK_MSKRXFRMRROGUE_FIELD =
{
    "MSKRXFRMRROGUE",
#if RU_INCLUDE_DESC
    "",
    "0 - Interrupt is masked. 1 - Interrupt is unmasked.",
#endif
    XPCSRX_RX_INT_MSK_MSKRXFRMRROGUE_FIELD_MASK,
    0,
    XPCSRX_RX_INT_MSK_MSKRXFRMRROGUE_FIELD_WIDTH,
    XPCSRX_RX_INT_MSK_MSKRXFRMRROGUE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPCSRX_RX_INT_MSK_MSK_INT_REGS_ERR
 ******************************************************************************/
const ru_field_rec XPCSRX_RX_INT_MSK_MSK_INT_REGS_ERR_FIELD =
{
    "MSK_INT_REGS_ERR",
#if RU_INCLUDE_DESC
    "",
    "0 - Interrupt is masked. 1 - Interrupt is unmasked.",
#endif
    XPCSRX_RX_INT_MSK_MSK_INT_REGS_ERR_FIELD_MASK,
    0,
    XPCSRX_RX_INT_MSK_MSK_INT_REGS_ERR_FIELD_WIDTH,
    XPCSRX_RX_INT_MSK_MSK_INT_REGS_ERR_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPCSRX_RX_FRAMER_CTL_RESERVED0
 ******************************************************************************/
const ru_field_rec XPCSRX_RX_FRAMER_CTL_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    XPCSRX_RX_FRAMER_CTL_RESERVED0_FIELD_MASK,
    0,
    XPCSRX_RX_FRAMER_CTL_RESERVED0_FIELD_WIDTH,
    XPCSRX_RX_FRAMER_CTL_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPCSRX_RX_FRAMER_CTL_CFGXPCSRXFRMRFRCEARLYALIGN
 ******************************************************************************/
const ru_field_rec XPCSRX_RX_FRAMER_CTL_CFGXPCSRXFRMRFRCEARLYALIGN_FIELD =
{
    "CFGXPCSRXFRMRFRCEARLYALIGN",
#if RU_INCLUDE_DESC
    "",
    "1 = block loading ofr alingMux0Sel when spLkCntMax achievedThis was"
    "the ECO or PIONEER B2",
#endif
    XPCSRX_RX_FRAMER_CTL_CFGXPCSRXFRMRFRCEARLYALIGN_FIELD_MASK,
    0,
    XPCSRX_RX_FRAMER_CTL_CFGXPCSRXFRMRFRCEARLYALIGN_FIELD_WIDTH,
    XPCSRX_RX_FRAMER_CTL_CFGXPCSRXFRMRFRCEARLYALIGN_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPCSRX_RX_FRAMER_CTL_CFGXPCSRXFRMRMODEA
 ******************************************************************************/
const ru_field_rec XPCSRX_RX_FRAMER_CTL_CFGXPCSRXFRMRMODEA_FIELD =
{
    "CFGXPCSRXFRMRMODEA",
#if RU_INCLUDE_DESC
    "",
    "0 = use framing mode for mux select = alignMux0SelQ for"
    "bdEbdAlignedData1 = use framing mode for mux select = alignMux0SelQQ"
    "for bdEbdAlignedData",
#endif
    XPCSRX_RX_FRAMER_CTL_CFGXPCSRXFRMRMODEA_FIELD_MASK,
    0,
    XPCSRX_RX_FRAMER_CTL_CFGXPCSRXFRMRMODEA_FIELD_WIDTH,
    XPCSRX_RX_FRAMER_CTL_CFGXPCSRXFRMRMODEA_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPCSRX_RX_FRAMER_CTL_CFGXPCSRXFRMROVERLAPBDEBDZERO
 ******************************************************************************/
const ru_field_rec XPCSRX_RX_FRAMER_CTL_CFGXPCSRXFRMROVERLAPBDEBDZERO_FIELD =
{
    "CFGXPCSRXFRMROVERLAPBDEBDZERO",
#if RU_INCLUDE_DESC
    "",
    "Allows framer to not require any space between EBD and BD fater"
    "framing once during overlaping grants."
    "cfgXPcsRxFrmrOverlapGntEn must be set to use this bit.",
#endif
    XPCSRX_RX_FRAMER_CTL_CFGXPCSRXFRMROVERLAPBDEBDZERO_FIELD_MASK,
    0,
    XPCSRX_RX_FRAMER_CTL_CFGXPCSRXFRMROVERLAPBDEBDZERO_FIELD_WIDTH,
    XPCSRX_RX_FRAMER_CTL_CFGXPCSRXFRMROVERLAPBDEBDZERO_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPCSRX_RX_FRAMER_CTL_CFGXPCSRXFRMROVERLAPGNTEN
 ******************************************************************************/
const ru_field_rec XPCSRX_RX_FRAMER_CTL_CFGXPCSRXFRMROVERLAPGNTEN_FIELD =
{
    "CFGXPCSRXFRMROVERLAPGNTEN",
#if RU_INCLUDE_DESC
    "",
    "Allows framer to not require spLkCntMax (look for BD without"
    "preceeding SP) after framing once during overlaping grants.",
#endif
    XPCSRX_RX_FRAMER_CTL_CFGXPCSRXFRMROVERLAPGNTEN_FIELD_MASK,
    0,
    XPCSRX_RX_FRAMER_CTL_CFGXPCSRXFRMROVERLAPGNTEN_FIELD_WIDTH,
    XPCSRX_RX_FRAMER_CTL_CFGXPCSRXFRMROVERLAPGNTEN_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPCSRX_RX_FRAMER_CTL_RESERVED1
 ******************************************************************************/
const ru_field_rec XPCSRX_RX_FRAMER_CTL_RESERVED1_FIELD =
{
    "RESERVED1",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    XPCSRX_RX_FRAMER_CTL_RESERVED1_FIELD_MASK,
    0,
    XPCSRX_RX_FRAMER_CTL_RESERVED1_FIELD_WIDTH,
    XPCSRX_RX_FRAMER_CTL_RESERVED1_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPCSRX_RX_FRAMER_CTL_CFGXPCSRXFRAMEBURSTOLDALIGN
 ******************************************************************************/
const ru_field_rec XPCSRX_RX_FRAMER_CTL_CFGXPCSRXFRAMEBURSTOLDALIGN_FIELD =
{
    "CFGXPCSRXFRAMEBURSTOLDALIGN",
#if RU_INCLUDE_DESC
    "",
    "Enable for burst mode framing using old alignment (xsbiPcsRxFifoVld"
    "instead of xsbiPcsRxFifoVldQ) for alignMuxSelQ and alignMuzSelQQ.",
#endif
    XPCSRX_RX_FRAMER_CTL_CFGXPCSRXFRAMEBURSTOLDALIGN_FIELD_MASK,
    0,
    XPCSRX_RX_FRAMER_CTL_CFGXPCSRXFRAMEBURSTOLDALIGN_FIELD_WIDTH,
    XPCSRX_RX_FRAMER_CTL_CFGXPCSRXFRAMEBURSTOLDALIGN_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPCSRX_RX_FRAMER_CTL_CFGXPCSRXFRMRMISBRSTTYPE
 ******************************************************************************/
const ru_field_rec XPCSRX_RX_FRAMER_CTL_CFGXPCSRXFRMRMISBRSTTYPE_FIELD =
{
    "CFGXPCSRXFRMRMISBRSTTYPE",
#if RU_INCLUDE_DESC
    "",
    "0 - Use falling edge of SP count max and no framer lock to count"
    "missing burst. 1 - Use unassigned strobe for detection of missing"
    "burst.",
#endif
    XPCSRX_RX_FRAMER_CTL_CFGXPCSRXFRMRMISBRSTTYPE_FIELD_MASK,
    0,
    XPCSRX_RX_FRAMER_CTL_CFGXPCSRXFRMRMISBRSTTYPE_FIELD_WIDTH,
    XPCSRX_RX_FRAMER_CTL_CFGXPCSRXFRMRMISBRSTTYPE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPCSRX_RX_FRAMER_CTL_CFGXPCSRXFRMREBDVLDEN
 ******************************************************************************/
const ru_field_rec XPCSRX_RX_FRAMER_CTL_CFGXPCSRXFRMREBDVLDEN_FIELD =
{
    "CFGXPCSRXFRMREBDVLDEN",
#if RU_INCLUDE_DESC
    "",
    "In burst mode only look at EBD at the end of codewords.",
#endif
    XPCSRX_RX_FRAMER_CTL_CFGXPCSRXFRMREBDVLDEN_FIELD_MASK,
    0,
    XPCSRX_RX_FRAMER_CTL_CFGXPCSRXFRMREBDVLDEN_FIELD_WIDTH,
    XPCSRX_RX_FRAMER_CTL_CFGXPCSRXFRMREBDVLDEN_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPCSRX_RX_FRAMER_CTL_CFGXPCSRXFRMRBDCNTEN
 ******************************************************************************/
const ru_field_rec XPCSRX_RX_FRAMER_CTL_CFGXPCSRXFRMRBDCNTEN_FIELD =
{
    "CFGXPCSRXFRMRBDCNTEN",
#if RU_INCLUDE_DESC
    "",
    "FPGA only.",
#endif
    XPCSRX_RX_FRAMER_CTL_CFGXPCSRXFRMRBDCNTEN_FIELD_MASK,
    0,
    XPCSRX_RX_FRAMER_CTL_CFGXPCSRXFRMRBDCNTEN_FIELD_WIDTH,
    XPCSRX_RX_FRAMER_CTL_CFGXPCSRXFRMRBDCNTEN_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPCSRX_RX_FRAMER_CTL_CFGXPCSRXFRMRBURSTBADSHEN
 ******************************************************************************/
const ru_field_rec XPCSRX_RX_FRAMER_CTL_CFGXPCSRXFRMRBURSTBADSHEN_FIELD =
{
    "CFGXPCSRXFRMRBURSTBADSHEN",
#if RU_INCLUDE_DESC
    "",
    "Allows framer to lose lock from 16 bad SH in 62 blocks for burst"
    "mode.",
#endif
    XPCSRX_RX_FRAMER_CTL_CFGXPCSRXFRMRBURSTBADSHEN_FIELD_MASK,
    0,
    XPCSRX_RX_FRAMER_CTL_CFGXPCSRXFRMRBURSTBADSHEN_FIELD_WIDTH,
    XPCSRX_RX_FRAMER_CTL_CFGXPCSRXFRMRBURSTBADSHEN_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPCSRX_RX_FRAMER_CTL_CFGXPCSRXFRMRSPULKEN
 ******************************************************************************/
const ru_field_rec XPCSRX_RX_FRAMER_CTL_CFGXPCSRXFRMRSPULKEN_FIELD =
{
    "CFGXPCSRXFRMRSPULKEN",
#if RU_INCLUDE_DESC
    "",
    "Allows framer to lose lock from detection of sync pattern.",
#endif
    XPCSRX_RX_FRAMER_CTL_CFGXPCSRXFRMRSPULKEN_FIELD_MASK,
    0,
    XPCSRX_RX_FRAMER_CTL_CFGXPCSRXFRMRSPULKEN_FIELD_WIDTH,
    XPCSRX_RX_FRAMER_CTL_CFGXPCSRXFRMRSPULKEN_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPCSRX_RX_FRAMER_CTL_CFGXPCSRXFRAMEBURST
 ******************************************************************************/
const ru_field_rec XPCSRX_RX_FRAMER_CTL_CFGXPCSRXFRAMEBURST_FIELD =
{
    "CFGXPCSRXFRAMEBURST",
#if RU_INCLUDE_DESC
    "",
    "Enable for burst mode framing.",
#endif
    XPCSRX_RX_FRAMER_CTL_CFGXPCSRXFRAMEBURST_FIELD_MASK,
    0,
    XPCSRX_RX_FRAMER_CTL_CFGXPCSRXFRAMEBURST_FIELD_WIDTH,
    XPCSRX_RX_FRAMER_CTL_CFGXPCSRXFRAMEBURST_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPCSRX_RX_FRAMER_CTL_CFGXPCSRXFRMREN
 ******************************************************************************/
const ru_field_rec XPCSRX_RX_FRAMER_CTL_CFGXPCSRXFRMREN_FIELD =
{
    "CFGXPCSRXFRMREN",
#if RU_INCLUDE_DESC
    "",
    "Enable for framer.",
#endif
    XPCSRX_RX_FRAMER_CTL_CFGXPCSRXFRMREN_FIELD_MASK,
    0,
    XPCSRX_RX_FRAMER_CTL_CFGXPCSRXFRMREN_FIELD_WIDTH,
    XPCSRX_RX_FRAMER_CTL_CFGXPCSRXFRMREN_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPCSRX_RX_FRAMER_CTL_CFGXPCSRXFRMRBLKFECFAIL
 ******************************************************************************/
const ru_field_rec XPCSRX_RX_FRAMER_CTL_CFGXPCSRXFRMRBLKFECFAIL_FIELD =
{
    "CFGXPCSRXFRMRBLKFECFAIL",
#if RU_INCLUDE_DESC
    "",
    "Allows for ignoring of FEC persist decode fail in clause 76 framing.",
#endif
    XPCSRX_RX_FRAMER_CTL_CFGXPCSRXFRMRBLKFECFAIL_FIELD_MASK,
    0,
    XPCSRX_RX_FRAMER_CTL_CFGXPCSRXFRMRBLKFECFAIL_FIELD_WIDTH,
    XPCSRX_RX_FRAMER_CTL_CFGXPCSRXFRMRBLKFECFAIL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPCSRX_RX_FRAMER_CTL_CFGXPCSRXFRAMEFEC
 ******************************************************************************/
const ru_field_rec XPCSRX_RX_FRAMER_CTL_CFGXPCSRXFRAMEFEC_FIELD =
{
    "CFGXPCSRXFRAMEFEC",
#if RU_INCLUDE_DESC
    "",
    "Enable for FEC framing.",
#endif
    XPCSRX_RX_FRAMER_CTL_CFGXPCSRXFRAMEFEC_FIELD_MASK,
    0,
    XPCSRX_RX_FRAMER_CTL_CFGXPCSRXFRAMEFEC_FIELD_WIDTH,
    XPCSRX_RX_FRAMER_CTL_CFGXPCSRXFRAMEFEC_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPCSRX_RX_FEC_CTL_RESERVED0
 ******************************************************************************/
const ru_field_rec XPCSRX_RX_FEC_CTL_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    XPCSRX_RX_FEC_CTL_RESERVED0_FIELD_MASK,
    0,
    XPCSRX_RX_FEC_CTL_RESERVED0_FIELD_WIDTH,
    XPCSRX_RX_FEC_CTL_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPCSRX_RX_FEC_CTL_CFGXPCSRXFECSTOPONERR
 ******************************************************************************/
const ru_field_rec XPCSRX_RX_FEC_CTL_CFGXPCSRXFECSTOPONERR_FIELD =
{
    "CFGXPCSRXFECSTOPONERR",
#if RU_INCLUDE_DESC
    "",
    "Freezes FEC decoder from writing and reading FEC RAM when an error"
    "is decoded."
    "No recovery after wards, requires that XPCS RX be reset.",
#endif
    XPCSRX_RX_FEC_CTL_CFGXPCSRXFECSTOPONERR_FIELD_MASK,
    0,
    XPCSRX_RX_FEC_CTL_CFGXPCSRXFECSTOPONERR_FIELD_WIDTH,
    XPCSRX_RX_FEC_CTL_CFGXPCSRXFECSTOPONERR_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPCSRX_RX_FEC_CTL_CFGXPCSRXFECCNTNQUECW
 ******************************************************************************/
const ru_field_rec XPCSRX_RX_FEC_CTL_CFGXPCSRXFECCNTNQUECW_FIELD =
{
    "CFGXPCSRXFECCNTNQUECW",
#if RU_INCLUDE_DESC
    "",
    "Tells FEC stats engine to count CW enqueued insted of total FEC"
    "decoded CW",
#endif
    XPCSRX_RX_FEC_CTL_CFGXPCSRXFECCNTNQUECW_FIELD_MASK,
    0,
    XPCSRX_RX_FEC_CTL_CFGXPCSRXFECCNTNQUECW_FIELD_WIDTH,
    XPCSRX_RX_FEC_CTL_CFGXPCSRXFECCNTNQUECW_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPCSRX_RX_FEC_CTL_CFGXPCSRXFECNQUERST
 ******************************************************************************/
const ru_field_rec XPCSRX_RX_FEC_CTL_CFGXPCSRXFECNQUERST_FIELD =
{
    "CFGXPCSRXFECNQUERST",
#if RU_INCLUDE_DESC
    "",
    "Reset the store and foward FIFO.",
#endif
    XPCSRX_RX_FEC_CTL_CFGXPCSRXFECNQUERST_FIELD_MASK,
    0,
    XPCSRX_RX_FEC_CTL_CFGXPCSRXFECNQUERST_FIELD_WIDTH,
    XPCSRX_RX_FEC_CTL_CFGXPCSRXFECNQUERST_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPCSRX_RX_FEC_CTL_CFGXPCSRXFECONEZEROMODE
 ******************************************************************************/
const ru_field_rec XPCSRX_RX_FEC_CTL_CFGXPCSRXFECONEZEROMODE_FIELD =
{
    "CFGXPCSRXFECONEZEROMODE",
#if RU_INCLUDE_DESC
    "",
    "0 - Count every bit correction for the FEC CW."
    "1 - Count only a single correction per 8 bits."
    "This only affects the corrected ones and corrected zero stats."
    "The total corrected is not effected by this control.",
#endif
    XPCSRX_RX_FEC_CTL_CFGXPCSRXFECONEZEROMODE_FIELD_MASK,
    0,
    XPCSRX_RX_FEC_CTL_CFGXPCSRXFECONEZEROMODE_FIELD_WIDTH,
    XPCSRX_RX_FEC_CTL_CFGXPCSRXFECONEZEROMODE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPCSRX_RX_FEC_CTL_CFGXPCSRXFECBLKCORRECT
 ******************************************************************************/
const ru_field_rec XPCSRX_RX_FEC_CTL_CFGXPCSRXFECBLKCORRECT_FIELD =
{
    "CFGXPCSRXFECBLKCORRECT",
#if RU_INCLUDE_DESC
    "",
    "Stop FEC decoder from making corrections.",
#endif
    XPCSRX_RX_FEC_CTL_CFGXPCSRXFECBLKCORRECT_FIELD_MASK,
    0,
    XPCSRX_RX_FEC_CTL_CFGXPCSRXFECBLKCORRECT_FIELD_WIDTH,
    XPCSRX_RX_FEC_CTL_CFGXPCSRXFECBLKCORRECT_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPCSRX_RX_FEC_CTL_CFGXPCSRXFECNQUETESTPAT
 ******************************************************************************/
const ru_field_rec XPCSRX_RX_FEC_CTL_CFGXPCSRXFECNQUETESTPAT_FIELD =
{
    "CFGXPCSRXFECNQUETESTPAT",
#if RU_INCLUDE_DESC
    "",
    "Replace all FEC CW with IEEE test CW.",
#endif
    XPCSRX_RX_FEC_CTL_CFGXPCSRXFECNQUETESTPAT_FIELD_MASK,
    0,
    XPCSRX_RX_FEC_CTL_CFGXPCSRXFECNQUETESTPAT_FIELD_WIDTH,
    XPCSRX_RX_FEC_CTL_CFGXPCSRXFECNQUETESTPAT_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPCSRX_RX_FEC_CTL_CFGXPCSRXFECFAILBLKSH0
 ******************************************************************************/
const ru_field_rec XPCSRX_RX_FEC_CTL_CFGXPCSRXFECFAILBLKSH0_FIELD =
{
    "CFGXPCSRXFECFAILBLKSH0",
#if RU_INCLUDE_DESC
    "",
    "0 - Do not blank out SH for failed FEC CW decodes. 1 - Blank out SH"
    "for failed FEC CW.  CW will pass as /E.",
#endif
    XPCSRX_RX_FEC_CTL_CFGXPCSRXFECFAILBLKSH0_FIELD_MASK,
    0,
    XPCSRX_RX_FEC_CTL_CFGXPCSRXFECFAILBLKSH0_FIELD_WIDTH,
    XPCSRX_RX_FEC_CTL_CFGXPCSRXFECFAILBLKSH0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPCSRX_RX_FEC_CTL_CFGXPCSRXFECSTRIP
 ******************************************************************************/
const ru_field_rec XPCSRX_RX_FEC_CTL_CFGXPCSRXFECSTRIP_FIELD =
{
    "CFGXPCSRXFECSTRIP",
#if RU_INCLUDE_DESC
    "",
    "Enable stripping of FEC parity for FEC decode bypass.",
#endif
    XPCSRX_RX_FEC_CTL_CFGXPCSRXFECSTRIP_FIELD_MASK,
    0,
    XPCSRX_RX_FEC_CTL_CFGXPCSRXFECSTRIP_FIELD_WIDTH,
    XPCSRX_RX_FEC_CTL_CFGXPCSRXFECSTRIP_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPCSRX_RX_FEC_CTL_CFGXPCSRXFECBYPAS
 ******************************************************************************/
const ru_field_rec XPCSRX_RX_FEC_CTL_CFGXPCSRXFECBYPAS_FIELD =
{
    "CFGXPCSRXFECBYPAS",
#if RU_INCLUDE_DESC
    "",
    "Elable FEC decode bypass.",
#endif
    XPCSRX_RX_FEC_CTL_CFGXPCSRXFECBYPAS_FIELD_MASK,
    0,
    XPCSRX_RX_FEC_CTL_CFGXPCSRXFECBYPAS_FIELD_WIDTH,
    XPCSRX_RX_FEC_CTL_CFGXPCSRXFECBYPAS_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPCSRX_RX_FEC_CTL_CFGXPCSRXFECIDLEINS
 ******************************************************************************/
const ru_field_rec XPCSRX_RX_FEC_CTL_CFGXPCSRXFECIDLEINS_FIELD =
{
    "CFGXPCSRXFECIDLEINS",
#if RU_INCLUDE_DESC
    "",
    "Enable idle insert to replace FEC parity lost in FEC decode.",
#endif
    XPCSRX_RX_FEC_CTL_CFGXPCSRXFECIDLEINS_FIELD_MASK,
    0,
    XPCSRX_RX_FEC_CTL_CFGXPCSRXFECIDLEINS_FIELD_WIDTH,
    XPCSRX_RX_FEC_CTL_CFGXPCSRXFECIDLEINS_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPCSRX_RX_FEC_CTL_CFGXPCSRXFECEN
 ******************************************************************************/
const ru_field_rec XPCSRX_RX_FEC_CTL_CFGXPCSRXFECEN_FIELD =
{
    "CFGXPCSRXFECEN",
#if RU_INCLUDE_DESC
    "",
    "Enable FEC decode.",
#endif
    XPCSRX_RX_FEC_CTL_CFGXPCSRXFECEN_FIELD_MASK,
    0,
    XPCSRX_RX_FEC_CTL_CFGXPCSRXFECEN_FIELD_WIDTH,
    XPCSRX_RX_FEC_CTL_CFGXPCSRXFECEN_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPCSRX_RX_DSCRAM_CTL_RESERVED0
 ******************************************************************************/
const ru_field_rec XPCSRX_RX_DSCRAM_CTL_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    XPCSRX_RX_DSCRAM_CTL_RESERVED0_FIELD_MASK,
    0,
    XPCSRX_RX_DSCRAM_CTL_RESERVED0_FIELD_WIDTH,
    XPCSRX_RX_DSCRAM_CTL_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPCSRX_RX_DSCRAM_CTL_CFGXPCSRXDSCRAMBYPAS
 ******************************************************************************/
const ru_field_rec XPCSRX_RX_DSCRAM_CTL_CFGXPCSRXDSCRAMBYPAS_FIELD =
{
    "CFGXPCSRXDSCRAMBYPAS",
#if RU_INCLUDE_DESC
    "",
    "Enable descrambler bypass.",
#endif
    XPCSRX_RX_DSCRAM_CTL_CFGXPCSRXDSCRAMBYPAS_FIELD_MASK,
    0,
    XPCSRX_RX_DSCRAM_CTL_CFGXPCSRXDSCRAMBYPAS_FIELD_WIDTH,
    XPCSRX_RX_DSCRAM_CTL_CFGXPCSRXDSCRAMBYPAS_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPCSRX_RX_64B66B_CTL_CFGXPCSRX64B66BTMASK1
 ******************************************************************************/
const ru_field_rec XPCSRX_RX_64B66B_CTL_CFGXPCSRX64B66BTMASK1_FIELD =
{
    "CFGXPCSRX64B66BTMASK1",
#if RU_INCLUDE_DESC
    "",
    "Defalut to T7 to T4 IPG vioalate det.",
#endif
    XPCSRX_RX_64B66B_CTL_CFGXPCSRX64B66BTMASK1_FIELD_MASK,
    0,
    XPCSRX_RX_64B66B_CTL_CFGXPCSRX64B66BTMASK1_FIELD_WIDTH,
    XPCSRX_RX_64B66B_CTL_CFGXPCSRX64B66BTMASK1_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPCSRX_RX_64B66B_CTL_CFGXPCSRX64B66BTMASK0
 ******************************************************************************/
const ru_field_rec XPCSRX_RX_64B66B_CTL_CFGXPCSRX64B66BTMASK0_FIELD =
{
    "CFGXPCSRX64B66BTMASK0",
#if RU_INCLUDE_DESC
    "",
    "Defalut to T7 to T4 IPG vioalate det.",
#endif
    XPCSRX_RX_64B66B_CTL_CFGXPCSRX64B66BTMASK0_FIELD_MASK,
    0,
    XPCSRX_RX_64B66B_CTL_CFGXPCSRX64B66BTMASK0_FIELD_WIDTH,
    XPCSRX_RX_64B66B_CTL_CFGXPCSRX64B66BTMASK0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPCSRX_RX_64B66B_CTL_RESERVED0
 ******************************************************************************/
const ru_field_rec XPCSRX_RX_64B66B_CTL_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    XPCSRX_RX_64B66B_CTL_RESERVED0_FIELD_MASK,
    0,
    XPCSRX_RX_64B66B_CTL_RESERVED0_FIELD_WIDTH,
    XPCSRX_RX_64B66B_CTL_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPCSRX_RX_64B66B_CTL_CFGXPCSRX64B66BSMASK1
 ******************************************************************************/
const ru_field_rec XPCSRX_RX_64B66B_CTL_CFGXPCSRX64B66BSMASK1_FIELD =
{
    "CFGXPCSRX64B66BSMASK1",
#if RU_INCLUDE_DESC
    "",
    "Defalut to S0 IPG vioalate det.",
#endif
    XPCSRX_RX_64B66B_CTL_CFGXPCSRX64B66BSMASK1_FIELD_MASK,
    0,
    XPCSRX_RX_64B66B_CTL_CFGXPCSRX64B66BSMASK1_FIELD_WIDTH,
    XPCSRX_RX_64B66B_CTL_CFGXPCSRX64B66BSMASK1_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPCSRX_RX_64B66B_CTL_RESERVED1
 ******************************************************************************/
const ru_field_rec XPCSRX_RX_64B66B_CTL_RESERVED1_FIELD =
{
    "RESERVED1",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    XPCSRX_RX_64B66B_CTL_RESERVED1_FIELD_MASK,
    0,
    XPCSRX_RX_64B66B_CTL_RESERVED1_FIELD_WIDTH,
    XPCSRX_RX_64B66B_CTL_RESERVED1_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPCSRX_RX_64B66B_CTL_CFGXPCSRX64B66BSMASK0
 ******************************************************************************/
const ru_field_rec XPCSRX_RX_64B66B_CTL_CFGXPCSRX64B66BSMASK0_FIELD =
{
    "CFGXPCSRX64B66BSMASK0",
#if RU_INCLUDE_DESC
    "",
    "Defalut to S0 IPG vioalate det.",
#endif
    XPCSRX_RX_64B66B_CTL_CFGXPCSRX64B66BSMASK0_FIELD_MASK,
    0,
    XPCSRX_RX_64B66B_CTL_CFGXPCSRX64B66BSMASK0_FIELD_WIDTH,
    XPCSRX_RX_64B66B_CTL_CFGXPCSRX64B66BSMASK0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPCSRX_RX_64B66B_CTL_RESERVED2
 ******************************************************************************/
const ru_field_rec XPCSRX_RX_64B66B_CTL_RESERVED2_FIELD =
{
    "RESERVED2",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    XPCSRX_RX_64B66B_CTL_RESERVED2_FIELD_MASK,
    0,
    XPCSRX_RX_64B66B_CTL_RESERVED2_FIELD_WIDTH,
    XPCSRX_RX_64B66B_CTL_RESERVED2_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPCSRX_RX_64B66B_CTL_CFGXPCSRX64B66BTDLAY
 ******************************************************************************/
const ru_field_rec XPCSRX_RX_64B66B_CTL_CFGXPCSRX64B66BTDLAY_FIELD =
{
    "CFGXPCSRX64B66BTDLAY",
#if RU_INCLUDE_DESC
    "",
    "Compare S one pipe behind T.",
#endif
    XPCSRX_RX_64B66B_CTL_CFGXPCSRX64B66BTDLAY_FIELD_MASK,
    0,
    XPCSRX_RX_64B66B_CTL_CFGXPCSRX64B66BTDLAY_FIELD_WIDTH,
    XPCSRX_RX_64B66B_CTL_CFGXPCSRX64B66BTDLAY_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPCSRX_RX_64B66B_CTL_RESERVED3
 ******************************************************************************/
const ru_field_rec XPCSRX_RX_64B66B_CTL_RESERVED3_FIELD =
{
    "RESERVED3",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    XPCSRX_RX_64B66B_CTL_RESERVED3_FIELD_MASK,
    0,
    XPCSRX_RX_64B66B_CTL_RESERVED3_FIELD_WIDTH,
    XPCSRX_RX_64B66B_CTL_RESERVED3_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPCSRX_RX_64B66B_CTL_CFGXPCSRX64B66BDECBYPAS
 ******************************************************************************/
const ru_field_rec XPCSRX_RX_64B66B_CTL_CFGXPCSRX64B66BDECBYPAS_FIELD =
{
    "CFGXPCSRX64B66BDECBYPAS",
#if RU_INCLUDE_DESC
    "",
    "Enable 64B/66B decoder bypass.",
#endif
    XPCSRX_RX_64B66B_CTL_CFGXPCSRX64B66BDECBYPAS_FIELD_MASK,
    0,
    XPCSRX_RX_64B66B_CTL_CFGXPCSRX64B66BDECBYPAS_FIELD_WIDTH,
    XPCSRX_RX_64B66B_CTL_CFGXPCSRX64B66BDECBYPAS_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPCSRX_RX_TEST_CTL_RESERVED0
 ******************************************************************************/
const ru_field_rec XPCSRX_RX_TEST_CTL_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    XPCSRX_RX_TEST_CTL_RESERVED0_FIELD_MASK,
    0,
    XPCSRX_RX_TEST_CTL_RESERVED0_FIELD_WIDTH,
    XPCSRX_RX_TEST_CTL_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPCSRX_RX_TEST_CTL_CFGXPCSRXTESTPRBSDETEN
 ******************************************************************************/
const ru_field_rec XPCSRX_RX_TEST_CTL_CFGXPCSRXTESTPRBSDETEN_FIELD =
{
    "CFGXPCSRXTESTPRBSDETEN",
#if RU_INCLUDE_DESC
    "",
    "Enable PRBS test pattern detector.",
#endif
    XPCSRX_RX_TEST_CTL_CFGXPCSRXTESTPRBSDETEN_FIELD_MASK,
    0,
    XPCSRX_RX_TEST_CTL_CFGXPCSRXTESTPRBSDETEN_FIELD_WIDTH,
    XPCSRX_RX_TEST_CTL_CFGXPCSRXTESTPRBSDETEN_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPCSRX_RX_TEST_CTL_CFGXPCSRXTESTPSUDODETEN
 ******************************************************************************/
const ru_field_rec XPCSRX_RX_TEST_CTL_CFGXPCSRXTESTPSUDODETEN_FIELD =
{
    "CFGXPCSRXTESTPSUDODETEN",
#if RU_INCLUDE_DESC
    "",
    "Enable psudo test pattern detector.",
#endif
    XPCSRX_RX_TEST_CTL_CFGXPCSRXTESTPSUDODETEN_FIELD_MASK,
    0,
    XPCSRX_RX_TEST_CTL_CFGXPCSRXTESTPSUDODETEN_FIELD_WIDTH,
    XPCSRX_RX_TEST_CTL_CFGXPCSRXTESTPSUDODETEN_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPCSRX_RX_IDLE_RD_TIMER_DLY_RESERVED0
 ******************************************************************************/
const ru_field_rec XPCSRX_RX_IDLE_RD_TIMER_DLY_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    XPCSRX_RX_IDLE_RD_TIMER_DLY_RESERVED0_FIELD_MASK,
    0,
    XPCSRX_RX_IDLE_RD_TIMER_DLY_RESERVED0_FIELD_WIDTH,
    XPCSRX_RX_IDLE_RD_TIMER_DLY_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPCSRX_RX_IDLE_RD_TIMER_DLY_CFGXPCSRXIDLERDDELAYTIMERMAX
 ******************************************************************************/
const ru_field_rec XPCSRX_RX_IDLE_RD_TIMER_DLY_CFGXPCSRXIDLERDDELAYTIMERMAX_FIELD =
{
    "CFGXPCSRXIDLERDDELAYTIMERMAX",
#if RU_INCLUDE_DESC
    "",
    "The delay time to start read of burst (default is 8'd60 ticks)."
    "Sets the delay to 8'd220 for 10K MTU.",
#endif
    XPCSRX_RX_IDLE_RD_TIMER_DLY_CFGXPCSRXIDLERDDELAYTIMERMAX_FIELD_MASK,
    0,
    XPCSRX_RX_IDLE_RD_TIMER_DLY_CFGXPCSRXIDLERDDELAYTIMERMAX_FIELD_WIDTH,
    XPCSRX_RX_IDLE_RD_TIMER_DLY_CFGXPCSRXIDLERDDELAYTIMERMAX_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPCSRX_RX_IDLE_GAP_SIZ_MAX_RESERVED0
 ******************************************************************************/
const ru_field_rec XPCSRX_RX_IDLE_GAP_SIZ_MAX_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    XPCSRX_RX_IDLE_GAP_SIZ_MAX_RESERVED0_FIELD_MASK,
    0,
    XPCSRX_RX_IDLE_GAP_SIZ_MAX_RESERVED0_FIELD_WIDTH,
    XPCSRX_RX_IDLE_GAP_SIZ_MAX_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPCSRX_RX_IDLE_GAP_SIZ_MAX_CFGXPCSRXIDLEOVRSIZMAX
 ******************************************************************************/
const ru_field_rec XPCSRX_RX_IDLE_GAP_SIZ_MAX_CFGXPCSRXIDLEOVRSIZMAX_FIELD =
{
    "CFGXPCSRXIDLEOVRSIZMAX",
#if RU_INCLUDE_DESC
    "",
    "Max size in blocks without an idle insert opportunity before idle"
    "insert is forced.",
#endif
    XPCSRX_RX_IDLE_GAP_SIZ_MAX_CFGXPCSRXIDLEOVRSIZMAX_FIELD_MASK,
    0,
    XPCSRX_RX_IDLE_GAP_SIZ_MAX_CFGXPCSRXIDLEOVRSIZMAX_FIELD_WIDTH,
    XPCSRX_RX_IDLE_GAP_SIZ_MAX_CFGXPCSRXIDLEOVRSIZMAX_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPCSRX_RX_IDLE_GAP_SIZ_MAX_CFGXPCSRXIDLESOPEOPGAP
 ******************************************************************************/
const ru_field_rec XPCSRX_RX_IDLE_GAP_SIZ_MAX_CFGXPCSRXIDLESOPEOPGAP_FIELD =
{
    "CFGXPCSRXIDLESOPEOPGAP",
#if RU_INCLUDE_DESC
    "",
    "Max distance in blocks between SOP and EOP.",
#endif
    XPCSRX_RX_IDLE_GAP_SIZ_MAX_CFGXPCSRXIDLESOPEOPGAP_FIELD_MASK,
    0,
    XPCSRX_RX_IDLE_GAP_SIZ_MAX_CFGXPCSRXIDLESOPEOPGAP_FIELD_WIDTH,
    XPCSRX_RX_IDLE_GAP_SIZ_MAX_CFGXPCSRXIDLESOPEOPGAP_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPCSRX_RX_FRAMER_LK_MAX_RESERVED0
 ******************************************************************************/
const ru_field_rec XPCSRX_RX_FRAMER_LK_MAX_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    XPCSRX_RX_FRAMER_LK_MAX_RESERVED0_FIELD_MASK,
    0,
    XPCSRX_RX_FRAMER_LK_MAX_RESERVED0_FIELD_WIDTH,
    XPCSRX_RX_FRAMER_LK_MAX_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPCSRX_RX_FRAMER_LK_MAX_CFGXPCSRXFRMRCWLKTIMERMAX
 ******************************************************************************/
const ru_field_rec XPCSRX_RX_FRAMER_LK_MAX_CFGXPCSRXFRMRCWLKTIMERMAX_FIELD =
{
    "CFGXPCSRXFRMRCWLKTIMERMAX",
#if RU_INCLUDE_DESC
    "",
    "Max delay for framer to inditcate to FEC circuit lock has been"
    "achieved.",
#endif
    XPCSRX_RX_FRAMER_LK_MAX_CFGXPCSRXFRMRCWLKTIMERMAX_FIELD_MASK,
    0,
    XPCSRX_RX_FRAMER_LK_MAX_CFGXPCSRXFRMRCWLKTIMERMAX_FIELD_WIDTH,
    XPCSRX_RX_FRAMER_LK_MAX_CFGXPCSRXFRMRCWLKTIMERMAX_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPCSRX_RX_FRAMER_UNLK_MAX_RESERVED0
 ******************************************************************************/
const ru_field_rec XPCSRX_RX_FRAMER_UNLK_MAX_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    XPCSRX_RX_FRAMER_UNLK_MAX_RESERVED0_FIELD_MASK,
    0,
    XPCSRX_RX_FRAMER_UNLK_MAX_RESERVED0_FIELD_WIDTH,
    XPCSRX_RX_FRAMER_UNLK_MAX_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPCSRX_RX_FRAMER_UNLK_MAX_CFGXPCSRXFRMRCWUNLKTIMERMAX
 ******************************************************************************/
const ru_field_rec XPCSRX_RX_FRAMER_UNLK_MAX_CFGXPCSRXFRMRCWUNLKTIMERMAX_FIELD =
{
    "CFGXPCSRXFRMRCWUNLKTIMERMAX",
#if RU_INCLUDE_DESC
    "",
    "Max delay for framer to inditcate to FEC circuit unlock has occured.",
#endif
    XPCSRX_RX_FRAMER_UNLK_MAX_CFGXPCSRXFRMRCWUNLKTIMERMAX_FIELD_MASK,
    0,
    XPCSRX_RX_FRAMER_UNLK_MAX_CFGXPCSRXFRMRCWUNLKTIMERMAX_FIELD_WIDTH,
    XPCSRX_RX_FRAMER_UNLK_MAX_CFGXPCSRXFRMRCWUNLKTIMERMAX_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPCSRX_RX_FRAMER_BD_SH_RESERVED0
 ******************************************************************************/
const ru_field_rec XPCSRX_RX_FRAMER_BD_SH_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    XPCSRX_RX_FRAMER_BD_SH_RESERVED0_FIELD_MASK,
    0,
    XPCSRX_RX_FRAMER_BD_SH_RESERVED0_FIELD_WIDTH,
    XPCSRX_RX_FRAMER_BD_SH_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPCSRX_RX_FRAMER_BD_SH_CFGXPCSRXOLTBDSH
 ******************************************************************************/
const ru_field_rec XPCSRX_RX_FRAMER_BD_SH_CFGXPCSRXOLTBDSH_FIELD =
{
    "CFGXPCSRXOLTBDSH",
#if RU_INCLUDE_DESC
    "",
    "BD sync header.",
#endif
    XPCSRX_RX_FRAMER_BD_SH_CFGXPCSRXOLTBDSH_FIELD_MASK,
    0,
    XPCSRX_RX_FRAMER_BD_SH_CFGXPCSRXOLTBDSH_FIELD_WIDTH,
    XPCSRX_RX_FRAMER_BD_SH_CFGXPCSRXOLTBDSH_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPCSRX_RX_FRAMER_BD_LO_CFGXPCSRXOLTBDLO
 ******************************************************************************/
const ru_field_rec XPCSRX_RX_FRAMER_BD_LO_CFGXPCSRXOLTBDLO_FIELD =
{
    "CFGXPCSRXOLTBDLO",
#if RU_INCLUDE_DESC
    "",
    "Low 32 bit value for the BD.",
#endif
    XPCSRX_RX_FRAMER_BD_LO_CFGXPCSRXOLTBDLO_FIELD_MASK,
    0,
    XPCSRX_RX_FRAMER_BD_LO_CFGXPCSRXOLTBDLO_FIELD_WIDTH,
    XPCSRX_RX_FRAMER_BD_LO_CFGXPCSRXOLTBDLO_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPCSRX_RX_FRAMER_BD_HI_CFGXPCSRXOLTBDHI
 ******************************************************************************/
const ru_field_rec XPCSRX_RX_FRAMER_BD_HI_CFGXPCSRXOLTBDHI_FIELD =
{
    "CFGXPCSRXOLTBDHI",
#if RU_INCLUDE_DESC
    "",
    "High 32 bit value for the BD.",
#endif
    XPCSRX_RX_FRAMER_BD_HI_CFGXPCSRXOLTBDHI_FIELD_MASK,
    0,
    XPCSRX_RX_FRAMER_BD_HI_CFGXPCSRXOLTBDHI_FIELD_WIDTH,
    XPCSRX_RX_FRAMER_BD_HI_CFGXPCSRXOLTBDHI_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPCSRX_RX_FRAMER_EBD_SH_RESERVED0
 ******************************************************************************/
const ru_field_rec XPCSRX_RX_FRAMER_EBD_SH_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    XPCSRX_RX_FRAMER_EBD_SH_RESERVED0_FIELD_MASK,
    0,
    XPCSRX_RX_FRAMER_EBD_SH_RESERVED0_FIELD_WIDTH,
    XPCSRX_RX_FRAMER_EBD_SH_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPCSRX_RX_FRAMER_EBD_SH_CFGXPCSRXOLTEBDSH
 ******************************************************************************/
const ru_field_rec XPCSRX_RX_FRAMER_EBD_SH_CFGXPCSRXOLTEBDSH_FIELD =
{
    "CFGXPCSRXOLTEBDSH",
#if RU_INCLUDE_DESC
    "",
    "EBD sync header.",
#endif
    XPCSRX_RX_FRAMER_EBD_SH_CFGXPCSRXOLTEBDSH_FIELD_MASK,
    0,
    XPCSRX_RX_FRAMER_EBD_SH_CFGXPCSRXOLTEBDSH_FIELD_WIDTH,
    XPCSRX_RX_FRAMER_EBD_SH_CFGXPCSRXOLTEBDSH_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPCSRX_RX_FRAMER_EBD_LO_CFGXPCSRXOLTEBDLO
 ******************************************************************************/
const ru_field_rec XPCSRX_RX_FRAMER_EBD_LO_CFGXPCSRXOLTEBDLO_FIELD =
{
    "CFGXPCSRXOLTEBDLO",
#if RU_INCLUDE_DESC
    "",
    "Low 32 bit value for the EBD.",
#endif
    XPCSRX_RX_FRAMER_EBD_LO_CFGXPCSRXOLTEBDLO_FIELD_MASK,
    0,
    XPCSRX_RX_FRAMER_EBD_LO_CFGXPCSRXOLTEBDLO_FIELD_WIDTH,
    XPCSRX_RX_FRAMER_EBD_LO_CFGXPCSRXOLTEBDLO_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPCSRX_RX_FRAMER_EBD_HI_CFGXPCSRXOLTEBDHI
 ******************************************************************************/
const ru_field_rec XPCSRX_RX_FRAMER_EBD_HI_CFGXPCSRXOLTEBDHI_FIELD =
{
    "CFGXPCSRXOLTEBDHI",
#if RU_INCLUDE_DESC
    "",
    "High 32 bit value for the EBD.",
#endif
    XPCSRX_RX_FRAMER_EBD_HI_CFGXPCSRXOLTEBDHI_FIELD_MASK,
    0,
    XPCSRX_RX_FRAMER_EBD_HI_CFGXPCSRXOLTEBDHI_FIELD_WIDTH,
    XPCSRX_RX_FRAMER_EBD_HI_CFGXPCSRXOLTEBDHI_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPCSRX_RX_STATUS_RESERVED0
 ******************************************************************************/
const ru_field_rec XPCSRX_RX_STATUS_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    XPCSRX_RX_STATUS_RESERVED0_FIELD_MASK,
    0,
    XPCSRX_RX_STATUS_RESERVED0_FIELD_WIDTH,
    XPCSRX_RX_STATUS_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: XPCSRX_RX_STATUS_STATRXIDLEDAJIT
 ******************************************************************************/
const ru_field_rec XPCSRX_RX_STATUS_STATRXIDLEDAJIT_FIELD =
{
    "STATRXIDLEDAJIT",
#if RU_INCLUDE_DESC
    "",
    "Raw status on interrups.",
#endif
    XPCSRX_RX_STATUS_STATRXIDLEDAJIT_FIELD_MASK,
    0,
    XPCSRX_RX_STATUS_STATRXIDLEDAJIT_FIELD_WIDTH,
    XPCSRX_RX_STATUS_STATRXIDLEDAJIT_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: XPCSRX_RX_STATUS_STATRXFRMRMISBRST
 ******************************************************************************/
const ru_field_rec XPCSRX_RX_STATUS_STATRXFRMRMISBRST_FIELD =
{
    "STATRXFRMRMISBRST",
#if RU_INCLUDE_DESC
    "",
    "Raw status on interrups.",
#endif
    XPCSRX_RX_STATUS_STATRXFRMRMISBRST_FIELD_MASK,
    0,
    XPCSRX_RX_STATUS_STATRXFRMRMISBRST_FIELD_WIDTH,
    XPCSRX_RX_STATUS_STATRXFRMRMISBRST_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: XPCSRX_RX_STATUS_STATRXIDLESOPEOPGAPBIG
 ******************************************************************************/
const ru_field_rec XPCSRX_RX_STATUS_STATRXIDLESOPEOPGAPBIG_FIELD =
{
    "STATRXIDLESOPEOPGAPBIG",
#if RU_INCLUDE_DESC
    "",
    "Raw status on interrups.",
#endif
    XPCSRX_RX_STATUS_STATRXIDLESOPEOPGAPBIG_FIELD_MASK,
    0,
    XPCSRX_RX_STATUS_STATRXIDLESOPEOPGAPBIG_FIELD_WIDTH,
    XPCSRX_RX_STATUS_STATRXIDLESOPEOPGAPBIG_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: XPCSRX_RX_STATUS_STATRXIDLEFRCINS
 ******************************************************************************/
const ru_field_rec XPCSRX_RX_STATUS_STATRXIDLEFRCINS_FIELD =
{
    "STATRXIDLEFRCINS",
#if RU_INCLUDE_DESC
    "",
    "Raw status on interrups.",
#endif
    XPCSRX_RX_STATUS_STATRXIDLEFRCINS_FIELD_MASK,
    0,
    XPCSRX_RX_STATUS_STATRXIDLEFRCINS_FIELD_WIDTH,
    XPCSRX_RX_STATUS_STATRXIDLEFRCINS_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: XPCSRX_RX_STATUS_STATRX64B66BMINIPGERR
 ******************************************************************************/
const ru_field_rec XPCSRX_RX_STATUS_STATRX64B66BMINIPGERR_FIELD =
{
    "STATRX64B66BMINIPGERR",
#if RU_INCLUDE_DESC
    "",
    "Raw status on interrups.",
#endif
    XPCSRX_RX_STATUS_STATRX64B66BMINIPGERR_FIELD_MASK,
    0,
    XPCSRX_RX_STATUS_STATRX64B66BMINIPGERR_FIELD_WIDTH,
    XPCSRX_RX_STATUS_STATRX64B66BMINIPGERR_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: XPCSRX_RX_STATUS_STATRXFECNQUECNTNEQ
 ******************************************************************************/
const ru_field_rec XPCSRX_RX_STATUS_STATRXFECNQUECNTNEQ_FIELD =
{
    "STATRXFECNQUECNTNEQ",
#if RU_INCLUDE_DESC
    "",
    "Raw status on interrups.",
#endif
    XPCSRX_RX_STATUS_STATRXFECNQUECNTNEQ_FIELD_MASK,
    0,
    XPCSRX_RX_STATUS_STATRXFECNQUECNTNEQ_FIELD_WIDTH,
    XPCSRX_RX_STATUS_STATRXFECNQUECNTNEQ_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: XPCSRX_RX_STATUS_STATRXIDLEFIFOUNDRUN
 ******************************************************************************/
const ru_field_rec XPCSRX_RX_STATUS_STATRXIDLEFIFOUNDRUN_FIELD =
{
    "STATRXIDLEFIFOUNDRUN",
#if RU_INCLUDE_DESC
    "",
    "Raw status on interrups.",
#endif
    XPCSRX_RX_STATUS_STATRXIDLEFIFOUNDRUN_FIELD_MASK,
    0,
    XPCSRX_RX_STATUS_STATRXIDLEFIFOUNDRUN_FIELD_WIDTH,
    XPCSRX_RX_STATUS_STATRXIDLEFIFOUNDRUN_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: XPCSRX_RX_STATUS_STATRXIDLEFIFOOVRRUN
 ******************************************************************************/
const ru_field_rec XPCSRX_RX_STATUS_STATRXIDLEFIFOOVRRUN_FIELD =
{
    "STATRXIDLEFIFOOVRRUN",
#if RU_INCLUDE_DESC
    "",
    "Raw status on interrups.",
#endif
    XPCSRX_RX_STATUS_STATRXIDLEFIFOOVRRUN_FIELD_MASK,
    0,
    XPCSRX_RX_STATUS_STATRXIDLEFIFOOVRRUN_FIELD_WIDTH,
    XPCSRX_RX_STATUS_STATRXIDLEFIFOOVRRUN_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: XPCSRX_RX_STATUS_STATRXFECHIGHCOR
 ******************************************************************************/
const ru_field_rec XPCSRX_RX_STATUS_STATRXFECHIGHCOR_FIELD =
{
    "STATRXFECHIGHCOR",
#if RU_INCLUDE_DESC
    "",
    "Raw status on interrups.",
#endif
    XPCSRX_RX_STATUS_STATRXFECHIGHCOR_FIELD_MASK,
    0,
    XPCSRX_RX_STATUS_STATRXFECHIGHCOR_FIELD_WIDTH,
    XPCSRX_RX_STATUS_STATRXFECHIGHCOR_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: XPCSRX_RX_STATUS_RESERVED1
 ******************************************************************************/
const ru_field_rec XPCSRX_RX_STATUS_RESERVED1_FIELD =
{
    "RESERVED1",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    XPCSRX_RX_STATUS_RESERVED1_FIELD_MASK,
    0,
    XPCSRX_RX_STATUS_RESERVED1_FIELD_WIDTH,
    XPCSRX_RX_STATUS_RESERVED1_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: XPCSRX_RX_STATUS_STATRXFECDECPASS
 ******************************************************************************/
const ru_field_rec XPCSRX_RX_STATUS_STATRXFECDECPASS_FIELD =
{
    "STATRXFECDECPASS",
#if RU_INCLUDE_DESC
    "",
    "Raw status on interrups.",
#endif
    XPCSRX_RX_STATUS_STATRXFECDECPASS_FIELD_MASK,
    0,
    XPCSRX_RX_STATUS_STATRXFECDECPASS_FIELD_WIDTH,
    XPCSRX_RX_STATUS_STATRXFECDECPASS_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: XPCSRX_RX_STATUS_STATRXSTATFRMRHIGHBER
 ******************************************************************************/
const ru_field_rec XPCSRX_RX_STATUS_STATRXSTATFRMRHIGHBER_FIELD =
{
    "STATRXSTATFRMRHIGHBER",
#if RU_INCLUDE_DESC
    "",
    "Raw status on interrups.",
#endif
    XPCSRX_RX_STATUS_STATRXSTATFRMRHIGHBER_FIELD_MASK,
    0,
    XPCSRX_RX_STATUS_STATRXSTATFRMRHIGHBER_FIELD_WIDTH,
    XPCSRX_RX_STATUS_STATRXSTATFRMRHIGHBER_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: XPCSRX_RX_STATUS_STATRXFRMREXITBYSP
 ******************************************************************************/
const ru_field_rec XPCSRX_RX_STATUS_STATRXFRMREXITBYSP_FIELD =
{
    "STATRXFRMREXITBYSP",
#if RU_INCLUDE_DESC
    "",
    "Raw status on interrups.",
#endif
    XPCSRX_RX_STATUS_STATRXFRMREXITBYSP_FIELD_MASK,
    0,
    XPCSRX_RX_STATUS_STATRXFRMREXITBYSP_FIELD_WIDTH,
    XPCSRX_RX_STATUS_STATRXFRMREXITBYSP_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: XPCSRX_RX_STATUS_STATRXFRMRBADSHMAX
 ******************************************************************************/
const ru_field_rec XPCSRX_RX_STATUS_STATRXFRMRBADSHMAX_FIELD =
{
    "STATRXFRMRBADSHMAX",
#if RU_INCLUDE_DESC
    "",
    "Raw status on interrups.",
#endif
    XPCSRX_RX_STATUS_STATRXFRMRBADSHMAX_FIELD_MASK,
    0,
    XPCSRX_RX_STATUS_STATRXFRMRBADSHMAX_FIELD_WIDTH,
    XPCSRX_RX_STATUS_STATRXFRMRBADSHMAX_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: XPCSRX_RX_STATUS_STATRXDSCRAMBURSTSEQOUT
 ******************************************************************************/
const ru_field_rec XPCSRX_RX_STATUS_STATRXDSCRAMBURSTSEQOUT_FIELD =
{
    "STATRXDSCRAMBURSTSEQOUT",
#if RU_INCLUDE_DESC
    "",
    "Raw status on interrups.",
#endif
    XPCSRX_RX_STATUS_STATRXDSCRAMBURSTSEQOUT_FIELD_MASK,
    0,
    XPCSRX_RX_STATUS_STATRXDSCRAMBURSTSEQOUT_FIELD_WIDTH,
    XPCSRX_RX_STATUS_STATRXDSCRAMBURSTSEQOUT_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: XPCSRX_RX_STATUS_STATRXTESTPSUDOLOCK
 ******************************************************************************/
const ru_field_rec XPCSRX_RX_STATUS_STATRXTESTPSUDOLOCK_FIELD =
{
    "STATRXTESTPSUDOLOCK",
#if RU_INCLUDE_DESC
    "",
    "Raw status on interrups.",
#endif
    XPCSRX_RX_STATUS_STATRXTESTPSUDOLOCK_FIELD_MASK,
    0,
    XPCSRX_RX_STATUS_STATRXTESTPSUDOLOCK_FIELD_WIDTH,
    XPCSRX_RX_STATUS_STATRXTESTPSUDOLOCK_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: XPCSRX_RX_STATUS_STATRXTESTPSUDOTYPE
 ******************************************************************************/
const ru_field_rec XPCSRX_RX_STATUS_STATRXTESTPSUDOTYPE_FIELD =
{
    "STATRXTESTPSUDOTYPE",
#if RU_INCLUDE_DESC
    "",
    "Raw status on interrups.",
#endif
    XPCSRX_RX_STATUS_STATRXTESTPSUDOTYPE_FIELD_MASK,
    0,
    XPCSRX_RX_STATUS_STATRXTESTPSUDOTYPE_FIELD_WIDTH,
    XPCSRX_RX_STATUS_STATRXTESTPSUDOTYPE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: XPCSRX_RX_STATUS_STATRXTESTPSUDOERR
 ******************************************************************************/
const ru_field_rec XPCSRX_RX_STATUS_STATRXTESTPSUDOERR_FIELD =
{
    "STATRXTESTPSUDOERR",
#if RU_INCLUDE_DESC
    "",
    "Raw status on interrups.",
#endif
    XPCSRX_RX_STATUS_STATRXTESTPSUDOERR_FIELD_MASK,
    0,
    XPCSRX_RX_STATUS_STATRXTESTPSUDOERR_FIELD_WIDTH,
    XPCSRX_RX_STATUS_STATRXTESTPSUDOERR_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: XPCSRX_RX_STATUS_STATRXTESTPRBSLOCK
 ******************************************************************************/
const ru_field_rec XPCSRX_RX_STATUS_STATRXTESTPRBSLOCK_FIELD =
{
    "STATRXTESTPRBSLOCK",
#if RU_INCLUDE_DESC
    "",
    "Raw status on interrups.",
#endif
    XPCSRX_RX_STATUS_STATRXTESTPRBSLOCK_FIELD_MASK,
    0,
    XPCSRX_RX_STATUS_STATRXTESTPRBSLOCK_FIELD_WIDTH,
    XPCSRX_RX_STATUS_STATRXTESTPRBSLOCK_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: XPCSRX_RX_STATUS_STATRXTESTPRBSERR
 ******************************************************************************/
const ru_field_rec XPCSRX_RX_STATUS_STATRXTESTPRBSERR_FIELD =
{
    "STATRXTESTPRBSERR",
#if RU_INCLUDE_DESC
    "",
    "Raw status on interrups.",
#endif
    XPCSRX_RX_STATUS_STATRXTESTPRBSERR_FIELD_MASK,
    0,
    XPCSRX_RX_STATUS_STATRXTESTPRBSERR_FIELD_WIDTH,
    XPCSRX_RX_STATUS_STATRXTESTPRBSERR_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: XPCSRX_RX_STATUS_STATRXFECPSISTDECFAIL
 ******************************************************************************/
const ru_field_rec XPCSRX_RX_STATUS_STATRXFECPSISTDECFAIL_FIELD =
{
    "STATRXFECPSISTDECFAIL",
#if RU_INCLUDE_DESC
    "",
    "Raw status on interrups.",
#endif
    XPCSRX_RX_STATUS_STATRXFECPSISTDECFAIL_FIELD_MASK,
    0,
    XPCSRX_RX_STATUS_STATRXFECPSISTDECFAIL_FIELD_WIDTH,
    XPCSRX_RX_STATUS_STATRXFECPSISTDECFAIL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: XPCSRX_RX_STATUS_STATRXFRAMERBADSH
 ******************************************************************************/
const ru_field_rec XPCSRX_RX_STATUS_STATRXFRAMERBADSH_FIELD =
{
    "STATRXFRAMERBADSH",
#if RU_INCLUDE_DESC
    "",
    "Raw status on interrups.",
#endif
    XPCSRX_RX_STATUS_STATRXFRAMERBADSH_FIELD_MASK,
    0,
    XPCSRX_RX_STATUS_STATRXFRAMERBADSH_FIELD_WIDTH,
    XPCSRX_RX_STATUS_STATRXFRAMERBADSH_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: XPCSRX_RX_STATUS_STATRXFRAMERCWLOSS
 ******************************************************************************/
const ru_field_rec XPCSRX_RX_STATUS_STATRXFRAMERCWLOSS_FIELD =
{
    "STATRXFRAMERCWLOSS",
#if RU_INCLUDE_DESC
    "",
    "Raw status on interrups.",
#endif
    XPCSRX_RX_STATUS_STATRXFRAMERCWLOSS_FIELD_MASK,
    0,
    XPCSRX_RX_STATUS_STATRXFRAMERCWLOSS_FIELD_WIDTH,
    XPCSRX_RX_STATUS_STATRXFRAMERCWLOSS_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: XPCSRX_RX_STATUS_STATRXFRAMERCWLOCK
 ******************************************************************************/
const ru_field_rec XPCSRX_RX_STATUS_STATRXFRAMERCWLOCK_FIELD =
{
    "STATRXFRAMERCWLOCK",
#if RU_INCLUDE_DESC
    "",
    "Raw status on interrups.",
#endif
    XPCSRX_RX_STATUS_STATRXFRAMERCWLOCK_FIELD_MASK,
    0,
    XPCSRX_RX_STATUS_STATRXFRAMERCWLOCK_FIELD_WIDTH,
    XPCSRX_RX_STATUS_STATRXFRAMERCWLOCK_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: XPCSRX_RX_STATUS_STATRXFECDECFAIL
 ******************************************************************************/
const ru_field_rec XPCSRX_RX_STATUS_STATRXFECDECFAIL_FIELD =
{
    "STATRXFECDECFAIL",
#if RU_INCLUDE_DESC
    "",
    "Raw status on interrups.",
#endif
    XPCSRX_RX_STATUS_STATRXFECDECFAIL_FIELD_MASK,
    0,
    XPCSRX_RX_STATUS_STATRXFECDECFAIL_FIELD_WIDTH,
    XPCSRX_RX_STATUS_STATRXFECDECFAIL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: XPCSRX_RX_STATUS_STATRX64B66BDECERR
 ******************************************************************************/
const ru_field_rec XPCSRX_RX_STATUS_STATRX64B66BDECERR_FIELD =
{
    "STATRX64B66BDECERR",
#if RU_INCLUDE_DESC
    "",
    "Raw status on interrups.",
#endif
    XPCSRX_RX_STATUS_STATRX64B66BDECERR_FIELD_MASK,
    0,
    XPCSRX_RX_STATUS_STATRX64B66BDECERR_FIELD_WIDTH,
    XPCSRX_RX_STATUS_STATRX64B66BDECERR_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: XPCSRX_RX_STATUS_STATRXFRMRNOLOCKLOS
 ******************************************************************************/
const ru_field_rec XPCSRX_RX_STATUS_STATRXFRMRNOLOCKLOS_FIELD =
{
    "STATRXFRMRNOLOCKLOS",
#if RU_INCLUDE_DESC
    "",
    "Raw status on interrups.",
#endif
    XPCSRX_RX_STATUS_STATRXFRMRNOLOCKLOS_FIELD_MASK,
    0,
    XPCSRX_RX_STATUS_STATRXFRMRNOLOCKLOS_FIELD_WIDTH,
    XPCSRX_RX_STATUS_STATRXFRMRNOLOCKLOS_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: XPCSRX_RX_STATUS_STATRXFRMRROGUE
 ******************************************************************************/
const ru_field_rec XPCSRX_RX_STATUS_STATRXFRMRROGUE_FIELD =
{
    "STATRXFRMRROGUE",
#if RU_INCLUDE_DESC
    "",
    "Raw status on interrups.",
#endif
    XPCSRX_RX_STATUS_STATRXFRMRROGUE_FIELD_MASK,
    0,
    XPCSRX_RX_STATUS_STATRXFRMRROGUE_FIELD_WIDTH,
    XPCSRX_RX_STATUS_STATRXFRMRROGUE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: XPCSRX_RX_FRAMER_LK_ULK_MAX_RESERVED0
 ******************************************************************************/
const ru_field_rec XPCSRX_RX_FRAMER_LK_ULK_MAX_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    XPCSRX_RX_FRAMER_LK_ULK_MAX_RESERVED0_FIELD_MASK,
    0,
    XPCSRX_RX_FRAMER_LK_ULK_MAX_RESERVED0_FIELD_WIDTH,
    XPCSRX_RX_FRAMER_LK_ULK_MAX_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPCSRX_RX_FRAMER_LK_ULK_MAX_CFGXPCSRXFRMRSPLKMAX
 ******************************************************************************/
const ru_field_rec XPCSRX_RX_FRAMER_LK_ULK_MAX_CFGXPCSRXFRMRSPLKMAX_FIELD =
{
    "CFGXPCSRXFRMRSPLKMAX",
#if RU_INCLUDE_DESC
    "",
    "The number of consecutive SP before a BD required to gain lock.",
#endif
    XPCSRX_RX_FRAMER_LK_ULK_MAX_CFGXPCSRXFRMRSPLKMAX_FIELD_MASK,
    0,
    XPCSRX_RX_FRAMER_LK_ULK_MAX_CFGXPCSRXFRMRSPLKMAX_FIELD_WIDTH,
    XPCSRX_RX_FRAMER_LK_ULK_MAX_CFGXPCSRXFRMRSPLKMAX_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPCSRX_RX_FRAMER_LK_ULK_MAX_RESERVED1
 ******************************************************************************/
const ru_field_rec XPCSRX_RX_FRAMER_LK_ULK_MAX_RESERVED1_FIELD =
{
    "RESERVED1",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    XPCSRX_RX_FRAMER_LK_ULK_MAX_RESERVED1_FIELD_MASK,
    0,
    XPCSRX_RX_FRAMER_LK_ULK_MAX_RESERVED1_FIELD_WIDTH,
    XPCSRX_RX_FRAMER_LK_ULK_MAX_RESERVED1_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPCSRX_RX_FRAMER_LK_ULK_MAX_CFGXPCSRXFRMRSPULKMAX
 ******************************************************************************/
const ru_field_rec XPCSRX_RX_FRAMER_LK_ULK_MAX_CFGXPCSRXFRMRSPULKMAX_FIELD =
{
    "CFGXPCSRXFRMRSPULKMAX",
#if RU_INCLUDE_DESC
    "",
    "The number of consecutive SP required to lose lock.",
#endif
    XPCSRX_RX_FRAMER_LK_ULK_MAX_CFGXPCSRXFRMRSPULKMAX_FIELD_MASK,
    0,
    XPCSRX_RX_FRAMER_LK_ULK_MAX_CFGXPCSRXFRMRSPULKMAX_FIELD_WIDTH,
    XPCSRX_RX_FRAMER_LK_ULK_MAX_CFGXPCSRXFRMRSPULKMAX_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPCSRX_RX_FRAMER_SP_SH_RESERVED0
 ******************************************************************************/
const ru_field_rec XPCSRX_RX_FRAMER_SP_SH_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    XPCSRX_RX_FRAMER_SP_SH_RESERVED0_FIELD_MASK,
    0,
    XPCSRX_RX_FRAMER_SP_SH_RESERVED0_FIELD_WIDTH,
    XPCSRX_RX_FRAMER_SP_SH_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPCSRX_RX_FRAMER_SP_SH_CFGXPCSRXOLTSPSH
 ******************************************************************************/
const ru_field_rec XPCSRX_RX_FRAMER_SP_SH_CFGXPCSRXOLTSPSH_FIELD =
{
    "CFGXPCSRXOLTSPSH",
#if RU_INCLUDE_DESC
    "",
    "The SH value for SP.",
#endif
    XPCSRX_RX_FRAMER_SP_SH_CFGXPCSRXOLTSPSH_FIELD_MASK,
    0,
    XPCSRX_RX_FRAMER_SP_SH_CFGXPCSRXOLTSPSH_FIELD_WIDTH,
    XPCSRX_RX_FRAMER_SP_SH_CFGXPCSRXOLTSPSH_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPCSRX_RX_FRAMER_SP_LO_CFGXPCSRXOLTSPLO
 ******************************************************************************/
const ru_field_rec XPCSRX_RX_FRAMER_SP_LO_CFGXPCSRXOLTSPLO_FIELD =
{
    "CFGXPCSRXOLTSPLO",
#if RU_INCLUDE_DESC
    "",
    "The lowest 32 bit value for the SP.",
#endif
    XPCSRX_RX_FRAMER_SP_LO_CFGXPCSRXOLTSPLO_FIELD_MASK,
    0,
    XPCSRX_RX_FRAMER_SP_LO_CFGXPCSRXOLTSPLO_FIELD_WIDTH,
    XPCSRX_RX_FRAMER_SP_LO_CFGXPCSRXOLTSPLO_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPCSRX_RX_FRAMER_SP_HI_CFGXPCSRXOLTSPHI
 ******************************************************************************/
const ru_field_rec XPCSRX_RX_FRAMER_SP_HI_CFGXPCSRXOLTSPHI_FIELD =
{
    "CFGXPCSRXOLTSPHI",
#if RU_INCLUDE_DESC
    "",
    "The highest 32 bit value for the SP.",
#endif
    XPCSRX_RX_FRAMER_SP_HI_CFGXPCSRXOLTSPHI_FIELD_MASK,
    0,
    XPCSRX_RX_FRAMER_SP_HI_CFGXPCSRXOLTSPHI_FIELD_WIDTH,
    XPCSRX_RX_FRAMER_SP_HI_CFGXPCSRXOLTSPHI_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPCSRX_RX_FRAMER_STATE_RESERVED0
 ******************************************************************************/
const ru_field_rec XPCSRX_RX_FRAMER_STATE_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    XPCSRX_RX_FRAMER_STATE_RESERVED0_FIELD_MASK,
    0,
    XPCSRX_RX_FRAMER_STATE_RESERVED0_FIELD_WIDTH,
    XPCSRX_RX_FRAMER_STATE_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: XPCSRX_RX_FRAMER_STATE_XPCSRXFRMRSTATE
 ******************************************************************************/
const ru_field_rec XPCSRX_RX_FRAMER_STATE_XPCSRXFRMRSTATE_FIELD =
{
    "XPCSRXFRMRSTATE",
#if RU_INCLUDE_DESC
    "",
    "The state of the framer state machine.",
#endif
    XPCSRX_RX_FRAMER_STATE_XPCSRXFRMRSTATE_FIELD_MASK,
    0,
    XPCSRX_RX_FRAMER_STATE_XPCSRXFRMRSTATE_FIELD_WIDTH,
    XPCSRX_RX_FRAMER_STATE_XPCSRXFRMRSTATE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: XPCSRX_RX_FRAMER_BD_EBD_HAM_RESERVED0
 ******************************************************************************/
const ru_field_rec XPCSRX_RX_FRAMER_BD_EBD_HAM_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    XPCSRX_RX_FRAMER_BD_EBD_HAM_RESERVED0_FIELD_MASK,
    0,
    XPCSRX_RX_FRAMER_BD_EBD_HAM_RESERVED0_FIELD_WIDTH,
    XPCSRX_RX_FRAMER_BD_EBD_HAM_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPCSRX_RX_FRAMER_BD_EBD_HAM_CFGXPCSRXFRMRSPHAM
 ******************************************************************************/
const ru_field_rec XPCSRX_RX_FRAMER_BD_EBD_HAM_CFGXPCSRXFRMRSPHAM_FIELD =
{
    "CFGXPCSRXFRMRSPHAM",
#if RU_INCLUDE_DESC
    "",
    "Hamming distance for SP.",
#endif
    XPCSRX_RX_FRAMER_BD_EBD_HAM_CFGXPCSRXFRMRSPHAM_FIELD_MASK,
    0,
    XPCSRX_RX_FRAMER_BD_EBD_HAM_CFGXPCSRXFRMRSPHAM_FIELD_WIDTH,
    XPCSRX_RX_FRAMER_BD_EBD_HAM_CFGXPCSRXFRMRSPHAM_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPCSRX_RX_FRAMER_BD_EBD_HAM_CFGXPCSRXFRMREBDHAM
 ******************************************************************************/
const ru_field_rec XPCSRX_RX_FRAMER_BD_EBD_HAM_CFGXPCSRXFRMREBDHAM_FIELD =
{
    "CFGXPCSRXFRMREBDHAM",
#if RU_INCLUDE_DESC
    "",
    "Hamming distance for EBD.",
#endif
    XPCSRX_RX_FRAMER_BD_EBD_HAM_CFGXPCSRXFRMREBDHAM_FIELD_MASK,
    0,
    XPCSRX_RX_FRAMER_BD_EBD_HAM_CFGXPCSRXFRMREBDHAM_FIELD_WIDTH,
    XPCSRX_RX_FRAMER_BD_EBD_HAM_CFGXPCSRXFRMREBDHAM_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPCSRX_RX_FRAMER_BD_EBD_HAM_CFGXPCSRXFRMRBDHAM
 ******************************************************************************/
const ru_field_rec XPCSRX_RX_FRAMER_BD_EBD_HAM_CFGXPCSRXFRMRBDHAM_FIELD =
{
    "CFGXPCSRXFRMRBDHAM",
#if RU_INCLUDE_DESC
    "",
    "Hamming distance for BD.",
#endif
    XPCSRX_RX_FRAMER_BD_EBD_HAM_CFGXPCSRXFRMRBDHAM_FIELD_MASK,
    0,
    XPCSRX_RX_FRAMER_BD_EBD_HAM_CFGXPCSRXFRMRBDHAM_FIELD_WIDTH,
    XPCSRX_RX_FRAMER_BD_EBD_HAM_CFGXPCSRXFRMRBDHAM_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPCSRX_RX_FRAMER_MISBRST_CNT_RXFRMRMISBRSTCNT
 ******************************************************************************/
const ru_field_rec XPCSRX_RX_FRAMER_MISBRST_CNT_RXFRMRMISBRSTCNT_FIELD =
{
    "RXFRMRMISBRSTCNT",
#if RU_INCLUDE_DESC
    "",
    "Count of possibe missed burst.",
#endif
    XPCSRX_RX_FRAMER_MISBRST_CNT_RXFRMRMISBRSTCNT_FIELD_MASK,
    0,
    XPCSRX_RX_FRAMER_MISBRST_CNT_RXFRMRMISBRSTCNT_FIELD_WIDTH,
    XPCSRX_RX_FRAMER_MISBRST_CNT_RXFRMRMISBRSTCNT_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPCSRX_RX_FRAMER_BD_ERR_RESERVED0
 ******************************************************************************/
const ru_field_rec XPCSRX_RX_FRAMER_BD_ERR_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    XPCSRX_RX_FRAMER_BD_ERR_RESERVED0_FIELD_MASK,
    0,
    XPCSRX_RX_FRAMER_BD_ERR_RESERVED0_FIELD_WIDTH,
    XPCSRX_RX_FRAMER_BD_ERR_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: XPCSRX_RX_FRAMER_BD_ERR_XPCSRXSTATFRMRBDERR
 ******************************************************************************/
const ru_field_rec XPCSRX_RX_FRAMER_BD_ERR_XPCSRXSTATFRMRBDERR_FIELD =
{
    "XPCSRXSTATFRMRBDERR",
#if RU_INCLUDE_DESC
    "",
    "A count of the errors in BD when it was found.",
#endif
    XPCSRX_RX_FRAMER_BD_ERR_XPCSRXSTATFRMRBDERR_FIELD_MASK,
    0,
    XPCSRX_RX_FRAMER_BD_ERR_XPCSRXSTATFRMRBDERR_FIELD_WIDTH,
    XPCSRX_RX_FRAMER_BD_ERR_XPCSRXSTATFRMRBDERR_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: XPCSRX_RX_FRAMER_ROGUE_CTL_CFGXPCSRXFRMRROGUEEN
 ******************************************************************************/
const ru_field_rec XPCSRX_RX_FRAMER_ROGUE_CTL_CFGXPCSRXFRMRROGUEEN_FIELD =
{
    "CFGXPCSRXFRMRROGUEEN",
#if RU_INCLUDE_DESC
    "",
    "0 - Rogue detection is disable. 1 - Rogue detection is enable.",
#endif
    XPCSRX_RX_FRAMER_ROGUE_CTL_CFGXPCSRXFRMRROGUEEN_FIELD_MASK,
    0,
    XPCSRX_RX_FRAMER_ROGUE_CTL_CFGXPCSRXFRMRROGUEEN_FIELD_WIDTH,
    XPCSRX_RX_FRAMER_ROGUE_CTL_CFGXPCSRXFRMRROGUEEN_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPCSRX_RX_FRAMER_ROGUE_CTL_RESERVED0
 ******************************************************************************/
const ru_field_rec XPCSRX_RX_FRAMER_ROGUE_CTL_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    XPCSRX_RX_FRAMER_ROGUE_CTL_RESERVED0_FIELD_MASK,
    0,
    XPCSRX_RX_FRAMER_ROGUE_CTL_RESERVED0_FIELD_WIDTH,
    XPCSRX_RX_FRAMER_ROGUE_CTL_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPCSRX_RX_FRAMER_ROGUE_CTL_CFGXPCSRXFRMRROGUESPTRESH
 ******************************************************************************/
const ru_field_rec XPCSRX_RX_FRAMER_ROGUE_CTL_CFGXPCSRXFRMRROGUESPTRESH_FIELD =
{
    "CFGXPCSRXFRMRROGUESPTRESH",
#if RU_INCLUDE_DESC
    "",
    "If SP count hits treshold and the ranging window endswithout a lock"
    "then a rogue detectoin alarm is set.",
#endif
    XPCSRX_RX_FRAMER_ROGUE_CTL_CFGXPCSRXFRMRROGUESPTRESH_FIELD_MASK,
    0,
    XPCSRX_RX_FRAMER_ROGUE_CTL_CFGXPCSRXFRMRROGUESPTRESH_FIELD_WIDTH,
    XPCSRX_RX_FRAMER_ROGUE_CTL_CFGXPCSRXFRMRROGUESPTRESH_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPCSRX_RX_FRAMER_NOLOCK_CTL_CFGXPCSRXFRMRNOLOCKLOSEN
 ******************************************************************************/
const ru_field_rec XPCSRX_RX_FRAMER_NOLOCK_CTL_CFGXPCSRXFRMRNOLOCKLOSEN_FIELD =
{
    "CFGXPCSRXFRMRNOLOCKLOSEN",
#if RU_INCLUDE_DESC
    "",
    "0 - No lock LOS detection is disable. 1 - No lock LOS detection is"
    "enable.",
#endif
    XPCSRX_RX_FRAMER_NOLOCK_CTL_CFGXPCSRXFRMRNOLOCKLOSEN_FIELD_MASK,
    0,
    XPCSRX_RX_FRAMER_NOLOCK_CTL_CFGXPCSRXFRMRNOLOCKLOSEN_FIELD_WIDTH,
    XPCSRX_RX_FRAMER_NOLOCK_CTL_CFGXPCSRXFRMRNOLOCKLOSEN_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPCSRX_RX_FRAMER_NOLOCK_CTL_RESERVED0
 ******************************************************************************/
const ru_field_rec XPCSRX_RX_FRAMER_NOLOCK_CTL_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    XPCSRX_RX_FRAMER_NOLOCK_CTL_RESERVED0_FIELD_MASK,
    0,
    XPCSRX_RX_FRAMER_NOLOCK_CTL_RESERVED0_FIELD_WIDTH,
    XPCSRX_RX_FRAMER_NOLOCK_CTL_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPCSRX_RX_FRAMER_NOLOCK_CTL_CFGXPCSRXFRMRNOLOCKLOSINTVAL
 ******************************************************************************/
const ru_field_rec XPCSRX_RX_FRAMER_NOLOCK_CTL_CFGXPCSRXFRMRNOLOCKLOSINTVAL_FIELD =
{
    "CFGXPCSRXFRMRNOLOCKLOSINTVAL",
#if RU_INCLUDE_DESC
    "",
    "Interval for LOS based on no lock found during this time.These are"
    "6.206 ns steps with default of 1ms.The counter is 24 bits with a max"
    "interval of 104 ms.",
#endif
    XPCSRX_RX_FRAMER_NOLOCK_CTL_CFGXPCSRXFRMRNOLOCKLOSINTVAL_FIELD_MASK,
    0,
    XPCSRX_RX_FRAMER_NOLOCK_CTL_CFGXPCSRXFRMRNOLOCKLOSINTVAL_FIELD_WIDTH,
    XPCSRX_RX_FRAMER_NOLOCK_CTL_CFGXPCSRXFRMRNOLOCKLOSINTVAL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPCSRX_RX_64B66B_IPG_DET_CNT_RX64B66BIPGDETCNT
 ******************************************************************************/
const ru_field_rec XPCSRX_RX_64B66B_IPG_DET_CNT_RX64B66BIPGDETCNT_FIELD =
{
    "RX64B66BIPGDETCNT",
#if RU_INCLUDE_DESC
    "",
    "This is the number of times that a realtionship between the EOP and"
    "SOP is found. The defalut is set to detect min-IPG violations.",
#endif
    XPCSRX_RX_64B66B_IPG_DET_CNT_RX64B66BIPGDETCNT_FIELD_MASK,
    0,
    XPCSRX_RX_64B66B_IPG_DET_CNT_RX64B66BIPGDETCNT_FIELD_WIDTH,
    XPCSRX_RX_64B66B_IPG_DET_CNT_RX64B66BIPGDETCNT_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPCSRX_RX_FEC_NQUE_IN_CNT_RXFECNQUEINCNT
 ******************************************************************************/
const ru_field_rec XPCSRX_RX_FEC_NQUE_IN_CNT_RXFECNQUEINCNT_FIELD =
{
    "RXFECNQUEINCNT",
#if RU_INCLUDE_DESC
    "",
    "The number of FEC CW written into the store/foward FIFO.",
#endif
    XPCSRX_RX_FEC_NQUE_IN_CNT_RXFECNQUEINCNT_FIELD_MASK,
    0,
    XPCSRX_RX_FEC_NQUE_IN_CNT_RXFECNQUEINCNT_FIELD_WIDTH,
    XPCSRX_RX_FEC_NQUE_IN_CNT_RXFECNQUEINCNT_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPCSRX_RX_FEC_NQUE_OUT_CNT_RXFECNQUEOUTCNT
 ******************************************************************************/
const ru_field_rec XPCSRX_RX_FEC_NQUE_OUT_CNT_RXFECNQUEOUTCNT_FIELD =
{
    "RXFECNQUEOUTCNT",
#if RU_INCLUDE_DESC
    "",
    "The number of FEC CW read from the store/foward FIFO.",
#endif
    XPCSRX_RX_FEC_NQUE_OUT_CNT_RXFECNQUEOUTCNT_FIELD_MASK,
    0,
    XPCSRX_RX_FEC_NQUE_OUT_CNT_RXFECNQUEOUTCNT_FIELD_WIDTH,
    XPCSRX_RX_FEC_NQUE_OUT_CNT_RXFECNQUEOUTCNT_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPCSRX_RX_IDLE_START_CNT_RXIDLESTARTCNT
 ******************************************************************************/
const ru_field_rec XPCSRX_RX_IDLE_START_CNT_RXIDLESTARTCNT_FIELD =
{
    "RXIDLESTARTCNT",
#if RU_INCLUDE_DESC
    "",
    "The number of SOP that the idle insertion logic detected.",
#endif
    XPCSRX_RX_IDLE_START_CNT_RXIDLESTARTCNT_FIELD_MASK,
    0,
    XPCSRX_RX_IDLE_START_CNT_RXIDLESTARTCNT_FIELD_WIDTH,
    XPCSRX_RX_IDLE_START_CNT_RXIDLESTARTCNT_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPCSRX_RX_IDLE_STOP_CNT_RXIDLESTOPCNT
 ******************************************************************************/
const ru_field_rec XPCSRX_RX_IDLE_STOP_CNT_RXIDLESTOPCNT_FIELD =
{
    "RXIDLESTOPCNT",
#if RU_INCLUDE_DESC
    "",
    "The number of EOP that the idle insertion logic detected.",
#endif
    XPCSRX_RX_IDLE_STOP_CNT_RXIDLESTOPCNT_FIELD_MASK,
    0,
    XPCSRX_RX_IDLE_STOP_CNT_RXIDLESTOPCNT_FIELD_WIDTH,
    XPCSRX_RX_IDLE_STOP_CNT_RXIDLESTOPCNT_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPCSRX_RX_FEC_COR_INTVAL_RESERVED0
 ******************************************************************************/
const ru_field_rec XPCSRX_RX_FEC_COR_INTVAL_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    XPCSRX_RX_FEC_COR_INTVAL_RESERVED0_FIELD_MASK,
    0,
    XPCSRX_RX_FEC_COR_INTVAL_RESERVED0_FIELD_WIDTH,
    XPCSRX_RX_FEC_COR_INTVAL_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPCSRX_RX_FEC_COR_INTVAL_CFGXPCSRXFECCORINTVAL
 ******************************************************************************/
const ru_field_rec XPCSRX_RX_FEC_COR_INTVAL_CFGXPCSRXFECCORINTVAL_FIELD =
{
    "CFGXPCSRXFECCORINTVAL",
#if RU_INCLUDE_DESC
    "",
    "Number of 161 MHz clock period for the ber interval (default is"
    "1ms).",
#endif
    XPCSRX_RX_FEC_COR_INTVAL_CFGXPCSRXFECCORINTVAL_FIELD_MASK,
    0,
    XPCSRX_RX_FEC_COR_INTVAL_CFGXPCSRXFECCORINTVAL_FIELD_WIDTH,
    XPCSRX_RX_FEC_COR_INTVAL_CFGXPCSRXFECCORINTVAL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPCSRX_RX_FEC_COR_TRESH_RESERVED0
 ******************************************************************************/
const ru_field_rec XPCSRX_RX_FEC_COR_TRESH_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    XPCSRX_RX_FEC_COR_TRESH_RESERVED0_FIELD_MASK,
    0,
    XPCSRX_RX_FEC_COR_TRESH_RESERVED0_FIELD_WIDTH,
    XPCSRX_RX_FEC_COR_TRESH_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPCSRX_RX_FEC_COR_TRESH_CFGXPCSRXFECCORTRESH
 ******************************************************************************/
const ru_field_rec XPCSRX_RX_FEC_COR_TRESH_CFGXPCSRXFECCORTRESH_FIELD =
{
    "CFGXPCSRXFECCORTRESH",
#if RU_INCLUDE_DESC
    "",
    "Number of FEC corrections made over a given time interval that will"
    "trigger the FEC high correction alarm (defalut = 0 = off).",
#endif
    XPCSRX_RX_FEC_COR_TRESH_CFGXPCSRXFECCORTRESH_FIELD_MASK,
    0,
    XPCSRX_RX_FEC_COR_TRESH_CFGXPCSRXFECCORTRESH_FIELD_WIDTH,
    XPCSRX_RX_FEC_COR_TRESH_CFGXPCSRXFECCORTRESH_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPCSRX_RX_FEC_CW_FAIL_CNT_RXFECDECCWFAILCNT
 ******************************************************************************/
const ru_field_rec XPCSRX_RX_FEC_CW_FAIL_CNT_RXFECDECCWFAILCNT_FIELD =
{
    "RXFECDECCWFAILCNT",
#if RU_INCLUDE_DESC
    "",
    "The number of uncorrectable FEC CW that have been recieved.",
#endif
    XPCSRX_RX_FEC_CW_FAIL_CNT_RXFECDECCWFAILCNT_FIELD_MASK,
    0,
    XPCSRX_RX_FEC_CW_FAIL_CNT_RXFECDECCWFAILCNT_FIELD_WIDTH,
    XPCSRX_RX_FEC_CW_FAIL_CNT_RXFECDECCWFAILCNT_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPCSRX_RX_FEC_CW_TOT_CNT_RXFECDECCWTOTCNT
 ******************************************************************************/
const ru_field_rec XPCSRX_RX_FEC_CW_TOT_CNT_RXFECDECCWTOTCNT_FIELD =
{
    "RXFECDECCWTOTCNT",
#if RU_INCLUDE_DESC
    "",
    "The number of total FEC CW that have been recieved.",
#endif
    XPCSRX_RX_FEC_CW_TOT_CNT_RXFECDECCWTOTCNT_FIELD_MASK,
    0,
    XPCSRX_RX_FEC_CW_TOT_CNT_RXFECDECCWTOTCNT_FIELD_WIDTH,
    XPCSRX_RX_FEC_CW_TOT_CNT_RXFECDECCWTOTCNT_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPCSRX_RX_FEC_CORRECT_CNT_LOWER_RXFECDECERRCORCNTLOWER
 ******************************************************************************/
const ru_field_rec XPCSRX_RX_FEC_CORRECT_CNT_LOWER_RXFECDECERRCORCNTLOWER_FIELD =
{
    "RXFECDECERRCORCNTLOWER",
#if RU_INCLUDE_DESC
    "",
    "Lower 32 bits of number of bits that the FEC corrected.",
#endif
    XPCSRX_RX_FEC_CORRECT_CNT_LOWER_RXFECDECERRCORCNTLOWER_FIELD_MASK,
    0,
    XPCSRX_RX_FEC_CORRECT_CNT_LOWER_RXFECDECERRCORCNTLOWER_FIELD_WIDTH,
    XPCSRX_RX_FEC_CORRECT_CNT_LOWER_RXFECDECERRCORCNTLOWER_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPCSRX_RX_FEC_CORRECT_CNT_UPPER_RESERVED0
 ******************************************************************************/
const ru_field_rec XPCSRX_RX_FEC_CORRECT_CNT_UPPER_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    XPCSRX_RX_FEC_CORRECT_CNT_UPPER_RESERVED0_FIELD_MASK,
    0,
    XPCSRX_RX_FEC_CORRECT_CNT_UPPER_RESERVED0_FIELD_WIDTH,
    XPCSRX_RX_FEC_CORRECT_CNT_UPPER_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: XPCSRX_RX_FEC_CORRECT_CNT_UPPER_RXFECDECERRCORCNTUPPER
 ******************************************************************************/
const ru_field_rec XPCSRX_RX_FEC_CORRECT_CNT_UPPER_RXFECDECERRCORCNTUPPER_FIELD =
{
    "RXFECDECERRCORCNTUPPER",
#if RU_INCLUDE_DESC
    "",
    "Upper 7 bits number of bits that the FEC corrected.",
#endif
    XPCSRX_RX_FEC_CORRECT_CNT_UPPER_RXFECDECERRCORCNTUPPER_FIELD_MASK,
    0,
    XPCSRX_RX_FEC_CORRECT_CNT_UPPER_RXFECDECERRCORCNTUPPER_FIELD_WIDTH,
    XPCSRX_RX_FEC_CORRECT_CNT_UPPER_RXFECDECERRCORCNTUPPER_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: XPCSRX_RX_FEC_CORRECT_CNT_SHADOW_RESERVED0
 ******************************************************************************/
const ru_field_rec XPCSRX_RX_FEC_CORRECT_CNT_SHADOW_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    XPCSRX_RX_FEC_CORRECT_CNT_SHADOW_RESERVED0_FIELD_MASK,
    0,
    XPCSRX_RX_FEC_CORRECT_CNT_SHADOW_RESERVED0_FIELD_WIDTH,
    XPCSRX_RX_FEC_CORRECT_CNT_SHADOW_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPCSRX_RX_FEC_CORRECT_CNT_SHADOW_RXFECDECERRCORCNTSHADOW
 ******************************************************************************/
const ru_field_rec XPCSRX_RX_FEC_CORRECT_CNT_SHADOW_RXFECDECERRCORCNTSHADOW_FIELD =
{
    "RXFECDECERRCORCNTSHADOW",
#if RU_INCLUDE_DESC
    "",
    "Upper 7 of bits that the FEC corrected.",
#endif
    XPCSRX_RX_FEC_CORRECT_CNT_SHADOW_RXFECDECERRCORCNTSHADOW_FIELD_MASK,
    0,
    XPCSRX_RX_FEC_CORRECT_CNT_SHADOW_RXFECDECERRCORCNTSHADOW_FIELD_WIDTH,
    XPCSRX_RX_FEC_CORRECT_CNT_SHADOW_RXFECDECERRCORCNTSHADOW_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPCSRX_RX_FEC_ONES_COR_CNT_LOWER_RXFECDECONESCORCNTLOWER
 ******************************************************************************/
const ru_field_rec XPCSRX_RX_FEC_ONES_COR_CNT_LOWER_RXFECDECONESCORCNTLOWER_FIELD =
{
    "RXFECDECONESCORCNTLOWER",
#if RU_INCLUDE_DESC
    "",
    "The number of ones that are correctd by the FEC decoder.",
#endif
    XPCSRX_RX_FEC_ONES_COR_CNT_LOWER_RXFECDECONESCORCNTLOWER_FIELD_MASK,
    0,
    XPCSRX_RX_FEC_ONES_COR_CNT_LOWER_RXFECDECONESCORCNTLOWER_FIELD_WIDTH,
    XPCSRX_RX_FEC_ONES_COR_CNT_LOWER_RXFECDECONESCORCNTLOWER_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPCSRX_RX_FEC_ONES_COR_CNT_UPPER_RESERVED0
 ******************************************************************************/
const ru_field_rec XPCSRX_RX_FEC_ONES_COR_CNT_UPPER_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    XPCSRX_RX_FEC_ONES_COR_CNT_UPPER_RESERVED0_FIELD_MASK,
    0,
    XPCSRX_RX_FEC_ONES_COR_CNT_UPPER_RESERVED0_FIELD_WIDTH,
    XPCSRX_RX_FEC_ONES_COR_CNT_UPPER_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: XPCSRX_RX_FEC_ONES_COR_CNT_UPPER_RXFECDECONESCORCNTUPPER
 ******************************************************************************/
const ru_field_rec XPCSRX_RX_FEC_ONES_COR_CNT_UPPER_RXFECDECONESCORCNTUPPER_FIELD =
{
    "RXFECDECONESCORCNTUPPER",
#if RU_INCLUDE_DESC
    "",
    "The number of ones that are correctd by the FEC decoder.",
#endif
    XPCSRX_RX_FEC_ONES_COR_CNT_UPPER_RXFECDECONESCORCNTUPPER_FIELD_MASK,
    0,
    XPCSRX_RX_FEC_ONES_COR_CNT_UPPER_RXFECDECONESCORCNTUPPER_FIELD_WIDTH,
    XPCSRX_RX_FEC_ONES_COR_CNT_UPPER_RXFECDECONESCORCNTUPPER_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: XPCSRX_RX_FEC_ONES_COR_CNT_SHADOW_RESERVED0
 ******************************************************************************/
const ru_field_rec XPCSRX_RX_FEC_ONES_COR_CNT_SHADOW_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    XPCSRX_RX_FEC_ONES_COR_CNT_SHADOW_RESERVED0_FIELD_MASK,
    0,
    XPCSRX_RX_FEC_ONES_COR_CNT_SHADOW_RESERVED0_FIELD_WIDTH,
    XPCSRX_RX_FEC_ONES_COR_CNT_SHADOW_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPCSRX_RX_FEC_ONES_COR_CNT_SHADOW_RXFECDECONESCORCNTSHADOW
 ******************************************************************************/
const ru_field_rec XPCSRX_RX_FEC_ONES_COR_CNT_SHADOW_RXFECDECONESCORCNTSHADOW_FIELD =
{
    "RXFECDECONESCORCNTSHADOW",
#if RU_INCLUDE_DESC
    "",
    "The number of ones that are correctd by the FEC decoder.",
#endif
    XPCSRX_RX_FEC_ONES_COR_CNT_SHADOW_RXFECDECONESCORCNTSHADOW_FIELD_MASK,
    0,
    XPCSRX_RX_FEC_ONES_COR_CNT_SHADOW_RXFECDECONESCORCNTSHADOW_FIELD_WIDTH,
    XPCSRX_RX_FEC_ONES_COR_CNT_SHADOW_RXFECDECONESCORCNTSHADOW_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPCSRX_RX_FEC_ZEROS_COR_CNT_LOWER_RXFECDECZEROSCORCNTLOWER
 ******************************************************************************/
const ru_field_rec XPCSRX_RX_FEC_ZEROS_COR_CNT_LOWER_RXFECDECZEROSCORCNTLOWER_FIELD =
{
    "RXFECDECZEROSCORCNTLOWER",
#if RU_INCLUDE_DESC
    "",
    "The number of zeros that are correctd by the FEC decoder.",
#endif
    XPCSRX_RX_FEC_ZEROS_COR_CNT_LOWER_RXFECDECZEROSCORCNTLOWER_FIELD_MASK,
    0,
    XPCSRX_RX_FEC_ZEROS_COR_CNT_LOWER_RXFECDECZEROSCORCNTLOWER_FIELD_WIDTH,
    XPCSRX_RX_FEC_ZEROS_COR_CNT_LOWER_RXFECDECZEROSCORCNTLOWER_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPCSRX_RX_FEC_ZEROS_COR_CNT_UPPER_RESERVED0
 ******************************************************************************/
const ru_field_rec XPCSRX_RX_FEC_ZEROS_COR_CNT_UPPER_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    XPCSRX_RX_FEC_ZEROS_COR_CNT_UPPER_RESERVED0_FIELD_MASK,
    0,
    XPCSRX_RX_FEC_ZEROS_COR_CNT_UPPER_RESERVED0_FIELD_WIDTH,
    XPCSRX_RX_FEC_ZEROS_COR_CNT_UPPER_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: XPCSRX_RX_FEC_ZEROS_COR_CNT_UPPER_RXFECDECZEROSCORCNTUPPER
 ******************************************************************************/
const ru_field_rec XPCSRX_RX_FEC_ZEROS_COR_CNT_UPPER_RXFECDECZEROSCORCNTUPPER_FIELD =
{
    "RXFECDECZEROSCORCNTUPPER",
#if RU_INCLUDE_DESC
    "",
    "The number of zeros that are correctd by the FEC decoder.",
#endif
    XPCSRX_RX_FEC_ZEROS_COR_CNT_UPPER_RXFECDECZEROSCORCNTUPPER_FIELD_MASK,
    0,
    XPCSRX_RX_FEC_ZEROS_COR_CNT_UPPER_RXFECDECZEROSCORCNTUPPER_FIELD_WIDTH,
    XPCSRX_RX_FEC_ZEROS_COR_CNT_UPPER_RXFECDECZEROSCORCNTUPPER_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: XPCSRX_RX_FEC_ZEROS_COR_CNT_SHADOW_RESERVED0
 ******************************************************************************/
const ru_field_rec XPCSRX_RX_FEC_ZEROS_COR_CNT_SHADOW_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    XPCSRX_RX_FEC_ZEROS_COR_CNT_SHADOW_RESERVED0_FIELD_MASK,
    0,
    XPCSRX_RX_FEC_ZEROS_COR_CNT_SHADOW_RESERVED0_FIELD_WIDTH,
    XPCSRX_RX_FEC_ZEROS_COR_CNT_SHADOW_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPCSRX_RX_FEC_ZEROS_COR_CNT_SHADOW_RXFECDECZEROSCORCNTSHADOW
 ******************************************************************************/
const ru_field_rec XPCSRX_RX_FEC_ZEROS_COR_CNT_SHADOW_RXFECDECZEROSCORCNTSHADOW_FIELD =
{
    "RXFECDECZEROSCORCNTSHADOW",
#if RU_INCLUDE_DESC
    "",
    "The number of zeros that are correctd by the FEC decoder.",
#endif
    XPCSRX_RX_FEC_ZEROS_COR_CNT_SHADOW_RXFECDECZEROSCORCNTSHADOW_FIELD_MASK,
    0,
    XPCSRX_RX_FEC_ZEROS_COR_CNT_SHADOW_RXFECDECZEROSCORCNTSHADOW_FIELD_WIDTH,
    XPCSRX_RX_FEC_ZEROS_COR_CNT_SHADOW_RXFECDECZEROSCORCNTSHADOW_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPCSRX_RX_FEC_STOP_ON_ERR_READ_POINTER_RESERVED0
 ******************************************************************************/
const ru_field_rec XPCSRX_RX_FEC_STOP_ON_ERR_READ_POINTER_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    XPCSRX_RX_FEC_STOP_ON_ERR_READ_POINTER_RESERVED0_FIELD_MASK,
    0,
    XPCSRX_RX_FEC_STOP_ON_ERR_READ_POINTER_RESERVED0_FIELD_WIDTH,
    XPCSRX_RX_FEC_STOP_ON_ERR_READ_POINTER_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: XPCSRX_RX_FEC_STOP_ON_ERR_READ_POINTER_RXFECSTOPONERRRDPTR
 ******************************************************************************/
const ru_field_rec XPCSRX_RX_FEC_STOP_ON_ERR_READ_POINTER_RXFECSTOPONERRRDPTR_FIELD =
{
    "RXFECSTOPONERRRDPTR",
#if RU_INCLUDE_DESC
    "",
    "Captures the read pointer for the FEC decoder when a fail decode"
    "happens."
    "This is for the feature that freezes the FEC decoder when decodes"
    "fails."
    "Requires cfgXPcsRxFecStopOnErr = 1.",
#endif
    XPCSRX_RX_FEC_STOP_ON_ERR_READ_POINTER_RXFECSTOPONERRRDPTR_FIELD_MASK,
    0,
    XPCSRX_RX_FEC_STOP_ON_ERR_READ_POINTER_RXFECSTOPONERRRDPTR_FIELD_WIDTH,
    XPCSRX_RX_FEC_STOP_ON_ERR_READ_POINTER_RXFECSTOPONERRRDPTR_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: XPCSRX_RX_FEC_STOP_ON_ERR_READ_POINTER_RXFECSTOPONERRWRPTR
 ******************************************************************************/
const ru_field_rec XPCSRX_RX_FEC_STOP_ON_ERR_READ_POINTER_RXFECSTOPONERRWRPTR_FIELD =
{
    "RXFECSTOPONERRWRPTR",
#if RU_INCLUDE_DESC
    "",
    "Captures the write pointer for the FEC decoder when a fail decode"
    "happens."
    "This is for the feature that freezes the FEC decoder when decodes"
    "fails."
    "Requires cfgXPcsRxFecStopOnErr = 1.",
#endif
    XPCSRX_RX_FEC_STOP_ON_ERR_READ_POINTER_RXFECSTOPONERRWRPTR_FIELD_MASK,
    0,
    XPCSRX_RX_FEC_STOP_ON_ERR_READ_POINTER_RXFECSTOPONERRWRPTR_FIELD_WIDTH,
    XPCSRX_RX_FEC_STOP_ON_ERR_READ_POINTER_RXFECSTOPONERRWRPTR_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: XPCSRX_RX_FEC_STOP_ON_ERR_BURST_LOCATION_RESERVED0
 ******************************************************************************/
const ru_field_rec XPCSRX_RX_FEC_STOP_ON_ERR_BURST_LOCATION_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    XPCSRX_RX_FEC_STOP_ON_ERR_BURST_LOCATION_RESERVED0_FIELD_MASK,
    0,
    XPCSRX_RX_FEC_STOP_ON_ERR_BURST_LOCATION_RESERVED0_FIELD_WIDTH,
    XPCSRX_RX_FEC_STOP_ON_ERR_BURST_LOCATION_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: XPCSRX_RX_FEC_STOP_ON_ERR_BURST_LOCATION_RXFECSTOPONERRBRSTLOC
 ******************************************************************************/
const ru_field_rec XPCSRX_RX_FEC_STOP_ON_ERR_BURST_LOCATION_RXFECSTOPONERRBRSTLOC_FIELD =
{
    "RXFECSTOPONERRBRSTLOC",
#if RU_INCLUDE_DESC
    "",
    "Captures the location witing hte burst where the stop on error"
    "occured."
    "This is in ticks of 161 clocks.  Requires cfgXPcsRxFecStopOnErr = 1.",
#endif
    XPCSRX_RX_FEC_STOP_ON_ERR_BURST_LOCATION_RXFECSTOPONERRBRSTLOC_FIELD_MASK,
    0,
    XPCSRX_RX_FEC_STOP_ON_ERR_BURST_LOCATION_RXFECSTOPONERRBRSTLOC_FIELD_WIDTH,
    XPCSRX_RX_FEC_STOP_ON_ERR_BURST_LOCATION_RXFECSTOPONERRBRSTLOC_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: XPCSRX_RX_64B66B_FAIL_CNT_RX64B66BDECERRCNT
 ******************************************************************************/
const ru_field_rec XPCSRX_RX_64B66B_FAIL_CNT_RX64B66BDECERRCNT_FIELD =
{
    "RX64B66BDECERRCNT",
#if RU_INCLUDE_DESC
    "",
    "The number of 64b/66b decoed errors.",
#endif
    XPCSRX_RX_64B66B_FAIL_CNT_RX64B66BDECERRCNT_FIELD_MASK,
    0,
    XPCSRX_RX_64B66B_FAIL_CNT_RX64B66BDECERRCNT_FIELD_WIDTH,
    XPCSRX_RX_64B66B_FAIL_CNT_RX64B66BDECERRCNT_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPCSRX_RX_FRMR_BAD_SH_CNT_RXFRMRBADSHCNT
 ******************************************************************************/
const ru_field_rec XPCSRX_RX_FRMR_BAD_SH_CNT_RXFRMRBADSHCNT_FIELD =
{
    "RXFRMRBADSHCNT",
#if RU_INCLUDE_DESC
    "",
    "The number bad SH while in CW lock.",
#endif
    XPCSRX_RX_FRMR_BAD_SH_CNT_RXFRMRBADSHCNT_FIELD_MASK,
    0,
    XPCSRX_RX_FRMR_BAD_SH_CNT_RXFRMRBADSHCNT_FIELD_WIDTH,
    XPCSRX_RX_FRMR_BAD_SH_CNT_RXFRMRBADSHCNT_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPCSRX_RX_PSUDO_CNT_RXTESTPSUDOERRCNT
 ******************************************************************************/
const ru_field_rec XPCSRX_RX_PSUDO_CNT_RXTESTPSUDOERRCNT_FIELD =
{
    "RXTESTPSUDOERRCNT",
#if RU_INCLUDE_DESC
    "",
    "The number of errors in test pattern 2 while in pattern lock.",
#endif
    XPCSRX_RX_PSUDO_CNT_RXTESTPSUDOERRCNT_FIELD_MASK,
    0,
    XPCSRX_RX_PSUDO_CNT_RXTESTPSUDOERRCNT_FIELD_WIDTH,
    XPCSRX_RX_PSUDO_CNT_RXTESTPSUDOERRCNT_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPCSRX_RX_PRBS_CNT_RXTESTPRBSERRCNT
 ******************************************************************************/
const ru_field_rec XPCSRX_RX_PRBS_CNT_RXTESTPRBSERRCNT_FIELD =
{
    "RXTESTPRBSERRCNT",
#if RU_INCLUDE_DESC
    "",
    "The number of errors in test pattern PRBS-31 while in patern lock.",
#endif
    XPCSRX_RX_PRBS_CNT_RXTESTPRBSERRCNT_FIELD_MASK,
    0,
    XPCSRX_RX_PRBS_CNT_RXTESTPRBSERRCNT_FIELD_WIDTH,
    XPCSRX_RX_PRBS_CNT_RXTESTPRBSERRCNT_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPCSRX_RX_BER_INTVAL_RESERVED0
 ******************************************************************************/
const ru_field_rec XPCSRX_RX_BER_INTVAL_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    XPCSRX_RX_BER_INTVAL_RESERVED0_FIELD_MASK,
    0,
    XPCSRX_RX_BER_INTVAL_RESERVED0_FIELD_WIDTH,
    XPCSRX_RX_BER_INTVAL_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPCSRX_RX_BER_INTVAL_CFGXPCSRXFRMRBERINTVAL
 ******************************************************************************/
const ru_field_rec XPCSRX_RX_BER_INTVAL_CFGXPCSRXFRMRBERINTVAL_FIELD =
{
    "CFGXPCSRXFRMRBERINTVAL",
#if RU_INCLUDE_DESC
    "",
    "Number of 161 MHz clock period for the interval over which SH BER is"
    "counter in the framer(default is 256 us).",
#endif
    XPCSRX_RX_BER_INTVAL_CFGXPCSRXFRMRBERINTVAL_FIELD_MASK,
    0,
    XPCSRX_RX_BER_INTVAL_CFGXPCSRXFRMRBERINTVAL_FIELD_WIDTH,
    XPCSRX_RX_BER_INTVAL_CFGXPCSRXFRMRBERINTVAL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPCSRX_RX_BER_TRESH_RESERVED0
 ******************************************************************************/
const ru_field_rec XPCSRX_RX_BER_TRESH_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    XPCSRX_RX_BER_TRESH_RESERVED0_FIELD_MASK,
    0,
    XPCSRX_RX_BER_TRESH_RESERVED0_FIELD_WIDTH,
    XPCSRX_RX_BER_TRESH_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPCSRX_RX_BER_TRESH_CFGXPCSRXFRMRBERTRESH
 ******************************************************************************/
const ru_field_rec XPCSRX_RX_BER_TRESH_CFGXPCSRXFRMRBERTRESH_FIELD =
{
    "CFGXPCSRXFRMRBERTRESH",
#if RU_INCLUDE_DESC
    "",
    "Number of SH error permitted over the BER interval. (defalut = 0 ="
    "off).",
#endif
    XPCSRX_RX_BER_TRESH_CFGXPCSRXFRMRBERTRESH_FIELD_MASK,
    0,
    XPCSRX_RX_BER_TRESH_CFGXPCSRXFRMRBERTRESH_FIELD_WIDTH,
    XPCSRX_RX_BER_TRESH_CFGXPCSRXFRMRBERTRESH_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPCSRX_RX_64B66B_START_CNT_RX64B66BDECSTARTCNT
 ******************************************************************************/
const ru_field_rec XPCSRX_RX_64B66B_START_CNT_RX64B66BDECSTARTCNT_FIELD =
{
    "RX64B66BDECSTARTCNT",
#if RU_INCLUDE_DESC
    "",
    "The number of SOP that the 64b/66b decoder found.",
#endif
    XPCSRX_RX_64B66B_START_CNT_RX64B66BDECSTARTCNT_FIELD_MASK,
    0,
    XPCSRX_RX_64B66B_START_CNT_RX64B66BDECSTARTCNT_FIELD_WIDTH,
    XPCSRX_RX_64B66B_START_CNT_RX64B66BDECSTARTCNT_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPCSRX_RX_IDLE_GOOD_PKT_CNT_RXIDLEGOODPKTCNT
 ******************************************************************************/
const ru_field_rec XPCSRX_RX_IDLE_GOOD_PKT_CNT_RXIDLEGOODPKTCNT_FIELD =
{
    "RXIDLEGOODPKTCNT",
#if RU_INCLUDE_DESC
    "",
    "The number of good packets found by the idle insertion logic.",
#endif
    XPCSRX_RX_IDLE_GOOD_PKT_CNT_RXIDLEGOODPKTCNT_FIELD_MASK,
    0,
    XPCSRX_RX_IDLE_GOOD_PKT_CNT_RXIDLEGOODPKTCNT_FIELD_WIDTH,
    XPCSRX_RX_IDLE_GOOD_PKT_CNT_RXIDLEGOODPKTCNT_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPCSRX_RX_IDLE_ERR_PKT_CNT_RXIDLEERRPKTCNT
 ******************************************************************************/
const ru_field_rec XPCSRX_RX_IDLE_ERR_PKT_CNT_RXIDLEERRPKTCNT_FIELD =
{
    "RXIDLEERRPKTCNT",
#if RU_INCLUDE_DESC
    "",
    "The number of errored packets found by the idle insertion logic.",
#endif
    XPCSRX_RX_IDLE_ERR_PKT_CNT_RXIDLEERRPKTCNT_FIELD_MASK,
    0,
    XPCSRX_RX_IDLE_ERR_PKT_CNT_RXIDLEERRPKTCNT_FIELD_WIDTH,
    XPCSRX_RX_IDLE_ERR_PKT_CNT_RXIDLEERRPKTCNT_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPCSRX_RX_64B66B_STOP_CNT_RX64B66BDECSTOPCNT
 ******************************************************************************/
const ru_field_rec XPCSRX_RX_64B66B_STOP_CNT_RX64B66BDECSTOPCNT_FIELD =
{
    "RX64B66BDECSTOPCNT",
#if RU_INCLUDE_DESC
    "",
    "The number of EOP that the 64b/66b decoder found.",
#endif
    XPCSRX_RX_64B66B_STOP_CNT_RX64B66BDECSTOPCNT_FIELD_MASK,
    0,
    XPCSRX_RX_64B66B_STOP_CNT_RX64B66BDECSTOPCNT_FIELD_WIDTH,
    XPCSRX_RX_64B66B_STOP_CNT_RX64B66BDECSTOPCNT_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPCSRX_RX_BURST_OUT_ODR_CNT_RXBURSTSEQOUTOFORDERCNT
 ******************************************************************************/
const ru_field_rec XPCSRX_RX_BURST_OUT_ODR_CNT_RXBURSTSEQOUTOFORDERCNT_FIELD =
{
    "RXBURSTSEQOUTOFORDERCNT",
#if RU_INCLUDE_DESC
    "",
    "The number of times recieved burst sequence number was out of order.",
#endif
    XPCSRX_RX_BURST_OUT_ODR_CNT_RXBURSTSEQOUTOFORDERCNT_FIELD_MASK,
    0,
    XPCSRX_RX_BURST_OUT_ODR_CNT_RXBURSTSEQOUTOFORDERCNT_FIELD_WIDTH,
    XPCSRX_RX_BURST_OUT_ODR_CNT_RXBURSTSEQOUTOFORDERCNT_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPCSRX_RX_IDLE_DA_JIT_DLY_RESERVED0
 ******************************************************************************/
const ru_field_rec XPCSRX_RX_IDLE_DA_JIT_DLY_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    XPCSRX_RX_IDLE_DA_JIT_DLY_RESERVED0_FIELD_MASK,
    0,
    XPCSRX_RX_IDLE_DA_JIT_DLY_RESERVED0_FIELD_WIDTH,
    XPCSRX_RX_IDLE_DA_JIT_DLY_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: XPCSRX_RX_IDLE_DA_JIT_DLY_RXIDLELASTDACNT
 ******************************************************************************/
const ru_field_rec XPCSRX_RX_IDLE_DA_JIT_DLY_RXIDLELASTDACNT_FIELD =
{
    "RXIDLELASTDACNT",
#if RU_INCLUDE_DESC
    "",
    "Previous delay for DA through the idle insert process.",
#endif
    XPCSRX_RX_IDLE_DA_JIT_DLY_RXIDLELASTDACNT_FIELD_MASK,
    0,
    XPCSRX_RX_IDLE_DA_JIT_DLY_RXIDLELASTDACNT_FIELD_WIDTH,
    XPCSRX_RX_IDLE_DA_JIT_DLY_RXIDLELASTDACNT_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: XPCSRX_RX_IDLE_DA_JIT_DLY_RESERVED1
 ******************************************************************************/
const ru_field_rec XPCSRX_RX_IDLE_DA_JIT_DLY_RESERVED1_FIELD =
{
    "RESERVED1",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    XPCSRX_RX_IDLE_DA_JIT_DLY_RESERVED1_FIELD_MASK,
    0,
    XPCSRX_RX_IDLE_DA_JIT_DLY_RESERVED1_FIELD_WIDTH,
    XPCSRX_RX_IDLE_DA_JIT_DLY_RESERVED1_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: XPCSRX_RX_IDLE_DA_JIT_DLY_RXIDLEDACNT
 ******************************************************************************/
const ru_field_rec XPCSRX_RX_IDLE_DA_JIT_DLY_RXIDLEDACNT_FIELD =
{
    "RXIDLEDACNT",
#if RU_INCLUDE_DESC
    "",
    "Current delay for DA through the idle insert process.",
#endif
    XPCSRX_RX_IDLE_DA_JIT_DLY_RXIDLEDACNT_FIELD_MASK,
    0,
    XPCSRX_RX_IDLE_DA_JIT_DLY_RXIDLEDACNT_FIELD_WIDTH,
    XPCSRX_RX_IDLE_DA_JIT_DLY_RXIDLEDACNT_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_read
#endif
};

/******************************************************************************
 * Field: XPCSRX_RX_DPORT_CTL_XPCSRXDPBUSY
 ******************************************************************************/
const ru_field_rec XPCSRX_RX_DPORT_CTL_XPCSRXDPBUSY_FIELD =
{
    "XPCSRXDPBUSY",
#if RU_INCLUDE_DESC
    "",
    "Data port busy.",
#endif
    XPCSRX_RX_DPORT_CTL_XPCSRXDPBUSY_FIELD_MASK,
    0,
    XPCSRX_RX_DPORT_CTL_XPCSRXDPBUSY_FIELD_WIDTH,
    XPCSRX_RX_DPORT_CTL_XPCSRXDPBUSY_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPCSRX_RX_DPORT_CTL_XPCSRXDPERR
 ******************************************************************************/
const ru_field_rec XPCSRX_RX_DPORT_CTL_XPCSRXDPERR_FIELD =
{
    "XPCSRXDPERR",
#if RU_INCLUDE_DESC
    "",
    "Data port error (always 0 for XPCS RX).",
#endif
    XPCSRX_RX_DPORT_CTL_XPCSRXDPERR_FIELD_MASK,
    0,
    XPCSRX_RX_DPORT_CTL_XPCSRXDPERR_FIELD_WIDTH,
    XPCSRX_RX_DPORT_CTL_XPCSRXDPERR_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPCSRX_RX_DPORT_CTL_RESERVED0
 ******************************************************************************/
const ru_field_rec XPCSRX_RX_DPORT_CTL_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    XPCSRX_RX_DPORT_CTL_RESERVED0_FIELD_MASK,
    0,
    XPCSRX_RX_DPORT_CTL_RESERVED0_FIELD_WIDTH,
    XPCSRX_RX_DPORT_CTL_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPCSRX_RX_DPORT_CTL_CFGXPCSRXDPCTL
 ******************************************************************************/
const ru_field_rec XPCSRX_RX_DPORT_CTL_CFGXPCSRXDPCTL_FIELD =
{
    "CFGXPCSRXDPCTL",
#if RU_INCLUDE_DESC
    "",
    "Data port command (0 = read and 1 = write).",
#endif
    XPCSRX_RX_DPORT_CTL_CFGXPCSRXDPCTL_FIELD_MASK,
    0,
    XPCSRX_RX_DPORT_CTL_CFGXPCSRXDPCTL_FIELD_WIDTH,
    XPCSRX_RX_DPORT_CTL_CFGXPCSRXDPCTL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPCSRX_RX_DPORT_CTL_CFGXPCSRXDPRAMSEL
 ******************************************************************************/
const ru_field_rec XPCSRX_RX_DPORT_CTL_CFGXPCSRXDPRAMSEL_FIELD =
{
    "CFGXPCSRXDPRAMSEL",
#if RU_INCLUDE_DESC
    "",
    "Data port RAM select :"
    "0 = capture FIFO RAM"
    "1 = FEC decode RAM"
    "2 = FEC stats RAM"
    "3 = FEC enqueue RAM"
    "4 = idle insert RAM",
#endif
    XPCSRX_RX_DPORT_CTL_CFGXPCSRXDPRAMSEL_FIELD_MASK,
    0,
    XPCSRX_RX_DPORT_CTL_CFGXPCSRXDPRAMSEL_FIELD_WIDTH,
    XPCSRX_RX_DPORT_CTL_CFGXPCSRXDPRAMSEL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPCSRX_RX_DPORT_CTL_CFGXPCSRXDPADDR
 ******************************************************************************/
const ru_field_rec XPCSRX_RX_DPORT_CTL_CFGXPCSRXDPADDR_FIELD =
{
    "CFGXPCSRXDPADDR",
#if RU_INCLUDE_DESC
    "",
    "Data port address."
    "NOTE: all 16 bits are available but the largest ram in XPCS RX"
    "XPcsRxCapFifoRam.vbis x256.",
#endif
    XPCSRX_RX_DPORT_CTL_CFGXPCSRXDPADDR_FIELD_MASK,
    0,
    XPCSRX_RX_DPORT_CTL_CFGXPCSRXDPADDR_FIELD_WIDTH,
    XPCSRX_RX_DPORT_CTL_CFGXPCSRXDPADDR_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPCSRX_RX_DPORT_DATA0_XPCSRXDPDATA0
 ******************************************************************************/
const ru_field_rec XPCSRX_RX_DPORT_DATA0_XPCSRXDPDATA0_FIELD =
{
    "XPCSRXDPDATA0",
#if RU_INCLUDE_DESC
    "",
    "Data port data bits 31 to 0.",
#endif
    XPCSRX_RX_DPORT_DATA0_XPCSRXDPDATA0_FIELD_MASK,
    0,
    XPCSRX_RX_DPORT_DATA0_XPCSRXDPDATA0_FIELD_WIDTH,
    XPCSRX_RX_DPORT_DATA0_XPCSRXDPDATA0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPCSRX_RX_DPORT_DATA1_XPCSRXDPDATA1
 ******************************************************************************/
const ru_field_rec XPCSRX_RX_DPORT_DATA1_XPCSRXDPDATA1_FIELD =
{
    "XPCSRXDPDATA1",
#if RU_INCLUDE_DESC
    "",
    "Data port data bits 63 to 32.",
#endif
    XPCSRX_RX_DPORT_DATA1_XPCSRXDPDATA1_FIELD_MASK,
    0,
    XPCSRX_RX_DPORT_DATA1_XPCSRXDPDATA1_FIELD_WIDTH,
    XPCSRX_RX_DPORT_DATA1_XPCSRXDPDATA1_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPCSRX_RX_DPORT_DATA2_XPCSRXDPDATA2
 ******************************************************************************/
const ru_field_rec XPCSRX_RX_DPORT_DATA2_XPCSRXDPDATA2_FIELD =
{
    "XPCSRXDPDATA2",
#if RU_INCLUDE_DESC
    "",
    "Data port data bits 95 to 64.",
#endif
    XPCSRX_RX_DPORT_DATA2_XPCSRXDPDATA2_FIELD_MASK,
    0,
    XPCSRX_RX_DPORT_DATA2_XPCSRXDPDATA2_FIELD_WIDTH,
    XPCSRX_RX_DPORT_DATA2_XPCSRXDPDATA2_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPCSRX_RX_DPORT_ACC_RESERVED0
 ******************************************************************************/
const ru_field_rec XPCSRX_RX_DPORT_ACC_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    XPCSRX_RX_DPORT_ACC_RESERVED0_FIELD_MASK,
    0,
    XPCSRX_RX_DPORT_ACC_RESERVED0_FIELD_WIDTH,
    XPCSRX_RX_DPORT_ACC_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPCSRX_RX_DPORT_ACC_CFGXPCSRXIDLERAMDPSEL
 ******************************************************************************/
const ru_field_rec XPCSRX_RX_DPORT_ACC_CFGXPCSRXIDLERAMDPSEL_FIELD =
{
    "CFGXPCSRXIDLERAMDPSEL",
#if RU_INCLUDE_DESC
    "",
    "Disable data path and selects data port for RAM access.",
#endif
    XPCSRX_RX_DPORT_ACC_CFGXPCSRXIDLERAMDPSEL_FIELD_MASK,
    0,
    XPCSRX_RX_DPORT_ACC_CFGXPCSRXIDLERAMDPSEL_FIELD_WIDTH,
    XPCSRX_RX_DPORT_ACC_CFGXPCSRXIDLERAMDPSEL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPCSRX_RX_DPORT_ACC_CFGXPCSRXFECDECRAMDPSEL
 ******************************************************************************/
const ru_field_rec XPCSRX_RX_DPORT_ACC_CFGXPCSRXFECDECRAMDPSEL_FIELD =
{
    "CFGXPCSRXFECDECRAMDPSEL",
#if RU_INCLUDE_DESC
    "",
    "Disable data path and selects data port for RAM access.",
#endif
    XPCSRX_RX_DPORT_ACC_CFGXPCSRXFECDECRAMDPSEL_FIELD_MASK,
    0,
    XPCSRX_RX_DPORT_ACC_CFGXPCSRXFECDECRAMDPSEL_FIELD_WIDTH,
    XPCSRX_RX_DPORT_ACC_CFGXPCSRXFECDECRAMDPSEL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPCSRX_RX_DPORT_ACC_CFGXPCSRXFECNQUERAMDPSEL
 ******************************************************************************/
const ru_field_rec XPCSRX_RX_DPORT_ACC_CFGXPCSRXFECNQUERAMDPSEL_FIELD =
{
    "CFGXPCSRXFECNQUERAMDPSEL",
#if RU_INCLUDE_DESC
    "",
    "Disable data path and selects data port for RAM access.",
#endif
    XPCSRX_RX_DPORT_ACC_CFGXPCSRXFECNQUERAMDPSEL_FIELD_MASK,
    0,
    XPCSRX_RX_DPORT_ACC_CFGXPCSRXFECNQUERAMDPSEL_FIELD_WIDTH,
    XPCSRX_RX_DPORT_ACC_CFGXPCSRXFECNQUERAMDPSEL_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPCSRX_RX_RAM_ECC_INT_STAT_RESERVED0
 ******************************************************************************/
const ru_field_rec XPCSRX_RX_RAM_ECC_INT_STAT_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    XPCSRX_RX_RAM_ECC_INT_STAT_RESERVED0_FIELD_MASK,
    0,
    XPCSRX_RX_RAM_ECC_INT_STAT_RESERVED0_FIELD_WIDTH,
    XPCSRX_RX_RAM_ECC_INT_STAT_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPCSRX_RX_RAM_ECC_INT_STAT_INTRXIDLERAMINITDONE
 ******************************************************************************/
const ru_field_rec XPCSRX_RX_RAM_ECC_INT_STAT_INTRXIDLERAMINITDONE_FIELD =
{
    "INTRXIDLERAMINITDONE",
#if RU_INCLUDE_DESC
    "",
    "Idle insert FIFO RAM init done interrupt.",
#endif
    XPCSRX_RX_RAM_ECC_INT_STAT_INTRXIDLERAMINITDONE_FIELD_MASK,
    0,
    XPCSRX_RX_RAM_ECC_INT_STAT_INTRXIDLERAMINITDONE_FIELD_WIDTH,
    XPCSRX_RX_RAM_ECC_INT_STAT_INTRXIDLERAMINITDONE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPCSRX_RX_RAM_ECC_INT_STAT_INTRXFECNQUERAMINITDONE
 ******************************************************************************/
const ru_field_rec XPCSRX_RX_RAM_ECC_INT_STAT_INTRXFECNQUERAMINITDONE_FIELD =
{
    "INTRXFECNQUERAMINITDONE",
#if RU_INCLUDE_DESC
    "",
    "FEC enqueue FIFO RAM init done interrupt.",
#endif
    XPCSRX_RX_RAM_ECC_INT_STAT_INTRXFECNQUERAMINITDONE_FIELD_MASK,
    0,
    XPCSRX_RX_RAM_ECC_INT_STAT_INTRXFECNQUERAMINITDONE_FIELD_WIDTH,
    XPCSRX_RX_RAM_ECC_INT_STAT_INTRXFECNQUERAMINITDONE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPCSRX_RX_RAM_ECC_INT_STAT_RESERVED1
 ******************************************************************************/
const ru_field_rec XPCSRX_RX_RAM_ECC_INT_STAT_RESERVED1_FIELD =
{
    "RESERVED1",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    XPCSRX_RX_RAM_ECC_INT_STAT_RESERVED1_FIELD_MASK,
    0,
    XPCSRX_RX_RAM_ECC_INT_STAT_RESERVED1_FIELD_WIDTH,
    XPCSRX_RX_RAM_ECC_INT_STAT_RESERVED1_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPCSRX_RX_RAM_ECC_INT_STAT_INTRXFECDECRAMINITDONE
 ******************************************************************************/
const ru_field_rec XPCSRX_RX_RAM_ECC_INT_STAT_INTRXFECDECRAMINITDONE_FIELD =
{
    "INTRXFECDECRAMINITDONE",
#if RU_INCLUDE_DESC
    "",
    "FEC decode FIFO RAM init done interrupt.",
#endif
    XPCSRX_RX_RAM_ECC_INT_STAT_INTRXFECDECRAMINITDONE_FIELD_MASK,
    0,
    XPCSRX_RX_RAM_ECC_INT_STAT_INTRXFECDECRAMINITDONE_FIELD_WIDTH,
    XPCSRX_RX_RAM_ECC_INT_STAT_INTRXFECDECRAMINITDONE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPCSRX_RX_RAM_ECC_INT_MSK_RESERVED0
 ******************************************************************************/
const ru_field_rec XPCSRX_RX_RAM_ECC_INT_MSK_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    XPCSRX_RX_RAM_ECC_INT_MSK_RESERVED0_FIELD_MASK,
    0,
    XPCSRX_RX_RAM_ECC_INT_MSK_RESERVED0_FIELD_WIDTH,
    XPCSRX_RX_RAM_ECC_INT_MSK_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPCSRX_RX_RAM_ECC_INT_MSK_MSKRXIDLERAMINITDONE
 ******************************************************************************/
const ru_field_rec XPCSRX_RX_RAM_ECC_INT_MSK_MSKRXIDLERAMINITDONE_FIELD =
{
    "MSKRXIDLERAMINITDONE",
#if RU_INCLUDE_DESC
    "",
    "Idle insert FIFO RAM init done interrupt mask.",
#endif
    XPCSRX_RX_RAM_ECC_INT_MSK_MSKRXIDLERAMINITDONE_FIELD_MASK,
    0,
    XPCSRX_RX_RAM_ECC_INT_MSK_MSKRXIDLERAMINITDONE_FIELD_WIDTH,
    XPCSRX_RX_RAM_ECC_INT_MSK_MSKRXIDLERAMINITDONE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPCSRX_RX_RAM_ECC_INT_MSK_MSKRXFECNQUERAMINITDONE
 ******************************************************************************/
const ru_field_rec XPCSRX_RX_RAM_ECC_INT_MSK_MSKRXFECNQUERAMINITDONE_FIELD =
{
    "MSKRXFECNQUERAMINITDONE",
#if RU_INCLUDE_DESC
    "",
    "FEC enqueue FIFO RAM init done interrupt mask.",
#endif
    XPCSRX_RX_RAM_ECC_INT_MSK_MSKRXFECNQUERAMINITDONE_FIELD_MASK,
    0,
    XPCSRX_RX_RAM_ECC_INT_MSK_MSKRXFECNQUERAMINITDONE_FIELD_WIDTH,
    XPCSRX_RX_RAM_ECC_INT_MSK_MSKRXFECNQUERAMINITDONE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPCSRX_RX_RAM_ECC_INT_MSK_RESERVED1
 ******************************************************************************/
const ru_field_rec XPCSRX_RX_RAM_ECC_INT_MSK_RESERVED1_FIELD =
{
    "RESERVED1",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    XPCSRX_RX_RAM_ECC_INT_MSK_RESERVED1_FIELD_MASK,
    0,
    XPCSRX_RX_RAM_ECC_INT_MSK_RESERVED1_FIELD_WIDTH,
    XPCSRX_RX_RAM_ECC_INT_MSK_RESERVED1_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPCSRX_RX_RAM_ECC_INT_MSK_MSKRXFECDECRAMINITDONE
 ******************************************************************************/
const ru_field_rec XPCSRX_RX_RAM_ECC_INT_MSK_MSKRXFECDECRAMINITDONE_FIELD =
{
    "MSKRXFECDECRAMINITDONE",
#if RU_INCLUDE_DESC
    "",
    "FEC decode FIFO RAM init done interrupt mask.",
#endif
    XPCSRX_RX_RAM_ECC_INT_MSK_MSKRXFECDECRAMINITDONE_FIELD_MASK,
    0,
    XPCSRX_RX_RAM_ECC_INT_MSK_MSKRXFECDECRAMINITDONE_FIELD_WIDTH,
    XPCSRX_RX_RAM_ECC_INT_MSK_MSKRXFECDECRAMINITDONE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPCSRX_RX_DFT_TESTMODE_RESERVED0
 ******************************************************************************/
const ru_field_rec XPCSRX_RX_DFT_TESTMODE_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    XPCSRX_RX_DFT_TESTMODE_RESERVED0_FIELD_MASK,
    0,
    XPCSRX_RX_DFT_TESTMODE_RESERVED0_FIELD_WIDTH,
    XPCSRX_RX_DFT_TESTMODE_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPCSRX_RX_DFT_TESTMODE_TM_PD
 ******************************************************************************/
const ru_field_rec XPCSRX_RX_DFT_TESTMODE_TM_PD_FIELD =
{
    "TM_PD",
#if RU_INCLUDE_DESC
    "",
    "DFT test mode for PD RAMs",
#endif
    XPCSRX_RX_DFT_TESTMODE_TM_PD_FIELD_MASK,
    0,
    XPCSRX_RX_DFT_TESTMODE_TM_PD_FIELD_WIDTH,
    XPCSRX_RX_DFT_TESTMODE_TM_PD_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPCSRX_RX_RAM_POWER_PDA_CTL0_RESERVED0
 ******************************************************************************/
const ru_field_rec XPCSRX_RX_RAM_POWER_PDA_CTL0_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    XPCSRX_RX_RAM_POWER_PDA_CTL0_RESERVED0_FIELD_MASK,
    0,
    XPCSRX_RX_RAM_POWER_PDA_CTL0_RESERVED0_FIELD_WIDTH,
    XPCSRX_RX_RAM_POWER_PDA_CTL0_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPCSRX_RX_RAM_POWER_PDA_CTL0_CFGXPCSRXIDLERAMPDA
 ******************************************************************************/
const ru_field_rec XPCSRX_RX_RAM_POWER_PDA_CTL0_CFGXPCSRXIDLERAMPDA_FIELD =
{
    "CFGXPCSRXIDLERAMPDA",
#if RU_INCLUDE_DESC
    "",
    "Array power down control for FEC decode RAM",
#endif
    XPCSRX_RX_RAM_POWER_PDA_CTL0_CFGXPCSRXIDLERAMPDA_FIELD_MASK,
    0,
    XPCSRX_RX_RAM_POWER_PDA_CTL0_CFGXPCSRXIDLERAMPDA_FIELD_WIDTH,
    XPCSRX_RX_RAM_POWER_PDA_CTL0_CFGXPCSRXIDLERAMPDA_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPCSRX_RX_RAM_POWER_PDA_CTL0_RESERVED1
 ******************************************************************************/
const ru_field_rec XPCSRX_RX_RAM_POWER_PDA_CTL0_RESERVED1_FIELD =
{
    "RESERVED1",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    XPCSRX_RX_RAM_POWER_PDA_CTL0_RESERVED1_FIELD_MASK,
    0,
    XPCSRX_RX_RAM_POWER_PDA_CTL0_RESERVED1_FIELD_WIDTH,
    XPCSRX_RX_RAM_POWER_PDA_CTL0_RESERVED1_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPCSRX_RX_RAM_POWER_PDA_CTL0_CFGXPCSRXFECDECRAMPDA
 ******************************************************************************/
const ru_field_rec XPCSRX_RX_RAM_POWER_PDA_CTL0_CFGXPCSRXFECDECRAMPDA_FIELD =
{
    "CFGXPCSRXFECDECRAMPDA",
#if RU_INCLUDE_DESC
    "",
    "Array power down control for FEC decode RAM",
#endif
    XPCSRX_RX_RAM_POWER_PDA_CTL0_CFGXPCSRXFECDECRAMPDA_FIELD_MASK,
    0,
    XPCSRX_RX_RAM_POWER_PDA_CTL0_CFGXPCSRXFECDECRAMPDA_FIELD_WIDTH,
    XPCSRX_RX_RAM_POWER_PDA_CTL0_CFGXPCSRXFECDECRAMPDA_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPCSRX_RX_INT_STAT1_RESERVED0
 ******************************************************************************/
const ru_field_rec XPCSRX_RX_INT_STAT1_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    XPCSRX_RX_INT_STAT1_RESERVED0_FIELD_MASK,
    0,
    XPCSRX_RX_INT_STAT1_RESERVED0_FIELD_WIDTH,
    XPCSRX_RX_INT_STAT1_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPCSRX_RX_INT_STAT1_INTRX64B66BTRAILSTART
 ******************************************************************************/
const ru_field_rec XPCSRX_RX_INT_STAT1_INTRX64B66BTRAILSTART_FIELD =
{
    "INTRX64B66BTRAILSTART",
#if RU_INCLUDE_DESC
    "",
    "trailing start at the end of burst",
#endif
    XPCSRX_RX_INT_STAT1_INTRX64B66BTRAILSTART_FIELD_MASK,
    0,
    XPCSRX_RX_INT_STAT1_INTRX64B66BTRAILSTART_FIELD_WIDTH,
    XPCSRX_RX_INT_STAT1_INTRX64B66BTRAILSTART_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPCSRX_RX_INT_STAT1_INTRX64B66BTWOSTOP
 ******************************************************************************/
const ru_field_rec XPCSRX_RX_INT_STAT1_INTRX64B66BTWOSTOP_FIELD =
{
    "INTRX64B66BTWOSTOP",
#if RU_INCLUDE_DESC
    "",
    "two stops in a row detected",
#endif
    XPCSRX_RX_INT_STAT1_INTRX64B66BTWOSTOP_FIELD_MASK,
    0,
    XPCSRX_RX_INT_STAT1_INTRX64B66BTWOSTOP_FIELD_WIDTH,
    XPCSRX_RX_INT_STAT1_INTRX64B66BTWOSTOP_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPCSRX_RX_INT_STAT1_INTRX64B66BTWOSTART
 ******************************************************************************/
const ru_field_rec XPCSRX_RX_INT_STAT1_INTRX64B66BTWOSTART_FIELD =
{
    "INTRX64B66BTWOSTART",
#if RU_INCLUDE_DESC
    "",
    "two starts in a row detected",
#endif
    XPCSRX_RX_INT_STAT1_INTRX64B66BTWOSTART_FIELD_MASK,
    0,
    XPCSRX_RX_INT_STAT1_INTRX64B66BTWOSTART_FIELD_WIDTH,
    XPCSRX_RX_INT_STAT1_INTRX64B66BTWOSTART_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPCSRX_RX_INT_STAT1_INTRX64B66BLEADSTOP
 ******************************************************************************/
const ru_field_rec XPCSRX_RX_INT_STAT1_INTRX64B66BLEADSTOP_FIELD =
{
    "INTRX64B66BLEADSTOP",
#if RU_INCLUDE_DESC
    "",
    "leading stop at the start of burst",
#endif
    XPCSRX_RX_INT_STAT1_INTRX64B66BLEADSTOP_FIELD_MASK,
    0,
    XPCSRX_RX_INT_STAT1_INTRX64B66BLEADSTOP_FIELD_WIDTH,
    XPCSRX_RX_INT_STAT1_INTRX64B66BLEADSTOP_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPCSRX_RX_INT_MSK1_RESERVED0
 ******************************************************************************/
const ru_field_rec XPCSRX_RX_INT_MSK1_RESERVED0_FIELD =
{
    "RESERVED0",
#if RU_INCLUDE_DESC
    "",
    "",
#endif
    XPCSRX_RX_INT_MSK1_RESERVED0_FIELD_MASK,
    0,
    XPCSRX_RX_INT_MSK1_RESERVED0_FIELD_WIDTH,
    XPCSRX_RX_INT_MSK1_RESERVED0_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPCSRX_RX_INT_MSK1_MSKRX64B66BTRAILSTART
 ******************************************************************************/
const ru_field_rec XPCSRX_RX_INT_MSK1_MSKRX64B66BTRAILSTART_FIELD =
{
    "MSKRX64B66BTRAILSTART",
#if RU_INCLUDE_DESC
    "",
    "0 - Interrupt is masked. 1 - Interrupt is unmasked.",
#endif
    XPCSRX_RX_INT_MSK1_MSKRX64B66BTRAILSTART_FIELD_MASK,
    0,
    XPCSRX_RX_INT_MSK1_MSKRX64B66BTRAILSTART_FIELD_WIDTH,
    XPCSRX_RX_INT_MSK1_MSKRX64B66BTRAILSTART_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPCSRX_RX_INT_MSK1_MSKRX64B66BTWOSTOP
 ******************************************************************************/
const ru_field_rec XPCSRX_RX_INT_MSK1_MSKRX64B66BTWOSTOP_FIELD =
{
    "MSKRX64B66BTWOSTOP",
#if RU_INCLUDE_DESC
    "",
    "0 - Interrupt is masked. 1 - Interrupt is unmasked.",
#endif
    XPCSRX_RX_INT_MSK1_MSKRX64B66BTWOSTOP_FIELD_MASK,
    0,
    XPCSRX_RX_INT_MSK1_MSKRX64B66BTWOSTOP_FIELD_WIDTH,
    XPCSRX_RX_INT_MSK1_MSKRX64B66BTWOSTOP_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPCSRX_RX_INT_MSK1_MSKRX64B66BTWOSTART
 ******************************************************************************/
const ru_field_rec XPCSRX_RX_INT_MSK1_MSKRX64B66BTWOSTART_FIELD =
{
    "MSKRX64B66BTWOSTART",
#if RU_INCLUDE_DESC
    "",
    "0 - Interrupt is masked. 1 - Interrupt is unmasked.",
#endif
    XPCSRX_RX_INT_MSK1_MSKRX64B66BTWOSTART_FIELD_MASK,
    0,
    XPCSRX_RX_INT_MSK1_MSKRX64B66BTWOSTART_FIELD_WIDTH,
    XPCSRX_RX_INT_MSK1_MSKRX64B66BTWOSTART_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPCSRX_RX_INT_MSK1_MSKRX64B66BLEADSTOP
 ******************************************************************************/
const ru_field_rec XPCSRX_RX_INT_MSK1_MSKRX64B66BLEADSTOP_FIELD =
{
    "MSKRX64B66BLEADSTOP",
#if RU_INCLUDE_DESC
    "",
    "0 - Interrupt is masked. 1 - Interrupt is unmasked.",
#endif
    XPCSRX_RX_INT_MSK1_MSKRX64B66BLEADSTOP_FIELD_MASK,
    0,
    XPCSRX_RX_INT_MSK1_MSKRX64B66BLEADSTOP_FIELD_WIDTH,
    XPCSRX_RX_INT_MSK1_MSKRX64B66BLEADSTOP_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

/******************************************************************************
 * Field: XPCSRX_RX_SPARE_CTL_CFGXPCSRXSPARE
 ******************************************************************************/
const ru_field_rec XPCSRX_RX_SPARE_CTL_CFGXPCSRXSPARE_FIELD =
{
    "CFGXPCSRXSPARE",
#if RU_INCLUDE_DESC
    "",
    "Spare RW bits",
#endif
    XPCSRX_RX_SPARE_CTL_CFGXPCSRXSPARE_FIELD_MASK,
    0,
    XPCSRX_RX_SPARE_CTL_CFGXPCSRXSPARE_FIELD_WIDTH,
    XPCSRX_RX_SPARE_CTL_CFGXPCSRXSPARE_FIELD_SHIFT,
#if RU_INCLUDE_ACCESS
    ru_access_rw
#endif
};

#endif /* RU_INCLUDE_FIELD_DB */

/******************************************************************************
 * Register: XPCSRX_RX_RST
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XPCSRX_RX_RST_FIELDS[] =
{
    &XPCSRX_RX_RST_RESERVED0_FIELD,
    &XPCSRX_RX_RST_CFGXPCSRXCLK161RSTN_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec XPCSRX_RX_RST_REG = 
{
    "RX_RST",
#if RU_INCLUDE_DESC
    "XPCSRX_RST Register",
    "Provides reset for submodules within XPCS RX and XSBI.",
#endif
    XPCSRX_RX_RST_REG_OFFSET,
    0,
    0,
    275,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    XPCSRX_RX_RST_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: XPCSRX_RX_INT_STAT
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XPCSRX_RX_INT_STAT_FIELDS[] =
{
    &XPCSRX_RX_INT_STAT_RESERVED0_FIELD,
    &XPCSRX_RX_INT_STAT_INTRXIDLEDAJIT_FIELD,
    &XPCSRX_RX_INT_STAT_INTRXFRMRMISBRST_FIELD,
    &XPCSRX_RX_INT_STAT_INTRXIDLESOPEOPGAPBIG_FIELD,
    &XPCSRX_RX_INT_STAT_INTRXIDLEFRCINS_FIELD,
    &XPCSRX_RX_INT_STAT_INTRX64B66BMINIPGERR_FIELD,
    &XPCSRX_RX_INT_STAT_INTRXFECNQUECNTNEQ_FIELD,
    &XPCSRX_RX_INT_STAT_INTRXIDLEFIFOUNDRUN_FIELD,
    &XPCSRX_RX_INT_STAT_INTRXIDLEFIFOOVRRUN_FIELD,
    &XPCSRX_RX_INT_STAT_INTRXFECHIGHCOR_FIELD,
    &XPCSRX_RX_INT_STAT_RESERVED1_FIELD,
    &XPCSRX_RX_INT_STAT_INTRXFECDECSTOPONERR_FIELD,
    &XPCSRX_RX_INT_STAT_INTRXFECDECPASS_FIELD,
    &XPCSRX_RX_INT_STAT_INTRXSTATFRMRHIGHBER_FIELD,
    &XPCSRX_RX_INT_STAT_INTRXFRMREXITBYSP_FIELD,
    &XPCSRX_RX_INT_STAT_INTRXFRMRBADSHMAX_FIELD,
    &XPCSRX_RX_INT_STAT_INTRXDSCRAMBURSTSEQOUT_FIELD,
    &XPCSRX_RX_INT_STAT_INTRXTESTPSUDOLOCK_FIELD,
    &XPCSRX_RX_INT_STAT_INTRXTESTPSUDOTYPE_FIELD,
    &XPCSRX_RX_INT_STAT_INTRXTESTPSUDOERR_FIELD,
    &XPCSRX_RX_INT_STAT_INTRXTESTPRBSLOCK_FIELD,
    &XPCSRX_RX_INT_STAT_INTRXTESTPRBSERR_FIELD,
    &XPCSRX_RX_INT_STAT_INTRXFECPSISTDECFAIL_FIELD,
    &XPCSRX_RX_INT_STAT_INTRXFRAMERBADSH_FIELD,
    &XPCSRX_RX_INT_STAT_INTRXFRAMERCWLOSS_FIELD,
    &XPCSRX_RX_INT_STAT_INTRXFRAMERCWLOCK_FIELD,
    &XPCSRX_RX_INT_STAT_INTRXFECDECFAIL_FIELD,
    &XPCSRX_RX_INT_STAT_INTRX64B66BDECERR_FIELD,
    &XPCSRX_RX_INT_STAT_INTRXFRMRNOLOCKLOS_FIELD,
    &XPCSRX_RX_INT_STAT_INTRXFRMRROGUE_FIELD,
    &XPCSRX_RX_INT_STAT_INT_REGS_ERR_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec XPCSRX_RX_INT_STAT_REG = 
{
    "RX_INT_STAT",
#if RU_INCLUDE_DESC
    "XPCSRX_INT_STAT Register",
    "Interrupt status for XPcsRx module.",
#endif
    XPCSRX_RX_INT_STAT_REG_OFFSET,
    0,
    0,
    276,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    31,
    XPCSRX_RX_INT_STAT_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: XPCSRX_RX_INT_MSK
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XPCSRX_RX_INT_MSK_FIELDS[] =
{
    &XPCSRX_RX_INT_MSK_RESERVED0_FIELD,
    &XPCSRX_RX_INT_MSK_MSKRXIDLEDAJIT_FIELD,
    &XPCSRX_RX_INT_MSK_MSKRXFRMRMISBRST_FIELD,
    &XPCSRX_RX_INT_MSK_MSKRXIDLESOPEOPGAPBIG_FIELD,
    &XPCSRX_RX_INT_MSK_MSKRXIDLEFRCINS_FIELD,
    &XPCSRX_RX_INT_MSK_MSKRX64B66BMINIPGERR_FIELD,
    &XPCSRX_RX_INT_MSK_MSKRXFECNQUECNTNEQ_FIELD,
    &XPCSRX_RX_INT_MSK_MSKRXIDLEFIFOUNDRUN_FIELD,
    &XPCSRX_RX_INT_MSK_MSKRXIDLEFIFOOVRRUN_FIELD,
    &XPCSRX_RX_INT_MSK_MSKRXFECHIGHCOR_FIELD,
    &XPCSRX_RX_INT_MSK_RESERVED1_FIELD,
    &XPCSRX_RX_INT_MSK_MSKRXFECDECSTOPONERR_FIELD,
    &XPCSRX_RX_INT_MSK_MSKRXFECDECPASS_FIELD,
    &XPCSRX_RX_INT_MSK_MSKRXSTATFRMRHIGHBER_FIELD,
    &XPCSRX_RX_INT_MSK_MSKRXFRMREXITBYSP_FIELD,
    &XPCSRX_RX_INT_MSK_MSKRXFRMRBADSHMAX_FIELD,
    &XPCSRX_RX_INT_MSK_MSKRXDSCRAMBURSTSEQOUT_FIELD,
    &XPCSRX_RX_INT_MSK_MSKRXTESTPSUDOLOCK_FIELD,
    &XPCSRX_RX_INT_MSK_MSKRXTESTPSUDOTYPE_FIELD,
    &XPCSRX_RX_INT_MSK_MSKRXTESTPSUDOERR_FIELD,
    &XPCSRX_RX_INT_MSK_MSKRXTESTPRBSLOCK_FIELD,
    &XPCSRX_RX_INT_MSK_MSKRXTESTPRBSERR_FIELD,
    &XPCSRX_RX_INT_MSK_MSKRXFECPSISTDECFAIL_FIELD,
    &XPCSRX_RX_INT_MSK_MSKRXFRAMERBADSH_FIELD,
    &XPCSRX_RX_INT_MSK_MSKRXFRAMERCWLOSS_FIELD,
    &XPCSRX_RX_INT_MSK_MSKRXFRAMERCWLOCK_FIELD,
    &XPCSRX_RX_INT_MSK_MSKRXFECDECFAIL_FIELD,
    &XPCSRX_RX_INT_MSK_MSKRX64B66BDECERR_FIELD,
    &XPCSRX_RX_INT_MSK_MSKRXFRMRNOLOCKLOS_FIELD,
    &XPCSRX_RX_INT_MSK_MSKRXFRMRROGUE_FIELD,
    &XPCSRX_RX_INT_MSK_MSK_INT_REGS_ERR_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec XPCSRX_RX_INT_MSK_REG = 
{
    "RX_INT_MSK",
#if RU_INCLUDE_DESC
    "XPCSRX_INT_MSK Register",
    "Interrupt mask for XPcsRx module.",
#endif
    XPCSRX_RX_INT_MSK_REG_OFFSET,
    0,
    0,
    277,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    31,
    XPCSRX_RX_INT_MSK_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: XPCSRX_RX_FRAMER_CTL
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XPCSRX_RX_FRAMER_CTL_FIELDS[] =
{
    &XPCSRX_RX_FRAMER_CTL_RESERVED0_FIELD,
    &XPCSRX_RX_FRAMER_CTL_CFGXPCSRXFRMRFRCEARLYALIGN_FIELD,
    &XPCSRX_RX_FRAMER_CTL_CFGXPCSRXFRMRMODEA_FIELD,
    &XPCSRX_RX_FRAMER_CTL_CFGXPCSRXFRMROVERLAPBDEBDZERO_FIELD,
    &XPCSRX_RX_FRAMER_CTL_CFGXPCSRXFRMROVERLAPGNTEN_FIELD,
    &XPCSRX_RX_FRAMER_CTL_RESERVED1_FIELD,
    &XPCSRX_RX_FRAMER_CTL_CFGXPCSRXFRAMEBURSTOLDALIGN_FIELD,
    &XPCSRX_RX_FRAMER_CTL_CFGXPCSRXFRMRMISBRSTTYPE_FIELD,
    &XPCSRX_RX_FRAMER_CTL_CFGXPCSRXFRMREBDVLDEN_FIELD,
    &XPCSRX_RX_FRAMER_CTL_CFGXPCSRXFRMRBDCNTEN_FIELD,
    &XPCSRX_RX_FRAMER_CTL_CFGXPCSRXFRMRBURSTBADSHEN_FIELD,
    &XPCSRX_RX_FRAMER_CTL_CFGXPCSRXFRMRSPULKEN_FIELD,
    &XPCSRX_RX_FRAMER_CTL_CFGXPCSRXFRAMEBURST_FIELD,
    &XPCSRX_RX_FRAMER_CTL_CFGXPCSRXFRMREN_FIELD,
    &XPCSRX_RX_FRAMER_CTL_CFGXPCSRXFRMRBLKFECFAIL_FIELD,
    &XPCSRX_RX_FRAMER_CTL_CFGXPCSRXFRAMEFEC_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec XPCSRX_RX_FRAMER_CTL_REG = 
{
    "RX_FRAMER_CTL",
#if RU_INCLUDE_DESC
    "XPCSRX_FRAMER_CTL Register",
    "Provides control over framer in XPCS RX.",
#endif
    XPCSRX_RX_FRAMER_CTL_REG_OFFSET,
    0,
    0,
    278,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    16,
    XPCSRX_RX_FRAMER_CTL_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: XPCSRX_RX_FEC_CTL
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XPCSRX_RX_FEC_CTL_FIELDS[] =
{
    &XPCSRX_RX_FEC_CTL_RESERVED0_FIELD,
    &XPCSRX_RX_FEC_CTL_CFGXPCSRXFECSTOPONERR_FIELD,
    &XPCSRX_RX_FEC_CTL_CFGXPCSRXFECCNTNQUECW_FIELD,
    &XPCSRX_RX_FEC_CTL_CFGXPCSRXFECNQUERST_FIELD,
    &XPCSRX_RX_FEC_CTL_CFGXPCSRXFECONEZEROMODE_FIELD,
    &XPCSRX_RX_FEC_CTL_CFGXPCSRXFECBLKCORRECT_FIELD,
    &XPCSRX_RX_FEC_CTL_CFGXPCSRXFECNQUETESTPAT_FIELD,
    &XPCSRX_RX_FEC_CTL_CFGXPCSRXFECFAILBLKSH0_FIELD,
    &XPCSRX_RX_FEC_CTL_CFGXPCSRXFECSTRIP_FIELD,
    &XPCSRX_RX_FEC_CTL_CFGXPCSRXFECBYPAS_FIELD,
    &XPCSRX_RX_FEC_CTL_CFGXPCSRXFECIDLEINS_FIELD,
    &XPCSRX_RX_FEC_CTL_CFGXPCSRXFECEN_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec XPCSRX_RX_FEC_CTL_REG = 
{
    "RX_FEC_CTL",
#if RU_INCLUDE_DESC
    "XPCSRX_FEC_CTL Register",
    "Provides control over FEC in XPCS RX.",
#endif
    XPCSRX_RX_FEC_CTL_REG_OFFSET,
    0,
    0,
    279,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    12,
    XPCSRX_RX_FEC_CTL_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: XPCSRX_RX_DSCRAM_CTL
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XPCSRX_RX_DSCRAM_CTL_FIELDS[] =
{
    &XPCSRX_RX_DSCRAM_CTL_RESERVED0_FIELD,
    &XPCSRX_RX_DSCRAM_CTL_CFGXPCSRXDSCRAMBYPAS_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec XPCSRX_RX_DSCRAM_CTL_REG = 
{
    "RX_DSCRAM_CTL",
#if RU_INCLUDE_DESC
    "XPCSRX_DSCRAM_CTL Register",
    "Provides control over descrambler in XPCS RX.",
#endif
    XPCSRX_RX_DSCRAM_CTL_REG_OFFSET,
    0,
    0,
    280,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    XPCSRX_RX_DSCRAM_CTL_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: XPCSRX_RX_64B66B_CTL
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XPCSRX_RX_64B66B_CTL_FIELDS[] =
{
    &XPCSRX_RX_64B66B_CTL_CFGXPCSRX64B66BTMASK1_FIELD,
    &XPCSRX_RX_64B66B_CTL_CFGXPCSRX64B66BTMASK0_FIELD,
    &XPCSRX_RX_64B66B_CTL_RESERVED0_FIELD,
    &XPCSRX_RX_64B66B_CTL_CFGXPCSRX64B66BSMASK1_FIELD,
    &XPCSRX_RX_64B66B_CTL_RESERVED1_FIELD,
    &XPCSRX_RX_64B66B_CTL_CFGXPCSRX64B66BSMASK0_FIELD,
    &XPCSRX_RX_64B66B_CTL_RESERVED2_FIELD,
    &XPCSRX_RX_64B66B_CTL_CFGXPCSRX64B66BTDLAY_FIELD,
    &XPCSRX_RX_64B66B_CTL_RESERVED3_FIELD,
    &XPCSRX_RX_64B66B_CTL_CFGXPCSRX64B66BDECBYPAS_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec XPCSRX_RX_64B66B_CTL_REG = 
{
    "RX_64B66B_CTL",
#if RU_INCLUDE_DESC
    "XPCSRX_64B66B_CTL Register",
    "Provides control over 64b/66b decoder in XPCS RX.",
#endif
    XPCSRX_RX_64B66B_CTL_REG_OFFSET,
    0,
    0,
    281,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    10,
    XPCSRX_RX_64B66B_CTL_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: XPCSRX_RX_TEST_CTL
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XPCSRX_RX_TEST_CTL_FIELDS[] =
{
    &XPCSRX_RX_TEST_CTL_RESERVED0_FIELD,
    &XPCSRX_RX_TEST_CTL_CFGXPCSRXTESTPRBSDETEN_FIELD,
    &XPCSRX_RX_TEST_CTL_CFGXPCSRXTESTPSUDODETEN_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec XPCSRX_RX_TEST_CTL_REG = 
{
    "RX_TEST_CTL",
#if RU_INCLUDE_DESC
    "XPCSRX_TEST_CTL Register",
    "Provides control over test circuits in XPCS RX.",
#endif
    XPCSRX_RX_TEST_CTL_REG_OFFSET,
    0,
    0,
    282,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    3,
    XPCSRX_RX_TEST_CTL_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: XPCSRX_RX_IDLE_RD_TIMER_DLY
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XPCSRX_RX_IDLE_RD_TIMER_DLY_FIELDS[] =
{
    &XPCSRX_RX_IDLE_RD_TIMER_DLY_RESERVED0_FIELD,
    &XPCSRX_RX_IDLE_RD_TIMER_DLY_CFGXPCSRXIDLERDDELAYTIMERMAX_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec XPCSRX_RX_IDLE_RD_TIMER_DLY_REG = 
{
    "RX_IDLE_RD_TIMER_DLY",
#if RU_INCLUDE_DESC
    "XPCSRX_IDLE_RD_TIMER_DLY Register",
    "Sets the delay time to start read of burst from idle insert FIFO.",
#endif
    XPCSRX_RX_IDLE_RD_TIMER_DLY_REG_OFFSET,
    0,
    0,
    283,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    XPCSRX_RX_IDLE_RD_TIMER_DLY_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: XPCSRX_RX_IDLE_GAP_SIZ_MAX
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XPCSRX_RX_IDLE_GAP_SIZ_MAX_FIELDS[] =
{
    &XPCSRX_RX_IDLE_GAP_SIZ_MAX_RESERVED0_FIELD,
    &XPCSRX_RX_IDLE_GAP_SIZ_MAX_CFGXPCSRXIDLEOVRSIZMAX_FIELD,
    &XPCSRX_RX_IDLE_GAP_SIZ_MAX_CFGXPCSRXIDLESOPEOPGAP_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec XPCSRX_RX_IDLE_GAP_SIZ_MAX_REG = 
{
    "RX_IDLE_GAP_SIZ_MAX",
#if RU_INCLUDE_DESC
    "XPCSRX_IDLE_GAP_SIZ_MAX Register",
    "Sets the size for over size frames based on delta between SOP and EOP.",
#endif
    XPCSRX_RX_IDLE_GAP_SIZ_MAX_REG_OFFSET,
    0,
    0,
    284,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    3,
    XPCSRX_RX_IDLE_GAP_SIZ_MAX_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: XPCSRX_RX_FRAMER_LK_MAX
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XPCSRX_RX_FRAMER_LK_MAX_FIELDS[] =
{
    &XPCSRX_RX_FRAMER_LK_MAX_RESERVED0_FIELD,
    &XPCSRX_RX_FRAMER_LK_MAX_CFGXPCSRXFRMRCWLKTIMERMAX_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec XPCSRX_RX_FRAMER_LK_MAX_REG = 
{
    "RX_FRAMER_LK_MAX",
#if RU_INCLUDE_DESC
    "XPCSRX_FRAMER_LK_MAX Register",
    "Sets the delay for indicating lock to FEC decode circuit."
    "For FEC modes use the defalut of 8'd280."
    "For FEC bypass modes use the defalut of 8'd26.",
#endif
    XPCSRX_RX_FRAMER_LK_MAX_REG_OFFSET,
    0,
    0,
    285,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    XPCSRX_RX_FRAMER_LK_MAX_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: XPCSRX_RX_FRAMER_UNLK_MAX
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XPCSRX_RX_FRAMER_UNLK_MAX_FIELDS[] =
{
    &XPCSRX_RX_FRAMER_UNLK_MAX_RESERVED0_FIELD,
    &XPCSRX_RX_FRAMER_UNLK_MAX_CFGXPCSRXFRMRCWUNLKTIMERMAX_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec XPCSRX_RX_FRAMER_UNLK_MAX_REG = 
{
    "RX_FRAMER_UNLK_MAX",
#if RU_INCLUDE_DESC
    "XPCSRX_FRAMER_UNLK_MAX Register",
    "Sets the delay for indicating unlock to FEC decode circuit."
    "For FEC modes use the defalut of 8'd280."
    "For FEC bypass modes use the defalut of 8'd26.",
#endif
    XPCSRX_RX_FRAMER_UNLK_MAX_REG_OFFSET,
    0,
    0,
    286,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    XPCSRX_RX_FRAMER_UNLK_MAX_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: XPCSRX_RX_FRAMER_BD_SH
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XPCSRX_RX_FRAMER_BD_SH_FIELDS[] =
{
    &XPCSRX_RX_FRAMER_BD_SH_RESERVED0_FIELD,
    &XPCSRX_RX_FRAMER_BD_SH_CFGXPCSRXOLTBDSH_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec XPCSRX_RX_FRAMER_BD_SH_REG = 
{
    "RX_FRAMER_BD_SH",
#if RU_INCLUDE_DESC
    "XPCSRX_FRAMER_BD_SH Register",
    "Sets the SH value for the BD for the framer.",
#endif
    XPCSRX_RX_FRAMER_BD_SH_REG_OFFSET,
    0,
    0,
    287,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    XPCSRX_RX_FRAMER_BD_SH_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: XPCSRX_RX_FRAMER_BD_LO
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XPCSRX_RX_FRAMER_BD_LO_FIELDS[] =
{
    &XPCSRX_RX_FRAMER_BD_LO_CFGXPCSRXOLTBDLO_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec XPCSRX_RX_FRAMER_BD_LO_REG = 
{
    "RX_FRAMER_BD_LO",
#if RU_INCLUDE_DESC
    "XPCSRX_FRAMER_BD_LO Register",
    "Sets the low 32 bit value for the BD for the framer.",
#endif
    XPCSRX_RX_FRAMER_BD_LO_REG_OFFSET,
    0,
    0,
    288,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    XPCSRX_RX_FRAMER_BD_LO_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: XPCSRX_RX_FRAMER_BD_HI
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XPCSRX_RX_FRAMER_BD_HI_FIELDS[] =
{
    &XPCSRX_RX_FRAMER_BD_HI_CFGXPCSRXOLTBDHI_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec XPCSRX_RX_FRAMER_BD_HI_REG = 
{
    "RX_FRAMER_BD_HI",
#if RU_INCLUDE_DESC
    "XPCSRX_FRAMER_BD_HI Register",
    "Sets the high 32 bit value for the BD for the framer.",
#endif
    XPCSRX_RX_FRAMER_BD_HI_REG_OFFSET,
    0,
    0,
    289,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    XPCSRX_RX_FRAMER_BD_HI_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: XPCSRX_RX_FRAMER_EBD_SH
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XPCSRX_RX_FRAMER_EBD_SH_FIELDS[] =
{
    &XPCSRX_RX_FRAMER_EBD_SH_RESERVED0_FIELD,
    &XPCSRX_RX_FRAMER_EBD_SH_CFGXPCSRXOLTEBDSH_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec XPCSRX_RX_FRAMER_EBD_SH_REG = 
{
    "RX_FRAMER_EBD_SH",
#if RU_INCLUDE_DESC
    "XPCSRX_FRAMER_EBD_SH Register",
    "Sets the SH value for the EBD for the framer.",
#endif
    XPCSRX_RX_FRAMER_EBD_SH_REG_OFFSET,
    0,
    0,
    290,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    XPCSRX_RX_FRAMER_EBD_SH_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: XPCSRX_RX_FRAMER_EBD_LO
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XPCSRX_RX_FRAMER_EBD_LO_FIELDS[] =
{
    &XPCSRX_RX_FRAMER_EBD_LO_CFGXPCSRXOLTEBDLO_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec XPCSRX_RX_FRAMER_EBD_LO_REG = 
{
    "RX_FRAMER_EBD_LO",
#if RU_INCLUDE_DESC
    "XPCSRX_FRAMER_EBD_LO Register",
    "Sets the low 32 bit value for the EBD for the framer.",
#endif
    XPCSRX_RX_FRAMER_EBD_LO_REG_OFFSET,
    0,
    0,
    291,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    XPCSRX_RX_FRAMER_EBD_LO_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: XPCSRX_RX_FRAMER_EBD_HI
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XPCSRX_RX_FRAMER_EBD_HI_FIELDS[] =
{
    &XPCSRX_RX_FRAMER_EBD_HI_CFGXPCSRXOLTEBDHI_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec XPCSRX_RX_FRAMER_EBD_HI_REG = 
{
    "RX_FRAMER_EBD_HI",
#if RU_INCLUDE_DESC
    "XPCSRX_FRAMER_EBD_HI Register",
    "Sets the high 32 bit value for the EBD for the framer.",
#endif
    XPCSRX_RX_FRAMER_EBD_HI_REG_OFFSET,
    0,
    0,
    292,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    XPCSRX_RX_FRAMER_EBD_HI_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: XPCSRX_RX_STATUS
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XPCSRX_RX_STATUS_FIELDS[] =
{
    &XPCSRX_RX_STATUS_RESERVED0_FIELD,
    &XPCSRX_RX_STATUS_STATRXIDLEDAJIT_FIELD,
    &XPCSRX_RX_STATUS_STATRXFRMRMISBRST_FIELD,
    &XPCSRX_RX_STATUS_STATRXIDLESOPEOPGAPBIG_FIELD,
    &XPCSRX_RX_STATUS_STATRXIDLEFRCINS_FIELD,
    &XPCSRX_RX_STATUS_STATRX64B66BMINIPGERR_FIELD,
    &XPCSRX_RX_STATUS_STATRXFECNQUECNTNEQ_FIELD,
    &XPCSRX_RX_STATUS_STATRXIDLEFIFOUNDRUN_FIELD,
    &XPCSRX_RX_STATUS_STATRXIDLEFIFOOVRRUN_FIELD,
    &XPCSRX_RX_STATUS_STATRXFECHIGHCOR_FIELD,
    &XPCSRX_RX_STATUS_RESERVED1_FIELD,
    &XPCSRX_RX_STATUS_STATRXFECDECPASS_FIELD,
    &XPCSRX_RX_STATUS_STATRXSTATFRMRHIGHBER_FIELD,
    &XPCSRX_RX_STATUS_STATRXFRMREXITBYSP_FIELD,
    &XPCSRX_RX_STATUS_STATRXFRMRBADSHMAX_FIELD,
    &XPCSRX_RX_STATUS_STATRXDSCRAMBURSTSEQOUT_FIELD,
    &XPCSRX_RX_STATUS_STATRXTESTPSUDOLOCK_FIELD,
    &XPCSRX_RX_STATUS_STATRXTESTPSUDOTYPE_FIELD,
    &XPCSRX_RX_STATUS_STATRXTESTPSUDOERR_FIELD,
    &XPCSRX_RX_STATUS_STATRXTESTPRBSLOCK_FIELD,
    &XPCSRX_RX_STATUS_STATRXTESTPRBSERR_FIELD,
    &XPCSRX_RX_STATUS_STATRXFECPSISTDECFAIL_FIELD,
    &XPCSRX_RX_STATUS_STATRXFRAMERBADSH_FIELD,
    &XPCSRX_RX_STATUS_STATRXFRAMERCWLOSS_FIELD,
    &XPCSRX_RX_STATUS_STATRXFRAMERCWLOCK_FIELD,
    &XPCSRX_RX_STATUS_STATRXFECDECFAIL_FIELD,
    &XPCSRX_RX_STATUS_STATRX64B66BDECERR_FIELD,
    &XPCSRX_RX_STATUS_STATRXFRMRNOLOCKLOS_FIELD,
    &XPCSRX_RX_STATUS_STATRXFRMRROGUE_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec XPCSRX_RX_STATUS_REG = 
{
    "RX_STATUS",
#if RU_INCLUDE_DESC
    "XPCSRX_STATUS Register",
    "Raw value for interrupt status.",
#endif
    XPCSRX_RX_STATUS_REG_OFFSET,
    0,
    0,
    293,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    29,
    XPCSRX_RX_STATUS_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: XPCSRX_RX_FRAMER_LK_ULK_MAX
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XPCSRX_RX_FRAMER_LK_ULK_MAX_FIELDS[] =
{
    &XPCSRX_RX_FRAMER_LK_ULK_MAX_RESERVED0_FIELD,
    &XPCSRX_RX_FRAMER_LK_ULK_MAX_CFGXPCSRXFRMRSPLKMAX_FIELD,
    &XPCSRX_RX_FRAMER_LK_ULK_MAX_RESERVED1_FIELD,
    &XPCSRX_RX_FRAMER_LK_ULK_MAX_CFGXPCSRXFRMRSPULKMAX_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec XPCSRX_RX_FRAMER_LK_ULK_MAX_REG = 
{
    "RX_FRAMER_LK_ULK_MAX",
#if RU_INCLUDE_DESC
    "XPCSRX_FRAMER_LK_ULK_MAX Register",
    "Sets number of SP detected in order to validate lock or unlock.",
#endif
    XPCSRX_RX_FRAMER_LK_ULK_MAX_REG_OFFSET,
    0,
    0,
    294,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    4,
    XPCSRX_RX_FRAMER_LK_ULK_MAX_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: XPCSRX_RX_FRAMER_SP_SH
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XPCSRX_RX_FRAMER_SP_SH_FIELDS[] =
{
    &XPCSRX_RX_FRAMER_SP_SH_RESERVED0_FIELD,
    &XPCSRX_RX_FRAMER_SP_SH_CFGXPCSRXOLTSPSH_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec XPCSRX_RX_FRAMER_SP_SH_REG = 
{
    "RX_FRAMER_SP_SH",
#if RU_INCLUDE_DESC
    "XPCSRX_FRAMER_SP_SH Register",
    "Sets the SH value for the SP for the framer.",
#endif
    XPCSRX_RX_FRAMER_SP_SH_REG_OFFSET,
    0,
    0,
    295,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    XPCSRX_RX_FRAMER_SP_SH_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: XPCSRX_RX_FRAMER_SP_LO
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XPCSRX_RX_FRAMER_SP_LO_FIELDS[] =
{
    &XPCSRX_RX_FRAMER_SP_LO_CFGXPCSRXOLTSPLO_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec XPCSRX_RX_FRAMER_SP_LO_REG = 
{
    "RX_FRAMER_SP_LO",
#if RU_INCLUDE_DESC
    "XPCSRX_FRAMER_SP_LO Register",
    "Sets the low 32 bit value for the SP for the framer.",
#endif
    XPCSRX_RX_FRAMER_SP_LO_REG_OFFSET,
    0,
    0,
    296,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    XPCSRX_RX_FRAMER_SP_LO_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: XPCSRX_RX_FRAMER_SP_HI
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XPCSRX_RX_FRAMER_SP_HI_FIELDS[] =
{
    &XPCSRX_RX_FRAMER_SP_HI_CFGXPCSRXOLTSPHI_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec XPCSRX_RX_FRAMER_SP_HI_REG = 
{
    "RX_FRAMER_SP_HI",
#if RU_INCLUDE_DESC
    "XPCSRX_FRAMER_SP_HI Register",
    "Sets the high 32 bit value for the SP for the framer.",
#endif
    XPCSRX_RX_FRAMER_SP_HI_REG_OFFSET,
    0,
    0,
    297,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    XPCSRX_RX_FRAMER_SP_HI_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: XPCSRX_RX_FRAMER_STATE
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XPCSRX_RX_FRAMER_STATE_FIELDS[] =
{
    &XPCSRX_RX_FRAMER_STATE_RESERVED0_FIELD,
    &XPCSRX_RX_FRAMER_STATE_XPCSRXFRMRSTATE_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec XPCSRX_RX_FRAMER_STATE_REG = 
{
    "RX_FRAMER_STATE",
#if RU_INCLUDE_DESC
    "XPCSRX_FRAMER_STATE Register",
    "The current value of framer state machine.",
#endif
    XPCSRX_RX_FRAMER_STATE_REG_OFFSET,
    0,
    0,
    298,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    XPCSRX_RX_FRAMER_STATE_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: XPCSRX_RX_FRAMER_BD_EBD_HAM
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XPCSRX_RX_FRAMER_BD_EBD_HAM_FIELDS[] =
{
    &XPCSRX_RX_FRAMER_BD_EBD_HAM_RESERVED0_FIELD,
    &XPCSRX_RX_FRAMER_BD_EBD_HAM_CFGXPCSRXFRMRSPHAM_FIELD,
    &XPCSRX_RX_FRAMER_BD_EBD_HAM_CFGXPCSRXFRMREBDHAM_FIELD,
    &XPCSRX_RX_FRAMER_BD_EBD_HAM_CFGXPCSRXFRMRBDHAM_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec XPCSRX_RX_FRAMER_BD_EBD_HAM_REG = 
{
    "RX_FRAMER_BD_EBD_HAM",
#if RU_INCLUDE_DESC
    "XPCSRX_FRAMER_BD_EBD_HAM Register",
    "Sets the hamming distance for SP",
#endif
    XPCSRX_RX_FRAMER_BD_EBD_HAM_REG_OFFSET,
    0,
    0,
    299,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    4,
    XPCSRX_RX_FRAMER_BD_EBD_HAM_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: XPCSRX_RX_FRAMER_MISBRST_CNT
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XPCSRX_RX_FRAMER_MISBRST_CNT_FIELDS[] =
{
    &XPCSRX_RX_FRAMER_MISBRST_CNT_RXFRMRMISBRSTCNT_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec XPCSRX_RX_FRAMER_MISBRST_CNT_REG = 
{
    "RX_FRAMER_MISBRST_CNT",
#if RU_INCLUDE_DESC
    "XPCSRX_FRAMER_MISBRST_CNT Register",
    "The count of the possible missing bursts that were detected. This is"
    "based on SP detect and SP loss with no BD found.",
#endif
    XPCSRX_RX_FRAMER_MISBRST_CNT_REG_OFFSET,
    0,
    0,
    300,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    XPCSRX_RX_FRAMER_MISBRST_CNT_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: XPCSRX_RX_FRAMER_BD_ERR
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XPCSRX_RX_FRAMER_BD_ERR_FIELDS[] =
{
    &XPCSRX_RX_FRAMER_BD_ERR_RESERVED0_FIELD,
    &XPCSRX_RX_FRAMER_BD_ERR_XPCSRXSTATFRMRBDERR_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec XPCSRX_RX_FRAMER_BD_ERR_REG = 
{
    "RX_FRAMER_BD_ERR",
#if RU_INCLUDE_DESC
    "XPCSRX_FRAMER_BD_ERR Register",
    "The number of bit errors found in the last BD when lock was declared.",
#endif
    XPCSRX_RX_FRAMER_BD_ERR_REG_OFFSET,
    0,
    0,
    301,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    XPCSRX_RX_FRAMER_BD_ERR_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: XPCSRX_RX_FRAMER_ROGUE_CTL
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XPCSRX_RX_FRAMER_ROGUE_CTL_FIELDS[] =
{
    &XPCSRX_RX_FRAMER_ROGUE_CTL_CFGXPCSRXFRMRROGUEEN_FIELD,
    &XPCSRX_RX_FRAMER_ROGUE_CTL_RESERVED0_FIELD,
    &XPCSRX_RX_FRAMER_ROGUE_CTL_CFGXPCSRXFRMRROGUESPTRESH_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec XPCSRX_RX_FRAMER_ROGUE_CTL_REG = 
{
    "RX_FRAMER_ROGUE_CTL",
#if RU_INCLUDE_DESC
    "XPCSRX_FRAMER_ROGUE_CTL Register",
    "Config for LOS based on no lock during a time interval",
#endif
    XPCSRX_RX_FRAMER_ROGUE_CTL_REG_OFFSET,
    0,
    0,
    302,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    3,
    XPCSRX_RX_FRAMER_ROGUE_CTL_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: XPCSRX_RX_FRAMER_NOLOCK_CTL
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XPCSRX_RX_FRAMER_NOLOCK_CTL_FIELDS[] =
{
    &XPCSRX_RX_FRAMER_NOLOCK_CTL_CFGXPCSRXFRMRNOLOCKLOSEN_FIELD,
    &XPCSRX_RX_FRAMER_NOLOCK_CTL_RESERVED0_FIELD,
    &XPCSRX_RX_FRAMER_NOLOCK_CTL_CFGXPCSRXFRMRNOLOCKLOSINTVAL_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec XPCSRX_RX_FRAMER_NOLOCK_CTL_REG = 
{
    "RX_FRAMER_NOLOCK_CTL",
#if RU_INCLUDE_DESC
    "XPCSRX_FRAMER_NOLOCK_CTL Register",
    "Config for LOS based on no lock during a time interval",
#endif
    XPCSRX_RX_FRAMER_NOLOCK_CTL_REG_OFFSET,
    0,
    0,
    303,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    3,
    XPCSRX_RX_FRAMER_NOLOCK_CTL_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: XPCSRX_RX_64B66B_IPG_DET_CNT
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XPCSRX_RX_64B66B_IPG_DET_CNT_FIELDS[] =
{
    &XPCSRX_RX_64B66B_IPG_DET_CNT_RX64B66BIPGDETCNT_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec XPCSRX_RX_64B66B_IPG_DET_CNT_REG = 
{
    "RX_64B66B_IPG_DET_CNT",
#if RU_INCLUDE_DESC
    "XPCSRX_64B66B_IPG_DET_CNT Register",
    "Min IPG violation detection count.",
#endif
    XPCSRX_RX_64B66B_IPG_DET_CNT_REG_OFFSET,
    0,
    0,
    304,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    XPCSRX_RX_64B66B_IPG_DET_CNT_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: XPCSRX_RX_FEC_NQUE_IN_CNT
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XPCSRX_RX_FEC_NQUE_IN_CNT_FIELDS[] =
{
    &XPCSRX_RX_FEC_NQUE_IN_CNT_RXFECNQUEINCNT_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec XPCSRX_RX_FEC_NQUE_IN_CNT_REG = 
{
    "RX_FEC_NQUE_IN_CNT",
#if RU_INCLUDE_DESC
    "XPCSRX_FEC_NQUE_IN_CNT Register",
    "Counts the number of FEC CW written to the store/foward enqueue FIFO.",
#endif
    XPCSRX_RX_FEC_NQUE_IN_CNT_REG_OFFSET,
    0,
    0,
    305,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    XPCSRX_RX_FEC_NQUE_IN_CNT_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: XPCSRX_RX_FEC_NQUE_OUT_CNT
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XPCSRX_RX_FEC_NQUE_OUT_CNT_FIELDS[] =
{
    &XPCSRX_RX_FEC_NQUE_OUT_CNT_RXFECNQUEOUTCNT_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec XPCSRX_RX_FEC_NQUE_OUT_CNT_REG = 
{
    "RX_FEC_NQUE_OUT_CNT",
#if RU_INCLUDE_DESC
    "XPCSRX_FEC_NQUE_OUT_CNT Register",
    "Counts the number of FEC codewrods read from the store/foward enqueue"
    "FIFO.",
#endif
    XPCSRX_RX_FEC_NQUE_OUT_CNT_REG_OFFSET,
    0,
    0,
    306,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    XPCSRX_RX_FEC_NQUE_OUT_CNT_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: XPCSRX_RX_IDLE_START_CNT
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XPCSRX_RX_IDLE_START_CNT_FIELDS[] =
{
    &XPCSRX_RX_IDLE_START_CNT_RXIDLESTARTCNT_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec XPCSRX_RX_IDLE_START_CNT_REG = 
{
    "RX_IDLE_START_CNT",
#if RU_INCLUDE_DESC
    "XPCSRX_IDLE_START_CNT Register",
    "Counts the number of SOP detected in the IDLE insert circuit.",
#endif
    XPCSRX_RX_IDLE_START_CNT_REG_OFFSET,
    0,
    0,
    307,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    XPCSRX_RX_IDLE_START_CNT_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: XPCSRX_RX_IDLE_STOP_CNT
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XPCSRX_RX_IDLE_STOP_CNT_FIELDS[] =
{
    &XPCSRX_RX_IDLE_STOP_CNT_RXIDLESTOPCNT_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec XPCSRX_RX_IDLE_STOP_CNT_REG = 
{
    "RX_IDLE_STOP_CNT",
#if RU_INCLUDE_DESC
    "XPCSRX_IDLE_STOP_CNT Register",
    "Counts the number of EOP detected in the IDLE insert circuit.",
#endif
    XPCSRX_RX_IDLE_STOP_CNT_REG_OFFSET,
    0,
    0,
    308,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    XPCSRX_RX_IDLE_STOP_CNT_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: XPCSRX_RX_FEC_COR_INTVAL
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XPCSRX_RX_FEC_COR_INTVAL_FIELDS[] =
{
    &XPCSRX_RX_FEC_COR_INTVAL_RESERVED0_FIELD,
    &XPCSRX_RX_FEC_COR_INTVAL_CFGXPCSRXFECCORINTVAL_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec XPCSRX_RX_FEC_COR_INTVAL_REG = 
{
    "RX_FEC_COR_INTVAL",
#if RU_INCLUDE_DESC
    "XPCSRX_FEC_COR_INTVAL Register",
    "Time interval setting (in 6.2 ns quanta) over which the number of FEC"
    "correctines is counted. Used for creating high FEC correction alarm.",
#endif
    XPCSRX_RX_FEC_COR_INTVAL_REG_OFFSET,
    0,
    0,
    309,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    XPCSRX_RX_FEC_COR_INTVAL_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: XPCSRX_RX_FEC_COR_TRESH
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XPCSRX_RX_FEC_COR_TRESH_FIELDS[] =
{
    &XPCSRX_RX_FEC_COR_TRESH_RESERVED0_FIELD,
    &XPCSRX_RX_FEC_COR_TRESH_CFGXPCSRXFECCORTRESH_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec XPCSRX_RX_FEC_COR_TRESH_REG = 
{
    "RX_FEC_COR_TRESH",
#if RU_INCLUDE_DESC
    "XPCSRX_FEC_COR_TRESH Register",
    "The threshold on the number of FEC correctoins made over a timer"
    "interval that will cause the FEC high correction alarm.",
#endif
    XPCSRX_RX_FEC_COR_TRESH_REG_OFFSET,
    0,
    0,
    310,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    XPCSRX_RX_FEC_COR_TRESH_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: XPCSRX_RX_FEC_CW_FAIL_CNT
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XPCSRX_RX_FEC_CW_FAIL_CNT_FIELDS[] =
{
    &XPCSRX_RX_FEC_CW_FAIL_CNT_RXFECDECCWFAILCNT_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec XPCSRX_RX_FEC_CW_FAIL_CNT_REG = 
{
    "RX_FEC_CW_FAIL_CNT",
#if RU_INCLUDE_DESC
    "XPCSRX_FEC_CW_FAIL_CNT Register",
    "Count the number of uncorrectable FEC CW that have been recieved.",
#endif
    XPCSRX_RX_FEC_CW_FAIL_CNT_REG_OFFSET,
    0,
    0,
    311,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    XPCSRX_RX_FEC_CW_FAIL_CNT_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: XPCSRX_RX_FEC_CW_TOT_CNT
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XPCSRX_RX_FEC_CW_TOT_CNT_FIELDS[] =
{
    &XPCSRX_RX_FEC_CW_TOT_CNT_RXFECDECCWTOTCNT_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec XPCSRX_RX_FEC_CW_TOT_CNT_REG = 
{
    "RX_FEC_CW_TOT_CNT",
#if RU_INCLUDE_DESC
    "XPCSRX_FEC_CW_TOT_CNT Register",
    "Count the number of total FEC CW that have been recievec.",
#endif
    XPCSRX_RX_FEC_CW_TOT_CNT_REG_OFFSET,
    0,
    0,
    312,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    XPCSRX_RX_FEC_CW_TOT_CNT_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: XPCSRX_RX_FEC_CORRECT_CNT_LOWER
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XPCSRX_RX_FEC_CORRECT_CNT_LOWER_FIELDS[] =
{
    &XPCSRX_RX_FEC_CORRECT_CNT_LOWER_RXFECDECERRCORCNTLOWER_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec XPCSRX_RX_FEC_CORRECT_CNT_LOWER_REG = 
{
    "RX_FEC_CORRECT_CNT_LOWER",
#if RU_INCLUDE_DESC
    "XPCSRX_FEC_CORRECT_CNT_LOWER Register",
    "The number of correctoins made in the FEC CW that have been recieved."
    "Lower 32 bits of a 39 bit stat."
    "Read this location before reading the upper 7 bis at"
    "XPCSRX_FEC_CORRECT_CNT_UPERER.",
#endif
    XPCSRX_RX_FEC_CORRECT_CNT_LOWER_REG_OFFSET,
    0,
    0,
    313,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    XPCSRX_RX_FEC_CORRECT_CNT_LOWER_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: XPCSRX_RX_FEC_CORRECT_CNT_UPPER
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XPCSRX_RX_FEC_CORRECT_CNT_UPPER_FIELDS[] =
{
    &XPCSRX_RX_FEC_CORRECT_CNT_UPPER_RESERVED0_FIELD,
    &XPCSRX_RX_FEC_CORRECT_CNT_UPPER_RXFECDECERRCORCNTUPPER_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec XPCSRX_RX_FEC_CORRECT_CNT_UPPER_REG = 
{
    "RX_FEC_CORRECT_CNT_UPPER",
#if RU_INCLUDE_DESC
    "XPCSRX_FEC_CORRECT_CNT_UPPER Register",
    "The number of correctoins made in the FEC CW that have been recieved."
    "Upper 7 bits of a 39 bit stat."
    "Bit 8 of this register represents overflow of the 39 bit stat."
    "Read this location after reading the lower 32 bits at"
    "XPCSRX_FEC_CORRECT_CNT_LOWER.",
#endif
    XPCSRX_RX_FEC_CORRECT_CNT_UPPER_REG_OFFSET,
    0,
    0,
    314,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    XPCSRX_RX_FEC_CORRECT_CNT_UPPER_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: XPCSRX_RX_FEC_CORRECT_CNT_SHADOW
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XPCSRX_RX_FEC_CORRECT_CNT_SHADOW_FIELDS[] =
{
    &XPCSRX_RX_FEC_CORRECT_CNT_SHADOW_RESERVED0_FIELD,
    &XPCSRX_RX_FEC_CORRECT_CNT_SHADOW_RXFECDECERRCORCNTSHADOW_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec XPCSRX_RX_FEC_CORRECT_CNT_SHADOW_REG = 
{
    "RX_FEC_CORRECT_CNT_SHADOW",
#if RU_INCLUDE_DESC
    "XPCSRX_FEC_CORRECT_CNT_SHADOW Register",
    "The number of correctoins made in the FEC CW that have been recieved."
    "Upper 7 bits of a 39 bit stat."
    "Bit 8 of this register represents overflow of the 39 bit stat."
    "This is a HW shadow for the upper bits DO NOT USE.",
#endif
    XPCSRX_RX_FEC_CORRECT_CNT_SHADOW_REG_OFFSET,
    0,
    0,
    315,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    XPCSRX_RX_FEC_CORRECT_CNT_SHADOW_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: XPCSRX_RX_FEC_ONES_COR_CNT_LOWER
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XPCSRX_RX_FEC_ONES_COR_CNT_LOWER_FIELDS[] =
{
    &XPCSRX_RX_FEC_ONES_COR_CNT_LOWER_RXFECDECONESCORCNTLOWER_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec XPCSRX_RX_FEC_ONES_COR_CNT_LOWER_REG = 
{
    "RX_FEC_ONES_COR_CNT_LOWER",
#if RU_INCLUDE_DESC
    "XPCSRX_FEC_ONES_COR_CNT_LOWER Register",
    "The number of correctoins made to ones in the FEC CW that have been"
    "recieved."
    "Lower 32 bits of a 39 bit stat."
    "Read this location before reading the upper 7 bis at"
    "XPCSRX_FEC_CORRECT_CNT_UPPER."
    "NOTE: Covers from start of CW through the first 40 bits of the parity.",
#endif
    XPCSRX_RX_FEC_ONES_COR_CNT_LOWER_REG_OFFSET,
    0,
    0,
    316,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    XPCSRX_RX_FEC_ONES_COR_CNT_LOWER_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: XPCSRX_RX_FEC_ONES_COR_CNT_UPPER
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XPCSRX_RX_FEC_ONES_COR_CNT_UPPER_FIELDS[] =
{
    &XPCSRX_RX_FEC_ONES_COR_CNT_UPPER_RESERVED0_FIELD,
    &XPCSRX_RX_FEC_ONES_COR_CNT_UPPER_RXFECDECONESCORCNTUPPER_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec XPCSRX_RX_FEC_ONES_COR_CNT_UPPER_REG = 
{
    "RX_FEC_ONES_COR_CNT_UPPER",
#if RU_INCLUDE_DESC
    "XPCSRX_FEC_ONES_COR_CNT_UPPER Register",
    "The number of correctoins made to ones in the FEC CW that have been"
    "recieved."
    "Upper 7 bits of a 39 bit stat."
    "Bit 8 of this register represents overflow of the 39 bit stat."
    "Read this location after reading the lower 32 bits at"
    "XPCSRX_FEC_CORRECT_CNT_LOWER."
    "NOTE: Covers from start of CW through the first 40 bits of the parity.",
#endif
    XPCSRX_RX_FEC_ONES_COR_CNT_UPPER_REG_OFFSET,
    0,
    0,
    317,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    XPCSRX_RX_FEC_ONES_COR_CNT_UPPER_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: XPCSRX_RX_FEC_ONES_COR_CNT_SHADOW
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XPCSRX_RX_FEC_ONES_COR_CNT_SHADOW_FIELDS[] =
{
    &XPCSRX_RX_FEC_ONES_COR_CNT_SHADOW_RESERVED0_FIELD,
    &XPCSRX_RX_FEC_ONES_COR_CNT_SHADOW_RXFECDECONESCORCNTSHADOW_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec XPCSRX_RX_FEC_ONES_COR_CNT_SHADOW_REG = 
{
    "RX_FEC_ONES_COR_CNT_SHADOW",
#if RU_INCLUDE_DESC
    "XPCSRX_FEC_ONES_COR_CNT_SHADOW Register",
    "The number of correctoins made to ones in the FEC CW that have been"
    "recieved."
    "Upper 7 bits of a 39 bit stat."
    "Bit 8 of this register represents overflow of the 39 bit stat."
    "This is a HW shadow for the upper bits DO NOT USE."
    "NOTE: Covers from start of CW through the first 40 bits of the parity.",
#endif
    XPCSRX_RX_FEC_ONES_COR_CNT_SHADOW_REG_OFFSET,
    0,
    0,
    318,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    XPCSRX_RX_FEC_ONES_COR_CNT_SHADOW_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: XPCSRX_RX_FEC_ZEROS_COR_CNT_LOWER
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XPCSRX_RX_FEC_ZEROS_COR_CNT_LOWER_FIELDS[] =
{
    &XPCSRX_RX_FEC_ZEROS_COR_CNT_LOWER_RXFECDECZEROSCORCNTLOWER_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec XPCSRX_RX_FEC_ZEROS_COR_CNT_LOWER_REG = 
{
    "RX_FEC_ZEROS_COR_CNT_LOWER",
#if RU_INCLUDE_DESC
    "XPCSRX_FEC_ZEROS_COR_CNT_LOWER Register",
    "The number of correctoins made to zeros in the FEC CW that have been"
    "recieved."
    "Lower 32 bits of a 39 bit stat."
    "Read this location before reading the upper 7 bis at"
    "XPCSRX_FEC_CORRECT_CNT_UPPER."
    "NOTE: Covers from start of CW through the first 40 bits of the parity.",
#endif
    XPCSRX_RX_FEC_ZEROS_COR_CNT_LOWER_REG_OFFSET,
    0,
    0,
    319,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    XPCSRX_RX_FEC_ZEROS_COR_CNT_LOWER_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: XPCSRX_RX_FEC_ZEROS_COR_CNT_UPPER
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XPCSRX_RX_FEC_ZEROS_COR_CNT_UPPER_FIELDS[] =
{
    &XPCSRX_RX_FEC_ZEROS_COR_CNT_UPPER_RESERVED0_FIELD,
    &XPCSRX_RX_FEC_ZEROS_COR_CNT_UPPER_RXFECDECZEROSCORCNTUPPER_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec XPCSRX_RX_FEC_ZEROS_COR_CNT_UPPER_REG = 
{
    "RX_FEC_ZEROS_COR_CNT_UPPER",
#if RU_INCLUDE_DESC
    "XPCSRX_FEC_ZEROS_COR_CNT_UPPER Register",
    "The number of correctoins made to zeros in the FEC CW that have been"
    "recieved."
    "Upper 7 bits of a 39 bit stat."
    "Bit 8 of this register represents overflow of the 39 bit stat."
    "Read this location after reading the lower 32 bits at"
    "XPCSRX_FEC_CORRECT_CNT_LOWER."
    "NOTE: Covers from start of CW through the first 40 bits of the parity.",
#endif
    XPCSRX_RX_FEC_ZEROS_COR_CNT_UPPER_REG_OFFSET,
    0,
    0,
    320,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    XPCSRX_RX_FEC_ZEROS_COR_CNT_UPPER_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: XPCSRX_RX_FEC_ZEROS_COR_CNT_SHADOW
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XPCSRX_RX_FEC_ZEROS_COR_CNT_SHADOW_FIELDS[] =
{
    &XPCSRX_RX_FEC_ZEROS_COR_CNT_SHADOW_RESERVED0_FIELD,
    &XPCSRX_RX_FEC_ZEROS_COR_CNT_SHADOW_RXFECDECZEROSCORCNTSHADOW_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec XPCSRX_RX_FEC_ZEROS_COR_CNT_SHADOW_REG = 
{
    "RX_FEC_ZEROS_COR_CNT_SHADOW",
#if RU_INCLUDE_DESC
    "XPCSRX_FEC_ZEROS_COR_CNT_SHADOW Register",
    "The number of correctoins made to zeros in the FEC CW that have been"
    "recieved."
    "Upper 7 bits of a 39 bit stat."
    "Bit 8 of this register represents overflow of the 39 bit stat."
    "This is a HW shadow for the upper bits DO NOT USE."
    "NOTE: Covers from start of CW through the first 40 bits of the parity.",
#endif
    XPCSRX_RX_FEC_ZEROS_COR_CNT_SHADOW_REG_OFFSET,
    0,
    0,
    321,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    XPCSRX_RX_FEC_ZEROS_COR_CNT_SHADOW_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: XPCSRX_RX_FEC_STOP_ON_ERR_READ_POINTER
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XPCSRX_RX_FEC_STOP_ON_ERR_READ_POINTER_FIELDS[] =
{
    &XPCSRX_RX_FEC_STOP_ON_ERR_READ_POINTER_RESERVED0_FIELD,
    &XPCSRX_RX_FEC_STOP_ON_ERR_READ_POINTER_RXFECSTOPONERRRDPTR_FIELD,
    &XPCSRX_RX_FEC_STOP_ON_ERR_READ_POINTER_RXFECSTOPONERRWRPTR_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec XPCSRX_RX_FEC_STOP_ON_ERR_READ_POINTER_REG = 
{
    "RX_FEC_STOP_ON_ERR_READ_POINTER",
#if RU_INCLUDE_DESC
    "XPCSRX_FEC_STOP_ON_ERR_READ_POINTER Register",
    "Captures the write and read pointer for the FEC decoder when a fail"
    "decode occurs.",
#endif
    XPCSRX_RX_FEC_STOP_ON_ERR_READ_POINTER_REG_OFFSET,
    0,
    0,
    322,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    3,
    XPCSRX_RX_FEC_STOP_ON_ERR_READ_POINTER_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: XPCSRX_RX_FEC_STOP_ON_ERR_BURST_LOCATION
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XPCSRX_RX_FEC_STOP_ON_ERR_BURST_LOCATION_FIELDS[] =
{
    &XPCSRX_RX_FEC_STOP_ON_ERR_BURST_LOCATION_RESERVED0_FIELD,
    &XPCSRX_RX_FEC_STOP_ON_ERR_BURST_LOCATION_RXFECSTOPONERRBRSTLOC_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec XPCSRX_RX_FEC_STOP_ON_ERR_BURST_LOCATION_REG = 
{
    "RX_FEC_STOP_ON_ERR_BURST_LOCATION",
#if RU_INCLUDE_DESC
    "XPCSRX_FEC_STOP_ON_ERR_BURST_LOCATION Register",
    "Captures the location within the burst where the stop on error occured",
#endif
    XPCSRX_RX_FEC_STOP_ON_ERR_BURST_LOCATION_REG_OFFSET,
    0,
    0,
    323,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    XPCSRX_RX_FEC_STOP_ON_ERR_BURST_LOCATION_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: XPCSRX_RX_64B66B_FAIL_CNT
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XPCSRX_RX_64B66B_FAIL_CNT_FIELDS[] =
{
    &XPCSRX_RX_64B66B_FAIL_CNT_RX64B66BDECERRCNT_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec XPCSRX_RX_64B66B_FAIL_CNT_REG = 
{
    "RX_64B66B_FAIL_CNT",
#if RU_INCLUDE_DESC
    "XPCSRX_64B66B_FAIL_CNT Register",
    "Count the number of 64b/66b decode errors.",
#endif
    XPCSRX_RX_64B66B_FAIL_CNT_REG_OFFSET,
    0,
    0,
    324,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    XPCSRX_RX_64B66B_FAIL_CNT_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: XPCSRX_RX_FRMR_BAD_SH_CNT
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XPCSRX_RX_FRMR_BAD_SH_CNT_FIELDS[] =
{
    &XPCSRX_RX_FRMR_BAD_SH_CNT_RXFRMRBADSHCNT_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec XPCSRX_RX_FRMR_BAD_SH_CNT_REG = 
{
    "RX_FRMR_BAD_SH_CNT",
#if RU_INCLUDE_DESC
    "XPCSRX_FRMR_BAD_SH_CNT Register",
    "Count the number of bad SH during CW lock.",
#endif
    XPCSRX_RX_FRMR_BAD_SH_CNT_REG_OFFSET,
    0,
    0,
    325,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    XPCSRX_RX_FRMR_BAD_SH_CNT_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: XPCSRX_RX_PSUDO_CNT
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XPCSRX_RX_PSUDO_CNT_FIELDS[] =
{
    &XPCSRX_RX_PSUDO_CNT_RXTESTPSUDOERRCNT_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec XPCSRX_RX_PSUDO_CNT_REG = 
{
    "RX_PSUDO_CNT",
#if RU_INCLUDE_DESC
    "XPCSRX_PSUDO_CNT Register",
    "Count the number of errors in test pattern 2 while in patter lock.",
#endif
    XPCSRX_RX_PSUDO_CNT_REG_OFFSET,
    0,
    0,
    326,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    XPCSRX_RX_PSUDO_CNT_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: XPCSRX_RX_PRBS_CNT
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XPCSRX_RX_PRBS_CNT_FIELDS[] =
{
    &XPCSRX_RX_PRBS_CNT_RXTESTPRBSERRCNT_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec XPCSRX_RX_PRBS_CNT_REG = 
{
    "RX_PRBS_CNT",
#if RU_INCLUDE_DESC
    "XPCSRX_PRBS_CNT Register",
    "Count the number of errors in test pattern PRBS-31 while in pattern"
    "lock.",
#endif
    XPCSRX_RX_PRBS_CNT_REG_OFFSET,
    0,
    0,
    327,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    XPCSRX_RX_PRBS_CNT_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: XPCSRX_RX_BER_INTVAL
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XPCSRX_RX_BER_INTVAL_FIELDS[] =
{
    &XPCSRX_RX_BER_INTVAL_RESERVED0_FIELD,
    &XPCSRX_RX_BER_INTVAL_CFGXPCSRXFRMRBERINTVAL_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec XPCSRX_RX_BER_INTVAL_REG = 
{
    "RX_BER_INTVAL",
#if RU_INCLUDE_DESC
    "XPCSRX_BER_INTVAL Register",
    "The interval over which SH BER is mearsured in the framer. Used to"
    "generate xPcsRxStatFrmrHighBer alarm.",
#endif
    XPCSRX_RX_BER_INTVAL_REG_OFFSET,
    0,
    0,
    328,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    XPCSRX_RX_BER_INTVAL_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: XPCSRX_RX_BER_TRESH
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XPCSRX_RX_BER_TRESH_FIELDS[] =
{
    &XPCSRX_RX_BER_TRESH_RESERVED0_FIELD,
    &XPCSRX_RX_BER_TRESH_CFGXPCSRXFRMRBERTRESH_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec XPCSRX_RX_BER_TRESH_REG = 
{
    "RX_BER_TRESH",
#if RU_INCLUDE_DESC
    "XPCSRX_BER_TRESH Register",
    "The threshold on the number of bit errors made over a timer interval"
    "that will cause the high BER alarm.",
#endif
    XPCSRX_RX_BER_TRESH_REG_OFFSET,
    0,
    0,
    329,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    XPCSRX_RX_BER_TRESH_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: XPCSRX_RX_64B66B_START_CNT
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XPCSRX_RX_64B66B_START_CNT_FIELDS[] =
{
    &XPCSRX_RX_64B66B_START_CNT_RX64B66BDECSTARTCNT_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec XPCSRX_RX_64B66B_START_CNT_REG = 
{
    "RX_64B66B_START_CNT",
#if RU_INCLUDE_DESC
    "XPCSRX_64B66B_START_CNT Register",
    "Count the number of SOP detected in the 64b/66b decoder.",
#endif
    XPCSRX_RX_64B66B_START_CNT_REG_OFFSET,
    0,
    0,
    330,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    XPCSRX_RX_64B66B_START_CNT_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: XPCSRX_RX_IDLE_GOOD_PKT_CNT
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XPCSRX_RX_IDLE_GOOD_PKT_CNT_FIELDS[] =
{
    &XPCSRX_RX_IDLE_GOOD_PKT_CNT_RXIDLEGOODPKTCNT_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec XPCSRX_RX_IDLE_GOOD_PKT_CNT_REG = 
{
    "RX_IDLE_GOOD_PKT_CNT",
#if RU_INCLUDE_DESC
    "XPCSRX_IDLE_GOOD_PKT_CNT Register",
    "Count the number of good packets detected in the idle insertion logic."
    "Based on finding SOP followed by EOP.",
#endif
    XPCSRX_RX_IDLE_GOOD_PKT_CNT_REG_OFFSET,
    0,
    0,
    331,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    XPCSRX_RX_IDLE_GOOD_PKT_CNT_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: XPCSRX_RX_IDLE_ERR_PKT_CNT
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XPCSRX_RX_IDLE_ERR_PKT_CNT_FIELDS[] =
{
    &XPCSRX_RX_IDLE_ERR_PKT_CNT_RXIDLEERRPKTCNT_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec XPCSRX_RX_IDLE_ERR_PKT_CNT_REG = 
{
    "RX_IDLE_ERR_PKT_CNT",
#if RU_INCLUDE_DESC
    "XPCSRX_IDLE_ERR_PKT_CNT Register",
    "Count the number of bad packets detected in the idle insertion logic.",
#endif
    XPCSRX_RX_IDLE_ERR_PKT_CNT_REG_OFFSET,
    0,
    0,
    332,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    XPCSRX_RX_IDLE_ERR_PKT_CNT_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: XPCSRX_RX_64B66B_STOP_CNT
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XPCSRX_RX_64B66B_STOP_CNT_FIELDS[] =
{
    &XPCSRX_RX_64B66B_STOP_CNT_RX64B66BDECSTOPCNT_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec XPCSRX_RX_64B66B_STOP_CNT_REG = 
{
    "RX_64B66B_STOP_CNT",
#if RU_INCLUDE_DESC
    "XPCSRX_64B66B_STOP_CNT Register",
    "Count the number of EOP detected in the 64b/66b decoder.",
#endif
    XPCSRX_RX_64B66B_STOP_CNT_REG_OFFSET,
    0,
    0,
    333,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    XPCSRX_RX_64B66B_STOP_CNT_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: XPCSRX_RX_BURST_OUT_ODR_CNT
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XPCSRX_RX_BURST_OUT_ODR_CNT_FIELDS[] =
{
    &XPCSRX_RX_BURST_OUT_ODR_CNT_RXBURSTSEQOUTOFORDERCNT_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec XPCSRX_RX_BURST_OUT_ODR_CNT_REG = 
{
    "RX_BURST_OUT_ODR_CNT",
#if RU_INCLUDE_DESC
    "XPCSRX_BURST_OUT_ODR_CNT Register",
    "Test mode used in FPGA only. Count the number of burst recieved out of"
    "order.",
#endif
    XPCSRX_RX_BURST_OUT_ODR_CNT_REG_OFFSET,
    0,
    0,
    334,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    XPCSRX_RX_BURST_OUT_ODR_CNT_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: XPCSRX_RX_IDLE_DA_JIT_DLY
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XPCSRX_RX_IDLE_DA_JIT_DLY_FIELDS[] =
{
    &XPCSRX_RX_IDLE_DA_JIT_DLY_RESERVED0_FIELD,
    &XPCSRX_RX_IDLE_DA_JIT_DLY_RXIDLELASTDACNT_FIELD,
    &XPCSRX_RX_IDLE_DA_JIT_DLY_RESERVED1_FIELD,
    &XPCSRX_RX_IDLE_DA_JIT_DLY_RXIDLEDACNT_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec XPCSRX_RX_IDLE_DA_JIT_DLY_REG = 
{
    "RX_IDLE_DA_JIT_DLY",
#if RU_INCLUDE_DESC
    "XPCSRX_IDLE_DA_JIT_DLY Register",
    "Gives delay values for two different DA through the IDLE insert"
    "process.",
#endif
    XPCSRX_RX_IDLE_DA_JIT_DLY_REG_OFFSET,
    0,
    0,
    335,
#if RU_INCLUDE_ACCESS
    ru_access_read,
#endif
#if RU_INCLUDE_FIELD_DB
    4,
    XPCSRX_RX_IDLE_DA_JIT_DLY_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: XPCSRX_RX_DPORT_CTL
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XPCSRX_RX_DPORT_CTL_FIELDS[] =
{
    &XPCSRX_RX_DPORT_CTL_XPCSRXDPBUSY_FIELD,
    &XPCSRX_RX_DPORT_CTL_XPCSRXDPERR_FIELD,
    &XPCSRX_RX_DPORT_CTL_RESERVED0_FIELD,
    &XPCSRX_RX_DPORT_CTL_CFGXPCSRXDPCTL_FIELD,
    &XPCSRX_RX_DPORT_CTL_CFGXPCSRXDPRAMSEL_FIELD,
    &XPCSRX_RX_DPORT_CTL_CFGXPCSRXDPADDR_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec XPCSRX_RX_DPORT_CTL_REG = 
{
    "RX_DPORT_CTL",
#if RU_INCLUDE_DESC
    "XPCSRX_DPORT_CTL Register",
    "Provides data port access to all XPCS RX RAMS."
    "The data port functions as a means to access RAMs by way of"
    "indirection."
    ""
    "The address, commands and busy status are located in this register."
    "XPCSRX_DPORT_CTL"
    ""
    "The data to be read or written are accesses at:"
    "XPCSRX_DPORT_DATA0"
    "XPCSRX_DPORT_DATA1"
    "XPCSRX_DPORT_DATA2"
    "XPCSRX_DPORT_DATA3"
    "XPCSRX_DPORT_DATA4"
    ""
    "The control to allow this data port to access data path RAMs are"
    "located:"
    "XPCSRX_DPORT_ACC"
    ""
    "The control to disable clear on read for XPCS RX Stats Ram is located:"
    "XPCSRX_DPORT_FEC_STATS_CTL"
    ""
    "The capture FIFO RAM, FEC decode RAM, FEC enqueue RAM and idle insert"
    "RAM"
    "are data path rams.  In order to access them a select must be set."
    "This"
    "disables the data path and allows the data port to have access.  The"
    "XPCS RX"
    "will not function properly under these conditions.  Accesses to these"
    "RAMs"
    "by this data port is for test only."
    ""
    "The FEC decode stats RAM also has a control bit associated with it."
    "This"
    "control bit is not associated with permitting access to this RAM."
    "Accesses"
    "to this RAM by this data port is intended for both the normal mode of"
    "operation and testing. This bit is to disable the clear on read"
    "function"
    "that occurs with normal read accesses to this RAM by this data port."
    ""
    "CONTRLS:"
    ""
    "To access the capture FIFO RAM, FEC decode RAM, FEC enqueue RAM or idle"
    ""
    "insert RAM, set the data port select bit in XPCSRX_DPORT_ACC."
    ""
    "To access the FEC decode stats RAM without clear on read set, set the"
    "disable bit in XPCSRX_DPORT_FEC_STATS_CTL.  Do not set this in"
    "normal operation.  It is intended that statistics are clear on read."
    ""
    "For writes, set the write values in the data registers."
    "Set the address, RAM select and control fields in this register."
    "Poll the busy bit in this register."
    "Once the busy bit is not set the data port has completed the command."
    "For reads, read the data register after the busy bit clears."
    ""
    "Statistics and Data Fields descriptions for the RAMS:"
    ""
    "RAM  | Type  | Size  | ecc field | data"
    "field"
    "======================================================================="
    "====="
    "Capture FIFO | PD  | 256x80 | [79:72] |"
    "[71:0]"
    "FEC enqueue | RF  | 32x80  | [79:72] |"
    "[71:0]"
    "FEC decode | PD  | 256x80 | [79:72] |"
    "[71:0]"
    "FEC stats | SP  | 128x151 | [150:142] |"
    "[141:0]"
    "Idle insert | PD  | 256x82 | [81:73] |"
    "[73:0]"
    ""
    "Data fields descripton:"
    ""
    "Capture FIFO"
    "[71:64]  XPCS RX control"
    "[63:0]  XPCS RX data"
    ""
    "FEC enqueue"
    "[71:0]  aligned 66b data geared up to 72b"
    ""
    "FEC decode"
    "[71:0]  aligned 66b data geared up to 72b"
    ""
    "FEC stats RAM"
    "[141:103] ones corected statistics"
    "[102:64] zeros corrected statistics"
    "[63:32]  code words decode fails statistics"
    "[31:0]  code words total statistics"
    ""
    "Idle insert"
    "[73:72]  number of invalid times between valid"
    "[71:68]  control indication"
    "[69:36]  four bytes of 8b data"
    "[35:32]  control indication"
    "[31:0]  four bytes of 8b data",
#endif
    XPCSRX_RX_DPORT_CTL_REG_OFFSET,
    0,
    0,
    336,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    6,
    XPCSRX_RX_DPORT_CTL_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: XPCSRX_RX_DPORT_DATA0
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XPCSRX_RX_DPORT_DATA0_FIELDS[] =
{
    &XPCSRX_RX_DPORT_DATA0_XPCSRXDPDATA0_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec XPCSRX_RX_DPORT_DATA0_REG = 
{
    "RX_DPORT_DATA0",
#if RU_INCLUDE_DESC
    "XPCSRX_DPORT_DATA0 Register",
    "Rreadback register and write register for data port accesses.",
#endif
    XPCSRX_RX_DPORT_DATA0_REG_OFFSET,
    0,
    0,
    337,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    XPCSRX_RX_DPORT_DATA0_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: XPCSRX_RX_DPORT_DATA1
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XPCSRX_RX_DPORT_DATA1_FIELDS[] =
{
    &XPCSRX_RX_DPORT_DATA1_XPCSRXDPDATA1_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec XPCSRX_RX_DPORT_DATA1_REG = 
{
    "RX_DPORT_DATA1",
#if RU_INCLUDE_DESC
    "XPCSRX_DPORT_DATA1 Register",
    "Rreadback register and write register for data port accesses.",
#endif
    XPCSRX_RX_DPORT_DATA1_REG_OFFSET,
    0,
    0,
    338,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    XPCSRX_RX_DPORT_DATA1_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: XPCSRX_RX_DPORT_DATA2
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XPCSRX_RX_DPORT_DATA2_FIELDS[] =
{
    &XPCSRX_RX_DPORT_DATA2_XPCSRXDPDATA2_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec XPCSRX_RX_DPORT_DATA2_REG = 
{
    "RX_DPORT_DATA2",
#if RU_INCLUDE_DESC
    "XPCSRX_DPORT_DATA2 Register",
    "Rreadback register and write register for data port accesses.",
#endif
    XPCSRX_RX_DPORT_DATA2_REG_OFFSET,
    0,
    0,
    339,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    XPCSRX_RX_DPORT_DATA2_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: XPCSRX_RX_DPORT_ACC
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XPCSRX_RX_DPORT_ACC_FIELDS[] =
{
    &XPCSRX_RX_DPORT_ACC_RESERVED0_FIELD,
    &XPCSRX_RX_DPORT_ACC_CFGXPCSRXIDLERAMDPSEL_FIELD,
    &XPCSRX_RX_DPORT_ACC_CFGXPCSRXFECDECRAMDPSEL_FIELD,
    &XPCSRX_RX_DPORT_ACC_CFGXPCSRXFECNQUERAMDPSEL_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec XPCSRX_RX_DPORT_ACC_REG = 
{
    "RX_DPORT_ACC",
#if RU_INCLUDE_DESC
    "XPCSRX_DPORT_ACC Register",
    "Provides data port access to all XPCS RX RAMS.",
#endif
    XPCSRX_RX_DPORT_ACC_REG_OFFSET,
    0,
    0,
    340,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    4,
    XPCSRX_RX_DPORT_ACC_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: XPCSRX_RX_RAM_ECC_INT_STAT
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XPCSRX_RX_RAM_ECC_INT_STAT_FIELDS[] =
{
    &XPCSRX_RX_RAM_ECC_INT_STAT_RESERVED0_FIELD,
    &XPCSRX_RX_RAM_ECC_INT_STAT_INTRXIDLERAMINITDONE_FIELD,
    &XPCSRX_RX_RAM_ECC_INT_STAT_INTRXFECNQUERAMINITDONE_FIELD,
    &XPCSRX_RX_RAM_ECC_INT_STAT_RESERVED1_FIELD,
    &XPCSRX_RX_RAM_ECC_INT_STAT_INTRXFECDECRAMINITDONE_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec XPCSRX_RX_RAM_ECC_INT_STAT_REG = 
{
    "RX_RAM_ECC_INT_STAT",
#if RU_INCLUDE_DESC
    "XPCSRX_RAM_ECC_INT_STAT Register",
    "Interrupt status for XPcsRx RAMs ECC.",
#endif
    XPCSRX_RX_RAM_ECC_INT_STAT_REG_OFFSET,
    0,
    0,
    341,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    5,
    XPCSRX_RX_RAM_ECC_INT_STAT_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: XPCSRX_RX_RAM_ECC_INT_MSK
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XPCSRX_RX_RAM_ECC_INT_MSK_FIELDS[] =
{
    &XPCSRX_RX_RAM_ECC_INT_MSK_RESERVED0_FIELD,
    &XPCSRX_RX_RAM_ECC_INT_MSK_MSKRXIDLERAMINITDONE_FIELD,
    &XPCSRX_RX_RAM_ECC_INT_MSK_MSKRXFECNQUERAMINITDONE_FIELD,
    &XPCSRX_RX_RAM_ECC_INT_MSK_RESERVED1_FIELD,
    &XPCSRX_RX_RAM_ECC_INT_MSK_MSKRXFECDECRAMINITDONE_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec XPCSRX_RX_RAM_ECC_INT_MSK_REG = 
{
    "RX_RAM_ECC_INT_MSK",
#if RU_INCLUDE_DESC
    "XPCSRX_RAM_ECC_INT_MSK Register",
    "Interrupt mask for XPcsRx RAMs ECC.",
#endif
    XPCSRX_RX_RAM_ECC_INT_MSK_REG_OFFSET,
    0,
    0,
    342,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    5,
    XPCSRX_RX_RAM_ECC_INT_MSK_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: XPCSRX_RX_DFT_TESTMODE
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XPCSRX_RX_DFT_TESTMODE_FIELDS[] =
{
    &XPCSRX_RX_DFT_TESTMODE_RESERVED0_FIELD,
    &XPCSRX_RX_DFT_TESTMODE_TM_PD_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec XPCSRX_RX_DFT_TESTMODE_REG = 
{
    "RX_DFT_TESTMODE",
#if RU_INCLUDE_DESC
    "XPCSRX_DFT_TESTMODE Register",
    "DFT test mode for PD RAMs",
#endif
    XPCSRX_RX_DFT_TESTMODE_REG_OFFSET,
    0,
    0,
    343,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    2,
    XPCSRX_RX_DFT_TESTMODE_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: XPCSRX_RX_RAM_POWER_PDA_CTL0
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XPCSRX_RX_RAM_POWER_PDA_CTL0_FIELDS[] =
{
    &XPCSRX_RX_RAM_POWER_PDA_CTL0_RESERVED0_FIELD,
    &XPCSRX_RX_RAM_POWER_PDA_CTL0_CFGXPCSRXIDLERAMPDA_FIELD,
    &XPCSRX_RX_RAM_POWER_PDA_CTL0_RESERVED1_FIELD,
    &XPCSRX_RX_RAM_POWER_PDA_CTL0_CFGXPCSRXFECDECRAMPDA_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec XPCSRX_RX_RAM_POWER_PDA_CTL0_REG = 
{
    "RX_RAM_POWER_PDA_CTL0",
#if RU_INCLUDE_DESC
    "XPCSRX_RAM_POWER_PDA_CTL0 Register",
    "Control register to selectively power one or more rowblocks of the"
    "memory to acieve improved power reduction. There is one bit per"
    "rowblock for the specific RAM. All array contents are lost for the"
    "rowblocks that are powered down.  the rowblocks in operational mode"
    "will be available for read/write and data retention."
    ""
    "1 = power down"
    "0 = operational"
    ""
    "NOTE:  When powering up, do NOT power up more than one array in one RAM"
    "at a time.  It may damage the RAM.",
#endif
    XPCSRX_RX_RAM_POWER_PDA_CTL0_REG_OFFSET,
    0,
    0,
    344,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    4,
    XPCSRX_RX_RAM_POWER_PDA_CTL0_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: XPCSRX_RX_INT_STAT1
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XPCSRX_RX_INT_STAT1_FIELDS[] =
{
    &XPCSRX_RX_INT_STAT1_RESERVED0_FIELD,
    &XPCSRX_RX_INT_STAT1_INTRX64B66BTRAILSTART_FIELD,
    &XPCSRX_RX_INT_STAT1_INTRX64B66BTWOSTOP_FIELD,
    &XPCSRX_RX_INT_STAT1_INTRX64B66BTWOSTART_FIELD,
    &XPCSRX_RX_INT_STAT1_INTRX64B66BLEADSTOP_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec XPCSRX_RX_INT_STAT1_REG = 
{
    "RX_INT_STAT1",
#if RU_INCLUDE_DESC
    "XPCSRX_INT_STAT1 Register",
    "More Interrupt status for XPcsRx module.",
#endif
    XPCSRX_RX_INT_STAT1_REG_OFFSET,
    0,
    0,
    345,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    5,
    XPCSRX_RX_INT_STAT1_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: XPCSRX_RX_INT_MSK1
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XPCSRX_RX_INT_MSK1_FIELDS[] =
{
    &XPCSRX_RX_INT_MSK1_RESERVED0_FIELD,
    &XPCSRX_RX_INT_MSK1_MSKRX64B66BTRAILSTART_FIELD,
    &XPCSRX_RX_INT_MSK1_MSKRX64B66BTWOSTOP_FIELD,
    &XPCSRX_RX_INT_MSK1_MSKRX64B66BTWOSTART_FIELD,
    &XPCSRX_RX_INT_MSK1_MSKRX64B66BLEADSTOP_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec XPCSRX_RX_INT_MSK1_REG = 
{
    "RX_INT_MSK1",
#if RU_INCLUDE_DESC
    "XPCSRX_INT_MSK1 Register",
    "More Interrupt mask for XPcsRx module.",
#endif
    XPCSRX_RX_INT_MSK1_REG_OFFSET,
    0,
    0,
    346,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    5,
    XPCSRX_RX_INT_MSK1_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Register: XPCSRX_RX_SPARE_CTL
 ******************************************************************************/
#if RU_INCLUDE_FIELD_DB
static const ru_field_rec *XPCSRX_RX_SPARE_CTL_FIELDS[] =
{
    &XPCSRX_RX_SPARE_CTL_CFGXPCSRXSPARE_FIELD,
};

#endif /* RU_INCLUDE_FIELD_DB */

const ru_reg_rec XPCSRX_RX_SPARE_CTL_REG = 
{
    "RX_SPARE_CTL",
#if RU_INCLUDE_DESC
    "XPCSRX_SPARE_CTL Register",
    "Spare RW bits",
#endif
    XPCSRX_RX_SPARE_CTL_REG_OFFSET,
    0,
    0,
    347,
#if RU_INCLUDE_ACCESS
    ru_access_rw,
#endif
#if RU_INCLUDE_FIELD_DB
    1,
    XPCSRX_RX_SPARE_CTL_FIELDS,
#endif /* RU_INCLUDE_FIELD_DB */
    ru_reg_size_32
};

/******************************************************************************
 * Block: XPCSRX
 ******************************************************************************/
static const ru_reg_rec *XPCSRX_REGS[] =
{
    &XPCSRX_RX_RST_REG,
    &XPCSRX_RX_INT_STAT_REG,
    &XPCSRX_RX_INT_MSK_REG,
    &XPCSRX_RX_FRAMER_CTL_REG,
    &XPCSRX_RX_FEC_CTL_REG,
    &XPCSRX_RX_DSCRAM_CTL_REG,
    &XPCSRX_RX_64B66B_CTL_REG,
    &XPCSRX_RX_TEST_CTL_REG,
    &XPCSRX_RX_IDLE_RD_TIMER_DLY_REG,
    &XPCSRX_RX_IDLE_GAP_SIZ_MAX_REG,
    &XPCSRX_RX_FRAMER_LK_MAX_REG,
    &XPCSRX_RX_FRAMER_UNLK_MAX_REG,
    &XPCSRX_RX_FRAMER_BD_SH_REG,
    &XPCSRX_RX_FRAMER_BD_LO_REG,
    &XPCSRX_RX_FRAMER_BD_HI_REG,
    &XPCSRX_RX_FRAMER_EBD_SH_REG,
    &XPCSRX_RX_FRAMER_EBD_LO_REG,
    &XPCSRX_RX_FRAMER_EBD_HI_REG,
    &XPCSRX_RX_STATUS_REG,
    &XPCSRX_RX_FRAMER_LK_ULK_MAX_REG,
    &XPCSRX_RX_FRAMER_SP_SH_REG,
    &XPCSRX_RX_FRAMER_SP_LO_REG,
    &XPCSRX_RX_FRAMER_SP_HI_REG,
    &XPCSRX_RX_FRAMER_STATE_REG,
    &XPCSRX_RX_FRAMER_BD_EBD_HAM_REG,
    &XPCSRX_RX_FRAMER_MISBRST_CNT_REG,
    &XPCSRX_RX_FRAMER_BD_ERR_REG,
    &XPCSRX_RX_FRAMER_ROGUE_CTL_REG,
    &XPCSRX_RX_FRAMER_NOLOCK_CTL_REG,
    &XPCSRX_RX_64B66B_IPG_DET_CNT_REG,
    &XPCSRX_RX_FEC_NQUE_IN_CNT_REG,
    &XPCSRX_RX_FEC_NQUE_OUT_CNT_REG,
    &XPCSRX_RX_IDLE_START_CNT_REG,
    &XPCSRX_RX_IDLE_STOP_CNT_REG,
    &XPCSRX_RX_FEC_COR_INTVAL_REG,
    &XPCSRX_RX_FEC_COR_TRESH_REG,
    &XPCSRX_RX_FEC_CW_FAIL_CNT_REG,
    &XPCSRX_RX_FEC_CW_TOT_CNT_REG,
    &XPCSRX_RX_FEC_CORRECT_CNT_LOWER_REG,
    &XPCSRX_RX_FEC_CORRECT_CNT_UPPER_REG,
    &XPCSRX_RX_FEC_CORRECT_CNT_SHADOW_REG,
    &XPCSRX_RX_FEC_ONES_COR_CNT_LOWER_REG,
    &XPCSRX_RX_FEC_ONES_COR_CNT_UPPER_REG,
    &XPCSRX_RX_FEC_ONES_COR_CNT_SHADOW_REG,
    &XPCSRX_RX_FEC_ZEROS_COR_CNT_LOWER_REG,
    &XPCSRX_RX_FEC_ZEROS_COR_CNT_UPPER_REG,
    &XPCSRX_RX_FEC_ZEROS_COR_CNT_SHADOW_REG,
    &XPCSRX_RX_FEC_STOP_ON_ERR_READ_POINTER_REG,
    &XPCSRX_RX_FEC_STOP_ON_ERR_BURST_LOCATION_REG,
    &XPCSRX_RX_64B66B_FAIL_CNT_REG,
    &XPCSRX_RX_FRMR_BAD_SH_CNT_REG,
    &XPCSRX_RX_PSUDO_CNT_REG,
    &XPCSRX_RX_PRBS_CNT_REG,
    &XPCSRX_RX_BER_INTVAL_REG,
    &XPCSRX_RX_BER_TRESH_REG,
    &XPCSRX_RX_64B66B_START_CNT_REG,
    &XPCSRX_RX_IDLE_GOOD_PKT_CNT_REG,
    &XPCSRX_RX_IDLE_ERR_PKT_CNT_REG,
    &XPCSRX_RX_64B66B_STOP_CNT_REG,
    &XPCSRX_RX_BURST_OUT_ODR_CNT_REG,
    &XPCSRX_RX_IDLE_DA_JIT_DLY_REG,
    &XPCSRX_RX_DPORT_CTL_REG,
    &XPCSRX_RX_DPORT_DATA0_REG,
    &XPCSRX_RX_DPORT_DATA1_REG,
    &XPCSRX_RX_DPORT_DATA2_REG,
    &XPCSRX_RX_DPORT_ACC_REG,
    &XPCSRX_RX_RAM_ECC_INT_STAT_REG,
    &XPCSRX_RX_RAM_ECC_INT_MSK_REG,
    &XPCSRX_RX_DFT_TESTMODE_REG,
    &XPCSRX_RX_RAM_POWER_PDA_CTL0_REG,
    &XPCSRX_RX_INT_STAT1_REG,
    &XPCSRX_RX_INT_MSK1_REG,
    &XPCSRX_RX_SPARE_CTL_REG,
};

static unsigned long XPCSRX_ADDRS[] =
{
    0x80143000,
};

const ru_block_rec XPCSRX_BLOCK = 
{
    "XPCSRX",
    XPCSRX_ADDRS,
    1,
    73,
    XPCSRX_REGS
};

/* End of file EPON_XPCSRX.c */
