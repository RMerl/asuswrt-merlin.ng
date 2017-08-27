/*
 * misc.c	Various miscellaneous functions.
 *
 * Version:	$Id$
 *
 *   This library is free software; you can redistribute it and/or
 *   modify it under the terms of the GNU Lesser General Public
 *   License as published by the Free Software Foundation; either
 *   version 2.1 of the License, or (at your option) any later version.
 *
 *   This library is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 *   Lesser General Public License for more details.
 *
 *   You should have received a copy of the GNU Lesser General Public
 *   License along with this library; if not, write to the Free Software
 *   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA
 *
 * Copyright 2000,2006  The FreeRADIUS server project
 */

RCSID("$Id$")

#include	<freeradius-devel/libradius.h>

#include	<ctype.h>
#include	<sys/file.h>
#include	<fcntl.h>
#include	<signal.h>

#define FR_PUT_LE16(a, val)\
	do {\
		a[1] = ((uint16_t) (val)) >> 8;\
		a[0] = ((uint16_t) (val)) & 0xff;\
	} while (0)

static int	fr_debugger_present = -1;

int	fr_dns_lookups = 0;
int	fr_debug_flag = 0;

/** Allocates a new talloc context from the root autofree context
 *
 * @param signum signal raised.
 */
static void _sigtrap_handler(UNUSED int signum)
{
    fr_debugger_present = 0;
    signal(SIGTRAP, SIG_DFL);
}

/** Break in GDB (if were running under GDB)
 *
 * If the server is running under GDB this will raise a SIGTRAP which
 * will pause the running process.
 *
 * If the server is not running under GDB then this will do nothing.
 */
void fr_debug_break(void)
{
    if (fr_debugger_present == -1) {
    	fr_debugger_present = 0;
        signal(SIGTRAP, _sigtrap_handler);
        raise(SIGTRAP);
    } else if (fr_debugger_present == 1) {
    	raise(SIGTRAP);
    }
}

/*
 *	Return an IP address in standard dot notation
 *
 *	FIXME: DELETE THIS
 */
char const *ip_ntoa(char *buffer, uint32_t ipaddr)
{
	ipaddr = ntohl(ipaddr);

	sprintf(buffer, "%d.%d.%d.%d",
		(ipaddr >> 24) & 0xff,
		(ipaddr >> 16) & 0xff,
		(ipaddr >>  8) & 0xff,
		(ipaddr      ) & 0xff);
	return buffer;
}

/*
 *	Internal wrapper for locking, to minimize the number of ifdef's
 *
 *	Use fcntl or error
 */
int rad_lockfd(int fd, int lock_len)
{
#ifdef F_WRLCK
	struct flock fl;

	fl.l_start = 0;
	fl.l_len = lock_len;
	fl.l_pid = getpid();
	fl.l_type = F_WRLCK;
	fl.l_whence = SEEK_CUR;

	return fcntl(fd, F_SETLKW, (void *)&fl);
#else
#error "missing definition for F_WRLCK, all file locks will fail"

	return -1;
#endif
}

/*
 *	Internal wrapper for locking, to minimize the number of ifdef's
 *
 *	Lock an fd, prefer lockf() over flock()
 *	Nonblocking version.
 */
int rad_lockfd_nonblock(int fd, int lock_len)
{
#ifdef F_WRLCK
	struct flock fl;

	fl.l_start = 0;
	fl.l_len = lock_len;
	fl.l_pid = getpid();
	fl.l_type = F_WRLCK;
	fl.l_whence = SEEK_CUR;

	return fcntl(fd, F_SETLK, (void *)&fl);
#else
#error "missing definition for F_WRLCK, all file locks will fail"

	return -1;
#endif
}

/*
 *	Internal wrapper for unlocking, to minimize the number of ifdef's
 *	in the source.
 *
 *	Unlock an fd, prefer lockf() over flock()
 */
int rad_unlockfd(int fd, int lock_len)
{
#ifdef F_WRLCK
	struct flock fl;

	fl.l_start = 0;
	fl.l_len = lock_len;
	fl.l_pid = getpid();
	fl.l_type = F_WRLCK;
	fl.l_whence = SEEK_CUR;

	return fcntl(fd, F_UNLCK, (void *)&fl);
#else
#error "missing definition for F_WRLCK, all file locks will fail"

	return -1;
#endif
}

