// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2011
 * Stefano Babic, DENX Software Engineering, sbabic@denx.de.
 */

#include "imagetool.h"
#include "aisimage.h"
#include <image.h>

#define IS_FNC_EXEC(c)	(cmd_table[c].AIS_cmd == AIS_CMD_FNLOAD)
#define WORD_ALIGN0	4
#define WORD_ALIGN(len) (((len)+WORD_ALIGN0-1) & ~(WORD_ALIGN0-1))
#define MAX_CMD_BUFFER	4096

static uint32_t ais_img_size;

/*
 * Supported commands for configuration file
 */
static table_entry_t aisimage_cmds[] = {
	{CMD_DATA,		"DATA",		"Reg Write Data"},
	{CMD_FILL,		"FILL",		"Fill range with pattern"},
	{CMD_CRCON,		"CRCON",	"CRC Enable"},
	{CMD_CRCOFF,		"CRCOFF",	"CRC Disable"},
	{CMD_CRCCHECK,		"CRCCHECK",	"CRC Validate"},
	{CMD_JMPCLOSE,		"JMPCLOSE",	"Jump & Close"},
	{CMD_JMP,		"JMP",		"Jump"},
	{CMD_SEQREAD,		"SEQREAD",	"Sequential read"},
	{CMD_PLL0,		"PLL0",		"PLL0"},
	{CMD_PLL1,		"PLL1",		"PLL1"},
	{CMD_CLK,		"CLK",		"Clock configuration"},
	{CMD_DDR2,		"DDR2",		"DDR2 Configuration"},
	{CMD_EMIFA,		"EMIFA",	"EMIFA"},
	{CMD_EMIFA_ASYNC,	"EMIFA_ASYNC",	"EMIFA Async"},
	{CMD_PLL,		"PLL",		"PLL & Clock configuration"},
	{CMD_PSC,		"PSC",		"PSC setup"},
	{CMD_PINMUX,		"PINMUX",	"Pinmux setup"},
	{CMD_BOOTTABLE,		"BOOT_TABLE",	"Boot table command"},
	{-1,			"",		""},
};

static struct ais_func_exec {
	uint32_t index;
	uint32_t argcnt;
} ais_func_table[] = {
	[CMD_PLL0] = {0, 2},
	[CMD_PLL1] = {1, 2},
	[CMD_CLK] = {2, 1},
	[CMD_DDR2] = {3, 8},
	[CMD_EMIFA] = {4, 5},
	[CMD_EMIFA_ASYNC] = {5, 5},
	[CMD_PLL] = {6, 3},
	[CMD_PSC] = {7, 1},
	[CMD_PINMUX] = {8, 3}
};

static struct cmd_table_t {
	uint32_t nargs;
	uint32_t AIS_cmd;
} cmd_table[] = {
	[CMD_FILL]	 =	{	4,	AIS_CMD_FILL},
	[CMD_CRCON]	=	{	0,	AIS_CMD_ENCRC},
	[CMD_CRCOFF]	=	{	0,	AIS_CMD_DISCRC},
	[CMD_CRCCHECK]	=	{	2,	AIS_CMD_ENCRC},
	[CMD_JMPCLOSE]	=	{	1,	AIS_CMD_JMPCLOSE},
	[CMD_JMP]	=	{	1,	AIS_CMD_JMP},
	[CMD_SEQREAD]	=	{	0,	AIS_CMD_SEQREAD},
	[CMD_PLL0]	=	{	2,	AIS_CMD_FNLOAD},
	[CMD_PLL1]	=	{	2,	AIS_CMD_FNLOAD},
	[CMD_CLK]	=	{	1,	AIS_CMD_FNLOAD},
	[CMD_DDR2]	=	{	8,	AIS_CMD_FNLOAD},
	[CMD_EMIFA]	=	{	5,	AIS_CMD_FNLOAD},
	[CMD_EMIFA_ASYNC] =	{	5,	AIS_CMD_FNLOAD},
	[CMD_PLL]	=	{	3,	AIS_CMD_FNLOAD},
	[CMD_PSC]	=	{	1,	AIS_CMD_FNLOAD},
	[CMD_PINMUX]	=	{	3,	AIS_CMD_FNLOAD},
	[CMD_BOOTTABLE]	=	{	4,	AIS_CMD_BOOTTBL},
};

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

static int get_ais_table_id(uint32_t *ptr)
{

	int i;
	int func_no;

	for (i = 0; i < ARRAY_SIZE(cmd_table); i++) {
		if (*ptr == cmd_table[i].AIS_cmd) {
			if (cmd_table[i].AIS_cmd != AIS_CMD_FNLOAD)
				return i;

			func_no = ((struct ais_cmd_func *)ptr)->func_args
				& 0xFFFF;
			if (func_no == ais_func_table[i].index)
				return i;
		}
	}

	return -1;
}

