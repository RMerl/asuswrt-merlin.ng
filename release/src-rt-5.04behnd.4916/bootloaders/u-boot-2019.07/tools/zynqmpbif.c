/*
 * Copyright (C) 2018 Alexander Graf <agraf@suse.de>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include "imagetool.h"
#include "mkimage.h"
#include "zynqmpimage.h"
#include <elf.h>
#include <image.h>

struct bif_entry {
	const char *filename;
	uint64_t flags;
	uint64_t dest_cpu;
	uint64_t exp_lvl;
	uint64_t dest_dev;
	uint64_t load;
	uint64_t entry;
	size_t offset;
};

enum bif_flag {
	BIF_FLAG_AESKEYFILE,
	BIF_FLAG_INIT,
	BIF_FLAG_UDF_BH,
	BIF_FLAG_HEADERSIGNATURE,
	BIF_FLAG_PPKFILE,
	BIF_FLAG_PSKFILE,
	BIF_FLAG_SPKFILE,
	BIF_FLAG_SSKFILE,
	BIF_FLAG_SPKSIGNATURE,
	BIF_FLAG_FSBL_CONFIG,
	BIF_FLAG_AUTH_PARAMS,
	BIF_FLAG_KEYSRC_ENCRYPTION,
	BIF_FLAG_PMUFW_IMAGE,
	BIF_FLAG_BOOTLOADER,
	BIF_FLAG_TZ,
	BIF_FLAG_BH_KEY_IV,
	BIF_FLAG_BH_KEYFILE,
	BIF_FLAG_PUF_FILE,
	BIF_FLAG_AARCH32,
	BIF_FLAG_PART_OWNER_UBOOT,

	/* Internal flags */
	BIF_FLAG_BIT_FILE,
	BIF_FLAG_ELF_FILE,
	BIF_FLAG_BIN_FILE,
};

struct bif_flags {
	const char name[32];
	uint64_t flag;
	char *(*parse)(char *line, struct bif_entry *bf);
};

struct bif_file_type {
	const char name[32];
	uint32_t header;
	int (*add)(struct bif_entry *bf);
};

struct bif_output {
	size_t data_len;
	char *data;
	struct image_header_table *imgheader;
	struct zynqmp_header *header;
	struct partition_header *last_part;
};

struct bif_output bif_output;

static uint32_t zynqmp_csum(void *start, void *end)
{
	uint32_t checksum = 0;
	uint32_t *ptr32 = start;

	while (ptr32 != end) {
		checksum += le32_to_cpu(*ptr32);
		ptr32++;
	}

	return ~checksum;
}

static int zynqmpbif_check_params(struct image_tool_params *params)
{
	if (!params)
		return 0;

	if (params->addr != 0x0) {
		fprintf(stderr, "Error: Load Address can not be specified.\n");
		return -1;
	}

	if (params->eflag) {
		fprintf(stderr, "Error: Entry Point can not be specified.\n");
		return -1;
	}

	return !(params->lflag || params->dflag);
}

static int zynqmpbif_check_image_types(uint8_t type)
{
	return (type == IH_TYPE_ZYNQMPBIF) ? EXIT_SUCCESS : EXIT_FAILURE;
}

static char *parse_dest_cpu(char *line, struct bif_entry *bf)
{
	uint64_t i;

	for (i = 0; i < ARRAY_SIZE(dest_cpus); i++) {
		if (!strncmp(line, dest_cpus[i], strlen(dest_cpus[i]))) {
			bf->dest_cpu = i << PART_ATTR_DEST_CPU_SHIFT;
			return line + strlen(dest_cpus[i]);
		}

		/* a5x can also be written as a53 */
		if (!strncmp(dest_cpus[i], "a5x", 3)) {
			char a53[] = "a53-X";

			a53[4] = dest_cpus[i][4];
			if (!strncmp(line, a53, strlen(a53))) {
				bf->dest_cpu = i << PART_ATTR_DEST_CPU_SHIFT;
				return line + strlen(a53);
			}
		}
	}

	return line;
}

