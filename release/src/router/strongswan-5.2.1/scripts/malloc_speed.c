/*
 * Copyright (C) 2013 Martin Willi
 * Copyright (C) 2013 revosec aG
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
#include <time.h>
#include <library.h>
#include <utils/debug.h>

#ifdef HAVE_MALLINFO
#include <malloc.h>
#endif /* HAVE_MALLINFO */

static void start_timing(struct timespec *start)
{
	clock_gettime(CLOCK_THREAD_CPUTIME_ID, start);
}

static double end_timing(struct timespec *start)
{
	struct timespec end;

	clock_gettime(CLOCK_THREAD_CPUTIME_ID, &end);
	return (end.tv_nsec - start->tv_nsec) / 1000000000.0 +
			(end.tv_sec - start->tv_sec) * 1.0;
}

static void print_mallinfo()
{
#ifdef HAVE_MALLINFO
	struct mallinfo mi = mallinfo();

	printf("malloc: sbrk %d, mmap %d, used %d, free %d\n",
		   mi.arena, mi.hblkhd, mi.uordblks, mi.fordblks);
#endif /* HAVE_MALLINFO */
}

#define ALLOCS 1024
#define ROUNDS 2048

int main(int argc, char *argv[])
{
	struct timespec timing;
	int i, round;
	void *m[ALLOCS];
	/* a random set of allocations we test */
	int sizes[16] = { 1, 13, 100, 1000, 16, 10000, 50, 17,
					  123, 32, 8, 64, 8096, 1024, 123, 9 };

	library_init(NULL, "malloc_speed");
	atexit(library_deinit);

	print_mallinfo();

	start_timing(&timing);

	for (round = 0; round < ROUNDS; round++)
	{
		for (i = 0; i < ALLOCS; i++)
		{
			m[i] = malloc(sizes[(round + i) % countof(sizes)]);
		}
		for (i = 0; i < ALLOCS; i++)
		{
			free(m[i]);
		}
	}
	printf("time for %d malloc/frees, repeating %d rounds: %.4fs\n",
		   ALLOCS, ROUNDS, end_timing(&timing));

	print_mallinfo();

	return 0;
}
