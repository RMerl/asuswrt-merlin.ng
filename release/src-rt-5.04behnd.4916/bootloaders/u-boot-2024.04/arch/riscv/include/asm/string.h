/*
 * Copyright (C) 2011 Andes Technology Corporation
 * Copyright (C) 2010 Shawn Lin (nobuhiro@andestech.com)
 * Copyright (C) 2011 Macpaul Lin (macpaul@andestech.com)
 * Copyright (C) 2017 Rick Chen (rick@andestech.com)
 *
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file "COPYING" in the main directory of this archive
 * for more details.
 */

#ifndef __ASM_RISCV_STRING_H
#define __ASM_RISCV_STRING_H

/*
 * We don't do inline string functions, since the
 * optimised inline asm versions are not small.
 */

#undef __HAVE_ARCH_STRRCHR
#undef __HAVE_ARCH_STRCHR
#undef __HAVE_ARCH_MEMCHR
#undef __HAVE_ARCH_MEMZERO

#undef __HAVE_ARCH_MEMCPY
#if CONFIG_IS_ENABLED(USE_ARCH_MEMCPY)
#define __HAVE_ARCH_MEMCPY
#endif
extern void *memcpy(void *, const void *, __kernel_size_t);

#undef __HAVE_ARCH_MEMMOVE
#if CONFIG_IS_ENABLED(USE_ARCH_MEMMOVE)
#define __HAVE_ARCH_MEMMOVE
#endif
extern void *memmove(void *, const void *, __kernel_size_t);

#undef __HAVE_ARCH_MEMZERO
#if CONFIG_IS_ENABLED(USE_ARCH_MEMSET)
#define __HAVE_ARCH_MEMSET
#endif
extern void *memset(void *, int, __kernel_size_t);

#undef __HAVE_ARCH_STRLEN
#if CONFIG_IS_ENABLED(USE_ARCH_STRLEN)
#define __HAVE_ARCH_STRLEN
#endif
extern __kernel_size_t strlen(const char *);

#undef __HAVE_ARCH_STRCMP
#if CONFIG_IS_ENABLED(USE_ARCH_STRCMP)
#define __HAVE_ARCH_STRCMP
#endif
extern int strcmp(const char *, const char *);

#undef __HAVE_ARCH_STRNCMP
#if CONFIG_IS_ENABLED(USE_ARCH_STRNCMP)
#define __HAVE_ARCH_STRNCMP
#endif
extern int strncmp(const char *, const char *, size_t __kernel_size_t);

#endif /* __ASM_RISCV_STRING_H */
