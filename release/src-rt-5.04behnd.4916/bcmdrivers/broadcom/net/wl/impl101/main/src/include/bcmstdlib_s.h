/*
 * Broadcom Secure Standard Library.
 *
 * Copyright (C) 2023, Broadcom. All Rights Reserved.
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
 * <<Broadcom-WL-IPTag/Dual:>>
 *
 * $Id$
 */

#ifndef	_bcmstdlib_s_h_
#define	_bcmstdlib_s_h_

#ifndef BWL_NO_INTERNAL_STDLIB_S_SUPPORT
#if !defined(__STDC_WANT_SECURE_LIB__) && !(defined(__STDC_LIB_EXT1__) && \
	defined(__STDC_WANT_LIB_EXT1__))
extern int memmove_s(void *dest, size_t destsz, const void *src, size_t n);
extern int memcpy_s(void *dest, size_t destsz, const void *src, size_t n);
extern int memset_s(void *dest, size_t destsz, int c, size_t n);
#endif /* !__STDC_WANT_SECURE_LIB__ && !(__STDC_LIB_EXT1__ && __STDC_WANT_LIB_EXT1__) */
#if !defined(FREEBSD) && !defined(MACOSX) && !defined(BCM_USE_PLATFORM_STRLCPY)
extern size_t strlcpy(char *dest, const char *src, size_t size);
#endif /* !defined(FREEBSD) && !defined(MACOSX) && !defined(BCM_USE_PLATFORM_STRLCPY) */
extern size_t strlcat_s(char *dest, const char *src, size_t size);

/* Remap xxx_s() APIs to use compiler builtin functions for C standard library functions.
 * The intent is to identify buffer overflow at compile-time for the safe stdlib APIs when
 * the user-specified destination buffer-size is incorrect.
 *
 * This is only intended as a compile-time test, and should be used by compile-only targets.
 */
#if defined(BCM_STDLIB_S_BUILTINS_TEST)
#define memmove_s(dest, destsz, src, n) ((void)(destsz), (int)__builtin_memmove((dest), (src), (n)))
#define memcpy_s(dest, destsz, src, n)  ((void)(destsz), (int)__builtin_memcpy((dest), (src), (n)))
#define memset_s(dest, destsz, c, n)    ((void)(destsz), (int)__builtin_memset((dest), (c), (n)))
#define strlcpy(dest, src, size)        ((void)(size), (size_t)__builtin_strcpy((dest), (src)))
#define strlcat_s(dest, src, size)      ((void)(size), (size_t)__builtin_strcat((dest), (src)))
#endif /* BCM_STDLIB_S_BUILTINS_TEST */

#endif /* !BWL_NO_INTERNAL_STDLIB_S_SUPPORT */
#endif /* _bcmstdlib_s_h_ */
