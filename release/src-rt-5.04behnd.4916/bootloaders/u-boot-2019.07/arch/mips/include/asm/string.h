/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) 1994, 95, 96, 97, 98, 2000, 01 Ralf Baechle
 * Copyright (c) 2000 by Silicon Graphics, Inc.
 * Copyright (c) 2001 MIPS Technologies, Inc.
 */
#ifndef _ASM_STRING_H
#define _ASM_STRING_H

/*
 * We don't do inline string functions, since the
 * optimised inline asm versions are not small.
 */

#undef __HAVE_ARCH_STRCPY
extern char *strcpy(char *__dest, __const__ char *__src);

#undef __HAVE_ARCH_STRNCPY
extern char *strncpy(char *__dest, __const__ char *__src, __kernel_size_t __n);

#undef __HAVE_ARCH_STRCMP
extern int strcmp(__const__ char *__cs, __const__ char *__ct);

#undef __HAVE_ARCH_STRNCMP
extern int strncmp(__const__ char *__cs, __const__ char *__ct, __kernel_size_t __count);

#undef __HAVE_ARCH_MEMSET
extern void *memset(void *__s, int __c, __kernel_size_t __count);

#undef __HAVE_ARCH_MEMCPY
extern void *memcpy(void *__to, __const__ void *__from, __kernel_size_t __n);

#undef __HAVE_ARCH_MEMMOVE
extern void *memmove(void *__dest, __const__ void *__src, __kernel_size_t __n);

#endif /* _ASM_STRING_H */
