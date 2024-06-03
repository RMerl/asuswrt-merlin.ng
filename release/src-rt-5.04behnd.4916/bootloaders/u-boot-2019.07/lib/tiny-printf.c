// SPDX-License-Identifier: LGPL-2.1+
/*
 * Tiny printf version for SPL
 *
 * Copied from:
 * http://www.sparetimelabs.com/printfrevisited/printfrevisited.php
 *
 * Copyright (C) 2004,2008  Kustaa Nyholm
 */

#include <common.h>
#include <stdarg.h>
#include <serial.h>
#include <linux/ctype.h>

struct printf_info {
	char *bf;	/* Digit buffer */
	char zs;	/* non-zero if a digit has been written */
	char *outstr;	/* Next output position for sprintf() */

	/* Output a character */
	void (*putc)(struct printf_info *info, char ch);
};

static void out(struct printf_info *info, char c)
{
	*info->bf++ = c;
}

static void out_dgt(struct printf_info *info, char dgt)
{
	out(info, dgt + (dgt < 10 ? '0' : 'a' - 10));
	info->zs = 1;
}

static void div_out(struct printf_info *info, unsigned long *num,
		    unsigned long div)
{
	unsigned char dgt = 0;

	while (*num >= div) {
		*num -= div;
		dgt++;
	}

	if (info->zs || dgt > 0)
		out_dgt(info, dgt);
}

#ifdef CONFIG_SPL_NET_SUPPORT
static void string(struct printf_info *info, char *s)
{
	char ch;

	while ((ch = *s++))
		out(info, ch);
}

static const char hex_asc[] = "0123456789abcdef";
#define hex_asc_lo(x)	hex_asc[((x) & 0x0f)]
#define hex_asc_hi(x)	hex_asc[((x) & 0xf0) >> 4]

static inline char *pack_hex_byte(char *buf, u8 byte)
{
	*buf++ = hex_asc_hi(byte);
	*buf++ = hex_asc_lo(byte);
	return buf;
}

static void mac_address_string(struct printf_info *info, u8 *addr,
				bool separator)
{
	/* (6 * 2 hex digits), 5 colons and trailing zero */
	char mac_addr[6 * 3];
	char *p = mac_addr;
	int i;

	for (i = 0; i < 6; i++) {
		p = pack_hex_byte(p, addr[i]);
		if (separator && i != 5)
			*p++ = ':';
	}
	*p = '\0';

	string(info, mac_addr);
}

static char *put_dec_trunc(char *buf, unsigned int q)
{
	unsigned int d3, d2, d1, d0;
	d1 = (q >> 4) & 0xf;
	d2 = (q >> 8) & 0xf;
	d3 = (q >> 12);

	d0 = 6 * (d3 + d2 + d1) + (q & 0xf);
	q = (d0 * 0xcd) >> 11;
	d0 = d0 - 10 * q;
	*buf++ = d0 + '0'; /* least significant digit */
	d1 = q + 9 * d3 + 5 * d2 + d1;
	if (d1 != 0) {
		q = (d1 * 0xcd) >> 11;
		d1 = d1 - 10 * q;
		*buf++ = d1 + '0'; /* next digit */

		d2 = q + 2 * d2;
		if ((d2 != 0) || (d3 != 0)) {
			q = (d2 * 0xd) >> 7;
			d2 = d2 - 10 * q;
			*buf++ = d2 + '0'; /* next digit */

			d3 = q + 4 * d3;
			if (d3 != 0) {
				q = (d3 * 0xcd) >> 11;
				d3 = d3 - 10 * q;
				*buf++ = d3 + '0';  /* next digit */
				if (q != 0)
					*buf++ = q + '0'; /* most sign. digit */
			}
		}
	}
	return buf;
}

static void ip4_addr_string(struct printf_info *info, u8 *addr)
{
	/* (4 * 3 decimal digits), 3 dots and trailing zero */
	char ip4_addr[4 * 4];
	char temp[3];	/* hold each IP quad in reverse order */
	char *p = ip4_addr;
	int i, digits;

	for (i = 0; i < 4; i++) {
		digits = put_dec_trunc(temp, addr[i]) - temp;
		/* reverse the digits in the quad */
		while (digits--)
			*p++ = temp[digits];
		if (i != 3)
			*p++ = '.';
	}
	*p = '\0';

	string(info, ip4_addr);
}
#endif

/*
 * Show a '%p' thing.  A kernel extension is that the '%p' is followed
 * by an extra set of characters that are extended format
 * specifiers.
 *
 * Right now we handle:
 *
 * - 'M' For a 6-byte MAC address, it prints the address in the
 *       usual colon-separated hex notation.
 * - 'm' Same as above except there is no colon-separator.
 * - 'I4'for IPv4 addresses printed in the usual way (dot-separated
 *       decimal).
 */

