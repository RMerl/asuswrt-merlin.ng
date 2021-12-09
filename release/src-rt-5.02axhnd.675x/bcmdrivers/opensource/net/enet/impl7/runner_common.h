/*
   <:copyright-BRCM:2015:DUAL/GPL:standard
   
      Copyright (c) 2015 Broadcom 
      All Rights Reserved
   
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
 *  Created on: May/2018
 *      Author: ido@broadcom.com
 */

#include "port.h"
#include <rdpa_api.h>

int _rdpa_cpu_send_sysb(bdmf_sysb sysb, rdpa_cpu_tx_info_t *info);
int port_runner_port_uninit(enetx_port_t *self);
int port_runner_mtu_set(enetx_port_t *self, int mtu);
void port_runner_print_status(enetx_port_t *self);
void port_runner_port_stats_get(enetx_port_t *self, struct rtnl_link_stats64 *net_stats);
void port_runner_port_stats_clear(enetx_port_t *self);
int port_runner_mib_dump(enetx_port_t *self, int all);
int port_runner_mib_dump_us(enetx_port_t *self, void *e); // add by Andrew
char *port_runner_print_priv(enetx_port_t *self);
bdmf_object_handle create_rdpa_port(rdpa_if rdpaif, rdpa_emac emac, bdmf_object_handle owner, rdpa_if control_sid);
int port_runner_sw_init(enetx_port_t *self);
int port_runner_sw_uninit(enetx_port_t *self);
int port_runner_sw_port_id_on_sw(port_info_t *port_info, int *port_id, port_type_t *port_type);
int link_switch_to_rdpa_port(bdmf_object_handle port_obj);
int port_runner_link_change(enetx_port_t *self, int up);

