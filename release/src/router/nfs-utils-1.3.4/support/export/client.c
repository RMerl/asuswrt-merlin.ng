/*
 * support/export/client.c
 *
 * Maintain list of nfsd clients.
 *
 * Copyright (C) 1995, 1996 Olaf Kirch <okir@monad.swb.de>
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <netdb.h>
#include <errno.h>

#include "sockaddr.h"
#include "misc.h"
#include "nfslib.h"
#include "exportfs.h"

/* netgroup stuff never seems to be defined in any header file. Linux is
 * not alone in this.
 */
#if !defined(__GLIBC__) || __GLIBC__ < 2
extern int	innetgr(char *netgr, char *host, char *, char *);
#endif

static char	*add_name(char *old, const char *add);

nfs_client	*clientlist[MCL_MAXTYPES] = { NULL, };


static void
init_addrlist(nfs_client *clp, const struct addrinfo *ai)
{
	int i;

	if (ai == NULL)
		return;

	for (i = 0; (ai != NULL) && (i < NFSCLNT_ADDRMAX); i++) {
		set_addrlist(clp, i, ai->ai_addr);
		ai = ai->ai_next;
	}

	clp->m_naddr = i;
}

static void
client_free(nfs_client *clp)
{
	free(clp->m_hostname);
	free(clp);
}

static int
init_netmask4(nfs_client *clp, const char *slash)
{
	struct sockaddr_in sin = {
		.sin_family		= AF_INET,
	};
	uint32_t shift;

	/*
	 * Decide what kind of netmask was specified.  If there's
	 * no '/' present, assume the netmask is all ones.  If
	 * there is a '/' and at least one '.', look for a spelled-
	 * out netmask.  Otherwise, assume it was a prefixlen.
	 */
	if (slash == NULL)
		shift = 0;
	else {
		unsigned long prefixlen;

		if (strchr(slash + 1, '.') != NULL) {
			if (inet_pton(AF_INET, slash + 1,
						&sin.sin_addr.s_addr) == 0)
				goto out_badmask;
			set_addrlist_in(clp, 1, &sin);
			return 1;
		} else {
			char *endptr;

			prefixlen = strtoul(slash + 1, &endptr, 10);
			if (*endptr != '\0' && prefixlen != ULONG_MAX &&
			    errno != ERANGE)
				goto out_badprefix;
		}
		if (prefixlen > 32)
			goto out_badprefix;
		shift = 32 - (uint32_t)prefixlen;
	}

	/*
	 * Now construct the full netmask bitmask in a sockaddr_in,
	 * and plant it in the nfs_client record.
	 */
	sin.sin_addr.s_addr = htonl((uint32_t)~0 << shift);
	set_addrlist_in(clp, 1, &sin);

	return 1;

out_badmask:
	xlog(L_ERROR, "Invalid netmask `%s' for %s", slash + 1, clp->m_hostname);
	return 0;

out_badprefix:
	xlog(L_ERROR, "Invalid prefix `%s' for %s", slash + 1, clp->m_hostname);
	return 0;
}

#ifdef IPV6_SUPPORTED
static int
init_netmask6(nfs_client *clp, const char *slash)
{
	struct sockaddr_in6 sin6 = {
		.sin6_family		= AF_INET6,
	};
	unsigned long prefixlen;
	uint32_t shift;
	int i;

	/*
	 * Decide what kind of netmask was specified.  If there's
	 * no '/' present, assume the netmask is all ones.  If
	 * there is a '/' and at least one ':', look for a spelled-
	 * out netmask.  Otherwise, assume it was a prefixlen.
	 */
	if (slash == NULL)
		prefixlen = 128;
	else {
		if (strchr(slash + 1, ':') != NULL) {
			if (!inet_pton(AF_INET6, slash + 1, &sin6.sin6_addr))
				goto out_badmask;
			set_addrlist_in6(clp, 1, &sin6);
			return 1;
		} else {
			char *endptr;

			prefixlen = strtoul(slash + 1, &endptr, 10);
			if (*endptr != '\0' && prefixlen != ULONG_MAX &&
			    errno != ERANGE)
				goto out_badprefix;
		}
		if (prefixlen > 128)
			goto out_badprefix;
	}

	/*
	 * Now construct the full netmask bitmask in a sockaddr_in6,
	 * and plant it in the nfs_client record.
	 */
	for (i = 0; prefixlen > 32; i++) {
		sin6.sin6_addr.s6_addr32[i] = 0xffffffff;
		prefixlen -= 32;
	}
	shift = 32 - (uint32_t)prefixlen;
	sin6.sin6_addr.s6_addr32[i] = htonl((uint32_t)~0 << shift);
	set_addrlist_in6(clp, 1, &sin6);

	return 1;

out_badmask:
	xlog(L_ERROR, "Invalid netmask `%s' for %s", slash + 1, clp->m_hostname);
	return 0;

out_badprefix:
	xlog(L_ERROR, "Invalid prefix `%s' for %s", slash + 1, clp->m_hostname);
	return 0;
}
#else	/* IPV6_SUPPORTED */
static int
init_netmask6(nfs_client *UNUSED(clp), const char *UNUSED(slash))
{
	return 0;
}
#endif	/* IPV6_SUPPORTED */

