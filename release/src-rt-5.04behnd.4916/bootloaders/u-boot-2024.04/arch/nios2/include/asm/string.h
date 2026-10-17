/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2004, Psyent Corporation <www.psyent.com>
 * Scott McNutt <smcnutt@psyent.com>
 */
#ifndef __ASM_NIOS2_STRING_H_
#define __ASM_NIOS2_STRING_H_

#undef __HAVE_ARCH_STRRCHR
extern char * strrchr(const char * s, int c);

#undef __HAVE_ARCH_STRCHR
extern char * strchr(const char * s, int c);

#undef __HAVE_ARCH_MEMCPY
extern void * memcpy(void *, const void *, __kernel_size_t);

#undef __HAVE_ARCH_MEMMOVE
extern void * memmove(void *, const void *, __kernel_size_t);

#undef __HAVE_ARCH_MEMCHR
extern void * memchr(const void *, int, __kernel_size_t);

#undef __HAVE_ARCH_MEMSET
extern void * memset(void *, int, __kernel_size_t);

#undef __HAVE_ARCH_MEMZERO
extern void memzero(void *ptr, __kernel_size_t n);

#endif /* __ASM_NIOS2_STRING_H_ */
