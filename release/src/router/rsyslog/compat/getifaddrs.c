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

#include <netdb.h>
#if !defined (_AIX)
#include <nss_dbdefs.h>
#endif

#include <netinet/in.h>
#include <sys/socket.h>
#if defined (_AIX)
#include <sys/errno.h>
#include <sys/ioctl.h>
#endif
#include <string.h>
#include <stdio.h>
#if defined (_AIX)
#include <netdb.h>
#endif
#if !defined (_AIX)
#include <sys/sockio.h>
#endif
#include <sys/types.h>
#include <stdlib.h>
#include <net/if.h>
#include <ifaddrs.h>
#if defined (_AIX)
#include <netinet/in6_var.h>
#endif

/* Normally this is defined in <net/if.h> but was new for Solaris 11 */
#ifndef LIFC_ENABLED
#define LIFC_ENABLED    0x20
#endif

#if defined (_AIX) /* Use ifaddrs_rsys instead of ifaddrs and ifreq instead of lifreq */
int getallifaddrs(sa_family_t af, struct ifaddrs_rsys **ifap, int64_t flags);
int getallifs(int s, sa_family_t af, struct ifreq **ifr, int *numifs,
	int64_t ifc_flags);
#else
int getallifaddrs(sa_family_t af, struct ifaddrs **ifap, int64_t flags);
int getallifs(int s, sa_family_t af, struct lifreq **lifr, int *numifs,
	int64_t lifc_flags);
#endif

/*
 * Create a linked list of `struct ifaddrs_rsys' structures, one for each
 * address that is UP. If successful, store the list in *ifap and
 * return 0.  On errors, return -1 and set `errno'.
 *
 * The storage returned in *ifap is allocated dynamically and can
 * only be properly freed by passing it to `freeifaddrs'.
 */
int
#if defined (_AIX)
getifaddrs(struct ifaddrs_rsys **ifap)
#else
getifaddrs(struct ifaddrs **ifap)
#endif
{
	int		err;
	char		*cp;
#if defined (_AIX)
	struct ifaddrs_rsys	*curr;
#else
	struct ifaddrs  *curr;
#endif

	if (ifap == NULL) {
		errno = EINVAL;
		return (-1);
	}
	*ifap = NULL;
	err = getallifaddrs(AF_UNSPEC, ifap, LIFC_ENABLED);
	if (err == 0) {
		for (curr = *ifap; curr != NULL; curr = curr->ifa_next) {
			if ((cp = strchr(curr->ifa_name, ':')) != NULL)
				*cp = '\0';
		}
	}
	return (err);
}

void
#if defined (_AIX)
freeifaddrs(struct ifaddrs_rsys *ifa)
#else
freeifaddrs(struct ifaddrs *ifa)
#endif
{
#if defined (_AIX)
	struct ifaddrs_rsys *curr;
#else
	struct ifaddrs *curr;
#endif

	while (ifa != NULL) {
		curr = ifa;
		ifa = ifa->ifa_next;
		free(curr->ifa_name);
		free(curr->ifa_addr);
		free(curr->ifa_netmask);
		free(curr->ifa_dstaddr);
		free(curr);
	}
}

/*
 * Returns all addresses configured on the system. If flags contain
 * LIFC_ENABLED, only the addresses that are UP are returned.
 * Address list that is returned by this function must be freed
 * using freeifaddrs().
 */
