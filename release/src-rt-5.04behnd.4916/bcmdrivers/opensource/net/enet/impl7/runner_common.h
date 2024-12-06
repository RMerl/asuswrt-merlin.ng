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
