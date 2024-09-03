/*
*    Copyright (c) 2003-2019 Broadcom
*    All Rights Reserved
*
<:label-BRCM:2019:DUAL/GPL:standard

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

#if IS_ENABLED(CONFIG_IPV6)

#include <net/addrconf.h>

int addrconf_update_lladdr(struct net_device *dev)
{
	struct inet6_dev *idev;
	struct inet6_ifaddr *ifladdr = NULL;
	struct inet6_ifaddr *ifp;
	struct in6_addr addr6;
	int err = -EADDRNOTAVAIL;

	ASSERT_RTNL();

	idev = __in6_dev_get(dev);
	if (idev != NULL)
	{
		read_lock_bh(&idev->lock);
        list_for_each_entry(ifp, &idev->addr_list, if_list) {
			if (IFA_LINK == ifp->scope)
			{
				ifladdr = ifp;
				in6_ifa_hold(ifp);
				break;
			}
		}
		read_unlock_bh(&idev->lock);

		if ( ifladdr )
		{
			/* delete the address */
			ipv6_del_addr(ifladdr);

			/* add new LLA */ 
			memset(&addr6, 0, sizeof(struct in6_addr));
			addr6.s6_addr32[0] = htonl(0xFE800000);

			if (0 == ipv6_generate_eui64(addr6.s6_addr + 8, dev))
			{
				addrconf_add_linklocal(idev, &addr6, 0);
				err = 0;
			}
		}
	}

	return err;

}

#endif /* IS_ENABLED(CONFIG_IPV6) */
