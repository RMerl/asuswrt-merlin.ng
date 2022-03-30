// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2008 Extreme Engineering Solutions, Inc.
 *
 * mmap/munmap implementation derived from:
 * Clamav Native Windows Port : mmap win32 compatibility layer
 * Copyright (c) 2005-2006 Gianluigi Tiesi <sherpya@netfarm.it>
 * Parts by Kees Zeelenberg <kzlg@users.sourceforge.net> (LibGW32C)
 */

#include "mingw_support.h"
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>
#include <assert.h>
#include <io.h>

int fsync(int fd)
{
	return _commit(fd);
}

void *mmap(void *addr, size_t len, int prot, int flags, int fd, int offset)
{
	void *map = NULL;
	HANDLE handle = INVALID_HANDLE_VALUE;
	DWORD cfm_flags = 0, mvf_flags = 0;

	switch (prot) {
	case PROT_READ | PROT_WRITE:
		cfm_flags = PAGE_READWRITE;
		mvf_flags = FILE_MAP_ALL_ACCESS;
		break;
	case PROT_WRITE:
		cfm_flags = PAGE_READWRITE;
		mvf_flags = FILE_MAP_WRITE;
		break;
	case PROT_READ:
		cfm_flags = PAGE_READONLY;
		mvf_flags = FILE_MAP_READ;
		break;
	default:
		return MAP_FAILED;
	}

	handle = CreateFileMappingA((HANDLE) _get_osfhandle(fd), NULL,
				cfm_flags, HIDWORD(len), LODWORD(len), NULL);
	if (!handle)
		return MAP_FAILED;

	map = MapViewOfFile(handle, mvf_flags, HIDWORD(offset),
			LODWORD(offset), len);
	CloseHandle(handle);

	if (!map)
		return MAP_FAILED;

	return map;
}

int munmap(void *addr, size_t len)
{
	if (!UnmapViewOfFile(addr))
		return -1;

	return 0;
}

/* Reentrant string tokenizer.  Generic version.
   Copyright (C) 1991,1996-1999,2001,2004,2007 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
 */

/* Parse S into tokens separated by characters in DELIM.
   If S is NULL, the saved pointer in SAVE_PTR is used as
   the next starting point.  For example:
	char s[] = "-abc-=-def";
	char *sp;
	x = strtok_r(s, "-", &sp);	// x = "abc", sp = "=-def"
	x = strtok_r(NULL, "-=", &sp);	// x = "def", sp = NULL
	x = strtok_r(NULL, "=", &sp);	// x = NULL
		// s = "abc\0-def\0"
*/
char *strtok_r(char *s, const char *delim, char **save_ptr)
{
	char *token;

	if (s == NULL)
		s = *save_ptr;

	/* Scan leading delimiters.  */
	s += strspn(s, delim);
	if (*s == '\0') {
		*save_ptr = s;
		return NULL;
	}

	/* Find the end of the token.  */
	token = s;
	s = strpbrk (token, delim);
	if (s == NULL) {
		/* This token finishes the string.  */
		*save_ptr = memchr(token, '\0', strlen(token));
	} else {
		/* Terminate the token and make *SAVE_PTR point past it.  */
		*s = '\0';
		*save_ptr = s + 1;
	}
	return token;
}

#include "getline.c"
