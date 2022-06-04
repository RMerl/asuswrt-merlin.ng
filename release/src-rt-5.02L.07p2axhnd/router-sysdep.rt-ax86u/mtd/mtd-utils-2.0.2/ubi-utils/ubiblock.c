/*
 * Copyright (c) Ezequiel Garcia, 2014
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 as published by
 * the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program; if not, write to the Free Software Foundation, Inc., 51
 * Franklin St, Fifth Floor, Boston, MA 02110-1301 USA
 */

/*
 * An utility to create/remove block devices on top of UBI volumes.
 */

#define PROGRAM_NAME "ubiblock"

#include <fcntl.h>
#include <stdio.h>
#include <stdint.h>
#include <getopt.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>

#include <libubi.h>
#include "common.h"

struct args {
	const char *node;
	int create;
};

static struct args args;

static const char doc[] = PROGRAM_NAME " version " VERSION
			 " - a tool to create/remove block device interface from UBI volumes.";

static const char optionsstr[] =
"-c, --create               create block on top of a volume\n"
"-r, --remove               remove block from volume\n"
"-h, --help                 print help message\n"
"-V, --version              print program version";

static const char usage[] =
"Usage: " PROGRAM_NAME " [-c,-r] <UBI volume node file name>\n"
"Example: " PROGRAM_NAME " --create /dev/ubi0_0";

static const struct option long_options[] = {
	{ .name = "create",   .has_arg = 1, .flag = NULL, .val = 'c' },
	{ .name = "remove",   .has_arg = 1, .flag = NULL, .val = 'r' },
	{ .name = "help",     .has_arg = 0, .flag = NULL, .val = 'h' },
	{ .name = "version",  .has_arg = 0, .flag = NULL, .val = 'V' },
	{ NULL, 0, NULL, 0}
};

static int parse_opt(int argc, char * const argv[])
{
	while (1) {
		int key;

		key = getopt_long(argc, argv, "c:r:h?V", long_options, NULL);
		if (key == -1)
			break;

		switch (key) {
		case 'c':
			args.create = 1;
			/* fall-through */
		case 'r':
			args.node = optarg;
			break;
		case 'h':
			printf("%s\n\n", doc);
			printf("%s\n\n", usage);
			printf("%s\n", optionsstr);
			exit(EXIT_SUCCESS);
		case '?':
			printf("%s\n\n", doc);
			printf("%s\n\n", usage);
			printf("%s\n", optionsstr);
			return -1;

		case 'V':
			common_print_version();
			exit(EXIT_SUCCESS);

		default:
			fprintf(stderr, "Use -h for help\n");
			return -1;
		}
	}

	if (!args.node)
		return errmsg("invalid arguments (use -h for help)");

	return 0;
}

int main(int argc, char * const argv[])
{
	int err, fd;
	libubi_t libubi;

	err = parse_opt(argc, argv);
	if (err)
		return -1;

	libubi = libubi_open();
	if (!libubi) {
		if (errno == 0)
			errmsg("UBI is not present in the system");
		return sys_errmsg("cannot open libubi");
	}

	err = ubi_probe_node(libubi, args.node);
	if (err == 1) {
		errmsg("\"%s\" is an UBI device node, not an UBI volume node",
		       args.node);
		goto out_libubi;
	} else if (err < 0) {
		if (errno == ENODEV)
			errmsg("\"%s\" is not an UBI volume node", args.node);
		else
			sys_errmsg("error while probing \"%s\"", args.node);
		goto out_libubi;
	}

	fd = open(args.node, O_RDWR);
	if (fd == -1) {
		sys_errmsg("cannot open UBI volume \"%s\"", args.node);
		goto out_libubi;
	}

	if (args.create) {
		err = ubi_vol_block_create(fd);
		if (err) {
			if (errno == ENOSYS)
				errmsg("UBI block is not present in the system");
			if (errno == ENOTTY)
				errmsg("UBI block not supported (check your kernel version)");
			sys_errmsg("cannot create block device");
			goto out_close;
		}
	} else {
		err = ubi_vol_block_remove(fd);
		if (err) {
			if (errno == ENOSYS)
				errmsg("UBI block is not present in the system");
			if (errno == ENOTTY)
				errmsg("UBI block not supported (check your kernel version)");
			sys_errmsg("cannot remove block device");
			goto out_close;
		}
	}

	close(fd);
	libubi_close(libubi);
	return 0;

out_close:
	close(fd);
out_libubi:
	libubi_close(libubi);
	return -1;
}
