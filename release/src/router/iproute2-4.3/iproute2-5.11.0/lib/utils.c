/*
 * utils.c
 *
 *		This program is free software; you can redistribute it and/or
 *		modify it under the terms of the GNU General Public License
 *		as published by the Free Software Foundation; either version
 *		2 of the License, or (at your option) any later version.
 *
 * Authors:	Alexey Kuznetsov, <kuznet@ms2.inr.ac.ru>
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <unistd.h>
#include <fcntl.h>
#include <limits.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <ctype.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <asm/types.h>
#include <linux/pkt_sched.h>
#include <linux/param.h>
#include <linux/if_arp.h>
#include <linux/mpls.h>
#include <linux/snmp.h>
#include <time.h>
#include <sys/time.h>
#include <errno.h>
#ifdef HAVE_LIBCAP
#include <sys/capability.h>
#endif

#include "rt_names.h"
#include "utils.h"
#include "ll_map.h"
#include "namespace.h"

int resolve_hosts;
int timestamp_short;
int pretty;
const char *_SL_ = "\n";

static int af_byte_len(int af);
static void print_time(char *buf, int len, __u32 time);
static void print_time64(char *buf, int len, __s64 time);

int read_prop(const char *dev, char *prop, long *value)
{
	char fname[128], buf[80], *endp, *nl;
	FILE *fp;
	long result;
	int ret;

	ret = snprintf(fname, sizeof(fname), "/sys/class/net/%s/%s",
			dev, prop);

	if (ret <= 0 || ret >= sizeof(fname)) {
		fprintf(stderr, "could not build pathname for property\n");
		return -1;
	}

	fp = fopen(fname, "r");
	if (fp == NULL) {
		fprintf(stderr, "fopen %s: %s\n", fname, strerror(errno));
		return -1;
	}

	if (!fgets(buf, sizeof(buf), fp)) {
		fprintf(stderr, "property \"%s\" in file %s is currently unknown\n", prop, fname);
		fclose(fp);
		goto out;
	}

	nl = strchr(buf, '\n');
	if (nl)
		*nl = '\0';

	fclose(fp);
	result = strtol(buf, &endp, 0);

	if (*endp || buf == endp) {
		fprintf(stderr, "value \"%s\" in file %s is not a number\n",
			buf, fname);
		goto out;
	}

	if ((result == LONG_MAX || result == LONG_MIN) && errno == ERANGE) {
		fprintf(stderr, "strtol %s: %s", fname, strerror(errno));
		goto out;
	}

	*value = result;
	return 0;
out:
	fprintf(stderr, "Failed to parse %s\n", fname);
	return -1;
}

int get_hex(char c)
{
	if (c >= 'A' && c <= 'F')
		return c - 'A' + 10;
	if (c >= 'a' && c <= 'f')
		return c - 'a' + 10;
	if (c >= '0' && c <= '9')
		return c - '0';

	return -1;
}

int get_integer(int *val, const char *arg, int base)
{
	long res;
	char *ptr;

	if (!arg || !*arg)
		return -1;

	res = strtol(arg, &ptr, base);

	/* If there were no digits at all, strtol()  stores
	 * the original value of nptr in *endptr (and returns 0).
	 * In particular, if *nptr is not '\0' but **endptr is '\0' on return,
	 * the entire string is valid.
	 */
	if (!ptr || ptr == arg || *ptr)
		return -1;

	/* If an underflow occurs, strtol() returns LONG_MIN.
	 * If an overflow occurs,  strtol() returns LONG_MAX.
	 * In both cases, errno is set to ERANGE.
	 */
	if ((res == LONG_MAX || res == LONG_MIN) && errno == ERANGE)
		return -1;

	/* Outside range of int */
	if (res < INT_MIN || res > INT_MAX)
		return -1;

	*val = res;
	return 0;
}

int mask2bits(__u32 netmask)
{
	unsigned int bits = 0;
	__u32 mask = ntohl(netmask);
	__u32 host = ~mask;

	/* a valid netmask must be 2^n - 1 */
	if ((host & (host + 1)) != 0)
		return -1;

	for (; mask; mask <<= 1)
		++bits;
	return bits;
}

static int get_netmask(unsigned int *val, const char *arg, int base)
{
	inet_prefix addr;

	if (!get_unsigned(val, arg, base))
		return 0;

	/* try converting dotted quad to CIDR */
	if (!get_addr_1(&addr, arg, AF_INET) && addr.family == AF_INET) {
		int b = mask2bits(addr.data[0]);

		if (b >= 0) {
			*val = b;
			return 0;
		}
	}

	return -1;
}

int get_unsigned(unsigned int *val, const char *arg, int base)
{
	unsigned long res;
	char *ptr;

	if (!arg || !*arg)
		return -1;

	res = strtoul(arg, &ptr, base);

	/* empty string or trailing non-digits */
	if (!ptr || ptr == arg || *ptr)
		return -1;

	/* overflow */
	if (res == ULONG_MAX && errno == ERANGE)
		return -1;

	/* out side range of unsigned */
	if (res > UINT_MAX)
		return -1;

	*val = res;
	return 0;
}

/*
 * get_time_rtt is "translated" from a similar routine "get_time" in
 * tc_util.c.  We don't use the exact same routine because tc passes
 * microseconds to the kernel and the callers of get_time_rtt want to
 * pass milliseconds (standard unit for rtt values since 2.6.27), and
 * have a different assumption for the units of a "raw" number.
 */
int get_time_rtt(unsigned int *val, const char *arg, int *raw)
{
	double t;
	unsigned long res;
	char *p;

	if (strchr(arg, '.') != NULL) {
		t = strtod(arg, &p);
		if (t < 0.0)
			return -1;

		/* no digits? */
		if (!p || p == arg)
			return -1;

		/* over/underflow */
		if ((t == HUGE_VALF || t == HUGE_VALL) && errno == ERANGE)
			return -1;
	} else {
		res = strtoul(arg, &p, 0);

		/* empty string? */
		if (!p || p == arg)
			return -1;

		/* overflow */
		if (res == ULONG_MAX && errno == ERANGE)
			return -1;

		t = (double)res;
	}

	if (p == arg)
		return -1;
	*raw = 1;

	if (*p) {
		*raw = 0;
		if (strcasecmp(p, "s") == 0 ||
		    strcasecmp(p, "sec") == 0 ||
		    strcasecmp(p, "secs") == 0)
			t *= 1000;
		else if (strcasecmp(p, "ms") == 0 ||
			 strcasecmp(p, "msec") == 0 ||
			 strcasecmp(p, "msecs") == 0)
			t *= 1.0; /* allow suffix, do nothing */
		else
			return -1;
	}

	/* emulate ceil() without having to bring-in -lm and always be >= 1 */
	*val = t;
	if (*val < t)
		*val += 1;

	return 0;

}

