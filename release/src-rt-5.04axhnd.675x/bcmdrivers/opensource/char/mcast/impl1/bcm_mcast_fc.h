/*
<:copyright-BRCM:2020:DUAL/GPL:standard

   Copyright (c) 2020 Broadcom 
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

#ifndef _BCM_MCAST_FC_H_
#define _BCM_MCAST_FC_H_

#include "bcm_mcast_whitelist.h"

extern int bcm_mcast_mode_prv;

typedef struct {
    union {
        void *hdl_p;
#if defined(CONFIG_BLOG)
        BlogActivateKey_t blog_key;
#endif
    };
    struct list_head list;
}bcm_mcast_flowkey_t;

typedef struct bcm_mcast_flowhdl
{
    int numkeys;
    struct list_head flowkey_list;
    struct list_head list;
}bcm_mcast_flowhdl_t; 

/* The VLANDEV_MAX_COUNT value was chosen arbitrarily
   to be a large number. Typically there are 2 or 3 lower VLAN devices
   on the source WAN side */
#define BCM_MCAST_VLANDEV_MAX_COUNT 15
typedef struct bcm_mcast_vlandev_list 
{
    int vlandev_count;
    struct net_device *vlandev[BCM_MCAST_VLANDEV_MAX_COUNT];
} bcm_mcast_vlandev_list_t;

typedef struct bcm_mcast_grpinfo_node
{
    struct hlist_node hlist;
    int is_ssm;
    bcm_mcast_ipaddr_t grp;
    bcm_mcast_ipaddr_t src;
    int mcast_excl_udp_port;
    char enRtpSeqCheck;
    struct list_head clientinfo_list;
    struct list_head white_list;
}bcm_mcast_grpinfo_node_t;

typedef struct bcm_mcast_clientinfo_node
{
    struct net_device *to_accel_dev;
    uint32_t wl_info;
    unsigned char clientmac[ETH_ALEN];
    BlogActivateKey_t blog_idx;
    Blog_t *fc_blog_p;
    BlogTraffic_t traffic;
    bcm_mcast_grpinfo_node_t *parent_grpinfo_node;
    struct list_head rxinfo_list;
    struct list_head list;
}bcm_mcast_clientinfo_node_t;

typedef struct bcm_mcast_rxinfo_node
{
    Blog_t *blog_p;
    bcm_mcast_clientinfo_node_t *parent_clientinfo_node;
    struct list_head list;
}bcm_mcast_rxinfo_node_t;

#if defined(CC_MCAST_WHITELIST_SUPPORT)
typedef struct bcm_mcast_whitelist_node
{
    uint16_t outer_vlanid;
    uint8_t refcnt;
    bcm_mcast_grpinfo_node_t *parent_grpinfo_node;
    whitelist_key_t whitelist_key;
    struct list_head list;
}bcm_mcast_whitelist_node_t; 
#endif

#define BCM_MCAST_FLOWCTRL_HASH_BITS 9
#define BCM_MCAST_FLOWCTRL_HASH_SIZE (1 << BCM_MCAST_FLOWCTRL_HASH_BITS)

typedef struct bcm_mcast_flowctrl
{
    struct hlist_head grpinfo_hash[BCM_MCAST_FLOWCTRL_HASH_SIZE];
    struct kmem_cache *grpinfo_cache;
    struct kmem_cache *clientinfo_cache;
    struct kmem_cache *rxinfo_cache;
#if defined(CC_MCAST_WHITELIST_SUPPORT)
    struct kmem_cache *whitelist_cache;
#endif
    int hash_salt;
}bcm_mcast_flowctrl_t;

int bcm_mcast_add_flowkey_to_flowhdl(uintptr_t flowhdl, 
                                     bcm_mcast_flowkey_t *flowkeyin_p);
int bcm_mcast_create_flow(bcm_mcast_ifdata  *pif, 
                          void *mc_fdb,
                          int proto, 
                          struct hlist_head *head, 
                          uintptr_t *flowhdl);
void bcm_mcast_delete_flow(int        proto,
                           uintptr_t *flowhdl);
int bcm_mcast_fc_init(void);
void bcm_mcast_fc_exit(void);
void bcm_mcast_fc_flowhdl_dump(void);
void bcm_mcast_dump_grpinfo_all(void);
#endif
