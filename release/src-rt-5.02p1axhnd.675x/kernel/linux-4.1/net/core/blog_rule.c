#if defined(CONFIG_BCM_KF_BLOG)
/* 
* <:copyright-BRCM:2010:DUAL/GPL:standard
* 
*    Copyright (c) 2010 Broadcom 
*    All Rights Reserved
* 
* Unless you and Broadcom execute a separate written software license
* agreement governing use of this software, this software is licensed
* to you under the terms of the GNU General Public License version 2
* (the "GPL"), available at http://www.broadcom.com/licenses/GPLv2.php,
* with the following added to such license:
* 
*    As a special exception, the copyright holders of this software give
*    you permission to link this software with independent modules, and
*    to copy and distribute the resulting executable under terms of your
*    choice, provided that you also meet, for each linked independent
*    module, the terms and conditions of the license of that module.
*    An independent module is a module which is not derived from this
*    software.  The special exception does not apply to any modifications
*    of the software.
* 
* Not withstanding the above, under no circumstances may you combine
* this software in any way with any other Broadcom software provided
* under a license other than the GPL, without Broadcom's express prior
* written consent.
* 
:>
*/

/*
 *******************************************************************************
 *
 * File Name  : blog_rule.c
 *
 * Description: Implements packet modification rules that can be associated to
 *              a Blog.
 *
 *******************************************************************************
 */

#include <linux/slab.h>
#include <linux/blog.h>
#include <linux/if_ether.h>
#include <linux/if_pppox.h>
#include <linux/if_vlan.h>
#include <linux/in.h>
#include <net/ip.h>
#include <linux/blog_rule.h>
#include <linux/export.h> 


/*
 *------------------------------------------------------------------------------
 * Private functions, macros and global variables.
 *------------------------------------------------------------------------------
 */

