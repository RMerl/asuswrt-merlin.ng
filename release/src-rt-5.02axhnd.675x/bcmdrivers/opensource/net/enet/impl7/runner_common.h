/*
   <:copyright-BRCM:2015:DUAL/GPL:standard
   
      Copyright (c) 2015 Broadcom 
      All Rights Reserved
   
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

