/* SPDX-License-Identifier: LGPL-2.0+ */
/*
 * Copyright 2008 Extreme Engineering Solutions, Inc.
 */

#ifndef __MINGW_SUPPORT_H_
#define __WINGW_SUPPORT_H_	1

/* Defining __INSIDE_MSYS__ helps to prevent u-boot/mingw overlap */
#define __INSIDE_MSYS__	1

#include <windows.h>

/* mmap protections */
#define PROT_READ	0x1		/* Page can be read */
#define PROT_WRITE	0x2		/* Page can be written */
#define PROT_EXEC	0x4		/* Page can be executed */
#define PROT_NONE	0x0		/* Page can not be accessed */

/* Sharing types (must choose one and only one of these) */
#define MAP_SHARED	0x01		/* Share changes */
#define MAP_PRIVATE	0x02		/* Changes are private */

/* File perms */
#ifndef S_IRGRP
# define S_IRGRP 0
#endif
#ifndef S_IWGRP
# define S_IWGRP 0
#endif

/* Windows 64-bit access macros */
#define LODWORD(x) ((DWORD)((DWORDLONG)(x)))
#define HIDWORD(x) ((DWORD)(((DWORDLONG)(x) >> 32) & 0xffffffff))

typedef	UINT	uint;
typedef	ULONG	ulong;

int fsync(int fd);
void *mmap(void *, size_t, int, int, int, int);
int munmap(void *, size_t);
char *strtok_r(char *s, const char *delim, char **save_ptr);
#include "getline.h"

#endif /* __MINGW_SUPPORT_H_ */
