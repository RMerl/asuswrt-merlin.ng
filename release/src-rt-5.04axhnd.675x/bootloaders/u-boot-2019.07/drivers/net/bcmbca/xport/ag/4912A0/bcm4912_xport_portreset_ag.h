// SPDX-License-Identifier: GPL-2.0+
/*
   Copyright (c) 2015 Broadcom
   All Rights Reserved

*/

#ifndef _BCM4912_XPORT_PORTRESET_AG_H_
#define _BCM4912_XPORT_PORTRESET_AG_H_

#include "access_macros.h"
#include "bcmtypes.h"

/**************************************************************************************************/
/* enable_xlmac_rx_disab:  - ...                                                                  */
/* enable_xlmac_tx_disab:  - ...                                                                  */
/* enable_xlmac_tx_discard:  - ...                                                                */
/* enable_xlmac_soft_reset:  - ...                                                                */
/* enable_mab_rx_port_init:  - ...                                                                */
/* enable_mab_tx_port_init:  - ...                                                                */
/* enable_mab_tx_credit_disab:  - ...                                                             */
/* enable_mab_tx_fifo_init:  - ...                                                                */
/* enable_port_is_under_reset:  - ...                                                             */
/* enable_xlmac_ep_discard:  - ...                                                                */
/**************************************************************************************************/
typedef struct
{
    uint8_t enable_xlmac_rx_disab;
    uint8_t enable_xlmac_tx_disab;
    uint8_t enable_xlmac_tx_discard;
    uint8_t enable_xlmac_soft_reset;
    uint8_t enable_mab_rx_port_init;
    uint8_t enable_mab_tx_port_init;
    uint8_t enable_mab_tx_credit_disab;
    uint8_t enable_mab_tx_fifo_init;
    uint8_t enable_port_is_under_reset;
    uint8_t enable_xlmac_ep_discard;
} xport_portreset_sig_en;



