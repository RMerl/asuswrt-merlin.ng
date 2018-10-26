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

#include "bcm63158_drivers_xport_ag.h"
#include "bcm63158_xport_reg_ag.h"
int ag_drv_xport_reg_xport_revision_get(uint32_t *xport_rev)
{
    uint32_t reg_xport_revision=0;

#ifdef VALIDATE_PARMS
    if(!xport_rev)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 2);
        return 2;
    }
#endif

    RU_REG_READ(0, XPORT_REG, XPORT_REVISION, reg_xport_revision);

    *xport_rev = RU_FIELD_GET(0, XPORT_REG, XPORT_REVISION, XPORT_REV, reg_xport_revision);

    return 0;
}

int ag_drv_xport_reg_led_0_cntrl_set(const xport_reg_led_0_cntrl *led_0_cntrl)
{
    uint32_t reg_led_0_cntrl=0;

#ifdef VALIDATE_PARMS
    if(!led_0_cntrl)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 1);
        return 1;
    }
    if((led_0_cntrl->lnk_ovrd_en >= _1BITS_MAX_VAL_) ||
       (led_0_cntrl->spd_ovrd_en >= _1BITS_MAX_VAL_) ||
       (led_0_cntrl->lnk_status_ovrd >= _1BITS_MAX_VAL_) ||
       (led_0_cntrl->led_spd_ovrd >= _3BITS_MAX_VAL_) ||
       (led_0_cntrl->act_led_pol_sel >= _1BITS_MAX_VAL_) ||
       (led_0_cntrl->spdlnk_led2_act_pol_sel >= _1BITS_MAX_VAL_) ||
       (led_0_cntrl->spdlnk_led1_act_pol_sel >= _1BITS_MAX_VAL_) ||
       (led_0_cntrl->spdlnk_led0_act_pol_sel >= _1BITS_MAX_VAL_) ||
       (led_0_cntrl->act_led_act_sel >= _1BITS_MAX_VAL_) ||
       (led_0_cntrl->spdlnk_led2_act_sel >= _1BITS_MAX_VAL_) ||
       (led_0_cntrl->spdlnk_led1_act_sel >= _1BITS_MAX_VAL_) ||
       (led_0_cntrl->spdlnk_led0_act_sel >= _1BITS_MAX_VAL_) ||
       (led_0_cntrl->tx_act_en >= _1BITS_MAX_VAL_) ||
       (led_0_cntrl->rx_act_en >= _1BITS_MAX_VAL_))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    reg_led_0_cntrl = RU_FIELD_SET(0, XPORT_REG, LED_0_CNTRL, LNK_OVRD_EN, reg_led_0_cntrl, led_0_cntrl->lnk_ovrd_en);
    reg_led_0_cntrl = RU_FIELD_SET(0, XPORT_REG, LED_0_CNTRL, SPD_OVRD_EN, reg_led_0_cntrl, led_0_cntrl->spd_ovrd_en);
    reg_led_0_cntrl = RU_FIELD_SET(0, XPORT_REG, LED_0_CNTRL, LNK_STATUS_OVRD, reg_led_0_cntrl, led_0_cntrl->lnk_status_ovrd);
    reg_led_0_cntrl = RU_FIELD_SET(0, XPORT_REG, LED_0_CNTRL, LED_SPD_OVRD, reg_led_0_cntrl, led_0_cntrl->led_spd_ovrd);
    reg_led_0_cntrl = RU_FIELD_SET(0, XPORT_REG, LED_0_CNTRL, ACT_LED_POL_SEL, reg_led_0_cntrl, led_0_cntrl->act_led_pol_sel);
    reg_led_0_cntrl = RU_FIELD_SET(0, XPORT_REG, LED_0_CNTRL, SPDLNK_LED2_ACT_POL_SEL, reg_led_0_cntrl, led_0_cntrl->spdlnk_led2_act_pol_sel);
    reg_led_0_cntrl = RU_FIELD_SET(0, XPORT_REG, LED_0_CNTRL, SPDLNK_LED1_ACT_POL_SEL, reg_led_0_cntrl, led_0_cntrl->spdlnk_led1_act_pol_sel);
    reg_led_0_cntrl = RU_FIELD_SET(0, XPORT_REG, LED_0_CNTRL, SPDLNK_LED0_ACT_POL_SEL, reg_led_0_cntrl, led_0_cntrl->spdlnk_led0_act_pol_sel);
    reg_led_0_cntrl = RU_FIELD_SET(0, XPORT_REG, LED_0_CNTRL, ACT_LED_ACT_SEL, reg_led_0_cntrl, led_0_cntrl->act_led_act_sel);
    reg_led_0_cntrl = RU_FIELD_SET(0, XPORT_REG, LED_0_CNTRL, SPDLNK_LED2_ACT_SEL, reg_led_0_cntrl, led_0_cntrl->spdlnk_led2_act_sel);
    reg_led_0_cntrl = RU_FIELD_SET(0, XPORT_REG, LED_0_CNTRL, SPDLNK_LED1_ACT_SEL, reg_led_0_cntrl, led_0_cntrl->spdlnk_led1_act_sel);
    reg_led_0_cntrl = RU_FIELD_SET(0, XPORT_REG, LED_0_CNTRL, SPDLNK_LED0_ACT_SEL, reg_led_0_cntrl, led_0_cntrl->spdlnk_led0_act_sel);
    reg_led_0_cntrl = RU_FIELD_SET(0, XPORT_REG, LED_0_CNTRL, TX_ACT_EN, reg_led_0_cntrl, led_0_cntrl->tx_act_en);
    reg_led_0_cntrl = RU_FIELD_SET(0, XPORT_REG, LED_0_CNTRL, RX_ACT_EN, reg_led_0_cntrl, led_0_cntrl->rx_act_en);

    RU_REG_WRITE(0, XPORT_REG, LED_0_CNTRL, reg_led_0_cntrl);

    return 0;
}

int ag_drv_xport_reg_led_0_cntrl_get(xport_reg_led_0_cntrl *led_0_cntrl)
{
    uint32_t reg_led_0_cntrl=0;

#ifdef VALIDATE_PARMS
    if(!led_0_cntrl)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 2);
        return 2;
    }
#endif

    RU_REG_READ(0, XPORT_REG, LED_0_CNTRL, reg_led_0_cntrl);

    led_0_cntrl->lnk_ovrd_en = RU_FIELD_GET(0, XPORT_REG, LED_0_CNTRL, LNK_OVRD_EN, reg_led_0_cntrl);
    led_0_cntrl->spd_ovrd_en = RU_FIELD_GET(0, XPORT_REG, LED_0_CNTRL, SPD_OVRD_EN, reg_led_0_cntrl);
    led_0_cntrl->lnk_status_ovrd = RU_FIELD_GET(0, XPORT_REG, LED_0_CNTRL, LNK_STATUS_OVRD, reg_led_0_cntrl);
    led_0_cntrl->led_spd_ovrd = RU_FIELD_GET(0, XPORT_REG, LED_0_CNTRL, LED_SPD_OVRD, reg_led_0_cntrl);
    led_0_cntrl->act_led_pol_sel = RU_FIELD_GET(0, XPORT_REG, LED_0_CNTRL, ACT_LED_POL_SEL, reg_led_0_cntrl);
    led_0_cntrl->spdlnk_led2_act_pol_sel = RU_FIELD_GET(0, XPORT_REG, LED_0_CNTRL, SPDLNK_LED2_ACT_POL_SEL, reg_led_0_cntrl);
    led_0_cntrl->spdlnk_led1_act_pol_sel = RU_FIELD_GET(0, XPORT_REG, LED_0_CNTRL, SPDLNK_LED1_ACT_POL_SEL, reg_led_0_cntrl);
    led_0_cntrl->spdlnk_led0_act_pol_sel = RU_FIELD_GET(0, XPORT_REG, LED_0_CNTRL, SPDLNK_LED0_ACT_POL_SEL, reg_led_0_cntrl);
    led_0_cntrl->act_led_act_sel = RU_FIELD_GET(0, XPORT_REG, LED_0_CNTRL, ACT_LED_ACT_SEL, reg_led_0_cntrl);
    led_0_cntrl->spdlnk_led2_act_sel = RU_FIELD_GET(0, XPORT_REG, LED_0_CNTRL, SPDLNK_LED2_ACT_SEL, reg_led_0_cntrl);
    led_0_cntrl->spdlnk_led1_act_sel = RU_FIELD_GET(0, XPORT_REG, LED_0_CNTRL, SPDLNK_LED1_ACT_SEL, reg_led_0_cntrl);
    led_0_cntrl->spdlnk_led0_act_sel = RU_FIELD_GET(0, XPORT_REG, LED_0_CNTRL, SPDLNK_LED0_ACT_SEL, reg_led_0_cntrl);
    led_0_cntrl->tx_act_en = RU_FIELD_GET(0, XPORT_REG, LED_0_CNTRL, TX_ACT_EN, reg_led_0_cntrl);
    led_0_cntrl->rx_act_en = RU_FIELD_GET(0, XPORT_REG, LED_0_CNTRL, RX_ACT_EN, reg_led_0_cntrl);

    return 0;
}

