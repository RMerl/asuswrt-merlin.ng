/* $Id$ */

/*
 * err.c
 *
 * Adapted from OpenBSD libc *err* *warn* code.
 *
 * Copyright (c) 2001-2010 Aaron Turner.
 *
 * Copyright (c) 2013-2022 Fred Klassen - AppNeta
 *
 * Copyright (c) 2000 Dug Song <dugsong@monkey.org>
 *
 * Copyright (c) 1993
 *    The Regents of the University of California.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *    This product includes software developed by the University of
 *    California, Berkeley and its contributors.
 * 4. Neither the name of the University nor the names of its contributors
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
 */

#include "config.h"
#include "common.h"
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * writes a notice message to stderr.  Always forces a newline
 */
void
notice(const char *fmt, ...)
{
    va_list ap;

    va_start(ap, fmt);
    if (fmt != NULL)
        (void)vfprintf(stderr, fmt, ap);
    (void)fprintf(stderr, "\n");
    va_end(ap);
    fflush(NULL);
}

/**
 * Inner call to dbgx() which prints the function, line & function along
 * with the message to stderr.  Always forces a newline.
 *
 * You don't actually want to call this!  use dbgx() instead!
 */
#ifdef DEBUG
void
our_verbose_dbgx(int dbg_level, const char *fmt, const char *function, const int line, const char *file, ...)
{
    va_list ap;

    if (debug < dbg_level)
        return;

    fprintf(stderr, "DEBUG%d in %s:%s() line %d: ", dbg_level, file, function, line);

    va_start(ap, file);

    if (fmt != NULL)
        (void)vfprintf(stderr, fmt, ap);
    (void)fprintf(stderr, "\n");
    va_end(ap);
    fflush(NULL);
}
#endif

/**
 * Inner call to errx() which when in DEBUG mode, prints the function, line & file
 * along with the actual error message to stderr.  Alawys forces a newline
 */
#ifdef DEBUG
void
our_verbose_errx(int eval, const char *fmt, const char *function, const int line, const char *file, ...)
{
#else
void
our_verbose_errx(int eval, const char *fmt, ...)
{
#endif

    va_list ap;

#ifdef DEBUG
    fprintf(stderr, "\nFatal Error in %s:%s() line %d:\n", file, function, line);
    va_start(ap, file);
#else
    fprintf(stderr, "\nFatal Error: ");
    va_start(ap, fmt);
#endif

    if (fmt != NULL)
        (void)vfprintf(stderr, fmt, ap);
    (void)fprintf(stderr, "\n");
    va_end(ap);
    exit(eval);
}

/**
 * Inner call to warnx() which when in DEBUG mode, prints the function, line & file
 * along with the actual warning to stderr.  Alawys forces a newline
 */
#ifdef DEBUG
void
our_verbose_warnx(const char *fmt, const char *function, const int line, const char *file, ...)
{
#else
void
our_verbose_warnx(const char *fmt, ...)
{
#endif

    va_list ap;
#ifdef DEBUG
    fprintf(stderr, "Warning in %s:%s() line %d:\n", file, function, line);
    va_start(ap, file);
#else
    fprintf(stderr, "Warning: ");
    va_start(ap, fmt);
#endif

    if (fmt != NULL)
        (void)vfprintf(stderr, fmt, ap);
    (void)fprintf(stderr, "\n");
    va_end(ap);
}