static char *parse_el(char *line, struct bif_entry *bf)
{
	const char *dest_els[] = { "none", "el-0", "el-1", "el-2", "el-3" };
	int i;

	for (i = 0; i < ARRAY_SIZE(dest_els); i++) {
		if (!strncmp(line, dest_els[i], strlen(dest_els[i]))) {
			bf->exp_lvl = i;
			return line + strlen(dest_els[i]);
		}
	}

	return line;
}

static char *parse_load(char *line, struct bif_entry *bf)
{
	char *endptr;

	bf->load = strtoll(line, &endptr, 0);

	return endptr;
}

static char *parse_entry(char *line, struct bif_entry *bf)
{
	char *endptr;

	bf->entry = strtoll(line, &endptr, 0);

	return endptr;
}

static char *parse_offset(char *line, struct bif_entry *bf)
{
	char *endptr;

	bf->offset = strtoll(line, &endptr, 0);

	return endptr;
}

static char *parse_partition_owner(char *line, struct bif_entry *bf)
{
	char *endptr = NULL;

	if (!strncmp(line, "fsbl", 4)) {
		endptr = line + 4;
	} else if (!strncmp(line, "uboot", 5)) {
		bf->flags |= 1ULL << BIF_FLAG_PART_OWNER_UBOOT;
		endptr = line + 5;
	} else {
		printf("ERROR: Unknown partition type '%s'\n", line);
	}

	return endptr;
}

static const struct bif_flags bif_flags[] = {
	{ "fsbl_config", BIF_FLAG_FSBL_CONFIG },
	{ "trustzone", BIF_FLAG_TZ },
	{ "pmufw_image", BIF_FLAG_PMUFW_IMAGE },
	{ "bootloader", BIF_FLAG_BOOTLOADER },
	{ "destination_cpu=", 0, parse_dest_cpu },
	{ "exception_level=", 0, parse_el },
	{ "load=", 0, parse_load },
	{ "startup=", 0, parse_entry },
	{ "offset=", 0, parse_offset },
	{ "partition_owner=", 0, parse_partition_owner },
};

static char *read_full_file(const char *filename, size_t *size)
{
	char *buf, *bufp;
	struct stat sbuf;
	int len = 0, r, fd;

	fd = open(filename, O_RDONLY);
	if (fd < 0)
		return NULL;

	if (fstat(fd, &sbuf) < 0)
		return NULL;

	if (size)
		*size = sbuf.st_size;

	buf = malloc(sbuf.st_size);
	if (!buf)
		return NULL;

	bufp = buf;
	while (len < sbuf.st_size) {
		r = read(fd, bufp, sbuf.st_size - len);
		if (r < 0)
			return NULL;
		len += r;
		bufp += r;
	}

	close(fd);

	return buf;
}

static int bif_add_blob(const void *data, size_t len, size_t *offset)
{
	size_t new_size;
	uintptr_t header_off;
	uintptr_t last_part_off;
	uintptr_t imgheader_off;
	uintptr_t old_data = (uintptr_t)bif_output.data;
	void *new_data;

	header_off = (uintptr_t)bif_output.header - old_data;
	last_part_off = (uintptr_t)bif_output.last_part - old_data;
	imgheader_off = (uintptr_t)bif_output.imgheader - old_data;

	if (offset && *offset) {
		/* Pad to a given offset */
		if (bif_output.data_len > *offset) {
			printf("Can not pad to offset %zx\n", *offset);
			return -1;
		}

		bif_output.data_len = *offset;
	}

	new_size = ROUND(bif_output.data_len + len, 64);
	new_data = realloc(bif_output.data, new_size);
	memcpy(new_data + bif_output.data_len, data, len);
	if (offset)
		*offset = bif_output.data_len;
	bif_output.data = new_data;
	bif_output.data_len = new_size;

	/* Readjust internal pointers */
	if (bif_output.header)
		bif_output.header = new_data + header_off;
	if (bif_output.last_part)
		bif_output.last_part = new_data + last_part_off;
	if (bif_output.imgheader)
		bif_output.imgheader = new_data + imgheader_off;

	return 0;
}