int get_u64(__u64 *val, const char *arg, int base)
{
	unsigned long long res;
	char *ptr;

	if (!arg || !*arg)
		return -1;

	res = strtoull(arg, &ptr, base);

	/* empty string or trailing non-digits */
	if (!ptr || ptr == arg || *ptr)
		return -1;

	/* overflow */
	if (res == ULLONG_MAX && errno == ERANGE)
		return -1;

	/* in case ULL is 128 bits */
	if (res > 0xFFFFFFFFFFFFFFFFULL)
		return -1;

	*val = res;
	return 0;
}

int get_u32(__u32 *val, const char *arg, int base)
{
	unsigned long res;
	char *ptr;

	if (!arg || !*arg)
		return -1;
	res = strtoul(arg, &ptr, base);

	/* empty string or trailing non-digits */
	if (!ptr || ptr == arg || *ptr)
		return -1;

	/* overflow */
	if (res == ULONG_MAX && errno == ERANGE)
		return -1;

	/* in case UL > 32 bits */
	if (res > 0xFFFFFFFFUL)
		return -1;

	*val = res;
	return 0;
}

int get_u16(__u16 *val, const char *arg, int base)
{
	unsigned long res;
	char *ptr;

	if (!arg || !*arg)
		return -1;
	res = strtoul(arg, &ptr, base);

	/* empty string or trailing non-digits */
	if (!ptr || ptr == arg || *ptr)
		return -1;

	/* overflow */
	if (res == ULONG_MAX && errno == ERANGE)
		return -1;

	if (res > 0xFFFFUL)
		return -1;

	*val = res;
	return 0;
}

int get_u8(__u8 *val, const char *arg, int base)
{
	unsigned long res;
	char *ptr;

	if (!arg || !*arg)
		return -1;

	res = strtoul(arg, &ptr, base);
	/* empty string or trailing non-digits */
	if (!ptr || ptr == arg || *ptr)
		return -1;

	/* overflow */
	if (res == ULONG_MAX && errno == ERANGE)
		return -1;

	if (res > 0xFFUL)
		return -1;

	*val = res;
	return 0;
}

int get_s64(__s64 *val, const char *arg, int base)
{
	long long res;
	char *ptr;

	errno = 0;

	if (!arg || !*arg)
		return -1;
	res = strtoll(arg, &ptr, base);
	if (!ptr || ptr == arg || *ptr)
		return -1;
	if ((res == LLONG_MIN || res == LLONG_MAX) && errno == ERANGE)
		return -1;
	if (res > INT64_MAX || res < INT64_MIN)
		return -1;

	*val = res;
	return 0;
}

int get_s32(__s32 *val, const char *arg, int base)
{
	long res;
	char *ptr;

	errno = 0;

	if (!arg || !*arg)
		return -1;
	res = strtol(arg, &ptr, base);
	if (!ptr || ptr == arg || *ptr)
		return -1;
	if ((res == LONG_MIN || res == LONG_MAX) && errno == ERANGE)
		return -1;
	if (res > INT32_MAX || res < INT32_MIN)
		return -1;

	*val = res;
	return 0;
}

int get_be64(__be64 *val, const char *arg, int base)
{
	__u64 v;
	int ret = get_u64(&v, arg, base);

	if (!ret)
		*val = htonll(v);

	return ret;
}

int get_be32(__be32 *val, const char *arg, int base)
{
	__u32 v;
	int ret = get_u32(&v, arg, base);

	if (!ret)
		*val = htonl(v);

	return ret;
}

int get_be16(__be16 *val, const char *arg, int base)
{
	__u16 v;
	int ret = get_u16(&v, arg, base);

	if (!ret)
		*val = htons(v);

	return ret;
}

/* This uses a non-standard parsing (ie not inet_aton, or inet_pton)
 * because of legacy choice to parse 10.8 as 10.8.0.0 not 10.0.0.8
 */
static int get_addr_ipv4(__u8 *ap, const char *cp)
{
	int i;

	for (i = 0; i < 4; i++) {
		unsigned long n;
		char *endp;

		n = strtoul(cp, &endp, 0);
		if (n > 255)
			return -1;	/* bogus network value */

		if (endp == cp) /* no digits */
			return -1;

		ap[i] = n;

		if (*endp == '\0')
			break;

		if (i == 3 || *endp != '.')
			return -1;	/* extra characters */
		cp = endp + 1;
	}

	return 1;
}

int get_addr64(__u64 *ap, const char *cp)
{
	int i;

	union {
		__u16 v16[4];
		__u64 v64;
	} val;

	for (i = 0; i < 4; i++) {
		unsigned long n;
		char *endp;

		n = strtoul(cp, &endp, 16);
		if (n > 0xffff)
			return -1;	/* bogus network value */

		if (endp == cp) /* no digits */
			return -1;

		val.v16[i] = htons(n);

		if (*endp == '\0')
			break;

		if (i == 3 || *endp != ':')
			return -1;	/* extra characters */
		cp = endp + 1;
	}

	*ap = val.v64;

	return 1;
}

static void set_address_type(inet_prefix *addr)
{
	switch (addr->family) {
	case AF_INET:
		if (!addr->data[0])
			addr->flags |= ADDRTYPE_INET_UNSPEC;
		else if (IN_MULTICAST(ntohl(addr->data[0])))
			addr->flags |= ADDRTYPE_INET_MULTI;
		else
			addr->flags |= ADDRTYPE_INET;
		break;
	case AF_INET6:
		if (IN6_IS_ADDR_UNSPECIFIED(addr->data))
			addr->flags |= ADDRTYPE_INET_UNSPEC;
		else if (IN6_IS_ADDR_MULTICAST(addr->data))
			addr->flags |= ADDRTYPE_INET_MULTI;
		else
			addr->flags |= ADDRTYPE_INET;
		break;
	}
}

