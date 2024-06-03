// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2015 Google, Inc
 *
 * Access to the EFI information table
 */

#include <common.h>
#include <efi.h>
#include <errno.h>
#include <mapmem.h>

int efi_info_get(enum efi_entry_t type, void **datap, int *sizep)
{
	struct efi_entry_hdr *entry;
	struct efi_info_hdr *info;
	int ret;

	if (!gd->arch.table)
		return -ENODATA;

	info = map_sysmem(gd->arch.table, 0);
	if (info->version != EFI_TABLE_VERSION) {
		ret = -EPROTONOSUPPORT;
		goto err;
	}

	entry = (struct efi_entry_hdr *)((ulong)info + info->hdr_size);
	while (entry->type != EFIET_END) {
		if (entry->type == type) {
			if (entry->addr)
				*datap = map_sysmem(entry->addr, entry->size);
			else
				*datap = entry + 1;
			*sizep = entry->size;
			return 0;
		}
		entry = (struct efi_entry_hdr *)((ulong)entry + entry->link);
	}

	ret = -ENOENT;
err:
	unmap_sysmem(info);

	return ret;
}
