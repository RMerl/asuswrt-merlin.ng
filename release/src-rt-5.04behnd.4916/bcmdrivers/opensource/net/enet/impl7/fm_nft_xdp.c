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

/*
 *  Created on: Jan/2023
 *      Author: itai.weiss@broadcom.com
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
#include <net/xdp_nft.h>
#include "fm_nft_priv.h"


static void nft_fm_dump_key(struct net_device *dev, xdp_fc_key_t *key)
{
    fm_nft_printk(LOGLEVEL_DEBUG, dev, "Adding the following key:  protocol: %d, rx_ifindex: %d\n", key->proto,
        key->rx_ifindex);
    fm_nft_printk(LOGLEVEL_DEBUG, dev, "source ip: %pI4, dest ip: %pI4\n", &key->src_ip, &key->dst_ip);
    fm_nft_printk(LOGLEVEL_DEBUG, dev, "source port: %d, dest port: %d\n", htons(key->src_port), htons(key->dst_port));
}

static void nft_fm_dump_ctx(struct net_device *dev, xdp_fc_ctx_t *ctx)
{
    fm_nft_printk(LOGLEVEL_DEBUG, dev, "Created context: tx_ifindex: %d\n", ctx->tx_ifindex);
    fm_nft_printk(LOGLEVEL_DEBUG, dev, "source nat ip: %pI4, dest nat ip: %pI4\n", &ctx->src_nat_ip, &ctx->dst_nat_ip);
    fm_nft_printk(LOGLEVEL_DEBUG, dev, "source nat port: %d, dest nat port: %d\n", htons(ctx->src_nat_port),
        htons(ctx->dst_nat_port));
    fm_nft_printk(LOGLEVEL_DEBUG, dev, "L2 header r_shift %d, L2 header l_shift %d\n", ctx->l2_header.r_shift,
        ctx->l2_header.l_shift);
    if (fm_nft_global_trace_level >= LOGLEVEL_DEBUG)
        print_hex_dump(KERN_DEBUG, "L2 HEADER:", DUMP_PREFIX_OFFSET, 16, 1, ctx->l2_header.hdr, 32, true);
}

static int xdp_update_map(struct net_device *dev, struct bpf_map *map, void *key, void *ctx, int is_delete)
{
    int rc;

    RETURN_ON_ERR(!map, -1, dev, "Invalid map\n");

    if (is_delete)
        rc = map->ops->map_delete_elem(map, key);
    else
        rc = map->ops->map_update_elem(map, key, ctx, BPF_ANY);

    RETURN_ON_ERR(rc, -1, dev, "Could not %s elem in map %s, rc=%d\n", is_delete ? "delete" : "update",
            map->name, rc);

    fm_nft_printk(LOGLEVEL_DEBUG, dev, "Map %s %s succeeded!\n", map->name, is_delete ? "deleted" : "updated");

    return rc;
}

static void l2_header_create(xdp_fc_ctx_t *ctx, nft_fm_tc_info_t *nft_fm_ctx)
{
    int i, offset = 0;

    /* Prepare L2 header */
    ctx->l2_header.r_shift = offset = MAX_L2_HEADER - nft_fm_ctx->l2_hdr_size;
    ctx->l2_header.l_shift = ctx->l2_header.r_shift + nft_fm_ctx->l2_hdr_delta;

    /* 1. Copy mac addresses and update offset */
    memcpy(&ctx->l2_header.hdr[offset], &nft_fm_ctx->eth, ETHHDR_ADDRS_LEN);
    offset += ETHHDR_ADDRS_LEN;

    /* 2. Copy vlan id and protocol */
    for (i = 0; i < nft_fm_ctx->vlan.vlan_tag_num && offset != MAX_L2_HEADER; i++)
    {
        memcpy(&ctx->l2_header.hdr[offset], &(nft_fm_ctx->vlan.vhdr[i].h_vlan_encapsulated_proto), sizeof(__be16));
        offset += sizeof(__be16);

        memcpy(&ctx->l2_header.hdr[offset], &(nft_fm_ctx->vlan.vhdr[i].h_vlan_TCI), sizeof(__be16));
        offset += sizeof(__be16);
    }
}

static void nft_fm_create_key(nft_fm_tc_info_t *info, xdp_fc_key_t *key)
{
    nft_fw_tuple_t *tuple = &info->tuple_key;

    memset(key, 0, sizeof(*key));

    key->src_ip = cpu_to_be32(tuple->ip.src_v4);
    key->dst_ip = cpu_to_be32(tuple->ip.dst_v4);
    key->proto = tuple->ip_proto;
    key->src_port = tuple->port.src;
    key->dst_port = tuple->port.dst;
    key->rx_ifindex = info->fm_nft_ctx->dev->ifindex;
}

static void nft_fm_create_ctx(nft_fm_tc_info_t *nft_fm_ctx, xdp_fc_ctx_t *ctx)
{
    nft_fw_tuple_t *tuple_nat = &nft_fm_ctx->tuple_ctx;

    ctx->src_nat_ip = tuple_nat->ip.src_v4;
    ctx->dst_nat_ip = tuple_nat->ip.dst_v4;
    ctx->src_nat_port = tuple_nat->port.src;
    ctx->dst_nat_port = tuple_nat->port.dst;

    l2_header_create(ctx, nft_fm_ctx);

    ctx->tx_ifindex = nft_fm_ctx->tx_ifindex;
    ctx->cookie = nft_fm_ctx->cookie;
}

