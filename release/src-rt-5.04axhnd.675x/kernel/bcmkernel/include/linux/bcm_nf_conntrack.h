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

#ifndef _BCM_NF_CONNTRACK_H
#define _BCM_NF_CONNTRACK_H

#include <linux/types.h>
#include <linux/netfilter/nf_conntrack_common.h>
#include <net/netfilter/nf_conntrack.h>
#include <net/netfilter/nf_conntrack_helper.h>

#include <linux/blog.h>
#include <linux/dpi.h>
#include <linux/iqos.h>


#if defined(CONFIG_BLOG)
static inline void bcm_nf_blog_ct_init(struct nf_conn *ct)
{
	/* no blog lock needed here */
	set_bit(IPS_BLOG_BIT, &ct->status);  /* Enable conntrack blogging */
	ct->bcm_ext.blog_key[IP_CT_DIR_ORIGINAL] = BLOG_KEY_FC_INVALID;
	ct->bcm_ext.blog_key[IP_CT_DIR_REPLY]    = BLOG_KEY_FC_INVALID;
} 

static inline int bcm_nf_blog_destroy_conntrack(struct nf_conn *ct)
{
	blog_lock();
	pr_debug("%s(%px) blog keys[0x%08x,0x%08x]\n", __func__,
		ct, ct->bcm_ext.blog_key[IP_CT_DIR_ORIGINAL],
		ct->bcm_ext.blog_key[IP_CT_DIR_REPLY]);


	/* Conntrack going away, notify blog client */
	if ((ct->bcm_ext.blog_key[IP_CT_DIR_ORIGINAL] != BLOG_KEY_FC_INVALID) ||
			(ct->bcm_ext.blog_key[IP_CT_DIR_REPLY] != BLOG_KEY_FC_INVALID)) {

		blog_notify(DESTROY_FLOWTRACK, (void*)ct,
					(uint32_t)ct->bcm_ext.blog_key[IP_CT_DIR_ORIGINAL],
					(uint32_t)ct->bcm_ext.blog_key[IP_CT_DIR_REPLY]);
	}

	clear_bit(IPS_BLOG_BIT, &ct->status);	/* Disable further blogging */
	blog_unlock();

	return 0;
}

static inline int bcm_nf_blog_link_ct(struct nf_conn *ct, enum ip_conntrack_info ctinfo,
			 struct sk_buff *skb, u_int16_t l3num, u_int8_t protonum)
{

	/* here we dont need blog lock as this blog is owned by this skb */

	struct nf_conn_help * help = nfct_help(ct);


	if ((help != (struct nf_conn_help *)NULL) &&
			(help->helper != (struct nf_conntrack_helper *)NULL) &&
			(help->helper->name && strcmp(help->helper->name, "BCM-NAT"))) {
		pr_debug("nf_conntrack_in: skb<%px> ct<%px> helper<%s> found\n",
				skb, ct, help->helper->name);
		clear_bit(IPS_BLOG_BIT, &ct->status);
	}

	if (test_bit(IPS_BLOG_BIT, &ct->status)) {	/* OK to blog ? */
		pr_debug("nf_conntrack_in: skb<%px> blog<%px> ct<%px>\n",
				skb, blog_ptr(skb), ct);

		blog_link(FLOWTRACK, blog_ptr(skb),
				(void*)ct, CTINFO2DIR(ctinfo), 0);
	} else {
		pr_debug("nf_conntrack_in: skb<%px> ct<%px> NOT BLOGible<%px>\n",
				skb, ct, blog_ptr(skb));
		blog_skip(skb, blog_skip_reason_ct_status_donot_blog); /* No blogging */
	}

	return 0;
}

static inline int bcm_nf_blog_update_timeout(struct nf_conn *ct, unsigned long extra_jiffies)
{
	blog_lock();
	
	if ((ct->bcm_ext.blog_key[IP_CT_DIR_ORIGINAL] != BLOG_KEY_FC_INVALID) ||
			(ct->bcm_ext.blog_key[IP_CT_DIR_REPLY] != BLOG_KEY_FC_INVALID)) {

		unsigned int blog_key = (ct->bcm_ext.blog_key[IP_CT_DIR_ORIGINAL] != BLOG_KEY_FC_INVALID) ? 
						ct->bcm_ext.blog_key[IP_CT_DIR_ORIGINAL] : ct->bcm_ext.blog_key[IP_CT_DIR_REPLY];

		blog_notify(UPDATE_FLOWTRACK_IDLE_TIMEOUT, (void*)ct,
					blog_key, (uint32_t)(extra_jiffies/HZ));
	}
	blog_unlock();

	return 0;
}

#endif /*CONFIG_BLOG */

static inline int nf_conntrack_ipv6_is_multicast(const __be32 ip6[4])
{
	return ((ip6[0] & htonl(0xFF000000)) == htonl(0xFF000000));
}

static inline void bcm_nf_conn_set_iq_prio(struct nf_conn *ct, struct sk_buff *skb)
{
#if IS_ENABLED(CONFIG_BCM_INGQOS)
#if defined(CONFIG_BLOG)
	if (skb != NULL && skb->blog_p != NULL ) 
		ct->bcm_ext.iq_prio = (blog_iq(skb) == BLOG_IQ_PRIO_HIGH) ? IQOS_PRIO_HIGH : IQOS_PRIO_LOW;
	else
#endif
	{

		switch (nf_ct_l3num(ct)) {
			case AF_INET:
				ct->bcm_ext.iq_prio = ipv4_is_multicast(ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.dst.u3.ip) ? IQOS_PRIO_HIGH : IQOS_PRIO_LOW;
				break;
			case AF_INET6:
				ct->bcm_ext.iq_prio = nf_conntrack_ipv6_is_multicast(ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.dst.u3.ip6) ? IQOS_PRIO_HIGH : IQOS_PRIO_LOW;
				break;
			default:
				ct->bcm_ext.iq_prio = IQOS_PRIO_LOW;
		}
	}	
#endif
}

