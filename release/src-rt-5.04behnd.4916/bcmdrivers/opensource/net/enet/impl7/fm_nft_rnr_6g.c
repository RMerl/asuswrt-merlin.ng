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

/* TODO : this file is still work in progress and not run-time tested */
#if 0

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
#include <linux/rhashtable.h>

#include <net/flow_dissector.h>
#include <net/pkt_cls.h>
#include "port.h"
#include "enet.h"
#include "fm_nft_priv.h"
#include <rdpa_api.h>
#include <bcm_util_func.h>
#include "cmdlist_api.h"

bdmf_object_handle ucast_class = NULL;


extern int nft_fm_rnr_l3_key_get(nft_fm_tc_info_t *info, rdpa_ip_flow_key_t *key);


#if 0
static int l3_add_nat(nft_fm_tc_info_t * nft_fm_ctx, rdpa_ip_flow_result_t *result,rdpa_traffic_dir dir)
{
    nft_fw_tuple_t * from = &nft_fm_ctx->tuple_key;
    nft_fw_tuple_t * to = &nft_fm_ctx->tuple_ctx;

    if (to->addr_type == FLOW_DISSECTOR_KEY_IPV4_ADDRS)
    {
        if (from->ip.src_v4 != to->ip.src_v4) {
            if (dir == rdpa_dir_ds)
                return -1; // mangling wrong direction
            result->nat_ip.addr.ipv4 = to->ip.src_v4;
            result->action_vec |= rdpa_fc_action_nat;
        }
        if (from->ip.dst_v4 != to->ip.dst_v4) {
            if (dir == rdpa_dir_us)
               return -1; // mangling wrong direction
            result->nat_ip.addr.ipv4 = to->ip.dst_v4;
            result->action_vec |= rdpa_fc_action_nat;
        }
    }
    else if (to->addr_type == FLOW_DISSECTOR_KEY_IPV6_ADDRS)
    {
        if (memcmp(&from->ip.src_v6,&to->ip.src_v6,sizeof(struct in6_addr))) {
            if (dir == rdpa_dir_ds)
                return -1; // mangling wrong direction
            memcpy(&result->nat_ip.addr.ipv6, &to->ip.src_v6,sizeof(struct in6_addr));
            result->action_vec |= rdpa_fc_action_nat;
        }
        if (memcmp(&from->ip.dst_v6,&to->ip.dst_v6,sizeof(struct in6_addr))) {
            if (dir == rdpa_dir_us)
                return -1; // mangling wrong direction
            memcpy(&result->nat_ip.addr.ipv6, &to->ip.dst_v6,sizeof(struct in6_addr));
            result->action_vec |= rdpa_fc_action_nat;
        }
    }
    else return -1;

    if (to->addr_type == FLOW_DISSECTOR_KEY_IPV4_ADDRS || to->addr_type == FLOW_DISSECTOR_KEY_IPV6_ADDRS)
    {
        if (from->port.src != to->port.src) {
            if (dir == rdpa_dir_ds) return -1; // mangling wrong direction
            result->nat_port = to->port.src;
            result->action_vec |= rdpa_fc_action_nat;
        }
        if (from->port.dst != to->port.dst) {
            if (dir == rdpa_dir_us) return -1; // mangling wrong direction
            result->nat_port = to->port.dst;
            result->action_vec |= rdpa_fc_action_nat;
        }
    }

    return 0;
}
#endif

static int l2_header_create(nft_fm_tc_info_t * nft_fm_ctx, rdpa_ip_flow_result_t *result)
{
    //unsigned int i, offset = ETHHDR_ADDRS_LEN;
    result->max_pkt_len = 1518 + 4*nft_fm_ctx->vlan.vlan_tag_num;

    cmdlist_init(result->cmd_list, RDPA_CMD_LIST_UCAST_LIST_SIZE, RDPA_CMD_LIST_UCAST_LIST_OFFSET);
#if 0
    	*err = pktrunner_ucast_cmdlist_create(blog_p, prepend_p, cmdlist_buffer_pp, &ip_flow.result);
    err = cmdlist_ucast_create(blog_p, target, prepend_p,
                               cmdlist_buffer_pp, PKTRUNNER_BRCM_TAG_MODE, &tx_adjust);
#endif

 
#if 0
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
#endif

    return 0;
}