static int __get_addr_1(inet_prefix *addr, const char *name, int family)
{
	memset(addr, 0, sizeof(*addr));

	if (strcmp(name, "default") == 0) {
		if ((family == AF_DECnet) || (family == AF_MPLS))
			return -1;
		addr->family = family;
		addr->bytelen = af_byte_len(addr->family);
		addr->bitlen = -2;
		addr->flags |= PREFIXLEN_SPECIFIED;
		return 0;
	}

	if (strcmp(name, "all") == 0 ||
	    strcmp(name, "any") == 0) {
		if ((family == AF_DECnet) || (family == AF_MPLS))
			return -1;
		addr->family = family;
		addr->bytelen = 0;
		addr->bitlen = -2;
		return 0;
	}

	if (family == AF_PACKET) {
		int len;

		len = ll_addr_a2n((char *) &addr->data, sizeof(addr->data),
				  name);
		if (len < 0)
			return -1;

		addr->family = AF_PACKET;
		addr->bytelen = len;
		addr->bitlen = len * 8;
		return 0;
	}

	if (strchr(name, ':')) {
		addr->family = AF_INET6;
		if (family != AF_UNSPEC && family != AF_INET6)
			return -1;
		if (inet_pton(AF_INET6, name, addr->data) <= 0)
			return -1;
		addr->bytelen = 16;
		addr->bitlen = -1;
		return 0;
	}

	if (family == AF_MPLS) {
		unsigned int maxlabels;
		int i;

		addr->family = AF_MPLS;
		if (mpls_pton(AF_MPLS, name, addr->data,
			      sizeof(addr->data)) <= 0)
			return -1;
		addr->bytelen = 4;
		addr->bitlen = 20;
		/* How many bytes do I need? */
		maxlabels = sizeof(addr->data) / sizeof(struct mpls_label);
		for (i = 0; i < maxlabels; i++) {
			if (ntohl(addr->data[i]) & MPLS_LS_S_MASK) {
				addr->bytelen = (i + 1)*4;
				break;
			}
		}
		return 0;
	}

	addr->family = AF_INET;
	if (family != AF_UNSPEC && family != AF_INET)
		return -1;

	if (get_addr_ipv4((__u8 *)addr->data, name) <= 0)
		return -1;

	addr->bytelen = 4;
	addr->bitlen = -1;
	return 0;
}

int get_addr_1(inet_prefix *addr, const char *name, int family)
{
	int ret;

	ret = __get_addr_1(addr, name, family);
	if (ret)
		return ret;

	set_address_type(addr);
	return 0;
}

int af_bit_len(int af)
{
	switch (af) {
	case AF_INET6:
		return 128;
	case AF_INET:
		return 32;
	case AF_DECnet:
		return 16;
	case AF_IPX:
		return 80;
	case AF_MPLS:
		return 20;
	}

	return 0;
}

static int af_byte_len(int af)
{
	return af_bit_len(af) / 8;
}

int get_prefix_1(inet_prefix *dst, char *arg, int family)
{
	char *slash;
	int err, bitlen, flags;

	slash = strchr(arg, '/');
	if (slash)
		*slash = 0;

	err = get_addr_1(dst, arg, family);

	if (slash)
		*slash = '/';

	if (err)
		return err;

	bitlen = af_bit_len(dst->family);

	flags = 0;
	if (slash) {
		unsigned int plen;

		if (dst->bitlen == -2)
			return -1;
		if (get_netmask(&plen, slash + 1, 0))
			return -1;
		if (plen > bitlen)
			return -1;

		flags |= PREFIXLEN_SPECIFIED;
		bitlen = plen;
	} else {
		if (dst->bitlen == -2)
			bitlen = 0;
	}

	dst->flags |= flags;
	dst->bitlen = bitlen;

	return 0;
}

static const char *family_name_verbose(int family)
{
	if (family == AF_UNSPEC)
		return "any valid";
	return family_name(family);
}

int get_addr(inet_prefix *dst, const char *arg, int family)
{
	if (get_addr_1(dst, arg, family)) {
		fprintf(stderr,
			"Error: %s address is expected rather than \"%s\".\n",
			family_name_verbose(family), arg);
		exit(1);
	}
	return 0;
}

int get_addr_rta(inet_prefix *dst, const struct rtattr *rta, int family)
{
	const int len = RTA_PAYLOAD(rta);
	const void *data = RTA_DATA(rta);

	switch (len) {
	case 4:
		dst->family = AF_INET;
		dst->bytelen = 4;
		memcpy(dst->data, data, 4);
		break;
	case 16:
		dst->family = AF_INET6;
		dst->bytelen = 16;
		memcpy(dst->data, data, 16);
		break;
	case 2:
		dst->family = AF_DECnet;
		dst->bytelen = 2;
		memcpy(dst->data, data, 2);
		break;
	case 10:
		dst->family = AF_IPX;
		dst->bytelen = 10;
		memcpy(dst->data, data, 10);
		break;
	default:
		return -1;
	}

	if (family != AF_UNSPEC && family != dst->family)
		return -2;

	dst->bitlen = -1;
	dst->flags = 0;

	set_address_type(dst);
	return 0;
}

int get_prefix(inet_prefix *dst, char *arg, int family)
{
	if (family == AF_PACKET) {
		fprintf(stderr,
			"Error: \"%s\" may be inet prefix, but it is not allowed in this context.\n",
			arg);
		exit(1);
	}

	if (get_prefix_1(dst, arg, family)) {
		fprintf(stderr,
			"Error: %s prefix is expected rather than \"%s\".\n",
			family_name_verbose(family), arg);
		exit(1);
	}
	return 0;
}

