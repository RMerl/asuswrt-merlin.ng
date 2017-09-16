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

/*
 * NSM for Linux.
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <sys/types.h>
#include <sys/socket.h>

#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <strings.h>
#include <netdb.h>
#include <arpa/inet.h>

#include "sockaddr.h"
#include "statd.h"
#include "xlog.h"

/**
 * statd_present_address - convert sockaddr to presentation address
 * @sap: pointer to socket address to convert
 * @buf: pointer to buffer to fill in
 * @buflen: length of buffer
 *
 * Convert the passed-in sockaddr-style address to presentation format.
 * The presentation format address is placed in @buf and is
 * '\0'-terminated.
 *
 * Returns true if successful; otherwise false.
 *
 * getnameinfo(3) is preferred, since it can parse IPv6 scope IDs.
 * An alternate version of statd_present_address() is available to
 * handle older glibcs that do not have getnameinfo(3).
 */
#ifdef HAVE_GETNAMEINFO
_Bool
statd_present_address(const struct sockaddr *sap, char *buf, const size_t buflen)
{
	socklen_t salen;
	int error;

	salen = nfs_sockaddr_length(sap);
	if (salen == 0) {
		xlog(D_GENERAL, "%s: unsupported address family",
				__func__);
		return false;
	}

	error = getnameinfo(sap, salen, buf, (socklen_t)buflen,
						NULL, 0, NI_NUMERICHOST);
	if (error != 0) {
		xlog(D_GENERAL, "%s: getnameinfo(3): %s",
				__func__, gai_strerror(error));
		return false;
	}
	return true;
}
#else	/* !HAVE_GETNAMEINFO */
_Bool
statd_present_address(const struct sockaddr *sap, char *buf, const size_t buflen)
{
	const struct sockaddr_in *sin = (const struct sockaddr_in *)sap;

	if (sin->sin_family != AF_INET) {
		xlog(D_GENERAL, "%s: unsupported address family", __func__);
		return false;
	}

	/* ensure '\0' termination */
	memset(buf, 0, buflen);

	if (inet_ntop(AF_INET, (char *)&sin->sin_addr,
					buf, (socklen_t)buflen) == NULL) {
		xlog(D_GENERAL, "%s: inet_ntop(3): %m", __func__);
		return false;
	}
	return true;
}
#endif	/* !HAVE_GETNAMEINFO */

/*
 * Look up the hostname; report exceptional errors.  Caller must
 * call freeaddrinfo(3) if a valid addrinfo is returned.
 */
__attribute__((__malloc__))
static struct addrinfo *
get_addrinfo(const char *hostname, const struct addrinfo *hint)
{
	struct addrinfo *ai = NULL;
	int error;

	error = getaddrinfo(hostname, NULL, hint, &ai);
	switch (error) {
	case 0:
		return ai;
	case EAI_NONAME:
		break;
	default:
		xlog(D_GENERAL, "%s: failed to resolve host %s: %s",
				__func__, hostname, gai_strerror(error));
	}

	return NULL;
}

#ifdef HAVE_GETNAMEINFO
static _Bool
get_nameinfo(const struct sockaddr *sap, const socklen_t salen,
		/*@out@*/ char *buf, const socklen_t buflen)
{
	int error;

	error = getnameinfo(sap, salen, buf, buflen, NULL, 0, NI_NAMEREQD);
	if (error != 0) {
		xlog(D_GENERAL, "%s: failed to resolve address: %s",
				__func__, gai_strerror(error));
		return false;
	}

	return true;
}
#else	/* !HAVE_GETNAMEINFO */
static _Bool
get_nameinfo(const struct sockaddr *sap,
		__attribute__ ((unused)) const socklen_t salen,
		/*@out@*/ char *buf, socklen_t buflen)
{
	struct sockaddr_in *sin = (struct sockaddr_in *)(char *)sap;
	struct hostent *hp;

	if (sin->sin_family != AF_INET) {
		xlog(D_GENERAL, "%s: unknown address family: %d",
				sin->sin_family);
		return false;
	}

	hp = gethostbyaddr((const char *)&(sin->sin_addr.s_addr),
				sizeof(struct in_addr), AF_INET);
	if (hp == NULL) {
		xlog(D_GENERAL, "%s: failed to resolve address: %m", __func__);
		return false;
	}

	strncpy(buf, hp->h_name, (size_t)buflen);
	return true;
}
#endif	/* !HAVE_GETNAMEINFO */

