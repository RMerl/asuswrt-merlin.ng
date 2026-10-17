// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2020 Marvell International Ltd.
 */

#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <arpa/inet.h>
#include <linux/compiler.h>
#include <u-boot/crc.h>

#include "mkimage.h"

#include "../arch/mips/mach-octeon/include/mach/cvmx-bootloader.h"

#define BUF_SIZE	(16 * 1024)
#define NAME_LEN	100

/* word offset */
#define WOFFSETOF(type, elem)	(offsetof(type, elem) / 4)

static int stage2_flag;
static int stage_1_5_flag;
static int stage_1_flag;

/* Getoptions variables must be global */
static int failsafe_flag;
static int pciboot_flag;
static int env_flag;

static const struct option long_options[] = {
	/* These options set a flag. */
	{"failsafe", no_argument, &failsafe_flag, 1},
	{"pciboot", no_argument, &pciboot_flag, 1},
	{"nandstage2", no_argument, &stage2_flag, 1},
	{"spistage2", no_argument, &stage2_flag, 1},
	{"norstage2", no_argument, &stage2_flag, 1},
	{"stage2", no_argument, &stage2_flag, 1},
	{"stage1.5", no_argument, &stage_1_5_flag, 1},
	{"stage1", no_argument, &stage_1_flag, 1},
	{"environment", no_argument, &env_flag, 1},
	/*
	 * These options don't set a flag.
	 * We distinguish them by their indices.
	 */
	{"board", required_argument, 0, 0},
	{"text_base", required_argument, 0, 0},
	{0, 0, 0, 0}
};

static int lookup_board_type(char *board_name)
{
	int i;
	int board_type = 0;
	char *substr = NULL;

	/* Detect stage 2 bootloader boards */
	if (strcasestr(board_name, "_stage2")) {
		printf("Stage 2 bootloader detected from substring %s in name %s\n",
		       "_stage2", board_name);
		stage2_flag = 1;
	} else {
		printf("Stage 2 bootloader NOT detected from name \"%s\"\n",
		       board_name);
	}

	if (strcasestr(board_name, "_stage1")) {
		printf("Stage 1 bootloader detected from substring %s in name %s\n",
		       "_stage1", board_name);
		stage_1_flag = 1;
	}

	/* Generic is a special case since there are numerous sub-types */
	if (!strncasecmp("generic", board_name, strlen("generic")))
		return CVMX_BOARD_TYPE_GENERIC;

	/*
	 * If we're an eMMC stage 2 bootloader, cut off the _emmc_stage2
	 * part of the name.
	 */
	substr = strcasestr(board_name, "_emmc_stage2");
	if (substr && (substr[strlen("_emmc_stage2")] == '\0')) {
		/*return CVMX_BOARD_TYPE_GENERIC;*/

		printf("  Converting board name %s to ", board_name);
		*substr = '\0';
		printf("%s\n", board_name);
	}

	/*
	 * If we're a NAND stage 2 bootloader, cut off the _nand_stage2
	 * part of the name.
	 */
	substr = strcasestr(board_name, "_nand_stage2");
	if (substr && (substr[strlen("_nand_stage2")] == '\0')) {
		/*return CVMX_BOARD_TYPE_GENERIC;*/

		printf("  Converting board name %s to ", board_name);
		*substr = '\0';
		printf("%s\n", board_name);
	}

	/*
	 * If we're a SPI stage 2 bootloader, cut off the _spi_stage2
	 * part of the name.
	 */
	substr = strcasestr(board_name, "_spi_stage2");
	if (substr && (substr[strlen("_spi_stage2")] == '\0')) {
		printf("  Converting board name %s to ", board_name);
		*substr = '\0';
		printf("%s\n", board_name);
	}

	for (i = CVMX_BOARD_TYPE_NULL; i < CVMX_BOARD_TYPE_MAX; i++)
		if (!strcasecmp(cvmx_board_type_to_string(i), board_name))
			board_type = i;

	for (i = CVMX_BOARD_TYPE_CUST_DEFINED_MIN;
	     i < CVMX_BOARD_TYPE_CUST_DEFINED_MAX; i++)
		if (!strncasecmp(cvmx_board_type_to_string(i), board_name,
				 strlen(cvmx_board_type_to_string(i))))
			board_type = i;

	for (i = CVMX_BOARD_TYPE_CUST_PRIVATE_MIN;
	     i < CVMX_BOARD_TYPE_CUST_PRIVATE_MAX; i++)
		if (!strncasecmp(cvmx_board_type_to_string(i), board_name,
				 strlen(cvmx_board_type_to_string(i))))
			board_type = i;

	return board_type;
}

static void usage(void)
{
	printf("Usage: update_octeon_header <filename> <board_name> [--failsafe] [--text_base=0xXXXXX]\n");
}

