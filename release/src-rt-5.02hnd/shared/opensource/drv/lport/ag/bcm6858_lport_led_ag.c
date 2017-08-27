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

#include "bcm6858_drivers_lport_ag.h"
#include "bcm6858_lport_led_ag.h"
#define BLOCK_ADDR_COUNT_BITS 3
#define BLOCK_ADDR_COUNT (1<<BLOCK_ADDR_COUNT_BITS)

int ag_drv_lport_led_cntrl_set(uint8_t led_id, const lport_led_cntrl *cntrl)
{
    uint32_t reg_cntrl=0;

#ifdef VALIDATE_PARMS
    if(!cntrl)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 1);
        return 1;
    }
    if((led_id >= BLOCK_ADDR_COUNT) ||
       (cntrl->lnk_ovrd_en >= _1BITS_MAX_VAL_) ||
       (cntrl->spd_ovrd_en >= _1BITS_MAX_VAL_) ||
       (cntrl->lnk_status_ovrd >= _1BITS_MAX_VAL_) ||
       (cntrl->led_spd_ovrd >= _3BITS_MAX_VAL_) ||
       (cntrl->act_led_pol_sel >= _1BITS_MAX_VAL_) ||
       (cntrl->spdlnk_led2_act_pol_sel >= _1BITS_MAX_VAL_) ||
       (cntrl->spdlnk_led1_act_pol_sel >= _1BITS_MAX_VAL_) ||
       (cntrl->spdlnk_led0_act_pol_sel >= _1BITS_MAX_VAL_) ||
       (cntrl->act_led_act_sel >= _1BITS_MAX_VAL_) ||
       (cntrl->spdlnk_led2_act_sel >= _1BITS_MAX_VAL_) ||
       (cntrl->spdlnk_led1_act_sel >= _1BITS_MAX_VAL_) ||
       (cntrl->spdlnk_led0_act_sel >= _1BITS_MAX_VAL_) ||
       (cntrl->tx_act_en >= _1BITS_MAX_VAL_) ||
       (cntrl->rx_act_en >= _1BITS_MAX_VAL_))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    reg_cntrl = RU_FIELD_SET(led_id, LPORT_LED, CNTRL, LNK_OVRD_EN, reg_cntrl, cntrl->lnk_ovrd_en);
    reg_cntrl = RU_FIELD_SET(led_id, LPORT_LED, CNTRL, SPD_OVRD_EN, reg_cntrl, cntrl->spd_ovrd_en);
    reg_cntrl = RU_FIELD_SET(led_id, LPORT_LED, CNTRL, LNK_STATUS_OVRD, reg_cntrl, cntrl->lnk_status_ovrd);
    reg_cntrl = RU_FIELD_SET(led_id, LPORT_LED, CNTRL, LED_SPD_OVRD, reg_cntrl, cntrl->led_spd_ovrd);
    reg_cntrl = RU_FIELD_SET(led_id, LPORT_LED, CNTRL, ACT_LED_POL_SEL, reg_cntrl, cntrl->act_led_pol_sel);
    reg_cntrl = RU_FIELD_SET(led_id, LPORT_LED, CNTRL, SPDLNK_LED2_ACT_POL_SEL, reg_cntrl, cntrl->spdlnk_led2_act_pol_sel);
    reg_cntrl = RU_FIELD_SET(led_id, LPORT_LED, CNTRL, SPDLNK_LED1_ACT_POL_SEL, reg_cntrl, cntrl->spdlnk_led1_act_pol_sel);
    reg_cntrl = RU_FIELD_SET(led_id, LPORT_LED, CNTRL, SPDLNK_LED0_ACT_POL_SEL, reg_cntrl, cntrl->spdlnk_led0_act_pol_sel);
    reg_cntrl = RU_FIELD_SET(led_id, LPORT_LED, CNTRL, ACT_LED_ACT_SEL, reg_cntrl, cntrl->act_led_act_sel);
    reg_cntrl = RU_FIELD_SET(led_id, LPORT_LED, CNTRL, SPDLNK_LED2_ACT_SEL, reg_cntrl, cntrl->spdlnk_led2_act_sel);
    reg_cntrl = RU_FIELD_SET(led_id, LPORT_LED, CNTRL, SPDLNK_LED1_ACT_SEL, reg_cntrl, cntrl->spdlnk_led1_act_sel);
    reg_cntrl = RU_FIELD_SET(led_id, LPORT_LED, CNTRL, SPDLNK_LED0_ACT_SEL, reg_cntrl, cntrl->spdlnk_led0_act_sel);
    reg_cntrl = RU_FIELD_SET(led_id, LPORT_LED, CNTRL, TX_ACT_EN, reg_cntrl, cntrl->tx_act_en);
    reg_cntrl = RU_FIELD_SET(led_id, LPORT_LED, CNTRL, RX_ACT_EN, reg_cntrl, cntrl->rx_act_en);

    RU_REG_WRITE(led_id, LPORT_LED, CNTRL, reg_cntrl);

    return 0;
}

