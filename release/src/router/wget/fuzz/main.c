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

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <fcntl.h>

#include "fuzzer.h"
#include "utils.h"

#ifdef TEST_RUN

#include <dirent.h>

#ifdef _WIN32
#  define SLASH '\\'
#else
#  define SLASH '/'
#endif

static int test_all_from(const char *dirname)
{
	DIR *dirp;

	if ((dirp = opendir(dirname))) {
		struct dirent *dp;

		while ((dp = readdir(dirp))) {
			if (*dp->d_name == '.') continue;

			char fname[strlen(dirname) + strlen(dp->d_name) + 2];
			snprintf(fname, sizeof(fname), "%s/%s", dirname, dp->d_name);

			struct file_memory *fmem;
			if ((fmem = wget_read_file(fname))) {
				printf("testing %ld bytes from '%s'\n", fmem->length, fname);
				fflush(stdout);
				LLVMFuzzerTestOneInput((uint8_t *)fmem->content, fmem->length);
				wget_read_file_free(fmem);
			}
		}
		closedir(dirp);
		return 0;
	}

	return 1;
}

int main(int argc, char **argv)
{
	const char *target;
	size_t target_len;

	if ((target = strrchr(argv[0], SLASH))) {
		if (strrchr(target, '/'))
			target = strrchr(target, '/');
	} else
		target = strrchr(argv[0], '/');

	target = target ? target + 1 : argv[0];

	if (strncmp(target, "lt-", 3) == 0)
		target += 3;

	target_len = strlen(target);

#ifdef _WIN32
	target_len -= 4; // ignore .exe
#endif

	{
		int rc;
		char corporadir[sizeof(SRCDIR) + 1 + target_len + 8];
		snprintf(corporadir, sizeof(corporadir), SRCDIR "/%.*s.in", (int) target_len, target);

		rc = test_all_from(corporadir);
		if (rc)
			fprintf(stderr, "Failed to find %s\n", corporadir);

		snprintf(corporadir, sizeof(corporadir), SRCDIR "/%.*s.repro", (int) target_len, target);
		if (test_all_from(corporadir) && rc)
			return 77; // SKIP
	}

	return 0;
}

#else

#ifndef __AFL_LOOP
static int __AFL_LOOP(int n)
{
	static int first = 1;

	if (first) {
		first = 0;
		return n && --n > 0;
	}

	return 0;
}
#endif

int main(int argc, char **argv)
{
	int ret;
	unsigned char buf[64 * 1024];

	while (__AFL_LOOP(10000)) { // only works with clang - we have to use 1 because static/global vars in wget
		ret = fread(buf, 1, sizeof(buf), stdin);
		if (ret < 0)
			return 0;

		LLVMFuzzerTestOneInput(buf, ret);
	}

	return 0;
}

#endif /* #ifdef TEST_RUN */