static int nft_fm_xdm_update_maps(fm_nft_ctx_t *fm_nft_ctx, xdp_fc_key_t *key, xdp_fc_ctx_t *ctx, __u32 tx_ifindex,
    int is_delete)
{
    int i, rc = 0;

    struct net_device *dev = fm_nft_ctx->dev;
    
        
    /* TODO - Use index table instead of strcmp (performance optimization) */
    for (i = 0; i < fm_nft_ctx->aux->used_map_cnt; i++)
    {
        struct bpf_map *curr_map = fm_nft_ctx->aux->used_maps[i];

        mutex_lock(&fm_nft_ctx->aux->used_maps_mutex);

        if (!strcmp(curr_map->name, XDP_FC_TABLE_MAP_NAME))
        {
            fm_nft_printk(LOGLEVEL_INFO, dev, "Updating map %s\n", curr_map->name);
            rc = xdp_update_map(dev, curr_map, key, ctx, is_delete);
        }
        else if (!strcmp(curr_map->name, XDP_FC_PACKETS_COUNT_MAP_NAME))
            fm_nft_printk(LOGLEVEL_INFO, dev, "Unexpected map %s\n", curr_map->name);
        else
            fm_nft_printk(LOGLEVEL_ERR, dev, "Unknown map %s\n", curr_map->name);

        mutex_unlock(&fm_nft_ctx->aux->used_maps_mutex);
    }

    return rc;
}

int nft_fm_to_xdp_lookup_entry_add(nft_fm_tc_info_t *nft_fm_tc_info_ctx, fm_nft_flow_node_t * flow_node)
{
    xdp_fc_key_t *key;
    xdp_fc_ctx_t ctx = {};
    int rc;
    struct bpf_prog *prog;
    fm_nft_ctx_t *fm_nft_ctx = nft_fm_tc_info_ctx->fm_nft_ctx;
    struct net_device *dev = fm_nft_ctx->dev;

    fm_nft_printk(LOGLEVEL_DEBUG, dev, "%s: Entered. cookie %lu\n", __FUNCTION__, flow_node->cookie);

    if (!fm_nft_ctx->get_xdp_prog_cb)
        return -1;
    prog = fm_nft_ctx->get_xdp_prog_cb(dev);
    if (!prog)
        return -1;

    key = &flow_node->key;

    nft_fm_create_key(nft_fm_tc_info_ctx, key);
    nft_fm_create_ctx(nft_fm_tc_info_ctx, &ctx);

    fm_nft_ctx->aux = prog->aux;

    nft_fm_dump_macs(dev, &nft_fm_tc_info_ctx->eth);
    nft_fm_dump_key(dev, key);
    nft_fm_dump_ctx(dev, &ctx);

    rc = nft_fm_xdm_update_maps(fm_nft_ctx, key, &ctx, nft_fm_tc_info_ctx->tx_ifindex, 0);
    RETURN_ON_ERR(rc,rc,dev,"nft_fm_xdm_update_maps fails %i\n",rc);

    flow_node->xdp_accellerated = 1;
    return 0;
}

int nft_fm_to_xdp_lookup_entry_delete(fm_nft_ctx_t *fm_nft_ctx, fm_nft_flow_node_t *flow_node)
{
    xdp_fc_key_t *key;
    int rc;
    __u32 tx_ifindex;
    struct net_device *dev;

    if (!fm_nft_ctx)
        return 0;

    dev = fm_nft_ctx->dev;

    fm_nft_printk(LOGLEVEL_VERBOSE, dev, "%s: Entered.\n", __FUNCTION__);

    key = &flow_node->key;
    tx_ifindex = flow_node->tx_ifindex;

    nft_fm_dump_key(dev, key);

    rc = nft_fm_xdm_update_maps(fm_nft_ctx, key, NULL, tx_ifindex, 1);
    if (rc)
    {
        fm_nft_printk(LOGLEVEL_ERR, dev, "%s: nft_fm_xdm_update_maps failed. rc %d\n",
            __FUNCTION__, rc);
    }

    return 0;
}

u32 fm_nft_run_xdp_meta(struct net_device *dev, struct bpf_prog *xdp_prog, struct xdp_buff *xdp, unsigned long *cookie)
{
    u32 act;

    /* Shift meta_data offset to leave room for L2 header and cookie */
    xdp->data_meta = xdp->data - sizeof(*cookie) - MAX_L2_HEADER;
    *(unsigned long *)xdp->data_meta = 0;

    act = bpf_prog_run_xdp(xdp_prog, xdp);

    *cookie = *(unsigned long *)xdp->data_meta;

    fm_nft_printk(LOGLEVEL_DEBUG, dev, "%s: Received cookie %lu\n", __FUNCTION__, *cookie);

    return act;
}


int xdp_nft_init(fm_nft_ctx_t *ctx, xdp_nft_get_progs_by_dev_cb get_xdp_prog_cb)
{
    ctx->get_xdp_prog_cb = get_xdp_prog_cb;

    return 0;
}