int ag_drv_lport_led_cntrl_get(uint8_t led_id, lport_led_cntrl *cntrl)
{
    uint32_t reg_cntrl=0;

#ifdef VALIDATE_PARMS
    if(!cntrl)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 2);
        return 2;
    }
    if((led_id >= BLOCK_ADDR_COUNT))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    RU_REG_READ(led_id, LPORT_LED, CNTRL, reg_cntrl);

    cntrl->lnk_ovrd_en = RU_FIELD_GET(led_id, LPORT_LED, CNTRL, LNK_OVRD_EN, reg_cntrl);
    cntrl->spd_ovrd_en = RU_FIELD_GET(led_id, LPORT_LED, CNTRL, SPD_OVRD_EN, reg_cntrl);
    cntrl->lnk_status_ovrd = RU_FIELD_GET(led_id, LPORT_LED, CNTRL, LNK_STATUS_OVRD, reg_cntrl);
    cntrl->led_spd_ovrd = RU_FIELD_GET(led_id, LPORT_LED, CNTRL, LED_SPD_OVRD, reg_cntrl);
    cntrl->act_led_pol_sel = RU_FIELD_GET(led_id, LPORT_LED, CNTRL, ACT_LED_POL_SEL, reg_cntrl);
    cntrl->spdlnk_led2_act_pol_sel = RU_FIELD_GET(led_id, LPORT_LED, CNTRL, SPDLNK_LED2_ACT_POL_SEL, reg_cntrl);
    cntrl->spdlnk_led1_act_pol_sel = RU_FIELD_GET(led_id, LPORT_LED, CNTRL, SPDLNK_LED1_ACT_POL_SEL, reg_cntrl);
    cntrl->spdlnk_led0_act_pol_sel = RU_FIELD_GET(led_id, LPORT_LED, CNTRL, SPDLNK_LED0_ACT_POL_SEL, reg_cntrl);
    cntrl->act_led_act_sel = RU_FIELD_GET(led_id, LPORT_LED, CNTRL, ACT_LED_ACT_SEL, reg_cntrl);
    cntrl->spdlnk_led2_act_sel = RU_FIELD_GET(led_id, LPORT_LED, CNTRL, SPDLNK_LED2_ACT_SEL, reg_cntrl);
    cntrl->spdlnk_led1_act_sel = RU_FIELD_GET(led_id, LPORT_LED, CNTRL, SPDLNK_LED1_ACT_SEL, reg_cntrl);
    cntrl->spdlnk_led0_act_sel = RU_FIELD_GET(led_id, LPORT_LED, CNTRL, SPDLNK_LED0_ACT_SEL, reg_cntrl);
    cntrl->tx_act_en = RU_FIELD_GET(led_id, LPORT_LED, CNTRL, TX_ACT_EN, reg_cntrl);
    cntrl->rx_act_en = RU_FIELD_GET(led_id, LPORT_LED, CNTRL, RX_ACT_EN, reg_cntrl);

    return 0;
}

