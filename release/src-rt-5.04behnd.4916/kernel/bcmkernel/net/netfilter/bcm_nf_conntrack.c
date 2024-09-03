/*
<:copyright-BRCM:2019:DUAL/GPL:standard 

   Copyright (c) 2019 Broadcom 
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

#include <linux/types.h>
#include <linux/seq_file.h>
#include <net/netfilter/nf_conntrack.h>
#include <net/netfilter/nf_conntrack_expect.h>
#include <linux/bcm_nf_conntrack.h>
#include <linux/bcm_nf_expect.h>

int ct_show_bcm_ext(struct seq_file *s, const struct nf_conn *ct)
{
#if IS_ENABLED(CONFIG_BCM_INGQOS)
	seq_printf(s, "iqprio=%u ", ct->bcm_ext.iq_prio);
#endif

#if defined(CONFIG_BCM_KF_NF_REGARDLESS_DROP)
	seq_printf(s, "swaccel=%u ", ct->bcm_ext.sw_accel_flows);
	seq_printf(s, "hwaccel=%u ", ct->bcm_ext.hw_accel_flows);
#endif

	return seq_has_overflowed(s);
}
EXPORT_SYMBOL(ct_show_bcm_ext);

#if defined(CONFIG_BLOG)
bool bcm_nf_blog_ct_is_expired(const struct nf_conn *ct)
{

	BlogCtTime_t ct_time;

	/*query the timeout status from blog */
	memset(&ct_time, 0, sizeof(ct_time));
	blog_lock();
	if (ct->bcm_ext.blog_key[BLOG_PARAM1_DIR_ORIG] != BLOG_KEY_FC_INVALID ||
	    ct->bcm_ext.blog_key[BLOG_PARAM1_DIR_REPLY] != BLOG_KEY_FC_INVALID) {
		blog_query(QUERY_FLOWTRACK, (void *)ct,
			   ct->bcm_ext.blog_key[BLOG_PARAM1_DIR_ORIG],
			   ct->bcm_ext.blog_key[BLOG_PARAM1_DIR_REPLY],
			   (unsigned long) &ct_time);
	}
	blog_unlock();

	if (ct_time.flags.valid) {
		signed long newtimeout;

		newtimeout = ct->bcm_ext.extra_jiffies - (ct_time.idle * HZ);

		if (newtimeout > 0) {

			((struct nf_conn *)ct)->timeout = newtimeout + nfct_time_stamp;
			return false;
		}
	}

	/* ct is expired */
	return true;
}
EXPORT_SYMBOL(bcm_nf_blog_ct_is_expired);
#endif /*CONFIG_BLOG */

#if defined(CONFIG_BCM_NF_DERIVED_CONN)
DEFINE_SPINLOCK(bcm_derived_conn_lock);
EXPORT_SYMBOL(bcm_derived_conn_lock);
#endif

#if IS_ENABLED(CONFIG_BCM_HW_FIREWALL)
bcm_hwf_ct_event_t __rcu bcm_hwf_ct_event_fn = NULL;
EXPORT_SYMBOL(bcm_hwf_ct_event_fn);

bcm_hwf_expect_event_t __rcu bcm_hwf_expect_event_fn = NULL;
EXPORT_SYMBOL(bcm_hwf_expect_event_fn);
#endif
