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

#include "drivers_common_ag.h"
#include "ngpon_gearbox_ag.h"
bdmf_error_t ag_drv_ngpon_gearbox_rx_ctl_0_set(const ngpon_gearbox_rx_ctl_0 *rx_ctl_0)
{
    uint32_t reg_rx_ctl_0=0;

#ifdef VALIDATE_PARMS
    if(!rx_ctl_0)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((rx_ctl_0->cfngpongboxrxfifordptr >= _4BITS_MAX_VAL_) ||
       (rx_ctl_0->cfngpongboxrxptrautolddis >= _1BITS_MAX_VAL_) ||
       (rx_ctl_0->cfngpongboxrxmaxbadk >= _3BITS_MAX_VAL_) ||
       (rx_ctl_0->cfngpongboxrxfrmk28only >= _1BITS_MAX_VAL_) ||
       (rx_ctl_0->cfngpongboxrxmaxgoodk >= _3BITS_MAX_VAL_) ||
       (rx_ctl_0->cfngpongboxrxfrchunt >= _1BITS_MAX_VAL_) ||
       (rx_ctl_0->cfngpongboxrxoutdataflip >= _1BITS_MAX_VAL_) ||
       (rx_ctl_0->cfngpongboxrxfrcmuxsel >= _1BITS_MAX_VAL_) ||
       (rx_ctl_0->cfngpongboxrxfrcmuxval >= _5BITS_MAX_VAL_) ||
       (rx_ctl_0->cfngpongboxrx20bdataflip >= _1BITS_MAX_VAL_) ||
       (rx_ctl_0->cfngpongboxrxserdataflip >= _1BITS_MAX_VAL_) ||
       (rx_ctl_0->cfngpongboxrxserdatainv >= _1BITS_MAX_VAL_) ||
       (rx_ctl_0->cfngpongboxrxfifoptrld >= _1BITS_MAX_VAL_) ||
       (rx_ctl_0->cfngpongboxrxswsynchold >= _1BITS_MAX_VAL_) ||
       (rx_ctl_0->cfngpongboxrxmode >= _1BITS_MAX_VAL_) ||
       (rx_ctl_0->cfngpongboxrxen >= _1BITS_MAX_VAL_) ||
       (rx_ctl_0->cfngpongboxrstn >= _1BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_rx_ctl_0 = RU_FIELD_SET(0, NGPON_GEARBOX, RX_CTL_0, CFNGPONGBOXRXFIFORDPTR, reg_rx_ctl_0, rx_ctl_0->cfngpongboxrxfifordptr);
    reg_rx_ctl_0 = RU_FIELD_SET(0, NGPON_GEARBOX, RX_CTL_0, CFNGPONGBOXRXPTRAUTOLDDIS, reg_rx_ctl_0, rx_ctl_0->cfngpongboxrxptrautolddis);
    reg_rx_ctl_0 = RU_FIELD_SET(0, NGPON_GEARBOX, RX_CTL_0, CFNGPONGBOXRXMAXBADK, reg_rx_ctl_0, rx_ctl_0->cfngpongboxrxmaxbadk);
    reg_rx_ctl_0 = RU_FIELD_SET(0, NGPON_GEARBOX, RX_CTL_0, CFNGPONGBOXRXFRMK28ONLY, reg_rx_ctl_0, rx_ctl_0->cfngpongboxrxfrmk28only);
    reg_rx_ctl_0 = RU_FIELD_SET(0, NGPON_GEARBOX, RX_CTL_0, CFNGPONGBOXRXMAXGOODK, reg_rx_ctl_0, rx_ctl_0->cfngpongboxrxmaxgoodk);
    reg_rx_ctl_0 = RU_FIELD_SET(0, NGPON_GEARBOX, RX_CTL_0, CFNGPONGBOXRXFRCHUNT, reg_rx_ctl_0, rx_ctl_0->cfngpongboxrxfrchunt);
    reg_rx_ctl_0 = RU_FIELD_SET(0, NGPON_GEARBOX, RX_CTL_0, CFNGPONGBOXRXOUTDATAFLIP, reg_rx_ctl_0, rx_ctl_0->cfngpongboxrxoutdataflip);
    reg_rx_ctl_0 = RU_FIELD_SET(0, NGPON_GEARBOX, RX_CTL_0, CFNGPONGBOXRXFRCMUXSEL, reg_rx_ctl_0, rx_ctl_0->cfngpongboxrxfrcmuxsel);
    reg_rx_ctl_0 = RU_FIELD_SET(0, NGPON_GEARBOX, RX_CTL_0, CFNGPONGBOXRXFRCMUXVAL, reg_rx_ctl_0, rx_ctl_0->cfngpongboxrxfrcmuxval);
    reg_rx_ctl_0 = RU_FIELD_SET(0, NGPON_GEARBOX, RX_CTL_0, CFNGPONGBOXRX20BDATAFLIP, reg_rx_ctl_0, rx_ctl_0->cfngpongboxrx20bdataflip);
    reg_rx_ctl_0 = RU_FIELD_SET(0, NGPON_GEARBOX, RX_CTL_0, CFNGPONGBOXRXSERDATAFLIP, reg_rx_ctl_0, rx_ctl_0->cfngpongboxrxserdataflip);
    reg_rx_ctl_0 = RU_FIELD_SET(0, NGPON_GEARBOX, RX_CTL_0, CFNGPONGBOXRXSERDATAINV, reg_rx_ctl_0, rx_ctl_0->cfngpongboxrxserdatainv);
    reg_rx_ctl_0 = RU_FIELD_SET(0, NGPON_GEARBOX, RX_CTL_0, CFNGPONGBOXRXFIFOPTRLD, reg_rx_ctl_0, rx_ctl_0->cfngpongboxrxfifoptrld);
    reg_rx_ctl_0 = RU_FIELD_SET(0, NGPON_GEARBOX, RX_CTL_0, CFNGPONGBOXRXSWSYNCHOLD, reg_rx_ctl_0, rx_ctl_0->cfngpongboxrxswsynchold);
    reg_rx_ctl_0 = RU_FIELD_SET(0, NGPON_GEARBOX, RX_CTL_0, CFNGPONGBOXRXMODE, reg_rx_ctl_0, rx_ctl_0->cfngpongboxrxmode);
    reg_rx_ctl_0 = RU_FIELD_SET(0, NGPON_GEARBOX, RX_CTL_0, CFNGPONGBOXRXEN, reg_rx_ctl_0, rx_ctl_0->cfngpongboxrxen);
    reg_rx_ctl_0 = RU_FIELD_SET(0, NGPON_GEARBOX, RX_CTL_0, CFNGPONGBOXRSTN, reg_rx_ctl_0, rx_ctl_0->cfngpongboxrstn);

    RU_REG_WRITE(0, NGPON_GEARBOX, RX_CTL_0, reg_rx_ctl_0);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_ngpon_gearbox_rx_ctl_0_get(ngpon_gearbox_rx_ctl_0 *rx_ctl_0)
{
    uint32_t reg_rx_ctl_0=0;

#ifdef VALIDATE_PARMS
    if(!rx_ctl_0)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, NGPON_GEARBOX, RX_CTL_0, reg_rx_ctl_0);

    rx_ctl_0->cfngpongboxrxfifordptr = RU_FIELD_GET(0, NGPON_GEARBOX, RX_CTL_0, CFNGPONGBOXRXFIFORDPTR, reg_rx_ctl_0);
    rx_ctl_0->cfngpongboxrxptrautolddis = RU_FIELD_GET(0, NGPON_GEARBOX, RX_CTL_0, CFNGPONGBOXRXPTRAUTOLDDIS, reg_rx_ctl_0);
    rx_ctl_0->cfngpongboxrxmaxbadk = RU_FIELD_GET(0, NGPON_GEARBOX, RX_CTL_0, CFNGPONGBOXRXMAXBADK, reg_rx_ctl_0);
    rx_ctl_0->cfngpongboxrxfrmk28only = RU_FIELD_GET(0, NGPON_GEARBOX, RX_CTL_0, CFNGPONGBOXRXFRMK28ONLY, reg_rx_ctl_0);
    rx_ctl_0->cfngpongboxrxmaxgoodk = RU_FIELD_GET(0, NGPON_GEARBOX, RX_CTL_0, CFNGPONGBOXRXMAXGOODK, reg_rx_ctl_0);
    rx_ctl_0->cfngpongboxrxfrchunt = RU_FIELD_GET(0, NGPON_GEARBOX, RX_CTL_0, CFNGPONGBOXRXFRCHUNT, reg_rx_ctl_0);
    rx_ctl_0->cfngpongboxrxoutdataflip = RU_FIELD_GET(0, NGPON_GEARBOX, RX_CTL_0, CFNGPONGBOXRXOUTDATAFLIP, reg_rx_ctl_0);
    rx_ctl_0->cfngpongboxrxfrcmuxsel = RU_FIELD_GET(0, NGPON_GEARBOX, RX_CTL_0, CFNGPONGBOXRXFRCMUXSEL, reg_rx_ctl_0);
    rx_ctl_0->cfngpongboxrxfrcmuxval = RU_FIELD_GET(0, NGPON_GEARBOX, RX_CTL_0, CFNGPONGBOXRXFRCMUXVAL, reg_rx_ctl_0);
    rx_ctl_0->cfngpongboxrx20bdataflip = RU_FIELD_GET(0, NGPON_GEARBOX, RX_CTL_0, CFNGPONGBOXRX20BDATAFLIP, reg_rx_ctl_0);
    rx_ctl_0->cfngpongboxrxserdataflip = RU_FIELD_GET(0, NGPON_GEARBOX, RX_CTL_0, CFNGPONGBOXRXSERDATAFLIP, reg_rx_ctl_0);
    rx_ctl_0->cfngpongboxrxserdatainv = RU_FIELD_GET(0, NGPON_GEARBOX, RX_CTL_0, CFNGPONGBOXRXSERDATAINV, reg_rx_ctl_0);
    rx_ctl_0->cfngpongboxrxfifoptrld = RU_FIELD_GET(0, NGPON_GEARBOX, RX_CTL_0, CFNGPONGBOXRXFIFOPTRLD, reg_rx_ctl_0);
    rx_ctl_0->cfngpongboxrxswsynchold = RU_FIELD_GET(0, NGPON_GEARBOX, RX_CTL_0, CFNGPONGBOXRXSWSYNCHOLD, reg_rx_ctl_0);
    rx_ctl_0->cfngpongboxrxmode = RU_FIELD_GET(0, NGPON_GEARBOX, RX_CTL_0, CFNGPONGBOXRXMODE, reg_rx_ctl_0);
    rx_ctl_0->cfngpongboxrxen = RU_FIELD_GET(0, NGPON_GEARBOX, RX_CTL_0, CFNGPONGBOXRXEN, reg_rx_ctl_0);
    rx_ctl_0->cfngpongboxrstn = RU_FIELD_GET(0, NGPON_GEARBOX, RX_CTL_0, CFNGPONGBOXRSTN, reg_rx_ctl_0);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_ngpon_gearbox_rx_ctl_1_set(uint32_t cfngpongboxrxmaxtimercnt)
{
    uint32_t reg_rx_ctl_1=0;

#ifdef VALIDATE_PARMS
    if((cfngpongboxrxmaxtimercnt >= _17BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_rx_ctl_1 = RU_FIELD_SET(0, NGPON_GEARBOX, RX_CTL_1, CFNGPONGBOXRXMAXTIMERCNT, reg_rx_ctl_1, cfngpongboxrxmaxtimercnt);

    RU_REG_WRITE(0, NGPON_GEARBOX, RX_CTL_1, reg_rx_ctl_1);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_ngpon_gearbox_rx_ctl_1_get(uint32_t *cfngpongboxrxmaxtimercnt)
{
    uint32_t reg_rx_ctl_1=0;

#ifdef VALIDATE_PARMS
    if(!cfngpongboxrxmaxtimercnt)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, NGPON_GEARBOX, RX_CTL_1, reg_rx_ctl_1);

    *cfngpongboxrxmaxtimercnt = RU_FIELD_GET(0, NGPON_GEARBOX, RX_CTL_1, CFNGPONGBOXRXMAXTIMERCNT, reg_rx_ctl_1);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_ngpon_gearbox_rx_ctl_2_set(uint16_t cfngpongboxrxk28d5rdp, uint16_t cfngpongboxrxk28d5rdn)
{
    uint32_t reg_rx_ctl_2=0;

#ifdef VALIDATE_PARMS
    if((cfngpongboxrxk28d5rdp >= _10BITS_MAX_VAL_) ||
       (cfngpongboxrxk28d5rdn >= _10BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_rx_ctl_2 = RU_FIELD_SET(0, NGPON_GEARBOX, RX_CTL_2, CFNGPONGBOXRXK28D5RDP, reg_rx_ctl_2, cfngpongboxrxk28d5rdp);
    reg_rx_ctl_2 = RU_FIELD_SET(0, NGPON_GEARBOX, RX_CTL_2, CFNGPONGBOXRXK28D5RDN, reg_rx_ctl_2, cfngpongboxrxk28d5rdn);

    RU_REG_WRITE(0, NGPON_GEARBOX, RX_CTL_2, reg_rx_ctl_2);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_ngpon_gearbox_rx_ctl_2_get(uint16_t *cfngpongboxrxk28d5rdp, uint16_t *cfngpongboxrxk28d5rdn)
{
    uint32_t reg_rx_ctl_2=0;

#ifdef VALIDATE_PARMS
    if(!cfngpongboxrxk28d5rdp || !cfngpongboxrxk28d5rdn)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, NGPON_GEARBOX, RX_CTL_2, reg_rx_ctl_2);

    *cfngpongboxrxk28d5rdp = RU_FIELD_GET(0, NGPON_GEARBOX, RX_CTL_2, CFNGPONGBOXRXK28D5RDP, reg_rx_ctl_2);
    *cfngpongboxrxk28d5rdn = RU_FIELD_GET(0, NGPON_GEARBOX, RX_CTL_2, CFNGPONGBOXRXK28D5RDN, reg_rx_ctl_2);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_ngpon_gearbox_rx_ctl_3_set(uint16_t cfngpongboxrxd5d7rdp, uint16_t cfngpongboxrxd5d7rdn)
{
    uint32_t reg_rx_ctl_3=0;

#ifdef VALIDATE_PARMS
    if((cfngpongboxrxd5d7rdp >= _10BITS_MAX_VAL_) ||
       (cfngpongboxrxd5d7rdn >= _10BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_rx_ctl_3 = RU_FIELD_SET(0, NGPON_GEARBOX, RX_CTL_3, CFNGPONGBOXRXD5D7RDP, reg_rx_ctl_3, cfngpongboxrxd5d7rdp);
    reg_rx_ctl_3 = RU_FIELD_SET(0, NGPON_GEARBOX, RX_CTL_3, CFNGPONGBOXRXD5D7RDN, reg_rx_ctl_3, cfngpongboxrxd5d7rdn);

    RU_REG_WRITE(0, NGPON_GEARBOX, RX_CTL_3, reg_rx_ctl_3);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_ngpon_gearbox_rx_ctl_3_get(uint16_t *cfngpongboxrxd5d7rdp, uint16_t *cfngpongboxrxd5d7rdn)
{
    uint32_t reg_rx_ctl_3=0;

#ifdef VALIDATE_PARMS
    if(!cfngpongboxrxd5d7rdp || !cfngpongboxrxd5d7rdn)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, NGPON_GEARBOX, RX_CTL_3, reg_rx_ctl_3);

    *cfngpongboxrxd5d7rdp = RU_FIELD_GET(0, NGPON_GEARBOX, RX_CTL_3, CFNGPONGBOXRXD5D7RDP, reg_rx_ctl_3);
    *cfngpongboxrxd5d7rdn = RU_FIELD_GET(0, NGPON_GEARBOX, RX_CTL_3, CFNGPONGBOXRXD5D7RDN, reg_rx_ctl_3);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_ngpon_gearbox_tx_ctl_set(const ngpon_gearbox_tx_ctl *tx_ctl)
{
    uint32_t reg_tx_ctl=0;

#ifdef VALIDATE_PARMS
    if(!tx_ctl)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((tx_ctl->cfngpongboxtxfifodatardptr >= _4BITS_MAX_VAL_) ||
       (tx_ctl->cfngpongboxtxfifovldoff >= _5BITS_MAX_VAL_) ||
       (tx_ctl->cfngpongboxtxservldflip >= _1BITS_MAX_VAL_) ||
       (tx_ctl->cfngpongboxtxserdataflip >= _1BITS_MAX_VAL_) ||
       (tx_ctl->cfngpongboxtxservldinv >= _1BITS_MAX_VAL_) ||
       (tx_ctl->cfngpongboxtxserdatainv >= _1BITS_MAX_VAL_) ||
       (tx_ctl->cfngpongboxtxfifovldptrld >= _1BITS_MAX_VAL_) ||
       (tx_ctl->cfngpongboxtxfifoptrld >= _1BITS_MAX_VAL_) ||
       (tx_ctl->cfngpongboxtxen >= _1BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_tx_ctl = RU_FIELD_SET(0, NGPON_GEARBOX, TX_CTL, CFNGPONGBOXTXFIFODATARDPTR, reg_tx_ctl, tx_ctl->cfngpongboxtxfifodatardptr);
    reg_tx_ctl = RU_FIELD_SET(0, NGPON_GEARBOX, TX_CTL, CFNGPONGBOXTXFIFOVLDOFF, reg_tx_ctl, tx_ctl->cfngpongboxtxfifovldoff);
    reg_tx_ctl = RU_FIELD_SET(0, NGPON_GEARBOX, TX_CTL, CFNGPONGBOXTXSERVLDFLIP, reg_tx_ctl, tx_ctl->cfngpongboxtxservldflip);
    reg_tx_ctl = RU_FIELD_SET(0, NGPON_GEARBOX, TX_CTL, CFNGPONGBOXTXSERDATAFLIP, reg_tx_ctl, tx_ctl->cfngpongboxtxserdataflip);
    reg_tx_ctl = RU_FIELD_SET(0, NGPON_GEARBOX, TX_CTL, CFNGPONGBOXTXSERVLDINV, reg_tx_ctl, tx_ctl->cfngpongboxtxservldinv);
    reg_tx_ctl = RU_FIELD_SET(0, NGPON_GEARBOX, TX_CTL, CFNGPONGBOXTXSERDATAINV, reg_tx_ctl, tx_ctl->cfngpongboxtxserdatainv);
    reg_tx_ctl = RU_FIELD_SET(0, NGPON_GEARBOX, TX_CTL, CFNGPONGBOXTXFIFOVLDPTRLD, reg_tx_ctl, tx_ctl->cfngpongboxtxfifovldptrld);
    reg_tx_ctl = RU_FIELD_SET(0, NGPON_GEARBOX, TX_CTL, CFNGPONGBOXTXFIFOPTRLD, reg_tx_ctl, tx_ctl->cfngpongboxtxfifoptrld);
    reg_tx_ctl = RU_FIELD_SET(0, NGPON_GEARBOX, TX_CTL, CFNGPONGBOXTXEN, reg_tx_ctl, tx_ctl->cfngpongboxtxen);

    RU_REG_WRITE(0, NGPON_GEARBOX, TX_CTL, reg_tx_ctl);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_ngpon_gearbox_tx_ctl_get(ngpon_gearbox_tx_ctl *tx_ctl)
{
    uint32_t reg_tx_ctl=0;

#ifdef VALIDATE_PARMS
    if(!tx_ctl)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, NGPON_GEARBOX, TX_CTL, reg_tx_ctl);

    tx_ctl->cfngpongboxtxfifodatardptr = RU_FIELD_GET(0, NGPON_GEARBOX, TX_CTL, CFNGPONGBOXTXFIFODATARDPTR, reg_tx_ctl);
    tx_ctl->cfngpongboxtxfifovldoff = RU_FIELD_GET(0, NGPON_GEARBOX, TX_CTL, CFNGPONGBOXTXFIFOVLDOFF, reg_tx_ctl);
    tx_ctl->cfngpongboxtxservldflip = RU_FIELD_GET(0, NGPON_GEARBOX, TX_CTL, CFNGPONGBOXTXSERVLDFLIP, reg_tx_ctl);
    tx_ctl->cfngpongboxtxserdataflip = RU_FIELD_GET(0, NGPON_GEARBOX, TX_CTL, CFNGPONGBOXTXSERDATAFLIP, reg_tx_ctl);
    tx_ctl->cfngpongboxtxservldinv = RU_FIELD_GET(0, NGPON_GEARBOX, TX_CTL, CFNGPONGBOXTXSERVLDINV, reg_tx_ctl);
    tx_ctl->cfngpongboxtxserdatainv = RU_FIELD_GET(0, NGPON_GEARBOX, TX_CTL, CFNGPONGBOXTXSERDATAINV, reg_tx_ctl);
    tx_ctl->cfngpongboxtxfifovldptrld = RU_FIELD_GET(0, NGPON_GEARBOX, TX_CTL, CFNGPONGBOXTXFIFOVLDPTRLD, reg_tx_ctl);
    tx_ctl->cfngpongboxtxfifoptrld = RU_FIELD_GET(0, NGPON_GEARBOX, TX_CTL, CFNGPONGBOXTXFIFOPTRLD, reg_tx_ctl);
    tx_ctl->cfngpongboxtxen = RU_FIELD_GET(0, NGPON_GEARBOX, TX_CTL, CFNGPONGBOXTXEN, reg_tx_ctl);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_ngpon_gearbox_status_set(const ngpon_gearbox_status *status)
{
    uint32_t reg_status=0;

#ifdef VALIDATE_PARMS
    if(!status)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((status->ngpontxgboxfifovldptrcol >= _1BITS_MAX_VAL_) ||
       (status->ngponrxgboxstate >= _2BITS_MAX_VAL_) ||
       (status->ngpontxgboxfifodataptrcol >= _1BITS_MAX_VAL_) ||
       (status->ngponrxgboxkcnt >= _3BITS_MAX_VAL_) ||
       (status->ngponrxgboxfifoptrdelta >= _4BITS_MAX_VAL_) ||
       (status->ngponrxgboxsyncacq >= _1BITS_MAX_VAL_) ||
       (status->ngponrxgboxfifoptrcol >= _1BITS_MAX_VAL_) ||
       (status->ngponrxgboxcodeerrcntstat >= _14BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_status = RU_FIELD_SET(0, NGPON_GEARBOX, STATUS, NGPONTXGBOXFIFOVLDPTRCOL, reg_status, status->ngpontxgboxfifovldptrcol);
    reg_status = RU_FIELD_SET(0, NGPON_GEARBOX, STATUS, NGPONRXGBOXSTATE, reg_status, status->ngponrxgboxstate);
    reg_status = RU_FIELD_SET(0, NGPON_GEARBOX, STATUS, NGPONTXGBOXFIFODATAPTRCOL, reg_status, status->ngpontxgboxfifodataptrcol);
    reg_status = RU_FIELD_SET(0, NGPON_GEARBOX, STATUS, NGPONRXGBOXKCNT, reg_status, status->ngponrxgboxkcnt);
    reg_status = RU_FIELD_SET(0, NGPON_GEARBOX, STATUS, NGPONRXGBOXFIFOPTRDELTA, reg_status, status->ngponrxgboxfifoptrdelta);
    reg_status = RU_FIELD_SET(0, NGPON_GEARBOX, STATUS, NGPONRXGBOXSYNCACQ, reg_status, status->ngponrxgboxsyncacq);
    reg_status = RU_FIELD_SET(0, NGPON_GEARBOX, STATUS, NGPONRXGBOXFIFOPTRCOL, reg_status, status->ngponrxgboxfifoptrcol);
    reg_status = RU_FIELD_SET(0, NGPON_GEARBOX, STATUS, NGPONRXGBOXCODEERRCNTSTAT, reg_status, status->ngponrxgboxcodeerrcntstat);

    RU_REG_WRITE(0, NGPON_GEARBOX, STATUS, reg_status);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_ngpon_gearbox_status_get(ngpon_gearbox_status *status)
{
    uint32_t reg_status=0;

#ifdef VALIDATE_PARMS
    if(!status)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, NGPON_GEARBOX, STATUS, reg_status);

    status->ngpontxgboxfifovldptrcol = RU_FIELD_GET(0, NGPON_GEARBOX, STATUS, NGPONTXGBOXFIFOVLDPTRCOL, reg_status);
    status->ngponrxgboxstate = RU_FIELD_GET(0, NGPON_GEARBOX, STATUS, NGPONRXGBOXSTATE, reg_status);
    status->ngpontxgboxfifodataptrcol = RU_FIELD_GET(0, NGPON_GEARBOX, STATUS, NGPONTXGBOXFIFODATAPTRCOL, reg_status);
    status->ngponrxgboxkcnt = RU_FIELD_GET(0, NGPON_GEARBOX, STATUS, NGPONRXGBOXKCNT, reg_status);
    status->ngponrxgboxfifoptrdelta = RU_FIELD_GET(0, NGPON_GEARBOX, STATUS, NGPONRXGBOXFIFOPTRDELTA, reg_status);
    status->ngponrxgboxsyncacq = RU_FIELD_GET(0, NGPON_GEARBOX, STATUS, NGPONRXGBOXSYNCACQ, reg_status);
    status->ngponrxgboxfifoptrcol = RU_FIELD_GET(0, NGPON_GEARBOX, STATUS, NGPONRXGBOXFIFOPTRCOL, reg_status);
    status->ngponrxgboxcodeerrcntstat = RU_FIELD_GET(0, NGPON_GEARBOX, STATUS, NGPONRXGBOXCODEERRCNTSTAT, reg_status);

    return BDMF_ERR_OK;
}

#ifdef USE_BDMF_SHELL
enum
{
    BDMF_rx_ctl_0,
    BDMF_rx_ctl_1,
    BDMF_rx_ctl_2,
    BDMF_rx_ctl_3,
    BDMF_tx_ctl,
    BDMF_status,
};

typedef enum
{
    bdmf_address_rx_ctl_0,
    bdmf_address_rx_ctl_1,
    bdmf_address_rx_ctl_2,
    bdmf_address_rx_ctl_3,
    bdmf_address_tx_ctl,
    bdmf_address_status,
}
bdmf_address;

static int bcm_ngpon_gearbox_cli_set(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    bdmf_error_t err;

    switch(parm[0].value.unumber)
    {
    case BDMF_rx_ctl_0:
    {
        ngpon_gearbox_rx_ctl_0 rx_ctl_0 = { .cfngpongboxrxfifordptr=parm[1].value.unumber, .cfngpongboxrxptrautolddis=parm[2].value.unumber, .cfngpongboxrxmaxbadk=parm[3].value.unumber, .cfngpongboxrxfrmk28only=parm[4].value.unumber, .cfngpongboxrxmaxgoodk=parm[5].value.unumber, .cfngpongboxrxfrchunt=parm[6].value.unumber, .cfngpongboxrxoutdataflip=parm[7].value.unumber, .cfngpongboxrxfrcmuxsel=parm[8].value.unumber, .cfngpongboxrxfrcmuxval=parm[9].value.unumber, .cfngpongboxrx20bdataflip=parm[10].value.unumber, .cfngpongboxrxserdataflip=parm[11].value.unumber, .cfngpongboxrxserdatainv=parm[12].value.unumber, .cfngpongboxrxfifoptrld=parm[13].value.unumber, .cfngpongboxrxswsynchold=parm[14].value.unumber, .cfngpongboxrxmode=parm[15].value.unumber, .cfngpongboxrxen=parm[16].value.unumber, .cfngpongboxrstn=parm[17].value.unumber};
        err = ag_drv_ngpon_gearbox_rx_ctl_0_set(&rx_ctl_0);
        break;
    }
    case BDMF_rx_ctl_1:
        err = ag_drv_ngpon_gearbox_rx_ctl_1_set(parm[1].value.unumber);
        break;
    case BDMF_rx_ctl_2:
        err = ag_drv_ngpon_gearbox_rx_ctl_2_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case BDMF_rx_ctl_3:
        err = ag_drv_ngpon_gearbox_rx_ctl_3_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case BDMF_tx_ctl:
    {
        ngpon_gearbox_tx_ctl tx_ctl = { .cfngpongboxtxfifodatardptr=parm[1].value.unumber, .cfngpongboxtxfifovldoff=parm[2].value.unumber, .cfngpongboxtxservldflip=parm[3].value.unumber, .cfngpongboxtxserdataflip=parm[4].value.unumber, .cfngpongboxtxservldinv=parm[5].value.unumber, .cfngpongboxtxserdatainv=parm[6].value.unumber, .cfngpongboxtxfifovldptrld=parm[7].value.unumber, .cfngpongboxtxfifoptrld=parm[8].value.unumber, .cfngpongboxtxen=parm[9].value.unumber};
        err = ag_drv_ngpon_gearbox_tx_ctl_set(&tx_ctl);
        break;
    }
    case BDMF_status:
    {
        ngpon_gearbox_status status = { .ngpontxgboxfifovldptrcol=parm[1].value.unumber, .ngponrxgboxstate=parm[2].value.unumber, .ngpontxgboxfifodataptrcol=parm[3].value.unumber, .ngponrxgboxkcnt=parm[4].value.unumber, .ngponrxgboxfifoptrdelta=parm[5].value.unumber, .ngponrxgboxsyncacq=parm[6].value.unumber, .ngponrxgboxfifoptrcol=parm[7].value.unumber, .ngponrxgboxcodeerrcntstat=parm[8].value.unumber};
        err = ag_drv_ngpon_gearbox_status_set(&status);
        break;
    }
    default:
        err = BDMF_ERR_NOT_SUPPORTED;
        break;
    }
    return err;
}

static int bcm_ngpon_gearbox_cli_get(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    bdmf_error_t err;

    switch(parm[0].value.unumber)
    {
    case BDMF_rx_ctl_0:
    {
        ngpon_gearbox_rx_ctl_0 rx_ctl_0;
        err = ag_drv_ngpon_gearbox_rx_ctl_0_get(&rx_ctl_0);
        bdmf_session_print(session, "cfngpongboxrxfifordptr = %u = 0x%x\n", rx_ctl_0.cfngpongboxrxfifordptr, rx_ctl_0.cfngpongboxrxfifordptr);
        bdmf_session_print(session, "cfngpongboxrxptrautolddis = %u = 0x%x\n", rx_ctl_0.cfngpongboxrxptrautolddis, rx_ctl_0.cfngpongboxrxptrautolddis);
        bdmf_session_print(session, "cfngpongboxrxmaxbadk = %u = 0x%x\n", rx_ctl_0.cfngpongboxrxmaxbadk, rx_ctl_0.cfngpongboxrxmaxbadk);
        bdmf_session_print(session, "cfngpongboxrxfrmk28only = %u = 0x%x\n", rx_ctl_0.cfngpongboxrxfrmk28only, rx_ctl_0.cfngpongboxrxfrmk28only);
        bdmf_session_print(session, "cfngpongboxrxmaxgoodk = %u = 0x%x\n", rx_ctl_0.cfngpongboxrxmaxgoodk, rx_ctl_0.cfngpongboxrxmaxgoodk);
        bdmf_session_print(session, "cfngpongboxrxfrchunt = %u = 0x%x\n", rx_ctl_0.cfngpongboxrxfrchunt, rx_ctl_0.cfngpongboxrxfrchunt);
        bdmf_session_print(session, "cfngpongboxrxoutdataflip = %u = 0x%x\n", rx_ctl_0.cfngpongboxrxoutdataflip, rx_ctl_0.cfngpongboxrxoutdataflip);
        bdmf_session_print(session, "cfngpongboxrxfrcmuxsel = %u = 0x%x\n", rx_ctl_0.cfngpongboxrxfrcmuxsel, rx_ctl_0.cfngpongboxrxfrcmuxsel);
        bdmf_session_print(session, "cfngpongboxrxfrcmuxval = %u = 0x%x\n", rx_ctl_0.cfngpongboxrxfrcmuxval, rx_ctl_0.cfngpongboxrxfrcmuxval);
        bdmf_session_print(session, "cfngpongboxrx20bdataflip = %u = 0x%x\n", rx_ctl_0.cfngpongboxrx20bdataflip, rx_ctl_0.cfngpongboxrx20bdataflip);
        bdmf_session_print(session, "cfngpongboxrxserdataflip = %u = 0x%x\n", rx_ctl_0.cfngpongboxrxserdataflip, rx_ctl_0.cfngpongboxrxserdataflip);
        bdmf_session_print(session, "cfngpongboxrxserdatainv = %u = 0x%x\n", rx_ctl_0.cfngpongboxrxserdatainv, rx_ctl_0.cfngpongboxrxserdatainv);
        bdmf_session_print(session, "cfngpongboxrxfifoptrld = %u = 0x%x\n", rx_ctl_0.cfngpongboxrxfifoptrld, rx_ctl_0.cfngpongboxrxfifoptrld);
        bdmf_session_print(session, "cfngpongboxrxswsynchold = %u = 0x%x\n", rx_ctl_0.cfngpongboxrxswsynchold, rx_ctl_0.cfngpongboxrxswsynchold);
        bdmf_session_print(session, "cfngpongboxrxmode = %u = 0x%x\n", rx_ctl_0.cfngpongboxrxmode, rx_ctl_0.cfngpongboxrxmode);
        bdmf_session_print(session, "cfngpongboxrxen = %u = 0x%x\n", rx_ctl_0.cfngpongboxrxen, rx_ctl_0.cfngpongboxrxen);
        bdmf_session_print(session, "cfngpongboxrstn = %u = 0x%x\n", rx_ctl_0.cfngpongboxrstn, rx_ctl_0.cfngpongboxrstn);
        break;
    }
    case BDMF_rx_ctl_1:
    {
        uint32_t cfngpongboxrxmaxtimercnt;
        err = ag_drv_ngpon_gearbox_rx_ctl_1_get(&cfngpongboxrxmaxtimercnt);
        bdmf_session_print(session, "cfngpongboxrxmaxtimercnt = %u = 0x%x\n", cfngpongboxrxmaxtimercnt, cfngpongboxrxmaxtimercnt);
        break;
    }
    case BDMF_rx_ctl_2:
    {
        uint16_t cfngpongboxrxk28d5rdp;
        uint16_t cfngpongboxrxk28d5rdn;
        err = ag_drv_ngpon_gearbox_rx_ctl_2_get(&cfngpongboxrxk28d5rdp, &cfngpongboxrxk28d5rdn);
        bdmf_session_print(session, "cfngpongboxrxk28d5rdp = %u = 0x%x\n", cfngpongboxrxk28d5rdp, cfngpongboxrxk28d5rdp);
        bdmf_session_print(session, "cfngpongboxrxk28d5rdn = %u = 0x%x\n", cfngpongboxrxk28d5rdn, cfngpongboxrxk28d5rdn);
        break;
    }
    case BDMF_rx_ctl_3:
    {
        uint16_t cfngpongboxrxd5d7rdp;
        uint16_t cfngpongboxrxd5d7rdn;
        err = ag_drv_ngpon_gearbox_rx_ctl_3_get(&cfngpongboxrxd5d7rdp, &cfngpongboxrxd5d7rdn);
        bdmf_session_print(session, "cfngpongboxrxd5d7rdp = %u = 0x%x\n", cfngpongboxrxd5d7rdp, cfngpongboxrxd5d7rdp);
        bdmf_session_print(session, "cfngpongboxrxd5d7rdn = %u = 0x%x\n", cfngpongboxrxd5d7rdn, cfngpongboxrxd5d7rdn);
        break;
    }
    case BDMF_tx_ctl:
    {
        ngpon_gearbox_tx_ctl tx_ctl;
        err = ag_drv_ngpon_gearbox_tx_ctl_get(&tx_ctl);
        bdmf_session_print(session, "cfngpongboxtxfifodatardptr = %u = 0x%x\n", tx_ctl.cfngpongboxtxfifodatardptr, tx_ctl.cfngpongboxtxfifodatardptr);
        bdmf_session_print(session, "cfngpongboxtxfifovldoff = %u = 0x%x\n", tx_ctl.cfngpongboxtxfifovldoff, tx_ctl.cfngpongboxtxfifovldoff);
        bdmf_session_print(session, "cfngpongboxtxservldflip = %u = 0x%x\n", tx_ctl.cfngpongboxtxservldflip, tx_ctl.cfngpongboxtxservldflip);
        bdmf_session_print(session, "cfngpongboxtxserdataflip = %u = 0x%x\n", tx_ctl.cfngpongboxtxserdataflip, tx_ctl.cfngpongboxtxserdataflip);
        bdmf_session_print(session, "cfngpongboxtxservldinv = %u = 0x%x\n", tx_ctl.cfngpongboxtxservldinv, tx_ctl.cfngpongboxtxservldinv);
        bdmf_session_print(session, "cfngpongboxtxserdatainv = %u = 0x%x\n", tx_ctl.cfngpongboxtxserdatainv, tx_ctl.cfngpongboxtxserdatainv);
        bdmf_session_print(session, "cfngpongboxtxfifovldptrld = %u = 0x%x\n", tx_ctl.cfngpongboxtxfifovldptrld, tx_ctl.cfngpongboxtxfifovldptrld);
        bdmf_session_print(session, "cfngpongboxtxfifoptrld = %u = 0x%x\n", tx_ctl.cfngpongboxtxfifoptrld, tx_ctl.cfngpongboxtxfifoptrld);
        bdmf_session_print(session, "cfngpongboxtxen = %u = 0x%x\n", tx_ctl.cfngpongboxtxen, tx_ctl.cfngpongboxtxen);
        break;
    }
    case BDMF_status:
    {
        ngpon_gearbox_status status;
        err = ag_drv_ngpon_gearbox_status_get(&status);
        bdmf_session_print(session, "ngpontxgboxfifovldptrcol = %u = 0x%x\n", status.ngpontxgboxfifovldptrcol, status.ngpontxgboxfifovldptrcol);
        bdmf_session_print(session, "ngponrxgboxstate = %u = 0x%x\n", status.ngponrxgboxstate, status.ngponrxgboxstate);
        bdmf_session_print(session, "ngpontxgboxfifodataptrcol = %u = 0x%x\n", status.ngpontxgboxfifodataptrcol, status.ngpontxgboxfifodataptrcol);
        bdmf_session_print(session, "ngponrxgboxkcnt = %u = 0x%x\n", status.ngponrxgboxkcnt, status.ngponrxgboxkcnt);
        bdmf_session_print(session, "ngponrxgboxfifoptrdelta = %u = 0x%x\n", status.ngponrxgboxfifoptrdelta, status.ngponrxgboxfifoptrdelta);
        bdmf_session_print(session, "ngponrxgboxsyncacq = %u = 0x%x\n", status.ngponrxgboxsyncacq, status.ngponrxgboxsyncacq);
        bdmf_session_print(session, "ngponrxgboxfifoptrcol = %u = 0x%x\n", status.ngponrxgboxfifoptrcol, status.ngponrxgboxfifoptrcol);
        bdmf_session_print(session, "ngponrxgboxcodeerrcntstat = %u = 0x%x\n", status.ngponrxgboxcodeerrcntstat, status.ngponrxgboxcodeerrcntstat);
        break;
    }
    default:
        err = BDMF_ERR_NOT_SUPPORTED;
        break;
    }
    return err;
}

static int bcm_ngpon_gearbox_cli_test(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    bdmf_test_method m = parm[0].value.unumber;
    bdmf_error_t err = BDMF_ERR_OK;

    {
        ngpon_gearbox_rx_ctl_0 rx_ctl_0 = {.cfngpongboxrxfifordptr=gtmv(m, 4), .cfngpongboxrxptrautolddis=gtmv(m, 1), .cfngpongboxrxmaxbadk=gtmv(m, 3), .cfngpongboxrxfrmk28only=gtmv(m, 1), .cfngpongboxrxmaxgoodk=gtmv(m, 3), .cfngpongboxrxfrchunt=gtmv(m, 1), .cfngpongboxrxoutdataflip=gtmv(m, 1), .cfngpongboxrxfrcmuxsel=gtmv(m, 1), .cfngpongboxrxfrcmuxval=gtmv(m, 5), .cfngpongboxrx20bdataflip=gtmv(m, 1), .cfngpongboxrxserdataflip=gtmv(m, 1), .cfngpongboxrxserdatainv=gtmv(m, 1), .cfngpongboxrxfifoptrld=gtmv(m, 1), .cfngpongboxrxswsynchold=gtmv(m, 1), .cfngpongboxrxmode=gtmv(m, 1), .cfngpongboxrxen=gtmv(m, 1), .cfngpongboxrstn=gtmv(m, 1)};
        if(!err) bdmf_session_print(session, "ag_drv_ngpon_gearbox_rx_ctl_0_set( %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u)\n", rx_ctl_0.cfngpongboxrxfifordptr, rx_ctl_0.cfngpongboxrxptrautolddis, rx_ctl_0.cfngpongboxrxmaxbadk, rx_ctl_0.cfngpongboxrxfrmk28only, rx_ctl_0.cfngpongboxrxmaxgoodk, rx_ctl_0.cfngpongboxrxfrchunt, rx_ctl_0.cfngpongboxrxoutdataflip, rx_ctl_0.cfngpongboxrxfrcmuxsel, rx_ctl_0.cfngpongboxrxfrcmuxval, rx_ctl_0.cfngpongboxrx20bdataflip, rx_ctl_0.cfngpongboxrxserdataflip, rx_ctl_0.cfngpongboxrxserdatainv, rx_ctl_0.cfngpongboxrxfifoptrld, rx_ctl_0.cfngpongboxrxswsynchold, rx_ctl_0.cfngpongboxrxmode, rx_ctl_0.cfngpongboxrxen, rx_ctl_0.cfngpongboxrstn);
        if(!err) ag_drv_ngpon_gearbox_rx_ctl_0_set(&rx_ctl_0);
        if(!err) ag_drv_ngpon_gearbox_rx_ctl_0_get( &rx_ctl_0);
        if(!err) bdmf_session_print(session, "ag_drv_ngpon_gearbox_rx_ctl_0_get( %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u)\n", rx_ctl_0.cfngpongboxrxfifordptr, rx_ctl_0.cfngpongboxrxptrautolddis, rx_ctl_0.cfngpongboxrxmaxbadk, rx_ctl_0.cfngpongboxrxfrmk28only, rx_ctl_0.cfngpongboxrxmaxgoodk, rx_ctl_0.cfngpongboxrxfrchunt, rx_ctl_0.cfngpongboxrxoutdataflip, rx_ctl_0.cfngpongboxrxfrcmuxsel, rx_ctl_0.cfngpongboxrxfrcmuxval, rx_ctl_0.cfngpongboxrx20bdataflip, rx_ctl_0.cfngpongboxrxserdataflip, rx_ctl_0.cfngpongboxrxserdatainv, rx_ctl_0.cfngpongboxrxfifoptrld, rx_ctl_0.cfngpongboxrxswsynchold, rx_ctl_0.cfngpongboxrxmode, rx_ctl_0.cfngpongboxrxen, rx_ctl_0.cfngpongboxrstn);
        if(err || rx_ctl_0.cfngpongboxrxfifordptr!=gtmv(m, 4) || rx_ctl_0.cfngpongboxrxptrautolddis!=gtmv(m, 1) || rx_ctl_0.cfngpongboxrxmaxbadk!=gtmv(m, 3) || rx_ctl_0.cfngpongboxrxfrmk28only!=gtmv(m, 1) || rx_ctl_0.cfngpongboxrxmaxgoodk!=gtmv(m, 3) || rx_ctl_0.cfngpongboxrxfrchunt!=gtmv(m, 1) || rx_ctl_0.cfngpongboxrxoutdataflip!=gtmv(m, 1) || rx_ctl_0.cfngpongboxrxfrcmuxsel!=gtmv(m, 1) || rx_ctl_0.cfngpongboxrxfrcmuxval!=gtmv(m, 5) || rx_ctl_0.cfngpongboxrx20bdataflip!=gtmv(m, 1) || rx_ctl_0.cfngpongboxrxserdataflip!=gtmv(m, 1) || rx_ctl_0.cfngpongboxrxserdatainv!=gtmv(m, 1) || rx_ctl_0.cfngpongboxrxfifoptrld!=gtmv(m, 1) || rx_ctl_0.cfngpongboxrxswsynchold!=gtmv(m, 1) || rx_ctl_0.cfngpongboxrxmode!=gtmv(m, 1) || rx_ctl_0.cfngpongboxrxen!=gtmv(m, 1) || rx_ctl_0.cfngpongboxrstn!=gtmv(m, 1))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint32_t cfngpongboxrxmaxtimercnt=gtmv(m, 17);
        if(!err) bdmf_session_print(session, "ag_drv_ngpon_gearbox_rx_ctl_1_set( %u)\n", cfngpongboxrxmaxtimercnt);
        if(!err) ag_drv_ngpon_gearbox_rx_ctl_1_set(cfngpongboxrxmaxtimercnt);
        if(!err) ag_drv_ngpon_gearbox_rx_ctl_1_get( &cfngpongboxrxmaxtimercnt);
        if(!err) bdmf_session_print(session, "ag_drv_ngpon_gearbox_rx_ctl_1_get( %u)\n", cfngpongboxrxmaxtimercnt);
        if(err || cfngpongboxrxmaxtimercnt!=gtmv(m, 17))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint16_t cfngpongboxrxk28d5rdp=gtmv(m, 10);
        uint16_t cfngpongboxrxk28d5rdn=gtmv(m, 10);
        if(!err) bdmf_session_print(session, "ag_drv_ngpon_gearbox_rx_ctl_2_set( %u %u)\n", cfngpongboxrxk28d5rdp, cfngpongboxrxk28d5rdn);
        if(!err) ag_drv_ngpon_gearbox_rx_ctl_2_set(cfngpongboxrxk28d5rdp, cfngpongboxrxk28d5rdn);
        if(!err) ag_drv_ngpon_gearbox_rx_ctl_2_get( &cfngpongboxrxk28d5rdp, &cfngpongboxrxk28d5rdn);
        if(!err) bdmf_session_print(session, "ag_drv_ngpon_gearbox_rx_ctl_2_get( %u %u)\n", cfngpongboxrxk28d5rdp, cfngpongboxrxk28d5rdn);
        if(err || cfngpongboxrxk28d5rdp!=gtmv(m, 10) || cfngpongboxrxk28d5rdn!=gtmv(m, 10))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint16_t cfngpongboxrxd5d7rdp=gtmv(m, 10);
        uint16_t cfngpongboxrxd5d7rdn=gtmv(m, 10);
        if(!err) bdmf_session_print(session, "ag_drv_ngpon_gearbox_rx_ctl_3_set( %u %u)\n", cfngpongboxrxd5d7rdp, cfngpongboxrxd5d7rdn);
        if(!err) ag_drv_ngpon_gearbox_rx_ctl_3_set(cfngpongboxrxd5d7rdp, cfngpongboxrxd5d7rdn);
        if(!err) ag_drv_ngpon_gearbox_rx_ctl_3_get( &cfngpongboxrxd5d7rdp, &cfngpongboxrxd5d7rdn);
        if(!err) bdmf_session_print(session, "ag_drv_ngpon_gearbox_rx_ctl_3_get( %u %u)\n", cfngpongboxrxd5d7rdp, cfngpongboxrxd5d7rdn);
        if(err || cfngpongboxrxd5d7rdp!=gtmv(m, 10) || cfngpongboxrxd5d7rdn!=gtmv(m, 10))
            return err ? err : BDMF_ERR_IO;
    }
    {
        ngpon_gearbox_tx_ctl tx_ctl = {.cfngpongboxtxfifodatardptr=gtmv(m, 4), .cfngpongboxtxfifovldoff=gtmv(m, 5), .cfngpongboxtxservldflip=gtmv(m, 1), .cfngpongboxtxserdataflip=gtmv(m, 1), .cfngpongboxtxservldinv=gtmv(m, 1), .cfngpongboxtxserdatainv=gtmv(m, 1), .cfngpongboxtxfifovldptrld=gtmv(m, 1), .cfngpongboxtxfifoptrld=gtmv(m, 1), .cfngpongboxtxen=gtmv(m, 1)};
        if(!err) bdmf_session_print(session, "ag_drv_ngpon_gearbox_tx_ctl_set( %u %u %u %u %u %u %u %u %u)\n", tx_ctl.cfngpongboxtxfifodatardptr, tx_ctl.cfngpongboxtxfifovldoff, tx_ctl.cfngpongboxtxservldflip, tx_ctl.cfngpongboxtxserdataflip, tx_ctl.cfngpongboxtxservldinv, tx_ctl.cfngpongboxtxserdatainv, tx_ctl.cfngpongboxtxfifovldptrld, tx_ctl.cfngpongboxtxfifoptrld, tx_ctl.cfngpongboxtxen);
        if(!err) ag_drv_ngpon_gearbox_tx_ctl_set(&tx_ctl);
        if(!err) ag_drv_ngpon_gearbox_tx_ctl_get( &tx_ctl);
        if(!err) bdmf_session_print(session, "ag_drv_ngpon_gearbox_tx_ctl_get( %u %u %u %u %u %u %u %u %u)\n", tx_ctl.cfngpongboxtxfifodatardptr, tx_ctl.cfngpongboxtxfifovldoff, tx_ctl.cfngpongboxtxservldflip, tx_ctl.cfngpongboxtxserdataflip, tx_ctl.cfngpongboxtxservldinv, tx_ctl.cfngpongboxtxserdatainv, tx_ctl.cfngpongboxtxfifovldptrld, tx_ctl.cfngpongboxtxfifoptrld, tx_ctl.cfngpongboxtxen);
        if(err || tx_ctl.cfngpongboxtxfifodatardptr!=gtmv(m, 4) || tx_ctl.cfngpongboxtxfifovldoff!=gtmv(m, 5) || tx_ctl.cfngpongboxtxservldflip!=gtmv(m, 1) || tx_ctl.cfngpongboxtxserdataflip!=gtmv(m, 1) || tx_ctl.cfngpongboxtxservldinv!=gtmv(m, 1) || tx_ctl.cfngpongboxtxserdatainv!=gtmv(m, 1) || tx_ctl.cfngpongboxtxfifovldptrld!=gtmv(m, 1) || tx_ctl.cfngpongboxtxfifoptrld!=gtmv(m, 1) || tx_ctl.cfngpongboxtxen!=gtmv(m, 1))
            return err ? err : BDMF_ERR_IO;
    }
    {
        ngpon_gearbox_status status = {.ngpontxgboxfifovldptrcol=gtmv(m, 1), .ngponrxgboxstate=gtmv(m, 2), .ngpontxgboxfifodataptrcol=gtmv(m, 1), .ngponrxgboxkcnt=gtmv(m, 3), .ngponrxgboxfifoptrdelta=gtmv(m, 4), .ngponrxgboxsyncacq=gtmv(m, 1), .ngponrxgboxfifoptrcol=gtmv(m, 1), .ngponrxgboxcodeerrcntstat=gtmv(m, 14)};
        if(!err) bdmf_session_print(session, "ag_drv_ngpon_gearbox_status_set( %u %u %u %u %u %u %u %u)\n", status.ngpontxgboxfifovldptrcol, status.ngponrxgboxstate, status.ngpontxgboxfifodataptrcol, status.ngponrxgboxkcnt, status.ngponrxgboxfifoptrdelta, status.ngponrxgboxsyncacq, status.ngponrxgboxfifoptrcol, status.ngponrxgboxcodeerrcntstat);
        if(!err) ag_drv_ngpon_gearbox_status_set(&status);
        if(!err) ag_drv_ngpon_gearbox_status_get( &status);
        if(!err) bdmf_session_print(session, "ag_drv_ngpon_gearbox_status_get( %u %u %u %u %u %u %u %u)\n", status.ngpontxgboxfifovldptrcol, status.ngponrxgboxstate, status.ngpontxgboxfifodataptrcol, status.ngponrxgboxkcnt, status.ngponrxgboxfifoptrdelta, status.ngponrxgboxsyncacq, status.ngponrxgboxfifoptrcol, status.ngponrxgboxcodeerrcntstat);
        if(err || status.ngpontxgboxfifovldptrcol!=gtmv(m, 1) || status.ngponrxgboxstate!=gtmv(m, 2) || status.ngpontxgboxfifodataptrcol!=gtmv(m, 1) || status.ngponrxgboxkcnt!=gtmv(m, 3) || status.ngponrxgboxfifoptrdelta!=gtmv(m, 4) || status.ngponrxgboxsyncacq!=gtmv(m, 1) || status.ngponrxgboxfifoptrcol!=gtmv(m, 1) || status.ngponrxgboxcodeerrcntstat!=gtmv(m, 14))
            return err ? err : BDMF_ERR_IO;
    }
    return err;
}

static int bcm_ngpon_gearbox_cli_address(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
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
    case bdmf_address_rx_ctl_0 : reg = &RU_REG(NGPON_GEARBOX, RX_CTL_0); blk = &RU_BLK(NGPON_GEARBOX); break;
    case bdmf_address_rx_ctl_1 : reg = &RU_REG(NGPON_GEARBOX, RX_CTL_1); blk = &RU_BLK(NGPON_GEARBOX); break;
    case bdmf_address_rx_ctl_2 : reg = &RU_REG(NGPON_GEARBOX, RX_CTL_2); blk = &RU_BLK(NGPON_GEARBOX); break;
    case bdmf_address_rx_ctl_3 : reg = &RU_REG(NGPON_GEARBOX, RX_CTL_3); blk = &RU_BLK(NGPON_GEARBOX); break;
    case bdmf_address_tx_ctl : reg = &RU_REG(NGPON_GEARBOX, TX_CTL); blk = &RU_BLK(NGPON_GEARBOX); break;
    case bdmf_address_status : reg = &RU_REG(NGPON_GEARBOX, STATUS); blk = &RU_BLK(NGPON_GEARBOX); break;
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
            bdmf_session_print(session, "(%3u) 0x%08X\n", i, blk->addr[i]+reg->addr);
    return 0;
}

bdmfmon_handle_t ag_drv_ngpon_gearbox_cli_init(bdmfmon_handle_t driver_dir)
{
    bdmfmon_handle_t dir;

    if ((dir = bdmfmon_dir_find(driver_dir, "ngpon_gearbox"))!=NULL)
        return dir;
    dir = bdmfmon_dir_add(driver_dir, "ngpon_gearbox", "ngpon_gearbox", BDMF_ACCESS_ADMIN, NULL);

    {
        static bdmfmon_cmd_parm_t set_rx_ctl_0[]={
            BDMFMON_MAKE_PARM("cfngpongboxrxfifordptr", "cfngpongboxrxfifordptr", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("cfngpongboxrxptrautolddis", "cfngpongboxrxptrautolddis", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("cfngpongboxrxmaxbadk", "cfngpongboxrxmaxbadk", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("cfngpongboxrxfrmk28only", "cfngpongboxrxfrmk28only", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("cfngpongboxrxmaxgoodk", "cfngpongboxrxmaxgoodk", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("cfngpongboxrxfrchunt", "cfngpongboxrxfrchunt", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("cfngpongboxrxoutdataflip", "cfngpongboxrxoutdataflip", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("cfngpongboxrxfrcmuxsel", "cfngpongboxrxfrcmuxsel", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("cfngpongboxrxfrcmuxval", "cfngpongboxrxfrcmuxval", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("cfngpongboxrx20bdataflip", "cfngpongboxrx20bdataflip", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("cfngpongboxrxserdataflip", "cfngpongboxrxserdataflip", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("cfngpongboxrxserdatainv", "cfngpongboxrxserdatainv", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("cfngpongboxrxfifoptrld", "cfngpongboxrxfifoptrld", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("cfngpongboxrxswsynchold", "cfngpongboxrxswsynchold", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("cfngpongboxrxmode", "cfngpongboxrxmode", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("cfngpongboxrxen", "cfngpongboxrxen", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("cfngpongboxrstn", "cfngpongboxrstn", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_rx_ctl_1[]={
            BDMFMON_MAKE_PARM("cfngpongboxrxmaxtimercnt", "cfngpongboxrxmaxtimercnt", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_rx_ctl_2[]={
            BDMFMON_MAKE_PARM("cfngpongboxrxk28d5rdp", "cfngpongboxrxk28d5rdp", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("cfngpongboxrxk28d5rdn", "cfngpongboxrxk28d5rdn", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_rx_ctl_3[]={
            BDMFMON_MAKE_PARM("cfngpongboxrxd5d7rdp", "cfngpongboxrxd5d7rdp", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("cfngpongboxrxd5d7rdn", "cfngpongboxrxd5d7rdn", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_tx_ctl[]={
            BDMFMON_MAKE_PARM("cfngpongboxtxfifodatardptr", "cfngpongboxtxfifodatardptr", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("cfngpongboxtxfifovldoff", "cfngpongboxtxfifovldoff", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("cfngpongboxtxservldflip", "cfngpongboxtxservldflip", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("cfngpongboxtxserdataflip", "cfngpongboxtxserdataflip", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("cfngpongboxtxservldinv", "cfngpongboxtxservldinv", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("cfngpongboxtxserdatainv", "cfngpongboxtxserdatainv", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("cfngpongboxtxfifovldptrld", "cfngpongboxtxfifovldptrld", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("cfngpongboxtxfifoptrld", "cfngpongboxtxfifoptrld", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("cfngpongboxtxen", "cfngpongboxtxen", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_status[]={
            BDMFMON_MAKE_PARM("ngpontxgboxfifovldptrcol", "ngpontxgboxfifovldptrcol", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("ngponrxgboxstate", "ngponrxgboxstate", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("ngpontxgboxfifodataptrcol", "ngpontxgboxfifodataptrcol", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("ngponrxgboxkcnt", "ngponrxgboxkcnt", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("ngponrxgboxfifoptrdelta", "ngponrxgboxfifoptrdelta", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("ngponrxgboxsyncacq", "ngponrxgboxsyncacq", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("ngponrxgboxfifoptrcol", "ngponrxgboxfifoptrcol", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("ngponrxgboxcodeerrcntstat", "ngponrxgboxcodeerrcntstat", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_enum_val_t selector_table[] = {
            { .name="rx_ctl_0", .val=BDMF_rx_ctl_0, .parms=set_rx_ctl_0 },
            { .name="rx_ctl_1", .val=BDMF_rx_ctl_1, .parms=set_rx_ctl_1 },
            { .name="rx_ctl_2", .val=BDMF_rx_ctl_2, .parms=set_rx_ctl_2 },
            { .name="rx_ctl_3", .val=BDMF_rx_ctl_3, .parms=set_rx_ctl_3 },
            { .name="tx_ctl", .val=BDMF_tx_ctl, .parms=set_tx_ctl },
            { .name="status", .val=BDMF_status, .parms=set_status },
            BDMFMON_ENUM_LAST
        };
        BDMFMON_MAKE_CMD(dir, "set", "set", bcm_ngpon_gearbox_cli_set,
            BDMFMON_MAKE_PARM_SELECTOR("purpose", "purpose", selector_table, 0));
    }
    {
        static bdmfmon_cmd_parm_t set_default[]={
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_enum_val_t selector_table[] = {
            { .name="rx_ctl_0", .val=BDMF_rx_ctl_0, .parms=set_default },
            { .name="rx_ctl_1", .val=BDMF_rx_ctl_1, .parms=set_default },
            { .name="rx_ctl_2", .val=BDMF_rx_ctl_2, .parms=set_default },
            { .name="rx_ctl_3", .val=BDMF_rx_ctl_3, .parms=set_default },
            { .name="tx_ctl", .val=BDMF_tx_ctl, .parms=set_default },
            { .name="status", .val=BDMF_status, .parms=set_default },
            BDMFMON_ENUM_LAST
        };
        BDMFMON_MAKE_CMD(dir, "get", "get", bcm_ngpon_gearbox_cli_get,
            BDMFMON_MAKE_PARM_SELECTOR("purpose", "purpose", selector_table, 0));
    }
    {
        static bdmfmon_enum_val_t enum_table_test_method[] = {
            { .name="low" , .val=bdmf_test_method_low },
            { .name="mid" , .val=bdmf_test_method_mid },
            { .name="high" , .val=bdmf_test_method_high },
            BDMFMON_ENUM_LAST
        };
        BDMFMON_MAKE_CMD(dir, "test", "test", bcm_ngpon_gearbox_cli_test,
            BDMFMON_MAKE_PARM_ENUM("method", "low: 0000, mid: 1000, high: 1111", enum_table_test_method, 0));
    }
    {
        static bdmfmon_enum_val_t enum_table_address[] = {
            { .name="RX_CTL_0" , .val=bdmf_address_rx_ctl_0 },
            { .name="RX_CTL_1" , .val=bdmf_address_rx_ctl_1 },
            { .name="RX_CTL_2" , .val=bdmf_address_rx_ctl_2 },
            { .name="RX_CTL_3" , .val=bdmf_address_rx_ctl_3 },
            { .name="TX_CTL" , .val=bdmf_address_tx_ctl },
            { .name="STATUS" , .val=bdmf_address_status },
            BDMFMON_ENUM_LAST
        };
        BDMFMON_MAKE_CMD(dir, "address", "address", bcm_ngpon_gearbox_cli_address,
            BDMFMON_MAKE_PARM_ENUM("method", "method", enum_table_address, 0),
            BDMFMON_MAKE_PARM("index2", "onu_id/alloc_id/port_id/etc...", BDMFMON_PARM_NUMBER, BDMFMON_PARM_FLAG_OPTIONAL));
    }
    return dir;
}
#endif /* USE_BDMF_SHELL */

