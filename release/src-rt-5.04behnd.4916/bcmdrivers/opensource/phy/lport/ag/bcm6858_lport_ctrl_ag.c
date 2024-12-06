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
#include "bcm6858_lport_ctrl_ag.h"
int ag_drv_lport_ctrl_control_set(const lport_ctrl_control *control)
{
    uint32_t reg_lport_cntrl=0;

#ifdef VALIDATE_PARMS
    if(!control)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 1);
        return 1;
    }
    if((control->timeout_rst_disable >= _1BITS_MAX_VAL_) ||
       (control->p4_mode >= _1BITS_MAX_VAL_) ||
       (control->p0_mode >= _1BITS_MAX_VAL_) ||
       (control->gport_sel_7 >= _1BITS_MAX_VAL_) ||
       (control->gport_sel_6 >= _1BITS_MAX_VAL_) ||
       (control->gport_sel_5 >= _1BITS_MAX_VAL_) ||
       (control->gport_sel_4 >= _1BITS_MAX_VAL_) ||
       (control->gport_sel_3 >= _1BITS_MAX_VAL_) ||
       (control->gport_sel_2 >= _1BITS_MAX_VAL_) ||
       (control->gport_sel_1 >= _1BITS_MAX_VAL_) ||
       (control->gport_sel_0 >= _1BITS_MAX_VAL_))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    reg_lport_cntrl = RU_FIELD_SET(0, LPORT_CTRL, LPORT_CNTRL, TIMEOUT_RST_DISABLE, reg_lport_cntrl, control->timeout_rst_disable);
    reg_lport_cntrl = RU_FIELD_SET(0, LPORT_CTRL, LPORT_CNTRL, P4_MODE, reg_lport_cntrl, control->p4_mode);
    reg_lport_cntrl = RU_FIELD_SET(0, LPORT_CTRL, LPORT_CNTRL, P0_MODE, reg_lport_cntrl, control->p0_mode);
    reg_lport_cntrl = RU_FIELD_SET(0, LPORT_CTRL, LPORT_CNTRL, GPORT_SEL_7, reg_lport_cntrl, control->gport_sel_7);
    reg_lport_cntrl = RU_FIELD_SET(0, LPORT_CTRL, LPORT_CNTRL, GPORT_SEL_6, reg_lport_cntrl, control->gport_sel_6);
    reg_lport_cntrl = RU_FIELD_SET(0, LPORT_CTRL, LPORT_CNTRL, GPORT_SEL_5, reg_lport_cntrl, control->gport_sel_5);
    reg_lport_cntrl = RU_FIELD_SET(0, LPORT_CTRL, LPORT_CNTRL, GPORT_SEL_4, reg_lport_cntrl, control->gport_sel_4);
    reg_lport_cntrl = RU_FIELD_SET(0, LPORT_CTRL, LPORT_CNTRL, GPORT_SEL_3, reg_lport_cntrl, control->gport_sel_3);
    reg_lport_cntrl = RU_FIELD_SET(0, LPORT_CTRL, LPORT_CNTRL, GPORT_SEL_2, reg_lport_cntrl, control->gport_sel_2);
    reg_lport_cntrl = RU_FIELD_SET(0, LPORT_CTRL, LPORT_CNTRL, GPORT_SEL_1, reg_lport_cntrl, control->gport_sel_1);
    reg_lport_cntrl = RU_FIELD_SET(0, LPORT_CTRL, LPORT_CNTRL, GPORT_SEL_0, reg_lport_cntrl, control->gport_sel_0);

    RU_REG_WRITE(0, LPORT_CTRL, LPORT_CNTRL, reg_lport_cntrl);

    return 0;
}

int ag_drv_lport_ctrl_control_get(lport_ctrl_control *control)
{
    uint32_t reg_lport_cntrl=0;

#ifdef VALIDATE_PARMS
    if(!control)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 2);
        return 2;
    }