static int bif_init(void)
{
	struct zynqmp_header header = { { 0 } };
	int r;

	zynqmpimage_default_header(&header);

	r = bif_add_blob(&header, sizeof(header), NULL);
	if (r)
		return r;

	bif_output.header = (void *)bif_output.data;

	return 0;
}

static int bif_add_pmufw(struct bif_entry *bf, const char *data, size_t len)
{
	int r;

	if (bif_output.header->image_offset) {
		printf("PMUFW expected before bootloader in your .bif file!\n");
		return -1;
	}

	r = bif_add_blob(data, len, &bf->offset);
	if (r)
		return r;

	len = ROUND(len, 64);
	bif_output.header->pfw_image_length = cpu_to_le32(len);
	bif_output.header->total_pfw_image_length = cpu_to_le32(len);
	bif_output.header->image_offset = cpu_to_le32(bf->offset);

	return 0;
}

static int bif_add_part(struct bif_entry *bf, const char *data, size_t len)
{
	size_t parthdr_offset = 0;
	size_t len_padded = ROUND(len, 4);

	struct partition_header parthdr = {
		.len_enc = cpu_to_le32(len_padded / 4),
		.len_unenc = cpu_to_le32(len_padded / 4),
		.len = cpu_to_le32(len_padded / 4),
		.entry_point = cpu_to_le64(bf->entry),
		.load_address = cpu_to_le64(bf->load),
	};
	int r;
	uint32_t csum;

	if (len < len_padded) {
		char *newdata = malloc(len_padded);
		memcpy(newdata, data, len);
		memset(newdata + len, 0, len_padded - len);
		data = newdata;
	}

	if (bf->flags & (1ULL << BIF_FLAG_PMUFW_IMAGE))
		return bif_add_pmufw(bf, data, len);

	r = bif_add_blob(data, len, &bf->offset);
	if (r)
		return r;

	parthdr.offset = cpu_to_le32(bf->offset / 4);

	if (bf->flags & (1ULL << BIF_FLAG_BOOTLOADER)) {
		if (bif_output.last_part) {
			printf("ERROR: Bootloader expected before others\n");
			return -1;
		}

		parthdr.offset = cpu_to_le32(bif_output.header->image_offset);
		parthdr.len = cpu_to_le32((bf->offset + len -
			bif_output.header->image_offset) / 4);
		parthdr.len_enc = parthdr.len;
		parthdr.len_unenc = parthdr.len;
	}

	/* Normalize EL */
	bf->exp_lvl = bf->exp_lvl ? bf->exp_lvl - 1 : 3;
	parthdr.attributes |= bf->exp_lvl << PART_ATTR_TARGET_EL_SHIFT;
	parthdr.attributes |= bf->dest_dev;
	parthdr.attributes |= bf->dest_cpu;
	if (bf->flags & (1ULL << BIF_FLAG_TZ))
		parthdr.attributes |= PART_ATTR_TZ_SECURE;
	if (bf->flags & (1ULL << BIF_FLAG_PART_OWNER_UBOOT))
		parthdr.attributes |= PART_ATTR_PART_OWNER_UBOOT;
	switch (bf->dest_cpu) {
	case PART_ATTR_DEST_CPU_NONE:
	case PART_ATTR_DEST_CPU_A53_0:
	case PART_ATTR_DEST_CPU_A53_1:
	case PART_ATTR_DEST_CPU_A53_2:
	case PART_ATTR_DEST_CPU_A53_3:
		if (bf->flags & (1ULL << BIF_FLAG_AARCH32))
			parthdr.attributes |= PART_ATTR_A53_EXEC_AARCH32;
	}

	csum = zynqmp_csum(&parthdr, &parthdr.checksum);
	parthdr.checksum = cpu_to_le32(csum);

	r = bif_add_blob(&parthdr, sizeof(parthdr), &parthdr_offset);
	if (r)
		return r;

	/* Add image header table if not there yet */
	if (!bif_output.imgheader) {
		size_t imghdr_off = 0;
		struct image_header_table imghdr = {
			.version = cpu_to_le32(0x01020000),
			.nr_parts = 0,
		};

		r = bif_add_blob(&imghdr, sizeof(imghdr), &imghdr_off);
		if (r)
			return r;

		bif_output.header->image_header_table_offset = imghdr_off;
		bif_output.imgheader = (void *)(bif_output.data + imghdr_off);
	}

	bif_output.imgheader->nr_parts = cpu_to_le32(le32_to_cpu(
		bif_output.imgheader->nr_parts) + 1);

	/* Link to this partition header */
	if (bif_output.last_part) {
		bif_output.last_part->next_partition_offset =
			cpu_to_le32(parthdr_offset / 4);

		/* Recalc checksum of last_part */
		csum = zynqmp_csum(bif_output.last_part,
				   &bif_output.last_part->checksum);
		bif_output.last_part->checksum = cpu_to_le32(csum);
	} else {
		bif_output.imgheader->partition_header_offset =
			cpu_to_le32(parthdr_offset / 4);
	}
	bif_output.last_part = (void *)(bif_output.data + parthdr_offset);

	if (bf->flags & (1ULL << BIF_FLAG_BOOTLOADER)) {
		bif_output.header->image_load = cpu_to_le32(bf->load);
		if (!bif_output.header->image_offset)
			bif_output.header->image_offset =
				cpu_to_le32(bf->offset);
		bif_output.header->image_size = cpu_to_le32(len_padded);
		bif_output.header->image_stored_size = cpu_to_le32(len_padded);

		bif_output.header->image_attributes &= ~HEADER_CPU_SELECT_MASK;
		switch (bf->dest_cpu) {
		default:
		case PART_ATTR_DEST_CPU_A53_0:
			if (bf->flags & BIF_FLAG_AARCH32)
				bif_output.header->image_attributes |=
					HEADER_CPU_SELECT_A53_32BIT;
			else
				bif_output.header->image_attributes |=
					HEADER_CPU_SELECT_A53_64BIT;
			break;
		case PART_ATTR_DEST_CPU_R5_0:
			bif_output.header->image_attributes |=
				HEADER_CPU_SELECT_R5_SINGLE;
			break;
		case PART_ATTR_DEST_CPU_R5_L:
			bif_output.header->image_attributes |=
				HEADER_CPU_SELECT_R5_DUAL;
			break;
		}
	}

	return 0;
}

