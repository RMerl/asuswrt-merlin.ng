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