int ag_drv_xport_reg_led_0_link_and_speed_encoding_sel_set(const xport_reg_led_0_link_and_speed_encoding_sel *led_0_link_and_speed_encoding_sel)
{
    uint32_t reg_led_0_link_and_speed_encoding_sel=0;

#ifdef VALIDATE_PARMS
    if(!led_0_link_and_speed_encoding_sel)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 1);
        return 1;
    }
    if((led_0_link_and_speed_encoding_sel->rsvd_sel_spd_encode_2 >= _3BITS_MAX_VAL_) ||
       (led_0_link_and_speed_encoding_sel->rsvd_sel_spd_encode_1 >= _3BITS_MAX_VAL_) ||
       (led_0_link_and_speed_encoding_sel->sel_10g_encode >= _3BITS_MAX_VAL_) ||
       (led_0_link_and_speed_encoding_sel->sel_2500m_encode >= _3BITS_MAX_VAL_) ||
       (led_0_link_and_speed_encoding_sel->sel_1000m_encode >= _3BITS_MAX_VAL_) ||
       (led_0_link_and_speed_encoding_sel->sel_100m_encode >= _3BITS_MAX_VAL_) ||
       (led_0_link_and_speed_encoding_sel->sel_10m_encode >= _3BITS_MAX_VAL_) ||
       (led_0_link_and_speed_encoding_sel->sel_no_link_encode >= _3BITS_MAX_VAL_))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    reg_led_0_link_and_speed_encoding_sel = RU_FIELD_SET(0, XPORT_REG, LED_0_LINK_AND_SPEED_ENCODING_SEL, RSVD_SEL_SPD_ENCODE_2, reg_led_0_link_and_speed_encoding_sel, led_0_link_and_speed_encoding_sel->rsvd_sel_spd_encode_2);
    reg_led_0_link_and_speed_encoding_sel = RU_FIELD_SET(0, XPORT_REG, LED_0_LINK_AND_SPEED_ENCODING_SEL, RSVD_SEL_SPD_ENCODE_1, reg_led_0_link_and_speed_encoding_sel, led_0_link_and_speed_encoding_sel->rsvd_sel_spd_encode_1);
    reg_led_0_link_and_speed_encoding_sel = RU_FIELD_SET(0, XPORT_REG, LED_0_LINK_AND_SPEED_ENCODING_SEL, SEL_10G_ENCODE, reg_led_0_link_and_speed_encoding_sel, led_0_link_and_speed_encoding_sel->sel_10g_encode);
    reg_led_0_link_and_speed_encoding_sel = RU_FIELD_SET(0, XPORT_REG, LED_0_LINK_AND_SPEED_ENCODING_SEL, SEL_2500M_ENCODE, reg_led_0_link_and_speed_encoding_sel, led_0_link_and_speed_encoding_sel->sel_2500m_encode);
    reg_led_0_link_and_speed_encoding_sel = RU_FIELD_SET(0, XPORT_REG, LED_0_LINK_AND_SPEED_ENCODING_SEL, SEL_1000M_ENCODE, reg_led_0_link_and_speed_encoding_sel, led_0_link_and_speed_encoding_sel->sel_1000m_encode);
    reg_led_0_link_and_speed_encoding_sel = RU_FIELD_SET(0, XPORT_REG, LED_0_LINK_AND_SPEED_ENCODING_SEL, SEL_100M_ENCODE, reg_led_0_link_and_speed_encoding_sel, led_0_link_and_speed_encoding_sel->sel_100m_encode);
    reg_led_0_link_and_speed_encoding_sel = RU_FIELD_SET(0, XPORT_REG, LED_0_LINK_AND_SPEED_ENCODING_SEL, SEL_10M_ENCODE, reg_led_0_link_and_speed_encoding_sel, led_0_link_and_speed_encoding_sel->sel_10m_encode);
    reg_led_0_link_and_speed_encoding_sel = RU_FIELD_SET(0, XPORT_REG, LED_0_LINK_AND_SPEED_ENCODING_SEL, SEL_NO_LINK_ENCODE, reg_led_0_link_and_speed_encoding_sel, led_0_link_and_speed_encoding_sel->sel_no_link_encode);

    RU_REG_WRITE(0, XPORT_REG, LED_0_LINK_AND_SPEED_ENCODING_SEL, reg_led_0_link_and_speed_encoding_sel);

    return 0;
}

int ag_drv_xport_reg_led_0_link_and_speed_encoding_sel_get(xport_reg_led_0_link_and_speed_encoding_sel *led_0_link_and_speed_encoding_sel)
{
    uint32_t reg_led_0_link_and_speed_encoding_sel=0;

#ifdef VALIDATE_PARMS
    if(!led_0_link_and_speed_encoding_sel)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 2);
        return 2;
    }
#endif

    RU_REG_READ(0, XPORT_REG, LED_0_LINK_AND_SPEED_ENCODING_SEL, reg_led_0_link_and_speed_encoding_sel);

    led_0_link_and_speed_encoding_sel->rsvd_sel_spd_encode_2 = RU_FIELD_GET(0, XPORT_REG, LED_0_LINK_AND_SPEED_ENCODING_SEL, RSVD_SEL_SPD_ENCODE_2, reg_led_0_link_and_speed_encoding_sel);
    led_0_link_and_speed_encoding_sel->rsvd_sel_spd_encode_1 = RU_FIELD_GET(0, XPORT_REG, LED_0_LINK_AND_SPEED_ENCODING_SEL, RSVD_SEL_SPD_ENCODE_1, reg_led_0_link_and_speed_encoding_sel);
    led_0_link_and_speed_encoding_sel->sel_10g_encode = RU_FIELD_GET(0, XPORT_REG, LED_0_LINK_AND_SPEED_ENCODING_SEL, SEL_10G_ENCODE, reg_led_0_link_and_speed_encoding_sel);
    led_0_link_and_speed_encoding_sel->sel_2500m_encode = RU_FIELD_GET(0, XPORT_REG, LED_0_LINK_AND_SPEED_ENCODING_SEL, SEL_2500M_ENCODE, reg_led_0_link_and_speed_encoding_sel);
    led_0_link_and_speed_encoding_sel->sel_1000m_encode = RU_FIELD_GET(0, XPORT_REG, LED_0_LINK_AND_SPEED_ENCODING_SEL, SEL_1000M_ENCODE, reg_led_0_link_and_speed_encoding_sel);
    led_0_link_and_speed_encoding_sel->sel_100m_encode = RU_FIELD_GET(0, XPORT_REG, LED_0_LINK_AND_SPEED_ENCODING_SEL, SEL_100M_ENCODE, reg_led_0_link_and_speed_encoding_sel);
    led_0_link_and_speed_encoding_sel->sel_10m_encode = RU_FIELD_GET(0, XPORT_REG, LED_0_LINK_AND_SPEED_ENCODING_SEL, SEL_10M_ENCODE, reg_led_0_link_and_speed_encoding_sel);
    led_0_link_and_speed_encoding_sel->sel_no_link_encode = RU_FIELD_GET(0, XPORT_REG, LED_0_LINK_AND_SPEED_ENCODING_SEL, SEL_NO_LINK_ENCODE, reg_led_0_link_and_speed_encoding_sel);

    return 0;
}