/* Add .bit bitstream */
static int bif_add_bit(struct bif_entry *bf)
{
	char *bit = read_full_file(bf->filename, NULL);
	char *bitbin;
	uint8_t initial_header[] = { 0x00, 0x09, 0x0f, 0xf0, 0x0f, 0xf0, 0x0f,
				     0xf0, 0x0f, 0xf0, 0x00, 0x00, 0x01, 0x61 };
	uint16_t len;
	uint32_t bitlen;
	int i;

	if (!bit)
		return -1;

	/* Skip initial header */
	if (memcmp(bit, initial_header, sizeof(initial_header)))
		return -1;

	bit += sizeof(initial_header);

	/* Design name */
	len = be16_to_cpu(*(uint16_t *)bit);
	bit += sizeof(uint16_t);
	debug("Design: %s\n", bit);
	bit += len;

	/* Device identifier */
	if (*bit != 'b')
		return -1;
	bit++;
	len = be16_to_cpu(*(uint16_t *)bit);
	bit += sizeof(uint16_t);
	debug("Device: %s\n", bit);
	bit += len;

	/* Date */
	if (*bit != 'c')
		return -1;
	bit++;
	len = be16_to_cpu(*(uint16_t *)bit);
	bit += sizeof(uint16_t);
	debug("Date: %s\n", bit);
	bit += len;

	/* Time */
	if (*bit != 'd')
		return -1;
	bit++;
	len = be16_to_cpu(*(uint16_t *)bit);
	bit += sizeof(uint16_t);
	debug("Time: %s\n", bit);
	bit += len;

	/* Bitstream length */
	if (*bit != 'e')
		return -1;
	bit++;
	bitlen = be32_to_cpu(*(uint32_t *)bit);
	bit += sizeof(uint32_t);
	bitbin = bit;

	debug("Bitstream Length: 0x%x\n", bitlen);
	for (i = 0; i < bitlen; i += sizeof(uint32_t)) {
		uint32_t *bitbin32 = (uint32_t *)&bitbin[i];
		*bitbin32 = __swab32(*bitbin32);
	}

	if (!bf->dest_dev)
		bf->dest_dev = PART_ATTR_DEST_DEVICE_PL;

	bf->load = 0xffffffff;
	bf->entry = 0;

	bf->flags |= 1ULL << BIF_FLAG_BIT_FILE;
	return bif_add_part(bf, bit, bitlen);
}

