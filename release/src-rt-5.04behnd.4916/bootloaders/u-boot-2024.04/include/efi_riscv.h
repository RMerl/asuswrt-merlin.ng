/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * RISCV_EFI_BOOT_PROTOCOL
 *
 * Copyright (c) 2022 Ventana Micro Systems Inc
 */

#include <efi_api.h>

#define RISCV_EFI_BOOT_PROTOCOL_REVISION 0x00010000

/**
 * struct riscv_efi_boot_protocol - RISCV_EFI_BOOT_PROTOCOL
 * @revision:		Version of the protocol implemented
 * @get_boot_hartid:	Get the boot hart ID
 */
struct riscv_efi_boot_protocol {
	u64 revision;

	efi_status_t (EFIAPI * get_boot_hartid) (struct riscv_efi_boot_protocol *this,
						 efi_uintn_t *boot_hartid);
};

extern struct riscv_efi_boot_protocol riscv_efi_boot_prot;