int ag_drv_xport_reg_led_0_link_and_speed_encoding_set(const xport_reg_led_0_link_and_speed_encoding *led_0_link_and_speed_encoding)
{
    uint32_t reg_led_0_link_and_speed_encoding=0;

#ifdef VALIDATE_PARMS
    if(!led_0_link_and_speed_encoding)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 1);
        return 1;
    }
    if((led_0_link_and_speed_encoding->rsvd_spd_encode_2 >= _3BITS_MAX_VAL_) ||
       (led_0_link_and_speed_encoding->rsvd_spd_encode_1 >= _3BITS_MAX_VAL_) ||
       (led_0_link_and_speed_encoding->m10g_encode >= _3BITS_MAX_VAL_) ||
       (led_0_link_and_speed_encoding->m2500_encode >= _3BITS_MAX_VAL_) ||
       (led_0_link_and_speed_encoding->m1000_encode >= _3BITS_MAX_VAL_) ||
       (led_0_link_and_speed_encoding->m100_encode >= _3BITS_MAX_VAL_) ||
       (led_0_link_and_speed_encoding->m10_encode >= _3BITS_MAX_VAL_) ||
       (led_0_link_and_speed_encoding->no_link_encode >= _3BITS_MAX_VAL_))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    reg_led_0_link_and_speed_encoding = RU_FIELD_SET(0, XPORT_REG, LED_0_LINK_AND_SPEED_ENCODING, RSVD_SPD_ENCODE_2, reg_led_0_link_and_speed_encoding, led_0_link_and_speed_encoding->rsvd_spd_encode_2);
    reg_led_0_link_and_speed_encoding = RU_FIELD_SET(0, XPORT_REG, LED_0_LINK_AND_SPEED_ENCODING, RSVD_SPD_ENCODE_1, reg_led_0_link_and_speed_encoding, led_0_link_and_speed_encoding->rsvd_spd_encode_1);
    reg_led_0_link_and_speed_encoding = RU_FIELD_SET(0, XPORT_REG, LED_0_LINK_AND_SPEED_ENCODING, M10G_ENCODE, reg_led_0_link_and_speed_encoding, led_0_link_and_speed_encoding->m10g_encode);
    reg_led_0_link_and_speed_encoding = RU_FIELD_SET(0, XPORT_REG, LED_0_LINK_AND_SPEED_ENCODING, M2500_ENCODE, reg_led_0_link_and_speed_encoding, led_0_link_and_speed_encoding->m2500_encode);
    reg_led_0_link_and_speed_encoding = RU_FIELD_SET(0, XPORT_REG, LED_0_LINK_AND_SPEED_ENCODING, M1000_ENCODE, reg_led_0_link_and_speed_encoding, led_0_link_and_speed_encoding->m1000_encode);
    reg_led_0_link_and_speed_encoding = RU_FIELD_SET(0, XPORT_REG, LED_0_LINK_AND_SPEED_ENCODING, M100_ENCODE, reg_led_0_link_and_speed_encoding, led_0_link_and_speed_encoding->m100_encode);
    reg_led_0_link_and_speed_encoding = RU_FIELD_SET(0, XPORT_REG, LED_0_LINK_AND_SPEED_ENCODING, M10_ENCODE, reg_led_0_link_and_speed_encoding, led_0_link_and_speed_encoding->m10_encode);
    reg_led_0_link_and_speed_encoding = RU_FIELD_SET(0, XPORT_REG, LED_0_LINK_AND_SPEED_ENCODING, NO_LINK_ENCODE, reg_led_0_link_and_speed_encoding, led_0_link_and_speed_encoding->no_link_encode);

    RU_REG_WRITE(0, XPORT_REG, LED_0_LINK_AND_SPEED_ENCODING, reg_led_0_link_and_speed_encoding);

    return 0;
}

int ag_drv_xport_reg_led_0_link_and_speed_encoding_get(xport_reg_led_0_link_and_speed_encoding *led_0_link_and_speed_encoding)
{
    uint32_t reg_led_0_link_and_speed_encoding=0;

#ifdef VALIDATE_PARMS
    if(!led_0_link_and_speed_encoding)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 2);
        return 2;
    }
#endif

    RU_REG_READ(0, XPORT_REG, LED_0_LINK_AND_SPEED_ENCODING, reg_led_0_link_and_speed_encoding);

    led_0_link_and_speed_encoding->rsvd_spd_encode_2 = RU_FIELD_GET(0, XPORT_REG, LED_0_LINK_AND_SPEED_ENCODING, RSVD_SPD_ENCODE_2, reg_led_0_link_and_speed_encoding);
    led_0_link_and_speed_encoding->rsvd_spd_encode_1 = RU_FIELD_GET(0, XPORT_REG, LED_0_LINK_AND_SPEED_ENCODING, RSVD_SPD_ENCODE_1, reg_led_0_link_and_speed_encoding);
    led_0_link_and_speed_encoding->m10g_encode = RU_FIELD_GET(0, XPORT_REG, LED_0_LINK_AND_SPEED_ENCODING, M10G_ENCODE, reg_led_0_link_and_speed_encoding);
    led_0_link_and_speed_encoding->m2500_encode = RU_FIELD_GET(0, XPORT_REG, LED_0_LINK_AND_SPEED_ENCODING, M2500_ENCODE, reg_led_0_link_and_speed_encoding);
    led_0_link_and_speed_encoding->m1000_encode = RU_FIELD_GET(0, XPORT_REG, LED_0_LINK_AND_SPEED_ENCODING, M1000_ENCODE, reg_led_0_link_and_speed_encoding);
    led_0_link_and_speed_encoding->m100_encode = RU_FIELD_GET(0, XPORT_REG, LED_0_LINK_AND_SPEED_ENCODING, M100_ENCODE, reg_led_0_link_and_speed_encoding);
    led_0_link_and_speed_encoding->m10_encode = RU_FIELD_GET(0, XPORT_REG, LED_0_LINK_AND_SPEED_ENCODING, M10_ENCODE, reg_led_0_link_and_speed_encoding);
    led_0_link_and_speed_encoding->no_link_encode = RU_FIELD_GET(0, XPORT_REG, LED_0_LINK_AND_SPEED_ENCODING, NO_LINK_ENCODE, reg_led_0_link_and_speed_encoding);

    return 0;
}

