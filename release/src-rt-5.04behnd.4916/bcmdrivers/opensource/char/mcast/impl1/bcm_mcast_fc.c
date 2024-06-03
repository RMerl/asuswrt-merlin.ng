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

#include "bcm_mcast_priv.h"
/* Global variable to hold mode = PRV/Flow-cache */
int bcm_mcast_mode_prv = 0;
static int bcm_mcast_fc_flow_add(Blog_t *blog_p, BlogTraffic_t traffic, bcm_mcast_flowkey_t *flow_key_p);
static Blog_t* bcm_mcast_fc_flow_delete(bcm_mcast_flowkey_t flow_key, BlogTraffic_t traffic);

struct list_head      flowhdl_list;
struct kmem_cache    *flowhdl_cache = NULL;
struct kmem_cache    *flowkey_cache = NULL;

static inline bcm_mcast_flowhdl_t* bcm_mcast_flowhdl_alloc(void)
{
    bcm_mcast_flowhdl_t *flowhdl_p = NULL;

    flowhdl_p = kmem_cache_alloc(flowhdl_cache, GFP_ATOMIC);
    if ( !flowhdl_p )
    {
        __logError("Unable to allocate flowhdl");
        return NULL;
    }
    return flowhdl_p;
}

static inline bcm_mcast_flowkey_t* bcm_mcast_flowkey_alloc(void)
{
    bcm_mcast_flowkey_t *flowkey_p = NULL;

    flowkey_p = kmem_cache_alloc(flowkey_cache, GFP_ATOMIC);
    if ( !flowkey_p )
    {
        __logError("Unable to allocate flowkey");
        return NULL;
    }
    return flowkey_p;
}

static inline void bcm_mcast_flowhdl_free(bcm_mcast_flowhdl_t *flowhdl_p)
{
    kmem_cache_free(flowhdl_cache, flowhdl_p);
}

static inline void bcm_mcast_flowkey_free(bcm_mcast_flowkey_t *flowkey_p)
{
    kmem_cache_free(flowkey_cache, flowkey_p);
}

void bcm_mcast_flowkey_dump(bcm_mcast_flowkey_t *flowkey_p)
{
    printk("\tFlowkey 0x%px\n", flowkey_p->hdl_p);
}

void bcm_mcast_flowhdl_dump(bcm_mcast_flowhdl_t *flowhdl_p)
{
    bcm_mcast_flowkey_t *flowkey_p=NULL;
    printk("flowhdl_p 0x%px numflowkeys %d\n", flowhdl_p, flowhdl_p->numkeys);

    if ( list_empty(&flowhdl_p->flowkey_list) )
    {
        printk("***No flowkey entries found***\n");
        return;
    }

    list_for_each_entry(flowkey_p, &flowhdl_p->flowkey_list, list)
    {
        if ( flowkey_p )
        {
            bcm_mcast_flowkey_dump(flowkey_p);
        }
    }
}

void bcm_mcast_fc_flowhdl_dump()
{
    bcm_mcast_flowhdl_t *flowhdl_p = NULL;

    if ( list_empty(&flowhdl_list) )
    {
        printk("***No flowhdl entries found***\n");
        return;
    }

    list_for_each_entry(flowhdl_p, &flowhdl_list, list)
    {
        if ( flowhdl_p )
        {
            bcm_mcast_flowhdl_dump(flowhdl_p);
        }
    }
}

static inline int bcm_mcast_flowhdl_init(uintptr_t flowhdl)
{
    bcm_mcast_flowhdl_t *flow_hdl_p = (bcm_mcast_flowhdl_t *)flowhdl;
    if ( flow_hdl_p )
    {
        flow_hdl_p->numkeys = 0;
        INIT_LIST_HEAD(&flow_hdl_p->flowkey_list);
    }
    return 0;
}

/* flowhdl is a return value used by multicast driver
   to later delete the flow. It is not used(NULL) for now
   since the flow handle which is the blogkey is currently
   stored in the mc_fdb. Later, a new interface structure
   would be defined instead of mc_fdb and a proper flow handle
   will be returned which will be used by mcast driver for
   mcast flow deletion */
