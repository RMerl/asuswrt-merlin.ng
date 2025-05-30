// SPDX-License-Identifier: GPL-2.0+
/*
   Copyright (c) 2017 Broadcom Corporation
   All Rights Reserved

*/
/*
 * xport_drv.h
 *
 */

#ifndef SHARED_OPENSOURCE_DRV_XPORT_XPORT_DRV_H_
#define SHARED_OPENSOURCE_DRV_XPORT_XPORT_DRV_H_

//includes
#include "xport_defs.h"
#include "mac_drv.h"
#include "wol.h"

//APIs
int xport_init_driver(xport_xlmac_port_info_s *init_params);
int xport_get_port_configuration(uint32_t portid, xport_port_cfg_s *port_conf);
int xport_set_port_configuration(uint32_t portid, xport_port_cfg_s *port_conf);
int xport_mtu_set(uint32_t portid, uint16_t port_mtu);
int xport_mtu_get(uint32_t portid, uint16_t *port_mtu);
int xport_get_pause_configuration(uint32_t portid, xport_flow_ctrl_cfg_s *flow_ctrl);
int xport_set_pause_configuration(uint32_t portid, xport_flow_ctrl_cfg_s *flow_ctrl);
int xport_get_pfc_configuration(uint32_t portid, xport_flow_ctrl_cfg_s *flow_ctrl);
int xport_set_pfc_configuration(uint32_t portid, xport_flow_ctrl_cfg_s *flow_ctrl);
int xport_set_port_rxtx_enable(uint32_t portid, uint8_t rx_en, uint8_t tx_en);
int xport_get_port_rxtx_enable(uint32_t portid, uint8_t *rx_en, uint8_t *tx_en);
int xport_get_port_status(uint32_t portid, xport_port_status_s *port_status);
int xport_get_port_link_status(uint32_t portid, uint8_t *link_up);
int xport_eee_set(uint32_t portid, uint8_t enable);
int xport_wol_enable(uint32_t portid, wol_params_t *wol_params);
int xport_reset_phy_cfg(uint32_t portid, xport_port_phycfg_s *phycfg);
int xport_rgmii_ate_start(uint32_t portid, uint32_t num_of_packets, uint8_t pkt_gen_en);
int xport_get_brcm_phy_status(uint16_t phyid,xport_port_status_s *port_status);
int xport_get_next_ts_from_fifo(uint8_t port_id, uint16_t *sequence_id, uint32_t *time_stamp);
int xport_handle_link_up(xport_xlmac_port_info_s *p_info);
int xport_handle_link_dn(xport_xlmac_port_info_s *p_info);

char *xport_rate_to_str(XPORT_PORT_RATE rate);
int xport_str_to_rate(char *str);
#endif /* SHARED_OPENSOURCE_DRV_XPORT_XPORT_DRV_H_ */
