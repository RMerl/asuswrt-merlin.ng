/*
 * Copyright 2010 Oracle.  All rights reserved.
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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <string.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <errno.h>

#include "sockaddr.h"
#include "exportfs.h"

/**
 * host_ntop - generate presentation address given a sockaddr
 * @sap: pointer to socket address
 * @buf: working storage
 * @buflen: size of @buf in bytes
 *
 * Returns a pointer to a @buf.
 */
#ifdef HAVE_GETNAMEINFO
char *
host_ntop(const struct sockaddr *sap, char *buf, const size_t buflen)
{
	socklen_t salen = nfs_sockaddr_length(sap);
	int error;

	memset(buf, 0, buflen);

	if (salen == 0) {
		(void)strncpy(buf, "bad family", buflen - 1);
		return buf;
	}

	error = getnameinfo(sap, salen, buf, (socklen_t)buflen,
						NULL, 0, NI_NUMERICHOST);
	if (error != 0) {
		buf[0] = '\0';
		(void)strncpy(buf, "bad address", buflen - 1);
	}

	return buf;
}
#else	/* !HAVE_GETNAMEINFO */
char *
host_ntop(const struct sockaddr *sap, char *buf, const size_t buflen)
{
	const struct sockaddr_in *sin = (const struct sockaddr_in *)(char *)sap;

	memset(buf, 0, buflen);

	if (sin->sin_family != AF_INET) {
		(void)strncpy(buf, "bad family", buflen - 1);
		return buf;
	}

	if (inet_ntop(AF_INET, &sin->sin_addr.s_addr, buf, buflen) != NULL)
		return buf;

	buf[0] = '\0';
	(void)strncpy(buf, "bad address", buflen - 1);
	return buf;
}
#endif	/* !HAVE_GETNAMEINFO */

/**
 * host_pton - return addrinfo for a given presentation address
 * @paddr: pointer to a '\0'-terminated ASCII string containing an
 *		IP presentation address
 *
 * Returns address info structure, or NULL if an error occurs.  Caller
 * must free the returned structure with freeaddrinfo(3).
 */
__attribute__((__malloc__))
struct addrinfo *
host_pton(const char *paddr)
{
	struct addrinfo *ai = NULL;
	struct addrinfo hint = {
		/* don't return duplicates */
		.ai_protocol	= (int)IPPROTO_UDP,
		.ai_flags	= AI_NUMERICHOST,
		.ai_family	= AF_UNSPEC,
	};
	struct sockaddr_in sin;
	int error, inet4;

	/*
	 * Although getaddrinfo(3) is easier to use and supports
	 * IPv6, it recognizes incomplete addresses like "10.4"
	 * as valid AF_INET addresses.  It also accepts presentation
	 * addresses that end with a blank.
	 *
	 * inet_pton(3) is much stricter.  Use it to be certain we
	 * have a real AF_INET presentation address, before invoking
	 * getaddrinfo(3) to generate the full addrinfo list.
	 */
	if (paddr == NULL) {
		xlog(D_GENERAL, "%s: passed a NULL presentation address",
			__func__);
		return NULL;
	}
	inet4 = 1;
	if (inet_pton(AF_INET, paddr, &sin.sin_addr) == 0)
		inet4 = 0;

	error = getaddrinfo(paddr, NULL, &hint, &ai);
	switch (error) {
	case 0:
		if (!inet4 && ai->ai_addr->sa_family == AF_INET) {
			xlog(D_GENERAL, "%s: failed to convert %s",
					__func__, paddr);
			freeaddrinfo(ai);
			break;
		}
		return ai;
	case EAI_NONAME:
		break;
	case EAI_SYSTEM:
		xlog(L_WARNING, "%s: failed to convert %s: (%d) %m",
				__func__, paddr, errno);
		break;
	default:
		xlog(L_WARNING, "%s: failed to convert %s: %s",
				__func__, paddr, gai_strerror(error));
		break;
	}

	return NULL;
}

/**
 * host_addrinfo - return addrinfo for a given hostname
 * @hostname: pointer to a '\0'-terminated ASCII string containing a hostname
 *
 * Returns address info structure with ai_canonname filled in, or NULL
 * if no information is available for @hostname.  Caller must free the
 * returned structure with freeaddrinfo(3).
 */
__attribute__((__malloc__))
struct addrinfo *
host_addrinfo(const char *hostname)
{
	struct addrinfo *ai = NULL;
	struct addrinfo hint = {
#ifdef IPV6_SUPPORTED
		.ai_family	= AF_UNSPEC,
#else
		.ai_family	= AF_INET,
#endif
		/* don't return duplicates */
		.ai_protocol	= (int)IPPROTO_UDP,
		.ai_flags	= AI_CANONNAME,
	};
	int error;

	error = getaddrinfo(hostname, NULL, &hint, &ai);
	switch (error) {
	case 0:
		return ai;
	case EAI_SYSTEM:
		xlog(D_PARSE, "%s: failed to resolve %s: (%d) %m",
				__func__, hostname, errno);
		break;
	default:
		xlog(D_PARSE, "%s: failed to resolve %s: %s",
				__func__, hostname, gai_strerror(error));
		break;
	}

	return NULL;
}

/**
 * host_canonname - return canonical hostname bound to an address
 * @sap: pointer to socket address to look up
 *
 * Discover the canonical hostname associated with the given socket
 * address.  The host's reverse mapping is verified in the process.
 *
 * Returns a '\0'-terminated ASCII string containing a hostname, or
 * NULL if no hostname can be found for @sap.  Caller must free
 * the string.
 */
