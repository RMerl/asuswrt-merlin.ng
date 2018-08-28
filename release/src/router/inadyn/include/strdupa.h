/* ==========================================================================
 * strdupa.h - Re-implementation of glibc strdupa.
 * --------------------------------------------------------------------------
 * Copyright (c) 2009  William Ahern
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to permit
 * persons to whom the Software is furnished to do so, subject to the
 * following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN
 * NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
 * DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE
 * USE OR OTHER DEALINGS IN THE SOFTWARE.
 * ==========================================================================
 */
#ifndef LITE_STRDUPA_H
#define LITE_STRDUPA_H

#if !defined(HAVE_STRDUPA)
#if defined(strdupa)
#define HAVE_STRDUPA	1
#endif
#endif

#if !HAVE_STRDUPA
#if defined(__GNUC__)
#include <stddef.h>	/* size_t */
#include <string.h>	/* memcpy(3) strlen(3) */

#define strdupa(src) (__extension__ ({			\
	size_t len_	= strlen(src);			\
	char *dst_	= __builtin_alloca(len_ + 1);	\
	dst_[len_]	= '\0';				\
	(char *)memcpy(dst_, src, len_);		\
}))

#else  /* If not GCC, e.g. Clang */
#error strdupa() may use an unsupported GNU C API, please forward any fix to maintainer, cheers!
#endif /* __GNUC__ */
#endif /* !HAVE_STRDUPA */

#endif /* LITE_STRDUPA_H */
