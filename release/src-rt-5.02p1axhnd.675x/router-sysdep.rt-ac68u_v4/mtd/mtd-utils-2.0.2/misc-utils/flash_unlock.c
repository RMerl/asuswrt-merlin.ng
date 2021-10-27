/*
 * flash_{lock,unlock}
 *
 * utilities for locking/unlocking sectors of flash devices
 */

enum flash_lock_request {
	REQUEST_LOCK,
	REQUEST_UNLOCK,
	REQUEST_ISLOCKED,
};

#ifndef PROGRAM_NAME
#define PROGRAM_NAME		"flash_unlock"
#define DEFAULT_REQUEST		REQUEST_UNLOCK
#else
#define DEFAULT_REQUEST		REQUEST_LOCK
#endif

#include <getopt.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <time.h>
#include <sys/ioctl.h>
#include <sys/mount.h>
#include <string.h>

#include "common.h"
#include <mtd/mtd-user.h>

static const char *flash_msg[] = {
	[ REQUEST_LOCK ]	= "lock",
	[ REQUEST_UNLOCK ]	= "unlock",
	[ REQUEST_ISLOCKED ]	= "check lock status",
};

static NORETURN void usage(int status)
{
	fprintf(status ? stderr : stdout,
		"Utility to lock, unlock, or check the lock status of the flash.\n"
		"Default action: %s\n"
		"\n"
		"Usage: %s [options] [--] <mtd device> [offset [block count]]\n"
		"\n"
		"Options:\n"
		" -h         --help              Display this help and exit\n"
		" -V         --version           Display version information and exit\n"
		" -i         --islocked          Check if flash region is locked\n"
		" -l         --lock              Lock a region of flash\n"
		" -u         --unlock            Unlock a region of flash\n"
		"\n"
		"If offset is not specified, it defaults to 0.\n"
		"If block count is not specified, it defaults to all blocks.\n"
		"A block count of -1 means all blocks.\n",
		flash_msg[DEFAULT_REQUEST],
		PROGRAM_NAME);
	exit(status);
}

static const char short_opts[] = "hiluV";
static const struct option long_opts[] = {
	{ "help",	no_argument,	0, 'h' },
	{ "islocked",	no_argument,	0, 'i' },
	{ "lock",	no_argument,	0, 'l' },
	{ "unlock",	no_argument,	0, 'u' },
	{ "version",	no_argument,	0, 'V' },
	{ NULL,		0,		0, 0 },
};

/* Program arguments */
static const char *dev, *offs_s, *count_s;
static enum flash_lock_request req = DEFAULT_REQUEST;

static void process_args(int argc, char *argv[])
{
	int arg_idx;
	int req_set = 0;

	for (;;) {
		int c;

		c = getopt_long(argc, argv, short_opts, long_opts, NULL);
		if (c == EOF)
			break;

		switch (c) {
		case 'h':
			usage(EXIT_SUCCESS);
		case 'i':
			req = REQUEST_ISLOCKED;
			req_set++;
			break;
		case 'l':
			req = REQUEST_LOCK;
			req_set++;
			break;
		case 'u':
			req = REQUEST_UNLOCK;
			req_set++;
			break;
		case 'V':
			common_print_version();
			exit(0);
		default:
			usage(EXIT_FAILURE);
		}
	}

	if (req_set > 1) {
		errmsg("cannot specify more than one lock/unlock/islocked option");
		usage(EXIT_FAILURE);
	}

	arg_idx = optind;

	/* Sanity checks */
	if (argc - arg_idx < 1) {
		errmsg("too few arguments");
		usage(EXIT_FAILURE);
	} else if (argc - arg_idx > 3) {
		errmsg("too many arguments");
		usage(EXIT_FAILURE);
	}

	/* First non-option argument */
	dev = argv[arg_idx++];

	/* Second non-option argument */
	if (arg_idx < argc)
		offs_s = argv[arg_idx++];
	else
		offs_s = NULL;

	/* Third non-option argument */
	if (arg_idx < argc)
		count_s = argv[arg_idx++];
	else
		count_s = NULL;

}

int main(int argc, char *argv[])
{
	int fd, request;
	struct mtd_info_user mtdInfo;
	struct erase_info_user mtdLockInfo;
	long count;
	int ret = 0;

	process_args(argc, argv);

	/* Get the device info to compare to command line sizes */
	fd = open(dev, O_RDWR);
	if (fd < 0)
		sys_errmsg_die("could not open: %s", dev);

	if (ioctl(fd, MEMGETINFO, &mtdInfo))
		sys_errmsg_die("could not get mtd info: %s", dev);

	/* Make sure user options are valid */
	if (offs_s) {
		mtdLockInfo.start = simple_strtol(offs_s, &ret);
		if (ret)
			errmsg_die("bad offset");
	} else {
		mtdLockInfo.start = 0;
	}
	if (mtdLockInfo.start >= mtdInfo.size)
		errmsg_die("%#x is beyond device size %#x",
			mtdLockInfo.start, mtdInfo.size);

	if (count_s) {
		count = simple_strtol(count_s, &ret);
		if (ret)
			errmsg_die("bad count");
		if (count == -1)
			mtdLockInfo.length = mtdInfo.size;
		else
			mtdLockInfo.length = mtdInfo.erasesize * count;
	} else {
		mtdLockInfo.length = mtdInfo.size;
	}
	if (mtdLockInfo.start + mtdLockInfo.length > mtdInfo.size)
		errmsg_die("range is more than device supports: %#x + %#x > %#x",
			mtdLockInfo.start, mtdLockInfo.length, mtdInfo.size);

	/* Finally do the operation */
	switch (req) {
	case REQUEST_LOCK:
		request = MEMLOCK;
		break;
	case REQUEST_UNLOCK:
		request = MEMUNLOCK;
		break;
	case REQUEST_ISLOCKED:
		request = MEMISLOCKED;
		break;
	default:
		errmsg_die("unknown request type: %d", req);
		break;
	}
	ret = ioctl(fd, request, &mtdLockInfo);
	if (ret < 0)
		sys_errmsg_die("could not %s device: %s\n",
				flash_msg[req], dev);

	if (req == REQUEST_ISLOCKED) {
		printf("Device: %s\n", dev);
		printf("Start: %#0x\n", mtdLockInfo.start);
		printf("Len: %#0x\n", mtdLockInfo.length);
		printf("Lock status: %s\n", ret ? "locked" : "unlocked");
		printf("Return code: %d\n", ret);
	}

	return 0;
}
