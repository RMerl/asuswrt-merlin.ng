// SPDX-License-Identifier: GPL-2.0+
/*
 * Freescale i.MX28 image generator
 *
 * Copyright (C) 2011 Marek Vasut <marek.vasut@gmail.com>
 * on behalf of DENX Software Engineering GmbH
 */

#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "compiler.h"

/* Taken from <linux/kernel.h> */
#define __round_mask(x, y) ((__typeof__(x))((y)-1))
#define round_down(x, y) ((x) & ~__round_mask(x, y))

/*
 * Default BCB layout.
 *
 * TWEAK this if you have blown any OCOTP fuses.
 */
#define	STRIDE_PAGES		64
#define	STRIDE_COUNT		4

/*
 * Layout for 256Mb big NAND with 2048b page size, 64b OOB size and
 * 128kb erase size.
 *
 * TWEAK this if you have different kind of NAND chip.
 */
static uint32_t nand_writesize = 2048;
static uint32_t nand_oobsize = 64;
static uint32_t nand_erasesize = 128 * 1024;

/*
 * Sector on which the SigmaTel boot partition (0x53) starts.
 */
static uint32_t sd_sector = 2048;

/*
 * Each of the U-Boot bootstreams is at maximum 1MB big.
 *
 * TWEAK this if, for some wild reason, you need to boot bigger image.
 */
#define	MAX_BOOTSTREAM_SIZE	(1 * 1024 * 1024)

/* i.MX28 NAND controller-specific constants. DO NOT TWEAK! */
#define	MXS_NAND_DMA_DESCRIPTOR_COUNT		4
#define	MXS_NAND_CHUNK_DATA_CHUNK_SIZE		512
#define	MXS_NAND_METADATA_SIZE			10
#define	MXS_NAND_BITS_PER_ECC_LEVEL		13
#define	MXS_NAND_COMMAND_BUFFER_SIZE		32

struct mx28_nand_fcb {
	uint32_t		checksum;
	uint32_t		fingerprint;
	uint32_t		version;
	struct {
		uint8_t			data_setup;
		uint8_t			data_hold;
		uint8_t			address_setup;
		uint8_t			dsample_time;
		uint8_t			nand_timing_state;
		uint8_t			rea;
		uint8_t			rloh;
		uint8_t			rhoh;
	}			timing;
	uint32_t		page_data_size;
	uint32_t		total_page_size;
	uint32_t		sectors_per_block;
	uint32_t		number_of_nands;		/* Ignored */
	uint32_t		total_internal_die;		/* Ignored */
	uint32_t		cell_type;			/* Ignored */
	uint32_t		ecc_block_n_ecc_type;
	uint32_t		ecc_block_0_size;
	uint32_t		ecc_block_n_size;
	uint32_t		ecc_block_0_ecc_type;
	uint32_t		metadata_bytes;
	uint32_t		num_ecc_blocks_per_page;
	uint32_t		ecc_block_n_ecc_level_sdk;	/* Ignored */
	uint32_t		ecc_block_0_size_sdk;		/* Ignored */
	uint32_t		ecc_block_n_size_sdk;		/* Ignored */
	uint32_t		ecc_block_0_ecc_level_sdk;	/* Ignored */
	uint32_t		num_ecc_blocks_per_page_sdk;	/* Ignored */
	uint32_t		metadata_bytes_sdk;		/* Ignored */
	uint32_t		erase_threshold;
	uint32_t		boot_patch;
	uint32_t		patch_sectors;
	uint32_t		firmware1_starting_sector;
	uint32_t		firmware2_starting_sector;
	uint32_t		sectors_in_firmware1;
	uint32_t		sectors_in_firmware2;
	uint32_t		dbbt_search_area_start_address;
	uint32_t		badblock_marker_byte;
	uint32_t		badblock_marker_start_bit;
	uint32_t		bb_marker_physical_offset;
};

struct mx28_nand_dbbt {
	uint32_t		checksum;
	uint32_t		fingerprint;
	uint32_t		version;
	uint32_t		number_bb;
	uint32_t		number_2k_pages_bb;
};

struct mx28_nand_bbt {
	uint32_t		nand;
	uint32_t		number_bb;
	uint32_t		badblock[510];
};

struct mx28_sd_drive_info {
	uint32_t		chip_num;
	uint32_t		drive_type;
	uint32_t		tag;
	uint32_t		first_sector_number;
	uint32_t		sector_count;
};