int ag_drv_xport_reg_led_1_cntrl_set(const xport_reg_led_1_cntrl *led_1_cntrl)
{
    uint32_t reg_led_1_cntrl=0;

#ifdef VALIDATE_PARMS
    if(!led_1_cntrl)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 1);
        return 1;
    }
    if((led_1_cntrl->lnk_ovrd_en >= _1BITS_MAX_VAL_) ||
       (led_1_cntrl->spd_ovrd_en >= _1BITS_MAX_VAL_) ||
       (led_1_cntrl->lnk_status_ovrd >= _1BITS_MAX_VAL_) ||
       (led_1_cntrl->led_spd_ovrd >= _3BITS_MAX_VAL_) ||
       (led_1_cntrl->act_led_pol_sel >= _1BITS_MAX_VAL_) ||
       (led_1_cntrl->spdlnk_led2_act_pol_sel >= _1BITS_MAX_VAL_) ||
       (led_1_cntrl->spdlnk_led1_act_pol_sel >= _1BITS_MAX_VAL_) ||
       (led_1_cntrl->spdlnk_led0_act_pol_sel >= _1BITS_MAX_VAL_) ||
       (led_1_cntrl->act_led_act_sel >= _1BITS_MAX_VAL_) ||
       (led_1_cntrl->spdlnk_led2_act_sel >= _1BITS_MAX_VAL_) ||
       (led_1_cntrl->spdlnk_led1_act_sel >= _1BITS_MAX_VAL_) ||
       (led_1_cntrl->spdlnk_led0_act_sel >= _1BITS_MAX_VAL_) ||
       (led_1_cntrl->tx_act_en >= _1BITS_MAX_VAL_) ||
       (led_1_cntrl->rx_act_en >= _1BITS_MAX_VAL_))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    reg_led_1_cntrl = RU_FIELD_SET(0, XPORT_REG, LED_1_CNTRL, LNK_OVRD_EN, reg_led_1_cntrl, led_1_cntrl->lnk_ovrd_en);
    reg_led_1_cntrl = RU_FIELD_SET(0, XPORT_REG, LED_1_CNTRL, SPD_OVRD_EN, reg_led_1_cntrl, led_1_cntrl->spd_ovrd_en);
    reg_led_1_cntrl = RU_FIELD_SET(0, XPORT_REG, LED_1_CNTRL, LNK_STATUS_OVRD, reg_led_1_cntrl, led_1_cntrl->lnk_status_ovrd);
    reg_led_1_cntrl = RU_FIELD_SET(0, XPORT_REG, LED_1_CNTRL, LED_SPD_OVRD, reg_led_1_cntrl, led_1_cntrl->led_spd_ovrd);
    reg_led_1_cntrl = RU_FIELD_SET(0, XPORT_REG, LED_1_CNTRL, ACT_LED_POL_SEL, reg_led_1_cntrl, led_1_cntrl->act_led_pol_sel);
    reg_led_1_cntrl = RU_FIELD_SET(0, XPORT_REG, LED_1_CNTRL, SPDLNK_LED2_ACT_POL_SEL, reg_led_1_cntrl, led_1_cntrl->spdlnk_led2_act_pol_sel);
    reg_led_1_cntrl = RU_FIELD_SET(0, XPORT_REG, LED_1_CNTRL, SPDLNK_LED1_ACT_POL_SEL, reg_led_1_cntrl, led_1_cntrl->spdlnk_led1_act_pol_sel);
    reg_led_1_cntrl = RU_FIELD_SET(0, XPORT_REG, LED_1_CNTRL, SPDLNK_LED0_ACT_POL_SEL, reg_led_1_cntrl, led_1_cntrl->spdlnk_led0_act_pol_sel);
    reg_led_1_cntrl = RU_FIELD_SET(0, XPORT_REG, LED_1_CNTRL, ACT_LED_ACT_SEL, reg_led_1_cntrl, led_1_cntrl->act_led_act_sel);
    reg_led_1_cntrl = RU_FIELD_SET(0, XPORT_REG, LED_1_CNTRL, SPDLNK_LED2_ACT_SEL, reg_led_1_cntrl, led_1_cntrl->spdlnk_led2_act_sel);
    reg_led_1_cntrl = RU_FIELD_SET(0, XPORT_REG, LED_1_CNTRL, SPDLNK_LED1_ACT_SEL, reg_led_1_cntrl, led_1_cntrl->spdlnk_led1_act_sel);
    reg_led_1_cntrl = RU_FIELD_SET(0, XPORT_REG, LED_1_CNTRL, SPDLNK_LED0_ACT_SEL, reg_led_1_cntrl, led_1_cntrl->spdlnk_led0_act_sel);
    reg_led_1_cntrl = RU_FIELD_SET(0, XPORT_REG, LED_1_CNTRL, TX_ACT_EN, reg_led_1_cntrl, led_1_cntrl->tx_act_en);
    reg_led_1_cntrl = RU_FIELD_SET(0, XPORT_REG, LED_1_CNTRL, RX_ACT_EN, reg_led_1_cntrl, led_1_cntrl->rx_act_en);

    RU_REG_WRITE(0, XPORT_REG, LED_1_CNTRL, reg_led_1_cntrl);

    return 0;
}

int ag_drv_xport_reg_led_1_cntrl_get(xport_reg_led_1_cntrl *led_1_cntrl)
{
    uint32_t reg_led_1_cntrl=0;

#ifdef VALIDATE_PARMS
    if(!led_1_cntrl)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 2);
        return 2;
    }
#endif

    RU_REG_READ(0, XPORT_REG, LED_1_CNTRL, reg_led_1_cntrl);

    led_1_cntrl->lnk_ovrd_en = RU_FIELD_GET(0, XPORT_REG, LED_1_CNTRL, LNK_OVRD_EN, reg_led_1_cntrl);
    led_1_cntrl->spd_ovrd_en = RU_FIELD_GET(0, XPORT_REG, LED_1_CNTRL, SPD_OVRD_EN, reg_led_1_cntrl);
    led_1_cntrl->lnk_status_ovrd = RU_FIELD_GET(0, XPORT_REG, LED_1_CNTRL, LNK_STATUS_OVRD, reg_led_1_cntrl);
    led_1_cntrl->led_spd_ovrd = RU_FIELD_GET(0, XPORT_REG, LED_1_CNTRL, LED_SPD_OVRD, reg_led_1_cntrl);
    led_1_cntrl->act_led_pol_sel = RU_FIELD_GET(0, XPORT_REG, LED_1_CNTRL, ACT_LED_POL_SEL, reg_led_1_cntrl);
    led_1_cntrl->spdlnk_led2_act_pol_sel = RU_FIELD_GET(0, XPORT_REG, LED_1_CNTRL, SPDLNK_LED2_ACT_POL_SEL, reg_led_1_cntrl);
    led_1_cntrl->spdlnk_led1_act_pol_sel = RU_FIELD_GET(0, XPORT_REG, LED_1_CNTRL, SPDLNK_LED1_ACT_POL_SEL, reg_led_1_cntrl);
    led_1_cntrl->spdlnk_led0_act_pol_sel = RU_FIELD_GET(0, XPORT_REG, LED_1_CNTRL, SPDLNK_LED0_ACT_POL_SEL, reg_led_1_cntrl);
    led_1_cntrl->act_led_act_sel = RU_FIELD_GET(0, XPORT_REG, LED_1_CNTRL, ACT_LED_ACT_SEL, reg_led_1_cntrl);
    led_1_cntrl->spdlnk_led2_act_sel = RU_FIELD_GET(0, XPORT_REG, LED_1_CNTRL, SPDLNK_LED2_ACT_SEL, reg_led_1_cntrl);
    led_1_cntrl->spdlnk_led1_act_sel = RU_FIELD_GET(0, XPORT_REG, LED_1_CNTRL, SPDLNK_LED1_ACT_SEL, reg_led_1_cntrl);
    led_1_cntrl->spdlnk_led0_act_sel = RU_FIELD_GET(0, XPORT_REG, LED_1_CNTRL, SPDLNK_LED0_ACT_SEL, reg_led_1_cntrl);
    led_1_cntrl->tx_act_en = RU_FIELD_GET(0, XPORT_REG, LED_1_CNTRL, TX_ACT_EN, reg_led_1_cntrl);
    led_1_cntrl->rx_act_en = RU_FIELD_GET(0, XPORT_REG, LED_1_CNTRL, RX_ACT_EN, reg_led_1_cntrl);

    return 0;
}

