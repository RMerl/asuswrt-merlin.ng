// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2018 NXP
 *
 * Peng Fan <peng.fan@nxp.com>
 */

#include "imx8image.h"

static int p_idx;
static int sector_size;
static soc_type_t soc;
static int container = -1;
static int32_t core_type = CFG_CORE_INVALID;
static bool emmc_fastboot;
static image_t param_stack[IMG_STACK_SIZE];
static uint8_t fuse_version;
static uint16_t sw_version;
static uint32_t custom_partition;
static uint32_t scfw_flags;

int imx8image_check_params(struct image_tool_params *params)
{
	return 0;
}

static void imx8image_set_header(void *ptr, struct stat *sbuf, int ifd,
				 struct image_tool_params *params)
{
}

static void imx8image_print_header(const void *ptr)
{
}

static int imx8image_check_image_types(uint8_t type)
{
	return (type == IH_TYPE_IMX8IMAGE) ? EXIT_SUCCESS : EXIT_FAILURE;
}

static table_entry_t imx8image_cmds[] = {
	{CMD_BOOT_FROM,         "BOOT_FROM",            "boot command",	      },
	{CMD_FUSE_VERSION,      "FUSE_VERSION",         "fuse version",	      },
	{CMD_SW_VERSION,        "SW_VERSION",           "sw version",	      },
	{CMD_MSG_BLOCK,         "MSG_BLOCK",            "msg block",	      },
	{CMD_FILEOFF,           "FILEOFF",              "fileoff",	      },
	{CMD_FLAG,              "FLAG",                 "flag",	      },
	{CMD_APPEND,            "APPEND",               "append a container", },
	{CMD_PARTITION,         "PARTITION",            "new partition",      },
	{CMD_SOC_TYPE,          "SOC_TYPE",             "soc type",           },
	{CMD_CONTAINER,         "CONTAINER",            "new container",      },
	{CMD_IMAGE,             "IMAGE",                "new image",          },
	{CMD_DATA,              "DATA",                 "new data",           },
	{-1,                    "",                     "",	              },
};

static table_entry_t imx8image_core_entries[] = {
	{CFG_SCU,	"SCU",			"scu core",	},
	{CFG_M40,	"M40",			"M4 core 0",	},
	{CFG_M41,	"M41",			"M4 core 1",	},
	{CFG_A35,	"A35",			"A35 core",	},
	{CFG_A53,	"A53",			"A53 core",	},
	{CFG_A72,	"A72",			"A72 core",	},
	{-1,		"",			"",		},
};

static table_entry_t imx8image_sector_size[] = {
	{0x400,		"sd",			"sd/emmc",},
	{0x400,		"emmc_fastboot",	"emmc fastboot",},
	{0x400,		"fspi",			"flexspi",	},
	{0x1000,	"nand_4k",		"nand 4K",	},
	{0x2000,	"nand_8k",		"nand 8K",	},
	{0x4000,	"nand_16k",		"nand 16K",	},
	{-1,		"",			"Invalid",	},
};

static void parse_cfg_cmd(image_t *param_stack, int32_t cmd, char *token,
			  char *name, int lineno)
{
	switch (cmd) {
	case CMD_BOOT_FROM:
		sector_size = get_table_entry_id(imx8image_sector_size,
						 "imximage boot option",
						 token);
		if (!strncmp("emmc_fastboot", token, 13))
			emmc_fastboot = true;
		break;
	case CMD_FUSE_VERSION:
		fuse_version = (uint8_t)(strtoll(token, NULL, 0) & 0xFF);
		break;
	case CMD_SW_VERSION:
		sw_version = (uint8_t)(strtoll(token, NULL, 0) & 0xFFFF);
		break;
	case CMD_FILEOFF:
		param_stack[p_idx].option = FILEOFF;
		param_stack[p_idx++].dst = (uint32_t)strtoll(token, NULL, 0);
		break;
	case CMD_MSG_BLOCK:
		param_stack[p_idx].option = MSG_BLOCK;
		param_stack[p_idx].filename = token;
		break;
	case CMD_FLAG:
		param_stack[p_idx].option = FLAG;
		param_stack[p_idx++].entry = (uint32_t)strtoll(token, NULL, 0);
		break;
	case CMD_APPEND:
		param_stack[p_idx].option = APPEND;
		param_stack[p_idx++].filename = token;
		break;
	case CMD_PARTITION:
		param_stack[p_idx].option = PARTITION;
		param_stack[p_idx++].entry = (uint32_t)strtoll(token, NULL, 0);
		break;
	case CMD_SOC_TYPE:
		if (!strncmp(token, "IMX8QX", 6)) {
			soc = QX;
		} else if (!strncmp(token, "IMX8QM", 6)) {
			soc = QM;
		} else {
			fprintf(stderr, "Unknown CMD_SOC_TYPE");
			exit(EXIT_FAILURE);
		}
		break;
	case CMD_IMAGE:
	case CMD_DATA:
		core_type = get_table_entry_id(imx8image_core_entries,
					       "imx8image core entries",
					       token);
		if (core_type < 0) {
			fprintf(stderr, "Wrong IMAGE core_type %s\n", token);
			exit(EXIT_FAILURE);
		}
		break;
	default:
		break;
	}
}

