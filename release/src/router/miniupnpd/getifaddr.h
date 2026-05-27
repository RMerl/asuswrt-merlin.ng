/* $Id: getifaddr.h,v 1.13 2025/04/08 21:28:42 nanard Exp $ */
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

#define GETIFADDR_OK 0
/*! \brief bad arguments */
#define GETIFADDR_BAD_ARGS -1
/*! \brief socket() call error */
#define GETIFADDR_SOCKET_ERROR -2
/*! \brief device not configure / no such device */
#define GETIFADDR_DEVICE_NOT_CONFIGURED -3
/*! \brief other error from ioctl() */
#define GETIFADDR_IOCTL_ERROR -4
/*! \brief network interface is down */
#define GETIFADDR_IF_DOWN -5
/*! \brief no IPv4 address for the network interface */
#define GETIFADDR_NO_ADDRESS -6
/*! \brief inet_ntop() call error */
#define GETIFADDR_INET_NTOP_ERROR -7
/*! \brief getifaddrs() call error */
#define GETIFADDR_GETIFADDRS_ERROR -8

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
 * \return 0 on success, a negative value on failure */
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
