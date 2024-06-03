// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2015 Google, Inc
 * Written by Simon Glass <sjg@chromium.org>
 */

#include <common.h>
#include <command.h>
#include <efi.h>
#include <errno.h>
#include <malloc.h>

static const char *const type_name[] = {
	"reserved",
	"loader_code",
	"loader_data",
	"bs_code",
	"bs_data",
	"rt_code",
	"rt_data",
	"conv",
	"unusable",
	"acpi_reclaim",
	"acpi_nvs",
	"io",
	"io_port",
	"pal_code",
};

static struct attr_info {
	u64 val;
	const char *name;
} mem_attr[] = {
	{ EFI_MEMORY_UC, "uncached" },
	{ EFI_MEMORY_WC, "write-coalescing" },
	{ EFI_MEMORY_WT, "write-through" },
	{ EFI_MEMORY_WB, "write-back" },
	{ EFI_MEMORY_UCE, "uncached & exported" },
	{ EFI_MEMORY_WP, "write-protect" },
	{ EFI_MEMORY_RP, "read-protect" },
	{ EFI_MEMORY_XP, "execute-protect" },
	{ EFI_MEMORY_NV, "non-volatile" },
	{ EFI_MEMORY_MORE_RELIABLE, "higher reliability" },
	{ EFI_MEMORY_RO, "read-only" },
	{ EFI_MEMORY_RUNTIME, "needs runtime mapping" }
};

/* Maximum different attribute values we can track */
#define ATTR_SEEN_MAX	30

static inline bool is_boot_services(int type)
{
	return type == EFI_LOADER_CODE || type == EFI_LOADER_DATA ||
		type == EFI_BOOT_SERVICES_CODE ||
		type == EFI_BOOT_SERVICES_DATA;
}

static int h_cmp_entry(const void *v1, const void *v2)
{
	const struct efi_mem_desc *desc1 = v1;
	const struct efi_mem_desc *desc2 = v2;
	int64_t diff = desc1->physical_start - desc2->physical_start;

	/*
	 * Manually calculate the difference to avoid sign loss in the 64-bit
	 * to 32-bit conversion
	 */
	return diff < 0 ? -1 : diff > 0 ? 1 : 0;
}

void *efi_build_mem_table(struct efi_entry_memmap *map, int size, bool skip_bs)
{
	struct efi_mem_desc *desc, *end, *base, *dest, *prev;
	int count;
	u64 addr;

	base = malloc(size + sizeof(*desc));
	if (!base) {
		debug("%s: Cannot allocate %#x bytes\n", __func__, size);
		return NULL;
	}
	end = (struct efi_mem_desc *)((ulong)map + size);
	count = ((ulong)end - (ulong)map->desc) / map->desc_size;
	memcpy(base, map->desc, (ulong)end - (ulong)map->desc);
	qsort(base, count, map->desc_size, h_cmp_entry);
	prev = NULL;
	addr = 0;
	dest = base;
	end = (struct efi_mem_desc *)((ulong)base + count * map->desc_size);
	for (desc = base; desc < end; desc = efi_get_next_mem_desc(map, desc)) {
		bool merge = true;
		int type = desc->type;

		if (skip_bs && is_boot_services(desc->type))
			type = EFI_CONVENTIONAL_MEMORY;

		memcpy(dest, desc, map->desc_size);
		dest->type = type;
		if (!skip_bs || !prev)
			merge = false;
		else if (desc->physical_start != addr)
			merge = false;
		else if (type != EFI_CONVENTIONAL_MEMORY)
			merge = false;
		else if (prev->type != EFI_CONVENTIONAL_MEMORY)
			merge = false;

		if (merge) {
			prev->num_pages += desc->num_pages;
		} else {
			prev = dest;
			dest = efi_get_next_mem_desc(map, dest);
		}
		addr = desc->physical_start + (desc->num_pages <<
				EFI_PAGE_SHIFT);
	}

	/* Mark the end */
	dest->type = EFI_TABLE_END;

	return base;
}

static void efi_print_mem_table(struct efi_entry_memmap *map,
				struct efi_mem_desc *desc, bool skip_bs)
{
	u64 attr_seen[ATTR_SEEN_MAX];
	int attr_seen_count;
	int upto, i;
	u64 addr;

