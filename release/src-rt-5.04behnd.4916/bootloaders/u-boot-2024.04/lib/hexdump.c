// SPDX-License-Identifier: GPL-2.0+
/*
 * lib/hexdump.c
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation. See README and COPYING for
 * more details.
 */

#include <hexdump.h>
#include <mapmem.h>
#include <vsprintf.h>
#include <linux/ctype.h>
#include <linux/compat.h>
#include <linux/log2.h>
#include <asm/unaligned.h>

#define MAX_LINE_LENGTH_BYTES	64

const char hex_asc[] = "0123456789abcdef";
const char hex_asc_upper[] = "0123456789ABCDEF";

#if CONFIG_IS_ENABLED(HEXDUMP)
int hex_dump_to_buffer(const void *buf, size_t len, int rowsize, int groupsize,
		       char *linebuf, size_t linebuflen, bool ascii)
{
	const u8 *ptr = buf;
	int ngroups;
	u8 ch;
	int j, lx = 0;
	int ascii_column;
	int ret;

	if (!rowsize)
		rowsize = 16;
	else
		rowsize = min(rowsize, MAX_LINE_LENGTH_BYTES);

	if (len > rowsize)		/* limit to one line at a time */
		len = rowsize;
	if (!is_power_of_2(groupsize) || groupsize > 8)
		groupsize = 1;
	if ((len % groupsize) != 0)	/* no mixed size output */
		groupsize = 1;

	ngroups = len / groupsize;
	ascii_column = rowsize * 2 + rowsize / groupsize + 1;

	if (!linebuflen)
		goto overflow1;

	if (!len)
		goto nil;

	if (groupsize == 8) {
		const u64 *ptr8 = buf;

		for (j = 0; j < ngroups; j++) {
			ret = snprintf(linebuf + lx, linebuflen - lx,
				       "%s%16.16llx", j ? " " : "",
				       get_unaligned(ptr8 + j));
			if (ret >= linebuflen - lx)
				goto overflow1;
			lx += ret;
		}
	} else if (groupsize == 4) {
		const u32 *ptr4 = buf;

		for (j = 0; j < ngroups; j++) {
			ret = snprintf(linebuf + lx, linebuflen - lx,
				       "%s%8.8x", j ? " " : "",
				       get_unaligned(ptr4 + j));
			if (ret >= linebuflen - lx)
				goto overflow1;
			lx += ret;
		}
	} else if (groupsize == 2) {
		const u16 *ptr2 = buf;

		for (j = 0; j < ngroups; j++) {
			ret = snprintf(linebuf + lx, linebuflen - lx,
				       "%s%4.4x", j ? " " : "",
				       get_unaligned(ptr2 + j));
			if (ret >= linebuflen - lx)
				goto overflow1;
			lx += ret;
		}
	} else {
		for (j = 0; j < len; j++) {
			if (linebuflen < lx + 2)
				goto overflow2;
			ch = ptr[j];
			linebuf[lx++] = hex_asc_hi(ch);
			if (linebuflen < lx + 2)
				goto overflow2;
			linebuf[lx++] = hex_asc_lo(ch);
			if (linebuflen < lx + 2)
				goto overflow2;
			linebuf[lx++] = ' ';
		}
		if (j)
			lx--;
	}
	if (!ascii)
		goto nil;

	while (lx < ascii_column) {
		if (linebuflen < lx + 2)
			goto overflow2;
		linebuf[lx++] = ' ';
	}
	for (j = 0; j < len; j++) {
		if (linebuflen < lx + 2)
			goto overflow2;
		ch = ptr[j];
		linebuf[lx++] = (isascii(ch) && isprint(ch)) ? ch : '.';
	}
nil:
	linebuf[lx] = '\0';
	return lx;
overflow2:
	linebuf[lx++] = '\0';
overflow1:
	return ascii ? ascii_column + len : (groupsize * 2 + 1) * ngroups - 1;
}

int print_hex_dump(const char *prefix_str, int prefix_type, int rowsize,
		   int groupsize, const void *buf, size_t len, bool ascii)
{
	const u8 *ptr = buf;
	int i, linelen, remaining = len;
	char linebuf[MAX_LINE_LENGTH_BYTES * 3 + 2 + MAX_LINE_LENGTH_BYTES + 1];

	if (!rowsize)
		rowsize = 16;
	else
		rowsize = min(rowsize, MAX_LINE_LENGTH_BYTES);

	for (i = 0; i < len; i += rowsize) {
		linelen = min(remaining, rowsize);
		remaining -= rowsize;

		hex_dump_to_buffer(ptr + i, linelen, rowsize, groupsize,
				   linebuf, sizeof(linebuf), ascii);

		switch (prefix_type) {
		case DUMP_PREFIX_ADDRESS:
			printf("%s%0*lx: %s\n", prefix_str,
			       IS_ENABLED(CONFIG_PHYS_64BIT) ? 16 : 8,
			       (ulong)map_to_sysmem(ptr) + i, linebuf);
			break;
		case DUMP_PREFIX_OFFSET:
			printf("%s%.8x: %s\n", prefix_str, i, linebuf);
			break;
		default:
			printf("%s%s\n", prefix_str, linebuf);
			break;
		}
		if (!IS_ENABLED(CONFIG_SPL_BUILD) && ctrlc())
			return -EINTR;
	}

	return 0;
}

void print_hex_dump_bytes(const char *prefix_str, int prefix_type,
			  const void *buf, size_t len)
{
	print_hex_dump(prefix_str, prefix_type, 16, 1, buf, len, true);
}
#else
/*
 * Some code in U-Boot copy-pasted from Linux kernel uses both
 * functions below so to keep stuff compilable we keep these stubs here.
 */
int print_hex_dump(const char *prefix_str, int prefix_type, int rowsize,
		   int groupsize, const void *buf, size_t len, bool ascii)
{
	return -ENOSYS;
}

void print_hex_dump_bytes(const char *prefix_str, int prefix_type,
			  const void *buf, size_t len)
{
}
#endif /* CONFIG_HEXDUMP */