static void pointer(struct printf_info *info, const char *fmt, void *ptr)
{
#ifdef DEBUG
	unsigned long num = (uintptr_t)ptr;
	unsigned long div;
#endif

	switch (*fmt) {
#ifdef DEBUG
	case 'a':

		switch (fmt[1]) {
		case 'p':
		default:
			num = *(phys_addr_t *)ptr;
			break;
		}
		break;
#endif
#ifdef CONFIG_SPL_NET_SUPPORT
	case 'm':
		return mac_address_string(info, ptr, false);
	case 'M':
		return mac_address_string(info, ptr, true);
	case 'I':
		if (fmt[1] == '4')
			return ip4_addr_string(info, ptr);
#endif
	default:
		break;
	}
#ifdef DEBUG
	div = 1UL << (sizeof(long) * 8 - 4);
	for (; div; div /= 0x10)
		div_out(info, &num, div);
#endif
}

static int _vprintf(struct printf_info *info, const char *fmt, va_list va)
{
	char ch;
	char *p;
	unsigned long num;
	char buf[12];
	unsigned long div;

	while ((ch = *(fmt++))) {
		if (ch != '%') {
			info->putc(info, ch);
		} else {
			bool lz = false;
			int width = 0;
			bool islong = false;

			ch = *(fmt++);
			if (ch == '-')
				ch = *(fmt++);

			if (ch == '0') {
				ch = *(fmt++);
				lz = 1;
			}

			if (ch >= '0' && ch <= '9') {
				width = 0;
				while (ch >= '0' && ch <= '9') {
					width = (width * 10) + ch - '0';
					ch = *fmt++;
				}
			}
			if (ch == 'l') {
				ch = *(fmt++);
				islong = true;
			}

			info->bf = buf;
			p = info->bf;
			info->zs = 0;

			switch (ch) {
			case '\0':
				goto abort;
			case 'u':
			case 'd':
				div = 1000000000;
				if (islong) {
					num = va_arg(va, unsigned long);
					if (sizeof(long) > 4)
						div *= div * 10;
				} else {
					num = va_arg(va, unsigned int);
				}

				if (ch == 'd') {
					if (islong && (long)num < 0) {
						num = -(long)num;
						out(info, '-');
					} else if (!islong && (int)num < 0) {
						num = -(int)num;
						out(info, '-');
					}
				}
				if (!num) {
					out_dgt(info, 0);
				} else {
					for (; div; div /= 10)
						div_out(info, &num, div);
				}
				break;
			case 'x':
				if (islong) {
					num = va_arg(va, unsigned long);
					div = 1UL << (sizeof(long) * 8 - 4);
				} else {
					num = va_arg(va, unsigned int);
					div = 0x10000000;
				}
				if (!num) {
					out_dgt(info, 0);
				} else {
					for (; div; div /= 0x10)
						div_out(info, &num, div);
				}
				break;
			case 'c':
				out(info, (char)(va_arg(va, int)));
				break;
			case 's':
				p = va_arg(va, char*);
				break;
			case 'p':
				pointer(info, fmt, va_arg(va, void *));
				while (isalnum(fmt[0]))
					fmt++;
				break;
			case '%':
				out(info, '%');
			default:
				break;
			}

			*info->bf = 0;
			info->bf = p;
			while (*info->bf++ && width > 0)
				width--;
			while (width-- > 0)
				info->putc(info, lz ? '0' : ' ');
			if (p) {
				while ((ch = *p++))
					info->putc(info, ch);
			}
		}
	}

abort:
	return 0;
}

#if CONFIG_IS_ENABLED(PRINTF)
static void putc_normal(struct printf_info *info, char ch)
{
	putc(ch);
}

int vprintf(const char *fmt, va_list va)
{
	struct printf_info info;

	info.putc = putc_normal;
	return _vprintf(&info, fmt, va);
}

int printf(const char *fmt, ...)
{
	struct printf_info info;

	va_list va;
	int ret;

	info.putc = putc_normal;
	va_start(va, fmt);
	ret = _vprintf(&info, fmt, va);
	va_end(va);

	return ret;
}
#endif

static void putc_outstr(struct printf_info *info, char ch)
{
	*info->outstr++ = ch;
}

int sprintf(char *buf, const char *fmt, ...)
{
	struct printf_info info;
	va_list va;
	int ret;

	va_start(va, fmt);
	info.outstr = buf;
	info.putc = putc_outstr;
	ret = _vprintf(&info, fmt, va);
	va_end(va);
	*info.outstr = '\0';

	return ret;
}

/* Note that size is ignored */
int snprintf(char *buf, size_t size, const char *fmt, ...)
{
	struct printf_info info;
	va_list va;
	int ret;

	va_start(va, fmt);
	info.outstr = buf;
	info.putc = putc_outstr;
	ret = _vprintf(&info, fmt, va);
	va_end(va);
	*info.outstr = '\0';

	return ret;
}