/*
 *	Return an interface-id in standard colon notation
 */
char *ifid_ntoa(char *buffer, size_t size, uint8_t const *ifid)
{
	snprintf(buffer, size, "%x:%x:%x:%x",
		 (ifid[0] << 8) + ifid[1], (ifid[2] << 8) + ifid[3],
		 (ifid[4] << 8) + ifid[5], (ifid[6] << 8) + ifid[7]);
	return buffer;
}


/*
 *	Return an interface-id from
 *	one supplied in standard colon notation.
 */
uint8_t *ifid_aton(char const *ifid_str, uint8_t *ifid)
{
	static char const xdigits[] = "0123456789abcdef";
	char const *p, *pch;
	int num_id = 0, val = 0, idx = 0;

	for (p = ifid_str; ; ++p) {
		if (*p == ':' || *p == '\0') {
			if (num_id <= 0)
				return NULL;

			/*
			 *	Drop 'val' into the array.
			 */
			ifid[idx] = (val >> 8) & 0xff;
			ifid[idx + 1] = val & 0xff;
			if (*p == '\0') {
				/*
				 *	Must have all entries before
				 *	end of the string.
				 */
				if (idx != 6)
					return NULL;
				break;
			}
			val = 0;
			num_id = 0;
			if ((idx += 2) > 6)
				return NULL;
		} else if ((pch = strchr(xdigits, tolower(*p))) != NULL) {
			if (++num_id > 4)
				return NULL;
			/*
			 *	Dumb version of 'scanf'
			 */
			val <<= 4;
			val |= (pch - xdigits);
		} else
			return NULL;
	}
	return ifid;
}


#ifndef HAVE_INET_PTON
static int inet_pton4(char const *src, struct in_addr *dst)
{
	int octet;
	unsigned int num;
	char const *p, *off;
	uint8_t tmp[4];
	static char const digits[] = "0123456789";

	octet = 0;
	p = src;
	while (1) {
		num = 0;
		while (*p && ((off = strchr(digits, *p)) != NULL)) {
			num *= 10;
			num += (off - digits);

			if (num > 255) return 0;

			p++;
		}
		if (!*p) break;

		/*
		 *	Not a digit, MUST be a dot, else we
		 *	die.
		 */
		if (*p != '.') {
			return 0;
		}

		tmp[octet++] = num;
		p++;
	}

	/*
	 *	End of the string.  At the fourth
	 *	octet is OK, anything else is an
	 *	error.
	 */
	if (octet != 3) {
		return 0;
	}
	tmp[3] = num;

	memcpy(dst, &tmp, sizeof(tmp));
	return 1;
}


#ifdef HAVE_STRUCT_SOCKADDR_IN6
/* int
 * inet_pton6(src, dst)
 *	convert presentation level address to network order binary form.
 * return:
 *	1 if `src' is a valid [RFC1884 2.2] address, else 0.
 * notice:
 *	(1) does not touch `dst' unless it's returning 1.
 *	(2) :: in a full address is silently ignored.
 * credit:
 *	inspired by Mark Andrews.
 * author:
 *	Paul Vixie, 1996.
 */
