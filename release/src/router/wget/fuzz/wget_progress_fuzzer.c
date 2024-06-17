/*
 * Copyright (c) 2017-2019, 2021-2024 Free Software Foundation, Inc.
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
  #include "progress.h"

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
	(void) pathname;
	return fopen("/dev/null", mode);
}

FILE *fopen_wgetrc(const char *pathname, const char *mode)
{
	(void) pathname;
	(void) mode;
	return NULL;
}

#ifdef FUZZING
void exit_wget(int status)
{
	(void) status;
}
#endif


#define NAMEPOS (2 * sizeof(wgint) + sizeof(double))
int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
	void *progress;

	if (size > 4096) // same as max_len = ... in .options file
		return 0;

	if (size < NAMEPOS)
		return 0;

//	CLOSE_STDERR

	wgint start = ((wgint *) data)[0];
	wgint end = ((wgint *) data)[1];
	double dltime = ((wgint *) data)[2];

	if (start < 0 || end < 0)
		return 0;

	if (start > end) {
		wgint x = start;
		start = end;
		end = x;
	}

//	double dltime = ((double *) (data + 2 * sizeof(wgint)))[0];

	char *filename = strndup((char *) (data + NAMEPOS), size - NAMEPOS);

//	printf("%ld %ld %lf %s\n", start, end, dltime, filename);

	set_progress_implementation("bar:force"); // [:force][:noscroll]

	progress = progress_create (filename, start, end);
	progress_update (progress, 0, dltime);
	progress_update (progress, end - start, dltime);
	progress_finish (progress, dltime);

	set_progress_implementation("dot:default");// [:default|:binary|:mega|:giga]
	progress = progress_create (filename, start, end);
	progress_update (progress, 0, dltime);
	progress_update (progress, end - start, dltime);
	progress_finish (progress, dltime);

	set_progress_implementation("dot:binary");// [:default|:binary|:mega|:giga]
	progress = progress_create (filename, start, end);
	progress_update (progress, 0, dltime);
	progress_update (progress, end - start, dltime);
	progress_finish (progress, dltime);

	set_progress_implementation("dot:mega");// [:default|:binary|:mega|:giga]
	progress = progress_create (filename, start, end);
	progress_update (progress, 0, dltime);
	progress_update (progress, end - start, dltime);
	progress_finish (progress, dltime);

	set_progress_implementation("dot:giga");// [:default|:binary|:mega|:giga]
	progress = progress_create (filename, start, end);
	progress_update (progress, 0, dltime);
	progress_update (progress, end - start, dltime);
	progress_finish (progress, dltime);

	free(filename);

//	RESTORE_STDERR

	return 0;
}
