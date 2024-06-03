/*
*    Copyright (c) 2003-2019 Broadcom
*    All Rights Reserved
*
<:label-BRCM:2019:DUAL/GPL:standard

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