int ag_drv_xport_portreset_p0_ctrl_set(uint8_t xlmac_id, uint8_t port_sw_reset);
int ag_drv_xport_portreset_p0_ctrl_get(uint8_t xlmac_id, uint8_t *port_sw_reset);
int ag_drv_xport_portreset_p1_ctrl_set(uint8_t xlmac_id, uint8_t port_sw_reset);
int ag_drv_xport_portreset_p1_ctrl_get(uint8_t xlmac_id, uint8_t *port_sw_reset);
int ag_drv_xport_portreset_p2_ctrl_set(uint8_t xlmac_id, uint8_t port_sw_reset);
int ag_drv_xport_portreset_p2_ctrl_get(uint8_t xlmac_id, uint8_t *port_sw_reset);
int ag_drv_xport_portreset_p3_ctrl_set(uint8_t xlmac_id, uint8_t port_sw_reset);
int ag_drv_xport_portreset_p3_ctrl_get(uint8_t xlmac_id, uint8_t *port_sw_reset);
int ag_drv_xport_portreset_config_set(uint8_t xlmac_id, uint8_t link_down_rst_en, uint8_t enable_sm_run, uint16_t tick_timer_ndiv);
int ag_drv_xport_portreset_config_get(uint8_t xlmac_id, uint8_t *link_down_rst_en, uint8_t *enable_sm_run, uint16_t *tick_timer_ndiv);
int ag_drv_xport_portreset_p0_link_stat_debounce_cfg_set(uint8_t xlmac_id, uint8_t disable, uint16_t debounce_time);
int ag_drv_xport_portreset_p0_link_stat_debounce_cfg_get(uint8_t xlmac_id, uint8_t *disable, uint16_t *debounce_time);
int ag_drv_xport_portreset_p1_link_stat_debounce_cfg_set(uint8_t xlmac_id, uint8_t disable, uint16_t debounce_time);
int ag_drv_xport_portreset_p1_link_stat_debounce_cfg_get(uint8_t xlmac_id, uint8_t *disable, uint16_t *debounce_time);
int ag_drv_xport_portreset_p2_link_stat_debounce_cfg_set(uint8_t xlmac_id, uint8_t disable, uint16_t debounce_time);
int ag_drv_xport_portreset_p2_link_stat_debounce_cfg_get(uint8_t xlmac_id, uint8_t *disable, uint16_t *debounce_time);
int ag_drv_xport_portreset_p3_link_stat_debounce_cfg_set(uint8_t xlmac_id, uint8_t disable, uint16_t debounce_time);
int ag_drv_xport_portreset_p3_link_stat_debounce_cfg_get(uint8_t xlmac_id, uint8_t *disable, uint16_t *debounce_time);
int ag_drv_xport_portreset_p0_sig_en_set(uint8_t xlmac_id, const xport_portreset_sig_en *p0_sig_en);
int ag_drv_xport_portreset_p0_sig_en_get(uint8_t xlmac_id, xport_portreset_sig_en *p0_sig_en);
int ag_drv_xport_portreset_p0_sig_assert_times_0_set(uint8_t xlmac_id, uint16_t xlmac_rx_disab_assert_time, uint16_t xlmac_tx_disab_assert_time);
int ag_drv_xport_portreset_p0_sig_assert_times_0_get(uint8_t xlmac_id, uint16_t *xlmac_rx_disab_assert_time, uint16_t *xlmac_tx_disab_assert_time);
int ag_drv_xport_portreset_p0_sig_assert_times_1_set(uint8_t xlmac_id, uint16_t xlmac_txdiscard_assert_time, uint16_t xlmac_soft_reset_assert_time);
int ag_drv_xport_portreset_p0_sig_assert_times_1_get(uint8_t xlmac_id, uint16_t *xlmac_txdiscard_assert_time, uint16_t *xlmac_soft_reset_assert_time);
int ag_drv_xport_portreset_p0_sig_assert_times_2_set(uint8_t xlmac_id, uint16_t mab_rx_port_init_assert_time, uint16_t mab_tx_port_init_assert_time);
int ag_drv_xport_portreset_p0_sig_assert_times_2_get(uint8_t xlmac_id, uint16_t *mab_rx_port_init_assert_time, uint16_t *mab_tx_port_init_assert_time);
int ag_drv_xport_portreset_p0_sig_assert_times_3_set(uint8_t xlmac_id, uint16_t mab_tx_credit_disab_assert_time, uint16_t mab_tx_fifo_init_assert_time);
int ag_drv_xport_portreset_p0_sig_assert_times_3_get(uint8_t xlmac_id, uint16_t *mab_tx_credit_disab_assert_time, uint16_t *mab_tx_fifo_init_assert_time);
int ag_drv_xport_portreset_p0_sig_assert_times_4_set(uint8_t xlmac_id, uint16_t port_is_under_reset_assert_time);
int ag_drv_xport_portreset_p0_sig_assert_times_4_get(uint8_t xlmac_id, uint16_t *port_is_under_reset_assert_time);
int ag_drv_xport_portreset_p0_sig_deassert_times_0_set(uint8_t xlmac_id, uint16_t xlmac_rx_disab_deassert_time, uint16_t xlmac_tx_disab_deassert_time);
int ag_drv_xport_portreset_p0_sig_deassert_times_0_get(uint8_t xlmac_id, uint16_t *xlmac_rx_disab_deassert_time, uint16_t *xlmac_tx_disab_deassert_time);
int ag_drv_xport_portreset_p0_sig_deassert_times_1_set(uint8_t xlmac_id, uint16_t xlmac_txdiscard_deassert_time, uint16_t xlmac_soft_reset_deassert_time);
int ag_drv_xport_portreset_p0_sig_deassert_times_1_get(uint8_t xlmac_id, uint16_t *xlmac_txdiscard_deassert_time, uint16_t *xlmac_soft_reset_deassert_time);
int ag_drv_xport_portreset_p0_sig_deassert_times_2_set(uint8_t xlmac_id, uint16_t mab_rx_port_init_deassert_time, uint16_t mab_tx_port_init_deassert_time);
int ag_drv_xport_portreset_p0_sig_deassert_times_2_get(uint8_t xlmac_id, uint16_t *mab_rx_port_init_deassert_time, uint16_t *mab_tx_port_init_deassert_time);
int ag_drv_xport_portreset_p0_sig_deassert_times_3_set(uint8_t xlmac_id, uint16_t mab_tx_credit_disab_deassert_time, uint16_t mab_tx_fifo_init_deassert_time);
int ag_drv_xport_portreset_p0_sig_deassert_times_3_get(uint8_t xlmac_id, uint16_t *mab_tx_credit_disab_deassert_time, uint16_t *mab_tx_fifo_init_deassert_time);
int ag_drv_xport_portreset_p0_sig_deassert_times_4_set(uint8_t xlmac_id, uint16_t port_is_under_reset_deassert_time);
int ag_drv_xport_portreset_p0_sig_deassert_times_4_get(uint8_t xlmac_id, uint16_t *port_is_under_reset_deassert_time);
int ag_drv_xport_portreset_p1_sig_en_set(uint8_t xlmac_id, const xport_portreset_sig_en *p1_sig_en);
int ag_drv_xport_portreset_p1_sig_en_get(uint8_t xlmac_id, xport_portreset_sig_en *p1_sig_en);
int ag_drv_xport_portreset_p1_sig_assert_times_0_set(uint8_t xlmac_id, uint16_t xlmac_rx_disab_assert_time, uint16_t xlmac_tx_disab_assert_time);
int ag_drv_xport_portreset_p1_sig_assert_times_0_get(uint8_t xlmac_id, uint16_t *xlmac_rx_disab_assert_time, uint16_t *xlmac_tx_disab_assert_time);
int ag_drv_xport_portreset_p1_sig_assert_times_1_set(uint8_t xlmac_id, uint16_t xlmac_txdiscard_assert_time, uint16_t xlmac_soft_reset_assert_time);
int ag_drv_xport_portreset_p1_sig_assert_times_1_get(uint8_t xlmac_id, uint16_t *xlmac_txdiscard_assert_time, uint16_t *xlmac_soft_reset_assert_time);
int ag_drv_xport_portreset_p1_sig_assert_times_2_set(uint8_t xlmac_id, uint16_t mab_rx_port_init_assert_time, uint16_t mab_tx_port_init_assert_time);
int ag_drv_xport_portreset_p1_sig_assert_times_2_get(uint8_t xlmac_id, uint16_t *mab_rx_port_init_assert_time, uint16_t *mab_tx_port_init_assert_time);
int ag_drv_xport_portreset_p1_sig_assert_times_3_set(uint8_t xlmac_id, uint16_t mab_tx_credit_disab_assert_time, uint16_t mab_tx_fifo_init_assert_time);
int ag_drv_xport_portreset_p1_sig_assert_times_3_get(uint8_t xlmac_id, uint16_t *mab_tx_credit_disab_assert_time, uint16_t *mab_tx_fifo_init_assert_time);
int ag_drv_xport_portreset_p1_sig_assert_times_4_set(uint8_t xlmac_id, uint16_t port_is_under_reset_assert_time);
int ag_drv_xport_portreset_p1_sig_assert_times_4_get(uint8_t xlmac_id, uint16_t *port_is_under_reset_assert_time);
int ag_drv_xport_portreset_p1_sig_deassert_times_0_set(uint8_t xlmac_id, uint16_t xlmac_rx_disab_deassert_time, uint16_t xlmac_tx_disab_deassert_time);
int ag_drv_xport_portreset_p1_sig_deassert_times_0_get(uint8_t xlmac_id, uint16_t *xlmac_rx_disab_deassert_time, uint16_t *xlmac_tx_disab_deassert_time);
int ag_drv_xport_portreset_p1_sig_deassert_times_1_set(uint8_t xlmac_id, uint16_t xlmac_txdiscard_deassert_time, uint16_t xlmac_soft_reset_deassert_time);
int ag_drv_xport_portreset_p1_sig_deassert_times_1_get(uint8_t xlmac_id, uint16_t *xlmac_txdiscard_deassert_time, uint16_t *xlmac_soft_reset_deassert_time);
int ag_drv_xport_portreset_p1_sig_deassert_times_2_set(uint8_t xlmac_id, uint16_t mab_rx_port_init_deassert_time, uint16_t mab_tx_port_init_deassert_time);
int ag_drv_xport_portreset_p1_sig_deassert_times_2_get(uint8_t xlmac_id, uint16_t *mab_rx_port_init_deassert_time, uint16_t *mab_tx_port_init_deassert_time);
int ag_drv_xport_portreset_p1_sig_deassert_times_3_set(uint8_t xlmac_id, uint16_t mab_tx_credit_disab_deassert_time, uint16_t mab_tx_fifo_init_deassert_time);
int ag_drv_xport_portreset_p1_sig_deassert_times_3_get(uint8_t xlmac_id, uint16_t *mab_tx_credit_disab_deassert_time, uint16_t *mab_tx_fifo_init_deassert_time);
int ag_drv_xport_portreset_p1_sig_deassert_times_4_set(uint8_t xlmac_id, uint16_t port_is_under_reset_deassert_time);
int ag_drv_xport_portreset_p1_sig_deassert_times_4_get(uint8_t xlmac_id, uint16_t *port_is_under_reset_deassert_time);
int ag_drv_xport_portreset_p2_sig_en_set(uint8_t xlmac_id, const xport_portreset_sig_en *p2_sig_en);
int ag_drv_xport_portreset_p2_sig_en_get(uint8_t xlmac_id, xport_portreset_sig_en *p2_sig_en);
int ag_drv_xport_portreset_p2_sig_assert_times_0_set(uint8_t xlmac_id, uint16_t xlmac_rx_disab_assert_time, uint16_t xlmac_tx_disab_assert_time);
int ag_drv_xport_portreset_p2_sig_assert_times_0_get(uint8_t xlmac_id, uint16_t *xlmac_rx_disab_assert_time, uint16_t *xlmac_tx_disab_assert_time);
int ag_drv_xport_portreset_p2_sig_assert_times_1_set(uint8_t xlmac_id, uint16_t xlmac_txdiscard_assert_time, uint16_t xlmac_soft_reset_assert_time);
int ag_drv_xport_portreset_p2_sig_assert_times_1_get(uint8_t xlmac_id, uint16_t *xlmac_txdiscard_assert_time, uint16_t *xlmac_soft_reset_assert_time);
int ag_drv_xport_portreset_p2_sig_assert_times_2_set(uint8_t xlmac_id, uint16_t mab_rx_port_init_assert_time, uint16_t mab_tx_port_init_assert_time);
int ag_drv_xport_portreset_p2_sig_assert_times_2_get(uint8_t xlmac_id, uint16_t *mab_rx_port_init_assert_time, uint16_t *mab_tx_port_init_assert_time);
int ag_drv_xport_portreset_p2_sig_assert_times_3_set(uint8_t xlmac_id, uint16_t mab_tx_credit_disab_assert_time, uint16_t mab_tx_fifo_init_assert_time);
int ag_drv_xport_portreset_p2_sig_assert_times_3_get(uint8_t xlmac_id, uint16_t *mab_tx_credit_disab_assert_time, uint16_t *mab_tx_fifo_init_assert_time);
int ag_drv_xport_portreset_p2_sig_assert_times_4_set(uint8_t xlmac_id, uint16_t port_is_under_reset_assert_time);
int ag_drv_xport_portreset_p2_sig_assert_times_4_get(uint8_t xlmac_id, uint16_t *port_is_under_reset_assert_time);
int ag_drv_xport_portreset_p2_sig_deassert_times_0_set(uint8_t xlmac_id, uint16_t xlmac_rx_disab_deassert_time, uint16_t xlmac_tx_disab_deassert_time);
int ag_drv_xport_portreset_p2_sig_deassert_times_0_get(uint8_t xlmac_id, uint16_t *xlmac_rx_disab_deassert_time, uint16_t *xlmac_tx_disab_deassert_time);
int ag_drv_xport_portreset_p2_sig_deassert_times_1_set(uint8_t xlmac_id, uint16_t xlmac_txdiscard_deassert_time, uint16_t xlmac_soft_reset_deassert_time);
int ag_drv_xport_portreset_p2_sig_deassert_times_1_get(uint8_t xlmac_id, uint16_t *xlmac_txdiscard_deassert_time, uint16_t *xlmac_soft_reset_deassert_time);
int ag_drv_xport_portreset_p2_sig_deassert_times_2_set(uint8_t xlmac_id, uint16_t mab_rx_port_init_deassert_time, uint16_t mab_tx_port_init_deassert_time);
int ag_drv_xport_portreset_p2_sig_deassert_times_2_get(uint8_t xlmac_id, uint16_t *mab_rx_port_init_deassert_time, uint16_t *mab_tx_port_init_deassert_time);
int ag_drv_xport_portreset_p2_sig_deassert_times_3_set(uint8_t xlmac_id, uint16_t mab_tx_credit_disab_deassert_time, uint16_t mab_tx_fifo_init_deassert_time);
int ag_drv_xport_portreset_p2_sig_deassert_times_3_get(uint8_t xlmac_id, uint16_t *mab_tx_credit_disab_deassert_time, uint16_t *mab_tx_fifo_init_deassert_time);
int ag_drv_xport_portreset_p2_sig_deassert_times_4_set(uint8_t xlmac_id, uint16_t port_is_under_reset_deassert_time);
int ag_drv_xport_portreset_p2_sig_deassert_times_4_get(uint8_t xlmac_id, uint16_t *port_is_under_reset_deassert_time);
int ag_drv_xport_portreset_p3_sig_en_set(uint8_t xlmac_id, const xport_portreset_sig_en *p3_sig_en);
int ag_drv_xport_portreset_p3_sig_en_get(uint8_t xlmac_id, xport_portreset_sig_en *p3_sig_en);
int ag_drv_xport_portreset_p3_sig_assert_times_0_set(uint8_t xlmac_id, uint16_t xlmac_rx_disab_assert_time, uint16_t xlmac_tx_disab_assert_time);
int ag_drv_xport_portreset_p3_sig_assert_times_0_get(uint8_t xlmac_id, uint16_t *xlmac_rx_disab_assert_time, uint16_t *xlmac_tx_disab_assert_time);
int ag_drv_xport_portreset_p3_sig_assert_times_1_set(uint8_t xlmac_id, uint16_t xlmac_txdiscard_assert_time, uint16_t xlmac_soft_reset_assert_time);
int ag_drv_xport_portreset_p3_sig_assert_times_1_get(uint8_t xlmac_id, uint16_t *xlmac_txdiscard_assert_time, uint16_t *xlmac_soft_reset_assert_time);
int ag_drv_xport_portreset_p3_sig_assert_times_2_set(uint8_t xlmac_id, uint16_t mab_rx_port_init_assert_time, uint16_t mab_tx_port_init_assert_time);
int ag_drv_xport_portreset_p3_sig_assert_times_2_get(uint8_t xlmac_id, uint16_t *mab_rx_port_init_assert_time, uint16_t *mab_tx_port_init_assert_time);
int ag_drv_xport_portreset_p3_sig_assert_times_3_set(uint8_t xlmac_id, uint16_t mab_tx_credit_disab_assert_time, uint16_t mab_tx_fifo_init_assert_time);
int ag_drv_xport_portreset_p3_sig_assert_times_3_get(uint8_t xlmac_id, uint16_t *mab_tx_credit_disab_assert_time, uint16_t *mab_tx_fifo_init_assert_time);
int ag_drv_xport_portreset_p3_sig_assert_times_4_set(uint8_t xlmac_id, uint16_t port_is_under_reset_assert_time);
int ag_drv_xport_portreset_p3_sig_assert_times_4_get(uint8_t xlmac_id, uint16_t *port_is_under_reset_assert_time);
int ag_drv_xport_portreset_p3_sig_deassert_times_0_set(uint8_t xlmac_id, uint16_t xlmac_rx_disab_deassert_time, uint16_t xlmac_tx_disab_deassert_time);
int ag_drv_xport_portreset_p3_sig_deassert_times_0_get(uint8_t xlmac_id, uint16_t *xlmac_rx_disab_deassert_time, uint16_t *xlmac_tx_disab_deassert_time);
int ag_drv_xport_portreset_p3_sig_deassert_times_1_set(uint8_t xlmac_id, uint16_t xlmac_txdiscard_deassert_time, uint16_t xlmac_soft_reset_deassert_time);
int ag_drv_xport_portreset_p3_sig_deassert_times_1_get(uint8_t xlmac_id, uint16_t *xlmac_txdiscard_deassert_time, uint16_t *xlmac_soft_reset_deassert_time);
int ag_drv_xport_portreset_p3_sig_deassert_times_2_set(uint8_t xlmac_id, uint16_t mab_rx_port_init_deassert_time, uint16_t mab_tx_port_init_deassert_time);
int ag_drv_xport_portreset_p3_sig_deassert_times_2_get(uint8_t xlmac_id, uint16_t *mab_rx_port_init_deassert_time, uint16_t *mab_tx_port_init_deassert_time);
int ag_drv_xport_portreset_p3_sig_deassert_times_3_set(uint8_t xlmac_id, uint16_t mab_tx_credit_disab_deassert_time, uint16_t mab_tx_fifo_init_deassert_time);
int ag_drv_xport_portreset_p3_sig_deassert_times_3_get(uint8_t xlmac_id, uint16_t *mab_tx_credit_disab_deassert_time, uint16_t *mab_tx_fifo_init_deassert_time);
int ag_drv_xport_portreset_p3_sig_deassert_times_4_set(uint8_t xlmac_id, uint16_t port_is_under_reset_deassert_time);
int ag_drv_xport_portreset_p3_sig_deassert_times_4_get(uint8_t xlmac_id, uint16_t *port_is_under_reset_deassert_time);
int ag_drv_xport_portreset_debug_get(uint8_t xlmac_id, uint8_t *p3_sm_state, uint8_t *p2_sm_state, uint8_t *p1_sm_state, uint8_t *p0_sm_state);

#ifdef USE_BDMF_SHELL
bdmfmon_handle_t ag_drv_xport_portreset_cli_init(bdmfmon_handle_t driver_dir);
#endif


#endif