static void parse_cfg_fld(image_t *param_stack, int32_t *cmd, char *token,
			  char *name, int lineno, int fld)
{
	switch (fld) {
	case CFG_COMMAND:
		*cmd = get_table_entry_id(imx8image_cmds, "imx8image cmds",
					  token);
		if (*cmd < 0) {
			fprintf(stderr, "Error: %s[%d] - Invalid command (%s)\n", name, lineno, token);
			exit(EXIT_FAILURE);
		}

		if (*cmd == CMD_CONTAINER) {
			fprintf(stdout, "New Container: \t%d\n", ++container);
			param_stack[p_idx++].option = NEW_CONTAINER;
		}
		break;
	case CFG_CORE_TYPE:
		parse_cfg_cmd(param_stack, *cmd, token, name, lineno);
		break;
	case CFG_IMAGE_NAME:
		if (*cmd == CMD_MSG_BLOCK) {
			if (!strncmp(token, "fuse", 4)) {
				param_stack[p_idx].ext = SC_R_OTP;
			} else if (!strncmp(token, "debug", 5)) {
				param_stack[p_idx].ext = SC_R_DEBUG;
			} else if (!strncmp(token, "field", 5)) {
				param_stack[p_idx].ext = SC_R_ROM_0;
			} else {
				fprintf(stderr, "MSG type not found %s\n", token);
				exit(EXIT_FAILURE);
			}
			break;
		}
		switch (core_type) {
		case CFG_SCU:
			param_stack[p_idx].option = SCFW;
			param_stack[p_idx++].filename = token;
			break;
		case CFG_M40:
			param_stack[p_idx].option = M40;
			param_stack[p_idx].ext = 0;
			param_stack[p_idx].filename = token;
			break;
		case CFG_M41:
			param_stack[p_idx].option = M41;
			param_stack[p_idx].ext = 1;
			param_stack[p_idx].filename = token;
			break;
		case CFG_A35:
			param_stack[p_idx].ext = CORE_CA35;
			param_stack[p_idx].option =
				(*cmd == CMD_DATA) ? DATA : AP;
			param_stack[p_idx].filename = token;
			break;
		case CFG_A53:
			param_stack[p_idx].ext = CORE_CA53;
			param_stack[p_idx].option =
				(*cmd == CMD_DATA) ? DATA : AP;
			param_stack[p_idx].filename = token;
			break;
		case CFG_A72:
			param_stack[p_idx].ext = CORE_CA72;
			param_stack[p_idx].option =
				(*cmd == CMD_DATA) ? DATA : AP;
			param_stack[p_idx].filename = token;
			break;
		}
		break;
	case CFG_LOAD_ADDR:
		if (*cmd == CMD_MSG_BLOCK) {
			param_stack[p_idx++].entry =
				(uint32_t)strtoll(token, NULL, 0);
			break;
		}
		switch (core_type) {
		case CFG_SCU:
			break;
		case CFG_M40:
		case CFG_M41:
		case CFG_A35:
		case CFG_A53:
		case CFG_A72:
			param_stack[p_idx++].entry =
				(uint32_t)strtoll(token, NULL, 0);
			break;
		}
	default:
		break;
	}
}

