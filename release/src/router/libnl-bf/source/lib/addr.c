/*
 * lib/addr.c		Abstract Address
 *
 *	This library is free software; you can redistribute it and/or
 *	modify it under the terms of the GNU Lesser General Public
 *	License as published by the Free Software Foundation version 2.1
 *	of the License.
 *
 * Copyright (c) 2003-2010 Thomas Graf <tgraf@suug.ch>
 */

/**
 * @ingroup core
 * @defgroup addr Abstract Address
 *
 * @par 1) Transform character string to abstract address
 * @code
 * struct nl_addr *a = nl_addr_parse("::1", AF_UNSPEC);
 * printf("Address family: %s\n", nl_af2str(nl_addr_get_family(a)));
 * nl_addr_put(a);
 * a = nl_addr_parse("11:22:33:44:55:66", AF_UNSPEC);
 * printf("Address family: %s\n", nl_af2str(nl_addr_get_family(a)));
 * nl_addr_put(a);
 * @endcode
 * @{
 */

#include <netlink-local.h>
#include <netlink/netlink.h>
#include <netlink/utils.h>
#include <netlink/addr.h>
#include <linux/socket.h>

/* All this DECnet stuff is stolen from iproute2, thanks to whoever wrote
 * this, probably Alexey. */
static inline uint16_t dn_ntohs(uint16_t addr)
{
	union {
		uint8_t byte[2];
		uint16_t word;
	} u = {
		.word = addr,
	};

	return ((uint16_t) u.byte[0]) | (((uint16_t) u.byte[1]) << 8);
}

static inline int do_digit(char *str, uint16_t *addr, uint16_t scale,
			   size_t *pos, size_t len, int *started)
{
	uint16_t tmp = *addr / scale;

	if (*pos == len)
		return 1;

	if (((tmp) > 0) || *started || (scale == 1)) {
		*str = tmp + '0';
		*started = 1;
		(*pos)++;
		*addr -= (tmp * scale);
	}

	return 0;
}

static const char *dnet_ntop(char *addrbuf, size_t addrlen, char *str,
			     size_t len)
{
	uint16_t addr = dn_ntohs(*(uint16_t *)addrbuf);
	uint16_t area = addr >> 10;
	size_t pos = 0;
	int started = 0;

	if (addrlen != 2)
		return NULL;

	addr &= 0x03ff;

	if (len == 0)
		return str;

	if (do_digit(str + pos, &area, 10, &pos, len, &started))
		return str;

	if (do_digit(str + pos, &area, 1, &pos, len, &started))
		return str;

	if (pos == len)
		return str;

	*(str + pos) = '.';
	pos++;
	started = 0;

	if (do_digit(str + pos, &addr, 1000, &pos, len, &started))
		return str;

	if (do_digit(str + pos, &addr, 100, &pos, len, &started))
		return str;

	if (do_digit(str + pos, &addr, 10, &pos, len, &started))
		return str;

	if (do_digit(str + pos, &addr, 1, &pos, len, &started))
		return str;

	if (pos == len)
		return str;

	*(str + pos) = 0;

	return str;
}

static int dnet_num(const char *src, uint16_t * dst)
{
	int rv = 0;
	int tmp;
	*dst = 0;

	while ((tmp = *src++) != 0) {
		tmp -= '0';
		if ((tmp < 0) || (tmp > 9))
			return rv;

		rv++;
		(*dst) *= 10;
		(*dst) += tmp;
	}

	return rv;
}

static inline int dnet_pton(const char *src, char *addrbuf)
{
	uint16_t area = 0;
	uint16_t node = 0;
	int pos;

	pos = dnet_num(src, &area);
	if ((pos == 0) || (area > 63) ||
	    ((*(src + pos) != '.') && (*(src + pos) != ',')))
		return -NLE_INVAL;

	pos = dnet_num(src + pos + 1, &node);
	if ((pos == 0) || (node > 1023))
		return -NLE_INVAL;

	*(uint16_t *)addrbuf = dn_ntohs((area << 10) | node);

	return 1;
}