/* Add .bin bitstream */
static int bif_add_bin(struct bif_entry *bf)
{
	size_t size;
	char *bin = read_full_file(bf->filename, &size);

	if (!bf->dest_dev)
		bf->dest_dev = PART_ATTR_DEST_DEVICE_PS;

	bf->flags |= 1ULL << BIF_FLAG_BIN_FILE;
	return bif_add_part(bf, bin, size);
}

/* Add elf file */
static char *elf2flat64(char *elf, size_t *flat_size, size_t *load_addr)
{
	Elf64_Ehdr *ehdr;
	Elf64_Shdr *shdr;
	size_t min_addr = -1, max_addr = 0;
	char *flat;
	int i;

	ehdr = (void *)elf;
	shdr = (void *)(elf + le64_to_cpu(ehdr->e_shoff));

	/* Look for smallest / biggest address */
	for (i = 0; i < le64_to_cpu(ehdr->e_shnum); i++, shdr++) {
		if (!shdr->sh_size || !shdr->sh_addr ||
		    !(shdr->sh_flags & SHF_ALLOC) ||
		    (shdr->sh_type == SHT_NOBITS))
			continue;

		if (le64_to_cpu(shdr->sh_addr) < min_addr)
			min_addr = le64_to_cpu(shdr->sh_addr);
		if ((le64_to_cpu(shdr->sh_addr) + le64_to_cpu(shdr->sh_size)) >
			max_addr)
			max_addr = le64_to_cpu(shdr->sh_addr) +
				   le64_to_cpu(shdr->sh_size);
	}

	*load_addr = min_addr;
	*flat_size = max_addr - min_addr;
	flat = calloc(1, *flat_size);
	if (!flat)
		return NULL;

	shdr = (void *)(elf + le64_to_cpu(ehdr->e_shoff));
	for (i = 0; i < le64_to_cpu(ehdr->e_shnum); i++, shdr++) {
		char *dst = flat + le64_to_cpu(shdr->sh_addr) - min_addr;
		char *src = elf + le64_to_cpu(shdr->sh_offset);

		if (!shdr->sh_size || !shdr->sh_addr ||
		    !(shdr->sh_flags & SHF_ALLOC))
			continue;

		if (shdr->sh_type != SHT_NOBITS)
			memcpy(dst, src, le64_to_cpu(shdr->sh_size));
	}

	return flat;
}

