// SPDX-License-Identifier: BSD-3-Clause
/*
 * reloc_ia32.c - position independent x86 ELF shared object relocator
 * Copyright (C) 1999 Hewlett-Packard Co.
 * Contributed by David Mosberger <davidm@hpl.hp.com>.
 *
 * All rights reserved.
 */

#include <common.h>
#include <efi.h>
#include <elf.h>

efi_status_t EFIAPI _relocate(long ldbase, Elf32_Dyn *dyn)
{
	long relsz = 0, relent = 0;
	Elf32_Rel *rel = 0;
	unsigned long *addr;
	int i;

	for (i = 0; dyn[i].d_tag != DT_NULL; ++i) {
		switch (dyn[i].d_tag) {
		case DT_REL:
			rel = (Elf32_Rel *)((unsigned long)dyn[i].d_un.d_ptr +
								ldbase);
			break;

		case DT_RELSZ:
			relsz = dyn[i].d_un.d_val;
			break;

		case DT_RELENT:
			relent = dyn[i].d_un.d_val;
			break;

		case DT_RELA:
			break;

		default:
			break;
		}
	}

	if (!rel && relent == 0)
		return EFI_SUCCESS;

	if (!rel || relent == 0)
		return EFI_LOAD_ERROR;

	while (relsz > 0) {
		/* apply the relocs */
		switch (ELF32_R_TYPE(rel->r_info)) {
		case R_386_NONE:
			break;

		case R_386_RELATIVE:
			addr = (unsigned long *)(ldbase + rel->r_offset);
			*addr += ldbase;
			break;

		default:
			break;
		}
		rel = (Elf32_Rel *)((char *)rel + relent);
		relsz -= relent;
	}

	return EFI_SUCCESS;
}