static inline void bcm_nf_iqos_destroy_conntrack(struct nf_conn *ct)
{
#if IS_ENABLED(CONFIG_BCM_INGQOS)
	if (test_bit(IPS_IQOS_BIT,&ct->status)) {
		clear_bit(IPS_IQOS_BIT, &ct->status);	
		iqos_rem_L4port(ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.dst.protonum,
				ntohs(ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.dst.u.all), IQOS_ENT_DYN);
		iqos_rem_L4port(ct->tuplehash[IP_CT_DIR_REPLY].tuple.dst.protonum,
				ntohs(ct->tuplehash[IP_CT_DIR_REPLY].tuple.dst.u.all), IQOS_ENT_DYN);
	}
#endif
}

#if defined(CONFIG_BCM_NF_DERIVED_CONN)
static inline void bcm_nf_ct_derived_list_add(struct nf_conn *ct)
{
 	BCM_DERIVED_CONN_LOCK_BH();

	 /* master ref count incremneted before calling this fcuntion */
	/* via exp->master safe, refcnt bumped in nf_ct_find_expectation */
	if(ct->master){

		list_add(&ct->bcm_ext.derived_list,
			 &ct->master->bcm_ext.derived_connections);
	}
	
 	BCM_DERIVED_CONN_UNLOCK_BH();
}

static inline void bcm_nf_ct_derived_list_del(struct nf_conn *ct)
{
 	BCM_DERIVED_CONN_LOCK_BH();
	if(ct->master){
		list_del(&ct->bcm_ext.derived_list);
	}
	
 	BCM_DERIVED_CONN_UNLOCK_BH();
}
static inline void bcm_nf_ct_derived_conn_init(struct nf_conn *ct)
{
	INIT_LIST_HEAD(&ct->bcm_ext.derived_connections);
	INIT_LIST_HEAD(&ct->bcm_ext.derived_list);
	ct->bcm_ext.derived_timeout = 0;
}
#else

static inline void bcm_nf_ct_derived_list_add(struct nf_conn *ct){};
static inline void bcm_nf_ct_derived_list_del(struct nf_conn *ct){};
static inline void bcm_nf_ct_derived_conn_init(struct nf_conn *ct){};
#endif


static inline unsigned long bcm_nf_ct_refresh(struct nf_conn *ct,
			  unsigned long extra_jiffies)
{
#if defined(CONFIG_BCM_NF_DERIVED_CONN)
	/* when derived time out is set always use it */
	if(ct->bcm_ext.derived_timeout)
		extra_jiffies = ct->bcm_ext.derived_timeout;
#endif

#if defined(CONFIG_BLOG)
	if(ct->bcm_ext.extra_jiffies != extra_jiffies) {
		ct->bcm_ext.extra_jiffies = extra_jiffies;
		/*notify accelerator */
		bcm_nf_blog_update_timeout(ct, extra_jiffies);
	}
#endif

	return extra_jiffies;
}

static void bcm_conntrack_init_end(void)
{
#if IS_ENABLED(CONFIG_BCM_DPI)
	dpi_conntrack_init();
#endif
}

static void bcm_conntrack_cleanup_end(void)
{
#if IS_ENABLED(CONFIG_BCM_DPI)
	dpi_conntrack_cleanup();
#endif
}

static void bcm_nf_ct_alloc(struct nf_conn *ct, struct sk_buff *skb)
{
#if defined(CONFIG_BLOG)
	bcm_nf_blog_ct_init(ct);
#endif
	/* REGARDLESS_DROP */
	INIT_LIST_HEAD(&ct->bcm_ext.safe_list);

	bcm_nf_ct_derived_conn_init(ct);
	bcm_nf_conn_set_iq_prio(ct, skb);

#if IS_ENABLED(CONFIG_BCM_DPI)
	memset(&ct->bcm_ext.dpi, 0, sizeof(ct->bcm_ext.dpi));
	if (skb && skb->dev && is_netdev_wan(skb->dev))
		set_bit(DPI_CT_INIT_FROM_WAN_BIT, &ct->bcm_ext.dpi.flags);
#endif
#if IS_ENABLED(CONFIG_NF_DYNDSCP)
	/* initialize dynamic dscp inheritance fields */
	ct->bcm_ext.dyndscp.status = 0;
	ct->bcm_ext.dyndscp.dscp[0] = 0;
	ct->bcm_ext.dyndscp.dscp[1] = 0;
#endif
}

static void bcm_nf_ct_init(struct nf_conn *ct)
{
	bcm_nf_ct_derived_list_add(ct);
}

static void bcm_nf_ct_delete_from_lists(struct nf_conn *ct)
{
#if IS_ENABLED(CONFIG_BCM_DPI)
	dpi_nf_ct_delete_from_lists(ct);
#endif
#if IS_ENABLED(CONFIG_BCM_SGS)
	sgs_nf_ct_delete_from_lists(ct);
#endif
}

static void bcm_nf_ct_destroy(struct nf_conn *ct)
{
#if defined(CONFIG_BLOG)
	bcm_nf_blog_destroy_conntrack(ct);
#endif
	bcm_nf_iqos_destroy_conntrack(ct);
	bcm_nf_ct_derived_list_del(ct);
}
#endif /* _BCM_NF_CONNTRACK_H */