__u32 get_addr32(const char *name)
{
	inet_prefix addr;

	if (get_addr_1(&addr, name, AF_INET)) {
		fprintf(stderr,
			"Error: an IP address is expected rather than \"%s\"\n",
			name);
		exit(1);
	}
	return addr.data[0];
}

void incomplete_command(void)
{
	fprintf(stderr, "Command line is not complete. Try option \"help\"\n");
	exit(-1);
}

void missarg(const char *key)
{
	fprintf(stderr, "Error: argument \"%s\" is required\n", key);
	exit(-1);
}

void invarg(const char *msg, const char *arg)
{
	fprintf(stderr, "Error: argument \"%s\" is wrong: %s\n", arg, msg);
	exit(-1);
}

void duparg(const char *key, const char *arg)
{
	fprintf(stderr,
		"Error: duplicate \"%s\": \"%s\" is the second value.\n",
		key, arg);
	exit(-1);
}

void duparg2(const char *key, const char *arg)
{
	fprintf(stderr,
		"Error: either \"%s\" is duplicate, or \"%s\" is a garbage.\n",
		key, arg);
	exit(-1);
}

int nodev(const char *dev)
{
	fprintf(stderr, "Cannot find device \"%s\"\n", dev);
	return -1;
}

static int __check_ifname(const char *name)
{
	if (*name == '\0')
		return -1;
	while (*name) {
		if (*name == '/' || isspace(*name))
			return -1;
		++name;
	}
	return 0;
}

int check_ifname(const char *name)
{
	/* These checks mimic kernel checks in dev_valid_name */
	if (strlen(name) >= IFNAMSIZ)
		return -1;
	return __check_ifname(name);
}

int check_altifname(const char *name)
{
	return __check_ifname(name);
}

/* buf is assumed to be IFNAMSIZ */
int get_ifname(char *buf, const char *name)
{
	int ret;

	ret = check_ifname(name);
	if (ret == 0)
		strncpy(buf, name, IFNAMSIZ);

	return ret;
}

const char *get_ifname_rta(int ifindex, const struct rtattr *rta)
{
	const char *name;

	if (rta) {
		name = rta_getattr_str(rta);
	} else {
		fprintf(stderr,
			"BUG: device with ifindex %d has nil ifname\n",
			ifindex);
		name = ll_idx_n2a(ifindex);
	}

	if (check_ifname(name))
		return NULL;

	return name;
}

/* Returns false if 'prefix' is a not empty prefix of 'string'.
 */
bool matches(const char *prefix, const char *string)
{
	if (!*prefix)
		return true;
	while (*string && *prefix == *string) {
		prefix++;
		string++;
	}

	return !!*prefix;
}

int inet_addr_match(const inet_prefix *a, const inet_prefix *b, int bits)
{
	const __u32 *a1 = a->data;
	const __u32 *a2 = b->data;
	int words = bits >> 0x05;

	bits &= 0x1f;

	if (words)
		if (memcmp(a1, a2, words << 2))
			return -1;

	if (bits) {
		__u32 w1, w2;
		__u32 mask;

		w1 = a1[words];
		w2 = a2[words];

		mask = htonl((0xffffffff) << (0x20 - bits));

		if ((w1 ^ w2) & mask)
			return 1;
	}

	return 0;
}

int inet_addr_match_rta(const inet_prefix *m, const struct rtattr *rta)
{
	inet_prefix dst;

	if (!rta || m->family == AF_UNSPEC || m->bitlen <= 0)
		return 0;

	if (get_addr_rta(&dst, rta, m->family))
		return -1;

	return inet_addr_match(&dst, m, m->bitlen);
}

int __iproute2_hz_internal;

int __get_hz(void)
{
	char name[1024];
	int hz = 0;
	FILE *fp;

	if (getenv("HZ"))
		return atoi(getenv("HZ")) ? : HZ;

	if (getenv("PROC_NET_PSCHED"))
		snprintf(name, sizeof(name)-1,
			 "%s", getenv("PROC_NET_PSCHED"));
	else if (getenv("PROC_ROOT"))
		snprintf(name, sizeof(name)-1,
			 "%s/net/psched", getenv("PROC_ROOT"));
	else
		strcpy(name, "/proc/net/psched");

	fp = fopen(name, "r");

	if (fp) {
		unsigned int nom, denom;

		if (fscanf(fp, "%*08x%*08x%08x%08x", &nom, &denom) == 2)
			if (nom == 1000000)
				hz = denom;
		fclose(fp);
	}
	if (hz)
		return hz;
	return HZ;
}

int __iproute2_user_hz_internal;

int __get_user_hz(void)
{
	return sysconf(_SC_CLK_TCK);
}

const char *rt_addr_n2a_r(int af, int len,
			  const void *addr, char *buf, int buflen)
{
	switch (af) {
	case AF_INET:
	case AF_INET6:
		return inet_ntop(af, addr, buf, buflen);
	case AF_MPLS:
		return mpls_ntop(af, addr, buf, buflen);
	case AF_PACKET:
		return ll_addr_n2a(addr, len, ARPHRD_VOID, buf, buflen);
	case AF_BRIDGE:
	{
		const union {
			struct sockaddr sa;
			struct sockaddr_in sin;
			struct sockaddr_in6 sin6;
		} *sa = addr;

		switch (sa->sa.sa_family) {
		case AF_INET:
			return inet_ntop(AF_INET, &sa->sin.sin_addr,
					 buf, buflen);
		case AF_INET6:
			return inet_ntop(AF_INET6, &sa->sin6.sin6_addr,
					 buf, buflen);
		}

		/* fallthrough */
	}
	default:
		return "???";
	}
}

const char *rt_addr_n2a(int af, int len, const void *addr)
{
	static char buf[256];

	return rt_addr_n2a_r(af, len, addr, buf, 256);
}

int read_family(const char *name)
{
	int family = AF_UNSPEC;

	if (strcmp(name, "inet") == 0)
		family = AF_INET;
	else if (strcmp(name, "inet6") == 0)
		family = AF_INET6;
	else if (strcmp(name, "link") == 0)
		family = AF_PACKET;
	else if (strcmp(name, "ipx") == 0)
		family = AF_IPX;
	else if (strcmp(name, "mpls") == 0)
		family = AF_MPLS;
	else if (strcmp(name, "bridge") == 0)
		family = AF_BRIDGE;
	return family;
}