static int
inet_pton6(char const *src, unsigned char *dst)
{
	static char const xdigits_l[] = "0123456789abcdef",
			  xdigits_u[] = "0123456789ABCDEF";
	u_char tmp[IN6ADDRSZ], *tp, *endp, *colonp;
	char const *xdigits, *curtok;
	int ch, saw_xdigit;
	u_int val;

	memset((tp = tmp), 0, IN6ADDRSZ);
	endp = tp + IN6ADDRSZ;
	colonp = NULL;
	/* Leading :: requires some special handling. */
	if (*src == ':')
		if (*++src != ':')
			return (0);
	curtok = src;
	saw_xdigit = 0;
	val = 0;
	while ((ch = *src++) != '\0') {
		char const *pch;

		if ((pch = strchr((xdigits = xdigits_l), ch)) == NULL)
			pch = strchr((xdigits = xdigits_u), ch);
		if (pch != NULL) {
			val <<= 4;
			val |= (pch - xdigits);
			if (val > 0xffff)
				return (0);
			saw_xdigit = 1;
			continue;
		}
		if (ch == ':') {
			curtok = src;
			if (!saw_xdigit) {
				if (colonp)
					return (0);
				colonp = tp;
				continue;
			}
			if (tp + INT16SZ > endp)
				return (0);
			*tp++ = (u_char) (val >> 8) & 0xff;
			*tp++ = (u_char) val & 0xff;
			saw_xdigit = 0;
			val = 0;
			continue;
		}
		if (ch == '.' && ((tp + INADDRSZ) <= endp) &&
		    inet_pton4(curtok, (struct in_addr *) tp) > 0) {
			tp += INADDRSZ;
			saw_xdigit = 0;
			break;	/* '\0' was seen by inet_pton4(). */
		}
		return (0);
	}
	if (saw_xdigit) {
		if (tp + INT16SZ > endp)
			return (0);
		*tp++ = (u_char) (val >> 8) & 0xff;
		*tp++ = (u_char) val & 0xff;
	}
	if (colonp != NULL) {
		/*
		 * Since some memmove()'s erroneously fail to handle
		 * overlapping regions, we'll do the shift by hand.
		 */
		int const n = tp - colonp;
		int i;

		for (i = 1; i <= n; i++) {
			endp[- i] = colonp[n - i];
			colonp[n - i] = 0;
		}
		tp = endp;
	}
	if (tp != endp)
		return (0);
	/* bcopy(tmp, dst, IN6ADDRSZ); */
	memcpy(dst, tmp, IN6ADDRSZ);
	return (1);
}
#endif

/*
 *	Utility function, so that the rest of the server doesn't
 *	have ifdef's around IPv6 support
 */
int inet_pton(int af, char const *src, void *dst)
{
	if (af == AF_INET) {
		return inet_pton4(src, dst);
	}
#ifdef HAVE_STRUCT_SOCKADDR_IN6

	if (af == AF_INET6) {
		return inet_pton6(src, dst);
	}
#endif

	return -1;
}
#endif


#ifndef HAVE_INET_NTOP
/*
 *	Utility function, so that the rest of the server doesn't
 *	have ifdef's around IPv6 support
 */
char const *inet_ntop(int af, void const *src, char *dst, size_t cnt)
{
	if (af == AF_INET) {
		uint8_t const *ipaddr = src;

		if (cnt <= INET_ADDRSTRLEN) return NULL;

		snprintf(dst, cnt, "%d.%d.%d.%d",
			 ipaddr[0], ipaddr[1],
			 ipaddr[2], ipaddr[3]);
		return dst;
	}

	/*
	 *	If the system doesn't define this, we define it
	 *	in missing.h
	 */
	if (af == AF_INET6) {
		struct const in6_addr *ipaddr = src;

		if (cnt <= INET6_ADDRSTRLEN) return NULL;

		snprintf(dst, cnt, "%x:%x:%x:%x:%x:%x:%x:%x",
			 (ipaddr->s6_addr[0] << 8) | ipaddr->s6_addr[1],
			 (ipaddr->s6_addr[2] << 8) | ipaddr->s6_addr[3],
			 (ipaddr->s6_addr[4] << 8) | ipaddr->s6_addr[5],
			 (ipaddr->s6_addr[6] << 8) | ipaddr->s6_addr[7],
			 (ipaddr->s6_addr[8] << 8) | ipaddr->s6_addr[9],
			 (ipaddr->s6_addr[10] << 8) | ipaddr->s6_addr[11],
			 (ipaddr->s6_addr[12] << 8) | ipaddr->s6_addr[13],
			 (ipaddr->s6_addr[14] << 8) | ipaddr->s6_addr[15]);
		return dst;
	}

	return NULL;		/* don't support IPv6 */
}
#endif


/*
 *	Wrappers for IPv4/IPv6 host to IP address lookup.
 *	This API returns only one IP address, of the specified
 *	address family, or the first address (of whatever family),
 *	if AF_UNSPEC is used.
 */
int ip_hton(char const *src, int af, fr_ipaddr_t *dst)
{
	int rcode;
	struct addrinfo hints, *ai = NULL, *res = NULL;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = af;

	if ((rcode = getaddrinfo(src, NULL, &hints, &res)) != 0) {
		fr_strerror_printf("ip_hton: %s", gai_strerror(rcode));
		return -1;
	}

	for (ai = res; ai; ai = ai->ai_next) {
		if ((af == ai->ai_family) || (af == AF_UNSPEC))
			break;
	}

	if (!ai) {
		fr_strerror_printf("ip_hton failed to find requested information for host %.100s", src);
		freeaddrinfo(ai);
		return -1;
	}

	rcode = fr_sockaddr2ipaddr((struct sockaddr_storage *)ai->ai_addr,
				   ai->ai_addrlen, dst, NULL);
	freeaddrinfo(ai);
	if (!rcode) return -1;

	return 0;
}

