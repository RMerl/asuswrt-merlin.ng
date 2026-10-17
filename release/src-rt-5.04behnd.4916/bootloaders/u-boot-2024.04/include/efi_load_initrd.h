/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (c) 2020, Linaro Limited
 */

#if !defined _EFI_LOAD_INITRD_H_
#define _EFI_LOAD_INITRD_H_

#include <efi.h>
#include <efi_api.h>

/*
 * Vendor GUID used by Linux to identify the handle with the
 * EFI_LOAD_FILE2_PROTOCOL and load an initial ramdisk.
 */
#define EFI_INITRD_MEDIA_GUID \
	EFI_GUID(0x5568e427, 0x68fc, 0x4f3d, \
		 0xac, 0x74, 0xca, 0x55, 0x52, 0x31, 0xcc, 0x68)
extern const efi_guid_t efi_lf2_initrd_guid;

struct efi_initrd_dp {
	struct efi_device_path_vendor vendor;
	struct efi_device_path end;
} __packed;

#endif