static char *elf2flat32(char *elf, size_t *flat_size, size_t *load_addr)
{
	Elf32_Ehdr *ehdr;
	Elf32_Shdr *shdr;
	size_t min_addr = -1, max_addr = 0;
	char *flat;
	int i;

	ehdr = (void *)elf;
	shdr = (void *)(elf + le32_to_cpu(ehdr->e_shoff));

	/* Look for smallest / biggest address */
	for (i = 0; i < le32_to_cpu(ehdr->e_shnum); i++, shdr++) {
		if (!shdr->sh_size || !shdr->sh_addr ||
		    !(shdr->sh_flags & SHF_ALLOC) ||
		    (shdr->sh_type == SHT_NOBITS))
			continue;

		if (le32_to_cpu(shdr->sh_addr) < min_addr)
			min_addr = le32_to_cpu(shdr->sh_addr);
		if ((le32_to_cpu(shdr->sh_addr) + le32_to_cpu(shdr->sh_size)) >
			max_addr)
			max_addr = le32_to_cpu(shdr->sh_addr) +
				   le32_to_cpu(shdr->sh_size);
	}

	*load_addr = min_addr;
	*flat_size = max_addr - min_addr;
	flat = calloc(1, *flat_size);
	if (!flat)
		return NULL;

	shdr = (void *)(elf + le32_to_cpu(ehdr->e_shoff));
	for (i = 0; i < le32_to_cpu(ehdr->e_shnum); i++, shdr++) {
		char *dst = flat + le32_to_cpu(shdr->sh_addr) - min_addr;
		char *src = elf + le32_to_cpu(shdr->sh_offset);

		if (!shdr->sh_size || !shdr->sh_addr ||
		    !(shdr->sh_flags & SHF_ALLOC))
			continue;

		if (shdr->sh_type != SHT_NOBITS)
			memcpy(dst, src, le32_to_cpu(shdr->sh_size));
	}

	return flat;
}

static int bif_add_elf(struct bif_entry *bf)
{
	size_t size;
	size_t elf_size;
	char *elf;
	char *flat;
	size_t load_addr;
	Elf32_Ehdr *ehdr32;
	Elf64_Ehdr *ehdr64;

	elf = read_full_file(bf->filename, &elf_size);
	if (!elf)
		return -1;

	ehdr32 = (void *)elf;
	ehdr64 = (void *)elf;

	switch (ehdr32->e_ident[EI_CLASS]) {
	case ELFCLASS32:
		flat = elf2flat32(elf, &size, &load_addr);
		bf->entry = le32_to_cpu(ehdr32->e_entry);
		bf->flags |= 1ULL << BIF_FLAG_AARCH32;
		break;
	case ELFCLASS64:
		flat = elf2flat64(elf, &size, &load_addr);
		bf->entry = le64_to_cpu(ehdr64->e_entry);
		break;
	default:
		printf("Unknown ELF class: %d\n", ehdr32->e_ident[EI_CLASS]);
		return -1;
	}

	if (!flat)
		return -1;

	bf->load = load_addr;
	if (!bf->dest_dev)
		bf->dest_dev = PART_ATTR_DEST_DEVICE_PS;

	bf->flags |= 1ULL << BIF_FLAG_ELF_FILE;
	return bif_add_part(bf, flat, size);
}

static const struct bif_file_type bif_file_types[] = {
	{
		.name = "bitstream (.bit)",
		.header = 0x00090ff0,
		.add = bif_add_bit,
	},

	{
		.name = "ELF",
		.header = 0x7f454c46,
		.add = bif_add_elf,
	},

	/* Anything else is a .bin file */
	{
		.name = ".bin",
		.add = bif_add_bin,
	},
};