int ag_drv_xport_reg_led_1_link_and_speed_encoding_sel_set(const xport_reg_led_1_link_and_speed_encoding_sel *led_1_link_and_speed_encoding_sel)
{
    uint32_t reg_led_1_link_and_speed_encoding_sel=0;

#ifdef VALIDATE_PARMS
    if(!led_1_link_and_speed_encoding_sel)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 1);
        return 1;
    }
    if((led_1_link_and_speed_encoding_sel->rsvd_sel_spd_encode_2 >= _3BITS_MAX_VAL_) ||
       (led_1_link_and_speed_encoding_sel->rsvd_sel_spd_encode_1 >= _3BITS_MAX_VAL_) ||
       (led_1_link_and_speed_encoding_sel->sel_10g_encode >= _3BITS_MAX_VAL_) ||
       (led_1_link_and_speed_encoding_sel->sel_2500m_encode >= _3BITS_MAX_VAL_) ||
       (led_1_link_and_speed_encoding_sel->sel_1000m_encode >= _3BITS_MAX_VAL_) ||
       (led_1_link_and_speed_encoding_sel->sel_100m_encode >= _3BITS_MAX_VAL_) ||
       (led_1_link_and_speed_encoding_sel->sel_10m_encode >= _3BITS_MAX_VAL_) ||
       (led_1_link_and_speed_encoding_sel->sel_no_link_encode >= _3BITS_MAX_VAL_))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    reg_led_1_link_and_speed_encoding_sel = RU_FIELD_SET(0, XPORT_REG, LED_1_LINK_AND_SPEED_ENCODING_SEL, RSVD_SEL_SPD_ENCODE_2, reg_led_1_link_and_speed_encoding_sel, led_1_link_and_speed_encoding_sel->rsvd_sel_spd_encode_2);
    reg_led_1_link_and_speed_encoding_sel = RU_FIELD_SET(0, XPORT_REG, LED_1_LINK_AND_SPEED_ENCODING_SEL, RSVD_SEL_SPD_ENCODE_1, reg_led_1_link_and_speed_encoding_sel, led_1_link_and_speed_encoding_sel->rsvd_sel_spd_encode_1);
    reg_led_1_link_and_speed_encoding_sel = RU_FIELD_SET(0, XPORT_REG, LED_1_LINK_AND_SPEED_ENCODING_SEL, SEL_10G_ENCODE, reg_led_1_link_and_speed_encoding_sel, led_1_link_and_speed_encoding_sel->sel_10g_encode);
    reg_led_1_link_and_speed_encoding_sel = RU_FIELD_SET(0, XPORT_REG, LED_1_LINK_AND_SPEED_ENCODING_SEL, SEL_2500M_ENCODE, reg_led_1_link_and_speed_encoding_sel, led_1_link_and_speed_encoding_sel->sel_2500m_encode);
    reg_led_1_link_and_speed_encoding_sel = RU_FIELD_SET(0, XPORT_REG, LED_1_LINK_AND_SPEED_ENCODING_SEL, SEL_1000M_ENCODE, reg_led_1_link_and_speed_encoding_sel, led_1_link_and_speed_encoding_sel->sel_1000m_encode);
    reg_led_1_link_and_speed_encoding_sel = RU_FIELD_SET(0, XPORT_REG, LED_1_LINK_AND_SPEED_ENCODING_SEL, SEL_100M_ENCODE, reg_led_1_link_and_speed_encoding_sel, led_1_link_and_speed_encoding_sel->sel_100m_encode);
    reg_led_1_link_and_speed_encoding_sel = RU_FIELD_SET(0, XPORT_REG, LED_1_LINK_AND_SPEED_ENCODING_SEL, SEL_10M_ENCODE, reg_led_1_link_and_speed_encoding_sel, led_1_link_and_speed_encoding_sel->sel_10m_encode);
    reg_led_1_link_and_speed_encoding_sel = RU_FIELD_SET(0, XPORT_REG, LED_1_LINK_AND_SPEED_ENCODING_SEL, SEL_NO_LINK_ENCODE, reg_led_1_link_and_speed_encoding_sel, led_1_link_and_speed_encoding_sel->sel_no_link_encode);

    RU_REG_WRITE(0, XPORT_REG, LED_1_LINK_AND_SPEED_ENCODING_SEL, reg_led_1_link_and_speed_encoding_sel);

    return 0;
}

int ag_drv_xport_reg_led_1_link_and_speed_encoding_sel_get(xport_reg_led_1_link_and_speed_encoding_sel *led_1_link_and_speed_encoding_sel)
{
    uint32_t reg_led_1_link_and_speed_encoding_sel=0;

#ifdef VALIDATE_PARMS
    if(!led_1_link_and_speed_encoding_sel)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 2);
        return 2;
    }
#endif

    RU_REG_READ(0, XPORT_REG, LED_1_LINK_AND_SPEED_ENCODING_SEL, reg_led_1_link_and_speed_encoding_sel);

    led_1_link_and_speed_encoding_sel->rsvd_sel_spd_encode_2 = RU_FIELD_GET(0, XPORT_REG, LED_1_LINK_AND_SPEED_ENCODING_SEL, RSVD_SEL_SPD_ENCODE_2, reg_led_1_link_and_speed_encoding_sel);
    led_1_link_and_speed_encoding_sel->rsvd_sel_spd_encode_1 = RU_FIELD_GET(0, XPORT_REG, LED_1_LINK_AND_SPEED_ENCODING_SEL, RSVD_SEL_SPD_ENCODE_1, reg_led_1_link_and_speed_encoding_sel);
    led_1_link_and_speed_encoding_sel->sel_10g_encode = RU_FIELD_GET(0, XPORT_REG, LED_1_LINK_AND_SPEED_ENCODING_SEL, SEL_10G_ENCODE, reg_led_1_link_and_speed_encoding_sel);
    led_1_link_and_speed_encoding_sel->sel_2500m_encode = RU_FIELD_GET(0, XPORT_REG, LED_1_LINK_AND_SPEED_ENCODING_SEL, SEL_2500M_ENCODE, reg_led_1_link_and_speed_encoding_sel);
    led_1_link_and_speed_encoding_sel->sel_1000m_encode = RU_FIELD_GET(0, XPORT_REG, LED_1_LINK_AND_SPEED_ENCODING_SEL, SEL_1000M_ENCODE, reg_led_1_link_and_speed_encoding_sel);
    led_1_link_and_speed_encoding_sel->sel_100m_encode = RU_FIELD_GET(0, XPORT_REG, LED_1_LINK_AND_SPEED_ENCODING_SEL, SEL_100M_ENCODE, reg_led_1_link_and_speed_encoding_sel);
    led_1_link_and_speed_encoding_sel->sel_10m_encode = RU_FIELD_GET(0, XPORT_REG, LED_1_LINK_AND_SPEED_ENCODING_SEL, SEL_10M_ENCODE, reg_led_1_link_and_speed_encoding_sel);
    led_1_link_and_speed_encoding_sel->sel_no_link_encode = RU_FIELD_GET(0, XPORT_REG, LED_1_LINK_AND_SPEED_ENCODING_SEL, SEL_NO_LINK_ENCODE, reg_led_1_link_and_speed_encoding_sel);

    return 0;
}

