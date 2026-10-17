// SPDX-License-Identifier: GPL-2.0+
/*
 * Generic network code. Moved from net.c
 *
 * Copyright 1994 - 2000 Neil Russell.
 * Copyright 2000 Roland Borde
 * Copyright 2000 Paolo Scaffardi
 * Copyright 2000-2002 Wolfgang Denk, wd@denx.de
 * Copyright 2009 Dirk Behme, dirk.behme@googlemail.com
 */

#include <net.h>
#include <net6.h>
#include <vsprintf.h>

struct in_addr string_to_ip(const char *s)
{
	struct in_addr addr;
	char *e;
	int i;

	addr.s_addr = 0;
	if (s == NULL)
		return addr;

	for (addr.s_addr = 0, i = 0; i < 4; ++i) {
		ulong val = s ? dectoul(s, &e) : 0;
		if (val > 255) {
			addr.s_addr = 0;
			return addr;
		}
		if (i != 3 && *e != '.') {
			addr.s_addr = 0;
			return addr;
		}
		addr.s_addr <<= 8;
		addr.s_addr |= (val & 0xFF);
		if (s) {
			s = (*e) ? e+1 : e;
		}
	}

	addr.s_addr = htonl(addr.s_addr);
	return addr;
}

#if IS_ENABLED(CONFIG_IPV6)
int string_to_ip6(const char *str, size_t len, struct in6_addr *addr)
{
	int colon_count = 0;
	int found_double_colon = 0;
	int xstart = 0;		/* first zero (double colon) */
	int section_num = 7;	/* num words the double colon represents */
	int i;
	const char *s = str;
	const char *const e = s + len;
	struct in_addr zero_ip = {.s_addr = 0};

	if (!str)
		return -1;

	/* First pass, verify the syntax and locate the double colon */
	while (s < e) {
		while (s < e && isxdigit((int)*s))
			s++;
		if (*s == '\0')
			break;
		if (*s != ':') {
			if (*s == '.' && section_num >= 2) {
				struct in_addr v4;

				while (s != str && *(s - 1) != ':')
					--s;
				v4 = string_to_ip(s);
				if (memcmp(&zero_ip, &v4,
					   sizeof(struct in_addr)) != 0) {
					section_num -= 2;
					break;
				}
			}
			/* This could be a valid address */
			break;
		}
		if (s == str) {
			/* The address begins with a colon */
			if (*++s != ':')
				/* Must start with a double colon or a number */
				goto out_err;
		} else {
			s++;
			if (found_double_colon)
				section_num--;
			else
				xstart++;
		}

		if (*s == ':') {
			if (found_double_colon)
				/* Two double colons are not allowed */
				goto out_err;
			found_double_colon = 1;
			section_num -= xstart;
			s++;
		}

		if (++colon_count == 7)
			/* Found all colons */
			break;
		++s;
	}

	if (colon_count == 0)
		goto out_err;
	if (*--s == ':')
		section_num++;

	/* Second pass, read the address */
	s = str;
	for (i = 0; i < 8; i++) {
		int val = 0;
		char *end;

		if (found_double_colon &&
		    i >= xstart && i < xstart + section_num) {
			addr->s6_addr16[i] = 0;
			continue;
		}
		while (*s == ':')
			s++;

		if (i == 6 && isdigit((int)*s)) {
			struct in_addr v4 = string_to_ip(s);

			if (memcmp(&zero_ip, &v4,
				   sizeof(struct in_addr)) != 0) {
				/* Ending with :IPv4-address */
				addr->s6_addr32[3] = v4.s_addr;
				break;
			}
		}

		val = simple_strtoul(s, &end, 16);
		if (end != e && *end != '\0' && *end != ':')
			goto out_err;
		addr->s6_addr16[i] = htons(val);
		s = end;
	}
	return 0;

out_err:
	return -1;
}
#endif

void string_to_enetaddr(const char *addr, uint8_t *enetaddr)
{
	char *end;
	int i;

	if (!enetaddr)
		return;

	for (i = 0; i < 6; ++i) {
		enetaddr[i] = addr ? hextoul(addr, &end) : 0;
		if (addr)
			addr = (*end) ? end + 1 : end;
	}
}

uint compute_ip_checksum(const void *vptr, uint nbytes)
{
	int sum, oddbyte;
	const unsigned short *ptr = vptr;

	sum = 0;
	while (nbytes > 1) {
		sum += *ptr++;
		nbytes -= 2;
	}
	if (nbytes == 1) {
		oddbyte = 0;
		((u8 *)&oddbyte)[0] = *(u8 *)ptr;
		((u8 *)&oddbyte)[1] = 0;
		sum += oddbyte;
	}
	sum = (sum >> 16) + (sum & 0xffff);
	sum += (sum >> 16);
	sum = ~sum & 0xffff;

	return sum;
}

uint add_ip_checksums(uint offset, uint sum, uint new)
{
	ulong checksum;

	sum = ~sum & 0xffff;
	new = ~new & 0xffff;
	if (offset & 1) {
		/*
		 * byte-swap the sum if it came from an odd offset; since the
		 * computation is endian-independent this works.
		 */
		new = ((new >> 8) & 0xff) | ((new << 8) & 0xff00);
	}
	checksum = sum + new;
	if (checksum > 0xffff)
		checksum -= 0xffff;

	return (~checksum) & 0xffff;
}

int ip_checksum_ok(const void *addr, uint nbytes)
{
	return !(compute_ip_checksum(addr, nbytes) & 0xfffe);
}