const char *family_name(int family)
{
	if (family == AF_INET)
		return "inet";
	if (family == AF_INET6)
		return "inet6";
	if (family == AF_PACKET)
		return "link";
	if (family == AF_IPX)
		return "ipx";
	if (family == AF_MPLS)
		return "mpls";
	if (family == AF_BRIDGE)
		return "bridge";
	return "???";
}

#ifdef RESOLVE_HOSTNAMES
struct namerec {
	struct namerec *next;
	const char *name;
	inet_prefix addr;
};

#define NHASH 257
static struct namerec *nht[NHASH];

static const char *resolve_address(const void *addr, int len, int af)
{
	struct namerec *n;
	struct hostent *h_ent;
	unsigned int hash;
	static int notfirst;


	if (af == AF_INET6 && ((__u32 *)addr)[0] == 0 &&
	    ((__u32 *)addr)[1] == 0 && ((__u32 *)addr)[2] == htonl(0xffff)) {
		af = AF_INET;
		addr += 12;
		len = 4;
	}

	hash = *(__u32 *)(addr + len - 4) % NHASH;

	for (n = nht[hash]; n; n = n->next) {
		if (n->addr.family == af &&
		    n->addr.bytelen == len &&
		    memcmp(n->addr.data, addr, len) == 0)
			return n->name;
	}
	n = malloc(sizeof(*n));
	if (n == NULL)
		return NULL;
	n->addr.family = af;
	n->addr.bytelen = len;
	n->name = NULL;
	memcpy(n->addr.data, addr, len);
	n->next = nht[hash];
	nht[hash] = n;
	if (++notfirst == 1)
		sethostent(1);
	fflush(stdout);

	h_ent = gethostbyaddr(addr, len, af);
	if (h_ent != NULL)
		n->name = strdup(h_ent->h_name);

	/* Even if we fail, "negative" entry is remembered. */
	return n->name;
}
#endif

const char *format_host_r(int af, int len, const void *addr,
			char *buf, int buflen)
{
#ifdef RESOLVE_HOSTNAMES
	if (resolve_hosts) {
		const char *n;

		len = len <= 0 ? af_byte_len(af) : len;

		if (len > 0 &&
		    (n = resolve_address(addr, len, af)) != NULL)
			return n;
	}
#endif
	return rt_addr_n2a_r(af, len, addr, buf, buflen);
}

const char *format_host(int af, int len, const void *addr)
{
	static char buf[256];

	return format_host_r(af, len, addr, buf, 256);
}


char *hexstring_n2a(const __u8 *str, int len, char *buf, int blen)
{
	char *ptr = buf;
	int i;

	for (i = 0; i < len; i++) {
		if (blen < 3)
			break;
		sprintf(ptr, "%02x", str[i]);
		ptr += 2;
		blen -= 2;
	}
	return buf;
}

__u8 *hexstring_a2n(const char *str, __u8 *buf, int blen, unsigned int *len)
{
	unsigned int cnt = 0;
	char *endptr;

	if (strlen(str) % 2)
		return NULL;
	while (cnt < blen && strlen(str) > 1) {
		unsigned int tmp;
		char tmpstr[3];

		strncpy(tmpstr, str, 2);
		tmpstr[2] = '\0';
		errno = 0;
		tmp = strtoul(tmpstr, &endptr, 16);
		if (errno != 0 || tmp > 0xFF || *endptr != '\0')
			return NULL;
		buf[cnt++] = tmp;
		str += 2;
	}

	if (len)
		*len = cnt;

	return buf;
}

int hex2mem(const char *buf, uint8_t *mem, int count)
{
	int i, j;
	int c;

	for (i = 0, j = 0; i < count; i++, j += 2) {
		c = get_hex(buf[j]);
		if (c < 0)
			return -1;

		mem[i] = c << 4;

		c = get_hex(buf[j + 1]);
		if (c < 0)
			return -1;

		mem[i] |= c;
	}

	return 0;
}

int addr64_n2a(__u64 addr, char *buff, size_t len)
{
	__u16 *words = (__u16 *)&addr;
	__u16 v;
	int i, ret;
	size_t written = 0;
	char *sep = ":";

	for (i = 0; i < 4; i++) {
		v = ntohs(words[i]);

		if (i == 3)
			sep = "";

		ret = snprintf(&buff[written], len - written, "%x%s", v, sep);
		if (ret < 0)
			return ret;

		written += ret;
	}

	return written;
}

/* Print buffer and escape bytes that are !isprint or among 'escape' */
void print_escape_buf(const __u8 *buf, size_t len, const char *escape)
{
	size_t i;

	for (i = 0; i < len; ++i) {
		if (isprint(buf[i]) && buf[i] != '\\' &&
		    !strchr(escape, buf[i]))
			printf("%c", buf[i]);
		else
			printf("\\%03o", buf[i]);
	}
}

int print_timestamp(FILE *fp)
{
	struct timeval tv;
	struct tm *tm;

	gettimeofday(&tv, NULL);
	tm = localtime(&tv.tv_sec);

	if (timestamp_short) {
		char tshort[40];

		strftime(tshort, sizeof(tshort), "%Y-%m-%dT%H:%M:%S", tm);
		fprintf(fp, "[%s.%06ld] ", tshort, tv.tv_usec);
	} else {
		char *tstr = asctime(tm);

		tstr[strlen(tstr)-1] = 0;
		fprintf(fp, "Timestamp: %s %ld usec\n",
			tstr, tv.tv_usec);
	}

	return 0;
}