#endif

    RU_REG_READ(0, LPORT_CTRL, LPORT_CNTRL, reg_lport_cntrl);

    control->timeout_rst_disable = RU_FIELD_GET(0, LPORT_CTRL, LPORT_CNTRL, TIMEOUT_RST_DISABLE, reg_lport_cntrl);
    control->p4_mode = RU_FIELD_GET(0, LPORT_CTRL, LPORT_CNTRL, P4_MODE, reg_lport_cntrl);
    control->p0_mode = RU_FIELD_GET(0, LPORT_CTRL, LPORT_CNTRL, P0_MODE, reg_lport_cntrl);
    control->gport_sel_7 = RU_FIELD_GET(0, LPORT_CTRL, LPORT_CNTRL, GPORT_SEL_7, reg_lport_cntrl);
    control->gport_sel_6 = RU_FIELD_GET(0, LPORT_CTRL, LPORT_CNTRL, GPORT_SEL_6, reg_lport_cntrl);
    control->gport_sel_5 = RU_FIELD_GET(0, LPORT_CTRL, LPORT_CNTRL, GPORT_SEL_5, reg_lport_cntrl);
    control->gport_sel_4 = RU_FIELD_GET(0, LPORT_CTRL, LPORT_CNTRL, GPORT_SEL_4, reg_lport_cntrl);
    control->gport_sel_3 = RU_FIELD_GET(0, LPORT_CTRL, LPORT_CNTRL, GPORT_SEL_3, reg_lport_cntrl);
    control->gport_sel_2 = RU_FIELD_GET(0, LPORT_CTRL, LPORT_CNTRL, GPORT_SEL_2, reg_lport_cntrl);
    control->gport_sel_1 = RU_FIELD_GET(0, LPORT_CTRL, LPORT_CNTRL, GPORT_SEL_1, reg_lport_cntrl);
    control->gport_sel_0 = RU_FIELD_GET(0, LPORT_CTRL, LPORT_CNTRL, GPORT_SEL_0, reg_lport_cntrl);

    return 0;
}

int ag_drv_lport_ctrl_lport_revision_get(uint16_t *lport_rev)
{
    uint32_t reg_lport_revision=0;

#ifdef VALIDATE_PARMS
    if(!lport_rev)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 2);
        return 2;
    }
#endif

    RU_REG_READ(0, LPORT_CTRL, LPORT_REVISION, reg_lport_revision);

    *lport_rev = RU_FIELD_GET(0, LPORT_CTRL, LPORT_REVISION, LPORT_REV, reg_lport_revision);

    return 0;
}

int ag_drv_lport_ctrl_qegphy_revision_get(uint16_t *quad_phy_rev)
{
    uint32_t reg_qegphy_revision=0;

#ifdef VALIDATE_PARMS
    if(!quad_phy_rev)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 2);
        return 2;
    }
#endif

    RU_REG_READ(0, LPORT_CTRL, QEGPHY_REVISION, reg_qegphy_revision);

    *quad_phy_rev = RU_FIELD_GET(0, LPORT_CTRL, QEGPHY_REVISION, QUAD_PHY_REV, reg_qegphy_revision);

    return 0;
}

int ag_drv_lport_ctrl_qegphy_test_cntrl_set(uint8_t pll_refclk_sel, uint8_t pll_sel_div5, uint8_t pll_clk125_250_sel, uint8_t phy_test_en)
{
    uint32_t reg_qegphy_test_cntrl=0;

#ifdef VALIDATE_PARMS
    if((pll_refclk_sel >= _2BITS_MAX_VAL_) ||
       (pll_sel_div5 >= _2BITS_MAX_VAL_) ||
       (pll_clk125_250_sel >= _1BITS_MAX_VAL_) ||
       (phy_test_en >= _1BITS_MAX_VAL_))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    reg_qegphy_test_cntrl = RU_FIELD_SET(0, LPORT_CTRL, QEGPHY_TEST_CNTRL, PLL_REFCLK_SEL, reg_qegphy_test_cntrl, pll_refclk_sel);
    reg_qegphy_test_cntrl = RU_FIELD_SET(0, LPORT_CTRL, QEGPHY_TEST_CNTRL, PLL_SEL_DIV5, reg_qegphy_test_cntrl, pll_sel_div5);
    reg_qegphy_test_cntrl = RU_FIELD_SET(0, LPORT_CTRL, QEGPHY_TEST_CNTRL, PLL_CLK125_250_SEL, reg_qegphy_test_cntrl, pll_clk125_250_sel);
    reg_qegphy_test_cntrl = RU_FIELD_SET(0, LPORT_CTRL, QEGPHY_TEST_CNTRL, PHY_TEST_EN, reg_qegphy_test_cntrl, phy_test_en);

    RU_REG_WRITE(0, LPORT_CTRL, QEGPHY_TEST_CNTRL, reg_qegphy_test_cntrl);

    return 0;
}

