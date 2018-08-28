/*
 * Copyright (c) 2015 Peter Rosin <peda@lysator.liu.se>
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

#ifndef cfg_compat_h
#define cfg_compat_h

#include <stdio.h>
#include <stdarg.h>

#if defined _MSC_VER && _MSC_VER < 1900
#define snprintf c99_snprintf
#define vsnprintf c99_vsnprintf

static __inline int
c99_vsnprintf(char *buf, size_t size, const char *fmt, va_list ap)
{
    int count = -1;

    if(size)
	count = _vsnprintf_s(buf, size, _TRUNCATE, fmt, ap);
    if(count == -1)
	count = _vscprintf(fmt, ap);

    return count;
}

static __inline int
c99_snprintf(char *buf, size_t size, const char *fmt, ...)
{
    int count;
    va_list ap;

    va_start(ap, fmt);
    count = c99_vsnprintf(buf, size, fmt, ap);
    va_end(ap);

    return count;
}
#endif

#endif
