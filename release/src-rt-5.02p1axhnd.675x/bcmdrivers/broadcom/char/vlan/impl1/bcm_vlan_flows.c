/*
<:copyright-BRCM:2012:proprietary:standard

   Copyright (c) 2012 Broadcom 
   All Rights Reserved

 This program is the proprietary software of Broadcom and/or its
 licensors, and may only be used, duplicated, modified or distributed pursuant
 to the terms and conditions of a separate, written license agreement executed
 between you and Broadcom (an "Authorized License").  Except as set forth in
 an Authorized License, Broadcom grants no license (express or implied), right
 to use, or waiver of any kind with respect to the Software, and Broadcom
 expressly reserves all rights in and to the Software and all intellectual
 property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU HAVE
 NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY NOTIFY
 BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.

 Except as expressly set forth in the Authorized License,

 1. This program, including its structure, sequence and organization,
    constitutes the valuable trade secrets of Broadcom, and you shall use
    all reasonable efforts to protect the confidentiality thereof, and to
    use this information only in connection with your use of Broadcom
    integrated circuit products.

 2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
    AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
    WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
    RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND
    ALL IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT,
    FITNESS FOR A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR
    COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE
    TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF USE OR
    PERFORMANCE OF THE SOFTWARE.

 3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR
    ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
    INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY
    WAY RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN
    IF BROADCOM HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES;
    OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE
    SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE LIMITATIONS
    SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF ANY
    LIMITED REMEDY.
:> 
*/
#include <linux/kernel.h>
#include <linux/spinlock.h>
#include <linux/netdevice.h>
#if defined(CONFIG_BLOG)
#include <linux/if_vlan.h>
#include <linux/blog.h>
#include <linux/blog_rule.h>
#include <linux/vlanctl_bind.h>
#include "bcm_vlan_local.h"
#include "bcm_vlan_flows.h"
#endif


#if defined(CONFIG_BLOG)

//#define TEST_VLAN_FLOWS

#ifdef TEST_VLAN_FLOWS
int testid = 0x8000;
#define BLOG_ACTIVATE(_blog_p) (testid++)
#define BLOG_DEACTIVATE(_ruleid)
#else
#if defined(CONFIG_BCM_RDPA) || defined(CONFIG_BCM_RDPA_MODULE)
#define VLANCTL_ACTIVATE(_blog_p)   vlanctl_activate(_blog_p, VLANCTL_BIND_CLIENT_RUNNER)
#define VLANCTL_DEACTIVATE(_ruleid) vlanctl_deactivate(_ruleid, VLANCTL_BIND_CLIENT_RUNNER)
#else
#define VLANCTL_ACTIVATE(_blog_p)   vlanctl_activate(_blog_p, VLANCTL_BIND_CLIENT_FAP)
#define VLANCTL_DEACTIVATE(_ruleid) vlanctl_deactivate(_ruleid, VLANCTL_BIND_CLIENT_FAP)
#endif
#endif

#define bcmVlan_dumpBlog(_blog_p) \
    do {if (bcmLog_logIsEnabled(BCM_LOG_ID_VLAN, BCM_LOG_LEVEL_INFO))\
        blog_dump(_blog_p);} while(0)

#define bcmVlan_dumpBlogRule(_blogRule_p)                            \
    do {if (bcmLog_logIsEnabled(BCM_LOG_ID_VLAN, BCM_LOG_LEVEL_INFO))\
        {printk("\n===============================================================\n"); \
        blog_rule_dump(_blogRule_p); printk("\n");}} while(0)


static void cleanup_flowDev_list(void);
static void cleanup_flowPath_list(bcmVlan_flowDev_t *flowDev_p);
static bcmVlan_flowDev_t * find_flowDev(struct net_device *txVlanDev_p);
static bcmVlan_flowDev_t * get_flowDev(struct net_device *txVlanDev_p);
static void free_ruleid_list(bcmVlan_blogRule_id_t *id_p);
static int init_blog_header(struct net_device *dev_p, BlogHeader_t *bh_p);
static bcmVlan_blogRule_id_t * activate_blog_rules(Blog_t *blog_p);
static bcmVlan_flowPath_t * deactivate_blog_rules(bcmVlan_flowDev_t *flowDev_p,
                                                  struct net_device *rxVlanDev_p);


/*
 * vlan flows link list structure is organized as follows:
 *
 * vlanFlows_p->[device]---------------------------------->[device]->NULL
 *                 |                                          |
 *              [path]->[blogRuleId]->[blogRuleId]->NULL   [path]->[blogRuleId]->NULL
 *                 |                                          |
 *              [path]->[blogRuleId]->NULL                 [path]->[blogRuleId]->NULL
 *                 |                                          |
 *              [path]->[blogRuleId]->NULL                  NULL
 *                 |
 *               NULL
 *
 * Notes:
 * - A vlan flow path is from a rx vlan flow device to a tx vlan flow device.
 *   A vlan flow path contains one or more blog rules, each represented by an ID.
 * - A vlan flow device is the in-most vlan device of a vlan path.
 *   For instance, for vlan paths:
 *       eth0 - eth0.1 (eth0.1 is the in-most vlan device)
 *       gpondef - gpon0 - gpon0.0 - veip0 - veip0.1          (veip0.1 is the in-most vlan device)
 *       gpondef - gpon0 - gpon0.0 - veip0 - veip0.1 - ppp0.1 (veip0.1 is the in-most vlan device)
 * - The device node contains the tx vlan device that has one or more flow paths.
 * - The path node contains the rx vlan device that has a path to the tx vlan
 *   device in the device node.
 * - The blogRuleId node contains the blog rule ID. 
 */
