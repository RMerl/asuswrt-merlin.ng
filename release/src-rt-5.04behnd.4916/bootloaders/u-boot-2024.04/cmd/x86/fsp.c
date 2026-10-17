// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2014-2015, Bin Meng <bmeng.cn@gmail.com>
 */

#include <common.h>
#include <command.h>
#include <asm/fsp/fsp_support.h>
#include <asm/global_data.h>

DECLARE_GLOBAL_DATA_PTR;

static int do_hdr(struct cmd_tbl *cmdtp, int flag, int argc, char *const argv[])
{
	struct fsp_header *hdr;
	u32 img_addr;
	char *sign;
	uint addr;
	int i;

#ifdef CONFIG_FSP_VERSION2
	/*
	 * Only FSP-S is displayed. FSP-M was used in SPL but may not still be
	 * around, and we didn't keep a pointer to it.
	 */
	hdr = gd->arch.fsp_s_hdr;
	img_addr = hdr->img_base;
	addr = img_addr;
#else
	addr = CONFIG_FSP_ADDR;
	hdr = fsp_find_header();
	img_addr = hdr->img_base;
#endif
	sign = (char *)&hdr->sign;

	printf("FSP    : binary %08x, header %08x\n", addr, (int)hdr);
	printf("Header : sign ");
	for (i = 0; i < sizeof(hdr->sign); i++)
		printf("%c", *sign++);
	printf(", size %x, rev %d\n", hdr->hdr_len, hdr->hdr_rev);
	printf("Image  : rev ");
	if (hdr->hdr_rev == FSP_HEADER_REVISION_1) {
		printf("%d.%d",
		       (hdr->img_rev >> 8) & 0xff, hdr->img_rev & 0xff);
	} else {
		printf("%d.%d.%d.%d",
		       (hdr->img_rev >> 24) & 0xff, (hdr->img_rev >> 16) & 0xff,
		       (hdr->img_rev >> 8) & 0xff, hdr->img_rev & 0xff);
	}
	printf(", id ");
	for (i = 0; i < ARRAY_SIZE(hdr->img_id); i++)
		printf("%c", hdr->img_id[i]);
	printf(", addr %08x, size %x\n", img_addr, hdr->img_size);
	if (hdr->hdr_rev >= FSP_HEADER_REVISION_1) {
		printf("GFX    :%ssupported\n",
		       hdr->img_attr & FSP_ATTR_GRAPHICS_SUPPORT ? " " : " un");
	}
	printf("VPD    : addr %08x, size %x\n",
	       hdr->cfg_region_off + img_addr, hdr->cfg_region_size);
	if (hdr->hdr_rev <= FSP_HEADER_REVISION_2)
		printf("\nNumber of APIs Supported : %d\n", hdr->api_num);
	if (hdr->fsp_tempram_init)
		printf("\tTempRamInit : %08x\n",
		       hdr->fsp_tempram_init + img_addr);
	if (hdr->fsp_init)
		printf("\tFspInit     : %08x\n", hdr->fsp_init + img_addr);
	if (hdr->fsp_notify)
		printf("\tFspNotify   : %08x\n", hdr->fsp_notify + img_addr);
	if (hdr->hdr_rev >= FSP_HEADER_REVISION_1) {
		if (hdr->fsp_mem_init)
			printf("\tMemoryInit  : %08x\n",
			       hdr->fsp_mem_init + img_addr);
		if (hdr->fsp_tempram_exit)
			printf("\tTempRamExit : %08x\n",
			       hdr->fsp_tempram_exit + img_addr);
		if (hdr->fsp_silicon_init)
			printf("\tSiliconInit : %08x\n",
			       hdr->fsp_silicon_init + img_addr);
	}

	return 0;
}

static struct cmd_tbl fsp_commands[] = {
	U_BOOT_CMD_MKENT(hdr, 0, 1, do_hdr, "", ""),
};

static int do_fsp(struct cmd_tbl *cmdtp, int flag, int argc, char *const argv[])
{
	struct cmd_tbl *fsp_cmd;
	int ret;

	if (argc < 2)
		return CMD_RET_USAGE;
	fsp_cmd = find_cmd_tbl(argv[1], fsp_commands, ARRAY_SIZE(fsp_commands));
	argc -= 2;
	argv += 2;
	if (!fsp_cmd || argc > fsp_cmd->maxargs)
		return CMD_RET_USAGE;

	ret = fsp_cmd->cmd(fsp_cmd, flag, argc, argv);

	return cmd_process_error(fsp_cmd, ret);
}

U_BOOT_CMD(
	fsp,	2,	1,	do_fsp,
	"Show Intel Firmware Support Package (FSP) related information",
	"hdr - Print FSP header information"
);