int bcm_mcast_create_flow(bcm_mcast_ifdata  *pif,
                          void              *mc_fdb,
                          int                proto,
                          struct hlist_head *head,
                          uintptr_t         *flowhdl)
{
    bcm_mcast_flowhdl_t *flowhdl_p = NULL;
    int numactivates = 0;

    if ((flowhdl_p = bcm_mcast_flowhdl_alloc()) == NULL)
    {
        return -1;
    }

    *flowhdl = (uintptr_t)flowhdl_p;
    bcm_mcast_flowhdl_init(*flowhdl);

    numactivates = bcm_mcast_blog_process(pif,
                                          mc_fdb,
                                          proto,
                                          head,
                                          *flowhdl);
    __logInfo("numactivates %d", numactivates);
    if ( numactivates > 0 )
    {
        list_add(&flowhdl_p->list, &flowhdl_list); //success
    } 
    else
    {
        // failure
        *flowhdl = 0;
        bcm_mcast_flowhdl_free(flowhdl_p);
    }
    return numactivates;
}

void bcm_mcast_delete_flow(int proto, uintptr_t *flowhdl)
{
    bcm_mcast_flowhdl_t *flowhdl_p;
    bcm_mcast_flowkey_t *flowkey_p;
    bcm_mcast_flowkey_t *flowkeytmp_p;

    //Check if flowhdl is non NULL
    if ( !flowhdl || !(*flowhdl) )
    {
        __logError("flowhdl NULL");
        return;
    }
    __logDebug("ENTER");

    flowhdl_p =  (bcm_mcast_flowhdl_t *)*flowhdl;

    list_for_each_entry_safe(flowkey_p, 
                             flowkeytmp_p, &flowhdl_p->flowkey_list, list) 
    {
        bcm_mcast_blog_release(proto, *flowkey_p);
        list_del(&flowkey_p->list);
        bcm_mcast_flowkey_free(flowkey_p);
        flowhdl_p->numkeys--;
    }

    if (flowhdl_p->numkeys != 0)
    {
        __logError("Number of flowkeys %d not 0 before deleting flow handle", flowhdl_p->numkeys);
        flowhdl_p->numkeys = 0;
    }
    list_del(&flowhdl_p->list);
    bcm_mcast_flowhdl_free(flowhdl_p);

    //Reset the flow handle in the mcast fdb
    *flowhdl = 0;
    __logDebug("EXIT");
}

int bcm_mcast_add_flowkey_to_flowhdl(uintptr_t flowhdl, 
                                     bcm_mcast_flowkey_t *flowkeyin_p)
{
    bcm_mcast_flowhdl_t *flow_hdl_p = (bcm_mcast_flowhdl_t *)flowhdl;
    bcm_mcast_flowkey_t *flowkey_p = NULL;

    if ((flowkey_p = bcm_mcast_flowkey_alloc()) == NULL)
    {
        return -1;
    }

    flowkey_p->hdl_p = flowkeyin_p->hdl_p;
    list_add(&flowkey_p->list, &flow_hdl_p->flowkey_list);
    flow_hdl_p->numkeys++;
    return 0;
}

bcm_mcast_flowctrl_t *flowctrl_p = NULL;

static inline int bcm_mcast_hash_3words(const u32 w1, const u32 w2, const u32 w3)
{
    return (jhash_3words(w1, w2, w3, flowctrl_p->hash_salt) & (BCM_MCAST_FLOWCTRL_HASH_SIZE - 1));
}

void bcm_mcast_dump_clientinfo(bcm_mcast_clientinfo_node_t *clientinfo_p)
{
    printk("\nclient_dev_p %s wl info 0x%x clientmac %pM\n", 
           clientinfo_p->client_dev_p ? clientinfo_p->client_dev_p->name:"NULL", 
           clientinfo_p->wl_info, clientinfo_p->clientmac);
}

void bcm_mcast_dump_rxinfo(bcm_mcast_rxinfo_node_t *rxinfo_p)
{
    printk("\t\trxinfo blog_p 0x%px\n", rxinfo_p->blog_p);
}

void bcm_mcast_dump_grpinfo(bcm_mcast_grpinfo_node_t *grpinfo_p)
{
    bcm_mcast_rxinfo_node_t *rxinfo_p;
    bcm_mcast_clientinfo_node_t *clientinfo_p;

    if ( grpinfo_p->grp.is_ipv4 )
    {
        printk("is_ssm %d grp %pI4 src %pI4\n",
                   grpinfo_p->is_ssm,
                   &grpinfo_p->grp.ipv4_addr, 
                   &grpinfo_p->src.ipv4_addr);
    }
    else
    {
        printk("is_ssm %d grp %pI6 src %pI6\n",
                   grpinfo_p->is_ssm,
                   &grpinfo_p->grp.ipv6_addr,
                   &grpinfo_p->src.ipv6_addr);
    }
    printk("mcast_exclude_udp_port 0x%x rtp seq check %d\n", 
           grpinfo_p->mcast_excl_udp_port, grpinfo_p->enRtpSeqCheck);

    if ( list_empty(&grpinfo_p->clientinfo_list) )
    {
        printk("\t***No Client List entries found***\n");
    }
    else
    {
        printk("\t***Client List***\n");
        list_for_each_entry(clientinfo_p, &grpinfo_p->clientinfo_list, list)
        {
            if ( clientinfo_p )
            {
                bcm_mcast_dump_clientinfo(clientinfo_p);

                if ( list_empty(&clientinfo_p->rxinfo_list) )
                {
                    printk("\t\t***No Rx Info list entries found***\n");
                }
                else
                {
                    printk("\t\t***Rx Info list***\n");
                    list_for_each_entry(rxinfo_p, &clientinfo_p->rxinfo_list, list)
                    {
                        if ( rxinfo_p )
                        {
                            bcm_mcast_dump_rxinfo(rxinfo_p);
                        }
                    }
                }
            }
        }
    }
}

