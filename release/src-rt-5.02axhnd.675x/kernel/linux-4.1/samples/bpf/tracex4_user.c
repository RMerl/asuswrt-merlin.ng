/* Copyright (c) 2015 PLUMgrid, http://plumgrid.com
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of version 2 of the GNU General Public
 * License as published by the Free Software Foundation.
 */
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <stdbool.h>
#include <string.h>
#include <time.h>
#include <linux/bpf.h>
#include "libbpf.h"
#include "bpf_load.h"

struct pair {
	long long val;
	__u64 ip;
};

static __u64 time_get_ns(void)
{
	struct timespec ts;

	clock_gettime(CLOCK_MONOTONIC, &ts);
	return ts.tv_sec * 1000000000ull + ts.tv_nsec;
}

static void print_old_objects(int fd)
{
	long long val = time_get_ns();
	__u64 key, next_key;
	struct pair v;

	key = write(1, "\e[1;1H\e[2J", 12); /* clear screen */

	key = -1;
	while (bpf_get_next_key(map_fd[0], &key, &next_key) == 0) {
		bpf_lookup_elem(map_fd[0], &next_key, &v);
		key = next_key;
		if (val - v.val < 1000000000ll)
			/* object was allocated more then 1 sec ago */
			continue;
		printf("obj 0x%llx is %2lldsec old was allocated at ip %llx\n",
		       next_key, (val - v.val) / 1000000000ll, v.ip);
	}
}

int main(int ac, char **argv)
{
	char filename[256];
	int i;

	snprintf(filename, sizeof(filename), "%s_kern.o", argv[0]);

	if (load_bpf_file(filename)) {
		printf("%s", bpf_log_buf);
		return 1;
	}

	for (i = 0; ; i++) {
		print_old_objects(map_fd[1]);
		sleep(1);
	}

	return 0;
}
