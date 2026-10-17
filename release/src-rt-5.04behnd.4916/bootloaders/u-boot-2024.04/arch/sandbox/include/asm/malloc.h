/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Sandbox access to system malloc (i.e. not U-Boot's)
 *
 * Copyright 2020 Google LLC
 */

#ifndef __ASM_MALLOC_H
#define __ASM_MALLOC_H

void *malloc(size_t size);
void free(void *ptr);
void *calloc(size_t nmemb, size_t size);
void *realloc(void *ptr, size_t size);
void *reallocarray(void *ptr, size_t nmemb, size_t size);

/*
 * This header allows calling the system allocation routines. It makes no
 * sense to also include U-Boot's malloc.h since that redfines malloc to
 * have a 'dl' prefix. These two implementations cannot be mixed and matched
 * in the same file.
 */
#ifdef __MALLOC_H__
#error "This sandbox header file cannot be included with malloc.h"
#endif

#endif