struct mx28_sd_config_block {
	uint32_t			signature;
	uint32_t			primary_boot_tag;
	uint32_t			secondary_boot_tag;
	uint32_t			num_copies;
	struct mx28_sd_drive_info	drv_info[1];
};

static inline uint32_t mx28_nand_ecc_chunk_cnt(uint32_t page_data_size)
{
	return page_data_size / MXS_NAND_CHUNK_DATA_CHUNK_SIZE;
}

static inline uint32_t mx28_nand_ecc_size_in_bits(uint32_t ecc_strength)
{
	return ecc_strength * MXS_NAND_BITS_PER_ECC_LEVEL;
}

static inline uint32_t mx28_nand_get_ecc_strength(uint32_t page_data_size,
						uint32_t page_oob_size)
{
	int ecc_strength;

	/*
	 * Determine the ECC layout with the formula:
	 *	ECC bits per chunk = (total page spare data bits) /
	 *		(bits per ECC level) / (chunks per page)
	 * where:
	 *	total page spare data bits =
	 *		(page oob size - meta data size) * (bits per byte)
	 */
	ecc_strength = ((page_oob_size - MXS_NAND_METADATA_SIZE) * 8)
			/ (MXS_NAND_BITS_PER_ECC_LEVEL *
				mx28_nand_ecc_chunk_cnt(page_data_size));

	return round_down(ecc_strength, 2);
}

static inline uint32_t mx28_nand_get_mark_offset(uint32_t page_data_size,
						uint32_t ecc_strength)
{
	uint32_t chunk_data_size_in_bits;
	uint32_t chunk_ecc_size_in_bits;
	uint32_t chunk_total_size_in_bits;
	uint32_t block_mark_chunk_number;
	uint32_t block_mark_chunk_bit_offset;
	uint32_t block_mark_bit_offset;

	chunk_data_size_in_bits = MXS_NAND_CHUNK_DATA_CHUNK_SIZE * 8;
	chunk_ecc_size_in_bits  = mx28_nand_ecc_size_in_bits(ecc_strength);

	chunk_total_size_in_bits =
			chunk_data_size_in_bits + chunk_ecc_size_in_bits;

	/* Compute the bit offset of the block mark within the physical page. */
	block_mark_bit_offset = page_data_size * 8;

	/* Subtract the metadata bits. */
	block_mark_bit_offset -= MXS_NAND_METADATA_SIZE * 8;

	/*
	 * Compute the chunk number (starting at zero) in which the block mark
	 * appears.
	 */
	block_mark_chunk_number =
			block_mark_bit_offset / chunk_total_size_in_bits;

	/*
	 * Compute the bit offset of the block mark within its chunk, and
	 * validate it.
	 */
	block_mark_chunk_bit_offset = block_mark_bit_offset -
			(block_mark_chunk_number * chunk_total_size_in_bits);

	if (block_mark_chunk_bit_offset > chunk_data_size_in_bits)
		return 1;

	/*
	 * Now that we know the chunk number in which the block mark appears,
	 * we can subtract all the ECC bits that appear before it.
	 */
	block_mark_bit_offset -=
		block_mark_chunk_number * chunk_ecc_size_in_bits;

	return block_mark_bit_offset;
}

static inline uint32_t mx28_nand_mark_byte_offset(void)
{
	uint32_t ecc_strength;
	ecc_strength = mx28_nand_get_ecc_strength(nand_writesize, nand_oobsize);
	return mx28_nand_get_mark_offset(nand_writesize, ecc_strength) >> 3;
}

static inline uint32_t mx28_nand_mark_bit_offset(void)
{
	uint32_t ecc_strength;
	ecc_strength = mx28_nand_get_ecc_strength(nand_writesize, nand_oobsize);
	return mx28_nand_get_mark_offset(nand_writesize, ecc_strength) & 0x7;
}

static uint32_t mx28_nand_block_csum(uint8_t *block, uint32_t size)
{
	uint32_t csum = 0;
	int i;

	for (i = 0; i < size; i++)
		csum += block[i];

	return csum ^ 0xffffffff;
}

static struct mx28_nand_fcb *mx28_nand_get_fcb(uint32_t size)
{
	struct mx28_nand_fcb *fcb;
	uint32_t bcb_size_bytes;
	uint32_t stride_size_bytes;
	uint32_t bootstream_size_pages;
	uint32_t fw1_start_page;
	uint32_t fw2_start_page;

