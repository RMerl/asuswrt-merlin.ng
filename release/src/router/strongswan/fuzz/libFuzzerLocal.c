/*
 * Copyright (C) 2017 Tobias Brunner
 * HSR Hochschule fuer Technik Rapperswil
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.  See <http://www.fsf.org/copyleft/gpl.txt>.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * for more details.
 */

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <library.h>

extern int LLVMFuzzerTestOneInput(const unsigned char *data, size_t size);
__attribute__((weak)) extern int LLVMFuzzerInitialize(int *argc, char ***argv);

/**
 * This is a simple driver for the fuzz targets to verify test inputs outside
 * of OSS-Fuzz.
 *
 * Failures will usually cause crashes.
 */
int main(int argc, char **argv)
{
	chunk_t *data;
	int i, res = 0;

	fprintf(stderr, "%s: running %d inputs\n", argv[0], argc - 1);
	if (LLVMFuzzerInitialize)
	{
		LLVMFuzzerInitialize(&argc, &argv);
	}
	for (i = 1; i < argc; i++)
	{
		fprintf(stderr, "running: %s\n", argv[i]);
		data = chunk_map(argv[i], FALSE);
		if (!data)
		{
			fprintf(stderr, "opening %s failed: %s\n", argv[i], strerror(errno));
			return 1;
		}
		res = LLVMFuzzerTestOneInput(data->ptr, data->len);
		fprintf(stderr, "done:    %s: (%zd bytes)\n", argv[i], data->len);
		chunk_unmap(data);
		if (res)
		{
			break;
		}
	}
	fprintf(stderr, "%s: completed %d inputs\n", argv[0], i-1);
	return res;
}
