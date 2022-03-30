/*
<:copyright-BRCM:2013:DUAL/GPL:standard

   Copyright (c) 2013 Broadcom 
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

#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/rculist.h>
#include <linux/spinlock.h>
#include <linux/times.h>
#include <linux/netdevice.h>
#if defined(CONFIG_BLOG)
#include <linux/blog.h>
#endif
#include "br_private.h"
#include "bcm_br_hooks_maclimit.h"
#if defined(CONFIG_BCM_WLAN_MODULE)
#include <linux/bcm_log.h>
#endif

/*copyied function from br_fdb.c */
static inline unsigned long hold_time(const struct net_bridge *br)
{
	/*TODO check why 4.1 kerenl is return forward delay as 15 secs
    * instead of forward_delay
    */ 
	return br->topology_change ? br->forward_delay : br->ageing_time;
}

int bcm_br_has_fdb_expired(const struct net_bridge *br,
				  const struct net_bridge_fdb_entry *fdb)
{

	if(!fdb->is_static && !fdb->added_by_external_learn){

		/*query accelerator */
#if defined(CONFIG_BLOG)
		blog_lock();
		if (fdb->blog_fdb_key != BLOG_FDB_KEY_INVALID)
			blog_query(QUERY_BRIDGEFDB, (void*)fdb, fdb->blog_fdb_key, 0, 0);
		blog_unlock();
#endif

		return (time_before_eq(fdb->updated + hold_time(br), jiffies));
	}

	return 0;
}

int bcm_br_fdb_notify(struct net_bridge *br,
		       const struct net_bridge_fdb_entry *fdb, int type,
		       bool swdev_notify)
{

	if(type == RTM_DELNEIGH)
	{
		/* note for update you will not get a RTM_DELNEIGH, so we have to explicty
		 * flush in bcm_br_fdb_update
		 */
#if defined(CONFIG_BLOG)
		blog_lock();
		blog_notify_async(DESTROY_BRIDGEFDB, (void*)fdb, fdb->blog_fdb_key, 0, NULL, NULL);
		blog_unlock();
#endif /* CONFIG_BLOG */
	}

	bcm_mac_limit_learning_notify(br, fdb, type);
	return 0;
}

int bcm_br_fdb_init(struct net_bridge_fdb_entry *fdb)
{
#if defined(CONFIG_BLOG)
		fdb->blog_fdb_key = BLOG_FDB_KEY_INVALID;
#endif
		return 0;
}

int bcm_br_fdb_fill_info(const struct net_bridge_fdb_entry *fdb)
{
#if defined(CONFIG_BLOG)
	blog_lock();
	if (fdb->blog_fdb_key != BLOG_FDB_KEY_INVALID)
		blog_query(QUERY_BRIDGEFDB, (void*)fdb, fdb->blog_fdb_key, 0, 0);
	blog_unlock();
#endif

	return 0;
}

int bcm_br_fdb_update(struct net_bridge_fdb_entry *fdb,struct net_bridge_port *source)
{
	if (unlikely(source != fdb->dst)) {
		/*flush existing entries in accelerator */

#if defined(CONFIG_BLOG)
		blog_lock();
		blog_notify_async(DESTROY_BRIDGEFDB, (void*)fdb, fdb->blog_fdb_key, 0, NULL, NULL);
		blog_unlock();
#endif /* CONFIG_BLOG */
#if defined(CONFIG_BCM_WLAN_MODULE)
{
		bcmFun_t *bcm_wl_update_bridgefdb = bcmFun_get(BCM_FUN_ID_WLAN_UPDATE_BRIDGEFDB);
		if (bcm_wl_update_bridgefdb != NULL)
			bcm_wl_update_bridgefdb((void *)fdb);
}
#endif
	}

	bcm_mac_limit_learning_update(fdb, source);

	return 0;
}

int bcm_br_fdb_cleanup(struct net_bridge_fdb_entry *fdb, 
                       unsigned long time_now, unsigned long delay)
{
	/* peform any check and set the fdb->updated to proper timeout
	 * if we want to keep the fdb instead on deleting
	 */
    unsigned long this_timer = fdb->updated + delay;
		
    if (time_before_eq(this_timer, time_now)) 
    {
        int flag = 0;
		
#if defined(CONFIG_BLOG)
        blog_lock();
        if (fdb->blog_fdb_key != BLOG_FDB_KEY_INVALID)
            blog_query(QUERY_BRIDGEFDB, (void*)fdb, fdb->blog_fdb_key, 0, 0);
        blog_unlock();
#endif

#if (defined(CONFIG_BCM_RDPA) || defined(CONFIG_BCM_RDPA_MODULE)) && (defined(CONFIG_BCM_RDPA_BRIDGE) || defined(CONFIG_BCM_RDPA_BRIDGE_MODULE))	
        if (fdb->dst)
            br_fp_hook(fdb->dst->br->dev, BR_FP_FDB_CHECK_AGE, fdb, &flag);
#endif /* CONFIG_BCM_RDPA && CONFIG_BCM_RDPA_BRIDGE && CONFIG_BCM_RDPA_BRIDGE_MODULE */

#if defined(CONFIG_BCM_WLAN_MODULE)
        if (flag == 0)
        {
            bcmFun_t *bcm_wl_query_bridgefdb = bcmFun_get(BCM_FUN_ID_WLAN_QUERY_BRIDGEFDB);
            if (bcm_wl_query_bridgefdb != NULL)
                flag = bcm_wl_query_bridgefdb((void *)fdb);
        }
#endif /* CONFIG_BCM_WLAN_MODULE */

        if (flag) /* The FDB entry was updated */
            fdb->updated = jiffies;
    }

    return 0;
}
