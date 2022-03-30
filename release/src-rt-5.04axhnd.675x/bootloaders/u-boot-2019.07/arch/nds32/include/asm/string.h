/*
 * Copyright (C) 2011 Andes Technology Corporation
 * Copyright (C) 2010 Shawn Lin (nobuhiro@andestech.com)
 * Copyright (C) 2011 Macpaul Lin (macpaul@andestech.com)
 *
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file "COPYING" in the main directory of this archive
 * for more details.
 */

#ifndef __ASM_NDS_STRING_H
#define __ASM_NDS_STRING_H

/*
 * We don't do inline string functions, since the
 * optimised inline asm versions are not small.
 */

#undef __HAVE_ARCH_STRRCHR
extern char *strrchr(const char *s, int c);

#undef __HAVE_ARCH_STRCHR
extern char *strchr(const char *s, int c);

#undef __HAVE_ARCH_MEMCPY
extern void *memcpy(void *, const void *, __kernel_size_t);

#undef __HAVE_ARCH_MEMMOVE
extern void *memmove(void *, const void *, __kernel_size_t);

#undef __HAVE_ARCH_MEMCHR
extern void *memchr(const void *, int, __kernel_size_t);

#undef __HAVE_ARCH_MEMZERO
#undef __HAVE_ARCH_MEMSET
extern void *memset(void *, int, __kernel_size_t);

#ifdef CONFIG_MARCO_MEMSET
extern void __memzero(void *ptr, __kernel_size_t n);

#define memset(p, v, n)							\
	({								\
		if ((n) != 0) {						\
			if (__builtin_constant_p((v)) && (v) == 0)	\
				__memzero((p), (n));			\
			else						\
				memset((p), (v), (n));			\
		}							\
		(p);							\
	})

#define memzero(p, n) ({ if ((n) != 0) __memzero((p), (n)); (p); })
#else
extern void memzero(void *ptr, __kernel_size_t n);
#endif

#endif /* __ASM_NDS_STRING_H */
