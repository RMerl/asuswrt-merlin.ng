#ifndef __ASM_I386_STRING_H
#define __ASM_I386_STRING_H

/*
 * We don't do inline string functions, since the
 * optimised inline asm versions are not small.
 */
#undef __HAVE_ARCH_STRNCPY
extern char *strncpy(char *__dest, __const__ char *__src, __kernel_size_t __n);

#undef __HAVE_ARCH_STRRCHR
extern char *strrchr(const char *s, int c);

#undef __HAVE_ARCH_STRCHR
extern char *strchr(const char *s, int c);

#ifdef CONFIG_X86_64

#undef __HAVE_ARCH_MEMCPY
extern void *memcpy(void *, const void *, __kernel_size_t);

#undef __HAVE_ARCH_MEMMOVE
extern void *memmove(void *, const void *, __kernel_size_t);

#undef __HAVE_ARCH_MEMSET
extern void *memset(void *, int, __kernel_size_t);

#else

#define __HAVE_ARCH_MEMCPY
extern void *memcpy(void *, const void *, __kernel_size_t);

#define __HAVE_ARCH_MEMMOVE
extern void *memmove(void *, const void *, __kernel_size_t);

#define __HAVE_ARCH_MEMSET
extern void *memset(void *, int, __kernel_size_t);

#endif /* CONFIG_X86_64 */

#undef __HAVE_ARCH_MEMCHR
extern void *memchr(const void *, int, __kernel_size_t);

#undef __HAVE_ARCH_MEMZERO
extern void memzero(void *ptr, __kernel_size_t n);

#endif
