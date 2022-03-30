// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2012-2014 Freescale Semiconductor, Inc.
 */
#include "imagetool.h"
#include <image.h>
#include "pblimage.h"
#include "pbl_crc32.h"

#define roundup(x, y)		((((x) + ((y) - 1)) / (y)) * (y))
#define PBL_ACS_CONT_CMD	0x81000000
#define PBL_ADDR_24BIT_MASK	0x00ffffff

/*
 * Initialize to an invalid value.
 */
static uint32_t next_pbl_cmd = 0x82000000;
/*
 * need to store all bytes in memory for calculating crc32, then write the
 * bytes to image file for PBL boot.
 */
static unsigned char mem_buf[1000000];
static unsigned char *pmem_buf = mem_buf;
static int pbl_size;
static char *fname = "Unknown";
static int lineno = -1;
static struct pbl_header pblimage_header;
static int uboot_size;
static int arch_flag;

static uint32_t pbl_cmd_initaddr;
static uint32_t pbi_crc_cmd1;
static uint32_t pbi_crc_cmd2;
static uint32_t pbl_end_cmd[4];

static union
{
	char c[4];
	unsigned char l;
} endian_test = { {'l', '?', '?', 'b'} };

#define ENDIANNESS ((char)endian_test.l)

/*
 * The PBL can load up to 64 bytes at a time, so we split the U-Boot
 * image into 64 byte chunks. PBL needs a command for each piece, of
 * the form "81xxxxxx", where "xxxxxx" is the offset. Calculate the
 * start offset by subtracting the size of the u-boot image from the
 * top of the allowable 24-bit range.
 */
static void generate_pbl_cmd(void)
{
	uint32_t val = next_pbl_cmd;
	next_pbl_cmd += 0x40;
	int i;

	for (i = 3; i >= 0; i--) {
		*pmem_buf++ = (val >> (i * 8)) & 0xff;
		pbl_size++;
	}
}

static void pbl_fget(size_t size, FILE *stream)
{
	unsigned char c = 0xff;
	int c_temp;

	while (size) {
		c_temp = fgetc(stream);
		if (c_temp != EOF)
			c = (unsigned char)c_temp;
		else if ((c_temp == EOF) && (arch_flag == IH_ARCH_ARM))
			c = 0xff;
		*pmem_buf++ = c;
		pbl_size++;
		size--;
	}
}

/* load split u-boot with PBI command 81xxxxxx. */
static void load_uboot(FILE *fp_uboot)
{
	next_pbl_cmd = pbl_cmd_initaddr - uboot_size;
	while (next_pbl_cmd < pbl_cmd_initaddr) {
		generate_pbl_cmd();
		pbl_fget(64, fp_uboot);
	}
}

static void check_get_hexval(char *token)
{
	uint32_t hexval;
	int i;

	if (!sscanf(token, "%x", &hexval)) {
		printf("Error:%s[%d] - Invalid hex data(%s)\n", fname,
			lineno, token);
		exit(EXIT_FAILURE);
	}
	for (i = 3; i >= 0; i--) {
		*pmem_buf++ = (hexval >> (i * 8)) & 0xff;
		pbl_size++;
	}
}

static void pbl_parser(char *name)
{
	FILE *fd = NULL;
	char *line = NULL;
	char *token, *saveptr1, *saveptr2;
	size_t len = 0;

	fname = name;
	fd = fopen(name, "r");
	if (fd == NULL) {
		printf("Error:%s - Can't open\n", fname);
		exit(EXIT_FAILURE);
	}

	while ((getline(&line, &len, fd)) > 0) {
		lineno++;
		token = strtok_r(line, "\r\n", &saveptr1);
		/* drop all lines with zero tokens (= empty lines) */
		if (token == NULL)
			continue;
		for (line = token;; line = NULL) {
			token = strtok_r(line, " \t", &saveptr2);
			if (token == NULL)
				break;
			/* Drop all text starting with '#' as comments */
			if (token[0] == '#')
				break;
			check_get_hexval(token);
		}
	}
	if (line)
		free(line);
	fclose(fd);
}

static uint32_t reverse_byte(uint32_t val)
{
	uint32_t temp;
	unsigned char *p1;
	int j;

	temp = val;
	p1 = (unsigned char *)&temp;
	for (j = 3; j >= 0; j--)
		*p1++ = (val >> (j * 8)) & 0xff;
	return temp;
}

/* write end command and crc command to memory. */
static void add_end_cmd(void)
{
	uint32_t crc32_pbl;
	int i;
	unsigned char *p = (unsigned char *)&pbl_end_cmd;

	if (ENDIANNESS == 'l') {
		for (i = 0; i < 4; i++)
			pbl_end_cmd[i] = reverse_byte(pbl_end_cmd[i]);
	}

	for (i = 0; i < 16; i++) {
		*pmem_buf++ = *p++;
		pbl_size++;
	}

	/* Add PBI CRC command. */
	*pmem_buf++ = 0x08;
	*pmem_buf++ = pbi_crc_cmd1;
	*pmem_buf++ = pbi_crc_cmd2;
	*pmem_buf++ = 0x40;
	pbl_size += 4;

	/* calculated CRC32 and write it to memory. */
	crc32_pbl = pbl_crc32(0, (const char *)mem_buf, pbl_size);
	*pmem_buf++ = (crc32_pbl >> 24) & 0xff;
	*pmem_buf++ = (crc32_pbl >> 16) & 0xff;
	*pmem_buf++ = (crc32_pbl >> 8) & 0xff;
	*pmem_buf++ = (crc32_pbl) & 0xff;
	pbl_size += 4;
}