static void aisimage_print_header(const void *hdr)
{
	struct ais_header *ais_hdr = (struct ais_header *)hdr;
	uint32_t *ptr;
	struct ais_cmd_load *ais_load;
	int id;

	if (ais_hdr->magic != AIS_MAGIC_WORD) {
		fprintf(stderr, "Error: - AIS Magic Number not found\n");
		return;
	}
	fprintf(stdout, "Image Type:   TI Davinci AIS Boot Image\n");
	fprintf(stdout, "AIS magic :   %08x\n", ais_hdr->magic);
	ptr = (uint32_t *)&ais_hdr->magic;
	ptr++;

	while (*ptr != AIS_CMD_JMPCLOSE) {
		/* Check if we find the image */
		if (*ptr == AIS_CMD_LOAD) {
			ais_load = (struct ais_cmd_load *)ptr;
			fprintf(stdout, "Image at  :   0x%08x size 0x%08x\n",
				ais_load->addr,
				ais_load->size);
			ptr = ais_load->data + ais_load->size / sizeof(*ptr);
			continue;
		}

		id = get_ais_table_id(ptr);
		if (id < 0) {
			fprintf(stderr, "Error: -  AIS Image corrupted\n");
			return;
		}
		fprintf(stdout, "AIS cmd   :   %s\n",
			get_table_entry_name(aisimage_cmds, NULL, id));
		ptr += cmd_table[id].nargs + IS_FNC_EXEC(id) + 1;
		if (((void *)ptr - hdr) > ais_img_size) {
			fprintf(stderr,
				"AIS Image not terminated by JMPCLOSE\n");
			return;
		}
	}
}

static uint32_t *ais_insert_cmd_header(uint32_t cmd, uint32_t nargs,
	uint32_t *parms, struct image_type_params *tparams,
	uint32_t *ptr)
{
	int i;

	*ptr++ = cmd_table[cmd].AIS_cmd;
	if (IS_FNC_EXEC(cmd))
		*ptr++ = ((nargs & 0xFFFF) << 16) + ais_func_table[cmd].index;

	/* Copy parameters */
	for (i = 0; i < nargs; i++)
		*ptr++ = cpu_to_le32(parms[i]);

	return ptr;

}

static uint32_t *ais_alloc_buffer(struct image_tool_params *params)
{
	int dfd;
	struct stat sbuf;
	char *datafile = params->datafile;
	uint32_t *ptr;

	dfd = open(datafile, O_RDONLY|O_BINARY);
	if (dfd < 0) {
		fprintf(stderr, "%s: Can't open %s: %s\n",
			params->cmdname, datafile, strerror(errno));
		exit(EXIT_FAILURE);
	}

	if (fstat(dfd, &sbuf) < 0) {
		fprintf(stderr, "%s: Can't stat %s: %s\n",
			params->cmdname, datafile, strerror(errno));
		exit(EXIT_FAILURE);
	}

	/*
	 * Place for header is allocated. The size is taken from
	 * the size of the datafile, that the ais_image_generate()
	 * will copy into the header. Copying the datafile
	 * is not left to the main program, because after the datafile
	 * the header must be terminated with the Jump & Close command.
	 */
	ais_img_size = WORD_ALIGN(sbuf.st_size) + MAX_CMD_BUFFER;
	ptr = (uint32_t *)malloc(WORD_ALIGN(sbuf.st_size) + MAX_CMD_BUFFER);
	if (!ptr) {
		fprintf(stderr, "%s: malloc return failure: %s\n",
			params->cmdname, strerror(errno));
		exit(EXIT_FAILURE);
	}

	close(dfd);

	return ptr;
}

static uint32_t *ais_copy_image(struct image_tool_params *params,
	uint32_t *aisptr)

{
	int dfd;
	struct stat sbuf;
	char *datafile = params->datafile;
	void *ptr;

	dfd = open(datafile, O_RDONLY|O_BINARY);
	if (dfd < 0) {
		fprintf(stderr, "%s: Can't open %s: %s\n",
			params->cmdname, datafile, strerror(errno));
		exit(EXIT_FAILURE);
	}

	if (fstat(dfd, &sbuf) < 0) {
		fprintf(stderr, "%s: Can't stat %s: %s\n",
			params->cmdname, datafile, strerror(errno));
		exit(EXIT_FAILURE);
	}

	ptr = mmap(0, sbuf.st_size, PROT_READ, MAP_SHARED, dfd, 0);
	*aisptr++ = AIS_CMD_LOAD;
	*aisptr++ = params->ep;
	*aisptr++ = sbuf.st_size;
	memcpy((void *)aisptr, ptr, sbuf.st_size);
	aisptr += WORD_ALIGN(sbuf.st_size) / sizeof(uint32_t);

	(void) munmap((void *)ptr, sbuf.st_size);
	(void) close(dfd);

	return aisptr;

}

