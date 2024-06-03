/*
* <:copyright-BRCM:2023:DUAL/GPL:standard
* 
*    Copyright (c) 2023 Broadcom 
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
#if IS_ENABLED(CONFIG_BCM_HW_FIREWALL)
typedef int (*bcm_hwf_expect_event_t)(enum ip_conntrack_expect_events, struct nf_conntrack_expect *);
extern bcm_hwf_expect_event_t __rcu bcm_hwf_expect_event_fn;
#endif

static inline void bcm_nf_ct_expect_event(enum ip_conntrack_expect_events e,
		struct nf_conntrack_expect *exp)
{
#if IS_ENABLED(CONFIG_BCM_HW_FIREWALL)
	{
		bcm_hwf_expect_event_t fn;
		rcu_read_lock();
		fn = rcu_dereference(bcm_hwf_expect_event_fn);
		if(fn)
			fn(e, exp); 
		rcu_read_unlock();
	}
#endif
}

