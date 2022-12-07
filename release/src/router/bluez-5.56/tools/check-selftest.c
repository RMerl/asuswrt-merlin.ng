// SPDX-License-Identifier: GPL-2.0-or-later
/*
 *
 *  BlueZ - Bluetooth protocol stack for Linux
 *
 *  Copyright (C) 2012-2014  Intel Corporation. All rights reserved.
 *
 *
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#define _GNU_SOURCE
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

static void check_result(const char *name, const char *pathname)
{
	FILE *fp;
	int i;

	for (i = 0; i < 50; i++) {
		struct stat st;

		if (!stat(pathname, &st)) {
			printf("Found %s selftest result\n", name);
			break;
		}

		usleep(25 * 1000);
	}

	fp = fopen(pathname, "re");
	if (fp) {
		char result[32], *ptr;

		ptr = fgets(result, sizeof(result), fp);
		fclose(fp);

		ptr = strpbrk(result, "\r\n");
		if (ptr)
			*ptr = '\0';

		printf("%s: %s\n", name, result);
	}
}

int main(int argc, char *argv[])
{
	check_result("ECDH", "/sys/kernel/debug/bluetooth/selftest_ecdh");
	check_result("SMP",  "/sys/kernel/debug/bluetooth/selftest_smp");

	return 0;
}
