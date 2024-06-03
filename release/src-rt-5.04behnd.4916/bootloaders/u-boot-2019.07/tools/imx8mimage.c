// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2018 NXP
 *
 * Peng Fan <peng.fan@nxp.com>
 */


#include "imagetool.h"
#include <image.h>
#include "imximage.h"
#include "compiler.h"

static uint32_t ap_start_addr, sld_start_addr, sld_src_off;
static char *ap_img, *sld_img, *signed_hdmi;
static imx_header_v3_t imx_header[2]; /* At most there are 3 IVT headers */
static uint32_t rom_image_offset;
static uint32_t sector_size = 0x200;
static uint32_t image_off;
static uint32_t sld_header_off;
static uint32_t ivt_offset;
static uint32_t using_fit;

#define CSF_SIZE 0x2000
#define HDMI_IVT_ID 0
#define IMAGE_IVT_ID 1

#define HDMI_FW_SIZE		0x17000 /* Use Last 0x1000 for IVT and CSF */
#define ALIGN_SIZE		0x1000
#define ALIGN(x,a)	__ALIGN_MASK((x), (__typeof__(x))(a) - 1, a)
#define __ALIGN_MASK(x,mask,mask2) (((x) + (mask)) / (mask2) * (mask2))

static uint32_t get_cfg_value(char *token, char *name,  int linenr)
{
	char *endptr;
	uint32_t value;

	errno = 0;
	value = strtoul(token, &endptr, 16);
	if (errno || token == endptr) {
		fprintf(stderr, "Error: %s[%d] - Invalid hex data(%s)\n",
			name,  linenr, token);
		exit(EXIT_FAILURE);
	}
	return value;
}

int imx8mimage_check_params(struct image_tool_params *params)
{
	return 0;
}

static void imx8mimage_set_header(void *ptr, struct stat *sbuf, int ifd,
				  struct image_tool_params *params)
{
}

static void imx8mimage_print_header(const void *ptr)
{
}

static int imx8mimage_check_image_types(uint8_t type)
{
	return (type == IH_TYPE_IMX8MIMAGE) ? EXIT_SUCCESS : EXIT_FAILURE;
}

static table_entry_t imx8mimage_cmds[] = {
	{CMD_BOOT_FROM,         "BOOT_FROM",            "boot command",	      },
	{CMD_FIT,               "FIT",                  "fit image",	      },
	{CMD_SIGNED_HDMI,       "SIGNED_HDMI",          "signed hdmi image",  },
	{CMD_LOADER,            "LOADER",               "loader image",       },
	{CMD_SECOND_LOADER,     "SECOND_LOADER",        "2nd loader image",   },
	{CMD_DDR_FW,            "DDR_FW",               "ddr firmware",       },
	{-1,                    "",                     "",	              },
};

static table_entry_t imx8mimage_ivt_offset[] = {
	{0x400,		"sd",			"sd/emmc",},
	{0x400,		"emmc_fastboot",	"emmc fastboot",},
	{0x1000,	"fspi",			"flexspi",	},
	{-1,		"",			"Invalid",	},
};

static void parse_cfg_cmd(int32_t cmd, char *token, char *name, int lineno)
{
	switch (cmd) {
	case CMD_BOOT_FROM:
		ivt_offset = get_table_entry_id(imx8mimage_ivt_offset,
						"imx8mimage ivt offset",
						token);
		if (!strncmp(token, "sd", 2))
			rom_image_offset = 0x8000;
		break;
	case CMD_LOADER:
		ap_img = token;
		break;
	case CMD_SECOND_LOADER:
		sld_img = token;
		break;
	case CMD_SIGNED_HDMI:
		signed_hdmi = token;
	case CMD_FIT:
		using_fit = 1;
		break;
	case CMD_DDR_FW:
		/* Do nothing */
		break;
	}
}