unsigned int print_name_and_link(const char *fmt,
				 const char *name, struct rtattr *tb[])
{
	const char *link = NULL;
	unsigned int m_flag = 0;
	SPRINT_BUF(b1);

	if (tb[IFLA_LINK]) {
		int iflink = rta_getattr_u32(tb[IFLA_LINK]);

		if (iflink) {
			if (tb[IFLA_LINK_NETNSID]) {
				if (is_json_context()) {
					print_int(PRINT_JSON,
						  "link_index", NULL, iflink);
				} else {
					link = ll_idx_n2a(iflink);
				}
			} else {
				link = ll_index_to_name(iflink);

				if (is_json_context()) {
					print_string(PRINT_JSON,
						     "link", NULL, link);
					link = NULL;
				}

				m_flag = ll_index_to_flags(iflink);
				m_flag = !(m_flag & IFF_UP);
			}
		} else {
			if (is_json_context())
				print_null(PRINT_JSON, "link", NULL, NULL);
			else
				link = "NONE";
		}

		if (link) {
			snprintf(b1, sizeof(b1), "%s@%s", name, link);
			name = b1;
		}
	}

	print_color_string(PRINT_ANY, COLOR_IFNAME, "ifname", fmt, name);

	return m_flag;
}

int cmdlineno;

/* Like glibc getline but handle continuation lines and comments */
ssize_t getcmdline(char **linep, size_t *lenp, FILE *in)
{
	ssize_t cc;
	char *cp;

	cc = getline(linep, lenp, in);
	if (cc < 0)
		return cc;	/* eof or error */
	++cmdlineno;

	cp = strchr(*linep, '#');
	if (cp)
		*cp = '\0';

	while ((cp = strstr(*linep, "\\\n")) != NULL) {
		char *line1 = NULL;
		size_t len1 = 0;
		ssize_t cc1;

		cc1 = getline(&line1, &len1, in);
		if (cc1 < 0) {
			fprintf(stderr, "Missing continuation line\n");
			return cc1;
		}

		++cmdlineno;
		*cp = 0;

		cp = strchr(line1, '#');
		if (cp)
			*cp = '\0';

		*lenp = strlen(*linep) + strlen(line1) + 1;
		*linep = realloc(*linep, *lenp);
		if (!*linep) {
			fprintf(stderr, "Out of memory\n");
			*lenp = 0;
			return -1;
		}
		cc += cc1 - 2;
		strcat(*linep, line1);
		free(line1);
	}
	return cc;
}

/* split command line into argument vector */
int makeargs(char *line, char *argv[], int maxargs)
{
	static const char ws[] = " \t\r\n";
	char *cp = line;
	int argc = 0;

	while (*cp) {
		/* skip leading whitespace */
		cp += strspn(cp, ws);

		if (*cp == '\0')
			break;

		if (argc >= (maxargs - 1)) {
			fprintf(stderr, "Too many arguments to command\n");
			exit(1);
		}

		/* word begins with quote */
		if (*cp == '\'' || *cp == '"') {
			char quote = *cp++;

			argv[argc++] = cp;
			/* find ending quote */
			cp = strchr(cp, quote);
			if (cp == NULL) {
				fprintf(stderr, "Unterminated quoted string\n");
				exit(1);
			}
		} else {
			argv[argc++] = cp;

			/* find end of word */
			cp += strcspn(cp, ws);
			if (*cp == '\0')
				break;
		}

		/* separate words */
		*cp++ = 0;
	}
	argv[argc] = NULL;

	return argc;
}

void print_nlmsg_timestamp(FILE *fp, const struct nlmsghdr *n)
{
	char *tstr;
	time_t secs = ((__u32 *)NLMSG_DATA(n))[0];
	long usecs = ((__u32 *)NLMSG_DATA(n))[1];

	tstr = asctime(localtime(&secs));
	tstr[strlen(tstr)-1] = 0;
	fprintf(fp, "Timestamp: %s %lu us\n", tstr, usecs);
}

char *int_to_str(int val, char *buf)
{
	sprintf(buf, "%d", val);
	return buf;
}

int get_guid(__u64 *guid, const char *arg)
{
	unsigned long tmp;
	char *endptr;
	int i;

#define GUID_STR_LEN 23
	/* Verify strict format: format string must be
	 * xx:xx:xx:xx:xx:xx:xx:xx where xx can be an arbitrary
	 * hex digit
	 */

	if (strlen(arg) != GUID_STR_LEN)
		return -1;

	/* make sure columns are in place */
	for (i = 0; i < 7; i++)
		if (arg[2 + i * 3] != ':')
			return -1;

	*guid = 0;
	for (i = 0; i < 8; i++) {
		tmp = strtoul(arg + i * 3, &endptr, 16);
		if (endptr != arg + i * 3 + 2)
			return -1;

		if (tmp > 255)
			return -1;

		*guid |= tmp << (56 - 8 * i);
	}

	return 0;
}

/* This is a necessary workaround for multicast route dumps */
int get_real_family(int rtm_type, int rtm_family)
{
	if (rtm_type != RTN_MULTICAST)
		return rtm_family;

	if (rtm_family == RTNL_FAMILY_IPMR)
		return AF_INET;

	if (rtm_family == RTNL_FAMILY_IP6MR)
		return AF_INET6;

	return rtm_family;
}

/* Based on copy_rtnl_link_stats() from kernel at net/core/rtnetlink.c */
static void copy_rtnl_link_stats64(struct rtnl_link_stats64 *stats64,
				   const struct rtnl_link_stats *stats)
{
	__u64 *a = (__u64 *)stats64;
	const __u32 *b = (const __u32 *)stats;
	const __u32 *e = b + sizeof(*stats) / sizeof(*b);

	while (b < e)
		*a++ = *b++;
}

#define IPSTATS_MIB_MAX_LEN	(__IPSTATS_MIB_MAX * sizeof(__u64))
static void get_snmp_counters(struct rtnl_link_stats64 *stats64,
			      struct rtattr *s)
{
	__u64 *mib = (__u64 *)RTA_DATA(s);

	memset(stats64, 0, sizeof(*stats64));

	stats64->rx_packets = mib[IPSTATS_MIB_INPKTS];
	stats64->rx_bytes = mib[IPSTATS_MIB_INOCTETS];
	stats64->tx_packets = mib[IPSTATS_MIB_OUTPKTS];
	stats64->tx_bytes = mib[IPSTATS_MIB_OUTOCTETS];
	stats64->rx_errors = mib[IPSTATS_MIB_INDISCARDS];
	stats64->tx_errors = mib[IPSTATS_MIB_OUTDISCARDS];
	stats64->multicast = mib[IPSTATS_MIB_INMCASTPKTS];
	stats64->rx_frame_errors = mib[IPSTATS_MIB_CSUMERRORS];
}

