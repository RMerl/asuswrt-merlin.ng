/* $Id$ */

/*
 * err.h
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
 *
 *    @(#)err.h    8.1 (Berkeley) 6/2/93
 */

#pragma once

#include "defines.h"
#include <stdlib.h>

#ifdef DEBUG
extern int debug;
#endif

/*
 * We define the following functions for reporting errors, warnings and debug messages:
 * err()   - Fatal error.  Pass exit code followed by static string
 * errx()  - Fatal error.  Pass exit code, format string, one or more variables
 * warn()  - Warning. Pass static string
 * warnx() - Warning. Pass format string, one or more variables
 * dbg()   - Debug. Debug level to trigger, static string
 * dbgx()  - Debug. Debug level to trigger, format string, one or more variables
 * notice() - Informational only via stderr, format string, one or more variables
 */

/* gcc accepts __FUNCTION__, but C99 says use __func__.  Necessary for SunPro compiler */
#if !defined(__GNUC__) && !defined(__FUNCTION__)
#  define __FUNCTION__ __func__
#endif

void notice(const char *fmt, ...);

#ifdef DEBUG /* then err, errx, warn, warnx print file, func, line */

#define dbg(x, y) do { \
    if (debug >= x) \
        fprintf(stderr, "DEBUG%d in %s:%s() line %d: %s\n", x, __FILE__, __FUNCTION__, __LINE__, y); \
    } while(0)
    
#define dbgx(x, y, ...) do { \
    if (debug >= x) { \
        fprintf(stderr, "DEBUG%d in %s:%s() line %d: " y "\n", x, __FILE__, __FUNCTION__, __LINE__, __VA_ARGS__); \
    } \
} while(0)
        

#define warn(x) fprintf(stderr, "Warning in %s:%s() line %d:\n%s\n", __FILE__, __FUNCTION__, __LINE__, x)


#define warnx(x, ...) fprintf(stderr, "Warning in %s:%s() line %d:\n" x "\n", __FILE__, __FUNCTION__, __LINE__, __VA_ARGS__)

#define err(x, y) do { \
        fprintf(stderr, "\nFatal Error in %s:%s() line %d:\n%s\n", __FILE__, __FUNCTION__, __LINE__, y); \
        fflush(NULL); \
        exit(x); \
    } while (0)

#define errx(x, y, ...) do {\
        fprintf(stderr, "\nFatal Error in %s:%s() line %d:\n " y "\n", __FILE__, __FUNCTION__, __LINE__, __VA_ARGS__); \
        fflush(NULL); \
        exit(x); \
    } while (0)

#define err_no_exit(y) do { \
        fprintf(stderr, "\nFatal Error in %s:%s() line %d:\n%s\n", __FILE__, __FUNCTION__, __LINE__, y); \
        fflush(NULL); \
    } while (0)

#define err_no_exitx(y, ...) do {\
        fprintf(stderr, "\nFatal Error in %s:%s() line %d:\n " y "\n", __FILE__, __FUNCTION__, __LINE__, __VA_ARGS__); \
        fflush(NULL); \
    } while (0)

#else /* no detailed DEBUG info */

/* dbg() and dbgx() become no-ops for non-DEBUG builds */
#define dbg(x, y) { }
#define dbgx(x, y, ...) { }

#define warn(x) fprintf(stderr, "Warning: %s\n", x)

#define warnx(x, ...) fprintf(stderr, "Warning: " x "\n", __VA_ARGS__)

#define err(x, y) do {\
        fprintf(stderr, "\nFatal Error:\n%s\n", y); \
        fflush(NULL); \
        exit(x); \
    } while(0)

#define errx(x, y, ...) do {\
        fprintf(stderr, "\nFatal Error: " y "\n", __VA_ARGS__); \
        fflush(NULL); \
        exit(x); \
    } while (0)

#define err_no_exit(y) do {\
        fprintf(stderr, "\nFatal Error:\n%s\n", y); \
        fflush(NULL); \
    } while(0)

#define err_no_exitx(y, ...) do {\
        fprintf(stderr, "\nFatal Error: " y "\n", __VA_ARGS__); \
        fflush(NULL); \
    } while (0)
#endif /* DEBUG */