static int aisimage_generate(struct image_tool_params *params,
	struct image_type_params *tparams)
{
	FILE *fd = NULL;
	char *line = NULL;
	char *token, *saveptr1, *saveptr2;
	int lineno = 0;
	int fld;
	size_t len;
	int32_t cmd;
	uint32_t nargs, cmd_parms[10];
	uint32_t value, size;
	char *name = params->imagename;
	uint32_t *aishdr;

	fd = fopen(name, "r");
	if (fd == 0) {
		fprintf(stderr,
			"Error: %s - Can't open AIS configuration\n", name);
		exit(EXIT_FAILURE);
	}

	/*
	 * the size of the header is variable and is computed
	 * scanning the configuration file.
	 */
	tparams->header_size = 0;

	/*
	 * Start allocating a buffer suitable for most command
	 * The buffer is then reallocated if it is too small
	 */
	aishdr = ais_alloc_buffer(params);
	tparams->hdr = aishdr;
	*aishdr++ = AIS_MAGIC_WORD;

	/* Very simple parsing, line starting with # are comments
	 * and are dropped
	 */
	while ((getline(&line, &len, fd)) > 0) {
		lineno++;

		token = strtok_r(line, "\r\n", &saveptr1);
		if (token == NULL)
			continue;

		/* Check inside the single line */
		line = token;
		fld = CFG_COMMAND;
		cmd = CMD_INVALID;
		nargs = 0;
		while (token != NULL) {
			token = strtok_r(line, " \t", &saveptr2);
			if (token == NULL)
				break;

			/* Drop all text starting with '#' as comments */
			if (token[0] == '#')
				break;

			switch (fld) {
			case CFG_COMMAND:
				cmd = get_table_entry_id(aisimage_cmds,
					"aisimage commands", token);
				if (cmd < 0) {
					fprintf(stderr,
					"Error: %s[%d] - Invalid command"
					"(%s)\n", name, lineno, token);

					exit(EXIT_FAILURE);
				}
				break;
			case CFG_VALUE:
				value = get_cfg_value(token, name, lineno);
				cmd_parms[nargs++] = value;
				if (nargs > cmd_table[cmd].nargs) {
					fprintf(stderr,
					 "Error: %s[%d] - too much arguments:"
						"(%s) for command %s\n", name,
						lineno, token,
						aisimage_cmds[cmd].sname);
					exit(EXIT_FAILURE);
				}
				break;
			}
			line = NULL;
			fld = CFG_VALUE;
		}
		if (cmd != CMD_INVALID) {
			/* Now insert the command into the header */
			aishdr = ais_insert_cmd_header(cmd, nargs, cmd_parms,
				tparams, aishdr);
		}

	}
	fclose(fd);

	aishdr = ais_copy_image(params, aishdr);

	/* Add Jmp & Close */
	*aishdr++ = AIS_CMD_JMPCLOSE;
	*aishdr++ = params->ep;

	size = (aishdr - (uint32_t *)tparams->hdr) * sizeof(uint32_t);
	tparams->header_size = size;

	return 0;
}

static int aisimage_check_image_types(uint8_t type)
{
	if (type == IH_TYPE_AISIMAGE)
		return EXIT_SUCCESS;
	else
		return EXIT_FAILURE;
}

static int aisimage_verify_header(unsigned char *ptr, int image_size,
			struct image_tool_params *params)
{
	struct ais_header *ais_hdr = (struct ais_header *)ptr;

	if (ais_hdr->magic != AIS_MAGIC_WORD)
		return -FDT_ERR_BADSTRUCTURE;

	/* Store the total size to remember in print_hdr */
	ais_img_size = image_size;

	return 0;
}

static void aisimage_set_header(void *ptr, struct stat *sbuf, int ifd,
				struct image_tool_params *params)
{
}

int aisimage_check_params(struct image_tool_params *params)
{
	if (!params)
		return CFG_INVALID;
	if (!strlen(params->imagename)) {
		fprintf(stderr, "Error: %s - Configuration file not specified, "
			"it is needed for aisimage generation\n",
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
 * aisimage parameters
 */
U_BOOT_IMAGE_TYPE(
	aisimage,
	"TI Davinci AIS Boot Image support",
	0,
	NULL,
	aisimage_check_params,
	aisimage_verify_header,
	aisimage_print_header,
	aisimage_set_header,
	NULL,
	aisimage_check_image_types,
	NULL,
	aisimage_generate
);