int ag_drv_lport_ctrl_qegphy_test_cntrl_get(uint8_t *pll_refclk_sel, uint8_t *pll_sel_div5, uint8_t *pll_clk125_250_sel, uint8_t *phy_test_en)
{
    uint32_t reg_qegphy_test_cntrl=0;

#ifdef VALIDATE_PARMS
    if(!pll_refclk_sel || !pll_sel_div5 || !pll_clk125_250_sel || !phy_test_en)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 2);
        return 2;
    }
#endif

    RU_REG_READ(0, LPORT_CTRL, QEGPHY_TEST_CNTRL, reg_qegphy_test_cntrl);

    *pll_refclk_sel = RU_FIELD_GET(0, LPORT_CTRL, QEGPHY_TEST_CNTRL, PLL_REFCLK_SEL, reg_qegphy_test_cntrl);
    *pll_sel_div5 = RU_FIELD_GET(0, LPORT_CTRL, QEGPHY_TEST_CNTRL, PLL_SEL_DIV5, reg_qegphy_test_cntrl);
    *pll_clk125_250_sel = RU_FIELD_GET(0, LPORT_CTRL, QEGPHY_TEST_CNTRL, PLL_CLK125_250_SEL, reg_qegphy_test_cntrl);
    *phy_test_en = RU_FIELD_GET(0, LPORT_CTRL, QEGPHY_TEST_CNTRL, PHY_TEST_EN, reg_qegphy_test_cntrl);

    return 0;
}

int ag_drv_lport_ctrl_qegphy_cntrl_set(const lport_ctrl_qegphy_cntrl *qegphy_cntrl)
{
    uint32_t reg_qegphy_cntrl=0;

#ifdef VALIDATE_PARMS
    if(!qegphy_cntrl)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 1);
        return 1;
    }
    if((qegphy_cntrl->phy_phyad >= _5BITS_MAX_VAL_) ||
       (qegphy_cntrl->phy_reset >= _1BITS_MAX_VAL_) ||
       (qegphy_cntrl->ck25_en >= _1BITS_MAX_VAL_) ||
       (qegphy_cntrl->iddq_global_pwr >= _1BITS_MAX_VAL_) ||
       (qegphy_cntrl->force_dll_en >= _1BITS_MAX_VAL_) ||
       (qegphy_cntrl->ext_pwr_down >= _4BITS_MAX_VAL_) ||
       (qegphy_cntrl->iddq_bias >= _1BITS_MAX_VAL_))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    reg_qegphy_cntrl = RU_FIELD_SET(0, LPORT_CTRL, QEGPHY_CNTRL, PHY_PHYAD, reg_qegphy_cntrl, qegphy_cntrl->phy_phyad);
    reg_qegphy_cntrl = RU_FIELD_SET(0, LPORT_CTRL, QEGPHY_CNTRL, PHY_RESET, reg_qegphy_cntrl, qegphy_cntrl->phy_reset);
    reg_qegphy_cntrl = RU_FIELD_SET(0, LPORT_CTRL, QEGPHY_CNTRL, CK25_EN, reg_qegphy_cntrl, qegphy_cntrl->ck25_en);
    reg_qegphy_cntrl = RU_FIELD_SET(0, LPORT_CTRL, QEGPHY_CNTRL, IDDQ_GLOBAL_PWR, reg_qegphy_cntrl, qegphy_cntrl->iddq_global_pwr);
    reg_qegphy_cntrl = RU_FIELD_SET(0, LPORT_CTRL, QEGPHY_CNTRL, FORCE_DLL_EN, reg_qegphy_cntrl, qegphy_cntrl->force_dll_en);
    reg_qegphy_cntrl = RU_FIELD_SET(0, LPORT_CTRL, QEGPHY_CNTRL, EXT_PWR_DOWN, reg_qegphy_cntrl, qegphy_cntrl->ext_pwr_down);
    reg_qegphy_cntrl = RU_FIELD_SET(0, LPORT_CTRL, QEGPHY_CNTRL, IDDQ_BIAS, reg_qegphy_cntrl, qegphy_cntrl->iddq_bias);

    RU_REG_WRITE(0, LPORT_CTRL, QEGPHY_CNTRL, reg_qegphy_cntrl);

    return 0;
}