static void addr_destroy(struct nl_addr *addr)
{
	if (!addr)
		return;

	if (addr->a_refcnt != 1)
		BUG();

	free(addr);
}

/**
 * @name Creating Abstract Addresses
 * @{
 */

/**
 * Allocate new abstract address object.
 * @arg maxsize		Maximum size of the binary address.
 * @return Newly allocated address object or NULL
 */
struct nl_addr *nl_addr_alloc(size_t maxsize)
{
	struct nl_addr *addr;
	
	addr = calloc(1, sizeof(*addr) + maxsize);
	if (!addr)
		return NULL;

	addr->a_refcnt = 1;
	addr->a_maxsize = maxsize;

	return addr;
}

/**
 * Allocate new abstract address object based on a binary address.
 * @arg family		Address family.
 * @arg buf		Buffer containing the binary address.
 * @arg size		Length of binary address buffer.
 * @return Newly allocated address handle or NULL
 */
struct nl_addr *nl_addr_build(int family, void *buf, size_t size)
{
	struct nl_addr *addr;

	addr = nl_addr_alloc(size);
	if (!addr)
		return NULL;

	addr->a_family = family;
	addr->a_len = size;
	addr->a_prefixlen = size*8;

	if (size)
		memcpy(addr->a_addr, buf, size);

	return addr;
}

/**
 * Allocate abstract address based on netlink attribute.
 * @arg nla		Netlink attribute of unspecific type.
 * @arg family		Address family.
 *
 * Considers the netlink attribute payload a address of the specified
 * family and allocates a new abstract address based on it.
 *
 * @return Newly allocated address handle or NULL.
 */
struct nl_addr *nl_addr_alloc_attr(struct nlattr *nla, int family)
{
	return nl_addr_build(family, nla_data(nla), nla_len(nla));
}

/**
 * Allocate abstract address object based on a character string
 * @arg addrstr		Address represented as character string.
 * @arg hint		Address family hint or AF_UNSPEC.
 * @arg result		Pointer to store resulting address.
 *
 * Regognizes the following address formats:
 *@code
 *  Format                      Len                Family
 *  ----------------------------------------------------------------
 *  IPv6 address format         16                 AF_INET6
 *  ddd.ddd.ddd.ddd             4                  AF_INET
 *  HH:HH:HH:HH:HH:HH           6                  AF_LLC
 *  AA{.|,}NNNN                 2                  AF_DECnet
 *  HH:HH:HH:...                variable           AF_UNSPEC
 * @endcode
 *
 *  Special values:
 *    - none: All bits and length set to 0.
 *    - {default|all|any}: All bits set to 0, length based on hint or
 *                         AF_INET if no hint is given.
 *
 * The prefix length may be appened at the end prefixed with a
 * slash, e.g. 10.0.0.0/8.
 *
 * @return 0 on success or a negative error code.
 */