	fcb = malloc(nand_writesize);
	if (!fcb) {
		printf("MX28 NAND: Unable to allocate FCB\n");
		return NULL;
	}

	memset(fcb, 0, nand_writesize);

	fcb->fingerprint =			0x20424346;
	fcb->version =				0x01000000;

	/*
	 * FIXME: These here are default values as found in kobs-ng. We should
	 * probably retrieve the data from NAND or something.
	 */
	fcb->timing.data_setup =		80;
	fcb->timing.data_hold =			60;
	fcb->timing.address_setup =		25;
	fcb->timing.dsample_time =		6;

	fcb->page_data_size =		nand_writesize;
	fcb->total_page_size =		nand_writesize + nand_oobsize;
	fcb->sectors_per_block =	nand_erasesize / nand_writesize;

	fcb->num_ecc_blocks_per_page =	(nand_writesize / 512) - 1;
	fcb->ecc_block_0_size =		512;
	fcb->ecc_block_n_size =		512;
	fcb->metadata_bytes =		10;
	fcb->ecc_block_n_ecc_type = mx28_nand_get_ecc_strength(
					nand_writesize, nand_oobsize) >> 1;
	fcb->ecc_block_0_ecc_type = mx28_nand_get_ecc_strength(
					nand_writesize, nand_oobsize) >> 1;
	if (fcb->ecc_block_n_ecc_type == 0) {
		printf("MX28 NAND: Unsupported NAND geometry\n");
		goto err;
	}

	fcb->boot_patch =			0;
	fcb->patch_sectors =			0;

	fcb->badblock_marker_byte =	mx28_nand_mark_byte_offset();
	fcb->badblock_marker_start_bit = mx28_nand_mark_bit_offset();
	fcb->bb_marker_physical_offset = nand_writesize;

	stride_size_bytes = STRIDE_PAGES * nand_writesize;
	bcb_size_bytes = stride_size_bytes * STRIDE_COUNT;

	bootstream_size_pages = (size + (nand_writesize - 1)) /
					nand_writesize;

	fw1_start_page = 2 * bcb_size_bytes / nand_writesize;
	fw2_start_page = (2 * bcb_size_bytes + MAX_BOOTSTREAM_SIZE) /
				nand_writesize;

	fcb->firmware1_starting_sector =	fw1_start_page;
	fcb->firmware2_starting_sector =	fw2_start_page;
	fcb->sectors_in_firmware1 =		bootstream_size_pages;
	fcb->sectors_in_firmware2 =		bootstream_size_pages;

	fcb->dbbt_search_area_start_address =	STRIDE_PAGES * STRIDE_COUNT;

	return fcb;

err:
	free(fcb);
	return NULL;
}

static struct mx28_nand_dbbt *mx28_nand_get_dbbt(void)
{
	struct mx28_nand_dbbt *dbbt;

	dbbt = malloc(nand_writesize);
	if (!dbbt) {
		printf("MX28 NAND: Unable to allocate DBBT\n");
		return NULL;
	}

	memset(dbbt, 0, nand_writesize);

	dbbt->fingerprint	= 0x54424244;
	dbbt->version		= 0x1;

	return dbbt;
}

static inline uint8_t mx28_nand_parity_13_8(const uint8_t b)
{
	uint32_t parity = 0, tmp;

	tmp = ((b >> 6) ^ (b >> 5) ^ (b >> 3) ^ (b >> 2)) & 1;
	parity |= tmp << 0;

	tmp = ((b >> 7) ^ (b >> 5) ^ (b >> 4) ^ (b >> 2) ^ (b >> 1)) & 1;
	parity |= tmp << 1;

	tmp = ((b >> 7) ^ (b >> 6) ^ (b >> 5) ^ (b >> 1) ^ (b >> 0)) & 1;
	parity |= tmp << 2;

	tmp = ((b >> 7) ^ (b >> 4) ^ (b >> 3) ^ (b >> 0)) & 1;
	parity |= tmp << 3;

	tmp = ((b >> 6) ^ (b >> 4) ^ (b >> 3) ^
		(b >> 2) ^ (b >> 1) ^ (b >> 0)) & 1;
	parity |= tmp << 4;

	return parity;
}