void bcm_mcast_dump_grpinfo_all(void)
{
    bcm_mcast_grpinfo_node_t *grpinfo_p;
    struct hlist_head *head;
    int hashix=0;
    int cnt=0;

    if (flowctrl_p == NULL)
    {
        printk("***No Group entries found\n");
        return;
    }
    for (hashix = 0; hashix < BCM_MCAST_FLOWCTRL_HASH_SIZE; hashix++)
    {
        head = &flowctrl_p->grpinfo_hash[hashix];
        hlist_for_each_entry(grpinfo_p, head, hlist) 
        {
            if ( grpinfo_p )
            {
                printk("***Group info entry %d hashix %d***\n", ++cnt, hashix);
                bcm_mcast_dump_grpinfo(grpinfo_p);
            }
        }
    }
    if ( cnt == 0 )
    {
        printk("***No Group info entries found***\n");
    }
}

static inline bcm_mcast_grpinfo_node_t* bcm_mcast_grpinfo_node_alloc(void)
{
    bcm_mcast_grpinfo_node_t *grpinfo_node_p = NULL;

    grpinfo_node_p = kmem_cache_alloc(flowctrl_p->grpinfo_cache, GFP_ATOMIC);
    if ( !grpinfo_node_p )
    {
        __logError("Unable to allocate group info node");
        return NULL;
    }
    return grpinfo_node_p;
}

static inline bcm_mcast_clientinfo_node_t* bcm_mcast_clientinfo_node_alloc(void)
{
    bcm_mcast_clientinfo_node_t *clientinfo_node_p = NULL;

    clientinfo_node_p = kmem_cache_alloc(flowctrl_p->clientinfo_cache, GFP_ATOMIC);
    if ( !clientinfo_node_p )
    {
        __logError("Unable to allocate client info node");
        return NULL;
    }
    return clientinfo_node_p;
}

static inline bcm_mcast_rxinfo_node_t* bcm_mcast_rxinfo_node_alloc(void)
{
    bcm_mcast_rxinfo_node_t *rxinfo_node_p = NULL;

    rxinfo_node_p = kmem_cache_alloc(flowctrl_p->rxinfo_cache, GFP_ATOMIC);
    if ( !rxinfo_node_p )
    {
        __logError("Unable to allocate rx info node");
        return NULL;
    }
    return rxinfo_node_p;
}

static inline void bcm_mcast_grpinfo_node_free(bcm_mcast_grpinfo_node_t *grpinfo_node_p)
{
    kmem_cache_free(flowctrl_p->grpinfo_cache, grpinfo_node_p);
}

static inline void bcm_mcast_clientinfo_node_free(bcm_mcast_clientinfo_node_t *clientinfo_node_p)
{
    kmem_cache_free(flowctrl_p->clientinfo_cache, clientinfo_node_p);
}

static inline void bcm_mcast_rxinfo_node_free(bcm_mcast_rxinfo_node_t *rxinfo_node_p)
{
    kmem_cache_free(flowctrl_p->rxinfo_cache, rxinfo_node_p);
}

static inline int bcm_mcast_get_hashidx_for_grpinfo(bcm_mcast_grpinfo_node_t *grpinfo_p)
{
    int hashix = 0;
    if ( grpinfo_p->grp.is_ipv4 )
    {
        hashix = bcm_mcast_hash_3words(grpinfo_p->grp.ipv4_addr.s_addr,
                                       grpinfo_p->is_ssm,
                                       grpinfo_p->src.ipv4_addr.s_addr);
    }
    else
    {
        hashix = bcm_mcast_hash_3words((grpinfo_p->grp.ipv6_addr.s6_addr32[0] | grpinfo_p->grp.ipv6_addr.s6_addr32[3]),
                                       grpinfo_p->is_ssm,
                                       (grpinfo_p->src.ipv6_addr.s6_addr32[0] | grpinfo_p->src.ipv6_addr.s6_addr32[3]));
    }
    return hashix;
}

