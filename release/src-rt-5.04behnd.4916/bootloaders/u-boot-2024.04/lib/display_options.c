// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2000-2002
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 */

#include <compiler.h>
#include <console.h>
#include <display_options.h>
#include <div64.h>
#include <version_string.h>
#include <linux/ctype.h>
#include <linux/kernel.h>
#include <asm/io.h>
#include <vsprintf.h>

char *display_options_get_banner_priv(bool newlines, const char *build_tag,
				      char *buf, int size)
{
	int len;

	len = snprintf(buf, size, "%s%s", newlines ? "\n\n" : "",
		       version_string);
	if (build_tag && len < size)
		len += snprintf(buf + len, size - len, ", Build: %s",
				build_tag);
	if (len > size - 3)
		len = size - 3;
	if (len < 0)
		len = 0;
	snprintf(buf + len, size - len, "\n\n");

	return buf;
}

#ifndef BUILD_TAG
#define BUILD_TAG NULL
#endif

char *display_options_get_banner(bool newlines, char *buf, int size)
{
	return display_options_get_banner_priv(newlines, BUILD_TAG, buf, size);
}

int display_options(void)
{
	char buf[DISPLAY_OPTIONS_BANNER_LENGTH];

	display_options_get_banner(true, buf, sizeof(buf));
	printf("%s", buf);

	return 0;
}

void print_freq(uint64_t freq, const char *s)
{
	unsigned long m = 0;
	uint32_t f;
	static const char names[] = {'G', 'M', 'k'};
	unsigned long d = 1e9;
	char c = 0;
	unsigned int i;

	for (i = 0; i < ARRAY_SIZE(names); i++, d /= 1000) {
		if (freq >= d) {
			c = names[i];
			break;
		}
	}

	if (!c) {
		printf("%llu Hz%s", freq, s);
		return;
	}

	f = do_div(freq, d);

	/* If there's a remainder, show the first few digits */
	if (f) {
		m = f;
		while (m > 1000)
			m /= 10;
		while (m && !(m % 10))
			m /= 10;
		if (m >= 100)
			m = (m / 10) + (m % 100 >= 50);
	}

	printf("%lu", (unsigned long) freq);
	if (m)
		printf(".%ld", m);
	printf(" %cHz%s", c, s);
}

void print_size(uint64_t size, const char *s)
{
	unsigned long m = 0, n;
	uint64_t f;
	static const char names[] = {'E', 'P', 'T', 'G', 'M', 'K'};
	unsigned long d = 10 * ARRAY_SIZE(names);
	char c = 0;
	unsigned int i;

	for (i = 0; i < ARRAY_SIZE(names); i++, d -= 10) {
		if (size >> d) {
			c = names[i];
			break;
		}
	}

	if (!c) {
		/*
		 * SPL tiny-printf is not capable for printing uint64_t.
		 * We have just checked that the size is small enought to fit
		 * unsigned int safely.
		 */
		printf("%u Bytes%s", (unsigned int)size, s);
		return;
	}

	n = size >> d;
	f = size & ((1ULL << d) - 1);

	/* If there's a remainder, deal with it */
	if (f) {
		m = (10ULL * f + (1ULL << (d - 1))) >> d;

		if (m >= 10) {
			m -= 10;
			n += 1;

			if (n == 1024 && i > 0) {
				n = 1;
				m = 0;
				c = names[i - 1];
			}
		}
	}

	printf ("%lu", n);
	if (m) {
		printf (".%ld", m);
	}
	printf (" %ciB%s", c, s);
}

#define MAX_LINE_LENGTH_BYTES		64
#define DEFAULT_LINE_LENGTH_BYTES	16

int hexdump_line(ulong addr, const void *data, uint width, uint count,
		 uint linelen, char *out, int size)
{
	/* linebuf as a union causes proper alignment */
	union linebuf {
		uint64_t uq[MAX_LINE_LENGTH_BYTES/sizeof(uint64_t) + 1];
		uint32_t ui[MAX_LINE_LENGTH_BYTES/sizeof(uint32_t) + 1];
		uint16_t us[MAX_LINE_LENGTH_BYTES/sizeof(uint16_t) + 1];
		uint8_t  uc[MAX_LINE_LENGTH_BYTES/sizeof(uint8_t) + 1];
	} lb;
	uint thislinelen;
	int i;
	ulong x;

	if (linelen * width > MAX_LINE_LENGTH_BYTES)
		linelen = MAX_LINE_LENGTH_BYTES / width;
	if (linelen < 1)
		linelen = DEFAULT_LINE_LENGTH_BYTES / width;

	/*
	 * Check the size here so that we don't need to use snprintf(). This
	 * helps to reduce code size
	 */
	if (size < HEXDUMP_MAX_BUF_LENGTH(linelen * width))
		return -ENOSPC;

	thislinelen = linelen;
	out += sprintf(out, "%08lx:", addr);

	/* check for overflow condition */
	if (count < thislinelen)
		thislinelen = count;

	/* Copy from memory into linebuf and print hex values */
	for (i = 0; i < thislinelen; i++) {
		if (width == 4)
			x = lb.ui[i] = *(volatile uint32_t *)data;
		else if (MEM_SUPPORT_64BIT_DATA && width == 8)
			x = lb.uq[i] = *(volatile ulong *)data;
		else if (width == 2)
			x = lb.us[i] = *(volatile uint16_t *)data;
		else
			x = lb.uc[i] = *(volatile uint8_t *)data;
		if (CONFIG_IS_ENABLED(USE_TINY_PRINTF))
			out += sprintf(out, " %x", (uint)x);
		else
			out += sprintf(out, " %0*lx", width * 2, x);
		data += width;
	}

	/* fill line with whitespace for nice ASCII print */
	for (i = 0; i < (linelen - thislinelen) * (width * 2 + 1); i++)
		*out++ = ' ';

	/* Print data in ASCII characters */
	for (i = 0; i < thislinelen * width; i++) {
		if (!isprint(lb.uc[i]) || lb.uc[i] >= 0x80)
			lb.uc[i] = '.';
	}
	lb.uc[i] = '\0';
	out += sprintf(out, "  %s", lb.uc);

	return thislinelen;
}

int print_buffer(ulong addr, const void *data, uint width, uint count,
		 uint linelen)
{
	if (linelen*width > MAX_LINE_LENGTH_BYTES)
		linelen = MAX_LINE_LENGTH_BYTES / width;
	if (linelen < 1)
		linelen = DEFAULT_LINE_LENGTH_BYTES / width;

	while (count) {
		uint thislinelen;
		char buf[HEXDUMP_MAX_BUF_LENGTH(width * linelen)];

		thislinelen = hexdump_line(addr, data, width, count, linelen,
					   buf, sizeof(buf));
		assert(thislinelen >= 0);
		puts(buf);
		putc('\n');

		/* update references */
		data += thislinelen * width;
		addr += thislinelen * width;
		count -= thislinelen;

		if (!IS_ENABLED(CONFIG_SPL_BUILD) && ctrlc())
			return -EINTR;
	}

	return 0;
}