int nl_addr_parse(const char *addrstr, int hint, struct nl_addr **result)
{
	int err, copy = 0, len = 0, family = AF_UNSPEC;
	char *str, *prefix, buf[32];
	struct nl_addr *addr = NULL; /* gcc ain't that smart */

	str = strdup(addrstr);
	if (!str) {
		err = -NLE_NOMEM;
		goto errout;
	}

	prefix = strchr(str, '/');
	if (prefix)
		*prefix = '\0';

	if (!strcasecmp(str, "none")) {
		family = hint;
		goto prefix;
	}

	if (!strcasecmp(str, "default") ||
	    !strcasecmp(str, "all") ||
	    !strcasecmp(str, "any")) {
			
		switch (hint) {
			case AF_INET:
			case AF_UNSPEC:
				/* Kind of a hack, we assume that if there is
				 * no hint given the user wants to have a IPv4
				 * address given back. */
				family = AF_INET;
				len = 4;
				goto prefix;

			case AF_INET6:
				family = AF_INET6;
				len = 16;
				goto prefix;

			case AF_LLC:
				family = AF_LLC;
				len = 6;
				goto prefix;

			default:
				err = -NLE_AF_NOSUPPORT;
				goto errout;
		}
	}

	copy = 1;

	if (hint == AF_INET || hint == AF_UNSPEC) {
		if (inet_pton(AF_INET, str, buf) > 0) {
			family = AF_INET;
			len = 4;
			goto prefix;
		}
		if (hint == AF_INET) {
			err = -NLE_NOADDR;
			goto errout;
		}
	}

	if (hint == AF_INET6 || hint == AF_UNSPEC) {
		if (inet_pton(AF_INET6, str, buf) > 0) {
			family = AF_INET6;
			len = 16;
			goto prefix;
		}
		if (hint == AF_INET6) {
			err = -NLE_NOADDR;
			goto errout;
		}
	}

	if ((hint == AF_LLC || hint == AF_UNSPEC) && strchr(str, ':')) {
		unsigned int a, b, c, d, e, f;

		if (sscanf(str, "%02x:%02x:%02x:%02x:%02x:%02x",
		    &a, &b, &c, &d, &e, &f) == 6) {
			family = AF_LLC;
			len = 6;
			buf[0] = (unsigned char) a;
			buf[1] = (unsigned char) b;
			buf[2] = (unsigned char) c;
			buf[3] = (unsigned char) d;
			buf[4] = (unsigned char) e;
			buf[5] = (unsigned char) f;
			goto prefix;
		}

		if (hint == AF_LLC) {
			err = -NLE_NOADDR;
			goto errout;
		}
	}

	if ((hint == AF_DECnet || hint == AF_UNSPEC) &&
	    (strchr(str, '.') || strchr(str, ','))) {
		if (dnet_pton(str, buf) > 0) {
			family = AF_DECnet;
			len = 2;
			goto prefix;
		}
		if (hint == AF_DECnet) {
			err = -NLE_NOADDR;
			goto errout;
		}
	}

	if (hint == AF_UNSPEC && strchr(str, ':')) {
		int i = 0;
		char *s = str, *p;
		for (;;) {
			long l = strtol(s, &p, 16);

			if (s == p || l > 0xff || i >= sizeof(buf)) {
				err = -NLE_INVAL;
				goto errout;
			}

			buf[i++] = (unsigned char) l;
			if (*p == '\0')
				break;
			s = ++p;
		}

		len = i;
		family = AF_UNSPEC;
		goto prefix;
	}

	err = -NLE_NOADDR;
	goto errout;

prefix:
	addr = nl_addr_alloc(len);
	if (!addr) {
		err = -NLE_NOMEM;
		goto errout;
	}

	nl_addr_set_family(addr, family);

	if (copy)
		nl_addr_set_binary_addr(addr, buf, len);

	if (prefix) {
		char *p;
		long pl = strtol(++prefix, &p, 0);
		if (p == prefix) {
			addr_destroy(addr);
			err = -NLE_INVAL;
			goto errout;
		}
		nl_addr_set_prefixlen(addr, pl);
	} else
		nl_addr_set_prefixlen(addr, len * 8);

	*result = addr;
	err = 0;
errout:
	free(str);

	return err;
}

/**
 * Clone existing abstract address object.
 * @arg addr		Abstract address object.
 * @return Newly allocated abstract address object being a duplicate of the
 *         specified address object or NULL if a failure occured.
 */
struct nl_addr *nl_addr_clone(struct nl_addr *addr)
{
	struct nl_addr *new;

	new = nl_addr_build(addr->a_family, addr->a_addr, addr->a_len);
	if (new)
		new->a_prefixlen = addr->a_prefixlen;

	return new;
}

/** @} */

/**
 * @name Managing Usage References
 * @{
 */

struct nl_addr *nl_addr_get(struct nl_addr *addr)
{
	addr->a_refcnt++;

	return addr;
}

void nl_addr_put(struct nl_addr *addr)
{
	if (!addr)
		return;

	if (addr->a_refcnt == 1)
		addr_destroy(addr);
	else
		addr->a_refcnt--;
}

/**
 * Check whether an abstract address object is shared.
 * @arg addr		Abstract address object.
 * @return Non-zero if the abstract address object is shared, otherwise 0.
 */
int nl_addr_shared(struct nl_addr *addr)
{
	return addr->a_refcnt > 1;
}

/** @} */