int ag_drv_xport_reg_led_1_link_and_speed_encoding_set(const xport_reg_led_1_link_and_speed_encoding *led_1_link_and_speed_encoding)
{
    uint32_t reg_led_1_link_and_speed_encoding=0;

#ifdef VALIDATE_PARMS
    if(!led_1_link_and_speed_encoding)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 1);
        return 1;
    }
    if((led_1_link_and_speed_encoding->rsvd_spd_encode_2 >= _3BITS_MAX_VAL_) ||
       (led_1_link_and_speed_encoding->rsvd_spd_encode_1 >= _3BITS_MAX_VAL_) ||
       (led_1_link_and_speed_encoding->m10g_encode >= _3BITS_MAX_VAL_) ||
       (led_1_link_and_speed_encoding->m2500_encode >= _3BITS_MAX_VAL_) ||
       (led_1_link_and_speed_encoding->m1000_encode >= _3BITS_MAX_VAL_) ||
       (led_1_link_and_speed_encoding->m100_encode >= _3BITS_MAX_VAL_) ||
       (led_1_link_and_speed_encoding->m10_encode >= _3BITS_MAX_VAL_) ||
       (led_1_link_and_speed_encoding->no_link_encode >= _3BITS_MAX_VAL_))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    reg_led_1_link_and_speed_encoding = RU_FIELD_SET(0, XPORT_REG, LED_1_LINK_AND_SPEED_ENCODING, RSVD_SPD_ENCODE_2, reg_led_1_link_and_speed_encoding, led_1_link_and_speed_encoding->rsvd_spd_encode_2);
    reg_led_1_link_and_speed_encoding = RU_FIELD_SET(0, XPORT_REG, LED_1_LINK_AND_SPEED_ENCODING, RSVD_SPD_ENCODE_1, reg_led_1_link_and_speed_encoding, led_1_link_and_speed_encoding->rsvd_spd_encode_1);
    reg_led_1_link_and_speed_encoding = RU_FIELD_SET(0, XPORT_REG, LED_1_LINK_AND_SPEED_ENCODING, M10G_ENCODE, reg_led_1_link_and_speed_encoding, led_1_link_and_speed_encoding->m10g_encode);
    reg_led_1_link_and_speed_encoding = RU_FIELD_SET(0, XPORT_REG, LED_1_LINK_AND_SPEED_ENCODING, M2500_ENCODE, reg_led_1_link_and_speed_encoding, led_1_link_and_speed_encoding->m2500_encode);
    reg_led_1_link_and_speed_encoding = RU_FIELD_SET(0, XPORT_REG, LED_1_LINK_AND_SPEED_ENCODING, M1000_ENCODE, reg_led_1_link_and_speed_encoding, led_1_link_and_speed_encoding->m1000_encode);
    reg_led_1_link_and_speed_encoding = RU_FIELD_SET(0, XPORT_REG, LED_1_LINK_AND_SPEED_ENCODING, M100_ENCODE, reg_led_1_link_and_speed_encoding, led_1_link_and_speed_encoding->m100_encode);
    reg_led_1_link_and_speed_encoding = RU_FIELD_SET(0, XPORT_REG, LED_1_LINK_AND_SPEED_ENCODING, M10_ENCODE, reg_led_1_link_and_speed_encoding, led_1_link_and_speed_encoding->m10_encode);
    reg_led_1_link_and_speed_encoding = RU_FIELD_SET(0, XPORT_REG, LED_1_LINK_AND_SPEED_ENCODING, NO_LINK_ENCODE, reg_led_1_link_and_speed_encoding, led_1_link_and_speed_encoding->no_link_encode);

    RU_REG_WRITE(0, XPORT_REG, LED_1_LINK_AND_SPEED_ENCODING, reg_led_1_link_and_speed_encoding);

    return 0;
}

int ag_drv_xport_reg_led_1_link_and_speed_encoding_get(xport_reg_led_1_link_and_speed_encoding *led_1_link_and_speed_encoding)
{
    uint32_t reg_led_1_link_and_speed_encoding=0;

#ifdef VALIDATE_PARMS
    if(!led_1_link_and_speed_encoding)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 2);
        return 2;
    }
#endif

    RU_REG_READ(0, XPORT_REG, LED_1_LINK_AND_SPEED_ENCODING, reg_led_1_link_and_speed_encoding);

    led_1_link_and_speed_encoding->rsvd_spd_encode_2 = RU_FIELD_GET(0, XPORT_REG, LED_1_LINK_AND_SPEED_ENCODING, RSVD_SPD_ENCODE_2, reg_led_1_link_and_speed_encoding);
    led_1_link_and_speed_encoding->rsvd_spd_encode_1 = RU_FIELD_GET(0, XPORT_REG, LED_1_LINK_AND_SPEED_ENCODING, RSVD_SPD_ENCODE_1, reg_led_1_link_and_speed_encoding);
    led_1_link_and_speed_encoding->m10g_encode = RU_FIELD_GET(0, XPORT_REG, LED_1_LINK_AND_SPEED_ENCODING, M10G_ENCODE, reg_led_1_link_and_speed_encoding);
    led_1_link_and_speed_encoding->m2500_encode = RU_FIELD_GET(0, XPORT_REG, LED_1_LINK_AND_SPEED_ENCODING, M2500_ENCODE, reg_led_1_link_and_speed_encoding);
    led_1_link_and_speed_encoding->m1000_encode = RU_FIELD_GET(0, XPORT_REG, LED_1_LINK_AND_SPEED_ENCODING, M1000_ENCODE, reg_led_1_link_and_speed_encoding);
    led_1_link_and_speed_encoding->m100_encode = RU_FIELD_GET(0, XPORT_REG, LED_1_LINK_AND_SPEED_ENCODING, M100_ENCODE, reg_led_1_link_and_speed_encoding);
    led_1_link_and_speed_encoding->m10_encode = RU_FIELD_GET(0, XPORT_REG, LED_1_LINK_AND_SPEED_ENCODING, M10_ENCODE, reg_led_1_link_and_speed_encoding);
    led_1_link_and_speed_encoding->no_link_encode = RU_FIELD_GET(0, XPORT_REG, LED_1_LINK_AND_SPEED_ENCODING, NO_LINK_ENCODE, reg_led_1_link_and_speed_encoding);

    return 0;
}

int ag_drv_xport_reg_led_blink_rate_cntrl_set(uint16_t led_on_time, uint16_t led_off_time)
{
    uint32_t reg_led_blink_rate_cntrl=0;

#ifdef VALIDATE_PARMS
#endif

    reg_led_blink_rate_cntrl = RU_FIELD_SET(0, XPORT_REG, LED_BLINK_RATE_CNTRL, LED_ON_TIME, reg_led_blink_rate_cntrl, led_on_time);
    reg_led_blink_rate_cntrl = RU_FIELD_SET(0, XPORT_REG, LED_BLINK_RATE_CNTRL, LED_OFF_TIME, reg_led_blink_rate_cntrl, led_off_time);

    RU_REG_WRITE(0, XPORT_REG, LED_BLINK_RATE_CNTRL, reg_led_blink_rate_cntrl);

    return 0;
}

int ag_drv_xport_reg_led_blink_rate_cntrl_get(uint16_t *led_on_time, uint16_t *led_off_time)
{
    uint32_t reg_led_blink_rate_cntrl=0;

#ifdef VALIDATE_PARMS
    if(!led_on_time || !led_off_time)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 2);
        return 2;
    }
#endif

    RU_REG_READ(0, XPORT_REG, LED_BLINK_RATE_CNTRL, reg_led_blink_rate_cntrl);

    *led_on_time = RU_FIELD_GET(0, XPORT_REG, LED_BLINK_RATE_CNTRL, LED_ON_TIME, reg_led_blink_rate_cntrl);
    *led_off_time = RU_FIELD_GET(0, XPORT_REG, LED_BLINK_RATE_CNTRL, LED_OFF_TIME, reg_led_blink_rate_cntrl);

    return 0;
}

int ag_drv_xport_reg_led_serial_ctrl_set(const xport_reg_led_serial_ctrl *led_serial_ctrl)
{
    uint32_t reg_led_serial_cntrl=0;

#ifdef VALIDATE_PARMS
    if(!led_serial_ctrl)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 1);
        return 1;
    }
    if((led_serial_ctrl->smode >= _2BITS_MAX_VAL_) ||
       (led_serial_ctrl->sled_clk_frequency >= _1BITS_MAX_VAL_) ||
       (led_serial_ctrl->sled_clk_pol >= _1BITS_MAX_VAL_) ||
       (led_serial_ctrl->refresh_period >= _5BITS_MAX_VAL_))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    reg_led_serial_cntrl = RU_FIELD_SET(0, XPORT_REG, LED_SERIAL_CNTRL, SMODE, reg_led_serial_cntrl, led_serial_ctrl->smode);
    reg_led_serial_cntrl = RU_FIELD_SET(0, XPORT_REG, LED_SERIAL_CNTRL, SLED_CLK_FREQUENCY, reg_led_serial_cntrl, led_serial_ctrl->sled_clk_frequency);
    reg_led_serial_cntrl = RU_FIELD_SET(0, XPORT_REG, LED_SERIAL_CNTRL, SLED_CLK_POL, reg_led_serial_cntrl, led_serial_ctrl->sled_clk_pol);
    reg_led_serial_cntrl = RU_FIELD_SET(0, XPORT_REG, LED_SERIAL_CNTRL, REFRESH_PERIOD, reg_led_serial_cntrl, led_serial_ctrl->refresh_period);
    reg_led_serial_cntrl = RU_FIELD_SET(0, XPORT_REG, LED_SERIAL_CNTRL, PORT_EN, reg_led_serial_cntrl, led_serial_ctrl->port_en);

    RU_REG_WRITE(0, XPORT_REG, LED_SERIAL_CNTRL, reg_led_serial_cntrl);

    return 0;
}

