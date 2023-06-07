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

#include <src/shared/util.h>

#define CONFIG_MAGIC	0x8723ab55

static const char *offset_to_str(uint16_t offset)
{
	switch (offset) {
	case 0x00f4:
		return "PCM_SETTING";
	case 0x000c:
		return "UART_CONFIG";
	case 0x003c:
		return "BD_ADDR";
	}

	return NULL;
}

static void analyze_memory(const uint8_t *buf, size_t len)
{
	const uint8_t *ptr = buf;
	uint32_t magic;
	uint16_t datalen;

	if (len < 6) {
		fprintf(stderr, "Invalid file length of %zu bytes\n", len);
		return;
	}

	magic = get_le32(ptr);
	datalen = get_le16(ptr + 4);

	printf("Signature: 0x%8.8x\n", magic);
	printf("Data len:  %u\n", datalen);

	if (magic != CONFIG_MAGIC) {
		fprintf(stderr, "Unsupported file signature\n");
		return;
	}

	ptr += 6;

	while (ptr < buf + datalen + 6) {
		uint16_t offset;
		uint8_t plen;
		const char *str;
		unsigned int i;

		offset = get_le16(ptr);
		plen = get_u8(ptr + 2);

		if (ptr + plen + 3 > buf + datalen + 6) {
			fprintf(stderr, "Invalid config entry size\n");
			break;
		}

		str = offset_to_str(offset);

		printf("len=%-3u offset=%4.4x,{ ", plen, offset);
		for (i = 0; i < plen; i++)
			printf("%2.2x ", ptr[3 + i]);
		printf("}%s%s\n", str ? "," : "", str ? : "");

		ptr += plen + 3;
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
	printf("Realtek Bluetooth firmware analyzer\n"
		"Usage:\n");
	printf("\trtlfw [options] <file>\n");
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
