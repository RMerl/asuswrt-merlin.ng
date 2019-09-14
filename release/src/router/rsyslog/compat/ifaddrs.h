#include "config.h"
#ifndef HAVE_GETIFADDRS
/*
 * CDDL HEADER START
 *
 * The contents of this file are subject to the terms of the
 * Common Development and Distribution License (the "License").
 * You may not use this file except in compliance with the License.
 *
 * You can obtain a copy of the license at usr/src/OPENSOLARIS.LICENSE
 * or http://www.opensolaris.org/os/licensing.
 * See the License for the specific language governing permissions
 * and limitations under the License.
 *
 * When distributing Covered Code, include this CDDL HEADER in each
 * file and include the License file at usr/src/OPENSOLARIS.LICENSE.
 * If applicable, add the following below this CDDL HEADER, with the
 * fields enclosed by brackets "[]" replaced with your own identifying
 * information: Portions Copyright [yyyy] [name of copyright owner]
 *
 * CDDL HEADER END
 */
/*
 * Copyright (c) 2010, Oracle and/or its affiliates. All rights reserved.
 */
#ifndef _IFADDRS_H
#define	_IFADDRS_H

#ifdef	__cplusplus
extern "C" {
#endif

#include <sys/types.h>

/*
 * The `getifaddrs' function generates a linked list of these structures.
 * Each element of the list describes one network interface.
 */
#if defined(_AIX)
struct ifaddrs_rsys {
	struct ifaddrs_rsys	*ifa_next;	/* Pointer to the next structure. */
#else
struct ifaddrs {
	struct ifaddrs	*ifa_next;	/* Pointer to the next structure. */
#endif
	char		*ifa_name;	/* Name of this network interface. */
	uint64_t	ifa_flags;	/* Flags as from SIOCGLIFFLAGS ioctl. */
	struct sockaddr	*ifa_addr;	/* Network address of this interface. */
	struct sockaddr	*ifa_netmask;	/* Netmask of this interface. */
	union {
		/*
		 * At most one of the following two is valid.  If the
		 * IFF_BROADCAST bit is set in `ifa_flags', then
		 * `ifa_broadaddr' is valid.  If the IFF_POINTOPOINT bit is
		 * set, then `ifa_dstaddr' is valid. It is never the case that
		 * both these bits are set at once.
		 */
		struct sockaddr	*ifu_broadaddr;
		struct sockaddr	*ifu_dstaddr;
	} ifa_ifu;
	void		*ifa_data; /* Address-specific data (may be unused). */
/*
 * This may have been defined in <net/if.h>.
 */
#ifndef ifa_broadaddr
#define	ifa_broadaddr	ifa_ifu.ifu_broadaddr	/* broadcast address */
#endif
#ifndef ifa_dstaddr
#define	ifa_dstaddr	ifa_ifu.ifu_dstaddr	/* other end of p-to-p link */
#endif
};

/*
 * Create a linked list of `struct ifaddrs' structures, one for each
 * network interface on the host machine.  If successful, store the
 * list in *ifap and return 0.  On errors, return -1 and set `errno'.
 *
 * The storage returned in *ifap is allocated dynamically and can
 * only be properly freed by passing it to `freeifaddrs'.
 */
#if defined(_AIX)
extern int getifaddrs(struct ifaddrs_rsys **);
#else
extern int getifaddrs(struct ifaddrs **);
#endif

/* Reclaim the storage allocated by a previous `getifaddrs' call. */
#if defined(_AIX)
extern void freeifaddrs(struct ifaddrs_rsys *);
#else
extern void freeifaddrs(struct ifaddrs *);
#endif


#ifdef	__cplusplus
}
#endif

#endif	/* _IFADDRS_H */
#endif /* HAVE_GETIFADDRS */
