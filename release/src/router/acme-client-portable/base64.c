/*	$Id$ */
/*
 * Copyright (c) 2016 Kristaps Dzonsons <kristaps@bsd.lv>
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHORS DISCLAIM ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */
#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <sys/types.h>

#include <assert.h>
#include <stdarg.h>
#include <stdlib.h>

#include "extern.h"

static const char b64[] = 
	"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
	"abcdefghijklmnopqrstuvwxyz"
	"0123456789+/";

/*
 * Compute the maximum buffer required for a base64 encoded string of
 * length "len".
 */
size_t
base64len(size_t len)
{

	return (((len + 2) / 3 * 4) + 1);
}

/*
 * Base64 computation.
 * This is heavily "assert"-d because Coverity complains.
 */
size_t 
base64buf(char *enc, const char *str, size_t len)
{
	size_t 	i, val;
	char 	*p;

	p = enc;

	for (i = 0; i < len - 2; i += 3) {
		val = (str[i] >> 2) & 0x3F;
		assert(val < sizeof(b64));
		*p++ = b64[val];

		val = ((str[i] & 0x3) << 4) | 
			((int)(str[i + 1] & 0xF0) >> 4);
		assert(val < sizeof(b64));
		*p++ = b64[val];

		val = ((str[i + 1] & 0xF) << 2) | 
			((int)(str[i + 2] & 0xC0) >> 6);
		assert(val < sizeof(b64));
		*p++ = b64[val];

		val = str[i + 2] & 0x3F;
		assert(val < sizeof(b64));
		*p++ = b64[val];
	}

	if (i < len) {
		val = (str[i] >> 2) & 0x3F;
		assert(val < sizeof(b64));
		*p++ = b64[val];

		if (i == (len - 1)) {
			val = ((str[i] & 0x3) << 4);
			assert(val < sizeof(b64));
			*p++ = b64[val];
			*p++ = '=';
		} else {
			val = ((str[i] & 0x3) << 4) |
				((int)(str[i + 1] & 0xF0) >> 4);
			assert(val < sizeof(b64));
			*p++ = b64[val];

			val = ((str[i + 1] & 0xF) << 2);
			assert(val < sizeof(b64));
			*p++ = b64[val];
		}
		*p++ = '=';
	}

	*p++ = '\0';
	return(p - enc);
}

/*
 * Pass a stream of bytes to be base64 encoded, then converted into
 * base64url format.
 * Returns NULL on allocation failure (not logged).
 */
char *
base64buf_url(const char *data, size_t len)
{
	size_t	 i, sz;
	char	*buf;

	sz = base64len(len);
	if (NULL == (buf = malloc(sz)))
		return (NULL);

	base64buf(buf, data, len);

	for (i = 0; i < sz; i++)
		if ('+' == buf[i])
			buf[i] = '-';
		else if ('/' == buf[i])
			buf[i] = '_';
		else if ('=' == buf[i])
			buf[i] = '\0';

	return (buf);
}