static bcmVlan_flowDev_t   *vlanFlows_p         = NULL;

/* Slab caches for each link lists. */
static struct kmem_cache   *vlanFlowDevCache    = NULL;
static struct kmem_cache   *vlanFlowPathCache   = NULL;
static struct kmem_cache   *vlanBlogRuleIdCache = NULL;


/*
 *------------------------------------------------------------------------------
 * Function:
 *   int bcmVlan_flows_init(void)
 * Description:
 *   Create slab caches required for vlan flows link list.
 * Parameters:
 *   void
 * Returns:
 *   errno
 *------------------------------------------------------------------------------
 */
int bcmVlan_flows_init(void)
{
    /* create a slab cache for vlan flow devices */
    vlanFlowDevCache = kmem_cache_create("bcmVlan_flowDev",
                                         sizeof(bcmVlan_flowDev_t),
                                         0, /* align */
                                         0, /* flags */
                                         NULL); /* ctor */
    if (vlanFlowDevCache == NULL)
    {
        BCM_LOG_ERROR(BCM_LOG_ID_VLAN, "Unable to create vlanFlowDevCache");

        return -ENOMEM;
    }

    /* create a slab cache for vlan flow paths */
    vlanFlowPathCache = kmem_cache_create("bcmVlan_flowPath",
                                          sizeof(bcmVlan_flowPath_t),
                                          0, /* align */
                                          0, /* flags */
                                          NULL); /* ctor */
    if (vlanFlowPathCache == NULL)
    {
        BCM_LOG_ERROR(BCM_LOG_ID_VLAN, "Unable to create vlanFlowPathCache");

        kmem_cache_destroy(vlanFlowDevCache);
        
        return -ENOMEM;
    }

    /* create a slab cache for vlan flow blog rule ids */
    vlanBlogRuleIdCache = kmem_cache_create("bcmVlan_blogRule_id",
                                            sizeof(bcmVlan_blogRule_id_t),
                                            0, /* align */
                                            0, /* flags */
                                            NULL); /* ctor */
    if (vlanBlogRuleIdCache == NULL)
    {
        BCM_LOG_ERROR(BCM_LOG_ID_VLAN, "Unable to create vlanBlogRuleIdCache");

        kmem_cache_destroy(vlanFlowDevCache);
        kmem_cache_destroy(vlanFlowPathCache);
        
        return -ENOMEM;
    }

    return 0;

}  /* bcmVlan_flows_init() */

/*
 *------------------------------------------------------------------------------
 * Function:
 *   void bcmVlan_flows_uninit(void)
 * Description:
 *   Destroy slab caches required for vlan flows link list.
 * Parameters:
 *   void
 *------------------------------------------------------------------------------
 */
void bcmVlan_flows_uninit(void)
{
   vlanFlows_p = NULL;

   kmem_cache_destroy(vlanFlowDevCache);
   kmem_cache_destroy(vlanFlowPathCache);
   kmem_cache_destroy(vlanBlogRuleIdCache);
    
}  /* bcmVlan_flows_uninit() */

/*
 *------------------------------------------------------------------------------
 * Function:
 *   bcmVlan_flowDev_t * alloc_flowDev(void)
 * Description:
 *   Allocate a vlan flow device node.
 * Parameters:
 *   void
 * Returns:
 *   pointer to the vlan flow device node.
 *------------------------------------------------------------------------------
 */
static inline bcmVlan_flowDev_t * alloc_flowDev(void)
{
#if defined(CONFIG_SMP) || defined(CONFIG_PREEMPT)
   return kmem_cache_alloc(vlanFlowDevCache, GFP_ATOMIC);
#else
   return kmem_cache_alloc(vlanFlowDevCache, GFP_KERNEL);
#endif
}  /* alloc_flowDev() */

/*
 *------------------------------------------------------------------------------
 * Function:
 *   void free_flowDev(bcmVlan_flowDev_t *free_p)
 * Description:
 *   Allocate a vlan flow device node.
 * Parameters:
 *   free_p: pointer to the node to be freed.
 * Returns:
 *   void
 *------------------------------------------------------------------------------
 */
static inline void free_flowDev(bcmVlan_flowDev_t *free_p)
{
   if (vlanFlows_p == free_p)
      vlanFlows_p = NULL;

   BCM_LOG_DEBUG(BCM_LOG_ID_VLAN, "txDev=%p node=%p",
                 free_p->txDev_p, free_p);
   kmem_cache_free(vlanFlowDevCache, free_p);
}  /* free_flowDev() */

