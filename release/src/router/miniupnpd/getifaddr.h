/* $Id: getifaddr.h,v 1.12 2025/04/03 21:11:35 nanard Exp $ */
/* vim: tabstop=4 shiftwidth=4 noexpandtab
 * MiniUPnP project
 * http://miniupnp.free.fr/ or https://miniupnp.tuxfamily.org/
 * (c) 2006-2025 Thomas Bernard
 * This software is subject to the conditions detailed
 * in the LICENCE file provided within the distribution */

#ifndef GETIFADDR_H_INCLUDED
#define GETIFADDR_H_INCLUDED

/*! \file getifaddr.h
 * \brief get IP address of network interface
 */

struct in_addr;
struct in6_addr;

/*! \brief get IPv4 address
 *
 * write the ip v4 address as text in the buffer
 * \param[in] ifname network interface name
 * \param[out] buf character buffer
 * \param[in] len buf length
 * \param[out] addr ip v4 address
 * \param[out] mask ip v4 mask
 * \return 0 on success, -1 on failure */
int
getifaddr(const char * ifname, char * buf, int len,
          struct in_addr * addr, struct in_addr * mask);

/*! \brief get IPv4 or IPv6 address
 *
 * Filters out loopback and linklocal addresses
 * \param[in] ifname network interface name
 * \param[in] af `AF_INET` or `AF_INET6`
 * \param[out] addr address
 * \return 0 on success, -1 on failure */
int
getifaddr_in6(const char * ifname, int af, struct in6_addr* addr);

/*! \brief find a non link local IP v6 address for the interface.
 *
 * if ifname is NULL, look for all interfaces
 * \param[in] ifname network interface name
 * \param[out] dst character buffer
 * \param[in] n dst length
 * \return 0 on success, -1 on failure */
int
find_ipv6_addr(const char * ifname,
               char * dst, int n);

/*! \brief check if address is in private / reserved block (e.g. local area network)
 *
 * \param addr IPv4 address
 * \return 1 for private address, 0 for routable address */
int
addr_is_reserved(struct in_addr * addr);

#endif
