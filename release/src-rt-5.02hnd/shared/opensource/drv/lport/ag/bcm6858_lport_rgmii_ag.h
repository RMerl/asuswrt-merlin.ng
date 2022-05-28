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

#ifndef _BCM6858_LPORT_RGMII_AG_H_
#define _BCM6858_LPORT_RGMII_AG_H_

#include "access_macros.h"
#include "bcmtypes.h"
typedef struct
{
    uint8_t col_crs_mask;
    uint8_t rx_err_mask;
    uint8_t lpi_count;
    uint8_t tx_clk_stop_en;
    uint8_t tx_pause_en;
    uint8_t rx_pause_en;
    uint8_t rvmii_ref_sel;
    uint8_t port_mode;
    uint8_t id_mode_dis;
    uint8_t rgmii_mode_en;
} lport_rgmii_cntrl;

typedef struct
{
    uint8_t reset;
    uint8_t dly_override;
    uint8_t dly_sel;
    uint8_t bypass;
    uint8_t iddq;
    uint8_t drng;
    uint8_t ctri;
} lport_rgmii_rx_clock_delay_cntrl;

typedef struct
{
    uint8_t ate_en;
    uint8_t pkt_count_rst;
    uint8_t good_count;
    uint16_t expected_data_1;
    uint16_t expected_data_0;
} lport_rgmii_ate_rx_cntrl_exp_data;

typedef struct
{
    uint8_t pkt_ipg;
    uint16_t payload_length;
    uint8_t pkt_cnt;
    uint8_t pkt_gen_en;
    uint8_t start_stop;
    uint8_t start_stop_ovrd;
} lport_rgmii_ate_tx_cntrl;

typedef struct
{
    uint8_t txd3_del_ovrd_en;
    uint8_t txd3_del_sel;
    uint8_t txd2_del_ovrd_en;
    uint8_t txd2_del_sel;
    uint8_t txd1_del_ovrd_en;
    uint8_t txd1_del_sel;
    uint8_t txd0_del_ovrd_en;
    uint8_t txd0_del_sel;
} lport_rgmii_tx_delay_cntrl_0;

typedef struct
{
    uint8_t txclk_id_del_ovrd_en;
    uint8_t txclk_id_del_sel;
    uint8_t txclk_del_ovrd_en;
    uint8_t txclk_del_sel;
    uint8_t txctl_del_ovrd_en;
    uint8_t txctl_del_sel;
} lport_rgmii_tx_delay_cntrl_1;

typedef struct
{
    uint8_t rxd3_del_ovrd_en;
    uint8_t rxd3_del_sel;
    uint8_t rxd2_del_ovrd_en;
    uint8_t rxd2_del_sel;
    uint8_t rxd1_del_ovrd_en;
    uint8_t rxd1_del_sel;
    uint8_t rxd0_del_ovrd_en;
    uint8_t rxd0_del_sel;
} lport_rgmii_rx_delay_cntrl_0;

typedef struct
{
    uint8_t rxd7_del_ovrd_en;
    uint8_t rxd7_del_sel;
    uint8_t rxd6_del_ovrd_en;
    uint8_t rxd6_del_sel;
    uint8_t rxd5_del_ovrd_en;
    uint8_t rxd5_del_sel;
    uint8_t rxd4_del_ovrd_en;
    uint8_t rxd4_del_sel;
} lport_rgmii_rx_delay_cntrl_1;

typedef struct
{
    uint8_t rxclk_del_ovrd_en;
    uint8_t rxclk_del_sel;
    uint8_t rxctl_neg_del_ovrd_en;
    uint8_t rxctl_neg_del_sel;
    uint8_t rxctl_pos_del_ovrd_en;
    uint8_t rxctl_pos_del_sel;
} lport_rgmii_rx_delay_cntrl_2;