/*
 *------------------------------------------------------------------------------
 * Function:
 *   bcmVlan_flowPath_t * alloc_flowPath(void)
 * Description:
 *   Allocate a vlan flow path node.
 * Parameters:
 *   void
 * Returns:
 *   pointer to the vlan flow path node.
 *------------------------------------------------------------------------------
 */
static inline bcmVlan_flowPath_t * alloc_flowPath(void)
{
#if defined(CONFIG_SMP) || defined(CONFIG_PREEMPT)
   return kmem_cache_alloc(vlanFlowPathCache, GFP_ATOMIC);
#else
   return kmem_cache_alloc(vlanFlowPathCache, GFP_KERNEL);
#endif
}  /* alloc_flowPath() */

/*
 *------------------------------------------------------------------------------
 * Function:
 *   void free_flowPath(bcmVlan_flowPath_t *free_p)
 * Description:
 *   Allocate a vlan flow path node.
 * Parameters:
 *   free_p: pointer to the node to be freed.
 * Returns:
 *   void
 *------------------------------------------------------------------------------
 */
static inline void free_flowPath(bcmVlan_flowPath_t *free_p)
{
   BCM_LOG_DEBUG(BCM_LOG_ID_VLAN, "rxDev=%p node=%p", 
                 free_p->rxDev_p, free_p);
   kmem_cache_free(vlanFlowPathCache, free_p);
}  /* free_flowPath() */

/*
 *------------------------------------------------------------------------------
 * Function:
 *   bcmVlan_blogRule_id_t * alloc_blogRule_id(void)
 * Description:
 *   Allocate a vlan flow path blog rule id node.
 * Parameters:
 *   void
 * Returns:
 *   pointer to the vlan flow path blog rule id node.
 *------------------------------------------------------------------------------
 */
static inline bcmVlan_blogRule_id_t * alloc_blogRule_id(void)
{
#if defined(CONFIG_SMP) || defined(CONFIG_PREEMPT)
   return kmem_cache_alloc(vlanBlogRuleIdCache, GFP_ATOMIC);
#else
   return kmem_cache_alloc(vlanBlogRuleIdCache, GFP_KERNEL);
#endif
}  /* alloc_blogRule_id() */

/*
 *------------------------------------------------------------------------------
 * Function:
 *   void free_blogRule_id(bcmVlan_blogRule_id_t *free_p)
 * Description:
 *   Allocate a vlan flow path blog rule id node.
 * Parameters:
 *   free_p: pointer to the node to be freed.
 * Returns:
 *   void
 *------------------------------------------------------------------------------
 */
static inline void free_blogRule_id(bcmVlan_blogRule_id_t *free_p)
{
   BCM_LOG_DEBUG(BCM_LOG_ID_VLAN, "id=0x%x node=%p",
                 free_p->id, free_p);
   kmem_cache_free(vlanBlogRuleIdCache, free_p);
}  /* free_blogRule_id() */

/*
 *------------------------------------------------------------------------------
 * Function:
 *   void cleanup_flowDev_list(void)
 * Description:
 *   Clean up flow device nodes that have no flow path.
 * Parameters:
 *   void
 * Returns:
 *   void
 *------------------------------------------------------------------------------
 */
void cleanup_flowDev_list(void)
{
   bcmVlan_flowDev_t *prevFlowdev_p = NULL;
   bcmVlan_flowDev_t *flowDev_p     = vlanFlows_p;
   
   while (flowDev_p != NULL)
   {
      if (flowDev_p->flowPath_p == NULL)
      {
         if (flowDev_p == vlanFlows_p)
         {
            /* the first node in list */
            vlanFlows_p = flowDev_p->next_p;
            free_flowDev(flowDev_p);
            flowDev_p = vlanFlows_p;
         }
         else
         {
            prevFlowdev_p->next_p = flowDev_p->next_p;
            free_flowDev(flowDev_p);
            flowDev_p = prevFlowdev_p->next_p;
         }
      }
      else
      {
         prevFlowdev_p = flowDev_p;
         flowDev_p     = flowDev_p->next_p; 
      }
   }
}  /* cleanup_flowDev_list() */

/*
 *------------------------------------------------------------------------------
 * Function:
 *   void cleanup_flowPath_list(bcmVlan_flowDev_t *flowDev_p)
 * Description:
 *   Clean up device flow paths that have no blog rules.
 * Parameters:
 *   flowDev_p: pointer to the flow device node.
 * Returns:
 *   void
 *------------------------------------------------------------------------------
 */
void cleanup_flowPath_list(bcmVlan_flowDev_t *flowDev_p)
{
   if (flowDev_p != NULL)
   {
      bcmVlan_flowPath_t *prevPath_p = NULL;
      bcmVlan_flowPath_t *path_p     = flowDev_p->flowPath_p;
   
      while (path_p != NULL)
      {
         if (path_p->blogRuleId_p == NULL)
         {
            if (path_p == flowDev_p->flowPath_p)
            {
               /* the first node in list */
               flowDev_p->flowPath_p = path_p->next_p;
               free_flowPath(path_p);
               path_p = flowDev_p->flowPath_p;
            }
            else
            {
               prevPath_p->next_p = path_p->next_p;
               free_flowPath(path_p);
               path_p = prevPath_p->next_p;
            }
         }
         else
         {
            prevPath_p = path_p;
            path_p     = path_p->next_p; 
         }
      }
   }
}  /* cleanup_flowPath_list() */