int ag_drv_lport_led_link_and_speed_encoding_sel_set(uint8_t led_id, const lport_led_link_and_speed_encoding_sel *link_and_speed_encoding_sel)
{
    uint32_t reg_link_and_speed_encoding_sel=0;

#ifdef VALIDATE_PARMS
    if(!link_and_speed_encoding_sel)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 1);
        return 1;
    }
    if((led_id >= BLOCK_ADDR_COUNT) ||
       (link_and_speed_encoding_sel->rsvd_sel_spd_encode_2 >= _3BITS_MAX_VAL_) ||
       (link_and_speed_encoding_sel->rsvd_sel_spd_encode_1 >= _3BITS_MAX_VAL_) ||
       (link_and_speed_encoding_sel->sel_10g_encode >= _3BITS_MAX_VAL_) ||
       (link_and_speed_encoding_sel->sel_2500m_encode >= _3BITS_MAX_VAL_) ||
       (link_and_speed_encoding_sel->sel_1000m_encode >= _3BITS_MAX_VAL_) ||
       (link_and_speed_encoding_sel->sel_100m_encode >= _3BITS_MAX_VAL_) ||
       (link_and_speed_encoding_sel->sel_10m_encode >= _3BITS_MAX_VAL_) ||
       (link_and_speed_encoding_sel->sel_no_link_encode >= _3BITS_MAX_VAL_))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    reg_link_and_speed_encoding_sel = RU_FIELD_SET(led_id, LPORT_LED, LINK_AND_SPEED_ENCODING_SEL, RSVD_SEL_SPD_ENCODE_2, reg_link_and_speed_encoding_sel, link_and_speed_encoding_sel->rsvd_sel_spd_encode_2);
    reg_link_and_speed_encoding_sel = RU_FIELD_SET(led_id, LPORT_LED, LINK_AND_SPEED_ENCODING_SEL, RSVD_SEL_SPD_ENCODE_1, reg_link_and_speed_encoding_sel, link_and_speed_encoding_sel->rsvd_sel_spd_encode_1);
    reg_link_and_speed_encoding_sel = RU_FIELD_SET(led_id, LPORT_LED, LINK_AND_SPEED_ENCODING_SEL, SEL_10G_ENCODE, reg_link_and_speed_encoding_sel, link_and_speed_encoding_sel->sel_10g_encode);
    reg_link_and_speed_encoding_sel = RU_FIELD_SET(led_id, LPORT_LED, LINK_AND_SPEED_ENCODING_SEL, SEL_2500M_ENCODE, reg_link_and_speed_encoding_sel, link_and_speed_encoding_sel->sel_2500m_encode);
    reg_link_and_speed_encoding_sel = RU_FIELD_SET(led_id, LPORT_LED, LINK_AND_SPEED_ENCODING_SEL, SEL_1000M_ENCODE, reg_link_and_speed_encoding_sel, link_and_speed_encoding_sel->sel_1000m_encode);
    reg_link_and_speed_encoding_sel = RU_FIELD_SET(led_id, LPORT_LED, LINK_AND_SPEED_ENCODING_SEL, SEL_100M_ENCODE, reg_link_and_speed_encoding_sel, link_and_speed_encoding_sel->sel_100m_encode);
    reg_link_and_speed_encoding_sel = RU_FIELD_SET(led_id, LPORT_LED, LINK_AND_SPEED_ENCODING_SEL, SEL_10M_ENCODE, reg_link_and_speed_encoding_sel, link_and_speed_encoding_sel->sel_10m_encode);
    reg_link_and_speed_encoding_sel = RU_FIELD_SET(led_id, LPORT_LED, LINK_AND_SPEED_ENCODING_SEL, SEL_NO_LINK_ENCODE, reg_link_and_speed_encoding_sel, link_and_speed_encoding_sel->sel_no_link_encode);

    RU_REG_WRITE(led_id, LPORT_LED, LINK_AND_SPEED_ENCODING_SEL, reg_link_and_speed_encoding_sel);

    return 0;
}

