/*
   <:copyright-BRCM:2023:DUAL/GPL:standard

      Copyright (c) 2023 Broadcom 
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

#include <net/netfilter/nf_conntrack.h>
#include <net/netfilter/nf_conntrack_core.h>
#include <net/netfilter/nf_conntrack_zones.h>
#include <net/netfilter/nf_conntrack_labels.h>
#include <net/netfilter/nf_conntrack_helper.h>
#include <net/netfilter/nf_conntrack_acct.h>
#include <uapi/linux/tc_act/tc_pedit.h>
#include <net/tc_act/tc_ct.h>
#include <net/flow_offload.h>
#include <net/netfilter/nf_flow_table.h>
#include <linux/workqueue.h>
#include <linux/refcount.h>
#include <linux/xarray.h>
#include <linux/if_vlan.h>

#include <net/flow_dissector.h>
#include <net/pkt_cls.h>
#include "port.h"
#include "enet.h"
#include "fm_nft_priv.h"
#include <rdpa_api.h>
#include <bcm_util_func.h>

bdmf_object_handle ip_class = NULL;


extern int nft_fm_rnr_l3_key_get(nft_fm_tc_info_t *info, rdpa_ip_flow_key_t *key);

static int l3_add_nat(struct net_device * dev, nft_fm_tc_info_t * nft_fm_ctx, rdpa_ip_flow_result_t *result,rdpa_traffic_dir dir)
{
    bool src_port_changed = false;
    nft_fw_tuple_t * from = &nft_fm_ctx->tuple_key;
    nft_fw_tuple_t * to = &nft_fm_ctx->tuple_ctx;

    if (to->addr_type == FLOW_DISSECTOR_KEY_IPV4_ADDRS)
    {
        if (from->ip.src_v4 != to->ip.src_v4) {
            result->nat_ip.addr.ipv4 = cpu_to_be32(to->ip.src_v4);
            result->action_vec |= rdpa_fc_action_nat;
            result->nat_port = to->port.src;
        }
        if (from->ip.dst_v4 != to->ip.dst_v4) {
            if (result->action_vec & rdpa_fc_action_nat)
                fm_nft_printk(LOGLEVEL_WARNING, dev, "%s: rdpa_fc_action_nat set twice\n",__FUNCTION__);
            result->nat_ip.addr.ipv4 = cpu_to_be32(to->ip.dst_v4);
            result->action_vec |= rdpa_fc_action_nat;
            result->nat_port = to->port.dst;
        }
    }
    else if (to->addr_type == FLOW_DISSECTOR_KEY_IPV6_ADDRS)
    {
        if (memcmp(&from->ip.src_v6,&to->ip.src_v6,sizeof(struct in6_addr))) {
            if (dir == rdpa_dir_ds)
                return -1; // mangling wrong direction
            memcpy(&result->nat_ip.addr.ipv6, &to->ip.src_v6,sizeof(struct in6_addr));
            result->action_vec |= rdpa_fc_action_nat;
            result->nat_port = to->port.src;
        }
        if (memcmp(&from->ip.dst_v6,&to->ip.dst_v6,sizeof(struct in6_addr))) {
            if (result->action_vec & rdpa_fc_action_nat)
                fm_nft_printk(LOGLEVEL_WARNING, dev, "%s: rdpa_fc_action_nat set twice\n",__FUNCTION__);
            memcpy(&result->nat_ip.addr.ipv6, &to->ip.dst_v6,sizeof(struct in6_addr));
            result->action_vec |= rdpa_fc_action_nat;
            result->nat_port = to->port.dst;
        }
    }
    else 
        return -1;

    if (to->addr_type == FLOW_DISSECTOR_KEY_IPV4_ADDRS || to->addr_type == FLOW_DISSECTOR_KEY_IPV6_ADDRS)
    {
        if (from->port.src != to->port.src) {
            result->nat_port = to->port.src;
            result->action_vec |= rdpa_fc_action_nat;
            src_port_changed = true;
        }
        if (from->port.dst != to->port.dst) {
            if (src_port_changed)
                fm_nft_printk(LOGLEVEL_WARNING, dev, "%s: rdpa_fc_action_nat set twice\n",__FUNCTION__);
            result->nat_port = to->port.dst;
            result->action_vec |= rdpa_fc_action_nat;
        }
    }

    return 0;
}

static int l2_header_create(nft_fm_tc_info_t * nft_fm_ctx, rdpa_ip_flow_result_t *result)
{
    unsigned int i, offset = ETHHDR_ADDRS_LEN;
    result->max_pkt_len = 1518 + 4*nft_fm_ctx->vlan.vlan_tag_num;
    result->l2_header_size  = nft_fm_ctx->l2_hdr_size + 2 /* eth_type*/;
 
    memcpy(&result->l2_header[0],&nft_fm_ctx->eth,ETHHDR_ADDRS_LEN);

    for (i = 0; i < nft_fm_ctx->vlan.vlan_tag_num && offset < RDPA_L2_HEADER_SIZE; i++)
    {
        memcpy(&result->l2_header[offset], &(nft_fm_ctx->vlan.vhdr[i].h_vlan_encapsulated_proto), sizeof(__be16));
        offset += sizeof(__be16);

        memcpy(&result->l2_header[offset], &(nft_fm_ctx->vlan.vhdr[i].h_vlan_TCI), sizeof(__be16));
        offset += sizeof(__be16);
    }

    result->l2_header_number_of_tags = nft_fm_ctx->vlan.vlan_tag_num;
    result->l2_header_offset = -nft_fm_ctx->l2_hdr_delta;

    memcpy(&result->l2_header[offset],&nft_fm_ctx->tuple_ctx.n_proto,sizeof(__be16));

    return 0;
}


