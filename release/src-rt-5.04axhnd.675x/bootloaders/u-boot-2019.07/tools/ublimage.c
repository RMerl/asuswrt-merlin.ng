// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2011
 * Heiko Schocher, DENX Software Engineering, hs@denx.de.
 *
 * Based on:
 * (C) Copyright 2009
 * Stefano Babic, DENX Software Engineering, sbabic@denx.de.
 *
 * (C) Copyright 2008
 * Marvell Semiconductor <www.marvell.com>
 * Written-by: Prafulla Wadaskar <prafulla@marvell.com>
 */

#include "imagetool.h"
#include <image.h>
#include "ublimage.h"

/*
 * Supported commands for configuration file
 */
static table_entry_t ublimage_cmds[] = {
	{CMD_BOOT_MODE,	"MODE",		"UBL special modes", },
	{CMD_ENTRY,	"ENTRY",	"Entry point addr for bootloader", },
	{CMD_PAGE,	"PAGES",
		"number of pages (size of bootloader)", },
	{CMD_ST_BLOCK,	"START_BLOCK",
		"block number where bootloader is present", },
	{CMD_ST_PAGE,	"START_PAGE",
		"page number where bootloader is present", },
	{CMD_LD_ADDR,	"LD_ADDR",
		"load addr", },
	{-1,		"",		"", },
};

/*
 * Supported Boot options for configuration file
 * this is needed to set the correct flash offset
 */
static table_entry_t ublimage_bootops[] = {
	{UBL_MAGIC_SAFE,	"safe",	"Safe boot mode",	},
	{-1,			"",	"Invalid",		},
};

static struct ubl_header ublimage_header;

static uint32_t get_cfg_value(char *token, char *name,  int linenr)
{
	char *endptr;
	uint32_t value;

	errno = 0;
	value = strtoul(token, &endptr, 16);
	if (errno || (token == endptr)) {
		fprintf(stderr, "Error: %s[%d] - Invalid hex data(%s)\n",
			name,  linenr, token);
		exit(EXIT_FAILURE);
	}
	return value;
}

static void print_hdr(struct ubl_header *ubl_hdr)
{
	printf("Image Type : Davinci UBL Boot Image\n");
	printf("UBL magic  : %08x\n", ubl_hdr->magic);
	printf("Entry Point: %08x\n", ubl_hdr->entry);
	printf("nr of pages: %08x\n", ubl_hdr->pages);
	printf("start block: %08x\n", ubl_hdr->block);
	printf("start page : %08x\n", ubl_hdr->page);
}

static void parse_cfg_cmd(struct ubl_header *ublhdr, int32_t cmd, char *token,
				char *name, int lineno, int fld, int dcd_len)
{
	static int cmd_ver_first = ~0;

	switch (cmd) {
	case CMD_BOOT_MODE:
		ublhdr->magic = get_table_entry_id(ublimage_bootops,
					"ublimage special boot mode", token);
		if (ublhdr->magic == -1) {
			fprintf(stderr, "Error: %s[%d] -Invalid boot mode"
				"(%s)\n", name, lineno, token);
			exit(EXIT_FAILURE);
		}
		ublhdr->magic += UBL_MAGIC_BASE;
		if (unlikely(cmd_ver_first != 1))
			cmd_ver_first = 0;
		break;
	case CMD_ENTRY:
		ublhdr->entry = get_cfg_value(token, name, lineno);
		break;
	case CMD_PAGE:
		ublhdr->pages = get_cfg_value(token, name, lineno);
		break;
	case CMD_ST_BLOCK:
		ublhdr->block = get_cfg_value(token, name, lineno);
		break;
	case CMD_ST_PAGE:
		ublhdr->page = get_cfg_value(token, name, lineno);
		break;
	case CMD_LD_ADDR:
		ublhdr->pll_m = get_cfg_value(token, name, lineno);
		break;
	}
}