/*
 *------------------------------------------------------------------------------
 * Function:
 *   bcmVlan_flowDev_t * find_flowDev(struct net_device *txVlanDev_p)
 * Description:
 *   Find the flow device node of a tx vlan device.
 * Parameters:
 *   txVlanDev_p: pointer to the tx Vlan device.
 * Returns:
 *   pointer to the flow device node
 *------------------------------------------------------------------------------
 */
bcmVlan_flowDev_t * find_flowDev(struct net_device *txVlanDev_p)
{
   bcmVlan_flowDev_t *flowDev_p;
   
   for (flowDev_p = vlanFlows_p; flowDev_p; flowDev_p = flowDev_p->next_p)
   {
      if (flowDev_p->txDev_p == txVlanDev_p)
         break;
   }

   return flowDev_p;   
      
}  /* find_flowDev() */

/*
 *------------------------------------------------------------------------------
 * Function:
 *   bcmVlan_flowDev_t * get_flowDev(struct net_device *txVlanDev_p)
 * Description:
 *   Find the flow device node of a tx vlan device. If not found, allocate
 *   a new node.
 * Parameters:
 *   txVlanDev_p: pointer to the tx Vlan device.
 * Returns:
 *   pointer to the flow device node
 *------------------------------------------------------------------------------
 */
bcmVlan_flowDev_t * get_flowDev(struct net_device *txVlanDev_p)
{
   bcmVlan_flowDev_t *flowDev_p;
   
   /* if there is no flowdev for this txVlanDev, allocate one. */
   flowDev_p = find_flowDev(txVlanDev_p);
   if (flowDev_p == NULL)
   {
      /* flowdev does not exist. create one. */
      flowDev_p = alloc_flowDev();
      if (flowDev_p == NULL)  
      {
         BCM_LOG_ERROR(BCM_LOG_ID_VLAN, "Failed to allocate vlanFlowDev node");
      }
      else
      {
         flowDev_p->txDev_p    = txVlanDev_p;
         flowDev_p->flowPath_p = NULL;
         flowDev_p->next_p     = vlanFlows_p;
         
         vlanFlows_p = flowDev_p;
         
         BCM_LOG_DEBUG(BCM_LOG_ID_VLAN, "txDev=%p node=%p",
                flowDev_p->txDev_p, flowDev_p);
      }
   }

   return flowDev_p;   
      
}  /* get_flowDev() */

/*
 *------------------------------------------------------------------------------
 * Function:
 *   void free_ruleid_list(bcmVlan_blogRule_id_t *id_p)
 * Description:
 *   Free a blog rule id list.
 * Parameters:
 *   id_p (input): pointer to the blog rule id list.
 *------------------------------------------------------------------------------
 */
void free_ruleid_list(bcmVlan_blogRule_id_t *id_p)
{
   bcmVlan_blogRule_id_t *nextid_p;
   
   while (id_p != NULL)
   {
      nextid_p = id_p->next_p;
      free_blogRule_id(id_p);
      id_p = nextid_p;
   }
}  /* free_ruleid_list() */

/*
 *------------------------------------------------------------------------------
 * Function:
 *   struct net_device * find_vlan_device(struct net_device *dev_p)
 * Description:
 *   Find the first vlan device from the leaf device node to the
 *   root device node. If there is no vlan device, the root device
 *   will be returned.
 * Parameters:
 *   dev_p:  The leaf device node of a vlan device tree.
 * Returns:
 *   pointer the device found.
 *------------------------------------------------------------------------------
 */
struct net_device * find_vlan_device(struct net_device *dev_p)
{
   while (1)
   {
      BCM_LOG_DEBUG(BCM_LOG_ID_VLAN, "name=%s priv_flags=0x%x",
                    dev_p->name, (unsigned int)dev_p->priv_flags);
                    
      if (netdev_path_is_root(dev_p))
         break;

      /* Only look for vlan device. "pppx.y" devices are flagged as (IFF_PPP | IFF_WANDEV),
       * Therefore they are not VLAN DEVICE and will be skipped.
       */
      if (dev_p->priv_flags & IFF_BCM_VLAN)
         break;
      
      dev_p = netdev_path_next_dev(dev_p);
   }
   
   return dev_p;
   
}  /* find_vlan_device() */

/*
 *------------------------------------------------------------------------------
 * Function:
 *   int init_blog_header(struct net_device *dev_p, BlogHeader_t *bh_p)
 * Description:
 *   Initialize the blog header data structure of a blog for
 *   the given device (dev_p).
 * Parameters:
 *   dev_p  (input): pointer to net device.
 *   bh_p   (input): pointer to the blog header data structure.
 * Returns:
 *   0:  succeeded.
 *   -1: failed.
 *------------------------------------------------------------------------------
 */