bcm_mcast_grpinfo_node_t* bcm_mcast_grpinfo_node_lookup(bcm_mcast_grpinfo_node_t *grpinfo_p)
{
    int hashix = 0;
    struct hlist_head *head;
    struct hlist_node *n;
    bcm_mcast_grpinfo_node_t *grpinfo_node_p;

    hashix = bcm_mcast_get_hashidx_for_grpinfo(grpinfo_p);
    head = &flowctrl_p->grpinfo_hash[hashix];

    hlist_for_each_entry_safe(grpinfo_node_p, n, head, hlist)
    {
        if (bcm_mcast_match_ipaddr(&grpinfo_node_p->grp, &grpinfo_p->grp) &&
            grpinfo_node_p->is_ssm ==grpinfo_p->is_ssm &&
            bcm_mcast_match_ipaddr(&grpinfo_node_p->src, &grpinfo_p->src))
        {
            /* Found a matching node */
            __logDebug("grpinfo node found");
            return grpinfo_node_p;
        }

    }
    return NULL;
}

static inline int bcm_mcast_grpinfo_node_fill_from_blog(bcm_mcast_grpinfo_node_t *grpinfo_p, 
                                                         Blog_t *blog_p, BlogTraffic_t traffic)
{
    if ( !grpinfo_p || !blog_p )
    {
        __logError("grpinfo_p 0x%px or blog_p 0x%px NULL", grpinfo_p, blog_p);
        return -1;
    }

    memset(grpinfo_p, 0, sizeof(*grpinfo_p));
    grpinfo_p->is_ssm = blog_p->is_ssm;
    if ( traffic == BlogTraffic_IPV4_MCAST )
    {
        grpinfo_p->grp.is_ipv4 = 1;
        grpinfo_p->grp.ipv4_addr.s_addr = blog_p->rx.tuple.daddr;
        grpinfo_p->src.is_ipv4 = 1;
        grpinfo_p->src.ipv4_addr.s_addr = blog_p->rx.tuple.saddr;
    }
    else
    {
        BCM_MCAST_IN6_ASSIGN_ADDR(&grpinfo_p->src.ipv6_addr, &blog_p->tupleV6.saddr);
        BCM_MCAST_IN6_ASSIGN_ADDR(&grpinfo_p->grp.ipv6_addr, &blog_p->tupleV6.daddr);
    }
    grpinfo_p->enRtpSeqCheck = blog_p->rtp_seq_chk;
    grpinfo_p->mcast_excl_udp_port = blog_p->mcast_excl_udp_port;

    return 0;
}

bcm_mcast_grpinfo_node_t* bcm_mcast_grpinfo_node_add(bcm_mcast_grpinfo_node_t *grpinfo_p)
{
    bcm_mcast_grpinfo_node_t *grpinfo_node_p = NULL;
    int hashix = 0;
    struct hlist_head *head;

    if ( grpinfo_p->grp.is_ipv4 )
    {
        __logDebug("is_ssm %d grp %pI4 src %pI4",
                   grpinfo_p->is_ssm,
                   &grpinfo_p->grp.ipv4_addr, 
                   &grpinfo_p->src.ipv4_addr);
    }
    else
    {
        __logDebug("is_ssm %d grp %pI6 src %pI6",
                   grpinfo_p->is_ssm,
                   &grpinfo_p->grp.ipv6_addr,
                   &grpinfo_p->src.ipv6_addr);
    }

    if ( (grpinfo_node_p = bcm_mcast_grpinfo_node_lookup(grpinfo_p)) == NULL )
    {
        __logDebug("Allocate new grpinfo_node");
        /* grpinfo node not found. Allocate a node */
        if ((grpinfo_node_p = bcm_mcast_grpinfo_node_alloc()) == NULL)
        {
            /* Memory allocation error */
            return NULL;
        }
        memset(grpinfo_node_p, 0, sizeof(*grpinfo_node_p));
        memcpy((char *)grpinfo_node_p, (char *)grpinfo_p, sizeof(*grpinfo_p));
        INIT_LIST_HEAD(&grpinfo_node_p->clientinfo_list);

        hashix = bcm_mcast_get_hashidx_for_grpinfo(grpinfo_node_p);
        head = &flowctrl_p->grpinfo_hash[hashix];
        hlist_add_head(&grpinfo_node_p->hlist, head);
    }
    __logDebug("grpinfo_node 0x%px",grpinfo_node_p);
    return grpinfo_node_p;
}

