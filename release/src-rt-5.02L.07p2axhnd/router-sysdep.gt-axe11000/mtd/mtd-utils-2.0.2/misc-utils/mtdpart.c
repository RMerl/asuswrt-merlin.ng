/*
 *  mtdpart.c
 *
 *  Copyright 2015 The Chromium OS Authors.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 *  Overview:
 *   This utility adds or removes a partition from an MTD device.
 */

#define PROGRAM_NAME "mtdpart"

#include <fcntl.h>
#include <getopt.h>
#include <limits.h>
#include <linux/blkpg.h>
#include <stdio.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "common.h"

static void display_help(int status)
{
	fprintf(status == EXIT_SUCCESS ? stdout : stderr,
"Usage: %1$s add [OPTION] <MTD_DEVICE> <PART_NAME> <START> <SIZE>\n"
"       %1$s del [OPTION] <MTD_DEVICE> <PART_NUMBER>\n"
"Adds a partition to an MTD device, or remove an existing partition from it.\n"
"\n"
"  -h, --help    Display this help and exit\n"
"  -V, --version Output version information and exit\n"
"\n"
"START location and SIZE of the partition are in bytes. They should align on\n"
"eraseblock size.\n",
	PROGRAM_NAME
	);
	exit(status);
}

static void display_version(void)
{
	common_print_version();
	printf("%1$s comes with NO WARRANTY\n"
			"to the extent permitted by law.\n"
			"\n"
			"You may redistribute copies of %1$s\n"
			"under the terms of the GNU General Public Licence.\n"
			"See the file `COPYING' for more information.\n",
			PROGRAM_NAME);
	exit(EXIT_SUCCESS);
}

/* Command arguments */

typedef enum {
	COMMAND_ADD,
	COMMAND_DEL
} command_type;

static command_type		command;		/* add or del */
static const char		*mtddev;		/* mtd device name */
static const char		*part_name;		/* partition name */
static int			part_no;		/* partition number */
static long long		start_addr;		/* start address */
static long long		length;			/* partition size */

static void process_options(int argc, char * const argv[])
{
	int error = 0;

	for (;;) {
		int option_index = 0;
		static const char short_options[] = "hV";
		static const struct option long_options[] = {
			{"version", no_argument, 0, 'V'},
			{"help", no_argument, 0, 'h'},
			{0, 0, 0, 0},
		};

		int c = getopt_long(argc, argv, short_options,
				long_options, &option_index);
		if (c == EOF) {
			break;
		}

		switch (c) {
			case 'V':
				display_version();
				break;
			case 'h':
				display_help(EXIT_SUCCESS);
				break;
			case '?':
				error++;
				break;
		}
	}

	if ((argc - optind) < 3 || error)
		display_help(EXIT_FAILURE);

	const char *s_command = argv[optind++];
	mtddev = argv[optind++];

	if (strcmp(s_command, "del") == 0 && (argc - optind) == 1) {
		const char *s_part_no = argv[optind++];

		long tmp = simple_strtol(s_part_no, &error);
		if (tmp < 0)
		       errmsg_die("Can't specify negative partition number: %ld",
				  tmp);
		if (tmp > INT_MAX)
		       errmsg_die("Partition number exceeds INT_MAX: %ld",
				  tmp);

		part_no = tmp;
		command = COMMAND_DEL;
	} else if (strcmp(s_command, "add") == 0 && (argc - optind) == 3) {
		const char *s_start;
		const char *s_length;

		part_name = argv[optind++];
		s_start = argv[optind++];
		s_length = argv[optind++];

		if (strlen(part_name) >= BLKPG_DEVNAMELTH)
			errmsg_die("Partition name (%s) should be less than %d characters",
				   part_name, BLKPG_DEVNAMELTH);

		start_addr = simple_strtoll(s_start, &error);
		if (start_addr < 0)
		       errmsg_die("Can't specify negative start offset: %lld",
				  start_addr);

		length = simple_strtoll(s_length, &error);
		if (length < 0)
		       errmsg_die("Can't specify negative length: %lld",
				  length);

		command = COMMAND_ADD;
	} else
		display_help(EXIT_FAILURE);

	if (error)
		display_help(EXIT_FAILURE);
}


int main(int argc, char * const argv[])
{
	int fd;
	struct blkpg_partition part;
	struct blkpg_ioctl_arg arg;

	process_options(argc, argv);

	fd = open(mtddev, O_RDWR | O_CLOEXEC);
	if (fd == -1)
		sys_errmsg_die("Cannot open %s", mtddev);

	memset(&part, 0, sizeof(part));

	memset(&arg, 0, sizeof(arg));
	arg.datalen = sizeof(part);
	arg.data = &part;

	switch (command) {
		case COMMAND_ADD:
			part.start = start_addr;
			part.length = length;
			strncpy(part.devname, part_name, sizeof(part.devname));
			arg.op = BLKPG_ADD_PARTITION;
			break;
		case COMMAND_DEL:
			part.pno = part_no;
			arg.op = BLKPG_DEL_PARTITION;
			break;
	}

	if (ioctl(fd, BLKPG, &arg))
		sys_errmsg_die("Failed to issue BLKPG ioctl");

	close(fd);

	/* Exit happy */
	return EXIT_SUCCESS;
}