int main(int argc, char *argv[])
{
	int fd;
	uint8_t buf[BUF_SIZE];
	uint32_t data_crc = 0;
	int len;
	int data_len = 0;
	struct bootloader_header header;
	char filename[NAME_LEN];
	int i;
	int option_index = 0;	/* getopt_long stores the option index here. */
	char board_name[NAME_LEN] = { 0 };
	char tmp_board_name[NAME_LEN] = { 0 };
	int c;
	int board_type = 0;
	unsigned long long address = 0;
	ssize_t ret;
	const char *type_str = NULL;
	int hdr_size = sizeof(struct bootloader_header);

	/*
	 * Compile time check, if the size of the bootloader_header structure
	 * has changed.
	 */
	compiletime_assert(sizeof(struct bootloader_header) == 192,
			   "Octeon bootloader header size changed (!= 192)!");

	/* Bail out, if argument count is incorrect */
	if (argc < 3) {
		usage();
		return -1;
	}

	debug("header size is: %d bytes\n", hdr_size);

	/* Parse command line options using getopt_long */
	while (1) {
		c = getopt_long(argc, argv, "h", long_options, &option_index);

		/* Detect the end of the options. */
		if (c == -1)
			break;

		switch (c) {
			/* All long options handled in case 0 */
		case 0:
			/* If this option set a flag, do nothing else now. */
			if (long_options[option_index].flag != 0)
				break;
			debug("option(l) %s", long_options[option_index].name);

			if (!optarg) {
				usage();
				return -1;
			}
			debug(" with arg %s\n", optarg);

			if (!strcmp(long_options[option_index].name, "board")) {
				if (strlen(optarg) >= NAME_LEN) {
					printf("strncpy() issue detected!");
					exit(-1);
				}
				strncpy(board_name, optarg, NAME_LEN);

				printf("Using user supplied board name: %s\n",
				       board_name);
			} else if (!strcmp(long_options[option_index].name,
					   "text_base")) {
				address = strtoull(optarg, NULL, 0);
				printf("Address of image is: 0x%llx\n",
				       (unsigned long long)address);
				if (!(address & 0xFFFFFFFFULL << 32)) {
					if (address & 1 << 31) {
						address |= 0xFFFFFFFFULL << 32;
						printf("Converting address to 64 bit compatibility space: 0x%llx\n",
						       address);
					}
				}
			}
			break;

		case 'h':
		case '?':
			/* getopt_long already printed an error message. */
			usage();
			return -1;

		default:
			abort();
		}
	}

	if (optind < argc) {
		/*
		 * We only support one argument - an optional bootloader
		 * file name
		 */
		if (argc - optind > 2) {
			fprintf(stderr, "non-option ARGV-elements: ");
			while (optind < argc)
				fprintf(stderr, "%s ", argv[optind++]);
			fprintf(stderr, "\n");

			usage();
			return -1;
		}
	}

	if (strlen(argv[optind]) >= NAME_LEN) {
		fprintf(stderr, "strncpy() issue detected!");
		exit(-1);
	}
	strncpy(filename, argv[optind], NAME_LEN);

	if (board_name[0] == '\0') {
		if (strlen(argv[optind + 1]) >= NAME_LEN) {
			fprintf(stderr, "strncpy() issue detected!");
			exit(-1);
		}
		strncpy(board_name, argv[optind + 1], NAME_LEN);
	}

	if (strlen(board_name) >= NAME_LEN) {
		fprintf(stderr, "strncpy() issue detected!");
		exit(-1);
	}
	strncpy(tmp_board_name, board_name, NAME_LEN);

	fd = open(filename, O_RDWR);
	if (fd < 0) {
		fprintf(stderr, "Unable to open file: %s\n", filename);
		exit(-1);
	}

	if (failsafe_flag)
		printf("Setting failsafe flag\n");

	if (strlen(board_name)) {
		int offset = 0;

		printf("Supplied board name of: %s\n", board_name);

		if (strstr(board_name, "failsafe")) {
			failsafe_flag = 1;
			printf("Setting failsafe flag based on board name\n");
		}
		/* Skip leading octeon_ if present. */
		if (!strncmp(board_name, "octeon_", 7))
			offset = 7;

		/*
		 * Check to see if 'failsafe' is in the name.  If so, set the
		 * failsafe flag.  Also, ignore extra trailing characters on
		 * passed parameter when comparing against board names.
		 * We actually use the configuration name from u-boot, so it
		 * may have some other variant names.  Variants other than
		 * failsafe _must_ be passed to this program explicitly
		 */

		board_type = lookup_board_type(board_name + offset);
		if (!board_type) {
			/* Retry with 'cust_' prefix to catch boards that are
			 * in the customer section (such as nb5)
			 */
			sprintf(tmp_board_name, "cust_%s", board_name + offset);
			board_type = lookup_board_type(tmp_board_name);
		}

		/* reset to original value */
		strncpy(tmp_board_name, board_name, NAME_LEN);
		if (!board_type) {
			/*
			 * Retry with 'cust_private_' prefix to catch boards
			 * that are in the customer private section
			 */
			sprintf(tmp_board_name, "cust_private_%s",
				board_name + offset);
			board_type = lookup_board_type(tmp_board_name);
		}

		if (!board_type) {
			fprintf(stderr,
				"ERROR: unable to determine board type\n");
			exit(-1);
		}
		printf("Board type is: %d: %s\n", board_type,
		       cvmx_board_type_to_string(board_type));
	} else {
		fprintf(stderr, "Board name must be specified!\n");
		exit(-1);
	}

	/*
	 * Check to see if there is either an existing header, or that there
	 * are zero valued bytes where we want to put the header
	 */
	len = read(fd, buf, BUF_SIZE);
	if (len > 0) {
		/*
		 * Copy the header, as the first word (jump instruction, needs
		 * to remain the same.
		 */
		memcpy(&header, buf, hdr_size);
		/*
		 * Check to see if we have zero bytes (excluding first 4, which
		 * are the jump instruction)
		 */
		for (i = 1; i < hdr_size / 4; i++) {
			if (((uint32_t *)buf)[i]) {
				fprintf(stderr,
					"ERROR: non-zero word found %x in location %d required for header, aborting\n",
				       ((uint32_t *)buf)[i], i);
				exit(-1);
			}
		}
		printf("Zero bytes found in header location, adding header.\n");

	} else {
		fprintf(stderr, "Unable to read from file %s\n", filename);
		exit(-1);
	}

	/* Read data bytes and generate CRC */
	lseek(fd, hdr_size, SEEK_SET);

	while ((len = read(fd, buf, BUF_SIZE)) > 0) {
		data_crc = crc32(data_crc, buf, len);
		data_len += len;
	}
	printf("CRC of data: 0x%x, length: %d\n", data_crc, data_len);

	/* Now create the new header */
	header.magic = htonl(BOOTLOADER_HEADER_MAGIC);
	header.maj_rev = htons(BOOTLOADER_HEADER_CURRENT_MAJOR_REV);
	header.min_rev = htons(BOOTLOADER_HEADER_CURRENT_MINOR_REV);
	header.dlen = htonl(data_len);
	header.dcrc = htonl(data_crc);
	header.board_type = htons(board_type);
	header.address = address;
	if (failsafe_flag)
		header.flags |= htonl(BL_HEADER_FLAG_FAILSAFE);

	printf("Stage 2 flag is %sset\n", stage2_flag ? "" : "not ");
	printf("Stage 1 flag is %sset\n", stage_1_flag ? "" : "not ");
	if (pciboot_flag)
		header.image_type = htons(BL_HEADER_IMAGE_PCIBOOT);
	else if (stage2_flag)
		header.image_type = htons(BL_HEADER_IMAGE_STAGE2);
	else if (stage_1_flag)
		header.image_type = htons(BL_HEADER_IMAGE_STAGE1);
	else if (env_flag)
		header.image_type = htons(BL_HEADER_IMAGE_UBOOT_ENV);
	else if (stage_1_5_flag || stage_1_flag)
		header.image_type = htons(BL_HEADER_IMAGE_PRE_UBOOT);
	else
		header.image_type = htons(BL_HEADER_IMAGE_NOR);

	switch (ntohs(header.image_type)) {
	case BL_HEADER_IMAGE_UNKNOWN:
		type_str = "Unknown";
		break;
	case BL_HEADER_IMAGE_STAGE1:
		type_str = "Stage 1";
		break;
	case BL_HEADER_IMAGE_STAGE2:
		type_str = "Stage 2";
		break;
	case BL_HEADER_IMAGE_PRE_UBOOT:
		type_str = "Pre-U-Boot";
		break;
	case BL_HEADER_IMAGE_STAGE3:
		type_str = "Stage 3";
		break;
	case BL_HEADER_IMAGE_NOR:
		type_str = "NOR";
		break;
	case BL_HEADER_IMAGE_PCIBOOT:
		type_str = "PCI Boot";
		break;
	case BL_HEADER_IMAGE_UBOOT_ENV:
		type_str = "U-Boot Environment";
		break;
	default:
		if (ntohs(header.image_type) >= BL_HEADER_IMAGE_CUST_RESERVED_MIN &&
		    ntohs(header.image_type) <= BL_HEADER_IMAGE_CUST_RESERVED_MAX)
			type_str = "Customer Reserved";
		else
			type_str = "Unsupported";
	}
	printf("Header image type: %s\n", type_str);
	header.hlen = htons(hdr_size);

	/* Now compute header CRC over all of the header excluding the CRC */
	header.hcrc = crc32(0, (void *)&header, 12);
	header.hcrc = htonl(crc32(header.hcrc, ((void *)&(header)) + 16,
				  hdr_size - 16));

	/* Seek to beginning of file */
	lseek(fd, 0, SEEK_SET);

	/* Write header to file */
	ret = write(fd, &header, hdr_size);
	if (ret < 0)
		perror("write");

	close(fd);

	printf("Header CRC: 0x%x\n", ntohl(header.hcrc));
	return 0;
}
