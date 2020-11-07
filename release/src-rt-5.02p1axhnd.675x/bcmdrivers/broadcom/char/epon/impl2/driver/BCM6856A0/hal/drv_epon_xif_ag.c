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
#include "drv_epon_xif_ag.h"
bdmf_error_t ag_drv_xif_ctl_set(const xif_ctl *ctl)
{
    uint32_t reg_ctl=0;

#ifdef VALIDATE_PARMS
    if(!ctl)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((ctl->rxencrypten >= _1BITS_MAX_VAL_) ||
       (ctl->cfgdisrxdasaencrpt >= _1BITS_MAX_VAL_) ||
       (ctl->rxencryptmode >= _2BITS_MAX_VAL_) ||
       (ctl->txencrypten >= _1BITS_MAX_VAL_) ||
       (ctl->cfgdistxdasaencrpt >= _1BITS_MAX_VAL_) ||
       (ctl->txencryptmode >= _2BITS_MAX_VAL_) ||
       (ctl->cfgllidmodemsk >= _1BITS_MAX_VAL_) ||
       (ctl->cfgxpnbadcrc32 >= _1BITS_MAX_VAL_) ||
       (ctl->cfgdisdiscinfo >= _1BITS_MAX_VAL_) ||
       (ctl->cfgpmctx2rxlpbk >= _1BITS_MAX_VAL_) ||
       (ctl->cfgpmctxencrc8bad >= _1BITS_MAX_VAL_) ||
       (ctl->cfgenp2p >= _1BITS_MAX_VAL_) ||
       (ctl->cfgllidpromiscuousmode >= _1BITS_MAX_VAL_) ||
       (ctl->cfgenidlepktsup >= _1BITS_MAX_VAL_) ||
       (ctl->cfgpmcrxencrc8chk >= _1BITS_MAX_VAL_) ||
       (ctl->cfgen1stidlepktconvert >= _1BITS_MAX_VAL_) ||
       (ctl->cfgfecen >= _1BITS_MAX_VAL_) ||
       (ctl->cfglegacyrcvtsupd >= _1BITS_MAX_VAL_) ||
       (ctl->cfgxpnencrcpassthru >= _1BITS_MAX_VAL_) ||
       (ctl->cfgxpndistimestampmod >= _1BITS_MAX_VAL_) ||
       (ctl->xifnotrdy >= _1BITS_MAX_VAL_) ||
       (ctl->xifdtportrstn >= _1BITS_MAX_VAL_) ||
       (ctl->xpntxrstn >= _1BITS_MAX_VAL_) ||
       (ctl->pmctxrstn >= _1BITS_MAX_VAL_) ||
       (ctl->sectxrstn >= _1BITS_MAX_VAL_) ||
       (ctl->cfgdistxoamencrpt >= _1BITS_MAX_VAL_) ||
       (ctl->cfgdistxmpcpencrpt >= _1BITS_MAX_VAL_) ||
       (ctl->pmcrxrstn >= _1BITS_MAX_VAL_) ||
       (ctl->secrxrstn >= _1BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_ctl = RU_FIELD_SET(0, XIF, CTL, RXENCRYPTEN, reg_ctl, ctl->rxencrypten);
    reg_ctl = RU_FIELD_SET(0, XIF, CTL, CFGDISRXDASAENCRPT, reg_ctl, ctl->cfgdisrxdasaencrpt);
    reg_ctl = RU_FIELD_SET(0, XIF, CTL, RXENCRYPTMODE, reg_ctl, ctl->rxencryptmode);
    reg_ctl = RU_FIELD_SET(0, XIF, CTL, TXENCRYPTEN, reg_ctl, ctl->txencrypten);
    reg_ctl = RU_FIELD_SET(0, XIF, CTL, CFGDISTXDASAENCRPT, reg_ctl, ctl->cfgdistxdasaencrpt);
    reg_ctl = RU_FIELD_SET(0, XIF, CTL, TXENCRYPTMODE, reg_ctl, ctl->txencryptmode);
    reg_ctl = RU_FIELD_SET(0, XIF, CTL, CFGLLIDMODEMSK, reg_ctl, ctl->cfgllidmodemsk);
    reg_ctl = RU_FIELD_SET(0, XIF, CTL, CFGXPNBADCRC32, reg_ctl, ctl->cfgxpnbadcrc32);
    reg_ctl = RU_FIELD_SET(0, XIF, CTL, CFGDISDISCINFO, reg_ctl, ctl->cfgdisdiscinfo);
    reg_ctl = RU_FIELD_SET(0, XIF, CTL, CFGPMCTX2RXLPBK, reg_ctl, ctl->cfgpmctx2rxlpbk);
    reg_ctl = RU_FIELD_SET(0, XIF, CTL, CFGPMCTXENCRC8BAD, reg_ctl, ctl->cfgpmctxencrc8bad);
    reg_ctl = RU_FIELD_SET(0, XIF, CTL, CFGENP2P, reg_ctl, ctl->cfgenp2p);
    reg_ctl = RU_FIELD_SET(0, XIF, CTL, CFGLLIDPROMISCUOUSMODE, reg_ctl, ctl->cfgllidpromiscuousmode);
    reg_ctl = RU_FIELD_SET(0, XIF, CTL, CFGENIDLEPKTSUP, reg_ctl, ctl->cfgenidlepktsup);
    reg_ctl = RU_FIELD_SET(0, XIF, CTL, CFGPMCRXENCRC8CHK, reg_ctl, ctl->cfgpmcrxencrc8chk);
    reg_ctl = RU_FIELD_SET(0, XIF, CTL, CFGEN1STIDLEPKTCONVERT, reg_ctl, ctl->cfgen1stidlepktconvert);
    reg_ctl = RU_FIELD_SET(0, XIF, CTL, CFGFECEN, reg_ctl, ctl->cfgfecen);
    reg_ctl = RU_FIELD_SET(0, XIF, CTL, CFGLEGACYRCVTSUPD, reg_ctl, ctl->cfglegacyrcvtsupd);
    reg_ctl = RU_FIELD_SET(0, XIF, CTL, CFGXPNENCRCPASSTHRU, reg_ctl, ctl->cfgxpnencrcpassthru);
    reg_ctl = RU_FIELD_SET(0, XIF, CTL, CFGXPNDISTIMESTAMPMOD, reg_ctl, ctl->cfgxpndistimestampmod);
    reg_ctl = RU_FIELD_SET(0, XIF, CTL, XIFNOTRDY, reg_ctl, ctl->xifnotrdy);
    reg_ctl = RU_FIELD_SET(0, XIF, CTL, XIFDTPORTRSTN, reg_ctl, ctl->xifdtportrstn);
    reg_ctl = RU_FIELD_SET(0, XIF, CTL, XPNTXRSTN, reg_ctl, ctl->xpntxrstn);
    reg_ctl = RU_FIELD_SET(0, XIF, CTL, PMCTXRSTN, reg_ctl, ctl->pmctxrstn);
    reg_ctl = RU_FIELD_SET(0, XIF, CTL, SECTXRSTN, reg_ctl, ctl->sectxrstn);
    reg_ctl = RU_FIELD_SET(0, XIF, CTL, CFGDISTXOAMENCRPT, reg_ctl, ctl->cfgdistxoamencrpt);
    reg_ctl = RU_FIELD_SET(0, XIF, CTL, CFGDISTXMPCPENCRPT, reg_ctl, ctl->cfgdistxmpcpencrpt);
    reg_ctl = RU_FIELD_SET(0, XIF, CTL, PMCRXRSTN, reg_ctl, ctl->pmcrxrstn);
    reg_ctl = RU_FIELD_SET(0, XIF, CTL, SECRXRSTN, reg_ctl, ctl->secrxrstn);

    RU_REG_WRITE(0, XIF, CTL, reg_ctl);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xif_ctl_get(xif_ctl *ctl)
{
    uint32_t reg_ctl=0;

#ifdef VALIDATE_PARMS
    if(!ctl)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, XIF, CTL, reg_ctl);

    ctl->rxencrypten = RU_FIELD_GET(0, XIF, CTL, RXENCRYPTEN, reg_ctl);
    ctl->cfgdisrxdasaencrpt = RU_FIELD_GET(0, XIF, CTL, CFGDISRXDASAENCRPT, reg_ctl);
    ctl->rxencryptmode = RU_FIELD_GET(0, XIF, CTL, RXENCRYPTMODE, reg_ctl);
    ctl->txencrypten = RU_FIELD_GET(0, XIF, CTL, TXENCRYPTEN, reg_ctl);
    ctl->cfgdistxdasaencrpt = RU_FIELD_GET(0, XIF, CTL, CFGDISTXDASAENCRPT, reg_ctl);
    ctl->txencryptmode = RU_FIELD_GET(0, XIF, CTL, TXENCRYPTMODE, reg_ctl);
    ctl->cfgllidmodemsk = RU_FIELD_GET(0, XIF, CTL, CFGLLIDMODEMSK, reg_ctl);
    ctl->cfgxpnbadcrc32 = RU_FIELD_GET(0, XIF, CTL, CFGXPNBADCRC32, reg_ctl);
    ctl->cfgdisdiscinfo = RU_FIELD_GET(0, XIF, CTL, CFGDISDISCINFO, reg_ctl);
    ctl->cfgpmctx2rxlpbk = RU_FIELD_GET(0, XIF, CTL, CFGPMCTX2RXLPBK, reg_ctl);
    ctl->cfgpmctxencrc8bad = RU_FIELD_GET(0, XIF, CTL, CFGPMCTXENCRC8BAD, reg_ctl);
    ctl->cfgenp2p = RU_FIELD_GET(0, XIF, CTL, CFGENP2P, reg_ctl);
    ctl->cfgllidpromiscuousmode = RU_FIELD_GET(0, XIF, CTL, CFGLLIDPROMISCUOUSMODE, reg_ctl);
    ctl->cfgenidlepktsup = RU_FIELD_GET(0, XIF, CTL, CFGENIDLEPKTSUP, reg_ctl);
    ctl->cfgpmcrxencrc8chk = RU_FIELD_GET(0, XIF, CTL, CFGPMCRXENCRC8CHK, reg_ctl);
    ctl->cfgen1stidlepktconvert = RU_FIELD_GET(0, XIF, CTL, CFGEN1STIDLEPKTCONVERT, reg_ctl);
    ctl->cfgfecen = RU_FIELD_GET(0, XIF, CTL, CFGFECEN, reg_ctl);
    ctl->cfglegacyrcvtsupd = RU_FIELD_GET(0, XIF, CTL, CFGLEGACYRCVTSUPD, reg_ctl);
    ctl->cfgxpnencrcpassthru = RU_FIELD_GET(0, XIF, CTL, CFGXPNENCRCPASSTHRU, reg_ctl);
    ctl->cfgxpndistimestampmod = RU_FIELD_GET(0, XIF, CTL, CFGXPNDISTIMESTAMPMOD, reg_ctl);
    ctl->xifnotrdy = RU_FIELD_GET(0, XIF, CTL, XIFNOTRDY, reg_ctl);
    ctl->xifdtportrstn = RU_FIELD_GET(0, XIF, CTL, XIFDTPORTRSTN, reg_ctl);
    ctl->xpntxrstn = RU_FIELD_GET(0, XIF, CTL, XPNTXRSTN, reg_ctl);
    ctl->pmctxrstn = RU_FIELD_GET(0, XIF, CTL, PMCTXRSTN, reg_ctl);
    ctl->sectxrstn = RU_FIELD_GET(0, XIF, CTL, SECTXRSTN, reg_ctl);
    ctl->cfgdistxoamencrpt = RU_FIELD_GET(0, XIF, CTL, CFGDISTXOAMENCRPT, reg_ctl);
    ctl->cfgdistxmpcpencrpt = RU_FIELD_GET(0, XIF, CTL, CFGDISTXMPCPENCRPT, reg_ctl);
    ctl->pmcrxrstn = RU_FIELD_GET(0, XIF, CTL, PMCRXRSTN, reg_ctl);
    ctl->secrxrstn = RU_FIELD_GET(0, XIF, CTL, SECRXRSTN, reg_ctl);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xif_int_status_set(const xif_int_status *int_status)
{
    uint32_t reg_int_status=0;

#ifdef VALIDATE_PARMS
    if(!int_status)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((int_status->secrxrplyprtctabrtint >= _1BITS_MAX_VAL_) ||
       (int_status->sectxpktnummaxint >= _1BITS_MAX_VAL_) ||
       (int_status->tsfullupdint >= _1BITS_MAX_VAL_) ||
       (int_status->txhangint >= _1BITS_MAX_VAL_) ||
       (int_status->negtimeint >= _1BITS_MAX_VAL_) ||
       (int_status->pmctsjttrint >= _1BITS_MAX_VAL_) ||
       (int_status->secrxoutffovrflwint >= _1BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_int_status = RU_FIELD_SET(0, XIF, INT_STATUS, SECRXRPLYPRTCTABRTINT, reg_int_status, int_status->secrxrplyprtctabrtint);
    reg_int_status = RU_FIELD_SET(0, XIF, INT_STATUS, SECTXPKTNUMMAXINT, reg_int_status, int_status->sectxpktnummaxint);
    reg_int_status = RU_FIELD_SET(0, XIF, INT_STATUS, TSFULLUPDINT, reg_int_status, int_status->tsfullupdint);
    reg_int_status = RU_FIELD_SET(0, XIF, INT_STATUS, TXHANGINT, reg_int_status, int_status->txhangint);
    reg_int_status = RU_FIELD_SET(0, XIF, INT_STATUS, NEGTIMEINT, reg_int_status, int_status->negtimeint);
    reg_int_status = RU_FIELD_SET(0, XIF, INT_STATUS, PMCTSJTTRINT, reg_int_status, int_status->pmctsjttrint);
    reg_int_status = RU_FIELD_SET(0, XIF, INT_STATUS, SECRXOUTFFOVRFLWINT, reg_int_status, int_status->secrxoutffovrflwint);

    RU_REG_WRITE(0, XIF, INT_STATUS, reg_int_status);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xif_int_status_get(xif_int_status *int_status)
{
    uint32_t reg_int_status=0;

#ifdef VALIDATE_PARMS
    if(!int_status)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, XIF, INT_STATUS, reg_int_status);

    int_status->secrxrplyprtctabrtint = RU_FIELD_GET(0, XIF, INT_STATUS, SECRXRPLYPRTCTABRTINT, reg_int_status);
    int_status->sectxpktnummaxint = RU_FIELD_GET(0, XIF, INT_STATUS, SECTXPKTNUMMAXINT, reg_int_status);
    int_status->tsfullupdint = RU_FIELD_GET(0, XIF, INT_STATUS, TSFULLUPDINT, reg_int_status);
    int_status->txhangint = RU_FIELD_GET(0, XIF, INT_STATUS, TXHANGINT, reg_int_status);
    int_status->negtimeint = RU_FIELD_GET(0, XIF, INT_STATUS, NEGTIMEINT, reg_int_status);
    int_status->pmctsjttrint = RU_FIELD_GET(0, XIF, INT_STATUS, PMCTSJTTRINT, reg_int_status);
    int_status->secrxoutffovrflwint = RU_FIELD_GET(0, XIF, INT_STATUS, SECRXOUTFFOVRFLWINT, reg_int_status);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xif_int_mask_set(const xif_int_mask *int_mask)
{
    uint32_t reg_int_mask=0;

#ifdef VALIDATE_PARMS
    if(!int_mask)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((int_mask->msksecrxreplayprotctabort >= _1BITS_MAX_VAL_) ||
       (int_mask->mskpktnumthreshint >= _1BITS_MAX_VAL_) ||
       (int_mask->msktsfullupdint >= _1BITS_MAX_VAL_) ||
       (int_mask->msktxhangint >= _1BITS_MAX_VAL_) ||
       (int_mask->msknegtimeint >= _1BITS_MAX_VAL_) ||
       (int_mask->mskpmctsjttrint >= _1BITS_MAX_VAL_) ||
       (int_mask->msksecrxoutffint >= _1BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_int_mask = RU_FIELD_SET(0, XIF, INT_MASK, MSKSECRXREPLAYPROTCTABORT, reg_int_mask, int_mask->msksecrxreplayprotctabort);
    reg_int_mask = RU_FIELD_SET(0, XIF, INT_MASK, MSKPKTNUMTHRESHINT, reg_int_mask, int_mask->mskpktnumthreshint);
    reg_int_mask = RU_FIELD_SET(0, XIF, INT_MASK, MSKTSFULLUPDINT, reg_int_mask, int_mask->msktsfullupdint);
    reg_int_mask = RU_FIELD_SET(0, XIF, INT_MASK, MSKTXHANGINT, reg_int_mask, int_mask->msktxhangint);
    reg_int_mask = RU_FIELD_SET(0, XIF, INT_MASK, MSKNEGTIMEINT, reg_int_mask, int_mask->msknegtimeint);
    reg_int_mask = RU_FIELD_SET(0, XIF, INT_MASK, MSKPMCTSJTTRINT, reg_int_mask, int_mask->mskpmctsjttrint);
    reg_int_mask = RU_FIELD_SET(0, XIF, INT_MASK, MSKSECRXOUTFFINT, reg_int_mask, int_mask->msksecrxoutffint);

    RU_REG_WRITE(0, XIF, INT_MASK, reg_int_mask);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xif_int_mask_get(xif_int_mask *int_mask)
{
    uint32_t reg_int_mask=0;

#ifdef VALIDATE_PARMS
    if(!int_mask)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, XIF, INT_MASK, reg_int_mask);

    int_mask->msksecrxreplayprotctabort = RU_FIELD_GET(0, XIF, INT_MASK, MSKSECRXREPLAYPROTCTABORT, reg_int_mask);
    int_mask->mskpktnumthreshint = RU_FIELD_GET(0, XIF, INT_MASK, MSKPKTNUMTHRESHINT, reg_int_mask);
    int_mask->msktsfullupdint = RU_FIELD_GET(0, XIF, INT_MASK, MSKTSFULLUPDINT, reg_int_mask);
    int_mask->msktxhangint = RU_FIELD_GET(0, XIF, INT_MASK, MSKTXHANGINT, reg_int_mask);
    int_mask->msknegtimeint = RU_FIELD_GET(0, XIF, INT_MASK, MSKNEGTIMEINT, reg_int_mask);
    int_mask->mskpmctsjttrint = RU_FIELD_GET(0, XIF, INT_MASK, MSKPMCTSJTTRINT, reg_int_mask);
    int_mask->msksecrxoutffint = RU_FIELD_GET(0, XIF, INT_MASK, MSKSECRXOUTFFINT, reg_int_mask);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xif_port_command_set(bdmf_boolean dataportbusy, uint8_t portselect, uint8_t portopcode, uint16_t portaddress)
{
    uint32_t reg_port_command=0;

#ifdef VALIDATE_PARMS
    if((dataportbusy >= _1BITS_MAX_VAL_) ||
       (portselect >= _6BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_port_command = RU_FIELD_SET(0, XIF, PORT_COMMAND, DATAPORTBUSY, reg_port_command, dataportbusy);
    reg_port_command = RU_FIELD_SET(0, XIF, PORT_COMMAND, PORTSELECT, reg_port_command, portselect);
    reg_port_command = RU_FIELD_SET(0, XIF, PORT_COMMAND, PORTOPCODE, reg_port_command, portopcode);
    reg_port_command = RU_FIELD_SET(0, XIF, PORT_COMMAND, PORTADDRESS, reg_port_command, portaddress);

    RU_REG_WRITE(0, XIF, PORT_COMMAND, reg_port_command);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xif_port_command_get(bdmf_boolean *dataportbusy, uint8_t *portselect, uint8_t *portopcode, uint16_t *portaddress)
{
    uint32_t reg_port_command=0;

#ifdef VALIDATE_PARMS
    if(!dataportbusy || !portselect || !portopcode || !portaddress)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, XIF, PORT_COMMAND, reg_port_command);

    *dataportbusy = RU_FIELD_GET(0, XIF, PORT_COMMAND, DATAPORTBUSY, reg_port_command);
    *portselect = RU_FIELD_GET(0, XIF, PORT_COMMAND, PORTSELECT, reg_port_command);
    *portopcode = RU_FIELD_GET(0, XIF, PORT_COMMAND, PORTOPCODE, reg_port_command);
    *portaddress = RU_FIELD_GET(0, XIF, PORT_COMMAND, PORTADDRESS, reg_port_command);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xif_port_data__set(uint8_t portidx, uint32_t portdata)
{
    uint32_t reg_port_data_=0;

#ifdef VALIDATE_PARMS
    if((portidx >= 8))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_port_data_ = RU_FIELD_SET(0, XIF, PORT_DATA_, PORTDATA, reg_port_data_, portdata);

    RU_REG_RAM_WRITE(0, portidx, XIF, PORT_DATA_, reg_port_data_);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xif_port_data__get(uint8_t portidx, uint32_t *portdata)
{
    uint32_t reg_port_data_=0;

#ifdef VALIDATE_PARMS
    if(!portdata)
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

    RU_REG_RAM_READ(0, portidx, XIF, PORT_DATA_, reg_port_data_);

    *portdata = RU_FIELD_GET(0, XIF, PORT_DATA_, PORTDATA, reg_port_data_);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xif_macsec_set(uint16_t cfgmacsecethertype)
{
    uint32_t reg_macsec=0;

#ifdef VALIDATE_PARMS
#endif

    reg_macsec = RU_FIELD_SET(0, XIF, MACSEC, CFGMACSECETHERTYPE, reg_macsec, cfgmacsecethertype);

    RU_REG_WRITE(0, XIF, MACSEC, reg_macsec);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xif_macsec_get(uint16_t *cfgmacsecethertype)
{
    uint32_t reg_macsec=0;

#ifdef VALIDATE_PARMS
    if(!cfgmacsecethertype)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, XIF, MACSEC, reg_macsec);

    *cfgmacsecethertype = RU_FIELD_GET(0, XIF, MACSEC, CFGMACSECETHERTYPE, reg_macsec);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xif_xpn_xmt_offset_set(uint16_t cfgxpnxmtoffset)
{
    uint32_t reg_xpn_xmt_offset=0;

#ifdef VALIDATE_PARMS
#endif

    reg_xpn_xmt_offset = RU_FIELD_SET(0, XIF, XPN_XMT_OFFSET, CFGXPNXMTOFFSET, reg_xpn_xmt_offset, cfgxpnxmtoffset);

    RU_REG_WRITE(0, XIF, XPN_XMT_OFFSET, reg_xpn_xmt_offset);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xif_xpn_xmt_offset_get(uint16_t *cfgxpnxmtoffset)
{
    uint32_t reg_xpn_xmt_offset=0;

#ifdef VALIDATE_PARMS
    if(!cfgxpnxmtoffset)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, XIF, XPN_XMT_OFFSET, reg_xpn_xmt_offset);

    *cfgxpnxmtoffset = RU_FIELD_GET(0, XIF, XPN_XMT_OFFSET, CFGXPNXMTOFFSET, reg_xpn_xmt_offset);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xif_xpn_timestamp_offset_set(uint32_t cfgxpnmpcptsoffset)
{
    uint32_t reg_xpn_timestamp_offset=0;

#ifdef VALIDATE_PARMS
#endif

    reg_xpn_timestamp_offset = RU_FIELD_SET(0, XIF, XPN_TIMESTAMP_OFFSET, CFGXPNMPCPTSOFFSET, reg_xpn_timestamp_offset, cfgxpnmpcptsoffset);

    RU_REG_WRITE(0, XIF, XPN_TIMESTAMP_OFFSET, reg_xpn_timestamp_offset);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xif_xpn_timestamp_offset_get(uint32_t *cfgxpnmpcptsoffset)
{
    uint32_t reg_xpn_timestamp_offset=0;

#ifdef VALIDATE_PARMS
    if(!cfgxpnmpcptsoffset)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, XIF, XPN_TIMESTAMP_OFFSET, reg_xpn_timestamp_offset);

    *cfgxpnmpcptsoffset = RU_FIELD_GET(0, XIF, XPN_TIMESTAMP_OFFSET, CFGXPNMPCPTSOFFSET, reg_xpn_timestamp_offset);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xif_xpn_pktgen_ctl_set(const xif_xpn_pktgen_ctl *xpn_pktgen_ctl)
{
    uint32_t reg_xpn_pktgen_ctl=0;

#ifdef VALIDATE_PARMS
    if(!xpn_pktgen_ctl)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((xpn_pktgen_ctl->cfgenbck2bckpktgen >= _1BITS_MAX_VAL_) ||
       (xpn_pktgen_ctl->cfgenallmpcppktgen >= _1BITS_MAX_VAL_) ||
       (xpn_pktgen_ctl->cfgxpnstartpktgen >= _1BITS_MAX_VAL_) ||
       (xpn_pktgen_ctl->cfgxpnenpktgen >= _1BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_xpn_pktgen_ctl = RU_FIELD_SET(0, XIF, XPN_PKTGEN_CTL, CFGONUBURSTSIZE, reg_xpn_pktgen_ctl, xpn_pktgen_ctl->cfgonuburstsize);
    reg_xpn_pktgen_ctl = RU_FIELD_SET(0, XIF, XPN_PKTGEN_CTL, CFGENBCK2BCKPKTGEN, reg_xpn_pktgen_ctl, xpn_pktgen_ctl->cfgenbck2bckpktgen);
    reg_xpn_pktgen_ctl = RU_FIELD_SET(0, XIF, XPN_PKTGEN_CTL, CFGENALLMPCPPKTGEN, reg_xpn_pktgen_ctl, xpn_pktgen_ctl->cfgenallmpcppktgen);
    reg_xpn_pktgen_ctl = RU_FIELD_SET(0, XIF, XPN_PKTGEN_CTL, CFGXPNSTARTPKTGEN, reg_xpn_pktgen_ctl, xpn_pktgen_ctl->cfgxpnstartpktgen);
    reg_xpn_pktgen_ctl = RU_FIELD_SET(0, XIF, XPN_PKTGEN_CTL, CFGXPNENPKTGEN, reg_xpn_pktgen_ctl, xpn_pktgen_ctl->cfgxpnenpktgen);

    RU_REG_WRITE(0, XIF, XPN_PKTGEN_CTL, reg_xpn_pktgen_ctl);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xif_xpn_pktgen_ctl_get(xif_xpn_pktgen_ctl *xpn_pktgen_ctl)
{
    uint32_t reg_xpn_pktgen_ctl=0;

#ifdef VALIDATE_PARMS
    if(!xpn_pktgen_ctl)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, XIF, XPN_PKTGEN_CTL, reg_xpn_pktgen_ctl);

    xpn_pktgen_ctl->cfgonuburstsize = RU_FIELD_GET(0, XIF, XPN_PKTGEN_CTL, CFGONUBURSTSIZE, reg_xpn_pktgen_ctl);
    xpn_pktgen_ctl->cfgenbck2bckpktgen = RU_FIELD_GET(0, XIF, XPN_PKTGEN_CTL, CFGENBCK2BCKPKTGEN, reg_xpn_pktgen_ctl);
    xpn_pktgen_ctl->cfgenallmpcppktgen = RU_FIELD_GET(0, XIF, XPN_PKTGEN_CTL, CFGENALLMPCPPKTGEN, reg_xpn_pktgen_ctl);
    xpn_pktgen_ctl->cfgxpnstartpktgen = RU_FIELD_GET(0, XIF, XPN_PKTGEN_CTL, CFGXPNSTARTPKTGEN, reg_xpn_pktgen_ctl);
    xpn_pktgen_ctl->cfgxpnenpktgen = RU_FIELD_GET(0, XIF, XPN_PKTGEN_CTL, CFGXPNENPKTGEN, reg_xpn_pktgen_ctl);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xif_xpn_pktgen_llid_set(uint16_t cfgxpnpktgenllid1, uint16_t cfgxpnpktgenllid0)
{
    uint32_t reg_xpn_pktgen_llid=0;

#ifdef VALIDATE_PARMS
#endif

    reg_xpn_pktgen_llid = RU_FIELD_SET(0, XIF, XPN_PKTGEN_LLID, CFGXPNPKTGENLLID1, reg_xpn_pktgen_llid, cfgxpnpktgenllid1);
    reg_xpn_pktgen_llid = RU_FIELD_SET(0, XIF, XPN_PKTGEN_LLID, CFGXPNPKTGENLLID0, reg_xpn_pktgen_llid, cfgxpnpktgenllid0);

    RU_REG_WRITE(0, XIF, XPN_PKTGEN_LLID, reg_xpn_pktgen_llid);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xif_xpn_pktgen_llid_get(uint16_t *cfgxpnpktgenllid1, uint16_t *cfgxpnpktgenllid0)
{
    uint32_t reg_xpn_pktgen_llid=0;

#ifdef VALIDATE_PARMS
    if(!cfgxpnpktgenllid1 || !cfgxpnpktgenllid0)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, XIF, XPN_PKTGEN_LLID, reg_xpn_pktgen_llid);

    *cfgxpnpktgenllid1 = RU_FIELD_GET(0, XIF, XPN_PKTGEN_LLID, CFGXPNPKTGENLLID1, reg_xpn_pktgen_llid);
    *cfgxpnpktgenllid0 = RU_FIELD_GET(0, XIF, XPN_PKTGEN_LLID, CFGXPNPKTGENLLID0, reg_xpn_pktgen_llid);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xif_xpn_pktgen_pkt_cnt_set(bdmf_boolean cfgxpnpktgenburstmode, uint32_t cfgxpnpktgenburstsize)
{
    uint32_t reg_xpn_pktgen_pkt_cnt=0;

#ifdef VALIDATE_PARMS
    if((cfgxpnpktgenburstmode >= _1BITS_MAX_VAL_) ||
       (cfgxpnpktgenburstsize >= _24BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_xpn_pktgen_pkt_cnt = RU_FIELD_SET(0, XIF, XPN_PKTGEN_PKT_CNT, CFGXPNPKTGENBURSTMODE, reg_xpn_pktgen_pkt_cnt, cfgxpnpktgenburstmode);
    reg_xpn_pktgen_pkt_cnt = RU_FIELD_SET(0, XIF, XPN_PKTGEN_PKT_CNT, CFGXPNPKTGENBURSTSIZE, reg_xpn_pktgen_pkt_cnt, cfgxpnpktgenburstsize);

    RU_REG_WRITE(0, XIF, XPN_PKTGEN_PKT_CNT, reg_xpn_pktgen_pkt_cnt);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xif_xpn_pktgen_pkt_cnt_get(bdmf_boolean *cfgxpnpktgenburstmode, uint32_t *cfgxpnpktgenburstsize)
{
    uint32_t reg_xpn_pktgen_pkt_cnt=0;

#ifdef VALIDATE_PARMS
    if(!cfgxpnpktgenburstmode || !cfgxpnpktgenburstsize)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, XIF, XPN_PKTGEN_PKT_CNT, reg_xpn_pktgen_pkt_cnt);

    *cfgxpnpktgenburstmode = RU_FIELD_GET(0, XIF, XPN_PKTGEN_PKT_CNT, CFGXPNPKTGENBURSTMODE, reg_xpn_pktgen_pkt_cnt);
    *cfgxpnpktgenburstsize = RU_FIELD_GET(0, XIF, XPN_PKTGEN_PKT_CNT, CFGXPNPKTGENBURSTSIZE, reg_xpn_pktgen_pkt_cnt);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xif_xpn_pktgen_pkt_size_set(bdmf_boolean cfgxpnpktgensizeincr, uint16_t cfgxpnpktgensizeend, uint16_t cfgxpnpktgensizestart)
{
    uint32_t reg_xpn_pktgen_pkt_size=0;

#ifdef VALIDATE_PARMS
    if((cfgxpnpktgensizeincr >= _1BITS_MAX_VAL_) ||
       (cfgxpnpktgensizeend >= _12BITS_MAX_VAL_) ||
       (cfgxpnpktgensizestart >= _12BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_xpn_pktgen_pkt_size = RU_FIELD_SET(0, XIF, XPN_PKTGEN_PKT_SIZE, CFGXPNPKTGENSIZEINCR, reg_xpn_pktgen_pkt_size, cfgxpnpktgensizeincr);
    reg_xpn_pktgen_pkt_size = RU_FIELD_SET(0, XIF, XPN_PKTGEN_PKT_SIZE, CFGXPNPKTGENSIZEEND, reg_xpn_pktgen_pkt_size, cfgxpnpktgensizeend);
    reg_xpn_pktgen_pkt_size = RU_FIELD_SET(0, XIF, XPN_PKTGEN_PKT_SIZE, CFGXPNPKTGENSIZESTART, reg_xpn_pktgen_pkt_size, cfgxpnpktgensizestart);

    RU_REG_WRITE(0, XIF, XPN_PKTGEN_PKT_SIZE, reg_xpn_pktgen_pkt_size);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xif_xpn_pktgen_pkt_size_get(bdmf_boolean *cfgxpnpktgensizeincr, uint16_t *cfgxpnpktgensizeend, uint16_t *cfgxpnpktgensizestart)
{
    uint32_t reg_xpn_pktgen_pkt_size=0;

#ifdef VALIDATE_PARMS
    if(!cfgxpnpktgensizeincr || !cfgxpnpktgensizeend || !cfgxpnpktgensizestart)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, XIF, XPN_PKTGEN_PKT_SIZE, reg_xpn_pktgen_pkt_size);

    *cfgxpnpktgensizeincr = RU_FIELD_GET(0, XIF, XPN_PKTGEN_PKT_SIZE, CFGXPNPKTGENSIZEINCR, reg_xpn_pktgen_pkt_size);
    *cfgxpnpktgensizeend = RU_FIELD_GET(0, XIF, XPN_PKTGEN_PKT_SIZE, CFGXPNPKTGENSIZEEND, reg_xpn_pktgen_pkt_size);
    *cfgxpnpktgensizestart = RU_FIELD_GET(0, XIF, XPN_PKTGEN_PKT_SIZE, CFGXPNPKTGENSIZESTART, reg_xpn_pktgen_pkt_size);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xif_xpn_pktgen_ipg_set(uint16_t cfgxpnpktgenbck2bckipg, uint16_t cfgxpnpktgenipg)
{
    uint32_t reg_xpn_pktgen_ipg=0;

#ifdef VALIDATE_PARMS
#endif

    reg_xpn_pktgen_ipg = RU_FIELD_SET(0, XIF, XPN_PKTGEN_IPG, CFGXPNPKTGENBCK2BCKIPG, reg_xpn_pktgen_ipg, cfgxpnpktgenbck2bckipg);
    reg_xpn_pktgen_ipg = RU_FIELD_SET(0, XIF, XPN_PKTGEN_IPG, CFGXPNPKTGENIPG, reg_xpn_pktgen_ipg, cfgxpnpktgenipg);

    RU_REG_WRITE(0, XIF, XPN_PKTGEN_IPG, reg_xpn_pktgen_ipg);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xif_xpn_pktgen_ipg_get(uint16_t *cfgxpnpktgenbck2bckipg, uint16_t *cfgxpnpktgenipg)
{
    uint32_t reg_xpn_pktgen_ipg=0;

#ifdef VALIDATE_PARMS
    if(!cfgxpnpktgenbck2bckipg || !cfgxpnpktgenipg)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, XIF, XPN_PKTGEN_IPG, reg_xpn_pktgen_ipg);

    *cfgxpnpktgenbck2bckipg = RU_FIELD_GET(0, XIF, XPN_PKTGEN_IPG, CFGXPNPKTGENBCK2BCKIPG, reg_xpn_pktgen_ipg);
    *cfgxpnpktgenipg = RU_FIELD_GET(0, XIF, XPN_PKTGEN_IPG, CFGXPNPKTGENIPG, reg_xpn_pktgen_ipg);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xif_ts_jitter_thresh_set(uint32_t cfgtsjttrthresh)
{
    uint32_t reg_ts_jitter_thresh=0;

#ifdef VALIDATE_PARMS
    if((cfgtsjttrthresh >= _31BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_ts_jitter_thresh = RU_FIELD_SET(0, XIF, TS_JITTER_THRESH, CFGTSJTTRTHRESH, reg_ts_jitter_thresh, cfgtsjttrthresh);

    RU_REG_WRITE(0, XIF, TS_JITTER_THRESH, reg_ts_jitter_thresh);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xif_ts_jitter_thresh_get(uint32_t *cfgtsjttrthresh)
{
    uint32_t reg_ts_jitter_thresh=0;

#ifdef VALIDATE_PARMS
    if(!cfgtsjttrthresh)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, XIF, TS_JITTER_THRESH, reg_ts_jitter_thresh);

    *cfgtsjttrthresh = RU_FIELD_GET(0, XIF, TS_JITTER_THRESH, CFGTSJTTRTHRESH, reg_ts_jitter_thresh);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xif_ts_update_set(uint16_t cfgtsfullupdthr, bdmf_boolean cfgenautotsupd, uint8_t cfgtsupdper)
{
    uint32_t reg_ts_update=0;

#ifdef VALIDATE_PARMS
    if((cfgenautotsupd >= _1BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_ts_update = RU_FIELD_SET(0, XIF, TS_UPDATE, CFGTSFULLUPDTHR, reg_ts_update, cfgtsfullupdthr);
    reg_ts_update = RU_FIELD_SET(0, XIF, TS_UPDATE, CFGENAUTOTSUPD, reg_ts_update, cfgenautotsupd);
    reg_ts_update = RU_FIELD_SET(0, XIF, TS_UPDATE, CFGTSUPDPER, reg_ts_update, cfgtsupdper);

    RU_REG_WRITE(0, XIF, TS_UPDATE, reg_ts_update);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xif_ts_update_get(uint16_t *cfgtsfullupdthr, bdmf_boolean *cfgenautotsupd, uint8_t *cfgtsupdper)
{
    uint32_t reg_ts_update=0;

#ifdef VALIDATE_PARMS
    if(!cfgtsfullupdthr || !cfgenautotsupd || !cfgtsupdper)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, XIF, TS_UPDATE, reg_ts_update);

    *cfgtsfullupdthr = RU_FIELD_GET(0, XIF, TS_UPDATE, CFGTSFULLUPDTHR, reg_ts_update);
    *cfgenautotsupd = RU_FIELD_GET(0, XIF, TS_UPDATE, CFGENAUTOTSUPD, reg_ts_update);
    *cfgtsupdper = RU_FIELD_GET(0, XIF, TS_UPDATE, CFGTSUPDPER, reg_ts_update);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xif_gnt_overhead_set(uint16_t cfggntoh)
{
    uint32_t reg_gnt_overhead=0;

#ifdef VALIDATE_PARMS
#endif

    reg_gnt_overhead = RU_FIELD_SET(0, XIF, GNT_OVERHEAD, CFGGNTOH, reg_gnt_overhead, cfggntoh);

    RU_REG_WRITE(0, XIF, GNT_OVERHEAD, reg_gnt_overhead);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xif_gnt_overhead_get(uint16_t *cfggntoh)
{
    uint32_t reg_gnt_overhead=0;

#ifdef VALIDATE_PARMS
    if(!cfggntoh)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, XIF, GNT_OVERHEAD, reg_gnt_overhead);

    *cfggntoh = RU_FIELD_GET(0, XIF, GNT_OVERHEAD, CFGGNTOH, reg_gnt_overhead);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xif_discover_overhead_set(uint16_t cfgdiscoh)
{
    uint32_t reg_discover_overhead=0;

#ifdef VALIDATE_PARMS
#endif

    reg_discover_overhead = RU_FIELD_SET(0, XIF, DISCOVER_OVERHEAD, CFGDISCOH, reg_discover_overhead, cfgdiscoh);

    RU_REG_WRITE(0, XIF, DISCOVER_OVERHEAD, reg_discover_overhead);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xif_discover_overhead_get(uint16_t *cfgdiscoh)
{
    uint32_t reg_discover_overhead=0;

#ifdef VALIDATE_PARMS
    if(!cfgdiscoh)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, XIF, DISCOVER_OVERHEAD, reg_discover_overhead);

    *cfgdiscoh = RU_FIELD_GET(0, XIF, DISCOVER_OVERHEAD, CFGDISCOH, reg_discover_overhead);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xif_discover_info_set(uint16_t cfgdiscinfofld)
{
    uint32_t reg_discover_info=0;

#ifdef VALIDATE_PARMS
#endif

    reg_discover_info = RU_FIELD_SET(0, XIF, DISCOVER_INFO, CFGDISCINFOFLD, reg_discover_info, cfgdiscinfofld);

    RU_REG_WRITE(0, XIF, DISCOVER_INFO, reg_discover_info);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xif_discover_info_get(uint16_t *cfgdiscinfofld)
{
    uint32_t reg_discover_info=0;

#ifdef VALIDATE_PARMS
    if(!cfgdiscinfofld)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, XIF, DISCOVER_INFO, reg_discover_info);

    *cfgdiscinfofld = RU_FIELD_GET(0, XIF, DISCOVER_INFO, CFGDISCINFOFLD, reg_discover_info);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xif_xpn_oversize_thresh_set(uint16_t cfgxpnovrszthresh)
{
    uint32_t reg_xpn_oversize_thresh=0;

#ifdef VALIDATE_PARMS
    if((cfgxpnovrszthresh >= _14BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_xpn_oversize_thresh = RU_FIELD_SET(0, XIF, XPN_OVERSIZE_THRESH, CFGXPNOVRSZTHRESH, reg_xpn_oversize_thresh, cfgxpnovrszthresh);

    RU_REG_WRITE(0, XIF, XPN_OVERSIZE_THRESH, reg_xpn_oversize_thresh);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xif_xpn_oversize_thresh_get(uint16_t *cfgxpnovrszthresh)
{
    uint32_t reg_xpn_oversize_thresh=0;

#ifdef VALIDATE_PARMS
    if(!cfgxpnovrszthresh)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, XIF, XPN_OVERSIZE_THRESH, reg_xpn_oversize_thresh);

    *cfgxpnovrszthresh = RU_FIELD_GET(0, XIF, XPN_OVERSIZE_THRESH, CFGXPNOVRSZTHRESH, reg_xpn_oversize_thresh);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xif_secrx_keynum_get(uint32_t *keystatrx)
{
    uint32_t reg_secrx_keynum=0;

#ifdef VALIDATE_PARMS
    if(!keystatrx)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, XIF, SECRX_KEYNUM, reg_secrx_keynum);

    *keystatrx = RU_FIELD_GET(0, XIF, SECRX_KEYNUM, KEYSTATRX, reg_secrx_keynum);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xif_secrx_encrypt_get(uint32_t *encrstatrx)
{
    uint32_t reg_secrx_encrypt=0;

#ifdef VALIDATE_PARMS
    if(!encrstatrx)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, XIF, SECRX_ENCRYPT, reg_secrx_encrypt);

    *encrstatrx = RU_FIELD_GET(0, XIF, SECRX_ENCRYPT, ENCRSTATRX, reg_secrx_encrypt);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xif_pmc_frame_rx_cnt_get(uint32_t *pmcrxframecnt)
{
    uint32_t reg_pmc_frame_rx_cnt=0;

#ifdef VALIDATE_PARMS
    if(!pmcrxframecnt)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, XIF, PMC_FRAME_RX_CNT, reg_pmc_frame_rx_cnt);

    *pmcrxframecnt = RU_FIELD_GET(0, XIF, PMC_FRAME_RX_CNT, PMCRXFRAMECNT, reg_pmc_frame_rx_cnt);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xif_pmc_byte_rx_cnt_get(uint32_t *pmcrxbytecnt)
{
    uint32_t reg_pmc_byte_rx_cnt=0;

#ifdef VALIDATE_PARMS
    if(!pmcrxbytecnt)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, XIF, PMC_BYTE_RX_CNT, reg_pmc_byte_rx_cnt);

    *pmcrxbytecnt = RU_FIELD_GET(0, XIF, PMC_BYTE_RX_CNT, PMCRXBYTECNT, reg_pmc_byte_rx_cnt);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xif_pmc_runt_rx_cnt_get(uint32_t *pmcrxruntcnt)
{
    uint32_t reg_pmc_runt_rx_cnt=0;

#ifdef VALIDATE_PARMS
    if(!pmcrxruntcnt)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, XIF, PMC_RUNT_RX_CNT, reg_pmc_runt_rx_cnt);

    *pmcrxruntcnt = RU_FIELD_GET(0, XIF, PMC_RUNT_RX_CNT, PMCRXRUNTCNT, reg_pmc_runt_rx_cnt);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xif_pmc_cw_err_rx_cnt_get(uint32_t *pmcrxcwerrcnt)
{
    uint32_t reg_pmc_cw_err_rx_cnt=0;

#ifdef VALIDATE_PARMS
    if(!pmcrxcwerrcnt)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, XIF, PMC_CW_ERR_RX_CNT, reg_pmc_cw_err_rx_cnt);

    *pmcrxcwerrcnt = RU_FIELD_GET(0, XIF, PMC_CW_ERR_RX_CNT, PMCRXCWERRCNT, reg_pmc_cw_err_rx_cnt);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xif_pmc_crc8_err_rx_cnt_get(uint32_t *pmcrxcrc8errcnt)
{
    uint32_t reg_pmc_crc8_err_rx_cnt=0;

#ifdef VALIDATE_PARMS
    if(!pmcrxcrc8errcnt)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, XIF, PMC_CRC8_ERR_RX_CNT, reg_pmc_crc8_err_rx_cnt);

    *pmcrxcrc8errcnt = RU_FIELD_GET(0, XIF, PMC_CRC8_ERR_RX_CNT, PMCRXCRC8ERRCNT, reg_pmc_crc8_err_rx_cnt);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xif_xpn_data_frm_cnt_get(uint32_t *xpndtframecnt)
{
    uint32_t reg_xpn_data_frm_cnt=0;

#ifdef VALIDATE_PARMS
    if(!xpndtframecnt)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, XIF, XPN_DATA_FRM_CNT, reg_xpn_data_frm_cnt);

    *xpndtframecnt = RU_FIELD_GET(0, XIF, XPN_DATA_FRM_CNT, XPNDTFRAMECNT, reg_xpn_data_frm_cnt);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xif_xpn_data_byte_cnt_get(uint32_t *xpndtbytecnt)
{
    uint32_t reg_xpn_data_byte_cnt=0;

#ifdef VALIDATE_PARMS
    if(!xpndtbytecnt)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, XIF, XPN_DATA_BYTE_CNT, reg_xpn_data_byte_cnt);

    *xpndtbytecnt = RU_FIELD_GET(0, XIF, XPN_DATA_BYTE_CNT, XPNDTBYTECNT, reg_xpn_data_byte_cnt);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xif_xpn_mpcp_frm_cnt_get(uint32_t *xpnmpcpframecnt)
{
    uint32_t reg_xpn_mpcp_frm_cnt=0;

#ifdef VALIDATE_PARMS
    if(!xpnmpcpframecnt)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, XIF, XPN_MPCP_FRM_CNT, reg_xpn_mpcp_frm_cnt);

    *xpnmpcpframecnt = RU_FIELD_GET(0, XIF, XPN_MPCP_FRM_CNT, XPNMPCPFRAMECNT, reg_xpn_mpcp_frm_cnt);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xif_xpn_oam_frm_cnt_get(uint32_t *xpnoamframecnt)
{
    uint32_t reg_xpn_oam_frm_cnt=0;

#ifdef VALIDATE_PARMS
    if(!xpnoamframecnt)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, XIF, XPN_OAM_FRM_CNT, reg_xpn_oam_frm_cnt);

    *xpnoamframecnt = RU_FIELD_GET(0, XIF, XPN_OAM_FRM_CNT, XPNOAMFRAMECNT, reg_xpn_oam_frm_cnt);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xif_xpn_oam_byte_cnt_get(uint32_t *xpnoambytecnt)
{
    uint32_t reg_xpn_oam_byte_cnt=0;

#ifdef VALIDATE_PARMS
    if(!xpnoambytecnt)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, XIF, XPN_OAM_BYTE_CNT, reg_xpn_oam_byte_cnt);

    *xpnoambytecnt = RU_FIELD_GET(0, XIF, XPN_OAM_BYTE_CNT, XPNOAMBYTECNT, reg_xpn_oam_byte_cnt);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xif_xpn_oversize_frm_cnt_get(uint32_t *xpndtoversizecnt)
{
    uint32_t reg_xpn_oversize_frm_cnt=0;

#ifdef VALIDATE_PARMS
    if(!xpndtoversizecnt)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, XIF, XPN_OVERSIZE_FRM_CNT, reg_xpn_oversize_frm_cnt);

    *xpndtoversizecnt = RU_FIELD_GET(0, XIF, XPN_OVERSIZE_FRM_CNT, XPNDTOVERSIZECNT, reg_xpn_oversize_frm_cnt);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xif_sec_abort_frm_cnt_get(uint32_t *secrxabortfrmcnt)
{
    uint32_t reg_sec_abort_frm_cnt=0;

#ifdef VALIDATE_PARMS
    if(!secrxabortfrmcnt)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, XIF, SEC_ABORT_FRM_CNT, reg_sec_abort_frm_cnt);

    *secrxabortfrmcnt = RU_FIELD_GET(0, XIF, SEC_ABORT_FRM_CNT, SECRXABORTFRMCNT, reg_sec_abort_frm_cnt);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xif_pmc_tx_neg_event_cnt_get(uint8_t *pmctxnegeventcnt)
{
    uint32_t reg_pmc_tx_neg_event_cnt=0;

#ifdef VALIDATE_PARMS
    if(!pmctxnegeventcnt)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, XIF, PMC_TX_NEG_EVENT_CNT, reg_pmc_tx_neg_event_cnt);

    *pmctxnegeventcnt = RU_FIELD_GET(0, XIF, PMC_TX_NEG_EVENT_CNT, PMCTXNEGEVENTCNT, reg_pmc_tx_neg_event_cnt);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xif_xpn_idle_pkt_cnt_get(uint16_t *xpnidleframecnt)
{
    uint32_t reg_xpn_idle_pkt_cnt=0;

#ifdef VALIDATE_PARMS
    if(!xpnidleframecnt)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, XIF, XPN_IDLE_PKT_CNT, reg_xpn_idle_pkt_cnt);

    *xpnidleframecnt = RU_FIELD_GET(0, XIF, XPN_IDLE_PKT_CNT, XPNIDLEFRAMECNT, reg_xpn_idle_pkt_cnt);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xif_llid_0_set(uint32_t cfgonullid0)
{
    uint32_t reg_llid_0=0;

#ifdef VALIDATE_PARMS
    if((cfgonullid0 >= _17BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_llid_0 = RU_FIELD_SET(0, XIF, LLID_0, CFGONULLID0, reg_llid_0, cfgonullid0);

    RU_REG_WRITE(0, XIF, LLID_0, reg_llid_0);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xif_llid_0_get(uint32_t *cfgonullid0)
{
    uint32_t reg_llid_0=0;

#ifdef VALIDATE_PARMS
    if(!cfgonullid0)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, XIF, LLID_0, reg_llid_0);

    *cfgonullid0 = RU_FIELD_GET(0, XIF, LLID_0, CFGONULLID0, reg_llid_0);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xif_llid_1_set(uint32_t cfgonullid1)
{
    uint32_t reg_llid_1=0;

#ifdef VALIDATE_PARMS
    if((cfgonullid1 >= _17BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_llid_1 = RU_FIELD_SET(0, XIF, LLID_1, CFGONULLID1, reg_llid_1, cfgonullid1);

    RU_REG_WRITE(0, XIF, LLID_1, reg_llid_1);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xif_llid_1_get(uint32_t *cfgonullid1)
{
    uint32_t reg_llid_1=0;

#ifdef VALIDATE_PARMS
    if(!cfgonullid1)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, XIF, LLID_1, reg_llid_1);

    *cfgonullid1 = RU_FIELD_GET(0, XIF, LLID_1, CFGONULLID1, reg_llid_1);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xif_llid_2_set(uint32_t cfgonullid2)
{
    uint32_t reg_llid_2=0;

#ifdef VALIDATE_PARMS
    if((cfgonullid2 >= _17BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_llid_2 = RU_FIELD_SET(0, XIF, LLID_2, CFGONULLID2, reg_llid_2, cfgonullid2);

    RU_REG_WRITE(0, XIF, LLID_2, reg_llid_2);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xif_llid_2_get(uint32_t *cfgonullid2)
{
    uint32_t reg_llid_2=0;

#ifdef VALIDATE_PARMS
    if(!cfgonullid2)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, XIF, LLID_2, reg_llid_2);

    *cfgonullid2 = RU_FIELD_GET(0, XIF, LLID_2, CFGONULLID2, reg_llid_2);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xif_llid_3_set(uint32_t cfgonullid3)
{
    uint32_t reg_llid_3=0;

#ifdef VALIDATE_PARMS
    if((cfgonullid3 >= _17BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_llid_3 = RU_FIELD_SET(0, XIF, LLID_3, CFGONULLID3, reg_llid_3, cfgonullid3);

    RU_REG_WRITE(0, XIF, LLID_3, reg_llid_3);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xif_llid_3_get(uint32_t *cfgonullid3)
{
    uint32_t reg_llid_3=0;

#ifdef VALIDATE_PARMS
    if(!cfgonullid3)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, XIF, LLID_3, reg_llid_3);

    *cfgonullid3 = RU_FIELD_GET(0, XIF, LLID_3, CFGONULLID3, reg_llid_3);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xif_llid_4_set(uint32_t cfgonullid4)
{
    uint32_t reg_llid_4=0;

#ifdef VALIDATE_PARMS
    if((cfgonullid4 >= _17BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_llid_4 = RU_FIELD_SET(0, XIF, LLID_4, CFGONULLID4, reg_llid_4, cfgonullid4);

    RU_REG_WRITE(0, XIF, LLID_4, reg_llid_4);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xif_llid_4_get(uint32_t *cfgonullid4)
{
    uint32_t reg_llid_4=0;

#ifdef VALIDATE_PARMS
    if(!cfgonullid4)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, XIF, LLID_4, reg_llid_4);

    *cfgonullid4 = RU_FIELD_GET(0, XIF, LLID_4, CFGONULLID4, reg_llid_4);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xif_llid_5_set(uint32_t cfgonullid5)
{
    uint32_t reg_llid_5=0;

#ifdef VALIDATE_PARMS
    if((cfgonullid5 >= _17BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_llid_5 = RU_FIELD_SET(0, XIF, LLID_5, CFGONULLID5, reg_llid_5, cfgonullid5);

    RU_REG_WRITE(0, XIF, LLID_5, reg_llid_5);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xif_llid_5_get(uint32_t *cfgonullid5)
{
    uint32_t reg_llid_5=0;

#ifdef VALIDATE_PARMS
    if(!cfgonullid5)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, XIF, LLID_5, reg_llid_5);

    *cfgonullid5 = RU_FIELD_GET(0, XIF, LLID_5, CFGONULLID5, reg_llid_5);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xif_llid_6_set(uint32_t cfgonullid6)
{
    uint32_t reg_llid_6=0;

#ifdef VALIDATE_PARMS
    if((cfgonullid6 >= _17BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_llid_6 = RU_FIELD_SET(0, XIF, LLID_6, CFGONULLID6, reg_llid_6, cfgonullid6);

    RU_REG_WRITE(0, XIF, LLID_6, reg_llid_6);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xif_llid_6_get(uint32_t *cfgonullid6)
{
    uint32_t reg_llid_6=0;

#ifdef VALIDATE_PARMS
    if(!cfgonullid6)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, XIF, LLID_6, reg_llid_6);

    *cfgonullid6 = RU_FIELD_GET(0, XIF, LLID_6, CFGONULLID6, reg_llid_6);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xif_llid_7_set(uint32_t cfgonullid7)
{
    uint32_t reg_llid_7=0;

#ifdef VALIDATE_PARMS
    if((cfgonullid7 >= _17BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_llid_7 = RU_FIELD_SET(0, XIF, LLID_7, CFGONULLID7, reg_llid_7, cfgonullid7);

    RU_REG_WRITE(0, XIF, LLID_7, reg_llid_7);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xif_llid_7_get(uint32_t *cfgonullid7)
{
    uint32_t reg_llid_7=0;

#ifdef VALIDATE_PARMS
    if(!cfgonullid7)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, XIF, LLID_7, reg_llid_7);

    *cfgonullid7 = RU_FIELD_GET(0, XIF, LLID_7, CFGONULLID7, reg_llid_7);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xif_llid_8_set(uint32_t cfgonullid8)
{
    uint32_t reg_llid_8=0;

#ifdef VALIDATE_PARMS
    if((cfgonullid8 >= _17BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_llid_8 = RU_FIELD_SET(0, XIF, LLID_8, CFGONULLID8, reg_llid_8, cfgonullid8);

    RU_REG_WRITE(0, XIF, LLID_8, reg_llid_8);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xif_llid_8_get(uint32_t *cfgonullid8)
{
    uint32_t reg_llid_8=0;

#ifdef VALIDATE_PARMS
    if(!cfgonullid8)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, XIF, LLID_8, reg_llid_8);

    *cfgonullid8 = RU_FIELD_GET(0, XIF, LLID_8, CFGONULLID8, reg_llid_8);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xif_llid_9_set(uint32_t cfgonullid9)
{
    uint32_t reg_llid_9=0;

#ifdef VALIDATE_PARMS
    if((cfgonullid9 >= _17BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_llid_9 = RU_FIELD_SET(0, XIF, LLID_9, CFGONULLID9, reg_llid_9, cfgonullid9);

    RU_REG_WRITE(0, XIF, LLID_9, reg_llid_9);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xif_llid_9_get(uint32_t *cfgonullid9)
{
    uint32_t reg_llid_9=0;

#ifdef VALIDATE_PARMS
    if(!cfgonullid9)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, XIF, LLID_9, reg_llid_9);

    *cfgonullid9 = RU_FIELD_GET(0, XIF, LLID_9, CFGONULLID9, reg_llid_9);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xif_llid_10_set(uint32_t cfgonullid10)
{
    uint32_t reg_llid_10=0;

#ifdef VALIDATE_PARMS
    if((cfgonullid10 >= _17BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_llid_10 = RU_FIELD_SET(0, XIF, LLID_10, CFGONULLID10, reg_llid_10, cfgonullid10);

    RU_REG_WRITE(0, XIF, LLID_10, reg_llid_10);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xif_llid_10_get(uint32_t *cfgonullid10)
{
    uint32_t reg_llid_10=0;

#ifdef VALIDATE_PARMS
    if(!cfgonullid10)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, XIF, LLID_10, reg_llid_10);

    *cfgonullid10 = RU_FIELD_GET(0, XIF, LLID_10, CFGONULLID10, reg_llid_10);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xif_llid_11_set(uint32_t cfgonullid11)
{
    uint32_t reg_llid_11=0;

#ifdef VALIDATE_PARMS
    if((cfgonullid11 >= _17BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_llid_11 = RU_FIELD_SET(0, XIF, LLID_11, CFGONULLID11, reg_llid_11, cfgonullid11);

    RU_REG_WRITE(0, XIF, LLID_11, reg_llid_11);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xif_llid_11_get(uint32_t *cfgonullid11)
{
    uint32_t reg_llid_11=0;

#ifdef VALIDATE_PARMS
    if(!cfgonullid11)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, XIF, LLID_11, reg_llid_11);

    *cfgonullid11 = RU_FIELD_GET(0, XIF, LLID_11, CFGONULLID11, reg_llid_11);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xif_llid_12_set(uint32_t cfgonullid12)
{
    uint32_t reg_llid_12=0;

#ifdef VALIDATE_PARMS
    if((cfgonullid12 >= _17BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_llid_12 = RU_FIELD_SET(0, XIF, LLID_12, CFGONULLID12, reg_llid_12, cfgonullid12);

    RU_REG_WRITE(0, XIF, LLID_12, reg_llid_12);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xif_llid_12_get(uint32_t *cfgonullid12)
{
    uint32_t reg_llid_12=0;

#ifdef VALIDATE_PARMS
    if(!cfgonullid12)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, XIF, LLID_12, reg_llid_12);

    *cfgonullid12 = RU_FIELD_GET(0, XIF, LLID_12, CFGONULLID12, reg_llid_12);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xif_llid_13_set(uint32_t cfgonullid13)
{
    uint32_t reg_llid_13=0;

#ifdef VALIDATE_PARMS
    if((cfgonullid13 >= _17BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_llid_13 = RU_FIELD_SET(0, XIF, LLID_13, CFGONULLID13, reg_llid_13, cfgonullid13);

    RU_REG_WRITE(0, XIF, LLID_13, reg_llid_13);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xif_llid_13_get(uint32_t *cfgonullid13)
{
    uint32_t reg_llid_13=0;

#ifdef VALIDATE_PARMS
    if(!cfgonullid13)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, XIF, LLID_13, reg_llid_13);

    *cfgonullid13 = RU_FIELD_GET(0, XIF, LLID_13, CFGONULLID13, reg_llid_13);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xif_llid_14_set(uint32_t cfgonullid14)
{
    uint32_t reg_llid_14=0;

#ifdef VALIDATE_PARMS
    if((cfgonullid14 >= _17BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_llid_14 = RU_FIELD_SET(0, XIF, LLID_14, CFGONULLID14, reg_llid_14, cfgonullid14);

    RU_REG_WRITE(0, XIF, LLID_14, reg_llid_14);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xif_llid_14_get(uint32_t *cfgonullid14)
{
    uint32_t reg_llid_14=0;

#ifdef VALIDATE_PARMS
    if(!cfgonullid14)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, XIF, LLID_14, reg_llid_14);

    *cfgonullid14 = RU_FIELD_GET(0, XIF, LLID_14, CFGONULLID14, reg_llid_14);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xif_llid_15_set(uint32_t cfgonullid15)
{
    uint32_t reg_llid_15=0;

#ifdef VALIDATE_PARMS
    if((cfgonullid15 >= _17BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_llid_15 = RU_FIELD_SET(0, XIF, LLID_15, CFGONULLID15, reg_llid_15, cfgonullid15);

    RU_REG_WRITE(0, XIF, LLID_15, reg_llid_15);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xif_llid_15_get(uint32_t *cfgonullid15)
{
    uint32_t reg_llid_15=0;

#ifdef VALIDATE_PARMS
    if(!cfgonullid15)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, XIF, LLID_15, reg_llid_15);

    *cfgonullid15 = RU_FIELD_GET(0, XIF, LLID_15, CFGONULLID15, reg_llid_15);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xif_llid_16_set(uint32_t cfgonullid16)
{
    uint32_t reg_llid_16=0;

#ifdef VALIDATE_PARMS
    if((cfgonullid16 >= _17BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_llid_16 = RU_FIELD_SET(0, XIF, LLID_16, CFGONULLID16, reg_llid_16, cfgonullid16);

    RU_REG_WRITE(0, XIF, LLID_16, reg_llid_16);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xif_llid_16_get(uint32_t *cfgonullid16)
{
    uint32_t reg_llid_16=0;

#ifdef VALIDATE_PARMS
    if(!cfgonullid16)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, XIF, LLID_16, reg_llid_16);

    *cfgonullid16 = RU_FIELD_GET(0, XIF, LLID_16, CFGONULLID16, reg_llid_16);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xif_llid_17_set(uint32_t cfgonullid17)
{
    uint32_t reg_llid_17=0;

#ifdef VALIDATE_PARMS
    if((cfgonullid17 >= _17BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_llid_17 = RU_FIELD_SET(0, XIF, LLID_17, CFGONULLID17, reg_llid_17, cfgonullid17);

    RU_REG_WRITE(0, XIF, LLID_17, reg_llid_17);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xif_llid_17_get(uint32_t *cfgonullid17)
{
    uint32_t reg_llid_17=0;

#ifdef VALIDATE_PARMS
    if(!cfgonullid17)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, XIF, LLID_17, reg_llid_17);

    *cfgonullid17 = RU_FIELD_GET(0, XIF, LLID_17, CFGONULLID17, reg_llid_17);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xif_llid_18_set(uint32_t cfgonullid18)
{
    uint32_t reg_llid_18=0;

#ifdef VALIDATE_PARMS
    if((cfgonullid18 >= _17BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_llid_18 = RU_FIELD_SET(0, XIF, LLID_18, CFGONULLID18, reg_llid_18, cfgonullid18);

    RU_REG_WRITE(0, XIF, LLID_18, reg_llid_18);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xif_llid_18_get(uint32_t *cfgonullid18)
{
    uint32_t reg_llid_18=0;

#ifdef VALIDATE_PARMS
    if(!cfgonullid18)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, XIF, LLID_18, reg_llid_18);

    *cfgonullid18 = RU_FIELD_GET(0, XIF, LLID_18, CFGONULLID18, reg_llid_18);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xif_llid_19_set(uint32_t cfgonullid19)
{
    uint32_t reg_llid_19=0;

#ifdef VALIDATE_PARMS
    if((cfgonullid19 >= _17BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_llid_19 = RU_FIELD_SET(0, XIF, LLID_19, CFGONULLID19, reg_llid_19, cfgonullid19);

    RU_REG_WRITE(0, XIF, LLID_19, reg_llid_19);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xif_llid_19_get(uint32_t *cfgonullid19)
{
    uint32_t reg_llid_19=0;

#ifdef VALIDATE_PARMS
    if(!cfgonullid19)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, XIF, LLID_19, reg_llid_19);

    *cfgonullid19 = RU_FIELD_GET(0, XIF, LLID_19, CFGONULLID19, reg_llid_19);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xif_llid_20_set(uint32_t cfgonullid20)
{
    uint32_t reg_llid_20=0;

#ifdef VALIDATE_PARMS
    if((cfgonullid20 >= _17BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_llid_20 = RU_FIELD_SET(0, XIF, LLID_20, CFGONULLID20, reg_llid_20, cfgonullid20);

    RU_REG_WRITE(0, XIF, LLID_20, reg_llid_20);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xif_llid_20_get(uint32_t *cfgonullid20)
{
    uint32_t reg_llid_20=0;

#ifdef VALIDATE_PARMS
    if(!cfgonullid20)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, XIF, LLID_20, reg_llid_20);

    *cfgonullid20 = RU_FIELD_GET(0, XIF, LLID_20, CFGONULLID20, reg_llid_20);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xif_llid_21_set(uint32_t cfgonullid21)
{
    uint32_t reg_llid_21=0;

#ifdef VALIDATE_PARMS
    if((cfgonullid21 >= _17BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_llid_21 = RU_FIELD_SET(0, XIF, LLID_21, CFGONULLID21, reg_llid_21, cfgonullid21);

    RU_REG_WRITE(0, XIF, LLID_21, reg_llid_21);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xif_llid_21_get(uint32_t *cfgonullid21)
{
    uint32_t reg_llid_21=0;

#ifdef VALIDATE_PARMS
    if(!cfgonullid21)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, XIF, LLID_21, reg_llid_21);

    *cfgonullid21 = RU_FIELD_GET(0, XIF, LLID_21, CFGONULLID21, reg_llid_21);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xif_llid_22_set(uint32_t cfgonullid22)
{
    uint32_t reg_llid_22=0;

#ifdef VALIDATE_PARMS
    if((cfgonullid22 >= _17BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_llid_22 = RU_FIELD_SET(0, XIF, LLID_22, CFGONULLID22, reg_llid_22, cfgonullid22);

    RU_REG_WRITE(0, XIF, LLID_22, reg_llid_22);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xif_llid_22_get(uint32_t *cfgonullid22)
{
    uint32_t reg_llid_22=0;

#ifdef VALIDATE_PARMS
    if(!cfgonullid22)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, XIF, LLID_22, reg_llid_22);

    *cfgonullid22 = RU_FIELD_GET(0, XIF, LLID_22, CFGONULLID22, reg_llid_22);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xif_llid_23_set(uint32_t cfgonullid23)
{
    uint32_t reg_llid_23=0;

#ifdef VALIDATE_PARMS
    if((cfgonullid23 >= _17BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_llid_23 = RU_FIELD_SET(0, XIF, LLID_23, CFGONULLID23, reg_llid_23, cfgonullid23);

    RU_REG_WRITE(0, XIF, LLID_23, reg_llid_23);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xif_llid_23_get(uint32_t *cfgonullid23)
{
    uint32_t reg_llid_23=0;

#ifdef VALIDATE_PARMS
    if(!cfgonullid23)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, XIF, LLID_23, reg_llid_23);

    *cfgonullid23 = RU_FIELD_GET(0, XIF, LLID_23, CFGONULLID23, reg_llid_23);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xif_llid_24_set(uint32_t cfgonullid24)
{
    uint32_t reg_llid_24=0;

#ifdef VALIDATE_PARMS
    if((cfgonullid24 >= _17BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_llid_24 = RU_FIELD_SET(0, XIF, LLID_24, CFGONULLID24, reg_llid_24, cfgonullid24);

    RU_REG_WRITE(0, XIF, LLID_24, reg_llid_24);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xif_llid_24_get(uint32_t *cfgonullid24)
{
    uint32_t reg_llid_24=0;

#ifdef VALIDATE_PARMS
    if(!cfgonullid24)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, XIF, LLID_24, reg_llid_24);

    *cfgonullid24 = RU_FIELD_GET(0, XIF, LLID_24, CFGONULLID24, reg_llid_24);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xif_llid_25_set(uint32_t cfgonullid25)
{
    uint32_t reg_llid_25=0;

#ifdef VALIDATE_PARMS
    if((cfgonullid25 >= _17BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_llid_25 = RU_FIELD_SET(0, XIF, LLID_25, CFGONULLID25, reg_llid_25, cfgonullid25);

    RU_REG_WRITE(0, XIF, LLID_25, reg_llid_25);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xif_llid_25_get(uint32_t *cfgonullid25)
{
    uint32_t reg_llid_25=0;

#ifdef VALIDATE_PARMS
    if(!cfgonullid25)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, XIF, LLID_25, reg_llid_25);

    *cfgonullid25 = RU_FIELD_GET(0, XIF, LLID_25, CFGONULLID25, reg_llid_25);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xif_llid_26_set(uint32_t cfgonullid26)
{
    uint32_t reg_llid_26=0;

#ifdef VALIDATE_PARMS
    if((cfgonullid26 >= _17BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_llid_26 = RU_FIELD_SET(0, XIF, LLID_26, CFGONULLID26, reg_llid_26, cfgonullid26);

    RU_REG_WRITE(0, XIF, LLID_26, reg_llid_26);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xif_llid_26_get(uint32_t *cfgonullid26)
{
    uint32_t reg_llid_26=0;

#ifdef VALIDATE_PARMS
    if(!cfgonullid26)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, XIF, LLID_26, reg_llid_26);

    *cfgonullid26 = RU_FIELD_GET(0, XIF, LLID_26, CFGONULLID26, reg_llid_26);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xif_llid_27_set(uint32_t cfgonullid27)
{
    uint32_t reg_llid_27=0;

#ifdef VALIDATE_PARMS
    if((cfgonullid27 >= _17BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_llid_27 = RU_FIELD_SET(0, XIF, LLID_27, CFGONULLID27, reg_llid_27, cfgonullid27);

    RU_REG_WRITE(0, XIF, LLID_27, reg_llid_27);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xif_llid_27_get(uint32_t *cfgonullid27)
{
    uint32_t reg_llid_27=0;

#ifdef VALIDATE_PARMS
    if(!cfgonullid27)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, XIF, LLID_27, reg_llid_27);

    *cfgonullid27 = RU_FIELD_GET(0, XIF, LLID_27, CFGONULLID27, reg_llid_27);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xif_llid_28_set(uint32_t cfgonullid28)
{
    uint32_t reg_llid_28=0;

#ifdef VALIDATE_PARMS
    if((cfgonullid28 >= _17BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_llid_28 = RU_FIELD_SET(0, XIF, LLID_28, CFGONULLID28, reg_llid_28, cfgonullid28);

    RU_REG_WRITE(0, XIF, LLID_28, reg_llid_28);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xif_llid_28_get(uint32_t *cfgonullid28)
{
    uint32_t reg_llid_28=0;

#ifdef VALIDATE_PARMS
    if(!cfgonullid28)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, XIF, LLID_28, reg_llid_28);

    *cfgonullid28 = RU_FIELD_GET(0, XIF, LLID_28, CFGONULLID28, reg_llid_28);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xif_llid_29_set(uint32_t cfgonullid29)
{
    uint32_t reg_llid_29=0;

#ifdef VALIDATE_PARMS
    if((cfgonullid29 >= _17BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_llid_29 = RU_FIELD_SET(0, XIF, LLID_29, CFGONULLID29, reg_llid_29, cfgonullid29);

    RU_REG_WRITE(0, XIF, LLID_29, reg_llid_29);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xif_llid_29_get(uint32_t *cfgonullid29)
{
    uint32_t reg_llid_29=0;

#ifdef VALIDATE_PARMS
    if(!cfgonullid29)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, XIF, LLID_29, reg_llid_29);

    *cfgonullid29 = RU_FIELD_GET(0, XIF, LLID_29, CFGONULLID29, reg_llid_29);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xif_llid_30_set(uint32_t cfgonullid30)
{
    uint32_t reg_llid_30=0;

#ifdef VALIDATE_PARMS
    if((cfgonullid30 >= _17BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_llid_30 = RU_FIELD_SET(0, XIF, LLID_30, CFGONULLID30, reg_llid_30, cfgonullid30);

    RU_REG_WRITE(0, XIF, LLID_30, reg_llid_30);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xif_llid_30_get(uint32_t *cfgonullid30)
{
    uint32_t reg_llid_30=0;

#ifdef VALIDATE_PARMS
    if(!cfgonullid30)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, XIF, LLID_30, reg_llid_30);

    *cfgonullid30 = RU_FIELD_GET(0, XIF, LLID_30, CFGONULLID30, reg_llid_30);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xif_llid_31_set(uint32_t cfgonullid31)
{
    uint32_t reg_llid_31=0;

#ifdef VALIDATE_PARMS
    if((cfgonullid31 >= _17BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_llid_31 = RU_FIELD_SET(0, XIF, LLID_31, CFGONULLID31, reg_llid_31, cfgonullid31);

    RU_REG_WRITE(0, XIF, LLID_31, reg_llid_31);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xif_llid_31_get(uint32_t *cfgonullid31)
{
    uint32_t reg_llid_31=0;

#ifdef VALIDATE_PARMS
    if(!cfgonullid31)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, XIF, LLID_31, reg_llid_31);

    *cfgonullid31 = RU_FIELD_GET(0, XIF, LLID_31, CFGONULLID31, reg_llid_31);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xif_max_mpcp_update_set(uint32_t cfgmaxposmpcpupd)
{
    uint32_t reg_max_mpcp_update=0;

#ifdef VALIDATE_PARMS
#endif

    reg_max_mpcp_update = RU_FIELD_SET(0, XIF, MAX_MPCP_UPDATE, CFGMAXPOSMPCPUPD, reg_max_mpcp_update, cfgmaxposmpcpupd);

    RU_REG_WRITE(0, XIF, MAX_MPCP_UPDATE, reg_max_mpcp_update);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xif_max_mpcp_update_get(uint32_t *cfgmaxposmpcpupd)
{
    uint32_t reg_max_mpcp_update=0;

#ifdef VALIDATE_PARMS
    if(!cfgmaxposmpcpupd)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, XIF, MAX_MPCP_UPDATE, reg_max_mpcp_update);

    *cfgmaxposmpcpupd = RU_FIELD_GET(0, XIF, MAX_MPCP_UPDATE, CFGMAXPOSMPCPUPD, reg_max_mpcp_update);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xif_ipg_insertion_set(bdmf_boolean cfgshortipg, bdmf_boolean cfginsertipg, uint8_t cfgipgword)
{
    uint32_t reg_ipg_insertion=0;

#ifdef VALIDATE_PARMS
    if((cfgshortipg >= _1BITS_MAX_VAL_) ||
       (cfginsertipg >= _1BITS_MAX_VAL_) ||
       (cfgipgword >= _7BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_ipg_insertion = RU_FIELD_SET(0, XIF, IPG_INSERTION, CFGSHORTIPG, reg_ipg_insertion, cfgshortipg);
    reg_ipg_insertion = RU_FIELD_SET(0, XIF, IPG_INSERTION, CFGINSERTIPG, reg_ipg_insertion, cfginsertipg);
    reg_ipg_insertion = RU_FIELD_SET(0, XIF, IPG_INSERTION, CFGIPGWORD, reg_ipg_insertion, cfgipgword);

    RU_REG_WRITE(0, XIF, IPG_INSERTION, reg_ipg_insertion);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xif_ipg_insertion_get(bdmf_boolean *cfgshortipg, bdmf_boolean *cfginsertipg, uint8_t *cfgipgword)
{
    uint32_t reg_ipg_insertion=0;

#ifdef VALIDATE_PARMS
    if(!cfgshortipg || !cfginsertipg || !cfgipgword)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, XIF, IPG_INSERTION, reg_ipg_insertion);

    *cfgshortipg = RU_FIELD_GET(0, XIF, IPG_INSERTION, CFGSHORTIPG, reg_ipg_insertion);
    *cfginsertipg = RU_FIELD_GET(0, XIF, IPG_INSERTION, CFGINSERTIPG, reg_ipg_insertion);
    *cfgipgword = RU_FIELD_GET(0, XIF, IPG_INSERTION, CFGIPGWORD, reg_ipg_insertion);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xif_transport_time_set(uint32_t cftransporttime)
{
    uint32_t reg_transport_time=0;

#ifdef VALIDATE_PARMS
#endif

    reg_transport_time = RU_FIELD_SET(0, XIF, TRANSPORT_TIME, CFTRANSPORTTIME, reg_transport_time, cftransporttime);

    RU_REG_WRITE(0, XIF, TRANSPORT_TIME, reg_transport_time);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xif_transport_time_get(uint32_t *cftransporttime)
{
    uint32_t reg_transport_time=0;

#ifdef VALIDATE_PARMS
    if(!cftransporttime)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, XIF, TRANSPORT_TIME, reg_transport_time);

    *cftransporttime = RU_FIELD_GET(0, XIF, TRANSPORT_TIME, CFTRANSPORTTIME, reg_transport_time);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xif_mpcp_time_get(uint32_t *curmpcpts)
{
    uint32_t reg_mpcp_time=0;

#ifdef VALIDATE_PARMS
    if(!curmpcpts)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, XIF, MPCP_TIME, reg_mpcp_time);

    *curmpcpts = RU_FIELD_GET(0, XIF, MPCP_TIME, CURMPCPTS, reg_mpcp_time);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xif_overlap_gnt_oh_set(uint32_t cfgovrlpoh)
{
    uint32_t reg_overlap_gnt_oh=0;

#ifdef VALIDATE_PARMS
#endif

    reg_overlap_gnt_oh = RU_FIELD_SET(0, XIF, OVERLAP_GNT_OH, CFGOVRLPOH, reg_overlap_gnt_oh, cfgovrlpoh);

    RU_REG_WRITE(0, XIF, OVERLAP_GNT_OH, reg_overlap_gnt_oh);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xif_overlap_gnt_oh_get(uint32_t *cfgovrlpoh)
{
    uint32_t reg_overlap_gnt_oh=0;

#ifdef VALIDATE_PARMS
    if(!cfgovrlpoh)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, XIF, OVERLAP_GNT_OH, reg_overlap_gnt_oh);

    *cfgovrlpoh = RU_FIELD_GET(0, XIF, OVERLAP_GNT_OH, CFGOVRLPOH, reg_overlap_gnt_oh);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xif_mac_mode_set(bdmf_boolean cfgennogntxmt)
{
    uint32_t reg_mac_mode=0;

#ifdef VALIDATE_PARMS
    if((cfgennogntxmt >= _1BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_mac_mode = RU_FIELD_SET(0, XIF, MAC_MODE, CFGENNOGNTXMT, reg_mac_mode, cfgennogntxmt);

    RU_REG_WRITE(0, XIF, MAC_MODE, reg_mac_mode);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xif_mac_mode_get(bdmf_boolean *cfgennogntxmt)
{
    uint32_t reg_mac_mode=0;

#ifdef VALIDATE_PARMS
    if(!cfgennogntxmt)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, XIF, MAC_MODE, reg_mac_mode);

    *cfgennogntxmt = RU_FIELD_GET(0, XIF, MAC_MODE, CFGENNOGNTXMT, reg_mac_mode);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xif_pmctx_ctl_set(const xif_pmctx_ctl *pmctx_ctl)
{
    uint32_t reg_pmctx_ctl=0;

#ifdef VALIDATE_PARMS
    if(!pmctx_ctl)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((pmctx_ctl->cfgdis4idleb4startchar >= _1BITS_MAX_VAL_) ||
       (pmctx_ctl->cfgenidledscrd >= _1BITS_MAX_VAL_) ||
       (pmctx_ctl->cfgseltxpontime >= _1BITS_MAX_VAL_) ||
       (pmctx_ctl->cfgmpcpcontupd >= _1BITS_MAX_VAL_) ||
       (pmctx_ctl->cfgenmaxmpcpupd >= _1BITS_MAX_VAL_) ||
       (pmctx_ctl->cfgennegtimeabort >= _1BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_pmctx_ctl = RU_FIELD_SET(0, XIF, PMCTX_CTL, CFGMPCPUPDPERIOD, reg_pmctx_ctl, pmctx_ctl->cfgmpcpupdperiod);
    reg_pmctx_ctl = RU_FIELD_SET(0, XIF, PMCTX_CTL, CFGDIS4IDLEB4STARTCHAR, reg_pmctx_ctl, pmctx_ctl->cfgdis4idleb4startchar);
    reg_pmctx_ctl = RU_FIELD_SET(0, XIF, PMCTX_CTL, CFGENIDLEDSCRD, reg_pmctx_ctl, pmctx_ctl->cfgenidledscrd);
    reg_pmctx_ctl = RU_FIELD_SET(0, XIF, PMCTX_CTL, CFGSELTXPONTIME, reg_pmctx_ctl, pmctx_ctl->cfgseltxpontime);
    reg_pmctx_ctl = RU_FIELD_SET(0, XIF, PMCTX_CTL, CFGMPCPCONTUPD, reg_pmctx_ctl, pmctx_ctl->cfgmpcpcontupd);
    reg_pmctx_ctl = RU_FIELD_SET(0, XIF, PMCTX_CTL, CFGENMAXMPCPUPD, reg_pmctx_ctl, pmctx_ctl->cfgenmaxmpcpupd);
    reg_pmctx_ctl = RU_FIELD_SET(0, XIF, PMCTX_CTL, CFGENNEGTIMEABORT, reg_pmctx_ctl, pmctx_ctl->cfgennegtimeabort);

    RU_REG_WRITE(0, XIF, PMCTX_CTL, reg_pmctx_ctl);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xif_pmctx_ctl_get(xif_pmctx_ctl *pmctx_ctl)
{
    uint32_t reg_pmctx_ctl=0;

#ifdef VALIDATE_PARMS
    if(!pmctx_ctl)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, XIF, PMCTX_CTL, reg_pmctx_ctl);

    pmctx_ctl->cfgmpcpupdperiod = RU_FIELD_GET(0, XIF, PMCTX_CTL, CFGMPCPUPDPERIOD, reg_pmctx_ctl);
    pmctx_ctl->cfgdis4idleb4startchar = RU_FIELD_GET(0, XIF, PMCTX_CTL, CFGDIS4IDLEB4STARTCHAR, reg_pmctx_ctl);
    pmctx_ctl->cfgenidledscrd = RU_FIELD_GET(0, XIF, PMCTX_CTL, CFGENIDLEDSCRD, reg_pmctx_ctl);
    pmctx_ctl->cfgseltxpontime = RU_FIELD_GET(0, XIF, PMCTX_CTL, CFGSELTXPONTIME, reg_pmctx_ctl);
    pmctx_ctl->cfgmpcpcontupd = RU_FIELD_GET(0, XIF, PMCTX_CTL, CFGMPCPCONTUPD, reg_pmctx_ctl);
    pmctx_ctl->cfgenmaxmpcpupd = RU_FIELD_GET(0, XIF, PMCTX_CTL, CFGENMAXMPCPUPD, reg_pmctx_ctl);
    pmctx_ctl->cfgennegtimeabort = RU_FIELD_GET(0, XIF, PMCTX_CTL, CFGENNEGTIMEABORT, reg_pmctx_ctl);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xif_sec_ctl_set(bdmf_boolean cfgsecrxenshortlen, bdmf_boolean cfgensectxfakeaes, bdmf_boolean cfgensecrxfakeaes, bdmf_boolean cfgenaereplayprct)
{
    uint32_t reg_sec_ctl=0;

#ifdef VALIDATE_PARMS
    if((cfgsecrxenshortlen >= _1BITS_MAX_VAL_) ||
       (cfgensectxfakeaes >= _1BITS_MAX_VAL_) ||
       (cfgensecrxfakeaes >= _1BITS_MAX_VAL_) ||
       (cfgenaereplayprct >= _1BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_sec_ctl = RU_FIELD_SET(0, XIF, SEC_CTL, CFGSECRXENSHORTLEN, reg_sec_ctl, cfgsecrxenshortlen);
    reg_sec_ctl = RU_FIELD_SET(0, XIF, SEC_CTL, CFGENSECTXFAKEAES, reg_sec_ctl, cfgensectxfakeaes);
    reg_sec_ctl = RU_FIELD_SET(0, XIF, SEC_CTL, CFGENSECRXFAKEAES, reg_sec_ctl, cfgensecrxfakeaes);
    reg_sec_ctl = RU_FIELD_SET(0, XIF, SEC_CTL, CFGENAEREPLAYPRCT, reg_sec_ctl, cfgenaereplayprct);

    RU_REG_WRITE(0, XIF, SEC_CTL, reg_sec_ctl);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xif_sec_ctl_get(bdmf_boolean *cfgsecrxenshortlen, bdmf_boolean *cfgensectxfakeaes, bdmf_boolean *cfgensecrxfakeaes, bdmf_boolean *cfgenaereplayprct)
{
    uint32_t reg_sec_ctl=0;

#ifdef VALIDATE_PARMS
    if(!cfgsecrxenshortlen || !cfgensectxfakeaes || !cfgensecrxfakeaes || !cfgenaereplayprct)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, XIF, SEC_CTL, reg_sec_ctl);

    *cfgsecrxenshortlen = RU_FIELD_GET(0, XIF, SEC_CTL, CFGSECRXENSHORTLEN, reg_sec_ctl);
    *cfgensectxfakeaes = RU_FIELD_GET(0, XIF, SEC_CTL, CFGENSECTXFAKEAES, reg_sec_ctl);
    *cfgensecrxfakeaes = RU_FIELD_GET(0, XIF, SEC_CTL, CFGENSECRXFAKEAES, reg_sec_ctl);
    *cfgenaereplayprct = RU_FIELD_GET(0, XIF, SEC_CTL, CFGENAEREPLAYPRCT, reg_sec_ctl);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xif_ae_pktnum_window_set(uint32_t cfgaepktnumwnd)
{
    uint32_t reg_ae_pktnum_window=0;

#ifdef VALIDATE_PARMS
#endif

    reg_ae_pktnum_window = RU_FIELD_SET(0, XIF, AE_PKTNUM_WINDOW, CFGAEPKTNUMWND, reg_ae_pktnum_window, cfgaepktnumwnd);

    RU_REG_WRITE(0, XIF, AE_PKTNUM_WINDOW, reg_ae_pktnum_window);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xif_ae_pktnum_window_get(uint32_t *cfgaepktnumwnd)
{
    uint32_t reg_ae_pktnum_window=0;

#ifdef VALIDATE_PARMS
    if(!cfgaepktnumwnd)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, XIF, AE_PKTNUM_WINDOW, reg_ae_pktnum_window);

    *cfgaepktnumwnd = RU_FIELD_GET(0, XIF, AE_PKTNUM_WINDOW, CFGAEPKTNUMWND, reg_ae_pktnum_window);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xif_ae_pktnum_thresh_set(uint32_t cfgpktnummaxthresh)
{
    uint32_t reg_ae_pktnum_thresh=0;

#ifdef VALIDATE_PARMS
#endif

    reg_ae_pktnum_thresh = RU_FIELD_SET(0, XIF, AE_PKTNUM_THRESH, CFGPKTNUMMAXTHRESH, reg_ae_pktnum_thresh, cfgpktnummaxthresh);

    RU_REG_WRITE(0, XIF, AE_PKTNUM_THRESH, reg_ae_pktnum_thresh);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xif_ae_pktnum_thresh_get(uint32_t *cfgpktnummaxthresh)
{
    uint32_t reg_ae_pktnum_thresh=0;

#ifdef VALIDATE_PARMS
    if(!cfgpktnummaxthresh)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, XIF, AE_PKTNUM_THRESH, reg_ae_pktnum_thresh);

    *cfgpktnummaxthresh = RU_FIELD_GET(0, XIF, AE_PKTNUM_THRESH, CFGPKTNUMMAXTHRESH, reg_ae_pktnum_thresh);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xif_sectx_keynum_get(uint32_t *keystattx)
{
    uint32_t reg_sectx_keynum=0;

#ifdef VALIDATE_PARMS
    if(!keystattx)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, XIF, SECTX_KEYNUM, reg_sectx_keynum);

    *keystattx = RU_FIELD_GET(0, XIF, SECTX_KEYNUM, KEYSTATTX, reg_sectx_keynum);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xif_sectx_encrypt_get(uint32_t *encrstattx)
{
    uint32_t reg_sectx_encrypt=0;

#ifdef VALIDATE_PARMS
    if(!encrstattx)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, XIF, SECTX_ENCRYPT, reg_sectx_encrypt);

    *encrstattx = RU_FIELD_GET(0, XIF, SECTX_ENCRYPT, ENCRSTATTX, reg_sectx_encrypt);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xif_ae_pktnum_stat_get(uint8_t *sectxindxwtpktnummax, uint8_t *secrxindxwtpktnumabort)
{
    uint32_t reg_ae_pktnum_stat=0;

#ifdef VALIDATE_PARMS
    if(!sectxindxwtpktnummax || !secrxindxwtpktnumabort)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, XIF, AE_PKTNUM_STAT, reg_ae_pktnum_stat);

    *sectxindxwtpktnummax = RU_FIELD_GET(0, XIF, AE_PKTNUM_STAT, SECTXINDXWTPKTNUMMAX, reg_ae_pktnum_stat);
    *secrxindxwtpktnumabort = RU_FIELD_GET(0, XIF, AE_PKTNUM_STAT, SECRXINDXWTPKTNUMABORT, reg_ae_pktnum_stat);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xif_mpcp_update_get(uint32_t *mpcpupdperiod)
{
    uint32_t reg_mpcp_update=0;

#ifdef VALIDATE_PARMS
    if(!mpcpupdperiod)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, XIF, MPCP_UPDATE, reg_mpcp_update);

    *mpcpupdperiod = RU_FIELD_GET(0, XIF, MPCP_UPDATE, MPCPUPDPERIOD, reg_mpcp_update);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xif_burst_prelaunch_offset_set(uint32_t cfgburstprelaunchoffset)
{
    uint32_t reg_burst_prelaunch_offset=0;

#ifdef VALIDATE_PARMS
#endif

    reg_burst_prelaunch_offset = RU_FIELD_SET(0, XIF, BURST_PRELAUNCH_OFFSET, CFGBURSTPRELAUNCHOFFSET, reg_burst_prelaunch_offset, cfgburstprelaunchoffset);

    RU_REG_WRITE(0, XIF, BURST_PRELAUNCH_OFFSET, reg_burst_prelaunch_offset);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xif_burst_prelaunch_offset_get(uint32_t *cfgburstprelaunchoffset)
{
    uint32_t reg_burst_prelaunch_offset=0;

#ifdef VALIDATE_PARMS
    if(!cfgburstprelaunchoffset)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, XIF, BURST_PRELAUNCH_OFFSET, reg_burst_prelaunch_offset);

    *cfgburstprelaunchoffset = RU_FIELD_GET(0, XIF, BURST_PRELAUNCH_OFFSET, CFGBURSTPRELAUNCHOFFSET, reg_burst_prelaunch_offset);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xif_vlan_type_set(uint16_t cfgvlantype)
{
    uint32_t reg_vlan_type=0;

#ifdef VALIDATE_PARMS
#endif

    reg_vlan_type = RU_FIELD_SET(0, XIF, VLAN_TYPE, CFGVLANTYPE, reg_vlan_type, cfgvlantype);

    RU_REG_WRITE(0, XIF, VLAN_TYPE, reg_vlan_type);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xif_vlan_type_get(uint16_t *cfgvlantype)
{
    uint32_t reg_vlan_type=0;

#ifdef VALIDATE_PARMS
    if(!cfgvlantype)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, XIF, VLAN_TYPE, reg_vlan_type);

    *cfgvlantype = RU_FIELD_GET(0, XIF, VLAN_TYPE, CFGVLANTYPE, reg_vlan_type);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xif_p2p_ae_sci_en_set(uint16_t cfgp2pscien)
{
    uint32_t reg_p2p_ae_sci_en=0;

#ifdef VALIDATE_PARMS
#endif

    reg_p2p_ae_sci_en = RU_FIELD_SET(0, XIF, P2P_AE_SCI_EN, CFGP2PSCIEN, reg_p2p_ae_sci_en, cfgp2pscien);

    RU_REG_WRITE(0, XIF, P2P_AE_SCI_EN, reg_p2p_ae_sci_en);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xif_p2p_ae_sci_en_get(uint16_t *cfgp2pscien)
{
    uint32_t reg_p2p_ae_sci_en=0;

#ifdef VALIDATE_PARMS
    if(!cfgp2pscien)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, XIF, P2P_AE_SCI_EN, reg_p2p_ae_sci_en);

    *cfgp2pscien = RU_FIELD_GET(0, XIF, P2P_AE_SCI_EN, CFGP2PSCIEN, reg_p2p_ae_sci_en);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xif_p2p_ae_sci_lo_0_set(uint32_t cfgp2psci_lo_0)
{
    uint32_t reg_p2p_ae_sci_lo_0=0;

#ifdef VALIDATE_PARMS
#endif

    reg_p2p_ae_sci_lo_0 = RU_FIELD_SET(0, XIF, P2P_AE_SCI_LO_0, CFGP2PSCI_LO_0, reg_p2p_ae_sci_lo_0, cfgp2psci_lo_0);

    RU_REG_WRITE(0, XIF, P2P_AE_SCI_LO_0, reg_p2p_ae_sci_lo_0);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xif_p2p_ae_sci_lo_0_get(uint32_t *cfgp2psci_lo_0)
{
    uint32_t reg_p2p_ae_sci_lo_0=0;

#ifdef VALIDATE_PARMS
    if(!cfgp2psci_lo_0)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, XIF, P2P_AE_SCI_LO_0, reg_p2p_ae_sci_lo_0);

    *cfgp2psci_lo_0 = RU_FIELD_GET(0, XIF, P2P_AE_SCI_LO_0, CFGP2PSCI_LO_0, reg_p2p_ae_sci_lo_0);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xif_p2p_ae_sci_hi_0_set(uint32_t cfgp2psci_hi_0)
{
    uint32_t reg_p2p_ae_sci_hi_0=0;

#ifdef VALIDATE_PARMS
#endif

    reg_p2p_ae_sci_hi_0 = RU_FIELD_SET(0, XIF, P2P_AE_SCI_HI_0, CFGP2PSCI_HI_0, reg_p2p_ae_sci_hi_0, cfgp2psci_hi_0);

    RU_REG_WRITE(0, XIF, P2P_AE_SCI_HI_0, reg_p2p_ae_sci_hi_0);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xif_p2p_ae_sci_hi_0_get(uint32_t *cfgp2psci_hi_0)
{
    uint32_t reg_p2p_ae_sci_hi_0=0;

#ifdef VALIDATE_PARMS
    if(!cfgp2psci_hi_0)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, XIF, P2P_AE_SCI_HI_0, reg_p2p_ae_sci_hi_0);

    *cfgp2psci_hi_0 = RU_FIELD_GET(0, XIF, P2P_AE_SCI_HI_0, CFGP2PSCI_HI_0, reg_p2p_ae_sci_hi_0);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xif_p2p_ae_sci_lo_1_set(uint32_t cfgp2psci_lo_1)
{
    uint32_t reg_p2p_ae_sci_lo_1=0;

#ifdef VALIDATE_PARMS
#endif

    reg_p2p_ae_sci_lo_1 = RU_FIELD_SET(0, XIF, P2P_AE_SCI_LO_1, CFGP2PSCI_LO_1, reg_p2p_ae_sci_lo_1, cfgp2psci_lo_1);

    RU_REG_WRITE(0, XIF, P2P_AE_SCI_LO_1, reg_p2p_ae_sci_lo_1);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xif_p2p_ae_sci_lo_1_get(uint32_t *cfgp2psci_lo_1)
{
    uint32_t reg_p2p_ae_sci_lo_1=0;

#ifdef VALIDATE_PARMS
    if(!cfgp2psci_lo_1)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, XIF, P2P_AE_SCI_LO_1, reg_p2p_ae_sci_lo_1);

    *cfgp2psci_lo_1 = RU_FIELD_GET(0, XIF, P2P_AE_SCI_LO_1, CFGP2PSCI_LO_1, reg_p2p_ae_sci_lo_1);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xif_p2p_ae_sci_hi_1_set(uint32_t cfgp2psci_hi_1)
{
    uint32_t reg_p2p_ae_sci_hi_1=0;

#ifdef VALIDATE_PARMS
#endif

    reg_p2p_ae_sci_hi_1 = RU_FIELD_SET(0, XIF, P2P_AE_SCI_HI_1, CFGP2PSCI_HI_1, reg_p2p_ae_sci_hi_1, cfgp2psci_hi_1);

    RU_REG_WRITE(0, XIF, P2P_AE_SCI_HI_1, reg_p2p_ae_sci_hi_1);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xif_p2p_ae_sci_hi_1_get(uint32_t *cfgp2psci_hi_1)
{
    uint32_t reg_p2p_ae_sci_hi_1=0;

#ifdef VALIDATE_PARMS
    if(!cfgp2psci_hi_1)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, XIF, P2P_AE_SCI_HI_1, reg_p2p_ae_sci_hi_1);

    *cfgp2psci_hi_1 = RU_FIELD_GET(0, XIF, P2P_AE_SCI_HI_1, CFGP2PSCI_HI_1, reg_p2p_ae_sci_hi_1);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xif_p2p_ae_sci_lo_2_set(uint32_t cfgp2psci_lo_2)
{
    uint32_t reg_p2p_ae_sci_lo_2=0;

#ifdef VALIDATE_PARMS
#endif

    reg_p2p_ae_sci_lo_2 = RU_FIELD_SET(0, XIF, P2P_AE_SCI_LO_2, CFGP2PSCI_LO_2, reg_p2p_ae_sci_lo_2, cfgp2psci_lo_2);

    RU_REG_WRITE(0, XIF, P2P_AE_SCI_LO_2, reg_p2p_ae_sci_lo_2);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xif_p2p_ae_sci_lo_2_get(uint32_t *cfgp2psci_lo_2)
{
    uint32_t reg_p2p_ae_sci_lo_2=0;

#ifdef VALIDATE_PARMS
    if(!cfgp2psci_lo_2)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, XIF, P2P_AE_SCI_LO_2, reg_p2p_ae_sci_lo_2);

    *cfgp2psci_lo_2 = RU_FIELD_GET(0, XIF, P2P_AE_SCI_LO_2, CFGP2PSCI_LO_2, reg_p2p_ae_sci_lo_2);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xif_p2p_ae_sci_hi_2_set(uint32_t cfgp2psci_hi_2)
{
    uint32_t reg_p2p_ae_sci_hi_2=0;

#ifdef VALIDATE_PARMS
#endif

    reg_p2p_ae_sci_hi_2 = RU_FIELD_SET(0, XIF, P2P_AE_SCI_HI_2, CFGP2PSCI_HI_2, reg_p2p_ae_sci_hi_2, cfgp2psci_hi_2);

    RU_REG_WRITE(0, XIF, P2P_AE_SCI_HI_2, reg_p2p_ae_sci_hi_2);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xif_p2p_ae_sci_hi_2_get(uint32_t *cfgp2psci_hi_2)
{
    uint32_t reg_p2p_ae_sci_hi_2=0;

#ifdef VALIDATE_PARMS
    if(!cfgp2psci_hi_2)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, XIF, P2P_AE_SCI_HI_2, reg_p2p_ae_sci_hi_2);

    *cfgp2psci_hi_2 = RU_FIELD_GET(0, XIF, P2P_AE_SCI_HI_2, CFGP2PSCI_HI_2, reg_p2p_ae_sci_hi_2);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xif_p2p_ae_sci_lo_3_set(uint32_t cfgp2psci_lo_3)
{
    uint32_t reg_p2p_ae_sci_lo_3=0;

#ifdef VALIDATE_PARMS
#endif

    reg_p2p_ae_sci_lo_3 = RU_FIELD_SET(0, XIF, P2P_AE_SCI_LO_3, CFGP2PSCI_LO_3, reg_p2p_ae_sci_lo_3, cfgp2psci_lo_3);

    RU_REG_WRITE(0, XIF, P2P_AE_SCI_LO_3, reg_p2p_ae_sci_lo_3);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xif_p2p_ae_sci_lo_3_get(uint32_t *cfgp2psci_lo_3)
{
    uint32_t reg_p2p_ae_sci_lo_3=0;

#ifdef VALIDATE_PARMS
    if(!cfgp2psci_lo_3)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, XIF, P2P_AE_SCI_LO_3, reg_p2p_ae_sci_lo_3);

    *cfgp2psci_lo_3 = RU_FIELD_GET(0, XIF, P2P_AE_SCI_LO_3, CFGP2PSCI_LO_3, reg_p2p_ae_sci_lo_3);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xif_p2p_ae_sci_hi_3_set(uint32_t cfgp2psci_hi_3)
{
    uint32_t reg_p2p_ae_sci_hi_3=0;

#ifdef VALIDATE_PARMS
#endif

    reg_p2p_ae_sci_hi_3 = RU_FIELD_SET(0, XIF, P2P_AE_SCI_HI_3, CFGP2PSCI_HI_3, reg_p2p_ae_sci_hi_3, cfgp2psci_hi_3);

    RU_REG_WRITE(0, XIF, P2P_AE_SCI_HI_3, reg_p2p_ae_sci_hi_3);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xif_p2p_ae_sci_hi_3_get(uint32_t *cfgp2psci_hi_3)
{
    uint32_t reg_p2p_ae_sci_hi_3=0;

#ifdef VALIDATE_PARMS
    if(!cfgp2psci_hi_3)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, XIF, P2P_AE_SCI_HI_3, reg_p2p_ae_sci_hi_3);

    *cfgp2psci_hi_3 = RU_FIELD_GET(0, XIF, P2P_AE_SCI_HI_3, CFGP2PSCI_HI_3, reg_p2p_ae_sci_hi_3);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xif_p2p_ae_sci_lo_4_set(uint32_t cfgp2psci_lo_4)
{
    uint32_t reg_p2p_ae_sci_lo_4=0;

#ifdef VALIDATE_PARMS
#endif

    reg_p2p_ae_sci_lo_4 = RU_FIELD_SET(0, XIF, P2P_AE_SCI_LO_4, CFGP2PSCI_LO_4, reg_p2p_ae_sci_lo_4, cfgp2psci_lo_4);

    RU_REG_WRITE(0, XIF, P2P_AE_SCI_LO_4, reg_p2p_ae_sci_lo_4);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xif_p2p_ae_sci_lo_4_get(uint32_t *cfgp2psci_lo_4)
{
    uint32_t reg_p2p_ae_sci_lo_4=0;

#ifdef VALIDATE_PARMS
    if(!cfgp2psci_lo_4)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, XIF, P2P_AE_SCI_LO_4, reg_p2p_ae_sci_lo_4);

    *cfgp2psci_lo_4 = RU_FIELD_GET(0, XIF, P2P_AE_SCI_LO_4, CFGP2PSCI_LO_4, reg_p2p_ae_sci_lo_4);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xif_p2p_ae_sci_hi_4_set(uint32_t cfgp2psci_hi_4)
{
    uint32_t reg_p2p_ae_sci_hi_4=0;

#ifdef VALIDATE_PARMS
#endif

    reg_p2p_ae_sci_hi_4 = RU_FIELD_SET(0, XIF, P2P_AE_SCI_HI_4, CFGP2PSCI_HI_4, reg_p2p_ae_sci_hi_4, cfgp2psci_hi_4);

    RU_REG_WRITE(0, XIF, P2P_AE_SCI_HI_4, reg_p2p_ae_sci_hi_4);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xif_p2p_ae_sci_hi_4_get(uint32_t *cfgp2psci_hi_4)
{
    uint32_t reg_p2p_ae_sci_hi_4=0;

#ifdef VALIDATE_PARMS
    if(!cfgp2psci_hi_4)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, XIF, P2P_AE_SCI_HI_4, reg_p2p_ae_sci_hi_4);

    *cfgp2psci_hi_4 = RU_FIELD_GET(0, XIF, P2P_AE_SCI_HI_4, CFGP2PSCI_HI_4, reg_p2p_ae_sci_hi_4);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xif_p2p_ae_sci_lo_5_set(uint32_t cfgp2psci_lo_5)
{
    uint32_t reg_p2p_ae_sci_lo_5=0;

#ifdef VALIDATE_PARMS
#endif

    reg_p2p_ae_sci_lo_5 = RU_FIELD_SET(0, XIF, P2P_AE_SCI_LO_5, CFGP2PSCI_LO_5, reg_p2p_ae_sci_lo_5, cfgp2psci_lo_5);

    RU_REG_WRITE(0, XIF, P2P_AE_SCI_LO_5, reg_p2p_ae_sci_lo_5);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xif_p2p_ae_sci_lo_5_get(uint32_t *cfgp2psci_lo_5)
{
    uint32_t reg_p2p_ae_sci_lo_5=0;

#ifdef VALIDATE_PARMS
    if(!cfgp2psci_lo_5)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, XIF, P2P_AE_SCI_LO_5, reg_p2p_ae_sci_lo_5);

    *cfgp2psci_lo_5 = RU_FIELD_GET(0, XIF, P2P_AE_SCI_LO_5, CFGP2PSCI_LO_5, reg_p2p_ae_sci_lo_5);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xif_p2p_ae_sci_hi_5_set(uint32_t cfgp2psci_hi_5)
{
    uint32_t reg_p2p_ae_sci_hi_5=0;

#ifdef VALIDATE_PARMS
#endif

    reg_p2p_ae_sci_hi_5 = RU_FIELD_SET(0, XIF, P2P_AE_SCI_HI_5, CFGP2PSCI_HI_5, reg_p2p_ae_sci_hi_5, cfgp2psci_hi_5);

    RU_REG_WRITE(0, XIF, P2P_AE_SCI_HI_5, reg_p2p_ae_sci_hi_5);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xif_p2p_ae_sci_hi_5_get(uint32_t *cfgp2psci_hi_5)
{
    uint32_t reg_p2p_ae_sci_hi_5=0;

#ifdef VALIDATE_PARMS
    if(!cfgp2psci_hi_5)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, XIF, P2P_AE_SCI_HI_5, reg_p2p_ae_sci_hi_5);

    *cfgp2psci_hi_5 = RU_FIELD_GET(0, XIF, P2P_AE_SCI_HI_5, CFGP2PSCI_HI_5, reg_p2p_ae_sci_hi_5);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xif_p2p_ae_sci_lo_6_set(uint32_t cfgp2psci_lo_6)
{
    uint32_t reg_p2p_ae_sci_lo_6=0;

#ifdef VALIDATE_PARMS
#endif

    reg_p2p_ae_sci_lo_6 = RU_FIELD_SET(0, XIF, P2P_AE_SCI_LO_6, CFGP2PSCI_LO_6, reg_p2p_ae_sci_lo_6, cfgp2psci_lo_6);

    RU_REG_WRITE(0, XIF, P2P_AE_SCI_LO_6, reg_p2p_ae_sci_lo_6);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xif_p2p_ae_sci_lo_6_get(uint32_t *cfgp2psci_lo_6)
{
    uint32_t reg_p2p_ae_sci_lo_6=0;

#ifdef VALIDATE_PARMS
    if(!cfgp2psci_lo_6)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, XIF, P2P_AE_SCI_LO_6, reg_p2p_ae_sci_lo_6);

    *cfgp2psci_lo_6 = RU_FIELD_GET(0, XIF, P2P_AE_SCI_LO_6, CFGP2PSCI_LO_6, reg_p2p_ae_sci_lo_6);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xif_p2p_ae_sci_hi_6_set(uint32_t cfgp2psci_hi_6)
{
    uint32_t reg_p2p_ae_sci_hi_6=0;

#ifdef VALIDATE_PARMS
#endif

    reg_p2p_ae_sci_hi_6 = RU_FIELD_SET(0, XIF, P2P_AE_SCI_HI_6, CFGP2PSCI_HI_6, reg_p2p_ae_sci_hi_6, cfgp2psci_hi_6);

    RU_REG_WRITE(0, XIF, P2P_AE_SCI_HI_6, reg_p2p_ae_sci_hi_6);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xif_p2p_ae_sci_hi_6_get(uint32_t *cfgp2psci_hi_6)
{
    uint32_t reg_p2p_ae_sci_hi_6=0;

#ifdef VALIDATE_PARMS
    if(!cfgp2psci_hi_6)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, XIF, P2P_AE_SCI_HI_6, reg_p2p_ae_sci_hi_6);

    *cfgp2psci_hi_6 = RU_FIELD_GET(0, XIF, P2P_AE_SCI_HI_6, CFGP2PSCI_HI_6, reg_p2p_ae_sci_hi_6);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xif_p2p_ae_sci_lo_7_set(uint32_t cfgp2psci_lo_7)
{
    uint32_t reg_p2p_ae_sci_lo_7=0;

#ifdef VALIDATE_PARMS
#endif

    reg_p2p_ae_sci_lo_7 = RU_FIELD_SET(0, XIF, P2P_AE_SCI_LO_7, CFGP2PSCI_LO_7, reg_p2p_ae_sci_lo_7, cfgp2psci_lo_7);

    RU_REG_WRITE(0, XIF, P2P_AE_SCI_LO_7, reg_p2p_ae_sci_lo_7);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xif_p2p_ae_sci_lo_7_get(uint32_t *cfgp2psci_lo_7)
{
    uint32_t reg_p2p_ae_sci_lo_7=0;

#ifdef VALIDATE_PARMS
    if(!cfgp2psci_lo_7)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, XIF, P2P_AE_SCI_LO_7, reg_p2p_ae_sci_lo_7);

    *cfgp2psci_lo_7 = RU_FIELD_GET(0, XIF, P2P_AE_SCI_LO_7, CFGP2PSCI_LO_7, reg_p2p_ae_sci_lo_7);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xif_p2p_ae_sci_hi_7_set(uint32_t cfgp2psci_hi_7)
{
    uint32_t reg_p2p_ae_sci_hi_7=0;

#ifdef VALIDATE_PARMS
#endif

    reg_p2p_ae_sci_hi_7 = RU_FIELD_SET(0, XIF, P2P_AE_SCI_HI_7, CFGP2PSCI_HI_7, reg_p2p_ae_sci_hi_7, cfgp2psci_hi_7);

    RU_REG_WRITE(0, XIF, P2P_AE_SCI_HI_7, reg_p2p_ae_sci_hi_7);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xif_p2p_ae_sci_hi_7_get(uint32_t *cfgp2psci_hi_7)
{
    uint32_t reg_p2p_ae_sci_hi_7=0;

#ifdef VALIDATE_PARMS
    if(!cfgp2psci_hi_7)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, XIF, P2P_AE_SCI_HI_7, reg_p2p_ae_sci_hi_7);

    *cfgp2psci_hi_7 = RU_FIELD_GET(0, XIF, P2P_AE_SCI_HI_7, CFGP2PSCI_HI_7, reg_p2p_ae_sci_hi_7);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xif_p2p_ae_sci_lo_8_set(uint32_t cfgp2psci_lo_8)
{
    uint32_t reg_p2p_ae_sci_lo_8=0;

#ifdef VALIDATE_PARMS
#endif

    reg_p2p_ae_sci_lo_8 = RU_FIELD_SET(0, XIF, P2P_AE_SCI_LO_8, CFGP2PSCI_LO_8, reg_p2p_ae_sci_lo_8, cfgp2psci_lo_8);

    RU_REG_WRITE(0, XIF, P2P_AE_SCI_LO_8, reg_p2p_ae_sci_lo_8);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xif_p2p_ae_sci_lo_8_get(uint32_t *cfgp2psci_lo_8)
{
    uint32_t reg_p2p_ae_sci_lo_8=0;

#ifdef VALIDATE_PARMS
    if(!cfgp2psci_lo_8)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, XIF, P2P_AE_SCI_LO_8, reg_p2p_ae_sci_lo_8);

    *cfgp2psci_lo_8 = RU_FIELD_GET(0, XIF, P2P_AE_SCI_LO_8, CFGP2PSCI_LO_8, reg_p2p_ae_sci_lo_8);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xif_p2p_ae_sci_hi_8_set(uint32_t cfgp2psci_hi_8)
{
    uint32_t reg_p2p_ae_sci_hi_8=0;

#ifdef VALIDATE_PARMS
#endif

    reg_p2p_ae_sci_hi_8 = RU_FIELD_SET(0, XIF, P2P_AE_SCI_HI_8, CFGP2PSCI_HI_8, reg_p2p_ae_sci_hi_8, cfgp2psci_hi_8);

    RU_REG_WRITE(0, XIF, P2P_AE_SCI_HI_8, reg_p2p_ae_sci_hi_8);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xif_p2p_ae_sci_hi_8_get(uint32_t *cfgp2psci_hi_8)
{
    uint32_t reg_p2p_ae_sci_hi_8=0;

#ifdef VALIDATE_PARMS
    if(!cfgp2psci_hi_8)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, XIF, P2P_AE_SCI_HI_8, reg_p2p_ae_sci_hi_8);

    *cfgp2psci_hi_8 = RU_FIELD_GET(0, XIF, P2P_AE_SCI_HI_8, CFGP2PSCI_HI_8, reg_p2p_ae_sci_hi_8);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xif_p2p_ae_sci_lo_9_set(uint32_t cfgp2psci_lo_9)
{
    uint32_t reg_p2p_ae_sci_lo_9=0;

#ifdef VALIDATE_PARMS
#endif

    reg_p2p_ae_sci_lo_9 = RU_FIELD_SET(0, XIF, P2P_AE_SCI_LO_9, CFGP2PSCI_LO_9, reg_p2p_ae_sci_lo_9, cfgp2psci_lo_9);

    RU_REG_WRITE(0, XIF, P2P_AE_SCI_LO_9, reg_p2p_ae_sci_lo_9);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xif_p2p_ae_sci_lo_9_get(uint32_t *cfgp2psci_lo_9)
{
    uint32_t reg_p2p_ae_sci_lo_9=0;

#ifdef VALIDATE_PARMS
    if(!cfgp2psci_lo_9)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, XIF, P2P_AE_SCI_LO_9, reg_p2p_ae_sci_lo_9);

    *cfgp2psci_lo_9 = RU_FIELD_GET(0, XIF, P2P_AE_SCI_LO_9, CFGP2PSCI_LO_9, reg_p2p_ae_sci_lo_9);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xif_p2p_ae_sci_hi_9_set(uint32_t cfgp2psci_hi_9)
{
    uint32_t reg_p2p_ae_sci_hi_9=0;

#ifdef VALIDATE_PARMS
#endif

    reg_p2p_ae_sci_hi_9 = RU_FIELD_SET(0, XIF, P2P_AE_SCI_HI_9, CFGP2PSCI_HI_9, reg_p2p_ae_sci_hi_9, cfgp2psci_hi_9);

    RU_REG_WRITE(0, XIF, P2P_AE_SCI_HI_9, reg_p2p_ae_sci_hi_9);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xif_p2p_ae_sci_hi_9_get(uint32_t *cfgp2psci_hi_9)
{
    uint32_t reg_p2p_ae_sci_hi_9=0;

#ifdef VALIDATE_PARMS
    if(!cfgp2psci_hi_9)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, XIF, P2P_AE_SCI_HI_9, reg_p2p_ae_sci_hi_9);

    *cfgp2psci_hi_9 = RU_FIELD_GET(0, XIF, P2P_AE_SCI_HI_9, CFGP2PSCI_HI_9, reg_p2p_ae_sci_hi_9);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xif_p2p_ae_sci_lo_10_set(uint32_t cfgp2psci_lo_10)
{
    uint32_t reg_p2p_ae_sci_lo_10=0;

#ifdef VALIDATE_PARMS
#endif

    reg_p2p_ae_sci_lo_10 = RU_FIELD_SET(0, XIF, P2P_AE_SCI_LO_10, CFGP2PSCI_LO_10, reg_p2p_ae_sci_lo_10, cfgp2psci_lo_10);

    RU_REG_WRITE(0, XIF, P2P_AE_SCI_LO_10, reg_p2p_ae_sci_lo_10);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xif_p2p_ae_sci_lo_10_get(uint32_t *cfgp2psci_lo_10)
{
    uint32_t reg_p2p_ae_sci_lo_10=0;

#ifdef VALIDATE_PARMS
    if(!cfgp2psci_lo_10)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, XIF, P2P_AE_SCI_LO_10, reg_p2p_ae_sci_lo_10);

    *cfgp2psci_lo_10 = RU_FIELD_GET(0, XIF, P2P_AE_SCI_LO_10, CFGP2PSCI_LO_10, reg_p2p_ae_sci_lo_10);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xif_p2p_ae_sci_hi_10_set(uint32_t cfgp2psci_hi_10)
{
    uint32_t reg_p2p_ae_sci_hi_10=0;

#ifdef VALIDATE_PARMS
#endif

    reg_p2p_ae_sci_hi_10 = RU_FIELD_SET(0, XIF, P2P_AE_SCI_HI_10, CFGP2PSCI_HI_10, reg_p2p_ae_sci_hi_10, cfgp2psci_hi_10);

    RU_REG_WRITE(0, XIF, P2P_AE_SCI_HI_10, reg_p2p_ae_sci_hi_10);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xif_p2p_ae_sci_hi_10_get(uint32_t *cfgp2psci_hi_10)
{
    uint32_t reg_p2p_ae_sci_hi_10=0;

#ifdef VALIDATE_PARMS
    if(!cfgp2psci_hi_10)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, XIF, P2P_AE_SCI_HI_10, reg_p2p_ae_sci_hi_10);

    *cfgp2psci_hi_10 = RU_FIELD_GET(0, XIF, P2P_AE_SCI_HI_10, CFGP2PSCI_HI_10, reg_p2p_ae_sci_hi_10);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xif_p2p_ae_sci_lo_11_set(uint32_t cfgp2psci_lo_11)
{
    uint32_t reg_p2p_ae_sci_lo_11=0;

#ifdef VALIDATE_PARMS
#endif

    reg_p2p_ae_sci_lo_11 = RU_FIELD_SET(0, XIF, P2P_AE_SCI_LO_11, CFGP2PSCI_LO_11, reg_p2p_ae_sci_lo_11, cfgp2psci_lo_11);

    RU_REG_WRITE(0, XIF, P2P_AE_SCI_LO_11, reg_p2p_ae_sci_lo_11);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xif_p2p_ae_sci_lo_11_get(uint32_t *cfgp2psci_lo_11)
{
    uint32_t reg_p2p_ae_sci_lo_11=0;

#ifdef VALIDATE_PARMS
    if(!cfgp2psci_lo_11)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, XIF, P2P_AE_SCI_LO_11, reg_p2p_ae_sci_lo_11);

    *cfgp2psci_lo_11 = RU_FIELD_GET(0, XIF, P2P_AE_SCI_LO_11, CFGP2PSCI_LO_11, reg_p2p_ae_sci_lo_11);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xif_p2p_ae_sci_hi_11_set(uint32_t cfgp2psci_hi_11)
{
    uint32_t reg_p2p_ae_sci_hi_11=0;

#ifdef VALIDATE_PARMS
#endif

    reg_p2p_ae_sci_hi_11 = RU_FIELD_SET(0, XIF, P2P_AE_SCI_HI_11, CFGP2PSCI_HI_11, reg_p2p_ae_sci_hi_11, cfgp2psci_hi_11);

    RU_REG_WRITE(0, XIF, P2P_AE_SCI_HI_11, reg_p2p_ae_sci_hi_11);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xif_p2p_ae_sci_hi_11_get(uint32_t *cfgp2psci_hi_11)
{
    uint32_t reg_p2p_ae_sci_hi_11=0;

#ifdef VALIDATE_PARMS
    if(!cfgp2psci_hi_11)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, XIF, P2P_AE_SCI_HI_11, reg_p2p_ae_sci_hi_11);

    *cfgp2psci_hi_11 = RU_FIELD_GET(0, XIF, P2P_AE_SCI_HI_11, CFGP2PSCI_HI_11, reg_p2p_ae_sci_hi_11);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xif_p2p_ae_sci_lo_12_set(uint32_t cfgp2psci_lo_12)
{
    uint32_t reg_p2p_ae_sci_lo_12=0;

#ifdef VALIDATE_PARMS
#endif

    reg_p2p_ae_sci_lo_12 = RU_FIELD_SET(0, XIF, P2P_AE_SCI_LO_12, CFGP2PSCI_LO_12, reg_p2p_ae_sci_lo_12, cfgp2psci_lo_12);

    RU_REG_WRITE(0, XIF, P2P_AE_SCI_LO_12, reg_p2p_ae_sci_lo_12);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xif_p2p_ae_sci_lo_12_get(uint32_t *cfgp2psci_lo_12)
{
    uint32_t reg_p2p_ae_sci_lo_12=0;

#ifdef VALIDATE_PARMS
    if(!cfgp2psci_lo_12)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, XIF, P2P_AE_SCI_LO_12, reg_p2p_ae_sci_lo_12);

    *cfgp2psci_lo_12 = RU_FIELD_GET(0, XIF, P2P_AE_SCI_LO_12, CFGP2PSCI_LO_12, reg_p2p_ae_sci_lo_12);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xif_p2p_ae_sci_hi_12_set(uint32_t cfgp2psci_hi_12)
{
    uint32_t reg_p2p_ae_sci_hi_12=0;

#ifdef VALIDATE_PARMS
#endif

    reg_p2p_ae_sci_hi_12 = RU_FIELD_SET(0, XIF, P2P_AE_SCI_HI_12, CFGP2PSCI_HI_12, reg_p2p_ae_sci_hi_12, cfgp2psci_hi_12);

    RU_REG_WRITE(0, XIF, P2P_AE_SCI_HI_12, reg_p2p_ae_sci_hi_12);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xif_p2p_ae_sci_hi_12_get(uint32_t *cfgp2psci_hi_12)
{
    uint32_t reg_p2p_ae_sci_hi_12=0;

#ifdef VALIDATE_PARMS
    if(!cfgp2psci_hi_12)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, XIF, P2P_AE_SCI_HI_12, reg_p2p_ae_sci_hi_12);

    *cfgp2psci_hi_12 = RU_FIELD_GET(0, XIF, P2P_AE_SCI_HI_12, CFGP2PSCI_HI_12, reg_p2p_ae_sci_hi_12);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xif_p2p_ae_sci_lo_13_set(uint32_t cfgp2psci_lo_13)
{
    uint32_t reg_p2p_ae_sci_lo_13=0;

#ifdef VALIDATE_PARMS
#endif

    reg_p2p_ae_sci_lo_13 = RU_FIELD_SET(0, XIF, P2P_AE_SCI_LO_13, CFGP2PSCI_LO_13, reg_p2p_ae_sci_lo_13, cfgp2psci_lo_13);

    RU_REG_WRITE(0, XIF, P2P_AE_SCI_LO_13, reg_p2p_ae_sci_lo_13);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xif_p2p_ae_sci_lo_13_get(uint32_t *cfgp2psci_lo_13)
{
    uint32_t reg_p2p_ae_sci_lo_13=0;

#ifdef VALIDATE_PARMS
    if(!cfgp2psci_lo_13)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, XIF, P2P_AE_SCI_LO_13, reg_p2p_ae_sci_lo_13);

    *cfgp2psci_lo_13 = RU_FIELD_GET(0, XIF, P2P_AE_SCI_LO_13, CFGP2PSCI_LO_13, reg_p2p_ae_sci_lo_13);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xif_p2p_ae_sci_hi_13_set(uint32_t cfgp2psci_hi_13)
{
    uint32_t reg_p2p_ae_sci_hi_13=0;

#ifdef VALIDATE_PARMS
#endif

    reg_p2p_ae_sci_hi_13 = RU_FIELD_SET(0, XIF, P2P_AE_SCI_HI_13, CFGP2PSCI_HI_13, reg_p2p_ae_sci_hi_13, cfgp2psci_hi_13);

    RU_REG_WRITE(0, XIF, P2P_AE_SCI_HI_13, reg_p2p_ae_sci_hi_13);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xif_p2p_ae_sci_hi_13_get(uint32_t *cfgp2psci_hi_13)
{
    uint32_t reg_p2p_ae_sci_hi_13=0;

#ifdef VALIDATE_PARMS
    if(!cfgp2psci_hi_13)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, XIF, P2P_AE_SCI_HI_13, reg_p2p_ae_sci_hi_13);

    *cfgp2psci_hi_13 = RU_FIELD_GET(0, XIF, P2P_AE_SCI_HI_13, CFGP2PSCI_HI_13, reg_p2p_ae_sci_hi_13);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xif_p2p_ae_sci_lo_14_set(uint32_t cfgp2psci_lo_14)
{
    uint32_t reg_p2p_ae_sci_lo_14=0;

#ifdef VALIDATE_PARMS
#endif

    reg_p2p_ae_sci_lo_14 = RU_FIELD_SET(0, XIF, P2P_AE_SCI_LO_14, CFGP2PSCI_LO_14, reg_p2p_ae_sci_lo_14, cfgp2psci_lo_14);

    RU_REG_WRITE(0, XIF, P2P_AE_SCI_LO_14, reg_p2p_ae_sci_lo_14);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xif_p2p_ae_sci_lo_14_get(uint32_t *cfgp2psci_lo_14)
{
    uint32_t reg_p2p_ae_sci_lo_14=0;

#ifdef VALIDATE_PARMS
    if(!cfgp2psci_lo_14)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, XIF, P2P_AE_SCI_LO_14, reg_p2p_ae_sci_lo_14);

    *cfgp2psci_lo_14 = RU_FIELD_GET(0, XIF, P2P_AE_SCI_LO_14, CFGP2PSCI_LO_14, reg_p2p_ae_sci_lo_14);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xif_p2p_ae_sci_hi_14_set(uint32_t cfgp2psci_hi_14)
{
    uint32_t reg_p2p_ae_sci_hi_14=0;

#ifdef VALIDATE_PARMS
#endif

    reg_p2p_ae_sci_hi_14 = RU_FIELD_SET(0, XIF, P2P_AE_SCI_HI_14, CFGP2PSCI_HI_14, reg_p2p_ae_sci_hi_14, cfgp2psci_hi_14);

    RU_REG_WRITE(0, XIF, P2P_AE_SCI_HI_14, reg_p2p_ae_sci_hi_14);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xif_p2p_ae_sci_hi_14_get(uint32_t *cfgp2psci_hi_14)
{
    uint32_t reg_p2p_ae_sci_hi_14=0;

#ifdef VALIDATE_PARMS
    if(!cfgp2psci_hi_14)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, XIF, P2P_AE_SCI_HI_14, reg_p2p_ae_sci_hi_14);

    *cfgp2psci_hi_14 = RU_FIELD_GET(0, XIF, P2P_AE_SCI_HI_14, CFGP2PSCI_HI_14, reg_p2p_ae_sci_hi_14);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xif_p2p_ae_sci_lo_15_set(uint32_t cfgp2psci_lo_15)
{
    uint32_t reg_p2p_ae_sci_lo_15=0;

#ifdef VALIDATE_PARMS
#endif

    reg_p2p_ae_sci_lo_15 = RU_FIELD_SET(0, XIF, P2P_AE_SCI_LO_15, CFGP2PSCI_LO_15, reg_p2p_ae_sci_lo_15, cfgp2psci_lo_15);

    RU_REG_WRITE(0, XIF, P2P_AE_SCI_LO_15, reg_p2p_ae_sci_lo_15);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xif_p2p_ae_sci_lo_15_get(uint32_t *cfgp2psci_lo_15)
{
    uint32_t reg_p2p_ae_sci_lo_15=0;

#ifdef VALIDATE_PARMS
    if(!cfgp2psci_lo_15)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, XIF, P2P_AE_SCI_LO_15, reg_p2p_ae_sci_lo_15);

    *cfgp2psci_lo_15 = RU_FIELD_GET(0, XIF, P2P_AE_SCI_LO_15, CFGP2PSCI_LO_15, reg_p2p_ae_sci_lo_15);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xif_p2p_ae_sci_hi_15_set(uint32_t cfgp2psci_hi_15)
{
    uint32_t reg_p2p_ae_sci_hi_15=0;

#ifdef VALIDATE_PARMS
#endif

    reg_p2p_ae_sci_hi_15 = RU_FIELD_SET(0, XIF, P2P_AE_SCI_HI_15, CFGP2PSCI_HI_15, reg_p2p_ae_sci_hi_15, cfgp2psci_hi_15);

    RU_REG_WRITE(0, XIF, P2P_AE_SCI_HI_15, reg_p2p_ae_sci_hi_15);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xif_p2p_ae_sci_hi_15_get(uint32_t *cfgp2psci_hi_15)
{
    uint32_t reg_p2p_ae_sci_hi_15=0;

#ifdef VALIDATE_PARMS
    if(!cfgp2psci_hi_15)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, XIF, P2P_AE_SCI_HI_15, reg_p2p_ae_sci_hi_15);

    *cfgp2psci_hi_15 = RU_FIELD_GET(0, XIF, P2P_AE_SCI_HI_15, CFGP2PSCI_HI_15, reg_p2p_ae_sci_hi_15);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xif_sectx_keynum_1_get(uint32_t *keystattx_hi)
{
    uint32_t reg_sectx_keynum_1=0;

#ifdef VALIDATE_PARMS
    if(!keystattx_hi)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, XIF, SECTX_KEYNUM_1, reg_sectx_keynum_1);

    *keystattx_hi = RU_FIELD_GET(0, XIF, SECTX_KEYNUM_1, KEYSTATTX_HI, reg_sectx_keynum_1);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xif_secrx_keynum_1_get(uint32_t *keystatrx_hi)
{
    uint32_t reg_secrx_keynum_1=0;

#ifdef VALIDATE_PARMS
    if(!keystatrx_hi)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, XIF, SECRX_KEYNUM_1, reg_secrx_keynum_1);

    *keystatrx_hi = RU_FIELD_GET(0, XIF, SECRX_KEYNUM_1, KEYSTATRX_HI, reg_secrx_keynum_1);

    return BDMF_ERR_OK;
}

#ifdef USE_BDMF_SHELL
enum
{
    BDMF_ctl,
    BDMF_int_status,
    BDMF_int_mask,
    BDMF_port_command,
    BDMF_port_data_,
    BDMF_macsec,
    BDMF_xpn_xmt_offset,
    BDMF_xpn_timestamp_offset,
    BDMF_xpn_pktgen_ctl,
    BDMF_xpn_pktgen_llid,
    BDMF_xpn_pktgen_pkt_cnt,
    BDMF_xpn_pktgen_pkt_size,
    BDMF_xpn_pktgen_ipg,
    BDMF_ts_jitter_thresh,
    BDMF_ts_update,
    BDMF_gnt_overhead,
    BDMF_discover_overhead,
    BDMF_discover_info,
    BDMF_xpn_oversize_thresh,
    BDMF_secrx_keynum,
    BDMF_secrx_encrypt,
    BDMF_pmc_frame_rx_cnt,
    BDMF_pmc_byte_rx_cnt,
    BDMF_pmc_runt_rx_cnt,
    BDMF_pmc_cw_err_rx_cnt,
    BDMF_pmc_crc8_err_rx_cnt,
    BDMF_xpn_data_frm_cnt,
    BDMF_xpn_data_byte_cnt,
    BDMF_xpn_mpcp_frm_cnt,
    BDMF_xpn_oam_frm_cnt,
    BDMF_xpn_oam_byte_cnt,
    BDMF_xpn_oversize_frm_cnt,
    BDMF_sec_abort_frm_cnt,
    BDMF_pmc_tx_neg_event_cnt,
    BDMF_xpn_idle_pkt_cnt,
    BDMF_llid_0,
    BDMF_llid_1,
    BDMF_llid_2,
    BDMF_llid_3,
    BDMF_llid_4,
    BDMF_llid_5,
    BDMF_llid_6,
    BDMF_llid_7,
    BDMF_llid_8,
    BDMF_llid_9,
    BDMF_llid_10,
    BDMF_llid_11,
    BDMF_llid_12,
    BDMF_llid_13,
    BDMF_llid_14,
    BDMF_llid_15,
    BDMF_llid_16,
    BDMF_llid_17,
    BDMF_llid_18,
    BDMF_llid_19,
    BDMF_llid_20,
    BDMF_llid_21,
    BDMF_llid_22,
    BDMF_llid_23,
    BDMF_llid_24,
    BDMF_llid_25,
    BDMF_llid_26,
    BDMF_llid_27,
    BDMF_llid_28,
    BDMF_llid_29,
    BDMF_llid_30,
    BDMF_llid_31,
    BDMF_max_mpcp_update,
    BDMF_ipg_insertion,
    BDMF_transport_time,
    BDMF_mpcp_time,
    BDMF_overlap_gnt_oh,
    BDMF_mac_mode,
    BDMF_pmctx_ctl,
    BDMF_sec_ctl,
    BDMF_ae_pktnum_window,
    BDMF_ae_pktnum_thresh,
    BDMF_sectx_keynum,
    BDMF_sectx_encrypt,
    BDMF_ae_pktnum_stat,
    BDMF_mpcp_update,
    BDMF_burst_prelaunch_offset,
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
    BDMF_sectx_keynum_1,
    BDMF_secrx_keynum_1,
};

typedef enum
{
    bdmf_address_ctl,
    bdmf_address_int_status,
    bdmf_address_int_mask,
    bdmf_address_port_command,
    bdmf_address_port_data_,
    bdmf_address_macsec,
    bdmf_address_xpn_xmt_offset,
    bdmf_address_xpn_timestamp_offset,
    bdmf_address_xpn_pktgen_ctl,
    bdmf_address_xpn_pktgen_llid,
    bdmf_address_xpn_pktgen_pkt_cnt,
    bdmf_address_xpn_pktgen_pkt_size,
    bdmf_address_xpn_pktgen_ipg,
    bdmf_address_ts_jitter_thresh,
    bdmf_address_ts_update,
    bdmf_address_gnt_overhead,
    bdmf_address_discover_overhead,
    bdmf_address_discover_info,
    bdmf_address_xpn_oversize_thresh,
    bdmf_address_secrx_keynum,
    bdmf_address_secrx_encrypt,
    bdmf_address_pmc_frame_rx_cnt,
    bdmf_address_pmc_byte_rx_cnt,
    bdmf_address_pmc_runt_rx_cnt,
    bdmf_address_pmc_cw_err_rx_cnt,
    bdmf_address_pmc_crc8_err_rx_cnt,
    bdmf_address_xpn_data_frm_cnt,
    bdmf_address_xpn_data_byte_cnt,
    bdmf_address_xpn_mpcp_frm_cnt,
    bdmf_address_xpn_oam_frm_cnt,
    bdmf_address_xpn_oam_byte_cnt,
    bdmf_address_xpn_oversize_frm_cnt,
    bdmf_address_sec_abort_frm_cnt,
    bdmf_address_pmc_tx_neg_event_cnt,
    bdmf_address_xpn_idle_pkt_cnt,
    bdmf_address_llid_0,
    bdmf_address_llid_1,
    bdmf_address_llid_2,
    bdmf_address_llid_3,
    bdmf_address_llid_4,
    bdmf_address_llid_5,
    bdmf_address_llid_6,
    bdmf_address_llid_7,
    bdmf_address_llid_8,
    bdmf_address_llid_9,
    bdmf_address_llid_10,
    bdmf_address_llid_11,
    bdmf_address_llid_12,
    bdmf_address_llid_13,
    bdmf_address_llid_14,
    bdmf_address_llid_15,
    bdmf_address_llid_16,
    bdmf_address_llid_17,
    bdmf_address_llid_18,
    bdmf_address_llid_19,
    bdmf_address_llid_20,
    bdmf_address_llid_21,
    bdmf_address_llid_22,
    bdmf_address_llid_23,
    bdmf_address_llid_24,
    bdmf_address_llid_25,
    bdmf_address_llid_26,
    bdmf_address_llid_27,
    bdmf_address_llid_28,
    bdmf_address_llid_29,
    bdmf_address_llid_30,
    bdmf_address_llid_31,
    bdmf_address_max_mpcp_update,
    bdmf_address_ipg_insertion,
    bdmf_address_transport_time,
    bdmf_address_mpcp_time,
    bdmf_address_overlap_gnt_oh,
    bdmf_address_mac_mode,
    bdmf_address_pmctx_ctl,
    bdmf_address_sec_ctl,
    bdmf_address_ae_pktnum_window,
    bdmf_address_ae_pktnum_thresh,
    bdmf_address_sectx_keynum,
    bdmf_address_sectx_encrypt,
    bdmf_address_ae_pktnum_stat,
    bdmf_address_mpcp_update,
    bdmf_address_burst_prelaunch_offset,
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
    bdmf_address_sectx_keynum_1,
    bdmf_address_secrx_keynum_1,
}
bdmf_address;

static int bcm_xif_cli_set(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    bdmf_error_t err;

    switch(parm[0].value.unumber)
    {
    case BDMF_ctl:
    {
        xif_ctl ctl = { .rxencrypten=parm[1].value.unumber, .cfgdisrxdasaencrpt=parm[2].value.unumber, .rxencryptmode=parm[3].value.unumber, .txencrypten=parm[4].value.unumber, .cfgdistxdasaencrpt=parm[5].value.unumber, .txencryptmode=parm[6].value.unumber, .cfgllidmodemsk=parm[7].value.unumber, .cfgxpnbadcrc32=parm[8].value.unumber, .cfgdisdiscinfo=parm[9].value.unumber, .cfgpmctx2rxlpbk=parm[10].value.unumber, .cfgpmctxencrc8bad=parm[11].value.unumber, .cfgenp2p=parm[12].value.unumber, .cfgllidpromiscuousmode=parm[13].value.unumber, .cfgenidlepktsup=parm[14].value.unumber, .cfgpmcrxencrc8chk=parm[15].value.unumber, .cfgen1stidlepktconvert=parm[16].value.unumber, .cfgfecen=parm[17].value.unumber, .cfglegacyrcvtsupd=parm[18].value.unumber, .cfgxpnencrcpassthru=parm[19].value.unumber, .cfgxpndistimestampmod=parm[20].value.unumber, .xifnotrdy=parm[21].value.unumber, .xifdtportrstn=parm[22].value.unumber, .xpntxrstn=parm[23].value.unumber, .pmctxrstn=parm[24].value.unumber, .sectxrstn=parm[25].value.unumber, .cfgdistxoamencrpt=parm[26].value.unumber, .cfgdistxmpcpencrpt=parm[27].value.unumber, .pmcrxrstn=parm[28].value.unumber, .secrxrstn=parm[29].value.unumber};
        err = ag_drv_xif_ctl_set(&ctl);
        break;
    }
    case BDMF_int_status:
    {
        xif_int_status int_status = { .secrxrplyprtctabrtint=parm[1].value.unumber, .sectxpktnummaxint=parm[2].value.unumber, .tsfullupdint=parm[3].value.unumber, .txhangint=parm[4].value.unumber, .negtimeint=parm[5].value.unumber, .pmctsjttrint=parm[6].value.unumber, .secrxoutffovrflwint=parm[7].value.unumber};
        err = ag_drv_xif_int_status_set(&int_status);
        break;
    }
    case BDMF_int_mask:
    {
        xif_int_mask int_mask = { .msksecrxreplayprotctabort=parm[1].value.unumber, .mskpktnumthreshint=parm[2].value.unumber, .msktsfullupdint=parm[3].value.unumber, .msktxhangint=parm[4].value.unumber, .msknegtimeint=parm[5].value.unumber, .mskpmctsjttrint=parm[6].value.unumber, .msksecrxoutffint=parm[7].value.unumber};
        err = ag_drv_xif_int_mask_set(&int_mask);
        break;
    }
    case BDMF_port_command:
        err = ag_drv_xif_port_command_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber, parm[4].value.unumber);
        break;
    case BDMF_port_data_:
        err = ag_drv_xif_port_data__set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case BDMF_macsec:
        err = ag_drv_xif_macsec_set(parm[1].value.unumber);
        break;
    case BDMF_xpn_xmt_offset:
        err = ag_drv_xif_xpn_xmt_offset_set(parm[1].value.unumber);
        break;
    case BDMF_xpn_timestamp_offset:
        err = ag_drv_xif_xpn_timestamp_offset_set(parm[1].value.unumber);
        break;
    case BDMF_xpn_pktgen_ctl:
    {
        xif_xpn_pktgen_ctl xpn_pktgen_ctl = { .cfgonuburstsize=parm[1].value.unumber, .cfgenbck2bckpktgen=parm[2].value.unumber, .cfgenallmpcppktgen=parm[3].value.unumber, .cfgxpnstartpktgen=parm[4].value.unumber, .cfgxpnenpktgen=parm[5].value.unumber};
        err = ag_drv_xif_xpn_pktgen_ctl_set(&xpn_pktgen_ctl);
        break;
    }
    case BDMF_xpn_pktgen_llid:
        err = ag_drv_xif_xpn_pktgen_llid_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case BDMF_xpn_pktgen_pkt_cnt:
        err = ag_drv_xif_xpn_pktgen_pkt_cnt_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case BDMF_xpn_pktgen_pkt_size:
        err = ag_drv_xif_xpn_pktgen_pkt_size_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber);
        break;
    case BDMF_xpn_pktgen_ipg:
        err = ag_drv_xif_xpn_pktgen_ipg_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case BDMF_ts_jitter_thresh:
        err = ag_drv_xif_ts_jitter_thresh_set(parm[1].value.unumber);
        break;
    case BDMF_ts_update:
        err = ag_drv_xif_ts_update_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber);
        break;
    case BDMF_gnt_overhead:
        err = ag_drv_xif_gnt_overhead_set(parm[1].value.unumber);
        break;
    case BDMF_discover_overhead:
        err = ag_drv_xif_discover_overhead_set(parm[1].value.unumber);
        break;
    case BDMF_discover_info:
        err = ag_drv_xif_discover_info_set(parm[1].value.unumber);
        break;
    case BDMF_xpn_oversize_thresh:
        err = ag_drv_xif_xpn_oversize_thresh_set(parm[1].value.unumber);
        break;
    case BDMF_llid_0:
        err = ag_drv_xif_llid_0_set(parm[1].value.unumber);
        break;
    case BDMF_llid_1:
        err = ag_drv_xif_llid_1_set(parm[1].value.unumber);
        break;
    case BDMF_llid_2:
        err = ag_drv_xif_llid_2_set(parm[1].value.unumber);
        break;
    case BDMF_llid_3:
        err = ag_drv_xif_llid_3_set(parm[1].value.unumber);
        break;
    case BDMF_llid_4:
        err = ag_drv_xif_llid_4_set(parm[1].value.unumber);
        break;
    case BDMF_llid_5:
        err = ag_drv_xif_llid_5_set(parm[1].value.unumber);
        break;
    case BDMF_llid_6:
        err = ag_drv_xif_llid_6_set(parm[1].value.unumber);
        break;
    case BDMF_llid_7:
        err = ag_drv_xif_llid_7_set(parm[1].value.unumber);
        break;
    case BDMF_llid_8:
        err = ag_drv_xif_llid_8_set(parm[1].value.unumber);
        break;
    case BDMF_llid_9:
        err = ag_drv_xif_llid_9_set(parm[1].value.unumber);
        break;
    case BDMF_llid_10:
        err = ag_drv_xif_llid_10_set(parm[1].value.unumber);
        break;
    case BDMF_llid_11:
        err = ag_drv_xif_llid_11_set(parm[1].value.unumber);
        break;
    case BDMF_llid_12:
        err = ag_drv_xif_llid_12_set(parm[1].value.unumber);
        break;
    case BDMF_llid_13:
        err = ag_drv_xif_llid_13_set(parm[1].value.unumber);
        break;
    case BDMF_llid_14:
        err = ag_drv_xif_llid_14_set(parm[1].value.unumber);
        break;
    case BDMF_llid_15:
        err = ag_drv_xif_llid_15_set(parm[1].value.unumber);
        break;
    case BDMF_llid_16:
        err = ag_drv_xif_llid_16_set(parm[1].value.unumber);
        break;
    case BDMF_llid_17:
        err = ag_drv_xif_llid_17_set(parm[1].value.unumber);
        break;
    case BDMF_llid_18:
        err = ag_drv_xif_llid_18_set(parm[1].value.unumber);
        break;
    case BDMF_llid_19:
        err = ag_drv_xif_llid_19_set(parm[1].value.unumber);
        break;
    case BDMF_llid_20:
        err = ag_drv_xif_llid_20_set(parm[1].value.unumber);
        break;
    case BDMF_llid_21:
        err = ag_drv_xif_llid_21_set(parm[1].value.unumber);
        break;
    case BDMF_llid_22:
        err = ag_drv_xif_llid_22_set(parm[1].value.unumber);
        break;
    case BDMF_llid_23:
        err = ag_drv_xif_llid_23_set(parm[1].value.unumber);
        break;
    case BDMF_llid_24:
        err = ag_drv_xif_llid_24_set(parm[1].value.unumber);
        break;
    case BDMF_llid_25:
        err = ag_drv_xif_llid_25_set(parm[1].value.unumber);
        break;
    case BDMF_llid_26:
        err = ag_drv_xif_llid_26_set(parm[1].value.unumber);
        break;
    case BDMF_llid_27:
        err = ag_drv_xif_llid_27_set(parm[1].value.unumber);
        break;
    case BDMF_llid_28:
        err = ag_drv_xif_llid_28_set(parm[1].value.unumber);
        break;
    case BDMF_llid_29:
        err = ag_drv_xif_llid_29_set(parm[1].value.unumber);
        break;
    case BDMF_llid_30:
        err = ag_drv_xif_llid_30_set(parm[1].value.unumber);
        break;
    case BDMF_llid_31:
        err = ag_drv_xif_llid_31_set(parm[1].value.unumber);
        break;
    case BDMF_max_mpcp_update:
        err = ag_drv_xif_max_mpcp_update_set(parm[1].value.unumber);
        break;
    case BDMF_ipg_insertion:
        err = ag_drv_xif_ipg_insertion_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber);
        break;
    case BDMF_transport_time:
        err = ag_drv_xif_transport_time_set(parm[1].value.unumber);
        break;
    case BDMF_overlap_gnt_oh:
        err = ag_drv_xif_overlap_gnt_oh_set(parm[1].value.unumber);
        break;
    case BDMF_mac_mode:
        err = ag_drv_xif_mac_mode_set(parm[1].value.unumber);
        break;
    case BDMF_pmctx_ctl:
    {
        xif_pmctx_ctl pmctx_ctl = { .cfgmpcpupdperiod=parm[1].value.unumber, .cfgdis4idleb4startchar=parm[2].value.unumber, .cfgenidledscrd=parm[3].value.unumber, .cfgseltxpontime=parm[4].value.unumber, .cfgmpcpcontupd=parm[5].value.unumber, .cfgenmaxmpcpupd=parm[6].value.unumber, .cfgennegtimeabort=parm[7].value.unumber};
        err = ag_drv_xif_pmctx_ctl_set(&pmctx_ctl);
        break;
    }
    case BDMF_sec_ctl:
        err = ag_drv_xif_sec_ctl_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber, parm[4].value.unumber);
        break;
    case BDMF_ae_pktnum_window:
        err = ag_drv_xif_ae_pktnum_window_set(parm[1].value.unumber);
        break;
    case BDMF_ae_pktnum_thresh:
        err = ag_drv_xif_ae_pktnum_thresh_set(parm[1].value.unumber);
        break;
    case BDMF_burst_prelaunch_offset:
        err = ag_drv_xif_burst_prelaunch_offset_set(parm[1].value.unumber);
        break;
    case BDMF_vlan_type:
        err = ag_drv_xif_vlan_type_set(parm[1].value.unumber);
        break;
    case BDMF_p2p_ae_sci_en:
        err = ag_drv_xif_p2p_ae_sci_en_set(parm[1].value.unumber);
        break;
    case BDMF_p2p_ae_sci_lo_0:
        err = ag_drv_xif_p2p_ae_sci_lo_0_set(parm[1].value.unumber);
        break;
    case BDMF_p2p_ae_sci_hi_0:
        err = ag_drv_xif_p2p_ae_sci_hi_0_set(parm[1].value.unumber);
        break;
    case BDMF_p2p_ae_sci_lo_1:
        err = ag_drv_xif_p2p_ae_sci_lo_1_set(parm[1].value.unumber);
        break;
    case BDMF_p2p_ae_sci_hi_1:
        err = ag_drv_xif_p2p_ae_sci_hi_1_set(parm[1].value.unumber);
        break;
    case BDMF_p2p_ae_sci_lo_2:
        err = ag_drv_xif_p2p_ae_sci_lo_2_set(parm[1].value.unumber);
        break;
    case BDMF_p2p_ae_sci_hi_2:
        err = ag_drv_xif_p2p_ae_sci_hi_2_set(parm[1].value.unumber);
        break;
    case BDMF_p2p_ae_sci_lo_3:
        err = ag_drv_xif_p2p_ae_sci_lo_3_set(parm[1].value.unumber);
        break;
    case BDMF_p2p_ae_sci_hi_3:
        err = ag_drv_xif_p2p_ae_sci_hi_3_set(parm[1].value.unumber);
        break;
    case BDMF_p2p_ae_sci_lo_4:
        err = ag_drv_xif_p2p_ae_sci_lo_4_set(parm[1].value.unumber);
        break;
    case BDMF_p2p_ae_sci_hi_4:
        err = ag_drv_xif_p2p_ae_sci_hi_4_set(parm[1].value.unumber);
        break;
    case BDMF_p2p_ae_sci_lo_5:
        err = ag_drv_xif_p2p_ae_sci_lo_5_set(parm[1].value.unumber);
        break;
    case BDMF_p2p_ae_sci_hi_5:
        err = ag_drv_xif_p2p_ae_sci_hi_5_set(parm[1].value.unumber);
        break;
    case BDMF_p2p_ae_sci_lo_6:
        err = ag_drv_xif_p2p_ae_sci_lo_6_set(parm[1].value.unumber);
        break;
    case BDMF_p2p_ae_sci_hi_6:
        err = ag_drv_xif_p2p_ae_sci_hi_6_set(parm[1].value.unumber);
        break;
    case BDMF_p2p_ae_sci_lo_7:
        err = ag_drv_xif_p2p_ae_sci_lo_7_set(parm[1].value.unumber);
        break;
    case BDMF_p2p_ae_sci_hi_7:
        err = ag_drv_xif_p2p_ae_sci_hi_7_set(parm[1].value.unumber);
        break;
    case BDMF_p2p_ae_sci_lo_8:
        err = ag_drv_xif_p2p_ae_sci_lo_8_set(parm[1].value.unumber);
        break;
    case BDMF_p2p_ae_sci_hi_8:
        err = ag_drv_xif_p2p_ae_sci_hi_8_set(parm[1].value.unumber);
        break;
    case BDMF_p2p_ae_sci_lo_9:
        err = ag_drv_xif_p2p_ae_sci_lo_9_set(parm[1].value.unumber);
        break;
    case BDMF_p2p_ae_sci_hi_9:
        err = ag_drv_xif_p2p_ae_sci_hi_9_set(parm[1].value.unumber);
        break;
    case BDMF_p2p_ae_sci_lo_10:
        err = ag_drv_xif_p2p_ae_sci_lo_10_set(parm[1].value.unumber);
        break;
    case BDMF_p2p_ae_sci_hi_10:
        err = ag_drv_xif_p2p_ae_sci_hi_10_set(parm[1].value.unumber);
        break;
    case BDMF_p2p_ae_sci_lo_11:
        err = ag_drv_xif_p2p_ae_sci_lo_11_set(parm[1].value.unumber);
        break;
    case BDMF_p2p_ae_sci_hi_11:
        err = ag_drv_xif_p2p_ae_sci_hi_11_set(parm[1].value.unumber);
        break;
    case BDMF_p2p_ae_sci_lo_12:
        err = ag_drv_xif_p2p_ae_sci_lo_12_set(parm[1].value.unumber);
        break;
    case BDMF_p2p_ae_sci_hi_12:
        err = ag_drv_xif_p2p_ae_sci_hi_12_set(parm[1].value.unumber);
        break;
    case BDMF_p2p_ae_sci_lo_13:
        err = ag_drv_xif_p2p_ae_sci_lo_13_set(parm[1].value.unumber);
        break;
    case BDMF_p2p_ae_sci_hi_13:
        err = ag_drv_xif_p2p_ae_sci_hi_13_set(parm[1].value.unumber);
        break;
    case BDMF_p2p_ae_sci_lo_14:
        err = ag_drv_xif_p2p_ae_sci_lo_14_set(parm[1].value.unumber);
        break;
    case BDMF_p2p_ae_sci_hi_14:
        err = ag_drv_xif_p2p_ae_sci_hi_14_set(parm[1].value.unumber);
        break;
    case BDMF_p2p_ae_sci_lo_15:
        err = ag_drv_xif_p2p_ae_sci_lo_15_set(parm[1].value.unumber);
        break;
    case BDMF_p2p_ae_sci_hi_15:
        err = ag_drv_xif_p2p_ae_sci_hi_15_set(parm[1].value.unumber);
        break;
    default:
        err = BDMF_ERR_NOT_SUPPORTED;
        break;
    }
    return err;
}

static int bcm_xif_cli_get(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    bdmf_error_t err;

    switch(parm[0].value.unumber)
    {
    case BDMF_ctl:
    {
        xif_ctl ctl;
        err = ag_drv_xif_ctl_get(&ctl);
        bdmf_session_print(session, "rxencrypten = %u = 0x%x\n", ctl.rxencrypten, ctl.rxencrypten);
        bdmf_session_print(session, "cfgdisrxdasaencrpt = %u = 0x%x\n", ctl.cfgdisrxdasaencrpt, ctl.cfgdisrxdasaencrpt);
        bdmf_session_print(session, "rxencryptmode = %u = 0x%x\n", ctl.rxencryptmode, ctl.rxencryptmode);
        bdmf_session_print(session, "txencrypten = %u = 0x%x\n", ctl.txencrypten, ctl.txencrypten);
        bdmf_session_print(session, "cfgdistxdasaencrpt = %u = 0x%x\n", ctl.cfgdistxdasaencrpt, ctl.cfgdistxdasaencrpt);
        bdmf_session_print(session, "txencryptmode = %u = 0x%x\n", ctl.txencryptmode, ctl.txencryptmode);
        bdmf_session_print(session, "cfgllidmodemsk = %u = 0x%x\n", ctl.cfgllidmodemsk, ctl.cfgllidmodemsk);
        bdmf_session_print(session, "cfgxpnbadcrc32 = %u = 0x%x\n", ctl.cfgxpnbadcrc32, ctl.cfgxpnbadcrc32);
        bdmf_session_print(session, "cfgdisdiscinfo = %u = 0x%x\n", ctl.cfgdisdiscinfo, ctl.cfgdisdiscinfo);
        bdmf_session_print(session, "cfgpmctx2rxlpbk = %u = 0x%x\n", ctl.cfgpmctx2rxlpbk, ctl.cfgpmctx2rxlpbk);
        bdmf_session_print(session, "cfgpmctxencrc8bad = %u = 0x%x\n", ctl.cfgpmctxencrc8bad, ctl.cfgpmctxencrc8bad);
        bdmf_session_print(session, "cfgenp2p = %u = 0x%x\n", ctl.cfgenp2p, ctl.cfgenp2p);
        bdmf_session_print(session, "cfgllidpromiscuousmode = %u = 0x%x\n", ctl.cfgllidpromiscuousmode, ctl.cfgllidpromiscuousmode);
        bdmf_session_print(session, "cfgenidlepktsup = %u = 0x%x\n", ctl.cfgenidlepktsup, ctl.cfgenidlepktsup);
        bdmf_session_print(session, "cfgpmcrxencrc8chk = %u = 0x%x\n", ctl.cfgpmcrxencrc8chk, ctl.cfgpmcrxencrc8chk);
        bdmf_session_print(session, "cfgen1stidlepktconvert = %u = 0x%x\n", ctl.cfgen1stidlepktconvert, ctl.cfgen1stidlepktconvert);
        bdmf_session_print(session, "cfgfecen = %u = 0x%x\n", ctl.cfgfecen, ctl.cfgfecen);
        bdmf_session_print(session, "cfglegacyrcvtsupd = %u = 0x%x\n", ctl.cfglegacyrcvtsupd, ctl.cfglegacyrcvtsupd);
        bdmf_session_print(session, "cfgxpnencrcpassthru = %u = 0x%x\n", ctl.cfgxpnencrcpassthru, ctl.cfgxpnencrcpassthru);
        bdmf_session_print(session, "cfgxpndistimestampmod = %u = 0x%x\n", ctl.cfgxpndistimestampmod, ctl.cfgxpndistimestampmod);
        bdmf_session_print(session, "xifnotrdy = %u = 0x%x\n", ctl.xifnotrdy, ctl.xifnotrdy);
        bdmf_session_print(session, "xifdtportrstn = %u = 0x%x\n", ctl.xifdtportrstn, ctl.xifdtportrstn);
        bdmf_session_print(session, "xpntxrstn = %u = 0x%x\n", ctl.xpntxrstn, ctl.xpntxrstn);
        bdmf_session_print(session, "pmctxrstn = %u = 0x%x\n", ctl.pmctxrstn, ctl.pmctxrstn);
        bdmf_session_print(session, "sectxrstn = %u = 0x%x\n", ctl.sectxrstn, ctl.sectxrstn);
        bdmf_session_print(session, "cfgdistxoamencrpt = %u = 0x%x\n", ctl.cfgdistxoamencrpt, ctl.cfgdistxoamencrpt);
        bdmf_session_print(session, "cfgdistxmpcpencrpt = %u = 0x%x\n", ctl.cfgdistxmpcpencrpt, ctl.cfgdistxmpcpencrpt);
        bdmf_session_print(session, "pmcrxrstn = %u = 0x%x\n", ctl.pmcrxrstn, ctl.pmcrxrstn);
        bdmf_session_print(session, "secrxrstn = %u = 0x%x\n", ctl.secrxrstn, ctl.secrxrstn);
        break;
    }
    case BDMF_int_status:
    {
        xif_int_status int_status;
        err = ag_drv_xif_int_status_get(&int_status);
        bdmf_session_print(session, "secrxrplyprtctabrtint = %u = 0x%x\n", int_status.secrxrplyprtctabrtint, int_status.secrxrplyprtctabrtint);
        bdmf_session_print(session, "sectxpktnummaxint = %u = 0x%x\n", int_status.sectxpktnummaxint, int_status.sectxpktnummaxint);
        bdmf_session_print(session, "tsfullupdint = %u = 0x%x\n", int_status.tsfullupdint, int_status.tsfullupdint);
        bdmf_session_print(session, "txhangint = %u = 0x%x\n", int_status.txhangint, int_status.txhangint);
        bdmf_session_print(session, "negtimeint = %u = 0x%x\n", int_status.negtimeint, int_status.negtimeint);
        bdmf_session_print(session, "pmctsjttrint = %u = 0x%x\n", int_status.pmctsjttrint, int_status.pmctsjttrint);
        bdmf_session_print(session, "secrxoutffovrflwint = %u = 0x%x\n", int_status.secrxoutffovrflwint, int_status.secrxoutffovrflwint);
        break;
    }
    case BDMF_int_mask:
    {
        xif_int_mask int_mask;
        err = ag_drv_xif_int_mask_get(&int_mask);
        bdmf_session_print(session, "msksecrxreplayprotctabort = %u = 0x%x\n", int_mask.msksecrxreplayprotctabort, int_mask.msksecrxreplayprotctabort);
        bdmf_session_print(session, "mskpktnumthreshint = %u = 0x%x\n", int_mask.mskpktnumthreshint, int_mask.mskpktnumthreshint);
        bdmf_session_print(session, "msktsfullupdint = %u = 0x%x\n", int_mask.msktsfullupdint, int_mask.msktsfullupdint);
        bdmf_session_print(session, "msktxhangint = %u = 0x%x\n", int_mask.msktxhangint, int_mask.msktxhangint);
        bdmf_session_print(session, "msknegtimeint = %u = 0x%x\n", int_mask.msknegtimeint, int_mask.msknegtimeint);
        bdmf_session_print(session, "mskpmctsjttrint = %u = 0x%x\n", int_mask.mskpmctsjttrint, int_mask.mskpmctsjttrint);
        bdmf_session_print(session, "msksecrxoutffint = %u = 0x%x\n", int_mask.msksecrxoutffint, int_mask.msksecrxoutffint);
        break;
    }
    case BDMF_port_command:
    {
        bdmf_boolean dataportbusy;
        uint8_t portselect;
        uint8_t portopcode;
        uint16_t portaddress;
        err = ag_drv_xif_port_command_get(&dataportbusy, &portselect, &portopcode, &portaddress);
        bdmf_session_print(session, "dataportbusy = %u = 0x%x\n", dataportbusy, dataportbusy);
        bdmf_session_print(session, "portselect = %u = 0x%x\n", portselect, portselect);
        bdmf_session_print(session, "portopcode = %u = 0x%x\n", portopcode, portopcode);
        bdmf_session_print(session, "portaddress = %u = 0x%x\n", portaddress, portaddress);
        break;
    }
    case BDMF_port_data_:
    {
        uint32_t portdata;
        err = ag_drv_xif_port_data__get(parm[1].value.unumber, &portdata);
        bdmf_session_print(session, "portdata = %u = 0x%x\n", portdata, portdata);
        break;
    }
    case BDMF_macsec:
    {
        uint16_t cfgmacsecethertype;
        err = ag_drv_xif_macsec_get(&cfgmacsecethertype);
        bdmf_session_print(session, "cfgmacsecethertype = %u = 0x%x\n", cfgmacsecethertype, cfgmacsecethertype);
        break;
    }
    case BDMF_xpn_xmt_offset:
    {
        uint16_t cfgxpnxmtoffset;
        err = ag_drv_xif_xpn_xmt_offset_get(&cfgxpnxmtoffset);
        bdmf_session_print(session, "cfgxpnxmtoffset = %u = 0x%x\n", cfgxpnxmtoffset, cfgxpnxmtoffset);
        break;
    }
    case BDMF_xpn_timestamp_offset:
    {
        uint32_t cfgxpnmpcptsoffset;
        err = ag_drv_xif_xpn_timestamp_offset_get(&cfgxpnmpcptsoffset);
        bdmf_session_print(session, "cfgxpnmpcptsoffset = %u = 0x%x\n", cfgxpnmpcptsoffset, cfgxpnmpcptsoffset);
        break;
    }
    case BDMF_xpn_pktgen_ctl:
    {
        xif_xpn_pktgen_ctl xpn_pktgen_ctl;
        err = ag_drv_xif_xpn_pktgen_ctl_get(&xpn_pktgen_ctl);
        bdmf_session_print(session, "cfgonuburstsize = %u = 0x%x\n", xpn_pktgen_ctl.cfgonuburstsize, xpn_pktgen_ctl.cfgonuburstsize);
        bdmf_session_print(session, "cfgenbck2bckpktgen = %u = 0x%x\n", xpn_pktgen_ctl.cfgenbck2bckpktgen, xpn_pktgen_ctl.cfgenbck2bckpktgen);
        bdmf_session_print(session, "cfgenallmpcppktgen = %u = 0x%x\n", xpn_pktgen_ctl.cfgenallmpcppktgen, xpn_pktgen_ctl.cfgenallmpcppktgen);
        bdmf_session_print(session, "cfgxpnstartpktgen = %u = 0x%x\n", xpn_pktgen_ctl.cfgxpnstartpktgen, xpn_pktgen_ctl.cfgxpnstartpktgen);
        bdmf_session_print(session, "cfgxpnenpktgen = %u = 0x%x\n", xpn_pktgen_ctl.cfgxpnenpktgen, xpn_pktgen_ctl.cfgxpnenpktgen);
        break;
    }
    case BDMF_xpn_pktgen_llid:
    {
        uint16_t cfgxpnpktgenllid1;
        uint16_t cfgxpnpktgenllid0;
        err = ag_drv_xif_xpn_pktgen_llid_get(&cfgxpnpktgenllid1, &cfgxpnpktgenllid0);
        bdmf_session_print(session, "cfgxpnpktgenllid1 = %u = 0x%x\n", cfgxpnpktgenllid1, cfgxpnpktgenllid1);
        bdmf_session_print(session, "cfgxpnpktgenllid0 = %u = 0x%x\n", cfgxpnpktgenllid0, cfgxpnpktgenllid0);
        break;
    }
    case BDMF_xpn_pktgen_pkt_cnt:
    {
        bdmf_boolean cfgxpnpktgenburstmode;
        uint32_t cfgxpnpktgenburstsize;
        err = ag_drv_xif_xpn_pktgen_pkt_cnt_get(&cfgxpnpktgenburstmode, &cfgxpnpktgenburstsize);
        bdmf_session_print(session, "cfgxpnpktgenburstmode = %u = 0x%x\n", cfgxpnpktgenburstmode, cfgxpnpktgenburstmode);
        bdmf_session_print(session, "cfgxpnpktgenburstsize = %u = 0x%x\n", cfgxpnpktgenburstsize, cfgxpnpktgenburstsize);
        break;
    }
    case BDMF_xpn_pktgen_pkt_size:
    {
        bdmf_boolean cfgxpnpktgensizeincr;
        uint16_t cfgxpnpktgensizeend;
        uint16_t cfgxpnpktgensizestart;
        err = ag_drv_xif_xpn_pktgen_pkt_size_get(&cfgxpnpktgensizeincr, &cfgxpnpktgensizeend, &cfgxpnpktgensizestart);
        bdmf_session_print(session, "cfgxpnpktgensizeincr = %u = 0x%x\n", cfgxpnpktgensizeincr, cfgxpnpktgensizeincr);
        bdmf_session_print(session, "cfgxpnpktgensizeend = %u = 0x%x\n", cfgxpnpktgensizeend, cfgxpnpktgensizeend);
        bdmf_session_print(session, "cfgxpnpktgensizestart = %u = 0x%x\n", cfgxpnpktgensizestart, cfgxpnpktgensizestart);
        break;
    }
    case BDMF_xpn_pktgen_ipg:
    {
        uint16_t cfgxpnpktgenbck2bckipg;
        uint16_t cfgxpnpktgenipg;
        err = ag_drv_xif_xpn_pktgen_ipg_get(&cfgxpnpktgenbck2bckipg, &cfgxpnpktgenipg);
        bdmf_session_print(session, "cfgxpnpktgenbck2bckipg = %u = 0x%x\n", cfgxpnpktgenbck2bckipg, cfgxpnpktgenbck2bckipg);
        bdmf_session_print(session, "cfgxpnpktgenipg = %u = 0x%x\n", cfgxpnpktgenipg, cfgxpnpktgenipg);
        break;
    }
    case BDMF_ts_jitter_thresh:
    {
        uint32_t cfgtsjttrthresh;
        err = ag_drv_xif_ts_jitter_thresh_get(&cfgtsjttrthresh);
        bdmf_session_print(session, "cfgtsjttrthresh = %u = 0x%x\n", cfgtsjttrthresh, cfgtsjttrthresh);
        break;
    }
    case BDMF_ts_update:
    {
        uint16_t cfgtsfullupdthr;
        bdmf_boolean cfgenautotsupd;
        uint8_t cfgtsupdper;
        err = ag_drv_xif_ts_update_get(&cfgtsfullupdthr, &cfgenautotsupd, &cfgtsupdper);
        bdmf_session_print(session, "cfgtsfullupdthr = %u = 0x%x\n", cfgtsfullupdthr, cfgtsfullupdthr);
        bdmf_session_print(session, "cfgenautotsupd = %u = 0x%x\n", cfgenautotsupd, cfgenautotsupd);
        bdmf_session_print(session, "cfgtsupdper = %u = 0x%x\n", cfgtsupdper, cfgtsupdper);
        break;
    }
    case BDMF_gnt_overhead:
    {
        uint16_t cfggntoh;
        err = ag_drv_xif_gnt_overhead_get(&cfggntoh);
        bdmf_session_print(session, "cfggntoh = %u = 0x%x\n", cfggntoh, cfggntoh);
        break;
    }
    case BDMF_discover_overhead:
    {
        uint16_t cfgdiscoh;
        err = ag_drv_xif_discover_overhead_get(&cfgdiscoh);
        bdmf_session_print(session, "cfgdiscoh = %u = 0x%x\n", cfgdiscoh, cfgdiscoh);
        break;
    }
    case BDMF_discover_info:
    {
        uint16_t cfgdiscinfofld;
        err = ag_drv_xif_discover_info_get(&cfgdiscinfofld);
        bdmf_session_print(session, "cfgdiscinfofld = %u = 0x%x\n", cfgdiscinfofld, cfgdiscinfofld);
        break;
    }
    case BDMF_xpn_oversize_thresh:
    {
        uint16_t cfgxpnovrszthresh;
        err = ag_drv_xif_xpn_oversize_thresh_get(&cfgxpnovrszthresh);
        bdmf_session_print(session, "cfgxpnovrszthresh = %u = 0x%x\n", cfgxpnovrszthresh, cfgxpnovrszthresh);
        break;
    }
    case BDMF_secrx_keynum:
    {
        uint32_t keystatrx;
        err = ag_drv_xif_secrx_keynum_get(&keystatrx);
        bdmf_session_print(session, "keystatrx = %u = 0x%x\n", keystatrx, keystatrx);
        break;
    }
    case BDMF_secrx_encrypt:
    {
        uint32_t encrstatrx;
        err = ag_drv_xif_secrx_encrypt_get(&encrstatrx);
        bdmf_session_print(session, "encrstatrx = %u = 0x%x\n", encrstatrx, encrstatrx);
        break;
    }
    case BDMF_pmc_frame_rx_cnt:
    {
        uint32_t pmcrxframecnt;
        err = ag_drv_xif_pmc_frame_rx_cnt_get(&pmcrxframecnt);
        bdmf_session_print(session, "pmcrxframecnt = %u = 0x%x\n", pmcrxframecnt, pmcrxframecnt);
        break;
    }
    case BDMF_pmc_byte_rx_cnt:
    {
        uint32_t pmcrxbytecnt;
        err = ag_drv_xif_pmc_byte_rx_cnt_get(&pmcrxbytecnt);
        bdmf_session_print(session, "pmcrxbytecnt = %u = 0x%x\n", pmcrxbytecnt, pmcrxbytecnt);
        break;
    }
    case BDMF_pmc_runt_rx_cnt:
    {
        uint32_t pmcrxruntcnt;
        err = ag_drv_xif_pmc_runt_rx_cnt_get(&pmcrxruntcnt);
        bdmf_session_print(session, "pmcrxruntcnt = %u = 0x%x\n", pmcrxruntcnt, pmcrxruntcnt);
        break;
    }
    case BDMF_pmc_cw_err_rx_cnt:
    {
        uint32_t pmcrxcwerrcnt;
        err = ag_drv_xif_pmc_cw_err_rx_cnt_get(&pmcrxcwerrcnt);
        bdmf_session_print(session, "pmcrxcwerrcnt = %u = 0x%x\n", pmcrxcwerrcnt, pmcrxcwerrcnt);
        break;
    }
    case BDMF_pmc_crc8_err_rx_cnt:
    {
        uint32_t pmcrxcrc8errcnt;
        err = ag_drv_xif_pmc_crc8_err_rx_cnt_get(&pmcrxcrc8errcnt);
        bdmf_session_print(session, "pmcrxcrc8errcnt = %u = 0x%x\n", pmcrxcrc8errcnt, pmcrxcrc8errcnt);
        break;
    }
    case BDMF_xpn_data_frm_cnt:
    {
        uint32_t xpndtframecnt;
        err = ag_drv_xif_xpn_data_frm_cnt_get(&xpndtframecnt);
        bdmf_session_print(session, "xpndtframecnt = %u = 0x%x\n", xpndtframecnt, xpndtframecnt);
        break;
    }
    case BDMF_xpn_data_byte_cnt:
    {
        uint32_t xpndtbytecnt;
        err = ag_drv_xif_xpn_data_byte_cnt_get(&xpndtbytecnt);
        bdmf_session_print(session, "xpndtbytecnt = %u = 0x%x\n", xpndtbytecnt, xpndtbytecnt);
        break;
    }
    case BDMF_xpn_mpcp_frm_cnt:
    {
        uint32_t xpnmpcpframecnt;
        err = ag_drv_xif_xpn_mpcp_frm_cnt_get(&xpnmpcpframecnt);
        bdmf_session_print(session, "xpnmpcpframecnt = %u = 0x%x\n", xpnmpcpframecnt, xpnmpcpframecnt);
        break;
    }
    case BDMF_xpn_oam_frm_cnt:
    {
        uint32_t xpnoamframecnt;
        err = ag_drv_xif_xpn_oam_frm_cnt_get(&xpnoamframecnt);
        bdmf_session_print(session, "xpnoamframecnt = %u = 0x%x\n", xpnoamframecnt, xpnoamframecnt);
        break;
    }
    case BDMF_xpn_oam_byte_cnt:
    {
        uint32_t xpnoambytecnt;
        err = ag_drv_xif_xpn_oam_byte_cnt_get(&xpnoambytecnt);
        bdmf_session_print(session, "xpnoambytecnt = %u = 0x%x\n", xpnoambytecnt, xpnoambytecnt);
        break;
    }
    case BDMF_xpn_oversize_frm_cnt:
    {
        uint32_t xpndtoversizecnt;
        err = ag_drv_xif_xpn_oversize_frm_cnt_get(&xpndtoversizecnt);
        bdmf_session_print(session, "xpndtoversizecnt = %u = 0x%x\n", xpndtoversizecnt, xpndtoversizecnt);
        break;
    }
    case BDMF_sec_abort_frm_cnt:
    {
        uint32_t secrxabortfrmcnt;
        err = ag_drv_xif_sec_abort_frm_cnt_get(&secrxabortfrmcnt);
        bdmf_session_print(session, "secrxabortfrmcnt = %u = 0x%x\n", secrxabortfrmcnt, secrxabortfrmcnt);
        break;
    }
    case BDMF_pmc_tx_neg_event_cnt:
    {
        uint8_t pmctxnegeventcnt;
        err = ag_drv_xif_pmc_tx_neg_event_cnt_get(&pmctxnegeventcnt);
        bdmf_session_print(session, "pmctxnegeventcnt = %u = 0x%x\n", pmctxnegeventcnt, pmctxnegeventcnt);
        break;
    }
    case BDMF_xpn_idle_pkt_cnt:
    {
        uint16_t xpnidleframecnt;
        err = ag_drv_xif_xpn_idle_pkt_cnt_get(&xpnidleframecnt);
        bdmf_session_print(session, "xpnidleframecnt = %u = 0x%x\n", xpnidleframecnt, xpnidleframecnt);
        break;
    }
    case BDMF_llid_0:
    {
        uint32_t cfgonullid0;
        err = ag_drv_xif_llid_0_get(&cfgonullid0);
        bdmf_session_print(session, "cfgonullid0 = %u = 0x%x\n", cfgonullid0, cfgonullid0);
        break;
    }
    case BDMF_llid_1:
    {
        uint32_t cfgonullid1;
        err = ag_drv_xif_llid_1_get(&cfgonullid1);
        bdmf_session_print(session, "cfgonullid1 = %u = 0x%x\n", cfgonullid1, cfgonullid1);
        break;
    }
    case BDMF_llid_2:
    {
        uint32_t cfgonullid2;
        err = ag_drv_xif_llid_2_get(&cfgonullid2);
        bdmf_session_print(session, "cfgonullid2 = %u = 0x%x\n", cfgonullid2, cfgonullid2);
        break;
    }
    case BDMF_llid_3:
    {
        uint32_t cfgonullid3;
        err = ag_drv_xif_llid_3_get(&cfgonullid3);
        bdmf_session_print(session, "cfgonullid3 = %u = 0x%x\n", cfgonullid3, cfgonullid3);
        break;
    }
    case BDMF_llid_4:
    {
        uint32_t cfgonullid4;
        err = ag_drv_xif_llid_4_get(&cfgonullid4);
        bdmf_session_print(session, "cfgonullid4 = %u = 0x%x\n", cfgonullid4, cfgonullid4);
        break;
    }
    case BDMF_llid_5:
    {
        uint32_t cfgonullid5;
        err = ag_drv_xif_llid_5_get(&cfgonullid5);
        bdmf_session_print(session, "cfgonullid5 = %u = 0x%x\n", cfgonullid5, cfgonullid5);
        break;
    }
    case BDMF_llid_6:
    {
        uint32_t cfgonullid6;
        err = ag_drv_xif_llid_6_get(&cfgonullid6);
        bdmf_session_print(session, "cfgonullid6 = %u = 0x%x\n", cfgonullid6, cfgonullid6);
        break;
    }
    case BDMF_llid_7:
    {
        uint32_t cfgonullid7;
        err = ag_drv_xif_llid_7_get(&cfgonullid7);
        bdmf_session_print(session, "cfgonullid7 = %u = 0x%x\n", cfgonullid7, cfgonullid7);
        break;
    }
    case BDMF_llid_8:
    {
        uint32_t cfgonullid8;
        err = ag_drv_xif_llid_8_get(&cfgonullid8);
        bdmf_session_print(session, "cfgonullid8 = %u = 0x%x\n", cfgonullid8, cfgonullid8);
        break;
    }
    case BDMF_llid_9:
    {
        uint32_t cfgonullid9;
        err = ag_drv_xif_llid_9_get(&cfgonullid9);
        bdmf_session_print(session, "cfgonullid9 = %u = 0x%x\n", cfgonullid9, cfgonullid9);
        break;
    }
    case BDMF_llid_10:
    {
        uint32_t cfgonullid10;
        err = ag_drv_xif_llid_10_get(&cfgonullid10);
        bdmf_session_print(session, "cfgonullid10 = %u = 0x%x\n", cfgonullid10, cfgonullid10);
        break;
    }
    case BDMF_llid_11:
    {
        uint32_t cfgonullid11;
        err = ag_drv_xif_llid_11_get(&cfgonullid11);
        bdmf_session_print(session, "cfgonullid11 = %u = 0x%x\n", cfgonullid11, cfgonullid11);
        break;
    }
    case BDMF_llid_12:
    {
        uint32_t cfgonullid12;
        err = ag_drv_xif_llid_12_get(&cfgonullid12);
        bdmf_session_print(session, "cfgonullid12 = %u = 0x%x\n", cfgonullid12, cfgonullid12);
        break;
    }
    case BDMF_llid_13:
    {
        uint32_t cfgonullid13;
        err = ag_drv_xif_llid_13_get(&cfgonullid13);
        bdmf_session_print(session, "cfgonullid13 = %u = 0x%x\n", cfgonullid13, cfgonullid13);
        break;
    }
    case BDMF_llid_14:
    {
        uint32_t cfgonullid14;
        err = ag_drv_xif_llid_14_get(&cfgonullid14);
        bdmf_session_print(session, "cfgonullid14 = %u = 0x%x\n", cfgonullid14, cfgonullid14);
        break;
    }
    case BDMF_llid_15:
    {
        uint32_t cfgonullid15;
        err = ag_drv_xif_llid_15_get(&cfgonullid15);
        bdmf_session_print(session, "cfgonullid15 = %u = 0x%x\n", cfgonullid15, cfgonullid15);
        break;
    }
    case BDMF_llid_16:
    {
        uint32_t cfgonullid16;
        err = ag_drv_xif_llid_16_get(&cfgonullid16);
        bdmf_session_print(session, "cfgonullid16 = %u = 0x%x\n", cfgonullid16, cfgonullid16);
        break;
    }
    case BDMF_llid_17:
    {
        uint32_t cfgonullid17;
        err = ag_drv_xif_llid_17_get(&cfgonullid17);
        bdmf_session_print(session, "cfgonullid17 = %u = 0x%x\n", cfgonullid17, cfgonullid17);
        break;
    }
    case BDMF_llid_18:
    {
        uint32_t cfgonullid18;
        err = ag_drv_xif_llid_18_get(&cfgonullid18);
        bdmf_session_print(session, "cfgonullid18 = %u = 0x%x\n", cfgonullid18, cfgonullid18);
        break;
    }
    case BDMF_llid_19:
    {
        uint32_t cfgonullid19;
        err = ag_drv_xif_llid_19_get(&cfgonullid19);
        bdmf_session_print(session, "cfgonullid19 = %u = 0x%x\n", cfgonullid19, cfgonullid19);
        break;
    }
    case BDMF_llid_20:
    {
        uint32_t cfgonullid20;
        err = ag_drv_xif_llid_20_get(&cfgonullid20);
        bdmf_session_print(session, "cfgonullid20 = %u = 0x%x\n", cfgonullid20, cfgonullid20);
        break;
    }
    case BDMF_llid_21:
    {
        uint32_t cfgonullid21;
        err = ag_drv_xif_llid_21_get(&cfgonullid21);
        bdmf_session_print(session, "cfgonullid21 = %u = 0x%x\n", cfgonullid21, cfgonullid21);
        break;
    }
    case BDMF_llid_22:
    {
        uint32_t cfgonullid22;
        err = ag_drv_xif_llid_22_get(&cfgonullid22);
        bdmf_session_print(session, "cfgonullid22 = %u = 0x%x\n", cfgonullid22, cfgonullid22);
        break;
    }
    case BDMF_llid_23:
    {
        uint32_t cfgonullid23;
        err = ag_drv_xif_llid_23_get(&cfgonullid23);
        bdmf_session_print(session, "cfgonullid23 = %u = 0x%x\n", cfgonullid23, cfgonullid23);
        break;
    }
    case BDMF_llid_24:
    {
        uint32_t cfgonullid24;
        err = ag_drv_xif_llid_24_get(&cfgonullid24);
        bdmf_session_print(session, "cfgonullid24 = %u = 0x%x\n", cfgonullid24, cfgonullid24);
        break;
    }
    case BDMF_llid_25:
    {
        uint32_t cfgonullid25;
        err = ag_drv_xif_llid_25_get(&cfgonullid25);
        bdmf_session_print(session, "cfgonullid25 = %u = 0x%x\n", cfgonullid25, cfgonullid25);
        break;
    }
    case BDMF_llid_26:
    {
        uint32_t cfgonullid26;
        err = ag_drv_xif_llid_26_get(&cfgonullid26);
        bdmf_session_print(session, "cfgonullid26 = %u = 0x%x\n", cfgonullid26, cfgonullid26);
        break;
    }
    case BDMF_llid_27:
    {
        uint32_t cfgonullid27;
        err = ag_drv_xif_llid_27_get(&cfgonullid27);
        bdmf_session_print(session, "cfgonullid27 = %u = 0x%x\n", cfgonullid27, cfgonullid27);
        break;
    }
    case BDMF_llid_28:
    {
        uint32_t cfgonullid28;
        err = ag_drv_xif_llid_28_get(&cfgonullid28);
        bdmf_session_print(session, "cfgonullid28 = %u = 0x%x\n", cfgonullid28, cfgonullid28);
        break;
    }
    case BDMF_llid_29:
    {
        uint32_t cfgonullid29;
        err = ag_drv_xif_llid_29_get(&cfgonullid29);
        bdmf_session_print(session, "cfgonullid29 = %u = 0x%x\n", cfgonullid29, cfgonullid29);
        break;
    }
    case BDMF_llid_30:
    {
        uint32_t cfgonullid30;
        err = ag_drv_xif_llid_30_get(&cfgonullid30);
        bdmf_session_print(session, "cfgonullid30 = %u = 0x%x\n", cfgonullid30, cfgonullid30);
        break;
    }
    case BDMF_llid_31:
    {
        uint32_t cfgonullid31;
        err = ag_drv_xif_llid_31_get(&cfgonullid31);
        bdmf_session_print(session, "cfgonullid31 = %u = 0x%x\n", cfgonullid31, cfgonullid31);
        break;
    }
    case BDMF_max_mpcp_update:
    {
        uint32_t cfgmaxposmpcpupd;
        err = ag_drv_xif_max_mpcp_update_get(&cfgmaxposmpcpupd);
        bdmf_session_print(session, "cfgmaxposmpcpupd = %u = 0x%x\n", cfgmaxposmpcpupd, cfgmaxposmpcpupd);
        break;
    }
    case BDMF_ipg_insertion:
    {
        bdmf_boolean cfgshortipg;
        bdmf_boolean cfginsertipg;
        uint8_t cfgipgword;
        err = ag_drv_xif_ipg_insertion_get(&cfgshortipg, &cfginsertipg, &cfgipgword);
        bdmf_session_print(session, "cfgshortipg = %u = 0x%x\n", cfgshortipg, cfgshortipg);
        bdmf_session_print(session, "cfginsertipg = %u = 0x%x\n", cfginsertipg, cfginsertipg);
        bdmf_session_print(session, "cfgipgword = %u = 0x%x\n", cfgipgword, cfgipgword);
        break;
    }
    case BDMF_transport_time:
    {
        uint32_t cftransporttime;
        err = ag_drv_xif_transport_time_get(&cftransporttime);
        bdmf_session_print(session, "cftransporttime = %u = 0x%x\n", cftransporttime, cftransporttime);
        break;
    }
    case BDMF_mpcp_time:
    {
        uint32_t curmpcpts;
        err = ag_drv_xif_mpcp_time_get(&curmpcpts);
        bdmf_session_print(session, "curmpcpts = %u = 0x%x\n", curmpcpts, curmpcpts);
        break;
    }
    case BDMF_overlap_gnt_oh:
    {
        uint32_t cfgovrlpoh;
        err = ag_drv_xif_overlap_gnt_oh_get(&cfgovrlpoh);
        bdmf_session_print(session, "cfgovrlpoh = %u = 0x%x\n", cfgovrlpoh, cfgovrlpoh);
        break;
    }
    case BDMF_mac_mode:
    {
        bdmf_boolean cfgennogntxmt;
        err = ag_drv_xif_mac_mode_get(&cfgennogntxmt);
        bdmf_session_print(session, "cfgennogntxmt = %u = 0x%x\n", cfgennogntxmt, cfgennogntxmt);
        break;
    }
    case BDMF_pmctx_ctl:
    {
        xif_pmctx_ctl pmctx_ctl;
        err = ag_drv_xif_pmctx_ctl_get(&pmctx_ctl);
        bdmf_session_print(session, "cfgmpcpupdperiod = %u = 0x%x\n", pmctx_ctl.cfgmpcpupdperiod, pmctx_ctl.cfgmpcpupdperiod);
        bdmf_session_print(session, "cfgdis4idleb4startchar = %u = 0x%x\n", pmctx_ctl.cfgdis4idleb4startchar, pmctx_ctl.cfgdis4idleb4startchar);
        bdmf_session_print(session, "cfgenidledscrd = %u = 0x%x\n", pmctx_ctl.cfgenidledscrd, pmctx_ctl.cfgenidledscrd);
        bdmf_session_print(session, "cfgseltxpontime = %u = 0x%x\n", pmctx_ctl.cfgseltxpontime, pmctx_ctl.cfgseltxpontime);
        bdmf_session_print(session, "cfgmpcpcontupd = %u = 0x%x\n", pmctx_ctl.cfgmpcpcontupd, pmctx_ctl.cfgmpcpcontupd);
        bdmf_session_print(session, "cfgenmaxmpcpupd = %u = 0x%x\n", pmctx_ctl.cfgenmaxmpcpupd, pmctx_ctl.cfgenmaxmpcpupd);
        bdmf_session_print(session, "cfgennegtimeabort = %u = 0x%x\n", pmctx_ctl.cfgennegtimeabort, pmctx_ctl.cfgennegtimeabort);
        break;
    }
    case BDMF_sec_ctl:
    {
        bdmf_boolean cfgsecrxenshortlen;
        bdmf_boolean cfgensectxfakeaes;
        bdmf_boolean cfgensecrxfakeaes;
        bdmf_boolean cfgenaereplayprct;
        err = ag_drv_xif_sec_ctl_get(&cfgsecrxenshortlen, &cfgensectxfakeaes, &cfgensecrxfakeaes, &cfgenaereplayprct);
        bdmf_session_print(session, "cfgsecrxenshortlen = %u = 0x%x\n", cfgsecrxenshortlen, cfgsecrxenshortlen);
        bdmf_session_print(session, "cfgensectxfakeaes = %u = 0x%x\n", cfgensectxfakeaes, cfgensectxfakeaes);
        bdmf_session_print(session, "cfgensecrxfakeaes = %u = 0x%x\n", cfgensecrxfakeaes, cfgensecrxfakeaes);
        bdmf_session_print(session, "cfgenaereplayprct = %u = 0x%x\n", cfgenaereplayprct, cfgenaereplayprct);
        break;
    }
    case BDMF_ae_pktnum_window:
    {
        uint32_t cfgaepktnumwnd;
        err = ag_drv_xif_ae_pktnum_window_get(&cfgaepktnumwnd);
        bdmf_session_print(session, "cfgaepktnumwnd = %u = 0x%x\n", cfgaepktnumwnd, cfgaepktnumwnd);
        break;
    }
    case BDMF_ae_pktnum_thresh:
    {
        uint32_t cfgpktnummaxthresh;
        err = ag_drv_xif_ae_pktnum_thresh_get(&cfgpktnummaxthresh);
        bdmf_session_print(session, "cfgpktnummaxthresh = %u = 0x%x\n", cfgpktnummaxthresh, cfgpktnummaxthresh);
        break;
    }
    case BDMF_sectx_keynum:
    {
        uint32_t keystattx;
        err = ag_drv_xif_sectx_keynum_get(&keystattx);
        bdmf_session_print(session, "keystattx = %u = 0x%x\n", keystattx, keystattx);
        break;
    }
    case BDMF_sectx_encrypt:
    {
        uint32_t encrstattx;
        err = ag_drv_xif_sectx_encrypt_get(&encrstattx);
        bdmf_session_print(session, "encrstattx = %u = 0x%x\n", encrstattx, encrstattx);
        break;
    }
    case BDMF_ae_pktnum_stat:
    {
        uint8_t sectxindxwtpktnummax;
        uint8_t secrxindxwtpktnumabort;
        err = ag_drv_xif_ae_pktnum_stat_get(&sectxindxwtpktnummax, &secrxindxwtpktnumabort);
        bdmf_session_print(session, "sectxindxwtpktnummax = %u = 0x%x\n", sectxindxwtpktnummax, sectxindxwtpktnummax);
        bdmf_session_print(session, "secrxindxwtpktnumabort = %u = 0x%x\n", secrxindxwtpktnumabort, secrxindxwtpktnumabort);
        break;
    }
    case BDMF_mpcp_update:
    {
        uint32_t mpcpupdperiod;
        err = ag_drv_xif_mpcp_update_get(&mpcpupdperiod);
        bdmf_session_print(session, "mpcpupdperiod = %u = 0x%x\n", mpcpupdperiod, mpcpupdperiod);
        break;
    }
    case BDMF_burst_prelaunch_offset:
    {
        uint32_t cfgburstprelaunchoffset;
        err = ag_drv_xif_burst_prelaunch_offset_get(&cfgburstprelaunchoffset);
        bdmf_session_print(session, "cfgburstprelaunchoffset = %u = 0x%x\n", cfgburstprelaunchoffset, cfgburstprelaunchoffset);
        break;
    }
    case BDMF_vlan_type:
    {
        uint16_t cfgvlantype;
        err = ag_drv_xif_vlan_type_get(&cfgvlantype);
        bdmf_session_print(session, "cfgvlantype = %u = 0x%x\n", cfgvlantype, cfgvlantype);
        break;
    }
    case BDMF_p2p_ae_sci_en:
    {
        uint16_t cfgp2pscien;
        err = ag_drv_xif_p2p_ae_sci_en_get(&cfgp2pscien);
        bdmf_session_print(session, "cfgp2pscien = %u = 0x%x\n", cfgp2pscien, cfgp2pscien);
        break;
    }
    case BDMF_p2p_ae_sci_lo_0:
    {
        uint32_t cfgp2psci_lo_0;
        err = ag_drv_xif_p2p_ae_sci_lo_0_get(&cfgp2psci_lo_0);
        bdmf_session_print(session, "cfgp2psci_lo_0 = %u = 0x%x\n", cfgp2psci_lo_0, cfgp2psci_lo_0);
        break;
    }
    case BDMF_p2p_ae_sci_hi_0:
    {
        uint32_t cfgp2psci_hi_0;
        err = ag_drv_xif_p2p_ae_sci_hi_0_get(&cfgp2psci_hi_0);
        bdmf_session_print(session, "cfgp2psci_hi_0 = %u = 0x%x\n", cfgp2psci_hi_0, cfgp2psci_hi_0);
        break;
    }
    case BDMF_p2p_ae_sci_lo_1:
    {
        uint32_t cfgp2psci_lo_1;
        err = ag_drv_xif_p2p_ae_sci_lo_1_get(&cfgp2psci_lo_1);
        bdmf_session_print(session, "cfgp2psci_lo_1 = %u = 0x%x\n", cfgp2psci_lo_1, cfgp2psci_lo_1);
        break;
    }
    case BDMF_p2p_ae_sci_hi_1:
    {
        uint32_t cfgp2psci_hi_1;
        err = ag_drv_xif_p2p_ae_sci_hi_1_get(&cfgp2psci_hi_1);
        bdmf_session_print(session, "cfgp2psci_hi_1 = %u = 0x%x\n", cfgp2psci_hi_1, cfgp2psci_hi_1);
        break;
    }
    case BDMF_p2p_ae_sci_lo_2:
    {
        uint32_t cfgp2psci_lo_2;
        err = ag_drv_xif_p2p_ae_sci_lo_2_get(&cfgp2psci_lo_2);
        bdmf_session_print(session, "cfgp2psci_lo_2 = %u = 0x%x\n", cfgp2psci_lo_2, cfgp2psci_lo_2);
        break;
    }
    case BDMF_p2p_ae_sci_hi_2:
    {
        uint32_t cfgp2psci_hi_2;
        err = ag_drv_xif_p2p_ae_sci_hi_2_get(&cfgp2psci_hi_2);
        bdmf_session_print(session, "cfgp2psci_hi_2 = %u = 0x%x\n", cfgp2psci_hi_2, cfgp2psci_hi_2);
        break;
    }
    case BDMF_p2p_ae_sci_lo_3:
    {
        uint32_t cfgp2psci_lo_3;
        err = ag_drv_xif_p2p_ae_sci_lo_3_get(&cfgp2psci_lo_3);
        bdmf_session_print(session, "cfgp2psci_lo_3 = %u = 0x%x\n", cfgp2psci_lo_3, cfgp2psci_lo_3);
        break;
    }
    case BDMF_p2p_ae_sci_hi_3:
    {
        uint32_t cfgp2psci_hi_3;
        err = ag_drv_xif_p2p_ae_sci_hi_3_get(&cfgp2psci_hi_3);
        bdmf_session_print(session, "cfgp2psci_hi_3 = %u = 0x%x\n", cfgp2psci_hi_3, cfgp2psci_hi_3);
        break;
    }
    case BDMF_p2p_ae_sci_lo_4:
    {
        uint32_t cfgp2psci_lo_4;
        err = ag_drv_xif_p2p_ae_sci_lo_4_get(&cfgp2psci_lo_4);
        bdmf_session_print(session, "cfgp2psci_lo_4 = %u = 0x%x\n", cfgp2psci_lo_4, cfgp2psci_lo_4);
        break;
    }
    case BDMF_p2p_ae_sci_hi_4:
    {
        uint32_t cfgp2psci_hi_4;
        err = ag_drv_xif_p2p_ae_sci_hi_4_get(&cfgp2psci_hi_4);
        bdmf_session_print(session, "cfgp2psci_hi_4 = %u = 0x%x\n", cfgp2psci_hi_4, cfgp2psci_hi_4);
        break;
    }
    case BDMF_p2p_ae_sci_lo_5:
    {
        uint32_t cfgp2psci_lo_5;
        err = ag_drv_xif_p2p_ae_sci_lo_5_get(&cfgp2psci_lo_5);
        bdmf_session_print(session, "cfgp2psci_lo_5 = %u = 0x%x\n", cfgp2psci_lo_5, cfgp2psci_lo_5);
        break;
    }
    case BDMF_p2p_ae_sci_hi_5:
    {
        uint32_t cfgp2psci_hi_5;
        err = ag_drv_xif_p2p_ae_sci_hi_5_get(&cfgp2psci_hi_5);
        bdmf_session_print(session, "cfgp2psci_hi_5 = %u = 0x%x\n", cfgp2psci_hi_5, cfgp2psci_hi_5);
        break;
    }
    case BDMF_p2p_ae_sci_lo_6:
    {
        uint32_t cfgp2psci_lo_6;
        err = ag_drv_xif_p2p_ae_sci_lo_6_get(&cfgp2psci_lo_6);
        bdmf_session_print(session, "cfgp2psci_lo_6 = %u = 0x%x\n", cfgp2psci_lo_6, cfgp2psci_lo_6);
        break;
    }
    case BDMF_p2p_ae_sci_hi_6:
    {
        uint32_t cfgp2psci_hi_6;
        err = ag_drv_xif_p2p_ae_sci_hi_6_get(&cfgp2psci_hi_6);
        bdmf_session_print(session, "cfgp2psci_hi_6 = %u = 0x%x\n", cfgp2psci_hi_6, cfgp2psci_hi_6);
        break;
    }
    case BDMF_p2p_ae_sci_lo_7:
    {
        uint32_t cfgp2psci_lo_7;
        err = ag_drv_xif_p2p_ae_sci_lo_7_get(&cfgp2psci_lo_7);
        bdmf_session_print(session, "cfgp2psci_lo_7 = %u = 0x%x\n", cfgp2psci_lo_7, cfgp2psci_lo_7);
        break;
    }
    case BDMF_p2p_ae_sci_hi_7:
    {
        uint32_t cfgp2psci_hi_7;
        err = ag_drv_xif_p2p_ae_sci_hi_7_get(&cfgp2psci_hi_7);
        bdmf_session_print(session, "cfgp2psci_hi_7 = %u = 0x%x\n", cfgp2psci_hi_7, cfgp2psci_hi_7);
        break;
    }
    case BDMF_p2p_ae_sci_lo_8:
    {
        uint32_t cfgp2psci_lo_8;
        err = ag_drv_xif_p2p_ae_sci_lo_8_get(&cfgp2psci_lo_8);
        bdmf_session_print(session, "cfgp2psci_lo_8 = %u = 0x%x\n", cfgp2psci_lo_8, cfgp2psci_lo_8);
        break;
    }
    case BDMF_p2p_ae_sci_hi_8:
    {
        uint32_t cfgp2psci_hi_8;
        err = ag_drv_xif_p2p_ae_sci_hi_8_get(&cfgp2psci_hi_8);
        bdmf_session_print(session, "cfgp2psci_hi_8 = %u = 0x%x\n", cfgp2psci_hi_8, cfgp2psci_hi_8);
        break;
    }
    case BDMF_p2p_ae_sci_lo_9:
    {
        uint32_t cfgp2psci_lo_9;
        err = ag_drv_xif_p2p_ae_sci_lo_9_get(&cfgp2psci_lo_9);
        bdmf_session_print(session, "cfgp2psci_lo_9 = %u = 0x%x\n", cfgp2psci_lo_9, cfgp2psci_lo_9);
        break;
    }
    case BDMF_p2p_ae_sci_hi_9:
    {
        uint32_t cfgp2psci_hi_9;
        err = ag_drv_xif_p2p_ae_sci_hi_9_get(&cfgp2psci_hi_9);
        bdmf_session_print(session, "cfgp2psci_hi_9 = %u = 0x%x\n", cfgp2psci_hi_9, cfgp2psci_hi_9);
        break;
    }
    case BDMF_p2p_ae_sci_lo_10:
    {
        uint32_t cfgp2psci_lo_10;
        err = ag_drv_xif_p2p_ae_sci_lo_10_get(&cfgp2psci_lo_10);
        bdmf_session_print(session, "cfgp2psci_lo_10 = %u = 0x%x\n", cfgp2psci_lo_10, cfgp2psci_lo_10);
        break;
    }
    case BDMF_p2p_ae_sci_hi_10:
    {
        uint32_t cfgp2psci_hi_10;
        err = ag_drv_xif_p2p_ae_sci_hi_10_get(&cfgp2psci_hi_10);
        bdmf_session_print(session, "cfgp2psci_hi_10 = %u = 0x%x\n", cfgp2psci_hi_10, cfgp2psci_hi_10);
        break;
    }
    case BDMF_p2p_ae_sci_lo_11:
    {
        uint32_t cfgp2psci_lo_11;
        err = ag_drv_xif_p2p_ae_sci_lo_11_get(&cfgp2psci_lo_11);
        bdmf_session_print(session, "cfgp2psci_lo_11 = %u = 0x%x\n", cfgp2psci_lo_11, cfgp2psci_lo_11);
        break;
    }
    case BDMF_p2p_ae_sci_hi_11:
    {
        uint32_t cfgp2psci_hi_11;
        err = ag_drv_xif_p2p_ae_sci_hi_11_get(&cfgp2psci_hi_11);
        bdmf_session_print(session, "cfgp2psci_hi_11 = %u = 0x%x\n", cfgp2psci_hi_11, cfgp2psci_hi_11);
        break;
    }
    case BDMF_p2p_ae_sci_lo_12:
    {
        uint32_t cfgp2psci_lo_12;
        err = ag_drv_xif_p2p_ae_sci_lo_12_get(&cfgp2psci_lo_12);
        bdmf_session_print(session, "cfgp2psci_lo_12 = %u = 0x%x\n", cfgp2psci_lo_12, cfgp2psci_lo_12);
        break;
    }
    case BDMF_p2p_ae_sci_hi_12:
    {
        uint32_t cfgp2psci_hi_12;
        err = ag_drv_xif_p2p_ae_sci_hi_12_get(&cfgp2psci_hi_12);
        bdmf_session_print(session, "cfgp2psci_hi_12 = %u = 0x%x\n", cfgp2psci_hi_12, cfgp2psci_hi_12);
        break;
    }
    case BDMF_p2p_ae_sci_lo_13:
    {
        uint32_t cfgp2psci_lo_13;
        err = ag_drv_xif_p2p_ae_sci_lo_13_get(&cfgp2psci_lo_13);
        bdmf_session_print(session, "cfgp2psci_lo_13 = %u = 0x%x\n", cfgp2psci_lo_13, cfgp2psci_lo_13);
        break;
    }
    case BDMF_p2p_ae_sci_hi_13:
    {
        uint32_t cfgp2psci_hi_13;
        err = ag_drv_xif_p2p_ae_sci_hi_13_get(&cfgp2psci_hi_13);
        bdmf_session_print(session, "cfgp2psci_hi_13 = %u = 0x%x\n", cfgp2psci_hi_13, cfgp2psci_hi_13);
        break;
    }
    case BDMF_p2p_ae_sci_lo_14:
    {
        uint32_t cfgp2psci_lo_14;
        err = ag_drv_xif_p2p_ae_sci_lo_14_get(&cfgp2psci_lo_14);
        bdmf_session_print(session, "cfgp2psci_lo_14 = %u = 0x%x\n", cfgp2psci_lo_14, cfgp2psci_lo_14);
        break;
    }
    case BDMF_p2p_ae_sci_hi_14:
    {
        uint32_t cfgp2psci_hi_14;
        err = ag_drv_xif_p2p_ae_sci_hi_14_get(&cfgp2psci_hi_14);
        bdmf_session_print(session, "cfgp2psci_hi_14 = %u = 0x%x\n", cfgp2psci_hi_14, cfgp2psci_hi_14);
        break;
    }
    case BDMF_p2p_ae_sci_lo_15:
    {
        uint32_t cfgp2psci_lo_15;
        err = ag_drv_xif_p2p_ae_sci_lo_15_get(&cfgp2psci_lo_15);
        bdmf_session_print(session, "cfgp2psci_lo_15 = %u = 0x%x\n", cfgp2psci_lo_15, cfgp2psci_lo_15);
        break;
    }
    case BDMF_p2p_ae_sci_hi_15:
    {
        uint32_t cfgp2psci_hi_15;
        err = ag_drv_xif_p2p_ae_sci_hi_15_get(&cfgp2psci_hi_15);
        bdmf_session_print(session, "cfgp2psci_hi_15 = %u = 0x%x\n", cfgp2psci_hi_15, cfgp2psci_hi_15);
        break;
    }
    case BDMF_sectx_keynum_1:
    {
        uint32_t keystattx_hi;
        err = ag_drv_xif_sectx_keynum_1_get(&keystattx_hi);
        bdmf_session_print(session, "keystattx_hi = %u = 0x%x\n", keystattx_hi, keystattx_hi);
        break;
    }
    case BDMF_secrx_keynum_1:
    {
        uint32_t keystatrx_hi;
        err = ag_drv_xif_secrx_keynum_1_get(&keystatrx_hi);
        bdmf_session_print(session, "keystatrx_hi = %u = 0x%x\n", keystatrx_hi, keystatrx_hi);
        break;
    }
    default:
        err = BDMF_ERR_NOT_SUPPORTED;
        break;
    }
    return err;
}

static int bcm_xif_cli_test(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    bdmf_test_method m = parm[0].value.unumber;
    bdmf_error_t err = BDMF_ERR_OK;

    {
        xif_ctl ctl = {.rxencrypten=gtmv(m, 1), .cfgdisrxdasaencrpt=gtmv(m, 1), .rxencryptmode=gtmv(m, 2), .txencrypten=gtmv(m, 1), .cfgdistxdasaencrpt=gtmv(m, 1), .txencryptmode=gtmv(m, 2), .cfgllidmodemsk=gtmv(m, 1), .cfgxpnbadcrc32=gtmv(m, 1), .cfgdisdiscinfo=gtmv(m, 1), .cfgpmctx2rxlpbk=gtmv(m, 1), .cfgpmctxencrc8bad=gtmv(m, 1), .cfgenp2p=gtmv(m, 1), .cfgllidpromiscuousmode=gtmv(m, 1), .cfgenidlepktsup=gtmv(m, 1), .cfgpmcrxencrc8chk=gtmv(m, 1), .cfgen1stidlepktconvert=gtmv(m, 1), .cfgfecen=gtmv(m, 1), .cfglegacyrcvtsupd=gtmv(m, 1), .cfgxpnencrcpassthru=gtmv(m, 1), .cfgxpndistimestampmod=gtmv(m, 1), .xifnotrdy=gtmv(m, 1), .xifdtportrstn=gtmv(m, 1), .xpntxrstn=gtmv(m, 1), .pmctxrstn=gtmv(m, 1), .sectxrstn=gtmv(m, 1), .cfgdistxoamencrpt=gtmv(m, 1), .cfgdistxmpcpencrpt=gtmv(m, 1), .pmcrxrstn=gtmv(m, 1), .secrxrstn=gtmv(m, 1)};
        if(!err) bdmf_session_print(session, "ag_drv_xif_ctl_set( %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u)\n", ctl.rxencrypten, ctl.cfgdisrxdasaencrpt, ctl.rxencryptmode, ctl.txencrypten, ctl.cfgdistxdasaencrpt, ctl.txencryptmode, ctl.cfgllidmodemsk, ctl.cfgxpnbadcrc32, ctl.cfgdisdiscinfo, ctl.cfgpmctx2rxlpbk, ctl.cfgpmctxencrc8bad, ctl.cfgenp2p, ctl.cfgllidpromiscuousmode, ctl.cfgenidlepktsup, ctl.cfgpmcrxencrc8chk, ctl.cfgen1stidlepktconvert, ctl.cfgfecen, ctl.cfglegacyrcvtsupd, ctl.cfgxpnencrcpassthru, ctl.cfgxpndistimestampmod, ctl.xifnotrdy, ctl.xifdtportrstn, ctl.xpntxrstn, ctl.pmctxrstn, ctl.sectxrstn, ctl.cfgdistxoamencrpt, ctl.cfgdistxmpcpencrpt, ctl.pmcrxrstn, ctl.secrxrstn);
        if(!err) ag_drv_xif_ctl_set(&ctl);
        if(!err) ag_drv_xif_ctl_get( &ctl);
        if(!err) bdmf_session_print(session, "ag_drv_xif_ctl_get( %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u)\n", ctl.rxencrypten, ctl.cfgdisrxdasaencrpt, ctl.rxencryptmode, ctl.txencrypten, ctl.cfgdistxdasaencrpt, ctl.txencryptmode, ctl.cfgllidmodemsk, ctl.cfgxpnbadcrc32, ctl.cfgdisdiscinfo, ctl.cfgpmctx2rxlpbk, ctl.cfgpmctxencrc8bad, ctl.cfgenp2p, ctl.cfgllidpromiscuousmode, ctl.cfgenidlepktsup, ctl.cfgpmcrxencrc8chk, ctl.cfgen1stidlepktconvert, ctl.cfgfecen, ctl.cfglegacyrcvtsupd, ctl.cfgxpnencrcpassthru, ctl.cfgxpndistimestampmod, ctl.xifnotrdy, ctl.xifdtportrstn, ctl.xpntxrstn, ctl.pmctxrstn, ctl.sectxrstn, ctl.cfgdistxoamencrpt, ctl.cfgdistxmpcpencrpt, ctl.pmcrxrstn, ctl.secrxrstn);
        if(err || ctl.rxencrypten!=gtmv(m, 1) || ctl.cfgdisrxdasaencrpt!=gtmv(m, 1) || ctl.rxencryptmode!=gtmv(m, 2) || ctl.txencrypten!=gtmv(m, 1) || ctl.cfgdistxdasaencrpt!=gtmv(m, 1) || ctl.txencryptmode!=gtmv(m, 2) || ctl.cfgllidmodemsk!=gtmv(m, 1) || ctl.cfgxpnbadcrc32!=gtmv(m, 1) || ctl.cfgdisdiscinfo!=gtmv(m, 1) || ctl.cfgpmctx2rxlpbk!=gtmv(m, 1) || ctl.cfgpmctxencrc8bad!=gtmv(m, 1) || ctl.cfgenp2p!=gtmv(m, 1) || ctl.cfgllidpromiscuousmode!=gtmv(m, 1) || ctl.cfgenidlepktsup!=gtmv(m, 1) || ctl.cfgpmcrxencrc8chk!=gtmv(m, 1) || ctl.cfgen1stidlepktconvert!=gtmv(m, 1) || ctl.cfgfecen!=gtmv(m, 1) || ctl.cfglegacyrcvtsupd!=gtmv(m, 1) || ctl.cfgxpnencrcpassthru!=gtmv(m, 1) || ctl.cfgxpndistimestampmod!=gtmv(m, 1) || ctl.xifnotrdy!=gtmv(m, 1) || ctl.xifdtportrstn!=gtmv(m, 1) || ctl.xpntxrstn!=gtmv(m, 1) || ctl.pmctxrstn!=gtmv(m, 1) || ctl.sectxrstn!=gtmv(m, 1) || ctl.cfgdistxoamencrpt!=gtmv(m, 1) || ctl.cfgdistxmpcpencrpt!=gtmv(m, 1) || ctl.pmcrxrstn!=gtmv(m, 1) || ctl.secrxrstn!=gtmv(m, 1))
            return err ? err : BDMF_ERR_IO;
    }
    {
        xif_int_status int_status = {.secrxrplyprtctabrtint=gtmv(m, 1), .sectxpktnummaxint=gtmv(m, 1), .tsfullupdint=gtmv(m, 1), .txhangint=gtmv(m, 1), .negtimeint=gtmv(m, 1), .pmctsjttrint=gtmv(m, 1), .secrxoutffovrflwint=gtmv(m, 1)};
        if(!err) bdmf_session_print(session, "ag_drv_xif_int_status_set( %u %u %u %u %u %u %u)\n", int_status.secrxrplyprtctabrtint, int_status.sectxpktnummaxint, int_status.tsfullupdint, int_status.txhangint, int_status.negtimeint, int_status.pmctsjttrint, int_status.secrxoutffovrflwint);
        if(!err) ag_drv_xif_int_status_set(&int_status);
        if(!err) ag_drv_xif_int_status_get( &int_status);
        if(!err) bdmf_session_print(session, "ag_drv_xif_int_status_get( %u %u %u %u %u %u %u)\n", int_status.secrxrplyprtctabrtint, int_status.sectxpktnummaxint, int_status.tsfullupdint, int_status.txhangint, int_status.negtimeint, int_status.pmctsjttrint, int_status.secrxoutffovrflwint);
        if(err || int_status.secrxrplyprtctabrtint!=gtmv(m, 1) || int_status.sectxpktnummaxint!=gtmv(m, 1) || int_status.tsfullupdint!=gtmv(m, 1) || int_status.txhangint!=gtmv(m, 1) || int_status.negtimeint!=gtmv(m, 1) || int_status.pmctsjttrint!=gtmv(m, 1) || int_status.secrxoutffovrflwint!=gtmv(m, 1))
            return err ? err : BDMF_ERR_IO;
    }
    {
        xif_int_mask int_mask = {.msksecrxreplayprotctabort=gtmv(m, 1), .mskpktnumthreshint=gtmv(m, 1), .msktsfullupdint=gtmv(m, 1), .msktxhangint=gtmv(m, 1), .msknegtimeint=gtmv(m, 1), .mskpmctsjttrint=gtmv(m, 1), .msksecrxoutffint=gtmv(m, 1)};
        if(!err) bdmf_session_print(session, "ag_drv_xif_int_mask_set( %u %u %u %u %u %u %u)\n", int_mask.msksecrxreplayprotctabort, int_mask.mskpktnumthreshint, int_mask.msktsfullupdint, int_mask.msktxhangint, int_mask.msknegtimeint, int_mask.mskpmctsjttrint, int_mask.msksecrxoutffint);
        if(!err) ag_drv_xif_int_mask_set(&int_mask);
        if(!err) ag_drv_xif_int_mask_get( &int_mask);
        if(!err) bdmf_session_print(session, "ag_drv_xif_int_mask_get( %u %u %u %u %u %u %u)\n", int_mask.msksecrxreplayprotctabort, int_mask.mskpktnumthreshint, int_mask.msktsfullupdint, int_mask.msktxhangint, int_mask.msknegtimeint, int_mask.mskpmctsjttrint, int_mask.msksecrxoutffint);
        if(err || int_mask.msksecrxreplayprotctabort!=gtmv(m, 1) || int_mask.mskpktnumthreshint!=gtmv(m, 1) || int_mask.msktsfullupdint!=gtmv(m, 1) || int_mask.msktxhangint!=gtmv(m, 1) || int_mask.msknegtimeint!=gtmv(m, 1) || int_mask.mskpmctsjttrint!=gtmv(m, 1) || int_mask.msksecrxoutffint!=gtmv(m, 1))
            return err ? err : BDMF_ERR_IO;
    }
    {
        bdmf_boolean dataportbusy=gtmv(m, 1);
        uint8_t portselect=gtmv(m, 6);
        uint8_t portopcode=gtmv(m, 8);
        uint16_t portaddress=gtmv(m, 16);
        if(!err) bdmf_session_print(session, "ag_drv_xif_port_command_set( %u %u %u %u)\n", dataportbusy, portselect, portopcode, portaddress);
        if(!err) ag_drv_xif_port_command_set(dataportbusy, portselect, portopcode, portaddress);
        if(!err) ag_drv_xif_port_command_get( &dataportbusy, &portselect, &portopcode, &portaddress);
        if(!err) bdmf_session_print(session, "ag_drv_xif_port_command_get( %u %u %u %u)\n", dataportbusy, portselect, portopcode, portaddress);
        if(err || dataportbusy!=gtmv(m, 1) || portselect!=gtmv(m, 6) || portopcode!=gtmv(m, 8) || portaddress!=gtmv(m, 16))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint8_t portidx=gtmv(m, 3);
        uint32_t portdata=gtmv(m, 32);
        if(!err) bdmf_session_print(session, "ag_drv_xif_port_data__set( %u %u)\n", portidx, portdata);
        if(!err) ag_drv_xif_port_data__set(portidx, portdata);
        if(!err) ag_drv_xif_port_data__get( portidx, &portdata);
        if(!err) bdmf_session_print(session, "ag_drv_xif_port_data__get( %u %u)\n", portidx, portdata);
        if(err || portdata!=gtmv(m, 32))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint16_t cfgmacsecethertype=gtmv(m, 16);
        if(!err) bdmf_session_print(session, "ag_drv_xif_macsec_set( %u)\n", cfgmacsecethertype);
        if(!err) ag_drv_xif_macsec_set(cfgmacsecethertype);
        if(!err) ag_drv_xif_macsec_get( &cfgmacsecethertype);
        if(!err) bdmf_session_print(session, "ag_drv_xif_macsec_get( %u)\n", cfgmacsecethertype);
        if(err || cfgmacsecethertype!=gtmv(m, 16))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint16_t cfgxpnxmtoffset=gtmv(m, 16);
        if(!err) bdmf_session_print(session, "ag_drv_xif_xpn_xmt_offset_set( %u)\n", cfgxpnxmtoffset);
        if(!err) ag_drv_xif_xpn_xmt_offset_set(cfgxpnxmtoffset);
        if(!err) ag_drv_xif_xpn_xmt_offset_get( &cfgxpnxmtoffset);
        if(!err) bdmf_session_print(session, "ag_drv_xif_xpn_xmt_offset_get( %u)\n", cfgxpnxmtoffset);
        if(err || cfgxpnxmtoffset!=gtmv(m, 16))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint32_t cfgxpnmpcptsoffset=gtmv(m, 32);
        if(!err) bdmf_session_print(session, "ag_drv_xif_xpn_timestamp_offset_set( %u)\n", cfgxpnmpcptsoffset);
        if(!err) ag_drv_xif_xpn_timestamp_offset_set(cfgxpnmpcptsoffset);
        if(!err) ag_drv_xif_xpn_timestamp_offset_get( &cfgxpnmpcptsoffset);
        if(!err) bdmf_session_print(session, "ag_drv_xif_xpn_timestamp_offset_get( %u)\n", cfgxpnmpcptsoffset);
        if(err || cfgxpnmpcptsoffset!=gtmv(m, 32))
            return err ? err : BDMF_ERR_IO;
    }
    {
        xif_xpn_pktgen_ctl xpn_pktgen_ctl = {.cfgonuburstsize=gtmv(m, 16), .cfgenbck2bckpktgen=gtmv(m, 1), .cfgenallmpcppktgen=gtmv(m, 1), .cfgxpnstartpktgen=gtmv(m, 1), .cfgxpnenpktgen=gtmv(m, 1)};
        if(!err) bdmf_session_print(session, "ag_drv_xif_xpn_pktgen_ctl_set( %u %u %u %u %u)\n", xpn_pktgen_ctl.cfgonuburstsize, xpn_pktgen_ctl.cfgenbck2bckpktgen, xpn_pktgen_ctl.cfgenallmpcppktgen, xpn_pktgen_ctl.cfgxpnstartpktgen, xpn_pktgen_ctl.cfgxpnenpktgen);
        if(!err) ag_drv_xif_xpn_pktgen_ctl_set(&xpn_pktgen_ctl);
        if(!err) ag_drv_xif_xpn_pktgen_ctl_get( &xpn_pktgen_ctl);
        if(!err) bdmf_session_print(session, "ag_drv_xif_xpn_pktgen_ctl_get( %u %u %u %u %u)\n", xpn_pktgen_ctl.cfgonuburstsize, xpn_pktgen_ctl.cfgenbck2bckpktgen, xpn_pktgen_ctl.cfgenallmpcppktgen, xpn_pktgen_ctl.cfgxpnstartpktgen, xpn_pktgen_ctl.cfgxpnenpktgen);
        if(err || xpn_pktgen_ctl.cfgonuburstsize!=gtmv(m, 16) || xpn_pktgen_ctl.cfgenbck2bckpktgen!=gtmv(m, 1) || xpn_pktgen_ctl.cfgenallmpcppktgen!=gtmv(m, 1) || xpn_pktgen_ctl.cfgxpnstartpktgen!=gtmv(m, 1) || xpn_pktgen_ctl.cfgxpnenpktgen!=gtmv(m, 1))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint16_t cfgxpnpktgenllid1=gtmv(m, 16);
        uint16_t cfgxpnpktgenllid0=gtmv(m, 16);
        if(!err) bdmf_session_print(session, "ag_drv_xif_xpn_pktgen_llid_set( %u %u)\n", cfgxpnpktgenllid1, cfgxpnpktgenllid0);
        if(!err) ag_drv_xif_xpn_pktgen_llid_set(cfgxpnpktgenllid1, cfgxpnpktgenllid0);
        if(!err) ag_drv_xif_xpn_pktgen_llid_get( &cfgxpnpktgenllid1, &cfgxpnpktgenllid0);
        if(!err) bdmf_session_print(session, "ag_drv_xif_xpn_pktgen_llid_get( %u %u)\n", cfgxpnpktgenllid1, cfgxpnpktgenllid0);
        if(err || cfgxpnpktgenllid1!=gtmv(m, 16) || cfgxpnpktgenllid0!=gtmv(m, 16))
            return err ? err : BDMF_ERR_IO;
    }
    {
        bdmf_boolean cfgxpnpktgenburstmode=gtmv(m, 1);
        uint32_t cfgxpnpktgenburstsize=gtmv(m, 24);
        if(!err) bdmf_session_print(session, "ag_drv_xif_xpn_pktgen_pkt_cnt_set( %u %u)\n", cfgxpnpktgenburstmode, cfgxpnpktgenburstsize);
        if(!err) ag_drv_xif_xpn_pktgen_pkt_cnt_set(cfgxpnpktgenburstmode, cfgxpnpktgenburstsize);
        if(!err) ag_drv_xif_xpn_pktgen_pkt_cnt_get( &cfgxpnpktgenburstmode, &cfgxpnpktgenburstsize);
        if(!err) bdmf_session_print(session, "ag_drv_xif_xpn_pktgen_pkt_cnt_get( %u %u)\n", cfgxpnpktgenburstmode, cfgxpnpktgenburstsize);
        if(err || cfgxpnpktgenburstmode!=gtmv(m, 1) || cfgxpnpktgenburstsize!=gtmv(m, 24))
            return err ? err : BDMF_ERR_IO;
    }
    {
        bdmf_boolean cfgxpnpktgensizeincr=gtmv(m, 1);
        uint16_t cfgxpnpktgensizeend=gtmv(m, 12);
        uint16_t cfgxpnpktgensizestart=gtmv(m, 12);
        if(!err) bdmf_session_print(session, "ag_drv_xif_xpn_pktgen_pkt_size_set( %u %u %u)\n", cfgxpnpktgensizeincr, cfgxpnpktgensizeend, cfgxpnpktgensizestart);
        if(!err) ag_drv_xif_xpn_pktgen_pkt_size_set(cfgxpnpktgensizeincr, cfgxpnpktgensizeend, cfgxpnpktgensizestart);
        if(!err) ag_drv_xif_xpn_pktgen_pkt_size_get( &cfgxpnpktgensizeincr, &cfgxpnpktgensizeend, &cfgxpnpktgensizestart);
        if(!err) bdmf_session_print(session, "ag_drv_xif_xpn_pktgen_pkt_size_get( %u %u %u)\n", cfgxpnpktgensizeincr, cfgxpnpktgensizeend, cfgxpnpktgensizestart);
        if(err || cfgxpnpktgensizeincr!=gtmv(m, 1) || cfgxpnpktgensizeend!=gtmv(m, 12) || cfgxpnpktgensizestart!=gtmv(m, 12))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint16_t cfgxpnpktgenbck2bckipg=gtmv(m, 16);
        uint16_t cfgxpnpktgenipg=gtmv(m, 16);
        if(!err) bdmf_session_print(session, "ag_drv_xif_xpn_pktgen_ipg_set( %u %u)\n", cfgxpnpktgenbck2bckipg, cfgxpnpktgenipg);
        if(!err) ag_drv_xif_xpn_pktgen_ipg_set(cfgxpnpktgenbck2bckipg, cfgxpnpktgenipg);
        if(!err) ag_drv_xif_xpn_pktgen_ipg_get( &cfgxpnpktgenbck2bckipg, &cfgxpnpktgenipg);
        if(!err) bdmf_session_print(session, "ag_drv_xif_xpn_pktgen_ipg_get( %u %u)\n", cfgxpnpktgenbck2bckipg, cfgxpnpktgenipg);
        if(err || cfgxpnpktgenbck2bckipg!=gtmv(m, 16) || cfgxpnpktgenipg!=gtmv(m, 16))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint32_t cfgtsjttrthresh=gtmv(m, 31);
        if(!err) bdmf_session_print(session, "ag_drv_xif_ts_jitter_thresh_set( %u)\n", cfgtsjttrthresh);
        if(!err) ag_drv_xif_ts_jitter_thresh_set(cfgtsjttrthresh);
        if(!err) ag_drv_xif_ts_jitter_thresh_get( &cfgtsjttrthresh);
        if(!err) bdmf_session_print(session, "ag_drv_xif_ts_jitter_thresh_get( %u)\n", cfgtsjttrthresh);
        if(err || cfgtsjttrthresh!=gtmv(m, 31))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint16_t cfgtsfullupdthr=gtmv(m, 16);
        bdmf_boolean cfgenautotsupd=gtmv(m, 1);
        uint8_t cfgtsupdper=gtmv(m, 8);
        if(!err) bdmf_session_print(session, "ag_drv_xif_ts_update_set( %u %u %u)\n", cfgtsfullupdthr, cfgenautotsupd, cfgtsupdper);
        if(!err) ag_drv_xif_ts_update_set(cfgtsfullupdthr, cfgenautotsupd, cfgtsupdper);
        if(!err) ag_drv_xif_ts_update_get( &cfgtsfullupdthr, &cfgenautotsupd, &cfgtsupdper);
        if(!err) bdmf_session_print(session, "ag_drv_xif_ts_update_get( %u %u %u)\n", cfgtsfullupdthr, cfgenautotsupd, cfgtsupdper);
        if(err || cfgtsfullupdthr!=gtmv(m, 16) || cfgenautotsupd!=gtmv(m, 1) || cfgtsupdper!=gtmv(m, 8))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint16_t cfggntoh=gtmv(m, 16);
        if(!err) bdmf_session_print(session, "ag_drv_xif_gnt_overhead_set( %u)\n", cfggntoh);
        if(!err) ag_drv_xif_gnt_overhead_set(cfggntoh);
        if(!err) ag_drv_xif_gnt_overhead_get( &cfggntoh);
        if(!err) bdmf_session_print(session, "ag_drv_xif_gnt_overhead_get( %u)\n", cfggntoh);
        if(err || cfggntoh!=gtmv(m, 16))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint16_t cfgdiscoh=gtmv(m, 16);
        if(!err) bdmf_session_print(session, "ag_drv_xif_discover_overhead_set( %u)\n", cfgdiscoh);
        if(!err) ag_drv_xif_discover_overhead_set(cfgdiscoh);
        if(!err) ag_drv_xif_discover_overhead_get( &cfgdiscoh);
        if(!err) bdmf_session_print(session, "ag_drv_xif_discover_overhead_get( %u)\n", cfgdiscoh);
        if(err || cfgdiscoh!=gtmv(m, 16))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint16_t cfgdiscinfofld=gtmv(m, 16);
        if(!err) bdmf_session_print(session, "ag_drv_xif_discover_info_set( %u)\n", cfgdiscinfofld);
        if(!err) ag_drv_xif_discover_info_set(cfgdiscinfofld);
        if(!err) ag_drv_xif_discover_info_get( &cfgdiscinfofld);
        if(!err) bdmf_session_print(session, "ag_drv_xif_discover_info_get( %u)\n", cfgdiscinfofld);
        if(err || cfgdiscinfofld!=gtmv(m, 16))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint16_t cfgxpnovrszthresh=gtmv(m, 14);
        if(!err) bdmf_session_print(session, "ag_drv_xif_xpn_oversize_thresh_set( %u)\n", cfgxpnovrszthresh);
        if(!err) ag_drv_xif_xpn_oversize_thresh_set(cfgxpnovrszthresh);
        if(!err) ag_drv_xif_xpn_oversize_thresh_get( &cfgxpnovrszthresh);
        if(!err) bdmf_session_print(session, "ag_drv_xif_xpn_oversize_thresh_get( %u)\n", cfgxpnovrszthresh);
        if(err || cfgxpnovrszthresh!=gtmv(m, 14))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint32_t keystatrx=gtmv(m, 32);
        if(!err) ag_drv_xif_secrx_keynum_get( &keystatrx);
        if(!err) bdmf_session_print(session, "ag_drv_xif_secrx_keynum_get( %u)\n", keystatrx);
    }
    {
        uint32_t encrstatrx=gtmv(m, 32);
        if(!err) ag_drv_xif_secrx_encrypt_get( &encrstatrx);
        if(!err) bdmf_session_print(session, "ag_drv_xif_secrx_encrypt_get( %u)\n", encrstatrx);
    }
    {
        uint32_t pmcrxframecnt=gtmv(m, 32);
        if(!err) ag_drv_xif_pmc_frame_rx_cnt_get( &pmcrxframecnt);
        if(!err) bdmf_session_print(session, "ag_drv_xif_pmc_frame_rx_cnt_get( %u)\n", pmcrxframecnt);
    }
    {
        uint32_t pmcrxbytecnt=gtmv(m, 32);
        if(!err) ag_drv_xif_pmc_byte_rx_cnt_get( &pmcrxbytecnt);
        if(!err) bdmf_session_print(session, "ag_drv_xif_pmc_byte_rx_cnt_get( %u)\n", pmcrxbytecnt);
    }
    {
        uint32_t pmcrxruntcnt=gtmv(m, 32);
        if(!err) ag_drv_xif_pmc_runt_rx_cnt_get( &pmcrxruntcnt);
        if(!err) bdmf_session_print(session, "ag_drv_xif_pmc_runt_rx_cnt_get( %u)\n", pmcrxruntcnt);
    }
    {
        uint32_t pmcrxcwerrcnt=gtmv(m, 32);
        if(!err) ag_drv_xif_pmc_cw_err_rx_cnt_get( &pmcrxcwerrcnt);
        if(!err) bdmf_session_print(session, "ag_drv_xif_pmc_cw_err_rx_cnt_get( %u)\n", pmcrxcwerrcnt);
    }
    {
        uint32_t pmcrxcrc8errcnt=gtmv(m, 32);
        if(!err) ag_drv_xif_pmc_crc8_err_rx_cnt_get( &pmcrxcrc8errcnt);
        if(!err) bdmf_session_print(session, "ag_drv_xif_pmc_crc8_err_rx_cnt_get( %u)\n", pmcrxcrc8errcnt);
    }
    {
        uint32_t xpndtframecnt=gtmv(m, 32);
        if(!err) ag_drv_xif_xpn_data_frm_cnt_get( &xpndtframecnt);
        if(!err) bdmf_session_print(session, "ag_drv_xif_xpn_data_frm_cnt_get( %u)\n", xpndtframecnt);
    }
    {
        uint32_t xpndtbytecnt=gtmv(m, 32);
        if(!err) ag_drv_xif_xpn_data_byte_cnt_get( &xpndtbytecnt);
        if(!err) bdmf_session_print(session, "ag_drv_xif_xpn_data_byte_cnt_get( %u)\n", xpndtbytecnt);
    }
    {
        uint32_t xpnmpcpframecnt=gtmv(m, 32);
        if(!err) ag_drv_xif_xpn_mpcp_frm_cnt_get( &xpnmpcpframecnt);
        if(!err) bdmf_session_print(session, "ag_drv_xif_xpn_mpcp_frm_cnt_get( %u)\n", xpnmpcpframecnt);
    }
    {
        uint32_t xpnoamframecnt=gtmv(m, 32);
        if(!err) ag_drv_xif_xpn_oam_frm_cnt_get( &xpnoamframecnt);
        if(!err) bdmf_session_print(session, "ag_drv_xif_xpn_oam_frm_cnt_get( %u)\n", xpnoamframecnt);
    }
    {
        uint32_t xpnoambytecnt=gtmv(m, 32);
        if(!err) ag_drv_xif_xpn_oam_byte_cnt_get( &xpnoambytecnt);
        if(!err) bdmf_session_print(session, "ag_drv_xif_xpn_oam_byte_cnt_get( %u)\n", xpnoambytecnt);
    }
    {
        uint32_t xpndtoversizecnt=gtmv(m, 32);
        if(!err) ag_drv_xif_xpn_oversize_frm_cnt_get( &xpndtoversizecnt);
        if(!err) bdmf_session_print(session, "ag_drv_xif_xpn_oversize_frm_cnt_get( %u)\n", xpndtoversizecnt);
    }
    {
        uint32_t secrxabortfrmcnt=gtmv(m, 32);
        if(!err) ag_drv_xif_sec_abort_frm_cnt_get( &secrxabortfrmcnt);
        if(!err) bdmf_session_print(session, "ag_drv_xif_sec_abort_frm_cnt_get( %u)\n", secrxabortfrmcnt);
    }
    {
        uint8_t pmctxnegeventcnt=gtmv(m, 8);
        if(!err) ag_drv_xif_pmc_tx_neg_event_cnt_get( &pmctxnegeventcnt);
        if(!err) bdmf_session_print(session, "ag_drv_xif_pmc_tx_neg_event_cnt_get( %u)\n", pmctxnegeventcnt);
    }
    {
        uint16_t xpnidleframecnt=gtmv(m, 16);
        if(!err) ag_drv_xif_xpn_idle_pkt_cnt_get( &xpnidleframecnt);
        if(!err) bdmf_session_print(session, "ag_drv_xif_xpn_idle_pkt_cnt_get( %u)\n", xpnidleframecnt);
    }
    {
        uint32_t cfgonullid0=gtmv(m, 17);
        if(!err) bdmf_session_print(session, "ag_drv_xif_llid_0_set( %u)\n", cfgonullid0);
        if(!err) ag_drv_xif_llid_0_set(cfgonullid0);
        if(!err) ag_drv_xif_llid_0_get( &cfgonullid0);
        if(!err) bdmf_session_print(session, "ag_drv_xif_llid_0_get( %u)\n", cfgonullid0);
        if(err || cfgonullid0!=gtmv(m, 17))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint32_t cfgonullid1=gtmv(m, 17);
        if(!err) bdmf_session_print(session, "ag_drv_xif_llid_1_set( %u)\n", cfgonullid1);
        if(!err) ag_drv_xif_llid_1_set(cfgonullid1);
        if(!err) ag_drv_xif_llid_1_get( &cfgonullid1);
        if(!err) bdmf_session_print(session, "ag_drv_xif_llid_1_get( %u)\n", cfgonullid1);
        if(err || cfgonullid1!=gtmv(m, 17))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint32_t cfgonullid2=gtmv(m, 17);
        if(!err) bdmf_session_print(session, "ag_drv_xif_llid_2_set( %u)\n", cfgonullid2);
        if(!err) ag_drv_xif_llid_2_set(cfgonullid2);
        if(!err) ag_drv_xif_llid_2_get( &cfgonullid2);
        if(!err) bdmf_session_print(session, "ag_drv_xif_llid_2_get( %u)\n", cfgonullid2);
        if(err || cfgonullid2!=gtmv(m, 17))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint32_t cfgonullid3=gtmv(m, 17);
        if(!err) bdmf_session_print(session, "ag_drv_xif_llid_3_set( %u)\n", cfgonullid3);
        if(!err) ag_drv_xif_llid_3_set(cfgonullid3);
        if(!err) ag_drv_xif_llid_3_get( &cfgonullid3);
        if(!err) bdmf_session_print(session, "ag_drv_xif_llid_3_get( %u)\n", cfgonullid3);
        if(err || cfgonullid3!=gtmv(m, 17))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint32_t cfgonullid4=gtmv(m, 17);
        if(!err) bdmf_session_print(session, "ag_drv_xif_llid_4_set( %u)\n", cfgonullid4);
        if(!err) ag_drv_xif_llid_4_set(cfgonullid4);
        if(!err) ag_drv_xif_llid_4_get( &cfgonullid4);
        if(!err) bdmf_session_print(session, "ag_drv_xif_llid_4_get( %u)\n", cfgonullid4);
        if(err || cfgonullid4!=gtmv(m, 17))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint32_t cfgonullid5=gtmv(m, 17);
        if(!err) bdmf_session_print(session, "ag_drv_xif_llid_5_set( %u)\n", cfgonullid5);
        if(!err) ag_drv_xif_llid_5_set(cfgonullid5);
        if(!err) ag_drv_xif_llid_5_get( &cfgonullid5);
        if(!err) bdmf_session_print(session, "ag_drv_xif_llid_5_get( %u)\n", cfgonullid5);
        if(err || cfgonullid5!=gtmv(m, 17))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint32_t cfgonullid6=gtmv(m, 17);
        if(!err) bdmf_session_print(session, "ag_drv_xif_llid_6_set( %u)\n", cfgonullid6);
        if(!err) ag_drv_xif_llid_6_set(cfgonullid6);
        if(!err) ag_drv_xif_llid_6_get( &cfgonullid6);
        if(!err) bdmf_session_print(session, "ag_drv_xif_llid_6_get( %u)\n", cfgonullid6);
        if(err || cfgonullid6!=gtmv(m, 17))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint32_t cfgonullid7=gtmv(m, 17);
        if(!err) bdmf_session_print(session, "ag_drv_xif_llid_7_set( %u)\n", cfgonullid7);
        if(!err) ag_drv_xif_llid_7_set(cfgonullid7);
        if(!err) ag_drv_xif_llid_7_get( &cfgonullid7);
        if(!err) bdmf_session_print(session, "ag_drv_xif_llid_7_get( %u)\n", cfgonullid7);
        if(err || cfgonullid7!=gtmv(m, 17))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint32_t cfgonullid8=gtmv(m, 17);
        if(!err) bdmf_session_print(session, "ag_drv_xif_llid_8_set( %u)\n", cfgonullid8);
        if(!err) ag_drv_xif_llid_8_set(cfgonullid8);
        if(!err) ag_drv_xif_llid_8_get( &cfgonullid8);
        if(!err) bdmf_session_print(session, "ag_drv_xif_llid_8_get( %u)\n", cfgonullid8);
        if(err || cfgonullid8!=gtmv(m, 17))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint32_t cfgonullid9=gtmv(m, 17);
        if(!err) bdmf_session_print(session, "ag_drv_xif_llid_9_set( %u)\n", cfgonullid9);
        if(!err) ag_drv_xif_llid_9_set(cfgonullid9);
        if(!err) ag_drv_xif_llid_9_get( &cfgonullid9);
        if(!err) bdmf_session_print(session, "ag_drv_xif_llid_9_get( %u)\n", cfgonullid9);
        if(err || cfgonullid9!=gtmv(m, 17))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint32_t cfgonullid10=gtmv(m, 17);
        if(!err) bdmf_session_print(session, "ag_drv_xif_llid_10_set( %u)\n", cfgonullid10);
        if(!err) ag_drv_xif_llid_10_set(cfgonullid10);
        if(!err) ag_drv_xif_llid_10_get( &cfgonullid10);
        if(!err) bdmf_session_print(session, "ag_drv_xif_llid_10_get( %u)\n", cfgonullid10);
        if(err || cfgonullid10!=gtmv(m, 17))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint32_t cfgonullid11=gtmv(m, 17);
        if(!err) bdmf_session_print(session, "ag_drv_xif_llid_11_set( %u)\n", cfgonullid11);
        if(!err) ag_drv_xif_llid_11_set(cfgonullid11);
        if(!err) ag_drv_xif_llid_11_get( &cfgonullid11);
        if(!err) bdmf_session_print(session, "ag_drv_xif_llid_11_get( %u)\n", cfgonullid11);
        if(err || cfgonullid11!=gtmv(m, 17))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint32_t cfgonullid12=gtmv(m, 17);
        if(!err) bdmf_session_print(session, "ag_drv_xif_llid_12_set( %u)\n", cfgonullid12);
        if(!err) ag_drv_xif_llid_12_set(cfgonullid12);
        if(!err) ag_drv_xif_llid_12_get( &cfgonullid12);
        if(!err) bdmf_session_print(session, "ag_drv_xif_llid_12_get( %u)\n", cfgonullid12);
        if(err || cfgonullid12!=gtmv(m, 17))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint32_t cfgonullid13=gtmv(m, 17);
        if(!err) bdmf_session_print(session, "ag_drv_xif_llid_13_set( %u)\n", cfgonullid13);
        if(!err) ag_drv_xif_llid_13_set(cfgonullid13);
        if(!err) ag_drv_xif_llid_13_get( &cfgonullid13);
        if(!err) bdmf_session_print(session, "ag_drv_xif_llid_13_get( %u)\n", cfgonullid13);
        if(err || cfgonullid13!=gtmv(m, 17))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint32_t cfgonullid14=gtmv(m, 17);
        if(!err) bdmf_session_print(session, "ag_drv_xif_llid_14_set( %u)\n", cfgonullid14);
        if(!err) ag_drv_xif_llid_14_set(cfgonullid14);
        if(!err) ag_drv_xif_llid_14_get( &cfgonullid14);
        if(!err) bdmf_session_print(session, "ag_drv_xif_llid_14_get( %u)\n", cfgonullid14);
        if(err || cfgonullid14!=gtmv(m, 17))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint32_t cfgonullid15=gtmv(m, 17);
        if(!err) bdmf_session_print(session, "ag_drv_xif_llid_15_set( %u)\n", cfgonullid15);
        if(!err) ag_drv_xif_llid_15_set(cfgonullid15);
        if(!err) ag_drv_xif_llid_15_get( &cfgonullid15);
        if(!err) bdmf_session_print(session, "ag_drv_xif_llid_15_get( %u)\n", cfgonullid15);
        if(err || cfgonullid15!=gtmv(m, 17))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint32_t cfgonullid16=gtmv(m, 17);
        if(!err) bdmf_session_print(session, "ag_drv_xif_llid_16_set( %u)\n", cfgonullid16);
        if(!err) ag_drv_xif_llid_16_set(cfgonullid16);
        if(!err) ag_drv_xif_llid_16_get( &cfgonullid16);
        if(!err) bdmf_session_print(session, "ag_drv_xif_llid_16_get( %u)\n", cfgonullid16);
        if(err || cfgonullid16!=gtmv(m, 17))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint32_t cfgonullid17=gtmv(m, 17);
        if(!err) bdmf_session_print(session, "ag_drv_xif_llid_17_set( %u)\n", cfgonullid17);
        if(!err) ag_drv_xif_llid_17_set(cfgonullid17);
        if(!err) ag_drv_xif_llid_17_get( &cfgonullid17);
        if(!err) bdmf_session_print(session, "ag_drv_xif_llid_17_get( %u)\n", cfgonullid17);
        if(err || cfgonullid17!=gtmv(m, 17))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint32_t cfgonullid18=gtmv(m, 17);
        if(!err) bdmf_session_print(session, "ag_drv_xif_llid_18_set( %u)\n", cfgonullid18);
        if(!err) ag_drv_xif_llid_18_set(cfgonullid18);
        if(!err) ag_drv_xif_llid_18_get( &cfgonullid18);
        if(!err) bdmf_session_print(session, "ag_drv_xif_llid_18_get( %u)\n", cfgonullid18);
        if(err || cfgonullid18!=gtmv(m, 17))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint32_t cfgonullid19=gtmv(m, 17);
        if(!err) bdmf_session_print(session, "ag_drv_xif_llid_19_set( %u)\n", cfgonullid19);
        if(!err) ag_drv_xif_llid_19_set(cfgonullid19);
        if(!err) ag_drv_xif_llid_19_get( &cfgonullid19);
        if(!err) bdmf_session_print(session, "ag_drv_xif_llid_19_get( %u)\n", cfgonullid19);
        if(err || cfgonullid19!=gtmv(m, 17))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint32_t cfgonullid20=gtmv(m, 17);
        if(!err) bdmf_session_print(session, "ag_drv_xif_llid_20_set( %u)\n", cfgonullid20);
        if(!err) ag_drv_xif_llid_20_set(cfgonullid20);
        if(!err) ag_drv_xif_llid_20_get( &cfgonullid20);
        if(!err) bdmf_session_print(session, "ag_drv_xif_llid_20_get( %u)\n", cfgonullid20);
        if(err || cfgonullid20!=gtmv(m, 17))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint32_t cfgonullid21=gtmv(m, 17);
        if(!err) bdmf_session_print(session, "ag_drv_xif_llid_21_set( %u)\n", cfgonullid21);
        if(!err) ag_drv_xif_llid_21_set(cfgonullid21);
        if(!err) ag_drv_xif_llid_21_get( &cfgonullid21);
        if(!err) bdmf_session_print(session, "ag_drv_xif_llid_21_get( %u)\n", cfgonullid21);
        if(err || cfgonullid21!=gtmv(m, 17))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint32_t cfgonullid22=gtmv(m, 17);
        if(!err) bdmf_session_print(session, "ag_drv_xif_llid_22_set( %u)\n", cfgonullid22);
        if(!err) ag_drv_xif_llid_22_set(cfgonullid22);
        if(!err) ag_drv_xif_llid_22_get( &cfgonullid22);
        if(!err) bdmf_session_print(session, "ag_drv_xif_llid_22_get( %u)\n", cfgonullid22);
        if(err || cfgonullid22!=gtmv(m, 17))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint32_t cfgonullid23=gtmv(m, 17);
        if(!err) bdmf_session_print(session, "ag_drv_xif_llid_23_set( %u)\n", cfgonullid23);
        if(!err) ag_drv_xif_llid_23_set(cfgonullid23);
        if(!err) ag_drv_xif_llid_23_get( &cfgonullid23);
        if(!err) bdmf_session_print(session, "ag_drv_xif_llid_23_get( %u)\n", cfgonullid23);
        if(err || cfgonullid23!=gtmv(m, 17))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint32_t cfgonullid24=gtmv(m, 17);
        if(!err) bdmf_session_print(session, "ag_drv_xif_llid_24_set( %u)\n", cfgonullid24);
        if(!err) ag_drv_xif_llid_24_set(cfgonullid24);
        if(!err) ag_drv_xif_llid_24_get( &cfgonullid24);
        if(!err) bdmf_session_print(session, "ag_drv_xif_llid_24_get( %u)\n", cfgonullid24);
        if(err || cfgonullid24!=gtmv(m, 17))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint32_t cfgonullid25=gtmv(m, 17);
        if(!err) bdmf_session_print(session, "ag_drv_xif_llid_25_set( %u)\n", cfgonullid25);
        if(!err) ag_drv_xif_llid_25_set(cfgonullid25);
        if(!err) ag_drv_xif_llid_25_get( &cfgonullid25);
        if(!err) bdmf_session_print(session, "ag_drv_xif_llid_25_get( %u)\n", cfgonullid25);
        if(err || cfgonullid25!=gtmv(m, 17))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint32_t cfgonullid26=gtmv(m, 17);
        if(!err) bdmf_session_print(session, "ag_drv_xif_llid_26_set( %u)\n", cfgonullid26);
        if(!err) ag_drv_xif_llid_26_set(cfgonullid26);
        if(!err) ag_drv_xif_llid_26_get( &cfgonullid26);
        if(!err) bdmf_session_print(session, "ag_drv_xif_llid_26_get( %u)\n", cfgonullid26);
        if(err || cfgonullid26!=gtmv(m, 17))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint32_t cfgonullid27=gtmv(m, 17);
        if(!err) bdmf_session_print(session, "ag_drv_xif_llid_27_set( %u)\n", cfgonullid27);
        if(!err) ag_drv_xif_llid_27_set(cfgonullid27);
        if(!err) ag_drv_xif_llid_27_get( &cfgonullid27);
        if(!err) bdmf_session_print(session, "ag_drv_xif_llid_27_get( %u)\n", cfgonullid27);
        if(err || cfgonullid27!=gtmv(m, 17))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint32_t cfgonullid28=gtmv(m, 17);
        if(!err) bdmf_session_print(session, "ag_drv_xif_llid_28_set( %u)\n", cfgonullid28);
        if(!err) ag_drv_xif_llid_28_set(cfgonullid28);
        if(!err) ag_drv_xif_llid_28_get( &cfgonullid28);
        if(!err) bdmf_session_print(session, "ag_drv_xif_llid_28_get( %u)\n", cfgonullid28);
        if(err || cfgonullid28!=gtmv(m, 17))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint32_t cfgonullid29=gtmv(m, 17);
        if(!err) bdmf_session_print(session, "ag_drv_xif_llid_29_set( %u)\n", cfgonullid29);
        if(!err) ag_drv_xif_llid_29_set(cfgonullid29);
        if(!err) ag_drv_xif_llid_29_get( &cfgonullid29);
        if(!err) bdmf_session_print(session, "ag_drv_xif_llid_29_get( %u)\n", cfgonullid29);
        if(err || cfgonullid29!=gtmv(m, 17))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint32_t cfgonullid30=gtmv(m, 17);
        if(!err) bdmf_session_print(session, "ag_drv_xif_llid_30_set( %u)\n", cfgonullid30);
        if(!err) ag_drv_xif_llid_30_set(cfgonullid30);
        if(!err) ag_drv_xif_llid_30_get( &cfgonullid30);
        if(!err) bdmf_session_print(session, "ag_drv_xif_llid_30_get( %u)\n", cfgonullid30);
        if(err || cfgonullid30!=gtmv(m, 17))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint32_t cfgonullid31=gtmv(m, 17);
        if(!err) bdmf_session_print(session, "ag_drv_xif_llid_31_set( %u)\n", cfgonullid31);
        if(!err) ag_drv_xif_llid_31_set(cfgonullid31);
        if(!err) ag_drv_xif_llid_31_get( &cfgonullid31);
        if(!err) bdmf_session_print(session, "ag_drv_xif_llid_31_get( %u)\n", cfgonullid31);
        if(err || cfgonullid31!=gtmv(m, 17))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint32_t cfgmaxposmpcpupd=gtmv(m, 32);
        if(!err) bdmf_session_print(session, "ag_drv_xif_max_mpcp_update_set( %u)\n", cfgmaxposmpcpupd);
        if(!err) ag_drv_xif_max_mpcp_update_set(cfgmaxposmpcpupd);
        if(!err) ag_drv_xif_max_mpcp_update_get( &cfgmaxposmpcpupd);
        if(!err) bdmf_session_print(session, "ag_drv_xif_max_mpcp_update_get( %u)\n", cfgmaxposmpcpupd);
        if(err || cfgmaxposmpcpupd!=gtmv(m, 32))
            return err ? err : BDMF_ERR_IO;
    }
    {
        bdmf_boolean cfgshortipg=gtmv(m, 1);
        bdmf_boolean cfginsertipg=gtmv(m, 1);
        uint8_t cfgipgword=gtmv(m, 7);
        if(!err) bdmf_session_print(session, "ag_drv_xif_ipg_insertion_set( %u %u %u)\n", cfgshortipg, cfginsertipg, cfgipgword);
        if(!err) ag_drv_xif_ipg_insertion_set(cfgshortipg, cfginsertipg, cfgipgword);
        if(!err) ag_drv_xif_ipg_insertion_get( &cfgshortipg, &cfginsertipg, &cfgipgword);
        if(!err) bdmf_session_print(session, "ag_drv_xif_ipg_insertion_get( %u %u %u)\n", cfgshortipg, cfginsertipg, cfgipgword);
        if(err || cfgshortipg!=gtmv(m, 1) || cfginsertipg!=gtmv(m, 1) || cfgipgword!=gtmv(m, 7))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint32_t cftransporttime=gtmv(m, 32);
        if(!err) bdmf_session_print(session, "ag_drv_xif_transport_time_set( %u)\n", cftransporttime);
        if(!err) ag_drv_xif_transport_time_set(cftransporttime);
        if(!err) ag_drv_xif_transport_time_get( &cftransporttime);
        if(!err) bdmf_session_print(session, "ag_drv_xif_transport_time_get( %u)\n", cftransporttime);
        if(err || cftransporttime!=gtmv(m, 32))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint32_t curmpcpts=gtmv(m, 32);
        if(!err) ag_drv_xif_mpcp_time_get( &curmpcpts);
        if(!err) bdmf_session_print(session, "ag_drv_xif_mpcp_time_get( %u)\n", curmpcpts);
    }
    {
        uint32_t cfgovrlpoh=gtmv(m, 32);
        if(!err) bdmf_session_print(session, "ag_drv_xif_overlap_gnt_oh_set( %u)\n", cfgovrlpoh);
        if(!err) ag_drv_xif_overlap_gnt_oh_set(cfgovrlpoh);
        if(!err) ag_drv_xif_overlap_gnt_oh_get( &cfgovrlpoh);
        if(!err) bdmf_session_print(session, "ag_drv_xif_overlap_gnt_oh_get( %u)\n", cfgovrlpoh);
        if(err || cfgovrlpoh!=gtmv(m, 32))
            return err ? err : BDMF_ERR_IO;
    }
    {
        bdmf_boolean cfgennogntxmt=gtmv(m, 1);
        if(!err) bdmf_session_print(session, "ag_drv_xif_mac_mode_set( %u)\n", cfgennogntxmt);
        if(!err) ag_drv_xif_mac_mode_set(cfgennogntxmt);
        if(!err) ag_drv_xif_mac_mode_get( &cfgennogntxmt);
        if(!err) bdmf_session_print(session, "ag_drv_xif_mac_mode_get( %u)\n", cfgennogntxmt);
        if(err || cfgennogntxmt!=gtmv(m, 1))
            return err ? err : BDMF_ERR_IO;
    }
    {
        xif_pmctx_ctl pmctx_ctl = {.cfgmpcpupdperiod=gtmv(m, 8), .cfgdis4idleb4startchar=gtmv(m, 1), .cfgenidledscrd=gtmv(m, 1), .cfgseltxpontime=gtmv(m, 1), .cfgmpcpcontupd=gtmv(m, 1), .cfgenmaxmpcpupd=gtmv(m, 1), .cfgennegtimeabort=gtmv(m, 1)};
        if(!err) bdmf_session_print(session, "ag_drv_xif_pmctx_ctl_set( %u %u %u %u %u %u %u)\n", pmctx_ctl.cfgmpcpupdperiod, pmctx_ctl.cfgdis4idleb4startchar, pmctx_ctl.cfgenidledscrd, pmctx_ctl.cfgseltxpontime, pmctx_ctl.cfgmpcpcontupd, pmctx_ctl.cfgenmaxmpcpupd, pmctx_ctl.cfgennegtimeabort);
        if(!err) ag_drv_xif_pmctx_ctl_set(&pmctx_ctl);
        if(!err) ag_drv_xif_pmctx_ctl_get( &pmctx_ctl);
        if(!err) bdmf_session_print(session, "ag_drv_xif_pmctx_ctl_get( %u %u %u %u %u %u %u)\n", pmctx_ctl.cfgmpcpupdperiod, pmctx_ctl.cfgdis4idleb4startchar, pmctx_ctl.cfgenidledscrd, pmctx_ctl.cfgseltxpontime, pmctx_ctl.cfgmpcpcontupd, pmctx_ctl.cfgenmaxmpcpupd, pmctx_ctl.cfgennegtimeabort);
        if(err || pmctx_ctl.cfgmpcpupdperiod!=gtmv(m, 8) || pmctx_ctl.cfgdis4idleb4startchar!=gtmv(m, 1) || pmctx_ctl.cfgenidledscrd!=gtmv(m, 1) || pmctx_ctl.cfgseltxpontime!=gtmv(m, 1) || pmctx_ctl.cfgmpcpcontupd!=gtmv(m, 1) || pmctx_ctl.cfgenmaxmpcpupd!=gtmv(m, 1) || pmctx_ctl.cfgennegtimeabort!=gtmv(m, 1))
            return err ? err : BDMF_ERR_IO;
    }
    {
        bdmf_boolean cfgsecrxenshortlen=gtmv(m, 1);
        bdmf_boolean cfgensectxfakeaes=gtmv(m, 1);
        bdmf_boolean cfgensecrxfakeaes=gtmv(m, 1);
        bdmf_boolean cfgenaereplayprct=gtmv(m, 1);
        if(!err) bdmf_session_print(session, "ag_drv_xif_sec_ctl_set( %u %u %u %u)\n", cfgsecrxenshortlen, cfgensectxfakeaes, cfgensecrxfakeaes, cfgenaereplayprct);
        if(!err) ag_drv_xif_sec_ctl_set(cfgsecrxenshortlen, cfgensectxfakeaes, cfgensecrxfakeaes, cfgenaereplayprct);
        if(!err) ag_drv_xif_sec_ctl_get( &cfgsecrxenshortlen, &cfgensectxfakeaes, &cfgensecrxfakeaes, &cfgenaereplayprct);
        if(!err) bdmf_session_print(session, "ag_drv_xif_sec_ctl_get( %u %u %u %u)\n", cfgsecrxenshortlen, cfgensectxfakeaes, cfgensecrxfakeaes, cfgenaereplayprct);
        if(err || cfgsecrxenshortlen!=gtmv(m, 1) || cfgensectxfakeaes!=gtmv(m, 1) || cfgensecrxfakeaes!=gtmv(m, 1) || cfgenaereplayprct!=gtmv(m, 1))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint32_t cfgaepktnumwnd=gtmv(m, 32);
        if(!err) bdmf_session_print(session, "ag_drv_xif_ae_pktnum_window_set( %u)\n", cfgaepktnumwnd);
        if(!err) ag_drv_xif_ae_pktnum_window_set(cfgaepktnumwnd);
        if(!err) ag_drv_xif_ae_pktnum_window_get( &cfgaepktnumwnd);
        if(!err) bdmf_session_print(session, "ag_drv_xif_ae_pktnum_window_get( %u)\n", cfgaepktnumwnd);
        if(err || cfgaepktnumwnd!=gtmv(m, 32))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint32_t cfgpktnummaxthresh=gtmv(m, 32);
        if(!err) bdmf_session_print(session, "ag_drv_xif_ae_pktnum_thresh_set( %u)\n", cfgpktnummaxthresh);
        if(!err) ag_drv_xif_ae_pktnum_thresh_set(cfgpktnummaxthresh);
        if(!err) ag_drv_xif_ae_pktnum_thresh_get( &cfgpktnummaxthresh);
        if(!err) bdmf_session_print(session, "ag_drv_xif_ae_pktnum_thresh_get( %u)\n", cfgpktnummaxthresh);
        if(err || cfgpktnummaxthresh!=gtmv(m, 32))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint32_t keystattx=gtmv(m, 32);
        if(!err) ag_drv_xif_sectx_keynum_get( &keystattx);
        if(!err) bdmf_session_print(session, "ag_drv_xif_sectx_keynum_get( %u)\n", keystattx);
    }
    {
        uint32_t encrstattx=gtmv(m, 32);
        if(!err) ag_drv_xif_sectx_encrypt_get( &encrstattx);
        if(!err) bdmf_session_print(session, "ag_drv_xif_sectx_encrypt_get( %u)\n", encrstattx);
    }
    {
        uint8_t sectxindxwtpktnummax=gtmv(m, 5);
        uint8_t secrxindxwtpktnumabort=gtmv(m, 5);
        if(!err) ag_drv_xif_ae_pktnum_stat_get( &sectxindxwtpktnummax, &secrxindxwtpktnumabort);
        if(!err) bdmf_session_print(session, "ag_drv_xif_ae_pktnum_stat_get( %u %u)\n", sectxindxwtpktnummax, secrxindxwtpktnumabort);
    }
    {
        uint32_t mpcpupdperiod=gtmv(m, 32);
        if(!err) ag_drv_xif_mpcp_update_get( &mpcpupdperiod);
        if(!err) bdmf_session_print(session, "ag_drv_xif_mpcp_update_get( %u)\n", mpcpupdperiod);
    }
    {
        uint32_t cfgburstprelaunchoffset=gtmv(m, 32);
        if(!err) bdmf_session_print(session, "ag_drv_xif_burst_prelaunch_offset_set( %u)\n", cfgburstprelaunchoffset);
        if(!err) ag_drv_xif_burst_prelaunch_offset_set(cfgburstprelaunchoffset);
        if(!err) ag_drv_xif_burst_prelaunch_offset_get( &cfgburstprelaunchoffset);
        if(!err) bdmf_session_print(session, "ag_drv_xif_burst_prelaunch_offset_get( %u)\n", cfgburstprelaunchoffset);
        if(err || cfgburstprelaunchoffset!=gtmv(m, 32))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint16_t cfgvlantype=gtmv(m, 16);
        if(!err) bdmf_session_print(session, "ag_drv_xif_vlan_type_set( %u)\n", cfgvlantype);
        if(!err) ag_drv_xif_vlan_type_set(cfgvlantype);
        if(!err) ag_drv_xif_vlan_type_get( &cfgvlantype);
        if(!err) bdmf_session_print(session, "ag_drv_xif_vlan_type_get( %u)\n", cfgvlantype);
        if(err || cfgvlantype!=gtmv(m, 16))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint16_t cfgp2pscien=gtmv(m, 16);
        if(!err) bdmf_session_print(session, "ag_drv_xif_p2p_ae_sci_en_set( %u)\n", cfgp2pscien);
        if(!err) ag_drv_xif_p2p_ae_sci_en_set(cfgp2pscien);
        if(!err) ag_drv_xif_p2p_ae_sci_en_get( &cfgp2pscien);
        if(!err) bdmf_session_print(session, "ag_drv_xif_p2p_ae_sci_en_get( %u)\n", cfgp2pscien);
        if(err || cfgp2pscien!=gtmv(m, 16))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint32_t cfgp2psci_lo_0=gtmv(m, 32);
        if(!err) bdmf_session_print(session, "ag_drv_xif_p2p_ae_sci_lo_0_set( %u)\n", cfgp2psci_lo_0);
        if(!err) ag_drv_xif_p2p_ae_sci_lo_0_set(cfgp2psci_lo_0);
        if(!err) ag_drv_xif_p2p_ae_sci_lo_0_get( &cfgp2psci_lo_0);
        if(!err) bdmf_session_print(session, "ag_drv_xif_p2p_ae_sci_lo_0_get( %u)\n", cfgp2psci_lo_0);
        if(err || cfgp2psci_lo_0!=gtmv(m, 32))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint32_t cfgp2psci_hi_0=gtmv(m, 32);
        if(!err) bdmf_session_print(session, "ag_drv_xif_p2p_ae_sci_hi_0_set( %u)\n", cfgp2psci_hi_0);
        if(!err) ag_drv_xif_p2p_ae_sci_hi_0_set(cfgp2psci_hi_0);
        if(!err) ag_drv_xif_p2p_ae_sci_hi_0_get( &cfgp2psci_hi_0);
        if(!err) bdmf_session_print(session, "ag_drv_xif_p2p_ae_sci_hi_0_get( %u)\n", cfgp2psci_hi_0);
        if(err || cfgp2psci_hi_0!=gtmv(m, 32))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint32_t cfgp2psci_lo_1=gtmv(m, 32);
        if(!err) bdmf_session_print(session, "ag_drv_xif_p2p_ae_sci_lo_1_set( %u)\n", cfgp2psci_lo_1);
        if(!err) ag_drv_xif_p2p_ae_sci_lo_1_set(cfgp2psci_lo_1);
        if(!err) ag_drv_xif_p2p_ae_sci_lo_1_get( &cfgp2psci_lo_1);
        if(!err) bdmf_session_print(session, "ag_drv_xif_p2p_ae_sci_lo_1_get( %u)\n", cfgp2psci_lo_1);
        if(err || cfgp2psci_lo_1!=gtmv(m, 32))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint32_t cfgp2psci_hi_1=gtmv(m, 32);
        if(!err) bdmf_session_print(session, "ag_drv_xif_p2p_ae_sci_hi_1_set( %u)\n", cfgp2psci_hi_1);
        if(!err) ag_drv_xif_p2p_ae_sci_hi_1_set(cfgp2psci_hi_1);
        if(!err) ag_drv_xif_p2p_ae_sci_hi_1_get( &cfgp2psci_hi_1);
        if(!err) bdmf_session_print(session, "ag_drv_xif_p2p_ae_sci_hi_1_get( %u)\n", cfgp2psci_hi_1);
        if(err || cfgp2psci_hi_1!=gtmv(m, 32))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint32_t cfgp2psci_lo_2=gtmv(m, 32);
        if(!err) bdmf_session_print(session, "ag_drv_xif_p2p_ae_sci_lo_2_set( %u)\n", cfgp2psci_lo_2);
        if(!err) ag_drv_xif_p2p_ae_sci_lo_2_set(cfgp2psci_lo_2);
        if(!err) ag_drv_xif_p2p_ae_sci_lo_2_get( &cfgp2psci_lo_2);
        if(!err) bdmf_session_print(session, "ag_drv_xif_p2p_ae_sci_lo_2_get( %u)\n", cfgp2psci_lo_2);
        if(err || cfgp2psci_lo_2!=gtmv(m, 32))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint32_t cfgp2psci_hi_2=gtmv(m, 32);
        if(!err) bdmf_session_print(session, "ag_drv_xif_p2p_ae_sci_hi_2_set( %u)\n", cfgp2psci_hi_2);
        if(!err) ag_drv_xif_p2p_ae_sci_hi_2_set(cfgp2psci_hi_2);
        if(!err) ag_drv_xif_p2p_ae_sci_hi_2_get( &cfgp2psci_hi_2);
        if(!err) bdmf_session_print(session, "ag_drv_xif_p2p_ae_sci_hi_2_get( %u)\n", cfgp2psci_hi_2);
        if(err || cfgp2psci_hi_2!=gtmv(m, 32))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint32_t cfgp2psci_lo_3=gtmv(m, 32);
        if(!err) bdmf_session_print(session, "ag_drv_xif_p2p_ae_sci_lo_3_set( %u)\n", cfgp2psci_lo_3);
        if(!err) ag_drv_xif_p2p_ae_sci_lo_3_set(cfgp2psci_lo_3);
        if(!err) ag_drv_xif_p2p_ae_sci_lo_3_get( &cfgp2psci_lo_3);
        if(!err) bdmf_session_print(session, "ag_drv_xif_p2p_ae_sci_lo_3_get( %u)\n", cfgp2psci_lo_3);
        if(err || cfgp2psci_lo_3!=gtmv(m, 32))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint32_t cfgp2psci_hi_3=gtmv(m, 32);
        if(!err) bdmf_session_print(session, "ag_drv_xif_p2p_ae_sci_hi_3_set( %u)\n", cfgp2psci_hi_3);
        if(!err) ag_drv_xif_p2p_ae_sci_hi_3_set(cfgp2psci_hi_3);
        if(!err) ag_drv_xif_p2p_ae_sci_hi_3_get( &cfgp2psci_hi_3);
        if(!err) bdmf_session_print(session, "ag_drv_xif_p2p_ae_sci_hi_3_get( %u)\n", cfgp2psci_hi_3);
        if(err || cfgp2psci_hi_3!=gtmv(m, 32))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint32_t cfgp2psci_lo_4=gtmv(m, 32);
        if(!err) bdmf_session_print(session, "ag_drv_xif_p2p_ae_sci_lo_4_set( %u)\n", cfgp2psci_lo_4);
        if(!err) ag_drv_xif_p2p_ae_sci_lo_4_set(cfgp2psci_lo_4);
        if(!err) ag_drv_xif_p2p_ae_sci_lo_4_get( &cfgp2psci_lo_4);
        if(!err) bdmf_session_print(session, "ag_drv_xif_p2p_ae_sci_lo_4_get( %u)\n", cfgp2psci_lo_4);
        if(err || cfgp2psci_lo_4!=gtmv(m, 32))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint32_t cfgp2psci_hi_4=gtmv(m, 32);
        if(!err) bdmf_session_print(session, "ag_drv_xif_p2p_ae_sci_hi_4_set( %u)\n", cfgp2psci_hi_4);
        if(!err) ag_drv_xif_p2p_ae_sci_hi_4_set(cfgp2psci_hi_4);
        if(!err) ag_drv_xif_p2p_ae_sci_hi_4_get( &cfgp2psci_hi_4);
        if(!err) bdmf_session_print(session, "ag_drv_xif_p2p_ae_sci_hi_4_get( %u)\n", cfgp2psci_hi_4);
        if(err || cfgp2psci_hi_4!=gtmv(m, 32))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint32_t cfgp2psci_lo_5=gtmv(m, 32);
        if(!err) bdmf_session_print(session, "ag_drv_xif_p2p_ae_sci_lo_5_set( %u)\n", cfgp2psci_lo_5);
        if(!err) ag_drv_xif_p2p_ae_sci_lo_5_set(cfgp2psci_lo_5);
        if(!err) ag_drv_xif_p2p_ae_sci_lo_5_get( &cfgp2psci_lo_5);
        if(!err) bdmf_session_print(session, "ag_drv_xif_p2p_ae_sci_lo_5_get( %u)\n", cfgp2psci_lo_5);
        if(err || cfgp2psci_lo_5!=gtmv(m, 32))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint32_t cfgp2psci_hi_5=gtmv(m, 32);
        if(!err) bdmf_session_print(session, "ag_drv_xif_p2p_ae_sci_hi_5_set( %u)\n", cfgp2psci_hi_5);
        if(!err) ag_drv_xif_p2p_ae_sci_hi_5_set(cfgp2psci_hi_5);
        if(!err) ag_drv_xif_p2p_ae_sci_hi_5_get( &cfgp2psci_hi_5);
        if(!err) bdmf_session_print(session, "ag_drv_xif_p2p_ae_sci_hi_5_get( %u)\n", cfgp2psci_hi_5);
        if(err || cfgp2psci_hi_5!=gtmv(m, 32))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint32_t cfgp2psci_lo_6=gtmv(m, 32);
        if(!err) bdmf_session_print(session, "ag_drv_xif_p2p_ae_sci_lo_6_set( %u)\n", cfgp2psci_lo_6);
        if(!err) ag_drv_xif_p2p_ae_sci_lo_6_set(cfgp2psci_lo_6);
        if(!err) ag_drv_xif_p2p_ae_sci_lo_6_get( &cfgp2psci_lo_6);
        if(!err) bdmf_session_print(session, "ag_drv_xif_p2p_ae_sci_lo_6_get( %u)\n", cfgp2psci_lo_6);
        if(err || cfgp2psci_lo_6!=gtmv(m, 32))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint32_t cfgp2psci_hi_6=gtmv(m, 32);
        if(!err) bdmf_session_print(session, "ag_drv_xif_p2p_ae_sci_hi_6_set( %u)\n", cfgp2psci_hi_6);
        if(!err) ag_drv_xif_p2p_ae_sci_hi_6_set(cfgp2psci_hi_6);
        if(!err) ag_drv_xif_p2p_ae_sci_hi_6_get( &cfgp2psci_hi_6);
        if(!err) bdmf_session_print(session, "ag_drv_xif_p2p_ae_sci_hi_6_get( %u)\n", cfgp2psci_hi_6);
        if(err || cfgp2psci_hi_6!=gtmv(m, 32))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint32_t cfgp2psci_lo_7=gtmv(m, 32);
        if(!err) bdmf_session_print(session, "ag_drv_xif_p2p_ae_sci_lo_7_set( %u)\n", cfgp2psci_lo_7);
        if(!err) ag_drv_xif_p2p_ae_sci_lo_7_set(cfgp2psci_lo_7);
        if(!err) ag_drv_xif_p2p_ae_sci_lo_7_get( &cfgp2psci_lo_7);
        if(!err) bdmf_session_print(session, "ag_drv_xif_p2p_ae_sci_lo_7_get( %u)\n", cfgp2psci_lo_7);
        if(err || cfgp2psci_lo_7!=gtmv(m, 32))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint32_t cfgp2psci_hi_7=gtmv(m, 32);
        if(!err) bdmf_session_print(session, "ag_drv_xif_p2p_ae_sci_hi_7_set( %u)\n", cfgp2psci_hi_7);
        if(!err) ag_drv_xif_p2p_ae_sci_hi_7_set(cfgp2psci_hi_7);
        if(!err) ag_drv_xif_p2p_ae_sci_hi_7_get( &cfgp2psci_hi_7);
        if(!err) bdmf_session_print(session, "ag_drv_xif_p2p_ae_sci_hi_7_get( %u)\n", cfgp2psci_hi_7);
        if(err || cfgp2psci_hi_7!=gtmv(m, 32))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint32_t cfgp2psci_lo_8=gtmv(m, 32);
        if(!err) bdmf_session_print(session, "ag_drv_xif_p2p_ae_sci_lo_8_set( %u)\n", cfgp2psci_lo_8);
        if(!err) ag_drv_xif_p2p_ae_sci_lo_8_set(cfgp2psci_lo_8);
        if(!err) ag_drv_xif_p2p_ae_sci_lo_8_get( &cfgp2psci_lo_8);
        if(!err) bdmf_session_print(session, "ag_drv_xif_p2p_ae_sci_lo_8_get( %u)\n", cfgp2psci_lo_8);
        if(err || cfgp2psci_lo_8!=gtmv(m, 32))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint32_t cfgp2psci_hi_8=gtmv(m, 32);
        if(!err) bdmf_session_print(session, "ag_drv_xif_p2p_ae_sci_hi_8_set( %u)\n", cfgp2psci_hi_8);
        if(!err) ag_drv_xif_p2p_ae_sci_hi_8_set(cfgp2psci_hi_8);
        if(!err) ag_drv_xif_p2p_ae_sci_hi_8_get( &cfgp2psci_hi_8);
        if(!err) bdmf_session_print(session, "ag_drv_xif_p2p_ae_sci_hi_8_get( %u)\n", cfgp2psci_hi_8);
        if(err || cfgp2psci_hi_8!=gtmv(m, 32))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint32_t cfgp2psci_lo_9=gtmv(m, 32);
        if(!err) bdmf_session_print(session, "ag_drv_xif_p2p_ae_sci_lo_9_set( %u)\n", cfgp2psci_lo_9);
        if(!err) ag_drv_xif_p2p_ae_sci_lo_9_set(cfgp2psci_lo_9);
        if(!err) ag_drv_xif_p2p_ae_sci_lo_9_get( &cfgp2psci_lo_9);
        if(!err) bdmf_session_print(session, "ag_drv_xif_p2p_ae_sci_lo_9_get( %u)\n", cfgp2psci_lo_9);
        if(err || cfgp2psci_lo_9!=gtmv(m, 32))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint32_t cfgp2psci_hi_9=gtmv(m, 32);
        if(!err) bdmf_session_print(session, "ag_drv_xif_p2p_ae_sci_hi_9_set( %u)\n", cfgp2psci_hi_9);
        if(!err) ag_drv_xif_p2p_ae_sci_hi_9_set(cfgp2psci_hi_9);
        if(!err) ag_drv_xif_p2p_ae_sci_hi_9_get( &cfgp2psci_hi_9);
        if(!err) bdmf_session_print(session, "ag_drv_xif_p2p_ae_sci_hi_9_get( %u)\n", cfgp2psci_hi_9);
        if(err || cfgp2psci_hi_9!=gtmv(m, 32))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint32_t cfgp2psci_lo_10=gtmv(m, 32);
        if(!err) bdmf_session_print(session, "ag_drv_xif_p2p_ae_sci_lo_10_set( %u)\n", cfgp2psci_lo_10);
        if(!err) ag_drv_xif_p2p_ae_sci_lo_10_set(cfgp2psci_lo_10);
        if(!err) ag_drv_xif_p2p_ae_sci_lo_10_get( &cfgp2psci_lo_10);
        if(!err) bdmf_session_print(session, "ag_drv_xif_p2p_ae_sci_lo_10_get( %u)\n", cfgp2psci_lo_10);
        if(err || cfgp2psci_lo_10!=gtmv(m, 32))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint32_t cfgp2psci_hi_10=gtmv(m, 32);
        if(!err) bdmf_session_print(session, "ag_drv_xif_p2p_ae_sci_hi_10_set( %u)\n", cfgp2psci_hi_10);
        if(!err) ag_drv_xif_p2p_ae_sci_hi_10_set(cfgp2psci_hi_10);
        if(!err) ag_drv_xif_p2p_ae_sci_hi_10_get( &cfgp2psci_hi_10);
        if(!err) bdmf_session_print(session, "ag_drv_xif_p2p_ae_sci_hi_10_get( %u)\n", cfgp2psci_hi_10);
        if(err || cfgp2psci_hi_10!=gtmv(m, 32))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint32_t cfgp2psci_lo_11=gtmv(m, 32);
        if(!err) bdmf_session_print(session, "ag_drv_xif_p2p_ae_sci_lo_11_set( %u)\n", cfgp2psci_lo_11);
        if(!err) ag_drv_xif_p2p_ae_sci_lo_11_set(cfgp2psci_lo_11);
        if(!err) ag_drv_xif_p2p_ae_sci_lo_11_get( &cfgp2psci_lo_11);
        if(!err) bdmf_session_print(session, "ag_drv_xif_p2p_ae_sci_lo_11_get( %u)\n", cfgp2psci_lo_11);
        if(err || cfgp2psci_lo_11!=gtmv(m, 32))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint32_t cfgp2psci_hi_11=gtmv(m, 32);
        if(!err) bdmf_session_print(session, "ag_drv_xif_p2p_ae_sci_hi_11_set( %u)\n", cfgp2psci_hi_11);
        if(!err) ag_drv_xif_p2p_ae_sci_hi_11_set(cfgp2psci_hi_11);
        if(!err) ag_drv_xif_p2p_ae_sci_hi_11_get( &cfgp2psci_hi_11);
        if(!err) bdmf_session_print(session, "ag_drv_xif_p2p_ae_sci_hi_11_get( %u)\n", cfgp2psci_hi_11);
        if(err || cfgp2psci_hi_11!=gtmv(m, 32))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint32_t cfgp2psci_lo_12=gtmv(m, 32);
        if(!err) bdmf_session_print(session, "ag_drv_xif_p2p_ae_sci_lo_12_set( %u)\n", cfgp2psci_lo_12);
        if(!err) ag_drv_xif_p2p_ae_sci_lo_12_set(cfgp2psci_lo_12);
        if(!err) ag_drv_xif_p2p_ae_sci_lo_12_get( &cfgp2psci_lo_12);
        if(!err) bdmf_session_print(session, "ag_drv_xif_p2p_ae_sci_lo_12_get( %u)\n", cfgp2psci_lo_12);
        if(err || cfgp2psci_lo_12!=gtmv(m, 32))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint32_t cfgp2psci_hi_12=gtmv(m, 32);
        if(!err) bdmf_session_print(session, "ag_drv_xif_p2p_ae_sci_hi_12_set( %u)\n", cfgp2psci_hi_12);
        if(!err) ag_drv_xif_p2p_ae_sci_hi_12_set(cfgp2psci_hi_12);
        if(!err) ag_drv_xif_p2p_ae_sci_hi_12_get( &cfgp2psci_hi_12);
        if(!err) bdmf_session_print(session, "ag_drv_xif_p2p_ae_sci_hi_12_get( %u)\n", cfgp2psci_hi_12);
        if(err || cfgp2psci_hi_12!=gtmv(m, 32))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint32_t cfgp2psci_lo_13=gtmv(m, 32);
        if(!err) bdmf_session_print(session, "ag_drv_xif_p2p_ae_sci_lo_13_set( %u)\n", cfgp2psci_lo_13);
        if(!err) ag_drv_xif_p2p_ae_sci_lo_13_set(cfgp2psci_lo_13);
        if(!err) ag_drv_xif_p2p_ae_sci_lo_13_get( &cfgp2psci_lo_13);
        if(!err) bdmf_session_print(session, "ag_drv_xif_p2p_ae_sci_lo_13_get( %u)\n", cfgp2psci_lo_13);
        if(err || cfgp2psci_lo_13!=gtmv(m, 32))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint32_t cfgp2psci_hi_13=gtmv(m, 32);
        if(!err) bdmf_session_print(session, "ag_drv_xif_p2p_ae_sci_hi_13_set( %u)\n", cfgp2psci_hi_13);
        if(!err) ag_drv_xif_p2p_ae_sci_hi_13_set(cfgp2psci_hi_13);
        if(!err) ag_drv_xif_p2p_ae_sci_hi_13_get( &cfgp2psci_hi_13);
        if(!err) bdmf_session_print(session, "ag_drv_xif_p2p_ae_sci_hi_13_get( %u)\n", cfgp2psci_hi_13);
        if(err || cfgp2psci_hi_13!=gtmv(m, 32))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint32_t cfgp2psci_lo_14=gtmv(m, 32);
        if(!err) bdmf_session_print(session, "ag_drv_xif_p2p_ae_sci_lo_14_set( %u)\n", cfgp2psci_lo_14);
        if(!err) ag_drv_xif_p2p_ae_sci_lo_14_set(cfgp2psci_lo_14);
        if(!err) ag_drv_xif_p2p_ae_sci_lo_14_get( &cfgp2psci_lo_14);
        if(!err) bdmf_session_print(session, "ag_drv_xif_p2p_ae_sci_lo_14_get( %u)\n", cfgp2psci_lo_14);
        if(err || cfgp2psci_lo_14!=gtmv(m, 32))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint32_t cfgp2psci_hi_14=gtmv(m, 32);
        if(!err) bdmf_session_print(session, "ag_drv_xif_p2p_ae_sci_hi_14_set( %u)\n", cfgp2psci_hi_14);
        if(!err) ag_drv_xif_p2p_ae_sci_hi_14_set(cfgp2psci_hi_14);
        if(!err) ag_drv_xif_p2p_ae_sci_hi_14_get( &cfgp2psci_hi_14);
        if(!err) bdmf_session_print(session, "ag_drv_xif_p2p_ae_sci_hi_14_get( %u)\n", cfgp2psci_hi_14);
        if(err || cfgp2psci_hi_14!=gtmv(m, 32))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint32_t cfgp2psci_lo_15=gtmv(m, 32);
        if(!err) bdmf_session_print(session, "ag_drv_xif_p2p_ae_sci_lo_15_set( %u)\n", cfgp2psci_lo_15);
        if(!err) ag_drv_xif_p2p_ae_sci_lo_15_set(cfgp2psci_lo_15);
        if(!err) ag_drv_xif_p2p_ae_sci_lo_15_get( &cfgp2psci_lo_15);
        if(!err) bdmf_session_print(session, "ag_drv_xif_p2p_ae_sci_lo_15_get( %u)\n", cfgp2psci_lo_15);
        if(err || cfgp2psci_lo_15!=gtmv(m, 32))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint32_t cfgp2psci_hi_15=gtmv(m, 32);
        if(!err) bdmf_session_print(session, "ag_drv_xif_p2p_ae_sci_hi_15_set( %u)\n", cfgp2psci_hi_15);
        if(!err) ag_drv_xif_p2p_ae_sci_hi_15_set(cfgp2psci_hi_15);
        if(!err) ag_drv_xif_p2p_ae_sci_hi_15_get( &cfgp2psci_hi_15);
        if(!err) bdmf_session_print(session, "ag_drv_xif_p2p_ae_sci_hi_15_get( %u)\n", cfgp2psci_hi_15);
        if(err || cfgp2psci_hi_15!=gtmv(m, 32))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint32_t keystattx_hi=gtmv(m, 32);
        if(!err) ag_drv_xif_sectx_keynum_1_get( &keystattx_hi);
        if(!err) bdmf_session_print(session, "ag_drv_xif_sectx_keynum_1_get( %u)\n", keystattx_hi);
    }
    {
        uint32_t keystatrx_hi=gtmv(m, 32);
        if(!err) ag_drv_xif_secrx_keynum_1_get( &keystatrx_hi);
        if(!err) bdmf_session_print(session, "ag_drv_xif_secrx_keynum_1_get( %u)\n", keystatrx_hi);
    }
    return err;
}

static int bcm_xif_cli_address(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
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
    case bdmf_address_ctl : reg = &RU_REG(XIF, CTL); blk = &RU_BLK(XIF); break;
    case bdmf_address_int_status : reg = &RU_REG(XIF, INT_STATUS); blk = &RU_BLK(XIF); break;
    case bdmf_address_int_mask : reg = &RU_REG(XIF, INT_MASK); blk = &RU_BLK(XIF); break;
    case bdmf_address_port_command : reg = &RU_REG(XIF, PORT_COMMAND); blk = &RU_BLK(XIF); break;
    case bdmf_address_port_data_ : reg = &RU_REG(XIF, PORT_DATA_); blk = &RU_BLK(XIF); break;
    case bdmf_address_macsec : reg = &RU_REG(XIF, MACSEC); blk = &RU_BLK(XIF); break;
    case bdmf_address_xpn_xmt_offset : reg = &RU_REG(XIF, XPN_XMT_OFFSET); blk = &RU_BLK(XIF); break;
    case bdmf_address_xpn_timestamp_offset : reg = &RU_REG(XIF, XPN_TIMESTAMP_OFFSET); blk = &RU_BLK(XIF); break;
    case bdmf_address_xpn_pktgen_ctl : reg = &RU_REG(XIF, XPN_PKTGEN_CTL); blk = &RU_BLK(XIF); break;
    case bdmf_address_xpn_pktgen_llid : reg = &RU_REG(XIF, XPN_PKTGEN_LLID); blk = &RU_BLK(XIF); break;
    case bdmf_address_xpn_pktgen_pkt_cnt : reg = &RU_REG(XIF, XPN_PKTGEN_PKT_CNT); blk = &RU_BLK(XIF); break;
    case bdmf_address_xpn_pktgen_pkt_size : reg = &RU_REG(XIF, XPN_PKTGEN_PKT_SIZE); blk = &RU_BLK(XIF); break;
    case bdmf_address_xpn_pktgen_ipg : reg = &RU_REG(XIF, XPN_PKTGEN_IPG); blk = &RU_BLK(XIF); break;
    case bdmf_address_ts_jitter_thresh : reg = &RU_REG(XIF, TS_JITTER_THRESH); blk = &RU_BLK(XIF); break;
    case bdmf_address_ts_update : reg = &RU_REG(XIF, TS_UPDATE); blk = &RU_BLK(XIF); break;
    case bdmf_address_gnt_overhead : reg = &RU_REG(XIF, GNT_OVERHEAD); blk = &RU_BLK(XIF); break;
    case bdmf_address_discover_overhead : reg = &RU_REG(XIF, DISCOVER_OVERHEAD); blk = &RU_BLK(XIF); break;
    case bdmf_address_discover_info : reg = &RU_REG(XIF, DISCOVER_INFO); blk = &RU_BLK(XIF); break;
    case bdmf_address_xpn_oversize_thresh : reg = &RU_REG(XIF, XPN_OVERSIZE_THRESH); blk = &RU_BLK(XIF); break;
    case bdmf_address_secrx_keynum : reg = &RU_REG(XIF, SECRX_KEYNUM); blk = &RU_BLK(XIF); break;
    case bdmf_address_secrx_encrypt : reg = &RU_REG(XIF, SECRX_ENCRYPT); blk = &RU_BLK(XIF); break;
    case bdmf_address_pmc_frame_rx_cnt : reg = &RU_REG(XIF, PMC_FRAME_RX_CNT); blk = &RU_BLK(XIF); break;
    case bdmf_address_pmc_byte_rx_cnt : reg = &RU_REG(XIF, PMC_BYTE_RX_CNT); blk = &RU_BLK(XIF); break;
    case bdmf_address_pmc_runt_rx_cnt : reg = &RU_REG(XIF, PMC_RUNT_RX_CNT); blk = &RU_BLK(XIF); break;
    case bdmf_address_pmc_cw_err_rx_cnt : reg = &RU_REG(XIF, PMC_CW_ERR_RX_CNT); blk = &RU_BLK(XIF); break;
    case bdmf_address_pmc_crc8_err_rx_cnt : reg = &RU_REG(XIF, PMC_CRC8_ERR_RX_CNT); blk = &RU_BLK(XIF); break;
    case bdmf_address_xpn_data_frm_cnt : reg = &RU_REG(XIF, XPN_DATA_FRM_CNT); blk = &RU_BLK(XIF); break;
    case bdmf_address_xpn_data_byte_cnt : reg = &RU_REG(XIF, XPN_DATA_BYTE_CNT); blk = &RU_BLK(XIF); break;
    case bdmf_address_xpn_mpcp_frm_cnt : reg = &RU_REG(XIF, XPN_MPCP_FRM_CNT); blk = &RU_BLK(XIF); break;
    case bdmf_address_xpn_oam_frm_cnt : reg = &RU_REG(XIF, XPN_OAM_FRM_CNT); blk = &RU_BLK(XIF); break;
    case bdmf_address_xpn_oam_byte_cnt : reg = &RU_REG(XIF, XPN_OAM_BYTE_CNT); blk = &RU_BLK(XIF); break;
    case bdmf_address_xpn_oversize_frm_cnt : reg = &RU_REG(XIF, XPN_OVERSIZE_FRM_CNT); blk = &RU_BLK(XIF); break;
    case bdmf_address_sec_abort_frm_cnt : reg = &RU_REG(XIF, SEC_ABORT_FRM_CNT); blk = &RU_BLK(XIF); break;
    case bdmf_address_pmc_tx_neg_event_cnt : reg = &RU_REG(XIF, PMC_TX_NEG_EVENT_CNT); blk = &RU_BLK(XIF); break;
    case bdmf_address_xpn_idle_pkt_cnt : reg = &RU_REG(XIF, XPN_IDLE_PKT_CNT); blk = &RU_BLK(XIF); break;
    case bdmf_address_llid_0 : reg = &RU_REG(XIF, LLID_0); blk = &RU_BLK(XIF); break;
    case bdmf_address_llid_1 : reg = &RU_REG(XIF, LLID_1); blk = &RU_BLK(XIF); break;
    case bdmf_address_llid_2 : reg = &RU_REG(XIF, LLID_2); blk = &RU_BLK(XIF); break;
    case bdmf_address_llid_3 : reg = &RU_REG(XIF, LLID_3); blk = &RU_BLK(XIF); break;
    case bdmf_address_llid_4 : reg = &RU_REG(XIF, LLID_4); blk = &RU_BLK(XIF); break;
    case bdmf_address_llid_5 : reg = &RU_REG(XIF, LLID_5); blk = &RU_BLK(XIF); break;
    case bdmf_address_llid_6 : reg = &RU_REG(XIF, LLID_6); blk = &RU_BLK(XIF); break;
    case bdmf_address_llid_7 : reg = &RU_REG(XIF, LLID_7); blk = &RU_BLK(XIF); break;
    case bdmf_address_llid_8 : reg = &RU_REG(XIF, LLID_8); blk = &RU_BLK(XIF); break;
    case bdmf_address_llid_9 : reg = &RU_REG(XIF, LLID_9); blk = &RU_BLK(XIF); break;
    case bdmf_address_llid_10 : reg = &RU_REG(XIF, LLID_10); blk = &RU_BLK(XIF); break;
    case bdmf_address_llid_11 : reg = &RU_REG(XIF, LLID_11); blk = &RU_BLK(XIF); break;
    case bdmf_address_llid_12 : reg = &RU_REG(XIF, LLID_12); blk = &RU_BLK(XIF); break;
    case bdmf_address_llid_13 : reg = &RU_REG(XIF, LLID_13); blk = &RU_BLK(XIF); break;
    case bdmf_address_llid_14 : reg = &RU_REG(XIF, LLID_14); blk = &RU_BLK(XIF); break;
    case bdmf_address_llid_15 : reg = &RU_REG(XIF, LLID_15); blk = &RU_BLK(XIF); break;
    case bdmf_address_llid_16 : reg = &RU_REG(XIF, LLID_16); blk = &RU_BLK(XIF); break;
    case bdmf_address_llid_17 : reg = &RU_REG(XIF, LLID_17); blk = &RU_BLK(XIF); break;
    case bdmf_address_llid_18 : reg = &RU_REG(XIF, LLID_18); blk = &RU_BLK(XIF); break;
    case bdmf_address_llid_19 : reg = &RU_REG(XIF, LLID_19); blk = &RU_BLK(XIF); break;
    case bdmf_address_llid_20 : reg = &RU_REG(XIF, LLID_20); blk = &RU_BLK(XIF); break;
    case bdmf_address_llid_21 : reg = &RU_REG(XIF, LLID_21); blk = &RU_BLK(XIF); break;
    case bdmf_address_llid_22 : reg = &RU_REG(XIF, LLID_22); blk = &RU_BLK(XIF); break;
    case bdmf_address_llid_23 : reg = &RU_REG(XIF, LLID_23); blk = &RU_BLK(XIF); break;
    case bdmf_address_llid_24 : reg = &RU_REG(XIF, LLID_24); blk = &RU_BLK(XIF); break;
    case bdmf_address_llid_25 : reg = &RU_REG(XIF, LLID_25); blk = &RU_BLK(XIF); break;
    case bdmf_address_llid_26 : reg = &RU_REG(XIF, LLID_26); blk = &RU_BLK(XIF); break;
    case bdmf_address_llid_27 : reg = &RU_REG(XIF, LLID_27); blk = &RU_BLK(XIF); break;
    case bdmf_address_llid_28 : reg = &RU_REG(XIF, LLID_28); blk = &RU_BLK(XIF); break;
    case bdmf_address_llid_29 : reg = &RU_REG(XIF, LLID_29); blk = &RU_BLK(XIF); break;
    case bdmf_address_llid_30 : reg = &RU_REG(XIF, LLID_30); blk = &RU_BLK(XIF); break;
    case bdmf_address_llid_31 : reg = &RU_REG(XIF, LLID_31); blk = &RU_BLK(XIF); break;
    case bdmf_address_max_mpcp_update : reg = &RU_REG(XIF, MAX_MPCP_UPDATE); blk = &RU_BLK(XIF); break;
    case bdmf_address_ipg_insertion : reg = &RU_REG(XIF, IPG_INSERTION); blk = &RU_BLK(XIF); break;
    case bdmf_address_transport_time : reg = &RU_REG(XIF, TRANSPORT_TIME); blk = &RU_BLK(XIF); break;
    case bdmf_address_mpcp_time : reg = &RU_REG(XIF, MPCP_TIME); blk = &RU_BLK(XIF); break;
    case bdmf_address_overlap_gnt_oh : reg = &RU_REG(XIF, OVERLAP_GNT_OH); blk = &RU_BLK(XIF); break;
    case bdmf_address_mac_mode : reg = &RU_REG(XIF, MAC_MODE); blk = &RU_BLK(XIF); break;
    case bdmf_address_pmctx_ctl : reg = &RU_REG(XIF, PMCTX_CTL); blk = &RU_BLK(XIF); break;
    case bdmf_address_sec_ctl : reg = &RU_REG(XIF, SEC_CTL); blk = &RU_BLK(XIF); break;
    case bdmf_address_ae_pktnum_window : reg = &RU_REG(XIF, AE_PKTNUM_WINDOW); blk = &RU_BLK(XIF); break;
    case bdmf_address_ae_pktnum_thresh : reg = &RU_REG(XIF, AE_PKTNUM_THRESH); blk = &RU_BLK(XIF); break;
    case bdmf_address_sectx_keynum : reg = &RU_REG(XIF, SECTX_KEYNUM); blk = &RU_BLK(XIF); break;
    case bdmf_address_sectx_encrypt : reg = &RU_REG(XIF, SECTX_ENCRYPT); blk = &RU_BLK(XIF); break;
    case bdmf_address_ae_pktnum_stat : reg = &RU_REG(XIF, AE_PKTNUM_STAT); blk = &RU_BLK(XIF); break;
    case bdmf_address_mpcp_update : reg = &RU_REG(XIF, MPCP_UPDATE); blk = &RU_BLK(XIF); break;
    case bdmf_address_burst_prelaunch_offset : reg = &RU_REG(XIF, BURST_PRELAUNCH_OFFSET); blk = &RU_BLK(XIF); break;
    case bdmf_address_vlan_type : reg = &RU_REG(XIF, VLAN_TYPE); blk = &RU_BLK(XIF); break;
    case bdmf_address_p2p_ae_sci_en : reg = &RU_REG(XIF, P2P_AE_SCI_EN); blk = &RU_BLK(XIF); break;
    case bdmf_address_p2p_ae_sci_lo_0 : reg = &RU_REG(XIF, P2P_AE_SCI_LO_0); blk = &RU_BLK(XIF); break;
    case bdmf_address_p2p_ae_sci_hi_0 : reg = &RU_REG(XIF, P2P_AE_SCI_HI_0); blk = &RU_BLK(XIF); break;
    case bdmf_address_p2p_ae_sci_lo_1 : reg = &RU_REG(XIF, P2P_AE_SCI_LO_1); blk = &RU_BLK(XIF); break;
    case bdmf_address_p2p_ae_sci_hi_1 : reg = &RU_REG(XIF, P2P_AE_SCI_HI_1); blk = &RU_BLK(XIF); break;
    case bdmf_address_p2p_ae_sci_lo_2 : reg = &RU_REG(XIF, P2P_AE_SCI_LO_2); blk = &RU_BLK(XIF); break;
    case bdmf_address_p2p_ae_sci_hi_2 : reg = &RU_REG(XIF, P2P_AE_SCI_HI_2); blk = &RU_BLK(XIF); break;
    case bdmf_address_p2p_ae_sci_lo_3 : reg = &RU_REG(XIF, P2P_AE_SCI_LO_3); blk = &RU_BLK(XIF); break;
    case bdmf_address_p2p_ae_sci_hi_3 : reg = &RU_REG(XIF, P2P_AE_SCI_HI_3); blk = &RU_BLK(XIF); break;
    case bdmf_address_p2p_ae_sci_lo_4 : reg = &RU_REG(XIF, P2P_AE_SCI_LO_4); blk = &RU_BLK(XIF); break;
    case bdmf_address_p2p_ae_sci_hi_4 : reg = &RU_REG(XIF, P2P_AE_SCI_HI_4); blk = &RU_BLK(XIF); break;
    case bdmf_address_p2p_ae_sci_lo_5 : reg = &RU_REG(XIF, P2P_AE_SCI_LO_5); blk = &RU_BLK(XIF); break;
    case bdmf_address_p2p_ae_sci_hi_5 : reg = &RU_REG(XIF, P2P_AE_SCI_HI_5); blk = &RU_BLK(XIF); break;
    case bdmf_address_p2p_ae_sci_lo_6 : reg = &RU_REG(XIF, P2P_AE_SCI_LO_6); blk = &RU_BLK(XIF); break;
    case bdmf_address_p2p_ae_sci_hi_6 : reg = &RU_REG(XIF, P2P_AE_SCI_HI_6); blk = &RU_BLK(XIF); break;
    case bdmf_address_p2p_ae_sci_lo_7 : reg = &RU_REG(XIF, P2P_AE_SCI_LO_7); blk = &RU_BLK(XIF); break;
    case bdmf_address_p2p_ae_sci_hi_7 : reg = &RU_REG(XIF, P2P_AE_SCI_HI_7); blk = &RU_BLK(XIF); break;
    case bdmf_address_p2p_ae_sci_lo_8 : reg = &RU_REG(XIF, P2P_AE_SCI_LO_8); blk = &RU_BLK(XIF); break;
    case bdmf_address_p2p_ae_sci_hi_8 : reg = &RU_REG(XIF, P2P_AE_SCI_HI_8); blk = &RU_BLK(XIF); break;
    case bdmf_address_p2p_ae_sci_lo_9 : reg = &RU_REG(XIF, P2P_AE_SCI_LO_9); blk = &RU_BLK(XIF); break;
    case bdmf_address_p2p_ae_sci_hi_9 : reg = &RU_REG(XIF, P2P_AE_SCI_HI_9); blk = &RU_BLK(XIF); break;
    case bdmf_address_p2p_ae_sci_lo_10 : reg = &RU_REG(XIF, P2P_AE_SCI_LO_10); blk = &RU_BLK(XIF); break;
    case bdmf_address_p2p_ae_sci_hi_10 : reg = &RU_REG(XIF, P2P_AE_SCI_HI_10); blk = &RU_BLK(XIF); break;
    case bdmf_address_p2p_ae_sci_lo_11 : reg = &RU_REG(XIF, P2P_AE_SCI_LO_11); blk = &RU_BLK(XIF); break;
    case bdmf_address_p2p_ae_sci_hi_11 : reg = &RU_REG(XIF, P2P_AE_SCI_HI_11); blk = &RU_BLK(XIF); break;
    case bdmf_address_p2p_ae_sci_lo_12 : reg = &RU_REG(XIF, P2P_AE_SCI_LO_12); blk = &RU_BLK(XIF); break;
    case bdmf_address_p2p_ae_sci_hi_12 : reg = &RU_REG(XIF, P2P_AE_SCI_HI_12); blk = &RU_BLK(XIF); break;
    case bdmf_address_p2p_ae_sci_lo_13 : reg = &RU_REG(XIF, P2P_AE_SCI_LO_13); blk = &RU_BLK(XIF); break;
    case bdmf_address_p2p_ae_sci_hi_13 : reg = &RU_REG(XIF, P2P_AE_SCI_HI_13); blk = &RU_BLK(XIF); break;
    case bdmf_address_p2p_ae_sci_lo_14 : reg = &RU_REG(XIF, P2P_AE_SCI_LO_14); blk = &RU_BLK(XIF); break;
    case bdmf_address_p2p_ae_sci_hi_14 : reg = &RU_REG(XIF, P2P_AE_SCI_HI_14); blk = &RU_BLK(XIF); break;
    case bdmf_address_p2p_ae_sci_lo_15 : reg = &RU_REG(XIF, P2P_AE_SCI_LO_15); blk = &RU_BLK(XIF); break;
    case bdmf_address_p2p_ae_sci_hi_15 : reg = &RU_REG(XIF, P2P_AE_SCI_HI_15); blk = &RU_BLK(XIF); break;
    case bdmf_address_sectx_keynum_1 : reg = &RU_REG(XIF, SECTX_KEYNUM_1); blk = &RU_BLK(XIF); break;
    case bdmf_address_secrx_keynum_1 : reg = &RU_REG(XIF, SECRX_KEYNUM_1); blk = &RU_BLK(XIF); break;
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

bdmfmon_handle_t ag_drv_xif_cli_init(bdmfmon_handle_t driver_dir)
{
    bdmfmon_handle_t dir;

    if ((dir = bdmfmon_dir_find(driver_dir, "xif"))!=NULL)
        return dir;
    dir = bdmfmon_dir_add(driver_dir, "xif", "xif", BDMF_ACCESS_ADMIN, NULL);

    {
        static bdmfmon_cmd_parm_t set_ctl[]={
            BDMFMON_MAKE_PARM("rxencrypten", "rxencrypten", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("cfgdisrxdasaencrpt", "cfgdisrxdasaencrpt", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("rxencryptmode", "rxencryptmode", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("txencrypten", "txencrypten", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("cfgdistxdasaencrpt", "cfgdistxdasaencrpt", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("txencryptmode", "txencryptmode", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("cfgllidmodemsk", "cfgllidmodemsk", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("cfgxpnbadcrc32", "cfgxpnbadcrc32", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("cfgdisdiscinfo", "cfgdisdiscinfo", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("cfgpmctx2rxlpbk", "cfgpmctx2rxlpbk", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("cfgpmctxencrc8bad", "cfgpmctxencrc8bad", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("cfgenp2p", "cfgenp2p", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("cfgllidpromiscuousmode", "cfgllidpromiscuousmode", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("cfgenidlepktsup", "cfgenidlepktsup", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("cfgpmcrxencrc8chk", "cfgpmcrxencrc8chk", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("cfgen1stidlepktconvert", "cfgen1stidlepktconvert", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("cfgfecen", "cfgfecen", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("cfglegacyrcvtsupd", "cfglegacyrcvtsupd", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("cfgxpnencrcpassthru", "cfgxpnencrcpassthru", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("cfgxpndistimestampmod", "cfgxpndistimestampmod", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("xifnotrdy", "xifnotrdy", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("xifdtportrstn", "xifdtportrstn", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("xpntxrstn", "xpntxrstn", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("pmctxrstn", "pmctxrstn", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("sectxrstn", "sectxrstn", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("cfgdistxoamencrpt", "cfgdistxoamencrpt", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("cfgdistxmpcpencrpt", "cfgdistxmpcpencrpt", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("pmcrxrstn", "pmcrxrstn", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("secrxrstn", "secrxrstn", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_int_status[]={
            BDMFMON_MAKE_PARM("secrxrplyprtctabrtint", "secrxrplyprtctabrtint", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("sectxpktnummaxint", "sectxpktnummaxint", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("tsfullupdint", "tsfullupdint", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("txhangint", "txhangint", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("negtimeint", "negtimeint", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("pmctsjttrint", "pmctsjttrint", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("secrxoutffovrflwint", "secrxoutffovrflwint", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_int_mask[]={
            BDMFMON_MAKE_PARM("msksecrxreplayprotctabort", "msksecrxreplayprotctabort", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("mskpktnumthreshint", "mskpktnumthreshint", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("msktsfullupdint", "msktsfullupdint", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("msktxhangint", "msktxhangint", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("msknegtimeint", "msknegtimeint", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("mskpmctsjttrint", "mskpmctsjttrint", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("msksecrxoutffint", "msksecrxoutffint", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_port_command[]={
            BDMFMON_MAKE_PARM("dataportbusy", "dataportbusy", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("portselect", "portselect", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("portopcode", "portopcode", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("portaddress", "portaddress", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_port_data_[]={
            BDMFMON_MAKE_PARM("portidx", "portidx", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("portdata", "portdata", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_macsec[]={
            BDMFMON_MAKE_PARM("cfgmacsecethertype", "cfgmacsecethertype", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_xpn_xmt_offset[]={
            BDMFMON_MAKE_PARM("cfgxpnxmtoffset", "cfgxpnxmtoffset", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_xpn_timestamp_offset[]={
            BDMFMON_MAKE_PARM("cfgxpnmpcptsoffset", "cfgxpnmpcptsoffset", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_xpn_pktgen_ctl[]={
            BDMFMON_MAKE_PARM("cfgonuburstsize", "cfgonuburstsize", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("cfgenbck2bckpktgen", "cfgenbck2bckpktgen", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("cfgenallmpcppktgen", "cfgenallmpcppktgen", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("cfgxpnstartpktgen", "cfgxpnstartpktgen", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("cfgxpnenpktgen", "cfgxpnenpktgen", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_xpn_pktgen_llid[]={
            BDMFMON_MAKE_PARM("cfgxpnpktgenllid1", "cfgxpnpktgenllid1", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("cfgxpnpktgenllid0", "cfgxpnpktgenllid0", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_xpn_pktgen_pkt_cnt[]={
            BDMFMON_MAKE_PARM("cfgxpnpktgenburstmode", "cfgxpnpktgenburstmode", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("cfgxpnpktgenburstsize", "cfgxpnpktgenburstsize", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_xpn_pktgen_pkt_size[]={
            BDMFMON_MAKE_PARM("cfgxpnpktgensizeincr", "cfgxpnpktgensizeincr", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("cfgxpnpktgensizeend", "cfgxpnpktgensizeend", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("cfgxpnpktgensizestart", "cfgxpnpktgensizestart", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_xpn_pktgen_ipg[]={
            BDMFMON_MAKE_PARM("cfgxpnpktgenbck2bckipg", "cfgxpnpktgenbck2bckipg", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("cfgxpnpktgenipg", "cfgxpnpktgenipg", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_ts_jitter_thresh[]={
            BDMFMON_MAKE_PARM("cfgtsjttrthresh", "cfgtsjttrthresh", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_ts_update[]={
            BDMFMON_MAKE_PARM("cfgtsfullupdthr", "cfgtsfullupdthr", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("cfgenautotsupd", "cfgenautotsupd", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("cfgtsupdper", "cfgtsupdper", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_gnt_overhead[]={
            BDMFMON_MAKE_PARM("cfggntoh", "cfggntoh", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_discover_overhead[]={
            BDMFMON_MAKE_PARM("cfgdiscoh", "cfgdiscoh", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_discover_info[]={
            BDMFMON_MAKE_PARM("cfgdiscinfofld", "cfgdiscinfofld", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_xpn_oversize_thresh[]={
            BDMFMON_MAKE_PARM("cfgxpnovrszthresh", "cfgxpnovrszthresh", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_llid_0[]={
            BDMFMON_MAKE_PARM("cfgonullid0", "cfgonullid0", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_llid_1[]={
            BDMFMON_MAKE_PARM("cfgonullid1", "cfgonullid1", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_llid_2[]={
            BDMFMON_MAKE_PARM("cfgonullid2", "cfgonullid2", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_llid_3[]={
            BDMFMON_MAKE_PARM("cfgonullid3", "cfgonullid3", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_llid_4[]={
            BDMFMON_MAKE_PARM("cfgonullid4", "cfgonullid4", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_llid_5[]={
            BDMFMON_MAKE_PARM("cfgonullid5", "cfgonullid5", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_llid_6[]={
            BDMFMON_MAKE_PARM("cfgonullid6", "cfgonullid6", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_llid_7[]={
            BDMFMON_MAKE_PARM("cfgonullid7", "cfgonullid7", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_llid_8[]={
            BDMFMON_MAKE_PARM("cfgonullid8", "cfgonullid8", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_llid_9[]={
            BDMFMON_MAKE_PARM("cfgonullid9", "cfgonullid9", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_llid_10[]={
            BDMFMON_MAKE_PARM("cfgonullid10", "cfgonullid10", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_llid_11[]={
            BDMFMON_MAKE_PARM("cfgonullid11", "cfgonullid11", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_llid_12[]={
            BDMFMON_MAKE_PARM("cfgonullid12", "cfgonullid12", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_llid_13[]={
            BDMFMON_MAKE_PARM("cfgonullid13", "cfgonullid13", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_llid_14[]={
            BDMFMON_MAKE_PARM("cfgonullid14", "cfgonullid14", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_llid_15[]={
            BDMFMON_MAKE_PARM("cfgonullid15", "cfgonullid15", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_llid_16[]={
            BDMFMON_MAKE_PARM("cfgonullid16", "cfgonullid16", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_llid_17[]={
            BDMFMON_MAKE_PARM("cfgonullid17", "cfgonullid17", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_llid_18[]={
            BDMFMON_MAKE_PARM("cfgonullid18", "cfgonullid18", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_llid_19[]={
            BDMFMON_MAKE_PARM("cfgonullid19", "cfgonullid19", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_llid_20[]={
            BDMFMON_MAKE_PARM("cfgonullid20", "cfgonullid20", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_llid_21[]={
            BDMFMON_MAKE_PARM("cfgonullid21", "cfgonullid21", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_llid_22[]={
            BDMFMON_MAKE_PARM("cfgonullid22", "cfgonullid22", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_llid_23[]={
            BDMFMON_MAKE_PARM("cfgonullid23", "cfgonullid23", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_llid_24[]={
            BDMFMON_MAKE_PARM("cfgonullid24", "cfgonullid24", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_llid_25[]={
            BDMFMON_MAKE_PARM("cfgonullid25", "cfgonullid25", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_llid_26[]={
            BDMFMON_MAKE_PARM("cfgonullid26", "cfgonullid26", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_llid_27[]={
            BDMFMON_MAKE_PARM("cfgonullid27", "cfgonullid27", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_llid_28[]={
            BDMFMON_MAKE_PARM("cfgonullid28", "cfgonullid28", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_llid_29[]={
            BDMFMON_MAKE_PARM("cfgonullid29", "cfgonullid29", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_llid_30[]={
            BDMFMON_MAKE_PARM("cfgonullid30", "cfgonullid30", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_llid_31[]={
            BDMFMON_MAKE_PARM("cfgonullid31", "cfgonullid31", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_max_mpcp_update[]={
            BDMFMON_MAKE_PARM("cfgmaxposmpcpupd", "cfgmaxposmpcpupd", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_ipg_insertion[]={
            BDMFMON_MAKE_PARM("cfgshortipg", "cfgshortipg", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("cfginsertipg", "cfginsertipg", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("cfgipgword", "cfgipgword", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_transport_time[]={
            BDMFMON_MAKE_PARM("cftransporttime", "cftransporttime", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_overlap_gnt_oh[]={
            BDMFMON_MAKE_PARM("cfgovrlpoh", "cfgovrlpoh", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_mac_mode[]={
            BDMFMON_MAKE_PARM("cfgennogntxmt", "cfgennogntxmt", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_pmctx_ctl[]={
            BDMFMON_MAKE_PARM("cfgmpcpupdperiod", "cfgmpcpupdperiod", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("cfgdis4idleb4startchar", "cfgdis4idleb4startchar", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("cfgenidledscrd", "cfgenidledscrd", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("cfgseltxpontime", "cfgseltxpontime", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("cfgmpcpcontupd", "cfgmpcpcontupd", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("cfgenmaxmpcpupd", "cfgenmaxmpcpupd", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("cfgennegtimeabort", "cfgennegtimeabort", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_sec_ctl[]={
            BDMFMON_MAKE_PARM("cfgsecrxenshortlen", "cfgsecrxenshortlen", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("cfgensectxfakeaes", "cfgensectxfakeaes", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("cfgensecrxfakeaes", "cfgensecrxfakeaes", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("cfgenaereplayprct", "cfgenaereplayprct", BDMFMON_PARM_NUMBER, 0),
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
        static bdmfmon_cmd_parm_t set_burst_prelaunch_offset[]={
            BDMFMON_MAKE_PARM("cfgburstprelaunchoffset", "cfgburstprelaunchoffset", BDMFMON_PARM_NUMBER, 0),
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
        static bdmfmon_enum_val_t selector_table[] = {
            { .name="ctl", .val=BDMF_ctl, .parms=set_ctl },
            { .name="int_status", .val=BDMF_int_status, .parms=set_int_status },
            { .name="int_mask", .val=BDMF_int_mask, .parms=set_int_mask },
            { .name="port_command", .val=BDMF_port_command, .parms=set_port_command },
            { .name="port_data_", .val=BDMF_port_data_, .parms=set_port_data_ },
            { .name="macsec", .val=BDMF_macsec, .parms=set_macsec },
            { .name="xpn_xmt_offset", .val=BDMF_xpn_xmt_offset, .parms=set_xpn_xmt_offset },
            { .name="xpn_timestamp_offset", .val=BDMF_xpn_timestamp_offset, .parms=set_xpn_timestamp_offset },
            { .name="xpn_pktgen_ctl", .val=BDMF_xpn_pktgen_ctl, .parms=set_xpn_pktgen_ctl },
            { .name="xpn_pktgen_llid", .val=BDMF_xpn_pktgen_llid, .parms=set_xpn_pktgen_llid },
            { .name="xpn_pktgen_pkt_cnt", .val=BDMF_xpn_pktgen_pkt_cnt, .parms=set_xpn_pktgen_pkt_cnt },
            { .name="xpn_pktgen_pkt_size", .val=BDMF_xpn_pktgen_pkt_size, .parms=set_xpn_pktgen_pkt_size },
            { .name="xpn_pktgen_ipg", .val=BDMF_xpn_pktgen_ipg, .parms=set_xpn_pktgen_ipg },
            { .name="ts_jitter_thresh", .val=BDMF_ts_jitter_thresh, .parms=set_ts_jitter_thresh },
            { .name="ts_update", .val=BDMF_ts_update, .parms=set_ts_update },
            { .name="gnt_overhead", .val=BDMF_gnt_overhead, .parms=set_gnt_overhead },
            { .name="discover_overhead", .val=BDMF_discover_overhead, .parms=set_discover_overhead },
            { .name="discover_info", .val=BDMF_discover_info, .parms=set_discover_info },
            { .name="xpn_oversize_thresh", .val=BDMF_xpn_oversize_thresh, .parms=set_xpn_oversize_thresh },
            { .name="llid_0", .val=BDMF_llid_0, .parms=set_llid_0 },
            { .name="llid_1", .val=BDMF_llid_1, .parms=set_llid_1 },
            { .name="llid_2", .val=BDMF_llid_2, .parms=set_llid_2 },
            { .name="llid_3", .val=BDMF_llid_3, .parms=set_llid_3 },
            { .name="llid_4", .val=BDMF_llid_4, .parms=set_llid_4 },
            { .name="llid_5", .val=BDMF_llid_5, .parms=set_llid_5 },
            { .name="llid_6", .val=BDMF_llid_6, .parms=set_llid_6 },
            { .name="llid_7", .val=BDMF_llid_7, .parms=set_llid_7 },
            { .name="llid_8", .val=BDMF_llid_8, .parms=set_llid_8 },
            { .name="llid_9", .val=BDMF_llid_9, .parms=set_llid_9 },
            { .name="llid_10", .val=BDMF_llid_10, .parms=set_llid_10 },
            { .name="llid_11", .val=BDMF_llid_11, .parms=set_llid_11 },
            { .name="llid_12", .val=BDMF_llid_12, .parms=set_llid_12 },
            { .name="llid_13", .val=BDMF_llid_13, .parms=set_llid_13 },
            { .name="llid_14", .val=BDMF_llid_14, .parms=set_llid_14 },
            { .name="llid_15", .val=BDMF_llid_15, .parms=set_llid_15 },
            { .name="llid_16", .val=BDMF_llid_16, .parms=set_llid_16 },
            { .name="llid_17", .val=BDMF_llid_17, .parms=set_llid_17 },
            { .name="llid_18", .val=BDMF_llid_18, .parms=set_llid_18 },
            { .name="llid_19", .val=BDMF_llid_19, .parms=set_llid_19 },
            { .name="llid_20", .val=BDMF_llid_20, .parms=set_llid_20 },
            { .name="llid_21", .val=BDMF_llid_21, .parms=set_llid_21 },
            { .name="llid_22", .val=BDMF_llid_22, .parms=set_llid_22 },
            { .name="llid_23", .val=BDMF_llid_23, .parms=set_llid_23 },
            { .name="llid_24", .val=BDMF_llid_24, .parms=set_llid_24 },
            { .name="llid_25", .val=BDMF_llid_25, .parms=set_llid_25 },
            { .name="llid_26", .val=BDMF_llid_26, .parms=set_llid_26 },
            { .name="llid_27", .val=BDMF_llid_27, .parms=set_llid_27 },
            { .name="llid_28", .val=BDMF_llid_28, .parms=set_llid_28 },
            { .name="llid_29", .val=BDMF_llid_29, .parms=set_llid_29 },
            { .name="llid_30", .val=BDMF_llid_30, .parms=set_llid_30 },
            { .name="llid_31", .val=BDMF_llid_31, .parms=set_llid_31 },
            { .name="max_mpcp_update", .val=BDMF_max_mpcp_update, .parms=set_max_mpcp_update },
            { .name="ipg_insertion", .val=BDMF_ipg_insertion, .parms=set_ipg_insertion },
            { .name="transport_time", .val=BDMF_transport_time, .parms=set_transport_time },
            { .name="overlap_gnt_oh", .val=BDMF_overlap_gnt_oh, .parms=set_overlap_gnt_oh },
            { .name="mac_mode", .val=BDMF_mac_mode, .parms=set_mac_mode },
            { .name="pmctx_ctl", .val=BDMF_pmctx_ctl, .parms=set_pmctx_ctl },
            { .name="sec_ctl", .val=BDMF_sec_ctl, .parms=set_sec_ctl },
            { .name="ae_pktnum_window", .val=BDMF_ae_pktnum_window, .parms=set_ae_pktnum_window },
            { .name="ae_pktnum_thresh", .val=BDMF_ae_pktnum_thresh, .parms=set_ae_pktnum_thresh },
            { .name="burst_prelaunch_offset", .val=BDMF_burst_prelaunch_offset, .parms=set_burst_prelaunch_offset },
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
            BDMFMON_ENUM_LAST
        };
        BDMFMON_MAKE_CMD(dir, "set", "set", bcm_xif_cli_set,
            BDMFMON_MAKE_PARM_SELECTOR("purpose", "purpose", selector_table, 0));
    }
    {
        static bdmfmon_cmd_parm_t set_default[]={
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_port_data_[]={
            BDMFMON_MAKE_PARM("portidx", "portidx", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_enum_val_t selector_table[] = {
            { .name="ctl", .val=BDMF_ctl, .parms=set_default },
            { .name="int_status", .val=BDMF_int_status, .parms=set_default },
            { .name="int_mask", .val=BDMF_int_mask, .parms=set_default },
            { .name="port_command", .val=BDMF_port_command, .parms=set_default },
            { .name="port_data_", .val=BDMF_port_data_, .parms=set_port_data_ },
            { .name="macsec", .val=BDMF_macsec, .parms=set_default },
            { .name="xpn_xmt_offset", .val=BDMF_xpn_xmt_offset, .parms=set_default },
            { .name="xpn_timestamp_offset", .val=BDMF_xpn_timestamp_offset, .parms=set_default },
            { .name="xpn_pktgen_ctl", .val=BDMF_xpn_pktgen_ctl, .parms=set_default },
            { .name="xpn_pktgen_llid", .val=BDMF_xpn_pktgen_llid, .parms=set_default },
            { .name="xpn_pktgen_pkt_cnt", .val=BDMF_xpn_pktgen_pkt_cnt, .parms=set_default },
            { .name="xpn_pktgen_pkt_size", .val=BDMF_xpn_pktgen_pkt_size, .parms=set_default },
            { .name="xpn_pktgen_ipg", .val=BDMF_xpn_pktgen_ipg, .parms=set_default },
            { .name="ts_jitter_thresh", .val=BDMF_ts_jitter_thresh, .parms=set_default },
            { .name="ts_update", .val=BDMF_ts_update, .parms=set_default },
            { .name="gnt_overhead", .val=BDMF_gnt_overhead, .parms=set_default },
            { .name="discover_overhead", .val=BDMF_discover_overhead, .parms=set_default },
            { .name="discover_info", .val=BDMF_discover_info, .parms=set_default },
            { .name="xpn_oversize_thresh", .val=BDMF_xpn_oversize_thresh, .parms=set_default },
            { .name="secrx_keynum", .val=BDMF_secrx_keynum, .parms=set_default },
            { .name="secrx_encrypt", .val=BDMF_secrx_encrypt, .parms=set_default },
            { .name="pmc_frame_rx_cnt", .val=BDMF_pmc_frame_rx_cnt, .parms=set_default },
            { .name="pmc_byte_rx_cnt", .val=BDMF_pmc_byte_rx_cnt, .parms=set_default },
            { .name="pmc_runt_rx_cnt", .val=BDMF_pmc_runt_rx_cnt, .parms=set_default },
            { .name="pmc_cw_err_rx_cnt", .val=BDMF_pmc_cw_err_rx_cnt, .parms=set_default },
            { .name="pmc_crc8_err_rx_cnt", .val=BDMF_pmc_crc8_err_rx_cnt, .parms=set_default },
            { .name="xpn_data_frm_cnt", .val=BDMF_xpn_data_frm_cnt, .parms=set_default },
            { .name="xpn_data_byte_cnt", .val=BDMF_xpn_data_byte_cnt, .parms=set_default },
            { .name="xpn_mpcp_frm_cnt", .val=BDMF_xpn_mpcp_frm_cnt, .parms=set_default },
            { .name="xpn_oam_frm_cnt", .val=BDMF_xpn_oam_frm_cnt, .parms=set_default },
            { .name="xpn_oam_byte_cnt", .val=BDMF_xpn_oam_byte_cnt, .parms=set_default },
            { .name="xpn_oversize_frm_cnt", .val=BDMF_xpn_oversize_frm_cnt, .parms=set_default },
            { .name="sec_abort_frm_cnt", .val=BDMF_sec_abort_frm_cnt, .parms=set_default },
            { .name="pmc_tx_neg_event_cnt", .val=BDMF_pmc_tx_neg_event_cnt, .parms=set_default },
            { .name="xpn_idle_pkt_cnt", .val=BDMF_xpn_idle_pkt_cnt, .parms=set_default },
            { .name="llid_0", .val=BDMF_llid_0, .parms=set_default },
            { .name="llid_1", .val=BDMF_llid_1, .parms=set_default },
            { .name="llid_2", .val=BDMF_llid_2, .parms=set_default },
            { .name="llid_3", .val=BDMF_llid_3, .parms=set_default },
            { .name="llid_4", .val=BDMF_llid_4, .parms=set_default },
            { .name="llid_5", .val=BDMF_llid_5, .parms=set_default },
            { .name="llid_6", .val=BDMF_llid_6, .parms=set_default },
            { .name="llid_7", .val=BDMF_llid_7, .parms=set_default },
            { .name="llid_8", .val=BDMF_llid_8, .parms=set_default },
            { .name="llid_9", .val=BDMF_llid_9, .parms=set_default },
            { .name="llid_10", .val=BDMF_llid_10, .parms=set_default },
            { .name="llid_11", .val=BDMF_llid_11, .parms=set_default },
            { .name="llid_12", .val=BDMF_llid_12, .parms=set_default },
            { .name="llid_13", .val=BDMF_llid_13, .parms=set_default },
            { .name="llid_14", .val=BDMF_llid_14, .parms=set_default },
            { .name="llid_15", .val=BDMF_llid_15, .parms=set_default },
            { .name="llid_16", .val=BDMF_llid_16, .parms=set_default },
            { .name="llid_17", .val=BDMF_llid_17, .parms=set_default },
            { .name="llid_18", .val=BDMF_llid_18, .parms=set_default },
            { .name="llid_19", .val=BDMF_llid_19, .parms=set_default },
            { .name="llid_20", .val=BDMF_llid_20, .parms=set_default },
            { .name="llid_21", .val=BDMF_llid_21, .parms=set_default },
            { .name="llid_22", .val=BDMF_llid_22, .parms=set_default },
            { .name="llid_23", .val=BDMF_llid_23, .parms=set_default },
            { .name="llid_24", .val=BDMF_llid_24, .parms=set_default },
            { .name="llid_25", .val=BDMF_llid_25, .parms=set_default },
            { .name="llid_26", .val=BDMF_llid_26, .parms=set_default },
            { .name="llid_27", .val=BDMF_llid_27, .parms=set_default },
            { .name="llid_28", .val=BDMF_llid_28, .parms=set_default },
            { .name="llid_29", .val=BDMF_llid_29, .parms=set_default },
            { .name="llid_30", .val=BDMF_llid_30, .parms=set_default },
            { .name="llid_31", .val=BDMF_llid_31, .parms=set_default },
            { .name="max_mpcp_update", .val=BDMF_max_mpcp_update, .parms=set_default },
            { .name="ipg_insertion", .val=BDMF_ipg_insertion, .parms=set_default },
            { .name="transport_time", .val=BDMF_transport_time, .parms=set_default },
            { .name="mpcp_time", .val=BDMF_mpcp_time, .parms=set_default },
            { .name="overlap_gnt_oh", .val=BDMF_overlap_gnt_oh, .parms=set_default },
            { .name="mac_mode", .val=BDMF_mac_mode, .parms=set_default },
            { .name="pmctx_ctl", .val=BDMF_pmctx_ctl, .parms=set_default },
            { .name="sec_ctl", .val=BDMF_sec_ctl, .parms=set_default },
            { .name="ae_pktnum_window", .val=BDMF_ae_pktnum_window, .parms=set_default },
            { .name="ae_pktnum_thresh", .val=BDMF_ae_pktnum_thresh, .parms=set_default },
            { .name="sectx_keynum", .val=BDMF_sectx_keynum, .parms=set_default },
            { .name="sectx_encrypt", .val=BDMF_sectx_encrypt, .parms=set_default },
            { .name="ae_pktnum_stat", .val=BDMF_ae_pktnum_stat, .parms=set_default },
            { .name="mpcp_update", .val=BDMF_mpcp_update, .parms=set_default },
            { .name="burst_prelaunch_offset", .val=BDMF_burst_prelaunch_offset, .parms=set_default },
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
            { .name="sectx_keynum_1", .val=BDMF_sectx_keynum_1, .parms=set_default },
            { .name="secrx_keynum_1", .val=BDMF_secrx_keynum_1, .parms=set_default },
            BDMFMON_ENUM_LAST
        };
        BDMFMON_MAKE_CMD(dir, "get", "get", bcm_xif_cli_get,
            BDMFMON_MAKE_PARM_SELECTOR("purpose", "purpose", selector_table, 0));
    }
    {
        static bdmfmon_enum_val_t enum_table_test_method[] = {
            { .name="low" , .val=bdmf_test_method_low },
            { .name="mid" , .val=bdmf_test_method_mid },
            { .name="high" , .val=bdmf_test_method_high },
            BDMFMON_ENUM_LAST
        };
        BDMFMON_MAKE_CMD(dir, "test", "test", bcm_xif_cli_test,
            BDMFMON_MAKE_PARM_ENUM("method", "low: 0000, mid: 1000, high: 1111", enum_table_test_method, 0));
    }
    {
        static bdmfmon_enum_val_t enum_table_address[] = {
            { .name="CTL" , .val=bdmf_address_ctl },
            { .name="INT_STATUS" , .val=bdmf_address_int_status },
            { .name="INT_MASK" , .val=bdmf_address_int_mask },
            { .name="PORT_COMMAND" , .val=bdmf_address_port_command },
            { .name="PORT_DATA_" , .val=bdmf_address_port_data_ },
            { .name="MACSEC" , .val=bdmf_address_macsec },
            { .name="XPN_XMT_OFFSET" , .val=bdmf_address_xpn_xmt_offset },
            { .name="XPN_TIMESTAMP_OFFSET" , .val=bdmf_address_xpn_timestamp_offset },
            { .name="XPN_PKTGEN_CTL" , .val=bdmf_address_xpn_pktgen_ctl },
            { .name="XPN_PKTGEN_LLID" , .val=bdmf_address_xpn_pktgen_llid },
            { .name="XPN_PKTGEN_PKT_CNT" , .val=bdmf_address_xpn_pktgen_pkt_cnt },
            { .name="XPN_PKTGEN_PKT_SIZE" , .val=bdmf_address_xpn_pktgen_pkt_size },
            { .name="XPN_PKTGEN_IPG" , .val=bdmf_address_xpn_pktgen_ipg },
            { .name="TS_JITTER_THRESH" , .val=bdmf_address_ts_jitter_thresh },
            { .name="TS_UPDATE" , .val=bdmf_address_ts_update },
            { .name="GNT_OVERHEAD" , .val=bdmf_address_gnt_overhead },
            { .name="DISCOVER_OVERHEAD" , .val=bdmf_address_discover_overhead },
            { .name="DISCOVER_INFO" , .val=bdmf_address_discover_info },
            { .name="XPN_OVERSIZE_THRESH" , .val=bdmf_address_xpn_oversize_thresh },
            { .name="SECRX_KEYNUM" , .val=bdmf_address_secrx_keynum },
            { .name="SECRX_ENCRYPT" , .val=bdmf_address_secrx_encrypt },
            { .name="PMC_FRAME_RX_CNT" , .val=bdmf_address_pmc_frame_rx_cnt },
            { .name="PMC_BYTE_RX_CNT" , .val=bdmf_address_pmc_byte_rx_cnt },
            { .name="PMC_RUNT_RX_CNT" , .val=bdmf_address_pmc_runt_rx_cnt },
            { .name="PMC_CW_ERR_RX_CNT" , .val=bdmf_address_pmc_cw_err_rx_cnt },
            { .name="PMC_CRC8_ERR_RX_CNT" , .val=bdmf_address_pmc_crc8_err_rx_cnt },
            { .name="XPN_DATA_FRM_CNT" , .val=bdmf_address_xpn_data_frm_cnt },
            { .name="XPN_DATA_BYTE_CNT" , .val=bdmf_address_xpn_data_byte_cnt },
            { .name="XPN_MPCP_FRM_CNT" , .val=bdmf_address_xpn_mpcp_frm_cnt },
            { .name="XPN_OAM_FRM_CNT" , .val=bdmf_address_xpn_oam_frm_cnt },
            { .name="XPN_OAM_BYTE_CNT" , .val=bdmf_address_xpn_oam_byte_cnt },
            { .name="XPN_OVERSIZE_FRM_CNT" , .val=bdmf_address_xpn_oversize_frm_cnt },
            { .name="SEC_ABORT_FRM_CNT" , .val=bdmf_address_sec_abort_frm_cnt },
            { .name="PMC_TX_NEG_EVENT_CNT" , .val=bdmf_address_pmc_tx_neg_event_cnt },
            { .name="XPN_IDLE_PKT_CNT" , .val=bdmf_address_xpn_idle_pkt_cnt },
            { .name="LLID_0" , .val=bdmf_address_llid_0 },
            { .name="LLID_1" , .val=bdmf_address_llid_1 },
            { .name="LLID_2" , .val=bdmf_address_llid_2 },
            { .name="LLID_3" , .val=bdmf_address_llid_3 },
            { .name="LLID_4" , .val=bdmf_address_llid_4 },
            { .name="LLID_5" , .val=bdmf_address_llid_5 },
            { .name="LLID_6" , .val=bdmf_address_llid_6 },
            { .name="LLID_7" , .val=bdmf_address_llid_7 },
            { .name="LLID_8" , .val=bdmf_address_llid_8 },
            { .name="LLID_9" , .val=bdmf_address_llid_9 },
            { .name="LLID_10" , .val=bdmf_address_llid_10 },
            { .name="LLID_11" , .val=bdmf_address_llid_11 },
            { .name="LLID_12" , .val=bdmf_address_llid_12 },
            { .name="LLID_13" , .val=bdmf_address_llid_13 },
            { .name="LLID_14" , .val=bdmf_address_llid_14 },
            { .name="LLID_15" , .val=bdmf_address_llid_15 },
            { .name="LLID_16" , .val=bdmf_address_llid_16 },
            { .name="LLID_17" , .val=bdmf_address_llid_17 },
            { .name="LLID_18" , .val=bdmf_address_llid_18 },
            { .name="LLID_19" , .val=bdmf_address_llid_19 },
            { .name="LLID_20" , .val=bdmf_address_llid_20 },
            { .name="LLID_21" , .val=bdmf_address_llid_21 },
            { .name="LLID_22" , .val=bdmf_address_llid_22 },
            { .name="LLID_23" , .val=bdmf_address_llid_23 },
            { .name="LLID_24" , .val=bdmf_address_llid_24 },
            { .name="LLID_25" , .val=bdmf_address_llid_25 },
            { .name="LLID_26" , .val=bdmf_address_llid_26 },
            { .name="LLID_27" , .val=bdmf_address_llid_27 },
            { .name="LLID_28" , .val=bdmf_address_llid_28 },
            { .name="LLID_29" , .val=bdmf_address_llid_29 },
            { .name="LLID_30" , .val=bdmf_address_llid_30 },
            { .name="LLID_31" , .val=bdmf_address_llid_31 },
            { .name="MAX_MPCP_UPDATE" , .val=bdmf_address_max_mpcp_update },
            { .name="IPG_INSERTION" , .val=bdmf_address_ipg_insertion },
            { .name="TRANSPORT_TIME" , .val=bdmf_address_transport_time },
            { .name="MPCP_TIME" , .val=bdmf_address_mpcp_time },
            { .name="OVERLAP_GNT_OH" , .val=bdmf_address_overlap_gnt_oh },
            { .name="MAC_MODE" , .val=bdmf_address_mac_mode },
            { .name="PMCTX_CTL" , .val=bdmf_address_pmctx_ctl },
            { .name="SEC_CTL" , .val=bdmf_address_sec_ctl },
            { .name="AE_PKTNUM_WINDOW" , .val=bdmf_address_ae_pktnum_window },
            { .name="AE_PKTNUM_THRESH" , .val=bdmf_address_ae_pktnum_thresh },
            { .name="SECTX_KEYNUM" , .val=bdmf_address_sectx_keynum },
            { .name="SECTX_ENCRYPT" , .val=bdmf_address_sectx_encrypt },
            { .name="AE_PKTNUM_STAT" , .val=bdmf_address_ae_pktnum_stat },
            { .name="MPCP_UPDATE" , .val=bdmf_address_mpcp_update },
            { .name="BURST_PRELAUNCH_OFFSET" , .val=bdmf_address_burst_prelaunch_offset },
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
            { .name="SECTX_KEYNUM_1" , .val=bdmf_address_sectx_keynum_1 },
            { .name="SECRX_KEYNUM_1" , .val=bdmf_address_secrx_keynum_1 },
            BDMFMON_ENUM_LAST
        };
        BDMFMON_MAKE_CMD(dir, "address", "address", bcm_xif_cli_address,
            BDMFMON_MAKE_PARM_ENUM("method", "method", enum_table_address, 0),
            BDMFMON_MAKE_PARM("index2", "onu_id/alloc_id/port_id/etc...", BDMFMON_PARM_NUMBER, BDMFMON_PARM_FLAG_OPTIONAL));
    }
    return dir;
}
#endif /* USE_BDMF_SHELL */