static uint8_t *mx28_nand_fcb_block(struct mx28_nand_fcb *fcb)
{
	uint8_t *block;
	uint8_t *ecc;
	int i;

	block = malloc(nand_writesize + nand_oobsize);
	if (!block) {
		printf("MX28 NAND: Unable to allocate FCB block\n");
		return NULL;
	}

	memset(block, 0, nand_writesize + nand_oobsize);

	/* Update the FCB checksum */
	fcb->checksum = mx28_nand_block_csum(((uint8_t *)fcb) + 4, 508);

	/* Figure 12-11. in iMX28RM, rev. 1, says FCB is at offset 12 */
	memcpy(block + 12, fcb, sizeof(struct mx28_nand_fcb));

	/* ECC is at offset 12 + 512 */
	ecc = block + 12 + 512;

	/* Compute the ECC parity */
	for (i = 0; i < sizeof(struct mx28_nand_fcb); i++)
		ecc[i] = mx28_nand_parity_13_8(block[i + 12]);

	return block;
}

static int mx28_nand_write_fcb(struct mx28_nand_fcb *fcb, uint8_t *buf)
{
	uint32_t offset;
	uint8_t *fcbblock;
	int ret = 0;
	int i;

	fcbblock = mx28_nand_fcb_block(fcb);
	if (!fcbblock)
		return -1;

	for (i = 0; i < STRIDE_PAGES * STRIDE_COUNT; i += STRIDE_PAGES) {
		offset = i * nand_writesize;
		memcpy(buf + offset, fcbblock, nand_writesize + nand_oobsize);
		/* Mark the NAND page is OK. */
		buf[offset + nand_writesize] = 0xff;
	}

	free(fcbblock);
	return ret;
}

static int mx28_nand_write_dbbt(struct mx28_nand_dbbt *dbbt, uint8_t *buf)
{
	uint32_t offset;
	int i = STRIDE_PAGES * STRIDE_COUNT;

	for (; i < 2 * STRIDE_PAGES * STRIDE_COUNT; i += STRIDE_PAGES) {
		offset = i * nand_writesize;
		memcpy(buf + offset, dbbt, sizeof(struct mx28_nand_dbbt));
	}

	return 0;
}

static int mx28_nand_write_firmware(struct mx28_nand_fcb *fcb, int infd,
				    uint8_t *buf)
{
	int ret;
	off_t size;
	uint32_t offset1, offset2;

	size = lseek(infd, 0, SEEK_END);
	lseek(infd, 0, SEEK_SET);

	offset1 = fcb->firmware1_starting_sector * nand_writesize;
	offset2 = fcb->firmware2_starting_sector * nand_writesize;

	ret = read(infd, buf + offset1, size);
	if (ret != size)
		return -1;

	memcpy(buf + offset2, buf + offset1, size);

	return 0;
}

static void usage(void)
{
	printf(
		"Usage: mxsboot [ops] <type> <infile> <outfile>\n"
		"Augment BootStream file with a proper header for i.MX28 boot\n"
		"\n"
		"  <type>	type of image:\n"
		"                 \"nand\" for NAND image\n"
		"                 \"sd\" for SD image\n"
		"  <infile>     input file, the u-boot.sb bootstream\n"
		"  <outfile>    output file, the bootable image\n"
		"\n");
	printf(
		"For NAND boot, these options are accepted:\n"
		"  -w <size>    NAND page size\n"
		"  -o <size>    NAND OOB size\n"
		"  -e <size>    NAND erase size\n"
		"\n"
		"For SD boot, these options are accepted:\n"
		"  -p <sector>  Sector where the SGTL partition starts\n"
	);
}

static int mx28_create_nand_image(int infd, int outfd)
{
	struct mx28_nand_fcb *fcb;
	struct mx28_nand_dbbt *dbbt;
	int ret = -1;
	uint8_t *buf;
	int size;
	ssize_t wr_size;

	size = nand_writesize * 512 + 2 * MAX_BOOTSTREAM_SIZE;

	buf = malloc(size);
	if (!buf) {
		printf("Can not allocate output buffer of %d bytes\n", size);
		goto err0;
	}

	memset(buf, 0, size);

	fcb = mx28_nand_get_fcb(MAX_BOOTSTREAM_SIZE);
	if (!fcb) {
		printf("Unable to compile FCB\n");
		goto err1;
	}

	dbbt = mx28_nand_get_dbbt();
	if (!dbbt) {
		printf("Unable to compile DBBT\n");
		goto err2;
	}

	ret = mx28_nand_write_fcb(fcb, buf);
	if (ret) {
		printf("Unable to write FCB to buffer\n");
		goto err3;
	}

	ret = mx28_nand_write_dbbt(dbbt, buf);
	if (ret) {
		printf("Unable to write DBBT to buffer\n");
		goto err3;
	}

	ret = mx28_nand_write_firmware(fcb, infd, buf);
	if (ret) {
		printf("Unable to write firmware to buffer\n");
		goto err3;
	}

	wr_size = write(outfd, buf, size);
	if (wr_size != size) {
		ret = -1;
		goto err3;
	}

	ret = 0;

err3:
	free(dbbt);
err2:
	free(fcb);
err1:
	free(buf);
err0:
	return ret;
}