static uint32_t parse_cfg_file(image_t *param_stack, char *name)
{
	FILE *fd = NULL;
	char *line = NULL;
	char *token, *saveptr1, *saveptr2;
	int lineno = 0;
	int fld;
	size_t len;
	int32_t cmd;

	fd = fopen(name, "r");
	if (fd == 0) {
		fprintf(stderr, "Error: %s - Can't open cfg file\n", name);
		exit(EXIT_FAILURE);
	}

	/*
	 * Very simple parsing, line starting with # are comments
	 * and are dropped
	 */
	while ((getline(&line, &len, fd)) > 0) {
		lineno++;

		token = strtok_r(line, "\r\n", &saveptr1);
		if (!token)
			continue;

		/* Check inside the single line */
		for (fld = CFG_COMMAND, cmd = CFG_INVALID,
		     line = token; ; line = NULL, fld++) {
			token = strtok_r(line, " \t", &saveptr2);
			if (!token)
				break;

			/* Drop all text starting with '#' as comments */
			if (token[0] == '#')
				break;

			parse_cfg_fld(param_stack, &cmd, token, name, lineno,
				      fld);
		}
	}

	return 0;
}

static void check_file(struct stat *sbuf, char *filename)
{
	int tmp_fd  = open(filename, O_RDONLY | O_BINARY);

	if (tmp_fd < 0) {
		fprintf(stderr, "%s: Can't open: %s\n",
			filename, strerror(errno));
		exit(EXIT_FAILURE);
	}

	if (fstat(tmp_fd, sbuf) < 0) {
		fprintf(stderr, "%s: Can't stat: %s\n",
			filename, strerror(errno));
		exit(EXIT_FAILURE);
	}

	close(tmp_fd);
}

static void copy_file_aligned(int ifd, const char *datafile, int offset,
			      int align)
{
	int dfd;
	struct stat sbuf;
	unsigned char *ptr;
	uint8_t zeros[0x4000];
	int size;
	int ret;

	if (align > 0x4000) {
		fprintf(stderr, "Wrong alignment requested %d\n", align);
		exit(EXIT_FAILURE);
	}

	memset(zeros, 0, sizeof(zeros));

	dfd = open(datafile, O_RDONLY | O_BINARY);
	if (dfd < 0) {
		fprintf(stderr, "Can't open %s: %s\n",
			datafile, strerror(errno));
		exit(EXIT_FAILURE);
	}

	if (fstat(dfd, &sbuf) < 0) {
		fprintf(stderr, "Can't stat %s: %s\n",
			datafile, strerror(errno));
		exit(EXIT_FAILURE);
	}

	if (sbuf.st_size == 0)
		goto close;

	ptr = mmap(0, sbuf.st_size, PROT_READ, MAP_SHARED, dfd, 0);
	if (ptr == MAP_FAILED) {
		fprintf(stderr, "Can't read %s: %s\n",
			datafile, strerror(errno));
		exit(EXIT_FAILURE);
	}

	size = sbuf.st_size;
	ret = lseek(ifd, offset, SEEK_SET);
	if (ret < 0) {
		fprintf(stderr, "%s: lseek error %s\n",
			__func__, strerror(errno));
		exit(EXIT_FAILURE);
	}

	if (write(ifd, ptr, size) != size) {
		fprintf(stderr, "Write error %s\n", strerror(errno));
		exit(EXIT_FAILURE);
	}

	align = ALIGN(size, align) - size;

	if (write(ifd, (char *)&zeros, align) != align) {
		fprintf(stderr, "Write error: %s\n", strerror(errno));
		exit(EXIT_FAILURE);
	}

	munmap((void *)ptr, sbuf.st_size);
close:
	close(dfd);
}