#if defined (_AIX)
int
getallifaddrs(sa_family_t af, struct ifaddrs_rsys **ifap, int64_t flags)
{
	struct ifreq *buf = NULL;
	struct ifreq *ifrp;
	struct ifreq ifrl;
	struct in6_ifreq ifrl6;
	int ret;
	int s, n, iflen;
	struct ifaddrs_rsys *curr, *prev;
	sa_family_t ifr_af;
	int sock4;
	int sock6;
	int err;
	int ifsize;
	char *s_ifrp, *e_ifrp;
	int flag;

	if ((sock4 = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
		return (-1);
	if ((sock6 = socket(AF_INET6, SOCK_DGRAM, 0)) < 0) {
		err = errno;
		close(sock4);
		errno = err;
		return (-1);
	}

retry:
	/* Get all interfaces from SIOCGIFCONF */
	ret = getallifs(sock4, af, &buf, &iflen, (flags & ~LIFC_ENABLED));
	if (ret != 0)
		goto fail;

	/*
	 * Loop through the interfaces obtained from SIOCGIFCOMF
	 * and retrieve the addresses, netmask and flags.
	 */
	prev = NULL;
	s_ifrp = (char *)buf;
	e_ifrp = (char *)buf + iflen;
	*ifap = NULL;
	while (s_ifrp < e_ifrp)
	{
		ifrp = (struct ifreq *)s_ifrp;
		ifsize = sizeof(struct ifreq);

		if (ifrp->ifr_addr.sa_len > sizeof(ifrp->ifr_ifru)) {
			ifsize += ifrp->ifr_addr.sa_len - sizeof(ifrp->ifr_ifru);
		}

		/* Prepare for the ioctl call */
		(void) strncpy(ifrl.ifr_name, ifrp->ifr_name,
		    sizeof (ifrl.ifr_name));
		(void) strncpy(ifrl6.ifr_name, ifrp->ifr_name,
		    sizeof (ifrl.ifr_name));
		ifr_af = ifrp->ifr_addr.sa_family;

		if (ifr_af != AF_INET && ifr_af != AF_INET6)
			goto next;

		s = (ifr_af == AF_INET ? sock4 : sock6);

		if (ioctl(s, SIOCGIFFLAGS, (caddr_t)&ifrl) < 0)
			goto fail;

		if ((flags & LIFC_ENABLED) && !(ifrl.ifr_flags & IFF_UP)) {
			goto next;
		}

		/*
		 * Allocate the current list node. Each node contains data
		 * for one ifaddrs structure.
		 */
		curr = calloc(1, sizeof (struct ifaddrs_rsys));
		if (curr == NULL)
			goto fail;

		if (prev != NULL) {
			prev->ifa_next = curr;
		} else {
			/* First node in the linked list */
			*ifap = curr;
		}
		prev = curr;

/* AIXPORT :  ifreq field names used instead of linux lifreq field names */
		curr->ifa_flags = ifrl.ifr_flags;
		if ((curr->ifa_name = strdup(ifrp->ifr_name)) == NULL)
			goto fail;

		curr->ifa_addr = malloc(sizeof (struct sockaddr_storage));
		if (curr->ifa_addr == NULL)
			goto fail;
		(void) memcpy(curr->ifa_addr, &ifrp->ifr_addr,
		    sizeof (struct sockaddr_storage));

		/* Get the netmask */
		if (ifr_af == AF_INET) {
			if (ioctl(s, SIOCGIFNETMASK, (caddr_t)&ifrl) < 0) {
				goto fail;
			}
			curr->ifa_netmask = malloc(sizeof (struct sockaddr_storage));
			if (curr->ifa_netmask == NULL)
				goto fail;
			(void) memcpy(curr->ifa_netmask, &ifrl.ifr_addr,
			    sizeof (struct sockaddr_storage));
		} else {
			if (ioctl(s, SIOCGIFNETMASK6, (caddr_t)&ifrl6) < 0) {
				goto fail;
			}
			curr->ifa_netmask = malloc(sizeof (struct sockaddr_storage));
			if (curr->ifa_netmask == NULL)
				goto fail;
			(void) memcpy(curr->ifa_netmask, &ifrl6.ifr_Addr,
			    sizeof (struct sockaddr_storage));
		}

		/* Get the destination for a pt-pt interface */
		if (curr->ifa_flags & IFF_POINTOPOINT) {
			if (ifr_af == AF_INET) {
				if (ioctl(s, SIOCGIFDSTADDR, (caddr_t)&ifrl) < 0)
					goto fail;
				curr->ifa_dstaddr = malloc(
				    sizeof (struct sockaddr_storage));
				if (curr->ifa_dstaddr == NULL)
					goto fail;
				(void) memcpy(curr->ifa_dstaddr, &ifrl.ifr_addr,
				    sizeof (struct sockaddr_storage));
			} else {
				if (ioctl(s, SIOCGIFDSTADDR6, (caddr_t)&ifrl6) < 0)
					goto fail;
				curr->ifa_dstaddr = malloc(
				    sizeof (struct sockaddr_storage));
				if (curr->ifa_dstaddr == NULL)
					goto fail;
				(void) memcpy(curr->ifa_dstaddr, &ifrl6.ifr_Addr,
				    sizeof (struct sockaddr_storage));
			}
			/* Do not get broadcast address for IPv6 */
		} else if ((curr->ifa_flags & IFF_BROADCAST) && (ifr_af == AF_INET)) {
			if (ioctl(s, SIOCGIFBRDADDR, (caddr_t)&ifrl) < 0)
				goto fail;
			curr->ifa_broadaddr = malloc(
			    sizeof (struct sockaddr_storage));
			if (curr->ifa_broadaddr == NULL)
				goto fail;
			(void) memcpy(curr->ifa_broadaddr, &ifrl.ifr_addr,
			    sizeof (struct sockaddr_storage));
		}
next:
		s_ifrp += ifsize;
	}
	free(buf);
	close(sock4);
	close(sock6);
	return (0);
fail:
	err = errno;
	free(buf);
	freeifaddrs(*ifap);
	*ifap = NULL;
	if (err == ENXIO)
		goto retry;
	close(sock4);
	close(sock6);
	errno = err;
	return (-1);
}

/*
 * Do a SIOCGIFCONF and store all the interfaces in `buf'.
 */
int
getallifs(int s, sa_family_t af, struct ifreq **ifr, int *iflen,
		int64_t ifc_flags)
{
	int ifsize;
	struct ifconf ifc;
	size_t bufsize;
	char *tmp;
	caddr_t *buf = (caddr_t *)ifr;

	*buf = NULL;
retry:
	if (ioctl(s, SIOCGSIZIFCONF, &ifsize) < 0)
		goto fail;

	/*
	 * When calculating the buffer size needed, add a small number
	 * of interfaces to those we counted.  We do this to capture
	 * the interface status of potential interfaces which may have
	 * been plumbed between the SIOCGSIZIFCONF and the SIOCGIFCONF.
	 */
	bufsize = ifsize + (4 * sizeof (struct in6_ifreq));

	if ((tmp = realloc(*buf, bufsize)) == NULL)
		goto fail;

	*buf = tmp;
	ifc.ifc_buf = *buf;
	ifc.ifc_len = bufsize;
	if (ioctl(s, SIOCGIFCONF, (char *)&ifc) < 0)
		goto fail;

	*iflen = ifc.ifc_len;
	if (*iflen >= bufsize) {
		/*
		 * If every entry was filled, there are probably
		 * more interfaces than (ifn + 4)
		 * Redo the ioctls SIOCGSIZIFCONF and SIOCGIFCONF to
		 * get all the interfaces.
		 */
		goto retry;
	}
	return (0);
fail:
	free(*buf);
	*buf = NULL;
	return (-1);
}
#else /* _AIX */
int
getallifaddrs(sa_family_t af, struct ifaddrs **ifap, int64_t flags)
{
	struct lifreq *buf = NULL;
	struct lifreq *lifrp;
	struct lifreq lifrl;
	int ret;
	int s, n, numifs;
	struct ifaddrs *curr, *prev;
	sa_family_t lifr_af;
	int sock4;
	int sock6;
	int err;

	if ((sock4 = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
		return (-1);
	if ((sock6 = socket(AF_INET6, SOCK_DGRAM, 0)) < 0) {
		err = errno;
		close(sock4);
		errno = err;
		return (-1);
	}

retry:
	/* Get all interfaces from SIOCGLIFCONF */
	ret = getallifs(sock4, af, &buf, &numifs, (flags & ~LIFC_ENABLED));
	if (ret != 0)
		goto fail;

	/*
	 * Loop through the interfaces obtained from SIOCGLIFCOMF
	 * and retrieve the addresses, netmask and flags.
	 */
	prev = NULL;
	lifrp = buf;
	*ifap = NULL;
	for (n = 0; n < numifs; n++, lifrp++) {

		/* Prepare for the ioctl call */
		(void) strncpy(lifrl.lifr_name, lifrp->lifr_name,
		    sizeof (lifrl.lifr_name));
		lifr_af = lifrp->lifr_addr.ss_family;
		if (af != AF_UNSPEC && lifr_af != af)
			continue;

		s = (lifr_af == AF_INET ? sock4 : sock6);

		if (ioctl(s, SIOCGLIFFLAGS, (caddr_t)&lifrl) < 0)
			goto fail;
		if ((flags & LIFC_ENABLED) && !(lifrl.lifr_flags & IFF_UP))
			continue;

		/*
		 * Allocate the current list node. Each node contains data
		 * for one ifaddrs structure.
		 */
		curr = calloc(1, sizeof (struct ifaddrs));
		if (curr == NULL)
			goto fail;

		if (prev != NULL) {
			prev->ifa_next = curr;
		} else {
			/* First node in the linked list */
			*ifap = curr;
		}
		prev = curr;

		curr->ifa_flags = lifrl.lifr_flags;
		if ((curr->ifa_name = strdup(lifrp->lifr_name)) == NULL)
			goto fail;

		curr->ifa_addr = malloc(sizeof (struct sockaddr_storage));
		if (curr->ifa_addr == NULL)
			goto fail;
		(void) memcpy(curr->ifa_addr, &lifrp->lifr_addr,
		    sizeof (struct sockaddr_storage));

		/* Get the netmask */
		if (ioctl(s, SIOCGLIFNETMASK, (caddr_t)&lifrl) < 0)
			goto fail;
		curr->ifa_netmask = malloc(sizeof (struct sockaddr_storage));
		if (curr->ifa_netmask == NULL)
			goto fail;
		(void) memcpy(curr->ifa_netmask, &lifrl.lifr_addr,
		    sizeof (struct sockaddr_storage));

		/* Get the destination for a pt-pt interface */
		if (curr->ifa_flags & IFF_POINTOPOINT) {
			if (ioctl(s, SIOCGLIFDSTADDR, (caddr_t)&lifrl) < 0)
				goto fail;
			curr->ifa_dstaddr = malloc(
			    sizeof (struct sockaddr_storage));
			if (curr->ifa_dstaddr == NULL)
				goto fail;
			(void) memcpy(curr->ifa_dstaddr, &lifrl.lifr_addr,
			    sizeof (struct sockaddr_storage));
		} else if (curr->ifa_flags & IFF_BROADCAST) {
			if (ioctl(s, SIOCGLIFBRDADDR, (caddr_t)&lifrl) < 0)
				goto fail;
			curr->ifa_broadaddr = malloc(
			    sizeof (struct sockaddr_storage));
			if (curr->ifa_broadaddr == NULL)
				goto fail;
			(void) memcpy(curr->ifa_broadaddr, &lifrl.lifr_addr,
			    sizeof (struct sockaddr_storage));
		}

	}
	free(buf);
	close(sock4);
	close(sock6);
	return (0);
fail:
	err = errno;
	free(buf);
	freeifaddrs(*ifap);
	*ifap = NULL;
	if (err == ENXIO)
		goto retry;
	close(sock4);
	close(sock6);
	errno = err;
	return (-1);
}

/*
 * Do a SIOCGLIFCONF and store all the interfaces in `buf'.
 */
int
getallifs(int s, sa_family_t af, struct lifreq **lifr, int *numifs,
		int64_t lifc_flags)
{
	struct lifnum lifn;
	struct lifconf lifc;
	size_t bufsize;
	char *tmp;
	caddr_t *buf = (caddr_t *)lifr;

	lifn.lifn_family = af;
	lifn.lifn_flags = lifc_flags;

	*buf = NULL;
retry:
	if (ioctl(s, SIOCGLIFNUM, &lifn) < 0)
		goto fail;

	/*
	 * When calculating the buffer size needed, add a small number
	 * of interfaces to those we counted.  We do this to capture
	 * the interface status of potential interfaces which may have
	 * been plumbed between the SIOCGLIFNUM and the SIOCGLIFCONF.
	 */
	bufsize = (lifn.lifn_count + 4) * sizeof (struct lifreq);

	if ((tmp = realloc(*buf, bufsize)) == NULL)
		goto fail;

	*buf = tmp;
	lifc.lifc_family = af;
	lifc.lifc_flags = lifc_flags;
	lifc.lifc_len = bufsize;
	lifc.lifc_buf = *buf;
	if (ioctl(s, SIOCGLIFCONF, (char *)&lifc) < 0)
		goto fail;

	*numifs = lifc.lifc_len / sizeof (struct lifreq);
	if (*numifs >= (lifn.lifn_count + 4)) {
		/*
		 * If every entry was filled, there are probably
		 * more interfaces than (lifn.lifn_count + 4).
		 * Redo the ioctls SIOCGLIFNUM and SIOCGLIFCONF to
		 * get all the interfaces.
		 */
		goto retry;
	}
	return (0);
fail:
	free(*buf);
	*buf = NULL;
	return (-1);
}
#endif /* _AIX */
#endif /* HAVE_GETIFADDRS */