int init_blog_header(struct net_device *dev_p, BlogHeader_t *bh_p)
{
   int ret = 0;
   uint32_t phyType;

   /* find the root device */
   dev_p = netdev_path_get_root(dev_p);

   phyType = netdev_path_get_hw_port_type(dev_p);
      
   bh_p->info.phyHdr = phyType & BLOG_PHYHDR_MASK;

   bh_p->info.phyHdrType = BLOG_GET_PHYTYPE(phyType);
     
   switch (bh_p->info.phyHdrType)
   {
      case BLOG_ENETPHY:
         bh_p->info.channel = netdev_path_get_hw_port(dev_p);
         bh_p->info.bmap.BCM_SWC = 1;
      break;
      
      case BLOG_XTMPHY:
         bh_p->info.channel = netdev_path_get_hw_port(dev_p);
         bh_p->info.bmap.BCM_XPHY = 1;
      break;

      case BLOG_GPONPHY:
      case BLOG_EPONPHY:
      case BLOG_SIDPHY:
         bh_p->info.channel = netdev_path_get_hw_port(dev_p);  /* ????? */
         bh_p->info.bmap.BCM_SWC = 1;
      break;
            
      default:
		   BCM_LOG_ERROR(BCM_LOG_ID_VLAN, "phyHdrType %d is not supported",
                       bh_p->info.phyHdrType);
         ret = -1;
      break;
   }

   return ret;
   
}  /* init_blog_header() */

/*
 *------------------------------------------------------------------------------
 * Function:
 *   bcmVlan_blogRule_id_t * activate_blog_rules(Blog_t *blog_p)
 * Description:
 *   Activate blog rules of a layer2 flow blog.
 * Parameters:
 *   blog_p (input): pointer to the layer2 flow blog.
 * Returns:
 *   The list of activated blog rule ids.
 *------------------------------------------------------------------------------
 */
bcmVlan_blogRule_id_t * activate_blog_rules(Blog_t *blog_p)
{
   Blog_t                  *new_blog_p;
   blogRule_t              *rule_p      = NULL;
   blogRule_t              *n_rule_p    = NULL;
   blogRuleFilter_t        *rule_filter = NULL;
   bcmVlan_blogRule_id_t   *ruleId_p    = NULL;
   bcmVlan_blogRule_id_t   *id_p        = NULL;
   uint32_t                vid          = 0;
   uint32_t                key;

   if (!blog_p || !blog_p->blogRule_p)
      return NULL;

	new_blog_p = blog_get();
	if (new_blog_p == BLOG_NULL)
   {
		BCM_LOG_ERROR(BCM_LOG_ID_VLAN, "new_blog_p allocation failed");
		return NULL;
	}

   /* get a copy of blog_p */
   blog_copy(new_blog_p, blog_p);

   bcmVlan_dumpBlog(blog_p);

   /* activate blog rules one at a time */
   for (rule_p = blog_p->blogRule_p; rule_p; rule_p = rule_p->next_p)
   {
      /* allocate a rule id node */
	   id_p = alloc_blogRule_id();
      if (id_p == NULL)
      {
		 BCM_LOG_ERROR(BCM_LOG_ID_VLAN, "ruleid_p allocation failed");
         break;
      }

      /* save pointer to the next blog rule */      
      n_rule_p = rule_p->next_p;
      
      /* terminate the current blog rule node */
      rule_p->next_p = NULL;

      /* assign the blog rule to the new blog */
      new_blog_p->blogRule_p = rule_p;

      /* update vlan tag info of the new blog based on the blog rule */
      rule_filter = &(((blogRule_t *)new_blog_p->blogRule_p)->filter);
      new_blog_p->vtag_num = rule_filter->nbrOfVlanTags;
      vid = ((rule_filter->vlan[0].value.h_vlan_TCI &
              rule_filter->vlan[0].mask.h_vlan_TCI) & 0xFFF);
      new_blog_p->vtag[0] = vid ? vid : 0xFFFF; 
      vid = ((rule_filter->vlan[1].value.h_vlan_TCI &
              rule_filter->vlan[1].mask.h_vlan_TCI) & 0xFFF);
      new_blog_p->vtag[1] = vid ? vid : 0xFFFF;

      /* activate the new blog */
      key = VLANCTL_ACTIVATE(new_blog_p);
      if (key == BLOG_KEY_INVALID)
      {
         /* Some flows can be rejected. use these prints only for debugging! */
         BCM_LOG_DEBUG(BCM_LOG_ID_VLAN, "blog_activate failed!");
         bcmVlan_dumpBlogRule(rule_p);

         free_blogRule_id(id_p);
      }
      else
      {
         BCM_LOG_DEBUG(BCM_LOG_ID_VLAN, "blog_activate succeeded. id=0x%x", key);
         bcmVlan_dumpBlogRule(rule_p);
         
         /* save the blog rule activation key */
         id_p->id     = key;
         id_p->next_p = ruleId_p;
         ruleId_p     = id_p;
      }

      /* restore pointer to the next blog rule */      
      rule_p->next_p = n_rule_p;
   }

   /* free the new blog */   
   blog_put(new_blog_p);
   
   BCM_LOG_DEBUG(BCM_LOG_ID_VLAN, "return ruleId_p=%p", ruleId_p);
   
   return ruleId_p;
   
} /* activate_blog_rules() */

