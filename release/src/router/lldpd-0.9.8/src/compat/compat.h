/* -*- mode: c; c-file-style: "openbsd" -*- */
/*
 * Copyright (c) 1991, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 *	@(#)queue.h	8.5 (Berkeley) 8/20/94
 */

#ifndef _COMPAT_H
#define _COMPAT_H

#if HAVE_CONFIG_H
#  include <config.h>
#endif

#include <stdio.h>
#include <stddef.h>
#include <stdarg.h>
#include <string.h>
#include <unistd.h>

#undef getopt

#if !HAVE_ASPRINTF
int vasprintf(char **, const char *, va_list) __attribute__ ((format (printf, 2, 0)));
int asprintf (char **, const char *, ...) __attribute__ ((format (printf, 2, 3)));
#endif

#if !HAVE_VSYSLOG
void vsyslog(int, const char *, va_list) __attribute__ ((format (printf, 2, 0)));
#endif

#if !HAVE_DAEMON
int daemon(int, int);
#endif

#if !HAVE_STRLCPY
size_t	strlcpy(char *, const char *, size_t);
#endif

#if !HAVE_STRNLEN
size_t	strnlen(const char *, size_t);
#endif

#if !HAVE_STRNDUP
char	*strndup(const char *, size_t);
#endif

#if !HAVE_STRTONUM
long long strtonum(const char *, long long, long long, const char **);
#endif

#if !HAVE_GETLINE
ssize_t getline(char **, size_t *, FILE *);
#endif

#if !HAVE_SETPROCTITLE
void setproctitle(const char *fmt, ...) __attribute__ ((format (printf, 1, 2)));
#endif

#if !HAVE_MALLOC
void *malloc(size_t size);
#endif

#if !HAVE_REALLOC
void *realloc(void *ptr, size_t size);
#endif

#endif
