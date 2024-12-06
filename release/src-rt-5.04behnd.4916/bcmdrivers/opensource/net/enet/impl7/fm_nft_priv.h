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

#ifndef __FM_NFT_PRIV_H__
#define __FM_NFT_PRIV_H__

#ifdef CONFIG_BCM_XDP
#include <net/xdp_nft.h>
#endif
#include "fm_nft.h"
#include "bdmf_data_types.h"

#define fm_nft_printk(lvl, dev, fmt, args...) \
    do { \
        if (fm_nft_global_trace_level >= lvl) \
        netdev_printk(loglevel2kern(lvl), dev, fmt, ## args); \
    } while(0)

extern int fm_nft_global_trace_level;
#define LOGLEVEL_VERBOSE (LOGLEVEL_DEBUG+1)

#define ETHHDR_ADDRS_LEN    ETH_ALEN * 2    /* Size of 2 MAC addresses */

typedef struct nft_fw_tuple {
    u16 addr_type;
    __be16 n_proto;
    u8 ip_proto;
    struct {
        union {
            __be32 src_v4;
            struct in6_addr src_v6;
        };
        union {
            __be32 dst_v4;
            struct in6_addr dst_v6;
        };
    } ip;
    struct {
        __be16 src;
        __be16 dst;
    } port;

    u16 zone;
} nft_fw_tuple_t;

typedef struct nft_fm_vlan {
    struct vlan_hdr vhdr[VLAN_MAX_DEPTH];
    int vlan_tag_num;
} nft_fm_vlan_t;

typedef struct fm_nft_ctx_t {
    struct rhashtable fm_nft_flow_table;
    struct rhashtable_params fm_nft_flow_params;

    struct bpf_prog_aux *aux;
    struct net_device *dev;
#ifdef CONFIG_BCM_XDP
    xdp_nft_get_progs_by_dev_cb get_xdp_prog_cb;
#endif
} fm_nft_ctx_t;

typedef struct nft_fm_tc_info_t {
    fm_nft_ctx_t *fm_nft_ctx;
    nft_fw_tuple_t tuple_key;
    nft_fw_tuple_t tuple_ctx;
    __u32 tx_ifindex;           /* egress interface index*/
    bool is_snat;
    int l2_hdr_size;
    int l2_hdr_delta;
    struct ethhdr eth;
    nft_fm_vlan_t vlan;
    unsigned long cookie;
} nft_fm_tc_info_t;

typedef enum {
    IFINDEX_ERR,
    IFINDEX_DIFFER,
    IFINDEX_MATCH
} ifindex_match_t;

typedef struct fm_nft_stats {
    u64 packets;
    u64 bytes;
    u64 drops;
} fm_nft_stats_t;

typedef struct fm_nft_flow_node_t {
    unsigned long cookie;
    struct rhash_head node;
    __u32 tx_ifindex;
    fm_nft_stats_t stats;       /* updated stats */
    fm_nft_stats_t prev_stats;  /* previous snap-shot of stats */
    unsigned long lastused;     /* jiffies */

#ifdef CONFIG_BCM_XDP
    bool xdp_accellerated;
#endif
    union {
#ifdef CONFIG_BCM_XDP
      xdp_fc_key_t key;
#endif
      bdmf_index index;
    };
} fm_nft_flow_node_t;

typedef enum {
    FN_NFT_HW_ACCEL_XDP,
    FN_NFT_HW_ACCEL_RNR,
} fm_nft_hw_accel_t;




#define RETURN_ON_ERR(rc,errcode,dev,msg,...) if ((rc)) { fm_nft_printk(LOGLEVEL_ERR, dev, "ERR %i : " msg,(uint32_t) rc, ##__VA_ARGS__); return errcode; }
#define RETURN_ON_ERR_LVL(rc,errcode,lvl,dev,msg,...) if ((rc)) { fm_nft_printk(lvl, dev, "ERR %i : " msg,(uint32_t) rc, ##__VA_ARGS__); return errcode; }


#define GOTO_ON_ERR(condition,label,dev,prntmsg,...) if ((condition)) { fm_nft_printk(LOGLEVEL_ERR, dev, prntmsg, ##__VA_ARGS__); goto label; }


const char *loglevel2kern(int level);
int nft_fm_tc_rule_to_key_tuple(struct net_device *dev, struct nft_fw_tuple *tuple, struct flow_rule *rule);
void nft_fm_dump_tuple(struct net_device *dev, nft_fw_tuple_t *tuple);
void nft_fm_dump_macs(struct net_device *dev, struct ethhdr *eth);
int nft_fm_tc_parse_actions(nft_fm_tc_info_t *ctx, struct flow_action *flow_action, struct netlink_ext_ack *extack);
extern const struct rhashtable_params fm_nft_flow_table_params;

int nft_fm_to_xdp_lookup_entry_add(nft_fm_tc_info_t *nft_fm_tc_info_ctx, fm_nft_flow_node_t * flow_node);
int nft_fm_to_rnr_lookup_entry_add(nft_fm_tc_info_t *nft_fm_tc_info_ctx, fm_nft_flow_node_t * flow_node);

int nft_fm_to_xdp_lookup_entry_delete(fm_nft_ctx_t *fm_nft_ctx, fm_nft_flow_node_t *flow_node);
int nft_fm_to_rnr_lookup_entry_delete(fm_nft_ctx_t *fm_nft_ctx, fm_nft_flow_node_t *flow_node);

// xdp specific functions
int xdp_nft_init(fm_nft_ctx_t * ctx, xdp_nft_get_progs_by_dev_cb get_xdp_prog_cb);
// rnr specific functions
int rnr_nft_init(struct net_device * dev);
void rnr_nft_uninit(void);
void nft_rnr_update_flow_stat(fm_nft_ctx_t * ctx, fm_nft_flow_node_t * flow_node);

bdmf_object_handle get_bdmf_object_handle_from_dev(struct net_device *root_dev);

#endif
