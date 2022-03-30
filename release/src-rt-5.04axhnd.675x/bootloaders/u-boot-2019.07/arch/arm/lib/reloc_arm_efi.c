// SPDX-License-Identifier: BSD-3-Clause
/*
 * reloc_arm.c - position-independent ARM ELF shared object relocator
 *
 * Copyright (C) 2014 Linaro Ltd. <ard.biesheuvel@linaro.org>
 * Copyright (C) 1999 Hewlett-Packard Co.
 * Contributed by David Mosberger <davidm@hpl.hp.com>.
 *
 * All rights reserved.
 *
 * This file is taken and modified from the gnu-efi project.
 */

#include <efi.h>
#include <elf.h>

efi_status_t EFIAPI _relocate(long ldbase, Elf32_Dyn *dyn)
{
	long relsz = 0, relent = 0;
	Elf32_Rel *rel = 0;
	ulong *addr;
	int i;

	for (i = 0; dyn[i].d_tag != DT_NULL; ++i) {
		switch (dyn[i].d_tag) {
		case DT_REL:
			rel = (Elf32_Rel *)((ulong)dyn[i].d_un.d_ptr
					+ ldbase);
			break;
		case DT_RELSZ:
			relsz = dyn[i].d_un.d_val;
			break;
		case DT_RELENT:
			relent = dyn[i].d_un.d_val;
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
		case R_ARM_NONE:
			break;
		case R_ARM_RELATIVE:
			addr = (ulong *)(ldbase + rel->r_offset);
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
