/*
*    Copyright (c) 2003-2022 Broadcom
*    All Rights Reserved
*
<:label-BRCM:2018:DUAL/GPL:standard

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

#include <linux/module.h>
#include <linux/skbuff.h>
#include <linux/ip.h>

#include <linux/netfilter/xt_mscs.h>
#include <linux/netfilter/x_tables.h>

#include <net/netfilter/nf_conntrack.h>

#if defined(CONFIG_BLOG)
#include <linux/blog.h>
#endif
#include <linux/bcm_skb_defines.h>
    
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Broadcom");
MODULE_DESCRIPTION("Xtables: mirrored stream classification service");
MODULE_ALIAS("ipt_MSCS");

#if 0
#define DEBUG_MSCS(fmt, ...) printk("@@%s:L%d "fmt, __FUNCTION__, __LINE__, ##__VA_ARGS__)
#else
#define DEBUG_MSCS(fmt, ...) {}
#endif

struct xt_mscs_data {
    struct list_head    head;
    spinlock_t          lock;
};

static void 
mscs_flow_flush(struct nf_conn *ct)
{
#if defined(CONFIG_BLOG)
	blog_lock();
	if (ct->bcm_ext.blog_key[ct->bcm_ext.mscs.dir] != BLOG_KEY_FC_INVALID) {
		/* remove flow from flow cache if exists */
		blog_notify(DESTROY_FLOWTRACK, (void*)ct,
							(uint32_t)ct->bcm_ext.blog_key[IP_CT_DIR_ORIGINAL],
							(uint32_t)ct->bcm_ext.blog_key[IP_CT_DIR_REPLY]);

	}
	blog_unlock();
#endif
}

static bool
mscs_mt(const struct sk_buff *skb, struct xt_action_param *par)
{
    const struct xt_mscs_mtinfo *info = par->matchinfo;
    struct xt_mscs_data *data = info->data;
    struct nf_conn *ct;
    enum ip_conntrack_info ctinfo;
    struct nf_mscs *ct_mscs;

    ct = nf_ct_get(skb, &ctinfo);
    if (unlikely(!ct)) return false;
    ct_mscs = &ct->bcm_ext.mscs;

//    DEBUG_MSCS("info=%8p[global=%d bmap=%x limit=%d]\n", info, info->is_global, info->up_bitmap, info->up_limit);
    if (info->is_global) {
        if (!CT_MSCS_INITED(ct))
            return false; 

        if (ct_mscs->dir==CTINFO2DIR(ctinfo)) { // global-DS: do mirroring
            struct sk_buff *s = (struct sk_buff*)skb;
            SET_WLAN_PRIORITY_OVRD(s, 1);
            s->mark = SKBMARK_SET_Q_PRIO(skb->mark, ct_mscs->priority);

            DEBUG_MSCS("gd>> ct=%8p skb=%8p pri=%x mark=%x mscs.dir=%x\n", ct, skb, skb->priority, skb->mark, ct_mscs->dir);
        } else {    // global-US: update priority if necessary
            uint32_t priority = GET_WLAN_PRIORITY(skb->priority);

            if (!(ct_mscs->up_bitmap & (1<<priority)))
                return true;

            priority = (priority < ct_mscs->up_limit) ? priority : ct_mscs->up_limit;
            if (ct_mscs->priority != priority) {
                ct_mscs->priority = priority;
                mscs_flow_flush(ct);
                DEBUG_MSCS("gu>> ct=%8p skb=%8p update pri=%d\n", ct, skb, ct_mscs->priority);
            }
        }
    } else if (!CT_MSCS_INITED(ct)) {  // per-rule-US: matching
        uint32_t priority = GET_WLAN_PRIORITY(skb->priority);
        
        if (!(info->up_bitmap & (1<<priority)))
            return false;

        priority = (priority < info->up_limit) ? priority : info->up_limit;            
        ct_mscs->dir = (CTINFO2DIR(ctinfo) != IP_CT_DIR_ORIGINAL) ? IP_CT_DIR_ORIGINAL : IP_CT_DIR_REPLY;  // direction to apply mirrored priority is reversed
        ct_mscs->priority = priority;
        ct_mscs->up_limit = info->up_limit;
        ct_mscs->up_bitmap = info->up_bitmap;
        ct_mscs->entry.next = ct_mscs->entry.prev = &ct_mscs->entry;

        spin_lock_bh(&data->lock);
        ct_mscs->lock = &data->lock;
        list_add_tail(&ct_mscs->entry, &data->head);
        spin_unlock_bh(&data->lock);
        
        mscs_flow_flush(ct);
        DEBUG_MSCS("iu<< add ct=%8p skb=%8p pri=%x mark=%x up_limit=%d mscs.dir=%x pri=%x\n", ct, skb,
            skb->priority, skb->mark, info->up_limit, ct_mscs->dir, ct_mscs->priority);
    }
    
    return true;
}

static int mscs_mt_check(const struct xt_mtchk_param *par)
{
    struct xt_mscs_mtinfo *info = par->matchinfo;
    struct xt_mscs_data *data;

    /* ct linked list is for per station rules */
    if (info->is_global) return 0;
    
    DEBUG_MSCS("info=%8p[global=%d bmap=%x limit=%d]\n", info, info->is_global, info->up_bitmap, info->up_limit);
    /* init private data */
    data = kmalloc(sizeof(*data), GFP_KERNEL);
    info->data = data;
    if (data) {
        data->head.next = data->head.prev = &data->head;
        spin_lock_init(&data->lock);
    }
    
    return PTR_ERR_OR_ZERO(data);
}

static void mscs_mt_destroy(const struct xt_mtdtor_param *par)
{
    struct xt_mscs_mtinfo *info = par->matchinfo;
    struct xt_mscs_data *data = info->data;

    /* ct linked list is for per station rules */
    if (info->is_global) return;

    DEBUG_MSCS("info=%8p[global=%d bmap=%x limit=%d]\n", info, info->is_global, info->up_bitmap, info->up_limit);
    /* traverse list to deactivate entry, flush flow */
    if (!data) return;

    spin_lock_bh(&data->lock);
    while (!list_empty(&data->head)) {
        struct nf_conn *ct;

        ct = container_of(data->head.next, struct nf_conn, bcm_ext.mscs.entry);
        list_del(&ct->bcm_ext.mscs.entry);
        ct->bcm_ext.mscs.lock = NULL;

        spin_unlock_bh(&data->lock);
        mscs_flow_flush(ct);
        DEBUG_MSCS("remove ct=%8p\n", ct);
        spin_lock_bh(&data->lock);
    }
    spin_unlock_bh(&data->lock);
    
    info->data = NULL;
    kfree(data);
}

static struct xt_match mscs_mt_reg __read_mostly = {
	.name           = "mscs",
	.revision       = 0,
	.family         = NFPROTO_UNSPEC,
	.checkentry     = mscs_mt_check,
	.match          = mscs_mt,
	.matchsize      = sizeof(struct xt_mscs_mtinfo),
	.usersize       = offsetof(struct xt_mscs_mtinfo, data),
	.destroy        = mscs_mt_destroy,
	.me             = THIS_MODULE,
};

static int __init mscs_mt_init(void)
{
    DEBUG_MSCS("\n");
	return xt_register_match(&mscs_mt_reg);
}

static void __exit mscs_mt_exit(void)
{
    DEBUG_MSCS("\n");
	xt_unregister_match(&mscs_mt_reg);
}

module_init(mscs_mt_init);
module_exit(mscs_mt_exit);