/*
 *	Look IP addresses up, and print names (depending on DNS config)
 */
char const *ip_ntoh(fr_ipaddr_t const *src, char *dst, size_t cnt)
{
	struct sockaddr_storage ss;
	int error;
	socklen_t salen;

	/*
	 *	No DNS lookups
	 */
	if (!fr_dns_lookups) {
		return inet_ntop(src->af, &(src->ipaddr), dst, cnt);
	}

	if (!fr_ipaddr2sockaddr(src, 0, &ss, &salen)) {
		return NULL;
	}

	if ((error = getnameinfo((struct sockaddr *)&ss, salen, dst, cnt, NULL, 0,
				 NI_NUMERICHOST | NI_NUMERICSERV)) != 0) {
		fr_strerror_printf("ip_ntoh: %s", gai_strerror(error));
		return NULL;
	}
	return dst;
}


static char const *hextab = "0123456789abcdef";

/** Convert hex strings to binary data
 *
 * @param bin Buffer to write output to.
 * @param hex input string.
 * @param outlen length of output buffer (or length of input string / 2).
 * @return length of data written to buffer.
 */
size_t fr_hex2bin(uint8_t *bin, char const *hex, size_t outlen)
{
	size_t i;
	char *c1, *c2;

	for (i = 0; i < outlen; i++) {
		if(!(c1 = memchr(hextab, tolower((int) hex[i << 1]), 16)) ||
		   !(c2 = memchr(hextab, tolower((int) hex[(i << 1) + 1]), 16)))
			break;
		 bin[i] = ((c1-hextab)<<4) + (c2-hextab);
	}

	return i;
}


/** Convert binary data to a hex string
 *
 * Ascii encoded hex string will not be prefixed with '0x'
 *
 * @warning If the output buffer isn't long enough, we have a buffer overflow.
 *
 * @param[out] hex Buffer to write hex output.
 * @param[in] bin input.
 * @param[in] inlen of bin input.
 * @return length of data written to buffer.
 */
size_t fr_bin2hex(char *hex, uint8_t const *bin, size_t inlen)
{
	size_t i;

	for (i = 0; i < inlen; i++) {
		hex[0] = hextab[((*bin) >> 4) & 0x0f];
		hex[1] = hextab[*bin & 0x0f];
		hex += 2;
		bin++;
	}

	*hex = '\0';
	return inlen * 2;
}


/*
 *	So we don't have ifdef's in the rest of the code
 */
#ifndef HAVE_CLOSEFROM
int closefrom(int fd)
{
	int i;
	int maxfd = 256;

#ifdef _SC_OPEN_MAX
	maxfd = sysconf(_SC_OPEN_MAX);
	if (maxfd < 0) {
	  maxfd = 256;
	}
#endif

	if (fd > maxfd) return 0;

	/*
	 *	FIXME: return EINTR?
	 *
	 *	Use F_CLOSEM?
	 */
	for (i = fd; i < maxfd; i++) {
		close(i);
	}

	return 0;
}
#endif

int fr_ipaddr_cmp(fr_ipaddr_t const *a, fr_ipaddr_t const *b)
{
	if (a->af < b->af) return -1;
	if (a->af > b->af) return +1;

	switch (a->af) {
	case AF_INET:
		return memcmp(&a->ipaddr.ip4addr,
			      &b->ipaddr.ip4addr,
			      sizeof(a->ipaddr.ip4addr));
		break;

#ifdef HAVE_STRUCT_SOCKADDR_IN6
	case AF_INET6:
		if (a->scope < b->scope) return -1;
		if (a->scope > b->scope) return +1;

		return memcmp(&a->ipaddr.ip6addr,
			      &b->ipaddr.ip6addr,
			      sizeof(a->ipaddr.ip6addr));
		break;
#endif

	default:
		break;
	}

	return -1;
}

