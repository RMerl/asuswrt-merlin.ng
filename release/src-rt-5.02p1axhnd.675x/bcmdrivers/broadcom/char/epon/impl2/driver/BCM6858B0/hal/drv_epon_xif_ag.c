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
       (ctl->cfgpmcrxencrc8chk >= _1BITS_MAX_VAL_) ||
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
    reg_ctl = RU_FIELD_SET(0, XIF, CTL, CFGPMCRXENCRC8CHK, reg_ctl, ctl->cfgpmcrxencrc8chk);
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
    ctl->cfgpmcrxencrc8chk = RU_FIELD_GET(0, XIF, CTL, CFGPMCRXENCRC8CHK, reg_ctl);
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

bdmf_error_t ag_drv_xif_llid__set(uint8_t llid_index, uint32_t cfgonullid)
{
    uint32_t reg_llid_=0;

#ifdef VALIDATE_PARMS
    if((llid_index >= 32) ||
       (cfgonullid >= _17BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_llid_ = RU_FIELD_SET(0, XIF, LLID_, CFGONULLID, reg_llid_, cfgonullid);

    RU_REG_RAM_WRITE(0, llid_index, XIF, LLID_, reg_llid_);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xif_llid__get(uint8_t llid_index, uint32_t *cfgonullid)
{
    uint32_t reg_llid_=0;

#ifdef VALIDATE_PARMS
    if(!cfgonullid)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((llid_index >= 32))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(0, llid_index, XIF, LLID_, reg_llid_);

    *cfgonullid = RU_FIELD_GET(0, XIF, LLID_, CFGONULLID, reg_llid_);

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

bdmf_error_t ag_drv_xif_sec_ctl_set(const xif_sec_ctl *sec_ctl)
{
    uint32_t reg_sec_ctl=0;

#ifdef VALIDATE_PARMS
    if(!sec_ctl)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((sec_ctl->cfgsecrxenshortlen >= _1BITS_MAX_VAL_) ||
       (sec_ctl->cfgensectxfakeaes >= _1BITS_MAX_VAL_) ||
       (sec_ctl->cfgensecrxfakeaes >= _1BITS_MAX_VAL_) ||
       (sec_ctl->cfgsecrxenpktnumrlovr >= _1BITS_MAX_VAL_) ||
       (sec_ctl->cfgsectxenpktnumrlovr >= _1BITS_MAX_VAL_) ||
       (sec_ctl->cfgenaereplayprct >= _1BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_sec_ctl = RU_FIELD_SET(0, XIF, SEC_CTL, CFGSECRXENSHORTLEN, reg_sec_ctl, sec_ctl->cfgsecrxenshortlen);
    reg_sec_ctl = RU_FIELD_SET(0, XIF, SEC_CTL, CFGENSECTXFAKEAES, reg_sec_ctl, sec_ctl->cfgensectxfakeaes);
    reg_sec_ctl = RU_FIELD_SET(0, XIF, SEC_CTL, CFGENSECRXFAKEAES, reg_sec_ctl, sec_ctl->cfgensecrxfakeaes);
    reg_sec_ctl = RU_FIELD_SET(0, XIF, SEC_CTL, CFGSECRXENPKTNUMRLOVR, reg_sec_ctl, sec_ctl->cfgsecrxenpktnumrlovr);
    reg_sec_ctl = RU_FIELD_SET(0, XIF, SEC_CTL, CFGSECTXENPKTNUMRLOVR, reg_sec_ctl, sec_ctl->cfgsectxenpktnumrlovr);
    reg_sec_ctl = RU_FIELD_SET(0, XIF, SEC_CTL, CFGENAEREPLAYPRCT, reg_sec_ctl, sec_ctl->cfgenaereplayprct);

    RU_REG_WRITE(0, XIF, SEC_CTL, reg_sec_ctl);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xif_sec_ctl_get(xif_sec_ctl *sec_ctl)
{
    uint32_t reg_sec_ctl=0;

#ifdef VALIDATE_PARMS
    if(!sec_ctl)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, XIF, SEC_CTL, reg_sec_ctl);

    sec_ctl->cfgsecrxenshortlen = RU_FIELD_GET(0, XIF, SEC_CTL, CFGSECRXENSHORTLEN, reg_sec_ctl);
    sec_ctl->cfgensectxfakeaes = RU_FIELD_GET(0, XIF, SEC_CTL, CFGENSECTXFAKEAES, reg_sec_ctl);
    sec_ctl->cfgensecrxfakeaes = RU_FIELD_GET(0, XIF, SEC_CTL, CFGENSECRXFAKEAES, reg_sec_ctl);
    sec_ctl->cfgsecrxenpktnumrlovr = RU_FIELD_GET(0, XIF, SEC_CTL, CFGSECRXENPKTNUMRLOVR, reg_sec_ctl);
    sec_ctl->cfgsectxenpktnumrlovr = RU_FIELD_GET(0, XIF, SEC_CTL, CFGSECTXENPKTNUMRLOVR, reg_sec_ctl);
    sec_ctl->cfgenaereplayprct = RU_FIELD_GET(0, XIF, SEC_CTL, CFGENAEREPLAYPRCT, reg_sec_ctl);

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
    BDMF_llid_,
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
    bdmf_address_llid_,
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
}
bdmf_address;

static int bcm_xif_cli_set(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    bdmf_error_t err;

    switch(parm[0].value.unumber)
    {
    case BDMF_ctl:
    {
        xif_ctl ctl = { .rxencrypten=parm[1].value.unumber, .cfgdisrxdasaencrpt=parm[2].value.unumber, .rxencryptmode=parm[3].value.unumber, .txencrypten=parm[4].value.unumber, .cfgdistxdasaencrpt=parm[5].value.unumber, .txencryptmode=parm[6].value.unumber, .cfgllidmodemsk=parm[7].value.unumber, .cfgxpnbadcrc32=parm[8].value.unumber, .cfgdisdiscinfo=parm[9].value.unumber, .cfgpmctx2rxlpbk=parm[10].value.unumber, .cfgpmctxencrc8bad=parm[11].value.unumber, .cfgenp2p=parm[12].value.unumber, .cfgpmcrxencrc8chk=parm[13].value.unumber, .cfgfecen=parm[14].value.unumber, .cfglegacyrcvtsupd=parm[15].value.unumber, .cfgxpnencrcpassthru=parm[16].value.unumber, .cfgxpndistimestampmod=parm[17].value.unumber, .xifnotrdy=parm[18].value.unumber, .xifdtportrstn=parm[19].value.unumber, .xpntxrstn=parm[20].value.unumber, .pmctxrstn=parm[21].value.unumber, .sectxrstn=parm[22].value.unumber, .cfgdistxoamencrpt=parm[23].value.unumber, .cfgdistxmpcpencrpt=parm[24].value.unumber, .pmcrxrstn=parm[25].value.unumber, .secrxrstn=parm[26].value.unumber};
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
    case BDMF_llid_:
        err = ag_drv_xif_llid__set(parm[1].value.unumber, parm[2].value.unumber);
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
    {
        xif_sec_ctl sec_ctl = { .cfgsecrxenshortlen=parm[1].value.unumber, .cfgensectxfakeaes=parm[2].value.unumber, .cfgensecrxfakeaes=parm[3].value.unumber, .cfgsecrxenpktnumrlovr=parm[4].value.unumber, .cfgsectxenpktnumrlovr=parm[5].value.unumber, .cfgenaereplayprct=parm[6].value.unumber};
        err = ag_drv_xif_sec_ctl_set(&sec_ctl);
        break;
    }
    case BDMF_ae_pktnum_window:
        err = ag_drv_xif_ae_pktnum_window_set(parm[1].value.unumber);
        break;
    case BDMF_ae_pktnum_thresh:
        err = ag_drv_xif_ae_pktnum_thresh_set(parm[1].value.unumber);
        break;
    case BDMF_burst_prelaunch_offset:
        err = ag_drv_xif_burst_prelaunch_offset_set(parm[1].value.unumber);
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
        bdmf_session_print(session, "cfgpmcrxencrc8chk = %u = 0x%x\n", ctl.cfgpmcrxencrc8chk, ctl.cfgpmcrxencrc8chk);
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
    case BDMF_llid_:
    {
        uint32_t cfgonullid;
        err = ag_drv_xif_llid__get(parm[1].value.unumber, &cfgonullid);
        bdmf_session_print(session, "cfgonullid = %u = 0x%x\n", cfgonullid, cfgonullid);
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
        xif_sec_ctl sec_ctl;
        err = ag_drv_xif_sec_ctl_get(&sec_ctl);
        bdmf_session_print(session, "cfgsecrxenshortlen = %u = 0x%x\n", sec_ctl.cfgsecrxenshortlen, sec_ctl.cfgsecrxenshortlen);
        bdmf_session_print(session, "cfgensectxfakeaes = %u = 0x%x\n", sec_ctl.cfgensectxfakeaes, sec_ctl.cfgensectxfakeaes);
        bdmf_session_print(session, "cfgensecrxfakeaes = %u = 0x%x\n", sec_ctl.cfgensecrxfakeaes, sec_ctl.cfgensecrxfakeaes);
        bdmf_session_print(session, "cfgsecrxenpktnumrlovr = %u = 0x%x\n", sec_ctl.cfgsecrxenpktnumrlovr, sec_ctl.cfgsecrxenpktnumrlovr);
        bdmf_session_print(session, "cfgsectxenpktnumrlovr = %u = 0x%x\n", sec_ctl.cfgsectxenpktnumrlovr, sec_ctl.cfgsectxenpktnumrlovr);
        bdmf_session_print(session, "cfgenaereplayprct = %u = 0x%x\n", sec_ctl.cfgenaereplayprct, sec_ctl.cfgenaereplayprct);
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
        xif_ctl ctl = {.rxencrypten=gtmv(m, 1), .cfgdisrxdasaencrpt=gtmv(m, 1), .rxencryptmode=gtmv(m, 2), .txencrypten=gtmv(m, 1), .cfgdistxdasaencrpt=gtmv(m, 1), .txencryptmode=gtmv(m, 2), .cfgllidmodemsk=gtmv(m, 1), .cfgxpnbadcrc32=gtmv(m, 1), .cfgdisdiscinfo=gtmv(m, 1), .cfgpmctx2rxlpbk=gtmv(m, 1), .cfgpmctxencrc8bad=gtmv(m, 1), .cfgenp2p=gtmv(m, 1), .cfgpmcrxencrc8chk=gtmv(m, 1), .cfgfecen=gtmv(m, 1), .cfglegacyrcvtsupd=gtmv(m, 1), .cfgxpnencrcpassthru=gtmv(m, 1), .cfgxpndistimestampmod=gtmv(m, 1), .xifnotrdy=gtmv(m, 1), .xifdtportrstn=gtmv(m, 1), .xpntxrstn=gtmv(m, 1), .pmctxrstn=gtmv(m, 1), .sectxrstn=gtmv(m, 1), .cfgdistxoamencrpt=gtmv(m, 1), .cfgdistxmpcpencrpt=gtmv(m, 1), .pmcrxrstn=gtmv(m, 1), .secrxrstn=gtmv(m, 1)};
        if(!err) bdmf_session_print(session, "ag_drv_xif_ctl_set( %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u)\n", ctl.rxencrypten, ctl.cfgdisrxdasaencrpt, ctl.rxencryptmode, ctl.txencrypten, ctl.cfgdistxdasaencrpt, ctl.txencryptmode, ctl.cfgllidmodemsk, ctl.cfgxpnbadcrc32, ctl.cfgdisdiscinfo, ctl.cfgpmctx2rxlpbk, ctl.cfgpmctxencrc8bad, ctl.cfgenp2p, ctl.cfgpmcrxencrc8chk, ctl.cfgfecen, ctl.cfglegacyrcvtsupd, ctl.cfgxpnencrcpassthru, ctl.cfgxpndistimestampmod, ctl.xifnotrdy, ctl.xifdtportrstn, ctl.xpntxrstn, ctl.pmctxrstn, ctl.sectxrstn, ctl.cfgdistxoamencrpt, ctl.cfgdistxmpcpencrpt, ctl.pmcrxrstn, ctl.secrxrstn);
        if(!err) ag_drv_xif_ctl_set(&ctl);
        if(!err) ag_drv_xif_ctl_get( &ctl);
        if(!err) bdmf_session_print(session, "ag_drv_xif_ctl_get( %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u)\n", ctl.rxencrypten, ctl.cfgdisrxdasaencrpt, ctl.rxencryptmode, ctl.txencrypten, ctl.cfgdistxdasaencrpt, ctl.txencryptmode, ctl.cfgllidmodemsk, ctl.cfgxpnbadcrc32, ctl.cfgdisdiscinfo, ctl.cfgpmctx2rxlpbk, ctl.cfgpmctxencrc8bad, ctl.cfgenp2p, ctl.cfgpmcrxencrc8chk, ctl.cfgfecen, ctl.cfglegacyrcvtsupd, ctl.cfgxpnencrcpassthru, ctl.cfgxpndistimestampmod, ctl.xifnotrdy, ctl.xifdtportrstn, ctl.xpntxrstn, ctl.pmctxrstn, ctl.sectxrstn, ctl.cfgdistxoamencrpt, ctl.cfgdistxmpcpencrpt, ctl.pmcrxrstn, ctl.secrxrstn);
        if(err || ctl.rxencrypten!=gtmv(m, 1) || ctl.cfgdisrxdasaencrpt!=gtmv(m, 1) || ctl.rxencryptmode!=gtmv(m, 2) || ctl.txencrypten!=gtmv(m, 1) || ctl.cfgdistxdasaencrpt!=gtmv(m, 1) || ctl.txencryptmode!=gtmv(m, 2) || ctl.cfgllidmodemsk!=gtmv(m, 1) || ctl.cfgxpnbadcrc32!=gtmv(m, 1) || ctl.cfgdisdiscinfo!=gtmv(m, 1) || ctl.cfgpmctx2rxlpbk!=gtmv(m, 1) || ctl.cfgpmctxencrc8bad!=gtmv(m, 1) || ctl.cfgenp2p!=gtmv(m, 1) || ctl.cfgpmcrxencrc8chk!=gtmv(m, 1) || ctl.cfgfecen!=gtmv(m, 1) || ctl.cfglegacyrcvtsupd!=gtmv(m, 1) || ctl.cfgxpnencrcpassthru!=gtmv(m, 1) || ctl.cfgxpndistimestampmod!=gtmv(m, 1) || ctl.xifnotrdy!=gtmv(m, 1) || ctl.xifdtportrstn!=gtmv(m, 1) || ctl.xpntxrstn!=gtmv(m, 1) || ctl.pmctxrstn!=gtmv(m, 1) || ctl.sectxrstn!=gtmv(m, 1) || ctl.cfgdistxoamencrpt!=gtmv(m, 1) || ctl.cfgdistxmpcpencrpt!=gtmv(m, 1) || ctl.pmcrxrstn!=gtmv(m, 1) || ctl.secrxrstn!=gtmv(m, 1))
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
        uint8_t llid_index=gtmv(m, 5);
        uint32_t cfgonullid=gtmv(m, 17);
        if(!err) bdmf_session_print(session, "ag_drv_xif_llid__set( %u %u)\n", llid_index, cfgonullid);
        if(!err) ag_drv_xif_llid__set(llid_index, cfgonullid);
        if(!err) ag_drv_xif_llid__get( llid_index, &cfgonullid);
        if(!err) bdmf_session_print(session, "ag_drv_xif_llid__get( %u %u)\n", llid_index, cfgonullid);
        if(err || cfgonullid!=gtmv(m, 17))
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
        xif_sec_ctl sec_ctl = {.cfgsecrxenshortlen=gtmv(m, 1), .cfgensectxfakeaes=gtmv(m, 1), .cfgensecrxfakeaes=gtmv(m, 1), .cfgsecrxenpktnumrlovr=gtmv(m, 1), .cfgsectxenpktnumrlovr=gtmv(m, 1), .cfgenaereplayprct=gtmv(m, 1)};
        if(!err) bdmf_session_print(session, "ag_drv_xif_sec_ctl_set( %u %u %u %u %u %u)\n", sec_ctl.cfgsecrxenshortlen, sec_ctl.cfgensectxfakeaes, sec_ctl.cfgensecrxfakeaes, sec_ctl.cfgsecrxenpktnumrlovr, sec_ctl.cfgsectxenpktnumrlovr, sec_ctl.cfgenaereplayprct);
        if(!err) ag_drv_xif_sec_ctl_set(&sec_ctl);
        if(!err) ag_drv_xif_sec_ctl_get( &sec_ctl);
        if(!err) bdmf_session_print(session, "ag_drv_xif_sec_ctl_get( %u %u %u %u %u %u)\n", sec_ctl.cfgsecrxenshortlen, sec_ctl.cfgensectxfakeaes, sec_ctl.cfgensecrxfakeaes, sec_ctl.cfgsecrxenpktnumrlovr, sec_ctl.cfgsectxenpktnumrlovr, sec_ctl.cfgenaereplayprct);
        if(err || sec_ctl.cfgsecrxenshortlen!=gtmv(m, 1) || sec_ctl.cfgensectxfakeaes!=gtmv(m, 1) || sec_ctl.cfgensecrxfakeaes!=gtmv(m, 1) || sec_ctl.cfgsecrxenpktnumrlovr!=gtmv(m, 1) || sec_ctl.cfgsectxenpktnumrlovr!=gtmv(m, 1) || sec_ctl.cfgenaereplayprct!=gtmv(m, 1))
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
    case bdmf_address_llid_ : reg = &RU_REG(XIF, LLID_); blk = &RU_BLK(XIF); break;
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
            BDMFMON_MAKE_PARM("cfgpmcrxencrc8chk", "cfgpmcrxencrc8chk", BDMFMON_PARM_NUMBER, 0),
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
        static bdmfmon_cmd_parm_t set_llid_[]={
            BDMFMON_MAKE_PARM("llid_index", "llid_index", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("cfgonullid", "cfgonullid", BDMFMON_PARM_NUMBER, 0),
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
            BDMFMON_MAKE_PARM("cfgsecrxenpktnumrlovr", "cfgsecrxenpktnumrlovr", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("cfgsectxenpktnumrlovr", "cfgsectxenpktnumrlovr", BDMFMON_PARM_NUMBER, 0),
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
            { .name="llid_", .val=BDMF_llid_, .parms=set_llid_ },
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
        static bdmfmon_cmd_parm_t set_llid_[]={
            BDMFMON_MAKE_PARM("llid_index", "llid_index", BDMFMON_PARM_NUMBER, 0),
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
            { .name="llid_", .val=BDMF_llid_, .parms=set_llid_ },
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
            { .name="LLID_" , .val=bdmf_address_llid_ },
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
            BDMFMON_ENUM_LAST
        };
        BDMFMON_MAKE_CMD(dir, "address", "address", bcm_xif_cli_address,
            BDMFMON_MAKE_PARM_ENUM("method", "method", enum_table_address, 0),
            BDMFMON_MAKE_PARM("index2", "onu_id/alloc_id/port_id/etc...", BDMFMON_PARM_NUMBER, BDMFMON_PARM_FLAG_OPTIONAL));
    }
    return dir;
}
#endif /* USE_BDMF_SHELL */