static void copy_file (int ifd, const char *datafile, int pad, int offset)
{
	int dfd;
	struct stat sbuf;
	unsigned char *ptr;
	int tail;
	int zero = 0;
	uint8_t zeros[4096];
	int size, ret;

	memset(zeros, 0, sizeof(zeros));

	dfd = open(datafile, O_RDONLY | O_BINARY);
	if (dfd < 0) {
		fprintf(stderr, "Can't open %s: %s\n",
			datafile, strerror(errno));
		exit(EXIT_FAILURE);
	}

	if (fstat(dfd, &sbuf) < 0) {
		fprintf(stderr, "Can't stat %s: %s\n",
			datafile, strerror(errno));
		exit(EXIT_FAILURE);
	}

	if (sbuf.st_size == 0)
		goto close;

	ptr = mmap(0, sbuf.st_size, PROT_READ, MAP_SHARED, dfd, 0);
	if (ptr == MAP_FAILED) {
		fprintf(stderr, "Can't read %s: %s\n",
			datafile, strerror(errno));
		exit(EXIT_FAILURE);
	}

	size = sbuf.st_size;
	ret = lseek(ifd, offset, SEEK_SET);
	if (ret < 0) {
		fprintf(stderr, "%s: lseek error %s\n",
			__func__, strerror(errno));
		exit(EXIT_FAILURE);
	}

	if (write(ifd, ptr, size) != size) {
		fprintf(stderr, "Write error %s\n",
			strerror(errno));
		exit(EXIT_FAILURE);
	}

	tail = size % 4;
	pad = pad - size;
	if (pad == 1 && tail != 0) {
		if (write(ifd, (char *)&zero, 4 - tail) != 4 - tail) {
			fprintf(stderr, "Write error on %s\n",
				strerror(errno));
			exit(EXIT_FAILURE);
		}
	} else if (pad > 1) {
		while (pad > 0) {
			int todo = sizeof(zeros);

			if (todo > pad)
				todo = pad;
			if (write(ifd, (char *)&zeros, todo) != todo) {
				fprintf(stderr, "Write error: %s\n",
					strerror(errno));
				exit(EXIT_FAILURE);
			}
			pad -= todo;
		}
	}

	munmap((void *)ptr, sbuf.st_size);
close:
	close(dfd);
}

uint64_t read_dcd_offset(char *filename)
{
	int dfd;
	struct stat sbuf;
	uint8_t *ptr;
	uint64_t offset = 0;

	dfd = open(filename, O_RDONLY | O_BINARY);
	if (dfd < 0) {
		fprintf(stderr, "Can't open %s: %s\n", filename, strerror(errno));
		exit(EXIT_FAILURE);
	}

	if (fstat(dfd, &sbuf) < 0) {
		fprintf(stderr, "Can't stat %s: %s\n", filename, strerror(errno));
		exit(EXIT_FAILURE);
	}

	ptr = mmap(0, sbuf.st_size, PROT_READ, MAP_SHARED, dfd, 0);
	if (ptr == MAP_FAILED) {
		fprintf(stderr, "Can't read %s: %s\n", filename, strerror(errno));
		exit(EXIT_FAILURE);
	}

	offset = *(uint32_t *)(ptr + DCD_ENTRY_ADDR_IN_SCFW);

	munmap((void *)ptr, sbuf.st_size);
	close(dfd);

	return offset;
}

static void set_image_hash(boot_img_t *img, char *filename, uint32_t hash_type)
{
	FILE *fp = NULL;
	char sha_command[512];
	char hash[2 * HASH_MAX_LEN + 1];
	int i, ret;

	if (img->size == 0)
		sprintf(sha_command, "sha%dsum /dev/null", hash_type);
	else
		sprintf(sha_command, "dd if=/dev/zero of=tmp_pad bs=%d count=1;\
			dd if=\'%s\' of=tmp_pad conv=notrunc;\
			sha%dsum tmp_pad; rm -f tmp_pad",
			img->size, filename, hash_type);

	switch (hash_type) {
	case HASH_TYPE_SHA_256:
		img->hab_flags |= IMG_FLAG_HASH_SHA256;
		break;
	case HASH_TYPE_SHA_384:
		img->hab_flags |= IMG_FLAG_HASH_SHA384;
		break;
	case HASH_TYPE_SHA_512:
		img->hab_flags |= IMG_FLAG_HASH_SHA512;
		break;
	default:
		fprintf(stderr, "Wrong hash type selected (%d) !!!\n\n",
			hash_type);
		exit(EXIT_FAILURE);
		break;
	}
	memset(img->hash, 0, HASH_MAX_LEN);

	fp = popen(sha_command, "r");
	if (!fp) {
		fprintf(stderr, "Failed to run command hash\n");
		exit(EXIT_FAILURE);
	}

	if (!fgets(hash, hash_type / 4 + 1, fp)) {
		fprintf(stderr, "Failed to hash file: %s\n", filename);
		exit(EXIT_FAILURE);
	}

	for (i = 0; i < strlen(hash) / 2; i++) {
		ret = sscanf(hash + 2 * i, "%02hhx", &img->hash[i]);
		if (ret < 0) {
			fprintf(stderr, "Failed sscanf hash: %d\n", ret);
			exit(EXIT_FAILURE);
		}
	}

	pclose(fp);
}