int bcm_mcast_grpinfo_node_del(bcm_mcast_grpinfo_node_t *grpinfo_node_p)
{
    if ( !grpinfo_node_p )
    {
        __logError("grpinfo_node_p NULL");
        return -1;
    }

    if ( (bcm_mcast_grpinfo_node_lookup(grpinfo_node_p)) == NULL )
    {
        __logError("Unable to find grpinfo node");
        bcm_mcast_dump_grpinfo(grpinfo_node_p);
        return -1;
    }

    if ( !list_empty(&grpinfo_node_p->clientinfo_list) )
    {
        __logDebug("Clientinfo list not empty, not deleting group");
        return 0;
    }

    hlist_del(&grpinfo_node_p->hlist);
    bcm_mcast_grpinfo_node_free(grpinfo_node_p);
    __logDebug("grpinfo node 0x%px freed successfully", grpinfo_node_p);
    return 0;
}

static inline bcm_mcast_rxinfo_node_t* bcm_mcast_rxinfo_node_lookup(Blog_t *blog_p, 
                                                                    struct list_head *rxinfolist_head_p)
{
    bcm_mcast_rxinfo_node_t *node_p;

    __logDebug("blog_p 0x%px", blog_p);

    if ( !rxinfolist_head_p )
    {
        __logError("rxinfolist head NULL");
        return NULL;
    }

    list_for_each_entry(node_p, rxinfolist_head_p, list)
    {
        if ( node_p && (node_p->blog_p == blog_p) )
        {
            __logDebug("rxinfo node found");
            return node_p;
        }
    }
    return NULL;
}

bcm_mcast_rxinfo_node_t* bcm_mcast_rxinfo_node_add(Blog_t *blog_p,
                                                   bcm_mcast_clientinfo_node_t *clientinfo_node_p)
{
    bcm_mcast_rxinfo_node_t *rxinfo_node_p = NULL;

    if ( !clientinfo_node_p || !blog_p)
    {
        __logError("clientinfo node 0x%px or blog_p 0x%px NULL", clientinfo_node_p, blog_p);
        return NULL;
    }

    if ( (rxinfo_node_p = bcm_mcast_rxinfo_node_lookup(blog_p, 
                                                       &clientinfo_node_p->rxinfo_list)) == NULL )
    {
        __logDebug("Allocate new rxinfo node");

        /* rxinfo node not found. Allocate a node */
        if ((rxinfo_node_p = bcm_mcast_rxinfo_node_alloc()) == NULL)
        {
            /* Memory allocation error */
            return NULL;
        }
        memset(rxinfo_node_p, 0, sizeof(*rxinfo_node_p));
        rxinfo_node_p->blog_p = blog_p;
        rxinfo_node_p->parent_clientinfo_node = clientinfo_node_p;
        list_add(&rxinfo_node_p->list, &clientinfo_node_p->rxinfo_list);
    }
    else
    {
        __logError("rxinfo node already exists 0x%px",rxinfo_node_p);
        return NULL;
    }
    __logDebug("rxinfo_node 0x%px", rxinfo_node_p);
    return rxinfo_node_p;
}

int bcm_mcast_rxinfo_node_del(bcm_mcast_rxinfo_node_t *rxinfo_node_p)
{
    bcm_mcast_clientinfo_node_t *parent_clientinfo_node_p;

    if ( !rxinfo_node_p || !rxinfo_node_p->blog_p )
    {
        __logError("rxinfo node 0x%px or rxinfo node blog_p 0x%px NULL", 
                   rxinfo_node_p, (rxinfo_node_p)? rxinfo_node_p->blog_p : 0);
        return -1;
    }
    parent_clientinfo_node_p = rxinfo_node_p->parent_clientinfo_node;

    if ( (bcm_mcast_rxinfo_node_lookup(rxinfo_node_p->blog_p, 
                                       &parent_clientinfo_node_p->rxinfo_list)) == NULL )
    {
        __logError("Unable to find rxinfo blog in parent list");
        return -1;
    }

    list_del(&rxinfo_node_p->list);
    bcm_mcast_rxinfo_node_free(rxinfo_node_p);
    __logDebug("rxinfo node 0x%px freed successfully", rxinfo_node_p);
    return 0;
}

static inline bcm_mcast_clientinfo_node_t* bcm_mcast_clientinfo_node_lookup(void *client_dev_p,
                                                                            uint32_t wl_info,
                                                                            struct list_head *clientinfo_head_p)
{
    bcm_mcast_clientinfo_node_t *clientinfo_node_p;
    __logDebug("client_dev_p = %s wl_info = 0x%x clientinfo_head_p = 0x%px",
               ((struct net_device *)client_dev_p)->name, wl_info, clientinfo_head_p);