/**
 * @name Miscellaneous
 * @{
 */

/**
 * Compares two abstract address objects.
 * @arg a		A abstract address object.
 * @arg b		Another abstract address object.
 *
 * @return Integer less than, equal to or greather than zero if \c is found,
 *         respectively to be less than, to, or be greater than \c b.
 */
int nl_addr_cmp(struct nl_addr *a, struct nl_addr *b)
{
	int d = a->a_family - b->a_family;

	if (d == 0) {
		d = a->a_len - b->a_len;

		if (a->a_len && d == 0)
			d = memcmp(a->a_addr, b->a_addr, a->a_len);

			if (d == 0)
				return (a->a_prefixlen - b->a_prefixlen);
	}

	return d;
}

/**
 * Compares the prefix of two abstract address objects.
 * @arg a		A abstract address object.
 * @arg b		Another abstract address object.
 *
 * @return Integer less than, equal to or greather than zero if \c is found,
 *         respectively to be less than, to, or be greater than \c b.
 */
int nl_addr_cmp_prefix(struct nl_addr *a, struct nl_addr *b)
{
	int d = a->a_family - b->a_family;

	if (d == 0) {
		int len = min(a->a_prefixlen, b->a_prefixlen);
		int bytes = len / 8;

		d = memcmp(a->a_addr, b->a_addr, bytes);
		if (d == 0) {
			int mask = (1UL << (len % 8)) - 1UL;

			d = (a->a_addr[bytes] & mask) -
			    (b->a_addr[bytes] & mask);
		}
	}

	return d;
}

/**
 * Returns true if the address consists of all zeros
 * @arg addr		Address to look at.
 */
int nl_addr_iszero(struct nl_addr *addr)
{
	int i;

	for (i = 0; i < addr->a_len; i++)
		if (addr->a_addr[i])
			return 0;

	return 1;
}

/**
 * Check if an address matches a certain family.
 * @arg addr		Address represented as character string.
 * @arg family		Desired address family.
 *
 * @return 1 if the address is of the desired address family,
 *         otherwise 0 is returned.
 */
int nl_addr_valid(char *addr, int family)
{
	int ret;
	char buf[32];

	switch (family) {
	case AF_INET:
	case AF_INET6:
		ret = inet_pton(family, addr, buf);
		if (ret <= 0)
			return 0;
		break;

	case AF_DECnet:
		ret = dnet_pton(addr, buf);
		if (ret <= 0)
			return 0;
		break;

	case AF_LLC:
		if (sscanf(addr, "%*02x:%*02x:%*02x:%*02x:%*02x:%*02x") != 6)
			return 0;
		break;
	}

	return 1;
}

/**
 * Guess address family of an abstract address object based on address size.
 * @arg addr		Abstract address object.
 * @return Address family or AF_UNSPEC if guessing wasn't successful.
 */
int nl_addr_guess_family(struct nl_addr *addr)
{
	switch (addr->a_len) {
		case 4:
			return AF_INET;
		case 6:
			return AF_LLC;
		case 16:
			return AF_INET6;
		default:
			return AF_UNSPEC;
	}
}

/**
 * Fill out sockaddr structure with values from abstract address object.
 * @arg addr		Abstract address object.
 * @arg sa		Destination sockaddr structure buffer.
 * @arg salen		Length of sockaddr structure buffer.
 *
 * Fills out the specified sockaddr structure with the data found in the
 * specified abstract address. The salen argument needs to be set to the
 * size of sa but will be modified to the actual size used during before
 * the function exits.
 *
 * @return 0 on success or a negative error code
 */
int nl_addr_fill_sockaddr(struct nl_addr *addr, struct sockaddr *sa,
			  socklen_t *salen)
{
	switch (addr->a_family) {
	case AF_INET: {
		struct sockaddr_in *sai = (struct sockaddr_in *) sa;

		if (*salen < sizeof(*sai))
			return -NLE_INVAL;

		sai->sin_family = addr->a_family;
		memcpy(&sai->sin_addr, addr->a_addr, 4);
		*salen = sizeof(*sai);
	}
		break;

	case AF_INET6: {
		struct sockaddr_in6 *sa6 = (struct sockaddr_in6 *) sa;

		if (*salen < sizeof(*sa6))
			return -NLE_INVAL;

		sa6->sin6_family = addr->a_family;
		memcpy(&sa6->sin6_addr, addr->a_addr, 16);
		*salen = sizeof(*sa6);
	}
		break;

	default:
		return -NLE_INVAL;
	}

	return 0;
}


