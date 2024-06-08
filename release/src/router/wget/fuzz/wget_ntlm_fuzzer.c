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
#include <assert.h> // assert

#include "wget.h"

#undef fopen_wgetrc

#ifdef __cplusplus
  extern "C" {
#endif
  #include "http-ntlm.h"

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


int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
	if (size > 128) // same as max_len = ... in .options file
		return 0;

	//	CLOSE_STDERR

	struct ntlmdata *ntlm = (struct ntlmdata *) calloc(1, sizeof(struct ntlmdata));
	char *data0 = (char *) malloc(size + 4 + 1);
	bool ready;

	assert(ntlm && data0);

	memcpy(data0, "NTLM", 4);
	memcpy(data0 + 4, data, size);
	data0[size + 4] = 0;

	if (ntlm_input(ntlm, data0))
		free(ntlm_output(ntlm, data0 + 4, data0 + 4, &ready));

	free(data0);
	free(ntlm);

//	RESTORE_STDERR

	return 0;
}