    list_for_each_entry(clientinfo_node_p, clientinfo_head_p, list)
    {
        if ( clientinfo_node_p && (clientinfo_node_p->client_dev_p == client_dev_p) &&
             (clientinfo_node_p->wl_info == wl_info) )
        {
            return clientinfo_node_p;
        }
    }
    return NULL;
}

bcm_mcast_clientinfo_node_t* bcm_mcast_clientinfo_node_add(Blog_t *blog_p, BlogTraffic_t traffic, 
                                                           void *client_dev_p,
                                                           uint32_t wl_info,
                                                           uint8_t *clientmac,
                                                           bcm_mcast_grpinfo_node_t* grpinfo_node_p)
{
    bcm_mcast_clientinfo_node_t *clientinfo_node_p;
    struct list_head *clientinfo_head_p = &grpinfo_node_p->clientinfo_list;

    __logInfo("client_dev_p = %s wl_info = 0x%x clientmac = %pM clientinfo_head_p = 0x%px grpinfo_node_p = 0x%px",
              ((struct net_device *)client_dev_p)->name, wl_info, clientmac, clientinfo_head_p, grpinfo_node_p);

    clientinfo_node_p = bcm_mcast_clientinfo_node_lookup(client_dev_p, wl_info, clientinfo_head_p);

    if ( clientinfo_node_p == NULL )
    {
        Blog_t *new_blog_p;
        BlogActivateKey_t *blog_activate_key_p;

        __logDebug("Allocate a new clientinfo node 0x%px", clientinfo_node_p);
        clientinfo_node_p = bcm_mcast_clientinfo_node_alloc();
        if ( clientinfo_node_p == NULL )
            return NULL;
        memset(clientinfo_node_p, 0, sizeof(*clientinfo_node_p));
        clientinfo_node_p->client_dev_p = client_dev_p;
        clientinfo_node_p->wl_info = wl_info;
        memcpy(clientinfo_node_p->clientmac, clientmac, sizeof(clientinfo_node_p->clientmac));
        clientinfo_node_p->parent_grpinfo_node = grpinfo_node_p;
        INIT_LIST_HEAD(&clientinfo_node_p->rxinfo_list);

        /* get a new blog and copy original blog */
        new_blog_p = blog_get();
        if (new_blog_p == BLOG_NULL) 
        {
            return NULL;
        }
        blog_copy(new_blog_p, blog_p);
        /* TODO -- do we need to copy the blog_rule pointer?
         * RDP platforms are still using blog_rule */

        /* TODO -- mark blog_rule_p = NULL when all platforms have stopped using blog_rule_p */

        clientinfo_node_p->fc_blog_p = new_blog_p;

        __logInfo("Invoking blog_activate() vlan0 0x%x vlan1 0x%x numtags %d wl 0x%x wlsta_id/wlinfo 0x%x", 
                  new_blog_p->vtag[0], new_blog_p->vtag[1], new_blog_p->vtag_num, new_blog_p->wl,
                  new_blog_p->wlsta_id);
        blog_activate_key_p = blog_activate(clientinfo_node_p->fc_blog_p, traffic);
        if ( blog_activate_key_p == NULL )
        {
            __logError("blog_activate failure");
            blog_put(new_blog_p);
            bcm_mcast_clientinfo_node_free(clientinfo_node_p);
            return NULL;
        }
        clientinfo_node_p->blog_idx = *blog_activate_key_p;
        clientinfo_node_p->traffic = traffic;
        list_add(&clientinfo_node_p->list, clientinfo_head_p);
    }

    return clientinfo_node_p;
}

