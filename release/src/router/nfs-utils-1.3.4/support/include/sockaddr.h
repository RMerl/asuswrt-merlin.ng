/*
 * Copyright 2009 Oracle.  All rights reserved.
 *
 * This file is part of nfs-utils.
 *
 * nfs-utils is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * nfs-utils is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with nfs-utils.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef NFS_UTILS_SOCKADDR_H
#define NFS_UTILS_SOCKADDR_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#ifdef HAVE_LIBIO_H
#include <libio.h>
#endif
#include <stdlib.h>
#include <stdbool.h>
#include <sys/socket.h>
#include <netinet/in.h>

/*
 * This type is for defining buffers that contain network socket
 * addresses.
 *
 * Casting a "struct sockaddr *" to the address of a "struct
 * sockaddr_storage" breaks C aliasing rules.  The "union
 * nfs_sockaddr" type follows C aliasing rules yet specifically
 * allows converting pointers to it between "struct sockaddr *"
 * and a few other network sockaddr-related pointer types.
 *
 * Note that this union is much smaller than a sockaddr_storage.
 * It should be used only for AF_INET or AF_INET6 socket addresses.
 * An AF_LOCAL sockaddr_un, for example, will clearly not fit into
 * a buffer of this type.
 */
union nfs_sockaddr {
	struct sockaddr		sa;
	struct sockaddr_in	s4;
	struct sockaddr_in6	s6;
};

#if SIZEOF_SOCKLEN_T - 0 == 0
#define socklen_t unsigned int
#endif

#define SIZEOF_SOCKADDR_UNKNOWN	(socklen_t)0
#define SIZEOF_SOCKADDR_IN	(socklen_t)sizeof(struct sockaddr_in)

#ifdef IPV6_SUPPORTED
#define SIZEOF_SOCKADDR_IN6	(socklen_t)sizeof(struct sockaddr_in6)
#else	/* !IPV6_SUPPORTED */
#define SIZEOF_SOCKADDR_IN6	SIZEOF_SOCKADDR_UNKNOWN
#endif	/* !IPV6_SUPPORTED */

/**
 * nfs_sockaddr_length - return the size in bytes of a socket address
 * @sap: pointer to socket address
 *
 * Returns the size in bytes of @sap, or zero if the family is
 * not recognized.
 */
static inline socklen_t
nfs_sockaddr_length(const struct sockaddr *sap)
{
	switch (sap->sa_family) {
	case AF_INET:
		return SIZEOF_SOCKADDR_IN;
	case AF_INET6:
		return SIZEOF_SOCKADDR_IN6;
	}
	return SIZEOF_SOCKADDR_UNKNOWN;
}

static inline uint16_t
get_port4(const struct sockaddr *sap)
{
	const struct sockaddr_in *sin = (const struct sockaddr_in *)sap;
	return ntohs(sin->sin_port);
}

#ifdef IPV6_SUPPORTED
static inline uint16_t
get_port6(const struct sockaddr *sap)
{
	const struct sockaddr_in6 *sin6 = (const struct sockaddr_in6 *)sap;
	return ntohs(sin6->sin6_port);
}
#else	/* !IPV6_SUPPORTED */
static inline uint16_t
get_port6(__attribute__ ((unused)) const struct sockaddr *sap)
{
	return 0;
}
#endif	/* !IPV6_SUPPORTED */

/**
 * nfs_get_port - extract port value from a socket address
 * @sap: pointer to socket address
 *
 * Returns port value in host byte order, or zero if the
 * socket address contains an unrecognized family.
 */
static inline uint16_t
nfs_get_port(const struct sockaddr *sap)
{
	switch (sap->sa_family) {
	case AF_INET:
		return get_port4(sap);
	case AF_INET6:
		return get_port6(sap);
	}
	return 0;
}

static inline void
set_port4(struct sockaddr *sap, const uint16_t port)
{
	struct sockaddr_in *sin = (struct sockaddr_in *)sap;
	sin->sin_port = htons(port);
}