static void set_image_array_entry(flash_header_v3_t *container,
				  soc_type_t soc, const image_t *image_stack,
				  uint32_t offset, uint32_t size,
				  char *tmp_filename, bool dcd_skip)
{
	uint64_t entry = image_stack->entry;
	uint64_t core = image_stack->ext;
	uint32_t meta;
	char *tmp_name = "";
	option_type_t type = image_stack->option;
	boot_img_t *img = &container->img[container->num_images];

	img->offset = offset;  /* Is re-adjusted later */
	img->size = size;

	set_image_hash(img, tmp_filename, IMAGE_HASH_ALGO_DEFAULT);

	switch (type) {
	case SECO:
		img->hab_flags |= IMG_TYPE_SECO;
		img->hab_flags |= CORE_SECO << BOOT_IMG_FLAGS_CORE_SHIFT;
		tmp_name = "SECO";
		img->dst = 0x20C00000;
		img->entry = 0x20000000;
		break;
	case AP:
		if (soc == QX && core == CORE_CA35) {
			meta = IMAGE_A35_DEFAULT_META(custom_partition);
		} else if (soc == QM && core == CORE_CA53) {
			meta = IMAGE_A53_DEFAULT_META(custom_partition);
		} else if (soc == QM && core == CORE_CA72) {
			meta = IMAGE_A72_DEFAULT_META(custom_partition);
		} else {
			fprintf(stderr,
				"Error: invalid AP core id: %" PRIu64 "\n",
				core);
			exit(EXIT_FAILURE);
		}
		img->hab_flags |= IMG_TYPE_EXEC;
		/* On B0, only core id = 4 is valid */
		img->hab_flags |= CORE_CA53 << BOOT_IMG_FLAGS_CORE_SHIFT;
		tmp_name = "AP";
		img->dst = entry;
		img->entry = entry;
		img->meta = meta;
		custom_partition = 0;
		break;
	case M40:
	case M41:
		if (core == 0) {
			core = CORE_CM4_0;
			meta = IMAGE_M4_0_DEFAULT_META(custom_partition);
		} else if (core == 1) {
			core = CORE_CM4_1;
			meta = IMAGE_M4_1_DEFAULT_META(custom_partition);
		} else {
			fprintf(stderr,
				"Error: invalid m4 core id: %" PRIu64 "\n",
				core);
			exit(EXIT_FAILURE);
		}
		img->hab_flags |= IMG_TYPE_EXEC;
		img->hab_flags |= core << BOOT_IMG_FLAGS_CORE_SHIFT;
		tmp_name = "M4";
		if ((entry & 0x7) != 0) {
			fprintf(stderr, "\n\nWarning: M4 Destination address is not 8 byte aligned\n\n");
			exit(EXIT_FAILURE);
		}
		img->dst = entry;
		img->entry = entry;
		img->meta = meta;
		custom_partition = 0;
		break;
	case DATA:
		img->hab_flags |= IMG_TYPE_DATA;
		img->hab_flags |= CORE_CA35 << BOOT_IMG_FLAGS_CORE_SHIFT;
		tmp_name = "DATA";
		img->dst = entry;
		break;
	case MSG_BLOCK:
		img->hab_flags |= IMG_TYPE_DATA;
		img->hab_flags |= CORE_CA35 << BOOT_IMG_FLAGS_CORE_SHIFT;
		img->meta = core << BOOT_IMG_META_MU_RID_SHIFT;
		tmp_name = "MSG_BLOCK";
		img->dst = entry;
		break;
	case SCFW:
		img->hab_flags |= scfw_flags & 0xFFFF0000;
		img->hab_flags |= IMG_TYPE_EXEC;
		img->hab_flags |= CORE_SC << BOOT_IMG_FLAGS_CORE_SHIFT;
		tmp_name = "SCFW";
		img->dst = 0x1FFE0000;
		img->entry = 0x1FFE0000;

		/* Lets add the DCD now */
		if (!dcd_skip) {
			container->num_images++;
			img = &container->img[container->num_images];
			img->hab_flags |= IMG_TYPE_DCD_DDR;
			img->hab_flags |= CORE_SC << BOOT_IMG_FLAGS_CORE_SHIFT;
			set_image_hash(img, "/dev/null",
				       IMAGE_HASH_ALGO_DEFAULT);
			img->offset = offset + img->size;
			img->entry = read_dcd_offset(tmp_filename);
			img->dst = img->entry - 1;
		}
		break;
	default:
		fprintf(stderr, "unrecognized image type (%d)\n", type);
		exit(EXIT_FAILURE);
	}

	fprintf(stdout, "%s file_offset = 0x%x size = 0x%x\n", tmp_name, offset, size);

	container->num_images++;
}