int get_rtnl_link_stats_rta(struct rtnl_link_stats64 *stats64,
			    struct rtattr *tb[])
{
	struct rtnl_link_stats stats;
	void *s;
	struct rtattr *rta;
	int size, len;

	if (tb[IFLA_STATS64]) {
		rta = tb[IFLA_STATS64];
		size = sizeof(struct rtnl_link_stats64);
		s = stats64;
	} else if (tb[IFLA_STATS]) {
		rta = tb[IFLA_STATS];
		size = sizeof(struct rtnl_link_stats);
		s = &stats;
	} else if (tb[IFLA_PROTINFO]) {
		struct rtattr *ptb[IPSTATS_MIB_MAX_LEN + 1];

		parse_rtattr_nested(ptb, IPSTATS_MIB_MAX_LEN,
				    tb[IFLA_PROTINFO]);
		if (ptb[IFLA_INET6_STATS])
			get_snmp_counters(stats64, ptb[IFLA_INET6_STATS]);
		return sizeof(*stats64);
	} else {
		return -1;
	}

	len = RTA_PAYLOAD(rta);
	if (len < size)
		memset(s + len, 0, size - len);
	else
		len = size;

	memcpy(s, RTA_DATA(rta), len);

	if (s != stats64)
		copy_rtnl_link_stats64(stats64, s);
	return size;
}

#ifdef NEED_STRLCPY
size_t strlcpy(char *dst, const char *src, size_t size)
{
	size_t srclen = strlen(src);

	if (size) {
		size_t minlen = min(srclen, size - 1);

		memcpy(dst, src, minlen);
		dst[minlen] = '\0';
	}
	return srclen;
}

size_t strlcat(char *dst, const char *src, size_t size)
{
	size_t dlen = strlen(dst);

	if (dlen >= size)
		return dlen + strlen(src);

	return dlen + strlcpy(dst + dlen, src, size - dlen);
}
#endif

void drop_cap(void)
{
#ifdef HAVE_LIBCAP
	/* don't harmstring root/sudo */
	if (getuid() != 0 && geteuid() != 0) {
		cap_t capabilities;
		cap_value_t net_admin = CAP_NET_ADMIN;
		cap_flag_t inheritable = CAP_INHERITABLE;
		cap_flag_value_t is_set;

		capabilities = cap_get_proc();
		if (!capabilities)
			exit(EXIT_FAILURE);
		if (cap_get_flag(capabilities, net_admin, inheritable,
		    &is_set) != 0)
			exit(EXIT_FAILURE);
		/* apps with ambient caps can fork and call ip */
		if (is_set == CAP_CLEAR) {
			if (cap_clear(capabilities) != 0)
				exit(EXIT_FAILURE);
			if (cap_set_proc(capabilities) != 0)
				exit(EXIT_FAILURE);
		}
		cap_free(capabilities);
	}
#endif
}

int get_time(unsigned int *time, const char *str)
{
	double t;
	char *p;

	t = strtod(str, &p);
	if (p == str)
		return -1;

	if (*p) {
		if (strcasecmp(p, "s") == 0 || strcasecmp(p, "sec") == 0 ||
		    strcasecmp(p, "secs") == 0)
			t *= TIME_UNITS_PER_SEC;
		else if (strcasecmp(p, "ms") == 0 || strcasecmp(p, "msec") == 0 ||
			 strcasecmp(p, "msecs") == 0)
			t *= TIME_UNITS_PER_SEC/1000;
		else if (strcasecmp(p, "us") == 0 || strcasecmp(p, "usec") == 0 ||
			 strcasecmp(p, "usecs") == 0)
			t *= TIME_UNITS_PER_SEC/1000000;
		else
			return -1;
	}

	*time = t;
	return 0;
}

static void print_time(char *buf, int len, __u32 time)
{
	double tmp = time;

	if (tmp >= TIME_UNITS_PER_SEC)
		snprintf(buf, len, "%.3gs", tmp/TIME_UNITS_PER_SEC);
	else if (tmp >= TIME_UNITS_PER_SEC/1000)
		snprintf(buf, len, "%.3gms", tmp/(TIME_UNITS_PER_SEC/1000));
	else
		snprintf(buf, len, "%uus", time);
}

char *sprint_time(__u32 time, char *buf)
{
	print_time(buf, SPRINT_BSIZE-1, time);
	return buf;
}

/* 64 bit times are represented internally in nanoseconds */
int get_time64(__s64 *time, const char *str)
{
	double nsec;
	char *p;

	nsec = strtod(str, &p);
	if (p == str)
		return -1;

	if (*p) {
		if (strcasecmp(p, "s") == 0 ||
		    strcasecmp(p, "sec") == 0 ||
		    strcasecmp(p, "secs") == 0)
			nsec *= NSEC_PER_SEC;
		else if (strcasecmp(p, "ms") == 0 ||
			 strcasecmp(p, "msec") == 0 ||
			 strcasecmp(p, "msecs") == 0)
			nsec *= NSEC_PER_MSEC;
		else if (strcasecmp(p, "us") == 0 ||
			 strcasecmp(p, "usec") == 0 ||
			 strcasecmp(p, "usecs") == 0)
			nsec *= NSEC_PER_USEC;
		else if (strcasecmp(p, "ns") == 0 ||
			 strcasecmp(p, "nsec") == 0 ||
			 strcasecmp(p, "nsecs") == 0)
			nsec *= 1;
		else
			return -1;
	}

	*time = nsec;
	return 0;
}

static void print_time64(char *buf, int len, __s64 time)
{
	double nsec = time;

	if (time >= NSEC_PER_SEC)
		snprintf(buf, len, "%.3gs", nsec/NSEC_PER_SEC);
	else if (time >= NSEC_PER_MSEC)
		snprintf(buf, len, "%.3gms", nsec/NSEC_PER_MSEC);
	else if (time >= NSEC_PER_USEC)
		snprintf(buf, len, "%.3gus", nsec/NSEC_PER_USEC);
	else
		snprintf(buf, len, "%lldns", time);
}