static int bif_fsbl_config(struct bif_entry *fsbl_config,
			   struct bif_entry *entries, int nr_entries)
{
	int i;
	int config_set = 0;
	struct {
		const char *name;
		uint64_t flags;
		uint64_t dest_cpu;
	} configs[] = {
		{ .name = "a5x_x64", .dest_cpu = PART_ATTR_DEST_CPU_A53_0 },
		{ .name = "a53_x64", .dest_cpu = PART_ATTR_DEST_CPU_A53_0 },
		{ .name = "a5x_x32", .dest_cpu = PART_ATTR_DEST_CPU_A53_0,
				     .flags = 1ULL << BIF_FLAG_AARCH32 },
		{ .name = "a53_x32", .dest_cpu = PART_ATTR_DEST_CPU_A53_0,
				     .flags = 1ULL << BIF_FLAG_AARCH32 },
		{ .name = "r5_single", .dest_cpu = PART_ATTR_DEST_CPU_R5_0 },
		{ .name = "r5_dual", .dest_cpu = PART_ATTR_DEST_CPU_R5_L },
	};

	/* Set target CPU of bootloader entry */
	for (i = 0; i < nr_entries; i++) {
		struct bif_entry *b = &entries[i];
		const char *config_attr = fsbl_config->filename;
		int j;

		if (!(b->flags & (1ULL << BIF_FLAG_BOOTLOADER)))
			continue;

		for (j = 0; j < ARRAY_SIZE(configs); j++) {
			if (!strncmp(config_attr, configs[j].name,
				     strlen(configs[j].name))) {
				b->dest_cpu = configs[j].dest_cpu;
				b->flags |= configs[j].flags;
				config_set = 1;
			}
		}

		if (!config_set) {
			printf("ERROR: Unsupported fsbl_config: %s\n",
			       config_attr);
			return -1;
		}
	}

	if (!config_set) {
		printf("ERROR: fsbl_config w/o bootloader\n");
		return -1;
	}

	return 0;
}

static const struct bif_flags *find_flag(char *str)
{
	const struct bif_flags *bf;
	int i;

	for (i = 0; i < ARRAY_SIZE(bif_flags); i++) {
		bf = &bif_flags[i];
		if (!strncmp(bf->name, str, strlen(bf->name)))
			return bf;
	}

	printf("ERROR: Flag '%s' not found\n", str);

	return NULL;
}

static int bif_open_file(struct bif_entry *entry)
{
	int fd = open(entry->filename, O_RDONLY);

	if (fd < 0)
		printf("Error opening file %s\n", entry->filename);

	return fd;
}

static const struct bif_file_type *get_file_type(struct bif_entry *entry)
{
	int fd = bif_open_file(entry);
	uint32_t header;
	int i;

	if (fd < 0)
		return NULL;

	if (read(fd, &header, sizeof(header)) != sizeof(header)) {
		printf("Error reading file %s", entry->filename);
		return NULL;
	}

	close(fd);

	for (i = 0; i < ARRAY_SIZE(bif_file_types); i++) {
		const struct bif_file_type *type = &bif_file_types[i];

		if (!type->header)
			return type;
		if (type->header == be32_to_cpu(header))
			return type;
	}

	return NULL;
}

#define NEXT_CHAR(str, chr) ({		\
	char *_n = strchr(str, chr);	\
	if (!_n)			\
		goto err;		\
	_n;				\
})

static char *skip_whitespace(char *str)
{
	while (*str == ' ' || *str == '\t')
		str++;

	return str;
}