int bcm_mcast_clientinfo_node_del(bcm_mcast_clientinfo_node_t *clientinfo_node_p)
{
    bcm_mcast_grpinfo_node_t *grpinfo_node_p = clientinfo_node_p->parent_grpinfo_node;
    struct list_head *clientinfo_head_p = &grpinfo_node_p->clientinfo_list;

    __logInfo("client_dev_p = 0x%px wl_info = 0x%x clientmac = %pM grpinfo_node_p = 0x%px grpinfo_node_p = 0x%px",
               clientinfo_node_p->client_dev_p, clientinfo_node_p->wl_info, clientinfo_node_p->clientmac, clientinfo_head_p, grpinfo_node_p);

    if ( bcm_mcast_clientinfo_node_lookup(clientinfo_node_p->client_dev_p, clientinfo_node_p->wl_info, 
                                          clientinfo_head_p) == NULL )
    {
        /* Error handling */
        __logError("No node client_dev_p = 0x%px wl_info = 0x%x clientmac = %pM grpinfo_node_p = 0x%px grpinfo_node_p = 0x%px",
                   clientinfo_node_p->client_dev_p, clientinfo_node_p->wl_info, clientinfo_node_p->clientmac, clientinfo_head_p, grpinfo_node_p);
        return -1;
    }

    if ( list_empty(&clientinfo_node_p->rxinfo_list) )
    {
        Blog_t *blog_p;

        __logInfo("Rxinfo list empty; Delete the client node 0x%px, Invoking blog_deactivate()", clientinfo_node_p);
        blog_p = blog_deactivate(clientinfo_node_p->blog_idx, clientinfo_node_p->traffic);
        if ( blog_p != clientinfo_node_p->fc_blog_p )
        {
            /* Error handling -- should not happen; Expect to get same blog from FlowCache */
            __logError("Blog mismatch, serious error");
        }
        blog_put(clientinfo_node_p->fc_blog_p);
        list_del(&clientinfo_node_p->list);
        bcm_mcast_clientinfo_node_free(clientinfo_node_p);
    }
    return 0;
}

/* Flow-cache specific functions */
static int bcm_mcast_fc_flow_add(Blog_t *blog_p, BlogTraffic_t traffic, bcm_mcast_flowkey_t *flow_key_p)
{
    bcm_mcast_grpinfo_node_t    grpinfo_node;
    bcm_mcast_grpinfo_node_t    *grpinfo_node_p;
    bcm_mcast_clientinfo_node_t *clientinfo_node_p;
    bcm_mcast_rxinfo_node_t     *rxinfo_node_p;

    __logDebug("ENTER");
    /* Build the Multicast group info from blog */
    if ( bcm_mcast_grpinfo_node_fill_from_blog(&grpinfo_node, blog_p, traffic) )
    {
        return -1;
    }

    /* Add multicast group info to the hash */
    grpinfo_node_p = bcm_mcast_grpinfo_node_add(&grpinfo_node);

    if ( grpinfo_node_p == NULL )
    {
        __logError("Error adding group info node");
        return -1;
    }

    /* Get the Client info list pointer and add the Client */
    clientinfo_node_p = bcm_mcast_clientinfo_node_add(blog_p, traffic, blog_p->tx_dev_p, 
                                                      blog_p->wl, blog_p->src_mac.u8, grpinfo_node_p);

    if ( clientinfo_node_p == NULL )
    {
        __logError("Error adding client info node");
        bcm_mcast_dump_grpinfo(grpinfo_node_p);
        bcm_mcast_grpinfo_node_del(grpinfo_node_p);
        return -1;
    }

    /* Get the rxinfo list pointer from Client node and add the rxInfo */
    rxinfo_node_p = bcm_mcast_rxinfo_node_add(blog_p, clientinfo_node_p);

    if ( rxinfo_node_p == NULL )
    {
        __logError("Error adding rxinfo node");
        bcm_mcast_dump_grpinfo(grpinfo_node_p);
        bcm_mcast_clientinfo_node_del(clientinfo_node_p);
        bcm_mcast_grpinfo_node_del(grpinfo_node_p);
        return -1;
    }

    flow_key_p->hdl_p = rxinfo_node_p;

    __logDebug("EXIT");
    return 0;
}

static Blog_t* bcm_mcast_fc_flow_delete(bcm_mcast_flowkey_t flow_key, BlogTraffic_t traffic)
{
    bcm_mcast_rxinfo_node_t     *rxinfo_node_p = flow_key.hdl_p;
    bcm_mcast_clientinfo_node_t *clientinfo_node_p = rxinfo_node_p->parent_clientinfo_node;
    bcm_mcast_grpinfo_node_t    *grpinfo_node_p = clientinfo_node_p->parent_grpinfo_node;
    Blog_t *blog_p = rxinfo_node_p->blog_p;

    __logDebug("ENTER");

    /* Delete the rxinfo_node */
    if ( bcm_mcast_rxinfo_node_del(rxinfo_node_p) )
    {
        __logError("rxinfo delete failed");
        bcm_mcast_dump_grpinfo(grpinfo_node_p);
        return NULL;
    }

    /* Delete the clientinfo_node (it will take care of not deleting itself if no more rxinfo_nodes attached */
    if ( bcm_mcast_clientinfo_node_del(clientinfo_node_p) )
    {
        __logError("clientinfo delete failed");
        bcm_mcast_dump_grpinfo(grpinfo_node_p);
        /* Error not expected but continue and return blog since the rxinfo
           node has been deleted to avoid blog leak */
    }

    /* Delete the grpinfo_node */
    if ( bcm_mcast_grpinfo_node_del(grpinfo_node_p) )
    {
        __logError("groupinfo delete failed");
        bcm_mcast_dump_grpinfo(grpinfo_node_p);
        /* Error not expected but continue and return blog since the rxinfo
           node has been deleted to avoid blog leak */
    }

    __logDebug("EXIT");
    return blog_p;
}


