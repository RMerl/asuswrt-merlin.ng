// SPDX-License-Identifier: GPL-2.0+
/*
 * Common code for EFI commands
 *
 * Copyright 2023 Google LLC
 * Written by Simon Glass <sjg@chromium.org>
 */

#include <common.h>
#include <efi.h>
#include <efi_api.h>
#include <uuid.h>

void efi_show_tables(struct efi_system_table *systab)
{
	int i;

	for (i = 0; i < systab->nr_tables; i++) {
		struct efi_configuration_table *tab = &systab->tables[i];

		printf("%p  %pUl  %s\n", tab->table, tab->guid.b,
		       uuid_guid_get_str(tab->guid.b) ?: "(unknown)");
	}
}
