/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (c) 2019, Linaro Limited
 */

#if !defined _EFI_RNG_H_
#define _EFI_RNG_H_

#include <efi.h>
#include <efi_api.h>

/* EFI random number generation protocol related GUID definitions */
#define EFI_RNG_ALGORITHM_RAW \
	EFI_GUID(0xe43176d7, 0xb6e8, 0x4827, 0xb7, 0x84, \
		 0x7f, 0xfd, 0xc4, 0xb6, 0x85, 0x61)

struct efi_rng_protocol {
	efi_status_t (EFIAPI *get_info)(struct efi_rng_protocol *protocol,
					efi_uintn_t *rng_algorithm_list_size,
					efi_guid_t *rng_algorithm_list);
	efi_status_t (EFIAPI *get_rng)(struct efi_rng_protocol *protocol,
				       efi_guid_t *rng_algorithm,
				       efi_uintn_t rng_value_length, uint8_t *rng_value);
};

efi_status_t platform_get_rng_device(struct udevice **dev);

#endif /* _EFI_RNG_H_ */
