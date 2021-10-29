/*
   Copyright (c) 2015 Broadcom Corporation
   All Rights Reserved

    <:label-BRCM:2015:DUAL/GPL:standard
    
    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License, version 2, as published by
    the Free Software Foundation (the "GPL").
    
    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.
    
    
    A copy of the GPL is available at http://www.broadcom.com/licenses/GPLv2.php, or by
    writing to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
    
:>
*/
/*
 * lport_drv.h
 *
 *  Created on: April 2015
 *      Author: yonatani
 */

#ifndef SHARED_OPENSOURCE_DRV_LPORT_LPORT_DRV_H_
#define SHARED_OPENSOURCE_DRV_LPORT_LPORT_DRV_H_

//includes
#include "lport_defs.h"




//APIs
int lport_init_driver(lport_init_s *init_params);
int lport_reinit_driver(lport_init_s *init_params, int log);
int lport_get_port_configuration(uint32_t portid, lport_port_cfg_s *port_conf);
int lport_set_port_configuration(uint32_t portid, lport_port_cfg_s *port_conf);
int lport_port_mtu_set(uint32_t portid, uint16_t port_mtu);
int lport_port_mtu_get(uint32_t portid, uint16_t *port_mtu);
LPORT_PORT_RATE lport_speed_get(uint8_t portid);
int lport_get_pause_configuration(uint32_t portid, lport_flow_ctrl_cfg_s *flow_ctrl);
int lport_set_pause_configuration(uint32_t portid, lport_flow_ctrl_cfg_s *flow_ctrl);
int lport_set_port_rxtx_enable(uint32_t portid, uint8_t rx_en, uint8_t tx_en);
int lport_get_port_rxtx_enable(uint32_t portid, uint8_t *rx_en, uint8_t *tx_en);
int lport_port_credits_restart(uint32_t portid);
int lport_get_rgmii_cfg(uint32_t portid, lport_rgmii_cfg_s *rgmii_cfg);
int lport_set_rgmii_cfg(uint32_t portid, lport_rgmii_cfg_s *rgmii_cfg);
int lport_get_port_status(uint32_t port, lport_port_status_s *port_status);
int lport_get_port_link_status(uint32_t port, uint8_t *link_up);
int lport_port_eee_set(uint32_t portid, uint8_t enable);
int lport_reset_phy_cfg(uint32_t port, lport_port_phycfg_s *phycfg);
int lport_rgmii_ate_config(uint32_t port, lport_rgmii_ate_s *rgmii_ate_conf);
int lport_rgmii_ate_start(uint32_t port, uint32_t num_of_packets, uint8_t pkt_gen_en);
int lport_get_brcm_phy_status(uint16_t phyid,lport_port_status_s *port_status);
int lport_get_next_ts_from_fifo(uint8_t port_id, uint16_t *sequence_id, uint32_t *time_stamp);

char *lport_rate_to_str(LPORT_PORT_RATE rate);
int lport_str_to_rate(char *str);
#endif /* SHARED_OPENSOURCE_DRV_LPORT_LPORT_DRV_H_ */