int ag_drv_xport_reg_led_serial_ctrl_get(xport_reg_led_serial_ctrl *led_serial_ctrl)
{
    uint32_t reg_led_serial_cntrl=0;

#ifdef VALIDATE_PARMS
    if(!led_serial_ctrl)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 2);
        return 2;
    }
#endif

    RU_REG_READ(0, XPORT_REG, LED_SERIAL_CNTRL, reg_led_serial_cntrl);

    led_serial_ctrl->smode = RU_FIELD_GET(0, XPORT_REG, LED_SERIAL_CNTRL, SMODE, reg_led_serial_cntrl);
    led_serial_ctrl->sled_clk_frequency = RU_FIELD_GET(0, XPORT_REG, LED_SERIAL_CNTRL, SLED_CLK_FREQUENCY, reg_led_serial_cntrl);
    led_serial_ctrl->sled_clk_pol = RU_FIELD_GET(0, XPORT_REG, LED_SERIAL_CNTRL, SLED_CLK_POL, reg_led_serial_cntrl);
    led_serial_ctrl->refresh_period = RU_FIELD_GET(0, XPORT_REG, LED_SERIAL_CNTRL, REFRESH_PERIOD, reg_led_serial_cntrl);
    led_serial_ctrl->port_en = RU_FIELD_GET(0, XPORT_REG, LED_SERIAL_CNTRL, PORT_EN, reg_led_serial_cntrl);

    return 0;
}

int ag_drv_xport_reg_refresh_period_cntrl_set(uint32_t refresh_period_cnt)
{
    uint32_t reg_refresh_period_cntrl=0;

#ifdef VALIDATE_PARMS
    if((refresh_period_cnt >= _24BITS_MAX_VAL_))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    reg_refresh_period_cntrl = RU_FIELD_SET(0, XPORT_REG, REFRESH_PERIOD_CNTRL, REFRESH_PERIOD_CNT, reg_refresh_period_cntrl, refresh_period_cnt);

    RU_REG_WRITE(0, XPORT_REG, REFRESH_PERIOD_CNTRL, reg_refresh_period_cntrl);

    return 0;
}

int ag_drv_xport_reg_refresh_period_cntrl_get(uint32_t *refresh_period_cnt)
{
    uint32_t reg_refresh_period_cntrl=0;

#ifdef VALIDATE_PARMS
    if(!refresh_period_cnt)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 2);
        return 2;
    }
#endif

    RU_REG_READ(0, XPORT_REG, REFRESH_PERIOD_CNTRL, reg_refresh_period_cntrl);

    *refresh_period_cnt = RU_FIELD_GET(0, XPORT_REG, REFRESH_PERIOD_CNTRL, REFRESH_PERIOD_CNT, reg_refresh_period_cntrl);

    return 0;
}

int ag_drv_xport_reg_aggregate_led_cntrl_set(uint8_t lnk_pol_sel, uint8_t act_pol_sel, uint8_t act_sel, uint16_t port_en)
{
    uint32_t reg_aggregate_led_cntrl=0;

#ifdef VALIDATE_PARMS
    if((lnk_pol_sel >= _1BITS_MAX_VAL_) ||
       (act_pol_sel >= _1BITS_MAX_VAL_) ||
       (act_sel >= _1BITS_MAX_VAL_))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    reg_aggregate_led_cntrl = RU_FIELD_SET(0, XPORT_REG, AGGREGATE_LED_CNTRL, LNK_POL_SEL, reg_aggregate_led_cntrl, lnk_pol_sel);
    reg_aggregate_led_cntrl = RU_FIELD_SET(0, XPORT_REG, AGGREGATE_LED_CNTRL, ACT_POL_SEL, reg_aggregate_led_cntrl, act_pol_sel);
    reg_aggregate_led_cntrl = RU_FIELD_SET(0, XPORT_REG, AGGREGATE_LED_CNTRL, ACT_SEL, reg_aggregate_led_cntrl, act_sel);
    reg_aggregate_led_cntrl = RU_FIELD_SET(0, XPORT_REG, AGGREGATE_LED_CNTRL, PORT_EN, reg_aggregate_led_cntrl, port_en);

    RU_REG_WRITE(0, XPORT_REG, AGGREGATE_LED_CNTRL, reg_aggregate_led_cntrl);

    return 0;
}

int ag_drv_xport_reg_aggregate_led_cntrl_get(uint8_t *lnk_pol_sel, uint8_t *act_pol_sel, uint8_t *act_sel, uint16_t *port_en)
{
    uint32_t reg_aggregate_led_cntrl=0;

#ifdef VALIDATE_PARMS
    if(!lnk_pol_sel || !act_pol_sel || !act_sel || !port_en)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 2);
        return 2;
    }
#endif

    RU_REG_READ(0, XPORT_REG, AGGREGATE_LED_CNTRL, reg_aggregate_led_cntrl);

    *lnk_pol_sel = RU_FIELD_GET(0, XPORT_REG, AGGREGATE_LED_CNTRL, LNK_POL_SEL, reg_aggregate_led_cntrl);
    *act_pol_sel = RU_FIELD_GET(0, XPORT_REG, AGGREGATE_LED_CNTRL, ACT_POL_SEL, reg_aggregate_led_cntrl);
    *act_sel = RU_FIELD_GET(0, XPORT_REG, AGGREGATE_LED_CNTRL, ACT_SEL, reg_aggregate_led_cntrl);
    *port_en = RU_FIELD_GET(0, XPORT_REG, AGGREGATE_LED_CNTRL, PORT_EN, reg_aggregate_led_cntrl);

    return 0;
}

int ag_drv_xport_reg_aggregate_led_blink_rate_cntrl_set(uint16_t led_on_time, uint16_t led_off_time)
{
    uint32_t reg_aggregate_led_blink_rate_cntrl=0;

#ifdef VALIDATE_PARMS
#endif

    reg_aggregate_led_blink_rate_cntrl = RU_FIELD_SET(0, XPORT_REG, AGGREGATE_LED_BLINK_RATE_CNTRL, LED_ON_TIME, reg_aggregate_led_blink_rate_cntrl, led_on_time);
    reg_aggregate_led_blink_rate_cntrl = RU_FIELD_SET(0, XPORT_REG, AGGREGATE_LED_BLINK_RATE_CNTRL, LED_OFF_TIME, reg_aggregate_led_blink_rate_cntrl, led_off_time);

    RU_REG_WRITE(0, XPORT_REG, AGGREGATE_LED_BLINK_RATE_CNTRL, reg_aggregate_led_blink_rate_cntrl);

    return 0;
}

int ag_drv_xport_reg_aggregate_led_blink_rate_cntrl_get(uint16_t *led_on_time, uint16_t *led_off_time)
{
    uint32_t reg_aggregate_led_blink_rate_cntrl=0;

#ifdef VALIDATE_PARMS
    if(!led_on_time || !led_off_time)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 2);
        return 2;
    }
