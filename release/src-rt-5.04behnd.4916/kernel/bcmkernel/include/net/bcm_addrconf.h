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
#ifndef _BCM_ADDRCONF_H
#define	_BCM_ADDRCONF_H

void ipv6_del_addr(struct inet6_ifaddr *ifp);
int ipv6_generate_eui64(u8 *eui, struct net_device *dev);

static inline int isULA(const struct in6_addr *addr)
{
	__be32 st;

	st = addr->s6_addr32[0];

	/* RFC 4193 */
	if ((st & htonl(0xFE000000)) == htonl(0xFC000000))
		return	1;
	else
		return	0;
}

static inline int isSpecialAddr(const struct in6_addr *addr)
{
	__be32 st;

	st = addr->s6_addr32[0];

	/* RFC 5156 */
	if (((st & htonl(0xFFFFFFFF)) == htonl(0x20010db8)) ||
		((st & htonl(0xFFFFFFF0)) == htonl(0x20010010)))
		return	1;
	else
		return	0;
}

int addrconf_update_lladdr(struct net_device *dev);

#endif	/* _BCM_ADDRCONF_H */