#ifdef IPV6_SUPPORTED
static inline void
set_port6(struct sockaddr *sap, const uint16_t port)
{
	struct sockaddr_in6 *sin6 = (struct sockaddr_in6 *)sap;
	sin6->sin6_port = htons(port);
}
#else	/* !IPV6_SUPPORTED */
static inline void
set_port6(__attribute__ ((unused)) struct sockaddr *sap,
		__attribute__ ((unused)) const uint16_t port)
{
}
#endif	/* !IPV6_SUPPORTED */

/**
 * nfs_set_port - set port value in a socket address
 * @sap: pointer to socket address
 * @port: port value to set
 *
 */
static inline void
nfs_set_port(struct sockaddr *sap, const uint16_t port)
{
	switch (sap->sa_family) {
	case AF_INET:
		set_port4(sap, port);
		break;
	case AF_INET6:
		set_port6(sap, port);
		break;
	}
}

/**
 * nfs_is_v4_loopback - test to see if socket address is AF_INET loopback
 * @sap: pointer to socket address
 *
 * Returns true if the socket address is the standard IPv4 loopback
 * address; otherwise false is returned.
 */
static inline _Bool
nfs_is_v4_loopback(const struct sockaddr *sap)
{
	const struct sockaddr_in *sin = (const struct sockaddr_in *)sap;

	if (sin->sin_family != AF_INET)
		return false;
	if (sin->sin_addr.s_addr != htonl(INADDR_LOOPBACK))
		return false;
        return true;
}

static inline _Bool
compare_sockaddr4(const struct sockaddr *sa1, const struct sockaddr *sa2)
{
	const struct sockaddr_in *sin1 = (const struct sockaddr_in *)sa1;
	const struct sockaddr_in *sin2 = (const struct sockaddr_in *)sa2;
	return sin1->sin_addr.s_addr == sin2->sin_addr.s_addr;
}

#ifdef IPV6_SUPPORTED
static inline _Bool
compare_sockaddr6(const struct sockaddr *sa1, const struct sockaddr *sa2)
{
	const struct sockaddr_in6 *sin1 = (const struct sockaddr_in6 *)sa1;
	const struct sockaddr_in6 *sin2 = (const struct sockaddr_in6 *)sa2;
	const struct in6_addr *saddr1 = &sin1->sin6_addr;
	const struct in6_addr *saddr2 = &sin2->sin6_addr;

	if (IN6_IS_ADDR_LINKLOCAL(saddr1) && IN6_IS_ADDR_LINKLOCAL(saddr2))
		if (sin1->sin6_scope_id != sin2->sin6_scope_id)
			return false;

	return IN6_ARE_ADDR_EQUAL(saddr1, saddr2);
}
#else	/* !IPV6_SUPPORTED */
static inline _Bool
compare_sockaddr6(__attribute__ ((unused)) const struct sockaddr *sa1,
		__attribute__ ((unused)) const struct sockaddr *sa2)
{
	return false;
}
#endif	/* !IPV6_SUPPORTED */

/**
 * nfs_compare_sockaddr - compare two socket addresses for equality
 * @sa1: pointer to a socket address
 * @sa2: pointer to a socket address
 *
 * Returns true if the two socket addresses contain equivalent
 * network addresses; otherwise false is returned.
 */
static inline _Bool
nfs_compare_sockaddr(const struct sockaddr *sa1, const struct sockaddr *sa2)
{
	if (sa1 == NULL || sa2 == NULL)
		return false;

	if (sa1->sa_family == sa2->sa_family)
		switch (sa1->sa_family) {
		case AF_INET:
			return compare_sockaddr4(sa1, sa2);
		case AF_INET6:
			return compare_sockaddr6(sa1, sa2);
		}

	return false;
}

#endif	/* !NFS_UTILS_SOCKADDR_H */