void set_container(flash_header_v3_t *container,  uint16_t sw_version,
		   uint32_t alignment, uint32_t flags, uint16_t fuse_version)
{
	container->sig_blk_hdr.tag = 0x90;
	container->sig_blk_hdr.length = sizeof(sig_blk_hdr_t);
	container->sw_version = sw_version;
	container->padding = alignment;
	container->fuse_version = fuse_version;
	container->flags = flags;
	fprintf(stdout, "container flags: 0x%x\n", container->flags);
}

static int get_container_image_start_pos(image_t *image_stack, uint32_t align)
{
	image_t *img_sp = image_stack;
	/*8K total container header*/
	int file_off = CONTAINER_IMAGE_ARRAY_START_OFFSET;
	FILE *fd = NULL;
	flash_header_v3_t header;
	int ret;

	while (img_sp->option != NO_IMG) {
		if (img_sp->option == APPEND) {
			fd = fopen(img_sp->filename, "r");
			if (!fd) {
				fprintf(stderr, "Fail open first container file %s\n", img_sp->filename);
				exit(EXIT_FAILURE);
			}

			ret = fread(&header, sizeof(header), 1, fd);
			if (ret != 1) {
				printf("Failure Read header %d\n", ret);
				exit(EXIT_FAILURE);
			}

			fclose(fd);

			if (header.tag != IVT_HEADER_TAG_B0) {
				fprintf(stderr, "header tag mismatched \n");
				exit(EXIT_FAILURE);
			} else {
				file_off +=
					header.img[header.num_images - 1].size;
				file_off = ALIGN(file_off, align);
			}
		}

		img_sp++;
	}

	return file_off;
}

static void set_imx_hdr_v3(imx_header_v3_t *imxhdr, uint32_t cont_id)
{
	flash_header_v3_t *fhdr_v3 = &imxhdr->fhdr[cont_id];

	/* Set magic number, Only >= B0 supported */
	fhdr_v3->tag = IVT_HEADER_TAG_B0;
	fhdr_v3->version = IVT_VERSION_B0;
}