/** @} */

/**
 * @name Getting Information About Addresses
 * @{
 */

/**
 * Call getaddrinfo() for an abstract address object.
 * @arg addr		Abstract address object.
 * @arg result		Pointer to store resulting address list.
 * 
 * Calls getaddrinfo() for the specified abstract address in AI_NUMERICHOST
 * mode.
 *
 * @note The caller is responsible for freeing the linked list using the
 *       interface provided by getaddrinfo(3).
 *
 * @return 0 on success or a negative error code.
 */
int nl_addr_info(struct nl_addr *addr, struct addrinfo **result)
{
	int err;
	char buf[INET6_ADDRSTRLEN+5];
	struct addrinfo hint = {
		.ai_flags = AI_NUMERICHOST,
		.ai_family = addr->a_family,
	};

	nl_addr2str(addr, buf, sizeof(buf));

	err = getaddrinfo(buf, NULL, &hint, result);
	if (err != 0) {
		switch (err) {
		case EAI_ADDRFAMILY: return -NLE_AF_NOSUPPORT;
		case EAI_AGAIN: return -NLE_AGAIN;
		case EAI_BADFLAGS: return -NLE_INVAL;
		case EAI_FAIL: return -NLE_NOADDR;
		case EAI_FAMILY: return -NLE_AF_NOSUPPORT;
		case EAI_MEMORY: return -NLE_NOMEM;
		case EAI_NODATA: return -NLE_NOADDR;
		case EAI_NONAME: return -NLE_OBJ_NOTFOUND;
		case EAI_SERVICE: return -NLE_OPNOTSUPP;
		case EAI_SOCKTYPE: return -NLE_BAD_SOCK;
		default: return -NLE_FAILURE;
		}
	}

	return 0;
}

/**
 * Resolve abstract address object to a name using getnameinfo().
 * @arg addr		Abstract address object.
 * @arg host		Destination buffer for host name.
 * @arg hostlen		Length of destination buffer.
 *
 * Resolves the abstract address to a name and writes the looked up result
 * into the host buffer. getnameinfo() is used to perform the lookup and
 * is put into NI_NAMEREQD mode so the function will fail if the lookup
 * couldn't be performed.
 *
 * @return 0 on success or a negative error code.
 */
int nl_addr_resolve(struct nl_addr *addr, char *host, size_t hostlen)
{
	int err;
	struct sockaddr_in6 buf;
	socklen_t salen = sizeof(buf);

	err = nl_addr_fill_sockaddr(addr, (struct sockaddr *) &buf, &salen);
	if (err < 0)
		return err;

	err = getnameinfo((struct sockaddr *) &buf, salen, host, hostlen,
			  NULL, 0, NI_NAMEREQD);
	if (err < 0)
		return nl_syserr2nlerr(err);

	return 0;
}

/** @} */

/**
 * @name Attributes
 * @{
 */

void nl_addr_set_family(struct nl_addr *addr, int family)
{
	addr->a_family = family;
}

int nl_addr_get_family(struct nl_addr *addr)
{
	return addr->a_family;
}

/**
 * Set binary address of abstract address object.
 * @arg addr		Abstract address object.
 * @arg buf		Buffer containing binary address.
 * @arg len		Length of buffer containing binary address.
 */
int nl_addr_set_binary_addr(struct nl_addr *addr, void *buf, size_t len)
{
	if (len > addr->a_maxsize)
		return -NLE_RANGE;

	addr->a_len = len;
	memcpy(addr->a_addr, buf, len);

	return 0;
}

/**
 * Get binary address of abstract address object.
 * @arg addr		Abstract address object.
 */
void *nl_addr_get_binary_addr(struct nl_addr *addr)
{
	return addr->a_addr;
}

