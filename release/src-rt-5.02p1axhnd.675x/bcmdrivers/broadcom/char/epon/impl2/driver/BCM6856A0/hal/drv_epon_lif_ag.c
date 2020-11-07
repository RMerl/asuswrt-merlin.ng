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
#include "drv_epon_lif_ag.h"
bdmf_error_t ag_drv_lif_pon_control_set(const lif_pon_control *pon_control)
{
    uint32_t reg_pon_control=0;

#ifdef VALIDATE_PARMS
    if(!pon_control)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((pon_control->cfgdisruntfilter >= _1BITS_MAX_VAL_) ||
       (pon_control->cfmaxcommaerrcnt >= _4BITS_MAX_VAL_) ||
       (pon_control->cfsyncsmselect >= _2BITS_MAX_VAL_) ||
       (pon_control->cfponrxforcenonfecabort >= _1BITS_MAX_VAL_) ||
       (pon_control->cfponrxforcefecabort >= _1BITS_MAX_VAL_) ||
       (pon_control->cfgrxdatabitflip >= _1BITS_MAX_VAL_) ||
       (pon_control->cfgenmpcpoampad >= _1BITS_MAX_VAL_) ||
       (pon_control->cfgentxidlepkt >= _1BITS_MAX_VAL_) ||
       (pon_control->cfenablesoftwaresynchold >= _1BITS_MAX_VAL_) ||
       (pon_control->cfenableextendsync >= _1BITS_MAX_VAL_) ||
       (pon_control->cfenablequicksync >= _1BITS_MAX_VAL_) ||
       (pon_control->cfppsen >= _1BITS_MAX_VAL_) ||
       (pon_control->cfppsclkrbc >= _1BITS_MAX_VAL_) ||
       (pon_control->cfrx2txlpback >= _1BITS_MAX_VAL_) ||
       (pon_control->cftx2rxlpback >= _1BITS_MAX_VAL_) ||
       (pon_control->cftxdataendurlon >= _1BITS_MAX_VAL_) ||
       (pon_control->cfp2pmode >= _1BITS_MAX_VAL_) ||
       (pon_control->cfp2pshortpre >= _1BITS_MAX_VAL_) ||
       (pon_control->cflaseren >= _1BITS_MAX_VAL_) ||
       (pon_control->cftxlaseron >= _1BITS_MAX_VAL_) ||
       (pon_control->cftxlaseronacthi >= _1BITS_MAX_VAL_) ||
       (pon_control->liftxrstn_pre >= _1BITS_MAX_VAL_) ||
       (pon_control->lifrxrstn_pre >= _1BITS_MAX_VAL_) ||
       (pon_control->liftxen >= _1BITS_MAX_VAL_) ||
       (pon_control->lifrxen >= _1BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_pon_control = RU_FIELD_SET(0, LIF, PON_CONTROL, CFGDISRUNTFILTER, reg_pon_control, pon_control->cfgdisruntfilter);
    reg_pon_control = RU_FIELD_SET(0, LIF, PON_CONTROL, CFMAXCOMMAERRCNT, reg_pon_control, pon_control->cfmaxcommaerrcnt);
    reg_pon_control = RU_FIELD_SET(0, LIF, PON_CONTROL, CFSYNCSMSELECT, reg_pon_control, pon_control->cfsyncsmselect);
    reg_pon_control = RU_FIELD_SET(0, LIF, PON_CONTROL, CFPONRXFORCENONFECABORT, reg_pon_control, pon_control->cfponrxforcenonfecabort);
    reg_pon_control = RU_FIELD_SET(0, LIF, PON_CONTROL, CFPONRXFORCEFECABORT, reg_pon_control, pon_control->cfponrxforcefecabort);
    reg_pon_control = RU_FIELD_SET(0, LIF, PON_CONTROL, CFGRXDATABITFLIP, reg_pon_control, pon_control->cfgrxdatabitflip);
    reg_pon_control = RU_FIELD_SET(0, LIF, PON_CONTROL, CFGENMPCPOAMPAD, reg_pon_control, pon_control->cfgenmpcpoampad);
    reg_pon_control = RU_FIELD_SET(0, LIF, PON_CONTROL, CFGENTXIDLEPKT, reg_pon_control, pon_control->cfgentxidlepkt);
    reg_pon_control = RU_FIELD_SET(0, LIF, PON_CONTROL, CFENABLESOFTWARESYNCHOLD, reg_pon_control, pon_control->cfenablesoftwaresynchold);
    reg_pon_control = RU_FIELD_SET(0, LIF, PON_CONTROL, CFENABLEEXTENDSYNC, reg_pon_control, pon_control->cfenableextendsync);
    reg_pon_control = RU_FIELD_SET(0, LIF, PON_CONTROL, CFENABLEQUICKSYNC, reg_pon_control, pon_control->cfenablequicksync);
    reg_pon_control = RU_FIELD_SET(0, LIF, PON_CONTROL, CFPPSEN, reg_pon_control, pon_control->cfppsen);
    reg_pon_control = RU_FIELD_SET(0, LIF, PON_CONTROL, CFPPSCLKRBC, reg_pon_control, pon_control->cfppsclkrbc);
    reg_pon_control = RU_FIELD_SET(0, LIF, PON_CONTROL, CFRX2TXLPBACK, reg_pon_control, pon_control->cfrx2txlpback);
    reg_pon_control = RU_FIELD_SET(0, LIF, PON_CONTROL, CFTX2RXLPBACK, reg_pon_control, pon_control->cftx2rxlpback);
    reg_pon_control = RU_FIELD_SET(0, LIF, PON_CONTROL, CFTXDATAENDURLON, reg_pon_control, pon_control->cftxdataendurlon);
    reg_pon_control = RU_FIELD_SET(0, LIF, PON_CONTROL, CFP2PMODE, reg_pon_control, pon_control->cfp2pmode);
    reg_pon_control = RU_FIELD_SET(0, LIF, PON_CONTROL, CFP2PSHORTPRE, reg_pon_control, pon_control->cfp2pshortpre);
    reg_pon_control = RU_FIELD_SET(0, LIF, PON_CONTROL, CFLASEREN, reg_pon_control, pon_control->cflaseren);
    reg_pon_control = RU_FIELD_SET(0, LIF, PON_CONTROL, CFTXLASERON, reg_pon_control, pon_control->cftxlaseron);
    reg_pon_control = RU_FIELD_SET(0, LIF, PON_CONTROL, CFTXLASERONACTHI, reg_pon_control, pon_control->cftxlaseronacthi);
    reg_pon_control = RU_FIELD_SET(0, LIF, PON_CONTROL, LIFTXRSTN_PRE, reg_pon_control, pon_control->liftxrstn_pre);
    reg_pon_control = RU_FIELD_SET(0, LIF, PON_CONTROL, LIFRXRSTN_PRE, reg_pon_control, pon_control->lifrxrstn_pre);
    reg_pon_control = RU_FIELD_SET(0, LIF, PON_CONTROL, LIFTXEN, reg_pon_control, pon_control->liftxen);
    reg_pon_control = RU_FIELD_SET(0, LIF, PON_CONTROL, LIFRXEN, reg_pon_control, pon_control->lifrxen);

    RU_REG_WRITE(0, LIF, PON_CONTROL, reg_pon_control);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_lif_pon_control_get(lif_pon_control *pon_control)
{
    uint32_t reg_pon_control=0;

#ifdef VALIDATE_PARMS
    if(!pon_control)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, LIF, PON_CONTROL, reg_pon_control);

    pon_control->cfgdisruntfilter = RU_FIELD_GET(0, LIF, PON_CONTROL, CFGDISRUNTFILTER, reg_pon_control);
    pon_control->cfmaxcommaerrcnt = RU_FIELD_GET(0, LIF, PON_CONTROL, CFMAXCOMMAERRCNT, reg_pon_control);
    pon_control->cfsyncsmselect = RU_FIELD_GET(0, LIF, PON_CONTROL, CFSYNCSMSELECT, reg_pon_control);
    pon_control->cfponrxforcenonfecabort = RU_FIELD_GET(0, LIF, PON_CONTROL, CFPONRXFORCENONFECABORT, reg_pon_control);
    pon_control->cfponrxforcefecabort = RU_FIELD_GET(0, LIF, PON_CONTROL, CFPONRXFORCEFECABORT, reg_pon_control);
    pon_control->cfgrxdatabitflip = RU_FIELD_GET(0, LIF, PON_CONTROL, CFGRXDATABITFLIP, reg_pon_control);
    pon_control->cfgenmpcpoampad = RU_FIELD_GET(0, LIF, PON_CONTROL, CFGENMPCPOAMPAD, reg_pon_control);
    pon_control->cfgentxidlepkt = RU_FIELD_GET(0, LIF, PON_CONTROL, CFGENTXIDLEPKT, reg_pon_control);
    pon_control->cfenablesoftwaresynchold = RU_FIELD_GET(0, LIF, PON_CONTROL, CFENABLESOFTWARESYNCHOLD, reg_pon_control);
    pon_control->cfenableextendsync = RU_FIELD_GET(0, LIF, PON_CONTROL, CFENABLEEXTENDSYNC, reg_pon_control);
    pon_control->cfenablequicksync = RU_FIELD_GET(0, LIF, PON_CONTROL, CFENABLEQUICKSYNC, reg_pon_control);
    pon_control->cfppsen = RU_FIELD_GET(0, LIF, PON_CONTROL, CFPPSEN, reg_pon_control);
    pon_control->cfppsclkrbc = RU_FIELD_GET(0, LIF, PON_CONTROL, CFPPSCLKRBC, reg_pon_control);
    pon_control->cfrx2txlpback = RU_FIELD_GET(0, LIF, PON_CONTROL, CFRX2TXLPBACK, reg_pon_control);
    pon_control->cftx2rxlpback = RU_FIELD_GET(0, LIF, PON_CONTROL, CFTX2RXLPBACK, reg_pon_control);
    pon_control->cftxdataendurlon = RU_FIELD_GET(0, LIF, PON_CONTROL, CFTXDATAENDURLON, reg_pon_control);
    pon_control->cfp2pmode = RU_FIELD_GET(0, LIF, PON_CONTROL, CFP2PMODE, reg_pon_control);
    pon_control->cfp2pshortpre = RU_FIELD_GET(0, LIF, PON_CONTROL, CFP2PSHORTPRE, reg_pon_control);
    pon_control->cflaseren = RU_FIELD_GET(0, LIF, PON_CONTROL, CFLASEREN, reg_pon_control);
    pon_control->cftxlaseron = RU_FIELD_GET(0, LIF, PON_CONTROL, CFTXLASERON, reg_pon_control);
    pon_control->cftxlaseronacthi = RU_FIELD_GET(0, LIF, PON_CONTROL, CFTXLASERONACTHI, reg_pon_control);
    pon_control->liftxrstn_pre = RU_FIELD_GET(0, LIF, PON_CONTROL, LIFTXRSTN_PRE, reg_pon_control);
    pon_control->lifrxrstn_pre = RU_FIELD_GET(0, LIF, PON_CONTROL, LIFRXRSTN_PRE, reg_pon_control);
    pon_control->liftxen = RU_FIELD_GET(0, LIF, PON_CONTROL, LIFTXEN, reg_pon_control);
    pon_control->lifrxen = RU_FIELD_GET(0, LIF, PON_CONTROL, LIFRXEN, reg_pon_control);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_lif_pon_inter_op_control_set(const lif_pon_inter_op_control *pon_inter_op_control)
{
    uint32_t reg_pon_inter_op_control=0;

#ifdef VALIDATE_PARMS
    if(!pon_inter_op_control)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((pon_inter_op_control->cfipgfilter >= _4BITS_MAX_VAL_) ||
       (pon_inter_op_control->cfdisableloslaserblock >= _1BITS_MAX_VAL_) ||
       (pon_inter_op_control->cfgllidpromiscuousmode >= _1BITS_MAX_VAL_) ||
       (pon_inter_op_control->cfgllidmodmsk >= _1BITS_MAX_VAL_) ||
       (pon_inter_op_control->cfusefecipg >= _1BITS_MAX_VAL_) ||
       (pon_inter_op_control->cfrxcrc8invchk >= _1BITS_MAX_VAL_) ||
       (pon_inter_op_control->cfrxcrc8bitswap >= _1BITS_MAX_VAL_) ||
       (pon_inter_op_control->cfrxcrc8msb2lsb >= _1BITS_MAX_VAL_) ||
       (pon_inter_op_control->cfrxcrc8disable >= _1BITS_MAX_VAL_) ||
       (pon_inter_op_control->cftxllidbit15set >= _1BITS_MAX_VAL_) ||
       (pon_inter_op_control->cftxcrc8inv >= _1BITS_MAX_VAL_) ||
       (pon_inter_op_control->cftxcrc8bad >= _1BITS_MAX_VAL_) ||
       (pon_inter_op_control->cftxcrc8bitswap >= _1BITS_MAX_VAL_) ||
       (pon_inter_op_control->cftxcrc8msb2lsb >= _1BITS_MAX_VAL_) ||
       (pon_inter_op_control->cftxshortpre >= _1BITS_MAX_VAL_) ||
       (pon_inter_op_control->cftxipgcnt >= _4BITS_MAX_VAL_) ||
       (pon_inter_op_control->cftxaasynclen >= _4BITS_MAX_VAL_) ||
       (pon_inter_op_control->cftxpipedelay >= _4BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_pon_inter_op_control = RU_FIELD_SET(0, LIF, PON_INTER_OP_CONTROL, CFIPGFILTER, reg_pon_inter_op_control, pon_inter_op_control->cfipgfilter);
    reg_pon_inter_op_control = RU_FIELD_SET(0, LIF, PON_INTER_OP_CONTROL, CFDISABLELOSLASERBLOCK, reg_pon_inter_op_control, pon_inter_op_control->cfdisableloslaserblock);
    reg_pon_inter_op_control = RU_FIELD_SET(0, LIF, PON_INTER_OP_CONTROL, CFGLLIDPROMISCUOUSMODE, reg_pon_inter_op_control, pon_inter_op_control->cfgllidpromiscuousmode);
    reg_pon_inter_op_control = RU_FIELD_SET(0, LIF, PON_INTER_OP_CONTROL, CFGLLIDMODMSK, reg_pon_inter_op_control, pon_inter_op_control->cfgllidmodmsk);
    reg_pon_inter_op_control = RU_FIELD_SET(0, LIF, PON_INTER_OP_CONTROL, CFUSEFECIPG, reg_pon_inter_op_control, pon_inter_op_control->cfusefecipg);
    reg_pon_inter_op_control = RU_FIELD_SET(0, LIF, PON_INTER_OP_CONTROL, CFRXCRC8INVCHK, reg_pon_inter_op_control, pon_inter_op_control->cfrxcrc8invchk);
    reg_pon_inter_op_control = RU_FIELD_SET(0, LIF, PON_INTER_OP_CONTROL, CFRXCRC8BITSWAP, reg_pon_inter_op_control, pon_inter_op_control->cfrxcrc8bitswap);
    reg_pon_inter_op_control = RU_FIELD_SET(0, LIF, PON_INTER_OP_CONTROL, CFRXCRC8MSB2LSB, reg_pon_inter_op_control, pon_inter_op_control->cfrxcrc8msb2lsb);
    reg_pon_inter_op_control = RU_FIELD_SET(0, LIF, PON_INTER_OP_CONTROL, CFRXCRC8DISABLE, reg_pon_inter_op_control, pon_inter_op_control->cfrxcrc8disable);
    reg_pon_inter_op_control = RU_FIELD_SET(0, LIF, PON_INTER_OP_CONTROL, CFTXLLIDBIT15SET, reg_pon_inter_op_control, pon_inter_op_control->cftxllidbit15set);
    reg_pon_inter_op_control = RU_FIELD_SET(0, LIF, PON_INTER_OP_CONTROL, CFTXCRC8INV, reg_pon_inter_op_control, pon_inter_op_control->cftxcrc8inv);
    reg_pon_inter_op_control = RU_FIELD_SET(0, LIF, PON_INTER_OP_CONTROL, CFTXCRC8BAD, reg_pon_inter_op_control, pon_inter_op_control->cftxcrc8bad);
    reg_pon_inter_op_control = RU_FIELD_SET(0, LIF, PON_INTER_OP_CONTROL, CFTXCRC8BITSWAP, reg_pon_inter_op_control, pon_inter_op_control->cftxcrc8bitswap);
    reg_pon_inter_op_control = RU_FIELD_SET(0, LIF, PON_INTER_OP_CONTROL, CFTXCRC8MSB2LSB, reg_pon_inter_op_control, pon_inter_op_control->cftxcrc8msb2lsb);
    reg_pon_inter_op_control = RU_FIELD_SET(0, LIF, PON_INTER_OP_CONTROL, CFTXSHORTPRE, reg_pon_inter_op_control, pon_inter_op_control->cftxshortpre);
    reg_pon_inter_op_control = RU_FIELD_SET(0, LIF, PON_INTER_OP_CONTROL, CFTXIPGCNT, reg_pon_inter_op_control, pon_inter_op_control->cftxipgcnt);
    reg_pon_inter_op_control = RU_FIELD_SET(0, LIF, PON_INTER_OP_CONTROL, CFTXAASYNCLEN, reg_pon_inter_op_control, pon_inter_op_control->cftxaasynclen);
    reg_pon_inter_op_control = RU_FIELD_SET(0, LIF, PON_INTER_OP_CONTROL, CFTXPIPEDELAY, reg_pon_inter_op_control, pon_inter_op_control->cftxpipedelay);

    RU_REG_WRITE(0, LIF, PON_INTER_OP_CONTROL, reg_pon_inter_op_control);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_lif_pon_inter_op_control_get(lif_pon_inter_op_control *pon_inter_op_control)
{
    uint32_t reg_pon_inter_op_control=0;

#ifdef VALIDATE_PARMS
    if(!pon_inter_op_control)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, LIF, PON_INTER_OP_CONTROL, reg_pon_inter_op_control);

    pon_inter_op_control->cfipgfilter = RU_FIELD_GET(0, LIF, PON_INTER_OP_CONTROL, CFIPGFILTER, reg_pon_inter_op_control);
    pon_inter_op_control->cfdisableloslaserblock = RU_FIELD_GET(0, LIF, PON_INTER_OP_CONTROL, CFDISABLELOSLASERBLOCK, reg_pon_inter_op_control);
    pon_inter_op_control->cfgllidpromiscuousmode = RU_FIELD_GET(0, LIF, PON_INTER_OP_CONTROL, CFGLLIDPROMISCUOUSMODE, reg_pon_inter_op_control);
    pon_inter_op_control->cfgllidmodmsk = RU_FIELD_GET(0, LIF, PON_INTER_OP_CONTROL, CFGLLIDMODMSK, reg_pon_inter_op_control);
    pon_inter_op_control->cfusefecipg = RU_FIELD_GET(0, LIF, PON_INTER_OP_CONTROL, CFUSEFECIPG, reg_pon_inter_op_control);
    pon_inter_op_control->cfrxcrc8invchk = RU_FIELD_GET(0, LIF, PON_INTER_OP_CONTROL, CFRXCRC8INVCHK, reg_pon_inter_op_control);
    pon_inter_op_control->cfrxcrc8bitswap = RU_FIELD_GET(0, LIF, PON_INTER_OP_CONTROL, CFRXCRC8BITSWAP, reg_pon_inter_op_control);
    pon_inter_op_control->cfrxcrc8msb2lsb = RU_FIELD_GET(0, LIF, PON_INTER_OP_CONTROL, CFRXCRC8MSB2LSB, reg_pon_inter_op_control);
    pon_inter_op_control->cfrxcrc8disable = RU_FIELD_GET(0, LIF, PON_INTER_OP_CONTROL, CFRXCRC8DISABLE, reg_pon_inter_op_control);
    pon_inter_op_control->cftxllidbit15set = RU_FIELD_GET(0, LIF, PON_INTER_OP_CONTROL, CFTXLLIDBIT15SET, reg_pon_inter_op_control);
    pon_inter_op_control->cftxcrc8inv = RU_FIELD_GET(0, LIF, PON_INTER_OP_CONTROL, CFTXCRC8INV, reg_pon_inter_op_control);
    pon_inter_op_control->cftxcrc8bad = RU_FIELD_GET(0, LIF, PON_INTER_OP_CONTROL, CFTXCRC8BAD, reg_pon_inter_op_control);
    pon_inter_op_control->cftxcrc8bitswap = RU_FIELD_GET(0, LIF, PON_INTER_OP_CONTROL, CFTXCRC8BITSWAP, reg_pon_inter_op_control);
    pon_inter_op_control->cftxcrc8msb2lsb = RU_FIELD_GET(0, LIF, PON_INTER_OP_CONTROL, CFTXCRC8MSB2LSB, reg_pon_inter_op_control);
    pon_inter_op_control->cftxshortpre = RU_FIELD_GET(0, LIF, PON_INTER_OP_CONTROL, CFTXSHORTPRE, reg_pon_inter_op_control);
    pon_inter_op_control->cftxipgcnt = RU_FIELD_GET(0, LIF, PON_INTER_OP_CONTROL, CFTXIPGCNT, reg_pon_inter_op_control);
    pon_inter_op_control->cftxaasynclen = RU_FIELD_GET(0, LIF, PON_INTER_OP_CONTROL, CFTXAASYNCLEN, reg_pon_inter_op_control);
    pon_inter_op_control->cftxpipedelay = RU_FIELD_GET(0, LIF, PON_INTER_OP_CONTROL, CFTXPIPEDELAY, reg_pon_inter_op_control);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_lif_fec_control_set(const lif_fec_control *fec_control)
{
    uint32_t reg_fec_control=0;

#ifdef VALIDATE_PARMS
    if(!fec_control)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((fec_control->cffecrxerrorprop >= _1BITS_MAX_VAL_) ||
       (fec_control->cffecrxforcenonfecabort >= _1BITS_MAX_VAL_) ||
       (fec_control->cffecrxforcefecabort >= _1BITS_MAX_VAL_) ||
       (fec_control->cffecrxenable >= _1BITS_MAX_VAL_) ||
       (fec_control->cffectxfecperllid >= _1BITS_MAX_VAL_) ||
       (fec_control->cffectxenable >= _1BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_fec_control = RU_FIELD_SET(0, LIF, FEC_CONTROL, CFFECRXERRORPROP, reg_fec_control, fec_control->cffecrxerrorprop);
    reg_fec_control = RU_FIELD_SET(0, LIF, FEC_CONTROL, CFFECRXFORCENONFECABORT, reg_fec_control, fec_control->cffecrxforcenonfecabort);
    reg_fec_control = RU_FIELD_SET(0, LIF, FEC_CONTROL, CFFECRXFORCEFECABORT, reg_fec_control, fec_control->cffecrxforcefecabort);
    reg_fec_control = RU_FIELD_SET(0, LIF, FEC_CONTROL, CFFECRXENABLE, reg_fec_control, fec_control->cffecrxenable);
    reg_fec_control = RU_FIELD_SET(0, LIF, FEC_CONTROL, CFFECTXFECPERLLID, reg_fec_control, fec_control->cffectxfecperllid);
    reg_fec_control = RU_FIELD_SET(0, LIF, FEC_CONTROL, CFFECTXENABLE, reg_fec_control, fec_control->cffectxenable);

    RU_REG_WRITE(0, LIF, FEC_CONTROL, reg_fec_control);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_lif_fec_control_get(lif_fec_control *fec_control)
{
    uint32_t reg_fec_control=0;

#ifdef VALIDATE_PARMS
    if(!fec_control)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, LIF, FEC_CONTROL, reg_fec_control);

    fec_control->cffecrxerrorprop = RU_FIELD_GET(0, LIF, FEC_CONTROL, CFFECRXERRORPROP, reg_fec_control);
    fec_control->cffecrxforcenonfecabort = RU_FIELD_GET(0, LIF, FEC_CONTROL, CFFECRXFORCENONFECABORT, reg_fec_control);
    fec_control->cffecrxforcefecabort = RU_FIELD_GET(0, LIF, FEC_CONTROL, CFFECRXFORCEFECABORT, reg_fec_control);
    fec_control->cffecrxenable = RU_FIELD_GET(0, LIF, FEC_CONTROL, CFFECRXENABLE, reg_fec_control);
    fec_control->cffectxfecperllid = RU_FIELD_GET(0, LIF, FEC_CONTROL, CFFECTXFECPERLLID, reg_fec_control);
    fec_control->cffectxenable = RU_FIELD_GET(0, LIF, FEC_CONTROL, CFFECTXENABLE, reg_fec_control);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_lif_sec_control_set(const lif_sec_control *sec_control)
{
    uint32_t reg_sec_control=0;

#ifdef VALIDATE_PARMS
    if(!sec_control)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((sec_control->cfgdismpcpencrypt >= _1BITS_MAX_VAL_) ||
       (sec_control->cfgdisoamencrypt >= _1BITS_MAX_VAL_) ||
       (sec_control->cfgsecenshortlen >= _1BITS_MAX_VAL_) ||
       (sec_control->cfgenaereplayprct >= _1BITS_MAX_VAL_) ||
       (sec_control->cfgenlegacyrcc >= _1BITS_MAX_VAL_) ||
       (sec_control->enfakeupaes >= _1BITS_MAX_VAL_) ||
       (sec_control->enfakednaes >= _1BITS_MAX_VAL_) ||
       (sec_control->disdndasaencrpt >= _1BITS_MAX_VAL_) ||
       (sec_control->entriplechurn >= _1BITS_MAX_VAL_) ||
       (sec_control->enepnmixencrypt >= _1BITS_MAX_VAL_) ||
       (sec_control->disupdasaencrpt >= _1BITS_MAX_VAL_) ||
       (sec_control->secupencryptscheme >= _2BITS_MAX_VAL_) ||
       (sec_control->secdnencryptscheme >= _3BITS_MAX_VAL_) ||
       (sec_control->secuprstn_pre >= _1BITS_MAX_VAL_) ||
       (sec_control->secdnrstn_pre >= _1BITS_MAX_VAL_) ||
       (sec_control->secenup >= _1BITS_MAX_VAL_) ||
       (sec_control->secendn >= _1BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_sec_control = RU_FIELD_SET(0, LIF, SEC_CONTROL, CFGDISMPCPENCRYPT, reg_sec_control, sec_control->cfgdismpcpencrypt);
    reg_sec_control = RU_FIELD_SET(0, LIF, SEC_CONTROL, CFGDISOAMENCRYPT, reg_sec_control, sec_control->cfgdisoamencrypt);
    reg_sec_control = RU_FIELD_SET(0, LIF, SEC_CONTROL, CFGSECENSHORTLEN, reg_sec_control, sec_control->cfgsecenshortlen);
    reg_sec_control = RU_FIELD_SET(0, LIF, SEC_CONTROL, CFGENAEREPLAYPRCT, reg_sec_control, sec_control->cfgenaereplayprct);
    reg_sec_control = RU_FIELD_SET(0, LIF, SEC_CONTROL, CFGENLEGACYRCC, reg_sec_control, sec_control->cfgenlegacyrcc);
    reg_sec_control = RU_FIELD_SET(0, LIF, SEC_CONTROL, ENFAKEUPAES, reg_sec_control, sec_control->enfakeupaes);
    reg_sec_control = RU_FIELD_SET(0, LIF, SEC_CONTROL, ENFAKEDNAES, reg_sec_control, sec_control->enfakednaes);
    reg_sec_control = RU_FIELD_SET(0, LIF, SEC_CONTROL, CFGFECIPGLEN, reg_sec_control, sec_control->cfgfecipglen);
    reg_sec_control = RU_FIELD_SET(0, LIF, SEC_CONTROL, DISDNDASAENCRPT, reg_sec_control, sec_control->disdndasaencrpt);
    reg_sec_control = RU_FIELD_SET(0, LIF, SEC_CONTROL, ENTRIPLECHURN, reg_sec_control, sec_control->entriplechurn);
    reg_sec_control = RU_FIELD_SET(0, LIF, SEC_CONTROL, ENEPNMIXENCRYPT, reg_sec_control, sec_control->enepnmixencrypt);
    reg_sec_control = RU_FIELD_SET(0, LIF, SEC_CONTROL, DISUPDASAENCRPT, reg_sec_control, sec_control->disupdasaencrpt);
    reg_sec_control = RU_FIELD_SET(0, LIF, SEC_CONTROL, SECUPENCRYPTSCHEME, reg_sec_control, sec_control->secupencryptscheme);
    reg_sec_control = RU_FIELD_SET(0, LIF, SEC_CONTROL, SECDNENCRYPTSCHEME, reg_sec_control, sec_control->secdnencryptscheme);
    reg_sec_control = RU_FIELD_SET(0, LIF, SEC_CONTROL, SECUPRSTN_PRE, reg_sec_control, sec_control->secuprstn_pre);
    reg_sec_control = RU_FIELD_SET(0, LIF, SEC_CONTROL, SECDNRSTN_PRE, reg_sec_control, sec_control->secdnrstn_pre);
    reg_sec_control = RU_FIELD_SET(0, LIF, SEC_CONTROL, SECENUP, reg_sec_control, sec_control->secenup);
    reg_sec_control = RU_FIELD_SET(0, LIF, SEC_CONTROL, SECENDN, reg_sec_control, sec_control->secendn);

    RU_REG_WRITE(0, LIF, SEC_CONTROL, reg_sec_control);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_lif_sec_control_get(lif_sec_control *sec_control)
{
    uint32_t reg_sec_control=0;

#ifdef VALIDATE_PARMS
    if(!sec_control)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, LIF, SEC_CONTROL, reg_sec_control);

    sec_control->cfgdismpcpencrypt = RU_FIELD_GET(0, LIF, SEC_CONTROL, CFGDISMPCPENCRYPT, reg_sec_control);
    sec_control->cfgdisoamencrypt = RU_FIELD_GET(0, LIF, SEC_CONTROL, CFGDISOAMENCRYPT, reg_sec_control);
    sec_control->cfgsecenshortlen = RU_FIELD_GET(0, LIF, SEC_CONTROL, CFGSECENSHORTLEN, reg_sec_control);
    sec_control->cfgenaereplayprct = RU_FIELD_GET(0, LIF, SEC_CONTROL, CFGENAEREPLAYPRCT, reg_sec_control);
    sec_control->cfgenlegacyrcc = RU_FIELD_GET(0, LIF, SEC_CONTROL, CFGENLEGACYRCC, reg_sec_control);
    sec_control->enfakeupaes = RU_FIELD_GET(0, LIF, SEC_CONTROL, ENFAKEUPAES, reg_sec_control);
    sec_control->enfakednaes = RU_FIELD_GET(0, LIF, SEC_CONTROL, ENFAKEDNAES, reg_sec_control);
    sec_control->cfgfecipglen = RU_FIELD_GET(0, LIF, SEC_CONTROL, CFGFECIPGLEN, reg_sec_control);
    sec_control->disdndasaencrpt = RU_FIELD_GET(0, LIF, SEC_CONTROL, DISDNDASAENCRPT, reg_sec_control);
    sec_control->entriplechurn = RU_FIELD_GET(0, LIF, SEC_CONTROL, ENTRIPLECHURN, reg_sec_control);
    sec_control->enepnmixencrypt = RU_FIELD_GET(0, LIF, SEC_CONTROL, ENEPNMIXENCRYPT, reg_sec_control);
    sec_control->disupdasaencrpt = RU_FIELD_GET(0, LIF, SEC_CONTROL, DISUPDASAENCRPT, reg_sec_control);
    sec_control->secupencryptscheme = RU_FIELD_GET(0, LIF, SEC_CONTROL, SECUPENCRYPTSCHEME, reg_sec_control);
    sec_control->secdnencryptscheme = RU_FIELD_GET(0, LIF, SEC_CONTROL, SECDNENCRYPTSCHEME, reg_sec_control);
    sec_control->secuprstn_pre = RU_FIELD_GET(0, LIF, SEC_CONTROL, SECUPRSTN_PRE, reg_sec_control);
    sec_control->secdnrstn_pre = RU_FIELD_GET(0, LIF, SEC_CONTROL, SECDNRSTN_PRE, reg_sec_control);
    sec_control->secenup = RU_FIELD_GET(0, LIF, SEC_CONTROL, SECENUP, reg_sec_control);
    sec_control->secendn = RU_FIELD_GET(0, LIF, SEC_CONTROL, SECENDN, reg_sec_control);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_lif_macsec_set(uint16_t cfgmacsecethertype)
{
    uint32_t reg_macsec=0;

#ifdef VALIDATE_PARMS
#endif

    reg_macsec = RU_FIELD_SET(0, LIF, MACSEC, CFGMACSECETHERTYPE, reg_macsec, cfgmacsecethertype);

    RU_REG_WRITE(0, LIF, MACSEC, reg_macsec);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_lif_macsec_get(uint16_t *cfgmacsecethertype)
{
    uint32_t reg_macsec=0;

#ifdef VALIDATE_PARMS
    if(!cfgmacsecethertype)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, LIF, MACSEC, reg_macsec);

    *cfgmacsecethertype = RU_FIELD_GET(0, LIF, MACSEC, CFGMACSECETHERTYPE, reg_macsec);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_lif_int_status_set(const lif_int_status *int_status)
{
    uint32_t reg_int_status=0;

#ifdef VALIDATE_PARMS
    if(!int_status)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((int_status->int_sop_sfec_ipg_violation >= _1BITS_MAX_VAL_) ||
       (int_status->laseronmax >= _1BITS_MAX_VAL_) ||
       (int_status->laseroff >= _1BITS_MAX_VAL_) ||
       (int_status->secdnreplayprotctabort >= _1BITS_MAX_VAL_) ||
       (int_status->secuppktnumoverflow >= _1BITS_MAX_VAL_) ||
       (int_status->intlaseroffdurburst >= _1BITS_MAX_VAL_) ||
       (int_status->intrxberthreshexc >= _1BITS_MAX_VAL_) ||
       (int_status->intfecrxfecrecvstatus >= _1BITS_MAX_VAL_) ||
       (int_status->intfecrxcorerrfifofullstatus >= _1BITS_MAX_VAL_) ||
       (int_status->intfecrxcorerrfifounexpempty >= _1BITS_MAX_VAL_) ||
       (int_status->intfecbufpopemptypush >= _1BITS_MAX_VAL_) ||
       (int_status->intfecbufpopemptynopush >= _1BITS_MAX_VAL_) ||
       (int_status->intfecbufpushfull >= _1BITS_MAX_VAL_) ||
       (int_status->intuptimefullupdstat >= _1BITS_MAX_VAL_) ||
       (int_status->intfroutofalignstat >= _1BITS_MAX_VAL_) ||
       (int_status->intgrntstarttimelagstat >= _1BITS_MAX_VAL_) ||
       (int_status->intabortrxfrmstat >= _1BITS_MAX_VAL_) ||
       (int_status->intnorxclkstat >= _1BITS_MAX_VAL_) ||
       (int_status->intrxmaxlenerrstat >= _1BITS_MAX_VAL_) ||
       (int_status->intrxerraftalignstat >= _1BITS_MAX_VAL_) ||
       (int_status->intrxsynchacqstat >= _1BITS_MAX_VAL_) ||
       (int_status->intrxoutofsynchstat >= _1BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_int_status = RU_FIELD_SET(0, LIF, INT_STATUS, INT_SOP_SFEC_IPG_VIOLATION, reg_int_status, int_status->int_sop_sfec_ipg_violation);
    reg_int_status = RU_FIELD_SET(0, LIF, INT_STATUS, LASERONMAX, reg_int_status, int_status->laseronmax);
    reg_int_status = RU_FIELD_SET(0, LIF, INT_STATUS, LASEROFF, reg_int_status, int_status->laseroff);
    reg_int_status = RU_FIELD_SET(0, LIF, INT_STATUS, SECDNREPLAYPROTCTABORT, reg_int_status, int_status->secdnreplayprotctabort);
    reg_int_status = RU_FIELD_SET(0, LIF, INT_STATUS, SECUPPKTNUMOVERFLOW, reg_int_status, int_status->secuppktnumoverflow);
    reg_int_status = RU_FIELD_SET(0, LIF, INT_STATUS, INTLASEROFFDURBURST, reg_int_status, int_status->intlaseroffdurburst);
    reg_int_status = RU_FIELD_SET(0, LIF, INT_STATUS, INTRXBERTHRESHEXC, reg_int_status, int_status->intrxberthreshexc);
    reg_int_status = RU_FIELD_SET(0, LIF, INT_STATUS, INTFECRXFECRECVSTATUS, reg_int_status, int_status->intfecrxfecrecvstatus);
    reg_int_status = RU_FIELD_SET(0, LIF, INT_STATUS, INTFECRXCORERRFIFOFULLSTATUS, reg_int_status, int_status->intfecrxcorerrfifofullstatus);
    reg_int_status = RU_FIELD_SET(0, LIF, INT_STATUS, INTFECRXCORERRFIFOUNEXPEMPTY, reg_int_status, int_status->intfecrxcorerrfifounexpempty);
    reg_int_status = RU_FIELD_SET(0, LIF, INT_STATUS, INTFECBUFPOPEMPTYPUSH, reg_int_status, int_status->intfecbufpopemptypush);
    reg_int_status = RU_FIELD_SET(0, LIF, INT_STATUS, INTFECBUFPOPEMPTYNOPUSH, reg_int_status, int_status->intfecbufpopemptynopush);
    reg_int_status = RU_FIELD_SET(0, LIF, INT_STATUS, INTFECBUFPUSHFULL, reg_int_status, int_status->intfecbufpushfull);
    reg_int_status = RU_FIELD_SET(0, LIF, INT_STATUS, INTUPTIMEFULLUPDSTAT, reg_int_status, int_status->intuptimefullupdstat);
    reg_int_status = RU_FIELD_SET(0, LIF, INT_STATUS, INTFROUTOFALIGNSTAT, reg_int_status, int_status->intfroutofalignstat);
    reg_int_status = RU_FIELD_SET(0, LIF, INT_STATUS, INTGRNTSTARTTIMELAGSTAT, reg_int_status, int_status->intgrntstarttimelagstat);
    reg_int_status = RU_FIELD_SET(0, LIF, INT_STATUS, INTABORTRXFRMSTAT, reg_int_status, int_status->intabortrxfrmstat);
    reg_int_status = RU_FIELD_SET(0, LIF, INT_STATUS, INTNORXCLKSTAT, reg_int_status, int_status->intnorxclkstat);
    reg_int_status = RU_FIELD_SET(0, LIF, INT_STATUS, INTRXMAXLENERRSTAT, reg_int_status, int_status->intrxmaxlenerrstat);
    reg_int_status = RU_FIELD_SET(0, LIF, INT_STATUS, INTRXERRAFTALIGNSTAT, reg_int_status, int_status->intrxerraftalignstat);
    reg_int_status = RU_FIELD_SET(0, LIF, INT_STATUS, INTRXSYNCHACQSTAT, reg_int_status, int_status->intrxsynchacqstat);
    reg_int_status = RU_FIELD_SET(0, LIF, INT_STATUS, INTRXOUTOFSYNCHSTAT, reg_int_status, int_status->intrxoutofsynchstat);

    RU_REG_WRITE(0, LIF, INT_STATUS, reg_int_status);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_lif_int_status_get(lif_int_status *int_status)
{
    uint32_t reg_int_status=0;

#ifdef VALIDATE_PARMS
    if(!int_status)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, LIF, INT_STATUS, reg_int_status);

    int_status->int_sop_sfec_ipg_violation = RU_FIELD_GET(0, LIF, INT_STATUS, INT_SOP_SFEC_IPG_VIOLATION, reg_int_status);
    int_status->laseronmax = RU_FIELD_GET(0, LIF, INT_STATUS, LASERONMAX, reg_int_status);
    int_status->laseroff = RU_FIELD_GET(0, LIF, INT_STATUS, LASEROFF, reg_int_status);
    int_status->secdnreplayprotctabort = RU_FIELD_GET(0, LIF, INT_STATUS, SECDNREPLAYPROTCTABORT, reg_int_status);
    int_status->secuppktnumoverflow = RU_FIELD_GET(0, LIF, INT_STATUS, SECUPPKTNUMOVERFLOW, reg_int_status);
    int_status->intlaseroffdurburst = RU_FIELD_GET(0, LIF, INT_STATUS, INTLASEROFFDURBURST, reg_int_status);
    int_status->intrxberthreshexc = RU_FIELD_GET(0, LIF, INT_STATUS, INTRXBERTHRESHEXC, reg_int_status);
    int_status->intfecrxfecrecvstatus = RU_FIELD_GET(0, LIF, INT_STATUS, INTFECRXFECRECVSTATUS, reg_int_status);
    int_status->intfecrxcorerrfifofullstatus = RU_FIELD_GET(0, LIF, INT_STATUS, INTFECRXCORERRFIFOFULLSTATUS, reg_int_status);
    int_status->intfecrxcorerrfifounexpempty = RU_FIELD_GET(0, LIF, INT_STATUS, INTFECRXCORERRFIFOUNEXPEMPTY, reg_int_status);
    int_status->intfecbufpopemptypush = RU_FIELD_GET(0, LIF, INT_STATUS, INTFECBUFPOPEMPTYPUSH, reg_int_status);
    int_status->intfecbufpopemptynopush = RU_FIELD_GET(0, LIF, INT_STATUS, INTFECBUFPOPEMPTYNOPUSH, reg_int_status);
    int_status->intfecbufpushfull = RU_FIELD_GET(0, LIF, INT_STATUS, INTFECBUFPUSHFULL, reg_int_status);
    int_status->intuptimefullupdstat = RU_FIELD_GET(0, LIF, INT_STATUS, INTUPTIMEFULLUPDSTAT, reg_int_status);
    int_status->intfroutofalignstat = RU_FIELD_GET(0, LIF, INT_STATUS, INTFROUTOFALIGNSTAT, reg_int_status);
    int_status->intgrntstarttimelagstat = RU_FIELD_GET(0, LIF, INT_STATUS, INTGRNTSTARTTIMELAGSTAT, reg_int_status);
    int_status->intabortrxfrmstat = RU_FIELD_GET(0, LIF, INT_STATUS, INTABORTRXFRMSTAT, reg_int_status);
    int_status->intnorxclkstat = RU_FIELD_GET(0, LIF, INT_STATUS, INTNORXCLKSTAT, reg_int_status);
    int_status->intrxmaxlenerrstat = RU_FIELD_GET(0, LIF, INT_STATUS, INTRXMAXLENERRSTAT, reg_int_status);
    int_status->intrxerraftalignstat = RU_FIELD_GET(0, LIF, INT_STATUS, INTRXERRAFTALIGNSTAT, reg_int_status);
    int_status->intrxsynchacqstat = RU_FIELD_GET(0, LIF, INT_STATUS, INTRXSYNCHACQSTAT, reg_int_status);
    int_status->intrxoutofsynchstat = RU_FIELD_GET(0, LIF, INT_STATUS, INTRXOUTOFSYNCHSTAT, reg_int_status);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_lif_int_mask_set(const lif_int_mask *int_mask)
{
    uint32_t reg_int_mask=0;

#ifdef VALIDATE_PARMS
    if(!int_mask)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((int_mask->int_sop_sfec_ipg_violation_mask >= _1BITS_MAX_VAL_) ||
       (int_mask->laseronmaxmask >= _1BITS_MAX_VAL_) ||
       (int_mask->laseroffmask >= _1BITS_MAX_VAL_) ||
       (int_mask->secdnreplayprotctabortmsk >= _1BITS_MAX_VAL_) ||
       (int_mask->secuppktnumoverflowmsk >= _1BITS_MAX_VAL_) ||
       (int_mask->intlaseroffdurburstmask >= _1BITS_MAX_VAL_) ||
       (int_mask->intrxberthreshexcmask >= _1BITS_MAX_VAL_) ||
       (int_mask->intfecrxfecrecvmask >= _1BITS_MAX_VAL_) ||
       (int_mask->intfecrxcorerrfifofullmask >= _1BITS_MAX_VAL_) ||
       (int_mask->intfecrxcorerrfifounexpemptymask >= _1BITS_MAX_VAL_) ||
       (int_mask->intfecbufpopemptypushmask >= _1BITS_MAX_VAL_) ||
       (int_mask->intfecbufpopemptynopushmask >= _1BITS_MAX_VAL_) ||
       (int_mask->intfecbufpushfullmask >= _1BITS_MAX_VAL_) ||
       (int_mask->intuptimefullupdmask >= _1BITS_MAX_VAL_) ||
       (int_mask->intfroutofalignmask >= _1BITS_MAX_VAL_) ||
       (int_mask->intgrntstarttimelagmask >= _1BITS_MAX_VAL_) ||
       (int_mask->intabortrxfrmmask >= _1BITS_MAX_VAL_) ||
       (int_mask->intnorxclkmask >= _1BITS_MAX_VAL_) ||
       (int_mask->intrxmaxlenerrmask >= _1BITS_MAX_VAL_) ||
       (int_mask->intrxerraftalignmask >= _1BITS_MAX_VAL_) ||
       (int_mask->intrxsynchacqmask >= _1BITS_MAX_VAL_) ||
       (int_mask->intrxoutofsynchmask >= _1BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_int_mask = RU_FIELD_SET(0, LIF, INT_MASK, INT_SOP_SFEC_IPG_VIOLATION_MASK, reg_int_mask, int_mask->int_sop_sfec_ipg_violation_mask);
    reg_int_mask = RU_FIELD_SET(0, LIF, INT_MASK, LASERONMAXMASK, reg_int_mask, int_mask->laseronmaxmask);
    reg_int_mask = RU_FIELD_SET(0, LIF, INT_MASK, LASEROFFMASK, reg_int_mask, int_mask->laseroffmask);
    reg_int_mask = RU_FIELD_SET(0, LIF, INT_MASK, SECDNREPLAYPROTCTABORTMSK, reg_int_mask, int_mask->secdnreplayprotctabortmsk);
    reg_int_mask = RU_FIELD_SET(0, LIF, INT_MASK, SECUPPKTNUMOVERFLOWMSK, reg_int_mask, int_mask->secuppktnumoverflowmsk);
    reg_int_mask = RU_FIELD_SET(0, LIF, INT_MASK, INTLASEROFFDURBURSTMASK, reg_int_mask, int_mask->intlaseroffdurburstmask);
    reg_int_mask = RU_FIELD_SET(0, LIF, INT_MASK, INTRXBERTHRESHEXCMASK, reg_int_mask, int_mask->intrxberthreshexcmask);
    reg_int_mask = RU_FIELD_SET(0, LIF, INT_MASK, INTFECRXFECRECVMASK, reg_int_mask, int_mask->intfecrxfecrecvmask);
    reg_int_mask = RU_FIELD_SET(0, LIF, INT_MASK, INTFECRXCORERRFIFOFULLMASK, reg_int_mask, int_mask->intfecrxcorerrfifofullmask);
    reg_int_mask = RU_FIELD_SET(0, LIF, INT_MASK, INTFECRXCORERRFIFOUNEXPEMPTYMASK, reg_int_mask, int_mask->intfecrxcorerrfifounexpemptymask);
    reg_int_mask = RU_FIELD_SET(0, LIF, INT_MASK, INTFECBUFPOPEMPTYPUSHMASK, reg_int_mask, int_mask->intfecbufpopemptypushmask);
    reg_int_mask = RU_FIELD_SET(0, LIF, INT_MASK, INTFECBUFPOPEMPTYNOPUSHMASK, reg_int_mask, int_mask->intfecbufpopemptynopushmask);
    reg_int_mask = RU_FIELD_SET(0, LIF, INT_MASK, INTFECBUFPUSHFULLMASK, reg_int_mask, int_mask->intfecbufpushfullmask);
    reg_int_mask = RU_FIELD_SET(0, LIF, INT_MASK, INTUPTIMEFULLUPDMASK, reg_int_mask, int_mask->intuptimefullupdmask);
    reg_int_mask = RU_FIELD_SET(0, LIF, INT_MASK, INTFROUTOFALIGNMASK, reg_int_mask, int_mask->intfroutofalignmask);
    reg_int_mask = RU_FIELD_SET(0, LIF, INT_MASK, INTGRNTSTARTTIMELAGMASK, reg_int_mask, int_mask->intgrntstarttimelagmask);
    reg_int_mask = RU_FIELD_SET(0, LIF, INT_MASK, INTABORTRXFRMMASK, reg_int_mask, int_mask->intabortrxfrmmask);
    reg_int_mask = RU_FIELD_SET(0, LIF, INT_MASK, INTNORXCLKMASK, reg_int_mask, int_mask->intnorxclkmask);
    reg_int_mask = RU_FIELD_SET(0, LIF, INT_MASK, INTRXMAXLENERRMASK, reg_int_mask, int_mask->intrxmaxlenerrmask);
    reg_int_mask = RU_FIELD_SET(0, LIF, INT_MASK, INTRXERRAFTALIGNMASK, reg_int_mask, int_mask->intrxerraftalignmask);
    reg_int_mask = RU_FIELD_SET(0, LIF, INT_MASK, INTRXSYNCHACQMASK, reg_int_mask, int_mask->intrxsynchacqmask);
    reg_int_mask = RU_FIELD_SET(0, LIF, INT_MASK, INTRXOUTOFSYNCHMASK, reg_int_mask, int_mask->intrxoutofsynchmask);

    RU_REG_WRITE(0, LIF, INT_MASK, reg_int_mask);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_lif_int_mask_get(lif_int_mask *int_mask)
{
    uint32_t reg_int_mask=0;

#ifdef VALIDATE_PARMS
    if(!int_mask)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, LIF, INT_MASK, reg_int_mask);

    int_mask->int_sop_sfec_ipg_violation_mask = RU_FIELD_GET(0, LIF, INT_MASK, INT_SOP_SFEC_IPG_VIOLATION_MASK, reg_int_mask);
    int_mask->laseronmaxmask = RU_FIELD_GET(0, LIF, INT_MASK, LASERONMAXMASK, reg_int_mask);
    int_mask->laseroffmask = RU_FIELD_GET(0, LIF, INT_MASK, LASEROFFMASK, reg_int_mask);
    int_mask->secdnreplayprotctabortmsk = RU_FIELD_GET(0, LIF, INT_MASK, SECDNREPLAYPROTCTABORTMSK, reg_int_mask);
    int_mask->secuppktnumoverflowmsk = RU_FIELD_GET(0, LIF, INT_MASK, SECUPPKTNUMOVERFLOWMSK, reg_int_mask);
    int_mask->intlaseroffdurburstmask = RU_FIELD_GET(0, LIF, INT_MASK, INTLASEROFFDURBURSTMASK, reg_int_mask);
    int_mask->intrxberthreshexcmask = RU_FIELD_GET(0, LIF, INT_MASK, INTRXBERTHRESHEXCMASK, reg_int_mask);
    int_mask->intfecrxfecrecvmask = RU_FIELD_GET(0, LIF, INT_MASK, INTFECRXFECRECVMASK, reg_int_mask);
    int_mask->intfecrxcorerrfifofullmask = RU_FIELD_GET(0, LIF, INT_MASK, INTFECRXCORERRFIFOFULLMASK, reg_int_mask);
    int_mask->intfecrxcorerrfifounexpemptymask = RU_FIELD_GET(0, LIF, INT_MASK, INTFECRXCORERRFIFOUNEXPEMPTYMASK, reg_int_mask);
    int_mask->intfecbufpopemptypushmask = RU_FIELD_GET(0, LIF, INT_MASK, INTFECBUFPOPEMPTYPUSHMASK, reg_int_mask);
    int_mask->intfecbufpopemptynopushmask = RU_FIELD_GET(0, LIF, INT_MASK, INTFECBUFPOPEMPTYNOPUSHMASK, reg_int_mask);
    int_mask->intfecbufpushfullmask = RU_FIELD_GET(0, LIF, INT_MASK, INTFECBUFPUSHFULLMASK, reg_int_mask);
    int_mask->intuptimefullupdmask = RU_FIELD_GET(0, LIF, INT_MASK, INTUPTIMEFULLUPDMASK, reg_int_mask);
    int_mask->intfroutofalignmask = RU_FIELD_GET(0, LIF, INT_MASK, INTFROUTOFALIGNMASK, reg_int_mask);
    int_mask->intgrntstarttimelagmask = RU_FIELD_GET(0, LIF, INT_MASK, INTGRNTSTARTTIMELAGMASK, reg_int_mask);
    int_mask->intabortrxfrmmask = RU_FIELD_GET(0, LIF, INT_MASK, INTABORTRXFRMMASK, reg_int_mask);
    int_mask->intnorxclkmask = RU_FIELD_GET(0, LIF, INT_MASK, INTNORXCLKMASK, reg_int_mask);
    int_mask->intrxmaxlenerrmask = RU_FIELD_GET(0, LIF, INT_MASK, INTRXMAXLENERRMASK, reg_int_mask);
    int_mask->intrxerraftalignmask = RU_FIELD_GET(0, LIF, INT_MASK, INTRXERRAFTALIGNMASK, reg_int_mask);
    int_mask->intrxsynchacqmask = RU_FIELD_GET(0, LIF, INT_MASK, INTRXSYNCHACQMASK, reg_int_mask);
    int_mask->intrxoutofsynchmask = RU_FIELD_GET(0, LIF, INT_MASK, INTRXOUTOFSYNCHMASK, reg_int_mask);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_lif_data_port_command_set(const lif_data_port_command *data_port_command)
{
    uint32_t reg_data_port_command=0;

#ifdef VALIDATE_PARMS
    if(!data_port_command)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((data_port_command->data_port_busy >= _1BITS_MAX_VAL_) ||
       (data_port_command->data_port_error >= _1BITS_MAX_VAL_) ||
       (data_port_command->ram_select >= _6BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_data_port_command = RU_FIELD_SET(0, LIF, DATA_PORT_COMMAND, DATA_PORT_BUSY, reg_data_port_command, data_port_command->data_port_busy);
    reg_data_port_command = RU_FIELD_SET(0, LIF, DATA_PORT_COMMAND, DATA_PORT_ERROR, reg_data_port_command, data_port_command->data_port_error);
    reg_data_port_command = RU_FIELD_SET(0, LIF, DATA_PORT_COMMAND, RAM_SELECT, reg_data_port_command, data_port_command->ram_select);
    reg_data_port_command = RU_FIELD_SET(0, LIF, DATA_PORT_COMMAND, DATA_PORT_OP_CODE, reg_data_port_command, data_port_command->data_port_op_code);
    reg_data_port_command = RU_FIELD_SET(0, LIF, DATA_PORT_COMMAND, DATA_PORT_ADDR, reg_data_port_command, data_port_command->data_port_addr);

    RU_REG_WRITE(0, LIF, DATA_PORT_COMMAND, reg_data_port_command);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_lif_data_port_command_get(lif_data_port_command *data_port_command)
{
    uint32_t reg_data_port_command=0;

#ifdef VALIDATE_PARMS
    if(!data_port_command)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, LIF, DATA_PORT_COMMAND, reg_data_port_command);

    data_port_command->data_port_busy = RU_FIELD_GET(0, LIF, DATA_PORT_COMMAND, DATA_PORT_BUSY, reg_data_port_command);
    data_port_command->data_port_error = RU_FIELD_GET(0, LIF, DATA_PORT_COMMAND, DATA_PORT_ERROR, reg_data_port_command);
    data_port_command->ram_select = RU_FIELD_GET(0, LIF, DATA_PORT_COMMAND, RAM_SELECT, reg_data_port_command);
    data_port_command->data_port_op_code = RU_FIELD_GET(0, LIF, DATA_PORT_COMMAND, DATA_PORT_OP_CODE, reg_data_port_command);
    data_port_command->data_port_addr = RU_FIELD_GET(0, LIF, DATA_PORT_COMMAND, DATA_PORT_ADDR, reg_data_port_command);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_lif_data_port_data_set(uint8_t portidx, uint32_t pbiportdata)
{
    uint32_t reg_data_port_data=0;

#ifdef VALIDATE_PARMS
    if((portidx >= 8))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_data_port_data = RU_FIELD_SET(0, LIF, DATA_PORT_DATA, PBIPORTDATA, reg_data_port_data, pbiportdata);

    RU_REG_RAM_WRITE(0, portidx, LIF, DATA_PORT_DATA, reg_data_port_data);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_lif_data_port_data_get(uint8_t portidx, uint32_t *pbiportdata)
{
    uint32_t reg_data_port_data=0;

#ifdef VALIDATE_PARMS
    if(!pbiportdata)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((portidx >= 8))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(0, portidx, LIF, DATA_PORT_DATA, reg_data_port_data);

    *pbiportdata = RU_FIELD_GET(0, LIF, DATA_PORT_DATA, PBIPORTDATA, reg_data_port_data);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_lif_llid_0_set(uint32_t cfgllid0)
{
    uint32_t reg_llid_0=0;

#ifdef VALIDATE_PARMS
    if((cfgllid0 >= _17BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_llid_0 = RU_FIELD_SET(0, LIF, LLID_0, CFGLLID0, reg_llid_0, cfgllid0);

    RU_REG_WRITE(0, LIF, LLID_0, reg_llid_0);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_lif_llid_0_get(uint32_t *cfgllid0)
{
    uint32_t reg_llid_0=0;

#ifdef VALIDATE_PARMS
    if(!cfgllid0)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, LIF, LLID_0, reg_llid_0);

    *cfgllid0 = RU_FIELD_GET(0, LIF, LLID_0, CFGLLID0, reg_llid_0);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_lif_llid_1_set(uint32_t cfgllid1)
{
    uint32_t reg_llid_1=0;

#ifdef VALIDATE_PARMS
    if((cfgllid1 >= _17BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_llid_1 = RU_FIELD_SET(0, LIF, LLID_1, CFGLLID1, reg_llid_1, cfgllid1);

    RU_REG_WRITE(0, LIF, LLID_1, reg_llid_1);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_lif_llid_1_get(uint32_t *cfgllid1)
{
    uint32_t reg_llid_1=0;

#ifdef VALIDATE_PARMS
    if(!cfgllid1)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, LIF, LLID_1, reg_llid_1);

    *cfgllid1 = RU_FIELD_GET(0, LIF, LLID_1, CFGLLID1, reg_llid_1);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_lif_llid_2_set(uint32_t cfgllid2)
{
    uint32_t reg_llid_2=0;

#ifdef VALIDATE_PARMS
    if((cfgllid2 >= _17BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_llid_2 = RU_FIELD_SET(0, LIF, LLID_2, CFGLLID2, reg_llid_2, cfgllid2);

    RU_REG_WRITE(0, LIF, LLID_2, reg_llid_2);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_lif_llid_2_get(uint32_t *cfgllid2)
{
    uint32_t reg_llid_2=0;

#ifdef VALIDATE_PARMS
    if(!cfgllid2)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, LIF, LLID_2, reg_llid_2);

    *cfgllid2 = RU_FIELD_GET(0, LIF, LLID_2, CFGLLID2, reg_llid_2);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_lif_llid_3_set(uint32_t cfgllid3)
{
    uint32_t reg_llid_3=0;

#ifdef VALIDATE_PARMS
    if((cfgllid3 >= _17BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_llid_3 = RU_FIELD_SET(0, LIF, LLID_3, CFGLLID3, reg_llid_3, cfgllid3);

    RU_REG_WRITE(0, LIF, LLID_3, reg_llid_3);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_lif_llid_3_get(uint32_t *cfgllid3)
{
    uint32_t reg_llid_3=0;

#ifdef VALIDATE_PARMS
    if(!cfgllid3)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, LIF, LLID_3, reg_llid_3);

    *cfgllid3 = RU_FIELD_GET(0, LIF, LLID_3, CFGLLID3, reg_llid_3);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_lif_llid_4_set(uint32_t cfgllid4)
{
    uint32_t reg_llid_4=0;

#ifdef VALIDATE_PARMS
    if((cfgllid4 >= _17BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_llid_4 = RU_FIELD_SET(0, LIF, LLID_4, CFGLLID4, reg_llid_4, cfgllid4);

    RU_REG_WRITE(0, LIF, LLID_4, reg_llid_4);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_lif_llid_4_get(uint32_t *cfgllid4)
{
    uint32_t reg_llid_4=0;

#ifdef VALIDATE_PARMS
    if(!cfgllid4)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, LIF, LLID_4, reg_llid_4);

    *cfgllid4 = RU_FIELD_GET(0, LIF, LLID_4, CFGLLID4, reg_llid_4);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_lif_llid_5_set(uint32_t cfgllid5)
{
    uint32_t reg_llid_5=0;

#ifdef VALIDATE_PARMS
    if((cfgllid5 >= _17BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_llid_5 = RU_FIELD_SET(0, LIF, LLID_5, CFGLLID5, reg_llid_5, cfgllid5);

    RU_REG_WRITE(0, LIF, LLID_5, reg_llid_5);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_lif_llid_5_get(uint32_t *cfgllid5)
{
    uint32_t reg_llid_5=0;

#ifdef VALIDATE_PARMS
    if(!cfgllid5)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, LIF, LLID_5, reg_llid_5);

    *cfgllid5 = RU_FIELD_GET(0, LIF, LLID_5, CFGLLID5, reg_llid_5);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_lif_llid_6_set(uint32_t cfgllid6)
{
    uint32_t reg_llid_6=0;

#ifdef VALIDATE_PARMS
    if((cfgllid6 >= _17BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_llid_6 = RU_FIELD_SET(0, LIF, LLID_6, CFGLLID6, reg_llid_6, cfgllid6);

    RU_REG_WRITE(0, LIF, LLID_6, reg_llid_6);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_lif_llid_6_get(uint32_t *cfgllid6)
{
    uint32_t reg_llid_6=0;

#ifdef VALIDATE_PARMS
    if(!cfgllid6)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, LIF, LLID_6, reg_llid_6);

    *cfgllid6 = RU_FIELD_GET(0, LIF, LLID_6, CFGLLID6, reg_llid_6);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_lif_llid_7_set(uint32_t cfgllid7)
{
    uint32_t reg_llid_7=0;

#ifdef VALIDATE_PARMS
    if((cfgllid7 >= _17BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_llid_7 = RU_FIELD_SET(0, LIF, LLID_7, CFGLLID7, reg_llid_7, cfgllid7);

    RU_REG_WRITE(0, LIF, LLID_7, reg_llid_7);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_lif_llid_7_get(uint32_t *cfgllid7)
{
    uint32_t reg_llid_7=0;

#ifdef VALIDATE_PARMS
    if(!cfgllid7)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, LIF, LLID_7, reg_llid_7);

    *cfgllid7 = RU_FIELD_GET(0, LIF, LLID_7, CFGLLID7, reg_llid_7);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_lif_llid_16_set(uint32_t cfgllid16)
{
    uint32_t reg_llid_16=0;

#ifdef VALIDATE_PARMS
    if((cfgllid16 >= _17BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_llid_16 = RU_FIELD_SET(0, LIF, LLID_16, CFGLLID16, reg_llid_16, cfgllid16);

    RU_REG_WRITE(0, LIF, LLID_16, reg_llid_16);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_lif_llid_16_get(uint32_t *cfgllid16)
{
    uint32_t reg_llid_16=0;

#ifdef VALIDATE_PARMS
    if(!cfgllid16)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, LIF, LLID_16, reg_llid_16);

    *cfgllid16 = RU_FIELD_GET(0, LIF, LLID_16, CFGLLID16, reg_llid_16);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_lif_llid_17_set(uint32_t cfgllid17)
{
    uint32_t reg_llid_17=0;

#ifdef VALIDATE_PARMS
    if((cfgllid17 >= _17BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_llid_17 = RU_FIELD_SET(0, LIF, LLID_17, CFGLLID17, reg_llid_17, cfgllid17);

    RU_REG_WRITE(0, LIF, LLID_17, reg_llid_17);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_lif_llid_17_get(uint32_t *cfgllid17)
{
    uint32_t reg_llid_17=0;

#ifdef VALIDATE_PARMS
    if(!cfgllid17)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, LIF, LLID_17, reg_llid_17);

    *cfgllid17 = RU_FIELD_GET(0, LIF, LLID_17, CFGLLID17, reg_llid_17);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_lif_llid_18_set(uint32_t cfgllid18)
{
    uint32_t reg_llid_18=0;

#ifdef VALIDATE_PARMS
    if((cfgllid18 >= _17BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_llid_18 = RU_FIELD_SET(0, LIF, LLID_18, CFGLLID18, reg_llid_18, cfgllid18);

    RU_REG_WRITE(0, LIF, LLID_18, reg_llid_18);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_lif_llid_18_get(uint32_t *cfgllid18)
{
    uint32_t reg_llid_18=0;

#ifdef VALIDATE_PARMS
    if(!cfgllid18)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, LIF, LLID_18, reg_llid_18);

    *cfgllid18 = RU_FIELD_GET(0, LIF, LLID_18, CFGLLID18, reg_llid_18);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_lif_llid_19_set(uint32_t cfgllid19)
{
    uint32_t reg_llid_19=0;

#ifdef VALIDATE_PARMS
    if((cfgllid19 >= _17BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_llid_19 = RU_FIELD_SET(0, LIF, LLID_19, CFGLLID19, reg_llid_19, cfgllid19);

    RU_REG_WRITE(0, LIF, LLID_19, reg_llid_19);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_lif_llid_19_get(uint32_t *cfgllid19)
{
    uint32_t reg_llid_19=0;

#ifdef VALIDATE_PARMS
    if(!cfgllid19)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, LIF, LLID_19, reg_llid_19);

    *cfgllid19 = RU_FIELD_GET(0, LIF, LLID_19, CFGLLID19, reg_llid_19);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_lif_llid_20_set(uint32_t cfgllid20)
{
    uint32_t reg_llid_20=0;

#ifdef VALIDATE_PARMS
    if((cfgllid20 >= _17BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_llid_20 = RU_FIELD_SET(0, LIF, LLID_20, CFGLLID20, reg_llid_20, cfgllid20);

    RU_REG_WRITE(0, LIF, LLID_20, reg_llid_20);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_lif_llid_20_get(uint32_t *cfgllid20)
{
    uint32_t reg_llid_20=0;

#ifdef VALIDATE_PARMS
    if(!cfgllid20)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, LIF, LLID_20, reg_llid_20);

    *cfgllid20 = RU_FIELD_GET(0, LIF, LLID_20, CFGLLID20, reg_llid_20);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_lif_llid_21_set(uint32_t cfgllid21)
{
    uint32_t reg_llid_21=0;

#ifdef VALIDATE_PARMS
    if((cfgllid21 >= _17BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_llid_21 = RU_FIELD_SET(0, LIF, LLID_21, CFGLLID21, reg_llid_21, cfgllid21);

    RU_REG_WRITE(0, LIF, LLID_21, reg_llid_21);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_lif_llid_21_get(uint32_t *cfgllid21)
{
    uint32_t reg_llid_21=0;

#ifdef VALIDATE_PARMS
    if(!cfgllid21)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, LIF, LLID_21, reg_llid_21);

    *cfgllid21 = RU_FIELD_GET(0, LIF, LLID_21, CFGLLID21, reg_llid_21);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_lif_llid_22_set(uint32_t cfgllid22)
{
    uint32_t reg_llid_22=0;

#ifdef VALIDATE_PARMS
    if((cfgllid22 >= _17BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_llid_22 = RU_FIELD_SET(0, LIF, LLID_22, CFGLLID22, reg_llid_22, cfgllid22);

    RU_REG_WRITE(0, LIF, LLID_22, reg_llid_22);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_lif_llid_22_get(uint32_t *cfgllid22)
{
    uint32_t reg_llid_22=0;

#ifdef VALIDATE_PARMS
    if(!cfgllid22)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, LIF, LLID_22, reg_llid_22);

    *cfgllid22 = RU_FIELD_GET(0, LIF, LLID_22, CFGLLID22, reg_llid_22);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_lif_llid_23_set(uint32_t cfgllid23)
{
    uint32_t reg_llid_23=0;

#ifdef VALIDATE_PARMS
    if((cfgllid23 >= _17BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_llid_23 = RU_FIELD_SET(0, LIF, LLID_23, CFGLLID23, reg_llid_23, cfgllid23);

    RU_REG_WRITE(0, LIF, LLID_23, reg_llid_23);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_lif_llid_23_get(uint32_t *cfgllid23)
{
    uint32_t reg_llid_23=0;

#ifdef VALIDATE_PARMS
    if(!cfgllid23)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, LIF, LLID_23, reg_llid_23);

    *cfgllid23 = RU_FIELD_GET(0, LIF, LLID_23, CFGLLID23, reg_llid_23);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_lif_time_ref_cnt_set(uint8_t cffullupdatevalue, uint8_t cfmaxnegvalue, uint8_t cfmaxposvalue)
{
    uint32_t reg_time_ref_cnt=0;

#ifdef VALIDATE_PARMS
#endif

    reg_time_ref_cnt = RU_FIELD_SET(0, LIF, TIME_REF_CNT, CFFULLUPDATEVALUE, reg_time_ref_cnt, cffullupdatevalue);
    reg_time_ref_cnt = RU_FIELD_SET(0, LIF, TIME_REF_CNT, CFMAXNEGVALUE, reg_time_ref_cnt, cfmaxnegvalue);
    reg_time_ref_cnt = RU_FIELD_SET(0, LIF, TIME_REF_CNT, CFMAXPOSVALUE, reg_time_ref_cnt, cfmaxposvalue);

    RU_REG_WRITE(0, LIF, TIME_REF_CNT, reg_time_ref_cnt);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_lif_time_ref_cnt_get(uint8_t *cffullupdatevalue, uint8_t *cfmaxnegvalue, uint8_t *cfmaxposvalue)
{
    uint32_t reg_time_ref_cnt=0;

#ifdef VALIDATE_PARMS
    if(!cffullupdatevalue || !cfmaxnegvalue || !cfmaxposvalue)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, LIF, TIME_REF_CNT, reg_time_ref_cnt);

    *cffullupdatevalue = RU_FIELD_GET(0, LIF, TIME_REF_CNT, CFFULLUPDATEVALUE, reg_time_ref_cnt);
    *cfmaxnegvalue = RU_FIELD_GET(0, LIF, TIME_REF_CNT, CFMAXNEGVALUE, reg_time_ref_cnt);
    *cfmaxposvalue = RU_FIELD_GET(0, LIF, TIME_REF_CNT, CFMAXPOSVALUE, reg_time_ref_cnt);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_lif_timestamp_upd_per_set(uint16_t cftimestampupdper)
{
    uint32_t reg_timestamp_upd_per=0;

#ifdef VALIDATE_PARMS
#endif

    reg_timestamp_upd_per = RU_FIELD_SET(0, LIF, TIMESTAMP_UPD_PER, CFTIMESTAMPUPDPER, reg_timestamp_upd_per, cftimestampupdper);

    RU_REG_WRITE(0, LIF, TIMESTAMP_UPD_PER, reg_timestamp_upd_per);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_lif_timestamp_upd_per_get(uint16_t *cftimestampupdper)
{
    uint32_t reg_timestamp_upd_per=0;

#ifdef VALIDATE_PARMS
    if(!cftimestampupdper)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, LIF, TIMESTAMP_UPD_PER, reg_timestamp_upd_per);

    *cftimestampupdper = RU_FIELD_GET(0, LIF, TIMESTAMP_UPD_PER, CFTIMESTAMPUPDPER, reg_timestamp_upd_per);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_lif_tp_time_set(uint32_t cftransporttime)
{
    uint32_t reg_tp_time=0;

#ifdef VALIDATE_PARMS
#endif

    reg_tp_time = RU_FIELD_SET(0, LIF, TP_TIME, CFTRANSPORTTIME, reg_tp_time, cftransporttime);

    RU_REG_WRITE(0, LIF, TP_TIME, reg_tp_time);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_lif_tp_time_get(uint32_t *cftransporttime)
{
    uint32_t reg_tp_time=0;

#ifdef VALIDATE_PARMS
    if(!cftransporttime)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, LIF, TP_TIME, reg_tp_time);

    *cftransporttime = RU_FIELD_GET(0, LIF, TP_TIME, CFTRANSPORTTIME, reg_tp_time);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_lif_mpcp_time_get(uint32_t *ltmpcptime)
{
    uint32_t reg_mpcp_time=0;

#ifdef VALIDATE_PARMS
    if(!ltmpcptime)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, LIF, MPCP_TIME, reg_mpcp_time);

    *ltmpcptime = RU_FIELD_GET(0, LIF, MPCP_TIME, LTMPCPTIME, reg_mpcp_time);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_lif_maxlen_ctr_set(uint16_t cfrxmaxframelength)
{
    uint32_t reg_maxlen_ctr=0;

#ifdef VALIDATE_PARMS
    if((cfrxmaxframelength >= _14BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_maxlen_ctr = RU_FIELD_SET(0, LIF, MAXLEN_CTR, CFRXMAXFRAMELENGTH, reg_maxlen_ctr, cfrxmaxframelength);

    RU_REG_WRITE(0, LIF, MAXLEN_CTR, reg_maxlen_ctr);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_lif_maxlen_ctr_get(uint16_t *cfrxmaxframelength)
{
    uint32_t reg_maxlen_ctr=0;

#ifdef VALIDATE_PARMS
    if(!cfrxmaxframelength)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, LIF, MAXLEN_CTR, reg_maxlen_ctr);

    *cfrxmaxframelength = RU_FIELD_GET(0, LIF, MAXLEN_CTR, CFRXMAXFRAMELENGTH, reg_maxlen_ctr);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_lif_laser_on_delta_set(uint16_t cftxlaserondelta)
{
    uint32_t reg_laser_on_delta=0;

#ifdef VALIDATE_PARMS
    if((cftxlaserondelta >= _13BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_laser_on_delta = RU_FIELD_SET(0, LIF, LASER_ON_DELTA, CFTXLASERONDELTA, reg_laser_on_delta, cftxlaserondelta);

    RU_REG_WRITE(0, LIF, LASER_ON_DELTA, reg_laser_on_delta);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_lif_laser_on_delta_get(uint16_t *cftxlaserondelta)
{
    uint32_t reg_laser_on_delta=0;

#ifdef VALIDATE_PARMS
    if(!cftxlaserondelta)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, LIF, LASER_ON_DELTA, reg_laser_on_delta);

    *cftxlaserondelta = RU_FIELD_GET(0, LIF, LASER_ON_DELTA, CFTXLASERONDELTA, reg_laser_on_delta);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_lif_laser_off_idle_set(uint16_t cftxinitidle, uint8_t cftxlaseroffdelta)
{
    uint32_t reg_laser_off_idle=0;

#ifdef VALIDATE_PARMS
#endif

    reg_laser_off_idle = RU_FIELD_SET(0, LIF, LASER_OFF_IDLE, CFTXINITIDLE, reg_laser_off_idle, cftxinitidle);
    reg_laser_off_idle = RU_FIELD_SET(0, LIF, LASER_OFF_IDLE, CFTXLASEROFFDELTA, reg_laser_off_idle, cftxlaseroffdelta);

    RU_REG_WRITE(0, LIF, LASER_OFF_IDLE, reg_laser_off_idle);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_lif_laser_off_idle_get(uint16_t *cftxinitidle, uint8_t *cftxlaseroffdelta)
{
    uint32_t reg_laser_off_idle=0;

#ifdef VALIDATE_PARMS
    if(!cftxinitidle || !cftxlaseroffdelta)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, LIF, LASER_OFF_IDLE, reg_laser_off_idle);

    *cftxinitidle = RU_FIELD_GET(0, LIF, LASER_OFF_IDLE, CFTXINITIDLE, reg_laser_off_idle);
    *cftxlaseroffdelta = RU_FIELD_GET(0, LIF, LASER_OFF_IDLE, CFTXLASEROFFDELTA, reg_laser_off_idle);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_lif_fec_init_idle_set(uint16_t cftxfecinitidle)
{
    uint32_t reg_fec_init_idle=0;

#ifdef VALIDATE_PARMS
#endif

    reg_fec_init_idle = RU_FIELD_SET(0, LIF, FEC_INIT_IDLE, CFTXFECINITIDLE, reg_fec_init_idle, cftxfecinitidle);

    RU_REG_WRITE(0, LIF, FEC_INIT_IDLE, reg_fec_init_idle);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_lif_fec_init_idle_get(uint16_t *cftxfecinitidle)
{
    uint32_t reg_fec_init_idle=0;

#ifdef VALIDATE_PARMS
    if(!cftxfecinitidle)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, LIF, FEC_INIT_IDLE, reg_fec_init_idle);

    *cftxfecinitidle = RU_FIELD_GET(0, LIF, FEC_INIT_IDLE, CFTXFECINITIDLE, reg_fec_init_idle);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_lif_fec_err_allow_set(uint8_t cfrxtfecbiterrallow, uint8_t cfrxsfecbiterrallow)
{
    uint32_t reg_fec_err_allow=0;

#ifdef VALIDATE_PARMS
    if((cfrxtfecbiterrallow >= _4BITS_MAX_VAL_) ||
       (cfrxsfecbiterrallow >= _4BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_fec_err_allow = RU_FIELD_SET(0, LIF, FEC_ERR_ALLOW, CFRXTFECBITERRALLOW, reg_fec_err_allow, cfrxtfecbiterrallow);
    reg_fec_err_allow = RU_FIELD_SET(0, LIF, FEC_ERR_ALLOW, CFRXSFECBITERRALLOW, reg_fec_err_allow, cfrxsfecbiterrallow);

    RU_REG_WRITE(0, LIF, FEC_ERR_ALLOW, reg_fec_err_allow);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_lif_fec_err_allow_get(uint8_t *cfrxtfecbiterrallow, uint8_t *cfrxsfecbiterrallow)
{
    uint32_t reg_fec_err_allow=0;

#ifdef VALIDATE_PARMS
    if(!cfrxtfecbiterrallow || !cfrxsfecbiterrallow)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, LIF, FEC_ERR_ALLOW, reg_fec_err_allow);

    *cfrxtfecbiterrallow = RU_FIELD_GET(0, LIF, FEC_ERR_ALLOW, CFRXTFECBITERRALLOW, reg_fec_err_allow);
    *cfrxsfecbiterrallow = RU_FIELD_GET(0, LIF, FEC_ERR_ALLOW, CFRXSFECBITERRALLOW, reg_fec_err_allow);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_lif_sec_key_sel_get(uint8_t link_idx, bdmf_boolean *data)
{
    uint32_t reg_sec_key_sel=0;

#ifdef VALIDATE_PARMS
    if(!data)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, LIF, SEC_KEY_SEL, reg_sec_key_sel);

    *data = FIELD_GET(reg_sec_key_sel, (link_idx % 32) *1, 0x1);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_lif_dn_encrypt_stat_set(uint8_t link_idx, bdmf_boolean enEncrypt)
{
    uint32_t reg_dn_encrypt_stat=0;

#ifdef VALIDATE_PARMS
#endif

    RU_REG_READ(0, LIF, DN_ENCRYPT_STAT, reg_dn_encrypt_stat);

    FIELD_SET(reg_dn_encrypt_stat, (link_idx % 32) *1, 0x1, enEncrypt);

    RU_REG_WRITE(0, LIF, DN_ENCRYPT_STAT, reg_dn_encrypt_stat);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_lif_dn_encrypt_stat_get(uint8_t link_idx, bdmf_boolean *enEncrypt)
{
    uint32_t reg_dn_encrypt_stat=0;

#ifdef VALIDATE_PARMS
    if(!enEncrypt)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, LIF, DN_ENCRYPT_STAT, reg_dn_encrypt_stat);

    *enEncrypt = FIELD_GET(reg_dn_encrypt_stat, (link_idx % 32) *1, 0x1);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_lif_sec_up_key_stat_get(uint8_t link_idx, bdmf_boolean *keyUpSel)
{
    uint32_t reg_sec_up_key_stat=0;

#ifdef VALIDATE_PARMS
    if(!keyUpSel)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, LIF, SEC_UP_KEY_STAT, reg_sec_up_key_stat);

    *keyUpSel = FIELD_GET(reg_sec_up_key_stat, (link_idx % 32) *1, 0x1);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_lif_sec_up_encrypt_stat_get(uint8_t link_idx, bdmf_boolean *enUpEncrypt)
{
    uint32_t reg_sec_up_encrypt_stat=0;

#ifdef VALIDATE_PARMS
    if(!enUpEncrypt)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, LIF, SEC_UP_ENCRYPT_STAT, reg_sec_up_encrypt_stat);

    *enUpEncrypt = FIELD_GET(reg_sec_up_encrypt_stat, (link_idx % 32) *1, 0x1);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_lif_sec_up_mpcp_offset_set(uint32_t secupmpcpoffset)
{
    uint32_t reg_sec_up_mpcp_offset=0;

#ifdef VALIDATE_PARMS
#endif

    reg_sec_up_mpcp_offset = RU_FIELD_SET(0, LIF, SEC_UP_MPCP_OFFSET, SECUPMPCPOFFSET, reg_sec_up_mpcp_offset, secupmpcpoffset);

    RU_REG_WRITE(0, LIF, SEC_UP_MPCP_OFFSET, reg_sec_up_mpcp_offset);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_lif_sec_up_mpcp_offset_get(uint32_t *secupmpcpoffset)
{
    uint32_t reg_sec_up_mpcp_offset=0;

#ifdef VALIDATE_PARMS
    if(!secupmpcpoffset)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, LIF, SEC_UP_MPCP_OFFSET, reg_sec_up_mpcp_offset);

    *secupmpcpoffset = RU_FIELD_GET(0, LIF, SEC_UP_MPCP_OFFSET, SECUPMPCPOFFSET, reg_sec_up_mpcp_offset);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_lif_fec_per_llid_set(uint8_t link_idx, bdmf_boolean cfFecTxFecEnLlid)
{
    uint32_t reg_fec_per_llid=0;

#ifdef VALIDATE_PARMS
#endif

    RU_REG_READ(0, LIF, FEC_PER_LLID, reg_fec_per_llid);

    FIELD_SET(reg_fec_per_llid, (link_idx % 32) *1, 0x1, cfFecTxFecEnLlid);

    RU_REG_WRITE(0, LIF, FEC_PER_LLID, reg_fec_per_llid);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_lif_fec_per_llid_get(uint8_t link_idx, bdmf_boolean *cfFecTxFecEnLlid)
{
    uint32_t reg_fec_per_llid=0;

#ifdef VALIDATE_PARMS
    if(!cfFecTxFecEnLlid)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, LIF, FEC_PER_LLID, reg_fec_per_llid);

    *cfFecTxFecEnLlid = FIELD_GET(reg_fec_per_llid, (link_idx % 32) *1, 0x1);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_lif_rx_line_code_err_cnt_set(uint32_t rxlinecodeerrcnt)
{
    uint32_t reg_rx_line_code_err_cnt=0;

#ifdef VALIDATE_PARMS
#endif

    reg_rx_line_code_err_cnt = RU_FIELD_SET(0, LIF, RX_LINE_CODE_ERR_CNT, RXLINECODEERRCNT, reg_rx_line_code_err_cnt, rxlinecodeerrcnt);

    RU_REG_WRITE(0, LIF, RX_LINE_CODE_ERR_CNT, reg_rx_line_code_err_cnt);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_lif_rx_line_code_err_cnt_get(uint32_t *rxlinecodeerrcnt)
{
    uint32_t reg_rx_line_code_err_cnt=0;

#ifdef VALIDATE_PARMS
    if(!rxlinecodeerrcnt)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, LIF, RX_LINE_CODE_ERR_CNT, reg_rx_line_code_err_cnt);

    *rxlinecodeerrcnt = RU_FIELD_GET(0, LIF, RX_LINE_CODE_ERR_CNT, RXLINECODEERRCNT, reg_rx_line_code_err_cnt);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_lif_rx_agg_mpcp_frm_set(uint32_t rxaggmpcpcnt)
{
    uint32_t reg_rx_agg_mpcp_frm=0;

#ifdef VALIDATE_PARMS
#endif

    reg_rx_agg_mpcp_frm = RU_FIELD_SET(0, LIF, RX_AGG_MPCP_FRM, RXAGGMPCPCNT, reg_rx_agg_mpcp_frm, rxaggmpcpcnt);

    RU_REG_WRITE(0, LIF, RX_AGG_MPCP_FRM, reg_rx_agg_mpcp_frm);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_lif_rx_agg_mpcp_frm_get(uint32_t *rxaggmpcpcnt)
{
    uint32_t reg_rx_agg_mpcp_frm=0;

#ifdef VALIDATE_PARMS
    if(!rxaggmpcpcnt)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, LIF, RX_AGG_MPCP_FRM, reg_rx_agg_mpcp_frm);

    *rxaggmpcpcnt = RU_FIELD_GET(0, LIF, RX_AGG_MPCP_FRM, RXAGGMPCPCNT, reg_rx_agg_mpcp_frm);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_lif_rx_agg_good_frm_set(uint32_t rxagggoodcnt)
{
    uint32_t reg_rx_agg_good_frm=0;

#ifdef VALIDATE_PARMS
#endif

    reg_rx_agg_good_frm = RU_FIELD_SET(0, LIF, RX_AGG_GOOD_FRM, RXAGGGOODCNT, reg_rx_agg_good_frm, rxagggoodcnt);

    RU_REG_WRITE(0, LIF, RX_AGG_GOOD_FRM, reg_rx_agg_good_frm);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_lif_rx_agg_good_frm_get(uint32_t *rxagggoodcnt)
{
    uint32_t reg_rx_agg_good_frm=0;

#ifdef VALIDATE_PARMS
    if(!rxagggoodcnt)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, LIF, RX_AGG_GOOD_FRM, reg_rx_agg_good_frm);

    *rxagggoodcnt = RU_FIELD_GET(0, LIF, RX_AGG_GOOD_FRM, RXAGGGOODCNT, reg_rx_agg_good_frm);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_lif_rx_agg_good_byte_set(uint32_t rxagggoodbytescnt)
{
    uint32_t reg_rx_agg_good_byte=0;

#ifdef VALIDATE_PARMS
#endif

    reg_rx_agg_good_byte = RU_FIELD_SET(0, LIF, RX_AGG_GOOD_BYTE, RXAGGGOODBYTESCNT, reg_rx_agg_good_byte, rxagggoodbytescnt);

    RU_REG_WRITE(0, LIF, RX_AGG_GOOD_BYTE, reg_rx_agg_good_byte);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_lif_rx_agg_good_byte_get(uint32_t *rxagggoodbytescnt)
{
    uint32_t reg_rx_agg_good_byte=0;

#ifdef VALIDATE_PARMS
    if(!rxagggoodbytescnt)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, LIF, RX_AGG_GOOD_BYTE, reg_rx_agg_good_byte);

    *rxagggoodbytescnt = RU_FIELD_GET(0, LIF, RX_AGG_GOOD_BYTE, RXAGGGOODBYTESCNT, reg_rx_agg_good_byte);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_lif_rx_agg_undersz_frm_set(uint32_t rxaggunderszcnt)
{
    uint32_t reg_rx_agg_undersz_frm=0;

#ifdef VALIDATE_PARMS
#endif

    reg_rx_agg_undersz_frm = RU_FIELD_SET(0, LIF, RX_AGG_UNDERSZ_FRM, RXAGGUNDERSZCNT, reg_rx_agg_undersz_frm, rxaggunderszcnt);

    RU_REG_WRITE(0, LIF, RX_AGG_UNDERSZ_FRM, reg_rx_agg_undersz_frm);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_lif_rx_agg_undersz_frm_get(uint32_t *rxaggunderszcnt)
{
    uint32_t reg_rx_agg_undersz_frm=0;

#ifdef VALIDATE_PARMS
    if(!rxaggunderszcnt)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, LIF, RX_AGG_UNDERSZ_FRM, reg_rx_agg_undersz_frm);

    *rxaggunderszcnt = RU_FIELD_GET(0, LIF, RX_AGG_UNDERSZ_FRM, RXAGGUNDERSZCNT, reg_rx_agg_undersz_frm);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_lif_rx_agg_oversz_frm_set(uint32_t rxaggoverszcnt)
{
    uint32_t reg_rx_agg_oversz_frm=0;

#ifdef VALIDATE_PARMS
#endif

    reg_rx_agg_oversz_frm = RU_FIELD_SET(0, LIF, RX_AGG_OVERSZ_FRM, RXAGGOVERSZCNT, reg_rx_agg_oversz_frm, rxaggoverszcnt);

    RU_REG_WRITE(0, LIF, RX_AGG_OVERSZ_FRM, reg_rx_agg_oversz_frm);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_lif_rx_agg_oversz_frm_get(uint32_t *rxaggoverszcnt)
{
    uint32_t reg_rx_agg_oversz_frm=0;

#ifdef VALIDATE_PARMS
    if(!rxaggoverszcnt)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, LIF, RX_AGG_OVERSZ_FRM, reg_rx_agg_oversz_frm);

    *rxaggoverszcnt = RU_FIELD_GET(0, LIF, RX_AGG_OVERSZ_FRM, RXAGGOVERSZCNT, reg_rx_agg_oversz_frm);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_lif_rx_agg_crc8_frm_set(uint32_t rxaggcrc8errcnt)
{
    uint32_t reg_rx_agg_crc8_frm=0;

#ifdef VALIDATE_PARMS
#endif

    reg_rx_agg_crc8_frm = RU_FIELD_SET(0, LIF, RX_AGG_CRC8_FRM, RXAGGCRC8ERRCNT, reg_rx_agg_crc8_frm, rxaggcrc8errcnt);

    RU_REG_WRITE(0, LIF, RX_AGG_CRC8_FRM, reg_rx_agg_crc8_frm);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_lif_rx_agg_crc8_frm_get(uint32_t *rxaggcrc8errcnt)
{
    uint32_t reg_rx_agg_crc8_frm=0;

#ifdef VALIDATE_PARMS
    if(!rxaggcrc8errcnt)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, LIF, RX_AGG_CRC8_FRM, reg_rx_agg_crc8_frm);

    *rxaggcrc8errcnt = RU_FIELD_GET(0, LIF, RX_AGG_CRC8_FRM, RXAGGCRC8ERRCNT, reg_rx_agg_crc8_frm);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_lif_rx_agg_fec_frm_set(uint32_t rxaggfec)
{
    uint32_t reg_rx_agg_fec_frm=0;

#ifdef VALIDATE_PARMS
#endif

    reg_rx_agg_fec_frm = RU_FIELD_SET(0, LIF, RX_AGG_FEC_FRM, RXAGGFEC, reg_rx_agg_fec_frm, rxaggfec);

    RU_REG_WRITE(0, LIF, RX_AGG_FEC_FRM, reg_rx_agg_fec_frm);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_lif_rx_agg_fec_frm_get(uint32_t *rxaggfec)
{
    uint32_t reg_rx_agg_fec_frm=0;

#ifdef VALIDATE_PARMS
    if(!rxaggfec)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, LIF, RX_AGG_FEC_FRM, reg_rx_agg_fec_frm);

    *rxaggfec = RU_FIELD_GET(0, LIF, RX_AGG_FEC_FRM, RXAGGFEC, reg_rx_agg_fec_frm);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_lif_rx_agg_fec_byte_set(uint32_t rxaggfecbytes)
{
    uint32_t reg_rx_agg_fec_byte=0;

#ifdef VALIDATE_PARMS
#endif

    reg_rx_agg_fec_byte = RU_FIELD_SET(0, LIF, RX_AGG_FEC_BYTE, RXAGGFECBYTES, reg_rx_agg_fec_byte, rxaggfecbytes);

    RU_REG_WRITE(0, LIF, RX_AGG_FEC_BYTE, reg_rx_agg_fec_byte);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_lif_rx_agg_fec_byte_get(uint32_t *rxaggfecbytes)
{
    uint32_t reg_rx_agg_fec_byte=0;

#ifdef VALIDATE_PARMS
    if(!rxaggfecbytes)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, LIF, RX_AGG_FEC_BYTE, reg_rx_agg_fec_byte);

    *rxaggfecbytes = RU_FIELD_GET(0, LIF, RX_AGG_FEC_BYTE, RXAGGFECBYTES, reg_rx_agg_fec_byte);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_lif_rx_agg_fec_exc_err_frm_set(uint32_t rxaggfecexceederrs)
{
    uint32_t reg_rx_agg_fec_exc_err_frm=0;

#ifdef VALIDATE_PARMS
#endif

    reg_rx_agg_fec_exc_err_frm = RU_FIELD_SET(0, LIF, RX_AGG_FEC_EXC_ERR_FRM, RXAGGFECEXCEEDERRS, reg_rx_agg_fec_exc_err_frm, rxaggfecexceederrs);

    RU_REG_WRITE(0, LIF, RX_AGG_FEC_EXC_ERR_FRM, reg_rx_agg_fec_exc_err_frm);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_lif_rx_agg_fec_exc_err_frm_get(uint32_t *rxaggfecexceederrs)
{
    uint32_t reg_rx_agg_fec_exc_err_frm=0;

#ifdef VALIDATE_PARMS
    if(!rxaggfecexceederrs)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, LIF, RX_AGG_FEC_EXC_ERR_FRM, reg_rx_agg_fec_exc_err_frm);

    *rxaggfecexceederrs = RU_FIELD_GET(0, LIF, RX_AGG_FEC_EXC_ERR_FRM, RXAGGFECEXCEEDERRS, reg_rx_agg_fec_exc_err_frm);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_lif_rx_agg_nonfec_good_frm_set(uint32_t rxaggnonfecgood)
{
    uint32_t reg_rx_agg_nonfec_good_frm=0;

#ifdef VALIDATE_PARMS
#endif

    reg_rx_agg_nonfec_good_frm = RU_FIELD_SET(0, LIF, RX_AGG_NONFEC_GOOD_FRM, RXAGGNONFECGOOD, reg_rx_agg_nonfec_good_frm, rxaggnonfecgood);

    RU_REG_WRITE(0, LIF, RX_AGG_NONFEC_GOOD_FRM, reg_rx_agg_nonfec_good_frm);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_lif_rx_agg_nonfec_good_frm_get(uint32_t *rxaggnonfecgood)
{
    uint32_t reg_rx_agg_nonfec_good_frm=0;

#ifdef VALIDATE_PARMS
    if(!rxaggnonfecgood)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, LIF, RX_AGG_NONFEC_GOOD_FRM, reg_rx_agg_nonfec_good_frm);

    *rxaggnonfecgood = RU_FIELD_GET(0, LIF, RX_AGG_NONFEC_GOOD_FRM, RXAGGNONFECGOOD, reg_rx_agg_nonfec_good_frm);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_lif_rx_agg_nonfec_good_byte_set(uint32_t rxaggnonfecgoodbytes)
{
    uint32_t reg_rx_agg_nonfec_good_byte=0;

#ifdef VALIDATE_PARMS
#endif

    reg_rx_agg_nonfec_good_byte = RU_FIELD_SET(0, LIF, RX_AGG_NONFEC_GOOD_BYTE, RXAGGNONFECGOODBYTES, reg_rx_agg_nonfec_good_byte, rxaggnonfecgoodbytes);

    RU_REG_WRITE(0, LIF, RX_AGG_NONFEC_GOOD_BYTE, reg_rx_agg_nonfec_good_byte);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_lif_rx_agg_nonfec_good_byte_get(uint32_t *rxaggnonfecgoodbytes)
{
    uint32_t reg_rx_agg_nonfec_good_byte=0;

#ifdef VALIDATE_PARMS
    if(!rxaggnonfecgoodbytes)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, LIF, RX_AGG_NONFEC_GOOD_BYTE, reg_rx_agg_nonfec_good_byte);

    *rxaggnonfecgoodbytes = RU_FIELD_GET(0, LIF, RX_AGG_NONFEC_GOOD_BYTE, RXAGGNONFECGOODBYTES, reg_rx_agg_nonfec_good_byte);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_lif_rx_agg_err_bytes_set(uint32_t rxaggerrbytes)
{
    uint32_t reg_rx_agg_err_bytes=0;

#ifdef VALIDATE_PARMS
#endif

    reg_rx_agg_err_bytes = RU_FIELD_SET(0, LIF, RX_AGG_ERR_BYTES, RXAGGERRBYTES, reg_rx_agg_err_bytes, rxaggerrbytes);

    RU_REG_WRITE(0, LIF, RX_AGG_ERR_BYTES, reg_rx_agg_err_bytes);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_lif_rx_agg_err_bytes_get(uint32_t *rxaggerrbytes)
{
    uint32_t reg_rx_agg_err_bytes=0;

#ifdef VALIDATE_PARMS
    if(!rxaggerrbytes)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, LIF, RX_AGG_ERR_BYTES, reg_rx_agg_err_bytes);

    *rxaggerrbytes = RU_FIELD_GET(0, LIF, RX_AGG_ERR_BYTES, RXAGGERRBYTES, reg_rx_agg_err_bytes);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_lif_rx_agg_err_zeroes_set(uint32_t rxaggerrzeroes)
{
    uint32_t reg_rx_agg_err_zeroes=0;

#ifdef VALIDATE_PARMS
#endif

    reg_rx_agg_err_zeroes = RU_FIELD_SET(0, LIF, RX_AGG_ERR_ZEROES, RXAGGERRZEROES, reg_rx_agg_err_zeroes, rxaggerrzeroes);

    RU_REG_WRITE(0, LIF, RX_AGG_ERR_ZEROES, reg_rx_agg_err_zeroes);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_lif_rx_agg_err_zeroes_get(uint32_t *rxaggerrzeroes)
{
    uint32_t reg_rx_agg_err_zeroes=0;

#ifdef VALIDATE_PARMS
    if(!rxaggerrzeroes)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, LIF, RX_AGG_ERR_ZEROES, reg_rx_agg_err_zeroes);

    *rxaggerrzeroes = RU_FIELD_GET(0, LIF, RX_AGG_ERR_ZEROES, RXAGGERRZEROES, reg_rx_agg_err_zeroes);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_lif_rx_agg_no_err_blks_set(uint32_t rxaggnoerrblks)
{
    uint32_t reg_rx_agg_no_err_blks=0;

#ifdef VALIDATE_PARMS
#endif

    reg_rx_agg_no_err_blks = RU_FIELD_SET(0, LIF, RX_AGG_NO_ERR_BLKS, RXAGGNOERRBLKS, reg_rx_agg_no_err_blks, rxaggnoerrblks);

    RU_REG_WRITE(0, LIF, RX_AGG_NO_ERR_BLKS, reg_rx_agg_no_err_blks);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_lif_rx_agg_no_err_blks_get(uint32_t *rxaggnoerrblks)
{
    uint32_t reg_rx_agg_no_err_blks=0;

#ifdef VALIDATE_PARMS
    if(!rxaggnoerrblks)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, LIF, RX_AGG_NO_ERR_BLKS, reg_rx_agg_no_err_blks);

    *rxaggnoerrblks = RU_FIELD_GET(0, LIF, RX_AGG_NO_ERR_BLKS, RXAGGNOERRBLKS, reg_rx_agg_no_err_blks);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_lif_rx_agg_cor_blks_set(uint32_t rxaggcorrblks)
{
    uint32_t reg_rx_agg_cor_blks=0;

#ifdef VALIDATE_PARMS
#endif

    reg_rx_agg_cor_blks = RU_FIELD_SET(0, LIF, RX_AGG_COR_BLKS, RXAGGCORRBLKS, reg_rx_agg_cor_blks, rxaggcorrblks);

    RU_REG_WRITE(0, LIF, RX_AGG_COR_BLKS, reg_rx_agg_cor_blks);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_lif_rx_agg_cor_blks_get(uint32_t *rxaggcorrblks)
{
    uint32_t reg_rx_agg_cor_blks=0;

#ifdef VALIDATE_PARMS
    if(!rxaggcorrblks)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, LIF, RX_AGG_COR_BLKS, reg_rx_agg_cor_blks);

    *rxaggcorrblks = RU_FIELD_GET(0, LIF, RX_AGG_COR_BLKS, RXAGGCORRBLKS, reg_rx_agg_cor_blks);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_lif_rx_agg_uncor_blks_set(uint32_t rxagguncorrblks)
{
    uint32_t reg_rx_agg_uncor_blks=0;

#ifdef VALIDATE_PARMS
#endif

    reg_rx_agg_uncor_blks = RU_FIELD_SET(0, LIF, RX_AGG_UNCOR_BLKS, RXAGGUNCORRBLKS, reg_rx_agg_uncor_blks, rxagguncorrblks);

    RU_REG_WRITE(0, LIF, RX_AGG_UNCOR_BLKS, reg_rx_agg_uncor_blks);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_lif_rx_agg_uncor_blks_get(uint32_t *rxagguncorrblks)
{
    uint32_t reg_rx_agg_uncor_blks=0;

#ifdef VALIDATE_PARMS
    if(!rxagguncorrblks)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, LIF, RX_AGG_UNCOR_BLKS, reg_rx_agg_uncor_blks);

    *rxagguncorrblks = RU_FIELD_GET(0, LIF, RX_AGG_UNCOR_BLKS, RXAGGUNCORRBLKS, reg_rx_agg_uncor_blks);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_lif_rx_agg_err_ones_set(uint32_t rxaggerrones)
{
    uint32_t reg_rx_agg_err_ones=0;

#ifdef VALIDATE_PARMS
#endif

    reg_rx_agg_err_ones = RU_FIELD_SET(0, LIF, RX_AGG_ERR_ONES, RXAGGERRONES, reg_rx_agg_err_ones, rxaggerrones);

    RU_REG_WRITE(0, LIF, RX_AGG_ERR_ONES, reg_rx_agg_err_ones);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_lif_rx_agg_err_ones_get(uint32_t *rxaggerrones)
{
    uint32_t reg_rx_agg_err_ones=0;

#ifdef VALIDATE_PARMS
    if(!rxaggerrones)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, LIF, RX_AGG_ERR_ONES, reg_rx_agg_err_ones);

    *rxaggerrones = RU_FIELD_GET(0, LIF, RX_AGG_ERR_ONES, RXAGGERRONES, reg_rx_agg_err_ones);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_lif_rx_agg_err_frm_set(uint32_t rxaggerroredcnt)
{
    uint32_t reg_rx_agg_err_frm=0;

#ifdef VALIDATE_PARMS
#endif

    reg_rx_agg_err_frm = RU_FIELD_SET(0, LIF, RX_AGG_ERR_FRM, RXAGGERROREDCNT, reg_rx_agg_err_frm, rxaggerroredcnt);

    RU_REG_WRITE(0, LIF, RX_AGG_ERR_FRM, reg_rx_agg_err_frm);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_lif_rx_agg_err_frm_get(uint32_t *rxaggerroredcnt)
{
    uint32_t reg_rx_agg_err_frm=0;

#ifdef VALIDATE_PARMS
    if(!rxaggerroredcnt)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, LIF, RX_AGG_ERR_FRM, reg_rx_agg_err_frm);

    *rxaggerroredcnt = RU_FIELD_GET(0, LIF, RX_AGG_ERR_FRM, RXAGGERROREDCNT, reg_rx_agg_err_frm);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_lif_tx_pkt_cnt_set(uint32_t txframecnt)
{
    uint32_t reg_tx_pkt_cnt=0;

#ifdef VALIDATE_PARMS
#endif

    reg_tx_pkt_cnt = RU_FIELD_SET(0, LIF, TX_PKT_CNT, TXFRAMECNT, reg_tx_pkt_cnt, txframecnt);

    RU_REG_WRITE(0, LIF, TX_PKT_CNT, reg_tx_pkt_cnt);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_lif_tx_pkt_cnt_get(uint32_t *txframecnt)
{
    uint32_t reg_tx_pkt_cnt=0;

#ifdef VALIDATE_PARMS
    if(!txframecnt)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, LIF, TX_PKT_CNT, reg_tx_pkt_cnt);

    *txframecnt = RU_FIELD_GET(0, LIF, TX_PKT_CNT, TXFRAMECNT, reg_tx_pkt_cnt);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_lif_tx_byte_cnt_set(uint32_t txbytecnt)
{
    uint32_t reg_tx_byte_cnt=0;

#ifdef VALIDATE_PARMS
#endif

    reg_tx_byte_cnt = RU_FIELD_SET(0, LIF, TX_BYTE_CNT, TXBYTECNT, reg_tx_byte_cnt, txbytecnt);

    RU_REG_WRITE(0, LIF, TX_BYTE_CNT, reg_tx_byte_cnt);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_lif_tx_byte_cnt_get(uint32_t *txbytecnt)
{
    uint32_t reg_tx_byte_cnt=0;

#ifdef VALIDATE_PARMS
    if(!txbytecnt)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, LIF, TX_BYTE_CNT, reg_tx_byte_cnt);

    *txbytecnt = RU_FIELD_GET(0, LIF, TX_BYTE_CNT, TXBYTECNT, reg_tx_byte_cnt);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_lif_tx_non_fec_pkt_cnt_set(uint32_t txnonfecframecnt)
{
    uint32_t reg_tx_non_fec_pkt_cnt=0;

#ifdef VALIDATE_PARMS
#endif

    reg_tx_non_fec_pkt_cnt = RU_FIELD_SET(0, LIF, TX_NON_FEC_PKT_CNT, TXNONFECFRAMECNT, reg_tx_non_fec_pkt_cnt, txnonfecframecnt);

    RU_REG_WRITE(0, LIF, TX_NON_FEC_PKT_CNT, reg_tx_non_fec_pkt_cnt);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_lif_tx_non_fec_pkt_cnt_get(uint32_t *txnonfecframecnt)
{
    uint32_t reg_tx_non_fec_pkt_cnt=0;

#ifdef VALIDATE_PARMS
    if(!txnonfecframecnt)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, LIF, TX_NON_FEC_PKT_CNT, reg_tx_non_fec_pkt_cnt);

    *txnonfecframecnt = RU_FIELD_GET(0, LIF, TX_NON_FEC_PKT_CNT, TXNONFECFRAMECNT, reg_tx_non_fec_pkt_cnt);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_lif_tx_non_fec_byte_cnt_set(uint32_t txnonfecbytecnt)
{
    uint32_t reg_tx_non_fec_byte_cnt=0;

#ifdef VALIDATE_PARMS
#endif

    reg_tx_non_fec_byte_cnt = RU_FIELD_SET(0, LIF, TX_NON_FEC_BYTE_CNT, TXNONFECBYTECNT, reg_tx_non_fec_byte_cnt, txnonfecbytecnt);

    RU_REG_WRITE(0, LIF, TX_NON_FEC_BYTE_CNT, reg_tx_non_fec_byte_cnt);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_lif_tx_non_fec_byte_cnt_get(uint32_t *txnonfecbytecnt)
{
    uint32_t reg_tx_non_fec_byte_cnt=0;

#ifdef VALIDATE_PARMS
    if(!txnonfecbytecnt)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, LIF, TX_NON_FEC_BYTE_CNT, reg_tx_non_fec_byte_cnt);

    *txnonfecbytecnt = RU_FIELD_GET(0, LIF, TX_NON_FEC_BYTE_CNT, TXNONFECBYTECNT, reg_tx_non_fec_byte_cnt);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_lif_tx_fec_pkt_cnt_set(uint32_t txfecframecnt)
{
    uint32_t reg_tx_fec_pkt_cnt=0;

#ifdef VALIDATE_PARMS
#endif

    reg_tx_fec_pkt_cnt = RU_FIELD_SET(0, LIF, TX_FEC_PKT_CNT, TXFECFRAMECNT, reg_tx_fec_pkt_cnt, txfecframecnt);

    RU_REG_WRITE(0, LIF, TX_FEC_PKT_CNT, reg_tx_fec_pkt_cnt);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_lif_tx_fec_pkt_cnt_get(uint32_t *txfecframecnt)
{
    uint32_t reg_tx_fec_pkt_cnt=0;

#ifdef VALIDATE_PARMS
    if(!txfecframecnt)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, LIF, TX_FEC_PKT_CNT, reg_tx_fec_pkt_cnt);

    *txfecframecnt = RU_FIELD_GET(0, LIF, TX_FEC_PKT_CNT, TXFECFRAMECNT, reg_tx_fec_pkt_cnt);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_lif_tx_fec_byte_cnt_set(uint32_t txfecbytecnt)
{
    uint32_t reg_tx_fec_byte_cnt=0;

#ifdef VALIDATE_PARMS
#endif

    reg_tx_fec_byte_cnt = RU_FIELD_SET(0, LIF, TX_FEC_BYTE_CNT, TXFECBYTECNT, reg_tx_fec_byte_cnt, txfecbytecnt);

    RU_REG_WRITE(0, LIF, TX_FEC_BYTE_CNT, reg_tx_fec_byte_cnt);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_lif_tx_fec_byte_cnt_get(uint32_t *txfecbytecnt)
{
    uint32_t reg_tx_fec_byte_cnt=0;

#ifdef VALIDATE_PARMS
    if(!txfecbytecnt)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, LIF, TX_FEC_BYTE_CNT, reg_tx_fec_byte_cnt);

    *txfecbytecnt = RU_FIELD_GET(0, LIF, TX_FEC_BYTE_CNT, TXFECBYTECNT, reg_tx_fec_byte_cnt);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_lif_tx_fec_blk_cnt_set(uint32_t txfecblkscnt)
{
    uint32_t reg_tx_fec_blk_cnt=0;

#ifdef VALIDATE_PARMS
#endif

    reg_tx_fec_blk_cnt = RU_FIELD_SET(0, LIF, TX_FEC_BLK_CNT, TXFECBLKSCNT, reg_tx_fec_blk_cnt, txfecblkscnt);

    RU_REG_WRITE(0, LIF, TX_FEC_BLK_CNT, reg_tx_fec_blk_cnt);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_lif_tx_fec_blk_cnt_get(uint32_t *txfecblkscnt)
{
    uint32_t reg_tx_fec_blk_cnt=0;

#ifdef VALIDATE_PARMS
    if(!txfecblkscnt)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, LIF, TX_FEC_BLK_CNT, reg_tx_fec_blk_cnt);

    *txfecblkscnt = RU_FIELD_GET(0, LIF, TX_FEC_BLK_CNT, TXFECBLKSCNT, reg_tx_fec_blk_cnt);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_lif_tx_mpcp_pkt_cnt_set(uint32_t txmpcpframecnt)
{
    uint32_t reg_tx_mpcp_pkt_cnt=0;

#ifdef VALIDATE_PARMS
#endif

    reg_tx_mpcp_pkt_cnt = RU_FIELD_SET(0, LIF, TX_MPCP_PKT_CNT, TXMPCPFRAMECNT, reg_tx_mpcp_pkt_cnt, txmpcpframecnt);

    RU_REG_WRITE(0, LIF, TX_MPCP_PKT_CNT, reg_tx_mpcp_pkt_cnt);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_lif_tx_mpcp_pkt_cnt_get(uint32_t *txmpcpframecnt)
{
    uint32_t reg_tx_mpcp_pkt_cnt=0;

#ifdef VALIDATE_PARMS
    if(!txmpcpframecnt)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, LIF, TX_MPCP_PKT_CNT, reg_tx_mpcp_pkt_cnt);

    *txmpcpframecnt = RU_FIELD_GET(0, LIF, TX_MPCP_PKT_CNT, TXMPCPFRAMECNT, reg_tx_mpcp_pkt_cnt);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_lif_debug_tx_data_pkt_cnt_set(uint32_t txdataframecnt)
{
    uint32_t reg_debug_tx_data_pkt_cnt=0;

#ifdef VALIDATE_PARMS
#endif

    reg_debug_tx_data_pkt_cnt = RU_FIELD_SET(0, LIF, DEBUG_TX_DATA_PKT_CNT, TXDATAFRAMECNT, reg_debug_tx_data_pkt_cnt, txdataframecnt);

    RU_REG_WRITE(0, LIF, DEBUG_TX_DATA_PKT_CNT, reg_debug_tx_data_pkt_cnt);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_lif_debug_tx_data_pkt_cnt_get(uint32_t *txdataframecnt)
{
    uint32_t reg_debug_tx_data_pkt_cnt=0;

#ifdef VALIDATE_PARMS
    if(!txdataframecnt)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, LIF, DEBUG_TX_DATA_PKT_CNT, reg_debug_tx_data_pkt_cnt);

    *txdataframecnt = RU_FIELD_GET(0, LIF, DEBUG_TX_DATA_PKT_CNT, TXDATAFRAMECNT, reg_debug_tx_data_pkt_cnt);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_lif_fec_llid_status_set(uint32_t stkyfecrevcllidbmsk)
{
    uint32_t reg_fec_llid_status=0;

#ifdef VALIDATE_PARMS
#endif

    reg_fec_llid_status = RU_FIELD_SET(0, LIF, FEC_LLID_STATUS, STKYFECREVCLLIDBMSK, reg_fec_llid_status, stkyfecrevcllidbmsk);

    RU_REG_WRITE(0, LIF, FEC_LLID_STATUS, reg_fec_llid_status);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_lif_fec_llid_status_get(uint32_t *stkyfecrevcllidbmsk)
{
    uint32_t reg_fec_llid_status=0;

#ifdef VALIDATE_PARMS
    if(!stkyfecrevcllidbmsk)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, LIF, FEC_LLID_STATUS, reg_fec_llid_status);

    *stkyfecrevcllidbmsk = RU_FIELD_GET(0, LIF, FEC_LLID_STATUS, STKYFECREVCLLIDBMSK, reg_fec_llid_status);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_lif_sec_rx_tek_ig_iv_llid_set(uint32_t cfigivnullllid)
{
    uint32_t reg_sec_rx_tek_ig_iv_llid=0;

#ifdef VALIDATE_PARMS
    if((cfigivnullllid >= _17BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_sec_rx_tek_ig_iv_llid = RU_FIELD_SET(0, LIF, SEC_RX_TEK_IG_IV_LLID, CFIGIVNULLLLID, reg_sec_rx_tek_ig_iv_llid, cfigivnullllid);

    RU_REG_WRITE(0, LIF, SEC_RX_TEK_IG_IV_LLID, reg_sec_rx_tek_ig_iv_llid);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_lif_sec_rx_tek_ig_iv_llid_get(uint32_t *cfigivnullllid)
{
    uint32_t reg_sec_rx_tek_ig_iv_llid=0;

#ifdef VALIDATE_PARMS
    if(!cfigivnullllid)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, LIF, SEC_RX_TEK_IG_IV_LLID, reg_sec_rx_tek_ig_iv_llid);

    *cfigivnullllid = RU_FIELD_GET(0, LIF, SEC_RX_TEK_IG_IV_LLID, CFIGIVNULLLLID, reg_sec_rx_tek_ig_iv_llid);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_lif_pon_ber_interv_thresh_set(uint32_t cfrxlifberinterval, uint16_t cfrxlifberthreshld, uint8_t cfrxlifbercntrl)
{
    uint32_t reg_pon_ber_interv_thresh=0;

#ifdef VALIDATE_PARMS
    if((cfrxlifberinterval >= _17BITS_MAX_VAL_) ||
       (cfrxlifberthreshld >= _13BITS_MAX_VAL_) ||
       (cfrxlifbercntrl >= _2BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_pon_ber_interv_thresh = RU_FIELD_SET(0, LIF, PON_BER_INTERV_THRESH, CFRXLIFBERINTERVAL, reg_pon_ber_interv_thresh, cfrxlifberinterval);
    reg_pon_ber_interv_thresh = RU_FIELD_SET(0, LIF, PON_BER_INTERV_THRESH, CFRXLIFBERTHRESHLD, reg_pon_ber_interv_thresh, cfrxlifberthreshld);
    reg_pon_ber_interv_thresh = RU_FIELD_SET(0, LIF, PON_BER_INTERV_THRESH, CFRXLIFBERCNTRL, reg_pon_ber_interv_thresh, cfrxlifbercntrl);

    RU_REG_WRITE(0, LIF, PON_BER_INTERV_THRESH, reg_pon_ber_interv_thresh);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_lif_pon_ber_interv_thresh_get(uint32_t *cfrxlifberinterval, uint16_t *cfrxlifberthreshld, uint8_t *cfrxlifbercntrl)
{
    uint32_t reg_pon_ber_interv_thresh=0;

#ifdef VALIDATE_PARMS
    if(!cfrxlifberinterval || !cfrxlifberthreshld || !cfrxlifbercntrl)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, LIF, PON_BER_INTERV_THRESH, reg_pon_ber_interv_thresh);

    *cfrxlifberinterval = RU_FIELD_GET(0, LIF, PON_BER_INTERV_THRESH, CFRXLIFBERINTERVAL, reg_pon_ber_interv_thresh);
    *cfrxlifberthreshld = RU_FIELD_GET(0, LIF, PON_BER_INTERV_THRESH, CFRXLIFBERTHRESHLD, reg_pon_ber_interv_thresh);
    *cfrxlifbercntrl = RU_FIELD_GET(0, LIF, PON_BER_INTERV_THRESH, CFRXLIFBERCNTRL, reg_pon_ber_interv_thresh);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_lif_lsr_mon_a_ctrl_set(bdmf_boolean iopbilaserens1a, bdmf_boolean cfglsrmonacthi, bdmf_boolean pbilasermonrsta_n_pre)
{
    uint32_t reg_lsr_mon_a_ctrl=0;

#ifdef VALIDATE_PARMS
    if((iopbilaserens1a >= _1BITS_MAX_VAL_) ||
       (cfglsrmonacthi >= _1BITS_MAX_VAL_) ||
       (pbilasermonrsta_n_pre >= _1BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_lsr_mon_a_ctrl = RU_FIELD_SET(0, LIF, LSR_MON_A_CTRL, IOPBILASERENS1A, reg_lsr_mon_a_ctrl, iopbilaserens1a);
    reg_lsr_mon_a_ctrl = RU_FIELD_SET(0, LIF, LSR_MON_A_CTRL, CFGLSRMONACTHI, reg_lsr_mon_a_ctrl, cfglsrmonacthi);
    reg_lsr_mon_a_ctrl = RU_FIELD_SET(0, LIF, LSR_MON_A_CTRL, PBILASERMONRSTA_N_PRE, reg_lsr_mon_a_ctrl, pbilasermonrsta_n_pre);

    RU_REG_WRITE(0, LIF, LSR_MON_A_CTRL, reg_lsr_mon_a_ctrl);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_lif_lsr_mon_a_ctrl_get(bdmf_boolean *iopbilaserens1a, bdmf_boolean *cfglsrmonacthi, bdmf_boolean *pbilasermonrsta_n_pre)
{
    uint32_t reg_lsr_mon_a_ctrl=0;

#ifdef VALIDATE_PARMS
    if(!iopbilaserens1a || !cfglsrmonacthi || !pbilasermonrsta_n_pre)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, LIF, LSR_MON_A_CTRL, reg_lsr_mon_a_ctrl);

    *iopbilaserens1a = RU_FIELD_GET(0, LIF, LSR_MON_A_CTRL, IOPBILASERENS1A, reg_lsr_mon_a_ctrl);
    *cfglsrmonacthi = RU_FIELD_GET(0, LIF, LSR_MON_A_CTRL, CFGLSRMONACTHI, reg_lsr_mon_a_ctrl);
    *pbilasermonrsta_n_pre = RU_FIELD_GET(0, LIF, LSR_MON_A_CTRL, PBILASERMONRSTA_N_PRE, reg_lsr_mon_a_ctrl);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_lif_lsr_mon_a_max_thr_set(uint32_t cfglasermonmaxa)
{
    uint32_t reg_lsr_mon_a_max_thr=0;

#ifdef VALIDATE_PARMS
#endif

    reg_lsr_mon_a_max_thr = RU_FIELD_SET(0, LIF, LSR_MON_A_MAX_THR, CFGLASERMONMAXA, reg_lsr_mon_a_max_thr, cfglasermonmaxa);

    RU_REG_WRITE(0, LIF, LSR_MON_A_MAX_THR, reg_lsr_mon_a_max_thr);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_lif_lsr_mon_a_max_thr_get(uint32_t *cfglasermonmaxa)
{
    uint32_t reg_lsr_mon_a_max_thr=0;

#ifdef VALIDATE_PARMS
    if(!cfglasermonmaxa)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, LIF, LSR_MON_A_MAX_THR, reg_lsr_mon_a_max_thr);

    *cfglasermonmaxa = RU_FIELD_GET(0, LIF, LSR_MON_A_MAX_THR, CFGLASERMONMAXA, reg_lsr_mon_a_max_thr);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_lif_lsr_mon_a_bst_len_get(uint32_t *laserontimea)
{
    uint32_t reg_lsr_mon_a_bst_len=0;

#ifdef VALIDATE_PARMS
    if(!laserontimea)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, LIF, LSR_MON_A_BST_LEN, reg_lsr_mon_a_bst_len);

    *laserontimea = RU_FIELD_GET(0, LIF, LSR_MON_A_BST_LEN, LASERONTIMEA, reg_lsr_mon_a_bst_len);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_lif_lsr_mon_a_bst_cnt_get(uint32_t *lasermonbrstcnta)
{
    uint32_t reg_lsr_mon_a_bst_cnt=0;

#ifdef VALIDATE_PARMS
    if(!lasermonbrstcnta)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, LIF, LSR_MON_A_BST_CNT, reg_lsr_mon_a_bst_cnt);

    *lasermonbrstcnta = RU_FIELD_GET(0, LIF, LSR_MON_A_BST_CNT, LASERMONBRSTCNTA, reg_lsr_mon_a_bst_cnt);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_lif_debug_pon_sm_get(uint8_t *aligncsqq, uint8_t *rxfecifcsqq)
{
    uint32_t reg_debug_pon_sm=0;

#ifdef VALIDATE_PARMS
    if(!aligncsqq || !rxfecifcsqq)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, LIF, DEBUG_PON_SM, reg_debug_pon_sm);

    *aligncsqq = RU_FIELD_GET(0, LIF, DEBUG_PON_SM, ALIGNCSQQ, reg_debug_pon_sm);
    *rxfecifcsqq = RU_FIELD_GET(0, LIF, DEBUG_PON_SM, RXFECIFCSQQ, reg_debug_pon_sm);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_lif_debug_fec_sm_get(uint8_t *rxsyncsqq, uint8_t *rxcorcs, uint8_t *fecrxoutcs)
{
    uint32_t reg_debug_fec_sm=0;

#ifdef VALIDATE_PARMS
    if(!rxsyncsqq || !rxcorcs || !fecrxoutcs)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, LIF, DEBUG_FEC_SM, reg_debug_fec_sm);

    *rxsyncsqq = RU_FIELD_GET(0, LIF, DEBUG_FEC_SM, RXSYNCSQQ, reg_debug_fec_sm);
    *rxcorcs = RU_FIELD_GET(0, LIF, DEBUG_FEC_SM, RXCORCS, reg_debug_fec_sm);
    *fecrxoutcs = RU_FIELD_GET(0, LIF, DEBUG_FEC_SM, FECRXOUTCS, reg_debug_fec_sm);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_lif_ae_pktnum_window_set(uint32_t cfgaepktnumwnd)
{
    uint32_t reg_ae_pktnum_window=0;

#ifdef VALIDATE_PARMS
#endif

    reg_ae_pktnum_window = RU_FIELD_SET(0, LIF, AE_PKTNUM_WINDOW, CFGAEPKTNUMWND, reg_ae_pktnum_window, cfgaepktnumwnd);

    RU_REG_WRITE(0, LIF, AE_PKTNUM_WINDOW, reg_ae_pktnum_window);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_lif_ae_pktnum_window_get(uint32_t *cfgaepktnumwnd)
{
    uint32_t reg_ae_pktnum_window=0;

#ifdef VALIDATE_PARMS
    if(!cfgaepktnumwnd)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, LIF, AE_PKTNUM_WINDOW, reg_ae_pktnum_window);

    *cfgaepktnumwnd = RU_FIELD_GET(0, LIF, AE_PKTNUM_WINDOW, CFGAEPKTNUMWND, reg_ae_pktnum_window);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_lif_ae_pktnum_thresh_set(uint32_t cfgpktnummaxthresh)
{
    uint32_t reg_ae_pktnum_thresh=0;

#ifdef VALIDATE_PARMS
#endif

    reg_ae_pktnum_thresh = RU_FIELD_SET(0, LIF, AE_PKTNUM_THRESH, CFGPKTNUMMAXTHRESH, reg_ae_pktnum_thresh, cfgpktnummaxthresh);

    RU_REG_WRITE(0, LIF, AE_PKTNUM_THRESH, reg_ae_pktnum_thresh);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_lif_ae_pktnum_thresh_get(uint32_t *cfgpktnummaxthresh)
{
    uint32_t reg_ae_pktnum_thresh=0;

#ifdef VALIDATE_PARMS
    if(!cfgpktnummaxthresh)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, LIF, AE_PKTNUM_THRESH, reg_ae_pktnum_thresh);

    *cfgpktnummaxthresh = RU_FIELD_GET(0, LIF, AE_PKTNUM_THRESH, CFGPKTNUMMAXTHRESH, reg_ae_pktnum_thresh);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_lif_ae_pktnum_stat_get(uint8_t *secupindxwtpktnummax, uint8_t *secdnindxwtpktnumabort)
{
    uint32_t reg_ae_pktnum_stat=0;

#ifdef VALIDATE_PARMS
    if(!secupindxwtpktnummax || !secdnindxwtpktnumabort)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, LIF, AE_PKTNUM_STAT, reg_ae_pktnum_stat);

    *secupindxwtpktnummax = RU_FIELD_GET(0, LIF, AE_PKTNUM_STAT, SECUPINDXWTPKTNUMMAX, reg_ae_pktnum_stat);
    *secdnindxwtpktnumabort = RU_FIELD_GET(0, LIF, AE_PKTNUM_STAT, SECDNINDXWTPKTNUMABORT, reg_ae_pktnum_stat);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_lif_llid_8_set(uint32_t cfgllid8)
{
    uint32_t reg_llid_8=0;

#ifdef VALIDATE_PARMS
    if((cfgllid8 >= _17BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_llid_8 = RU_FIELD_SET(0, LIF, LLID_8, CFGLLID8, reg_llid_8, cfgllid8);

    RU_REG_WRITE(0, LIF, LLID_8, reg_llid_8);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_lif_llid_8_get(uint32_t *cfgllid8)
{
    uint32_t reg_llid_8=0;

#ifdef VALIDATE_PARMS
    if(!cfgllid8)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, LIF, LLID_8, reg_llid_8);

    *cfgllid8 = RU_FIELD_GET(0, LIF, LLID_8, CFGLLID8, reg_llid_8);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_lif_llid_9_set(uint32_t cfgllid9)
{
    uint32_t reg_llid_9=0;

#ifdef VALIDATE_PARMS
    if((cfgllid9 >= _17BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_llid_9 = RU_FIELD_SET(0, LIF, LLID_9, CFGLLID9, reg_llid_9, cfgllid9);

    RU_REG_WRITE(0, LIF, LLID_9, reg_llid_9);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_lif_llid_9_get(uint32_t *cfgllid9)
{
    uint32_t reg_llid_9=0;

#ifdef VALIDATE_PARMS
    if(!cfgllid9)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, LIF, LLID_9, reg_llid_9);

    *cfgllid9 = RU_FIELD_GET(0, LIF, LLID_9, CFGLLID9, reg_llid_9);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_lif_llid_10_set(uint32_t cfgllid10)
{
    uint32_t reg_llid_10=0;

#ifdef VALIDATE_PARMS
    if((cfgllid10 >= _17BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_llid_10 = RU_FIELD_SET(0, LIF, LLID_10, CFGLLID10, reg_llid_10, cfgllid10);

    RU_REG_WRITE(0, LIF, LLID_10, reg_llid_10);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_lif_llid_10_get(uint32_t *cfgllid10)
{
    uint32_t reg_llid_10=0;

#ifdef VALIDATE_PARMS
    if(!cfgllid10)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, LIF, LLID_10, reg_llid_10);

    *cfgllid10 = RU_FIELD_GET(0, LIF, LLID_10, CFGLLID10, reg_llid_10);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_lif_llid_11_set(uint32_t cfgllid11)
{
    uint32_t reg_llid_11=0;

#ifdef VALIDATE_PARMS
    if((cfgllid11 >= _17BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_llid_11 = RU_FIELD_SET(0, LIF, LLID_11, CFGLLID11, reg_llid_11, cfgllid11);

    RU_REG_WRITE(0, LIF, LLID_11, reg_llid_11);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_lif_llid_11_get(uint32_t *cfgllid11)
{
    uint32_t reg_llid_11=0;

#ifdef VALIDATE_PARMS
    if(!cfgllid11)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, LIF, LLID_11, reg_llid_11);

    *cfgllid11 = RU_FIELD_GET(0, LIF, LLID_11, CFGLLID11, reg_llid_11);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_lif_llid_12_set(uint32_t cfgllid12)
{
    uint32_t reg_llid_12=0;

#ifdef VALIDATE_PARMS
    if((cfgllid12 >= _17BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_llid_12 = RU_FIELD_SET(0, LIF, LLID_12, CFGLLID12, reg_llid_12, cfgllid12);

    RU_REG_WRITE(0, LIF, LLID_12, reg_llid_12);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_lif_llid_12_get(uint32_t *cfgllid12)
{
    uint32_t reg_llid_12=0;

#ifdef VALIDATE_PARMS
    if(!cfgllid12)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, LIF, LLID_12, reg_llid_12);

    *cfgllid12 = RU_FIELD_GET(0, LIF, LLID_12, CFGLLID12, reg_llid_12);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_lif_llid_13_set(uint32_t cfgllid13)
{
    uint32_t reg_llid_13=0;

#ifdef VALIDATE_PARMS
    if((cfgllid13 >= _17BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_llid_13 = RU_FIELD_SET(0, LIF, LLID_13, CFGLLID13, reg_llid_13, cfgllid13);

    RU_REG_WRITE(0, LIF, LLID_13, reg_llid_13);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_lif_llid_13_get(uint32_t *cfgllid13)
{
    uint32_t reg_llid_13=0;

#ifdef VALIDATE_PARMS
    if(!cfgllid13)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, LIF, LLID_13, reg_llid_13);

    *cfgllid13 = RU_FIELD_GET(0, LIF, LLID_13, CFGLLID13, reg_llid_13);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_lif_llid_14_set(uint32_t cfgllid14)
{
    uint32_t reg_llid_14=0;

#ifdef VALIDATE_PARMS
    if((cfgllid14 >= _17BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_llid_14 = RU_FIELD_SET(0, LIF, LLID_14, CFGLLID14, reg_llid_14, cfgllid14);

    RU_REG_WRITE(0, LIF, LLID_14, reg_llid_14);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_lif_llid_14_get(uint32_t *cfgllid14)
{
    uint32_t reg_llid_14=0;

#ifdef VALIDATE_PARMS
    if(!cfgllid14)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, LIF, LLID_14, reg_llid_14);

    *cfgllid14 = RU_FIELD_GET(0, LIF, LLID_14, CFGLLID14, reg_llid_14);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_lif_llid_15_set(uint32_t cfgllid15)
{
    uint32_t reg_llid_15=0;

#ifdef VALIDATE_PARMS
    if((cfgllid15 >= _17BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_llid_15 = RU_FIELD_SET(0, LIF, LLID_15, CFGLLID15, reg_llid_15, cfgllid15);

    RU_REG_WRITE(0, LIF, LLID_15, reg_llid_15);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_lif_llid_15_get(uint32_t *cfgllid15)
{
    uint32_t reg_llid_15=0;

#ifdef VALIDATE_PARMS
    if(!cfgllid15)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, LIF, LLID_15, reg_llid_15);

    *cfgllid15 = RU_FIELD_GET(0, LIF, LLID_15, CFGLLID15, reg_llid_15);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_lif_llid_24_set(uint32_t cfgllid24)
{
    uint32_t reg_llid_24=0;

#ifdef VALIDATE_PARMS
    if((cfgllid24 >= _17BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_llid_24 = RU_FIELD_SET(0, LIF, LLID_24, CFGLLID24, reg_llid_24, cfgllid24);

    RU_REG_WRITE(0, LIF, LLID_24, reg_llid_24);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_lif_llid_24_get(uint32_t *cfgllid24)
{
    uint32_t reg_llid_24=0;

#ifdef VALIDATE_PARMS
    if(!cfgllid24)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, LIF, LLID_24, reg_llid_24);

    *cfgllid24 = RU_FIELD_GET(0, LIF, LLID_24, CFGLLID24, reg_llid_24);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_lif_llid_25_set(uint32_t cfgllid25)
{
    uint32_t reg_llid_25=0;

#ifdef VALIDATE_PARMS
    if((cfgllid25 >= _17BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_llid_25 = RU_FIELD_SET(0, LIF, LLID_25, CFGLLID25, reg_llid_25, cfgllid25);

    RU_REG_WRITE(0, LIF, LLID_25, reg_llid_25);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_lif_llid_25_get(uint32_t *cfgllid25)
{
    uint32_t reg_llid_25=0;

#ifdef VALIDATE_PARMS
    if(!cfgllid25)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, LIF, LLID_25, reg_llid_25);

    *cfgllid25 = RU_FIELD_GET(0, LIF, LLID_25, CFGLLID25, reg_llid_25);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_lif_llid_26_set(uint32_t cfgllid26)
{
    uint32_t reg_llid_26=0;

#ifdef VALIDATE_PARMS
    if((cfgllid26 >= _17BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_llid_26 = RU_FIELD_SET(0, LIF, LLID_26, CFGLLID26, reg_llid_26, cfgllid26);

    RU_REG_WRITE(0, LIF, LLID_26, reg_llid_26);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_lif_llid_26_get(uint32_t *cfgllid26)
{
    uint32_t reg_llid_26=0;

#ifdef VALIDATE_PARMS
    if(!cfgllid26)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, LIF, LLID_26, reg_llid_26);

    *cfgllid26 = RU_FIELD_GET(0, LIF, LLID_26, CFGLLID26, reg_llid_26);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_lif_llid_27_set(uint32_t cfgllid27)
{
    uint32_t reg_llid_27=0;

#ifdef VALIDATE_PARMS
    if((cfgllid27 >= _17BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_llid_27 = RU_FIELD_SET(0, LIF, LLID_27, CFGLLID27, reg_llid_27, cfgllid27);

    RU_REG_WRITE(0, LIF, LLID_27, reg_llid_27);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_lif_llid_27_get(uint32_t *cfgllid27)
{
    uint32_t reg_llid_27=0;

#ifdef VALIDATE_PARMS
    if(!cfgllid27)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, LIF, LLID_27, reg_llid_27);

    *cfgllid27 = RU_FIELD_GET(0, LIF, LLID_27, CFGLLID27, reg_llid_27);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_lif_llid_28_set(uint32_t cfgllid28)
{
    uint32_t reg_llid_28=0;

#ifdef VALIDATE_PARMS
    if((cfgllid28 >= _17BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_llid_28 = RU_FIELD_SET(0, LIF, LLID_28, CFGLLID28, reg_llid_28, cfgllid28);

    RU_REG_WRITE(0, LIF, LLID_28, reg_llid_28);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_lif_llid_28_get(uint32_t *cfgllid28)
{
    uint32_t reg_llid_28=0;

#ifdef VALIDATE_PARMS
    if(!cfgllid28)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, LIF, LLID_28, reg_llid_28);

    *cfgllid28 = RU_FIELD_GET(0, LIF, LLID_28, CFGLLID28, reg_llid_28);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_lif_llid_29_set(uint32_t cfgllid29)
{
    uint32_t reg_llid_29=0;

#ifdef VALIDATE_PARMS
    if((cfgllid29 >= _17BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_llid_29 = RU_FIELD_SET(0, LIF, LLID_29, CFGLLID29, reg_llid_29, cfgllid29);

    RU_REG_WRITE(0, LIF, LLID_29, reg_llid_29);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_lif_llid_29_get(uint32_t *cfgllid29)
{
    uint32_t reg_llid_29=0;

#ifdef VALIDATE_PARMS
    if(!cfgllid29)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, LIF, LLID_29, reg_llid_29);

    *cfgllid29 = RU_FIELD_GET(0, LIF, LLID_29, CFGLLID29, reg_llid_29);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_lif_llid_30_set(uint32_t cfgllid30)
{
    uint32_t reg_llid_30=0;

#ifdef VALIDATE_PARMS
    if((cfgllid30 >= _17BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_llid_30 = RU_FIELD_SET(0, LIF, LLID_30, CFGLLID30, reg_llid_30, cfgllid30);

    RU_REG_WRITE(0, LIF, LLID_30, reg_llid_30);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_lif_llid_30_get(uint32_t *cfgllid30)
{
    uint32_t reg_llid_30=0;

#ifdef VALIDATE_PARMS
    if(!cfgllid30)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, LIF, LLID_30, reg_llid_30);

    *cfgllid30 = RU_FIELD_GET(0, LIF, LLID_30, CFGLLID30, reg_llid_30);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_lif_llid_31_set(uint32_t cfgllid31)
{
    uint32_t reg_llid_31=0;

#ifdef VALIDATE_PARMS
    if((cfgllid31 >= _17BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_llid_31 = RU_FIELD_SET(0, LIF, LLID_31, CFGLLID31, reg_llid_31, cfgllid31);

    RU_REG_WRITE(0, LIF, LLID_31, reg_llid_31);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_lif_llid_31_get(uint32_t *cfgllid31)
{
    uint32_t reg_llid_31=0;

#ifdef VALIDATE_PARMS
    if(!cfgllid31)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, LIF, LLID_31, reg_llid_31);

    *cfgllid31 = RU_FIELD_GET(0, LIF, LLID_31, CFGLLID31, reg_llid_31);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_lif_vlan_type_set(uint16_t cfgvlantype)
{
    uint32_t reg_vlan_type=0;

#ifdef VALIDATE_PARMS
#endif

    reg_vlan_type = RU_FIELD_SET(0, LIF, VLAN_TYPE, CFGVLANTYPE, reg_vlan_type, cfgvlantype);

    RU_REG_WRITE(0, LIF, VLAN_TYPE, reg_vlan_type);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_lif_vlan_type_get(uint16_t *cfgvlantype)
{
    uint32_t reg_vlan_type=0;

#ifdef VALIDATE_PARMS
    if(!cfgvlantype)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, LIF, VLAN_TYPE, reg_vlan_type);

    *cfgvlantype = RU_FIELD_GET(0, LIF, VLAN_TYPE, CFGVLANTYPE, reg_vlan_type);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_lif_p2p_ae_sci_en_set(uint16_t cfgp2pscien)
{
    uint32_t reg_p2p_ae_sci_en=0;

#ifdef VALIDATE_PARMS
#endif

    reg_p2p_ae_sci_en = RU_FIELD_SET(0, LIF, P2P_AE_SCI_EN, CFGP2PSCIEN, reg_p2p_ae_sci_en, cfgp2pscien);

    RU_REG_WRITE(0, LIF, P2P_AE_SCI_EN, reg_p2p_ae_sci_en);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_lif_p2p_ae_sci_en_get(uint16_t *cfgp2pscien)
{
    uint32_t reg_p2p_ae_sci_en=0;

#ifdef VALIDATE_PARMS
    if(!cfgp2pscien)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, LIF, P2P_AE_SCI_EN, reg_p2p_ae_sci_en);

    *cfgp2pscien = RU_FIELD_GET(0, LIF, P2P_AE_SCI_EN, CFGP2PSCIEN, reg_p2p_ae_sci_en);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_lif_p2p_ae_sci_lo_0_set(uint32_t cfgp2psci_lo_0)
{
    uint32_t reg_p2p_ae_sci_lo_0=0;

#ifdef VALIDATE_PARMS
#endif

    reg_p2p_ae_sci_lo_0 = RU_FIELD_SET(0, LIF, P2P_AE_SCI_LO_0, CFGP2PSCI_LO_0, reg_p2p_ae_sci_lo_0, cfgp2psci_lo_0);

    RU_REG_WRITE(0, LIF, P2P_AE_SCI_LO_0, reg_p2p_ae_sci_lo_0);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_lif_p2p_ae_sci_lo_0_get(uint32_t *cfgp2psci_lo_0)
{
    uint32_t reg_p2p_ae_sci_lo_0=0;

#ifdef VALIDATE_PARMS
    if(!cfgp2psci_lo_0)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, LIF, P2P_AE_SCI_LO_0, reg_p2p_ae_sci_lo_0);

    *cfgp2psci_lo_0 = RU_FIELD_GET(0, LIF, P2P_AE_SCI_LO_0, CFGP2PSCI_LO_0, reg_p2p_ae_sci_lo_0);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_lif_p2p_ae_sci_hi_0_set(uint32_t cfgp2psci_hi_0)
{
    uint32_t reg_p2p_ae_sci_hi_0=0;

#ifdef VALIDATE_PARMS
#endif

    reg_p2p_ae_sci_hi_0 = RU_FIELD_SET(0, LIF, P2P_AE_SCI_HI_0, CFGP2PSCI_HI_0, reg_p2p_ae_sci_hi_0, cfgp2psci_hi_0);

    RU_REG_WRITE(0, LIF, P2P_AE_SCI_HI_0, reg_p2p_ae_sci_hi_0);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_lif_p2p_ae_sci_hi_0_get(uint32_t *cfgp2psci_hi_0)
{
    uint32_t reg_p2p_ae_sci_hi_0=0;

#ifdef VALIDATE_PARMS
    if(!cfgp2psci_hi_0)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, LIF, P2P_AE_SCI_HI_0, reg_p2p_ae_sci_hi_0);

    *cfgp2psci_hi_0 = RU_FIELD_GET(0, LIF, P2P_AE_SCI_HI_0, CFGP2PSCI_HI_0, reg_p2p_ae_sci_hi_0);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_lif_p2p_ae_sci_lo_1_set(uint32_t cfgp2psci_lo_1)
{
    uint32_t reg_p2p_ae_sci_lo_1=0;

#ifdef VALIDATE_PARMS
#endif

    reg_p2p_ae_sci_lo_1 = RU_FIELD_SET(0, LIF, P2P_AE_SCI_LO_1, CFGP2PSCI_LO_1, reg_p2p_ae_sci_lo_1, cfgp2psci_lo_1);

    RU_REG_WRITE(0, LIF, P2P_AE_SCI_LO_1, reg_p2p_ae_sci_lo_1);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_lif_p2p_ae_sci_lo_1_get(uint32_t *cfgp2psci_lo_1)
{
    uint32_t reg_p2p_ae_sci_lo_1=0;

#ifdef VALIDATE_PARMS
    if(!cfgp2psci_lo_1)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, LIF, P2P_AE_SCI_LO_1, reg_p2p_ae_sci_lo_1);

    *cfgp2psci_lo_1 = RU_FIELD_GET(0, LIF, P2P_AE_SCI_LO_1, CFGP2PSCI_LO_1, reg_p2p_ae_sci_lo_1);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_lif_p2p_ae_sci_hi_1_set(uint32_t cfgp2psci_hi_1)
{
    uint32_t reg_p2p_ae_sci_hi_1=0;

#ifdef VALIDATE_PARMS
#endif

    reg_p2p_ae_sci_hi_1 = RU_FIELD_SET(0, LIF, P2P_AE_SCI_HI_1, CFGP2PSCI_HI_1, reg_p2p_ae_sci_hi_1, cfgp2psci_hi_1);

    RU_REG_WRITE(0, LIF, P2P_AE_SCI_HI_1, reg_p2p_ae_sci_hi_1);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_lif_p2p_ae_sci_hi_1_get(uint32_t *cfgp2psci_hi_1)
{
    uint32_t reg_p2p_ae_sci_hi_1=0;

#ifdef VALIDATE_PARMS
    if(!cfgp2psci_hi_1)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, LIF, P2P_AE_SCI_HI_1, reg_p2p_ae_sci_hi_1);

    *cfgp2psci_hi_1 = RU_FIELD_GET(0, LIF, P2P_AE_SCI_HI_1, CFGP2PSCI_HI_1, reg_p2p_ae_sci_hi_1);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_lif_p2p_ae_sci_lo_2_set(uint32_t cfgp2psci_lo_2)
{
    uint32_t reg_p2p_ae_sci_lo_2=0;

#ifdef VALIDATE_PARMS
#endif

    reg_p2p_ae_sci_lo_2 = RU_FIELD_SET(0, LIF, P2P_AE_SCI_LO_2, CFGP2PSCI_LO_2, reg_p2p_ae_sci_lo_2, cfgp2psci_lo_2);

    RU_REG_WRITE(0, LIF, P2P_AE_SCI_LO_2, reg_p2p_ae_sci_lo_2);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_lif_p2p_ae_sci_lo_2_get(uint32_t *cfgp2psci_lo_2)
{
    uint32_t reg_p2p_ae_sci_lo_2=0;

#ifdef VALIDATE_PARMS
    if(!cfgp2psci_lo_2)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, LIF, P2P_AE_SCI_LO_2, reg_p2p_ae_sci_lo_2);

    *cfgp2psci_lo_2 = RU_FIELD_GET(0, LIF, P2P_AE_SCI_LO_2, CFGP2PSCI_LO_2, reg_p2p_ae_sci_lo_2);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_lif_p2p_ae_sci_hi_2_set(uint32_t cfgp2psci_hi_2)
{
    uint32_t reg_p2p_ae_sci_hi_2=0;

#ifdef VALIDATE_PARMS
#endif

    reg_p2p_ae_sci_hi_2 = RU_FIELD_SET(0, LIF, P2P_AE_SCI_HI_2, CFGP2PSCI_HI_2, reg_p2p_ae_sci_hi_2, cfgp2psci_hi_2);

    RU_REG_WRITE(0, LIF, P2P_AE_SCI_HI_2, reg_p2p_ae_sci_hi_2);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_lif_p2p_ae_sci_hi_2_get(uint32_t *cfgp2psci_hi_2)
{
    uint32_t reg_p2p_ae_sci_hi_2=0;

#ifdef VALIDATE_PARMS
    if(!cfgp2psci_hi_2)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, LIF, P2P_AE_SCI_HI_2, reg_p2p_ae_sci_hi_2);

    *cfgp2psci_hi_2 = RU_FIELD_GET(0, LIF, P2P_AE_SCI_HI_2, CFGP2PSCI_HI_2, reg_p2p_ae_sci_hi_2);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_lif_p2p_ae_sci_lo_3_set(uint32_t cfgp2psci_lo_3)
{
    uint32_t reg_p2p_ae_sci_lo_3=0;

#ifdef VALIDATE_PARMS
#endif

    reg_p2p_ae_sci_lo_3 = RU_FIELD_SET(0, LIF, P2P_AE_SCI_LO_3, CFGP2PSCI_LO_3, reg_p2p_ae_sci_lo_3, cfgp2psci_lo_3);

    RU_REG_WRITE(0, LIF, P2P_AE_SCI_LO_3, reg_p2p_ae_sci_lo_3);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_lif_p2p_ae_sci_lo_3_get(uint32_t *cfgp2psci_lo_3)
{
    uint32_t reg_p2p_ae_sci_lo_3=0;

#ifdef VALIDATE_PARMS
    if(!cfgp2psci_lo_3)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, LIF, P2P_AE_SCI_LO_3, reg_p2p_ae_sci_lo_3);

    *cfgp2psci_lo_3 = RU_FIELD_GET(0, LIF, P2P_AE_SCI_LO_3, CFGP2PSCI_LO_3, reg_p2p_ae_sci_lo_3);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_lif_p2p_ae_sci_hi_3_set(uint32_t cfgp2psci_hi_3)
{
    uint32_t reg_p2p_ae_sci_hi_3=0;

#ifdef VALIDATE_PARMS
#endif

    reg_p2p_ae_sci_hi_3 = RU_FIELD_SET(0, LIF, P2P_AE_SCI_HI_3, CFGP2PSCI_HI_3, reg_p2p_ae_sci_hi_3, cfgp2psci_hi_3);

    RU_REG_WRITE(0, LIF, P2P_AE_SCI_HI_3, reg_p2p_ae_sci_hi_3);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_lif_p2p_ae_sci_hi_3_get(uint32_t *cfgp2psci_hi_3)
{
    uint32_t reg_p2p_ae_sci_hi_3=0;

#ifdef VALIDATE_PARMS
    if(!cfgp2psci_hi_3)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, LIF, P2P_AE_SCI_HI_3, reg_p2p_ae_sci_hi_3);

    *cfgp2psci_hi_3 = RU_FIELD_GET(0, LIF, P2P_AE_SCI_HI_3, CFGP2PSCI_HI_3, reg_p2p_ae_sci_hi_3);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_lif_p2p_ae_sci_lo_4_set(uint32_t cfgp2psci_lo_4)
{
    uint32_t reg_p2p_ae_sci_lo_4=0;

#ifdef VALIDATE_PARMS
#endif

    reg_p2p_ae_sci_lo_4 = RU_FIELD_SET(0, LIF, P2P_AE_SCI_LO_4, CFGP2PSCI_LO_4, reg_p2p_ae_sci_lo_4, cfgp2psci_lo_4);

    RU_REG_WRITE(0, LIF, P2P_AE_SCI_LO_4, reg_p2p_ae_sci_lo_4);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_lif_p2p_ae_sci_lo_4_get(uint32_t *cfgp2psci_lo_4)
{
    uint32_t reg_p2p_ae_sci_lo_4=0;

#ifdef VALIDATE_PARMS
    if(!cfgp2psci_lo_4)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, LIF, P2P_AE_SCI_LO_4, reg_p2p_ae_sci_lo_4);

    *cfgp2psci_lo_4 = RU_FIELD_GET(0, LIF, P2P_AE_SCI_LO_4, CFGP2PSCI_LO_4, reg_p2p_ae_sci_lo_4);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_lif_p2p_ae_sci_hi_4_set(uint32_t cfgp2psci_hi_4)
{
    uint32_t reg_p2p_ae_sci_hi_4=0;

#ifdef VALIDATE_PARMS
#endif

    reg_p2p_ae_sci_hi_4 = RU_FIELD_SET(0, LIF, P2P_AE_SCI_HI_4, CFGP2PSCI_HI_4, reg_p2p_ae_sci_hi_4, cfgp2psci_hi_4);

    RU_REG_WRITE(0, LIF, P2P_AE_SCI_HI_4, reg_p2p_ae_sci_hi_4);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_lif_p2p_ae_sci_hi_4_get(uint32_t *cfgp2psci_hi_4)
{
    uint32_t reg_p2p_ae_sci_hi_4=0;

#ifdef VALIDATE_PARMS
    if(!cfgp2psci_hi_4)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, LIF, P2P_AE_SCI_HI_4, reg_p2p_ae_sci_hi_4);

    *cfgp2psci_hi_4 = RU_FIELD_GET(0, LIF, P2P_AE_SCI_HI_4, CFGP2PSCI_HI_4, reg_p2p_ae_sci_hi_4);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_lif_p2p_ae_sci_lo_5_set(uint32_t cfgp2psci_lo_5)
{
    uint32_t reg_p2p_ae_sci_lo_5=0;

#ifdef VALIDATE_PARMS
#endif

    reg_p2p_ae_sci_lo_5 = RU_FIELD_SET(0, LIF, P2P_AE_SCI_LO_5, CFGP2PSCI_LO_5, reg_p2p_ae_sci_lo_5, cfgp2psci_lo_5);

    RU_REG_WRITE(0, LIF, P2P_AE_SCI_LO_5, reg_p2p_ae_sci_lo_5);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_lif_p2p_ae_sci_lo_5_get(uint32_t *cfgp2psci_lo_5)
{
    uint32_t reg_p2p_ae_sci_lo_5=0;

#ifdef VALIDATE_PARMS
    if(!cfgp2psci_lo_5)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, LIF, P2P_AE_SCI_LO_5, reg_p2p_ae_sci_lo_5);

    *cfgp2psci_lo_5 = RU_FIELD_GET(0, LIF, P2P_AE_SCI_LO_5, CFGP2PSCI_LO_5, reg_p2p_ae_sci_lo_5);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_lif_p2p_ae_sci_hi_5_set(uint32_t cfgp2psci_hi_5)
{
    uint32_t reg_p2p_ae_sci_hi_5=0;

#ifdef VALIDATE_PARMS
#endif

    reg_p2p_ae_sci_hi_5 = RU_FIELD_SET(0, LIF, P2P_AE_SCI_HI_5, CFGP2PSCI_HI_5, reg_p2p_ae_sci_hi_5, cfgp2psci_hi_5);

    RU_REG_WRITE(0, LIF, P2P_AE_SCI_HI_5, reg_p2p_ae_sci_hi_5);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_lif_p2p_ae_sci_hi_5_get(uint32_t *cfgp2psci_hi_5)
{
    uint32_t reg_p2p_ae_sci_hi_5=0;

#ifdef VALIDATE_PARMS
    if(!cfgp2psci_hi_5)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, LIF, P2P_AE_SCI_HI_5, reg_p2p_ae_sci_hi_5);

    *cfgp2psci_hi_5 = RU_FIELD_GET(0, LIF, P2P_AE_SCI_HI_5, CFGP2PSCI_HI_5, reg_p2p_ae_sci_hi_5);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_lif_p2p_ae_sci_lo_6_set(uint32_t cfgp2psci_lo_6)
{
    uint32_t reg_p2p_ae_sci_lo_6=0;

#ifdef VALIDATE_PARMS
#endif

    reg_p2p_ae_sci_lo_6 = RU_FIELD_SET(0, LIF, P2P_AE_SCI_LO_6, CFGP2PSCI_LO_6, reg_p2p_ae_sci_lo_6, cfgp2psci_lo_6);

    RU_REG_WRITE(0, LIF, P2P_AE_SCI_LO_6, reg_p2p_ae_sci_lo_6);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_lif_p2p_ae_sci_lo_6_get(uint32_t *cfgp2psci_lo_6)
{
    uint32_t reg_p2p_ae_sci_lo_6=0;

#ifdef VALIDATE_PARMS
    if(!cfgp2psci_lo_6)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, LIF, P2P_AE_SCI_LO_6, reg_p2p_ae_sci_lo_6);

    *cfgp2psci_lo_6 = RU_FIELD_GET(0, LIF, P2P_AE_SCI_LO_6, CFGP2PSCI_LO_6, reg_p2p_ae_sci_lo_6);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_lif_p2p_ae_sci_hi_6_set(uint32_t cfgp2psci_hi_6)
{
    uint32_t reg_p2p_ae_sci_hi_6=0;

#ifdef VALIDATE_PARMS
#endif

    reg_p2p_ae_sci_hi_6 = RU_FIELD_SET(0, LIF, P2P_AE_SCI_HI_6, CFGP2PSCI_HI_6, reg_p2p_ae_sci_hi_6, cfgp2psci_hi_6);

    RU_REG_WRITE(0, LIF, P2P_AE_SCI_HI_6, reg_p2p_ae_sci_hi_6);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_lif_p2p_ae_sci_hi_6_get(uint32_t *cfgp2psci_hi_6)
{
    uint32_t reg_p2p_ae_sci_hi_6=0;

#ifdef VALIDATE_PARMS
    if(!cfgp2psci_hi_6)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, LIF, P2P_AE_SCI_HI_6, reg_p2p_ae_sci_hi_6);

    *cfgp2psci_hi_6 = RU_FIELD_GET(0, LIF, P2P_AE_SCI_HI_6, CFGP2PSCI_HI_6, reg_p2p_ae_sci_hi_6);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_lif_p2p_ae_sci_lo_7_set(uint32_t cfgp2psci_lo_7)
{
    uint32_t reg_p2p_ae_sci_lo_7=0;

#ifdef VALIDATE_PARMS
#endif

    reg_p2p_ae_sci_lo_7 = RU_FIELD_SET(0, LIF, P2P_AE_SCI_LO_7, CFGP2PSCI_LO_7, reg_p2p_ae_sci_lo_7, cfgp2psci_lo_7);

    RU_REG_WRITE(0, LIF, P2P_AE_SCI_LO_7, reg_p2p_ae_sci_lo_7);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_lif_p2p_ae_sci_lo_7_get(uint32_t *cfgp2psci_lo_7)
{
    uint32_t reg_p2p_ae_sci_lo_7=0;

#ifdef VALIDATE_PARMS
    if(!cfgp2psci_lo_7)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, LIF, P2P_AE_SCI_LO_7, reg_p2p_ae_sci_lo_7);

    *cfgp2psci_lo_7 = RU_FIELD_GET(0, LIF, P2P_AE_SCI_LO_7, CFGP2PSCI_LO_7, reg_p2p_ae_sci_lo_7);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_lif_p2p_ae_sci_hi_7_set(uint32_t cfgp2psci_hi_7)
{
    uint32_t reg_p2p_ae_sci_hi_7=0;

#ifdef VALIDATE_PARMS
#endif

    reg_p2p_ae_sci_hi_7 = RU_FIELD_SET(0, LIF, P2P_AE_SCI_HI_7, CFGP2PSCI_HI_7, reg_p2p_ae_sci_hi_7, cfgp2psci_hi_7);

    RU_REG_WRITE(0, LIF, P2P_AE_SCI_HI_7, reg_p2p_ae_sci_hi_7);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_lif_p2p_ae_sci_hi_7_get(uint32_t *cfgp2psci_hi_7)
{
    uint32_t reg_p2p_ae_sci_hi_7=0;

#ifdef VALIDATE_PARMS
    if(!cfgp2psci_hi_7)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, LIF, P2P_AE_SCI_HI_7, reg_p2p_ae_sci_hi_7);

    *cfgp2psci_hi_7 = RU_FIELD_GET(0, LIF, P2P_AE_SCI_HI_7, CFGP2PSCI_HI_7, reg_p2p_ae_sci_hi_7);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_lif_p2p_ae_sci_lo_8_set(uint32_t cfgp2psci_lo_8)
{
    uint32_t reg_p2p_ae_sci_lo_8=0;

#ifdef VALIDATE_PARMS
#endif

    reg_p2p_ae_sci_lo_8 = RU_FIELD_SET(0, LIF, P2P_AE_SCI_LO_8, CFGP2PSCI_LO_8, reg_p2p_ae_sci_lo_8, cfgp2psci_lo_8);

    RU_REG_WRITE(0, LIF, P2P_AE_SCI_LO_8, reg_p2p_ae_sci_lo_8);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_lif_p2p_ae_sci_lo_8_get(uint32_t *cfgp2psci_lo_8)
{
    uint32_t reg_p2p_ae_sci_lo_8=0;

#ifdef VALIDATE_PARMS
    if(!cfgp2psci_lo_8)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, LIF, P2P_AE_SCI_LO_8, reg_p2p_ae_sci_lo_8);

    *cfgp2psci_lo_8 = RU_FIELD_GET(0, LIF, P2P_AE_SCI_LO_8, CFGP2PSCI_LO_8, reg_p2p_ae_sci_lo_8);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_lif_p2p_ae_sci_hi_8_set(uint32_t cfgp2psci_hi_8)
{
    uint32_t reg_p2p_ae_sci_hi_8=0;

#ifdef VALIDATE_PARMS
#endif

    reg_p2p_ae_sci_hi_8 = RU_FIELD_SET(0, LIF, P2P_AE_SCI_HI_8, CFGP2PSCI_HI_8, reg_p2p_ae_sci_hi_8, cfgp2psci_hi_8);

    RU_REG_WRITE(0, LIF, P2P_AE_SCI_HI_8, reg_p2p_ae_sci_hi_8);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_lif_p2p_ae_sci_hi_8_get(uint32_t *cfgp2psci_hi_8)
{
    uint32_t reg_p2p_ae_sci_hi_8=0;

#ifdef VALIDATE_PARMS
    if(!cfgp2psci_hi_8)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, LIF, P2P_AE_SCI_HI_8, reg_p2p_ae_sci_hi_8);

    *cfgp2psci_hi_8 = RU_FIELD_GET(0, LIF, P2P_AE_SCI_HI_8, CFGP2PSCI_HI_8, reg_p2p_ae_sci_hi_8);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_lif_p2p_ae_sci_lo_9_set(uint32_t cfgp2psci_lo_9)
{
    uint32_t reg_p2p_ae_sci_lo_9=0;

#ifdef VALIDATE_PARMS
#endif

    reg_p2p_ae_sci_lo_9 = RU_FIELD_SET(0, LIF, P2P_AE_SCI_LO_9, CFGP2PSCI_LO_9, reg_p2p_ae_sci_lo_9, cfgp2psci_lo_9);

    RU_REG_WRITE(0, LIF, P2P_AE_SCI_LO_9, reg_p2p_ae_sci_lo_9);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_lif_p2p_ae_sci_lo_9_get(uint32_t *cfgp2psci_lo_9)
{
    uint32_t reg_p2p_ae_sci_lo_9=0;

#ifdef VALIDATE_PARMS
    if(!cfgp2psci_lo_9)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, LIF, P2P_AE_SCI_LO_9, reg_p2p_ae_sci_lo_9);

    *cfgp2psci_lo_9 = RU_FIELD_GET(0, LIF, P2P_AE_SCI_LO_9, CFGP2PSCI_LO_9, reg_p2p_ae_sci_lo_9);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_lif_p2p_ae_sci_hi_9_set(uint32_t cfgp2psci_hi_9)
{
    uint32_t reg_p2p_ae_sci_hi_9=0;

#ifdef VALIDATE_PARMS
#endif

    reg_p2p_ae_sci_hi_9 = RU_FIELD_SET(0, LIF, P2P_AE_SCI_HI_9, CFGP2PSCI_HI_9, reg_p2p_ae_sci_hi_9, cfgp2psci_hi_9);

    RU_REG_WRITE(0, LIF, P2P_AE_SCI_HI_9, reg_p2p_ae_sci_hi_9);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_lif_p2p_ae_sci_hi_9_get(uint32_t *cfgp2psci_hi_9)
{
    uint32_t reg_p2p_ae_sci_hi_9=0;

#ifdef VALIDATE_PARMS
    if(!cfgp2psci_hi_9)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, LIF, P2P_AE_SCI_HI_9, reg_p2p_ae_sci_hi_9);

    *cfgp2psci_hi_9 = RU_FIELD_GET(0, LIF, P2P_AE_SCI_HI_9, CFGP2PSCI_HI_9, reg_p2p_ae_sci_hi_9);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_lif_p2p_ae_sci_lo_10_set(uint32_t cfgp2psci_lo_10)
{
    uint32_t reg_p2p_ae_sci_lo_10=0;

#ifdef VALIDATE_PARMS
#endif

    reg_p2p_ae_sci_lo_10 = RU_FIELD_SET(0, LIF, P2P_AE_SCI_LO_10, CFGP2PSCI_LO_10, reg_p2p_ae_sci_lo_10, cfgp2psci_lo_10);

    RU_REG_WRITE(0, LIF, P2P_AE_SCI_LO_10, reg_p2p_ae_sci_lo_10);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_lif_p2p_ae_sci_lo_10_get(uint32_t *cfgp2psci_lo_10)
{
    uint32_t reg_p2p_ae_sci_lo_10=0;

#ifdef VALIDATE_PARMS
    if(!cfgp2psci_lo_10)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, LIF, P2P_AE_SCI_LO_10, reg_p2p_ae_sci_lo_10);

    *cfgp2psci_lo_10 = RU_FIELD_GET(0, LIF, P2P_AE_SCI_LO_10, CFGP2PSCI_LO_10, reg_p2p_ae_sci_lo_10);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_lif_p2p_ae_sci_hi_10_set(uint32_t cfgp2psci_hi_10)
{
    uint32_t reg_p2p_ae_sci_hi_10=0;

#ifdef VALIDATE_PARMS
#endif

    reg_p2p_ae_sci_hi_10 = RU_FIELD_SET(0, LIF, P2P_AE_SCI_HI_10, CFGP2PSCI_HI_10, reg_p2p_ae_sci_hi_10, cfgp2psci_hi_10);

    RU_REG_WRITE(0, LIF, P2P_AE_SCI_HI_10, reg_p2p_ae_sci_hi_10);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_lif_p2p_ae_sci_hi_10_get(uint32_t *cfgp2psci_hi_10)
{
    uint32_t reg_p2p_ae_sci_hi_10=0;

#ifdef VALIDATE_PARMS
    if(!cfgp2psci_hi_10)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, LIF, P2P_AE_SCI_HI_10, reg_p2p_ae_sci_hi_10);

    *cfgp2psci_hi_10 = RU_FIELD_GET(0, LIF, P2P_AE_SCI_HI_10, CFGP2PSCI_HI_10, reg_p2p_ae_sci_hi_10);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_lif_p2p_ae_sci_lo_11_set(uint32_t cfgp2psci_lo_11)
{
    uint32_t reg_p2p_ae_sci_lo_11=0;

#ifdef VALIDATE_PARMS
#endif

    reg_p2p_ae_sci_lo_11 = RU_FIELD_SET(0, LIF, P2P_AE_SCI_LO_11, CFGP2PSCI_LO_11, reg_p2p_ae_sci_lo_11, cfgp2psci_lo_11);

    RU_REG_WRITE(0, LIF, P2P_AE_SCI_LO_11, reg_p2p_ae_sci_lo_11);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_lif_p2p_ae_sci_lo_11_get(uint32_t *cfgp2psci_lo_11)
{
    uint32_t reg_p2p_ae_sci_lo_11=0;

#ifdef VALIDATE_PARMS
    if(!cfgp2psci_lo_11)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, LIF, P2P_AE_SCI_LO_11, reg_p2p_ae_sci_lo_11);

    *cfgp2psci_lo_11 = RU_FIELD_GET(0, LIF, P2P_AE_SCI_LO_11, CFGP2PSCI_LO_11, reg_p2p_ae_sci_lo_11);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_lif_p2p_ae_sci_hi_11_set(uint32_t cfgp2psci_hi_11)
{
    uint32_t reg_p2p_ae_sci_hi_11=0;

#ifdef VALIDATE_PARMS
#endif

    reg_p2p_ae_sci_hi_11 = RU_FIELD_SET(0, LIF, P2P_AE_SCI_HI_11, CFGP2PSCI_HI_11, reg_p2p_ae_sci_hi_11, cfgp2psci_hi_11);

    RU_REG_WRITE(0, LIF, P2P_AE_SCI_HI_11, reg_p2p_ae_sci_hi_11);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_lif_p2p_ae_sci_hi_11_get(uint32_t *cfgp2psci_hi_11)
{
    uint32_t reg_p2p_ae_sci_hi_11=0;

#ifdef VALIDATE_PARMS
    if(!cfgp2psci_hi_11)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, LIF, P2P_AE_SCI_HI_11, reg_p2p_ae_sci_hi_11);

    *cfgp2psci_hi_11 = RU_FIELD_GET(0, LIF, P2P_AE_SCI_HI_11, CFGP2PSCI_HI_11, reg_p2p_ae_sci_hi_11);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_lif_p2p_ae_sci_lo_12_set(uint32_t cfgp2psci_lo_12)
{
    uint32_t reg_p2p_ae_sci_lo_12=0;

#ifdef VALIDATE_PARMS
#endif

    reg_p2p_ae_sci_lo_12 = RU_FIELD_SET(0, LIF, P2P_AE_SCI_LO_12, CFGP2PSCI_LO_12, reg_p2p_ae_sci_lo_12, cfgp2psci_lo_12);

    RU_REG_WRITE(0, LIF, P2P_AE_SCI_LO_12, reg_p2p_ae_sci_lo_12);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_lif_p2p_ae_sci_lo_12_get(uint32_t *cfgp2psci_lo_12)
{
    uint32_t reg_p2p_ae_sci_lo_12=0;

#ifdef VALIDATE_PARMS
    if(!cfgp2psci_lo_12)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, LIF, P2P_AE_SCI_LO_12, reg_p2p_ae_sci_lo_12);

    *cfgp2psci_lo_12 = RU_FIELD_GET(0, LIF, P2P_AE_SCI_LO_12, CFGP2PSCI_LO_12, reg_p2p_ae_sci_lo_12);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_lif_p2p_ae_sci_hi_12_set(uint32_t cfgp2psci_hi_12)
{
    uint32_t reg_p2p_ae_sci_hi_12=0;

#ifdef VALIDATE_PARMS
#endif

    reg_p2p_ae_sci_hi_12 = RU_FIELD_SET(0, LIF, P2P_AE_SCI_HI_12, CFGP2PSCI_HI_12, reg_p2p_ae_sci_hi_12, cfgp2psci_hi_12);

    RU_REG_WRITE(0, LIF, P2P_AE_SCI_HI_12, reg_p2p_ae_sci_hi_12);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_lif_p2p_ae_sci_hi_12_get(uint32_t *cfgp2psci_hi_12)
{
    uint32_t reg_p2p_ae_sci_hi_12=0;

#ifdef VALIDATE_PARMS
    if(!cfgp2psci_hi_12)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, LIF, P2P_AE_SCI_HI_12, reg_p2p_ae_sci_hi_12);

    *cfgp2psci_hi_12 = RU_FIELD_GET(0, LIF, P2P_AE_SCI_HI_12, CFGP2PSCI_HI_12, reg_p2p_ae_sci_hi_12);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_lif_p2p_ae_sci_lo_13_set(uint32_t cfgp2psci_lo_13)
{
    uint32_t reg_p2p_ae_sci_lo_13=0;

#ifdef VALIDATE_PARMS
#endif

    reg_p2p_ae_sci_lo_13 = RU_FIELD_SET(0, LIF, P2P_AE_SCI_LO_13, CFGP2PSCI_LO_13, reg_p2p_ae_sci_lo_13, cfgp2psci_lo_13);

    RU_REG_WRITE(0, LIF, P2P_AE_SCI_LO_13, reg_p2p_ae_sci_lo_13);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_lif_p2p_ae_sci_lo_13_get(uint32_t *cfgp2psci_lo_13)
{
    uint32_t reg_p2p_ae_sci_lo_13=0;

#ifdef VALIDATE_PARMS
    if(!cfgp2psci_lo_13)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, LIF, P2P_AE_SCI_LO_13, reg_p2p_ae_sci_lo_13);

    *cfgp2psci_lo_13 = RU_FIELD_GET(0, LIF, P2P_AE_SCI_LO_13, CFGP2PSCI_LO_13, reg_p2p_ae_sci_lo_13);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_lif_p2p_ae_sci_hi_13_set(uint32_t cfgp2psci_hi_13)
{
    uint32_t reg_p2p_ae_sci_hi_13=0;

#ifdef VALIDATE_PARMS
#endif

    reg_p2p_ae_sci_hi_13 = RU_FIELD_SET(0, LIF, P2P_AE_SCI_HI_13, CFGP2PSCI_HI_13, reg_p2p_ae_sci_hi_13, cfgp2psci_hi_13);

    RU_REG_WRITE(0, LIF, P2P_AE_SCI_HI_13, reg_p2p_ae_sci_hi_13);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_lif_p2p_ae_sci_hi_13_get(uint32_t *cfgp2psci_hi_13)
{
    uint32_t reg_p2p_ae_sci_hi_13=0;

#ifdef VALIDATE_PARMS
    if(!cfgp2psci_hi_13)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, LIF, P2P_AE_SCI_HI_13, reg_p2p_ae_sci_hi_13);

    *cfgp2psci_hi_13 = RU_FIELD_GET(0, LIF, P2P_AE_SCI_HI_13, CFGP2PSCI_HI_13, reg_p2p_ae_sci_hi_13);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_lif_p2p_ae_sci_lo_14_set(uint32_t cfgp2psci_lo_14)
{
    uint32_t reg_p2p_ae_sci_lo_14=0;

#ifdef VALIDATE_PARMS
#endif

    reg_p2p_ae_sci_lo_14 = RU_FIELD_SET(0, LIF, P2P_AE_SCI_LO_14, CFGP2PSCI_LO_14, reg_p2p_ae_sci_lo_14, cfgp2psci_lo_14);

    RU_REG_WRITE(0, LIF, P2P_AE_SCI_LO_14, reg_p2p_ae_sci_lo_14);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_lif_p2p_ae_sci_lo_14_get(uint32_t *cfgp2psci_lo_14)
{
    uint32_t reg_p2p_ae_sci_lo_14=0;

#ifdef VALIDATE_PARMS
    if(!cfgp2psci_lo_14)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, LIF, P2P_AE_SCI_LO_14, reg_p2p_ae_sci_lo_14);

    *cfgp2psci_lo_14 = RU_FIELD_GET(0, LIF, P2P_AE_SCI_LO_14, CFGP2PSCI_LO_14, reg_p2p_ae_sci_lo_14);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_lif_p2p_ae_sci_hi_14_set(uint32_t cfgp2psci_hi_14)
{
    uint32_t reg_p2p_ae_sci_hi_14=0;

#ifdef VALIDATE_PARMS
#endif

    reg_p2p_ae_sci_hi_14 = RU_FIELD_SET(0, LIF, P2P_AE_SCI_HI_14, CFGP2PSCI_HI_14, reg_p2p_ae_sci_hi_14, cfgp2psci_hi_14);

    RU_REG_WRITE(0, LIF, P2P_AE_SCI_HI_14, reg_p2p_ae_sci_hi_14);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_lif_p2p_ae_sci_hi_14_get(uint32_t *cfgp2psci_hi_14)
{
    uint32_t reg_p2p_ae_sci_hi_14=0;

#ifdef VALIDATE_PARMS
    if(!cfgp2psci_hi_14)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, LIF, P2P_AE_SCI_HI_14, reg_p2p_ae_sci_hi_14);

    *cfgp2psci_hi_14 = RU_FIELD_GET(0, LIF, P2P_AE_SCI_HI_14, CFGP2PSCI_HI_14, reg_p2p_ae_sci_hi_14);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_lif_p2p_ae_sci_lo_15_set(uint32_t cfgp2psci_lo_15)
{
    uint32_t reg_p2p_ae_sci_lo_15=0;

#ifdef VALIDATE_PARMS
#endif

    reg_p2p_ae_sci_lo_15 = RU_FIELD_SET(0, LIF, P2P_AE_SCI_LO_15, CFGP2PSCI_LO_15, reg_p2p_ae_sci_lo_15, cfgp2psci_lo_15);

    RU_REG_WRITE(0, LIF, P2P_AE_SCI_LO_15, reg_p2p_ae_sci_lo_15);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_lif_p2p_ae_sci_lo_15_get(uint32_t *cfgp2psci_lo_15)
{
    uint32_t reg_p2p_ae_sci_lo_15=0;

#ifdef VALIDATE_PARMS
    if(!cfgp2psci_lo_15)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, LIF, P2P_AE_SCI_LO_15, reg_p2p_ae_sci_lo_15);

    *cfgp2psci_lo_15 = RU_FIELD_GET(0, LIF, P2P_AE_SCI_LO_15, CFGP2PSCI_LO_15, reg_p2p_ae_sci_lo_15);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_lif_p2p_ae_sci_hi_15_set(uint32_t cfgp2psci_hi_15)
{
    uint32_t reg_p2p_ae_sci_hi_15=0;

#ifdef VALIDATE_PARMS
#endif

    reg_p2p_ae_sci_hi_15 = RU_FIELD_SET(0, LIF, P2P_AE_SCI_HI_15, CFGP2PSCI_HI_15, reg_p2p_ae_sci_hi_15, cfgp2psci_hi_15);

    RU_REG_WRITE(0, LIF, P2P_AE_SCI_HI_15, reg_p2p_ae_sci_hi_15);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_lif_p2p_ae_sci_hi_15_get(uint32_t *cfgp2psci_hi_15)
{
    uint32_t reg_p2p_ae_sci_hi_15=0;

#ifdef VALIDATE_PARMS
    if(!cfgp2psci_hi_15)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, LIF, P2P_AE_SCI_HI_15, reg_p2p_ae_sci_hi_15);

    *cfgp2psci_hi_15 = RU_FIELD_GET(0, LIF, P2P_AE_SCI_HI_15, CFGP2PSCI_HI_15, reg_p2p_ae_sci_hi_15);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_lif_sec_up_key_stat_1_get(uint32_t *keyupsel_hi)
{
    uint32_t reg_sec_up_key_stat_1=0;

#ifdef VALIDATE_PARMS
    if(!keyupsel_hi)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, LIF, SEC_UP_KEY_STAT_1, reg_sec_up_key_stat_1);

    *keyupsel_hi = RU_FIELD_GET(0, LIF, SEC_UP_KEY_STAT_1, KEYUPSEL_HI, reg_sec_up_key_stat_1);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_lif_sec_key_sel_1_get(uint32_t *keysel_hi)
{
    uint32_t reg_sec_key_sel_1=0;

#ifdef VALIDATE_PARMS
    if(!keysel_hi)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, LIF, SEC_KEY_SEL_1, reg_sec_key_sel_1);

    *keysel_hi = RU_FIELD_GET(0, LIF, SEC_KEY_SEL_1, KEYSEL_HI, reg_sec_key_sel_1);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_lif_pon_sec_tx_plaintxt_ae_pad_control_set(uint8_t cf_plaintxt_oh_b_idle_pad, uint8_t cf_plaintxt_oh_a_idle_pad)
{
    uint32_t reg_pon_sec_tx_plaintxt_ae_pad_control=0;

#ifdef VALIDATE_PARMS
    if((cf_plaintxt_oh_b_idle_pad >= _6BITS_MAX_VAL_) ||
       (cf_plaintxt_oh_a_idle_pad >= _6BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_pon_sec_tx_plaintxt_ae_pad_control = RU_FIELD_SET(0, LIF, PON_SEC_TX_PLAINTXT_AE_PAD_CONTROL, CF_PLAINTXT_OH_B_IDLE_PAD, reg_pon_sec_tx_plaintxt_ae_pad_control, cf_plaintxt_oh_b_idle_pad);
    reg_pon_sec_tx_plaintxt_ae_pad_control = RU_FIELD_SET(0, LIF, PON_SEC_TX_PLAINTXT_AE_PAD_CONTROL, CF_PLAINTXT_OH_A_IDLE_PAD, reg_pon_sec_tx_plaintxt_ae_pad_control, cf_plaintxt_oh_a_idle_pad);

    RU_REG_WRITE(0, LIF, PON_SEC_TX_PLAINTXT_AE_PAD_CONTROL, reg_pon_sec_tx_plaintxt_ae_pad_control);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_lif_pon_sec_tx_plaintxt_ae_pad_control_get(uint8_t *cf_plaintxt_oh_b_idle_pad, uint8_t *cf_plaintxt_oh_a_idle_pad)
{
    uint32_t reg_pon_sec_tx_plaintxt_ae_pad_control=0;

#ifdef VALIDATE_PARMS
    if(!cf_plaintxt_oh_b_idle_pad || !cf_plaintxt_oh_a_idle_pad)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, LIF, PON_SEC_TX_PLAINTXT_AE_PAD_CONTROL, reg_pon_sec_tx_plaintxt_ae_pad_control);

    *cf_plaintxt_oh_b_idle_pad = RU_FIELD_GET(0, LIF, PON_SEC_TX_PLAINTXT_AE_PAD_CONTROL, CF_PLAINTXT_OH_B_IDLE_PAD, reg_pon_sec_tx_plaintxt_ae_pad_control);
    *cf_plaintxt_oh_a_idle_pad = RU_FIELD_GET(0, LIF, PON_SEC_TX_PLAINTXT_AE_PAD_CONTROL, CF_PLAINTXT_OH_A_IDLE_PAD, reg_pon_sec_tx_plaintxt_ae_pad_control);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_lif_p2p_autoneg_control_set(uint16_t cf_autoneg_linktimer, bdmf_boolean cf_autoneg_mode_sel, bdmf_boolean cf_autoneg_restart, bdmf_boolean cf_autoneg_en)
{
    uint32_t reg_p2p_autoneg_control=0;

#ifdef VALIDATE_PARMS
    if((cf_autoneg_mode_sel >= _1BITS_MAX_VAL_) ||
       (cf_autoneg_restart >= _1BITS_MAX_VAL_) ||
       (cf_autoneg_en >= _1BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_p2p_autoneg_control = RU_FIELD_SET(0, LIF, P2P_AUTONEG_CONTROL, CF_AUTONEG_LINKTIMER, reg_p2p_autoneg_control, cf_autoneg_linktimer);
    reg_p2p_autoneg_control = RU_FIELD_SET(0, LIF, P2P_AUTONEG_CONTROL, CF_AUTONEG_MODE_SEL, reg_p2p_autoneg_control, cf_autoneg_mode_sel);
    reg_p2p_autoneg_control = RU_FIELD_SET(0, LIF, P2P_AUTONEG_CONTROL, CF_AUTONEG_RESTART, reg_p2p_autoneg_control, cf_autoneg_restart);
    reg_p2p_autoneg_control = RU_FIELD_SET(0, LIF, P2P_AUTONEG_CONTROL, CF_AUTONEG_EN, reg_p2p_autoneg_control, cf_autoneg_en);

    RU_REG_WRITE(0, LIF, P2P_AUTONEG_CONTROL, reg_p2p_autoneg_control);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_lif_p2p_autoneg_control_get(uint16_t *cf_autoneg_linktimer, bdmf_boolean *cf_autoneg_mode_sel, bdmf_boolean *cf_autoneg_restart, bdmf_boolean *cf_autoneg_en)
{
    uint32_t reg_p2p_autoneg_control=0;

#ifdef VALIDATE_PARMS
    if(!cf_autoneg_linktimer || !cf_autoneg_mode_sel || !cf_autoneg_restart || !cf_autoneg_en)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, LIF, P2P_AUTONEG_CONTROL, reg_p2p_autoneg_control);

    *cf_autoneg_linktimer = RU_FIELD_GET(0, LIF, P2P_AUTONEG_CONTROL, CF_AUTONEG_LINKTIMER, reg_p2p_autoneg_control);
    *cf_autoneg_mode_sel = RU_FIELD_GET(0, LIF, P2P_AUTONEG_CONTROL, CF_AUTONEG_MODE_SEL, reg_p2p_autoneg_control);
    *cf_autoneg_restart = RU_FIELD_GET(0, LIF, P2P_AUTONEG_CONTROL, CF_AUTONEG_RESTART, reg_p2p_autoneg_control);
    *cf_autoneg_en = RU_FIELD_GET(0, LIF, P2P_AUTONEG_CONTROL, CF_AUTONEG_EN, reg_p2p_autoneg_control);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_lif_p2p_autoneg_status_get(bdmf_boolean *an_lp_remote_fault, bdmf_boolean *an_sync_status, bdmf_boolean *an_complete)
{
    uint32_t reg_p2p_autoneg_status=0;

#ifdef VALIDATE_PARMS
    if(!an_lp_remote_fault || !an_sync_status || !an_complete)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, LIF, P2P_AUTONEG_STATUS, reg_p2p_autoneg_status);

    *an_lp_remote_fault = RU_FIELD_GET(0, LIF, P2P_AUTONEG_STATUS, AN_LP_REMOTE_FAULT, reg_p2p_autoneg_status);
    *an_sync_status = RU_FIELD_GET(0, LIF, P2P_AUTONEG_STATUS, AN_SYNC_STATUS, reg_p2p_autoneg_status);
    *an_complete = RU_FIELD_GET(0, LIF, P2P_AUTONEG_STATUS, AN_COMPLETE, reg_p2p_autoneg_status);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_lif_p2p_autoneg_ability_config_reg_set(uint16_t cf_lif_p2p_ae_autoneg_config_ability)
{
    uint32_t reg_p2p_autoneg_ability_config_reg=0;

#ifdef VALIDATE_PARMS
#endif

    reg_p2p_autoneg_ability_config_reg = RU_FIELD_SET(0, LIF, P2P_AUTONEG_ABILITY_CONFIG_REG, CF_LIF_P2P_AE_AUTONEG_CONFIG_ABILITY, reg_p2p_autoneg_ability_config_reg, cf_lif_p2p_ae_autoneg_config_ability);

    RU_REG_WRITE(0, LIF, P2P_AUTONEG_ABILITY_CONFIG_REG, reg_p2p_autoneg_ability_config_reg);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_lif_p2p_autoneg_ability_config_reg_get(uint16_t *cf_lif_p2p_ae_autoneg_config_ability)
{
    uint32_t reg_p2p_autoneg_ability_config_reg=0;

#ifdef VALIDATE_PARMS
    if(!cf_lif_p2p_ae_autoneg_config_ability)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, LIF, P2P_AUTONEG_ABILITY_CONFIG_REG, reg_p2p_autoneg_ability_config_reg);

    *cf_lif_p2p_ae_autoneg_config_ability = RU_FIELD_GET(0, LIF, P2P_AUTONEG_ABILITY_CONFIG_REG, CF_LIF_P2P_AE_AUTONEG_CONFIG_ABILITY, reg_p2p_autoneg_ability_config_reg);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_lif_p2p_autoneg_link_partner_ability_config_read_get(uint16_t *cf_lif_p2p_ae_autoneg_lp_ability_read)
{
    uint32_t reg_p2p_autoneg_link_partner_ability_config_read=0;

#ifdef VALIDATE_PARMS
    if(!cf_lif_p2p_ae_autoneg_lp_ability_read)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, LIF, P2P_AUTONEG_LINK_PARTNER_ABILITY_CONFIG_READ, reg_p2p_autoneg_link_partner_ability_config_read);

    *cf_lif_p2p_ae_autoneg_lp_ability_read = RU_FIELD_GET(0, LIF, P2P_AUTONEG_LINK_PARTNER_ABILITY_CONFIG_READ, CF_LIF_P2P_AE_AUTONEG_LP_ABILITY_READ, reg_p2p_autoneg_link_partner_ability_config_read);

    return BDMF_ERR_OK;
}

#ifdef USE_BDMF_SHELL
enum
{
    BDMF_pon_control,
    BDMF_pon_inter_op_control,
    BDMF_fec_control,
    BDMF_sec_control,
    BDMF_macsec,
    BDMF_int_status,
    BDMF_int_mask,
    BDMF_data_port_command,
    BDMF_data_port_data,
    BDMF_llid_0,
    BDMF_llid_1,
    BDMF_llid_2,
    BDMF_llid_3,
    BDMF_llid_4,
    BDMF_llid_5,
    BDMF_llid_6,
    BDMF_llid_7,
    BDMF_llid_16,
    BDMF_llid_17,
    BDMF_llid_18,
    BDMF_llid_19,
    BDMF_llid_20,
    BDMF_llid_21,
    BDMF_llid_22,
    BDMF_llid_23,
    BDMF_time_ref_cnt,
    BDMF_timestamp_upd_per,
    BDMF_tp_time,
    BDMF_mpcp_time,
    BDMF_maxlen_ctr,
    BDMF_laser_on_delta,
    BDMF_laser_off_idle,
    BDMF_fec_init_idle,
    BDMF_fec_err_allow,
    BDMF_sec_key_sel,
    BDMF_dn_encrypt_stat,
    BDMF_sec_up_key_stat,
    BDMF_sec_up_encrypt_stat,
    BDMF_sec_up_mpcp_offset,
    BDMF_fec_per_llid,
    BDMF_rx_line_code_err_cnt,
    BDMF_rx_agg_mpcp_frm,
    BDMF_rx_agg_good_frm,
    BDMF_rx_agg_good_byte,
    BDMF_rx_agg_undersz_frm,
    BDMF_rx_agg_oversz_frm,
    BDMF_rx_agg_crc8_frm,
    BDMF_rx_agg_fec_frm,
    BDMF_rx_agg_fec_byte,
    BDMF_rx_agg_fec_exc_err_frm,
    BDMF_rx_agg_nonfec_good_frm,
    BDMF_rx_agg_nonfec_good_byte,
    BDMF_rx_agg_err_bytes,
    BDMF_rx_agg_err_zeroes,
    BDMF_rx_agg_no_err_blks,
    BDMF_rx_agg_cor_blks,
    BDMF_rx_agg_uncor_blks,
    BDMF_rx_agg_err_ones,
    BDMF_rx_agg_err_frm,
    BDMF_tx_pkt_cnt,
    BDMF_tx_byte_cnt,
    BDMF_tx_non_fec_pkt_cnt,
    BDMF_tx_non_fec_byte_cnt,
    BDMF_tx_fec_pkt_cnt,
    BDMF_tx_fec_byte_cnt,
    BDMF_tx_fec_blk_cnt,
    BDMF_tx_mpcp_pkt_cnt,
    BDMF_debug_tx_data_pkt_cnt,
    BDMF_fec_llid_status,
    BDMF_sec_rx_tek_ig_iv_llid,
    BDMF_pon_ber_interv_thresh,
    BDMF_lsr_mon_a_ctrl,
    BDMF_lsr_mon_a_max_thr,
    BDMF_lsr_mon_a_bst_len,
    BDMF_lsr_mon_a_bst_cnt,
    BDMF_debug_pon_sm,
    BDMF_debug_fec_sm,
    BDMF_ae_pktnum_window,
    BDMF_ae_pktnum_thresh,
    BDMF_ae_pktnum_stat,
    BDMF_llid_8,
    BDMF_llid_9,
    BDMF_llid_10,
    BDMF_llid_11,
    BDMF_llid_12,
    BDMF_llid_13,
    BDMF_llid_14,
    BDMF_llid_15,
    BDMF_llid_24,
    BDMF_llid_25,
    BDMF_llid_26,
    BDMF_llid_27,
    BDMF_llid_28,
    BDMF_llid_29,
    BDMF_llid_30,
    BDMF_llid_31,
    BDMF_vlan_type,
    BDMF_p2p_ae_sci_en,
    BDMF_p2p_ae_sci_lo_0,
    BDMF_p2p_ae_sci_hi_0,
    BDMF_p2p_ae_sci_lo_1,
    BDMF_p2p_ae_sci_hi_1,
    BDMF_p2p_ae_sci_lo_2,
    BDMF_p2p_ae_sci_hi_2,
    BDMF_p2p_ae_sci_lo_3,
    BDMF_p2p_ae_sci_hi_3,
    BDMF_p2p_ae_sci_lo_4,
    BDMF_p2p_ae_sci_hi_4,
    BDMF_p2p_ae_sci_lo_5,
    BDMF_p2p_ae_sci_hi_5,
    BDMF_p2p_ae_sci_lo_6,
    BDMF_p2p_ae_sci_hi_6,
    BDMF_p2p_ae_sci_lo_7,
    BDMF_p2p_ae_sci_hi_7,
    BDMF_p2p_ae_sci_lo_8,
    BDMF_p2p_ae_sci_hi_8,
    BDMF_p2p_ae_sci_lo_9,
    BDMF_p2p_ae_sci_hi_9,
    BDMF_p2p_ae_sci_lo_10,
    BDMF_p2p_ae_sci_hi_10,
    BDMF_p2p_ae_sci_lo_11,
    BDMF_p2p_ae_sci_hi_11,
    BDMF_p2p_ae_sci_lo_12,
    BDMF_p2p_ae_sci_hi_12,
    BDMF_p2p_ae_sci_lo_13,
    BDMF_p2p_ae_sci_hi_13,
    BDMF_p2p_ae_sci_lo_14,
    BDMF_p2p_ae_sci_hi_14,
    BDMF_p2p_ae_sci_lo_15,
    BDMF_p2p_ae_sci_hi_15,
    BDMF_sec_up_key_stat_1,
    BDMF_sec_key_sel_1,
    BDMF_pon_sec_tx_plaintxt_ae_pad_control,
    BDMF_p2p_autoneg_control,
    BDMF_p2p_autoneg_status,
    BDMF_p2p_autoneg_ability_config_reg,
    BDMF_p2p_autoneg_link_partner_ability_config_read,
};

typedef enum
{
    bdmf_address_pon_control,
    bdmf_address_pon_inter_op_control,
    bdmf_address_fec_control,
    bdmf_address_sec_control,
    bdmf_address_macsec,
    bdmf_address_int_status,
    bdmf_address_int_mask,
    bdmf_address_data_port_command,
    bdmf_address_data_port_data,
    bdmf_address_llid_0,
    bdmf_address_llid_1,
    bdmf_address_llid_2,
    bdmf_address_llid_3,
    bdmf_address_llid_4,
    bdmf_address_llid_5,
    bdmf_address_llid_6,
    bdmf_address_llid_7,
    bdmf_address_llid_16,
    bdmf_address_llid_17,
    bdmf_address_llid_18,
    bdmf_address_llid_19,
    bdmf_address_llid_20,
    bdmf_address_llid_21,
    bdmf_address_llid_22,
    bdmf_address_llid_23,
    bdmf_address_time_ref_cnt,
    bdmf_address_timestamp_upd_per,
    bdmf_address_tp_time,
    bdmf_address_mpcp_time,
    bdmf_address_maxlen_ctr,
    bdmf_address_laser_on_delta,
    bdmf_address_laser_off_idle,
    bdmf_address_fec_init_idle,
    bdmf_address_fec_err_allow,
    bdmf_address_sec_key_sel,
    bdmf_address_dn_encrypt_stat,
    bdmf_address_sec_up_key_stat,
    bdmf_address_sec_up_encrypt_stat,
    bdmf_address_sec_up_mpcp_offset,
    bdmf_address_fec_per_llid,
    bdmf_address_rx_line_code_err_cnt,
    bdmf_address_rx_agg_mpcp_frm,
    bdmf_address_rx_agg_good_frm,
    bdmf_address_rx_agg_good_byte,
    bdmf_address_rx_agg_undersz_frm,
    bdmf_address_rx_agg_oversz_frm,
    bdmf_address_rx_agg_crc8_frm,
    bdmf_address_rx_agg_fec_frm,
    bdmf_address_rx_agg_fec_byte,
    bdmf_address_rx_agg_fec_exc_err_frm,
    bdmf_address_rx_agg_nonfec_good_frm,
    bdmf_address_rx_agg_nonfec_good_byte,
    bdmf_address_rx_agg_err_bytes,
    bdmf_address_rx_agg_err_zeroes,
    bdmf_address_rx_agg_no_err_blks,
    bdmf_address_rx_agg_cor_blks,
    bdmf_address_rx_agg_uncor_blks,
    bdmf_address_rx_agg_err_ones,
    bdmf_address_rx_agg_err_frm,
    bdmf_address_tx_pkt_cnt,
    bdmf_address_tx_byte_cnt,
    bdmf_address_tx_non_fec_pkt_cnt,
    bdmf_address_tx_non_fec_byte_cnt,
    bdmf_address_tx_fec_pkt_cnt,
    bdmf_address_tx_fec_byte_cnt,
    bdmf_address_tx_fec_blk_cnt,
    bdmf_address_tx_mpcp_pkt_cnt,
    bdmf_address_debug_tx_data_pkt_cnt,
    bdmf_address_fec_llid_status,
    bdmf_address_sec_rx_tek_ig_iv_llid,
    bdmf_address_pon_ber_interv_thresh,
    bdmf_address_lsr_mon_a_ctrl,
    bdmf_address_lsr_mon_a_max_thr,
    bdmf_address_lsr_mon_a_bst_len,
    bdmf_address_lsr_mon_a_bst_cnt,
    bdmf_address_debug_pon_sm,
    bdmf_address_debug_fec_sm,
    bdmf_address_ae_pktnum_window,
    bdmf_address_ae_pktnum_thresh,
    bdmf_address_ae_pktnum_stat,
    bdmf_address_llid_8,
    bdmf_address_llid_9,
    bdmf_address_llid_10,
    bdmf_address_llid_11,
    bdmf_address_llid_12,
    bdmf_address_llid_13,
    bdmf_address_llid_14,
    bdmf_address_llid_15,
    bdmf_address_llid_24,
    bdmf_address_llid_25,
    bdmf_address_llid_26,
    bdmf_address_llid_27,
    bdmf_address_llid_28,
    bdmf_address_llid_29,
    bdmf_address_llid_30,
    bdmf_address_llid_31,
    bdmf_address_vlan_type,
    bdmf_address_p2p_ae_sci_en,
    bdmf_address_p2p_ae_sci_lo_0,
    bdmf_address_p2p_ae_sci_hi_0,
    bdmf_address_p2p_ae_sci_lo_1,
    bdmf_address_p2p_ae_sci_hi_1,
    bdmf_address_p2p_ae_sci_lo_2,
    bdmf_address_p2p_ae_sci_hi_2,
    bdmf_address_p2p_ae_sci_lo_3,
    bdmf_address_p2p_ae_sci_hi_3,
    bdmf_address_p2p_ae_sci_lo_4,
    bdmf_address_p2p_ae_sci_hi_4,
    bdmf_address_p2p_ae_sci_lo_5,
    bdmf_address_p2p_ae_sci_hi_5,
    bdmf_address_p2p_ae_sci_lo_6,
    bdmf_address_p2p_ae_sci_hi_6,
    bdmf_address_p2p_ae_sci_lo_7,
    bdmf_address_p2p_ae_sci_hi_7,
    bdmf_address_p2p_ae_sci_lo_8,
    bdmf_address_p2p_ae_sci_hi_8,
    bdmf_address_p2p_ae_sci_lo_9,
    bdmf_address_p2p_ae_sci_hi_9,
    bdmf_address_p2p_ae_sci_lo_10,
    bdmf_address_p2p_ae_sci_hi_10,
    bdmf_address_p2p_ae_sci_lo_11,
    bdmf_address_p2p_ae_sci_hi_11,
    bdmf_address_p2p_ae_sci_lo_12,
    bdmf_address_p2p_ae_sci_hi_12,
    bdmf_address_p2p_ae_sci_lo_13,
    bdmf_address_p2p_ae_sci_hi_13,
    bdmf_address_p2p_ae_sci_lo_14,
    bdmf_address_p2p_ae_sci_hi_14,
    bdmf_address_p2p_ae_sci_lo_15,
    bdmf_address_p2p_ae_sci_hi_15,
    bdmf_address_sec_up_key_stat_1,
    bdmf_address_sec_key_sel_1,
    bdmf_address_pon_sec_tx_plaintxt_ae_pad_control,
    bdmf_address_p2p_autoneg_control,
    bdmf_address_p2p_autoneg_status,
    bdmf_address_p2p_autoneg_ability_config_reg,
    bdmf_address_p2p_autoneg_link_partner_ability_config_read,
}
bdmf_address;

static int bcm_lif_cli_set(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    bdmf_error_t err;

    switch(parm[0].value.unumber)
    {
    case BDMF_pon_control:
    {
        lif_pon_control pon_control = { .cfgdisruntfilter=parm[1].value.unumber, .cfmaxcommaerrcnt=parm[2].value.unumber, .cfsyncsmselect=parm[3].value.unumber, .cfponrxforcenonfecabort=parm[4].value.unumber, .cfponrxforcefecabort=parm[5].value.unumber, .cfgrxdatabitflip=parm[6].value.unumber, .cfgenmpcpoampad=parm[7].value.unumber, .cfgentxidlepkt=parm[8].value.unumber, .cfenablesoftwaresynchold=parm[9].value.unumber, .cfenableextendsync=parm[10].value.unumber, .cfenablequicksync=parm[11].value.unumber, .cfppsen=parm[12].value.unumber, .cfppsclkrbc=parm[13].value.unumber, .cfrx2txlpback=parm[14].value.unumber, .cftx2rxlpback=parm[15].value.unumber, .cftxdataendurlon=parm[16].value.unumber, .cfp2pmode=parm[17].value.unumber, .cfp2pshortpre=parm[18].value.unumber, .cflaseren=parm[19].value.unumber, .cftxlaseron=parm[20].value.unumber, .cftxlaseronacthi=parm[21].value.unumber, .liftxrstn_pre=parm[22].value.unumber, .lifrxrstn_pre=parm[23].value.unumber, .liftxen=parm[24].value.unumber, .lifrxen=parm[25].value.unumber};
        err = ag_drv_lif_pon_control_set(&pon_control);
        break;
    }
    case BDMF_pon_inter_op_control:
    {
        lif_pon_inter_op_control pon_inter_op_control = { .cfipgfilter=parm[1].value.unumber, .cfdisableloslaserblock=parm[2].value.unumber, .cfgllidpromiscuousmode=parm[3].value.unumber, .cfgllidmodmsk=parm[4].value.unumber, .cfusefecipg=parm[5].value.unumber, .cfrxcrc8invchk=parm[6].value.unumber, .cfrxcrc8bitswap=parm[7].value.unumber, .cfrxcrc8msb2lsb=parm[8].value.unumber, .cfrxcrc8disable=parm[9].value.unumber, .cftxllidbit15set=parm[10].value.unumber, .cftxcrc8inv=parm[11].value.unumber, .cftxcrc8bad=parm[12].value.unumber, .cftxcrc8bitswap=parm[13].value.unumber, .cftxcrc8msb2lsb=parm[14].value.unumber, .cftxshortpre=parm[15].value.unumber, .cftxipgcnt=parm[16].value.unumber, .cftxaasynclen=parm[17].value.unumber, .cftxpipedelay=parm[18].value.unumber};
        err = ag_drv_lif_pon_inter_op_control_set(&pon_inter_op_control);
        break;
    }
    case BDMF_fec_control:
    {
        lif_fec_control fec_control = { .cffecrxerrorprop=parm[1].value.unumber, .cffecrxforcenonfecabort=parm[2].value.unumber, .cffecrxforcefecabort=parm[3].value.unumber, .cffecrxenable=parm[4].value.unumber, .cffectxfecperllid=parm[5].value.unumber, .cffectxenable=parm[6].value.unumber};
        err = ag_drv_lif_fec_control_set(&fec_control);
        break;
    }
    case BDMF_sec_control:
    {
        lif_sec_control sec_control = { .cfgdismpcpencrypt=parm[1].value.unumber, .cfgdisoamencrypt=parm[2].value.unumber, .cfgsecenshortlen=parm[3].value.unumber, .cfgenaereplayprct=parm[4].value.unumber, .cfgenlegacyrcc=parm[5].value.unumber, .enfakeupaes=parm[6].value.unumber, .enfakednaes=parm[7].value.unumber, .cfgfecipglen=parm[8].value.unumber, .disdndasaencrpt=parm[9].value.unumber, .entriplechurn=parm[10].value.unumber, .enepnmixencrypt=parm[11].value.unumber, .disupdasaencrpt=parm[12].value.unumber, .secupencryptscheme=parm[13].value.unumber, .secdnencryptscheme=parm[14].value.unumber, .secuprstn_pre=parm[15].value.unumber, .secdnrstn_pre=parm[16].value.unumber, .secenup=parm[17].value.unumber, .secendn=parm[18].value.unumber};
        err = ag_drv_lif_sec_control_set(&sec_control);
        break;
    }
    case BDMF_macsec:
        err = ag_drv_lif_macsec_set(parm[1].value.unumber);
        break;
    case BDMF_int_status:
    {
        lif_int_status int_status = { .int_sop_sfec_ipg_violation=parm[1].value.unumber, .laseronmax=parm[2].value.unumber, .laseroff=parm[3].value.unumber, .secdnreplayprotctabort=parm[4].value.unumber, .secuppktnumoverflow=parm[5].value.unumber, .intlaseroffdurburst=parm[6].value.unumber, .intrxberthreshexc=parm[7].value.unumber, .intfecrxfecrecvstatus=parm[8].value.unumber, .intfecrxcorerrfifofullstatus=parm[9].value.unumber, .intfecrxcorerrfifounexpempty=parm[10].value.unumber, .intfecbufpopemptypush=parm[11].value.unumber, .intfecbufpopemptynopush=parm[12].value.unumber, .intfecbufpushfull=parm[13].value.unumber, .intuptimefullupdstat=parm[14].value.unumber, .intfroutofalignstat=parm[15].value.unumber, .intgrntstarttimelagstat=parm[16].value.unumber, .intabortrxfrmstat=parm[17].value.unumber, .intnorxclkstat=parm[18].value.unumber, .intrxmaxlenerrstat=parm[19].value.unumber, .intrxerraftalignstat=parm[20].value.unumber, .intrxsynchacqstat=parm[21].value.unumber, .intrxoutofsynchstat=parm[22].value.unumber};
        err = ag_drv_lif_int_status_set(&int_status);
        break;
    }
    case BDMF_int_mask:
    {
        lif_int_mask int_mask = { .int_sop_sfec_ipg_violation_mask=parm[1].value.unumber, .laseronmaxmask=parm[2].value.unumber, .laseroffmask=parm[3].value.unumber, .secdnreplayprotctabortmsk=parm[4].value.unumber, .secuppktnumoverflowmsk=parm[5].value.unumber, .intlaseroffdurburstmask=parm[6].value.unumber, .intrxberthreshexcmask=parm[7].value.unumber, .intfecrxfecrecvmask=parm[8].value.unumber, .intfecrxcorerrfifofullmask=parm[9].value.unumber, .intfecrxcorerrfifounexpemptymask=parm[10].value.unumber, .intfecbufpopemptypushmask=parm[11].value.unumber, .intfecbufpopemptynopushmask=parm[12].value.unumber, .intfecbufpushfullmask=parm[13].value.unumber, .intuptimefullupdmask=parm[14].value.unumber, .intfroutofalignmask=parm[15].value.unumber, .intgrntstarttimelagmask=parm[16].value.unumber, .intabortrxfrmmask=parm[17].value.unumber, .intnorxclkmask=parm[18].value.unumber, .intrxmaxlenerrmask=parm[19].value.unumber, .intrxerraftalignmask=parm[20].value.unumber, .intrxsynchacqmask=parm[21].value.unumber, .intrxoutofsynchmask=parm[22].value.unumber};
        err = ag_drv_lif_int_mask_set(&int_mask);
        break;
    }
    case BDMF_data_port_command:
    {
        lif_data_port_command data_port_command = { .data_port_busy=parm[1].value.unumber, .data_port_error=parm[2].value.unumber, .ram_select=parm[3].value.unumber, .data_port_op_code=parm[4].value.unumber, .data_port_addr=parm[5].value.unumber};
        err = ag_drv_lif_data_port_command_set(&data_port_command);
        break;
    }
    case BDMF_data_port_data:
        err = ag_drv_lif_data_port_data_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case BDMF_llid_0:
        err = ag_drv_lif_llid_0_set(parm[1].value.unumber);
        break;
    case BDMF_llid_1:
        err = ag_drv_lif_llid_1_set(parm[1].value.unumber);
        break;
    case BDMF_llid_2:
        err = ag_drv_lif_llid_2_set(parm[1].value.unumber);
        break;
    case BDMF_llid_3:
        err = ag_drv_lif_llid_3_set(parm[1].value.unumber);
        break;
    case BDMF_llid_4:
        err = ag_drv_lif_llid_4_set(parm[1].value.unumber);
        break;
    case BDMF_llid_5:
        err = ag_drv_lif_llid_5_set(parm[1].value.unumber);
        break;
    case BDMF_llid_6:
        err = ag_drv_lif_llid_6_set(parm[1].value.unumber);
        break;
    case BDMF_llid_7:
        err = ag_drv_lif_llid_7_set(parm[1].value.unumber);
        break;
    case BDMF_llid_16:
        err = ag_drv_lif_llid_16_set(parm[1].value.unumber);
        break;
    case BDMF_llid_17:
        err = ag_drv_lif_llid_17_set(parm[1].value.unumber);
        break;
    case BDMF_llid_18:
        err = ag_drv_lif_llid_18_set(parm[1].value.unumber);
        break;
    case BDMF_llid_19:
        err = ag_drv_lif_llid_19_set(parm[1].value.unumber);
        break;
    case BDMF_llid_20:
        err = ag_drv_lif_llid_20_set(parm[1].value.unumber);
        break;
    case BDMF_llid_21:
        err = ag_drv_lif_llid_21_set(parm[1].value.unumber);
        break;
    case BDMF_llid_22:
        err = ag_drv_lif_llid_22_set(parm[1].value.unumber);
        break;
    case BDMF_llid_23:
        err = ag_drv_lif_llid_23_set(parm[1].value.unumber);
        break;
    case BDMF_time_ref_cnt:
        err = ag_drv_lif_time_ref_cnt_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber);
        break;
    case BDMF_timestamp_upd_per:
        err = ag_drv_lif_timestamp_upd_per_set(parm[1].value.unumber);
        break;
    case BDMF_tp_time:
        err = ag_drv_lif_tp_time_set(parm[1].value.unumber);
        break;
    case BDMF_maxlen_ctr:
        err = ag_drv_lif_maxlen_ctr_set(parm[1].value.unumber);
        break;
    case BDMF_laser_on_delta:
        err = ag_drv_lif_laser_on_delta_set(parm[1].value.unumber);
        break;
    case BDMF_laser_off_idle:
        err = ag_drv_lif_laser_off_idle_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case BDMF_fec_init_idle:
        err = ag_drv_lif_fec_init_idle_set(parm[1].value.unumber);
        break;
    case BDMF_fec_err_allow:
        err = ag_drv_lif_fec_err_allow_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case BDMF_dn_encrypt_stat:
        err = ag_drv_lif_dn_encrypt_stat_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case BDMF_sec_up_mpcp_offset:
        err = ag_drv_lif_sec_up_mpcp_offset_set(parm[2].value.unumber);
        break;
    case BDMF_fec_per_llid:
        err = ag_drv_lif_fec_per_llid_set(parm[2].value.unumber, parm[3].value.unumber);
        break;
    case BDMF_rx_line_code_err_cnt:
        err = ag_drv_lif_rx_line_code_err_cnt_set(parm[3].value.unumber);
        break;
    case BDMF_rx_agg_mpcp_frm:
        err = ag_drv_lif_rx_agg_mpcp_frm_set(parm[3].value.unumber);
        break;
    case BDMF_rx_agg_good_frm:
        err = ag_drv_lif_rx_agg_good_frm_set(parm[3].value.unumber);
        break;
    case BDMF_rx_agg_good_byte:
        err = ag_drv_lif_rx_agg_good_byte_set(parm[3].value.unumber);
        break;
    case BDMF_rx_agg_undersz_frm:
        err = ag_drv_lif_rx_agg_undersz_frm_set(parm[3].value.unumber);
        break;
    case BDMF_rx_agg_oversz_frm:
        err = ag_drv_lif_rx_agg_oversz_frm_set(parm[3].value.unumber);
        break;
    case BDMF_rx_agg_crc8_frm:
        err = ag_drv_lif_rx_agg_crc8_frm_set(parm[3].value.unumber);
        break;
    case BDMF_rx_agg_fec_frm:
        err = ag_drv_lif_rx_agg_fec_frm_set(parm[3].value.unumber);
        break;
    case BDMF_rx_agg_fec_byte:
        err = ag_drv_lif_rx_agg_fec_byte_set(parm[3].value.unumber);
        break;
    case BDMF_rx_agg_fec_exc_err_frm:
        err = ag_drv_lif_rx_agg_fec_exc_err_frm_set(parm[3].value.unumber);
        break;
    case BDMF_rx_agg_nonfec_good_frm:
        err = ag_drv_lif_rx_agg_nonfec_good_frm_set(parm[3].value.unumber);
        break;
    case BDMF_rx_agg_nonfec_good_byte:
        err = ag_drv_lif_rx_agg_nonfec_good_byte_set(parm[3].value.unumber);
        break;
    case BDMF_rx_agg_err_bytes:
        err = ag_drv_lif_rx_agg_err_bytes_set(parm[3].value.unumber);
        break;
    case BDMF_rx_agg_err_zeroes:
        err = ag_drv_lif_rx_agg_err_zeroes_set(parm[3].value.unumber);
        break;
    case BDMF_rx_agg_no_err_blks:
        err = ag_drv_lif_rx_agg_no_err_blks_set(parm[3].value.unumber);
        break;
    case BDMF_rx_agg_cor_blks:
        err = ag_drv_lif_rx_agg_cor_blks_set(parm[3].value.unumber);
        break;
    case BDMF_rx_agg_uncor_blks:
        err = ag_drv_lif_rx_agg_uncor_blks_set(parm[3].value.unumber);
        break;
    case BDMF_rx_agg_err_ones:
        err = ag_drv_lif_rx_agg_err_ones_set(parm[3].value.unumber);
        break;
    case BDMF_rx_agg_err_frm:
        err = ag_drv_lif_rx_agg_err_frm_set(parm[3].value.unumber);
        break;
    case BDMF_tx_pkt_cnt:
        err = ag_drv_lif_tx_pkt_cnt_set(parm[3].value.unumber);
        break;
    case BDMF_tx_byte_cnt:
        err = ag_drv_lif_tx_byte_cnt_set(parm[3].value.unumber);
        break;
    case BDMF_tx_non_fec_pkt_cnt:
        err = ag_drv_lif_tx_non_fec_pkt_cnt_set(parm[3].value.unumber);
        break;
    case BDMF_tx_non_fec_byte_cnt:
        err = ag_drv_lif_tx_non_fec_byte_cnt_set(parm[3].value.unumber);
        break;
    case BDMF_tx_fec_pkt_cnt:
        err = ag_drv_lif_tx_fec_pkt_cnt_set(parm[3].value.unumber);
        break;
    case BDMF_tx_fec_byte_cnt:
        err = ag_drv_lif_tx_fec_byte_cnt_set(parm[3].value.unumber);
        break;
    case BDMF_tx_fec_blk_cnt:
        err = ag_drv_lif_tx_fec_blk_cnt_set(parm[3].value.unumber);
        break;
    case BDMF_tx_mpcp_pkt_cnt:
        err = ag_drv_lif_tx_mpcp_pkt_cnt_set(parm[3].value.unumber);
        break;
    case BDMF_debug_tx_data_pkt_cnt:
        err = ag_drv_lif_debug_tx_data_pkt_cnt_set(parm[3].value.unumber);
        break;
    case BDMF_fec_llid_status:
        err = ag_drv_lif_fec_llid_status_set(parm[3].value.unumber);
        break;
    case BDMF_sec_rx_tek_ig_iv_llid:
        err = ag_drv_lif_sec_rx_tek_ig_iv_llid_set(parm[3].value.unumber);
        break;
    case BDMF_pon_ber_interv_thresh:
        err = ag_drv_lif_pon_ber_interv_thresh_set(parm[3].value.unumber, parm[4].value.unumber, parm[5].value.unumber);
        break;
    case BDMF_lsr_mon_a_ctrl:
        err = ag_drv_lif_lsr_mon_a_ctrl_set(parm[3].value.unumber, parm[4].value.unumber, parm[5].value.unumber);
        break;
    case BDMF_lsr_mon_a_max_thr:
        err = ag_drv_lif_lsr_mon_a_max_thr_set(parm[3].value.unumber);
        break;
    case BDMF_ae_pktnum_window:
        err = ag_drv_lif_ae_pktnum_window_set(parm[3].value.unumber);
        break;
    case BDMF_ae_pktnum_thresh:
        err = ag_drv_lif_ae_pktnum_thresh_set(parm[3].value.unumber);
        break;
    case BDMF_llid_8:
        err = ag_drv_lif_llid_8_set(parm[3].value.unumber);
        break;
    case BDMF_llid_9:
        err = ag_drv_lif_llid_9_set(parm[3].value.unumber);
        break;
    case BDMF_llid_10:
        err = ag_drv_lif_llid_10_set(parm[3].value.unumber);
        break;
    case BDMF_llid_11:
        err = ag_drv_lif_llid_11_set(parm[3].value.unumber);
        break;
    case BDMF_llid_12:
        err = ag_drv_lif_llid_12_set(parm[3].value.unumber);
        break;
    case BDMF_llid_13:
        err = ag_drv_lif_llid_13_set(parm[3].value.unumber);
        break;
    case BDMF_llid_14:
        err = ag_drv_lif_llid_14_set(parm[3].value.unumber);
        break;
    case BDMF_llid_15:
        err = ag_drv_lif_llid_15_set(parm[3].value.unumber);
        break;
    case BDMF_llid_24:
        err = ag_drv_lif_llid_24_set(parm[3].value.unumber);
        break;
    case BDMF_llid_25:
        err = ag_drv_lif_llid_25_set(parm[3].value.unumber);
        break;
    case BDMF_llid_26:
        err = ag_drv_lif_llid_26_set(parm[3].value.unumber);
        break;
    case BDMF_llid_27:
        err = ag_drv_lif_llid_27_set(parm[3].value.unumber);
        break;
    case BDMF_llid_28:
        err = ag_drv_lif_llid_28_set(parm[3].value.unumber);
        break;
    case BDMF_llid_29:
        err = ag_drv_lif_llid_29_set(parm[3].value.unumber);
        break;
    case BDMF_llid_30:
        err = ag_drv_lif_llid_30_set(parm[3].value.unumber);
        break;
    case BDMF_llid_31:
        err = ag_drv_lif_llid_31_set(parm[3].value.unumber);
        break;
    case BDMF_vlan_type:
        err = ag_drv_lif_vlan_type_set(parm[3].value.unumber);
        break;
    case BDMF_p2p_ae_sci_en:
        err = ag_drv_lif_p2p_ae_sci_en_set(parm[3].value.unumber);
        break;
    case BDMF_p2p_ae_sci_lo_0:
        err = ag_drv_lif_p2p_ae_sci_lo_0_set(parm[3].value.unumber);
        break;
    case BDMF_p2p_ae_sci_hi_0:
        err = ag_drv_lif_p2p_ae_sci_hi_0_set(parm[3].value.unumber);
        break;
    case BDMF_p2p_ae_sci_lo_1:
        err = ag_drv_lif_p2p_ae_sci_lo_1_set(parm[3].value.unumber);
        break;
    case BDMF_p2p_ae_sci_hi_1:
        err = ag_drv_lif_p2p_ae_sci_hi_1_set(parm[3].value.unumber);
        break;
    case BDMF_p2p_ae_sci_lo_2:
        err = ag_drv_lif_p2p_ae_sci_lo_2_set(parm[3].value.unumber);
        break;
    case BDMF_p2p_ae_sci_hi_2:
        err = ag_drv_lif_p2p_ae_sci_hi_2_set(parm[3].value.unumber);
        break;
    case BDMF_p2p_ae_sci_lo_3:
        err = ag_drv_lif_p2p_ae_sci_lo_3_set(parm[3].value.unumber);
        break;
    case BDMF_p2p_ae_sci_hi_3:
        err = ag_drv_lif_p2p_ae_sci_hi_3_set(parm[3].value.unumber);
        break;
    case BDMF_p2p_ae_sci_lo_4:
        err = ag_drv_lif_p2p_ae_sci_lo_4_set(parm[3].value.unumber);
        break;
    case BDMF_p2p_ae_sci_hi_4:
        err = ag_drv_lif_p2p_ae_sci_hi_4_set(parm[3].value.unumber);
        break;
    case BDMF_p2p_ae_sci_lo_5:
        err = ag_drv_lif_p2p_ae_sci_lo_5_set(parm[3].value.unumber);
        break;
    case BDMF_p2p_ae_sci_hi_5:
        err = ag_drv_lif_p2p_ae_sci_hi_5_set(parm[3].value.unumber);
        break;
    case BDMF_p2p_ae_sci_lo_6:
        err = ag_drv_lif_p2p_ae_sci_lo_6_set(parm[3].value.unumber);
        break;
    case BDMF_p2p_ae_sci_hi_6:
        err = ag_drv_lif_p2p_ae_sci_hi_6_set(parm[3].value.unumber);
        break;
    case BDMF_p2p_ae_sci_lo_7:
        err = ag_drv_lif_p2p_ae_sci_lo_7_set(parm[3].value.unumber);
        break;
    case BDMF_p2p_ae_sci_hi_7:
        err = ag_drv_lif_p2p_ae_sci_hi_7_set(parm[3].value.unumber);
        break;
    case BDMF_p2p_ae_sci_lo_8:
        err = ag_drv_lif_p2p_ae_sci_lo_8_set(parm[3].value.unumber);
        break;
    case BDMF_p2p_ae_sci_hi_8:
        err = ag_drv_lif_p2p_ae_sci_hi_8_set(parm[3].value.unumber);
        break;
    case BDMF_p2p_ae_sci_lo_9:
        err = ag_drv_lif_p2p_ae_sci_lo_9_set(parm[3].value.unumber);
        break;
    case BDMF_p2p_ae_sci_hi_9:
        err = ag_drv_lif_p2p_ae_sci_hi_9_set(parm[3].value.unumber);
        break;
    case BDMF_p2p_ae_sci_lo_10:
        err = ag_drv_lif_p2p_ae_sci_lo_10_set(parm[3].value.unumber);
        break;
    case BDMF_p2p_ae_sci_hi_10:
        err = ag_drv_lif_p2p_ae_sci_hi_10_set(parm[3].value.unumber);
        break;
    case BDMF_p2p_ae_sci_lo_11:
        err = ag_drv_lif_p2p_ae_sci_lo_11_set(parm[3].value.unumber);
        break;
    case BDMF_p2p_ae_sci_hi_11:
        err = ag_drv_lif_p2p_ae_sci_hi_11_set(parm[3].value.unumber);
        break;
    case BDMF_p2p_ae_sci_lo_12:
        err = ag_drv_lif_p2p_ae_sci_lo_12_set(parm[3].value.unumber);
        break;
    case BDMF_p2p_ae_sci_hi_12:
        err = ag_drv_lif_p2p_ae_sci_hi_12_set(parm[3].value.unumber);
        break;
    case BDMF_p2p_ae_sci_lo_13:
        err = ag_drv_lif_p2p_ae_sci_lo_13_set(parm[3].value.unumber);
        break;
    case BDMF_p2p_ae_sci_hi_13:
        err = ag_drv_lif_p2p_ae_sci_hi_13_set(parm[3].value.unumber);
        break;
    case BDMF_p2p_ae_sci_lo_14:
        err = ag_drv_lif_p2p_ae_sci_lo_14_set(parm[3].value.unumber);
        break;
    case BDMF_p2p_ae_sci_hi_14:
        err = ag_drv_lif_p2p_ae_sci_hi_14_set(parm[3].value.unumber);
        break;
    case BDMF_p2p_ae_sci_lo_15:
        err = ag_drv_lif_p2p_ae_sci_lo_15_set(parm[3].value.unumber);
        break;
    case BDMF_p2p_ae_sci_hi_15:
        err = ag_drv_lif_p2p_ae_sci_hi_15_set(parm[3].value.unumber);
        break;
    case BDMF_pon_sec_tx_plaintxt_ae_pad_control:
        err = ag_drv_lif_pon_sec_tx_plaintxt_ae_pad_control_set(parm[3].value.unumber, parm[4].value.unumber);
        break;
    case BDMF_p2p_autoneg_control:
        err = ag_drv_lif_p2p_autoneg_control_set(parm[3].value.unumber, parm[4].value.unumber, parm[5].value.unumber, parm[6].value.unumber);
        break;
    case BDMF_p2p_autoneg_ability_config_reg:
        err = ag_drv_lif_p2p_autoneg_ability_config_reg_set(parm[3].value.unumber);
        break;
    default:
        err = BDMF_ERR_NOT_SUPPORTED;
        break;
    }
    return err;
}

static int bcm_lif_cli_get(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    bdmf_error_t err;

    switch(parm[0].value.unumber)
    {
    case BDMF_pon_control:
    {
        lif_pon_control pon_control;
        err = ag_drv_lif_pon_control_get(&pon_control);
        bdmf_session_print(session, "cfgdisruntfilter = %u = 0x%x\n", pon_control.cfgdisruntfilter, pon_control.cfgdisruntfilter);
        bdmf_session_print(session, "cfmaxcommaerrcnt = %u = 0x%x\n", pon_control.cfmaxcommaerrcnt, pon_control.cfmaxcommaerrcnt);
        bdmf_session_print(session, "cfsyncsmselect = %u = 0x%x\n", pon_control.cfsyncsmselect, pon_control.cfsyncsmselect);
        bdmf_session_print(session, "cfponrxforcenonfecabort = %u = 0x%x\n", pon_control.cfponrxforcenonfecabort, pon_control.cfponrxforcenonfecabort);
        bdmf_session_print(session, "cfponrxforcefecabort = %u = 0x%x\n", pon_control.cfponrxforcefecabort, pon_control.cfponrxforcefecabort);
        bdmf_session_print(session, "cfgrxdatabitflip = %u = 0x%x\n", pon_control.cfgrxdatabitflip, pon_control.cfgrxdatabitflip);
        bdmf_session_print(session, "cfgenmpcpoampad = %u = 0x%x\n", pon_control.cfgenmpcpoampad, pon_control.cfgenmpcpoampad);
        bdmf_session_print(session, "cfgentxidlepkt = %u = 0x%x\n", pon_control.cfgentxidlepkt, pon_control.cfgentxidlepkt);
        bdmf_session_print(session, "cfenablesoftwaresynchold = %u = 0x%x\n", pon_control.cfenablesoftwaresynchold, pon_control.cfenablesoftwaresynchold);
        bdmf_session_print(session, "cfenableextendsync = %u = 0x%x\n", pon_control.cfenableextendsync, pon_control.cfenableextendsync);
        bdmf_session_print(session, "cfenablequicksync = %u = 0x%x\n", pon_control.cfenablequicksync, pon_control.cfenablequicksync);
        bdmf_session_print(session, "cfppsen = %u = 0x%x\n", pon_control.cfppsen, pon_control.cfppsen);
        bdmf_session_print(session, "cfppsclkrbc = %u = 0x%x\n", pon_control.cfppsclkrbc, pon_control.cfppsclkrbc);
        bdmf_session_print(session, "cfrx2txlpback = %u = 0x%x\n", pon_control.cfrx2txlpback, pon_control.cfrx2txlpback);
        bdmf_session_print(session, "cftx2rxlpback = %u = 0x%x\n", pon_control.cftx2rxlpback, pon_control.cftx2rxlpback);
        bdmf_session_print(session, "cftxdataendurlon = %u = 0x%x\n", pon_control.cftxdataendurlon, pon_control.cftxdataendurlon);
        bdmf_session_print(session, "cfp2pmode = %u = 0x%x\n", pon_control.cfp2pmode, pon_control.cfp2pmode);
        bdmf_session_print(session, "cfp2pshortpre = %u = 0x%x\n", pon_control.cfp2pshortpre, pon_control.cfp2pshortpre);
        bdmf_session_print(session, "cflaseren = %u = 0x%x\n", pon_control.cflaseren, pon_control.cflaseren);
        bdmf_session_print(session, "cftxlaseron = %u = 0x%x\n", pon_control.cftxlaseron, pon_control.cftxlaseron);
        bdmf_session_print(session, "cftxlaseronacthi = %u = 0x%x\n", pon_control.cftxlaseronacthi, pon_control.cftxlaseronacthi);
        bdmf_session_print(session, "liftxrstn_pre = %u = 0x%x\n", pon_control.liftxrstn_pre, pon_control.liftxrstn_pre);
        bdmf_session_print(session, "lifrxrstn_pre = %u = 0x%x\n", pon_control.lifrxrstn_pre, pon_control.lifrxrstn_pre);
        bdmf_session_print(session, "liftxen = %u = 0x%x\n", pon_control.liftxen, pon_control.liftxen);
        bdmf_session_print(session, "lifrxen = %u = 0x%x\n", pon_control.lifrxen, pon_control.lifrxen);
        break;
    }
    case BDMF_pon_inter_op_control:
    {
        lif_pon_inter_op_control pon_inter_op_control;
        err = ag_drv_lif_pon_inter_op_control_get(&pon_inter_op_control);
        bdmf_session_print(session, "cfipgfilter = %u = 0x%x\n", pon_inter_op_control.cfipgfilter, pon_inter_op_control.cfipgfilter);
        bdmf_session_print(session, "cfdisableloslaserblock = %u = 0x%x\n", pon_inter_op_control.cfdisableloslaserblock, pon_inter_op_control.cfdisableloslaserblock);
        bdmf_session_print(session, "cfgllidpromiscuousmode = %u = 0x%x\n", pon_inter_op_control.cfgllidpromiscuousmode, pon_inter_op_control.cfgllidpromiscuousmode);
        bdmf_session_print(session, "cfgllidmodmsk = %u = 0x%x\n", pon_inter_op_control.cfgllidmodmsk, pon_inter_op_control.cfgllidmodmsk);
        bdmf_session_print(session, "cfusefecipg = %u = 0x%x\n", pon_inter_op_control.cfusefecipg, pon_inter_op_control.cfusefecipg);
        bdmf_session_print(session, "cfrxcrc8invchk = %u = 0x%x\n", pon_inter_op_control.cfrxcrc8invchk, pon_inter_op_control.cfrxcrc8invchk);
        bdmf_session_print(session, "cfrxcrc8bitswap = %u = 0x%x\n", pon_inter_op_control.cfrxcrc8bitswap, pon_inter_op_control.cfrxcrc8bitswap);
        bdmf_session_print(session, "cfrxcrc8msb2lsb = %u = 0x%x\n", pon_inter_op_control.cfrxcrc8msb2lsb, pon_inter_op_control.cfrxcrc8msb2lsb);
        bdmf_session_print(session, "cfrxcrc8disable = %u = 0x%x\n", pon_inter_op_control.cfrxcrc8disable, pon_inter_op_control.cfrxcrc8disable);
        bdmf_session_print(session, "cftxllidbit15set = %u = 0x%x\n", pon_inter_op_control.cftxllidbit15set, pon_inter_op_control.cftxllidbit15set);
        bdmf_session_print(session, "cftxcrc8inv = %u = 0x%x\n", pon_inter_op_control.cftxcrc8inv, pon_inter_op_control.cftxcrc8inv);
        bdmf_session_print(session, "cftxcrc8bad = %u = 0x%x\n", pon_inter_op_control.cftxcrc8bad, pon_inter_op_control.cftxcrc8bad);
        bdmf_session_print(session, "cftxcrc8bitswap = %u = 0x%x\n", pon_inter_op_control.cftxcrc8bitswap, pon_inter_op_control.cftxcrc8bitswap);
        bdmf_session_print(session, "cftxcrc8msb2lsb = %u = 0x%x\n", pon_inter_op_control.cftxcrc8msb2lsb, pon_inter_op_control.cftxcrc8msb2lsb);
        bdmf_session_print(session, "cftxshortpre = %u = 0x%x\n", pon_inter_op_control.cftxshortpre, pon_inter_op_control.cftxshortpre);
        bdmf_session_print(session, "cftxipgcnt = %u = 0x%x\n", pon_inter_op_control.cftxipgcnt, pon_inter_op_control.cftxipgcnt);
        bdmf_session_print(session, "cftxaasynclen = %u = 0x%x\n", pon_inter_op_control.cftxaasynclen, pon_inter_op_control.cftxaasynclen);
        bdmf_session_print(session, "cftxpipedelay = %u = 0x%x\n", pon_inter_op_control.cftxpipedelay, pon_inter_op_control.cftxpipedelay);
        break;
    }
    case BDMF_fec_control:
    {
        lif_fec_control fec_control;
        err = ag_drv_lif_fec_control_get(&fec_control);
        bdmf_session_print(session, "cffecrxerrorprop = %u = 0x%x\n", fec_control.cffecrxerrorprop, fec_control.cffecrxerrorprop);
        bdmf_session_print(session, "cffecrxforcenonfecabort = %u = 0x%x\n", fec_control.cffecrxforcenonfecabort, fec_control.cffecrxforcenonfecabort);
        bdmf_session_print(session, "cffecrxforcefecabort = %u = 0x%x\n", fec_control.cffecrxforcefecabort, fec_control.cffecrxforcefecabort);
        bdmf_session_print(session, "cffecrxenable = %u = 0x%x\n", fec_control.cffecrxenable, fec_control.cffecrxenable);
        bdmf_session_print(session, "cffectxfecperllid = %u = 0x%x\n", fec_control.cffectxfecperllid, fec_control.cffectxfecperllid);
        bdmf_session_print(session, "cffectxenable = %u = 0x%x\n", fec_control.cffectxenable, fec_control.cffectxenable);
        break;
    }
    case BDMF_sec_control:
    {
        lif_sec_control sec_control;
        err = ag_drv_lif_sec_control_get(&sec_control);
        bdmf_session_print(session, "cfgdismpcpencrypt = %u = 0x%x\n", sec_control.cfgdismpcpencrypt, sec_control.cfgdismpcpencrypt);
        bdmf_session_print(session, "cfgdisoamencrypt = %u = 0x%x\n", sec_control.cfgdisoamencrypt, sec_control.cfgdisoamencrypt);
        bdmf_session_print(session, "cfgsecenshortlen = %u = 0x%x\n", sec_control.cfgsecenshortlen, sec_control.cfgsecenshortlen);
        bdmf_session_print(session, "cfgenaereplayprct = %u = 0x%x\n", sec_control.cfgenaereplayprct, sec_control.cfgenaereplayprct);
        bdmf_session_print(session, "cfgenlegacyrcc = %u = 0x%x\n", sec_control.cfgenlegacyrcc, sec_control.cfgenlegacyrcc);
        bdmf_session_print(session, "enfakeupaes = %u = 0x%x\n", sec_control.enfakeupaes, sec_control.enfakeupaes);
        bdmf_session_print(session, "enfakednaes = %u = 0x%x\n", sec_control.enfakednaes, sec_control.enfakednaes);
        bdmf_session_print(session, "cfgfecipglen = %u = 0x%x\n", sec_control.cfgfecipglen, sec_control.cfgfecipglen);
        bdmf_session_print(session, "disdndasaencrpt = %u = 0x%x\n", sec_control.disdndasaencrpt, sec_control.disdndasaencrpt);
        bdmf_session_print(session, "entriplechurn = %u = 0x%x\n", sec_control.entriplechurn, sec_control.entriplechurn);
        bdmf_session_print(session, "enepnmixencrypt = %u = 0x%x\n", sec_control.enepnmixencrypt, sec_control.enepnmixencrypt);
        bdmf_session_print(session, "disupdasaencrpt = %u = 0x%x\n", sec_control.disupdasaencrpt, sec_control.disupdasaencrpt);
        bdmf_session_print(session, "secupencryptscheme = %u = 0x%x\n", sec_control.secupencryptscheme, sec_control.secupencryptscheme);
        bdmf_session_print(session, "secdnencryptscheme = %u = 0x%x\n", sec_control.secdnencryptscheme, sec_control.secdnencryptscheme);
        bdmf_session_print(session, "secuprstn_pre = %u = 0x%x\n", sec_control.secuprstn_pre, sec_control.secuprstn_pre);
        bdmf_session_print(session, "secdnrstn_pre = %u = 0x%x\n", sec_control.secdnrstn_pre, sec_control.secdnrstn_pre);
        bdmf_session_print(session, "secenup = %u = 0x%x\n", sec_control.secenup, sec_control.secenup);
        bdmf_session_print(session, "secendn = %u = 0x%x\n", sec_control.secendn, sec_control.secendn);
        break;
    }
    case BDMF_macsec:
    {
        uint16_t cfgmacsecethertype;
        err = ag_drv_lif_macsec_get(&cfgmacsecethertype);
        bdmf_session_print(session, "cfgmacsecethertype = %u = 0x%x\n", cfgmacsecethertype, cfgmacsecethertype);
        break;
    }
    case BDMF_int_status:
    {
        lif_int_status int_status;
        err = ag_drv_lif_int_status_get(&int_status);
        bdmf_session_print(session, "int_sop_sfec_ipg_violation = %u = 0x%x\n", int_status.int_sop_sfec_ipg_violation, int_status.int_sop_sfec_ipg_violation);
        bdmf_session_print(session, "laseronmax = %u = 0x%x\n", int_status.laseronmax, int_status.laseronmax);
        bdmf_session_print(session, "laseroff = %u = 0x%x\n", int_status.laseroff, int_status.laseroff);
        bdmf_session_print(session, "secdnreplayprotctabort = %u = 0x%x\n", int_status.secdnreplayprotctabort, int_status.secdnreplayprotctabort);
        bdmf_session_print(session, "secuppktnumoverflow = %u = 0x%x\n", int_status.secuppktnumoverflow, int_status.secuppktnumoverflow);
        bdmf_session_print(session, "intlaseroffdurburst = %u = 0x%x\n", int_status.intlaseroffdurburst, int_status.intlaseroffdurburst);
        bdmf_session_print(session, "intrxberthreshexc = %u = 0x%x\n", int_status.intrxberthreshexc, int_status.intrxberthreshexc);
        bdmf_session_print(session, "intfecrxfecrecvstatus = %u = 0x%x\n", int_status.intfecrxfecrecvstatus, int_status.intfecrxfecrecvstatus);
        bdmf_session_print(session, "intfecrxcorerrfifofullstatus = %u = 0x%x\n", int_status.intfecrxcorerrfifofullstatus, int_status.intfecrxcorerrfifofullstatus);
        bdmf_session_print(session, "intfecrxcorerrfifounexpempty = %u = 0x%x\n", int_status.intfecrxcorerrfifounexpempty, int_status.intfecrxcorerrfifounexpempty);
        bdmf_session_print(session, "intfecbufpopemptypush = %u = 0x%x\n", int_status.intfecbufpopemptypush, int_status.intfecbufpopemptypush);
        bdmf_session_print(session, "intfecbufpopemptynopush = %u = 0x%x\n", int_status.intfecbufpopemptynopush, int_status.intfecbufpopemptynopush);
        bdmf_session_print(session, "intfecbufpushfull = %u = 0x%x\n", int_status.intfecbufpushfull, int_status.intfecbufpushfull);
        bdmf_session_print(session, "intuptimefullupdstat = %u = 0x%x\n", int_status.intuptimefullupdstat, int_status.intuptimefullupdstat);
        bdmf_session_print(session, "intfroutofalignstat = %u = 0x%x\n", int_status.intfroutofalignstat, int_status.intfroutofalignstat);
        bdmf_session_print(session, "intgrntstarttimelagstat = %u = 0x%x\n", int_status.intgrntstarttimelagstat, int_status.intgrntstarttimelagstat);
        bdmf_session_print(session, "intabortrxfrmstat = %u = 0x%x\n", int_status.intabortrxfrmstat, int_status.intabortrxfrmstat);
        bdmf_session_print(session, "intnorxclkstat = %u = 0x%x\n", int_status.intnorxclkstat, int_status.intnorxclkstat);
        bdmf_session_print(session, "intrxmaxlenerrstat = %u = 0x%x\n", int_status.intrxmaxlenerrstat, int_status.intrxmaxlenerrstat);
        bdmf_session_print(session, "intrxerraftalignstat = %u = 0x%x\n", int_status.intrxerraftalignstat, int_status.intrxerraftalignstat);
        bdmf_session_print(session, "intrxsynchacqstat = %u = 0x%x\n", int_status.intrxsynchacqstat, int_status.intrxsynchacqstat);
        bdmf_session_print(session, "intrxoutofsynchstat = %u = 0x%x\n", int_status.intrxoutofsynchstat, int_status.intrxoutofsynchstat);
        break;
    }
    case BDMF_int_mask:
    {
        lif_int_mask int_mask;
        err = ag_drv_lif_int_mask_get(&int_mask);
        bdmf_session_print(session, "int_sop_sfec_ipg_violation_mask = %u = 0x%x\n", int_mask.int_sop_sfec_ipg_violation_mask, int_mask.int_sop_sfec_ipg_violation_mask);
        bdmf_session_print(session, "laseronmaxmask = %u = 0x%x\n", int_mask.laseronmaxmask, int_mask.laseronmaxmask);
        bdmf_session_print(session, "laseroffmask = %u = 0x%x\n", int_mask.laseroffmask, int_mask.laseroffmask);
        bdmf_session_print(session, "secdnreplayprotctabortmsk = %u = 0x%x\n", int_mask.secdnreplayprotctabortmsk, int_mask.secdnreplayprotctabortmsk);
        bdmf_session_print(session, "secuppktnumoverflowmsk = %u = 0x%x\n", int_mask.secuppktnumoverflowmsk, int_mask.secuppktnumoverflowmsk);
        bdmf_session_print(session, "intlaseroffdurburstmask = %u = 0x%x\n", int_mask.intlaseroffdurburstmask, int_mask.intlaseroffdurburstmask);
        bdmf_session_print(session, "intrxberthreshexcmask = %u = 0x%x\n", int_mask.intrxberthreshexcmask, int_mask.intrxberthreshexcmask);
        bdmf_session_print(session, "intfecrxfecrecvmask = %u = 0x%x\n", int_mask.intfecrxfecrecvmask, int_mask.intfecrxfecrecvmask);
        bdmf_session_print(session, "intfecrxcorerrfifofullmask = %u = 0x%x\n", int_mask.intfecrxcorerrfifofullmask, int_mask.intfecrxcorerrfifofullmask);
        bdmf_session_print(session, "intfecrxcorerrfifounexpemptymask = %u = 0x%x\n", int_mask.intfecrxcorerrfifounexpemptymask, int_mask.intfecrxcorerrfifounexpemptymask);
        bdmf_session_print(session, "intfecbufpopemptypushmask = %u = 0x%x\n", int_mask.intfecbufpopemptypushmask, int_mask.intfecbufpopemptypushmask);
        bdmf_session_print(session, "intfecbufpopemptynopushmask = %u = 0x%x\n", int_mask.intfecbufpopemptynopushmask, int_mask.intfecbufpopemptynopushmask);
        bdmf_session_print(session, "intfecbufpushfullmask = %u = 0x%x\n", int_mask.intfecbufpushfullmask, int_mask.intfecbufpushfullmask);
        bdmf_session_print(session, "intuptimefullupdmask = %u = 0x%x\n", int_mask.intuptimefullupdmask, int_mask.intuptimefullupdmask);
        bdmf_session_print(session, "intfroutofalignmask = %u = 0x%x\n", int_mask.intfroutofalignmask, int_mask.intfroutofalignmask);
        bdmf_session_print(session, "intgrntstarttimelagmask = %u = 0x%x\n", int_mask.intgrntstarttimelagmask, int_mask.intgrntstarttimelagmask);
        bdmf_session_print(session, "intabortrxfrmmask = %u = 0x%x\n", int_mask.intabortrxfrmmask, int_mask.intabortrxfrmmask);
        bdmf_session_print(session, "intnorxclkmask = %u = 0x%x\n", int_mask.intnorxclkmask, int_mask.intnorxclkmask);
        bdmf_session_print(session, "intrxmaxlenerrmask = %u = 0x%x\n", int_mask.intrxmaxlenerrmask, int_mask.intrxmaxlenerrmask);
        bdmf_session_print(session, "intrxerraftalignmask = %u = 0x%x\n", int_mask.intrxerraftalignmask, int_mask.intrxerraftalignmask);
        bdmf_session_print(session, "intrxsynchacqmask = %u = 0x%x\n", int_mask.intrxsynchacqmask, int_mask.intrxsynchacqmask);
        bdmf_session_print(session, "intrxoutofsynchmask = %u = 0x%x\n", int_mask.intrxoutofsynchmask, int_mask.intrxoutofsynchmask);
        break;
    }
    case BDMF_data_port_command:
    {
        lif_data_port_command data_port_command;
        err = ag_drv_lif_data_port_command_get(&data_port_command);
        bdmf_session_print(session, "data_port_busy = %u = 0x%x\n", data_port_command.data_port_busy, data_port_command.data_port_busy);
        bdmf_session_print(session, "data_port_error = %u = 0x%x\n", data_port_command.data_port_error, data_port_command.data_port_error);
        bdmf_session_print(session, "ram_select = %u = 0x%x\n", data_port_command.ram_select, data_port_command.ram_select);
        bdmf_session_print(session, "data_port_op_code = %u = 0x%x\n", data_port_command.data_port_op_code, data_port_command.data_port_op_code);
        bdmf_session_print(session, "data_port_addr = %u = 0x%x\n", data_port_command.data_port_addr, data_port_command.data_port_addr);
        break;
    }
    case BDMF_data_port_data:
    {
        uint32_t pbiportdata;
        err = ag_drv_lif_data_port_data_get(parm[1].value.unumber, &pbiportdata);
        bdmf_session_print(session, "pbiportdata = %u = 0x%x\n", pbiportdata, pbiportdata);
        break;
    }
    case BDMF_llid_0:
    {
        uint32_t cfgllid0;
        err = ag_drv_lif_llid_0_get(&cfgllid0);
        bdmf_session_print(session, "cfgllid0 = %u = 0x%x\n", cfgllid0, cfgllid0);
        break;
    }
    case BDMF_llid_1:
    {
        uint32_t cfgllid1;
        err = ag_drv_lif_llid_1_get(&cfgllid1);
        bdmf_session_print(session, "cfgllid1 = %u = 0x%x\n", cfgllid1, cfgllid1);
        break;
    }
    case BDMF_llid_2:
    {
        uint32_t cfgllid2;
        err = ag_drv_lif_llid_2_get(&cfgllid2);
        bdmf_session_print(session, "cfgllid2 = %u = 0x%x\n", cfgllid2, cfgllid2);
        break;
    }
    case BDMF_llid_3:
    {
        uint32_t cfgllid3;
        err = ag_drv_lif_llid_3_get(&cfgllid3);
        bdmf_session_print(session, "cfgllid3 = %u = 0x%x\n", cfgllid3, cfgllid3);
        break;
    }
    case BDMF_llid_4:
    {
        uint32_t cfgllid4;
        err = ag_drv_lif_llid_4_get(&cfgllid4);
        bdmf_session_print(session, "cfgllid4 = %u = 0x%x\n", cfgllid4, cfgllid4);
        break;
    }
    case BDMF_llid_5:
    {
        uint32_t cfgllid5;
        err = ag_drv_lif_llid_5_get(&cfgllid5);
        bdmf_session_print(session, "cfgllid5 = %u = 0x%x\n", cfgllid5, cfgllid5);
        break;
    }
    case BDMF_llid_6:
    {
        uint32_t cfgllid6;
        err = ag_drv_lif_llid_6_get(&cfgllid6);
        bdmf_session_print(session, "cfgllid6 = %u = 0x%x\n", cfgllid6, cfgllid6);
        break;
    }
    case BDMF_llid_7:
    {
        uint32_t cfgllid7;
        err = ag_drv_lif_llid_7_get(&cfgllid7);
        bdmf_session_print(session, "cfgllid7 = %u = 0x%x\n", cfgllid7, cfgllid7);
        break;
    }
    case BDMF_llid_16:
    {
        uint32_t cfgllid16;
        err = ag_drv_lif_llid_16_get(&cfgllid16);
        bdmf_session_print(session, "cfgllid16 = %u = 0x%x\n", cfgllid16, cfgllid16);
        break;
    }
    case BDMF_llid_17:
    {
        uint32_t cfgllid17;
        err = ag_drv_lif_llid_17_get(&cfgllid17);
        bdmf_session_print(session, "cfgllid17 = %u = 0x%x\n", cfgllid17, cfgllid17);
        break;
    }
    case BDMF_llid_18:
    {
        uint32_t cfgllid18;
        err = ag_drv_lif_llid_18_get(&cfgllid18);
        bdmf_session_print(session, "cfgllid18 = %u = 0x%x\n", cfgllid18, cfgllid18);
        break;
    }
    case BDMF_llid_19:
    {
        uint32_t cfgllid19;
        err = ag_drv_lif_llid_19_get(&cfgllid19);
        bdmf_session_print(session, "cfgllid19 = %u = 0x%x\n", cfgllid19, cfgllid19);
        break;
    }
    case BDMF_llid_20:
    {
        uint32_t cfgllid20;
        err = ag_drv_lif_llid_20_get(&cfgllid20);
        bdmf_session_print(session, "cfgllid20 = %u = 0x%x\n", cfgllid20, cfgllid20);
        break;
    }
    case BDMF_llid_21:
    {
        uint32_t cfgllid21;
        err = ag_drv_lif_llid_21_get(&cfgllid21);
        bdmf_session_print(session, "cfgllid21 = %u = 0x%x\n", cfgllid21, cfgllid21);
        break;
    }
    case BDMF_llid_22:
    {
        uint32_t cfgllid22;
        err = ag_drv_lif_llid_22_get(&cfgllid22);
        bdmf_session_print(session, "cfgllid22 = %u = 0x%x\n", cfgllid22, cfgllid22);
        break;
    }
    case BDMF_llid_23:
    {
        uint32_t cfgllid23;
        err = ag_drv_lif_llid_23_get(&cfgllid23);
        bdmf_session_print(session, "cfgllid23 = %u = 0x%x\n", cfgllid23, cfgllid23);
        break;
    }
    case BDMF_time_ref_cnt:
    {
        uint8_t cffullupdatevalue;
        uint8_t cfmaxnegvalue;
        uint8_t cfmaxposvalue;
        err = ag_drv_lif_time_ref_cnt_get(&cffullupdatevalue, &cfmaxnegvalue, &cfmaxposvalue);
        bdmf_session_print(session, "cffullupdatevalue = %u = 0x%x\n", cffullupdatevalue, cffullupdatevalue);
        bdmf_session_print(session, "cfmaxnegvalue = %u = 0x%x\n", cfmaxnegvalue, cfmaxnegvalue);
        bdmf_session_print(session, "cfmaxposvalue = %u = 0x%x\n", cfmaxposvalue, cfmaxposvalue);
        break;
    }
    case BDMF_timestamp_upd_per:
    {
        uint16_t cftimestampupdper;
        err = ag_drv_lif_timestamp_upd_per_get(&cftimestampupdper);
        bdmf_session_print(session, "cftimestampupdper = %u = 0x%x\n", cftimestampupdper, cftimestampupdper);
        break;
    }
    case BDMF_tp_time:
    {
        uint32_t cftransporttime;
        err = ag_drv_lif_tp_time_get(&cftransporttime);
        bdmf_session_print(session, "cftransporttime = %u = 0x%x\n", cftransporttime, cftransporttime);
        break;
    }
    case BDMF_mpcp_time:
    {
        uint32_t ltmpcptime;
        err = ag_drv_lif_mpcp_time_get(&ltmpcptime);
        bdmf_session_print(session, "ltmpcptime = %u = 0x%x\n", ltmpcptime, ltmpcptime);
        break;
    }
    case BDMF_maxlen_ctr:
    {
        uint16_t cfrxmaxframelength;
        err = ag_drv_lif_maxlen_ctr_get(&cfrxmaxframelength);
        bdmf_session_print(session, "cfrxmaxframelength = %u = 0x%x\n", cfrxmaxframelength, cfrxmaxframelength);
        break;
    }
    case BDMF_laser_on_delta:
    {
        uint16_t cftxlaserondelta;
        err = ag_drv_lif_laser_on_delta_get(&cftxlaserondelta);
        bdmf_session_print(session, "cftxlaserondelta = %u = 0x%x\n", cftxlaserondelta, cftxlaserondelta);
        break;
    }
    case BDMF_laser_off_idle:
    {
        uint16_t cftxinitidle;
        uint8_t cftxlaseroffdelta;
        err = ag_drv_lif_laser_off_idle_get(&cftxinitidle, &cftxlaseroffdelta);
        bdmf_session_print(session, "cftxinitidle = %u = 0x%x\n", cftxinitidle, cftxinitidle);
        bdmf_session_print(session, "cftxlaseroffdelta = %u = 0x%x\n", cftxlaseroffdelta, cftxlaseroffdelta);
        break;
    }
    case BDMF_fec_init_idle:
    {
        uint16_t cftxfecinitidle;
        err = ag_drv_lif_fec_init_idle_get(&cftxfecinitidle);
        bdmf_session_print(session, "cftxfecinitidle = %u = 0x%x\n", cftxfecinitidle, cftxfecinitidle);
        break;
    }
    case BDMF_fec_err_allow:
    {
        uint8_t cfrxtfecbiterrallow;
        uint8_t cfrxsfecbiterrallow;
        err = ag_drv_lif_fec_err_allow_get(&cfrxtfecbiterrallow, &cfrxsfecbiterrallow);
        bdmf_session_print(session, "cfrxtfecbiterrallow = %u = 0x%x\n", cfrxtfecbiterrallow, cfrxtfecbiterrallow);
        bdmf_session_print(session, "cfrxsfecbiterrallow = %u = 0x%x\n", cfrxsfecbiterrallow, cfrxsfecbiterrallow);
        break;
    }
    case BDMF_sec_key_sel:
    {
        bdmf_boolean data;
        err = ag_drv_lif_sec_key_sel_get(parm[1].value.unumber, &data);
        bdmf_session_print(session, "data = %u = 0x%x\n", data, data);
        break;
    }
    case BDMF_dn_encrypt_stat:
    {
        bdmf_boolean enEncrypt;
        err = ag_drv_lif_dn_encrypt_stat_get(parm[1].value.unumber, &enEncrypt);
        bdmf_session_print(session, "enEncrypt = %u = 0x%x\n", enEncrypt, enEncrypt);
        break;
    }
    case BDMF_sec_up_key_stat:
    {
        bdmf_boolean keyUpSel;
        err = ag_drv_lif_sec_up_key_stat_get(parm[1].value.unumber, &keyUpSel);
        bdmf_session_print(session, "keyUpSel = %u = 0x%x\n", keyUpSel, keyUpSel);
        break;
    }
    case BDMF_sec_up_encrypt_stat:
    {
        bdmf_boolean enUpEncrypt;
        err = ag_drv_lif_sec_up_encrypt_stat_get(parm[1].value.unumber, &enUpEncrypt);
        bdmf_session_print(session, "enUpEncrypt = %u = 0x%x\n", enUpEncrypt, enUpEncrypt);
        break;
    }
    case BDMF_sec_up_mpcp_offset:
    {
        uint32_t secupmpcpoffset;
        err = ag_drv_lif_sec_up_mpcp_offset_get(&secupmpcpoffset);
        bdmf_session_print(session, "secupmpcpoffset = %u = 0x%x\n", secupmpcpoffset, secupmpcpoffset);
        break;
    }
    case BDMF_fec_per_llid:
    {
        bdmf_boolean cfFecTxFecEnLlid;
        err = ag_drv_lif_fec_per_llid_get(parm[1].value.unumber, &cfFecTxFecEnLlid);
        bdmf_session_print(session, "cfFecTxFecEnLlid = %u = 0x%x\n", cfFecTxFecEnLlid, cfFecTxFecEnLlid);
        break;
    }
    case BDMF_rx_line_code_err_cnt:
    {
        uint32_t rxlinecodeerrcnt;
        err = ag_drv_lif_rx_line_code_err_cnt_get(&rxlinecodeerrcnt);
        bdmf_session_print(session, "rxlinecodeerrcnt = %u = 0x%x\n", rxlinecodeerrcnt, rxlinecodeerrcnt);
        break;
    }
    case BDMF_rx_agg_mpcp_frm:
    {
        uint32_t rxaggmpcpcnt;
        err = ag_drv_lif_rx_agg_mpcp_frm_get(&rxaggmpcpcnt);
        bdmf_session_print(session, "rxaggmpcpcnt = %u = 0x%x\n", rxaggmpcpcnt, rxaggmpcpcnt);
        break;
    }
    case BDMF_rx_agg_good_frm:
    {
        uint32_t rxagggoodcnt;
        err = ag_drv_lif_rx_agg_good_frm_get(&rxagggoodcnt);
        bdmf_session_print(session, "rxagggoodcnt = %u = 0x%x\n", rxagggoodcnt, rxagggoodcnt);
        break;
    }
    case BDMF_rx_agg_good_byte:
    {
        uint32_t rxagggoodbytescnt;
        err = ag_drv_lif_rx_agg_good_byte_get(&rxagggoodbytescnt);
        bdmf_session_print(session, "rxagggoodbytescnt = %u = 0x%x\n", rxagggoodbytescnt, rxagggoodbytescnt);
        break;
    }
    case BDMF_rx_agg_undersz_frm:
    {
        uint32_t rxaggunderszcnt;
        err = ag_drv_lif_rx_agg_undersz_frm_get(&rxaggunderszcnt);
        bdmf_session_print(session, "rxaggunderszcnt = %u = 0x%x\n", rxaggunderszcnt, rxaggunderszcnt);
        break;
    }
    case BDMF_rx_agg_oversz_frm:
    {
        uint32_t rxaggoverszcnt;
        err = ag_drv_lif_rx_agg_oversz_frm_get(&rxaggoverszcnt);
        bdmf_session_print(session, "rxaggoverszcnt = %u = 0x%x\n", rxaggoverszcnt, rxaggoverszcnt);
        break;
    }
    case BDMF_rx_agg_crc8_frm:
    {
        uint32_t rxaggcrc8errcnt;
        err = ag_drv_lif_rx_agg_crc8_frm_get(&rxaggcrc8errcnt);
        bdmf_session_print(session, "rxaggcrc8errcnt = %u = 0x%x\n", rxaggcrc8errcnt, rxaggcrc8errcnt);
        break;
    }
    case BDMF_rx_agg_fec_frm:
    {
        uint32_t rxaggfec;
        err = ag_drv_lif_rx_agg_fec_frm_get(&rxaggfec);
        bdmf_session_print(session, "rxaggfec = %u = 0x%x\n", rxaggfec, rxaggfec);
        break;
    }
    case BDMF_rx_agg_fec_byte:
    {
        uint32_t rxaggfecbytes;
        err = ag_drv_lif_rx_agg_fec_byte_get(&rxaggfecbytes);
        bdmf_session_print(session, "rxaggfecbytes = %u = 0x%x\n", rxaggfecbytes, rxaggfecbytes);
        break;
    }
    case BDMF_rx_agg_fec_exc_err_frm:
    {
        uint32_t rxaggfecexceederrs;
        err = ag_drv_lif_rx_agg_fec_exc_err_frm_get(&rxaggfecexceederrs);
        bdmf_session_print(session, "rxaggfecexceederrs = %u = 0x%x\n", rxaggfecexceederrs, rxaggfecexceederrs);
        break;
    }
    case BDMF_rx_agg_nonfec_good_frm:
    {
        uint32_t rxaggnonfecgood;
        err = ag_drv_lif_rx_agg_nonfec_good_frm_get(&rxaggnonfecgood);
        bdmf_session_print(session, "rxaggnonfecgood = %u = 0x%x\n", rxaggnonfecgood, rxaggnonfecgood);
        break;
    }
    case BDMF_rx_agg_nonfec_good_byte:
    {
        uint32_t rxaggnonfecgoodbytes;
        err = ag_drv_lif_rx_agg_nonfec_good_byte_get(&rxaggnonfecgoodbytes);
        bdmf_session_print(session, "rxaggnonfecgoodbytes = %u = 0x%x\n", rxaggnonfecgoodbytes, rxaggnonfecgoodbytes);
        break;
    }
    case BDMF_rx_agg_err_bytes:
    {
        uint32_t rxaggerrbytes;
        err = ag_drv_lif_rx_agg_err_bytes_get(&rxaggerrbytes);
        bdmf_session_print(session, "rxaggerrbytes = %u = 0x%x\n", rxaggerrbytes, rxaggerrbytes);
        break;
    }
    case BDMF_rx_agg_err_zeroes:
    {
        uint32_t rxaggerrzeroes;
        err = ag_drv_lif_rx_agg_err_zeroes_get(&rxaggerrzeroes);
        bdmf_session_print(session, "rxaggerrzeroes = %u = 0x%x\n", rxaggerrzeroes, rxaggerrzeroes);
        break;
    }
    case BDMF_rx_agg_no_err_blks:
    {
        uint32_t rxaggnoerrblks;
        err = ag_drv_lif_rx_agg_no_err_blks_get(&rxaggnoerrblks);
        bdmf_session_print(session, "rxaggnoerrblks = %u = 0x%x\n", rxaggnoerrblks, rxaggnoerrblks);
        break;
    }
    case BDMF_rx_agg_cor_blks:
    {
        uint32_t rxaggcorrblks;
        err = ag_drv_lif_rx_agg_cor_blks_get(&rxaggcorrblks);
        bdmf_session_print(session, "rxaggcorrblks = %u = 0x%x\n", rxaggcorrblks, rxaggcorrblks);
        break;
    }
    case BDMF_rx_agg_uncor_blks:
    {
        uint32_t rxagguncorrblks;
        err = ag_drv_lif_rx_agg_uncor_blks_get(&rxagguncorrblks);
        bdmf_session_print(session, "rxagguncorrblks = %u = 0x%x\n", rxagguncorrblks, rxagguncorrblks);
        break;
    }
    case BDMF_rx_agg_err_ones:
    {
        uint32_t rxaggerrones;
        err = ag_drv_lif_rx_agg_err_ones_get(&rxaggerrones);
        bdmf_session_print(session, "rxaggerrones = %u = 0x%x\n", rxaggerrones, rxaggerrones);
        break;
    }
    case BDMF_rx_agg_err_frm:
    {
        uint32_t rxaggerroredcnt;
        err = ag_drv_lif_rx_agg_err_frm_get(&rxaggerroredcnt);
        bdmf_session_print(session, "rxaggerroredcnt = %u = 0x%x\n", rxaggerroredcnt, rxaggerroredcnt);
        break;
    }
    case BDMF_tx_pkt_cnt:
    {
        uint32_t txframecnt;
        err = ag_drv_lif_tx_pkt_cnt_get(&txframecnt);
        bdmf_session_print(session, "txframecnt = %u = 0x%x\n", txframecnt, txframecnt);
        break;
    }
    case BDMF_tx_byte_cnt:
    {
        uint32_t txbytecnt;
        err = ag_drv_lif_tx_byte_cnt_get(&txbytecnt);
        bdmf_session_print(session, "txbytecnt = %u = 0x%x\n", txbytecnt, txbytecnt);
        break;
    }
    case BDMF_tx_non_fec_pkt_cnt:
    {
        uint32_t txnonfecframecnt;
        err = ag_drv_lif_tx_non_fec_pkt_cnt_get(&txnonfecframecnt);
        bdmf_session_print(session, "txnonfecframecnt = %u = 0x%x\n", txnonfecframecnt, txnonfecframecnt);
        break;
    }
    case BDMF_tx_non_fec_byte_cnt:
    {
        uint32_t txnonfecbytecnt;
        err = ag_drv_lif_tx_non_fec_byte_cnt_get(&txnonfecbytecnt);
        bdmf_session_print(session, "txnonfecbytecnt = %u = 0x%x\n", txnonfecbytecnt, txnonfecbytecnt);
        break;
    }
    case BDMF_tx_fec_pkt_cnt:
    {
        uint32_t txfecframecnt;
        err = ag_drv_lif_tx_fec_pkt_cnt_get(&txfecframecnt);
        bdmf_session_print(session, "txfecframecnt = %u = 0x%x\n", txfecframecnt, txfecframecnt);
        break;
    }
    case BDMF_tx_fec_byte_cnt:
    {
        uint32_t txfecbytecnt;
        err = ag_drv_lif_tx_fec_byte_cnt_get(&txfecbytecnt);
        bdmf_session_print(session, "txfecbytecnt = %u = 0x%x\n", txfecbytecnt, txfecbytecnt);
        break;
    }
    case BDMF_tx_fec_blk_cnt:
    {
        uint32_t txfecblkscnt;
        err = ag_drv_lif_tx_fec_blk_cnt_get(&txfecblkscnt);
        bdmf_session_print(session, "txfecblkscnt = %u = 0x%x\n", txfecblkscnt, txfecblkscnt);
        break;
    }
    case BDMF_tx_mpcp_pkt_cnt:
    {
        uint32_t txmpcpframecnt;
        err = ag_drv_lif_tx_mpcp_pkt_cnt_get(&txmpcpframecnt);
        bdmf_session_print(session, "txmpcpframecnt = %u = 0x%x\n", txmpcpframecnt, txmpcpframecnt);
        break;
    }
    case BDMF_debug_tx_data_pkt_cnt:
    {
        uint32_t txdataframecnt;
        err = ag_drv_lif_debug_tx_data_pkt_cnt_get(&txdataframecnt);
        bdmf_session_print(session, "txdataframecnt = %u = 0x%x\n", txdataframecnt, txdataframecnt);
        break;
    }
    case BDMF_fec_llid_status:
    {
        uint32_t stkyfecrevcllidbmsk;
        err = ag_drv_lif_fec_llid_status_get(&stkyfecrevcllidbmsk);
        bdmf_session_print(session, "stkyfecrevcllidbmsk = %u = 0x%x\n", stkyfecrevcllidbmsk, stkyfecrevcllidbmsk);
        break;
    }
    case BDMF_sec_rx_tek_ig_iv_llid:
    {
        uint32_t cfigivnullllid;
        err = ag_drv_lif_sec_rx_tek_ig_iv_llid_get(&cfigivnullllid);
        bdmf_session_print(session, "cfigivnullllid = %u = 0x%x\n", cfigivnullllid, cfigivnullllid);
        break;
    }
    case BDMF_pon_ber_interv_thresh:
    {
        uint32_t cfrxlifberinterval;
        uint16_t cfrxlifberthreshld;
        uint8_t cfrxlifbercntrl;
        err = ag_drv_lif_pon_ber_interv_thresh_get(&cfrxlifberinterval, &cfrxlifberthreshld, &cfrxlifbercntrl);
        bdmf_session_print(session, "cfrxlifberinterval = %u = 0x%x\n", cfrxlifberinterval, cfrxlifberinterval);
        bdmf_session_print(session, "cfrxlifberthreshld = %u = 0x%x\n", cfrxlifberthreshld, cfrxlifberthreshld);
        bdmf_session_print(session, "cfrxlifbercntrl = %u = 0x%x\n", cfrxlifbercntrl, cfrxlifbercntrl);
        break;
    }
    case BDMF_lsr_mon_a_ctrl:
    {
        bdmf_boolean iopbilaserens1a;
        bdmf_boolean cfglsrmonacthi;
        bdmf_boolean pbilasermonrsta_n_pre;
        err = ag_drv_lif_lsr_mon_a_ctrl_get(&iopbilaserens1a, &cfglsrmonacthi, &pbilasermonrsta_n_pre);
        bdmf_session_print(session, "iopbilaserens1a = %u = 0x%x\n", iopbilaserens1a, iopbilaserens1a);
        bdmf_session_print(session, "cfglsrmonacthi = %u = 0x%x\n", cfglsrmonacthi, cfglsrmonacthi);
        bdmf_session_print(session, "pbilasermonrsta_n_pre = %u = 0x%x\n", pbilasermonrsta_n_pre, pbilasermonrsta_n_pre);
        break;
    }
    case BDMF_lsr_mon_a_max_thr:
    {
        uint32_t cfglasermonmaxa;
        err = ag_drv_lif_lsr_mon_a_max_thr_get(&cfglasermonmaxa);
        bdmf_session_print(session, "cfglasermonmaxa = %u = 0x%x\n", cfglasermonmaxa, cfglasermonmaxa);
        break;
    }
    case BDMF_lsr_mon_a_bst_len:
    {
        uint32_t laserontimea;
        err = ag_drv_lif_lsr_mon_a_bst_len_get(&laserontimea);
        bdmf_session_print(session, "laserontimea = %u = 0x%x\n", laserontimea, laserontimea);
        break;
    }
    case BDMF_lsr_mon_a_bst_cnt:
    {
        uint32_t lasermonbrstcnta;
        err = ag_drv_lif_lsr_mon_a_bst_cnt_get(&lasermonbrstcnta);
        bdmf_session_print(session, "lasermonbrstcnta = %u = 0x%x\n", lasermonbrstcnta, lasermonbrstcnta);
        break;
    }
    case BDMF_debug_pon_sm:
    {
        uint8_t aligncsqq;
        uint8_t rxfecifcsqq;
        err = ag_drv_lif_debug_pon_sm_get(&aligncsqq, &rxfecifcsqq);
        bdmf_session_print(session, "aligncsqq = %u = 0x%x\n", aligncsqq, aligncsqq);
        bdmf_session_print(session, "rxfecifcsqq = %u = 0x%x\n", rxfecifcsqq, rxfecifcsqq);
        break;
    }
    case BDMF_debug_fec_sm:
    {
        uint8_t rxsyncsqq;
        uint8_t rxcorcs;
        uint8_t fecrxoutcs;
        err = ag_drv_lif_debug_fec_sm_get(&rxsyncsqq, &rxcorcs, &fecrxoutcs);
        bdmf_session_print(session, "rxsyncsqq = %u = 0x%x\n", rxsyncsqq, rxsyncsqq);
        bdmf_session_print(session, "rxcorcs = %u = 0x%x\n", rxcorcs, rxcorcs);
        bdmf_session_print(session, "fecrxoutcs = %u = 0x%x\n", fecrxoutcs, fecrxoutcs);
        break;
    }
    case BDMF_ae_pktnum_window:
    {
        uint32_t cfgaepktnumwnd;
        err = ag_drv_lif_ae_pktnum_window_get(&cfgaepktnumwnd);
        bdmf_session_print(session, "cfgaepktnumwnd = %u = 0x%x\n", cfgaepktnumwnd, cfgaepktnumwnd);
        break;
    }
    case BDMF_ae_pktnum_thresh:
    {
        uint32_t cfgpktnummaxthresh;
        err = ag_drv_lif_ae_pktnum_thresh_get(&cfgpktnummaxthresh);
        bdmf_session_print(session, "cfgpktnummaxthresh = %u = 0x%x\n", cfgpktnummaxthresh, cfgpktnummaxthresh);
        break;
    }
    case BDMF_ae_pktnum_stat:
    {
        uint8_t secupindxwtpktnummax;
        uint8_t secdnindxwtpktnumabort;
        err = ag_drv_lif_ae_pktnum_stat_get(&secupindxwtpktnummax, &secdnindxwtpktnumabort);
        bdmf_session_print(session, "secupindxwtpktnummax = %u = 0x%x\n", secupindxwtpktnummax, secupindxwtpktnummax);
        bdmf_session_print(session, "secdnindxwtpktnumabort = %u = 0x%x\n", secdnindxwtpktnumabort, secdnindxwtpktnumabort);
        break;
    }
    case BDMF_llid_8:
    {
        uint32_t cfgllid8;
        err = ag_drv_lif_llid_8_get(&cfgllid8);
        bdmf_session_print(session, "cfgllid8 = %u = 0x%x\n", cfgllid8, cfgllid8);
        break;
    }
    case BDMF_llid_9:
    {
        uint32_t cfgllid9;
        err = ag_drv_lif_llid_9_get(&cfgllid9);
        bdmf_session_print(session, "cfgllid9 = %u = 0x%x\n", cfgllid9, cfgllid9);
        break;
    }
    case BDMF_llid_10:
    {
        uint32_t cfgllid10;
        err = ag_drv_lif_llid_10_get(&cfgllid10);
        bdmf_session_print(session, "cfgllid10 = %u = 0x%x\n", cfgllid10, cfgllid10);
        break;
    }
    case BDMF_llid_11:
    {
        uint32_t cfgllid11;
        err = ag_drv_lif_llid_11_get(&cfgllid11);
        bdmf_session_print(session, "cfgllid11 = %u = 0x%x\n", cfgllid11, cfgllid11);
        break;
    }
    case BDMF_llid_12:
    {
        uint32_t cfgllid12;
        err = ag_drv_lif_llid_12_get(&cfgllid12);
        bdmf_session_print(session, "cfgllid12 = %u = 0x%x\n", cfgllid12, cfgllid12);
        break;
    }
    case BDMF_llid_13:
    {
        uint32_t cfgllid13;
        err = ag_drv_lif_llid_13_get(&cfgllid13);
        bdmf_session_print(session, "cfgllid13 = %u = 0x%x\n", cfgllid13, cfgllid13);
        break;
    }
    case BDMF_llid_14:
    {
        uint32_t cfgllid14;
        err = ag_drv_lif_llid_14_get(&cfgllid14);
        bdmf_session_print(session, "cfgllid14 = %u = 0x%x\n", cfgllid14, cfgllid14);
        break;
    }
    case BDMF_llid_15:
    {
        uint32_t cfgllid15;
        err = ag_drv_lif_llid_15_get(&cfgllid15);
        bdmf_session_print(session, "cfgllid15 = %u = 0x%x\n", cfgllid15, cfgllid15);
        break;
    }
    case BDMF_llid_24:
    {
        uint32_t cfgllid24;
        err = ag_drv_lif_llid_24_get(&cfgllid24);
        bdmf_session_print(session, "cfgllid24 = %u = 0x%x\n", cfgllid24, cfgllid24);
        break;
    }
    case BDMF_llid_25:
    {
        uint32_t cfgllid25;
        err = ag_drv_lif_llid_25_get(&cfgllid25);
        bdmf_session_print(session, "cfgllid25 = %u = 0x%x\n", cfgllid25, cfgllid25);
        break;
    }
    case BDMF_llid_26:
    {
        uint32_t cfgllid26;
        err = ag_drv_lif_llid_26_get(&cfgllid26);
        bdmf_session_print(session, "cfgllid26 = %u = 0x%x\n", cfgllid26, cfgllid26);
        break;
    }
    case BDMF_llid_27:
    {
        uint32_t cfgllid27;
        err = ag_drv_lif_llid_27_get(&cfgllid27);
        bdmf_session_print(session, "cfgllid27 = %u = 0x%x\n", cfgllid27, cfgllid27);
        break;
    }
    case BDMF_llid_28:
    {
        uint32_t cfgllid28;
        err = ag_drv_lif_llid_28_get(&cfgllid28);
        bdmf_session_print(session, "cfgllid28 = %u = 0x%x\n", cfgllid28, cfgllid28);
        break;
    }
    case BDMF_llid_29:
    {
        uint32_t cfgllid29;
        err = ag_drv_lif_llid_29_get(&cfgllid29);
        bdmf_session_print(session, "cfgllid29 = %u = 0x%x\n", cfgllid29, cfgllid29);
        break;
    }
    case BDMF_llid_30:
    {
        uint32_t cfgllid30;
        err = ag_drv_lif_llid_30_get(&cfgllid30);
        bdmf_session_print(session, "cfgllid30 = %u = 0x%x\n", cfgllid30, cfgllid30);
        break;
    }
    case BDMF_llid_31:
    {
        uint32_t cfgllid31;
        err = ag_drv_lif_llid_31_get(&cfgllid31);
        bdmf_session_print(session, "cfgllid31 = %u = 0x%x\n", cfgllid31, cfgllid31);
        break;
    }
    case BDMF_vlan_type:
    {
        uint16_t cfgvlantype;
        err = ag_drv_lif_vlan_type_get(&cfgvlantype);
        bdmf_session_print(session, "cfgvlantype = %u = 0x%x\n", cfgvlantype, cfgvlantype);
        break;
    }
    case BDMF_p2p_ae_sci_en:
    {
        uint16_t cfgp2pscien;
        err = ag_drv_lif_p2p_ae_sci_en_get(&cfgp2pscien);
        bdmf_session_print(session, "cfgp2pscien = %u = 0x%x\n", cfgp2pscien, cfgp2pscien);
        break;
    }
    case BDMF_p2p_ae_sci_lo_0:
    {
        uint32_t cfgp2psci_lo_0;
        err = ag_drv_lif_p2p_ae_sci_lo_0_get(&cfgp2psci_lo_0);
        bdmf_session_print(session, "cfgp2psci_lo_0 = %u = 0x%x\n", cfgp2psci_lo_0, cfgp2psci_lo_0);
        break;
    }
    case BDMF_p2p_ae_sci_hi_0:
    {
        uint32_t cfgp2psci_hi_0;
        err = ag_drv_lif_p2p_ae_sci_hi_0_get(&cfgp2psci_hi_0);
        bdmf_session_print(session, "cfgp2psci_hi_0 = %u = 0x%x\n", cfgp2psci_hi_0, cfgp2psci_hi_0);
        break;
    }
    case BDMF_p2p_ae_sci_lo_1:
    {
        uint32_t cfgp2psci_lo_1;
        err = ag_drv_lif_p2p_ae_sci_lo_1_get(&cfgp2psci_lo_1);
        bdmf_session_print(session, "cfgp2psci_lo_1 = %u = 0x%x\n", cfgp2psci_lo_1, cfgp2psci_lo_1);
        break;
    }
    case BDMF_p2p_ae_sci_hi_1:
    {
        uint32_t cfgp2psci_hi_1;
        err = ag_drv_lif_p2p_ae_sci_hi_1_get(&cfgp2psci_hi_1);
        bdmf_session_print(session, "cfgp2psci_hi_1 = %u = 0x%x\n", cfgp2psci_hi_1, cfgp2psci_hi_1);
        break;
    }
    case BDMF_p2p_ae_sci_lo_2:
    {
        uint32_t cfgp2psci_lo_2;
        err = ag_drv_lif_p2p_ae_sci_lo_2_get(&cfgp2psci_lo_2);
        bdmf_session_print(session, "cfgp2psci_lo_2 = %u = 0x%x\n", cfgp2psci_lo_2, cfgp2psci_lo_2);
        break;
    }
    case BDMF_p2p_ae_sci_hi_2:
    {
        uint32_t cfgp2psci_hi_2;
        err = ag_drv_lif_p2p_ae_sci_hi_2_get(&cfgp2psci_hi_2);
        bdmf_session_print(session, "cfgp2psci_hi_2 = %u = 0x%x\n", cfgp2psci_hi_2, cfgp2psci_hi_2);
        break;
    }
    case BDMF_p2p_ae_sci_lo_3:
    {
        uint32_t cfgp2psci_lo_3;
        err = ag_drv_lif_p2p_ae_sci_lo_3_get(&cfgp2psci_lo_3);
        bdmf_session_print(session, "cfgp2psci_lo_3 = %u = 0x%x\n", cfgp2psci_lo_3, cfgp2psci_lo_3);
        break;
    }
    case BDMF_p2p_ae_sci_hi_3:
    {
        uint32_t cfgp2psci_hi_3;
        err = ag_drv_lif_p2p_ae_sci_hi_3_get(&cfgp2psci_hi_3);
        bdmf_session_print(session, "cfgp2psci_hi_3 = %u = 0x%x\n", cfgp2psci_hi_3, cfgp2psci_hi_3);
        break;
    }
    case BDMF_p2p_ae_sci_lo_4:
    {
        uint32_t cfgp2psci_lo_4;
        err = ag_drv_lif_p2p_ae_sci_lo_4_get(&cfgp2psci_lo_4);
        bdmf_session_print(session, "cfgp2psci_lo_4 = %u = 0x%x\n", cfgp2psci_lo_4, cfgp2psci_lo_4);
        break;
    }
    case BDMF_p2p_ae_sci_hi_4:
    {
        uint32_t cfgp2psci_hi_4;
        err = ag_drv_lif_p2p_ae_sci_hi_4_get(&cfgp2psci_hi_4);
        bdmf_session_print(session, "cfgp2psci_hi_4 = %u = 0x%x\n", cfgp2psci_hi_4, cfgp2psci_hi_4);
        break;
    }
    case BDMF_p2p_ae_sci_lo_5:
    {
        uint32_t cfgp2psci_lo_5;
        err = ag_drv_lif_p2p_ae_sci_lo_5_get(&cfgp2psci_lo_5);
        bdmf_session_print(session, "cfgp2psci_lo_5 = %u = 0x%x\n", cfgp2psci_lo_5, cfgp2psci_lo_5);
        break;
    }
    case BDMF_p2p_ae_sci_hi_5:
    {
        uint32_t cfgp2psci_hi_5;
        err = ag_drv_lif_p2p_ae_sci_hi_5_get(&cfgp2psci_hi_5);
        bdmf_session_print(session, "cfgp2psci_hi_5 = %u = 0x%x\n", cfgp2psci_hi_5, cfgp2psci_hi_5);
        break;
    }
    case BDMF_p2p_ae_sci_lo_6:
    {
        uint32_t cfgp2psci_lo_6;
        err = ag_drv_lif_p2p_ae_sci_lo_6_get(&cfgp2psci_lo_6);
        bdmf_session_print(session, "cfgp2psci_lo_6 = %u = 0x%x\n", cfgp2psci_lo_6, cfgp2psci_lo_6);
        break;
    }
    case BDMF_p2p_ae_sci_hi_6:
    {
        uint32_t cfgp2psci_hi_6;
        err = ag_drv_lif_p2p_ae_sci_hi_6_get(&cfgp2psci_hi_6);
        bdmf_session_print(session, "cfgp2psci_hi_6 = %u = 0x%x\n", cfgp2psci_hi_6, cfgp2psci_hi_6);
        break;
    }
    case BDMF_p2p_ae_sci_lo_7:
    {
        uint32_t cfgp2psci_lo_7;
        err = ag_drv_lif_p2p_ae_sci_lo_7_get(&cfgp2psci_lo_7);
        bdmf_session_print(session, "cfgp2psci_lo_7 = %u = 0x%x\n", cfgp2psci_lo_7, cfgp2psci_lo_7);
        break;
    }
    case BDMF_p2p_ae_sci_hi_7:
    {
        uint32_t cfgp2psci_hi_7;
        err = ag_drv_lif_p2p_ae_sci_hi_7_get(&cfgp2psci_hi_7);
        bdmf_session_print(session, "cfgp2psci_hi_7 = %u = 0x%x\n", cfgp2psci_hi_7, cfgp2psci_hi_7);
        break;
    }
    case BDMF_p2p_ae_sci_lo_8:
    {
        uint32_t cfgp2psci_lo_8;
        err = ag_drv_lif_p2p_ae_sci_lo_8_get(&cfgp2psci_lo_8);
        bdmf_session_print(session, "cfgp2psci_lo_8 = %u = 0x%x\n", cfgp2psci_lo_8, cfgp2psci_lo_8);
        break;
    }
    case BDMF_p2p_ae_sci_hi_8:
    {
        uint32_t cfgp2psci_hi_8;
        err = ag_drv_lif_p2p_ae_sci_hi_8_get(&cfgp2psci_hi_8);
        bdmf_session_print(session, "cfgp2psci_hi_8 = %u = 0x%x\n", cfgp2psci_hi_8, cfgp2psci_hi_8);
        break;
    }
    case BDMF_p2p_ae_sci_lo_9:
    {
        uint32_t cfgp2psci_lo_9;
        err = ag_drv_lif_p2p_ae_sci_lo_9_get(&cfgp2psci_lo_9);
        bdmf_session_print(session, "cfgp2psci_lo_9 = %u = 0x%x\n", cfgp2psci_lo_9, cfgp2psci_lo_9);
        break;
    }
    case BDMF_p2p_ae_sci_hi_9:
    {
        uint32_t cfgp2psci_hi_9;
        err = ag_drv_lif_p2p_ae_sci_hi_9_get(&cfgp2psci_hi_9);
        bdmf_session_print(session, "cfgp2psci_hi_9 = %u = 0x%x\n", cfgp2psci_hi_9, cfgp2psci_hi_9);
        break;
    }
    case BDMF_p2p_ae_sci_lo_10:
    {
        uint32_t cfgp2psci_lo_10;
        err = ag_drv_lif_p2p_ae_sci_lo_10_get(&cfgp2psci_lo_10);
        bdmf_session_print(session, "cfgp2psci_lo_10 = %u = 0x%x\n", cfgp2psci_lo_10, cfgp2psci_lo_10);
        break;
    }
    case BDMF_p2p_ae_sci_hi_10:
    {
        uint32_t cfgp2psci_hi_10;
        err = ag_drv_lif_p2p_ae_sci_hi_10_get(&cfgp2psci_hi_10);
        bdmf_session_print(session, "cfgp2psci_hi_10 = %u = 0x%x\n", cfgp2psci_hi_10, cfgp2psci_hi_10);
        break;
    }
    case BDMF_p2p_ae_sci_lo_11:
    {
        uint32_t cfgp2psci_lo_11;
        err = ag_drv_lif_p2p_ae_sci_lo_11_get(&cfgp2psci_lo_11);
        bdmf_session_print(session, "cfgp2psci_lo_11 = %u = 0x%x\n", cfgp2psci_lo_11, cfgp2psci_lo_11);
        break;
    }
    case BDMF_p2p_ae_sci_hi_11:
    {
        uint32_t cfgp2psci_hi_11;
        err = ag_drv_lif_p2p_ae_sci_hi_11_get(&cfgp2psci_hi_11);
        bdmf_session_print(session, "cfgp2psci_hi_11 = %u = 0x%x\n", cfgp2psci_hi_11, cfgp2psci_hi_11);
        break;
    }
    case BDMF_p2p_ae_sci_lo_12:
    {
        uint32_t cfgp2psci_lo_12;
        err = ag_drv_lif_p2p_ae_sci_lo_12_get(&cfgp2psci_lo_12);
        bdmf_session_print(session, "cfgp2psci_lo_12 = %u = 0x%x\n", cfgp2psci_lo_12, cfgp2psci_lo_12);
        break;
    }
    case BDMF_p2p_ae_sci_hi_12:
    {
        uint32_t cfgp2psci_hi_12;
        err = ag_drv_lif_p2p_ae_sci_hi_12_get(&cfgp2psci_hi_12);
        bdmf_session_print(session, "cfgp2psci_hi_12 = %u = 0x%x\n", cfgp2psci_hi_12, cfgp2psci_hi_12);
        break;
    }
    case BDMF_p2p_ae_sci_lo_13:
    {
        uint32_t cfgp2psci_lo_13;
        err = ag_drv_lif_p2p_ae_sci_lo_13_get(&cfgp2psci_lo_13);
        bdmf_session_print(session, "cfgp2psci_lo_13 = %u = 0x%x\n", cfgp2psci_lo_13, cfgp2psci_lo_13);
        break;
    }
    case BDMF_p2p_ae_sci_hi_13:
    {
        uint32_t cfgp2psci_hi_13;
        err = ag_drv_lif_p2p_ae_sci_hi_13_get(&cfgp2psci_hi_13);
        bdmf_session_print(session, "cfgp2psci_hi_13 = %u = 0x%x\n", cfgp2psci_hi_13, cfgp2psci_hi_13);
        break;
    }
    case BDMF_p2p_ae_sci_lo_14:
    {
        uint32_t cfgp2psci_lo_14;
        err = ag_drv_lif_p2p_ae_sci_lo_14_get(&cfgp2psci_lo_14);
        bdmf_session_print(session, "cfgp2psci_lo_14 = %u = 0x%x\n", cfgp2psci_lo_14, cfgp2psci_lo_14);
        break;
    }
    case BDMF_p2p_ae_sci_hi_14:
    {
        uint32_t cfgp2psci_hi_14;
        err = ag_drv_lif_p2p_ae_sci_hi_14_get(&cfgp2psci_hi_14);
        bdmf_session_print(session, "cfgp2psci_hi_14 = %u = 0x%x\n", cfgp2psci_hi_14, cfgp2psci_hi_14);
        break;
    }
    case BDMF_p2p_ae_sci_lo_15:
    {
        uint32_t cfgp2psci_lo_15;
        err = ag_drv_lif_p2p_ae_sci_lo_15_get(&cfgp2psci_lo_15);
        bdmf_session_print(session, "cfgp2psci_lo_15 = %u = 0x%x\n", cfgp2psci_lo_15, cfgp2psci_lo_15);
        break;
    }
    case BDMF_p2p_ae_sci_hi_15:
    {
        uint32_t cfgp2psci_hi_15;
        err = ag_drv_lif_p2p_ae_sci_hi_15_get(&cfgp2psci_hi_15);
        bdmf_session_print(session, "cfgp2psci_hi_15 = %u = 0x%x\n", cfgp2psci_hi_15, cfgp2psci_hi_15);
        break;
    }
    case BDMF_sec_up_key_stat_1:
    {
        uint32_t keyupsel_hi;
        err = ag_drv_lif_sec_up_key_stat_1_get(&keyupsel_hi);
        bdmf_session_print(session, "keyupsel_hi = %u = 0x%x\n", keyupsel_hi, keyupsel_hi);
        break;
    }
    case BDMF_sec_key_sel_1:
    {
        uint32_t keysel_hi;
        err = ag_drv_lif_sec_key_sel_1_get(&keysel_hi);
        bdmf_session_print(session, "keysel_hi = %u = 0x%x\n", keysel_hi, keysel_hi);
        break;
    }
    case BDMF_pon_sec_tx_plaintxt_ae_pad_control:
    {
        uint8_t cf_plaintxt_oh_b_idle_pad;
        uint8_t cf_plaintxt_oh_a_idle_pad;
        err = ag_drv_lif_pon_sec_tx_plaintxt_ae_pad_control_get(&cf_plaintxt_oh_b_idle_pad, &cf_plaintxt_oh_a_idle_pad);
        bdmf_session_print(session, "cf_plaintxt_oh_b_idle_pad = %u = 0x%x\n", cf_plaintxt_oh_b_idle_pad, cf_plaintxt_oh_b_idle_pad);
        bdmf_session_print(session, "cf_plaintxt_oh_a_idle_pad = %u = 0x%x\n", cf_plaintxt_oh_a_idle_pad, cf_plaintxt_oh_a_idle_pad);
        break;
    }
    case BDMF_p2p_autoneg_control:
    {
        uint16_t cf_autoneg_linktimer;
        bdmf_boolean cf_autoneg_mode_sel;
        bdmf_boolean cf_autoneg_restart;
        bdmf_boolean cf_autoneg_en;
        err = ag_drv_lif_p2p_autoneg_control_get(&cf_autoneg_linktimer, &cf_autoneg_mode_sel, &cf_autoneg_restart, &cf_autoneg_en);
        bdmf_session_print(session, "cf_autoneg_linktimer = %u = 0x%x\n", cf_autoneg_linktimer, cf_autoneg_linktimer);
        bdmf_session_print(session, "cf_autoneg_mode_sel = %u = 0x%x\n", cf_autoneg_mode_sel, cf_autoneg_mode_sel);
        bdmf_session_print(session, "cf_autoneg_restart = %u = 0x%x\n", cf_autoneg_restart, cf_autoneg_restart);
        bdmf_session_print(session, "cf_autoneg_en = %u = 0x%x\n", cf_autoneg_en, cf_autoneg_en);
        break;
    }
    case BDMF_p2p_autoneg_status:
    {
        bdmf_boolean an_lp_remote_fault;
        bdmf_boolean an_sync_status;
        bdmf_boolean an_complete;
        err = ag_drv_lif_p2p_autoneg_status_get(&an_lp_remote_fault, &an_sync_status, &an_complete);
        bdmf_session_print(session, "an_lp_remote_fault = %u = 0x%x\n", an_lp_remote_fault, an_lp_remote_fault);
        bdmf_session_print(session, "an_sync_status = %u = 0x%x\n", an_sync_status, an_sync_status);
        bdmf_session_print(session, "an_complete = %u = 0x%x\n", an_complete, an_complete);
        break;
    }
    case BDMF_p2p_autoneg_ability_config_reg:
    {
        uint16_t cf_lif_p2p_ae_autoneg_config_ability;
        err = ag_drv_lif_p2p_autoneg_ability_config_reg_get(&cf_lif_p2p_ae_autoneg_config_ability);
        bdmf_session_print(session, "cf_lif_p2p_ae_autoneg_config_ability = %u = 0x%x\n", cf_lif_p2p_ae_autoneg_config_ability, cf_lif_p2p_ae_autoneg_config_ability);
        break;
    }
    case BDMF_p2p_autoneg_link_partner_ability_config_read:
    {
        uint16_t cf_lif_p2p_ae_autoneg_lp_ability_read;
        err = ag_drv_lif_p2p_autoneg_link_partner_ability_config_read_get(&cf_lif_p2p_ae_autoneg_lp_ability_read);
        bdmf_session_print(session, "cf_lif_p2p_ae_autoneg_lp_ability_read = %u = 0x%x\n", cf_lif_p2p_ae_autoneg_lp_ability_read, cf_lif_p2p_ae_autoneg_lp_ability_read);
        break;
    }
    default:
        err = BDMF_ERR_NOT_SUPPORTED;
        break;
    }
    return err;
}

static int bcm_lif_cli_test(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    bdmf_test_method m = parm[0].value.unumber;
    bdmf_error_t err = BDMF_ERR_OK;

    {
        lif_pon_control pon_control = {.cfgdisruntfilter=gtmv(m, 1), .cfmaxcommaerrcnt=gtmv(m, 4), .cfsyncsmselect=gtmv(m, 2), .cfponrxforcenonfecabort=gtmv(m, 1), .cfponrxforcefecabort=gtmv(m, 1), .cfgrxdatabitflip=gtmv(m, 1), .cfgenmpcpoampad=gtmv(m, 1), .cfgentxidlepkt=gtmv(m, 1), .cfenablesoftwaresynchold=gtmv(m, 1), .cfenableextendsync=gtmv(m, 1), .cfenablequicksync=gtmv(m, 1), .cfppsen=gtmv(m, 1), .cfppsclkrbc=gtmv(m, 1), .cfrx2txlpback=gtmv(m, 1), .cftx2rxlpback=gtmv(m, 1), .cftxdataendurlon=gtmv(m, 1), .cfp2pmode=gtmv(m, 1), .cfp2pshortpre=gtmv(m, 1), .cflaseren=gtmv(m, 1), .cftxlaseron=gtmv(m, 1), .cftxlaseronacthi=gtmv(m, 1), .liftxrstn_pre=gtmv(m, 1), .lifrxrstn_pre=gtmv(m, 1), .liftxen=gtmv(m, 1), .lifrxen=gtmv(m, 1)};
        if(!err) bdmf_session_print(session, "ag_drv_lif_pon_control_set( %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u)\n", pon_control.cfgdisruntfilter, pon_control.cfmaxcommaerrcnt, pon_control.cfsyncsmselect, pon_control.cfponrxforcenonfecabort, pon_control.cfponrxforcefecabort, pon_control.cfgrxdatabitflip, pon_control.cfgenmpcpoampad, pon_control.cfgentxidlepkt, pon_control.cfenablesoftwaresynchold, pon_control.cfenableextendsync, pon_control.cfenablequicksync, pon_control.cfppsen, pon_control.cfppsclkrbc, pon_control.cfrx2txlpback, pon_control.cftx2rxlpback, pon_control.cftxdataendurlon, pon_control.cfp2pmode, pon_control.cfp2pshortpre, pon_control.cflaseren, pon_control.cftxlaseron, pon_control.cftxlaseronacthi, pon_control.liftxrstn_pre, pon_control.lifrxrstn_pre, pon_control.liftxen, pon_control.lifrxen);
        if(!err) ag_drv_lif_pon_control_set(&pon_control);
        if(!err) ag_drv_lif_pon_control_get( &pon_control);
        if(!err) bdmf_session_print(session, "ag_drv_lif_pon_control_get( %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u)\n", pon_control.cfgdisruntfilter, pon_control.cfmaxcommaerrcnt, pon_control.cfsyncsmselect, pon_control.cfponrxforcenonfecabort, pon_control.cfponrxforcefecabort, pon_control.cfgrxdatabitflip, pon_control.cfgenmpcpoampad, pon_control.cfgentxidlepkt, pon_control.cfenablesoftwaresynchold, pon_control.cfenableextendsync, pon_control.cfenablequicksync, pon_control.cfppsen, pon_control.cfppsclkrbc, pon_control.cfrx2txlpback, pon_control.cftx2rxlpback, pon_control.cftxdataendurlon, pon_control.cfp2pmode, pon_control.cfp2pshortpre, pon_control.cflaseren, pon_control.cftxlaseron, pon_control.cftxlaseronacthi, pon_control.liftxrstn_pre, pon_control.lifrxrstn_pre, pon_control.liftxen, pon_control.lifrxen);
        if(err || pon_control.cfgdisruntfilter!=gtmv(m, 1) || pon_control.cfmaxcommaerrcnt!=gtmv(m, 4) || pon_control.cfsyncsmselect!=gtmv(m, 2) || pon_control.cfponrxforcenonfecabort!=gtmv(m, 1) || pon_control.cfponrxforcefecabort!=gtmv(m, 1) || pon_control.cfgrxdatabitflip!=gtmv(m, 1) || pon_control.cfgenmpcpoampad!=gtmv(m, 1) || pon_control.cfgentxidlepkt!=gtmv(m, 1) || pon_control.cfenablesoftwaresynchold!=gtmv(m, 1) || pon_control.cfenableextendsync!=gtmv(m, 1) || pon_control.cfenablequicksync!=gtmv(m, 1) || pon_control.cfppsen!=gtmv(m, 1) || pon_control.cfppsclkrbc!=gtmv(m, 1) || pon_control.cfrx2txlpback!=gtmv(m, 1) || pon_control.cftx2rxlpback!=gtmv(m, 1) || pon_control.cftxdataendurlon!=gtmv(m, 1) || pon_control.cfp2pmode!=gtmv(m, 1) || pon_control.cfp2pshortpre!=gtmv(m, 1) || pon_control.cflaseren!=gtmv(m, 1) || pon_control.cftxlaseron!=gtmv(m, 1) || pon_control.cftxlaseronacthi!=gtmv(m, 1) || pon_control.liftxrstn_pre!=gtmv(m, 1) || pon_control.lifrxrstn_pre!=gtmv(m, 1) || pon_control.liftxen!=gtmv(m, 1) || pon_control.lifrxen!=gtmv(m, 1))
            return err ? err : BDMF_ERR_IO;
    }
    {
        lif_pon_inter_op_control pon_inter_op_control = {.cfipgfilter=gtmv(m, 4), .cfdisableloslaserblock=gtmv(m, 1), .cfgllidpromiscuousmode=gtmv(m, 1), .cfgllidmodmsk=gtmv(m, 1), .cfusefecipg=gtmv(m, 1), .cfrxcrc8invchk=gtmv(m, 1), .cfrxcrc8bitswap=gtmv(m, 1), .cfrxcrc8msb2lsb=gtmv(m, 1), .cfrxcrc8disable=gtmv(m, 1), .cftxllidbit15set=gtmv(m, 1), .cftxcrc8inv=gtmv(m, 1), .cftxcrc8bad=gtmv(m, 1), .cftxcrc8bitswap=gtmv(m, 1), .cftxcrc8msb2lsb=gtmv(m, 1), .cftxshortpre=gtmv(m, 1), .cftxipgcnt=gtmv(m, 4), .cftxaasynclen=gtmv(m, 4), .cftxpipedelay=gtmv(m, 4)};
        if(!err) bdmf_session_print(session, "ag_drv_lif_pon_inter_op_control_set( %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u)\n", pon_inter_op_control.cfipgfilter, pon_inter_op_control.cfdisableloslaserblock, pon_inter_op_control.cfgllidpromiscuousmode, pon_inter_op_control.cfgllidmodmsk, pon_inter_op_control.cfusefecipg, pon_inter_op_control.cfrxcrc8invchk, pon_inter_op_control.cfrxcrc8bitswap, pon_inter_op_control.cfrxcrc8msb2lsb, pon_inter_op_control.cfrxcrc8disable, pon_inter_op_control.cftxllidbit15set, pon_inter_op_control.cftxcrc8inv, pon_inter_op_control.cftxcrc8bad, pon_inter_op_control.cftxcrc8bitswap, pon_inter_op_control.cftxcrc8msb2lsb, pon_inter_op_control.cftxshortpre, pon_inter_op_control.cftxipgcnt, pon_inter_op_control.cftxaasynclen, pon_inter_op_control.cftxpipedelay);
        if(!err) ag_drv_lif_pon_inter_op_control_set(&pon_inter_op_control);
        if(!err) ag_drv_lif_pon_inter_op_control_get( &pon_inter_op_control);
        if(!err) bdmf_session_print(session, "ag_drv_lif_pon_inter_op_control_get( %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u)\n", pon_inter_op_control.cfipgfilter, pon_inter_op_control.cfdisableloslaserblock, pon_inter_op_control.cfgllidpromiscuousmode, pon_inter_op_control.cfgllidmodmsk, pon_inter_op_control.cfusefecipg, pon_inter_op_control.cfrxcrc8invchk, pon_inter_op_control.cfrxcrc8bitswap, pon_inter_op_control.cfrxcrc8msb2lsb, pon_inter_op_control.cfrxcrc8disable, pon_inter_op_control.cftxllidbit15set, pon_inter_op_control.cftxcrc8inv, pon_inter_op_control.cftxcrc8bad, pon_inter_op_control.cftxcrc8bitswap, pon_inter_op_control.cftxcrc8msb2lsb, pon_inter_op_control.cftxshortpre, pon_inter_op_control.cftxipgcnt, pon_inter_op_control.cftxaasynclen, pon_inter_op_control.cftxpipedelay);
        if(err || pon_inter_op_control.cfipgfilter!=gtmv(m, 4) || pon_inter_op_control.cfdisableloslaserblock!=gtmv(m, 1) || pon_inter_op_control.cfgllidpromiscuousmode!=gtmv(m, 1) || pon_inter_op_control.cfgllidmodmsk!=gtmv(m, 1) || pon_inter_op_control.cfusefecipg!=gtmv(m, 1) || pon_inter_op_control.cfrxcrc8invchk!=gtmv(m, 1) || pon_inter_op_control.cfrxcrc8bitswap!=gtmv(m, 1) || pon_inter_op_control.cfrxcrc8msb2lsb!=gtmv(m, 1) || pon_inter_op_control.cfrxcrc8disable!=gtmv(m, 1) || pon_inter_op_control.cftxllidbit15set!=gtmv(m, 1) || pon_inter_op_control.cftxcrc8inv!=gtmv(m, 1) || pon_inter_op_control.cftxcrc8bad!=gtmv(m, 1) || pon_inter_op_control.cftxcrc8bitswap!=gtmv(m, 1) || pon_inter_op_control.cftxcrc8msb2lsb!=gtmv(m, 1) || pon_inter_op_control.cftxshortpre!=gtmv(m, 1) || pon_inter_op_control.cftxipgcnt!=gtmv(m, 4) || pon_inter_op_control.cftxaasynclen!=gtmv(m, 4) || pon_inter_op_control.cftxpipedelay!=gtmv(m, 4))
            return err ? err : BDMF_ERR_IO;
    }
    {
        lif_fec_control fec_control = {.cffecrxerrorprop=gtmv(m, 1), .cffecrxforcenonfecabort=gtmv(m, 1), .cffecrxforcefecabort=gtmv(m, 1), .cffecrxenable=gtmv(m, 1), .cffectxfecperllid=gtmv(m, 1), .cffectxenable=gtmv(m, 1)};
        if(!err) bdmf_session_print(session, "ag_drv_lif_fec_control_set( %u %u %u %u %u %u)\n", fec_control.cffecrxerrorprop, fec_control.cffecrxforcenonfecabort, fec_control.cffecrxforcefecabort, fec_control.cffecrxenable, fec_control.cffectxfecperllid, fec_control.cffectxenable);
        if(!err) ag_drv_lif_fec_control_set(&fec_control);
        if(!err) ag_drv_lif_fec_control_get( &fec_control);
        if(!err) bdmf_session_print(session, "ag_drv_lif_fec_control_get( %u %u %u %u %u %u)\n", fec_control.cffecrxerrorprop, fec_control.cffecrxforcenonfecabort, fec_control.cffecrxforcefecabort, fec_control.cffecrxenable, fec_control.cffectxfecperllid, fec_control.cffectxenable);
        if(err || fec_control.cffecrxerrorprop!=gtmv(m, 1) || fec_control.cffecrxforcenonfecabort!=gtmv(m, 1) || fec_control.cffecrxforcefecabort!=gtmv(m, 1) || fec_control.cffecrxenable!=gtmv(m, 1) || fec_control.cffectxfecperllid!=gtmv(m, 1) || fec_control.cffectxenable!=gtmv(m, 1))
            return err ? err : BDMF_ERR_IO;
    }
    {
        lif_sec_control sec_control = {.cfgdismpcpencrypt=gtmv(m, 1), .cfgdisoamencrypt=gtmv(m, 1), .cfgsecenshortlen=gtmv(m, 1), .cfgenaereplayprct=gtmv(m, 1), .cfgenlegacyrcc=gtmv(m, 1), .enfakeupaes=gtmv(m, 1), .enfakednaes=gtmv(m, 1), .cfgfecipglen=gtmv(m, 8), .disdndasaencrpt=gtmv(m, 1), .entriplechurn=gtmv(m, 1), .enepnmixencrypt=gtmv(m, 1), .disupdasaencrpt=gtmv(m, 1), .secupencryptscheme=gtmv(m, 2), .secdnencryptscheme=gtmv(m, 3), .secuprstn_pre=gtmv(m, 1), .secdnrstn_pre=gtmv(m, 1), .secenup=gtmv(m, 1), .secendn=gtmv(m, 1)};
        if(!err) bdmf_session_print(session, "ag_drv_lif_sec_control_set( %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u)\n", sec_control.cfgdismpcpencrypt, sec_control.cfgdisoamencrypt, sec_control.cfgsecenshortlen, sec_control.cfgenaereplayprct, sec_control.cfgenlegacyrcc, sec_control.enfakeupaes, sec_control.enfakednaes, sec_control.cfgfecipglen, sec_control.disdndasaencrpt, sec_control.entriplechurn, sec_control.enepnmixencrypt, sec_control.disupdasaencrpt, sec_control.secupencryptscheme, sec_control.secdnencryptscheme, sec_control.secuprstn_pre, sec_control.secdnrstn_pre, sec_control.secenup, sec_control.secendn);
        if(!err) ag_drv_lif_sec_control_set(&sec_control);
        if(!err) ag_drv_lif_sec_control_get( &sec_control);
        if(!err) bdmf_session_print(session, "ag_drv_lif_sec_control_get( %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u)\n", sec_control.cfgdismpcpencrypt, sec_control.cfgdisoamencrypt, sec_control.cfgsecenshortlen, sec_control.cfgenaereplayprct, sec_control.cfgenlegacyrcc, sec_control.enfakeupaes, sec_control.enfakednaes, sec_control.cfgfecipglen, sec_control.disdndasaencrpt, sec_control.entriplechurn, sec_control.enepnmixencrypt, sec_control.disupdasaencrpt, sec_control.secupencryptscheme, sec_control.secdnencryptscheme, sec_control.secuprstn_pre, sec_control.secdnrstn_pre, sec_control.secenup, sec_control.secendn);
        if(err || sec_control.cfgdismpcpencrypt!=gtmv(m, 1) || sec_control.cfgdisoamencrypt!=gtmv(m, 1) || sec_control.cfgsecenshortlen!=gtmv(m, 1) || sec_control.cfgenaereplayprct!=gtmv(m, 1) || sec_control.cfgenlegacyrcc!=gtmv(m, 1) || sec_control.enfakeupaes!=gtmv(m, 1) || sec_control.enfakednaes!=gtmv(m, 1) || sec_control.cfgfecipglen!=gtmv(m, 8) || sec_control.disdndasaencrpt!=gtmv(m, 1) || sec_control.entriplechurn!=gtmv(m, 1) || sec_control.enepnmixencrypt!=gtmv(m, 1) || sec_control.disupdasaencrpt!=gtmv(m, 1) || sec_control.secupencryptscheme!=gtmv(m, 2) || sec_control.secdnencryptscheme!=gtmv(m, 3) || sec_control.secuprstn_pre!=gtmv(m, 1) || sec_control.secdnrstn_pre!=gtmv(m, 1) || sec_control.secenup!=gtmv(m, 1) || sec_control.secendn!=gtmv(m, 1))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint16_t cfgmacsecethertype=gtmv(m, 16);
        if(!err) bdmf_session_print(session, "ag_drv_lif_macsec_set( %u)\n", cfgmacsecethertype);
        if(!err) ag_drv_lif_macsec_set(cfgmacsecethertype);
        if(!err) ag_drv_lif_macsec_get( &cfgmacsecethertype);
        if(!err) bdmf_session_print(session, "ag_drv_lif_macsec_get( %u)\n", cfgmacsecethertype);
        if(err || cfgmacsecethertype!=gtmv(m, 16))
            return err ? err : BDMF_ERR_IO;
    }
    {
        lif_int_status int_status = {.int_sop_sfec_ipg_violation=gtmv(m, 1), .laseronmax=gtmv(m, 1), .laseroff=gtmv(m, 1), .secdnreplayprotctabort=gtmv(m, 1), .secuppktnumoverflow=gtmv(m, 1), .intlaseroffdurburst=gtmv(m, 1), .intrxberthreshexc=gtmv(m, 1), .intfecrxfecrecvstatus=gtmv(m, 1), .intfecrxcorerrfifofullstatus=gtmv(m, 1), .intfecrxcorerrfifounexpempty=gtmv(m, 1), .intfecbufpopemptypush=gtmv(m, 1), .intfecbufpopemptynopush=gtmv(m, 1), .intfecbufpushfull=gtmv(m, 1), .intuptimefullupdstat=gtmv(m, 1), .intfroutofalignstat=gtmv(m, 1), .intgrntstarttimelagstat=gtmv(m, 1), .intabortrxfrmstat=gtmv(m, 1), .intnorxclkstat=gtmv(m, 1), .intrxmaxlenerrstat=gtmv(m, 1), .intrxerraftalignstat=gtmv(m, 1), .intrxsynchacqstat=gtmv(m, 1), .intrxoutofsynchstat=gtmv(m, 1)};
        if(!err) bdmf_session_print(session, "ag_drv_lif_int_status_set( %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u)\n", int_status.int_sop_sfec_ipg_violation, int_status.laseronmax, int_status.laseroff, int_status.secdnreplayprotctabort, int_status.secuppktnumoverflow, int_status.intlaseroffdurburst, int_status.intrxberthreshexc, int_status.intfecrxfecrecvstatus, int_status.intfecrxcorerrfifofullstatus, int_status.intfecrxcorerrfifounexpempty, int_status.intfecbufpopemptypush, int_status.intfecbufpopemptynopush, int_status.intfecbufpushfull, int_status.intuptimefullupdstat, int_status.intfroutofalignstat, int_status.intgrntstarttimelagstat, int_status.intabortrxfrmstat, int_status.intnorxclkstat, int_status.intrxmaxlenerrstat, int_status.intrxerraftalignstat, int_status.intrxsynchacqstat, int_status.intrxoutofsynchstat);
        if(!err) ag_drv_lif_int_status_set(&int_status);
        if(!err) ag_drv_lif_int_status_get( &int_status);
        if(!err) bdmf_session_print(session, "ag_drv_lif_int_status_get( %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u)\n", int_status.int_sop_sfec_ipg_violation, int_status.laseronmax, int_status.laseroff, int_status.secdnreplayprotctabort, int_status.secuppktnumoverflow, int_status.intlaseroffdurburst, int_status.intrxberthreshexc, int_status.intfecrxfecrecvstatus, int_status.intfecrxcorerrfifofullstatus, int_status.intfecrxcorerrfifounexpempty, int_status.intfecbufpopemptypush, int_status.intfecbufpopemptynopush, int_status.intfecbufpushfull, int_status.intuptimefullupdstat, int_status.intfroutofalignstat, int_status.intgrntstarttimelagstat, int_status.intabortrxfrmstat, int_status.intnorxclkstat, int_status.intrxmaxlenerrstat, int_status.intrxerraftalignstat, int_status.intrxsynchacqstat, int_status.intrxoutofsynchstat);
        if(err || int_status.int_sop_sfec_ipg_violation!=gtmv(m, 1) || int_status.laseronmax!=gtmv(m, 1) || int_status.laseroff!=gtmv(m, 1) || int_status.secdnreplayprotctabort!=gtmv(m, 1) || int_status.secuppktnumoverflow!=gtmv(m, 1) || int_status.intlaseroffdurburst!=gtmv(m, 1) || int_status.intrxberthreshexc!=gtmv(m, 1) || int_status.intfecrxfecrecvstatus!=gtmv(m, 1) || int_status.intfecrxcorerrfifofullstatus!=gtmv(m, 1) || int_status.intfecrxcorerrfifounexpempty!=gtmv(m, 1) || int_status.intfecbufpopemptypush!=gtmv(m, 1) || int_status.intfecbufpopemptynopush!=gtmv(m, 1) || int_status.intfecbufpushfull!=gtmv(m, 1) || int_status.intuptimefullupdstat!=gtmv(m, 1) || int_status.intfroutofalignstat!=gtmv(m, 1) || int_status.intgrntstarttimelagstat!=gtmv(m, 1) || int_status.intabortrxfrmstat!=gtmv(m, 1) || int_status.intnorxclkstat!=gtmv(m, 1) || int_status.intrxmaxlenerrstat!=gtmv(m, 1) || int_status.intrxerraftalignstat!=gtmv(m, 1) || int_status.intrxsynchacqstat!=gtmv(m, 1) || int_status.intrxoutofsynchstat!=gtmv(m, 1))
            return err ? err : BDMF_ERR_IO;
    }
    {
        lif_int_mask int_mask = {.int_sop_sfec_ipg_violation_mask=gtmv(m, 1), .laseronmaxmask=gtmv(m, 1), .laseroffmask=gtmv(m, 1), .secdnreplayprotctabortmsk=gtmv(m, 1), .secuppktnumoverflowmsk=gtmv(m, 1), .intlaseroffdurburstmask=gtmv(m, 1), .intrxberthreshexcmask=gtmv(m, 1), .intfecrxfecrecvmask=gtmv(m, 1), .intfecrxcorerrfifofullmask=gtmv(m, 1), .intfecrxcorerrfifounexpemptymask=gtmv(m, 1), .intfecbufpopemptypushmask=gtmv(m, 1), .intfecbufpopemptynopushmask=gtmv(m, 1), .intfecbufpushfullmask=gtmv(m, 1), .intuptimefullupdmask=gtmv(m, 1), .intfroutofalignmask=gtmv(m, 1), .intgrntstarttimelagmask=gtmv(m, 1), .intabortrxfrmmask=gtmv(m, 1), .intnorxclkmask=gtmv(m, 1), .intrxmaxlenerrmask=gtmv(m, 1), .intrxerraftalignmask=gtmv(m, 1), .intrxsynchacqmask=gtmv(m, 1), .intrxoutofsynchmask=gtmv(m, 1)};
        if(!err) bdmf_session_print(session, "ag_drv_lif_int_mask_set( %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u)\n", int_mask.int_sop_sfec_ipg_violation_mask, int_mask.laseronmaxmask, int_mask.laseroffmask, int_mask.secdnreplayprotctabortmsk, int_mask.secuppktnumoverflowmsk, int_mask.intlaseroffdurburstmask, int_mask.intrxberthreshexcmask, int_mask.intfecrxfecrecvmask, int_mask.intfecrxcorerrfifofullmask, int_mask.intfecrxcorerrfifounexpemptymask, int_mask.intfecbufpopemptypushmask, int_mask.intfecbufpopemptynopushmask, int_mask.intfecbufpushfullmask, int_mask.intuptimefullupdmask, int_mask.intfroutofalignmask, int_mask.intgrntstarttimelagmask, int_mask.intabortrxfrmmask, int_mask.intnorxclkmask, int_mask.intrxmaxlenerrmask, int_mask.intrxerraftalignmask, int_mask.intrxsynchacqmask, int_mask.intrxoutofsynchmask);
        if(!err) ag_drv_lif_int_mask_set(&int_mask);
        if(!err) ag_drv_lif_int_mask_get( &int_mask);
        if(!err) bdmf_session_print(session, "ag_drv_lif_int_mask_get( %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u)\n", int_mask.int_sop_sfec_ipg_violation_mask, int_mask.laseronmaxmask, int_mask.laseroffmask, int_mask.secdnreplayprotctabortmsk, int_mask.secuppktnumoverflowmsk, int_mask.intlaseroffdurburstmask, int_mask.intrxberthreshexcmask, int_mask.intfecrxfecrecvmask, int_mask.intfecrxcorerrfifofullmask, int_mask.intfecrxcorerrfifounexpemptymask, int_mask.intfecbufpopemptypushmask, int_mask.intfecbufpopemptynopushmask, int_mask.intfecbufpushfullmask, int_mask.intuptimefullupdmask, int_mask.intfroutofalignmask, int_mask.intgrntstarttimelagmask, int_mask.intabortrxfrmmask, int_mask.intnorxclkmask, int_mask.intrxmaxlenerrmask, int_mask.intrxerraftalignmask, int_mask.intrxsynchacqmask, int_mask.intrxoutofsynchmask);
        if(err || int_mask.int_sop_sfec_ipg_violation_mask!=gtmv(m, 1) || int_mask.laseronmaxmask!=gtmv(m, 1) || int_mask.laseroffmask!=gtmv(m, 1) || int_mask.secdnreplayprotctabortmsk!=gtmv(m, 1) || int_mask.secuppktnumoverflowmsk!=gtmv(m, 1) || int_mask.intlaseroffdurburstmask!=gtmv(m, 1) || int_mask.intrxberthreshexcmask!=gtmv(m, 1) || int_mask.intfecrxfecrecvmask!=gtmv(m, 1) || int_mask.intfecrxcorerrfifofullmask!=gtmv(m, 1) || int_mask.intfecrxcorerrfifounexpemptymask!=gtmv(m, 1) || int_mask.intfecbufpopemptypushmask!=gtmv(m, 1) || int_mask.intfecbufpopemptynopushmask!=gtmv(m, 1) || int_mask.intfecbufpushfullmask!=gtmv(m, 1) || int_mask.intuptimefullupdmask!=gtmv(m, 1) || int_mask.intfroutofalignmask!=gtmv(m, 1) || int_mask.intgrntstarttimelagmask!=gtmv(m, 1) || int_mask.intabortrxfrmmask!=gtmv(m, 1) || int_mask.intnorxclkmask!=gtmv(m, 1) || int_mask.intrxmaxlenerrmask!=gtmv(m, 1) || int_mask.intrxerraftalignmask!=gtmv(m, 1) || int_mask.intrxsynchacqmask!=gtmv(m, 1) || int_mask.intrxoutofsynchmask!=gtmv(m, 1))
            return err ? err : BDMF_ERR_IO;
    }
    {
        lif_data_port_command data_port_command = {.data_port_busy=gtmv(m, 1), .data_port_error=gtmv(m, 1), .ram_select=gtmv(m, 6), .data_port_op_code=gtmv(m, 8), .data_port_addr=gtmv(m, 16)};
        if(!err) bdmf_session_print(session, "ag_drv_lif_data_port_command_set( %u %u %u %u %u)\n", data_port_command.data_port_busy, data_port_command.data_port_error, data_port_command.ram_select, data_port_command.data_port_op_code, data_port_command.data_port_addr);
        if(!err) ag_drv_lif_data_port_command_set(&data_port_command);
        if(!err) ag_drv_lif_data_port_command_get( &data_port_command);
        if(!err) bdmf_session_print(session, "ag_drv_lif_data_port_command_get( %u %u %u %u %u)\n", data_port_command.data_port_busy, data_port_command.data_port_error, data_port_command.ram_select, data_port_command.data_port_op_code, data_port_command.data_port_addr);
        if(err || data_port_command.data_port_busy!=gtmv(m, 1) || data_port_command.data_port_error!=gtmv(m, 1) || data_port_command.ram_select!=gtmv(m, 6) || data_port_command.data_port_op_code!=gtmv(m, 8) || data_port_command.data_port_addr!=gtmv(m, 16))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint8_t portidx=gtmv(m, 3);
        uint32_t pbiportdata=gtmv(m, 32);
        if(!err) bdmf_session_print(session, "ag_drv_lif_data_port_data_set( %u %u)\n", portidx, pbiportdata);
        if(!err) ag_drv_lif_data_port_data_set(portidx, pbiportdata);
        if(!err) ag_drv_lif_data_port_data_get( portidx, &pbiportdata);
        if(!err) bdmf_session_print(session, "ag_drv_lif_data_port_data_get( %u %u)\n", portidx, pbiportdata);
        if(err || pbiportdata!=gtmv(m, 32))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint32_t cfgllid0=gtmv(m, 17);
        if(!err) bdmf_session_print(session, "ag_drv_lif_llid_0_set( %u)\n", cfgllid0);
        if(!err) ag_drv_lif_llid_0_set(cfgllid0);
        if(!err) ag_drv_lif_llid_0_get( &cfgllid0);
        if(!err) bdmf_session_print(session, "ag_drv_lif_llid_0_get( %u)\n", cfgllid0);
        if(err || cfgllid0!=gtmv(m, 17))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint32_t cfgllid1=gtmv(m, 17);
        if(!err) bdmf_session_print(session, "ag_drv_lif_llid_1_set( %u)\n", cfgllid1);
        if(!err) ag_drv_lif_llid_1_set(cfgllid1);
        if(!err) ag_drv_lif_llid_1_get( &cfgllid1);
        if(!err) bdmf_session_print(session, "ag_drv_lif_llid_1_get( %u)\n", cfgllid1);
        if(err || cfgllid1!=gtmv(m, 17))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint32_t cfgllid2=gtmv(m, 17);
        if(!err) bdmf_session_print(session, "ag_drv_lif_llid_2_set( %u)\n", cfgllid2);
        if(!err) ag_drv_lif_llid_2_set(cfgllid2);
        if(!err) ag_drv_lif_llid_2_get( &cfgllid2);
        if(!err) bdmf_session_print(session, "ag_drv_lif_llid_2_get( %u)\n", cfgllid2);
        if(err || cfgllid2!=gtmv(m, 17))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint32_t cfgllid3=gtmv(m, 17);
        if(!err) bdmf_session_print(session, "ag_drv_lif_llid_3_set( %u)\n", cfgllid3);
        if(!err) ag_drv_lif_llid_3_set(cfgllid3);
        if(!err) ag_drv_lif_llid_3_get( &cfgllid3);
        if(!err) bdmf_session_print(session, "ag_drv_lif_llid_3_get( %u)\n", cfgllid3);
        if(err || cfgllid3!=gtmv(m, 17))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint32_t cfgllid4=gtmv(m, 17);
        if(!err) bdmf_session_print(session, "ag_drv_lif_llid_4_set( %u)\n", cfgllid4);
        if(!err) ag_drv_lif_llid_4_set(cfgllid4);
        if(!err) ag_drv_lif_llid_4_get( &cfgllid4);
        if(!err) bdmf_session_print(session, "ag_drv_lif_llid_4_get( %u)\n", cfgllid4);
        if(err || cfgllid4!=gtmv(m, 17))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint32_t cfgllid5=gtmv(m, 17);
        if(!err) bdmf_session_print(session, "ag_drv_lif_llid_5_set( %u)\n", cfgllid5);
        if(!err) ag_drv_lif_llid_5_set(cfgllid5);
        if(!err) ag_drv_lif_llid_5_get( &cfgllid5);
        if(!err) bdmf_session_print(session, "ag_drv_lif_llid_5_get( %u)\n", cfgllid5);
        if(err || cfgllid5!=gtmv(m, 17))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint32_t cfgllid6=gtmv(m, 17);
        if(!err) bdmf_session_print(session, "ag_drv_lif_llid_6_set( %u)\n", cfgllid6);
        if(!err) ag_drv_lif_llid_6_set(cfgllid6);
        if(!err) ag_drv_lif_llid_6_get( &cfgllid6);
        if(!err) bdmf_session_print(session, "ag_drv_lif_llid_6_get( %u)\n", cfgllid6);
        if(err || cfgllid6!=gtmv(m, 17))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint32_t cfgllid7=gtmv(m, 17);
        if(!err) bdmf_session_print(session, "ag_drv_lif_llid_7_set( %u)\n", cfgllid7);
        if(!err) ag_drv_lif_llid_7_set(cfgllid7);
        if(!err) ag_drv_lif_llid_7_get( &cfgllid7);
        if(!err) bdmf_session_print(session, "ag_drv_lif_llid_7_get( %u)\n", cfgllid7);
        if(err || cfgllid7!=gtmv(m, 17))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint32_t cfgllid16=gtmv(m, 17);
        if(!err) bdmf_session_print(session, "ag_drv_lif_llid_16_set( %u)\n", cfgllid16);
        if(!err) ag_drv_lif_llid_16_set(cfgllid16);
        if(!err) ag_drv_lif_llid_16_get( &cfgllid16);
        if(!err) bdmf_session_print(session, "ag_drv_lif_llid_16_get( %u)\n", cfgllid16);
        if(err || cfgllid16!=gtmv(m, 17))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint32_t cfgllid17=gtmv(m, 17);
        if(!err) bdmf_session_print(session, "ag_drv_lif_llid_17_set( %u)\n", cfgllid17);
        if(!err) ag_drv_lif_llid_17_set(cfgllid17);
        if(!err) ag_drv_lif_llid_17_get( &cfgllid17);
        if(!err) bdmf_session_print(session, "ag_drv_lif_llid_17_get( %u)\n", cfgllid17);
        if(err || cfgllid17!=gtmv(m, 17))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint32_t cfgllid18=gtmv(m, 17);
        if(!err) bdmf_session_print(session, "ag_drv_lif_llid_18_set( %u)\n", cfgllid18);
        if(!err) ag_drv_lif_llid_18_set(cfgllid18);
        if(!err) ag_drv_lif_llid_18_get( &cfgllid18);
        if(!err) bdmf_session_print(session, "ag_drv_lif_llid_18_get( %u)\n", cfgllid18);
        if(err || cfgllid18!=gtmv(m, 17))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint32_t cfgllid19=gtmv(m, 17);
        if(!err) bdmf_session_print(session, "ag_drv_lif_llid_19_set( %u)\n", cfgllid19);
        if(!err) ag_drv_lif_llid_19_set(cfgllid19);
        if(!err) ag_drv_lif_llid_19_get( &cfgllid19);
        if(!err) bdmf_session_print(session, "ag_drv_lif_llid_19_get( %u)\n", cfgllid19);
        if(err || cfgllid19!=gtmv(m, 17))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint32_t cfgllid20=gtmv(m, 17);
        if(!err) bdmf_session_print(session, "ag_drv_lif_llid_20_set( %u)\n", cfgllid20);
        if(!err) ag_drv_lif_llid_20_set(cfgllid20);
        if(!err) ag_drv_lif_llid_20_get( &cfgllid20);
        if(!err) bdmf_session_print(session, "ag_drv_lif_llid_20_get( %u)\n", cfgllid20);
        if(err || cfgllid20!=gtmv(m, 17))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint32_t cfgllid21=gtmv(m, 17);
        if(!err) bdmf_session_print(session, "ag_drv_lif_llid_21_set( %u)\n", cfgllid21);
        if(!err) ag_drv_lif_llid_21_set(cfgllid21);
        if(!err) ag_drv_lif_llid_21_get( &cfgllid21);
        if(!err) bdmf_session_print(session, "ag_drv_lif_llid_21_get( %u)\n", cfgllid21);
        if(err || cfgllid21!=gtmv(m, 17))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint32_t cfgllid22=gtmv(m, 17);
        if(!err) bdmf_session_print(session, "ag_drv_lif_llid_22_set( %u)\n", cfgllid22);
        if(!err) ag_drv_lif_llid_22_set(cfgllid22);
        if(!err) ag_drv_lif_llid_22_get( &cfgllid22);
        if(!err) bdmf_session_print(session, "ag_drv_lif_llid_22_get( %u)\n", cfgllid22);
        if(err || cfgllid22!=gtmv(m, 17))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint32_t cfgllid23=gtmv(m, 17);
        if(!err) bdmf_session_print(session, "ag_drv_lif_llid_23_set( %u)\n", cfgllid23);
        if(!err) ag_drv_lif_llid_23_set(cfgllid23);
        if(!err) ag_drv_lif_llid_23_get( &cfgllid23);
        if(!err) bdmf_session_print(session, "ag_drv_lif_llid_23_get( %u)\n", cfgllid23);
        if(err || cfgllid23!=gtmv(m, 17))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint8_t cffullupdatevalue=gtmv(m, 8);
        uint8_t cfmaxnegvalue=gtmv(m, 8);
        uint8_t cfmaxposvalue=gtmv(m, 8);
        if(!err) bdmf_session_print(session, "ag_drv_lif_time_ref_cnt_set( %u %u %u)\n", cffullupdatevalue, cfmaxnegvalue, cfmaxposvalue);
        if(!err) ag_drv_lif_time_ref_cnt_set(cffullupdatevalue, cfmaxnegvalue, cfmaxposvalue);
        if(!err) ag_drv_lif_time_ref_cnt_get( &cffullupdatevalue, &cfmaxnegvalue, &cfmaxposvalue);
        if(!err) bdmf_session_print(session, "ag_drv_lif_time_ref_cnt_get( %u %u %u)\n", cffullupdatevalue, cfmaxnegvalue, cfmaxposvalue);
        if(err || cffullupdatevalue!=gtmv(m, 8) || cfmaxnegvalue!=gtmv(m, 8) || cfmaxposvalue!=gtmv(m, 8))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint16_t cftimestampupdper=gtmv(m, 16);
        if(!err) bdmf_session_print(session, "ag_drv_lif_timestamp_upd_per_set( %u)\n", cftimestampupdper);
        if(!err) ag_drv_lif_timestamp_upd_per_set(cftimestampupdper);
        if(!err) ag_drv_lif_timestamp_upd_per_get( &cftimestampupdper);
        if(!err) bdmf_session_print(session, "ag_drv_lif_timestamp_upd_per_get( %u)\n", cftimestampupdper);
        if(err || cftimestampupdper!=gtmv(m, 16))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint32_t cftransporttime=gtmv(m, 32);
        if(!err) bdmf_session_print(session, "ag_drv_lif_tp_time_set( %u)\n", cftransporttime);
        if(!err) ag_drv_lif_tp_time_set(cftransporttime);
        if(!err) ag_drv_lif_tp_time_get( &cftransporttime);
        if(!err) bdmf_session_print(session, "ag_drv_lif_tp_time_get( %u)\n", cftransporttime);
        if(err || cftransporttime!=gtmv(m, 32))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint32_t ltmpcptime=gtmv(m, 32);
        if(!err) ag_drv_lif_mpcp_time_get( &ltmpcptime);
        if(!err) bdmf_session_print(session, "ag_drv_lif_mpcp_time_get( %u)\n", ltmpcptime);
    }
    {
        uint16_t cfrxmaxframelength=gtmv(m, 14);
        if(!err) bdmf_session_print(session, "ag_drv_lif_maxlen_ctr_set( %u)\n", cfrxmaxframelength);
        if(!err) ag_drv_lif_maxlen_ctr_set(cfrxmaxframelength);
        if(!err) ag_drv_lif_maxlen_ctr_get( &cfrxmaxframelength);
        if(!err) bdmf_session_print(session, "ag_drv_lif_maxlen_ctr_get( %u)\n", cfrxmaxframelength);
        if(err || cfrxmaxframelength!=gtmv(m, 14))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint16_t cftxlaserondelta=gtmv(m, 13);
        if(!err) bdmf_session_print(session, "ag_drv_lif_laser_on_delta_set( %u)\n", cftxlaserondelta);
        if(!err) ag_drv_lif_laser_on_delta_set(cftxlaserondelta);
        if(!err) ag_drv_lif_laser_on_delta_get( &cftxlaserondelta);
        if(!err) bdmf_session_print(session, "ag_drv_lif_laser_on_delta_get( %u)\n", cftxlaserondelta);
        if(err || cftxlaserondelta!=gtmv(m, 13))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint16_t cftxinitidle=gtmv(m, 16);
        uint8_t cftxlaseroffdelta=gtmv(m, 8);
        if(!err) bdmf_session_print(session, "ag_drv_lif_laser_off_idle_set( %u %u)\n", cftxinitidle, cftxlaseroffdelta);
        if(!err) ag_drv_lif_laser_off_idle_set(cftxinitidle, cftxlaseroffdelta);
        if(!err) ag_drv_lif_laser_off_idle_get( &cftxinitidle, &cftxlaseroffdelta);
        if(!err) bdmf_session_print(session, "ag_drv_lif_laser_off_idle_get( %u %u)\n", cftxinitidle, cftxlaseroffdelta);
        if(err || cftxinitidle!=gtmv(m, 16) || cftxlaseroffdelta!=gtmv(m, 8))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint16_t cftxfecinitidle=gtmv(m, 16);
        if(!err) bdmf_session_print(session, "ag_drv_lif_fec_init_idle_set( %u)\n", cftxfecinitidle);
        if(!err) ag_drv_lif_fec_init_idle_set(cftxfecinitidle);
        if(!err) ag_drv_lif_fec_init_idle_get( &cftxfecinitidle);
        if(!err) bdmf_session_print(session, "ag_drv_lif_fec_init_idle_get( %u)\n", cftxfecinitidle);
        if(err || cftxfecinitidle!=gtmv(m, 16))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint8_t cfrxtfecbiterrallow=gtmv(m, 4);
        uint8_t cfrxsfecbiterrallow=gtmv(m, 4);
        if(!err) bdmf_session_print(session, "ag_drv_lif_fec_err_allow_set( %u %u)\n", cfrxtfecbiterrallow, cfrxsfecbiterrallow);
        if(!err) ag_drv_lif_fec_err_allow_set(cfrxtfecbiterrallow, cfrxsfecbiterrallow);
        if(!err) ag_drv_lif_fec_err_allow_get( &cfrxtfecbiterrallow, &cfrxsfecbiterrallow);
        if(!err) bdmf_session_print(session, "ag_drv_lif_fec_err_allow_get( %u %u)\n", cfrxtfecbiterrallow, cfrxsfecbiterrallow);
        if(err || cfrxtfecbiterrallow!=gtmv(m, 4) || cfrxsfecbiterrallow!=gtmv(m, 4))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint8_t link_idx=gtmv(m, 5);
        bdmf_boolean data=gtmv(m, 1);
        if(!err) ag_drv_lif_sec_key_sel_get( link_idx, &data);
        if(!err) bdmf_session_print(session, "ag_drv_lif_sec_key_sel_get( %u %u)\n", link_idx, data);
    }
    {
        uint8_t link_idx=gtmv(m, 5);
        bdmf_boolean enEncrypt=gtmv(m, 1);
        if(!err) bdmf_session_print(session, "ag_drv_lif_dn_encrypt_stat_set( %u %u)\n", link_idx, enEncrypt);
        if(!err) ag_drv_lif_dn_encrypt_stat_set(link_idx, enEncrypt);
        if(!err) ag_drv_lif_dn_encrypt_stat_get( link_idx, &enEncrypt);
        if(!err) bdmf_session_print(session, "ag_drv_lif_dn_encrypt_stat_get( %u %u)\n", link_idx, enEncrypt);
        if(err || enEncrypt!=gtmv(m, 1))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint8_t link_idx=gtmv(m, 5);
        bdmf_boolean keyUpSel=gtmv(m, 1);
        if(!err) ag_drv_lif_sec_up_key_stat_get( link_idx, &keyUpSel);
        if(!err) bdmf_session_print(session, "ag_drv_lif_sec_up_key_stat_get( %u %u)\n", link_idx, keyUpSel);
    }
    {
        uint8_t link_idx=gtmv(m, 5);
        bdmf_boolean enUpEncrypt=gtmv(m, 1);
        if(!err) ag_drv_lif_sec_up_encrypt_stat_get( link_idx, &enUpEncrypt);
        if(!err) bdmf_session_print(session, "ag_drv_lif_sec_up_encrypt_stat_get( %u %u)\n", link_idx, enUpEncrypt);
    }
    {
        uint32_t secupmpcpoffset=gtmv(m, 32);
        if(!err) bdmf_session_print(session, "ag_drv_lif_sec_up_mpcp_offset_set( %u)\n", secupmpcpoffset);
        if(!err) ag_drv_lif_sec_up_mpcp_offset_set(secupmpcpoffset);
        if(!err) ag_drv_lif_sec_up_mpcp_offset_get( &secupmpcpoffset);
        if(!err) bdmf_session_print(session, "ag_drv_lif_sec_up_mpcp_offset_get( %u)\n", secupmpcpoffset);
        if(err || secupmpcpoffset!=gtmv(m, 32))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint8_t link_idx=gtmv(m, 5);
        bdmf_boolean cfFecTxFecEnLlid=gtmv(m, 1);
        if(!err) bdmf_session_print(session, "ag_drv_lif_fec_per_llid_set( %u %u)\n", link_idx, cfFecTxFecEnLlid);
        if(!err) ag_drv_lif_fec_per_llid_set(link_idx, cfFecTxFecEnLlid);
        if(!err) ag_drv_lif_fec_per_llid_get( link_idx, &cfFecTxFecEnLlid);
        if(!err) bdmf_session_print(session, "ag_drv_lif_fec_per_llid_get( %u %u)\n", link_idx, cfFecTxFecEnLlid);
        if(err || cfFecTxFecEnLlid!=gtmv(m, 1))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint32_t rxlinecodeerrcnt=gtmv(m, 32);
        if(!err) bdmf_session_print(session, "ag_drv_lif_rx_line_code_err_cnt_set( %u)\n", rxlinecodeerrcnt);
        if(!err) ag_drv_lif_rx_line_code_err_cnt_set(rxlinecodeerrcnt);
        if(!err) ag_drv_lif_rx_line_code_err_cnt_get( &rxlinecodeerrcnt);
        if(!err) bdmf_session_print(session, "ag_drv_lif_rx_line_code_err_cnt_get( %u)\n", rxlinecodeerrcnt);
        if(err || rxlinecodeerrcnt!=gtmv(m, 32))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint32_t rxaggmpcpcnt=gtmv(m, 32);
        if(!err) bdmf_session_print(session, "ag_drv_lif_rx_agg_mpcp_frm_set( %u)\n", rxaggmpcpcnt);
        if(!err) ag_drv_lif_rx_agg_mpcp_frm_set(rxaggmpcpcnt);
        if(!err) ag_drv_lif_rx_agg_mpcp_frm_get( &rxaggmpcpcnt);
        if(!err) bdmf_session_print(session, "ag_drv_lif_rx_agg_mpcp_frm_get( %u)\n", rxaggmpcpcnt);
        if(err || rxaggmpcpcnt!=gtmv(m, 32))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint32_t rxagggoodcnt=gtmv(m, 32);
        if(!err) bdmf_session_print(session, "ag_drv_lif_rx_agg_good_frm_set( %u)\n", rxagggoodcnt);
        if(!err) ag_drv_lif_rx_agg_good_frm_set(rxagggoodcnt);
        if(!err) ag_drv_lif_rx_agg_good_frm_get( &rxagggoodcnt);
        if(!err) bdmf_session_print(session, "ag_drv_lif_rx_agg_good_frm_get( %u)\n", rxagggoodcnt);
        if(err || rxagggoodcnt!=gtmv(m, 32))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint32_t rxagggoodbytescnt=gtmv(m, 32);
        if(!err) bdmf_session_print(session, "ag_drv_lif_rx_agg_good_byte_set( %u)\n", rxagggoodbytescnt);
        if(!err) ag_drv_lif_rx_agg_good_byte_set(rxagggoodbytescnt);
        if(!err) ag_drv_lif_rx_agg_good_byte_get( &rxagggoodbytescnt);
        if(!err) bdmf_session_print(session, "ag_drv_lif_rx_agg_good_byte_get( %u)\n", rxagggoodbytescnt);
        if(err || rxagggoodbytescnt!=gtmv(m, 32))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint32_t rxaggunderszcnt=gtmv(m, 32);
        if(!err) bdmf_session_print(session, "ag_drv_lif_rx_agg_undersz_frm_set( %u)\n", rxaggunderszcnt);
        if(!err) ag_drv_lif_rx_agg_undersz_frm_set(rxaggunderszcnt);
        if(!err) ag_drv_lif_rx_agg_undersz_frm_get( &rxaggunderszcnt);
        if(!err) bdmf_session_print(session, "ag_drv_lif_rx_agg_undersz_frm_get( %u)\n", rxaggunderszcnt);
        if(err || rxaggunderszcnt!=gtmv(m, 32))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint32_t rxaggoverszcnt=gtmv(m, 32);
        if(!err) bdmf_session_print(session, "ag_drv_lif_rx_agg_oversz_frm_set( %u)\n", rxaggoverszcnt);
        if(!err) ag_drv_lif_rx_agg_oversz_frm_set(rxaggoverszcnt);
        if(!err) ag_drv_lif_rx_agg_oversz_frm_get( &rxaggoverszcnt);
        if(!err) bdmf_session_print(session, "ag_drv_lif_rx_agg_oversz_frm_get( %u)\n", rxaggoverszcnt);
        if(err || rxaggoverszcnt!=gtmv(m, 32))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint32_t rxaggcrc8errcnt=gtmv(m, 32);
        if(!err) bdmf_session_print(session, "ag_drv_lif_rx_agg_crc8_frm_set( %u)\n", rxaggcrc8errcnt);
        if(!err) ag_drv_lif_rx_agg_crc8_frm_set(rxaggcrc8errcnt);
        if(!err) ag_drv_lif_rx_agg_crc8_frm_get( &rxaggcrc8errcnt);
        if(!err) bdmf_session_print(session, "ag_drv_lif_rx_agg_crc8_frm_get( %u)\n", rxaggcrc8errcnt);
        if(err || rxaggcrc8errcnt!=gtmv(m, 32))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint32_t rxaggfec=gtmv(m, 32);
        if(!err) bdmf_session_print(session, "ag_drv_lif_rx_agg_fec_frm_set( %u)\n", rxaggfec);
        if(!err) ag_drv_lif_rx_agg_fec_frm_set(rxaggfec);
        if(!err) ag_drv_lif_rx_agg_fec_frm_get( &rxaggfec);
        if(!err) bdmf_session_print(session, "ag_drv_lif_rx_agg_fec_frm_get( %u)\n", rxaggfec);
        if(err || rxaggfec!=gtmv(m, 32))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint32_t rxaggfecbytes=gtmv(m, 32);
        if(!err) bdmf_session_print(session, "ag_drv_lif_rx_agg_fec_byte_set( %u)\n", rxaggfecbytes);
        if(!err) ag_drv_lif_rx_agg_fec_byte_set(rxaggfecbytes);
        if(!err) ag_drv_lif_rx_agg_fec_byte_get( &rxaggfecbytes);
        if(!err) bdmf_session_print(session, "ag_drv_lif_rx_agg_fec_byte_get( %u)\n", rxaggfecbytes);
        if(err || rxaggfecbytes!=gtmv(m, 32))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint32_t rxaggfecexceederrs=gtmv(m, 32);
        if(!err) bdmf_session_print(session, "ag_drv_lif_rx_agg_fec_exc_err_frm_set( %u)\n", rxaggfecexceederrs);
        if(!err) ag_drv_lif_rx_agg_fec_exc_err_frm_set(rxaggfecexceederrs);
        if(!err) ag_drv_lif_rx_agg_fec_exc_err_frm_get( &rxaggfecexceederrs);
        if(!err) bdmf_session_print(session, "ag_drv_lif_rx_agg_fec_exc_err_frm_get( %u)\n", rxaggfecexceederrs);
        if(err || rxaggfecexceederrs!=gtmv(m, 32))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint32_t rxaggnonfecgood=gtmv(m, 32);
        if(!err) bdmf_session_print(session, "ag_drv_lif_rx_agg_nonfec_good_frm_set( %u)\n", rxaggnonfecgood);
        if(!err) ag_drv_lif_rx_agg_nonfec_good_frm_set(rxaggnonfecgood);
        if(!err) ag_drv_lif_rx_agg_nonfec_good_frm_get( &rxaggnonfecgood);
        if(!err) bdmf_session_print(session, "ag_drv_lif_rx_agg_nonfec_good_frm_get( %u)\n", rxaggnonfecgood);
        if(err || rxaggnonfecgood!=gtmv(m, 32))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint32_t rxaggnonfecgoodbytes=gtmv(m, 32);
        if(!err) bdmf_session_print(session, "ag_drv_lif_rx_agg_nonfec_good_byte_set( %u)\n", rxaggnonfecgoodbytes);
        if(!err) ag_drv_lif_rx_agg_nonfec_good_byte_set(rxaggnonfecgoodbytes);
        if(!err) ag_drv_lif_rx_agg_nonfec_good_byte_get( &rxaggnonfecgoodbytes);
        if(!err) bdmf_session_print(session, "ag_drv_lif_rx_agg_nonfec_good_byte_get( %u)\n", rxaggnonfecgoodbytes);
        if(err || rxaggnonfecgoodbytes!=gtmv(m, 32))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint32_t rxaggerrbytes=gtmv(m, 32);
        if(!err) bdmf_session_print(session, "ag_drv_lif_rx_agg_err_bytes_set( %u)\n", rxaggerrbytes);
        if(!err) ag_drv_lif_rx_agg_err_bytes_set(rxaggerrbytes);
        if(!err) ag_drv_lif_rx_agg_err_bytes_get( &rxaggerrbytes);
        if(!err) bdmf_session_print(session, "ag_drv_lif_rx_agg_err_bytes_get( %u)\n", rxaggerrbytes);
        if(err || rxaggerrbytes!=gtmv(m, 32))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint32_t rxaggerrzeroes=gtmv(m, 32);
        if(!err) bdmf_session_print(session, "ag_drv_lif_rx_agg_err_zeroes_set( %u)\n", rxaggerrzeroes);
        if(!err) ag_drv_lif_rx_agg_err_zeroes_set(rxaggerrzeroes);
        if(!err) ag_drv_lif_rx_agg_err_zeroes_get( &rxaggerrzeroes);
        if(!err) bdmf_session_print(session, "ag_drv_lif_rx_agg_err_zeroes_get( %u)\n", rxaggerrzeroes);
        if(err || rxaggerrzeroes!=gtmv(m, 32))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint32_t rxaggnoerrblks=gtmv(m, 32);
        if(!err) bdmf_session_print(session, "ag_drv_lif_rx_agg_no_err_blks_set( %u)\n", rxaggnoerrblks);
        if(!err) ag_drv_lif_rx_agg_no_err_blks_set(rxaggnoerrblks);
        if(!err) ag_drv_lif_rx_agg_no_err_blks_get( &rxaggnoerrblks);
        if(!err) bdmf_session_print(session, "ag_drv_lif_rx_agg_no_err_blks_get( %u)\n", rxaggnoerrblks);
        if(err || rxaggnoerrblks!=gtmv(m, 32))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint32_t rxaggcorrblks=gtmv(m, 32);
        if(!err) bdmf_session_print(session, "ag_drv_lif_rx_agg_cor_blks_set( %u)\n", rxaggcorrblks);
        if(!err) ag_drv_lif_rx_agg_cor_blks_set(rxaggcorrblks);
        if(!err) ag_drv_lif_rx_agg_cor_blks_get( &rxaggcorrblks);
        if(!err) bdmf_session_print(session, "ag_drv_lif_rx_agg_cor_blks_get( %u)\n", rxaggcorrblks);
        if(err || rxaggcorrblks!=gtmv(m, 32))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint32_t rxagguncorrblks=gtmv(m, 32);
        if(!err) bdmf_session_print(session, "ag_drv_lif_rx_agg_uncor_blks_set( %u)\n", rxagguncorrblks);
        if(!err) ag_drv_lif_rx_agg_uncor_blks_set(rxagguncorrblks);
        if(!err) ag_drv_lif_rx_agg_uncor_blks_get( &rxagguncorrblks);
        if(!err) bdmf_session_print(session, "ag_drv_lif_rx_agg_uncor_blks_get( %u)\n", rxagguncorrblks);
        if(err || rxagguncorrblks!=gtmv(m, 32))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint32_t rxaggerrones=gtmv(m, 32);
        if(!err) bdmf_session_print(session, "ag_drv_lif_rx_agg_err_ones_set( %u)\n", rxaggerrones);
        if(!err) ag_drv_lif_rx_agg_err_ones_set(rxaggerrones);
        if(!err) ag_drv_lif_rx_agg_err_ones_get( &rxaggerrones);
        if(!err) bdmf_session_print(session, "ag_drv_lif_rx_agg_err_ones_get( %u)\n", rxaggerrones);
        if(err || rxaggerrones!=gtmv(m, 32))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint32_t rxaggerroredcnt=gtmv(m, 32);
        if(!err) bdmf_session_print(session, "ag_drv_lif_rx_agg_err_frm_set( %u)\n", rxaggerroredcnt);
        if(!err) ag_drv_lif_rx_agg_err_frm_set(rxaggerroredcnt);
        if(!err) ag_drv_lif_rx_agg_err_frm_get( &rxaggerroredcnt);
        if(!err) bdmf_session_print(session, "ag_drv_lif_rx_agg_err_frm_get( %u)\n", rxaggerroredcnt);
        if(err || rxaggerroredcnt!=gtmv(m, 32))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint32_t txframecnt=gtmv(m, 32);
        if(!err) bdmf_session_print(session, "ag_drv_lif_tx_pkt_cnt_set( %u)\n", txframecnt);
        if(!err) ag_drv_lif_tx_pkt_cnt_set(txframecnt);
        if(!err) ag_drv_lif_tx_pkt_cnt_get( &txframecnt);
        if(!err) bdmf_session_print(session, "ag_drv_lif_tx_pkt_cnt_get( %u)\n", txframecnt);
        if(err || txframecnt!=gtmv(m, 32))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint32_t txbytecnt=gtmv(m, 32);
        if(!err) bdmf_session_print(session, "ag_drv_lif_tx_byte_cnt_set( %u)\n", txbytecnt);
        if(!err) ag_drv_lif_tx_byte_cnt_set(txbytecnt);
        if(!err) ag_drv_lif_tx_byte_cnt_get( &txbytecnt);
        if(!err) bdmf_session_print(session, "ag_drv_lif_tx_byte_cnt_get( %u)\n", txbytecnt);
        if(err || txbytecnt!=gtmv(m, 32))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint32_t txnonfecframecnt=gtmv(m, 32);
        if(!err) bdmf_session_print(session, "ag_drv_lif_tx_non_fec_pkt_cnt_set( %u)\n", txnonfecframecnt);
        if(!err) ag_drv_lif_tx_non_fec_pkt_cnt_set(txnonfecframecnt);
        if(!err) ag_drv_lif_tx_non_fec_pkt_cnt_get( &txnonfecframecnt);
        if(!err) bdmf_session_print(session, "ag_drv_lif_tx_non_fec_pkt_cnt_get( %u)\n", txnonfecframecnt);
        if(err || txnonfecframecnt!=gtmv(m, 32))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint32_t txnonfecbytecnt=gtmv(m, 32);
        if(!err) bdmf_session_print(session, "ag_drv_lif_tx_non_fec_byte_cnt_set( %u)\n", txnonfecbytecnt);
        if(!err) ag_drv_lif_tx_non_fec_byte_cnt_set(txnonfecbytecnt);
        if(!err) ag_drv_lif_tx_non_fec_byte_cnt_get( &txnonfecbytecnt);
        if(!err) bdmf_session_print(session, "ag_drv_lif_tx_non_fec_byte_cnt_get( %u)\n", txnonfecbytecnt);
        if(err || txnonfecbytecnt!=gtmv(m, 32))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint32_t txfecframecnt=gtmv(m, 32);
        if(!err) bdmf_session_print(session, "ag_drv_lif_tx_fec_pkt_cnt_set( %u)\n", txfecframecnt);
        if(!err) ag_drv_lif_tx_fec_pkt_cnt_set(txfecframecnt);
        if(!err) ag_drv_lif_tx_fec_pkt_cnt_get( &txfecframecnt);
        if(!err) bdmf_session_print(session, "ag_drv_lif_tx_fec_pkt_cnt_get( %u)\n", txfecframecnt);
        if(err || txfecframecnt!=gtmv(m, 32))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint32_t txfecbytecnt=gtmv(m, 32);
        if(!err) bdmf_session_print(session, "ag_drv_lif_tx_fec_byte_cnt_set( %u)\n", txfecbytecnt);
        if(!err) ag_drv_lif_tx_fec_byte_cnt_set(txfecbytecnt);
        if(!err) ag_drv_lif_tx_fec_byte_cnt_get( &txfecbytecnt);
        if(!err) bdmf_session_print(session, "ag_drv_lif_tx_fec_byte_cnt_get( %u)\n", txfecbytecnt);
        if(err || txfecbytecnt!=gtmv(m, 32))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint32_t txfecblkscnt=gtmv(m, 32);
        if(!err) bdmf_session_print(session, "ag_drv_lif_tx_fec_blk_cnt_set( %u)\n", txfecblkscnt);
        if(!err) ag_drv_lif_tx_fec_blk_cnt_set(txfecblkscnt);
        if(!err) ag_drv_lif_tx_fec_blk_cnt_get( &txfecblkscnt);
        if(!err) bdmf_session_print(session, "ag_drv_lif_tx_fec_blk_cnt_get( %u)\n", txfecblkscnt);
        if(err || txfecblkscnt!=gtmv(m, 32))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint32_t txmpcpframecnt=gtmv(m, 32);
        if(!err) bdmf_session_print(session, "ag_drv_lif_tx_mpcp_pkt_cnt_set( %u)\n", txmpcpframecnt);
        if(!err) ag_drv_lif_tx_mpcp_pkt_cnt_set(txmpcpframecnt);
        if(!err) ag_drv_lif_tx_mpcp_pkt_cnt_get( &txmpcpframecnt);
        if(!err) bdmf_session_print(session, "ag_drv_lif_tx_mpcp_pkt_cnt_get( %u)\n", txmpcpframecnt);
        if(err || txmpcpframecnt!=gtmv(m, 32))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint32_t txdataframecnt=gtmv(m, 32);
        if(!err) bdmf_session_print(session, "ag_drv_lif_debug_tx_data_pkt_cnt_set( %u)\n", txdataframecnt);
        if(!err) ag_drv_lif_debug_tx_data_pkt_cnt_set(txdataframecnt);
        if(!err) ag_drv_lif_debug_tx_data_pkt_cnt_get( &txdataframecnt);
        if(!err) bdmf_session_print(session, "ag_drv_lif_debug_tx_data_pkt_cnt_get( %u)\n", txdataframecnt);
        if(err || txdataframecnt!=gtmv(m, 32))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint32_t stkyfecrevcllidbmsk=gtmv(m, 32);
        if(!err) bdmf_session_print(session, "ag_drv_lif_fec_llid_status_set( %u)\n", stkyfecrevcllidbmsk);
        if(!err) ag_drv_lif_fec_llid_status_set(stkyfecrevcllidbmsk);
        if(!err) ag_drv_lif_fec_llid_status_get( &stkyfecrevcllidbmsk);
        if(!err) bdmf_session_print(session, "ag_drv_lif_fec_llid_status_get( %u)\n", stkyfecrevcllidbmsk);
        if(err || stkyfecrevcllidbmsk!=gtmv(m, 32))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint32_t cfigivnullllid=gtmv(m, 17);
        if(!err) bdmf_session_print(session, "ag_drv_lif_sec_rx_tek_ig_iv_llid_set( %u)\n", cfigivnullllid);
        if(!err) ag_drv_lif_sec_rx_tek_ig_iv_llid_set(cfigivnullllid);
        if(!err) ag_drv_lif_sec_rx_tek_ig_iv_llid_get( &cfigivnullllid);
        if(!err) bdmf_session_print(session, "ag_drv_lif_sec_rx_tek_ig_iv_llid_get( %u)\n", cfigivnullllid);
        if(err || cfigivnullllid!=gtmv(m, 17))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint32_t cfrxlifberinterval=gtmv(m, 17);
        uint16_t cfrxlifberthreshld=gtmv(m, 13);
        uint8_t cfrxlifbercntrl=gtmv(m, 2);
        if(!err) bdmf_session_print(session, "ag_drv_lif_pon_ber_interv_thresh_set( %u %u %u)\n", cfrxlifberinterval, cfrxlifberthreshld, cfrxlifbercntrl);
        if(!err) ag_drv_lif_pon_ber_interv_thresh_set(cfrxlifberinterval, cfrxlifberthreshld, cfrxlifbercntrl);
        if(!err) ag_drv_lif_pon_ber_interv_thresh_get( &cfrxlifberinterval, &cfrxlifberthreshld, &cfrxlifbercntrl);
        if(!err) bdmf_session_print(session, "ag_drv_lif_pon_ber_interv_thresh_get( %u %u %u)\n", cfrxlifberinterval, cfrxlifberthreshld, cfrxlifbercntrl);
        if(err || cfrxlifberinterval!=gtmv(m, 17) || cfrxlifberthreshld!=gtmv(m, 13) || cfrxlifbercntrl!=gtmv(m, 2))
            return err ? err : BDMF_ERR_IO;
    }
    {
        bdmf_boolean iopbilaserens1a=gtmv(m, 1);
        bdmf_boolean cfglsrmonacthi=gtmv(m, 1);
        bdmf_boolean pbilasermonrsta_n_pre=gtmv(m, 1);
        if(!err) bdmf_session_print(session, "ag_drv_lif_lsr_mon_a_ctrl_set( %u %u %u)\n", iopbilaserens1a, cfglsrmonacthi, pbilasermonrsta_n_pre);
        if(!err) ag_drv_lif_lsr_mon_a_ctrl_set(iopbilaserens1a, cfglsrmonacthi, pbilasermonrsta_n_pre);
        if(!err) ag_drv_lif_lsr_mon_a_ctrl_get( &iopbilaserens1a, &cfglsrmonacthi, &pbilasermonrsta_n_pre);
        if(!err) bdmf_session_print(session, "ag_drv_lif_lsr_mon_a_ctrl_get( %u %u %u)\n", iopbilaserens1a, cfglsrmonacthi, pbilasermonrsta_n_pre);
        if(err || iopbilaserens1a!=gtmv(m, 1) || cfglsrmonacthi!=gtmv(m, 1) || pbilasermonrsta_n_pre!=gtmv(m, 1))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint32_t cfglasermonmaxa=gtmv(m, 32);
        if(!err) bdmf_session_print(session, "ag_drv_lif_lsr_mon_a_max_thr_set( %u)\n", cfglasermonmaxa);
        if(!err) ag_drv_lif_lsr_mon_a_max_thr_set(cfglasermonmaxa);
        if(!err) ag_drv_lif_lsr_mon_a_max_thr_get( &cfglasermonmaxa);
        if(!err) bdmf_session_print(session, "ag_drv_lif_lsr_mon_a_max_thr_get( %u)\n", cfglasermonmaxa);
        if(err || cfglasermonmaxa!=gtmv(m, 32))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint32_t laserontimea=gtmv(m, 32);
        if(!err) ag_drv_lif_lsr_mon_a_bst_len_get( &laserontimea);
        if(!err) bdmf_session_print(session, "ag_drv_lif_lsr_mon_a_bst_len_get( %u)\n", laserontimea);
    }
    {
        uint32_t lasermonbrstcnta=gtmv(m, 32);
        if(!err) ag_drv_lif_lsr_mon_a_bst_cnt_get( &lasermonbrstcnta);
        if(!err) bdmf_session_print(session, "ag_drv_lif_lsr_mon_a_bst_cnt_get( %u)\n", lasermonbrstcnta);
    }
    {
        uint8_t aligncsqq=gtmv(m, 6);
        uint8_t rxfecifcsqq=gtmv(m, 5);
        if(!err) ag_drv_lif_debug_pon_sm_get( &aligncsqq, &rxfecifcsqq);
        if(!err) bdmf_session_print(session, "ag_drv_lif_debug_pon_sm_get( %u %u)\n", aligncsqq, rxfecifcsqq);
    }
    {
        uint8_t rxsyncsqq=gtmv(m, 5);
        uint8_t rxcorcs=gtmv(m, 2);
        uint8_t fecrxoutcs=gtmv(m, 5);
        if(!err) ag_drv_lif_debug_fec_sm_get( &rxsyncsqq, &rxcorcs, &fecrxoutcs);
        if(!err) bdmf_session_print(session, "ag_drv_lif_debug_fec_sm_get( %u %u %u)\n", rxsyncsqq, rxcorcs, fecrxoutcs);
    }
    {
        uint32_t cfgaepktnumwnd=gtmv(m, 32);
        if(!err) bdmf_session_print(session, "ag_drv_lif_ae_pktnum_window_set( %u)\n", cfgaepktnumwnd);
        if(!err) ag_drv_lif_ae_pktnum_window_set(cfgaepktnumwnd);
        if(!err) ag_drv_lif_ae_pktnum_window_get( &cfgaepktnumwnd);
        if(!err) bdmf_session_print(session, "ag_drv_lif_ae_pktnum_window_get( %u)\n", cfgaepktnumwnd);
        if(err || cfgaepktnumwnd!=gtmv(m, 32))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint32_t cfgpktnummaxthresh=gtmv(m, 32);
        if(!err) bdmf_session_print(session, "ag_drv_lif_ae_pktnum_thresh_set( %u)\n", cfgpktnummaxthresh);
        if(!err) ag_drv_lif_ae_pktnum_thresh_set(cfgpktnummaxthresh);
        if(!err) ag_drv_lif_ae_pktnum_thresh_get( &cfgpktnummaxthresh);
        if(!err) bdmf_session_print(session, "ag_drv_lif_ae_pktnum_thresh_get( %u)\n", cfgpktnummaxthresh);
        if(err || cfgpktnummaxthresh!=gtmv(m, 32))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint8_t secupindxwtpktnummax=gtmv(m, 5);
        uint8_t secdnindxwtpktnumabort=gtmv(m, 5);
        if(!err) ag_drv_lif_ae_pktnum_stat_get( &secupindxwtpktnummax, &secdnindxwtpktnumabort);
        if(!err) bdmf_session_print(session, "ag_drv_lif_ae_pktnum_stat_get( %u %u)\n", secupindxwtpktnummax, secdnindxwtpktnumabort);
    }
    {
        uint32_t cfgllid8=gtmv(m, 17);
        if(!err) bdmf_session_print(session, "ag_drv_lif_llid_8_set( %u)\n", cfgllid8);
        if(!err) ag_drv_lif_llid_8_set(cfgllid8);
        if(!err) ag_drv_lif_llid_8_get( &cfgllid8);
        if(!err) bdmf_session_print(session, "ag_drv_lif_llid_8_get( %u)\n", cfgllid8);
        if(err || cfgllid8!=gtmv(m, 17))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint32_t cfgllid9=gtmv(m, 17);
        if(!err) bdmf_session_print(session, "ag_drv_lif_llid_9_set( %u)\n", cfgllid9);
        if(!err) ag_drv_lif_llid_9_set(cfgllid9);
        if(!err) ag_drv_lif_llid_9_get( &cfgllid9);
        if(!err) bdmf_session_print(session, "ag_drv_lif_llid_9_get( %u)\n", cfgllid9);
        if(err || cfgllid9!=gtmv(m, 17))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint32_t cfgllid10=gtmv(m, 17);
        if(!err) bdmf_session_print(session, "ag_drv_lif_llid_10_set( %u)\n", cfgllid10);
        if(!err) ag_drv_lif_llid_10_set(cfgllid10);
        if(!err) ag_drv_lif_llid_10_get( &cfgllid10);
        if(!err) bdmf_session_print(session, "ag_drv_lif_llid_10_get( %u)\n", cfgllid10);
        if(err || cfgllid10!=gtmv(m, 17))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint32_t cfgllid11=gtmv(m, 17);
        if(!err) bdmf_session_print(session, "ag_drv_lif_llid_11_set( %u)\n", cfgllid11);
        if(!err) ag_drv_lif_llid_11_set(cfgllid11);
        if(!err) ag_drv_lif_llid_11_get( &cfgllid11);
        if(!err) bdmf_session_print(session, "ag_drv_lif_llid_11_get( %u)\n", cfgllid11);
        if(err || cfgllid11!=gtmv(m, 17))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint32_t cfgllid12=gtmv(m, 17);
        if(!err) bdmf_session_print(session, "ag_drv_lif_llid_12_set( %u)\n", cfgllid12);
        if(!err) ag_drv_lif_llid_12_set(cfgllid12);
        if(!err) ag_drv_lif_llid_12_get( &cfgllid12);
        if(!err) bdmf_session_print(session, "ag_drv_lif_llid_12_get( %u)\n", cfgllid12);
        if(err || cfgllid12!=gtmv(m, 17))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint32_t cfgllid13=gtmv(m, 17);
        if(!err) bdmf_session_print(session, "ag_drv_lif_llid_13_set( %u)\n", cfgllid13);
        if(!err) ag_drv_lif_llid_13_set(cfgllid13);
        if(!err) ag_drv_lif_llid_13_get( &cfgllid13);
        if(!err) bdmf_session_print(session, "ag_drv_lif_llid_13_get( %u)\n", cfgllid13);
        if(err || cfgllid13!=gtmv(m, 17))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint32_t cfgllid14=gtmv(m, 17);
        if(!err) bdmf_session_print(session, "ag_drv_lif_llid_14_set( %u)\n", cfgllid14);
        if(!err) ag_drv_lif_llid_14_set(cfgllid14);
        if(!err) ag_drv_lif_llid_14_get( &cfgllid14);
        if(!err) bdmf_session_print(session, "ag_drv_lif_llid_14_get( %u)\n", cfgllid14);
        if(err || cfgllid14!=gtmv(m, 17))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint32_t cfgllid15=gtmv(m, 17);
        if(!err) bdmf_session_print(session, "ag_drv_lif_llid_15_set( %u)\n", cfgllid15);
        if(!err) ag_drv_lif_llid_15_set(cfgllid15);
        if(!err) ag_drv_lif_llid_15_get( &cfgllid15);
        if(!err) bdmf_session_print(session, "ag_drv_lif_llid_15_get( %u)\n", cfgllid15);
        if(err || cfgllid15!=gtmv(m, 17))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint32_t cfgllid24=gtmv(m, 17);
        if(!err) bdmf_session_print(session, "ag_drv_lif_llid_24_set( %u)\n", cfgllid24);
        if(!err) ag_drv_lif_llid_24_set(cfgllid24);
        if(!err) ag_drv_lif_llid_24_get( &cfgllid24);
        if(!err) bdmf_session_print(session, "ag_drv_lif_llid_24_get( %u)\n", cfgllid24);
        if(err || cfgllid24!=gtmv(m, 17))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint32_t cfgllid25=gtmv(m, 17);
        if(!err) bdmf_session_print(session, "ag_drv_lif_llid_25_set( %u)\n", cfgllid25);
        if(!err) ag_drv_lif_llid_25_set(cfgllid25);
        if(!err) ag_drv_lif_llid_25_get( &cfgllid25);
        if(!err) bdmf_session_print(session, "ag_drv_lif_llid_25_get( %u)\n", cfgllid25);
        if(err || cfgllid25!=gtmv(m, 17))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint32_t cfgllid26=gtmv(m, 17);
        if(!err) bdmf_session_print(session, "ag_drv_lif_llid_26_set( %u)\n", cfgllid26);
        if(!err) ag_drv_lif_llid_26_set(cfgllid26);
        if(!err) ag_drv_lif_llid_26_get( &cfgllid26);
        if(!err) bdmf_session_print(session, "ag_drv_lif_llid_26_get( %u)\n", cfgllid26);
        if(err || cfgllid26!=gtmv(m, 17))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint32_t cfgllid27=gtmv(m, 17);
        if(!err) bdmf_session_print(session, "ag_drv_lif_llid_27_set( %u)\n", cfgllid27);
        if(!err) ag_drv_lif_llid_27_set(cfgllid27);
        if(!err) ag_drv_lif_llid_27_get( &cfgllid27);
        if(!err) bdmf_session_print(session, "ag_drv_lif_llid_27_get( %u)\n", cfgllid27);
        if(err || cfgllid27!=gtmv(m, 17))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint32_t cfgllid28=gtmv(m, 17);
        if(!err) bdmf_session_print(session, "ag_drv_lif_llid_28_set( %u)\n", cfgllid28);
        if(!err) ag_drv_lif_llid_28_set(cfgllid28);
        if(!err) ag_drv_lif_llid_28_get( &cfgllid28);
        if(!err) bdmf_session_print(session, "ag_drv_lif_llid_28_get( %u)\n", cfgllid28);
        if(err || cfgllid28!=gtmv(m, 17))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint32_t cfgllid29=gtmv(m, 17);
        if(!err) bdmf_session_print(session, "ag_drv_lif_llid_29_set( %u)\n", cfgllid29);
        if(!err) ag_drv_lif_llid_29_set(cfgllid29);
        if(!err) ag_drv_lif_llid_29_get( &cfgllid29);
        if(!err) bdmf_session_print(session, "ag_drv_lif_llid_29_get( %u)\n", cfgllid29);
        if(err || cfgllid29!=gtmv(m, 17))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint32_t cfgllid30=gtmv(m, 17);
        if(!err) bdmf_session_print(session, "ag_drv_lif_llid_30_set( %u)\n", cfgllid30);
        if(!err) ag_drv_lif_llid_30_set(cfgllid30);
        if(!err) ag_drv_lif_llid_30_get( &cfgllid30);
        if(!err) bdmf_session_print(session, "ag_drv_lif_llid_30_get( %u)\n", cfgllid30);
        if(err || cfgllid30!=gtmv(m, 17))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint32_t cfgllid31=gtmv(m, 17);
        if(!err) bdmf_session_print(session, "ag_drv_lif_llid_31_set( %u)\n", cfgllid31);
        if(!err) ag_drv_lif_llid_31_set(cfgllid31);
        if(!err) ag_drv_lif_llid_31_get( &cfgllid31);
        if(!err) bdmf_session_print(session, "ag_drv_lif_llid_31_get( %u)\n", cfgllid31);
        if(err || cfgllid31!=gtmv(m, 17))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint16_t cfgvlantype=gtmv(m, 16);
        if(!err) bdmf_session_print(session, "ag_drv_lif_vlan_type_set( %u)\n", cfgvlantype);
        if(!err) ag_drv_lif_vlan_type_set(cfgvlantype);
        if(!err) ag_drv_lif_vlan_type_get( &cfgvlantype);
        if(!err) bdmf_session_print(session, "ag_drv_lif_vlan_type_get( %u)\n", cfgvlantype);
        if(err || cfgvlantype!=gtmv(m, 16))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint16_t cfgp2pscien=gtmv(m, 16);
        if(!err) bdmf_session_print(session, "ag_drv_lif_p2p_ae_sci_en_set( %u)\n", cfgp2pscien);
        if(!err) ag_drv_lif_p2p_ae_sci_en_set(cfgp2pscien);
        if(!err) ag_drv_lif_p2p_ae_sci_en_get( &cfgp2pscien);
        if(!err) bdmf_session_print(session, "ag_drv_lif_p2p_ae_sci_en_get( %u)\n", cfgp2pscien);
        if(err || cfgp2pscien!=gtmv(m, 16))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint32_t cfgp2psci_lo_0=gtmv(m, 32);
        if(!err) bdmf_session_print(session, "ag_drv_lif_p2p_ae_sci_lo_0_set( %u)\n", cfgp2psci_lo_0);
        if(!err) ag_drv_lif_p2p_ae_sci_lo_0_set(cfgp2psci_lo_0);
        if(!err) ag_drv_lif_p2p_ae_sci_lo_0_get( &cfgp2psci_lo_0);
        if(!err) bdmf_session_print(session, "ag_drv_lif_p2p_ae_sci_lo_0_get( %u)\n", cfgp2psci_lo_0);
        if(err || cfgp2psci_lo_0!=gtmv(m, 32))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint32_t cfgp2psci_hi_0=gtmv(m, 32);
        if(!err) bdmf_session_print(session, "ag_drv_lif_p2p_ae_sci_hi_0_set( %u)\n", cfgp2psci_hi_0);
        if(!err) ag_drv_lif_p2p_ae_sci_hi_0_set(cfgp2psci_hi_0);
        if(!err) ag_drv_lif_p2p_ae_sci_hi_0_get( &cfgp2psci_hi_0);
        if(!err) bdmf_session_print(session, "ag_drv_lif_p2p_ae_sci_hi_0_get( %u)\n", cfgp2psci_hi_0);
        if(err || cfgp2psci_hi_0!=gtmv(m, 32))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint32_t cfgp2psci_lo_1=gtmv(m, 32);
        if(!err) bdmf_session_print(session, "ag_drv_lif_p2p_ae_sci_lo_1_set( %u)\n", cfgp2psci_lo_1);
        if(!err) ag_drv_lif_p2p_ae_sci_lo_1_set(cfgp2psci_lo_1);
        if(!err) ag_drv_lif_p2p_ae_sci_lo_1_get( &cfgp2psci_lo_1);
        if(!err) bdmf_session_print(session, "ag_drv_lif_p2p_ae_sci_lo_1_get( %u)\n", cfgp2psci_lo_1);
        if(err || cfgp2psci_lo_1!=gtmv(m, 32))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint32_t cfgp2psci_hi_1=gtmv(m, 32);
        if(!err) bdmf_session_print(session, "ag_drv_lif_p2p_ae_sci_hi_1_set( %u)\n", cfgp2psci_hi_1);
        if(!err) ag_drv_lif_p2p_ae_sci_hi_1_set(cfgp2psci_hi_1);
        if(!err) ag_drv_lif_p2p_ae_sci_hi_1_get( &cfgp2psci_hi_1);
        if(!err) bdmf_session_print(session, "ag_drv_lif_p2p_ae_sci_hi_1_get( %u)\n", cfgp2psci_hi_1);
        if(err || cfgp2psci_hi_1!=gtmv(m, 32))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint32_t cfgp2psci_lo_2=gtmv(m, 32);
        if(!err) bdmf_session_print(session, "ag_drv_lif_p2p_ae_sci_lo_2_set( %u)\n", cfgp2psci_lo_2);
        if(!err) ag_drv_lif_p2p_ae_sci_lo_2_set(cfgp2psci_lo_2);
        if(!err) ag_drv_lif_p2p_ae_sci_lo_2_get( &cfgp2psci_lo_2);
        if(!err) bdmf_session_print(session, "ag_drv_lif_p2p_ae_sci_lo_2_get( %u)\n", cfgp2psci_lo_2);
        if(err || cfgp2psci_lo_2!=gtmv(m, 32))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint32_t cfgp2psci_hi_2=gtmv(m, 32);
        if(!err) bdmf_session_print(session, "ag_drv_lif_p2p_ae_sci_hi_2_set( %u)\n", cfgp2psci_hi_2);
        if(!err) ag_drv_lif_p2p_ae_sci_hi_2_set(cfgp2psci_hi_2);
        if(!err) ag_drv_lif_p2p_ae_sci_hi_2_get( &cfgp2psci_hi_2);
        if(!err) bdmf_session_print(session, "ag_drv_lif_p2p_ae_sci_hi_2_get( %u)\n", cfgp2psci_hi_2);
        if(err || cfgp2psci_hi_2!=gtmv(m, 32))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint32_t cfgp2psci_lo_3=gtmv(m, 32);
        if(!err) bdmf_session_print(session, "ag_drv_lif_p2p_ae_sci_lo_3_set( %u)\n", cfgp2psci_lo_3);
        if(!err) ag_drv_lif_p2p_ae_sci_lo_3_set(cfgp2psci_lo_3);
        if(!err) ag_drv_lif_p2p_ae_sci_lo_3_get( &cfgp2psci_lo_3);
        if(!err) bdmf_session_print(session, "ag_drv_lif_p2p_ae_sci_lo_3_get( %u)\n", cfgp2psci_lo_3);
        if(err || cfgp2psci_lo_3!=gtmv(m, 32))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint32_t cfgp2psci_hi_3=gtmv(m, 32);
        if(!err) bdmf_session_print(session, "ag_drv_lif_p2p_ae_sci_hi_3_set( %u)\n", cfgp2psci_hi_3);
        if(!err) ag_drv_lif_p2p_ae_sci_hi_3_set(cfgp2psci_hi_3);
        if(!err) ag_drv_lif_p2p_ae_sci_hi_3_get( &cfgp2psci_hi_3);
        if(!err) bdmf_session_print(session, "ag_drv_lif_p2p_ae_sci_hi_3_get( %u)\n", cfgp2psci_hi_3);
        if(err || cfgp2psci_hi_3!=gtmv(m, 32))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint32_t cfgp2psci_lo_4=gtmv(m, 32);
        if(!err) bdmf_session_print(session, "ag_drv_lif_p2p_ae_sci_lo_4_set( %u)\n", cfgp2psci_lo_4);
        if(!err) ag_drv_lif_p2p_ae_sci_lo_4_set(cfgp2psci_lo_4);
        if(!err) ag_drv_lif_p2p_ae_sci_lo_4_get( &cfgp2psci_lo_4);
        if(!err) bdmf_session_print(session, "ag_drv_lif_p2p_ae_sci_lo_4_get( %u)\n", cfgp2psci_lo_4);
        if(err || cfgp2psci_lo_4!=gtmv(m, 32))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint32_t cfgp2psci_hi_4=gtmv(m, 32);
        if(!err) bdmf_session_print(session, "ag_drv_lif_p2p_ae_sci_hi_4_set( %u)\n", cfgp2psci_hi_4);
        if(!err) ag_drv_lif_p2p_ae_sci_hi_4_set(cfgp2psci_hi_4);
        if(!err) ag_drv_lif_p2p_ae_sci_hi_4_get( &cfgp2psci_hi_4);
        if(!err) bdmf_session_print(session, "ag_drv_lif_p2p_ae_sci_hi_4_get( %u)\n", cfgp2psci_hi_4);
        if(err || cfgp2psci_hi_4!=gtmv(m, 32))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint32_t cfgp2psci_lo_5=gtmv(m, 32);
        if(!err) bdmf_session_print(session, "ag_drv_lif_p2p_ae_sci_lo_5_set( %u)\n", cfgp2psci_lo_5);
        if(!err) ag_drv_lif_p2p_ae_sci_lo_5_set(cfgp2psci_lo_5);
        if(!err) ag_drv_lif_p2p_ae_sci_lo_5_get( &cfgp2psci_lo_5);
        if(!err) bdmf_session_print(session, "ag_drv_lif_p2p_ae_sci_lo_5_get( %u)\n", cfgp2psci_lo_5);
        if(err || cfgp2psci_lo_5!=gtmv(m, 32))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint32_t cfgp2psci_hi_5=gtmv(m, 32);
        if(!err) bdmf_session_print(session, "ag_drv_lif_p2p_ae_sci_hi_5_set( %u)\n", cfgp2psci_hi_5);
        if(!err) ag_drv_lif_p2p_ae_sci_hi_5_set(cfgp2psci_hi_5);
        if(!err) ag_drv_lif_p2p_ae_sci_hi_5_get( &cfgp2psci_hi_5);
        if(!err) bdmf_session_print(session, "ag_drv_lif_p2p_ae_sci_hi_5_get( %u)\n", cfgp2psci_hi_5);
        if(err || cfgp2psci_hi_5!=gtmv(m, 32))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint32_t cfgp2psci_lo_6=gtmv(m, 32);
        if(!err) bdmf_session_print(session, "ag_drv_lif_p2p_ae_sci_lo_6_set( %u)\n", cfgp2psci_lo_6);
        if(!err) ag_drv_lif_p2p_ae_sci_lo_6_set(cfgp2psci_lo_6);
        if(!err) ag_drv_lif_p2p_ae_sci_lo_6_get( &cfgp2psci_lo_6);
        if(!err) bdmf_session_print(session, "ag_drv_lif_p2p_ae_sci_lo_6_get( %u)\n", cfgp2psci_lo_6);
        if(err || cfgp2psci_lo_6!=gtmv(m, 32))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint32_t cfgp2psci_hi_6=gtmv(m, 32);
        if(!err) bdmf_session_print(session, "ag_drv_lif_p2p_ae_sci_hi_6_set( %u)\n", cfgp2psci_hi_6);
        if(!err) ag_drv_lif_p2p_ae_sci_hi_6_set(cfgp2psci_hi_6);
        if(!err) ag_drv_lif_p2p_ae_sci_hi_6_get( &cfgp2psci_hi_6);
        if(!err) bdmf_session_print(session, "ag_drv_lif_p2p_ae_sci_hi_6_get( %u)\n", cfgp2psci_hi_6);
        if(err || cfgp2psci_hi_6!=gtmv(m, 32))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint32_t cfgp2psci_lo_7=gtmv(m, 32);
        if(!err) bdmf_session_print(session, "ag_drv_lif_p2p_ae_sci_lo_7_set( %u)\n", cfgp2psci_lo_7);
        if(!err) ag_drv_lif_p2p_ae_sci_lo_7_set(cfgp2psci_lo_7);
        if(!err) ag_drv_lif_p2p_ae_sci_lo_7_get( &cfgp2psci_lo_7);
        if(!err) bdmf_session_print(session, "ag_drv_lif_p2p_ae_sci_lo_7_get( %u)\n", cfgp2psci_lo_7);
        if(err || cfgp2psci_lo_7!=gtmv(m, 32))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint32_t cfgp2psci_hi_7=gtmv(m, 32);
        if(!err) bdmf_session_print(session, "ag_drv_lif_p2p_ae_sci_hi_7_set( %u)\n", cfgp2psci_hi_7);
        if(!err) ag_drv_lif_p2p_ae_sci_hi_7_set(cfgp2psci_hi_7);
        if(!err) ag_drv_lif_p2p_ae_sci_hi_7_get( &cfgp2psci_hi_7);
        if(!err) bdmf_session_print(session, "ag_drv_lif_p2p_ae_sci_hi_7_get( %u)\n", cfgp2psci_hi_7);
        if(err || cfgp2psci_hi_7!=gtmv(m, 32))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint32_t cfgp2psci_lo_8=gtmv(m, 32);
        if(!err) bdmf_session_print(session, "ag_drv_lif_p2p_ae_sci_lo_8_set( %u)\n", cfgp2psci_lo_8);
        if(!err) ag_drv_lif_p2p_ae_sci_lo_8_set(cfgp2psci_lo_8);
        if(!err) ag_drv_lif_p2p_ae_sci_lo_8_get( &cfgp2psci_lo_8);
        if(!err) bdmf_session_print(session, "ag_drv_lif_p2p_ae_sci_lo_8_get( %u)\n", cfgp2psci_lo_8);
        if(err || cfgp2psci_lo_8!=gtmv(m, 32))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint32_t cfgp2psci_hi_8=gtmv(m, 32);
        if(!err) bdmf_session_print(session, "ag_drv_lif_p2p_ae_sci_hi_8_set( %u)\n", cfgp2psci_hi_8);
        if(!err) ag_drv_lif_p2p_ae_sci_hi_8_set(cfgp2psci_hi_8);
        if(!err) ag_drv_lif_p2p_ae_sci_hi_8_get( &cfgp2psci_hi_8);
        if(!err) bdmf_session_print(session, "ag_drv_lif_p2p_ae_sci_hi_8_get( %u)\n", cfgp2psci_hi_8);
        if(err || cfgp2psci_hi_8!=gtmv(m, 32))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint32_t cfgp2psci_lo_9=gtmv(m, 32);
        if(!err) bdmf_session_print(session, "ag_drv_lif_p2p_ae_sci_lo_9_set( %u)\n", cfgp2psci_lo_9);
        if(!err) ag_drv_lif_p2p_ae_sci_lo_9_set(cfgp2psci_lo_9);
        if(!err) ag_drv_lif_p2p_ae_sci_lo_9_get( &cfgp2psci_lo_9);
        if(!err) bdmf_session_print(session, "ag_drv_lif_p2p_ae_sci_lo_9_get( %u)\n", cfgp2psci_lo_9);
        if(err || cfgp2psci_lo_9!=gtmv(m, 32))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint32_t cfgp2psci_hi_9=gtmv(m, 32);
        if(!err) bdmf_session_print(session, "ag_drv_lif_p2p_ae_sci_hi_9_set( %u)\n", cfgp2psci_hi_9);
        if(!err) ag_drv_lif_p2p_ae_sci_hi_9_set(cfgp2psci_hi_9);
        if(!err) ag_drv_lif_p2p_ae_sci_hi_9_get( &cfgp2psci_hi_9);
        if(!err) bdmf_session_print(session, "ag_drv_lif_p2p_ae_sci_hi_9_get( %u)\n", cfgp2psci_hi_9);
        if(err || cfgp2psci_hi_9!=gtmv(m, 32))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint32_t cfgp2psci_lo_10=gtmv(m, 32);
        if(!err) bdmf_session_print(session, "ag_drv_lif_p2p_ae_sci_lo_10_set( %u)\n", cfgp2psci_lo_10);
        if(!err) ag_drv_lif_p2p_ae_sci_lo_10_set(cfgp2psci_lo_10);
        if(!err) ag_drv_lif_p2p_ae_sci_lo_10_get( &cfgp2psci_lo_10);
        if(!err) bdmf_session_print(session, "ag_drv_lif_p2p_ae_sci_lo_10_get( %u)\n", cfgp2psci_lo_10);
        if(err || cfgp2psci_lo_10!=gtmv(m, 32))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint32_t cfgp2psci_hi_10=gtmv(m, 32);
        if(!err) bdmf_session_print(session, "ag_drv_lif_p2p_ae_sci_hi_10_set( %u)\n", cfgp2psci_hi_10);
        if(!err) ag_drv_lif_p2p_ae_sci_hi_10_set(cfgp2psci_hi_10);
        if(!err) ag_drv_lif_p2p_ae_sci_hi_10_get( &cfgp2psci_hi_10);
        if(!err) bdmf_session_print(session, "ag_drv_lif_p2p_ae_sci_hi_10_get( %u)\n", cfgp2psci_hi_10);
        if(err || cfgp2psci_hi_10!=gtmv(m, 32))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint32_t cfgp2psci_lo_11=gtmv(m, 32);
        if(!err) bdmf_session_print(session, "ag_drv_lif_p2p_ae_sci_lo_11_set( %u)\n", cfgp2psci_lo_11);
        if(!err) ag_drv_lif_p2p_ae_sci_lo_11_set(cfgp2psci_lo_11);
        if(!err) ag_drv_lif_p2p_ae_sci_lo_11_get( &cfgp2psci_lo_11);
        if(!err) bdmf_session_print(session, "ag_drv_lif_p2p_ae_sci_lo_11_get( %u)\n", cfgp2psci_lo_11);
        if(err || cfgp2psci_lo_11!=gtmv(m, 32))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint32_t cfgp2psci_hi_11=gtmv(m, 32);
        if(!err) bdmf_session_print(session, "ag_drv_lif_p2p_ae_sci_hi_11_set( %u)\n", cfgp2psci_hi_11);
        if(!err) ag_drv_lif_p2p_ae_sci_hi_11_set(cfgp2psci_hi_11);
        if(!err) ag_drv_lif_p2p_ae_sci_hi_11_get( &cfgp2psci_hi_11);
        if(!err) bdmf_session_print(session, "ag_drv_lif_p2p_ae_sci_hi_11_get( %u)\n", cfgp2psci_hi_11);
        if(err || cfgp2psci_hi_11!=gtmv(m, 32))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint32_t cfgp2psci_lo_12=gtmv(m, 32);
        if(!err) bdmf_session_print(session, "ag_drv_lif_p2p_ae_sci_lo_12_set( %u)\n", cfgp2psci_lo_12);
        if(!err) ag_drv_lif_p2p_ae_sci_lo_12_set(cfgp2psci_lo_12);
        if(!err) ag_drv_lif_p2p_ae_sci_lo_12_get( &cfgp2psci_lo_12);
        if(!err) bdmf_session_print(session, "ag_drv_lif_p2p_ae_sci_lo_12_get( %u)\n", cfgp2psci_lo_12);
        if(err || cfgp2psci_lo_12!=gtmv(m, 32))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint32_t cfgp2psci_hi_12=gtmv(m, 32);
        if(!err) bdmf_session_print(session, "ag_drv_lif_p2p_ae_sci_hi_12_set( %u)\n", cfgp2psci_hi_12);
        if(!err) ag_drv_lif_p2p_ae_sci_hi_12_set(cfgp2psci_hi_12);
        if(!err) ag_drv_lif_p2p_ae_sci_hi_12_get( &cfgp2psci_hi_12);
        if(!err) bdmf_session_print(session, "ag_drv_lif_p2p_ae_sci_hi_12_get( %u)\n", cfgp2psci_hi_12);
        if(err || cfgp2psci_hi_12!=gtmv(m, 32))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint32_t cfgp2psci_lo_13=gtmv(m, 32);
        if(!err) bdmf_session_print(session, "ag_drv_lif_p2p_ae_sci_lo_13_set( %u)\n", cfgp2psci_lo_13);
        if(!err) ag_drv_lif_p2p_ae_sci_lo_13_set(cfgp2psci_lo_13);
        if(!err) ag_drv_lif_p2p_ae_sci_lo_13_get( &cfgp2psci_lo_13);
        if(!err) bdmf_session_print(session, "ag_drv_lif_p2p_ae_sci_lo_13_get( %u)\n", cfgp2psci_lo_13);
        if(err || cfgp2psci_lo_13!=gtmv(m, 32))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint32_t cfgp2psci_hi_13=gtmv(m, 32);
        if(!err) bdmf_session_print(session, "ag_drv_lif_p2p_ae_sci_hi_13_set( %u)\n", cfgp2psci_hi_13);
        if(!err) ag_drv_lif_p2p_ae_sci_hi_13_set(cfgp2psci_hi_13);
        if(!err) ag_drv_lif_p2p_ae_sci_hi_13_get( &cfgp2psci_hi_13);
        if(!err) bdmf_session_print(session, "ag_drv_lif_p2p_ae_sci_hi_13_get( %u)\n", cfgp2psci_hi_13);
        if(err || cfgp2psci_hi_13!=gtmv(m, 32))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint32_t cfgp2psci_lo_14=gtmv(m, 32);
        if(!err) bdmf_session_print(session, "ag_drv_lif_p2p_ae_sci_lo_14_set( %u)\n", cfgp2psci_lo_14);
        if(!err) ag_drv_lif_p2p_ae_sci_lo_14_set(cfgp2psci_lo_14);
        if(!err) ag_drv_lif_p2p_ae_sci_lo_14_get( &cfgp2psci_lo_14);
        if(!err) bdmf_session_print(session, "ag_drv_lif_p2p_ae_sci_lo_14_get( %u)\n", cfgp2psci_lo_14);
        if(err || cfgp2psci_lo_14!=gtmv(m, 32))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint32_t cfgp2psci_hi_14=gtmv(m, 32);
        if(!err) bdmf_session_print(session, "ag_drv_lif_p2p_ae_sci_hi_14_set( %u)\n", cfgp2psci_hi_14);
        if(!err) ag_drv_lif_p2p_ae_sci_hi_14_set(cfgp2psci_hi_14);
        if(!err) ag_drv_lif_p2p_ae_sci_hi_14_get( &cfgp2psci_hi_14);
        if(!err) bdmf_session_print(session, "ag_drv_lif_p2p_ae_sci_hi_14_get( %u)\n", cfgp2psci_hi_14);
        if(err || cfgp2psci_hi_14!=gtmv(m, 32))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint32_t cfgp2psci_lo_15=gtmv(m, 32);
        if(!err) bdmf_session_print(session, "ag_drv_lif_p2p_ae_sci_lo_15_set( %u)\n", cfgp2psci_lo_15);
        if(!err) ag_drv_lif_p2p_ae_sci_lo_15_set(cfgp2psci_lo_15);
        if(!err) ag_drv_lif_p2p_ae_sci_lo_15_get( &cfgp2psci_lo_15);
        if(!err) bdmf_session_print(session, "ag_drv_lif_p2p_ae_sci_lo_15_get( %u)\n", cfgp2psci_lo_15);
        if(err || cfgp2psci_lo_15!=gtmv(m, 32))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint32_t cfgp2psci_hi_15=gtmv(m, 32);
        if(!err) bdmf_session_print(session, "ag_drv_lif_p2p_ae_sci_hi_15_set( %u)\n", cfgp2psci_hi_15);
        if(!err) ag_drv_lif_p2p_ae_sci_hi_15_set(cfgp2psci_hi_15);
        if(!err) ag_drv_lif_p2p_ae_sci_hi_15_get( &cfgp2psci_hi_15);
        if(!err) bdmf_session_print(session, "ag_drv_lif_p2p_ae_sci_hi_15_get( %u)\n", cfgp2psci_hi_15);
        if(err || cfgp2psci_hi_15!=gtmv(m, 32))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint32_t keyupsel_hi=gtmv(m, 32);
        if(!err) ag_drv_lif_sec_up_key_stat_1_get( &keyupsel_hi);
        if(!err) bdmf_session_print(session, "ag_drv_lif_sec_up_key_stat_1_get( %u)\n", keyupsel_hi);
    }
    {
        uint32_t keysel_hi=gtmv(m, 32);
        if(!err) ag_drv_lif_sec_key_sel_1_get( &keysel_hi);
        if(!err) bdmf_session_print(session, "ag_drv_lif_sec_key_sel_1_get( %u)\n", keysel_hi);
    }
    {
        uint8_t cf_plaintxt_oh_b_idle_pad=gtmv(m, 6);
        uint8_t cf_plaintxt_oh_a_idle_pad=gtmv(m, 6);
        if(!err) bdmf_session_print(session, "ag_drv_lif_pon_sec_tx_plaintxt_ae_pad_control_set( %u %u)\n", cf_plaintxt_oh_b_idle_pad, cf_plaintxt_oh_a_idle_pad);
        if(!err) ag_drv_lif_pon_sec_tx_plaintxt_ae_pad_control_set(cf_plaintxt_oh_b_idle_pad, cf_plaintxt_oh_a_idle_pad);
        if(!err) ag_drv_lif_pon_sec_tx_plaintxt_ae_pad_control_get( &cf_plaintxt_oh_b_idle_pad, &cf_plaintxt_oh_a_idle_pad);
        if(!err) bdmf_session_print(session, "ag_drv_lif_pon_sec_tx_plaintxt_ae_pad_control_get( %u %u)\n", cf_plaintxt_oh_b_idle_pad, cf_plaintxt_oh_a_idle_pad);
        if(err || cf_plaintxt_oh_b_idle_pad!=gtmv(m, 6) || cf_plaintxt_oh_a_idle_pad!=gtmv(m, 6))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint16_t cf_autoneg_linktimer=gtmv(m, 16);
        bdmf_boolean cf_autoneg_mode_sel=gtmv(m, 1);
        bdmf_boolean cf_autoneg_restart=gtmv(m, 1);
        bdmf_boolean cf_autoneg_en=gtmv(m, 1);
        if(!err) bdmf_session_print(session, "ag_drv_lif_p2p_autoneg_control_set( %u %u %u %u)\n", cf_autoneg_linktimer, cf_autoneg_mode_sel, cf_autoneg_restart, cf_autoneg_en);
        if(!err) ag_drv_lif_p2p_autoneg_control_set(cf_autoneg_linktimer, cf_autoneg_mode_sel, cf_autoneg_restart, cf_autoneg_en);
        if(!err) ag_drv_lif_p2p_autoneg_control_get( &cf_autoneg_linktimer, &cf_autoneg_mode_sel, &cf_autoneg_restart, &cf_autoneg_en);
        if(!err) bdmf_session_print(session, "ag_drv_lif_p2p_autoneg_control_get( %u %u %u %u)\n", cf_autoneg_linktimer, cf_autoneg_mode_sel, cf_autoneg_restart, cf_autoneg_en);
        if(err || cf_autoneg_linktimer!=gtmv(m, 16) || cf_autoneg_mode_sel!=gtmv(m, 1) || cf_autoneg_restart!=gtmv(m, 1) || cf_autoneg_en!=gtmv(m, 1))
            return err ? err : BDMF_ERR_IO;
    }
    {
        bdmf_boolean an_lp_remote_fault=gtmv(m, 1);
        bdmf_boolean an_sync_status=gtmv(m, 1);
        bdmf_boolean an_complete=gtmv(m, 1);
        if(!err) ag_drv_lif_p2p_autoneg_status_get( &an_lp_remote_fault, &an_sync_status, &an_complete);
        if(!err) bdmf_session_print(session, "ag_drv_lif_p2p_autoneg_status_get( %u %u %u)\n", an_lp_remote_fault, an_sync_status, an_complete);
    }
    {
        uint16_t cf_lif_p2p_ae_autoneg_config_ability=gtmv(m, 16);
        if(!err) bdmf_session_print(session, "ag_drv_lif_p2p_autoneg_ability_config_reg_set( %u)\n", cf_lif_p2p_ae_autoneg_config_ability);
        if(!err) ag_drv_lif_p2p_autoneg_ability_config_reg_set(cf_lif_p2p_ae_autoneg_config_ability);
        if(!err) ag_drv_lif_p2p_autoneg_ability_config_reg_get( &cf_lif_p2p_ae_autoneg_config_ability);
        if(!err) bdmf_session_print(session, "ag_drv_lif_p2p_autoneg_ability_config_reg_get( %u)\n", cf_lif_p2p_ae_autoneg_config_ability);
        if(err || cf_lif_p2p_ae_autoneg_config_ability!=gtmv(m, 16))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint16_t cf_lif_p2p_ae_autoneg_lp_ability_read=gtmv(m, 16);
        if(!err) ag_drv_lif_p2p_autoneg_link_partner_ability_config_read_get( &cf_lif_p2p_ae_autoneg_lp_ability_read);
        if(!err) bdmf_session_print(session, "ag_drv_lif_p2p_autoneg_link_partner_ability_config_read_get( %u)\n", cf_lif_p2p_ae_autoneg_lp_ability_read);
    }
    return err;
}

static int bcm_lif_cli_address(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
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
    case bdmf_address_pon_control : reg = &RU_REG(LIF, PON_CONTROL); blk = &RU_BLK(LIF); break;
    case bdmf_address_pon_inter_op_control : reg = &RU_REG(LIF, PON_INTER_OP_CONTROL); blk = &RU_BLK(LIF); break;
    case bdmf_address_fec_control : reg = &RU_REG(LIF, FEC_CONTROL); blk = &RU_BLK(LIF); break;
    case bdmf_address_sec_control : reg = &RU_REG(LIF, SEC_CONTROL); blk = &RU_BLK(LIF); break;
    case bdmf_address_macsec : reg = &RU_REG(LIF, MACSEC); blk = &RU_BLK(LIF); break;
    case bdmf_address_int_status : reg = &RU_REG(LIF, INT_STATUS); blk = &RU_BLK(LIF); break;
    case bdmf_address_int_mask : reg = &RU_REG(LIF, INT_MASK); blk = &RU_BLK(LIF); break;
    case bdmf_address_data_port_command : reg = &RU_REG(LIF, DATA_PORT_COMMAND); blk = &RU_BLK(LIF); break;
    case bdmf_address_data_port_data : reg = &RU_REG(LIF, DATA_PORT_DATA); blk = &RU_BLK(LIF); break;
    case bdmf_address_llid_0 : reg = &RU_REG(LIF, LLID_0); blk = &RU_BLK(LIF); break;
    case bdmf_address_llid_1 : reg = &RU_REG(LIF, LLID_1); blk = &RU_BLK(LIF); break;
    case bdmf_address_llid_2 : reg = &RU_REG(LIF, LLID_2); blk = &RU_BLK(LIF); break;
    case bdmf_address_llid_3 : reg = &RU_REG(LIF, LLID_3); blk = &RU_BLK(LIF); break;
    case bdmf_address_llid_4 : reg = &RU_REG(LIF, LLID_4); blk = &RU_BLK(LIF); break;
    case bdmf_address_llid_5 : reg = &RU_REG(LIF, LLID_5); blk = &RU_BLK(LIF); break;
    case bdmf_address_llid_6 : reg = &RU_REG(LIF, LLID_6); blk = &RU_BLK(LIF); break;
    case bdmf_address_llid_7 : reg = &RU_REG(LIF, LLID_7); blk = &RU_BLK(LIF); break;
    case bdmf_address_llid_16 : reg = &RU_REG(LIF, LLID_16); blk = &RU_BLK(LIF); break;
    case bdmf_address_llid_17 : reg = &RU_REG(LIF, LLID_17); blk = &RU_BLK(LIF); break;
    case bdmf_address_llid_18 : reg = &RU_REG(LIF, LLID_18); blk = &RU_BLK(LIF); break;
    case bdmf_address_llid_19 : reg = &RU_REG(LIF, LLID_19); blk = &RU_BLK(LIF); break;
    case bdmf_address_llid_20 : reg = &RU_REG(LIF, LLID_20); blk = &RU_BLK(LIF); break;
    case bdmf_address_llid_21 : reg = &RU_REG(LIF, LLID_21); blk = &RU_BLK(LIF); break;
    case bdmf_address_llid_22 : reg = &RU_REG(LIF, LLID_22); blk = &RU_BLK(LIF); break;
    case bdmf_address_llid_23 : reg = &RU_REG(LIF, LLID_23); blk = &RU_BLK(LIF); break;
    case bdmf_address_time_ref_cnt : reg = &RU_REG(LIF, TIME_REF_CNT); blk = &RU_BLK(LIF); break;
    case bdmf_address_timestamp_upd_per : reg = &RU_REG(LIF, TIMESTAMP_UPD_PER); blk = &RU_BLK(LIF); break;
    case bdmf_address_tp_time : reg = &RU_REG(LIF, TP_TIME); blk = &RU_BLK(LIF); break;
    case bdmf_address_mpcp_time : reg = &RU_REG(LIF, MPCP_TIME); blk = &RU_BLK(LIF); break;
    case bdmf_address_maxlen_ctr : reg = &RU_REG(LIF, MAXLEN_CTR); blk = &RU_BLK(LIF); break;
    case bdmf_address_laser_on_delta : reg = &RU_REG(LIF, LASER_ON_DELTA); blk = &RU_BLK(LIF); break;
    case bdmf_address_laser_off_idle : reg = &RU_REG(LIF, LASER_OFF_IDLE); blk = &RU_BLK(LIF); break;
    case bdmf_address_fec_init_idle : reg = &RU_REG(LIF, FEC_INIT_IDLE); blk = &RU_BLK(LIF); break;
    case bdmf_address_fec_err_allow : reg = &RU_REG(LIF, FEC_ERR_ALLOW); blk = &RU_BLK(LIF); break;
    case bdmf_address_sec_key_sel : reg = &RU_REG(LIF, SEC_KEY_SEL); blk = &RU_BLK(LIF); break;
    case bdmf_address_dn_encrypt_stat : reg = &RU_REG(LIF, DN_ENCRYPT_STAT); blk = &RU_BLK(LIF); break;
    case bdmf_address_sec_up_key_stat : reg = &RU_REG(LIF, SEC_UP_KEY_STAT); blk = &RU_BLK(LIF); break;
    case bdmf_address_sec_up_encrypt_stat : reg = &RU_REG(LIF, SEC_UP_ENCRYPT_STAT); blk = &RU_BLK(LIF); break;
    case bdmf_address_sec_up_mpcp_offset : reg = &RU_REG(LIF, SEC_UP_MPCP_OFFSET); blk = &RU_BLK(LIF); break;
    case bdmf_address_fec_per_llid : reg = &RU_REG(LIF, FEC_PER_LLID); blk = &RU_BLK(LIF); break;
    case bdmf_address_rx_line_code_err_cnt : reg = &RU_REG(LIF, RX_LINE_CODE_ERR_CNT); blk = &RU_BLK(LIF); break;
    case bdmf_address_rx_agg_mpcp_frm : reg = &RU_REG(LIF, RX_AGG_MPCP_FRM); blk = &RU_BLK(LIF); break;
    case bdmf_address_rx_agg_good_frm : reg = &RU_REG(LIF, RX_AGG_GOOD_FRM); blk = &RU_BLK(LIF); break;
    case bdmf_address_rx_agg_good_byte : reg = &RU_REG(LIF, RX_AGG_GOOD_BYTE); blk = &RU_BLK(LIF); break;
    case bdmf_address_rx_agg_undersz_frm : reg = &RU_REG(LIF, RX_AGG_UNDERSZ_FRM); blk = &RU_BLK(LIF); break;
    case bdmf_address_rx_agg_oversz_frm : reg = &RU_REG(LIF, RX_AGG_OVERSZ_FRM); blk = &RU_BLK(LIF); break;
    case bdmf_address_rx_agg_crc8_frm : reg = &RU_REG(LIF, RX_AGG_CRC8_FRM); blk = &RU_BLK(LIF); break;
    case bdmf_address_rx_agg_fec_frm : reg = &RU_REG(LIF, RX_AGG_FEC_FRM); blk = &RU_BLK(LIF); break;
    case bdmf_address_rx_agg_fec_byte : reg = &RU_REG(LIF, RX_AGG_FEC_BYTE); blk = &RU_BLK(LIF); break;
    case bdmf_address_rx_agg_fec_exc_err_frm : reg = &RU_REG(LIF, RX_AGG_FEC_EXC_ERR_FRM); blk = &RU_BLK(LIF); break;
    case bdmf_address_rx_agg_nonfec_good_frm : reg = &RU_REG(LIF, RX_AGG_NONFEC_GOOD_FRM); blk = &RU_BLK(LIF); break;
    case bdmf_address_rx_agg_nonfec_good_byte : reg = &RU_REG(LIF, RX_AGG_NONFEC_GOOD_BYTE); blk = &RU_BLK(LIF); break;
    case bdmf_address_rx_agg_err_bytes : reg = &RU_REG(LIF, RX_AGG_ERR_BYTES); blk = &RU_BLK(LIF); break;
    case bdmf_address_rx_agg_err_zeroes : reg = &RU_REG(LIF, RX_AGG_ERR_ZEROES); blk = &RU_BLK(LIF); break;
    case bdmf_address_rx_agg_no_err_blks : reg = &RU_REG(LIF, RX_AGG_NO_ERR_BLKS); blk = &RU_BLK(LIF); break;
    case bdmf_address_rx_agg_cor_blks : reg = &RU_REG(LIF, RX_AGG_COR_BLKS); blk = &RU_BLK(LIF); break;
    case bdmf_address_rx_agg_uncor_blks : reg = &RU_REG(LIF, RX_AGG_UNCOR_BLKS); blk = &RU_BLK(LIF); break;
    case bdmf_address_rx_agg_err_ones : reg = &RU_REG(LIF, RX_AGG_ERR_ONES); blk = &RU_BLK(LIF); break;
    case bdmf_address_rx_agg_err_frm : reg = &RU_REG(LIF, RX_AGG_ERR_FRM); blk = &RU_BLK(LIF); break;
    case bdmf_address_tx_pkt_cnt : reg = &RU_REG(LIF, TX_PKT_CNT); blk = &RU_BLK(LIF); break;
    case bdmf_address_tx_byte_cnt : reg = &RU_REG(LIF, TX_BYTE_CNT); blk = &RU_BLK(LIF); break;
    case bdmf_address_tx_non_fec_pkt_cnt : reg = &RU_REG(LIF, TX_NON_FEC_PKT_CNT); blk = &RU_BLK(LIF); break;
    case bdmf_address_tx_non_fec_byte_cnt : reg = &RU_REG(LIF, TX_NON_FEC_BYTE_CNT); blk = &RU_BLK(LIF); break;
    case bdmf_address_tx_fec_pkt_cnt : reg = &RU_REG(LIF, TX_FEC_PKT_CNT); blk = &RU_BLK(LIF); break;
    case bdmf_address_tx_fec_byte_cnt : reg = &RU_REG(LIF, TX_FEC_BYTE_CNT); blk = &RU_BLK(LIF); break;
    case bdmf_address_tx_fec_blk_cnt : reg = &RU_REG(LIF, TX_FEC_BLK_CNT); blk = &RU_BLK(LIF); break;
    case bdmf_address_tx_mpcp_pkt_cnt : reg = &RU_REG(LIF, TX_MPCP_PKT_CNT); blk = &RU_BLK(LIF); break;
    case bdmf_address_debug_tx_data_pkt_cnt : reg = &RU_REG(LIF, DEBUG_TX_DATA_PKT_CNT); blk = &RU_BLK(LIF); break;
    case bdmf_address_fec_llid_status : reg = &RU_REG(LIF, FEC_LLID_STATUS); blk = &RU_BLK(LIF); break;
    case bdmf_address_sec_rx_tek_ig_iv_llid : reg = &RU_REG(LIF, SEC_RX_TEK_IG_IV_LLID); blk = &RU_BLK(LIF); break;
    case bdmf_address_pon_ber_interv_thresh : reg = &RU_REG(LIF, PON_BER_INTERV_THRESH); blk = &RU_BLK(LIF); break;
    case bdmf_address_lsr_mon_a_ctrl : reg = &RU_REG(LIF, LSR_MON_A_CTRL); blk = &RU_BLK(LIF); break;
    case bdmf_address_lsr_mon_a_max_thr : reg = &RU_REG(LIF, LSR_MON_A_MAX_THR); blk = &RU_BLK(LIF); break;
    case bdmf_address_lsr_mon_a_bst_len : reg = &RU_REG(LIF, LSR_MON_A_BST_LEN); blk = &RU_BLK(LIF); break;
    case bdmf_address_lsr_mon_a_bst_cnt : reg = &RU_REG(LIF, LSR_MON_A_BST_CNT); blk = &RU_BLK(LIF); break;
    case bdmf_address_debug_pon_sm : reg = &RU_REG(LIF, DEBUG_PON_SM); blk = &RU_BLK(LIF); break;
    case bdmf_address_debug_fec_sm : reg = &RU_REG(LIF, DEBUG_FEC_SM); blk = &RU_BLK(LIF); break;
    case bdmf_address_ae_pktnum_window : reg = &RU_REG(LIF, AE_PKTNUM_WINDOW); blk = &RU_BLK(LIF); break;
    case bdmf_address_ae_pktnum_thresh : reg = &RU_REG(LIF, AE_PKTNUM_THRESH); blk = &RU_BLK(LIF); break;
    case bdmf_address_ae_pktnum_stat : reg = &RU_REG(LIF, AE_PKTNUM_STAT); blk = &RU_BLK(LIF); break;
    case bdmf_address_llid_8 : reg = &RU_REG(LIF, LLID_8); blk = &RU_BLK(LIF); break;
    case bdmf_address_llid_9 : reg = &RU_REG(LIF, LLID_9); blk = &RU_BLK(LIF); break;
    case bdmf_address_llid_10 : reg = &RU_REG(LIF, LLID_10); blk = &RU_BLK(LIF); break;
    case bdmf_address_llid_11 : reg = &RU_REG(LIF, LLID_11); blk = &RU_BLK(LIF); break;
    case bdmf_address_llid_12 : reg = &RU_REG(LIF, LLID_12); blk = &RU_BLK(LIF); break;
    case bdmf_address_llid_13 : reg = &RU_REG(LIF, LLID_13); blk = &RU_BLK(LIF); break;
    case bdmf_address_llid_14 : reg = &RU_REG(LIF, LLID_14); blk = &RU_BLK(LIF); break;
    case bdmf_address_llid_15 : reg = &RU_REG(LIF, LLID_15); blk = &RU_BLK(LIF); break;
    case bdmf_address_llid_24 : reg = &RU_REG(LIF, LLID_24); blk = &RU_BLK(LIF); break;
    case bdmf_address_llid_25 : reg = &RU_REG(LIF, LLID_25); blk = &RU_BLK(LIF); break;
    case bdmf_address_llid_26 : reg = &RU_REG(LIF, LLID_26); blk = &RU_BLK(LIF); break;
    case bdmf_address_llid_27 : reg = &RU_REG(LIF, LLID_27); blk = &RU_BLK(LIF); break;
    case bdmf_address_llid_28 : reg = &RU_REG(LIF, LLID_28); blk = &RU_BLK(LIF); break;
    case bdmf_address_llid_29 : reg = &RU_REG(LIF, LLID_29); blk = &RU_BLK(LIF); break;
    case bdmf_address_llid_30 : reg = &RU_REG(LIF, LLID_30); blk = &RU_BLK(LIF); break;
    case bdmf_address_llid_31 : reg = &RU_REG(LIF, LLID_31); blk = &RU_BLK(LIF); break;
    case bdmf_address_vlan_type : reg = &RU_REG(LIF, VLAN_TYPE); blk = &RU_BLK(LIF); break;
    case bdmf_address_p2p_ae_sci_en : reg = &RU_REG(LIF, P2P_AE_SCI_EN); blk = &RU_BLK(LIF); break;
    case bdmf_address_p2p_ae_sci_lo_0 : reg = &RU_REG(LIF, P2P_AE_SCI_LO_0); blk = &RU_BLK(LIF); break;
    case bdmf_address_p2p_ae_sci_hi_0 : reg = &RU_REG(LIF, P2P_AE_SCI_HI_0); blk = &RU_BLK(LIF); break;
    case bdmf_address_p2p_ae_sci_lo_1 : reg = &RU_REG(LIF, P2P_AE_SCI_LO_1); blk = &RU_BLK(LIF); break;
    case bdmf_address_p2p_ae_sci_hi_1 : reg = &RU_REG(LIF, P2P_AE_SCI_HI_1); blk = &RU_BLK(LIF); break;
    case bdmf_address_p2p_ae_sci_lo_2 : reg = &RU_REG(LIF, P2P_AE_SCI_LO_2); blk = &RU_BLK(LIF); break;
    case bdmf_address_p2p_ae_sci_hi_2 : reg = &RU_REG(LIF, P2P_AE_SCI_HI_2); blk = &RU_BLK(LIF); break;
    case bdmf_address_p2p_ae_sci_lo_3 : reg = &RU_REG(LIF, P2P_AE_SCI_LO_3); blk = &RU_BLK(LIF); break;
    case bdmf_address_p2p_ae_sci_hi_3 : reg = &RU_REG(LIF, P2P_AE_SCI_HI_3); blk = &RU_BLK(LIF); break;
    case bdmf_address_p2p_ae_sci_lo_4 : reg = &RU_REG(LIF, P2P_AE_SCI_LO_4); blk = &RU_BLK(LIF); break;
    case bdmf_address_p2p_ae_sci_hi_4 : reg = &RU_REG(LIF, P2P_AE_SCI_HI_4); blk = &RU_BLK(LIF); break;
    case bdmf_address_p2p_ae_sci_lo_5 : reg = &RU_REG(LIF, P2P_AE_SCI_LO_5); blk = &RU_BLK(LIF); break;
    case bdmf_address_p2p_ae_sci_hi_5 : reg = &RU_REG(LIF, P2P_AE_SCI_HI_5); blk = &RU_BLK(LIF); break;
    case bdmf_address_p2p_ae_sci_lo_6 : reg = &RU_REG(LIF, P2P_AE_SCI_LO_6); blk = &RU_BLK(LIF); break;
    case bdmf_address_p2p_ae_sci_hi_6 : reg = &RU_REG(LIF, P2P_AE_SCI_HI_6); blk = &RU_BLK(LIF); break;
    case bdmf_address_p2p_ae_sci_lo_7 : reg = &RU_REG(LIF, P2P_AE_SCI_LO_7); blk = &RU_BLK(LIF); break;
    case bdmf_address_p2p_ae_sci_hi_7 : reg = &RU_REG(LIF, P2P_AE_SCI_HI_7); blk = &RU_BLK(LIF); break;
    case bdmf_address_p2p_ae_sci_lo_8 : reg = &RU_REG(LIF, P2P_AE_SCI_LO_8); blk = &RU_BLK(LIF); break;
    case bdmf_address_p2p_ae_sci_hi_8 : reg = &RU_REG(LIF, P2P_AE_SCI_HI_8); blk = &RU_BLK(LIF); break;
    case bdmf_address_p2p_ae_sci_lo_9 : reg = &RU_REG(LIF, P2P_AE_SCI_LO_9); blk = &RU_BLK(LIF); break;
    case bdmf_address_p2p_ae_sci_hi_9 : reg = &RU_REG(LIF, P2P_AE_SCI_HI_9); blk = &RU_BLK(LIF); break;
    case bdmf_address_p2p_ae_sci_lo_10 : reg = &RU_REG(LIF, P2P_AE_SCI_LO_10); blk = &RU_BLK(LIF); break;
    case bdmf_address_p2p_ae_sci_hi_10 : reg = &RU_REG(LIF, P2P_AE_SCI_HI_10); blk = &RU_BLK(LIF); break;
    case bdmf_address_p2p_ae_sci_lo_11 : reg = &RU_REG(LIF, P2P_AE_SCI_LO_11); blk = &RU_BLK(LIF); break;
    case bdmf_address_p2p_ae_sci_hi_11 : reg = &RU_REG(LIF, P2P_AE_SCI_HI_11); blk = &RU_BLK(LIF); break;
    case bdmf_address_p2p_ae_sci_lo_12 : reg = &RU_REG(LIF, P2P_AE_SCI_LO_12); blk = &RU_BLK(LIF); break;
    case bdmf_address_p2p_ae_sci_hi_12 : reg = &RU_REG(LIF, P2P_AE_SCI_HI_12); blk = &RU_BLK(LIF); break;
    case bdmf_address_p2p_ae_sci_lo_13 : reg = &RU_REG(LIF, P2P_AE_SCI_LO_13); blk = &RU_BLK(LIF); break;
    case bdmf_address_p2p_ae_sci_hi_13 : reg = &RU_REG(LIF, P2P_AE_SCI_HI_13); blk = &RU_BLK(LIF); break;
    case bdmf_address_p2p_ae_sci_lo_14 : reg = &RU_REG(LIF, P2P_AE_SCI_LO_14); blk = &RU_BLK(LIF); break;
    case bdmf_address_p2p_ae_sci_hi_14 : reg = &RU_REG(LIF, P2P_AE_SCI_HI_14); blk = &RU_BLK(LIF); break;
    case bdmf_address_p2p_ae_sci_lo_15 : reg = &RU_REG(LIF, P2P_AE_SCI_LO_15); blk = &RU_BLK(LIF); break;
    case bdmf_address_p2p_ae_sci_hi_15 : reg = &RU_REG(LIF, P2P_AE_SCI_HI_15); blk = &RU_BLK(LIF); break;
    case bdmf_address_sec_up_key_stat_1 : reg = &RU_REG(LIF, SEC_UP_KEY_STAT_1); blk = &RU_BLK(LIF); break;
    case bdmf_address_sec_key_sel_1 : reg = &RU_REG(LIF, SEC_KEY_SEL_1); blk = &RU_BLK(LIF); break;
    case bdmf_address_pon_sec_tx_plaintxt_ae_pad_control : reg = &RU_REG(LIF, PON_SEC_TX_PLAINTXT_AE_PAD_CONTROL); blk = &RU_BLK(LIF); break;
    case bdmf_address_p2p_autoneg_control : reg = &RU_REG(LIF, P2P_AUTONEG_CONTROL); blk = &RU_BLK(LIF); break;
    case bdmf_address_p2p_autoneg_status : reg = &RU_REG(LIF, P2P_AUTONEG_STATUS); blk = &RU_BLK(LIF); break;
    case bdmf_address_p2p_autoneg_ability_config_reg : reg = &RU_REG(LIF, P2P_AUTONEG_ABILITY_CONFIG_REG); blk = &RU_BLK(LIF); break;
    case bdmf_address_p2p_autoneg_link_partner_ability_config_read : reg = &RU_REG(LIF, P2P_AUTONEG_LINK_PARTNER_ABILITY_CONFIG_READ); blk = &RU_BLK(LIF); break;
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

bdmfmon_handle_t ag_drv_lif_cli_init(bdmfmon_handle_t driver_dir)
{
    bdmfmon_handle_t dir;

    if ((dir = bdmfmon_dir_find(driver_dir, "lif"))!=NULL)
        return dir;
    dir = bdmfmon_dir_add(driver_dir, "lif", "lif", BDMF_ACCESS_ADMIN, NULL);

    {
        static bdmfmon_cmd_parm_t set_pon_control[]={
            BDMFMON_MAKE_PARM("cfgdisruntfilter", "cfgdisruntfilter", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("cfmaxcommaerrcnt", "cfmaxcommaerrcnt", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("cfsyncsmselect", "cfsyncsmselect", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("cfponrxforcenonfecabort", "cfponrxforcenonfecabort", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("cfponrxforcefecabort", "cfponrxforcefecabort", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("cfgrxdatabitflip", "cfgrxdatabitflip", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("cfgenmpcpoampad", "cfgenmpcpoampad", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("cfgentxidlepkt", "cfgentxidlepkt", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("cfenablesoftwaresynchold", "cfenablesoftwaresynchold", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("cfenableextendsync", "cfenableextendsync", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("cfenablequicksync", "cfenablequicksync", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("cfppsen", "cfppsen", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("cfppsclkrbc", "cfppsclkrbc", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("cfrx2txlpback", "cfrx2txlpback", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("cftx2rxlpback", "cftx2rxlpback", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("cftxdataendurlon", "cftxdataendurlon", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("cfp2pmode", "cfp2pmode", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("cfp2pshortpre", "cfp2pshortpre", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("cflaseren", "cflaseren", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("cftxlaseron", "cftxlaseron", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("cftxlaseronacthi", "cftxlaseronacthi", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("liftxrstn_pre", "liftxrstn_pre", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("lifrxrstn_pre", "lifrxrstn_pre", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("liftxen", "liftxen", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("lifrxen", "lifrxen", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_pon_inter_op_control[]={
            BDMFMON_MAKE_PARM("cfipgfilter", "cfipgfilter", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("cfdisableloslaserblock", "cfdisableloslaserblock", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("cfgllidpromiscuousmode", "cfgllidpromiscuousmode", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("cfgllidmodmsk", "cfgllidmodmsk", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("cfusefecipg", "cfusefecipg", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("cfrxcrc8invchk", "cfrxcrc8invchk", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("cfrxcrc8bitswap", "cfrxcrc8bitswap", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("cfrxcrc8msb2lsb", "cfrxcrc8msb2lsb", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("cfrxcrc8disable", "cfrxcrc8disable", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("cftxllidbit15set", "cftxllidbit15set", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("cftxcrc8inv", "cftxcrc8inv", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("cftxcrc8bad", "cftxcrc8bad", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("cftxcrc8bitswap", "cftxcrc8bitswap", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("cftxcrc8msb2lsb", "cftxcrc8msb2lsb", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("cftxshortpre", "cftxshortpre", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("cftxipgcnt", "cftxipgcnt", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("cftxaasynclen", "cftxaasynclen", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("cftxpipedelay", "cftxpipedelay", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_fec_control[]={
            BDMFMON_MAKE_PARM("cffecrxerrorprop", "cffecrxerrorprop", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("cffecrxforcenonfecabort", "cffecrxforcenonfecabort", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("cffecrxforcefecabort", "cffecrxforcefecabort", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("cffecrxenable", "cffecrxenable", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("cffectxfecperllid", "cffectxfecperllid", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("cffectxenable", "cffectxenable", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_sec_control[]={
            BDMFMON_MAKE_PARM("cfgdismpcpencrypt", "cfgdismpcpencrypt", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("cfgdisoamencrypt", "cfgdisoamencrypt", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("cfgsecenshortlen", "cfgsecenshortlen", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("cfgenaereplayprct", "cfgenaereplayprct", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("cfgenlegacyrcc", "cfgenlegacyrcc", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("enfakeupaes", "enfakeupaes", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("enfakednaes", "enfakednaes", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("cfgfecipglen", "cfgfecipglen", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("disdndasaencrpt", "disdndasaencrpt", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("entriplechurn", "entriplechurn", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("enepnmixencrypt", "enepnmixencrypt", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("disupdasaencrpt", "disupdasaencrpt", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("secupencryptscheme", "secupencryptscheme", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("secdnencryptscheme", "secdnencryptscheme", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("secuprstn_pre", "secuprstn_pre", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("secdnrstn_pre", "secdnrstn_pre", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("secenup", "secenup", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("secendn", "secendn", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_macsec[]={
            BDMFMON_MAKE_PARM("cfgmacsecethertype", "cfgmacsecethertype", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_int_status[]={
            BDMFMON_MAKE_PARM("int_sop_sfec_ipg_violation", "int_sop_sfec_ipg_violation", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("laseronmax", "laseronmax", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("laseroff", "laseroff", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("secdnreplayprotctabort", "secdnreplayprotctabort", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("secuppktnumoverflow", "secuppktnumoverflow", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("intlaseroffdurburst", "intlaseroffdurburst", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("intrxberthreshexc", "intrxberthreshexc", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("intfecrxfecrecvstatus", "intfecrxfecrecvstatus", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("intfecrxcorerrfifofullstatus", "intfecrxcorerrfifofullstatus", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("intfecrxcorerrfifounexpempty", "intfecrxcorerrfifounexpempty", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("intfecbufpopemptypush", "intfecbufpopemptypush", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("intfecbufpopemptynopush", "intfecbufpopemptynopush", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("intfecbufpushfull", "intfecbufpushfull", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("intuptimefullupdstat", "intuptimefullupdstat", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("intfroutofalignstat", "intfroutofalignstat", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("intgrntstarttimelagstat", "intgrntstarttimelagstat", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("intabortrxfrmstat", "intabortrxfrmstat", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("intnorxclkstat", "intnorxclkstat", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("intrxmaxlenerrstat", "intrxmaxlenerrstat", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("intrxerraftalignstat", "intrxerraftalignstat", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("intrxsynchacqstat", "intrxsynchacqstat", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("intrxoutofsynchstat", "intrxoutofsynchstat", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_int_mask[]={
            BDMFMON_MAKE_PARM("int_sop_sfec_ipg_violation_mask", "int_sop_sfec_ipg_violation_mask", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("laseronmaxmask", "laseronmaxmask", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("laseroffmask", "laseroffmask", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("secdnreplayprotctabortmsk", "secdnreplayprotctabortmsk", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("secuppktnumoverflowmsk", "secuppktnumoverflowmsk", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("intlaseroffdurburstmask", "intlaseroffdurburstmask", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("intrxberthreshexcmask", "intrxberthreshexcmask", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("intfecrxfecrecvmask", "intfecrxfecrecvmask", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("intfecrxcorerrfifofullmask", "intfecrxcorerrfifofullmask", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("intfecrxcorerrfifounexpemptymask", "intfecrxcorerrfifounexpemptymask", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("intfecbufpopemptypushmask", "intfecbufpopemptypushmask", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("intfecbufpopemptynopushmask", "intfecbufpopemptynopushmask", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("intfecbufpushfullmask", "intfecbufpushfullmask", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("intuptimefullupdmask", "intuptimefullupdmask", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("intfroutofalignmask", "intfroutofalignmask", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("intgrntstarttimelagmask", "intgrntstarttimelagmask", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("intabortrxfrmmask", "intabortrxfrmmask", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("intnorxclkmask", "intnorxclkmask", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("intrxmaxlenerrmask", "intrxmaxlenerrmask", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("intrxerraftalignmask", "intrxerraftalignmask", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("intrxsynchacqmask", "intrxsynchacqmask", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("intrxoutofsynchmask", "intrxoutofsynchmask", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_data_port_command[]={
            BDMFMON_MAKE_PARM("data_port_busy", "data_port_busy", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("data_port_error", "data_port_error", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("ram_select", "ram_select", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("data_port_op_code", "data_port_op_code", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("data_port_addr", "data_port_addr", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_data_port_data[]={
            BDMFMON_MAKE_PARM("portidx", "portidx", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("pbiportdata", "pbiportdata", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_llid_0[]={
            BDMFMON_MAKE_PARM("cfgllid0", "cfgllid0", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_llid_1[]={
            BDMFMON_MAKE_PARM("cfgllid1", "cfgllid1", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_llid_2[]={
            BDMFMON_MAKE_PARM("cfgllid2", "cfgllid2", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_llid_3[]={
            BDMFMON_MAKE_PARM("cfgllid3", "cfgllid3", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_llid_4[]={
            BDMFMON_MAKE_PARM("cfgllid4", "cfgllid4", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_llid_5[]={
            BDMFMON_MAKE_PARM("cfgllid5", "cfgllid5", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_llid_6[]={
            BDMFMON_MAKE_PARM("cfgllid6", "cfgllid6", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_llid_7[]={
            BDMFMON_MAKE_PARM("cfgllid7", "cfgllid7", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_llid_16[]={
            BDMFMON_MAKE_PARM("cfgllid16", "cfgllid16", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_llid_17[]={
            BDMFMON_MAKE_PARM("cfgllid17", "cfgllid17", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_llid_18[]={
            BDMFMON_MAKE_PARM("cfgllid18", "cfgllid18", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_llid_19[]={
            BDMFMON_MAKE_PARM("cfgllid19", "cfgllid19", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_llid_20[]={
            BDMFMON_MAKE_PARM("cfgllid20", "cfgllid20", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_llid_21[]={
            BDMFMON_MAKE_PARM("cfgllid21", "cfgllid21", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_llid_22[]={
            BDMFMON_MAKE_PARM("cfgllid22", "cfgllid22", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_llid_23[]={
            BDMFMON_MAKE_PARM("cfgllid23", "cfgllid23", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_time_ref_cnt[]={
            BDMFMON_MAKE_PARM("cffullupdatevalue", "cffullupdatevalue", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("cfmaxnegvalue", "cfmaxnegvalue", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("cfmaxposvalue", "cfmaxposvalue", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_timestamp_upd_per[]={
            BDMFMON_MAKE_PARM("cftimestampupdper", "cftimestampupdper", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_tp_time[]={
            BDMFMON_MAKE_PARM("cftransporttime", "cftransporttime", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_maxlen_ctr[]={
            BDMFMON_MAKE_PARM("cfrxmaxframelength", "cfrxmaxframelength", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_laser_on_delta[]={
            BDMFMON_MAKE_PARM("cftxlaserondelta", "cftxlaserondelta", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_laser_off_idle[]={
            BDMFMON_MAKE_PARM("cftxinitidle", "cftxinitidle", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("cftxlaseroffdelta", "cftxlaseroffdelta", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_fec_init_idle[]={
            BDMFMON_MAKE_PARM("cftxfecinitidle", "cftxfecinitidle", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_fec_err_allow[]={
            BDMFMON_MAKE_PARM("cfrxtfecbiterrallow", "cfrxtfecbiterrallow", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("cfrxsfecbiterrallow", "cfrxsfecbiterrallow", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_dn_encrypt_stat[]={
            BDMFMON_MAKE_PARM("link_idx", "link_idx", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("enencrypt", "enencrypt", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_sec_up_mpcp_offset[]={
            BDMFMON_MAKE_PARM("secupmpcpoffset", "secupmpcpoffset", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_fec_per_llid[]={
            BDMFMON_MAKE_PARM("link_idx", "link_idx", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("cffectxfecenllid", "cffectxfecenllid", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_rx_line_code_err_cnt[]={
            BDMFMON_MAKE_PARM("rxlinecodeerrcnt", "rxlinecodeerrcnt", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_rx_agg_mpcp_frm[]={
            BDMFMON_MAKE_PARM("rxaggmpcpcnt", "rxaggmpcpcnt", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_rx_agg_good_frm[]={
            BDMFMON_MAKE_PARM("rxagggoodcnt", "rxagggoodcnt", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_rx_agg_good_byte[]={
            BDMFMON_MAKE_PARM("rxagggoodbytescnt", "rxagggoodbytescnt", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_rx_agg_undersz_frm[]={
            BDMFMON_MAKE_PARM("rxaggunderszcnt", "rxaggunderszcnt", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_rx_agg_oversz_frm[]={
            BDMFMON_MAKE_PARM("rxaggoverszcnt", "rxaggoverszcnt", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_rx_agg_crc8_frm[]={
            BDMFMON_MAKE_PARM("rxaggcrc8errcnt", "rxaggcrc8errcnt", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_rx_agg_fec_frm[]={
            BDMFMON_MAKE_PARM("rxaggfec", "rxaggfec", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_rx_agg_fec_byte[]={
            BDMFMON_MAKE_PARM("rxaggfecbytes", "rxaggfecbytes", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_rx_agg_fec_exc_err_frm[]={
            BDMFMON_MAKE_PARM("rxaggfecexceederrs", "rxaggfecexceederrs", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_rx_agg_nonfec_good_frm[]={
            BDMFMON_MAKE_PARM("rxaggnonfecgood", "rxaggnonfecgood", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_rx_agg_nonfec_good_byte[]={
            BDMFMON_MAKE_PARM("rxaggnonfecgoodbytes", "rxaggnonfecgoodbytes", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_rx_agg_err_bytes[]={
            BDMFMON_MAKE_PARM("rxaggerrbytes", "rxaggerrbytes", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_rx_agg_err_zeroes[]={
            BDMFMON_MAKE_PARM("rxaggerrzeroes", "rxaggerrzeroes", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_rx_agg_no_err_blks[]={
            BDMFMON_MAKE_PARM("rxaggnoerrblks", "rxaggnoerrblks", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_rx_agg_cor_blks[]={
            BDMFMON_MAKE_PARM("rxaggcorrblks", "rxaggcorrblks", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_rx_agg_uncor_blks[]={
            BDMFMON_MAKE_PARM("rxagguncorrblks", "rxagguncorrblks", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_rx_agg_err_ones[]={
            BDMFMON_MAKE_PARM("rxaggerrones", "rxaggerrones", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_rx_agg_err_frm[]={
            BDMFMON_MAKE_PARM("rxaggerroredcnt", "rxaggerroredcnt", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_tx_pkt_cnt[]={
            BDMFMON_MAKE_PARM("txframecnt", "txframecnt", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_tx_byte_cnt[]={
            BDMFMON_MAKE_PARM("txbytecnt", "txbytecnt", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_tx_non_fec_pkt_cnt[]={
            BDMFMON_MAKE_PARM("txnonfecframecnt", "txnonfecframecnt", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_tx_non_fec_byte_cnt[]={
            BDMFMON_MAKE_PARM("txnonfecbytecnt", "txnonfecbytecnt", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_tx_fec_pkt_cnt[]={
            BDMFMON_MAKE_PARM("txfecframecnt", "txfecframecnt", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_tx_fec_byte_cnt[]={
            BDMFMON_MAKE_PARM("txfecbytecnt", "txfecbytecnt", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_tx_fec_blk_cnt[]={
            BDMFMON_MAKE_PARM("txfecblkscnt", "txfecblkscnt", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_tx_mpcp_pkt_cnt[]={
            BDMFMON_MAKE_PARM("txmpcpframecnt", "txmpcpframecnt", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_debug_tx_data_pkt_cnt[]={
            BDMFMON_MAKE_PARM("txdataframecnt", "txdataframecnt", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_fec_llid_status[]={
            BDMFMON_MAKE_PARM("stkyfecrevcllidbmsk", "stkyfecrevcllidbmsk", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_sec_rx_tek_ig_iv_llid[]={
            BDMFMON_MAKE_PARM("cfigivnullllid", "cfigivnullllid", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_pon_ber_interv_thresh[]={
            BDMFMON_MAKE_PARM("cfrxlifberinterval", "cfrxlifberinterval", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("cfrxlifberthreshld", "cfrxlifberthreshld", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("cfrxlifbercntrl", "cfrxlifbercntrl", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_lsr_mon_a_ctrl[]={
            BDMFMON_MAKE_PARM("iopbilaserens1a", "iopbilaserens1a", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("cfglsrmonacthi", "cfglsrmonacthi", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("pbilasermonrsta_n_pre", "pbilasermonrsta_n_pre", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_lsr_mon_a_max_thr[]={
            BDMFMON_MAKE_PARM("cfglasermonmaxa", "cfglasermonmaxa", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_ae_pktnum_window[]={
            BDMFMON_MAKE_PARM("cfgaepktnumwnd", "cfgaepktnumwnd", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_ae_pktnum_thresh[]={
            BDMFMON_MAKE_PARM("cfgpktnummaxthresh", "cfgpktnummaxthresh", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_llid_8[]={
            BDMFMON_MAKE_PARM("cfgllid8", "cfgllid8", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_llid_9[]={
            BDMFMON_MAKE_PARM("cfgllid9", "cfgllid9", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_llid_10[]={
            BDMFMON_MAKE_PARM("cfgllid10", "cfgllid10", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_llid_11[]={
            BDMFMON_MAKE_PARM("cfgllid11", "cfgllid11", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_llid_12[]={
            BDMFMON_MAKE_PARM("cfgllid12", "cfgllid12", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_llid_13[]={
            BDMFMON_MAKE_PARM("cfgllid13", "cfgllid13", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_llid_14[]={
            BDMFMON_MAKE_PARM("cfgllid14", "cfgllid14", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_llid_15[]={
            BDMFMON_MAKE_PARM("cfgllid15", "cfgllid15", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_llid_24[]={
            BDMFMON_MAKE_PARM("cfgllid24", "cfgllid24", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_llid_25[]={
            BDMFMON_MAKE_PARM("cfgllid25", "cfgllid25", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_llid_26[]={
            BDMFMON_MAKE_PARM("cfgllid26", "cfgllid26", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_llid_27[]={
            BDMFMON_MAKE_PARM("cfgllid27", "cfgllid27", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_llid_28[]={
            BDMFMON_MAKE_PARM("cfgllid28", "cfgllid28", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_llid_29[]={
            BDMFMON_MAKE_PARM("cfgllid29", "cfgllid29", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_llid_30[]={
            BDMFMON_MAKE_PARM("cfgllid30", "cfgllid30", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_llid_31[]={
            BDMFMON_MAKE_PARM("cfgllid31", "cfgllid31", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_vlan_type[]={
            BDMFMON_MAKE_PARM("cfgvlantype", "cfgvlantype", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_p2p_ae_sci_en[]={
            BDMFMON_MAKE_PARM("cfgp2pscien", "cfgp2pscien", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_p2p_ae_sci_lo_0[]={
            BDMFMON_MAKE_PARM("cfgp2psci_lo_0", "cfgp2psci_lo_0", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_p2p_ae_sci_hi_0[]={
            BDMFMON_MAKE_PARM("cfgp2psci_hi_0", "cfgp2psci_hi_0", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_p2p_ae_sci_lo_1[]={
            BDMFMON_MAKE_PARM("cfgp2psci_lo_1", "cfgp2psci_lo_1", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_p2p_ae_sci_hi_1[]={
            BDMFMON_MAKE_PARM("cfgp2psci_hi_1", "cfgp2psci_hi_1", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_p2p_ae_sci_lo_2[]={
            BDMFMON_MAKE_PARM("cfgp2psci_lo_2", "cfgp2psci_lo_2", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_p2p_ae_sci_hi_2[]={
            BDMFMON_MAKE_PARM("cfgp2psci_hi_2", "cfgp2psci_hi_2", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_p2p_ae_sci_lo_3[]={
            BDMFMON_MAKE_PARM("cfgp2psci_lo_3", "cfgp2psci_lo_3", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_p2p_ae_sci_hi_3[]={
            BDMFMON_MAKE_PARM("cfgp2psci_hi_3", "cfgp2psci_hi_3", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_p2p_ae_sci_lo_4[]={
            BDMFMON_MAKE_PARM("cfgp2psci_lo_4", "cfgp2psci_lo_4", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_p2p_ae_sci_hi_4[]={
            BDMFMON_MAKE_PARM("cfgp2psci_hi_4", "cfgp2psci_hi_4", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_p2p_ae_sci_lo_5[]={
            BDMFMON_MAKE_PARM("cfgp2psci_lo_5", "cfgp2psci_lo_5", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_p2p_ae_sci_hi_5[]={
            BDMFMON_MAKE_PARM("cfgp2psci_hi_5", "cfgp2psci_hi_5", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_p2p_ae_sci_lo_6[]={
            BDMFMON_MAKE_PARM("cfgp2psci_lo_6", "cfgp2psci_lo_6", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_p2p_ae_sci_hi_6[]={
            BDMFMON_MAKE_PARM("cfgp2psci_hi_6", "cfgp2psci_hi_6", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_p2p_ae_sci_lo_7[]={
            BDMFMON_MAKE_PARM("cfgp2psci_lo_7", "cfgp2psci_lo_7", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_p2p_ae_sci_hi_7[]={
            BDMFMON_MAKE_PARM("cfgp2psci_hi_7", "cfgp2psci_hi_7", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_p2p_ae_sci_lo_8[]={
            BDMFMON_MAKE_PARM("cfgp2psci_lo_8", "cfgp2psci_lo_8", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_p2p_ae_sci_hi_8[]={
            BDMFMON_MAKE_PARM("cfgp2psci_hi_8", "cfgp2psci_hi_8", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_p2p_ae_sci_lo_9[]={
            BDMFMON_MAKE_PARM("cfgp2psci_lo_9", "cfgp2psci_lo_9", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_p2p_ae_sci_hi_9[]={
            BDMFMON_MAKE_PARM("cfgp2psci_hi_9", "cfgp2psci_hi_9", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_p2p_ae_sci_lo_10[]={
            BDMFMON_MAKE_PARM("cfgp2psci_lo_10", "cfgp2psci_lo_10", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_p2p_ae_sci_hi_10[]={
            BDMFMON_MAKE_PARM("cfgp2psci_hi_10", "cfgp2psci_hi_10", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_p2p_ae_sci_lo_11[]={
            BDMFMON_MAKE_PARM("cfgp2psci_lo_11", "cfgp2psci_lo_11", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_p2p_ae_sci_hi_11[]={
            BDMFMON_MAKE_PARM("cfgp2psci_hi_11", "cfgp2psci_hi_11", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_p2p_ae_sci_lo_12[]={
            BDMFMON_MAKE_PARM("cfgp2psci_lo_12", "cfgp2psci_lo_12", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_p2p_ae_sci_hi_12[]={
            BDMFMON_MAKE_PARM("cfgp2psci_hi_12", "cfgp2psci_hi_12", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_p2p_ae_sci_lo_13[]={
            BDMFMON_MAKE_PARM("cfgp2psci_lo_13", "cfgp2psci_lo_13", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_p2p_ae_sci_hi_13[]={
            BDMFMON_MAKE_PARM("cfgp2psci_hi_13", "cfgp2psci_hi_13", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_p2p_ae_sci_lo_14[]={
            BDMFMON_MAKE_PARM("cfgp2psci_lo_14", "cfgp2psci_lo_14", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_p2p_ae_sci_hi_14[]={
            BDMFMON_MAKE_PARM("cfgp2psci_hi_14", "cfgp2psci_hi_14", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_p2p_ae_sci_lo_15[]={
            BDMFMON_MAKE_PARM("cfgp2psci_lo_15", "cfgp2psci_lo_15", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_p2p_ae_sci_hi_15[]={
            BDMFMON_MAKE_PARM("cfgp2psci_hi_15", "cfgp2psci_hi_15", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_pon_sec_tx_plaintxt_ae_pad_control[]={
            BDMFMON_MAKE_PARM("cf_plaintxt_oh_b_idle_pad", "cf_plaintxt_oh_b_idle_pad", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("cf_plaintxt_oh_a_idle_pad", "cf_plaintxt_oh_a_idle_pad", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_p2p_autoneg_control[]={
            BDMFMON_MAKE_PARM("cf_autoneg_linktimer", "cf_autoneg_linktimer", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("cf_autoneg_mode_sel", "cf_autoneg_mode_sel", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("cf_autoneg_restart", "cf_autoneg_restart", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("cf_autoneg_en", "cf_autoneg_en", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_p2p_autoneg_ability_config_reg[]={
            BDMFMON_MAKE_PARM("cf_lif_p2p_ae_autoneg_config_ability", "cf_lif_p2p_ae_autoneg_config_ability", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_enum_val_t selector_table[] = {
            { .name="pon_control", .val=BDMF_pon_control, .parms=set_pon_control },
            { .name="pon_inter_op_control", .val=BDMF_pon_inter_op_control, .parms=set_pon_inter_op_control },
            { .name="fec_control", .val=BDMF_fec_control, .parms=set_fec_control },
            { .name="sec_control", .val=BDMF_sec_control, .parms=set_sec_control },
            { .name="macsec", .val=BDMF_macsec, .parms=set_macsec },
            { .name="int_status", .val=BDMF_int_status, .parms=set_int_status },
            { .name="int_mask", .val=BDMF_int_mask, .parms=set_int_mask },
            { .name="data_port_command", .val=BDMF_data_port_command, .parms=set_data_port_command },
            { .name="data_port_data", .val=BDMF_data_port_data, .parms=set_data_port_data },
            { .name="llid_0", .val=BDMF_llid_0, .parms=set_llid_0 },
            { .name="llid_1", .val=BDMF_llid_1, .parms=set_llid_1 },
            { .name="llid_2", .val=BDMF_llid_2, .parms=set_llid_2 },
            { .name="llid_3", .val=BDMF_llid_3, .parms=set_llid_3 },
            { .name="llid_4", .val=BDMF_llid_4, .parms=set_llid_4 },
            { .name="llid_5", .val=BDMF_llid_5, .parms=set_llid_5 },
            { .name="llid_6", .val=BDMF_llid_6, .parms=set_llid_6 },
            { .name="llid_7", .val=BDMF_llid_7, .parms=set_llid_7 },
            { .name="llid_16", .val=BDMF_llid_16, .parms=set_llid_16 },
            { .name="llid_17", .val=BDMF_llid_17, .parms=set_llid_17 },
            { .name="llid_18", .val=BDMF_llid_18, .parms=set_llid_18 },
            { .name="llid_19", .val=BDMF_llid_19, .parms=set_llid_19 },
            { .name="llid_20", .val=BDMF_llid_20, .parms=set_llid_20 },
            { .name="llid_21", .val=BDMF_llid_21, .parms=set_llid_21 },
            { .name="llid_22", .val=BDMF_llid_22, .parms=set_llid_22 },
            { .name="llid_23", .val=BDMF_llid_23, .parms=set_llid_23 },
            { .name="time_ref_cnt", .val=BDMF_time_ref_cnt, .parms=set_time_ref_cnt },
            { .name="timestamp_upd_per", .val=BDMF_timestamp_upd_per, .parms=set_timestamp_upd_per },
            { .name="tp_time", .val=BDMF_tp_time, .parms=set_tp_time },
            { .name="maxlen_ctr", .val=BDMF_maxlen_ctr, .parms=set_maxlen_ctr },
            { .name="laser_on_delta", .val=BDMF_laser_on_delta, .parms=set_laser_on_delta },
            { .name="laser_off_idle", .val=BDMF_laser_off_idle, .parms=set_laser_off_idle },
            { .name="fec_init_idle", .val=BDMF_fec_init_idle, .parms=set_fec_init_idle },
            { .name="fec_err_allow", .val=BDMF_fec_err_allow, .parms=set_fec_err_allow },
            { .name="dn_encrypt_stat", .val=BDMF_dn_encrypt_stat, .parms=set_dn_encrypt_stat },
            { .name="sec_up_mpcp_offset", .val=BDMF_sec_up_mpcp_offset, .parms=set_sec_up_mpcp_offset },
            { .name="fec_per_llid", .val=BDMF_fec_per_llid, .parms=set_fec_per_llid },
            { .name="rx_line_code_err_cnt", .val=BDMF_rx_line_code_err_cnt, .parms=set_rx_line_code_err_cnt },
            { .name="rx_agg_mpcp_frm", .val=BDMF_rx_agg_mpcp_frm, .parms=set_rx_agg_mpcp_frm },
            { .name="rx_agg_good_frm", .val=BDMF_rx_agg_good_frm, .parms=set_rx_agg_good_frm },
            { .name="rx_agg_good_byte", .val=BDMF_rx_agg_good_byte, .parms=set_rx_agg_good_byte },
            { .name="rx_agg_undersz_frm", .val=BDMF_rx_agg_undersz_frm, .parms=set_rx_agg_undersz_frm },
            { .name="rx_agg_oversz_frm", .val=BDMF_rx_agg_oversz_frm, .parms=set_rx_agg_oversz_frm },
            { .name="rx_agg_crc8_frm", .val=BDMF_rx_agg_crc8_frm, .parms=set_rx_agg_crc8_frm },
            { .name="rx_agg_fec_frm", .val=BDMF_rx_agg_fec_frm, .parms=set_rx_agg_fec_frm },
            { .name="rx_agg_fec_byte", .val=BDMF_rx_agg_fec_byte, .parms=set_rx_agg_fec_byte },
            { .name="rx_agg_fec_exc_err_frm", .val=BDMF_rx_agg_fec_exc_err_frm, .parms=set_rx_agg_fec_exc_err_frm },
            { .name="rx_agg_nonfec_good_frm", .val=BDMF_rx_agg_nonfec_good_frm, .parms=set_rx_agg_nonfec_good_frm },
            { .name="rx_agg_nonfec_good_byte", .val=BDMF_rx_agg_nonfec_good_byte, .parms=set_rx_agg_nonfec_good_byte },
            { .name="rx_agg_err_bytes", .val=BDMF_rx_agg_err_bytes, .parms=set_rx_agg_err_bytes },
            { .name="rx_agg_err_zeroes", .val=BDMF_rx_agg_err_zeroes, .parms=set_rx_agg_err_zeroes },
            { .name="rx_agg_no_err_blks", .val=BDMF_rx_agg_no_err_blks, .parms=set_rx_agg_no_err_blks },
            { .name="rx_agg_cor_blks", .val=BDMF_rx_agg_cor_blks, .parms=set_rx_agg_cor_blks },
            { .name="rx_agg_uncor_blks", .val=BDMF_rx_agg_uncor_blks, .parms=set_rx_agg_uncor_blks },
            { .name="rx_agg_err_ones", .val=BDMF_rx_agg_err_ones, .parms=set_rx_agg_err_ones },
            { .name="rx_agg_err_frm", .val=BDMF_rx_agg_err_frm, .parms=set_rx_agg_err_frm },
            { .name="tx_pkt_cnt", .val=BDMF_tx_pkt_cnt, .parms=set_tx_pkt_cnt },
            { .name="tx_byte_cnt", .val=BDMF_tx_byte_cnt, .parms=set_tx_byte_cnt },
            { .name="tx_non_fec_pkt_cnt", .val=BDMF_tx_non_fec_pkt_cnt, .parms=set_tx_non_fec_pkt_cnt },
            { .name="tx_non_fec_byte_cnt", .val=BDMF_tx_non_fec_byte_cnt, .parms=set_tx_non_fec_byte_cnt },
            { .name="tx_fec_pkt_cnt", .val=BDMF_tx_fec_pkt_cnt, .parms=set_tx_fec_pkt_cnt },
            { .name="tx_fec_byte_cnt", .val=BDMF_tx_fec_byte_cnt, .parms=set_tx_fec_byte_cnt },
            { .name="tx_fec_blk_cnt", .val=BDMF_tx_fec_blk_cnt, .parms=set_tx_fec_blk_cnt },
            { .name="tx_mpcp_pkt_cnt", .val=BDMF_tx_mpcp_pkt_cnt, .parms=set_tx_mpcp_pkt_cnt },
            { .name="debug_tx_data_pkt_cnt", .val=BDMF_debug_tx_data_pkt_cnt, .parms=set_debug_tx_data_pkt_cnt },
            { .name="fec_llid_status", .val=BDMF_fec_llid_status, .parms=set_fec_llid_status },
            { .name="sec_rx_tek_ig_iv_llid", .val=BDMF_sec_rx_tek_ig_iv_llid, .parms=set_sec_rx_tek_ig_iv_llid },
            { .name="pon_ber_interv_thresh", .val=BDMF_pon_ber_interv_thresh, .parms=set_pon_ber_interv_thresh },
            { .name="lsr_mon_a_ctrl", .val=BDMF_lsr_mon_a_ctrl, .parms=set_lsr_mon_a_ctrl },
            { .name="lsr_mon_a_max_thr", .val=BDMF_lsr_mon_a_max_thr, .parms=set_lsr_mon_a_max_thr },
            { .name="ae_pktnum_window", .val=BDMF_ae_pktnum_window, .parms=set_ae_pktnum_window },
            { .name="ae_pktnum_thresh", .val=BDMF_ae_pktnum_thresh, .parms=set_ae_pktnum_thresh },
            { .name="llid_8", .val=BDMF_llid_8, .parms=set_llid_8 },
            { .name="llid_9", .val=BDMF_llid_9, .parms=set_llid_9 },
            { .name="llid_10", .val=BDMF_llid_10, .parms=set_llid_10 },
            { .name="llid_11", .val=BDMF_llid_11, .parms=set_llid_11 },
            { .name="llid_12", .val=BDMF_llid_12, .parms=set_llid_12 },
            { .name="llid_13", .val=BDMF_llid_13, .parms=set_llid_13 },
            { .name="llid_14", .val=BDMF_llid_14, .parms=set_llid_14 },
            { .name="llid_15", .val=BDMF_llid_15, .parms=set_llid_15 },
            { .name="llid_24", .val=BDMF_llid_24, .parms=set_llid_24 },
            { .name="llid_25", .val=BDMF_llid_25, .parms=set_llid_25 },
            { .name="llid_26", .val=BDMF_llid_26, .parms=set_llid_26 },
            { .name="llid_27", .val=BDMF_llid_27, .parms=set_llid_27 },
            { .name="llid_28", .val=BDMF_llid_28, .parms=set_llid_28 },
            { .name="llid_29", .val=BDMF_llid_29, .parms=set_llid_29 },
            { .name="llid_30", .val=BDMF_llid_30, .parms=set_llid_30 },
            { .name="llid_31", .val=BDMF_llid_31, .parms=set_llid_31 },
            { .name="vlan_type", .val=BDMF_vlan_type, .parms=set_vlan_type },
            { .name="p2p_ae_sci_en", .val=BDMF_p2p_ae_sci_en, .parms=set_p2p_ae_sci_en },
            { .name="p2p_ae_sci_lo_0", .val=BDMF_p2p_ae_sci_lo_0, .parms=set_p2p_ae_sci_lo_0 },
            { .name="p2p_ae_sci_hi_0", .val=BDMF_p2p_ae_sci_hi_0, .parms=set_p2p_ae_sci_hi_0 },
            { .name="p2p_ae_sci_lo_1", .val=BDMF_p2p_ae_sci_lo_1, .parms=set_p2p_ae_sci_lo_1 },
            { .name="p2p_ae_sci_hi_1", .val=BDMF_p2p_ae_sci_hi_1, .parms=set_p2p_ae_sci_hi_1 },
            { .name="p2p_ae_sci_lo_2", .val=BDMF_p2p_ae_sci_lo_2, .parms=set_p2p_ae_sci_lo_2 },
            { .name="p2p_ae_sci_hi_2", .val=BDMF_p2p_ae_sci_hi_2, .parms=set_p2p_ae_sci_hi_2 },
            { .name="p2p_ae_sci_lo_3", .val=BDMF_p2p_ae_sci_lo_3, .parms=set_p2p_ae_sci_lo_3 },
            { .name="p2p_ae_sci_hi_3", .val=BDMF_p2p_ae_sci_hi_3, .parms=set_p2p_ae_sci_hi_3 },
            { .name="p2p_ae_sci_lo_4", .val=BDMF_p2p_ae_sci_lo_4, .parms=set_p2p_ae_sci_lo_4 },
            { .name="p2p_ae_sci_hi_4", .val=BDMF_p2p_ae_sci_hi_4, .parms=set_p2p_ae_sci_hi_4 },
            { .name="p2p_ae_sci_lo_5", .val=BDMF_p2p_ae_sci_lo_5, .parms=set_p2p_ae_sci_lo_5 },
            { .name="p2p_ae_sci_hi_5", .val=BDMF_p2p_ae_sci_hi_5, .parms=set_p2p_ae_sci_hi_5 },
            { .name="p2p_ae_sci_lo_6", .val=BDMF_p2p_ae_sci_lo_6, .parms=set_p2p_ae_sci_lo_6 },
            { .name="p2p_ae_sci_hi_6", .val=BDMF_p2p_ae_sci_hi_6, .parms=set_p2p_ae_sci_hi_6 },
            { .name="p2p_ae_sci_lo_7", .val=BDMF_p2p_ae_sci_lo_7, .parms=set_p2p_ae_sci_lo_7 },
            { .name="p2p_ae_sci_hi_7", .val=BDMF_p2p_ae_sci_hi_7, .parms=set_p2p_ae_sci_hi_7 },
            { .name="p2p_ae_sci_lo_8", .val=BDMF_p2p_ae_sci_lo_8, .parms=set_p2p_ae_sci_lo_8 },
            { .name="p2p_ae_sci_hi_8", .val=BDMF_p2p_ae_sci_hi_8, .parms=set_p2p_ae_sci_hi_8 },
            { .name="p2p_ae_sci_lo_9", .val=BDMF_p2p_ae_sci_lo_9, .parms=set_p2p_ae_sci_lo_9 },
            { .name="p2p_ae_sci_hi_9", .val=BDMF_p2p_ae_sci_hi_9, .parms=set_p2p_ae_sci_hi_9 },
            { .name="p2p_ae_sci_lo_10", .val=BDMF_p2p_ae_sci_lo_10, .parms=set_p2p_ae_sci_lo_10 },
            { .name="p2p_ae_sci_hi_10", .val=BDMF_p2p_ae_sci_hi_10, .parms=set_p2p_ae_sci_hi_10 },
            { .name="p2p_ae_sci_lo_11", .val=BDMF_p2p_ae_sci_lo_11, .parms=set_p2p_ae_sci_lo_11 },
            { .name="p2p_ae_sci_hi_11", .val=BDMF_p2p_ae_sci_hi_11, .parms=set_p2p_ae_sci_hi_11 },
            { .name="p2p_ae_sci_lo_12", .val=BDMF_p2p_ae_sci_lo_12, .parms=set_p2p_ae_sci_lo_12 },
            { .name="p2p_ae_sci_hi_12", .val=BDMF_p2p_ae_sci_hi_12, .parms=set_p2p_ae_sci_hi_12 },
            { .name="p2p_ae_sci_lo_13", .val=BDMF_p2p_ae_sci_lo_13, .parms=set_p2p_ae_sci_lo_13 },
            { .name="p2p_ae_sci_hi_13", .val=BDMF_p2p_ae_sci_hi_13, .parms=set_p2p_ae_sci_hi_13 },
            { .name="p2p_ae_sci_lo_14", .val=BDMF_p2p_ae_sci_lo_14, .parms=set_p2p_ae_sci_lo_14 },
            { .name="p2p_ae_sci_hi_14", .val=BDMF_p2p_ae_sci_hi_14, .parms=set_p2p_ae_sci_hi_14 },
            { .name="p2p_ae_sci_lo_15", .val=BDMF_p2p_ae_sci_lo_15, .parms=set_p2p_ae_sci_lo_15 },
            { .name="p2p_ae_sci_hi_15", .val=BDMF_p2p_ae_sci_hi_15, .parms=set_p2p_ae_sci_hi_15 },
            { .name="pon_sec_tx_plaintxt_ae_pad_control", .val=BDMF_pon_sec_tx_plaintxt_ae_pad_control, .parms=set_pon_sec_tx_plaintxt_ae_pad_control },
            { .name="p2p_autoneg_control", .val=BDMF_p2p_autoneg_control, .parms=set_p2p_autoneg_control },
            { .name="p2p_autoneg_ability_config_reg", .val=BDMF_p2p_autoneg_ability_config_reg, .parms=set_p2p_autoneg_ability_config_reg },
            BDMFMON_ENUM_LAST
        };
        BDMFMON_MAKE_CMD(dir, "set", "set", bcm_lif_cli_set,
            BDMFMON_MAKE_PARM_SELECTOR("purpose", "purpose", selector_table, 0));
    }
    {
        static bdmfmon_cmd_parm_t set_default[]={
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_data_port_data[]={
            BDMFMON_MAKE_PARM("portidx", "portidx", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_enum_val_t selector_table[] = {
            { .name="pon_control", .val=BDMF_pon_control, .parms=set_default },
            { .name="pon_inter_op_control", .val=BDMF_pon_inter_op_control, .parms=set_default },
            { .name="fec_control", .val=BDMF_fec_control, .parms=set_default },
            { .name="sec_control", .val=BDMF_sec_control, .parms=set_default },
            { .name="macsec", .val=BDMF_macsec, .parms=set_default },
            { .name="int_status", .val=BDMF_int_status, .parms=set_default },
            { .name="int_mask", .val=BDMF_int_mask, .parms=set_default },
            { .name="data_port_command", .val=BDMF_data_port_command, .parms=set_default },
            { .name="data_port_data", .val=BDMF_data_port_data, .parms=set_data_port_data },
            { .name="llid_0", .val=BDMF_llid_0, .parms=set_default },
            { .name="llid_1", .val=BDMF_llid_1, .parms=set_default },
            { .name="llid_2", .val=BDMF_llid_2, .parms=set_default },
            { .name="llid_3", .val=BDMF_llid_3, .parms=set_default },
            { .name="llid_4", .val=BDMF_llid_4, .parms=set_default },
            { .name="llid_5", .val=BDMF_llid_5, .parms=set_default },
            { .name="llid_6", .val=BDMF_llid_6, .parms=set_default },
            { .name="llid_7", .val=BDMF_llid_7, .parms=set_default },
            { .name="llid_16", .val=BDMF_llid_16, .parms=set_default },
            { .name="llid_17", .val=BDMF_llid_17, .parms=set_default },
            { .name="llid_18", .val=BDMF_llid_18, .parms=set_default },
            { .name="llid_19", .val=BDMF_llid_19, .parms=set_default },
            { .name="llid_20", .val=BDMF_llid_20, .parms=set_default },
            { .name="llid_21", .val=BDMF_llid_21, .parms=set_default },
            { .name="llid_22", .val=BDMF_llid_22, .parms=set_default },
            { .name="llid_23", .val=BDMF_llid_23, .parms=set_default },
            { .name="time_ref_cnt", .val=BDMF_time_ref_cnt, .parms=set_default },
            { .name="timestamp_upd_per", .val=BDMF_timestamp_upd_per, .parms=set_default },
            { .name="tp_time", .val=BDMF_tp_time, .parms=set_default },
            { .name="mpcp_time", .val=BDMF_mpcp_time, .parms=set_default },
            { .name="maxlen_ctr", .val=BDMF_maxlen_ctr, .parms=set_default },
            { .name="laser_on_delta", .val=BDMF_laser_on_delta, .parms=set_default },
            { .name="laser_off_idle", .val=BDMF_laser_off_idle, .parms=set_default },
            { .name="fec_init_idle", .val=BDMF_fec_init_idle, .parms=set_default },
            { .name="fec_err_allow", .val=BDMF_fec_err_allow, .parms=set_default },
            { .name="sec_key_sel", .val=BDMF_sec_key_sel, .parms=set_default },
            { .name="dn_encrypt_stat", .val=BDMF_dn_encrypt_stat, .parms=set_default },
            { .name="sec_up_key_stat", .val=BDMF_sec_up_key_stat, .parms=set_default },
            { .name="sec_up_encrypt_stat", .val=BDMF_sec_up_encrypt_stat, .parms=set_default },
            { .name="sec_up_mpcp_offset", .val=BDMF_sec_up_mpcp_offset, .parms=set_default },
            { .name="fec_per_llid", .val=BDMF_fec_per_llid, .parms=set_default },
            { .name="rx_line_code_err_cnt", .val=BDMF_rx_line_code_err_cnt, .parms=set_default },
            { .name="rx_agg_mpcp_frm", .val=BDMF_rx_agg_mpcp_frm, .parms=set_default },
            { .name="rx_agg_good_frm", .val=BDMF_rx_agg_good_frm, .parms=set_default },
            { .name="rx_agg_good_byte", .val=BDMF_rx_agg_good_byte, .parms=set_default },
            { .name="rx_agg_undersz_frm", .val=BDMF_rx_agg_undersz_frm, .parms=set_default },
            { .name="rx_agg_oversz_frm", .val=BDMF_rx_agg_oversz_frm, .parms=set_default },
            { .name="rx_agg_crc8_frm", .val=BDMF_rx_agg_crc8_frm, .parms=set_default },
            { .name="rx_agg_fec_frm", .val=BDMF_rx_agg_fec_frm, .parms=set_default },
            { .name="rx_agg_fec_byte", .val=BDMF_rx_agg_fec_byte, .parms=set_default },
            { .name="rx_agg_fec_exc_err_frm", .val=BDMF_rx_agg_fec_exc_err_frm, .parms=set_default },
            { .name="rx_agg_nonfec_good_frm", .val=BDMF_rx_agg_nonfec_good_frm, .parms=set_default },
            { .name="rx_agg_nonfec_good_byte", .val=BDMF_rx_agg_nonfec_good_byte, .parms=set_default },
            { .name="rx_agg_err_bytes", .val=BDMF_rx_agg_err_bytes, .parms=set_default },
            { .name="rx_agg_err_zeroes", .val=BDMF_rx_agg_err_zeroes, .parms=set_default },
            { .name="rx_agg_no_err_blks", .val=BDMF_rx_agg_no_err_blks, .parms=set_default },
            { .name="rx_agg_cor_blks", .val=BDMF_rx_agg_cor_blks, .parms=set_default },
            { .name="rx_agg_uncor_blks", .val=BDMF_rx_agg_uncor_blks, .parms=set_default },
            { .name="rx_agg_err_ones", .val=BDMF_rx_agg_err_ones, .parms=set_default },
            { .name="rx_agg_err_frm", .val=BDMF_rx_agg_err_frm, .parms=set_default },
            { .name="tx_pkt_cnt", .val=BDMF_tx_pkt_cnt, .parms=set_default },
            { .name="tx_byte_cnt", .val=BDMF_tx_byte_cnt, .parms=set_default },
            { .name="tx_non_fec_pkt_cnt", .val=BDMF_tx_non_fec_pkt_cnt, .parms=set_default },
            { .name="tx_non_fec_byte_cnt", .val=BDMF_tx_non_fec_byte_cnt, .parms=set_default },
            { .name="tx_fec_pkt_cnt", .val=BDMF_tx_fec_pkt_cnt, .parms=set_default },
            { .name="tx_fec_byte_cnt", .val=BDMF_tx_fec_byte_cnt, .parms=set_default },
            { .name="tx_fec_blk_cnt", .val=BDMF_tx_fec_blk_cnt, .parms=set_default },
            { .name="tx_mpcp_pkt_cnt", .val=BDMF_tx_mpcp_pkt_cnt, .parms=set_default },
            { .name="debug_tx_data_pkt_cnt", .val=BDMF_debug_tx_data_pkt_cnt, .parms=set_default },
            { .name="fec_llid_status", .val=BDMF_fec_llid_status, .parms=set_default },
            { .name="sec_rx_tek_ig_iv_llid", .val=BDMF_sec_rx_tek_ig_iv_llid, .parms=set_default },
            { .name="pon_ber_interv_thresh", .val=BDMF_pon_ber_interv_thresh, .parms=set_default },
            { .name="lsr_mon_a_ctrl", .val=BDMF_lsr_mon_a_ctrl, .parms=set_default },
            { .name="lsr_mon_a_max_thr", .val=BDMF_lsr_mon_a_max_thr, .parms=set_default },
            { .name="lsr_mon_a_bst_len", .val=BDMF_lsr_mon_a_bst_len, .parms=set_default },
            { .name="lsr_mon_a_bst_cnt", .val=BDMF_lsr_mon_a_bst_cnt, .parms=set_default },
            { .name="debug_pon_sm", .val=BDMF_debug_pon_sm, .parms=set_default },
            { .name="debug_fec_sm", .val=BDMF_debug_fec_sm, .parms=set_default },
            { .name="ae_pktnum_window", .val=BDMF_ae_pktnum_window, .parms=set_default },
            { .name="ae_pktnum_thresh", .val=BDMF_ae_pktnum_thresh, .parms=set_default },
            { .name="ae_pktnum_stat", .val=BDMF_ae_pktnum_stat, .parms=set_default },
            { .name="llid_8", .val=BDMF_llid_8, .parms=set_default },
            { .name="llid_9", .val=BDMF_llid_9, .parms=set_default },
            { .name="llid_10", .val=BDMF_llid_10, .parms=set_default },
            { .name="llid_11", .val=BDMF_llid_11, .parms=set_default },
            { .name="llid_12", .val=BDMF_llid_12, .parms=set_default },
            { .name="llid_13", .val=BDMF_llid_13, .parms=set_default },
            { .name="llid_14", .val=BDMF_llid_14, .parms=set_default },
            { .name="llid_15", .val=BDMF_llid_15, .parms=set_default },
            { .name="llid_24", .val=BDMF_llid_24, .parms=set_default },
            { .name="llid_25", .val=BDMF_llid_25, .parms=set_default },
            { .name="llid_26", .val=BDMF_llid_26, .parms=set_default },
            { .name="llid_27", .val=BDMF_llid_27, .parms=set_default },
            { .name="llid_28", .val=BDMF_llid_28, .parms=set_default },
            { .name="llid_29", .val=BDMF_llid_29, .parms=set_default },
            { .name="llid_30", .val=BDMF_llid_30, .parms=set_default },
            { .name="llid_31", .val=BDMF_llid_31, .parms=set_default },
            { .name="vlan_type", .val=BDMF_vlan_type, .parms=set_default },
            { .name="p2p_ae_sci_en", .val=BDMF_p2p_ae_sci_en, .parms=set_default },
            { .name="p2p_ae_sci_lo_0", .val=BDMF_p2p_ae_sci_lo_0, .parms=set_default },
            { .name="p2p_ae_sci_hi_0", .val=BDMF_p2p_ae_sci_hi_0, .parms=set_default },
            { .name="p2p_ae_sci_lo_1", .val=BDMF_p2p_ae_sci_lo_1, .parms=set_default },
            { .name="p2p_ae_sci_hi_1", .val=BDMF_p2p_ae_sci_hi_1, .parms=set_default },
            { .name="p2p_ae_sci_lo_2", .val=BDMF_p2p_ae_sci_lo_2, .parms=set_default },
            { .name="p2p_ae_sci_hi_2", .val=BDMF_p2p_ae_sci_hi_2, .parms=set_default },
            { .name="p2p_ae_sci_lo_3", .val=BDMF_p2p_ae_sci_lo_3, .parms=set_default },
            { .name="p2p_ae_sci_hi_3", .val=BDMF_p2p_ae_sci_hi_3, .parms=set_default },
            { .name="p2p_ae_sci_lo_4", .val=BDMF_p2p_ae_sci_lo_4, .parms=set_default },
            { .name="p2p_ae_sci_hi_4", .val=BDMF_p2p_ae_sci_hi_4, .parms=set_default },
            { .name="p2p_ae_sci_lo_5", .val=BDMF_p2p_ae_sci_lo_5, .parms=set_default },
            { .name="p2p_ae_sci_hi_5", .val=BDMF_p2p_ae_sci_hi_5, .parms=set_default },
            { .name="p2p_ae_sci_lo_6", .val=BDMF_p2p_ae_sci_lo_6, .parms=set_default },
            { .name="p2p_ae_sci_hi_6", .val=BDMF_p2p_ae_sci_hi_6, .parms=set_default },
            { .name="p2p_ae_sci_lo_7", .val=BDMF_p2p_ae_sci_lo_7, .parms=set_default },
            { .name="p2p_ae_sci_hi_7", .val=BDMF_p2p_ae_sci_hi_7, .parms=set_default },
            { .name="p2p_ae_sci_lo_8", .val=BDMF_p2p_ae_sci_lo_8, .parms=set_default },
            { .name="p2p_ae_sci_hi_8", .val=BDMF_p2p_ae_sci_hi_8, .parms=set_default },
            { .name="p2p_ae_sci_lo_9", .val=BDMF_p2p_ae_sci_lo_9, .parms=set_default },
            { .name="p2p_ae_sci_hi_9", .val=BDMF_p2p_ae_sci_hi_9, .parms=set_default },
            { .name="p2p_ae_sci_lo_10", .val=BDMF_p2p_ae_sci_lo_10, .parms=set_default },
            { .name="p2p_ae_sci_hi_10", .val=BDMF_p2p_ae_sci_hi_10, .parms=set_default },
            { .name="p2p_ae_sci_lo_11", .val=BDMF_p2p_ae_sci_lo_11, .parms=set_default },
            { .name="p2p_ae_sci_hi_11", .val=BDMF_p2p_ae_sci_hi_11, .parms=set_default },
            { .name="p2p_ae_sci_lo_12", .val=BDMF_p2p_ae_sci_lo_12, .parms=set_default },
            { .name="p2p_ae_sci_hi_12", .val=BDMF_p2p_ae_sci_hi_12, .parms=set_default },
            { .name="p2p_ae_sci_lo_13", .val=BDMF_p2p_ae_sci_lo_13, .parms=set_default },
            { .name="p2p_ae_sci_hi_13", .val=BDMF_p2p_ae_sci_hi_13, .parms=set_default },
            { .name="p2p_ae_sci_lo_14", .val=BDMF_p2p_ae_sci_lo_14, .parms=set_default },
            { .name="p2p_ae_sci_hi_14", .val=BDMF_p2p_ae_sci_hi_14, .parms=set_default },
            { .name="p2p_ae_sci_lo_15", .val=BDMF_p2p_ae_sci_lo_15, .parms=set_default },
            { .name="p2p_ae_sci_hi_15", .val=BDMF_p2p_ae_sci_hi_15, .parms=set_default },
            { .name="sec_up_key_stat_1", .val=BDMF_sec_up_key_stat_1, .parms=set_default },
            { .name="sec_key_sel_1", .val=BDMF_sec_key_sel_1, .parms=set_default },
            { .name="pon_sec_tx_plaintxt_ae_pad_control", .val=BDMF_pon_sec_tx_plaintxt_ae_pad_control, .parms=set_default },
            { .name="p2p_autoneg_control", .val=BDMF_p2p_autoneg_control, .parms=set_default },
            { .name="p2p_autoneg_status", .val=BDMF_p2p_autoneg_status, .parms=set_default },
            { .name="p2p_autoneg_ability_config_reg", .val=BDMF_p2p_autoneg_ability_config_reg, .parms=set_default },
            { .name="p2p_autoneg_link_partner_ability_config_read", .val=BDMF_p2p_autoneg_link_partner_ability_config_read, .parms=set_default },
            BDMFMON_ENUM_LAST
        };
        BDMFMON_MAKE_CMD(dir, "get", "get", bcm_lif_cli_get,
            BDMFMON_MAKE_PARM_SELECTOR("purpose", "purpose", selector_table, 0));
    }
    {
        static bdmfmon_enum_val_t enum_table_test_method[] = {
            { .name="low" , .val=bdmf_test_method_low },
            { .name="mid" , .val=bdmf_test_method_mid },
            { .name="high" , .val=bdmf_test_method_high },
            BDMFMON_ENUM_LAST
        };
        BDMFMON_MAKE_CMD(dir, "test", "test", bcm_lif_cli_test,
            BDMFMON_MAKE_PARM_ENUM("method", "low: 0000, mid: 1000, high: 1111", enum_table_test_method, 0));
    }
    {
        static bdmfmon_enum_val_t enum_table_address[] = {
            { .name="PON_CONTROL" , .val=bdmf_address_pon_control },
            { .name="PON_INTER_OP_CONTROL" , .val=bdmf_address_pon_inter_op_control },
            { .name="FEC_CONTROL" , .val=bdmf_address_fec_control },
            { .name="SEC_CONTROL" , .val=bdmf_address_sec_control },
            { .name="MACSEC" , .val=bdmf_address_macsec },
            { .name="INT_STATUS" , .val=bdmf_address_int_status },
            { .name="INT_MASK" , .val=bdmf_address_int_mask },
            { .name="DATA_PORT_COMMAND" , .val=bdmf_address_data_port_command },
            { .name="DATA_PORT_DATA" , .val=bdmf_address_data_port_data },
            { .name="LLID_0" , .val=bdmf_address_llid_0 },
            { .name="LLID_1" , .val=bdmf_address_llid_1 },
            { .name="LLID_2" , .val=bdmf_address_llid_2 },
            { .name="LLID_3" , .val=bdmf_address_llid_3 },
            { .name="LLID_4" , .val=bdmf_address_llid_4 },
            { .name="LLID_5" , .val=bdmf_address_llid_5 },
            { .name="LLID_6" , .val=bdmf_address_llid_6 },
            { .name="LLID_7" , .val=bdmf_address_llid_7 },
            { .name="LLID_16" , .val=bdmf_address_llid_16 },
            { .name="LLID_17" , .val=bdmf_address_llid_17 },
            { .name="LLID_18" , .val=bdmf_address_llid_18 },
            { .name="LLID_19" , .val=bdmf_address_llid_19 },
            { .name="LLID_20" , .val=bdmf_address_llid_20 },
            { .name="LLID_21" , .val=bdmf_address_llid_21 },
            { .name="LLID_22" , .val=bdmf_address_llid_22 },
            { .name="LLID_23" , .val=bdmf_address_llid_23 },
            { .name="TIME_REF_CNT" , .val=bdmf_address_time_ref_cnt },
            { .name="TIMESTAMP_UPD_PER" , .val=bdmf_address_timestamp_upd_per },
            { .name="TP_TIME" , .val=bdmf_address_tp_time },
            { .name="MPCP_TIME" , .val=bdmf_address_mpcp_time },
            { .name="MAXLEN_CTR" , .val=bdmf_address_maxlen_ctr },
            { .name="LASER_ON_DELTA" , .val=bdmf_address_laser_on_delta },
            { .name="LASER_OFF_IDLE" , .val=bdmf_address_laser_off_idle },
            { .name="FEC_INIT_IDLE" , .val=bdmf_address_fec_init_idle },
            { .name="FEC_ERR_ALLOW" , .val=bdmf_address_fec_err_allow },
            { .name="SEC_KEY_SEL" , .val=bdmf_address_sec_key_sel },
            { .name="DN_ENCRYPT_STAT" , .val=bdmf_address_dn_encrypt_stat },
            { .name="SEC_UP_KEY_STAT" , .val=bdmf_address_sec_up_key_stat },
            { .name="SEC_UP_ENCRYPT_STAT" , .val=bdmf_address_sec_up_encrypt_stat },
            { .name="SEC_UP_MPCP_OFFSET" , .val=bdmf_address_sec_up_mpcp_offset },
            { .name="FEC_PER_LLID" , .val=bdmf_address_fec_per_llid },
            { .name="RX_LINE_CODE_ERR_CNT" , .val=bdmf_address_rx_line_code_err_cnt },
            { .name="RX_AGG_MPCP_FRM" , .val=bdmf_address_rx_agg_mpcp_frm },
            { .name="RX_AGG_GOOD_FRM" , .val=bdmf_address_rx_agg_good_frm },
            { .name="RX_AGG_GOOD_BYTE" , .val=bdmf_address_rx_agg_good_byte },
            { .name="RX_AGG_UNDERSZ_FRM" , .val=bdmf_address_rx_agg_undersz_frm },
            { .name="RX_AGG_OVERSZ_FRM" , .val=bdmf_address_rx_agg_oversz_frm },
            { .name="RX_AGG_CRC8_FRM" , .val=bdmf_address_rx_agg_crc8_frm },
            { .name="RX_AGG_FEC_FRM" , .val=bdmf_address_rx_agg_fec_frm },
            { .name="RX_AGG_FEC_BYTE" , .val=bdmf_address_rx_agg_fec_byte },
            { .name="RX_AGG_FEC_EXC_ERR_FRM" , .val=bdmf_address_rx_agg_fec_exc_err_frm },
            { .name="RX_AGG_NONFEC_GOOD_FRM" , .val=bdmf_address_rx_agg_nonfec_good_frm },
            { .name="RX_AGG_NONFEC_GOOD_BYTE" , .val=bdmf_address_rx_agg_nonfec_good_byte },
            { .name="RX_AGG_ERR_BYTES" , .val=bdmf_address_rx_agg_err_bytes },
            { .name="RX_AGG_ERR_ZEROES" , .val=bdmf_address_rx_agg_err_zeroes },
            { .name="RX_AGG_NO_ERR_BLKS" , .val=bdmf_address_rx_agg_no_err_blks },
            { .name="RX_AGG_COR_BLKS" , .val=bdmf_address_rx_agg_cor_blks },
            { .name="RX_AGG_UNCOR_BLKS" , .val=bdmf_address_rx_agg_uncor_blks },
            { .name="RX_AGG_ERR_ONES" , .val=bdmf_address_rx_agg_err_ones },
            { .name="RX_AGG_ERR_FRM" , .val=bdmf_address_rx_agg_err_frm },
            { .name="TX_PKT_CNT" , .val=bdmf_address_tx_pkt_cnt },
            { .name="TX_BYTE_CNT" , .val=bdmf_address_tx_byte_cnt },
            { .name="TX_NON_FEC_PKT_CNT" , .val=bdmf_address_tx_non_fec_pkt_cnt },
            { .name="TX_NON_FEC_BYTE_CNT" , .val=bdmf_address_tx_non_fec_byte_cnt },
            { .name="TX_FEC_PKT_CNT" , .val=bdmf_address_tx_fec_pkt_cnt },
            { .name="TX_FEC_BYTE_CNT" , .val=bdmf_address_tx_fec_byte_cnt },
            { .name="TX_FEC_BLK_CNT" , .val=bdmf_address_tx_fec_blk_cnt },
            { .name="TX_MPCP_PKT_CNT" , .val=bdmf_address_tx_mpcp_pkt_cnt },
            { .name="DEBUG_TX_DATA_PKT_CNT" , .val=bdmf_address_debug_tx_data_pkt_cnt },
            { .name="FEC_LLID_STATUS" , .val=bdmf_address_fec_llid_status },
            { .name="SEC_RX_TEK_IG_IV_LLID" , .val=bdmf_address_sec_rx_tek_ig_iv_llid },
            { .name="PON_BER_INTERV_THRESH" , .val=bdmf_address_pon_ber_interv_thresh },
            { .name="LSR_MON_A_CTRL" , .val=bdmf_address_lsr_mon_a_ctrl },
            { .name="LSR_MON_A_MAX_THR" , .val=bdmf_address_lsr_mon_a_max_thr },
            { .name="LSR_MON_A_BST_LEN" , .val=bdmf_address_lsr_mon_a_bst_len },
            { .name="LSR_MON_A_BST_CNT" , .val=bdmf_address_lsr_mon_a_bst_cnt },
            { .name="DEBUG_PON_SM" , .val=bdmf_address_debug_pon_sm },
            { .name="DEBUG_FEC_SM" , .val=bdmf_address_debug_fec_sm },
            { .name="AE_PKTNUM_WINDOW" , .val=bdmf_address_ae_pktnum_window },
            { .name="AE_PKTNUM_THRESH" , .val=bdmf_address_ae_pktnum_thresh },
            { .name="AE_PKTNUM_STAT" , .val=bdmf_address_ae_pktnum_stat },
            { .name="LLID_8" , .val=bdmf_address_llid_8 },
            { .name="LLID_9" , .val=bdmf_address_llid_9 },
            { .name="LLID_10" , .val=bdmf_address_llid_10 },
            { .name="LLID_11" , .val=bdmf_address_llid_11 },
            { .name="LLID_12" , .val=bdmf_address_llid_12 },
            { .name="LLID_13" , .val=bdmf_address_llid_13 },
            { .name="LLID_14" , .val=bdmf_address_llid_14 },
            { .name="LLID_15" , .val=bdmf_address_llid_15 },
            { .name="LLID_24" , .val=bdmf_address_llid_24 },
            { .name="LLID_25" , .val=bdmf_address_llid_25 },
            { .name="LLID_26" , .val=bdmf_address_llid_26 },
            { .name="LLID_27" , .val=bdmf_address_llid_27 },
            { .name="LLID_28" , .val=bdmf_address_llid_28 },
            { .name="LLID_29" , .val=bdmf_address_llid_29 },
            { .name="LLID_30" , .val=bdmf_address_llid_30 },
            { .name="LLID_31" , .val=bdmf_address_llid_31 },
            { .name="VLAN_TYPE" , .val=bdmf_address_vlan_type },
            { .name="P2P_AE_SCI_EN" , .val=bdmf_address_p2p_ae_sci_en },
            { .name="P2P_AE_SCI_LO_0" , .val=bdmf_address_p2p_ae_sci_lo_0 },
            { .name="P2P_AE_SCI_HI_0" , .val=bdmf_address_p2p_ae_sci_hi_0 },
            { .name="P2P_AE_SCI_LO_1" , .val=bdmf_address_p2p_ae_sci_lo_1 },
            { .name="P2P_AE_SCI_HI_1" , .val=bdmf_address_p2p_ae_sci_hi_1 },
            { .name="P2P_AE_SCI_LO_2" , .val=bdmf_address_p2p_ae_sci_lo_2 },
            { .name="P2P_AE_SCI_HI_2" , .val=bdmf_address_p2p_ae_sci_hi_2 },
            { .name="P2P_AE_SCI_LO_3" , .val=bdmf_address_p2p_ae_sci_lo_3 },
            { .name="P2P_AE_SCI_HI_3" , .val=bdmf_address_p2p_ae_sci_hi_3 },
            { .name="P2P_AE_SCI_LO_4" , .val=bdmf_address_p2p_ae_sci_lo_4 },
            { .name="P2P_AE_SCI_HI_4" , .val=bdmf_address_p2p_ae_sci_hi_4 },
            { .name="P2P_AE_SCI_LO_5" , .val=bdmf_address_p2p_ae_sci_lo_5 },
            { .name="P2P_AE_SCI_HI_5" , .val=bdmf_address_p2p_ae_sci_hi_5 },
            { .name="P2P_AE_SCI_LO_6" , .val=bdmf_address_p2p_ae_sci_lo_6 },
            { .name="P2P_AE_SCI_HI_6" , .val=bdmf_address_p2p_ae_sci_hi_6 },
            { .name="P2P_AE_SCI_LO_7" , .val=bdmf_address_p2p_ae_sci_lo_7 },
            { .name="P2P_AE_SCI_HI_7" , .val=bdmf_address_p2p_ae_sci_hi_7 },
            { .name="P2P_AE_SCI_LO_8" , .val=bdmf_address_p2p_ae_sci_lo_8 },
            { .name="P2P_AE_SCI_HI_8" , .val=bdmf_address_p2p_ae_sci_hi_8 },
            { .name="P2P_AE_SCI_LO_9" , .val=bdmf_address_p2p_ae_sci_lo_9 },
            { .name="P2P_AE_SCI_HI_9" , .val=bdmf_address_p2p_ae_sci_hi_9 },
            { .name="P2P_AE_SCI_LO_10" , .val=bdmf_address_p2p_ae_sci_lo_10 },
            { .name="P2P_AE_SCI_HI_10" , .val=bdmf_address_p2p_ae_sci_hi_10 },
            { .name="P2P_AE_SCI_LO_11" , .val=bdmf_address_p2p_ae_sci_lo_11 },
            { .name="P2P_AE_SCI_HI_11" , .val=bdmf_address_p2p_ae_sci_hi_11 },
            { .name="P2P_AE_SCI_LO_12" , .val=bdmf_address_p2p_ae_sci_lo_12 },
            { .name="P2P_AE_SCI_HI_12" , .val=bdmf_address_p2p_ae_sci_hi_12 },
            { .name="P2P_AE_SCI_LO_13" , .val=bdmf_address_p2p_ae_sci_lo_13 },
            { .name="P2P_AE_SCI_HI_13" , .val=bdmf_address_p2p_ae_sci_hi_13 },
            { .name="P2P_AE_SCI_LO_14" , .val=bdmf_address_p2p_ae_sci_lo_14 },
            { .name="P2P_AE_SCI_HI_14" , .val=bdmf_address_p2p_ae_sci_hi_14 },
            { .name="P2P_AE_SCI_LO_15" , .val=bdmf_address_p2p_ae_sci_lo_15 },
            { .name="P2P_AE_SCI_HI_15" , .val=bdmf_address_p2p_ae_sci_hi_15 },
            { .name="SEC_UP_KEY_STAT_1" , .val=bdmf_address_sec_up_key_stat_1 },
            { .name="SEC_KEY_SEL_1" , .val=bdmf_address_sec_key_sel_1 },
            { .name="PON_SEC_TX_PLAINTXT_AE_PAD_CONTROL" , .val=bdmf_address_pon_sec_tx_plaintxt_ae_pad_control },
            { .name="P2P_AUTONEG_CONTROL" , .val=bdmf_address_p2p_autoneg_control },
            { .name="P2P_AUTONEG_STATUS" , .val=bdmf_address_p2p_autoneg_status },
            { .name="P2P_AUTONEG_ABILITY_CONFIG_REG" , .val=bdmf_address_p2p_autoneg_ability_config_reg },
            { .name="P2P_AUTONEG_LINK_PARTNER_ABILITY_CONFIG_READ" , .val=bdmf_address_p2p_autoneg_link_partner_ability_config_read },
            BDMFMON_ENUM_LAST
        };
        BDMFMON_MAKE_CMD(dir, "address", "address", bcm_lif_cli_address,
            BDMFMON_MAKE_PARM_ENUM("method", "method", enum_table_address, 0),
            BDMFMON_MAKE_PARM("index2", "onu_id/alloc_id/port_id/etc...", BDMFMON_PARM_NUMBER, BDMFMON_PARM_FLAG_OPTIONAL));
    }
    return dir;
}
#endif /* USE_BDMF_SHELL */

