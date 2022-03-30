// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2015
 * Linus Walleij, Linaro
 *
 * Support for ARM Flash Partitions
 */
#include <common.h>
#include <command.h>
#include <console.h>
#include <asm/io.h>

#define MAX_REGIONS 4
#define MAX_IMAGES 32

struct afs_region {
	u32 load_address;
	u32 size;
	u32 offset;
};

struct afs_image {
	flash_info_t *flinfo;
	const char *name;
	u32 version;
	u32 entrypoint;
	u32 attributes;
	u32 region_count;
	struct afs_region regions[MAX_REGIONS];
	ulong flash_mem_start;
	ulong flash_mem_end;
};

static struct afs_image afs_images[MAX_IMAGES];
static int num_afs_images;

static u32 compute_crc(ulong start, u32 len)
{
	u32 sum = 0;
	int i;

	if (len % 4 != 0) {
		printf("bad checksumming\n");
		return 0;
	}

	for (i = 0; i < len; i += 4) {
		u32 val;

		val = readl((void *)start + i);
		if (val > ~sum)
			sum++;
		sum += val;
	}
	return ~sum;
}

static void parse_bank(ulong bank)
{
	int i;
	ulong flstart, flend;
	flash_info_t *info;

	info = &flash_info[bank];
	if (info->flash_id != FLASH_MAN_CFI) {
		printf("Bank %lu: missing or unknown FLASH type\n", bank);
		return;
	}
	if (!info->sector_count) {
		printf("Bank %lu: no FLASH sectors\n", bank);
		return;
	}

	flstart = info->start[0];
	flend = flstart + info->size;

	for (i = 0; i < info->sector_count; ++i) {
		ulong secend;
		u32 foot1, foot2;

		if (ctrlc())
			break;

		if (i == info->sector_count-1)
			secend = flend;
		else
			secend = info->start[i+1];

		/* Check for v1 header */
		foot1 = readl((void *)secend - 0x0c);
		if (foot1 == 0xA0FFFF9FU) {
			struct afs_image *afi = &afs_images[num_afs_images];
			ulong imginfo;

			afi->flinfo = info;
			afi->version = 1;
			afi->flash_mem_start = readl((void *)secend - 0x10);
			afi->flash_mem_end = readl((void *)secend - 0x14);
			afi->attributes = readl((void *)secend - 0x08);
			/* Adjust to even address */
			imginfo = afi->flash_mem_end + afi->flash_mem_end % 4;
			/* Record as a single region */
			afi->region_count = 1;
			afi->regions[0].offset = readl((void *)imginfo + 0x04);
			afi->regions[0].load_address =
				readl((void *)imginfo + 0x08);
			afi->regions[0].size = readl((void *)imginfo + 0x0C);
			afi->entrypoint = readl((void *)imginfo + 0x10);
			afi->name = (const char *)imginfo + 0x14;
			num_afs_images++;
		}

		/* Check for v2 header */
		foot1 = readl((void *)secend - 0x04);
		foot2 = readl((void *)secend - 0x08);
		/* This makes up the string "HSLFTOOF" flash footer */
		if (foot1 == 0x464F4F54U && foot2 == 0x464C5348U) {
			struct afs_image *afi = &afs_images[num_afs_images];
			ulong imginfo;
			u32 block_start, block_end;
			int j;

			afi->flinfo = info;
			afi->version = readl((void *)secend - 0x0c);
			imginfo = secend - 0x30 - readl((void *)secend - 0x10);
			afi->name = (const char *)secend - 0x30;

			afi->entrypoint = readl((void *)imginfo+0x08);
			afi->attributes = readl((void *)imginfo+0x0c);
			afi->region_count = readl((void *)imginfo+0x10);
			block_start = readl((void *)imginfo+0x54);
			block_end = readl((void *)imginfo+0x58);
			afi->flash_mem_start = afi->flinfo->start[block_start];
			afi->flash_mem_end = afi->flinfo->start[block_end];

			/*
			 * Check footer CRC, the algorithm saves the inverse
			 * checksum as part of the summed words, and thus
			 * the result should be zero.
			 */
			if (compute_crc(imginfo + 8, 0x88) != 0) {
				printf("BAD CRC on ARM image info\n");
				printf("(continuing anyway)\n");
			}

			/* Parse regions */
			for (j = 0; j < afi->region_count; j++) {
				afi->regions[j].load_address =
					readl((void *)imginfo+0x14 + j*0x10);
				afi->regions[j].size =
					readl((void *)imginfo+0x18 + j*0x10);
				afi->regions[j].offset =
					readl((void *)imginfo+0x1c + j*0x10);
				/*
				 * At offset 0x20 + j*0x10 there is a region
				 * checksum which seems to be the running
				 * sum + 3, however since we anyway checksum
				 * the entire footer this is skipped over for
				 * checking here.
				 */
			}
			num_afs_images++;
		}
	}
}

