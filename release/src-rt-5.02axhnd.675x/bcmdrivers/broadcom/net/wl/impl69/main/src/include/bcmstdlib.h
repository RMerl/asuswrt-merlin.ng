/*
 * prototypes for functions defined in bcmstdlib.c
 * Copyright (C) 2020, Broadcom. All Rights Reserved.
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 * SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
 * OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 *
 * <<Broadcom-WL-IPTag/Open:>>
 * $Id: bcmstdlib.h 777067 2019-07-18 09:28:30Z $:
 */

/*
 * bcmstdlib.h file should be used only to construct an OSL or alone without any OSL
 * It should not be used with any orbitarary OSL's as there could be a conflict
 * with some of the routines defined here.
*/

#ifndef	_BCMSTDLIB_H
#define	_BCMSTDLIB_H

#include <typedefs.h>
#include <bcmdefs.h>
#include <stdarg.h>

#ifndef INT_MAX
#define INT_MAX 2147483647 /* from limits.h */
#endif // endif

/* For backwards compatibility, define "BWL_NO_INTERNAL_STDLIB_SUPPORT" to
 * exclude support for the BRCM stdlib APIs. This should be cleaned-up such
 * that platforms that require the BRCM stdlib API should simply define
 * "BWL_INTERNAL_STDLIB_SUPPORT". This would eliminate the need for the
 * following #ifndef check.
 */
#ifndef BWL_NO_INTERNAL_STDLIB_SUPPORT
#define BWL_INTERNAL_STDLIB_SUPPORT
#endif // endif

#ifdef BWL_INTERNAL_STDLIB_SUPPORT
/* This should be cleaned-up such that platforms that require the BRCM stdlib
 * API should simply define "BWL_INTERNAL_STDLIB_SUPPORT". This would eliminate
 * the need for the following #ifdef check.
 */
#define PRINTF_BUFLEN	256

#if !defined(_CFE_)

typedef int FILE;
#define stdout ((FILE *)1)
#define stderr ((FILE *)2)

/* i/o functions */
extern int fputc(int c, FILE *stream);
#ifdef BCM_STDLIB_NO_PUTC
#define putc(c) do {(void)(c);} while (0)
#else
extern void putc(int c);
#endif // endif
/* extern int putc(int c, FILE *stream); */
#define putchar(c) putc(c)
/* XXX
 * When included by RTE posix stubs, "puts()" unfortunately caused
 * a shadow variable warning in one of the ThreadX files...mask out
 * the puts in such path using this #ifdef hack.
 * Use #include <bcmstdlib.h> directly where it needs to call puts().
 */
#if !defined(_STDLIB_H_) && !defined(_STRING_H_) && !defined(_STDIO_H_)
extern int fputs(const char *s, FILE *stream);
extern int puts(const char *s);
#endif // endif
extern int getc(void);
extern bool keypressed(void);

/* bcopy, bcmp, and bzero */
#define	bcopy(src, dst, len)	memcpy((dst), (src), (len))
#define	bcmp(b1, b2, len)	memcmp((b1), (b2), (len))
#define	bzero(b, len)		memset((b), '\0', (len))

extern unsigned long rand(void);

#define	atoi(s)	((int)(strtoul((s), NULL, 10)))
#ifdef ATE_BUILD
extern char *strcpy(char *dest, const char *src);
#else
#define strcpy(dest, src)	use_strncpy_instead(dest, src)
#endif // endif
#define strcat(dest, src)	use_strncat_instead(dest, src)
#endif /* !_CFE_ */

#if !defined(_CFE_)
/* string functions */
extern int printf(const char *fmt, ...)
	__attribute__ ((format (__printf__, 1, 2)));
#ifndef BCMSTDLIB_SNPRINTF_ONLY
extern int sprintf(char *buf, const char *fmt, ...)
	__attribute__ ((format (__printf__, 2, 3)));
#else
#define sprintf(buf, fmt, ...)	use_snprintf_instead(buf, fmt, __VA_ARGS__)
#endif // endif

extern int strcmp(const char *s1, const char *s2);
extern size_t strlen(const char *s);
extern char *strstr(const char *s, const char *find);
extern char *strncpy(char *dest, const char *src, size_t n);
extern int strncmp(const char *s1, const char *s2, size_t n);
extern char *strchr(const char *str, int c);
extern char *strrchr(const char *str, int c);
extern size_t strspn(const char *s1, const char *s2);
extern size_t strcspn(const char *s1, const char *s2);
extern unsigned long strtoul(const char *cp, char **endp, int base);
extern size_t strlcpy(char *dest, const char *src, size_t size);
#define strtol(nptr, endptr, base) ((long)strtoul((nptr), (endptr), (base)))

extern void *memmove(void *dest, const void *src, size_t n);
extern void *memchr(const void *s, int c, size_t n);

extern int vsprintf(char *buf, const char *fmt, va_list ap);
/* mem functions */
/* For EFI, using EFIDriverLib versions */
/* Cannot use memmem in ROM because of character array initialization wiht "" in gcc */
extern void *memset(void *dest, int c, size_t n);
/* Cannot use memcpy in ROM because of structure assignmnets in gcc */
extern void *memcpy(void *dest, const void *src, size_t n);
extern int memcmp(const void *s1, const void *s2, size_t n);

#endif /* !_CFE_ */
#endif   /* BWL_INTERNAL_STDLIB_SUPPORT */

extern int snprintf(char *str, size_t n, char const *fmt, ...)
	__attribute__ ((format (__printf__, 3, 4)));

extern int vsnprintf(char *buf, size_t size, const char *fmt, va_list ap);

/* setup output sendup func */
typedef int (*printf_sendup_output_fn_t)(void *ctx, char *buf, int len);
void printf_set_sendup_output_fn(printf_sendup_output_fn_t fn, void *ctx);

size_t strnlen(const char *s, size_t maxlen);

#ifdef DONGLEBUILD
void printf_set_rodata_invalid(void);
void printf_suppress_timestamp(bool suppress);
#endif /* DONGLEBUILD */
#endif /* _BCMSTDLIB_H */