#ifdef HAVE_GETNAMEINFO
__attribute__((__malloc__))
char *
host_canonname(const struct sockaddr *sap)
{
	socklen_t salen = nfs_sockaddr_length(sap);
	char buf[NI_MAXHOST];
	int error;

	if (salen == 0) {
		xlog(D_GENERAL, "%s: unsupported address family %d",
				__func__, sap->sa_family);
		return NULL;
	}

	memset(buf, 0, sizeof(buf));
	error = getnameinfo(sap, salen, buf, (socklen_t)sizeof(buf),
							NULL, 0, NI_NAMEREQD);
	switch (error) {
	case 0:
		break;
	case EAI_SYSTEM:
		xlog(D_GENERAL, "%s: getnameinfo(3) failed: (%d) %m",
				__func__, errno);
		return NULL;
	default:
		(void)getnameinfo(sap, salen, buf, (socklen_t)sizeof(buf),
							NULL, 0, NI_NUMERICHOST);
		xlog(D_PARSE, "%s: failed to resolve %s: %s",
				__func__, buf, gai_strerror(error));
		return NULL;
	}

	return strdup(buf);
}
#else	/* !HAVE_GETNAMEINFO */
__attribute__((__malloc__))
char *
host_canonname(const struct sockaddr *sap)
{
	const struct sockaddr_in *sin = (const struct sockaddr_in *)(char *)sap;
	const struct in_addr *addr = &sin->sin_addr;
	struct hostent *hp;

	if (sap->sa_family != AF_INET)
		return NULL;

	hp = gethostbyaddr(addr, (socklen_t)sizeof(addr), AF_INET);
	if (hp == NULL)
		return NULL;

	return strdup(hp->h_name);
}
#endif	/* !HAVE_GETNAMEINFO */

/**
 * host_reliable_addrinfo - return addrinfo for a given address
 * @sap: pointer to socket address to look up
 *
 * Reverse and forward lookups are performed to ensure the address has
 * matching forward and reverse mappings.
 *
 * Returns addrinfo structure with just the provided address with
 * ai_canonname filled in. If there is a problem with resolution or
 * the resolved records don't match up properly then it returns NULL
 *
 * Caller must free the returned structure with freeaddrinfo(3).
 */
__attribute__((__malloc__))
struct addrinfo *
host_reliable_addrinfo(const struct sockaddr *sap)
{
	struct addrinfo *ai, *a;
	char *hostname;

	hostname = host_canonname(sap);
	if (hostname == NULL)
		return NULL;

	ai = host_addrinfo(hostname);
	if (!ai)
		goto out_free_hostname;

	/* make sure there's a matching address in the list */
	for (a = ai; a; a = a->ai_next)
		if (nfs_compare_sockaddr(a->ai_addr, sap))
			break;

	freeaddrinfo(ai);
	if (!a)
		goto out_free_hostname;

	/* get addrinfo with just the original address */
	ai = host_numeric_addrinfo(sap);
	if (!ai)
		goto out_free_hostname;

	/* and populate its ai_canonname field */
	free(ai->ai_canonname);
	ai->ai_canonname = hostname;
	return ai;

out_free_hostname:
	free(hostname);
	return NULL;
}

/**
 * host_numeric_addrinfo - return addrinfo without doing DNS queries
 * @sap: pointer to socket address
 *
 * Returns address info structure, or NULL if an error occurred.
 * Caller must free the returned structure with freeaddrinfo(3).
 */
#ifdef HAVE_GETNAMEINFO
__attribute__((__malloc__))
struct addrinfo *
host_numeric_addrinfo(const struct sockaddr *sap)
{
	socklen_t salen = nfs_sockaddr_length(sap);
	char buf[INET6_ADDRSTRLEN];
	struct addrinfo *ai;
	int error;

	if (salen == 0) {
		xlog(D_GENERAL, "%s: unsupported address family %d",
				__func__, sap->sa_family);
		return NULL;
	}

	memset(buf, 0, sizeof(buf));
	error = getnameinfo(sap, salen, buf, (socklen_t)sizeof(buf),
						NULL, 0, NI_NUMERICHOST);
	switch (error) {
	case 0:
		break;
	case EAI_SYSTEM:
		xlog(D_GENERAL, "%s: getnameinfo(3) failed: (%d) %m",
				__func__, errno);
		return NULL;
	default:
		xlog(D_GENERAL, "%s: getnameinfo(3) failed: %s",
				__func__, gai_strerror(error));
		return NULL;
	}

	ai = host_pton(buf);

	/*
	 * getaddrinfo(AI_NUMERICHOST) never fills in ai_canonname
	 */
	if (ai != NULL) {
		free(ai->ai_canonname);		/* just in case */
		ai->ai_canonname = strdup(buf);
		if (ai->ai_canonname == NULL) {
			freeaddrinfo(ai);
			ai = NULL;
		}
	}

	return ai;
}
#else	/* !HAVE_GETNAMEINFO */
__attribute__((__malloc__))
struct addrinfo *
host_numeric_addrinfo(const struct sockaddr *sap)
{
	const struct sockaddr_in *sin = (const struct sockaddr_in *)sap;
	const struct in_addr *addr = &sin->sin_addr;
	char buf[INET_ADDRSTRLEN];
	struct addrinfo *ai;

	if (sap->sa_family != AF_INET)
		return NULL;

	memset(buf, 0, sizeof(buf));
	if (inet_ntop(AF_INET, (char *)addr, buf,
					(socklen_t)sizeof(buf)) == NULL)
		return NULL;

	ai = host_pton(buf);

	/*
	 * getaddrinfo(AI_NUMERICHOST) never fills in ai_canonname
	 */
	if (ai != NULL) {
		ai->ai_canonname = strdup(buf);
		if (ai->ai_canonname == NULL) {
			freeaddrinfo(ai);
			ai = NULL;
		}
	}

	return ai;
}
#endif	/* !HAVE_GETNAMEINFO */
