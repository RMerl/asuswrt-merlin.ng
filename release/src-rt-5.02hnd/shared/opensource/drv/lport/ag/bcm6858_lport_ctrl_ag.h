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

#ifndef _BCM6858_LPORT_CTRL_AG_H_
#define _BCM6858_LPORT_CTRL_AG_H_

#include "access_macros.h"
#include "bcmtypes.h"
typedef struct
{
    uint8_t timeout_rst_disable;
    uint8_t p4_mode;
    uint8_t p0_mode;
    uint8_t gport_sel_7;
    uint8_t gport_sel_6;
    uint8_t gport_sel_5;
    uint8_t gport_sel_4;
    uint8_t gport_sel_3;
    uint8_t gport_sel_2;
    uint8_t gport_sel_1;
    uint8_t gport_sel_0;
} lport_ctrl_control;

typedef struct
{
    uint8_t phy_phyad;
    uint8_t phy_reset;
    uint8_t ck25_en;
    uint8_t iddq_global_pwr;
    uint8_t force_dll_en;
    uint8_t ext_pwr_down;
    uint8_t iddq_bias;
} lport_ctrl_qegphy_cntrl;

typedef struct
{
    uint8_t gphy_test_status;
    uint8_t recovered_clk_lock;
    uint8_t pll_lock;
    uint8_t energy_det_apd;
    uint8_t energy_det_masked;
} lport_ctrl_qegphy_status;

typedef struct
{
    uint8_t smode;
    uint8_t sled_clk_frequency;
    uint8_t sled_clk_pol;
    uint8_t refresh_period;
    uint16_t port_en;
} lport_ctrl_led_serial_cntrl;

int ag_drv_lport_ctrl_control_set(const lport_ctrl_control *control);
int ag_drv_lport_ctrl_control_get(lport_ctrl_control *control);
int ag_drv_lport_ctrl_lport_revision_get(uint16_t *lport_rev);
int ag_drv_lport_ctrl_qegphy_revision_get(uint16_t *quad_phy_rev);
int ag_drv_lport_ctrl_qegphy_test_cntrl_set(uint8_t pll_refclk_sel, uint8_t pll_sel_div5, uint8_t pll_clk125_250_sel, uint8_t phy_test_en);
int ag_drv_lport_ctrl_qegphy_test_cntrl_get(uint8_t *pll_refclk_sel, uint8_t *pll_sel_div5, uint8_t *pll_clk125_250_sel, uint8_t *phy_test_en);
int ag_drv_lport_ctrl_qegphy_cntrl_set(const lport_ctrl_qegphy_cntrl *qegphy_cntrl);
int ag_drv_lport_ctrl_qegphy_cntrl_get(lport_ctrl_qegphy_cntrl *qegphy_cntrl);
int ag_drv_lport_ctrl_qegphy_status_get(lport_ctrl_qegphy_status *qegphy_status);
int ag_drv_lport_ctrl_led_blink_rate_cntrl_set(uint16_t led_on_time, uint16_t led_off_time);
int ag_drv_lport_ctrl_led_blink_rate_cntrl_get(uint16_t *led_on_time, uint16_t *led_off_time);
int ag_drv_lport_ctrl_led_serial_cntrl_set(const lport_ctrl_led_serial_cntrl *led_serial_cntrl);
int ag_drv_lport_ctrl_led_serial_cntrl_get(lport_ctrl_led_serial_cntrl *led_serial_cntrl);

#ifdef USE_BDMF_SHELL
bdmfmon_handle_t ag_drv_lport_ctrl_cli_init(bdmfmon_handle_t driver_dir);
#endif


#endif