static uint8_t *flatten_container_header(imx_header_v3_t *imx_header,
					 uint8_t containers_count,
					 uint32_t *size_out,
					 uint32_t file_offset)
{
	uint8_t *flat = NULL;
	uint8_t *ptr = NULL;
	uint16_t size = 0;
	int i, j;

	/* Compute size of all container headers */
	for (i = 0; i < containers_count; i++) {
		flash_header_v3_t *container = &imx_header->fhdr[i];

		container->sig_blk_offset = HEADER_IMG_ARRAY_OFFSET +
			container->num_images * IMG_ARRAY_ENTRY_SIZE;

		container->length = HEADER_IMG_ARRAY_OFFSET +
			(IMG_ARRAY_ENTRY_SIZE * container->num_images) +
			sizeof(sig_blk_hdr_t);

		/* Print info needed by CST to sign the container header */
		fprintf(stdout, "CST: CONTAINER %d offset: 0x%x\n",
			i, file_offset + size);
		fprintf(stdout, "CST: CONTAINER %d: Signature Block: offset is at 0x%x\n", i,
			file_offset + size + container->length -
			SIGNATURE_BLOCK_HEADER_LENGTH);

		size += ALIGN(container->length, container->padding);
	}

	flat = calloc(size, sizeof(uint8_t));
	if (!flat) {
		fprintf(stderr, "Failed to allocate memory (%d)\n", size);
		exit(EXIT_FAILURE);
	}

	ptr = flat;
	*size_out = size;

	for (i = 0; i < containers_count; i++) {
		flash_header_v3_t *container = &imx_header->fhdr[i];
		uint32_t container_start_offset = ptr - flat;

		/* Append container header */
		append(ptr, container, HEADER_IMG_ARRAY_OFFSET);

		/* Adjust images offset to start from container headers start */
		for (j = 0; j < container->num_images; j++) {
			container->img[j].offset -=
				container_start_offset + file_offset;
		}
		/* Append each image array entry */
		for (j = 0; j < container->num_images; j++)
			append(ptr, &container->img[j], sizeof(boot_img_t));

		append(ptr, &container->sig_blk_hdr, sizeof(sig_blk_hdr_t));

		/* Padding for container (if necessary) */
		ptr += ALIGN(container->length, container->padding) -
			container->length;
	}

	return flat;
}

