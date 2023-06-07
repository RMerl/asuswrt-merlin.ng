// SPDX-License-Identifier: GPL-2.0-or-later
/*
 *
 *  BlueZ - Bluetooth protocol stack for Linux
 *
 *  Copyright (C) 2012-2013  Intel Corporation
 *
 *
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#define _GNU_SOURCE
#include <stdio.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <getopt.h>
#include <stdlib.h>
#include <stdint.h>
#include <sys/stat.h>
#include <sys/mman.h>

static void print_cmd(uint16_t opcode, const uint8_t *buf, uint8_t plen)
{
	switch (opcode) {
	case 0xfc4c:
		printf(" Write_RAM [address=0x%08x]",
			buf[0] | buf[1] << 8 | buf[2] << 16 | buf[3] << 24);
		break;
	case 0xfc4e:
		printf(" Launch_RAM [address=0x%08x]",
			buf[0] | buf[1] << 8 | buf[2] << 16 | buf[3] << 24);
		break;
	}
}

static void analyze_memory(const uint8_t *buf, size_t len)
{
	const uint8_t *ptr = buf;

	while (len >= 3) {
		uint16_t opcode = ptr[0] | ptr[1] << 8;
		uint8_t plen = ptr[2];

		ptr += 3;
		len -= 3;

		if (len < plen) {
			fprintf(stderr, "Corrupted file\n");
			break;
		}

		printf("opcode=0x%04x plen=%-3u", opcode, plen);
		print_cmd(opcode, ptr + 3, plen);
		printf("\n");

		ptr += plen;
		len -= plen;
	}
}

static void analyze_file(const char *pathname)
{
	struct stat st;
	void *map;
	int fd;

	printf("Analyzing %s\n", pathname);

	fd = open(pathname, O_RDONLY | O_CLOEXEC);
	if (fd < 0) {
		perror("Failed to open file");
		return;
	}

	if (fstat(fd, &st) < 0) {
		fprintf(stderr, "Failed get file size\n");
		close(fd);
		return;
	}

	if (st.st_size == 0) {
		fprintf(stderr, "Empty file\n");
		close(fd);
		return;
	}

	map = mmap(NULL, st.st_size, PROT_READ, MAP_SHARED, fd, 0);
	if (!map || map == MAP_FAILED) {
		fprintf(stderr, "Failed to map file\n");
		close(fd);
		return;
        }

	analyze_memory(map, st.st_size);

	munmap(map, st.st_size);
	close(fd);
}

static void usage(void)
{
	printf("Broadcom Bluetooth firmware analyzer\n"
		"Usage:\n");
	printf("\tbcmfw [options] <file>\n");
	printf("Options:\n"
		"\t-h, --help             Show help options\n");
}

static const struct option main_options[] = {
	{ "version", no_argument,       NULL, 'v' },
	{ "help",    no_argument,       NULL, 'h' },
	{ }
};

int main(int argc, char *argv[])
{
	int i;

	for (;;) {
		int opt;

		opt = getopt_long(argc, argv, "vh", main_options, NULL);
		if (opt < 0)
			break;

		switch (opt) {
		case 'v':
			printf("%s\n", VERSION);
			return EXIT_SUCCESS;
		case 'h':
			usage();
			return EXIT_SUCCESS;
		default:
			return EXIT_FAILURE;
		}
	}

	if (argc - optind < 1) {
		fprintf(stderr, "No input firmware files provided\n");
		return EXIT_FAILURE;
	}

	for (i = optind; i < argc; i++)
		analyze_file(argv[i]);

	return EXIT_SUCCESS;
}