void pbl_load_uboot(int ifd, struct image_tool_params *params)
{
	FILE *fp_uboot;
	int size;

	/* parse the rcw.cfg file. */
	pbl_parser(params->imagename);

	/* parse the pbi.cfg file. */
	if (params->imagename2[0] != '\0')
		pbl_parser(params->imagename2);

	if (params->datafile) {
		fp_uboot = fopen(params->datafile, "r");
		if (fp_uboot == NULL) {
			printf("Error: %s open failed\n", params->datafile);
			exit(EXIT_FAILURE);
		}

		load_uboot(fp_uboot);
		fclose(fp_uboot);
	}
	add_end_cmd();
	lseek(ifd, 0, SEEK_SET);

	size = pbl_size;
	if (write(ifd, (const void *)&mem_buf, size) != size) {
		fprintf(stderr, "Write error on %s: %s\n",
			params->imagefile, strerror(errno));
		exit(EXIT_FAILURE);
	}
}

static int pblimage_check_image_types(uint8_t type)
{
	if (type == IH_TYPE_PBLIMAGE)
		return EXIT_SUCCESS;
	else
		return EXIT_FAILURE;
}

static int pblimage_verify_header(unsigned char *ptr, int image_size,
			struct image_tool_params *params)
{
	struct pbl_header *pbl_hdr = (struct pbl_header *) ptr;

	/* Only a few checks can be done: search for magic numbers */
	if (ENDIANNESS == 'l') {
		if (pbl_hdr->preamble != reverse_byte(RCW_PREAMBLE))
			return -FDT_ERR_BADSTRUCTURE;

		if (pbl_hdr->rcwheader != reverse_byte(RCW_HEADER))
			return -FDT_ERR_BADSTRUCTURE;
	} else {
		if (pbl_hdr->preamble != RCW_PREAMBLE)
			return -FDT_ERR_BADSTRUCTURE;

		if (pbl_hdr->rcwheader != RCW_HEADER)
			return -FDT_ERR_BADSTRUCTURE;
	}
	return 0;
}

static void pblimage_print_header(const void *ptr)
{
	printf("Image Type:   Freescale PBL Boot Image\n");
}

static void pblimage_set_header(void *ptr, struct stat *sbuf, int ifd,
				struct image_tool_params *params)
{
	/*nothing need to do, pbl_load_uboot takes care of whole file. */
}

int pblimage_check_params(struct image_tool_params *params)
{
	FILE *fp_uboot;
	int fd;
	struct stat st;

	if (!params)
		return EXIT_FAILURE;

	if (params->datafile) {
		fp_uboot = fopen(params->datafile, "r");
		if (fp_uboot == NULL) {
			printf("Error: %s open failed\n", params->datafile);
			exit(EXIT_FAILURE);
		}
		fd = fileno(fp_uboot);

		if (fstat(fd, &st) == -1) {
			printf("Error: Could not determine u-boot image size. %s\n",
			       strerror(errno));
			exit(EXIT_FAILURE);
		}

		/* For the variable size, pad it to 64 byte boundary */
		uboot_size = roundup(st.st_size, 64);
		fclose(fp_uboot);
	}

	if (params->arch == IH_ARCH_ARM) {
		arch_flag = IH_ARCH_ARM;
		pbi_crc_cmd1 = 0x61;
		pbi_crc_cmd2 = 0;
		pbl_cmd_initaddr = params->addr & PBL_ADDR_24BIT_MASK;
		pbl_cmd_initaddr |= PBL_ACS_CONT_CMD;
		pbl_cmd_initaddr += uboot_size;
		pbl_end_cmd[0] = 0x09610000;
		pbl_end_cmd[1] = 0x00000000;
		pbl_end_cmd[2] = 0x096100c0;
		pbl_end_cmd[3] = 0x00000000;
	} else if (params->arch == IH_ARCH_PPC) {
		arch_flag = IH_ARCH_PPC;
		pbi_crc_cmd1 = 0x13;
		pbi_crc_cmd2 = 0x80;
		pbl_cmd_initaddr = 0x82000000;
		pbl_end_cmd[0] = 0x091380c0;
		pbl_end_cmd[1] = 0x00000000;
		pbl_end_cmd[2] = 0x091380c0;
		pbl_end_cmd[3] = 0x00000000;
	}

	next_pbl_cmd = pbl_cmd_initaddr;
	return 0;
};

/* pblimage parameters */
U_BOOT_IMAGE_TYPE(
	pblimage,
	"Freescale PBL Boot Image support",
	sizeof(struct pbl_header),
	(void *)&pblimage_header,
	pblimage_check_params,
	pblimage_verify_header,
	pblimage_print_header,
	pblimage_set_header,
	NULL,
	pblimage_check_image_types,
	NULL,
	NULL
);