/*
 * Parse the network mask for M_SUBNETWORK type clients.
 *
 * Return TRUE if successful, or FALSE if some error occurred.
 */
static int
init_subnetwork(nfs_client *clp)
{
	struct addrinfo *ai;
	sa_family_t family;
	int result = 0;
	char *slash;

	slash = strchr(clp->m_hostname, '/');
	if (slash != NULL) {
		*slash = '\0';
		ai = host_pton(clp->m_hostname);
		*slash = '/';
	} else
		ai = host_pton(clp->m_hostname);
	if (ai == NULL) {
		xlog(L_ERROR, "Invalid IP address %s", clp->m_hostname);
		return result;
	}

	set_addrlist(clp, 0, ai->ai_addr);
	family = ai->ai_addr->sa_family;

	freeaddrinfo(ai);

	switch (family) {
	case AF_INET:
		result = init_netmask4(clp, slash);
		break;
	case AF_INET6:
		result = init_netmask6(clp, slash);
		break;
	default:
		xlog(L_ERROR, "Unsupported address family for %s",
			clp->m_hostname);
	}

	return result;
}

static int
client_init(nfs_client *clp, const char *hname, const struct addrinfo *ai)
{
	clp->m_hostname = strdup(hname);
	if (clp->m_hostname == NULL)
		return 0;

	clp->m_exported = 0;
	clp->m_count = 0;
	clp->m_naddr = 0;

	if (clp->m_type == MCL_SUBNETWORK)
		return init_subnetwork(clp);

	init_addrlist(clp, ai);
	return 1;
}

static void
client_add(nfs_client *clp)
{
	nfs_client **cpp;

	cpp = &clientlist[clp->m_type];
	while (*cpp != NULL)
		cpp = &((*cpp)->m_next);
	clp->m_next = NULL;
	*cpp = clp;
}

/**
 * client_lookup - look for @hname in our list of cached nfs_clients
 * @hname: '\0'-terminated ASCII string containing hostname to look for
 * @canonical: if set, @hname is known to be canonical DNS name
 *
 * Returns pointer to a matching or freshly created nfs_client.  NULL
 * is returned if some problem occurs.
 */
nfs_client *
client_lookup(char *hname, int canonical)
{
	nfs_client	*clp = NULL;
	int		htype;
	struct addrinfo	*ai = NULL;

	htype = client_gettype(hname);

	if (htype == MCL_FQDN && !canonical) {
		ai = host_addrinfo(hname);
		if (!ai) {
			xlog(L_WARNING, "Failed to resolve %s", hname);
			goto out;
		}
		hname = ai->ai_canonname;

		for (clp = clientlist[htype]; clp; clp = clp->m_next)
			if (client_check(clp, ai))
				break;
	} else {
		for (clp = clientlist[htype]; clp; clp = clp->m_next) {
			if (strcasecmp(hname, clp->m_hostname)==0)
				break;
		}
	}

	if (clp == NULL) {
		clp = calloc(1, sizeof(*clp));
		if (clp == NULL)
			goto out;
		clp->m_type = htype;
		if (!client_init(clp, hname, NULL)) {
			client_free(clp);
			clp = NULL;
			goto out;
		}
		client_add(clp);
	}

	if (htype == MCL_FQDN && clp->m_naddr == 0)
		init_addrlist(clp, ai);

out:
	freeaddrinfo(ai);
	return clp;
}

/**
 * client_dup - create a copy of an nfs_client
 * @clp: pointer to nfs_client to copy
 * @ai: pointer to addrinfo used to initialize the new client's addrlist
 *
 * Returns a dynamically allocated nfs_client if successful, or
 * NULL if some problem occurs.  Caller must free the returned
 * nfs_client with free(3).
 */
