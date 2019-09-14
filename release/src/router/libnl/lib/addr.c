/*
 * lib/addr.c		Network Address
 *
 *	This library is free software; you can redistribute it and/or
 *	modify it under the terms of the GNU Lesser General Public
 *	License as published by the Free Software Foundation version 2.1
 *	of the License.
 *
 * Copyright (c) 2003-2013 Thomas Graf <tgraf@suug.ch>
 */

/**
 * @ingroup core_types
 * @defgroup addr Network Address
 *
 * Abstract data type representing any kind of network address
 *
 * Related sections in the development guide:
 * - @core_doc{_abstract_address, Network Addresses}
 *
 * @{
 *
 * Header
 * ------
 * ~~~~{.c}
 * #include <netlink/addr.h>
 * ~~~~
 */

#include <netlink-private/netlink.h>
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
 * @name Creating Abstract Network Addresses
 * @{
 */

/**
 * Allocate empty abstract address
 * @arg maxsize		Upper limit of the binary address to be stored
 *
 * The new address object will be empty with a prefix length of 0 and will
 * be capable of holding binary addresses up to the specified limit.
 *
 * @see nl_addr_build()
 * @see nl_addr_parse()
 * @see nl_addr_put()
 *
 * @return Allocated address object or NULL upon failure.
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
 * Allocate abstract address based on a binary address.
 * @arg family		Address family
 * @arg buf		Binary address
 * @arg size		Length of binary address
 *
 * This function will allocate an abstract address capable of holding the
 * binary address specified. The prefix length will be set to the full
 * length of the binary address provided.
 *
 * @see nl_addr_alloc()
 * @see nl_addr_alloc_attr()
 * @see nl_addr_parse()
 * @see nl_addr_put()
 *
 * @return Allocated address object or NULL upon failure.
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
 * Allocate abstract address based on Netlink attribute.
 * @arg nla		Netlink attribute
 * @arg family		Address family.
 *
 * Allocates an abstract address based on the specified Netlink attribute
 * by interpreting the payload of the Netlink attribute as the binary
 * address.
 *
 * This function is identical to:
 * @code
 * nl_addr_build(family, nla_data(nla), nla_len(nla));
 * @endcode
 *
 * @see nl_addr_alloc()
 * @see nl_addr_build()
 * @see nl_addr_parse()
 * @see nl_addr_put()
 *
 * @return Allocated address object or NULL upon failure.
 */
struct nl_addr *nl_addr_alloc_attr(struct nlattr *nla, int family)
{
	return nl_addr_build(family, nla_data(nla), nla_len(nla));
}

/**
 * Allocate abstract address based on character string
 * @arg addrstr		Address represented as character string.
 * @arg hint		Address family hint or AF_UNSPEC.
 * @arg result		Pointer to store resulting address.
 *
 * Regognizes the following address formats:
 * @code
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
 * @see nl_addr_alloc()
 * @see nl_addr_build()
 * @see nl_addr_put()
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

		len = 0;

		switch (hint) {
			case AF_INET:
			case AF_UNSPEC:
				/* Kind of a hack, we assume that if there is
				 * no hint given the user wants to have a IPv4
				 * address given back. */
				family = AF_INET;
				goto prefix;

			case AF_INET6:
				family = AF_INET6;
				goto prefix;

			case AF_LLC:
				family = AF_LLC;
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
		size_t i = 0;
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
 * Clone existing abstract address object
 * @arg addr		Abstract address object
 *
 * Allocates new abstract address representing an identical clone of an
 * existing address.
 *
 * @see nl_addr_alloc()
 * @see nl_addr_put()
 *
 * @return Allocated abstract address or NULL upon failure.
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

/**
 * Increase the reference counter of an abstract address
 * @arg addr		Abstract address
 *
 * Increases the reference counter of the address and thus prevents the
 * release of the memory resources until the reference is given back
 * using the function nl_addr_put().
 *
 * @see nl_addr_put()
 *
 * @return Pointer to the existing abstract address
 */
struct nl_addr *nl_addr_get(struct nl_addr *addr)
{
	addr->a_refcnt++;

	return addr;
}

/**
 * Decrease the reference counter of an abstract address
 * @arg addr		Abstract addr
 *
 * @note The resources of the abstract address will be freed after the
 *       last reference to the address has been returned.
 *
 * @see nl_addr_get()
 */
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
 * Check whether an abstract address is shared.
 * @arg addr		Abstract address object.
 *
 * @return Non-zero if the abstract address is shared, otherwise 0.
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
 * Compare abstract addresses
 * @arg a		An abstract address
 * @arg b		Another abstract address
 *
 * Verifies whether the address family, address length, prefix length, and
 * binary addresses of two abstract addresses matches.
 *
 * @note This function will *not* respect the prefix length in the sense
 *       that only the actual prefix will be compared. Please refer to the
 *       nl_addr_cmp_prefix() function if you require this functionality.
 *
 * @see nl_addr_cmp_prefix()
 *
 * @return Integer less than, equal to or greather than zero if the two
 *         addresses match.
 */
int nl_addr_cmp(struct nl_addr *a, struct nl_addr *b)
{
	int d = a->a_family - b->a_family;

	if (d == 0) {
		d = a->a_len - b->a_len;

		if (a->a_len && d == 0) {
			d = memcmp(a->a_addr, b->a_addr, a->a_len);

			if (d == 0)
				return (a->a_prefixlen - b->a_prefixlen);
		}
	}

	return d;
}

/**
 * Compare the prefix of two abstract addresses
 * @arg a		An abstract address
 * @arg b		Another abstract address
 *
 * Verifies whether the address family and the binary address covered by
 * the smaller prefix length of the two abstract addresses matches.
 *
 * @see nl_addr_cmp()
 *
 * @return Integer less than, equal to or greather than zero if the two
 *         addresses match.
 */
int nl_addr_cmp_prefix(struct nl_addr *a, struct nl_addr *b)
{
	int d = a->a_family - b->a_family;

	if (d == 0) {
		int len = min(a->a_prefixlen, b->a_prefixlen);
		int bytes = len / 8;

		d = memcmp(a->a_addr, b->a_addr, bytes);
		if (d == 0 && (len % 8) != 0) {
			int mask = (0xFF00 >> (len % 8)) & 0xFF;

			d = (a->a_addr[bytes] & mask) -
			    (b->a_addr[bytes] & mask);
		}
	}

	return d;
}

/**
 * Returns true if the address consists of all zeros
 * @arg addr		Abstract address
 *
 * @return 1 if the binary address consists of all zeros, 0 otherwise.
 */
int nl_addr_iszero(struct nl_addr *addr)
{
	unsigned int i;

	for (i = 0; i < addr->a_len; i++)
		if (addr->a_addr[i])
			return 0;

	return 1;
}

/**
 * Check if address string is parseable for a specific address family
 * @arg addr		Address represented as character string.
 * @arg family		Desired address family.
 *
 * @return 1 if the address is parseable assuming the specified address family,
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
 * Guess address family of abstract address based on address size
 * @arg addr		Abstract address object.
 *
 * @return Numeric address family or AF_UNSPEC
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

/**
 * Set address family
 * @arg addr		Abstract address object
 * @arg family		Address family
 *
 * @see nl_addr_get_family()
 */
void nl_addr_set_family(struct nl_addr *addr, int family)
{
	addr->a_family = family;
}

/**
 * Return address family
 * @arg addr		Abstract address object
 *
 * @see nl_addr_set_family()
 *
 * @return The numeric address family or `AF_UNSPEC`
 */
int nl_addr_get_family(struct nl_addr *addr)
{
	return addr->a_family;
}

/**
 * Set binary address of abstract address object.
 * @arg addr		Abstract address object.
 * @arg buf		Buffer containing binary address.
 * @arg len		Length of buffer containing binary address.
 *
 * Modifies the binary address portion of the abstract address. The
 * abstract address must be capable of holding the required amount
 * or this function will fail.
 *
 * @note This function will *not* modify the prefix length. It is within
 *       the responsibility of the caller to set the prefix length to the
 *       desirable length.
 *
 * @see nl_addr_alloc()
 * @see nl_addr_get_binary_addr()
 * @see nl_addr_get_len()
 *
 * @return 0 on success or a negative error code.
 */
int nl_addr_set_binary_addr(struct nl_addr *addr, void *buf, size_t len)
{
	if (len > addr->a_maxsize)
		return -NLE_RANGE;

	addr->a_len = len;
	memset(addr->a_addr, 0, addr->a_maxsize);

	if (len)
		memcpy(addr->a_addr, buf, len);

	return 0;
}

/**
 * Get binary address of abstract address object.
 * @arg addr		Abstract address object.
 *
 * @see nl_addr_set_binary_addr()
 * @see nl_addr_get_len()
 *
 * @return Pointer to binary address of length nl_addr_get_len()
 */
void *nl_addr_get_binary_addr(struct nl_addr *addr)
{
	return addr->a_addr;
}

/**
 * Get length of binary address of abstract address object.
 * @arg addr		Abstract address object.
 *
 * @see nl_addr_get_binary_addr()
 * @see nl_addr_set_binary_addr()
 */
unsigned int nl_addr_get_len(struct nl_addr *addr)
{
	return addr->a_len;
}

/**
 * Set the prefix length of an abstract address
 * @arg addr		Abstract address object
 * @arg prefixlen	New prefix length
 *
 * @see nl_addr_get_prefixlen()
 */
void nl_addr_set_prefixlen(struct nl_addr *addr, int prefixlen)
{
	addr->a_prefixlen = prefixlen;
}

/**
 * Return prefix length of abstract address object.
 * @arg addr		Abstract address object
 *
 * @see nl_addr_set_prefixlen()
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
	unsigned int i;
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
	__ADD(AF_PACKET,packet)
	__ADD(AF_ASH,ash)
	__ADD(AF_ECONET,econet)
	__ADD(AF_ATMSVC,atmsvc)
#ifdef AF_RDS
	__ADD(AF_RDS,rds)
#endif
	__ADD(AF_SNA,sna)
	__ADD(AF_IRDA,irda)
	__ADD(AF_PPPOX,pppox)
	__ADD(AF_WANPIPE,wanpipe)
	__ADD(AF_LLC,llc)
#ifdef AF_CAN
	__ADD(AF_CAN,can)
#endif
#ifdef AF_TIPC
	__ADD(AF_TIPC,tipc)
#endif
	__ADD(AF_BLUETOOTH,bluetooth)
#ifdef AF_IUCV
	__ADD(AF_IUCV,iucv)
#endif
#ifdef AF_RXRPC
	__ADD(AF_RXRPC,rxrpc)
#endif
#ifdef AF_ISDN
	__ADD(AF_ISDN,isdn)
#endif
#ifdef AF_PHONET
	__ADD(AF_PHONET,phonet)
#endif
#ifdef AF_IEEE802154
	__ADD(AF_IEEE802154,ieee802154)
#endif
#ifdef AF_CAIF
	__ADD(AF_CAIF,caif)
#endif
#ifdef AF_ALG
	__ADD(AF_ALG,alg)
#endif
#ifdef AF_NFC
	__ADD(AF_NFC,nfc)
#endif
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