#if defined(CC_CONFIG_BLOG_RULE_DEBUG)
#define blog_rule_assertv(cond)                                         \
    if ( !(cond) ) {                                                    \
        printk( "BLOG RULE ASSERT %s : " #cond, __FUNCTION__ );         \
        return;                                                         \
    }
#define blog_rule_assertr(cond, rtn)                                    \
    if ( !(cond) ) {                                                    \
        printk( "BLOG RULE ASSERT %s : " #cond CLRN, __FUNCTION__ );    \
        return rtn;                                                     \
    }
#else
#define blog_rule_assertv(cond)
#define blog_rule_assertr(cond, rtn)
#endif

typedef struct {
    struct kmem_cache *kmemCache;
} blogRule_Ctrl_t;

static blogRule_Ctrl_t blogRuleCtrl;

/* External hooks */
blogRuleVlanHook_t blogRuleVlanHook = NULL;
blogRuleVlanNotifyHook_t blogRuleVlanNotifyHook = NULL;
#if (defined(CONFIG_BCM_ARL) || defined(CONFIG_BCM_ARL_MODULE))
blogArlHook_t bcm_arl_process_hook_g = NULL;
#endif

#undef  BLOG_RULE_DECL
#define BLOG_RULE_DECL(x) #x

static char *blogRuleCommandName[] = {
    BLOG_RULE_DECL(BLOG_RULE_CMD_NOP),
    BLOG_RULE_DECL(BLOG_RULE_CMD_SET_MAC_DA),
    BLOG_RULE_DECL(BLOG_RULE_CMD_SET_MAC_SA),
    BLOG_RULE_DECL(BLOG_RULE_CMD_SET_ETHERTYPE),
    BLOG_RULE_DECL(BLOG_RULE_CMD_PUSH_VLAN_HDR),
    BLOG_RULE_DECL(BLOG_RULE_CMD_POP_VLAN_HDR),
    BLOG_RULE_DECL(BLOG_RULE_CMD_SET_PBITS),
    BLOG_RULE_DECL(BLOG_RULE_CMD_SET_DEI),
    BLOG_RULE_DECL(BLOG_RULE_CMD_SET_VID),
    BLOG_RULE_DECL(BLOG_RULE_CMD_SET_VLAN_PROTO),
    BLOG_RULE_DECL(BLOG_RULE_CMD_COPY_PBITS),
    BLOG_RULE_DECL(BLOG_RULE_CMD_COPY_DEI),
    BLOG_RULE_DECL(BLOG_RULE_CMD_COPY_VID),
    BLOG_RULE_DECL(BLOG_RULE_CMD_COPY_VLAN_PROTO),
//    BLOG_RULE_DECL(BLOG_RULE_CMD_XLATE_DSCP_TO_PBITS),
    BLOG_RULE_DECL(BLOG_RULE_CMD_POP_PPPOE_HDR),
    BLOG_RULE_DECL(BLOG_RULE_CMD_SET_DSCP),
    BLOG_RULE_DECL(BLOG_RULE_CMD_DECR_TTL),
    BLOG_RULE_DECL(BLOG_RULE_CMD_DECR_HOP_LIMIT),
    BLOG_RULE_DECL(BLOG_RULE_CMD_DROP),
    BLOG_RULE_DECL(BLOG_RULE_CMD_SET_SKB_MARK_PORT),
    BLOG_RULE_DECL(BLOG_RULE_CMD_SET_SKB_MARK_QUEUE),
    BLOG_RULE_DECL(BLOG_RULE_CMD_OVRD_LEARNING_VID),
    BLOG_RULE_DECL(BLOG_RULE_CMD_SET_STA_MAC_ADDRESS),
    BLOG_RULE_DECL(BLOG_RULE_CMD_MAX)
};

static void __printEthAddr(char *name, char *addr)
{
    int i;

    printk("%s : ", name);

    for(i=0; i<ETH_ALEN; ++i)
    {
        printk("%02X", addr[i]);
        if(i != ETH_ALEN-1) printk(":");
    }

    printk("\n");
}

/*
 *------------------------------------------------------------------------------
 * Public API
 *------------------------------------------------------------------------------
 */

/*
 *------------------------------------------------------------------------------
 * Function     : blog_rule_alloc
 * Description  : Allocates a Blog Rule from the Blog Rule cache. Once the Blog
 *                Rule is not needed, it must be freed back to the Blog Rule
 *                cache via blog_rule_free().
 *------------------------------------------------------------------------------
 */
blogRule_t *blog_rule_alloc(void)
{
    return kmem_cache_alloc(blogRuleCtrl.kmemCache, GFP_ATOMIC);
}

/*
 *------------------------------------------------------------------------------
 * Function     : blog_rule_free
 * Description  : Frees a Blog Rule previously allocated via blog_rule_alloc().
 *------------------------------------------------------------------------------
 */
void blog_rule_free(blogRule_t *blogRule_p)
{
    kmem_cache_free(blogRuleCtrl.kmemCache, blogRule_p);
}

/*
 *------------------------------------------------------------------------------
 * Function     : blog_rule_free_list
 * Description  : Frees all Blog Rules linked to a Blog.
 *------------------------------------------------------------------------------
 */
int blog_rule_free_list(Blog_t *blog_p)
{
    blogRule_t *blogRule_p;
    blogRule_t *nextBlogRule_p;
    int blogRuleCount;

    blogRule_p = (blogRule_t *)blog_p->blogRule_p;
    blogRuleCount = 0;

    while(blogRule_p != NULL)
    {
        nextBlogRule_p = blogRule_p->next_p;

        blog_rule_free(blogRule_p);

        blogRule_p = nextBlogRule_p;

        blogRuleCount++;
    }

    return blogRuleCount;
}

/*
 *------------------------------------------------------------------------------
 * Function     : blog_rule_init
 * Description  : Initializes a Blog Rule with no filters and no modifications.
 *------------------------------------------------------------------------------
 */
void blog_rule_init(blogRule_t *blogRule_p)
{
    blog_rule_assertv(blogRule_p != NULL);

    memset(blogRule_p, 0, sizeof(blogRule_t));
}

/*
 *------------------------------------------------------------------------------
 * Function     : blog_rule_dump
 * Description  : Prints the contents of a Blog Rule.
 *------------------------------------------------------------------------------
 */
void blog_rule_dump(blogRule_t *blogRule_p)
{
    int i;
    blogRuleFilter_t *filter_p;
    blogRuleFilterVlan_t *vlanFilter_p;
    blogRuleAction_t *action_p;

    blog_rule_assertv(blogRule_p != NULL);

    printk("Blog Rule <%p>, next <%p>\n",
           blogRule_p, blogRule_p->next_p);

    filter_p = &blogRule_p->filter;

    if(filter_p->flags)
    {
        printk("Flags: ");
        if(filter_p->flags & BLOG_RULE_FILTER_FLAGS_IS_UNICAST)
        {
            printk("IS_UNICAST ");
        }
        if(filter_p->flags & BLOG_RULE_FILTER_FLAGS_IS_MULTICAST)
        {
            printk("IS_MULTICAST ");
        }
        if(filter_p->flags & BLOG_RULE_FILTER_FLAGS_IS_BROADCAST)
        {
            printk("IS_BROADCAST ");
        }
        printk("\n");
    }

    printk("Ethernet Filters:\n");
    if(blog_rule_filterInUse(filter_p->eth.mask.h_dest))
    {
        __printEthAddr("\tDA", filter_p->eth.value.h_dest);
    }
    if(blog_rule_filterInUse(filter_p->eth.mask.h_source))
    {
        __printEthAddr("\tSA", filter_p->eth.value.h_source);
    }
    if(blog_rule_filterInUse(filter_p->eth.mask.h_proto))
    {
        printk("\tEthertype : 0x%04X\n", filter_p->eth.value.h_proto);
    }

    printk("PPPoE Header: %s\n", filter_p->hasPppoeHeader ? "Yes" : "No");

    printk("VLAN Filters:\n");
    printk("\tNumber of Tags : <%d>\n", filter_p->nbrOfVlanTags);
    for(i=0; i<filter_p->nbrOfVlanTags; ++i)
    {
        vlanFilter_p = &filter_p->vlan[i];

        if(vlanFilter_p->mask.h_vlan_TCI & BLOG_RULE_PBITS_MASK)
        {
            printk("\tPBITS : <%d>, tag <%d>\n",
                   BLOG_RULE_GET_TCI_PBITS(vlanFilter_p->value.h_vlan_TCI), i);
        }

        if(vlanFilter_p->mask.h_vlan_TCI & BLOG_RULE_DEI_MASK)
        {
            printk("\tDEI   : <%d>, tag <%d>\n",
                   BLOG_RULE_GET_TCI_DEI(vlanFilter_p->value.h_vlan_TCI), i);
        }

        if(vlanFilter_p->mask.h_vlan_TCI & BLOG_RULE_VID_MASK)
        {
            printk("\tVID   : <%d>, tag <%d>\n",
                   BLOG_RULE_GET_TCI_VID(vlanFilter_p->value.h_vlan_TCI), i);
        }

        if(vlanFilter_p->mask.h_vlan_encapsulated_proto)
        {
            printk("  etherType   : <%04x>, tag <%d>\n",
                   vlanFilter_p->value.h_vlan_encapsulated_proto, i);
        }
    }

    printk("IPv4 Filters:\n");
    if(blog_rule_filterInUse(filter_p->ipv4.mask.tos))
    {
        printk("\tTOS : 0x%04X -> DSCP <%d>\n",
               filter_p->ipv4.value.tos,
               filter_p->ipv4.value.tos >> BLOG_RULE_DSCP_IN_TOS_SHIFT);
    }
    if(blog_rule_filterInUse(filter_p->ipv4.mask.ip_proto))
    {
        printk("\tIP-PROTO : %d \n", filter_p->ipv4.value.ip_proto);
    }

    printk("SKB Filters:\n");
    if(blog_rule_filterInUse(filter_p->skb.priority))
    {
        printk("\tpriority : %d\n", filter_p->skb.priority - 1);
    }
    if(blog_rule_filterInUse(filter_p->skb.markFlowId))
    {
        printk("\tmark->flowId : %d\n", filter_p->skb.markFlowId);
    }
    if(blog_rule_filterInUse(filter_p->skb.markPort))
    {
        printk("\tmark->port : %d\n", filter_p->skb.markPort - 1);
    }

    printk("Actions:\n");
    for(i=0; i<blogRule_p->actionCount; ++i)
    {
        action_p = &blogRule_p->action[i];

        if(action_p->cmd == BLOG_RULE_CMD_SET_MAC_DA ||
           action_p->cmd == BLOG_RULE_CMD_SET_MAC_SA ||
           action_p->cmd == BLOG_RULE_CMD_SET_STA_MAC_ADDRESS)
        {
            printk("\t");
            __printEthAddr(blogRuleCommandName[action_p->cmd],
                           action_p->macAddr);
        }
        else
        {
            printk("\t%s : arg <%d>/<0x%02X>, tag <%d>\n",
                   blogRuleCommandName[action_p->cmd],
                   action_p->arg, action_p->arg, action_p->toTag);
        }
    }
}

/*
 *------------------------------------------------------------------------------
 * Function     : blog_rule_add_action
 * Description  : Adds an action to a Blog Rule.
 *------------------------------------------------------------------------------
 */
int blog_rule_add_action(blogRule_t *blogRule_p, blogRuleAction_t *action_p)
{
    int ret = 0;

    if(blogRule_p->actionCount == BLOG_RULE_ACTION_MAX)
    {
        printk("ERROR : Maximum number of actions reached for blogRule_p <%p>\n",
               blogRule_p);

        ret = -ENOMEM;
        goto out;
    }

    blogRule_p->action[blogRule_p->actionCount] = *action_p;

    blogRule_p->actionCount++;

out:
    return ret;
}

/*
 *------------------------------------------------------------------------------
 * Function     : blog_rule_delete_action
 * Description  : Set actionCount of all blogRule chain to 0 for deleting action
 *------------------------------------------------------------------------------
 */
int blog_rule_delete_action( void *rule_p )
{
    blogRule_t *blogrule_p = (blogRule_t *)rule_p;
    int ret = 0;

    while ( blogrule_p != NULL )
    {
        blogrule_p->actionCount = 0;
        blogrule_p = blogrule_p->next_p;
    }

    return ret;
}


/*
 *------------------------------------------------------------------------------
 * Function     : __init_blog_rule
 * Description  : Initializes the Blog Rule subsystem.
 *------------------------------------------------------------------------------
 */
static int __init __init_blog_rule(void)
{
    int ret = 0;

    /* create a slab cache for device descriptors */
    blogRuleCtrl.kmemCache = kmem_cache_create("blog_rule",
                                               sizeof(blogRule_t),
                                               0, /* align */
                                               SLAB_HWCACHE_ALIGN, /* flags */
                                               NULL); /* ctor */
    if(blogRuleCtrl.kmemCache == NULL)
    {
        printk("ERROR : Unable to create Blog Rule cache\n");

        ret = -ENOMEM;
        goto out;
    }

    printk("BLOG Rule %s Initialized\n", BLOG_RULE_VERSION);

out:
    return ret;
}

/* /\* */
/*  *------------------------------------------------------------------------------ */
/*  * Function     : __exit_blog_rule */
/*  * Description  : Brings down the Blog Rule subsystem. */
/*  *------------------------------------------------------------------------------ */
/*  *\/ */
/* void __exit __exit_blog_rule(void) */
/* { */
/*     kmem_cache_destroy(blogRuleCtrl.kmemCache); */
/* } */

subsys_initcall(__init_blog_rule);

EXPORT_SYMBOL(blog_rule_alloc);
EXPORT_SYMBOL(blog_rule_free);
EXPORT_SYMBOL(blog_rule_free_list);
EXPORT_SYMBOL(blog_rule_init);
EXPORT_SYMBOL(blog_rule_dump);
EXPORT_SYMBOL(blog_rule_add_action);
EXPORT_SYMBOL(blog_rule_delete_action);
EXPORT_SYMBOL(blogRuleVlanHook);
EXPORT_SYMBOL(blogRuleVlanNotifyHook);
#if (defined(CONFIG_BCM_ARL) || defined(CONFIG_BCM_ARL_MODULE))
EXPORT_SYMBOL(bcm_arl_process_hook_g);
#endif
#endif /* defined(CONFIG_BCM_KF_BLOG) && defined(CONFIG_BLOG) */