int ag_drv_lport_led_link_and_speed_encoding_sel_get(uint8_t led_id, lport_led_link_and_speed_encoding_sel *link_and_speed_encoding_sel)
{
    uint32_t reg_link_and_speed_encoding_sel=0;

#ifdef VALIDATE_PARMS
    if(!link_and_speed_encoding_sel)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 2);
        return 2;
    }
    if((led_id >= BLOCK_ADDR_COUNT))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    RU_REG_READ(led_id, LPORT_LED, LINK_AND_SPEED_ENCODING_SEL, reg_link_and_speed_encoding_sel);

    link_and_speed_encoding_sel->rsvd_sel_spd_encode_2 = RU_FIELD_GET(led_id, LPORT_LED, LINK_AND_SPEED_ENCODING_SEL, RSVD_SEL_SPD_ENCODE_2, reg_link_and_speed_encoding_sel);
    link_and_speed_encoding_sel->rsvd_sel_spd_encode_1 = RU_FIELD_GET(led_id, LPORT_LED, LINK_AND_SPEED_ENCODING_SEL, RSVD_SEL_SPD_ENCODE_1, reg_link_and_speed_encoding_sel);
    link_and_speed_encoding_sel->sel_10g_encode = RU_FIELD_GET(led_id, LPORT_LED, LINK_AND_SPEED_ENCODING_SEL, SEL_10G_ENCODE, reg_link_and_speed_encoding_sel);
    link_and_speed_encoding_sel->sel_2500m_encode = RU_FIELD_GET(led_id, LPORT_LED, LINK_AND_SPEED_ENCODING_SEL, SEL_2500M_ENCODE, reg_link_and_speed_encoding_sel);
    link_and_speed_encoding_sel->sel_1000m_encode = RU_FIELD_GET(led_id, LPORT_LED, LINK_AND_SPEED_ENCODING_SEL, SEL_1000M_ENCODE, reg_link_and_speed_encoding_sel);
    link_and_speed_encoding_sel->sel_100m_encode = RU_FIELD_GET(led_id, LPORT_LED, LINK_AND_SPEED_ENCODING_SEL, SEL_100M_ENCODE, reg_link_and_speed_encoding_sel);
    link_and_speed_encoding_sel->sel_10m_encode = RU_FIELD_GET(led_id, LPORT_LED, LINK_AND_SPEED_ENCODING_SEL, SEL_10M_ENCODE, reg_link_and_speed_encoding_sel);
    link_and_speed_encoding_sel->sel_no_link_encode = RU_FIELD_GET(led_id, LPORT_LED, LINK_AND_SPEED_ENCODING_SEL, SEL_NO_LINK_ENCODE, reg_link_and_speed_encoding_sel);

    return 0;
}

int ag_drv_lport_led_link_and_speed_encoding_set(uint8_t led_id, const lport_led_link_and_speed_encoding *link_and_speed_encoding)
{
    uint32_t reg_link_and_speed_encoding=0;

#ifdef VALIDATE_PARMS
    if(!link_and_speed_encoding)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 1);
        return 1;
    }
    if((led_id >= BLOCK_ADDR_COUNT) ||
       (link_and_speed_encoding->rsvd_spd_encode_2 >= _3BITS_MAX_VAL_) ||
       (link_and_speed_encoding->rsvd_spd_encode_1 >= _3BITS_MAX_VAL_) ||
       (link_and_speed_encoding->m10g_encode >= _3BITS_MAX_VAL_) ||
       (link_and_speed_encoding->m2500_encode >= _3BITS_MAX_VAL_) ||
       (link_and_speed_encoding->m1000_encode >= _3BITS_MAX_VAL_) ||
       (link_and_speed_encoding->m100_encode >= _3BITS_MAX_VAL_) ||
       (link_and_speed_encoding->m10_encode >= _3BITS_MAX_VAL_) ||
       (link_and_speed_encoding->no_link_encode >= _3BITS_MAX_VAL_))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    reg_link_and_speed_encoding = RU_FIELD_SET(led_id, LPORT_LED, LINK_AND_SPEED_ENCODING, RSVD_SPD_ENCODE_2, reg_link_and_speed_encoding, link_and_speed_encoding->rsvd_spd_encode_2);
    reg_link_and_speed_encoding = RU_FIELD_SET(led_id, LPORT_LED, LINK_AND_SPEED_ENCODING, RSVD_SPD_ENCODE_1, reg_link_and_speed_encoding, link_and_speed_encoding->rsvd_spd_encode_1);
    reg_link_and_speed_encoding = RU_FIELD_SET(led_id, LPORT_LED, LINK_AND_SPEED_ENCODING, M10G_ENCODE, reg_link_and_speed_encoding, link_and_speed_encoding->m10g_encode);
    reg_link_and_speed_encoding = RU_FIELD_SET(led_id, LPORT_LED, LINK_AND_SPEED_ENCODING, M2500_ENCODE, reg_link_and_speed_encoding, link_and_speed_encoding->m2500_encode);
    reg_link_and_speed_encoding = RU_FIELD_SET(led_id, LPORT_LED, LINK_AND_SPEED_ENCODING, M1000_ENCODE, reg_link_and_speed_encoding, link_and_speed_encoding->m1000_encode);
    reg_link_and_speed_encoding = RU_FIELD_SET(led_id, LPORT_LED, LINK_AND_SPEED_ENCODING, M100_ENCODE, reg_link_and_speed_encoding, link_and_speed_encoding->m100_encode);
    reg_link_and_speed_encoding = RU_FIELD_SET(led_id, LPORT_LED, LINK_AND_SPEED_ENCODING, M10_ENCODE, reg_link_and_speed_encoding, link_and_speed_encoding->m10_encode);
    reg_link_and_speed_encoding = RU_FIELD_SET(led_id, LPORT_LED, LINK_AND_SPEED_ENCODING, NO_LINK_ENCODE, reg_link_and_speed_encoding, link_and_speed_encoding->no_link_encode);

    RU_REG_WRITE(led_id, LPORT_LED, LINK_AND_SPEED_ENCODING, reg_link_and_speed_encoding);

    return 0;
}