#endif

    RU_REG_READ(0, XPORT_REG, AGGREGATE_LED_BLINK_RATE_CNTRL, reg_aggregate_led_blink_rate_cntrl);

    *led_on_time = RU_FIELD_GET(0, XPORT_REG, AGGREGATE_LED_BLINK_RATE_CNTRL, LED_ON_TIME, reg_aggregate_led_blink_rate_cntrl);
    *led_off_time = RU_FIELD_GET(0, XPORT_REG, AGGREGATE_LED_BLINK_RATE_CNTRL, LED_OFF_TIME, reg_aggregate_led_blink_rate_cntrl);

    return 0;
}

int ag_drv_xport_reg_spare_cntrl_set(uint32_t spare_reg)
{
    uint32_t reg_spare_cntrl=0;

#ifdef VALIDATE_PARMS
#endif

    reg_spare_cntrl = RU_FIELD_SET(0, XPORT_REG, SPARE_CNTRL, SPARE_REG, reg_spare_cntrl, spare_reg);

    RU_REG_WRITE(0, XPORT_REG, SPARE_CNTRL, reg_spare_cntrl);

    return 0;
}

int ag_drv_xport_reg_spare_cntrl_get(uint32_t *spare_reg)
{
    uint32_t reg_spare_cntrl=0;

#ifdef VALIDATE_PARMS
    if(!spare_reg)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 2);
        return 2;
    }
#endif

    RU_REG_READ(0, XPORT_REG, SPARE_CNTRL, reg_spare_cntrl);

    *spare_reg = RU_FIELD_GET(0, XPORT_REG, SPARE_CNTRL, SPARE_REG, reg_spare_cntrl);

    return 0;
}

int ag_drv_xport_reg_xport_cntrl_1_set(uint8_t msbus_clk_sel, uint8_t wan_led0_sel, uint8_t timeout_rst_disable, uint8_t p0_mode)
{
    uint32_t reg_xport_cntrl_1=0;

#ifdef VALIDATE_PARMS
    if((msbus_clk_sel >= _1BITS_MAX_VAL_) ||
       (wan_led0_sel >= _1BITS_MAX_VAL_) ||
       (timeout_rst_disable >= _1BITS_MAX_VAL_) ||
       (p0_mode >= _1BITS_MAX_VAL_))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    reg_xport_cntrl_1 = RU_FIELD_SET(0, XPORT_REG, XPORT_CNTRL_1, MSBUS_CLK_SEL, reg_xport_cntrl_1, msbus_clk_sel);
    reg_xport_cntrl_1 = RU_FIELD_SET(0, XPORT_REG, XPORT_CNTRL_1, WAN_LED0_SEL, reg_xport_cntrl_1, wan_led0_sel);
    reg_xport_cntrl_1 = RU_FIELD_SET(0, XPORT_REG, XPORT_CNTRL_1, TIMEOUT_RST_DISABLE, reg_xport_cntrl_1, timeout_rst_disable);
    reg_xport_cntrl_1 = RU_FIELD_SET(0, XPORT_REG, XPORT_CNTRL_1, P0_MODE, reg_xport_cntrl_1, p0_mode);

    RU_REG_WRITE(0, XPORT_REG, XPORT_CNTRL_1, reg_xport_cntrl_1);

    return 0;
}

int ag_drv_xport_reg_xport_cntrl_1_get(uint8_t *msbus_clk_sel, uint8_t *wan_led0_sel, uint8_t *timeout_rst_disable, uint8_t *p0_mode)
{
    uint32_t reg_xport_cntrl_1=0;

#ifdef VALIDATE_PARMS
    if(!msbus_clk_sel || !wan_led0_sel || !timeout_rst_disable || !p0_mode)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 2);
        return 2;
    }
#endif

    RU_REG_READ(0, XPORT_REG, XPORT_CNTRL_1, reg_xport_cntrl_1);

    *msbus_clk_sel = RU_FIELD_GET(0, XPORT_REG, XPORT_CNTRL_1, MSBUS_CLK_SEL, reg_xport_cntrl_1);
    *wan_led0_sel = RU_FIELD_GET(0, XPORT_REG, XPORT_CNTRL_1, WAN_LED0_SEL, reg_xport_cntrl_1);
    *timeout_rst_disable = RU_FIELD_GET(0, XPORT_REG, XPORT_CNTRL_1, TIMEOUT_RST_DISABLE, reg_xport_cntrl_1);
    *p0_mode = RU_FIELD_GET(0, XPORT_REG, XPORT_CNTRL_1, P0_MODE, reg_xport_cntrl_1);

    return 0;
}

int ag_drv_xport_reg_crossbar_status_get(xport_reg_crossbar_status *crossbar_status)
{
    uint32_t reg_crossbar_status=0;

#ifdef VALIDATE_PARMS
    if(!crossbar_status)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 2);
        return 2;
    }
#endif

    RU_REG_READ(0, XPORT_REG, CROSSBAR_STATUS, reg_crossbar_status);

    crossbar_status->full_duplex = RU_FIELD_GET(0, XPORT_REG, CROSSBAR_STATUS, FULL_DUPLEX, reg_crossbar_status);
    crossbar_status->pause_tx = RU_FIELD_GET(0, XPORT_REG, CROSSBAR_STATUS, PAUSE_TX, reg_crossbar_status);
    crossbar_status->pause_rx = RU_FIELD_GET(0, XPORT_REG, CROSSBAR_STATUS, PAUSE_RX, reg_crossbar_status);
    crossbar_status->speed_2500 = RU_FIELD_GET(0, XPORT_REG, CROSSBAR_STATUS, SPEED_2500, reg_crossbar_status);
    crossbar_status->speed_1000 = RU_FIELD_GET(0, XPORT_REG, CROSSBAR_STATUS, SPEED_1000, reg_crossbar_status);
    crossbar_status->speed_100 = RU_FIELD_GET(0, XPORT_REG, CROSSBAR_STATUS, SPEED_100, reg_crossbar_status);
    crossbar_status->speed_10 = RU_FIELD_GET(0, XPORT_REG, CROSSBAR_STATUS, SPEED_10, reg_crossbar_status);
    crossbar_status->link_status = RU_FIELD_GET(0, XPORT_REG, CROSSBAR_STATUS, LINK_STATUS, reg_crossbar_status);

    return 0;
}

int ag_drv_xport_reg_pon_ae_serdes_status_get(xport_reg_pon_ae_serdes_status *pon_ae_serdes_status)
{
    uint32_t reg_pon_ae_serdes_status=0;

#ifdef VALIDATE_PARMS
    if(!pon_ae_serdes_status)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 2);
        return 2;
    }
#endif

    RU_REG_READ(0, XPORT_REG, PON_AE_SERDES_STATUS, reg_pon_ae_serdes_status);

    pon_ae_serdes_status->mod_def0 = RU_FIELD_GET(0, XPORT_REG, PON_AE_SERDES_STATUS, MOD_DEF0, reg_pon_ae_serdes_status);
    pon_ae_serdes_status->ext_sig_det = RU_FIELD_GET(0, XPORT_REG, PON_AE_SERDES_STATUS, EXT_SIG_DET, reg_pon_ae_serdes_status);
    pon_ae_serdes_status->pll1_lock = RU_FIELD_GET(0, XPORT_REG, PON_AE_SERDES_STATUS, PLL1_LOCK, reg_pon_ae_serdes_status);
    pon_ae_serdes_status->pll0_lock = RU_FIELD_GET(0, XPORT_REG, PON_AE_SERDES_STATUS, PLL0_LOCK, reg_pon_ae_serdes_status);
    pon_ae_serdes_status->link_status = RU_FIELD_GET(0, XPORT_REG, PON_AE_SERDES_STATUS, LINK_STATUS, reg_pon_ae_serdes_status);
    pon_ae_serdes_status->cdr_lock = RU_FIELD_GET(0, XPORT_REG, PON_AE_SERDES_STATUS, CDR_LOCK, reg_pon_ae_serdes_status);
    pon_ae_serdes_status->rx_sigdet = RU_FIELD_GET(0, XPORT_REG, PON_AE_SERDES_STATUS, RX_SIGDET, reg_pon_ae_serdes_status);

    return 0;
}