int ag_drv_lport_ctrl_qegphy_cntrl_get(lport_ctrl_qegphy_cntrl *qegphy_cntrl)
{
    uint32_t reg_qegphy_cntrl=0;

#ifdef VALIDATE_PARMS
    if(!qegphy_cntrl)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 2);
        return 2;
    }
#endif

    RU_REG_READ(0, LPORT_CTRL, QEGPHY_CNTRL, reg_qegphy_cntrl);

    qegphy_cntrl->phy_phyad = RU_FIELD_GET(0, LPORT_CTRL, QEGPHY_CNTRL, PHY_PHYAD, reg_qegphy_cntrl);
    qegphy_cntrl->phy_reset = RU_FIELD_GET(0, LPORT_CTRL, QEGPHY_CNTRL, PHY_RESET, reg_qegphy_cntrl);
    qegphy_cntrl->ck25_en = RU_FIELD_GET(0, LPORT_CTRL, QEGPHY_CNTRL, CK25_EN, reg_qegphy_cntrl);
    qegphy_cntrl->iddq_global_pwr = RU_FIELD_GET(0, LPORT_CTRL, QEGPHY_CNTRL, IDDQ_GLOBAL_PWR, reg_qegphy_cntrl);
    qegphy_cntrl->force_dll_en = RU_FIELD_GET(0, LPORT_CTRL, QEGPHY_CNTRL, FORCE_DLL_EN, reg_qegphy_cntrl);
    qegphy_cntrl->ext_pwr_down = RU_FIELD_GET(0, LPORT_CTRL, QEGPHY_CNTRL, EXT_PWR_DOWN, reg_qegphy_cntrl);
    qegphy_cntrl->iddq_bias = RU_FIELD_GET(0, LPORT_CTRL, QEGPHY_CNTRL, IDDQ_BIAS, reg_qegphy_cntrl);

    return 0;
}

int ag_drv_lport_ctrl_qegphy_status_get(lport_ctrl_qegphy_status *qegphy_status)
{
    uint32_t reg_qegphy_status=0;

#ifdef VALIDATE_PARMS
    if(!qegphy_status)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 2);
        return 2;
    }
#endif

    RU_REG_READ(0, LPORT_CTRL, QEGPHY_STATUS, reg_qegphy_status);

    qegphy_status->gphy_test_status = RU_FIELD_GET(0, LPORT_CTRL, QEGPHY_STATUS, GPHY_TEST_STATUS, reg_qegphy_status);
    qegphy_status->recovered_clk_lock = RU_FIELD_GET(0, LPORT_CTRL, QEGPHY_STATUS, RECOVERED_CLK_LOCK, reg_qegphy_status);
    qegphy_status->pll_lock = RU_FIELD_GET(0, LPORT_CTRL, QEGPHY_STATUS, PLL_LOCK, reg_qegphy_status);
    qegphy_status->energy_det_apd = RU_FIELD_GET(0, LPORT_CTRL, QEGPHY_STATUS, ENERGY_DET_APD, reg_qegphy_status);
    qegphy_status->energy_det_masked = RU_FIELD_GET(0, LPORT_CTRL, QEGPHY_STATUS, ENERGY_DET_MASKED, reg_qegphy_status);

    return 0;
}

int ag_drv_lport_ctrl_led_blink_rate_cntrl_set(uint16_t led_on_time, uint16_t led_off_time)
{
    uint32_t reg_led_blink_rate_cntrl=0;

#ifdef VALIDATE_PARMS
#endif

    reg_led_blink_rate_cntrl = RU_FIELD_SET(0, LPORT_CTRL, LED_BLINK_RATE_CNTRL, LED_ON_TIME, reg_led_blink_rate_cntrl, led_on_time);
    reg_led_blink_rate_cntrl = RU_FIELD_SET(0, LPORT_CTRL, LED_BLINK_RATE_CNTRL, LED_OFF_TIME, reg_led_blink_rate_cntrl, led_off_time);

    RU_REG_WRITE(0, LPORT_CTRL, LED_BLINK_RATE_CNTRL, reg_led_blink_rate_cntrl);

    return 0;
}