static int add_fwd_commands(rdpa_ip_flow_result_t *result, bdmf_object_handle port_egress_obj)
{
    result->port_egress_obj = port_egress_obj;

    result->is_routed = 1;

    result->tos = 8;

    result->ip_addresses_table_index = 4;

    result->pathstat_idx  = 1;

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

#if 0
    rc = l3_add_nat(nft_fm_tc_info_ctx, &ip_flow.result,ip_flow.key.dir);
    RETURN_ON_ERR(rc,rc,dev,"l3_add_nat");
#endif
    
    nft_fm_dump_macs(dev, &nft_fm_tc_info_ctx->eth);

    rc = rdpa_ucast_flow_add(ucast_class, &flow_node->index, &ip_flow);
    RETURN_ON_ERR(rc < 0, -1, dev, "Failed to activate flow");

    return 0;
}


int nft_fm_to_rnr_lookup_entry_delete(fm_nft_ctx_t *fm_nft_ctx,  fm_nft_flow_node_t * flow_node) 
{
    rdpa_ucast_flow_delete(ucast_class, flow_node->index);
    return 0;
}

void nft_rnr_update_flow_stat(fm_nft_ctx_t * ctx, fm_nft_flow_node_t * flow_node)
{
    rdpa_stat_t flow_stat;
    int rc = rdpa_ucast_flow_stat_get(ucast_class, flow_node->index, &flow_stat);
    RETURN_ON_ERR(rc,,ctx->dev,"cannot retrieve stat for index %i\n",(uint32_t) flow_node->index);

    flow_node->stats.packets = flow_stat.packets;
    flow_node->stats.bytes = flow_stat.bytes;
}

static int ucast_class_created_here;

void rnr_nft_uninit(void)
{
    if (ucast_class && ucast_class_created_here)
        bdmf_destroy(ucast_class);
}


int rnr_nft_init(struct net_device * dev)
{
    int rc;
    BDMF_MATTR_ALLOC(ucast_attrs, rdpa_ucast_drv());
    rc = rdpa_ucast_get(&ucast_class);
    if (rc) {
        rc = bdmf_new_and_set(rdpa_ucast_drv(), NULL, ucast_attrs, &ucast_class);
        GOTO_ON_ERR(rc,ucast_class_error,dev,"rdpa ucast_class object does not exist and can't be created.\n");
        ucast_class_created_here = 1; //  planning to move this code to rdpa_mw_sys_init.c 
    }
#if 0 // might be needed when we add NAT
    {
        cmdlist_hooks_t cmdlist_hooks;

        cmdlist_hooks.ipv6_addresses_table_add = runnerUcast_ipv6_addresses_table_add;
        cmdlist_hooks.ipv4_addresses_table_add = runnerUcast_ipv4_addresses_table_add;
        cmdlist_hooks.brcm_tag_info = NULL;

        cmdlist_bind(&cmdlist_hooks);
    }
#endif

#if 0 // NOT SURE WE NEED THIS: LEAVE IT FOR NOW : REVIEW ME
    rc = rdpa_ucast_class_key_type_set(ucast_class, RDPA_IP_CLASS_6TUPLE);
    GOTO_ON_ERR(rc,ucast_class_error,dev,"ucast_class can't set key_type attribute\n");
#endif

    return 0;

ucast_class_error:
    BDMF_MATTR_FREE(ucast_attrs);
    return -1;
}

#else
#include "port.h"
#include "enet.h"
#include "fm_nft_priv.h"
int nft_fm_to_rnr_lookup_entry_add(nft_fm_tc_info_t *nft_fm_tc_info_ctx, fm_nft_flow_node_t * flow_node)
{
  return -1;
}
int nft_fm_to_rnr_lookup_entry_delete(fm_nft_ctx_t *fm_nft_ctx,  fm_nft_flow_node_t * flow_node)
{
  return -1;
}
int rnr_nft_init(struct net_device * dev)
{
  return 0;
}
void rnr_nft_uninit(void) { }
void nft_rnr_update_flow_stat(fm_nft_ctx_t * ctx, fm_nft_flow_node_t * flow_node) { }
#endif