char *sprint_time64(__s64 time, char *buf)
{
	print_time64(buf, SPRINT_BSIZE-1, time);
	return buf;
}

int do_batch(const char *name, bool force,
	     int (*cmd)(int argc, char *argv[], void *data), void *data)
{
	char *line = NULL;
	size_t len = 0;
	int ret = EXIT_SUCCESS;

	if (name && strcmp(name, "-") != 0) {
		if (freopen(name, "r", stdin) == NULL) {
			fprintf(stderr,
				"Cannot open file \"%s\" for reading: %s\n",
				name, strerror(errno));
			return EXIT_FAILURE;
		}
	}

	cmdlineno = 0;
	while (getcmdline(&line, &len, stdin) != -1) {
		char *largv[100];
		int largc;

		largc = makeargs(line, largv, 100);
		if (!largc)
			continue;	/* blank line */

		if (cmd(largc, largv, data)) {
			fprintf(stderr, "Command failed %s:%d\n",
				name, cmdlineno);
			ret = EXIT_FAILURE;
			if (!force)
				break;
		}
	}

	if (line)
		free(line);

	return ret;
}

int parse_one_of(const char *msg, const char *realval, const char * const *list,
		 size_t len, int *p_err)
{
	int i;

	for (i = 0; i < len; i++) {
		if (list[i] && matches(realval, list[i]) == 0) {
			*p_err = 0;
			return i;
		}
	}

	fprintf(stderr, "Error: argument of \"%s\" must be one of ", msg);
	for (i = 0; i < len; i++)
		if (list[i])
			fprintf(stderr, "\"%s\", ", list[i]);
	fprintf(stderr, "not \"%s\"\n", realval);
	*p_err = -EINVAL;
	return 0;
}

bool parse_on_off(const char *msg, const char *realval, int *p_err)
{
	static const char * const values_on_off[] = { "off", "on" };

	return parse_one_of(msg, realval, values_on_off, ARRAY_SIZE(values_on_off), p_err);
}

int parse_mapping_gen(int *argcp, char ***argvp,
		      int (*key_cb)(__u32 *keyp, const char *key),
		      int (*mapping_cb)(__u32 key, char *value, void *data),
		      void *mapping_cb_data)
{
	int argc = *argcp;
	char **argv = *argvp;
	int ret = 0;

	while (argc > 0) {
		char *colon = strchr(*argv, ':');
		__u32 key;

		if (!colon)
			break;
		*colon = '\0';

		if (key_cb(&key, *argv)) {
			ret = 1;
			break;
		}
		if (mapping_cb(key, colon + 1, mapping_cb_data)) {
			ret = 1;
			break;
		}

		argc--, argv++;
	}

	*argcp = argc;
	*argvp = argv;
	return ret;
}

static int parse_mapping_num(__u32 *keyp, const char *key)
{
	return get_u32(keyp, key, 0);
}

int parse_mapping_num_all(__u32 *keyp, const char *key)
{
	if (matches(key, "all") == 0) {
		*keyp = (__u32) -1;
		return 0;
	}
	return parse_mapping_num(keyp, key);
}

int parse_mapping(int *argcp, char ***argvp, bool allow_all,
		  int (*mapping_cb)(__u32 key, char *value, void *data),
		  void *mapping_cb_data)
{
	if (allow_all)
		return parse_mapping_gen(argcp, argvp, parse_mapping_num_all,
					 mapping_cb, mapping_cb_data);
	else
		return parse_mapping_gen(argcp, argvp, parse_mapping_num,
					 mapping_cb, mapping_cb_data);
}

int str_map_lookup_str(const struct str_num_map *map, const char *needle)
{
	if (!needle)
		return -EINVAL;

	/* Process array which is NULL terminated by the string. */
	while (map && map->str) {
		if (strcmp(map->str, needle) == 0)
			return map->num;

		map++;
	}
	return -EINVAL;
}

const char *str_map_lookup_uint(const struct str_num_map *map, unsigned int val)
{
	unsigned int num = val;

	while (map && map->str) {
		if (num == map->num)
			return map->str;

		map++;
	}
	return NULL;
}

const char *str_map_lookup_u16(const struct str_num_map *map, uint16_t val)
{
	unsigned int num = val;

	while (map && map->str) {
		if (num == map->num)
			return map->str;

		map++;
	}
	return NULL;
}

const char *str_map_lookup_u8(const struct str_num_map *map, uint8_t val)
{
	unsigned int num = val;

	while (map && map->str) {
		if (num == map->num)
			return map->str;

		map++;
	}
	return NULL;
}

unsigned int get_str_char_count(const char *str, int match)
{
	unsigned int count = 0;
	const char *pos = str;

	while ((pos = strchr(pos, match))) {
		count++;
		pos++;
	}
	return count;
}

int str_split_by_char(char *str, char **before, char **after, int match)
{
	char *slash;

	slash = strrchr(str, match);
	if (!slash)
		return -EINVAL;
	*slash = '\0';
	*before = str;
	*after = slash + 1;
	return 0;
}

struct indent_mem *alloc_indent_mem(void)
{
	struct indent_mem *mem = malloc(sizeof(*mem));

	if (!mem)
		return NULL;
	strcpy(mem->indent_str, "");
	mem->indent_level = 0;
	return mem;
}

void free_indent_mem(struct indent_mem *mem)
{
	free(mem);
}

#define INDENT_STR_STEP 2

void inc_indent(struct indent_mem *mem)
{
	if (mem->indent_level + INDENT_STR_STEP > INDENT_STR_MAXLEN)
		return;
	mem->indent_level += INDENT_STR_STEP;
	memset(mem->indent_str, ' ', sizeof(mem->indent_str));
	mem->indent_str[mem->indent_level] = '\0';
}

void dec_indent(struct indent_mem *mem)
{
	if (mem->indent_level - INDENT_STR_STEP < 0)
		return;
	mem->indent_level -= INDENT_STR_STEP;
	mem->indent_str[mem->indent_level] = '\0';
}

void print_indent(struct indent_mem *mem)
{
	if (mem->indent_level)
		printf("%s", mem->indent_str);
}