static void parse_cfg_fld(struct ubl_header *ublhdr, int32_t *cmd,
		char *token, char *name, int lineno, int fld, int *dcd_len)
{

	switch (fld) {
	case CFG_COMMAND:
		*cmd = get_table_entry_id(ublimage_cmds,
			"ublimage commands", token);
		if (*cmd < 0) {
			fprintf(stderr, "Error: %s[%d] - Invalid command"
			"(%s)\n", name, lineno, token);
			exit(EXIT_FAILURE);
		}
		break;
	case CFG_REG_VALUE:
		parse_cfg_cmd(ublhdr, *cmd, token, name, lineno, fld, *dcd_len);
		break;
	default:
		break;
	}
}
static uint32_t parse_cfg_file(struct ubl_header *ublhdr, char *name)
{
	FILE *fd = NULL;
	char *line = NULL;
	char *token, *saveptr1, *saveptr2;
	int lineno = 0;
	int	i;
	char *ptr = (char *)ublhdr;
	int fld;
	size_t len;
	int dcd_len = 0;
	int32_t cmd;
	int ublhdrlen = sizeof(struct ubl_header);

	fd = fopen(name, "r");
	if (fd == 0) {
		fprintf(stderr, "Error: %s - Can't open DCD file\n", name);
		exit(EXIT_FAILURE);
	}

	/* Fill header with 0xff */
	for (i = 0; i < ublhdrlen; i++) {
		*ptr = 0xff;
		ptr++;
	}

	/*
	 * Very simple parsing, line starting with # are comments
	 * and are dropped
	 */
	while ((getline(&line, &len, fd)) > 0) {
		lineno++;

		token = strtok_r(line, "\r\n", &saveptr1);
		if (token == NULL)
			continue;

		/* Check inside the single line */
		for (fld = CFG_COMMAND, cmd = CMD_INVALID,
				line = token; ; line = NULL, fld++) {
			token = strtok_r(line, " \t", &saveptr2);
			if (token == NULL)
				break;

			/* Drop all text starting with '#' as comments */
			if (token[0] == '#')
				break;

			parse_cfg_fld(ublhdr, &cmd, token, name,
					lineno, fld, &dcd_len);
		}
	}
	fclose(fd);

	return dcd_len;
}

static int ublimage_check_image_types(uint8_t type)
{
	if (type == IH_TYPE_UBLIMAGE)
		return EXIT_SUCCESS;
	else
		return EXIT_FAILURE;
}

static int ublimage_verify_header(unsigned char *ptr, int image_size,
			struct image_tool_params *params)
{
	struct ubl_header *ubl_hdr = (struct ubl_header *)ptr;

	if ((ubl_hdr->magic & 0xFFFFFF00) != UBL_MAGIC_BASE)
		return -1;

	return 0;
}

static void ublimage_print_header(const void *ptr)
{
	struct ubl_header *ubl_hdr = (struct ubl_header *) ptr;

	print_hdr(ubl_hdr);
}

static void ublimage_set_header(void *ptr, struct stat *sbuf, int ifd,
				struct image_tool_params *params)
{
	struct ubl_header *ublhdr = (struct ubl_header *)ptr;

	/* Parse configuration file */
	parse_cfg_file(ublhdr, params->imagename);
}

int ublimage_check_params(struct image_tool_params *params)
{
	if (!params)
		return CFG_INVALID;
	if (!strlen(params->imagename)) {
		fprintf(stderr, "Error: %s - Configuration file not"
			"specified, it is needed for ublimage generation\n",
			params->cmdname);
		return CFG_INVALID;
	}
	/*
	 * Check parameters:
	 * XIP is not allowed and verify that incompatible
	 * parameters are not sent at the same time
	 * For example, if list is required a data image must not be provided
	 */
	return	(params->dflag && (params->fflag || params->lflag)) ||
		(params->fflag && (params->dflag || params->lflag)) ||
		(params->lflag && (params->dflag || params->fflag)) ||
		(params->xflag) || !(strlen(params->imagename));
}

/*
 * ublimage parameters
 */
U_BOOT_IMAGE_TYPE(
	ublimage,
	"Davinci UBL boot support",
	sizeof(struct ubl_header),
	(void *)&ublimage_header,
	ublimage_check_params,
	ublimage_verify_header,
	ublimage_print_header,
	ublimage_set_header,
	NULL,
	ublimage_check_image_types,
	NULL,
	NULL
);