static void parse_cfg_fld(int32_t *cmd, char *token,
			  char *name, int lineno, int fld)
{
	switch (fld) {
	case CFG_COMMAND:
		*cmd = get_table_entry_id(imx8mimage_cmds,
					  "imx8mimage commands", token);
		if (*cmd < 0) {
			fprintf(stderr, "Error: %s[%d] - Invalid command" "(%s)\n",
				name, lineno, token);
			exit(EXIT_FAILURE);
		}
		break;
	case CFG_REG_SIZE:
		parse_cfg_cmd(*cmd, token, name, lineno);
		break;
	case CFG_REG_ADDRESS:
		switch (*cmd) {
		case CMD_LOADER:
			ap_start_addr = get_cfg_value(token, name, lineno);
			break;
		case CMD_SECOND_LOADER:
			sld_start_addr = get_cfg_value(token, name, lineno);
			break;
		}
		break;
	case CFG_REG_VALUE:
		switch (*cmd) {
		case CMD_SECOND_LOADER:
			sld_src_off = get_cfg_value(token, name, lineno);
			break;
		}
	default:
		break;
	}
}

static uint32_t parse_cfg_file(char *name)
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

			parse_cfg_fld(&cmd, token, name, lineno, fld);
		}
	}

	return 0;
}

static void fill_zero(int ifd, int size, int offset)
{
	int fill_size;
	uint8_t zeros[4096];
	int ret;

	memset(zeros, 0, sizeof(zeros));

	ret = lseek(ifd, offset, SEEK_SET);
	if (ret < 0) {
		fprintf(stderr, "%s seek: %s\n", __func__, strerror(errno));
		exit(EXIT_FAILURE);
	}

	while (size) {
		if (size > 4096)
			fill_size = 4096;
		else
			fill_size = size;

		if (write(ifd, (char *)&zeros, fill_size) != fill_size) {
			fprintf(stderr, "Write error: %s\n",
				strerror(errno));
			exit(EXIT_FAILURE);
		}

		size -= fill_size;
	};
}

static void copy_file(int ifd, const char *datafile, int pad, int offset,
		      int datafile_offset)
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

	ptr = mmap(0, sbuf.st_size, PROT_READ, MAP_SHARED, dfd, 0);
	if (ptr == MAP_FAILED) {
		fprintf(stderr, "Can't read %s: %s\n",
			datafile, strerror(errno));
		exit(EXIT_FAILURE);
	}

	size = sbuf.st_size - datafile_offset;
	ret = lseek(ifd, offset, SEEK_SET);
	if (ret < 0) {
		fprintf(stderr, "lseek ifd fail\n");
		exit(EXIT_FAILURE);
	}

	if (write(ifd, ptr + datafile_offset, size) != size) {
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
	close(dfd);
}

/* Return this IVT offset in the final output file */
static int generate_ivt_for_fit(int fd, int fit_offset, uint32_t ep,
				uint32_t *fit_load_addr)
{
	image_header_t image_header;
	int ret;

	uint32_t fit_size, load_addr;
	int align_len = 64 - 1; /* 64 is cacheline size */

	ret = lseek(fd, fit_offset, SEEK_SET);
	if (ret < 0) {
		fprintf(stderr, "lseek fd fail for fit\n");
		exit(EXIT_FAILURE);
	}

	if (read(fd, (char *)&image_header, sizeof(image_header_t)) !=
	    sizeof(image_header_t)) {
		fprintf(stderr, "generate_ivt_for_fit read failed: %s\n",
			strerror(errno));
		exit(EXIT_FAILURE);
	}

	if (be32_to_cpu(image_header.ih_magic) != FDT_MAGIC) {
		fprintf(stderr, "%s error: not a FIT file\n", __func__);
		exit(EXIT_FAILURE);
	}

	fit_size = fdt_totalsize(&image_header);
	fit_size = (fit_size + 3) & ~3;

	fit_size = ALIGN(fit_size, ALIGN_SIZE);

	ret = lseek(fd, fit_offset + fit_size, SEEK_SET);
	if (ret < 0) {
		fprintf(stderr, "lseek fd fail for fit\n");
		exit(EXIT_FAILURE);
	}

	/*
	 * ep is the u-boot entry. SPL loads the FIT before the u-boot
	 * address. 0x2000 is for CSF_SIZE
	 */
	load_addr = (ep - (fit_size + CSF_SIZE) - 512 - align_len) &
		~align_len;

	flash_header_v2_t ivt_header = { { 0xd1, 0x2000, 0x40 },
		load_addr, 0, 0, 0,
		(load_addr + fit_size),
		(load_addr + fit_size + 0x20),
		0 };

	if (write(fd, &ivt_header, sizeof(flash_header_v2_t)) !=
	    sizeof(flash_header_v2_t)) {
		fprintf(stderr, "IVT writing error on fit image\n");
		exit(EXIT_FAILURE);
	}

	*fit_load_addr = load_addr;

	return fit_offset + fit_size;
}

