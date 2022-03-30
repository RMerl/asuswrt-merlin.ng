// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2007-2011
 * Allwinner Technology Co., Ltd. <www.allwinnertech.com>
 * Tom Cubie <tangliang@allwinnertech.com>
 *
 * a simple tool to generate bootable image for sunxi platform.
 */
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "../arch/arm/include/asm/arch-sunxi/spl.h"

#define STAMP_VALUE                     0x5F0A6C39

/* check sum functon from sun4i boot code */
int gen_check_sum(struct boot_file_head *head_p)
{
	uint32_t length;
	uint32_t *buf;
	uint32_t loop;
	uint32_t i;
	uint32_t sum;

	length = le32_to_cpu(head_p->length);
	if ((length & 0x3) != 0)	/* must 4-byte-aligned */
		return -1;
	buf = (uint32_t *)head_p;
	head_p->check_sum = cpu_to_le32(STAMP_VALUE);	/* fill stamp */
	loop = length >> 2;

	/* calculate the sum */
	for (i = 0, sum = 0; i < loop; i++)
		sum += le32_to_cpu(buf[i]);

	/* write back check sum */
	head_p->check_sum = cpu_to_le32(sum);

	return 0;
}

#define ALIGN(x, a) __ALIGN_MASK((x), (typeof(x))(a)-1)
#define __ALIGN_MASK(x, mask) (((x)+(mask))&~(mask))

#define SUNXI_SRAM_SIZE 0x8000	/* SoC with smaller size are limited before */
#define SRAM_LOAD_MAX_SIZE (SUNXI_SRAM_SIZE - sizeof(struct boot_file_head))

/*
 * BROM (at least on A10 and A20) requires NAND-images to be explicitly aligned
 * to a multiple of 8K, and rejects the image otherwise. MMC-images are fine
 * with 512B blocks. To cater for both, align to the largest of the two.
 */
#define BLOCK_SIZE 0x2000

struct boot_img {
	struct boot_file_head header;
	char code[SRAM_LOAD_MAX_SIZE];
	char pad[BLOCK_SIZE];
};

int main(int argc, char *argv[])
{
	int fd_in, fd_out;
	struct boot_img img;
	unsigned file_size;
	int count;
	char *tool_name = argv[0];
	char *default_dt = NULL;

	/* a sanity check */
	if ((sizeof(img.header) % 32) != 0) {
		fprintf(stderr, "ERROR: the SPL header must be a multiple ");
		fprintf(stderr, "of 32 bytes.\n");
		return EXIT_FAILURE;
	}

	/* process optional command line switches */
	while (argc >= 2 && argv[1][0] == '-') {
		if (strcmp(argv[1], "--default-dt") == 0) {
			if (argc >= 3) {
				default_dt = argv[2];
				argv += 2;
				argc -= 2;
				continue;
			}
			fprintf(stderr, "ERROR: no --default-dt arg\n");
			return EXIT_FAILURE;
		} else {
			fprintf(stderr, "ERROR: bad option '%s'\n", argv[1]);
			return EXIT_FAILURE;
		}
	}

	if (argc < 3) {
		printf("This program converts an input binary file to a sunxi bootable image.\n");
		printf("\nUsage: %s [options] input_file output_file\n",
		       tool_name);
		printf("Where [options] may be:\n");
		printf("  --default-dt arg         - 'arg' is the default device tree name\n");
		printf("                             (CONFIG_DEFAULT_DEVICE_TREE).\n");
		return EXIT_FAILURE;
	}

	fd_in = open(argv[1], O_RDONLY);
	if (fd_in < 0) {
		perror("Open input file");
		return EXIT_FAILURE;
	}

	memset(&img, 0, sizeof(img));

	/* get input file size */
	file_size = lseek(fd_in, 0, SEEK_END);

	if (file_size > SRAM_LOAD_MAX_SIZE) {
		fprintf(stderr, "ERROR: File too large!\n");
		return EXIT_FAILURE;
	}

	fd_out = open(argv[2], O_WRONLY | O_CREAT, 0666);
	if (fd_out < 0) {
		perror("Open output file");
		return EXIT_FAILURE;
	}

	/* read file to buffer to calculate checksum */
	lseek(fd_in, 0, SEEK_SET);
	count = read(fd_in, img.code, file_size);
	if (count != file_size) {
		perror("Reading input image");
		return EXIT_FAILURE;
	}

	/* fill the header */
	img.header.b_instruction =	/* b instruction */
		0xEA000000 |	/* jump to the first instr after the header */
		((sizeof(struct boot_file_head) / sizeof(int) - 2)
		 & 0x00FFFFFF);
	memcpy(img.header.magic, BOOT0_MAGIC, 8);	/* no '0' termination */
	img.header.length =
		ALIGN(file_size + sizeof(struct boot_file_head), BLOCK_SIZE);
	img.header.b_instruction = cpu_to_le32(img.header.b_instruction);
	img.header.length = cpu_to_le32(img.header.length);

	memcpy(img.header.spl_signature, SPL_SIGNATURE, 3); /* "sunxi" marker */
	img.header.spl_signature[3] = SPL_HEADER_VERSION;

	if (default_dt) {
		if (strlen(default_dt) + 1 <= sizeof(img.header.string_pool)) {
			strcpy((char *)img.header.string_pool, default_dt);
			img.header.dt_name_offset =
				cpu_to_le32(offsetof(struct boot_file_head,
						     string_pool));
		} else {
			printf("WARNING: The SPL header is too small\n");
			printf("         and has no space to store the dt name.\n");
		}
	}

	gen_check_sum(&img.header);

	count = write(fd_out, &img, le32_to_cpu(img.header.length));
	if (count != le32_to_cpu(img.header.length)) {
		perror("Writing output");
		return EXIT_FAILURE;
	}

	close(fd_in);
	close(fd_out);

	return EXIT_SUCCESS;
}
