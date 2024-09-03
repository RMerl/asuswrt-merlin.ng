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

int port_runner_port_post_init(enetx_port_t *self);
rdpa_port_type port_runner_port_type_mapping(port_type_t port_type);
int _rdpa_cpu_send_sysb(bdmf_sysb sysb, rdpa_cpu_tx_info_t *info);
int port_runner_port_uninit(enetx_port_t *self);
int port_runner_mtu_set(enetx_port_t *self, int mtu);
void port_runner_print_status(enetx_port_t *self);
void port_runner_port_stats_get(enetx_port_t *self, struct rtnl_link_stats64 *net_stats);
void port_runner_port_stats_clear(enetx_port_t *self);
int port_runner_wan_role_set(enetx_port_t *self, port_netdev_role_t role);
int port_runner_mib_dump(enetx_port_t *self, int all);
int port_runner_mib_dump_us(enetx_port_t *self, void *e); // add by Andrew
char *port_runner_print_priv(enetx_port_t *self);
bdmf_object_handle create_rdpa_port(enetx_port_t *self, net_device_handle_t handle, bdmf_object_handle owner,
    int create_egress_tm, int enable_set, int create_ingress_filters);
int port_runner_sw_init(enetx_port_t *self);
int port_runner_sw_uninit(enetx_port_t *self);
int port_runner_sw_port_id_on_sw(port_info_t *port_info, int *port_id, port_type_t *port_type);
int port_runner_sw_config_trunk(enetx_port_t *sw, enetx_port_t *port, int grp_no, int add);
int port_runner_link_change(enetx_port_t *self, int up);
typedef enum
{
    RDPA_FILTERS_GROUP_LAN = 0,
    RDPA_FILTERS_GROUP_WAN,
    RDPA_FILTERS_GROUP_WAN_GBE
} rdpa_filters_group;
int runner_default_filter_init(bdmf_object_handle port_obj, rdpa_filters_group filters_group);

#if defined(CONFIG_NET_SWITCHDEV) && (LINUX_VERSION_CODE < KERNEL_VERSION(5,1,0))
int runner_port_attr_get(struct net_device *dev, struct switchdev_attr *attr);
int runner_port_attr_set(struct net_device *dev, const struct switchdev_attr *attr, struct switchdev_trans *trans);
#endif
int create_rdpa_egress_tm(bdmf_object_handle port_obj);