static void dump_header_v2(imx_header_v3_t *imx_header, int index)
{
	const char *ivt_name[2] = {"HDMI FW", "LOADER IMAGE"};

	fprintf(stdout, "========= IVT HEADER [%s] =========\n",
		ivt_name[index]);
	fprintf(stdout, "header.tag: \t\t0x%x\n",
		imx_header[index].fhdr.header.tag);
	fprintf(stdout, "header.length: \t\t0x%x\n",
		imx_header[index].fhdr.header.length);
	fprintf(stdout, "header.version: \t0x%x\n",
		imx_header[index].fhdr.header.version);
	fprintf(stdout, "entry: \t\t\t0x%x\n",
		imx_header[index].fhdr.entry);
	fprintf(stdout, "reserved1: \t\t0x%x\n",
		imx_header[index].fhdr.reserved1);
	fprintf(stdout, "dcd_ptr: \t\t0x%x\n",
		imx_header[index].fhdr.dcd_ptr);
	fprintf(stdout, "boot_data_ptr: \t\t0x%x\n",
		imx_header[index].fhdr.boot_data_ptr);
	fprintf(stdout, "self: \t\t\t0x%x\n",
		imx_header[index].fhdr.self);
	fprintf(stdout, "csf: \t\t\t0x%x\n",
		imx_header[index].fhdr.csf);
	fprintf(stdout, "reserved2: \t\t0x%x\n",
		imx_header[index].fhdr.reserved2);

	fprintf(stdout, "boot_data.start: \t0x%x\n",
		imx_header[index].boot_data.start);
	fprintf(stdout, "boot_data.size: \t0x%x\n",
		imx_header[index].boot_data.size);
	fprintf(stdout, "boot_data.plugin: \t0x%x\n",
		imx_header[index].boot_data.plugin);
}