/**
 * statd_canonical_name - choose file name for monitor record files
 * @hostname: C string containing hostname or presentation address
 *
 * Returns a '\0'-terminated ASCII string containing a fully qualified
 * canonical hostname, or NULL if @hostname does not have a reverse
 * mapping.  Caller must free the result with free(3).
 *
 * Incoming hostnames are looked up to determine the canonical hostname,
 * and incoming presentation addresses are converted to canonical
 * hostnames.
 */
__attribute__((__malloc__))
char *
statd_canonical_name(const char *hostname)
{
	struct addrinfo hint = {
#ifdef IPV6_SUPPORTED
		.ai_family	= AF_UNSPEC,
#else	/* !IPV6_SUPPORTED */
		.ai_family	= AF_INET,
#endif	/* !IPV6_SUPPORTED */
		.ai_flags	= AI_NUMERICHOST,
		.ai_protocol	= (int)IPPROTO_UDP,
	};
	char buf[NI_MAXHOST];
	struct addrinfo *ai;

	ai = get_addrinfo(hostname, &hint);
	if (ai != NULL) {
		/* @hostname was a presentation address */
		_Bool result;
		result = get_nameinfo(ai->ai_addr, ai->ai_addrlen,
					buf, (socklen_t)sizeof(buf));
		freeaddrinfo(ai);
		if (!result || buf[0] == '\0')
			/* OK to use presentation address,
			 * if no reverse map exists */
			return strdup(hostname);
		return strdup(buf);
	}

	/* @hostname was a hostname */
	hint.ai_flags = AI_CANONNAME;
	ai = get_addrinfo(hostname, &hint);
	if (ai == NULL)
		return NULL;
	strcpy(buf, ai->ai_canonname);
	freeaddrinfo(ai);

	return strdup(buf);
}

/*
 * Take care to perform an explicit reverse lookup on presentation
 * addresses.  Otherwise we don't get a real canonical name or a
 * complete list of addresses.
 *
 * Returns an addrinfo list that has ai_canonname filled in, or
 * NULL if some error occurs.  Caller must free the returned
 * list with freeaddrinfo(3).
 */
__attribute__((__malloc__))
static struct addrinfo *
statd_canonical_list(const char *hostname)
{
	struct addrinfo hint = {
#ifdef IPV6_SUPPORTED
		.ai_family	= AF_UNSPEC,
#else	/* !IPV6_SUPPORTED */
		.ai_family	= AF_INET,
#endif	/* !IPV6_SUPPORTED */
		.ai_flags	= AI_NUMERICHOST,
		.ai_protocol	= (int)IPPROTO_UDP,
	};
	char buf[NI_MAXHOST];
	struct addrinfo *ai;

	ai = get_addrinfo(hostname, &hint);
	if (ai != NULL) {
		/* @hostname was a presentation address */
		_Bool result;
		result = get_nameinfo(ai->ai_addr, ai->ai_addrlen,
					buf, (socklen_t)sizeof(buf));
		freeaddrinfo(ai);
		if (result)
			goto out;
	}
	/* @hostname was a hostname or had no reverse mapping */
	strcpy(buf, hostname);

out:
	hint.ai_flags = AI_CANONNAME;
	return get_addrinfo(buf, &hint);
}

/**
 * statd_matchhostname - check if two hostnames are equivalent
 * @hostname1: C string containing hostname
 * @hostname2: C string containing hostname
 *
 * Returns true if the hostnames are the same, the hostnames resolve
 * to the same canonical name, or the hostnames resolve to at least
 * one address that is the same.  False is returned if the hostnames
 * do not match in any of these ways, if either hostname contains
 * wildcard characters, if either hostname is a netgroup name, or
 * if an error occurs.
 */
_Bool
statd_matchhostname(const char *hostname1, const char *hostname2)
{
	struct addrinfo *ai1, *ai2, *results1 = NULL, *results2 = NULL;
	_Bool result = false;

	if (strcasecmp(hostname1, hostname2) == 0) {
		result = true;
		goto out;
	}

	results1 = statd_canonical_list(hostname1);
	if (results1 == NULL)
		goto out;
	results2 = statd_canonical_list(hostname2);
	if (results2 == NULL)
		goto out;

	if (strcasecmp(results1->ai_canonname, results2->ai_canonname) == 0) {
		result = true;
		goto out;
	}

	for (ai1 = results1; ai1 != NULL; ai1 = ai1->ai_next)
		for (ai2 = results2; ai2 != NULL; ai2 = ai2->ai_next)
			if (nfs_compare_sockaddr(ai1->ai_addr, ai2->ai_addr)) {
				result = true;
				break;
			}

out:
	freeaddrinfo(results2);
	freeaddrinfo(results1);

	xlog(D_CALL, "%s: hostnames %s and %s %s", __func__,
			hostname1, hostname2,
			(result ? "matched" : "did not match"));
	return result;
}
