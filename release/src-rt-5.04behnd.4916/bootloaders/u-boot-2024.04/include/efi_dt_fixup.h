/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * EFI_DT_FIXUP_PROTOCOL
 *
 * Copyright (c) 2020 Heinrich Schuchardt
 */

#include <efi_api.h>

#define EFI_DT_FIXUP_PROTOCOL_REVISION 0x00010000

/* Add nodes and update properties */
#define EFI_DT_APPLY_FIXUPS    0x00000001
/*
 * Reserve memory according to the /reserved-memory node
 * and the memory reservation block
 */
#define EFI_DT_RESERVE_MEMORY  0x00000002
/* Install the device-tree as configuration table */
#define EFI_DT_INSTALL_TABLE   0x00000004

#define EFI_DT_ALL (EFI_DT_APPLY_FIXUPS | \
		    EFI_DT_RESERVE_MEMORY | \
		    EFI_DT_INSTALL_TABLE)

struct efi_dt_fixup_protocol {
	u64 revision;
	efi_status_t (EFIAPI *fixup) (struct efi_dt_fixup_protocol *this,
				      void *dtb,
				      efi_uintn_t *buffer_size,
				      u32 flags);
};

extern struct efi_dt_fixup_protocol efi_dt_fixup_prot;
extern const efi_guid_t efi_guid_dt_fixup_protocol;