	printf(" #  %-14s  %10s  %10s  %10s  %s\n", "Type", "Physical",
	       "Virtual", "Size", "Attributes");

	/* Keep track of all the different attributes we have seen */
	attr_seen_count = 0;
	addr = 0;
	for (upto = 0; desc->type != EFI_TABLE_END;
	     upto++, desc = efi_get_next_mem_desc(map, desc)) {
		const char *name;
		u64 size;

		if (skip_bs && is_boot_services(desc->type))
			continue;
		if (desc->physical_start != addr) {
			printf("    %-14s  %010llx  %10s  %010llx\n", "<gap>",
			       addr, "", desc->physical_start - addr);
		}
		size = desc->num_pages << EFI_PAGE_SHIFT;

		name = desc->type < ARRAY_SIZE(type_name) ?
				type_name[desc->type] : "<invalid>";
		printf("%2d  %x:%-12s  %010llx  %010llx  %010llx  ", upto,
		       desc->type, name, desc->physical_start,
		       desc->virtual_start, size);
		if (desc->attribute & EFI_MEMORY_RUNTIME)
			putc('r');
		printf("%llx", desc->attribute & ~EFI_MEMORY_RUNTIME);
		putc('\n');

		for (i = 0; i < attr_seen_count; i++) {
			if (attr_seen[i] == desc->attribute)
				break;
		}
		if (i == attr_seen_count && i < ATTR_SEEN_MAX)
			attr_seen[attr_seen_count++] = desc->attribute;
		addr = desc->physical_start + size;
	}

	printf("\nAttributes key:\n");
	for (i = 0; i < attr_seen_count; i++) {
		u64 attr = attr_seen[i];
		bool first;
		int j;

		printf("%c%llx: ", (attr & EFI_MEMORY_RUNTIME) ? 'r' : ' ',
		       attr & ~EFI_MEMORY_RUNTIME);
		for (j = 0, first = true; j < ARRAY_SIZE(mem_attr); j++) {
			if (attr & mem_attr[j].val) {
				if (first)
					first = false;
				else
					printf(", ");
				printf("%s", mem_attr[j].name);
			}
		}
		putc('\n');
	}
	if (skip_bs)
		printf("*Some areas are merged (use 'all' to see)\n");
}

static int do_efi_mem(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	struct efi_mem_desc *desc;
	struct efi_entry_memmap *map;
	int size, ret;
	bool skip_bs;

	skip_bs = !argc || *argv[0] != 'a';
	ret = efi_info_get(EFIET_MEMORY_MAP, (void **)&map, &size);
	switch (ret) {
	case -ENOENT:
		printf("No EFI table available\n");
		goto done;
	case -EPROTONOSUPPORT:
		printf("Incorrect EFI table version\n");
		goto done;
	}
	printf("EFI table at %lx, memory map %p, size %x, version %x, descr. size %#x\n",
	       gd->arch.table, map, size, map->version, map->desc_size);
	if (map->version != EFI_MEM_DESC_VERSION) {
		printf("Incorrect memory map version\n");
		ret = -EPROTONOSUPPORT;
		goto done;
	}

	desc = efi_build_mem_table(map, size, skip_bs);
	if (!desc) {
		ret = -ENOMEM;
		goto done;
	}

	efi_print_mem_table(map, desc, skip_bs);
	free(desc);
done:
	if (ret)
		printf("Error: %d\n", ret);

	return ret ? CMD_RET_FAILURE : 0;
}

static cmd_tbl_t efi_commands[] = {
	U_BOOT_CMD_MKENT(mem, 1, 1, do_efi_mem, "", ""),
};

static int do_efi(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	cmd_tbl_t *efi_cmd;
	int ret;

	if (argc < 2)
		return CMD_RET_USAGE;
	efi_cmd = find_cmd_tbl(argv[1], efi_commands, ARRAY_SIZE(efi_commands));
	argc -= 2;
	argv += 2;
	if (!efi_cmd || argc > efi_cmd->maxargs)
		return CMD_RET_USAGE;

	ret = efi_cmd->cmd(efi_cmd, flag, argc, argv);

	return cmd_process_error(efi_cmd, ret);
}

U_BOOT_CMD(
	efi,     3,      1,      do_efi,
	"EFI access",
	"mem [all]        Dump memory information [include boot services]"
);
