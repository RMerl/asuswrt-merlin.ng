// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2014-2015, Bin Meng <bmeng.cn@gmail.com>
 */

#include <common.h>
#include <command.h>
#include <asm/fsp/fsp_support.h>

DECLARE_GLOBAL_DATA_PTR;

static char *hob_type[] = {
	"reserved",
	"Hand-off",
	"Mem Alloc",
	"Res Desc",
	"GUID Ext",
	"FV",
	"CPU",
	"Mem Pool",
	"reserved",
	"FV2",
	"Load PEIM",
	"Capsule",
};

static int do_hdr(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	struct fsp_header *hdr = find_fsp_header();
	u32 img_addr = hdr->img_base;
	char *sign = (char *)&hdr->sign;
	int i;

	printf("FSP    : binary 0x%08x, header 0x%08x\n",
	       CONFIG_FSP_ADDR, (int)hdr);
	printf("Header : sign ");
	for (i = 0; i < sizeof(hdr->sign); i++)
		printf("%c", *sign++);
	printf(", size %d, rev %d\n", hdr->hdr_len, hdr->hdr_rev);
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
	printf(", addr 0x%08x, size %d\n", img_addr, hdr->img_size);
	if (hdr->hdr_rev == FSP_HEADER_REVISION_2) {
		printf("GFX    :%ssupported\n",
		       hdr->img_attr & FSP_ATTR_GRAPHICS_SUPPORT ? " " : " un");
	}
	printf("VPD    : addr 0x%08x, size %d\n",
	       hdr->cfg_region_off + img_addr, hdr->cfg_region_size);
	printf("\nNumber of APIs Supported : %d\n", hdr->api_num);
	printf("\tTempRamInit : 0x%08x\n", hdr->fsp_tempram_init + img_addr);
	printf("\tFspInit     : 0x%08x\n", hdr->fsp_init + img_addr);
	printf("\tFspNotify   : 0x%08x\n", hdr->fsp_notify + img_addr);
	if (hdr->hdr_rev == FSP_HEADER_REVISION_2) {
		printf("\tMemoryInit  : 0x%08x\n",
		       hdr->fsp_mem_init + img_addr);
		printf("\tTempRamExit : 0x%08x\n",
		       hdr->fsp_tempram_exit + img_addr);
		printf("\tSiliconInit : 0x%08x\n",
		       hdr->fsp_silicon_init + img_addr);
	}

	return 0;
}

static int do_hob(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	const struct hob_header *hdr;
	uint type;
	char *desc;
	int i = 0;

	hdr = gd->arch.hob_list;

	printf("HOB list address: 0x%08x\n\n", (unsigned int)hdr);

	printf("#  | Address  | Type      | Len  | ");
	printf("%42s\n", "GUID");
	printf("---|----------|-----------|------|-");
	printf("------------------------------------------\n");
	while (!end_of_hob(hdr)) {
		printf("%02x | %08x | ", i, (unsigned int)hdr);
		type = hdr->type;
		if (type == HOB_TYPE_UNUSED)
			desc = "*Unused*";
		else if (type == HOB_TYPE_EOH)
			desc = "*EOH*";
		else if (type >= 0 && type <= ARRAY_SIZE(hob_type))
			desc = hob_type[type];
		else
			desc = "*Invalid*";
		printf("%-9s | %04x | ", desc, hdr->len);

		if (type == HOB_TYPE_MEM_ALLOC || type == HOB_TYPE_RES_DESC ||
		    type == HOB_TYPE_GUID_EXT) {
			struct efi_guid *guid = (struct efi_guid *)(hdr + 1);
			int j;

			printf("%08x-%04x-%04x", guid->data1,
			       guid->data2, guid->data3);
			for (j = 0; j < ARRAY_SIZE(guid->data4); j++)
				printf("-%02x", guid->data4[j]);
		} else {
			printf("%42s", "Not Available");
		}
		printf("\n");
		hdr = get_next_hob(hdr);
		i++;
	}

	return 0;
}

static cmd_tbl_t fsp_commands[] = {
	U_BOOT_CMD_MKENT(hdr, 0, 1, do_hdr, "", ""),
	U_BOOT_CMD_MKENT(hob, 0, 1, do_hob, "", ""),
};

static int do_fsp(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	cmd_tbl_t *fsp_cmd;
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
	"hdr - Print FSP header information\n"
	"fsp hob - Print FSP Hand-Off Block (HOB) information"
);