int ag_drv_lport_led_link_and_speed_encoding_get(uint8_t led_id, lport_led_link_and_speed_encoding *link_and_speed_encoding)
{
    uint32_t reg_link_and_speed_encoding=0;

#ifdef VALIDATE_PARMS
    if(!link_and_speed_encoding)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 2);
        return 2;
    }
    if((led_id >= BLOCK_ADDR_COUNT))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    RU_REG_READ(led_id, LPORT_LED, LINK_AND_SPEED_ENCODING, reg_link_and_speed_encoding);

    link_and_speed_encoding->rsvd_spd_encode_2 = RU_FIELD_GET(led_id, LPORT_LED, LINK_AND_SPEED_ENCODING, RSVD_SPD_ENCODE_2, reg_link_and_speed_encoding);
    link_and_speed_encoding->rsvd_spd_encode_1 = RU_FIELD_GET(led_id, LPORT_LED, LINK_AND_SPEED_ENCODING, RSVD_SPD_ENCODE_1, reg_link_and_speed_encoding);
    link_and_speed_encoding->m10g_encode = RU_FIELD_GET(led_id, LPORT_LED, LINK_AND_SPEED_ENCODING, M10G_ENCODE, reg_link_and_speed_encoding);
    link_and_speed_encoding->m2500_encode = RU_FIELD_GET(led_id, LPORT_LED, LINK_AND_SPEED_ENCODING, M2500_ENCODE, reg_link_and_speed_encoding);
    link_and_speed_encoding->m1000_encode = RU_FIELD_GET(led_id, LPORT_LED, LINK_AND_SPEED_ENCODING, M1000_ENCODE, reg_link_and_speed_encoding);
    link_and_speed_encoding->m100_encode = RU_FIELD_GET(led_id, LPORT_LED, LINK_AND_SPEED_ENCODING, M100_ENCODE, reg_link_and_speed_encoding);
    link_and_speed_encoding->m10_encode = RU_FIELD_GET(led_id, LPORT_LED, LINK_AND_SPEED_ENCODING, M10_ENCODE, reg_link_and_speed_encoding);
    link_and_speed_encoding->no_link_encode = RU_FIELD_GET(led_id, LPORT_LED, LINK_AND_SPEED_ENCODING, NO_LINK_ENCODE, reg_link_and_speed_encoding);

    return 0;
}

int ag_drv_lport_led_aggregate_led_cntrl_set(uint8_t led_id, uint8_t lnk_pol_sel, uint8_t act_pol_sel, uint8_t act_sel, uint16_t port_en)
{
    uint32_t reg_aggregate_led_cntrl=0;

#ifdef VALIDATE_PARMS
    if((led_id >= BLOCK_ADDR_COUNT) ||
       (lnk_pol_sel >= _1BITS_MAX_VAL_) ||
       (act_pol_sel >= _1BITS_MAX_VAL_) ||
       (act_sel >= _1BITS_MAX_VAL_))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    reg_aggregate_led_cntrl = RU_FIELD_SET(led_id, LPORT_LED, AGGREGATE_LED_CNTRL, LNK_POL_SEL, reg_aggregate_led_cntrl, lnk_pol_sel);
    reg_aggregate_led_cntrl = RU_FIELD_SET(led_id, LPORT_LED, AGGREGATE_LED_CNTRL, ACT_POL_SEL, reg_aggregate_led_cntrl, act_pol_sel);
    reg_aggregate_led_cntrl = RU_FIELD_SET(led_id, LPORT_LED, AGGREGATE_LED_CNTRL, ACT_SEL, reg_aggregate_led_cntrl, act_sel);
    reg_aggregate_led_cntrl = RU_FIELD_SET(led_id, LPORT_LED, AGGREGATE_LED_CNTRL, PORT_EN, reg_aggregate_led_cntrl, port_en);

    RU_REG_WRITE(led_id, LPORT_LED, AGGREGATE_LED_CNTRL, reg_aggregate_led_cntrl);

    return 0;
}