nfs_client *
client_dup(const nfs_client *clp, const struct addrinfo *ai)
{
	nfs_client		*new;

	new = (nfs_client *)malloc(sizeof(*new));
	if (new == NULL)
		return NULL;
	memcpy(new, clp, sizeof(*new));
	new->m_type = MCL_FQDN;
	new->m_hostname = NULL;

	if (!client_init(new, ai->ai_canonname, ai)) {
		client_free(new);
		return NULL;
	}
	client_add(new);
	return new;
}

/**
 * client_release - drop a reference to an nfs_client record
 *
 */
void
client_release(nfs_client *clp)
{
	if (clp->m_count <= 0)
		xlog(L_FATAL, "client_free: m_count <= 0!");
	clp->m_count--;
}

/**
 * client_freeall - deallocate all nfs_client records
 *
 */
void
client_freeall(void)
{
	nfs_client	*clp, **head;
	int		i;

	for (i = 0; i < MCL_MAXTYPES; i++) {
		head = clientlist + i;
		while (*head) {
			*head = (clp = *head)->m_next;
			client_free(clp);
		}
	}
}

/**
 * client_resolve - look up an IP address
 * @sap: pointer to socket address to resolve
 *
 * Returns an addrinfo structure, or NULL if some problem occurred.
 * Caller must free the result with freeaddrinfo(3).
 */
struct addrinfo *
client_resolve(const struct sockaddr *sap)
{
	struct addrinfo *ai = NULL;

	if (clientlist[MCL_WILDCARD] || clientlist[MCL_NETGROUP])
		ai = host_reliable_addrinfo(sap);
	if (ai == NULL)
		ai = host_numeric_addrinfo(sap);

	return ai;
}

/**
 * client_compose - Make a list of cached hostnames that match an IP address
 * @ai: pointer to addrinfo containing IP address information to match
 *
 * Gather all known client hostnames that match the IP address, and sort
 * the result into a comma-separated list.
 *
 * Returns a '\0'-terminated ASCII string containing a comma-separated
 * sorted list of client hostnames, or NULL if no client records matched
 * the IP address or memory could not be allocated.  Caller must free the
 * returned string with free(3).
 */
char *
client_compose(const struct addrinfo *ai)
{
	char *name = NULL;
	int i;

	for (i = 0 ; i < MCL_MAXTYPES; i++) {
		nfs_client	*clp;
		for (clp = clientlist[i]; clp ; clp = clp->m_next) {
			if (!client_check(clp, ai))
				continue;
			name = add_name(name, clp->m_hostname);
		}
	}
	return name;
}

/**
 * client_member - check if @name is contained in the list @client
 * @client: '\0'-terminated ASCII string containing
 *		comma-separated list of hostnames
 * @name: '\0'-terminated ASCII string containing hostname to look for
 *
 * Returns 1 if @name was found in @client, otherwise zero is returned.
 */
int
client_member(const char *client, const char *name)
{
	size_t l = strlen(name);

	while (*client) {
		if (strncmp(client, name, l) == 0 &&
		    (client[l] == ',' || client[l] == '\0'))
			return 1;
		client = strchr(client, ',');
		if (client == NULL)
			return 0;
		client++;
	}
	return 0;
}

static int
name_cmp(const char *a, const char *b)
{
	/* compare strings a and b, but only upto ',' in a */
	while (*a && *b && *a != ',' && *a == *b)
		a++, b++;
	if (!*b && (!*a || *a == ','))
		return 0;
	if (!*b) return 1;
	if (!*a || *a == ',') return -1;
	return *a - *b;
}

static char *
add_name(char *old, const char *add)
{
	size_t len = strlen(add) + 2;
	char *new;
	char *cp;
	if (old) len += strlen(old);
	
	new = malloc(len);
	if (!new) {
		free(old);
		return NULL;
	}
	cp = old;
	while (cp && *cp && name_cmp(cp, add) < 0) {
		/* step cp forward over a name */
		char *e = strchr(cp, ',');
		if (e)
			cp = e+1;
		else
			cp = cp + strlen(cp);
	}
	if (old) {
		strncpy(new, old, cp-old);
		new[cp-old] = 0;
	} else {
		new[0] = 0;
	}
	if (cp != old && !*cp)
		strcat(new, ",");
	strcat(new, add);
	if (cp && *cp) {
		strcat(new, ",");
		strcat(new, cp);
	}
	free(old);
	return new;
}

