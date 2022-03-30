/*
<:copyright-BRCM:2019:DUAL/GPL:standard 

   Copyright (c) 2019 Broadcom 
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

#include <linux/types.h>
#include <linux/seq_file.h>
#include <net/netfilter/nf_conntrack.h>
#include <linux/bcm_nf_conntrack.h>

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
bool bcm_nf_blog_ct_is_expired(struct nf_conn *ct)
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
			ct->timeout = newtimeout + nfct_time_stamp;
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
