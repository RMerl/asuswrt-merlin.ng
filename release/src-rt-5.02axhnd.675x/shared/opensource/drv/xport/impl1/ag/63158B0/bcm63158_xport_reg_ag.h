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

#ifndef _BCM63158_XPORT_REG_AG_H_
#define _BCM63158_XPORT_REG_AG_H_

//#include "access_macros.h"
#include "bcmtypes.h"
typedef struct
{
    uint8_t lnk_ovrd_en;
    uint8_t spd_ovrd_en;
    uint8_t lnk_status_ovrd;
    uint8_t led_spd_ovrd;
    uint8_t act_led_pol_sel;
    uint8_t spdlnk_led2_act_pol_sel;
    uint8_t spdlnk_led1_act_pol_sel;
    uint8_t spdlnk_led0_act_pol_sel;
    uint8_t act_led_act_sel;
    uint8_t spdlnk_led2_act_sel;
    uint8_t spdlnk_led1_act_sel;
    uint8_t spdlnk_led0_act_sel;
    uint8_t tx_act_en;
    uint8_t rx_act_en;
} xport_reg_led_0_cntrl;

typedef struct
{
    uint8_t rsvd_sel_spd_encode_2;
    uint8_t rsvd_sel_spd_encode_1;
    uint8_t sel_10g_encode;
    uint8_t sel_2500m_encode;
    uint8_t sel_1000m_encode;
    uint8_t sel_100m_encode;
    uint8_t sel_10m_encode;
    uint8_t sel_no_link_encode;
} xport_reg_led_0_link_and_speed_encoding_sel;

typedef struct
{
    uint8_t rsvd_spd_encode_2;
    uint8_t rsvd_spd_encode_1;
    uint8_t m10g_encode;
    uint8_t m2500_encode;
    uint8_t m1000_encode;
    uint8_t m100_encode;
    uint8_t m10_encode;
    uint8_t no_link_encode;
} xport_reg_led_0_link_and_speed_encoding;

typedef struct
{
    uint8_t lnk_ovrd_en;
    uint8_t spd_ovrd_en;
    uint8_t lnk_status_ovrd;
    uint8_t led_spd_ovrd;
    uint8_t act_led_pol_sel;
    uint8_t spdlnk_led2_act_pol_sel;
    uint8_t spdlnk_led1_act_pol_sel;
    uint8_t spdlnk_led0_act_pol_sel;
    uint8_t act_led_act_sel;
    uint8_t spdlnk_led2_act_sel;
    uint8_t spdlnk_led1_act_sel;
    uint8_t spdlnk_led0_act_sel;
    uint8_t tx_act_en;
    uint8_t rx_act_en;
} xport_reg_led_1_cntrl;

typedef struct
{
    uint8_t rsvd_sel_spd_encode_2;
    uint8_t rsvd_sel_spd_encode_1;
    uint8_t sel_10g_encode;
    uint8_t sel_2500m_encode;
    uint8_t sel_1000m_encode;
    uint8_t sel_100m_encode;
    uint8_t sel_10m_encode;
    uint8_t sel_no_link_encode;
} xport_reg_led_1_link_and_speed_encoding_sel;

typedef struct
{
    uint8_t rsvd_spd_encode_2;
    uint8_t rsvd_spd_encode_1;
    uint8_t m10g_encode;
    uint8_t m2500_encode;
    uint8_t m1000_encode;
    uint8_t m100_encode;
    uint8_t m10_encode;
    uint8_t no_link_encode;
} xport_reg_led_1_link_and_speed_encoding;

typedef struct
{
    uint8_t smode;
    uint8_t sled_clk_frequency;
    uint8_t sled_clk_pol;
    uint8_t refresh_period;
    uint16_t port_en;
} xport_reg_led_serial_ctrl;

typedef struct
{
    uint8_t full_duplex;
    uint8_t pause_tx;
    uint8_t pause_rx;
    uint8_t speed_2500;
    uint8_t speed_1000;
    uint8_t speed_100;
    uint8_t speed_10;
    uint8_t link_status;
} xport_reg_crossbar_status;

typedef struct
{
    uint8_t mod_def0;
    uint8_t ext_sig_det;
    uint8_t pll1_lock;
    uint8_t pll0_lock;
    uint8_t link_status;
    uint8_t cdr_lock;
    uint8_t rx_sigdet;
} xport_reg_pon_ae_serdes_status;