int bcm_mcast_fc_init()
{
    /* Initialize common stuff across PRV/FlowCache */
    flowhdl_cache = kmem_cache_create("bcm_mcast_fc_flowhdl_cache",
                                      sizeof(bcm_mcast_flowhdl_t),
                                      0,
                                      SLAB_HWCACHE_ALIGN, NULL);
    if ( NULL == flowhdl_cache )
    {
        __logError("failed to allocate flowhdl_cache\n");
        return -ENOMEM;
    }

    INIT_LIST_HEAD(&flowhdl_list);

    flowkey_cache = kmem_cache_create("bcm_mcast_fc_flowkey_cache",
                                      sizeof(bcm_mcast_flowkey_t),
                                      0,
                                      SLAB_HWCACHE_ALIGN, NULL);
    if ( NULL == flowkey_cache )
    {
        __logError("failed to allocate flowkey_cache\n");
        return -ENOMEM;
    }

    /* Initialize FlowCache specific stuff */
    if ( bcm_mcast_mode_prv ) return 0;

    bcm_mcast_flow_add_hook = bcm_mcast_fc_flow_add;
    bcm_mcast_flow_delete_hook = bcm_mcast_fc_flow_delete;

    flowctrl_p = kmalloc(sizeof(*flowctrl_p), GFP_KERNEL);
    if ( NULL == flowctrl_p )
    {
        return -ENOMEM;
    }
    memset(flowctrl_p, 0, sizeof(*flowctrl_p)); 

    get_random_bytes(&flowctrl_p->hash_salt, sizeof(flowctrl_p->hash_salt));

    flowctrl_p->grpinfo_cache = kmem_cache_create("bcm_mcast_grpinfo_node_cache",
                                                  sizeof(bcm_mcast_grpinfo_node_t),
                                                  0,
                                                  SLAB_HWCACHE_ALIGN, NULL);
    if ( NULL == flowctrl_p->grpinfo_cache )
    {
        __logError("failed to allocate grpinfo_cache\n");
        return -ENOMEM;
    }

    flowctrl_p->clientinfo_cache = kmem_cache_create("bcm_mcast_clientinfo_node_cache",
                                                     sizeof(bcm_mcast_clientinfo_node_t),
                                                     0,
                                                     SLAB_HWCACHE_ALIGN, NULL);
    if ( NULL == flowctrl_p->clientinfo_cache )
    {
        __logError("failed to allocate clientinfo_cache\n");
        return -ENOMEM;
    }

    flowctrl_p->rxinfo_cache = kmem_cache_create("bcm_mcast_rxinfo_node_cache",
                                                 sizeof(bcm_mcast_rxinfo_node_t),
                                                 0,
                                                 SLAB_HWCACHE_ALIGN, NULL);
    if ( NULL == flowctrl_p->rxinfo_cache )
    {
        __logError("failed to allocate rxinfo_cache\n");
        return -ENOMEM;
    }

    return 0;
}

void bcm_mcast_fc_exit()
{
    bcm_mcast_flowhdl_t *flowhdl_p;

    list_for_each_entry(flowhdl_p, &flowhdl_list, list)
    {
        list_del(&flowhdl_p->list);
        kmem_cache_free(flowhdl_cache, flowhdl_p);
    }

    if ( flowhdl_cache )
    {
        kmem_cache_destroy(flowhdl_cache);
    }

    if ( flowkey_cache )
    {
        kmem_cache_destroy(flowkey_cache);
    }

    if ( bcm_mcast_mode_prv ) return;

    /* Release FlowCache specific stuff */
    if ( flowctrl_p->grpinfo_cache )
    {
        kmem_cache_destroy(flowctrl_p->grpinfo_cache);
    }

    if ( flowctrl_p->clientinfo_cache )
    {
        kmem_cache_destroy(flowctrl_p->clientinfo_cache);
    }

    if ( flowctrl_p->rxinfo_cache )
    {
        kmem_cache_destroy(flowctrl_p->rxinfo_cache);
    }

    kfree(flowctrl_p);
}
