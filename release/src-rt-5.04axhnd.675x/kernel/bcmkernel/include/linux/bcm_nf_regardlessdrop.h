/*
* <:copyright-BRCM:2012:DUAL/GPL:standard
* 
*    Copyright (c) 2012 Broadcom 
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

#ifndef _BCM_NF_REGARDLESSDROP_
#define _BCM_NF_REGARDLESSDROP_

#define NF_CT_SAFE_LISTS_MAX 6
/* regardless drop of connection when conntrack table is full */
struct safe_list {
	spinlock_t lock;
	struct list_head low;          /* low priority linux only */
	struct list_head low_sw_accel; /* low priority sw acceleartor */
	struct list_head low_hw_accel; /* low priority hw accelerator */
	struct list_head hi;           /* high priority linux only */
	struct list_head hi_sw_accel;  /* high priority sw acceleartor */
	struct list_head hi_hw_accel;  /* high priority hw accelerator */
    struct list_head *drop_list_order[NF_CT_SAFE_LISTS_MAX]; /*order in which lists are checked to drop a connection */
};

struct safe_list ct_safe_lists = {
	.lock 		     = __SPIN_LOCK_UNLOCKED(ct_safe_lists.lock),
	.low		     = LIST_HEAD_INIT(ct_safe_lists.low),
	.low_sw_accel    = LIST_HEAD_INIT(ct_safe_lists.low_sw_accel),
	.low_hw_accel    = LIST_HEAD_INIT(ct_safe_lists.low_hw_accel),
	.hi			     = LIST_HEAD_INIT(ct_safe_lists.hi),
	.hi_sw_accel     = LIST_HEAD_INIT(ct_safe_lists.hi_sw_accel),
	.hi_hw_accel     = LIST_HEAD_INIT(ct_safe_lists.hi_hw_accel),
#if CONFIG_BCM_NETFILTER_REGARDLESS_DROP_ORDER == 1
    /* when trying to find a drop candidate search safe_list's in the order of
     * non-accelerated-->sw_accelerated-->hw_accelerated
     * this is the default policy
     */
    .drop_list_order = {&ct_safe_lists.low, &ct_safe_lists.hi,
                        &ct_safe_lists.low_sw_accel,&ct_safe_lists.hi_sw_accel,
                        &ct_safe_lists.low_hw_accel, &ct_safe_lists.hi_hw_accel}
#elif CONFIG_BCM_NETFILTER_REGARDLESS_DROP_ORDER == 2
    .drop_list_order = {&ct_safe_lists.low, &ct_safe_lists.low_sw_accel,
                        &ct_safe_lists.hi, &ct_safe_lists.hi_sw_accel,
                        &ct_safe_lists.low_hw_accel, &ct_safe_lists.hi_hw_accel}
#elif CONFIG_BCM_NETFILTER_REGARDLESS_DROP_ORDER == 3
    .drop_list_order = {&ct_safe_lists.low, &ct_safe_lists.low_sw_accel,
                        &ct_safe_lists.low_hw_accel, &ct_safe_lists.hi,
                        &ct_safe_lists.hi_sw_accel, &ct_safe_lists.hi_hw_accel}
#else
#error "Netfilter Regardless Drop Order is not set"
#endif
};


static inline void ct_set_curr_safe_list(struct nf_conn *ct)
{
	/*first try to move to HW list */
	if(ct->bcm_ext.hw_accel_flows) {
#if IS_ENABLED(CONFIG_BCM_INGQOS)
		if (ct->bcm_ext.iq_prio == IQOS_PRIO_HIGH)
			ct->bcm_ext.curr_safe_list = &ct_safe_lists.hi_hw_accel;
		else
#endif
			ct->bcm_ext.curr_safe_list = &ct_safe_lists.low_hw_accel;
	}else if(ct->bcm_ext.sw_accel_flows) {
#if IS_ENABLED(CONFIG_BCM_INGQOS)
		if (ct->bcm_ext.iq_prio == IQOS_PRIO_HIGH)
			ct->bcm_ext.curr_safe_list = &ct_safe_lists.hi_sw_accel;
		else
#endif
			ct->bcm_ext.curr_safe_list = &ct_safe_lists.low_sw_accel;
	}else{
		/*move to SW only list */
#if IS_ENABLED(CONFIG_BCM_INGQOS)
		if (ct->bcm_ext.iq_prio == IQOS_PRIO_HIGH)
			ct->bcm_ext.curr_safe_list = &ct_safe_lists.hi;
		else
#endif
			ct->bcm_ext.curr_safe_list = &ct_safe_lists.low;
	}
}

static inline void ct_safe_list_add_tail(struct nf_conn *ct)
{
	spin_lock_bh(&ct_safe_lists.lock);

	ct->bcm_ext.hw_accel_flows = 0;
	ct->bcm_ext.sw_accel_flows = 0;

	ct_set_curr_safe_list(ct);

	list_add_tail(&ct->bcm_ext.safe_list, ct->bcm_ext.curr_safe_list);

	spin_unlock_bh(&ct_safe_lists.lock);
}

static inline void ct_safe_list_move_tail(struct nf_conn *ct)
{
	spin_lock_bh(&ct_safe_lists.lock);

	list_move_tail(&ct->bcm_ext.safe_list, ct->bcm_ext.curr_safe_list);

	spin_unlock_bh(&ct_safe_lists.lock);
}