static int build_container(soc_type_t soc, uint32_t sector_size,
			   bool emmc_fastboot, image_t *image_stack,
			   bool dcd_skip, uint8_t fuse_version,
			   uint16_t sw_version, int ofd)
{
	static imx_header_v3_t imx_header;
	image_t *img_sp = image_stack;
	int file_off;
	uint8_t *tmp;
	struct stat sbuf;
	char *tmp_filename = NULL;
	uint32_t size = 0;
	uint32_t file_padding = 0;
	int ret;

	int container = -1;
	int cont_img_count = 0; /* indexes to arrange the container */

	memset((char *)&imx_header, 0, sizeof(imx_header_v3_t));

	if (!image_stack) {
		fprintf(stderr, "Empty image stack ");
		exit(EXIT_FAILURE);
	}

	if (soc == QX)
		fprintf(stdout, "Platform:\ti.MX8QXP B0\n");
	else if (soc == QM)
		fprintf(stdout, "Platform:\ti.MX8QM B0\n");

	set_imx_hdr_v3(&imx_header, 0);
	set_imx_hdr_v3(&imx_header, 1);

	file_off = get_container_image_start_pos(image_stack, sector_size);
	fprintf(stdout, "container image offset (aligned):%x\n", file_off);

	/* step through image stack and generate the header */
	img_sp = image_stack;

	/* stop once we reach null terminator */
	while (img_sp->option != NO_IMG) {
		switch (img_sp->option) {
		case AP:
		case M40:
		case M41:
		case SCFW:
		case DATA:
		case MSG_BLOCK:
			if (container < 0) {
				fprintf(stderr, "No container found\n");
				exit(EXIT_FAILURE);
			}
			check_file(&sbuf, img_sp->filename);
			tmp_filename = img_sp->filename;
			set_image_array_entry(&imx_header.fhdr[container],
					      soc, img_sp, file_off,
					      ALIGN(sbuf.st_size, sector_size),
					      tmp_filename, dcd_skip);
			img_sp->src = file_off;

			file_off += ALIGN(sbuf.st_size, sector_size);
			cont_img_count++;
			break;

		case SECO:
			if (container < 0) {
				fprintf(stderr, "No container found\n");
				exit(EXIT_FAILURE);
			}
			check_file(&sbuf, img_sp->filename);
			tmp_filename = img_sp->filename;
			set_image_array_entry(&imx_header.fhdr[container],
					      soc,
					      img_sp,
					      file_off,
					      sbuf.st_size,
					      tmp_filename, dcd_skip);
			img_sp->src = file_off;

			file_off += sbuf.st_size;
			cont_img_count++;
			break;

		case NEW_CONTAINER:
			container++;
			set_container(&imx_header.fhdr[container], sw_version,
				      CONTAINER_ALIGNMENT,
				      CONTAINER_FLAGS_DEFAULT,
				      fuse_version);
			/* reset img count when moving to new container */
			cont_img_count = 0;
			scfw_flags = 0;
			break;

		case APPEND:
			/*
			 * nothing to do here, the container is appended
			 * in the output
			 */
			break;
		case FLAG:
			/*
			 * override the flags for scfw in current container
			 * mask off bottom 16 bits.
			 */
			scfw_flags = img_sp->entry & 0xFFFF0000;
			break;
		case FILEOFF:
			if (file_off > img_sp->dst) {
				fprintf(stderr, "FILEOFF address less than current file offset!!!\n");
				exit(EXIT_FAILURE);
			}
			if (img_sp->dst != ALIGN(img_sp->dst, sector_size)) {
				fprintf(stderr, "FILEOFF address is not aligned to sector size!!!\n");
				exit(EXIT_FAILURE);
			}
			file_off = img_sp->dst;
			break;
		case PARTITION:
			/*
			 * keep custom partition until next executable image
			 * use a global var for default behaviour
			 */
			custom_partition = img_sp->entry;
			break;
		default:
			fprintf(stderr, "unrecognized option in input stack (%d)\n", img_sp->option);
			exit(EXIT_FAILURE);
		}
		img_sp++; /* advance index */
	}

	/* Append container (if specified) */
	img_sp = image_stack;
	do {
		if (img_sp->option == APPEND) {
			copy_file(ofd, img_sp->filename, 0, 0);
			file_padding += FIRST_CONTAINER_HEADER_LENGTH;
		}
		img_sp++;
	} while (img_sp->option != NO_IMG);

	/* Add padding or skip appended container */
	ret = lseek(ofd, file_padding, SEEK_SET);
	if (ret < 0) {
		fprintf(stderr, "%s: lseek error %s\n",
			__func__, strerror(errno));
		exit(EXIT_FAILURE);
	}

	if (container >= 0) {
		/* Note: Image offset are not contained in the image */
		tmp = flatten_container_header(&imx_header, container + 1,
					       &size, file_padding);
		/* Write image header */
		if (write(ofd, tmp, size) != size) {
			fprintf(stderr, "error writing image hdr\n");
			exit(EXIT_FAILURE);
		}

		/* Clean-up memory used by the headers */
		free(tmp);
	}

	/*
	 * step through the image stack again this time copying
	 * images to final bin, stop once we reach null terminator.
	 */
	img_sp = image_stack;
	while (img_sp->option != NO_IMG) {
		if (img_sp->option == M40 || img_sp->option == M41 ||
		    img_sp->option == AP || img_sp->option == DATA ||
		    img_sp->option == SCD || img_sp->option == SCFW ||
		    img_sp->option == SECO || img_sp->option == MSG_BLOCK) {
			copy_file_aligned(ofd, img_sp->filename, img_sp->src,
					  sector_size);
		}
		img_sp++;
	}

	return 0;
}

int imx8image_copy_image(int outfd, struct image_tool_params *mparams)
{
	image_t *img_sp = param_stack;

	/*
	 * SECO FW is a container image, this is to calculate the
	 * 2nd container offset.
	 */
	fprintf(stdout, "parsing %s\n", mparams->imagename);
	parse_cfg_file(img_sp, mparams->imagename);

	if (sector_size == 0) {
		fprintf(stderr, "Wrong sector size\n");
		exit(EXIT_FAILURE);
	}

	fprintf(stdout, "CONTAINER Sector size:\t%08x\n", sector_size);
	fprintf(stdout, "CONTAINER FUSE VERSION:\t0x%02x\n", fuse_version);
	fprintf(stdout, "CONTAINER SW VERSION:\t0x%04x\n", sw_version);

	build_container(soc, sector_size, emmc_fastboot,
			img_sp, false, fuse_version, sw_version, outfd);

	return 0;
}

/*
 * imx8image parameters
 */
U_BOOT_IMAGE_TYPE(
	imx8image,
	"NXP i.MX8 Boot Image support",
	0,
	NULL,
	imx8image_check_params,
	NULL,
	imx8image_print_header,
	imx8image_set_header,
	NULL,
	imx8image_check_image_types,
	NULL,
	NULL
);
