/*
<:copyright-BRCM:2020:DUAL/GPL:standard

   Copyright (c) 2020 Broadcom 
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
}bcm_mcast_grpinfo_node_t;

typedef struct bcm_mcast_clientinfo_node
{
    struct net_device *client_dev_p;
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

#define BCM_MCAST_FLOWCTRL_HASH_BITS 9
#define BCM_MCAST_FLOWCTRL_HASH_SIZE (1 << BCM_MCAST_FLOWCTRL_HASH_BITS)

typedef struct bcm_mcast_flowctrl
{
    struct hlist_head grpinfo_hash[BCM_MCAST_FLOWCTRL_HASH_SIZE];
    struct kmem_cache *grpinfo_cache;
    struct kmem_cache *clientinfo_cache;
    struct kmem_cache *rxinfo_cache;
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
