/*
   <:copyright-BRCM:2023:DUAL/GPL:standard
   
      Copyright (c) 2023 Broadcom 
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

#include "enet.h"
#include "fm_nft_priv.h"
#include <rdpa_api.h>

extern int port_by_netdev(struct net_device *dev, enetx_port_t **match);

bdmf_object_handle get_bdmf_object_handle_from_dev(struct net_device *root_dev)
{
    bcm_netdev_priv_info_get_cb_fn_t priv_info_get = bcm_netdev_ext_field_get(root_dev, bcm_netdev_cb_fn);
    bcm_netdev_priv_info_out_t info_out = {};

    if (!priv_info_get || priv_info_get(root_dev, BCM_NETDEV_TO_RDPA_PORT_OBJ, &info_out))
        return NULL;

    return info_out.bcm_netdev_to_rdpa_port_obj.rdpa_port_obj;
}

int nft_fm_rnr_l3_key_get(nft_fm_tc_info_t *info, rdpa_ip_flow_key_t *key)
{
    nft_fw_tuple_t *tuple = &info->tuple_key;
    struct net_device *dev = info->fm_nft_ctx->dev;
    enetx_port_t *enetx_port;
    bcm_netdev_priv_info_out_t netdev_priv;
    int rc = port_by_netdev(dev, &enetx_port);

    RETURN_ON_ERR(rc,-EINVAL,dev,"cannot retrieve enetx_port object from dev %i\n",dev->ifindex);

    memset(key, 0, sizeof(*key));

    key->dir = PORT_ROLE_IS_WAN(enetx_port) ? rdpa_dir_ds : rdpa_dir_us; 

    enet_priv_info_get(dev, BCM_NETDEV_TO_RDPA_PORT_OBJ, &netdev_priv);

    key->port_ingress_obj = netdev_priv.bcm_netdev_to_rdpa_port_obj.rdpa_port_obj;

    RETURN_ON_ERR(!key->port_ingress_obj,-1,dev,"enet_priv_info_get failed, cannot retrieve rdpa port object handle\n");

    fm_nft_printk(LOGLEVEL_DEBUG, dev, "%s: dir:%s\n", __FUNCTION__, key->dir == rdpa_dir_us ? "us" : "ds");

    key->prot = tuple->ip_proto;
    key->tcp_pure_ack = 0;

    if (tuple->addr_type == FLOW_DISSECTOR_KEY_IPV4_ADDRS)
    {
        key->src_ip.addr.ipv4 = htonl(tuple->ip.src_v4);
        key->dst_ip.addr.ipv4 = htonl(tuple->ip.dst_v4);
        key->src_ip.family = bdmf_ip_family_ipv4;
        key->dst_ip.family = bdmf_ip_family_ipv4;
    }
    else if (tuple->addr_type == FLOW_DISSECTOR_KEY_IPV6_ADDRS)
    {
        memcpy(&key->src_ip.addr.ipv6, &tuple->ip.src_v6, sizeof(bdmf_ipv6_t));
        memcpy(&key->dst_ip.addr.ipv6, &tuple->ip.dst_v6, sizeof(bdmf_ipv6_t));
        key->src_ip.family = bdmf_ip_family_ipv6;
        key->dst_ip.family = bdmf_ip_family_ipv6;
    }

    if (tuple->ip_proto == IPPROTO_TCP || tuple->ip_proto == IPPROTO_UDP)
    {
        key->src_port = tuple->port.src;
        key->dst_port = tuple->port.dst;
    }
    else if (tuple->ip_proto == IPPROTO_GRE)
    {
        return -EOPNOTSUPP;
    }
    else if (tuple->ip_proto == IPPROTO_ESP)
    {
        return -EOPNOTSUPP;
    }

    key->client_idx = 0; /* used for unicast flooding */

    return 0;
}

