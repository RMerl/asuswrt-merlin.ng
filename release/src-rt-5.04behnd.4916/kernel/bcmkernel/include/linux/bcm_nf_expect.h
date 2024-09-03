/*
* <:copyright-BRCM:2023:DUAL/GPL:standard
* 
*    Copyright (c) 2023 Broadcom 
*    All Rights Reserved
* 
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License, version 2, as published by
* the Free Software Foundation (the "GPL").
* 
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
* 
* 
* A copy of the GPL is available at http://www.broadcom.com/licenses/GPLv2.php, or by
* writing to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
* Boston, MA 02111-1307, USA.
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

