/* Collection of frog DNA
 *
 * Copyright (c) 2008-2021  Joachim Wiberg <troglobit@gmail.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */
#ifndef INADYN_COMPAT_H_
#define INADYN_COMPAT_H_

#include <errno.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>
#include <sys/param.h> /* MAX(), isset(), setbit(), TRUE, FALSE, et consortes. :-) */
#include <sys/types.h>
#include "strdupa.h"

/* From The Practice of Programming, by Kernighan and Pike */
#ifndef NELEMS
#define NELEMS(array) (sizeof(array) / sizeof(array[0]))
#endif

int     mkpath     (char *dir, mode_t mode);

#ifndef pidfile
int     pidfile    (const char *basename);
#endif

#ifndef strlcpy
size_t  strlcpy    (char *dst, const char *src, size_t siz);
#endif

#ifndef strlcat
size_t  strlcat    (char *dst, const char *src, size_t siz);
#endif

#ifndef strtonum
long long strtonum (const char *numstr, long long minval, long long maxval, const char **errstrp);
#endif

/* Convert string to natural number (0-2147483647), returns -1 on error. */
static inline int atonum(const char *str)
{
	int val = -1;
	const char *errstr;

	if (str) {
		val = strtonum(str, 0, INT32_MAX, &errstr);
		if (errstr)
			return -1;
	}

	return val;
}

/* Check if a file exists in the file system */
static inline int fexist(char *file)
{
	if (!file) {
		errno = EINVAL;
		return 0;	/* Doesn't exist ... */
	}

	if (-1 == access(file, F_OK))
		return 0;

	return 1;
}

/* Validate string, non NULL and not zero length */
static inline int string_valid(const char *s)
{
   return s && strlen(s);
}

/* Relaxed comparison, e.g., sys_string_match("small", "smaller") => TRUE */
static inline int string_match(const char *a, const char *b)
{
   size_t min = MIN(strlen(a), strlen(b));

   return !strncasecmp(a, b, min);
}

/* Strict comparison, e.g., sys_string_match("small", "smaller") => FALSE */
static inline int string_compare(const char *a, const char *b)
{
   return strlen(a) == strlen(b) && !strcmp(a, b);
}

#endif /* INADYN_COMPAT_H_ */
