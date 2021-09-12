/* -*- mode: c; c-file-style: "openbsd" -*- */
/*
 * Copyright (c) 2012 Vincent Bernat <bernat@luffy.cx>
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#if HAVE_CONFIG_H
#  include <config.h>
#endif

#if ! defined FIXEDPOINT_H && defined ENABLE_LLDPMED
#define FIXEDPOINT_H

struct fp_number {
	struct {
		long long value;
		unsigned bits;
	} integer;
	struct {
		long long value;
		unsigned bits;
		unsigned precision;
	} fraction;
};
struct fp_number fp_strtofp(const char *, char **, unsigned, unsigned);
struct fp_number fp_buftofp(const unsigned char *, unsigned, unsigned, unsigned);
struct fp_number fp_negate(struct fp_number);
char *fp_fptostr(struct fp_number, const char *);
void  fp_fptobuf(struct fp_number, unsigned char *, unsigned);

#endif
