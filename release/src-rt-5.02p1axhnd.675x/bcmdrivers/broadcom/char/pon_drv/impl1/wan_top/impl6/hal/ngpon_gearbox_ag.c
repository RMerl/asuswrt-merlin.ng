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
    if((rx_ctl_0->cfngpongboxrstn >= _1BITS_MAX_VAL_) ||
       (rx_ctl_0->cfngpongboxrxen >= _1BITS_MAX_VAL_) ||
       (rx_ctl_0->cfngpongboxrxmode >= _1BITS_MAX_VAL_) ||
       (rx_ctl_0->cfngpongboxrxswsynchold >= _1BITS_MAX_VAL_) ||
       (rx_ctl_0->cfngpongboxrxfifoptrld >= _1BITS_MAX_VAL_) ||
       (rx_ctl_0->cfngpongboxrxserdatainv >= _1BITS_MAX_VAL_) ||
       (rx_ctl_0->cfngpongboxrxserdataflip >= _1BITS_MAX_VAL_) ||
       (rx_ctl_0->cfngpongboxrx20bdataflip >= _1BITS_MAX_VAL_) ||
       (rx_ctl_0->cfngpongboxrxfrcmuxval >= _5BITS_MAX_VAL_) ||
       (rx_ctl_0->cfngpongboxrxfrcmuxsel >= _1BITS_MAX_VAL_) ||
       (rx_ctl_0->cfngpongboxrxoutdataflip >= _1BITS_MAX_VAL_) ||
       (rx_ctl_0->cfngpongboxrxfrchunt >= _1BITS_MAX_VAL_) ||
       (rx_ctl_0->cfngpongboxrxmaxgoodk >= _3BITS_MAX_VAL_) ||
       (rx_ctl_0->cfngpongboxrxfrmk28only >= _1BITS_MAX_VAL_) ||
       (rx_ctl_0->cfngpongboxrxmaxbadk >= _3BITS_MAX_VAL_) ||
       (rx_ctl_0->cfngpongboxrxptrautolddis >= _1BITS_MAX_VAL_) ||
       (rx_ctl_0->cfngpongboxrxfifordptr >= _4BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_rx_ctl_0 = RU_FIELD_SET(0, NGPON_GEARBOX, RX_CTL_0, CFNGPONGBOXRSTN, reg_rx_ctl_0, rx_ctl_0->cfngpongboxrstn);
    reg_rx_ctl_0 = RU_FIELD_SET(0, NGPON_GEARBOX, RX_CTL_0, CFNGPONGBOXRXEN, reg_rx_ctl_0, rx_ctl_0->cfngpongboxrxen);
    reg_rx_ctl_0 = RU_FIELD_SET(0, NGPON_GEARBOX, RX_CTL_0, CFNGPONGBOXRXMODE, reg_rx_ctl_0, rx_ctl_0->cfngpongboxrxmode);
    reg_rx_ctl_0 = RU_FIELD_SET(0, NGPON_GEARBOX, RX_CTL_0, CFNGPONGBOXRXSWSYNCHOLD, reg_rx_ctl_0, rx_ctl_0->cfngpongboxrxswsynchold);
    reg_rx_ctl_0 = RU_FIELD_SET(0, NGPON_GEARBOX, RX_CTL_0, CFNGPONGBOXRXFIFOPTRLD, reg_rx_ctl_0, rx_ctl_0->cfngpongboxrxfifoptrld);
    reg_rx_ctl_0 = RU_FIELD_SET(0, NGPON_GEARBOX, RX_CTL_0, CFNGPONGBOXRXSERDATAINV, reg_rx_ctl_0, rx_ctl_0->cfngpongboxrxserdatainv);
    reg_rx_ctl_0 = RU_FIELD_SET(0, NGPON_GEARBOX, RX_CTL_0, CFNGPONGBOXRXSERDATAFLIP, reg_rx_ctl_0, rx_ctl_0->cfngpongboxrxserdataflip);
    reg_rx_ctl_0 = RU_FIELD_SET(0, NGPON_GEARBOX, RX_CTL_0, CFNGPONGBOXRX20BDATAFLIP, reg_rx_ctl_0, rx_ctl_0->cfngpongboxrx20bdataflip);
    reg_rx_ctl_0 = RU_FIELD_SET(0, NGPON_GEARBOX, RX_CTL_0, CFNGPONGBOXRXFRCMUXVAL, reg_rx_ctl_0, rx_ctl_0->cfngpongboxrxfrcmuxval);
    reg_rx_ctl_0 = RU_FIELD_SET(0, NGPON_GEARBOX, RX_CTL_0, CFNGPONGBOXRXFRCMUXSEL, reg_rx_ctl_0, rx_ctl_0->cfngpongboxrxfrcmuxsel);
    reg_rx_ctl_0 = RU_FIELD_SET(0, NGPON_GEARBOX, RX_CTL_0, CFNGPONGBOXRXOUTDATAFLIP, reg_rx_ctl_0, rx_ctl_0->cfngpongboxrxoutdataflip);
    reg_rx_ctl_0 = RU_FIELD_SET(0, NGPON_GEARBOX, RX_CTL_0, CFNGPONGBOXRXFRCHUNT, reg_rx_ctl_0, rx_ctl_0->cfngpongboxrxfrchunt);
    reg_rx_ctl_0 = RU_FIELD_SET(0, NGPON_GEARBOX, RX_CTL_0, CFNGPONGBOXRXMAXGOODK, reg_rx_ctl_0, rx_ctl_0->cfngpongboxrxmaxgoodk);
    reg_rx_ctl_0 = RU_FIELD_SET(0, NGPON_GEARBOX, RX_CTL_0, CFNGPONGBOXRXFRMK28ONLY, reg_rx_ctl_0, rx_ctl_0->cfngpongboxrxfrmk28only);
    reg_rx_ctl_0 = RU_FIELD_SET(0, NGPON_GEARBOX, RX_CTL_0, CFNGPONGBOXRXMAXBADK, reg_rx_ctl_0, rx_ctl_0->cfngpongboxrxmaxbadk);
    reg_rx_ctl_0 = RU_FIELD_SET(0, NGPON_GEARBOX, RX_CTL_0, CFNGPONGBOXRXPTRAUTOLDDIS, reg_rx_ctl_0, rx_ctl_0->cfngpongboxrxptrautolddis);
    reg_rx_ctl_0 = RU_FIELD_SET(0, NGPON_GEARBOX, RX_CTL_0, CFNGPONGBOXRXFIFORDPTR, reg_rx_ctl_0, rx_ctl_0->cfngpongboxrxfifordptr);

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

    rx_ctl_0->cfngpongboxrstn = RU_FIELD_GET(0, NGPON_GEARBOX, RX_CTL_0, CFNGPONGBOXRSTN, reg_rx_ctl_0);
    rx_ctl_0->cfngpongboxrxen = RU_FIELD_GET(0, NGPON_GEARBOX, RX_CTL_0, CFNGPONGBOXRXEN, reg_rx_ctl_0);
    rx_ctl_0->cfngpongboxrxmode = RU_FIELD_GET(0, NGPON_GEARBOX, RX_CTL_0, CFNGPONGBOXRXMODE, reg_rx_ctl_0);
    rx_ctl_0->cfngpongboxrxswsynchold = RU_FIELD_GET(0, NGPON_GEARBOX, RX_CTL_0, CFNGPONGBOXRXSWSYNCHOLD, reg_rx_ctl_0);
    rx_ctl_0->cfngpongboxrxfifoptrld = RU_FIELD_GET(0, NGPON_GEARBOX, RX_CTL_0, CFNGPONGBOXRXFIFOPTRLD, reg_rx_ctl_0);
    rx_ctl_0->cfngpongboxrxserdatainv = RU_FIELD_GET(0, NGPON_GEARBOX, RX_CTL_0, CFNGPONGBOXRXSERDATAINV, reg_rx_ctl_0);
    rx_ctl_0->cfngpongboxrxserdataflip = RU_FIELD_GET(0, NGPON_GEARBOX, RX_CTL_0, CFNGPONGBOXRXSERDATAFLIP, reg_rx_ctl_0);
    rx_ctl_0->cfngpongboxrx20bdataflip = RU_FIELD_GET(0, NGPON_GEARBOX, RX_CTL_0, CFNGPONGBOXRX20BDATAFLIP, reg_rx_ctl_0);
    rx_ctl_0->cfngpongboxrxfrcmuxval = RU_FIELD_GET(0, NGPON_GEARBOX, RX_CTL_0, CFNGPONGBOXRXFRCMUXVAL, reg_rx_ctl_0);
    rx_ctl_0->cfngpongboxrxfrcmuxsel = RU_FIELD_GET(0, NGPON_GEARBOX, RX_CTL_0, CFNGPONGBOXRXFRCMUXSEL, reg_rx_ctl_0);
    rx_ctl_0->cfngpongboxrxoutdataflip = RU_FIELD_GET(0, NGPON_GEARBOX, RX_CTL_0, CFNGPONGBOXRXOUTDATAFLIP, reg_rx_ctl_0);
    rx_ctl_0->cfngpongboxrxfrchunt = RU_FIELD_GET(0, NGPON_GEARBOX, RX_CTL_0, CFNGPONGBOXRXFRCHUNT, reg_rx_ctl_0);
    rx_ctl_0->cfngpongboxrxmaxgoodk = RU_FIELD_GET(0, NGPON_GEARBOX, RX_CTL_0, CFNGPONGBOXRXMAXGOODK, reg_rx_ctl_0);
    rx_ctl_0->cfngpongboxrxfrmk28only = RU_FIELD_GET(0, NGPON_GEARBOX, RX_CTL_0, CFNGPONGBOXRXFRMK28ONLY, reg_rx_ctl_0);
    rx_ctl_0->cfngpongboxrxmaxbadk = RU_FIELD_GET(0, NGPON_GEARBOX, RX_CTL_0, CFNGPONGBOXRXMAXBADK, reg_rx_ctl_0);
    rx_ctl_0->cfngpongboxrxptrautolddis = RU_FIELD_GET(0, NGPON_GEARBOX, RX_CTL_0, CFNGPONGBOXRXPTRAUTOLDDIS, reg_rx_ctl_0);
    rx_ctl_0->cfngpongboxrxfifordptr = RU_FIELD_GET(0, NGPON_GEARBOX, RX_CTL_0, CFNGPONGBOXRXFIFORDPTR, reg_rx_ctl_0);

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
    if((cfngpongboxrxk28d5rdn >= _10BITS_MAX_VAL_) ||
       (cfngpongboxrxk28d5rdp >= _10BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_rx_ctl_2 = RU_FIELD_SET(0, NGPON_GEARBOX, RX_CTL_2, CFNGPONGBOXRXK28D5RDN, reg_rx_ctl_2, cfngpongboxrxk28d5rdn);
    reg_rx_ctl_2 = RU_FIELD_SET(0, NGPON_GEARBOX, RX_CTL_2, CFNGPONGBOXRXK28D5RDP, reg_rx_ctl_2, cfngpongboxrxk28d5rdp);

    RU_REG_WRITE(0, NGPON_GEARBOX, RX_CTL_2, reg_rx_ctl_2);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_ngpon_gearbox_rx_ctl_2_get(uint16_t *cfngpongboxrxk28d5rdp, uint16_t *cfngpongboxrxk28d5rdn)
{
    uint32_t reg_rx_ctl_2=0;

#ifdef VALIDATE_PARMS
    if(!cfngpongboxrxk28d5rdn || !cfngpongboxrxk28d5rdp)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, NGPON_GEARBOX, RX_CTL_2, reg_rx_ctl_2);

    *cfngpongboxrxk28d5rdn = RU_FIELD_GET(0, NGPON_GEARBOX, RX_CTL_2, CFNGPONGBOXRXK28D5RDN, reg_rx_ctl_2);
    *cfngpongboxrxk28d5rdp = RU_FIELD_GET(0, NGPON_GEARBOX, RX_CTL_2, CFNGPONGBOXRXK28D5RDP, reg_rx_ctl_2);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_ngpon_gearbox_rx_ctl_3_set(uint16_t cfngpongboxrxd5d7rdp, uint16_t cfngpongboxrxd5d7rdn)
{
    uint32_t reg_rx_ctl_3=0;

#ifdef VALIDATE_PARMS
    if((cfngpongboxrxd5d7rdn >= _10BITS_MAX_VAL_) ||
       (cfngpongboxrxd5d7rdp >= _10BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_rx_ctl_3 = RU_FIELD_SET(0, NGPON_GEARBOX, RX_CTL_3, CFNGPONGBOXRXD5D7RDN, reg_rx_ctl_3, cfngpongboxrxd5d7rdn);
    reg_rx_ctl_3 = RU_FIELD_SET(0, NGPON_GEARBOX, RX_CTL_3, CFNGPONGBOXRXD5D7RDP, reg_rx_ctl_3, cfngpongboxrxd5d7rdp);

    RU_REG_WRITE(0, NGPON_GEARBOX, RX_CTL_3, reg_rx_ctl_3);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_ngpon_gearbox_rx_ctl_3_get(uint16_t *cfngpongboxrxd5d7rdp, uint16_t *cfngpongboxrxd5d7rdn)
{
    uint32_t reg_rx_ctl_3=0;

#ifdef VALIDATE_PARMS
    if(!cfngpongboxrxd5d7rdn || !cfngpongboxrxd5d7rdp)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, NGPON_GEARBOX, RX_CTL_3, reg_rx_ctl_3);

    *cfngpongboxrxd5d7rdn = RU_FIELD_GET(0, NGPON_GEARBOX, RX_CTL_3, CFNGPONGBOXRXD5D7RDN, reg_rx_ctl_3);
    *cfngpongboxrxd5d7rdp = RU_FIELD_GET(0, NGPON_GEARBOX, RX_CTL_3, CFNGPONGBOXRXD5D7RDP, reg_rx_ctl_3);

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
    if((tx_ctl->cfngpongboxtxen >= _1BITS_MAX_VAL_) ||
       (tx_ctl->cfngpongboxtxfifoptrld >= _1BITS_MAX_VAL_) ||
       (tx_ctl->cfngpongboxtxfifovldptrld >= _1BITS_MAX_VAL_) ||
       (tx_ctl->cfngpongboxtxserdatainv >= _1BITS_MAX_VAL_) ||
       (tx_ctl->cfngpongboxtxservldinv >= _1BITS_MAX_VAL_) ||
       (tx_ctl->cfngpongboxtxserdataflip >= _1BITS_MAX_VAL_) ||
       (tx_ctl->cfngpongboxtxservldflip >= _1BITS_MAX_VAL_) ||
       (tx_ctl->cfngpongboxtxfifovldoff >= _5BITS_MAX_VAL_) ||
       (tx_ctl->cfngpongboxtxfifodatardptr >= _4BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_tx_ctl = RU_FIELD_SET(0, NGPON_GEARBOX, TX_CTL, CFNGPONGBOXTXEN, reg_tx_ctl, tx_ctl->cfngpongboxtxen);
    reg_tx_ctl = RU_FIELD_SET(0, NGPON_GEARBOX, TX_CTL, CFNGPONGBOXTXFIFOPTRLD, reg_tx_ctl, tx_ctl->cfngpongboxtxfifoptrld);
    reg_tx_ctl = RU_FIELD_SET(0, NGPON_GEARBOX, TX_CTL, CFNGPONGBOXTXFIFOVLDPTRLD, reg_tx_ctl, tx_ctl->cfngpongboxtxfifovldptrld);
    reg_tx_ctl = RU_FIELD_SET(0, NGPON_GEARBOX, TX_CTL, CFNGPONGBOXTXSERDATAINV, reg_tx_ctl, tx_ctl->cfngpongboxtxserdatainv);
    reg_tx_ctl = RU_FIELD_SET(0, NGPON_GEARBOX, TX_CTL, CFNGPONGBOXTXSERVLDINV, reg_tx_ctl, tx_ctl->cfngpongboxtxservldinv);
    reg_tx_ctl = RU_FIELD_SET(0, NGPON_GEARBOX, TX_CTL, CFNGPONGBOXTXSERDATAFLIP, reg_tx_ctl, tx_ctl->cfngpongboxtxserdataflip);
    reg_tx_ctl = RU_FIELD_SET(0, NGPON_GEARBOX, TX_CTL, CFNGPONGBOXTXSERVLDFLIP, reg_tx_ctl, tx_ctl->cfngpongboxtxservldflip);
    reg_tx_ctl = RU_FIELD_SET(0, NGPON_GEARBOX, TX_CTL, CFNGPONGBOXTXFIFOVLDOFF, reg_tx_ctl, tx_ctl->cfngpongboxtxfifovldoff);
    reg_tx_ctl = RU_FIELD_SET(0, NGPON_GEARBOX, TX_CTL, CFNGPONGBOXTXFIFODATARDPTR, reg_tx_ctl, tx_ctl->cfngpongboxtxfifodatardptr);

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

    tx_ctl->cfngpongboxtxen = RU_FIELD_GET(0, NGPON_GEARBOX, TX_CTL, CFNGPONGBOXTXEN, reg_tx_ctl);
    tx_ctl->cfngpongboxtxfifoptrld = RU_FIELD_GET(0, NGPON_GEARBOX, TX_CTL, CFNGPONGBOXTXFIFOPTRLD, reg_tx_ctl);
    tx_ctl->cfngpongboxtxfifovldptrld = RU_FIELD_GET(0, NGPON_GEARBOX, TX_CTL, CFNGPONGBOXTXFIFOVLDPTRLD, reg_tx_ctl);
    tx_ctl->cfngpongboxtxserdatainv = RU_FIELD_GET(0, NGPON_GEARBOX, TX_CTL, CFNGPONGBOXTXSERDATAINV, reg_tx_ctl);
    tx_ctl->cfngpongboxtxservldinv = RU_FIELD_GET(0, NGPON_GEARBOX, TX_CTL, CFNGPONGBOXTXSERVLDINV, reg_tx_ctl);
    tx_ctl->cfngpongboxtxserdataflip = RU_FIELD_GET(0, NGPON_GEARBOX, TX_CTL, CFNGPONGBOXTXSERDATAFLIP, reg_tx_ctl);
    tx_ctl->cfngpongboxtxservldflip = RU_FIELD_GET(0, NGPON_GEARBOX, TX_CTL, CFNGPONGBOXTXSERVLDFLIP, reg_tx_ctl);
    tx_ctl->cfngpongboxtxfifovldoff = RU_FIELD_GET(0, NGPON_GEARBOX, TX_CTL, CFNGPONGBOXTXFIFOVLDOFF, reg_tx_ctl);
    tx_ctl->cfngpongboxtxfifodatardptr = RU_FIELD_GET(0, NGPON_GEARBOX, TX_CTL, CFNGPONGBOXTXFIFODATARDPTR, reg_tx_ctl);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_ngpon_gearbox_ngpon_gearbox_status_get(ngpon_gearbox_ngpon_gearbox_status *ngpon_gearbox_status)
{
    uint32_t reg_status=0;

#ifdef VALIDATE_PARMS
    if(!ngpon_gearbox_status)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, NGPON_GEARBOX, STATUS, reg_status);

    ngpon_gearbox_status->ngponrxgboxcodeerrcntstat = RU_FIELD_GET(0, NGPON_GEARBOX, STATUS, NGPONRXGBOXCODEERRCNTSTAT, reg_status);
    ngpon_gearbox_status->ngponrxgboxfifoptrcol = RU_FIELD_GET(0, NGPON_GEARBOX, STATUS, NGPONRXGBOXFIFOPTRCOL, reg_status);
    ngpon_gearbox_status->ngponrxgboxsyncacq = RU_FIELD_GET(0, NGPON_GEARBOX, STATUS, NGPONRXGBOXSYNCACQ, reg_status);
    ngpon_gearbox_status->ngponrxgboxfifoptrdelta = RU_FIELD_GET(0, NGPON_GEARBOX, STATUS, NGPONRXGBOXFIFOPTRDELTA, reg_status);
    ngpon_gearbox_status->ngponrxgboxkcnt = RU_FIELD_GET(0, NGPON_GEARBOX, STATUS, NGPONRXGBOXKCNT, reg_status);
    ngpon_gearbox_status->ngpontxgboxfifodataptrcol = RU_FIELD_GET(0, NGPON_GEARBOX, STATUS, NGPONTXGBOXFIFODATAPTRCOL, reg_status);
    ngpon_gearbox_status->ngponrxgboxstate = RU_FIELD_GET(0, NGPON_GEARBOX, STATUS, NGPONRXGBOXSTATE, reg_status);
    ngpon_gearbox_status->ngpontxgboxfifovldptrcol = RU_FIELD_GET(0, NGPON_GEARBOX, STATUS, NGPONTXGBOXFIFOVLDPTRCOL, reg_status);

    return BDMF_ERR_OK;
}