/**
 * Get length of binary address of abstract address object.
 * @arg addr		Abstract address object.
 */
unsigned int nl_addr_get_len(struct nl_addr *addr)
{
	return addr->a_len;
}

void nl_addr_set_prefixlen(struct nl_addr *addr, int prefixlen)
{
	addr->a_prefixlen = prefixlen;
}

/**
 * Get prefix length of abstract address object.
 * @arg addr		Abstract address object.
 */
unsigned int nl_addr_get_prefixlen(struct nl_addr *addr)
{
	return addr->a_prefixlen;
}

/** @} */

/**
 * @name Translations to Strings
 * @{
 */

/**
 * Convert abstract address object to character string.
 * @arg addr		Abstract address object.
 * @arg buf		Destination buffer.
 * @arg size		Size of destination buffer.
 *
 * Converts an abstract address to a character string and stores
 * the result in the specified destination buffer.
 *
 * @return Address represented in ASCII stored in destination buffer.
 */
char *nl_addr2str(struct nl_addr *addr, char *buf, size_t size)
{
	int i;
	char tmp[16];

	if (!addr || !addr->a_len) {
		snprintf(buf, size, "none");
		if (addr)
			goto prefix;
		else
			return buf;
	}

	switch (addr->a_family) {
		case AF_INET:
			inet_ntop(AF_INET, addr->a_addr, buf, size);
			break;

		case AF_INET6:
			inet_ntop(AF_INET6, addr->a_addr, buf, size);
			break;

		case AF_DECnet:
			dnet_ntop(addr->a_addr, addr->a_len, buf, size);
			break;

		case AF_LLC:
		default:
			snprintf(buf, size, "%02x",
				 (unsigned char) addr->a_addr[0]);
			for (i = 1; i < addr->a_len; i++) {
				snprintf(tmp, sizeof(tmp), ":%02x",
					 (unsigned char) addr->a_addr[i]);
				strncat(buf, tmp, size - strlen(buf) - 1);
			}
			break;
	}

prefix:
	if (addr->a_prefixlen != (8 * addr->a_len)) {
		snprintf(tmp, sizeof(tmp), "/%u", addr->a_prefixlen);
		strncat(buf, tmp, size - strlen(buf) - 1);
	}

	return buf;
}

/** @} */

/**
 * @name Address Family Transformations
 * @{
 */

static const struct trans_tbl afs[] = {
	__ADD(AF_UNSPEC,unspec)
	__ADD(AF_UNIX,unix)
	__ADD(AF_LOCAL,local)
	__ADD(AF_INET,inet)
	__ADD(AF_AX25,ax25)
	__ADD(AF_IPX,ipx)
	__ADD(AF_APPLETALK,appletalk)
	__ADD(AF_NETROM,netrom)
	__ADD(AF_BRIDGE,bridge)
	__ADD(AF_ATMPVC,atmpvc)
	__ADD(AF_X25,x25)
	__ADD(AF_INET6,inet6)
	__ADD(AF_ROSE,rose)
	__ADD(AF_DECnet,decnet)
	__ADD(AF_NETBEUI,netbeui)
	__ADD(AF_SECURITY,security)
	__ADD(AF_KEY,key)
	__ADD(AF_NETLINK,netlink)
	__ADD(AF_ROUTE,route)
	__ADD(AF_PACKET,packet)
	__ADD(AF_ASH,ash)
	__ADD(AF_ECONET,econet)
	__ADD(AF_ATMSVC,atmsvc)
	__ADD(AF_SNA,sna)
	__ADD(AF_IRDA,irda)
	__ADD(AF_PPPOX,pppox)
	__ADD(AF_WANPIPE,wanpipe)
	__ADD(AF_LLC,llc)
	__ADD(AF_BLUETOOTH,bluetooth)
};

char *nl_af2str(int family, char *buf, size_t size)
{
	return __type2str(family, buf, size, afs, ARRAY_SIZE(afs));
}

int nl_str2af(const char *name)
{
	int fam = __str2type(name, afs, ARRAY_SIZE(afs));
	return fam >= 0 ? fam : -EINVAL;
}

/** @} */

/** @} */
