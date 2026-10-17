// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2023 Linus Walleij <linus.walleij@linaro.org>
 * Support for the "SEAttle iMAge" SEAMA NAND image format
 */

#include <common.h>
#include <command.h>
#include <nand.h>

/*
 * All SEAMA data is stored in the flash in "network endianness"
 * i.e. big endian, which means that it needs to be byte-swapped
 * on all little endian platforms.
 *
 * structure for a SEAMA entity in NAND flash:
 *
 * 32 bit SEAMA magic 0x5EA3A417
 * 16 bit reserved
 * 16 bit metadata size (following the header)
 * 32 bit image size
 * 16 bytes MD5 digest of the image
 * meta data
 * ... image data ...
 *
 * Then if a new SEAMA magic follows, that is the next image.
 */

#define SEAMA_MAGIC		0x5EA3A417
#define SEAMA_HDR_NO_META_SZ	28
#define SEAMA_MAX_META_SZ	(1024 - SEAMA_HDR_NO_META_SZ)

struct seama_header {
	u32 magic;
	u32 meta_size;
	u32 image_size;
	u8 md5[16];
	u8 metadata[SEAMA_MAX_META_SZ];
};

static struct seama_header shdr;

static int env_set_val(const char *varname, ulong val)
{
	int ret;

	ret = env_set_hex(varname, val);
	if (ret)
		printf("Failed to %s env var\n", varname);

	return ret;
}

static int do_seama_load_image(struct cmd_tbl *cmdtp, int flag, int argc,
			       char *const argv[])
{
	struct mtd_info *mtd;
	uintptr_t load_addr;
	unsigned long image_index;
	u32 len;
	size_t readsz;
	int ret;
	u32 *start;
	u32 *offset;
	u32 *end;
	u32 tmp;

	if (argc < 2 || argc > 3)
		return CMD_RET_USAGE;

	load_addr = hextoul(argv[1], NULL);
	if (!load_addr) {
		printf("Invalid load address\n");
		return CMD_RET_USAGE;
	}

	/* Can be 0 for first image */
	image_index = hextoul(argv[2], NULL);

	/* We only support one NAND, the first one */
	nand_curr_device = 0;
	mtd = get_nand_dev_by_index(0);
	if (!mtd) {
		printf("NAND Device 0 not available\n");
		return CMD_RET_FAILURE;
	}

#ifdef CONFIG_SYS_NAND_SELECT_DEVICE
	board_nand_select_device(mtd_to_nand(mtd), 0);
#endif

	printf("Loading SEAMA image %lu from %s\n", image_index, mtd->name);

	readsz = sizeof(shdr);
	offset = 0;
	ret = nand_read_skip_bad(mtd, 0, &readsz, NULL, mtd->size,
				 (u_char *)&shdr);
	if (ret) {
		printf("Read error reading SEAMA header\n");
		return CMD_RET_FAILURE;
	}

	if (shdr.magic != SEAMA_MAGIC) {
		printf("Invalid SEAMA image magic: 0x%08x\n", shdr.magic);
		return CMD_RET_FAILURE;
	}

	/* Only the lower 16 bits are valid */
	shdr.meta_size &= 0xFFFF;

	if (env_set_val("seama_image_size", 0))
		return CMD_RET_FAILURE;

	printf("SEMA IMAGE:\n");
	printf("  metadata size %d\n", shdr.meta_size);
	printf("  image size %d\n", shdr.image_size);
	printf("  checksum %02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x\n",
	       shdr.md5[0], shdr.md5[1], shdr.md5[2], shdr.md5[3],
	       shdr.md5[4], shdr.md5[5], shdr.md5[6], shdr.md5[7],
	       shdr.md5[8], shdr.md5[9], shdr.md5[10], shdr.md5[11],
	       shdr.md5[12], shdr.md5[13], shdr.md5[14], shdr.md5[15]);

	/* TODO: handle metadata if needed */

	len = shdr.image_size;
	if (env_set_val("seama_image_size", len))
		return CMD_RET_FAILURE;

	/* We need to include the header (read full pages) */
	readsz = shdr.image_size + SEAMA_HDR_NO_META_SZ + shdr.meta_size;
	ret = nand_read_skip_bad(mtd, 0, &readsz, NULL, mtd->size,
				 (u_char *)load_addr);
	if (ret) {
		printf("Read error reading SEAMA main image\n");
		return CMD_RET_FAILURE;
	}

	/* We use a temporary variable tmp to avoid to hairy casts */
	start = (u32 *)load_addr;
	tmp = (u32)start;
	tmp += SEAMA_HDR_NO_META_SZ + shdr.meta_size;
	offset = (u32 *)tmp;
	tmp += shdr.image_size;
	end = (u32 *)tmp;

	printf("Decoding SEAMA image 0x%08x..0x%08x to 0x%08x\n",
	       (u32)offset, (u32)end, (u32)start);
	for (; start < end; start++, offset++)
		*start = be32_to_cpu(*offset);

	return CMD_RET_SUCCESS;
}

U_BOOT_CMD
	(seama, 3, 1, do_seama_load_image,
	 "Load the SEAMA image and sets envs",
	 "seama <addr> <imageindex>\n"
);