/*
 *------------------------------------------------------------------------------
 * Function:
 *   bcmVlan_flowPath_t * deactivate_blog_rules(bcmVlan_flowDev_t *flowDev_p,
 *                                              struct net_device *rxVlanDev_p)
 * Description:
 *   Deactivate blog rules associated with a layer2 flow path.
 * Parameters:
 *   flowDev_p (input): pointer to the flow device node.
 *   rxVlanDev_p (input): the rx vlan device.
 * Returns:
 *   pointer to the flow path if found.
 *   NULL if flow path not found.
 *------------------------------------------------------------------------------
 */
bcmVlan_flowPath_t * deactivate_blog_rules(bcmVlan_flowDev_t *flowDev_p,
                                           struct net_device *rxVlanDev_p)
{
   bcmVlan_flowPath_t      *path_p = flowDev_p->flowPath_p;
   bcmVlan_blogRule_id_t   *id_p;
   
   BCM_LOG_DEBUG(BCM_LOG_ID_VLAN, "rxVlanDev=%p flowDev_p=%p",
                 rxVlanDev_p, flowDev_p);
   
   while (path_p != NULL)
   {
      /* deactivate all blog rules of the flow path rxVlanDev to txVlanDev
       * in flowdev. if rxVlanDev_p is NULL, then deactivate blog rules of
       * all the flow paths going from any rxVlanDev to txVlanDev in flowdev.
       */
      if (rxVlanDev_p == NULL || rxVlanDev_p == path_p->rxDev_p)
      {
         BCM_LOG_DEBUG(BCM_LOG_ID_VLAN, "path_p=%p", path_p);
   
         /* found the existing flow path. Deactivate all the old blog rules. */
         id_p = path_p->blogRuleId_p;
         
         while (id_p != NULL)
         {
            /* deactivate vlanctl rule */
            BCM_LOG_DEBUG(BCM_LOG_ID_VLAN, "rule id 0x%x", id_p->id);

            VLANCTL_DEACTIVATE(id_p->id);
            id_p = id_p->next_p;
         }
         
         free_ruleid_list(path_p->blogRuleId_p);
         path_p->blogRuleId_p = NULL;
         
         if (rxVlanDev_p != NULL)
            break;
      }
      path_p = path_p->next_p;
   }
  
   return path_p;
   
}  /* deactivate_blog_rules() */

/*
 *------------------------------------------------------------------------------
 * Function:
 *   int bcmVlan_flowPath_create(struct net_device *rxDev_p,
 *                               struct net_device *txDev_p)
 * Description:
 *   Generate and activate blog rules for a layer2 flow path going
 *   from the rx vlan device to the tx vlan device.
 * Parameters:
 *   rxVlanDev_p (input): rx vlan flow device 
 *   txVlanDev_p (input): tx vlan flow device 
 * Returns:
 *   0:  succeeded
 *   -1 or -EINVAL: failed
 *------------------------------------------------------------------------------
 */
int bcmVlan_flowPath_create(struct net_device *rxDev_p,
                            struct net_device *txDev_p)
{
   struct net_device       *rxVlanDev_p = NULL;
   struct net_device       *txVlanDev_p = NULL;
   Blog_t                  *blog_p      = BLOG_NULL;
   bcmVlan_flowDev_t       *flowDev_p   = NULL;
   bcmVlan_flowPath_t      *path_p      = NULL;
   bcmVlan_blogRule_id_t   *newRuleId_p = NULL;
   
   int ret = 0;

   if (blogRuleVlanHook == NULL) 
   {
      BCM_LOG_ERROR(BCM_LOG_ID_VLAN, "Blog rule generation is not enabled.");
      return -EPERM;
   }

   BCM_LOG_DEBUG(BCM_LOG_ID_VLAN, "rxDev=%p txDev=%p",
                 rxDev_p, txDev_p);
          
   if (rxDev_p == NULL || txDev_p == NULL)
	{
      BCM_LOG_ERROR(BCM_LOG_ID_VLAN, "rx or tx device not specified");
      return -EINVAL;
   }

   /* find the in-most vlan device */
   rxVlanDev_p = find_vlan_device(rxDev_p);
   txVlanDev_p = find_vlan_device(txDev_p);

   if (netdev_path_is_root(rxVlanDev_p) && netdev_path_is_root(txVlanDev_p))
   {
      BCM_LOG_ERROR(BCM_LOG_ID_VLAN, "rx and tx devices cannot both be root device");
      return -EINVAL;
   }
   
   BCM_LOG_DEBUG(BCM_LOG_ID_VLAN, "rxVlanDev=%p txVlanDev=%p",
                 rxVlanDev_p, txVlanDev_p);
          
   /* allocate blog */
   blog_p = blog_get();
   if (blog_p == BLOG_NULL) 
   {
      BCM_LOG_ERROR(BCM_LOG_ID_VLAN, "blog_p allocation failed");
      return -ENOMEM;
   }