/*
 * Check each address listed in @ai against each address
 * stored in @clp.  Return 1 if a match is found, otherwise
 * zero.
 */
static int
check_fqdn(const nfs_client *clp, const struct addrinfo *ai)
{
	int i;

	for (; ai; ai = ai->ai_next)
		for (i = 0; i < clp->m_naddr; i++)
			if (nfs_compare_sockaddr(ai->ai_addr,
							get_addrlist(clp, i)))
				return 1;

	return 0;
}

static _Bool
mask_match(const uint32_t a, const uint32_t b, const uint32_t m)
{
	return ((a ^ b) & m) == 0;
}

static int
check_subnet_v4(const struct sockaddr_in *address,
		const struct sockaddr_in *mask, const struct addrinfo *ai)
{
	for (; ai; ai = ai->ai_next) {
		struct sockaddr_in *sin = (struct sockaddr_in *)ai->ai_addr;

		if (sin->sin_family != AF_INET)
			continue;

		if (mask_match(address->sin_addr.s_addr,
				sin->sin_addr.s_addr,
				mask->sin_addr.s_addr))
			return 1;
	}
	return 0;
}

#ifdef IPV6_SUPPORTED
static int
check_subnet_v6(const struct sockaddr_in6 *address,
		const struct sockaddr_in6 *mask, const struct addrinfo *ai)
{
	for (; ai; ai = ai->ai_next) {
		struct sockaddr_in6 *sin6 = (struct sockaddr_in6 *)ai->ai_addr;

		if (sin6->sin6_family != AF_INET6)
			continue;

		if (mask_match(address->sin6_addr.s6_addr32[0],
				sin6->sin6_addr.s6_addr32[0],
				mask->sin6_addr.s6_addr32[0]) &&
		    mask_match(address->sin6_addr.s6_addr32[1],
				sin6->sin6_addr.s6_addr32[1],
				mask->sin6_addr.s6_addr32[1]) &&
		    mask_match(address->sin6_addr.s6_addr32[2],
				sin6->sin6_addr.s6_addr32[2],
				mask->sin6_addr.s6_addr32[2]) &&
		    mask_match(address->sin6_addr.s6_addr32[3],
				sin6->sin6_addr.s6_addr32[3],
				mask->sin6_addr.s6_addr32[3]))
			return 1;
	}
	return 0;
}
#else	/* !IPV6_SUPPORTED */
static int
check_subnet_v6(const struct sockaddr_in6 *UNUSED(address),
		const struct sockaddr_in6 *UNUSED(mask),
		const struct addrinfo *UNUSED(ai))
{
	return 0;
}
#endif	/* !IPV6_SUPPORTED */

/*
 * Check each address listed in @ai against the subnetwork or
 * host address stored in @clp.  Return 1 if an address in @hp
 * matches the host address stored in @clp, otherwise zero.
 */
static int
check_subnetwork(const nfs_client *clp, const struct addrinfo *ai)
{
	switch (get_addrlist(clp, 0)->sa_family) {
	case AF_INET:
		return check_subnet_v4(get_addrlist_in(clp, 0),
				get_addrlist_in(clp, 1), ai);
	case AF_INET6:
		return check_subnet_v6(get_addrlist_in6(clp, 0),
				get_addrlist_in6(clp, 1), ai);
	}

	return 0;
}

/*
 * Check if a wildcard nfs_client record matches the canonical name
 * or the aliases of a host.  Return 1 if a match is found, otherwise
 * zero.
 */
static int
check_wildcard(const nfs_client *clp, const struct addrinfo *ai)
{
	char *cname = clp->m_hostname;
	char *hname = ai->ai_canonname;
	struct hostent *hp;
	char **ap;

	if (wildmat(hname, cname))
		return 1;

	/* See if hname aliases listed in /etc/hosts or nis[+]
	 * match the requested wildcard */
	hp = gethostbyname(hname);
	if (hp != NULL) {
		for (ap = hp->h_aliases; *ap; ap++)
			if (wildmat(*ap, cname))
				return 1;
	}

	return 0;
}

/*
 * Check if @ai's hostname or aliases fall in a given netgroup.
 * Return 1 if @ai represents a host in the netgroup, otherwise
 * zero.
 */