int ag_drv_xport_reg_xport_revision_get(uint32_t *xport_rev);
int ag_drv_xport_reg_led_pwm_cntrl_set(uint8_t pwm_enable);
int ag_drv_xport_reg_led_pwm_cntrl_get(uint8_t *pwm_enable);
int ag_drv_xport_reg_led_intensity_cntrl_set(uint16_t led_on_low, uint16_t led_on_high);
int ag_drv_xport_reg_led_intensity_cntrl_get(uint16_t *led_on_low, uint16_t *led_on_high);
int ag_drv_xport_reg_led_0_cntrl_set(const xport_reg_led_0_cntrl *led_0_cntrl);
int ag_drv_xport_reg_led_0_cntrl_get(xport_reg_led_0_cntrl *led_0_cntrl);
int ag_drv_xport_reg_led_0_link_and_speed_encoding_sel_set(const xport_reg_led_0_link_and_speed_encoding_sel *led_0_link_and_speed_encoding_sel);
int ag_drv_xport_reg_led_0_link_and_speed_encoding_sel_get(xport_reg_led_0_link_and_speed_encoding_sel *led_0_link_and_speed_encoding_sel);
int ag_drv_xport_reg_led_0_link_and_speed_encoding_set(const xport_reg_led_0_link_and_speed_encoding *led_0_link_and_speed_encoding);
int ag_drv_xport_reg_led_0_link_and_speed_encoding_get(xport_reg_led_0_link_and_speed_encoding *led_0_link_and_speed_encoding);
int ag_drv_xport_reg_led_1_cntrl_set(const xport_reg_led_1_cntrl *led_1_cntrl);
int ag_drv_xport_reg_led_1_cntrl_get(xport_reg_led_1_cntrl *led_1_cntrl);
int ag_drv_xport_reg_led_1_link_and_speed_encoding_sel_set(const xport_reg_led_1_link_and_speed_encoding_sel *led_1_link_and_speed_encoding_sel);
int ag_drv_xport_reg_led_1_link_and_speed_encoding_sel_get(xport_reg_led_1_link_and_speed_encoding_sel *led_1_link_and_speed_encoding_sel);
int ag_drv_xport_reg_led_1_link_and_speed_encoding_set(const xport_reg_led_1_link_and_speed_encoding *led_1_link_and_speed_encoding);
int ag_drv_xport_reg_led_1_link_and_speed_encoding_get(xport_reg_led_1_link_and_speed_encoding *led_1_link_and_speed_encoding);
int ag_drv_xport_reg_led_blink_rate_cntrl_set(uint16_t led_on_time, uint16_t led_off_time);
int ag_drv_xport_reg_led_blink_rate_cntrl_get(uint16_t *led_on_time, uint16_t *led_off_time);
int ag_drv_xport_reg_led_serial_ctrl_set(const xport_reg_led_serial_ctrl *led_serial_ctrl);
int ag_drv_xport_reg_led_serial_ctrl_get(xport_reg_led_serial_ctrl *led_serial_ctrl);
int ag_drv_xport_reg_refresh_period_cntrl_set(uint32_t refresh_period_cnt);
int ag_drv_xport_reg_refresh_period_cntrl_get(uint32_t *refresh_period_cnt);
int ag_drv_xport_reg_aggregate_led_cntrl_set(uint8_t lnk_pol_sel, uint8_t act_pol_sel, uint8_t act_sel, uint16_t port_en);
int ag_drv_xport_reg_aggregate_led_cntrl_get(uint8_t *lnk_pol_sel, uint8_t *act_pol_sel, uint8_t *act_sel, uint16_t *port_en);
int ag_drv_xport_reg_aggregate_led_blink_rate_cntrl_set(uint16_t led_on_time, uint16_t led_off_time);
int ag_drv_xport_reg_aggregate_led_blink_rate_cntrl_get(uint16_t *led_on_time, uint16_t *led_off_time);
int ag_drv_xport_reg_spare_cntrl_set(uint32_t spare_reg);
int ag_drv_xport_reg_spare_cntrl_get(uint32_t *spare_reg);
int ag_drv_xport_reg_xport_cntrl_1_set(uint8_t msbus_clk_sel, uint8_t wan_led0_sel, uint8_t timeout_rst_disable, uint8_t p0_mode);
int ag_drv_xport_reg_xport_cntrl_1_get(uint8_t *msbus_clk_sel, uint8_t *wan_led0_sel, uint8_t *timeout_rst_disable, uint8_t *p0_mode);
int ag_drv_xport_reg_crossbar_status_get(xport_reg_crossbar_status *crossbar_status);
int ag_drv_xport_reg_pon_ae_serdes_status_get(xport_reg_pon_ae_serdes_status *pon_ae_serdes_status);

#ifdef USE_BDMF_SHELL
bdmfmon_handle_t ag_drv_xport_reg_cli_init(bdmfmon_handle_t driver_dir);
#endif


#endif

