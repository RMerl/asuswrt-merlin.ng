/*
   Copyright (c) 2017 Broadcom Corporation
   All Rights Reserved

    <:label-BRCM:2017:DUAL/GPL:standard

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
/*
 * xport_drv.h
 *
 */

#ifndef SHARED_OPENSOURCE_DRV_XPORT_XPORT_DRV_H_
#define SHARED_OPENSOURCE_DRV_XPORT_XPORT_DRV_H_

//includes
#include "xport_defs.h"
#include "mac_drv.h"

//APIs
int xport_init_driver(xport_xlmac_port_info_s *init_params);
int xport_get_port_configuration(uint32_t portid, xport_port_cfg_s *port_conf);
int xport_set_port_configuration(uint32_t portid, xport_port_cfg_s *port_conf);
int xport_port_mtu_set(uint32_t portid, uint16_t port_mtu);
int xport_port_mtu_get(uint32_t portid, uint16_t *port_mtu);
int xport_get_pause_configuration(uint32_t portid, xport_flow_ctrl_cfg_s *flow_ctrl);
int xport_set_pause_configuration(uint32_t portid, xport_flow_ctrl_cfg_s *flow_ctrl);
int xport_get_pfc_configuration(uint32_t portid, xport_flow_ctrl_cfg_s *flow_ctrl);
int xport_set_pfc_configuration(uint32_t portid, xport_flow_ctrl_cfg_s *flow_ctrl);
int xport_set_port_rxtx_enable(uint32_t portid, uint8_t rx_en, uint8_t tx_en);
int xport_get_port_rxtx_enable(uint32_t portid, uint8_t *rx_en, uint8_t *tx_en);
int xport_get_port_status(uint32_t port, xport_port_status_s *port_status);
int xport_get_port_link_status(uint32_t port, uint8_t *link_up);
int xport_port_eee_set(uint32_t portid, uint8_t enable);
int xport_reset_phy_cfg(uint32_t port, xport_port_phycfg_s *phycfg);
int xport_rgmii_ate_start(uint32_t port, uint32_t num_of_packets, uint8_t pkt_gen_en);
int xport_get_brcm_phy_status(uint16_t phyid,xport_port_status_s *port_status);
int xport_get_next_ts_from_fifo(uint8_t port_id, uint16_t *sequence_id, uint32_t *time_stamp);
int xport_handle_link_up(xport_xlmac_port_info_s *p_info);
int xport_handle_link_dn(xport_xlmac_port_info_s *p_info);

char *xport_rate_to_str(XPORT_PORT_RATE rate);
int xport_str_to_rate(char *str);
#endif /* SHARED_OPENSOURCE_DRV_XPORT_XPORT_DRV_H_ */