static int mx28_create_sd_image(int infd, int outfd)
{
	int ret = -1;
	uint32_t *buf;
	int size;
	off_t fsize;
	ssize_t wr_size;
	struct mx28_sd_config_block *cb;

	fsize = lseek(infd, 0, SEEK_END);
	lseek(infd, 0, SEEK_SET);
	size = fsize + 4 * 512;

	buf = malloc(size);
	if (!buf) {
		printf("Can not allocate output buffer of %d bytes\n", size);
		goto err0;
	}

	ret = read(infd, (uint8_t *)buf + 4 * 512, fsize);
	if (ret != fsize) {
		ret = -1;
		goto err1;
	}

	cb = (struct mx28_sd_config_block *)buf;

	cb->signature = cpu_to_le32(0x00112233);
	cb->primary_boot_tag = cpu_to_le32(0x1);
	cb->secondary_boot_tag = cpu_to_le32(0x1);
	cb->num_copies = cpu_to_le32(1);
	cb->drv_info[0].chip_num = cpu_to_le32(0x0);
	cb->drv_info[0].drive_type = cpu_to_le32(0x0);
	cb->drv_info[0].tag = cpu_to_le32(0x1);
	cb->drv_info[0].first_sector_number = cpu_to_le32(sd_sector + 4);
	cb->drv_info[0].sector_count = cpu_to_le32((size - 4) / 512);

	wr_size = write(outfd, buf, size);
	if (wr_size != size) {
		ret = -1;
		goto err1;
	}

	ret = 0;

err1:
	free(buf);
err0:
	return ret;
}

static int parse_ops(int argc, char **argv)
{
	int i;
	int tmp;
	char *end;
	enum param {
		PARAM_WRITE,
		PARAM_OOB,
		PARAM_ERASE,
		PARAM_PART,
		PARAM_SD,
		PARAM_NAND
	};
	int type;

	if (argc < 4)
		return -1;

	for (i = 1; i < argc; i++) {
		if (!strncmp(argv[i], "-w", 2))
			type = PARAM_WRITE;
		else if (!strncmp(argv[i], "-o", 2))
			type = PARAM_OOB;
		else if (!strncmp(argv[i], "-e", 2))
			type = PARAM_ERASE;
		else if (!strncmp(argv[i], "-p", 2))
			type = PARAM_PART;
		else	/* SD/MMC */
			break;

		tmp = strtol(argv[++i], &end, 10);
		if (tmp % 2)
			return -1;
		if (tmp <= 0)
			return -1;

		if (type == PARAM_WRITE)
			nand_writesize = tmp;
		if (type == PARAM_OOB)
			nand_oobsize = tmp;
		if (type == PARAM_ERASE)
			nand_erasesize = tmp;
		if (type == PARAM_PART)
			sd_sector = tmp;
	}

	if (strcmp(argv[i], "sd") && strcmp(argv[i], "nand"))
		return -1;

	if (i + 3 != argc)
		return -1;

	return i;
}

int main(int argc, char **argv)
{
	int infd, outfd;
	int ret = 0;
	int offset;

	offset = parse_ops(argc, argv);
	if (offset < 0) {
		usage();
		ret = 1;
		goto err1;
	}

	infd = open(argv[offset + 1], O_RDONLY);
	if (infd < 0) {
		printf("Input BootStream file can not be opened\n");
		ret = 2;
		goto err1;
	}

	outfd = open(argv[offset + 2], O_CREAT | O_TRUNC | O_WRONLY,
					S_IRUSR | S_IWUSR);
	if (outfd < 0) {
		printf("Output file can not be created\n");
		ret = 3;
		goto err2;
	}

	if (!strcmp(argv[offset], "sd"))
		ret = mx28_create_sd_image(infd, outfd);
	else if (!strcmp(argv[offset], "nand"))
		ret = mx28_create_nand_image(infd, outfd);

	close(outfd);
err2:
	close(infd);
err1:
	return ret;
}
