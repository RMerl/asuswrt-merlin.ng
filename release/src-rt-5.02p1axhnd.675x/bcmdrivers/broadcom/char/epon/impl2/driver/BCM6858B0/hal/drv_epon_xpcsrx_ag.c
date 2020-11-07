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

#include "drivers_epon_ag.h"
#include "drv_epon_xpcsrx_ag.h"
bdmf_error_t ag_drv_xpcsrx_rx_rst_set(bdmf_boolean cfgxpcsrxclk161rstn)
{
    uint32_t reg_rx_rst=0;

#ifdef VALIDATE_PARMS
    if((cfgxpcsrxclk161rstn >= _1BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_rx_rst = RU_FIELD_SET(0, XPCSRX, RX_RST, CFGXPCSRXCLK161RSTN, reg_rx_rst, cfgxpcsrxclk161rstn);

    RU_REG_WRITE(0, XPCSRX, RX_RST, reg_rx_rst);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xpcsrx_rx_rst_get(bdmf_boolean *cfgxpcsrxclk161rstn)
{
    uint32_t reg_rx_rst=0;

#ifdef VALIDATE_PARMS
    if(!cfgxpcsrxclk161rstn)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, XPCSRX, RX_RST, reg_rx_rst);

    *cfgxpcsrxclk161rstn = RU_FIELD_GET(0, XPCSRX, RX_RST, CFGXPCSRXCLK161RSTN, reg_rx_rst);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xpcsrx_rx_int_stat_set(const xpcsrx_rx_int_stat *rx_int_stat)
{
    uint32_t reg_rx_int_stat=0;

#ifdef VALIDATE_PARMS
    if(!rx_int_stat)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((rx_int_stat->intrxidledajit >= _1BITS_MAX_VAL_) ||
       (rx_int_stat->intrxfrmrmisbrst >= _1BITS_MAX_VAL_) ||
       (rx_int_stat->intrxidlesopeopgapbig >= _1BITS_MAX_VAL_) ||
       (rx_int_stat->intrxidlefrcins >= _1BITS_MAX_VAL_) ||
       (rx_int_stat->intrx64b66bminipgerr >= _1BITS_MAX_VAL_) ||
       (rx_int_stat->intrxfecnquecntneq >= _1BITS_MAX_VAL_) ||
       (rx_int_stat->intrxidlefifoundrun >= _1BITS_MAX_VAL_) ||
       (rx_int_stat->intrxidlefifoovrrun >= _1BITS_MAX_VAL_) ||
       (rx_int_stat->intrxfechighcor >= _1BITS_MAX_VAL_) ||
       (rx_int_stat->intrxfecdecstoponerr >= _1BITS_MAX_VAL_) ||
       (rx_int_stat->intrxfecdecpass >= _1BITS_MAX_VAL_) ||
       (rx_int_stat->intrxstatfrmrhighber >= _1BITS_MAX_VAL_) ||
       (rx_int_stat->intrxfrmrexitbysp >= _1BITS_MAX_VAL_) ||
       (rx_int_stat->intrxfrmrbadshmax >= _1BITS_MAX_VAL_) ||
       (rx_int_stat->intrxdscramburstseqout >= _1BITS_MAX_VAL_) ||
       (rx_int_stat->intrxtestpsudolock >= _1BITS_MAX_VAL_) ||
       (rx_int_stat->intrxtestpsudotype >= _1BITS_MAX_VAL_) ||
       (rx_int_stat->intrxtestpsudoerr >= _1BITS_MAX_VAL_) ||
       (rx_int_stat->intrxtestprbslock >= _1BITS_MAX_VAL_) ||
       (rx_int_stat->intrxtestprbserr >= _1BITS_MAX_VAL_) ||
       (rx_int_stat->intrxfecpsistdecfail >= _1BITS_MAX_VAL_) ||
       (rx_int_stat->intrxframerbadsh >= _1BITS_MAX_VAL_) ||
       (rx_int_stat->intrxframercwloss >= _1BITS_MAX_VAL_) ||
       (rx_int_stat->intrxframercwlock >= _1BITS_MAX_VAL_) ||
       (rx_int_stat->intrxfecdecfail >= _1BITS_MAX_VAL_) ||
       (rx_int_stat->intrx64b66bdecerr >= _1BITS_MAX_VAL_) ||
       (rx_int_stat->intrxfrmrnolocklos >= _1BITS_MAX_VAL_) ||
       (rx_int_stat->intrxfrmrrogue >= _1BITS_MAX_VAL_) ||
       (rx_int_stat->int_regs_err >= _1BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_rx_int_stat = RU_FIELD_SET(0, XPCSRX, RX_INT_STAT, INTRXIDLEDAJIT, reg_rx_int_stat, rx_int_stat->intrxidledajit);
    reg_rx_int_stat = RU_FIELD_SET(0, XPCSRX, RX_INT_STAT, INTRXFRMRMISBRST, reg_rx_int_stat, rx_int_stat->intrxfrmrmisbrst);
    reg_rx_int_stat = RU_FIELD_SET(0, XPCSRX, RX_INT_STAT, INTRXIDLESOPEOPGAPBIG, reg_rx_int_stat, rx_int_stat->intrxidlesopeopgapbig);
    reg_rx_int_stat = RU_FIELD_SET(0, XPCSRX, RX_INT_STAT, INTRXIDLEFRCINS, reg_rx_int_stat, rx_int_stat->intrxidlefrcins);
    reg_rx_int_stat = RU_FIELD_SET(0, XPCSRX, RX_INT_STAT, INTRX64B66BMINIPGERR, reg_rx_int_stat, rx_int_stat->intrx64b66bminipgerr);
    reg_rx_int_stat = RU_FIELD_SET(0, XPCSRX, RX_INT_STAT, INTRXFECNQUECNTNEQ, reg_rx_int_stat, rx_int_stat->intrxfecnquecntneq);
    reg_rx_int_stat = RU_FIELD_SET(0, XPCSRX, RX_INT_STAT, INTRXIDLEFIFOUNDRUN, reg_rx_int_stat, rx_int_stat->intrxidlefifoundrun);
    reg_rx_int_stat = RU_FIELD_SET(0, XPCSRX, RX_INT_STAT, INTRXIDLEFIFOOVRRUN, reg_rx_int_stat, rx_int_stat->intrxidlefifoovrrun);
    reg_rx_int_stat = RU_FIELD_SET(0, XPCSRX, RX_INT_STAT, INTRXFECHIGHCOR, reg_rx_int_stat, rx_int_stat->intrxfechighcor);
    reg_rx_int_stat = RU_FIELD_SET(0, XPCSRX, RX_INT_STAT, INTRXFECDECSTOPONERR, reg_rx_int_stat, rx_int_stat->intrxfecdecstoponerr);
    reg_rx_int_stat = RU_FIELD_SET(0, XPCSRX, RX_INT_STAT, INTRXFECDECPASS, reg_rx_int_stat, rx_int_stat->intrxfecdecpass);
    reg_rx_int_stat = RU_FIELD_SET(0, XPCSRX, RX_INT_STAT, INTRXSTATFRMRHIGHBER, reg_rx_int_stat, rx_int_stat->intrxstatfrmrhighber);
    reg_rx_int_stat = RU_FIELD_SET(0, XPCSRX, RX_INT_STAT, INTRXFRMREXITBYSP, reg_rx_int_stat, rx_int_stat->intrxfrmrexitbysp);
    reg_rx_int_stat = RU_FIELD_SET(0, XPCSRX, RX_INT_STAT, INTRXFRMRBADSHMAX, reg_rx_int_stat, rx_int_stat->intrxfrmrbadshmax);
    reg_rx_int_stat = RU_FIELD_SET(0, XPCSRX, RX_INT_STAT, INTRXDSCRAMBURSTSEQOUT, reg_rx_int_stat, rx_int_stat->intrxdscramburstseqout);
    reg_rx_int_stat = RU_FIELD_SET(0, XPCSRX, RX_INT_STAT, INTRXTESTPSUDOLOCK, reg_rx_int_stat, rx_int_stat->intrxtestpsudolock);
    reg_rx_int_stat = RU_FIELD_SET(0, XPCSRX, RX_INT_STAT, INTRXTESTPSUDOTYPE, reg_rx_int_stat, rx_int_stat->intrxtestpsudotype);
    reg_rx_int_stat = RU_FIELD_SET(0, XPCSRX, RX_INT_STAT, INTRXTESTPSUDOERR, reg_rx_int_stat, rx_int_stat->intrxtestpsudoerr);
    reg_rx_int_stat = RU_FIELD_SET(0, XPCSRX, RX_INT_STAT, INTRXTESTPRBSLOCK, reg_rx_int_stat, rx_int_stat->intrxtestprbslock);
    reg_rx_int_stat = RU_FIELD_SET(0, XPCSRX, RX_INT_STAT, INTRXTESTPRBSERR, reg_rx_int_stat, rx_int_stat->intrxtestprbserr);
    reg_rx_int_stat = RU_FIELD_SET(0, XPCSRX, RX_INT_STAT, INTRXFECPSISTDECFAIL, reg_rx_int_stat, rx_int_stat->intrxfecpsistdecfail);
    reg_rx_int_stat = RU_FIELD_SET(0, XPCSRX, RX_INT_STAT, INTRXFRAMERBADSH, reg_rx_int_stat, rx_int_stat->intrxframerbadsh);
    reg_rx_int_stat = RU_FIELD_SET(0, XPCSRX, RX_INT_STAT, INTRXFRAMERCWLOSS, reg_rx_int_stat, rx_int_stat->intrxframercwloss);
    reg_rx_int_stat = RU_FIELD_SET(0, XPCSRX, RX_INT_STAT, INTRXFRAMERCWLOCK, reg_rx_int_stat, rx_int_stat->intrxframercwlock);
    reg_rx_int_stat = RU_FIELD_SET(0, XPCSRX, RX_INT_STAT, INTRXFECDECFAIL, reg_rx_int_stat, rx_int_stat->intrxfecdecfail);
    reg_rx_int_stat = RU_FIELD_SET(0, XPCSRX, RX_INT_STAT, INTRX64B66BDECERR, reg_rx_int_stat, rx_int_stat->intrx64b66bdecerr);
    reg_rx_int_stat = RU_FIELD_SET(0, XPCSRX, RX_INT_STAT, INTRXFRMRNOLOCKLOS, reg_rx_int_stat, rx_int_stat->intrxfrmrnolocklos);
    reg_rx_int_stat = RU_FIELD_SET(0, XPCSRX, RX_INT_STAT, INTRXFRMRROGUE, reg_rx_int_stat, rx_int_stat->intrxfrmrrogue);
    reg_rx_int_stat = RU_FIELD_SET(0, XPCSRX, RX_INT_STAT, INT_REGS_ERR, reg_rx_int_stat, rx_int_stat->int_regs_err);

    RU_REG_WRITE(0, XPCSRX, RX_INT_STAT, reg_rx_int_stat);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xpcsrx_rx_int_stat_get(xpcsrx_rx_int_stat *rx_int_stat)
{
    uint32_t reg_rx_int_stat=0;

#ifdef VALIDATE_PARMS
    if(!rx_int_stat)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, XPCSRX, RX_INT_STAT, reg_rx_int_stat);

    rx_int_stat->intrxidledajit = RU_FIELD_GET(0, XPCSRX, RX_INT_STAT, INTRXIDLEDAJIT, reg_rx_int_stat);
    rx_int_stat->intrxfrmrmisbrst = RU_FIELD_GET(0, XPCSRX, RX_INT_STAT, INTRXFRMRMISBRST, reg_rx_int_stat);
    rx_int_stat->intrxidlesopeopgapbig = RU_FIELD_GET(0, XPCSRX, RX_INT_STAT, INTRXIDLESOPEOPGAPBIG, reg_rx_int_stat);
    rx_int_stat->intrxidlefrcins = RU_FIELD_GET(0, XPCSRX, RX_INT_STAT, INTRXIDLEFRCINS, reg_rx_int_stat);
    rx_int_stat->intrx64b66bminipgerr = RU_FIELD_GET(0, XPCSRX, RX_INT_STAT, INTRX64B66BMINIPGERR, reg_rx_int_stat);
    rx_int_stat->intrxfecnquecntneq = RU_FIELD_GET(0, XPCSRX, RX_INT_STAT, INTRXFECNQUECNTNEQ, reg_rx_int_stat);
    rx_int_stat->intrxidlefifoundrun = RU_FIELD_GET(0, XPCSRX, RX_INT_STAT, INTRXIDLEFIFOUNDRUN, reg_rx_int_stat);
    rx_int_stat->intrxidlefifoovrrun = RU_FIELD_GET(0, XPCSRX, RX_INT_STAT, INTRXIDLEFIFOOVRRUN, reg_rx_int_stat);
    rx_int_stat->intrxfechighcor = RU_FIELD_GET(0, XPCSRX, RX_INT_STAT, INTRXFECHIGHCOR, reg_rx_int_stat);
    rx_int_stat->intrxfecdecstoponerr = RU_FIELD_GET(0, XPCSRX, RX_INT_STAT, INTRXFECDECSTOPONERR, reg_rx_int_stat);
    rx_int_stat->intrxfecdecpass = RU_FIELD_GET(0, XPCSRX, RX_INT_STAT, INTRXFECDECPASS, reg_rx_int_stat);
    rx_int_stat->intrxstatfrmrhighber = RU_FIELD_GET(0, XPCSRX, RX_INT_STAT, INTRXSTATFRMRHIGHBER, reg_rx_int_stat);
    rx_int_stat->intrxfrmrexitbysp = RU_FIELD_GET(0, XPCSRX, RX_INT_STAT, INTRXFRMREXITBYSP, reg_rx_int_stat);
    rx_int_stat->intrxfrmrbadshmax = RU_FIELD_GET(0, XPCSRX, RX_INT_STAT, INTRXFRMRBADSHMAX, reg_rx_int_stat);
    rx_int_stat->intrxdscramburstseqout = RU_FIELD_GET(0, XPCSRX, RX_INT_STAT, INTRXDSCRAMBURSTSEQOUT, reg_rx_int_stat);
    rx_int_stat->intrxtestpsudolock = RU_FIELD_GET(0, XPCSRX, RX_INT_STAT, INTRXTESTPSUDOLOCK, reg_rx_int_stat);
    rx_int_stat->intrxtestpsudotype = RU_FIELD_GET(0, XPCSRX, RX_INT_STAT, INTRXTESTPSUDOTYPE, reg_rx_int_stat);
    rx_int_stat->intrxtestpsudoerr = RU_FIELD_GET(0, XPCSRX, RX_INT_STAT, INTRXTESTPSUDOERR, reg_rx_int_stat);
    rx_int_stat->intrxtestprbslock = RU_FIELD_GET(0, XPCSRX, RX_INT_STAT, INTRXTESTPRBSLOCK, reg_rx_int_stat);
    rx_int_stat->intrxtestprbserr = RU_FIELD_GET(0, XPCSRX, RX_INT_STAT, INTRXTESTPRBSERR, reg_rx_int_stat);
    rx_int_stat->intrxfecpsistdecfail = RU_FIELD_GET(0, XPCSRX, RX_INT_STAT, INTRXFECPSISTDECFAIL, reg_rx_int_stat);
    rx_int_stat->intrxframerbadsh = RU_FIELD_GET(0, XPCSRX, RX_INT_STAT, INTRXFRAMERBADSH, reg_rx_int_stat);
    rx_int_stat->intrxframercwloss = RU_FIELD_GET(0, XPCSRX, RX_INT_STAT, INTRXFRAMERCWLOSS, reg_rx_int_stat);
    rx_int_stat->intrxframercwlock = RU_FIELD_GET(0, XPCSRX, RX_INT_STAT, INTRXFRAMERCWLOCK, reg_rx_int_stat);
    rx_int_stat->intrxfecdecfail = RU_FIELD_GET(0, XPCSRX, RX_INT_STAT, INTRXFECDECFAIL, reg_rx_int_stat);
    rx_int_stat->intrx64b66bdecerr = RU_FIELD_GET(0, XPCSRX, RX_INT_STAT, INTRX64B66BDECERR, reg_rx_int_stat);
    rx_int_stat->intrxfrmrnolocklos = RU_FIELD_GET(0, XPCSRX, RX_INT_STAT, INTRXFRMRNOLOCKLOS, reg_rx_int_stat);
    rx_int_stat->intrxfrmrrogue = RU_FIELD_GET(0, XPCSRX, RX_INT_STAT, INTRXFRMRROGUE, reg_rx_int_stat);
    rx_int_stat->int_regs_err = RU_FIELD_GET(0, XPCSRX, RX_INT_STAT, INT_REGS_ERR, reg_rx_int_stat);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xpcsrx_rx_int_msk_set(const xpcsrx_rx_int_msk *rx_int_msk)
{
    uint32_t reg_rx_int_msk=0;

#ifdef VALIDATE_PARMS
    if(!rx_int_msk)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((rx_int_msk->mskrxidledajit >= _1BITS_MAX_VAL_) ||
       (rx_int_msk->mskrxfrmrmisbrst >= _1BITS_MAX_VAL_) ||
       (rx_int_msk->mskrxidlesopeopgapbig >= _1BITS_MAX_VAL_) ||
       (rx_int_msk->mskrxidlefrcins >= _1BITS_MAX_VAL_) ||
       (rx_int_msk->mskrx64b66bminipgerr >= _1BITS_MAX_VAL_) ||
       (rx_int_msk->mskrxfecnquecntneq >= _1BITS_MAX_VAL_) ||
       (rx_int_msk->mskrxidlefifoundrun >= _1BITS_MAX_VAL_) ||
       (rx_int_msk->mskrxidlefifoovrrun >= _1BITS_MAX_VAL_) ||
       (rx_int_msk->mskrxfechighcor >= _1BITS_MAX_VAL_) ||
       (rx_int_msk->mskrxfecdecstoponerr >= _1BITS_MAX_VAL_) ||
       (rx_int_msk->mskrxfecdecpass >= _1BITS_MAX_VAL_) ||
       (rx_int_msk->mskrxstatfrmrhighber >= _1BITS_MAX_VAL_) ||
       (rx_int_msk->mskrxfrmrexitbysp >= _1BITS_MAX_VAL_) ||
       (rx_int_msk->mskrxfrmrbadshmax >= _1BITS_MAX_VAL_) ||
       (rx_int_msk->mskrxdscramburstseqout >= _1BITS_MAX_VAL_) ||
       (rx_int_msk->mskrxtestpsudolock >= _1BITS_MAX_VAL_) ||
       (rx_int_msk->mskrxtestpsudotype >= _1BITS_MAX_VAL_) ||
       (rx_int_msk->mskrxtestpsudoerr >= _1BITS_MAX_VAL_) ||
       (rx_int_msk->mskrxtestprbslock >= _1BITS_MAX_VAL_) ||
       (rx_int_msk->mskrxtestprbserr >= _1BITS_MAX_VAL_) ||
       (rx_int_msk->mskrxfecpsistdecfail >= _1BITS_MAX_VAL_) ||
       (rx_int_msk->mskrxframerbadsh >= _1BITS_MAX_VAL_) ||
       (rx_int_msk->mskrxframercwloss >= _1BITS_MAX_VAL_) ||
       (rx_int_msk->mskrxframercwlock >= _1BITS_MAX_VAL_) ||
       (rx_int_msk->mskrxfecdecfail >= _1BITS_MAX_VAL_) ||
       (rx_int_msk->mskrx64b66bdecerr >= _1BITS_MAX_VAL_) ||
       (rx_int_msk->mskrxfrmrnolocklos >= _1BITS_MAX_VAL_) ||
       (rx_int_msk->mskrxfrmrrogue >= _1BITS_MAX_VAL_) ||
       (rx_int_msk->msk_int_regs_err >= _1BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_rx_int_msk = RU_FIELD_SET(0, XPCSRX, RX_INT_MSK, MSKRXIDLEDAJIT, reg_rx_int_msk, rx_int_msk->mskrxidledajit);
    reg_rx_int_msk = RU_FIELD_SET(0, XPCSRX, RX_INT_MSK, MSKRXFRMRMISBRST, reg_rx_int_msk, rx_int_msk->mskrxfrmrmisbrst);
    reg_rx_int_msk = RU_FIELD_SET(0, XPCSRX, RX_INT_MSK, MSKRXIDLESOPEOPGAPBIG, reg_rx_int_msk, rx_int_msk->mskrxidlesopeopgapbig);
    reg_rx_int_msk = RU_FIELD_SET(0, XPCSRX, RX_INT_MSK, MSKRXIDLEFRCINS, reg_rx_int_msk, rx_int_msk->mskrxidlefrcins);
    reg_rx_int_msk = RU_FIELD_SET(0, XPCSRX, RX_INT_MSK, MSKRX64B66BMINIPGERR, reg_rx_int_msk, rx_int_msk->mskrx64b66bminipgerr);
    reg_rx_int_msk = RU_FIELD_SET(0, XPCSRX, RX_INT_MSK, MSKRXFECNQUECNTNEQ, reg_rx_int_msk, rx_int_msk->mskrxfecnquecntneq);
    reg_rx_int_msk = RU_FIELD_SET(0, XPCSRX, RX_INT_MSK, MSKRXIDLEFIFOUNDRUN, reg_rx_int_msk, rx_int_msk->mskrxidlefifoundrun);
    reg_rx_int_msk = RU_FIELD_SET(0, XPCSRX, RX_INT_MSK, MSKRXIDLEFIFOOVRRUN, reg_rx_int_msk, rx_int_msk->mskrxidlefifoovrrun);
    reg_rx_int_msk = RU_FIELD_SET(0, XPCSRX, RX_INT_MSK, MSKRXFECHIGHCOR, reg_rx_int_msk, rx_int_msk->mskrxfechighcor);
    reg_rx_int_msk = RU_FIELD_SET(0, XPCSRX, RX_INT_MSK, MSKRXFECDECSTOPONERR, reg_rx_int_msk, rx_int_msk->mskrxfecdecstoponerr);
    reg_rx_int_msk = RU_FIELD_SET(0, XPCSRX, RX_INT_MSK, MSKRXFECDECPASS, reg_rx_int_msk, rx_int_msk->mskrxfecdecpass);
    reg_rx_int_msk = RU_FIELD_SET(0, XPCSRX, RX_INT_MSK, MSKRXSTATFRMRHIGHBER, reg_rx_int_msk, rx_int_msk->mskrxstatfrmrhighber);
    reg_rx_int_msk = RU_FIELD_SET(0, XPCSRX, RX_INT_MSK, MSKRXFRMREXITBYSP, reg_rx_int_msk, rx_int_msk->mskrxfrmrexitbysp);
    reg_rx_int_msk = RU_FIELD_SET(0, XPCSRX, RX_INT_MSK, MSKRXFRMRBADSHMAX, reg_rx_int_msk, rx_int_msk->mskrxfrmrbadshmax);
    reg_rx_int_msk = RU_FIELD_SET(0, XPCSRX, RX_INT_MSK, MSKRXDSCRAMBURSTSEQOUT, reg_rx_int_msk, rx_int_msk->mskrxdscramburstseqout);
    reg_rx_int_msk = RU_FIELD_SET(0, XPCSRX, RX_INT_MSK, MSKRXTESTPSUDOLOCK, reg_rx_int_msk, rx_int_msk->mskrxtestpsudolock);
    reg_rx_int_msk = RU_FIELD_SET(0, XPCSRX, RX_INT_MSK, MSKRXTESTPSUDOTYPE, reg_rx_int_msk, rx_int_msk->mskrxtestpsudotype);
    reg_rx_int_msk = RU_FIELD_SET(0, XPCSRX, RX_INT_MSK, MSKRXTESTPSUDOERR, reg_rx_int_msk, rx_int_msk->mskrxtestpsudoerr);
    reg_rx_int_msk = RU_FIELD_SET(0, XPCSRX, RX_INT_MSK, MSKRXTESTPRBSLOCK, reg_rx_int_msk, rx_int_msk->mskrxtestprbslock);
    reg_rx_int_msk = RU_FIELD_SET(0, XPCSRX, RX_INT_MSK, MSKRXTESTPRBSERR, reg_rx_int_msk, rx_int_msk->mskrxtestprbserr);
    reg_rx_int_msk = RU_FIELD_SET(0, XPCSRX, RX_INT_MSK, MSKRXFECPSISTDECFAIL, reg_rx_int_msk, rx_int_msk->mskrxfecpsistdecfail);
    reg_rx_int_msk = RU_FIELD_SET(0, XPCSRX, RX_INT_MSK, MSKRXFRAMERBADSH, reg_rx_int_msk, rx_int_msk->mskrxframerbadsh);
    reg_rx_int_msk = RU_FIELD_SET(0, XPCSRX, RX_INT_MSK, MSKRXFRAMERCWLOSS, reg_rx_int_msk, rx_int_msk->mskrxframercwloss);
    reg_rx_int_msk = RU_FIELD_SET(0, XPCSRX, RX_INT_MSK, MSKRXFRAMERCWLOCK, reg_rx_int_msk, rx_int_msk->mskrxframercwlock);
    reg_rx_int_msk = RU_FIELD_SET(0, XPCSRX, RX_INT_MSK, MSKRXFECDECFAIL, reg_rx_int_msk, rx_int_msk->mskrxfecdecfail);
    reg_rx_int_msk = RU_FIELD_SET(0, XPCSRX, RX_INT_MSK, MSKRX64B66BDECERR, reg_rx_int_msk, rx_int_msk->mskrx64b66bdecerr);
    reg_rx_int_msk = RU_FIELD_SET(0, XPCSRX, RX_INT_MSK, MSKRXFRMRNOLOCKLOS, reg_rx_int_msk, rx_int_msk->mskrxfrmrnolocklos);
    reg_rx_int_msk = RU_FIELD_SET(0, XPCSRX, RX_INT_MSK, MSKRXFRMRROGUE, reg_rx_int_msk, rx_int_msk->mskrxfrmrrogue);
    reg_rx_int_msk = RU_FIELD_SET(0, XPCSRX, RX_INT_MSK, MSK_INT_REGS_ERR, reg_rx_int_msk, rx_int_msk->msk_int_regs_err);

    RU_REG_WRITE(0, XPCSRX, RX_INT_MSK, reg_rx_int_msk);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xpcsrx_rx_int_msk_get(xpcsrx_rx_int_msk *rx_int_msk)
{
    uint32_t reg_rx_int_msk=0;

#ifdef VALIDATE_PARMS
    if(!rx_int_msk)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, XPCSRX, RX_INT_MSK, reg_rx_int_msk);

    rx_int_msk->mskrxidledajit = RU_FIELD_GET(0, XPCSRX, RX_INT_MSK, MSKRXIDLEDAJIT, reg_rx_int_msk);
    rx_int_msk->mskrxfrmrmisbrst = RU_FIELD_GET(0, XPCSRX, RX_INT_MSK, MSKRXFRMRMISBRST, reg_rx_int_msk);
    rx_int_msk->mskrxidlesopeopgapbig = RU_FIELD_GET(0, XPCSRX, RX_INT_MSK, MSKRXIDLESOPEOPGAPBIG, reg_rx_int_msk);
    rx_int_msk->mskrxidlefrcins = RU_FIELD_GET(0, XPCSRX, RX_INT_MSK, MSKRXIDLEFRCINS, reg_rx_int_msk);
    rx_int_msk->mskrx64b66bminipgerr = RU_FIELD_GET(0, XPCSRX, RX_INT_MSK, MSKRX64B66BMINIPGERR, reg_rx_int_msk);
    rx_int_msk->mskrxfecnquecntneq = RU_FIELD_GET(0, XPCSRX, RX_INT_MSK, MSKRXFECNQUECNTNEQ, reg_rx_int_msk);
    rx_int_msk->mskrxidlefifoundrun = RU_FIELD_GET(0, XPCSRX, RX_INT_MSK, MSKRXIDLEFIFOUNDRUN, reg_rx_int_msk);
    rx_int_msk->mskrxidlefifoovrrun = RU_FIELD_GET(0, XPCSRX, RX_INT_MSK, MSKRXIDLEFIFOOVRRUN, reg_rx_int_msk);
    rx_int_msk->mskrxfechighcor = RU_FIELD_GET(0, XPCSRX, RX_INT_MSK, MSKRXFECHIGHCOR, reg_rx_int_msk);
    rx_int_msk->mskrxfecdecstoponerr = RU_FIELD_GET(0, XPCSRX, RX_INT_MSK, MSKRXFECDECSTOPONERR, reg_rx_int_msk);
    rx_int_msk->mskrxfecdecpass = RU_FIELD_GET(0, XPCSRX, RX_INT_MSK, MSKRXFECDECPASS, reg_rx_int_msk);
    rx_int_msk->mskrxstatfrmrhighber = RU_FIELD_GET(0, XPCSRX, RX_INT_MSK, MSKRXSTATFRMRHIGHBER, reg_rx_int_msk);
    rx_int_msk->mskrxfrmrexitbysp = RU_FIELD_GET(0, XPCSRX, RX_INT_MSK, MSKRXFRMREXITBYSP, reg_rx_int_msk);
    rx_int_msk->mskrxfrmrbadshmax = RU_FIELD_GET(0, XPCSRX, RX_INT_MSK, MSKRXFRMRBADSHMAX, reg_rx_int_msk);
    rx_int_msk->mskrxdscramburstseqout = RU_FIELD_GET(0, XPCSRX, RX_INT_MSK, MSKRXDSCRAMBURSTSEQOUT, reg_rx_int_msk);
    rx_int_msk->mskrxtestpsudolock = RU_FIELD_GET(0, XPCSRX, RX_INT_MSK, MSKRXTESTPSUDOLOCK, reg_rx_int_msk);
    rx_int_msk->mskrxtestpsudotype = RU_FIELD_GET(0, XPCSRX, RX_INT_MSK, MSKRXTESTPSUDOTYPE, reg_rx_int_msk);
    rx_int_msk->mskrxtestpsudoerr = RU_FIELD_GET(0, XPCSRX, RX_INT_MSK, MSKRXTESTPSUDOERR, reg_rx_int_msk);
    rx_int_msk->mskrxtestprbslock = RU_FIELD_GET(0, XPCSRX, RX_INT_MSK, MSKRXTESTPRBSLOCK, reg_rx_int_msk);
    rx_int_msk->mskrxtestprbserr = RU_FIELD_GET(0, XPCSRX, RX_INT_MSK, MSKRXTESTPRBSERR, reg_rx_int_msk);
    rx_int_msk->mskrxfecpsistdecfail = RU_FIELD_GET(0, XPCSRX, RX_INT_MSK, MSKRXFECPSISTDECFAIL, reg_rx_int_msk);
    rx_int_msk->mskrxframerbadsh = RU_FIELD_GET(0, XPCSRX, RX_INT_MSK, MSKRXFRAMERBADSH, reg_rx_int_msk);
    rx_int_msk->mskrxframercwloss = RU_FIELD_GET(0, XPCSRX, RX_INT_MSK, MSKRXFRAMERCWLOSS, reg_rx_int_msk);
    rx_int_msk->mskrxframercwlock = RU_FIELD_GET(0, XPCSRX, RX_INT_MSK, MSKRXFRAMERCWLOCK, reg_rx_int_msk);
    rx_int_msk->mskrxfecdecfail = RU_FIELD_GET(0, XPCSRX, RX_INT_MSK, MSKRXFECDECFAIL, reg_rx_int_msk);
    rx_int_msk->mskrx64b66bdecerr = RU_FIELD_GET(0, XPCSRX, RX_INT_MSK, MSKRX64B66BDECERR, reg_rx_int_msk);
    rx_int_msk->mskrxfrmrnolocklos = RU_FIELD_GET(0, XPCSRX, RX_INT_MSK, MSKRXFRMRNOLOCKLOS, reg_rx_int_msk);
    rx_int_msk->mskrxfrmrrogue = RU_FIELD_GET(0, XPCSRX, RX_INT_MSK, MSKRXFRMRROGUE, reg_rx_int_msk);
    rx_int_msk->msk_int_regs_err = RU_FIELD_GET(0, XPCSRX, RX_INT_MSK, MSK_INT_REGS_ERR, reg_rx_int_msk);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xpcsrx_rx_framer_ctl_set(const xpcsrx_rx_framer_ctl *rx_framer_ctl)
{
    uint32_t reg_rx_framer_ctl=0;

#ifdef VALIDATE_PARMS
    if(!rx_framer_ctl)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((rx_framer_ctl->cfgxpcsrxfrmrfrcearlyalign >= _1BITS_MAX_VAL_) ||
       (rx_framer_ctl->cfgxpcsrxfrmrmodea >= _1BITS_MAX_VAL_) ||
       (rx_framer_ctl->cfgxpcsrxfrmroverlapbdebdzero >= _1BITS_MAX_VAL_) ||
       (rx_framer_ctl->cfgxpcsrxfrmroverlapgnten >= _1BITS_MAX_VAL_) ||
       (rx_framer_ctl->cfgxpcsrxframeburstoldalign >= _1BITS_MAX_VAL_) ||
       (rx_framer_ctl->cfgxpcsrxfrmrmisbrsttype >= _1BITS_MAX_VAL_) ||
       (rx_framer_ctl->cfgxpcsrxfrmrebdvlden >= _1BITS_MAX_VAL_) ||
       (rx_framer_ctl->cfgxpcsrxfrmrbdcnten >= _1BITS_MAX_VAL_) ||
       (rx_framer_ctl->cfgxpcsrxfrmrburstbadshen >= _1BITS_MAX_VAL_) ||
       (rx_framer_ctl->cfgxpcsrxfrmrspulken >= _1BITS_MAX_VAL_) ||
       (rx_framer_ctl->cfgxpcsrxframeburst >= _1BITS_MAX_VAL_) ||
       (rx_framer_ctl->cfgxpcsrxfrmren >= _1BITS_MAX_VAL_) ||
       (rx_framer_ctl->cfgxpcsrxfrmrblkfecfail >= _1BITS_MAX_VAL_) ||
       (rx_framer_ctl->cfgxpcsrxframefec >= _1BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_rx_framer_ctl = RU_FIELD_SET(0, XPCSRX, RX_FRAMER_CTL, CFGXPCSRXFRMRFRCEARLYALIGN, reg_rx_framer_ctl, rx_framer_ctl->cfgxpcsrxfrmrfrcearlyalign);
    reg_rx_framer_ctl = RU_FIELD_SET(0, XPCSRX, RX_FRAMER_CTL, CFGXPCSRXFRMRMODEA, reg_rx_framer_ctl, rx_framer_ctl->cfgxpcsrxfrmrmodea);
    reg_rx_framer_ctl = RU_FIELD_SET(0, XPCSRX, RX_FRAMER_CTL, CFGXPCSRXFRMROVERLAPBDEBDZERO, reg_rx_framer_ctl, rx_framer_ctl->cfgxpcsrxfrmroverlapbdebdzero);
    reg_rx_framer_ctl = RU_FIELD_SET(0, XPCSRX, RX_FRAMER_CTL, CFGXPCSRXFRMROVERLAPGNTEN, reg_rx_framer_ctl, rx_framer_ctl->cfgxpcsrxfrmroverlapgnten);
    reg_rx_framer_ctl = RU_FIELD_SET(0, XPCSRX, RX_FRAMER_CTL, CFGXPCSRXFRAMEBURSTOLDALIGN, reg_rx_framer_ctl, rx_framer_ctl->cfgxpcsrxframeburstoldalign);
    reg_rx_framer_ctl = RU_FIELD_SET(0, XPCSRX, RX_FRAMER_CTL, CFGXPCSRXFRMRMISBRSTTYPE, reg_rx_framer_ctl, rx_framer_ctl->cfgxpcsrxfrmrmisbrsttype);
    reg_rx_framer_ctl = RU_FIELD_SET(0, XPCSRX, RX_FRAMER_CTL, CFGXPCSRXFRMREBDVLDEN, reg_rx_framer_ctl, rx_framer_ctl->cfgxpcsrxfrmrebdvlden);
    reg_rx_framer_ctl = RU_FIELD_SET(0, XPCSRX, RX_FRAMER_CTL, CFGXPCSRXFRMRBDCNTEN, reg_rx_framer_ctl, rx_framer_ctl->cfgxpcsrxfrmrbdcnten);
    reg_rx_framer_ctl = RU_FIELD_SET(0, XPCSRX, RX_FRAMER_CTL, CFGXPCSRXFRMRBURSTBADSHEN, reg_rx_framer_ctl, rx_framer_ctl->cfgxpcsrxfrmrburstbadshen);
    reg_rx_framer_ctl = RU_FIELD_SET(0, XPCSRX, RX_FRAMER_CTL, CFGXPCSRXFRMRSPULKEN, reg_rx_framer_ctl, rx_framer_ctl->cfgxpcsrxfrmrspulken);
    reg_rx_framer_ctl = RU_FIELD_SET(0, XPCSRX, RX_FRAMER_CTL, CFGXPCSRXFRAMEBURST, reg_rx_framer_ctl, rx_framer_ctl->cfgxpcsrxframeburst);
    reg_rx_framer_ctl = RU_FIELD_SET(0, XPCSRX, RX_FRAMER_CTL, CFGXPCSRXFRMREN, reg_rx_framer_ctl, rx_framer_ctl->cfgxpcsrxfrmren);
    reg_rx_framer_ctl = RU_FIELD_SET(0, XPCSRX, RX_FRAMER_CTL, CFGXPCSRXFRMRBLKFECFAIL, reg_rx_framer_ctl, rx_framer_ctl->cfgxpcsrxfrmrblkfecfail);
    reg_rx_framer_ctl = RU_FIELD_SET(0, XPCSRX, RX_FRAMER_CTL, CFGXPCSRXFRAMEFEC, reg_rx_framer_ctl, rx_framer_ctl->cfgxpcsrxframefec);

    RU_REG_WRITE(0, XPCSRX, RX_FRAMER_CTL, reg_rx_framer_ctl);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xpcsrx_rx_framer_ctl_get(xpcsrx_rx_framer_ctl *rx_framer_ctl)
{
    uint32_t reg_rx_framer_ctl=0;

#ifdef VALIDATE_PARMS
    if(!rx_framer_ctl)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, XPCSRX, RX_FRAMER_CTL, reg_rx_framer_ctl);

    rx_framer_ctl->cfgxpcsrxfrmrfrcearlyalign = RU_FIELD_GET(0, XPCSRX, RX_FRAMER_CTL, CFGXPCSRXFRMRFRCEARLYALIGN, reg_rx_framer_ctl);
    rx_framer_ctl->cfgxpcsrxfrmrmodea = RU_FIELD_GET(0, XPCSRX, RX_FRAMER_CTL, CFGXPCSRXFRMRMODEA, reg_rx_framer_ctl);
    rx_framer_ctl->cfgxpcsrxfrmroverlapbdebdzero = RU_FIELD_GET(0, XPCSRX, RX_FRAMER_CTL, CFGXPCSRXFRMROVERLAPBDEBDZERO, reg_rx_framer_ctl);
    rx_framer_ctl->cfgxpcsrxfrmroverlapgnten = RU_FIELD_GET(0, XPCSRX, RX_FRAMER_CTL, CFGXPCSRXFRMROVERLAPGNTEN, reg_rx_framer_ctl);
    rx_framer_ctl->cfgxpcsrxframeburstoldalign = RU_FIELD_GET(0, XPCSRX, RX_FRAMER_CTL, CFGXPCSRXFRAMEBURSTOLDALIGN, reg_rx_framer_ctl);
    rx_framer_ctl->cfgxpcsrxfrmrmisbrsttype = RU_FIELD_GET(0, XPCSRX, RX_FRAMER_CTL, CFGXPCSRXFRMRMISBRSTTYPE, reg_rx_framer_ctl);
    rx_framer_ctl->cfgxpcsrxfrmrebdvlden = RU_FIELD_GET(0, XPCSRX, RX_FRAMER_CTL, CFGXPCSRXFRMREBDVLDEN, reg_rx_framer_ctl);
    rx_framer_ctl->cfgxpcsrxfrmrbdcnten = RU_FIELD_GET(0, XPCSRX, RX_FRAMER_CTL, CFGXPCSRXFRMRBDCNTEN, reg_rx_framer_ctl);
    rx_framer_ctl->cfgxpcsrxfrmrburstbadshen = RU_FIELD_GET(0, XPCSRX, RX_FRAMER_CTL, CFGXPCSRXFRMRBURSTBADSHEN, reg_rx_framer_ctl);
    rx_framer_ctl->cfgxpcsrxfrmrspulken = RU_FIELD_GET(0, XPCSRX, RX_FRAMER_CTL, CFGXPCSRXFRMRSPULKEN, reg_rx_framer_ctl);
    rx_framer_ctl->cfgxpcsrxframeburst = RU_FIELD_GET(0, XPCSRX, RX_FRAMER_CTL, CFGXPCSRXFRAMEBURST, reg_rx_framer_ctl);
    rx_framer_ctl->cfgxpcsrxfrmren = RU_FIELD_GET(0, XPCSRX, RX_FRAMER_CTL, CFGXPCSRXFRMREN, reg_rx_framer_ctl);
    rx_framer_ctl->cfgxpcsrxfrmrblkfecfail = RU_FIELD_GET(0, XPCSRX, RX_FRAMER_CTL, CFGXPCSRXFRMRBLKFECFAIL, reg_rx_framer_ctl);
    rx_framer_ctl->cfgxpcsrxframefec = RU_FIELD_GET(0, XPCSRX, RX_FRAMER_CTL, CFGXPCSRXFRAMEFEC, reg_rx_framer_ctl);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xpcsrx_rx_fec_ctl_set(const xpcsrx_rx_fec_ctl *rx_fec_ctl)
{
    uint32_t reg_rx_fec_ctl=0;

#ifdef VALIDATE_PARMS
    if(!rx_fec_ctl)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((rx_fec_ctl->cfgxpcsrxfecstoponerr >= _1BITS_MAX_VAL_) ||
       (rx_fec_ctl->cfgxpcsrxfeccntnquecw >= _1BITS_MAX_VAL_) ||
       (rx_fec_ctl->cfgxpcsrxfecnquerst >= _1BITS_MAX_VAL_) ||
       (rx_fec_ctl->cfgxpcsrxfeconezeromode >= _1BITS_MAX_VAL_) ||
       (rx_fec_ctl->cfgxpcsrxfecblkcorrect >= _1BITS_MAX_VAL_) ||
       (rx_fec_ctl->cfgxpcsrxfecnquetestpat >= _1BITS_MAX_VAL_) ||
       (rx_fec_ctl->cfgxpcsrxfecfailblksh0 >= _1BITS_MAX_VAL_) ||
       (rx_fec_ctl->cfgxpcsrxfecstrip >= _1BITS_MAX_VAL_) ||
       (rx_fec_ctl->cfgxpcsrxfecbypas >= _1BITS_MAX_VAL_) ||
       (rx_fec_ctl->cfgxpcsrxfecidleins >= _1BITS_MAX_VAL_) ||
       (rx_fec_ctl->cfgxpcsrxfecen >= _1BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_rx_fec_ctl = RU_FIELD_SET(0, XPCSRX, RX_FEC_CTL, CFGXPCSRXFECSTOPONERR, reg_rx_fec_ctl, rx_fec_ctl->cfgxpcsrxfecstoponerr);
    reg_rx_fec_ctl = RU_FIELD_SET(0, XPCSRX, RX_FEC_CTL, CFGXPCSRXFECCNTNQUECW, reg_rx_fec_ctl, rx_fec_ctl->cfgxpcsrxfeccntnquecw);
    reg_rx_fec_ctl = RU_FIELD_SET(0, XPCSRX, RX_FEC_CTL, CFGXPCSRXFECNQUERST, reg_rx_fec_ctl, rx_fec_ctl->cfgxpcsrxfecnquerst);
    reg_rx_fec_ctl = RU_FIELD_SET(0, XPCSRX, RX_FEC_CTL, CFGXPCSRXFECONEZEROMODE, reg_rx_fec_ctl, rx_fec_ctl->cfgxpcsrxfeconezeromode);
    reg_rx_fec_ctl = RU_FIELD_SET(0, XPCSRX, RX_FEC_CTL, CFGXPCSRXFECBLKCORRECT, reg_rx_fec_ctl, rx_fec_ctl->cfgxpcsrxfecblkcorrect);
    reg_rx_fec_ctl = RU_FIELD_SET(0, XPCSRX, RX_FEC_CTL, CFGXPCSRXFECNQUETESTPAT, reg_rx_fec_ctl, rx_fec_ctl->cfgxpcsrxfecnquetestpat);
    reg_rx_fec_ctl = RU_FIELD_SET(0, XPCSRX, RX_FEC_CTL, CFGXPCSRXFECFAILBLKSH0, reg_rx_fec_ctl, rx_fec_ctl->cfgxpcsrxfecfailblksh0);
    reg_rx_fec_ctl = RU_FIELD_SET(0, XPCSRX, RX_FEC_CTL, CFGXPCSRXFECSTRIP, reg_rx_fec_ctl, rx_fec_ctl->cfgxpcsrxfecstrip);
    reg_rx_fec_ctl = RU_FIELD_SET(0, XPCSRX, RX_FEC_CTL, CFGXPCSRXFECBYPAS, reg_rx_fec_ctl, rx_fec_ctl->cfgxpcsrxfecbypas);
    reg_rx_fec_ctl = RU_FIELD_SET(0, XPCSRX, RX_FEC_CTL, CFGXPCSRXFECIDLEINS, reg_rx_fec_ctl, rx_fec_ctl->cfgxpcsrxfecidleins);
    reg_rx_fec_ctl = RU_FIELD_SET(0, XPCSRX, RX_FEC_CTL, CFGXPCSRXFECEN, reg_rx_fec_ctl, rx_fec_ctl->cfgxpcsrxfecen);

    RU_REG_WRITE(0, XPCSRX, RX_FEC_CTL, reg_rx_fec_ctl);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xpcsrx_rx_fec_ctl_get(xpcsrx_rx_fec_ctl *rx_fec_ctl)
{
    uint32_t reg_rx_fec_ctl=0;

#ifdef VALIDATE_PARMS
    if(!rx_fec_ctl)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, XPCSRX, RX_FEC_CTL, reg_rx_fec_ctl);

    rx_fec_ctl->cfgxpcsrxfecstoponerr = RU_FIELD_GET(0, XPCSRX, RX_FEC_CTL, CFGXPCSRXFECSTOPONERR, reg_rx_fec_ctl);
    rx_fec_ctl->cfgxpcsrxfeccntnquecw = RU_FIELD_GET(0, XPCSRX, RX_FEC_CTL, CFGXPCSRXFECCNTNQUECW, reg_rx_fec_ctl);
    rx_fec_ctl->cfgxpcsrxfecnquerst = RU_FIELD_GET(0, XPCSRX, RX_FEC_CTL, CFGXPCSRXFECNQUERST, reg_rx_fec_ctl);
    rx_fec_ctl->cfgxpcsrxfeconezeromode = RU_FIELD_GET(0, XPCSRX, RX_FEC_CTL, CFGXPCSRXFECONEZEROMODE, reg_rx_fec_ctl);
    rx_fec_ctl->cfgxpcsrxfecblkcorrect = RU_FIELD_GET(0, XPCSRX, RX_FEC_CTL, CFGXPCSRXFECBLKCORRECT, reg_rx_fec_ctl);
    rx_fec_ctl->cfgxpcsrxfecnquetestpat = RU_FIELD_GET(0, XPCSRX, RX_FEC_CTL, CFGXPCSRXFECNQUETESTPAT, reg_rx_fec_ctl);
    rx_fec_ctl->cfgxpcsrxfecfailblksh0 = RU_FIELD_GET(0, XPCSRX, RX_FEC_CTL, CFGXPCSRXFECFAILBLKSH0, reg_rx_fec_ctl);
    rx_fec_ctl->cfgxpcsrxfecstrip = RU_FIELD_GET(0, XPCSRX, RX_FEC_CTL, CFGXPCSRXFECSTRIP, reg_rx_fec_ctl);
    rx_fec_ctl->cfgxpcsrxfecbypas = RU_FIELD_GET(0, XPCSRX, RX_FEC_CTL, CFGXPCSRXFECBYPAS, reg_rx_fec_ctl);
    rx_fec_ctl->cfgxpcsrxfecidleins = RU_FIELD_GET(0, XPCSRX, RX_FEC_CTL, CFGXPCSRXFECIDLEINS, reg_rx_fec_ctl);
    rx_fec_ctl->cfgxpcsrxfecen = RU_FIELD_GET(0, XPCSRX, RX_FEC_CTL, CFGXPCSRXFECEN, reg_rx_fec_ctl);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xpcsrx_rx_dscram_ctl_set(bdmf_boolean cfgxpcsrxdscrambypas)
{
    uint32_t reg_rx_dscram_ctl=0;

#ifdef VALIDATE_PARMS
    if((cfgxpcsrxdscrambypas >= _1BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_rx_dscram_ctl = RU_FIELD_SET(0, XPCSRX, RX_DSCRAM_CTL, CFGXPCSRXDSCRAMBYPAS, reg_rx_dscram_ctl, cfgxpcsrxdscrambypas);

    RU_REG_WRITE(0, XPCSRX, RX_DSCRAM_CTL, reg_rx_dscram_ctl);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xpcsrx_rx_dscram_ctl_get(bdmf_boolean *cfgxpcsrxdscrambypas)
{
    uint32_t reg_rx_dscram_ctl=0;

#ifdef VALIDATE_PARMS
    if(!cfgxpcsrxdscrambypas)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, XPCSRX, RX_DSCRAM_CTL, reg_rx_dscram_ctl);

    *cfgxpcsrxdscrambypas = RU_FIELD_GET(0, XPCSRX, RX_DSCRAM_CTL, CFGXPCSRXDSCRAMBYPAS, reg_rx_dscram_ctl);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xpcsrx_rx_64b66b_ctl_set(const xpcsrx_rx_64b66b_ctl *rx_64b66b_ctl)
{
    uint32_t reg_rx_64b66b_ctl=0;

#ifdef VALIDATE_PARMS
    if(!rx_64b66b_ctl)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((rx_64b66b_ctl->cfgxpcsrx64b66bsmask1 >= _2BITS_MAX_VAL_) ||
       (rx_64b66b_ctl->cfgxpcsrx64b66bsmask0 >= _2BITS_MAX_VAL_) ||
       (rx_64b66b_ctl->cfgxpcsrx64b66btdlay >= _2BITS_MAX_VAL_) ||
       (rx_64b66b_ctl->cfgxpcsrx64b66bdecbypas >= _1BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_rx_64b66b_ctl = RU_FIELD_SET(0, XPCSRX, RX_64B66B_CTL, CFGXPCSRX64B66BTMASK1, reg_rx_64b66b_ctl, rx_64b66b_ctl->cfgxpcsrx64b66btmask1);
    reg_rx_64b66b_ctl = RU_FIELD_SET(0, XPCSRX, RX_64B66B_CTL, CFGXPCSRX64B66BTMASK0, reg_rx_64b66b_ctl, rx_64b66b_ctl->cfgxpcsrx64b66btmask0);
    reg_rx_64b66b_ctl = RU_FIELD_SET(0, XPCSRX, RX_64B66B_CTL, CFGXPCSRX64B66BSMASK1, reg_rx_64b66b_ctl, rx_64b66b_ctl->cfgxpcsrx64b66bsmask1);
    reg_rx_64b66b_ctl = RU_FIELD_SET(0, XPCSRX, RX_64B66B_CTL, CFGXPCSRX64B66BSMASK0, reg_rx_64b66b_ctl, rx_64b66b_ctl->cfgxpcsrx64b66bsmask0);
    reg_rx_64b66b_ctl = RU_FIELD_SET(0, XPCSRX, RX_64B66B_CTL, CFGXPCSRX64B66BTDLAY, reg_rx_64b66b_ctl, rx_64b66b_ctl->cfgxpcsrx64b66btdlay);
    reg_rx_64b66b_ctl = RU_FIELD_SET(0, XPCSRX, RX_64B66B_CTL, CFGXPCSRX64B66BDECBYPAS, reg_rx_64b66b_ctl, rx_64b66b_ctl->cfgxpcsrx64b66bdecbypas);

    RU_REG_WRITE(0, XPCSRX, RX_64B66B_CTL, reg_rx_64b66b_ctl);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xpcsrx_rx_64b66b_ctl_get(xpcsrx_rx_64b66b_ctl *rx_64b66b_ctl)
{
    uint32_t reg_rx_64b66b_ctl=0;

#ifdef VALIDATE_PARMS
    if(!rx_64b66b_ctl)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, XPCSRX, RX_64B66B_CTL, reg_rx_64b66b_ctl);

    rx_64b66b_ctl->cfgxpcsrx64b66btmask1 = RU_FIELD_GET(0, XPCSRX, RX_64B66B_CTL, CFGXPCSRX64B66BTMASK1, reg_rx_64b66b_ctl);
    rx_64b66b_ctl->cfgxpcsrx64b66btmask0 = RU_FIELD_GET(0, XPCSRX, RX_64B66B_CTL, CFGXPCSRX64B66BTMASK0, reg_rx_64b66b_ctl);
    rx_64b66b_ctl->cfgxpcsrx64b66bsmask1 = RU_FIELD_GET(0, XPCSRX, RX_64B66B_CTL, CFGXPCSRX64B66BSMASK1, reg_rx_64b66b_ctl);
    rx_64b66b_ctl->cfgxpcsrx64b66bsmask0 = RU_FIELD_GET(0, XPCSRX, RX_64B66B_CTL, CFGXPCSRX64B66BSMASK0, reg_rx_64b66b_ctl);
    rx_64b66b_ctl->cfgxpcsrx64b66btdlay = RU_FIELD_GET(0, XPCSRX, RX_64B66B_CTL, CFGXPCSRX64B66BTDLAY, reg_rx_64b66b_ctl);
    rx_64b66b_ctl->cfgxpcsrx64b66bdecbypas = RU_FIELD_GET(0, XPCSRX, RX_64B66B_CTL, CFGXPCSRX64B66BDECBYPAS, reg_rx_64b66b_ctl);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xpcsrx_rx_test_ctl_set(bdmf_boolean cfgxpcsrxtestprbsdeten, bdmf_boolean cfgxpcsrxtestpsudodeten)
{
    uint32_t reg_rx_test_ctl=0;

#ifdef VALIDATE_PARMS
    if((cfgxpcsrxtestprbsdeten >= _1BITS_MAX_VAL_) ||
       (cfgxpcsrxtestpsudodeten >= _1BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_rx_test_ctl = RU_FIELD_SET(0, XPCSRX, RX_TEST_CTL, CFGXPCSRXTESTPRBSDETEN, reg_rx_test_ctl, cfgxpcsrxtestprbsdeten);
    reg_rx_test_ctl = RU_FIELD_SET(0, XPCSRX, RX_TEST_CTL, CFGXPCSRXTESTPSUDODETEN, reg_rx_test_ctl, cfgxpcsrxtestpsudodeten);

    RU_REG_WRITE(0, XPCSRX, RX_TEST_CTL, reg_rx_test_ctl);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xpcsrx_rx_test_ctl_get(bdmf_boolean *cfgxpcsrxtestprbsdeten, bdmf_boolean *cfgxpcsrxtestpsudodeten)
{
    uint32_t reg_rx_test_ctl=0;

#ifdef VALIDATE_PARMS
    if(!cfgxpcsrxtestprbsdeten || !cfgxpcsrxtestpsudodeten)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, XPCSRX, RX_TEST_CTL, reg_rx_test_ctl);

    *cfgxpcsrxtestprbsdeten = RU_FIELD_GET(0, XPCSRX, RX_TEST_CTL, CFGXPCSRXTESTPRBSDETEN, reg_rx_test_ctl);
    *cfgxpcsrxtestpsudodeten = RU_FIELD_GET(0, XPCSRX, RX_TEST_CTL, CFGXPCSRXTESTPSUDODETEN, reg_rx_test_ctl);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xpcsrx_rx_idle_rd_timer_dly_set(uint8_t cfgxpcsrxidlerddelaytimermax)
{
    uint32_t reg_rx_idle_rd_timer_dly=0;

#ifdef VALIDATE_PARMS
#endif

    reg_rx_idle_rd_timer_dly = RU_FIELD_SET(0, XPCSRX, RX_IDLE_RD_TIMER_DLY, CFGXPCSRXIDLERDDELAYTIMERMAX, reg_rx_idle_rd_timer_dly, cfgxpcsrxidlerddelaytimermax);

    RU_REG_WRITE(0, XPCSRX, RX_IDLE_RD_TIMER_DLY, reg_rx_idle_rd_timer_dly);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xpcsrx_rx_idle_rd_timer_dly_get(uint8_t *cfgxpcsrxidlerddelaytimermax)
{
    uint32_t reg_rx_idle_rd_timer_dly=0;

#ifdef VALIDATE_PARMS
    if(!cfgxpcsrxidlerddelaytimermax)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, XPCSRX, RX_IDLE_RD_TIMER_DLY, reg_rx_idle_rd_timer_dly);

    *cfgxpcsrxidlerddelaytimermax = RU_FIELD_GET(0, XPCSRX, RX_IDLE_RD_TIMER_DLY, CFGXPCSRXIDLERDDELAYTIMERMAX, reg_rx_idle_rd_timer_dly);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xpcsrx_rx_idle_gap_siz_max_set(uint16_t cfgxpcsrxidleovrsizmax, uint16_t cfgxpcsrxidlesopeopgap)
{
    uint32_t reg_rx_idle_gap_siz_max=0;

#ifdef VALIDATE_PARMS
    if((cfgxpcsrxidleovrsizmax >= _11BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_rx_idle_gap_siz_max = RU_FIELD_SET(0, XPCSRX, RX_IDLE_GAP_SIZ_MAX, CFGXPCSRXIDLEOVRSIZMAX, reg_rx_idle_gap_siz_max, cfgxpcsrxidleovrsizmax);
    reg_rx_idle_gap_siz_max = RU_FIELD_SET(0, XPCSRX, RX_IDLE_GAP_SIZ_MAX, CFGXPCSRXIDLESOPEOPGAP, reg_rx_idle_gap_siz_max, cfgxpcsrxidlesopeopgap);

    RU_REG_WRITE(0, XPCSRX, RX_IDLE_GAP_SIZ_MAX, reg_rx_idle_gap_siz_max);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xpcsrx_rx_idle_gap_siz_max_get(uint16_t *cfgxpcsrxidleovrsizmax, uint16_t *cfgxpcsrxidlesopeopgap)
{
    uint32_t reg_rx_idle_gap_siz_max=0;

#ifdef VALIDATE_PARMS
    if(!cfgxpcsrxidleovrsizmax || !cfgxpcsrxidlesopeopgap)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, XPCSRX, RX_IDLE_GAP_SIZ_MAX, reg_rx_idle_gap_siz_max);

    *cfgxpcsrxidleovrsizmax = RU_FIELD_GET(0, XPCSRX, RX_IDLE_GAP_SIZ_MAX, CFGXPCSRXIDLEOVRSIZMAX, reg_rx_idle_gap_siz_max);
    *cfgxpcsrxidlesopeopgap = RU_FIELD_GET(0, XPCSRX, RX_IDLE_GAP_SIZ_MAX, CFGXPCSRXIDLESOPEOPGAP, reg_rx_idle_gap_siz_max);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xpcsrx_rx_framer_lk_max_set(uint16_t cfgxpcsrxfrmrcwlktimermax)
{
    uint32_t reg_rx_framer_lk_max=0;

#ifdef VALIDATE_PARMS
    if((cfgxpcsrxfrmrcwlktimermax >= _9BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_rx_framer_lk_max = RU_FIELD_SET(0, XPCSRX, RX_FRAMER_LK_MAX, CFGXPCSRXFRMRCWLKTIMERMAX, reg_rx_framer_lk_max, cfgxpcsrxfrmrcwlktimermax);

    RU_REG_WRITE(0, XPCSRX, RX_FRAMER_LK_MAX, reg_rx_framer_lk_max);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xpcsrx_rx_framer_lk_max_get(uint16_t *cfgxpcsrxfrmrcwlktimermax)
{
    uint32_t reg_rx_framer_lk_max=0;

#ifdef VALIDATE_PARMS
    if(!cfgxpcsrxfrmrcwlktimermax)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, XPCSRX, RX_FRAMER_LK_MAX, reg_rx_framer_lk_max);

    *cfgxpcsrxfrmrcwlktimermax = RU_FIELD_GET(0, XPCSRX, RX_FRAMER_LK_MAX, CFGXPCSRXFRMRCWLKTIMERMAX, reg_rx_framer_lk_max);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xpcsrx_rx_framer_unlk_max_set(uint16_t cfgxpcsrxfrmrcwunlktimermax)
{
    uint32_t reg_rx_framer_unlk_max=0;

#ifdef VALIDATE_PARMS
    if((cfgxpcsrxfrmrcwunlktimermax >= _9BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_rx_framer_unlk_max = RU_FIELD_SET(0, XPCSRX, RX_FRAMER_UNLK_MAX, CFGXPCSRXFRMRCWUNLKTIMERMAX, reg_rx_framer_unlk_max, cfgxpcsrxfrmrcwunlktimermax);

    RU_REG_WRITE(0, XPCSRX, RX_FRAMER_UNLK_MAX, reg_rx_framer_unlk_max);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xpcsrx_rx_framer_unlk_max_get(uint16_t *cfgxpcsrxfrmrcwunlktimermax)
{
    uint32_t reg_rx_framer_unlk_max=0;

#ifdef VALIDATE_PARMS
    if(!cfgxpcsrxfrmrcwunlktimermax)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, XPCSRX, RX_FRAMER_UNLK_MAX, reg_rx_framer_unlk_max);

    *cfgxpcsrxfrmrcwunlktimermax = RU_FIELD_GET(0, XPCSRX, RX_FRAMER_UNLK_MAX, CFGXPCSRXFRMRCWUNLKTIMERMAX, reg_rx_framer_unlk_max);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xpcsrx_rx_framer_bd_sh_set(uint8_t cfgxpcsrxoltbdsh)
{
    uint32_t reg_rx_framer_bd_sh=0;

#ifdef VALIDATE_PARMS
    if((cfgxpcsrxoltbdsh >= _2BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_rx_framer_bd_sh = RU_FIELD_SET(0, XPCSRX, RX_FRAMER_BD_SH, CFGXPCSRXOLTBDSH, reg_rx_framer_bd_sh, cfgxpcsrxoltbdsh);

    RU_REG_WRITE(0, XPCSRX, RX_FRAMER_BD_SH, reg_rx_framer_bd_sh);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xpcsrx_rx_framer_bd_sh_get(uint8_t *cfgxpcsrxoltbdsh)
{
    uint32_t reg_rx_framer_bd_sh=0;

#ifdef VALIDATE_PARMS
    if(!cfgxpcsrxoltbdsh)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, XPCSRX, RX_FRAMER_BD_SH, reg_rx_framer_bd_sh);

    *cfgxpcsrxoltbdsh = RU_FIELD_GET(0, XPCSRX, RX_FRAMER_BD_SH, CFGXPCSRXOLTBDSH, reg_rx_framer_bd_sh);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xpcsrx_rx_framer_bd_lo_set(uint32_t cfgxpcsrxoltbdlo)
{
    uint32_t reg_rx_framer_bd_lo=0;

#ifdef VALIDATE_PARMS
#endif

    reg_rx_framer_bd_lo = RU_FIELD_SET(0, XPCSRX, RX_FRAMER_BD_LO, CFGXPCSRXOLTBDLO, reg_rx_framer_bd_lo, cfgxpcsrxoltbdlo);

    RU_REG_WRITE(0, XPCSRX, RX_FRAMER_BD_LO, reg_rx_framer_bd_lo);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xpcsrx_rx_framer_bd_lo_get(uint32_t *cfgxpcsrxoltbdlo)
{
    uint32_t reg_rx_framer_bd_lo=0;

#ifdef VALIDATE_PARMS
    if(!cfgxpcsrxoltbdlo)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, XPCSRX, RX_FRAMER_BD_LO, reg_rx_framer_bd_lo);

    *cfgxpcsrxoltbdlo = RU_FIELD_GET(0, XPCSRX, RX_FRAMER_BD_LO, CFGXPCSRXOLTBDLO, reg_rx_framer_bd_lo);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xpcsrx_rx_framer_bd_hi_set(uint32_t cfgxpcsrxoltbdhi)
{
    uint32_t reg_rx_framer_bd_hi=0;

#ifdef VALIDATE_PARMS
#endif

    reg_rx_framer_bd_hi = RU_FIELD_SET(0, XPCSRX, RX_FRAMER_BD_HI, CFGXPCSRXOLTBDHI, reg_rx_framer_bd_hi, cfgxpcsrxoltbdhi);

    RU_REG_WRITE(0, XPCSRX, RX_FRAMER_BD_HI, reg_rx_framer_bd_hi);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xpcsrx_rx_framer_bd_hi_get(uint32_t *cfgxpcsrxoltbdhi)
{
    uint32_t reg_rx_framer_bd_hi=0;

#ifdef VALIDATE_PARMS
    if(!cfgxpcsrxoltbdhi)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, XPCSRX, RX_FRAMER_BD_HI, reg_rx_framer_bd_hi);

    *cfgxpcsrxoltbdhi = RU_FIELD_GET(0, XPCSRX, RX_FRAMER_BD_HI, CFGXPCSRXOLTBDHI, reg_rx_framer_bd_hi);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xpcsrx_rx_framer_ebd_sh_set(uint8_t cfgxpcsrxoltebdsh)
{
    uint32_t reg_rx_framer_ebd_sh=0;

#ifdef VALIDATE_PARMS
    if((cfgxpcsrxoltebdsh >= _2BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_rx_framer_ebd_sh = RU_FIELD_SET(0, XPCSRX, RX_FRAMER_EBD_SH, CFGXPCSRXOLTEBDSH, reg_rx_framer_ebd_sh, cfgxpcsrxoltebdsh);

    RU_REG_WRITE(0, XPCSRX, RX_FRAMER_EBD_SH, reg_rx_framer_ebd_sh);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xpcsrx_rx_framer_ebd_sh_get(uint8_t *cfgxpcsrxoltebdsh)
{
    uint32_t reg_rx_framer_ebd_sh=0;

#ifdef VALIDATE_PARMS
    if(!cfgxpcsrxoltebdsh)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, XPCSRX, RX_FRAMER_EBD_SH, reg_rx_framer_ebd_sh);

    *cfgxpcsrxoltebdsh = RU_FIELD_GET(0, XPCSRX, RX_FRAMER_EBD_SH, CFGXPCSRXOLTEBDSH, reg_rx_framer_ebd_sh);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xpcsrx_rx_framer_ebd_lo_set(uint32_t cfgxpcsrxoltebdlo)
{
    uint32_t reg_rx_framer_ebd_lo=0;

#ifdef VALIDATE_PARMS
#endif

    reg_rx_framer_ebd_lo = RU_FIELD_SET(0, XPCSRX, RX_FRAMER_EBD_LO, CFGXPCSRXOLTEBDLO, reg_rx_framer_ebd_lo, cfgxpcsrxoltebdlo);

    RU_REG_WRITE(0, XPCSRX, RX_FRAMER_EBD_LO, reg_rx_framer_ebd_lo);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xpcsrx_rx_framer_ebd_lo_get(uint32_t *cfgxpcsrxoltebdlo)
{
    uint32_t reg_rx_framer_ebd_lo=0;

#ifdef VALIDATE_PARMS
    if(!cfgxpcsrxoltebdlo)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, XPCSRX, RX_FRAMER_EBD_LO, reg_rx_framer_ebd_lo);

    *cfgxpcsrxoltebdlo = RU_FIELD_GET(0, XPCSRX, RX_FRAMER_EBD_LO, CFGXPCSRXOLTEBDLO, reg_rx_framer_ebd_lo);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xpcsrx_rx_framer_ebd_hi_set(uint32_t cfgxpcsrxoltebdhi)
{
    uint32_t reg_rx_framer_ebd_hi=0;

#ifdef VALIDATE_PARMS
#endif

    reg_rx_framer_ebd_hi = RU_FIELD_SET(0, XPCSRX, RX_FRAMER_EBD_HI, CFGXPCSRXOLTEBDHI, reg_rx_framer_ebd_hi, cfgxpcsrxoltebdhi);

    RU_REG_WRITE(0, XPCSRX, RX_FRAMER_EBD_HI, reg_rx_framer_ebd_hi);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xpcsrx_rx_framer_ebd_hi_get(uint32_t *cfgxpcsrxoltebdhi)
{
    uint32_t reg_rx_framer_ebd_hi=0;

#ifdef VALIDATE_PARMS
    if(!cfgxpcsrxoltebdhi)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, XPCSRX, RX_FRAMER_EBD_HI, reg_rx_framer_ebd_hi);

    *cfgxpcsrxoltebdhi = RU_FIELD_GET(0, XPCSRX, RX_FRAMER_EBD_HI, CFGXPCSRXOLTEBDHI, reg_rx_framer_ebd_hi);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xpcsrx_rx_status_get(xpcsrx_rx_status *rx_status)
{
    uint32_t reg_rx_status=0;

#ifdef VALIDATE_PARMS
    if(!rx_status)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, XPCSRX, RX_STATUS, reg_rx_status);

    rx_status->statrxidledajit = RU_FIELD_GET(0, XPCSRX, RX_STATUS, STATRXIDLEDAJIT, reg_rx_status);
    rx_status->statrxfrmrmisbrst = RU_FIELD_GET(0, XPCSRX, RX_STATUS, STATRXFRMRMISBRST, reg_rx_status);
    rx_status->statrxidlesopeopgapbig = RU_FIELD_GET(0, XPCSRX, RX_STATUS, STATRXIDLESOPEOPGAPBIG, reg_rx_status);
    rx_status->statrxidlefrcins = RU_FIELD_GET(0, XPCSRX, RX_STATUS, STATRXIDLEFRCINS, reg_rx_status);
    rx_status->statrx64b66bminipgerr = RU_FIELD_GET(0, XPCSRX, RX_STATUS, STATRX64B66BMINIPGERR, reg_rx_status);
    rx_status->statrxfecnquecntneq = RU_FIELD_GET(0, XPCSRX, RX_STATUS, STATRXFECNQUECNTNEQ, reg_rx_status);
    rx_status->statrxidlefifoundrun = RU_FIELD_GET(0, XPCSRX, RX_STATUS, STATRXIDLEFIFOUNDRUN, reg_rx_status);
    rx_status->statrxidlefifoovrrun = RU_FIELD_GET(0, XPCSRX, RX_STATUS, STATRXIDLEFIFOOVRRUN, reg_rx_status);
    rx_status->statrxfechighcor = RU_FIELD_GET(0, XPCSRX, RX_STATUS, STATRXFECHIGHCOR, reg_rx_status);
    rx_status->statrxfecdecpass = RU_FIELD_GET(0, XPCSRX, RX_STATUS, STATRXFECDECPASS, reg_rx_status);
    rx_status->statrxstatfrmrhighber = RU_FIELD_GET(0, XPCSRX, RX_STATUS, STATRXSTATFRMRHIGHBER, reg_rx_status);
    rx_status->statrxfrmrexitbysp = RU_FIELD_GET(0, XPCSRX, RX_STATUS, STATRXFRMREXITBYSP, reg_rx_status);
    rx_status->statrxfrmrbadshmax = RU_FIELD_GET(0, XPCSRX, RX_STATUS, STATRXFRMRBADSHMAX, reg_rx_status);
    rx_status->statrxdscramburstseqout = RU_FIELD_GET(0, XPCSRX, RX_STATUS, STATRXDSCRAMBURSTSEQOUT, reg_rx_status);
    rx_status->statrxtestpsudolock = RU_FIELD_GET(0, XPCSRX, RX_STATUS, STATRXTESTPSUDOLOCK, reg_rx_status);
    rx_status->statrxtestpsudotype = RU_FIELD_GET(0, XPCSRX, RX_STATUS, STATRXTESTPSUDOTYPE, reg_rx_status);
    rx_status->statrxtestpsudoerr = RU_FIELD_GET(0, XPCSRX, RX_STATUS, STATRXTESTPSUDOERR, reg_rx_status);
    rx_status->statrxtestprbslock = RU_FIELD_GET(0, XPCSRX, RX_STATUS, STATRXTESTPRBSLOCK, reg_rx_status);
    rx_status->statrxtestprbserr = RU_FIELD_GET(0, XPCSRX, RX_STATUS, STATRXTESTPRBSERR, reg_rx_status);
    rx_status->statrxfecpsistdecfail = RU_FIELD_GET(0, XPCSRX, RX_STATUS, STATRXFECPSISTDECFAIL, reg_rx_status);
    rx_status->statrxframerbadsh = RU_FIELD_GET(0, XPCSRX, RX_STATUS, STATRXFRAMERBADSH, reg_rx_status);
    rx_status->statrxframercwloss = RU_FIELD_GET(0, XPCSRX, RX_STATUS, STATRXFRAMERCWLOSS, reg_rx_status);
    rx_status->statrxframercwlock = RU_FIELD_GET(0, XPCSRX, RX_STATUS, STATRXFRAMERCWLOCK, reg_rx_status);
    rx_status->statrxfecdecfail = RU_FIELD_GET(0, XPCSRX, RX_STATUS, STATRXFECDECFAIL, reg_rx_status);
    rx_status->statrx64b66bdecerr = RU_FIELD_GET(0, XPCSRX, RX_STATUS, STATRX64B66BDECERR, reg_rx_status);
    rx_status->statrxfrmrnolocklos = RU_FIELD_GET(0, XPCSRX, RX_STATUS, STATRXFRMRNOLOCKLOS, reg_rx_status);
    rx_status->statrxfrmrrogue = RU_FIELD_GET(0, XPCSRX, RX_STATUS, STATRXFRMRROGUE, reg_rx_status);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xpcsrx_rx_framer_lk_ulk_max_set(uint16_t cfgxpcsrxfrmrsplkmax, uint16_t cfgxpcsrxfrmrspulkmax)
{
    uint32_t reg_rx_framer_lk_ulk_max=0;

#ifdef VALIDATE_PARMS
    if((cfgxpcsrxfrmrsplkmax >= _13BITS_MAX_VAL_) ||
       (cfgxpcsrxfrmrspulkmax >= _13BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_rx_framer_lk_ulk_max = RU_FIELD_SET(0, XPCSRX, RX_FRAMER_LK_ULK_MAX, CFGXPCSRXFRMRSPLKMAX, reg_rx_framer_lk_ulk_max, cfgxpcsrxfrmrsplkmax);
    reg_rx_framer_lk_ulk_max = RU_FIELD_SET(0, XPCSRX, RX_FRAMER_LK_ULK_MAX, CFGXPCSRXFRMRSPULKMAX, reg_rx_framer_lk_ulk_max, cfgxpcsrxfrmrspulkmax);

    RU_REG_WRITE(0, XPCSRX, RX_FRAMER_LK_ULK_MAX, reg_rx_framer_lk_ulk_max);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xpcsrx_rx_framer_lk_ulk_max_get(uint16_t *cfgxpcsrxfrmrsplkmax, uint16_t *cfgxpcsrxfrmrspulkmax)
{
    uint32_t reg_rx_framer_lk_ulk_max=0;

#ifdef VALIDATE_PARMS
    if(!cfgxpcsrxfrmrsplkmax || !cfgxpcsrxfrmrspulkmax)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, XPCSRX, RX_FRAMER_LK_ULK_MAX, reg_rx_framer_lk_ulk_max);

    *cfgxpcsrxfrmrsplkmax = RU_FIELD_GET(0, XPCSRX, RX_FRAMER_LK_ULK_MAX, CFGXPCSRXFRMRSPLKMAX, reg_rx_framer_lk_ulk_max);
    *cfgxpcsrxfrmrspulkmax = RU_FIELD_GET(0, XPCSRX, RX_FRAMER_LK_ULK_MAX, CFGXPCSRXFRMRSPULKMAX, reg_rx_framer_lk_ulk_max);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xpcsrx_rx_framer_sp_sh_set(uint8_t cfgxpcsrxoltspsh)
{
    uint32_t reg_rx_framer_sp_sh=0;

#ifdef VALIDATE_PARMS
    if((cfgxpcsrxoltspsh >= _2BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_rx_framer_sp_sh = RU_FIELD_SET(0, XPCSRX, RX_FRAMER_SP_SH, CFGXPCSRXOLTSPSH, reg_rx_framer_sp_sh, cfgxpcsrxoltspsh);

    RU_REG_WRITE(0, XPCSRX, RX_FRAMER_SP_SH, reg_rx_framer_sp_sh);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xpcsrx_rx_framer_sp_sh_get(uint8_t *cfgxpcsrxoltspsh)
{
    uint32_t reg_rx_framer_sp_sh=0;

#ifdef VALIDATE_PARMS
    if(!cfgxpcsrxoltspsh)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, XPCSRX, RX_FRAMER_SP_SH, reg_rx_framer_sp_sh);

    *cfgxpcsrxoltspsh = RU_FIELD_GET(0, XPCSRX, RX_FRAMER_SP_SH, CFGXPCSRXOLTSPSH, reg_rx_framer_sp_sh);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xpcsrx_rx_framer_sp_lo_set(uint32_t cfgxpcsrxoltsplo)
{
    uint32_t reg_rx_framer_sp_lo=0;

#ifdef VALIDATE_PARMS
#endif

    reg_rx_framer_sp_lo = RU_FIELD_SET(0, XPCSRX, RX_FRAMER_SP_LO, CFGXPCSRXOLTSPLO, reg_rx_framer_sp_lo, cfgxpcsrxoltsplo);

    RU_REG_WRITE(0, XPCSRX, RX_FRAMER_SP_LO, reg_rx_framer_sp_lo);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xpcsrx_rx_framer_sp_lo_get(uint32_t *cfgxpcsrxoltsplo)
{
    uint32_t reg_rx_framer_sp_lo=0;

#ifdef VALIDATE_PARMS
    if(!cfgxpcsrxoltsplo)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, XPCSRX, RX_FRAMER_SP_LO, reg_rx_framer_sp_lo);

    *cfgxpcsrxoltsplo = RU_FIELD_GET(0, XPCSRX, RX_FRAMER_SP_LO, CFGXPCSRXOLTSPLO, reg_rx_framer_sp_lo);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xpcsrx_rx_framer_sp_hi_set(uint32_t cfgxpcsrxoltsphi)
{
    uint32_t reg_rx_framer_sp_hi=0;

#ifdef VALIDATE_PARMS
#endif

    reg_rx_framer_sp_hi = RU_FIELD_SET(0, XPCSRX, RX_FRAMER_SP_HI, CFGXPCSRXOLTSPHI, reg_rx_framer_sp_hi, cfgxpcsrxoltsphi);

    RU_REG_WRITE(0, XPCSRX, RX_FRAMER_SP_HI, reg_rx_framer_sp_hi);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xpcsrx_rx_framer_sp_hi_get(uint32_t *cfgxpcsrxoltsphi)
{
    uint32_t reg_rx_framer_sp_hi=0;

#ifdef VALIDATE_PARMS
    if(!cfgxpcsrxoltsphi)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, XPCSRX, RX_FRAMER_SP_HI, reg_rx_framer_sp_hi);

    *cfgxpcsrxoltsphi = RU_FIELD_GET(0, XPCSRX, RX_FRAMER_SP_HI, CFGXPCSRXOLTSPHI, reg_rx_framer_sp_hi);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xpcsrx_rx_framer_state_get(uint8_t *xpcsrxfrmrstate)
{
    uint32_t reg_rx_framer_state=0;

#ifdef VALIDATE_PARMS
    if(!xpcsrxfrmrstate)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, XPCSRX, RX_FRAMER_STATE, reg_rx_framer_state);

    *xpcsrxfrmrstate = RU_FIELD_GET(0, XPCSRX, RX_FRAMER_STATE, XPCSRXFRMRSTATE, reg_rx_framer_state);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xpcsrx_rx_framer_bd_ebd_ham_set(uint8_t cfgxpcsrxfrmrspham, uint8_t cfgxpcsrxfrmrebdham, uint8_t cfgxpcsrxfrmrbdham)
{
    uint32_t reg_rx_framer_bd_ebd_ham=0;

#ifdef VALIDATE_PARMS
    if((cfgxpcsrxfrmrspham >= _4BITS_MAX_VAL_) ||
       (cfgxpcsrxfrmrebdham >= _4BITS_MAX_VAL_) ||
       (cfgxpcsrxfrmrbdham >= _4BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_rx_framer_bd_ebd_ham = RU_FIELD_SET(0, XPCSRX, RX_FRAMER_BD_EBD_HAM, CFGXPCSRXFRMRSPHAM, reg_rx_framer_bd_ebd_ham, cfgxpcsrxfrmrspham);
    reg_rx_framer_bd_ebd_ham = RU_FIELD_SET(0, XPCSRX, RX_FRAMER_BD_EBD_HAM, CFGXPCSRXFRMREBDHAM, reg_rx_framer_bd_ebd_ham, cfgxpcsrxfrmrebdham);
    reg_rx_framer_bd_ebd_ham = RU_FIELD_SET(0, XPCSRX, RX_FRAMER_BD_EBD_HAM, CFGXPCSRXFRMRBDHAM, reg_rx_framer_bd_ebd_ham, cfgxpcsrxfrmrbdham);

    RU_REG_WRITE(0, XPCSRX, RX_FRAMER_BD_EBD_HAM, reg_rx_framer_bd_ebd_ham);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xpcsrx_rx_framer_bd_ebd_ham_get(uint8_t *cfgxpcsrxfrmrspham, uint8_t *cfgxpcsrxfrmrebdham, uint8_t *cfgxpcsrxfrmrbdham)
{
    uint32_t reg_rx_framer_bd_ebd_ham=0;

#ifdef VALIDATE_PARMS
    if(!cfgxpcsrxfrmrspham || !cfgxpcsrxfrmrebdham || !cfgxpcsrxfrmrbdham)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, XPCSRX, RX_FRAMER_BD_EBD_HAM, reg_rx_framer_bd_ebd_ham);

    *cfgxpcsrxfrmrspham = RU_FIELD_GET(0, XPCSRX, RX_FRAMER_BD_EBD_HAM, CFGXPCSRXFRMRSPHAM, reg_rx_framer_bd_ebd_ham);
    *cfgxpcsrxfrmrebdham = RU_FIELD_GET(0, XPCSRX, RX_FRAMER_BD_EBD_HAM, CFGXPCSRXFRMREBDHAM, reg_rx_framer_bd_ebd_ham);
    *cfgxpcsrxfrmrbdham = RU_FIELD_GET(0, XPCSRX, RX_FRAMER_BD_EBD_HAM, CFGXPCSRXFRMRBDHAM, reg_rx_framer_bd_ebd_ham);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xpcsrx_rx_framer_misbrst_cnt_set(uint32_t rxfrmrmisbrstcnt)
{
    uint32_t reg_rx_framer_misbrst_cnt=0;

#ifdef VALIDATE_PARMS
#endif

    reg_rx_framer_misbrst_cnt = RU_FIELD_SET(0, XPCSRX, RX_FRAMER_MISBRST_CNT, RXFRMRMISBRSTCNT, reg_rx_framer_misbrst_cnt, rxfrmrmisbrstcnt);

    RU_REG_WRITE(0, XPCSRX, RX_FRAMER_MISBRST_CNT, reg_rx_framer_misbrst_cnt);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xpcsrx_rx_framer_misbrst_cnt_get(uint32_t *rxfrmrmisbrstcnt)
{
    uint32_t reg_rx_framer_misbrst_cnt=0;

#ifdef VALIDATE_PARMS
    if(!rxfrmrmisbrstcnt)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, XPCSRX, RX_FRAMER_MISBRST_CNT, reg_rx_framer_misbrst_cnt);

    *rxfrmrmisbrstcnt = RU_FIELD_GET(0, XPCSRX, RX_FRAMER_MISBRST_CNT, RXFRMRMISBRSTCNT, reg_rx_framer_misbrst_cnt);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xpcsrx_rx_framer_bd_err_get(uint8_t *xpcsrxstatfrmrbderr)
{
    uint32_t reg_rx_framer_bd_err=0;

#ifdef VALIDATE_PARMS
    if(!xpcsrxstatfrmrbderr)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, XPCSRX, RX_FRAMER_BD_ERR, reg_rx_framer_bd_err);

    *xpcsrxstatfrmrbderr = RU_FIELD_GET(0, XPCSRX, RX_FRAMER_BD_ERR, XPCSRXSTATFRMRBDERR, reg_rx_framer_bd_err);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xpcsrx_rx_framer_rogue_ctl_set(bdmf_boolean cfgxpcsrxfrmrrogueen, uint16_t cfgxpcsrxfrmrroguesptresh)
{
    uint32_t reg_rx_framer_rogue_ctl=0;

#ifdef VALIDATE_PARMS
    if((cfgxpcsrxfrmrrogueen >= _1BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_rx_framer_rogue_ctl = RU_FIELD_SET(0, XPCSRX, RX_FRAMER_ROGUE_CTL, CFGXPCSRXFRMRROGUEEN, reg_rx_framer_rogue_ctl, cfgxpcsrxfrmrrogueen);
    reg_rx_framer_rogue_ctl = RU_FIELD_SET(0, XPCSRX, RX_FRAMER_ROGUE_CTL, CFGXPCSRXFRMRROGUESPTRESH, reg_rx_framer_rogue_ctl, cfgxpcsrxfrmrroguesptresh);

    RU_REG_WRITE(0, XPCSRX, RX_FRAMER_ROGUE_CTL, reg_rx_framer_rogue_ctl);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xpcsrx_rx_framer_rogue_ctl_get(bdmf_boolean *cfgxpcsrxfrmrrogueen, uint16_t *cfgxpcsrxfrmrroguesptresh)
{
    uint32_t reg_rx_framer_rogue_ctl=0;

#ifdef VALIDATE_PARMS
    if(!cfgxpcsrxfrmrrogueen || !cfgxpcsrxfrmrroguesptresh)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, XPCSRX, RX_FRAMER_ROGUE_CTL, reg_rx_framer_rogue_ctl);

    *cfgxpcsrxfrmrrogueen = RU_FIELD_GET(0, XPCSRX, RX_FRAMER_ROGUE_CTL, CFGXPCSRXFRMRROGUEEN, reg_rx_framer_rogue_ctl);
    *cfgxpcsrxfrmrroguesptresh = RU_FIELD_GET(0, XPCSRX, RX_FRAMER_ROGUE_CTL, CFGXPCSRXFRMRROGUESPTRESH, reg_rx_framer_rogue_ctl);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xpcsrx_rx_framer_nolock_ctl_set(bdmf_boolean cfgxpcsrxfrmrnolocklosen, uint32_t cfgxpcsrxfrmrnolocklosintval)
{
    uint32_t reg_rx_framer_nolock_ctl=0;

#ifdef VALIDATE_PARMS
    if((cfgxpcsrxfrmrnolocklosen >= _1BITS_MAX_VAL_) ||
       (cfgxpcsrxfrmrnolocklosintval >= _24BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_rx_framer_nolock_ctl = RU_FIELD_SET(0, XPCSRX, RX_FRAMER_NOLOCK_CTL, CFGXPCSRXFRMRNOLOCKLOSEN, reg_rx_framer_nolock_ctl, cfgxpcsrxfrmrnolocklosen);
    reg_rx_framer_nolock_ctl = RU_FIELD_SET(0, XPCSRX, RX_FRAMER_NOLOCK_CTL, CFGXPCSRXFRMRNOLOCKLOSINTVAL, reg_rx_framer_nolock_ctl, cfgxpcsrxfrmrnolocklosintval);

    RU_REG_WRITE(0, XPCSRX, RX_FRAMER_NOLOCK_CTL, reg_rx_framer_nolock_ctl);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xpcsrx_rx_framer_nolock_ctl_get(bdmf_boolean *cfgxpcsrxfrmrnolocklosen, uint32_t *cfgxpcsrxfrmrnolocklosintval)
{
    uint32_t reg_rx_framer_nolock_ctl=0;

#ifdef VALIDATE_PARMS
    if(!cfgxpcsrxfrmrnolocklosen || !cfgxpcsrxfrmrnolocklosintval)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, XPCSRX, RX_FRAMER_NOLOCK_CTL, reg_rx_framer_nolock_ctl);

    *cfgxpcsrxfrmrnolocklosen = RU_FIELD_GET(0, XPCSRX, RX_FRAMER_NOLOCK_CTL, CFGXPCSRXFRMRNOLOCKLOSEN, reg_rx_framer_nolock_ctl);
    *cfgxpcsrxfrmrnolocklosintval = RU_FIELD_GET(0, XPCSRX, RX_FRAMER_NOLOCK_CTL, CFGXPCSRXFRMRNOLOCKLOSINTVAL, reg_rx_framer_nolock_ctl);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xpcsrx_rx_64b66b_ipg_det_cnt_set(uint32_t rx64b66bipgdetcnt)
{
    uint32_t reg_rx_64b66b_ipg_det_cnt=0;

#ifdef VALIDATE_PARMS
#endif

    reg_rx_64b66b_ipg_det_cnt = RU_FIELD_SET(0, XPCSRX, RX_64B66B_IPG_DET_CNT, RX64B66BIPGDETCNT, reg_rx_64b66b_ipg_det_cnt, rx64b66bipgdetcnt);

    RU_REG_WRITE(0, XPCSRX, RX_64B66B_IPG_DET_CNT, reg_rx_64b66b_ipg_det_cnt);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xpcsrx_rx_64b66b_ipg_det_cnt_get(uint32_t *rx64b66bipgdetcnt)
{
    uint32_t reg_rx_64b66b_ipg_det_cnt=0;

#ifdef VALIDATE_PARMS
    if(!rx64b66bipgdetcnt)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, XPCSRX, RX_64B66B_IPG_DET_CNT, reg_rx_64b66b_ipg_det_cnt);

    *rx64b66bipgdetcnt = RU_FIELD_GET(0, XPCSRX, RX_64B66B_IPG_DET_CNT, RX64B66BIPGDETCNT, reg_rx_64b66b_ipg_det_cnt);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xpcsrx_rx_fec_nque_in_cnt_set(uint32_t rxfecnqueincnt)
{
    uint32_t reg_rx_fec_nque_in_cnt=0;

#ifdef VALIDATE_PARMS
#endif

    reg_rx_fec_nque_in_cnt = RU_FIELD_SET(0, XPCSRX, RX_FEC_NQUE_IN_CNT, RXFECNQUEINCNT, reg_rx_fec_nque_in_cnt, rxfecnqueincnt);

    RU_REG_WRITE(0, XPCSRX, RX_FEC_NQUE_IN_CNT, reg_rx_fec_nque_in_cnt);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xpcsrx_rx_fec_nque_in_cnt_get(uint32_t *rxfecnqueincnt)
{
    uint32_t reg_rx_fec_nque_in_cnt=0;

#ifdef VALIDATE_PARMS
    if(!rxfecnqueincnt)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, XPCSRX, RX_FEC_NQUE_IN_CNT, reg_rx_fec_nque_in_cnt);

    *rxfecnqueincnt = RU_FIELD_GET(0, XPCSRX, RX_FEC_NQUE_IN_CNT, RXFECNQUEINCNT, reg_rx_fec_nque_in_cnt);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xpcsrx_rx_fec_nque_out_cnt_set(uint32_t rxfecnqueoutcnt)
{
    uint32_t reg_rx_fec_nque_out_cnt=0;

#ifdef VALIDATE_PARMS
#endif

    reg_rx_fec_nque_out_cnt = RU_FIELD_SET(0, XPCSRX, RX_FEC_NQUE_OUT_CNT, RXFECNQUEOUTCNT, reg_rx_fec_nque_out_cnt, rxfecnqueoutcnt);

    RU_REG_WRITE(0, XPCSRX, RX_FEC_NQUE_OUT_CNT, reg_rx_fec_nque_out_cnt);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xpcsrx_rx_fec_nque_out_cnt_get(uint32_t *rxfecnqueoutcnt)
{
    uint32_t reg_rx_fec_nque_out_cnt=0;

#ifdef VALIDATE_PARMS
    if(!rxfecnqueoutcnt)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, XPCSRX, RX_FEC_NQUE_OUT_CNT, reg_rx_fec_nque_out_cnt);

    *rxfecnqueoutcnt = RU_FIELD_GET(0, XPCSRX, RX_FEC_NQUE_OUT_CNT, RXFECNQUEOUTCNT, reg_rx_fec_nque_out_cnt);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xpcsrx_rx_idle_start_cnt_set(uint32_t rxidlestartcnt)
{
    uint32_t reg_rx_idle_start_cnt=0;

#ifdef VALIDATE_PARMS
#endif

    reg_rx_idle_start_cnt = RU_FIELD_SET(0, XPCSRX, RX_IDLE_START_CNT, RXIDLESTARTCNT, reg_rx_idle_start_cnt, rxidlestartcnt);

    RU_REG_WRITE(0, XPCSRX, RX_IDLE_START_CNT, reg_rx_idle_start_cnt);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xpcsrx_rx_idle_start_cnt_get(uint32_t *rxidlestartcnt)
{
    uint32_t reg_rx_idle_start_cnt=0;

#ifdef VALIDATE_PARMS
    if(!rxidlestartcnt)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, XPCSRX, RX_IDLE_START_CNT, reg_rx_idle_start_cnt);

    *rxidlestartcnt = RU_FIELD_GET(0, XPCSRX, RX_IDLE_START_CNT, RXIDLESTARTCNT, reg_rx_idle_start_cnt);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xpcsrx_rx_idle_stop_cnt_set(uint32_t rxidlestopcnt)
{
    uint32_t reg_rx_idle_stop_cnt=0;

#ifdef VALIDATE_PARMS
#endif

    reg_rx_idle_stop_cnt = RU_FIELD_SET(0, XPCSRX, RX_IDLE_STOP_CNT, RXIDLESTOPCNT, reg_rx_idle_stop_cnt, rxidlestopcnt);

    RU_REG_WRITE(0, XPCSRX, RX_IDLE_STOP_CNT, reg_rx_idle_stop_cnt);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xpcsrx_rx_idle_stop_cnt_get(uint32_t *rxidlestopcnt)
{
    uint32_t reg_rx_idle_stop_cnt=0;

#ifdef VALIDATE_PARMS
    if(!rxidlestopcnt)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, XPCSRX, RX_IDLE_STOP_CNT, reg_rx_idle_stop_cnt);

    *rxidlestopcnt = RU_FIELD_GET(0, XPCSRX, RX_IDLE_STOP_CNT, RXIDLESTOPCNT, reg_rx_idle_stop_cnt);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xpcsrx_rx_fec_cor_intval_set(uint32_t cfgxpcsrxfeccorintval)
{
    uint32_t reg_rx_fec_cor_intval=0;

#ifdef VALIDATE_PARMS
    if((cfgxpcsrxfeccorintval >= _24BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_rx_fec_cor_intval = RU_FIELD_SET(0, XPCSRX, RX_FEC_COR_INTVAL, CFGXPCSRXFECCORINTVAL, reg_rx_fec_cor_intval, cfgxpcsrxfeccorintval);

    RU_REG_WRITE(0, XPCSRX, RX_FEC_COR_INTVAL, reg_rx_fec_cor_intval);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xpcsrx_rx_fec_cor_intval_get(uint32_t *cfgxpcsrxfeccorintval)
{
    uint32_t reg_rx_fec_cor_intval=0;

#ifdef VALIDATE_PARMS
    if(!cfgxpcsrxfeccorintval)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, XPCSRX, RX_FEC_COR_INTVAL, reg_rx_fec_cor_intval);

    *cfgxpcsrxfeccorintval = RU_FIELD_GET(0, XPCSRX, RX_FEC_COR_INTVAL, CFGXPCSRXFECCORINTVAL, reg_rx_fec_cor_intval);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xpcsrx_rx_fec_cor_tresh_set(uint32_t cfgxpcsrxfeccortresh)
{
    uint32_t reg_rx_fec_cor_tresh=0;

#ifdef VALIDATE_PARMS
    if((cfgxpcsrxfeccortresh >= _24BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_rx_fec_cor_tresh = RU_FIELD_SET(0, XPCSRX, RX_FEC_COR_TRESH, CFGXPCSRXFECCORTRESH, reg_rx_fec_cor_tresh, cfgxpcsrxfeccortresh);

    RU_REG_WRITE(0, XPCSRX, RX_FEC_COR_TRESH, reg_rx_fec_cor_tresh);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xpcsrx_rx_fec_cor_tresh_get(uint32_t *cfgxpcsrxfeccortresh)
{
    uint32_t reg_rx_fec_cor_tresh=0;

#ifdef VALIDATE_PARMS
    if(!cfgxpcsrxfeccortresh)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, XPCSRX, RX_FEC_COR_TRESH, reg_rx_fec_cor_tresh);

    *cfgxpcsrxfeccortresh = RU_FIELD_GET(0, XPCSRX, RX_FEC_COR_TRESH, CFGXPCSRXFECCORTRESH, reg_rx_fec_cor_tresh);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xpcsrx_rx_fec_cw_fail_cnt_set(uint32_t rxfecdeccwfailcnt)
{
    uint32_t reg_rx_fec_cw_fail_cnt=0;

#ifdef VALIDATE_PARMS
#endif

    reg_rx_fec_cw_fail_cnt = RU_FIELD_SET(0, XPCSRX, RX_FEC_CW_FAIL_CNT, RXFECDECCWFAILCNT, reg_rx_fec_cw_fail_cnt, rxfecdeccwfailcnt);

    RU_REG_WRITE(0, XPCSRX, RX_FEC_CW_FAIL_CNT, reg_rx_fec_cw_fail_cnt);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xpcsrx_rx_fec_cw_fail_cnt_get(uint32_t *rxfecdeccwfailcnt)
{
    uint32_t reg_rx_fec_cw_fail_cnt=0;

#ifdef VALIDATE_PARMS
    if(!rxfecdeccwfailcnt)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, XPCSRX, RX_FEC_CW_FAIL_CNT, reg_rx_fec_cw_fail_cnt);

    *rxfecdeccwfailcnt = RU_FIELD_GET(0, XPCSRX, RX_FEC_CW_FAIL_CNT, RXFECDECCWFAILCNT, reg_rx_fec_cw_fail_cnt);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xpcsrx_rx_fec_cw_tot_cnt_set(uint32_t rxfecdeccwtotcnt)
{
    uint32_t reg_rx_fec_cw_tot_cnt=0;

#ifdef VALIDATE_PARMS
#endif

    reg_rx_fec_cw_tot_cnt = RU_FIELD_SET(0, XPCSRX, RX_FEC_CW_TOT_CNT, RXFECDECCWTOTCNT, reg_rx_fec_cw_tot_cnt, rxfecdeccwtotcnt);

    RU_REG_WRITE(0, XPCSRX, RX_FEC_CW_TOT_CNT, reg_rx_fec_cw_tot_cnt);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xpcsrx_rx_fec_cw_tot_cnt_get(uint32_t *rxfecdeccwtotcnt)
{
    uint32_t reg_rx_fec_cw_tot_cnt=0;

#ifdef VALIDATE_PARMS
    if(!rxfecdeccwtotcnt)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, XPCSRX, RX_FEC_CW_TOT_CNT, reg_rx_fec_cw_tot_cnt);

    *rxfecdeccwtotcnt = RU_FIELD_GET(0, XPCSRX, RX_FEC_CW_TOT_CNT, RXFECDECCWTOTCNT, reg_rx_fec_cw_tot_cnt);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xpcsrx_rx_fec_correct_cnt_lower_set(uint32_t rxfecdecerrcorcntlower)
{
    uint32_t reg_rx_fec_correct_cnt_lower=0;

#ifdef VALIDATE_PARMS
#endif

    reg_rx_fec_correct_cnt_lower = RU_FIELD_SET(0, XPCSRX, RX_FEC_CORRECT_CNT_LOWER, RXFECDECERRCORCNTLOWER, reg_rx_fec_correct_cnt_lower, rxfecdecerrcorcntlower);

    RU_REG_WRITE(0, XPCSRX, RX_FEC_CORRECT_CNT_LOWER, reg_rx_fec_correct_cnt_lower);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xpcsrx_rx_fec_correct_cnt_lower_get(uint32_t *rxfecdecerrcorcntlower)
{
    uint32_t reg_rx_fec_correct_cnt_lower=0;

#ifdef VALIDATE_PARMS
    if(!rxfecdecerrcorcntlower)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, XPCSRX, RX_FEC_CORRECT_CNT_LOWER, reg_rx_fec_correct_cnt_lower);

    *rxfecdecerrcorcntlower = RU_FIELD_GET(0, XPCSRX, RX_FEC_CORRECT_CNT_LOWER, RXFECDECERRCORCNTLOWER, reg_rx_fec_correct_cnt_lower);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xpcsrx_rx_fec_correct_cnt_upper_get(uint8_t *rxfecdecerrcorcntupper)
{
    uint32_t reg_rx_fec_correct_cnt_upper=0;

#ifdef VALIDATE_PARMS
    if(!rxfecdecerrcorcntupper)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, XPCSRX, RX_FEC_CORRECT_CNT_UPPER, reg_rx_fec_correct_cnt_upper);

    *rxfecdecerrcorcntupper = RU_FIELD_GET(0, XPCSRX, RX_FEC_CORRECT_CNT_UPPER, RXFECDECERRCORCNTUPPER, reg_rx_fec_correct_cnt_upper);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xpcsrx_rx_fec_correct_cnt_shadow_set(uint8_t rxfecdecerrcorcntshadow)
{
    uint32_t reg_rx_fec_correct_cnt_shadow=0;

#ifdef VALIDATE_PARMS
#endif

    reg_rx_fec_correct_cnt_shadow = RU_FIELD_SET(0, XPCSRX, RX_FEC_CORRECT_CNT_SHADOW, RXFECDECERRCORCNTSHADOW, reg_rx_fec_correct_cnt_shadow, rxfecdecerrcorcntshadow);

    RU_REG_WRITE(0, XPCSRX, RX_FEC_CORRECT_CNT_SHADOW, reg_rx_fec_correct_cnt_shadow);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xpcsrx_rx_fec_correct_cnt_shadow_get(uint8_t *rxfecdecerrcorcntshadow)
{
    uint32_t reg_rx_fec_correct_cnt_shadow=0;

#ifdef VALIDATE_PARMS
    if(!rxfecdecerrcorcntshadow)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, XPCSRX, RX_FEC_CORRECT_CNT_SHADOW, reg_rx_fec_correct_cnt_shadow);

    *rxfecdecerrcorcntshadow = RU_FIELD_GET(0, XPCSRX, RX_FEC_CORRECT_CNT_SHADOW, RXFECDECERRCORCNTSHADOW, reg_rx_fec_correct_cnt_shadow);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xpcsrx_rx_fec_ones_cor_cnt_lower_set(uint32_t rxfecdeconescorcntlower)
{
    uint32_t reg_rx_fec_ones_cor_cnt_lower=0;

#ifdef VALIDATE_PARMS
#endif

    reg_rx_fec_ones_cor_cnt_lower = RU_FIELD_SET(0, XPCSRX, RX_FEC_ONES_COR_CNT_LOWER, RXFECDECONESCORCNTLOWER, reg_rx_fec_ones_cor_cnt_lower, rxfecdeconescorcntlower);

    RU_REG_WRITE(0, XPCSRX, RX_FEC_ONES_COR_CNT_LOWER, reg_rx_fec_ones_cor_cnt_lower);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xpcsrx_rx_fec_ones_cor_cnt_lower_get(uint32_t *rxfecdeconescorcntlower)
{
    uint32_t reg_rx_fec_ones_cor_cnt_lower=0;

#ifdef VALIDATE_PARMS
    if(!rxfecdeconescorcntlower)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, XPCSRX, RX_FEC_ONES_COR_CNT_LOWER, reg_rx_fec_ones_cor_cnt_lower);

    *rxfecdeconescorcntlower = RU_FIELD_GET(0, XPCSRX, RX_FEC_ONES_COR_CNT_LOWER, RXFECDECONESCORCNTLOWER, reg_rx_fec_ones_cor_cnt_lower);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xpcsrx_rx_fec_ones_cor_cnt_upper_get(uint8_t *rxfecdeconescorcntupper)
{
    uint32_t reg_rx_fec_ones_cor_cnt_upper=0;

#ifdef VALIDATE_PARMS
    if(!rxfecdeconescorcntupper)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, XPCSRX, RX_FEC_ONES_COR_CNT_UPPER, reg_rx_fec_ones_cor_cnt_upper);

    *rxfecdeconescorcntupper = RU_FIELD_GET(0, XPCSRX, RX_FEC_ONES_COR_CNT_UPPER, RXFECDECONESCORCNTUPPER, reg_rx_fec_ones_cor_cnt_upper);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xpcsrx_rx_fec_ones_cor_cnt_shadow_set(uint8_t rxfecdeconescorcntshadow)
{
    uint32_t reg_rx_fec_ones_cor_cnt_shadow=0;

#ifdef VALIDATE_PARMS
#endif

    reg_rx_fec_ones_cor_cnt_shadow = RU_FIELD_SET(0, XPCSRX, RX_FEC_ONES_COR_CNT_SHADOW, RXFECDECONESCORCNTSHADOW, reg_rx_fec_ones_cor_cnt_shadow, rxfecdeconescorcntshadow);

    RU_REG_WRITE(0, XPCSRX, RX_FEC_ONES_COR_CNT_SHADOW, reg_rx_fec_ones_cor_cnt_shadow);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xpcsrx_rx_fec_ones_cor_cnt_shadow_get(uint8_t *rxfecdeconescorcntshadow)
{
    uint32_t reg_rx_fec_ones_cor_cnt_shadow=0;

#ifdef VALIDATE_PARMS
    if(!rxfecdeconescorcntshadow)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, XPCSRX, RX_FEC_ONES_COR_CNT_SHADOW, reg_rx_fec_ones_cor_cnt_shadow);

    *rxfecdeconescorcntshadow = RU_FIELD_GET(0, XPCSRX, RX_FEC_ONES_COR_CNT_SHADOW, RXFECDECONESCORCNTSHADOW, reg_rx_fec_ones_cor_cnt_shadow);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xpcsrx_rx_fec_zeros_cor_cnt_lower_set(uint32_t rxfecdeczeroscorcntlower)
{
    uint32_t reg_rx_fec_zeros_cor_cnt_lower=0;

#ifdef VALIDATE_PARMS
#endif

    reg_rx_fec_zeros_cor_cnt_lower = RU_FIELD_SET(0, XPCSRX, RX_FEC_ZEROS_COR_CNT_LOWER, RXFECDECZEROSCORCNTLOWER, reg_rx_fec_zeros_cor_cnt_lower, rxfecdeczeroscorcntlower);

    RU_REG_WRITE(0, XPCSRX, RX_FEC_ZEROS_COR_CNT_LOWER, reg_rx_fec_zeros_cor_cnt_lower);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xpcsrx_rx_fec_zeros_cor_cnt_lower_get(uint32_t *rxfecdeczeroscorcntlower)
{
    uint32_t reg_rx_fec_zeros_cor_cnt_lower=0;

#ifdef VALIDATE_PARMS
    if(!rxfecdeczeroscorcntlower)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, XPCSRX, RX_FEC_ZEROS_COR_CNT_LOWER, reg_rx_fec_zeros_cor_cnt_lower);

    *rxfecdeczeroscorcntlower = RU_FIELD_GET(0, XPCSRX, RX_FEC_ZEROS_COR_CNT_LOWER, RXFECDECZEROSCORCNTLOWER, reg_rx_fec_zeros_cor_cnt_lower);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xpcsrx_rx_fec_zeros_cor_cnt_upper_get(uint8_t *rxfecdeczeroscorcntupper)
{
    uint32_t reg_rx_fec_zeros_cor_cnt_upper=0;

#ifdef VALIDATE_PARMS
    if(!rxfecdeczeroscorcntupper)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, XPCSRX, RX_FEC_ZEROS_COR_CNT_UPPER, reg_rx_fec_zeros_cor_cnt_upper);

    *rxfecdeczeroscorcntupper = RU_FIELD_GET(0, XPCSRX, RX_FEC_ZEROS_COR_CNT_UPPER, RXFECDECZEROSCORCNTUPPER, reg_rx_fec_zeros_cor_cnt_upper);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xpcsrx_rx_fec_zeros_cor_cnt_shadow_set(uint8_t rxfecdeczeroscorcntshadow)
{
    uint32_t reg_rx_fec_zeros_cor_cnt_shadow=0;

#ifdef VALIDATE_PARMS
#endif

    reg_rx_fec_zeros_cor_cnt_shadow = RU_FIELD_SET(0, XPCSRX, RX_FEC_ZEROS_COR_CNT_SHADOW, RXFECDECZEROSCORCNTSHADOW, reg_rx_fec_zeros_cor_cnt_shadow, rxfecdeczeroscorcntshadow);

    RU_REG_WRITE(0, XPCSRX, RX_FEC_ZEROS_COR_CNT_SHADOW, reg_rx_fec_zeros_cor_cnt_shadow);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xpcsrx_rx_fec_zeros_cor_cnt_shadow_get(uint8_t *rxfecdeczeroscorcntshadow)
{
    uint32_t reg_rx_fec_zeros_cor_cnt_shadow=0;

#ifdef VALIDATE_PARMS
    if(!rxfecdeczeroscorcntshadow)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, XPCSRX, RX_FEC_ZEROS_COR_CNT_SHADOW, reg_rx_fec_zeros_cor_cnt_shadow);

    *rxfecdeczeroscorcntshadow = RU_FIELD_GET(0, XPCSRX, RX_FEC_ZEROS_COR_CNT_SHADOW, RXFECDECZEROSCORCNTSHADOW, reg_rx_fec_zeros_cor_cnt_shadow);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xpcsrx_rx_fec_stop_on_err_read_pointer_get(uint8_t *rxfecstoponerrrdptr, uint8_t *rxfecstoponerrwrptr)
{
    uint32_t reg_rx_fec_stop_on_err_read_pointer=0;

#ifdef VALIDATE_PARMS
    if(!rxfecstoponerrrdptr || !rxfecstoponerrwrptr)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, XPCSRX, RX_FEC_STOP_ON_ERR_READ_POINTER, reg_rx_fec_stop_on_err_read_pointer);

    *rxfecstoponerrrdptr = RU_FIELD_GET(0, XPCSRX, RX_FEC_STOP_ON_ERR_READ_POINTER, RXFECSTOPONERRRDPTR, reg_rx_fec_stop_on_err_read_pointer);
    *rxfecstoponerrwrptr = RU_FIELD_GET(0, XPCSRX, RX_FEC_STOP_ON_ERR_READ_POINTER, RXFECSTOPONERRWRPTR, reg_rx_fec_stop_on_err_read_pointer);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xpcsrx_rx_fec_stop_on_err_burst_location_get(uint32_t *rxfecstoponerrbrstloc)
{
    uint32_t reg_rx_fec_stop_on_err_burst_location=0;

#ifdef VALIDATE_PARMS
    if(!rxfecstoponerrbrstloc)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, XPCSRX, RX_FEC_STOP_ON_ERR_BURST_LOCATION, reg_rx_fec_stop_on_err_burst_location);

    *rxfecstoponerrbrstloc = RU_FIELD_GET(0, XPCSRX, RX_FEC_STOP_ON_ERR_BURST_LOCATION, RXFECSTOPONERRBRSTLOC, reg_rx_fec_stop_on_err_burst_location);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xpcsrx_rx_64b66b_fail_cnt_set(uint32_t rx64b66bdecerrcnt)
{
    uint32_t reg_rx_64b66b_fail_cnt=0;

#ifdef VALIDATE_PARMS
#endif

    reg_rx_64b66b_fail_cnt = RU_FIELD_SET(0, XPCSRX, RX_64B66B_FAIL_CNT, RX64B66BDECERRCNT, reg_rx_64b66b_fail_cnt, rx64b66bdecerrcnt);

    RU_REG_WRITE(0, XPCSRX, RX_64B66B_FAIL_CNT, reg_rx_64b66b_fail_cnt);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xpcsrx_rx_64b66b_fail_cnt_get(uint32_t *rx64b66bdecerrcnt)
{
    uint32_t reg_rx_64b66b_fail_cnt=0;

#ifdef VALIDATE_PARMS
    if(!rx64b66bdecerrcnt)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, XPCSRX, RX_64B66B_FAIL_CNT, reg_rx_64b66b_fail_cnt);

    *rx64b66bdecerrcnt = RU_FIELD_GET(0, XPCSRX, RX_64B66B_FAIL_CNT, RX64B66BDECERRCNT, reg_rx_64b66b_fail_cnt);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xpcsrx_rx_frmr_bad_sh_cnt_set(uint32_t rxfrmrbadshcnt)
{
    uint32_t reg_rx_frmr_bad_sh_cnt=0;

#ifdef VALIDATE_PARMS
#endif

    reg_rx_frmr_bad_sh_cnt = RU_FIELD_SET(0, XPCSRX, RX_FRMR_BAD_SH_CNT, RXFRMRBADSHCNT, reg_rx_frmr_bad_sh_cnt, rxfrmrbadshcnt);

    RU_REG_WRITE(0, XPCSRX, RX_FRMR_BAD_SH_CNT, reg_rx_frmr_bad_sh_cnt);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xpcsrx_rx_frmr_bad_sh_cnt_get(uint32_t *rxfrmrbadshcnt)
{
    uint32_t reg_rx_frmr_bad_sh_cnt=0;

#ifdef VALIDATE_PARMS
    if(!rxfrmrbadshcnt)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, XPCSRX, RX_FRMR_BAD_SH_CNT, reg_rx_frmr_bad_sh_cnt);

    *rxfrmrbadshcnt = RU_FIELD_GET(0, XPCSRX, RX_FRMR_BAD_SH_CNT, RXFRMRBADSHCNT, reg_rx_frmr_bad_sh_cnt);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xpcsrx_rx_psudo_cnt_set(uint32_t rxtestpsudoerrcnt)
{
    uint32_t reg_rx_psudo_cnt=0;

#ifdef VALIDATE_PARMS
#endif

    reg_rx_psudo_cnt = RU_FIELD_SET(0, XPCSRX, RX_PSUDO_CNT, RXTESTPSUDOERRCNT, reg_rx_psudo_cnt, rxtestpsudoerrcnt);

    RU_REG_WRITE(0, XPCSRX, RX_PSUDO_CNT, reg_rx_psudo_cnt);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xpcsrx_rx_psudo_cnt_get(uint32_t *rxtestpsudoerrcnt)
{
    uint32_t reg_rx_psudo_cnt=0;

#ifdef VALIDATE_PARMS
    if(!rxtestpsudoerrcnt)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, XPCSRX, RX_PSUDO_CNT, reg_rx_psudo_cnt);

    *rxtestpsudoerrcnt = RU_FIELD_GET(0, XPCSRX, RX_PSUDO_CNT, RXTESTPSUDOERRCNT, reg_rx_psudo_cnt);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xpcsrx_rx_prbs_cnt_set(uint32_t rxtestprbserrcnt)
{
    uint32_t reg_rx_prbs_cnt=0;

#ifdef VALIDATE_PARMS
#endif

    reg_rx_prbs_cnt = RU_FIELD_SET(0, XPCSRX, RX_PRBS_CNT, RXTESTPRBSERRCNT, reg_rx_prbs_cnt, rxtestprbserrcnt);

    RU_REG_WRITE(0, XPCSRX, RX_PRBS_CNT, reg_rx_prbs_cnt);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xpcsrx_rx_prbs_cnt_get(uint32_t *rxtestprbserrcnt)
{
    uint32_t reg_rx_prbs_cnt=0;

#ifdef VALIDATE_PARMS
    if(!rxtestprbserrcnt)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, XPCSRX, RX_PRBS_CNT, reg_rx_prbs_cnt);

    *rxtestprbserrcnt = RU_FIELD_GET(0, XPCSRX, RX_PRBS_CNT, RXTESTPRBSERRCNT, reg_rx_prbs_cnt);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xpcsrx_rx_ber_intval_set(uint32_t cfgxpcsrxfrmrberintval)
{
    uint32_t reg_rx_ber_intval=0;

#ifdef VALIDATE_PARMS
    if((cfgxpcsrxfrmrberintval >= _24BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_rx_ber_intval = RU_FIELD_SET(0, XPCSRX, RX_BER_INTVAL, CFGXPCSRXFRMRBERINTVAL, reg_rx_ber_intval, cfgxpcsrxfrmrberintval);

    RU_REG_WRITE(0, XPCSRX, RX_BER_INTVAL, reg_rx_ber_intval);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xpcsrx_rx_ber_intval_get(uint32_t *cfgxpcsrxfrmrberintval)
{
    uint32_t reg_rx_ber_intval=0;

#ifdef VALIDATE_PARMS
    if(!cfgxpcsrxfrmrberintval)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, XPCSRX, RX_BER_INTVAL, reg_rx_ber_intval);

    *cfgxpcsrxfrmrberintval = RU_FIELD_GET(0, XPCSRX, RX_BER_INTVAL, CFGXPCSRXFRMRBERINTVAL, reg_rx_ber_intval);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xpcsrx_rx_ber_tresh_set(uint16_t cfgxpcsrxfrmrbertresh)
{
    uint32_t reg_rx_ber_tresh=0;

#ifdef VALIDATE_PARMS
    if((cfgxpcsrxfrmrbertresh >= _9BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_rx_ber_tresh = RU_FIELD_SET(0, XPCSRX, RX_BER_TRESH, CFGXPCSRXFRMRBERTRESH, reg_rx_ber_tresh, cfgxpcsrxfrmrbertresh);

    RU_REG_WRITE(0, XPCSRX, RX_BER_TRESH, reg_rx_ber_tresh);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xpcsrx_rx_ber_tresh_get(uint16_t *cfgxpcsrxfrmrbertresh)
{
    uint32_t reg_rx_ber_tresh=0;

#ifdef VALIDATE_PARMS
    if(!cfgxpcsrxfrmrbertresh)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, XPCSRX, RX_BER_TRESH, reg_rx_ber_tresh);

    *cfgxpcsrxfrmrbertresh = RU_FIELD_GET(0, XPCSRX, RX_BER_TRESH, CFGXPCSRXFRMRBERTRESH, reg_rx_ber_tresh);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xpcsrx_rx_64b66b_start_cnt_set(uint32_t rx64b66bdecstartcnt)
{
    uint32_t reg_rx_64b66b_start_cnt=0;

#ifdef VALIDATE_PARMS
#endif

    reg_rx_64b66b_start_cnt = RU_FIELD_SET(0, XPCSRX, RX_64B66B_START_CNT, RX64B66BDECSTARTCNT, reg_rx_64b66b_start_cnt, rx64b66bdecstartcnt);

    RU_REG_WRITE(0, XPCSRX, RX_64B66B_START_CNT, reg_rx_64b66b_start_cnt);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xpcsrx_rx_64b66b_start_cnt_get(uint32_t *rx64b66bdecstartcnt)
{
    uint32_t reg_rx_64b66b_start_cnt=0;

#ifdef VALIDATE_PARMS
    if(!rx64b66bdecstartcnt)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, XPCSRX, RX_64B66B_START_CNT, reg_rx_64b66b_start_cnt);

    *rx64b66bdecstartcnt = RU_FIELD_GET(0, XPCSRX, RX_64B66B_START_CNT, RX64B66BDECSTARTCNT, reg_rx_64b66b_start_cnt);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xpcsrx_rx_idle_good_pkt_cnt_set(uint32_t rxidlegoodpktcnt)
{
    uint32_t reg_rx_idle_good_pkt_cnt=0;

#ifdef VALIDATE_PARMS
#endif

    reg_rx_idle_good_pkt_cnt = RU_FIELD_SET(0, XPCSRX, RX_IDLE_GOOD_PKT_CNT, RXIDLEGOODPKTCNT, reg_rx_idle_good_pkt_cnt, rxidlegoodpktcnt);

    RU_REG_WRITE(0, XPCSRX, RX_IDLE_GOOD_PKT_CNT, reg_rx_idle_good_pkt_cnt);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xpcsrx_rx_idle_good_pkt_cnt_get(uint32_t *rxidlegoodpktcnt)
{
    uint32_t reg_rx_idle_good_pkt_cnt=0;

#ifdef VALIDATE_PARMS
    if(!rxidlegoodpktcnt)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, XPCSRX, RX_IDLE_GOOD_PKT_CNT, reg_rx_idle_good_pkt_cnt);

    *rxidlegoodpktcnt = RU_FIELD_GET(0, XPCSRX, RX_IDLE_GOOD_PKT_CNT, RXIDLEGOODPKTCNT, reg_rx_idle_good_pkt_cnt);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xpcsrx_rx_idle_err_pkt_cnt_set(uint32_t rxidleerrpktcnt)
{
    uint32_t reg_rx_idle_err_pkt_cnt=0;

#ifdef VALIDATE_PARMS
#endif

    reg_rx_idle_err_pkt_cnt = RU_FIELD_SET(0, XPCSRX, RX_IDLE_ERR_PKT_CNT, RXIDLEERRPKTCNT, reg_rx_idle_err_pkt_cnt, rxidleerrpktcnt);

    RU_REG_WRITE(0, XPCSRX, RX_IDLE_ERR_PKT_CNT, reg_rx_idle_err_pkt_cnt);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xpcsrx_rx_idle_err_pkt_cnt_get(uint32_t *rxidleerrpktcnt)
{
    uint32_t reg_rx_idle_err_pkt_cnt=0;

#ifdef VALIDATE_PARMS
    if(!rxidleerrpktcnt)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, XPCSRX, RX_IDLE_ERR_PKT_CNT, reg_rx_idle_err_pkt_cnt);

    *rxidleerrpktcnt = RU_FIELD_GET(0, XPCSRX, RX_IDLE_ERR_PKT_CNT, RXIDLEERRPKTCNT, reg_rx_idle_err_pkt_cnt);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xpcsrx_rx_64b66b_stop_cnt_set(uint32_t rx64b66bdecstopcnt)
{
    uint32_t reg_rx_64b66b_stop_cnt=0;

#ifdef VALIDATE_PARMS
#endif

    reg_rx_64b66b_stop_cnt = RU_FIELD_SET(0, XPCSRX, RX_64B66B_STOP_CNT, RX64B66BDECSTOPCNT, reg_rx_64b66b_stop_cnt, rx64b66bdecstopcnt);

    RU_REG_WRITE(0, XPCSRX, RX_64B66B_STOP_CNT, reg_rx_64b66b_stop_cnt);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xpcsrx_rx_64b66b_stop_cnt_get(uint32_t *rx64b66bdecstopcnt)
{
    uint32_t reg_rx_64b66b_stop_cnt=0;

#ifdef VALIDATE_PARMS
    if(!rx64b66bdecstopcnt)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, XPCSRX, RX_64B66B_STOP_CNT, reg_rx_64b66b_stop_cnt);

    *rx64b66bdecstopcnt = RU_FIELD_GET(0, XPCSRX, RX_64B66B_STOP_CNT, RX64B66BDECSTOPCNT, reg_rx_64b66b_stop_cnt);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xpcsrx_rx_burst_out_odr_cnt_set(uint32_t rxburstseqoutofordercnt)
{
    uint32_t reg_rx_burst_out_odr_cnt=0;

#ifdef VALIDATE_PARMS
#endif

    reg_rx_burst_out_odr_cnt = RU_FIELD_SET(0, XPCSRX, RX_BURST_OUT_ODR_CNT, RXBURSTSEQOUTOFORDERCNT, reg_rx_burst_out_odr_cnt, rxburstseqoutofordercnt);

    RU_REG_WRITE(0, XPCSRX, RX_BURST_OUT_ODR_CNT, reg_rx_burst_out_odr_cnt);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xpcsrx_rx_burst_out_odr_cnt_get(uint32_t *rxburstseqoutofordercnt)
{
    uint32_t reg_rx_burst_out_odr_cnt=0;

#ifdef VALIDATE_PARMS
    if(!rxburstseqoutofordercnt)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, XPCSRX, RX_BURST_OUT_ODR_CNT, reg_rx_burst_out_odr_cnt);

    *rxburstseqoutofordercnt = RU_FIELD_GET(0, XPCSRX, RX_BURST_OUT_ODR_CNT, RXBURSTSEQOUTOFORDERCNT, reg_rx_burst_out_odr_cnt);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xpcsrx_rx_idle_da_jit_dly_get(uint16_t *rxidlelastdacnt, uint16_t *rxidledacnt)
{
    uint32_t reg_rx_idle_da_jit_dly=0;

#ifdef VALIDATE_PARMS
    if(!rxidlelastdacnt || !rxidledacnt)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, XPCSRX, RX_IDLE_DA_JIT_DLY, reg_rx_idle_da_jit_dly);

    *rxidlelastdacnt = RU_FIELD_GET(0, XPCSRX, RX_IDLE_DA_JIT_DLY, RXIDLELASTDACNT, reg_rx_idle_da_jit_dly);
    *rxidledacnt = RU_FIELD_GET(0, XPCSRX, RX_IDLE_DA_JIT_DLY, RXIDLEDACNT, reg_rx_idle_da_jit_dly);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xpcsrx_rx_dport_ctl_set(const xpcsrx_rx_dport_ctl *rx_dport_ctl)
{
    uint32_t reg_rx_dport_ctl=0;

#ifdef VALIDATE_PARMS
    if(!rx_dport_ctl)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((rx_dport_ctl->xpcsrxdpbusy >= _1BITS_MAX_VAL_) ||
       (rx_dport_ctl->xpcsrxdperr >= _1BITS_MAX_VAL_) ||
       (rx_dport_ctl->cfgxpcsrxdpramsel >= _4BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_rx_dport_ctl = RU_FIELD_SET(0, XPCSRX, RX_DPORT_CTL, XPCSRXDPBUSY, reg_rx_dport_ctl, rx_dport_ctl->xpcsrxdpbusy);
    reg_rx_dport_ctl = RU_FIELD_SET(0, XPCSRX, RX_DPORT_CTL, XPCSRXDPERR, reg_rx_dport_ctl, rx_dport_ctl->xpcsrxdperr);
    reg_rx_dport_ctl = RU_FIELD_SET(0, XPCSRX, RX_DPORT_CTL, CFGXPCSRXDPCTL, reg_rx_dport_ctl, rx_dport_ctl->cfgxpcsrxdpctl);
    reg_rx_dport_ctl = RU_FIELD_SET(0, XPCSRX, RX_DPORT_CTL, CFGXPCSRXDPRAMSEL, reg_rx_dport_ctl, rx_dport_ctl->cfgxpcsrxdpramsel);
    reg_rx_dport_ctl = RU_FIELD_SET(0, XPCSRX, RX_DPORT_CTL, CFGXPCSRXDPADDR, reg_rx_dport_ctl, rx_dport_ctl->cfgxpcsrxdpaddr);

    RU_REG_WRITE(0, XPCSRX, RX_DPORT_CTL, reg_rx_dport_ctl);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xpcsrx_rx_dport_ctl_get(xpcsrx_rx_dport_ctl *rx_dport_ctl)
{
    uint32_t reg_rx_dport_ctl=0;

#ifdef VALIDATE_PARMS
    if(!rx_dport_ctl)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, XPCSRX, RX_DPORT_CTL, reg_rx_dport_ctl);

    rx_dport_ctl->xpcsrxdpbusy = RU_FIELD_GET(0, XPCSRX, RX_DPORT_CTL, XPCSRXDPBUSY, reg_rx_dport_ctl);
    rx_dport_ctl->xpcsrxdperr = RU_FIELD_GET(0, XPCSRX, RX_DPORT_CTL, XPCSRXDPERR, reg_rx_dport_ctl);
    rx_dport_ctl->cfgxpcsrxdpctl = RU_FIELD_GET(0, XPCSRX, RX_DPORT_CTL, CFGXPCSRXDPCTL, reg_rx_dport_ctl);
    rx_dport_ctl->cfgxpcsrxdpramsel = RU_FIELD_GET(0, XPCSRX, RX_DPORT_CTL, CFGXPCSRXDPRAMSEL, reg_rx_dport_ctl);
    rx_dport_ctl->cfgxpcsrxdpaddr = RU_FIELD_GET(0, XPCSRX, RX_DPORT_CTL, CFGXPCSRXDPADDR, reg_rx_dport_ctl);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xpcsrx_rx_dport_data0_set(uint32_t xpcsrxdpdata0)
{
    uint32_t reg_rx_dport_data0=0;

#ifdef VALIDATE_PARMS
#endif

    reg_rx_dport_data0 = RU_FIELD_SET(0, XPCSRX, RX_DPORT_DATA0, XPCSRXDPDATA0, reg_rx_dport_data0, xpcsrxdpdata0);

    RU_REG_WRITE(0, XPCSRX, RX_DPORT_DATA0, reg_rx_dport_data0);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xpcsrx_rx_dport_data0_get(uint32_t *xpcsrxdpdata0)
{
    uint32_t reg_rx_dport_data0=0;

#ifdef VALIDATE_PARMS
    if(!xpcsrxdpdata0)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, XPCSRX, RX_DPORT_DATA0, reg_rx_dport_data0);

    *xpcsrxdpdata0 = RU_FIELD_GET(0, XPCSRX, RX_DPORT_DATA0, XPCSRXDPDATA0, reg_rx_dport_data0);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xpcsrx_rx_dport_data1_set(uint32_t xpcsrxdpdata1)
{
    uint32_t reg_rx_dport_data1=0;

#ifdef VALIDATE_PARMS
#endif

    reg_rx_dport_data1 = RU_FIELD_SET(0, XPCSRX, RX_DPORT_DATA1, XPCSRXDPDATA1, reg_rx_dport_data1, xpcsrxdpdata1);

    RU_REG_WRITE(0, XPCSRX, RX_DPORT_DATA1, reg_rx_dport_data1);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xpcsrx_rx_dport_data1_get(uint32_t *xpcsrxdpdata1)
{
    uint32_t reg_rx_dport_data1=0;

#ifdef VALIDATE_PARMS
    if(!xpcsrxdpdata1)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, XPCSRX, RX_DPORT_DATA1, reg_rx_dport_data1);

    *xpcsrxdpdata1 = RU_FIELD_GET(0, XPCSRX, RX_DPORT_DATA1, XPCSRXDPDATA1, reg_rx_dport_data1);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xpcsrx_rx_dport_data2_set(uint32_t xpcsrxdpdata2)
{
    uint32_t reg_rx_dport_data2=0;

#ifdef VALIDATE_PARMS
#endif

    reg_rx_dport_data2 = RU_FIELD_SET(0, XPCSRX, RX_DPORT_DATA2, XPCSRXDPDATA2, reg_rx_dport_data2, xpcsrxdpdata2);

    RU_REG_WRITE(0, XPCSRX, RX_DPORT_DATA2, reg_rx_dport_data2);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xpcsrx_rx_dport_data2_get(uint32_t *xpcsrxdpdata2)
{
    uint32_t reg_rx_dport_data2=0;

#ifdef VALIDATE_PARMS
    if(!xpcsrxdpdata2)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, XPCSRX, RX_DPORT_DATA2, reg_rx_dport_data2);

    *xpcsrxdpdata2 = RU_FIELD_GET(0, XPCSRX, RX_DPORT_DATA2, XPCSRXDPDATA2, reg_rx_dport_data2);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xpcsrx_rx_dport_acc_set(bdmf_boolean cfgxpcsrxidleramdpsel, bdmf_boolean cfgxpcsrxfecdecramdpsel, bdmf_boolean cfgxpcsrxfecnqueramdpsel)
{
    uint32_t reg_rx_dport_acc=0;

#ifdef VALIDATE_PARMS
    if((cfgxpcsrxidleramdpsel >= _1BITS_MAX_VAL_) ||
       (cfgxpcsrxfecdecramdpsel >= _1BITS_MAX_VAL_) ||
       (cfgxpcsrxfecnqueramdpsel >= _1BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_rx_dport_acc = RU_FIELD_SET(0, XPCSRX, RX_DPORT_ACC, CFGXPCSRXIDLERAMDPSEL, reg_rx_dport_acc, cfgxpcsrxidleramdpsel);
    reg_rx_dport_acc = RU_FIELD_SET(0, XPCSRX, RX_DPORT_ACC, CFGXPCSRXFECDECRAMDPSEL, reg_rx_dport_acc, cfgxpcsrxfecdecramdpsel);
    reg_rx_dport_acc = RU_FIELD_SET(0, XPCSRX, RX_DPORT_ACC, CFGXPCSRXFECNQUERAMDPSEL, reg_rx_dport_acc, cfgxpcsrxfecnqueramdpsel);

    RU_REG_WRITE(0, XPCSRX, RX_DPORT_ACC, reg_rx_dport_acc);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xpcsrx_rx_dport_acc_get(bdmf_boolean *cfgxpcsrxidleramdpsel, bdmf_boolean *cfgxpcsrxfecdecramdpsel, bdmf_boolean *cfgxpcsrxfecnqueramdpsel)
{
    uint32_t reg_rx_dport_acc=0;

#ifdef VALIDATE_PARMS
    if(!cfgxpcsrxidleramdpsel || !cfgxpcsrxfecdecramdpsel || !cfgxpcsrxfecnqueramdpsel)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, XPCSRX, RX_DPORT_ACC, reg_rx_dport_acc);

    *cfgxpcsrxidleramdpsel = RU_FIELD_GET(0, XPCSRX, RX_DPORT_ACC, CFGXPCSRXIDLERAMDPSEL, reg_rx_dport_acc);
    *cfgxpcsrxfecdecramdpsel = RU_FIELD_GET(0, XPCSRX, RX_DPORT_ACC, CFGXPCSRXFECDECRAMDPSEL, reg_rx_dport_acc);
    *cfgxpcsrxfecnqueramdpsel = RU_FIELD_GET(0, XPCSRX, RX_DPORT_ACC, CFGXPCSRXFECNQUERAMDPSEL, reg_rx_dport_acc);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xpcsrx_rx_ram_ecc_int_stat_set(bdmf_boolean intrxidleraminitdone, bdmf_boolean intrxfecnqueraminitdone, bdmf_boolean intrxfecdecraminitdone)
{
    uint32_t reg_rx_ram_ecc_int_stat=0;

#ifdef VALIDATE_PARMS
    if((intrxidleraminitdone >= _1BITS_MAX_VAL_) ||
       (intrxfecnqueraminitdone >= _1BITS_MAX_VAL_) ||
       (intrxfecdecraminitdone >= _1BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_rx_ram_ecc_int_stat = RU_FIELD_SET(0, XPCSRX, RX_RAM_ECC_INT_STAT, INTRXIDLERAMINITDONE, reg_rx_ram_ecc_int_stat, intrxidleraminitdone);
    reg_rx_ram_ecc_int_stat = RU_FIELD_SET(0, XPCSRX, RX_RAM_ECC_INT_STAT, INTRXFECNQUERAMINITDONE, reg_rx_ram_ecc_int_stat, intrxfecnqueraminitdone);
    reg_rx_ram_ecc_int_stat = RU_FIELD_SET(0, XPCSRX, RX_RAM_ECC_INT_STAT, INTRXFECDECRAMINITDONE, reg_rx_ram_ecc_int_stat, intrxfecdecraminitdone);

    RU_REG_WRITE(0, XPCSRX, RX_RAM_ECC_INT_STAT, reg_rx_ram_ecc_int_stat);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xpcsrx_rx_ram_ecc_int_stat_get(bdmf_boolean *intrxidleraminitdone, bdmf_boolean *intrxfecnqueraminitdone, bdmf_boolean *intrxfecdecraminitdone)
{
    uint32_t reg_rx_ram_ecc_int_stat=0;

#ifdef VALIDATE_PARMS
    if(!intrxidleraminitdone || !intrxfecnqueraminitdone || !intrxfecdecraminitdone)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, XPCSRX, RX_RAM_ECC_INT_STAT, reg_rx_ram_ecc_int_stat);

    *intrxidleraminitdone = RU_FIELD_GET(0, XPCSRX, RX_RAM_ECC_INT_STAT, INTRXIDLERAMINITDONE, reg_rx_ram_ecc_int_stat);
    *intrxfecnqueraminitdone = RU_FIELD_GET(0, XPCSRX, RX_RAM_ECC_INT_STAT, INTRXFECNQUERAMINITDONE, reg_rx_ram_ecc_int_stat);
    *intrxfecdecraminitdone = RU_FIELD_GET(0, XPCSRX, RX_RAM_ECC_INT_STAT, INTRXFECDECRAMINITDONE, reg_rx_ram_ecc_int_stat);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xpcsrx_rx_ram_ecc_int_msk_set(bdmf_boolean mskrxidleraminitdone, bdmf_boolean mskrxfecnqueraminitdone, bdmf_boolean mskrxfecdecraminitdone)
{
    uint32_t reg_rx_ram_ecc_int_msk=0;

#ifdef VALIDATE_PARMS
    if((mskrxidleraminitdone >= _1BITS_MAX_VAL_) ||
       (mskrxfecnqueraminitdone >= _1BITS_MAX_VAL_) ||
       (mskrxfecdecraminitdone >= _1BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_rx_ram_ecc_int_msk = RU_FIELD_SET(0, XPCSRX, RX_RAM_ECC_INT_MSK, MSKRXIDLERAMINITDONE, reg_rx_ram_ecc_int_msk, mskrxidleraminitdone);
    reg_rx_ram_ecc_int_msk = RU_FIELD_SET(0, XPCSRX, RX_RAM_ECC_INT_MSK, MSKRXFECNQUERAMINITDONE, reg_rx_ram_ecc_int_msk, mskrxfecnqueraminitdone);
    reg_rx_ram_ecc_int_msk = RU_FIELD_SET(0, XPCSRX, RX_RAM_ECC_INT_MSK, MSKRXFECDECRAMINITDONE, reg_rx_ram_ecc_int_msk, mskrxfecdecraminitdone);

    RU_REG_WRITE(0, XPCSRX, RX_RAM_ECC_INT_MSK, reg_rx_ram_ecc_int_msk);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xpcsrx_rx_ram_ecc_int_msk_get(bdmf_boolean *mskrxidleraminitdone, bdmf_boolean *mskrxfecnqueraminitdone, bdmf_boolean *mskrxfecdecraminitdone)
{
    uint32_t reg_rx_ram_ecc_int_msk=0;

#ifdef VALIDATE_PARMS
    if(!mskrxidleraminitdone || !mskrxfecnqueraminitdone || !mskrxfecdecraminitdone)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, XPCSRX, RX_RAM_ECC_INT_MSK, reg_rx_ram_ecc_int_msk);

    *mskrxidleraminitdone = RU_FIELD_GET(0, XPCSRX, RX_RAM_ECC_INT_MSK, MSKRXIDLERAMINITDONE, reg_rx_ram_ecc_int_msk);
    *mskrxfecnqueraminitdone = RU_FIELD_GET(0, XPCSRX, RX_RAM_ECC_INT_MSK, MSKRXFECNQUERAMINITDONE, reg_rx_ram_ecc_int_msk);
    *mskrxfecdecraminitdone = RU_FIELD_GET(0, XPCSRX, RX_RAM_ECC_INT_MSK, MSKRXFECDECRAMINITDONE, reg_rx_ram_ecc_int_msk);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xpcsrx_rx_dft_testmode_set(uint16_t tm_pd)
{
    uint32_t reg_rx_dft_testmode=0;

#ifdef VALIDATE_PARMS
    if((tm_pd >= _10BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_rx_dft_testmode = RU_FIELD_SET(0, XPCSRX, RX_DFT_TESTMODE, TM_PD, reg_rx_dft_testmode, tm_pd);

    RU_REG_WRITE(0, XPCSRX, RX_DFT_TESTMODE, reg_rx_dft_testmode);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xpcsrx_rx_dft_testmode_get(uint16_t *tm_pd)
{
    uint32_t reg_rx_dft_testmode=0;

#ifdef VALIDATE_PARMS
    if(!tm_pd)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, XPCSRX, RX_DFT_TESTMODE, reg_rx_dft_testmode);

    *tm_pd = RU_FIELD_GET(0, XPCSRX, RX_DFT_TESTMODE, TM_PD, reg_rx_dft_testmode);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xpcsrx_rx_ram_power_pda_ctl0_set(bdmf_boolean cfgxpcsrxidlerampda, bdmf_boolean cfgxpcsrxfecdecrampda)
{
    uint32_t reg_rx_ram_power_pda_ctl0=0;

#ifdef VALIDATE_PARMS
    if((cfgxpcsrxidlerampda >= _1BITS_MAX_VAL_) ||
       (cfgxpcsrxfecdecrampda >= _1BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_rx_ram_power_pda_ctl0 = RU_FIELD_SET(0, XPCSRX, RX_RAM_POWER_PDA_CTL0, CFGXPCSRXIDLERAMPDA, reg_rx_ram_power_pda_ctl0, cfgxpcsrxidlerampda);
    reg_rx_ram_power_pda_ctl0 = RU_FIELD_SET(0, XPCSRX, RX_RAM_POWER_PDA_CTL0, CFGXPCSRXFECDECRAMPDA, reg_rx_ram_power_pda_ctl0, cfgxpcsrxfecdecrampda);

    RU_REG_WRITE(0, XPCSRX, RX_RAM_POWER_PDA_CTL0, reg_rx_ram_power_pda_ctl0);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xpcsrx_rx_ram_power_pda_ctl0_get(bdmf_boolean *cfgxpcsrxidlerampda, bdmf_boolean *cfgxpcsrxfecdecrampda)
{
    uint32_t reg_rx_ram_power_pda_ctl0=0;

#ifdef VALIDATE_PARMS
    if(!cfgxpcsrxidlerampda || !cfgxpcsrxfecdecrampda)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, XPCSRX, RX_RAM_POWER_PDA_CTL0, reg_rx_ram_power_pda_ctl0);

    *cfgxpcsrxidlerampda = RU_FIELD_GET(0, XPCSRX, RX_RAM_POWER_PDA_CTL0, CFGXPCSRXIDLERAMPDA, reg_rx_ram_power_pda_ctl0);
    *cfgxpcsrxfecdecrampda = RU_FIELD_GET(0, XPCSRX, RX_RAM_POWER_PDA_CTL0, CFGXPCSRXFECDECRAMPDA, reg_rx_ram_power_pda_ctl0);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xpcsrx_rx_int_stat1_set(bdmf_boolean intrx64b66btrailstart, bdmf_boolean intrx64b66btwostop, bdmf_boolean intrx64b66btwostart, bdmf_boolean intrx64b66bleadstop)
{
    uint32_t reg_rx_int_stat1=0;

#ifdef VALIDATE_PARMS
    if((intrx64b66btrailstart >= _1BITS_MAX_VAL_) ||
       (intrx64b66btwostop >= _1BITS_MAX_VAL_) ||
       (intrx64b66btwostart >= _1BITS_MAX_VAL_) ||
       (intrx64b66bleadstop >= _1BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_rx_int_stat1 = RU_FIELD_SET(0, XPCSRX, RX_INT_STAT1, INTRX64B66BTRAILSTART, reg_rx_int_stat1, intrx64b66btrailstart);
    reg_rx_int_stat1 = RU_FIELD_SET(0, XPCSRX, RX_INT_STAT1, INTRX64B66BTWOSTOP, reg_rx_int_stat1, intrx64b66btwostop);
    reg_rx_int_stat1 = RU_FIELD_SET(0, XPCSRX, RX_INT_STAT1, INTRX64B66BTWOSTART, reg_rx_int_stat1, intrx64b66btwostart);
    reg_rx_int_stat1 = RU_FIELD_SET(0, XPCSRX, RX_INT_STAT1, INTRX64B66BLEADSTOP, reg_rx_int_stat1, intrx64b66bleadstop);

    RU_REG_WRITE(0, XPCSRX, RX_INT_STAT1, reg_rx_int_stat1);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xpcsrx_rx_int_stat1_get(bdmf_boolean *intrx64b66btrailstart, bdmf_boolean *intrx64b66btwostop, bdmf_boolean *intrx64b66btwostart, bdmf_boolean *intrx64b66bleadstop)
{
    uint32_t reg_rx_int_stat1=0;

#ifdef VALIDATE_PARMS
    if(!intrx64b66btrailstart || !intrx64b66btwostop || !intrx64b66btwostart || !intrx64b66bleadstop)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, XPCSRX, RX_INT_STAT1, reg_rx_int_stat1);

    *intrx64b66btrailstart = RU_FIELD_GET(0, XPCSRX, RX_INT_STAT1, INTRX64B66BTRAILSTART, reg_rx_int_stat1);
    *intrx64b66btwostop = RU_FIELD_GET(0, XPCSRX, RX_INT_STAT1, INTRX64B66BTWOSTOP, reg_rx_int_stat1);
    *intrx64b66btwostart = RU_FIELD_GET(0, XPCSRX, RX_INT_STAT1, INTRX64B66BTWOSTART, reg_rx_int_stat1);
    *intrx64b66bleadstop = RU_FIELD_GET(0, XPCSRX, RX_INT_STAT1, INTRX64B66BLEADSTOP, reg_rx_int_stat1);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xpcsrx_rx_int_msk1_set(bdmf_boolean mskrx64b66btrailstart, bdmf_boolean mskrx64b66btwostop, bdmf_boolean mskrx64b66btwostart, bdmf_boolean mskrx64b66bleadstop)
{
    uint32_t reg_rx_int_msk1=0;

#ifdef VALIDATE_PARMS
    if((mskrx64b66btrailstart >= _1BITS_MAX_VAL_) ||
       (mskrx64b66btwostop >= _1BITS_MAX_VAL_) ||
       (mskrx64b66btwostart >= _1BITS_MAX_VAL_) ||
       (mskrx64b66bleadstop >= _1BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_rx_int_msk1 = RU_FIELD_SET(0, XPCSRX, RX_INT_MSK1, MSKRX64B66BTRAILSTART, reg_rx_int_msk1, mskrx64b66btrailstart);
    reg_rx_int_msk1 = RU_FIELD_SET(0, XPCSRX, RX_INT_MSK1, MSKRX64B66BTWOSTOP, reg_rx_int_msk1, mskrx64b66btwostop);
    reg_rx_int_msk1 = RU_FIELD_SET(0, XPCSRX, RX_INT_MSK1, MSKRX64B66BTWOSTART, reg_rx_int_msk1, mskrx64b66btwostart);
    reg_rx_int_msk1 = RU_FIELD_SET(0, XPCSRX, RX_INT_MSK1, MSKRX64B66BLEADSTOP, reg_rx_int_msk1, mskrx64b66bleadstop);

    RU_REG_WRITE(0, XPCSRX, RX_INT_MSK1, reg_rx_int_msk1);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xpcsrx_rx_int_msk1_get(bdmf_boolean *mskrx64b66btrailstart, bdmf_boolean *mskrx64b66btwostop, bdmf_boolean *mskrx64b66btwostart, bdmf_boolean *mskrx64b66bleadstop)
{
    uint32_t reg_rx_int_msk1=0;

#ifdef VALIDATE_PARMS
    if(!mskrx64b66btrailstart || !mskrx64b66btwostop || !mskrx64b66btwostart || !mskrx64b66bleadstop)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, XPCSRX, RX_INT_MSK1, reg_rx_int_msk1);

    *mskrx64b66btrailstart = RU_FIELD_GET(0, XPCSRX, RX_INT_MSK1, MSKRX64B66BTRAILSTART, reg_rx_int_msk1);
    *mskrx64b66btwostop = RU_FIELD_GET(0, XPCSRX, RX_INT_MSK1, MSKRX64B66BTWOSTOP, reg_rx_int_msk1);
    *mskrx64b66btwostart = RU_FIELD_GET(0, XPCSRX, RX_INT_MSK1, MSKRX64B66BTWOSTART, reg_rx_int_msk1);
    *mskrx64b66bleadstop = RU_FIELD_GET(0, XPCSRX, RX_INT_MSK1, MSKRX64B66BLEADSTOP, reg_rx_int_msk1);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xpcsrx_rx_spare_ctl_set(uint32_t cfgxpcsrxspare)
{
    uint32_t reg_rx_spare_ctl=0;

#ifdef VALIDATE_PARMS
#endif

    reg_rx_spare_ctl = RU_FIELD_SET(0, XPCSRX, RX_SPARE_CTL, CFGXPCSRXSPARE, reg_rx_spare_ctl, cfgxpcsrxspare);

    RU_REG_WRITE(0, XPCSRX, RX_SPARE_CTL, reg_rx_spare_ctl);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xpcsrx_rx_spare_ctl_get(uint32_t *cfgxpcsrxspare)
{
    uint32_t reg_rx_spare_ctl=0;

#ifdef VALIDATE_PARMS
    if(!cfgxpcsrxspare)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, XPCSRX, RX_SPARE_CTL, reg_rx_spare_ctl);

    *cfgxpcsrxspare = RU_FIELD_GET(0, XPCSRX, RX_SPARE_CTL, CFGXPCSRXSPARE, reg_rx_spare_ctl);

    return BDMF_ERR_OK;
}

#ifdef USE_BDMF_SHELL
enum
{
    BDMF_rx_rst,
    BDMF_rx_int_stat,
    BDMF_rx_int_msk,
    BDMF_rx_framer_ctl,
    BDMF_rx_fec_ctl,
    BDMF_rx_dscram_ctl,
    BDMF_rx_64b66b_ctl,
    BDMF_rx_test_ctl,
    BDMF_rx_idle_rd_timer_dly,
    BDMF_rx_idle_gap_siz_max,
    BDMF_rx_framer_lk_max,
    BDMF_rx_framer_unlk_max,
    BDMF_rx_framer_bd_sh,
    BDMF_rx_framer_bd_lo,
    BDMF_rx_framer_bd_hi,
    BDMF_rx_framer_ebd_sh,
    BDMF_rx_framer_ebd_lo,
    BDMF_rx_framer_ebd_hi,
    BDMF_rx_status,
    BDMF_rx_framer_lk_ulk_max,
    BDMF_rx_framer_sp_sh,
    BDMF_rx_framer_sp_lo,
    BDMF_rx_framer_sp_hi,
    BDMF_rx_framer_state,
    BDMF_rx_framer_bd_ebd_ham,
    BDMF_rx_framer_misbrst_cnt,
    BDMF_rx_framer_bd_err,
    BDMF_rx_framer_rogue_ctl,
    BDMF_rx_framer_nolock_ctl,
    BDMF_rx_64b66b_ipg_det_cnt,
    BDMF_rx_fec_nque_in_cnt,
    BDMF_rx_fec_nque_out_cnt,
    BDMF_rx_idle_start_cnt,
    BDMF_rx_idle_stop_cnt,
    BDMF_rx_fec_cor_intval,
    BDMF_rx_fec_cor_tresh,
    BDMF_rx_fec_cw_fail_cnt,
    BDMF_rx_fec_cw_tot_cnt,
    BDMF_rx_fec_correct_cnt_lower,
    BDMF_rx_fec_correct_cnt_upper,
    BDMF_rx_fec_correct_cnt_shadow,
    BDMF_rx_fec_ones_cor_cnt_lower,
    BDMF_rx_fec_ones_cor_cnt_upper,
    BDMF_rx_fec_ones_cor_cnt_shadow,
    BDMF_rx_fec_zeros_cor_cnt_lower,
    BDMF_rx_fec_zeros_cor_cnt_upper,
    BDMF_rx_fec_zeros_cor_cnt_shadow,
    BDMF_rx_fec_stop_on_err_read_pointer,
    BDMF_rx_fec_stop_on_err_burst_location,
    BDMF_rx_64b66b_fail_cnt,
    BDMF_rx_frmr_bad_sh_cnt,
    BDMF_rx_psudo_cnt,
    BDMF_rx_prbs_cnt,
    BDMF_rx_ber_intval,
    BDMF_rx_ber_tresh,
    BDMF_rx_64b66b_start_cnt,
    BDMF_rx_idle_good_pkt_cnt,
    BDMF_rx_idle_err_pkt_cnt,
    BDMF_rx_64b66b_stop_cnt,
    BDMF_rx_burst_out_odr_cnt,
    BDMF_rx_idle_da_jit_dly,
    BDMF_rx_dport_ctl,
    BDMF_rx_dport_data0,
    BDMF_rx_dport_data1,
    BDMF_rx_dport_data2,
    BDMF_rx_dport_acc,
    BDMF_rx_ram_ecc_int_stat,
    BDMF_rx_ram_ecc_int_msk,
    BDMF_rx_dft_testmode,
    BDMF_rx_ram_power_pda_ctl0,
    BDMF_rx_int_stat1,
    BDMF_rx_int_msk1,
    BDMF_rx_spare_ctl,
};

typedef enum
{
    bdmf_address_rx_rst,
    bdmf_address_rx_int_stat,
    bdmf_address_rx_int_msk,
    bdmf_address_rx_framer_ctl,
    bdmf_address_rx_fec_ctl,
    bdmf_address_rx_dscram_ctl,
    bdmf_address_rx_64b66b_ctl,
    bdmf_address_rx_test_ctl,
    bdmf_address_rx_idle_rd_timer_dly,
    bdmf_address_rx_idle_gap_siz_max,
    bdmf_address_rx_framer_lk_max,
    bdmf_address_rx_framer_unlk_max,
    bdmf_address_rx_framer_bd_sh,
    bdmf_address_rx_framer_bd_lo,
    bdmf_address_rx_framer_bd_hi,
    bdmf_address_rx_framer_ebd_sh,
    bdmf_address_rx_framer_ebd_lo,
    bdmf_address_rx_framer_ebd_hi,
    bdmf_address_rx_status,
    bdmf_address_rx_framer_lk_ulk_max,
    bdmf_address_rx_framer_sp_sh,
    bdmf_address_rx_framer_sp_lo,
    bdmf_address_rx_framer_sp_hi,
    bdmf_address_rx_framer_state,
    bdmf_address_rx_framer_bd_ebd_ham,
    bdmf_address_rx_framer_misbrst_cnt,
    bdmf_address_rx_framer_bd_err,
    bdmf_address_rx_framer_rogue_ctl,
    bdmf_address_rx_framer_nolock_ctl,
    bdmf_address_rx_64b66b_ipg_det_cnt,
    bdmf_address_rx_fec_nque_in_cnt,
    bdmf_address_rx_fec_nque_out_cnt,
    bdmf_address_rx_idle_start_cnt,
    bdmf_address_rx_idle_stop_cnt,
    bdmf_address_rx_fec_cor_intval,
    bdmf_address_rx_fec_cor_tresh,
    bdmf_address_rx_fec_cw_fail_cnt,
    bdmf_address_rx_fec_cw_tot_cnt,
    bdmf_address_rx_fec_correct_cnt_lower,
    bdmf_address_rx_fec_correct_cnt_upper,
    bdmf_address_rx_fec_correct_cnt_shadow,
    bdmf_address_rx_fec_ones_cor_cnt_lower,
    bdmf_address_rx_fec_ones_cor_cnt_upper,
    bdmf_address_rx_fec_ones_cor_cnt_shadow,
    bdmf_address_rx_fec_zeros_cor_cnt_lower,
    bdmf_address_rx_fec_zeros_cor_cnt_upper,
    bdmf_address_rx_fec_zeros_cor_cnt_shadow,
    bdmf_address_rx_fec_stop_on_err_read_pointer,
    bdmf_address_rx_fec_stop_on_err_burst_location,
    bdmf_address_rx_64b66b_fail_cnt,
    bdmf_address_rx_frmr_bad_sh_cnt,
    bdmf_address_rx_psudo_cnt,
    bdmf_address_rx_prbs_cnt,
    bdmf_address_rx_ber_intval,
    bdmf_address_rx_ber_tresh,
    bdmf_address_rx_64b66b_start_cnt,
    bdmf_address_rx_idle_good_pkt_cnt,
    bdmf_address_rx_idle_err_pkt_cnt,
    bdmf_address_rx_64b66b_stop_cnt,
    bdmf_address_rx_burst_out_odr_cnt,
    bdmf_address_rx_idle_da_jit_dly,
    bdmf_address_rx_dport_ctl,
    bdmf_address_rx_dport_data0,
    bdmf_address_rx_dport_data1,
    bdmf_address_rx_dport_data2,
    bdmf_address_rx_dport_acc,
    bdmf_address_rx_ram_ecc_int_stat,
    bdmf_address_rx_ram_ecc_int_msk,
    bdmf_address_rx_dft_testmode,
    bdmf_address_rx_ram_power_pda_ctl0,
    bdmf_address_rx_int_stat1,
    bdmf_address_rx_int_msk1,
    bdmf_address_rx_spare_ctl,
}
bdmf_address;

static int bcm_xpcsrx_cli_set(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    bdmf_error_t err;

    switch(parm[0].value.unumber)
    {
    case BDMF_rx_rst:
        err = ag_drv_xpcsrx_rx_rst_set(parm[1].value.unumber);
        break;
    case BDMF_rx_int_stat:
    {
        xpcsrx_rx_int_stat rx_int_stat = { .intrxidledajit=parm[1].value.unumber, .intrxfrmrmisbrst=parm[2].value.unumber, .intrxidlesopeopgapbig=parm[3].value.unumber, .intrxidlefrcins=parm[4].value.unumber, .intrx64b66bminipgerr=parm[5].value.unumber, .intrxfecnquecntneq=parm[6].value.unumber, .intrxidlefifoundrun=parm[7].value.unumber, .intrxidlefifoovrrun=parm[8].value.unumber, .intrxfechighcor=parm[9].value.unumber, .intrxfecdecstoponerr=parm[10].value.unumber, .intrxfecdecpass=parm[11].value.unumber, .intrxstatfrmrhighber=parm[12].value.unumber, .intrxfrmrexitbysp=parm[13].value.unumber, .intrxfrmrbadshmax=parm[14].value.unumber, .intrxdscramburstseqout=parm[15].value.unumber, .intrxtestpsudolock=parm[16].value.unumber, .intrxtestpsudotype=parm[17].value.unumber, .intrxtestpsudoerr=parm[18].value.unumber, .intrxtestprbslock=parm[19].value.unumber, .intrxtestprbserr=parm[20].value.unumber, .intrxfecpsistdecfail=parm[21].value.unumber, .intrxframerbadsh=parm[22].value.unumber, .intrxframercwloss=parm[23].value.unumber, .intrxframercwlock=parm[24].value.unumber, .intrxfecdecfail=parm[25].value.unumber, .intrx64b66bdecerr=parm[26].value.unumber, .intrxfrmrnolocklos=parm[27].value.unumber, .intrxfrmrrogue=parm[28].value.unumber, .int_regs_err=parm[29].value.unumber};
        err = ag_drv_xpcsrx_rx_int_stat_set(&rx_int_stat);
        break;
    }
    case BDMF_rx_int_msk:
    {
        xpcsrx_rx_int_msk rx_int_msk = { .mskrxidledajit=parm[1].value.unumber, .mskrxfrmrmisbrst=parm[2].value.unumber, .mskrxidlesopeopgapbig=parm[3].value.unumber, .mskrxidlefrcins=parm[4].value.unumber, .mskrx64b66bminipgerr=parm[5].value.unumber, .mskrxfecnquecntneq=parm[6].value.unumber, .mskrxidlefifoundrun=parm[7].value.unumber, .mskrxidlefifoovrrun=parm[8].value.unumber, .mskrxfechighcor=parm[9].value.unumber, .mskrxfecdecstoponerr=parm[10].value.unumber, .mskrxfecdecpass=parm[11].value.unumber, .mskrxstatfrmrhighber=parm[12].value.unumber, .mskrxfrmrexitbysp=parm[13].value.unumber, .mskrxfrmrbadshmax=parm[14].value.unumber, .mskrxdscramburstseqout=parm[15].value.unumber, .mskrxtestpsudolock=parm[16].value.unumber, .mskrxtestpsudotype=parm[17].value.unumber, .mskrxtestpsudoerr=parm[18].value.unumber, .mskrxtestprbslock=parm[19].value.unumber, .mskrxtestprbserr=parm[20].value.unumber, .mskrxfecpsistdecfail=parm[21].value.unumber, .mskrxframerbadsh=parm[22].value.unumber, .mskrxframercwloss=parm[23].value.unumber, .mskrxframercwlock=parm[24].value.unumber, .mskrxfecdecfail=parm[25].value.unumber, .mskrx64b66bdecerr=parm[26].value.unumber, .mskrxfrmrnolocklos=parm[27].value.unumber, .mskrxfrmrrogue=parm[28].value.unumber, .msk_int_regs_err=parm[29].value.unumber};
        err = ag_drv_xpcsrx_rx_int_msk_set(&rx_int_msk);
        break;
    }
    case BDMF_rx_framer_ctl:
    {
        xpcsrx_rx_framer_ctl rx_framer_ctl = { .cfgxpcsrxfrmrfrcearlyalign=parm[1].value.unumber, .cfgxpcsrxfrmrmodea=parm[2].value.unumber, .cfgxpcsrxfrmroverlapbdebdzero=parm[3].value.unumber, .cfgxpcsrxfrmroverlapgnten=parm[4].value.unumber, .cfgxpcsrxframeburstoldalign=parm[5].value.unumber, .cfgxpcsrxfrmrmisbrsttype=parm[6].value.unumber, .cfgxpcsrxfrmrebdvlden=parm[7].value.unumber, .cfgxpcsrxfrmrbdcnten=parm[8].value.unumber, .cfgxpcsrxfrmrburstbadshen=parm[9].value.unumber, .cfgxpcsrxfrmrspulken=parm[10].value.unumber, .cfgxpcsrxframeburst=parm[11].value.unumber, .cfgxpcsrxfrmren=parm[12].value.unumber, .cfgxpcsrxfrmrblkfecfail=parm[13].value.unumber, .cfgxpcsrxframefec=parm[14].value.unumber};
        err = ag_drv_xpcsrx_rx_framer_ctl_set(&rx_framer_ctl);
        break;
    }
    case BDMF_rx_fec_ctl:
    {
        xpcsrx_rx_fec_ctl rx_fec_ctl = { .cfgxpcsrxfecstoponerr=parm[1].value.unumber, .cfgxpcsrxfeccntnquecw=parm[2].value.unumber, .cfgxpcsrxfecnquerst=parm[3].value.unumber, .cfgxpcsrxfeconezeromode=parm[4].value.unumber, .cfgxpcsrxfecblkcorrect=parm[5].value.unumber, .cfgxpcsrxfecnquetestpat=parm[6].value.unumber, .cfgxpcsrxfecfailblksh0=parm[7].value.unumber, .cfgxpcsrxfecstrip=parm[8].value.unumber, .cfgxpcsrxfecbypas=parm[9].value.unumber, .cfgxpcsrxfecidleins=parm[10].value.unumber, .cfgxpcsrxfecen=parm[11].value.unumber};
        err = ag_drv_xpcsrx_rx_fec_ctl_set(&rx_fec_ctl);
        break;
    }
    case BDMF_rx_dscram_ctl:
        err = ag_drv_xpcsrx_rx_dscram_ctl_set(parm[1].value.unumber);
        break;
    case BDMF_rx_64b66b_ctl:
    {
        xpcsrx_rx_64b66b_ctl rx_64b66b_ctl = { .cfgxpcsrx64b66btmask1=parm[1].value.unumber, .cfgxpcsrx64b66btmask0=parm[2].value.unumber, .cfgxpcsrx64b66bsmask1=parm[3].value.unumber, .cfgxpcsrx64b66bsmask0=parm[4].value.unumber, .cfgxpcsrx64b66btdlay=parm[5].value.unumber, .cfgxpcsrx64b66bdecbypas=parm[6].value.unumber};
        err = ag_drv_xpcsrx_rx_64b66b_ctl_set(&rx_64b66b_ctl);
        break;
    }
    case BDMF_rx_test_ctl:
        err = ag_drv_xpcsrx_rx_test_ctl_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case BDMF_rx_idle_rd_timer_dly:
        err = ag_drv_xpcsrx_rx_idle_rd_timer_dly_set(parm[1].value.unumber);
        break;
    case BDMF_rx_idle_gap_siz_max:
        err = ag_drv_xpcsrx_rx_idle_gap_siz_max_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case BDMF_rx_framer_lk_max:
        err = ag_drv_xpcsrx_rx_framer_lk_max_set(parm[1].value.unumber);
        break;
    case BDMF_rx_framer_unlk_max:
        err = ag_drv_xpcsrx_rx_framer_unlk_max_set(parm[1].value.unumber);
        break;
    case BDMF_rx_framer_bd_sh:
        err = ag_drv_xpcsrx_rx_framer_bd_sh_set(parm[1].value.unumber);
        break;
    case BDMF_rx_framer_bd_lo:
        err = ag_drv_xpcsrx_rx_framer_bd_lo_set(parm[1].value.unumber);
        break;
    case BDMF_rx_framer_bd_hi:
        err = ag_drv_xpcsrx_rx_framer_bd_hi_set(parm[1].value.unumber);
        break;
    case BDMF_rx_framer_ebd_sh:
        err = ag_drv_xpcsrx_rx_framer_ebd_sh_set(parm[1].value.unumber);
        break;
    case BDMF_rx_framer_ebd_lo:
        err = ag_drv_xpcsrx_rx_framer_ebd_lo_set(parm[1].value.unumber);
        break;
    case BDMF_rx_framer_ebd_hi:
        err = ag_drv_xpcsrx_rx_framer_ebd_hi_set(parm[1].value.unumber);
        break;
    case BDMF_rx_framer_lk_ulk_max:
        err = ag_drv_xpcsrx_rx_framer_lk_ulk_max_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case BDMF_rx_framer_sp_sh:
        err = ag_drv_xpcsrx_rx_framer_sp_sh_set(parm[1].value.unumber);
        break;
    case BDMF_rx_framer_sp_lo:
        err = ag_drv_xpcsrx_rx_framer_sp_lo_set(parm[1].value.unumber);
        break;
    case BDMF_rx_framer_sp_hi:
        err = ag_drv_xpcsrx_rx_framer_sp_hi_set(parm[1].value.unumber);
        break;
    case BDMF_rx_framer_bd_ebd_ham:
        err = ag_drv_xpcsrx_rx_framer_bd_ebd_ham_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber);
        break;
    case BDMF_rx_framer_misbrst_cnt:
        err = ag_drv_xpcsrx_rx_framer_misbrst_cnt_set(parm[1].value.unumber);
        break;
    case BDMF_rx_framer_rogue_ctl:
        err = ag_drv_xpcsrx_rx_framer_rogue_ctl_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case BDMF_rx_framer_nolock_ctl:
        err = ag_drv_xpcsrx_rx_framer_nolock_ctl_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case BDMF_rx_64b66b_ipg_det_cnt:
        err = ag_drv_xpcsrx_rx_64b66b_ipg_det_cnt_set(parm[1].value.unumber);
        break;
    case BDMF_rx_fec_nque_in_cnt:
        err = ag_drv_xpcsrx_rx_fec_nque_in_cnt_set(parm[1].value.unumber);
        break;
    case BDMF_rx_fec_nque_out_cnt:
        err = ag_drv_xpcsrx_rx_fec_nque_out_cnt_set(parm[1].value.unumber);
        break;
    case BDMF_rx_idle_start_cnt:
        err = ag_drv_xpcsrx_rx_idle_start_cnt_set(parm[1].value.unumber);
        break;
    case BDMF_rx_idle_stop_cnt:
        err = ag_drv_xpcsrx_rx_idle_stop_cnt_set(parm[1].value.unumber);
        break;
    case BDMF_rx_fec_cor_intval:
        err = ag_drv_xpcsrx_rx_fec_cor_intval_set(parm[1].value.unumber);
        break;
    case BDMF_rx_fec_cor_tresh:
        err = ag_drv_xpcsrx_rx_fec_cor_tresh_set(parm[1].value.unumber);
        break;
    case BDMF_rx_fec_cw_fail_cnt:
        err = ag_drv_xpcsrx_rx_fec_cw_fail_cnt_set(parm[1].value.unumber);
        break;
    case BDMF_rx_fec_cw_tot_cnt:
        err = ag_drv_xpcsrx_rx_fec_cw_tot_cnt_set(parm[1].value.unumber);
        break;
    case BDMF_rx_fec_correct_cnt_lower:
        err = ag_drv_xpcsrx_rx_fec_correct_cnt_lower_set(parm[1].value.unumber);
        break;
    case BDMF_rx_fec_correct_cnt_shadow:
        err = ag_drv_xpcsrx_rx_fec_correct_cnt_shadow_set(parm[1].value.unumber);
        break;
    case BDMF_rx_fec_ones_cor_cnt_lower:
        err = ag_drv_xpcsrx_rx_fec_ones_cor_cnt_lower_set(parm[1].value.unumber);
        break;
    case BDMF_rx_fec_ones_cor_cnt_shadow:
        err = ag_drv_xpcsrx_rx_fec_ones_cor_cnt_shadow_set(parm[1].value.unumber);
        break;
    case BDMF_rx_fec_zeros_cor_cnt_lower:
        err = ag_drv_xpcsrx_rx_fec_zeros_cor_cnt_lower_set(parm[1].value.unumber);
        break;
    case BDMF_rx_fec_zeros_cor_cnt_shadow:
        err = ag_drv_xpcsrx_rx_fec_zeros_cor_cnt_shadow_set(parm[1].value.unumber);
        break;
    case BDMF_rx_64b66b_fail_cnt:
        err = ag_drv_xpcsrx_rx_64b66b_fail_cnt_set(parm[1].value.unumber);
        break;
    case BDMF_rx_frmr_bad_sh_cnt:
        err = ag_drv_xpcsrx_rx_frmr_bad_sh_cnt_set(parm[1].value.unumber);
        break;
    case BDMF_rx_psudo_cnt:
        err = ag_drv_xpcsrx_rx_psudo_cnt_set(parm[1].value.unumber);
        break;
    case BDMF_rx_prbs_cnt:
        err = ag_drv_xpcsrx_rx_prbs_cnt_set(parm[1].value.unumber);
        break;
    case BDMF_rx_ber_intval:
        err = ag_drv_xpcsrx_rx_ber_intval_set(parm[1].value.unumber);
        break;
    case BDMF_rx_ber_tresh:
        err = ag_drv_xpcsrx_rx_ber_tresh_set(parm[1].value.unumber);
        break;
    case BDMF_rx_64b66b_start_cnt:
        err = ag_drv_xpcsrx_rx_64b66b_start_cnt_set(parm[1].value.unumber);
        break;
    case BDMF_rx_idle_good_pkt_cnt:
        err = ag_drv_xpcsrx_rx_idle_good_pkt_cnt_set(parm[1].value.unumber);
        break;
    case BDMF_rx_idle_err_pkt_cnt:
        err = ag_drv_xpcsrx_rx_idle_err_pkt_cnt_set(parm[1].value.unumber);
        break;
    case BDMF_rx_64b66b_stop_cnt:
        err = ag_drv_xpcsrx_rx_64b66b_stop_cnt_set(parm[1].value.unumber);
        break;
    case BDMF_rx_burst_out_odr_cnt:
        err = ag_drv_xpcsrx_rx_burst_out_odr_cnt_set(parm[1].value.unumber);
        break;
    case BDMF_rx_dport_ctl:
    {
        xpcsrx_rx_dport_ctl rx_dport_ctl = { .xpcsrxdpbusy=parm[1].value.unumber, .xpcsrxdperr=parm[2].value.unumber, .cfgxpcsrxdpctl=parm[3].value.unumber, .cfgxpcsrxdpramsel=parm[4].value.unumber, .cfgxpcsrxdpaddr=parm[5].value.unumber};
        err = ag_drv_xpcsrx_rx_dport_ctl_set(&rx_dport_ctl);
        break;
    }
    case BDMF_rx_dport_data0:
        err = ag_drv_xpcsrx_rx_dport_data0_set(parm[1].value.unumber);
        break;
    case BDMF_rx_dport_data1:
        err = ag_drv_xpcsrx_rx_dport_data1_set(parm[1].value.unumber);
        break;
    case BDMF_rx_dport_data2:
        err = ag_drv_xpcsrx_rx_dport_data2_set(parm[1].value.unumber);
        break;
    case BDMF_rx_dport_acc:
        err = ag_drv_xpcsrx_rx_dport_acc_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber);
        break;
    case BDMF_rx_ram_ecc_int_stat:
        err = ag_drv_xpcsrx_rx_ram_ecc_int_stat_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber);
        break;
    case BDMF_rx_ram_ecc_int_msk:
        err = ag_drv_xpcsrx_rx_ram_ecc_int_msk_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber);
        break;
    case BDMF_rx_dft_testmode:
        err = ag_drv_xpcsrx_rx_dft_testmode_set(parm[1].value.unumber);
        break;
    case BDMF_rx_ram_power_pda_ctl0:
        err = ag_drv_xpcsrx_rx_ram_power_pda_ctl0_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case BDMF_rx_int_stat1:
        err = ag_drv_xpcsrx_rx_int_stat1_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber, parm[4].value.unumber);
        break;
    case BDMF_rx_int_msk1:
        err = ag_drv_xpcsrx_rx_int_msk1_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber, parm[4].value.unumber);
        break;
    case BDMF_rx_spare_ctl:
        err = ag_drv_xpcsrx_rx_spare_ctl_set(parm[1].value.unumber);
        break;
    default:
        err = BDMF_ERR_NOT_SUPPORTED;
        break;
    }
    return err;
}

static int bcm_xpcsrx_cli_get(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    bdmf_error_t err;

    switch(parm[0].value.unumber)
    {
    case BDMF_rx_rst:
    {
        bdmf_boolean cfgxpcsrxclk161rstn;
        err = ag_drv_xpcsrx_rx_rst_get(&cfgxpcsrxclk161rstn);
        bdmf_session_print(session, "cfgxpcsrxclk161rstn = %u = 0x%x\n", cfgxpcsrxclk161rstn, cfgxpcsrxclk161rstn);
        break;
    }
    case BDMF_rx_int_stat:
    {
        xpcsrx_rx_int_stat rx_int_stat;
        err = ag_drv_xpcsrx_rx_int_stat_get(&rx_int_stat);
        bdmf_session_print(session, "intrxidledajit = %u = 0x%x\n", rx_int_stat.intrxidledajit, rx_int_stat.intrxidledajit);
        bdmf_session_print(session, "intrxfrmrmisbrst = %u = 0x%x\n", rx_int_stat.intrxfrmrmisbrst, rx_int_stat.intrxfrmrmisbrst);
        bdmf_session_print(session, "intrxidlesopeopgapbig = %u = 0x%x\n", rx_int_stat.intrxidlesopeopgapbig, rx_int_stat.intrxidlesopeopgapbig);
        bdmf_session_print(session, "intrxidlefrcins = %u = 0x%x\n", rx_int_stat.intrxidlefrcins, rx_int_stat.intrxidlefrcins);
        bdmf_session_print(session, "intrx64b66bminipgerr = %u = 0x%x\n", rx_int_stat.intrx64b66bminipgerr, rx_int_stat.intrx64b66bminipgerr);
        bdmf_session_print(session, "intrxfecnquecntneq = %u = 0x%x\n", rx_int_stat.intrxfecnquecntneq, rx_int_stat.intrxfecnquecntneq);
        bdmf_session_print(session, "intrxidlefifoundrun = %u = 0x%x\n", rx_int_stat.intrxidlefifoundrun, rx_int_stat.intrxidlefifoundrun);
        bdmf_session_print(session, "intrxidlefifoovrrun = %u = 0x%x\n", rx_int_stat.intrxidlefifoovrrun, rx_int_stat.intrxidlefifoovrrun);
        bdmf_session_print(session, "intrxfechighcor = %u = 0x%x\n", rx_int_stat.intrxfechighcor, rx_int_stat.intrxfechighcor);
        bdmf_session_print(session, "intrxfecdecstoponerr = %u = 0x%x\n", rx_int_stat.intrxfecdecstoponerr, rx_int_stat.intrxfecdecstoponerr);
        bdmf_session_print(session, "intrxfecdecpass = %u = 0x%x\n", rx_int_stat.intrxfecdecpass, rx_int_stat.intrxfecdecpass);
        bdmf_session_print(session, "intrxstatfrmrhighber = %u = 0x%x\n", rx_int_stat.intrxstatfrmrhighber, rx_int_stat.intrxstatfrmrhighber);
        bdmf_session_print(session, "intrxfrmrexitbysp = %u = 0x%x\n", rx_int_stat.intrxfrmrexitbysp, rx_int_stat.intrxfrmrexitbysp);
        bdmf_session_print(session, "intrxfrmrbadshmax = %u = 0x%x\n", rx_int_stat.intrxfrmrbadshmax, rx_int_stat.intrxfrmrbadshmax);
        bdmf_session_print(session, "intrxdscramburstseqout = %u = 0x%x\n", rx_int_stat.intrxdscramburstseqout, rx_int_stat.intrxdscramburstseqout);
        bdmf_session_print(session, "intrxtestpsudolock = %u = 0x%x\n", rx_int_stat.intrxtestpsudolock, rx_int_stat.intrxtestpsudolock);
        bdmf_session_print(session, "intrxtestpsudotype = %u = 0x%x\n", rx_int_stat.intrxtestpsudotype, rx_int_stat.intrxtestpsudotype);
        bdmf_session_print(session, "intrxtestpsudoerr = %u = 0x%x\n", rx_int_stat.intrxtestpsudoerr, rx_int_stat.intrxtestpsudoerr);
        bdmf_session_print(session, "intrxtestprbslock = %u = 0x%x\n", rx_int_stat.intrxtestprbslock, rx_int_stat.intrxtestprbslock);
        bdmf_session_print(session, "intrxtestprbserr = %u = 0x%x\n", rx_int_stat.intrxtestprbserr, rx_int_stat.intrxtestprbserr);
        bdmf_session_print(session, "intrxfecpsistdecfail = %u = 0x%x\n", rx_int_stat.intrxfecpsistdecfail, rx_int_stat.intrxfecpsistdecfail);
        bdmf_session_print(session, "intrxframerbadsh = %u = 0x%x\n", rx_int_stat.intrxframerbadsh, rx_int_stat.intrxframerbadsh);
        bdmf_session_print(session, "intrxframercwloss = %u = 0x%x\n", rx_int_stat.intrxframercwloss, rx_int_stat.intrxframercwloss);
        bdmf_session_print(session, "intrxframercwlock = %u = 0x%x\n", rx_int_stat.intrxframercwlock, rx_int_stat.intrxframercwlock);
        bdmf_session_print(session, "intrxfecdecfail = %u = 0x%x\n", rx_int_stat.intrxfecdecfail, rx_int_stat.intrxfecdecfail);
        bdmf_session_print(session, "intrx64b66bdecerr = %u = 0x%x\n", rx_int_stat.intrx64b66bdecerr, rx_int_stat.intrx64b66bdecerr);
        bdmf_session_print(session, "intrxfrmrnolocklos = %u = 0x%x\n", rx_int_stat.intrxfrmrnolocklos, rx_int_stat.intrxfrmrnolocklos);
        bdmf_session_print(session, "intrxfrmrrogue = %u = 0x%x\n", rx_int_stat.intrxfrmrrogue, rx_int_stat.intrxfrmrrogue);
        bdmf_session_print(session, "int_regs_err = %u = 0x%x\n", rx_int_stat.int_regs_err, rx_int_stat.int_regs_err);
        break;
    }
    case BDMF_rx_int_msk:
    {
        xpcsrx_rx_int_msk rx_int_msk;
        err = ag_drv_xpcsrx_rx_int_msk_get(&rx_int_msk);
        bdmf_session_print(session, "mskrxidledajit = %u = 0x%x\n", rx_int_msk.mskrxidledajit, rx_int_msk.mskrxidledajit);
        bdmf_session_print(session, "mskrxfrmrmisbrst = %u = 0x%x\n", rx_int_msk.mskrxfrmrmisbrst, rx_int_msk.mskrxfrmrmisbrst);
        bdmf_session_print(session, "mskrxidlesopeopgapbig = %u = 0x%x\n", rx_int_msk.mskrxidlesopeopgapbig, rx_int_msk.mskrxidlesopeopgapbig);
        bdmf_session_print(session, "mskrxidlefrcins = %u = 0x%x\n", rx_int_msk.mskrxidlefrcins, rx_int_msk.mskrxidlefrcins);
        bdmf_session_print(session, "mskrx64b66bminipgerr = %u = 0x%x\n", rx_int_msk.mskrx64b66bminipgerr, rx_int_msk.mskrx64b66bminipgerr);
        bdmf_session_print(session, "mskrxfecnquecntneq = %u = 0x%x\n", rx_int_msk.mskrxfecnquecntneq, rx_int_msk.mskrxfecnquecntneq);
        bdmf_session_print(session, "mskrxidlefifoundrun = %u = 0x%x\n", rx_int_msk.mskrxidlefifoundrun, rx_int_msk.mskrxidlefifoundrun);
        bdmf_session_print(session, "mskrxidlefifoovrrun = %u = 0x%x\n", rx_int_msk.mskrxidlefifoovrrun, rx_int_msk.mskrxidlefifoovrrun);
        bdmf_session_print(session, "mskrxfechighcor = %u = 0x%x\n", rx_int_msk.mskrxfechighcor, rx_int_msk.mskrxfechighcor);
        bdmf_session_print(session, "mskrxfecdecstoponerr = %u = 0x%x\n", rx_int_msk.mskrxfecdecstoponerr, rx_int_msk.mskrxfecdecstoponerr);
        bdmf_session_print(session, "mskrxfecdecpass = %u = 0x%x\n", rx_int_msk.mskrxfecdecpass, rx_int_msk.mskrxfecdecpass);
        bdmf_session_print(session, "mskrxstatfrmrhighber = %u = 0x%x\n", rx_int_msk.mskrxstatfrmrhighber, rx_int_msk.mskrxstatfrmrhighber);
        bdmf_session_print(session, "mskrxfrmrexitbysp = %u = 0x%x\n", rx_int_msk.mskrxfrmrexitbysp, rx_int_msk.mskrxfrmrexitbysp);
        bdmf_session_print(session, "mskrxfrmrbadshmax = %u = 0x%x\n", rx_int_msk.mskrxfrmrbadshmax, rx_int_msk.mskrxfrmrbadshmax);
        bdmf_session_print(session, "mskrxdscramburstseqout = %u = 0x%x\n", rx_int_msk.mskrxdscramburstseqout, rx_int_msk.mskrxdscramburstseqout);
        bdmf_session_print(session, "mskrxtestpsudolock = %u = 0x%x\n", rx_int_msk.mskrxtestpsudolock, rx_int_msk.mskrxtestpsudolock);
        bdmf_session_print(session, "mskrxtestpsudotype = %u = 0x%x\n", rx_int_msk.mskrxtestpsudotype, rx_int_msk.mskrxtestpsudotype);
        bdmf_session_print(session, "mskrxtestpsudoerr = %u = 0x%x\n", rx_int_msk.mskrxtestpsudoerr, rx_int_msk.mskrxtestpsudoerr);
        bdmf_session_print(session, "mskrxtestprbslock = %u = 0x%x\n", rx_int_msk.mskrxtestprbslock, rx_int_msk.mskrxtestprbslock);
        bdmf_session_print(session, "mskrxtestprbserr = %u = 0x%x\n", rx_int_msk.mskrxtestprbserr, rx_int_msk.mskrxtestprbserr);
        bdmf_session_print(session, "mskrxfecpsistdecfail = %u = 0x%x\n", rx_int_msk.mskrxfecpsistdecfail, rx_int_msk.mskrxfecpsistdecfail);
        bdmf_session_print(session, "mskrxframerbadsh = %u = 0x%x\n", rx_int_msk.mskrxframerbadsh, rx_int_msk.mskrxframerbadsh);
        bdmf_session_print(session, "mskrxframercwloss = %u = 0x%x\n", rx_int_msk.mskrxframercwloss, rx_int_msk.mskrxframercwloss);
        bdmf_session_print(session, "mskrxframercwlock = %u = 0x%x\n", rx_int_msk.mskrxframercwlock, rx_int_msk.mskrxframercwlock);
        bdmf_session_print(session, "mskrxfecdecfail = %u = 0x%x\n", rx_int_msk.mskrxfecdecfail, rx_int_msk.mskrxfecdecfail);
        bdmf_session_print(session, "mskrx64b66bdecerr = %u = 0x%x\n", rx_int_msk.mskrx64b66bdecerr, rx_int_msk.mskrx64b66bdecerr);
        bdmf_session_print(session, "mskrxfrmrnolocklos = %u = 0x%x\n", rx_int_msk.mskrxfrmrnolocklos, rx_int_msk.mskrxfrmrnolocklos);
        bdmf_session_print(session, "mskrxfrmrrogue = %u = 0x%x\n", rx_int_msk.mskrxfrmrrogue, rx_int_msk.mskrxfrmrrogue);
        bdmf_session_print(session, "msk_int_regs_err = %u = 0x%x\n", rx_int_msk.msk_int_regs_err, rx_int_msk.msk_int_regs_err);
        break;
    }
    case BDMF_rx_framer_ctl:
    {
        xpcsrx_rx_framer_ctl rx_framer_ctl;
        err = ag_drv_xpcsrx_rx_framer_ctl_get(&rx_framer_ctl);
        bdmf_session_print(session, "cfgxpcsrxfrmrfrcearlyalign = %u = 0x%x\n", rx_framer_ctl.cfgxpcsrxfrmrfrcearlyalign, rx_framer_ctl.cfgxpcsrxfrmrfrcearlyalign);
        bdmf_session_print(session, "cfgxpcsrxfrmrmodea = %u = 0x%x\n", rx_framer_ctl.cfgxpcsrxfrmrmodea, rx_framer_ctl.cfgxpcsrxfrmrmodea);
        bdmf_session_print(session, "cfgxpcsrxfrmroverlapbdebdzero = %u = 0x%x\n", rx_framer_ctl.cfgxpcsrxfrmroverlapbdebdzero, rx_framer_ctl.cfgxpcsrxfrmroverlapbdebdzero);
        bdmf_session_print(session, "cfgxpcsrxfrmroverlapgnten = %u = 0x%x\n", rx_framer_ctl.cfgxpcsrxfrmroverlapgnten, rx_framer_ctl.cfgxpcsrxfrmroverlapgnten);
        bdmf_session_print(session, "cfgxpcsrxframeburstoldalign = %u = 0x%x\n", rx_framer_ctl.cfgxpcsrxframeburstoldalign, rx_framer_ctl.cfgxpcsrxframeburstoldalign);
        bdmf_session_print(session, "cfgxpcsrxfrmrmisbrsttype = %u = 0x%x\n", rx_framer_ctl.cfgxpcsrxfrmrmisbrsttype, rx_framer_ctl.cfgxpcsrxfrmrmisbrsttype);
        bdmf_session_print(session, "cfgxpcsrxfrmrebdvlden = %u = 0x%x\n", rx_framer_ctl.cfgxpcsrxfrmrebdvlden, rx_framer_ctl.cfgxpcsrxfrmrebdvlden);
        bdmf_session_print(session, "cfgxpcsrxfrmrbdcnten = %u = 0x%x\n", rx_framer_ctl.cfgxpcsrxfrmrbdcnten, rx_framer_ctl.cfgxpcsrxfrmrbdcnten);
        bdmf_session_print(session, "cfgxpcsrxfrmrburstbadshen = %u = 0x%x\n", rx_framer_ctl.cfgxpcsrxfrmrburstbadshen, rx_framer_ctl.cfgxpcsrxfrmrburstbadshen);
        bdmf_session_print(session, "cfgxpcsrxfrmrspulken = %u = 0x%x\n", rx_framer_ctl.cfgxpcsrxfrmrspulken, rx_framer_ctl.cfgxpcsrxfrmrspulken);
        bdmf_session_print(session, "cfgxpcsrxframeburst = %u = 0x%x\n", rx_framer_ctl.cfgxpcsrxframeburst, rx_framer_ctl.cfgxpcsrxframeburst);
        bdmf_session_print(session, "cfgxpcsrxfrmren = %u = 0x%x\n", rx_framer_ctl.cfgxpcsrxfrmren, rx_framer_ctl.cfgxpcsrxfrmren);
        bdmf_session_print(session, "cfgxpcsrxfrmrblkfecfail = %u = 0x%x\n", rx_framer_ctl.cfgxpcsrxfrmrblkfecfail, rx_framer_ctl.cfgxpcsrxfrmrblkfecfail);
        bdmf_session_print(session, "cfgxpcsrxframefec = %u = 0x%x\n", rx_framer_ctl.cfgxpcsrxframefec, rx_framer_ctl.cfgxpcsrxframefec);
        break;
    }
    case BDMF_rx_fec_ctl:
    {
        xpcsrx_rx_fec_ctl rx_fec_ctl;
        err = ag_drv_xpcsrx_rx_fec_ctl_get(&rx_fec_ctl);
        bdmf_session_print(session, "cfgxpcsrxfecstoponerr = %u = 0x%x\n", rx_fec_ctl.cfgxpcsrxfecstoponerr, rx_fec_ctl.cfgxpcsrxfecstoponerr);
        bdmf_session_print(session, "cfgxpcsrxfeccntnquecw = %u = 0x%x\n", rx_fec_ctl.cfgxpcsrxfeccntnquecw, rx_fec_ctl.cfgxpcsrxfeccntnquecw);
        bdmf_session_print(session, "cfgxpcsrxfecnquerst = %u = 0x%x\n", rx_fec_ctl.cfgxpcsrxfecnquerst, rx_fec_ctl.cfgxpcsrxfecnquerst);
        bdmf_session_print(session, "cfgxpcsrxfeconezeromode = %u = 0x%x\n", rx_fec_ctl.cfgxpcsrxfeconezeromode, rx_fec_ctl.cfgxpcsrxfeconezeromode);
        bdmf_session_print(session, "cfgxpcsrxfecblkcorrect = %u = 0x%x\n", rx_fec_ctl.cfgxpcsrxfecblkcorrect, rx_fec_ctl.cfgxpcsrxfecblkcorrect);
        bdmf_session_print(session, "cfgxpcsrxfecnquetestpat = %u = 0x%x\n", rx_fec_ctl.cfgxpcsrxfecnquetestpat, rx_fec_ctl.cfgxpcsrxfecnquetestpat);
        bdmf_session_print(session, "cfgxpcsrxfecfailblksh0 = %u = 0x%x\n", rx_fec_ctl.cfgxpcsrxfecfailblksh0, rx_fec_ctl.cfgxpcsrxfecfailblksh0);
        bdmf_session_print(session, "cfgxpcsrxfecstrip = %u = 0x%x\n", rx_fec_ctl.cfgxpcsrxfecstrip, rx_fec_ctl.cfgxpcsrxfecstrip);
        bdmf_session_print(session, "cfgxpcsrxfecbypas = %u = 0x%x\n", rx_fec_ctl.cfgxpcsrxfecbypas, rx_fec_ctl.cfgxpcsrxfecbypas);
        bdmf_session_print(session, "cfgxpcsrxfecidleins = %u = 0x%x\n", rx_fec_ctl.cfgxpcsrxfecidleins, rx_fec_ctl.cfgxpcsrxfecidleins);
        bdmf_session_print(session, "cfgxpcsrxfecen = %u = 0x%x\n", rx_fec_ctl.cfgxpcsrxfecen, rx_fec_ctl.cfgxpcsrxfecen);
        break;
    }
    case BDMF_rx_dscram_ctl:
    {
        bdmf_boolean cfgxpcsrxdscrambypas;
        err = ag_drv_xpcsrx_rx_dscram_ctl_get(&cfgxpcsrxdscrambypas);
        bdmf_session_print(session, "cfgxpcsrxdscrambypas = %u = 0x%x\n", cfgxpcsrxdscrambypas, cfgxpcsrxdscrambypas);
        break;
    }
    case BDMF_rx_64b66b_ctl:
    {
        xpcsrx_rx_64b66b_ctl rx_64b66b_ctl;
        err = ag_drv_xpcsrx_rx_64b66b_ctl_get(&rx_64b66b_ctl);
        bdmf_session_print(session, "cfgxpcsrx64b66btmask1 = %u = 0x%x\n", rx_64b66b_ctl.cfgxpcsrx64b66btmask1, rx_64b66b_ctl.cfgxpcsrx64b66btmask1);
        bdmf_session_print(session, "cfgxpcsrx64b66btmask0 = %u = 0x%x\n", rx_64b66b_ctl.cfgxpcsrx64b66btmask0, rx_64b66b_ctl.cfgxpcsrx64b66btmask0);
        bdmf_session_print(session, "cfgxpcsrx64b66bsmask1 = %u = 0x%x\n", rx_64b66b_ctl.cfgxpcsrx64b66bsmask1, rx_64b66b_ctl.cfgxpcsrx64b66bsmask1);
        bdmf_session_print(session, "cfgxpcsrx64b66bsmask0 = %u = 0x%x\n", rx_64b66b_ctl.cfgxpcsrx64b66bsmask0, rx_64b66b_ctl.cfgxpcsrx64b66bsmask0);
        bdmf_session_print(session, "cfgxpcsrx64b66btdlay = %u = 0x%x\n", rx_64b66b_ctl.cfgxpcsrx64b66btdlay, rx_64b66b_ctl.cfgxpcsrx64b66btdlay);
        bdmf_session_print(session, "cfgxpcsrx64b66bdecbypas = %u = 0x%x\n", rx_64b66b_ctl.cfgxpcsrx64b66bdecbypas, rx_64b66b_ctl.cfgxpcsrx64b66bdecbypas);
        break;
    }
    case BDMF_rx_test_ctl:
    {
        bdmf_boolean cfgxpcsrxtestprbsdeten;
        bdmf_boolean cfgxpcsrxtestpsudodeten;
        err = ag_drv_xpcsrx_rx_test_ctl_get(&cfgxpcsrxtestprbsdeten, &cfgxpcsrxtestpsudodeten);
        bdmf_session_print(session, "cfgxpcsrxtestprbsdeten = %u = 0x%x\n", cfgxpcsrxtestprbsdeten, cfgxpcsrxtestprbsdeten);
        bdmf_session_print(session, "cfgxpcsrxtestpsudodeten = %u = 0x%x\n", cfgxpcsrxtestpsudodeten, cfgxpcsrxtestpsudodeten);
        break;
    }
    case BDMF_rx_idle_rd_timer_dly:
    {
        uint8_t cfgxpcsrxidlerddelaytimermax;
        err = ag_drv_xpcsrx_rx_idle_rd_timer_dly_get(&cfgxpcsrxidlerddelaytimermax);
        bdmf_session_print(session, "cfgxpcsrxidlerddelaytimermax = %u = 0x%x\n", cfgxpcsrxidlerddelaytimermax, cfgxpcsrxidlerddelaytimermax);
        break;
    }
    case BDMF_rx_idle_gap_siz_max:
    {
        uint16_t cfgxpcsrxidleovrsizmax;
        uint16_t cfgxpcsrxidlesopeopgap;
        err = ag_drv_xpcsrx_rx_idle_gap_siz_max_get(&cfgxpcsrxidleovrsizmax, &cfgxpcsrxidlesopeopgap);
        bdmf_session_print(session, "cfgxpcsrxidleovrsizmax = %u = 0x%x\n", cfgxpcsrxidleovrsizmax, cfgxpcsrxidleovrsizmax);
        bdmf_session_print(session, "cfgxpcsrxidlesopeopgap = %u = 0x%x\n", cfgxpcsrxidlesopeopgap, cfgxpcsrxidlesopeopgap);
        break;
    }
    case BDMF_rx_framer_lk_max:
    {
        uint16_t cfgxpcsrxfrmrcwlktimermax;
        err = ag_drv_xpcsrx_rx_framer_lk_max_get(&cfgxpcsrxfrmrcwlktimermax);
        bdmf_session_print(session, "cfgxpcsrxfrmrcwlktimermax = %u = 0x%x\n", cfgxpcsrxfrmrcwlktimermax, cfgxpcsrxfrmrcwlktimermax);
        break;
    }
    case BDMF_rx_framer_unlk_max:
    {
        uint16_t cfgxpcsrxfrmrcwunlktimermax;
        err = ag_drv_xpcsrx_rx_framer_unlk_max_get(&cfgxpcsrxfrmrcwunlktimermax);
        bdmf_session_print(session, "cfgxpcsrxfrmrcwunlktimermax = %u = 0x%x\n", cfgxpcsrxfrmrcwunlktimermax, cfgxpcsrxfrmrcwunlktimermax);
        break;
    }
    case BDMF_rx_framer_bd_sh:
    {
        uint8_t cfgxpcsrxoltbdsh;
        err = ag_drv_xpcsrx_rx_framer_bd_sh_get(&cfgxpcsrxoltbdsh);
        bdmf_session_print(session, "cfgxpcsrxoltbdsh = %u = 0x%x\n", cfgxpcsrxoltbdsh, cfgxpcsrxoltbdsh);
        break;
    }
    case BDMF_rx_framer_bd_lo:
    {
        uint32_t cfgxpcsrxoltbdlo;
        err = ag_drv_xpcsrx_rx_framer_bd_lo_get(&cfgxpcsrxoltbdlo);
        bdmf_session_print(session, "cfgxpcsrxoltbdlo = %u = 0x%x\n", cfgxpcsrxoltbdlo, cfgxpcsrxoltbdlo);
        break;
    }
    case BDMF_rx_framer_bd_hi:
    {
        uint32_t cfgxpcsrxoltbdhi;
        err = ag_drv_xpcsrx_rx_framer_bd_hi_get(&cfgxpcsrxoltbdhi);
        bdmf_session_print(session, "cfgxpcsrxoltbdhi = %u = 0x%x\n", cfgxpcsrxoltbdhi, cfgxpcsrxoltbdhi);
        break;
    }
    case BDMF_rx_framer_ebd_sh:
    {
        uint8_t cfgxpcsrxoltebdsh;
        err = ag_drv_xpcsrx_rx_framer_ebd_sh_get(&cfgxpcsrxoltebdsh);
        bdmf_session_print(session, "cfgxpcsrxoltebdsh = %u = 0x%x\n", cfgxpcsrxoltebdsh, cfgxpcsrxoltebdsh);
        break;
    }
    case BDMF_rx_framer_ebd_lo:
    {
        uint32_t cfgxpcsrxoltebdlo;
        err = ag_drv_xpcsrx_rx_framer_ebd_lo_get(&cfgxpcsrxoltebdlo);
        bdmf_session_print(session, "cfgxpcsrxoltebdlo = %u = 0x%x\n", cfgxpcsrxoltebdlo, cfgxpcsrxoltebdlo);
        break;
    }
    case BDMF_rx_framer_ebd_hi:
    {
        uint32_t cfgxpcsrxoltebdhi;
        err = ag_drv_xpcsrx_rx_framer_ebd_hi_get(&cfgxpcsrxoltebdhi);
        bdmf_session_print(session, "cfgxpcsrxoltebdhi = %u = 0x%x\n", cfgxpcsrxoltebdhi, cfgxpcsrxoltebdhi);
        break;
    }
    case BDMF_rx_status:
    {
        xpcsrx_rx_status rx_status;
        err = ag_drv_xpcsrx_rx_status_get(&rx_status);
        bdmf_session_print(session, "statrxidledajit = %u = 0x%x\n", rx_status.statrxidledajit, rx_status.statrxidledajit);
        bdmf_session_print(session, "statrxfrmrmisbrst = %u = 0x%x\n", rx_status.statrxfrmrmisbrst, rx_status.statrxfrmrmisbrst);
        bdmf_session_print(session, "statrxidlesopeopgapbig = %u = 0x%x\n", rx_status.statrxidlesopeopgapbig, rx_status.statrxidlesopeopgapbig);
        bdmf_session_print(session, "statrxidlefrcins = %u = 0x%x\n", rx_status.statrxidlefrcins, rx_status.statrxidlefrcins);
        bdmf_session_print(session, "statrx64b66bminipgerr = %u = 0x%x\n", rx_status.statrx64b66bminipgerr, rx_status.statrx64b66bminipgerr);
        bdmf_session_print(session, "statrxfecnquecntneq = %u = 0x%x\n", rx_status.statrxfecnquecntneq, rx_status.statrxfecnquecntneq);
        bdmf_session_print(session, "statrxidlefifoundrun = %u = 0x%x\n", rx_status.statrxidlefifoundrun, rx_status.statrxidlefifoundrun);
        bdmf_session_print(session, "statrxidlefifoovrrun = %u = 0x%x\n", rx_status.statrxidlefifoovrrun, rx_status.statrxidlefifoovrrun);
        bdmf_session_print(session, "statrxfechighcor = %u = 0x%x\n", rx_status.statrxfechighcor, rx_status.statrxfechighcor);
        bdmf_session_print(session, "statrxfecdecpass = %u = 0x%x\n", rx_status.statrxfecdecpass, rx_status.statrxfecdecpass);
        bdmf_session_print(session, "statrxstatfrmrhighber = %u = 0x%x\n", rx_status.statrxstatfrmrhighber, rx_status.statrxstatfrmrhighber);
        bdmf_session_print(session, "statrxfrmrexitbysp = %u = 0x%x\n", rx_status.statrxfrmrexitbysp, rx_status.statrxfrmrexitbysp);
        bdmf_session_print(session, "statrxfrmrbadshmax = %u = 0x%x\n", rx_status.statrxfrmrbadshmax, rx_status.statrxfrmrbadshmax);
        bdmf_session_print(session, "statrxdscramburstseqout = %u = 0x%x\n", rx_status.statrxdscramburstseqout, rx_status.statrxdscramburstseqout);
        bdmf_session_print(session, "statrxtestpsudolock = %u = 0x%x\n", rx_status.statrxtestpsudolock, rx_status.statrxtestpsudolock);
        bdmf_session_print(session, "statrxtestpsudotype = %u = 0x%x\n", rx_status.statrxtestpsudotype, rx_status.statrxtestpsudotype);
        bdmf_session_print(session, "statrxtestpsudoerr = %u = 0x%x\n", rx_status.statrxtestpsudoerr, rx_status.statrxtestpsudoerr);
        bdmf_session_print(session, "statrxtestprbslock = %u = 0x%x\n", rx_status.statrxtestprbslock, rx_status.statrxtestprbslock);
        bdmf_session_print(session, "statrxtestprbserr = %u = 0x%x\n", rx_status.statrxtestprbserr, rx_status.statrxtestprbserr);
        bdmf_session_print(session, "statrxfecpsistdecfail = %u = 0x%x\n", rx_status.statrxfecpsistdecfail, rx_status.statrxfecpsistdecfail);
        bdmf_session_print(session, "statrxframerbadsh = %u = 0x%x\n", rx_status.statrxframerbadsh, rx_status.statrxframerbadsh);
        bdmf_session_print(session, "statrxframercwloss = %u = 0x%x\n", rx_status.statrxframercwloss, rx_status.statrxframercwloss);
        bdmf_session_print(session, "statrxframercwlock = %u = 0x%x\n", rx_status.statrxframercwlock, rx_status.statrxframercwlock);
        bdmf_session_print(session, "statrxfecdecfail = %u = 0x%x\n", rx_status.statrxfecdecfail, rx_status.statrxfecdecfail);
        bdmf_session_print(session, "statrx64b66bdecerr = %u = 0x%x\n", rx_status.statrx64b66bdecerr, rx_status.statrx64b66bdecerr);
        bdmf_session_print(session, "statrxfrmrnolocklos = %u = 0x%x\n", rx_status.statrxfrmrnolocklos, rx_status.statrxfrmrnolocklos);
        bdmf_session_print(session, "statrxfrmrrogue = %u = 0x%x\n", rx_status.statrxfrmrrogue, rx_status.statrxfrmrrogue);
        break;
    }
    case BDMF_rx_framer_lk_ulk_max:
    {
        uint16_t cfgxpcsrxfrmrsplkmax;
        uint16_t cfgxpcsrxfrmrspulkmax;
        err = ag_drv_xpcsrx_rx_framer_lk_ulk_max_get(&cfgxpcsrxfrmrsplkmax, &cfgxpcsrxfrmrspulkmax);
        bdmf_session_print(session, "cfgxpcsrxfrmrsplkmax = %u = 0x%x\n", cfgxpcsrxfrmrsplkmax, cfgxpcsrxfrmrsplkmax);
        bdmf_session_print(session, "cfgxpcsrxfrmrspulkmax = %u = 0x%x\n", cfgxpcsrxfrmrspulkmax, cfgxpcsrxfrmrspulkmax);
        break;
    }
    case BDMF_rx_framer_sp_sh:
    {
        uint8_t cfgxpcsrxoltspsh;
        err = ag_drv_xpcsrx_rx_framer_sp_sh_get(&cfgxpcsrxoltspsh);
        bdmf_session_print(session, "cfgxpcsrxoltspsh = %u = 0x%x\n", cfgxpcsrxoltspsh, cfgxpcsrxoltspsh);
        break;
    }
    case BDMF_rx_framer_sp_lo:
    {
        uint32_t cfgxpcsrxoltsplo;
        err = ag_drv_xpcsrx_rx_framer_sp_lo_get(&cfgxpcsrxoltsplo);
        bdmf_session_print(session, "cfgxpcsrxoltsplo = %u = 0x%x\n", cfgxpcsrxoltsplo, cfgxpcsrxoltsplo);
        break;
    }
    case BDMF_rx_framer_sp_hi:
    {
        uint32_t cfgxpcsrxoltsphi;
        err = ag_drv_xpcsrx_rx_framer_sp_hi_get(&cfgxpcsrxoltsphi);
        bdmf_session_print(session, "cfgxpcsrxoltsphi = %u = 0x%x\n", cfgxpcsrxoltsphi, cfgxpcsrxoltsphi);
        break;
    }
    case BDMF_rx_framer_state:
    {
        uint8_t xpcsrxfrmrstate;
        err = ag_drv_xpcsrx_rx_framer_state_get(&xpcsrxfrmrstate);
        bdmf_session_print(session, "xpcsrxfrmrstate = %u = 0x%x\n", xpcsrxfrmrstate, xpcsrxfrmrstate);
        break;
    }
    case BDMF_rx_framer_bd_ebd_ham:
    {
        uint8_t cfgxpcsrxfrmrspham;
        uint8_t cfgxpcsrxfrmrebdham;
        uint8_t cfgxpcsrxfrmrbdham;
        err = ag_drv_xpcsrx_rx_framer_bd_ebd_ham_get(&cfgxpcsrxfrmrspham, &cfgxpcsrxfrmrebdham, &cfgxpcsrxfrmrbdham);
        bdmf_session_print(session, "cfgxpcsrxfrmrspham = %u = 0x%x\n", cfgxpcsrxfrmrspham, cfgxpcsrxfrmrspham);
        bdmf_session_print(session, "cfgxpcsrxfrmrebdham = %u = 0x%x\n", cfgxpcsrxfrmrebdham, cfgxpcsrxfrmrebdham);
        bdmf_session_print(session, "cfgxpcsrxfrmrbdham = %u = 0x%x\n", cfgxpcsrxfrmrbdham, cfgxpcsrxfrmrbdham);
        break;
    }
    case BDMF_rx_framer_misbrst_cnt:
    {
        uint32_t rxfrmrmisbrstcnt;
        err = ag_drv_xpcsrx_rx_framer_misbrst_cnt_get(&rxfrmrmisbrstcnt);
        bdmf_session_print(session, "rxfrmrmisbrstcnt = %u = 0x%x\n", rxfrmrmisbrstcnt, rxfrmrmisbrstcnt);
        break;
    }
    case BDMF_rx_framer_bd_err:
    {
        uint8_t xpcsrxstatfrmrbderr;
        err = ag_drv_xpcsrx_rx_framer_bd_err_get(&xpcsrxstatfrmrbderr);
        bdmf_session_print(session, "xpcsrxstatfrmrbderr = %u = 0x%x\n", xpcsrxstatfrmrbderr, xpcsrxstatfrmrbderr);
        break;
    }
    case BDMF_rx_framer_rogue_ctl:
    {
        bdmf_boolean cfgxpcsrxfrmrrogueen;
        uint16_t cfgxpcsrxfrmrroguesptresh;
        err = ag_drv_xpcsrx_rx_framer_rogue_ctl_get(&cfgxpcsrxfrmrrogueen, &cfgxpcsrxfrmrroguesptresh);
        bdmf_session_print(session, "cfgxpcsrxfrmrrogueen = %u = 0x%x\n", cfgxpcsrxfrmrrogueen, cfgxpcsrxfrmrrogueen);
        bdmf_session_print(session, "cfgxpcsrxfrmrroguesptresh = %u = 0x%x\n", cfgxpcsrxfrmrroguesptresh, cfgxpcsrxfrmrroguesptresh);
        break;
    }
    case BDMF_rx_framer_nolock_ctl:
    {
        bdmf_boolean cfgxpcsrxfrmrnolocklosen;
        uint32_t cfgxpcsrxfrmrnolocklosintval;
        err = ag_drv_xpcsrx_rx_framer_nolock_ctl_get(&cfgxpcsrxfrmrnolocklosen, &cfgxpcsrxfrmrnolocklosintval);
        bdmf_session_print(session, "cfgxpcsrxfrmrnolocklosen = %u = 0x%x\n", cfgxpcsrxfrmrnolocklosen, cfgxpcsrxfrmrnolocklosen);
        bdmf_session_print(session, "cfgxpcsrxfrmrnolocklosintval = %u = 0x%x\n", cfgxpcsrxfrmrnolocklosintval, cfgxpcsrxfrmrnolocklosintval);
        break;
    }
    case BDMF_rx_64b66b_ipg_det_cnt:
    {
        uint32_t rx64b66bipgdetcnt;
        err = ag_drv_xpcsrx_rx_64b66b_ipg_det_cnt_get(&rx64b66bipgdetcnt);
        bdmf_session_print(session, "rx64b66bipgdetcnt = %u = 0x%x\n", rx64b66bipgdetcnt, rx64b66bipgdetcnt);
        break;
    }
    case BDMF_rx_fec_nque_in_cnt:
    {
        uint32_t rxfecnqueincnt;
        err = ag_drv_xpcsrx_rx_fec_nque_in_cnt_get(&rxfecnqueincnt);
        bdmf_session_print(session, "rxfecnqueincnt = %u = 0x%x\n", rxfecnqueincnt, rxfecnqueincnt);
        break;
    }
    case BDMF_rx_fec_nque_out_cnt:
    {
        uint32_t rxfecnqueoutcnt;
        err = ag_drv_xpcsrx_rx_fec_nque_out_cnt_get(&rxfecnqueoutcnt);
        bdmf_session_print(session, "rxfecnqueoutcnt = %u = 0x%x\n", rxfecnqueoutcnt, rxfecnqueoutcnt);
        break;
    }
    case BDMF_rx_idle_start_cnt:
    {
        uint32_t rxidlestartcnt;
        err = ag_drv_xpcsrx_rx_idle_start_cnt_get(&rxidlestartcnt);
        bdmf_session_print(session, "rxidlestartcnt = %u = 0x%x\n", rxidlestartcnt, rxidlestartcnt);
        break;
    }
    case BDMF_rx_idle_stop_cnt:
    {
        uint32_t rxidlestopcnt;
        err = ag_drv_xpcsrx_rx_idle_stop_cnt_get(&rxidlestopcnt);
        bdmf_session_print(session, "rxidlestopcnt = %u = 0x%x\n", rxidlestopcnt, rxidlestopcnt);
        break;
    }
    case BDMF_rx_fec_cor_intval:
    {
        uint32_t cfgxpcsrxfeccorintval;
        err = ag_drv_xpcsrx_rx_fec_cor_intval_get(&cfgxpcsrxfeccorintval);
        bdmf_session_print(session, "cfgxpcsrxfeccorintval = %u = 0x%x\n", cfgxpcsrxfeccorintval, cfgxpcsrxfeccorintval);
        break;
    }
    case BDMF_rx_fec_cor_tresh:
    {
        uint32_t cfgxpcsrxfeccortresh;
        err = ag_drv_xpcsrx_rx_fec_cor_tresh_get(&cfgxpcsrxfeccortresh);
        bdmf_session_print(session, "cfgxpcsrxfeccortresh = %u = 0x%x\n", cfgxpcsrxfeccortresh, cfgxpcsrxfeccortresh);
        break;
    }
    case BDMF_rx_fec_cw_fail_cnt:
    {
        uint32_t rxfecdeccwfailcnt;
        err = ag_drv_xpcsrx_rx_fec_cw_fail_cnt_get(&rxfecdeccwfailcnt);
        bdmf_session_print(session, "rxfecdeccwfailcnt = %u = 0x%x\n", rxfecdeccwfailcnt, rxfecdeccwfailcnt);
        break;
    }
    case BDMF_rx_fec_cw_tot_cnt:
    {
        uint32_t rxfecdeccwtotcnt;
        err = ag_drv_xpcsrx_rx_fec_cw_tot_cnt_get(&rxfecdeccwtotcnt);
        bdmf_session_print(session, "rxfecdeccwtotcnt = %u = 0x%x\n", rxfecdeccwtotcnt, rxfecdeccwtotcnt);
        break;
    }
    case BDMF_rx_fec_correct_cnt_lower:
    {
        uint32_t rxfecdecerrcorcntlower;
        err = ag_drv_xpcsrx_rx_fec_correct_cnt_lower_get(&rxfecdecerrcorcntlower);
        bdmf_session_print(session, "rxfecdecerrcorcntlower = %u = 0x%x\n", rxfecdecerrcorcntlower, rxfecdecerrcorcntlower);
        break;
    }
    case BDMF_rx_fec_correct_cnt_upper:
    {
        uint8_t rxfecdecerrcorcntupper;
        err = ag_drv_xpcsrx_rx_fec_correct_cnt_upper_get(&rxfecdecerrcorcntupper);
        bdmf_session_print(session, "rxfecdecerrcorcntupper = %u = 0x%x\n", rxfecdecerrcorcntupper, rxfecdecerrcorcntupper);
        break;
    }
    case BDMF_rx_fec_correct_cnt_shadow:
    {
        uint8_t rxfecdecerrcorcntshadow;
        err = ag_drv_xpcsrx_rx_fec_correct_cnt_shadow_get(&rxfecdecerrcorcntshadow);
        bdmf_session_print(session, "rxfecdecerrcorcntshadow = %u = 0x%x\n", rxfecdecerrcorcntshadow, rxfecdecerrcorcntshadow);
        break;
    }
    case BDMF_rx_fec_ones_cor_cnt_lower:
    {
        uint32_t rxfecdeconescorcntlower;
        err = ag_drv_xpcsrx_rx_fec_ones_cor_cnt_lower_get(&rxfecdeconescorcntlower);
        bdmf_session_print(session, "rxfecdeconescorcntlower = %u = 0x%x\n", rxfecdeconescorcntlower, rxfecdeconescorcntlower);
        break;
    }
    case BDMF_rx_fec_ones_cor_cnt_upper:
    {
        uint8_t rxfecdeconescorcntupper;
        err = ag_drv_xpcsrx_rx_fec_ones_cor_cnt_upper_get(&rxfecdeconescorcntupper);
        bdmf_session_print(session, "rxfecdeconescorcntupper = %u = 0x%x\n", rxfecdeconescorcntupper, rxfecdeconescorcntupper);
        break;
    }
    case BDMF_rx_fec_ones_cor_cnt_shadow:
    {
        uint8_t rxfecdeconescorcntshadow;
        err = ag_drv_xpcsrx_rx_fec_ones_cor_cnt_shadow_get(&rxfecdeconescorcntshadow);
        bdmf_session_print(session, "rxfecdeconescorcntshadow = %u = 0x%x\n", rxfecdeconescorcntshadow, rxfecdeconescorcntshadow);
        break;
    }
    case BDMF_rx_fec_zeros_cor_cnt_lower:
    {
        uint32_t rxfecdeczeroscorcntlower;
        err = ag_drv_xpcsrx_rx_fec_zeros_cor_cnt_lower_get(&rxfecdeczeroscorcntlower);
        bdmf_session_print(session, "rxfecdeczeroscorcntlower = %u = 0x%x\n", rxfecdeczeroscorcntlower, rxfecdeczeroscorcntlower);
        break;
    }
    case BDMF_rx_fec_zeros_cor_cnt_upper:
    {
        uint8_t rxfecdeczeroscorcntupper;
        err = ag_drv_xpcsrx_rx_fec_zeros_cor_cnt_upper_get(&rxfecdeczeroscorcntupper);
        bdmf_session_print(session, "rxfecdeczeroscorcntupper = %u = 0x%x\n", rxfecdeczeroscorcntupper, rxfecdeczeroscorcntupper);
        break;
    }
    case BDMF_rx_fec_zeros_cor_cnt_shadow:
    {
        uint8_t rxfecdeczeroscorcntshadow;
        err = ag_drv_xpcsrx_rx_fec_zeros_cor_cnt_shadow_get(&rxfecdeczeroscorcntshadow);
        bdmf_session_print(session, "rxfecdeczeroscorcntshadow = %u = 0x%x\n", rxfecdeczeroscorcntshadow, rxfecdeczeroscorcntshadow);
        break;
    }
    case BDMF_rx_fec_stop_on_err_read_pointer:
    {
        uint8_t rxfecstoponerrrdptr;
        uint8_t rxfecstoponerrwrptr;
        err = ag_drv_xpcsrx_rx_fec_stop_on_err_read_pointer_get(&rxfecstoponerrrdptr, &rxfecstoponerrwrptr);
        bdmf_session_print(session, "rxfecstoponerrrdptr = %u = 0x%x\n", rxfecstoponerrrdptr, rxfecstoponerrrdptr);
        bdmf_session_print(session, "rxfecstoponerrwrptr = %u = 0x%x\n", rxfecstoponerrwrptr, rxfecstoponerrwrptr);
        break;
    }
    case BDMF_rx_fec_stop_on_err_burst_location:
    {
        uint32_t rxfecstoponerrbrstloc;
        err = ag_drv_xpcsrx_rx_fec_stop_on_err_burst_location_get(&rxfecstoponerrbrstloc);
        bdmf_session_print(session, "rxfecstoponerrbrstloc = %u = 0x%x\n", rxfecstoponerrbrstloc, rxfecstoponerrbrstloc);
        break;
    }
    case BDMF_rx_64b66b_fail_cnt:
    {
        uint32_t rx64b66bdecerrcnt;
        err = ag_drv_xpcsrx_rx_64b66b_fail_cnt_get(&rx64b66bdecerrcnt);
        bdmf_session_print(session, "rx64b66bdecerrcnt = %u = 0x%x\n", rx64b66bdecerrcnt, rx64b66bdecerrcnt);
        break;
    }
    case BDMF_rx_frmr_bad_sh_cnt:
    {
        uint32_t rxfrmrbadshcnt;
        err = ag_drv_xpcsrx_rx_frmr_bad_sh_cnt_get(&rxfrmrbadshcnt);
        bdmf_session_print(session, "rxfrmrbadshcnt = %u = 0x%x\n", rxfrmrbadshcnt, rxfrmrbadshcnt);
        break;
    }
    case BDMF_rx_psudo_cnt:
    {
        uint32_t rxtestpsudoerrcnt;
        err = ag_drv_xpcsrx_rx_psudo_cnt_get(&rxtestpsudoerrcnt);
        bdmf_session_print(session, "rxtestpsudoerrcnt = %u = 0x%x\n", rxtestpsudoerrcnt, rxtestpsudoerrcnt);
        break;
    }
    case BDMF_rx_prbs_cnt:
    {
        uint32_t rxtestprbserrcnt;
        err = ag_drv_xpcsrx_rx_prbs_cnt_get(&rxtestprbserrcnt);
        bdmf_session_print(session, "rxtestprbserrcnt = %u = 0x%x\n", rxtestprbserrcnt, rxtestprbserrcnt);
        break;
    }
    case BDMF_rx_ber_intval:
    {
        uint32_t cfgxpcsrxfrmrberintval;
        err = ag_drv_xpcsrx_rx_ber_intval_get(&cfgxpcsrxfrmrberintval);
        bdmf_session_print(session, "cfgxpcsrxfrmrberintval = %u = 0x%x\n", cfgxpcsrxfrmrberintval, cfgxpcsrxfrmrberintval);
        break;
    }
    case BDMF_rx_ber_tresh:
    {
        uint16_t cfgxpcsrxfrmrbertresh;
        err = ag_drv_xpcsrx_rx_ber_tresh_get(&cfgxpcsrxfrmrbertresh);
        bdmf_session_print(session, "cfgxpcsrxfrmrbertresh = %u = 0x%x\n", cfgxpcsrxfrmrbertresh, cfgxpcsrxfrmrbertresh);
        break;
    }
    case BDMF_rx_64b66b_start_cnt:
    {
        uint32_t rx64b66bdecstartcnt;
        err = ag_drv_xpcsrx_rx_64b66b_start_cnt_get(&rx64b66bdecstartcnt);
        bdmf_session_print(session, "rx64b66bdecstartcnt = %u = 0x%x\n", rx64b66bdecstartcnt, rx64b66bdecstartcnt);
        break;
    }
    case BDMF_rx_idle_good_pkt_cnt:
    {
        uint32_t rxidlegoodpktcnt;
        err = ag_drv_xpcsrx_rx_idle_good_pkt_cnt_get(&rxidlegoodpktcnt);
        bdmf_session_print(session, "rxidlegoodpktcnt = %u = 0x%x\n", rxidlegoodpktcnt, rxidlegoodpktcnt);
        break;
    }
    case BDMF_rx_idle_err_pkt_cnt:
    {
        uint32_t rxidleerrpktcnt;
        err = ag_drv_xpcsrx_rx_idle_err_pkt_cnt_get(&rxidleerrpktcnt);
        bdmf_session_print(session, "rxidleerrpktcnt = %u = 0x%x\n", rxidleerrpktcnt, rxidleerrpktcnt);
        break;
    }
    case BDMF_rx_64b66b_stop_cnt:
    {
        uint32_t rx64b66bdecstopcnt;
        err = ag_drv_xpcsrx_rx_64b66b_stop_cnt_get(&rx64b66bdecstopcnt);
        bdmf_session_print(session, "rx64b66bdecstopcnt = %u = 0x%x\n", rx64b66bdecstopcnt, rx64b66bdecstopcnt);
        break;
    }
    case BDMF_rx_burst_out_odr_cnt:
    {
        uint32_t rxburstseqoutofordercnt;
        err = ag_drv_xpcsrx_rx_burst_out_odr_cnt_get(&rxburstseqoutofordercnt);
        bdmf_session_print(session, "rxburstseqoutofordercnt = %u = 0x%x\n", rxburstseqoutofordercnt, rxburstseqoutofordercnt);
        break;
    }
    case BDMF_rx_idle_da_jit_dly:
    {
        uint16_t rxidlelastdacnt;
        uint16_t rxidledacnt;
        err = ag_drv_xpcsrx_rx_idle_da_jit_dly_get(&rxidlelastdacnt, &rxidledacnt);
        bdmf_session_print(session, "rxidlelastdacnt = %u = 0x%x\n", rxidlelastdacnt, rxidlelastdacnt);
        bdmf_session_print(session, "rxidledacnt = %u = 0x%x\n", rxidledacnt, rxidledacnt);
        break;
    }
    case BDMF_rx_dport_ctl:
    {
        xpcsrx_rx_dport_ctl rx_dport_ctl;
        err = ag_drv_xpcsrx_rx_dport_ctl_get(&rx_dport_ctl);
        bdmf_session_print(session, "xpcsrxdpbusy = %u = 0x%x\n", rx_dport_ctl.xpcsrxdpbusy, rx_dport_ctl.xpcsrxdpbusy);
        bdmf_session_print(session, "xpcsrxdperr = %u = 0x%x\n", rx_dport_ctl.xpcsrxdperr, rx_dport_ctl.xpcsrxdperr);
        bdmf_session_print(session, "cfgxpcsrxdpctl = %u = 0x%x\n", rx_dport_ctl.cfgxpcsrxdpctl, rx_dport_ctl.cfgxpcsrxdpctl);
        bdmf_session_print(session, "cfgxpcsrxdpramsel = %u = 0x%x\n", rx_dport_ctl.cfgxpcsrxdpramsel, rx_dport_ctl.cfgxpcsrxdpramsel);
        bdmf_session_print(session, "cfgxpcsrxdpaddr = %u = 0x%x\n", rx_dport_ctl.cfgxpcsrxdpaddr, rx_dport_ctl.cfgxpcsrxdpaddr);
        break;
    }
    case BDMF_rx_dport_data0:
    {
        uint32_t xpcsrxdpdata0;
        err = ag_drv_xpcsrx_rx_dport_data0_get(&xpcsrxdpdata0);
        bdmf_session_print(session, "xpcsrxdpdata0 = %u = 0x%x\n", xpcsrxdpdata0, xpcsrxdpdata0);
        break;
    }
    case BDMF_rx_dport_data1:
    {
        uint32_t xpcsrxdpdata1;
        err = ag_drv_xpcsrx_rx_dport_data1_get(&xpcsrxdpdata1);
        bdmf_session_print(session, "xpcsrxdpdata1 = %u = 0x%x\n", xpcsrxdpdata1, xpcsrxdpdata1);
        break;
    }
    case BDMF_rx_dport_data2:
    {
        uint32_t xpcsrxdpdata2;
        err = ag_drv_xpcsrx_rx_dport_data2_get(&xpcsrxdpdata2);
        bdmf_session_print(session, "xpcsrxdpdata2 = %u = 0x%x\n", xpcsrxdpdata2, xpcsrxdpdata2);
        break;
    }
    case BDMF_rx_dport_acc:
    {
        bdmf_boolean cfgxpcsrxidleramdpsel;
        bdmf_boolean cfgxpcsrxfecdecramdpsel;
        bdmf_boolean cfgxpcsrxfecnqueramdpsel;
        err = ag_drv_xpcsrx_rx_dport_acc_get(&cfgxpcsrxidleramdpsel, &cfgxpcsrxfecdecramdpsel, &cfgxpcsrxfecnqueramdpsel);
        bdmf_session_print(session, "cfgxpcsrxidleramdpsel = %u = 0x%x\n", cfgxpcsrxidleramdpsel, cfgxpcsrxidleramdpsel);
        bdmf_session_print(session, "cfgxpcsrxfecdecramdpsel = %u = 0x%x\n", cfgxpcsrxfecdecramdpsel, cfgxpcsrxfecdecramdpsel);
        bdmf_session_print(session, "cfgxpcsrxfecnqueramdpsel = %u = 0x%x\n", cfgxpcsrxfecnqueramdpsel, cfgxpcsrxfecnqueramdpsel);
        break;
    }
    case BDMF_rx_ram_ecc_int_stat:
    {
        bdmf_boolean intrxidleraminitdone;
        bdmf_boolean intrxfecnqueraminitdone;
        bdmf_boolean intrxfecdecraminitdone;
        err = ag_drv_xpcsrx_rx_ram_ecc_int_stat_get(&intrxidleraminitdone, &intrxfecnqueraminitdone, &intrxfecdecraminitdone);
        bdmf_session_print(session, "intrxidleraminitdone = %u = 0x%x\n", intrxidleraminitdone, intrxidleraminitdone);
        bdmf_session_print(session, "intrxfecnqueraminitdone = %u = 0x%x\n", intrxfecnqueraminitdone, intrxfecnqueraminitdone);
        bdmf_session_print(session, "intrxfecdecraminitdone = %u = 0x%x\n", intrxfecdecraminitdone, intrxfecdecraminitdone);
        break;
    }
    case BDMF_rx_ram_ecc_int_msk:
    {
        bdmf_boolean mskrxidleraminitdone;
        bdmf_boolean mskrxfecnqueraminitdone;
        bdmf_boolean mskrxfecdecraminitdone;
        err = ag_drv_xpcsrx_rx_ram_ecc_int_msk_get(&mskrxidleraminitdone, &mskrxfecnqueraminitdone, &mskrxfecdecraminitdone);
        bdmf_session_print(session, "mskrxidleraminitdone = %u = 0x%x\n", mskrxidleraminitdone, mskrxidleraminitdone);
        bdmf_session_print(session, "mskrxfecnqueraminitdone = %u = 0x%x\n", mskrxfecnqueraminitdone, mskrxfecnqueraminitdone);
        bdmf_session_print(session, "mskrxfecdecraminitdone = %u = 0x%x\n", mskrxfecdecraminitdone, mskrxfecdecraminitdone);
        break;
    }
    case BDMF_rx_dft_testmode:
    {
        uint16_t tm_pd;
        err = ag_drv_xpcsrx_rx_dft_testmode_get(&tm_pd);
        bdmf_session_print(session, "tm_pd = %u = 0x%x\n", tm_pd, tm_pd);
        break;
    }
    case BDMF_rx_ram_power_pda_ctl0:
    {
        bdmf_boolean cfgxpcsrxidlerampda;
        bdmf_boolean cfgxpcsrxfecdecrampda;
        err = ag_drv_xpcsrx_rx_ram_power_pda_ctl0_get(&cfgxpcsrxidlerampda, &cfgxpcsrxfecdecrampda);
        bdmf_session_print(session, "cfgxpcsrxidlerampda = %u = 0x%x\n", cfgxpcsrxidlerampda, cfgxpcsrxidlerampda);
        bdmf_session_print(session, "cfgxpcsrxfecdecrampda = %u = 0x%x\n", cfgxpcsrxfecdecrampda, cfgxpcsrxfecdecrampda);
        break;
    }
    case BDMF_rx_int_stat1:
    {
        bdmf_boolean intrx64b66btrailstart;
        bdmf_boolean intrx64b66btwostop;
        bdmf_boolean intrx64b66btwostart;
        bdmf_boolean intrx64b66bleadstop;
        err = ag_drv_xpcsrx_rx_int_stat1_get(&intrx64b66btrailstart, &intrx64b66btwostop, &intrx64b66btwostart, &intrx64b66bleadstop);
        bdmf_session_print(session, "intrx64b66btrailstart = %u = 0x%x\n", intrx64b66btrailstart, intrx64b66btrailstart);
        bdmf_session_print(session, "intrx64b66btwostop = %u = 0x%x\n", intrx64b66btwostop, intrx64b66btwostop);
        bdmf_session_print(session, "intrx64b66btwostart = %u = 0x%x\n", intrx64b66btwostart, intrx64b66btwostart);
        bdmf_session_print(session, "intrx64b66bleadstop = %u = 0x%x\n", intrx64b66bleadstop, intrx64b66bleadstop);
        break;
    }
    case BDMF_rx_int_msk1:
    {
        bdmf_boolean mskrx64b66btrailstart;
        bdmf_boolean mskrx64b66btwostop;
        bdmf_boolean mskrx64b66btwostart;
        bdmf_boolean mskrx64b66bleadstop;
        err = ag_drv_xpcsrx_rx_int_msk1_get(&mskrx64b66btrailstart, &mskrx64b66btwostop, &mskrx64b66btwostart, &mskrx64b66bleadstop);
        bdmf_session_print(session, "mskrx64b66btrailstart = %u = 0x%x\n", mskrx64b66btrailstart, mskrx64b66btrailstart);
        bdmf_session_print(session, "mskrx64b66btwostop = %u = 0x%x\n", mskrx64b66btwostop, mskrx64b66btwostop);
        bdmf_session_print(session, "mskrx64b66btwostart = %u = 0x%x\n", mskrx64b66btwostart, mskrx64b66btwostart);
        bdmf_session_print(session, "mskrx64b66bleadstop = %u = 0x%x\n", mskrx64b66bleadstop, mskrx64b66bleadstop);
        break;
    }
    case BDMF_rx_spare_ctl:
    {
        uint32_t cfgxpcsrxspare;
        err = ag_drv_xpcsrx_rx_spare_ctl_get(&cfgxpcsrxspare);
        bdmf_session_print(session, "cfgxpcsrxspare = %u = 0x%x\n", cfgxpcsrxspare, cfgxpcsrxspare);
        break;
    }
    default:
        err = BDMF_ERR_NOT_SUPPORTED;
        break;
    }
    return err;
}

static int bcm_xpcsrx_cli_test(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    bdmf_test_method m = parm[0].value.unumber;
    bdmf_error_t err = BDMF_ERR_OK;

    {
        bdmf_boolean cfgxpcsrxclk161rstn=gtmv(m, 1);
        if(!err) bdmf_session_print(session, "ag_drv_xpcsrx_rx_rst_set( %u)\n", cfgxpcsrxclk161rstn);
        if(!err) ag_drv_xpcsrx_rx_rst_set(cfgxpcsrxclk161rstn);
        if(!err) ag_drv_xpcsrx_rx_rst_get( &cfgxpcsrxclk161rstn);
        if(!err) bdmf_session_print(session, "ag_drv_xpcsrx_rx_rst_get( %u)\n", cfgxpcsrxclk161rstn);
        if(err || cfgxpcsrxclk161rstn!=gtmv(m, 1))
            return err ? err : BDMF_ERR_IO;
    }
    {
        xpcsrx_rx_int_stat rx_int_stat = {.intrxidledajit=gtmv(m, 1), .intrxfrmrmisbrst=gtmv(m, 1), .intrxidlesopeopgapbig=gtmv(m, 1), .intrxidlefrcins=gtmv(m, 1), .intrx64b66bminipgerr=gtmv(m, 1), .intrxfecnquecntneq=gtmv(m, 1), .intrxidlefifoundrun=gtmv(m, 1), .intrxidlefifoovrrun=gtmv(m, 1), .intrxfechighcor=gtmv(m, 1), .intrxfecdecstoponerr=gtmv(m, 1), .intrxfecdecpass=gtmv(m, 1), .intrxstatfrmrhighber=gtmv(m, 1), .intrxfrmrexitbysp=gtmv(m, 1), .intrxfrmrbadshmax=gtmv(m, 1), .intrxdscramburstseqout=gtmv(m, 1), .intrxtestpsudolock=gtmv(m, 1), .intrxtestpsudotype=gtmv(m, 1), .intrxtestpsudoerr=gtmv(m, 1), .intrxtestprbslock=gtmv(m, 1), .intrxtestprbserr=gtmv(m, 1), .intrxfecpsistdecfail=gtmv(m, 1), .intrxframerbadsh=gtmv(m, 1), .intrxframercwloss=gtmv(m, 1), .intrxframercwlock=gtmv(m, 1), .intrxfecdecfail=gtmv(m, 1), .intrx64b66bdecerr=gtmv(m, 1), .intrxfrmrnolocklos=gtmv(m, 1), .intrxfrmrrogue=gtmv(m, 1), .int_regs_err=gtmv(m, 1)};
        if(!err) bdmf_session_print(session, "ag_drv_xpcsrx_rx_int_stat_set( %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u)\n", rx_int_stat.intrxidledajit, rx_int_stat.intrxfrmrmisbrst, rx_int_stat.intrxidlesopeopgapbig, rx_int_stat.intrxidlefrcins, rx_int_stat.intrx64b66bminipgerr, rx_int_stat.intrxfecnquecntneq, rx_int_stat.intrxidlefifoundrun, rx_int_stat.intrxidlefifoovrrun, rx_int_stat.intrxfechighcor, rx_int_stat.intrxfecdecstoponerr, rx_int_stat.intrxfecdecpass, rx_int_stat.intrxstatfrmrhighber, rx_int_stat.intrxfrmrexitbysp, rx_int_stat.intrxfrmrbadshmax, rx_int_stat.intrxdscramburstseqout, rx_int_stat.intrxtestpsudolock, rx_int_stat.intrxtestpsudotype, rx_int_stat.intrxtestpsudoerr, rx_int_stat.intrxtestprbslock, rx_int_stat.intrxtestprbserr, rx_int_stat.intrxfecpsistdecfail, rx_int_stat.intrxframerbadsh, rx_int_stat.intrxframercwloss, rx_int_stat.intrxframercwlock, rx_int_stat.intrxfecdecfail, rx_int_stat.intrx64b66bdecerr, rx_int_stat.intrxfrmrnolocklos, rx_int_stat.intrxfrmrrogue, rx_int_stat.int_regs_err);
        if(!err) ag_drv_xpcsrx_rx_int_stat_set(&rx_int_stat);
        if(!err) ag_drv_xpcsrx_rx_int_stat_get( &rx_int_stat);
        if(!err) bdmf_session_print(session, "ag_drv_xpcsrx_rx_int_stat_get( %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u)\n", rx_int_stat.intrxidledajit, rx_int_stat.intrxfrmrmisbrst, rx_int_stat.intrxidlesopeopgapbig, rx_int_stat.intrxidlefrcins, rx_int_stat.intrx64b66bminipgerr, rx_int_stat.intrxfecnquecntneq, rx_int_stat.intrxidlefifoundrun, rx_int_stat.intrxidlefifoovrrun, rx_int_stat.intrxfechighcor, rx_int_stat.intrxfecdecstoponerr, rx_int_stat.intrxfecdecpass, rx_int_stat.intrxstatfrmrhighber, rx_int_stat.intrxfrmrexitbysp, rx_int_stat.intrxfrmrbadshmax, rx_int_stat.intrxdscramburstseqout, rx_int_stat.intrxtestpsudolock, rx_int_stat.intrxtestpsudotype, rx_int_stat.intrxtestpsudoerr, rx_int_stat.intrxtestprbslock, rx_int_stat.intrxtestprbserr, rx_int_stat.intrxfecpsistdecfail, rx_int_stat.intrxframerbadsh, rx_int_stat.intrxframercwloss, rx_int_stat.intrxframercwlock, rx_int_stat.intrxfecdecfail, rx_int_stat.intrx64b66bdecerr, rx_int_stat.intrxfrmrnolocklos, rx_int_stat.intrxfrmrrogue, rx_int_stat.int_regs_err);
        if(err || rx_int_stat.intrxidledajit!=gtmv(m, 1) || rx_int_stat.intrxfrmrmisbrst!=gtmv(m, 1) || rx_int_stat.intrxidlesopeopgapbig!=gtmv(m, 1) || rx_int_stat.intrxidlefrcins!=gtmv(m, 1) || rx_int_stat.intrx64b66bminipgerr!=gtmv(m, 1) || rx_int_stat.intrxfecnquecntneq!=gtmv(m, 1) || rx_int_stat.intrxidlefifoundrun!=gtmv(m, 1) || rx_int_stat.intrxidlefifoovrrun!=gtmv(m, 1) || rx_int_stat.intrxfechighcor!=gtmv(m, 1) || rx_int_stat.intrxfecdecstoponerr!=gtmv(m, 1) || rx_int_stat.intrxfecdecpass!=gtmv(m, 1) || rx_int_stat.intrxstatfrmrhighber!=gtmv(m, 1) || rx_int_stat.intrxfrmrexitbysp!=gtmv(m, 1) || rx_int_stat.intrxfrmrbadshmax!=gtmv(m, 1) || rx_int_stat.intrxdscramburstseqout!=gtmv(m, 1) || rx_int_stat.intrxtestpsudolock!=gtmv(m, 1) || rx_int_stat.intrxtestpsudotype!=gtmv(m, 1) || rx_int_stat.intrxtestpsudoerr!=gtmv(m, 1) || rx_int_stat.intrxtestprbslock!=gtmv(m, 1) || rx_int_stat.intrxtestprbserr!=gtmv(m, 1) || rx_int_stat.intrxfecpsistdecfail!=gtmv(m, 1) || rx_int_stat.intrxframerbadsh!=gtmv(m, 1) || rx_int_stat.intrxframercwloss!=gtmv(m, 1) || rx_int_stat.intrxframercwlock!=gtmv(m, 1) || rx_int_stat.intrxfecdecfail!=gtmv(m, 1) || rx_int_stat.intrx64b66bdecerr!=gtmv(m, 1) || rx_int_stat.intrxfrmrnolocklos!=gtmv(m, 1) || rx_int_stat.intrxfrmrrogue!=gtmv(m, 1) || rx_int_stat.int_regs_err!=gtmv(m, 1))
            return err ? err : BDMF_ERR_IO;
    }
    {
        xpcsrx_rx_int_msk rx_int_msk = {.mskrxidledajit=gtmv(m, 1), .mskrxfrmrmisbrst=gtmv(m, 1), .mskrxidlesopeopgapbig=gtmv(m, 1), .mskrxidlefrcins=gtmv(m, 1), .mskrx64b66bminipgerr=gtmv(m, 1), .mskrxfecnquecntneq=gtmv(m, 1), .mskrxidlefifoundrun=gtmv(m, 1), .mskrxidlefifoovrrun=gtmv(m, 1), .mskrxfechighcor=gtmv(m, 1), .mskrxfecdecstoponerr=gtmv(m, 1), .mskrxfecdecpass=gtmv(m, 1), .mskrxstatfrmrhighber=gtmv(m, 1), .mskrxfrmrexitbysp=gtmv(m, 1), .mskrxfrmrbadshmax=gtmv(m, 1), .mskrxdscramburstseqout=gtmv(m, 1), .mskrxtestpsudolock=gtmv(m, 1), .mskrxtestpsudotype=gtmv(m, 1), .mskrxtestpsudoerr=gtmv(m, 1), .mskrxtestprbslock=gtmv(m, 1), .mskrxtestprbserr=gtmv(m, 1), .mskrxfecpsistdecfail=gtmv(m, 1), .mskrxframerbadsh=gtmv(m, 1), .mskrxframercwloss=gtmv(m, 1), .mskrxframercwlock=gtmv(m, 1), .mskrxfecdecfail=gtmv(m, 1), .mskrx64b66bdecerr=gtmv(m, 1), .mskrxfrmrnolocklos=gtmv(m, 1), .mskrxfrmrrogue=gtmv(m, 1), .msk_int_regs_err=gtmv(m, 1)};
        if(!err) bdmf_session_print(session, "ag_drv_xpcsrx_rx_int_msk_set( %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u)\n", rx_int_msk.mskrxidledajit, rx_int_msk.mskrxfrmrmisbrst, rx_int_msk.mskrxidlesopeopgapbig, rx_int_msk.mskrxidlefrcins, rx_int_msk.mskrx64b66bminipgerr, rx_int_msk.mskrxfecnquecntneq, rx_int_msk.mskrxidlefifoundrun, rx_int_msk.mskrxidlefifoovrrun, rx_int_msk.mskrxfechighcor, rx_int_msk.mskrxfecdecstoponerr, rx_int_msk.mskrxfecdecpass, rx_int_msk.mskrxstatfrmrhighber, rx_int_msk.mskrxfrmrexitbysp, rx_int_msk.mskrxfrmrbadshmax, rx_int_msk.mskrxdscramburstseqout, rx_int_msk.mskrxtestpsudolock, rx_int_msk.mskrxtestpsudotype, rx_int_msk.mskrxtestpsudoerr, rx_int_msk.mskrxtestprbslock, rx_int_msk.mskrxtestprbserr, rx_int_msk.mskrxfecpsistdecfail, rx_int_msk.mskrxframerbadsh, rx_int_msk.mskrxframercwloss, rx_int_msk.mskrxframercwlock, rx_int_msk.mskrxfecdecfail, rx_int_msk.mskrx64b66bdecerr, rx_int_msk.mskrxfrmrnolocklos, rx_int_msk.mskrxfrmrrogue, rx_int_msk.msk_int_regs_err);
        if(!err) ag_drv_xpcsrx_rx_int_msk_set(&rx_int_msk);
        if(!err) ag_drv_xpcsrx_rx_int_msk_get( &rx_int_msk);
        if(!err) bdmf_session_print(session, "ag_drv_xpcsrx_rx_int_msk_get( %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u)\n", rx_int_msk.mskrxidledajit, rx_int_msk.mskrxfrmrmisbrst, rx_int_msk.mskrxidlesopeopgapbig, rx_int_msk.mskrxidlefrcins, rx_int_msk.mskrx64b66bminipgerr, rx_int_msk.mskrxfecnquecntneq, rx_int_msk.mskrxidlefifoundrun, rx_int_msk.mskrxidlefifoovrrun, rx_int_msk.mskrxfechighcor, rx_int_msk.mskrxfecdecstoponerr, rx_int_msk.mskrxfecdecpass, rx_int_msk.mskrxstatfrmrhighber, rx_int_msk.mskrxfrmrexitbysp, rx_int_msk.mskrxfrmrbadshmax, rx_int_msk.mskrxdscramburstseqout, rx_int_msk.mskrxtestpsudolock, rx_int_msk.mskrxtestpsudotype, rx_int_msk.mskrxtestpsudoerr, rx_int_msk.mskrxtestprbslock, rx_int_msk.mskrxtestprbserr, rx_int_msk.mskrxfecpsistdecfail, rx_int_msk.mskrxframerbadsh, rx_int_msk.mskrxframercwloss, rx_int_msk.mskrxframercwlock, rx_int_msk.mskrxfecdecfail, rx_int_msk.mskrx64b66bdecerr, rx_int_msk.mskrxfrmrnolocklos, rx_int_msk.mskrxfrmrrogue, rx_int_msk.msk_int_regs_err);
        if(err || rx_int_msk.mskrxidledajit!=gtmv(m, 1) || rx_int_msk.mskrxfrmrmisbrst!=gtmv(m, 1) || rx_int_msk.mskrxidlesopeopgapbig!=gtmv(m, 1) || rx_int_msk.mskrxidlefrcins!=gtmv(m, 1) || rx_int_msk.mskrx64b66bminipgerr!=gtmv(m, 1) || rx_int_msk.mskrxfecnquecntneq!=gtmv(m, 1) || rx_int_msk.mskrxidlefifoundrun!=gtmv(m, 1) || rx_int_msk.mskrxidlefifoovrrun!=gtmv(m, 1) || rx_int_msk.mskrxfechighcor!=gtmv(m, 1) || rx_int_msk.mskrxfecdecstoponerr!=gtmv(m, 1) || rx_int_msk.mskrxfecdecpass!=gtmv(m, 1) || rx_int_msk.mskrxstatfrmrhighber!=gtmv(m, 1) || rx_int_msk.mskrxfrmrexitbysp!=gtmv(m, 1) || rx_int_msk.mskrxfrmrbadshmax!=gtmv(m, 1) || rx_int_msk.mskrxdscramburstseqout!=gtmv(m, 1) || rx_int_msk.mskrxtestpsudolock!=gtmv(m, 1) || rx_int_msk.mskrxtestpsudotype!=gtmv(m, 1) || rx_int_msk.mskrxtestpsudoerr!=gtmv(m, 1) || rx_int_msk.mskrxtestprbslock!=gtmv(m, 1) || rx_int_msk.mskrxtestprbserr!=gtmv(m, 1) || rx_int_msk.mskrxfecpsistdecfail!=gtmv(m, 1) || rx_int_msk.mskrxframerbadsh!=gtmv(m, 1) || rx_int_msk.mskrxframercwloss!=gtmv(m, 1) || rx_int_msk.mskrxframercwlock!=gtmv(m, 1) || rx_int_msk.mskrxfecdecfail!=gtmv(m, 1) || rx_int_msk.mskrx64b66bdecerr!=gtmv(m, 1) || rx_int_msk.mskrxfrmrnolocklos!=gtmv(m, 1) || rx_int_msk.mskrxfrmrrogue!=gtmv(m, 1) || rx_int_msk.msk_int_regs_err!=gtmv(m, 1))
            return err ? err : BDMF_ERR_IO;
    }
    {
        xpcsrx_rx_framer_ctl rx_framer_ctl = {.cfgxpcsrxfrmrfrcearlyalign=gtmv(m, 1), .cfgxpcsrxfrmrmodea=gtmv(m, 1), .cfgxpcsrxfrmroverlapbdebdzero=gtmv(m, 1), .cfgxpcsrxfrmroverlapgnten=gtmv(m, 1), .cfgxpcsrxframeburstoldalign=gtmv(m, 1), .cfgxpcsrxfrmrmisbrsttype=gtmv(m, 1), .cfgxpcsrxfrmrebdvlden=gtmv(m, 1), .cfgxpcsrxfrmrbdcnten=gtmv(m, 1), .cfgxpcsrxfrmrburstbadshen=gtmv(m, 1), .cfgxpcsrxfrmrspulken=gtmv(m, 1), .cfgxpcsrxframeburst=gtmv(m, 1), .cfgxpcsrxfrmren=gtmv(m, 1), .cfgxpcsrxfrmrblkfecfail=gtmv(m, 1), .cfgxpcsrxframefec=gtmv(m, 1)};
        if(!err) bdmf_session_print(session, "ag_drv_xpcsrx_rx_framer_ctl_set( %u %u %u %u %u %u %u %u %u %u %u %u %u %u)\n", rx_framer_ctl.cfgxpcsrxfrmrfrcearlyalign, rx_framer_ctl.cfgxpcsrxfrmrmodea, rx_framer_ctl.cfgxpcsrxfrmroverlapbdebdzero, rx_framer_ctl.cfgxpcsrxfrmroverlapgnten, rx_framer_ctl.cfgxpcsrxframeburstoldalign, rx_framer_ctl.cfgxpcsrxfrmrmisbrsttype, rx_framer_ctl.cfgxpcsrxfrmrebdvlden, rx_framer_ctl.cfgxpcsrxfrmrbdcnten, rx_framer_ctl.cfgxpcsrxfrmrburstbadshen, rx_framer_ctl.cfgxpcsrxfrmrspulken, rx_framer_ctl.cfgxpcsrxframeburst, rx_framer_ctl.cfgxpcsrxfrmren, rx_framer_ctl.cfgxpcsrxfrmrblkfecfail, rx_framer_ctl.cfgxpcsrxframefec);
        if(!err) ag_drv_xpcsrx_rx_framer_ctl_set(&rx_framer_ctl);
        if(!err) ag_drv_xpcsrx_rx_framer_ctl_get( &rx_framer_ctl);
        if(!err) bdmf_session_print(session, "ag_drv_xpcsrx_rx_framer_ctl_get( %u %u %u %u %u %u %u %u %u %u %u %u %u %u)\n", rx_framer_ctl.cfgxpcsrxfrmrfrcearlyalign, rx_framer_ctl.cfgxpcsrxfrmrmodea, rx_framer_ctl.cfgxpcsrxfrmroverlapbdebdzero, rx_framer_ctl.cfgxpcsrxfrmroverlapgnten, rx_framer_ctl.cfgxpcsrxframeburstoldalign, rx_framer_ctl.cfgxpcsrxfrmrmisbrsttype, rx_framer_ctl.cfgxpcsrxfrmrebdvlden, rx_framer_ctl.cfgxpcsrxfrmrbdcnten, rx_framer_ctl.cfgxpcsrxfrmrburstbadshen, rx_framer_ctl.cfgxpcsrxfrmrspulken, rx_framer_ctl.cfgxpcsrxframeburst, rx_framer_ctl.cfgxpcsrxfrmren, rx_framer_ctl.cfgxpcsrxfrmrblkfecfail, rx_framer_ctl.cfgxpcsrxframefec);
        if(err || rx_framer_ctl.cfgxpcsrxfrmrfrcearlyalign!=gtmv(m, 1) || rx_framer_ctl.cfgxpcsrxfrmrmodea!=gtmv(m, 1) || rx_framer_ctl.cfgxpcsrxfrmroverlapbdebdzero!=gtmv(m, 1) || rx_framer_ctl.cfgxpcsrxfrmroverlapgnten!=gtmv(m, 1) || rx_framer_ctl.cfgxpcsrxframeburstoldalign!=gtmv(m, 1) || rx_framer_ctl.cfgxpcsrxfrmrmisbrsttype!=gtmv(m, 1) || rx_framer_ctl.cfgxpcsrxfrmrebdvlden!=gtmv(m, 1) || rx_framer_ctl.cfgxpcsrxfrmrbdcnten!=gtmv(m, 1) || rx_framer_ctl.cfgxpcsrxfrmrburstbadshen!=gtmv(m, 1) || rx_framer_ctl.cfgxpcsrxfrmrspulken!=gtmv(m, 1) || rx_framer_ctl.cfgxpcsrxframeburst!=gtmv(m, 1) || rx_framer_ctl.cfgxpcsrxfrmren!=gtmv(m, 1) || rx_framer_ctl.cfgxpcsrxfrmrblkfecfail!=gtmv(m, 1) || rx_framer_ctl.cfgxpcsrxframefec!=gtmv(m, 1))
            return err ? err : BDMF_ERR_IO;
    }
    {
        xpcsrx_rx_fec_ctl rx_fec_ctl = {.cfgxpcsrxfecstoponerr=gtmv(m, 1), .cfgxpcsrxfeccntnquecw=gtmv(m, 1), .cfgxpcsrxfecnquerst=gtmv(m, 1), .cfgxpcsrxfeconezeromode=gtmv(m, 1), .cfgxpcsrxfecblkcorrect=gtmv(m, 1), .cfgxpcsrxfecnquetestpat=gtmv(m, 1), .cfgxpcsrxfecfailblksh0=gtmv(m, 1), .cfgxpcsrxfecstrip=gtmv(m, 1), .cfgxpcsrxfecbypas=gtmv(m, 1), .cfgxpcsrxfecidleins=gtmv(m, 1), .cfgxpcsrxfecen=gtmv(m, 1)};
        if(!err) bdmf_session_print(session, "ag_drv_xpcsrx_rx_fec_ctl_set( %u %u %u %u %u %u %u %u %u %u %u)\n", rx_fec_ctl.cfgxpcsrxfecstoponerr, rx_fec_ctl.cfgxpcsrxfeccntnquecw, rx_fec_ctl.cfgxpcsrxfecnquerst, rx_fec_ctl.cfgxpcsrxfeconezeromode, rx_fec_ctl.cfgxpcsrxfecblkcorrect, rx_fec_ctl.cfgxpcsrxfecnquetestpat, rx_fec_ctl.cfgxpcsrxfecfailblksh0, rx_fec_ctl.cfgxpcsrxfecstrip, rx_fec_ctl.cfgxpcsrxfecbypas, rx_fec_ctl.cfgxpcsrxfecidleins, rx_fec_ctl.cfgxpcsrxfecen);
        if(!err) ag_drv_xpcsrx_rx_fec_ctl_set(&rx_fec_ctl);
        if(!err) ag_drv_xpcsrx_rx_fec_ctl_get( &rx_fec_ctl);
        if(!err) bdmf_session_print(session, "ag_drv_xpcsrx_rx_fec_ctl_get( %u %u %u %u %u %u %u %u %u %u %u)\n", rx_fec_ctl.cfgxpcsrxfecstoponerr, rx_fec_ctl.cfgxpcsrxfeccntnquecw, rx_fec_ctl.cfgxpcsrxfecnquerst, rx_fec_ctl.cfgxpcsrxfeconezeromode, rx_fec_ctl.cfgxpcsrxfecblkcorrect, rx_fec_ctl.cfgxpcsrxfecnquetestpat, rx_fec_ctl.cfgxpcsrxfecfailblksh0, rx_fec_ctl.cfgxpcsrxfecstrip, rx_fec_ctl.cfgxpcsrxfecbypas, rx_fec_ctl.cfgxpcsrxfecidleins, rx_fec_ctl.cfgxpcsrxfecen);
        if(err || rx_fec_ctl.cfgxpcsrxfecstoponerr!=gtmv(m, 1) || rx_fec_ctl.cfgxpcsrxfeccntnquecw!=gtmv(m, 1) || rx_fec_ctl.cfgxpcsrxfecnquerst!=gtmv(m, 1) || rx_fec_ctl.cfgxpcsrxfeconezeromode!=gtmv(m, 1) || rx_fec_ctl.cfgxpcsrxfecblkcorrect!=gtmv(m, 1) || rx_fec_ctl.cfgxpcsrxfecnquetestpat!=gtmv(m, 1) || rx_fec_ctl.cfgxpcsrxfecfailblksh0!=gtmv(m, 1) || rx_fec_ctl.cfgxpcsrxfecstrip!=gtmv(m, 1) || rx_fec_ctl.cfgxpcsrxfecbypas!=gtmv(m, 1) || rx_fec_ctl.cfgxpcsrxfecidleins!=gtmv(m, 1) || rx_fec_ctl.cfgxpcsrxfecen!=gtmv(m, 1))
            return err ? err : BDMF_ERR_IO;
    }
    {
        bdmf_boolean cfgxpcsrxdscrambypas=gtmv(m, 1);
        if(!err) bdmf_session_print(session, "ag_drv_xpcsrx_rx_dscram_ctl_set( %u)\n", cfgxpcsrxdscrambypas);
        if(!err) ag_drv_xpcsrx_rx_dscram_ctl_set(cfgxpcsrxdscrambypas);
        if(!err) ag_drv_xpcsrx_rx_dscram_ctl_get( &cfgxpcsrxdscrambypas);
        if(!err) bdmf_session_print(session, "ag_drv_xpcsrx_rx_dscram_ctl_get( %u)\n", cfgxpcsrxdscrambypas);
        if(err || cfgxpcsrxdscrambypas!=gtmv(m, 1))
            return err ? err : BDMF_ERR_IO;
    }
    {
        xpcsrx_rx_64b66b_ctl rx_64b66b_ctl = {.cfgxpcsrx64b66btmask1=gtmv(m, 8), .cfgxpcsrx64b66btmask0=gtmv(m, 8), .cfgxpcsrx64b66bsmask1=gtmv(m, 2), .cfgxpcsrx64b66bsmask0=gtmv(m, 2), .cfgxpcsrx64b66btdlay=gtmv(m, 2), .cfgxpcsrx64b66bdecbypas=gtmv(m, 1)};
        if(!err) bdmf_session_print(session, "ag_drv_xpcsrx_rx_64b66b_ctl_set( %u %u %u %u %u %u)\n", rx_64b66b_ctl.cfgxpcsrx64b66btmask1, rx_64b66b_ctl.cfgxpcsrx64b66btmask0, rx_64b66b_ctl.cfgxpcsrx64b66bsmask1, rx_64b66b_ctl.cfgxpcsrx64b66bsmask0, rx_64b66b_ctl.cfgxpcsrx64b66btdlay, rx_64b66b_ctl.cfgxpcsrx64b66bdecbypas);
        if(!err) ag_drv_xpcsrx_rx_64b66b_ctl_set(&rx_64b66b_ctl);
        if(!err) ag_drv_xpcsrx_rx_64b66b_ctl_get( &rx_64b66b_ctl);
        if(!err) bdmf_session_print(session, "ag_drv_xpcsrx_rx_64b66b_ctl_get( %u %u %u %u %u %u)\n", rx_64b66b_ctl.cfgxpcsrx64b66btmask1, rx_64b66b_ctl.cfgxpcsrx64b66btmask0, rx_64b66b_ctl.cfgxpcsrx64b66bsmask1, rx_64b66b_ctl.cfgxpcsrx64b66bsmask0, rx_64b66b_ctl.cfgxpcsrx64b66btdlay, rx_64b66b_ctl.cfgxpcsrx64b66bdecbypas);
        if(err || rx_64b66b_ctl.cfgxpcsrx64b66btmask1!=gtmv(m, 8) || rx_64b66b_ctl.cfgxpcsrx64b66btmask0!=gtmv(m, 8) || rx_64b66b_ctl.cfgxpcsrx64b66bsmask1!=gtmv(m, 2) || rx_64b66b_ctl.cfgxpcsrx64b66bsmask0!=gtmv(m, 2) || rx_64b66b_ctl.cfgxpcsrx64b66btdlay!=gtmv(m, 2) || rx_64b66b_ctl.cfgxpcsrx64b66bdecbypas!=gtmv(m, 1))
            return err ? err : BDMF_ERR_IO;
    }
    {
        bdmf_boolean cfgxpcsrxtestprbsdeten=gtmv(m, 1);
        bdmf_boolean cfgxpcsrxtestpsudodeten=gtmv(m, 1);
        if(!err) bdmf_session_print(session, "ag_drv_xpcsrx_rx_test_ctl_set( %u %u)\n", cfgxpcsrxtestprbsdeten, cfgxpcsrxtestpsudodeten);
        if(!err) ag_drv_xpcsrx_rx_test_ctl_set(cfgxpcsrxtestprbsdeten, cfgxpcsrxtestpsudodeten);
        if(!err) ag_drv_xpcsrx_rx_test_ctl_get( &cfgxpcsrxtestprbsdeten, &cfgxpcsrxtestpsudodeten);
        if(!err) bdmf_session_print(session, "ag_drv_xpcsrx_rx_test_ctl_get( %u %u)\n", cfgxpcsrxtestprbsdeten, cfgxpcsrxtestpsudodeten);
        if(err || cfgxpcsrxtestprbsdeten!=gtmv(m, 1) || cfgxpcsrxtestpsudodeten!=gtmv(m, 1))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint8_t cfgxpcsrxidlerddelaytimermax=gtmv(m, 8);
        if(!err) bdmf_session_print(session, "ag_drv_xpcsrx_rx_idle_rd_timer_dly_set( %u)\n", cfgxpcsrxidlerddelaytimermax);
        if(!err) ag_drv_xpcsrx_rx_idle_rd_timer_dly_set(cfgxpcsrxidlerddelaytimermax);
        if(!err) ag_drv_xpcsrx_rx_idle_rd_timer_dly_get( &cfgxpcsrxidlerddelaytimermax);
        if(!err) bdmf_session_print(session, "ag_drv_xpcsrx_rx_idle_rd_timer_dly_get( %u)\n", cfgxpcsrxidlerddelaytimermax);
        if(err || cfgxpcsrxidlerddelaytimermax!=gtmv(m, 8))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint16_t cfgxpcsrxidleovrsizmax=gtmv(m, 11);
        uint16_t cfgxpcsrxidlesopeopgap=gtmv(m, 16);
        if(!err) bdmf_session_print(session, "ag_drv_xpcsrx_rx_idle_gap_siz_max_set( %u %u)\n", cfgxpcsrxidleovrsizmax, cfgxpcsrxidlesopeopgap);
        if(!err) ag_drv_xpcsrx_rx_idle_gap_siz_max_set(cfgxpcsrxidleovrsizmax, cfgxpcsrxidlesopeopgap);
        if(!err) ag_drv_xpcsrx_rx_idle_gap_siz_max_get( &cfgxpcsrxidleovrsizmax, &cfgxpcsrxidlesopeopgap);
        if(!err) bdmf_session_print(session, "ag_drv_xpcsrx_rx_idle_gap_siz_max_get( %u %u)\n", cfgxpcsrxidleovrsizmax, cfgxpcsrxidlesopeopgap);
        if(err || cfgxpcsrxidleovrsizmax!=gtmv(m, 11) || cfgxpcsrxidlesopeopgap!=gtmv(m, 16))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint16_t cfgxpcsrxfrmrcwlktimermax=gtmv(m, 9);
        if(!err) bdmf_session_print(session, "ag_drv_xpcsrx_rx_framer_lk_max_set( %u)\n", cfgxpcsrxfrmrcwlktimermax);
        if(!err) ag_drv_xpcsrx_rx_framer_lk_max_set(cfgxpcsrxfrmrcwlktimermax);
        if(!err) ag_drv_xpcsrx_rx_framer_lk_max_get( &cfgxpcsrxfrmrcwlktimermax);
        if(!err) bdmf_session_print(session, "ag_drv_xpcsrx_rx_framer_lk_max_get( %u)\n", cfgxpcsrxfrmrcwlktimermax);
        if(err || cfgxpcsrxfrmrcwlktimermax!=gtmv(m, 9))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint16_t cfgxpcsrxfrmrcwunlktimermax=gtmv(m, 9);
        if(!err) bdmf_session_print(session, "ag_drv_xpcsrx_rx_framer_unlk_max_set( %u)\n", cfgxpcsrxfrmrcwunlktimermax);
        if(!err) ag_drv_xpcsrx_rx_framer_unlk_max_set(cfgxpcsrxfrmrcwunlktimermax);
        if(!err) ag_drv_xpcsrx_rx_framer_unlk_max_get( &cfgxpcsrxfrmrcwunlktimermax);
        if(!err) bdmf_session_print(session, "ag_drv_xpcsrx_rx_framer_unlk_max_get( %u)\n", cfgxpcsrxfrmrcwunlktimermax);
        if(err || cfgxpcsrxfrmrcwunlktimermax!=gtmv(m, 9))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint8_t cfgxpcsrxoltbdsh=gtmv(m, 2);
        if(!err) bdmf_session_print(session, "ag_drv_xpcsrx_rx_framer_bd_sh_set( %u)\n", cfgxpcsrxoltbdsh);
        if(!err) ag_drv_xpcsrx_rx_framer_bd_sh_set(cfgxpcsrxoltbdsh);
        if(!err) ag_drv_xpcsrx_rx_framer_bd_sh_get( &cfgxpcsrxoltbdsh);
        if(!err) bdmf_session_print(session, "ag_drv_xpcsrx_rx_framer_bd_sh_get( %u)\n", cfgxpcsrxoltbdsh);
        if(err || cfgxpcsrxoltbdsh!=gtmv(m, 2))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint32_t cfgxpcsrxoltbdlo=gtmv(m, 32);
        if(!err) bdmf_session_print(session, "ag_drv_xpcsrx_rx_framer_bd_lo_set( %u)\n", cfgxpcsrxoltbdlo);
        if(!err) ag_drv_xpcsrx_rx_framer_bd_lo_set(cfgxpcsrxoltbdlo);
        if(!err) ag_drv_xpcsrx_rx_framer_bd_lo_get( &cfgxpcsrxoltbdlo);
        if(!err) bdmf_session_print(session, "ag_drv_xpcsrx_rx_framer_bd_lo_get( %u)\n", cfgxpcsrxoltbdlo);
        if(err || cfgxpcsrxoltbdlo!=gtmv(m, 32))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint32_t cfgxpcsrxoltbdhi=gtmv(m, 32);
        if(!err) bdmf_session_print(session, "ag_drv_xpcsrx_rx_framer_bd_hi_set( %u)\n", cfgxpcsrxoltbdhi);
        if(!err) ag_drv_xpcsrx_rx_framer_bd_hi_set(cfgxpcsrxoltbdhi);
        if(!err) ag_drv_xpcsrx_rx_framer_bd_hi_get( &cfgxpcsrxoltbdhi);
        if(!err) bdmf_session_print(session, "ag_drv_xpcsrx_rx_framer_bd_hi_get( %u)\n", cfgxpcsrxoltbdhi);
        if(err || cfgxpcsrxoltbdhi!=gtmv(m, 32))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint8_t cfgxpcsrxoltebdsh=gtmv(m, 2);
        if(!err) bdmf_session_print(session, "ag_drv_xpcsrx_rx_framer_ebd_sh_set( %u)\n", cfgxpcsrxoltebdsh);
        if(!err) ag_drv_xpcsrx_rx_framer_ebd_sh_set(cfgxpcsrxoltebdsh);
        if(!err) ag_drv_xpcsrx_rx_framer_ebd_sh_get( &cfgxpcsrxoltebdsh);
        if(!err) bdmf_session_print(session, "ag_drv_xpcsrx_rx_framer_ebd_sh_get( %u)\n", cfgxpcsrxoltebdsh);
        if(err || cfgxpcsrxoltebdsh!=gtmv(m, 2))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint32_t cfgxpcsrxoltebdlo=gtmv(m, 32);
        if(!err) bdmf_session_print(session, "ag_drv_xpcsrx_rx_framer_ebd_lo_set( %u)\n", cfgxpcsrxoltebdlo);
        if(!err) ag_drv_xpcsrx_rx_framer_ebd_lo_set(cfgxpcsrxoltebdlo);
        if(!err) ag_drv_xpcsrx_rx_framer_ebd_lo_get( &cfgxpcsrxoltebdlo);
        if(!err) bdmf_session_print(session, "ag_drv_xpcsrx_rx_framer_ebd_lo_get( %u)\n", cfgxpcsrxoltebdlo);
        if(err || cfgxpcsrxoltebdlo!=gtmv(m, 32))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint32_t cfgxpcsrxoltebdhi=gtmv(m, 32);
        if(!err) bdmf_session_print(session, "ag_drv_xpcsrx_rx_framer_ebd_hi_set( %u)\n", cfgxpcsrxoltebdhi);
        if(!err) ag_drv_xpcsrx_rx_framer_ebd_hi_set(cfgxpcsrxoltebdhi);
        if(!err) ag_drv_xpcsrx_rx_framer_ebd_hi_get( &cfgxpcsrxoltebdhi);
        if(!err) bdmf_session_print(session, "ag_drv_xpcsrx_rx_framer_ebd_hi_get( %u)\n", cfgxpcsrxoltebdhi);
        if(err || cfgxpcsrxoltebdhi!=gtmv(m, 32))
            return err ? err : BDMF_ERR_IO;
    }
    {
        xpcsrx_rx_status rx_status = {.statrxidledajit=gtmv(m, 1), .statrxfrmrmisbrst=gtmv(m, 1), .statrxidlesopeopgapbig=gtmv(m, 1), .statrxidlefrcins=gtmv(m, 1), .statrx64b66bminipgerr=gtmv(m, 1), .statrxfecnquecntneq=gtmv(m, 1), .statrxidlefifoundrun=gtmv(m, 1), .statrxidlefifoovrrun=gtmv(m, 1), .statrxfechighcor=gtmv(m, 1), .statrxfecdecpass=gtmv(m, 1), .statrxstatfrmrhighber=gtmv(m, 1), .statrxfrmrexitbysp=gtmv(m, 1), .statrxfrmrbadshmax=gtmv(m, 1), .statrxdscramburstseqout=gtmv(m, 1), .statrxtestpsudolock=gtmv(m, 1), .statrxtestpsudotype=gtmv(m, 1), .statrxtestpsudoerr=gtmv(m, 1), .statrxtestprbslock=gtmv(m, 1), .statrxtestprbserr=gtmv(m, 1), .statrxfecpsistdecfail=gtmv(m, 1), .statrxframerbadsh=gtmv(m, 1), .statrxframercwloss=gtmv(m, 1), .statrxframercwlock=gtmv(m, 1), .statrxfecdecfail=gtmv(m, 1), .statrx64b66bdecerr=gtmv(m, 1), .statrxfrmrnolocklos=gtmv(m, 1), .statrxfrmrrogue=gtmv(m, 1)};
        if(!err) ag_drv_xpcsrx_rx_status_get( &rx_status);
        if(!err) bdmf_session_print(session, "ag_drv_xpcsrx_rx_status_get( %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u)\n", rx_status.statrxidledajit, rx_status.statrxfrmrmisbrst, rx_status.statrxidlesopeopgapbig, rx_status.statrxidlefrcins, rx_status.statrx64b66bminipgerr, rx_status.statrxfecnquecntneq, rx_status.statrxidlefifoundrun, rx_status.statrxidlefifoovrrun, rx_status.statrxfechighcor, rx_status.statrxfecdecpass, rx_status.statrxstatfrmrhighber, rx_status.statrxfrmrexitbysp, rx_status.statrxfrmrbadshmax, rx_status.statrxdscramburstseqout, rx_status.statrxtestpsudolock, rx_status.statrxtestpsudotype, rx_status.statrxtestpsudoerr, rx_status.statrxtestprbslock, rx_status.statrxtestprbserr, rx_status.statrxfecpsistdecfail, rx_status.statrxframerbadsh, rx_status.statrxframercwloss, rx_status.statrxframercwlock, rx_status.statrxfecdecfail, rx_status.statrx64b66bdecerr, rx_status.statrxfrmrnolocklos, rx_status.statrxfrmrrogue);
    }
    {
        uint16_t cfgxpcsrxfrmrsplkmax=gtmv(m, 13);
        uint16_t cfgxpcsrxfrmrspulkmax=gtmv(m, 13);
        if(!err) bdmf_session_print(session, "ag_drv_xpcsrx_rx_framer_lk_ulk_max_set( %u %u)\n", cfgxpcsrxfrmrsplkmax, cfgxpcsrxfrmrspulkmax);
        if(!err) ag_drv_xpcsrx_rx_framer_lk_ulk_max_set(cfgxpcsrxfrmrsplkmax, cfgxpcsrxfrmrspulkmax);
        if(!err) ag_drv_xpcsrx_rx_framer_lk_ulk_max_get( &cfgxpcsrxfrmrsplkmax, &cfgxpcsrxfrmrspulkmax);
        if(!err) bdmf_session_print(session, "ag_drv_xpcsrx_rx_framer_lk_ulk_max_get( %u %u)\n", cfgxpcsrxfrmrsplkmax, cfgxpcsrxfrmrspulkmax);
        if(err || cfgxpcsrxfrmrsplkmax!=gtmv(m, 13) || cfgxpcsrxfrmrspulkmax!=gtmv(m, 13))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint8_t cfgxpcsrxoltspsh=gtmv(m, 2);
        if(!err) bdmf_session_print(session, "ag_drv_xpcsrx_rx_framer_sp_sh_set( %u)\n", cfgxpcsrxoltspsh);
        if(!err) ag_drv_xpcsrx_rx_framer_sp_sh_set(cfgxpcsrxoltspsh);
        if(!err) ag_drv_xpcsrx_rx_framer_sp_sh_get( &cfgxpcsrxoltspsh);
        if(!err) bdmf_session_print(session, "ag_drv_xpcsrx_rx_framer_sp_sh_get( %u)\n", cfgxpcsrxoltspsh);
        if(err || cfgxpcsrxoltspsh!=gtmv(m, 2))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint32_t cfgxpcsrxoltsplo=gtmv(m, 32);
        if(!err) bdmf_session_print(session, "ag_drv_xpcsrx_rx_framer_sp_lo_set( %u)\n", cfgxpcsrxoltsplo);
        if(!err) ag_drv_xpcsrx_rx_framer_sp_lo_set(cfgxpcsrxoltsplo);
        if(!err) ag_drv_xpcsrx_rx_framer_sp_lo_get( &cfgxpcsrxoltsplo);
        if(!err) bdmf_session_print(session, "ag_drv_xpcsrx_rx_framer_sp_lo_get( %u)\n", cfgxpcsrxoltsplo);
        if(err || cfgxpcsrxoltsplo!=gtmv(m, 32))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint32_t cfgxpcsrxoltsphi=gtmv(m, 32);
        if(!err) bdmf_session_print(session, "ag_drv_xpcsrx_rx_framer_sp_hi_set( %u)\n", cfgxpcsrxoltsphi);
        if(!err) ag_drv_xpcsrx_rx_framer_sp_hi_set(cfgxpcsrxoltsphi);
        if(!err) ag_drv_xpcsrx_rx_framer_sp_hi_get( &cfgxpcsrxoltsphi);
        if(!err) bdmf_session_print(session, "ag_drv_xpcsrx_rx_framer_sp_hi_get( %u)\n", cfgxpcsrxoltsphi);
        if(err || cfgxpcsrxoltsphi!=gtmv(m, 32))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint8_t xpcsrxfrmrstate=gtmv(m, 4);
        if(!err) ag_drv_xpcsrx_rx_framer_state_get( &xpcsrxfrmrstate);
        if(!err) bdmf_session_print(session, "ag_drv_xpcsrx_rx_framer_state_get( %u)\n", xpcsrxfrmrstate);
    }
    {
        uint8_t cfgxpcsrxfrmrspham=gtmv(m, 4);
        uint8_t cfgxpcsrxfrmrebdham=gtmv(m, 4);
        uint8_t cfgxpcsrxfrmrbdham=gtmv(m, 4);
        if(!err) bdmf_session_print(session, "ag_drv_xpcsrx_rx_framer_bd_ebd_ham_set( %u %u %u)\n", cfgxpcsrxfrmrspham, cfgxpcsrxfrmrebdham, cfgxpcsrxfrmrbdham);
        if(!err) ag_drv_xpcsrx_rx_framer_bd_ebd_ham_set(cfgxpcsrxfrmrspham, cfgxpcsrxfrmrebdham, cfgxpcsrxfrmrbdham);
        if(!err) ag_drv_xpcsrx_rx_framer_bd_ebd_ham_get( &cfgxpcsrxfrmrspham, &cfgxpcsrxfrmrebdham, &cfgxpcsrxfrmrbdham);
        if(!err) bdmf_session_print(session, "ag_drv_xpcsrx_rx_framer_bd_ebd_ham_get( %u %u %u)\n", cfgxpcsrxfrmrspham, cfgxpcsrxfrmrebdham, cfgxpcsrxfrmrbdham);
        if(err || cfgxpcsrxfrmrspham!=gtmv(m, 4) || cfgxpcsrxfrmrebdham!=gtmv(m, 4) || cfgxpcsrxfrmrbdham!=gtmv(m, 4))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint32_t rxfrmrmisbrstcnt=gtmv(m, 32);
        if(!err) bdmf_session_print(session, "ag_drv_xpcsrx_rx_framer_misbrst_cnt_set( %u)\n", rxfrmrmisbrstcnt);
        if(!err) ag_drv_xpcsrx_rx_framer_misbrst_cnt_set(rxfrmrmisbrstcnt);
        if(!err) ag_drv_xpcsrx_rx_framer_misbrst_cnt_get( &rxfrmrmisbrstcnt);
        if(!err) bdmf_session_print(session, "ag_drv_xpcsrx_rx_framer_misbrst_cnt_get( %u)\n", rxfrmrmisbrstcnt);
        if(err || rxfrmrmisbrstcnt!=gtmv(m, 32))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint8_t xpcsrxstatfrmrbderr=gtmv(m, 4);
        if(!err) ag_drv_xpcsrx_rx_framer_bd_err_get( &xpcsrxstatfrmrbderr);
        if(!err) bdmf_session_print(session, "ag_drv_xpcsrx_rx_framer_bd_err_get( %u)\n", xpcsrxstatfrmrbderr);
    }
    {
        bdmf_boolean cfgxpcsrxfrmrrogueen=gtmv(m, 1);
        uint16_t cfgxpcsrxfrmrroguesptresh=gtmv(m, 16);
        if(!err) bdmf_session_print(session, "ag_drv_xpcsrx_rx_framer_rogue_ctl_set( %u %u)\n", cfgxpcsrxfrmrrogueen, cfgxpcsrxfrmrroguesptresh);
        if(!err) ag_drv_xpcsrx_rx_framer_rogue_ctl_set(cfgxpcsrxfrmrrogueen, cfgxpcsrxfrmrroguesptresh);
        if(!err) ag_drv_xpcsrx_rx_framer_rogue_ctl_get( &cfgxpcsrxfrmrrogueen, &cfgxpcsrxfrmrroguesptresh);
        if(!err) bdmf_session_print(session, "ag_drv_xpcsrx_rx_framer_rogue_ctl_get( %u %u)\n", cfgxpcsrxfrmrrogueen, cfgxpcsrxfrmrroguesptresh);
        if(err || cfgxpcsrxfrmrrogueen!=gtmv(m, 1) || cfgxpcsrxfrmrroguesptresh!=gtmv(m, 16))
            return err ? err : BDMF_ERR_IO;
    }
    {
        bdmf_boolean cfgxpcsrxfrmrnolocklosen=gtmv(m, 1);
        uint32_t cfgxpcsrxfrmrnolocklosintval=gtmv(m, 24);
        if(!err) bdmf_session_print(session, "ag_drv_xpcsrx_rx_framer_nolock_ctl_set( %u %u)\n", cfgxpcsrxfrmrnolocklosen, cfgxpcsrxfrmrnolocklosintval);
        if(!err) ag_drv_xpcsrx_rx_framer_nolock_ctl_set(cfgxpcsrxfrmrnolocklosen, cfgxpcsrxfrmrnolocklosintval);
        if(!err) ag_drv_xpcsrx_rx_framer_nolock_ctl_get( &cfgxpcsrxfrmrnolocklosen, &cfgxpcsrxfrmrnolocklosintval);
        if(!err) bdmf_session_print(session, "ag_drv_xpcsrx_rx_framer_nolock_ctl_get( %u %u)\n", cfgxpcsrxfrmrnolocklosen, cfgxpcsrxfrmrnolocklosintval);
        if(err || cfgxpcsrxfrmrnolocklosen!=gtmv(m, 1) || cfgxpcsrxfrmrnolocklosintval!=gtmv(m, 24))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint32_t rx64b66bipgdetcnt=gtmv(m, 32);
        if(!err) bdmf_session_print(session, "ag_drv_xpcsrx_rx_64b66b_ipg_det_cnt_set( %u)\n", rx64b66bipgdetcnt);
        if(!err) ag_drv_xpcsrx_rx_64b66b_ipg_det_cnt_set(rx64b66bipgdetcnt);
        if(!err) ag_drv_xpcsrx_rx_64b66b_ipg_det_cnt_get( &rx64b66bipgdetcnt);
        if(!err) bdmf_session_print(session, "ag_drv_xpcsrx_rx_64b66b_ipg_det_cnt_get( %u)\n", rx64b66bipgdetcnt);
        if(err || rx64b66bipgdetcnt!=gtmv(m, 32))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint32_t rxfecnqueincnt=gtmv(m, 32);
        if(!err) bdmf_session_print(session, "ag_drv_xpcsrx_rx_fec_nque_in_cnt_set( %u)\n", rxfecnqueincnt);
        if(!err) ag_drv_xpcsrx_rx_fec_nque_in_cnt_set(rxfecnqueincnt);
        if(!err) ag_drv_xpcsrx_rx_fec_nque_in_cnt_get( &rxfecnqueincnt);
        if(!err) bdmf_session_print(session, "ag_drv_xpcsrx_rx_fec_nque_in_cnt_get( %u)\n", rxfecnqueincnt);
        if(err || rxfecnqueincnt!=gtmv(m, 32))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint32_t rxfecnqueoutcnt=gtmv(m, 32);
        if(!err) bdmf_session_print(session, "ag_drv_xpcsrx_rx_fec_nque_out_cnt_set( %u)\n", rxfecnqueoutcnt);
        if(!err) ag_drv_xpcsrx_rx_fec_nque_out_cnt_set(rxfecnqueoutcnt);
        if(!err) ag_drv_xpcsrx_rx_fec_nque_out_cnt_get( &rxfecnqueoutcnt);
        if(!err) bdmf_session_print(session, "ag_drv_xpcsrx_rx_fec_nque_out_cnt_get( %u)\n", rxfecnqueoutcnt);
        if(err || rxfecnqueoutcnt!=gtmv(m, 32))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint32_t rxidlestartcnt=gtmv(m, 32);
        if(!err) bdmf_session_print(session, "ag_drv_xpcsrx_rx_idle_start_cnt_set( %u)\n", rxidlestartcnt);
        if(!err) ag_drv_xpcsrx_rx_idle_start_cnt_set(rxidlestartcnt);
        if(!err) ag_drv_xpcsrx_rx_idle_start_cnt_get( &rxidlestartcnt);
        if(!err) bdmf_session_print(session, "ag_drv_xpcsrx_rx_idle_start_cnt_get( %u)\n", rxidlestartcnt);
        if(err || rxidlestartcnt!=gtmv(m, 32))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint32_t rxidlestopcnt=gtmv(m, 32);
        if(!err) bdmf_session_print(session, "ag_drv_xpcsrx_rx_idle_stop_cnt_set( %u)\n", rxidlestopcnt);
        if(!err) ag_drv_xpcsrx_rx_idle_stop_cnt_set(rxidlestopcnt);
        if(!err) ag_drv_xpcsrx_rx_idle_stop_cnt_get( &rxidlestopcnt);
        if(!err) bdmf_session_print(session, "ag_drv_xpcsrx_rx_idle_stop_cnt_get( %u)\n", rxidlestopcnt);
        if(err || rxidlestopcnt!=gtmv(m, 32))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint32_t cfgxpcsrxfeccorintval=gtmv(m, 24);
        if(!err) bdmf_session_print(session, "ag_drv_xpcsrx_rx_fec_cor_intval_set( %u)\n", cfgxpcsrxfeccorintval);
        if(!err) ag_drv_xpcsrx_rx_fec_cor_intval_set(cfgxpcsrxfeccorintval);
        if(!err) ag_drv_xpcsrx_rx_fec_cor_intval_get( &cfgxpcsrxfeccorintval);
        if(!err) bdmf_session_print(session, "ag_drv_xpcsrx_rx_fec_cor_intval_get( %u)\n", cfgxpcsrxfeccorintval);
        if(err || cfgxpcsrxfeccorintval!=gtmv(m, 24))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint32_t cfgxpcsrxfeccortresh=gtmv(m, 24);
        if(!err) bdmf_session_print(session, "ag_drv_xpcsrx_rx_fec_cor_tresh_set( %u)\n", cfgxpcsrxfeccortresh);
        if(!err) ag_drv_xpcsrx_rx_fec_cor_tresh_set(cfgxpcsrxfeccortresh);
        if(!err) ag_drv_xpcsrx_rx_fec_cor_tresh_get( &cfgxpcsrxfeccortresh);
        if(!err) bdmf_session_print(session, "ag_drv_xpcsrx_rx_fec_cor_tresh_get( %u)\n", cfgxpcsrxfeccortresh);
        if(err || cfgxpcsrxfeccortresh!=gtmv(m, 24))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint32_t rxfecdeccwfailcnt=gtmv(m, 32);
        if(!err) bdmf_session_print(session, "ag_drv_xpcsrx_rx_fec_cw_fail_cnt_set( %u)\n", rxfecdeccwfailcnt);
        if(!err) ag_drv_xpcsrx_rx_fec_cw_fail_cnt_set(rxfecdeccwfailcnt);
        if(!err) ag_drv_xpcsrx_rx_fec_cw_fail_cnt_get( &rxfecdeccwfailcnt);
        if(!err) bdmf_session_print(session, "ag_drv_xpcsrx_rx_fec_cw_fail_cnt_get( %u)\n", rxfecdeccwfailcnt);
        if(err || rxfecdeccwfailcnt!=gtmv(m, 32))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint32_t rxfecdeccwtotcnt=gtmv(m, 32);
        if(!err) bdmf_session_print(session, "ag_drv_xpcsrx_rx_fec_cw_tot_cnt_set( %u)\n", rxfecdeccwtotcnt);
        if(!err) ag_drv_xpcsrx_rx_fec_cw_tot_cnt_set(rxfecdeccwtotcnt);
        if(!err) ag_drv_xpcsrx_rx_fec_cw_tot_cnt_get( &rxfecdeccwtotcnt);
        if(!err) bdmf_session_print(session, "ag_drv_xpcsrx_rx_fec_cw_tot_cnt_get( %u)\n", rxfecdeccwtotcnt);
        if(err || rxfecdeccwtotcnt!=gtmv(m, 32))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint32_t rxfecdecerrcorcntlower=gtmv(m, 32);
        if(!err) bdmf_session_print(session, "ag_drv_xpcsrx_rx_fec_correct_cnt_lower_set( %u)\n", rxfecdecerrcorcntlower);
        if(!err) ag_drv_xpcsrx_rx_fec_correct_cnt_lower_set(rxfecdecerrcorcntlower);
        if(!err) ag_drv_xpcsrx_rx_fec_correct_cnt_lower_get( &rxfecdecerrcorcntlower);
        if(!err) bdmf_session_print(session, "ag_drv_xpcsrx_rx_fec_correct_cnt_lower_get( %u)\n", rxfecdecerrcorcntlower);
        if(err || rxfecdecerrcorcntlower!=gtmv(m, 32))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint8_t rxfecdecerrcorcntupper=gtmv(m, 8);
        if(!err) ag_drv_xpcsrx_rx_fec_correct_cnt_upper_get( &rxfecdecerrcorcntupper);
        if(!err) bdmf_session_print(session, "ag_drv_xpcsrx_rx_fec_correct_cnt_upper_get( %u)\n", rxfecdecerrcorcntupper);
    }
    {
        uint8_t rxfecdecerrcorcntshadow=gtmv(m, 8);
        if(!err) bdmf_session_print(session, "ag_drv_xpcsrx_rx_fec_correct_cnt_shadow_set( %u)\n", rxfecdecerrcorcntshadow);
        if(!err) ag_drv_xpcsrx_rx_fec_correct_cnt_shadow_set(rxfecdecerrcorcntshadow);
        if(!err) ag_drv_xpcsrx_rx_fec_correct_cnt_shadow_get( &rxfecdecerrcorcntshadow);
        if(!err) bdmf_session_print(session, "ag_drv_xpcsrx_rx_fec_correct_cnt_shadow_get( %u)\n", rxfecdecerrcorcntshadow);
        if(err || rxfecdecerrcorcntshadow!=gtmv(m, 8))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint32_t rxfecdeconescorcntlower=gtmv(m, 32);
        if(!err) bdmf_session_print(session, "ag_drv_xpcsrx_rx_fec_ones_cor_cnt_lower_set( %u)\n", rxfecdeconescorcntlower);
        if(!err) ag_drv_xpcsrx_rx_fec_ones_cor_cnt_lower_set(rxfecdeconescorcntlower);
        if(!err) ag_drv_xpcsrx_rx_fec_ones_cor_cnt_lower_get( &rxfecdeconescorcntlower);
        if(!err) bdmf_session_print(session, "ag_drv_xpcsrx_rx_fec_ones_cor_cnt_lower_get( %u)\n", rxfecdeconescorcntlower);
        if(err || rxfecdeconescorcntlower!=gtmv(m, 32))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint8_t rxfecdeconescorcntupper=gtmv(m, 8);
        if(!err) ag_drv_xpcsrx_rx_fec_ones_cor_cnt_upper_get( &rxfecdeconescorcntupper);
        if(!err) bdmf_session_print(session, "ag_drv_xpcsrx_rx_fec_ones_cor_cnt_upper_get( %u)\n", rxfecdeconescorcntupper);
    }
    {
        uint8_t rxfecdeconescorcntshadow=gtmv(m, 8);
        if(!err) bdmf_session_print(session, "ag_drv_xpcsrx_rx_fec_ones_cor_cnt_shadow_set( %u)\n", rxfecdeconescorcntshadow);
        if(!err) ag_drv_xpcsrx_rx_fec_ones_cor_cnt_shadow_set(rxfecdeconescorcntshadow);
        if(!err) ag_drv_xpcsrx_rx_fec_ones_cor_cnt_shadow_get( &rxfecdeconescorcntshadow);
        if(!err) bdmf_session_print(session, "ag_drv_xpcsrx_rx_fec_ones_cor_cnt_shadow_get( %u)\n", rxfecdeconescorcntshadow);
        if(err || rxfecdeconescorcntshadow!=gtmv(m, 8))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint32_t rxfecdeczeroscorcntlower=gtmv(m, 32);
        if(!err) bdmf_session_print(session, "ag_drv_xpcsrx_rx_fec_zeros_cor_cnt_lower_set( %u)\n", rxfecdeczeroscorcntlower);
        if(!err) ag_drv_xpcsrx_rx_fec_zeros_cor_cnt_lower_set(rxfecdeczeroscorcntlower);
        if(!err) ag_drv_xpcsrx_rx_fec_zeros_cor_cnt_lower_get( &rxfecdeczeroscorcntlower);
        if(!err) bdmf_session_print(session, "ag_drv_xpcsrx_rx_fec_zeros_cor_cnt_lower_get( %u)\n", rxfecdeczeroscorcntlower);
        if(err || rxfecdeczeroscorcntlower!=gtmv(m, 32))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint8_t rxfecdeczeroscorcntupper=gtmv(m, 8);
        if(!err) ag_drv_xpcsrx_rx_fec_zeros_cor_cnt_upper_get( &rxfecdeczeroscorcntupper);
        if(!err) bdmf_session_print(session, "ag_drv_xpcsrx_rx_fec_zeros_cor_cnt_upper_get( %u)\n", rxfecdeczeroscorcntupper);
    }
    {
        uint8_t rxfecdeczeroscorcntshadow=gtmv(m, 8);
        if(!err) bdmf_session_print(session, "ag_drv_xpcsrx_rx_fec_zeros_cor_cnt_shadow_set( %u)\n", rxfecdeczeroscorcntshadow);
        if(!err) ag_drv_xpcsrx_rx_fec_zeros_cor_cnt_shadow_set(rxfecdeczeroscorcntshadow);
        if(!err) ag_drv_xpcsrx_rx_fec_zeros_cor_cnt_shadow_get( &rxfecdeczeroscorcntshadow);
        if(!err) bdmf_session_print(session, "ag_drv_xpcsrx_rx_fec_zeros_cor_cnt_shadow_get( %u)\n", rxfecdeczeroscorcntshadow);
        if(err || rxfecdeczeroscorcntshadow!=gtmv(m, 8))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint8_t rxfecstoponerrrdptr=gtmv(m, 8);
        uint8_t rxfecstoponerrwrptr=gtmv(m, 8);
        if(!err) ag_drv_xpcsrx_rx_fec_stop_on_err_read_pointer_get( &rxfecstoponerrrdptr, &rxfecstoponerrwrptr);
        if(!err) bdmf_session_print(session, "ag_drv_xpcsrx_rx_fec_stop_on_err_read_pointer_get( %u %u)\n", rxfecstoponerrrdptr, rxfecstoponerrwrptr);
    }
    {
        uint32_t rxfecstoponerrbrstloc=gtmv(m, 24);
        if(!err) ag_drv_xpcsrx_rx_fec_stop_on_err_burst_location_get( &rxfecstoponerrbrstloc);
        if(!err) bdmf_session_print(session, "ag_drv_xpcsrx_rx_fec_stop_on_err_burst_location_get( %u)\n", rxfecstoponerrbrstloc);
    }
    {
        uint32_t rx64b66bdecerrcnt=gtmv(m, 32);
        if(!err) bdmf_session_print(session, "ag_drv_xpcsrx_rx_64b66b_fail_cnt_set( %u)\n", rx64b66bdecerrcnt);
        if(!err) ag_drv_xpcsrx_rx_64b66b_fail_cnt_set(rx64b66bdecerrcnt);
        if(!err) ag_drv_xpcsrx_rx_64b66b_fail_cnt_get( &rx64b66bdecerrcnt);
        if(!err) bdmf_session_print(session, "ag_drv_xpcsrx_rx_64b66b_fail_cnt_get( %u)\n", rx64b66bdecerrcnt);
        if(err || rx64b66bdecerrcnt!=gtmv(m, 32))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint32_t rxfrmrbadshcnt=gtmv(m, 32);
        if(!err) bdmf_session_print(session, "ag_drv_xpcsrx_rx_frmr_bad_sh_cnt_set( %u)\n", rxfrmrbadshcnt);
        if(!err) ag_drv_xpcsrx_rx_frmr_bad_sh_cnt_set(rxfrmrbadshcnt);
        if(!err) ag_drv_xpcsrx_rx_frmr_bad_sh_cnt_get( &rxfrmrbadshcnt);
        if(!err) bdmf_session_print(session, "ag_drv_xpcsrx_rx_frmr_bad_sh_cnt_get( %u)\n", rxfrmrbadshcnt);
        if(err || rxfrmrbadshcnt!=gtmv(m, 32))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint32_t rxtestpsudoerrcnt=gtmv(m, 32);
        if(!err) bdmf_session_print(session, "ag_drv_xpcsrx_rx_psudo_cnt_set( %u)\n", rxtestpsudoerrcnt);
        if(!err) ag_drv_xpcsrx_rx_psudo_cnt_set(rxtestpsudoerrcnt);
        if(!err) ag_drv_xpcsrx_rx_psudo_cnt_get( &rxtestpsudoerrcnt);
        if(!err) bdmf_session_print(session, "ag_drv_xpcsrx_rx_psudo_cnt_get( %u)\n", rxtestpsudoerrcnt);
        if(err || rxtestpsudoerrcnt!=gtmv(m, 32))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint32_t rxtestprbserrcnt=gtmv(m, 32);
        if(!err) bdmf_session_print(session, "ag_drv_xpcsrx_rx_prbs_cnt_set( %u)\n", rxtestprbserrcnt);
        if(!err) ag_drv_xpcsrx_rx_prbs_cnt_set(rxtestprbserrcnt);
        if(!err) ag_drv_xpcsrx_rx_prbs_cnt_get( &rxtestprbserrcnt);
        if(!err) bdmf_session_print(session, "ag_drv_xpcsrx_rx_prbs_cnt_get( %u)\n", rxtestprbserrcnt);
        if(err || rxtestprbserrcnt!=gtmv(m, 32))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint32_t cfgxpcsrxfrmrberintval=gtmv(m, 24);
        if(!err) bdmf_session_print(session, "ag_drv_xpcsrx_rx_ber_intval_set( %u)\n", cfgxpcsrxfrmrberintval);
        if(!err) ag_drv_xpcsrx_rx_ber_intval_set(cfgxpcsrxfrmrberintval);
        if(!err) ag_drv_xpcsrx_rx_ber_intval_get( &cfgxpcsrxfrmrberintval);
        if(!err) bdmf_session_print(session, "ag_drv_xpcsrx_rx_ber_intval_get( %u)\n", cfgxpcsrxfrmrberintval);
        if(err || cfgxpcsrxfrmrberintval!=gtmv(m, 24))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint16_t cfgxpcsrxfrmrbertresh=gtmv(m, 9);
        if(!err) bdmf_session_print(session, "ag_drv_xpcsrx_rx_ber_tresh_set( %u)\n", cfgxpcsrxfrmrbertresh);
        if(!err) ag_drv_xpcsrx_rx_ber_tresh_set(cfgxpcsrxfrmrbertresh);
        if(!err) ag_drv_xpcsrx_rx_ber_tresh_get( &cfgxpcsrxfrmrbertresh);
        if(!err) bdmf_session_print(session, "ag_drv_xpcsrx_rx_ber_tresh_get( %u)\n", cfgxpcsrxfrmrbertresh);
        if(err || cfgxpcsrxfrmrbertresh!=gtmv(m, 9))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint32_t rx64b66bdecstartcnt=gtmv(m, 32);
        if(!err) bdmf_session_print(session, "ag_drv_xpcsrx_rx_64b66b_start_cnt_set( %u)\n", rx64b66bdecstartcnt);
        if(!err) ag_drv_xpcsrx_rx_64b66b_start_cnt_set(rx64b66bdecstartcnt);
        if(!err) ag_drv_xpcsrx_rx_64b66b_start_cnt_get( &rx64b66bdecstartcnt);
        if(!err) bdmf_session_print(session, "ag_drv_xpcsrx_rx_64b66b_start_cnt_get( %u)\n", rx64b66bdecstartcnt);
        if(err || rx64b66bdecstartcnt!=gtmv(m, 32))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint32_t rxidlegoodpktcnt=gtmv(m, 32);
        if(!err) bdmf_session_print(session, "ag_drv_xpcsrx_rx_idle_good_pkt_cnt_set( %u)\n", rxidlegoodpktcnt);
        if(!err) ag_drv_xpcsrx_rx_idle_good_pkt_cnt_set(rxidlegoodpktcnt);
        if(!err) ag_drv_xpcsrx_rx_idle_good_pkt_cnt_get( &rxidlegoodpktcnt);
        if(!err) bdmf_session_print(session, "ag_drv_xpcsrx_rx_idle_good_pkt_cnt_get( %u)\n", rxidlegoodpktcnt);
        if(err || rxidlegoodpktcnt!=gtmv(m, 32))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint32_t rxidleerrpktcnt=gtmv(m, 32);
        if(!err) bdmf_session_print(session, "ag_drv_xpcsrx_rx_idle_err_pkt_cnt_set( %u)\n", rxidleerrpktcnt);
        if(!err) ag_drv_xpcsrx_rx_idle_err_pkt_cnt_set(rxidleerrpktcnt);
        if(!err) ag_drv_xpcsrx_rx_idle_err_pkt_cnt_get( &rxidleerrpktcnt);
        if(!err) bdmf_session_print(session, "ag_drv_xpcsrx_rx_idle_err_pkt_cnt_get( %u)\n", rxidleerrpktcnt);
        if(err || rxidleerrpktcnt!=gtmv(m, 32))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint32_t rx64b66bdecstopcnt=gtmv(m, 32);
        if(!err) bdmf_session_print(session, "ag_drv_xpcsrx_rx_64b66b_stop_cnt_set( %u)\n", rx64b66bdecstopcnt);
        if(!err) ag_drv_xpcsrx_rx_64b66b_stop_cnt_set(rx64b66bdecstopcnt);
        if(!err) ag_drv_xpcsrx_rx_64b66b_stop_cnt_get( &rx64b66bdecstopcnt);
        if(!err) bdmf_session_print(session, "ag_drv_xpcsrx_rx_64b66b_stop_cnt_get( %u)\n", rx64b66bdecstopcnt);
        if(err || rx64b66bdecstopcnt!=gtmv(m, 32))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint32_t rxburstseqoutofordercnt=gtmv(m, 32);
        if(!err) bdmf_session_print(session, "ag_drv_xpcsrx_rx_burst_out_odr_cnt_set( %u)\n", rxburstseqoutofordercnt);
        if(!err) ag_drv_xpcsrx_rx_burst_out_odr_cnt_set(rxburstseqoutofordercnt);
        if(!err) ag_drv_xpcsrx_rx_burst_out_odr_cnt_get( &rxburstseqoutofordercnt);
        if(!err) bdmf_session_print(session, "ag_drv_xpcsrx_rx_burst_out_odr_cnt_get( %u)\n", rxburstseqoutofordercnt);
        if(err || rxburstseqoutofordercnt!=gtmv(m, 32))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint16_t rxidlelastdacnt=gtmv(m, 9);
        uint16_t rxidledacnt=gtmv(m, 9);
        if(!err) ag_drv_xpcsrx_rx_idle_da_jit_dly_get( &rxidlelastdacnt, &rxidledacnt);
        if(!err) bdmf_session_print(session, "ag_drv_xpcsrx_rx_idle_da_jit_dly_get( %u %u)\n", rxidlelastdacnt, rxidledacnt);
    }
    {
        xpcsrx_rx_dport_ctl rx_dport_ctl = {.xpcsrxdpbusy=gtmv(m, 1), .xpcsrxdperr=gtmv(m, 1), .cfgxpcsrxdpctl=gtmv(m, 8), .cfgxpcsrxdpramsel=gtmv(m, 4), .cfgxpcsrxdpaddr=gtmv(m, 16)};
        if(!err) bdmf_session_print(session, "ag_drv_xpcsrx_rx_dport_ctl_set( %u %u %u %u %u)\n", rx_dport_ctl.xpcsrxdpbusy, rx_dport_ctl.xpcsrxdperr, rx_dport_ctl.cfgxpcsrxdpctl, rx_dport_ctl.cfgxpcsrxdpramsel, rx_dport_ctl.cfgxpcsrxdpaddr);
        if(!err) ag_drv_xpcsrx_rx_dport_ctl_set(&rx_dport_ctl);
        if(!err) ag_drv_xpcsrx_rx_dport_ctl_get( &rx_dport_ctl);
        if(!err) bdmf_session_print(session, "ag_drv_xpcsrx_rx_dport_ctl_get( %u %u %u %u %u)\n", rx_dport_ctl.xpcsrxdpbusy, rx_dport_ctl.xpcsrxdperr, rx_dport_ctl.cfgxpcsrxdpctl, rx_dport_ctl.cfgxpcsrxdpramsel, rx_dport_ctl.cfgxpcsrxdpaddr);
        if(err || rx_dport_ctl.xpcsrxdpbusy!=gtmv(m, 1) || rx_dport_ctl.xpcsrxdperr!=gtmv(m, 1) || rx_dport_ctl.cfgxpcsrxdpctl!=gtmv(m, 8) || rx_dport_ctl.cfgxpcsrxdpramsel!=gtmv(m, 4) || rx_dport_ctl.cfgxpcsrxdpaddr!=gtmv(m, 16))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint32_t xpcsrxdpdata0=gtmv(m, 32);
        if(!err) bdmf_session_print(session, "ag_drv_xpcsrx_rx_dport_data0_set( %u)\n", xpcsrxdpdata0);
        if(!err) ag_drv_xpcsrx_rx_dport_data0_set(xpcsrxdpdata0);
        if(!err) ag_drv_xpcsrx_rx_dport_data0_get( &xpcsrxdpdata0);
        if(!err) bdmf_session_print(session, "ag_drv_xpcsrx_rx_dport_data0_get( %u)\n", xpcsrxdpdata0);
        if(err || xpcsrxdpdata0!=gtmv(m, 32))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint32_t xpcsrxdpdata1=gtmv(m, 32);
        if(!err) bdmf_session_print(session, "ag_drv_xpcsrx_rx_dport_data1_set( %u)\n", xpcsrxdpdata1);
        if(!err) ag_drv_xpcsrx_rx_dport_data1_set(xpcsrxdpdata1);
        if(!err) ag_drv_xpcsrx_rx_dport_data1_get( &xpcsrxdpdata1);
        if(!err) bdmf_session_print(session, "ag_drv_xpcsrx_rx_dport_data1_get( %u)\n", xpcsrxdpdata1);
        if(err || xpcsrxdpdata1!=gtmv(m, 32))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint32_t xpcsrxdpdata2=gtmv(m, 32);
        if(!err) bdmf_session_print(session, "ag_drv_xpcsrx_rx_dport_data2_set( %u)\n", xpcsrxdpdata2);
        if(!err) ag_drv_xpcsrx_rx_dport_data2_set(xpcsrxdpdata2);
        if(!err) ag_drv_xpcsrx_rx_dport_data2_get( &xpcsrxdpdata2);
        if(!err) bdmf_session_print(session, "ag_drv_xpcsrx_rx_dport_data2_get( %u)\n", xpcsrxdpdata2);
        if(err || xpcsrxdpdata2!=gtmv(m, 32))
            return err ? err : BDMF_ERR_IO;
    }
    {
        bdmf_boolean cfgxpcsrxidleramdpsel=gtmv(m, 1);
        bdmf_boolean cfgxpcsrxfecdecramdpsel=gtmv(m, 1);
        bdmf_boolean cfgxpcsrxfecnqueramdpsel=gtmv(m, 1);
        if(!err) bdmf_session_print(session, "ag_drv_xpcsrx_rx_dport_acc_set( %u %u %u)\n", cfgxpcsrxidleramdpsel, cfgxpcsrxfecdecramdpsel, cfgxpcsrxfecnqueramdpsel);
        if(!err) ag_drv_xpcsrx_rx_dport_acc_set(cfgxpcsrxidleramdpsel, cfgxpcsrxfecdecramdpsel, cfgxpcsrxfecnqueramdpsel);
        if(!err) ag_drv_xpcsrx_rx_dport_acc_get( &cfgxpcsrxidleramdpsel, &cfgxpcsrxfecdecramdpsel, &cfgxpcsrxfecnqueramdpsel);
        if(!err) bdmf_session_print(session, "ag_drv_xpcsrx_rx_dport_acc_get( %u %u %u)\n", cfgxpcsrxidleramdpsel, cfgxpcsrxfecdecramdpsel, cfgxpcsrxfecnqueramdpsel);
        if(err || cfgxpcsrxidleramdpsel!=gtmv(m, 1) || cfgxpcsrxfecdecramdpsel!=gtmv(m, 1) || cfgxpcsrxfecnqueramdpsel!=gtmv(m, 1))
            return err ? err : BDMF_ERR_IO;
    }
    {
        bdmf_boolean intrxidleraminitdone=gtmv(m, 1);
        bdmf_boolean intrxfecnqueraminitdone=gtmv(m, 1);
        bdmf_boolean intrxfecdecraminitdone=gtmv(m, 1);
        if(!err) bdmf_session_print(session, "ag_drv_xpcsrx_rx_ram_ecc_int_stat_set( %u %u %u)\n", intrxidleraminitdone, intrxfecnqueraminitdone, intrxfecdecraminitdone);
        if(!err) ag_drv_xpcsrx_rx_ram_ecc_int_stat_set(intrxidleraminitdone, intrxfecnqueraminitdone, intrxfecdecraminitdone);
        if(!err) ag_drv_xpcsrx_rx_ram_ecc_int_stat_get( &intrxidleraminitdone, &intrxfecnqueraminitdone, &intrxfecdecraminitdone);
        if(!err) bdmf_session_print(session, "ag_drv_xpcsrx_rx_ram_ecc_int_stat_get( %u %u %u)\n", intrxidleraminitdone, intrxfecnqueraminitdone, intrxfecdecraminitdone);
        if(err || intrxidleraminitdone!=gtmv(m, 1) || intrxfecnqueraminitdone!=gtmv(m, 1) || intrxfecdecraminitdone!=gtmv(m, 1))
            return err ? err : BDMF_ERR_IO;
    }
    {
        bdmf_boolean mskrxidleraminitdone=gtmv(m, 1);
        bdmf_boolean mskrxfecnqueraminitdone=gtmv(m, 1);
        bdmf_boolean mskrxfecdecraminitdone=gtmv(m, 1);
        if(!err) bdmf_session_print(session, "ag_drv_xpcsrx_rx_ram_ecc_int_msk_set( %u %u %u)\n", mskrxidleraminitdone, mskrxfecnqueraminitdone, mskrxfecdecraminitdone);
        if(!err) ag_drv_xpcsrx_rx_ram_ecc_int_msk_set(mskrxidleraminitdone, mskrxfecnqueraminitdone, mskrxfecdecraminitdone);
        if(!err) ag_drv_xpcsrx_rx_ram_ecc_int_msk_get( &mskrxidleraminitdone, &mskrxfecnqueraminitdone, &mskrxfecdecraminitdone);
        if(!err) bdmf_session_print(session, "ag_drv_xpcsrx_rx_ram_ecc_int_msk_get( %u %u %u)\n", mskrxidleraminitdone, mskrxfecnqueraminitdone, mskrxfecdecraminitdone);
        if(err || mskrxidleraminitdone!=gtmv(m, 1) || mskrxfecnqueraminitdone!=gtmv(m, 1) || mskrxfecdecraminitdone!=gtmv(m, 1))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint16_t tm_pd=gtmv(m, 10);
        if(!err) bdmf_session_print(session, "ag_drv_xpcsrx_rx_dft_testmode_set( %u)\n", tm_pd);
        if(!err) ag_drv_xpcsrx_rx_dft_testmode_set(tm_pd);
        if(!err) ag_drv_xpcsrx_rx_dft_testmode_get( &tm_pd);
        if(!err) bdmf_session_print(session, "ag_drv_xpcsrx_rx_dft_testmode_get( %u)\n", tm_pd);
        if(err || tm_pd!=gtmv(m, 10))
            return err ? err : BDMF_ERR_IO;
    }
    {
        bdmf_boolean cfgxpcsrxidlerampda=gtmv(m, 1);
        bdmf_boolean cfgxpcsrxfecdecrampda=gtmv(m, 1);
        if(!err) bdmf_session_print(session, "ag_drv_xpcsrx_rx_ram_power_pda_ctl0_set( %u %u)\n", cfgxpcsrxidlerampda, cfgxpcsrxfecdecrampda);
        if(!err) ag_drv_xpcsrx_rx_ram_power_pda_ctl0_set(cfgxpcsrxidlerampda, cfgxpcsrxfecdecrampda);
        if(!err) ag_drv_xpcsrx_rx_ram_power_pda_ctl0_get( &cfgxpcsrxidlerampda, &cfgxpcsrxfecdecrampda);
        if(!err) bdmf_session_print(session, "ag_drv_xpcsrx_rx_ram_power_pda_ctl0_get( %u %u)\n", cfgxpcsrxidlerampda, cfgxpcsrxfecdecrampda);
        if(err || cfgxpcsrxidlerampda!=gtmv(m, 1) || cfgxpcsrxfecdecrampda!=gtmv(m, 1))
            return err ? err : BDMF_ERR_IO;
    }
    {
        bdmf_boolean intrx64b66btrailstart=gtmv(m, 1);
        bdmf_boolean intrx64b66btwostop=gtmv(m, 1);
        bdmf_boolean intrx64b66btwostart=gtmv(m, 1);
        bdmf_boolean intrx64b66bleadstop=gtmv(m, 1);
        if(!err) bdmf_session_print(session, "ag_drv_xpcsrx_rx_int_stat1_set( %u %u %u %u)\n", intrx64b66btrailstart, intrx64b66btwostop, intrx64b66btwostart, intrx64b66bleadstop);
        if(!err) ag_drv_xpcsrx_rx_int_stat1_set(intrx64b66btrailstart, intrx64b66btwostop, intrx64b66btwostart, intrx64b66bleadstop);
        if(!err) ag_drv_xpcsrx_rx_int_stat1_get( &intrx64b66btrailstart, &intrx64b66btwostop, &intrx64b66btwostart, &intrx64b66bleadstop);
        if(!err) bdmf_session_print(session, "ag_drv_xpcsrx_rx_int_stat1_get( %u %u %u %u)\n", intrx64b66btrailstart, intrx64b66btwostop, intrx64b66btwostart, intrx64b66bleadstop);
        if(err || intrx64b66btrailstart!=gtmv(m, 1) || intrx64b66btwostop!=gtmv(m, 1) || intrx64b66btwostart!=gtmv(m, 1) || intrx64b66bleadstop!=gtmv(m, 1))
            return err ? err : BDMF_ERR_IO;
    }
    {
        bdmf_boolean mskrx64b66btrailstart=gtmv(m, 1);
        bdmf_boolean mskrx64b66btwostop=gtmv(m, 1);
        bdmf_boolean mskrx64b66btwostart=gtmv(m, 1);
        bdmf_boolean mskrx64b66bleadstop=gtmv(m, 1);
        if(!err) bdmf_session_print(session, "ag_drv_xpcsrx_rx_int_msk1_set( %u %u %u %u)\n", mskrx64b66btrailstart, mskrx64b66btwostop, mskrx64b66btwostart, mskrx64b66bleadstop);
        if(!err) ag_drv_xpcsrx_rx_int_msk1_set(mskrx64b66btrailstart, mskrx64b66btwostop, mskrx64b66btwostart, mskrx64b66bleadstop);
        if(!err) ag_drv_xpcsrx_rx_int_msk1_get( &mskrx64b66btrailstart, &mskrx64b66btwostop, &mskrx64b66btwostart, &mskrx64b66bleadstop);
        if(!err) bdmf_session_print(session, "ag_drv_xpcsrx_rx_int_msk1_get( %u %u %u %u)\n", mskrx64b66btrailstart, mskrx64b66btwostop, mskrx64b66btwostart, mskrx64b66bleadstop);
        if(err || mskrx64b66btrailstart!=gtmv(m, 1) || mskrx64b66btwostop!=gtmv(m, 1) || mskrx64b66btwostart!=gtmv(m, 1) || mskrx64b66bleadstop!=gtmv(m, 1))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint32_t cfgxpcsrxspare=gtmv(m, 32);
        if(!err) bdmf_session_print(session, "ag_drv_xpcsrx_rx_spare_ctl_set( %u)\n", cfgxpcsrxspare);
        if(!err) ag_drv_xpcsrx_rx_spare_ctl_set(cfgxpcsrxspare);
        if(!err) ag_drv_xpcsrx_rx_spare_ctl_get( &cfgxpcsrxspare);
        if(!err) bdmf_session_print(session, "ag_drv_xpcsrx_rx_spare_ctl_get( %u)\n", cfgxpcsrxspare);
        if(err || cfgxpcsrxspare!=gtmv(m, 32))
            return err ? err : BDMF_ERR_IO;
    }
    return err;
}

static int bcm_xpcsrx_cli_address(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    uint32_t i;
    uint32_t j;
    uint32_t index1_start=0;
    uint32_t index1_stop;
    uint32_t index2_start=0;
    uint32_t index2_stop;
    bdmfmon_cmd_parm_t * bdmf_parm;
    const ru_reg_rec * reg;
    const ru_block_rec * blk;
    const char * enum_string = bdmfmon_enum_parm_stringval(session, 0, parm[0].value.unumber);

    if(!enum_string)
        return BDMF_ERR_INTERNAL;

    switch (parm[0].value.unumber)
    {
    case bdmf_address_rx_rst : reg = &RU_REG(XPCSRX, RX_RST); blk = &RU_BLK(XPCSRX); break;
    case bdmf_address_rx_int_stat : reg = &RU_REG(XPCSRX, RX_INT_STAT); blk = &RU_BLK(XPCSRX); break;
    case bdmf_address_rx_int_msk : reg = &RU_REG(XPCSRX, RX_INT_MSK); blk = &RU_BLK(XPCSRX); break;
    case bdmf_address_rx_framer_ctl : reg = &RU_REG(XPCSRX, RX_FRAMER_CTL); blk = &RU_BLK(XPCSRX); break;
    case bdmf_address_rx_fec_ctl : reg = &RU_REG(XPCSRX, RX_FEC_CTL); blk = &RU_BLK(XPCSRX); break;
    case bdmf_address_rx_dscram_ctl : reg = &RU_REG(XPCSRX, RX_DSCRAM_CTL); blk = &RU_BLK(XPCSRX); break;
    case bdmf_address_rx_64b66b_ctl : reg = &RU_REG(XPCSRX, RX_64B66B_CTL); blk = &RU_BLK(XPCSRX); break;
    case bdmf_address_rx_test_ctl : reg = &RU_REG(XPCSRX, RX_TEST_CTL); blk = &RU_BLK(XPCSRX); break;
    case bdmf_address_rx_idle_rd_timer_dly : reg = &RU_REG(XPCSRX, RX_IDLE_RD_TIMER_DLY); blk = &RU_BLK(XPCSRX); break;
    case bdmf_address_rx_idle_gap_siz_max : reg = &RU_REG(XPCSRX, RX_IDLE_GAP_SIZ_MAX); blk = &RU_BLK(XPCSRX); break;
    case bdmf_address_rx_framer_lk_max : reg = &RU_REG(XPCSRX, RX_FRAMER_LK_MAX); blk = &RU_BLK(XPCSRX); break;
    case bdmf_address_rx_framer_unlk_max : reg = &RU_REG(XPCSRX, RX_FRAMER_UNLK_MAX); blk = &RU_BLK(XPCSRX); break;
    case bdmf_address_rx_framer_bd_sh : reg = &RU_REG(XPCSRX, RX_FRAMER_BD_SH); blk = &RU_BLK(XPCSRX); break;
    case bdmf_address_rx_framer_bd_lo : reg = &RU_REG(XPCSRX, RX_FRAMER_BD_LO); blk = &RU_BLK(XPCSRX); break;
    case bdmf_address_rx_framer_bd_hi : reg = &RU_REG(XPCSRX, RX_FRAMER_BD_HI); blk = &RU_BLK(XPCSRX); break;
    case bdmf_address_rx_framer_ebd_sh : reg = &RU_REG(XPCSRX, RX_FRAMER_EBD_SH); blk = &RU_BLK(XPCSRX); break;
    case bdmf_address_rx_framer_ebd_lo : reg = &RU_REG(XPCSRX, RX_FRAMER_EBD_LO); blk = &RU_BLK(XPCSRX); break;
    case bdmf_address_rx_framer_ebd_hi : reg = &RU_REG(XPCSRX, RX_FRAMER_EBD_HI); blk = &RU_BLK(XPCSRX); break;
    case bdmf_address_rx_status : reg = &RU_REG(XPCSRX, RX_STATUS); blk = &RU_BLK(XPCSRX); break;
    case bdmf_address_rx_framer_lk_ulk_max : reg = &RU_REG(XPCSRX, RX_FRAMER_LK_ULK_MAX); blk = &RU_BLK(XPCSRX); break;
    case bdmf_address_rx_framer_sp_sh : reg = &RU_REG(XPCSRX, RX_FRAMER_SP_SH); blk = &RU_BLK(XPCSRX); break;
    case bdmf_address_rx_framer_sp_lo : reg = &RU_REG(XPCSRX, RX_FRAMER_SP_LO); blk = &RU_BLK(XPCSRX); break;
    case bdmf_address_rx_framer_sp_hi : reg = &RU_REG(XPCSRX, RX_FRAMER_SP_HI); blk = &RU_BLK(XPCSRX); break;
    case bdmf_address_rx_framer_state : reg = &RU_REG(XPCSRX, RX_FRAMER_STATE); blk = &RU_BLK(XPCSRX); break;
    case bdmf_address_rx_framer_bd_ebd_ham : reg = &RU_REG(XPCSRX, RX_FRAMER_BD_EBD_HAM); blk = &RU_BLK(XPCSRX); break;
    case bdmf_address_rx_framer_misbrst_cnt : reg = &RU_REG(XPCSRX, RX_FRAMER_MISBRST_CNT); blk = &RU_BLK(XPCSRX); break;
    case bdmf_address_rx_framer_bd_err : reg = &RU_REG(XPCSRX, RX_FRAMER_BD_ERR); blk = &RU_BLK(XPCSRX); break;
    case bdmf_address_rx_framer_rogue_ctl : reg = &RU_REG(XPCSRX, RX_FRAMER_ROGUE_CTL); blk = &RU_BLK(XPCSRX); break;
    case bdmf_address_rx_framer_nolock_ctl : reg = &RU_REG(XPCSRX, RX_FRAMER_NOLOCK_CTL); blk = &RU_BLK(XPCSRX); break;
    case bdmf_address_rx_64b66b_ipg_det_cnt : reg = &RU_REG(XPCSRX, RX_64B66B_IPG_DET_CNT); blk = &RU_BLK(XPCSRX); break;
    case bdmf_address_rx_fec_nque_in_cnt : reg = &RU_REG(XPCSRX, RX_FEC_NQUE_IN_CNT); blk = &RU_BLK(XPCSRX); break;
    case bdmf_address_rx_fec_nque_out_cnt : reg = &RU_REG(XPCSRX, RX_FEC_NQUE_OUT_CNT); blk = &RU_BLK(XPCSRX); break;
    case bdmf_address_rx_idle_start_cnt : reg = &RU_REG(XPCSRX, RX_IDLE_START_CNT); blk = &RU_BLK(XPCSRX); break;
    case bdmf_address_rx_idle_stop_cnt : reg = &RU_REG(XPCSRX, RX_IDLE_STOP_CNT); blk = &RU_BLK(XPCSRX); break;
    case bdmf_address_rx_fec_cor_intval : reg = &RU_REG(XPCSRX, RX_FEC_COR_INTVAL); blk = &RU_BLK(XPCSRX); break;
    case bdmf_address_rx_fec_cor_tresh : reg = &RU_REG(XPCSRX, RX_FEC_COR_TRESH); blk = &RU_BLK(XPCSRX); break;
    case bdmf_address_rx_fec_cw_fail_cnt : reg = &RU_REG(XPCSRX, RX_FEC_CW_FAIL_CNT); blk = &RU_BLK(XPCSRX); break;
    case bdmf_address_rx_fec_cw_tot_cnt : reg = &RU_REG(XPCSRX, RX_FEC_CW_TOT_CNT); blk = &RU_BLK(XPCSRX); break;
    case bdmf_address_rx_fec_correct_cnt_lower : reg = &RU_REG(XPCSRX, RX_FEC_CORRECT_CNT_LOWER); blk = &RU_BLK(XPCSRX); break;
    case bdmf_address_rx_fec_correct_cnt_upper : reg = &RU_REG(XPCSRX, RX_FEC_CORRECT_CNT_UPPER); blk = &RU_BLK(XPCSRX); break;
    case bdmf_address_rx_fec_correct_cnt_shadow : reg = &RU_REG(XPCSRX, RX_FEC_CORRECT_CNT_SHADOW); blk = &RU_BLK(XPCSRX); break;
    case bdmf_address_rx_fec_ones_cor_cnt_lower : reg = &RU_REG(XPCSRX, RX_FEC_ONES_COR_CNT_LOWER); blk = &RU_BLK(XPCSRX); break;
    case bdmf_address_rx_fec_ones_cor_cnt_upper : reg = &RU_REG(XPCSRX, RX_FEC_ONES_COR_CNT_UPPER); blk = &RU_BLK(XPCSRX); break;
    case bdmf_address_rx_fec_ones_cor_cnt_shadow : reg = &RU_REG(XPCSRX, RX_FEC_ONES_COR_CNT_SHADOW); blk = &RU_BLK(XPCSRX); break;
    case bdmf_address_rx_fec_zeros_cor_cnt_lower : reg = &RU_REG(XPCSRX, RX_FEC_ZEROS_COR_CNT_LOWER); blk = &RU_BLK(XPCSRX); break;
    case bdmf_address_rx_fec_zeros_cor_cnt_upper : reg = &RU_REG(XPCSRX, RX_FEC_ZEROS_COR_CNT_UPPER); blk = &RU_BLK(XPCSRX); break;
    case bdmf_address_rx_fec_zeros_cor_cnt_shadow : reg = &RU_REG(XPCSRX, RX_FEC_ZEROS_COR_CNT_SHADOW); blk = &RU_BLK(XPCSRX); break;
    case bdmf_address_rx_fec_stop_on_err_read_pointer : reg = &RU_REG(XPCSRX, RX_FEC_STOP_ON_ERR_READ_POINTER); blk = &RU_BLK(XPCSRX); break;
    case bdmf_address_rx_fec_stop_on_err_burst_location : reg = &RU_REG(XPCSRX, RX_FEC_STOP_ON_ERR_BURST_LOCATION); blk = &RU_BLK(XPCSRX); break;
    case bdmf_address_rx_64b66b_fail_cnt : reg = &RU_REG(XPCSRX, RX_64B66B_FAIL_CNT); blk = &RU_BLK(XPCSRX); break;
    case bdmf_address_rx_frmr_bad_sh_cnt : reg = &RU_REG(XPCSRX, RX_FRMR_BAD_SH_CNT); blk = &RU_BLK(XPCSRX); break;
    case bdmf_address_rx_psudo_cnt : reg = &RU_REG(XPCSRX, RX_PSUDO_CNT); blk = &RU_BLK(XPCSRX); break;
    case bdmf_address_rx_prbs_cnt : reg = &RU_REG(XPCSRX, RX_PRBS_CNT); blk = &RU_BLK(XPCSRX); break;
    case bdmf_address_rx_ber_intval : reg = &RU_REG(XPCSRX, RX_BER_INTVAL); blk = &RU_BLK(XPCSRX); break;
    case bdmf_address_rx_ber_tresh : reg = &RU_REG(XPCSRX, RX_BER_TRESH); blk = &RU_BLK(XPCSRX); break;
    case bdmf_address_rx_64b66b_start_cnt : reg = &RU_REG(XPCSRX, RX_64B66B_START_CNT); blk = &RU_BLK(XPCSRX); break;
    case bdmf_address_rx_idle_good_pkt_cnt : reg = &RU_REG(XPCSRX, RX_IDLE_GOOD_PKT_CNT); blk = &RU_BLK(XPCSRX); break;
    case bdmf_address_rx_idle_err_pkt_cnt : reg = &RU_REG(XPCSRX, RX_IDLE_ERR_PKT_CNT); blk = &RU_BLK(XPCSRX); break;
    case bdmf_address_rx_64b66b_stop_cnt : reg = &RU_REG(XPCSRX, RX_64B66B_STOP_CNT); blk = &RU_BLK(XPCSRX); break;
    case bdmf_address_rx_burst_out_odr_cnt : reg = &RU_REG(XPCSRX, RX_BURST_OUT_ODR_CNT); blk = &RU_BLK(XPCSRX); break;
    case bdmf_address_rx_idle_da_jit_dly : reg = &RU_REG(XPCSRX, RX_IDLE_DA_JIT_DLY); blk = &RU_BLK(XPCSRX); break;
    case bdmf_address_rx_dport_ctl : reg = &RU_REG(XPCSRX, RX_DPORT_CTL); blk = &RU_BLK(XPCSRX); break;
    case bdmf_address_rx_dport_data0 : reg = &RU_REG(XPCSRX, RX_DPORT_DATA0); blk = &RU_BLK(XPCSRX); break;
    case bdmf_address_rx_dport_data1 : reg = &RU_REG(XPCSRX, RX_DPORT_DATA1); blk = &RU_BLK(XPCSRX); break;
    case bdmf_address_rx_dport_data2 : reg = &RU_REG(XPCSRX, RX_DPORT_DATA2); blk = &RU_BLK(XPCSRX); break;
    case bdmf_address_rx_dport_acc : reg = &RU_REG(XPCSRX, RX_DPORT_ACC); blk = &RU_BLK(XPCSRX); break;
    case bdmf_address_rx_ram_ecc_int_stat : reg = &RU_REG(XPCSRX, RX_RAM_ECC_INT_STAT); blk = &RU_BLK(XPCSRX); break;
    case bdmf_address_rx_ram_ecc_int_msk : reg = &RU_REG(XPCSRX, RX_RAM_ECC_INT_MSK); blk = &RU_BLK(XPCSRX); break;
    case bdmf_address_rx_dft_testmode : reg = &RU_REG(XPCSRX, RX_DFT_TESTMODE); blk = &RU_BLK(XPCSRX); break;
    case bdmf_address_rx_ram_power_pda_ctl0 : reg = &RU_REG(XPCSRX, RX_RAM_POWER_PDA_CTL0); blk = &RU_BLK(XPCSRX); break;
    case bdmf_address_rx_int_stat1 : reg = &RU_REG(XPCSRX, RX_INT_STAT1); blk = &RU_BLK(XPCSRX); break;
    case bdmf_address_rx_int_msk1 : reg = &RU_REG(XPCSRX, RX_INT_MSK1); blk = &RU_BLK(XPCSRX); break;
    case bdmf_address_rx_spare_ctl : reg = &RU_REG(XPCSRX, RX_SPARE_CTL); blk = &RU_BLK(XPCSRX); break;
    default :
        return BDMF_ERR_NOT_SUPPORTED;
        break;
    }
    if((bdmf_parm = bdmfmon_find_named_parm(session,"index1")))
    {
        index1_start = bdmf_parm->value.unumber;
        index1_stop = index1_start + 1;
    }
    else
        index1_stop = blk->addr_count;
    if((bdmf_parm = bdmfmon_find_named_parm(session,"index2")))
    {
        index2_start = bdmf_parm->value.unumber;
        index2_stop = index2_start + 1;
    }
    else
        index2_stop = reg->ram_count + 1;
    if(index1_stop > blk->addr_count)
    {
        bdmf_session_print(session, "index1 (%u) is out of range (%u).\n", index1_stop, blk->addr_count);
        return BDMF_ERR_RANGE;
    }
    if(index2_stop > (reg->ram_count + 1))
    {
        bdmf_session_print(session, "index2 (%u) is out of range (%u).\n", index2_stop, reg->ram_count + 1);
        return BDMF_ERR_RANGE;
    }
    if(reg->ram_count)
        for (i = index1_start; i < index1_stop; i++)
        {
            bdmf_session_print(session, "index1 = %u\n", i);
            for (j = index2_start; j < index2_stop; j++)
                bdmf_session_print(session, 	 "(%5u) 0x%08X\n", j, (uint32_t)((uint32_t*)(blk->addr[i] + reg->addr) + j));
        }
    else
        for (i = index1_start; i < index1_stop; i++)
            bdmf_session_print(session, "(%3u) 0x%08X\n", i, (uint32_t)((uint32_t*)(blk->addr[i] + reg->addr)));
    return 0;
}

bdmfmon_handle_t ag_drv_xpcsrx_cli_init(bdmfmon_handle_t driver_dir)
{
    bdmfmon_handle_t dir;

    if ((dir = bdmfmon_dir_find(driver_dir, "xpcsrx"))!=NULL)
        return dir;
    dir = bdmfmon_dir_add(driver_dir, "xpcsrx", "xpcsrx", BDMF_ACCESS_ADMIN, NULL);

    {
        static bdmfmon_cmd_parm_t set_rx_rst[]={
            BDMFMON_MAKE_PARM("cfgxpcsrxclk161rstn", "cfgxpcsrxclk161rstn", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_rx_int_stat[]={
            BDMFMON_MAKE_PARM("intrxidledajit", "intrxidledajit", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("intrxfrmrmisbrst", "intrxfrmrmisbrst", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("intrxidlesopeopgapbig", "intrxidlesopeopgapbig", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("intrxidlefrcins", "intrxidlefrcins", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("intrx64b66bminipgerr", "intrx64b66bminipgerr", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("intrxfecnquecntneq", "intrxfecnquecntneq", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("intrxidlefifoundrun", "intrxidlefifoundrun", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("intrxidlefifoovrrun", "intrxidlefifoovrrun", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("intrxfechighcor", "intrxfechighcor", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("intrxfecdecstoponerr", "intrxfecdecstoponerr", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("intrxfecdecpass", "intrxfecdecpass", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("intrxstatfrmrhighber", "intrxstatfrmrhighber", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("intrxfrmrexitbysp", "intrxfrmrexitbysp", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("intrxfrmrbadshmax", "intrxfrmrbadshmax", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("intrxdscramburstseqout", "intrxdscramburstseqout", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("intrxtestpsudolock", "intrxtestpsudolock", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("intrxtestpsudotype", "intrxtestpsudotype", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("intrxtestpsudoerr", "intrxtestpsudoerr", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("intrxtestprbslock", "intrxtestprbslock", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("intrxtestprbserr", "intrxtestprbserr", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("intrxfecpsistdecfail", "intrxfecpsistdecfail", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("intrxframerbadsh", "intrxframerbadsh", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("intrxframercwloss", "intrxframercwloss", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("intrxframercwlock", "intrxframercwlock", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("intrxfecdecfail", "intrxfecdecfail", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("intrx64b66bdecerr", "intrx64b66bdecerr", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("intrxfrmrnolocklos", "intrxfrmrnolocklos", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("intrxfrmrrogue", "intrxfrmrrogue", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("int_regs_err", "int_regs_err", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_rx_int_msk[]={
            BDMFMON_MAKE_PARM("mskrxidledajit", "mskrxidledajit", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("mskrxfrmrmisbrst", "mskrxfrmrmisbrst", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("mskrxidlesopeopgapbig", "mskrxidlesopeopgapbig", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("mskrxidlefrcins", "mskrxidlefrcins", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("mskrx64b66bminipgerr", "mskrx64b66bminipgerr", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("mskrxfecnquecntneq", "mskrxfecnquecntneq", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("mskrxidlefifoundrun", "mskrxidlefifoundrun", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("mskrxidlefifoovrrun", "mskrxidlefifoovrrun", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("mskrxfechighcor", "mskrxfechighcor", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("mskrxfecdecstoponerr", "mskrxfecdecstoponerr", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("mskrxfecdecpass", "mskrxfecdecpass", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("mskrxstatfrmrhighber", "mskrxstatfrmrhighber", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("mskrxfrmrexitbysp", "mskrxfrmrexitbysp", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("mskrxfrmrbadshmax", "mskrxfrmrbadshmax", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("mskrxdscramburstseqout", "mskrxdscramburstseqout", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("mskrxtestpsudolock", "mskrxtestpsudolock", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("mskrxtestpsudotype", "mskrxtestpsudotype", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("mskrxtestpsudoerr", "mskrxtestpsudoerr", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("mskrxtestprbslock", "mskrxtestprbslock", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("mskrxtestprbserr", "mskrxtestprbserr", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("mskrxfecpsistdecfail", "mskrxfecpsistdecfail", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("mskrxframerbadsh", "mskrxframerbadsh", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("mskrxframercwloss", "mskrxframercwloss", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("mskrxframercwlock", "mskrxframercwlock", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("mskrxfecdecfail", "mskrxfecdecfail", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("mskrx64b66bdecerr", "mskrx64b66bdecerr", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("mskrxfrmrnolocklos", "mskrxfrmrnolocklos", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("mskrxfrmrrogue", "mskrxfrmrrogue", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("msk_int_regs_err", "msk_int_regs_err", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_rx_framer_ctl[]={
            BDMFMON_MAKE_PARM("cfgxpcsrxfrmrfrcearlyalign", "cfgxpcsrxfrmrfrcearlyalign", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("cfgxpcsrxfrmrmodea", "cfgxpcsrxfrmrmodea", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("cfgxpcsrxfrmroverlapbdebdzero", "cfgxpcsrxfrmroverlapbdebdzero", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("cfgxpcsrxfrmroverlapgnten", "cfgxpcsrxfrmroverlapgnten", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("cfgxpcsrxframeburstoldalign", "cfgxpcsrxframeburstoldalign", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("cfgxpcsrxfrmrmisbrsttype", "cfgxpcsrxfrmrmisbrsttype", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("cfgxpcsrxfrmrebdvlden", "cfgxpcsrxfrmrebdvlden", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("cfgxpcsrxfrmrbdcnten", "cfgxpcsrxfrmrbdcnten", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("cfgxpcsrxfrmrburstbadshen", "cfgxpcsrxfrmrburstbadshen", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("cfgxpcsrxfrmrspulken", "cfgxpcsrxfrmrspulken", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("cfgxpcsrxframeburst", "cfgxpcsrxframeburst", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("cfgxpcsrxfrmren", "cfgxpcsrxfrmren", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("cfgxpcsrxfrmrblkfecfail", "cfgxpcsrxfrmrblkfecfail", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("cfgxpcsrxframefec", "cfgxpcsrxframefec", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_rx_fec_ctl[]={
            BDMFMON_MAKE_PARM("cfgxpcsrxfecstoponerr", "cfgxpcsrxfecstoponerr", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("cfgxpcsrxfeccntnquecw", "cfgxpcsrxfeccntnquecw", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("cfgxpcsrxfecnquerst", "cfgxpcsrxfecnquerst", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("cfgxpcsrxfeconezeromode", "cfgxpcsrxfeconezeromode", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("cfgxpcsrxfecblkcorrect", "cfgxpcsrxfecblkcorrect", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("cfgxpcsrxfecnquetestpat", "cfgxpcsrxfecnquetestpat", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("cfgxpcsrxfecfailblksh0", "cfgxpcsrxfecfailblksh0", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("cfgxpcsrxfecstrip", "cfgxpcsrxfecstrip", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("cfgxpcsrxfecbypas", "cfgxpcsrxfecbypas", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("cfgxpcsrxfecidleins", "cfgxpcsrxfecidleins", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("cfgxpcsrxfecen", "cfgxpcsrxfecen", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_rx_dscram_ctl[]={
            BDMFMON_MAKE_PARM("cfgxpcsrxdscrambypas", "cfgxpcsrxdscrambypas", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_rx_64b66b_ctl[]={
            BDMFMON_MAKE_PARM("cfgxpcsrx64b66btmask1", "cfgxpcsrx64b66btmask1", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("cfgxpcsrx64b66btmask0", "cfgxpcsrx64b66btmask0", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("cfgxpcsrx64b66bsmask1", "cfgxpcsrx64b66bsmask1", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("cfgxpcsrx64b66bsmask0", "cfgxpcsrx64b66bsmask0", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("cfgxpcsrx64b66btdlay", "cfgxpcsrx64b66btdlay", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("cfgxpcsrx64b66bdecbypas", "cfgxpcsrx64b66bdecbypas", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_rx_test_ctl[]={
            BDMFMON_MAKE_PARM("cfgxpcsrxtestprbsdeten", "cfgxpcsrxtestprbsdeten", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("cfgxpcsrxtestpsudodeten", "cfgxpcsrxtestpsudodeten", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_rx_idle_rd_timer_dly[]={
            BDMFMON_MAKE_PARM("cfgxpcsrxidlerddelaytimermax", "cfgxpcsrxidlerddelaytimermax", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_rx_idle_gap_siz_max[]={
            BDMFMON_MAKE_PARM("cfgxpcsrxidleovrsizmax", "cfgxpcsrxidleovrsizmax", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("cfgxpcsrxidlesopeopgap", "cfgxpcsrxidlesopeopgap", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_rx_framer_lk_max[]={
            BDMFMON_MAKE_PARM("cfgxpcsrxfrmrcwlktimermax", "cfgxpcsrxfrmrcwlktimermax", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_rx_framer_unlk_max[]={
            BDMFMON_MAKE_PARM("cfgxpcsrxfrmrcwunlktimermax", "cfgxpcsrxfrmrcwunlktimermax", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_rx_framer_bd_sh[]={
            BDMFMON_MAKE_PARM("cfgxpcsrxoltbdsh", "cfgxpcsrxoltbdsh", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_rx_framer_bd_lo[]={
            BDMFMON_MAKE_PARM("cfgxpcsrxoltbdlo", "cfgxpcsrxoltbdlo", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_rx_framer_bd_hi[]={
            BDMFMON_MAKE_PARM("cfgxpcsrxoltbdhi", "cfgxpcsrxoltbdhi", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_rx_framer_ebd_sh[]={
            BDMFMON_MAKE_PARM("cfgxpcsrxoltebdsh", "cfgxpcsrxoltebdsh", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_rx_framer_ebd_lo[]={
            BDMFMON_MAKE_PARM("cfgxpcsrxoltebdlo", "cfgxpcsrxoltebdlo", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_rx_framer_ebd_hi[]={
            BDMFMON_MAKE_PARM("cfgxpcsrxoltebdhi", "cfgxpcsrxoltebdhi", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_rx_framer_lk_ulk_max[]={
            BDMFMON_MAKE_PARM("cfgxpcsrxfrmrsplkmax", "cfgxpcsrxfrmrsplkmax", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("cfgxpcsrxfrmrspulkmax", "cfgxpcsrxfrmrspulkmax", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_rx_framer_sp_sh[]={
            BDMFMON_MAKE_PARM("cfgxpcsrxoltspsh", "cfgxpcsrxoltspsh", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_rx_framer_sp_lo[]={
            BDMFMON_MAKE_PARM("cfgxpcsrxoltsplo", "cfgxpcsrxoltsplo", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_rx_framer_sp_hi[]={
            BDMFMON_MAKE_PARM("cfgxpcsrxoltsphi", "cfgxpcsrxoltsphi", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_rx_framer_bd_ebd_ham[]={
            BDMFMON_MAKE_PARM("cfgxpcsrxfrmrspham", "cfgxpcsrxfrmrspham", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("cfgxpcsrxfrmrebdham", "cfgxpcsrxfrmrebdham", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("cfgxpcsrxfrmrbdham", "cfgxpcsrxfrmrbdham", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_rx_framer_misbrst_cnt[]={
            BDMFMON_MAKE_PARM("rxfrmrmisbrstcnt", "rxfrmrmisbrstcnt", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_rx_framer_rogue_ctl[]={
            BDMFMON_MAKE_PARM("cfgxpcsrxfrmrrogueen", "cfgxpcsrxfrmrrogueen", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("cfgxpcsrxfrmrroguesptresh", "cfgxpcsrxfrmrroguesptresh", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_rx_framer_nolock_ctl[]={
            BDMFMON_MAKE_PARM("cfgxpcsrxfrmrnolocklosen", "cfgxpcsrxfrmrnolocklosen", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("cfgxpcsrxfrmrnolocklosintval", "cfgxpcsrxfrmrnolocklosintval", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_rx_64b66b_ipg_det_cnt[]={
            BDMFMON_MAKE_PARM("rx64b66bipgdetcnt", "rx64b66bipgdetcnt", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_rx_fec_nque_in_cnt[]={
            BDMFMON_MAKE_PARM("rxfecnqueincnt", "rxfecnqueincnt", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_rx_fec_nque_out_cnt[]={
            BDMFMON_MAKE_PARM("rxfecnqueoutcnt", "rxfecnqueoutcnt", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_rx_idle_start_cnt[]={
            BDMFMON_MAKE_PARM("rxidlestartcnt", "rxidlestartcnt", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_rx_idle_stop_cnt[]={
            BDMFMON_MAKE_PARM("rxidlestopcnt", "rxidlestopcnt", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_rx_fec_cor_intval[]={
            BDMFMON_MAKE_PARM("cfgxpcsrxfeccorintval", "cfgxpcsrxfeccorintval", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_rx_fec_cor_tresh[]={
            BDMFMON_MAKE_PARM("cfgxpcsrxfeccortresh", "cfgxpcsrxfeccortresh", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_rx_fec_cw_fail_cnt[]={
            BDMFMON_MAKE_PARM("rxfecdeccwfailcnt", "rxfecdeccwfailcnt", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_rx_fec_cw_tot_cnt[]={
            BDMFMON_MAKE_PARM("rxfecdeccwtotcnt", "rxfecdeccwtotcnt", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_rx_fec_correct_cnt_lower[]={
            BDMFMON_MAKE_PARM("rxfecdecerrcorcntlower", "rxfecdecerrcorcntlower", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_rx_fec_correct_cnt_shadow[]={
            BDMFMON_MAKE_PARM("rxfecdecerrcorcntshadow", "rxfecdecerrcorcntshadow", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_rx_fec_ones_cor_cnt_lower[]={
            BDMFMON_MAKE_PARM("rxfecdeconescorcntlower", "rxfecdeconescorcntlower", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_rx_fec_ones_cor_cnt_shadow[]={
            BDMFMON_MAKE_PARM("rxfecdeconescorcntshadow", "rxfecdeconescorcntshadow", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_rx_fec_zeros_cor_cnt_lower[]={
            BDMFMON_MAKE_PARM("rxfecdeczeroscorcntlower", "rxfecdeczeroscorcntlower", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_rx_fec_zeros_cor_cnt_shadow[]={
            BDMFMON_MAKE_PARM("rxfecdeczeroscorcntshadow", "rxfecdeczeroscorcntshadow", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_rx_64b66b_fail_cnt[]={
            BDMFMON_MAKE_PARM("rx64b66bdecerrcnt", "rx64b66bdecerrcnt", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_rx_frmr_bad_sh_cnt[]={
            BDMFMON_MAKE_PARM("rxfrmrbadshcnt", "rxfrmrbadshcnt", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_rx_psudo_cnt[]={
            BDMFMON_MAKE_PARM("rxtestpsudoerrcnt", "rxtestpsudoerrcnt", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_rx_prbs_cnt[]={
            BDMFMON_MAKE_PARM("rxtestprbserrcnt", "rxtestprbserrcnt", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_rx_ber_intval[]={
            BDMFMON_MAKE_PARM("cfgxpcsrxfrmrberintval", "cfgxpcsrxfrmrberintval", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_rx_ber_tresh[]={
            BDMFMON_MAKE_PARM("cfgxpcsrxfrmrbertresh", "cfgxpcsrxfrmrbertresh", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_rx_64b66b_start_cnt[]={
            BDMFMON_MAKE_PARM("rx64b66bdecstartcnt", "rx64b66bdecstartcnt", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_rx_idle_good_pkt_cnt[]={
            BDMFMON_MAKE_PARM("rxidlegoodpktcnt", "rxidlegoodpktcnt", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_rx_idle_err_pkt_cnt[]={
            BDMFMON_MAKE_PARM("rxidleerrpktcnt", "rxidleerrpktcnt", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_rx_64b66b_stop_cnt[]={
            BDMFMON_MAKE_PARM("rx64b66bdecstopcnt", "rx64b66bdecstopcnt", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_rx_burst_out_odr_cnt[]={
            BDMFMON_MAKE_PARM("rxburstseqoutofordercnt", "rxburstseqoutofordercnt", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_rx_dport_ctl[]={
            BDMFMON_MAKE_PARM("xpcsrxdpbusy", "xpcsrxdpbusy", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("xpcsrxdperr", "xpcsrxdperr", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("cfgxpcsrxdpctl", "cfgxpcsrxdpctl", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("cfgxpcsrxdpramsel", "cfgxpcsrxdpramsel", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("cfgxpcsrxdpaddr", "cfgxpcsrxdpaddr", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_rx_dport_data0[]={
            BDMFMON_MAKE_PARM("xpcsrxdpdata0", "xpcsrxdpdata0", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_rx_dport_data1[]={
            BDMFMON_MAKE_PARM("xpcsrxdpdata1", "xpcsrxdpdata1", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_rx_dport_data2[]={
            BDMFMON_MAKE_PARM("xpcsrxdpdata2", "xpcsrxdpdata2", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_rx_dport_acc[]={
            BDMFMON_MAKE_PARM("cfgxpcsrxidleramdpsel", "cfgxpcsrxidleramdpsel", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("cfgxpcsrxfecdecramdpsel", "cfgxpcsrxfecdecramdpsel", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("cfgxpcsrxfecnqueramdpsel", "cfgxpcsrxfecnqueramdpsel", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_rx_ram_ecc_int_stat[]={
            BDMFMON_MAKE_PARM("intrxidleraminitdone", "intrxidleraminitdone", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("intrxfecnqueraminitdone", "intrxfecnqueraminitdone", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("intrxfecdecraminitdone", "intrxfecdecraminitdone", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_rx_ram_ecc_int_msk[]={
            BDMFMON_MAKE_PARM("mskrxidleraminitdone", "mskrxidleraminitdone", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("mskrxfecnqueraminitdone", "mskrxfecnqueraminitdone", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("mskrxfecdecraminitdone", "mskrxfecdecraminitdone", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_rx_dft_testmode[]={
            BDMFMON_MAKE_PARM("tm_pd", "tm_pd", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_rx_ram_power_pda_ctl0[]={
            BDMFMON_MAKE_PARM("cfgxpcsrxidlerampda", "cfgxpcsrxidlerampda", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("cfgxpcsrxfecdecrampda", "cfgxpcsrxfecdecrampda", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_rx_int_stat1[]={
            BDMFMON_MAKE_PARM("intrx64b66btrailstart", "intrx64b66btrailstart", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("intrx64b66btwostop", "intrx64b66btwostop", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("intrx64b66btwostart", "intrx64b66btwostart", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("intrx64b66bleadstop", "intrx64b66bleadstop", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_rx_int_msk1[]={
            BDMFMON_MAKE_PARM("mskrx64b66btrailstart", "mskrx64b66btrailstart", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("mskrx64b66btwostop", "mskrx64b66btwostop", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("mskrx64b66btwostart", "mskrx64b66btwostart", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("mskrx64b66bleadstop", "mskrx64b66bleadstop", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_rx_spare_ctl[]={
            BDMFMON_MAKE_PARM("cfgxpcsrxspare", "cfgxpcsrxspare", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_enum_val_t selector_table[] = {
            { .name="rx_rst", .val=BDMF_rx_rst, .parms=set_rx_rst },
            { .name="rx_int_stat", .val=BDMF_rx_int_stat, .parms=set_rx_int_stat },
            { .name="rx_int_msk", .val=BDMF_rx_int_msk, .parms=set_rx_int_msk },
            { .name="rx_framer_ctl", .val=BDMF_rx_framer_ctl, .parms=set_rx_framer_ctl },
            { .name="rx_fec_ctl", .val=BDMF_rx_fec_ctl, .parms=set_rx_fec_ctl },
            { .name="rx_dscram_ctl", .val=BDMF_rx_dscram_ctl, .parms=set_rx_dscram_ctl },
            { .name="rx_64b66b_ctl", .val=BDMF_rx_64b66b_ctl, .parms=set_rx_64b66b_ctl },
            { .name="rx_test_ctl", .val=BDMF_rx_test_ctl, .parms=set_rx_test_ctl },
            { .name="rx_idle_rd_timer_dly", .val=BDMF_rx_idle_rd_timer_dly, .parms=set_rx_idle_rd_timer_dly },
            { .name="rx_idle_gap_siz_max", .val=BDMF_rx_idle_gap_siz_max, .parms=set_rx_idle_gap_siz_max },
            { .name="rx_framer_lk_max", .val=BDMF_rx_framer_lk_max, .parms=set_rx_framer_lk_max },
            { .name="rx_framer_unlk_max", .val=BDMF_rx_framer_unlk_max, .parms=set_rx_framer_unlk_max },
            { .name="rx_framer_bd_sh", .val=BDMF_rx_framer_bd_sh, .parms=set_rx_framer_bd_sh },
            { .name="rx_framer_bd_lo", .val=BDMF_rx_framer_bd_lo, .parms=set_rx_framer_bd_lo },
            { .name="rx_framer_bd_hi", .val=BDMF_rx_framer_bd_hi, .parms=set_rx_framer_bd_hi },
            { .name="rx_framer_ebd_sh", .val=BDMF_rx_framer_ebd_sh, .parms=set_rx_framer_ebd_sh },
            { .name="rx_framer_ebd_lo", .val=BDMF_rx_framer_ebd_lo, .parms=set_rx_framer_ebd_lo },
            { .name="rx_framer_ebd_hi", .val=BDMF_rx_framer_ebd_hi, .parms=set_rx_framer_ebd_hi },
            { .name="rx_framer_lk_ulk_max", .val=BDMF_rx_framer_lk_ulk_max, .parms=set_rx_framer_lk_ulk_max },
            { .name="rx_framer_sp_sh", .val=BDMF_rx_framer_sp_sh, .parms=set_rx_framer_sp_sh },
            { .name="rx_framer_sp_lo", .val=BDMF_rx_framer_sp_lo, .parms=set_rx_framer_sp_lo },
            { .name="rx_framer_sp_hi", .val=BDMF_rx_framer_sp_hi, .parms=set_rx_framer_sp_hi },
            { .name="rx_framer_bd_ebd_ham", .val=BDMF_rx_framer_bd_ebd_ham, .parms=set_rx_framer_bd_ebd_ham },
            { .name="rx_framer_misbrst_cnt", .val=BDMF_rx_framer_misbrst_cnt, .parms=set_rx_framer_misbrst_cnt },
            { .name="rx_framer_rogue_ctl", .val=BDMF_rx_framer_rogue_ctl, .parms=set_rx_framer_rogue_ctl },
            { .name="rx_framer_nolock_ctl", .val=BDMF_rx_framer_nolock_ctl, .parms=set_rx_framer_nolock_ctl },
            { .name="rx_64b66b_ipg_det_cnt", .val=BDMF_rx_64b66b_ipg_det_cnt, .parms=set_rx_64b66b_ipg_det_cnt },
            { .name="rx_fec_nque_in_cnt", .val=BDMF_rx_fec_nque_in_cnt, .parms=set_rx_fec_nque_in_cnt },
            { .name="rx_fec_nque_out_cnt", .val=BDMF_rx_fec_nque_out_cnt, .parms=set_rx_fec_nque_out_cnt },
            { .name="rx_idle_start_cnt", .val=BDMF_rx_idle_start_cnt, .parms=set_rx_idle_start_cnt },
            { .name="rx_idle_stop_cnt", .val=BDMF_rx_idle_stop_cnt, .parms=set_rx_idle_stop_cnt },
            { .name="rx_fec_cor_intval", .val=BDMF_rx_fec_cor_intval, .parms=set_rx_fec_cor_intval },
            { .name="rx_fec_cor_tresh", .val=BDMF_rx_fec_cor_tresh, .parms=set_rx_fec_cor_tresh },
            { .name="rx_fec_cw_fail_cnt", .val=BDMF_rx_fec_cw_fail_cnt, .parms=set_rx_fec_cw_fail_cnt },
            { .name="rx_fec_cw_tot_cnt", .val=BDMF_rx_fec_cw_tot_cnt, .parms=set_rx_fec_cw_tot_cnt },
            { .name="rx_fec_correct_cnt_lower", .val=BDMF_rx_fec_correct_cnt_lower, .parms=set_rx_fec_correct_cnt_lower },
            { .name="rx_fec_correct_cnt_shadow", .val=BDMF_rx_fec_correct_cnt_shadow, .parms=set_rx_fec_correct_cnt_shadow },
            { .name="rx_fec_ones_cor_cnt_lower", .val=BDMF_rx_fec_ones_cor_cnt_lower, .parms=set_rx_fec_ones_cor_cnt_lower },
            { .name="rx_fec_ones_cor_cnt_shadow", .val=BDMF_rx_fec_ones_cor_cnt_shadow, .parms=set_rx_fec_ones_cor_cnt_shadow },
            { .name="rx_fec_zeros_cor_cnt_lower", .val=BDMF_rx_fec_zeros_cor_cnt_lower, .parms=set_rx_fec_zeros_cor_cnt_lower },
            { .name="rx_fec_zeros_cor_cnt_shadow", .val=BDMF_rx_fec_zeros_cor_cnt_shadow, .parms=set_rx_fec_zeros_cor_cnt_shadow },
            { .name="rx_64b66b_fail_cnt", .val=BDMF_rx_64b66b_fail_cnt, .parms=set_rx_64b66b_fail_cnt },
            { .name="rx_frmr_bad_sh_cnt", .val=BDMF_rx_frmr_bad_sh_cnt, .parms=set_rx_frmr_bad_sh_cnt },
            { .name="rx_psudo_cnt", .val=BDMF_rx_psudo_cnt, .parms=set_rx_psudo_cnt },
            { .name="rx_prbs_cnt", .val=BDMF_rx_prbs_cnt, .parms=set_rx_prbs_cnt },
            { .name="rx_ber_intval", .val=BDMF_rx_ber_intval, .parms=set_rx_ber_intval },
            { .name="rx_ber_tresh", .val=BDMF_rx_ber_tresh, .parms=set_rx_ber_tresh },
            { .name="rx_64b66b_start_cnt", .val=BDMF_rx_64b66b_start_cnt, .parms=set_rx_64b66b_start_cnt },
            { .name="rx_idle_good_pkt_cnt", .val=BDMF_rx_idle_good_pkt_cnt, .parms=set_rx_idle_good_pkt_cnt },
            { .name="rx_idle_err_pkt_cnt", .val=BDMF_rx_idle_err_pkt_cnt, .parms=set_rx_idle_err_pkt_cnt },
            { .name="rx_64b66b_stop_cnt", .val=BDMF_rx_64b66b_stop_cnt, .parms=set_rx_64b66b_stop_cnt },
            { .name="rx_burst_out_odr_cnt", .val=BDMF_rx_burst_out_odr_cnt, .parms=set_rx_burst_out_odr_cnt },
            { .name="rx_dport_ctl", .val=BDMF_rx_dport_ctl, .parms=set_rx_dport_ctl },
            { .name="rx_dport_data0", .val=BDMF_rx_dport_data0, .parms=set_rx_dport_data0 },
            { .name="rx_dport_data1", .val=BDMF_rx_dport_data1, .parms=set_rx_dport_data1 },
            { .name="rx_dport_data2", .val=BDMF_rx_dport_data2, .parms=set_rx_dport_data2 },
            { .name="rx_dport_acc", .val=BDMF_rx_dport_acc, .parms=set_rx_dport_acc },
            { .name="rx_ram_ecc_int_stat", .val=BDMF_rx_ram_ecc_int_stat, .parms=set_rx_ram_ecc_int_stat },
            { .name="rx_ram_ecc_int_msk", .val=BDMF_rx_ram_ecc_int_msk, .parms=set_rx_ram_ecc_int_msk },
            { .name="rx_dft_testmode", .val=BDMF_rx_dft_testmode, .parms=set_rx_dft_testmode },
            { .name="rx_ram_power_pda_ctl0", .val=BDMF_rx_ram_power_pda_ctl0, .parms=set_rx_ram_power_pda_ctl0 },
            { .name="rx_int_stat1", .val=BDMF_rx_int_stat1, .parms=set_rx_int_stat1 },
            { .name="rx_int_msk1", .val=BDMF_rx_int_msk1, .parms=set_rx_int_msk1 },
            { .name="rx_spare_ctl", .val=BDMF_rx_spare_ctl, .parms=set_rx_spare_ctl },
            BDMFMON_ENUM_LAST
        };
        BDMFMON_MAKE_CMD(dir, "set", "set", bcm_xpcsrx_cli_set,
            BDMFMON_MAKE_PARM_SELECTOR("purpose", "purpose", selector_table, 0));
    }
    {
        static bdmfmon_cmd_parm_t set_default[]={
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_enum_val_t selector_table[] = {
            { .name="rx_rst", .val=BDMF_rx_rst, .parms=set_default },
            { .name="rx_int_stat", .val=BDMF_rx_int_stat, .parms=set_default },
            { .name="rx_int_msk", .val=BDMF_rx_int_msk, .parms=set_default },
            { .name="rx_framer_ctl", .val=BDMF_rx_framer_ctl, .parms=set_default },
            { .name="rx_fec_ctl", .val=BDMF_rx_fec_ctl, .parms=set_default },
            { .name="rx_dscram_ctl", .val=BDMF_rx_dscram_ctl, .parms=set_default },
            { .name="rx_64b66b_ctl", .val=BDMF_rx_64b66b_ctl, .parms=set_default },
            { .name="rx_test_ctl", .val=BDMF_rx_test_ctl, .parms=set_default },
            { .name="rx_idle_rd_timer_dly", .val=BDMF_rx_idle_rd_timer_dly, .parms=set_default },
            { .name="rx_idle_gap_siz_max", .val=BDMF_rx_idle_gap_siz_max, .parms=set_default },
            { .name="rx_framer_lk_max", .val=BDMF_rx_framer_lk_max, .parms=set_default },
            { .name="rx_framer_unlk_max", .val=BDMF_rx_framer_unlk_max, .parms=set_default },
            { .name="rx_framer_bd_sh", .val=BDMF_rx_framer_bd_sh, .parms=set_default },
            { .name="rx_framer_bd_lo", .val=BDMF_rx_framer_bd_lo, .parms=set_default },
            { .name="rx_framer_bd_hi", .val=BDMF_rx_framer_bd_hi, .parms=set_default },
            { .name="rx_framer_ebd_sh", .val=BDMF_rx_framer_ebd_sh, .parms=set_default },
            { .name="rx_framer_ebd_lo", .val=BDMF_rx_framer_ebd_lo, .parms=set_default },
            { .name="rx_framer_ebd_hi", .val=BDMF_rx_framer_ebd_hi, .parms=set_default },
            { .name="rx_status", .val=BDMF_rx_status, .parms=set_default },
            { .name="rx_framer_lk_ulk_max", .val=BDMF_rx_framer_lk_ulk_max, .parms=set_default },
            { .name="rx_framer_sp_sh", .val=BDMF_rx_framer_sp_sh, .parms=set_default },
            { .name="rx_framer_sp_lo", .val=BDMF_rx_framer_sp_lo, .parms=set_default },
            { .name="rx_framer_sp_hi", .val=BDMF_rx_framer_sp_hi, .parms=set_default },
            { .name="rx_framer_state", .val=BDMF_rx_framer_state, .parms=set_default },
            { .name="rx_framer_bd_ebd_ham", .val=BDMF_rx_framer_bd_ebd_ham, .parms=set_default },
            { .name="rx_framer_misbrst_cnt", .val=BDMF_rx_framer_misbrst_cnt, .parms=set_default },
            { .name="rx_framer_bd_err", .val=BDMF_rx_framer_bd_err, .parms=set_default },
            { .name="rx_framer_rogue_ctl", .val=BDMF_rx_framer_rogue_ctl, .parms=set_default },
            { .name="rx_framer_nolock_ctl", .val=BDMF_rx_framer_nolock_ctl, .parms=set_default },
            { .name="rx_64b66b_ipg_det_cnt", .val=BDMF_rx_64b66b_ipg_det_cnt, .parms=set_default },
            { .name="rx_fec_nque_in_cnt", .val=BDMF_rx_fec_nque_in_cnt, .parms=set_default },
            { .name="rx_fec_nque_out_cnt", .val=BDMF_rx_fec_nque_out_cnt, .parms=set_default },
            { .name="rx_idle_start_cnt", .val=BDMF_rx_idle_start_cnt, .parms=set_default },
            { .name="rx_idle_stop_cnt", .val=BDMF_rx_idle_stop_cnt, .parms=set_default },
            { .name="rx_fec_cor_intval", .val=BDMF_rx_fec_cor_intval, .parms=set_default },
            { .name="rx_fec_cor_tresh", .val=BDMF_rx_fec_cor_tresh, .parms=set_default },
            { .name="rx_fec_cw_fail_cnt", .val=BDMF_rx_fec_cw_fail_cnt, .parms=set_default },
            { .name="rx_fec_cw_tot_cnt", .val=BDMF_rx_fec_cw_tot_cnt, .parms=set_default },
            { .name="rx_fec_correct_cnt_lower", .val=BDMF_rx_fec_correct_cnt_lower, .parms=set_default },
            { .name="rx_fec_correct_cnt_upper", .val=BDMF_rx_fec_correct_cnt_upper, .parms=set_default },
            { .name="rx_fec_correct_cnt_shadow", .val=BDMF_rx_fec_correct_cnt_shadow, .parms=set_default },
            { .name="rx_fec_ones_cor_cnt_lower", .val=BDMF_rx_fec_ones_cor_cnt_lower, .parms=set_default },
            { .name="rx_fec_ones_cor_cnt_upper", .val=BDMF_rx_fec_ones_cor_cnt_upper, .parms=set_default },
            { .name="rx_fec_ones_cor_cnt_shadow", .val=BDMF_rx_fec_ones_cor_cnt_shadow, .parms=set_default },
            { .name="rx_fec_zeros_cor_cnt_lower", .val=BDMF_rx_fec_zeros_cor_cnt_lower, .parms=set_default },
            { .name="rx_fec_zeros_cor_cnt_upper", .val=BDMF_rx_fec_zeros_cor_cnt_upper, .parms=set_default },
            { .name="rx_fec_zeros_cor_cnt_shadow", .val=BDMF_rx_fec_zeros_cor_cnt_shadow, .parms=set_default },
            { .name="rx_fec_stop_on_err_read_pointer", .val=BDMF_rx_fec_stop_on_err_read_pointer, .parms=set_default },
            { .name="rx_fec_stop_on_err_burst_location", .val=BDMF_rx_fec_stop_on_err_burst_location, .parms=set_default },
            { .name="rx_64b66b_fail_cnt", .val=BDMF_rx_64b66b_fail_cnt, .parms=set_default },
            { .name="rx_frmr_bad_sh_cnt", .val=BDMF_rx_frmr_bad_sh_cnt, .parms=set_default },
            { .name="rx_psudo_cnt", .val=BDMF_rx_psudo_cnt, .parms=set_default },
            { .name="rx_prbs_cnt", .val=BDMF_rx_prbs_cnt, .parms=set_default },
            { .name="rx_ber_intval", .val=BDMF_rx_ber_intval, .parms=set_default },
            { .name="rx_ber_tresh", .val=BDMF_rx_ber_tresh, .parms=set_default },
            { .name="rx_64b66b_start_cnt", .val=BDMF_rx_64b66b_start_cnt, .parms=set_default },
            { .name="rx_idle_good_pkt_cnt", .val=BDMF_rx_idle_good_pkt_cnt, .parms=set_default },
            { .name="rx_idle_err_pkt_cnt", .val=BDMF_rx_idle_err_pkt_cnt, .parms=set_default },
            { .name="rx_64b66b_stop_cnt", .val=BDMF_rx_64b66b_stop_cnt, .parms=set_default },
            { .name="rx_burst_out_odr_cnt", .val=BDMF_rx_burst_out_odr_cnt, .parms=set_default },
            { .name="rx_idle_da_jit_dly", .val=BDMF_rx_idle_da_jit_dly, .parms=set_default },
            { .name="rx_dport_ctl", .val=BDMF_rx_dport_ctl, .parms=set_default },
            { .name="rx_dport_data0", .val=BDMF_rx_dport_data0, .parms=set_default },
            { .name="rx_dport_data1", .val=BDMF_rx_dport_data1, .parms=set_default },
            { .name="rx_dport_data2", .val=BDMF_rx_dport_data2, .parms=set_default },
            { .name="rx_dport_acc", .val=BDMF_rx_dport_acc, .parms=set_default },
            { .name="rx_ram_ecc_int_stat", .val=BDMF_rx_ram_ecc_int_stat, .parms=set_default },
            { .name="rx_ram_ecc_int_msk", .val=BDMF_rx_ram_ecc_int_msk, .parms=set_default },
            { .name="rx_dft_testmode", .val=BDMF_rx_dft_testmode, .parms=set_default },
            { .name="rx_ram_power_pda_ctl0", .val=BDMF_rx_ram_power_pda_ctl0, .parms=set_default },
            { .name="rx_int_stat1", .val=BDMF_rx_int_stat1, .parms=set_default },
            { .name="rx_int_msk1", .val=BDMF_rx_int_msk1, .parms=set_default },
            { .name="rx_spare_ctl", .val=BDMF_rx_spare_ctl, .parms=set_default },
            BDMFMON_ENUM_LAST
        };
        BDMFMON_MAKE_CMD(dir, "get", "get", bcm_xpcsrx_cli_get,
            BDMFMON_MAKE_PARM_SELECTOR("purpose", "purpose", selector_table, 0));
    }
    {
        static bdmfmon_enum_val_t enum_table_test_method[] = {
            { .name="low" , .val=bdmf_test_method_low },
            { .name="mid" , .val=bdmf_test_method_mid },
            { .name="high" , .val=bdmf_test_method_high },
            BDMFMON_ENUM_LAST
        };
        BDMFMON_MAKE_CMD(dir, "test", "test", bcm_xpcsrx_cli_test,
            BDMFMON_MAKE_PARM_ENUM("method", "low: 0000, mid: 1000, high: 1111", enum_table_test_method, 0));
    }
    {
        static bdmfmon_enum_val_t enum_table_address[] = {
            { .name="RX_RST" , .val=bdmf_address_rx_rst },
            { .name="RX_INT_STAT" , .val=bdmf_address_rx_int_stat },
            { .name="RX_INT_MSK" , .val=bdmf_address_rx_int_msk },
            { .name="RX_FRAMER_CTL" , .val=bdmf_address_rx_framer_ctl },
            { .name="RX_FEC_CTL" , .val=bdmf_address_rx_fec_ctl },
            { .name="RX_DSCRAM_CTL" , .val=bdmf_address_rx_dscram_ctl },
            { .name="RX_64B66B_CTL" , .val=bdmf_address_rx_64b66b_ctl },
            { .name="RX_TEST_CTL" , .val=bdmf_address_rx_test_ctl },
            { .name="RX_IDLE_RD_TIMER_DLY" , .val=bdmf_address_rx_idle_rd_timer_dly },
            { .name="RX_IDLE_GAP_SIZ_MAX" , .val=bdmf_address_rx_idle_gap_siz_max },
            { .name="RX_FRAMER_LK_MAX" , .val=bdmf_address_rx_framer_lk_max },
            { .name="RX_FRAMER_UNLK_MAX" , .val=bdmf_address_rx_framer_unlk_max },
            { .name="RX_FRAMER_BD_SH" , .val=bdmf_address_rx_framer_bd_sh },
            { .name="RX_FRAMER_BD_LO" , .val=bdmf_address_rx_framer_bd_lo },
            { .name="RX_FRAMER_BD_HI" , .val=bdmf_address_rx_framer_bd_hi },
            { .name="RX_FRAMER_EBD_SH" , .val=bdmf_address_rx_framer_ebd_sh },
            { .name="RX_FRAMER_EBD_LO" , .val=bdmf_address_rx_framer_ebd_lo },
            { .name="RX_FRAMER_EBD_HI" , .val=bdmf_address_rx_framer_ebd_hi },
            { .name="RX_STATUS" , .val=bdmf_address_rx_status },
            { .name="RX_FRAMER_LK_ULK_MAX" , .val=bdmf_address_rx_framer_lk_ulk_max },
            { .name="RX_FRAMER_SP_SH" , .val=bdmf_address_rx_framer_sp_sh },
            { .name="RX_FRAMER_SP_LO" , .val=bdmf_address_rx_framer_sp_lo },
            { .name="RX_FRAMER_SP_HI" , .val=bdmf_address_rx_framer_sp_hi },
            { .name="RX_FRAMER_STATE" , .val=bdmf_address_rx_framer_state },
            { .name="RX_FRAMER_BD_EBD_HAM" , .val=bdmf_address_rx_framer_bd_ebd_ham },
            { .name="RX_FRAMER_MISBRST_CNT" , .val=bdmf_address_rx_framer_misbrst_cnt },
            { .name="RX_FRAMER_BD_ERR" , .val=bdmf_address_rx_framer_bd_err },
            { .name="RX_FRAMER_ROGUE_CTL" , .val=bdmf_address_rx_framer_rogue_ctl },
            { .name="RX_FRAMER_NOLOCK_CTL" , .val=bdmf_address_rx_framer_nolock_ctl },
            { .name="RX_64B66B_IPG_DET_CNT" , .val=bdmf_address_rx_64b66b_ipg_det_cnt },
            { .name="RX_FEC_NQUE_IN_CNT" , .val=bdmf_address_rx_fec_nque_in_cnt },
            { .name="RX_FEC_NQUE_OUT_CNT" , .val=bdmf_address_rx_fec_nque_out_cnt },
            { .name="RX_IDLE_START_CNT" , .val=bdmf_address_rx_idle_start_cnt },
            { .name="RX_IDLE_STOP_CNT" , .val=bdmf_address_rx_idle_stop_cnt },
            { .name="RX_FEC_COR_INTVAL" , .val=bdmf_address_rx_fec_cor_intval },
            { .name="RX_FEC_COR_TRESH" , .val=bdmf_address_rx_fec_cor_tresh },
            { .name="RX_FEC_CW_FAIL_CNT" , .val=bdmf_address_rx_fec_cw_fail_cnt },
            { .name="RX_FEC_CW_TOT_CNT" , .val=bdmf_address_rx_fec_cw_tot_cnt },
            { .name="RX_FEC_CORRECT_CNT_LOWER" , .val=bdmf_address_rx_fec_correct_cnt_lower },
            { .name="RX_FEC_CORRECT_CNT_UPPER" , .val=bdmf_address_rx_fec_correct_cnt_upper },
            { .name="RX_FEC_CORRECT_CNT_SHADOW" , .val=bdmf_address_rx_fec_correct_cnt_shadow },
            { .name="RX_FEC_ONES_COR_CNT_LOWER" , .val=bdmf_address_rx_fec_ones_cor_cnt_lower },
            { .name="RX_FEC_ONES_COR_CNT_UPPER" , .val=bdmf_address_rx_fec_ones_cor_cnt_upper },
            { .name="RX_FEC_ONES_COR_CNT_SHADOW" , .val=bdmf_address_rx_fec_ones_cor_cnt_shadow },
            { .name="RX_FEC_ZEROS_COR_CNT_LOWER" , .val=bdmf_address_rx_fec_zeros_cor_cnt_lower },
            { .name="RX_FEC_ZEROS_COR_CNT_UPPER" , .val=bdmf_address_rx_fec_zeros_cor_cnt_upper },
            { .name="RX_FEC_ZEROS_COR_CNT_SHADOW" , .val=bdmf_address_rx_fec_zeros_cor_cnt_shadow },
            { .name="RX_FEC_STOP_ON_ERR_READ_POINTER" , .val=bdmf_address_rx_fec_stop_on_err_read_pointer },
            { .name="RX_FEC_STOP_ON_ERR_BURST_LOCATION" , .val=bdmf_address_rx_fec_stop_on_err_burst_location },
            { .name="RX_64B66B_FAIL_CNT" , .val=bdmf_address_rx_64b66b_fail_cnt },
            { .name="RX_FRMR_BAD_SH_CNT" , .val=bdmf_address_rx_frmr_bad_sh_cnt },
            { .name="RX_PSUDO_CNT" , .val=bdmf_address_rx_psudo_cnt },
            { .name="RX_PRBS_CNT" , .val=bdmf_address_rx_prbs_cnt },
            { .name="RX_BER_INTVAL" , .val=bdmf_address_rx_ber_intval },
            { .name="RX_BER_TRESH" , .val=bdmf_address_rx_ber_tresh },
            { .name="RX_64B66B_START_CNT" , .val=bdmf_address_rx_64b66b_start_cnt },
            { .name="RX_IDLE_GOOD_PKT_CNT" , .val=bdmf_address_rx_idle_good_pkt_cnt },
            { .name="RX_IDLE_ERR_PKT_CNT" , .val=bdmf_address_rx_idle_err_pkt_cnt },
            { .name="RX_64B66B_STOP_CNT" , .val=bdmf_address_rx_64b66b_stop_cnt },
            { .name="RX_BURST_OUT_ODR_CNT" , .val=bdmf_address_rx_burst_out_odr_cnt },
            { .name="RX_IDLE_DA_JIT_DLY" , .val=bdmf_address_rx_idle_da_jit_dly },
            { .name="RX_DPORT_CTL" , .val=bdmf_address_rx_dport_ctl },
            { .name="RX_DPORT_DATA0" , .val=bdmf_address_rx_dport_data0 },
            { .name="RX_DPORT_DATA1" , .val=bdmf_address_rx_dport_data1 },
            { .name="RX_DPORT_DATA2" , .val=bdmf_address_rx_dport_data2 },
            { .name="RX_DPORT_ACC" , .val=bdmf_address_rx_dport_acc },
            { .name="RX_RAM_ECC_INT_STAT" , .val=bdmf_address_rx_ram_ecc_int_stat },
            { .name="RX_RAM_ECC_INT_MSK" , .val=bdmf_address_rx_ram_ecc_int_msk },
            { .name="RX_DFT_TESTMODE" , .val=bdmf_address_rx_dft_testmode },
            { .name="RX_RAM_POWER_PDA_CTL0" , .val=bdmf_address_rx_ram_power_pda_ctl0 },
            { .name="RX_INT_STAT1" , .val=bdmf_address_rx_int_stat1 },
            { .name="RX_INT_MSK1" , .val=bdmf_address_rx_int_msk1 },
            { .name="RX_SPARE_CTL" , .val=bdmf_address_rx_spare_ctl },
            BDMFMON_ENUM_LAST
        };
        BDMFMON_MAKE_CMD(dir, "address", "address", bcm_xpcsrx_cli_address,
            BDMFMON_MAKE_PARM_ENUM("method", "method", enum_table_address, 0),
            BDMFMON_MAKE_PARM("index2", "onu_id/alloc_id/port_id/etc...", BDMFMON_PARM_NUMBER, BDMFMON_PARM_FLAG_OPTIONAL));
    }
    return dir;
}
#endif /* USE_BDMF_SHELL */