   /* save the leaf vlan device */
	blog_p->rx_dev_p = rxVlanDev_p;
 
   /* initialize the blog header for the rx vlan device */
   if (init_blog_header(rxVlanDev_p, &(blog_p->rx)) != 0)
   {
      BCM_LOG_ERROR(BCM_LOG_ID_VLAN, "init_blog_header for rxVlanDev_p failed");
      blog_put(blog_p);
      return -EPERM;
   }
   
   /* save the leaf vlan device */
	blog_p->tx_dev_p = txVlanDev_p;

   /* initialize the blog header for the tx vlan device */
   if (init_blog_header(txVlanDev_p, &(blog_p->tx)) != 0)
   {
      BCM_LOG_ERROR(BCM_LOG_ID_VLAN, "init_blog_header for txVlanDev_p failed");
      blog_put(blog_p);
      return -EPERM;
   }

   blog_p->mark = blog_p->priority = 0;

   //????   
//   blog_p->key.l1_tuple.phy     = blog_p->rx.info.phyHdr;
//   blog_p->key.l1_tuple.channel = blog_p->rx.info.channel;
//   blog_p->key.protocol         = BLOG_IPPROTO_UDP;

   blog_p->blogRule_p = NULL;

   /* add vlan blog rules, if any vlan interfaces were found */
   if (blogRuleVlanHook(blog_p,
                        (netdev_path_is_root(rxVlanDev_p))? NULL : rxVlanDev_p,
                        (netdev_path_is_root(txVlanDev_p))? NULL : txVlanDev_p) < 0)
   {
      BCM_LOG_ERROR(BCM_LOG_ID_VLAN, "Error while processing VLAN blog rules");
      
      blog_rule_free_list(blog_p);
      blog_put(blog_p);
      return -EPERM;
   }

#if defined(CONFIG_BCM_RDPA) || defined(CONFIG_BCM_RDPA_MODULE)
   /* find the flowdev node of txVlanDev */
   flowDev_p = get_flowDev(txVlanDev_p);
   if (flowDev_p == NULL)  
   {
      BCM_LOG_ERROR(BCM_LOG_ID_VLAN, "Failed to allocate vlanFlowDev node for txVlanDev");
      
      blog_rule_free_list(blog_p);
      blog_put(blog_p);
      return -ENOMEM;
   }
   
   /* Runner must remove old rule then create new */
   /* deactivate the old blog rules of the same flow path. */
   path_p = deactivate_blog_rules(flowDev_p, rxVlanDev_p);

   /* activate new blog rules for flow path rxVlanDev -> txVlanDev */
   newRuleId_p = activate_blog_rules(blog_p);

   /* blog rule and blog are no longer needed. free them. */
   blog_rule_free_list(blog_p);
   blog_put(blog_p);
#else
   /* activate new blog rules for flow path rxVlanDev -> txVlanDev */
   newRuleId_p = activate_blog_rules(blog_p);

   /* blog rule and blog are no longer needed. free them. */
   blog_rule_free_list(blog_p);
   blog_put(blog_p);

   /* find the flowdev node of txVlanDev */
   flowDev_p = get_flowDev(txVlanDev_p);
   if (flowDev_p == NULL)  
   {
      BCM_LOG_ERROR(BCM_LOG_ID_VLAN, "Failed to allocate vlanFlowDev node for txVlanDev");
      
      free_ruleid_list(newRuleId_p);
      return -ENOMEM;
   }
   
   /* deactivate the old blog rules of the same flow path. */
   path_p = deactivate_blog_rules(flowDev_p, rxVlanDev_p);
#endif
   if (path_p == NULL)
   {
      /* did not find the old blog rule id list for flow path
       * rxVlanDev -> txVlanDev. Allocate a flow path for the
       * newly activated blog rule id list.
       */
      path_p = alloc_flowPath();
      BCM_LOG_DEBUG(BCM_LOG_ID_VLAN, "alloc new path_p=%p", path_p);
      if (path_p == NULL)  
      {
         BCM_LOG_ERROR(BCM_LOG_ID_VLAN, "Failed to allocate vlanFlowPath node");
         
         free_ruleid_list(newRuleId_p);
         free_flowDev(flowDev_p);
         return -ENOMEM;
      }

      path_p->rxDev_p       = rxVlanDev_p;
      path_p->next_p        = flowDev_p->flowPath_p;
      flowDev_p->flowPath_p = path_p;
   }
   
   /* save the newly activated blog rule id list */
   path_p->blogRuleId_p = newRuleId_p;

   BCM_LOG_DEBUG(BCM_LOG_ID_VLAN, "flowDev_p=%p flowPath_p=%p blogRuleId_p=%p",
                 flowDev_p, flowDev_p->flowPath_p,
                 flowDev_p->flowPath_p->blogRuleId_p);
   BCM_LOG_DEBUG(BCM_LOG_ID_VLAN, "ruleId:");
   while (newRuleId_p != NULL)
   {
      BCM_LOG_DEBUG(BCM_LOG_ID_VLAN, " 0x%x", newRuleId_p->id);
      newRuleId_p = newRuleId_p->next_p;
   }   
          
