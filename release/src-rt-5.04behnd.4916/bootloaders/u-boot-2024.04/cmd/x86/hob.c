// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2014-2015, Bin Meng <bmeng.cn@gmail.com>
 */

#include <common.h>
#include <command.h>
#include <efi.h>
#include <uuid.h>
#include <asm/global_data.h>
#include <asm/hob.h>
#include <asm/fsp/fsp_hob.h>

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

static char *res_type[] = {
	"System",
	"Memory-mapped I/O",
	"I/O",
	"Firmware device",
	"Memory-mapped I/O port",
	"Reserved",
	"I/O reserved",
};

static struct guid_name {
	efi_guid_t guid;
	const char *name;
} guid_name[] = {
	{ FSP_HOB_RESOURCE_OWNER_TSEG_GUID, "TSEG" },
	{ FSP_HOB_RESOURCE_OWNER_FSP_GUID, "FSP" },
	{ FSP_HOB_RESOURCE_OWNER_SMM_PEI_SMRAM_GUID, "SMM PEI SMRAM" },
	{ FSP_NON_VOLATILE_STORAGE_HOB_GUID, "NVS" },
	{ FSP_VARIABLE_NV_DATA_HOB_GUID, "Variable NVS" },
	{ FSP_GRAPHICS_INFO_HOB_GUID, "Graphics info" },
	{ FSP_HOB_RESOURCE_OWNER_PCD_DATABASE_GUID1, "PCD database ea" },
	{ FSP_HOB_RESOURCE_OWNER_PCD_DATABASE_GUID2, "PCD database 9b" },
	{ FSP_HOB_RESOURCE_OWNER_PEIM_DXE_GUID, "PEIM Init DXE" },
	{ FSP_HOB_RESOURCE_OWNER_ALLOC_STACK_GUID, "Alloc stack" },
	{ FSP_HOB_RESOURCE_OWNER_SMBIOS_MEMORY_GUID, "SMBIOS memory" },
	{ {}, "zero-guid" },
	{}
};

static const char *guid_to_name(const efi_guid_t *guid)
{
	struct guid_name *entry;

	for (entry = guid_name; entry->name; entry++) {
		if (!guidcmp(guid, &entry->guid))
			return entry->name;
	}

	return NULL;
}

static void show_hob_details(const struct hob_header *hdr)
{
	const void *ptr = hdr;

	switch (hdr->type) {
	case HOB_TYPE_RES_DESC: {
		const struct hob_res_desc *res = ptr;
		const char *typename;

		typename = res->type >= RES_SYS_MEM && res->type <= RES_MAX_MEM_TYPE ?
			res_type[res->type] : "unknown";

		printf("     base = %08llx, len = %08llx, end = %08llx, type = %d (%s)\n\n",
		       res->phys_start, res->len, res->phys_start + res->len,
		       res->type, typename);
		break;
	}
	}
}

static int do_hob(struct cmd_tbl *cmdtp, int flag, int argc, char *const argv[])
{
	const struct hob_header *hdr;
	uint type;
	char *desc;
	int i = 0;
	efi_guid_t *guid;
	char uuid[UUID_STR_LEN + 1];
	bool verbose = false;
	int seq = -1;	/* Show all by default */

	argc--;
	argv++;
	if (argc) {
		if (!strcmp("-v", *argv)) {
			verbose = true;
			argc--;
			argv++;
		}
		if (argc)
			seq = simple_strtol(*argv, NULL, 16);
	}
	hdr = gd->arch.hob_list;

	printf("HOB list address: 0x%08x\n\n", (unsigned int)hdr);

	printf("#  | Address  | Type      | Len  | ");
	printf("%36s\n", "GUID");
	printf("---|----------|-----------|------|-");
	printf("------------------------------------\n");
	for (i = 0; !end_of_hob(hdr); i++, hdr = get_next_hob(hdr)) {
		if (seq != -1 && seq != i)
			continue;
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
			const char *name;

			guid = (efi_guid_t *)(hdr + 1);
			name = guid_to_name(guid);
			if (!name) {
				uuid_bin_to_str(guid->b, uuid,
						UUID_STR_FORMAT_GUID);
				name = uuid;
			}
			printf("%36s", name);
		} else {
			printf("%36s", "Not Available");
		}
		printf("\n");
		if (verbose)
			show_hob_details(hdr);
	}

	return 0;
}

U_BOOT_CMD(hob, 3, 1, do_hob,
	   "[-v] [seq]  Print Hand-Off Block (HOB) information",
	   "   -v  - Show detailed HOB information where available\n"
	   "   seq - Record # to show (all by default)"
);