static inline void ct_safe_list_del(struct nf_conn *ct)
{
	spin_lock_bh(&ct_safe_lists.lock);
	list_del(&ct->bcm_ext.safe_list);
	spin_unlock_bh(&ct_safe_lists.lock);
}

#if defined(CONFIG_BLOG)

static inline void __ct_blog_flow_accel_activate_event(struct nf_conn *ct,
		BlogFlowEventInfo_t *info)
{
	spin_lock_bh(&ct_safe_lists.lock);
	/* ensure ct is not being deleted */
	if(likely(atomic_read(&ct->ct_general.use) >= 1))
	{
		if(info->flow_event_type == FLOW_EVENT_TYPE_HW)
			ct->bcm_ext.hw_accel_flows++;
		else
			ct->bcm_ext.sw_accel_flows++;

		ct_set_curr_safe_list(ct);
		
		list_move_tail(&ct->bcm_ext.safe_list, ct->bcm_ext.curr_safe_list);
	}
	spin_unlock_bh(&ct_safe_lists.lock);
}

static inline void ct_blog_flow_activate_event(BlogFlowEventInfo_t *info)
{
    int idx;

    if ((info->flow_event_type == FLOW_EVENT_TYPE_FC) || 
			(info->flow_event_type == FLOW_EVENT_TYPE_HW)){

        for (idx=0; idx < info->ct_count; idx++)
        {
            if(info->ct_p[idx] == NULL) 
                continue;

		    __ct_blog_flow_accel_activate_event(info->ct_p[idx], info);
        }
    }
}

static inline void __ct_blog_flow_accel_deactivate_event(struct nf_conn *ct,
		BlogFlowEventInfo_t *info)
{
	spin_lock_bh(&ct_safe_lists.lock);
	/* ensure ct is not being deleted */
	if(likely(atomic_read(&ct->ct_general.use) >= 1)){

		if(info->flow_event_type == FLOW_EVENT_TYPE_HW)
			ct->bcm_ext.hw_accel_flows--;
		else
			ct->bcm_ext.sw_accel_flows--;

		ct_set_curr_safe_list(ct);

		list_move_tail(&ct->bcm_ext.safe_list, ct->bcm_ext.curr_safe_list);
	}
	spin_unlock_bh(&ct_safe_lists.lock);
}

static inline void ct_blog_flow_deactivate_event( BlogFlowEventInfo_t *info)
{
    int idx;

    if ((info->flow_event_type == FLOW_EVENT_TYPE_FC) || 
			(info->flow_event_type == FLOW_EVENT_TYPE_HW)){

        for (idx=0; idx < info->ct_count; idx++)
        {
            if(info->ct_p[idx] == NULL) 
                continue;

            __ct_blog_flow_accel_deactivate_event(info->ct_p[idx], info);
        }
    }
}

static int ct_blog_flowevent_notify(struct notifier_block * nb,
                 unsigned long  event, void *info)
{
    switch(event){

        case FLOW_EVENT_ACTIVATE:
            ct_blog_flow_activate_event(info);
            break;

        case FLOW_EVENT_DEACTIVATE:
            ct_blog_flow_deactivate_event(info);
            break;

        default:
            break;

    }
    return NOTIFY_OK;
}

#endif /*CONFIG_BLOG */


/*caller must have safe_list lock acquired */
static inline struct nf_conn* __ct_find_drop_candidate(struct net *net, struct list_head *list )
{
	struct list_head *tmp;
	struct nf_conn *ct, *ct_candidate=NULL;

	if (!list_empty(list)) {
		list_for_each(tmp, list) {
			ct = container_of(tmp, struct nf_conn, bcm_ext.safe_list);

			/*TODO  IPS_OFFLOAD_BIT, should we check all lists or only sw only lists*/


			if (likely(!test_bit(IPS_OFFLOAD_BIT, &ct->status) && nf_ct_is_confirmed(ct) 
				&& !nf_ct_is_dying(ct) && net_eq(nf_ct_net(ct), net) 
				&& atomic_inc_not_zero(&ct->ct_general.use))) {

			/* we dont need to check if connection is expired or not to drop it,
			 * or trrigger gc, as we are already using LRU
			 */ 
				ct_candidate = ct;
				/* move to the tail of the list while it's being deleted*/
				list_move_tail(&ct_candidate->bcm_ext.safe_list, list);
				break;
			}
		}
	}
	/* refcount of ct_candidate is incremented in this fucntion,
     * it's callers responsibility to decrement it
     */
	return ct_candidate;
}

/* Choose a LRU connection based on the configured drop order policy */
static int bcm_regardless_drop(struct net *net)
{
	struct nf_conn *ct_candidate = NULL;
	int i,dropped = 0;

	spin_lock_bh(&ct_safe_lists.lock);

	for( i=0; i < NF_CT_SAFE_LISTS_MAX; i++ ) {
		ct_candidate = __ct_find_drop_candidate(net, ct_safe_lists.drop_list_order[i]);
		if(ct_candidate)
			break;
	}

	spin_unlock_bh(&ct_safe_lists.lock);

	if (unlikely(ct_candidate == NULL))
		return dropped;

	if(nf_ct_delete(ct_candidate, 0, 0) == true){
		dropped = 1;
		NF_CT_STAT_INC_ATOMIC(net, early_drop);
	}

	nf_ct_put(ct_candidate);
	return dropped;
}

#endif /* _BCM_NF_REGARDLESSDROP_ */
