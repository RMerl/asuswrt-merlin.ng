/*
 * Copyright (c) 2017-2024 Free Software Foundation, Inc.
 *
 * This file is part of GNU Wget.
 *
 * GNU Wget is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * GNU Wget is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Wget.  If not, see <https://www.gnu.org/licenses/>.
 */

#include <config.h>

#include <sys/types.h>
#include <dirent.h> // opendir, readdir
#include <stdint.h> // uint8_t
#include <stdio.h>  // fmemopen
#include <string.h>  // strncmp
#include <stdlib.h>  // free
#include <fcntl.h>  // open flags
#include <unistd.h>  // close
#include <setjmp.h> // longjmp, setjmp

#include "wget.h"
#undef fopen_wgetrc

#ifdef __cplusplus
  extern "C" {
#endif
  #include "ftp.h"

  // declarations for wget internal functions
  int main_wget(int argc, const char **argv);
  void cleanup(void);
  FILE *fopen_wget(const char *pathname, const char *mode);
  FILE *fopen_wgetrc(const char *pathname, const char *mode);
  void exit_wget(int status);
#ifdef __cplusplus
  }
#endif

#include "fuzzer.h"

FILE *fopen_wget(const char *pathname, const char *mode)
{
	return fopen("/dev/null", mode);
}

FILE *fopen_wgetrc(const char *pathname, const char *mode)
{
	return NULL;
}

static int do_jump;
static jmp_buf jmpbuf;
#ifdef FUZZING
void exit_wget(int status)
{
	longjmp(jmpbuf, 1);
}
#elif defined HAVE_DLFCN_H
#include <dlfcn.h> // dlsym
#ifndef RTLD_NEXT
#define RTLD_NEXT RTLD_GLOBAL
#endif
void exit(int status)
{
	if (do_jump) {
		longjmp(jmpbuf, 1);
	} else {
		void (*libc_exit)(int) = (void(*)(int)) dlsym (RTLD_NEXT, "exit");
		libc_exit(status);
	}
}
#endif

int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
#ifdef HAVE_FMEMOPEN
	FILE *fp;
	struct fileinfo *fi;

	if (size > 4096) // same as max_len = ... in .options file
		return 0;

	fp = fmemopen((void *) data, size, "r");
	if (!fp) return 0;

	CLOSE_STDERR

	do_jump = 1;

	if (setjmp(jmpbuf))
		goto done;

	fi = ftp_parse_ls_fp(fp, ST_UNIX);
	freefileinfo(fi);
	rewind(fp);

	fi = ftp_parse_ls_fp(fp, ST_VMS);
	freefileinfo(fi);
	rewind(fp);

	fi = ftp_parse_ls_fp(fp, ST_WINNT);
	freefileinfo(fi);
	rewind(fp);

	fi = ftp_parse_ls_fp(fp, ST_MACOS);

done:
	freefileinfo(fi);
	fclose(fp);

	do_jump = 0;

	RESTORE_STDERR
#endif
	return 0;
}