int ag_drv_lport_ctrl_led_blink_rate_cntrl_get(uint16_t *led_on_time, uint16_t *led_off_time)
{
    uint32_t reg_led_blink_rate_cntrl=0;

#ifdef VALIDATE_PARMS
    if(!led_on_time || !led_off_time)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 2);
        return 2;
    }
#endif

    RU_REG_READ(0, LPORT_CTRL, LED_BLINK_RATE_CNTRL, reg_led_blink_rate_cntrl);

    *led_on_time = RU_FIELD_GET(0, LPORT_CTRL, LED_BLINK_RATE_CNTRL, LED_ON_TIME, reg_led_blink_rate_cntrl);
    *led_off_time = RU_FIELD_GET(0, LPORT_CTRL, LED_BLINK_RATE_CNTRL, LED_OFF_TIME, reg_led_blink_rate_cntrl);

    return 0;
}

int ag_drv_lport_ctrl_led_serial_cntrl_set(const lport_ctrl_led_serial_cntrl *led_serial_cntrl)
{
    uint32_t reg_led_serial_cntrl=0;

#ifdef VALIDATE_PARMS
    if(!led_serial_cntrl)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 1);
        return 1;
    }
    if((led_serial_cntrl->smode >= _2BITS_MAX_VAL_) ||
       (led_serial_cntrl->sled_clk_frequency >= _1BITS_MAX_VAL_) ||
       (led_serial_cntrl->sled_clk_pol >= _1BITS_MAX_VAL_) ||
       (led_serial_cntrl->refresh_period >= _5BITS_MAX_VAL_))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    reg_led_serial_cntrl = RU_FIELD_SET(0, LPORT_CTRL, LED_SERIAL_CNTRL, SMODE, reg_led_serial_cntrl, led_serial_cntrl->smode);
    reg_led_serial_cntrl = RU_FIELD_SET(0, LPORT_CTRL, LED_SERIAL_CNTRL, SLED_CLK_FREQUENCY, reg_led_serial_cntrl, led_serial_cntrl->sled_clk_frequency);
    reg_led_serial_cntrl = RU_FIELD_SET(0, LPORT_CTRL, LED_SERIAL_CNTRL, SLED_CLK_POL, reg_led_serial_cntrl, led_serial_cntrl->sled_clk_pol);
    reg_led_serial_cntrl = RU_FIELD_SET(0, LPORT_CTRL, LED_SERIAL_CNTRL, REFRESH_PERIOD, reg_led_serial_cntrl, led_serial_cntrl->refresh_period);
    reg_led_serial_cntrl = RU_FIELD_SET(0, LPORT_CTRL, LED_SERIAL_CNTRL, PORT_EN, reg_led_serial_cntrl, led_serial_cntrl->port_en);

    RU_REG_WRITE(0, LPORT_CTRL, LED_SERIAL_CNTRL, reg_led_serial_cntrl);

    return 0;
}

int ag_drv_lport_ctrl_led_serial_cntrl_get(lport_ctrl_led_serial_cntrl *led_serial_cntrl)
{
    uint32_t reg_led_serial_cntrl=0;

#ifdef VALIDATE_PARMS
    if(!led_serial_cntrl)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 2);
        return 2;
    }
#endif

    RU_REG_READ(0, LPORT_CTRL, LED_SERIAL_CNTRL, reg_led_serial_cntrl);

    led_serial_cntrl->smode = RU_FIELD_GET(0, LPORT_CTRL, LED_SERIAL_CNTRL, SMODE, reg_led_serial_cntrl);
    led_serial_cntrl->sled_clk_frequency = RU_FIELD_GET(0, LPORT_CTRL, LED_SERIAL_CNTRL, SLED_CLK_FREQUENCY, reg_led_serial_cntrl);
    led_serial_cntrl->sled_clk_pol = RU_FIELD_GET(0, LPORT_CTRL, LED_SERIAL_CNTRL, SLED_CLK_POL, reg_led_serial_cntrl);
    led_serial_cntrl->refresh_period = RU_FIELD_GET(0, LPORT_CTRL, LED_SERIAL_CNTRL, REFRESH_PERIOD, reg_led_serial_cntrl);
    led_serial_cntrl->port_en = RU_FIELD_GET(0, LPORT_CTRL, LED_SERIAL_CNTRL, PORT_EN, reg_led_serial_cntrl);

    return 0;
}