int fr_ipaddr2sockaddr(fr_ipaddr_t const *ipaddr, int port,
		       struct sockaddr_storage *sa, socklen_t *salen)
{
	if (ipaddr->af == AF_INET) {
		struct sockaddr_in s4;

		*salen = sizeof(s4);

		memset(&s4, 0, sizeof(s4));
		s4.sin_family = AF_INET;
		s4.sin_addr = ipaddr->ipaddr.ip4addr;
		s4.sin_port = htons(port);
		memset(sa, 0, sizeof(*sa));
		memcpy(sa, &s4, sizeof(s4));

#ifdef HAVE_STRUCT_SOCKADDR_IN6
	} else if (ipaddr->af == AF_INET6) {
		struct sockaddr_in6 s6;

		*salen = sizeof(s6);

		memset(&s6, 0, sizeof(s6));
		s6.sin6_family = AF_INET6;
		s6.sin6_addr = ipaddr->ipaddr.ip6addr;
		s6.sin6_port = htons(port);
		s6.sin6_scope_id = ipaddr->scope;
		memset(sa, 0, sizeof(*sa));
		memcpy(sa, &s6, sizeof(s6));
#endif
	} else {
		return 0;
	}

	return 1;
}


int fr_sockaddr2ipaddr(struct sockaddr_storage const *sa, socklen_t salen,
		       fr_ipaddr_t *ipaddr, int *port)
{
	if (sa->ss_family == AF_INET) {
		struct sockaddr_in	s4;

		if (salen < sizeof(s4)) {
			fr_strerror_printf("IPv4 address is too small");
			return 0;
		}

		memcpy(&s4, sa, sizeof(s4));
		ipaddr->af = AF_INET;
		ipaddr->ipaddr.ip4addr = s4.sin_addr;
		if (port) *port = ntohs(s4.sin_port);

#ifdef HAVE_STRUCT_SOCKADDR_IN6
	} else if (sa->ss_family == AF_INET6) {
		struct sockaddr_in6	s6;

		if (salen < sizeof(s6)) {
			fr_strerror_printf("IPv6 address is too small");
			return 0;
		}

		memcpy(&s6, sa, sizeof(s6));
		ipaddr->af = AF_INET6;
		ipaddr->ipaddr.ip6addr = s6.sin6_addr;
		if (port) *port = ntohs(s6.sin6_port);
		ipaddr->scope = s6.sin6_scope_id;
#endif

	} else {
		fr_strerror_printf("Unsupported address famility %d",
				   sa->ss_family);
		return 0;
	}

	return 1;
}

/** Convert UTF8 string to UCS2 encoding
 *
 * @note Borrowed from src/crypto/ms_funcs.c of wpa_supplicant project (http://hostap.epitest.fi/wpa_supplicant/)
 *
 * @param[out] out Where to write the ucs2 string.
 * @param[in] outlen Size of output buffer.
 * @param[in] in UTF8 string to convert.
 * @param[in] inlen length of UTF8 string.
 * @return the size of the UCS2 string written to the output buffer (in bytes).
 */
ssize_t fr_utf8_to_ucs2(uint8_t *out, size_t outlen, char const *in, size_t inlen)
{
	size_t i;
	uint8_t *start = out;

	for (i = 0; i < inlen; i++) {
		uint8_t c, c2, c3;

		c = in[i];
		if ((size_t)(out - start) >= outlen) {
			/* input too long */
			return -1;
		}

		/* One-byte encoding */
		if (c <= 0x7f) {
			FR_PUT_LE16(out, c);
			out += 2;
			continue;
		} else if ((i == (inlen - 1)) || ((size_t)(out - start) >= (outlen - 1))) {
			/* Incomplete surrogate */
			return -1;
		}

		c2 = in[++i];
		/* Two-byte encoding */
		if ((c & 0xe0) == 0xc0) {
			FR_PUT_LE16(out, ((c & 0x1f) << 6) | (c2 & 0x3f));
			out += 2;
			continue;
		}
		if ((i == inlen) || ((size_t)(out - start) >= (outlen - 1))) {
			/* Incomplete surrogate */
			return -1;
		}

		/* Three-byte encoding */
		c3 = in[++i];
		FR_PUT_LE16(out, ((c & 0xf) << 12) | ((c2 & 0x3f) << 6) | (c3 & 0x3f));
		out += 2;
	}

	return out - start;
}