int ag_drv_lport_led_aggregate_led_cntrl_get(uint8_t led_id, uint8_t *lnk_pol_sel, uint8_t *act_pol_sel, uint8_t *act_sel, uint16_t *port_en)
{
    uint32_t reg_aggregate_led_cntrl=0;

#ifdef VALIDATE_PARMS
    if(!lnk_pol_sel || !act_pol_sel || !act_sel || !port_en)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 2);
        return 2;
    }
    if((led_id >= BLOCK_ADDR_COUNT))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    RU_REG_READ(led_id, LPORT_LED, AGGREGATE_LED_CNTRL, reg_aggregate_led_cntrl);

    *lnk_pol_sel = RU_FIELD_GET(led_id, LPORT_LED, AGGREGATE_LED_CNTRL, LNK_POL_SEL, reg_aggregate_led_cntrl);
    *act_pol_sel = RU_FIELD_GET(led_id, LPORT_LED, AGGREGATE_LED_CNTRL, ACT_POL_SEL, reg_aggregate_led_cntrl);
    *act_sel = RU_FIELD_GET(led_id, LPORT_LED, AGGREGATE_LED_CNTRL, ACT_SEL, reg_aggregate_led_cntrl);
    *port_en = RU_FIELD_GET(led_id, LPORT_LED, AGGREGATE_LED_CNTRL, PORT_EN, reg_aggregate_led_cntrl);

    return 0;
}

int ag_drv_lport_led_aggregate_led_blink_rate_cntrl_set(uint8_t led_id, uint16_t led_on_time, uint16_t led_off_time)
{
    uint32_t reg_aggregate_led_blink_rate_cntrl=0;

#ifdef VALIDATE_PARMS
    if((led_id >= BLOCK_ADDR_COUNT))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    reg_aggregate_led_blink_rate_cntrl = RU_FIELD_SET(led_id, LPORT_LED, AGGREGATE_LED_BLINK_RATE_CNTRL, LED_ON_TIME, reg_aggregate_led_blink_rate_cntrl, led_on_time);
    reg_aggregate_led_blink_rate_cntrl = RU_FIELD_SET(led_id, LPORT_LED, AGGREGATE_LED_BLINK_RATE_CNTRL, LED_OFF_TIME, reg_aggregate_led_blink_rate_cntrl, led_off_time);

    RU_REG_WRITE(led_id, LPORT_LED, AGGREGATE_LED_BLINK_RATE_CNTRL, reg_aggregate_led_blink_rate_cntrl);

    return 0;
}

int ag_drv_lport_led_aggregate_led_blink_rate_cntrl_get(uint8_t led_id, uint16_t *led_on_time, uint16_t *led_off_time)
{
    uint32_t reg_aggregate_led_blink_rate_cntrl=0;

#ifdef VALIDATE_PARMS
    if(!led_on_time || !led_off_time)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 2);
        return 2;
    }
    if((led_id >= BLOCK_ADDR_COUNT))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    RU_REG_READ(led_id, LPORT_LED, AGGREGATE_LED_BLINK_RATE_CNTRL, reg_aggregate_led_blink_rate_cntrl);

    *led_on_time = RU_FIELD_GET(led_id, LPORT_LED, AGGREGATE_LED_BLINK_RATE_CNTRL, LED_ON_TIME, reg_aggregate_led_blink_rate_cntrl);
    *led_off_time = RU_FIELD_GET(led_id, LPORT_LED, AGGREGATE_LED_BLINK_RATE_CNTRL, LED_OFF_TIME, reg_aggregate_led_blink_rate_cntrl);

    return 0;
}