static int add_fwd_commands(rdpa_ip_flow_result_t *result, bdmf_object_handle port_egress_obj)
{
    result->port_egress_obj = port_egress_obj;

    result->action_vec = rdpa_fc_action_ttl;

    result->queue_id = 0;

    return 0;
}

int nft_fm_to_rnr_lookup_entry_add(nft_fm_tc_info_t *nft_fm_tc_info_ctx, fm_nft_flow_node_t * flow_node)
{
    rdpa_ip_flow_info_t ip_flow = {};

    int rc;

    bdmf_object_handle egress_object_handle;
    fm_nft_ctx_t * fm_nft_ctx = nft_fm_tc_info_ctx->fm_nft_ctx;
    struct net_device * dev = fm_nft_ctx->dev;
    struct net_device * egress = dev_get_by_index(&init_net,nft_fm_tc_info_ctx->tx_ifindex);

    RETURN_ON_ERR(!egress,-ENOMEM,dev,"Fail getting netdevice from ifindex %i\n",nft_fm_tc_info_ctx->tx_ifindex);

    egress_object_handle = get_bdmf_object_handle_from_dev(egress);
    RETURN_ON_ERR(!egress_object_handle,-ENOMEM,dev,"Fail getting bdmf_object_handle from ifindex %i\n",nft_fm_tc_info_ctx->tx_ifindex);

    fm_nft_printk(LOGLEVEL_DEBUG, dev, "%s: Entered. cookie %lu\n", __FUNCTION__, flow_node->cookie);

    rc = nft_fm_rnr_l3_key_get(nft_fm_tc_info_ctx, &ip_flow.key);
    RETURN_ON_ERR(rc,rc,dev,"nft_fm_rnr_l3_key_get");

    rc = add_fwd_commands(&ip_flow.result, egress_object_handle);
    RETURN_ON_ERR(rc,rc,dev,"add_fwd_commands");

    rc = l2_header_create(nft_fm_tc_info_ctx, &ip_flow.result);
    RETURN_ON_ERR(rc,rc,dev,"l2_header_create");

    rc = l3_add_nat(dev,nft_fm_tc_info_ctx, &ip_flow.result,ip_flow.key.dir);
    RETURN_ON_ERR(rc,rc,dev,"l3_add_nat");
    
    nft_fm_dump_macs(dev, &nft_fm_tc_info_ctx->eth);

    rc = rdpa_ip_class_flow_add(ip_class, &flow_node->index, &ip_flow);
    RETURN_ON_ERR(rc < 0, -1, dev, "Failed to activate flow");

    return 0;
}


int nft_fm_to_rnr_lookup_entry_delete(fm_nft_ctx_t *fm_nft_ctx,  fm_nft_flow_node_t * flow_node) 
{
    fm_nft_printk(LOGLEVEL_VERBOSE, fm_nft_ctx->dev, "%s: cookie %lu\n",__FUNCTION__,flow_node->cookie);
    rdpa_ip_class_flow_delete(ip_class, flow_node->index);
    return 0;
}

void nft_rnr_update_flow_stat(fm_nft_ctx_t * ctx, fm_nft_flow_node_t * flow_node)
{
    rdpa_stat_t flow_stat;
    int rc = rdpa_ip_class_flow_stat_get(ip_class, flow_node->index, &flow_stat);
    RETURN_ON_ERR(rc,,ctx->dev,"cannot retrieve stat for index %i\n",(uint32_t) flow_node->index);

    if (flow_node->stats.bytes != flow_stat.bytes)
        flow_node->lastused = jiffies;

    flow_node->stats.packets = flow_stat.packets;
    flow_node->stats.bytes = flow_stat.bytes;
}

void rnr_nft_uninit(void)
{
    if (ip_class)
        bdmf_destroy(ip_class);
}


int rnr_nft_init(struct net_device * dev)
{
    int rc;
    BDMF_MATTR_ALLOC(ip_class_attrs, rdpa_ip_class_drv());

    rc = rdpa_ip_class_key_type_set(ip_class_attrs, RDPA_IP_CLASS_6TUPLE);
    GOTO_ON_ERR(rc,ip_class_error,dev,"ip_class can't set key_type attribute\n");

    rc = rdpa_ip_class_tos_mflows_set(ip_class_attrs, 0);
    GOTO_ON_ERR(rc,ip_class_error,dev,"ip_class can't set tos mflows \n");

    rc = bdmf_new_and_set(rdpa_ip_class_drv(), NULL, ip_class_attrs, &ip_class);
    GOTO_ON_ERR(rc,ip_class_error,dev,"rdpa ip_class object does not exist and can't be created.\n");

    return 0;

ip_class_error:
    BDMF_MATTR_FREE(ip_class_attrs);
    return -1;
}

