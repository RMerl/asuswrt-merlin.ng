/* -*- mode: c; c-file-style: "openbsd" -*- */
/*
 * Copyright (c) 2009 The NetBSD Foundation, Inc.
 *
 * This code is derived from software contributed to The NetBSD Foundation
 * by Roy Marples.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/* Several modifications to make the code more portable (and less robust and far less efficient) */

#include <stdio.h>
#include <errno.h>
#include <limits.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MINBUF 128

static ssize_t
___getdelim(char **buf, size_t *buflen,
    int sep, FILE *fp)
{
	int p;
	size_t len = 0, newlen;
	char *newb;

	if (buf == NULL || buflen == NULL) {
		errno = EINVAL;
		return -1;
	}

	/* If buf is NULL, we have to assume a size of zero */
	if (*buf == NULL)
		*buflen = 0;

	do {
		p = fgetc(fp);
		if (ferror(fp))
			return -1;
		if (p == EOF)
			break;

		/* Ensure we can handle it */
		if (len > SSIZE_MAX) {
			errno = EOVERFLOW;
			return -1;
		}
		newlen = len + 2; /* reserve space for NUL terminator */
		if (newlen > *buflen) {
			if (newlen < MINBUF)
				newlen = MINBUF;
#define powerof2(x) ((((x)-1)&(x))==0)
			if (!powerof2(newlen)) {
				/* Grow the buffer to the next power of 2 */
				newlen--;
				newlen |= newlen >> 1;
				newlen |= newlen >> 2;
				newlen |= newlen >> 4;
				newlen |= newlen >> 8;
				newlen |= newlen >> 16;
#if SIZE_MAX > 0xffffffffU
				newlen |= newlen >> 32;
#endif
				newlen++;
			}

			newb = realloc(*buf, newlen);
			if (newb == NULL)
				return -1;
			*buf = newb;
			*buflen = newlen;
		}

		(*buf)[len++] = p;
	} while (p != sep);

	/* POSIX demands we return -1 on EOF. */
	if (len == 0)
		return -1;

	if (*buf != NULL)
		(*buf)[len] = '\0';
	return (ssize_t)len;
}

ssize_t
getline(char **buf, size_t *buflen, FILE *fp)
{
	return ___getdelim(buf, buflen, '\n', fp);
}