int ag_drv_lport_rgmii_cntrl_set(uint8_t rgmii_id, const lport_rgmii_cntrl *cntrl);
int ag_drv_lport_rgmii_cntrl_get(uint8_t rgmii_id, lport_rgmii_cntrl *cntrl);
int ag_drv_lport_rgmii_ib_status_set(uint8_t rgmii_id, uint8_t ib_status_ovrd, uint8_t link_decode, uint8_t duplex_decode, uint8_t speed_decode);
int ag_drv_lport_rgmii_ib_status_get(uint8_t rgmii_id, uint8_t *ib_status_ovrd, uint8_t *link_decode, uint8_t *duplex_decode, uint8_t *speed_decode);
int ag_drv_lport_rgmii_rx_clock_delay_cntrl_set(uint8_t rgmii_id, const lport_rgmii_rx_clock_delay_cntrl *rx_clock_delay_cntrl);
int ag_drv_lport_rgmii_rx_clock_delay_cntrl_get(uint8_t rgmii_id, lport_rgmii_rx_clock_delay_cntrl *rx_clock_delay_cntrl);
int ag_drv_lport_rgmii_ate_rx_cntrl_exp_data_set(uint8_t rgmii_id, const lport_rgmii_ate_rx_cntrl_exp_data *ate_rx_cntrl_exp_data);
int ag_drv_lport_rgmii_ate_rx_cntrl_exp_data_get(uint8_t rgmii_id, lport_rgmii_ate_rx_cntrl_exp_data *ate_rx_cntrl_exp_data);
int ag_drv_lport_rgmii_ate_rx_exp_data_1_set(uint8_t rgmii_id, uint16_t expected_data_3, uint16_t expected_data_2);
int ag_drv_lport_rgmii_ate_rx_exp_data_1_get(uint8_t rgmii_id, uint16_t *expected_data_3, uint16_t *expected_data_2);
int ag_drv_lport_rgmii_ate_rx_status_0_get(uint8_t rgmii_id, uint8_t *rx_ok, uint16_t *received_data_1, uint16_t *received_data_0);
int ag_drv_lport_rgmii_ate_rx_status_1_get(uint8_t rgmii_id, uint16_t *received_data_3, uint16_t *received_data_2);
int ag_drv_lport_rgmii_ate_tx_cntrl_set(uint8_t rgmii_id, const lport_rgmii_ate_tx_cntrl *ate_tx_cntrl);
int ag_drv_lport_rgmii_ate_tx_cntrl_get(uint8_t rgmii_id, lport_rgmii_ate_tx_cntrl *ate_tx_cntrl);
int ag_drv_lport_rgmii_ate_tx_data_0_set(uint8_t rgmii_id, uint16_t tx_data_1, uint16_t tx_data_0);
int ag_drv_lport_rgmii_ate_tx_data_0_get(uint8_t rgmii_id, uint16_t *tx_data_1, uint16_t *tx_data_0);
int ag_drv_lport_rgmii_ate_tx_data_1_set(uint8_t rgmii_id, uint16_t tx_data_3, uint16_t tx_data_2);
int ag_drv_lport_rgmii_ate_tx_data_1_get(uint8_t rgmii_id, uint16_t *tx_data_3, uint16_t *tx_data_2);
int ag_drv_lport_rgmii_ate_tx_data_2_set(uint8_t rgmii_id, uint16_t ether_type, uint8_t tx_data_5, uint8_t tx_data_4);
int ag_drv_lport_rgmii_ate_tx_data_2_get(uint8_t rgmii_id, uint16_t *ether_type, uint8_t *tx_data_5, uint8_t *tx_data_4);
int ag_drv_lport_rgmii_tx_delay_cntrl_0_set(uint8_t rgmii_id, const lport_rgmii_tx_delay_cntrl_0 *tx_delay_cntrl_0);
int ag_drv_lport_rgmii_tx_delay_cntrl_0_get(uint8_t rgmii_id, lport_rgmii_tx_delay_cntrl_0 *tx_delay_cntrl_0);
int ag_drv_lport_rgmii_tx_delay_cntrl_1_set(uint8_t rgmii_id, const lport_rgmii_tx_delay_cntrl_1 *tx_delay_cntrl_1);
int ag_drv_lport_rgmii_tx_delay_cntrl_1_get(uint8_t rgmii_id, lport_rgmii_tx_delay_cntrl_1 *tx_delay_cntrl_1);
int ag_drv_lport_rgmii_rx_delay_cntrl_0_set(uint8_t rgmii_id, const lport_rgmii_rx_delay_cntrl_0 *rx_delay_cntrl_0);
int ag_drv_lport_rgmii_rx_delay_cntrl_0_get(uint8_t rgmii_id, lport_rgmii_rx_delay_cntrl_0 *rx_delay_cntrl_0);
int ag_drv_lport_rgmii_rx_delay_cntrl_1_set(uint8_t rgmii_id, const lport_rgmii_rx_delay_cntrl_1 *rx_delay_cntrl_1);
int ag_drv_lport_rgmii_rx_delay_cntrl_1_get(uint8_t rgmii_id, lport_rgmii_rx_delay_cntrl_1 *rx_delay_cntrl_1);
int ag_drv_lport_rgmii_rx_delay_cntrl_2_set(uint8_t rgmii_id, const lport_rgmii_rx_delay_cntrl_2 *rx_delay_cntrl_2);
int ag_drv_lport_rgmii_rx_delay_cntrl_2_get(uint8_t rgmii_id, lport_rgmii_rx_delay_cntrl_2 *rx_delay_cntrl_2);

#ifdef USE_BDMF_SHELL
bdmfmon_handle_t ag_drv_lport_rgmii_cli_init(bdmfmon_handle_t driver_dir);
#endif


#endif