void build_image(int ofd)
{
	int file_off, header_hdmi_off = 0, header_image_off;
	int hdmi_fd, ap_fd, sld_fd;
	uint32_t sld_load_addr = 0;
	uint32_t csf_off, sld_csf_off = 0;
	int ret;
	struct stat sbuf;

	if (!ap_img) {
		fprintf(stderr, "No LOADER image specificed\n");
		exit(EXIT_FAILURE);
	}

	file_off = 0;

	if (signed_hdmi) {
		header_hdmi_off = file_off + ivt_offset;

		hdmi_fd = open(signed_hdmi, O_RDONLY | O_BINARY);
		if (hdmi_fd < 0) {
			fprintf(stderr, "%s: Can't open: %s\n",
				signed_hdmi, strerror(errno));
			exit(EXIT_FAILURE);
		}

		if (fstat(hdmi_fd, &sbuf) < 0) {
			fprintf(stderr, "%s: Can't stat: %s\n",
				signed_hdmi, strerror(errno));
			exit(EXIT_FAILURE);
		}
		close(hdmi_fd);

		/*
		 * Aligned to 104KB = 92KB FW image + 0x8000
		 * (IVT and alignment) + 0x4000 (second IVT + CSF)
		 */
		file_off += ALIGN(sbuf.st_size,
				  HDMI_FW_SIZE + 0x2000 + 0x1000);
	}

	header_image_off = file_off + ivt_offset;

	ap_fd = open(ap_img, O_RDONLY | O_BINARY);
	if (ap_fd < 0) {
		fprintf(stderr, "%s: Can't open: %s\n",
			ap_img, strerror(errno));
		exit(EXIT_FAILURE);
	}
	if (fstat(ap_fd, &sbuf) < 0) {
		fprintf(stderr, "%s: Can't stat: %s\n",
			ap_img, strerror(errno));
		exit(EXIT_FAILURE);
	}
	close(ap_fd);

	imx_header[IMAGE_IVT_ID].fhdr.header.tag = IVT_HEADER_TAG; /* 0xD1 */
	imx_header[IMAGE_IVT_ID].fhdr.header.length =
		cpu_to_be16(sizeof(flash_header_v2_t));
	imx_header[IMAGE_IVT_ID].fhdr.header.version = IVT_VERSION_V3; /* 0x41 */
	imx_header[IMAGE_IVT_ID].fhdr.entry = ap_start_addr;
	imx_header[IMAGE_IVT_ID].fhdr.self = ap_start_addr -
		sizeof(imx_header_v3_t);
	imx_header[IMAGE_IVT_ID].fhdr.dcd_ptr = 0;
	imx_header[IMAGE_IVT_ID].fhdr.boot_data_ptr =
		imx_header[IMAGE_IVT_ID].fhdr.self +
		offsetof(imx_header_v3_t, boot_data);
	imx_header[IMAGE_IVT_ID].boot_data.start =
		imx_header[IMAGE_IVT_ID].fhdr.self - ivt_offset;
	imx_header[IMAGE_IVT_ID].boot_data.size =
		ALIGN(sbuf.st_size + sizeof(imx_header_v3_t) + ivt_offset,
		      sector_size);

	image_off = header_image_off + sizeof(imx_header_v3_t);
	file_off +=  imx_header[IMAGE_IVT_ID].boot_data.size;

	imx_header[IMAGE_IVT_ID].boot_data.plugin = 0;
	imx_header[IMAGE_IVT_ID].fhdr.csf =
		imx_header[IMAGE_IVT_ID].boot_data.start +
		imx_header[IMAGE_IVT_ID].boot_data.size;

	imx_header[IMAGE_IVT_ID].boot_data.size += CSF_SIZE; /* 8K region dummy CSF */

	csf_off = file_off;
	file_off += CSF_SIZE;

	/* Second boot loader image */
	if (sld_img) {
		if (!using_fit) {
			fprintf(stderr, "Not support no fit\n");
			exit(EXIT_FAILURE);
		} else {
			sld_header_off = sld_src_off - rom_image_offset;
			/*
			 * Record the second bootloader relative offset in
			 * image's IVT reserved1
			 */
			imx_header[IMAGE_IVT_ID].fhdr.reserved1 =
				sld_header_off - header_image_off;
			sld_fd = open(sld_img, O_RDONLY | O_BINARY);
			if (sld_fd < 0) {
				fprintf(stderr, "%s: Can't open: %s\n",
					sld_img, strerror(errno));
				exit(EXIT_FAILURE);
			}

			if (fstat(sld_fd, &sbuf) < 0) {
				fprintf(stderr, "%s: Can't stat: %s\n",
					sld_img, strerror(errno));
				exit(EXIT_FAILURE);
			}

			close(sld_fd);

			file_off = sld_header_off;
			file_off += sbuf.st_size + sizeof(image_header_t);
		}
	}

	if (signed_hdmi) {
		header_hdmi_off -= ivt_offset;
		ret = lseek(ofd, header_hdmi_off, SEEK_SET);
		if (ret < 0) {
			fprintf(stderr, "lseek ofd fail for hdmi\n");
			exit(EXIT_FAILURE);
		}

		/* The signed HDMI FW has 0x400 IVT offset, need remove it */
		copy_file(ofd, signed_hdmi, 0, header_hdmi_off, 0x400);
	}

	/* Main Image */
	header_image_off -= ivt_offset;
	image_off -= ivt_offset;
	ret = lseek(ofd, header_image_off, SEEK_SET);
	if (ret < 0) {
		fprintf(stderr, "lseek ofd fail\n");
		exit(EXIT_FAILURE);
	}

	/* Write image header */
	if (write(ofd, &imx_header[IMAGE_IVT_ID], sizeof(imx_header_v3_t)) !=
	    sizeof(imx_header_v3_t)) {
		fprintf(stderr, "error writing image hdr\n");
		exit(1);
	}

	copy_file(ofd, ap_img, 0, image_off, 0);

	csf_off -= ivt_offset;
	fill_zero(ofd, CSF_SIZE, csf_off);

	if (sld_img) {
		sld_header_off -= ivt_offset;
		ret = lseek(ofd, sld_header_off, SEEK_SET);
		if (ret < 0) {
			fprintf(stderr, "lseek ofd fail for sld_img\n");
			exit(EXIT_FAILURE);
		}

		/* Write image header */
		if (!using_fit) {
			/* TODO */
		} else {
			copy_file(ofd, sld_img, 0, sld_header_off, 0);
			sld_csf_off =
				generate_ivt_for_fit(ofd, sld_header_off,
						     sld_start_addr,
						     &sld_load_addr) + 0x20;
		}
	}

	if (!signed_hdmi)
		dump_header_v2(imx_header, 0);
	dump_header_v2(imx_header, 1);

	fprintf(stdout, "========= OFFSET dump =========");
	if (signed_hdmi) {
		fprintf(stdout, "\nSIGNED HDMI FW:\n");
		fprintf(stdout, " header_hdmi_off \t0x%x\n",
			header_hdmi_off);
	}

	fprintf(stdout, "\nLoader IMAGE:\n");
	fprintf(stdout, " header_image_off \t0x%x\n image_off \t\t0x%x\n csf_off \t\t0x%x\n",
		header_image_off, image_off, csf_off);
	fprintf(stdout, " spl hab block: \t0x%x 0x%x 0x%x\n",
		imx_header[IMAGE_IVT_ID].fhdr.self, header_image_off,
		csf_off - header_image_off);

	fprintf(stdout, "\nSecond Loader IMAGE:\n");
	fprintf(stdout, " sld_header_off \t0x%x\n",
		sld_header_off);
	fprintf(stdout, " sld_csf_off \t\t0x%x\n",
		sld_csf_off);
	fprintf(stdout, " sld hab block: \t0x%x 0x%x 0x%x\n",
		sld_load_addr, sld_header_off, sld_csf_off - sld_header_off);
}

int imx8mimage_copy_image(int outfd, struct image_tool_params *mparams)
{
	/*
	 * SECO FW is a container image, this is to calculate the
	 * 2nd container offset.
	 */
	fprintf(stdout, "parsing %s\n", mparams->imagename);
	parse_cfg_file(mparams->imagename);

	build_image(outfd);

	return 0;
}

/*
 * imx8mimage parameters
 */
U_BOOT_IMAGE_TYPE(
	imx8mimage,
	"NXP i.MX8M Boot Image support",
	0,
	NULL,
	imx8mimage_check_params,
	NULL,
	imx8mimage_print_header,
	imx8mimage_set_header,
	NULL,
	imx8mimage_check_image_types,
	NULL,
	NULL
);