#ifdef HAVE_INNETGR
static int
check_netgroup(const nfs_client *clp, const struct addrinfo *ai)
{
	const char *netgroup = clp->m_hostname + 1;
	struct addrinfo *tmp = NULL;
	struct hostent *hp;
	char *dot, *hname, *ip;
	int i, match;

	match = 0;

	hname = strdup(ai->ai_canonname);
	if (hname == NULL) {
		xlog(D_GENERAL, "%s: no memory for strdup", __func__);
		goto out;
	}

	/* First, try to match the hostname without
	 * splitting off the domain */
	if (innetgr(netgroup, hname, NULL, NULL)) {
		match = 1;
		goto out;
	}

	/* See if hname aliases listed in /etc/hosts or nis[+]
	 * match the requested netgroup */
	hp = gethostbyname(hname);
	if (hp != NULL) {
		for (i = 0; hp->h_aliases[i]; i++)
			if (innetgr(netgroup, hp->h_aliases[i], NULL, NULL)) {
				match = 1;
				goto out;
			}
	}

	/* If hname happens to be an IP address, convert it
	 * to a the canonical DNS name bound to this address. */
	tmp = host_pton(hname);
	if (tmp != NULL) {
		char *cname = host_canonname(tmp->ai_addr);
		freeaddrinfo(tmp);

		/* The resulting FQDN may be in our netgroup. */
		if (cname != NULL) {
			free(hname);
			hname = cname;
			if (innetgr(netgroup, hname, NULL, NULL)) {
				match = 1;
				goto out;
			}
		}
	}

	/* check whether the IP itself is in the netgroup */
	ip = calloc(INET6_ADDRSTRLEN, 1);
	if (inet_ntop(ai->ai_family, &(((struct sockaddr_in *)ai->ai_addr)->sin_addr), ip, INET6_ADDRSTRLEN) == ip) {
		if (innetgr(netgroup, ip, NULL, NULL)) {
			free(hname);
			hname = ip;
			match = 1;
			goto out;
		}
	}
	free(ip);

	/* Okay, strip off the domain (if we have one) */
	dot = strchr(hname, '.');
	if (dot == NULL)
		goto out;

	*dot = '\0';
	match = innetgr(netgroup, hname, NULL, NULL);

out:
	free(hname);
	return match;
}
#else	/* !HAVE_INNETGR */
static int
check_netgroup(__attribute__((unused)) const nfs_client *clp,
		__attribute__((unused)) const struct addrinfo *ai)
{
	return 0;
}
#endif	/* !HAVE_INNETGR */

/**
 * client_check - check if IP address information matches a cached nfs_client
 * @clp: pointer to a cached nfs_client record
 * @ai: pointer to addrinfo to compare it with
 *
 * Returns 1 if the address information matches the cached nfs_client,
 * otherwise zero.
 */
int
client_check(const nfs_client *clp, const struct addrinfo *ai)
{
	switch (clp->m_type) {
	case MCL_FQDN:
		return check_fqdn(clp, ai);
	case MCL_SUBNETWORK:
		return check_subnetwork(clp, ai);
	case MCL_WILDCARD:
		return check_wildcard(clp, ai);
	case MCL_NETGROUP:
		return check_netgroup(clp, ai);
	case MCL_ANONYMOUS:
		return 1;
	case MCL_GSS:
		return 0;
	default:
		xlog(D_GENERAL, "%s: unrecognized client type: %d",
				__func__, clp->m_type);
	}

	return 0;
}

/**
 * client_gettype - determine type of nfs_client given an identifier
 * @ident: '\0'-terminated ASCII string containing a client identifier
 *
 * Returns the type of nfs_client record that would be used for
 * this client.
 */
int
client_gettype(char *ident)
{
	char *sp;

	if (ident[0] == '\0' || strcmp(ident, "*")==0)
		return MCL_ANONYMOUS;
	if (strncmp(ident, "gss/", 4) == 0)
		return MCL_GSS;
	if (ident[0] == '@') {
#ifndef HAVE_INNETGR
		xlog(L_WARNING, "netgroup support not compiled in");
#endif
		return MCL_NETGROUP;
	}
	for (sp = ident; *sp; sp++) {
		if (*sp == '*' || *sp == '?' || *sp == '[')
			return MCL_WILDCARD;
		if (*sp == '/')
			return MCL_SUBNETWORK;
		if (*sp == '\\' && sp[1])
			sp++;
	}

	return MCL_FQDN;
}