static void parse_flash(void)
{
	ulong bank;

	/* We have already parsed the images in flash */
	if (num_afs_images > 0)
		return;
	for (bank = 0; bank < CONFIG_SYS_MAX_FLASH_BANKS; ++bank)
		parse_bank(bank);
}

static int load_image(const char * const name, const ulong address)
{
	struct afs_image *afi = NULL;
	int i;

	parse_flash();
	for (i = 0; i < num_afs_images; i++) {
		struct afs_image *tmp = &afs_images[i];

		if (!strcmp(tmp->name, name)) {
			afi = tmp;
			break;
		}
	}
	if (!afi) {
		printf("image \"%s\" not found in flash\n", name);
		return CMD_RET_FAILURE;
	}

	for (i = 0; i < afi->region_count; i++) {
		ulong from, to;

		from = afi->flash_mem_start + afi->regions[i].offset;
		if (address) {
			to = address;
		} else if (afi->regions[i].load_address) {
			to = afi->regions[i].load_address;
		} else {
			printf("no valid load address\n");
			return CMD_RET_FAILURE;
		}

		memcpy((void *)to, (void *)from, afi->regions[i].size);

		printf("loaded region %d from %08lX to %08lX, %08X bytes\n",
		       i,
		       from,
		       to,
		       afi->regions[i].size);
	}
	return CMD_RET_SUCCESS;
}

static void print_images(void)
{
	int i;

	parse_flash();
	for (i = 0; i < num_afs_images; i++) {
		struct afs_image *afi = &afs_images[i];
		int j;

		printf("Image: \"%s\" (v%d):\n", afi->name, afi->version);
		printf("    Entry point: 0x%08X\n", afi->entrypoint);
		printf("    Attributes: 0x%08X: ", afi->attributes);
		if (afi->attributes == 0x01)
			printf("ARM executable");
		if (afi->attributes == 0x08)
			printf("ARM backup");
		printf("\n");
		printf("    Flash mem start: 0x%08lX\n",
		       afi->flash_mem_start);
		printf("    Flash mem end: 0x%08lX\n",
		       afi->flash_mem_end);
		for (j = 0; j < afi->region_count; j++) {
			printf("    region %d\n"
			       "        load address: %08X\n"
			       "        size: %08X\n"
			       "        offset: %08X\n",
			       j,
			       afi->regions[j].load_address,
			       afi->regions[j].size,
			       afi->regions[j].offset);
		}
	}
}

static int exists(const char * const name)
{
	int i;

	parse_flash();
	for (i = 0; i < num_afs_images; i++) {
		struct afs_image *afi = &afs_images[i];

		if (strcmp(afi->name, name) == 0)
			return CMD_RET_SUCCESS;
	}
	return CMD_RET_FAILURE;
}

static int do_afs(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	int ret = CMD_RET_SUCCESS;

	if (argc == 1) {
		print_images();
	} else if (argc == 3 && !strcmp(argv[1], "exists")) {
		ret = exists(argv[2]);
	} else if (argc == 3 && !strcmp(argv[1], "load")) {
		ret = load_image(argv[2], 0x0);
	} else if (argc == 4 && !strcmp(argv[1], "load")) {
		ulong load_addr;

		load_addr = simple_strtoul(argv[3], NULL, 16);
		ret = load_image(argv[2], load_addr);
	} else {
		return CMD_RET_USAGE;
	}

	return ret;
}

U_BOOT_CMD(afs, 4, 0, do_afs, "show AFS partitions",
	   "no arguments\n"
	   "    - list images in flash\n"
	   "exists <image>\n"
	   "    - returns 1 if an image exists, else 0\n"
	   "load <image>\n"
	   "    - load an image to the location indicated in the header\n"
	   "load <image> 0x<address>\n"
	   "    - load an image to the location specified\n");
