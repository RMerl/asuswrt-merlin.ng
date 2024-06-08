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

#include "wget.h"
#undef fopen_wgetrc

#ifdef __cplusplus
  extern "C" {
#endif
  #include "cookies.h"

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

#ifdef FUZZING
void exit_wget(int status)
{
}
#endif

int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
	struct cookie_jar *cookie_jar;
	char *set_cookie;

	if (size > 1024) // same as max_len = ... in .options file
		return 0;

	set_cookie = (char *) malloc(size + 1);
	memcpy(set_cookie, data, size);
	set_cookie[size] = 0;

	CLOSE_STDERR

	cookie_jar = cookie_jar_new();
	cookie_handle_set_cookie(cookie_jar, "x", 81, "p", set_cookie);
	cookie_handle_set_cookie(cookie_jar, "x", 81, "p", set_cookie);
	cookie_handle_set_cookie(cookie_jar, "x", 80, "p/d/", set_cookie);
	cookie_jar_delete(cookie_jar);

	RESTORE_STDERR

	free(set_cookie);

	return 0;
}
