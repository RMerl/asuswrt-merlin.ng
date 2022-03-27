/*
 *
 *  BlueZ - Bluetooth protocol stack for Linux
 *
 *  Copyright (C) 2012-2014  Intel Corporation. All rights reserved.
 *
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

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