   return ret;
    
}  /* bcmVlan_flowPath_create() */

/*
 *------------------------------------------------------------------------------
 * Function:
 *   int bcmVlan_flowPath_delete(struct net_device *rxDev_p,
 *                               struct net_device *txDev_p)
 * Description:
 *   Deactivate blog rules for the flow path going from
 *   rxDev_p to txDev_p. Either rxDev_p or txDev_p can
 *   be NULL, but not both.
 *   If rxDev_p is NULL, all blog rules for flow paths
 *   going from any rx vlan devices to txDev_p will be
 *   deactivated.
 *   If txDev_p is NULL, all blog rules for flow paths
 *   going from rxDev_p to any tx vlan devices will be
 *   deactivated.
 * Parameters:
 *   rxDev_p (input): rx vlan flow device 
 *   txDev_p (input): tx vlan flow device 
 * Returns:
 *   0:  succeeded
 *   -EINVAL: failed
 *------------------------------------------------------------------------------
 */
int bcmVlan_flowPath_delete(struct net_device *rxDev_p,
                            struct net_device *txDev_p)
{
   struct net_device *rxVlanDev_p = NULL;
   struct net_device *txVlanDev_p = NULL;
   bcmVlan_flowDev_t *flowDev_p;
   
   BCM_LOG_DEBUG(BCM_LOG_ID_VLAN, "rxDev=%p txDev=%p",
                 rxDev_p, txDev_p);
          
   if (rxDev_p == NULL && txDev_p == NULL)
	{
      BCM_LOG_ERROR(BCM_LOG_ID_VLAN, "both rx and tx devices not specified");
      return -EINVAL;
   }
   
   rxVlanDev_p = rxDev_p;
   if (rxDev_p != NULL)
   {
      /* find the in-most vlan device */
      rxVlanDev_p = find_vlan_device(rxDev_p);
   }
   
   txVlanDev_p = txDev_p;
   if (txDev_p != NULL)
   {
      /* find the in-most vlan device */
      txVlanDev_p = find_vlan_device(txDev_p);
   }
   
   BCM_LOG_DEBUG(BCM_LOG_ID_VLAN, "rxVlanDev=%p txVlanDev=%p",
                 rxVlanDev_p, txVlanDev_p);
          
   /* deactivate all blog rules of the flow path rxVlanDev to txVlanDev.
    * if txVlanDev_p is NULL, then deactivate blog rules of all the flow
    * paths going from rxVlanDev to any txVlanDev.
    */
   for (flowDev_p = vlanFlows_p; flowDev_p; flowDev_p = flowDev_p->next_p)
   {
      if (txVlanDev_p == NULL || flowDev_p->txDev_p == txVlanDev_p)
      {
         /* deactivate all the blog rules of the flow path. */
         deactivate_blog_rules(flowDev_p, rxVlanDev_p);
   
         /* now, clean up flow paths that do not have any blog rule */
         cleanup_flowPath_list(flowDev_p);
      
         if (txVlanDev_p != NULL)
            break;
      }
   }
   
   /* now, clean up flow device nodes that do not have any flow path */
   cleanup_flowDev_list();
     
   return 0;
      
}  /* bcmVlan_flowPath_delete() */

/*
 *------------------------------------------------------------------------------
 * Function:
 *   int bcmVlan_flowDev_delete(struct net_device *dev_p)
 * Description:
 *   Delete a vlan flow device and all the flow paths associated
 *   with it.
 * Parameters:
 *   dev_p (input): vlan flow device 
 * Returns:
 *   errno
 *------------------------------------------------------------------------------
 */
int bcmVlan_flowDev_delete(struct net_device *dev_p)
{
   int ret;

   BCM_LOG_DEBUG(BCM_LOG_ID_VLAN, "dev_p %p", dev_p);
   
   /* first delete all the flow paths that receive from any device and
    * transmit out of dev_p.
    */   
   ret = bcmVlan_flowPath_delete(NULL, dev_p);
   if (ret == 0)
   {
      /* then delete all the flow paths that receive from dev_p and
       * transmit out of any device. */   
      ret = bcmVlan_flowPath_delete(dev_p, NULL);
   }
   
   return ret;
   
}  /* bcmVlan_flowDev_delete() */

#else

int bcmVlan_flows_init(void)
{
   return -EPERM;
   
}  /* bcmVlan_flows_init() */

void bcmVlan_flows_uninit(void)
{
   return;
   
}  /* bcmVlan_flows_uninit() */

int bcmVlan_flowPath_create(struct net_device *rxDev_p,
                            struct net_device *txDev_p)
{
   return -EPERM;
   
}  /* bcmVlan_flowPath_create() */

int bcmVlan_flowPath_delete(struct net_device *rxDev_p,
                            struct net_device *txDev_p)
{
   return -EPERM;
   
}  /* bcmVlan_flowPath_delete() */

int bcmVlan_flowDev_delete(struct net_device *dev_p)
{
   return -EPERM;
   
}  /* bcmVlan_flowDev_delete() */

#endif