int zynqmpbif_copy_image(int outfd, struct image_tool_params *mparams)
{
	char *bif, *bifp, *bifpn;
	char *line;
	struct bif_entry entries[32] = { { 0 } };
	int nr_entries = 0;
	struct bif_entry *entry = entries;
	size_t len;
	int i;
	uint32_t csum;
	int bldr = -1;

	bif_init();

	/* Read .bif input file */
	bif = read_full_file(mparams->datafile, NULL);
	if (!bif)
		goto err;

	/* Interpret .bif file */
	bifp = bif;

	/* A bif description starts with a { section */
	bifp = NEXT_CHAR(bifp, '{') + 1;

	/* Read every line */
	while (1) {
		bifpn = NEXT_CHAR(bifp, '\n');

		if (bifpn[-1] == '\r')
			bifpn[-1] = '\0';

		*bifpn = '\0';
		bifpn++;
		line = bifp;

		line = skip_whitespace(line);

		/* Attributes? */
		if (*line == '[') {
			line++;
			while (1) {
				const struct bif_flags *bf;

				line = skip_whitespace(line);
				bf = find_flag(line);
				if (!bf)
					goto err;

				line += strlen(bf->name);
				if (bf->parse)
					line = bf->parse(line, entry);
				else
					entry->flags |= 1ULL << bf->flag;

				if (!line)
					goto err;

				/* Go to next attribute or quit */
				if (*line == ']') {
					line++;
					break;
				}
				if (*line == ',')
					line++;
			}
		}

		/* End of image description */
		if (*line == '}')
			break;

		if (*line) {
			line = skip_whitespace(line);
			entry->filename = line;
			nr_entries++;
			entry++;
		}

		/* Use next line */
		bifp = bifpn;
	}

	for (i = 0; i < nr_entries; i++) {
		debug("Entry flags=%#lx name=%s\n", entries[i].flags,
		      entries[i].filename);
	}

	/*
	 * Some entries are actually configuration option for other ones,
	 * let's apply them in an intermediate step.
	 */
	for (i = 0; i < nr_entries; i++) {
		struct bif_entry *entry = &entries[i];

		if (entry->flags & (1ULL << BIF_FLAG_FSBL_CONFIG))
			if (bif_fsbl_config(entry, entries, nr_entries))
				goto err;
	}

	/* Make sure PMUFW comes before bootloader */
	for (i = 0; i < nr_entries; i++) {
		struct bif_entry *entry = &entries[i];

		if (entry->flags & (1ULL << BIF_FLAG_BOOTLOADER))
			bldr = i;
		if (entry->flags & (1ULL << BIF_FLAG_PMUFW_IMAGE)) {
			if (bldr >= 0) {
				struct bif_entry tmp = *entry;

				*entry = entries[bldr];
				entries[bldr] = tmp;
			}
		}
	}

	for (i = 0; i < nr_entries; i++) {
		struct bif_entry *entry = &entries[i];
		const struct bif_file_type *type;
		int r;

		if (entry->flags & (1ULL << BIF_FLAG_FSBL_CONFIG))
			continue;

		type = get_file_type(entry);
		if (!type)
			goto err;

		debug("type=%s file=%s\n", type->name, entry->filename);
		r = type->add(entry);
		if (r)
			goto err;
	}

	/* Calculate checksums */
	csum = zynqmp_csum(&bif_output.header->width_detection,
			   &bif_output.header->checksum);
	bif_output.header->checksum = cpu_to_le32(csum);

	if (bif_output.imgheader) {
		csum = zynqmp_csum(bif_output.imgheader,
				   &bif_output.imgheader->checksum);
		bif_output.imgheader->checksum = cpu_to_le32(csum);
	}

	/* Write headers and components */
	if (lseek(outfd, 0, SEEK_SET) != 0)
		goto err;

	len = bif_output.data_len;
	bifp = bif_output.data;
	while (len) {
		int r;

		r = write(outfd, bifp, len);
		if (r < 0)
			goto err;
		len -= r;
		bifp += r;
	}

	return 0;

err:
	fprintf(stderr, "Error: Failed to create image.\n");
	return -1;
}

/* Needs to be stubbed out so we can print after creation */
static void zynqmpbif_set_header(void *ptr, struct stat *sbuf, int ifd,
				 struct image_tool_params *params)
{
}

static struct zynqmp_header zynqmpimage_header;

U_BOOT_IMAGE_TYPE(
	zynqmpbif,
	"Xilinx ZynqMP Boot Image support (bif)",
	sizeof(struct zynqmp_header),
	(void *)&zynqmpimage_header,
	zynqmpbif_check_params,
	NULL,
	zynqmpimage_print_header,
	zynqmpbif_set_header,
	NULL,
	zynqmpbif_check_image_types,
	NULL,
	NULL
);
