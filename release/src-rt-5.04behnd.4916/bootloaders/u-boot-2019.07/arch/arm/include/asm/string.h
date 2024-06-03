#ifndef __ASM_ARM_STRING_H
#define __ASM_ARM_STRING_H

#include <config.h>

/*
 * We don't do inline string functions, since the
 * optimised inline asm versions are not small.
 */

#undef __HAVE_ARCH_STRRCHR
extern char * strrchr(const char * s, int c);

#undef __HAVE_ARCH_STRCHR
extern char * strchr(const char * s, int c);

#if CONFIG_IS_ENABLED(USE_ARCH_MEMCPY)
#define __HAVE_ARCH_MEMCPY
#endif
extern void * memcpy(void *, const void *, __kernel_size_t);

#undef __HAVE_ARCH_MEMMOVE
extern void * memmove(void *, const void *, __kernel_size_t);

#undef __HAVE_ARCH_MEMCHR
extern void * memchr(const void *, int, __kernel_size_t);

#undef __HAVE_ARCH_MEMZERO
#if CONFIG_IS_ENABLED(USE_ARCH_MEMSET)
#define __HAVE_ARCH_MEMSET
#endif
extern void * memset(void *, int, __kernel_size_t);

#if 0
extern void __memzero(void *ptr, __kernel_size_t n);

#define memset(p,v,n)							\
	({								\
		if ((n) != 0) {						\
			if (__builtin_constant_p((v)) && (v) == 0)	\
				__memzero((p),(n));			\
			else						\
				memset((p),(v),(n));			\
		}							\
		(p);							\
	})

#define memzero(p,n) ({ if ((n) != 0) __memzero((p),(n)); (p); })
#else
extern void memzero(void *ptr, __kernel_size_t n);
#endif

#endif
